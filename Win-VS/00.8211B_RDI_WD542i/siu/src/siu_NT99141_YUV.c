
/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu_NT99141_YUV.c

Abstract:

    The routines of Sensor Interface Unit.
    Control NT99141 (1.3M) sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2009/04/17  Lucian Yuan  Create  
*/


#include "general.h"
#if (Sensor_OPTION == Sensor_NT99141_YUV601)
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
#define NT99141_FRAME_RATE  15
#else   // (VIDEO_RESOLUTION_SEL == VIDEO_VGA_IN_VGA_OUT)
#define NT99141_FRAME_RATE  30
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

#if((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
#if(SENSOR_PARAMETER_VERSION == 1) 
const s8 AETargetMeanTab[9] = {0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0};
#else
const s8 AETargetMeanTab[9] = {0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0};
#endif //end of SENSOR_PARAMETER_VERSION
#else
const s8 AETargetMeanTab[9] = {0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0};
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

#if 0   // first version

void SetNT99141_720P_15FPS(void)
{
    int i;
    
    DEBUG_SIU("SetNT99141_720P_15FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3108, 0x00);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0xA0);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3250, 0xC0);
    i2cWrite_NT99141(0x3251, 0x00);
    i2cWrite_NT99141(0x3252, 0xDF);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xEB);
    i2cWrite_NT99141(0x3256, 0x81);
    i2cWrite_NT99141(0x3257, 0x40);
    i2cWrite_NT99141(0x329B, 0x01);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3210, 0x16);
    i2cWrite_NT99141(0x3211, 0x19);
    i2cWrite_NT99141(0x3212, 0x16);
    i2cWrite_NT99141(0x3213, 0x14);
    i2cWrite_NT99141(0x3214, 0x15);
    i2cWrite_NT99141(0x3215, 0x18);
    i2cWrite_NT99141(0x3216, 0x15);
    i2cWrite_NT99141(0x3217, 0x14);
    i2cWrite_NT99141(0x3218, 0x15);
    i2cWrite_NT99141(0x3219, 0x18);
    i2cWrite_NT99141(0x321A, 0x15);
    i2cWrite_NT99141(0x321B, 0x14);
    i2cWrite_NT99141(0x321C, 0x14);
    i2cWrite_NT99141(0x321D, 0x17);
    i2cWrite_NT99141(0x321E, 0x14);
    i2cWrite_NT99141(0x321F, 0x12);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x40);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0x96);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x29);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xBA);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0xF5);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0x51);
    i2cWrite_NT99141(0x330E, 0x01);
    i2cWrite_NT99141(0x330F, 0x30);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x16);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xBA);
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x3327, 0x0A);
    i2cWrite_NT99141(0x3328, 0x0A);
    i2cWrite_NT99141(0x3329, 0x06);
    i2cWrite_NT99141(0x332A, 0x06);
    i2cWrite_NT99141(0x332B, 0x1C);
    i2cWrite_NT99141(0x332C, 0x1C);
    i2cWrite_NT99141(0x332D, 0x00);
    i2cWrite_NT99141(0x332E, 0x1D);
    i2cWrite_NT99141(0x332F, 0x1F);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x30);
    i2cWrite_NT99141(0x3369, 0x28);
    i2cWrite_NT99141(0x336A, 0x20);
    i2cWrite_NT99141(0x336B, 0x10);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x38);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x0A);
    i2cWrite_NT99141(0x3332, 0xFF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3375, 0x0A);
    i2cWrite_NT99141(0x3376, 0x0C);
    i2cWrite_NT99141(0x3377, 0x10);
    i2cWrite_NT99141(0x3378, 0x14);
    i2cWrite_NT99141(0x3060, 0x01);
    i2cWrite_NT99141(0x300A, 0x06);
    i2cWrite_NT99141(0x300B, 0x3D);
    i2cWrite_NT99141(0x32F0, 0x00);
    i2cWrite_NT99141(0x306A, 0x02);
    i2cWrite_NT99141(0x3060, 0x01);

}

#elif 0 // 20130923

void SetNT99141_720P_15FPS(void)
{
    int i;
    
    DEBUG_SIU("SetNT99141_720P_15FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3108, 0x00);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0xA0);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3250, 0xC0);
    i2cWrite_NT99141(0x3251, 0x00);
    i2cWrite_NT99141(0x3252, 0xDF);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xEB);
    i2cWrite_NT99141(0x3256, 0x81);
    i2cWrite_NT99141(0x3257, 0x40);
    i2cWrite_NT99141(0x329B, 0x01);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3210, 0x16);
    i2cWrite_NT99141(0x3211, 0x19);
    i2cWrite_NT99141(0x3212, 0x16);
    i2cWrite_NT99141(0x3213, 0x14);
    i2cWrite_NT99141(0x3214, 0x15);
    i2cWrite_NT99141(0x3215, 0x18);
    i2cWrite_NT99141(0x3216, 0x15);
    i2cWrite_NT99141(0x3217, 0x14);
    i2cWrite_NT99141(0x3218, 0x15);
    i2cWrite_NT99141(0x3219, 0x18);
    i2cWrite_NT99141(0x321A, 0x15);
    i2cWrite_NT99141(0x321B, 0x14);
    i2cWrite_NT99141(0x321C, 0x14);
    i2cWrite_NT99141(0x321D, 0x17);
    i2cWrite_NT99141(0x321E, 0x14);
    i2cWrite_NT99141(0x321F, 0x12);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x40);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0x96);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x29);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xBA);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0xF5);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0x51);
    i2cWrite_NT99141(0x330E, 0x01);
    i2cWrite_NT99141(0x330F, 0x30);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x16);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xBA);
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x3327, 0x0A);
    i2cWrite_NT99141(0x3328, 0x0A);
    i2cWrite_NT99141(0x3329, 0x06);
    i2cWrite_NT99141(0x332A, 0x06);
    i2cWrite_NT99141(0x332B, 0x1C);
    i2cWrite_NT99141(0x332C, 0x1C);
    i2cWrite_NT99141(0x332D, 0x00);
    i2cWrite_NT99141(0x332E, 0x1D);
    i2cWrite_NT99141(0x332F, 0x1F);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x30);
    i2cWrite_NT99141(0x3369, 0x28);
    i2cWrite_NT99141(0x336A, 0x20);
    i2cWrite_NT99141(0x336B, 0x10);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x38);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x0A);
    i2cWrite_NT99141(0x3332, 0xFF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3375, 0x0A);
    i2cWrite_NT99141(0x3376, 0x0C);
    i2cWrite_NT99141(0x3377, 0x10);
    i2cWrite_NT99141(0x3378, 0x14);
    i2cWrite_NT99141(0x3060, 0x01);

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

}

#elif(HW_BOARD_OPTION == MR8120_TX_RDI)

// [Date] 20130930
// [Sensor] NT99141
// MCLK = 24Mhz
// PCLK = 40Mhz
// Frame Rate
// Preview: 15fps for Yuv
// Flicker: 60hz/50hz
void SetNT99141_720P_15FPS_RDI(void)
{
    int i;
    
    DEBUG_SIU("SetNT99141_720P_15FPS_RDI()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

#if 0
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3250,0xC0);
    i2cWrite_NT99141(0x3251,0x00);
    i2cWrite_NT99141(0x3252,0xF0);
    i2cWrite_NT99141(0x3253,0xA0);
    i2cWrite_NT99141(0x3254,0x00);
    i2cWrite_NT99141(0x3255,0xC0);
    i2cWrite_NT99141(0x3256,0x50);
    i2cWrite_NT99141(0x3257,0x40);
    i2cWrite_NT99141(0x329B,0x00);
    i2cWrite_NT99141(0x32A1,0x00);
    i2cWrite_NT99141(0x32A2,0xA0);
    i2cWrite_NT99141(0x32A3,0x01);
    i2cWrite_NT99141(0x32A4,0xA0);
    i2cWrite_NT99141(0x32A5,0x01);
    i2cWrite_NT99141(0x32A6,0x18);
    i2cWrite_NT99141(0x32A7,0x01);
    i2cWrite_NT99141(0x32A8,0xE0);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x2E);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xEB);
    i2cWrite_NT99141(0x3306,0x07);
    i2cWrite_NT99141(0x3307,0xE5);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0x89);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x7F);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xF9);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0xDD);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0x86);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0x9E);
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14); 
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x306A,0x03);
    i2cWrite_NT99141(0x3060,0x01);
#elif 0   // 20140224
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0x80);
    i2cWrite_NT99141(0x3251, 0x03);
    i2cWrite_NT99141(0x3252, 0xFF);
    i2cWrite_NT99141(0x3253, 0x00);
    i2cWrite_NT99141(0x3254, 0x03);
    i2cWrite_NT99141(0x3255, 0xFF);
    i2cWrite_NT99141(0x3256, 0x00);
    i2cWrite_NT99141(0x3257, 0x40);
    i2cWrite_NT99141(0x329B, 0x00);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x40);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0x96);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x29);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xBA);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0xF5);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0x51);
    i2cWrite_NT99141(0x330E, 0x01);
    i2cWrite_NT99141(0x330F, 0x30);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x16);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xBA);
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x0A);
    i2cWrite_NT99141(0x3332, 0xFF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x30);
    i2cWrite_NT99141(0x3369, 0x28);
    i2cWrite_NT99141(0x336A, 0x20);
    i2cWrite_NT99141(0x336B, 0x10);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x38);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
	//i2cWrite_NT99141(0x3069, 0x01);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#elif 0 // 20140707
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8),
    i2cWrite_NT99141(0x3251, 0x01),
    i2cWrite_NT99141(0x3252, 0x1B),
    i2cWrite_NT99141(0x3253, 0x85),
    i2cWrite_NT99141(0x3254, 0x00),
    i2cWrite_NT99141(0x3255, 0xC9),
    i2cWrite_NT99141(0x3256, 0x91),
    i2cWrite_NT99141(0x3257, 0x46),
    i2cWrite_NT99141(0x3258, 0x0A),
    i2cWrite_NT99141(0x329B, 0x31),
    i2cWrite_NT99141(0x32A1, 0x00),
    i2cWrite_NT99141(0x32A2, 0xFF),
    i2cWrite_NT99141(0x32A3, 0x01),
    i2cWrite_NT99141(0x32A4, 0xB0),
    i2cWrite_NT99141(0x32A5, 0x01),
    i2cWrite_NT99141(0x32A6, 0x50),
    i2cWrite_NT99141(0x32A7, 0x01),
    i2cWrite_NT99141(0x32A8, 0xE0),
    i2cWrite_NT99141(0x3270, 0x00),
    i2cWrite_NT99141(0x3271, 0x05),
    i2cWrite_NT99141(0x3272, 0x11),
    i2cWrite_NT99141(0x3273, 0x28),
    i2cWrite_NT99141(0x3274, 0x43),
    i2cWrite_NT99141(0x3275, 0x5C),
    i2cWrite_NT99141(0x3276, 0x7E),
    i2cWrite_NT99141(0x3277, 0x9B),
    i2cWrite_NT99141(0x3278, 0xB3),
    i2cWrite_NT99141(0x3279, 0xC6),
    i2cWrite_NT99141(0x327A, 0xD9),
    i2cWrite_NT99141(0x327B, 0xE7),
    i2cWrite_NT99141(0x327C, 0xF6),
    i2cWrite_NT99141(0x327D, 0xFF),
    i2cWrite_NT99141(0x327E, 0xFF),
    i2cWrite_NT99141(0x3302, 0x00),
    i2cWrite_NT99141(0x3303, 0x00),
    i2cWrite_NT99141(0x3304, 0x00),
    i2cWrite_NT99141(0x3305, 0xE1),
    i2cWrite_NT99141(0x3306, 0x00),
    i2cWrite_NT99141(0x3307, 0x1E),
    i2cWrite_NT99141(0x3308, 0x07),
    i2cWrite_NT99141(0x3309, 0xCF),
    i2cWrite_NT99141(0x330A, 0x06),
    i2cWrite_NT99141(0x330B, 0x8F),
    i2cWrite_NT99141(0x330C, 0x01),
    i2cWrite_NT99141(0x330D, 0xA2),
    i2cWrite_NT99141(0x330E, 0x00),
    i2cWrite_NT99141(0x330F, 0xF0),
    i2cWrite_NT99141(0x3310, 0x07),
    i2cWrite_NT99141(0x3311, 0x19),
    i2cWrite_NT99141(0x3312, 0x07),
    i2cWrite_NT99141(0x3313, 0xF7),
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x78);
    i2cWrite_NT99141(0x3369, 0x60);
    i2cWrite_NT99141(0x336A, 0x48);
    i2cWrite_NT99141(0x336B, 0x32);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#elif 0   // 20140708
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8),
    i2cWrite_NT99141(0x3251, 0x01),
    i2cWrite_NT99141(0x3252, 0x1B),
    i2cWrite_NT99141(0x3253, 0x85),
    i2cWrite_NT99141(0x3254, 0x00),
    i2cWrite_NT99141(0x3255, 0xC9),
    i2cWrite_NT99141(0x3256, 0x91),
    i2cWrite_NT99141(0x3257, 0x46),
    i2cWrite_NT99141(0x3258, 0x0A),
    i2cWrite_NT99141(0x329B, 0x31),
    i2cWrite_NT99141(0x32A1, 0x00),
    i2cWrite_NT99141(0x32A2, 0xFF),
    i2cWrite_NT99141(0x32A3, 0x01),
    i2cWrite_NT99141(0x32A4, 0xB0),
    i2cWrite_NT99141(0x32A5, 0x01),
    i2cWrite_NT99141(0x32A6, 0x50),
    i2cWrite_NT99141(0x32A7, 0x01),
    i2cWrite_NT99141(0x32A8, 0xE0),
    i2cWrite_NT99141(0x3270, 0x00),
    i2cWrite_NT99141(0x3271, 0x05),
    i2cWrite_NT99141(0x3272, 0x11),
    i2cWrite_NT99141(0x3273, 0x28),
    i2cWrite_NT99141(0x3274, 0x43),
    i2cWrite_NT99141(0x3275, 0x5C),
    i2cWrite_NT99141(0x3276, 0x7E),
    i2cWrite_NT99141(0x3277, 0x9B),
    i2cWrite_NT99141(0x3278, 0xB3),
    i2cWrite_NT99141(0x3279, 0xC6),
    i2cWrite_NT99141(0x327A, 0xD9),
    i2cWrite_NT99141(0x327B, 0xE7),
    i2cWrite_NT99141(0x327C, 0xF6),
    i2cWrite_NT99141(0x327D, 0xFF),
    i2cWrite_NT99141(0x327E, 0xFF),
    i2cWrite_NT99141(0x3302, 0x00),
    i2cWrite_NT99141(0x3303, 0x00),
    i2cWrite_NT99141(0x3304, 0x00),
    i2cWrite_NT99141(0x3305, 0xE1),
    i2cWrite_NT99141(0x3306, 0x00),
    i2cWrite_NT99141(0x3307, 0x1E),
    i2cWrite_NT99141(0x3308, 0x07),
    i2cWrite_NT99141(0x3309, 0xCF),
    i2cWrite_NT99141(0x330A, 0x06),
    i2cWrite_NT99141(0x330B, 0x8F),
    i2cWrite_NT99141(0x330C, 0x01),
    i2cWrite_NT99141(0x330D, 0xA2),
    i2cWrite_NT99141(0x330E, 0x00),
    i2cWrite_NT99141(0x330F, 0xF0),
    i2cWrite_NT99141(0x3310, 0x07),
    i2cWrite_NT99141(0x3311, 0x19),
    i2cWrite_NT99141(0x3312, 0x07),
    i2cWrite_NT99141(0x3313, 0xF7),
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x56);
    i2cWrite_NT99141(0x3368, 0x70);
    i2cWrite_NT99141(0x3369, 0x58);
    i2cWrite_NT99141(0x336A, 0x40);
    i2cWrite_NT99141(0x336B, 0x2C);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1A);
    i2cWrite_NT99141(0x336F, 0x16);
    i2cWrite_NT99141(0x3370, 0x0E);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#else   // 20140710
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8);
    i2cWrite_NT99141(0x3251, 0x01);
    i2cWrite_NT99141(0x3252, 0x1B);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xC9);
    i2cWrite_NT99141(0x3256, 0x91);
    i2cWrite_NT99141(0x3257, 0x46);
    i2cWrite_NT99141(0x3258, 0x0A);
    i2cWrite_NT99141(0x329B, 0x31);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xFF);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xB0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x50);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x05);
    i2cWrite_NT99141(0x3272, 0x11);
    i2cWrite_NT99141(0x3273, 0x28);
    i2cWrite_NT99141(0x3274, 0x43);
    i2cWrite_NT99141(0x3275, 0x5C);
    i2cWrite_NT99141(0x3276, 0x7E);
    i2cWrite_NT99141(0x3277, 0x9B);
    i2cWrite_NT99141(0x3278, 0xB3);
    i2cWrite_NT99141(0x3279, 0xC6);
    i2cWrite_NT99141(0x327A, 0xD9);
    i2cWrite_NT99141(0x327B, 0xE7);
    i2cWrite_NT99141(0x327C, 0xF6);
    i2cWrite_NT99141(0x327D, 0xFF);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x00);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0xE1);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x1E);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xCF);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0x8F);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0xA2);
    i2cWrite_NT99141(0x330E, 0x00);
    i2cWrite_NT99141(0x330F, 0xF0);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x19);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xF7);
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x70);
    i2cWrite_NT99141(0x3366, 0x60);
    i2cWrite_NT99141(0x3367, 0x48);
    i2cWrite_NT99141(0x3368, 0x70);
    i2cWrite_NT99141(0x3369, 0x58);
    i2cWrite_NT99141(0x336A, 0x40);
    i2cWrite_NT99141(0x336B, 0x2C);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1A);
    i2cWrite_NT99141(0x336F, 0x16);
    i2cWrite_NT99141(0x3370, 0x0E);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#endif

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
}

#else   // 20130930

// [Date] 20130930
// [Sensor] NT99141
// MCLK = 24Mhz
// PCLK = 40Mhz
// Frame Rate
// Preview: 15fps for Yuv
// Flicker: 60hz/50hz
void SetNT99141_720P_15FPS(void)
{
    int i;
    
    DEBUG_SIU("SetNT99141_720P_15FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

    i2cWrite_NT99141(0x3109, 0x04);	
    i2cWrite_NT99141(0x3040, 0x04);	
    i2cWrite_NT99141(0x3041, 0x02);	
    i2cWrite_NT99141(0x3042, 0xFF);	
    i2cWrite_NT99141(0x3043, 0x08);	
    i2cWrite_NT99141(0x3052, 0xE0);	
    i2cWrite_NT99141(0x305F, 0x33);	
    i2cWrite_NT99141(0x3100, 0x07);	
    i2cWrite_NT99141(0x3106, 0x03);	
    i2cWrite_NT99141(0x3108, 0x00);	
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);	
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);	
    i2cWrite_NT99141(0x3135, 0x00);	
    i2cWrite_NT99141(0x32F0, 0x01);	
    i2cWrite_NT99141(0x3290, 0x01);	
    i2cWrite_NT99141(0x3291, 0xA0);	 
    i2cWrite_NT99141(0x3296, 0x01);	
    i2cWrite_NT99141(0x3297, 0x73);	
    i2cWrite_NT99141(0x3250, 0xC0); 
    i2cWrite_NT99141(0x3251, 0x00);  
    i2cWrite_NT99141(0x3252, 0xDF);  
    i2cWrite_NT99141(0x3253, 0x85);  
    i2cWrite_NT99141(0x3254, 0x00);  
    i2cWrite_NT99141(0x3255, 0xEB);  
    i2cWrite_NT99141(0x3256, 0x81);  
    i2cWrite_NT99141(0x3257, 0x40);	
    i2cWrite_NT99141(0x329B, 0x01);	
    i2cWrite_NT99141(0x32A1, 0x00);	
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);	
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);	
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);	
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3210, 0x16); 
    i2cWrite_NT99141(0x3211, 0x19); 
    i2cWrite_NT99141(0x3212, 0x16); 
    i2cWrite_NT99141(0x3213, 0x14);  
    i2cWrite_NT99141(0x3214, 0x15);  
    i2cWrite_NT99141(0x3215, 0x18);  
    i2cWrite_NT99141(0x3216, 0x15);  
    i2cWrite_NT99141(0x3217, 0x14);  
    i2cWrite_NT99141(0x3218, 0x15);  
    i2cWrite_NT99141(0x3219, 0x18);  
    i2cWrite_NT99141(0x321A, 0x15);  
    i2cWrite_NT99141(0x321B, 0x14);  
    i2cWrite_NT99141(0x321C, 0x14);  
    i2cWrite_NT99141(0x321D, 0x17);  
    i2cWrite_NT99141(0x321E, 0x14);  
    i2cWrite_NT99141(0x321F, 0x12);  
    i2cWrite_NT99141(0x3231, 0x74);  
    i2cWrite_NT99141(0x3232, 0xC4);  
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00); 
    i2cWrite_NT99141(0x3303, 0x40);  
    i2cWrite_NT99141(0x3304, 0x00);  
    i2cWrite_NT99141(0x3305, 0x96);  
    i2cWrite_NT99141(0x3306, 0x00);  
    i2cWrite_NT99141(0x3307, 0x29);  
    i2cWrite_NT99141(0x3308, 0x07);  
    i2cWrite_NT99141(0x3309, 0xBA);  
    i2cWrite_NT99141(0x330A, 0x06);  
    i2cWrite_NT99141(0x330B, 0xF5);  
    i2cWrite_NT99141(0x330C, 0x01);  
    i2cWrite_NT99141(0x330D, 0x51);  
    i2cWrite_NT99141(0x330E, 0x01);  
    i2cWrite_NT99141(0x330F, 0x30);  
    i2cWrite_NT99141(0x3310, 0x07);  
    i2cWrite_NT99141(0x3311, 0x16);  
    i2cWrite_NT99141(0x3312, 0x07);  
    i2cWrite_NT99141(0x3313, 0xBA);  
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x3327, 0x0A);
    i2cWrite_NT99141(0x3328, 0x0A);
    i2cWrite_NT99141(0x3329, 0x06);
    i2cWrite_NT99141(0x332A, 0x06);
    i2cWrite_NT99141(0x332B, 0x1C);
    i2cWrite_NT99141(0x332C, 0x1C);
    i2cWrite_NT99141(0x332D, 0x00);
    i2cWrite_NT99141(0x332E, 0x1D);
    i2cWrite_NT99141(0x332F, 0x1F);
    i2cWrite_NT99141(0x3360, 0x10);  
    i2cWrite_NT99141(0x3361, 0x18); 
    i2cWrite_NT99141(0x3362, 0x1f);  
    i2cWrite_NT99141(0x3363, 0x37); 
    i2cWrite_NT99141(0x3364, 0x80);  
    i2cWrite_NT99141(0x3365, 0x80); 
    i2cWrite_NT99141(0x3366, 0x64);//0x68  
    i2cWrite_NT99141(0x3367, 0x4C);//0x60  
    i2cWrite_NT99141(0x3368, 0x30); 
    i2cWrite_NT99141(0x3369, 0x28);  
    i2cWrite_NT99141(0x336A, 0x20);  
    i2cWrite_NT99141(0x336B, 0x12);//0x10  
    i2cWrite_NT99141(0x336C, 0x00);  
    i2cWrite_NT99141(0x336D, 0x20);  
    i2cWrite_NT99141(0x336E, 0x1C);  
    i2cWrite_NT99141(0x336F, 0x18);  
    i2cWrite_NT99141(0x3370, 0x10);  
    i2cWrite_NT99141(0x3371, 0x38);  
    i2cWrite_NT99141(0x3372, 0x3C);  
    i2cWrite_NT99141(0x3373, 0x3F);  
    i2cWrite_NT99141(0x3374, 0x3F);   
    i2cWrite_NT99141(0x338A, 0x34);  
    i2cWrite_NT99141(0x338B, 0x7F); 
    i2cWrite_NT99141(0x338C, 0x10); 
    i2cWrite_NT99141(0x338D, 0x23);  
    i2cWrite_NT99141(0x338E, 0x7F);  
    i2cWrite_NT99141(0x338F, 0x14);  
    i2cWrite_NT99141(0x32F6, 0x0F);	
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);	
    i2cWrite_NT99141(0x3330, 0x00);	
    i2cWrite_NT99141(0x3331, 0x08);	
    i2cWrite_NT99141(0x3332, 0xFF);	
    i2cWrite_NT99141(0x3338, 0x30);	
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);	
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x12);  
    i2cWrite_NT99141(0x3378, 0x18); 
    i2cWrite_NT99141(0x3069, 0x03); 
//#if (HW_BOARD_OPTION == MR8120_TX_COMMAX)
//    i2cWrite_NT99141(0x306A, 0x00);
//#else
    i2cWrite_NT99141(0x306A, 0x03);
//#endif
    i2cWrite_NT99141(0x3060, 0x01);

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

}


#endif


#if((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
#if(SENSOR_PARAMETER == 0)
void SetNT99141_VGA_30FPS_TRANWO(void)
{
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
#if(SENSOR_PARAMETER_VERSION == 1) // use new parameter
	i2cWrite_NT99141(0x3109, 0x04);
	i2cWrite_NT99141(0x3040, 0x04);
	i2cWrite_NT99141(0x3041, 0x02);
	i2cWrite_NT99141(0x3042, 0xFF);
	i2cWrite_NT99141(0x3043, 0x08);
	i2cWrite_NT99141(0x3052, 0xE0);
	i2cWrite_NT99141(0x305F, 0x33);
	i2cWrite_NT99141(0x3100, 0x07);
	i2cWrite_NT99141(0x3106, 0x03);
	i2cWrite_NT99141(0x3105, 0x01);
	i2cWrite_NT99141(0x3108, 0x05);
	i2cWrite_NT99141(0x3110, 0x22);
	i2cWrite_NT99141(0x3111, 0x57);
	i2cWrite_NT99141(0x3112, 0x22);
	i2cWrite_NT99141(0x3113, 0x55);
	i2cWrite_NT99141(0x3114, 0x05);
	i2cWrite_NT99141(0x3135, 0x00);
	i2cWrite_NT99141(0x32F0, 0x01);
	i2cWrite_NT99141(0x3290, 0x01);
	i2cWrite_NT99141(0x3291, 0x80);
	i2cWrite_NT99141(0x3296, 0x01);
	i2cWrite_NT99141(0x3297, 0x73);
	i2cWrite_NT99141(0x3250, 0xc0);
	i2cWrite_NT99141(0x3251, 0x00);
	i2cWrite_NT99141(0x3252, 0xE3);
	i2cWrite_NT99141(0x3253, 0x8C);
	i2cWrite_NT99141(0x3254, 0x00);
	i2cWrite_NT99141(0x3255, 0xF0);
	i2cWrite_NT99141(0x3256, 0x7C);
	i2cWrite_NT99141(0x3257, 0x38);
	i2cWrite_NT99141(0x329b, 0x01);
	i2cWrite_NT99141(0x32a1, 0x00);
	i2cWrite_NT99141(0x32a2, 0xa0);
	i2cWrite_NT99141(0x32a3, 0x01);
	i2cWrite_NT99141(0x32a4, 0xA0);
	i2cWrite_NT99141(0x32a5, 0x01);
	i2cWrite_NT99141(0x32a6, 0x18);
	i2cWrite_NT99141(0x32a7, 0x01);
	i2cWrite_NT99141(0x32a8, 0xe0);

	i2cWrite_NT99141(0x3210, 0x11); 
	i2cWrite_NT99141(0x3211, 0x14); 
	i2cWrite_NT99141(0x3212, 0x11); 
	i2cWrite_NT99141(0x3213, 0x10); 
	i2cWrite_NT99141(0x3214, 0x0F); 
	i2cWrite_NT99141(0x3215, 0x12); 
	i2cWrite_NT99141(0x3216, 0x10); 
	i2cWrite_NT99141(0x3217, 0x0F); 
	i2cWrite_NT99141(0x3218, 0x0F); 
	i2cWrite_NT99141(0x3219, 0x13); 
	i2cWrite_NT99141(0x321A, 0x10); 
	i2cWrite_NT99141(0x321B, 0x0F); 
	i2cWrite_NT99141(0x321C, 0x0F); 
	i2cWrite_NT99141(0x321D, 0x12); 
	i2cWrite_NT99141(0x321E, 0x0F); 
	i2cWrite_NT99141(0x321F, 0x0D); 
	i2cWrite_NT99141(0x3231, 0x74); 
	i2cWrite_NT99141(0x3232, 0xC4); 
	i2cWrite_NT99141(0x3270, 0x00);
	i2cWrite_NT99141(0x3271, 0x0C);
	i2cWrite_NT99141(0x3272, 0x17);
	i2cWrite_NT99141(0x3273, 0x2E);
	i2cWrite_NT99141(0x3274, 0x46);
	i2cWrite_NT99141(0x3275, 0x5D);
	i2cWrite_NT99141(0x3276, 0x85);
	i2cWrite_NT99141(0x3277, 0xA2);
	i2cWrite_NT99141(0x3278, 0xBA);
	i2cWrite_NT99141(0x3279, 0xCD);
	i2cWrite_NT99141(0x327a, 0xE9);
	i2cWrite_NT99141(0x327b, 0xf6);
	i2cWrite_NT99141(0x327c, 0xfB);
	i2cWrite_NT99141(0x327d, 0xfe);
	i2cWrite_NT99141(0x3302, 0x00);
	i2cWrite_NT99141(0x3303, 0x07); 
	i2cWrite_NT99141(0x3304, 0x00);
	i2cWrite_NT99141(0x3305, 0xcb);
	i2cWrite_NT99141(0x3306, 0x00);
	i2cWrite_NT99141(0x3307, 0x2c);
	i2cWrite_NT99141(0x3308, 0x07);
	i2cWrite_NT99141(0x3309, 0xd7);
	i2cWrite_NT99141(0x330a, 0x06);
	i2cWrite_NT99141(0x330b, 0xc5);
	i2cWrite_NT99141(0x330c, 0x01);
	i2cWrite_NT99141(0x330d, 0x65);
	i2cWrite_NT99141(0x330e, 0x01);
	i2cWrite_NT99141(0x330f, 0x1e);
	i2cWrite_NT99141(0x3310, 0x07);
	i2cWrite_NT99141(0x3311, 0x1d);
	i2cWrite_NT99141(0x3312, 0x07);
	i2cWrite_NT99141(0x3313, 0xc6);
	i2cWrite_NT99141(0x3326, 0x03);
	i2cWrite_NT99141(0x32F6, 0x0F);
#if(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    i2cWrite_NT99141(0x32F3, 0x98);
#else
	i2cWrite_NT99141(0x32F3, 0xB0);
#endif
	i2cWrite_NT99141(0x32F9, 0x42);
	i2cWrite_NT99141(0x32FA, 0x24);
	i2cWrite_NT99141(0x3325, 0x4A);
	i2cWrite_NT99141(0x3330, 0x00);
	i2cWrite_NT99141(0x3331, 0x0A);
	i2cWrite_NT99141(0x3332, 0xA0);
	i2cWrite_NT99141(0x3338, 0x30);
	i2cWrite_NT99141(0x3339, 0x84);
	i2cWrite_NT99141(0x333A, 0x48);
	i2cWrite_NT99141(0x333F, 0x07);                
	i2cWrite_NT99141(0x3360, 0x10);
	i2cWrite_NT99141(0x3361, 0x18);
	i2cWrite_NT99141(0x3362, 0x1f);
	i2cWrite_NT99141(0x3363, 0x37);
	i2cWrite_NT99141(0x3364, 0x80);
	i2cWrite_NT99141(0x3365, 0x80);
	i2cWrite_NT99141(0x3366, 0x68);
	i2cWrite_NT99141(0x3367, 0x50);
	i2cWrite_NT99141(0x3368, 0x28);
	i2cWrite_NT99141(0x3369, 0x22);
	i2cWrite_NT99141(0x336A, 0x1C);
	i2cWrite_NT99141(0x336B, 0x10);
	i2cWrite_NT99141(0x336C, 0x00);
	i2cWrite_NT99141(0x336D, 0x20);
	i2cWrite_NT99141(0x336E, 0x1C);
	i2cWrite_NT99141(0x336F, 0x18);
	i2cWrite_NT99141(0x3370, 0x10);
	i2cWrite_NT99141(0x3371, 0x38);
	i2cWrite_NT99141(0x3372, 0x3C);
	i2cWrite_NT99141(0x3373, 0x3F);
	i2cWrite_NT99141(0x3374, 0x3F);
	i2cWrite_NT99141(0x338A, 0x34);
	i2cWrite_NT99141(0x338B, 0x7F);
	i2cWrite_NT99141(0x338C, 0x10);
	i2cWrite_NT99141(0x338D, 0x23);
	i2cWrite_NT99141(0x338E, 0x7F);
	i2cWrite_NT99141(0x338F, 0x14);
	i2cWrite_NT99141(0x3375, 0x0A); 
	i2cWrite_NT99141(0x3376, 0x0C); 
	i2cWrite_NT99141(0x3377, 0x10); 
	i2cWrite_NT99141(0x3378, 0x14);               
	i2cWrite_NT99141(0x3012, 0x02);
	i2cWrite_NT99141(0x3013, 0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF, 0x60); 
    i2cWrite_NT99141(0x32C0, 0x5A); 
    i2cWrite_NT99141(0x32C1, 0x5A); 
    i2cWrite_NT99141(0x32C2, 0x5A); 
    i2cWrite_NT99141(0x32C3, 0x00); 
    i2cWrite_NT99141(0x32C4, 0x28); 
    i2cWrite_NT99141(0x32C5, 0x20); 
    i2cWrite_NT99141(0x32C6, 0x20); 
    i2cWrite_NT99141(0x32C7, 0x00); 
    i2cWrite_NT99141(0x32C8, 0xE0); 
    i2cWrite_NT99141(0x32C9, 0x5A); 
    i2cWrite_NT99141(0x32CA, 0x7A); 
    i2cWrite_NT99141(0x32CB, 0x7A); 
    i2cWrite_NT99141(0x32CC, 0x7A); 
    i2cWrite_NT99141(0x32CD, 0x7A); 
    i2cWrite_NT99141(0x32DB, 0x7B); 
    i2cWrite_NT99141(0x32E0, 0x02); 
    i2cWrite_NT99141(0x32E1, 0x80); 
    i2cWrite_NT99141(0x32E2, 0x01); 
    i2cWrite_NT99141(0x32E3, 0xE0); 
    i2cWrite_NT99141(0x32E4, 0x00); 
    i2cWrite_NT99141(0x32E5, 0x80); 
    i2cWrite_NT99141(0x32E6, 0x00); 
    i2cWrite_NT99141(0x32E7, 0x80); 
    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x09); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0xA4); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x04); 
    i2cWrite_NT99141(0x3007,0x63); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x05); 
    i2cWrite_NT99141(0x300B,0x3C); 
    i2cWrite_NT99141(0x300C,0x02); 
    i2cWrite_NT99141(0x300D,0xEA); 
    i2cWrite_NT99141(0x300E,0x03); 
    i2cWrite_NT99141(0x300F,0xC0); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x7F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01); 
    i2cWrite_NT99141(0x3069,0x00); 
    i2cWrite_NT99141(0x306a,0x00); 

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

#else //use old parameter in 2014

	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x03);
	i2cWrite_NT99141(0x3252,0xFF);
	i2cWrite_NT99141(0x3253,0x00);
	i2cWrite_NT99141(0x3254,0x03);
	i2cWrite_NT99141(0x3255,0xFF);
	i2cWrite_NT99141(0x3256,0x00);
	i2cWrite_NT99141(0x3257,0x50);
	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270,0x00);
	i2cWrite_NT99141(0x3271,0x0C);
	i2cWrite_NT99141(0x3272,0x18);
	i2cWrite_NT99141(0x3273,0x32);
	i2cWrite_NT99141(0x3274,0x44);
	i2cWrite_NT99141(0x3275,0x54);
	i2cWrite_NT99141(0x3276,0x70);
	i2cWrite_NT99141(0x3277,0x88);
	i2cWrite_NT99141(0x3278,0x9D);
	i2cWrite_NT99141(0x3279,0xB0);
	i2cWrite_NT99141(0x327A,0xCF);
	i2cWrite_NT99141(0x327B,0xE2);
	i2cWrite_NT99141(0x327C,0xEF);
	i2cWrite_NT99141(0x327D,0xF7);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x30);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0xAC);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x23);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xC0);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xD6);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x6A);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x61);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xEA);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xB5);	
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x32F6,0x0F);
	i2cWrite_NT99141(0x32F3,0xB0);
	i2cWrite_NT99141(0x32F9,0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xFF);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x30);
	i2cWrite_NT99141(0x3369,0x28);
	i2cWrite_NT99141(0x336A,0x20);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x5A); 
    i2cWrite_NT99141(0x32C1,0x5A); 
    i2cWrite_NT99141(0x32C2,0x5A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0xE0); 
    i2cWrite_NT99141(0x32C9,0x5A); 
    i2cWrite_NT99141(0x32CA,0x7A); 
    i2cWrite_NT99141(0x32CB,0x7A); 
    i2cWrite_NT99141(0x32CC,0x7A); 
    i2cWrite_NT99141(0x32CD,0x7A); 
    i2cWrite_NT99141(0x32DB,0x7B); 
    i2cWrite_NT99141(0x32E0,0x02); 
    i2cWrite_NT99141(0x32E1,0x80); 
    i2cWrite_NT99141(0x32E2,0x01); 
    i2cWrite_NT99141(0x32E3,0xE0); 
    i2cWrite_NT99141(0x32E4,0x00); 
    i2cWrite_NT99141(0x32E5,0x80); 
    i2cWrite_NT99141(0x32E6,0x00); 
    i2cWrite_NT99141(0x32E7,0x80); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x09); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0xA4); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x04); 
    i2cWrite_NT99141(0x3007,0x63); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x05); 
    i2cWrite_NT99141(0x300B,0x3C); 
    i2cWrite_NT99141(0x300C,0x02); 
    i2cWrite_NT99141(0x300D,0xEA); 
    i2cWrite_NT99141(0x300E,0x03); 
    i2cWrite_NT99141(0x300F,0xC0); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x7F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01); 

    i2cWrite_NT99141(0x3068,0x01);
    i2cWrite_NT99141(0x3069,0x01); 
    i2cWrite_NT99141(0x306a,0x01); 

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
#endif // end of SENSOR_PARAMTER_VERSION
}

void SetNT99141_720P_15FPS_TRANWO(void)
{
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
#if(SENSOR_PARAMETER_VERSION == 1) //use new parameter
    #if 0
    // 151103 v4
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);		
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x01);
	i2cWrite_NT99141(0x3252,0x4B);
	i2cWrite_NT99141(0x3253,0xA0);
	i2cWrite_NT99141(0x3254,0x00);
	i2cWrite_NT99141(0x3255,0xf0);
	i2cWrite_NT99141(0x3256,0x80);
	i2cWrite_NT99141(0x3257,0x40);
	i2cWrite_NT99141(0x329B,0x01);
	i2cWrite_NT99141(0x32A1,0x01);
	i2cWrite_NT99141(0x32A2,0x50);
	i2cWrite_NT99141(0x32A3,0x01);
	i2cWrite_NT99141(0x32A4,0xC0);
	i2cWrite_NT99141(0x32A5,0x01);
	i2cWrite_NT99141(0x32A6,0x10);
	i2cWrite_NT99141(0x32A7,0x02);
	i2cWrite_NT99141(0x32A8,0x30);
	i2cWrite_NT99141(0x32A9,0x11);
	i2cWrite_NT99141(0x32Aa,0x00);
	i2cWrite_NT99141(0x32B0,0x55);
	i2cWrite_NT99141(0x32B1,0x69);
	i2cWrite_NT99141(0x32B2,0x69);
	i2cWrite_NT99141(0x32B3,0x55);
	i2cWrite_NT99141(0x3210,0x11);
	i2cWrite_NT99141(0x3211,0x11);
	i2cWrite_NT99141(0x3212,0x11);
	i2cWrite_NT99141(0x3213,0x11);
	i2cWrite_NT99141(0x3214,0x10);
	i2cWrite_NT99141(0x3215,0x10);
	i2cWrite_NT99141(0x3216,0x10);
	i2cWrite_NT99141(0x3217,0x10);
	i2cWrite_NT99141(0x3218,0x10);
	i2cWrite_NT99141(0x3219,0x10);
	i2cWrite_NT99141(0x321A,0x10);
	i2cWrite_NT99141(0x321B,0x10);
	i2cWrite_NT99141(0x321C,0x0f);
	i2cWrite_NT99141(0x321D,0x0f);
	i2cWrite_NT99141(0x321E,0x0f);
	i2cWrite_NT99141(0x321F,0x0f);
	i2cWrite_NT99141(0x3231,0x68);
	i2cWrite_NT99141(0x3232,0xC4);
	i2cWrite_NT99141(0x3270,0x06);
	i2cWrite_NT99141(0x3271,0x12);
	i2cWrite_NT99141(0x3272,0x1e);
	i2cWrite_NT99141(0x3273,0x34);
	i2cWrite_NT99141(0x3274,0x48);
	i2cWrite_NT99141(0x3275,0x59);
	i2cWrite_NT99141(0x3276,0x74);
	i2cWrite_NT99141(0x3277,0x87);
	i2cWrite_NT99141(0x3278,0x98);
	i2cWrite_NT99141(0x3279,0xa5);
	i2cWrite_NT99141(0x327A,0xbc);
	i2cWrite_NT99141(0x327B,0xd0);
	i2cWrite_NT99141(0x327C,0xE4);
	i2cWrite_NT99141(0x327D,0xF5);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x5c);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0x6c);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x39);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xbe);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xf9);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x46);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x0d);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xfd);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xfc);
	i2cWrite_NT99141(0x3060,0x01);
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x3327,0x0A);
	i2cWrite_NT99141(0x3328,0x0A);
	i2cWrite_NT99141(0x3329,0x06);
	i2cWrite_NT99141(0x332A,0x06);
	i2cWrite_NT99141(0x332B,0x1C);
	i2cWrite_NT99141(0x332C,0x1C);
	i2cWrite_NT99141(0x332D,0x00);
	i2cWrite_NT99141(0x332E,0x1D);
	i2cWrite_NT99141(0x332F,0x1F);
	i2cWrite_NT99141(0x32F6,0xCF);
	i2cWrite_NT99141(0x32F9,0x21);
	i2cWrite_NT99141(0x32FA,0x12);
	i2cWrite_NT99141(0x3325,0x5F);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x04);
	i2cWrite_NT99141(0x3332,0xdc);
	i2cWrite_NT99141(0x3338,0x08);
	i2cWrite_NT99141(0x3339,0x63);
	i2cWrite_NT99141(0x333A,0x36);
	i2cWrite_NT99141(0x333F,0x07);
	i2cWrite_NT99141(0x3360,0x0a);
	i2cWrite_NT99141(0x3361,0x14);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x70);
	i2cWrite_NT99141(0x3365,0x60);
	i2cWrite_NT99141(0x3366,0x50);
	i2cWrite_NT99141(0x3367,0x48);
	i2cWrite_NT99141(0x3368,0xa0);
	i2cWrite_NT99141(0x3369,0x90);
	i2cWrite_NT99141(0x336A,0x70);
	i2cWrite_NT99141(0x336B,0x60);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x14);
	i2cWrite_NT99141(0x3371,0x20);
	i2cWrite_NT99141(0x3372,0x24);
	i2cWrite_NT99141(0x3373,0x30);
	i2cWrite_NT99141(0x3374,0x38);
	i2cWrite_NT99141(0x3375,0x06);
	i2cWrite_NT99141(0x3376,0x06);
	i2cWrite_NT99141(0x3377,0x08);
	i2cWrite_NT99141(0x3378,0x0B);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3069,0x02);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3053,0x4f);
	i2cWrite_NT99141(0x32F2,0x88);
	i2cWrite_NT99141(0x32Fc,0x00);
	i2cWrite_NT99141(0x3060,0x01);             
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x28); 
    i2cWrite_NT99141(0x32C6,0x28); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x48); 
    i2cWrite_NT99141(0x32B9,0x38); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x40); 
    i2cWrite_NT99141(0x32BD,0x44); 
    i2cWrite_NT99141(0x32BE,0x3C); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
   	i2cWrite_NT99141(0x3060,0x01);
    #elif 1
	i2cWrite_NT99141(0x3109, 0x04);
	i2cWrite_NT99141(0x3040, 0x04);
	i2cWrite_NT99141(0x3041, 0x02);
	i2cWrite_NT99141(0x3042, 0xFF);
	i2cWrite_NT99141(0x3043, 0x08);
	i2cWrite_NT99141(0x3052, 0xE0);
	i2cWrite_NT99141(0x305F, 0x33);
	i2cWrite_NT99141(0x3100, 0x07);
	i2cWrite_NT99141(0x3106, 0x03);
	i2cWrite_NT99141(0x3105, 0x01);
	i2cWrite_NT99141(0x3108, 0x05);
	i2cWrite_NT99141(0x3110, 0x22);
	i2cWrite_NT99141(0x3111, 0x57);
	i2cWrite_NT99141(0x3112, 0x22);
	i2cWrite_NT99141(0x3113, 0x55);
	i2cWrite_NT99141(0x3114, 0x05);
	i2cWrite_NT99141(0x3135, 0x00);
	i2cWrite_NT99141(0x32F0, 0x01);
	i2cWrite_NT99141(0x3290, 0x01);
	i2cWrite_NT99141(0x3291, 0x80);
	i2cWrite_NT99141(0x3296, 0x01);
	i2cWrite_NT99141(0x3297, 0x73);
	i2cWrite_NT99141(0x3250, 0xc0);
	i2cWrite_NT99141(0x3251, 0x00);
	i2cWrite_NT99141(0x3252, 0xE3);
	i2cWrite_NT99141(0x3253, 0x8C);
	i2cWrite_NT99141(0x3254, 0x00);
	i2cWrite_NT99141(0x3255, 0xF0);
	i2cWrite_NT99141(0x3256, 0x7C);
	i2cWrite_NT99141(0x3257, 0x38);
	i2cWrite_NT99141(0x329b, 0x01);
	i2cWrite_NT99141(0x32a1, 0x00);
	i2cWrite_NT99141(0x32a2, 0xa0);
	i2cWrite_NT99141(0x32a3, 0x01);
	i2cWrite_NT99141(0x32a4, 0xA0);
	i2cWrite_NT99141(0x32a5, 0x01);
	i2cWrite_NT99141(0x32a6, 0x18);
	i2cWrite_NT99141(0x32a7, 0x01);
	i2cWrite_NT99141(0x32a8, 0xe0);

	i2cWrite_NT99141(0x3210, 0x11); 
	i2cWrite_NT99141(0x3211, 0x14); 
	i2cWrite_NT99141(0x3212, 0x11); 
	i2cWrite_NT99141(0x3213, 0x10); 
	i2cWrite_NT99141(0x3214, 0x0F); 
	i2cWrite_NT99141(0x3215, 0x12); 
	i2cWrite_NT99141(0x3216, 0x10); 
	i2cWrite_NT99141(0x3217, 0x0F); 
	i2cWrite_NT99141(0x3218, 0x0F); 
	i2cWrite_NT99141(0x3219, 0x13); 
	i2cWrite_NT99141(0x321A, 0x10); 
	i2cWrite_NT99141(0x321B, 0x0F); 
	i2cWrite_NT99141(0x321C, 0x0F); 
	i2cWrite_NT99141(0x321D, 0x12); 
	i2cWrite_NT99141(0x321E, 0x0F); 
	i2cWrite_NT99141(0x321F, 0x0D); 
	i2cWrite_NT99141(0x3231, 0x74); 
	i2cWrite_NT99141(0x3232, 0xC4); 
	i2cWrite_NT99141(0x3270, 0x00);
	i2cWrite_NT99141(0x3271, 0x0C);
	i2cWrite_NT99141(0x3272, 0x17);
	i2cWrite_NT99141(0x3273, 0x2E);
	i2cWrite_NT99141(0x3274, 0x46);
	i2cWrite_NT99141(0x3275, 0x5D);
	i2cWrite_NT99141(0x3276, 0x85);
	i2cWrite_NT99141(0x3277, 0xA2);
	i2cWrite_NT99141(0x3278, 0xBA);
	i2cWrite_NT99141(0x3279, 0xCD);
	i2cWrite_NT99141(0x327a, 0xE9);
	i2cWrite_NT99141(0x327b, 0xf6);
	i2cWrite_NT99141(0x327c, 0xfB);
	i2cWrite_NT99141(0x327d, 0xfe);
	i2cWrite_NT99141(0x327e, 0xff);
	i2cWrite_NT99141(0x3302, 0x00);
	i2cWrite_NT99141(0x3303, 0x07); 
	i2cWrite_NT99141(0x3304, 0x00);
	i2cWrite_NT99141(0x3305, 0xcb);
	i2cWrite_NT99141(0x3306, 0x00);
	i2cWrite_NT99141(0x3307, 0x2c);
	i2cWrite_NT99141(0x3308, 0x07);
	i2cWrite_NT99141(0x3309, 0xd7);
	i2cWrite_NT99141(0x330a, 0x06);
	i2cWrite_NT99141(0x330b, 0xc5);
	i2cWrite_NT99141(0x330c, 0x01);
	i2cWrite_NT99141(0x330d, 0x65);
	i2cWrite_NT99141(0x330e, 0x01);
	i2cWrite_NT99141(0x330f, 0x1e);
	i2cWrite_NT99141(0x3310, 0x07);
	i2cWrite_NT99141(0x3311, 0x1d);
	i2cWrite_NT99141(0x3312, 0x07);
	i2cWrite_NT99141(0x3313, 0xc6);
	i2cWrite_NT99141(0x3326, 0x03);
	i2cWrite_NT99141(0x32F6, 0x0F);
#if(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
        i2cWrite_NT99141(0x32F3, 0x98);
#else
        i2cWrite_NT99141(0x32F3, 0xB0);
#endif
	i2cWrite_NT99141(0x32F9, 0x42);
	i2cWrite_NT99141(0x32FA, 0x24);
	i2cWrite_NT99141(0x3325, 0x4A);
	i2cWrite_NT99141(0x3330, 0x00);
	i2cWrite_NT99141(0x3331, 0x0A);
	i2cWrite_NT99141(0x3332, 0xA0);
	i2cWrite_NT99141(0x3338, 0x30);
	i2cWrite_NT99141(0x3339, 0x84);
	i2cWrite_NT99141(0x333A, 0x48);
	i2cWrite_NT99141(0x333F, 0x07);                
	i2cWrite_NT99141(0x3360, 0x10);
	i2cWrite_NT99141(0x3361, 0x18);
	i2cWrite_NT99141(0x3362, 0x1f);
	i2cWrite_NT99141(0x3363, 0x37);
	i2cWrite_NT99141(0x3364, 0x80);
	i2cWrite_NT99141(0x3365, 0x80);
	i2cWrite_NT99141(0x3366, 0x68);
	i2cWrite_NT99141(0x3367, 0x50);
	i2cWrite_NT99141(0x3368, 0x28);
	i2cWrite_NT99141(0x3369, 0x22);
	i2cWrite_NT99141(0x336A, 0x1C);
	i2cWrite_NT99141(0x336B, 0x10);
	i2cWrite_NT99141(0x336C, 0x00);
	i2cWrite_NT99141(0x336D, 0x20);
	i2cWrite_NT99141(0x336E, 0x1C);
	i2cWrite_NT99141(0x336F, 0x18);
	i2cWrite_NT99141(0x3370, 0x10);
	i2cWrite_NT99141(0x3371, 0x38);
	i2cWrite_NT99141(0x3372, 0x3C);
	i2cWrite_NT99141(0x3373, 0x3F);
	i2cWrite_NT99141(0x3374, 0x3F);
	i2cWrite_NT99141(0x338A, 0x34);
	i2cWrite_NT99141(0x338B, 0x7F);
	i2cWrite_NT99141(0x338C, 0x10);
	i2cWrite_NT99141(0x338D, 0x23);
	i2cWrite_NT99141(0x338E, 0x7F);
	i2cWrite_NT99141(0x338F, 0x14);
	i2cWrite_NT99141(0x3375, 0x0A); 
	i2cWrite_NT99141(0x3376, 0x0C); 
	i2cWrite_NT99141(0x3377, 0x10); 
	i2cWrite_NT99141(0x3378, 0x14);               
	i2cWrite_NT99141(0x3012, 0x02);
	i2cWrite_NT99141(0x3013, 0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF, 0x60); 
    i2cWrite_NT99141(0x32C0, 0x6A); 
    i2cWrite_NT99141(0x32C1, 0x6A); 
    i2cWrite_NT99141(0x32C2, 0x6A); 
    i2cWrite_NT99141(0x32C3, 0x00); 
    i2cWrite_NT99141(0x32C4, 0x28); 
    i2cWrite_NT99141(0x32C5, 0x20); 
    i2cWrite_NT99141(0x32C6, 0x20); 
    i2cWrite_NT99141(0x32C7, 0x00); 
    i2cWrite_NT99141(0x32C8, 0x91); 
    i2cWrite_NT99141(0x32C9, 0x6A); 
    i2cWrite_NT99141(0x32CA, 0x8A); 
    i2cWrite_NT99141(0x32CB, 0x8A); 
    i2cWrite_NT99141(0x32CC, 0x8A); 
    i2cWrite_NT99141(0x32CD, 0x8A); 
    i2cWrite_NT99141(0x32DB, 0x72); 
    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x07); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023, 0x24); 
    i2cWrite_NT99141(0x3002, 0x00); 
    i2cWrite_NT99141(0x3003, 0x04); 
    i2cWrite_NT99141(0x3004, 0x00); 
    i2cWrite_NT99141(0x3005, 0x04); 
    i2cWrite_NT99141(0x3006, 0x05); 
    i2cWrite_NT99141(0x3007, 0x03); 
    i2cWrite_NT99141(0x3008, 0x02); 
    i2cWrite_NT99141(0x3009, 0xD3); 
    i2cWrite_NT99141(0x300A, 0x06); 
    i2cWrite_NT99141(0x300B, 0x7C); 
    i2cWrite_NT99141(0x300C, 0x03); 
    i2cWrite_NT99141(0x300D, 0xC3); 
    i2cWrite_NT99141(0x300E, 0x05); 
    i2cWrite_NT99141(0x300F, 0x00); 
    i2cWrite_NT99141(0x3010, 0x02); 
    i2cWrite_NT99141(0x3011, 0xD0); 
    i2cWrite_NT99141(0x32B8, 0x3F); 
    i2cWrite_NT99141(0x32B9, 0x31); 
    i2cWrite_NT99141(0x32BB, 0x87); 
    i2cWrite_NT99141(0x32BC, 0x38); 
    i2cWrite_NT99141(0x32BD, 0x3C); 
    i2cWrite_NT99141(0x32BE, 0x34); 
    i2cWrite_NT99141(0x3201, 0x3F); 
    i2cWrite_NT99141(0x3021, 0x06); 
    i2cWrite_NT99141(0x3060, 0x01);
    i2cWrite_NT99141(0x3069, 0x00); 
    i2cWrite_NT99141(0x306a, 0x00);
    #endif
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

    /* brightness */
    i2cWrite_NT99141(0x32F2 ,AETargetMeanTab[siuY_TargetIndex]);

#else //use old parameter in 2014

	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x03);
	i2cWrite_NT99141(0x3252,0xFF);
	i2cWrite_NT99141(0x3253,0x00);
	i2cWrite_NT99141(0x3254,0x03);
	i2cWrite_NT99141(0x3255,0xFF);
	i2cWrite_NT99141(0x3256,0x00);
	i2cWrite_NT99141(0x3257,0x50);
	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270,0x00);
	i2cWrite_NT99141(0x3271,0x0C);
	i2cWrite_NT99141(0x3272,0x18);
	i2cWrite_NT99141(0x3273,0x32);
	i2cWrite_NT99141(0x3274,0x44);
	i2cWrite_NT99141(0x3275,0x54);
	i2cWrite_NT99141(0x3276,0x70);
	i2cWrite_NT99141(0x3277,0x88);
	i2cWrite_NT99141(0x3278,0x9D);
	i2cWrite_NT99141(0x3279,0xB0);
	i2cWrite_NT99141(0x327A,0xCF);
	i2cWrite_NT99141(0x327B,0xE2);
	i2cWrite_NT99141(0x327C,0xEF);
	i2cWrite_NT99141(0x327D,0xF7);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x30);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0xAC);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x23);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xC0);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xD6);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x6A);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x61);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xEA);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xB5);	
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x32F6,0x0F);
	i2cWrite_NT99141(0x32F3,0xB0);
	i2cWrite_NT99141(0x32F9,0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xFF);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x30);
	i2cWrite_NT99141(0x3369,0x28);
	i2cWrite_NT99141(0x336A,0x20);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);

    i2cWrite_NT99141(0x3068,0x01);
    i2cWrite_NT99141(0x3069,0x01); 
    i2cWrite_NT99141(0x306a,0x01);
    
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
#endif //end of SENSOR_PARAMETER_VERSION
}








#else //SENSOR_PARAMETER == 1
void SetNT99141_VGA_30FPS_TRANWO(void)
{
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250, 0xc0);
	i2cWrite_NT99141(0x3251, 0x00);
	i2cWrite_NT99141(0x3252, 0xdf);
	i2cWrite_NT99141(0x3253, 0x85);
	i2cWrite_NT99141(0x3254, 0x00);
	i2cWrite_NT99141(0x3255, 0xeb);
	i2cWrite_NT99141(0x3256, 0x81);
	i2cWrite_NT99141(0x3257, 0x40);
	i2cWrite_NT99141(0x329b, 0x01);
	i2cWrite_NT99141(0x32a1, 0x00);
	i2cWrite_NT99141(0x32a2, 0xa0);
	i2cWrite_NT99141(0x32a3, 0x01);
	i2cWrite_NT99141(0x32a4, 0x50);
	i2cWrite_NT99141(0x32a5, 0x01);
	i2cWrite_NT99141(0x32a6, 0x18);
	i2cWrite_NT99141(0x32a7, 0x01);
	i2cWrite_NT99141(0x32a8, 0xe0);

	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270, 0x00);
	i2cWrite_NT99141(0x3271, 0x0b);
	i2cWrite_NT99141(0x3272, 0x16);
	i2cWrite_NT99141(0x3273, 0x2b);
	i2cWrite_NT99141(0x3274, 0x3f);
	i2cWrite_NT99141(0x3275, 0x51);
	i2cWrite_NT99141(0x3276, 0x72);
	i2cWrite_NT99141(0x3277, 0x8f);
	i2cWrite_NT99141(0x3278, 0xa7);
	i2cWrite_NT99141(0x3279, 0xbc);
	i2cWrite_NT99141(0x327a, 0xdc);
	i2cWrite_NT99141(0x327b, 0xf0);
	i2cWrite_NT99141(0x327c, 0xfa);
	i2cWrite_NT99141(0x327d, 0xfe);
	i2cWrite_NT99141(0x327e, 0xff);
	i2cWrite_NT99141(0x3302, 0x00);
	i2cWrite_NT99141(0x3303, 0x07); 
	i2cWrite_NT99141(0x3304, 0x00);
	i2cWrite_NT99141(0x3305, 0xcb);
	i2cWrite_NT99141(0x3306, 0x00);
	i2cWrite_NT99141(0x3307, 0x2c);
	i2cWrite_NT99141(0x3308, 0x07);
	i2cWrite_NT99141(0x3309, 0xd7);
	i2cWrite_NT99141(0x330a, 0x06);
	i2cWrite_NT99141(0x330b, 0xc5);
	i2cWrite_NT99141(0x330c, 0x01);
	i2cWrite_NT99141(0x330d, 0x65);
	i2cWrite_NT99141(0x330e, 0x01);
	i2cWrite_NT99141(0x330f, 0x1e);
	i2cWrite_NT99141(0x3310, 0x07);
	i2cWrite_NT99141(0x3311, 0x1d);
	i2cWrite_NT99141(0x3312, 0x07);
	i2cWrite_NT99141(0x3313, 0xc6);
	i2cWrite_NT99141(0x3326, 0x03);
	i2cWrite_NT99141(0x32F6, 0x0F);
	i2cWrite_NT99141(0x32F9, 0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xb8);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x28);
	i2cWrite_NT99141(0x3369,0x22);
	i2cWrite_NT99141(0x336A,0x1c);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x32f1,0x05);
	i2cWrite_NT99141(0x32F3,0xD0);	
	i2cWrite_NT99141(0x32f5,0x7C);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x5A); 
    i2cWrite_NT99141(0x32C1,0x5A); 
    i2cWrite_NT99141(0x32C2,0x5A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0xE0); 
    i2cWrite_NT99141(0x32C9,0x5A); 
    i2cWrite_NT99141(0x32CA,0x7A); 
    i2cWrite_NT99141(0x32CB,0x7A); 
    i2cWrite_NT99141(0x32CC,0x7A); 
    i2cWrite_NT99141(0x32CD,0x7A); 
    i2cWrite_NT99141(0x32DB,0x7B); 
    i2cWrite_NT99141(0x32E0,0x02); 
    i2cWrite_NT99141(0x32E1,0x80); 
    i2cWrite_NT99141(0x32E2,0x01); 
    i2cWrite_NT99141(0x32E3,0xE0); 
    i2cWrite_NT99141(0x32E4,0x00); 
    i2cWrite_NT99141(0x32E5,0x80); 
    i2cWrite_NT99141(0x32E6,0x00); 
    i2cWrite_NT99141(0x32E7,0x80); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x09); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0xA4); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x04); 
    i2cWrite_NT99141(0x3007,0x63); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x05); 
    i2cWrite_NT99141(0x300B,0x3C); 
    i2cWrite_NT99141(0x300C,0x02); 
    i2cWrite_NT99141(0x300D,0xEA); 
    i2cWrite_NT99141(0x300E,0x03); 
    i2cWrite_NT99141(0x300F,0xC0); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x7F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x320a,0x73);	
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03); 

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
}

void SetNT99141_720P_15FPS_TRANWO(void)
{
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    
#if 1
    // 151103 v4
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);		
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x01);
	i2cWrite_NT99141(0x3252,0x4B);
	i2cWrite_NT99141(0x3253,0xA0);
	i2cWrite_NT99141(0x3254,0x00);
	i2cWrite_NT99141(0x3255,0xf0);
	i2cWrite_NT99141(0x3256,0x80);
	i2cWrite_NT99141(0x3257,0x40);
	i2cWrite_NT99141(0x329B,0x01);
	i2cWrite_NT99141(0x32A1,0x01);
	i2cWrite_NT99141(0x32A2,0x50);
	i2cWrite_NT99141(0x32A3,0x01);
	i2cWrite_NT99141(0x32A4,0xC0);
	i2cWrite_NT99141(0x32A5,0x01);
	i2cWrite_NT99141(0x32A6,0x10);
	i2cWrite_NT99141(0x32A7,0x02);
	i2cWrite_NT99141(0x32A8,0x30);
	i2cWrite_NT99141(0x32A9,0x11);
	i2cWrite_NT99141(0x32Aa,0x00);
	i2cWrite_NT99141(0x32B0,0x55);
	i2cWrite_NT99141(0x32B1,0x69);
	i2cWrite_NT99141(0x32B2,0x69);
	i2cWrite_NT99141(0x32B3,0x55);
	i2cWrite_NT99141(0x3210,0x11);
	i2cWrite_NT99141(0x3211,0x11);
	i2cWrite_NT99141(0x3212,0x11);
	i2cWrite_NT99141(0x3213,0x11);
	i2cWrite_NT99141(0x3214,0x10);
	i2cWrite_NT99141(0x3215,0x10);
	i2cWrite_NT99141(0x3216,0x10);
	i2cWrite_NT99141(0x3217,0x10);
	i2cWrite_NT99141(0x3218,0x10);
	i2cWrite_NT99141(0x3219,0x10);
	i2cWrite_NT99141(0x321A,0x10);
	i2cWrite_NT99141(0x321B,0x10);
	i2cWrite_NT99141(0x321C,0x0f);
	i2cWrite_NT99141(0x321D,0x0f);
	i2cWrite_NT99141(0x321E,0x0f);
	i2cWrite_NT99141(0x321F,0x0f);
	i2cWrite_NT99141(0x3231,0x68);
	i2cWrite_NT99141(0x3232,0xC4);
	i2cWrite_NT99141(0x3270,0x06);
	i2cWrite_NT99141(0x3271,0x12);
	i2cWrite_NT99141(0x3272,0x1e);
	i2cWrite_NT99141(0x3273,0x34);
	i2cWrite_NT99141(0x3274,0x48);
	i2cWrite_NT99141(0x3275,0x59);
	i2cWrite_NT99141(0x3276,0x74);
	i2cWrite_NT99141(0x3277,0x87);
	i2cWrite_NT99141(0x3278,0x98);
	i2cWrite_NT99141(0x3279,0xa5);
	i2cWrite_NT99141(0x327A,0xbc);
	i2cWrite_NT99141(0x327B,0xd0);
	i2cWrite_NT99141(0x327C,0xE4);
	i2cWrite_NT99141(0x327D,0xF5);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x5c);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0x6c);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x39);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xbe);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xf9);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x46);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x0d);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xfd);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xfc);
	i2cWrite_NT99141(0x3060,0x01);
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x3327,0x0A);
	i2cWrite_NT99141(0x3328,0x0A);
	i2cWrite_NT99141(0x3329,0x06);
	i2cWrite_NT99141(0x332A,0x06);
	i2cWrite_NT99141(0x332B,0x1C);
	i2cWrite_NT99141(0x332C,0x1C);
	i2cWrite_NT99141(0x332D,0x00);
	i2cWrite_NT99141(0x332E,0x1D);
	i2cWrite_NT99141(0x332F,0x1F);
	i2cWrite_NT99141(0x32F6,0xCF);
	i2cWrite_NT99141(0x32F9,0x21);
	i2cWrite_NT99141(0x32FA,0x12);
	i2cWrite_NT99141(0x3325,0x5F);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x04);
	i2cWrite_NT99141(0x3332,0xdc);
	i2cWrite_NT99141(0x3338,0x08);
	i2cWrite_NT99141(0x3339,0x63);
	i2cWrite_NT99141(0x333A,0x36);
	i2cWrite_NT99141(0x333F,0x07);
	i2cWrite_NT99141(0x3360,0x0a);
	i2cWrite_NT99141(0x3361,0x14);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x70);
	i2cWrite_NT99141(0x3365,0x60);
	i2cWrite_NT99141(0x3366,0x50);
	i2cWrite_NT99141(0x3367,0x48);
	i2cWrite_NT99141(0x3368,0xa0);
	i2cWrite_NT99141(0x3369,0x90);
	i2cWrite_NT99141(0x336A,0x70);
	i2cWrite_NT99141(0x336B,0x60);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x14);
	i2cWrite_NT99141(0x3371,0x20);
	i2cWrite_NT99141(0x3372,0x24);
	i2cWrite_NT99141(0x3373,0x30);
	i2cWrite_NT99141(0x3374,0x38);
	i2cWrite_NT99141(0x3375,0x06);
	i2cWrite_NT99141(0x3376,0x06);
	i2cWrite_NT99141(0x3377,0x08);
	i2cWrite_NT99141(0x3378,0x0B);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3069,0x02);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3053,0x4f);
	i2cWrite_NT99141(0x32F2,0x88);
	i2cWrite_NT99141(0x32Fc,0x00);
	i2cWrite_NT99141(0x3060,0x01);             
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x28); 
    i2cWrite_NT99141(0x32C6,0x28); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x48); 
    i2cWrite_NT99141(0x32B9,0x38); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x40); 
    i2cWrite_NT99141(0x32BD,0x44); 
    i2cWrite_NT99141(0x32BE,0x3C); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
   	i2cWrite_NT99141(0x3060,0x01);

#elif 0
    // 15103 v3
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);		
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x01);
	i2cWrite_NT99141(0x3252,0x4B);
	i2cWrite_NT99141(0x3253,0xA0);
	i2cWrite_NT99141(0x3254,0x00);
	i2cWrite_NT99141(0x3255,0xf0);
	i2cWrite_NT99141(0x3256,0x80);
	i2cWrite_NT99141(0x3257,0x40);
	i2cWrite_NT99141(0x329B,0x01);
	i2cWrite_NT99141(0x32A1,0x01);
	i2cWrite_NT99141(0x32A2,0x40);
	i2cWrite_NT99141(0x32A3,0x01);
	i2cWrite_NT99141(0x32A4,0xC0);
	i2cWrite_NT99141(0x32A5,0x01);
	i2cWrite_NT99141(0x32A6,0x20);
	i2cWrite_NT99141(0x32A7,0x02);
	i2cWrite_NT99141(0x32A8,0x10);
	i2cWrite_NT99141(0x32A9,0x11);
	i2cWrite_NT99141(0x32Aa,0x00);
	i2cWrite_NT99141(0x32B0,0x55);
	i2cWrite_NT99141(0x32B1,0x69);
	i2cWrite_NT99141(0x32B2,0x69);
	i2cWrite_NT99141(0x32B3,0x55);
	i2cWrite_NT99141(0x3210,0x11);
	i2cWrite_NT99141(0x3211,0x11);
	i2cWrite_NT99141(0x3212,0x11);
	i2cWrite_NT99141(0x3213,0x11);
	i2cWrite_NT99141(0x3214,0x10);
	i2cWrite_NT99141(0x3215,0x10);
	i2cWrite_NT99141(0x3216,0x10);
	i2cWrite_NT99141(0x3217,0x10);
	i2cWrite_NT99141(0x3218,0x10);
	i2cWrite_NT99141(0x3219,0x10);
	i2cWrite_NT99141(0x321A,0x10);
	i2cWrite_NT99141(0x321B,0x10);
	i2cWrite_NT99141(0x321C,0x0f);
	i2cWrite_NT99141(0x321D,0x0f);
	i2cWrite_NT99141(0x321E,0x0f);
	i2cWrite_NT99141(0x321F,0x0f);
	i2cWrite_NT99141(0x3231,0x68);
	i2cWrite_NT99141(0x3232,0xC4);
	i2cWrite_NT99141(0x3270,0x06);
	i2cWrite_NT99141(0x3271,0x12);
	i2cWrite_NT99141(0x3272,0x1e);
	i2cWrite_NT99141(0x3273,0x34);
	i2cWrite_NT99141(0x3274,0x48);
	i2cWrite_NT99141(0x3275,0x59);
	i2cWrite_NT99141(0x3276,0x74);
	i2cWrite_NT99141(0x3277,0x87);
	i2cWrite_NT99141(0x3278,0x98);
	i2cWrite_NT99141(0x3279,0xa5);
	i2cWrite_NT99141(0x327A,0xbc);
	i2cWrite_NT99141(0x327B,0xd0);
	i2cWrite_NT99141(0x327C,0xE4);
	i2cWrite_NT99141(0x327D,0xF5);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x5c);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0x6c);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x39);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xbe);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xf9);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x46);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x0d);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xfd);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xfc);
	i2cWrite_NT99141(0x3060,0x01);
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x3327,0x0A);
	i2cWrite_NT99141(0x3328,0x0A);
	i2cWrite_NT99141(0x3329,0x06);
	i2cWrite_NT99141(0x332A,0x06);
	i2cWrite_NT99141(0x332B,0x1C);
	i2cWrite_NT99141(0x332C,0x1C);
	i2cWrite_NT99141(0x332D,0x00);
	i2cWrite_NT99141(0x332E,0x1D);
	i2cWrite_NT99141(0x332F,0x1F);
	i2cWrite_NT99141(0x32F6,0xCF);
	i2cWrite_NT99141(0x32F9,0x21);
	i2cWrite_NT99141(0x32FA,0x12);
	i2cWrite_NT99141(0x3325,0x5F);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x04);
	i2cWrite_NT99141(0x3332,0xdc);
	i2cWrite_NT99141(0x3338,0x08);
	i2cWrite_NT99141(0x3339,0x63);
	i2cWrite_NT99141(0x333A,0x36);
	i2cWrite_NT99141(0x333F,0x07);
	i2cWrite_NT99141(0x3360,0x0a);
	i2cWrite_NT99141(0x3361,0x14);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x70);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0xa0);
	i2cWrite_NT99141(0x3369,0x90);
	i2cWrite_NT99141(0x336A,0x70);
	i2cWrite_NT99141(0x336B,0x60);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x14);
	i2cWrite_NT99141(0x3371,0x20);
	i2cWrite_NT99141(0x3372,0x24);
	i2cWrite_NT99141(0x3373,0x30);
	i2cWrite_NT99141(0x3374,0x38);
	i2cWrite_NT99141(0x3375,0x06);
	i2cWrite_NT99141(0x3376,0x06);
	i2cWrite_NT99141(0x3377,0x08);
	i2cWrite_NT99141(0x3378,0x0B);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3069,0x02);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3053,0x4f);
	i2cWrite_NT99141(0x32F2,0x88);
	i2cWrite_NT99141(0x32Fc,0x00);
	i2cWrite_NT99141(0x3060,0x01);             
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x28); 
    i2cWrite_NT99141(0x32C6,0x28); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x48); 
    i2cWrite_NT99141(0x32B9,0x38); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x40); 
    i2cWrite_NT99141(0x32BD,0x44); 
    i2cWrite_NT99141(0x32BE,0x3C); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
   	i2cWrite_NT99141(0x3060,0x01);
#elif 0
    // 151103 v2
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);		
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x01);
	i2cWrite_NT99141(0x3252,0x4B);
	i2cWrite_NT99141(0x3253,0xA0);
	i2cWrite_NT99141(0x3254,0x00);
	i2cWrite_NT99141(0x3255,0xf0);
	i2cWrite_NT99141(0x3256,0x80);
	i2cWrite_NT99141(0x3257,0x40);
	i2cWrite_NT99141(0x329B,0x01);
	i2cWrite_NT99141(0x32A1,0x01);
	i2cWrite_NT99141(0x32A2,0x40);
	i2cWrite_NT99141(0x32A3,0x01);
	i2cWrite_NT99141(0x32A4,0xA8);
	i2cWrite_NT99141(0x32A5,0x01);
	i2cWrite_NT99141(0x32A6,0x28);
	i2cWrite_NT99141(0x32A7,0x02);
	i2cWrite_NT99141(0x32A8,0x20);
	i2cWrite_NT99141(0x32A9,0x11);
	i2cWrite_NT99141(0x32Aa,0x00);
	i2cWrite_NT99141(0x32B0,0x55);
	i2cWrite_NT99141(0x32B1,0x69);
	i2cWrite_NT99141(0x32B2,0x69);
	i2cWrite_NT99141(0x32B3,0x55);
	i2cWrite_NT99141(0x3210,0x11);
	i2cWrite_NT99141(0x3211,0x11);
	i2cWrite_NT99141(0x3212,0x11);
	i2cWrite_NT99141(0x3213,0x11);
	i2cWrite_NT99141(0x3214,0x10);
	i2cWrite_NT99141(0x3215,0x10);
	i2cWrite_NT99141(0x3216,0x10);
	i2cWrite_NT99141(0x3217,0x10);
	i2cWrite_NT99141(0x3218,0x10);
	i2cWrite_NT99141(0x3219,0x10);
	i2cWrite_NT99141(0x321A,0x10);
	i2cWrite_NT99141(0x321B,0x10);
	i2cWrite_NT99141(0x321C,0x0f);
	i2cWrite_NT99141(0x321D,0x0f);
	i2cWrite_NT99141(0x321E,0x0f);
	i2cWrite_NT99141(0x321F,0x0f);
	i2cWrite_NT99141(0x3231,0x68);
	i2cWrite_NT99141(0x3232,0xC4);
	i2cWrite_NT99141(0x3270,0x06);
	i2cWrite_NT99141(0x3271,0x12);
	i2cWrite_NT99141(0x3272,0x1e);
	i2cWrite_NT99141(0x3273,0x34);
	i2cWrite_NT99141(0x3274,0x48);
	i2cWrite_NT99141(0x3275,0x59);
	i2cWrite_NT99141(0x3276,0x74);
	i2cWrite_NT99141(0x3277,0x87);
	i2cWrite_NT99141(0x3278,0x98);
	i2cWrite_NT99141(0x3279,0xa5);
	i2cWrite_NT99141(0x327A,0xbc);
	i2cWrite_NT99141(0x327B,0xd0);
	i2cWrite_NT99141(0x327C,0xE4);
	i2cWrite_NT99141(0x327D,0xF5);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x5c);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0x6c);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x39);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xbe);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xf9);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x46);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x0d);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xfd);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xfc);
	i2cWrite_NT99141(0x3060,0x01);
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x3327,0x0A);
	i2cWrite_NT99141(0x3328,0x0A);
	i2cWrite_NT99141(0x3329,0x06);
	i2cWrite_NT99141(0x332A,0x06);
	i2cWrite_NT99141(0x332B,0x1C);
	i2cWrite_NT99141(0x332C,0x1C);
	i2cWrite_NT99141(0x332D,0x00);
	i2cWrite_NT99141(0x332E,0x1D);
	i2cWrite_NT99141(0x332F,0x1F);
	i2cWrite_NT99141(0x32F6,0xCF);
	i2cWrite_NT99141(0x32F9,0x21);
	i2cWrite_NT99141(0x32FA,0x12);
	i2cWrite_NT99141(0x3325,0x5F);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x04);
	i2cWrite_NT99141(0x3332,0xdc);
	i2cWrite_NT99141(0x3338,0x08);
	i2cWrite_NT99141(0x3339,0x63);
	i2cWrite_NT99141(0x333A,0x36);
	i2cWrite_NT99141(0x333F,0x07);
	i2cWrite_NT99141(0x3360,0x0a);
	i2cWrite_NT99141(0x3361,0x14);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x98);
	i2cWrite_NT99141(0x3365,0x88);
	i2cWrite_NT99141(0x3366,0x78);
	i2cWrite_NT99141(0x3367,0x64);
	i2cWrite_NT99141(0x3368,0xa0);
	i2cWrite_NT99141(0x3369,0x90);
	i2cWrite_NT99141(0x336A,0x70);
	i2cWrite_NT99141(0x336B,0x60);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x14);
	i2cWrite_NT99141(0x3371,0x20);
	i2cWrite_NT99141(0x3372,0x24);
	i2cWrite_NT99141(0x3373,0x30);
	i2cWrite_NT99141(0x3374,0x38);
	i2cWrite_NT99141(0x3375,0x06);
	i2cWrite_NT99141(0x3376,0x06);
	i2cWrite_NT99141(0x3377,0x08);
	i2cWrite_NT99141(0x3378,0x0B);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3069,0x02);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3053,0x4f);
	i2cWrite_NT99141(0x32F2,0x88);
	i2cWrite_NT99141(0x32Fc,0x00);
	i2cWrite_NT99141(0x3060,0x01);             
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x28); 
    i2cWrite_NT99141(0x32C6,0x28); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x48); 
    i2cWrite_NT99141(0x32B9,0x38); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x40); 
    i2cWrite_NT99141(0x32BD,0x44); 
    i2cWrite_NT99141(0x32BE,0x3C); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
   	i2cWrite_NT99141(0x3060,0x01);
#elif 0
    // 151103 V1
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);		
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x01);
	i2cWrite_NT99141(0x3252,0x4B);
	i2cWrite_NT99141(0x3253,0xA0);
	i2cWrite_NT99141(0x3254,0x00);
	i2cWrite_NT99141(0x3255,0xf0);
	i2cWrite_NT99141(0x3256,0x80);
	i2cWrite_NT99141(0x3257,0x40);
	i2cWrite_NT99141(0x329B,0x01);
	i2cWrite_NT99141(0x32A1,0x01);
	i2cWrite_NT99141(0x32A2,0x00);
	i2cWrite_NT99141(0x32A3,0x01);
	i2cWrite_NT99141(0x32A4,0xa0);
	i2cWrite_NT99141(0x32A5,0x01);
	i2cWrite_NT99141(0x32A6,0x28);
	i2cWrite_NT99141(0x32A7,0x02);
	i2cWrite_NT99141(0x32A8,0x20);
	i2cWrite_NT99141(0x32A9,0x11);
	i2cWrite_NT99141(0x32Aa,0x00);
	i2cWrite_NT99141(0x32B0,0x55);
	i2cWrite_NT99141(0x32B1,0x69);
	i2cWrite_NT99141(0x32B2,0x69);
	i2cWrite_NT99141(0x32B3,0x55);
	i2cWrite_NT99141(0x3210,0x11);
	i2cWrite_NT99141(0x3211,0x11);
	i2cWrite_NT99141(0x3212,0x11);
	i2cWrite_NT99141(0x3213,0x11);
	i2cWrite_NT99141(0x3214,0x10);
	i2cWrite_NT99141(0x3215,0x10);
	i2cWrite_NT99141(0x3216,0x10);
	i2cWrite_NT99141(0x3217,0x10);
	i2cWrite_NT99141(0x3218,0x10);
	i2cWrite_NT99141(0x3219,0x10);
	i2cWrite_NT99141(0x321A,0x10);
	i2cWrite_NT99141(0x321B,0x10);
	i2cWrite_NT99141(0x321C,0x0f);
	i2cWrite_NT99141(0x321D,0x0f);
	i2cWrite_NT99141(0x321E,0x0f);
	i2cWrite_NT99141(0x321F,0x0f);
	i2cWrite_NT99141(0x3231,0x68);
	i2cWrite_NT99141(0x3232,0xC4);
	i2cWrite_NT99141(0x3270,0x06);
	i2cWrite_NT99141(0x3271,0x12);
	i2cWrite_NT99141(0x3272,0x1e);
	i2cWrite_NT99141(0x3273,0x34);
	i2cWrite_NT99141(0x3274,0x48);
	i2cWrite_NT99141(0x3275,0x59);
	i2cWrite_NT99141(0x3276,0x74);
	i2cWrite_NT99141(0x3277,0x87);
	i2cWrite_NT99141(0x3278,0x98);
	i2cWrite_NT99141(0x3279,0xa5);
	i2cWrite_NT99141(0x327A,0xbc);
	i2cWrite_NT99141(0x327B,0xd0);
	i2cWrite_NT99141(0x327C,0xE4);
	i2cWrite_NT99141(0x327D,0xF5);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x5c);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0x6c);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x39);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xbe);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xf9);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x46);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x0d);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xfd);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xfc);
	i2cWrite_NT99141(0x3060,0x01);
	i2cWrite_NT99141(0x3326,0x03);
	i2cWrite_NT99141(0x3327,0x0A);
	i2cWrite_NT99141(0x3328,0x0A);
	i2cWrite_NT99141(0x3329,0x06);
	i2cWrite_NT99141(0x332A,0x06);
	i2cWrite_NT99141(0x332B,0x1C);
	i2cWrite_NT99141(0x332C,0x1C);
	i2cWrite_NT99141(0x332D,0x00);
	i2cWrite_NT99141(0x332E,0x1D);
	i2cWrite_NT99141(0x332F,0x1F);
	i2cWrite_NT99141(0x32F6,0xCF);
	i2cWrite_NT99141(0x32F9,0x21);
	i2cWrite_NT99141(0x32FA,0x12);
	i2cWrite_NT99141(0x3325,0x5F);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x04);
	i2cWrite_NT99141(0x3332,0xdc);
	i2cWrite_NT99141(0x3338,0x08);
	i2cWrite_NT99141(0x3339,0x63);
	i2cWrite_NT99141(0x333A,0x36);
	i2cWrite_NT99141(0x333F,0x07);
	i2cWrite_NT99141(0x3360,0x0a);
	i2cWrite_NT99141(0x3361,0x14);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x98);
	i2cWrite_NT99141(0x3365,0x88);
	i2cWrite_NT99141(0x3366,0x78);
	i2cWrite_NT99141(0x3367,0x64);
	i2cWrite_NT99141(0x3368,0xa0);
	i2cWrite_NT99141(0x3369,0x90);
	i2cWrite_NT99141(0x336A,0x70);
	i2cWrite_NT99141(0x336B,0x60);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x14);
	i2cWrite_NT99141(0x3371,0x20);
	i2cWrite_NT99141(0x3372,0x2c);
	i2cWrite_NT99141(0x3373,0x38);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x3375,0x06);
	i2cWrite_NT99141(0x3376,0x06);
	i2cWrite_NT99141(0x3377,0x08);
	i2cWrite_NT99141(0x3378,0x0B);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3069,0x02);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3053,0x4f);
	i2cWrite_NT99141(0x32F2,0x80);
	i2cWrite_NT99141(0x32Fc,0x00);
	i2cWrite_NT99141(0x3060,0x01);             
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x28); 
    i2cWrite_NT99141(0x32C6,0x28); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F);
    i2cWrite_NT99141(0x32B9,0x31);
    i2cWrite_NT99141(0x32BB,0x87);
    i2cWrite_NT99141(0x32BC,0x38);
    i2cWrite_NT99141(0x32BD,0x3C);
    i2cWrite_NT99141(0x32BE,0x34);
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
   	i2cWrite_NT99141(0x3060,0x01);

#elif 0
    // old
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x03);
	i2cWrite_NT99141(0x3252,0xFF);
	i2cWrite_NT99141(0x3253,0x00);
	i2cWrite_NT99141(0x3254,0x03);
	i2cWrite_NT99141(0x3255,0xFF);
	i2cWrite_NT99141(0x3256,0x00);
	i2cWrite_NT99141(0x3257,0x50);
	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270,0x00);
	i2cWrite_NT99141(0x3271,0x0C);
	i2cWrite_NT99141(0x3272,0x18);
	i2cWrite_NT99141(0x3273,0x32);
	i2cWrite_NT99141(0x3274,0x44);
	i2cWrite_NT99141(0x3275,0x54);
	i2cWrite_NT99141(0x3276,0x70);
	i2cWrite_NT99141(0x3277,0x88);
	i2cWrite_NT99141(0x3278,0x9D);
	i2cWrite_NT99141(0x3279,0xB0);
	i2cWrite_NT99141(0x327A,0xCF);
	i2cWrite_NT99141(0x327B,0xE2);
	i2cWrite_NT99141(0x327C,0xEF);
	i2cWrite_NT99141(0x327D,0xF7);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x30);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0xAC);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x23);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xC0);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xD6);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x6A);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x61);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xEA);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xB5);	
	i2cWrite_NT99141(0x3326,0x03);
    i2cWrite_NT99141(0x32F1,0x05);
	i2cWrite_NT99141(0x32F6,0x0F);
	i2cWrite_NT99141(0x32F3,0xD0);
	i2cWrite_NT99141(0x32F9,0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xB8);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x28);
	i2cWrite_NT99141(0x3369,0x22);
	i2cWrite_NT99141(0x336A,0x1C);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);
    
    
    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x28); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    i2cWrite_NT99141(0x320a,0x00);
	i2cWrite_NT99141(0x3060,0x01);
#endif
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

    /* brightness */
    i2cWrite_NT99141(0x32F2 ,AETargetMeanTab[siuY_TargetIndex]);
}
#endif //end of SENSOR_PARAMETER

#elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
void SetNT99141_VGA_30FPS_MAYON(void)
{
#if 0
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11); 
    i2cWrite_NT99141(0x3211,0x14); 
    i2cWrite_NT99141(0x3212,0x11); 
    i2cWrite_NT99141(0x3213,0x10); 
    i2cWrite_NT99141(0x3214,0x0F); 
    i2cWrite_NT99141(0x3215,0x12); 
    i2cWrite_NT99141(0x3216,0x10); 
    i2cWrite_NT99141(0x3217,0x0F); 
    i2cWrite_NT99141(0x3218,0x0F); 
    i2cWrite_NT99141(0x3219,0x13); 
    i2cWrite_NT99141(0x321A,0x10); 
    i2cWrite_NT99141(0x321B,0x0F); 
    i2cWrite_NT99141(0x321C,0x0F); 
    i2cWrite_NT99141(0x321D,0x12); 
    i2cWrite_NT99141(0x321E,0x0F); 
    i2cWrite_NT99141(0x321F,0x0D); 
    i2cWrite_NT99141(0x3231,0x74); 
    i2cWrite_NT99141(0x3232,0xC4); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x30);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);  
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x50);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14);               
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x5A); 
    i2cWrite_NT99141(0x32C1,0x5A); 
    i2cWrite_NT99141(0x32C2,0x5A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0xE0); 
    i2cWrite_NT99141(0x32C9,0x5A); 
    i2cWrite_NT99141(0x32CA,0x7A); 
    i2cWrite_NT99141(0x32CB,0x7A); 
    i2cWrite_NT99141(0x32CC,0x7A); 
    i2cWrite_NT99141(0x32CD,0x7A); 
    i2cWrite_NT99141(0x32DB,0x7B); 
    i2cWrite_NT99141(0x32E0,0x02); 
    i2cWrite_NT99141(0x32E1,0x80); 
    i2cWrite_NT99141(0x32E2,0x01); 
    i2cWrite_NT99141(0x32E3,0xE0); 
    i2cWrite_NT99141(0x32E4,0x00); 
    i2cWrite_NT99141(0x32E5,0x80); 
    i2cWrite_NT99141(0x32E6,0x00); 
    i2cWrite_NT99141(0x32E7,0x80); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x09); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0xA4); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x04); 
    i2cWrite_NT99141(0x3007,0x63); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x05); 
    i2cWrite_NT99141(0x300B,0x3C); 
    i2cWrite_NT99141(0x300C,0x02); 
    i2cWrite_NT99141(0x300D,0xEA); 
    i2cWrite_NT99141(0x300E,0x03); 
    i2cWrite_NT99141(0x300F,0xC0); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x7F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01); 
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03); 

    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

#elif 0 // 20140320

    //----- Initial Setting 20140320 ----//   
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x40); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x04);
    i2cWrite_NT99141(0x3272,0x0E);
    i2cWrite_NT99141(0x3273,0x28);
    i2cWrite_NT99141(0x3274,0x3F);
    i2cWrite_NT99141(0x3275,0x50);
    i2cWrite_NT99141(0x3276,0x6E);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0xA0);
    i2cWrite_NT99141(0x3279,0xB3);
    i2cWrite_NT99141(0x327A,0xD2);
    i2cWrite_NT99141(0x327B,0xE8);
    i2cWrite_NT99141(0x327C,0xF5);
    i2cWrite_NT99141(0x327D,0xFF);
    i2cWrite_NT99141(0x327E,0xFF);               
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0x96);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x1D);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xDE);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x75);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xAE);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x40);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xD7);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xEA);             
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14);                
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306A,0x03);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x32E0,0x02);
    i2cWrite_NT99141(0x32E1,0x80);
    i2cWrite_NT99141(0x32E2,0x01);
    i2cWrite_NT99141(0x32E3,0xE0);
    i2cWrite_NT99141(0x32E4,0x00);
    i2cWrite_NT99141(0x32E5,0x80);
    i2cWrite_NT99141(0x32E6,0x00);
    i2cWrite_NT99141(0x32E7,0x80);
    i2cWrite_NT99141(0x3200,0x3E);
    i2cWrite_NT99141(0x3201,0x0F);
    i2cWrite_NT99141(0x3028,0x09);
    i2cWrite_NT99141(0x3029,0x00);
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24);
    i2cWrite_NT99141(0x3002,0x00);
    i2cWrite_NT99141(0x3003,0xA4);
    i2cWrite_NT99141(0x3004,0x00);
    i2cWrite_NT99141(0x3005,0x04);
    i2cWrite_NT99141(0x3006,0x04);
    i2cWrite_NT99141(0x3007,0x63);
    i2cWrite_NT99141(0x3008,0x02);
    i2cWrite_NT99141(0x3009,0xD3);
    i2cWrite_NT99141(0x300A,0x05);
    i2cWrite_NT99141(0x300B,0x47);
    i2cWrite_NT99141(0x300C,0x02);
    i2cWrite_NT99141(0x300D,0xE4);
    i2cWrite_NT99141(0x300E,0x03);
    i2cWrite_NT99141(0x300F,0xC0);
    i2cWrite_NT99141(0x3010,0x02);
    i2cWrite_NT99141(0x3011,0xD0);
    i2cWrite_NT99141(0x32B8,0x3F);
    i2cWrite_NT99141(0x32B9,0x31);
    i2cWrite_NT99141(0x32BB,0x87);
    i2cWrite_NT99141(0x32BC,0x38);
    i2cWrite_NT99141(0x32BD,0x3C);
    i2cWrite_NT99141(0x32BE,0x34);
    i2cWrite_NT99141(0x3201,0x7F);
    i2cWrite_NT99141(0x3021,0x06);
    i2cWrite_NT99141(0x320A,0x73);
    i2cWrite_NT99141(0x3060,0x01);

#else   // 20140324

    //----- Initial Setting 20140324 ----//
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);
    i2cWrite_NT99141(0x3326,0x01);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x98);
    i2cWrite_NT99141(0x3365,0x98);
    i2cWrite_NT99141(0x3366,0x80);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x48);
    i2cWrite_NT99141(0x3369,0x38);
    i2cWrite_NT99141(0x336A,0x30);
    i2cWrite_NT99141(0x336B,0x20);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A);
    i2cWrite_NT99141(0x3376,0x0C);
    i2cWrite_NT99141(0x3377,0x10);
    i2cWrite_NT99141(0x3378,0x14);
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x32E0, 0x02); 
    i2cWrite_NT99141(0x32E1, 0x80); 
    i2cWrite_NT99141(0x32E2, 0x01); 
    i2cWrite_NT99141(0x32E3, 0xE0); 
    i2cWrite_NT99141(0x32E4, 0x00); 
    i2cWrite_NT99141(0x32E5, 0x80); 
    i2cWrite_NT99141(0x32E6, 0x00); 
    i2cWrite_NT99141(0x32E7, 0x80); 
    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x09); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023, 0x24); 
    i2cWrite_NT99141(0x3002, 0x00); 
    i2cWrite_NT99141(0x3003, 0xA4); 
    i2cWrite_NT99141(0x3004, 0x00); 
    i2cWrite_NT99141(0x3005, 0x04); 
    i2cWrite_NT99141(0x3006, 0x04); 
    i2cWrite_NT99141(0x3007, 0x63); 
    i2cWrite_NT99141(0x3008, 0x02); 
    i2cWrite_NT99141(0x3009, 0xD3); 
    i2cWrite_NT99141(0x300A, 0x05); 
    i2cWrite_NT99141(0x300B, 0x3C); 
#if(FORECE_MPEG_DROP_1_10_FPS==0)
    i2cWrite_NT99141(0x300C, 0x02); 
    i2cWrite_NT99141(0x300D, 0xEA); 
#endif
    i2cWrite_NT99141(0x300E, 0x03); 
    i2cWrite_NT99141(0x300F, 0xC0); 
    i2cWrite_NT99141(0x3010, 0x02); 
    i2cWrite_NT99141(0x3011, 0xD0); 
    i2cWrite_NT99141(0x32B8, 0x3F); 
    i2cWrite_NT99141(0x32B9, 0x31); 
    i2cWrite_NT99141(0x32BB, 0x87); 
    i2cWrite_NT99141(0x32BC, 0x38); 
    i2cWrite_NT99141(0x32BD, 0x3C); 
    i2cWrite_NT99141(0x32BE, 0x34); 
    i2cWrite_NT99141(0x3201, 0x7F); 
    i2cWrite_NT99141(0x3021, 0x06);
    i2cWrite_NT99141(0x320A, 0x73); 
    i2cWrite_NT99141(0x3069, 0x03); 
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3024, 0x08);
    i2cWrite_NT99141(0x3060, 0x01); 


#endif

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

}

void SetNT99141_720P_15FPS_MAYON(void)
{
#if 0
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11); 
    i2cWrite_NT99141(0x3211,0x14); 
    i2cWrite_NT99141(0x3212,0x11); 
    i2cWrite_NT99141(0x3213,0x10); 
    i2cWrite_NT99141(0x3214,0x0F); 
    i2cWrite_NT99141(0x3215,0x12); 
    i2cWrite_NT99141(0x3216,0x10); 
    i2cWrite_NT99141(0x3217,0x0F); 
    i2cWrite_NT99141(0x3218,0x0F); 
    i2cWrite_NT99141(0x3219,0x13); 
    i2cWrite_NT99141(0x321A,0x10); 
    i2cWrite_NT99141(0x321B,0x0F); 
    i2cWrite_NT99141(0x321C,0x0F); 
    i2cWrite_NT99141(0x321D,0x12); 
    i2cWrite_NT99141(0x321E,0x0F); 
    i2cWrite_NT99141(0x321F,0x0D); 
    i2cWrite_NT99141(0x3231,0x74); 
    i2cWrite_NT99141(0x3232,0xC4); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x30);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);  
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x50);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14);               
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

#elif 0 // 20140320

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x40); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x04);
    i2cWrite_NT99141(0x3272,0x0E);
    i2cWrite_NT99141(0x3273,0x28);
    i2cWrite_NT99141(0x3274,0x3F);
    i2cWrite_NT99141(0x3275,0x50);
    i2cWrite_NT99141(0x3276,0x6E);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0xA0);
    i2cWrite_NT99141(0x3279,0xB3);
    i2cWrite_NT99141(0x327A,0xD2);
    i2cWrite_NT99141(0x327B,0xE8);
    i2cWrite_NT99141(0x327C,0xF5);
    i2cWrite_NT99141(0x327D,0xFF);
    i2cWrite_NT99141(0x327E,0xFF);               
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0x96);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x1D);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xDE);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x75);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xAE);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x40);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xD7);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xEA);             
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14); 
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306A,0x03);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x3200,0x3E);
    i2cWrite_NT99141(0x3201,0x0F);
    i2cWrite_NT99141(0x3028,0x07);
    i2cWrite_NT99141(0x3029,0x00);
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24);
    i2cWrite_NT99141(0x3002,0x00);
    i2cWrite_NT99141(0x3003,0x04);
    i2cWrite_NT99141(0x3004,0x00);
    i2cWrite_NT99141(0x3005,0x04);
    i2cWrite_NT99141(0x3006,0x05);
    i2cWrite_NT99141(0x3007,0x03);
    i2cWrite_NT99141(0x3008,0x02);
    i2cWrite_NT99141(0x3009,0xD3);
    i2cWrite_NT99141(0x300A,0x08);
    i2cWrite_NT99141(0x300B,0x39);
    i2cWrite_NT99141(0x300C,0x02);
    i2cWrite_NT99141(0x300D,0xF8);
    i2cWrite_NT99141(0x300E,0x05);
    i2cWrite_NT99141(0x300F,0x00);
    i2cWrite_NT99141(0x3010,0x02);
    i2cWrite_NT99141(0x3011,0xD0);
    i2cWrite_NT99141(0x32B8,0x3F);
    i2cWrite_NT99141(0x32B9,0x31);
    i2cWrite_NT99141(0x32BB,0x87);
    i2cWrite_NT99141(0x32BC,0x38);
    i2cWrite_NT99141(0x32BD,0x3C);
    i2cWrite_NT99141(0x32BE,0x34);
    i2cWrite_NT99141(0x3201,0x3F);
    i2cWrite_NT99141(0x3021,0x06);
    i2cWrite_NT99141(0x320A,0x00);
    i2cWrite_NT99141(0x3060,0x01);

#else   // 20140324

    //----- Initial Setting 20140324 ----//
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);
    i2cWrite_NT99141(0x3326,0x01);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x98);
    i2cWrite_NT99141(0x3365,0x98);
    i2cWrite_NT99141(0x3366,0x80);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x48);
    i2cWrite_NT99141(0x3369,0x38);
    i2cWrite_NT99141(0x336A,0x30);
    i2cWrite_NT99141(0x336B,0x20);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A);
    i2cWrite_NT99141(0x3376,0x0C);
    i2cWrite_NT99141(0x3377,0x10);
    i2cWrite_NT99141(0x3378,0x14);
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x07); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04); 
    i2cWrite_NT99141(0x3022, 0x24); 
    i2cWrite_NT99141(0x3023, 0x24); 
    i2cWrite_NT99141(0x3002, 0x00); 
    i2cWrite_NT99141(0x3003, 0x04); 
    i2cWrite_NT99141(0x3004, 0x00); 
    i2cWrite_NT99141(0x3005, 0x04); 
    i2cWrite_NT99141(0x3006, 0x05); 
    i2cWrite_NT99141(0x3007, 0x03); 
    i2cWrite_NT99141(0x3008, 0x02); 
    i2cWrite_NT99141(0x3009, 0xD3); 
#if( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
    i2cWrite_NT99141(0x300A, 0x0C); 
    i2cWrite_NT99141(0x300B, 0xBC); 
    i2cWrite_NT99141(0x300C, 0x02); 
    i2cWrite_NT99141(0x300D, 0xE0); 
#else
    i2cWrite_NT99141(0x300A, 0x06); 
    i2cWrite_NT99141(0x300B, 0x7C); 
    i2cWrite_NT99141(0x300C, 0x03); 
    i2cWrite_NT99141(0x300D, 0xC3); 
#endif
    i2cWrite_NT99141(0x300E, 0x05); 
    i2cWrite_NT99141(0x300F, 0x00); 
    i2cWrite_NT99141(0x3010, 0x02); 
    i2cWrite_NT99141(0x3011, 0xD0); 
    i2cWrite_NT99141(0x32B8, 0x3F); 
    i2cWrite_NT99141(0x32B9, 0x31); 
    i2cWrite_NT99141(0x32BB, 0x87); 
    i2cWrite_NT99141(0x32BC, 0x38); 
    i2cWrite_NT99141(0x32BD, 0x3C); 
    i2cWrite_NT99141(0x32BE, 0x34); 
    i2cWrite_NT99141(0x3201, 0x3F); 
    i2cWrite_NT99141(0x3021, 0x06);
    i2cWrite_NT99141(0x320A, 0x00); 
    i2cWrite_NT99141(0x3069, 0x03); 
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3024, 0x08);
    i2cWrite_NT99141(0x3060, 0x01); 

#endif

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
}

#elif((HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||  (HW_BOARD_OPTION == MR8120_TX_MA8806))
void SetNT99141_VGA_30FPS_MAYON(void)
{
#if 0
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x03);
	i2cWrite_NT99141(0x3252,0xFF);
	i2cWrite_NT99141(0x3253,0x00);
	i2cWrite_NT99141(0x3254,0x03);
	i2cWrite_NT99141(0x3255,0xFF);
	i2cWrite_NT99141(0x3256,0x00);
	i2cWrite_NT99141(0x3257,0x50);
	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270,0x00);
	i2cWrite_NT99141(0x3271,0x0C);
	i2cWrite_NT99141(0x3272,0x18);
	i2cWrite_NT99141(0x3273,0x32);
	i2cWrite_NT99141(0x3274,0x44);
	i2cWrite_NT99141(0x3275,0x54);
	i2cWrite_NT99141(0x3276,0x70);
	i2cWrite_NT99141(0x3277,0x88);
	i2cWrite_NT99141(0x3278,0x9D);
	i2cWrite_NT99141(0x3279,0xB0);
	i2cWrite_NT99141(0x327A,0xCF);
	i2cWrite_NT99141(0x327B,0xE2);
	i2cWrite_NT99141(0x327C,0xEF);
	i2cWrite_NT99141(0x327D,0xF7);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x30);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0xAC);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x23);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xC0);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xD6);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x6A);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x61);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xEA);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xB5);	
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x32F6,0x0F);
	i2cWrite_NT99141(0x32F3,0xB0);
	i2cWrite_NT99141(0x32F9,0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xFF);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x30);
	i2cWrite_NT99141(0x3369,0x28);
	i2cWrite_NT99141(0x336A,0x20);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x5A); 
    i2cWrite_NT99141(0x32C1,0x5A); 
    i2cWrite_NT99141(0x32C2,0x5A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0xE0); 
    i2cWrite_NT99141(0x32C9,0x5A); 
    i2cWrite_NT99141(0x32CA,0x7A); 
    i2cWrite_NT99141(0x32CB,0x7A); 
    i2cWrite_NT99141(0x32CC,0x7A); 
    i2cWrite_NT99141(0x32CD,0x7A); 
    i2cWrite_NT99141(0x32DB,0x7B); 
    i2cWrite_NT99141(0x32E0,0x02); 
    i2cWrite_NT99141(0x32E1,0x80); 
    i2cWrite_NT99141(0x32E2,0x01); 
    i2cWrite_NT99141(0x32E3,0xE0); 
    i2cWrite_NT99141(0x32E4,0x00); 
    i2cWrite_NT99141(0x32E5,0x80); 
    i2cWrite_NT99141(0x32E6,0x00); 
    i2cWrite_NT99141(0x32E7,0x80); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x09); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0xA4); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x04); 
    i2cWrite_NT99141(0x3007,0x63); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x05); 
    i2cWrite_NT99141(0x300B,0x3C); 
    i2cWrite_NT99141(0x300C,0x02); 
    i2cWrite_NT99141(0x300D,0xEA); 
    i2cWrite_NT99141(0x300E,0x03); 
    i2cWrite_NT99141(0x300F,0xC0); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x7F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01); 
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03); 

    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

#elif 0 // 20140320

    //----- Initial Setting 20140320 ----//   
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x40); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x04);
    i2cWrite_NT99141(0x3272,0x0E);
    i2cWrite_NT99141(0x3273,0x28);
    i2cWrite_NT99141(0x3274,0x3F);
    i2cWrite_NT99141(0x3275,0x50);
    i2cWrite_NT99141(0x3276,0x6E);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0xA0);
    i2cWrite_NT99141(0x3279,0xB3);
    i2cWrite_NT99141(0x327A,0xD2);
    i2cWrite_NT99141(0x327B,0xE8);
    i2cWrite_NT99141(0x327C,0xF5);
    i2cWrite_NT99141(0x327D,0xFF);
    i2cWrite_NT99141(0x327E,0xFF);               
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0x96);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x1D);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xDE);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x75);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xAE);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x40);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xD7);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xEA);             
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14);                
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306A,0x03);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x32E0,0x02);
    i2cWrite_NT99141(0x32E1,0x80);
    i2cWrite_NT99141(0x32E2,0x01);
    i2cWrite_NT99141(0x32E3,0xE0);
    i2cWrite_NT99141(0x32E4,0x00);
    i2cWrite_NT99141(0x32E5,0x80);
    i2cWrite_NT99141(0x32E6,0x00);
    i2cWrite_NT99141(0x32E7,0x80);
    i2cWrite_NT99141(0x3200,0x3E);
    i2cWrite_NT99141(0x3201,0x0F);
    i2cWrite_NT99141(0x3028,0x09);
    i2cWrite_NT99141(0x3029,0x00);
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24);
    i2cWrite_NT99141(0x3002,0x00);
    i2cWrite_NT99141(0x3003,0xA4);
    i2cWrite_NT99141(0x3004,0x00);
    i2cWrite_NT99141(0x3005,0x04);
    i2cWrite_NT99141(0x3006,0x04);
    i2cWrite_NT99141(0x3007,0x63);
    i2cWrite_NT99141(0x3008,0x02);
    i2cWrite_NT99141(0x3009,0xD3);
    i2cWrite_NT99141(0x300A,0x05);
    i2cWrite_NT99141(0x300B,0x47);
    i2cWrite_NT99141(0x300C,0x02);
    i2cWrite_NT99141(0x300D,0xE4);
    i2cWrite_NT99141(0x300E,0x03);
    i2cWrite_NT99141(0x300F,0xC0);
    i2cWrite_NT99141(0x3010,0x02);
    i2cWrite_NT99141(0x3011,0xD0);
    i2cWrite_NT99141(0x32B8,0x3F);
    i2cWrite_NT99141(0x32B9,0x31);
    i2cWrite_NT99141(0x32BB,0x87);
    i2cWrite_NT99141(0x32BC,0x38);
    i2cWrite_NT99141(0x32BD,0x3C);
    i2cWrite_NT99141(0x32BE,0x34);
    i2cWrite_NT99141(0x3201,0x7F);
    i2cWrite_NT99141(0x3021,0x06);
    i2cWrite_NT99141(0x320A,0x73);
    i2cWrite_NT99141(0x3060,0x01);

#else   // 20140324

    //----- Initial Setting 20140324 ----//
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x30);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x50);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A);
    i2cWrite_NT99141(0x3376,0x0C);
    i2cWrite_NT99141(0x3377,0x10);
    i2cWrite_NT99141(0x3378,0x14);
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x32E0, 0x02); 
    i2cWrite_NT99141(0x32E1, 0x80); 
    i2cWrite_NT99141(0x32E2, 0x01); 
    i2cWrite_NT99141(0x32E3, 0xE0); 
    i2cWrite_NT99141(0x32E4, 0x00); 
    i2cWrite_NT99141(0x32E5, 0x80); 
    i2cWrite_NT99141(0x32E6, 0x00); 
    i2cWrite_NT99141(0x32E7, 0x80); 
    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x09); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04); 
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023, 0x24); 
    i2cWrite_NT99141(0x3002, 0x00); 
    i2cWrite_NT99141(0x3003, 0xA4); 
    i2cWrite_NT99141(0x3004, 0x00); 
    i2cWrite_NT99141(0x3005, 0x04); 
    i2cWrite_NT99141(0x3006, 0x04); 
    i2cWrite_NT99141(0x3007, 0x63); 
    i2cWrite_NT99141(0x3008, 0x02); 
    i2cWrite_NT99141(0x3009, 0xD3); 
    i2cWrite_NT99141(0x300A, 0x05); 
    i2cWrite_NT99141(0x300B, 0x3C); 
#if(FORECE_MPEG_DROP_1_10_FPS==0)
    i2cWrite_NT99141(0x300C, 0x02); 
    i2cWrite_NT99141(0x300D, 0xEA); 
#endif
    i2cWrite_NT99141(0x300E, 0x03); 
    i2cWrite_NT99141(0x300F, 0xC0); 
    i2cWrite_NT99141(0x3010, 0x02); 
    i2cWrite_NT99141(0x3011, 0xD0); 
    i2cWrite_NT99141(0x32B8, 0x3F); 
    i2cWrite_NT99141(0x32B9, 0x31); 
    i2cWrite_NT99141(0x32BB, 0x87); 
    i2cWrite_NT99141(0x32BC, 0x38); 
    i2cWrite_NT99141(0x32BD, 0x3C); 
    i2cWrite_NT99141(0x32BE, 0x34); 
    i2cWrite_NT99141(0x3201, 0x7F); 
    i2cWrite_NT99141(0x3021, 0x06);
    i2cWrite_NT99141(0x320A, 0x73); 
    i2cWrite_NT99141(0x3069, 0x03); 
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3024, 0x08);
    i2cWrite_NT99141(0x3060, 0x01); 


#endif

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

}

void SetNT99141_720P_15FPS_MAYON(void)
{
#if 0
    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
	i2cWrite_NT99141(0x3109,0x04);
	i2cWrite_NT99141(0x3040,0x04);
	i2cWrite_NT99141(0x3041,0x02);
	i2cWrite_NT99141(0x3042,0xFF);
	i2cWrite_NT99141(0x3043,0x08);
	i2cWrite_NT99141(0x3052,0xE0);
	i2cWrite_NT99141(0x305F,0x33);
	i2cWrite_NT99141(0x3100,0x07);
	i2cWrite_NT99141(0x3106,0x03);
	i2cWrite_NT99141(0x3105,0x01);
	i2cWrite_NT99141(0x3108,0x05);
	i2cWrite_NT99141(0x3110,0x22);
	i2cWrite_NT99141(0x3111,0x57);
	i2cWrite_NT99141(0x3112,0x22);
	i2cWrite_NT99141(0x3113,0x55);
	i2cWrite_NT99141(0x3114,0x05);
	i2cWrite_NT99141(0x3135,0x00);
	i2cWrite_NT99141(0x32F0,0x01);
	i2cWrite_NT99141(0x3290,0x01);
	i2cWrite_NT99141(0x3291,0x80);
	i2cWrite_NT99141(0x3296,0x01);
	i2cWrite_NT99141(0x3297,0x73);
	i2cWrite_NT99141(0x3250,0x80);
	i2cWrite_NT99141(0x3251,0x03);
	i2cWrite_NT99141(0x3252,0xFF);
	i2cWrite_NT99141(0x3253,0x00);
	i2cWrite_NT99141(0x3254,0x03);
	i2cWrite_NT99141(0x3255,0xFF);
	i2cWrite_NT99141(0x3256,0x00);
	i2cWrite_NT99141(0x3257,0x50);
	i2cWrite_NT99141(0x3210,0x11); 
	i2cWrite_NT99141(0x3211,0x14); 
	i2cWrite_NT99141(0x3212,0x11); 
	i2cWrite_NT99141(0x3213,0x10); 
	i2cWrite_NT99141(0x3214,0x0F); 
	i2cWrite_NT99141(0x3215,0x12); 
	i2cWrite_NT99141(0x3216,0x10); 
	i2cWrite_NT99141(0x3217,0x0F); 
	i2cWrite_NT99141(0x3218,0x0F); 
	i2cWrite_NT99141(0x3219,0x13); 
	i2cWrite_NT99141(0x321A,0x10); 
	i2cWrite_NT99141(0x321B,0x0F); 
	i2cWrite_NT99141(0x321C,0x0F); 
	i2cWrite_NT99141(0x321D,0x12); 
	i2cWrite_NT99141(0x321E,0x0F); 
	i2cWrite_NT99141(0x321F,0x0D); 
	i2cWrite_NT99141(0x3231,0x74); 
	i2cWrite_NT99141(0x3232,0xC4); 
	i2cWrite_NT99141(0x3270,0x00);
	i2cWrite_NT99141(0x3271,0x0C);
	i2cWrite_NT99141(0x3272,0x18);
	i2cWrite_NT99141(0x3273,0x32);
	i2cWrite_NT99141(0x3274,0x44);
	i2cWrite_NT99141(0x3275,0x54);
	i2cWrite_NT99141(0x3276,0x70);
	i2cWrite_NT99141(0x3277,0x88);
	i2cWrite_NT99141(0x3278,0x9D);
	i2cWrite_NT99141(0x3279,0xB0);
	i2cWrite_NT99141(0x327A,0xCF);
	i2cWrite_NT99141(0x327B,0xE2);
	i2cWrite_NT99141(0x327C,0xEF);
	i2cWrite_NT99141(0x327D,0xF7);
	i2cWrite_NT99141(0x327E,0xFF);
	i2cWrite_NT99141(0x3302,0x00);
	i2cWrite_NT99141(0x3303,0x30);
	i2cWrite_NT99141(0x3304,0x00);
	i2cWrite_NT99141(0x3305,0xAC);
	i2cWrite_NT99141(0x3306,0x00);
	i2cWrite_NT99141(0x3307,0x23);
	i2cWrite_NT99141(0x3308,0x07);
	i2cWrite_NT99141(0x3309,0xC0);
	i2cWrite_NT99141(0x330A,0x06);
	i2cWrite_NT99141(0x330B,0xD6);
	i2cWrite_NT99141(0x330C,0x01);
	i2cWrite_NT99141(0x330D,0x6A);
	i2cWrite_NT99141(0x330E,0x01);
	i2cWrite_NT99141(0x330F,0x61);
	i2cWrite_NT99141(0x3310,0x06);
	i2cWrite_NT99141(0x3311,0xEA);
	i2cWrite_NT99141(0x3312,0x07);
	i2cWrite_NT99141(0x3313,0xB5);	
	i2cWrite_NT99141(0x3326,0x02);
	i2cWrite_NT99141(0x32F6,0x0F);
	i2cWrite_NT99141(0x32F3,0xB0);
	i2cWrite_NT99141(0x32F9,0x42);
	i2cWrite_NT99141(0x32FA,0x24);
	i2cWrite_NT99141(0x3325,0x4A);
	i2cWrite_NT99141(0x3330,0x00);
	i2cWrite_NT99141(0x3331,0x0A);
	i2cWrite_NT99141(0x3332,0xFF);
	i2cWrite_NT99141(0x3338,0x30);
	i2cWrite_NT99141(0x3339,0x84);
	i2cWrite_NT99141(0x333A,0x48);
	i2cWrite_NT99141(0x333F,0x07);                
	i2cWrite_NT99141(0x3360,0x10);
	i2cWrite_NT99141(0x3361,0x18);
	i2cWrite_NT99141(0x3362,0x1f);
	i2cWrite_NT99141(0x3363,0x37);
	i2cWrite_NT99141(0x3364,0x80);
	i2cWrite_NT99141(0x3365,0x80);
	i2cWrite_NT99141(0x3366,0x68);
	i2cWrite_NT99141(0x3367,0x50);
	i2cWrite_NT99141(0x3368,0x30);
	i2cWrite_NT99141(0x3369,0x28);
	i2cWrite_NT99141(0x336A,0x20);
	i2cWrite_NT99141(0x336B,0x10);
	i2cWrite_NT99141(0x336C,0x00);
	i2cWrite_NT99141(0x336D,0x20);
	i2cWrite_NT99141(0x336E,0x1C);
	i2cWrite_NT99141(0x336F,0x18);
	i2cWrite_NT99141(0x3370,0x10);
	i2cWrite_NT99141(0x3371,0x38);
	i2cWrite_NT99141(0x3372,0x3C);
	i2cWrite_NT99141(0x3373,0x3F);
	i2cWrite_NT99141(0x3374,0x3F);
	i2cWrite_NT99141(0x338A,0x34);
	i2cWrite_NT99141(0x338B,0x7F);
	i2cWrite_NT99141(0x338C,0x10);
	i2cWrite_NT99141(0x338D,0x23);
	i2cWrite_NT99141(0x338E,0x7F);
	i2cWrite_NT99141(0x338F,0x14);
	i2cWrite_NT99141(0x3375,0x0A); 
	i2cWrite_NT99141(0x3376,0x0C); 
	i2cWrite_NT99141(0x3377,0x10); 
	i2cWrite_NT99141(0x3378,0x14);               
	i2cWrite_NT99141(0x3012,0x02);
	i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
	i2cWrite_NT99141(0x32BF,0x60); 
    i2cWrite_NT99141(0x32C0,0x6A); 
    i2cWrite_NT99141(0x32C1,0x6A); 
    i2cWrite_NT99141(0x32C2,0x6A); 
    i2cWrite_NT99141(0x32C3,0x00); 
    i2cWrite_NT99141(0x32C4,0x20); 
    i2cWrite_NT99141(0x32C5,0x20); 
    i2cWrite_NT99141(0x32C6,0x20); 
    i2cWrite_NT99141(0x32C7,0x00); 
    i2cWrite_NT99141(0x32C8,0x91); 
    i2cWrite_NT99141(0x32C9,0x6A); 
    i2cWrite_NT99141(0x32CA,0x8A); 
    i2cWrite_NT99141(0x32CB,0x8A); 
    i2cWrite_NT99141(0x32CC,0x8A); 
    i2cWrite_NT99141(0x32CD,0x8A); 
    i2cWrite_NT99141(0x32DB,0x72); 
    i2cWrite_NT99141(0x3200,0x3E); 
    i2cWrite_NT99141(0x3201,0x0F); 
    i2cWrite_NT99141(0x3028,0x07); 
    i2cWrite_NT99141(0x3029,0x00); 
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24); 
    i2cWrite_NT99141(0x3002,0x00); 
    i2cWrite_NT99141(0x3003,0x04); 
    i2cWrite_NT99141(0x3004,0x00); 
    i2cWrite_NT99141(0x3005,0x04); 
    i2cWrite_NT99141(0x3006,0x05); 
    i2cWrite_NT99141(0x3007,0x03); 
    i2cWrite_NT99141(0x3008,0x02); 
    i2cWrite_NT99141(0x3009,0xD3); 
    i2cWrite_NT99141(0x300A,0x06); 
    i2cWrite_NT99141(0x300B,0x7C); 
    i2cWrite_NT99141(0x300C,0x03); 
    i2cWrite_NT99141(0x300D,0xC3); 
    i2cWrite_NT99141(0x300E,0x05); 
    i2cWrite_NT99141(0x300F,0x00); 
    i2cWrite_NT99141(0x3010,0x02); 
    i2cWrite_NT99141(0x3011,0xD0); 
    i2cWrite_NT99141(0x32B8,0x3F); 
    i2cWrite_NT99141(0x32B9,0x31); 
    i2cWrite_NT99141(0x32BB,0x87); 
    i2cWrite_NT99141(0x32BC,0x38); 
    i2cWrite_NT99141(0x32BD,0x3C); 
    i2cWrite_NT99141(0x32BE,0x34); 
    i2cWrite_NT99141(0x3201,0x3F); 
    i2cWrite_NT99141(0x3021,0x06); 
    i2cWrite_NT99141(0x3060,0x01);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306a,0x03);
    
    //siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

#elif 0 // 20140320

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x40); 
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x04);
    i2cWrite_NT99141(0x3272,0x0E);
    i2cWrite_NT99141(0x3273,0x28);
    i2cWrite_NT99141(0x3274,0x3F);
    i2cWrite_NT99141(0x3275,0x50);
    i2cWrite_NT99141(0x3276,0x6E);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0xA0);
    i2cWrite_NT99141(0x3279,0xB3);
    i2cWrite_NT99141(0x327A,0xD2);
    i2cWrite_NT99141(0x327B,0xE8);
    i2cWrite_NT99141(0x327C,0xF5);
    i2cWrite_NT99141(0x327D,0xFF);
    i2cWrite_NT99141(0x327E,0xFF);               
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x4C);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0x96);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x1D);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xDE);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x75);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xAE);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x40);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xD7);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xEA);             
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);                
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14); 
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
    i2cWrite_NT99141(0x3069,0x03); 
    i2cWrite_NT99141(0x306A,0x03);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------
    i2cWrite_NT99141(0x3200,0x3E);
    i2cWrite_NT99141(0x3201,0x0F);
    i2cWrite_NT99141(0x3028,0x07);
    i2cWrite_NT99141(0x3029,0x00);
    i2cWrite_NT99141(0x302A,0x04);
#if(SENSOR_ROW_COL_MIRROR)
    i2cWrite_NT99141(0x3022,0x27);
#else
    i2cWrite_NT99141(0x3022,0x24);
#endif
    i2cWrite_NT99141(0x3023,0x24);
    i2cWrite_NT99141(0x3002,0x00);
    i2cWrite_NT99141(0x3003,0x04);
    i2cWrite_NT99141(0x3004,0x00);
    i2cWrite_NT99141(0x3005,0x04);
    i2cWrite_NT99141(0x3006,0x05);
    i2cWrite_NT99141(0x3007,0x03);
    i2cWrite_NT99141(0x3008,0x02);
    i2cWrite_NT99141(0x3009,0xD3);
    i2cWrite_NT99141(0x300A,0x08);
    i2cWrite_NT99141(0x300B,0x39);
    i2cWrite_NT99141(0x300C,0x02);
    i2cWrite_NT99141(0x300D,0xF8);
    i2cWrite_NT99141(0x300E,0x05);
    i2cWrite_NT99141(0x300F,0x00);
    i2cWrite_NT99141(0x3010,0x02);
    i2cWrite_NT99141(0x3011,0xD0);
    i2cWrite_NT99141(0x32B8,0x3F);
    i2cWrite_NT99141(0x32B9,0x31);
    i2cWrite_NT99141(0x32BB,0x87);
    i2cWrite_NT99141(0x32BC,0x38);
    i2cWrite_NT99141(0x32BD,0x3C);
    i2cWrite_NT99141(0x32BE,0x34);
    i2cWrite_NT99141(0x3201,0x3F);
    i2cWrite_NT99141(0x3021,0x06);
    i2cWrite_NT99141(0x320A,0x00);
    i2cWrite_NT99141(0x3060,0x01);

#else   // 20140324

    //----- Initial Setting 20140324 ----//
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3250,0x80);
    i2cWrite_NT99141(0x3251,0x03);
    i2cWrite_NT99141(0x3252,0xFF);
    i2cWrite_NT99141(0x3253,0x00);
    i2cWrite_NT99141(0x3254,0x03);
    i2cWrite_NT99141(0x3255,0xFF);
    i2cWrite_NT99141(0x3256,0x00);
    i2cWrite_NT99141(0x3257,0x50);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x30);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xAC);
    i2cWrite_NT99141(0x3306,0x00);
    i2cWrite_NT99141(0x3307,0x23);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0xC0);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0xD6);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0x6A);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0x61);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0xEA);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0xB5);
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F3,0xB0);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x50);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A);
    i2cWrite_NT99141(0x3376,0x0C);
    i2cWrite_NT99141(0x3377,0x10);
    i2cWrite_NT99141(0x3378,0x14);
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);

    //----------------------------------------------
    //Timing Setting
    //----------------------------------------------

    i2cWrite_NT99141(0x3200, 0x3E); 
    i2cWrite_NT99141(0x3201, 0x0F); 
    i2cWrite_NT99141(0x3028, 0x07); 
    i2cWrite_NT99141(0x3029, 0x00); 
    i2cWrite_NT99141(0x302A, 0x04); 
    i2cWrite_NT99141(0x3022, 0x24); 
    i2cWrite_NT99141(0x3023, 0x24); 
    i2cWrite_NT99141(0x3002, 0x00); 
    i2cWrite_NT99141(0x3003, 0x04); 
    i2cWrite_NT99141(0x3004, 0x00); 
    i2cWrite_NT99141(0x3005, 0x04); 
    i2cWrite_NT99141(0x3006, 0x05); 
    i2cWrite_NT99141(0x3007, 0x03); 
    i2cWrite_NT99141(0x3008, 0x02); 
    i2cWrite_NT99141(0x3009, 0xD3); 
    i2cWrite_NT99141(0x300A, 0x06); 
    i2cWrite_NT99141(0x300B, 0x7C); 
    i2cWrite_NT99141(0x300C, 0x03); 
    i2cWrite_NT99141(0x300D, 0xC3); 
    i2cWrite_NT99141(0x300E, 0x05); 
    i2cWrite_NT99141(0x300F, 0x00); 
    i2cWrite_NT99141(0x3010, 0x02); 
    i2cWrite_NT99141(0x3011, 0xD0); 
    i2cWrite_NT99141(0x32B8, 0x3F); 
    i2cWrite_NT99141(0x32B9, 0x31); 
    i2cWrite_NT99141(0x32BB, 0x87); 
    i2cWrite_NT99141(0x32BC, 0x38); 
    i2cWrite_NT99141(0x32BD, 0x3C); 
    i2cWrite_NT99141(0x32BE, 0x34); 
    i2cWrite_NT99141(0x3201, 0x3F); 
    i2cWrite_NT99141(0x3021, 0x06);
    i2cWrite_NT99141(0x320A, 0x00); 
    i2cWrite_NT99141(0x3069, 0x03); 
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3024, 0x08);
    i2cWrite_NT99141(0x3060, 0x01); 

#endif

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
}


#endif


#if 0   // 20130923

void SetNT99141_VGA_30FPS(void)
{
    int i;

    
    DEBUG_SIU("SetNT99141_VGA_30FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3108, 0x00);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0xA0);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3250, 0xC0);
    i2cWrite_NT99141(0x3251, 0x00);
    i2cWrite_NT99141(0x3252, 0xDF);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xEB);
    i2cWrite_NT99141(0x3256, 0x81);
    i2cWrite_NT99141(0x3257, 0x40);
    i2cWrite_NT99141(0x329B, 0x01);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3210, 0x16);
    i2cWrite_NT99141(0x3211, 0x19);
    i2cWrite_NT99141(0x3212, 0x16);
    i2cWrite_NT99141(0x3213, 0x14);
    i2cWrite_NT99141(0x3214, 0x15);
    i2cWrite_NT99141(0x3215, 0x18);
    i2cWrite_NT99141(0x3216, 0x15);
    i2cWrite_NT99141(0x3217, 0x14);
    i2cWrite_NT99141(0x3218, 0x15);
    i2cWrite_NT99141(0x3219, 0x18);
    i2cWrite_NT99141(0x321A, 0x15);
    i2cWrite_NT99141(0x321B, 0x14);
    i2cWrite_NT99141(0x321C, 0x14);
    i2cWrite_NT99141(0x321D, 0x17);
    i2cWrite_NT99141(0x321E, 0x14);
    i2cWrite_NT99141(0x321F, 0x12);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x40);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0x96);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x29);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xBA);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0xF5);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0x51);
    i2cWrite_NT99141(0x330E, 0x01);
    i2cWrite_NT99141(0x330F, 0x30);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x16);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xBA);
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x3327, 0x0A);
    i2cWrite_NT99141(0x3328, 0x0A);
    i2cWrite_NT99141(0x3329, 0x06);
    i2cWrite_NT99141(0x332A, 0x06);
    i2cWrite_NT99141(0x332B, 0x1C);
    i2cWrite_NT99141(0x332C, 0x1C);
    i2cWrite_NT99141(0x332D, 0x00);
    i2cWrite_NT99141(0x332E, 0x1D);
    i2cWrite_NT99141(0x332F, 0x1F);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x30);
    i2cWrite_NT99141(0x3369, 0x28);
    i2cWrite_NT99141(0x336A, 0x20);
    i2cWrite_NT99141(0x336B, 0x10);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x38);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x0A);
    i2cWrite_NT99141(0x3332, 0xFF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3375, 0x0A);
    i2cWrite_NT99141(0x3376, 0x0C);
    i2cWrite_NT99141(0x3377, 0x10);
    i2cWrite_NT99141(0x3378, 0x14);
    i2cWrite_NT99141(0x3060, 0x01);

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

}

#elif(HW_BOARD_OPTION == MR8120_TX_RDI)

void SetNT99141_VGA_30FPS_RDI(void)
{
    int i;

    
    DEBUG_SIU("SetNT99141_VGA_30FPS_RDI()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

#if 0
    i2cWrite_NT99141(0x3109,0x04);
    i2cWrite_NT99141(0x3040,0x04);
    i2cWrite_NT99141(0x3041,0x02);
    i2cWrite_NT99141(0x3042,0xFF);
    i2cWrite_NT99141(0x3043,0x08);
    i2cWrite_NT99141(0x3052,0xE0);
    i2cWrite_NT99141(0x305F,0x33);
    i2cWrite_NT99141(0x3100,0x07);
    i2cWrite_NT99141(0x3106,0x03);
    i2cWrite_NT99141(0x3105,0x01);
    i2cWrite_NT99141(0x3108,0x05);
    i2cWrite_NT99141(0x3110,0x22);
    i2cWrite_NT99141(0x3111,0x57);
    i2cWrite_NT99141(0x3112,0x22);
    i2cWrite_NT99141(0x3113,0x55);
    i2cWrite_NT99141(0x3114,0x05);
    i2cWrite_NT99141(0x3135,0x00);
    i2cWrite_NT99141(0x32F0,0x01);
    i2cWrite_NT99141(0x3290,0x01);
    i2cWrite_NT99141(0x3291,0x80);
    i2cWrite_NT99141(0x3296,0x01);
    i2cWrite_NT99141(0x3297,0x73);
    i2cWrite_NT99141(0x3210,0x11);
    i2cWrite_NT99141(0x3211,0x14);
    i2cWrite_NT99141(0x3212,0x11);
    i2cWrite_NT99141(0x3213,0x10);
    i2cWrite_NT99141(0x3214,0x0F);
    i2cWrite_NT99141(0x3215,0x12);
    i2cWrite_NT99141(0x3216,0x10);
    i2cWrite_NT99141(0x3217,0x0F);
    i2cWrite_NT99141(0x3218,0x0F);
    i2cWrite_NT99141(0x3219,0x13);
    i2cWrite_NT99141(0x321A,0x10);
    i2cWrite_NT99141(0x321B,0x0F);
    i2cWrite_NT99141(0x321C,0x0F);
    i2cWrite_NT99141(0x321D,0x12);
    i2cWrite_NT99141(0x321E,0x0F);
    i2cWrite_NT99141(0x321F,0x0D);
    i2cWrite_NT99141(0x3231,0x74);
    i2cWrite_NT99141(0x3232,0xC4);
    i2cWrite_NT99141(0x3250,0xC0);
    i2cWrite_NT99141(0x3251,0x00);
    i2cWrite_NT99141(0x3252,0xF0);
    i2cWrite_NT99141(0x3253,0xA0);
    i2cWrite_NT99141(0x3254,0x00);
    i2cWrite_NT99141(0x3255,0xC0);
    i2cWrite_NT99141(0x3256,0x50);
    i2cWrite_NT99141(0x3257,0x40);
    i2cWrite_NT99141(0x329B,0x00);
    i2cWrite_NT99141(0x32A1,0x00);
    i2cWrite_NT99141(0x32A2,0xA0);
    i2cWrite_NT99141(0x32A3,0x01);
    i2cWrite_NT99141(0x32A4,0xA0);
    i2cWrite_NT99141(0x32A5,0x01);
    i2cWrite_NT99141(0x32A6,0x18);
    i2cWrite_NT99141(0x32A7,0x01);
    i2cWrite_NT99141(0x32A8,0xE0);
    i2cWrite_NT99141(0x3270,0x00);
    i2cWrite_NT99141(0x3271,0x0C);
    i2cWrite_NT99141(0x3272,0x18);
    i2cWrite_NT99141(0x3273,0x32);
    i2cWrite_NT99141(0x3274,0x44);
    i2cWrite_NT99141(0x3275,0x54);
    i2cWrite_NT99141(0x3276,0x70);
    i2cWrite_NT99141(0x3277,0x88);
    i2cWrite_NT99141(0x3278,0x9D);
    i2cWrite_NT99141(0x3279,0xB0);
    i2cWrite_NT99141(0x327A,0xCF);
    i2cWrite_NT99141(0x327B,0xE2);
    i2cWrite_NT99141(0x327C,0xEF);
    i2cWrite_NT99141(0x327D,0xF7);
    i2cWrite_NT99141(0x327E,0xFF);
    i2cWrite_NT99141(0x3302,0x00);
    i2cWrite_NT99141(0x3303,0x2E);
    i2cWrite_NT99141(0x3304,0x00);
    i2cWrite_NT99141(0x3305,0xEB);
    i2cWrite_NT99141(0x3306,0x07);
    i2cWrite_NT99141(0x3307,0xE5);
    i2cWrite_NT99141(0x3308,0x07);
    i2cWrite_NT99141(0x3309,0x89);
    i2cWrite_NT99141(0x330A,0x06);
    i2cWrite_NT99141(0x330B,0x7F);
    i2cWrite_NT99141(0x330C,0x01);
    i2cWrite_NT99141(0x330D,0xF9);
    i2cWrite_NT99141(0x330E,0x01);
    i2cWrite_NT99141(0x330F,0xDD);
    i2cWrite_NT99141(0x3310,0x06);
    i2cWrite_NT99141(0x3311,0x86);
    i2cWrite_NT99141(0x3312,0x07);
    i2cWrite_NT99141(0x3313,0x9E);
    i2cWrite_NT99141(0x3326,0x02);
    i2cWrite_NT99141(0x32F6,0x0F);
    i2cWrite_NT99141(0x32F9,0x42);
    i2cWrite_NT99141(0x32FA,0x24);
    i2cWrite_NT99141(0x3325,0x4A);
    i2cWrite_NT99141(0x3330,0x00);
    i2cWrite_NT99141(0x3331,0x0A);
    i2cWrite_NT99141(0x3332,0xFF);
    i2cWrite_NT99141(0x3338,0x30);
    i2cWrite_NT99141(0x3339,0x84);
    i2cWrite_NT99141(0x333A,0x48);
    i2cWrite_NT99141(0x333F,0x07);
    i2cWrite_NT99141(0x3360,0x10);
    i2cWrite_NT99141(0x3361,0x18);
    i2cWrite_NT99141(0x3362,0x1f);
    i2cWrite_NT99141(0x3363,0x37);
    i2cWrite_NT99141(0x3364,0x80);
    i2cWrite_NT99141(0x3365,0x80);
    i2cWrite_NT99141(0x3366,0x68);
    i2cWrite_NT99141(0x3367,0x60);
    i2cWrite_NT99141(0x3368,0x30);
    i2cWrite_NT99141(0x3369,0x28);
    i2cWrite_NT99141(0x336A,0x20);
    i2cWrite_NT99141(0x336B,0x10);
    i2cWrite_NT99141(0x336C,0x00);
    i2cWrite_NT99141(0x336D,0x20);
    i2cWrite_NT99141(0x336E,0x1C);
    i2cWrite_NT99141(0x336F,0x18);
    i2cWrite_NT99141(0x3370,0x10);
    i2cWrite_NT99141(0x3371,0x38);
    i2cWrite_NT99141(0x3372,0x3C);
    i2cWrite_NT99141(0x3373,0x3F);
    i2cWrite_NT99141(0x3374,0x3F);
    i2cWrite_NT99141(0x338A,0x34);
    i2cWrite_NT99141(0x338B,0x7F);
    i2cWrite_NT99141(0x338C,0x10);
    i2cWrite_NT99141(0x338D,0x23);
    i2cWrite_NT99141(0x338E,0x7F);
    i2cWrite_NT99141(0x338F,0x14);
    i2cWrite_NT99141(0x3375,0x0A); 
    i2cWrite_NT99141(0x3376,0x0C); 
    i2cWrite_NT99141(0x3377,0x10); 
    i2cWrite_NT99141(0x3378,0x14); 
    i2cWrite_NT99141(0x3012,0x02);
    i2cWrite_NT99141(0x3013,0xD0);
	i2cWrite_NT99141(0x306A,0x03);
	i2cWrite_NT99141(0x3060,0x01);
#elif 0   // 20140224
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0x80);
    i2cWrite_NT99141(0x3251, 0x03);
    i2cWrite_NT99141(0x3252, 0xFF);
    i2cWrite_NT99141(0x3253, 0x00);
    i2cWrite_NT99141(0x3254, 0x03);
    i2cWrite_NT99141(0x3255, 0xFF);
    i2cWrite_NT99141(0x3256, 0x00);
    i2cWrite_NT99141(0x3257, 0x40);
    i2cWrite_NT99141(0x329B, 0x00);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x40);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0x96);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x29);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xBA);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0xF5);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0x51);
    i2cWrite_NT99141(0x330E, 0x01);
    i2cWrite_NT99141(0x330F, 0x30);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x16);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xBA);
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x0A);
    i2cWrite_NT99141(0x3332, 0xFF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x30);
    i2cWrite_NT99141(0x3369, 0x28);
    i2cWrite_NT99141(0x336A, 0x20);
    i2cWrite_NT99141(0x336B, 0x10);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x38);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
	//i2cWrite_NT99141(0x3069, 0x01);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#elif 0   // 20140707
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8),
    i2cWrite_NT99141(0x3251, 0x01),
    i2cWrite_NT99141(0x3252, 0x1B),
    i2cWrite_NT99141(0x3253, 0x85),
    i2cWrite_NT99141(0x3254, 0x00),
    i2cWrite_NT99141(0x3255, 0xC9),
    i2cWrite_NT99141(0x3256, 0x91),
    i2cWrite_NT99141(0x3257, 0x46),
    i2cWrite_NT99141(0x3258, 0x0A),
    i2cWrite_NT99141(0x329B, 0x31),
    i2cWrite_NT99141(0x32A1, 0x00),
    i2cWrite_NT99141(0x32A2, 0xFF),
    i2cWrite_NT99141(0x32A3, 0x01),
    i2cWrite_NT99141(0x32A4, 0xB0),
    i2cWrite_NT99141(0x32A5, 0x01),
    i2cWrite_NT99141(0x32A6, 0x50),
    i2cWrite_NT99141(0x32A7, 0x01),
    i2cWrite_NT99141(0x32A8, 0xE0),
    i2cWrite_NT99141(0x3270, 0x00),
    i2cWrite_NT99141(0x3271, 0x05),
    i2cWrite_NT99141(0x3272, 0x11),
    i2cWrite_NT99141(0x3273, 0x28),
    i2cWrite_NT99141(0x3274, 0x43),
    i2cWrite_NT99141(0x3275, 0x5C),
    i2cWrite_NT99141(0x3276, 0x7E),
    i2cWrite_NT99141(0x3277, 0x9B),
    i2cWrite_NT99141(0x3278, 0xB3),
    i2cWrite_NT99141(0x3279, 0xC6),
    i2cWrite_NT99141(0x327A, 0xD9),
    i2cWrite_NT99141(0x327B, 0xE7),
    i2cWrite_NT99141(0x327C, 0xF6),
    i2cWrite_NT99141(0x327D, 0xFF),
    i2cWrite_NT99141(0x327E, 0xFF),
    i2cWrite_NT99141(0x3302, 0x00),
    i2cWrite_NT99141(0x3303, 0x00),
    i2cWrite_NT99141(0x3304, 0x00),
    i2cWrite_NT99141(0x3305, 0xE1),
    i2cWrite_NT99141(0x3306, 0x00),
    i2cWrite_NT99141(0x3307, 0x1E),
    i2cWrite_NT99141(0x3308, 0x07),
    i2cWrite_NT99141(0x3309, 0xCF),
    i2cWrite_NT99141(0x330A, 0x06),
    i2cWrite_NT99141(0x330B, 0x8F),
    i2cWrite_NT99141(0x330C, 0x01),
    i2cWrite_NT99141(0x330D, 0xA2),
    i2cWrite_NT99141(0x330E, 0x00),
    i2cWrite_NT99141(0x330F, 0xF0),
    i2cWrite_NT99141(0x3310, 0x07),
    i2cWrite_NT99141(0x3311, 0x19),
    i2cWrite_NT99141(0x3312, 0x07),
    i2cWrite_NT99141(0x3313, 0xF7),
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x60);
    i2cWrite_NT99141(0x3368, 0x78);
    i2cWrite_NT99141(0x3369, 0x60);
    i2cWrite_NT99141(0x336A, 0x48);
    i2cWrite_NT99141(0x336B, 0x32);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1C);
    i2cWrite_NT99141(0x336F, 0x18);
    i2cWrite_NT99141(0x3370, 0x10);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#elif 0   // 20140708
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8);
    i2cWrite_NT99141(0x3251, 0x01);
    i2cWrite_NT99141(0x3252, 0x1B);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xC9);
    i2cWrite_NT99141(0x3256, 0x91);
    i2cWrite_NT99141(0x3257, 0x46);
    i2cWrite_NT99141(0x3258, 0x0A);
    i2cWrite_NT99141(0x329B, 0x31);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xFF);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xB0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x50);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x05);
    i2cWrite_NT99141(0x3272, 0x11);
    i2cWrite_NT99141(0x3273, 0x28);
    i2cWrite_NT99141(0x3274, 0x43);
    i2cWrite_NT99141(0x3275, 0x5C);
    i2cWrite_NT99141(0x3276, 0x7E);
    i2cWrite_NT99141(0x3277, 0x9B);
    i2cWrite_NT99141(0x3278, 0xB3);
    i2cWrite_NT99141(0x3279, 0xC6);
    i2cWrite_NT99141(0x327A, 0xD9);
    i2cWrite_NT99141(0x327B, 0xE7);
    i2cWrite_NT99141(0x327C, 0xF6);
    i2cWrite_NT99141(0x327D, 0xFF);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x00);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0xE1);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x1E);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xCF);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0x8F);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0xA2);
    i2cWrite_NT99141(0x330E, 0x00);
    i2cWrite_NT99141(0x330F, 0xF0);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x19);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xF7);
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x80);
    i2cWrite_NT99141(0x3366, 0x68);
    i2cWrite_NT99141(0x3367, 0x56);
    i2cWrite_NT99141(0x3368, 0x70);
    i2cWrite_NT99141(0x3369, 0x58);
    i2cWrite_NT99141(0x336A, 0x40);
    i2cWrite_NT99141(0x336B, 0x2C);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1A);
    i2cWrite_NT99141(0x336F, 0x16);
    i2cWrite_NT99141(0x3370, 0x0E);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#else   // 20140710
    i2cWrite_NT99141(0x3109, 0x04);
    i2cWrite_NT99141(0x3040, 0x04);
    i2cWrite_NT99141(0x3041, 0x02);
    i2cWrite_NT99141(0x3042, 0xFF);
    i2cWrite_NT99141(0x3043, 0x08);
    i2cWrite_NT99141(0x3052, 0xE0);
    i2cWrite_NT99141(0x305F, 0x33);
    i2cWrite_NT99141(0x3100, 0x07);
    i2cWrite_NT99141(0x3106, 0x03);
    i2cWrite_NT99141(0x3105, 0x01);
    i2cWrite_NT99141(0x3108, 0x05);
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);
    i2cWrite_NT99141(0x3135, 0x00);
    i2cWrite_NT99141(0x32F0, 0x01);
    i2cWrite_NT99141(0x3290, 0x01);
    i2cWrite_NT99141(0x3291, 0x80);
    i2cWrite_NT99141(0x3296, 0x01);
    i2cWrite_NT99141(0x3297, 0x73);
    i2cWrite_NT99141(0x3210, 0x11);
    i2cWrite_NT99141(0x3211, 0x14);
    i2cWrite_NT99141(0x3212, 0x11);
    i2cWrite_NT99141(0x3213, 0x10);
    i2cWrite_NT99141(0x3214, 0x0F);
    i2cWrite_NT99141(0x3215, 0x12);
    i2cWrite_NT99141(0x3216, 0x10);
    i2cWrite_NT99141(0x3217, 0x0F);
    i2cWrite_NT99141(0x3218, 0x0F);
    i2cWrite_NT99141(0x3219, 0x13);
    i2cWrite_NT99141(0x321A, 0x10);
    i2cWrite_NT99141(0x321B, 0x0F);
    i2cWrite_NT99141(0x321C, 0x0F);
    i2cWrite_NT99141(0x321D, 0x12);
    i2cWrite_NT99141(0x321E, 0x0F);
    i2cWrite_NT99141(0x321F, 0x0D);
    i2cWrite_NT99141(0x3231, 0x74);
    i2cWrite_NT99141(0x3232, 0xC4);
    i2cWrite_NT99141(0x3250, 0xC8);
    i2cWrite_NT99141(0x3251, 0x01);
    i2cWrite_NT99141(0x3252, 0x1B);
    i2cWrite_NT99141(0x3253, 0x85);
    i2cWrite_NT99141(0x3254, 0x00);
    i2cWrite_NT99141(0x3255, 0xC9);
    i2cWrite_NT99141(0x3256, 0x91);
    i2cWrite_NT99141(0x3257, 0x46);
    i2cWrite_NT99141(0x3258, 0x0A);
    i2cWrite_NT99141(0x329B, 0x31);
    i2cWrite_NT99141(0x32A1, 0x00);
    i2cWrite_NT99141(0x32A2, 0xFF);
    i2cWrite_NT99141(0x32A3, 0x01);
    i2cWrite_NT99141(0x32A4, 0xB0);
    i2cWrite_NT99141(0x32A5, 0x01);
    i2cWrite_NT99141(0x32A6, 0x50);
    i2cWrite_NT99141(0x32A7, 0x01);
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x05);
    i2cWrite_NT99141(0x3272, 0x11);
    i2cWrite_NT99141(0x3273, 0x28);
    i2cWrite_NT99141(0x3274, 0x43);
    i2cWrite_NT99141(0x3275, 0x5C);
    i2cWrite_NT99141(0x3276, 0x7E);
    i2cWrite_NT99141(0x3277, 0x9B);
    i2cWrite_NT99141(0x3278, 0xB3);
    i2cWrite_NT99141(0x3279, 0xC6);
    i2cWrite_NT99141(0x327A, 0xD9);
    i2cWrite_NT99141(0x327B, 0xE7);
    i2cWrite_NT99141(0x327C, 0xF6);
    i2cWrite_NT99141(0x327D, 0xFF);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00);
    i2cWrite_NT99141(0x3303, 0x00);
    i2cWrite_NT99141(0x3304, 0x00);
    i2cWrite_NT99141(0x3305, 0xE1);
    i2cWrite_NT99141(0x3306, 0x00);
    i2cWrite_NT99141(0x3307, 0x1E);
    i2cWrite_NT99141(0x3308, 0x07);
    i2cWrite_NT99141(0x3309, 0xCF);
    i2cWrite_NT99141(0x330A, 0x06);
    i2cWrite_NT99141(0x330B, 0x8F);
    i2cWrite_NT99141(0x330C, 0x01);
    i2cWrite_NT99141(0x330D, 0xA2);
    i2cWrite_NT99141(0x330E, 0x00);
    i2cWrite_NT99141(0x330F, 0xF0);
    i2cWrite_NT99141(0x3310, 0x07);
    i2cWrite_NT99141(0x3311, 0x19);
    i2cWrite_NT99141(0x3312, 0x07);
    i2cWrite_NT99141(0x3313, 0xF7);
    i2cWrite_NT99141(0x3326, 0x03);
    i2cWrite_NT99141(0x32F6, 0x0F);
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);
    i2cWrite_NT99141(0x3330, 0x00);
    i2cWrite_NT99141(0x3331, 0x07);
    i2cWrite_NT99141(0x3332, 0xDF);
    i2cWrite_NT99141(0x3338, 0x30);
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);
    i2cWrite_NT99141(0x3360, 0x10);
    i2cWrite_NT99141(0x3361, 0x18);
    i2cWrite_NT99141(0x3362, 0x1f);
    i2cWrite_NT99141(0x3363, 0x37);
    i2cWrite_NT99141(0x3364, 0x80);
    i2cWrite_NT99141(0x3365, 0x70);
    i2cWrite_NT99141(0x3366, 0x60);
    i2cWrite_NT99141(0x3367, 0x48);
    i2cWrite_NT99141(0x3368, 0x70);
    i2cWrite_NT99141(0x3369, 0x58);
    i2cWrite_NT99141(0x336A, 0x40);
    i2cWrite_NT99141(0x336B, 0x2C);
    i2cWrite_NT99141(0x336C, 0x00);
    i2cWrite_NT99141(0x336D, 0x20);
    i2cWrite_NT99141(0x336E, 0x1A);
    i2cWrite_NT99141(0x336F, 0x16);
    i2cWrite_NT99141(0x3370, 0x0E);
    i2cWrite_NT99141(0x3371, 0x36);
    i2cWrite_NT99141(0x3372, 0x3C);
    i2cWrite_NT99141(0x3373, 0x3F);
    i2cWrite_NT99141(0x3374, 0x3F);
    i2cWrite_NT99141(0x338A, 0x34);
    i2cWrite_NT99141(0x338B, 0x7F);
    i2cWrite_NT99141(0x338C, 0x10);
    i2cWrite_NT99141(0x338D, 0x23);
    i2cWrite_NT99141(0x338E, 0x7F);
    i2cWrite_NT99141(0x338F, 0x14);
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x10); 
    i2cWrite_NT99141(0x3378, 0x14); 
    i2cWrite_NT99141(0x3012, 0x02);
    i2cWrite_NT99141(0x3013, 0xD0);
    i2cWrite_NT99141(0x306A, 0x03);
    i2cWrite_NT99141(0x3060, 0x01);
#endif

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
}

#else   // 20130930

// [Date] 20130930
// [Sensor] NT99141
// MCLK = 24Mhz
// PCLK = 60Mhz
// Frame Rate
// Preview: 30fps for Yuv
// Flicker: 60hz/50hz
void SetNT99141_VGA_30FPS(void)
{
    int i;

    
    DEBUG_SIU("SetNT99141_VGA_30FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------

    i2cWrite_NT99141(0x3109, 0x04);	
    i2cWrite_NT99141(0x3040, 0x04);	
    i2cWrite_NT99141(0x3041, 0x02);	
    i2cWrite_NT99141(0x3042, 0xFF);	
    i2cWrite_NT99141(0x3043, 0x08);	
    i2cWrite_NT99141(0x3052, 0xE0);	
    i2cWrite_NT99141(0x305F, 0x33);	
    i2cWrite_NT99141(0x3100, 0x07);	
    i2cWrite_NT99141(0x3106, 0x03);	
    i2cWrite_NT99141(0x3108, 0x00);	
    i2cWrite_NT99141(0x3110, 0x22);
    i2cWrite_NT99141(0x3111, 0x57);
    i2cWrite_NT99141(0x3112, 0x22);	
    i2cWrite_NT99141(0x3113, 0x55);
    i2cWrite_NT99141(0x3114, 0x05);	
    i2cWrite_NT99141(0x3135, 0x00);	
    i2cWrite_NT99141(0x32F0, 0x01);	
    i2cWrite_NT99141(0x3290, 0x01);	
    i2cWrite_NT99141(0x3291, 0xA0);	 
    i2cWrite_NT99141(0x3296, 0x01);	
    i2cWrite_NT99141(0x3297, 0x73);	
    i2cWrite_NT99141(0x3250, 0xC0); 
    i2cWrite_NT99141(0x3251, 0x00);  
    i2cWrite_NT99141(0x3252, 0xDF);  
    i2cWrite_NT99141(0x3253, 0x85);  
    i2cWrite_NT99141(0x3254, 0x00);  
    i2cWrite_NT99141(0x3255, 0xEB);  
    i2cWrite_NT99141(0x3256, 0x81);  
    i2cWrite_NT99141(0x3257, 0x40);	
    i2cWrite_NT99141(0x329B, 0x01);	
    i2cWrite_NT99141(0x32A1, 0x00);	
    i2cWrite_NT99141(0x32A2, 0xA0);
    i2cWrite_NT99141(0x32A3, 0x01);	
    i2cWrite_NT99141(0x32A4, 0xA0);
    i2cWrite_NT99141(0x32A5, 0x01);	
    i2cWrite_NT99141(0x32A6, 0x18);
    i2cWrite_NT99141(0x32A7, 0x01);	
    i2cWrite_NT99141(0x32A8, 0xE0);
    i2cWrite_NT99141(0x3210, 0x16); 
    i2cWrite_NT99141(0x3211, 0x19); 
    i2cWrite_NT99141(0x3212, 0x16); 
    i2cWrite_NT99141(0x3213, 0x14);  
    i2cWrite_NT99141(0x3214, 0x15);  
    i2cWrite_NT99141(0x3215, 0x18);  
    i2cWrite_NT99141(0x3216, 0x15);  
    i2cWrite_NT99141(0x3217, 0x14);  
    i2cWrite_NT99141(0x3218, 0x15);  
    i2cWrite_NT99141(0x3219, 0x18);  
    i2cWrite_NT99141(0x321A, 0x15);  
    i2cWrite_NT99141(0x321B, 0x14);  
    i2cWrite_NT99141(0x321C, 0x14);  
    i2cWrite_NT99141(0x321D, 0x17);  
    i2cWrite_NT99141(0x321E, 0x14);  
    i2cWrite_NT99141(0x321F, 0x12);  
    i2cWrite_NT99141(0x3231, 0x74);  
    i2cWrite_NT99141(0x3232, 0xC4);  
    i2cWrite_NT99141(0x3270, 0x00);
    i2cWrite_NT99141(0x3271, 0x0C);
    i2cWrite_NT99141(0x3272, 0x18);
    i2cWrite_NT99141(0x3273, 0x32);
    i2cWrite_NT99141(0x3274, 0x44);
    i2cWrite_NT99141(0x3275, 0x54);
    i2cWrite_NT99141(0x3276, 0x70);
    i2cWrite_NT99141(0x3277, 0x88);
    i2cWrite_NT99141(0x3278, 0x9D);
    i2cWrite_NT99141(0x3279, 0xB0);
    i2cWrite_NT99141(0x327A, 0xCF);
    i2cWrite_NT99141(0x327B, 0xE2);
    i2cWrite_NT99141(0x327C, 0xEF);
    i2cWrite_NT99141(0x327D, 0xF7);
    i2cWrite_NT99141(0x327E, 0xFF);
    i2cWrite_NT99141(0x3302, 0x00); 
    i2cWrite_NT99141(0x3303, 0x40);  
    i2cWrite_NT99141(0x3304, 0x00);  
    i2cWrite_NT99141(0x3305, 0x96);  
    i2cWrite_NT99141(0x3306, 0x00);  
    i2cWrite_NT99141(0x3307, 0x29);  
    i2cWrite_NT99141(0x3308, 0x07);  
    i2cWrite_NT99141(0x3309, 0xBA);  
    i2cWrite_NT99141(0x330A, 0x06);  
    i2cWrite_NT99141(0x330B, 0xF5);  
    i2cWrite_NT99141(0x330C, 0x01);  
    i2cWrite_NT99141(0x330D, 0x51);  
    i2cWrite_NT99141(0x330E, 0x01);  
    i2cWrite_NT99141(0x330F, 0x30);  
    i2cWrite_NT99141(0x3310, 0x07);  
    i2cWrite_NT99141(0x3311, 0x16);  
    i2cWrite_NT99141(0x3312, 0x07);  
    i2cWrite_NT99141(0x3313, 0xBA);  
    i2cWrite_NT99141(0x3326, 0x02);
    i2cWrite_NT99141(0x3327, 0x0A);
    i2cWrite_NT99141(0x3328, 0x0A);
    i2cWrite_NT99141(0x3329, 0x06);
    i2cWrite_NT99141(0x332A, 0x06);
    i2cWrite_NT99141(0x332B, 0x1C);
    i2cWrite_NT99141(0x332C, 0x1C);
    i2cWrite_NT99141(0x332D, 0x00);
    i2cWrite_NT99141(0x332E, 0x1D);
    i2cWrite_NT99141(0x332F, 0x1F);
    i2cWrite_NT99141(0x3360, 0x10);  
    i2cWrite_NT99141(0x3361, 0x18); 
    i2cWrite_NT99141(0x3362, 0x1f);  
    i2cWrite_NT99141(0x3363, 0x37); 
    i2cWrite_NT99141(0x3364, 0x80);  
    i2cWrite_NT99141(0x3365, 0x80); 
    i2cWrite_NT99141(0x3366, 0x64);//0x68  
    i2cWrite_NT99141(0x3367, 0x4C);//0x60  
    i2cWrite_NT99141(0x3368, 0x30); 
    i2cWrite_NT99141(0x3369, 0x28);  
    i2cWrite_NT99141(0x336A, 0x20);  
    i2cWrite_NT99141(0x336B, 0x12);//0x10  
    i2cWrite_NT99141(0x336C, 0x00);  
    i2cWrite_NT99141(0x336D, 0x20);  
    i2cWrite_NT99141(0x336E, 0x1C);  
    i2cWrite_NT99141(0x336F, 0x18);  
    i2cWrite_NT99141(0x3370, 0x10);  
    i2cWrite_NT99141(0x3371, 0x38);  
    i2cWrite_NT99141(0x3372, 0x3C);  
    i2cWrite_NT99141(0x3373, 0x3F);  
    i2cWrite_NT99141(0x3374, 0x3F);   
    i2cWrite_NT99141(0x338A, 0x34);  
    i2cWrite_NT99141(0x338B, 0x7F); 
    i2cWrite_NT99141(0x338C, 0x10); 
    i2cWrite_NT99141(0x338D, 0x23);  
    i2cWrite_NT99141(0x338E, 0x7F);  
    i2cWrite_NT99141(0x338F, 0x14);  
    i2cWrite_NT99141(0x32F6, 0x0F);	
    i2cWrite_NT99141(0x32F9, 0x42);
    i2cWrite_NT99141(0x32FA, 0x24);
    i2cWrite_NT99141(0x3325, 0x4A);	
    i2cWrite_NT99141(0x3330, 0x00);	
    i2cWrite_NT99141(0x3331, 0x08);	
    i2cWrite_NT99141(0x3332, 0xFF);	
    i2cWrite_NT99141(0x3338, 0x30);	
    i2cWrite_NT99141(0x3339, 0x84);
    i2cWrite_NT99141(0x333A, 0x48);
    i2cWrite_NT99141(0x333F, 0x07);	
    i2cWrite_NT99141(0x3375, 0x0A); 
    i2cWrite_NT99141(0x3376, 0x0C); 
    i2cWrite_NT99141(0x3377, 0x12);  
    i2cWrite_NT99141(0x3378, 0x18); 
    i2cWrite_NT99141(0x3069, 0x03); 
//#if (HW_BOARD_OPTION == MR8120_TX_COMMAX)
//    i2cWrite_NT99141(0x306A, 0x00);
//#else
    i2cWrite_NT99141(0x306A, 0x03);
//#endif
    i2cWrite_NT99141(0x3060, 0x01);

    siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

    /* brightness */
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

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
    //----------//
    
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
#elif(HW_BOARD_OPTION == A1013_REALCHIP_A)
    //---------Run on Mclk=24 MHz--------------//
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
    
#elif(HW_BOARD_OPTION == A1018_REALCHIP_A)
   //not implement    
   
#else   
   //---------Run on Mclk=24 MHz--------------//
   #if(SYS_CPU_CLK_FREQ == 96000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0b; //MClk=288/12=24MHz
    //SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x04; //MClk=288/12=57.6MHz
   #elif(SYS_CPU_CLK_FREQ == 108000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x08; //MClk=216/9=24MHz
    //SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x03; //MClk=216/4=54MHz
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
                FrameRate   = 15;
            #if(HW_BOARD_OPTION == MR8120_TX_RDI)
                SetNT99141_720P_15FPS_RDI();
            #elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
                  (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                //SetNT99141_720P_15FPS_TRANWO();
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #elif ((HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) || (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||  (HW_BOARD_OPTION == MR8120_TX_MA8806))
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #else
                SetNT99141_720P_15FPS();
            #endif
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240) )
            {
                FrameRate   = 30;
            #if(HW_BOARD_OPTION == MR8120_TX_RDI)
                SetNT99141_VGA_30FPS_RDI(); 
            #elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) ||\
                  (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                //SetNT99141_VGA_30FPS_TRANWO();
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #elif((HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) || (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||  (HW_BOARD_OPTION == MR8120_TX_MA8806))
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #else
                SetNT99141_VGA_30FPS();
            #endif
            }
			else
		    {
                FrameRate   = 30;
            #if(HW_BOARD_OPTION == MR8120_TX_RDI)
                SetNT99141_VGA_30FPS_RDI();
            #elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) ||\
                  (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                //SetNT99141_VGA_30FPS_TRANWO();
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #elif((HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) || (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C) || (HW_BOARD_OPTION == MR8120_TX_MA8806))
                siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            #else
                SetNT99141_VGA_30FPS();
            #endif
			}
        #if(HW_BOARD_OPTION == MR8120_TX_RDI)
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
        #elif ((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
            gpioGetLevel(GPIO_GROUP_IR_DETECTION,GPIO_PIN_IR_DETECTION,&level);
        
            if(level == SIU_DAY_MODE)
            {
                DEBUG_SIU("@@enter day %d\n",level);
                siuSetSensorDayNight(SIU_DAY_MODE);
            }
            else 
            {
                DEBUG_SIU("##enter night %d\n",level);
                siuSetSensorDayNight(SIU_NIGHT_MODE);
            }
        #elif(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
            level   = ((AdcRecData_G1G2 & 0x00000fff) > 0x00000400) ? SIU_DAY_MODE : SIU_NIGHT_MODE;
        
            if(level == SIU_DAY_MODE)
            {
                DEBUG_SIU("@@enter day %d\n", level);
                siuSetSensorDayNight(SIU_DAY_MODE);
            }
            else 
            {
                DEBUG_SIU("##enter night %d\n", level);
                siuSetSensorDayNight(SIU_NIGHT_MODE);
            }
        #elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
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
		#elif( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE==1) )
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
	const   s8  ExposureBiasValueTab[9]    = {-20, -15, -10, -5, 0, 5, 10, 15, 20};
    u8          level;
	
    if((expBiasValue > 8) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetExposureValue(%d) fail!!!\n", expBiasValue);
        return 0;
    }

	exifSetExposureBiasValue(ExposureBiasValueTab[expBiasValue]);		/* expBiasValue = Exposure Bias Value * 10 */		
	siuY_TargetIndex    = expBiasValue;

	/* set exposure bias */
#if(HW_BOARD_OPTION == MR8120_TX_RDI)
    gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level);
    if(level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
	{   //夜間模式
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] + 0x20);
	}
    else
    {   //日間模式
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
    }
#elif ((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
    gpioGetLevel(GPIO_GROUP_IR_DETECTION,GPIO_PIN_IR_DETECTION,&level);
    if(level == SIU_NIGHT_MODE)
    {
    #if(SENSOR_PARAMETER_VERSION == 1)
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] - 0x1A);    
    #else
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] + 0x28);    
    #endif
    }
    else
    {
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

    }
#elif(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    level   = ((AdcRecData_G1G2 & 0x00000fff) > 0x00000400) ? SIU_DAY_MODE : SIU_NIGHT_MODE;
        
    if(level == SIU_NIGHT_MODE)
    {
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] - 0x1A);    
    }
    else
    {
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

    }
#else
    i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);

#endif

	return 1;
}

#if 0   // 20130923

s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x70);
            i2cWrite_NT99141(0x32C1, 0x70);
            i2cWrite_NT99141(0x32C2, 0x70);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x20);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0x5F);
            i2cWrite_NT99141(0x32C9, 0x70);
            i2cWrite_NT99141(0x32CA, 0x90);
            i2cWrite_NT99141(0x32CB, 0x90);
            i2cWrite_NT99141(0x32CC, 0x90);
            i2cWrite_NT99141(0x32CD, 0x90);
            i2cWrite_NT99141(0x32DB, 0x67);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x12);
            i2cWrite_NT99141(0x3029, 0x12);
            i2cWrite_NT99141(0x302A, 0x00);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0x04);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x05);
            i2cWrite_NT99141(0x3007, 0x03);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x06);
            i2cWrite_NT99141(0x300B, 0x7C);
            i2cWrite_NT99141(0x300C, 0x02);
            i2cWrite_NT99141(0x300D, 0xFB);
            i2cWrite_NT99141(0x300E, 0x05);
            i2cWrite_NT99141(0x300F, 0x00);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x3F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        }
        else //50Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x6A);
            i2cWrite_NT99141(0x32C1, 0x6A);
            i2cWrite_NT99141(0x32C2, 0x6A);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x20);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0x72);
            i2cWrite_NT99141(0x32C9, 0x6A);
            i2cWrite_NT99141(0x32CA, 0x8A);
            i2cWrite_NT99141(0x32CB, 0x8A);
            i2cWrite_NT99141(0x32CC, 0x8A);
            i2cWrite_NT99141(0x32CD, 0x8A);
            i2cWrite_NT99141(0x32DB, 0x6C);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x12);
            i2cWrite_NT99141(0x3029, 0x12);
            i2cWrite_NT99141(0x302A, 0x00);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0x04);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x05);
            i2cWrite_NT99141(0x3007, 0x03);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x06);
            i2cWrite_NT99141(0x300B, 0x7C);
            i2cWrite_NT99141(0x300C, 0x02);
            i2cWrite_NT99141(0x300D, 0xFB);
            i2cWrite_NT99141(0x300E, 0x05);
            i2cWrite_NT99141(0x300F, 0x00);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x3F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        }
    }
    else    // VGA
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x60);
            i2cWrite_NT99141(0x32C1, 0x5F);
            i2cWrite_NT99141(0x32C2, 0x5F);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x20);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0xE6);
            i2cWrite_NT99141(0x32C9, 0x5F);
            i2cWrite_NT99141(0x32CA, 0x7F);
            i2cWrite_NT99141(0x32CB, 0x7F);
            i2cWrite_NT99141(0x32CC, 0x7F);
            i2cWrite_NT99141(0x32CD, 0x80);
            i2cWrite_NT99141(0x32DB, 0x7C);
            i2cWrite_NT99141(0x32E0, 0x02);
            i2cWrite_NT99141(0x32E1, 0x80);
            i2cWrite_NT99141(0x32E2, 0x01);
            i2cWrite_NT99141(0x32E3, 0xE0);
            i2cWrite_NT99141(0x32E4, 0x00);
            i2cWrite_NT99141(0x32E5, 0x80);
            i2cWrite_NT99141(0x32E6, 0x00);
            i2cWrite_NT99141(0x32E7, 0x80);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x24);
            i2cWrite_NT99141(0x3029, 0x20);
            i2cWrite_NT99141(0x302A, 0x04);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0xA4);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x04);
            i2cWrite_NT99141(0x3007, 0x63);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x05);
            i2cWrite_NT99141(0x300B, 0x3C);
            i2cWrite_NT99141(0x300C, 0x03);
            i2cWrite_NT99141(0x300D, 0x98);
            i2cWrite_NT99141(0x300E, 0x03);
            i2cWrite_NT99141(0x300F, 0xC0);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x7F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        }
        else //50Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x5A);
            i2cWrite_NT99141(0x32C1, 0x5A);
            i2cWrite_NT99141(0x32C2, 0x5A);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x20);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x40);
            i2cWrite_NT99141(0x32C8, 0x14);
            i2cWrite_NT99141(0x32C9, 0x5A);
            i2cWrite_NT99141(0x32CA, 0x7A);
            i2cWrite_NT99141(0x32CB, 0x7A);
            i2cWrite_NT99141(0x32CC, 0x7A);
            i2cWrite_NT99141(0x32CD, 0x7A);
            i2cWrite_NT99141(0x32DB, 0x81);
            i2cWrite_NT99141(0x32E0, 0x02);
            i2cWrite_NT99141(0x32E1, 0x80);
            i2cWrite_NT99141(0x32E2, 0x01);
            i2cWrite_NT99141(0x32E3, 0xE0);
            i2cWrite_NT99141(0x32E4, 0x00);
            i2cWrite_NT99141(0x32E5, 0x80);
            i2cWrite_NT99141(0x32E6, 0x00);
            i2cWrite_NT99141(0x32E7, 0x80);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x24);
            i2cWrite_NT99141(0x3029, 0x20);
            i2cWrite_NT99141(0x302A, 0x04);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0xA4);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x04);
            i2cWrite_NT99141(0x3007, 0x63);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x05);
            i2cWrite_NT99141(0x300B, 0x3C);
            i2cWrite_NT99141(0x300C, 0x03);
            i2cWrite_NT99141(0x300D, 0x98);
            i2cWrite_NT99141(0x300E, 0x03);
            i2cWrite_NT99141(0x300F, 0xC0);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x7F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        }
    }

    return 0;
}


#elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) ||\
      (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    u8 i;

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    

    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        SetNT99141_720P_15FPS_TRANWO();
        #if(SENSOR_PARAMETER_VERSION == 1) //use new parameter
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x70); 
            i2cWrite_NT99141(0x32C1, 0x70); 
            i2cWrite_NT99141(0x32C2, 0x70); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x28);
            #if 1 
            // 151103 v2
            i2cWrite_NT99141(0x32C5, 0x28); 
            i2cWrite_NT99141(0x32C6, 0x28); 
            #else
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            #endif
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x78); 
            i2cWrite_NT99141(0x32C9, 0x70); 
            i2cWrite_NT99141(0x32CA, 0x90); 
            i2cWrite_NT99141(0x32CB, 0x90); 
            i2cWrite_NT99141(0x32CC, 0x90); 
            i2cWrite_NT99141(0x32CD, 0x90); 
            i2cWrite_NT99141(0x32DB, 0x6E);     
        }
        else
        {
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x6A); 
            i2cWrite_NT99141(0x32C1, 0x6A); 
            i2cWrite_NT99141(0x32C2, 0x6A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x28); 
            #if 1
            // 151103 v2
            i2cWrite_NT99141(0x32C5, 0x28); 
            i2cWrite_NT99141(0x32C6, 0x28); 
            #else
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            #endif
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x91); 
            i2cWrite_NT99141(0x32C9, 0x6A); 
            i2cWrite_NT99141(0x32CA, 0x8A); 
            i2cWrite_NT99141(0x32CB, 0x8A); 
            i2cWrite_NT99141(0x32CC, 0x8A); 
            i2cWrite_NT99141(0x32CD, 0x8A); 
            i2cWrite_NT99141(0x32DB, 0x72);     
        }
        #else //use old parameter in 2014
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x70); 
            i2cWrite_NT99141(0x32C1,0x70); 
            i2cWrite_NT99141(0x32C2,0x70); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x78); 
            i2cWrite_NT99141(0x32C9,0x70); 
            i2cWrite_NT99141(0x32CA,0x90); 
            i2cWrite_NT99141(0x32CB,0x90); 
            i2cWrite_NT99141(0x32CC,0x90); 
            i2cWrite_NT99141(0x32CD,0x90); 
            i2cWrite_NT99141(0x32DB,0x6E);     
        }
        else
        {
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x6A); 
            i2cWrite_NT99141(0x32C1,0x6A); 
            i2cWrite_NT99141(0x32C2,0x6A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x91); 
            i2cWrite_NT99141(0x32C9,0x6A); 
            i2cWrite_NT99141(0x32CA,0x8A); 
            i2cWrite_NT99141(0x32CB,0x8A); 
            i2cWrite_NT99141(0x32CC,0x8A); 
            i2cWrite_NT99141(0x32CD,0x8A); 
            i2cWrite_NT99141(0x32DB,0x72);     
        }
        #endif
    }
    else  /* VGA */
    {
        SetNT99141_VGA_30FPS_TRANWO();
        #if(SENSOR_PARAMETER_VERSION == 1) //use new parameter
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x60); 
            i2cWrite_NT99141(0x32C1, 0x5F); 
            i2cWrite_NT99141(0x32C2, 0x5F); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x28); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x5F); 
            i2cWrite_NT99141(0x32CA, 0x7F); 
            i2cWrite_NT99141(0x32CB, 0x7F); 
            i2cWrite_NT99141(0x32CC, 0x7F); 
            i2cWrite_NT99141(0x32CD, 0x80); 
            i2cWrite_NT99141(0x32DB, 0x77);         
        }
        else
        {
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x5A); 
            i2cWrite_NT99141(0x32C1, 0x5A); 
            i2cWrite_NT99141(0x32C2, 0x5A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x28); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xE0); 
            i2cWrite_NT99141(0x32C9, 0x5A); 
            i2cWrite_NT99141(0x32CA, 0x7A); 
            i2cWrite_NT99141(0x32CB, 0x7A); 
            i2cWrite_NT99141(0x32CC, 0x7A); 
            i2cWrite_NT99141(0x32CD, 0x7A); 
            i2cWrite_NT99141(0x32DB, 0x7B);     
        }    
        #else //use old parameter in 2014
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x60); 
            i2cWrite_NT99141(0x32C1,0x5F); 
            i2cWrite_NT99141(0x32C2,0x5F); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xBB); 
            i2cWrite_NT99141(0x32C9,0x5F); 
            i2cWrite_NT99141(0x32CA,0x7F); 
            i2cWrite_NT99141(0x32CB,0x7F); 
            i2cWrite_NT99141(0x32CC,0x7F); 
            i2cWrite_NT99141(0x32CD,0x80); 
            i2cWrite_NT99141(0x32DB,0x77);         
        }
        else
        {
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x5A); 
            i2cWrite_NT99141(0x32C1,0x5A); 
            i2cWrite_NT99141(0x32C2,0x5A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xE0); 
            i2cWrite_NT99141(0x32C9,0x5A); 
            i2cWrite_NT99141(0x32CA,0x7A); 
            i2cWrite_NT99141(0x32CB,0x7A); 
            i2cWrite_NT99141(0x32CC,0x7A); 
            i2cWrite_NT99141(0x32CD,0x7A); 
            i2cWrite_NT99141(0x32DB,0x7B);     
        }
        #endif
    }
}
#elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    u8 i;

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );


    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)  || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        SetNT99141_720P_15FPS_MAYON();
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
    #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x70); 
            i2cWrite_NT99141(0x32C1,0x70); 
            i2cWrite_NT99141(0x32C2,0x70); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x78); 
            i2cWrite_NT99141(0x32C9,0x70); 
            i2cWrite_NT99141(0x32CA,0x90); 
            i2cWrite_NT99141(0x32CB,0x90); 
            i2cWrite_NT99141(0x32CC,0x90); 
            i2cWrite_NT99141(0x32CD,0x90); 
            i2cWrite_NT99141(0x32DB,0x6E);     
    #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x70);
            i2cWrite_NT99141(0x32C1,0x70);
            i2cWrite_NT99141(0x32C2,0x70);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0x5F);
            i2cWrite_NT99141(0x32C9,0x70);
            i2cWrite_NT99141(0x32CA,0x90);
            i2cWrite_NT99141(0x32CB,0x90);
            i2cWrite_NT99141(0x32CC,0x90);
            i2cWrite_NT99141(0x32CD,0x90);
            i2cWrite_NT99141(0x32DB,0x67);
    #else   // 20140324
            //---- 1280x720_60HZ_AntiFlicker  20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x78); 
            i2cWrite_NT99141(0x32C1, 0x78); 
            i2cWrite_NT99141(0x32C2, 0x78); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x32); 
            i2cWrite_NT99141(0x32C5, 0x2F); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x3D); 
            i2cWrite_NT99141(0x32C9, 0x78); 
            i2cWrite_NT99141(0x32CA, 0x98); 
            i2cWrite_NT99141(0x32CB, 0x98); 
            i2cWrite_NT99141(0x32CC, 0x98); 
            i2cWrite_NT99141(0x32CD, 0x98); 
            i2cWrite_NT99141(0x32DB, 0x5E); 
    #endif
        }
        else    // 50Hz
        {
    #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x6A); 
            i2cWrite_NT99141(0x32C1,0x6A); 
            i2cWrite_NT99141(0x32C2,0x6A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x91); 
            i2cWrite_NT99141(0x32C9,0x6A); 
            i2cWrite_NT99141(0x32CA,0x8A); 
            i2cWrite_NT99141(0x32CB,0x8A); 
            i2cWrite_NT99141(0x32CC,0x8A); 
            i2cWrite_NT99141(0x32CD,0x8A); 
            i2cWrite_NT99141(0x32DB,0x72);     
    #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x6A);
            i2cWrite_NT99141(0x32C1,0x6A);
            i2cWrite_NT99141(0x32C2,0x6A);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0x72);
            i2cWrite_NT99141(0x32C9,0x6A);
            i2cWrite_NT99141(0x32CA,0x8A);
            i2cWrite_NT99141(0x32CB,0x8A);
            i2cWrite_NT99141(0x32CC,0x8A);
            i2cWrite_NT99141(0x32CD,0x8A);
            i2cWrite_NT99141(0x32DB,0x6C);
    #else   // 20140324
            //---- 1280x720_50HZ_AntiFlicker  20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x74); 
            i2cWrite_NT99141(0x32C1, 0x74); 
            i2cWrite_NT99141(0x32C2, 0x74); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x32); 
            i2cWrite_NT99141(0x32C5, 0x2F); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x4A); 
            i2cWrite_NT99141(0x32C9, 0x74); 
            i2cWrite_NT99141(0x32CA, 0x94); 
            i2cWrite_NT99141(0x32CB, 0x94); 
            i2cWrite_NT99141(0x32CC, 0x94); 
            i2cWrite_NT99141(0x32CD, 0x94); 
            i2cWrite_NT99141(0x32DB, 0x62); 
    #endif
        }
    }
    else  /* VGA */
    {
        SetNT99141_VGA_30FPS_MAYON();
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
    #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x60); 
            i2cWrite_NT99141(0x32C1,0x5F); 
            i2cWrite_NT99141(0x32C2,0x5F); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xBB); 
            i2cWrite_NT99141(0x32C9,0x5F); 
            i2cWrite_NT99141(0x32CA,0x7F); 
            i2cWrite_NT99141(0x32CB,0x7F); 
            i2cWrite_NT99141(0x32CC,0x7F); 
            i2cWrite_NT99141(0x32CD,0x80); 
            i2cWrite_NT99141(0x32DB,0x77);         
    #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x60);
            i2cWrite_NT99141(0x32C1,0x5F);
            i2cWrite_NT99141(0x32C2,0x5F);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0xB9);
            i2cWrite_NT99141(0x32C9,0x5F);
            i2cWrite_NT99141(0x32CA,0x7F);
            i2cWrite_NT99141(0x32CB,0x7F);
            i2cWrite_NT99141(0x32CC,0x7F);
            i2cWrite_NT99141(0x32CD,0x80);
            i2cWrite_NT99141(0x32DB,0x77);
    #else   // 20140324
            //---- 640x480_60HZ_AntiFlicker 20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x63); 
            i2cWrite_NT99141(0x32C1, 0x63); 
            i2cWrite_NT99141(0x32C2, 0x63); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x32); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x63); 
            i2cWrite_NT99141(0x32CA, 0x83); 
            i2cWrite_NT99141(0x32CB, 0x83); 
            i2cWrite_NT99141(0x32CC, 0x83); 
            i2cWrite_NT99141(0x32CD, 0x83); 
            i2cWrite_NT99141(0x32DB, 0x77); 
        #if(FORECE_MPEG_DROP_1_10_FPS)
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA);
        #endif
    #endif
        }
        else    // 50Hz
        {
    #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x5A); 
            i2cWrite_NT99141(0x32C1,0x5A); 
            i2cWrite_NT99141(0x32C2,0x5A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xE0); 
            i2cWrite_NT99141(0x32C9,0x5A); 
            i2cWrite_NT99141(0x32CA,0x7A); 
            i2cWrite_NT99141(0x32CB,0x7A); 
            i2cWrite_NT99141(0x32CC,0x7A); 
            i2cWrite_NT99141(0x32CD,0x7A); 
            i2cWrite_NT99141(0x32DB,0x7B);     
    #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x5A);
            i2cWrite_NT99141(0x32C1,0x5A);
            i2cWrite_NT99141(0x32C2,0x5A);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0xDE);
            i2cWrite_NT99141(0x32C9,0x5A);
            i2cWrite_NT99141(0x32CA,0x7A);
            i2cWrite_NT99141(0x32CB,0x7A);
            i2cWrite_NT99141(0x32CC,0x7A);
            i2cWrite_NT99141(0x32CD,0x7A);
            i2cWrite_NT99141(0x32DB,0x7B);
    #else   // 20140324
            //---- 640x480_50HZ_AntiFlicker 20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x60); 
            i2cWrite_NT99141(0x32C1, 0x60); 
            i2cWrite_NT99141(0x32C2, 0x60); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x32); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xE0); 
            i2cWrite_NT99141(0x32C9, 0x60); 
            i2cWrite_NT99141(0x32CA, 0x80); 
            i2cWrite_NT99141(0x32CB, 0x80); 
            i2cWrite_NT99141(0x32CC, 0x80); 
            i2cWrite_NT99141(0x32CD, 0x80); 
            i2cWrite_NT99141(0x32DB, 0x7B); 
        #if(FORECE_MPEG_DROP_1_10_FPS)
            i2cWrite_NT99141(0x300C, 0x03); 
            i2cWrite_NT99141(0x300D, 0x7F);
        #endif
    #endif
        }    
    }

}

#elif((HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||  (HW_BOARD_OPTION == MR8120_TX_MA8806))
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    u8 i;

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );


    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        SetNT99141_720P_15FPS_MAYON();
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
        #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x70); 
            i2cWrite_NT99141(0x32C1,0x70); 
            i2cWrite_NT99141(0x32C2,0x70); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x78); 
            i2cWrite_NT99141(0x32C9,0x70); 
            i2cWrite_NT99141(0x32CA,0x90); 
            i2cWrite_NT99141(0x32CB,0x90); 
            i2cWrite_NT99141(0x32CC,0x90); 
            i2cWrite_NT99141(0x32CD,0x90); 
            i2cWrite_NT99141(0x32DB,0x6E);     
        #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x70);
            i2cWrite_NT99141(0x32C1,0x70);
            i2cWrite_NT99141(0x32C2,0x70);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0x5F);
            i2cWrite_NT99141(0x32C9,0x70);
            i2cWrite_NT99141(0x32CA,0x90);
            i2cWrite_NT99141(0x32CB,0x90);
            i2cWrite_NT99141(0x32CC,0x90);
            i2cWrite_NT99141(0x32CD,0x90);
            i2cWrite_NT99141(0x32DB,0x67);
        #else   // 20140324
            //---- 1280x720_60HZ_AntiFlicker  20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x74); 
            i2cWrite_NT99141(0x32C1, 0x73); 
            i2cWrite_NT99141(0x32C2, 0x73); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x78); 
            i2cWrite_NT99141(0x32C9, 0x73); 
            i2cWrite_NT99141(0x32CA, 0x93); 
            i2cWrite_NT99141(0x32CB, 0x93); 
            i2cWrite_NT99141(0x32CC, 0x93); 
            i2cWrite_NT99141(0x32CD, 0x94); 
            i2cWrite_NT99141(0x32DB, 0x6E); 
        #endif
        }
        else    // 50Hz
        {
        #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x6A); 
            i2cWrite_NT99141(0x32C1,0x6A); 
            i2cWrite_NT99141(0x32C2,0x6A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0x91); 
            i2cWrite_NT99141(0x32C9,0x6A); 
            i2cWrite_NT99141(0x32CA,0x8A); 
            i2cWrite_NT99141(0x32CB,0x8A); 
            i2cWrite_NT99141(0x32CC,0x8A); 
            i2cWrite_NT99141(0x32CD,0x8A); 
            i2cWrite_NT99141(0x32DB,0x72);     
        #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x6A);
            i2cWrite_NT99141(0x32C1,0x6A);
            i2cWrite_NT99141(0x32C2,0x6A);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0x72);
            i2cWrite_NT99141(0x32C9,0x6A);
            i2cWrite_NT99141(0x32CA,0x8A);
            i2cWrite_NT99141(0x32CB,0x8A);
            i2cWrite_NT99141(0x32CC,0x8A);
            i2cWrite_NT99141(0x32CD,0x8A);
            i2cWrite_NT99141(0x32DB,0x6C);
        #else   // 20140324
            //---- 1280x720_50HZ_AntiFlicker  20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x70); 
            i2cWrite_NT99141(0x32C1, 0x70); 
            i2cWrite_NT99141(0x32C2, 0x70); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x91); 
            i2cWrite_NT99141(0x32C9, 0x70); 
            i2cWrite_NT99141(0x32CA, 0x90); 
            i2cWrite_NT99141(0x32CB, 0x90); 
            i2cWrite_NT99141(0x32CC, 0x90); 
            i2cWrite_NT99141(0x32CD, 0x90); 
            i2cWrite_NT99141(0x32DB, 0x72); 
        #endif
        }
    }
    else  /* VGA */
    {
        SetNT99141_VGA_30FPS_MAYON();
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
        #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x60); 
            i2cWrite_NT99141(0x32C1,0x5F); 
            i2cWrite_NT99141(0x32C2,0x5F); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xBB); 
            i2cWrite_NT99141(0x32C9,0x5F); 
            i2cWrite_NT99141(0x32CA,0x7F); 
            i2cWrite_NT99141(0x32CB,0x7F); 
            i2cWrite_NT99141(0x32CC,0x7F); 
            i2cWrite_NT99141(0x32CD,0x80); 
            i2cWrite_NT99141(0x32DB,0x77);         
        #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x60);
            i2cWrite_NT99141(0x32C1,0x5F);
            i2cWrite_NT99141(0x32C2,0x5F);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0xB9);
            i2cWrite_NT99141(0x32C9,0x5F);
            i2cWrite_NT99141(0x32CA,0x7F);
            i2cWrite_NT99141(0x32CB,0x7F);
            i2cWrite_NT99141(0x32CC,0x7F);
            i2cWrite_NT99141(0x32CD,0x80);
            i2cWrite_NT99141(0x32DB,0x77);
        #else   // 20140324
            //---- 640x480_60HZ_AntiFlicker 20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x63); 
            i2cWrite_NT99141(0x32C1, 0x63); 
            i2cWrite_NT99141(0x32C2, 0x63); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x63); 
            i2cWrite_NT99141(0x32CA, 0x83); 
            i2cWrite_NT99141(0x32CB, 0x83); 
            i2cWrite_NT99141(0x32CC, 0x83); 
            i2cWrite_NT99141(0x32CD, 0x83); 
            i2cWrite_NT99141(0x32DB, 0x77); 
            #if(FORECE_MPEG_DROP_1_10_FPS)
            i2cWrite_NT99141(0x300C, 0x02); 
			i2cWrite_NT99141(0x300D, 0xEA);
            #endif
        #endif
        }
        else    // 50Hz
        {
        #if 0
            i2cWrite_NT99141(0x32BF,0x60); 
            i2cWrite_NT99141(0x32C0,0x5A); 
            i2cWrite_NT99141(0x32C1,0x5A); 
            i2cWrite_NT99141(0x32C2,0x5A); 
            i2cWrite_NT99141(0x32C3,0x00); 
            i2cWrite_NT99141(0x32C4,0x20); 
            i2cWrite_NT99141(0x32C5,0x20); 
            i2cWrite_NT99141(0x32C6,0x20); 
            i2cWrite_NT99141(0x32C7,0x00); 
            i2cWrite_NT99141(0x32C8,0xE0); 
            i2cWrite_NT99141(0x32C9,0x5A); 
            i2cWrite_NT99141(0x32CA,0x7A); 
            i2cWrite_NT99141(0x32CB,0x7A); 
            i2cWrite_NT99141(0x32CC,0x7A); 
            i2cWrite_NT99141(0x32CD,0x7A); 
            i2cWrite_NT99141(0x32DB,0x7B);     
        #elif 0 // 20140320
            i2cWrite_NT99141(0x32BF,0x60);
            i2cWrite_NT99141(0x32C0,0x5A);
            i2cWrite_NT99141(0x32C1,0x5A);
            i2cWrite_NT99141(0x32C2,0x5A);
            i2cWrite_NT99141(0x32C3,0x00);
            i2cWrite_NT99141(0x32C4,0x30);
            i2cWrite_NT99141(0x32C5,0x20);
            i2cWrite_NT99141(0x32C6,0x20);
            i2cWrite_NT99141(0x32C7,0x00);
            i2cWrite_NT99141(0x32C8,0xDE);
            i2cWrite_NT99141(0x32C9,0x5A);
            i2cWrite_NT99141(0x32CA,0x7A);
            i2cWrite_NT99141(0x32CB,0x7A);
            i2cWrite_NT99141(0x32CC,0x7A);
            i2cWrite_NT99141(0x32CD,0x7A);
            i2cWrite_NT99141(0x32DB,0x7B);
        #else   // 20140324
            //---- 640x480_50HZ_AntiFlicker 20140324 ----//
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x60); 
            i2cWrite_NT99141(0x32C1, 0x60); 
            i2cWrite_NT99141(0x32C2, 0x60); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xE0); 
            i2cWrite_NT99141(0x32C9, 0x60); 
            i2cWrite_NT99141(0x32CA, 0x80); 
            i2cWrite_NT99141(0x32CB, 0x80); 
            i2cWrite_NT99141(0x32CC, 0x80); 
            i2cWrite_NT99141(0x32CD, 0x80); 
            i2cWrite_NT99141(0x32DB, 0x7B); 
            #if(FORECE_MPEG_DROP_1_10_FPS)
            i2cWrite_NT99141(0x300C, 0x03); 
			i2cWrite_NT99141(0x300D, 0x7F);
            #endif
        #endif
        }    
    }

}

#elif(HW_BOARD_OPTION == MR8120_TX_RDI)

// 20140707
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x70);
            i2cWrite_NT99141(0x32C1, 0x70);
            i2cWrite_NT99141(0x32C2, 0x70);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x2F);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0x78);
            i2cWrite_NT99141(0x32C9, 0x70);
            i2cWrite_NT99141(0x32CA, 0x90);
            i2cWrite_NT99141(0x32CB, 0x90);
            i2cWrite_NT99141(0x32CC, 0x90);
            i2cWrite_NT99141(0x32CD, 0x90);
            i2cWrite_NT99141(0x32DB, 0x6E);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x07);
            i2cWrite_NT99141(0x3029, 0x00);
            i2cWrite_NT99141(0x302A, 0x04);
            i2cWrite_NT99141(0x3022, 0x24); 
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0x04);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x05);
            i2cWrite_NT99141(0x3007, 0x03);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x06);
            i2cWrite_NT99141(0x300B, 0x7C);
            i2cWrite_NT99141(0x300C, 0x03);
            i2cWrite_NT99141(0x300D, 0xC3);
            i2cWrite_NT99141(0x300E, 0x05);
            i2cWrite_NT99141(0x300F, 0x00);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x3F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        }
        else //50Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x6A); 
            i2cWrite_NT99141(0x32C1, 0x6A); 
            i2cWrite_NT99141(0x32C2, 0x6A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x2F); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x91); 
            i2cWrite_NT99141(0x32C9, 0x6A); 
            i2cWrite_NT99141(0x32CA, 0x8A); 
            i2cWrite_NT99141(0x32CB, 0x8A); 
            i2cWrite_NT99141(0x32CC, 0x8A); 
            i2cWrite_NT99141(0x32CD, 0x8A); 
            i2cWrite_NT99141(0x32DB, 0x72); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x07); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
            i2cWrite_NT99141(0x3022, 0x24); 
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0x04); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x05); 
            i2cWrite_NT99141(0x3007, 0x03); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x06); 
            i2cWrite_NT99141(0x300B, 0x7C); 
            i2cWrite_NT99141(0x300C, 0x03); 
            i2cWrite_NT99141(0x300D, 0xC3); 
            i2cWrite_NT99141(0x300E, 0x05); 
            i2cWrite_NT99141(0x300F, 0x00); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x3F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
        }
    }
    else    // VGA
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60); //60 HZ 20FPS~30FPS
            i2cWrite_NT99141(0x32C0, 0x68); 
            i2cWrite_NT99141(0x32C1, 0x68); 
            i2cWrite_NT99141(0x32C2, 0x68); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x2F); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x68); 
            i2cWrite_NT99141(0x32CA, 0x88); 
            i2cWrite_NT99141(0x32CB, 0x88); 
            i2cWrite_NT99141(0x32CC, 0x88); 
            i2cWrite_NT99141(0x32CD, 0x88); 
            i2cWrite_NT99141(0x32DB, 0x77); 
            i2cWrite_NT99141(0x32E0, 0x02); 
            i2cWrite_NT99141(0x32E1, 0x80); 
            i2cWrite_NT99141(0x32E2, 0x01); 
            i2cWrite_NT99141(0x32E3, 0xE0); 
            i2cWrite_NT99141(0x32E4, 0x00); 
            i2cWrite_NT99141(0x32E5, 0x80); 
            i2cWrite_NT99141(0x32E6, 0x00); 
            i2cWrite_NT99141(0x32E7, 0x80); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
            i2cWrite_NT99141(0x3022, 0x24); 
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0xA4); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x04); 
            i2cWrite_NT99141(0x3007, 0x63); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x05); 
            i2cWrite_NT99141(0x300B, 0x3C); 
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA); 
            i2cWrite_NT99141(0x300E, 0x03); 
            i2cWrite_NT99141(0x300F, 0xC0); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x7F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01);  
        }
        else //50Hz
        {
            i2cWrite_NT99141(0x32BF, 0x60);  //50 HZ 20FPS~30FPS
            i2cWrite_NT99141(0x32C0, 0x63); 
            i2cWrite_NT99141(0x32C1, 0x64); 
            i2cWrite_NT99141(0x32C2, 0x64); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x2F); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xE0); 
            i2cWrite_NT99141(0x32C9, 0x64); 
            i2cWrite_NT99141(0x32CA, 0x84); 
            i2cWrite_NT99141(0x32CB, 0x84); 
            i2cWrite_NT99141(0x32CC, 0x84); 
            i2cWrite_NT99141(0x32CD, 0x83); 
            i2cWrite_NT99141(0x32DB, 0x7B); 
            i2cWrite_NT99141(0x32E0, 0x02); 
            i2cWrite_NT99141(0x32E1, 0x80); 
            i2cWrite_NT99141(0x32E2, 0x01); 
            i2cWrite_NT99141(0x32E3, 0xE0); 
            i2cWrite_NT99141(0x32E4, 0x00); 
            i2cWrite_NT99141(0x32E5, 0x80); 
            i2cWrite_NT99141(0x32E6, 0x00); 
            i2cWrite_NT99141(0x32E7, 0x80); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
            i2cWrite_NT99141(0x3022, 0x24); 
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0xA4); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x04); 
            i2cWrite_NT99141(0x3007, 0x63); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x05); 
            i2cWrite_NT99141(0x300B, 0x3C); 
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA); 
            i2cWrite_NT99141(0x300E, 0x03); 
            i2cWrite_NT99141(0x300F, 0xC0); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x7F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
        }
    }

    return 0;
}


#else   // 20130930

s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // 720P
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            //PCLK=40MHz
        #if 0
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x70); 
            i2cWrite_NT99141(0x32C1, 0x70); 
            i2cWrite_NT99141(0x32C2, 0x70); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x40); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x64); 
            i2cWrite_NT99141(0x32C9, 0x70); 
            i2cWrite_NT99141(0x32CA, 0x90); 
            i2cWrite_NT99141(0x32CB, 0x90); 
            i2cWrite_NT99141(0x32CC, 0x90); 
            i2cWrite_NT99141(0x32CD, 0x90); 
            i2cWrite_NT99141(0x32DB, 0x69); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x02); 
            i2cWrite_NT99141(0x302A, 0x00);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif            
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0x04); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x05); 
            i2cWrite_NT99141(0x3007, 0x03); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x06); 
            i2cWrite_NT99141(0x300B, 0x7C); 
            i2cWrite_NT99141(0x300C, 0x03); 
            i2cWrite_NT99141(0x300D, 0x23); 
            i2cWrite_NT99141(0x300E, 0x05); 
            i2cWrite_NT99141(0x300F, 0x00); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x3F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
        #else   // 20140224
            i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x70);
            i2cWrite_NT99141(0x32C1, 0x70);
            i2cWrite_NT99141(0x32C2, 0x70);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x20);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0x78);
            i2cWrite_NT99141(0x32C9, 0x70);
            i2cWrite_NT99141(0x32CA, 0x90);
            i2cWrite_NT99141(0x32CB, 0x90);
            i2cWrite_NT99141(0x32CC, 0x90);
            i2cWrite_NT99141(0x32CD, 0x90);
            i2cWrite_NT99141(0x32DB, 0x6E);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x07);
            i2cWrite_NT99141(0x3029, 0x00);
            i2cWrite_NT99141(0x302A, 0x04);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif            
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0x04);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x05);
            i2cWrite_NT99141(0x3007, 0x03);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x06);
            i2cWrite_NT99141(0x300B, 0x7C);
            i2cWrite_NT99141(0x300C, 0x03);
            i2cWrite_NT99141(0x300D, 0xC3);
            i2cWrite_NT99141(0x300E, 0x05);
            i2cWrite_NT99141(0x300F, 0x00);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x3F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        #endif
        }
        else //50Hz
        {
            //PCLK=40MHz
        #if 0
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x6A); 
            i2cWrite_NT99141(0x32C1, 0x6A); 
            i2cWrite_NT99141(0x32C2, 0x6A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x40); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x78); 
            i2cWrite_NT99141(0x32C9, 0x6A); 
            i2cWrite_NT99141(0x32CA, 0x8A); 
            i2cWrite_NT99141(0x32CB, 0x8A); 
            i2cWrite_NT99141(0x32CC, 0x8A); 
            i2cWrite_NT99141(0x32CD, 0x8A); 
            i2cWrite_NT99141(0x32DB, 0x6E); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x02); 
            i2cWrite_NT99141(0x302A, 0x00); 
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif   
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0x04); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x05); 
            i2cWrite_NT99141(0x3007, 0x03); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x06); 
            i2cWrite_NT99141(0x300B, 0x7C); 
            i2cWrite_NT99141(0x300C, 0x03); 
            i2cWrite_NT99141(0x300D, 0x23); 
            i2cWrite_NT99141(0x300E, 0x05); 
            i2cWrite_NT99141(0x300F, 0x00); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x3F); 
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01); 
        #else   // 20140224
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x6A); 
            i2cWrite_NT99141(0x32C1, 0x6A); 
            i2cWrite_NT99141(0x32C2, 0x6A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0x91); 
            i2cWrite_NT99141(0x32C9, 0x6A); 
            i2cWrite_NT99141(0x32CA, 0x8A); 
            i2cWrite_NT99141(0x32CB, 0x8A); 
            i2cWrite_NT99141(0x32CC, 0x8A); 
            i2cWrite_NT99141(0x32CD, 0x8A); 
            i2cWrite_NT99141(0x32DB, 0x72); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x07); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif   
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0x04); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x05); 
            i2cWrite_NT99141(0x3007, 0x03); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x06); 
            i2cWrite_NT99141(0x300B, 0x7C); 
            i2cWrite_NT99141(0x300C, 0x03); 
            i2cWrite_NT99141(0x300D, 0xC3); 
            i2cWrite_NT99141(0x300E, 0x05); 
            i2cWrite_NT99141(0x300F, 0x00); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x3F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
    #endif
        }
    }
    else    // VGA
    {
        if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
        {
            //PCLK=60MHz
        #if 0
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x60); 
            i2cWrite_NT99141(0x32C1, 0x5F); 
            i2cWrite_NT99141(0x32C2, 0x5F); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x40); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x5F); 
            i2cWrite_NT99141(0x32CA, 0x7F); 
            i2cWrite_NT99141(0x32CB, 0x7F); 
            i2cWrite_NT99141(0x32CC, 0x7F); 
            i2cWrite_NT99141(0x32CD, 0x80); 
            i2cWrite_NT99141(0x32DB, 0x77); 
            i2cWrite_NT99141(0x32E0, 0x02); 
            i2cWrite_NT99141(0x32E1, 0x80); 
            i2cWrite_NT99141(0x32E2, 0x01); 
            i2cWrite_NT99141(0x32E3, 0xE0); 
            i2cWrite_NT99141(0x32E4, 0x00); 
            i2cWrite_NT99141(0x32E5, 0x80); 
            i2cWrite_NT99141(0x32E6, 0x00); 
            i2cWrite_NT99141(0x32E7, 0x80); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0xA4); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x04); 
            i2cWrite_NT99141(0x3007, 0x63); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x05); 
            i2cWrite_NT99141(0x300B, 0x3C); 
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA); 
            i2cWrite_NT99141(0x300E, 0x03); 
            i2cWrite_NT99141(0x300F, 0xC0); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x7F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01);
        #else   // 20140224
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x60); 
            i2cWrite_NT99141(0x32C1, 0x5F); 
            i2cWrite_NT99141(0x32C2, 0x5F); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x30); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xBB); 
            i2cWrite_NT99141(0x32C9, 0x5F); 
            i2cWrite_NT99141(0x32CA, 0x7F); 
            i2cWrite_NT99141(0x32CB, 0x7F); 
            i2cWrite_NT99141(0x32CC, 0x7F); 
            i2cWrite_NT99141(0x32CD, 0x80); 
            i2cWrite_NT99141(0x32DB, 0x77); 
            i2cWrite_NT99141(0x32E0, 0x02); 
            i2cWrite_NT99141(0x32E1, 0x80); 
            i2cWrite_NT99141(0x32E2, 0x01); 
            i2cWrite_NT99141(0x32E3, 0xE0); 
            i2cWrite_NT99141(0x32E4, 0x00); 
            i2cWrite_NT99141(0x32E5, 0x80); 
            i2cWrite_NT99141(0x32E6, 0x00); 
            i2cWrite_NT99141(0x32E7, 0x80); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif  
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0xA4); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x04); 
            i2cWrite_NT99141(0x3007, 0x63); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x05); 
            i2cWrite_NT99141(0x300B, 0x3C); 
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA); 
            i2cWrite_NT99141(0x300E, 0x03); 
            i2cWrite_NT99141(0x300F, 0xC0); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x7F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
        #endif
        }
        else //50Hz
        {
            //PCLK=60 
        #if 0
            i2cWrite_NT99141(0x32BF, 0x60); 
            i2cWrite_NT99141(0x32C0, 0x5A); 
            i2cWrite_NT99141(0x32C1, 0x5A); 
            i2cWrite_NT99141(0x32C2, 0x5A); 
            i2cWrite_NT99141(0x32C3, 0x00); 
            i2cWrite_NT99141(0x32C4, 0x40); 
            i2cWrite_NT99141(0x32C5, 0x20); 
            i2cWrite_NT99141(0x32C6, 0x20); 
            i2cWrite_NT99141(0x32C7, 0x00); 
            i2cWrite_NT99141(0x32C8, 0xE0); 
            i2cWrite_NT99141(0x32C9, 0x5A); 
            i2cWrite_NT99141(0x32CA, 0x7A); 
            i2cWrite_NT99141(0x32CB, 0x7A); 
            i2cWrite_NT99141(0x32CC, 0x7A); 
            i2cWrite_NT99141(0x32CD, 0x7A); 
            i2cWrite_NT99141(0x32DB, 0x7B); 
            i2cWrite_NT99141(0x32E0, 0x02); 
            i2cWrite_NT99141(0x32E1, 0x80); 
            i2cWrite_NT99141(0x32E2, 0x01); 
            i2cWrite_NT99141(0x32E3, 0xE0); 
            i2cWrite_NT99141(0x32E4, 0x00); 
            i2cWrite_NT99141(0x32E5, 0x80); 
            i2cWrite_NT99141(0x32E6, 0x00); 
            i2cWrite_NT99141(0x32E7, 0x80); 
            i2cWrite_NT99141(0x3200, 0x3E); 
            i2cWrite_NT99141(0x3201, 0x0F); 
            i2cWrite_NT99141(0x3028, 0x09); 
            i2cWrite_NT99141(0x3029, 0x00); 
            i2cWrite_NT99141(0x302A, 0x04); 
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif   
            i2cWrite_NT99141(0x3023, 0x24); 
            i2cWrite_NT99141(0x3002, 0x00); 
            i2cWrite_NT99141(0x3003, 0xA4); 
            i2cWrite_NT99141(0x3004, 0x00); 
            i2cWrite_NT99141(0x3005, 0x04); 
            i2cWrite_NT99141(0x3006, 0x04); 
            i2cWrite_NT99141(0x3007, 0x63); 
            i2cWrite_NT99141(0x3008, 0x02); 
            i2cWrite_NT99141(0x3009, 0xD3); 
            i2cWrite_NT99141(0x300A, 0x05); 
            i2cWrite_NT99141(0x300B, 0x3C); 
            i2cWrite_NT99141(0x300C, 0x02); 
            i2cWrite_NT99141(0x300D, 0xEA); 
            i2cWrite_NT99141(0x300E, 0x03); 
            i2cWrite_NT99141(0x300F, 0xC0); 
            i2cWrite_NT99141(0x3010, 0x02); 
            i2cWrite_NT99141(0x3011, 0xD0); 
            i2cWrite_NT99141(0x32B8, 0x3F); 
            i2cWrite_NT99141(0x32B9, 0x31); 
            i2cWrite_NT99141(0x32BB, 0x87); 
            i2cWrite_NT99141(0x32BC, 0x38); 
            i2cWrite_NT99141(0x32BD, 0x3C); 
            i2cWrite_NT99141(0x32BE, 0x34); 
            i2cWrite_NT99141(0x3201, 0x7F); 
            i2cWrite_NT99141(0x3021, 0x06); 
            i2cWrite_NT99141(0x3060, 0x01); 
        #else   // 20140224
        	i2cWrite_NT99141(0x32BF, 0x60);
            i2cWrite_NT99141(0x32C0, 0x5A);
            i2cWrite_NT99141(0x32C1, 0x5A);
            i2cWrite_NT99141(0x32C2, 0x5A);
            i2cWrite_NT99141(0x32C3, 0x00);
            i2cWrite_NT99141(0x32C4, 0x30);
            i2cWrite_NT99141(0x32C5, 0x20);
            i2cWrite_NT99141(0x32C6, 0x20);
            i2cWrite_NT99141(0x32C7, 0x00);
            i2cWrite_NT99141(0x32C8, 0xE0);
            i2cWrite_NT99141(0x32C9, 0x5A);
            i2cWrite_NT99141(0x32CA, 0x7A);
            i2cWrite_NT99141(0x32CB, 0x7A);
            i2cWrite_NT99141(0x32CC, 0x7A);
            i2cWrite_NT99141(0x32CD, 0x7A);
            i2cWrite_NT99141(0x32DB, 0x7B);
            i2cWrite_NT99141(0x32E0, 0x02);
            i2cWrite_NT99141(0x32E1, 0x80);
            i2cWrite_NT99141(0x32E2, 0x01);
            i2cWrite_NT99141(0x32E3, 0xE0);
            i2cWrite_NT99141(0x32E4, 0x00);
            i2cWrite_NT99141(0x32E5, 0x80);
            i2cWrite_NT99141(0x32E6, 0x00);
            i2cWrite_NT99141(0x32E7, 0x80);
            i2cWrite_NT99141(0x3200, 0x3E);
            i2cWrite_NT99141(0x3201, 0x0F);
            i2cWrite_NT99141(0x3028, 0x09);
            i2cWrite_NT99141(0x3029, 0x00);
            i2cWrite_NT99141(0x302A, 0x04);
        #if(SENSOR_ROW_COL_MIRROR)
            i2cWrite_NT99141(0x3022, 0x27);     
        #else
            i2cWrite_NT99141(0x3022, 0x24); 
        #endif   
            i2cWrite_NT99141(0x3023, 0x24);
            i2cWrite_NT99141(0x3002, 0x00);
            i2cWrite_NT99141(0x3003, 0xA4);
            i2cWrite_NT99141(0x3004, 0x00);
            i2cWrite_NT99141(0x3005, 0x04);
            i2cWrite_NT99141(0x3006, 0x04);
            i2cWrite_NT99141(0x3007, 0x63);
            i2cWrite_NT99141(0x3008, 0x02);
            i2cWrite_NT99141(0x3009, 0xD3);
            i2cWrite_NT99141(0x300A, 0x05);
            i2cWrite_NT99141(0x300B, 0x3C);
            i2cWrite_NT99141(0x300C, 0x02);
            i2cWrite_NT99141(0x300D, 0xEA);
            i2cWrite_NT99141(0x300E, 0x03);
            i2cWrite_NT99141(0x300F, 0xC0);
            i2cWrite_NT99141(0x3010, 0x02);
            i2cWrite_NT99141(0x3011, 0xD0);
            i2cWrite_NT99141(0x32B8, 0x3F);
            i2cWrite_NT99141(0x32B9, 0x31);
            i2cWrite_NT99141(0x32BB, 0x87);
            i2cWrite_NT99141(0x32BC, 0x38);
            i2cWrite_NT99141(0x32BD, 0x3C);
            i2cWrite_NT99141(0x32BE, 0x34);
            i2cWrite_NT99141(0x3201, 0x7F);
            i2cWrite_NT99141(0x3021, 0x06);
            i2cWrite_NT99141(0x3060, 0x01);
        #endif
        }
    }

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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
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
    static int  Mode    = -1;

	while (1)
	{
		OSSemPend(siuSemEvt, OS_IPC_WAIT_FOREVER, &err);

		if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: siuSemEvt is %d.\n", err);
			//return ;
		}


    #if HW_MD_SUPPORT
        if(MotionDetect_en)
           MotionDetect_API(MD_SIU_ID);
    #endif

#if 0   // move to ciuTask_CH1(), Peter
    #if(HW_BOARD_OPTION == MR8120_TX_RDI)
        gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level);

        if(Mode != level)
        {
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
        }
    #elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
        if(Main_Init_Ready)
        {
            gpioGetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,&level);
            if(Mode != level)
            {
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
            }
        }
    #elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
          (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
        gpioDetectIRLED();
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

#if(HW_BOARD_OPTION == MR8120_TX_RDI)
void siuSetSensorDayNight(u8 Level)
{

    if(Level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
	{   //夜間模式
        DEBUG_SIU("##enter night\n");
    #if 0
        i2cWrite_NT99141(0x3053 ,0x0C); 
        //i2cWrite_NT99141(0x32f2 ,0xA0);
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] + 0x20);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x38);
        i2cWrite_NT99141(0x32F1 ,0x01);
        i2cWrite_NT99141(0x3060 ,0x01);
    #elif 0   // 20140224
        i2cWrite_NT99141(0x3053, 0x0C);
        i2cWrite_NT99141(0x32f2, AETargetMeanTab[siuY_TargetIndex] + 0x20);
        i2cWrite_NT99141(0x32fc, 0x0D);
        i2cWrite_NT99141(0x32f8, 0x01);
        i2cWrite_NT99141(0x336D, 0x24);
        i2cWrite_NT99141(0x336E, 0x1F);
        i2cWrite_NT99141(0x336F, 0x19);
        i2cWrite_NT99141(0x3370, 0x13);
        i2cWrite_NT99141(0x32C4, 0x38);
        i2cWrite_NT99141(0x32F1, 0x01);
    	i2cWrite_NT99141(0x3270, 0x03);
        i2cWrite_NT99141(0x3271, 0x0F);
        i2cWrite_NT99141(0x3272, 0x1B);
        i2cWrite_NT99141(0x3273, 0x2F);
        i2cWrite_NT99141(0x3274, 0x45);
        i2cWrite_NT99141(0x3275, 0x57);
        i2cWrite_NT99141(0x3276, 0x72);
        i2cWrite_NT99141(0x3277, 0x88);
        i2cWrite_NT99141(0x3278, 0x98);
        i2cWrite_NT99141(0x3279, 0xA8);
        i2cWrite_NT99141(0x327A, 0xBE);
        i2cWrite_NT99141(0x327B, 0xCE);
        i2cWrite_NT99141(0x327C, 0xDB);
        i2cWrite_NT99141(0x327D, 0xE6);
        i2cWrite_NT99141(0x327E, 0xED);
        i2cWrite_NT99141(0x3060, 0x01);
    #elif 0   // 20140708
        i2cWrite_NT99141(0x3331, 0x0A);
    	  
        i2cWrite_NT99141(0x3270, 0x04); //Gamma_YIWEI_Tune     
        i2cWrite_NT99141(0x3271, 0x14);  
        i2cWrite_NT99141(0x3272, 0x22);  
        i2cWrite_NT99141(0x3273, 0x37);  
        i2cWrite_NT99141(0x3274, 0x49);  
        i2cWrite_NT99141(0x3275, 0x5a);  
        i2cWrite_NT99141(0x3276, 0x76);  
        i2cWrite_NT99141(0x3277, 0x8b);  
        i2cWrite_NT99141(0x3278, 0x9F);  
        i2cWrite_NT99141(0x3279, 0xB0);  
        i2cWrite_NT99141(0x327A, 0xcd);  
        i2cWrite_NT99141(0x327B, 0xe2);  
        i2cWrite_NT99141(0x327C, 0xef);  
        i2cWrite_NT99141(0x327D, 0xF9);  
        i2cWrite_NT99141(0x327E, 0xFF);  
        
        i2cWrite_NT99141(0x3302, 0x00);  //No CC
        i2cWrite_NT99141(0x3303, 0x4C);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0x96);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1D);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xD5);  
        i2cWrite_NT99141(0x330A, 0x07);  
        i2cWrite_NT99141(0x330B, 0xAC);  
        i2cWrite_NT99141(0x330C, 0x00);  
        i2cWrite_NT99141(0x330D, 0x80);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0x80);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x95);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xEC); 
        
        i2cWrite_NT99141(0x3060, 0x01); 	
    #elif 0   // 20140710
    	i2cWrite_NT99141(0x3331, 0x0A);
    	i2cWrite_NT99141(0x32C4, 0x37);
    	i2cWrite_NT99141(0x3053, 0x0D);
          
    	i2cWrite_NT99141(0x3270, 0x04); //Gamma_YIWEI_Tune     
        i2cWrite_NT99141(0x3271, 0x14);  
        i2cWrite_NT99141(0x3272, 0x22);  
        i2cWrite_NT99141(0x3273, 0x37);  
        i2cWrite_NT99141(0x3274, 0x49);  
        i2cWrite_NT99141(0x3275, 0x5a);  
        i2cWrite_NT99141(0x3276, 0x76);  
        i2cWrite_NT99141(0x3277, 0x8b);  
        i2cWrite_NT99141(0x3278, 0x9F);  
        i2cWrite_NT99141(0x3279, 0xB0);  
        i2cWrite_NT99141(0x327A, 0xcd);  
        i2cWrite_NT99141(0x327B, 0xe2);  
        i2cWrite_NT99141(0x327C, 0xef);  
        i2cWrite_NT99141(0x327D, 0xF9);  
        i2cWrite_NT99141(0x327E, 0xFF);  
        
        i2cWrite_NT99141(0x3302, 0x00);  //No CC
        i2cWrite_NT99141(0x3303, 0x4C);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0x96);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1D);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xD5);  
        i2cWrite_NT99141(0x330A, 0x07);  
        i2cWrite_NT99141(0x330B, 0xAC);  
        i2cWrite_NT99141(0x330C, 0x00);  
        i2cWrite_NT99141(0x330D, 0x80);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0x80);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x95);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xEC); 
        
        i2cWrite_NT99141(0x3060, 0x01); 	
    #else   // 20140717
        i2cWrite_NT99141(0x32C4, 0x37);
        i2cWrite_NT99141(0x3053, 0x0D);
          
        i2cWrite_NT99141(0x3270, 0x14); //Gamma_13      
        i2cWrite_NT99141(0x3271, 0x1E);                 
        i2cWrite_NT99141(0x3272, 0x28);                 
        i2cWrite_NT99141(0x3273, 0x44);                 
        i2cWrite_NT99141(0x3274, 0x5D);                 
        i2cWrite_NT99141(0x3275, 0x72);                 
        i2cWrite_NT99141(0x3276, 0x95);                 
        i2cWrite_NT99141(0x3277, 0xB1);                 
        i2cWrite_NT99141(0x3278, 0xC6);                 
        i2cWrite_NT99141(0x3279, 0xD5);                 
        i2cWrite_NT99141(0x327A, 0xEA);                 
        i2cWrite_NT99141(0x327B, 0xF5);                 
        i2cWrite_NT99141(0x327C, 0xFB);                 
        i2cWrite_NT99141(0x327D, 0xFE);                 
        i2cWrite_NT99141(0x327E, 0xFF);                 
        
        i2cWrite_NT99141(0x3302, 0x00);  //No CC
        i2cWrite_NT99141(0x3303, 0x4C);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0x96);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1D);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xD5);  
        i2cWrite_NT99141(0x330A, 0x07);  
        i2cWrite_NT99141(0x330B, 0xAC);  
        i2cWrite_NT99141(0x330C, 0x00);  
        i2cWrite_NT99141(0x330D, 0x80);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0x80);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x95);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xEC); 
        
        i2cWrite_NT99141(0x3060, 0x01);     
    #endif
	}
    else
    {   //日間模式
        DEBUG_SIU("##enter day\n");
    #if 0
        i2cWrite_NT99141(0x3053 ,0x10); 
        //i2cWrite_NT99141(0x32f2 ,0x80);
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x30);
        //i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x32F1 ,0x05);     // 改由0x32f2控制亮度
        i2cWrite_NT99141(0x3060 ,0x01);
    #elif 0   // 20140224
        i2cWrite_NT99141(0x3053, 0x10);
        i2cWrite_NT99141(0x32f2, AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc, 0x00);
        i2cWrite_NT99141(0x32f8, 0x10);
        i2cWrite_NT99141(0x336D, 0x20);
        i2cWrite_NT99141(0x336E, 0x1C);
        i2cWrite_NT99141(0x336F, 0x18);
        i2cWrite_NT99141(0x3370, 0x10);
        i2cWrite_NT99141(0x32C4, 0x30);
        i2cWrite_NT99141(0x32F1, 0x00);
    	i2cWrite_NT99141(0x3270, 0x00);
        i2cWrite_NT99141(0x3271, 0x0C);
        i2cWrite_NT99141(0x3272, 0x18);
        i2cWrite_NT99141(0x3273, 0x32);
        i2cWrite_NT99141(0x3274, 0x44);
        i2cWrite_NT99141(0x3275, 0x54);
        i2cWrite_NT99141(0x3276, 0x70);
        i2cWrite_NT99141(0x3277, 0x88);
        i2cWrite_NT99141(0x3278, 0x9D);
        i2cWrite_NT99141(0x3279, 0xB0);
        i2cWrite_NT99141(0x327A, 0xCF);
        i2cWrite_NT99141(0x327B, 0xE2);
        i2cWrite_NT99141(0x327C, 0xEF);
        i2cWrite_NT99141(0x327D, 0xF7);
        i2cWrite_NT99141(0x327E, 0xFF);
        i2cWrite_NT99141(0x3060, 0x01);
    #elif 0   // 20140708
        i2cWrite_NT99141(0x3331, 0x07);
    		  
        i2cWrite_NT99141(0x3270, 0x00);  //Gamma_Mid9712   
        i2cWrite_NT99141(0x3271, 0x05);  
        i2cWrite_NT99141(0x3272, 0x11);  
        i2cWrite_NT99141(0x3273, 0x28);  
        i2cWrite_NT99141(0x3274, 0x43);  
        i2cWrite_NT99141(0x3275, 0x5C);  
        i2cWrite_NT99141(0x3276, 0x7E);  
        i2cWrite_NT99141(0x3277, 0x9B);  
        i2cWrite_NT99141(0x3278, 0xB3);  
        i2cWrite_NT99141(0x3279, 0xC6);  
        i2cWrite_NT99141(0x327A, 0xD9);  
        i2cWrite_NT99141(0x327B, 0xE7);  
        i2cWrite_NT99141(0x327C, 0xF6);  
        i2cWrite_NT99141(0x327D, 0xFF);  
        i2cWrite_NT99141(0x327E, 0xFF); 
         
        i2cWrite_NT99141(0x3302, 0x00);  //[CC_Matrix_DXWY_11_4]
        i2cWrite_NT99141(0x3303, 0x00);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0xE1);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1E);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xCF);  
        i2cWrite_NT99141(0x330A, 0x06);  
        i2cWrite_NT99141(0x330B, 0x8F);  
        i2cWrite_NT99141(0x330C, 0x01);  
        i2cWrite_NT99141(0x330D, 0xA2);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0xF0);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x19);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xF7);
          
        i2cWrite_NT99141(0x3060, 0x01);
    #elif 0   // 20140710
    	i2cWrite_NT99141(0x3331, 0x07);
    	i2cWrite_NT99141(0x32C4, 0x2F);
    	i2cWrite_NT99141(0x3053, 0x10);
        	  
    	i2cWrite_NT99141(0x3270, 0x00);  //Gamma_Mid9712   
        i2cWrite_NT99141(0x3271, 0x05);  
        i2cWrite_NT99141(0x3272, 0x11);  
        i2cWrite_NT99141(0x3273, 0x28);  
        i2cWrite_NT99141(0x3274, 0x43);  
        i2cWrite_NT99141(0x3275, 0x5C);  
        i2cWrite_NT99141(0x3276, 0x7E);  
        i2cWrite_NT99141(0x3277, 0x9B);  
        i2cWrite_NT99141(0x3278, 0xB3);  
        i2cWrite_NT99141(0x3279, 0xC6);  
        i2cWrite_NT99141(0x327A, 0xD9);  
        i2cWrite_NT99141(0x327B, 0xE7);  
        i2cWrite_NT99141(0x327C, 0xF6);  
        i2cWrite_NT99141(0x327D, 0xFF);  
        i2cWrite_NT99141(0x327E, 0xFF); 
         
        i2cWrite_NT99141(0x3302, 0x00);  //[CC_Matrix_DXWY_11_4]
        i2cWrite_NT99141(0x3303, 0x00);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0xE1);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1E);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xCF);  
        i2cWrite_NT99141(0x330A, 0x06);  
        i2cWrite_NT99141(0x330B, 0x8F);  
        i2cWrite_NT99141(0x330C, 0x01);  
        i2cWrite_NT99141(0x330D, 0xA2);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0xF0);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x19);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xF7);
          
        i2cWrite_NT99141(0x3060, 0x01);
    #else   // 20140717
        i2cWrite_NT99141(0x32C4, 0x2F);
        i2cWrite_NT99141(0x3053, 0x10);
              
        i2cWrite_NT99141(0x3270, 0x00);  //Gamma_Mid9712   
        i2cWrite_NT99141(0x3271, 0x05);  
        i2cWrite_NT99141(0x3272, 0x11);  
        i2cWrite_NT99141(0x3273, 0x28);  
        i2cWrite_NT99141(0x3274, 0x43);  
        i2cWrite_NT99141(0x3275, 0x5C);  
        i2cWrite_NT99141(0x3276, 0x7E);  
        i2cWrite_NT99141(0x3277, 0x9B);  
        i2cWrite_NT99141(0x3278, 0xB3);  
        i2cWrite_NT99141(0x3279, 0xC6);  
        i2cWrite_NT99141(0x327A, 0xD9);  
        i2cWrite_NT99141(0x327B, 0xE7);  
        i2cWrite_NT99141(0x327C, 0xF6);  
        i2cWrite_NT99141(0x327D, 0xFF);  
        i2cWrite_NT99141(0x327E, 0xFF); 
         
        i2cWrite_NT99141(0x3302, 0x00);  //[CC_Matrix_DXWY_11_4]
        i2cWrite_NT99141(0x3303, 0x00);  
        i2cWrite_NT99141(0x3304, 0x00);  
        i2cWrite_NT99141(0x3305, 0xE1);  
        i2cWrite_NT99141(0x3306, 0x00);  
        i2cWrite_NT99141(0x3307, 0x1E);  
        i2cWrite_NT99141(0x3308, 0x07);  
        i2cWrite_NT99141(0x3309, 0xCF);  
        i2cWrite_NT99141(0x330A, 0x06);  
        i2cWrite_NT99141(0x330B, 0x8F);  
        i2cWrite_NT99141(0x330C, 0x01);  
        i2cWrite_NT99141(0x330D, 0xA2);  
        i2cWrite_NT99141(0x330E, 0x00);  
        i2cWrite_NT99141(0x330F, 0xF0);  
        i2cWrite_NT99141(0x3310, 0x07);  
        i2cWrite_NT99141(0x3311, 0x19);  
        i2cWrite_NT99141(0x3312, 0x07);  
        i2cWrite_NT99141(0x3313, 0xF7);
          
        i2cWrite_NT99141(0x3060, 0x01);
    #endif
    }
}
#elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )

void siuSetSensorDayNight(u8 Mode)
{
    if(Mode == SIU_NIGHT_MODE)
    {
    #if 0
        i2cWrite_NT99141(0x3053 ,0x0C); 
        i2cWrite_NT99141(0x32f2 ,0x98);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x38);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);          
    #elif 0 // 20140320
        i2cWrite_NT99141(0x3371,0x26);
        i2cWrite_NT99141(0x3372,0x2A);
        i2cWrite_NT99141(0x3373,0x2D);
        i2cWrite_NT99141(0x3374,0x2D);
        i2cWrite_NT99141(0x3375,0x17);
        i2cWrite_NT99141(0x3376,0x19);
        i2cWrite_NT99141(0x3377,0x1C);
        i2cWrite_NT99141(0x3378,0x21);
        i2cWrite_NT99141(0x3368,0x4E);
        i2cWrite_NT99141(0x3369,0x45);
        i2cWrite_NT99141(0x336A,0x3D);
        i2cWrite_NT99141(0x336B,0x2D);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x0D);
        i2cWrite_NT99141(0x3272,0x19);
        i2cWrite_NT99141(0x3273,0x32);
        i2cWrite_NT99141(0x3274,0x44);
        i2cWrite_NT99141(0x3275,0x55);
        i2cWrite_NT99141(0x3276,0x6E);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0x9D);
        i2cWrite_NT99141(0x3279,0xB1);
        i2cWrite_NT99141(0x327A,0xD0);
        i2cWrite_NT99141(0x327B,0xE1);
        i2cWrite_NT99141(0x327C,0xEE);
        i2cWrite_NT99141(0x327D,0xF7);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32F1,0x01);
        i2cWrite_NT99141(0x32f2,0xAC);
        i2cWrite_NT99141(0x32fc,0x10);
        i2cWrite_NT99141(0x3060,0x01);
    #else   // 20140324
        //---- Night_Mode 20141125 ----//
        i2cWrite_NT99141(0x3363,0x00);       
        i2cWrite_NT99141(0x3331,0x04);  
        i2cWrite_NT99141(0x32c4,0x32);   
        i2cWrite_NT99141(0x3326,0x01);
        i2cWrite_NT99141(0x3327,0x0F);
        i2cWrite_NT99141(0x3328,0x0F);
        i2cWrite_NT99141(0x3371,0x38); 
        i2cWrite_NT99141(0x3372,0x3F); 
        i2cWrite_NT99141(0x3373,0x3F); 
        i2cWrite_NT99141(0x3374,0x3F); 
        i2cWrite_NT99141(0x3375,0x17); 
        i2cWrite_NT99141(0x3376,0x19);
        i2cWrite_NT99141(0x3377,0x1C);
        i2cWrite_NT99141(0x3378,0x21);
        i2cWrite_NT99141(0x3368,0x58);
        i2cWrite_NT99141(0x3369,0x4B);  
				i2cWrite_NT99141(0x336A,0x40);
				i2cWrite_NT99141(0x336B,0x38);
				i2cWrite_NT99141(0x3270,0x08);
				i2cWrite_NT99141(0x3271,0x14);
				i2cWrite_NT99141(0x3272,0x1F);
				i2cWrite_NT99141(0x3273,0x36);
				i2cWrite_NT99141(0x3274,0x4C);
				i2cWrite_NT99141(0x3275,0x61);
				i2cWrite_NT99141(0x3276,0x88);
				i2cWrite_NT99141(0x3277,0xA6);
				i2cWrite_NT99141(0x3278,0xBD);
				i2cWrite_NT99141(0x3279,0xCD);
				i2cWrite_NT99141(0x327A,0xE1);
				i2cWrite_NT99141(0x327B,0xED);
        i2cWrite_NT99141(0x327C,0xF5); 
        i2cWrite_NT99141(0x327D,0xFB);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x3320,0x18);
        i2cWrite_NT99141(0x3321,0x30);   
        i2cWrite_NT99141(0x3322,0x30);
        i2cWrite_NT99141(0x3340,0x18);
        i2cWrite_NT99141(0x32F1,0x01);
        i2cWrite_NT99141(0x32f2,AETargetMeanTab[siuY_TargetIndex]+0x28);
        i2cWrite_NT99141(0x32fc,0x10);
        i2cWrite_NT99141(0x32B8,0x1B);
        i2cWrite_NT99141(0x32B9,0x15);
        i2cWrite_NT99141(0x32BC,0x18);
        i2cWrite_NT99141(0x32BD,0x1A);
        i2cWrite_NT99141(0x32BE,0x16);
        i2cWrite_NT99141(0x3053,0x0C);
        i2cWrite_NT99141(0x3060,0x01);
    #endif
    }
    else    // Day mode
    {
    #if 0
        i2cWrite_NT99141(0x3053 ,0x10); 
        i2cWrite_NT99141(0x32f2 ,0x80);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x30);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);
    #elif 0 // 20140320
        i2cWrite_NT99141(0x3371,0x38);
        i2cWrite_NT99141(0x3372,0x3C);
        i2cWrite_NT99141(0x3373,0x3F);
        i2cWrite_NT99141(0x3374,0x3F);
        i2cWrite_NT99141(0x3375,0x0A);
        i2cWrite_NT99141(0x3376,0x0C);
        i2cWrite_NT99141(0x3377,0x10);
        i2cWrite_NT99141(0x3378,0x14);
        i2cWrite_NT99141(0x3368,0x30);
        i2cWrite_NT99141(0x3369,0x28);
        i2cWrite_NT99141(0x336A,0x20);
        i2cWrite_NT99141(0x336B,0x10);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x04);
        i2cWrite_NT99141(0x3272,0x0E);
        i2cWrite_NT99141(0x3273,0x28);
        i2cWrite_NT99141(0x3274,0x3F);
        i2cWrite_NT99141(0x3275,0x50);
        i2cWrite_NT99141(0x3276,0x6E);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0xA0);
        i2cWrite_NT99141(0x3279,0xB3);
        i2cWrite_NT99141(0x327A,0xD2);
        i2cWrite_NT99141(0x327B,0xE8);
        i2cWrite_NT99141(0x327C,0xF5);
        i2cWrite_NT99141(0x327D,0xFF);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32f2,0x80);
        i2cWrite_NT99141(0x32fc,0x00);
        i2cWrite_NT99141(0x32F1,0x00);
        i2cWrite_NT99141(0x3060,0x01);
    #else   // 20140324
        //---- Day_Mode 20141125 ----//
        i2cWrite_NT99141(0x3363,0x37);       
        i2cWrite_NT99141(0x3331,0x0A);  
        i2cWrite_NT99141(0x32c4,0x32);   
        i2cWrite_NT99141(0x3326,0x01);
        i2cWrite_NT99141(0x3327,0x0A);
        i2cWrite_NT99141(0x3328,0x0A);
        i2cWrite_NT99141(0x3371,0x38); 
        i2cWrite_NT99141(0x3372,0x3C); 
        i2cWrite_NT99141(0x3373,0x3F); 
        i2cWrite_NT99141(0x3374,0x3F); 
        i2cWrite_NT99141(0x3375,0x0A); 
        i2cWrite_NT99141(0x3376,0x0C);
        i2cWrite_NT99141(0x3377,0x10);
        i2cWrite_NT99141(0x3378,0x14);
        i2cWrite_NT99141(0x3368,0x48);
        i2cWrite_NT99141(0x3369,0x38);  
				i2cWrite_NT99141(0x336A,0x30);
				i2cWrite_NT99141(0x336B,0x20);
				i2cWrite_NT99141(0x3270,0x00);
				i2cWrite_NT99141(0x3271,0x0C);
				i2cWrite_NT99141(0x3272,0x18);
				i2cWrite_NT99141(0x3273,0x32);
				i2cWrite_NT99141(0x3274,0x44);
				i2cWrite_NT99141(0x3275,0x54);
				i2cWrite_NT99141(0x3276,0x70);
				i2cWrite_NT99141(0x3277,0x88);
				i2cWrite_NT99141(0x3278,0x9D);
				i2cWrite_NT99141(0x3279,0xB0);
				i2cWrite_NT99141(0x327A,0xCF);
				i2cWrite_NT99141(0x327B,0xE2);
        i2cWrite_NT99141(0x327C,0xEF); 
        i2cWrite_NT99141(0x327D,0xF7);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x3320,0x18);
        i2cWrite_NT99141(0x3321,0x30);   
        i2cWrite_NT99141(0x3322,0x30);
        i2cWrite_NT99141(0x3340,0x18);
        i2cWrite_NT99141(0x32F1,0x00);
        i2cWrite_NT99141(0x32f2,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc,0x00);
        i2cWrite_NT99141(0x32B8,0x3F); 
        i2cWrite_NT99141(0x32B9,0x31); 
        i2cWrite_NT99141(0x32BB,0x87); 
        i2cWrite_NT99141(0x32BC,0x38); 
        i2cWrite_NT99141(0x32BD,0x3C); 
        i2cWrite_NT99141(0x32BE,0x34);
        i2cWrite_NT99141(0x3053,0x10);
        i2cWrite_NT99141(0x3060,0x01);
    #endif
    }
}

#elif ((HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)|| (HW_BOARD_OPTION == MR8120_TX_MA8806))
void siuSetSensorDayNight(u8 Mode)
{
    if(Mode == SIU_NIGHT_MODE)
    {
    #if 0
        i2cWrite_NT99141(0x3053 ,0x0C); 
        i2cWrite_NT99141(0x32f2 ,0x98);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x38);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);          
    #elif 0 // 20140320
        i2cWrite_NT99141(0x3371,0x26);
        i2cWrite_NT99141(0x3372,0x2A);
        i2cWrite_NT99141(0x3373,0x2D);
        i2cWrite_NT99141(0x3374,0x2D);
        i2cWrite_NT99141(0x3375,0x17);
        i2cWrite_NT99141(0x3376,0x19);
        i2cWrite_NT99141(0x3377,0x1C);
        i2cWrite_NT99141(0x3378,0x21);
        i2cWrite_NT99141(0x3368,0x4E);
        i2cWrite_NT99141(0x3369,0x45);
        i2cWrite_NT99141(0x336A,0x3D);
        i2cWrite_NT99141(0x336B,0x2D);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x0D);
        i2cWrite_NT99141(0x3272,0x19);
        i2cWrite_NT99141(0x3273,0x32);
        i2cWrite_NT99141(0x3274,0x44);
        i2cWrite_NT99141(0x3275,0x55);
        i2cWrite_NT99141(0x3276,0x6E);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0x9D);
        i2cWrite_NT99141(0x3279,0xB1);
        i2cWrite_NT99141(0x327A,0xD0);
        i2cWrite_NT99141(0x327B,0xE1);
        i2cWrite_NT99141(0x327C,0xEE);
        i2cWrite_NT99141(0x327D,0xF7);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32F1,0x01);
        i2cWrite_NT99141(0x32f2,0xAC);
        i2cWrite_NT99141(0x32fc,0x10);
        i2cWrite_NT99141(0x3060,0x01);
    #else   // 20140324
        //---- Night_Mode 20140324 ----//
        i2cWrite_NT99141(0x3371,0x38);
        i2cWrite_NT99141(0x3372,0x3C);
        i2cWrite_NT99141(0x3373,0x3F);
        i2cWrite_NT99141(0x3374,0x3F); 
        i2cWrite_NT99141(0x3375,0x17); 
        i2cWrite_NT99141(0x3376,0x19); 
        i2cWrite_NT99141(0x3377,0x1C); 
        i2cWrite_NT99141(0x3378,0x21); 
        i2cWrite_NT99141(0x3368,0x4E);
        i2cWrite_NT99141(0x3369,0x45);
        i2cWrite_NT99141(0x336A,0x3D);
        i2cWrite_NT99141(0x336B,0x2D);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x0D);
        i2cWrite_NT99141(0x3272,0x19);
        i2cWrite_NT99141(0x3273,0x32);
        i2cWrite_NT99141(0x3274,0x44);
        i2cWrite_NT99141(0x3275,0x55);
        i2cWrite_NT99141(0x3276,0x6E);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0x9D);
        i2cWrite_NT99141(0x3279,0xB1);
        i2cWrite_NT99141(0x327A,0xD0);
        i2cWrite_NT99141(0x327B,0xE1);
        i2cWrite_NT99141(0x327C,0xEE);
        i2cWrite_NT99141(0x327D,0xF7);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32F1,0x01);
        i2cWrite_NT99141(0x32f2,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc,0x10);
        i2cWrite_NT99141(0x3053,0x0C);
        i2cWrite_NT99141(0x3060,0x01);
    #endif
    }
    else    // Day mode
    {
    #if 0
        i2cWrite_NT99141(0x3053 ,0x10); 
        i2cWrite_NT99141(0x32f2 ,0x80);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x30);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);
    #elif 0 // 20140320
        i2cWrite_NT99141(0x3371,0x38);
        i2cWrite_NT99141(0x3372,0x3C);
        i2cWrite_NT99141(0x3373,0x3F);
        i2cWrite_NT99141(0x3374,0x3F);
        i2cWrite_NT99141(0x3375,0x0A);
        i2cWrite_NT99141(0x3376,0x0C);
        i2cWrite_NT99141(0x3377,0x10);
        i2cWrite_NT99141(0x3378,0x14);
        i2cWrite_NT99141(0x3368,0x30);
        i2cWrite_NT99141(0x3369,0x28);
        i2cWrite_NT99141(0x336A,0x20);
        i2cWrite_NT99141(0x336B,0x10);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x04);
        i2cWrite_NT99141(0x3272,0x0E);
        i2cWrite_NT99141(0x3273,0x28);
        i2cWrite_NT99141(0x3274,0x3F);
        i2cWrite_NT99141(0x3275,0x50);
        i2cWrite_NT99141(0x3276,0x6E);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0xA0);
        i2cWrite_NT99141(0x3279,0xB3);
        i2cWrite_NT99141(0x327A,0xD2);
        i2cWrite_NT99141(0x327B,0xE8);
        i2cWrite_NT99141(0x327C,0xF5);
        i2cWrite_NT99141(0x327D,0xFF);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32f2,0x80);
        i2cWrite_NT99141(0x32fc,0x00);
        i2cWrite_NT99141(0x32F1,0x00);
        i2cWrite_NT99141(0x3060,0x01);
    #else   // 20140324
        //---- Day_Mode 20140324 ----//
        i2cWrite_NT99141(0x3371,0x38);
        i2cWrite_NT99141(0x3372,0x3C);
        i2cWrite_NT99141(0x3373,0x3F);
        i2cWrite_NT99141(0x3374,0x3F);
        i2cWrite_NT99141(0x3375,0x0A);
        i2cWrite_NT99141(0x3376,0x0C);
        i2cWrite_NT99141(0x3377,0x10);
        i2cWrite_NT99141(0x3378,0x14);
        i2cWrite_NT99141(0x3368,0x30);
        i2cWrite_NT99141(0x3369,0x28);
        i2cWrite_NT99141(0x336A,0x20);
        i2cWrite_NT99141(0x336B,0x10);
        i2cWrite_NT99141(0x3270,0x00);
        i2cWrite_NT99141(0x3271,0x0C);
        i2cWrite_NT99141(0x3272,0x18);
        i2cWrite_NT99141(0x3273,0x32);
        i2cWrite_NT99141(0x3274,0x44);
        i2cWrite_NT99141(0x3275,0x54);
        i2cWrite_NT99141(0x3276,0x70);
        i2cWrite_NT99141(0x3277,0x88);
        i2cWrite_NT99141(0x3278,0x9D);
        i2cWrite_NT99141(0x3279,0xB0);
        i2cWrite_NT99141(0x327A,0xCF);
        i2cWrite_NT99141(0x327B,0xE2);
        i2cWrite_NT99141(0x327C,0xEF);
        i2cWrite_NT99141(0x327D,0xF7);
        i2cWrite_NT99141(0x327E,0xFF);
        i2cWrite_NT99141(0x32F1,0x00);
        i2cWrite_NT99141(0x32f2,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc,0x00);
        i2cWrite_NT99141(0x3053,0x10);
        i2cWrite_NT99141(0x3060,0x01);
    #endif
    }
}


#elif ((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
void siuSetSensorDayNight(u8 Mode)
{
#if(SENSOR_PARAMETER_VERSION == 1)
    if(Mode == SIU_NIGHT_MODE)
    {
        i2cWrite_NT99141(0x3053 ,0x0C); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] - 0x1A);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x34);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);    
    }
    else
    {
        i2cWrite_NT99141(0x3053 ,0x10); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x28);
        i2cWrite_NT99141(0x32F1 ,0x05);
		i2cWrite_NT99141(0x32F3 ,0xB0);
        i2cWrite_NT99141(0x3060 ,0x01);
    }
}
#else
    if(Mode == SIU_NIGHT_MODE)
    {
        i2cWrite_NT99141(0x3053 ,0x0C); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] +0x28);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x38);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);          
    }
    else
    {
        i2cWrite_NT99141(0x3053 ,0x10); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x30);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);
    }
}
#endif
#elif (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
void siuSetSensorDayNight(u8 Mode)
{
    if(Mode == SIU_NIGHT_MODE)
    {
        i2cWrite_NT99141(0x3053 ,0x0C); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex] - 0x1A);
        i2cWrite_NT99141(0x32fc ,0x0D);
        i2cWrite_NT99141(0x32f8 ,0x01);
        i2cWrite_NT99141(0x336D ,0x24);
        i2cWrite_NT99141(0x336E ,0x1F);
        i2cWrite_NT99141(0x336F ,0x19);
        i2cWrite_NT99141(0x3370 ,0x13);
        i2cWrite_NT99141(0x32C4 ,0x34);
        i2cWrite_NT99141(0x32F1 ,0x00);
        i2cWrite_NT99141(0x3060 ,0x01);    
    }
    else
    {
        i2cWrite_NT99141(0x3053 ,0x10); 
        i2cWrite_NT99141(0x32f2 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_NT99141(0x32fc ,0x00);
        i2cWrite_NT99141(0x32f8 ,0x10);
        i2cWrite_NT99141(0x336D ,0x20);
        i2cWrite_NT99141(0x336E ,0x1C);
        i2cWrite_NT99141(0x336F ,0x18);
        i2cWrite_NT99141(0x3370 ,0x10);
        i2cWrite_NT99141(0x32C4 ,0x28);
        i2cWrite_NT99141(0x32F1 ,0x05);
#if(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
            i2cWrite_NT99141(0x32F3, 0x98);
#else
            i2cWrite_NT99141(0x32F3, 0xB0);
#endif
        i2cWrite_NT99141(0x3060 ,0x01);
    }
}
#elif( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE==1) )
void siuSetSensorDayNight(u8 Mode)
{
	if(Mode == SIU_NIGHT_MODE)
	{
		i2cWrite_NT99141(0x3053 ,0x0C); 
		i2cWrite_NT99141(0x32f2 ,0xC8);//<--
		i2cWrite_NT99141(0x32fc ,0x0D);
		i2cWrite_NT99141(0x32f8 ,0x01);
		i2cWrite_NT99141(0x336D ,0x24);
		i2cWrite_NT99141(0x336E ,0x1F);
		i2cWrite_NT99141(0x336F ,0x19);
		i2cWrite_NT99141(0x3370 ,0x13);
		i2cWrite_NT99141(0x32C4 ,0x34);
		i2cWrite_NT99141(0x32F1 ,0x00);
		i2cWrite_NT99141(0x32F1 ,0x01);//Black and White
		i2cWrite_NT99141(0x3060 ,0x01);    
	}
	else
	{
		i2cWrite_NT99141(0x3053 ,0x10); 
		i2cWrite_NT99141(0x32f2 ,0x80);//<--
		i2cWrite_NT99141(0x32fc ,0x00);
		i2cWrite_NT99141(0x32f8 ,0x10);
		i2cWrite_NT99141(0x336D ,0x20);
		i2cWrite_NT99141(0x336E ,0x1C);
		i2cWrite_NT99141(0x336F ,0x18);
		i2cWrite_NT99141(0x3370 ,0x10);
		i2cWrite_NT99141(0x32C4 ,0x28);
		//i2cWrite_NT99141(0x32F1 ,0x05);
		i2cWrite_NT99141(0x32F1 ,0x00);
		//i2cWrite_NT99141(0x32F3, 0x98);
		i2cWrite_NT99141(0x32F3, 0xB0);
		i2cWrite_NT99141(0x3060 ,0x01);
	}
}

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

void siu_NT99141PowerDown(void)
{
   i2cWrite_SENSOR(0x0d, 0x0006);
}

void siu_NT99141PowerUp(void)
{
   i2cWrite_SENSOR(0x0d, 0x0000);
}

void siuSetSensorChrome(u8 level)
{

}


#endif	




