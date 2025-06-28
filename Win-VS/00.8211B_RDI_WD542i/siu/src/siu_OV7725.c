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
	
	2005/08/26	David Tsai	Create	
*/

#include "general.h"
 #if (Sensor_OPTION == Sensor_OV7725_VGA)	//Lisa 5M
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
    #include "mpeg4api.h"
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
 //----- Function Test -----//
 #if 0//(HW_BOARD_OPTION == RDI_CARMREC)
 #define USE_SENSOR_AE    0   //使用ov7725 內建 AE algorithm
 #else
 #define USE_SENSOR_AE    1   //使用ov7725 內建 AE algorithm
 #endif


/*--------- For new AE Algorithm---------*/
#if (AE_report==AE_report_Soft)
  #define AE_WSIZE_SEGMENT              5
  #define AE_HSIZE_SEGMENT              5
  #define AE_WIN_WIDTH                  160     //PANNEL_X  160
  #define AE_WIN_HEIGHT                 240     //PANNEL_Y    240
  #define AE_DOWN_SCALAR_X              4
  #define AE_DOWN_SCALAR_Y              8

  #define AE_WIN_WIDTH_TV               640     //PANNEL_X  160
  #define AE_WIN_HEIGHT_TV              480     //PANNEL_Y    240
  #define AE_DOWN_SCALAR_X_TV           32
  #define AE_DOWN_SCALAR_Y_TV           32
#endif



#define AE_AGC_SYNC_FRAMENUM              1

#define AE_SYNC_AGC_SW_POLEN              0    //AGC/SW set use polling or semephore

#define AECNT_PREVIEW                     5  
#define AECNT_VideoCap                    24 

#define SIU_CAPOEFINT_DISA	    0xffffd0ff //Register mask for disable siu interrrupt


// For new AE Table


#define Gain_Level 6
#define AE_E_Table_Num	    16
#define AE_Table_Row_Num	15


#define SW_VIDEOCLIP2PREVIEW_RATION    1000
#define SW_CAPTURE2PREVIEW_RATION      1000   //x1000, 




#define DAY_MODE_SWITCH_THR   (10*16)
#define NIGHT_MODE_SWITCH_THR (8*16)

#define DENOISE_ON_THR        (8*16)
#define MODESWITCH_THR        3

 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

//-------- Digital Zoom ---------//
/*Hardware limit: In preview/VideoClip mode, Y,X direction cannot do up-scaling.*/

ZOOM_FACTOR PreviewZoomParm_VGA[MAX_PREVIEW_ZOOM_FACTOR] = 
{
	    {{640, 480},  100}, //0
        {{592, 444},  108}, //1
        {{544, 408},  118}, //2
        {{512, 384},  125}, //3
        {{464, 348},  138}, //4
        {{432, 324},  148}, //5
        {{400, 300},  160}, //6
        {{368, 276},  174}, //7
        {{352, 264},  182}, //8

        {{320,  240},  200},//9        
        {{296,  222},  211},//10
        {{272,  204},  235},//11
        {{256,  192},  250},//12
        {{232,  174},  267},//13
        {{216,  162},  286},//14
        {{200,  150},  308},//15
        {{184,  138},  333},//16
        {{176,  132},  364},//17
        {{168,  128},  390},//18

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
/*
#define SIU_DEF_PIX_TBL_COUNT	3
SIU_COORD siuDefPixTbl[SIU_DEF_PIX_TBL_COUNT] = 
{
	{ 0x0055,	0x0055 },
	{ 0x00aa,	0x00aa },
	{ 0xffff,	0xffff },		
};
*/
__align(16) DEFECT_PIXEL_COORDINATE_SDV1 Coordinate_SDV1[Capture_TargetNum_SDV1];


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
#if (AE_report==AE_report_Soft)
  const u8 siuSetYtargetTab[9]={30, 43, 60, 85, 120, 170, 239, 240, 255};   //Lucian: modify to 120
  u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT];
#else
  const u8 siuSetYtargetTab[9]={8, 13, 21, 34, 55, 90, 148, 240, 245};  //Lucian: modify 65->50->42->55  
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
  #if(HW_BOARD_OPTION == D010_CARDVR) //For Car-DVR use
    u8 siu_dftAeWeight1[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT] = {
                                0, 0,  0,  0, 0,
                                0, 1,  0,  1, 0,
                                2, 4,  6,  4, 2,
                                4, 8, 12,  8, 4,
                                2, 6,  8,  6, 2
                          };

  #else
    u8 siu_dftAeWeight1[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT] = {
                                0, 1,  3,  1, 0,
                                1, 2,  5,  2, 1,
                                3, 4,  8,  4, 3,
                                1, 4,  6,  4, 1,
                                0, 3,  5,  3, 0
                          };
  #endif
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
 
extern u8 VideoRecFrameRate;

extern const s32 d50_IPU_CCM[10];
// OSD_AFreport, DefectPixel
#if(FACTORY_TOOL == TOOL_ON)
extern u8 config_mode_enable[LEVEL1_NODE_NUM*2];
#endif

extern s32 mp4_avifrmcnt, isu_avifrmcnt; /*BJ 0530 S*/	
extern s16 Preview_ipuCCM_DFT[];
extern s16 ipuCCM_WB_Matrix[];
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

#if (AE_report==AE_report_Soft)
   extern volatile s32 isu_idufrmcnt;
#endif

extern u8 uiMenuVideoSizeSetting;
extern u8 IsuOutAreaFlagOnTV;


#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 
void siu_RGBgamma_ON();
void siu_RGBgamma_OFF();



/* SW */
s32 siuGetAeYavg(u8*);

s32 siuInit(void);
s32 siuSensorInit(u8,u8);
s32 siuAeInit(void);
s32 siuSetOpticalBlack(u8, u8);
s32 siuSetDigitalGain(u8, u16);
s32 siuSetLensShading(SIU_LENS_SHADING*);
s32 siuSetRGB_PreGamma(u16*,u8*,u8*,u8*,u8*);
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
void Set_Sensor_UV_Saturation(void);

extern void ipuDenoise_ON();
extern void ipuDenoise_OFF();
extern void ipuYUVgamma_ON();
extern void ipuYUVgamma_OFF();
extern s32 ipuSetEdgeEnhance(int index);


#if HW_MD_SUPPORT
 extern void mduMotionDetect_init();  
#endif

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */  


#define R_Gain_MAX           1600
#define R_Gain_MIN           500
#define B_Gain_MAX           2200
#define B_Gain_MIN           800


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

	if(Rsum==0) Rsum=1;
	Rgain = (Gsum*1000)/Rsum;
	if(Rgain>R_Gain_MAX) Rgain=R_Gain_MAX;
	if(Rgain<R_Gain_MIN) Rgain=R_Gain_MIN;	
	
	Ggain = 1000;
	
	if(Bsum==0) Bsum=1;
	Bgain = (Gsum*1000)/Bsum;
	if(Bgain>B_Gain_MAX) Bgain=B_Gain_MAX;
	if(Bgain<B_Gain_MIN) Bgain=B_Gain_MIN;	
	
#if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
    
    if(mode == SIU_AWB_NORMAL)
    {
        AWBgain_Preview[0] = (s16)Rgain;
        AWBgain_Preview[1] = (s16)Ggain;
        AWBgain_Preview[2] = (s16)Bgain;    

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
    }
    else //force to gray 
    {
        AWBgain_Preview[0] = 1000;
        AWBgain_Preview[1] = 1000;
        AWBgain_Preview[2] = 1000;    

        Rgain  =  Rgain * ( 255/(255-OB_R));
        Grgain =  Ggain * ( 255/(255-OB_Gr));
        Gbgain =  Ggain * ( 255/(255-OB_Gb));
        Bgain  =  Bgain * ( 255/(255-OB_B));
        
        Preview_ipuCCM_DFT[0] = (s16)ipuCCM_WB_Matrix[0];
        Preview_ipuCCM_DFT[1] = (s16)ipuCCM_WB_Matrix[1];
        Preview_ipuCCM_DFT[2] = (s16)ipuCCM_WB_Matrix[2];
        Preview_ipuCCM_DFT[3] = (s16)ipuCCM_WB_Matrix[3];
        Preview_ipuCCM_DFT[4] = (s16)ipuCCM_WB_Matrix[4];
        Preview_ipuCCM_DFT[5] = (s16)ipuCCM_WB_Matrix[5];
        Preview_ipuCCM_DFT[6] = (s16)ipuCCM_WB_Matrix[6];
        Preview_ipuCCM_DFT[7] = (s16)ipuCCM_WB_Matrix[7];
        Preview_ipuCCM_DFT[8] = (s16)ipuCCM_WB_Matrix[8];
     }
      
     ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
     siuSetAwbGainOnPreGamma(Rgain,Grgain,Gbgain,Bgain,mode);
     siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
#else
      #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A))
            if(mode == SIU_AWB_NORMAL)
            {       
               AWBgain_Preview[0] = (s16)Rgain;
        	   AWBgain_Preview[1] = (s16)Ggain;
        	   AWBgain_Preview[2] = (s16)Bgain;
               
               ipuCCM_DFT[0]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[6])/256;
               ipuCCM_DFT[1]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[7])/256;
               ipuCCM_DFT[2]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[8])/256;

               ipuCCM_DFT[3]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[6])/256;
               ipuCCM_DFT[4]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[7])/256;
               ipuCCM_DFT[5]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[8])/256;

               ipuCCM_DFT[6]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[6])/256;
               ipuCCM_DFT[7]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[7])/256;
               ipuCCM_DFT[8]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[8])/256;     
            } 
            else
            {
               AWBgain_Preview[0] = 1000;
        	   AWBgain_Preview[1] = 1000;
        	   AWBgain_Preview[2] = 1000;
                
               ipuCCM_DFT[0]= ( (s32)ipuColorCorrTransform[0]*ipuCCM_WB_Matrix[0]+ (s32)ipuColorCorrTransform[1]*ipuCCM_WB_Matrix[3]+  (s32)ipuColorCorrTransform[2]*ipuCCM_WB_Matrix[6])/256;
               ipuCCM_DFT[1]= ( (s32)ipuColorCorrTransform[0]*ipuCCM_WB_Matrix[1]+ (s32)ipuColorCorrTransform[1]*ipuCCM_WB_Matrix[4]+  (s32)ipuColorCorrTransform[2]*ipuCCM_WB_Matrix[7])/256;
               ipuCCM_DFT[2]= ( (s32)ipuColorCorrTransform[0]*ipuCCM_WB_Matrix[2]+ (s32)ipuColorCorrTransform[1]*ipuCCM_WB_Matrix[5]+  (s32)ipuColorCorrTransform[2]*ipuCCM_WB_Matrix[8])/256;

               ipuCCM_DFT[3]= ( (s32)ipuColorCorrTransform[3]*ipuCCM_WB_Matrix[0]+ (s32)ipuColorCorrTransform[4]*ipuCCM_WB_Matrix[3]+  (s32)ipuColorCorrTransform[5]*ipuCCM_WB_Matrix[6])/256;
               ipuCCM_DFT[4]= ( (s32)ipuColorCorrTransform[3]*ipuCCM_WB_Matrix[1]+ (s32)ipuColorCorrTransform[4]*ipuCCM_WB_Matrix[4]+  (s32)ipuColorCorrTransform[5]*ipuCCM_WB_Matrix[7])/256;
               ipuCCM_DFT[5]= ( (s32)ipuColorCorrTransform[3]*ipuCCM_WB_Matrix[2]+ (s32)ipuColorCorrTransform[4]*ipuCCM_WB_Matrix[5]+  (s32)ipuColorCorrTransform[5]*ipuCCM_WB_Matrix[8])/256;

               ipuCCM_DFT[6]= ( (s32)ipuColorCorrTransform[6]*ipuCCM_WB_Matrix[0]+ (s32)ipuColorCorrTransform[7]*ipuCCM_WB_Matrix[3]+  (s32)ipuColorCorrTransform[8]*ipuCCM_WB_Matrix[6])/256;
               ipuCCM_DFT[7]= ( (s32)ipuColorCorrTransform[6]*ipuCCM_WB_Matrix[1]+ (s32)ipuColorCorrTransform[7]*ipuCCM_WB_Matrix[4]+  (s32)ipuColorCorrTransform[8]*ipuCCM_WB_Matrix[7])/256;
               ipuCCM_DFT[8]= ( (s32)ipuColorCorrTransform[6]*ipuCCM_WB_Matrix[2]+ (s32)ipuColorCorrTransform[7]*ipuCCM_WB_Matrix[5]+  (s32)ipuColorCorrTransform[8]*ipuCCM_WB_Matrix[8])/256;     
            } 
            ipuSetColorCorrMatrix(ipuCCM_DFT);
         #if  PA9003_AWB_GAIN_IN_SIU
            siuSetAWBGain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                            (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                         );
         #else
            ipuSetCfaAWBgain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                               (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                            );   
         #endif
      #else //PA9001D,PA9002D: Color transform 已內建進 Hardware.
        if(mode == SIU_AWB_NORMAL)
        {
            AWBgain_Preview[0] = (s16)Rgain;
    	    AWBgain_Preview[1] = (s16)Ggain;
    	    AWBgain_Preview[2] = (s16)Bgain;
            
        	Preview_ipuCCM_DFT[0] = (s16)( (d50_IPU_CCM[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R) );
        	Preview_ipuCCM_DFT[1] = (s16)(  d50_IPU_CCM[1]*AWBgain_Preview[1]/1000);
        	Preview_ipuCCM_DFT[2] = (s16)(  d50_IPU_CCM[2]*AWBgain_Preview[2]/1000);
        	Preview_ipuCCM_DFT[3] = (s16)(  d50_IPU_CCM[3]*AWBgain_Preview[0]/1000);
        	Preview_ipuCCM_DFT[4] = (s16)( (d50_IPU_CCM[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr) );
        	Preview_ipuCCM_DFT[5] = (s16)(  d50_IPU_CCM[5]*AWBgain_Preview[2]/1000);
        	Preview_ipuCCM_DFT[6] = (s16)(  d50_IPU_CCM[6]*AWBgain_Preview[0]/1000);
        	Preview_ipuCCM_DFT[7] = (s16)(  d50_IPU_CCM[7]*AWBgain_Preview[1]/1000);
        	Preview_ipuCCM_DFT[8] = (s16)( (d50_IPU_CCM[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B) );
         }
         else
         {
            AWBgain_Preview[0] = 1000;
    	    AWBgain_Preview[1] = 1000;
    	    AWBgain_Preview[2] = 1000;
        
            Preview_ipuCCM_DFT[0] = (s16)( (ipuCCM_WB_Matrix[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R) );
        	Preview_ipuCCM_DFT[1] = (s16)(  ipuCCM_WB_Matrix[1]*AWBgain_Preview[1]/1000);
        	Preview_ipuCCM_DFT[2] = (s16)(  ipuCCM_WB_Matrix[2]*AWBgain_Preview[2]/1000);
        	Preview_ipuCCM_DFT[3] = (s16)(  ipuCCM_WB_Matrix[3]*AWBgain_Preview[0]/1000);
        	Preview_ipuCCM_DFT[4] = (s16)( (ipuCCM_WB_Matrix[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr) );
        	Preview_ipuCCM_DFT[5] = (s16)(  ipuCCM_WB_Matrix[5]*AWBgain_Preview[2]/1000);
        	Preview_ipuCCM_DFT[6] = (s16)(  ipuCCM_WB_Matrix[6]*AWBgain_Preview[0]/1000);
        	Preview_ipuCCM_DFT[7] = (s16)(  ipuCCM_WB_Matrix[7]*AWBgain_Preview[1]/1000);
        	Preview_ipuCCM_DFT[8] = (s16)( (ipuCCM_WB_Matrix[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B) );
         }
    	 ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
      #endif	

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
    const static u16 SW_Prev_60Hz[AE_TAB_ENTRY_NUM]={
              1,	  1,	  1,	  1,	  1,	  1,
              2,	  2,	  2,	  3,	  3,	  3,
              4,	  4,	  5,	  5,	  6,	  7,
              8,	  9,	 10,	 11,	 13,	 14,
             16,	 18,	 20,	 23,	 25,	 28,
             32, 	 36,	 40,	 45,	 51,	 57,
             64, 	 72,	 81,	 90,	101,	114,
            128,	128,	128,	128,	128,	128,
            255, 	255,	255,	383,	383,	383,
            510, 	510,	510,	510,	510,	510,
            510, 	510,	510,	510,	510,	510,
            510, 	510,	510,	510,	510,	510,
            510, 	510,	510,	510,	510,	510,
            510,	510,	510,	510,	510,	510,
            510,	510,	510,	510,	510,	510
    };
    
    const static u8 AG_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
            0,	2,	4,	7,	10,	13, 
            0,	2,	5,	0,	 2,	 4, 
            0,	2,	0,	2,	 0,	 0, 
            0,	0,	0,	1,	 0,	 0, 
            0,	0,	0,	0,	 0,	 0, 
            0, 	0,	0,	0,	 0,	 0, 
            0, 	0,	0,	0,	 0,	 0, 
            0,	2,	4,	7,	10,	13, 
            0, 	2,	5,	0,	 2,	 4, 
            0, 	2,	4,	7,	10,	13, 
           16, 18, 20, 23,	26,	29, 
           48, 50, 52, 55,	58,	61, 
          112,114,116,119, 122,125, 
          240,242,244,247, 250,253, 
          255,255,255,255, 255,255 
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0
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
          0, 0, 0, 0, 0, 0 
    };
    
    const static u16 DBTab_Prev_60Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,        1243,        1090,         972,         875,      
        1048,        1371,        1187,        1048,         937,         574,
        1048,         937,        1048,         574,        1371,        1187,
        1048,         937,        1387,         947,         659,        1187,
        1048,         937,        1243,         742,        1008,        1187,
        1048,         937,        1048,        1113,         989,        1031,
        1047,        1048,         937,        1026,        1076,        1031,
        1047,         937,        1243,        1091,         971,         841,
        1048,        1371,        1199,        1048,         937,         562,
        1048,         937,        1243,        1090,         972,         875,
        1048,         937,        1243,        1090,         972,         875,
        1048,         937,        1243,        1091,         971,         875,
        1048,         937,        1243,        1091,         971,         875,
        1048,         937,        1243,        1091,         971,         593,
           0,           0,           0,           0,           0,           0 

    };
    //--------------------------Preview/Capture Mode,50Hz----------------------------//
    const static u16 SW_Prev_50Hz[AE_TAB_ENTRY_NUM]={
             1,	  1,	  1,	  1,	  1,	  1,
             2,	  2,	  2,	  3,	  3,	  4,
             5,	  5,	  6,	  6,	  8,	  9,
            10,	 11,	 13,	 14,	 16,	 18,
            19,	 21,	 24,	 27,	 30,	 34,
            38,	 43,	 48,	 54,	 60,	 68,
            77,	 86,	 97,	109,	122,	137,
           153,	153,	153,	153,	153,	153,
           306,	306,	306,	459,	459,	459,
           612,	612,	612,	612,	612,	612,
           612,	612,	612,	612,	612,	612,
           612,	612,	612,	612,	612,	612,
           612,	612,	612,	612,	612,	612,
           612,	612,	612,	612,	612,	612,
           612,	612,	612,	612,	612,	612,
    };
    
    const static u8 AG_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
            0,  2,  4,  7,  10,  13, 
            0,  2,  5,  0,   3,   1, 
            0,  2,  1,  3,   0,   0, 
            0,  1,  0,  0,   0,   0, 
            0,  0,  0,  0,   0,   0, 
            0,  0,  0,  0,   0,   0, 
            0,  0,  0,  0,   0,   0, 
            0,  2,  4,  7,  10,  13, 
            0,  2,  5,  0,   2,   4, 
            0,  2,  4,  7,  10,  13, 
           16, 18, 20, 23,  26,  29, 
           48, 50, 52, 55,  58,  61, 
          112,114,116,119, 122, 125, 
          240,242,244,247, 250, 253, 
          255,255,255,255, 255, 255     
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0
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
          0, 0, 0, 0, 0, 0
    };
    
    const static u16 DBTab_Prev_50Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,        1243,        1090,         972,         875,
        1048,        1371,        1187,        1529,        1569,        1446,
        1048,        1113,         989,        1030,        1048,         937,
        1387,         947,         659,        1187,        1048,         481,
         890,        1188,        1047,         937,        1114,         989,
        1099,         979,        1047,         938,        1113,        1105,
         984,        1070,        1037,        1003,        1031,         982,
        1048,         937,        1243,        1091,         971,         876,
        1047,        1371,        1188,        1048,         937,         574,
        1047,         937,        1243,        1091,         971,         876,
        1047,         937,        1244,        1090,         971,         876,
        1047,         938,        1243,        1090,         971,         876,
        1048,         937,        1243,        1090,         971,         876,
        1048,         937,        1243,        1090,         972,         593,
           0,           0,           0,           0,           0,           0
    };
    //-------------------------------Video Mode,60Hz----------------------------------------------------//
    
      const static u16 SW_Video_60Hz[AE_TAB_ENTRY_NUM]={
        1,	  1,	  1,	  2,	  2,	  2,
        2,	  2,	  2,	  3,	  3,	  4,
        5,	  6,	  6,	  7,	  8,	  9,
       10,	 12,	 12,	 14,	 16,	 18,
       21,	 23,	 26,	 30,	 34,	 38,
       43, 	 48,	 54,	 61,	 68,	 76,
       85, 	 95,	107,	120,	135,	151,
      170,	170,	170,	170,	170,	170,
      340, 	340,	340,	340,	340,	340,
      340, 	340,	340,	340,	340,	340,
      340, 	340,	340,	340,	340,	340,
      680, 	680,	680,	680,	680,	680,
      680, 	680,	680,	680,	680,	680,
      680,	680,	680,	680,	680,	680,
      680,	680,	680,	680,	680,	680
    };
    
    const static u8 AG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
            6,	9, 12,  0,	 2,	 4,
            6,	9, 12,	5,	 7,	 3,
            1,	0,	2,	1,	 1,	 1,
            1,	0,	2,	1,	 1,	 1,
            0,	0,	0,	0,	 0,	 0,
            0, 	0,	0,	0,	 0,	 0,
            0, 	0,	0,	0,	 0,	 0,
            0,	2,	4,	7,	10,	13,
            0, 	2,	4,	7,	10,	13,
            16, 18, 20, 23,	26,	29,
           48, 50, 52, 54,	57,	60,
           48, 50, 52, 54,	57,	60,
          112,114,116,119, 122,125,
          240,242,244,247, 250,253,
          255,255,255,255, 255,255          
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0 
    };
    
    const static u16 DBTab_Video_60Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1137,        1008,        1188,        1048,         937,         848,
        1137,        1008,        1047,         809,         860,         995,
        1083,        1047,         863,        1187,        1048,         937,
        1083,        1047,         863,        1188,        1047,         832,
         809,        1091,        1272,        1114,         989,        1099,
         979,        1047,        1085,         966,         989,         995,
         990,        1058,        1020,        1047,         996,        1055,
        1047,         937,        1243,        1091,         971,         876,
        1047,         937,        1243,        1091,         971,         876,
        1047,         937,        1244,        1090,         971,         876,
        1047,         938,         847,        1137,        1008,        1188,
        1048,         937,         847,        1137,        1008,        1188,
        1048,         937,        1243,        1090,         972,         875,
        1048,         937,        1243,        1090,         972,         593,
           0,           0,           0,           0,           0,           0
    };
    //--------------------------Video Mode,50Hz----------------------------//
    const static u16 SW_Video_50Hz[AE_TAB_ENTRY_NUM]={
           1,	  1,	  2,	  2,	  2,	  2,
           3,	  3,	  4,	  4,	  5,	  5,
           6,	  6,	  8,	  8,	 10,	 10,
          13,	 14,	 14,	 14,	 21,	 23,
          26,	 29,	 33,	 37,	 41,	 46,
          51,	 57,	 64,	 72,	 81,	 91,
         102,	114,	128,	144,	162,	181,
         204,	204,	204,	204,	204,	204,
         408,	408,	408,	408,	408,	408,
         408,	408,	408,	408,	408,	408,
         816,	816,	816,	816,	816,	816,
         816,	816,	816,	816,	816,	816,
         816,	816,	816,	816,	816,	816,
         816,	816,	816,	816,	816,	816,
    };
    
    const static u8 AG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
       10, 13,  0,  2,   4,   7,
        1,  3,  0,  2,   0,   2,
        1,  3,  0,  2,   0,   2,
        0,  1,  3,  5,   0,   0,
        0,  0,  0,  0,   0,   0,
        0,  0,  0,  0,   0,   0,
        0,  0,  0,  0,   0,   0,
        0,  2,  4,  7,  10,  13,
        0,  2,  4,  7,  10,  13,
       16, 18, 20, 23,  26,  29,
       48, 50, 52, 55,  58,  61,
       48, 50, 52, 55,  58,  61,
      112,114,116,119, 122, 125,
      240,242,244,247, 250, 253,
      255,255,255,255, 255, 255        
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0
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
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0
    };
    
    const static u16 DBTab_Video_50Hz[AE_TAB_ENTRY_NUM+1]={
           0,
         972,         875,        1048,         937,        1243,         918,
         989,        1030,        1048,         937,        1048,        1113,
         989,        1030,        1048,         937,        1048,        1286,
        1198,         989,         890,        1188,         809,        1091,
         971,        1149,        1018,         913,        1023,         918,
         989,        1031,        1047,        1048,        1035,        1015,
         989,        1031,        1047,        1048,         986,        1064,
        1048,         937,        1243,        1091,         971,         875,
        1048,         937,        1243,        1091,         971,         875,
        1048,         937,        1243,        1091,         971,         876,
        1047,         937,        1243,        1091,         971,         876,
        1047,         937,        1243,        1091,         971,         876,
        1047,         937,        1244,        1090,         971,         876,
        1047,         938,        1243,        1090,         971,         594,
           0,           0,           0,           0,           0,           0
    };
      //-----------------------------------------------------------------------------------//
      for(i=0;i<AE_TAB_ENTRY_NUM;i++)
	  {
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DB_p		= DBTab_Prev_60Hz[i+1];
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DB_n		= DBTab_Prev_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].EL			= SW_Prev_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].AG.gain	    = AG_Prev_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].AG.mul		= MUL_Prev_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].F_num		= 0;
		AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DG			= DG_Prev_60Hz[i];
	  }
	  
	  for(i=0;i<AE_TAB_ENTRY_NUM;i++)
	  {
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DB_p		= DBTab_Video_60Hz[i+1];
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DB_n		= DBTab_Video_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].EL			= SW_Video_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].AG.gain	    = AG_Video_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].AG.mul		= MUL_Video_60Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].F_num		= 0;
		AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DG			= DG_Video_60Hz[i];
	  }
	  
	  for(i=0;i<AE_TAB_ENTRY_NUM;i++)
	  {
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DB_p		= DBTab_Prev_50Hz[i+1];
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DB_n		= DBTab_Prev_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].EL			= SW_Prev_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].AG.gain	    = AG_Prev_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].AG.mul		= MUL_Prev_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].F_num		= 0;
		AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DG			= DG_Prev_50Hz[i];
	  }
	  
	  for(i=0;i<AE_TAB_ENTRY_NUM;i++)
	  {
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DB_p		= DBTab_Video_50Hz[i+1];
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DB_n		= DBTab_Video_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].EL			= SW_Video_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].AG.gain	    = AG_Video_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].AG.mul		= MUL_Video_50Hz[i];
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].F_num		= 0;
		AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DG			= DG_Video_50Hz[i];
	  }


}

/* SW 0107 E */

void SetSensorSW(u16 EL)
{
	
        i2cWrite_SENSOR(0x08, (EL>>8) & 0x0ff); //AE=128
        i2cWrite_SENSOR(0x10, EL & 0x0ff); 
}

void SetSensor_DigitalGain(u8 ggain)//set globe Digital gain
{
	

}

void SetSensor_AnalogGain(Analog_Gain ggain)//set globe Analog gain
{
	 i2cWrite_SENSOR(0x00, ggain.gain & 0x0ff); 
}

void siuAdjustAE(u16 cur_index)
{
	u32	EL;
	u8	DG;
	u16	data;
	Analog_Gain AG;

    //Inital state in Preview mode
    
    if(siuOpMode == SIUMODE_MPEGAVI)
    {
        if(cur_index>AEVideoCurSet_Max)
            cur_index=AEVideoCurSet_Max;
        EL	= (AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].EL * SW_VIDEOCLIP2PREVIEW_RATION + 500) / 1000;
    	AG	= AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].AG;
    	DG	= AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].DG;
    }
    else 
    {
    	EL	= AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].EL;
    	AG	= AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].AG;
    	DG	= AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].DG;
    }


	SetSensorSW(EL);
	SetSensor_AnalogGain(AG);
	SetSensor_DigitalGain(DG);

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
                SIU_DEF_PIX_DISA |
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
        		SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
//        		SIU_CAPT_ENA |                          //SIU out enable
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
         #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
               SIU_RGB_GAMMA_ENA |
         #else
               SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
         #endif

         #if(CHIP_OPTION == CHIP_PA9001D)
                SIU_AE_ENA |                            //AE report enable
         #endif
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
         (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
         (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
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
	siuSetLensShading(&lensShading);

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
//        		SIU_CAPT_ENA |                          //SIU out enable
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		// Sensor data is not shifted 2 bits. 
        		SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit) 	// Sensor data is shifted 2 bits.     	
        	  	SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	// Sensor data is shifted 4 bits.     	
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
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
         #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
                SIU_RGB_GAMMA_ENA |
         #else
                SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
         #endif

         #if(CHIP_OPTION == CHIP_PA9001D)
        		SIU_AE_ENA |                            //AE report enable
         #endif
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
         (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
         (CHIP_OPTION == CHIP_A1026A))
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
	siuSetLensShading(&lensShading);

	// Set Defect Pixel Table 
	SiuDefPixTbl = (u32)Coordinate_SDV1;
	
	// Set RGB pre-Gamma Table
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
	 				
		    
	// Set AE block weighting
	siuSetAeBlkWeight(siu_dftAeWeight1, &(aeWin.weightacc));

	
    
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
    int FrameRate=30;
	
	
#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;
#endif
    
	switch(siuSensorMode)   	
    {
    #if USE_SENSOR_AE
      case SIUMODE_CAPTURE: //Capture mode
         break;
    #else
       case SIUMODE_CAPTURE: //Capture mode
    #endif
       case SIUMODE_PREVIEW: //Preview mode 
       case SIUMODE_MPEGAVI: 
         
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
		    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=27MHz 
		   #endif		    		 
		 #endif

    	   i2cWrite_SENSOR(0x12, 0x0080); //SCCB Register reset
           for(i=0;i<10000;i++);
           i2cWrite_SENSOR(0x12, 0x0000);

           i2cRead_SENSOR(0x0a, &data);
           DEBUG_SIU("PID_M=0x%x\n",data);
           i2cRead_SENSOR(0x0b, &data);
           DEBUG_SIU("PID_L=0x%x\n",data);
                      
           i2cWrite_SENSOR(0x3d, 0x0003); //DC offset compensation for analog process
           i2cWrite_SENSOR(0x12, 0x0011); //Sensor raw selected
             
           i2cWrite_SENSOR(0x64, 0x0083); 
           i2cWrite_SENSOR(0x66, 0x0000); 
           i2cWrite_SENSOR(0x67, 0x0003);
           
        #if(HW_BOARD_OPTION == D010_CARDVR)
           i2cWrite_SENSOR(0x0d, 0x71);    //PLL 4x,AEC Low 2/3 window for car DVR. pixel clock= 24*4/4/2=12MHz
           i2cWrite_SENSOR(0x11, 0x0001);  //Internal clock
        #else
           i2cWrite_SENSOR(0x0d, 0x41);    //PLL 4x,AEC full window
           i2cWrite_SENSOR(0x11, 0x0001);  //Internal clock
        #endif
           i2cWrite_SENSOR(0x14, 0x0030);  //Max AGC to 16x
           
           if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )     //60Hz
           {
              i2cWrite_SENSOR(0x22, 128);    
              i2cWrite_SENSOR(0x23, 0x0003); // 01/03/07/0f for 60/30/15/7.5fps
              i2cWrite_SENSOR(0x33, 0x0000); //Insert dummy line. -->7
              i2cWrite_SENSOR(0x34, 0);
              i2cWrite_SENSOR(0x2d, 0x0000); //insert dummy line to Vsync.
           }
           else //50Hz
           {
              i2cWrite_SENSOR(0x22, 153); 
              i2cWrite_SENSOR(0x23, 0x0003); //01/03  for 50/25 fps
              i2cWrite_SENSOR(0x33, 102);    //Insert dummy line. -->102
              i2cWrite_SENSOR(0x34, 0);
              i2cWrite_SENSOR(0x2d, 0x0000); //insert dummy line to Vsync.
           }
           i2cWrite_SENSOR(0x2b, 0x0000); 
           i2cWrite_SENSOR(0x2a, 0x0000);
            
           i2cWrite_SENSOR(0x42, 0x007F);    //BLC Blue Channel Target Value
           i2cWrite_SENSOR(0x4d, 0x0009);    //Analog Fix Gain Amplifier: B x 1.5, R x1.25
                    
           i2cWrite_SENSOR(0x15, 0x0002);     //Vsync negtive

           //------Windowing-----//
           i2cWrite_SENSOR(0x17, 0x0022); 
           i2cWrite_SENSOR(0x18, 0x00a1); 
           i2cWrite_SENSOR(0x19, 0x0006);
           i2cWrite_SENSOR(0x1a, 0x00f3); 
           i2cWrite_SENSOR(0x29, 0x00a1); 
           i2cWrite_SENSOR(0x2c, 0x00f3); 
           
           
           i2cWrite_SENSOR(0x32,0x00); 

           #if SENSOR_ROW_COL_MIRROR  
               i2cWrite_SENSOR(0x0c,0x00); //rotate 180 degree
           #else
               i2cWrite_SENSOR( 0x0c, 0xc0); //rotate 0 degree
           #endif
           //----------------------------------//
           //----AE Contorl---//
           i2cWrite_SENSOR(0x24, 0x0048); //Up limit
           i2cWrite_SENSOR(0x25, 0x0038); //Bottom limit
           i2cWrite_SENSOR(0x26, 0x00B2); 
                     
            
           //----Lens Shading----//
           i2cWrite_SENSOR(0x46, 0x05); //Lens correction enable; apply different coeff. on RGB
           i2cWrite_SENSOR(0x47, 0x00); 
           i2cWrite_SENSOR(0x48, 0x00); 
           i2cWrite_SENSOR(0x49, 0x10); //G gain
           i2cWrite_SENSOR(0x4a, 0x00);  
           i2cWrite_SENSOR(0x4b, 0x10); //B gain
           i2cWrite_SENSOR(0x4c, 0x16); //R gain 

         #if USE_SENSOR_AE
           i2cWrite_SENSOR(0x13, 0xfd); //Disalbe AE/AWB/AGC
         #else
           i2cWrite_SENSOR(0x13, 0x00); //Disalbe AE/AWB/AGC
         #endif
          break;
      
    } 
    
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
  #if USE_SENSOR_AE
  #else
    siuAdjustAE(AECurSet);	
  #endif
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
	   
    #if USE_SENSOR_AE
        siuFrameCount++;	
       #if MOTIONDETEC_ENA
          frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  MD_period_Video : MD_period_Preview;
       #else
          frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  AECNT_VideoCap : AECNT_PREVIEW;
       #endif
		if (siuFrameCount > frameAct_Thd)
		{
		    if(siuOpMode == SIUMODE_CAPTURE) 
                OSSemPost(siuCapEvt);
            else
                OSSemPost(siuSemEvt);           
            siuFrameCount=0;
			
		}
    #else
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
		if (siuAeEnable == 1)
	        siuFrameCount++;
		else
		    siuFrameCount = 0;
        frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  AECNT_VideoCap : AECNT_PREVIEW;
		if (siuFrameCount > frameAct_Thd)
		{
			/* signal event semaphore */
			switch (siuOpMode) 
			{
                    case SIUMODE_PREVIEW:		                   
                    case SIUMODE_MPEGAVI:
                        if ((siuPreviewFlag == 1) && (uiDigZoomSetFlag == 0))
                        {       //uiDigZoomSetFlag: 避免Digital Zoom在調整sensor window size時, 啟動AE control.
                                siuPreviewFlag = 0;
                                OSSemPost(siuSemEvt);
                        }        
                        break;
                    case SIUMODE_CAPTURE:
                        if ((siuCaptureFlag == 1) && (siuPreFlashCount == 3) && (siuFrameCount == siuFlashGetYFrameCount))                                        
                        {
		
		                        SiuSensCtrl &= SIU_CAPOEFINT_DISA; // disable siu interrupt and data output.
                                siuCaptureFlag = 0;
                                OSSemPost(siuCapEvt);
                        }        
                        break;
		     }
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
        #if( (HW_BOARD_OPTION==GOS_DVRBOX) ||  (HW_BOARD_OPTION==VER100_CARDVR) )
            #if ISU_OVERLAY_ENABLE
            {
                static  u32 i   = 0;
                if((i % 2) == 0)
                    sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
                i++;
            }
            #endif
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
#if (AE_report==AE_report_Soft)
//-----AE_report------//by lisa_0425_S
/*

Routine Description:

	Get AE Window Y  average by software.

Arguments:

	siuBlkWeight - The weight of  AE window.

Return Value:

	0 - Failure.
	1 - Success.

*/
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
    u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT]; 
    
       
    blk_vidx = 0;
    nextblk_vidx= (AE_WIN_HEIGHT_TV/AE_HSIZE_SEGMENT); 

    i=((isu_idufrmcnt-1) % 3);
    if(i==0)
        img_ptr=(u8*) PKBuf0;
    else if (i==1)
        img_ptr=(u8*) PKBuf1;
    else
        img_ptr=(u8*) PKBuf2;

    memset(AE_blk_YSum,0,AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT*sizeof(u32));

    for (j= 0; j < AE_WIN_HEIGHT_TV; j+=AE_DOWN_SCALAR_Y_TV)     
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
        for (i=0; i< (AE_WIN_WIDTH_TV*2); i+=(2*AE_DOWN_SCALAR_X_TV) )
        {
                if (i>= nextblk_hidx )   
                {
                        AE_blk_YSum[(blk_vidx)*(AE_WSIZE_SEGMENT)+blk_hidx] += Sum;
                        Sum=0;
                        blk_hidx++;
                        nextblk_hidx += (AE_WIN_WIDTH_TV*2/AE_WSIZE_SEGMENT);
                }                        
                Sum += (*pp + *(pp+1) + 1)/2;
                pp+=(2*AE_DOWN_SCALAR_X_TV) ;
        }
    }
    for(k=0;k<(AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT);k++)
    {
       ae_WeightedYsum += (AE_blk_YSum[k] ) * (*(siuBlkWeight+k));
       aeweight_sum +=*(siuBlkWeight+k);
    }
    ae_WinSize=(AE_WIN_HEIGHT_TV/AE_DOWN_SCALAR_Y_TV)*(AE_WIN_WIDTH_TV/AE_DOWN_SCALAR_X_TV);
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
                Sum += (*pp+ *(pp+1) + 1)/2;
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
//-----AE_report------//by lisa_0425_E
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
	exifSetSubjectDistance(subjectDistance);	/* in unit of centimeter */

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
	
	exifSetExposureBiasValue(ExposureBiasValueTab[expBiasValue]);		/* expBiasValue = Exposure Bias Value * 10 */		
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
//        		SIU_CAPT_ENA |                          //SIU out enable
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        		SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit) 	/* Sensor data is shifted 2 bits. */    	
        	  	SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
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
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
           #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
                SIU_RGB_GAMMA_ENA |
           #else
                SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
           #endif
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
#if 0
    SiuSensCtrl |=	SIU_DST_SDRAM;                 //SIU to SDRAM 
#else
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
		//SIU_CAPT_ENA |                //set 1 for capture.
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        	SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)	/* Sensor data is shifted 2 bits. */    	
        	SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        	  	SIU_DATA_8b |         	  	        	
#endif    
		SIU_PIX_CLK_ACT_RIS |           //positive edge latch
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif  		
		SIU_OB_COMP_ENA |              //OB compensation enable
		SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
 #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
        SIU_RGB_GAMMA_ENA |
 #else
        SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
 #endif

 #if(CHIP_OPTION == CHIP_PA9001D)
		SIU_AE_ENA |		            //AE report disable
 #endif
	 	SIU_TEST_PATRN_DISA |           //Test pattern disable
		SIU_SINGLE_CAPTURE_DISA;  	

#endif

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
         (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
         (CHIP_OPTION == CHIP_A1026A))
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
	siuSetLensShading(&lensShading);

	/* Set Defect Pixel Table */
	SiuDefPixTbl = (u32)Coordinate_SDV1;
	
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
		//SIU_CAPT_ENA |                //set 1 for capture.
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        	SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)	/* Sensor data is shifted 2 bits. */    	
        	SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        	  	SIU_DATA_8b |         	  	        	
#endif    
		SIU_PIX_CLK_ACT_RIS |           //positive edge latch
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B)|| (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif  		
		SIU_OB_COMP_ENA |              //OB compensation enable
		SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
		#if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
           SIU_RGB_GAMMA_ENA |
        #else
           SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        #endif
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

	/* Set Defect Pixel Table */
	SiuDefPixTbl = (u32)Coordinate_SDV1;
	
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
		//SIU_CAPT_ENA |                //set 1 for capture.
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
		#if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
           SIU_RGB_GAMMA_ENA |
        #else
           SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        #endif
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

	/* Set Defect Pixel Table */
	SiuDefPixTbl = (u32)Coordinate_SDV1;
	
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
    
    /*
    // initialize sensor
    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.    
	siuSensorInit(SIUMODE_MPEGAVI,zoomFactor); 
	siuAdjustAE(AECurSet); //Lucian: Preview->Video, AE index transfer, Set Sensor Again.
    */
	//DEBUG_SIU("VideoMode: EPL=%d, GC=%d\n", siu_aeinfo.Exposure_Line, siu_aeinfo.Gain_Code);
#if(HW_BOARD_OPTION==SALIX_SDV)
	OSTimeDly(3);
#else
    OSTimeDly(1);
#endif
	siuVideoClipInit(zoomFactor);	
    siuPreviewFlag = 1;
	

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
//        		SIU_CAPT_ENA |                          //SIU out enable
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        		SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit) 	/* Sensor data is shifted 2 bits. */    	
        	  	SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        	  	SIU_DATA_8b |         	  	        	  	
#endif        	  
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
          (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
          (CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif          		
        		SIU_OB_COMP_ENA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
            #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
                SIU_RGB_GAMMA_ENA |
            #else
                SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
            #endif
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
 #if USE_SENSOR_AE
    u16 Again_x16,temp;
   #if 0
    u16 EL;
   #endif
    static int  Mode=SIU_DAY_MODE;
    static unsigned int DayCount=0;
    static unsigned int NightCount=0;
    static int El_Max_1_60=1;
 #endif
    //-----------------//
	while (1)
	{
		OSSemPend(siuSemEvt, OS_IPC_WAIT_FOREVER, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: siuSemEvt is %d.\n", err);
			return ;
		}

        if(siuOpMode == SIUMODE_CAPTURE)
            continue;
        
	    siuPreviewFlag = 1;	
		//-------- Auto-Exposure------- //
    #if USE_SENSOR_AE
        //No use build-in AE algorithm
    #else
		#if(HW_BOARD_OPTION==ALTERA_FPGA)

		#else
		if (siuAeEnable)			
		{	
			siuAutoExposure();
        }
		#endif	
    #endif
        //-----Motion detection-----//

    #if HW_MD_SUPPORT
        if(MotionDetect_en)
           MotionDetect_API(MD_SIU_ID);
    #endif

		//------ auto-white-balance---- //
    #if USE_SENSOR_AE
      #if 0
        i2cRead_SENSOR(0x08, &EL);
        EL <<= 8;
        i2cRead_SENSOR(0x10, &temp);
        EL=EL | temp;
        DEBUG_SIU("EL=%d\n",EL);
      #endif   
         #if 0//(HW_BOARD_OPTION == RDI_CARMREC)
             Set_Sensor_UV_Saturation();
         #else
            i2cRead_SENSOR(0x0, &temp);
            Again_x16= ((temp>>4) + 1)*(16+ (temp & 0x0f));
            //DEBUG_SIU("Again_x16=%d,El_Max_1_60=%d\n",Again_x16,El_Max_1_60);
            if(Again_x16 >= DENOISE_ON_THR)
               ipuDenoise_ON(); //AGC >= 8x: Denoise  enable
            else
               ipuDenoise_OFF();

            //-----Exposure time 1/30 and 1/60 switch -----//
         #if 1
            if(VideoRecFrameRate   == MPEG4_VIDEO_FRAMERATE_60)
            {
               i2cWrite_SENSOR(0x23, 3);
            }
            else
            {
                if(El_Max_1_60 == 1)
                {
                    if(Again_x16 >= 8*16)
                    {
                       El_Max_1_60=0;
                       i2cWrite_SENSOR(0x23, 3);
                    }
                    else
                    {
                       El_Max_1_60=1;
                       i2cWrite_SENSOR(0x23, 1);
                    }
                }
                else
                {
                    if(Again_x16 < 3*16)
                    {
                       El_Max_1_60=1;
                       i2cWrite_SENSOR(0x23, 1);
                    }
                    else
                    {
                       El_Max_1_60=0;
                       i2cWrite_SENSOR(0x23, 3);
                    }
                }
            }
         #endif
            //----Mode select----//
            if(Mode ==SIU_DAY_MODE)
            {                  
               if( Again_x16 >= DAY_MODE_SWITCH_THR )
                  NightCount ++;
               else
                  NightCount =0;

               if(NightCount > MODESWITCH_THR) 
               {
                  Mode =SIU_NIGHT_MODE;  //switch to Night mode
                  DayCount =0;
               }
            }
            else //Night mode
            {
               if( Again_x16 < NIGHT_MODE_SWITCH_THR )
                  DayCount ++;
               else
                  DayCount =0; 

               if(DayCount > MODESWITCH_THR)
               {
                  Mode =SIU_DAY_MODE; //switch to Day mode
                  NightCount=0;
               }
            }

            if(Mode ==SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
    		{  //夜間模式
               ipuYUVgamma_OFF();
               siuAwbGrayWorld(SIU_AWB_FORCEGRAY);
               ipuSetEdgeEnhance(2);
    		}
            else
            {  //日間模式
    		   siuAwbGrayWorld(SIU_AWB_NORMAL);
               if(Again_x16>=DENOISE_ON_THR) //AGC > 8x
                 ipuSetEdgeEnhance(1);
               else
                 ipuSetEdgeEnhance(0);
               ipuYUVgamma_ON();
            }
        #endif
    #else
        #if 0//(HW_BOARD_OPTION == RDI_CARMREC)
            Set_Sensor_UV_Saturation();
        #else
            siuAwbGrayWorld(SIU_AWB_NORMAL);
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
//    gpioSetLevel(1, 1,1 );	//lisa 5M reset
  if(sysTVOutOnFlag) //TV-out
  {
     if(IsuOutAreaFlagOnTV==FULL_TV_OUT)
        siuGetAeYavg_Soft_TV_Full(siu_dftAeWeight1);
     else
        siuGetAeYavg_Soft_TV_Half(siu_dftAeWeight1);
  }
  else
     siuGetAeYavg_Soft_Panel(siu_dftAeWeight1);
//    gpioSetLevel(1, 1,0 );
//-----AE_report------//by lisa_0425_E
#else
	for (i = 0; i < 25; i++)
		siuGetAeWinYsum(i, &aeWinYsum[i]);
		
	/*	目前algorithm 並沒有用到此參數,暫時拿掉.
	for (i = SIU_AE_HIST_OUT; i <= SIU_AE_HIST_INN; i++)
		siuGetAeHist(i, &AEWinRingHist[i]);
	*/
	
	/* SW */		
	siuGetAeYavg(siu_dftAeWeight1);
#endif
	//	Hist_WgtAdj(); //20051226
	if (siuOpMode != SIUMODE_CAPTURE)
	{
		siuAE_ExposureControl();        
        if(AECurSet>=72)
            ipuDenoise_ON();
        else
            ipuDenoise_OFF();
	}
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
	u16 ae_logYtarget = AE_Log_Mapping_Table[ siuSetYtargetTab[3] ]; //x 1024
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
	    DEBUG_SIU("\nAECurSet=%d,ae_abs_logYdiff=%d\n",AECurSet,ae_abs_logYdiff);
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
            #if 1
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
            #endif
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
            #if 1
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
            #endif
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
        DEBUG_SIU("\n#AECurSet=%d,ae_abs_logYdiff=%d\n",AECurSet,ae_abs_logYdiff);
        
		if (AEPreSet != AECurSet)
		{		     
			 if (AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AEPreSet].EL != AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL)
			 {
              #if(HW_BOARD_OPTION==ALTERA_FPGA)
                   SetSensorSW(AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL*27/48);
              #else
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
	        /*
	           Fix 9001D bug: 9001D IPU ouput size只能接受Width >= 656,但OV7725最多只能吐出width=656.
	           因此後續會用CPU補點. IPU input size=660, IPU outsize=656.
	        */
	        Img_Width =640;
            Img_Height=480;
    		sensor_validsize.imgSize.x = Img_Width + 4;
    		sensor_validsize.imgSize.y = Img_Height + 4; 
         #if SENSOR_ROW_COL_MIRROR
            sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 0;	       
         #else
    		sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 1;	       
    	 #endif	
    		
    		SiuValidSize = 	(sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
			                (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);	
        

        	SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
        			        (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
                
            //DEBUG_SIU("size[%d, %d], str[%d, %d]\n", sensor_validsize.imgSize.x, sensor_validsize.imgSize.y, sensor_validsize.imgStr.x, sensor_validsize.imgStr.y);
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
         	isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-4);
    		break;

	    case SIUMODE_PREVIEW: //Preview mode	         		
	        Img_Width =640;
            Img_Height=480;
            //Img_Width =PreviewZoomParm_VGA[zoomFactor].size.x;
            //Img_Height=PreviewZoomParm_VGA[zoomFactor].size.y;
    		sensor_validsize.imgSize.x = Img_Width + 4;
    		sensor_validsize.imgSize.y = Img_Height + 6;//Fix 9002D bug: 於最後兩條line 會有overflow. 故多送兩條.
         #if SENSOR_ROW_COL_MIRROR
            sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 0;	       
         #else
    		sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 1;	       
    	 #endif	
    		SiuValidSize = 	(sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);	
        
        	SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
        			        (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
                
            //DEBUG_SIU("size[%d, %d], str[%d, %d]\n", sensor_validsize.imgSize.x, sensor_validsize.imgSize.y, sensor_validsize.imgStr.x, sensor_validsize.imgStr.y);
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
         	isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-6); //Fix 9002D bug: 於最後兩條line 會有overflow. 故多送兩條.	
         	break;
         		         		
        case SIUMODE_MPEGAVI: //Video clip mode: 不支援Zoom-in/Out
            Img_Width =640;
            Img_Height=480;
    		sensor_validsize.imgSize.x = Img_Width + 4;
    		sensor_validsize.imgSize.y = Img_Height + 6;//Fix 9002D bug: 於最後兩條line 會有overflow. 故多送兩條.
         #if SENSOR_ROW_COL_MIRROR
            sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 0;	       
         #else
    		sensor_validsize.imgStr.x = 0;
    		sensor_validsize.imgStr.y = 1;	       
    	 #endif	
    		SiuValidSize = 	(sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
			                (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);	
        

        	SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
        			        (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
                
            //DEBUG_SIU("size[%d, %d], str[%d, %d]\n", sensor_validsize.imgSize.x, sensor_validsize.imgSize.y, sensor_validsize.imgStr.x, sensor_validsize.imgStr.y);
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
         	isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-6); //Fix 9002D bug: 於最後兩條line 會有overflow. 故多送兩條.	
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
     *X=PreviewZoomParm_VGA[zoomFactor].size.x;
     *Y=PreviewZoomParm_VGA[zoomFactor].size.y;
}


void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H)
{
    *W=PreviewZoomParm_VGA[zoomFactor].size.x;
    *H=PreviewZoomParm_VGA[zoomFactor].size.y;
}


u16 getVideoZoomScale(s32 zoomFactor)
{
     return 0; 
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

void siu_FID_INT_ena()
{
}

void siu_FID_INT_disa()
{
}

void siu_RGBgamma_ON()
{
   SiuSensCtrl |= SIU_RGB_GAMMA_ENA;
}

void siu_RGBgamma_OFF()
{
   SiuSensCtrl &= (~SIU_RGB_GAMMA_ENA);
}

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
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

void siuSensorSet_60fps()
{

	    
    i2cWrite_SENSOR( 0x11, 0x0000);  // Internal clock= input_clock*4/((0+1)*2)=input_clock = 48MHz

    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )     //60Hz
    {
      i2cWrite_SENSOR(0x22, 128);    
      i2cWrite_SENSOR(0x23, 3); // 01/03/07/0f for 60/30/15/7.5fps
      i2cWrite_SENSOR(0x33, 0x0000); //Insert dummy line. -->7
      i2cWrite_SENSOR(0x34, 0);
      i2cWrite_SENSOR(0x2d, 0x0000); //insert dummy line to Vsync.
    }
    else //50Hz
    {
      i2cWrite_SENSOR(0x22, 153); 
      i2cWrite_SENSOR(0x23, 3); //01/03  for 50/25 fps
      i2cWrite_SENSOR(0x33, 102);    //Insert dummy line. -->102
      i2cWrite_SENSOR(0x34, 0);
      i2cWrite_SENSOR(0x2d, 0x0000); //insert dummy line to Vsync.
    }
    i2cWrite_SENSOR(0x2b, 0x0000); 
    i2cWrite_SENSOR(0x2a, 0x0000);
}

#if(HW_BOARD_OPTION == RDI_CARMREC)
/*

Routine Description:

    set Sensor UV saturation.

Arguments:

  	o - color mode
   	1 - monochrome

Return Value:

    None

*/

void Set_Sensor_UV_Saturation(void)
{
    u8 level;
    u16 Again;
    
    gpioGetLevel(0, GPIO_CHECKBIT_IR_LED, &level);
    i2cRead_SENSOR(0x0, &Again);

    if(Again>=DENOISE_ON_THR)
       ipuDenoise_ON(); //AGC > 8x: Denoise  enable
    else
       ipuDenoise_OFF();
    
    if(level)
    {
        /*monochrome */
        ipuDenoise_ON(); //Denoise  enable
        ipuSetEdgeEnhance(2);
        siuAwbGrayWorld(SIU_AWB_FORCEGRAY);
        DEBUG_SIU("\nSIU set sensor monochrome mode\n");
    }
    else
    {
        /*color*/
        ipuDenoise_OFF();
        if(Again>=DENOISE_ON_THR)
            ipuSetEdgeEnhance(1);
        else
            ipuSetEdgeEnhance(0);
 	    siuAwbGrayWorld(SIU_AWB_NORMAL);
        DEBUG_SIU("\nSIU set sensor color mode\n");
    }       
        
}
#endif


#endif	//Lisa 5M 

