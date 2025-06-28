
/*
Copyright (c) 2014 Mars Semiconductor Corp.

Module Name:

    siu_PO2210K_YUV.c

Abstract:

    The routines of Sensor Interface Unit.
    Control PO2210K 1080P sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2105/11/04  Amon Li Create  
*/


#include "general.h"
#if (Sensor_OPTION == Sensor_PO2210K_YUV601)
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

#if ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
    #define PO2210K_YUV601_FLAGTEST 1
    #if PO2210K_YUV601_FLAGTEST
    OS_FLAG_GRP  *SiuFlagGrp;
    #define SIU_EVET_SENSOR_SET      0x00000001
    #endif
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

DEF_AE_Tab AE_EG_Tab_VideoPrev_5060Hz[2][2][AE_E_Table_Num*Gain_Level];  //AE Index Table

u8 AE_Flicker_50_60_sel = SENSOR_AE_FLICKER_60HZ;
u8 siuY_TargetIndex     = 5;
u8 siuContrastIndex     = 5;
u8 siuSaturationIndex   = 5;
u8 siuSharpnessIndex    = 5;

#if ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) ||(HW_BOARD_OPTION  == MR9100_TX_RDI_USB) )
#define TAR_MID   0xF4
#define CON_MID   0x40
#define SAT_MID   0x40
#define SHA_MID   0x10
#define TAR_MID_N 0x20
#elif (HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4)
#define TAR_MID   0xF4
#define CON_MID   0x40
#define SAT_MID   0x40
#define SHA_MID   0x10
#define TAR_MID_N 0x20
#elif (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T || HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
#define TAR_MID   0x00
#define CON_MID   0x40
#define SAT_MID   0x48
#define SHA_MID   0x10
#define TAR_MID_N 0x00
#else
#define TAR_MID   0x00
#define CON_MID   0x40
#define SAT_MID   0x20
#define SHA_MID   0x10
#define TAR_MID_N 0x00
#endif
const s8 AETargetMeanTab[11]    =   {TAR_MID-0x50  , TAR_MID-0x40  , TAR_MID-0x30  , TAR_MID-0x20  , TAR_MID-0x10  , TAR_MID  , TAR_MID+0x10  , TAR_MID+0x20  , TAR_MID+0x30  , TAR_MID+0x40  , TAR_MID+0x50  };
const s8 AETargetMeanTab_N[11]  =   {TAR_MID_N-0x50, TAR_MID_N-0x40, TAR_MID_N-0x30, TAR_MID_N-0x20, TAR_MID_N-0x10, TAR_MID_N, TAR_MID_N+0x10, TAR_MID_N+0x20, TAR_MID_N+0x30, TAR_MID_N+0x40, TAR_MID_N+0x50};

const s8 AEContrastTab[11]      =   {CON_MID-0x40, CON_MID-0x38, CON_MID-0x30, CON_MID-0x20, CON_MID-0x10, CON_MID, CON_MID+0x10, CON_MID+20, CON_MID+0x30, CON_MID+0x38, CON_MID+0x40};

const s8 AESaturationTab[11]    =   {SAT_MID-20, SAT_MID-16, SAT_MID-12, SAT_MID-8, SAT_MID-4, SAT_MID, SAT_MID+4, SAT_MID+8, SAT_MID+12, SAT_MID+16, SAT_MID+20};

const s8 AESharpnessTab[11]     =   {0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A};

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
#if ( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) || (HW_BOARD_OPTION  == MR9100_TX_RDI_USB))
    #if ( (CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    u32 PO2210K_720P_FrameRate  = 20;
    u32 PO2210K_1080P_FrameRate  = 10;
    #else
    u32 PO2210K_720P_FrameRate  = 15;
    u32 PO2210K_1080P_FrameRate  = 10;
    #endif
#else
    #if ( (CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    u32 PO2210K_720P_FrameRate  = 20;
    u32 PO2210K_1080P_FrameRate  = 10;
    #else
    u32 PO2210K_720P_FrameRate  = 15;
    u32 PO2210K_1080P_FrameRate  = 10;
    #endif
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

s32 siuSetFlicker50_60Hz(int flicker_sel)
{

    if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        //DEBUG_SIU("60Hz\n");
        i2cWrite_PO2210K(0x03,0x00);
        i2cWrite_PO2210K(0x4A,0x08);
//        i2cWrite_PO2210K(0x03,0x00);
//        i2cWrite_PO2210K(0x54,0x00);
//        i2cWrite_PO2210K(0x55,0x5D);
//        i2cWrite_PO2210K(0x56,0x2A);
    }
    else //50Hz
    {
        //DEBUG_SIU("50Hz\n");
        i2cWrite_PO2210K(0x03,0x00);
        i2cWrite_PO2210K(0x4A,0x04);
//        i2cWrite_PO2210K(0x03,0x00);
//        i2cWrite_PO2210K(0x57,0x00);
//        i2cWrite_PO2210K(0x58,0x6F);
//        i2cWrite_PO2210K(0x59,0xCC);
    }

    return 1;
}

s32 siuSetExposureValue(s8 expBiasValue)
{	
    u32 waitFlag;
    u8  err;

#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuSetExposureValue SiuFlagGrp is 0x%x.\n", err);
		return ;
	}
#endif
    if((expBiasValue > 10) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetExposureValue(%d) fail!!!\n", expBiasValue);
        #if PO2210K_YUV601_FLAGTEST
            OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
        #endif
        return 0;
    }
	siuY_TargetIndex    = expBiasValue;
    //DEBUG_SIU("siuSetExposureValue(%d) \n", expBiasValue);
    
    //ybrightness
    #if ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
        if(DayNightMode == SIU_NIGHT_MODE)
        {
            i2cWrite_PO2210K(0x03,0x02);
            i2cWrite_PO2210K(0x95 ,AETargetMeanTab_N[siuY_TargetIndex]);
            i2cWrite_PO2210K(0x96 ,AETargetMeanTab_N[siuY_TargetIndex]);
            i2cWrite_PO2210K(0x97 ,AETargetMeanTab_N[siuY_TargetIndex]);

        }
        else if(DayNightMode == SIU_DAY_MODE)
        {
            i2cWrite_PO2210K(0x03,0x02);
            i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
            i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
            i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
        }
    #else
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
    #endif

#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif
	return 1;
}

#if (HW_BOARD_OPTION == MR9100_TX_RDI_USB)
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

#else
s32 siuSetContrast(s8 expBiasValue)
{
//    DEBUG_SIU("siuSetContrast(%d) \n", expBiasValue);
    u32 waitFlag;
    u8  err;
#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuSetContrast SiuFlagGrp is 0x%x.\n", err);
        #if PO2210K_YUV601_FLAGTEST
            OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
        #endif
        return ;
	}
#endif
    if((expBiasValue > 10) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetContrast(%d) fail!!!\n", expBiasValue);
        return 0;
    }
    siuContrastIndex = expBiasValue;
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE8 ,AEContrastTab[siuContrastIndex]);
    i2cWrite_PO2210K(0xE9 ,AEContrastTab[siuContrastIndex]);
    i2cWrite_PO2210K(0xEA ,AEContrastTab[siuContrastIndex]);
#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif
    return 1;
}

s32 siuSetSaturation(s8 expBiasValue)
{

    u32 waitFlag;
    u8  err;
#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuSetSaturation SiuFlagGrp is 0x%x.\n", err);
		return ;
	}
#endif
//    DEBUG_SIU("siuSetSaturation(%d) \n", expBiasValue);
    if((expBiasValue > 10) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetSaturation(%d) fail!!!\n", expBiasValue);
        #if PO2210K_YUV601_FLAGTEST
            OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
        #endif
        return 0;
    }
    siuSaturationIndex = expBiasValue;
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0D ,AESaturationTab[siuSaturationIndex]);
#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif
    return 1;
}

s32 siuSetSharpness(s8 expBiasValue)
{

#if 0 //  會影響壓縮不調整
//    DEBUG_SIU("siuSetSharpness(%d) \n", expBiasValue);
    if((expBiasValue > 10) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetSharpness(%d) fail!!!\n", expBiasValue);
        return 0;
    }
    siuSharpnessIndex = expBiasValue;
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x5B ,AESharpnessTab[siuSharpnessIndex]);
    i2cWrite_PO2210K(0x5C ,AESharpnessTab[siuSharpnessIndex]);
    i2cWrite_PO2210K(0x5D ,AESharpnessTab[siuSharpnessIndex]);
#endif

    return 1;
}
#endif
s32 siuSetImage(void)
{
    #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
         (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
         (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1)  || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
         (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
    siuSetFlicker50_60Hz(iconflag[UI_MENU_SETIDX_TX_FLICKER]);
    #endif
    siuSetExposureValue(iconflag[UI_MENU_SETIDX_TX_BRIGHT]);
    siuSetContrast(iconflag[UI_MENU_SETIDX_TX_CONTRAST]);
    siuSetSaturation(iconflag[UI_MENU_SETIDX_TX_SATURATION]);
    siuSetSharpness(iconflag[UI_MENU_SETIDX_TX_SHARPNESS]);
    #if (SENSOR_ROW_COL_MIRROR==1)
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x05,0x00);
    #else
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x05,0x03);
    #endif
    
	return 1;
}

#if ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811)||\
      (HW_BOARD_OPTION == MR9100_TX_RDI_USB) )
void SetPO2210K_image_Quality(void)
{

    u32 waitFlag;
    u8  err;
#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: SetPO2210K_image_Quality SiuFlagGrp is 0x%x.\n", err);
		return ;
	}
#endif

//////////////////////////////////////////////////////////////////////// ablc
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x38,0x90); // analog_control_02
    i2cWrite_PO2210K(0x3D,0x2F); // analog_control_07

    i2cWrite_PO2210K(0x03,0x01); // bank B
    i2cWrite_PO2210K(0x1F,0x51); // bayer_control_10
    i2cWrite_PO2210K(0x20,0xA8); // Median value for filter and Average value for selection
    i2cWrite_PO2210K(0xA3,0xE0); // blc_top_th
    i2cWrite_PO2210K(0xA4,0x70); // blc_bot_th
    i2cWrite_PO2210K(0xA5,0x02); // ablc_update

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x06,0xA1); // auto_control_3[0] : ablc fitting enable

    // fitting x reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xC7,0x00); // overOBP_xref0 
    i2cWrite_PO2210K(0xC8,0x08); // overOBP_xref1 
    i2cWrite_PO2210K(0xC9,0x1E); // overOBP_xref2 
    i2cWrite_PO2210K(0xCA,0x32); // overOBP_xref3 
    i2cWrite_PO2210K(0xCB,0x58); // overOBP_xref4 
       
    // fitting y reference
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xDC,0x00); // overOBP_yref0 
    i2cWrite_PO2210K(0xDD,0x16); // overOBP_yref1 
    i2cWrite_PO2210K(0xDE,0x1B); // overOBP_yref2 
    i2cWrite_PO2210K(0xE0,0x25); // overOBP_yref3 
    i2cWrite_PO2210K(0xE1,0x30); // overOBP_yref4 		

    //////////////////////////////////////////////////////////////////////// intp
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x30,0x00);	// intp_w0       (10)
    i2cWrite_PO2210K(0x31,0xFF);	// intp_x0       (00)
    i2cWrite_PO2210K(0x32,0x40);	// intp_slope    (40)

    //////////////////////////////////////////////////////////////////////// blf
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);	// [4] edge_blf_mode : 0=new, 1=old	FB

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x33,0x00);	// blf_w0_ref0	00
    i2cWrite_PO2210K(0x34,0x40);	// blf_w0_ref1	00
    i2cWrite_PO2210K(0x35,0x40);	// blf_w0_ref2	00

    i2cWrite_PO2210K(0x37,0x20);	// blf_x0	20
    i2cWrite_PO2210K(0x38,0x40);	// blf_slope	40

    i2cWrite_PO2210K(0x39,0x7F);	// blf_c0	80	7F
    i2cWrite_PO2210K(0x3A,0x78);	// blf_c1	60	78
    i2cWrite_PO2210K(0x3B,0x63);	// blf_c2	40	63
    i2cWrite_PO2210K(0x3C,0x3F);	// blf_c3	20	2F
    i2cWrite_PO2210K(0x3D,0x2B);	// blf_c4	10	0B
    i2cWrite_PO2210K(0x3E,0x18);	// blf_c5	08	00

    //////////////////////////////////////////////////////////////////////// sc
    i2cWrite_PO2210K(0x03,0x09);
    i2cWrite_PO2210K(0x04,0x03); //	acce_ctrl_0 [1]:acce enable, [0]:histogram enable (00)
    i2cWrite_PO2210K(0x6D,0x04); // ac_ctrl_0 [2]:AE relate mode

    i2cWrite_PO2210K(0x49,0x30);	// ce_th      (20)
    i2cWrite_PO2210K(0x4A,0x10);	// ce_x0      (40)
    i2cWrite_PO2210K(0x4B,0x40);	// ce_slope   (40)

    i2cWrite_PO2210K(0xAD,0x08); //08 // lpf_w1 (08)
    i2cWrite_PO2210K(0xAE,0x10); //10 // lpf_w2 (18)
    i2cWrite_PO2210K(0xAF,0x20); //20 // lpf_w3 (40)
    i2cWrite_PO2210K(0xB0,0x10); //10 // lpf_w4 (18)
    i2cWrite_PO2210K(0xB1,0x08); //08 // lpf_w5 (08)

    i2cWrite_PO2210K(0xB2,0x04); // ac_offset
    i2cWrite_PO2210K(0xB3,0x60); // max_ac_gain0
    i2cWrite_PO2210K(0xB4,0x60); // max_ac_gain1
    i2cWrite_PO2210K(0xB5,0x40); // max_ac_gain2

    i2cWrite_PO2210K(0xB7,0x40); // min_ac_gain
    i2cWrite_PO2210K(0xB8,0x03); // ac_speed
    i2cWrite_PO2210K(0xB9,0x02); // ac_lock
    i2cWrite_PO2210K(0xBB,0x04); // ac_frame

    i2cWrite_PO2210K(0x8E,0x00); // ac_cv_w0 
    i2cWrite_PO2210K(0x8F,0x04); // ac_cv_w1 
    i2cWrite_PO2210K(0x90,0x06); // ac_cv_w2 
    i2cWrite_PO2210K(0x91,0x06); // ac_cv_w3 
    i2cWrite_PO2210K(0x92,0x04); // ac_cv_w4 
    i2cWrite_PO2210K(0x93,0x03); // ac_cv_w5 
    i2cWrite_PO2210K(0x94,0x01); // ac_cv_w6 
    i2cWrite_PO2210K(0x95,0x00); // ac_cv_w7 
    
    //////////////////////////////////////////////////////////////////////// gm

    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x3D,0x00);
    i2cWrite_PO2210K(0x3E,0x03);
    i2cWrite_PO2210K(0x3F,0x0C);
    i2cWrite_PO2210K(0x40,0x19);
    i2cWrite_PO2210K(0x41,0x26);
    i2cWrite_PO2210K(0x42,0x3F);
    i2cWrite_PO2210K(0x43,0x52);
    i2cWrite_PO2210K(0x44,0x6E);
    i2cWrite_PO2210K(0x45,0x82);
    i2cWrite_PO2210K(0x46,0xA1);
    i2cWrite_PO2210K(0x47,0xB9);
    i2cWrite_PO2210K(0x48,0xCE);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0xF0);
    i2cWrite_PO2210K(0x4B,0xFF);

    //gamma curve fitting
    i2cWrite_PO2210K(0x4C,0x00);
    i2cWrite_PO2210K(0x4D,0x11);
    i2cWrite_PO2210K(0x4E,0x1B);
    i2cWrite_PO2210K(0x4F,0x23);
    i2cWrite_PO2210K(0x50,0x2A);
    i2cWrite_PO2210K(0x51,0x37);
    i2cWrite_PO2210K(0x52,0x42);
    i2cWrite_PO2210K(0x53,0x56);
    i2cWrite_PO2210K(0x54,0x68);
    i2cWrite_PO2210K(0x55,0x87);
    i2cWrite_PO2210K(0x56,0xA3);
    i2cWrite_PO2210K(0x57,0xBC);
    i2cWrite_PO2210K(0x58,0xD4);
    i2cWrite_PO2210K(0x59,0xEA);
    i2cWrite_PO2210K(0x5A,0xFF);

    
    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x03);
    i2cWrite_PO2210K(0x5D,0x0C);
    i2cWrite_PO2210K(0x5E,0x19);
    i2cWrite_PO2210K(0x5F,0x26);
    i2cWrite_PO2210K(0x60,0x3F);
    i2cWrite_PO2210K(0x61,0x52);
    i2cWrite_PO2210K(0x62,0x6E);
    i2cWrite_PO2210K(0x63,0x82);
    i2cWrite_PO2210K(0x64,0xA1);
    i2cWrite_PO2210K(0x65,0xB9);
    i2cWrite_PO2210K(0x66,0xCE);
    i2cWrite_PO2210K(0x67,0xE0);
    i2cWrite_PO2210K(0x68,0xF0);
    i2cWrite_PO2210K(0x69,0xFF);

    //////////////////////////////////////////////////////////////////////// awb

    // Set AWB Sampling Boundary
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x40);
    i2cWrite_PO2210K(0x5E,0xA0);
    i2cWrite_PO2210K(0x5F,0x01);
    i2cWrite_PO2210K(0x60,0x02);
    i2cWrite_PO2210K(0x61,0x50);
    i2cWrite_PO2210K(0x62,0x02);
    i2cWrite_PO2210K(0x63,0x00);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x6E);
    i2cWrite_PO2210K(0x66,0x45);

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x67,0x27);
    i2cWrite_PO2210K(0x68,0x4F);
    i2cWrite_PO2210K(0x69,0x64);
    i2cWrite_PO2210K(0x6A,0xC4);
    i2cWrite_PO2210K(0x6B,0x0A);
    i2cWrite_PO2210K(0x6C,0x46);
    i2cWrite_PO2210K(0x6D,0x32);
    i2cWrite_PO2210K(0x6E,0x78);
    i2cWrite_PO2210K(0x6F,0x37);
    i2cWrite_PO2210K(0x70,0xAF);
    i2cWrite_PO2210K(0x71,0x32);
    i2cWrite_PO2210K(0x72,0x23);

    //Ai2cWrite_PO2210K(0xB option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x7E,0x08);
    i2cWrite_PO2210K(0x7F,0x04);

    //lens / cs axis
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0A,0x3E);
    i2cWrite_PO2210K(0x0B,0x5D);
    i2cWrite_PO2210K(0x0C,0x6C);

    ////////////////////////////////////////////////////////////////////// color

    //Color correction
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x33,0x2F);
    i2cWrite_PO2210K(0x34,0x85);
    i2cWrite_PO2210K(0x35,0x8A);
    i2cWrite_PO2210K(0x36,0x90);
    i2cWrite_PO2210K(0x37,0x3E);
    i2cWrite_PO2210K(0x38,0x8E);
    i2cWrite_PO2210K(0x39,0x91);
    i2cWrite_PO2210K(0x3A,0x81);
    i2cWrite_PO2210K(0x3B,0x32);

    //Color saturation
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x80,0x1B);
    i2cWrite_PO2210K(0x81,0x05);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x1E);
    
    //Color saturation weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0D,0x40);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x0B,0x86); // user_cs_mode [3]

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x8A,0xF0);	// dc_y2

    //////////////////////////////////////////////////////////////// edge

    //Edge control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);
    i2cWrite_PO2210K(0x09,0x00);
    i2cWrite_PO2210K(0x0B,0x82);

    //sharpness control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x70,0x40);
    i2cWrite_PO2210K(0x71,0x40);

    //Edge gamma curve
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x48,0x50);
    i2cWrite_PO2210K(0x49,0x40);
    i2cWrite_PO2210K(0x4A,0x30);
    i2cWrite_PO2210K(0x4B,0x20);
    i2cWrite_PO2210K(0x4C,0x18);
    i2cWrite_PO2210K(0x4D,0x10);
    i2cWrite_PO2210K(0x4E,0x10);

    //edge_gain_lf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x53,0x18);
    i2cWrite_PO2210K(0x54,0x10);
    i2cWrite_PO2210K(0x55,0x08);

    //edge_gain_ghf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x57,0x10);
    i2cWrite_PO2210K(0x58,0x10);
    i2cWrite_PO2210K(0x59,0x10);

    //edge_gain_ehf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x5B,0x10);
    i2cWrite_PO2210K(0x5C,0x10);
    i2cWrite_PO2210K(0x5D,0x10);

    ////////////////////////////////////////////////////////////////////////// vec

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x93,0x0C); // vec_en (00)

    // Set sample range for Vector Control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x94,0x14);
    i2cWrite_PO2210K(0x95,0x1B);
    i2cWrite_PO2210K(0x98,0x1C);
    i2cWrite_PO2210K(0x99,0x26);
    i2cWrite_PO2210K(0x9C,0x35);
    i2cWrite_PO2210K(0x9D,0x42);
    i2cWrite_PO2210K(0xA0,0x44);
    i2cWrite_PO2210K(0xA1,0x62);
    i2cWrite_PO2210K(0xA4,0x64);
    i2cWrite_PO2210K(0xA5,0x72);
    i2cWrite_PO2210K(0xA8,0x76);
    i2cWrite_PO2210K(0xA9,0x80);

    // Set hue & saturation (a)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAC,0x90);
    i2cWrite_PO2210K(0xAF,0x04);
    i2cWrite_PO2210K(0xB2,0x07);
    i2cWrite_PO2210K(0xB5,0x04);
    i2cWrite_PO2210K(0xB8,0x82);
    i2cWrite_PO2210K(0xBB,0x04);
    i2cWrite_PO2210K(0xBE,0x92);
    i2cWrite_PO2210K(0xC1,0x04);
    i2cWrite_PO2210K(0xC4,0x85);
    i2cWrite_PO2210K(0xC7,0x04);
    i2cWrite_PO2210K(0xCA,0x86);
    i2cWrite_PO2210K(0xCD,0x04);

    // Set hue & saturation (b)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAD,0x88);
    i2cWrite_PO2210K(0xB0,0x04);
    i2cWrite_PO2210K(0xB3,0x0B);
    i2cWrite_PO2210K(0xB6,0x00);
    i2cWrite_PO2210K(0xB9,0x05);
    i2cWrite_PO2210K(0xBC,0x04);
    i2cWrite_PO2210K(0xBF,0x8B);
    i2cWrite_PO2210K(0xC2,0x04);
    i2cWrite_PO2210K(0xC5,0x85);
    i2cWrite_PO2210K(0xC8,0x04);
    i2cWrite_PO2210K(0xCB,0x84);
    i2cWrite_PO2210K(0xCE,0x04);

    // Set hue & saturation (c)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAE,0x88);
    i2cWrite_PO2210K(0xB1,0x04);
    i2cWrite_PO2210K(0xB4,0x82);
    i2cWrite_PO2210K(0xB7,0x04);
    i2cWrite_PO2210K(0xBA,0x01);
    i2cWrite_PO2210K(0xBD,0x04);
    i2cWrite_PO2210K(0xC0,0x00);
    i2cWrite_PO2210K(0xC3,0x06);
    i2cWrite_PO2210K(0xC6,0x07);
    i2cWrite_PO2210K(0xC9,0x04);
    i2cWrite_PO2210K(0xCC,0x00);
    i2cWrite_PO2210K(0xCF,0x04);

    //////////////////////////////////////////////////////////////////////// dark

    //Darkness X reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x98,0x00);
    i2cWrite_PO2210K(0x99,0x04);
    i2cWrite_PO2210K(0x9A,0x00);
    i2cWrite_PO2210K(0x9B,0x10);
    i2cWrite_PO2210K(0x9C,0x00);
    i2cWrite_PO2210K(0x9D,0x20);

    //dark_y_weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7A,0x40);
    i2cWrite_PO2210K(0x7B,0x40);
    i2cWrite_PO2210K(0x7C,0x80);

    //dark_ccr
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7E,0x04);
    i2cWrite_PO2210K(0x7F,0x04);
    i2cWrite_PO2210K(0x80,0x04);

    //dark_dc
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x0A);
    i2cWrite_PO2210K(0x84,0x24);

    //ycont_th2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE4,0x80);
    i2cWrite_PO2210K(0xE5,0x80);
    i2cWrite_PO2210K(0xE6,0x80);

    //ycont_slope2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE8,0x40);
    i2cWrite_PO2210K(0xE9,0x40);
    i2cWrite_PO2210K(0xEA,0x40);

    //dark_edge_gm
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x4F,0x00);
    i2cWrite_PO2210K(0x50,0x00);
    i2cWrite_PO2210K(0x51,0x00);

    //dark_ec_pth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x60,0x04);
    i2cWrite_PO2210K(0x61,0x04);
    i2cWrite_PO2210K(0x62,0x04);

    //dark_ec_mth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x04);
    i2cWrite_PO2210K(0x66,0x04);

    //dark_dpc_p
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1A,0x00);
    i2cWrite_PO2210K(0x1B,0x04);
    i2cWrite_PO2210K(0x1C,0x7F);

    //dark_dpc_n
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x08);
    i2cWrite_PO2210K(0x20,0x18);

    //ybrightness
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x95,0xF4);
    i2cWrite_PO2210K(0x96,0xF4);
    i2cWrite_PO2210K(0x97,0xF4);

    //blf_darkness
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x3F,0x38);
    i2cWrite_PO2210K(0x40,0x10);
    i2cWrite_PO2210K(0x41,0x00);

    //dark_ec_pmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x68,0x7F);
    i2cWrite_PO2210K(0x69,0x10);
    i2cWrite_PO2210K(0x6A,0x08);

    //dark_ec_mmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x6C,0x7F);
    i2cWrite_PO2210K(0x6D,0x20);
    i2cWrite_PO2210K(0x6E,0x08);

    //hf_dir_max
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x24,0x1C);
    i2cWrite_PO2210K(0x25,0x1C);
    i2cWrite_PO2210K(0x26,0x7F);

    //intp_dir_th
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x28,0x08);
    i2cWrite_PO2210K(0x29,0x08);
    i2cWrite_PO2210K(0x2A,0x7F);
//////////////////////////////////////
    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x68);
    i2cWrite_PO2210K(0x3C,0x60);
    i2cWrite_PO2210K(0x3D,0x58);
    i2cWrite_PO2210K(0x3E,0x60);
    i2cWrite_PO2210K(0x3F,0x60);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);
    
    //awb rg/bg ratio fitting
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x75,0x7A);
    i2cWrite_PO2210K(0x76,0x79);
    i2cWrite_PO2210K(0x77,0x7A);
    i2cWrite_PO2210K(0x78,0x79);
    //Front black control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x05,0x64);
    i2cWrite_PO2210K(0x06,0xA1);
    //Front black fitting
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0xB3,0x8A);
    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x08);
#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif

}
#elif (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK) 
void SetPO2210K_image_Quality(void)
{
    // 20160304 setting
    //////////////////////////////////////////////////////////////////////// ablc
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x38,0x90); // analog_control_02
    i2cWrite_PO2210K(0x3D,0x2F); // analog_control_07

    i2cWrite_PO2210K(0x03,0x01); // bank B
    i2cWrite_PO2210K(0x1F,0x51); // bayer_control_10
    i2cWrite_PO2210K(0x20,0xA8); // Median value for filter and Average value for selection
    i2cWrite_PO2210K(0xA3,0xE0); // blc_top_th
    i2cWrite_PO2210K(0xA4,0x70); // blc_bot_th
    i2cWrite_PO2210K(0xA5,0x02); // ablc_update

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x06,0xA1); // auto_control_3[0] : ablc fitting enable

    // fitting x reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xC7,0x00); // overOBP_xref0 
    i2cWrite_PO2210K(0xC8,0x08); // overOBP_xref1 
    i2cWrite_PO2210K(0xC9,0x1E); // overOBP_xref2 
    i2cWrite_PO2210K(0xCA,0x32); // overOBP_xref3 
    i2cWrite_PO2210K(0xCB,0x58); // overOBP_xref4 
       
    // fitting y reference
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xDC,0x00); // overOBP_yref0 
    i2cWrite_PO2210K(0xDD,0x16); // overOBP_yref1 
    i2cWrite_PO2210K(0xDE,0x1B); // overOBP_yref2 
    i2cWrite_PO2210K(0xE0,0x25); // overOBP_yref3 
    i2cWrite_PO2210K(0xE1,0x30); // overOBP_yref4 		

    //////////////////////////////////////////////////////////////////////// intp
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x30,0x00);	// intp_w0       (10)
    i2cWrite_PO2210K(0x31,0xFF);	// intp_x0       (00)
    i2cWrite_PO2210K(0x32,0x40);	// intp_slope    (40)

    //////////////////////////////////////////////////////////////////////// blf
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);	// [4] edge_blf_mode : 0=new, 1=old	FB

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x33,0x00);	// blf_w0_ref0	00
    i2cWrite_PO2210K(0x34,0x40);	// blf_w0_ref1	00
    i2cWrite_PO2210K(0x35,0x40);	// blf_w0_ref2	00

    i2cWrite_PO2210K(0x37,0x20);	// blf_x0	20
    i2cWrite_PO2210K(0x38,0x40);	// blf_slope	40

    i2cWrite_PO2210K(0x39,0x7F);	// blf_c0	80	7F
    i2cWrite_PO2210K(0x3A,0x78);	// blf_c1	60	78
    i2cWrite_PO2210K(0x3B,0x63);	// blf_c2	40	63
    i2cWrite_PO2210K(0x3C,0x3F);	// blf_c3	20	2F
    i2cWrite_PO2210K(0x3D,0x2B);	// blf_c4	10	0B
    i2cWrite_PO2210K(0x3E,0x18);	// blf_c5	08	00

    //////////////////////////////////////////////////////////////////////// sc
    i2cWrite_PO2210K(0x03,0x09);
    i2cWrite_PO2210K(0x04,0x03); //	acce_ctrl_0 [1]:acce enable, [0]:histogram enable (00)
    i2cWrite_PO2210K(0x6D,0x04); // ac_ctrl_0 [2]:AE relate mode

    i2cWrite_PO2210K(0x49,0x30);	// ce_th      (20)
    i2cWrite_PO2210K(0x4A,0x10);	// ce_x0      (40)
    i2cWrite_PO2210K(0x4B,0x40);	// ce_slope   (40)

    i2cWrite_PO2210K(0xAD,0x08); //08 // lpf_w1 (08)
    i2cWrite_PO2210K(0xAE,0x10); //10 // lpf_w2 (18)
    i2cWrite_PO2210K(0xAF,0x20); //20 // lpf_w3 (40)
    i2cWrite_PO2210K(0xB0,0x10); //10 // lpf_w4 (18)
    i2cWrite_PO2210K(0xB1,0x08); //08 // lpf_w5 (08)

    i2cWrite_PO2210K(0xB2,0x04); // ac_offset
    i2cWrite_PO2210K(0xB3,0x60); // max_ac_gain0
    i2cWrite_PO2210K(0xB4,0x60); // max_ac_gain1
    i2cWrite_PO2210K(0xB5,0x40); // max_ac_gain2

    i2cWrite_PO2210K(0xB7,0x40); // min_ac_gain
    i2cWrite_PO2210K(0xB8,0x03); // ac_speed
    i2cWrite_PO2210K(0xB9,0x02); // ac_lock
    i2cWrite_PO2210K(0xBB,0x04); // ac_frame

    i2cWrite_PO2210K(0x8E,0x00); // ac_cv_w0 
    i2cWrite_PO2210K(0x8F,0x04); // ac_cv_w1 
    i2cWrite_PO2210K(0x90,0x06); // ac_cv_w2 
    i2cWrite_PO2210K(0x91,0x06); // ac_cv_w3 
    i2cWrite_PO2210K(0x92,0x04); // ac_cv_w4 
    i2cWrite_PO2210K(0x93,0x03); // ac_cv_w5 
    i2cWrite_PO2210K(0x94,0x01); // ac_cv_w6 
    i2cWrite_PO2210K(0x95,0x00); // ac_cv_w7 
    
    //////////////////////////////////////////////////////////////////////// gm

    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x3D,0x00);
    i2cWrite_PO2210K(0x3E,0x0B);
    i2cWrite_PO2210K(0x3F,0x1e);
    i2cWrite_PO2210K(0x40,0x2a);
    i2cWrite_PO2210K(0x41,0x33);
    i2cWrite_PO2210K(0x42,0x41);
    i2cWrite_PO2210K(0x43,0x53);
    i2cWrite_PO2210K(0x44,0x73);
    i2cWrite_PO2210K(0x45,0x86);
    i2cWrite_PO2210K(0x46,0xA5);
    i2cWrite_PO2210K(0x47,0xBD);
    i2cWrite_PO2210K(0x48,0xD3);
    i2cWrite_PO2210K(0x49,0xE3);
    i2cWrite_PO2210K(0x4A,0xEF);
    i2cWrite_PO2210K(0x4B,0xFF);

    //gamma curve fitting
    i2cWrite_PO2210K(0x4C,0x00);
    i2cWrite_PO2210K(0x4D,0x11);
    i2cWrite_PO2210K(0x4E,0x1B);
    i2cWrite_PO2210K(0x4F,0x23);
    i2cWrite_PO2210K(0x50,0x2A);
    i2cWrite_PO2210K(0x51,0x37);
    i2cWrite_PO2210K(0x52,0x42);
    i2cWrite_PO2210K(0x53,0x56);
    i2cWrite_PO2210K(0x54,0x68);
    i2cWrite_PO2210K(0x55,0x87);
    i2cWrite_PO2210K(0x56,0xA3);
    i2cWrite_PO2210K(0x57,0xBC);
    i2cWrite_PO2210K(0x58,0xD4);
    i2cWrite_PO2210K(0x59,0xEA);
    i2cWrite_PO2210K(0x5A,0xFF);

    
    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x06);
    i2cWrite_PO2210K(0x5D,0x18);
    i2cWrite_PO2210K(0x5E,0x2c);
    i2cWrite_PO2210K(0x5F,0x3c);
    i2cWrite_PO2210K(0x60,0x54);
    i2cWrite_PO2210K(0x61,0x65);
    i2cWrite_PO2210K(0x62,0x7D);
    i2cWrite_PO2210K(0x63,0x8F);
    i2cWrite_PO2210K(0x64,0xAB);
    i2cWrite_PO2210K(0x65,0xC1);
    i2cWrite_PO2210K(0x66,0xD3);
    i2cWrite_PO2210K(0x67,0xE3);
    i2cWrite_PO2210K(0x68,0xF2);
    i2cWrite_PO2210K(0x69,0xFF);

    //////////////////////////////////////////////////////////////////////// awb

    // Set AWB Sampling Boundary
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x40);
    i2cWrite_PO2210K(0x5E,0xA0);
    i2cWrite_PO2210K(0x5F,0x01);
    i2cWrite_PO2210K(0x60,0x02);
    i2cWrite_PO2210K(0x61,0x50);
    i2cWrite_PO2210K(0x62,0x02);
    i2cWrite_PO2210K(0x63,0x00);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x6E);
    i2cWrite_PO2210K(0x66,0x45);

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x67,0x27);
    i2cWrite_PO2210K(0x68,0x4F);
    i2cWrite_PO2210K(0x69,0x64);
    i2cWrite_PO2210K(0x6A,0xC4);
    i2cWrite_PO2210K(0x6B,0x0A);
    i2cWrite_PO2210K(0x6C,0x46);
    i2cWrite_PO2210K(0x6D,0x32);
    i2cWrite_PO2210K(0x6E,0x78);
    i2cWrite_PO2210K(0x6F,0x37);
    i2cWrite_PO2210K(0x70,0xAF);
    i2cWrite_PO2210K(0x71,0x32);
    i2cWrite_PO2210K(0x72,0x23);

    //Ai2cWrite_PO2210K(0xB option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x7E,0x08);
    i2cWrite_PO2210K(0x7F,0x04);

    //lens / cs axis
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0A,0x3E);
    i2cWrite_PO2210K(0x0B,0x5D);
    i2cWrite_PO2210K(0x0C,0x6C);

    ////////////////////////////////////////////////////////////////////// color

    //Color correction
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x33,0x3D); //161123
    i2cWrite_PO2210K(0x34,0x85);
    i2cWrite_PO2210K(0x35,0x98); //161123
    i2cWrite_PO2210K(0x36,0x90);
    i2cWrite_PO2210K(0x37,0x3E);
    i2cWrite_PO2210K(0x38,0x8E);
    i2cWrite_PO2210K(0x39,0x91);
    i2cWrite_PO2210K(0x3A,0x81);
    i2cWrite_PO2210K(0x3B,0x32);

    //Color saturation
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x80,0x25);
    i2cWrite_PO2210K(0x81,0x05);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x1E);
    
    //Color saturation weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0D,0x29); //161123

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x0B,0x86); // user_cs_mode [3]

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x8A,0xF0);	// dc_y2

    //////////////////////////////////////////////////////////////// edge

    //Edge control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);
    i2cWrite_PO2210K(0x09,0x00);
    i2cWrite_PO2210K(0x0B,0x82);

    //sharpness control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x70,0x40);
    i2cWrite_PO2210K(0x71,0x40);

    //Edge gamma curve
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x48,0x50);
    i2cWrite_PO2210K(0x49,0x40);
    i2cWrite_PO2210K(0x4A,0x30);
    i2cWrite_PO2210K(0x4B,0x20);
    i2cWrite_PO2210K(0x4C,0x18);
    i2cWrite_PO2210K(0x4D,0x10);
    i2cWrite_PO2210K(0x4E,0x10);

    //edge_gain_lf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x53,0x18);
    i2cWrite_PO2210K(0x54,0x10);
    i2cWrite_PO2210K(0x55,0x08);

    //edge_gain_ghf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x57,0x10);
    i2cWrite_PO2210K(0x58,0x10);
    i2cWrite_PO2210K(0x59,0x10);

    //edge_gain_ehf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x5B,0x10);
    i2cWrite_PO2210K(0x5C,0x10);
    i2cWrite_PO2210K(0x5D,0x10);

    ////////////////////////////////////////////////////////////////////////// vec

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x93,0x0C); // vec_en (00)

    // Set sample range for Vector Control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x94,0x14);
    i2cWrite_PO2210K(0x95,0x1B);
    i2cWrite_PO2210K(0x98,0x1C);
    i2cWrite_PO2210K(0x99,0x26);
    i2cWrite_PO2210K(0x9C,0x35);
    i2cWrite_PO2210K(0x9D,0x42);
    i2cWrite_PO2210K(0xA0,0x44);
    i2cWrite_PO2210K(0xA1,0x62);
    i2cWrite_PO2210K(0xA4,0x64);
    i2cWrite_PO2210K(0xA5,0x72);
    i2cWrite_PO2210K(0xA8,0x76);
    i2cWrite_PO2210K(0xA9,0x80);

    // Set hue & saturation (a)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAC,0x90);
    i2cWrite_PO2210K(0xAF,0x04);
    i2cWrite_PO2210K(0xB2,0x07);
    i2cWrite_PO2210K(0xB5,0x04);
    i2cWrite_PO2210K(0xB8,0x82);
    i2cWrite_PO2210K(0xBB,0x04);
    i2cWrite_PO2210K(0xBE,0x92);
    i2cWrite_PO2210K(0xC1,0x04);
    i2cWrite_PO2210K(0xC4,0x85);
    i2cWrite_PO2210K(0xC7,0x04);
    i2cWrite_PO2210K(0xCA,0x86);
    i2cWrite_PO2210K(0xCD,0x04);

    // Set hue & saturation (b)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAD,0x88);
    i2cWrite_PO2210K(0xB0,0x04);
    i2cWrite_PO2210K(0xB3,0x0B);
    i2cWrite_PO2210K(0xB6,0x00);
    i2cWrite_PO2210K(0xB9,0x05);
    i2cWrite_PO2210K(0xBC,0x04);
    i2cWrite_PO2210K(0xBF,0x8B);
    i2cWrite_PO2210K(0xC2,0x04);
    i2cWrite_PO2210K(0xC5,0x85);
    i2cWrite_PO2210K(0xC8,0x04);
    i2cWrite_PO2210K(0xCB,0x84);
    i2cWrite_PO2210K(0xCE,0x04);

    // Set hue & saturation (c)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAE,0x88);
    i2cWrite_PO2210K(0xB1,0x04);
    i2cWrite_PO2210K(0xB4,0x82);
    i2cWrite_PO2210K(0xB7,0x04);
    i2cWrite_PO2210K(0xBA,0x01);
    i2cWrite_PO2210K(0xBD,0x04);
    i2cWrite_PO2210K(0xC0,0x00);
    i2cWrite_PO2210K(0xC3,0x06);
    i2cWrite_PO2210K(0xC6,0x07);
    i2cWrite_PO2210K(0xC9,0x04);
    i2cWrite_PO2210K(0xCC,0x00);
    i2cWrite_PO2210K(0xCF,0x04);

    //////////////////////////////////////////////////////////////////////// dark

    //Darkness X reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x98,0x00);
    i2cWrite_PO2210K(0x99,0x04);
    i2cWrite_PO2210K(0x9A,0x00);
    i2cWrite_PO2210K(0x9B,0x10);
    i2cWrite_PO2210K(0x9C,0x00);
    i2cWrite_PO2210K(0x9D,0x20);

    //dark_y_weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7A,0x40);
    i2cWrite_PO2210K(0x7B,0x40);
    i2cWrite_PO2210K(0x7C,0x80);

    //dark_ccr
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7E,0x04);
    i2cWrite_PO2210K(0x7F,0x04);
    i2cWrite_PO2210K(0x80,0x04);

    //dark_dc
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x0A);
    i2cWrite_PO2210K(0x84,0x24);

    //ycont_th2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE4,0x80);
    i2cWrite_PO2210K(0xE5,0x80);
    i2cWrite_PO2210K(0xE6,0x80);

    //ycont_slope2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE8,0x2A); //161123
    i2cWrite_PO2210K(0xE9,0x2A); //161123
    i2cWrite_PO2210K(0xEA,0x40);

    //dark_edge_gm
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x4F,0x00);
    i2cWrite_PO2210K(0x50,0x00);
    i2cWrite_PO2210K(0x51,0x00);

    //dark_ec_pth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x60,0x04);
    i2cWrite_PO2210K(0x61,0x04);
    i2cWrite_PO2210K(0x62,0x04);

    //dark_ec_mth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x04);
    i2cWrite_PO2210K(0x66,0x04);

    //dark_dpc_p
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1A,0x00);
    i2cWrite_PO2210K(0x1B,0x04);
    i2cWrite_PO2210K(0x1C,0x7F);

    //dark_dpc_n
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x08);
    i2cWrite_PO2210K(0x20,0x18);

    //ybrightness
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x95,0xE0);
    i2cWrite_PO2210K(0x96,0xE0);
    i2cWrite_PO2210K(0x97,0xE0);

    //blf_darkness
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x3F,0x38);
    i2cWrite_PO2210K(0x40,0x10);
    i2cWrite_PO2210K(0x41,0x00);

    //dark_ec_pmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x68,0x40);
    i2cWrite_PO2210K(0x69,0x10);
    i2cWrite_PO2210K(0x6A,0x08);

    //dark_ec_mmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x6C,0x40);
    i2cWrite_PO2210K(0x6D,0x20);
    i2cWrite_PO2210K(0x6E,0x08);

    //hf_dir_max
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x24,0x1C);
    i2cWrite_PO2210K(0x25,0x1C);
    i2cWrite_PO2210K(0x26,0x7F);

    //intp_dir_th
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x28,0x08);
    i2cWrite_PO2210K(0x29,0x08);
    i2cWrite_PO2210K(0x2A,0x7F);
       
}

#elif (HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4)
void SetPO2210K_image_Quality(void)
{

    u32 waitFlag;
    u8  err;

//////////////////////////////////////////////////////////////////////// ablc
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x38,0x90); // analog_control_02
    i2cWrite_PO2210K(0x3D,0x2F); // analog_control_07

    i2cWrite_PO2210K(0x03,0x01); // bank B
    i2cWrite_PO2210K(0x1F,0x51); // bayer_control_10
    i2cWrite_PO2210K(0x20,0xA8); // Median value for filter and Average value for selection
    i2cWrite_PO2210K(0xA3,0xE0); // blc_top_th
    i2cWrite_PO2210K(0xA4,0x70); // blc_bot_th
    i2cWrite_PO2210K(0xA5,0x02); // ablc_update

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x06,0xA1); // auto_control_3[0] : ablc fitting enable

    // fitting x reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xC7,0x00); // overOBP_xref0 
    i2cWrite_PO2210K(0xC8,0x08); // overOBP_xref1 
    i2cWrite_PO2210K(0xC9,0x1E); // overOBP_xref2 
    i2cWrite_PO2210K(0xCA,0x32); // overOBP_xref3 
    i2cWrite_PO2210K(0xCB,0x58); // overOBP_xref4 
       
    // fitting y reference
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xDC,0x00); // overOBP_yref0 
    i2cWrite_PO2210K(0xDD,0x16); // overOBP_yref1 
    i2cWrite_PO2210K(0xDE,0x1B); // overOBP_yref2 
    i2cWrite_PO2210K(0xE0,0x25); // overOBP_yref3 
    i2cWrite_PO2210K(0xE1,0x30); // overOBP_yref4 		

    //////////////////////////////////////////////////////////////////////// intp
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x30,0x00);	// intp_w0       (10)
    i2cWrite_PO2210K(0x31,0xFF);	// intp_x0       (00)
    i2cWrite_PO2210K(0x32,0x40);	// intp_slope    (40)

    //////////////////////////////////////////////////////////////////////// blf
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);	// [4] edge_blf_mode : 0=new, 1=old	FB

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x33,0x00);	// blf_w0_ref0	00
    i2cWrite_PO2210K(0x34,0x40);	// blf_w0_ref1	00
    i2cWrite_PO2210K(0x35,0x40);	// blf_w0_ref2	00

    i2cWrite_PO2210K(0x37,0x20);	// blf_x0	20
    i2cWrite_PO2210K(0x38,0x40);	// blf_slope	40

    i2cWrite_PO2210K(0x39,0x7F);	// blf_c0	80	7F
    i2cWrite_PO2210K(0x3A,0x78);	// blf_c1	60	78
    i2cWrite_PO2210K(0x3B,0x63);	// blf_c2	40	63
    i2cWrite_PO2210K(0x3C,0x3F);	// blf_c3	20	2F
    i2cWrite_PO2210K(0x3D,0x2B);	// blf_c4	10	0B
    i2cWrite_PO2210K(0x3E,0x18);	// blf_c5	08	00

    //////////////////////////////////////////////////////////////////////// sc
    i2cWrite_PO2210K(0x03,0x09);
    i2cWrite_PO2210K(0x04,0x03); //	acce_ctrl_0 [1]:acce enable, [0]:histogram enable (00)
    i2cWrite_PO2210K(0x6D,0x04); // ac_ctrl_0 [2]:AE relate mode

    i2cWrite_PO2210K(0x49,0x30);	// ce_th      (20)
    i2cWrite_PO2210K(0x4A,0x10);	// ce_x0      (40)
    i2cWrite_PO2210K(0x4B,0x40);	// ce_slope   (40)

    i2cWrite_PO2210K(0xAD,0x08); //08 // lpf_w1 (08)
    i2cWrite_PO2210K(0xAE,0x10); //10 // lpf_w2 (18)
    i2cWrite_PO2210K(0xAF,0x20); //20 // lpf_w3 (40)
    i2cWrite_PO2210K(0xB0,0x10); //10 // lpf_w4 (18)
    i2cWrite_PO2210K(0xB1,0x08); //08 // lpf_w5 (08)

    i2cWrite_PO2210K(0xB2,0x04); // ac_offset
    i2cWrite_PO2210K(0xB3,0x60); // max_ac_gain0
    i2cWrite_PO2210K(0xB4,0x60); // max_ac_gain1
    i2cWrite_PO2210K(0xB5,0x40); // max_ac_gain2

    i2cWrite_PO2210K(0xB7,0x40); // min_ac_gain
    i2cWrite_PO2210K(0xB8,0x03); // ac_speed
    i2cWrite_PO2210K(0xB9,0x02); // ac_lock
    i2cWrite_PO2210K(0xBB,0x04); // ac_frame

    i2cWrite_PO2210K(0x8E,0x00); // ac_cv_w0 
    i2cWrite_PO2210K(0x8F,0x04); // ac_cv_w1 
    i2cWrite_PO2210K(0x90,0x06); // ac_cv_w2 
    i2cWrite_PO2210K(0x91,0x06); // ac_cv_w3 
    i2cWrite_PO2210K(0x92,0x04); // ac_cv_w4 
    i2cWrite_PO2210K(0x93,0x03); // ac_cv_w5 
    i2cWrite_PO2210K(0x94,0x01); // ac_cv_w6 
    i2cWrite_PO2210K(0x95,0x00); // ac_cv_w7 
    
    //////////////////////////////////////////////////////////////////////// gm

    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x3D,0x00);
    i2cWrite_PO2210K(0x3E,0x03);
    i2cWrite_PO2210K(0x3F,0x0C);
    i2cWrite_PO2210K(0x40,0x19);
    i2cWrite_PO2210K(0x41,0x26);
    i2cWrite_PO2210K(0x42,0x3F);
    i2cWrite_PO2210K(0x43,0x52);
    i2cWrite_PO2210K(0x44,0x6E);
    i2cWrite_PO2210K(0x45,0x82);
    i2cWrite_PO2210K(0x46,0xA1);
    i2cWrite_PO2210K(0x47,0xB9);
    i2cWrite_PO2210K(0x48,0xCE);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0xF0);
    i2cWrite_PO2210K(0x4B,0xFF);

    //gamma curve fitting
    i2cWrite_PO2210K(0x4C,0x00);
    i2cWrite_PO2210K(0x4D,0x11);
    i2cWrite_PO2210K(0x4E,0x1B);
    i2cWrite_PO2210K(0x4F,0x23);
    i2cWrite_PO2210K(0x50,0x2A);
    i2cWrite_PO2210K(0x51,0x37);
    i2cWrite_PO2210K(0x52,0x42);
    i2cWrite_PO2210K(0x53,0x56);
    i2cWrite_PO2210K(0x54,0x68);
    i2cWrite_PO2210K(0x55,0x87);
    i2cWrite_PO2210K(0x56,0xA3);
    i2cWrite_PO2210K(0x57,0xBC);
    i2cWrite_PO2210K(0x58,0xD4);
    i2cWrite_PO2210K(0x59,0xEA);
    i2cWrite_PO2210K(0x5A,0xFF);

    
    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x03);
    i2cWrite_PO2210K(0x5D,0x0C);
    i2cWrite_PO2210K(0x5E,0x19);
    i2cWrite_PO2210K(0x5F,0x26);
    i2cWrite_PO2210K(0x60,0x3F);
    i2cWrite_PO2210K(0x61,0x52);
    i2cWrite_PO2210K(0x62,0x6E);
    i2cWrite_PO2210K(0x63,0x82);
    i2cWrite_PO2210K(0x64,0xA1);
    i2cWrite_PO2210K(0x65,0xB9);
    i2cWrite_PO2210K(0x66,0xCE);
    i2cWrite_PO2210K(0x67,0xE0);
    i2cWrite_PO2210K(0x68,0xF0);
    i2cWrite_PO2210K(0x69,0xFF);

    //////////////////////////////////////////////////////////////////////// awb

    // Set AWB Sampling Boundary
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x40);
    i2cWrite_PO2210K(0x5E,0xA0);
    i2cWrite_PO2210K(0x5F,0x01);
    i2cWrite_PO2210K(0x60,0x02);
    i2cWrite_PO2210K(0x61,0x50);
    i2cWrite_PO2210K(0x62,0x02);
    i2cWrite_PO2210K(0x63,0x00);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x6E);
    i2cWrite_PO2210K(0x66,0x45);

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x67,0x27);
    i2cWrite_PO2210K(0x68,0x4F);
    i2cWrite_PO2210K(0x69,0x64);
    i2cWrite_PO2210K(0x6A,0xC4);
    i2cWrite_PO2210K(0x6B,0x0A);
    i2cWrite_PO2210K(0x6C,0x46);
    i2cWrite_PO2210K(0x6D,0x32);
    i2cWrite_PO2210K(0x6E,0x78);
    i2cWrite_PO2210K(0x6F,0x37);
    i2cWrite_PO2210K(0x70,0xAF);
    i2cWrite_PO2210K(0x71,0x32);
    i2cWrite_PO2210K(0x72,0x23);

    //Ai2cWrite_PO2210K(0xB option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x7E,0x08);
    i2cWrite_PO2210K(0x7F,0x04);

    //lens / cs axis
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0A,0x3E);
    i2cWrite_PO2210K(0x0B,0x5D);
    i2cWrite_PO2210K(0x0C,0x6C);

    ////////////////////////////////////////////////////////////////////// color

    //Color correction
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x33,0x2F);
    i2cWrite_PO2210K(0x34,0x85);
    i2cWrite_PO2210K(0x35,0x8A);
    i2cWrite_PO2210K(0x36,0x90);
    i2cWrite_PO2210K(0x37,0x3E);
    i2cWrite_PO2210K(0x38,0x8E);
    i2cWrite_PO2210K(0x39,0x91);
    i2cWrite_PO2210K(0x3A,0x81);
    i2cWrite_PO2210K(0x3B,0x32);

    //Color saturation
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x80,0x1B);
    i2cWrite_PO2210K(0x81,0x05);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x1E);
    
    //Color saturation weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0D,0x40);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x0B,0x86); // user_cs_mode [3]

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x8A,0xF0);	// dc_y2

    //////////////////////////////////////////////////////////////// edge

    //Edge control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);
    i2cWrite_PO2210K(0x09,0x00);
    i2cWrite_PO2210K(0x0B,0x82);

    //sharpness control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x70,0x40);
    i2cWrite_PO2210K(0x71,0x40);

    //Edge gamma curve
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x48,0x50);
    i2cWrite_PO2210K(0x49,0x40);
    i2cWrite_PO2210K(0x4A,0x30);
    i2cWrite_PO2210K(0x4B,0x20);
    i2cWrite_PO2210K(0x4C,0x18);
    i2cWrite_PO2210K(0x4D,0x10);
    i2cWrite_PO2210K(0x4E,0x10);

    //edge_gain_lf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x53,0x18);
    i2cWrite_PO2210K(0x54,0x10);
    i2cWrite_PO2210K(0x55,0x08);

    //edge_gain_ghf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x57,0x10);
    i2cWrite_PO2210K(0x58,0x10);
    i2cWrite_PO2210K(0x59,0x10);

    //edge_gain_ehf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x5B,0x10);
    i2cWrite_PO2210K(0x5C,0x10);
    i2cWrite_PO2210K(0x5D,0x10);

    ////////////////////////////////////////////////////////////////////////// vec

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x93,0x0C); // vec_en (00)

    // Set sample range for Vector Control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x94,0x14);
    i2cWrite_PO2210K(0x95,0x1B);
    i2cWrite_PO2210K(0x98,0x1C);
    i2cWrite_PO2210K(0x99,0x26);
    i2cWrite_PO2210K(0x9C,0x35);
    i2cWrite_PO2210K(0x9D,0x42);
    i2cWrite_PO2210K(0xA0,0x44);
    i2cWrite_PO2210K(0xA1,0x62);
    i2cWrite_PO2210K(0xA4,0x64);
    i2cWrite_PO2210K(0xA5,0x72);
    i2cWrite_PO2210K(0xA8,0x76);
    i2cWrite_PO2210K(0xA9,0x80);

    // Set hue & saturation (a)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAC,0x90);
    i2cWrite_PO2210K(0xAF,0x04);
    i2cWrite_PO2210K(0xB2,0x07);
    i2cWrite_PO2210K(0xB5,0x04);
    i2cWrite_PO2210K(0xB8,0x82);
    i2cWrite_PO2210K(0xBB,0x04);
    i2cWrite_PO2210K(0xBE,0x92);
    i2cWrite_PO2210K(0xC1,0x04);
    i2cWrite_PO2210K(0xC4,0x85);
    i2cWrite_PO2210K(0xC7,0x04);
    i2cWrite_PO2210K(0xCA,0x86);
    i2cWrite_PO2210K(0xCD,0x04);

    // Set hue & saturation (b)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAD,0x88);
    i2cWrite_PO2210K(0xB0,0x04);
    i2cWrite_PO2210K(0xB3,0x0B);
    i2cWrite_PO2210K(0xB6,0x00);
    i2cWrite_PO2210K(0xB9,0x05);
    i2cWrite_PO2210K(0xBC,0x04);
    i2cWrite_PO2210K(0xBF,0x8B);
    i2cWrite_PO2210K(0xC2,0x04);
    i2cWrite_PO2210K(0xC5,0x85);
    i2cWrite_PO2210K(0xC8,0x04);
    i2cWrite_PO2210K(0xCB,0x84);
    i2cWrite_PO2210K(0xCE,0x04);

    // Set hue & saturation (c)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAE,0x88);
    i2cWrite_PO2210K(0xB1,0x04);
    i2cWrite_PO2210K(0xB4,0x82);
    i2cWrite_PO2210K(0xB7,0x04);
    i2cWrite_PO2210K(0xBA,0x01);
    i2cWrite_PO2210K(0xBD,0x04);
    i2cWrite_PO2210K(0xC0,0x00);
    i2cWrite_PO2210K(0xC3,0x06);
    i2cWrite_PO2210K(0xC6,0x07);
    i2cWrite_PO2210K(0xC9,0x04);
    i2cWrite_PO2210K(0xCC,0x00);
    i2cWrite_PO2210K(0xCF,0x04);

    //////////////////////////////////////////////////////////////////////// dark

    //Darkness X reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x98,0x00);
    i2cWrite_PO2210K(0x99,0x04);
    i2cWrite_PO2210K(0x9A,0x00);
    i2cWrite_PO2210K(0x9B,0x10);
    i2cWrite_PO2210K(0x9C,0x00);
    i2cWrite_PO2210K(0x9D,0x20);

    //dark_y_weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7A,0x40);
    i2cWrite_PO2210K(0x7B,0x40);
    i2cWrite_PO2210K(0x7C,0x80);

    //dark_ccr
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7E,0x04);
    i2cWrite_PO2210K(0x7F,0x04);
    i2cWrite_PO2210K(0x80,0x04);

    //dark_dc
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x0A);
    i2cWrite_PO2210K(0x84,0x24);

    //ycont_th2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE4,0x80);
    i2cWrite_PO2210K(0xE5,0x80);
    i2cWrite_PO2210K(0xE6,0x80);

    //ycont_slope2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE8,0x40);
    i2cWrite_PO2210K(0xE9,0x40);
    i2cWrite_PO2210K(0xEA,0x40);

    //dark_edge_gm
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x4F,0x00);
    i2cWrite_PO2210K(0x50,0x00);
    i2cWrite_PO2210K(0x51,0x00);

    //dark_ec_pth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x60,0x04);
    i2cWrite_PO2210K(0x61,0x04);
    i2cWrite_PO2210K(0x62,0x04);

    //dark_ec_mth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x04);
    i2cWrite_PO2210K(0x66,0x04);

    //dark_dpc_p
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1A,0x00);
    i2cWrite_PO2210K(0x1B,0x04);
    i2cWrite_PO2210K(0x1C,0x7F);

    //dark_dpc_n
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x08);
    i2cWrite_PO2210K(0x20,0x18);

    //ybrightness
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x95,0xF4);
    i2cWrite_PO2210K(0x96,0xF4);
    i2cWrite_PO2210K(0x97,0xF4);

    //blf_darkness
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x3F,0x38);
    i2cWrite_PO2210K(0x40,0x10);
    i2cWrite_PO2210K(0x41,0x00);

    //dark_ec_pmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x68,0x7F);
    i2cWrite_PO2210K(0x69,0x10);
    i2cWrite_PO2210K(0x6A,0x08);

    //dark_ec_mmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x6C,0x7F);
    i2cWrite_PO2210K(0x6D,0x20);
    i2cWrite_PO2210K(0x6E,0x08);

    //hf_dir_max
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x24,0x1C);
    i2cWrite_PO2210K(0x25,0x1C);
    i2cWrite_PO2210K(0x26,0x7F);

    //intp_dir_th
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x28,0x08);
    i2cWrite_PO2210K(0x29,0x08);
    i2cWrite_PO2210K(0x2A,0x7F);
//////////////////////////////////////
    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x68);
    i2cWrite_PO2210K(0x3C,0x60);
    i2cWrite_PO2210K(0x3D,0x58);
    i2cWrite_PO2210K(0x3E,0x60);
    i2cWrite_PO2210K(0x3F,0x60);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);
    
    //awb rg/bg ratio fitting
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x75,0x7A);
    i2cWrite_PO2210K(0x76,0x79);
    i2cWrite_PO2210K(0x77,0x7A);
    i2cWrite_PO2210K(0x78,0x79);
    //Front black control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x05,0x64);
    i2cWrite_PO2210K(0x06,0xA1);
    //Front black fitting
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0xB3,0x8A);
    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x08);

}
#else
void SetPO2210K_image_Quality(void)
{
    // 20160304 setting
    //////////////////////////////////////////////////////////////////////// ablc
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x38,0x90); // analog_control_02
    i2cWrite_PO2210K(0x3D,0x2F); // analog_control_07

    i2cWrite_PO2210K(0x03,0x01); // bank B
    i2cWrite_PO2210K(0x1F,0x51); // bayer_control_10
    i2cWrite_PO2210K(0x20,0xA8); // Median value for filter and Average value for selection
    i2cWrite_PO2210K(0xA3,0xE0); // blc_top_th
    i2cWrite_PO2210K(0xA4,0x70); // blc_bot_th
    i2cWrite_PO2210K(0xA5,0x02); // ablc_update

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x06,0xA1); // auto_control_3[0] : ablc fitting enable

    // fitting x reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xC7,0x00); // overOBP_xref0 
    i2cWrite_PO2210K(0xC8,0x08); // overOBP_xref1 
    i2cWrite_PO2210K(0xC9,0x1E); // overOBP_xref2 
    i2cWrite_PO2210K(0xCA,0x32); // overOBP_xref3 
    i2cWrite_PO2210K(0xCB,0x58); // overOBP_xref4 
       
    // fitting y reference
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xDC,0x00); // overOBP_yref0 
    i2cWrite_PO2210K(0xDD,0x16); // overOBP_yref1 
    i2cWrite_PO2210K(0xDE,0x1B); // overOBP_yref2 
    i2cWrite_PO2210K(0xE0,0x25); // overOBP_yref3 
    i2cWrite_PO2210K(0xE1,0x30); // overOBP_yref4 		

    //////////////////////////////////////////////////////////////////////// intp
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x30,0x00);	// intp_w0       (10)
    i2cWrite_PO2210K(0x31,0xFF);	// intp_x0       (00)
    i2cWrite_PO2210K(0x32,0x40);	// intp_slope    (40)

    //////////////////////////////////////////////////////////////////////// blf
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);	// [4] edge_blf_mode : 0=new, 1=old	FB

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x33,0x00);	// blf_w0_ref0	00
    i2cWrite_PO2210K(0x34,0x40);	// blf_w0_ref1	00
    i2cWrite_PO2210K(0x35,0x40);	// blf_w0_ref2	00

    i2cWrite_PO2210K(0x37,0x20);	// blf_x0	20
    i2cWrite_PO2210K(0x38,0x40);	// blf_slope	40

    i2cWrite_PO2210K(0x39,0x7F);	// blf_c0	80	7F
    i2cWrite_PO2210K(0x3A,0x78);	// blf_c1	60	78
    i2cWrite_PO2210K(0x3B,0x63);	// blf_c2	40	63
    i2cWrite_PO2210K(0x3C,0x3F);	// blf_c3	20	2F
    i2cWrite_PO2210K(0x3D,0x2B);	// blf_c4	10	0B
    i2cWrite_PO2210K(0x3E,0x18);	// blf_c5	08	00

    //////////////////////////////////////////////////////////////////////// sc
    i2cWrite_PO2210K(0x03,0x09);
    i2cWrite_PO2210K(0x04,0x03); //	acce_ctrl_0 [1]:acce enable, [0]:histogram enable (00)
    i2cWrite_PO2210K(0x6D,0x04); // ac_ctrl_0 [2]:AE relate mode

    i2cWrite_PO2210K(0x49,0x30);	// ce_th      (20)
    i2cWrite_PO2210K(0x4A,0x10);	// ce_x0      (40)
    i2cWrite_PO2210K(0x4B,0x40);	// ce_slope   (40)

    i2cWrite_PO2210K(0xAD,0x08); //08 // lpf_w1 (08)
    i2cWrite_PO2210K(0xAE,0x10); //10 // lpf_w2 (18)
    i2cWrite_PO2210K(0xAF,0x20); //20 // lpf_w3 (40)
    i2cWrite_PO2210K(0xB0,0x10); //10 // lpf_w4 (18)
    i2cWrite_PO2210K(0xB1,0x08); //08 // lpf_w5 (08)

    i2cWrite_PO2210K(0xB2,0x04); // ac_offset
    i2cWrite_PO2210K(0xB3,0x60); // max_ac_gain0
    i2cWrite_PO2210K(0xB4,0x60); // max_ac_gain1
    i2cWrite_PO2210K(0xB5,0x40); // max_ac_gain2

    i2cWrite_PO2210K(0xB7,0x40); // min_ac_gain
    i2cWrite_PO2210K(0xB8,0x03); // ac_speed
    i2cWrite_PO2210K(0xB9,0x02); // ac_lock
    i2cWrite_PO2210K(0xBB,0x04); // ac_frame

    i2cWrite_PO2210K(0x8E,0x00); // ac_cv_w0 
    i2cWrite_PO2210K(0x8F,0x04); // ac_cv_w1 
    i2cWrite_PO2210K(0x90,0x06); // ac_cv_w2 
    i2cWrite_PO2210K(0x91,0x06); // ac_cv_w3 
    i2cWrite_PO2210K(0x92,0x04); // ac_cv_w4 
    i2cWrite_PO2210K(0x93,0x03); // ac_cv_w5 
    i2cWrite_PO2210K(0x94,0x01); // ac_cv_w6 
    i2cWrite_PO2210K(0x95,0x00); // ac_cv_w7 
    
    //////////////////////////////////////////////////////////////////////// gm

    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x3D,0x00);
    i2cWrite_PO2210K(0x3E,0x0B);
    i2cWrite_PO2210K(0x3F,0x1e);
    i2cWrite_PO2210K(0x40,0x2a);
    i2cWrite_PO2210K(0x41,0x33);
    i2cWrite_PO2210K(0x42,0x41);
    i2cWrite_PO2210K(0x43,0x53);
    i2cWrite_PO2210K(0x44,0x73);
    i2cWrite_PO2210K(0x45,0x86);
    i2cWrite_PO2210K(0x46,0xA5);
    i2cWrite_PO2210K(0x47,0xBD);
    i2cWrite_PO2210K(0x48,0xD3);
    i2cWrite_PO2210K(0x49,0xE3);
    i2cWrite_PO2210K(0x4A,0xEF);
    i2cWrite_PO2210K(0x4B,0xFF);

    //gamma curve fitting
    i2cWrite_PO2210K(0x4C,0x00);
    i2cWrite_PO2210K(0x4D,0x11);
    i2cWrite_PO2210K(0x4E,0x1B);
    i2cWrite_PO2210K(0x4F,0x23);
    i2cWrite_PO2210K(0x50,0x2A);
    i2cWrite_PO2210K(0x51,0x37);
    i2cWrite_PO2210K(0x52,0x42);
    i2cWrite_PO2210K(0x53,0x56);
    i2cWrite_PO2210K(0x54,0x68);
    i2cWrite_PO2210K(0x55,0x87);
    i2cWrite_PO2210K(0x56,0xA3);
    i2cWrite_PO2210K(0x57,0xBC);
    i2cWrite_PO2210K(0x58,0xD4);
    i2cWrite_PO2210K(0x59,0xEA);
    i2cWrite_PO2210K(0x5A,0xFF);

    
    //gamma curve fitting
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x06);
    i2cWrite_PO2210K(0x5D,0x18);
    i2cWrite_PO2210K(0x5E,0x2c);
    i2cWrite_PO2210K(0x5F,0x3c);
    i2cWrite_PO2210K(0x60,0x54);
    i2cWrite_PO2210K(0x61,0x65);
    i2cWrite_PO2210K(0x62,0x7D);
    i2cWrite_PO2210K(0x63,0x8F);
    i2cWrite_PO2210K(0x64,0xAB);
    i2cWrite_PO2210K(0x65,0xC1);
    i2cWrite_PO2210K(0x66,0xD3);
    i2cWrite_PO2210K(0x67,0xE3);
    i2cWrite_PO2210K(0x68,0xF2);
    i2cWrite_PO2210K(0x69,0xFF);

    //////////////////////////////////////////////////////////////////////// awb

    // Set AWB Sampling Boundary
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x40);
    i2cWrite_PO2210K(0x5E,0xA0);
    i2cWrite_PO2210K(0x5F,0x01);
    i2cWrite_PO2210K(0x60,0x02);
    i2cWrite_PO2210K(0x61,0x50);
    i2cWrite_PO2210K(0x62,0x02);
    i2cWrite_PO2210K(0x63,0x00);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x6E);
    i2cWrite_PO2210K(0x66,0x45);

    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x67,0x27);
    i2cWrite_PO2210K(0x68,0x4F);
    i2cWrite_PO2210K(0x69,0x64);
    i2cWrite_PO2210K(0x6A,0xC4);
    i2cWrite_PO2210K(0x6B,0x0A);
    i2cWrite_PO2210K(0x6C,0x46);
    i2cWrite_PO2210K(0x6D,0x32);
    i2cWrite_PO2210K(0x6E,0x78);
    i2cWrite_PO2210K(0x6F,0x37);
    i2cWrite_PO2210K(0x70,0xAF);
    i2cWrite_PO2210K(0x71,0x32);
    i2cWrite_PO2210K(0x72,0x23);

    //Ai2cWrite_PO2210K(0xB option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x7E,0x08);
    i2cWrite_PO2210K(0x7F,0x04);

    //lens / cs axis
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0A,0x3E);
    i2cWrite_PO2210K(0x0B,0x5D);
    i2cWrite_PO2210K(0x0C,0x6C);

    ////////////////////////////////////////////////////////////////////// color

    //Color correction
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x33,0x2F);
    i2cWrite_PO2210K(0x34,0x85);
    i2cWrite_PO2210K(0x35,0x8A);
    i2cWrite_PO2210K(0x36,0x90);
    i2cWrite_PO2210K(0x37,0x3E);
    i2cWrite_PO2210K(0x38,0x8E);
    i2cWrite_PO2210K(0x39,0x91);
    i2cWrite_PO2210K(0x3A,0x81);
    i2cWrite_PO2210K(0x3B,0x32);

    //Color saturation
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x80,0x25);
    i2cWrite_PO2210K(0x81,0x05);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x1E);
    
    //Color saturation weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x0D,0x30);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x0B,0x86); // user_cs_mode [3]

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x8A,0xF0);	// dc_y2

    //////////////////////////////////////////////////////////////// edge

    //Edge control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x05,0xFB);
    i2cWrite_PO2210K(0x09,0x00);
    i2cWrite_PO2210K(0x0B,0x82);

    //sharpness control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x70,0x40);
    i2cWrite_PO2210K(0x71,0x40);

    //Edge gamma curve
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x48,0x50);
    i2cWrite_PO2210K(0x49,0x40);
    i2cWrite_PO2210K(0x4A,0x30);
    i2cWrite_PO2210K(0x4B,0x20);
    i2cWrite_PO2210K(0x4C,0x18);
    i2cWrite_PO2210K(0x4D,0x10);
    i2cWrite_PO2210K(0x4E,0x10);

    //edge_gain_lf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x53,0x18);
    i2cWrite_PO2210K(0x54,0x10);
    i2cWrite_PO2210K(0x55,0x08);

    //edge_gain_ghf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x57,0x10);
    i2cWrite_PO2210K(0x58,0x10);
    i2cWrite_PO2210K(0x59,0x10);

    //edge_gain_ehf
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x5B,0x10);
    i2cWrite_PO2210K(0x5C,0x10);
    i2cWrite_PO2210K(0x5D,0x10);

    ////////////////////////////////////////////////////////////////////////// vec

    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x93,0x0C); // vec_en (00)

    // Set sample range for Vector Control
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x94,0x14);
    i2cWrite_PO2210K(0x95,0x1B);
    i2cWrite_PO2210K(0x98,0x1C);
    i2cWrite_PO2210K(0x99,0x26);
    i2cWrite_PO2210K(0x9C,0x35);
    i2cWrite_PO2210K(0x9D,0x42);
    i2cWrite_PO2210K(0xA0,0x44);
    i2cWrite_PO2210K(0xA1,0x62);
    i2cWrite_PO2210K(0xA4,0x64);
    i2cWrite_PO2210K(0xA5,0x72);
    i2cWrite_PO2210K(0xA8,0x76);
    i2cWrite_PO2210K(0xA9,0x80);

    // Set hue & saturation (a)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAC,0x90);
    i2cWrite_PO2210K(0xAF,0x04);
    i2cWrite_PO2210K(0xB2,0x07);
    i2cWrite_PO2210K(0xB5,0x04);
    i2cWrite_PO2210K(0xB8,0x82);
    i2cWrite_PO2210K(0xBB,0x04);
    i2cWrite_PO2210K(0xBE,0x92);
    i2cWrite_PO2210K(0xC1,0x04);
    i2cWrite_PO2210K(0xC4,0x85);
    i2cWrite_PO2210K(0xC7,0x04);
    i2cWrite_PO2210K(0xCA,0x86);
    i2cWrite_PO2210K(0xCD,0x04);

    // Set hue & saturation (b)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAD,0x88);
    i2cWrite_PO2210K(0xB0,0x04);
    i2cWrite_PO2210K(0xB3,0x0B);
    i2cWrite_PO2210K(0xB6,0x00);
    i2cWrite_PO2210K(0xB9,0x05);
    i2cWrite_PO2210K(0xBC,0x04);
    i2cWrite_PO2210K(0xBF,0x8B);
    i2cWrite_PO2210K(0xC2,0x04);
    i2cWrite_PO2210K(0xC5,0x85);
    i2cWrite_PO2210K(0xC8,0x04);
    i2cWrite_PO2210K(0xCB,0x84);
    i2cWrite_PO2210K(0xCE,0x04);

    // Set hue & saturation (c)
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xAE,0x88);
    i2cWrite_PO2210K(0xB1,0x04);
    i2cWrite_PO2210K(0xB4,0x82);
    i2cWrite_PO2210K(0xB7,0x04);
    i2cWrite_PO2210K(0xBA,0x01);
    i2cWrite_PO2210K(0xBD,0x04);
    i2cWrite_PO2210K(0xC0,0x00);
    i2cWrite_PO2210K(0xC3,0x06);
    i2cWrite_PO2210K(0xC6,0x07);
    i2cWrite_PO2210K(0xC9,0x04);
    i2cWrite_PO2210K(0xCC,0x00);
    i2cWrite_PO2210K(0xCF,0x04);

    //////////////////////////////////////////////////////////////////////// dark

    //Darkness X reference
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x98,0x00);
    i2cWrite_PO2210K(0x99,0x04);
    i2cWrite_PO2210K(0x9A,0x00);
    i2cWrite_PO2210K(0x9B,0x10);
    i2cWrite_PO2210K(0x9C,0x00);
    i2cWrite_PO2210K(0x9D,0x20);

    //dark_y_weight
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7A,0x40);
    i2cWrite_PO2210K(0x7B,0x40);
    i2cWrite_PO2210K(0x7C,0x80);

    //dark_ccr
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x7E,0x04);
    i2cWrite_PO2210K(0x7F,0x04);
    i2cWrite_PO2210K(0x80,0x04);

    //dark_dc
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x82,0x00);
    i2cWrite_PO2210K(0x83,0x0A);
    i2cWrite_PO2210K(0x84,0x24);

    //ycont_th2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE4,0x80);
    i2cWrite_PO2210K(0xE5,0x80);
    i2cWrite_PO2210K(0xE6,0x80);

    //ycont_slope2
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0xE8,0x40);
    i2cWrite_PO2210K(0xE9,0x40);
    i2cWrite_PO2210K(0xEA,0x40);

    //dark_edge_gm
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x4F,0x00);
    i2cWrite_PO2210K(0x50,0x00);
    i2cWrite_PO2210K(0x51,0x00);

    //dark_ec_pth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x60,0x04);
    i2cWrite_PO2210K(0x61,0x04);
    i2cWrite_PO2210K(0x62,0x04);

    //dark_ec_mth
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x64,0x04);
    i2cWrite_PO2210K(0x65,0x04);
    i2cWrite_PO2210K(0x66,0x04);

    //dark_dpc_p
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1A,0x00);
    i2cWrite_PO2210K(0x1B,0x04);
    i2cWrite_PO2210K(0x1C,0x7F);

    //dark_dpc_n
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x08);
    i2cWrite_PO2210K(0x20,0x18);

    //ybrightness
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x95,0xE0);
    i2cWrite_PO2210K(0x96,0xE0);
    i2cWrite_PO2210K(0x97,0xE0);

    //blf_darkness
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x3F,0x38);
    i2cWrite_PO2210K(0x40,0x10);
    i2cWrite_PO2210K(0x41,0x00);

    //dark_ec_pmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x68,0x40);
    i2cWrite_PO2210K(0x69,0x10);
    i2cWrite_PO2210K(0x6A,0x08);

    //dark_ec_mmax
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x6C,0x40);
    i2cWrite_PO2210K(0x6D,0x20);
    i2cWrite_PO2210K(0x6E,0x08);

    //hf_dir_max
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x24,0x1C);
    i2cWrite_PO2210K(0x25,0x1C);
    i2cWrite_PO2210K(0x26,0x7F);

    //intp_dir_th
    i2cWrite_PO2210K(0x03,0x03);
    i2cWrite_PO2210K(0x28,0x08);
    i2cWrite_PO2210K(0x29,0x08);
    i2cWrite_PO2210K(0x2A,0x7F);
       
}
#endif

void SetPO2210K_1080P_10FPS(void)
{
    int i;
    u32 waitFlag;
    u8  err;

#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: SetPO2210K_1080P_10FPS SiuFlagGrp is 0x%x.\n", err);
		return ;
	}
#endif

#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) 

#else
    DEBUG_SIU("SetPO2210K_1080P_10FPS()\n");
#endif
    //gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset not correct
    //for(i=0;i<0x0ff;i++);
    //gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    //for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1); //reset pull hi, then 8ms ~ 140ms SDA

    //////////////////////////// Base Settings ////////////////////////////////

    #if (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) //Lucian: 為加速AE收斂,開機時給予上次關機前的AE expo value.
 
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x08,0x01);

        if (SIUMODE == SIU_DAY_MODE)
        {
            i2cWrite_PO2210K(0x27,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dt]);
            i2cWrite_PO2210K(0x28,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh]);
            i2cWrite_PO2210K(0x29,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm]);
            i2cWrite_PO2210K(0x2a,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dl]);
        }
        else
        {
            i2cWrite_PO2210K(0x27,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nt]);
            i2cWrite_PO2210K(0x28,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nh]);
            i2cWrite_PO2210K(0x29,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nm]);
            i2cWrite_PO2210K(0x2a,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nl]);
        }

        //maunal AWB is useless. AWB calculate again(3 frames) when set auto AWB. 
        //manual AE applying also need 3 frames. MPEG4 throw 3 dark frames, avoid VMD difference huge. refer TX_PIRREC_VMDCHK function
    #endif
    
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x3C,0xC4); // Internal DVDD OFF, bgrcon_15 = 000b (1.35V)

    i2cWrite_PO2210K(0x04,0x02);	// chip mode (00 : BT1120, 01 : sampling, 02 : ccir601)

    i2cWrite_PO2210K(0x06,0x08); // framewidth_h
    i2cWrite_PO2210K(0x07,0x97); // framewidth_l

    i2cWrite_PO2210K(0x24,0x00); // clkdiv1 (08)
    i2cWrite_PO2210K(0x25,0x00); // clkdiv2

    i2cWrite_PO2210K(0x2F,0x01); // pad_control7        (01)
    i2cWrite_PO2210K(0x2A,0x43); // pad_control2        (00)
    i2cWrite_PO2210K(0x2B,0x9C); // pad_control3        (00)
    i2cWrite_PO2210K(0x2E,0x03); // pad_control6        (00)
    i2cWrite_PO2210K(0x30,0xFF); // pad_control8        (00)
    i2cWrite_PO2210K(0x31,0x00); // pad_control9        (00)

    // 27MHz to 49.5MHz 10FPS
    i2cWrite_PO2210K(0x40,0x0B); // pll_m_cnt
    i2cWrite_PO2210K(0x41,0x06); // pll_r_cnt

    i2cWrite_PO2210K(0x3F,0x50); // pll_control1
    for(i=0;i<0x0ff;i++);
    i2cWrite_PO2210K(0x3F,0x40); // pll_control1

    ////////////////////////// Start Settings ////////////////////////////////
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x16,0x04); // led_dsel
    i2cWrite_PO2210K(0xB7,0x40); // adcoffset

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x2B,0x14); // dpc_offset
    i2cWrite_PO2210K(0x9B,0x02);

    //////////////////////////////////////////////////////////////////////// blacksun

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x1E,0x0E); // bsmode off
    i2cWrite_PO2210K(0x26,0x04); // blacksunth_h

    //////////////////////////////////////////////////////////////////////// recommended by design1 150813
    i2cWrite_PO2210K(0x03,0x01);   // Limiter reference fitting due to gain
    i2cWrite_PO2210K(0xF6,0x0E);   // bs_ofst0
    i2cWrite_PO2210K(0xF7,0x14);   // bs_ofst1
    i2cWrite_PO2210K(0xF8,0x24);   // bs_ofst2
    i2cWrite_PO2210K(0xF9,0x26);   // bs_ofst3
    i2cWrite_PO2210K(0xFA,0x26);   // bs_ofst4
    i2cWrite_PO2210K(0xFB,0x26);   // bs_ofst5
    i2cWrite_PO2210K(0xFC,0x26);   // bs_ofst6
    i2cWrite_PO2210K(0xFD,0x26);   // bs_ofst_max
    i2cWrite_PO2210K(0xFE,0x00);   // bs_ofst_min

    //////////////////////////////////////////////////////////////////////// cds v1.1
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x35,0x08); // pixelbias (01)
    i2cWrite_PO2210K(0x36,0x04); // compbias (02)

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x19,0xC4); // ramppclk_sel
    i2cWrite_PO2210K(0x1C,0x11); // ramp speed X1, adc speed X1

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x57,0x08);
    i2cWrite_PO2210K(0x58,0x7F);
    i2cWrite_PO2210K(0x59,0x08);
    i2cWrite_PO2210K(0x5A,0x96);
    i2cWrite_PO2210K(0x53,0x00);
    i2cWrite_PO2210K(0x54,0x02);
    i2cWrite_PO2210K(0x55,0x08);
    i2cWrite_PO2210K(0x56,0x7F);
    i2cWrite_PO2210K(0x67,0x00);
    i2cWrite_PO2210K(0x68,0x54);
    i2cWrite_PO2210K(0x69,0x00);
    i2cWrite_PO2210K(0x6A,0x5E);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x08);
    i2cWrite_PO2210K(0x5E,0x7F);
    i2cWrite_PO2210K(0x5F,0x00);
    i2cWrite_PO2210K(0x60,0x00);
    i2cWrite_PO2210K(0x61,0x00);
    i2cWrite_PO2210K(0x62,0x50);
    i2cWrite_PO2210K(0x99,0x00);
    i2cWrite_PO2210K(0x9A,0x54);
    i2cWrite_PO2210K(0x9B,0x08);
    i2cWrite_PO2210K(0x9C,0x7F);
    i2cWrite_PO2210K(0x6F,0x00);
    i2cWrite_PO2210K(0x70,0x00);
    i2cWrite_PO2210K(0x71,0x05);
    i2cWrite_PO2210K(0x72,0x7A);
    i2cWrite_PO2210K(0x73,0x00);
    i2cWrite_PO2210K(0x74,0x00);
    i2cWrite_PO2210K(0x75,0x05);
    i2cWrite_PO2210K(0x76,0x78);
    i2cWrite_PO2210K(0x77,0x08);
    i2cWrite_PO2210K(0x78,0x95);
    i2cWrite_PO2210K(0x79,0x08);
    i2cWrite_PO2210K(0x7A,0x96);
    //WB740
    i2cWrite_PO2210K(0x8F,0x00);
    i2cWrite_PO2210K(0x90,0x52);
    i2cWrite_PO2210K(0x8B,0x00);
    i2cWrite_PO2210K(0x8C,0x64);
    i2cWrite_PO2210K(0x8D,0x08);
    i2cWrite_PO2210K(0x8E,0x6A);
    i2cWrite_PO2210K(0x87,0x08);
    i2cWrite_PO2210K(0x88,0x48);
    i2cWrite_PO2210K(0x89,0x08);
    i2cWrite_PO2210K(0x8A,0x7C);
    i2cWrite_PO2210K(0x95,0x08);
    i2cWrite_PO2210K(0x96,0x80);
    i2cWrite_PO2210K(0x97,0x08);
    i2cWrite_PO2210K(0x98,0x8F);
    i2cWrite_PO2210K(0x91,0x08);
    i2cWrite_PO2210K(0x92,0x80);
    i2cWrite_PO2210K(0x93,0x08);
    i2cWrite_PO2210K(0x94,0x97);
    i2cWrite_PO2210K(0x7F,0x08);
    i2cWrite_PO2210K(0x80,0x80);
    i2cWrite_PO2210K(0x81,0x08);
    i2cWrite_PO2210K(0x82,0x8F);
    i2cWrite_PO2210K(0x83,0x08);
    i2cWrite_PO2210K(0x84,0x80);
    i2cWrite_PO2210K(0x85,0x08);
    i2cWrite_PO2210K(0x86,0x8F);
    i2cWrite_PO2210K(0xB9,0x08);
    i2cWrite_PO2210K(0xBA,0x80);
    i2cWrite_PO2210K(0xBB,0x08);
    i2cWrite_PO2210K(0xBC,0x8F);
    i2cWrite_PO2210K(0xA1,0x0B);
    i2cWrite_PO2210K(0xA2,0x84);
    i2cWrite_PO2210K(0x36,0x00);
    i2cWrite_PO2210K(0x37,0xBE);
    i2cWrite_PO2210K(0x38,0x08);
    i2cWrite_PO2210K(0x39,0x4E);
    i2cWrite_PO2210K(0x7B,0x00);
    i2cWrite_PO2210K(0x7C,0x00);
    i2cWrite_PO2210K(0x7D,0x05);
    i2cWrite_PO2210K(0x7E,0x7C);
    i2cWrite_PO2210K(0x3E,0x00);
    i2cWrite_PO2210K(0x3F,0xBE);
    i2cWrite_PO2210K(0x40,0x08);
    i2cWrite_PO2210K(0x41,0x4E);


    //////////////////////////////////////////////////////////////////////// ae

    //Flicker canceling mode - manual 60hz
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x4A,0x08);

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x54,0x00);
    i2cWrite_PO2210K(0x55,0x5D);
    i2cWrite_PO2210K(0x56,0x2A);
    i2cWrite_PO2210K(0x57,0x00);
    i2cWrite_PO2210K(0x58,0x6F);
    i2cWrite_PO2210K(0x59,0xCC);

    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x50);
    i2cWrite_PO2210K(0x3C,0x50);
    i2cWrite_PO2210K(0x3D,0x48);
    i2cWrite_PO2210K(0x3E,0x50);
    i2cWrite_PO2210K(0x3F,0x48);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);

    //User Y target weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x54,0x90);

    //Auto exposure control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x5E);
    i2cWrite_PO2210K(0x14,0x04);
    i2cWrite_PO2210K(0x15,0x5E);
    i2cWrite_PO2210K(0x16,0x04);
    i2cWrite_PO2210K(0x17,0x5E);
    i2cWrite_PO2210K(0x1B,0x00);
    i2cWrite_PO2210K(0x1C,0x8B);
    i2cWrite_PO2210K(0x1D,0xC0);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x8B);
    i2cWrite_PO2210K(0x20,0xC0);

    //Reference Gain Control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xBA,0x10);

    //saturation level th
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x2C,0x66);

    //Auto exposure option
    i2cWrite_PO2210K(0x03,0x04);
 #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)
    i2cWrite_PO2210K(0x55,0x0c);
    i2cWrite_PO2210K(0x56,0x0c);
 #else
    i2cWrite_PO2210K(0x55,0x04);
    i2cWrite_PO2210K(0x56,0x04);
 #endif   
    i2cWrite_PO2210K(0x57,0x0C);

    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x20);

    //Image format control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x29,0x00);
    i2cWrite_PO2210K(0x88,0x7F);
    i2cWrite_PO2210K(0x8E,0xFE);
    i2cWrite_PO2210K(0x9A,0x80);
    
    //Image size control
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x0C,0x00);
    i2cWrite_PO2210K(0x0D,0x05);
    i2cWrite_PO2210K(0x0E,0x00);
    i2cWrite_PO2210K(0x0F,0x05);
    i2cWrite_PO2210K(0x10,0x07);
    i2cWrite_PO2210K(0x11,0x84);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x3C);
    i2cWrite_PO2210K(0x15,0x16);
    i2cWrite_PO2210K(0x17,0x51);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x7B,0x20);
    i2cWrite_PO2210K(0x7C,0x20);
    i2cWrite_PO2210K(0x7D,0x00);
    i2cWrite_PO2210K(0x7E,0x0A);

#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif
    SetPO2210K_image_Quality();

}

void SetPO2210K_1080P_12FPS(void)
{
    int i;

    DEBUG_SIU("SetPO2210K_1080P_12FPS() \n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1); //reset pull hi, then 8ms ~ 140ms SDA
    //////////////////////////// Base Settings ////////////////////////////////

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x3C,0xC4); // Internal DVDD OFF, bgrcon_15 = 000b (1.35V)

    i2cWrite_PO2210K(0x04,0x02);	// chip mode (00 : BT1120, 01 : sampling, 02 : ccir601)

    i2cWrite_PO2210K(0x06,0x08); // framewidth_h
    i2cWrite_PO2210K(0x07,0x97); // framewidth_l

    i2cWrite_PO2210K(0x24,0x00); // clkdiv1 (08)
    i2cWrite_PO2210K(0x25,0x00); // clkdiv2

    i2cWrite_PO2210K(0x2F,0x01); // pad_control7        (01)
    i2cWrite_PO2210K(0x2A,0x43); // pad_control2        (00)
    i2cWrite_PO2210K(0x2B,0x9C); // pad_control3        (00)
    i2cWrite_PO2210K(0x2E,0x03); // pad_control6        (00)
    i2cWrite_PO2210K(0x30,0xFF); // pad_control8        (00)
    i2cWrite_PO2210K(0x31,0x00); // pad_control9        (00)

    // 27MHz to 59.4MHz 10FPS
    i2cWrite_PO2210K(0x40,0x0B); // pll_m_cnt
    i2cWrite_PO2210K(0x41,0x05); // pll_r_cnt

    i2cWrite_PO2210K(0x3F,0x50); // pll_control1
    for(i=0;i<0x0ff;i++);
    i2cWrite_PO2210K(0x3F,0x40); // pll_control1

    ////////////////////////// Start Settings ////////////////////////////////
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x16,0x04); // led_dsel
    i2cWrite_PO2210K(0xB7,0x40); // adcoffset

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x2B,0x14); // dpc_offset
    i2cWrite_PO2210K(0x9B,0x02);

    //////////////////////////////////////////////////////////////////////// blacksun

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x1E,0x0E); // bsmode off
    i2cWrite_PO2210K(0x26,0x04); // blacksunth_h

    //////////////////////////////////////////////////////////////////////// recommended by design1 150813
    i2cWrite_PO2210K(0x03,0x01);   // Limiter reference fitting due to gain
    i2cWrite_PO2210K(0xF6,0x0E);   // bs_ofst0
    i2cWrite_PO2210K(0xF7,0x14);   // bs_ofst1
    i2cWrite_PO2210K(0xF8,0x24);   // bs_ofst2
    i2cWrite_PO2210K(0xF9,0x26);   // bs_ofst3
    i2cWrite_PO2210K(0xFA,0x26);   // bs_ofst4
    i2cWrite_PO2210K(0xFB,0x26);   // bs_ofst5
    i2cWrite_PO2210K(0xFC,0x26);   // bs_ofst6
    i2cWrite_PO2210K(0xFD,0x26);   // bs_ofst_max
    i2cWrite_PO2210K(0xFE,0x00);   // bs_ofst_min

    //////////////////////////////////////////////////////////////////////// cds v1.1
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x35,0x08); // pixelbias (01)
    i2cWrite_PO2210K(0x36,0x04); // compbias (02)

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x19,0xC4); // ramppclk_sel
    i2cWrite_PO2210K(0x1C,0x11); // ramp speed X1, adc speed X1

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x57,0x08);
    i2cWrite_PO2210K(0x58,0x7F);
    i2cWrite_PO2210K(0x59,0x08);
    i2cWrite_PO2210K(0x5A,0x96);
    i2cWrite_PO2210K(0x53,0x00);
    i2cWrite_PO2210K(0x54,0x02);
    i2cWrite_PO2210K(0x55,0x08);
    i2cWrite_PO2210K(0x56,0x7F);
    i2cWrite_PO2210K(0x67,0x00);
    i2cWrite_PO2210K(0x68,0x54);
    i2cWrite_PO2210K(0x69,0x00);
    i2cWrite_PO2210K(0x6A,0x5E);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x08);
    i2cWrite_PO2210K(0x5E,0x7F);
    i2cWrite_PO2210K(0x5F,0x00);
    i2cWrite_PO2210K(0x60,0x00);
    i2cWrite_PO2210K(0x61,0x00);
    i2cWrite_PO2210K(0x62,0x50);
    i2cWrite_PO2210K(0x99,0x00);
    i2cWrite_PO2210K(0x9A,0x54);
    i2cWrite_PO2210K(0x9B,0x08);
    i2cWrite_PO2210K(0x9C,0x7F);
    i2cWrite_PO2210K(0x6F,0x00);
    i2cWrite_PO2210K(0x70,0x00);
    i2cWrite_PO2210K(0x71,0x05);
    i2cWrite_PO2210K(0x72,0x7A);
    i2cWrite_PO2210K(0x73,0x00);
    i2cWrite_PO2210K(0x74,0x00);
    i2cWrite_PO2210K(0x75,0x05);
    i2cWrite_PO2210K(0x76,0x78);
    i2cWrite_PO2210K(0x77,0x08);
    i2cWrite_PO2210K(0x78,0x95);
    i2cWrite_PO2210K(0x79,0x08);
    i2cWrite_PO2210K(0x7A,0x96);
    //WB740
    i2cWrite_PO2210K(0x8F,0x00);
    i2cWrite_PO2210K(0x90,0x52);
    i2cWrite_PO2210K(0x8B,0x00);
    i2cWrite_PO2210K(0x8C,0x64);
    i2cWrite_PO2210K(0x8D,0x08);
    i2cWrite_PO2210K(0x8E,0x6A);
    i2cWrite_PO2210K(0x87,0x08);
    i2cWrite_PO2210K(0x88,0x48);
    i2cWrite_PO2210K(0x89,0x08);
    i2cWrite_PO2210K(0x8A,0x7C);
    i2cWrite_PO2210K(0x95,0x08);
    i2cWrite_PO2210K(0x96,0x80);
    i2cWrite_PO2210K(0x97,0x08);
    i2cWrite_PO2210K(0x98,0x8F);
    i2cWrite_PO2210K(0x91,0x08);
    i2cWrite_PO2210K(0x92,0x80);
    i2cWrite_PO2210K(0x93,0x08);
    i2cWrite_PO2210K(0x94,0x97);
    i2cWrite_PO2210K(0x7F,0x08);
    i2cWrite_PO2210K(0x80,0x80);
    i2cWrite_PO2210K(0x81,0x08);
    i2cWrite_PO2210K(0x82,0x8F);
    i2cWrite_PO2210K(0x83,0x08);
    i2cWrite_PO2210K(0x84,0x80);
    i2cWrite_PO2210K(0x85,0x08);
    i2cWrite_PO2210K(0x86,0x8F);
    i2cWrite_PO2210K(0xB9,0x08);
    i2cWrite_PO2210K(0xBA,0x80);
    i2cWrite_PO2210K(0xBB,0x08);
    i2cWrite_PO2210K(0xBC,0x8F);
    i2cWrite_PO2210K(0xA1,0x0B);
    i2cWrite_PO2210K(0xA2,0x84);
    i2cWrite_PO2210K(0x36,0x00);
    i2cWrite_PO2210K(0x37,0xBE);
    i2cWrite_PO2210K(0x38,0x08);
    i2cWrite_PO2210K(0x39,0x4E);
    i2cWrite_PO2210K(0x7B,0x00);
    i2cWrite_PO2210K(0x7C,0x00);
    i2cWrite_PO2210K(0x7D,0x05);
    i2cWrite_PO2210K(0x7E,0x7C);
    i2cWrite_PO2210K(0x3E,0x00);
    i2cWrite_PO2210K(0x3F,0xBE);
    i2cWrite_PO2210K(0x40,0x08);
    i2cWrite_PO2210K(0x41,0x4E);


    //////////////////////////////////////////////////////////////////////// ae

    //Flicker canceling mode - manual 60hz
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x4A,0x08);

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x54,0x00);
    i2cWrite_PO2210K(0x55,0x6F);
    i2cWrite_PO2210K(0x56,0xCC);
    
    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x50);
    i2cWrite_PO2210K(0x3C,0x50);
    i2cWrite_PO2210K(0x3D,0x48);
    i2cWrite_PO2210K(0x3E,0x50);
    i2cWrite_PO2210K(0x3F,0x48);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);

    //Auto exposure control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x5E);
    i2cWrite_PO2210K(0x14,0x04);
    i2cWrite_PO2210K(0x15,0x5E);
    i2cWrite_PO2210K(0x16,0x04);
    i2cWrite_PO2210K(0x17,0x5E);
    i2cWrite_PO2210K(0x1B,0x00);
    i2cWrite_PO2210K(0x1C,0x8B);
    i2cWrite_PO2210K(0x1D,0xC0);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x8B);
    i2cWrite_PO2210K(0x20,0xC0);

    //Reference Gain Control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xBA,0x10);

    //saturation level th
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x2C,0x66);

    //Auto exposure option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x55,0x04);
    i2cWrite_PO2210K(0x56,0x04);
    i2cWrite_PO2210K(0x57,0x0C);

    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x20);

    //Image format control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x29,0x00);
    i2cWrite_PO2210K(0x88,0x7F);
    i2cWrite_PO2210K(0x8E,0xFE);
    i2cWrite_PO2210K(0x9A,0x80);

    //Image size control
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x0C,0x00);
    i2cWrite_PO2210K(0x0D,0x05);
    i2cWrite_PO2210K(0x0E,0x00);
    i2cWrite_PO2210K(0x0F,0x05);
    i2cWrite_PO2210K(0x10,0x07);
    i2cWrite_PO2210K(0x11,0x84);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x3C);
    i2cWrite_PO2210K(0x15,0x16);
    i2cWrite_PO2210K(0x17,0x51);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x7B,0x20);
    i2cWrite_PO2210K(0x7C,0x20);
    i2cWrite_PO2210K(0x7D,0x00);
    i2cWrite_PO2210K(0x7E,0x0A);

    SetPO2210K_image_Quality();

}

void SetPO2210K_720P_15FPS(void)
{
    int i;

    DEBUG_SIU("SetPO2210K_720P_15FPS() \n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1); //reset pull hi, then 8ms ~ 140ms SDA

    //////////////////////////// Base Settings ////////////////////////////////

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x3C,0xC4); // Internal DVDD OFF, bgrcon_15 = 000b (1.35V)

    i2cWrite_PO2210K(0x04,0x02);	// chip mode (00 : BT1120, 01 : sampling, 02 : ccir601)

    i2cWrite_PO2210K(0x06,0x08); // framewidth_h
    i2cWrite_PO2210K(0x07,0x97); // framewidth_l

    i2cWrite_PO2210K(0x24,0x00); // clkdiv1 (08)
    i2cWrite_PO2210K(0x25,0x00); // clkdiv2

    i2cWrite_PO2210K(0x2F,0x01); // pad_control7        (01)
    i2cWrite_PO2210K(0x2A,0x43); // pad_control2        (00)
    i2cWrite_PO2210K(0x2B,0x9C); // pad_control3        (00)
    i2cWrite_PO2210K(0x2E,0x03); // pad_control6        (00)
    i2cWrite_PO2210K(0x30,0xFF); // pad_control8        (00)
    i2cWrite_PO2210K(0x31,0x00); // pad_control9        (00)

    i2cWrite_PO2210K(0x40,0x0B); // pll_m_cnt
    i2cWrite_PO2210K(0x41,0x04); // pll_r_cnt

    i2cWrite_PO2210K(0x3F,0x50); // pll_control1
    for(i=0;i<0x0ff;i++);
    i2cWrite_PO2210K(0x3F,0x40); // pll_control1

    ////////////////////////// Start Settings ////////////////////////////////
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x16,0x04); // led_dsel
    i2cWrite_PO2210K(0xB7,0x40); // adcoffset

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x2B,0x14); // dpc_offset
    i2cWrite_PO2210K(0x9B,0x02);

    //////////////////////////////////////////////////////////////////////// blacksun

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x1E,0x0E); // bsmode off
    i2cWrite_PO2210K(0x26,0x04); // blacksunth_h

    //////////////////////////////////////////////////////////////////////// recommended by design1 150813
    i2cWrite_PO2210K(0x03,0x01);   // Limiter reference fitting due to gain
    i2cWrite_PO2210K(0xF6,0x0E);   // bs_ofst0
    i2cWrite_PO2210K(0xF7,0x14);   // bs_ofst1
    i2cWrite_PO2210K(0xF8,0x24);   // bs_ofst2
    i2cWrite_PO2210K(0xF9,0x26);   // bs_ofst3
    i2cWrite_PO2210K(0xFA,0x26);   // bs_ofst4
    i2cWrite_PO2210K(0xFB,0x26);   // bs_ofst5
    i2cWrite_PO2210K(0xFC,0x26);   // bs_ofst6
    i2cWrite_PO2210K(0xFD,0x26);   // bs_ofst_max
    i2cWrite_PO2210K(0xFE,0x00);   // bs_ofst_min

    //////////////////////////////////////////////////////////////////////// cds v1.1
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x35,0x08); // pixelbias (01)
    i2cWrite_PO2210K(0x36,0x04); // compbias (02)

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x19,0xC4); // ramppclk_sel
    i2cWrite_PO2210K(0x1C,0x11); // ramp speed X1, adc speed X1

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x57,0x08);
    i2cWrite_PO2210K(0x58,0x7F);
    i2cWrite_PO2210K(0x59,0x08);
    i2cWrite_PO2210K(0x5A,0x96);
    i2cWrite_PO2210K(0x53,0x00);
    i2cWrite_PO2210K(0x54,0x02);
    i2cWrite_PO2210K(0x55,0x08);
    i2cWrite_PO2210K(0x56,0x7F);
    i2cWrite_PO2210K(0x67,0x00);
    i2cWrite_PO2210K(0x68,0x54);
    i2cWrite_PO2210K(0x69,0x00);
    i2cWrite_PO2210K(0x6A,0x5E);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x08);
    i2cWrite_PO2210K(0x5E,0x7F);
    i2cWrite_PO2210K(0x5F,0x00);
    i2cWrite_PO2210K(0x60,0x00);
    i2cWrite_PO2210K(0x61,0x00);
    i2cWrite_PO2210K(0x62,0x50);
    i2cWrite_PO2210K(0x99,0x00);
    i2cWrite_PO2210K(0x9A,0x54);
    i2cWrite_PO2210K(0x9B,0x08);
    i2cWrite_PO2210K(0x9C,0x7F);
    i2cWrite_PO2210K(0x6F,0x00);
    i2cWrite_PO2210K(0x70,0x00);
    i2cWrite_PO2210K(0x71,0x05);
    i2cWrite_PO2210K(0x72,0x7A);
    i2cWrite_PO2210K(0x73,0x00);
    i2cWrite_PO2210K(0x74,0x00);
    i2cWrite_PO2210K(0x75,0x05);
    i2cWrite_PO2210K(0x76,0x78);
    i2cWrite_PO2210K(0x77,0x08);
    i2cWrite_PO2210K(0x78,0x95);
    i2cWrite_PO2210K(0x79,0x08);
    i2cWrite_PO2210K(0x7A,0x96);
    //WB740
    i2cWrite_PO2210K(0x8F,0x00);
    i2cWrite_PO2210K(0x90,0x52);
    i2cWrite_PO2210K(0x8B,0x00);
    i2cWrite_PO2210K(0x8C,0x64);
    i2cWrite_PO2210K(0x8D,0x08);
    i2cWrite_PO2210K(0x8E,0x6A);
    i2cWrite_PO2210K(0x87,0x08);
    i2cWrite_PO2210K(0x88,0x48);
    i2cWrite_PO2210K(0x89,0x08);
    i2cWrite_PO2210K(0x8A,0x7C);
    i2cWrite_PO2210K(0x95,0x08);
    i2cWrite_PO2210K(0x96,0x80);
    i2cWrite_PO2210K(0x97,0x08);
    i2cWrite_PO2210K(0x98,0x8F);
    i2cWrite_PO2210K(0x91,0x08);
    i2cWrite_PO2210K(0x92,0x80);
    i2cWrite_PO2210K(0x93,0x08);
    i2cWrite_PO2210K(0x94,0x97);
    i2cWrite_PO2210K(0x7F,0x08);
    i2cWrite_PO2210K(0x80,0x80);
    i2cWrite_PO2210K(0x81,0x08);
    i2cWrite_PO2210K(0x82,0x8F);
    i2cWrite_PO2210K(0x83,0x08);
    i2cWrite_PO2210K(0x84,0x80);
    i2cWrite_PO2210K(0x85,0x08);
    i2cWrite_PO2210K(0x86,0x8F);
    i2cWrite_PO2210K(0xB9,0x08);
    i2cWrite_PO2210K(0xBA,0x80);
    i2cWrite_PO2210K(0xBB,0x08);
    i2cWrite_PO2210K(0xBC,0x8F);
    i2cWrite_PO2210K(0xA1,0x0B);
    i2cWrite_PO2210K(0xA2,0x84);
    i2cWrite_PO2210K(0x36,0x00);
    i2cWrite_PO2210K(0x37,0xBE);
    i2cWrite_PO2210K(0x38,0x08);
    i2cWrite_PO2210K(0x39,0x4E);
    i2cWrite_PO2210K(0x7B,0x00);
    i2cWrite_PO2210K(0x7C,0x00);
    i2cWrite_PO2210K(0x7D,0x05);
    i2cWrite_PO2210K(0x7E,0x7C);
    i2cWrite_PO2210K(0x3E,0x00);
    i2cWrite_PO2210K(0x3F,0xBE);
    i2cWrite_PO2210K(0x40,0x08);
    i2cWrite_PO2210K(0x41,0x4E);


    //////////////////////////////////////////////////////////////////////// ae

    //Flicker canceling mode - manual 60hz
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x4A,0x08);

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x54,0x00);
    i2cWrite_PO2210K(0x55,0x8B);
    i2cWrite_PO2210K(0x56,0xC0);
    i2cWrite_PO2210K(0x57,0x00);
    i2cWrite_PO2210K(0x58,0xA8);
    i2cWrite_PO2210K(0x59,0xC0);

    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x50);
    i2cWrite_PO2210K(0x3C,0x50);
    i2cWrite_PO2210K(0x3D,0x48);
    i2cWrite_PO2210K(0x3E,0x50);
    i2cWrite_PO2210K(0x3F,0x48);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);

    //Auto exposure control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x5E);
    i2cWrite_PO2210K(0x14,0x04);
    i2cWrite_PO2210K(0x15,0x5E);
    i2cWrite_PO2210K(0x16,0x04);
    i2cWrite_PO2210K(0x17,0x5E);
    i2cWrite_PO2210K(0x1B,0x00);
    i2cWrite_PO2210K(0x1C,0x8B);
    i2cWrite_PO2210K(0x1D,0xC0);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x8B);
    i2cWrite_PO2210K(0x20,0xC0);

    //Reference Gain Control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xBA,0x10);

    //saturation level th
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x2C,0x66);

    //Auto exposure option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x55,0x04);
    i2cWrite_PO2210K(0x56,0x04);
    i2cWrite_PO2210K(0x57,0x0C);

    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x20);

    //Image format control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x29,0x00);
    i2cWrite_PO2210K(0x88,0x7F);
    i2cWrite_PO2210K(0x8E,0xFE);
    i2cWrite_PO2210K(0x9A,0x80);

    //Image size control
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x0C,0x00);
    i2cWrite_PO2210K(0x0D,0x03);
    i2cWrite_PO2210K(0x0E,0x00);
    i2cWrite_PO2210K(0x0F,0x03);
    i2cWrite_PO2210K(0x10,0x05);
    i2cWrite_PO2210K(0x11,0x02);
    i2cWrite_PO2210K(0x12,0x02);
    i2cWrite_PO2210K(0x13,0xD2);
    i2cWrite_PO2210K(0x15,0x16);
    i2cWrite_PO2210K(0x17,0x51);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x7B,0x30);
    i2cWrite_PO2210K(0x7C,0x30);
    i2cWrite_PO2210K(0x7D,0x03);
    i2cWrite_PO2210K(0x7E,0x5C);

    SetPO2210K_image_Quality();

}

void SetPO2210K_720P_20FPS(void)
{
    int i;
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) 

#else
    DEBUG_SIU("SetPO2210K_720P_20FPS() \n");
#endif
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1);

    //////////////////////////// Base Settings ////////////////////////////////

    #if (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) //Lucian: 為加速AE收斂,開機時給予上次關機前的AE expo value.
 
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x08,0x01);

        if (SIUMODE == SIU_DAY_MODE)
        {
            i2cWrite_PO2210K(0x27,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dt]);
            i2cWrite_PO2210K(0x28,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh]);
            i2cWrite_PO2210K(0x29,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm]);
            i2cWrite_PO2210K(0x2a,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dl]);
        }
        else
        {
            i2cWrite_PO2210K(0x27,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nt]);
            i2cWrite_PO2210K(0x28,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nh]);
            i2cWrite_PO2210K(0x29,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nm]);
            i2cWrite_PO2210K(0x2a,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nl]);
        }

        //maunal AWB is useless. AWB calculate again(3 bad frames) when set auto AWB. MPEG4 throw 3 frames, record from 4th frame
    #endif

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x3C,0xC4); // Internal DVDD OFF, bgrcon_15 = 000b (1.35V)

    i2cWrite_PO2210K(0x04,0x02);	// chip mode (00 : BT1120, 01 : sampling, 02 : ccir601)

    i2cWrite_PO2210K(0x06,0x08); // framewidth_h
    i2cWrite_PO2210K(0x07,0x97); // framewidth_l

    i2cWrite_PO2210K(0x24,0x00); // clkdiv1 (08)
    i2cWrite_PO2210K(0x25,0x00); // clkdiv2

    i2cWrite_PO2210K(0x2F,0x01); // pad_control7        (01)
    i2cWrite_PO2210K(0x2A,0x43); // pad_control2        (00)
    i2cWrite_PO2210K(0x2B,0x9C); // pad_control3        (00)
    i2cWrite_PO2210K(0x2E,0x03); // pad_control6        (00)
    i2cWrite_PO2210K(0x30,0xFF); // pad_control8        (00)
    i2cWrite_PO2210K(0x31,0x00); // pad_control9        (00)

    // 27MHz to 99MHz
    i2cWrite_PO2210K(0x40,0x0B); // pll_m_cnt
    i2cWrite_PO2210K(0x41,0x03); // pll_r_cnt

    i2cWrite_PO2210K(0x3F,0x50); // pll_control1
    for(i=0;i<0x0ff;i++);
    i2cWrite_PO2210K(0x3F,0x40); // pll_control1

    ////////////////////////// Start Settings ////////////////////////////////
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x16,0x04); // led_dsel
    i2cWrite_PO2210K(0xB7,0x40); // adcoffset

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x2B,0x14); // dpc_offset
    i2cWrite_PO2210K(0x9B,0x02);

    //////////////////////////////////////////////////////////////////////// blacksun

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x1E,0x0E); // bsmode off
    i2cWrite_PO2210K(0x26,0x04); // blacksunth_h

    //////////////////////////////////////////////////////////////////////// recommended by design1 150813
    i2cWrite_PO2210K(0x03,0x01);   // Limiter reference fitting due to gain
    i2cWrite_PO2210K(0xF6,0x0E);   // bs_ofst0
    i2cWrite_PO2210K(0xF7,0x14);   // bs_ofst1
    i2cWrite_PO2210K(0xF8,0x24);   // bs_ofst2
    i2cWrite_PO2210K(0xF9,0x26);   // bs_ofst3
    i2cWrite_PO2210K(0xFA,0x26);   // bs_ofst4
    i2cWrite_PO2210K(0xFB,0x26);   // bs_ofst5
    i2cWrite_PO2210K(0xFC,0x26);   // bs_ofst6
    i2cWrite_PO2210K(0xFD,0x26);   // bs_ofst_max
    i2cWrite_PO2210K(0xFE,0x00);   // bs_ofst_min

    //////////////////////////////////////////////////////////////////////// cds v1.1
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x35,0x08); // pixelbias (01)
    i2cWrite_PO2210K(0x36,0x04); // compbias (02)

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x19,0xC4); // ramppclk_sel
    i2cWrite_PO2210K(0x1C,0x11); // ramp speed X1, adc speed X1

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x57,0x08);
    i2cWrite_PO2210K(0x58,0x7F);
    i2cWrite_PO2210K(0x59,0x08);
    i2cWrite_PO2210K(0x5A,0x96);
    i2cWrite_PO2210K(0x53,0x00);
    i2cWrite_PO2210K(0x54,0x02);
    i2cWrite_PO2210K(0x55,0x08);
    i2cWrite_PO2210K(0x56,0x7F);
    i2cWrite_PO2210K(0x67,0x00);
    i2cWrite_PO2210K(0x68,0x54);
    i2cWrite_PO2210K(0x69,0x00);
    i2cWrite_PO2210K(0x6A,0x5E);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x08);
    i2cWrite_PO2210K(0x5E,0x7F);
    i2cWrite_PO2210K(0x5F,0x00);
    i2cWrite_PO2210K(0x60,0x00);
    i2cWrite_PO2210K(0x61,0x00);
    i2cWrite_PO2210K(0x62,0x50);
    i2cWrite_PO2210K(0x99,0x00);
    i2cWrite_PO2210K(0x9A,0x54);
    i2cWrite_PO2210K(0x9B,0x08);
    i2cWrite_PO2210K(0x9C,0x7F);
    i2cWrite_PO2210K(0x6F,0x00);
    i2cWrite_PO2210K(0x70,0x00);
    i2cWrite_PO2210K(0x71,0x05);
    i2cWrite_PO2210K(0x72,0x7A);
    i2cWrite_PO2210K(0x73,0x00);
    i2cWrite_PO2210K(0x74,0x00);
    i2cWrite_PO2210K(0x75,0x05);
    i2cWrite_PO2210K(0x76,0x78);
    i2cWrite_PO2210K(0x77,0x08);
    i2cWrite_PO2210K(0x78,0x95);
    i2cWrite_PO2210K(0x79,0x08);
    i2cWrite_PO2210K(0x7A,0x96);
    //WB740
    i2cWrite_PO2210K(0x8F,0x00);
    i2cWrite_PO2210K(0x90,0x52);
    i2cWrite_PO2210K(0x8B,0x00);
    i2cWrite_PO2210K(0x8C,0x64);
    i2cWrite_PO2210K(0x8D,0x08);
    i2cWrite_PO2210K(0x8E,0x6A);
    i2cWrite_PO2210K(0x87,0x08);
    i2cWrite_PO2210K(0x88,0x48);
    i2cWrite_PO2210K(0x89,0x08);
    i2cWrite_PO2210K(0x8A,0x7C);
    i2cWrite_PO2210K(0x95,0x08);
    i2cWrite_PO2210K(0x96,0x80);
    i2cWrite_PO2210K(0x97,0x08);
    i2cWrite_PO2210K(0x98,0x8F);
    i2cWrite_PO2210K(0x91,0x08);
    i2cWrite_PO2210K(0x92,0x80);
    i2cWrite_PO2210K(0x93,0x08);
    i2cWrite_PO2210K(0x94,0x97);
    i2cWrite_PO2210K(0x7F,0x08);
    i2cWrite_PO2210K(0x80,0x80);
    i2cWrite_PO2210K(0x81,0x08);
    i2cWrite_PO2210K(0x82,0x8F);
    i2cWrite_PO2210K(0x83,0x08);
    i2cWrite_PO2210K(0x84,0x80);
    i2cWrite_PO2210K(0x85,0x08);
    i2cWrite_PO2210K(0x86,0x8F);
    i2cWrite_PO2210K(0xB9,0x08);
    i2cWrite_PO2210K(0xBA,0x80);
    i2cWrite_PO2210K(0xBB,0x08);
    i2cWrite_PO2210K(0xBC,0x8F);
    i2cWrite_PO2210K(0xA1,0x0B);
    i2cWrite_PO2210K(0xA2,0x84);
    i2cWrite_PO2210K(0x36,0x00);
    i2cWrite_PO2210K(0x37,0xBE);
    i2cWrite_PO2210K(0x38,0x08);
    i2cWrite_PO2210K(0x39,0x4E);
    i2cWrite_PO2210K(0x7B,0x00);
    i2cWrite_PO2210K(0x7C,0x00);
    i2cWrite_PO2210K(0x7D,0x05);
    i2cWrite_PO2210K(0x7E,0x7C);
    i2cWrite_PO2210K(0x3E,0x00);
    i2cWrite_PO2210K(0x3F,0xBE);
    i2cWrite_PO2210K(0x40,0x08);
    i2cWrite_PO2210K(0x41,0x4E);


    //////////////////////////////////////////////////////////////////////// ae

    //Flicker canceling mode - manual 60hz
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x4A,0x08);

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x54,0x00);
    i2cWrite_PO2210K(0x55,0xBA);
    i2cWrite_PO2210K(0x56,0x55);
    i2cWrite_PO2210K(0x57,0x00);
    i2cWrite_PO2210K(0x58,0xE1);
    i2cWrite_PO2210K(0x59,0x00);

    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x50);
    i2cWrite_PO2210K(0x3C,0x50);
    i2cWrite_PO2210K(0x3D,0x48);
    i2cWrite_PO2210K(0x3E,0x50);
    i2cWrite_PO2210K(0x3F,0x48);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);

    //Auto exposure control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x5E);
    i2cWrite_PO2210K(0x14,0x04);
    i2cWrite_PO2210K(0x15,0x5E);
    i2cWrite_PO2210K(0x16,0x04);
    i2cWrite_PO2210K(0x17,0x5E);
    i2cWrite_PO2210K(0x1B,0x00);
    i2cWrite_PO2210K(0x1C,0x8B);
    i2cWrite_PO2210K(0x1D,0xC0);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x8B);
    i2cWrite_PO2210K(0x20,0xC0);

    //Reference Gain Control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xBA,0x10);

    //saturation level th
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x2C,0x66);

    //Auto exposure option
    i2cWrite_PO2210K(0x03,0x04);
 #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)
    i2cWrite_PO2210K(0x55,0x0c);
    i2cWrite_PO2210K(0x56,0x0c);
 #else    
    i2cWrite_PO2210K(0x55,0x04);
    i2cWrite_PO2210K(0x56,0x04);
 #endif   
    i2cWrite_PO2210K(0x57,0x0C);

    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x20);

    //Image format control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x29,0x00);
    i2cWrite_PO2210K(0x88,0x7F);
    i2cWrite_PO2210K(0x8E,0xFE);
    i2cWrite_PO2210K(0x9A,0x80);

    //Image size control
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x0C,0x00);
    i2cWrite_PO2210K(0x0D,0x03);
    i2cWrite_PO2210K(0x0E,0x00);
    i2cWrite_PO2210K(0x0F,0x03);
    i2cWrite_PO2210K(0x10,0x05);
    i2cWrite_PO2210K(0x11,0x02);
    i2cWrite_PO2210K(0x12,0x02);
    i2cWrite_PO2210K(0x13,0xD2);
    i2cWrite_PO2210K(0x15,0x16);
    i2cWrite_PO2210K(0x17,0x51);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x7B,0x30);
    i2cWrite_PO2210K(0x7C,0x30);
    i2cWrite_PO2210K(0x7D,0x03);
    i2cWrite_PO2210K(0x7E,0x5C);

    SetPO2210K_image_Quality();

}
void SetPO2210K_720P_21FPS(void)
{
    int i;

    DEBUG_SIU("SetPO2210K_720P_21FPS() \n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1); //reset pull hi, then 8ms ~ 140ms SDA

    //////////////////////////// Base Settings ////////////////////////////////

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x3C,0xC4); // Internal DVDD OFF, bgrcon_15 = 000b (1.35V)

    i2cWrite_PO2210K(0x04,0x02);	// chip mode (00 : BT1120, 01 : sampling, 02 : ccir601)

    i2cWrite_PO2210K(0x06,0x08); // framewidth_h
    i2cWrite_PO2210K(0x07,0x97); // framewidth_l

    i2cWrite_PO2210K(0x24,0x00); // clkdiv1 (08)
    i2cWrite_PO2210K(0x25,0x00); // clkdiv2

    i2cWrite_PO2210K(0x2F,0x01); // pad_control7        (01)
    i2cWrite_PO2210K(0x2A,0x43); // pad_control2        (00)
    i2cWrite_PO2210K(0x2B,0x9C); // pad_control3        (00)
    i2cWrite_PO2210K(0x2E,0x03); // pad_control6        (00)
    i2cWrite_PO2210K(0x30,0xFF); // pad_control8        (00)
    i2cWrite_PO2210K(0x31,0x00); // pad_control9        (00)

    // 27MHz to 108MHz
    i2cWrite_PO2210K(0x40,0x04); // pll_m_cnt
    i2cWrite_PO2210K(0x41,0x01); // pll_r_cnt

    i2cWrite_PO2210K(0x3F,0x50); // pll_control1
    for(i=0;i<0x0ff;i++);
    i2cWrite_PO2210K(0x3F,0x40); // pll_control1

    ////////////////////////// Start Settings ////////////////////////////////
    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x16,0x04); // led_dsel
    i2cWrite_PO2210K(0xB7,0x40); // adcoffset

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x2B,0x14); // dpc_offset
    i2cWrite_PO2210K(0x9B,0x02);

    //////////////////////////////////////////////////////////////////////// blacksun

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x1E,0x0E); // bsmode off
    i2cWrite_PO2210K(0x26,0x04); // blacksunth_h

    //////////////////////////////////////////////////////////////////////// recommended by design1 150813
    i2cWrite_PO2210K(0x03,0x01);   // Limiter reference fitting due to gain
    i2cWrite_PO2210K(0xF6,0x0E);   // bs_ofst0
    i2cWrite_PO2210K(0xF7,0x14);   // bs_ofst1
    i2cWrite_PO2210K(0xF8,0x24);   // bs_ofst2
    i2cWrite_PO2210K(0xF9,0x26);   // bs_ofst3
    i2cWrite_PO2210K(0xFA,0x26);   // bs_ofst4
    i2cWrite_PO2210K(0xFB,0x26);   // bs_ofst5
    i2cWrite_PO2210K(0xFC,0x26);   // bs_ofst6
    i2cWrite_PO2210K(0xFD,0x26);   // bs_ofst_max
    i2cWrite_PO2210K(0xFE,0x00);   // bs_ofst_min

    //////////////////////////////////////////////////////////////////////// cds v1.1
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x35,0x08); // pixelbias (01)
    i2cWrite_PO2210K(0x36,0x04); // compbias (02)

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x19,0xC4); // ramppclk_sel
    i2cWrite_PO2210K(0x1C,0x11); // ramp speed X1, adc speed X1

    i2cWrite_PO2210K(0x03,0x01);
    i2cWrite_PO2210K(0x57,0x08);
    i2cWrite_PO2210K(0x58,0x7F);
    i2cWrite_PO2210K(0x59,0x08);
    i2cWrite_PO2210K(0x5A,0x96);
    i2cWrite_PO2210K(0x53,0x00);
    i2cWrite_PO2210K(0x54,0x02);
    i2cWrite_PO2210K(0x55,0x08);
    i2cWrite_PO2210K(0x56,0x7F);
    i2cWrite_PO2210K(0x67,0x00);
    i2cWrite_PO2210K(0x68,0x54);
    i2cWrite_PO2210K(0x69,0x00);
    i2cWrite_PO2210K(0x6A,0x5E);
    i2cWrite_PO2210K(0x5B,0x00);
    i2cWrite_PO2210K(0x5C,0x00);
    i2cWrite_PO2210K(0x5D,0x08);
    i2cWrite_PO2210K(0x5E,0x7F);
    i2cWrite_PO2210K(0x5F,0x00);
    i2cWrite_PO2210K(0x60,0x00);
    i2cWrite_PO2210K(0x61,0x00);
    i2cWrite_PO2210K(0x62,0x50);
    i2cWrite_PO2210K(0x99,0x00);
    i2cWrite_PO2210K(0x9A,0x54);
    i2cWrite_PO2210K(0x9B,0x08);
    i2cWrite_PO2210K(0x9C,0x7F);
    i2cWrite_PO2210K(0x6F,0x00);
    i2cWrite_PO2210K(0x70,0x00);
    i2cWrite_PO2210K(0x71,0x05);
    i2cWrite_PO2210K(0x72,0x7A);
    i2cWrite_PO2210K(0x73,0x00);
    i2cWrite_PO2210K(0x74,0x00);
    i2cWrite_PO2210K(0x75,0x05);
    i2cWrite_PO2210K(0x76,0x78);
    i2cWrite_PO2210K(0x77,0x08);
    i2cWrite_PO2210K(0x78,0x95);
    i2cWrite_PO2210K(0x79,0x08);
    i2cWrite_PO2210K(0x7A,0x96);
    //WB740
    i2cWrite_PO2210K(0x8F,0x00);
    i2cWrite_PO2210K(0x90,0x52);
    i2cWrite_PO2210K(0x8B,0x00);
    i2cWrite_PO2210K(0x8C,0x64);
    i2cWrite_PO2210K(0x8D,0x08);
    i2cWrite_PO2210K(0x8E,0x6A);
    i2cWrite_PO2210K(0x87,0x08);
    i2cWrite_PO2210K(0x88,0x48);
    i2cWrite_PO2210K(0x89,0x08);
    i2cWrite_PO2210K(0x8A,0x7C);
    i2cWrite_PO2210K(0x95,0x08);
    i2cWrite_PO2210K(0x96,0x80);
    i2cWrite_PO2210K(0x97,0x08);
    i2cWrite_PO2210K(0x98,0x8F);
    i2cWrite_PO2210K(0x91,0x08);
    i2cWrite_PO2210K(0x92,0x80);
    i2cWrite_PO2210K(0x93,0x08);
    i2cWrite_PO2210K(0x94,0x97);
    i2cWrite_PO2210K(0x7F,0x08);
    i2cWrite_PO2210K(0x80,0x80);
    i2cWrite_PO2210K(0x81,0x08);
    i2cWrite_PO2210K(0x82,0x8F);
    i2cWrite_PO2210K(0x83,0x08);
    i2cWrite_PO2210K(0x84,0x80);
    i2cWrite_PO2210K(0x85,0x08);
    i2cWrite_PO2210K(0x86,0x8F);
    i2cWrite_PO2210K(0xB9,0x08);
    i2cWrite_PO2210K(0xBA,0x80);
    i2cWrite_PO2210K(0xBB,0x08);
    i2cWrite_PO2210K(0xBC,0x8F);
    i2cWrite_PO2210K(0xA1,0x0B);
    i2cWrite_PO2210K(0xA2,0x84);
    i2cWrite_PO2210K(0x36,0x00);
    i2cWrite_PO2210K(0x37,0xBE);
    i2cWrite_PO2210K(0x38,0x08);
    i2cWrite_PO2210K(0x39,0x4E);
    i2cWrite_PO2210K(0x7B,0x00);
    i2cWrite_PO2210K(0x7C,0x00);
    i2cWrite_PO2210K(0x7D,0x05);
    i2cWrite_PO2210K(0x7E,0x7C);
    i2cWrite_PO2210K(0x3E,0x00);
    i2cWrite_PO2210K(0x3F,0xBE);
    i2cWrite_PO2210K(0x40,0x08);
    i2cWrite_PO2210K(0x41,0x4E);


    //////////////////////////////////////////////////////////////////////// ae

    //Flicker canceling mode - manual 60hz
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x4A,0x08);

    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x54,0x00);
    i2cWrite_PO2210K(0x55,0xCB);
    i2cWrite_PO2210K(0x56,0x46);

    //Y target control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x3B,0x50);
    i2cWrite_PO2210K(0x3C,0x50);
    i2cWrite_PO2210K(0x3D,0x48);
    i2cWrite_PO2210K(0x3E,0x50);
    i2cWrite_PO2210K(0x3F,0x48);
    i2cWrite_PO2210K(0x40,0x48);
    i2cWrite_PO2210K(0x41,0x00);
    i2cWrite_PO2210K(0x42,0x00);
    i2cWrite_PO2210K(0x43,0x14);
    i2cWrite_PO2210K(0x44,0x00);
    i2cWrite_PO2210K(0x45,0x02);
    i2cWrite_PO2210K(0x46,0xE8);
    i2cWrite_PO2210K(0x47,0x00);
    i2cWrite_PO2210K(0x48,0x45);
    i2cWrite_PO2210K(0x49,0xE0);
    i2cWrite_PO2210K(0x4A,0x00);
    i2cWrite_PO2210K(0x4B,0x8B);
    i2cWrite_PO2210K(0x4C,0xC0);

    //Auto exposure control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x12,0x04);
    i2cWrite_PO2210K(0x13,0x5E);
    i2cWrite_PO2210K(0x14,0x04);
    i2cWrite_PO2210K(0x15,0x5E);
    i2cWrite_PO2210K(0x16,0x04);
    i2cWrite_PO2210K(0x17,0x5E);
    i2cWrite_PO2210K(0x1B,0x00);
    i2cWrite_PO2210K(0x1C,0x8B);
    i2cWrite_PO2210K(0x1D,0xC0);
    i2cWrite_PO2210K(0x1E,0x00);
    i2cWrite_PO2210K(0x1F,0x8B);
    i2cWrite_PO2210K(0x20,0xC0);

    //Reference Gain Control
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0xBA,0x10);

    //saturation level th
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x2C,0x66);

    //Auto exposure option
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x55,0x04);
    i2cWrite_PO2210K(0x56,0x04);
    i2cWrite_PO2210K(0x57,0x0C);

    //Center window weight
    i2cWrite_PO2210K(0x03,0x04);
    i2cWrite_PO2210K(0x34,0x20);

    //Image format control
    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x29,0x00);
    i2cWrite_PO2210K(0x88,0x7F);
    i2cWrite_PO2210K(0x8E,0xFE);
    i2cWrite_PO2210K(0x9A,0x80);

    //Image size control
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x0C,0x00);
    i2cWrite_PO2210K(0x0D,0x03);
    i2cWrite_PO2210K(0x0E,0x00);
    i2cWrite_PO2210K(0x0F,0x03);
    i2cWrite_PO2210K(0x10,0x05);
    i2cWrite_PO2210K(0x11,0x02);
    i2cWrite_PO2210K(0x12,0x02);
    i2cWrite_PO2210K(0x13,0xD2);
    i2cWrite_PO2210K(0x15,0x16);
    i2cWrite_PO2210K(0x17,0x51);

    i2cWrite_PO2210K(0x03,0x02);
    i2cWrite_PO2210K(0x7B,0x30);
    i2cWrite_PO2210K(0x7C,0x30);
    i2cWrite_PO2210K(0x7D,0x03);
    i2cWrite_PO2210K(0x7E,0x5C);
    
    SetPO2210K_image_Quality();

}
void SetPO2210K_VGA_30FPS(void)
{
    DEBUG_SIU("Error no support SetPO2210K_VGA_30FPS()\n");
}


void SetPO2210K_720P_Change_to_27FPS(void)
{
    DEBUG_SIU("Error no support SetPO2210K_720P_Change_to_27FPS()\n");
}

void SetPO2210K_720P_Change_to_30FPS(void)
{
    DEBUG_SIU("Error no support support SetPO2210K_720P_Change_to_30FPS()\n");
}

void siuSetStandbyMode(u8 onoff) //Standby or Normal mode
{
    u8 data = 0x66 | (onoff << 7);
    i2cWrite_PO2210K(0x03,0x00);
    i2cWrite_PO2210K(0x29,data);
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
#if PO2210K_YUV601_FLAGTEST
    SiuFlagGrp=OSFlagCreate(SIU_EVET_SENSOR_SET, &err);
    if(err != OS_NO_ERR)
        DEBUG_SIU("SiuFlagGrp OSFlagCreate error %d\n", err);
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 

            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896) )
            {
                if (PO2210K_1080P_FrameRate == 12)
                {
                    FrameRate   = 12;
                    SetPO2210K_1080P_12FPS();
                }
                else
                {
                    FrameRate   = 10;
                    SetPO2210K_1080P_10FPS();
                }
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                if (PO2210K_720P_FrameRate == 21)
                {               
                    FrameRate   = 21;
                    SetPO2210K_720P_21FPS();
                }
                else if (PO2210K_720P_FrameRate == 20)
                {
                    FrameRate   = 20;
                    SetPO2210K_720P_20FPS();
                }
                else
                {
                    FrameRate   = 15;
                    SetPO2210K_720P_15FPS();
                }
            }
			else
		    {
                FrameRate   = 15;
                SetPO2210K_720P_15FPS();
			}

            siuSetImage();
            #if( (HW_BOARD_OPTION == MR9120_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI) || (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION  == MR9100_TX_RDI_USB) )
            siuSetSensorDayNight(SIUMODE);
            #elif ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) || (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T) || (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM))
            siuSetSensorDayNight(SIUMODE);
            #endif

            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) 
            i2cWrite_PO2210K(0x03,0x04);
            i2cWrite_PO2210K(0x08,0x00); //set AE auto
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


#if ( (HW_BOARD_OPTION == MR9120_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI) || (HW_BOARD_OPTION == A1019A_SKB_128M_TX))
void siuSetSensorDayNight(u8 Level)
{
    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x3D,0x00);
        i2cWrite_PO2210K(0x3E,0x27);
        i2cWrite_PO2210K(0x3F,0x36);
        i2cWrite_PO2210K(0x40,0x40);
        i2cWrite_PO2210K(0x41,0x49);
        i2cWrite_PO2210K(0x42,0x58);
        i2cWrite_PO2210K(0x43,0x64);
        i2cWrite_PO2210K(0x44,0x78);
        i2cWrite_PO2210K(0x45,0x89);
        i2cWrite_PO2210K(0x46,0xA4);
        i2cWrite_PO2210K(0x47,0xBB);
        i2cWrite_PO2210K(0x48,0xCF);
        i2cWrite_PO2210K(0x49,0xE0);
        i2cWrite_PO2210K(0x4A,0xE1);
        i2cWrite_PO2210K(0x4B,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x4C,0x00);
        i2cWrite_PO2210K(0x4D,0x2B);
        i2cWrite_PO2210K(0x4E,0x37);
        i2cWrite_PO2210K(0x4F,0x43);
        i2cWrite_PO2210K(0x50,0x4F);
        i2cWrite_PO2210K(0x51,0x5C);
        i2cWrite_PO2210K(0x52,0x65);
        i2cWrite_PO2210K(0x53,0x71);
        i2cWrite_PO2210K(0x54,0x7A);
        i2cWrite_PO2210K(0x55,0x8E);
        i2cWrite_PO2210K(0x56,0xA3);
        i2cWrite_PO2210K(0x57,0xBC);
        i2cWrite_PO2210K(0x58,0xD4);
        i2cWrite_PO2210K(0x59,0xEA);
        i2cWrite_PO2210K(0x5A,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x5B,0x00);
        i2cWrite_PO2210K(0x5C,0x01);
        i2cWrite_PO2210K(0x5D,0x07);
        i2cWrite_PO2210K(0x5E,0x10);
        i2cWrite_PO2210K(0x5F,0x1C);
        i2cWrite_PO2210K(0x60,0x35);
        i2cWrite_PO2210K(0x61,0x4C);
        i2cWrite_PO2210K(0x62,0x70);
        i2cWrite_PO2210K(0x63,0x89);
        i2cWrite_PO2210K(0x64,0xAB);
        i2cWrite_PO2210K(0x65,0xC3);
        i2cWrite_PO2210K(0x66,0xD5);
        i2cWrite_PO2210K(0x67,0xE5);
        i2cWrite_PO2210K(0x68,0xF3);
        i2cWrite_PO2210K(0x69,0xFF);

        //Darkness X reference
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x98,0x00);
        i2cWrite_PO2210K(0x99,0x24);
        i2cWrite_PO2210K(0x9A,0x00);
        i2cWrite_PO2210K(0x9B,0x24);
        i2cWrite_PO2210K(0x9C,0x01);
        i2cWrite_PO2210K(0x9D,0x11);

        //ycont_th2
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE4,0x00);
        i2cWrite_PO2210K(0xE5,0x00);
        i2cWrite_PO2210K(0xE6,0x00);

        //BRIGHTNESS Default:0x14
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95,0x14);
        i2cWrite_PO2210K(0x96,0x14);
        i2cWrite_PO2210K(0x97,0x14);
        //CONTRAST Range:0x00-0xff Default:0x40
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE8,0x40);
        i2cWrite_PO2210K(0xE9,0x40);
        i2cWrite_PO2210K(0xEA,0x40);

        //SATURATION Range:0x00-0xff Default:0x00
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
        //SHARPNESS Range:0x00-0xff Default:0x10
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x5B,0x10);
        i2cWrite_PO2210K(0x5C,0x10);
        i2cWrite_PO2210K(0x5D,0x10);
		
        //Auto exposure control
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        i2cWrite_PO2210K(0x1B,0x00);
        #if (HW_BOARD_OPTION == MR9120_TX_OPCOM_CVI)
        i2cWrite_PO2210K(0x1C,0x57); //x20
        i2cWrite_PO2210K(0x1D,0x50);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x57);
        i2cWrite_PO2210K(0x20,0x50);
        #else
        i2cWrite_PO2210K(0x1C,0x45);
        i2cWrite_PO2210K(0x1D,0xE0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x45);
        i2cWrite_PO2210K(0x20,0xE0);
        #endif

		i2cWrite_PO2210K(0x03,0x00);
		i2cWrite_PO2210K(0x4A,0x00);
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x3D,0x00);
        i2cWrite_PO2210K(0x3E,0x0B);
        i2cWrite_PO2210K(0x3F,0x1E);
        i2cWrite_PO2210K(0x40,0x2A);
        i2cWrite_PO2210K(0x41,0x33);
        i2cWrite_PO2210K(0x42,0x41);
        i2cWrite_PO2210K(0x43,0x53);
        i2cWrite_PO2210K(0x44,0x73);
        i2cWrite_PO2210K(0x45,0x86);
        i2cWrite_PO2210K(0x46,0xA5);
        i2cWrite_PO2210K(0x47,0xBD);
        i2cWrite_PO2210K(0x48,0xD3);
        i2cWrite_PO2210K(0x49,0xE3);
        i2cWrite_PO2210K(0x4A,0xEF);
        i2cWrite_PO2210K(0x4B,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x4C,0x00);
        i2cWrite_PO2210K(0x4D,0x11);
        i2cWrite_PO2210K(0x4E,0x1B);
        i2cWrite_PO2210K(0x4F,0x23);
        i2cWrite_PO2210K(0x50,0x2A);
        i2cWrite_PO2210K(0x51,0x37);
        i2cWrite_PO2210K(0x52,0x42);
        i2cWrite_PO2210K(0x53,0x56);
        i2cWrite_PO2210K(0x54,0x68);
        i2cWrite_PO2210K(0x55,0x87);
        i2cWrite_PO2210K(0x56,0xA3);
        i2cWrite_PO2210K(0x57,0xBC);
        i2cWrite_PO2210K(0x58,0xD4);
        i2cWrite_PO2210K(0x59,0xEA);
        i2cWrite_PO2210K(0x5A,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x5B,0x00);
        i2cWrite_PO2210K(0x5C,0x06);
        i2cWrite_PO2210K(0x5D,0x18);
        i2cWrite_PO2210K(0x5E,0x2C);
        i2cWrite_PO2210K(0x5F,0x3C);
        i2cWrite_PO2210K(0x60,0x54);
        i2cWrite_PO2210K(0x61,0x65);
        i2cWrite_PO2210K(0x62,0x7D);
        i2cWrite_PO2210K(0x63,0x8F);
        i2cWrite_PO2210K(0x64,0xAB);
        i2cWrite_PO2210K(0x65,0xC1);
        i2cWrite_PO2210K(0x66,0xD3);
        i2cWrite_PO2210K(0x67,0xE3);
        i2cWrite_PO2210K(0x68,0xF2);
        i2cWrite_PO2210K(0x69,0xFF);

        //Darkness X reference
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x98,0x00);
        i2cWrite_PO2210K(0x99,0x04);
        i2cWrite_PO2210K(0x9A,0x00);
        i2cWrite_PO2210K(0x9B,0x10);
        i2cWrite_PO2210K(0x9C,0x00);
        i2cWrite_PO2210K(0x9D,0x20);

        //ycont_th2
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE4,0x80);
        i2cWrite_PO2210K(0xE5,0x80);
        i2cWrite_PO2210K(0xE6,0x80);

        //BRIGHTNESS Default:0xE0
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95,0xE0);
        i2cWrite_PO2210K(0x96,0xE0);
        i2cWrite_PO2210K(0x97,0xE0);
        //CONTRAST Range:0x00-0xff Default:0x40
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE8,0x40);
        i2cWrite_PO2210K(0xE9,0x40);
        i2cWrite_PO2210K(0xEA,0x40);

        //SATURATION Range:0x00-0xff Default:0x30
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x30);
        //SHARPNESS Range:0x00-0xff Default:0x10
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x5B,0x10);
        i2cWrite_PO2210K(0x5C,0x10);
        i2cWrite_PO2210K(0x5D,0x10);
		
		//Auto exposure control
		i2cWrite_PO2210K(0x03,0x04);
		i2cWrite_PO2210K(0x12,0x01);
		i2cWrite_PO2210K(0x13,0x75);
		i2cWrite_PO2210K(0x14,0x01);
		i2cWrite_PO2210K(0x15,0x75);
		i2cWrite_PO2210K(0x16,0x01);
		i2cWrite_PO2210K(0x17,0x75);
		i2cWrite_PO2210K(0x1B,0x00);
		i2cWrite_PO2210K(0x1C,0x2e);
		i2cWrite_PO2210K(0x1D,0xA0);
		i2cWrite_PO2210K(0x1E,0x00);
		i2cWrite_PO2210K(0x1F,0x2e);
		i2cWrite_PO2210K(0x20,0xA0);
		
		i2cWrite_PO2210K(0x03,0x00);
		i2cWrite_PO2210K(0x4A,0x08);
    }
}
#elif (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK)
void siuSetSensorDayNight(u8 Level)
{
    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        //edge_gain_lf
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x53,0x18);//20161123
        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x3D,0x00);
        i2cWrite_PO2210K(0x3E,0x27);
        i2cWrite_PO2210K(0x3F,0x36);
        i2cWrite_PO2210K(0x40,0x40);
        i2cWrite_PO2210K(0x41,0x49);
        i2cWrite_PO2210K(0x42,0x58);
        i2cWrite_PO2210K(0x43,0x64);
        i2cWrite_PO2210K(0x44,0x78);
        i2cWrite_PO2210K(0x45,0x89);
        i2cWrite_PO2210K(0x46,0xA4);
        i2cWrite_PO2210K(0x47,0xBB);
        i2cWrite_PO2210K(0x48,0xCF);
        i2cWrite_PO2210K(0x49,0xE0);
        i2cWrite_PO2210K(0x4A,0xE1);
        i2cWrite_PO2210K(0x4B,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x4C,0x00);
        i2cWrite_PO2210K(0x4D,0x2B);
        i2cWrite_PO2210K(0x4E,0x37);
        i2cWrite_PO2210K(0x4F,0x43);
        i2cWrite_PO2210K(0x50,0x4F);
        i2cWrite_PO2210K(0x51,0x5C);
        i2cWrite_PO2210K(0x52,0x65);
        i2cWrite_PO2210K(0x53,0x71);
        i2cWrite_PO2210K(0x54,0x7A);
        i2cWrite_PO2210K(0x55,0x8E);
        i2cWrite_PO2210K(0x56,0xA3);
        i2cWrite_PO2210K(0x57,0xBC);
        i2cWrite_PO2210K(0x58,0xD4);
        i2cWrite_PO2210K(0x59,0xEA);
        i2cWrite_PO2210K(0x5A,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x5B,0x00);
        i2cWrite_PO2210K(0x5C,0x01);
        i2cWrite_PO2210K(0x5D,0x07);
        i2cWrite_PO2210K(0x5E,0x10);
        i2cWrite_PO2210K(0x5F,0x1C);
        i2cWrite_PO2210K(0x60,0x35);
        i2cWrite_PO2210K(0x61,0x4C);
        i2cWrite_PO2210K(0x62,0x70);
        i2cWrite_PO2210K(0x63,0x89);
        i2cWrite_PO2210K(0x64,0xAB);
        i2cWrite_PO2210K(0x65,0xC3);
        i2cWrite_PO2210K(0x66,0xD5);
        i2cWrite_PO2210K(0x67,0xE5);
        i2cWrite_PO2210K(0x68,0xF3);
        i2cWrite_PO2210K(0x69,0xFF);

        //Darkness X reference
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x98,0x00);
        i2cWrite_PO2210K(0x99,0x24);
        i2cWrite_PO2210K(0x9A,0x00);
        i2cWrite_PO2210K(0x9B,0x24);
        i2cWrite_PO2210K(0x9C,0x01);
        i2cWrite_PO2210K(0x9D,0x11);

        //ycont_th2
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE4,0x00);
        i2cWrite_PO2210K(0xE5,0x00);
        i2cWrite_PO2210K(0xE6,0x00);

        //BRIGHTNESS Default:0xE0
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95,0xE0);
        i2cWrite_PO2210K(0x96,0xE0);
        i2cWrite_PO2210K(0x97,0xE0);
        //BRIGHTNESS Default:0x14
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95,0x14);
        i2cWrite_PO2210K(0x96,0x14);
        i2cWrite_PO2210K(0x97,0x14);
        //CONTRAST Range:0x00-0xff Default:0x40
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE8,0x40);
        i2cWrite_PO2210K(0xE9,0x40);
        i2cWrite_PO2210K(0xEA,0x40);

        //SATURATION Range:0x00-0xff Default:0x00
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
        //SHARPNESS Range:0x00-0xff Default:0x10
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x5B,0x10);
        i2cWrite_PO2210K(0x5C,0x10);
        i2cWrite_PO2210K(0x5D,0x10);
		
#if 1
		//Auto exposure control
		i2cWrite_PO2210K(0x03,0x04);
		i2cWrite_PO2210K(0x12,0x04);
		i2cWrite_PO2210K(0x13,0x5E);
		i2cWrite_PO2210K(0x14,0x04);
		i2cWrite_PO2210K(0x15,0x5E);
		i2cWrite_PO2210K(0x16,0x04);
		i2cWrite_PO2210K(0x17,0x5E);
		i2cWrite_PO2210K(0x1B,0x00);
		i2cWrite_PO2210K(0x1C,0x6D);// x24 gain 20161123
		i2cWrite_PO2210K(0x1D,0x20);
		i2cWrite_PO2210K(0x1E,0x00);
		i2cWrite_PO2210K(0x1F,0x6D);// x24 gain
		i2cWrite_PO2210K(0x20,0x20);
#else
        //Auto exposure control
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0x57); //x20
        i2cWrite_PO2210K(0x1D,0x50);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x57);
        i2cWrite_PO2210K(0x20,0x50);
#endif

		i2cWrite_PO2210K(0x03,0x00);
		i2cWrite_PO2210K(0x4A,0x00);
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        //edge_gain_lf
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x53,0x80);//0x18 increase the sharpness 20161123
        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x3D,0x00);
        i2cWrite_PO2210K(0x3E,0x0B);
        i2cWrite_PO2210K(0x3F,0x1E);
        i2cWrite_PO2210K(0x40,0x2A);
        i2cWrite_PO2210K(0x41,0x33);
        i2cWrite_PO2210K(0x42,0x41);
        i2cWrite_PO2210K(0x43,0x53);
        i2cWrite_PO2210K(0x44,0x73);
        i2cWrite_PO2210K(0x45,0x86);
        i2cWrite_PO2210K(0x46,0xA5);
        i2cWrite_PO2210K(0x47,0xBD);
        i2cWrite_PO2210K(0x48,0xD3);
        i2cWrite_PO2210K(0x49,0xE3);
        i2cWrite_PO2210K(0x4A,0xEF);
        i2cWrite_PO2210K(0x4B,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x4C,0x00);
        i2cWrite_PO2210K(0x4D,0x11);
        i2cWrite_PO2210K(0x4E,0x1B);
        i2cWrite_PO2210K(0x4F,0x23);
        i2cWrite_PO2210K(0x50,0x2A);
        i2cWrite_PO2210K(0x51,0x37);
        i2cWrite_PO2210K(0x52,0x42);
        i2cWrite_PO2210K(0x53,0x56);
        i2cWrite_PO2210K(0x54,0x68);
        i2cWrite_PO2210K(0x55,0x87);
        i2cWrite_PO2210K(0x56,0xA3);
        i2cWrite_PO2210K(0x57,0xBC);
        i2cWrite_PO2210K(0x58,0xD4);
        i2cWrite_PO2210K(0x59,0xEA);
        i2cWrite_PO2210K(0x5A,0xFF);

        //gamma curve fitting
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x5B,0x00);
        i2cWrite_PO2210K(0x5C,0x06);
        i2cWrite_PO2210K(0x5D,0x18);
        i2cWrite_PO2210K(0x5E,0x2C);
        i2cWrite_PO2210K(0x5F,0x3C);
        i2cWrite_PO2210K(0x60,0x54);
        i2cWrite_PO2210K(0x61,0x65);
        i2cWrite_PO2210K(0x62,0x7D);
        i2cWrite_PO2210K(0x63,0x8F);
        i2cWrite_PO2210K(0x64,0xAB);
        i2cWrite_PO2210K(0x65,0xC1);
        i2cWrite_PO2210K(0x66,0xD3);
        i2cWrite_PO2210K(0x67,0xE3);
        i2cWrite_PO2210K(0x68,0xF2);
        i2cWrite_PO2210K(0x69,0xFF);

        //Darkness X reference
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x98,0x00);
        i2cWrite_PO2210K(0x99,0x04);
        i2cWrite_PO2210K(0x9A,0x00);
        i2cWrite_PO2210K(0x9B,0x10);
        i2cWrite_PO2210K(0x9C,0x00);
        i2cWrite_PO2210K(0x9D,0x20);

        //ycont_th2
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE4,0x80);
        i2cWrite_PO2210K(0xE5,0x80);
        i2cWrite_PO2210K(0xE6,0x80);

        //BRIGHTNESS Default:0xE0
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95,0xE0);
        i2cWrite_PO2210K(0x96,0xE0);
        i2cWrite_PO2210K(0x97,0xE0);
        //CONTRAST Range:0x00-0xff Default:0x40
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0xE8,0x2A);
        i2cWrite_PO2210K(0xE9,0x2A);
        i2cWrite_PO2210K(0xEA,0x40);

        //SATURATION Range:0x00-0xff Default:0x30
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x29);
        //SHARPNESS Range:0x00-0xff Default:0x10
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x5B,0x10);
        i2cWrite_PO2210K(0x5C,0x10);
        i2cWrite_PO2210K(0x5D,0x10);
		
		//Auto exposure control
		i2cWrite_PO2210K(0x03,0x04);
		i2cWrite_PO2210K(0x12,0x01);
		i2cWrite_PO2210K(0x13,0x75);
		i2cWrite_PO2210K(0x14,0x01);
		i2cWrite_PO2210K(0x15,0x75);
		i2cWrite_PO2210K(0x16,0x01);
		i2cWrite_PO2210K(0x17,0x75);
		i2cWrite_PO2210K(0x1B,0x00);
		i2cWrite_PO2210K(0x1C,0x2e);
		i2cWrite_PO2210K(0x1D,0xA0);
		i2cWrite_PO2210K(0x1E,0x00);
		i2cWrite_PO2210K(0x1F,0x2e);
		i2cWrite_PO2210K(0x20,0xA0);
		
		i2cWrite_PO2210K(0x03,0x00);
		i2cWrite_PO2210K(0x4A,0x08);
    }
}
#elif ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) )
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;
    u32 waitFlag;
    u8  err;

#if PO2210K_YUV601_FLAGTEST
    waitFlag = OSFlagPend(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuSetSensorDayNight SiuFlagGrp is 0x%x.\n", err);
		return ;
	}
#endif
    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        //Auto exposure control 64x1
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        #if 0 // 64x1
        i2cWrite_PO2210K(0x1B,0x01);
        i2cWrite_PO2210K(0x1C,0x17);
        i2cWrite_PO2210K(0x1D,0x80);
        i2cWrite_PO2210K(0x1E,0x01);
        i2cWrite_PO2210K(0x1F,0x17);
        i2cWrite_PO2210K(0x20,0x80);
        #else // 48x1
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0xD1);
        i2cWrite_PO2210K(0x1D,0xA0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0xD1);
        i2cWrite_PO2210K(0x20,0xA0);
        #endif
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab_N[siuY_TargetIndex]);
        
#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        //Auto exposure control
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0x8B);
        i2cWrite_PO2210K(0x1D,0xC0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x8B);
        i2cWrite_PO2210K(0x20,0xC0);
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);

        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,AESaturationTab[siuSaturationIndex]);
    }

#if PO2210K_YUV601_FLAGTEST
    OSFlagPost(SiuFlagGrp, SIU_EVET_SENSOR_SET, OS_FLAG_SET, &err);
#endif

}

#elif(HW_BOARD_OPTION  == MR9100_TX_RDI_USB)
void siuSetSensorDayNight(u8 Level)
{
	u8	data;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        //Auto exposure control 64x1
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        #if 0 // 64x1
        i2cWrite_PO2210K(0x1B,0x01);
        i2cWrite_PO2210K(0x1C,0x17);
        i2cWrite_PO2210K(0x1D,0x80);
        i2cWrite_PO2210K(0x1E,0x01);
        i2cWrite_PO2210K(0x1F,0x17);
        i2cWrite_PO2210K(0x20,0x80);
        #else // 48x1
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0xD1);
        i2cWrite_PO2210K(0x1D,0xA0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0xD1);
        i2cWrite_PO2210K(0x20,0xA0);
        #endif
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab_N[siuY_TargetIndex]);
        
#if 1 // 夜間彩色
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        //Auto exposure control
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0x8B);
        i2cWrite_PO2210K(0x1D,0xC0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x8B);
        i2cWrite_PO2210K(0x20,0xC0);
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);

        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,AESaturationTab[siuSaturationIndex]);
    }

}
#elif (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T || HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
//        i2cWrite_PO2210K(0x03,0x03);
//        i2cWrite_PO2210K(0x0D,0x00);
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x10);
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x48);
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x06,0xA1);
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x73,0x80);
        i2cWrite_PO2210K(0x74,0x80);
        i2cWrite_PO2210K(0x75,0x80);
        i2cWrite_PO2210K(0x76,0x7A);
        i2cWrite_PO2210K(0x77,0x80);
        i2cWrite_PO2210K(0x78,0x7A);
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x53,0x38);
        i2cWrite_PO2210K(0x54,0x10);
        i2cWrite_PO2210K(0x55,0x08);
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
    }
}
#elif (HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4)
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
        //Auto exposure control 64x1
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        #if 0 // 64x1
        i2cWrite_PO2210K(0x1B,0x01);
        i2cWrite_PO2210K(0x1C,0x17);
        i2cWrite_PO2210K(0x1D,0x80);
        i2cWrite_PO2210K(0x1E,0x01);
        i2cWrite_PO2210K(0x1F,0x17);
        i2cWrite_PO2210K(0x20,0x80);
        #else // 48x1
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0xD1);
        i2cWrite_PO2210K(0x1D,0xA0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0xD1);
        i2cWrite_PO2210K(0x20,0xA0);
        #endif
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab_N[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab_N[siuY_TargetIndex]);
        
#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        //Auto exposure control
        i2cWrite_PO2210K(0x03,0x04);
        i2cWrite_PO2210K(0x12,0x04);
        i2cWrite_PO2210K(0x13,0x5E);
        i2cWrite_PO2210K(0x14,0x04);
        i2cWrite_PO2210K(0x15,0x5E);
        i2cWrite_PO2210K(0x16,0x04);
        i2cWrite_PO2210K(0x17,0x5E);
        i2cWrite_PO2210K(0x1B,0x00);
        i2cWrite_PO2210K(0x1C,0x8B);
        i2cWrite_PO2210K(0x1D,0xC0);
        i2cWrite_PO2210K(0x1E,0x00);
        i2cWrite_PO2210K(0x1F,0x8B);
        i2cWrite_PO2210K(0x20,0xC0);
        
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);

        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,AESaturationTab[siuSaturationIndex]);
    }

}
#else
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode    = Level;
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,0x00);
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_PO2210K(0x03,0x03);
        i2cWrite_PO2210K(0x0D,AESaturationTab[siuSaturationIndex]);
        //ybrightness
        i2cWrite_PO2210K(0x03,0x02);
        i2cWrite_PO2210K(0x95 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x96 ,AETargetMeanTab[siuY_TargetIndex]);
        i2cWrite_PO2210K(0x97 ,AETargetMeanTab[siuY_TargetIndex]);
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
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
            {
	            Img_Width   = 1920;
              #if ENABLE_H264_1080
	            Img_Height  = 1080;
              #else
	            Img_Height  = 1072;
              #endif
	                        
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
    u8 err;
    // set SIU_OE = 0
    //SiuSensCtrl &= 0xffffdfff;
    //SiuSensCtrl &= ~(SIU_CAPT_ENA | SIU_INT_ENA_FRAM | SIU_DEF_PIX_ENA);
    #if PO2210K_YUV601_FLAGTEST
    SiuFlagGrp=OSFlagDel(SiuFlagGrp, OS_DEL_ALWAYS, &err);
    if(err != OS_NO_ERR)
        DEBUG_SIU("SiuFlagGrp OSFlagDel error %d\n", err);
    #endif
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

void siu_SetPO2210K_720P_FrameRate(u32 FrameRate)
{

    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
    {
        if(PO2210K_720P_FrameRate  != FrameRate)
        {
            DEBUG_SIU("siu_SetPO2210K_720P_FrameRate(%d)\n", FrameRate);
            if(FrameRate == 15)
            {
                SetPO2210K_720P_15FPS();
                PO2210K_720P_FrameRate=15;
            }
            else if(FrameRate == 20)
            {
                SetPO2210K_720P_20FPS();
                PO2210K_720P_FrameRate=20;
            }
            else if(FrameRate == 21)
            {
                SetPO2210K_720P_21FPS();
                PO2210K_720P_FrameRate=21;
            }
        }
    }
}
void siu_SetPO2210K_1080P_FrameRate(u32 FrameRate)
{
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
    {
        if(PO2210K_1080P_FrameRate  != FrameRate)
        {
            DEBUG_SIU("siu_SetPO2210K_1080P_FrameRate(%d)\n", FrameRate);
            if(FrameRate == 10)
            {
                SetPO2210K_1080P_10FPS();
                PO2210K_1080P_FrameRate=FrameRate;
            }
            else if(FrameRate == 12)
            {
                SetPO2210K_1080P_12FPS();
                PO2210K_1080P_FrameRate=FrameRate;
            }
        }
    }
}

#endif	






