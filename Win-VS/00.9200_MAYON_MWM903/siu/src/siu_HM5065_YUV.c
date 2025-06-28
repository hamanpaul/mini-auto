
/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu_HM5065_YUV.c

Abstract:

    The routines of Sensor Interface Unit.
    Control HM5065 (5M) sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2104/07/28  Peter Hsu Create  
*/


#include "general.h"
#if (Sensor_OPTION == Sensor_HM5065_YUV601)
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
const s8 AETargetMeanTab[9] = {0x00, 0x0c, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c};

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

void SetHM5065_1280x960_30FPS(void)
{
    //DEBUG_SIU("SetHM5065_1280x960_30FPS()\n");

    //*************************************************************************
    // Sensor: HM5065
    // I2C ID: 1F
    // Resolution: 2592x1544(5M)
    // Lens: 
    // Flicker: Auto detect
    // Frequency(In) : 12M
    // Frequency(Out): 80M
    // Description: 
    //   Lens Type : L9545
    //   VCM Type : JCT-B3360
    // Note:
    //   For Gen1. Image is rotated
    // IQ Rev.: 
    // Release: 2013-12-24
    // -----------
    // $Revision: 689 $
    // $Date:: 2014-04-07 14:01:47 +0800#$
    //*************************************************************************
    
    //*************************************************************************
    // 926BA Firmware patch, v1.20 + Himax WB freeze threshold
    // Date: 13-Aug-2013
    //*************************************************************************
    
    i2cWrite_HM5065(0x0010, 0x02); // CMD_PowerOff
    OSTimeDly(4);

    i2cWrite_HM5065(0xffff, 0x01);	// MCU bypass//
    i2cWrite_HM5065(0x9000, 0x03);	// Enable Ram and enable Write//
    
    ////////////////////////////////////////////////
    // A000~A47A, 1147 bytes 
    ////////////////////////////////////////////////
    i2cWrite_HM5065(0xA000, 0x90);
    i2cWrite_HM5065(0xA001, 0x0C);
    i2cWrite_HM5065(0xA002, 0x56);
    i2cWrite_HM5065(0xA003, 0xE0);
    i2cWrite_HM5065(0xA004, 0xFE);
    i2cWrite_HM5065(0xA005, 0xA3);
    i2cWrite_HM5065(0xA006, 0xE0);
    i2cWrite_HM5065(0xA007, 0xFF);
    i2cWrite_HM5065(0xA008, 0x12);
    i2cWrite_HM5065(0xA009, 0x42);
    i2cWrite_HM5065(0xA00A, 0x85);
    i2cWrite_HM5065(0xA00B, 0x90);
    i2cWrite_HM5065(0xA00C, 0x01);
    i2cWrite_HM5065(0xA00D, 0xB7);
    i2cWrite_HM5065(0xA00E, 0xEE);
    i2cWrite_HM5065(0xA00F, 0xF0);
    i2cWrite_HM5065(0xA010, 0xFC);
    i2cWrite_HM5065(0xA011, 0xA3);
    i2cWrite_HM5065(0xA012, 0xEF);
    i2cWrite_HM5065(0xA013, 0xF0);
    i2cWrite_HM5065(0xA014, 0xFD);
    i2cWrite_HM5065(0xA015, 0x90);
    i2cWrite_HM5065(0xA016, 0x06);
    i2cWrite_HM5065(0xA017, 0x05);
    i2cWrite_HM5065(0xA018, 0xE0);
    i2cWrite_HM5065(0xA019, 0x75);
    i2cWrite_HM5065(0xA01A, 0xF0);
    i2cWrite_HM5065(0xA01B, 0x02);
    i2cWrite_HM5065(0xA01C, 0xA4);
    i2cWrite_HM5065(0xA01D, 0x2D);
    i2cWrite_HM5065(0xA01E, 0xFF);
    i2cWrite_HM5065(0xA01F, 0xE5);
    i2cWrite_HM5065(0xA020, 0xF0);
    i2cWrite_HM5065(0xA021, 0x3C);
    i2cWrite_HM5065(0xA022, 0xFE);
    i2cWrite_HM5065(0xA023, 0xAB);
    i2cWrite_HM5065(0xA024, 0x07);
    i2cWrite_HM5065(0xA025, 0xFA);
    i2cWrite_HM5065(0xA026, 0x33);
    i2cWrite_HM5065(0xA027, 0x95);
    i2cWrite_HM5065(0xA028, 0xE0);
    i2cWrite_HM5065(0xA029, 0xF9);
    i2cWrite_HM5065(0xA02A, 0xF8);
    i2cWrite_HM5065(0xA02B, 0x90);
    i2cWrite_HM5065(0xA02C, 0x0B);
    i2cWrite_HM5065(0xA02D, 0x4B);
    i2cWrite_HM5065(0xA02E, 0xE0);
    i2cWrite_HM5065(0xA02F, 0xFE);
    i2cWrite_HM5065(0xA030, 0xA3);
    i2cWrite_HM5065(0xA031, 0xE0);
    i2cWrite_HM5065(0xA032, 0xFF);
    i2cWrite_HM5065(0xA033, 0xEE);
    i2cWrite_HM5065(0xA034, 0x33);
    i2cWrite_HM5065(0xA035, 0x95);
    i2cWrite_HM5065(0xA036, 0xE0);
    i2cWrite_HM5065(0xA037, 0xFD);
    i2cWrite_HM5065(0xA038, 0xFC);
    i2cWrite_HM5065(0xA039, 0x12);
    i2cWrite_HM5065(0xA03A, 0x0C);
    i2cWrite_HM5065(0xA03B, 0x7B);
    i2cWrite_HM5065(0xA03C, 0x90);
    i2cWrite_HM5065(0xA03D, 0x01);
    i2cWrite_HM5065(0xA03E, 0xB9);
    i2cWrite_HM5065(0xA03F, 0x12);
    i2cWrite_HM5065(0xA040, 0x0E);
    i2cWrite_HM5065(0xA041, 0x05);
    i2cWrite_HM5065(0xA042, 0x90);
    i2cWrite_HM5065(0xA043, 0x01);
    i2cWrite_HM5065(0xA044, 0xB9);
    i2cWrite_HM5065(0xA045, 0xE0);
    i2cWrite_HM5065(0xA046, 0xFC);
    i2cWrite_HM5065(0xA047, 0xA3);
    i2cWrite_HM5065(0xA048, 0xE0);
    i2cWrite_HM5065(0xA049, 0xFD);
    i2cWrite_HM5065(0xA04A, 0xA3);
    i2cWrite_HM5065(0xA04B, 0xE0);
    i2cWrite_HM5065(0xA04C, 0xFE);
    i2cWrite_HM5065(0xA04D, 0xA3);
    i2cWrite_HM5065(0xA04E, 0xE0);
    i2cWrite_HM5065(0xA04F, 0xFF);
    i2cWrite_HM5065(0xA050, 0x78);
    i2cWrite_HM5065(0xA051, 0x08);
    i2cWrite_HM5065(0xA052, 0x12);
    i2cWrite_HM5065(0xA053, 0x0D);
    i2cWrite_HM5065(0xA054, 0xBF);
    i2cWrite_HM5065(0xA055, 0xA8);
    i2cWrite_HM5065(0xA056, 0x04);
    i2cWrite_HM5065(0xA057, 0xA9);
    i2cWrite_HM5065(0xA058, 0x05);
    i2cWrite_HM5065(0xA059, 0xAA);
    i2cWrite_HM5065(0xA05A, 0x06);
    i2cWrite_HM5065(0xA05B, 0xAB);
    i2cWrite_HM5065(0xA05C, 0x07);
    i2cWrite_HM5065(0xA05D, 0x90);
    i2cWrite_HM5065(0xA05E, 0x0B);
    i2cWrite_HM5065(0xA05F, 0x49);
    i2cWrite_HM5065(0xA060, 0xE0);
    i2cWrite_HM5065(0xA061, 0xFE);
    i2cWrite_HM5065(0xA062, 0xA3);
    i2cWrite_HM5065(0xA063, 0xE0);
    i2cWrite_HM5065(0xA064, 0xFF);
    i2cWrite_HM5065(0xA065, 0xEE);
    i2cWrite_HM5065(0xA066, 0x33);
    i2cWrite_HM5065(0xA067, 0x95);
    i2cWrite_HM5065(0xA068, 0xE0);
    i2cWrite_HM5065(0xA069, 0xFD);
    i2cWrite_HM5065(0xA06A, 0xFC);
    i2cWrite_HM5065(0xA06B, 0xC3);
    i2cWrite_HM5065(0xA06C, 0xEF);
    i2cWrite_HM5065(0xA06D, 0x9B);
    i2cWrite_HM5065(0xA06E, 0xFF);
    i2cWrite_HM5065(0xA06F, 0xEE);
    i2cWrite_HM5065(0xA070, 0x9A);
    i2cWrite_HM5065(0xA071, 0xFE);
    i2cWrite_HM5065(0xA072, 0xED);
    i2cWrite_HM5065(0xA073, 0x99);
    i2cWrite_HM5065(0xA074, 0xFD);
    i2cWrite_HM5065(0xA075, 0xEC);
    i2cWrite_HM5065(0xA076, 0x98);
    i2cWrite_HM5065(0xA077, 0xFC);
    i2cWrite_HM5065(0xA078, 0x78);
    i2cWrite_HM5065(0xA079, 0x01);
    i2cWrite_HM5065(0xA07A, 0x12);
    i2cWrite_HM5065(0xA07B, 0x0D);
    i2cWrite_HM5065(0xA07C, 0xBF);
    i2cWrite_HM5065(0xA07D, 0x90);
    i2cWrite_HM5065(0xA07E, 0x0C);
    i2cWrite_HM5065(0xA07F, 0x4A);
    i2cWrite_HM5065(0xA080, 0xE0);
    i2cWrite_HM5065(0xA081, 0xFC);
    i2cWrite_HM5065(0xA082, 0xA3);
    i2cWrite_HM5065(0xA083, 0xE0);
    i2cWrite_HM5065(0xA084, 0xF5);
    i2cWrite_HM5065(0xA085, 0x82);
    i2cWrite_HM5065(0xA086, 0x8C);
    i2cWrite_HM5065(0xA087, 0x83);
    i2cWrite_HM5065(0xA088, 0xC0);
    i2cWrite_HM5065(0xA089, 0x83);
    i2cWrite_HM5065(0xA08A, 0xC0);
    i2cWrite_HM5065(0xA08B, 0x82);
    i2cWrite_HM5065(0xA08C, 0x90);
    i2cWrite_HM5065(0xA08D, 0x0B);
    i2cWrite_HM5065(0xA08E, 0x48);
    i2cWrite_HM5065(0xA08F, 0xE0);
    i2cWrite_HM5065(0xA090, 0xD0);
    i2cWrite_HM5065(0xA091, 0x82);
    i2cWrite_HM5065(0xA092, 0xD0);
    i2cWrite_HM5065(0xA093, 0x83);
    i2cWrite_HM5065(0xA094, 0x75);
    i2cWrite_HM5065(0xA095, 0xF0);
    i2cWrite_HM5065(0xA096, 0x02);
    i2cWrite_HM5065(0xA097, 0x12);
    i2cWrite_HM5065(0xA098, 0x0E);
    i2cWrite_HM5065(0xA099, 0x45);
    i2cWrite_HM5065(0xA09A, 0xEE);
    i2cWrite_HM5065(0xA09B, 0xF0);
    i2cWrite_HM5065(0xA09C, 0xA3);
    i2cWrite_HM5065(0xA09D, 0xEF);
    i2cWrite_HM5065(0xA09E, 0xF0);
    i2cWrite_HM5065(0xA09F, 0x02);
    i2cWrite_HM5065(0xA0A0, 0xBA);
    i2cWrite_HM5065(0xA0A1, 0xD8);
    i2cWrite_HM5065(0xA0A2, 0x90);
    i2cWrite_HM5065(0xA0A3, 0x30);
    i2cWrite_HM5065(0xA0A4, 0x18);
    i2cWrite_HM5065(0xA0A5, 0xe4);
    i2cWrite_HM5065(0xA0A6, 0xf0);
    i2cWrite_HM5065(0xA0A7, 0x74);
    i2cWrite_HM5065(0xA0A8, 0x3f);
    i2cWrite_HM5065(0xA0A9, 0xf0);
    i2cWrite_HM5065(0xA0AA, 0x22);
    i2cWrite_HM5065(0xA0AB, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0AC, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0AD, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0AE, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0AF, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B0, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B1, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B2, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B3, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B4, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B5, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B6, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B7, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B8, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0B9, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BA, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BB, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BC, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BD, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BE, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA0BF, 0x90);
    i2cWrite_HM5065(0xA0C0, 0x00);
    i2cWrite_HM5065(0xA0C1, 0x5E);
    i2cWrite_HM5065(0xA0C2, 0xE0);
    i2cWrite_HM5065(0xA0C3, 0xFF);
    i2cWrite_HM5065(0xA0C4, 0x70);
    i2cWrite_HM5065(0xA0C5, 0x20);
    i2cWrite_HM5065(0xA0C6, 0x90);
    i2cWrite_HM5065(0xA0C7, 0x47);
    i2cWrite_HM5065(0xA0C8, 0x04);
    i2cWrite_HM5065(0xA0C9, 0x74);
    i2cWrite_HM5065(0xA0CA, 0x0A);
    i2cWrite_HM5065(0xA0CB, 0xF0);
    i2cWrite_HM5065(0xA0CC, 0xA3);
    i2cWrite_HM5065(0xA0CD, 0x74);
    i2cWrite_HM5065(0xA0CE, 0x30);
    i2cWrite_HM5065(0xA0CF, 0xF0);
    i2cWrite_HM5065(0xA0D0, 0x90);
    i2cWrite_HM5065(0xA0D1, 0x47);
    i2cWrite_HM5065(0xA0D2, 0x0C);
    i2cWrite_HM5065(0xA0D3, 0x74);
    i2cWrite_HM5065(0xA0D4, 0x07);
    i2cWrite_HM5065(0xA0D5, 0xF0);
    i2cWrite_HM5065(0xA0D6, 0xA3);
    i2cWrite_HM5065(0xA0D7, 0x74);
    i2cWrite_HM5065(0xA0D8, 0xA8);
    i2cWrite_HM5065(0xA0D9, 0xF0);
    i2cWrite_HM5065(0xA0DA, 0x90);
    i2cWrite_HM5065(0xA0DB, 0x47);
    i2cWrite_HM5065(0xA0DC, 0xA4);
    i2cWrite_HM5065(0xA0DD, 0x74);
    i2cWrite_HM5065(0xA0DE, 0x01);
    i2cWrite_HM5065(0xA0DF, 0xF0);
    i2cWrite_HM5065(0xA0E0, 0x90);
    i2cWrite_HM5065(0xA0E1, 0x47);
    i2cWrite_HM5065(0xA0E2, 0xA8);
    i2cWrite_HM5065(0xA0E3, 0xF0);
    i2cWrite_HM5065(0xA0E4, 0x80);
    i2cWrite_HM5065(0xA0E5, 0x50);
    i2cWrite_HM5065(0xA0E6, 0xEF);
    i2cWrite_HM5065(0xA0E7, 0x64);
    i2cWrite_HM5065(0xA0E8, 0x01);
    i2cWrite_HM5065(0xA0E9, 0x60);
    i2cWrite_HM5065(0xA0EA, 0x04);
    i2cWrite_HM5065(0xA0EB, 0xEF);
    i2cWrite_HM5065(0xA0EC, 0xB4);
    i2cWrite_HM5065(0xA0ED, 0x03);
    i2cWrite_HM5065(0xA0EE, 0x20);
    i2cWrite_HM5065(0xA0EF, 0x90);
    i2cWrite_HM5065(0xA0F0, 0x47);
    i2cWrite_HM5065(0xA0F1, 0x04);
    i2cWrite_HM5065(0xA0F2, 0x74);
    i2cWrite_HM5065(0xA0F3, 0x05);
    i2cWrite_HM5065(0xA0F4, 0xF0);
    i2cWrite_HM5065(0xA0F5, 0xA3);
    i2cWrite_HM5065(0xA0F6, 0x74);
    i2cWrite_HM5065(0xA0F7, 0x18);
    i2cWrite_HM5065(0xA0F8, 0xF0);
    i2cWrite_HM5065(0xA0F9, 0x90);
    i2cWrite_HM5065(0xA0FA, 0x47);
    i2cWrite_HM5065(0xA0FB, 0x0C);
    i2cWrite_HM5065(0xA0FC, 0x74);
    i2cWrite_HM5065(0xA0FD, 0x03);
    i2cWrite_HM5065(0xA0FE, 0xF0);
    i2cWrite_HM5065(0xA0FF, 0xA3);
    i2cWrite_HM5065(0xA100, 0x74);
    i2cWrite_HM5065(0xA101, 0xD4);
    i2cWrite_HM5065(0xA102, 0xF0);
    i2cWrite_HM5065(0xA103, 0x90);
    i2cWrite_HM5065(0xA104, 0x47);
    i2cWrite_HM5065(0xA105, 0xA4);
    i2cWrite_HM5065(0xA106, 0x74);
    i2cWrite_HM5065(0xA107, 0x02);
    i2cWrite_HM5065(0xA108, 0xF0);
    i2cWrite_HM5065(0xA109, 0x90);
    i2cWrite_HM5065(0xA10A, 0x47);
    i2cWrite_HM5065(0xA10B, 0xA8);
    i2cWrite_HM5065(0xA10C, 0xF0);
    i2cWrite_HM5065(0xA10D, 0x80);
    i2cWrite_HM5065(0xA10E, 0x27);
    i2cWrite_HM5065(0xA10F, 0xEF);
    i2cWrite_HM5065(0xA110, 0x64);
    i2cWrite_HM5065(0xA111, 0x02);
    i2cWrite_HM5065(0xA112, 0x60);
    i2cWrite_HM5065(0xA113, 0x04);
    i2cWrite_HM5065(0xA114, 0xEF);
    i2cWrite_HM5065(0xA115, 0xB4);
    i2cWrite_HM5065(0xA116, 0x04);
    i2cWrite_HM5065(0xA117, 0x1E);
    i2cWrite_HM5065(0xA118, 0x90);
    i2cWrite_HM5065(0xA119, 0x47);
    i2cWrite_HM5065(0xA11A, 0x04);
    i2cWrite_HM5065(0xA11B, 0x74);
    i2cWrite_HM5065(0xA11C, 0x02);
    i2cWrite_HM5065(0xA11D, 0xF0);
    i2cWrite_HM5065(0xA11E, 0xA3);
    i2cWrite_HM5065(0xA11F, 0x74);
    i2cWrite_HM5065(0xA120, 0x8C);
    i2cWrite_HM5065(0xA121, 0xF0);
    i2cWrite_HM5065(0xA122, 0x90);
    i2cWrite_HM5065(0xA123, 0x47);
    i2cWrite_HM5065(0xA124, 0x0C);
    i2cWrite_HM5065(0xA125, 0x74);
    i2cWrite_HM5065(0xA126, 0x01);
    i2cWrite_HM5065(0xA127, 0xF0);
    i2cWrite_HM5065(0xA128, 0xA3);
    i2cWrite_HM5065(0xA129, 0x74);
    i2cWrite_HM5065(0xA12A, 0xEA);
    i2cWrite_HM5065(0xA12B, 0xF0);
    i2cWrite_HM5065(0xA12C, 0x90);
    i2cWrite_HM5065(0xA12D, 0x47);
    i2cWrite_HM5065(0xA12E, 0xA4);
    i2cWrite_HM5065(0xA12F, 0x74);
    i2cWrite_HM5065(0xA130, 0x04);
    i2cWrite_HM5065(0xA131, 0xF0);
    i2cWrite_HM5065(0xA132, 0x90);
    i2cWrite_HM5065(0xA133, 0x47);
    i2cWrite_HM5065(0xA134, 0xA8);
    i2cWrite_HM5065(0xA135, 0xF0);
    i2cWrite_HM5065(0xA136, 0x22);
    i2cWrite_HM5065(0xA137, 0x74);
    i2cWrite_HM5065(0xA138, 0x04);
    i2cWrite_HM5065(0xA139, 0xF0);
    i2cWrite_HM5065(0xA13A, 0xA3);
    i2cWrite_HM5065(0xA13B, 0x74);
    i2cWrite_HM5065(0xA13C, 0x20);
    i2cWrite_HM5065(0xA13D, 0xF0);
    i2cWrite_HM5065(0xA13E, 0xE4);
    i2cWrite_HM5065(0xA13F, 0xF5);
    i2cWrite_HM5065(0xA140, 0x22);
    i2cWrite_HM5065(0xA141, 0xE5);
    i2cWrite_HM5065(0xA142, 0x22);
    i2cWrite_HM5065(0xA143, 0xC3);
    i2cWrite_HM5065(0xA144, 0x94);
    i2cWrite_HM5065(0xA145, 0x40);
    i2cWrite_HM5065(0xA146, 0x40);
    i2cWrite_HM5065(0xA147, 0x03);
    i2cWrite_HM5065(0xA148, 0x02);
    i2cWrite_HM5065(0xA149, 0xF1);
    i2cWrite_HM5065(0xA14A, 0xFD);
    i2cWrite_HM5065(0xA14B, 0x90);
    i2cWrite_HM5065(0xA14C, 0x0A);
    i2cWrite_HM5065(0xA14D, 0xBA);
    i2cWrite_HM5065(0xA14E, 0xE0);
    i2cWrite_HM5065(0xA14F, 0xFE);
    i2cWrite_HM5065(0xA150, 0xA3);
    i2cWrite_HM5065(0xA151, 0xE0);
    i2cWrite_HM5065(0xA152, 0xFF);
    i2cWrite_HM5065(0xA153, 0xF5);
    i2cWrite_HM5065(0xA154, 0x82);
    i2cWrite_HM5065(0xA155, 0x8E);
    i2cWrite_HM5065(0xA156, 0x83);
    i2cWrite_HM5065(0xA157, 0xE0);
    i2cWrite_HM5065(0xA158, 0x54);
    i2cWrite_HM5065(0xA159, 0x70);
    i2cWrite_HM5065(0xA15A, 0xFD);
    i2cWrite_HM5065(0xA15B, 0xC4);
    i2cWrite_HM5065(0xA15C, 0x54);
    i2cWrite_HM5065(0xA15D, 0x0F);
    i2cWrite_HM5065(0xA15E, 0xFD);
    i2cWrite_HM5065(0xA15F, 0x90);
    i2cWrite_HM5065(0xA160, 0x0A);
    i2cWrite_HM5065(0xA161, 0xBC);
    i2cWrite_HM5065(0xA162, 0xE0);
    i2cWrite_HM5065(0xA163, 0xFA);
    i2cWrite_HM5065(0xA164, 0xA3);
    i2cWrite_HM5065(0xA165, 0xE0);
    i2cWrite_HM5065(0xA166, 0xF5);
    i2cWrite_HM5065(0xA167, 0x82);
    i2cWrite_HM5065(0xA168, 0x8A);
    i2cWrite_HM5065(0xA169, 0x83);
    i2cWrite_HM5065(0xA16A, 0xED);
    i2cWrite_HM5065(0xA16B, 0xF0);
    i2cWrite_HM5065(0xA16C, 0x90);
    i2cWrite_HM5065(0xA16D, 0x0A);
    i2cWrite_HM5065(0xA16E, 0xBD);
    i2cWrite_HM5065(0xA16F, 0xE0);
    i2cWrite_HM5065(0xA170, 0x04);
    i2cWrite_HM5065(0xA171, 0xF0);
    i2cWrite_HM5065(0xA172, 0x70);
    i2cWrite_HM5065(0xA173, 0x06);
    i2cWrite_HM5065(0xA174, 0x90);
    i2cWrite_HM5065(0xA175, 0x0A);
    i2cWrite_HM5065(0xA176, 0xBC);
    i2cWrite_HM5065(0xA177, 0xE0);
    i2cWrite_HM5065(0xA178, 0x04);
    i2cWrite_HM5065(0xA179, 0xF0);
    i2cWrite_HM5065(0xA17A, 0x8F);
    i2cWrite_HM5065(0xA17B, 0x82);
    i2cWrite_HM5065(0xA17C, 0x8E);
    i2cWrite_HM5065(0xA17D, 0x83);
    i2cWrite_HM5065(0xA17E, 0xA3);
    i2cWrite_HM5065(0xA17F, 0xE0);
    i2cWrite_HM5065(0xA180, 0xFF);
    i2cWrite_HM5065(0xA181, 0x90);
    i2cWrite_HM5065(0xA182, 0x0A);
    i2cWrite_HM5065(0xA183, 0xBC);
    i2cWrite_HM5065(0xA184, 0xE0);
    i2cWrite_HM5065(0xA185, 0xFC);
    i2cWrite_HM5065(0xA186, 0xA3);
    i2cWrite_HM5065(0xA187, 0xE0);
    i2cWrite_HM5065(0xA188, 0xF5);
    i2cWrite_HM5065(0xA189, 0x82);
    i2cWrite_HM5065(0xA18A, 0x8C);
    i2cWrite_HM5065(0xA18B, 0x83);
    i2cWrite_HM5065(0xA18C, 0xEF);
    i2cWrite_HM5065(0xA18D, 0xF0);
    i2cWrite_HM5065(0xA18E, 0x90);
    i2cWrite_HM5065(0xA18F, 0x0A);
    i2cWrite_HM5065(0xA190, 0xBD);
    i2cWrite_HM5065(0xA191, 0xE0);
    i2cWrite_HM5065(0xA192, 0x04);
    i2cWrite_HM5065(0xA193, 0xF0);
    i2cWrite_HM5065(0xA194, 0x70);
    i2cWrite_HM5065(0xA195, 0x06);
    i2cWrite_HM5065(0xA196, 0x90);
    i2cWrite_HM5065(0xA197, 0x0A);
    i2cWrite_HM5065(0xA198, 0xBC);
    i2cWrite_HM5065(0xA199, 0xE0);
    i2cWrite_HM5065(0xA19A, 0x04);
    i2cWrite_HM5065(0xA19B, 0xF0);
    i2cWrite_HM5065(0xA19C, 0x90);
    i2cWrite_HM5065(0xA19D, 0x0A);
    i2cWrite_HM5065(0xA19E, 0xBA);
    i2cWrite_HM5065(0xA19F, 0xE0);
    i2cWrite_HM5065(0xA1A0, 0xFE);
    i2cWrite_HM5065(0xA1A1, 0xA3);
    i2cWrite_HM5065(0xA1A2, 0xE0);
    i2cWrite_HM5065(0xA1A3, 0xFF);
    i2cWrite_HM5065(0xA1A4, 0xF5);
    i2cWrite_HM5065(0xA1A5, 0x82);
    i2cWrite_HM5065(0xA1A6, 0x8E);
    i2cWrite_HM5065(0xA1A7, 0x83);
    i2cWrite_HM5065(0xA1A8, 0xE0);
    i2cWrite_HM5065(0xA1A9, 0x54);
    i2cWrite_HM5065(0xA1AA, 0x07);
    i2cWrite_HM5065(0xA1AB, 0xFD);
    i2cWrite_HM5065(0xA1AC, 0x90);
    i2cWrite_HM5065(0xA1AD, 0x0A);
    i2cWrite_HM5065(0xA1AE, 0xBC);
    i2cWrite_HM5065(0xA1AF, 0xE0);
    i2cWrite_HM5065(0xA1B0, 0xFA);
    i2cWrite_HM5065(0xA1B1, 0xA3);
    i2cWrite_HM5065(0xA1B2, 0xE0);
    i2cWrite_HM5065(0xA1B3, 0xF5);
    i2cWrite_HM5065(0xA1B4, 0x82);
    i2cWrite_HM5065(0xA1B5, 0x8A);
    i2cWrite_HM5065(0xA1B6, 0x83);
    i2cWrite_HM5065(0xA1B7, 0xED);
    i2cWrite_HM5065(0xA1B8, 0xF0);
    i2cWrite_HM5065(0xA1B9, 0x90);
    i2cWrite_HM5065(0xA1BA, 0x0A);
    i2cWrite_HM5065(0xA1BB, 0xBD);
    i2cWrite_HM5065(0xA1BC, 0xE0);
    i2cWrite_HM5065(0xA1BD, 0x04);
    i2cWrite_HM5065(0xA1BE, 0xF0);
    i2cWrite_HM5065(0xA1BF, 0x70);
    i2cWrite_HM5065(0xA1C0, 0x06);
    i2cWrite_HM5065(0xA1C1, 0x90);
    i2cWrite_HM5065(0xA1C2, 0x0A);
    i2cWrite_HM5065(0xA1C3, 0xBC);
    i2cWrite_HM5065(0xA1C4, 0xE0);
    i2cWrite_HM5065(0xA1C5, 0x04);
    i2cWrite_HM5065(0xA1C6, 0xF0);
    i2cWrite_HM5065(0xA1C7, 0x8F);
    i2cWrite_HM5065(0xA1C8, 0x82);
    i2cWrite_HM5065(0xA1C9, 0x8E);
    i2cWrite_HM5065(0xA1CA, 0x83);
    i2cWrite_HM5065(0xA1CB, 0xA3);
    i2cWrite_HM5065(0xA1CC, 0xA3);
    i2cWrite_HM5065(0xA1CD, 0xE0);
    i2cWrite_HM5065(0xA1CE, 0xFF);
    i2cWrite_HM5065(0xA1CF, 0x90);
    i2cWrite_HM5065(0xA1D0, 0x0A);
    i2cWrite_HM5065(0xA1D1, 0xBC);
    i2cWrite_HM5065(0xA1D2, 0xE0);
    i2cWrite_HM5065(0xA1D3, 0xFC);
    i2cWrite_HM5065(0xA1D4, 0xA3);
    i2cWrite_HM5065(0xA1D5, 0xE0);
    i2cWrite_HM5065(0xA1D6, 0xF5);
    i2cWrite_HM5065(0xA1D7, 0x82);
    i2cWrite_HM5065(0xA1D8, 0x8C);
    i2cWrite_HM5065(0xA1D9, 0x83);
    i2cWrite_HM5065(0xA1DA, 0xEF);
    i2cWrite_HM5065(0xA1DB, 0xF0);
    i2cWrite_HM5065(0xA1DC, 0x90);
    i2cWrite_HM5065(0xA1DD, 0x0A);
    i2cWrite_HM5065(0xA1DE, 0xBD);
    i2cWrite_HM5065(0xA1DF, 0xE0);
    i2cWrite_HM5065(0xA1E0, 0x04);
    i2cWrite_HM5065(0xA1E1, 0xF0);
    i2cWrite_HM5065(0xA1E2, 0x70);
    i2cWrite_HM5065(0xA1E3, 0x06);
    i2cWrite_HM5065(0xA1E4, 0x90);
    i2cWrite_HM5065(0xA1E5, 0x0A);
    i2cWrite_HM5065(0xA1E6, 0xBC);
    i2cWrite_HM5065(0xA1E7, 0xE0);
    i2cWrite_HM5065(0xA1E8, 0x04);
    i2cWrite_HM5065(0xA1E9, 0xF0);
    i2cWrite_HM5065(0xA1EA, 0x90);
    i2cWrite_HM5065(0xA1EB, 0x0A);
    i2cWrite_HM5065(0xA1EC, 0xBB);
    i2cWrite_HM5065(0xA1ED, 0xE0);
    i2cWrite_HM5065(0xA1EE, 0x24);
    i2cWrite_HM5065(0xA1EF, 0x03);
    i2cWrite_HM5065(0xA1F0, 0xF0);
    i2cWrite_HM5065(0xA1F1, 0x90);
    i2cWrite_HM5065(0xA1F2, 0x0A);
    i2cWrite_HM5065(0xA1F3, 0xBA);
    i2cWrite_HM5065(0xA1F4, 0xE0);
    i2cWrite_HM5065(0xA1F5, 0x34);
    i2cWrite_HM5065(0xA1F6, 0x00);
    i2cWrite_HM5065(0xA1F7, 0xF0);
    i2cWrite_HM5065(0xA1F8, 0x05);
    i2cWrite_HM5065(0xA1F9, 0x22);
    i2cWrite_HM5065(0xA1FA, 0x02);
    i2cWrite_HM5065(0xA1FB, 0xF1);
    i2cWrite_HM5065(0xA1FC, 0x41);
    i2cWrite_HM5065(0xA1FD, 0x90);
    i2cWrite_HM5065(0xA1FE, 0x0A);
    i2cWrite_HM5065(0xA1FF, 0xBA);
    i2cWrite_HM5065(0xA200, 0x74);
    i2cWrite_HM5065(0xA201, 0x0E);
    i2cWrite_HM5065(0xA202, 0xF0);
    i2cWrite_HM5065(0xA203, 0xA3);
    i2cWrite_HM5065(0xA204, 0x74);
    i2cWrite_HM5065(0xA205, 0xDC);
    i2cWrite_HM5065(0xA206, 0xF0);
    i2cWrite_HM5065(0xA207, 0xA3);
    i2cWrite_HM5065(0xA208, 0x74);
    i2cWrite_HM5065(0xA209, 0x05);
    i2cWrite_HM5065(0xA20A, 0xF0);
    i2cWrite_HM5065(0xA20B, 0xA3);
    i2cWrite_HM5065(0xA20C, 0x74);
    i2cWrite_HM5065(0xA20D, 0x61);
    i2cWrite_HM5065(0xA20E, 0xF0);
    i2cWrite_HM5065(0xA20F, 0x90);
    i2cWrite_HM5065(0xA210, 0x0A);
    i2cWrite_HM5065(0xA211, 0xBA);
    i2cWrite_HM5065(0xA212, 0xE0);
    i2cWrite_HM5065(0xA213, 0xFE);
    i2cWrite_HM5065(0xA214, 0xA3);
    i2cWrite_HM5065(0xA215, 0xE0);
    i2cWrite_HM5065(0xA216, 0xAA);
    i2cWrite_HM5065(0xA217, 0x06);
    i2cWrite_HM5065(0xA218, 0xF9);
    i2cWrite_HM5065(0xA219, 0x7B);
    i2cWrite_HM5065(0xA21A, 0x01);
    i2cWrite_HM5065(0xA21B, 0xC0);
    i2cWrite_HM5065(0xA21C, 0x02);
    i2cWrite_HM5065(0xA21D, 0xA3);
    i2cWrite_HM5065(0xA21E, 0xE0);
    i2cWrite_HM5065(0xA21F, 0xFE);
    i2cWrite_HM5065(0xA220, 0xA3);
    i2cWrite_HM5065(0xA221, 0xE0);
    i2cWrite_HM5065(0xA222, 0xAA);
    i2cWrite_HM5065(0xA223, 0x06);
    i2cWrite_HM5065(0xA224, 0xF8);
    i2cWrite_HM5065(0xA225, 0xAC);
    i2cWrite_HM5065(0xA226, 0x02);
    i2cWrite_HM5065(0xA227, 0x7D);
    i2cWrite_HM5065(0xA228, 0x01);
    i2cWrite_HM5065(0xA229, 0xD0);
    i2cWrite_HM5065(0xA22A, 0x02);
    i2cWrite_HM5065(0xA22B, 0x7E);
    i2cWrite_HM5065(0xA22C, 0x00);
    i2cWrite_HM5065(0xA22D, 0x7F);
    i2cWrite_HM5065(0xA22E, 0x04);
    i2cWrite_HM5065(0xA22F, 0x12);
    i2cWrite_HM5065(0xA230, 0x0F);
    i2cWrite_HM5065(0xA231, 0x6F);
    i2cWrite_HM5065(0xA232, 0x02);
    i2cWrite_HM5065(0xA233, 0x66);
    i2cWrite_HM5065(0xA234, 0xD9);
    i2cWrite_HM5065(0xA235, 0x90);
    i2cWrite_HM5065(0xA236, 0x07);
    i2cWrite_HM5065(0xA237, 0xD0);
    i2cWrite_HM5065(0xA238, 0x02);
    i2cWrite_HM5065(0xA239, 0xA2);
    i2cWrite_HM5065(0xA23A, 0x69);
    i2cWrite_HM5065(0xA23B, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA23C, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA23D, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA23E, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA23F, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA240, 0x02);
    i2cWrite_HM5065(0xA241, 0x21);
    i2cWrite_HM5065(0xA242, 0x7F);
    i2cWrite_HM5065(0xA243, 0x02);
    i2cWrite_HM5065(0xA244, 0x21);
    i2cWrite_HM5065(0xA245, 0xF4);
    i2cWrite_HM5065(0xA246, 0x02);
    i2cWrite_HM5065(0xA247, 0xA6);
    i2cWrite_HM5065(0xA248, 0x15);
    i2cWrite_HM5065(0xA249, 0x60);
    i2cWrite_HM5065(0xA24A, 0x0A);
    i2cWrite_HM5065(0xA24B, 0xEF);
    i2cWrite_HM5065(0xA24C, 0xB4);
    i2cWrite_HM5065(0xA24D, 0x01);
    i2cWrite_HM5065(0xA24E, 0x16);
    i2cWrite_HM5065(0xA24F, 0x90);
    i2cWrite_HM5065(0xA250, 0x00);
    i2cWrite_HM5065(0xA251, 0x5D);
    i2cWrite_HM5065(0xA252, 0xE0);
    i2cWrite_HM5065(0xA253, 0x70);
    i2cWrite_HM5065(0xA254, 0x10);
    i2cWrite_HM5065(0xA255, 0x12);
    i2cWrite_HM5065(0xA256, 0x26);
    i2cWrite_HM5065(0xA257, 0xC8);
    i2cWrite_HM5065(0xA258, 0x90);
    i2cWrite_HM5065(0xA259, 0x00);
    i2cWrite_HM5065(0xA25A, 0x11);
    i2cWrite_HM5065(0xA25B, 0x74);
    i2cWrite_HM5065(0xA25C, 0x30);
    i2cWrite_HM5065(0xA25D, 0xF0);
    i2cWrite_HM5065(0xA25E, 0x90);
    i2cWrite_HM5065(0xA25F, 0x00);
    i2cWrite_HM5065(0xA260, 0x10);
    i2cWrite_HM5065(0xA261, 0x74);
    i2cWrite_HM5065(0xA262, 0x01);
    i2cWrite_HM5065(0xA263, 0xF0);
    i2cWrite_HM5065(0xA264, 0x22);
    i2cWrite_HM5065(0xA265, 0x12);
    i2cWrite_HM5065(0xA266, 0x25);
    i2cWrite_HM5065(0xA267, 0xA8);
    i2cWrite_HM5065(0xA268, 0x02);
    i2cWrite_HM5065(0xA269, 0x29);
    i2cWrite_HM5065(0xA26A, 0xFC);
    i2cWrite_HM5065(0xA26B, 0x44);
    i2cWrite_HM5065(0xA26C, 0x18);
    i2cWrite_HM5065(0xA26D, 0xF0);
    i2cWrite_HM5065(0xA26E, 0x90);
    i2cWrite_HM5065(0xA26F, 0x72);
    i2cWrite_HM5065(0xA270, 0x18);
    i2cWrite_HM5065(0xA271, 0xE0);
    i2cWrite_HM5065(0xA272, 0x44);
    i2cWrite_HM5065(0xA273, 0x18);
    i2cWrite_HM5065(0xA274, 0xF0);
    i2cWrite_HM5065(0xA275, 0x00);
    i2cWrite_HM5065(0xA276, 0x00);
    i2cWrite_HM5065(0xA277, 0x00);
    i2cWrite_HM5065(0xA278, 0x00);
    i2cWrite_HM5065(0xA279, 0x00);
    i2cWrite_HM5065(0xA27A, 0x00);
    i2cWrite_HM5065(0xA27B, 0x90);
    i2cWrite_HM5065(0xA27C, 0x72);
    i2cWrite_HM5065(0xA27D, 0x08);
    i2cWrite_HM5065(0xA27E, 0xE0);
    i2cWrite_HM5065(0xA27F, 0x44);
    i2cWrite_HM5065(0xA280, 0x10);
    i2cWrite_HM5065(0xA281, 0xF0);
    i2cWrite_HM5065(0xA282, 0x90);
    i2cWrite_HM5065(0xA283, 0x72);
    i2cWrite_HM5065(0xA284, 0x14);
    i2cWrite_HM5065(0xA285, 0xE0);
    i2cWrite_HM5065(0xA286, 0x54);
    i2cWrite_HM5065(0xA287, 0xFD);
    i2cWrite_HM5065(0xA288, 0xF0);
    i2cWrite_HM5065(0xA289, 0x22);
    i2cWrite_HM5065(0xA28A, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA28B, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA28C, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA28D, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA28E, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA28F, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA290, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA291, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA292, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA293, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA294, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA295, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA296, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA297, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA298, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA299, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA29A, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA29B, 0xF0);
    i2cWrite_HM5065(0xA29C, 0xD3);
    i2cWrite_HM5065(0xA29D, 0x90);
    i2cWrite_HM5065(0xA29E, 0x07);
    i2cWrite_HM5065(0xA29F, 0x91);
    i2cWrite_HM5065(0xA2A0, 0xE0);
    i2cWrite_HM5065(0xA2A1, 0x94);
    i2cWrite_HM5065(0xA2A2, 0x21);
    i2cWrite_HM5065(0xA2A3, 0x90);
    i2cWrite_HM5065(0xA2A4, 0x07);
    i2cWrite_HM5065(0xA2A5, 0x90);
    i2cWrite_HM5065(0xA2A6, 0xE0);
    i2cWrite_HM5065(0xA2A7, 0x64);
    i2cWrite_HM5065(0xA2A8, 0x80);
    i2cWrite_HM5065(0xA2A9, 0x94);
    i2cWrite_HM5065(0xA2AA, 0x81);
    i2cWrite_HM5065(0xA2AB, 0x40);
    i2cWrite_HM5065(0xA2AC, 0x08);
    i2cWrite_HM5065(0xA2AD, 0x90);
    i2cWrite_HM5065(0xA2AE, 0x07);
    i2cWrite_HM5065(0xA2AF, 0xCB);
    i2cWrite_HM5065(0xA2B0, 0x74);
    i2cWrite_HM5065(0xA2B1, 0xFF);
    i2cWrite_HM5065(0xA2B2, 0xF0);
    i2cWrite_HM5065(0xA2B3, 0x80);
    i2cWrite_HM5065(0xA2B4, 0x06);
    i2cWrite_HM5065(0xA2B5, 0x90);
    i2cWrite_HM5065(0xA2B6, 0x07);
    i2cWrite_HM5065(0xA2B7, 0xCB);
    i2cWrite_HM5065(0xA2B8, 0x74);
    i2cWrite_HM5065(0xA2B9, 0x01);
    i2cWrite_HM5065(0xA2BA, 0xF0);
    i2cWrite_HM5065(0xA2BB, 0x02);
    i2cWrite_HM5065(0xA2BC, 0xB5);
    i2cWrite_HM5065(0xA2BD, 0xC3);
    i2cWrite_HM5065(0xA2BE, 0x90);
    i2cWrite_HM5065(0xA2BF, 0x08);
    i2cWrite_HM5065(0xA2C0, 0x34);
    i2cWrite_HM5065(0xA2C1, 0xE0);
    i2cWrite_HM5065(0xA2C2, 0xFC);
    i2cWrite_HM5065(0xA2C3, 0xA3);
    i2cWrite_HM5065(0xA2C4, 0xE0);
    i2cWrite_HM5065(0xA2C5, 0xFD);
    i2cWrite_HM5065(0xA2C6, 0xA3);
    i2cWrite_HM5065(0xA2C7, 0xE0);
    i2cWrite_HM5065(0xA2C8, 0xFE);
    i2cWrite_HM5065(0xA2C9, 0xA3);
    i2cWrite_HM5065(0xA2CA, 0xE0);
    i2cWrite_HM5065(0xA2CB, 0xFF);
    i2cWrite_HM5065(0xA2CC, 0x90);
    i2cWrite_HM5065(0xA2CD, 0x07);
    i2cWrite_HM5065(0xA2CE, 0xD0);
    i2cWrite_HM5065(0xA2CF, 0xE0);
    i2cWrite_HM5065(0xA2D0, 0xF8);
    i2cWrite_HM5065(0xA2D1, 0xA3);
    i2cWrite_HM5065(0xA2D2, 0xE0);
    i2cWrite_HM5065(0xA2D3, 0xF9);
    i2cWrite_HM5065(0xA2D4, 0xA3);
    i2cWrite_HM5065(0xA2D5, 0xE0);
    i2cWrite_HM5065(0xA2D6, 0xFA);
    i2cWrite_HM5065(0xA2D7, 0xA3);
    i2cWrite_HM5065(0xA2D8, 0xE0);
    i2cWrite_HM5065(0xA2D9, 0xFB);
    i2cWrite_HM5065(0xA2DA, 0xD3);
    i2cWrite_HM5065(0xA2DB, 0x12);
    i2cWrite_HM5065(0xA2DC, 0x0D);
    i2cWrite_HM5065(0xA2DD, 0xAE);
    i2cWrite_HM5065(0xA2DE, 0x40);
    i2cWrite_HM5065(0xA2DF, 0x0B);
    i2cWrite_HM5065(0xA2E0, 0x12);
    i2cWrite_HM5065(0xA2E1, 0xB5);
    i2cWrite_HM5065(0xA2E2, 0x49);
    i2cWrite_HM5065(0xA2E3, 0x90);
    i2cWrite_HM5065(0xA2E4, 0x07);
    i2cWrite_HM5065(0xA2E5, 0xA4);
    i2cWrite_HM5065(0xA2E6, 0x74);
    i2cWrite_HM5065(0xA2E7, 0x02);
    i2cWrite_HM5065(0xA2E8, 0xF0);
    i2cWrite_HM5065(0xA2E9, 0x80);
    i2cWrite_HM5065(0xA2EA, 0x09);
    i2cWrite_HM5065(0xA2EB, 0x12);
    i2cWrite_HM5065(0xA2EC, 0xB7);
    i2cWrite_HM5065(0xA2ED, 0x51);
    i2cWrite_HM5065(0xA2EE, 0x90);
    i2cWrite_HM5065(0xA2EF, 0x07);
    i2cWrite_HM5065(0xA2F0, 0xA4);
    i2cWrite_HM5065(0xA2F1, 0x74);
    i2cWrite_HM5065(0xA2F2, 0x05);
    i2cWrite_HM5065(0xA2F3, 0xF0);
    i2cWrite_HM5065(0xA2F4, 0x02);
    i2cWrite_HM5065(0xA2F5, 0xA2);
    i2cWrite_HM5065(0xA2F6, 0xDA);
    i2cWrite_HM5065(0xA2F7, 0x90);
    i2cWrite_HM5065(0xA2F8, 0x0E);
    i2cWrite_HM5065(0xA2F9, 0xE0);
    i2cWrite_HM5065(0xA2FA, 0xE0);
    i2cWrite_HM5065(0xA2FB, 0xFD);
    i2cWrite_HM5065(0xA2FC, 0xA3);
    i2cWrite_HM5065(0xA2FD, 0xE0);
    i2cWrite_HM5065(0xA2FE, 0x90);
    i2cWrite_HM5065(0xA2FF, 0x02);
    i2cWrite_HM5065(0xA300, 0xA2);
    i2cWrite_HM5065(0xA301, 0xCD);
    i2cWrite_HM5065(0xA302, 0xF0);
    i2cWrite_HM5065(0xA303, 0xA3);
    i2cWrite_HM5065(0xA304, 0xED);
    i2cWrite_HM5065(0xA305, 0xF0);
    i2cWrite_HM5065(0xA306, 0x90);
    i2cWrite_HM5065(0xA307, 0x0E);
    i2cWrite_HM5065(0xA308, 0xE2);
    i2cWrite_HM5065(0xA309, 0xE0);
    i2cWrite_HM5065(0xA30A, 0xFD);
    i2cWrite_HM5065(0xA30B, 0xA3);
    i2cWrite_HM5065(0xA30C, 0xE0);
    i2cWrite_HM5065(0xA30D, 0x90);
    i2cWrite_HM5065(0xA30E, 0x02);
    i2cWrite_HM5065(0xA30F, 0xA8);
    i2cWrite_HM5065(0xA310, 0xCD);
    i2cWrite_HM5065(0xA311, 0xF0);
    i2cWrite_HM5065(0xA312, 0xA3);
    i2cWrite_HM5065(0xA313, 0xED);
    i2cWrite_HM5065(0xA314, 0xF0);
    i2cWrite_HM5065(0xA315, 0xE4);
    i2cWrite_HM5065(0xA316, 0x90);
    i2cWrite_HM5065(0xA317, 0x06);
    i2cWrite_HM5065(0xA318, 0x38);
    i2cWrite_HM5065(0xA319, 0xF0);
    i2cWrite_HM5065(0xA31A, 0x02);
    i2cWrite_HM5065(0xA31B, 0x67);
    i2cWrite_HM5065(0xA31C, 0x63);
    i2cWrite_HM5065(0xA31D, 0x90);
    i2cWrite_HM5065(0xA31E, 0x0E);
    i2cWrite_HM5065(0xA31F, 0xE8);
    i2cWrite_HM5065(0xA320, 0xE0);
    i2cWrite_HM5065(0xA321, 0x90);
    i2cWrite_HM5065(0xA322, 0x02);
    i2cWrite_HM5065(0xA323, 0x62);
    i2cWrite_HM5065(0xA324, 0xF0);
    i2cWrite_HM5065(0xA325, 0x90);
    i2cWrite_HM5065(0xA326, 0x0E);
    i2cWrite_HM5065(0xA327, 0xE9);
    i2cWrite_HM5065(0xA328, 0xE0);
    i2cWrite_HM5065(0xA329, 0x90);
    i2cWrite_HM5065(0xA32A, 0x02);
    i2cWrite_HM5065(0xA32B, 0x63);
    i2cWrite_HM5065(0xA32C, 0xF0);
    i2cWrite_HM5065(0xA32D, 0x02);
    i2cWrite_HM5065(0xA32E, 0x67);
    i2cWrite_HM5065(0xA32F, 0x1F);
    i2cWrite_HM5065(0xA330, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA331, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA332, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA333, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA334, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA335, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA336, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA337, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA338, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA339, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA33A, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA33B, 0x90);
    i2cWrite_HM5065(0xA33C, 0x0E);
    i2cWrite_HM5065(0xA33D, 0x14);
    i2cWrite_HM5065(0xA33E, 0xE0);
    i2cWrite_HM5065(0xA33F, 0xFE);
    i2cWrite_HM5065(0xA340, 0xA3);
    i2cWrite_HM5065(0xA341, 0xE0);
    i2cWrite_HM5065(0xA342, 0xFF);
    i2cWrite_HM5065(0xA343, 0x90);
    i2cWrite_HM5065(0xA344, 0x06);
    i2cWrite_HM5065(0xA345, 0xD9);
    i2cWrite_HM5065(0xA346, 0xEE);
    i2cWrite_HM5065(0xA347, 0xF0);
    i2cWrite_HM5065(0xA348, 0xA3);
    i2cWrite_HM5065(0xA349, 0xEF);
    i2cWrite_HM5065(0xA34A, 0xF0);
    i2cWrite_HM5065(0xA34B, 0x90);
    i2cWrite_HM5065(0xA34C, 0x0E);
    i2cWrite_HM5065(0xA34D, 0x18);
    i2cWrite_HM5065(0xA34E, 0xE0);
    i2cWrite_HM5065(0xA34F, 0xFD);
    i2cWrite_HM5065(0xA350, 0x7C);
    i2cWrite_HM5065(0xA351, 0x00);
    i2cWrite_HM5065(0xA352, 0xC3);
    i2cWrite_HM5065(0xA353, 0xEF);
    i2cWrite_HM5065(0xA354, 0x9D);
    i2cWrite_HM5065(0xA355, 0xEE);
    i2cWrite_HM5065(0xA356, 0x9C);
    i2cWrite_HM5065(0xA357, 0x50);
    i2cWrite_HM5065(0xA358, 0x09);
    i2cWrite_HM5065(0xA359, 0xE4);
    i2cWrite_HM5065(0xA35A, 0x90);
    i2cWrite_HM5065(0xA35B, 0x06);
    i2cWrite_HM5065(0xA35C, 0xD7);
    i2cWrite_HM5065(0xA35D, 0xF0);
    i2cWrite_HM5065(0xA35E, 0xA3);
    i2cWrite_HM5065(0xA35F, 0xF0);
    i2cWrite_HM5065(0xA360, 0x80);
    i2cWrite_HM5065(0xA361, 0x13);
    i2cWrite_HM5065(0xA362, 0xC3);
    i2cWrite_HM5065(0xA363, 0x90);
    i2cWrite_HM5065(0xA364, 0x06);
    i2cWrite_HM5065(0xA365, 0xDA);
    i2cWrite_HM5065(0xA366, 0xE0);
    i2cWrite_HM5065(0xA367, 0x9D);
    i2cWrite_HM5065(0xA368, 0xFE);
    i2cWrite_HM5065(0xA369, 0x90);
    i2cWrite_HM5065(0xA36A, 0x06);
    i2cWrite_HM5065(0xA36B, 0xD9);
    i2cWrite_HM5065(0xA36C, 0xE0);
    i2cWrite_HM5065(0xA36D, 0x9C);
    i2cWrite_HM5065(0xA36E, 0x90);
    i2cWrite_HM5065(0xA36F, 0x06);
    i2cWrite_HM5065(0xA370, 0xD7);
    i2cWrite_HM5065(0xA371, 0xF0);
    i2cWrite_HM5065(0xA372, 0xA3);
    i2cWrite_HM5065(0xA373, 0xCE);
    i2cWrite_HM5065(0xA374, 0xF0);
    i2cWrite_HM5065(0xA375, 0x90);
    i2cWrite_HM5065(0xA376, 0x0E);
    i2cWrite_HM5065(0xA377, 0x18);
    i2cWrite_HM5065(0xA378, 0xE0);
    i2cWrite_HM5065(0xA379, 0xF9);
    i2cWrite_HM5065(0xA37A, 0xFF);
    i2cWrite_HM5065(0xA37B, 0x90);
    i2cWrite_HM5065(0xA37C, 0x06);
    i2cWrite_HM5065(0xA37D, 0xC2);
    i2cWrite_HM5065(0xA37E, 0xE0);
    i2cWrite_HM5065(0xA37F, 0xFC);
    i2cWrite_HM5065(0xA380, 0xA3);
    i2cWrite_HM5065(0xA381, 0xE0);
    i2cWrite_HM5065(0xA382, 0xFD);
    i2cWrite_HM5065(0xA383, 0xC3);
    i2cWrite_HM5065(0xA384, 0x9F);
    i2cWrite_HM5065(0xA385, 0xFF);
    i2cWrite_HM5065(0xA386, 0xEC);
    i2cWrite_HM5065(0xA387, 0x94);
    i2cWrite_HM5065(0xA388, 0x00);
    i2cWrite_HM5065(0xA389, 0xFE);
    i2cWrite_HM5065(0xA38A, 0x90);
    i2cWrite_HM5065(0xA38B, 0x0E);
    i2cWrite_HM5065(0xA38C, 0x16);
    i2cWrite_HM5065(0xA38D, 0xE0);
    i2cWrite_HM5065(0xA38E, 0xFA);
    i2cWrite_HM5065(0xA38F, 0xA3);
    i2cWrite_HM5065(0xA390, 0xE0);
    i2cWrite_HM5065(0xA391, 0xFB);
    i2cWrite_HM5065(0xA392, 0xD3);
    i2cWrite_HM5065(0xA393, 0x9F);
    i2cWrite_HM5065(0xA394, 0xEA);
    i2cWrite_HM5065(0xA395, 0x9E);
    i2cWrite_HM5065(0xA396, 0x40);
    i2cWrite_HM5065(0xA397, 0x0A);
    i2cWrite_HM5065(0xA398, 0x90);
    i2cWrite_HM5065(0xA399, 0x06);
    i2cWrite_HM5065(0xA39A, 0xD5);
    i2cWrite_HM5065(0xA39B, 0xEC);
    i2cWrite_HM5065(0xA39C, 0xF0);
    i2cWrite_HM5065(0xA39D, 0xA3);
    i2cWrite_HM5065(0xA39E, 0xED);
    i2cWrite_HM5065(0xA39F, 0xF0);
    i2cWrite_HM5065(0xA3A0, 0x80);
    i2cWrite_HM5065(0xA3A1, 0x0E);
    i2cWrite_HM5065(0xA3A2, 0xE9);
    i2cWrite_HM5065(0xA3A3, 0x7E);
    i2cWrite_HM5065(0xA3A4, 0x00);
    i2cWrite_HM5065(0xA3A5, 0x2B);
    i2cWrite_HM5065(0xA3A6, 0xFF);
    i2cWrite_HM5065(0xA3A7, 0xEE);
    i2cWrite_HM5065(0xA3A8, 0x3A);
    i2cWrite_HM5065(0xA3A9, 0x90);
    i2cWrite_HM5065(0xA3AA, 0x06);
    i2cWrite_HM5065(0xA3AB, 0xD5);
    i2cWrite_HM5065(0xA3AC, 0xF0);
    i2cWrite_HM5065(0xA3AD, 0xA3);
    i2cWrite_HM5065(0xA3AE, 0xEF);
    i2cWrite_HM5065(0xA3AF, 0xF0);
    i2cWrite_HM5065(0xA3B0, 0xE9);
    i2cWrite_HM5065(0xA3B1, 0xFB);
    i2cWrite_HM5065(0xA3B2, 0x7A);
    i2cWrite_HM5065(0xA3B3, 0x00);
    i2cWrite_HM5065(0xA3B4, 0x90);
    i2cWrite_HM5065(0xA3B5, 0x0E);
    i2cWrite_HM5065(0xA3B6, 0x15);
    i2cWrite_HM5065(0xA3B7, 0xE0);
    i2cWrite_HM5065(0xA3B8, 0x2B);
    i2cWrite_HM5065(0xA3B9, 0xFE);
    i2cWrite_HM5065(0xA3BA, 0x90);
    i2cWrite_HM5065(0xA3BB, 0x0E);
    i2cWrite_HM5065(0xA3BC, 0x14);
    i2cWrite_HM5065(0xA3BD, 0xE0);
    i2cWrite_HM5065(0xA3BE, 0x3A);
    i2cWrite_HM5065(0xA3BF, 0x90);
    i2cWrite_HM5065(0xA3C0, 0x06);
    i2cWrite_HM5065(0xA3C1, 0xE1);
    i2cWrite_HM5065(0xA3C2, 0xF0);
    i2cWrite_HM5065(0xA3C3, 0xA3);
    i2cWrite_HM5065(0xA3C4, 0xCE);
    i2cWrite_HM5065(0xA3C5, 0xF0);
    i2cWrite_HM5065(0xA3C6, 0xC3);
    i2cWrite_HM5065(0xA3C7, 0x90);
    i2cWrite_HM5065(0xA3C8, 0x0E);
    i2cWrite_HM5065(0xA3C9, 0x17);
    i2cWrite_HM5065(0xA3CA, 0xE0);
    i2cWrite_HM5065(0xA3CB, 0x9B);
    i2cWrite_HM5065(0xA3CC, 0xFE);
    i2cWrite_HM5065(0xA3CD, 0x90);
    i2cWrite_HM5065(0xA3CE, 0x0E);
    i2cWrite_HM5065(0xA3CF, 0x16);
    i2cWrite_HM5065(0xA3D0, 0x02);
    i2cWrite_HM5065(0xA3D1, 0x20);
    i2cWrite_HM5065(0xA3D2, 0xD5);
    i2cWrite_HM5065(0xA3D3, 0x90);
    i2cWrite_HM5065(0xA3d4, 0x0E);
    i2cWrite_HM5065(0xA3d5, 0xE4);
    i2cWrite_HM5065(0xA3d6, 0xE0);
    i2cWrite_HM5065(0xA3d7, 0x90);
    i2cWrite_HM5065(0xA3d8, 0x02);
    i2cWrite_HM5065(0xA3d9, 0x66);
    i2cWrite_HM5065(0xA3da, 0xF0);
    i2cWrite_HM5065(0xA3DB, 0x90);
    i2cWrite_HM5065(0xA3dc, 0x0E);
    i2cWrite_HM5065(0xA3dd, 0xE5);
    i2cWrite_HM5065(0xA3de, 0xE0);
    i2cWrite_HM5065(0xA3df, 0x90);
    i2cWrite_HM5065(0xA3e0, 0x02);
    i2cWrite_HM5065(0xA3e1, 0x64);
    i2cWrite_HM5065(0xA3e2, 0xF0);
    i2cWrite_HM5065(0xA3e3, 0x90);
    i2cWrite_HM5065(0xA3e4, 0x0E);
    i2cWrite_HM5065(0xA3e5, 0xE6);
    i2cWrite_HM5065(0xA3e6, 0xE0);
    i2cWrite_HM5065(0xA3e7, 0x90);
    i2cWrite_HM5065(0xA3e8, 0x02);
    i2cWrite_HM5065(0xA3e9, 0x65);
    i2cWrite_HM5065(0xA3ea, 0xF0);
    i2cWrite_HM5065(0xA3eb, 0x02);
    i2cWrite_HM5065(0xA3ec, 0x67);
    i2cWrite_HM5065(0xA3ed, 0xA5);
    i2cWrite_HM5065(0xA3EE, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA3EF, 0x00);	// Dummy for Burst Mode
    i2cWrite_HM5065(0xA3f0, 0x12);
    i2cWrite_HM5065(0xA3f1, 0x47);
    i2cWrite_HM5065(0xA3f2, 0x59);
    i2cWrite_HM5065(0xA3f3, 0x90);
    i2cWrite_HM5065(0xA3f4, 0x00);
    i2cWrite_HM5065(0xA3f5, 0xB5);
    i2cWrite_HM5065(0xA3f6, 0xE0);
    i2cWrite_HM5065(0xA3f7, 0xB4);
    i2cWrite_HM5065(0xA3f8, 0x02);
    i2cWrite_HM5065(0xA3f9, 0x03);
    i2cWrite_HM5065(0xA3fa, 0x12);
    i2cWrite_HM5065(0xA3fb, 0x47);
    i2cWrite_HM5065(0xA3fc, 0x59);
    i2cWrite_HM5065(0xA3fd, 0x02);
    i2cWrite_HM5065(0xA3fe, 0xC5);
    i2cWrite_HM5065(0xA3ff, 0xC3);
    i2cWrite_HM5065(0xA400, 0x90);
    i2cWrite_HM5065(0xA401, 0x00);
    i2cWrite_HM5065(0xA402, 0x3D);
    i2cWrite_HM5065(0xA403, 0xF0);
    i2cWrite_HM5065(0xA404, 0x90);
    i2cWrite_HM5065(0xA405, 0x00);
    i2cWrite_HM5065(0xA406, 0x84);
    i2cWrite_HM5065(0xA407, 0xE0);
    i2cWrite_HM5065(0xA408, 0xFE);
    i2cWrite_HM5065(0xA409, 0x90);
    i2cWrite_HM5065(0xA40A, 0x00);
    i2cWrite_HM5065(0xA40B, 0x3E);
    i2cWrite_HM5065(0xA40C, 0xF0);
    i2cWrite_HM5065(0xA40D, 0xEF);
    i2cWrite_HM5065(0xA40E, 0x70);
    i2cWrite_HM5065(0xA40F, 0x03);
    i2cWrite_HM5065(0xA410, 0xEE);
    i2cWrite_HM5065(0xA411, 0x60);
    i2cWrite_HM5065(0xA412, 0x04);
    i2cWrite_HM5065(0xA413, 0x7F);
    i2cWrite_HM5065(0xA414, 0x01);
    i2cWrite_HM5065(0xA415, 0x80);
    i2cWrite_HM5065(0xA416, 0x02);
    i2cWrite_HM5065(0xA417, 0x7F);
    i2cWrite_HM5065(0xA418, 0x00);
    i2cWrite_HM5065(0xA419, 0x90);
    i2cWrite_HM5065(0xA41A, 0x00);
    i2cWrite_HM5065(0xA41B, 0x3F);
    i2cWrite_HM5065(0xA41C, 0xEF);
    i2cWrite_HM5065(0xA41D, 0xF0);
    i2cWrite_HM5065(0xA41E, 0x02);
    i2cWrite_HM5065(0xA41F, 0x89);
    i2cWrite_HM5065(0xA420, 0xD3);
    i2cWrite_HM5065(0xA421, 0x90);
    i2cWrite_HM5065(0xA422, 0x00);
    i2cWrite_HM5065(0xA423, 0x12);
    i2cWrite_HM5065(0xA424, 0xE0);
    i2cWrite_HM5065(0xA425, 0xFF);
    i2cWrite_HM5065(0xA426, 0x70);
    i2cWrite_HM5065(0xA427, 0x0C);
    i2cWrite_HM5065(0xA428, 0x90);
    i2cWrite_HM5065(0xA429, 0x00);
    i2cWrite_HM5065(0xA42A, 0x46);
    i2cWrite_HM5065(0xA42B, 0xE0);
    i2cWrite_HM5065(0xA42C, 0xC3);
    i2cWrite_HM5065(0xA42D, 0x94);
    i2cWrite_HM5065(0xA42E, 0x07);
    i2cWrite_HM5065(0xA42F, 0x40);
    i2cWrite_HM5065(0xA430, 0x03);
    i2cWrite_HM5065(0xA431, 0x75);
    i2cWrite_HM5065(0xA432, 0x2E);
    i2cWrite_HM5065(0xA433, 0x02);
    i2cWrite_HM5065(0xA434, 0xEF);
    i2cWrite_HM5065(0xA435, 0xB4);
    i2cWrite_HM5065(0xA436, 0x01);
    i2cWrite_HM5065(0xA437, 0x0C);
    i2cWrite_HM5065(0xA438, 0x90);
    i2cWrite_HM5065(0xA439, 0x00);
    i2cWrite_HM5065(0xA43A, 0x66);
    i2cWrite_HM5065(0xA43B, 0xE0);
    i2cWrite_HM5065(0xA43C, 0xC3);
    i2cWrite_HM5065(0xA43D, 0x94);
    i2cWrite_HM5065(0xA43E, 0x07);
    i2cWrite_HM5065(0xA43F, 0x40);
    i2cWrite_HM5065(0xA440, 0x03);
    i2cWrite_HM5065(0xA441, 0x75);
    i2cWrite_HM5065(0xA442, 0x2E);
    i2cWrite_HM5065(0xA443, 0x02);
    i2cWrite_HM5065(0xA444, 0x02);
    i2cWrite_HM5065(0xA445, 0xA7);
    i2cWrite_HM5065(0xA446, 0x9E);
    i2cWrite_HM5065(0xA447, 0xC3);
    i2cWrite_HM5065(0xA448, 0x90);
    i2cWrite_HM5065(0xA449, 0x0B);
    i2cWrite_HM5065(0xA44A, 0x8F);
    i2cWrite_HM5065(0xA44B, 0xE0);
    i2cWrite_HM5065(0xA44C, 0x94);
    i2cWrite_HM5065(0xA44D, 0x00);
    i2cWrite_HM5065(0xA44E, 0x90);
    i2cWrite_HM5065(0xA44F, 0x0B);
    i2cWrite_HM5065(0xA450, 0x8E);
    i2cWrite_HM5065(0xA451, 0xE0);
    i2cWrite_HM5065(0xA452, 0x94);
    i2cWrite_HM5065(0xA453, 0x41);
    i2cWrite_HM5065(0xA454, 0x40);
    i2cWrite_HM5065(0xA455, 0x22);
    i2cWrite_HM5065(0xA456, 0x90);
    i2cWrite_HM5065(0xA457, 0x0B);
    i2cWrite_HM5065(0xA458, 0x91);
    i2cWrite_HM5065(0xA459, 0xE0);
    i2cWrite_HM5065(0xA45A, 0x94);
    i2cWrite_HM5065(0xA45B, 0x00);
    i2cWrite_HM5065(0xA45C, 0x90);
    i2cWrite_HM5065(0xA45D, 0x0B);
    i2cWrite_HM5065(0xA45E, 0x90);
    i2cWrite_HM5065(0xA45F, 0xE0);
    i2cWrite_HM5065(0xA460, 0x94);
    i2cWrite_HM5065(0xA461, 0x41);
    i2cWrite_HM5065(0xA462, 0x40);
    i2cWrite_HM5065(0xA463, 0x14);
    i2cWrite_HM5065(0xA464, 0x90);
    i2cWrite_HM5065(0xA465, 0x0B);
    i2cWrite_HM5065(0xA466, 0x93);
    i2cWrite_HM5065(0xA467, 0xE0);
    i2cWrite_HM5065(0xA468, 0x94);
    i2cWrite_HM5065(0xA469, 0x00);
    i2cWrite_HM5065(0xA46A, 0x90);
    i2cWrite_HM5065(0xA46B, 0x0B);
    i2cWrite_HM5065(0xA46C, 0x92);
    i2cWrite_HM5065(0xA46D, 0xE0);
    i2cWrite_HM5065(0xA46E, 0x94);
    i2cWrite_HM5065(0xA46F, 0x41);
    i2cWrite_HM5065(0xA470, 0x40);
    i2cWrite_HM5065(0xA471, 0x06);
    i2cWrite_HM5065(0xA472, 0x90);
    i2cWrite_HM5065(0xA473, 0x01);
    i2cWrite_HM5065(0xA474, 0xA4);
    i2cWrite_HM5065(0xA475, 0x02);
    i2cWrite_HM5065(0xA476, 0x86);
    i2cWrite_HM5065(0xA477, 0x57);
    i2cWrite_HM5065(0xA478, 0x02);
    i2cWrite_HM5065(0xA479, 0x86);
    i2cWrite_HM5065(0xA47A, 0x5C);

    /////////////////////////////////////////
    // A500~A620, 289 byte
    /////////////////////////////////////////
    i2cWrite_HM5065(0xA500, 0xF5);
    i2cWrite_HM5065(0xA501, 0x3B);
    i2cWrite_HM5065(0xA502, 0x90);
    i2cWrite_HM5065(0xA503, 0x06);
    i2cWrite_HM5065(0xA504, 0x6C);
    i2cWrite_HM5065(0xA505, 0xE0);
    i2cWrite_HM5065(0xA506, 0xFF);
    i2cWrite_HM5065(0xA507, 0xE5);
    i2cWrite_HM5065(0xA508, 0x3B);
    i2cWrite_HM5065(0xA509, 0xC3);
    i2cWrite_HM5065(0xA50A, 0x9F);
    i2cWrite_HM5065(0xA50B, 0x40);
    i2cWrite_HM5065(0xA50C, 0x03);
    i2cWrite_HM5065(0xA50D, 0x02);
    i2cWrite_HM5065(0xA50E, 0xF6);
    i2cWrite_HM5065(0xA50F, 0x0E);
    i2cWrite_HM5065(0xA510, 0x90);
    i2cWrite_HM5065(0xA511, 0x0B);
    i2cWrite_HM5065(0xA512, 0xC6);
    i2cWrite_HM5065(0xA513, 0xE0);
    i2cWrite_HM5065(0xA514, 0x14);
    i2cWrite_HM5065(0xA515, 0x60);
    i2cWrite_HM5065(0xA516, 0x3C);
    i2cWrite_HM5065(0xA517, 0x14);
    i2cWrite_HM5065(0xA518, 0x60);
    i2cWrite_HM5065(0xA519, 0x6B);
    i2cWrite_HM5065(0xA51A, 0x24);
    i2cWrite_HM5065(0xA51B, 0x02);
    i2cWrite_HM5065(0xA51C, 0x60);
    i2cWrite_HM5065(0xA51D, 0x03);
    i2cWrite_HM5065(0xA51E, 0x02);
    i2cWrite_HM5065(0xA51F, 0xF5);
    i2cWrite_HM5065(0xA520, 0xB5);
    i2cWrite_HM5065(0xA521, 0x90);
    i2cWrite_HM5065(0xA522, 0x0A);
    i2cWrite_HM5065(0xA523, 0x9A);
    i2cWrite_HM5065(0xA524, 0xE0);
    i2cWrite_HM5065(0xA525, 0xFB);
    i2cWrite_HM5065(0xA526, 0xA3);
    i2cWrite_HM5065(0xA527, 0xE0);
    i2cWrite_HM5065(0xA528, 0xFA);
    i2cWrite_HM5065(0xA529, 0xA3);
    i2cWrite_HM5065(0xA52A, 0xE0);
    i2cWrite_HM5065(0xA52B, 0xF9);
    i2cWrite_HM5065(0xA52C, 0x85);
    i2cWrite_HM5065(0xA52D, 0x3B);
    i2cWrite_HM5065(0xA52E, 0x82);
    i2cWrite_HM5065(0xA52F, 0x75);
    i2cWrite_HM5065(0xA530, 0x83);
    i2cWrite_HM5065(0xA531, 0x00);
    i2cWrite_HM5065(0xA532, 0x12);
    i2cWrite_HM5065(0xA533, 0x0A);
    i2cWrite_HM5065(0xA534, 0xB8);
    i2cWrite_HM5065(0xA535, 0xFF);
    i2cWrite_HM5065(0xA536, 0x74);
    i2cWrite_HM5065(0xA537, 0xAB);
    i2cWrite_HM5065(0xA538, 0x25);
    i2cWrite_HM5065(0xA539, 0x3B);
    i2cWrite_HM5065(0xA53A, 0xF5);
    i2cWrite_HM5065(0xA53B, 0x82);
    i2cWrite_HM5065(0xA53C, 0xE4);
    i2cWrite_HM5065(0xA53D, 0x34);
    i2cWrite_HM5065(0xA53E, 0x0A);
    i2cWrite_HM5065(0xA53F, 0xF5);
    i2cWrite_HM5065(0xA540, 0x83);
    i2cWrite_HM5065(0xA541, 0xE0);
    i2cWrite_HM5065(0xA542, 0xFD);
    i2cWrite_HM5065(0xA543, 0xC3);
    i2cWrite_HM5065(0xA544, 0xEF);
    i2cWrite_HM5065(0xA545, 0x9D);
    i2cWrite_HM5065(0xA546, 0xFE);
    i2cWrite_HM5065(0xA547, 0xE4);
    i2cWrite_HM5065(0xA548, 0x94);
    i2cWrite_HM5065(0xA549, 0x00);
    i2cWrite_HM5065(0xA54A, 0x90);
    i2cWrite_HM5065(0xA54B, 0x0B);
    i2cWrite_HM5065(0xA54C, 0xCA);
    i2cWrite_HM5065(0xA54D, 0xF0);
    i2cWrite_HM5065(0xA54E, 0xA3);
    i2cWrite_HM5065(0xA54F, 0xCE);
    i2cWrite_HM5065(0xA550, 0xF0);
    i2cWrite_HM5065(0xA551, 0x80);
    i2cWrite_HM5065(0xA552, 0x62);
    i2cWrite_HM5065(0xA553, 0x90);
    i2cWrite_HM5065(0xA554, 0x0A);
    i2cWrite_HM5065(0xA555, 0x9A);
    i2cWrite_HM5065(0xA556, 0xE0);
    i2cWrite_HM5065(0xA557, 0xFB);
    i2cWrite_HM5065(0xA558, 0xA3);
    i2cWrite_HM5065(0xA559, 0xE0);
    i2cWrite_HM5065(0xA55A, 0xFA);
    i2cWrite_HM5065(0xA55B, 0xA3);
    i2cWrite_HM5065(0xA55C, 0xE0);
    i2cWrite_HM5065(0xA55D, 0xF9);
    i2cWrite_HM5065(0xA55E, 0x85);
    i2cWrite_HM5065(0xA55F, 0x3B);
    i2cWrite_HM5065(0xA560, 0x82);
    i2cWrite_HM5065(0xA561, 0x75);
    i2cWrite_HM5065(0xA562, 0x83);
    i2cWrite_HM5065(0xA563, 0x00);
    i2cWrite_HM5065(0xA564, 0x12);
    i2cWrite_HM5065(0xA565, 0x0A);
    i2cWrite_HM5065(0xA566, 0xB8);
    i2cWrite_HM5065(0xA567, 0xFF);
    i2cWrite_HM5065(0xA568, 0x74);
    i2cWrite_HM5065(0xA569, 0x9D);
    i2cWrite_HM5065(0xA56A, 0x25);
    i2cWrite_HM5065(0xA56B, 0x3B);
    i2cWrite_HM5065(0xA56C, 0xF5);
    i2cWrite_HM5065(0xA56D, 0x82);
    i2cWrite_HM5065(0xA56E, 0xE4);
    i2cWrite_HM5065(0xA56F, 0x34);
    i2cWrite_HM5065(0xA570, 0x0A);
    i2cWrite_HM5065(0xA571, 0xF5);
    i2cWrite_HM5065(0xA572, 0x83);
    i2cWrite_HM5065(0xA573, 0xE0);
    i2cWrite_HM5065(0xA574, 0xFD);
    i2cWrite_HM5065(0xA575, 0xC3);
    i2cWrite_HM5065(0xA576, 0xEF);
    i2cWrite_HM5065(0xA577, 0x9D);
    i2cWrite_HM5065(0xA578, 0xFE);
    i2cWrite_HM5065(0xA579, 0xE4);
    i2cWrite_HM5065(0xA57A, 0x94);
    i2cWrite_HM5065(0xA57B, 0x00);
    i2cWrite_HM5065(0xA57C, 0x90);
    i2cWrite_HM5065(0xA57D, 0x0B);
    i2cWrite_HM5065(0xA57E, 0xCA);
    i2cWrite_HM5065(0xA57F, 0xF0);
    i2cWrite_HM5065(0xA580, 0xA3);
    i2cWrite_HM5065(0xA581, 0xCE);
    i2cWrite_HM5065(0xA582, 0xF0);
    i2cWrite_HM5065(0xA583, 0x80);
    i2cWrite_HM5065(0xA584, 0x30);
    i2cWrite_HM5065(0xA585, 0x90);
    i2cWrite_HM5065(0xA586, 0x0A);
    i2cWrite_HM5065(0xA587, 0x9A);
    i2cWrite_HM5065(0xA588, 0xE0);
    i2cWrite_HM5065(0xA589, 0xFB);
    i2cWrite_HM5065(0xA58A, 0xA3);
    i2cWrite_HM5065(0xA58B, 0xE0);
    i2cWrite_HM5065(0xA58C, 0xFA);
    i2cWrite_HM5065(0xA58D, 0xA3);
    i2cWrite_HM5065(0xA58E, 0xE0);
    i2cWrite_HM5065(0xA58F, 0xF9);
    i2cWrite_HM5065(0xA590, 0x85);
    i2cWrite_HM5065(0xA591, 0x3B);
    i2cWrite_HM5065(0xA592, 0x82);
    i2cWrite_HM5065(0xA593, 0x75);
    i2cWrite_HM5065(0xA594, 0x83);
    i2cWrite_HM5065(0xA595, 0x00);
    i2cWrite_HM5065(0xA596, 0x12);
    i2cWrite_HM5065(0xA597, 0x0A);
    i2cWrite_HM5065(0xA598, 0xB8);
    i2cWrite_HM5065(0xA599, 0xFF);
    i2cWrite_HM5065(0xA59A, 0x74);
    i2cWrite_HM5065(0xA59B, 0xA4);
    i2cWrite_HM5065(0xA59C, 0x25);
    i2cWrite_HM5065(0xA59D, 0x3B);
    i2cWrite_HM5065(0xA59E, 0xF5);
    i2cWrite_HM5065(0xA59F, 0x82);
    i2cWrite_HM5065(0xA5A0, 0xE4);
    i2cWrite_HM5065(0xA5A1, 0x34);
    i2cWrite_HM5065(0xA5A2, 0x0A);
    i2cWrite_HM5065(0xA5A3, 0xF5);
    i2cWrite_HM5065(0xA5A4, 0x83);
    i2cWrite_HM5065(0xA5A5, 0xE0);
    i2cWrite_HM5065(0xA5A6, 0xFD);
    i2cWrite_HM5065(0xA5A7, 0xC3);
    i2cWrite_HM5065(0xA5A8, 0xEF);
    i2cWrite_HM5065(0xA5A9, 0x9D);
    i2cWrite_HM5065(0xA5AA, 0xFE);
    i2cWrite_HM5065(0xA5AB, 0xE4);
    i2cWrite_HM5065(0xA5AC, 0x94);
    i2cWrite_HM5065(0xA5AD, 0x00);
    i2cWrite_HM5065(0xA5AE, 0x90);
    i2cWrite_HM5065(0xA5AF, 0x0B);
    i2cWrite_HM5065(0xA5B0, 0xCA);
    i2cWrite_HM5065(0xA5B1, 0xF0);
    i2cWrite_HM5065(0xA5B2, 0xA3);
    i2cWrite_HM5065(0xA5B3, 0xCE);
    i2cWrite_HM5065(0xA5B4, 0xF0);
    i2cWrite_HM5065(0xA5B5, 0x90);
    i2cWrite_HM5065(0xA5B6, 0x07);
    i2cWrite_HM5065(0xA5B7, 0x83);
    i2cWrite_HM5065(0xA5B8, 0xE0);
    i2cWrite_HM5065(0xA5B9, 0xFF);
    i2cWrite_HM5065(0xA5BA, 0x7E);
    i2cWrite_HM5065(0xA5BB, 0x00);
    i2cWrite_HM5065(0xA5BC, 0x90);
    i2cWrite_HM5065(0xA5BD, 0x0D);
    i2cWrite_HM5065(0xA5BE, 0xF6);
    i2cWrite_HM5065(0xA5BF, 0xEE);
    i2cWrite_HM5065(0xA5C0, 0xF0);
    i2cWrite_HM5065(0xA5C1, 0xA3);
    i2cWrite_HM5065(0xA5C2, 0xEF);
    i2cWrite_HM5065(0xA5C3, 0xF0);
    i2cWrite_HM5065(0xA5C4, 0x90);
    i2cWrite_HM5065(0xA5C5, 0x0B);
    i2cWrite_HM5065(0xA5C6, 0xCA);
    i2cWrite_HM5065(0xA5C7, 0xE0);
    i2cWrite_HM5065(0xA5C8, 0xFC);
    i2cWrite_HM5065(0xA5C9, 0xA3);
    i2cWrite_HM5065(0xA5CA, 0xE0);
    i2cWrite_HM5065(0xA5CB, 0xFD);
    i2cWrite_HM5065(0xA5CC, 0xD3);
    i2cWrite_HM5065(0xA5CD, 0x9F);
    i2cWrite_HM5065(0xA5CE, 0x74);
    i2cWrite_HM5065(0xA5CF, 0x80);
    i2cWrite_HM5065(0xA5D0, 0xF8);
    i2cWrite_HM5065(0xA5D1, 0xEC);
    i2cWrite_HM5065(0xA5D2, 0x64);
    i2cWrite_HM5065(0xA5D3, 0x80);
    i2cWrite_HM5065(0xA5D4, 0x98);
    i2cWrite_HM5065(0xA5D5, 0x40);
    i2cWrite_HM5065(0xA5D6, 0x0C);
    i2cWrite_HM5065(0xA5D7, 0x90);
    i2cWrite_HM5065(0xA5D8, 0x0B);
    i2cWrite_HM5065(0xA5D9, 0xC8);
    i2cWrite_HM5065(0xA5DA, 0xE0);
    i2cWrite_HM5065(0xA5DB, 0x04);
    i2cWrite_HM5065(0xA5DC, 0xF0);
    i2cWrite_HM5065(0xA5DD, 0xA3);
    i2cWrite_HM5065(0xA5DE, 0xE0);
    i2cWrite_HM5065(0xA5DF, 0x04);
    i2cWrite_HM5065(0xA5E0, 0xF0);
    i2cWrite_HM5065(0xA5E1, 0x80);
    i2cWrite_HM5065(0xA5E2, 0x26);
    i2cWrite_HM5065(0xA5E3, 0x90);
    i2cWrite_HM5065(0xA5E4, 0x0D);
    i2cWrite_HM5065(0xA5E5, 0xF6);
    i2cWrite_HM5065(0xA5E6, 0xE0);
    i2cWrite_HM5065(0xA5E7, 0xFE);
    i2cWrite_HM5065(0xA5E8, 0xA3);
    i2cWrite_HM5065(0xA5E9, 0xE0);
    i2cWrite_HM5065(0xA5EA, 0xFF);
    i2cWrite_HM5065(0xA5EB, 0xC3);
    i2cWrite_HM5065(0xA5EC, 0xE4);
    i2cWrite_HM5065(0xA5ED, 0x9F);
    i2cWrite_HM5065(0xA5EE, 0xFF);
    i2cWrite_HM5065(0xA5EF, 0xE4);
    i2cWrite_HM5065(0xA5F0, 0x9E);
    i2cWrite_HM5065(0xA5F1, 0xFE);
    i2cWrite_HM5065(0xA5F2, 0xC3);
    i2cWrite_HM5065(0xA5F3, 0xED);
    i2cWrite_HM5065(0xA5F4, 0x9F);
    i2cWrite_HM5065(0xA5F5, 0xEE);
    i2cWrite_HM5065(0xA5F6, 0x64);
    i2cWrite_HM5065(0xA5F7, 0x80);
    i2cWrite_HM5065(0xA5F8, 0xF8);
    i2cWrite_HM5065(0xA5F9, 0xEC);
    i2cWrite_HM5065(0xA5FA, 0x64);
    i2cWrite_HM5065(0xA5FB, 0x80);
    i2cWrite_HM5065(0xA5FC, 0x98);
    i2cWrite_HM5065(0xA5FD, 0x50);
    i2cWrite_HM5065(0xA5FE, 0x0A);
    i2cWrite_HM5065(0xA5FF, 0x90);
    i2cWrite_HM5065(0xA600, 0x0B);
    i2cWrite_HM5065(0xA601, 0xC8);
    i2cWrite_HM5065(0xA602, 0xE0);
    i2cWrite_HM5065(0xA603, 0x14);
    i2cWrite_HM5065(0xA604, 0xF0);
    i2cWrite_HM5065(0xA605, 0xA3);
    i2cWrite_HM5065(0xA606, 0xE0);
    i2cWrite_HM5065(0xA607, 0x04);
    i2cWrite_HM5065(0xA608, 0xF0);
    i2cWrite_HM5065(0xA609, 0x05);
    i2cWrite_HM5065(0xA60A, 0x3B);
    i2cWrite_HM5065(0xA60B, 0x02);
    i2cWrite_HM5065(0xA60C, 0xF5);
    i2cWrite_HM5065(0xA60D, 0x02);
    i2cWrite_HM5065(0xA60E, 0x90);
    i2cWrite_HM5065(0xA60F, 0x08);
    i2cWrite_HM5065(0xA610, 0x58);
    i2cWrite_HM5065(0xA611, 0x02);
    i2cWrite_HM5065(0xA612, 0x9D);
    i2cWrite_HM5065(0xA613, 0x50);
    i2cWrite_HM5065(0xA614, 0x90);
    i2cWrite_HM5065(0xA615, 0x00);
    i2cWrite_HM5065(0xA616, 0x3E);
    i2cWrite_HM5065(0xA617, 0xE0);
    i2cWrite_HM5065(0xA618, 0xFF);
    i2cWrite_HM5065(0xA619, 0x90);
    i2cWrite_HM5065(0xA61A, 0x00);
    i2cWrite_HM5065(0xA61B, 0x3D);
    i2cWrite_HM5065(0xA61C, 0xE0);
    i2cWrite_HM5065(0xA61D, 0x6F);
    i2cWrite_HM5065(0xA61E, 0x02);
    i2cWrite_HM5065(0xA61F, 0x8C);
    i2cWrite_HM5065(0xA620, 0x0D);
    
    ///////////////////////////////////////////////////////////////////////////////
    // Patch Entries
    ///////////////////////////////////////////////////////////////////////////////
    // Breakpoint table entry 0     Contrast
    i2cWrite_HM5065(0x9006, 0xBA);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9007, 0x75);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9008, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9009, 0x00);	// Offset Low byte//
    i2cWrite_HM5065(0x900A, 0x02);	// Enable BP 0//
    
    // Breakpoint table entry 1     AF p74_04  part 1 
    i2cWrite_HM5065(0x900D, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x900E, 0xA2);	// Patch break point address high byte//
    i2cWrite_HM5065(0x900F, 0x8F);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9010, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9011, 0xCB);	// Offset Low byte//
    i2cWrite_HM5065(0x9012, 0x03);	// Enable BP 1//
    
    // Breakpoint table entry 2     CSI ULP enter
    i2cWrite_HM5065(0x9016, 0xE6);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9017, 0x6B);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9018, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9019, 0x6B);	// Offset Low byte//
    i2cWrite_HM5065(0x901A, 0x02);	// Enable BP 2//
    
    // Breakpoint table entry 3      AF p74_03 udwFocusMeasure -> udwPrevFocusMeasureAF                       
    i2cWrite_HM5065(0x901D, 0x01);    //	
    i2cWrite_HM5065(0x901E, 0xAC);	// Patch break point address high byte//
    i2cWrite_HM5065(0x901F, 0x70);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9020, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9021, 0xC5);	// Offset Low byte//
    i2cWrite_HM5065(0x9022, 0x03);	// Enable BP 3//
    
    // Breakpoint table entry 4     AV
    i2cWrite_HM5065(0x9026, 0x9C);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9027, 0x5B);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9028, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9029, 0xBF);	// Offset Low byte//
    i2cWrite_HM5065(0x902A, 0x02);	// Enable BP 4//
    
    // Breakpoint table entry 5     AV OTP
    i2cWrite_HM5065(0x902E, 0x60);	// Patch break point address high byte//
    i2cWrite_HM5065(0x902F, 0x1C);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9030, 0x01);	// Offset High byte//
    i2cWrite_HM5065(0x9031, 0x37);	// Offset Low byte//
    i2cWrite_HM5065(0x9032, 0x02);	// Enable BP 3//
    
    // Breakpoint table entry 6     AF stat
    i2cWrite_HM5065(0x9035, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x9036, 0xBA);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9037, 0x70);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9038, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9039, 0x00);	// Offset Low byte//
    i2cWrite_HM5065(0x903A, 0x03);	// Enable BP 6//
    
    // Breakpoint table entry 7     AF Init  part 1
    i2cWrite_HM5065(0x903E, 0x21);	// Patch break point address high byte//
    i2cWrite_HM5065(0x903F, 0x3F);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9040, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9041, 0x40);	// Offset Low byte//
    i2cWrite_HM5065(0x9042, 0x02);	// Enable BP 7//
    
    // Breakpoint table entry 8     AF Init part 2
    i2cWrite_HM5065(0x9046, 0x21);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9047, 0xEA);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9048, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9049, 0x43);	// Offset Low byte//
    i2cWrite_HM5065(0x904A, 0x02);	// Enable BP 8//
    
    // Breakpoint table entry 9     Stop/Start  part 1
    i2cWrite_HM5065(0x904E, 0xA6);	// Patch break point address high byte//
    i2cWrite_HM5065(0x904F, 0x12);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9050, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9051, 0x46);	// Offset Low byte//
    i2cWrite_HM5065(0x9052, 0x02);	// Enable BP 9//
    
    // Breakpoint table entry 10     Stop/Start part 2
    i2cWrite_HM5065(0x9056, 0x29);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9057, 0xE3);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9058, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9059, 0x49);	// Offset Low byte//
    i2cWrite_HM5065(0x905A, 0x02);	// Enable BP 10//
    
    // Breakpoint table entry 11     AF light 
    i2cWrite_HM5065(0x905D, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x905E, 0x9C);	// Patch break point address high byte//
    i2cWrite_HM5065(0x905F, 0x6E);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9060, 0x05);	// Offset High byte//
    i2cWrite_HM5065(0x9061, 0x00);	// Offset Low byte//
    i2cWrite_HM5065(0x9062, 0x02);	// Enable BP 11//
    
    // Breakpoint table entry 12     //   AF p74_04  part 2
    i2cWrite_HM5065(0x9065, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x9066, 0xA2);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9067, 0x66);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9068, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9069, 0x35);	// Offset Low byte//
    i2cWrite_HM5065(0x906A, 0x02);	// Enable BP 12//
    
    // Breakpoint table entry 13     AF p74-05  
    i2cWrite_HM5065(0x906D, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x906E, 0xB5);	// Patch break point address high byte//
    i2cWrite_HM5065(0x906F, 0xC2);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9070, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9071, 0x9B);	// Offset Low byte//
    i2cWrite_HM5065(0x9072, 0x02);	// Enable BP 13//
    
    // Breakpoint table entry 14     //   AF p74_06  
    i2cWrite_HM5065(0x9075, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x9076, 0xA2);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9077, 0xD4);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9078, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9079, 0xBE);	// Offset Low byte//
    i2cWrite_HM5065(0x907A, 0x02);	// Enable BP 14//
    
    // Breakpoint table entry 15     bAF_NextState = AF_STATE_COARSE  
    i2cWrite_HM5065(0x907D, 0x01);	// Patch break point address bank//
    i2cWrite_HM5065(0x907E, 0xB7);	// Patch break point address high byte//
    i2cWrite_HM5065(0x907F, 0xEA);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9080, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x9081, 0x02);	// Offset Low byte//
    i2cWrite_HM5065(0x9082, 0x03);	// Enable BP 15//
    
    // Breakpoint table entry 16     //   OTP WB  
    i2cWrite_HM5065(0x9086, 0x67);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9087, 0x31);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9088, 0x02);	// Offset High byte//
    i2cWrite_HM5065(0x9089, 0xF7);	// Offset Low byte//
    i2cWrite_HM5065(0x908A, 0x02);	// Enable BP 16//
    
    // Breakpoint table entry 17      OTP darkcal   
    i2cWrite_HM5065(0x908E, 0x66);	// Patch break point address high byte//
    i2cWrite_HM5065(0x908F, 0xED);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9090, 0x03);	// Offset High byte//
    i2cWrite_HM5065(0x9091, 0x1D);	// Offset Low byte//
    i2cWrite_HM5065(0x9092, 0x02);	// Enable BP 17//
    
    // Breakpoint table entry 18     //   OTP HFPN  
    i2cWrite_HM5065(0x9096, 0x67);	// Patch break point address high byte//
    i2cWrite_HM5065(0x9097, 0x73);	// Patch break point address low byte//
    i2cWrite_HM5065(0x9098, 0x03);	// Offset High byte//
    i2cWrite_HM5065(0x9099, 0xD3);	// Offset Low byte//
    i2cWrite_HM5065(0x909A, 0x02);	// Enable BP 18//
    
    // Breakpoint table entry 19      OTP VCM   
    i2cWrite_HM5065(0x909E, 0x20);	// Patch break point address high byte//
    i2cWrite_HM5065(0x909F, 0x40);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90A0, 0x03);	// Offset High byte//
    i2cWrite_HM5065(0x90A1, 0x3B);	// Offset Low byte//
    i2cWrite_HM5065(0x90A2, 0x02);	// Enable BP 19//
    
    // Breakpoint table entry 20     //   TWakeup work around for PLL3 div   
    i2cWrite_HM5065(0x90A6, 0xC5);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90A7, 0xC0);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90A8, 0x03);	// Offset High byte//
    i2cWrite_HM5065(0x90A9, 0xF0);	// Offset Low byte//
    i2cWrite_HM5065(0x90AA, 0x02);	// Enable BP 20//
    
    // Breakpoint table entry 21   // reset the pipe at stop stage
    i2cWrite_HM5065(0x90AE, 0x41);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90AF, 0xB3);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90B0, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x90B1, 0xA2);	// Offset Low byte//
    i2cWrite_HM5065(0x90B2, 0x02);	// Enable BP 21//
    
    // Breakpoint table entry 22     //   CSI Tinit   
    i2cWrite_HM5065(0x90B6, 0x44);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90B7, 0xBA);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90B8, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x90B9, 0xF0);	// Offset Low byte//
    i2cWrite_HM5065(0x90BA, 0x03);	// Enable BP 22//
    
    // Breakpoint table entry 23      AV2x2    
    i2cWrite_HM5065(0x90BE, 0x89);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90BF, 0x99);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90C0, 0x04);	// Offset High byte//
    i2cWrite_HM5065(0x90C1, 0x00);	// Offset Low byte//
    i2cWrite_HM5065(0x90C2, 0x02);	// Enable BP 23//
    
    // Breakpoint table entry 24     //   DL JPEG CSI clk /2   
    i2cWrite_HM5065(0x90C6, 0xA7);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90C7, 0x91);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90C8, 0x04);	// Offset High byte//
    i2cWrite_HM5065(0x90C9, 0x21);	// Offset Low byte//
    i2cWrite_HM5065(0x90CA, 0x02);	// Enable BP 24//
    
    // Breakpoint table entry 25      pipe reset at pause stage    
    i2cWrite_HM5065(0x90CE, 0x3A);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90CF, 0x51);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90D0, 0x00);	// Offset High byte//
    i2cWrite_HM5065(0x90D1, 0xA2);	// Offset Low byte//
    i2cWrite_HM5065(0x90D2, 0x02);	// Enable BP 25//
    
    // Breakpoint table entry 26     Freeze WB at low stat
    i2cWrite_HM5065(0x90D6, 0x86);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90D7, 0x54);	// Patch break point address low byte// 
    i2cWrite_HM5065(0x90D8, 0x04);	// Offset High byte//                   
    i2cWrite_HM5065(0x90D9, 0x47);	// Offset Low byte//                    
    i2cWrite_HM5065(0x90DA, 0x02);	// Enable BP 26//                       
    
    // Breakpoint table entry 27        AV2x2 XY flip
    i2cWrite_HM5065(0x90DE, 0x8C);	// Patch break point address high byte//
    i2cWrite_HM5065(0x90DF, 0x09);	// Patch break point address low byte//
    i2cWrite_HM5065(0x90E0, 0x06);	// Offset High byte//
    i2cWrite_HM5065(0x90E1, 0x14);	// Offset Low byte//
    i2cWrite_HM5065(0x90E2, 0x02);	// Enable BP 27//
    
    i2cWrite_HM5065(0x9000, 0x01);	// Enable patch//
    i2cWrite_HM5065(0xffff, 0x00);	// MCU release//
    
    OSTimeDly(4);
    
    //////////////////////////////////////////
    
    //i2cWrite_HM5065(0x0009, 0x10);  // MCLK 12 MHz
    i2cWrite_HM5065(0x0009, 0x16);  // MCLK 24 MHz
    i2cWrite_HM5065(0x0012, 0x00);
    i2cWrite_HM5065(0x0013, 0x00);
    i2cWrite_HM5065(0x0016, 0x00);
    i2cWrite_HM5065(0x0021, 0x00);
    i2cWrite_HM5065(0x0022, 0x01);
    
    i2cWrite_HM5065(0x0040, 0x01); 	// AB2
    i2cWrite_HM5065(0x0041, 0x0a); 	// Image Size Manual
    i2cWrite_HM5065(0x0042, 0x05); 	// 1280
    i2cWrite_HM5065(0x0043, 0x00); 
    i2cWrite_HM5065(0x0044, 0x03); 	// 960
    i2cWrite_HM5065(0x0045, 0xC0); 
    i2cWrite_HM5065(0x0046, 0x02); 	// DataFormat_YCbCr_Custom
    
    i2cWrite_HM5065(0x0060, 0x00);
    i2cWrite_HM5065(0x0061, 0x00);
    i2cWrite_HM5065(0x0066, 0x02);
    
    i2cWrite_HM5065(0x0083, 0x01);        // Horizontal Mirror Enable
    i2cWrite_HM5065(0x0084, 0x01);        // Vertical Flip Enable
    i2cWrite_HM5065(0x0085, 0x03);	// YCbYCr Order
    
    i2cWrite_HM5065(0x00B2, 0x50);        // set PLL output 713MHz
    i2cWrite_HM5065(0x00B3, 0xC9);
    i2cWrite_HM5065(0x00B4, 0x01);        // E_div
    i2cWrite_HM5065(0x00B5, 0x02);        // PLL3_div 2
    
    i2cWrite_HM5065(0x00E8, 0x01);
    i2cWrite_HM5065(0x00ED, 0x05);	// Min Framerate
    i2cWrite_HM5065(0x00EE, 0x1E);	// Max Framerate
    
    i2cWrite_HM5065(0x0129, 0x00);
    i2cWrite_HM5065(0x0130, 0x00);
    
    i2cWrite_HM5065(0x019C, 0x4B);
    i2cWrite_HM5065(0x019D, 0xC0);
    
    i2cWrite_HM5065(0x01A0, 0x01);
    i2cWrite_HM5065(0x01A1, 0x80);
    i2cWrite_HM5065(0x01A2, 0x80);
    i2cWrite_HM5065(0x01A3, 0x80);
    
    i2cWrite_HM5065(0x5200, 0x01);
                          
    i2cWrite_HM5065(0x7000, 0x0C);
                          
    i2cWrite_HM5065(0x7101, 0xC4);
    i2cWrite_HM5065(0x7102, 0x01);
    i2cWrite_HM5065(0x7103, 0x00);
    i2cWrite_HM5065(0x7104, 0x02);	// OIF threshold = 512
    i2cWrite_HM5065(0x7105, 0x00);	//
    i2cWrite_HM5065(0x7158, 0x00);
                          
    //**********************************************
    // System Max Integration Time limits
    //**********************************************
                          
    // Set UserMaximumIntegratonTime_us = 133376
    i2cWrite_HM5065(0x0143, 0x60);
    i2cWrite_HM5065(0x0144, 0x09);
                          
    //**********************************************
    // System Gain limits 
    //**********************************************
                          
    // Set SensorAnalogGainCeiling = 8.0
    i2cWrite_HM5065(0x02C2, 0x00);
    i2cWrite_HM5065(0x02C3, 0xE0);
                          
    // Set DigitalGainCeiling = 2.0
    i2cWrite_HM5065(0x015E, 0x40);
    i2cWrite_HM5065(0x015F, 0x00);
                          
                          
    //**********************************************
    // Defects correction and noise reduction
    //**********************************************
                          
    // Arctic controls    
    i2cWrite_HM5065(0x0390, 0x01);	// ArcticControl fArcticEnable
    i2cWrite_HM5065(0x0391, 0x00);	// ArcticControl fArcticConfig DEFAULT CONFIG
    i2cWrite_HM5065(0x0392, 0x00);	// ArcticControl fGNFConfig    DEFAULT CONFIG
                          
    // SetArcticCCSigmaControl tuned settings
    // Value LL = 4 (3444736us)
    // Value HL = 20 (24000us)
    i2cWrite_HM5065(0x03A0, 0x14);	// ArcticCCSigmaControl fMaximumCCSigma 
    i2cWrite_HM5065(0x03A1, 0x00);	// ArcticCCSigmaControl fDisablePromotion {CompiledExposureTime}
    i2cWrite_HM5065(0x03A2, 0x5A);	// ArcticCCSigmaControl fDamperLowThreshold {MSB}   //2400
    i2cWrite_HM5065(0x03A3, 0xEE);	// ArcticCCSigmaControl fDamperLowThreshold {LSB}   
    i2cWrite_HM5065(0x03A4, 0x69);	// ArcticCCSigmaControl fDamperHighThreshold {MSB}   //3444736
    i2cWrite_HM5065(0x03A5, 0x49);	// ArcticCCSigmaControl fDamperHighThreshold {LSB}
    i2cWrite_HM5065(0x03A6, 0x3E);	// ArcticCCSigmaControl fY1 {MSB}  // Low threshold
    i2cWrite_HM5065(0x03A7, 0x00);	// ArcticCCSigmaControl fY1 {LSB} 
    i2cWrite_HM5065(0x03A8, 0x39);	// ArcticCCSigmaControl fY2 {MSB}  // High threshold
    i2cWrite_HM5065(0x03A9, 0x33);	// ArcticCCSigmaControl fY2 {LSB} 
                          
    // SetArcticRingControl  tuned settings
    // Value LL = 75 (3444736us)
    // Value HL = 96 (24000us)
    i2cWrite_HM5065(0x03B0, 0x60);	// ArcticCCSigmaControl fMaximumRing 
    i2cWrite_HM5065(0x03B1, 0x00);	// ArcticCCSigmaControl fDisablePromotion {CompiledExposureTime}
    i2cWrite_HM5065(0x03B2, 0x5A);	// ArcticCCSigmaControl fDamperLowThreshold {MSB}    //24000
    i2cWrite_HM5065(0x03B3, 0xEE);	// ArcticCCSigmaControl fDamperLowThreshold {LSB}
    i2cWrite_HM5065(0x03B4, 0x69);	// ArcticCCSigmaControl DamperHighThreshold {MSB}    //3444736
    i2cWrite_HM5065(0x03B5, 0x49);	// ArcticCCSigmaControl DamperHighThreshold {LSB}
    i2cWrite_HM5065(0x03B6, 0x3E);	// ArcticCCSigmaControl fY1 {MSB}  //Low threshold
    i2cWrite_HM5065(0x03B7, 0x00);	// ArcticCCSigmaControl fY1 {LSB} 
    i2cWrite_HM5065(0x03B8, 0x3D);	// ArcticCCSigmaControl fY2 {MSB}  //High threshold
    i2cWrite_HM5065(0x03B9, 0x20);	// ArcticCCSigmaControl fY2 {LSB}  
                          
    // SetArcticScoringControl  default settings
    // Value LL = 16 (3444736us)
    // Value HL = 5 (24000us)
    i2cWrite_HM5065(0x03C0, 0x10);	// ArcticCCSigmaControl fMaximumScoring 
    i2cWrite_HM5065(0x03C1, 0x00);	// ArcticCCSigmaControl fDisablePromotion {CompiledExposureTime}
    i2cWrite_HM5065(0x03C2, 0x5A);	// ArcticCCSigmaControl fDamperLowThreshold {MSB}    //24000
    i2cWrite_HM5065(0x03C3, 0xEE);	// ArcticCCSigmaControl fDamperLowThreshold {LSB}
    i2cWrite_HM5065(0x03C4, 0x69);	// ArcticCCSigmaControl DamperHighThreshold {MSB}    //3444736
    i2cWrite_HM5065(0x03C5, 0x49);	// ArcticCCSigmaControl DamperHighThreshold {LSB}
    i2cWrite_HM5065(0x03C6, 0x3A);	// ArcticCCSigmaControl fMinimumDamperOutput {MSB}
    i2cWrite_HM5065(0x03C7, 0x80);	// ArcticCCSigmaControl fMinimumDamperOutput {LSB} 
                          
    // SetArcticGNFTh1Control  default settings
    // Value HW GNFTH1 LL = 100 (3444736us)
    // Value HW GNFTH1 HL = 4 (24000us)
    i2cWrite_HM5065(0x03D0, 0x64);	// ArcticCCSigmaControl fMaximumScoring 
    i2cWrite_HM5065(0x03D1, 0x00);	// ArcticCCSigmaControl fDisablePromotion {CompiledExposureTime}
    i2cWrite_HM5065(0x03D2, 0x5A);	// ArcticCCSigmaControl fDamperLowThreshold {MSB}   //24000
    i2cWrite_HM5065(0x03D3, 0xEE);	// ArcticCCSigmaControl fDamperLowThreshold {LSB}
    i2cWrite_HM5065(0x03D4, 0x69);	// ArcticCCSigmaControl DamperHighThreshold {MSB}   //3444736
    i2cWrite_HM5065(0x03D5, 0x49);	// ArcticCCSigmaControl DamperHighThreshold {LSB}
    i2cWrite_HM5065(0x03D6, 0x34);	// ArcticCCSigmaControl fMinimumDamperOutput {MSB}
    i2cWrite_HM5065(0x03D7, 0xD1);	// ArcticCCSigmaControl fMinimumDamperOutput {LSB} 
                          
                          
    //**********************************************
    // Sharpness          
    //**********************************************
                          
    // SetPeakingControl  
    i2cWrite_HM5065(0x004C, 0x08);	// PipeSetupBank0 fPeakingGain
    i2cWrite_HM5065(0x006C, 0x08);	// PipeSetupBank1 fPeakingGain
                          
    // Peaking LL value = 3 (3444736us)
    // peaking HL value = 14 (24000us)
    i2cWrite_HM5065(0x0350, 0x00);	// PeakingControl fDisableGainDamping  {CompiledExposureTime}
    i2cWrite_HM5065(0x0351, 0x5A);	// PeakingControl fDamperLowThreshold_Gain  {MSB}   //24000
    i2cWrite_HM5065(0x0352, 0xEE);	// PeakingControl fDamperLowThreshold_Gain  {LSB}
    i2cWrite_HM5065(0x0353, 0x69);	// PeakingControl fDamperHighThreshold_Gain  {MSB}  //3444736
    i2cWrite_HM5065(0x0354, 0x49);	// PeakingControl fDamperHighThreshold_Gain  {LSB}
    i2cWrite_HM5065(0x0355, 0x39);	// PeakingControl fMinimumDamperOutput_Gain  {MSB}
    i2cWrite_HM5065(0x0356, 0x6D);	// PeakingControl fMinimumDamperOutput_Gain  {LSB}
                          
    // Coring LL Value = 25 (3444736us)
    // Coring HL Value = 5 (24000us)
    i2cWrite_HM5065(0x0357, 0x19);	// PeakingControl fUserPeakLoThresh 
    i2cWrite_HM5065(0x0358, 0x00);	// PeakingControl fDisableCoringDamping  {CompiledExposureTime}
    i2cWrite_HM5065(0x0359, 0x3C);	// PeakingControl fUserPeakHiThresh
                          
    i2cWrite_HM5065(0x035A, 0x5A);	// PeakingControl fDamperLowThreshold_Coring  {MSB}  //24000
    i2cWrite_HM5065(0x035B, 0xEE);	// PeakingControl fDamperLowThreshold_Coring  {LSB}
    i2cWrite_HM5065(0x035C, 0x69);	// PeakingControl fDamperHighThreshold_Coring {MSB}  //3444736
    i2cWrite_HM5065(0x035D, 0x49);	// PeakingControl fDamperHighThreshold_Coring  {LSB}
    i2cWrite_HM5065(0x035E, 0x39);	// PeakingControl fMinimumDamperOutput_Coring  {MSB}
    i2cWrite_HM5065(0x035F, 0x85);	// PeakingControl fMinimumDamperOutput_Coring  {LSB}
                          
                          
    //**********************************************
    // Gamma              
    //**********************************************
                          
    i2cWrite_HM5065(0x0049, 0x14);	// PipeSetupBank0 bGammaGain 
    i2cWrite_HM5065(0x004A, 0x0E);	// PipeSetupBank0 bGammaInterpolationGain
    i2cWrite_HM5065(0x0069, 0x14);	// PipeSetupBank1 bGammaGain 
    i2cWrite_HM5065(0x006A, 0x0E);	// PipeSetupBank1 bGammaInterpolationGain
                          
    // SetGammaGainDamperControl
    // Value LL = 5 (3444736us)
    // Value HL = 20 (24000us)
    i2cWrite_HM5065(0x0090, 0x5A);	// GammaGainDamperControl fpX1 {MSB}   //24000
    i2cWrite_HM5065(0x0091, 0xEE);	// GammaGainDamperControl fpX1 {LSB}
    i2cWrite_HM5065(0x0092, 0x3E);	// GammaGainDamperControl fpY1 {MSB}   //1
    i2cWrite_HM5065(0x0093, 0x00);	// GammaGainDamperControl fpY1 {LSB}
    i2cWrite_HM5065(0x0094, 0x69);	// GammaGainDamperControl fpX2 {MSB}   //3444736
    i2cWrite_HM5065(0x0095, 0x49);	// GammaGainDamperControl fpX2 {LSB}
    i2cWrite_HM5065(0x0096, 0x39);	// GammaGainDamperControl fpY2 {MSB}   //0.238
    i2cWrite_HM5065(0x0097, 0xCF);	// GammaGainDamperControl fpY2 {LSB}
    i2cWrite_HM5065(0x0098, 0x01);	// GammaGainDamperControl fDisable
                          
    // SetGammaInterpolationDamperControl
    // Value LL = 6 (3444736us)
    // Value HL = 13 (24000us)
    i2cWrite_HM5065(0x00A0, 0x5A);	// GammaInterpolationDamperControl fpX1 {MSB}   //24000 
    i2cWrite_HM5065(0x00A1, 0xEE);	// GammaInterpolationDamperControl fpX1 {LSB} 
    i2cWrite_HM5065(0x00A2, 0x3E);	// GammaInterpolationDamperControl fpY1 {MSB}   //1 
    i2cWrite_HM5065(0x00A3, 0x00);	// GammaInterpolationDamperControl fpY1 {LSB} 
    i2cWrite_HM5065(0x00A4, 0x69);	// GammaInterpolationDamperControl fpX2 {MSB}   //3444736 
    i2cWrite_HM5065(0x00A5, 0x49);	// GammaInterpolationDamperControl fpX2 {LSB} 
    i2cWrite_HM5065(0x00A6, 0x3B);	// GammaInterpolationDamperControl fpY2 {MSB}   //0.4375 
    i2cWrite_HM5065(0x00A7, 0x80);	// GammaInterpolationDamperControl fpY2 {LSB} 
    i2cWrite_HM5065(0x00A8, 0x01);	// GammaInterpolationDamperControl fDisable
                          
                          
    //**********************************************
    // Lens shading based on LGA - Adaptive Lens Shading Parameters
    //**********************************************
                          
    ////////////////////////////////////////////////
    // 0420~051F, 256 bytes  
    ////////////////////////////////////////////////
                          
    i2cWrite_HM5065(0x0420, 0x00);	// C0_GreenRed_X
    i2cWrite_HM5065(0x0421, 0x73);	// C0_GreenRed_X LSB
    i2cWrite_HM5065(0x0422, 0x00);	// C0_GreenRed_Y
    i2cWrite_HM5065(0x0423, 0x73);	// C0_GreenRed_Y LSB
    i2cWrite_HM5065(0x0424, 0x00);	// C0_GreenRed_X2
    i2cWrite_HM5065(0x0425, 0x93);	// C0_GreenRed_X2 LSB
    i2cWrite_HM5065(0x0426, 0x00);	// C0_GreenRed_Y2
    i2cWrite_HM5065(0x0427, 0xBD);	// C0_GreenRed_Y2 LSB
    i2cWrite_HM5065(0x0428, 0x00);	// C0_GreenRed_XY
    i2cWrite_HM5065(0x0429, 0xB0);	// C0_GreenRed_XY LSB
    i2cWrite_HM5065(0x042A, 0x00);	// C0_GreenRed_X2Y
    i2cWrite_HM5065(0x042B, 0x6A);	// C0_GreenRed_X2Y LSB
    i2cWrite_HM5065(0x042C, 0x01);	// C0_GreenRed_XY2
    i2cWrite_HM5065(0x042D, 0x15);	// C0_GreenRed_XY2 LSB
    i2cWrite_HM5065(0x042E, 0xFF);	// C0_GreenRed_X2Y2
    i2cWrite_HM5065(0x042F, 0xF6);	// C0_GreenRed_X2Y2 LSB
    i2cWrite_HM5065(0x0430, 0x00);	// C0_Red_X
    i2cWrite_HM5065(0x0431, 0xA7);	// C0_Red_X LSB
    i2cWrite_HM5065(0x0432, 0x00);	// C0_Red_Y
    i2cWrite_HM5065(0x0433, 0x00);	// C0_Red_Y LSB
    i2cWrite_HM5065(0x0434, 0x01);	// C0_Red_X2
    i2cWrite_HM5065(0x0435, 0x28);	// C0_Red_X2 LSB
    i2cWrite_HM5065(0x0436, 0x01);	// C0_Red_Y2
    i2cWrite_HM5065(0x0437, 0x45);	// C0_Red_Y2 LSB
    i2cWrite_HM5065(0x0438, 0x00);	// C0_Red_XY
    i2cWrite_HM5065(0x0439, 0x30);	// C0_Red_XY LSB
    i2cWrite_HM5065(0x043A, 0xFF);	// C0_Red_X2Y
    i2cWrite_HM5065(0x043B, 0xA7);	// C0_Red_X2Y LSB
    i2cWrite_HM5065(0x043C, 0x01);	// C0_Red_XY2
    i2cWrite_HM5065(0x043D, 0x14);	// C0_Red_XY2 LSB
    i2cWrite_HM5065(0x043E, 0xFF);	// C0_Red_X2Y2
    i2cWrite_HM5065(0x043F, 0xD0);	// C0_Red_X2Y2 LSB
    i2cWrite_HM5065(0x0440, 0x00);	// C0_GreenBlue_X
    i2cWrite_HM5065(0x0441, 0x30);	// C0_GreenBlue_X LSB
    i2cWrite_HM5065(0x0442, 0x00);	// C0_GreenBlue_Y
    i2cWrite_HM5065(0x0443, 0x6B);	// C0_GreenBlue_Y LSB
    i2cWrite_HM5065(0x0444, 0x00);	// C0_GreenBlue_X2
    i2cWrite_HM5065(0x0445, 0xA3);	// C0_GreenBlue_X2 LSB
    i2cWrite_HM5065(0x0446, 0x00);	// C0_GreenBlue_Y2
    i2cWrite_HM5065(0x0447, 0xAB);	// C0_GreenBlue_Y2 LSB
    i2cWrite_HM5065(0x0448, 0x00);	// C0_GreenBlue_XY
    i2cWrite_HM5065(0x0449, 0xB0);	// C0_GreenBlue_XY LSB
    i2cWrite_HM5065(0x044A, 0x00);	// C0_GreenBlue_X2Y
    i2cWrite_HM5065(0x044B, 0x40);	// C0_GreenBlue_X2Y LSB
    i2cWrite_HM5065(0x044C, 0xFF);	// C0_GreenBlue_XY2
    i2cWrite_HM5065(0x044D, 0xF5);	// C0_GreenBlue_XY2 LSB
    i2cWrite_HM5065(0x044E, 0xFF);	// C0_GreenBlue_X2Y2
    i2cWrite_HM5065(0x044F, 0xED);	// C0_GreenBlue_X2Y2 LSB
    i2cWrite_HM5065(0x0450, 0x00);	// C0_Blue_X
    i2cWrite_HM5065(0x0451, 0x7F);	// C0_Blue_X LSB
    i2cWrite_HM5065(0x0452, 0x00);	// C0_Blue_Y
    i2cWrite_HM5065(0x0453, 0x4C);	// C0_Blue_Y LSB
    i2cWrite_HM5065(0x0454, 0x00);	// C0_Blue_X2
    i2cWrite_HM5065(0x0455, 0x7F);	// C0_Blue_X2 LSB
    i2cWrite_HM5065(0x0456, 0x00);	// C0_Blue_Y2
    i2cWrite_HM5065(0x0457, 0x99);	// C0_Blue_Y2 LSB
    i2cWrite_HM5065(0x0458, 0xFF);	// C0_Blue_XY
    i2cWrite_HM5065(0x0459, 0xC0);	// C0_Blue_XY LSB
    i2cWrite_HM5065(0x045A, 0xFF);	// C0_Blue_X2Y
    i2cWrite_HM5065(0x045B, 0xD3);	// C0_Blue_X2Y LSB
    i2cWrite_HM5065(0x045C, 0x00);	// C0_Blue_XY2
    i2cWrite_HM5065(0x045D, 0x06);	// C0_Blue_XY2 LSB
    i2cWrite_HM5065(0x045E, 0x00);	// C0_Blue_X2Y2
    i2cWrite_HM5065(0x045F, 0x39);	// C0_Blue_X2Y2 LSB
    i2cWrite_HM5065(0x0460, 0x00);	// C1_GreenRed_X
    i2cWrite_HM5065(0x0461, 0x0B);	// C1_GreenRed_X LSB
    i2cWrite_HM5065(0x0462, 0x00);	// C1_GreenRed_Y
    i2cWrite_HM5065(0x0463, 0x79);	// C1_GreenRed_Y LSB
    i2cWrite_HM5065(0x0464, 0x00);	// C1_GreenRed_X2
    i2cWrite_HM5065(0x0465, 0x88);	// C1_GreenRed_X2 LSB
    i2cWrite_HM5065(0x0466, 0x00);	// C1_GreenRed_Y2
    i2cWrite_HM5065(0x0467, 0xA8);	// C1_GreenRed_Y2 LSB
    i2cWrite_HM5065(0x0468, 0x01);	// C1_GreenRed_XY
    i2cWrite_HM5065(0x0469, 0x20);	// C1_GreenRed_XY LSB
    i2cWrite_HM5065(0x046A, 0x00);	// C1_GreenRed_X2Y
    i2cWrite_HM5065(0x046B, 0x8C);	// C1_GreenRed_X2Y LSB
    i2cWrite_HM5065(0x046C, 0x00);	// C1_GreenRed_XY2
    i2cWrite_HM5065(0x046D, 0x41);	// C1_GreenRed_XY2 LSB
    i2cWrite_HM5065(0x046E, 0x00);	// C1_GreenRed_X2Y2
    i2cWrite_HM5065(0x046F, 0x02);	// C1_GreenRed_X2Y2 LSB
    i2cWrite_HM5065(0x0470, 0x00);	// C1_Red_X
    i2cWrite_HM5065(0x0471, 0x16);	// C1_Red_X LSB
    i2cWrite_HM5065(0x0472, 0x00);	// C1_Red_Y
    i2cWrite_HM5065(0x0473, 0x08);	// C1_Red_Y LSB
    i2cWrite_HM5065(0x0474, 0x00);	// C1_Red_X2
    i2cWrite_HM5065(0x0475, 0xCD);	// C1_Red_X2 LSB
    i2cWrite_HM5065(0x0476, 0x00);	// C1_Red_Y2
    i2cWrite_HM5065(0x0477, 0xE4);	// C1_Red_Y2 LSB
    i2cWrite_HM5065(0x0478, 0x00);	// C1_Red_XY
    i2cWrite_HM5065(0x0479, 0x3B);	// C1_Red_XY LSB
    i2cWrite_HM5065(0x047A, 0x00);	// C1_Red_X2Y
    i2cWrite_HM5065(0x047B, 0x1A);	// C1_Red_X2Y LSB
    i2cWrite_HM5065(0x047C, 0xFF);	// C1_Red_XY2
    i2cWrite_HM5065(0x047D, 0x82);	// C1_Red_XY2 LSB
    i2cWrite_HM5065(0x047E, 0xFF);	// C1_Red_X2Y2
    i2cWrite_HM5065(0x047F, 0xE8);	// C1_Red_X2Y2 LSB
    i2cWrite_HM5065(0x0480, 0xFF);	// C1_GreenBlue_X
    i2cWrite_HM5065(0x0481, 0xC7);	// C1_GreenBlue_X LSB
    i2cWrite_HM5065(0x0482, 0x00);	// C1_GreenBlue_Y
    i2cWrite_HM5065(0x0483, 0x80);	// C1_GreenBlue_Y LSB
    i2cWrite_HM5065(0x0484, 0x00);	// C1_GreenBlue_X2
    i2cWrite_HM5065(0x0485, 0x93);	// C1_GreenBlue_X2 LSB
    i2cWrite_HM5065(0x0486, 0x00);	// C1_GreenBlue_Y2
    i2cWrite_HM5065(0x0487, 0x9D);	// C1_GreenBlue_Y2 LSB
    i2cWrite_HM5065(0x0488, 0x01);	// C1_GreenBlue_XY
    i2cWrite_HM5065(0x0489, 0x20);	// C1_GreenBlue_XY LSB
    i2cWrite_HM5065(0x048A, 0x00);	// C1_GreenBlue_X2Y
    i2cWrite_HM5065(0x048B, 0x4B);	// C1_GreenBlue_X2Y LSB
    i2cWrite_HM5065(0x048C, 0xFF);	// C1_GreenBlue_XY2
    i2cWrite_HM5065(0x048D, 0x5B);	// C1_GreenBlue_XY2 LSB
    i2cWrite_HM5065(0x048E, 0xFF);	// C1_GreenBlue_X2Y2
    i2cWrite_HM5065(0x048F, 0xFC);	// C1_GreenBlue_X2Y2 LSB
    i2cWrite_HM5065(0x0490, 0x00);	// C1_Blue_X
    i2cWrite_HM5065(0x0491, 0x2E);	// C1_Blue_X LSB
    i2cWrite_HM5065(0x0492, 0x00);	// C1_Blue_Y
    i2cWrite_HM5065(0x0493, 0x45);	// C1_Blue_Y LSB
    i2cWrite_HM5065(0x0494, 0x00);	// C1_Blue_X2
    i2cWrite_HM5065(0x0495, 0x72);	// C1_Blue_X2 LSB
    i2cWrite_HM5065(0x0496, 0x00);	// C1_Blue_Y2
    i2cWrite_HM5065(0x0497, 0x88);	// C1_Blue_Y2 LSB
    i2cWrite_HM5065(0x0498, 0x00);	// C1_Blue_XY
    i2cWrite_HM5065(0x0499, 0x20);	// C1_Blue_XY LSB
    i2cWrite_HM5065(0x049A, 0x00);	// C1_Blue_X2Y
    i2cWrite_HM5065(0x049B, 0x04);	// C1_Blue_X2Y LSB
    i2cWrite_HM5065(0x049C, 0xFF);	// C1_Blue_XY2
    i2cWrite_HM5065(0x049D, 0x9C);	// C1_Blue_XY2 LSB
    i2cWrite_HM5065(0x049E, 0xFF);	// C1_Blue_X2Y2
    i2cWrite_HM5065(0x049F, 0xF8);	// C1_Blue_X2Y2 LSB
    i2cWrite_HM5065(0x04A0, 0x00);	// C2_GreenRed_X
    i2cWrite_HM5065(0x04A1, 0xA2);	// C2_GreenRed_X LSB
    i2cWrite_HM5065(0x04A2, 0x00);	// C2_GreenRed_Y
    i2cWrite_HM5065(0x04A3, 0x7F);	// C2_GreenRed_Y LSB
    i2cWrite_HM5065(0x04A4, 0x00);	// C2_GreenRed_X2
    i2cWrite_HM5065(0x04A5, 0xC2);	// C2_GreenRed_X2 LSB
    i2cWrite_HM5065(0x04A6, 0x00);	// C2_GreenRed_Y2
    i2cWrite_HM5065(0x04A7, 0xDA);	// C2_GreenRed_Y2 LSB
    i2cWrite_HM5065(0x04A8, 0x01);	// C2_GreenRed_XY
    i2cWrite_HM5065(0x04A9, 0x00);	// C2_GreenRed_XY LSB
    i2cWrite_HM5065(0x04AA, 0x00);	// C2_GreenRed_X2Y
    i2cWrite_HM5065(0x04AB, 0xAB);	// C2_GreenRed_X2Y LSB
    i2cWrite_HM5065(0x04AC, 0x01);	// C2_GreenRed_XY2
    i2cWrite_HM5065(0x04AD, 0x6F);	// C2_GreenRed_XY2 LSB
    i2cWrite_HM5065(0x04AE, 0x00);	// C2_GreenRed_X2Y2
    i2cWrite_HM5065(0x04AF, 0x6D);	// C2_GreenRed_X2Y2 LSB
    i2cWrite_HM5065(0x04B0, 0x00);	// C2_Red_X
    i2cWrite_HM5065(0x04B1, 0xCE);	// C2_Red_X LSB
    i2cWrite_HM5065(0x04B2, 0x00);	// C2_Red_Y
    i2cWrite_HM5065(0x04B3, 0x2F);	// C2_Red_Y LSB
    i2cWrite_HM5065(0x04B4, 0x01);	// C2_Red_X2
    i2cWrite_HM5065(0x04B5, 0x3D);	// C2_Red_X2 LSB
    i2cWrite_HM5065(0x04B6, 0x01);	// C2_Red_Y2
    i2cWrite_HM5065(0x04B7, 0x4E);	// C2_Red_Y2 LSB
    i2cWrite_HM5065(0x04B8, 0x00);	// C2_Red_XY
    i2cWrite_HM5065(0x04B9, 0x0D);	// C2_Red_XY LSB
    i2cWrite_HM5065(0x04BA, 0xFF);	// C2_Red_X2Y
    i2cWrite_HM5065(0x04BB, 0xEC);	// C2_Red_X2Y LSB
    i2cWrite_HM5065(0x04BC, 0x00);	// C2_Red_XY2
    i2cWrite_HM5065(0x04BD, 0xD4);	// C2_Red_XY2 LSB
    i2cWrite_HM5065(0x04BE, 0x00);	// C2_Red_X2Y2
    i2cWrite_HM5065(0x04BF, 0xCB);	// C2_Red_X2Y2 LSB
    i2cWrite_HM5065(0x04C0, 0x00);	// C2_GreenBlue_X
    i2cWrite_HM5065(0x04C1, 0x42);	// C2_GreenBlue_X LSB
    i2cWrite_HM5065(0x04C2, 0x00);	// C2_GreenBlue_Y
    i2cWrite_HM5065(0x04C3, 0x84);	// C2_GreenBlue_Y LSB
    i2cWrite_HM5065(0x04C4, 0x00);	// C2_GreenBlue_X2
    i2cWrite_HM5065(0x04C5, 0xC9);	// C2_GreenBlue_X2 LSB
    i2cWrite_HM5065(0x04C6, 0x00);	// C2_GreenBlue_Y2
    i2cWrite_HM5065(0x04C7, 0xD2);	// C2_GreenBlue_Y2 LSB
    i2cWrite_HM5065(0x04C8, 0x01);	// C2_GreenBlue_XY
    i2cWrite_HM5065(0x04C9, 0x00);	// C2_GreenBlue_XY LSB
    i2cWrite_HM5065(0x04CA, 0x00);	// C2_GreenBlue_X2Y
    i2cWrite_HM5065(0x04CB, 0x28);	// C2_GreenBlue_X2Y LSB
    i2cWrite_HM5065(0x04CC, 0xFF);	// C2_GreenBlue_XY2
    i2cWrite_HM5065(0x04CD, 0xEA);	// C2_GreenBlue_XY2 LSB
    i2cWrite_HM5065(0x04CE, 0x00);	// C2_GreenBlue_X2Y2
    i2cWrite_HM5065(0x04CF, 0x5F);	// C2_GreenBlue_X2Y2 LSB
    i2cWrite_HM5065(0x04D0, 0x00);	// C2_Blue_X
    i2cWrite_HM5065(0x04D1, 0xB2);	// C2_Blue_X LSB
    i2cWrite_HM5065(0x04D2, 0x00);	// C2_Blue_Y
    i2cWrite_HM5065(0x04D3, 0x3E);	// C2_Blue_Y LSB
    i2cWrite_HM5065(0x04D4, 0x00);	// C2_Blue_X2
    i2cWrite_HM5065(0x04D5, 0xA1);	// C2_Blue_X2 LSB
    i2cWrite_HM5065(0x04D6, 0x00);	// C2_Blue_Y2
    i2cWrite_HM5065(0x04D7, 0xB4);	// C2_Blue_Y2 LSB
    i2cWrite_HM5065(0x04D8, 0xFF);	// C2_Blue_XY
    i2cWrite_HM5065(0x04D9, 0xE0);	// C2_Blue_XY LSB
    i2cWrite_HM5065(0x04DA, 0xFF);	// C2_Blue_X2Y
    i2cWrite_HM5065(0x04DB, 0xB8);	// C2_Blue_X2Y LSB
    i2cWrite_HM5065(0x04DC, 0x00);	// C2_Blue_XY2
    i2cWrite_HM5065(0x04DD, 0x78);	// C2_Blue_XY2 LSB
    i2cWrite_HM5065(0x04DE, 0x00);	// C2_Blue_X2Y2
    i2cWrite_HM5065(0x04DF, 0x71);	// C2_Blue_X2Y2 LSB
    i2cWrite_HM5065(0x04E0, 0x00);	// C3_GreenRed_X
    i2cWrite_HM5065(0x04E1, 0x49);	// C3_GreenRed_X LSB
    i2cWrite_HM5065(0x04E2, 0x00);	// C3_GreenRed_Y
    i2cWrite_HM5065(0x04E3, 0x77);	// C3_GreenRed_Y LSB
    i2cWrite_HM5065(0x04E4, 0x00);	// C3_GreenRed_X2
    i2cWrite_HM5065(0x04E5, 0x88);	// C3_GreenRed_X2 LSB
    i2cWrite_HM5065(0x04E6, 0x00);	// C3_GreenRed_Y2
    i2cWrite_HM5065(0x04E7, 0xA1);	// C3_GreenRed_Y2 LSB
    i2cWrite_HM5065(0x04E8, 0x01);	// C3_GreenRed_XY
    i2cWrite_HM5065(0x04E9, 0x00);	// C3_GreenRed_XY LSB
    i2cWrite_HM5065(0x04EA, 0x00);	// C3_GreenRed_X2Y
    i2cWrite_HM5065(0x04EB, 0x88);	// C3_GreenRed_X2Y LSB
    i2cWrite_HM5065(0x04EC, 0x00);	// C3_GreenRed_XY2
    i2cWrite_HM5065(0x04ED, 0xEB);	// C3_GreenRed_XY2 LSB
    i2cWrite_HM5065(0x04EE, 0x00);	// C3_GreenRed_X2Y2
    i2cWrite_HM5065(0x04EF, 0x14);	// C3_GreenRed_X2Y2 LSB
    i2cWrite_HM5065(0x04F0, 0x00);	// C3_Red_X
    i2cWrite_HM5065(0x04F1, 0x54);	// C3_Red_X LSB
    i2cWrite_HM5065(0x04F2, 0x00);	// C3_Red_Y
    i2cWrite_HM5065(0x04F3, 0x0E);	// C3_Red_Y LSB
    i2cWrite_HM5065(0x04F4, 0x00);	// C3_Red_X2
    i2cWrite_HM5065(0x04F5, 0xC1);	// C3_Red_X2 LSB
    i2cWrite_HM5065(0x04F6, 0x00);	// C3_Red_Y2
    i2cWrite_HM5065(0x04F7, 0xD2);	// C3_Red_Y2 LSB
    i2cWrite_HM5065(0x04F8, 0x00);	// C3_Red_XY
    i2cWrite_HM5065(0x04F9, 0x00);	// C3_Red_XY LSB
    i2cWrite_HM5065(0x04FA, 0x00);	// C3_Red_X2Y
    i2cWrite_HM5065(0x04FB, 0x20);	// C3_Red_X2Y LSB
    i2cWrite_HM5065(0x04FC, 0x00);	// C3_Red_XY2
    i2cWrite_HM5065(0x04FD, 0x25);	// C3_Red_XY2 LSB
    i2cWrite_HM5065(0x04FE, 0x00);	// C3_Red_X2Y2
    i2cWrite_HM5065(0x04FF, 0x41);	// C3_Red_X2Y2 LSB
    i2cWrite_HM5065(0x0500, 0xFF);	// C3_GreenBlue_X
    i2cWrite_HM5065(0x0501, 0xF9);	// C3_GreenBlue_X LSB
    i2cWrite_HM5065(0x0502, 0x00);	// C3_GreenBlue_Y
    i2cWrite_HM5065(0x0503, 0x77);	// C3_GreenBlue_Y LSB
    i2cWrite_HM5065(0x0504, 0x00);	// C3_GreenBlue_X2
    i2cWrite_HM5065(0x0505, 0x8E);	// C3_GreenBlue_X2 LSB
    i2cWrite_HM5065(0x0506, 0x00);	// C3_GreenBlue_Y2
    i2cWrite_HM5065(0x0507, 0x9A);	// C3_GreenBlue_Y2 LSB
    i2cWrite_HM5065(0x0508, 0x01);	// C3_GreenBlue_XY
    i2cWrite_HM5065(0x0509, 0x00);	// C3_GreenBlue_XY LSB
    i2cWrite_HM5065(0x050A, 0x00);	// C3_GreenBlue_X2Y
    i2cWrite_HM5065(0x050B, 0x2C);	// C3_GreenBlue_X2Y LSB
    i2cWrite_HM5065(0x050C, 0xFF);	// C3_GreenBlue_XY2
    i2cWrite_HM5065(0x050D, 0xDD);	// C3_GreenBlue_XY2 LSB
    i2cWrite_HM5065(0x050E, 0x00);	// C3_GreenBlue_X2Y2
    i2cWrite_HM5065(0x050F, 0x0C);	// C3_GreenBlue_X2Y2 LSB
    i2cWrite_HM5065(0x0510, 0x00);	// C3_Blue_X
    i2cWrite_HM5065(0x0511, 0x7E);	// C3_Blue_X LSB
    i2cWrite_HM5065(0x0512, 0x00);	// C3_Blue_Y
    i2cWrite_HM5065(0x0513, 0x35);	// C3_Blue_Y LSB
    i2cWrite_HM5065(0x0514, 0x00);	// C3_Blue_X2
    i2cWrite_HM5065(0x0515, 0x72);	// C3_Blue_X2 LSB
    i2cWrite_HM5065(0x0516, 0x00);	// C3_Blue_Y2
    i2cWrite_HM5065(0x0517, 0x86);	// C3_Blue_Y2 LSB
    i2cWrite_HM5065(0x0518, 0xFF);	// C3_Blue_XY
    i2cWrite_HM5065(0x0519, 0xC0);	// C3_Blue_XY LSB
    i2cWrite_HM5065(0x051A, 0xFF);	// C3_Blue_X2Y
    i2cWrite_HM5065(0x051B, 0xD8);	// C3_Blue_X2Y LSB
    i2cWrite_HM5065(0x051C, 0x00);	// C3_Blue_XY2
    i2cWrite_HM5065(0x051D, 0x57);	// C3_Blue_XY2 LSB
    i2cWrite_HM5065(0x051E, 0x00);	// C3_Blue_X2Y2
    i2cWrite_HM5065(0x051F, 0x22);	// C3_Blue_X2Y2 LSB
                          
    i2cWrite_HM5065(0x0561, 0x21);	// C0 Unity
    i2cWrite_HM5065(0x0562, 0x1C);	// C1 Unity
    i2cWrite_HM5065(0x0563, 0x17);	// C2 Unity
    i2cWrite_HM5065(0x0564, 0x14);	// C3 Unity
                          
    // Casts              
    i2cWrite_HM5065(0x0324, 0x3A);	// NormRedGain_Cast0 Hor
    i2cWrite_HM5065(0x0325, 0x00);	// NormRedGain_Cast0_LSB 
    i2cWrite_HM5065(0x0326, 0x3A);	// NormRedGain_Cast1 IncA
    i2cWrite_HM5065(0x0327, 0x67);	// NormRedGain_Cast1_LSB 
    i2cWrite_HM5065(0x0328, 0x3A);	// NormRedGain_Cast2 CWF
    i2cWrite_HM5065(0x0329, 0xCD);	// NormRedGain_Cast2_LSB 
    i2cWrite_HM5065(0x032A, 0x3A);	// NormRedGain_Cast3 D65
    i2cWrite_HM5065(0x032B, 0xF6);	// NormRedGain_Cast3_LSB 
                          
    // Antivignette Control
    i2cWrite_HM5065(0x0320, 0x01);	// AntiVignetteControl - Enable
    i2cWrite_HM5065(0x0321, 0x04);	// NbOfPresets
    i2cWrite_HM5065(0x0322, 0x01);	// AdaptiveAntiVignetteControlEnable - Enable
    i2cWrite_HM5065(0x0323, 0x01);	// LoLightAntiVignetteControlDisable - Damper Off
                          
                          
    //**********************************************
    // Adaptive Colour Matrices
    //**********************************************
                          
    // Adaptive Colour Matrices
    i2cWrite_HM5065(0x0330, 0x01);	// Turn off colour matrix damper
    i2cWrite_HM5065(0x0384, 0x00);	// Turn off colour effects
    i2cWrite_HM5065(0x0337, 0x01);	// Turn on adaptive colour matrix
                          
    // Normalised red gain presets for Colour Matrices
    i2cWrite_HM5065(0x03EC, 0x3A);	// Matrix 0
    i2cWrite_HM5065(0x03ED, 0x00);	// LSB
    i2cWrite_HM5065(0x03FC, 0x3A);	// Matrix 1
    i2cWrite_HM5065(0x03FD, 0x67);	// LSB
    i2cWrite_HM5065(0x040C, 0x3A);	// Matrix 2
    i2cWrite_HM5065(0x040D, 0xB9);	// LSB
    i2cWrite_HM5065(0x041C, 0x3B);	// Matrix 3
    i2cWrite_HM5065(0x041D, 0x0B);	// LSB
                          
    // Colour Matrices based on 953 cut 3.0 measured matrices
                          
    // Colour Matrix 0 - IncA
                          
    i2cWrite_HM5065(0x03E0, 0xB6);	// GInR
    i2cWrite_HM5065(0x03E1, 0x04);	//
    i2cWrite_HM5065(0x03E2, 0xBB);	// BInR
    i2cWrite_HM5065(0x03E3, 0xE9);	//
    i2cWrite_HM5065(0x03E4, 0xBC);	// RInG
    i2cWrite_HM5065(0x03E5, 0x70);	//
    i2cWrite_HM5065(0x03E6, 0x39);	// BInG
    i2cWrite_HM5065(0x03E7, 0x95);	//
    i2cWrite_HM5065(0x03E8, 0xBB);	// RInB
    i2cWrite_HM5065(0x03E9, 0xAE);	//
    i2cWrite_HM5065(0x03EA, 0xBF);	// GInB
    i2cWrite_HM5065(0x03EB, 0x81);	//
                          
    // Colour Matrix 1 - 4000K
                          
    i2cWrite_HM5065(0x03F0, 0xBA);	// GInR
    i2cWrite_HM5065(0x03F1, 0x7B);	//
    i2cWrite_HM5065(0x03F2, 0xBA);	// BInR
    i2cWrite_HM5065(0x03F3, 0x83);	//
    i2cWrite_HM5065(0x03F4, 0xBB);	// RInG
    i2cWrite_HM5065(0x03F5, 0xBC);	//
    i2cWrite_HM5065(0x03F6, 0x39);	// BInG
    i2cWrite_HM5065(0x03F7, 0x95);	//
    i2cWrite_HM5065(0x03F8, 0xBB);	// RInB
    i2cWrite_HM5065(0x03F9, 0x23);	//
    i2cWrite_HM5065(0x03FA, 0xBD);	// GInB
    i2cWrite_HM5065(0x03FB, 0xAC);	//
                          
    // Colour Matrix 2 - D50
                          
    i2cWrite_HM5065(0x0400, 0xBB);	// GInR
    i2cWrite_HM5065(0x0401, 0xA8);	//
    i2cWrite_HM5065(0x0402, 0xBC);	// BInR
    i2cWrite_HM5065(0x0403, 0x02);	//
    i2cWrite_HM5065(0x0404, 0xB9);	// RInG
    i2cWrite_HM5065(0x0405, 0x50);	//
    i2cWrite_HM5065(0x0406, 0xBA);	// BInG
    i2cWrite_HM5065(0x0407, 0x37);	//
    i2cWrite_HM5065(0x0408, 0xB7);	// RInB
    i2cWrite_HM5065(0x0409, 0xC7);	//
    i2cWrite_HM5065(0x040A, 0xBC);	// GInB
    i2cWrite_HM5065(0x040B, 0x18);	//
                          
    // Colour Matrix 3 - D65
                          
    i2cWrite_HM5065(0x0410, 0xBB);	// GInR
    i2cWrite_HM5065(0x0411, 0xE0);	//
    i2cWrite_HM5065(0x0412, 0xB9);	// BInR
    i2cWrite_HM5065(0x0413, 0x16);	//
    i2cWrite_HM5065(0x0414, 0xBB);	// RInG
    i2cWrite_HM5065(0x0415, 0x1B);	//
    i2cWrite_HM5065(0x0416, 0xBA);	// BInG
    i2cWrite_HM5065(0x0417, 0x98);	//
    i2cWrite_HM5065(0x0418, 0xB8);	// RInB
    i2cWrite_HM5065(0x0419, 0x4E);	//
    i2cWrite_HM5065(0x041A, 0xBC);	// GInB
    i2cWrite_HM5065(0x041B, 0x5E);	//
                          
                          
    //**********************************************
    // White Balance Setup - Saturation, MWWB tilt gains, Tilts, Gain Clip, Constrainer
    //**********************************************
                          
    i2cWrite_HM5065(0x01A5, 0x3E);	// Hue Red Bias
    i2cWrite_HM5065(0x01A6, 0x00);
    i2cWrite_HM5065(0x01A7, 0x3E);	// Hue Blue Bias
    i2cWrite_HM5065(0x01A8, 0x00);
                          
    i2cWrite_HM5065(0x01f8, 0x3c);	// fpMaximumDistanceAllowedFromLocus
    i2cWrite_HM5065(0x01f9, 0x00);	// =0.5
    i2cWrite_HM5065(0x01fa, 0x00);	// fEnableConstrainedWhiteBalance = false
                          
    i2cWrite_HM5065(0x02a2, 0x3e);	// fpRedTilt
    i2cWrite_HM5065(0x02a3, 0x00);	// = 1.00
    i2cWrite_HM5065(0x02a4, 0x3e);	// fpGreenTilt1
    i2cWrite_HM5065(0x02a5, 0x00);	// = 1.00
    i2cWrite_HM5065(0x02a6, 0x3e);	// fpGreenTilt2
    i2cWrite_HM5065(0x02a7, 0x00);	// = 1.00
    i2cWrite_HM5065(0x02a8, 0x3e);	// fpBlueTilt
    i2cWrite_HM5065(0x02a9, 0x00);	// = 1.00
                          
    //MWWB Tilts need set high to avoid constrainer bug
    i2cWrite_HM5065(0x056a, 0x02);
    i2cWrite_HM5065(0x056b, 0xf0);
    i2cWrite_HM5065(0x056c, 0x42);	// fpRedTilt
    i2cWrite_HM5065(0x056d, 0x00);	// = 4.00
    i2cWrite_HM5065(0x056e, 0x42);	// fpGreenTilt1
    i2cWrite_HM5065(0x056f, 0x00);	// = 4.00
    i2cWrite_HM5065(0x0570, 0x42);	// fpGreenTilt2
    i2cWrite_HM5065(0x0571, 0x00);	// = 4.00
    i2cWrite_HM5065(0x0572, 0x42);	// fpBlueTilt
    i2cWrite_HM5065(0x0573, 0x00);	// = 4.00
                          
                          
    //**********************************************
    // Colour Saturation  
    //**********************************************
                          
    // Set Saturation     
    i2cWrite_HM5065(0x0081, 0x5C);	// PipeSetupCommon bColourSaturation
                          
    // Set Colour Saturation Damper
    // Value LL = 85      
    // Value HL = 105     
    i2cWrite_HM5065(0x0588, 0x00);	// ColourSaturationDamper fDisable {CompiledExposureTime}
    i2cWrite_HM5065(0x0589, 0x5A);	// ColourSaturationDamper fpLowThreshold {MSB}
    i2cWrite_HM5065(0x058A, 0xEE);	// ColourSaturationDamper fpLowThreshold {LSB}
    i2cWrite_HM5065(0x058B, 0x69);	// ColourSaturationDamper fpHighThreshold {MSB}
    i2cWrite_HM5065(0x058C, 0x49);	// ColourSaturationDamper fpHighThreshold {LSB}
    i2cWrite_HM5065(0x058D, 0x3D);	// ColourSaturationDamper fpMinimumOutput {MSB}
    i2cWrite_HM5065(0x058E, 0x3D);	// ColourSaturationDamper fpMinimumOutput {LSB}
                          
                          
    //**********************************************
    // Others             
    //**********************************************
                          
    // Set Contrast       
    i2cWrite_HM5065(0x0080, 0x6C);	// PipeSetupCommon bContrast
                          
    // Set Brightness     
    i2cWrite_HM5065(0x0082, 0x5A);	// PipeSetupCommon bBrightness
                          
    // Set Exposure Compensation
    // WriteByte(0x0130, 0xff)//     //iExposureCompensation = -1
                          
                          
    //**********************************************
    // AF and VCM settings - 29Aug2012
    //**********************************************
    i2cWrite_HM5065(0x0659, 0x01);	// AFStatsControls->bCoringValue = 1 Coring Value // Midified by HP 2013/01/08 
    i2cWrite_HM5065(0x065A, 0x00);	// AFStatsControls->bWindowsSystem = 7 zone AF system 
                          
    i2cWrite_HM5065(0x06C9, 0x01);	// FLADriverLowLevelParameters->AutoSkipNextFrame = ENABLED
    i2cWrite_HM5065(0x06CD, 0x01);	// FLADriverLowLevelParameters->AF_OTP_uwHostDefMacro MSB = 445
    i2cWrite_HM5065(0x06CE, 0xBD);	// FLADriverLowLevelParameters->AF_OTP_uwHostDefMacro LSB
    i2cWrite_HM5065(0x06CF, 0x00);	// FLADriverLowLevelParameters->AF_OTP_uwHostDefInfinity MSB = 147
    i2cWrite_HM5065(0x06D0, 0x93);	// FLADriverLowLevelParameters->AF_OTP_uwHostDefInfinity LSB
    i2cWrite_HM5065(0x06D1, 0x02);	// FLADriverLowLevelParameters->AF_OTP_bStepsMultiStepDriver = 2 step driver
    i2cWrite_HM5065(0x06D2, 0x30);	// FLADriverLowLevelParameters->AF_OTP_uwMultiStepTimeDelay MSB = 12.5ms
    i2cWrite_HM5065(0x06D3, 0xD4);	// FLADriverLowLevelParameters->AF_OTP_uwMultiStepTimeDelay LSB
    i2cWrite_HM5065(0x06D4, 0x01);	// FLADriverLowLevelParameters->AF_OTP_fHostEnableOTPRead (1 = disabled)
    i2cWrite_HM5065(0x06DB, 0x59);	// FLADriverLowLevelParameters->fpActuatorResponseTime MSB 12.5ms (FP900) 
    i2cWrite_HM5065(0x06DC, 0x0d);	// FLADriverLowLevelParameters->fpActuatorResponseTime LSB
                          
    i2cWrite_HM5065(0x0730, 0x00);	// FocusRangeConstants->wFullRange_LensMinPosition MSB = 0
    i2cWrite_HM5065(0x0731, 0x00);	// FocusRangeConstants->wFullRange_LensMinPosition LSB
    i2cWrite_HM5065(0x0732, 0x03);	// FocusRangeConstants->wFullRange_LensMaxPosition MSB = 1023
    i2cWrite_HM5065(0x0733, 0xFF);	// FocusRangeConstants->wFullRange_LensMaxPosition LSB
    i2cWrite_HM5065(0x0734, 0x03);	// FocusRangeConstants->wFullRange_LensRecoveryPosition MSB = 880
    i2cWrite_HM5065(0x0735, 0x70);	// FocusRangeConstants->wFullRange_LensRecoveryPosition LSB
                          
    i2cWrite_HM5065(0x0755, 0x01);	// AutoFocusControls->fEnableSimpleCoarseThEvaluation = ENABLED
    i2cWrite_HM5065(0x0756, 0x00);	// AutoFocusControls->bSelectedMultizoneBehavior = REGIONSELECTIONMETHOD_AVERAGE //Modified by HP
    i2cWrite_HM5065(0x075A, 0x00);	// AutoFocusControls-fLightVariationEnable = FALSE = do not enable with bugs //Modified by HP
    i2cWrite_HM5065(0x075B, 0x01);	// AutoFocusControls->fEnableTrackingThresholdEvaluation = DISABLED //MK change to 0x1
    i2cWrite_HM5065(0x075E, 0x00);	// AutoFocusControls->fFineToCoarseAutoTransitionEnable = DISABLED
    //i2cWrite_HM5065(0x075F, 0x00);	// AutoFocusControls-fEnableTimedFineExecution = FALSE // Disable  // Modified by HP
    i2cWrite_HM5065(0x0764, 0x01);	// AutoFocusControls->fResetHCSPos = TRUE = Start from Recovery Position for every HCS
    i2cWrite_HM5065(0x0766, 0x01);	// AutoFocusControls->fEnablePrioritiesMacro = FALSE = Do not prioritise Macro //mk change to 0x01
    i2cWrite_HM5065(0x0768, 0x01);	// AutoFocusControls->fEnableInterpolationAfterFineSearch = TRUE
    i2cWrite_HM5065(0x076A, 0x00);	// AutoFocusControls->fReducedZoneSetup = TRUE //mk change to 0x0
                          
                          
    i2cWrite_HM5065(0x0758, 0x01);	// AutoFocusControls->bWeighedFunctionSelected = TRAPEZIUM
    i2cWrite_HM5065(0x075C, 0x01);	// AutoFocusControls->fEnableHeuristicMethod = FALSE
                          
    i2cWrite_HM5065(0x0770, 0x98);	// AutoFocusConstants->bCoarseStep = 95
    i2cWrite_HM5065(0x0771, 0x19);	// AutoFocusConstants->bFineStep = 16
    i2cWrite_HM5065(0x0772, 0x1B);	// AutoFocusConstants->bFullSearchStep = 27
    i2cWrite_HM5065(0x0774, 0x01);	// AutoFocusConstants->uwFineThreshold MSB = 330 
    i2cWrite_HM5065(0x0775, 0x4a);	// AutoFocusConstants->uwFineThreshold LSB 
    i2cWrite_HM5065(0x0777, 0x00);	// AutoFocusConstants->uwBacklightThreshold MSB = 69
    i2cWrite_HM5065(0x0778, 0x45);	// AutoFocusConstants->uwBacklightThreshold LSB 
    i2cWrite_HM5065(0x0779, 0x00);	// AutoFocusConstants->uwMotionBlurInRatio MSB = 2
    i2cWrite_HM5065(0x077A, 0x02);	// AutoFocusConstants->uwMotionBlurInRatio LSB
    i2cWrite_HM5065(0x077D, 0x01);	// AutoFocusConstants->bMaxNumberContinuouslyInstableTime = 1
    i2cWrite_HM5065(0x077E, 0x03);	// AutoFocusConstants->bMaxNumberContinuouslyStableFrame = 3
    i2cWrite_HM5065(0x0783, 0x10);	// AutoFocusConstants->bLightGap = 10 
    i2cWrite_HM5065(0x0785, 0x14);	// AutoFocusConstants->uwDeltaValue = 20
    i2cWrite_HM5065(0x0788, 0x04);	// AutoFocusConstants->bMinNumberMacroRegion = 4 //mk add
                          
    i2cWrite_HM5065(0x0846, 0x06);	// AutoFocusHeuristicConstants->bHighToMaxFMShiftFactor = 6
    i2cWrite_HM5065(0x0847, 0x05);	// AutoFocusHeuristicConstants->bLowToHighFMShiftFactor = 5
                          
                          
    //**********************************************
    // MIPI CSI settings - 14May2012
    //**********************************************
    i2cWrite_HM5065(0xC41A, 0x05);	// TEST_LP_TX (clock slew rate)
    i2cWrite_HM5065(0xC423, 0x11);	// TEST_LP_TX_SLEW_RATE_DL1
    i2cWrite_HM5065(0xC427, 0x11);	// TEST_LP_TX_SLEW_RATE_DL2
    i2cWrite_HM5065(0x300B, 0x09);	// esc_clk_div (clk_sys div by 10)
                          
    //**********************************************
    // CMD_RUN            
    //**********************************************
                          
    //i2cWrite_HM5065(0x0010, 0x01);	// CMD_RUN
                          
    //**********************************************
    // AV                 
    //**********************************************
    OSTimeDly(4);			// Sleep... Important !
                          
    // Set horizontal and vertical offsets to zero
    i2cWrite_HM5065(0x4708, 0x00);	// av2x2_h_offset
    i2cWrite_HM5065(0x4709, 0x00);	// LSB
    i2cWrite_HM5065(0x4710, 0x00);	// av2x2_v_offset {0x4710 & 11} are correct!
    i2cWrite_HM5065(0x4711, 0x00);	// LSB
                          
    //**********************************************
    // PLL Output Setting Uodate If Necessary
    //**********************************************
    i2cWrite_HM5065(0x0010, 0x02);	// CMD_POWEROFF
    i2cWrite_HM5065(0x00B5, 0x01);	// PLL3_div 1
    OSTimeDly(1);			//    
    //i2cWrite_HM5065(0x0010, 0x01);	// CMD_RUN
                          
    //**********************************************
    // Start C-AF         
    //**********************************************
                          
    //OSTimeDly(4);			// Sleep... Important !
    //i2cWrite_HM5065(0x070A 01);	// Start C-AF
}                         

void SetHM5065_720P_30FPS(void)
{
    DEBUG_SIU("SetHM5065_720P_30FPS()\n");

    SetHM5065_1280x960_30FPS();

    //i2cWrite_HM5065(0x0030, 0x11); //
#if 1
    // Max Frame Rate = 1/30 s
    i2cWrite_HM5065(0x0040, 0x01); // AB2
    i2cWrite_HM5065(0x0041, 0x0a); // ImageSize Manual
    i2cWrite_HM5065(0x0042, 0x05);
    i2cWrite_HM5065(0x0043, 0x00); // 1280
    i2cWrite_HM5065(0x0044, 0x02);
    i2cWrite_HM5065(0x0045, 0xD0); // 720
    i2cWrite_HM5065(0x00B2, 0x50); // TargetPLL=713MHz
    //i2cWrite_HM5065(0x00B2, 0x4f); // TargetPLL=713MHz
    i2cWrite_HM5065(0x00B3, 0xC9);
    //i2cWrite_HM5065(0x00B3, 0x84);
    i2cWrite_HM5065(0x00B4, 0x01);
    //i2cWrite_HM5065(0x00B5, 0x01); // PLL3_Div  // 30fps
    i2cWrite_HM5065(0x00B5, 0x02); // PLL3_Div  // 15fps
    i2cWrite_HM5065(0x0030, 0x12); // 0x00B5=0x02時,0x0030=0x11.
    
    i2cWrite_HM5065(0x03A0, 0x14);	// ArcticCCSigmaControl fMaximumCCSigma
    i2cWrite_HM5065(0x03B0, 0x60);	// ArcticCCSigmaControl fMaximumRing 
    i2cWrite_HM5065(0x03D0, 0x64);	// ArcticCCSigmaControl fMaximumScoring 
    
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x02); // CMD_PowerOff
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x01); // CMD_R
#endif
}

void SetHM5065_VGA_30FPS(void)
{
    DEBUG_SIU("SetHM5065_VGA_30FPS()\n");

    SetHM5065_1280x960_30FPS();

    // Max Frame Rate = 1/30 s
    i2cWrite_HM5065(0x0040, 0x01); // AB2
    i2cWrite_HM5065(0x0041, 0x04); // ImageSize VGA
    i2cWrite_HM5065(0x00B2, 0x50); // TargetPLL=713MHz
    i2cWrite_HM5065(0x00B3, 0xC9); 
    i2cWrite_HM5065(0x00B4, 0x01);
    i2cWrite_HM5065(0x00B5, 0x01); // PLL3_Div
    
    i2cWrite_HM5065(0x03A0, 0x14);	// ArcticCCSigmaControl fMaximumCCSigma
    i2cWrite_HM5065(0x03B0, 0x60);	// ArcticCCSigmaControl fMaximumRing 
    i2cWrite_HM5065(0x03D0, 0x64);	// ArcticCCSigmaControl fMaximumScoring
    
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x02); // CMD_PowerOff
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x01); // CMD_R
}

void SetHM5065_1080P_15FPS(void)
{
    DEBUG_SIU("SetHM5065_1080P_15FPS()\n");

    SetHM5065_1280x960_30FPS();

    i2cWrite_HM5065(0x0030, 0x11);
    i2cWrite_HM5065(0x0040, 0x00);
    i2cWrite_HM5065(0x0041, 0x0A);
    i2cWrite_HM5065(0x0042, 0x07);
    i2cWrite_HM5065(0x0043, 0x80);
    i2cWrite_HM5065(0x0044, 0x04);
    i2cWrite_HM5065(0x0045, 0x38);

    i2cWrite_HM5065(0x00B2, 0x50);
    i2cWrite_HM5065(0x00B3, 0xC9);  // set PLL to 713MHz
    //i2cWrite_HM5065(0x00B4, 0x01);  // 7.5 fps
    i2cWrite_HM5065(0x00B4, 0x02);  // 3.75 fps
    //i2cWrite_HM5065(0x00B4, 0x03);  // 2.5 fps
    i2cWrite_HM5065(0x00B5, 0x02);
    
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x02); // CMD_PowerOff
    OSTimeDly(12);
    i2cWrite_HM5065(0x0010, 0x01); // CMD_R
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
   //---------Run on Mclk=24 MHz--------------//
   #if(SYS_CPU_CLK_FREQ == 96000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0b; //MClk=288/12=24MHz
   #elif(SYS_CPU_CLK_FREQ == 108000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x08; //MClk=216/9=24MHz
   #elif( (SYS_CPU_CLK_FREQ == 160000000) || (SYS_CPU_CLK_FREQ == 162000000) )
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=480/20=24MHz 
   #elif(SYS_CPU_CLK_FREQ == 180000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=24MHz  
   #endif
      
#endif

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_FHD;
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 

            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                FrameRate   = 30;
                SetHM5065_720P_30FPS();
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240) )
            {
                FrameRate   = 30;
                SetHM5065_VGA_30FPS();
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
            {
                FrameRate   = 7;
                SetHM5065_1080P_15FPS();
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
            {
                FrameRate   = 7;
                SetHM5065_1080P_15FPS();
            }
			else
		    {
                FrameRate   = 30;
                SetHM5065_VGA_30FPS();
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
	return 1;
}


s32 siuSetFlicker50_60Hz(int flicker_sel)
{
	u8	data;
    int count;
#if 0
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
 #if SD_CARD_DISABLE

 #else
	/* set related EXIF IFD ... */
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

#if 1// After Fine Tune @Lucian 20080729
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
            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)  )
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
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
            {
	            Img_Width   = 1920;
	            Img_Height  = 1072;
	                        
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
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896) )
            {
	            Img_Width   = 1600;
	            Img_Height  = 896;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = (1920-1600)/2;
	            sensor_validsize.imgStr.y   = (1088-896)/2;

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

void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;
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

#endif	





