
/*
Copyright (c) 2014 Mars Semiconductor Corp.

Module Name:

    siu_XC7021_ISP

Abstract:

    The routines of Sensor Interface Unit.
    Control XC7021 ISP + SC2133 sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2107/03/22  Amon Li Create  
*/


#include "general.h"
#include "board.h"
#if (Sensor_OPTION == Sensor_XC7021_SC2133)
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


#define TAR_MID 0xd0
#define CON_MID 0x40
#define SAT_MID 0x20
#define SHA_MID 0x10
#define TAR_MID_N 0x00

const u8 AETargetMeanTab[11]    =   {TAR_MID-0xb0, TAR_MID-0xb0, TAR_MID-0x50, TAR_MID-0x50, TAR_MID, TAR_MID+0x70, TAR_MID+0xd0, TAR_MID+0xd0, TAR_MID+0x110, TAR_MID+0x110, TAR_MID+0x110};
const s8 AETargetMeanTab_N[11]    =   {TAR_MID_N-0x50, TAR_MID_N-0x40, TAR_MID_N-0x30, TAR_MID_N-0x20, TAR_MID_N-0x10, TAR_MID_N, TAR_MID_N+0x10, TAR_MID_N+0x20, TAR_MID_N+0x30, TAR_MID_N+0x40, TAR_MID_N+0x50};

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

#if ( (CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
u32 XC721_720P_FrameRate  = 20;
u32 XC7021_1080P_FrameRate  = 10;
#else
u32 XC7021_720P_FrameRate  = 15;
u32 XC7021_1080P_FrameRate  = 10;
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

s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    i2cWrite_XC7021(0xfffe,0x14);
    if (flicker_sel)
        i2cWrite_XC7021(0x00b4,0x01);  //banding mode 01:60HZ 02:50HZ
    else
        i2cWrite_XC7021(0x00b4,0x02);  //banding mode 01:60HZ 02:50HZ
   
    switch(uiMenuVideoSizeSetting)
    {
        case UI_MENU_VIDEO_SIZE_1920x1072:
        case UI_MENU_VIDEO_SIZE_1600x896:
            i2cWrite_XC7021(0x00b6,0x0b);
            i2cWrite_XC7021(0x00b7,0xe0);	//60HZ Unit
            i2cWrite_XC7021(0x00b8,0x0e);
            i2cWrite_XC7021(0x00b9,0x40);	//50HZ Unit
            break;
        case UI_MENU_VIDEO_SIZE_1280X720:
        case UI_MENU_VIDEO_SIZE_640x352:
        case UI_MENU_VIDEO_SIZE_704x480:
            i2cWrite_XC7021(0x00b6,0x18);
            i2cWrite_XC7021(0x00b7,0x0a);	//60HZ Unit
            i2cWrite_XC7021(0x00b8,0x1c);
            i2cWrite_XC7021(0x00b9,0xda);	//50HZ Unit
            break;
		default:
            break;
    }
}

s32 siuSetExposureValue(s8 expBiasValue)
{

    siuY_TargetIndex = expBiasValue;
    DEBUG_SIU("siuSetExposureValue(%d) \n", expBiasValue);
    //RX 6 level only, index RX to TX {0,0,2,4,5,6,8}
    i2cWrite_XC7021(0xfffe, 0x14); 
    if (expBiasValue < 5)
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
s32 SC2133_Init_1080P(void) //init setting is original 1080p related
{
    u8 data;
    DEBUG_SIU("SC2133_Init_1080P() \n");
    XC7021BypassOn();
    i2cWrite_SC2133(0x0103,0x01);  // reset all registers
    i2cWrite_SC2133(0x0100,0x00);  // stream output hold
    i2cWrite_SC2133(0x3e03,0x03);  //AE AG setting
    i2cWrite_SC2133(0x0103,0x01);
    i2cWrite_SC2133(0x0100,0x00);
    i2cWrite_SC2133(0x3e03,0x03);
    i2cWrite_SC2133(0x3e01,0x46);
    i2cWrite_SC2133(0x3e08,0x00);
    i2cWrite_SC2133(0x3e09,0x10);
    i2cWrite_SC2133(0x3416,0x11);
    i2cWrite_SC2133(0x3300,0x20);
    i2cWrite_SC2133(0x3301,0x08);
    i2cWrite_SC2133(0x3303,0x30);
    i2cWrite_SC2133(0x3306,0x78);
    i2cWrite_SC2133(0x330b,0xd0);
    i2cWrite_SC2133(0x3309,0x30);
    i2cWrite_SC2133(0x3308,0x0a);
    i2cWrite_SC2133(0x331e,0x26);
    i2cWrite_SC2133(0x331f,0x26);
    i2cWrite_SC2133(0x3320,0x2c);
    i2cWrite_SC2133(0x3321,0x2c);
    i2cWrite_SC2133(0x3322,0x2c);
    i2cWrite_SC2133(0x3323,0x2c);
    i2cWrite_SC2133(0x330e,0x20);
    i2cWrite_SC2133(0x3f05,0xdf);
    i2cWrite_SC2133(0x3f01,0x04);
    i2cWrite_SC2133(0x3626,0x04);
    i2cWrite_SC2133(0x3312,0x06);
    i2cWrite_SC2133(0x3340,0x04);
    i2cWrite_SC2133(0x3341,0x68);

    i2cWrite_SC2133(0x3342,0x02);
    i2cWrite_SC2133(0x3343,0x20);
    i2cWrite_SC2133(0x3333,0x10);
    i2cWrite_SC2133(0x3334,0x20);
    i2cWrite_SC2133(0x3621,0x18);
    i2cWrite_SC2133(0x3626,0x04);
    i2cWrite_SC2133(0x3635,0x34);
    i2cWrite_SC2133(0x3038,0xa4);
    i2cWrite_SC2133(0x3630,0x84);
    i2cWrite_SC2133(0x3622,0x0e);
    i2cWrite_SC2133(0x3620,0x62);
    i2cWrite_SC2133(0x3627,0x08);
    i2cWrite_SC2133(0x3637,0x87);
    i2cWrite_SC2133(0x3638,0x86);
    i2cWrite_SC2133(0x3034,0xd2);
    i2cWrite_SC2133(0x5780,0xff);
    i2cWrite_SC2133(0x5781,0x0c);
    i2cWrite_SC2133(0x5785,0x10);
    i2cWrite_SC2133(0x3d08,0x00);
    i2cWrite_SC2133(0x3640,0x03);
    i2cWrite_SC2133(0x320c,0x04);
    i2cWrite_SC2133(0x320d,0xe8);

    i2cWrite_SC2133(0x3662,0x82);
    i2cWrite_SC2133(0x335d,0x00);
    i2cWrite_SC2133(0x4501,0xa4);
    i2cWrite_SC2133(0x3333,0x00);
    i2cWrite_SC2133(0x3627,0x02);
    i2cWrite_SC2133(0x3620,0x62);
    i2cWrite_SC2133(0x5781,0x04);
    i2cWrite_SC2133(0x3333,0x10);
    i2cWrite_SC2133(0x3306,0x69);
    i2cWrite_SC2133(0x3635,0x52);
    i2cWrite_SC2133(0x3636,0x7c);
    i2cWrite_SC2133(0x3631,0x84);
    i2cWrite_SC2133(0x330b,0xe0);
    i2cWrite_SC2133(0x3637,0x88);
    i2cWrite_SC2133(0x3306,0x6b);
    i2cWrite_SC2133(0x330b,0xd0);
    i2cWrite_SC2133(0x3630,0x84);
    i2cWrite_SC2133(0x335d,0x20);
    i2cWrite_SC2133(0x3368,0x02);
    i2cWrite_SC2133(0x3369,0x00);
    i2cWrite_SC2133(0x336A,0x04);
    i2cWrite_SC2133(0x336b,0x65);
    i2cWrite_SC2133(0x330E,0x20);
    i2cWrite_SC2133(0x3367,0x05);
    i2cWrite_SC2133(0x3620,0x92);
    i2cWrite_SC2133(0x3634,0xd2);
    i2cWrite_SC2133(0x3633,0x17);
    i2cWrite_SC2133(0x3315,0x02);
    i2cWrite_SC2133(0x303a,0x18);
    i2cWrite_SC2133(0x3039,0x9e);
    i2cWrite_SC2133(0x3035,0x25);
    i2cWrite_SC2133(0x3034,0x16);
    i2cWrite_SC2133(0x3036,0x00);
    i2cWrite_SC2133(0x3633,0x16);
    i2cWrite_SC2133(0x3039,0xd6);
    i2cWrite_SC2133(0x320e,0x04);
    i2cWrite_SC2133(0x320f,0x74);
    i2cWrite_SC2133(0x3306,0x5a);
    i2cWrite_SC2133(0x330b,0x98);
    #if (SENSOR_ROW_COL_MIRROR == 1)
    i2cWrite_SC2133(0x3220,0x00);
    i2cWrite_SC2133(0x3221,0x00);
    i2cWrite_SC2133(0x3213,0x10);
    #else
    i2cWrite_SC2133(0x3220,0x06);
    i2cWrite_SC2133(0x3221,0x06);
    i2cWrite_SC2133(0x3213,0x11);
    #endif
    i2cWrite_SC2133(0x3e01,0x01);
    i2cWrite_SC2133(0x0100,0x01);
    XC7021BypassOff();
}

s32 XC7021_Init_1080P(void) //init setting is original 1080p related
{
    int i;

    DEBUG_SIU("XC7021_Init_1080P() \n");

    #if(HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613)
    //RST pin is inverse, pull low => pullup inside sensor => modify in gpio_project.c
    #else
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    #endif
    OSTimeDly(10);
    
    i2cWrite_XC7021(0xfffd,0x80); 
    i2cWrite_XC7021(0xfffe,0x50);  //分頁 
    i2cWrite_XC7021(0x001c,0xff);  //CLOCK  
    i2cWrite_XC7021(0x001d,0xff); 
    i2cWrite_XC7021(0x001e,0xff); 
    i2cWrite_XC7021(0x001f,0xef); 
    i2cWrite_XC7021(0x0018,0x00);  //RESET 
    i2cWrite_XC7021(0x0019,0x00); 
    i2cWrite_XC7021(0x001a,0x00); 
    i2cWrite_XC7021(0x001b,0x00); 
    i2cWrite_XC7021(0x00bc,0x11);  //PAD 
    i2cWrite_XC7021(0x00bd,0x00); 
    i2cWrite_XC7021(0x00be,0x00); 
    i2cWrite_XC7021(0x00bf,0x00);
     
    i2cWrite_XC7021(0x0030,0x09);
    i2cWrite_XC7021(0x0031,0x02);
    i2cWrite_XC7021(0x0032,0x05);//
    i2cWrite_XC7021(0x0020,0x01);
    i2cWrite_XC7021(0x0021,0x0E);
    i2cWrite_XC7021(0x0023,0x02);
    i2cWrite_XC7021(0x0024,0x06);
    i2cWrite_XC7021(0x0025,0x00);//0
    i2cWrite_XC7021(0x0026,0x01);
    i2cWrite_XC7021(0x0027,0x06); 
    i2cWrite_XC7021(0x0028,0x08);  //I2C_CLK 
    i2cWrite_XC7021(0x0029,0x00); 
    i2cWrite_XC7021(0xfffe,0x25); 
    i2cWrite_XC7021(0x200b,0x33);  //de-glitch                            
    i2cWrite_XC7021(0x0002,0x80);  //I2C master speed（I2C 主控速度）     
    i2cWrite_XC7021(0xfffe,0x50);                                         
    i2cWrite_XC7021(0x0200,0x03);  //SPWD_GPIO_enable bit1:pwdn bit0:reset
    i2cWrite_XC7021(0x0204,0x03);  //配置GPIO腳為output                   
    i2cWrite_XC7021(0x0208,0x02);  // pwdn high active, rst low active    
    i2cWrite_XC7021(0x0208,0x01);  //pwdn low，reset high                 

    i2cWrite_XC7021(0xfffe,0x25);
    i2cWrite_XC7021(0x6002,0x24);//max:50
    i2cWrite_XC7021(0x6003,0x10);
    i2cWrite_XC7021(0x6008,0x01);//bit[0] pclk極性;bit[1] Vsync極性;bit[2] Hsync極性

    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x00bc,0x11);
    i2cWrite_XC7021(0x001b,0x00);
    i2cWrite_XC7021(0x0090,0x29); //29 test pattern 2b
    i2cWrite_XC7021(0xfffe,0x20);
    i2cWrite_XC7021(0x0000,0x00);
    i2cWrite_XC7021(0x0004,0x07);
    i2cWrite_XC7021(0x0005,0x80);
    i2cWrite_XC7021(0x0006,0x04);
    i2cWrite_XC7021(0x0007,0x38);
    i2cWrite_XC7021(0xfffe,0x26);
    i2cWrite_XC7021(0x4000,0xF9);
    i2cWrite_XC7021(0x6001,0x14);
    i2cWrite_XC7021(0x6005,0xc4);
    i2cWrite_XC7021(0x6006,0x0A);
    i2cWrite_XC7021(0x6007,0x8C);
    i2cWrite_XC7021(0x6008,0x09);

    i2cWrite_XC7021(0x6009,0xFC);
    i2cWrite_XC7021(0x8000,0x3f);
    i2cWrite_XC7021(0x8001,0x80);
    i2cWrite_XC7021(0x8002,0x07);
    i2cWrite_XC7021(0x8003,0x38);
    i2cWrite_XC7021(0x8004,0x04);
    i2cWrite_XC7021(0x8005,0x03);
    i2cWrite_XC7021(0x8006,0x05);
    i2cWrite_XC7021(0x8007,0x99);
    i2cWrite_XC7021(0x8008,0x14);
    i2cWrite_XC7021(0x8010,0x04);
    i2cWrite_XC7021(0x8012,0x80);
    i2cWrite_XC7021(0x8013,0x07);
    i2cWrite_XC7021(0x8014,0x38);
    i2cWrite_XC7021(0x8015,0x04);
    i2cWrite_XC7021(0x8016,0x00);
    i2cWrite_XC7021(0x8017,0x00);
    i2cWrite_XC7021(0x8018,0x00);
    i2cWrite_XC7021(0x8019,0x00);
    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0001,0x11);

    i2cWrite_XC7021(0x0004,0x18);
    i2cWrite_XC7021(0x0006,0x07);
    i2cWrite_XC7021(0x0007,0x80);
    i2cWrite_XC7021(0x0008,0x04);
    i2cWrite_XC7021(0x0009,0x38);
    i2cWrite_XC7021(0x000a,0x05);
    i2cWrite_XC7021(0x000b,0x00);
    i2cWrite_XC7021(0x000c,0x02);
    i2cWrite_XC7021(0x000d,0xD0);

    i2cWrite_XC7021(0x0027,0xF7);
    i2cWrite_XC7021(0x005e,0x07);
    i2cWrite_XC7021(0x005f,0x7F);
    i2cWrite_XC7021(0x0060,0x04);
    i2cWrite_XC7021(0x0061,0x37);
    i2cWrite_XC7021(0x1908,0x00);
    i2cWrite_XC7021(0x1900,0x00);
    i2cWrite_XC7021(0x1901,0x00);
    i2cWrite_XC7021(0x1902,0x00);
    i2cWrite_XC7021(0x1903,0x00);
    i2cWrite_XC7021(0x1904,0x07);
    i2cWrite_XC7021(0x1905,0x80);
    i2cWrite_XC7021(0x1906,0x04);
    i2cWrite_XC7021(0x1907,0x38);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x1f00,0x00);
    i2cWrite_XC7021(0x1f01,0x00);
    i2cWrite_XC7021(0x1f02,0x00);
    i2cWrite_XC7021(0x1f03,0x00);
    i2cWrite_XC7021(0x1f04,0x07);
    i2cWrite_XC7021(0x1f05,0x80);
    i2cWrite_XC7021(0x1f06,0x04);
    i2cWrite_XC7021(0x1f07,0x38);
    i2cWrite_XC7021(0x1f08,0x03);

    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x0026,0x01);    
    i2cWrite_XC7021(0x0035,0x01);
    i2cWrite_XC7021(0x00be,0x60);  
    i2cWrite_XC7021(0x00bf,0x01);  
    i2cWrite_XC7021(0x00c0,0x00);  
    i2cWrite_XC7021(0x00c1,0x00);  
    //exposure
    i2cWrite_XC7021(0x00c4,0x00);  
    i2cWrite_XC7021(0x00c5,0x00);
    i2cWrite_XC7021(0x00c6,0x3e);  
    i2cWrite_XC7021(0x00c7,0x01);
    i2cWrite_XC7021(0x00c8,0x3e);  
    i2cWrite_XC7021(0x00c9,0x02);
    i2cWrite_XC7021(0x00cc,0x00);  
    i2cWrite_XC7021(0x00cd,0x00);  
    i2cWrite_XC7021(0x00ce,0x00);  
    i2cWrite_XC7021(0x00cf,0xff);
    i2cWrite_XC7021(0x00d0,0x00);  
    i2cWrite_XC7021(0x00d1,0x0f);
    i2cWrite_XC7021(0x00d2,0x00);  
    i2cWrite_XC7021(0x00d3,0x00);
    //gain
    i2cWrite_XC7021(0x00e4,0x3e);  
    i2cWrite_XC7021(0x00e5,0x08);
    i2cWrite_XC7021(0x00e6,0x3e);  
    i2cWrite_XC7021(0x00e7,0x09);
    i2cWrite_XC7021(0x00ec,0x00);  
    i2cWrite_XC7021(0x00ed,0xff);
    i2cWrite_XC7021(0x00ee,0x00);  
    i2cWrite_XC7021(0x00ef,0xff);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0051,0x03);
    i2cWrite_XC7021(0xfffe,0x14);  // ===== AE CFG  =====
    i2cWrite_XC7021(0x00aa,0x06);
    i2cWrite_XC7021(0x00ab,0x00);  //max gain
    i2cWrite_XC7021(0x00ba,0x00);  //low light mode
    i2cWrite_XC7021(0x006a,0x01);
    i2cWrite_XC7021(0x006b,0xc0);  //night target
    i2cWrite_XC7021(0x006c,0x01);
    i2cWrite_XC7021(0x006d,0xf0);  //day target
    i2cWrite_XC7021(0x00b4,0x02);  //banding mode 01:60HZ 02:50HZ
    i2cWrite_XC7021(0x00b6,0x18);
    i2cWrite_XC7021(0x00b7,0x0a);
    i2cWrite_XC7021(0x00b8,0x1c);
    i2cWrite_XC7021(0x00b9,0xda);

    i2cWrite_XC7021(0x0092,0x43);  //max exposure   //2d
    i2cWrite_XC7021(0x0093,0x00);
    i2cWrite_XC7021(0xfffe,0x14);  // ===== AE SPEED  =====

    i2cWrite_XC7021(0x0039,0x01);  //detect enable
    i2cWrite_XC7021(0x0060,0x00);
    i2cWrite_XC7021(0x0024,0x02);
    i2cWrite_XC7021(0x0025,0x02);
    i2cWrite_XC7021(0x0071,0xf0);  //60 //target offset
    i2cWrite_XC7021(0x0076,0x02); 	// mutiple frame delay
    i2cWrite_XC7021(0x007a,0x00);  //thr_l
    i2cWrite_XC7021(0x007b,0x30);
    i2cWrite_XC7021(0x007c,0x00);  //thr_h
    i2cWrite_XC7021(0x007d,0xb0);
    i2cWrite_XC7021(0x0077,0x30);	
    i2cWrite_XC7021(0x0078,0x01);  //AEC speed; 01:fast; 00:slow
    i2cWrite_XC7021(0x007f,0x60);  //Jump_Thr		
    i2cWrite_XC7021(0x0087,0x60);
    i2cWrite_XC7021(0x0088,0x02);
    i2cWrite_XC7021(0x008a,0x00);
    i2cWrite_XC7021(0x008b,0x00);
    i2cWrite_XC7021(0x008c,0x03);
    i2cWrite_XC7021(0x008d,0x80);

    // AE SMART
    i2cWrite_XC7021(0x0073,0x28);  // revise diff
    i2cWrite_XC7021(0x0112,0x03);  // smart target speed
    i2cWrite_XC7021(0x011a,0x00);  // attenton min max limit
    i2cWrite_XC7021(0x011b,0x00);   
    i2cWrite_XC7021(0x011c,0x02);   
    i2cWrite_XC7021(0x011d,0x20);  
    i2cWrite_XC7021(0x011e,0x00);  // all block min max limit
    i2cWrite_XC7021(0x011f,0x00);
    i2cWrite_XC7021(0x0120,0x02);
    i2cWrite_XC7021(0x0121,0x80);
    //disable day night select
    i2cWrite_XC7021(0x0062,0xff);
    i2cWrite_XC7021(0x0063,0xf0);  
    i2cWrite_XC7021(0x0064,0xff);
    i2cWrite_XC7021(0x0065,0xf0);   
    i2cWrite_XC7021(0x0066,0x00);  
    i2cWrite_XC7021(0x0067,0x00);
    i2cWrite_XC7021(0x0074,0x00);  
    i2cWrite_XC7021(0x0075,0x00);  
    i2cWrite_XC7021(0x012a,0x00);  //variance incrase bright
    i2cWrite_XC7021(0x012b,0x00);
    i2cWrite_XC7021(0xfffd,0x80); 
    i2cWrite_XC7021(0xfffe,0x14);   
    i2cWrite_XC7021(0x003f,0x10);
    i2cWrite_XC7021(0x0040,0x10);
    i2cWrite_XC7021(0x0041,0x10);
    i2cWrite_XC7021(0x0042,0x10);
    i2cWrite_XC7021(0x0043,0x10);
    i2cWrite_XC7021(0x0044,0x10);
    i2cWrite_XC7021(0x0045,0x10);
    i2cWrite_XC7021(0x0046,0x10);
    i2cWrite_XC7021(0x0047,0x10);
    i2cWrite_XC7021(0x0048,0x10);
    i2cWrite_XC7021(0x0049,0x10);
    i2cWrite_XC7021(0x004a,0x10);
    i2cWrite_XC7021(0x004b,0x10);
    i2cWrite_XC7021(0x004c,0x10);
    i2cWrite_XC7021(0x004d,0x10);
    i2cWrite_XC7021(0x004e,0x10);
    i2cWrite_XC7021(0x004f,0x10);
    i2cWrite_XC7021(0x0050,0x10);
    i2cWrite_XC7021(0x0051,0x10);
    i2cWrite_XC7021(0x0052,0x10);
    i2cWrite_XC7021(0x0053,0x10);
    i2cWrite_XC7021(0x0054,0x10);
    i2cWrite_XC7021(0x0055,0x10);
    i2cWrite_XC7021(0x0056,0x10);
    i2cWrite_XC7021(0x0057,0x10);
    i2cWrite_XC7021(0x005c,0x00);
    i2cWrite_XC7021(0x005d,0x00);			
    i2cWrite_XC7021(0x005e,0x00);  //attention block cfg
    i2cWrite_XC7021(0x005f,0x00);
    i2cWrite_XC7021(0x003a,0x01);  //單次有效,設置之後權重和關注區域生效
                         
    i2cWrite_XC7021(0x0060,0x00);                                                                        
    i2cWrite_XC7021(0x006e,0x00); 
    i2cWrite_XC7021(0x006f,0x00); 
    i2cWrite_XC7021(0x0070,0xff);                                                                    
    i2cWrite_XC7021(0x003b,0x40); 
    i2cWrite_XC7021(0x003c,0x40); 
    i2cWrite_XC7021(0x003d,0x90); 
    i2cWrite_XC7021(0x003e,0x10); 
    i2cWrite_XC7021(0x0039,0x00); 
    //0927 by wenzhe
    i2cWrite_XC7021(0x0112,0x08);  //target speed 
    i2cWrite_XC7021(0x012e,0x01);  // patch add 
    i2cWrite_XC7021(0x012f,0xc0);
    i2cWrite_XC7021(0x0131,0x10);
    i2cWrite_XC7021(0x010f,0x00);  // gain and exp delay
    i2cWrite_XC7021(0x0110,0x00);
    i2cWrite_XC7021(0x0071,0xC0);  // target offset
    //0928 by wenzhe
    i2cWrite_XC7021(0x0077,0x20);  // finally thr
    i2cWrite_XC7021(0x006c,0x01);  // target
    i2cWrite_XC7021(0x006d,0xd0);
    i2cWrite_XC7021(0x0112,0x06);  // target speed

    i2cWrite_XC7021(0xfffe,0x30);  //LENC START
    i2cWrite_XC7021(0x0200,0x3f);  //lenc_85
    i2cWrite_XC7021(0x0201,0x29);
    i2cWrite_XC7021(0x0202,0x1d);
    i2cWrite_XC7021(0x0203,0x1b);
    i2cWrite_XC7021(0x0204,0x25);
    i2cWrite_XC7021(0x0205,0x3f);
    i2cWrite_XC7021(0x0206,0x09);
    i2cWrite_XC7021(0x0207,0x04);
    i2cWrite_XC7021(0x0208,0x02);
    i2cWrite_XC7021(0x0209,0x02);
    i2cWrite_XC7021(0x020a,0x04);
    i2cWrite_XC7021(0x020b,0x07);
    i2cWrite_XC7021(0x020c,0x03);
    i2cWrite_XC7021(0x020d,0x01);
    i2cWrite_XC7021(0x020e,0x00);
    i2cWrite_XC7021(0x020f,0x00);
    i2cWrite_XC7021(0x0210,0x01);
    i2cWrite_XC7021(0x0211,0x02);
    i2cWrite_XC7021(0x0212,0x03);
    i2cWrite_XC7021(0x0213,0x01);
    i2cWrite_XC7021(0x0214,0x00);
    i2cWrite_XC7021(0x0215,0x00);
    i2cWrite_XC7021(0x0216,0x00);
    i2cWrite_XC7021(0x0217,0x02);
    i2cWrite_XC7021(0x0218,0x08);
    i2cWrite_XC7021(0x0219,0x03);
    i2cWrite_XC7021(0x021a,0x01);
    i2cWrite_XC7021(0x021b,0x01);
    i2cWrite_XC7021(0x021c,0x03);
    i2cWrite_XC7021(0x021d,0x07);
    i2cWrite_XC7021(0x021e,0x3f);
    i2cWrite_XC7021(0x021f,0x21);
    i2cWrite_XC7021(0x0220,0x18);
    i2cWrite_XC7021(0x0221,0x17);
    i2cWrite_XC7021(0x0222,0x20);
    i2cWrite_XC7021(0x0223,0x3e);
    i2cWrite_XC7021(0x0224,0x20);
    i2cWrite_XC7021(0x0225,0x22);
    i2cWrite_XC7021(0x0226,0x23);
    i2cWrite_XC7021(0x0227,0x13);
    i2cWrite_XC7021(0x0228,0x11);
    i2cWrite_XC7021(0x022a,0x35);
    i2cWrite_XC7021(0x022b,0x45);
    i2cWrite_XC7021(0x022c,0x55);
    i2cWrite_XC7021(0x022d,0x45);
    i2cWrite_XC7021(0x022e,0x36);
    i2cWrite_XC7021(0x0230,0x45);
    i2cWrite_XC7021(0x0231,0x55);
    i2cWrite_XC7021(0x0232,0x55);
    i2cWrite_XC7021(0x0233,0x55);
    i2cWrite_XC7021(0x0234,0x56);
    i2cWrite_XC7021(0x0236,0x46);
    i2cWrite_XC7021(0x0237,0x45);
    i2cWrite_XC7021(0x0238,0x55);
    i2cWrite_XC7021(0x0239,0x56);
    i2cWrite_XC7021(0x023a,0x46);
    i2cWrite_XC7021(0x023c,0x02);
    i2cWrite_XC7021(0x023d,0x24);
    i2cWrite_XC7021(0x023e,0x24);
    i2cWrite_XC7021(0x023f,0x24);
    i2cWrite_XC7021(0x0240,0x12);
    i2cWrite_XC7021(0x0248,0xbb);

    i2cWrite_XC7021(0xfffe,0x30);	
    i2cWrite_XC7021(0x024d,0x00);	  
    i2cWrite_XC7021(0x024e,0xCC);	
    i2cWrite_XC7021(0x024f,0x01);	  
    i2cWrite_XC7021(0x0250,0x6C);	  
    i2cWrite_XC7021(0x0251,0x01);	  
    i2cWrite_XC7021(0x0252,0x11);	  
    i2cWrite_XC7021(0x0253,0x00);	  
    i2cWrite_XC7021(0x0254,0xF2);  //LENC END

    i2cWrite_XC7021(0xfffe,0x14);   //AWB_init    
    i2cWrite_XC7021(0x0027,0x01);  //firmware中 awb_enable                              
    i2cWrite_XC7021(0x013c,0x02);  //0.AWB_ARITH_ORIGIN  1.AWB_SW_PRO  2.AWB_ARITH_MANUAL
    i2cWrite_XC7021(0x0176,0x06);   //int B gain
    i2cWrite_XC7021(0x0177,0x00);  
    i2cWrite_XC7021(0x017a,0x04);  //int Gb gain
    i2cWrite_XC7021(0x017b,0x00); 
    i2cWrite_XC7021(0x017e,0x04);   //int Gr gain
    i2cWrite_XC7021(0x017f,0x00); 
    i2cWrite_XC7021(0x0182,0x04);   //int R gain
    i2cWrite_XC7021(0x0183,0x04);        
    i2cWrite_XC7021(0x01aa,0x06);    //B_temp      
    i2cWrite_XC7021(0x01ab,0x00);           
    i2cWrite_XC7021(0x01ae,0x04);    //G_ temp          
    i2cWrite_XC7021(0x01af,0x00);      
    i2cWrite_XC7021(0x01b2,0x04);   //R_temp      
    i2cWrite_XC7021(0x01b3,0x04); 

    i2cWrite_XC7021(0xfffe,0x14);   //C_AWB
    i2cWrite_XC7021(0x0027,0x01);  //firmware中 awb_enable
    i2cWrite_XC7021(0x013c,0x01);  //0.AWB_ARITH_ORIGIN  1.AWB_SW_PRO  2.AWB_ARITH_MANUAL
    i2cWrite_XC7021(0x013d,0x01);  //AWBFlexiMap_en
    i2cWrite_XC7021(0x013e,0x00);  //AWBMove_en
    i2cWrite_XC7021(0x0170,0x0d);  //nMaxAwbGain
    i2cWrite_XC7021(0x0171,0xff);

    i2cWrite_XC7021(0x0708,0x03);
    i2cWrite_XC7021(0x0709,0xf0);
    i2cWrite_XC7021(0x070a,0x00);
    i2cWrite_XC7021(0x070b,0x0c);
    i2cWrite_XC7021(0x0001,0x67);
    i2cWrite_XC7021(0x0003,0xe5);
    i2cWrite_XC7021(0x0051,0x03);
    i2cWrite_XC7021(0x0096,0x83);
    i2cWrite_XC7021(0x0019,0x48);
    i2cWrite_XC7021(0x071c,0x0a);

    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x016e,0x08);
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0000,0xc7);
    i2cWrite_XC7021(0x0001,0x23);
    i2cWrite_XC7021(0x0003,0xe5);

    i2cWrite_XC7021(0x0730,0x5d);
    i2cWrite_XC7021(0x0731,0x85);
    i2cWrite_XC7021(0x0732,0x3a);
    i2cWrite_XC7021(0x0733,0x4c);
    i2cWrite_XC7021(0x0734,0x4b);
    i2cWrite_XC7021(0x0735,0x7c);
    i2cWrite_XC7021(0x0736,0x41);
    i2cWrite_XC7021(0x0737,0x68);
    i2cWrite_XC7021(0x0738,0x3c);
    i2cWrite_XC7021(0x0739,0x57);
    i2cWrite_XC7021(0x073a,0x55);
    i2cWrite_XC7021(0x073b,0x7c);
    i2cWrite_XC7021(0x073c,0x43);
    i2cWrite_XC7021(0x073d,0x77);
    i2cWrite_XC7021(0x073e,0x49);
    i2cWrite_XC7021(0x073f,0x7a);
    i2cWrite_XC7021(0x0740,0x00);
    i2cWrite_XC7021(0x0741,0x00);
    i2cWrite_XC7021(0x0742,0x00);
    i2cWrite_XC7021(0x0743,0x00);
    i2cWrite_XC7021(0x0744,0x00);
    i2cWrite_XC7021(0x0745,0x00);
    i2cWrite_XC7021(0x0746,0x00);
    i2cWrite_XC7021(0x0747,0x00);
    i2cWrite_XC7021(0x0748,0x00);
    i2cWrite_XC7021(0x0749,0x00);
    i2cWrite_XC7021(0x074a,0x00);
    i2cWrite_XC7021(0x074b,0x00);
    i2cWrite_XC7021(0x074c,0x00);
    i2cWrite_XC7021(0x074d,0x00);
    i2cWrite_XC7021(0x074e,0x00);
    i2cWrite_XC7021(0x074f,0x00);
    i2cWrite_XC7021(0x0750,0x00);
    i2cWrite_XC7021(0x0751,0x00);
    i2cWrite_XC7021(0x0752,0x00);
    i2cWrite_XC7021(0x0753,0x00);
    i2cWrite_XC7021(0x0754,0x00);
    i2cWrite_XC7021(0x0755,0x00);
    i2cWrite_XC7021(0x0756,0x00);
    i2cWrite_XC7021(0x0757,0x00);
    i2cWrite_XC7021(0x0758,0x00);
    i2cWrite_XC7021(0x0759,0x00);
    i2cWrite_XC7021(0x075a,0x00);
    i2cWrite_XC7021(0x075b,0x00);
    i2cWrite_XC7021(0x075c,0x00);
    i2cWrite_XC7021(0x075d,0x00);
    i2cWrite_XC7021(0x075e,0x00);
    i2cWrite_XC7021(0x075f,0x00);
    i2cWrite_XC7021(0x0760,0x00);
    i2cWrite_XC7021(0x0761,0x00);
    i2cWrite_XC7021(0x0762,0x00);
    i2cWrite_XC7021(0x0763,0x00);
    i2cWrite_XC7021(0x0764,0x00);
    i2cWrite_XC7021(0x0765,0x00);
    i2cWrite_XC7021(0x0766,0x00);
    i2cWrite_XC7021(0x0767,0x00);
    i2cWrite_XC7021(0x0768,0x00);
    i2cWrite_XC7021(0x0769,0x00);
    i2cWrite_XC7021(0x076a,0x00);
    i2cWrite_XC7021(0x076b,0x00);
    i2cWrite_XC7021(0x076c,0x00);
    i2cWrite_XC7021(0x076d,0x00);
    i2cWrite_XC7021(0x076e,0x00);
    i2cWrite_XC7021(0x076f,0x00);
    i2cWrite_XC7021(0x0770,0x22);
    i2cWrite_XC7021(0x0771,0x21);
    i2cWrite_XC7021(0x0772,0x00);
    i2cWrite_XC7021(0x0773,0x00);
    i2cWrite_XC7021(0x0774,0x00);
    i2cWrite_XC7021(0x0775,0x00);
    i2cWrite_XC7021(0x0776,0x00);
    i2cWrite_XC7021(0x0777,0x00);

    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x1400,0x00);
    i2cWrite_XC7021(0x1401,0x03);
    i2cWrite_XC7021(0x1402,0x06);
    i2cWrite_XC7021(0x1403,0x0a);
    i2cWrite_XC7021(0x1404,0x0f);
    i2cWrite_XC7021(0x1405,0x13);
    i2cWrite_XC7021(0x1406,0x18);
    i2cWrite_XC7021(0x1407,0x1c);
    i2cWrite_XC7021(0x1408,0x21);
    i2cWrite_XC7021(0x1409,0x25);
    i2cWrite_XC7021(0x140a,0x2a);
    i2cWrite_XC7021(0x140b,0x2f);
    i2cWrite_XC7021(0x140c,0x33);
    i2cWrite_XC7021(0x140d,0x38);
    i2cWrite_XC7021(0x140e,0x3c);
    i2cWrite_XC7021(0x140f,0x41);
    i2cWrite_XC7021(0x1410,0x46);
    i2cWrite_XC7021(0x1411,0x4a);
    i2cWrite_XC7021(0x1412,0x4e);
    i2cWrite_XC7021(0x1413,0x53);
    i2cWrite_XC7021(0x1414,0x57);
    i2cWrite_XC7021(0x1415,0x5b);
    i2cWrite_XC7021(0x1416,0x5f);
    i2cWrite_XC7021(0x1417,0x63);
    i2cWrite_XC7021(0x1418,0x67);
    i2cWrite_XC7021(0x1419,0x6b);
    i2cWrite_XC7021(0x141a,0x6f);
    i2cWrite_XC7021(0x141b,0x73);
    i2cWrite_XC7021(0x141c,0x76);
    i2cWrite_XC7021(0x141d,0x7a);
    i2cWrite_XC7021(0x141e,0x7d);
    i2cWrite_XC7021(0x141f,0x81);
    i2cWrite_XC7021(0x1420,0x84);
    i2cWrite_XC7021(0x1421,0x8b);
    i2cWrite_XC7021(0x1422,0x91);
    i2cWrite_XC7021(0x1423,0x97);
    i2cWrite_XC7021(0x1424,0x9d);
    i2cWrite_XC7021(0x1425,0xa3);
    i2cWrite_XC7021(0x1426,0xa8);
    i2cWrite_XC7021(0x1427,0xad);
    i2cWrite_XC7021(0x1428,0xb2);
    i2cWrite_XC7021(0x1429,0xb7);
    i2cWrite_XC7021(0x142a,0xbc);
    i2cWrite_XC7021(0x142b,0xc0);
    i2cWrite_XC7021(0x142c,0xc4);
    i2cWrite_XC7021(0x142d,0xc8);
    i2cWrite_XC7021(0x142e,0xcc);
    i2cWrite_XC7021(0x142f,0xcf);
    i2cWrite_XC7021(0x1430,0xd3);
    i2cWrite_XC7021(0x1431,0xd9);
    i2cWrite_XC7021(0x1432,0xde);
    i2cWrite_XC7021(0x1433,0xe3);
    i2cWrite_XC7021(0x1434,0xe8);
    i2cWrite_XC7021(0x1435,0xec);
    i2cWrite_XC7021(0x1436,0xee);
    i2cWrite_XC7021(0x1437,0xf0);
    i2cWrite_XC7021(0x1438,0xf2);
    i2cWrite_XC7021(0x1439,0xf4);
    i2cWrite_XC7021(0x143a,0xf6);
    i2cWrite_XC7021(0x143b,0xf8);
    i2cWrite_XC7021(0x143c,0xfa);
    i2cWrite_XC7021(0x143d,0xfb);
    i2cWrite_XC7021(0x143e,0xfc);
    i2cWrite_XC7021(0x143f,0xfd);
    i2cWrite_XC7021(0x1440,0xfe);

    i2cWrite_XC7021(0xfffe,0x30);

    i2cWrite_XC7021(0x1200,0x04);		//DAY
    i2cWrite_XC7021(0x1201,0xDB);		
    i2cWrite_XC7021(0x1202,0x00);		
    i2cWrite_XC7021(0x1203,0x5E);		
    i2cWrite_XC7021(0x1204,0x02);		
    i2cWrite_XC7021(0x1205,0x56);		
    i2cWrite_XC7021(0x1206,0x01);		
    i2cWrite_XC7021(0x1207,0x77);		
    i2cWrite_XC7021(0x1208,0x00);		
    i2cWrite_XC7021(0x1209,0xF0);		
    i2cWrite_XC7021(0x120A,0x03);		
    i2cWrite_XC7021(0x120B,0x12);		
    i2cWrite_XC7021(0x120C,0x09);		//ALIGHT
    i2cWrite_XC7021(0x120D,0xCC);		
    i2cWrite_XC7021(0x120E,0x00);		
    i2cWrite_XC7021(0x120F,0x1E);		
    i2cWrite_XC7021(0x1210,0x01);		
    i2cWrite_XC7021(0x1211,0x13);		
    i2cWrite_XC7021(0x1212,0x01);		
    i2cWrite_XC7021(0x1213,0xF9);		
    i2cWrite_XC7021(0x1214,0x01);		
    i2cWrite_XC7021(0x1215,0x09);		
    i2cWrite_XC7021(0x1216,0x01);		
    i2cWrite_XC7021(0x1217,0x7D);		
    i2cWrite_XC7021(0x1218,0x05);		//CWF
    i2cWrite_XC7021(0x1219,0xCF);		
    i2cWrite_XC7021(0x121A,0x00);		
    i2cWrite_XC7021(0x121B,0x3F);		
    i2cWrite_XC7021(0x121C,0x01);		
    i2cWrite_XC7021(0x121D,0xA0);		
    i2cWrite_XC7021(0x121E,0x01);		
    i2cWrite_XC7021(0x121F,0xEC);		
    i2cWrite_XC7021(0x1220,0x00);		
    i2cWrite_XC7021(0x1221,0x88);		
    i2cWrite_XC7021(0x1222,0x04);		
    i2cWrite_XC7021(0x1223,0x70);		
    i2cWrite_XC7021(0x122e,0x00);		//sign
    i2cWrite_XC7021(0x122F,0x00);		
    i2cWrite_XC7021(0x1230,0x00);		
    i2cWrite_XC7021(0xD31,0x003);		
    i2cWrite_XC7021(0x1228,0x00);		
    i2cWrite_XC7021(0x1229,0x66);		
    i2cWrite_XC7021(0x122A,0x00);		
    i2cWrite_XC7021(0x122B,0x66);		
    i2cWrite_XC7021(0x122C,0x00);		
    i2cWrite_XC7021(0x122D,0xEC);				

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x1231,0x02);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x2000,0x37);
    i2cWrite_XC7021(0x2001,0x0d);
    i2cWrite_XC7021(0x2002,0x04);
    i2cWrite_XC7021(0x2003,0x02);
    i2cWrite_XC7021(0x2004,0x01);
    i2cWrite_XC7021(0x2005,0x01);
    i2cWrite_XC7021(0x2006,0x02);
    i2cWrite_XC7021(0x2007,0x03);
    i2cWrite_XC7021(0x2008,0x04);
    i2cWrite_XC7021(0x2009,0x05);
    i2cWrite_XC7021(0x200a,0x06);
    i2cWrite_XC7021(0x200b,0x1f);

    i2cWrite_XC7021(0x1907,0x38);
    i2cWrite_XC7021(0x1908,0x00);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0e00,0x34);
    i2cWrite_XC7021(0x0e01,0x08);
    i2cWrite_XC7021(0x0e02,0x0f);
    i2cWrite_XC7021(0x0e03,0x14);
    i2cWrite_XC7021(0x0e04,0x24);
    i2cWrite_XC7021(0x0e05,0x34);
    i2cWrite_XC7021(0x0e06,0x38);
    i2cWrite_XC7021(0x0e07,0x38);
    i2cWrite_XC7021(0x0e08,0x38);
    i2cWrite_XC7021(0x0e09,0x3f);
    i2cWrite_XC7021(0x0e0a,0x0d);
    i2cWrite_XC7021(0x0e0b,0x20);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x130e,0x08);
    i2cWrite_XC7021(0x130f,0x0d);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0f00,0x20);
    i2cWrite_XC7021(0x0f01,0x00);
    i2cWrite_XC7021(0x0f02,0x00);
    i2cWrite_XC7021(0x0f03,0x00);
    i2cWrite_XC7021(0x0f04,0x00);
    i2cWrite_XC7021(0x0f05,0x00);
    i2cWrite_XC7021(0x0f06,0x00);
    i2cWrite_XC7021(0x0f07,0x00);
    i2cWrite_XC7021(0x0f08,0x00);
    i2cWrite_XC7021(0x0f09,0x00);
    i2cWrite_XC7021(0x0f0a,0x0f);
    i2cWrite_XC7021(0x0f0b,0x00);
    i2cWrite_XC7021(0x0f0c,0x00);
    i2cWrite_XC7021(0x0f0d,0x00);
    i2cWrite_XC7021(0x0f0e,0x00);
    i2cWrite_XC7021(0x0f0f,0x00);

    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x0160,0x00);
    i2cWrite_XC7021(0x0162,0x40);
    i2cWrite_XC7021(0x0163,0x38);
    i2cWrite_XC7021(0x0164,0x30);
    i2cWrite_XC7021(0x0165,0x2c);
    i2cWrite_XC7021(0x0166,0x28);
    i2cWrite_XC7021(0x0167,0x24);
    i2cWrite_XC7021(0x0168,0x44);
    i2cWrite_XC7021(0x0169,0x3c);
    i2cWrite_XC7021(0x016a,0x34);
    i2cWrite_XC7021(0x016b,0x30);
    i2cWrite_XC7021(0x016c,0x2c);
    i2cWrite_XC7021(0x016d,0x28);

    //TOP
    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0000,0xcF);  //LENC OPEN
    i2cWrite_XC7021(0x0001,0xb3);  //a7

    i2cWrite_XC7021(0x0003,0xe5);
    i2cWrite_XC7021(0x0013,0x20);
    i2cWrite_XC7021(0x071c,0x0a);
    i2cWrite_XC7021(0x1700,0x09);
    i2cWrite_XC7021(0x1701,0x60);
    i2cWrite_XC7021(0x1702,0x60);
    i2cWrite_XC7021(0x1704,0x22);
    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x0027,0x01);
    i2cWrite_XC7021(0x0096,0x00);
    i2cWrite_XC7021(0x0097,0x20);
    i2cWrite_XC7021(0x009e,0x00);
    i2cWrite_XC7021(0x009f,0x20);


}

s32 XC7021_1080P_10fps(void)
{
    i2cWrite_XC7021(0xfffe,0x26); 
    i2cWrite_XC7021(0x8010,0x08);

    DEBUG_SIU("XC7021_1080P_10fps\n");
    //10fps
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x0004,0x00);
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x1300,0x04); //寫入暫存器的個數

    i2cWrite_XC7021(0x1301,0x33); //第一個寄存器地址H
    i2cWrite_XC7021(0x1302,0x40); //第一個寄存器地址L
    i2cWrite_XC7021(0x1303,0x09); //第一個寄存器的值
    i2cWrite_XC7021(0x1304,0x33); //第二個寄存器地址H
    i2cWrite_XC7021(0x1305,0x41); //第二個寄存器地址L
    i2cWrite_XC7021(0x1306,0x44); //第二個寄存器的值
    i2cWrite_XC7021(0x1307,0x32);
    i2cWrite_XC7021(0x1308,0x0c);
    i2cWrite_XC7021(0x1309,0x09);
    i2cWrite_XC7021(0x130a,0x32);
    i2cWrite_XC7021(0x130b,0x0d);
    i2cWrite_XC7021(0x130c,0xc4);

    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x0007,0x60); //sensor I2C地址
    i2cWrite_XC7021(0x000d,0x31); //[7:4]速度[3:0]模式
    i2cWrite_XC7021(0x0009,0x00);
    i2cWrite_XC7021(0x00c4,0x10);
    i2cWrite_XC7021(0x00c0,0x01);
    //OSTimeDly(20);

    i2cWrite_XC7021(0xfffe,0x26);
    i2cWrite_XC7021(0x4000,0xF9);
    i2cWrite_XC7021(0x6001,0x14);
    i2cWrite_XC7021(0x6005,0xc4);
    i2cWrite_XC7021(0x6006,0x0F);
    i2cWrite_XC7021(0x6007,0xA0);
    i2cWrite_XC7021(0x6008,0x0E);
    i2cWrite_XC7021(0x6009,0xFC);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0001,0x01);
    i2cWrite_XC7021(0x0004,0x18);
    i2cWrite_XC7021(0x0006,0x07);
    i2cWrite_XC7021(0x0007,0x80);
    i2cWrite_XC7021(0x0008,0x04);
    i2cWrite_XC7021(0x0009,0x38);
    i2cWrite_XC7021(0x000a,0x07);
    i2cWrite_XC7021(0x000b,0x80);
    i2cWrite_XC7021(0x000c,0x04);
    i2cWrite_XC7021(0x000d,0x38);
    i2cWrite_XC7021(0x0027,0xF7);



    i2cWrite_XC7021(0xfffe,0x14);

    i2cWrite_XC7021(0x00b4,0x02);
    i2cWrite_XC7021(0x00b6,0x0b);
    i2cWrite_XC7021(0x00b7,0xe0);
    i2cWrite_XC7021(0x00b8,0x0e);
    i2cWrite_XC7021(0x00b9,0x40);
    i2cWrite_XC7021(0x0092,0x43);
    i2cWrite_XC7021(0x0093,0x00);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0000,0xcF);
    i2cWrite_XC7021(0x0001,0xab);
    i2cWrite_XC7021(0x1907,0x3a);
    i2cWrite_XC7021(0x1908,0x01);
    OSTimeDly(10);
    
    i2cWrite_XC7021(0xfffe,0x26); 
    i2cWrite_XC7021(0x8010,0x0c);

}

s32 XC7021_720P_20fps()
{
    i2cWrite_XC7021(0xfffe,0x26);
    i2cWrite_XC7021(0x8010,0x08);

    DEBUG_SIU("XC7021_720P_20fps\n");

    //20fps
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x0004,0x00);
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x14);
    i2cWrite_XC7021(0x1300,0x04); //寫入暫存器的個數

    i2cWrite_XC7021(0x1301,0x33); //第一個寄存器地址H
    i2cWrite_XC7021(0x1302,0x40); //第一個寄存器地址L
    i2cWrite_XC7021(0x1303,0x04); //第一個寄存器的值
    i2cWrite_XC7021(0x1304,0x33); //第二個寄存器地址H
    i2cWrite_XC7021(0x1305,0x41); //第二個寄存器地址L
    i2cWrite_XC7021(0x1306,0x68); //第二個寄存器的值
    i2cWrite_XC7021(0x1307,0x32);
    i2cWrite_XC7021(0x1308,0x0c);
    i2cWrite_XC7021(0x1309,0x04);
    i2cWrite_XC7021(0x130a,0x32);
    i2cWrite_XC7021(0x130b,0x0d);
    i2cWrite_XC7021(0x130c,0xe8);

    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x0007,0x60); //sensor I2C地址
    i2cWrite_XC7021(0x000d,0x31); //[7:4]速度[3:0]模式
    i2cWrite_XC7021(0x0009,0x00);
    i2cWrite_XC7021(0x00c4,0x10);
    i2cWrite_XC7021(0x00c0,0x01);

    //OSTimeDly(20);

    i2cWrite_XC7021(0xfffe,0x26);
    i2cWrite_XC7021(0x4000,0xF9);
    i2cWrite_XC7021(0x6001,0x14);
    i2cWrite_XC7021(0x6005,0xc4);
    i2cWrite_XC7021(0x6006,0x0A);
    i2cWrite_XC7021(0x6007,0x8C);
    i2cWrite_XC7021(0x6008,0x09);
    i2cWrite_XC7021(0x6009,0xFC);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0001,0x11);
    i2cWrite_XC7021(0x0004,0x18);
    i2cWrite_XC7021(0x0006,0x07);
    i2cWrite_XC7021(0x0007,0x80);
    i2cWrite_XC7021(0x0008,0x04);
    i2cWrite_XC7021(0x0009,0x38);
    i2cWrite_XC7021(0x000a,0x05);
    i2cWrite_XC7021(0x000b,0x00);
    i2cWrite_XC7021(0x000c,0x02);
    i2cWrite_XC7021(0x000d,0xD0);
    i2cWrite_XC7021(0x0027,0xF7);

    i2cWrite_XC7021(0xfffe,0x14);

    i2cWrite_XC7021(0x00b4,0x02);
    i2cWrite_XC7021(0x00b6,0x18);
    i2cWrite_XC7021(0x00b7,0x0A);
    i2cWrite_XC7021(0x00b8,0x1c);
    i2cWrite_XC7021(0x00b9,0xda);
    i2cWrite_XC7021(0x0092,0x43);
    i2cWrite_XC7021(0x0093,0x00);

    i2cWrite_XC7021(0xfffe,0x30);
    i2cWrite_XC7021(0x0000,0xcF);
    i2cWrite_XC7021(0x0001,0xb3);
    i2cWrite_XC7021(0x1907,0x38);
    i2cWrite_XC7021(0x1908,0x00);
    OSTimeDly(10);
    
    i2cWrite_XC7021(0xfffe,0x26);
    i2cWrite_XC7021(0x8010,0x0c);
    
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
                    XC7021_Init_1080P();
                    SC2133_Init_1080P();
                }
                XC7021_1080P_10fps();
                FrameRate   = 10;

            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                if (justboot == 0)
                {
                    XC7021_Init_1080P();
                    SC2133_Init_1080P();
                }
                XC7021_720P_20fps();
                FrameRate   = 20;
            }
			else
		    {
    		    if (justboot == 0)
                {
                    XC7021_Init_1080P();
                    SC2133_Init_1080P();
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
                IsuScUpFIFODepth = 0x1d1d1111; //把MP的scalling的頻寬分給SP，提升scalling 效能 //避花屏
            }
            
            siuSetImage();
            #if( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) )
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

#if( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) )
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
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x81); //彩色黑白切換，09：彩色、81：黑白
        i2cWrite_XC7021(0x1701,0x48); //飽和度 Cb
        i2cWrite_XC7021(0x1702,0x48); //飽和度 Cr
        i2cWrite_XC7021(0x1704,0x1a); //對比度
        i2cWrite_XC7021(0x0f00,0xf0); //去噪，max：f0 
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x0e0a,0x56); //raw去噪, 太大會導致畫面不清晰;白天<6, 晚上<28
        i2cWrite_XC7021(0x130e,0x09); //RGB去噪, 建議<0x0a; 白天8,晚上9
        i2cWrite_XC7021(0x130f,0x0d);

#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x09); //彩色黑白切換，09：彩色、81：黑白
        i2cWrite_XC7021(0x1701,0x48); //飽和度 Cb
        i2cWrite_XC7021(0x1702,0x48); //飽和度 Cr
        i2cWrite_XC7021(0x1704,0x22); //對比度
        i2cWrite_XC7021(0x0f00,0x20); //去噪，max：f0 
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x0e0a,0x04);
        i2cWrite_XC7021(0x130e,0x08);
        i2cWrite_XC7021(0x130f,0x0d);
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
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x81);//彩色黑白切?，09：彩色、81：黑白
        i2cWrite_XC7021(0x1701,0x60);//飽和度 Cb
        i2cWrite_XC7021(0x1702,0x60);//飽和度 Cr
        i2cWrite_XC7021(0x1704,0x22);//對比度
        i2cWrite_XC7021(0x0f00,0x20);//去噪，max：f0       
#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x09);//彩色黑白切?，09：彩色、81：黑白
        i2cWrite_XC7021(0x1701,0x60);//飽和度 Cb
        i2cWrite_XC7021(0x1702,0x60);//飽和度 Cr
        i2cWrite_XC7021(0x1704,0x22);//對比度
        i2cWrite_XC7021(0x0f00,0x20);//去噪，max：f0
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





