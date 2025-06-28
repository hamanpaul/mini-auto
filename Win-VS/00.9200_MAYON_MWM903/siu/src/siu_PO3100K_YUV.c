
/*
Copyright (c) 2014 Mars Semiconductor Corp.

Module Name:

    siu_PO3100K_YUV.c

Abstract:

    The routines of Sensor Interface Unit.
    Control PO3100K 720P sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2104/08/07  Peter Hsu Create  
*/


#include "general.h"
#if (Sensor_OPTION == Sensor_PO3100K_YUV601)
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
const s8 AETargetMeanTab[9] = {0x36, 0x46, 0x56, 0x66, 0x76, 0x86, 0x96, 0xa6, 0xb6};

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
#if ( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) ||\
    (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_BT_USB))
u32 PO3100K_720P_FrameRate  = 30;
#else
u32 PO3100K_720P_FrameRate  = 27;
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

s32 siuSetContrast(s8 expBiasValue)
{
    return 1;
}

s32 siuSetSaturation(s8 expBiasValue)
{
    return 1;
}

s32 siuSetSharpness(s8 expBiasValue)
{
    return 1;
}
s32 siuSetFlipMirror(s8 Value)
{
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x05,Value);
    return 1;
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
        		SIU_PIX_CLK_ACT_FAL |                   //positive edge latch 
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

#if ( (HW_BOARD_OPTION == MR9120_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK) )

void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
	{   //夜間模式
        DEBUG_SIU("##enter night\n");

        #if 1
        //night 150911
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01); //AE center window Y start position 
        i2cWrite_PO3100K(0xC0, 0x27);
        i2cWrite_PO3100K(0xC1, 0x02); //AE center window Y stop position
        i2cWrite_PO3100K(0xC2, 0x48);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x4E); //AE reference
        i2cWrite_PO3100K(0x1D, 0x00);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x4E);
        i2cWrite_PO3100K(0x20, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);                             
        i2cWrite_PO3100K(0x56, 0xFF);
        i2cWrite_PO3100K(0x57, 0xFF);
        i2cWrite_PO3100K(0x58, 0xFF);     
        i2cWrite_PO3100K(0x03, 0x04); 
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x68);
        i2cWrite_PO3100K(0x3C, 0x60);
        i2cWrite_PO3100K(0x3D, 0x60);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);                                                          
        i2cWrite_PO3100K(0x03, 0x03);              
        i2cWrite_PO3100K(0xA1, 0x30);
        i2cWrite_PO3100K(0xA2, 0x30);
        i2cWrite_PO3100K(0xA3, 0x30);
        i2cWrite_PO3100K(0x03, 0x03);          
        i2cWrite_PO3100K(0xA5, 0x00);
        i2cWrite_PO3100K(0xA6, 0x00);
        i2cWrite_PO3100K(0xA7, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x00);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x10);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x02);
        i2cWrite_PO3100K(0x31, 0x02);
        i2cWrite_PO3100K(0x32, 0x02);
        i2cWrite_PO3100K(0x33, 0x02);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x03);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x04);
        i2cWrite_PO3100K(0x3F, 0x08);
        i2cWrite_PO3100K(0x40, 0x18);

        #endif
	}
    else if(Level == SIU_DAY_MODE)
    {   //日間模式
        DEBUG_SIU("##enter day\n");


        #if 1
        // 151030
        i2cWrite_PO3100K(0x03,0x02);     //day 
        i2cWrite_PO3100K(0xBF,0x01);
        i2cWrite_PO3100K(0xC0,0x0C);
        i2cWrite_PO3100K(0xC1,0x02);
        i2cWrite_PO3100K(0xC2,0x32);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x1C,0xBA);
        i2cWrite_PO3100K(0x1D,0x00);
        i2cWrite_PO3100K(0x1E,0x00);
        i2cWrite_PO3100K(0x1F,0xBA);
        i2cWrite_PO3100K(0x20,0x00);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x56,0x10);
        i2cWrite_PO3100K(0x57,0x04);
        i2cWrite_PO3100K(0x58,0x08);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x05,0x64);
        i2cWrite_PO3100K(0x3B,0x90);
        i2cWrite_PO3100K(0x3C,0x78);
        i2cWrite_PO3100K(0x3D,0x70);
        i2cWrite_PO3100K(0x3E,0x78);
        i2cWrite_PO3100K(0x3F,0x24);
        i2cWrite_PO3100K(0x40,0x4B);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0xA1,0x14);
        i2cWrite_PO3100K(0xA2,0x38);
        i2cWrite_PO3100K(0xA3,0x5F);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0xA5,0x05);
        i2cWrite_PO3100K(0xA6,0x1A);
        i2cWrite_PO3100K(0xA7,0x34);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x19,0x31);
        i2cWrite_PO3100K(0x03,0x02);
        i2cWrite_PO3100K(0x2F,0x24);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x30,0x08);
        i2cWrite_PO3100K(0x31,0x08);
        i2cWrite_PO3100K(0x32,0x08);
        i2cWrite_PO3100K(0x33,0x08);
        i2cWrite_PO3100K(0x34,0x0A);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x82,0x02);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x3E,0x01);
        i2cWrite_PO3100K(0x3F,0x04);
        i2cWrite_PO3100K(0x40,0x18);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x05,0x24);
        i2cWrite_PO3100K(0x03,0x02);
        i2cWrite_PO3100K(0x80,0x25);
        i2cWrite_PO3100K(0x81,0x00);
        i2cWrite_PO3100K(0x82,0x00);
        i2cWrite_PO3100K(0x83,0x2E);
        #elif 0
        //day 20151013
        i2cWrite_PO3100K(0x03,0x02);
        i2cWrite_PO3100K(0xBF,0x01);
        i2cWrite_PO3100K(0xC0,0x0C);
        i2cWrite_PO3100K(0xC1,0x02);
        i2cWrite_PO3100K(0xC2,0x32);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x1C,0xBA);
        i2cWrite_PO3100K(0x1D,0x00);
        i2cWrite_PO3100K(0x1E,0x00);
        i2cWrite_PO3100K(0x1F,0xBA);
        i2cWrite_PO3100K(0x20,0x00);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x56,0x10);
        i2cWrite_PO3100K(0x57,0x04);
        i2cWrite_PO3100K(0x58,0x08);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x05,0x64);
        i2cWrite_PO3100K(0x3B,0x90);
        i2cWrite_PO3100K(0x3C,0x78);
        i2cWrite_PO3100K(0x3D,0x70);
        i2cWrite_PO3100K(0x3E,0x78);
        i2cWrite_PO3100K(0x3F,0x24);
        i2cWrite_PO3100K(0x40,0x4B);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0xA1,0x30); // 20150518
        i2cWrite_PO3100K(0xA2,0x7F);
        i2cWrite_PO3100K(0xA3,0x7F);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0xA5,0x05);
        i2cWrite_PO3100K(0xA6,0x1A);
        i2cWrite_PO3100K(0xA7,0x34);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x19,0x31);
        i2cWrite_PO3100K(0x03,0x02);
        i2cWrite_PO3100K(0x2F,0x24);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x30,0x08);
        i2cWrite_PO3100K(0x31,0x08);
        i2cWrite_PO3100K(0x32,0x08);
        i2cWrite_PO3100K(0x33,0x08);
        i2cWrite_PO3100K(0x34,0x0A);
        i2cWrite_PO3100K(0x03,0x04);
        i2cWrite_PO3100K(0x82,0x02);
        i2cWrite_PO3100K(0x03,0x03);
        i2cWrite_PO3100K(0x3E,0x01);
        i2cWrite_PO3100K(0x3F,0x04);
        i2cWrite_PO3100K(0x40,0x18);
        i2cWrite_PO3100K(0x03,0x04); 
        i2cWrite_PO3100K(0x05,0x24);
        i2cWrite_PO3100K(0x03,0x02);
        i2cWrite_PO3100K(0x80,0x25);
        i2cWrite_PO3100K(0x81,0x00);
        i2cWrite_PO3100K(0x82,0x00);
        i2cWrite_PO3100K(0x83,0x2E);
        #else
        //day 150911
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01);
        i2cWrite_PO3100K(0xC0, 0x0C);
        i2cWrite_PO3100K(0xC1, 0x02);
        i2cWrite_PO3100K(0xC2, 0x32);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x17);
        i2cWrite_PO3100K(0x1D, 0x40);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x17);
        i2cWrite_PO3100K(0x20, 0x40);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x56, 0x10);
        i2cWrite_PO3100K(0x57, 0x04);
        i2cWrite_PO3100K(0x58, 0x08);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x90);
        i2cWrite_PO3100K(0x3C, 0x70);
        i2cWrite_PO3100K(0x3D, 0x70);
        i2cWrite_PO3100K(0x3E, 0x68);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA1, 0x25); // 20150518
        i2cWrite_PO3100K(0xA2, 0x14);
        i2cWrite_PO3100K(0xA3, 0x14);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA5, 0x05);
        i2cWrite_PO3100K(0xA6, 0x1A);
        i2cWrite_PO3100K(0xA7, 0x34);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x3E);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x24);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x08);
        i2cWrite_PO3100K(0x31, 0x08);
        i2cWrite_PO3100K(0x32, 0x08);
        i2cWrite_PO3100K(0x33, 0x08);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x02);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x01);
        i2cWrite_PO3100K(0x3F, 0x04);
        i2cWrite_PO3100K(0x40, 0x18);
        #endif
        
    }
}

void SetPO3100K_720P_30FPS(void)
{
    int i;

    DEBUG_SIU("SetPO3100K_720P_30FPS()\n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

#if 1
    // 151030
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x02);
    i2cWrite_PO3100K(0x34,0x01);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x88);
    i2cWrite_PO3100K(0xA5,0x88);
    i2cWrite_PO3100K(0xA6,0x88);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x78);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x78);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0xBA);
    i2cWrite_PO3100K(0x1D,0x00);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0xBA);
    i2cWrite_PO3100K(0x20,0x00);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x08);
    i2cWrite_PO3100K(0x43,0x10);
    i2cWrite_PO3100K(0x44,0x20);
    i2cWrite_PO3100K(0x2E,0x04);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x37);
    i2cWrite_PO3100K(0x34,0x8A);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8B);
    i2cWrite_PO3100K(0x37,0x3E);
    i2cWrite_PO3100K(0x38,0x90);
    i2cWrite_PO3100K(0x39,0x84);
    i2cWrite_PO3100K(0x3A,0x98);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x08,0x25);
    i2cWrite_PO3100K(0x09,0x86);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x25);
    i2cWrite_PO3100K(0x0C,0x25);
    i2cWrite_PO3100K(0x0D,0x88);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x00);
    i2cWrite_PO3100K(0x3E,0x27);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x40);
    i2cWrite_PO3100K(0x41,0x49);
    i2cWrite_PO3100K(0x42,0x58);
    i2cWrite_PO3100K(0x43,0x64);
    i2cWrite_PO3100K(0x44,0x78);
    i2cWrite_PO3100K(0x45,0x89);
    i2cWrite_PO3100K(0x46,0xA4);
    i2cWrite_PO3100K(0x47,0xBB);
    i2cWrite_PO3100K(0x48,0xCF);
    i2cWrite_PO3100K(0x49,0xE0);
    i2cWrite_PO3100K(0x4A,0xF1);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x03);
    i2cWrite_PO3100K(0x5D,0x0C);
    i2cWrite_PO3100K(0x5E,0x19);
    i2cWrite_PO3100K(0x5F,0x26);
    i2cWrite_PO3100K(0x60,0x3F);
    i2cWrite_PO3100K(0x61,0x52);
    i2cWrite_PO3100K(0x62,0x6E);
    i2cWrite_PO3100K(0x63,0x82);
    i2cWrite_PO3100K(0x64,0xA1);
    i2cWrite_PO3100K(0x65,0xB9);
    i2cWrite_PO3100K(0x66,0xCE);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF0);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x0B);
    i2cWrite_PO3100K(0x6C,0x13);
    i2cWrite_PO3100K(0x6D,0x1A);
    i2cWrite_PO3100K(0x6E,0x20);
    i2cWrite_PO3100K(0x6F,0x2B);
    i2cWrite_PO3100K(0x70,0x36);
    i2cWrite_PO3100K(0x71,0x49);
    i2cWrite_PO3100K(0x72,0x5A);
    i2cWrite_PO3100K(0x73,0x7B);
    i2cWrite_PO3100K(0x74,0x98);
    i2cWrite_PO3100K(0x75,0xB4);
    i2cWrite_PO3100K(0x76,0xCE);
    i2cWrite_PO3100K(0x77,0xE7);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x2F,0x14);
    i2cWrite_PO3100K(0x30,0x40);
    i2cWrite_PO3100K(0x31,0x40);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x00);
    i2cWrite_PO3100K(0x78,0x00);
    i2cWrite_PO3100K(0x79,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x14);
    i2cWrite_PO3100K(0xA2,0x38);
    i2cWrite_PO3100K(0xA3,0x5F);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x24);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x80,0x25);
    i2cWrite_PO3100K(0x81,0x00);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x83,0x2E);

#elif 0
    // 20151013 test2
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x02);
    i2cWrite_PO3100K(0x34,0x01);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x88);
    i2cWrite_PO3100K(0xA5,0x88);
    i2cWrite_PO3100K(0xA6,0x88);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x78);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x78);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0xBA);
    i2cWrite_PO3100K(0x1D,0x00);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0xBA);
    i2cWrite_PO3100K(0x20,0x00);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x08);
    i2cWrite_PO3100K(0x43,0x10);
    i2cWrite_PO3100K(0x44,0x20);
    i2cWrite_PO3100K(0x2E,0x04);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x37);
    i2cWrite_PO3100K(0x34,0x8A);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8B);
    i2cWrite_PO3100K(0x37,0x3E);
    i2cWrite_PO3100K(0x38,0x90);
    i2cWrite_PO3100K(0x39,0x84);
    i2cWrite_PO3100K(0x3A,0x98);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x08,0x25);
    i2cWrite_PO3100K(0x09,0x86);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x25);
    i2cWrite_PO3100K(0x0C,0x25);
    i2cWrite_PO3100K(0x0D,0x88);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x00);
    i2cWrite_PO3100K(0x3E,0x27);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x40);
    i2cWrite_PO3100K(0x41,0x49);
    i2cWrite_PO3100K(0x42,0x58);
    i2cWrite_PO3100K(0x43,0x64);
    i2cWrite_PO3100K(0x44,0x78);
    i2cWrite_PO3100K(0x45,0x89);
    i2cWrite_PO3100K(0x46,0xA4);
    i2cWrite_PO3100K(0x47,0xBB);
    i2cWrite_PO3100K(0x48,0xCF);
    i2cWrite_PO3100K(0x49,0xE0);
    i2cWrite_PO3100K(0x4A,0xF1);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x03);
    i2cWrite_PO3100K(0x5D,0x0C);
    i2cWrite_PO3100K(0x5E,0x19);
    i2cWrite_PO3100K(0x5F,0x26);
    i2cWrite_PO3100K(0x60,0x3F);
    i2cWrite_PO3100K(0x61,0x52);
    i2cWrite_PO3100K(0x62,0x6E);
    i2cWrite_PO3100K(0x63,0x82);
    i2cWrite_PO3100K(0x64,0xA1);
    i2cWrite_PO3100K(0x65,0xB9);
    i2cWrite_PO3100K(0x66,0xCE);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF0);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x0B);
    i2cWrite_PO3100K(0x6C,0x13);
    i2cWrite_PO3100K(0x6D,0x1A);
    i2cWrite_PO3100K(0x6E,0x20);
    i2cWrite_PO3100K(0x6F,0x2B);
    i2cWrite_PO3100K(0x70,0x36);
    i2cWrite_PO3100K(0x71,0x49);
    i2cWrite_PO3100K(0x72,0x5A);
    i2cWrite_PO3100K(0x73,0x7B);
    i2cWrite_PO3100K(0x74,0x98);
    i2cWrite_PO3100K(0x75,0xB4);
    i2cWrite_PO3100K(0x76,0xCE);
    i2cWrite_PO3100K(0x77,0xE7);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x2F,0x14);
    i2cWrite_PO3100K(0x30,0x40);
    i2cWrite_PO3100K(0x31,0x40);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x00);
    i2cWrite_PO3100K(0x78,0x00);
    i2cWrite_PO3100K(0x79,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x30);
    i2cWrite_PO3100K(0xA2,0x7F);
    i2cWrite_PO3100K(0xA3,0x7F);
    i2cWrite_PO3100K(0x03,0x04); 
    i2cWrite_PO3100K(0x05,0x24);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x80,0x25);
    i2cWrite_PO3100K(0x81,0x00);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x83,0x2E);
#elif 0
    // 20150929
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    #if SENSOR_ROW_COL_MIRROR
    i2cWrite_PO3100K(0x05,0x03);
    #else
    i2cWrite_PO3100K(0x05,0x00);
    #endif
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x81);
    i2cWrite_PO3100K(0xA5,0x81);
    i2cWrite_PO3100K(0xA6,0x81);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x36);
    i2cWrite_PO3100K(0x34,0x86);
    i2cWrite_PO3100K(0x35,0x90);
    i2cWrite_PO3100K(0x36,0x8C);
    i2cWrite_PO3100K(0x37,0x50);
    i2cWrite_PO3100K(0x38,0xA3);
    i2cWrite_PO3100K(0x39,0x83);
    i2cWrite_PO3100K(0x3A,0x99);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x04,0x25);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x06,0x00);
    i2cWrite_PO3100K(0x07,0x1C);
    i2cWrite_PO3100K(0x08,0x1E);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x20);
    i2cWrite_PO3100K(0x0C,0x28);
    i2cWrite_PO3100K(0x0D,0x00);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x0B);
    i2cWrite_PO3100K(0x3E,0x2B);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x3F);
    i2cWrite_PO3100K(0x41,0x47);
    i2cWrite_PO3100K(0x42,0x52);
    i2cWrite_PO3100K(0x43,0x5A);
    i2cWrite_PO3100K(0x44,0x6C);
    i2cWrite_PO3100K(0x45,0x7A);
    i2cWrite_PO3100K(0x46,0x95);
    i2cWrite_PO3100K(0x47,0xAE);
    i2cWrite_PO3100K(0x48,0xC5);
    i2cWrite_PO3100K(0x49,0xDA);
    i2cWrite_PO3100K(0x4A,0xED);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x4C,0x00);
    i2cWrite_PO3100K(0x4D,0x05);
    i2cWrite_PO3100K(0x4E,0x14);
    i2cWrite_PO3100K(0x4F,0x25);
    i2cWrite_PO3100K(0x50,0x34);
    i2cWrite_PO3100K(0x51,0x4B);
    i2cWrite_PO3100K(0x52,0x5B);
    i2cWrite_PO3100K(0x53,0x73);
    i2cWrite_PO3100K(0x54,0x86);
    i2cWrite_PO3100K(0x55,0xA3);
    i2cWrite_PO3100K(0x56,0xBA);
    i2cWrite_PO3100K(0x57,0xCE);
    i2cWrite_PO3100K(0x58,0xE0);
    i2cWrite_PO3100K(0x59,0xF0);
    i2cWrite_PO3100K(0x5A,0xFF);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x27);
    i2cWrite_PO3100K(0x5D,0x36);
    i2cWrite_PO3100K(0x5E,0x40);
    i2cWrite_PO3100K(0x5F,0x49);
    i2cWrite_PO3100K(0x60,0x58);
    i2cWrite_PO3100K(0x61,0x64);
    i2cWrite_PO3100K(0x62,0x78);
    i2cWrite_PO3100K(0x63,0x89);
    i2cWrite_PO3100K(0x64,0xA4);
    i2cWrite_PO3100K(0x65,0xBB);
    i2cWrite_PO3100K(0x66,0xCF);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF1);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x2B);
    i2cWrite_PO3100K(0x6C,0x3A);
    i2cWrite_PO3100K(0x6D,0x45);
    i2cWrite_PO3100K(0x6E,0x4E);
    i2cWrite_PO3100K(0x6F,0x5C);
    i2cWrite_PO3100K(0x70,0x68);
    i2cWrite_PO3100K(0x71,0x7C);
    i2cWrite_PO3100K(0x72,0x8D);
    i2cWrite_PO3100K(0x73,0xA8);
    i2cWrite_PO3100K(0x74,0xBE);
    i2cWrite_PO3100K(0x75,0xD1);
    i2cWrite_PO3100K(0x76,0xE2);
    i2cWrite_PO3100K(0x77,0xF1);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x01);
    i2cWrite_PO3100K(0x0B,0x80);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x07);
    i2cWrite_PO3100K(0x78,0x07);
    i2cWrite_PO3100K(0x79,0x07);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x1C);
    i2cWrite_PO3100K(0xA2,0x14);
    i2cWrite_PO3100K(0xA3,0x14);
    i2cWrite_PO3100K(0xA5,0x00);
    i2cWrite_PO3100K(0xA6,0x1A);
    i2cWrite_PO3100K(0xA7,0x34);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x3A);
    i2cWrite_PO3100K(0x1D,0x26);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x3A);
    i2cWrite_PO3100K(0x20,0x26);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x32);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x3E);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x10);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x64);
    i2cWrite_PO3100K(0x28,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x4E,0x00);
    i2cWrite_PO3100K(0x4F,0x20);
    i2cWrite_PO3100K(0x50,0x20);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x70);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x68);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x10);
    i2cWrite_PO3100K(0x43,0x1A);
    i2cWrite_PO3100K(0x44,0x25);
    i2cWrite_PO3100K(0x2E,0x03);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x03);
    i2cWrite_PO3100K(0x83,0x20);
    i2cWrite_PO3100K(0x84,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x91,0x40);
    i2cWrite_PO3100K(0x92,0x40);
    i2cWrite_PO3100K(0x93,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x95,0xFF);
    i2cWrite_PO3100K(0x96,0xF8);
    i2cWrite_PO3100K(0x97,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x36,0x1F);
    i2cWrite_PO3100K(0x37,0x1F);
    i2cWrite_PO3100K(0x38,0x1F);
    i2cWrite_PO3100K(0x3A,0xFF);
    i2cWrite_PO3100K(0x3B,0xFF);
    i2cWrite_PO3100K(0x3C,0xFF);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x0A);
    i2cWrite_PO3100K(0x03,0x05);
    i2cWrite_PO3100K(0x04,0x00);
    #else // 20150514
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    #if SENSOR_ROW_COL_MIRROR
    i2cWrite_PO3100K(0x05,0x03);
    #else
    i2cWrite_PO3100K(0x05,0x00);
    #endif
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x81);
    i2cWrite_PO3100K(0xA5,0x81);
    i2cWrite_PO3100K(0xA6,0x81);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x36);
    i2cWrite_PO3100K(0x34,0x89);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8C);
    i2cWrite_PO3100K(0x37,0x3A);
    i2cWrite_PO3100K(0x38,0x8D);
    i2cWrite_PO3100K(0x39,0x83);
    i2cWrite_PO3100K(0x3A,0x99);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x04,0x25);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x06,0x00);
    i2cWrite_PO3100K(0x07,0x1C);
    i2cWrite_PO3100K(0x08,0x1E);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x20);
    i2cWrite_PO3100K(0x0C,0x28);
    i2cWrite_PO3100K(0x0D,0x00);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x0B);
    i2cWrite_PO3100K(0x3E,0x2B);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x3F);
    i2cWrite_PO3100K(0x41,0x47);
    i2cWrite_PO3100K(0x42,0x52);
    i2cWrite_PO3100K(0x43,0x5A);
    i2cWrite_PO3100K(0x44,0x6C);
    i2cWrite_PO3100K(0x45,0x7A);
    i2cWrite_PO3100K(0x46,0x95);
    i2cWrite_PO3100K(0x47,0xAE);
    i2cWrite_PO3100K(0x48,0xC5);
    i2cWrite_PO3100K(0x49,0xDA);
    i2cWrite_PO3100K(0x4A,0xED);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x4C,0x00);
    i2cWrite_PO3100K(0x4D,0x05);
    i2cWrite_PO3100K(0x4E,0x14);
    i2cWrite_PO3100K(0x4F,0x25);
    i2cWrite_PO3100K(0x50,0x34);
    i2cWrite_PO3100K(0x51,0x4B);
    i2cWrite_PO3100K(0x52,0x5B);
    i2cWrite_PO3100K(0x53,0x73);
    i2cWrite_PO3100K(0x54,0x86);
    i2cWrite_PO3100K(0x55,0xA3);
    i2cWrite_PO3100K(0x56,0xBA);
    i2cWrite_PO3100K(0x57,0xCE);
    i2cWrite_PO3100K(0x58,0xE0);
    i2cWrite_PO3100K(0x59,0xF0);
    i2cWrite_PO3100K(0x5A,0xFF);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x27);
    i2cWrite_PO3100K(0x5D,0x36);
    i2cWrite_PO3100K(0x5E,0x40);
    i2cWrite_PO3100K(0x5F,0x49);
    i2cWrite_PO3100K(0x60,0x58);
    i2cWrite_PO3100K(0x61,0x64);
    i2cWrite_PO3100K(0x62,0x78);
    i2cWrite_PO3100K(0x63,0x89);
    i2cWrite_PO3100K(0x64,0xA4);
    i2cWrite_PO3100K(0x65,0xBB);
    i2cWrite_PO3100K(0x66,0xCF);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF1);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x2B);
    i2cWrite_PO3100K(0x6C,0x3A);
    i2cWrite_PO3100K(0x6D,0x45);
    i2cWrite_PO3100K(0x6E,0x4E);
    i2cWrite_PO3100K(0x6F,0x5C);
    i2cWrite_PO3100K(0x70,0x68);
    i2cWrite_PO3100K(0x71,0x7C);
    i2cWrite_PO3100K(0x72,0x8D);
    i2cWrite_PO3100K(0x73,0xA8);
    i2cWrite_PO3100K(0x74,0xBE);
    i2cWrite_PO3100K(0x75,0xD1);
    i2cWrite_PO3100K(0x76,0xE2);
    i2cWrite_PO3100K(0x77,0xF1);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x01);
    i2cWrite_PO3100K(0x0B,0x80);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x07);
    i2cWrite_PO3100K(0x78,0x07);
    i2cWrite_PO3100K(0x79,0x07);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x1C);
    i2cWrite_PO3100K(0xA2,0x14);
    i2cWrite_PO3100K(0xA3,0x14);
    i2cWrite_PO3100K(0xA5,0x00);
    i2cWrite_PO3100K(0xA6,0x1A);
    i2cWrite_PO3100K(0xA7,0x34);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x3A);
    i2cWrite_PO3100K(0x1D,0x26);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x3A);
    i2cWrite_PO3100K(0x20,0x26);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x90);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x24);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x10);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x64);
    i2cWrite_PO3100K(0x28,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x4E,0x00);
    i2cWrite_PO3100K(0x4F,0x20);
    i2cWrite_PO3100K(0x50,0x20);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x70);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x68);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x10);
    i2cWrite_PO3100K(0x43,0x1A);
    i2cWrite_PO3100K(0x44,0x25);
    i2cWrite_PO3100K(0x2E,0x03);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x03);
    i2cWrite_PO3100K(0x83,0x20);
    i2cWrite_PO3100K(0x84,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x91,0x40);
    i2cWrite_PO3100K(0x92,0x40);
    i2cWrite_PO3100K(0x93,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x95,0xF0);
    i2cWrite_PO3100K(0x96,0xF8);
    i2cWrite_PO3100K(0x97,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x36,0x1F);
    i2cWrite_PO3100K(0x37,0x1F);
    i2cWrite_PO3100K(0x38,0x1F);
    i2cWrite_PO3100K(0x3A,0xFF);
    i2cWrite_PO3100K(0x3B,0xFF);
    i2cWrite_PO3100K(0x3C,0xFF);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x0A);

    i2cWrite_PO3100K(0x03,0x05);
    i2cWrite_PO3100K(0x04,0x00);

#endif
    siuSetSensorDayNight(DayNightMode);
}
#elif ( (HW_BOARD_OPTION == MR9120_TX_RDI_CA831) || (HW_BOARD_OPTION  == A1019A_TX_RDI_CA831) )
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
	{   //夜間模式
        DEBUG_SIU("##enter night\n");

        // 20150514
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01); //AE center window Y start position 
        i2cWrite_PO3100K(0xC0, 0x27);
        i2cWrite_PO3100K(0xC1, 0x02); //AE center window Y stop position
        i2cWrite_PO3100K(0xC2, 0x48);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x4E); //AE reference
        i2cWrite_PO3100K(0x1D, 0x00);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x4E);
        i2cWrite_PO3100K(0x20, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);                             
        i2cWrite_PO3100K(0x56, 0xFF);
        i2cWrite_PO3100K(0x57, 0xFF);
        i2cWrite_PO3100K(0x58, 0xFF);     
        i2cWrite_PO3100K(0x03, 0x04); 
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x68);
        i2cWrite_PO3100K(0x3C, 0x60);
        i2cWrite_PO3100K(0x3D, 0x60);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);                                                          
        i2cWrite_PO3100K(0x03, 0x03);              
        i2cWrite_PO3100K(0xA1, 0x30);
        i2cWrite_PO3100K(0xA2, 0x30);
        i2cWrite_PO3100K(0xA3, 0x30);
        i2cWrite_PO3100K(0x03, 0x03);          
        i2cWrite_PO3100K(0xA5, 0x00);
        i2cWrite_PO3100K(0xA6, 0x00);
        i2cWrite_PO3100K(0xA7, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x00);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x10);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x02);
        i2cWrite_PO3100K(0x31, 0x02);
        i2cWrite_PO3100K(0x32, 0x02);
        i2cWrite_PO3100K(0x33, 0x02);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x03);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x04);
        i2cWrite_PO3100K(0x3F, 0x08);
        i2cWrite_PO3100K(0x40, 0x18);
        //i2cWrite_PO3100K(0x03, 0x03);
        //i2cWrite_PO3100K(0xA1, 0x1C);
        //i2cWrite_PO3100K(0xA2, 0x14);
        //i2cWrite_PO3100K(0xA3, 0x14);
	}
    else if(Level == SIU_DAY_MODE)
    {   //日間模式
        DEBUG_SIU("##enter day\n");

        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01);
        i2cWrite_PO3100K(0xC0, 0x5C);
        i2cWrite_PO3100K(0xC1, 0x02);
        i2cWrite_PO3100K(0xC2, 0x82);       
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x9F);
        i2cWrite_PO3100K(0x1D, 0xD8);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0xBF);
        i2cWrite_PO3100K(0x20, 0xD0);       
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x56, 0x00);
        i2cWrite_PO3100K(0x57, 0x04);
        i2cWrite_PO3100K(0x58, 0x08);
        
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x98);
        i2cWrite_PO3100K(0x3C, 0x68);
        i2cWrite_PO3100K(0x3D, 0x68);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);        
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA1, 0x25);
        i2cWrite_PO3100K(0xA2, 0x14);
        i2cWrite_PO3100K(0xA3, 0x14);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA5, 0x05);
        i2cWrite_PO3100K(0xA6, 0x1A);
        i2cWrite_PO3100K(0xA7, 0x34);        
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x24);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x24);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x08);
        i2cWrite_PO3100K(0x31, 0x08);
        i2cWrite_PO3100K(0x32, 0x08);
        i2cWrite_PO3100K(0x33, 0x08);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x03);        
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x04);
        i2cWrite_PO3100K(0x3F, 0x08);
        i2cWrite_PO3100K(0x40, 0x18);

        #if 1 //參數測試 20150818 //基本+亮度低 20150826
            switch(1)
            {
                case 0:
                    //完全
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x51,0x10);
                    i2cWrite_PO3100K(0x52,0xE0);
                    i2cWrite_PO3100K(0x53,0x02);
                    i2cWrite_PO3100K(0x54,0x02);
                    i2cWrite_PO3100K(0x55,0x40);
                    i2cWrite_PO3100K(0x56,0xC0);
                    i2cWrite_PO3100K(0x57,0x04);
                    i2cWrite_PO3100K(0x58,0x6E);
                    i2cWrite_PO3100K(0x59,0x45);

                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x5A,0x23);
                    i2cWrite_PO3100K(0x5B,0x55);
                    i2cWrite_PO3100K(0x5C,0x60);
                    i2cWrite_PO3100K(0x5D,0xC8);
                    i2cWrite_PO3100K(0x5E,0x41);
                    i2cWrite_PO3100K(0x5F,0x23);
                    i2cWrite_PO3100K(0x60,0x55);
                    i2cWrite_PO3100K(0x61,0x8C);
                    i2cWrite_PO3100K(0x62,0x37);
                    i2cWrite_PO3100K(0x63,0x87);
                    i2cWrite_PO3100K(0x64,0x37);
                    i2cWrite_PO3100K(0x65,0x23);
                    i2cWrite_PO3100K(0x03,0x02);
                    i2cWrite_PO3100K(0x33,0x37);
                    i2cWrite_PO3100K(0x34,0x88);
                    i2cWrite_PO3100K(0x35,0x8F);
                    i2cWrite_PO3100K(0x36,0x8C);
                    i2cWrite_PO3100K(0x37,0x3A);
                    i2cWrite_PO3100K(0x38,0x8E);
                    i2cWrite_PO3100K(0x39,0x83);
                    i2cWrite_PO3100K(0x3A,0x98);
                    i2cWrite_PO3100K(0x3B,0x3B);
                    // 亮度低
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x05,0x64);
                    i2cWrite_PO3100K(0x3B,0x78);
                    i2cWrite_PO3100K(0x3C,0x68);
                    i2cWrite_PO3100K(0x3D,0x68);
                    i2cWrite_PO3100K(0x3E,0x60);
                    i2cWrite_PO3100K(0x3F,0x24);
                    i2cWrite_PO3100K(0x40,0x4B);
                    break;
                case 1:
                    //基本
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x51,0x10);
                    i2cWrite_PO3100K(0x52,0xAF);
                    i2cWrite_PO3100K(0x53,0x02);
                    i2cWrite_PO3100K(0x54,0x02);
                    i2cWrite_PO3100K(0x55,0x40);
                    i2cWrite_PO3100K(0x56,0xC0);
                    i2cWrite_PO3100K(0x57,0x04);
                    i2cWrite_PO3100K(0x58,0x6E);
                    i2cWrite_PO3100K(0x59,0x45);

                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x5A,0x23);
                    i2cWrite_PO3100K(0x5B,0x55);
                    i2cWrite_PO3100K(0x5C,0x5A);
                    i2cWrite_PO3100K(0x5D,0xC8);
                    i2cWrite_PO3100K(0x5E,0x41);
                    i2cWrite_PO3100K(0x5F,0x28);
                    i2cWrite_PO3100K(0x60,0x2D);
                    i2cWrite_PO3100K(0x61,0x73);
                    i2cWrite_PO3100K(0x62,0x32);
                    i2cWrite_PO3100K(0x63,0x78);
                    i2cWrite_PO3100K(0x64,0x2D);
                    i2cWrite_PO3100K(0x65,0x19);
                    i2cWrite_PO3100K(0x03,0x02);
                    i2cWrite_PO3100K(0x33,0x37);
                    i2cWrite_PO3100K(0x34,0x88);
                    i2cWrite_PO3100K(0x35,0x8F);
                    i2cWrite_PO3100K(0x36,0x8C);
                    i2cWrite_PO3100K(0x37,0x3A);
                    i2cWrite_PO3100K(0x38,0x8E);
                    i2cWrite_PO3100K(0x39,0x83);
                    i2cWrite_PO3100K(0x3A,0x98);
                    // 亮度低
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x05,0x64);
                    i2cWrite_PO3100K(0x3B,0x78);
                    i2cWrite_PO3100K(0x3C,0x68);
                    i2cWrite_PO3100K(0x3D,0x68);
                    i2cWrite_PO3100K(0x3E,0x60);
                    i2cWrite_PO3100K(0x3F,0x24);
                    i2cWrite_PO3100K(0x40,0x4B);
                    break;
                case 3:
                    //基本
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x51,0x10);
                    i2cWrite_PO3100K(0x52,0xAF);
                    i2cWrite_PO3100K(0x53,0x02);
                    i2cWrite_PO3100K(0x54,0x02);
                    i2cWrite_PO3100K(0x55,0x40);
                    i2cWrite_PO3100K(0x56,0xC0);
                    i2cWrite_PO3100K(0x57,0x04);
                    i2cWrite_PO3100K(0x58,0x6E);
                    i2cWrite_PO3100K(0x59,0x45);

                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x5A,0x23);
                    i2cWrite_PO3100K(0x5B,0x55);
                    i2cWrite_PO3100K(0x5C,0x5A);
                    i2cWrite_PO3100K(0x5D,0xC8);
                    i2cWrite_PO3100K(0x5E,0x41);
                    i2cWrite_PO3100K(0x5F,0x28);
                    i2cWrite_PO3100K(0x60,0x2D);
                    i2cWrite_PO3100K(0x61,0x73);
                    i2cWrite_PO3100K(0x62,0x32);
                    i2cWrite_PO3100K(0x63,0x78);
                    i2cWrite_PO3100K(0x64,0x2D);
                    i2cWrite_PO3100K(0x65,0x19);
                    i2cWrite_PO3100K(0x03,0x02);
                    i2cWrite_PO3100K(0x33,0x37);
                    i2cWrite_PO3100K(0x34,0x88);
                    i2cWrite_PO3100K(0x35,0x8F);
                    i2cWrite_PO3100K(0x36,0x8C);
                    i2cWrite_PO3100K(0x37,0x3A);
                    i2cWrite_PO3100K(0x38,0x8E);
                    i2cWrite_PO3100K(0x39,0x83);
                    i2cWrite_PO3100K(0x3A,0x98);
                    // 亮度更低
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x05,0x64);
                    i2cWrite_PO3100K(0x3B,0x68);
                    i2cWrite_PO3100K(0x3C,0x60);
                    i2cWrite_PO3100K(0x3D,0x60);
                    i2cWrite_PO3100K(0x3E,0x60);
                    i2cWrite_PO3100K(0x3F,0x24);
                    i2cWrite_PO3100K(0x40,0x4B);
                    break;
                case 2:
                    //完全
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x51,0x10);
                    i2cWrite_PO3100K(0x52,0xE0);
                    i2cWrite_PO3100K(0x53,0x02);
                    i2cWrite_PO3100K(0x54,0x02);
                    i2cWrite_PO3100K(0x55,0x40);
                    i2cWrite_PO3100K(0x56,0xC0);
                    i2cWrite_PO3100K(0x57,0x04);
                    i2cWrite_PO3100K(0x58,0x6E);
                    i2cWrite_PO3100K(0x59,0x45);

                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x5A,0x23);
                    i2cWrite_PO3100K(0x5B,0x55);
                    i2cWrite_PO3100K(0x5C,0x60);
                    i2cWrite_PO3100K(0x5D,0xC8);
                    i2cWrite_PO3100K(0x5E,0x41);
                    i2cWrite_PO3100K(0x5F,0x23);
                    i2cWrite_PO3100K(0x60,0x55);
                    i2cWrite_PO3100K(0x61,0x8C);
                    i2cWrite_PO3100K(0x62,0x37);
                    i2cWrite_PO3100K(0x63,0x87);
                    i2cWrite_PO3100K(0x64,0x37);
                    i2cWrite_PO3100K(0x65,0x23);
                    i2cWrite_PO3100K(0x03,0x02);
                    i2cWrite_PO3100K(0x33,0x37);
                    i2cWrite_PO3100K(0x34,0x88);
                    i2cWrite_PO3100K(0x35,0x8F);
                    i2cWrite_PO3100K(0x36,0x8C);
                    i2cWrite_PO3100K(0x37,0x3A);
                    i2cWrite_PO3100K(0x38,0x8E);
                    i2cWrite_PO3100K(0x39,0x83);
                    i2cWrite_PO3100K(0x3A,0x98);
                    i2cWrite_PO3100K(0x3B,0x3B);
                    // 亮度更低
                    i2cWrite_PO3100K(0x03,0x04);
                    i2cWrite_PO3100K(0x05,0x64);
                    i2cWrite_PO3100K(0x3B,0x68);
                    i2cWrite_PO3100K(0x3C,0x60);
                    i2cWrite_PO3100K(0x3D,0x60);
                    i2cWrite_PO3100K(0x3E,0x60);
                    i2cWrite_PO3100K(0x3F,0x24);
                    i2cWrite_PO3100K(0x40,0x4B);
                    break;
                default:
                    break;
            }
        #endif
            #if 1 // day test sensor 20160127
                printf("day test sensor 20160127 \n");
                i2cWrite_PO3100K(0x03,0x02);
                i2cWrite_PO3100K(0x3D,0x00);
                i2cWrite_PO3100K(0x3E,0x1A);
                i2cWrite_PO3100K(0x3F,0x26);
                i2cWrite_PO3100K(0x40,0x2F);
                i2cWrite_PO3100K(0x41,0x38);
                i2cWrite_PO3100K(0x42,0x46);
                i2cWrite_PO3100K(0x43,0x51);
                i2cWrite_PO3100K(0x44,0x66);
                i2cWrite_PO3100K(0x45,0x77);
                i2cWrite_PO3100K(0x46,0x95);
                i2cWrite_PO3100K(0x47,0xAF);
                i2cWrite_PO3100K(0x48,0xC5);
                i2cWrite_PO3100K(0x49,0xDA);
                i2cWrite_PO3100K(0x4A,0xED);
                i2cWrite_PO3100K(0x4B,0xFF);
                i2cWrite_PO3100K(0x5B,0x00);
                i2cWrite_PO3100K(0x5C,0x20);
                i2cWrite_PO3100K(0x5D,0x2D);
                i2cWrite_PO3100K(0x5E,0x37);
                i2cWrite_PO3100K(0x5F,0x40);
                i2cWrite_PO3100K(0x60,0x4E);
                i2cWrite_PO3100K(0x61,0x5A);
                i2cWrite_PO3100K(0x62,0x6F);
                i2cWrite_PO3100K(0x63,0x80);
                i2cWrite_PO3100K(0x64,0x9C);
                i2cWrite_PO3100K(0x65,0xB5);
                i2cWrite_PO3100K(0x66,0xCA);
                i2cWrite_PO3100K(0x67,0xDD);
                i2cWrite_PO3100K(0x68,0xEF);
                i2cWrite_PO3100K(0x69,0xFF);
                i2cWrite_PO3100K(0x03,0x03);
                i2cWrite_PO3100K(0x19,0x28);
                i2cWrite_PO3100K(0x03,0x04);
                i2cWrite_PO3100K(0x3B,0x70);
                i2cWrite_PO3100K(0x3C,0x60);
                i2cWrite_PO3100K(0x3D,0x60);
                i2cWrite_PO3100K(0x3E,0x60);
                i2cWrite_PO3100K(0x51,0x10);
                i2cWrite_PO3100K(0x52,0xAF);
                i2cWrite_PO3100K(0x53,0x02);
                i2cWrite_PO3100K(0x54,0x02);
                i2cWrite_PO3100K(0x55,0x40);
                i2cWrite_PO3100K(0x56,0xC0);
                i2cWrite_PO3100K(0x57,0x04);
                i2cWrite_PO3100K(0x58,0x6E);
                i2cWrite_PO3100K(0x59,0x45);
                i2cWrite_PO3100K(0x5A,0x23);
                i2cWrite_PO3100K(0x5B,0x55);
                i2cWrite_PO3100K(0x5C,0x5A);
                i2cWrite_PO3100K(0x5D,0xB4);
                i2cWrite_PO3100K(0x5E,0x2D);
                i2cWrite_PO3100K(0x5F,0x28);
                i2cWrite_PO3100K(0x60,0x2D);
                i2cWrite_PO3100K(0x61,0x73);
                i2cWrite_PO3100K(0x62,0x32);
                i2cWrite_PO3100K(0x63,0x78);
                i2cWrite_PO3100K(0x64,0x2D);
                i2cWrite_PO3100K(0x65,0x19);
            #endif

        
    }
}

void SetPO3100K_720P_30FPS_RDI_Init(void)
{
    
    DEBUG_SIU("SetPO3100K_720P_30FPS_RDI_Init()\n");
//BANK B

i2cWrite_PO3100K(0x03,0x01);
i2cWrite_PO3100K(0x04,0xF0);
i2cWrite_PO3100K(0x05,0x02);
i2cWrite_PO3100K(0x06,0x02);
i2cWrite_PO3100K(0x07,0x67);
i2cWrite_PO3100K(0x08,0x20);
i2cWrite_PO3100K(0x09,0x04);
i2cWrite_PO3100K(0x0A,0xF0);
i2cWrite_PO3100K(0x0B,0x10);
i2cWrite_PO3100K(0x0C,0x00);
i2cWrite_PO3100K(0x0D,0x00);
i2cWrite_PO3100K(0x0E,0x1B);
i2cWrite_PO3100K(0x0F,0x00);
i2cWrite_PO3100K(0x10,0x00);
i2cWrite_PO3100K(0x11,0x04);
i2cWrite_PO3100K(0x12,0x0A);
i2cWrite_PO3100K(0x13,0x40);
i2cWrite_PO3100K(0x14,0x01);
i2cWrite_PO3100K(0x15,0x81);
i2cWrite_PO3100K(0x16,0x05);
i2cWrite_PO3100K(0x17,0xFA);
i2cWrite_PO3100K(0x18,0xC3);
i2cWrite_PO3100K(0x19,0xC4);
i2cWrite_PO3100K(0x1A,0xF0);
i2cWrite_PO3100K(0x1B,0x00);
i2cWrite_PO3100K(0x1C,0x11);
i2cWrite_PO3100K(0x1D,0x47);
i2cWrite_PO3100K(0x1E,0x0E);
i2cWrite_PO3100K(0x1F,0x00);
i2cWrite_PO3100K(0x20,0x00);
i2cWrite_PO3100K(0x21,0xFF);
i2cWrite_PO3100K(0x22,0x00);
i2cWrite_PO3100K(0x23,0x00);
i2cWrite_PO3100K(0x24,0x00);
i2cWrite_PO3100K(0x25,0x00);
i2cWrite_PO3100K(0x26,0x03);
i2cWrite_PO3100K(0x27,0xFF);
i2cWrite_PO3100K(0x28,0x07);
i2cWrite_PO3100K(0x29,0xFF);
i2cWrite_PO3100K(0x2A,0x00);
i2cWrite_PO3100K(0x2B,0x00);
i2cWrite_PO3100K(0x2C,0x00);
i2cWrite_PO3100K(0x2D,0x00);
i2cWrite_PO3100K(0x2E,0x00);
i2cWrite_PO3100K(0x2F,0x80);
i2cWrite_PO3100K(0x30,0x00);
i2cWrite_PO3100K(0x31,0x00);
i2cWrite_PO3100K(0x32,0x00);
i2cWrite_PO3100K(0x33,0x0A);
i2cWrite_PO3100K(0x34,0x02);
i2cWrite_PO3100K(0x35,0xEA);
i2cWrite_PO3100K(0x36,0x01);
i2cWrite_PO3100K(0x37,0x12);
i2cWrite_PO3100K(0x38,0x06);
i2cWrite_PO3100K(0x39,0x22);
i2cWrite_PO3100K(0x3A,0x00);
i2cWrite_PO3100K(0x3B,0x02);
i2cWrite_PO3100K(0x3C,0x00);
i2cWrite_PO3100K(0x3D,0x08);
i2cWrite_PO3100K(0x3E,0x01);
i2cWrite_PO3100K(0x3F,0x12);
i2cWrite_PO3100K(0x40,0x06);
i2cWrite_PO3100K(0x41,0x22);
i2cWrite_PO3100K(0x42,0x00);
i2cWrite_PO3100K(0x43,0x00);
i2cWrite_PO3100K(0x44,0x00);
i2cWrite_PO3100K(0x45,0x00);
i2cWrite_PO3100K(0x46,0x00);
i2cWrite_PO3100K(0x47,0x00);
i2cWrite_PO3100K(0x48,0x01);
i2cWrite_PO3100K(0x49,0x00);
i2cWrite_PO3100K(0x4A,0x01);
i2cWrite_PO3100K(0x4B,0x00);
i2cWrite_PO3100K(0x4C,0x01);
i2cWrite_PO3100K(0x4D,0x00);
i2cWrite_PO3100K(0x4E,0x01);
i2cWrite_PO3100K(0x4F,0x02);
i2cWrite_PO3100K(0x50,0x7F);
i2cWrite_PO3100K(0x51,0x04);
i2cWrite_PO3100K(0x52,0xFF);
i2cWrite_PO3100K(0x53,0x02);
i2cWrite_PO3100K(0x54,0xD0);
i2cWrite_PO3100K(0x55,0x06);
i2cWrite_PO3100K(0x56,0x24);
i2cWrite_PO3100K(0x57,0x06);
i2cWrite_PO3100K(0x58,0x24);
i2cWrite_PO3100K(0x59,0x06);
i2cWrite_PO3100K(0x5A,0x53);
i2cWrite_PO3100K(0x5B,0x00);
i2cWrite_PO3100K(0x5C,0x02);
i2cWrite_PO3100K(0x5D,0x06);
i2cWrite_PO3100K(0x5E,0x1C);
i2cWrite_PO3100K(0x5F,0x02);
i2cWrite_PO3100K(0x60,0xC8);
i2cWrite_PO3100K(0x61,0x03);
i2cWrite_PO3100K(0x62,0x66);
i2cWrite_PO3100K(0x63,0x00);
i2cWrite_PO3100K(0x64,0x00);
i2cWrite_PO3100K(0x65,0x00);
i2cWrite_PO3100K(0x66,0x00);
i2cWrite_PO3100K(0x67,0x03);
i2cWrite_PO3100K(0x68,0x79);
i2cWrite_PO3100K(0x69,0x03);
i2cWrite_PO3100K(0x6A,0x8C);
i2cWrite_PO3100K(0x6B,0x00);
i2cWrite_PO3100K(0x6C,0x00);
i2cWrite_PO3100K(0x6D,0x00);
i2cWrite_PO3100K(0x6E,0x00);
i2cWrite_PO3100K(0x6F,0x03);
i2cWrite_PO3100K(0x70,0x96);
i2cWrite_PO3100K(0x71,0x06);
i2cWrite_PO3100K(0x72,0x1A);
i2cWrite_PO3100K(0x73,0x03);
i2cWrite_PO3100K(0x74,0x9C);
i2cWrite_PO3100K(0x75,0x06);
i2cWrite_PO3100K(0x76,0x18);
i2cWrite_PO3100K(0x77,0x06);
i2cWrite_PO3100K(0x78,0x46);
i2cWrite_PO3100K(0x79,0x06);
i2cWrite_PO3100K(0x7A,0x50);
i2cWrite_PO3100K(0x7B,0x03);
i2cWrite_PO3100K(0x7C,0x9C);
i2cWrite_PO3100K(0x7D,0x06);
i2cWrite_PO3100K(0x7E,0x18);
i2cWrite_PO3100K(0x7F,0x06);
i2cWrite_PO3100K(0x80,0x30);
i2cWrite_PO3100K(0x81,0x06);
i2cWrite_PO3100K(0x82,0x67);
i2cWrite_PO3100K(0x83,0x06);
i2cWrite_PO3100K(0x84,0x30);
i2cWrite_PO3100K(0x85,0x06);
i2cWrite_PO3100K(0x86,0x67);
i2cWrite_PO3100K(0x87,0x06);
i2cWrite_PO3100K(0x88,0x28);
i2cWrite_PO3100K(0x89,0x06);
i2cWrite_PO3100K(0x8A,0x2D);
i2cWrite_PO3100K(0x8B,0x03);
i2cWrite_PO3100K(0x8C,0x9E);
i2cWrite_PO3100K(0x8D,0x06);
i2cWrite_PO3100K(0x8E,0x30);
i2cWrite_PO3100K(0x8F,0x03);
i2cWrite_PO3100K(0x90,0x73);
i2cWrite_PO3100K(0x91,0x06);
i2cWrite_PO3100K(0x92,0x30);
i2cWrite_PO3100K(0x93,0x06);
i2cWrite_PO3100K(0x94,0x71);
i2cWrite_PO3100K(0x95,0x06);
i2cWrite_PO3100K(0x96,0x30);
i2cWrite_PO3100K(0x97,0x06);
i2cWrite_PO3100K(0x98,0x41);
i2cWrite_PO3100K(0x99,0x03);
i2cWrite_PO3100K(0x9A,0x79);
i2cWrite_PO3100K(0x9B,0x06);
i2cWrite_PO3100K(0x9C,0x24);
i2cWrite_PO3100K(0x9D,0x00);
i2cWrite_PO3100K(0x9E,0x00);
i2cWrite_PO3100K(0x9F,0x06);
i2cWrite_PO3100K(0xA0,0x56);
i2cWrite_PO3100K(0xA1,0x04);
i2cWrite_PO3100K(0xA2,0x6E);
i2cWrite_PO3100K(0xA3,0x00);
i2cWrite_PO3100K(0xA4,0x88);
i2cWrite_PO3100K(0xA5,0x88);
i2cWrite_PO3100K(0xA6,0x88);
i2cWrite_PO3100K(0xA7,0x00);
i2cWrite_PO3100K(0xA8,0x00);
i2cWrite_PO3100K(0xA9,0x08);
i2cWrite_PO3100K(0xAA,0xFF);
i2cWrite_PO3100K(0xAB,0x7F);
i2cWrite_PO3100K(0xAC,0x88);
i2cWrite_PO3100K(0xAD,0x00);
i2cWrite_PO3100K(0xAE,0x20);
i2cWrite_PO3100K(0xAF,0x20);
i2cWrite_PO3100K(0xB0,0x20);
i2cWrite_PO3100K(0xB1,0x20);
i2cWrite_PO3100K(0xB2,0x00);
i2cWrite_PO3100K(0xB3,0x40);
i2cWrite_PO3100K(0xB4,0x40);
i2cWrite_PO3100K(0xB5,0x40);
i2cWrite_PO3100K(0xB6,0x00);
i2cWrite_PO3100K(0xB7,0x00);
i2cWrite_PO3100K(0xB8,0x00);
i2cWrite_PO3100K(0xB9,0x06);
i2cWrite_PO3100K(0xBA,0x30);
i2cWrite_PO3100K(0xBB,0x06);
i2cWrite_PO3100K(0xBC,0x67);
i2cWrite_PO3100K(0xBD,0x00);
i2cWrite_PO3100K(0xBE,0x08);
i2cWrite_PO3100K(0xBF,0x00);
i2cWrite_PO3100K(0xC0,0x02);
i2cWrite_PO3100K(0xC1,0xA3);
i2cWrite_PO3100K(0xC2,0x00);
i2cWrite_PO3100K(0xC3,0x17);
i2cWrite_PO3100K(0xC4,0x40);
i2cWrite_PO3100K(0xC5,0x00);
i2cWrite_PO3100K(0xC6,0x11);
i2cWrite_PO3100K(0xC7,0x70);
i2cWrite_PO3100K(0xC8,0x01);
i2cWrite_PO3100K(0xC9,0x01);
i2cWrite_PO3100K(0xCA,0x00);
i2cWrite_PO3100K(0xCB,0x00);
i2cWrite_PO3100K(0xCC,0x00);
i2cWrite_PO3100K(0xCD,0x00);
i2cWrite_PO3100K(0xCE,0x00);
i2cWrite_PO3100K(0xCF,0x00);
i2cWrite_PO3100K(0xD0,0x00);
i2cWrite_PO3100K(0xD1,0x00);
i2cWrite_PO3100K(0xD2,0x00);
i2cWrite_PO3100K(0xD3,0x00);
i2cWrite_PO3100K(0xD4,0x00);
i2cWrite_PO3100K(0xD5,0x00);
i2cWrite_PO3100K(0xD6,0x00);
i2cWrite_PO3100K(0xD7,0x00);
i2cWrite_PO3100K(0xD8,0x00);
i2cWrite_PO3100K(0xD9,0x00);
i2cWrite_PO3100K(0xDA,0x00);
i2cWrite_PO3100K(0xDB,0x00);
i2cWrite_PO3100K(0xDC,0x00);
i2cWrite_PO3100K(0xDD,0xFF);
i2cWrite_PO3100K(0xDE,0x66);
i2cWrite_PO3100K(0xDF,0x65);
i2cWrite_PO3100K(0xE0,0x66);
i2cWrite_PO3100K(0xE1,0x00);
i2cWrite_PO3100K(0xE2,0x6D);
i2cWrite_PO3100K(0xE3,0x00);
i2cWrite_PO3100K(0xE4,0x00);
i2cWrite_PO3100K(0xE5,0x00);
i2cWrite_PO3100K(0xE6,0x6D);
i2cWrite_PO3100K(0xE7,0x00);
i2cWrite_PO3100K(0xE8,0x00);
i2cWrite_PO3100K(0xE9,0x00);
i2cWrite_PO3100K(0xEA,0x02);
i2cWrite_PO3100K(0xEB,0xED);
i2cWrite_PO3100K(0xEC,0x07);
i2cWrite_PO3100K(0xED,0xBB);
i2cWrite_PO3100K(0xEE,0x17);
i2cWrite_PO3100K(0xEF,0x02);
i2cWrite_PO3100K(0xF0,0xA3);
i2cWrite_PO3100K(0xF1,0x00);
i2cWrite_PO3100K(0xF2,0x00);
i2cWrite_PO3100K(0xF3,0x00);
i2cWrite_PO3100K(0xF4,0x00);
i2cWrite_PO3100K(0xF5,0x00);
i2cWrite_PO3100K(0xF6,0x00);
i2cWrite_PO3100K(0xF7,0x00);
i2cWrite_PO3100K(0xF8,0x00);
i2cWrite_PO3100K(0xF9,0x00);
i2cWrite_PO3100K(0xFA,0x00);
i2cWrite_PO3100K(0xFB,0x00);
i2cWrite_PO3100K(0xFC,0x00);
i2cWrite_PO3100K(0xFD,0x00);
i2cWrite_PO3100K(0xFE,0x00);
i2cWrite_PO3100K(0xFF,0x00);
                    
                    
//BANK C             
                    
i2cWrite_PO3100K(0x03,0x02);                 
i2cWrite_PO3100K(0x04,0xF7);                 
i2cWrite_PO3100K(0x05,0xDE);                 
i2cWrite_PO3100K(0x06,0x00);  
i2cWrite_PO3100K(0x07,0xE0);  
i2cWrite_PO3100K(0x08,0x00);  
i2cWrite_PO3100K(0x09,0x80);  
i2cWrite_PO3100K(0x0A,0x95);  
i2cWrite_PO3100K(0x0B,0x80);  
i2cWrite_PO3100K(0x0C,0x00);  
i2cWrite_PO3100K(0x0D,0x51);  
i2cWrite_PO3100K(0x0E,0x51);  
i2cWrite_PO3100K(0x0F,0x51);  
i2cWrite_PO3100K(0x10,0x51);  
i2cWrite_PO3100K(0x11,0x51);  
i2cWrite_PO3100K(0x12,0x51);  
i2cWrite_PO3100K(0x13,0x51);  
i2cWrite_PO3100K(0x14,0x51);  
i2cWrite_PO3100K(0x15,0x51);  
i2cWrite_PO3100K(0x16,0x51);  
i2cWrite_PO3100K(0x17,0x51);  
i2cWrite_PO3100K(0x18,0x51);  
i2cWrite_PO3100K(0x19,0x51);  
i2cWrite_PO3100K(0x1A,0x51);  
i2cWrite_PO3100K(0x1B,0x51);  
i2cWrite_PO3100K(0x1C,0x51);  
i2cWrite_PO3100K(0x1D,0x04);  
i2cWrite_PO3100K(0x1E,0x02);  
i2cWrite_PO3100K(0x1F,0x02);  
i2cWrite_PO3100K(0x20,0x00);  
i2cWrite_PO3100K(0x21,0x00);  
i2cWrite_PO3100K(0x22,0x00);  
i2cWrite_PO3100K(0x23,0x00);  
i2cWrite_PO3100K(0x24,0x00);  
i2cWrite_PO3100K(0x25,0x00);  
i2cWrite_PO3100K(0x26,0x00);  
i2cWrite_PO3100K(0x27,0x00);  
i2cWrite_PO3100K(0x28,0x00);  
i2cWrite_PO3100K(0x29,0x00);  
i2cWrite_PO3100K(0x2A,0x00);  
i2cWrite_PO3100K(0x2B,0x14);  
i2cWrite_PO3100K(0x2C,0x00);  
i2cWrite_PO3100K(0x2D,0x40);  
i2cWrite_PO3100K(0x2E,0x30);  
i2cWrite_PO3100K(0x2F,0x26);  
i2cWrite_PO3100K(0x30,0x78);  
i2cWrite_PO3100K(0x31,0x78);  
i2cWrite_PO3100K(0x32,0x00);  
i2cWrite_PO3100K(0x33,0x36);  
i2cWrite_PO3100K(0x34,0x88);  
i2cWrite_PO3100K(0x35,0x8D);  
i2cWrite_PO3100K(0x36,0x8C);  
i2cWrite_PO3100K(0x37,0x3A);  
i2cWrite_PO3100K(0x38,0x8D);  
i2cWrite_PO3100K(0x39,0x85);  
i2cWrite_PO3100K(0x3A,0x98);  
i2cWrite_PO3100K(0x3B,0x3B);  
i2cWrite_PO3100K(0x3C,0x00);  
i2cWrite_PO3100K(0x3D,0x0B);  
i2cWrite_PO3100K(0x3E,0x2B);  
i2cWrite_PO3100K(0x3F,0x36);  
i2cWrite_PO3100K(0x40,0x3F);  
i2cWrite_PO3100K(0x41,0x47);  
i2cWrite_PO3100K(0x42,0x52);  
i2cWrite_PO3100K(0x43,0x5A);  
i2cWrite_PO3100K(0x44,0x6C);  
i2cWrite_PO3100K(0x45,0x7A);  
i2cWrite_PO3100K(0x46,0x95);  
i2cWrite_PO3100K(0x47,0xAE);  
i2cWrite_PO3100K(0x48,0xC5);  
i2cWrite_PO3100K(0x49,0xDA);  
i2cWrite_PO3100K(0x4A,0xED);  
i2cWrite_PO3100K(0x4B,0xFF);  
i2cWrite_PO3100K(0x4C,0x00);  
i2cWrite_PO3100K(0x4D,0x05);  
i2cWrite_PO3100K(0x4E,0x14);  
i2cWrite_PO3100K(0x4F,0x25);  
i2cWrite_PO3100K(0x50,0x34);  
i2cWrite_PO3100K(0x51,0x4B);  
i2cWrite_PO3100K(0x52,0x5B);  
i2cWrite_PO3100K(0x53,0x73);  
i2cWrite_PO3100K(0x54,0x86);  
i2cWrite_PO3100K(0x55,0xA3);  
i2cWrite_PO3100K(0x56,0xBA);  
i2cWrite_PO3100K(0x57,0xCE);  
i2cWrite_PO3100K(0x58,0xE0);  
i2cWrite_PO3100K(0x59,0xF0);  
i2cWrite_PO3100K(0x5A,0xFF);  
i2cWrite_PO3100K(0x5B,0x00);  
i2cWrite_PO3100K(0x5C,0x27);  
i2cWrite_PO3100K(0x5D,0x36);  
i2cWrite_PO3100K(0x5E,0x40);  
i2cWrite_PO3100K(0x5F,0x49);  
i2cWrite_PO3100K(0x60,0x58);  
i2cWrite_PO3100K(0x61,0x64);  
i2cWrite_PO3100K(0x62,0x78);  
i2cWrite_PO3100K(0x63,0x89);  
i2cWrite_PO3100K(0x64,0xA4);  
i2cWrite_PO3100K(0x65,0xBB);  
i2cWrite_PO3100K(0x66,0xCF);  
i2cWrite_PO3100K(0x67,0xE0);  
i2cWrite_PO3100K(0x68,0xF1);  
i2cWrite_PO3100K(0x69,0xFF);  
i2cWrite_PO3100K(0x6A,0x00);  
i2cWrite_PO3100K(0x6B,0x2B);  
i2cWrite_PO3100K(0x6C,0x3A);  
i2cWrite_PO3100K(0x6D,0x45);  
i2cWrite_PO3100K(0x6E,0x4E);  
i2cWrite_PO3100K(0x6F,0x5C);  
i2cWrite_PO3100K(0x70,0x68);  
i2cWrite_PO3100K(0x71,0x7C);  
i2cWrite_PO3100K(0x72,0x8D);  
i2cWrite_PO3100K(0x73,0xA8);  
i2cWrite_PO3100K(0x74,0xBE);  
i2cWrite_PO3100K(0x75,0xD1);  
i2cWrite_PO3100K(0x76,0xE2);  
i2cWrite_PO3100K(0x77,0xF1);  
i2cWrite_PO3100K(0x78,0xFF);  
i2cWrite_PO3100K(0x79,0x00);  
i2cWrite_PO3100K(0x7A,0x8C);  
i2cWrite_PO3100K(0x7B,0x20);  
i2cWrite_PO3100K(0x7C,0x20);  
i2cWrite_PO3100K(0x7D,0x00);  
i2cWrite_PO3100K(0x7E,0x0A);  
i2cWrite_PO3100K(0x7F,0x80);  
i2cWrite_PO3100K(0x80,0x2A);  
i2cWrite_PO3100K(0x81,0x00);  
i2cWrite_PO3100K(0x82,0x00);  
i2cWrite_PO3100K(0x83,0x2A);  
i2cWrite_PO3100K(0x84,0x00);  
i2cWrite_PO3100K(0x85,0x00);  
i2cWrite_PO3100K(0x86,0x80);  
i2cWrite_PO3100K(0x87,0x80);  
i2cWrite_PO3100K(0x88,0x7F);  
i2cWrite_PO3100K(0x89,0x00);  
i2cWrite_PO3100K(0x8A,0x00);  
i2cWrite_PO3100K(0x8B,0x00);  
i2cWrite_PO3100K(0x8C,0x00);  
i2cWrite_PO3100K(0x8D,0x40);  
i2cWrite_PO3100K(0x8E,0xFE);  
i2cWrite_PO3100K(0x8F,0x00);  
i2cWrite_PO3100K(0x90,0x00);  
i2cWrite_PO3100K(0x91,0x41);  
i2cWrite_PO3100K(0x92,0x40);  
i2cWrite_PO3100K(0x93,0x40);  
i2cWrite_PO3100K(0x94,0x40);  
i2cWrite_PO3100K(0x95,0xF6);  
i2cWrite_PO3100K(0x96,0xF8);  
i2cWrite_PO3100K(0x97,0x08);  
i2cWrite_PO3100K(0x98,0xF6);  
i2cWrite_PO3100K(0x99,0x00);  
i2cWrite_PO3100K(0x9A,0x80);  
i2cWrite_PO3100K(0x9B,0x00);  
i2cWrite_PO3100K(0x9C,0x80);  
i2cWrite_PO3100K(0x9D,0x00);  
i2cWrite_PO3100K(0x9E,0x00);  
i2cWrite_PO3100K(0x9F,0x00);  
i2cWrite_PO3100K(0xA0,0x00);  
i2cWrite_PO3100K(0xA1,0x01);  
i2cWrite_PO3100K(0xA2,0x02);  
i2cWrite_PO3100K(0xA3,0x80);  
i2cWrite_PO3100K(0xA4,0x00);  
i2cWrite_PO3100K(0xA5,0x01);  
i2cWrite_PO3100K(0xA6,0x00);  
i2cWrite_PO3100K(0xA7,0xF0);  
i2cWrite_PO3100K(0xA8,0x00);  
i2cWrite_PO3100K(0xA9,0x80);  
i2cWrite_PO3100K(0xAA,0x80);  
i2cWrite_PO3100K(0xAB,0x00);  
i2cWrite_PO3100K(0xAC,0x00);  
i2cWrite_PO3100K(0xAD,0x00);  
i2cWrite_PO3100K(0xAE,0x00);  
i2cWrite_PO3100K(0xAF,0x0C);  
i2cWrite_PO3100K(0xB0,0x04);  
i2cWrite_PO3100K(0xB1,0x4B);  
i2cWrite_PO3100K(0xB2,0x00);  
i2cWrite_PO3100K(0xB3,0x00);  
i2cWrite_PO3100K(0xB4,0x05);  
i2cWrite_PO3100K(0xB5,0x05);  
i2cWrite_PO3100K(0xB6,0x04);  
i2cWrite_PO3100K(0xB7,0x00);  
i2cWrite_PO3100K(0xB8,0x05);  
i2cWrite_PO3100K(0xB9,0x02);  
i2cWrite_PO3100K(0xBA,0xD4);  
i2cWrite_PO3100K(0xBB,0x01);  
i2cWrite_PO3100K(0xBC,0x3E);  
i2cWrite_PO3100K(0xBD,0x03);  
i2cWrite_PO3100K(0xBE,0xC6);  
i2cWrite_PO3100K(0xBF,0x01);  
i2cWrite_PO3100K(0xC0,0x5C);  
i2cWrite_PO3100K(0xC1,0x02);  
i2cWrite_PO3100K(0xC2,0x82);  
i2cWrite_PO3100K(0xC3,0x02);  
i2cWrite_PO3100K(0xC4,0x85);  
i2cWrite_PO3100K(0xC5,0x01);  
i2cWrite_PO3100K(0xC6,0x6D);  
i2cWrite_PO3100K(0xC7,0x00);  
i2cWrite_PO3100K(0xC8,0x00);  
i2cWrite_PO3100K(0xC9,0x05);  
i2cWrite_PO3100K(0xCA,0x10);  
i2cWrite_PO3100K(0xCB,0x00);  
i2cWrite_PO3100K(0xCC,0x00);  
i2cWrite_PO3100K(0xCD,0x02);  
i2cWrite_PO3100K(0xCE,0xE0);  
i2cWrite_PO3100K(0xCF,0x00);  
i2cWrite_PO3100K(0xD0,0x00);  
i2cWrite_PO3100K(0xD1,0x00);  
i2cWrite_PO3100K(0xD2,0x00);  
i2cWrite_PO3100K(0xD3,0x00);  
i2cWrite_PO3100K(0xD4,0x00);  
i2cWrite_PO3100K(0xD5,0x00);  
i2cWrite_PO3100K(0xD6,0x00);  
i2cWrite_PO3100K(0xD7,0x00);  
i2cWrite_PO3100K(0xD8,0x01);  
i2cWrite_PO3100K(0xD9,0xAF);  
i2cWrite_PO3100K(0xDA,0x03);  
i2cWrite_PO3100K(0xDB,0x5A);  
i2cWrite_PO3100K(0xDC,0x00);  
i2cWrite_PO3100K(0xDD,0xF5);  
i2cWrite_PO3100K(0xDE,0x01);  
i2cWrite_PO3100K(0xDF,0xE4);  
i2cWrite_PO3100K(0xE0,0x08);  
i2cWrite_PO3100K(0xE1,0x00);  
i2cWrite_PO3100K(0xE2,0x00);  
i2cWrite_PO3100K(0xE3,0x40);  
i2cWrite_PO3100K(0xE4,0x40);  
i2cWrite_PO3100K(0xE5,0x40);  
i2cWrite_PO3100K(0xE6,0x40);  
i2cWrite_PO3100K(0xE7,0x40);  
i2cWrite_PO3100K(0xE8,0x40);  
i2cWrite_PO3100K(0xE9,0x40);  
i2cWrite_PO3100K(0xEA,0x40);  
i2cWrite_PO3100K(0xEB,0x08);  
i2cWrite_PO3100K(0xEC,0x00);  
i2cWrite_PO3100K(0xED,0x80);  
i2cWrite_PO3100K(0xEE,0x80);  
i2cWrite_PO3100K(0xEF,0x04);  
i2cWrite_PO3100K(0xF0,0x04);  
i2cWrite_PO3100K(0xF1,0x04);  
i2cWrite_PO3100K(0xF2,0x00);  
i2cWrite_PO3100K(0xF3,0x3C);  
i2cWrite_PO3100K(0xF4,0x02);  
i2cWrite_PO3100K(0xF5,0x3A);  
i2cWrite_PO3100K(0xF6,0x00);  
i2cWrite_PO3100K(0xF7,0x00);  
i2cWrite_PO3100K(0xF8,0x00);  
i2cWrite_PO3100K(0xF9,0x00);  
i2cWrite_PO3100K(0xFA,0x00);  
i2cWrite_PO3100K(0xFB,0x00);  
i2cWrite_PO3100K(0xFC,0x00);  
i2cWrite_PO3100K(0xFD,0x00);  
i2cWrite_PO3100K(0xFE,0x00);  
i2cWrite_PO3100K(0xFF,0x00);  
                          
                          
//BANK D                   
                          
i2cWrite_PO3100K(0x03,0x03);                      
i2cWrite_PO3100K(0x04,0x25);                      
i2cWrite_PO3100K(0x05,0x00);                      
i2cWrite_PO3100K(0x06,0x00);                      
i2cWrite_PO3100K(0x07,0x1C);                      
i2cWrite_PO3100K(0x08,0x20);                      
i2cWrite_PO3100K(0x09,0x00);                      
i2cWrite_PO3100K(0x0A,0x00);                      
i2cWrite_PO3100K(0x0B,0x20);                      
i2cWrite_PO3100K(0x0C,0x25);                      
i2cWrite_PO3100K(0x0D,0x00);                      
i2cWrite_PO3100K(0x0E,0x00);                      
i2cWrite_PO3100K(0x0F,0x25);                      
i2cWrite_PO3100K(0x10,0x03);                      
i2cWrite_PO3100K(0x11,0x00);                      
i2cWrite_PO3100K(0x12,0x03);                      
i2cWrite_PO3100K(0x13,0x00);                      
i2cWrite_PO3100K(0x14,0x04);                      
i2cWrite_PO3100K(0x15,0x00);                      
i2cWrite_PO3100K(0x16,0x3A);                      
i2cWrite_PO3100K(0x17,0x50);                      
i2cWrite_PO3100K(0x18,0x60);                      
i2cWrite_PO3100K(0x19,0x25);                      
i2cWrite_PO3100K(0x1A,0x00);                      
i2cWrite_PO3100K(0x1B,0x00);                      
i2cWrite_PO3100K(0x1C,0x64);                      
i2cWrite_PO3100K(0x1D,0x00);                      
i2cWrite_PO3100K(0x1E,0x40);                      
i2cWrite_PO3100K(0x1F,0x00);                      
i2cWrite_PO3100K(0x20,0x62);                      
i2cWrite_PO3100K(0x21,0x00);                      
i2cWrite_PO3100K(0x22,0x00);                      
i2cWrite_PO3100K(0x23,0x00);                      
i2cWrite_PO3100K(0x24,0x00);                      
i2cWrite_PO3100K(0x25,0x00);                      
i2cWrite_PO3100K(0x26,0x00);                      
i2cWrite_PO3100K(0x27,0x10);                      
i2cWrite_PO3100K(0x28,0x20);                      
i2cWrite_PO3100K(0x29,0x01);                      
i2cWrite_PO3100K(0x2A,0x00);                      
i2cWrite_PO3100K(0x2B,0x0C);                      
i2cWrite_PO3100K(0x2C,0x1C);                      
i2cWrite_PO3100K(0x2D,0x01);                      
i2cWrite_PO3100K(0x2E,0x5F);                      
i2cWrite_PO3100K(0x2F,0x3F);                      
i2cWrite_PO3100K(0x30,0x3F);                      
i2cWrite_PO3100K(0x31,0x5B);                      
i2cWrite_PO3100K(0x32,0x40);                      
i2cWrite_PO3100K(0x33,0x40);                      
i2cWrite_PO3100K(0x34,0x40);                      
i2cWrite_PO3100K(0x35,0x40);                      
i2cWrite_PO3100K(0x36,0x1F);                      
i2cWrite_PO3100K(0x37,0x1F);                      
i2cWrite_PO3100K(0x38,0x1F);                      
i2cWrite_PO3100K(0x39,0x1F);                      
i2cWrite_PO3100K(0x3A,0xFF);                      
i2cWrite_PO3100K(0x3B,0xFF);                      
i2cWrite_PO3100K(0x3C,0xFF);                      
i2cWrite_PO3100K(0x3D,0xFF);                      
i2cWrite_PO3100K(0x3E,0x04);                      
i2cWrite_PO3100K(0x3F,0x08);                      
i2cWrite_PO3100K(0x40,0x18);                      
i2cWrite_PO3100K(0x41,0x04);                      
i2cWrite_PO3100K(0x42,0x00);                      
i2cWrite_PO3100K(0x43,0xFF);                      
i2cWrite_PO3100K(0x44,0xFF);                      
i2cWrite_PO3100K(0x45,0xFF);                      
i2cWrite_PO3100K(0x46,0xFF);                      
i2cWrite_PO3100K(0x47,0x50);                      
i2cWrite_PO3100K(0x48,0x50);                      
i2cWrite_PO3100K(0x49,0x50);                      
i2cWrite_PO3100K(0x4A,0x50);                      
i2cWrite_PO3100K(0x4B,0x00);                      
i2cWrite_PO3100K(0x4C,0x00);                      
i2cWrite_PO3100K(0x4D,0x00);                      
i2cWrite_PO3100K(0x4E,0x14);                      
i2cWrite_PO3100K(0x4F,0x20);                      
i2cWrite_PO3100K(0x50,0x20);                      
i2cWrite_PO3100K(0x51,0x15);                      
i2cWrite_PO3100K(0x52,0x00);                      
i2cWrite_PO3100K(0x53,0x10);                      
i2cWrite_PO3100K(0x54,0x20);                      
i2cWrite_PO3100K(0x55,0x01);                      
i2cWrite_PO3100K(0x56,0x00);                      
i2cWrite_PO3100K(0x57,0x04);                      
i2cWrite_PO3100K(0x58,0x08);                      
i2cWrite_PO3100K(0x59,0x00);                      
i2cWrite_PO3100K(0x5A,0x00);                      
i2cWrite_PO3100K(0x5B,0x80);                      
i2cWrite_PO3100K(0x5C,0x00);                      
i2cWrite_PO3100K(0x5D,0x05);                      
i2cWrite_PO3100K(0x5E,0x00);                      
i2cWrite_PO3100K(0x5F,0x00);                      
i2cWrite_PO3100K(0x60,0x51);                      
i2cWrite_PO3100K(0x61,0x51); 
i2cWrite_PO3100K(0x62,0x00); 
i2cWrite_PO3100K(0x63,0x00); 
i2cWrite_PO3100K(0x64,0x00); 
i2cWrite_PO3100K(0x65,0x0F); 
i2cWrite_PO3100K(0x66,0x0F); 
i2cWrite_PO3100K(0x67,0x0F); 
i2cWrite_PO3100K(0x68,0x0F); 
i2cWrite_PO3100K(0x69,0x04); 
i2cWrite_PO3100K(0x6A,0x04); 
i2cWrite_PO3100K(0x6B,0x04); 
i2cWrite_PO3100K(0x6C,0x04); 
i2cWrite_PO3100K(0x6D,0x7F); 
i2cWrite_PO3100K(0x6E,0x7F); 
i2cWrite_PO3100K(0x6F,0x7F); 
i2cWrite_PO3100K(0x70,0x7F); 
i2cWrite_PO3100K(0x71,0x7F); 
i2cWrite_PO3100K(0x72,0x7F); 
i2cWrite_PO3100K(0x73,0x7F); 
i2cWrite_PO3100K(0x74,0x7F); 
i2cWrite_PO3100K(0x75,0x00); 
i2cWrite_PO3100K(0x76,0x00); 
i2cWrite_PO3100K(0x77,0x07); 
i2cWrite_PO3100K(0x78,0x07); 
i2cWrite_PO3100K(0x79,0x07); 
i2cWrite_PO3100K(0x7A,0x07); 
i2cWrite_PO3100K(0x7B,0x10); 
i2cWrite_PO3100K(0x7C,0x16); 
i2cWrite_PO3100K(0x7D,0x20); 
i2cWrite_PO3100K(0x7E,0x10); 
i2cWrite_PO3100K(0x7F,0xFF); 
i2cWrite_PO3100K(0x80,0x00); 
i2cWrite_PO3100K(0x81,0x00); 
i2cWrite_PO3100K(0x82,0x34); 
i2cWrite_PO3100K(0x83,0x8B); 
i2cWrite_PO3100K(0x84,0x00); 
i2cWrite_PO3100K(0x85,0x00); 
i2cWrite_PO3100K(0x86,0x00); 
i2cWrite_PO3100K(0x87,0x00); 
i2cWrite_PO3100K(0x88,0x00); 
i2cWrite_PO3100K(0x89,0x00); 
i2cWrite_PO3100K(0x8A,0x00); 
i2cWrite_PO3100K(0x8B,0x00); 
i2cWrite_PO3100K(0x8C,0x00); 
i2cWrite_PO3100K(0x8D,0x00); 
i2cWrite_PO3100K(0x8E,0x01); 
i2cWrite_PO3100K(0x8F,0x77); 
i2cWrite_PO3100K(0x90,0xB4); 
i2cWrite_PO3100K(0x91,0x00); 
i2cWrite_PO3100K(0x92,0xBF); 
i2cWrite_PO3100K(0x93,0x3F); 
i2cWrite_PO3100K(0x94,0xAD); 
i2cWrite_PO3100K(0x95,0x00); 
i2cWrite_PO3100K(0x96,0x00); 
i2cWrite_PO3100K(0x97,0x37); 
i2cWrite_PO3100K(0x98,0x38); 
i2cWrite_PO3100K(0x99,0x38); 
i2cWrite_PO3100K(0x9A,0x37); 
i2cWrite_PO3100K(0x9B,0x37); 
i2cWrite_PO3100K(0x9C,0x34); 
i2cWrite_PO3100K(0x9D,0x34); 
i2cWrite_PO3100K(0x9E,0x36); 
i2cWrite_PO3100K(0x9F,0x00); 
i2cWrite_PO3100K(0xA0,0x00); 
i2cWrite_PO3100K(0xA1,0x1C); 
i2cWrite_PO3100K(0xA2,0x14); 
i2cWrite_PO3100K(0xA3,0x14); 
i2cWrite_PO3100K(0xA4,0x1B); 
i2cWrite_PO3100K(0xA5,0x00); 
i2cWrite_PO3100K(0xA6,0x1A); 
i2cWrite_PO3100K(0xA7,0x34); 
i2cWrite_PO3100K(0xA8,0x02); 
i2cWrite_PO3100K(0xA9,0x00); 
i2cWrite_PO3100K(0xAA,0x00); 
i2cWrite_PO3100K(0xAB,0x00); 
i2cWrite_PO3100K(0xAC,0x00); 
i2cWrite_PO3100K(0xAD,0x00); 
i2cWrite_PO3100K(0xAE,0x00); 
i2cWrite_PO3100K(0xAF,0x00); 
i2cWrite_PO3100K(0xB0,0x00); 
i2cWrite_PO3100K(0xB1,0x00); 
i2cWrite_PO3100K(0xB2,0x00); 
i2cWrite_PO3100K(0xB3,0x00); 
i2cWrite_PO3100K(0xB4,0x00); 
i2cWrite_PO3100K(0xB5,0x00); 
i2cWrite_PO3100K(0xB6,0x00); 
i2cWrite_PO3100K(0xB7,0x00); 
i2cWrite_PO3100K(0xB8,0x00); 
i2cWrite_PO3100K(0xB9,0x00); 
i2cWrite_PO3100K(0xBA,0x00); 
i2cWrite_PO3100K(0xBB,0x00); 
i2cWrite_PO3100K(0xBC,0x00); 
i2cWrite_PO3100K(0xBD,0x00); 
i2cWrite_PO3100K(0xBE,0x00); 
i2cWrite_PO3100K(0xBF,0x00); 
i2cWrite_PO3100K(0xC0,0x00); 
i2cWrite_PO3100K(0xC1,0x00); 
i2cWrite_PO3100K(0xC2,0x00); 
i2cWrite_PO3100K(0xC3,0x00); 
i2cWrite_PO3100K(0xC4,0x00); 
i2cWrite_PO3100K(0xC5,0x00); 
i2cWrite_PO3100K(0xC6,0x00); 
i2cWrite_PO3100K(0xC7,0x00); 
i2cWrite_PO3100K(0xC8,0x00); 
i2cWrite_PO3100K(0xC9,0x00); 
i2cWrite_PO3100K(0xCA,0x00); 
i2cWrite_PO3100K(0xCB,0x00); 
i2cWrite_PO3100K(0xCC,0x00); 
i2cWrite_PO3100K(0xCD,0x00); 
i2cWrite_PO3100K(0xCE,0x00); 
i2cWrite_PO3100K(0xCF,0x00); 
i2cWrite_PO3100K(0xD0,0x00); 
i2cWrite_PO3100K(0xD1,0x00); 
i2cWrite_PO3100K(0xD2,0x00); 
i2cWrite_PO3100K(0xD3,0x00); 
i2cWrite_PO3100K(0xD4,0x00); 
i2cWrite_PO3100K(0xD5,0x00); 
i2cWrite_PO3100K(0xD6,0x00); 
i2cWrite_PO3100K(0xD7,0x00); 
i2cWrite_PO3100K(0xD8,0x00); 
i2cWrite_PO3100K(0xD9,0x00); 
i2cWrite_PO3100K(0xDA,0x00); 
i2cWrite_PO3100K(0xDB,0x00); 
i2cWrite_PO3100K(0xDC,0x00); 
i2cWrite_PO3100K(0xDD,0x00); 
i2cWrite_PO3100K(0xDE,0x00); 
i2cWrite_PO3100K(0xDF,0x00); 
i2cWrite_PO3100K(0xE0,0x00); 
i2cWrite_PO3100K(0xE1,0x00); 
i2cWrite_PO3100K(0xE2,0x00); 
i2cWrite_PO3100K(0xE3,0x00); 
i2cWrite_PO3100K(0xE4,0x00); 
i2cWrite_PO3100K(0xE5,0x00); 
i2cWrite_PO3100K(0xE6,0x00); 
i2cWrite_PO3100K(0xE7,0x00); 
i2cWrite_PO3100K(0xE8,0x00); 
i2cWrite_PO3100K(0xE9,0x00); 
i2cWrite_PO3100K(0xEA,0x00); 
i2cWrite_PO3100K(0xEB,0x00); 
i2cWrite_PO3100K(0xEC,0x00); 
i2cWrite_PO3100K(0xED,0x00); 
i2cWrite_PO3100K(0xEE,0x00); 
i2cWrite_PO3100K(0xEF,0x00); 
i2cWrite_PO3100K(0xF0,0x00); 
i2cWrite_PO3100K(0xF1,0x00); 
i2cWrite_PO3100K(0xF2,0x00); 
i2cWrite_PO3100K(0xF3,0x00); 
i2cWrite_PO3100K(0xF4,0x00); 
i2cWrite_PO3100K(0xF5,0x00); 
i2cWrite_PO3100K(0xF6,0x00); 
i2cWrite_PO3100K(0xF7,0x00); 
i2cWrite_PO3100K(0xF8,0x00); 
i2cWrite_PO3100K(0xF9,0x00); 
i2cWrite_PO3100K(0xFA,0x00); 
i2cWrite_PO3100K(0xFB,0x00); 
i2cWrite_PO3100K(0xFC,0x00); 
i2cWrite_PO3100K(0xFD,0x00); 
i2cWrite_PO3100K(0xFE,0x00); 
i2cWrite_PO3100K(0xFF,0x00); 
                        
                        
//BANK E                  
                        
i2cWrite_PO3100K(0x03,0x04);                    
i2cWrite_PO3100K(0x04,0x98);                    
i2cWrite_PO3100K(0x05,0x64);                    
i2cWrite_PO3100K(0x06,0xB8);                
i2cWrite_PO3100K(0x07,0x80);                
i2cWrite_PO3100K(0x08,0x00);                
i2cWrite_PO3100K(0x09,0x01);                
i2cWrite_PO3100K(0x0A,0x20);                
i2cWrite_PO3100K(0x0B,0x80);                
i2cWrite_PO3100K(0x0C,0x00);                
i2cWrite_PO3100K(0x0D,0x00);                
i2cWrite_PO3100K(0x0E,0x00);                
i2cWrite_PO3100K(0x0F,0x20);                
i2cWrite_PO3100K(0x10,0x40);                
i2cWrite_PO3100K(0x11,0xEE);                
i2cWrite_PO3100K(0x12,0x02);                
i2cWrite_PO3100K(0x13,0xE8);
i2cWrite_PO3100K(0x14,0x02);
i2cWrite_PO3100K(0x15,0xE8);
i2cWrite_PO3100K(0x16,0x02);
i2cWrite_PO3100K(0x17,0xE8);
i2cWrite_PO3100K(0x18,0x00);
i2cWrite_PO3100K(0x19,0x00);
i2cWrite_PO3100K(0x1A,0x20);
i2cWrite_PO3100K(0x1B,0x00);
i2cWrite_PO3100K(0x1C,0x9F);
i2cWrite_PO3100K(0x1D,0xD8);
i2cWrite_PO3100K(0x1E,0x00);
i2cWrite_PO3100K(0x1F,0xBF);
i2cWrite_PO3100K(0x20,0xD0);
i2cWrite_PO3100K(0x21,0x00);
i2cWrite_PO3100K(0x22,0x00);
i2cWrite_PO3100K(0x23,0x80);
i2cWrite_PO3100K(0x24,0x00);
i2cWrite_PO3100K(0x25,0x01);
i2cWrite_PO3100K(0x26,0x00);
i2cWrite_PO3100K(0x27,0x00);
i2cWrite_PO3100K(0x28,0x08);
i2cWrite_PO3100K(0x29,0x6E);
i2cWrite_PO3100K(0x2A,0xB1);
i2cWrite_PO3100K(0x2B,0x00);
i2cWrite_PO3100K(0x2C,0xBB);
i2cWrite_PO3100K(0x2D,0x00);
i2cWrite_PO3100K(0x2E,0x03);
i2cWrite_PO3100K(0x2F,0x15);
i2cWrite_PO3100K(0x30,0x08);
i2cWrite_PO3100K(0x31,0x08);
i2cWrite_PO3100K(0x32,0x08);
i2cWrite_PO3100K(0x33,0x08);
i2cWrite_PO3100K(0x34,0x08);
i2cWrite_PO3100K(0x35,0x10);
i2cWrite_PO3100K(0x36,0x40);
i2cWrite_PO3100K(0x37,0x80);
i2cWrite_PO3100K(0x38,0xFF);
i2cWrite_PO3100K(0x39,0x00);
i2cWrite_PO3100K(0x3A,0x8B);
i2cWrite_PO3100K(0x3B,0x90);
i2cWrite_PO3100K(0x3C,0x70);
i2cWrite_PO3100K(0x3D,0x80);
i2cWrite_PO3100K(0x3E,0x68);
i2cWrite_PO3100K(0x3F,0x24);
i2cWrite_PO3100K(0x40,0x4B);
i2cWrite_PO3100K(0x41,0x04);
i2cWrite_PO3100K(0x42,0x10);
i2cWrite_PO3100K(0x43,0x1A);
i2cWrite_PO3100K(0x44,0x25);
i2cWrite_PO3100K(0x45,0x04);
i2cWrite_PO3100K(0x46,0x8E);
i2cWrite_PO3100K(0x47,0x80);
i2cWrite_PO3100K(0x48,0x08);
i2cWrite_PO3100K(0x49,0x08);
i2cWrite_PO3100K(0x4A,0x11);
i2cWrite_PO3100K(0x4B,0x01);
i2cWrite_PO3100K(0x4C,0xFF);
i2cWrite_PO3100K(0x4D,0xFF);
i2cWrite_PO3100K(0x4E,0x00);
i2cWrite_PO3100K(0x4F,0x00);
i2cWrite_PO3100K(0x50,0x03);
i2cWrite_PO3100K(0x51,0x10);
i2cWrite_PO3100K(0x52,0xE0);
i2cWrite_PO3100K(0x53,0x02);
i2cWrite_PO3100K(0x54,0x02);
i2cWrite_PO3100K(0x55,0x40);
i2cWrite_PO3100K(0x56,0xC0);
i2cWrite_PO3100K(0x57,0x04);
i2cWrite_PO3100K(0x58,0x6E);
i2cWrite_PO3100K(0x59,0x45);
i2cWrite_PO3100K(0x5A,0x23);
i2cWrite_PO3100K(0x5B,0x46);
i2cWrite_PO3100K(0x5C,0x5A);
i2cWrite_PO3100K(0x5D,0xC8);
i2cWrite_PO3100K(0x5E,0x19);
i2cWrite_PO3100K(0x5F,0x28);
i2cWrite_PO3100K(0x60,0x2D);
i2cWrite_PO3100K(0x61,0x69);
i2cWrite_PO3100K(0x62,0x32);
i2cWrite_PO3100K(0x63,0x78);
i2cWrite_PO3100K(0x64,0x2D);
i2cWrite_PO3100K(0x65,0x19);
i2cWrite_PO3100K(0x66,0x00);
i2cWrite_PO3100K(0x67,0x00);
i2cWrite_PO3100K(0x68,0x80);
i2cWrite_PO3100K(0x69,0x80);
i2cWrite_PO3100K(0x6A,0x80);
i2cWrite_PO3100K(0x6B,0x80);
i2cWrite_PO3100K(0x6C,0x80);
i2cWrite_PO3100K(0x6D,0x80);
i2cWrite_PO3100K(0x6E,0x3A);
i2cWrite_PO3100K(0x6F,0x50);
i2cWrite_PO3100K(0x70,0x60);
i2cWrite_PO3100K(0x71,0x80);
i2cWrite_PO3100K(0x72,0x80);
i2cWrite_PO3100K(0x73,0x08);
i2cWrite_PO3100K(0x74,0x04);
i2cWrite_PO3100K(0x75,0x28);
i2cWrite_PO3100K(0x76,0x28);
i2cWrite_PO3100K(0x77,0x78);
i2cWrite_PO3100K(0x78,0x78);
i2cWrite_PO3100K(0x79,0x48);
i2cWrite_PO3100K(0x7A,0x48);
i2cWrite_PO3100K(0x7B,0xB8);
i2cWrite_PO3100K(0x7C,0xB8);
i2cWrite_PO3100K(0x7D,0x01);
i2cWrite_PO3100K(0x7E,0x00);
i2cWrite_PO3100K(0x7F,0x02);
i2cWrite_PO3100K(0x80,0x07);
i2cWrite_PO3100K(0x81,0x00);
i2cWrite_PO3100K(0x82,0x03);
i2cWrite_PO3100K(0x83,0x20);
i2cWrite_PO3100K(0x84,0x40);
i2cWrite_PO3100K(0x85,0x0C);
i2cWrite_PO3100K(0x86,0x18);
i2cWrite_PO3100K(0x87,0x20);
i2cWrite_PO3100K(0x88,0x06);
i2cWrite_PO3100K(0x89,0x00);
i2cWrite_PO3100K(0x8A,0x00);
i2cWrite_PO3100K(0x8B,0x00);
i2cWrite_PO3100K(0x8C,0x00);
i2cWrite_PO3100K(0x8D,0x00);
i2cWrite_PO3100K(0x8E,0x00);
i2cWrite_PO3100K(0x8F,0x00);
i2cWrite_PO3100K(0x90,0x90);
i2cWrite_PO3100K(0x91,0x80);
i2cWrite_PO3100K(0x92,0x00);
i2cWrite_PO3100K(0x93,0xBF);
i2cWrite_PO3100K(0x94,0xD0);
i2cWrite_PO3100K(0x95,0x00);
i2cWrite_PO3100K(0x96,0x0C);
i2cWrite_PO3100K(0x97,0x2A);
i2cWrite_PO3100K(0x98,0x00);
i2cWrite_PO3100K(0x99,0x30);
i2cWrite_PO3100K(0x9A,0xA8);
i2cWrite_PO3100K(0x9B,0x00);
i2cWrite_PO3100K(0x9C,0x00);
i2cWrite_PO3100K(0x9D,0x00);
i2cWrite_PO3100K(0x9E,0x06);
i2cWrite_PO3100K(0x9F,0x0A);
i2cWrite_PO3100K(0xA0,0x0D);
i2cWrite_PO3100K(0xA1,0x0F);
i2cWrite_PO3100K(0xA2,0x00);
i2cWrite_PO3100K(0xA3,0x64);
i2cWrite_PO3100K(0xA4,0x64);
i2cWrite_PO3100K(0xA5,0x63);
i2cWrite_PO3100K(0xA6,0x74);
i2cWrite_PO3100K(0xA7,0x28);
i2cWrite_PO3100K(0xA8,0x40);
i2cWrite_PO3100K(0xA9,0x29);
i2cWrite_PO3100K(0xAA,0x00);
i2cWrite_PO3100K(0xAB,0x00);
i2cWrite_PO3100K(0xAC,0x00);
i2cWrite_PO3100K(0xAD,0x00);
i2cWrite_PO3100K(0xAE,0x00);
i2cWrite_PO3100K(0xAF,0x00);
i2cWrite_PO3100K(0xB0,0x00);
i2cWrite_PO3100K(0xB1,0x00);
i2cWrite_PO3100K(0xB2,0x00);
i2cWrite_PO3100K(0xB3,0x00);
i2cWrite_PO3100K(0xB4,0x00);
i2cWrite_PO3100K(0xB5,0x00);
i2cWrite_PO3100K(0xB6,0x00);
i2cWrite_PO3100K(0xB7,0x03);
i2cWrite_PO3100K(0xB8,0x6E);
i2cWrite_PO3100K(0xB9,0x49);
i2cWrite_PO3100K(0xBA,0x03);
i2cWrite_PO3100K(0xBB,0x6E);
i2cWrite_PO3100K(0xBC,0x9E);
i2cWrite_PO3100K(0xBD,0x02);
i2cWrite_PO3100K(0xBE,0x11);
i2cWrite_PO3100K(0xBF,0xE6);
i2cWrite_PO3100K(0xC0,0x02);
i2cWrite_PO3100K(0xC1,0x17);
i2cWrite_PO3100K(0xC2,0x54);
i2cWrite_PO3100K(0xC3,0x02);
i2cWrite_PO3100K(0xC4,0xEB);
i2cWrite_PO3100K(0xC5,0xDF);
i2cWrite_PO3100K(0xC6,0x00);
i2cWrite_PO3100K(0xC7,0xAC);
i2cWrite_PO3100K(0xC8,0x8C);
i2cWrite_PO3100K(0xC9,0x00);
i2cWrite_PO3100K(0xCA,0x98);
i2cWrite_PO3100K(0xCB,0xA3);
i2cWrite_PO3100K(0xCC,0x02);
i2cWrite_PO3100K(0xCD,0x0D);
i2cWrite_PO3100K(0xCE,0xFC);
i2cWrite_PO3100K(0xCF,0x22);
i2cWrite_PO3100K(0xD0,0x02);
i2cWrite_PO3100K(0xD1,0x0C);
i2cWrite_PO3100K(0xD2,0xD5);
i2cWrite_PO3100K(0xD3,0x81);
i2cWrite_PO3100K(0xD4,0x01);
i2cWrite_PO3100K(0xD5,0x0A);
i2cWrite_PO3100K(0xD6,0x48);
i2cWrite_PO3100K(0xD7,0x04);
i2cWrite_PO3100K(0xD8,0x00);
i2cWrite_PO3100K(0xD9,0xC0);
i2cWrite_PO3100K(0xDA,0x7D);
i2cWrite_PO3100K(0xDB,0x21);
i2cWrite_PO3100K(0xDC,0x01);
i2cWrite_PO3100K(0xDD,0xEF);
i2cWrite_PO3100K(0xDE,0xB1);
i2cWrite_PO3100K(0xDF,0x49);
i2cWrite_PO3100K(0xE0,0xD1);
i2cWrite_PO3100K(0xE1,0x00);
i2cWrite_PO3100K(0xE2,0x00);
i2cWrite_PO3100K(0xE3,0x29);
i2cWrite_PO3100K(0xE4,0x40);
i2cWrite_PO3100K(0xE5,0x29);
i2cWrite_PO3100K(0xE6,0x52);
i2cWrite_PO3100K(0xE7,0x81);
i2cWrite_PO3100K(0xE8,0x53);
i2cWrite_PO3100K(0xE9,0x00);
i2cWrite_PO3100K(0xEA,0x00);
i2cWrite_PO3100K(0xEB,0x00);
i2cWrite_PO3100K(0xEC,0x00);
i2cWrite_PO3100K(0xED,0x00);
i2cWrite_PO3100K(0xEE,0x00);
i2cWrite_PO3100K(0xEF,0x00);
i2cWrite_PO3100K(0xF0,0x00);
i2cWrite_PO3100K(0xF1,0x00);
i2cWrite_PO3100K(0xF2,0x00);
i2cWrite_PO3100K(0xF3,0x00);
i2cWrite_PO3100K(0xF4,0x00);
i2cWrite_PO3100K(0xF5,0x00);
i2cWrite_PO3100K(0xF6,0x00);
i2cWrite_PO3100K(0xF7,0x00);
i2cWrite_PO3100K(0xF8,0x00);
i2cWrite_PO3100K(0xF9,0x00);
i2cWrite_PO3100K(0xFA,0x00);
i2cWrite_PO3100K(0xFB,0x00);
i2cWrite_PO3100K(0xFC,0x00);
i2cWrite_PO3100K(0xFD,0x00);
i2cWrite_PO3100K(0xFE,0x00);
i2cWrite_PO3100K(0xFF,0x00);
                        
                        
//BANK F                 
                        
i2cWrite_PO3100K(0x03,0x05);
i2cWrite_PO3100K(0x04,0x00);
i2cWrite_PO3100K(0x05,0x00);
i2cWrite_PO3100K(0x06,0x00);
i2cWrite_PO3100K(0x07,0x00);
i2cWrite_PO3100K(0x08,0x00);
i2cWrite_PO3100K(0x09,0x00);
i2cWrite_PO3100K(0x0A,0x7F);
i2cWrite_PO3100K(0x0B,0x00);
i2cWrite_PO3100K(0x0C,0xAD);
i2cWrite_PO3100K(0x0D,0x02);
i2cWrite_PO3100K(0x0E,0x09);
i2cWrite_PO3100K(0x0F,0x0A);
i2cWrite_PO3100K(0x10,0x0E);
i2cWrite_PO3100K(0x11,0x11);
i2cWrite_PO3100K(0x12,0x04);
i2cWrite_PO3100K(0x13,0x18);
i2cWrite_PO3100K(0x14,0x21);
i2cWrite_PO3100K(0x15,0x06);
i2cWrite_PO3100K(0x16,0x00);
i2cWrite_PO3100K(0x17,0xFE);
i2cWrite_PO3100K(0x18,0x44);
i2cWrite_PO3100K(0x19,0x47);
i2cWrite_PO3100K(0x1A,0x07);
i2cWrite_PO3100K(0x1B,0x0C);
i2cWrite_PO3100K(0x1C,0x15);
i2cWrite_PO3100K(0x1D,0x02);
i2cWrite_PO3100K(0x1E,0x41);
i2cWrite_PO3100K(0x1F,0x00);
i2cWrite_PO3100K(0x20,0x45);
i2cWrite_PO3100K(0x21,0x00);
i2cWrite_PO3100K(0x22,0x46);
i2cWrite_PO3100K(0x23,0x00);
i2cWrite_PO3100K(0x24,0x89);
i2cWrite_PO3100K(0x25,0x00);
i2cWrite_PO3100K(0x26,0x8C);
i2cWrite_PO3100K(0x27,0x02);
i2cWrite_PO3100K(0x28,0x83);
i2cWrite_PO3100K(0x29,0x00);
i2cWrite_PO3100K(0x2A,0x91);
i2cWrite_PO3100K(0x2B,0x00);
i2cWrite_PO3100K(0x2C,0xDA);
i2cWrite_PO3100K(0x2D,0x02);
i2cWrite_PO3100K(0x2E,0xC5);
i2cWrite_PO3100K(0x2F,0x00);
i2cWrite_PO3100K(0x30,0xDE);
i2cWrite_PO3100K(0x31,0x01);
i2cWrite_PO3100K(0x32,0x40);
i2cWrite_PO3100K(0x33,0x00);
i2cWrite_PO3100K(0x34,0x00);
i2cWrite_PO3100K(0x35,0x00);
i2cWrite_PO3100K(0x36,0x03);
i2cWrite_PO3100K(0x37,0xFF);
i2cWrite_PO3100K(0x38,0x03);
i2cWrite_PO3100K(0x39,0xFF);
i2cWrite_PO3100K(0x3A,0x03);
i2cWrite_PO3100K(0x3B,0xFF);
i2cWrite_PO3100K(0x3C,0x03);
i2cWrite_PO3100K(0x3D,0xFF);
i2cWrite_PO3100K(0x3E,0x03);
i2cWrite_PO3100K(0x3F,0xFF);
i2cWrite_PO3100K(0x40,0x03);
i2cWrite_PO3100K(0x41,0xFF);
i2cWrite_PO3100K(0x42,0x03);
i2cWrite_PO3100K(0x43,0xFF);
i2cWrite_PO3100K(0x44,0x03);
i2cWrite_PO3100K(0x45,0xFF);
i2cWrite_PO3100K(0x46,0x01);
i2cWrite_PO3100K(0x47,0xFF);
i2cWrite_PO3100K(0x48,0x01);
i2cWrite_PO3100K(0x49,0xFF);
i2cWrite_PO3100K(0x4A,0x01);
i2cWrite_PO3100K(0x4B,0xFF);
i2cWrite_PO3100K(0x4C,0x01);
i2cWrite_PO3100K(0x4D,0xFF);
i2cWrite_PO3100K(0x4E,0x01);
i2cWrite_PO3100K(0x4F,0xFF);
i2cWrite_PO3100K(0x50,0x01);
i2cWrite_PO3100K(0x51,0xFF);
i2cWrite_PO3100K(0x52,0x01);
i2cWrite_PO3100K(0x53,0xFF);
i2cWrite_PO3100K(0x54,0x01);
i2cWrite_PO3100K(0x55,0xFF);
i2cWrite_PO3100K(0x56,0x00);
i2cWrite_PO3100K(0x57,0x00);
i2cWrite_PO3100K(0x58,0x00);
i2cWrite_PO3100K(0x59,0x00);
i2cWrite_PO3100K(0x5A,0x00);
i2cWrite_PO3100K(0x5B,0x00);
i2cWrite_PO3100K(0x5C,0x00);
i2cWrite_PO3100K(0x5D,0x00);
i2cWrite_PO3100K(0x5E,0x00);
i2cWrite_PO3100K(0x5F,0x00);
i2cWrite_PO3100K(0x60,0x00);
i2cWrite_PO3100K(0x61,0x00);
i2cWrite_PO3100K(0x62,0x00);
i2cWrite_PO3100K(0x63,0x00);
i2cWrite_PO3100K(0x64,0x00);
i2cWrite_PO3100K(0x65,0x00);
i2cWrite_PO3100K(0x66,0x00);
i2cWrite_PO3100K(0x67,0x00);
i2cWrite_PO3100K(0x68,0x00);
i2cWrite_PO3100K(0x69,0x00);
i2cWrite_PO3100K(0x6A,0x00);
i2cWrite_PO3100K(0x6B,0x0F);
i2cWrite_PO3100K(0x6C,0x00);
i2cWrite_PO3100K(0x6D,0x00);
i2cWrite_PO3100K(0x6E,0x00);
i2cWrite_PO3100K(0x6F,0x00);
i2cWrite_PO3100K(0x70,0x00);
i2cWrite_PO3100K(0x71,0x00);
i2cWrite_PO3100K(0x72,0x00);
i2cWrite_PO3100K(0x73,0x00);
i2cWrite_PO3100K(0x74,0x00);
i2cWrite_PO3100K(0x75,0x00);
i2cWrite_PO3100K(0x76,0x00);
i2cWrite_PO3100K(0x77,0x00);
i2cWrite_PO3100K(0x78,0x00);
i2cWrite_PO3100K(0x79,0x00);
i2cWrite_PO3100K(0x7A,0x00);
i2cWrite_PO3100K(0x7B,0x00);
i2cWrite_PO3100K(0x7C,0x00);
i2cWrite_PO3100K(0x7D,0x00);
i2cWrite_PO3100K(0x7E,0x00);
i2cWrite_PO3100K(0x7F,0x00);
i2cWrite_PO3100K(0x80,0x00);
i2cWrite_PO3100K(0x81,0x00);
i2cWrite_PO3100K(0x82,0x00);
i2cWrite_PO3100K(0x83,0x00);
i2cWrite_PO3100K(0x84,0x00);
i2cWrite_PO3100K(0x85,0x00);
i2cWrite_PO3100K(0x86,0x00);
i2cWrite_PO3100K(0x87,0x00);
i2cWrite_PO3100K(0x88,0x00);
i2cWrite_PO3100K(0x89,0x00);
i2cWrite_PO3100K(0x8A,0x00);
i2cWrite_PO3100K(0x8B,0x00);
i2cWrite_PO3100K(0x8C,0x00);
i2cWrite_PO3100K(0x8D,0x00);
i2cWrite_PO3100K(0x8E,0x00);
i2cWrite_PO3100K(0x8F,0x00);
i2cWrite_PO3100K(0x90,0x00);
i2cWrite_PO3100K(0x91,0x01);
i2cWrite_PO3100K(0x92,0x01);
i2cWrite_PO3100K(0x93,0x01);
i2cWrite_PO3100K(0x94,0x40);
i2cWrite_PO3100K(0x95,0x40);
i2cWrite_PO3100K(0x96,0x40);
i2cWrite_PO3100K(0x97,0x40);
i2cWrite_PO3100K(0x98,0x00);
i2cWrite_PO3100K(0x99,0x00);
i2cWrite_PO3100K(0x9A,0x00);
i2cWrite_PO3100K(0x9B,0x00);
i2cWrite_PO3100K(0x9C,0x00);
i2cWrite_PO3100K(0x9D,0x00);
i2cWrite_PO3100K(0x9E,0x00);
i2cWrite_PO3100K(0x9F,0x00);
i2cWrite_PO3100K(0xA0,0x00);
i2cWrite_PO3100K(0xA1,0x00);
i2cWrite_PO3100K(0xA2,0x00);
i2cWrite_PO3100K(0xA3,0x06);
i2cWrite_PO3100K(0xA4,0xD8);
i2cWrite_PO3100K(0xA5,0x02);
i2cWrite_PO3100K(0xA6,0x05);
i2cWrite_PO3100K(0xA7,0x00);
i2cWrite_PO3100K(0xA8,0x1E);
i2cWrite_PO3100K(0xA9,0x1E);
i2cWrite_PO3100K(0xAA,0x00);
i2cWrite_PO3100K(0xAB,0xFE);
i2cWrite_PO3100K(0xAC,0x00);
i2cWrite_PO3100K(0xAD,0x01);
i2cWrite_PO3100K(0xAE,0x00);
i2cWrite_PO3100K(0xAF,0x00);
i2cWrite_PO3100K(0xB0,0x05);
i2cWrite_PO3100K(0xB1,0x01);
i2cWrite_PO3100K(0xB2,0x0A);
i2cWrite_PO3100K(0xB3,0x01);
i2cWrite_PO3100K(0xB4,0x00);
i2cWrite_PO3100K(0xB5,0x00);
i2cWrite_PO3100K(0xB6,0x00);
i2cWrite_PO3100K(0xB7,0x32);
i2cWrite_PO3100K(0xB8,0x00);
i2cWrite_PO3100K(0xB9,0x64);
i2cWrite_PO3100K(0xBA,0x00);
i2cWrite_PO3100K(0xBB,0x37);
i2cWrite_PO3100K(0xBC,0x00);
i2cWrite_PO3100K(0xBD,0x5A);
i2cWrite_PO3100K(0xBE,0x00);
i2cWrite_PO3100K(0xBF,0x00);
i2cWrite_PO3100K(0xC0,0x00);
i2cWrite_PO3100K(0xC1,0x64);
i2cWrite_PO3100K(0xC2,0x0A);
i2cWrite_PO3100K(0xC3,0x05);
i2cWrite_PO3100K(0xC4,0x00);
i2cWrite_PO3100K(0xC5,0x00);
i2cWrite_PO3100K(0xC6,0x1F);
i2cWrite_PO3100K(0xC7,0x1E);
i2cWrite_PO3100K(0xC8,0x1C);
i2cWrite_PO3100K(0xC9,0x18);
i2cWrite_PO3100K(0xCA,0x10);
i2cWrite_PO3100K(0xCB,0x00);
i2cWrite_PO3100K(0xCC,0xFF);
i2cWrite_PO3100K(0xCD,0x03);
i2cWrite_PO3100K(0xCE,0x00);
i2cWrite_PO3100K(0xCF,0x00);
i2cWrite_PO3100K(0xD0,0x00);
i2cWrite_PO3100K(0xD1,0x00);
i2cWrite_PO3100K(0xD2,0x00);
i2cWrite_PO3100K(0xD3,0x00);
i2cWrite_PO3100K(0xD4,0x00);
i2cWrite_PO3100K(0xD5,0x00);
i2cWrite_PO3100K(0xD6,0x00);
i2cWrite_PO3100K(0xD7,0x00);
i2cWrite_PO3100K(0xD8,0x00);
i2cWrite_PO3100K(0xD9,0x00);
i2cWrite_PO3100K(0xDA,0x00);
i2cWrite_PO3100K(0xDB,0x00);
i2cWrite_PO3100K(0xDC,0x00);
i2cWrite_PO3100K(0xDD,0x00);
i2cWrite_PO3100K(0xDE,0x00);
i2cWrite_PO3100K(0xDF,0x00);
i2cWrite_PO3100K(0xE0,0x00);
i2cWrite_PO3100K(0xE1,0x00);
i2cWrite_PO3100K(0xE2,0x00);
i2cWrite_PO3100K(0xE3,0x00);
i2cWrite_PO3100K(0xE4,0x00);
i2cWrite_PO3100K(0xE5,0x00);
i2cWrite_PO3100K(0xE6,0x00);
i2cWrite_PO3100K(0xE7,0x00);
i2cWrite_PO3100K(0xE8,0x00);
i2cWrite_PO3100K(0xE9,0x00);
i2cWrite_PO3100K(0xEA,0x00);
i2cWrite_PO3100K(0xEB,0x00);
i2cWrite_PO3100K(0xEC,0x00);
i2cWrite_PO3100K(0xED,0x00);
i2cWrite_PO3100K(0xEE,0x00);
i2cWrite_PO3100K(0xEF,0x00);
i2cWrite_PO3100K(0xF0,0x00);
i2cWrite_PO3100K(0xF1,0x00);
i2cWrite_PO3100K(0xF2,0x00);
i2cWrite_PO3100K(0xF3,0x00);
i2cWrite_PO3100K(0xF4,0x00);
i2cWrite_PO3100K(0xF5,0x00);
i2cWrite_PO3100K(0xF6,0x00);
i2cWrite_PO3100K(0xF7,0x00);
i2cWrite_PO3100K(0xF8,0x00);
i2cWrite_PO3100K(0xF9,0x00);
i2cWrite_PO3100K(0xFA,0x00);
i2cWrite_PO3100K(0xFB,0x00);
i2cWrite_PO3100K(0xFC,0x00);
i2cWrite_PO3100K(0xFD,0x00);
i2cWrite_PO3100K(0xFE,0x00);
i2cWrite_PO3100K(0xFF,0x00);
                      
                      
//////////////////////	// 0730    
                      
//Color saturation weight
i2cWrite_PO3100K(0x03,0x03);
i2cWrite_PO3100K(0x19,0x30);                 
                      
//sharpness control    
i2cWrite_PO3100K(0x03,0x02);               
i2cWrite_PO3100K(0x2F,0x24);                
i2cWrite_PO3100K(0x30,0x78);                 
i2cWrite_PO3100K(0x31,0x78);               
                      
//Y target control     
i2cWrite_PO3100K(0x03,0x04);                
i2cWrite_PO3100K(0x05,0x64);                 
i2cWrite_PO3100K(0x3B,0x98);                 
i2cWrite_PO3100K(0x3C,0x68);                 
i2cWrite_PO3100K(0x3D,0x68);                 
i2cWrite_PO3100K(0x3E,0x60);                 
i2cWrite_PO3100K(0x3F,0x24);                
i2cWrite_PO3100K(0x40,0x4B);                 
                      
//gamma curve fitting  
i2cWrite_PO3100K(0x03,0x02);                 
i2cWrite_PO3100K(0x3D,0x00);                 
i2cWrite_PO3100K(0x3E,0x0A);                 
i2cWrite_PO3100K(0x3F,0x1B);                 
i2cWrite_PO3100K(0x40,0x28);                 
i2cWrite_PO3100K(0x41,0x33);                 
i2cWrite_PO3100K(0x42,0x43);                 
i2cWrite_PO3100K(0x43,0x4F);                 
i2cWrite_PO3100K(0x44,0x65);                 
i2cWrite_PO3100K(0x45,0x77);                 
i2cWrite_PO3100K(0x46,0x95);                 
i2cWrite_PO3100K(0x47,0xAE);                 
i2cWrite_PO3100K(0x48,0xC5);                 
i2cWrite_PO3100K(0x49,0xDA);                 
i2cWrite_PO3100K(0x4A,0xED);                 
i2cWrite_PO3100K(0x4B,0xFF);                 
                      
//gamma curve fitting  
i2cWrite_PO3100K(0x03,0x02);                 
i2cWrite_PO3100K(0x5B,0x00);                 
i2cWrite_PO3100K(0x5C,0x04);                 
i2cWrite_PO3100K(0x5D,0x11);                 
i2cWrite_PO3100K(0x5E,0x20);                 
i2cWrite_PO3100K(0x5F,0x2D);                 
i2cWrite_PO3100K(0x60,0x42);                 
i2cWrite_PO3100K(0x61,0x52);                 
i2cWrite_PO3100K(0x62,0x6A);                 
i2cWrite_PO3100K(0x63,0x7D);                 
i2cWrite_PO3100K(0x64,0x9B);                 
i2cWrite_PO3100K(0x65,0xB4);
i2cWrite_PO3100K(0x66,0xC9);
i2cWrite_PO3100K(0x67,0xDD);
i2cWrite_PO3100K(0x68,0xEF);
i2cWrite_PO3100K(0x69,0xFF);

//Color saturation weight
i2cWrite_PO3100K(0x03,0x03);
i2cWrite_PO3100K(0x19,0x24);

//ybrightness
i2cWrite_PO3100K(0x03,0x02);
i2cWrite_PO3100K(0x95,0xF0);
i2cWrite_PO3100K(0x96,0xF8);
i2cWrite_PO3100K(0x97,0x08);

//ycont_slope2
i2cWrite_PO3100K(0x03,0x03);
i2cWrite_PO3100K(0x9B,0x40);
i2cWrite_PO3100K(0x9C,0x3C);
i2cWrite_PO3100K(0x9D,0x34);

//dark_e_blf
i2cWrite_PO3100K(0x03,0x03);
i2cWrite_PO3100K(0xA1,0x3F);
i2cWrite_PO3100K(0xA2,0x14);
i2cWrite_PO3100K(0xA3,0x14);

// Set AWB Sampling Boundary
i2cWrite_PO3100K(0x03,0x04);
i2cWrite_PO3100K(0x51,0x10);
i2cWrite_PO3100K(0x52,0xE0);
i2cWrite_PO3100K(0x53,0x02);
i2cWrite_PO3100K(0x54,0x02);
i2cWrite_PO3100K(0x55,0x40);
i2cWrite_PO3100K(0x56,0xC0);
i2cWrite_PO3100K(0x57,0x04);
i2cWrite_PO3100K(0x58,0x6E);
i2cWrite_PO3100K(0x59,0x45);

i2cWrite_PO3100K(0x03,0x04);
i2cWrite_PO3100K(0x5A,0x23);
i2cWrite_PO3100K(0x5B,0x55);
i2cWrite_PO3100K(0x5C,0x5A);
i2cWrite_PO3100K(0x5D,0xC8);
i2cWrite_PO3100K(0x5E,0x41);
i2cWrite_PO3100K(0x5F,0x28);
i2cWrite_PO3100K(0x60,0x2D);
i2cWrite_PO3100K(0x61,0x73);
i2cWrite_PO3100K(0x62,0x32);
i2cWrite_PO3100K(0x63,0x78);
i2cWrite_PO3100K(0x64,0x2D);
i2cWrite_PO3100K(0x65,0x19);
                    
//Color saturation   
i2cWrite_PO3100K(0x03,0x03);
i2cWrite_PO3100K(0x04,0x25);
i2cWrite_PO3100K(0x05,0x00);
i2cWrite_PO3100K(0x06,0x00);
i2cWrite_PO3100K(0x07,0x25);

siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);

}

void SetPO3100K_720P_30FPS(void)
{
    int i;

    DEBUG_SIU("SetPO3100K_720P_30FPS()\n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

#if 0   // 20140918
#else // 20150514
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    #if SENSOR_ROW_COL_MIRROR
    i2cWrite_PO3100K(0x05,0x03);
    #else
    i2cWrite_PO3100K(0x05,0x00);
    #endif
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x81);
    i2cWrite_PO3100K(0xA5,0x81);
    i2cWrite_PO3100K(0xA6,0x81);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x36);
    i2cWrite_PO3100K(0x34,0x89);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8C);
    i2cWrite_PO3100K(0x37,0x3A);
    i2cWrite_PO3100K(0x38,0x8D);
    i2cWrite_PO3100K(0x39,0x83);
    i2cWrite_PO3100K(0x3A,0x99);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x04,0x25);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x06,0x00);
    i2cWrite_PO3100K(0x07,0x1C);
    i2cWrite_PO3100K(0x08,0x1E);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x20);
    i2cWrite_PO3100K(0x0C,0x28);
    i2cWrite_PO3100K(0x0D,0x00);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x0B);
    i2cWrite_PO3100K(0x3E,0x2B);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x3F);
    i2cWrite_PO3100K(0x41,0x47);
    i2cWrite_PO3100K(0x42,0x52);
    i2cWrite_PO3100K(0x43,0x5A);
    i2cWrite_PO3100K(0x44,0x6C);
    i2cWrite_PO3100K(0x45,0x7A);
    i2cWrite_PO3100K(0x46,0x95);
    i2cWrite_PO3100K(0x47,0xAE);
    i2cWrite_PO3100K(0x48,0xC5);
    i2cWrite_PO3100K(0x49,0xDA);
    i2cWrite_PO3100K(0x4A,0xED);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x4C,0x00);
    i2cWrite_PO3100K(0x4D,0x05);
    i2cWrite_PO3100K(0x4E,0x14);
    i2cWrite_PO3100K(0x4F,0x25);
    i2cWrite_PO3100K(0x50,0x34);
    i2cWrite_PO3100K(0x51,0x4B);
    i2cWrite_PO3100K(0x52,0x5B);
    i2cWrite_PO3100K(0x53,0x73);
    i2cWrite_PO3100K(0x54,0x86);
    i2cWrite_PO3100K(0x55,0xA3);
    i2cWrite_PO3100K(0x56,0xBA);
    i2cWrite_PO3100K(0x57,0xCE);
    i2cWrite_PO3100K(0x58,0xE0);
    i2cWrite_PO3100K(0x59,0xF0);
    i2cWrite_PO3100K(0x5A,0xFF);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x27);
    i2cWrite_PO3100K(0x5D,0x36);
    i2cWrite_PO3100K(0x5E,0x40);
    i2cWrite_PO3100K(0x5F,0x49);
    i2cWrite_PO3100K(0x60,0x58);
    i2cWrite_PO3100K(0x61,0x64);
    i2cWrite_PO3100K(0x62,0x78);
    i2cWrite_PO3100K(0x63,0x89);
    i2cWrite_PO3100K(0x64,0xA4);
    i2cWrite_PO3100K(0x65,0xBB);
    i2cWrite_PO3100K(0x66,0xCF);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF1);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x2B);
    i2cWrite_PO3100K(0x6C,0x3A);
    i2cWrite_PO3100K(0x6D,0x45);
    i2cWrite_PO3100K(0x6E,0x4E);
    i2cWrite_PO3100K(0x6F,0x5C);
    i2cWrite_PO3100K(0x70,0x68);
    i2cWrite_PO3100K(0x71,0x7C);
    i2cWrite_PO3100K(0x72,0x8D);
    i2cWrite_PO3100K(0x73,0xA8);
    i2cWrite_PO3100K(0x74,0xBE);
    i2cWrite_PO3100K(0x75,0xD1);
    i2cWrite_PO3100K(0x76,0xE2);
    i2cWrite_PO3100K(0x77,0xF1);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x01);
    i2cWrite_PO3100K(0x0B,0x80);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x07);
    i2cWrite_PO3100K(0x78,0x07);
    i2cWrite_PO3100K(0x79,0x07);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x1C);
    i2cWrite_PO3100K(0xA2,0x14);
    i2cWrite_PO3100K(0xA3,0x14);
    i2cWrite_PO3100K(0xA5,0x00);
    i2cWrite_PO3100K(0xA6,0x1A);
    i2cWrite_PO3100K(0xA7,0x34);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x3A);
    i2cWrite_PO3100K(0x1D,0x26);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x3A);
    i2cWrite_PO3100K(0x20,0x26);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x90);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x24);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x10);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x64);
    i2cWrite_PO3100K(0x28,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x4E,0x00);
    i2cWrite_PO3100K(0x4F,0x20);
    i2cWrite_PO3100K(0x50,0x20);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x70);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x68);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x10);
    i2cWrite_PO3100K(0x43,0x1A);
    i2cWrite_PO3100K(0x44,0x25);
    i2cWrite_PO3100K(0x2E,0x03);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x03);
    i2cWrite_PO3100K(0x83,0x20);
    i2cWrite_PO3100K(0x84,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x91,0x40);
    i2cWrite_PO3100K(0x92,0x40);
    i2cWrite_PO3100K(0x93,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x95,0xF0);
    i2cWrite_PO3100K(0x96,0xF8);
    i2cWrite_PO3100K(0x97,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x36,0x1F);
    i2cWrite_PO3100K(0x37,0x1F);
    i2cWrite_PO3100K(0x38,0x1F);
    i2cWrite_PO3100K(0x3A,0xFF);
    i2cWrite_PO3100K(0x3B,0xFF);
    i2cWrite_PO3100K(0x3C,0xFF);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x0A);

    i2cWrite_PO3100K(0x03,0x05);
    i2cWrite_PO3100K(0x04,0x00);

#endif
    SetPO3100K_720P_30FPS_RDI_Init();

    siuSetSensorDayNight(DayNightMode);
}
#elif ( (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_SKY_AHD) || (HW_BOARD_OPTION == MR9100_TX_SKY_AHD) ||\
    (HW_BOARD_OPTION == MR9100_TX_SKY_W_AHD) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB))

void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
	{   //夜間模式
        DEBUG_SIU("##enter night\n");

#if 1// 20150514
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01); //AE center window Y start position 
        i2cWrite_PO3100K(0xC0, 0x27);
        i2cWrite_PO3100K(0xC1, 0x02); //AE center window Y stop position
        i2cWrite_PO3100K(0xC2, 0x48);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x4E); //AE reference
        i2cWrite_PO3100K(0x1D, 0x00);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x4E);
        i2cWrite_PO3100K(0x20, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);                             
        i2cWrite_PO3100K(0x56, 0xFF);
        i2cWrite_PO3100K(0x57, 0xFF);
        i2cWrite_PO3100K(0x58, 0xFF);     
        i2cWrite_PO3100K(0x03, 0x04); 
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x68);
        i2cWrite_PO3100K(0x3C, 0x60);
        i2cWrite_PO3100K(0x3D, 0x60);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);                                                          
        i2cWrite_PO3100K(0x03, 0x03);              
        i2cWrite_PO3100K(0xA1, 0x30);
        i2cWrite_PO3100K(0xA2, 0x30);
        i2cWrite_PO3100K(0xA3, 0x30);
        i2cWrite_PO3100K(0x03, 0x03);          
        i2cWrite_PO3100K(0xA5, 0x00);
        i2cWrite_PO3100K(0xA6, 0x00);
        i2cWrite_PO3100K(0xA7, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x00);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x10);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x02);
        i2cWrite_PO3100K(0x31, 0x02);
        i2cWrite_PO3100K(0x32, 0x02);
        i2cWrite_PO3100K(0x33, 0x02);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x03);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x04);
        i2cWrite_PO3100K(0x3F, 0x08);
        i2cWrite_PO3100K(0x40, 0x18);
#endif
	}
    else if(Level == SIU_DAY_MODE)
    {   //日間模式
        DEBUG_SIU("##enter day\n");



        // 20150514
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01);
        i2cWrite_PO3100K(0xC0, 0x0C);
        i2cWrite_PO3100K(0xC1, 0x02);
        i2cWrite_PO3100K(0xC2, 0x32);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x17);
        i2cWrite_PO3100K(0x1D, 0x40);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x17);
        i2cWrite_PO3100K(0x20, 0x40);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x56, 0x10);
        i2cWrite_PO3100K(0x57, 0x04);
        i2cWrite_PO3100K(0x58, 0x08);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x90);
        i2cWrite_PO3100K(0x3C, 0x70);
        i2cWrite_PO3100K(0x3D, 0x70);
        i2cWrite_PO3100K(0x3E, 0x68);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA1, 0x25); // 20150518
        i2cWrite_PO3100K(0xA2, 0x14);
        i2cWrite_PO3100K(0xA3, 0x14);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA5, 0x05);
        i2cWrite_PO3100K(0xA6, 0x1A);
        i2cWrite_PO3100K(0xA7, 0x34);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x08);
        i2cWrite_PO3100K(0x31, 0x08);
        i2cWrite_PO3100K(0x32, 0x08);
        i2cWrite_PO3100K(0x33, 0x08);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x02);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x01);
        i2cWrite_PO3100K(0x3F, 0x04);
        i2cWrite_PO3100K(0x40, 0x18);

        //20151125
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x58);
        
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x20); // 20160328
        i2cWrite_PO3100K(0x30, 0x80);
        i2cWrite_PO3100K(0x31, 0x80);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x09, 0x80);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x5D, 0x06);
        
        
    }
}

void SetPO3100K_720P_30FPS(void)
{
    int i;

    DEBUG_SIU("SetPO3100K_720P_30FPS() SKY\n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

#if 0   // 20140918
#else // 20150514
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    #if SENSOR_ROW_COL_MIRROR
    i2cWrite_PO3100K(0x05,0x03);
    #else
    i2cWrite_PO3100K(0x05,0x00);
    #endif
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x81);
    i2cWrite_PO3100K(0xA5,0x81);
    i2cWrite_PO3100K(0xA6,0x81);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x36);
    i2cWrite_PO3100K(0x34,0x89);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8C);
    i2cWrite_PO3100K(0x37,0x3A);
    i2cWrite_PO3100K(0x38,0x8D);
    i2cWrite_PO3100K(0x39,0x83);
    i2cWrite_PO3100K(0x3A,0x99);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x04,0x25);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x06,0x00);
    i2cWrite_PO3100K(0x07,0x1C);
    i2cWrite_PO3100K(0x08,0x1E);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x20);
    i2cWrite_PO3100K(0x0C,0x28);
    i2cWrite_PO3100K(0x0D,0x00);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x0B);
    i2cWrite_PO3100K(0x3E,0x2B);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x3F);
    i2cWrite_PO3100K(0x41,0x47);
    i2cWrite_PO3100K(0x42,0x52);
    i2cWrite_PO3100K(0x43,0x5A);
    i2cWrite_PO3100K(0x44,0x6C);
    i2cWrite_PO3100K(0x45,0x7A);
    i2cWrite_PO3100K(0x46,0x95);
    i2cWrite_PO3100K(0x47,0xAE);
    i2cWrite_PO3100K(0x48,0xC5);
    i2cWrite_PO3100K(0x49,0xDA);
    i2cWrite_PO3100K(0x4A,0xED);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x4C,0x00);
    i2cWrite_PO3100K(0x4D,0x05);
    i2cWrite_PO3100K(0x4E,0x14);
    i2cWrite_PO3100K(0x4F,0x25);
    i2cWrite_PO3100K(0x50,0x34);
    i2cWrite_PO3100K(0x51,0x4B);
    i2cWrite_PO3100K(0x52,0x5B);
    i2cWrite_PO3100K(0x53,0x73);
    i2cWrite_PO3100K(0x54,0x86);
    i2cWrite_PO3100K(0x55,0xA3);
    i2cWrite_PO3100K(0x56,0xBA);
    i2cWrite_PO3100K(0x57,0xCE);
    i2cWrite_PO3100K(0x58,0xE0);
    i2cWrite_PO3100K(0x59,0xF0);
    i2cWrite_PO3100K(0x5A,0xFF);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x27);
    i2cWrite_PO3100K(0x5D,0x36);
    i2cWrite_PO3100K(0x5E,0x40);
    i2cWrite_PO3100K(0x5F,0x49);
    i2cWrite_PO3100K(0x60,0x58);
    i2cWrite_PO3100K(0x61,0x64);
    i2cWrite_PO3100K(0x62,0x78);
    i2cWrite_PO3100K(0x63,0x89);
    i2cWrite_PO3100K(0x64,0xA4);
    i2cWrite_PO3100K(0x65,0xBB);
    i2cWrite_PO3100K(0x66,0xCF);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF1);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x2B);
    i2cWrite_PO3100K(0x6C,0x3A);
    i2cWrite_PO3100K(0x6D,0x45);
    i2cWrite_PO3100K(0x6E,0x4E);
    i2cWrite_PO3100K(0x6F,0x5C);
    i2cWrite_PO3100K(0x70,0x68);
    i2cWrite_PO3100K(0x71,0x7C);
    i2cWrite_PO3100K(0x72,0x8D);
    i2cWrite_PO3100K(0x73,0xA8);
    i2cWrite_PO3100K(0x74,0xBE);
    i2cWrite_PO3100K(0x75,0xD1);
    i2cWrite_PO3100K(0x76,0xE2);
    i2cWrite_PO3100K(0x77,0xF1);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x01);
    i2cWrite_PO3100K(0x0B,0x80);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x07);
    i2cWrite_PO3100K(0x78,0x07);
    i2cWrite_PO3100K(0x79,0x07);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x1C);
    i2cWrite_PO3100K(0xA2,0x14);
    i2cWrite_PO3100K(0xA3,0x14);
    i2cWrite_PO3100K(0xA5,0x00);
    i2cWrite_PO3100K(0xA6,0x1A);
    i2cWrite_PO3100K(0xA7,0x34);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x3A);
    i2cWrite_PO3100K(0x1D,0x26);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x3A);
    i2cWrite_PO3100K(0x20,0x26);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x90);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x24);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x10);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x64);
    i2cWrite_PO3100K(0x28,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x4E,0x00);
    i2cWrite_PO3100K(0x4F,0x20);
    i2cWrite_PO3100K(0x50,0x20);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x70);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x68);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x10);
    i2cWrite_PO3100K(0x43,0x1A);
    i2cWrite_PO3100K(0x44,0x25);
    i2cWrite_PO3100K(0x2E,0x03);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x03);
    i2cWrite_PO3100K(0x83,0x20);
    i2cWrite_PO3100K(0x84,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x91,0x40);
    i2cWrite_PO3100K(0x92,0x40);
    i2cWrite_PO3100K(0x93,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x95,0xF0);
    i2cWrite_PO3100K(0x96,0xF8);
    i2cWrite_PO3100K(0x97,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x36,0x1F);
    i2cWrite_PO3100K(0x37,0x1F);
    i2cWrite_PO3100K(0x38,0x1F);
    i2cWrite_PO3100K(0x3A,0xFF);
    i2cWrite_PO3100K(0x3B,0xFF);
    i2cWrite_PO3100K(0x3C,0xFF);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x0A);

    i2cWrite_PO3100K(0x03,0x05);
    i2cWrite_PO3100K(0x04,0x00);

#endif
    siuSetSensorDayNight(DayNightMode);
}
#else

void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
	{   //夜間模式
        DEBUG_SIU("##enter night\n");

#if 0   // 150428
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01);
        i2cWrite_PO3100K(0xC0, 0x27);
        i2cWrite_PO3100K(0xC1, 0x02);
        i2cWrite_PO3100K(0xC2, 0x48);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x4E);
        i2cWrite_PO3100K(0x1D, 0x00);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x4E);
        i2cWrite_PO3100K(0x20, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x56, 0xFF);
        i2cWrite_PO3100K(0x57, 0xFF);
        i2cWrite_PO3100K(0x58, 0xFF);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x68);
        i2cWrite_PO3100K(0x3C, 0x60);
        i2cWrite_PO3100K(0x3D, 0x60);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA1, 0x30);
        i2cWrite_PO3100K(0xA2, 0x30);
        i2cWrite_PO3100K(0xA3, 0x30);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA5, 0x00);
        i2cWrite_PO3100K(0xA6, 0x00);
        i2cWrite_PO3100K(0xA7, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x00);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x10);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x02);
        i2cWrite_PO3100K(0x31, 0x02);
        i2cWrite_PO3100K(0x32, 0x02);
        i2cWrite_PO3100K(0x33, 0x02);
        i2cWrite_PO3100K(0x34, 0x0A);
#else   // 20150514
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01); //AE center window Y start position 
        i2cWrite_PO3100K(0xC0, 0x27);
        i2cWrite_PO3100K(0xC1, 0x02); //AE center window Y stop position
        i2cWrite_PO3100K(0xC2, 0x48);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x4E); //AE reference
        i2cWrite_PO3100K(0x1D, 0x00);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x4E);
        i2cWrite_PO3100K(0x20, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);                             
        i2cWrite_PO3100K(0x56, 0xFF);
        i2cWrite_PO3100K(0x57, 0xFF);
        i2cWrite_PO3100K(0x58, 0xFF);     
        i2cWrite_PO3100K(0x03, 0x04); 
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x68);
        i2cWrite_PO3100K(0x3C, 0x60);
        i2cWrite_PO3100K(0x3D, 0x60);
        i2cWrite_PO3100K(0x3E, 0x60);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);                                                          
        i2cWrite_PO3100K(0x03, 0x03);              
        i2cWrite_PO3100K(0xA1, 0x30);
        i2cWrite_PO3100K(0xA2, 0x30);
        i2cWrite_PO3100K(0xA3, 0x30);
        i2cWrite_PO3100K(0x03, 0x03);          
        i2cWrite_PO3100K(0xA5, 0x00);
        i2cWrite_PO3100K(0xA6, 0x00);
        i2cWrite_PO3100K(0xA7, 0x00);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x00);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x10);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x02);
        i2cWrite_PO3100K(0x31, 0x02);
        i2cWrite_PO3100K(0x32, 0x02);
        i2cWrite_PO3100K(0x33, 0x02);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x03);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x04);
        i2cWrite_PO3100K(0x3F, 0x08);
        i2cWrite_PO3100K(0x40, 0x18);
        //i2cWrite_PO3100K(0x03, 0x03);
        //i2cWrite_PO3100K(0xA1, 0x1C);
        //i2cWrite_PO3100K(0xA2, 0x14);
        //i2cWrite_PO3100K(0xA3, 0x14);
#endif
	}
    else if(Level == SIU_DAY_MODE)
    {   //日間模式
        DEBUG_SIU("##enter day\n");



        // 20150514
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0xBF, 0x01);
        i2cWrite_PO3100K(0xC0, 0x0C);
        i2cWrite_PO3100K(0xC1, 0x02);
        i2cWrite_PO3100K(0xC2, 0x32);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x1C, 0x17);
        i2cWrite_PO3100K(0x1D, 0x40);
        i2cWrite_PO3100K(0x1E, 0x00);
        i2cWrite_PO3100K(0x1F, 0x17);
        i2cWrite_PO3100K(0x20, 0x40);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x56, 0x10);
        i2cWrite_PO3100K(0x57, 0x04);
        i2cWrite_PO3100K(0x58, 0x08);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x05, 0x64);
        i2cWrite_PO3100K(0x3B, 0x90);
        i2cWrite_PO3100K(0x3C, 0x70);
        i2cWrite_PO3100K(0x3D, 0x70);
        i2cWrite_PO3100K(0x3E, 0x68);
        i2cWrite_PO3100K(0x3F, 0x24);
        i2cWrite_PO3100K(0x40, 0x4B);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA1, 0x25); // 20150518
        i2cWrite_PO3100K(0xA2, 0x14);
        i2cWrite_PO3100K(0xA3, 0x14);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0xA5, 0x05);
        i2cWrite_PO3100K(0xA6, 0x1A);
        i2cWrite_PO3100K(0xA7, 0x34);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x19, 0x24);
        i2cWrite_PO3100K(0x03, 0x02);
        i2cWrite_PO3100K(0x2F, 0x24);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x30, 0x08);
        i2cWrite_PO3100K(0x31, 0x08);
        i2cWrite_PO3100K(0x32, 0x08);
        i2cWrite_PO3100K(0x33, 0x08);
        i2cWrite_PO3100K(0x34, 0x0A);
        i2cWrite_PO3100K(0x03, 0x04);
        i2cWrite_PO3100K(0x82, 0x02);
        i2cWrite_PO3100K(0x03, 0x03);
        i2cWrite_PO3100K(0x3E, 0x01);
        i2cWrite_PO3100K(0x3F, 0x04);
        i2cWrite_PO3100K(0x40, 0x18);
        
    }
}

void SetPO3100K_720P_30FPS(void)
{
    int i;

    DEBUG_SIU("SetPO3100K_720P_30FPS()\n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

#if 0   // 20140918
#else // 20150514
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    #if SENSOR_ROW_COL_MIRROR
    i2cWrite_PO3100K(0x05,0x03);
    #else
    i2cWrite_PO3100K(0x05,0x00);
    #endif
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x81);
    i2cWrite_PO3100K(0xA5,0x81);
    i2cWrite_PO3100K(0xA6,0x81);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x36);
    i2cWrite_PO3100K(0x34,0x89);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8C);
    i2cWrite_PO3100K(0x37,0x3A);
    i2cWrite_PO3100K(0x38,0x8D);
    i2cWrite_PO3100K(0x39,0x83);
    i2cWrite_PO3100K(0x3A,0x99);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x04,0x25);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x06,0x00);
    i2cWrite_PO3100K(0x07,0x1C);
    i2cWrite_PO3100K(0x08,0x1E);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x20);
    i2cWrite_PO3100K(0x0C,0x28);
    i2cWrite_PO3100K(0x0D,0x00);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x0B);
    i2cWrite_PO3100K(0x3E,0x2B);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x3F);
    i2cWrite_PO3100K(0x41,0x47);
    i2cWrite_PO3100K(0x42,0x52);
    i2cWrite_PO3100K(0x43,0x5A);
    i2cWrite_PO3100K(0x44,0x6C);
    i2cWrite_PO3100K(0x45,0x7A);
    i2cWrite_PO3100K(0x46,0x95);
    i2cWrite_PO3100K(0x47,0xAE);
    i2cWrite_PO3100K(0x48,0xC5);
    i2cWrite_PO3100K(0x49,0xDA);
    i2cWrite_PO3100K(0x4A,0xED);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x4C,0x00);
    i2cWrite_PO3100K(0x4D,0x05);
    i2cWrite_PO3100K(0x4E,0x14);
    i2cWrite_PO3100K(0x4F,0x25);
    i2cWrite_PO3100K(0x50,0x34);
    i2cWrite_PO3100K(0x51,0x4B);
    i2cWrite_PO3100K(0x52,0x5B);
    i2cWrite_PO3100K(0x53,0x73);
    i2cWrite_PO3100K(0x54,0x86);
    i2cWrite_PO3100K(0x55,0xA3);
    i2cWrite_PO3100K(0x56,0xBA);
    i2cWrite_PO3100K(0x57,0xCE);
    i2cWrite_PO3100K(0x58,0xE0);
    i2cWrite_PO3100K(0x59,0xF0);
    i2cWrite_PO3100K(0x5A,0xFF);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x27);
    i2cWrite_PO3100K(0x5D,0x36);
    i2cWrite_PO3100K(0x5E,0x40);
    i2cWrite_PO3100K(0x5F,0x49);
    i2cWrite_PO3100K(0x60,0x58);
    i2cWrite_PO3100K(0x61,0x64);
    i2cWrite_PO3100K(0x62,0x78);
    i2cWrite_PO3100K(0x63,0x89);
    i2cWrite_PO3100K(0x64,0xA4);
    i2cWrite_PO3100K(0x65,0xBB);
    i2cWrite_PO3100K(0x66,0xCF);
    i2cWrite_PO3100K(0x67,0xE0);
    i2cWrite_PO3100K(0x68,0xF1);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x2B);
    i2cWrite_PO3100K(0x6C,0x3A);
    i2cWrite_PO3100K(0x6D,0x45);
    i2cWrite_PO3100K(0x6E,0x4E);
    i2cWrite_PO3100K(0x6F,0x5C);
    i2cWrite_PO3100K(0x70,0x68);
    i2cWrite_PO3100K(0x71,0x7C);
    i2cWrite_PO3100K(0x72,0x8D);
    i2cWrite_PO3100K(0x73,0xA8);
    i2cWrite_PO3100K(0x74,0xBE);
    i2cWrite_PO3100K(0x75,0xD1);
    i2cWrite_PO3100K(0x76,0xE2);
    i2cWrite_PO3100K(0x77,0xF1);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x01);
    i2cWrite_PO3100K(0x0B,0x80);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x07);
    i2cWrite_PO3100K(0x78,0x07);
    i2cWrite_PO3100K(0x79,0x07);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x1C);
    i2cWrite_PO3100K(0xA2,0x14);
    i2cWrite_PO3100K(0xA3,0x14);
    i2cWrite_PO3100K(0xA5,0x00);
    i2cWrite_PO3100K(0xA6,0x1A);
    i2cWrite_PO3100K(0xA7,0x34);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xE8);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0xE8);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE8);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x3A);
    i2cWrite_PO3100K(0x1D,0x26);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x3A);
    i2cWrite_PO3100K(0x20,0x26);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x90);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x24);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x10);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x64);
    i2cWrite_PO3100K(0x28,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x4E,0x00);
    i2cWrite_PO3100K(0x4F,0x20);
    i2cWrite_PO3100K(0x50,0x20);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x70);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x68);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x10);
    i2cWrite_PO3100K(0x43,0x1A);
    i2cWrite_PO3100K(0x44,0x25);
    i2cWrite_PO3100K(0x2E,0x03);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x03);
    i2cWrite_PO3100K(0x83,0x20);
    i2cWrite_PO3100K(0x84,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x91,0x40);
    i2cWrite_PO3100K(0x92,0x40);
    i2cWrite_PO3100K(0x93,0x40);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x95,0xF0);
    i2cWrite_PO3100K(0x96,0xF8);
    i2cWrite_PO3100K(0x97,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x36,0x1F);
    i2cWrite_PO3100K(0x37,0x1F);
    i2cWrite_PO3100K(0x38,0x1F);
    i2cWrite_PO3100K(0x3A,0xFF);
    i2cWrite_PO3100K(0x3B,0xFF);
    i2cWrite_PO3100K(0x3C,0xFF);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x0A);

    i2cWrite_PO3100K(0x03,0x05);
    i2cWrite_PO3100K(0x04,0x00);

#endif
    siuSetSensorDayNight(DayNightMode);
}

#endif
void SetPO3100K_VGA_30FPS(void)
{
    int i;

    DEBUG_SIU("SetPO3100K_VGA_30FPS()\n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2D,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x01);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x29,0x98);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x04,0x02);
    i2cWrite_PO3100K(0x05,0x00);
    i2cWrite_PO3100K(0x41,0x02);
    i2cWrite_PO3100K(0x42,0x0B);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x3C);
    i2cWrite_PO3100K(0x06,0x06);
    i2cWrite_PO3100K(0x07,0x71);
    i2cWrite_PO3100K(0x08,0x02);
    i2cWrite_PO3100K(0x09,0xED);
    i2cWrite_PO3100K(0x0A,0x02);
    i2cWrite_PO3100K(0x0B,0xED);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x05);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x05);
    i2cWrite_PO3100K(0x10,0x05);
    i2cWrite_PO3100K(0x11,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0xD4);
    i2cWrite_PO3100K(0x14,0x00);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x29,0x00);
    i2cWrite_PO3100K(0x88,0x7F);
    i2cWrite_PO3100K(0x8E,0xFE);
    i2cWrite_PO3100K(0x9A,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x0C,0x00);
    i2cWrite_PO3100K(0x0D,0x03);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x03);
    i2cWrite_PO3100K(0x10,0x02);
    i2cWrite_PO3100K(0x11,0x82);
    i2cWrite_PO3100K(0x12,0x01);
    i2cWrite_PO3100K(0x13,0xE2);
    i2cWrite_PO3100K(0x15,0x15);
    i2cWrite_PO3100K(0x17,0xE9);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x7B,0x40);
    i2cWrite_PO3100K(0x7C,0x30);
    i2cWrite_PO3100K(0x7D,0x02);
    i2cWrite_PO3100K(0x7E,0x8D);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0xB3,0x00);
    i2cWrite_PO3100K(0xB4,0x02);
    i2cWrite_PO3100K(0xB5,0x02);
    i2cWrite_PO3100K(0xB6,0x82);
    i2cWrite_PO3100K(0xB7,0x00);
    i2cWrite_PO3100K(0xB8,0x03);
    i2cWrite_PO3100K(0xB9,0x01);
    i2cWrite_PO3100K(0xBA,0xE2);
    i2cWrite_PO3100K(0xBB,0x00);
    i2cWrite_PO3100K(0xBC,0xD7);
    i2cWrite_PO3100K(0xBD,0x01);
    i2cWrite_PO3100K(0xBE,0xAD);
    i2cWrite_PO3100K(0xBF,0x00);
    i2cWrite_PO3100K(0xC0,0xA3);
    i2cWrite_PO3100K(0xC1,0x01);
    i2cWrite_PO3100K(0xC2,0x42);
    i2cWrite_PO3100K(0xC3,0x01);
    i2cWrite_PO3100K(0xC4,0x42);
    i2cWrite_PO3100K(0xC5,0x00);
    i2cWrite_PO3100K(0xC6,0xF3);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0xC7,0x00);
    i2cWrite_PO3100K(0xC8,0x02);
    i2cWrite_PO3100K(0xC9,0x02);
    i2cWrite_PO3100K(0xCA,0x82);
    i2cWrite_PO3100K(0xCB,0x00);
    i2cWrite_PO3100K(0xCC,0x03);
    i2cWrite_PO3100K(0xCD,0x01);
    i2cWrite_PO3100K(0xCE,0xE2);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0xD8,0x00);
    i2cWrite_PO3100K(0xD9,0xD7);
    i2cWrite_PO3100K(0xDA,0x01);
    i2cWrite_PO3100K(0xDB,0xAD);
    i2cWrite_PO3100K(0xDC,0x00);
    i2cWrite_PO3100K(0xDD,0xA3);
    i2cWrite_PO3100K(0xDE,0x01);
    i2cWrite_PO3100K(0xDF,0x42);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x33,0x05);
    i2cWrite_PO3100K(0x34,0x02);
    i2cWrite_PO3100K(0x36,0x80);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x38,0x58);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x1E,0x0E);
    i2cWrite_PO3100K(0x26,0x03);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xB1,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0x98);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0xA4,0x88);
    i2cWrite_PO3100K(0xA5,0x88);
    i2cWrite_PO3100K(0xA6,0x88);
    i2cWrite_PO3100K(0xA7,0x00);
    i2cWrite_PO3100K(0xA8,0x00);
    i2cWrite_PO3100K(0xA9,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x06,0xB8);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x75,0x28);
    i2cWrite_PO3100K(0x76,0x28);
    i2cWrite_PO3100K(0x77,0x78);
    i2cWrite_PO3100K(0x78,0x78);
    i2cWrite_PO3100K(0x79,0x48);
    i2cWrite_PO3100K(0x7A,0x48);
    i2cWrite_PO3100K(0x7B,0xB8);
    i2cWrite_PO3100K(0x7C,0xB8);
    i2cWrite_PO3100K(0x7D,0x01);
    i2cWrite_PO3100K(0x7E,0x00);
    i2cWrite_PO3100K(0x7F,0x02);
    i2cWrite_PO3100K(0x80,0x07);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x73,0x08);
    i2cWrite_PO3100K(0x74,0x04);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x64);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x6E,0x3A);
    i2cWrite_PO3100K(0x6F,0x50);
    i2cWrite_PO3100K(0x70,0x60);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x16,0x3A);
    i2cWrite_PO3100K(0x17,0x50);
    i2cWrite_PO3100K(0x18,0x60);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x3B,0x90);
    i2cWrite_PO3100K(0x3C,0x78);
    i2cWrite_PO3100K(0x3D,0x70);
    i2cWrite_PO3100K(0x3E,0x78);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x2C,0xBB);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x41,0x04);
    i2cWrite_PO3100K(0x42,0x08);
    i2cWrite_PO3100K(0x43,0x10);
    i2cWrite_PO3100K(0x44,0x20);
    i2cWrite_PO3100K(0x2E,0x04);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x4F,0x08);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x59,0x00);
    i2cWrite_PO3100K(0x5A,0xBA);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x37);
    i2cWrite_PO3100K(0x34,0x90);
    i2cWrite_PO3100K(0x35,0x87);
    i2cWrite_PO3100K(0x36,0x8E);
    i2cWrite_PO3100K(0x37,0x3B);
    i2cWrite_PO3100K(0x38,0x8C);
    i2cWrite_PO3100K(0x39,0x82);
    i2cWrite_PO3100K(0x3A,0x98);
    i2cWrite_PO3100K(0x3B,0x3A);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x0C,0x25);
    i2cWrite_PO3100K(0x0D,0x88);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x00);
    i2cWrite_PO3100K(0x3E,0x27);
    i2cWrite_PO3100K(0x3F,0x36);
    i2cWrite_PO3100K(0x40,0x40);
    i2cWrite_PO3100K(0x41,0x49);
    i2cWrite_PO3100K(0x42,0x58);
    i2cWrite_PO3100K(0x43,0x64);
    i2cWrite_PO3100K(0x44,0x78);
    i2cWrite_PO3100K(0x45,0x89);
    i2cWrite_PO3100K(0x46,0xA4);
    i2cWrite_PO3100K(0x47,0xBB);
    i2cWrite_PO3100K(0x48,0xCF);
    i2cWrite_PO3100K(0x49,0xE0);
    i2cWrite_PO3100K(0x4A,0xF1);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x5B,0x00);
    i2cWrite_PO3100K(0x5C,0x0B);
    i2cWrite_PO3100K(0x5D,0x13);
    i2cWrite_PO3100K(0x5E,0x1A);
    i2cWrite_PO3100K(0x5F,0x20);
    i2cWrite_PO3100K(0x60,0x2B);
    i2cWrite_PO3100K(0x61,0x36);
    i2cWrite_PO3100K(0x62,0x49);
    i2cWrite_PO3100K(0x63,0x5A);
    i2cWrite_PO3100K(0x64,0x7B);
    i2cWrite_PO3100K(0x65,0x98);
    i2cWrite_PO3100K(0x66,0xB4);
    i2cWrite_PO3100K(0x67,0xCE);
    i2cWrite_PO3100K(0x68,0xE7);
    i2cWrite_PO3100K(0x69,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x6A,0x00);
    i2cWrite_PO3100K(0x6B,0x0B);
    i2cWrite_PO3100K(0x6C,0x13);
    i2cWrite_PO3100K(0x6D,0x1A);
    i2cWrite_PO3100K(0x6E,0x20);
    i2cWrite_PO3100K(0x6F,0x2B);
    i2cWrite_PO3100K(0x70,0x36);
    i2cWrite_PO3100K(0x71,0x49);
    i2cWrite_PO3100K(0x72,0x5A);
    i2cWrite_PO3100K(0x73,0x7B);
    i2cWrite_PO3100K(0x74,0x98);
    i2cWrite_PO3100K(0x75,0xB4);
    i2cWrite_PO3100K(0x76,0xCE);
    i2cWrite_PO3100K(0x77,0xE7);
    i2cWrite_PO3100K(0x78,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x30);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x09,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x26,0x00);
    i2cWrite_PO3100K(0x27,0x10);
    i2cWrite_PO3100K(0x28,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x2E,0x7F);
    i2cWrite_PO3100K(0x2F,0x7F);
    i2cWrite_PO3100K(0x30,0x7F);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x32,0x00);
    i2cWrite_PO3100K(0x33,0x00);
    i2cWrite_PO3100K(0x34,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x77,0x00);
    i2cWrite_PO3100K(0x78,0x00);
    i2cWrite_PO3100K(0x79,0x00);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x30);
    i2cWrite_PO3100K(0xA2,0x7F);
    i2cWrite_PO3100K(0xA3,0x7F);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x05);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x12,0x02);
    i2cWrite_PO3100K(0x13,0x70);
    i2cWrite_PO3100K(0x14,0x02);
    i2cWrite_PO3100K(0x15,0x70);
    i2cWrite_PO3100K(0x16,0x02);
    i2cWrite_PO3100K(0x17,0x70);
    i2cWrite_PO3100K(0x1B,0x00);
    i2cWrite_PO3100K(0x1C,0x17);
    i2cWrite_PO3100K(0x1D,0x40);
    i2cWrite_PO3100K(0x1E,0x00);
    i2cWrite_PO3100K(0x1F,0x17);
    i2cWrite_PO3100K(0x20,0x40);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x82,0x0C);
    i2cWrite_PO3100K(0x83,0x13);
    i2cWrite_PO3100K(0x84,0x15);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x2A,0x80);
    i2cWrite_PO3100K(0x2C,0x80);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x07,0xA1);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8E,0x00);
    i2cWrite_PO3100K(0x8F,0x01);
    i2cWrite_PO3100K(0x90,0x50);
    i2cWrite_PO3100K(0x91,0xB8);
    i2cWrite_PO3100K(0x92,0x10);
    i2cWrite_PO3100K(0x93,0xFF);
    i2cWrite_PO3100K(0x03,0x01);
    i2cWrite_PO3100K(0x16,0x04);
    i2cWrite_PO3100K(0x17,0xFA);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x99,0x00);
    i2cWrite_PO3100K(0x9A,0x00);
    i2cWrite_PO3100K(0x9B,0x00);
    i2cWrite_PO3100K(0x9C,0x00);
    i2cWrite_PO3100K(0x9D,0x00);
    i2cWrite_PO3100K(0x9E,0x25);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7F,0x00);
    i2cWrite_PO3100K(0x80,0x04);
    i2cWrite_PO3100K(0x81,0x0E);
    i2cWrite_PO3100K(0x82,0x00);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x8A,0x02);
    i2cWrite_PO3100K(0x8B,0x47);
    i2cWrite_PO3100K(0x8C,0xE0);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x7C,0x04);
    i2cWrite_PO3100K(0x7D,0x77);
    i2cWrite_PO3100K(0x97,0x04);
    i2cWrite_PO3100K(0x98,0x77);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x51,0x10);
    i2cWrite_PO3100K(0x52,0xE0);
    i2cWrite_PO3100K(0x53,0x02);
    i2cWrite_PO3100K(0x54,0x02);
    i2cWrite_PO3100K(0x55,0x40);
    i2cWrite_PO3100K(0x56,0xC0);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x6E);
    i2cWrite_PO3100K(0x59,0x45);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x5A,0x23);
    i2cWrite_PO3100K(0x5B,0x4B);
    i2cWrite_PO3100K(0x5C,0x82);
    i2cWrite_PO3100K(0x5D,0xAA);
    i2cWrite_PO3100K(0x5E,0x23);
    i2cWrite_PO3100K(0x5F,0x28);
    i2cWrite_PO3100K(0x60,0x4B);
    i2cWrite_PO3100K(0x61,0x73);
    i2cWrite_PO3100K(0x62,0x3C);
    i2cWrite_PO3100K(0x63,0x87);
    i2cWrite_PO3100K(0x64,0x2D);
    i2cWrite_PO3100K(0x65,0x2D);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x48,0x08);
    i2cWrite_PO3100K(0x49,0x08);
    i2cWrite_PO3100K(0x4A,0x0A);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0xA1,0x10);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x3D,0x00);
    i2cWrite_PO3100K(0x3E,0x06);
    i2cWrite_PO3100K(0x3F,0x18);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x41,0x3C);
    i2cWrite_PO3100K(0x42,0x54);
    i2cWrite_PO3100K(0x43,0x65);
    i2cWrite_PO3100K(0x44,0x7D);
    i2cWrite_PO3100K(0x45,0x8F);
    i2cWrite_PO3100K(0x46,0xAB);
    i2cWrite_PO3100K(0x47,0xC1);
    i2cWrite_PO3100K(0x48,0xD3);
    i2cWrite_PO3100K(0x49,0xE3);
    i2cWrite_PO3100K(0x4A,0xF2);
    i2cWrite_PO3100K(0x4B,0xFF);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8D,0x50);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x0A,0x95);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x0A,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x9B,0x45);
    i2cWrite_PO3100K(0x9C,0x45);
    i2cWrite_PO3100K(0x9D,0x45);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x97,0x90);
    i2cWrite_PO3100K(0x98,0x90);
    i2cWrite_PO3100K(0x99,0x90);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x33,0x37);
    i2cWrite_PO3100K(0x34,0x8A);
    i2cWrite_PO3100K(0x35,0x8D);
    i2cWrite_PO3100K(0x36,0x8B);
    i2cWrite_PO3100K(0x37,0x3E);
    i2cWrite_PO3100K(0x38,0x90);
    i2cWrite_PO3100K(0x39,0x84);
    i2cWrite_PO3100K(0x3A,0x98);
    i2cWrite_PO3100K(0x3B,0x3C);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x08,0x25);
    i2cWrite_PO3100K(0x09,0x86);
    i2cWrite_PO3100K(0x0A,0x00);
    i2cWrite_PO3100K(0x0B,0x25);
    i2cWrite_PO3100K(0x0C,0x25);
    i2cWrite_PO3100K(0x0D,0x88);
    i2cWrite_PO3100K(0x0E,0x00);
    i2cWrite_PO3100K(0x0F,0x25);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x19,0x2A);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0x56,0x00);
    i2cWrite_PO3100K(0x57,0x04);
    i2cWrite_PO3100K(0x58,0x08);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x3B,0x66);
    i2cWrite_PO3100K(0x3C,0x66);
    i2cWrite_PO3100K(0x3D,0x66);
    i2cWrite_PO3100K(0x3E,0x66);
    i2cWrite_PO3100K(0x3F,0x24);
    i2cWrite_PO3100K(0x40,0x4B);
    i2cWrite_PO3100K(0x03,0x04);
    i2cWrite_PO3100K(0x34,0x08);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA1,0x20);
    i2cWrite_PO3100K(0xA2,0x20);
    i2cWrite_PO3100K(0xA3,0x20);
    i2cWrite_PO3100K(0x03,0x03);
    i2cWrite_PO3100K(0xA5,0x6F);
    i2cWrite_PO3100K(0xA6,0x6F);
    i2cWrite_PO3100K(0xA7,0x6F);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x27,0x66);
    i2cWrite_PO3100K(0x28,0x50);
    i2cWrite_PO3100K(0x03,0x00);
    i2cWrite_PO3100K(0x40,0x2C);
    i2cWrite_PO3100K(0x03,0x02);
    i2cWrite_PO3100K(0x8B,0x00);

    siuSetSensorDayNight(DayNightMode);
}


void SetPO3100K_720P_Change_to_27FPS(void)
{
    //[Change_to_27_Fps]
    DEBUG_SIU("SetPO3100K_720P_Change_to_27FPS()\n");
    i2cWrite_PO3100K(0x03, 0x00);
    i2cWrite_PO3100K(0x08, 0x03);
    i2cWrite_PO3100K(0x09, 0x40);
    i2cWrite_PO3100K(0x0A, 0x03);
    i2cWrite_PO3100K(0x0B, 0x40);
}

// 20141121 & 20141211
void SetPO3100K_720P_Change_to_30FPS(void)
{
    //[Change_to_30_Fps]
    DEBUG_SIU("SetPO3100K_720P_Change_to_30FPS()\n");
    i2cWrite_PO3100K(0x03, 0x00);
    i2cWrite_PO3100K(0x08, 0x02);
    i2cWrite_PO3100K(0x09, 0xED);
    i2cWrite_PO3100K(0x0A, 0x02);
    i2cWrite_PO3100K(0x0B, 0xED);
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
	u8  data, level;
    u32 i;
    u32 tempReg;
    int FrameRate=30;
	u8	data1=0;
    //----------//
    
#if(FPGA_BOARD_A1018_SERIES)
    //---------Run on Mclk=32 MHz--------------//    
    #if(SYS_CPU_CLK_FREQ == 32000000)
	  SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x00; //MClk=32/1=32MHz
	#elif(SYS_CPU_CLK_FREQ == 48000000)
      SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
	#endif    
#else   
   //---------Run on Mclk=27 MHz--------------//
   #if( (SYS_CPU_CLK_FREQ == 160000000) || (SYS_CPU_CLK_FREQ == 162000000) )
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

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 

            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                if(PO3100K_720P_FrameRate == 30)
                {
                    FrameRate   = 30;
                    SetPO3100K_720P_30FPS();
                }
                else if(PO3100K_720P_FrameRate == 27)
                {
                    FrameRate   = 27;
                    SetPO3100K_720P_30FPS();
                    SetPO3100K_720P_Change_to_27FPS();
                }
            }
			else
		    {
                    FrameRate   = 30;
                    SetPO3100K_VGA_30FPS();
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

    siuY_TargetIndex = expBiasValue;

    i2cWrite_PO3100K(0x03,0x04);
//    i2cWrite_PO3100K(0x05,0x64);
    i2cWrite_PO3100K(0x3B,AETargetMeanTab[siuY_TargetIndex]);
    i2cWrite_PO3100K(0x3C,AETargetMeanTab[siuY_TargetIndex]);
    i2cWrite_PO3100K(0x3D,AETargetMeanTab[siuY_TargetIndex]);
    i2cWrite_PO3100K(0x3E,AETargetMeanTab[siuY_TargetIndex]);
//    i2cWrite_PO3100K(0x3F,0x24);
//    i2cWrite_PO3100K(0x40,0x4B);

	return 1;
}


s32 siuSetFlicker50_60Hz(int flicker_sel)
{
	u8	data;
    int count;
#if 1

    if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        DEBUG_SIU("siuSetFlicker50_60Hz 60\n");
        i2cWrite_PO3100K(0x03,0x00);
        i2cWrite_PO3100K(0x4F,0x08);
    }
    else //50Hz
    {
        DEBUG_SIU("siuSetFlicker50_60Hz 50\n");
        i2cWrite_PO3100K(0x03,0x00);
        i2cWrite_PO3100K(0x4F,0x04);
    }

#endif
    return 0;
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

void siu_MI1320PowerDown(void)
{
   i2cWrite_SENSOR(0x0d, 0x0006);
}

void siu_MI1320PowerUp(void)
{
   i2cWrite_SENSOR(0x0d, 0x0000);
}

void siu_SetPO3100K_720P_FrameRate(u32 FrameRate)
{

    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
    {
        if(PO3100K_720P_FrameRate  != FrameRate)
        {
            DEBUG_SIU("siu_SetPO3100K_720P_FrameRate(%d)\n", FrameRate);
            if(FrameRate == 30)
            {
                SetPO3100K_720P_Change_to_30FPS();
                PO3100K_720P_FrameRate=30;
            }
            else if(FrameRate == 27)
            {
                SetPO3100K_720P_Change_to_27FPS();
                PO3100K_720P_FrameRate=27;
            }
        }
    }
}



#endif	






