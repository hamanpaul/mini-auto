
/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu_CCIR656.c

Abstract:

    The routines of Sensor Interface Unit.
    1. TV decoder 參數設定:
        a. Preview/Capture/Video clip mode.
    2. SIU 參數設定.          

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2009/01/07  Lucian Yuan  Create  
*/


#include "general.h"
#if ( (Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV))	//Lisa 5M
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
#include "asfapi.h"
#include "logfileapi.h"
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
 
/*--------- For new AE Algorithm---------*/
// For new AE Table

#define AECNT_PREVIEW                     5    //30/(5+1)=5 次/sec (motion detection)
#define AECNT_VideoCap                   24

#define Gain_Level 6
#define AE_E_Table_Num	    16
#define AE_Table_Row_Num	15

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
        
        {{640,  480},  200},//9        
        {{592,  444},  211},//10
        {{544,  408},  235},//11
        {{512,  384},  250},//12
        {{464,  348},  267},//13
        {{432,  324},  286},//14
        {{400,  300},  308},//15
        {{368,  276},  333},//16
        {{352,  264},  364},//17
        {{336,  252},  390},//18

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
u8 siuY_TargetIndex=4;
u8 AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_60HZ;


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
 
extern u32 sysVideoInSel;
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


#if( (Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV))
 extern  u8  FieldTypeID;  // 1: odd field, 0: even field
#endif
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

u16 Cal_LOG2(u16 X);
 

#if ( (Sensor_OPTION == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_OV7740_RAW) )
 s32 sysBack_Set_Sensor_Color(s32 dummy);
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

void siuCCIR656_Ena(void)
{
    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08362000;
                SIU_656_DATALATCH_FAL   |
                SIU_YUVMAP_27           |
                SIU_FIELD_POL_POS;

}

s32 siuPreviewInit(u8 zoomFactor,u8 mode)
{
    //while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
    
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
                SIU_INT_DISA_FRAM |
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
        #if(Sensor_OPTION ==Sensor_CCIR656)
            #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
            #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
            #endif        
        		SIU_PIX_CLK_ACT_FAL |                   //falling edge latch 
        #else
            #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
            #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
            #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
        #endif
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_DISA |                      //OB correct disable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                           //AE report disable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

#if(Sensor_OPTION ==Sensor_CCIR656)
    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08362000;
                SIU_656_DATALATCH_FAL   |
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27           |
                SIU_FIELD_POL_POS;

#elif(Sensor_OPTION ==Sensor_CCIR601)
    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27;
#elif(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)
    if(sysVideoInSel==VIDEO_IN_TV)
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_27;
    else
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_36;
#endif
        
	
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
                SIU_INT_DISA_FRAM |
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
        #if(Sensor_OPTION ==Sensor_CCIR656)
            #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
            #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
            #endif       
        		SIU_PIX_CLK_ACT_FAL |                   //falling edge latch 
        #else
           #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
           #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
           #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
        #endif
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_DISA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                            //AE report enable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

#if(Sensor_OPTION ==Sensor_CCIR656)
    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08362000;
                SIU_656_DATALATCH_FAL   |
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27           |
                SIU_FIELD_POL_POS;

#elif(Sensor_OPTION ==Sensor_CCIR601)
    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27;
#elif(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)
    if(sysVideoInSel==VIDEO_IN_TV)
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_27;
    else
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_36;
#endif

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
    u16 data;
	int i;	
    int FrameRate=30;

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;
#endif


#if(Sensor_OPTION ==Sensor_CCIR601_MIX_OV7740YUV)
    if(sysVideoInSel==VIDEO_IN_SENSOR)
    {
       switch(siuSensorMode)   	
       {	
       case SIUMODE_PREVIEW: 
       case SIUMODE_MPEGAVI: 
       case SIUMODE_CAPTURE: 
        
            //---Software reset---//
            i2cWrite_SENSOR( 0x12, 0x80);
            for(i=0;i<10000;i++);
            i2cWrite_SENSOR(0x13, 0x00);
            i2cWrite_SENSOR(0x11, 0x01); //00/01/03/07 for 60/30/15/7.5fps
            i2cWrite_SENSOR(0x12, 0x00);
            i2cWrite_SENSOR(0xd5, 0x10);
            

        #if SENSOR_ROW_COL_MIRROR
            i2cWrite_SENSOR( 0x0c, 0x02); //H mirror, V flip
            i2cWrite_SENSOR( 0x16, 0x00); //H start: D1:0
            i2cWrite_SENSOR( 0x17, 0x25); //Sensor Horizontal Output start D9:2
        #else
            i2cWrite_SENSOR( 0x0c, 0xc2); //H mirror, V flip
            i2cWrite_SENSOR( 0x16, 0x01); //H start: D1:0
            i2cWrite_SENSOR( 0x17, 0x25); //Sensor Horizontal Output start D9:2
        #endif
            i2cWrite_SENSOR(0x0d, 0x34);
            i2cWrite_SENSOR(0x18, 0xa0);
            i2cWrite_SENSOR(0x19, 0x03);
            i2cWrite_SENSOR(0x1a, 0xf0);
            i2cWrite_SENSOR(0x1b, 0x89);
            i2cWrite_SENSOR(0x1e, 0x13);
            i2cWrite_SENSOR(0x22, 0x03);
    
            i2cWrite_SENSOR( 0x28, 0x02); //Vsync negative,Hsync positive
            i2cWrite_SENSOR(0x29, 0x17);
            i2cWrite_SENSOR(0x2b, 0xf8);
            i2cWrite_SENSOR(0x2c, 0x01);
            i2cWrite_SENSOR(0x31, 0xa0);
            i2cWrite_SENSOR(0x32, 0xf0);
            i2cWrite_SENSOR(0x33, 0xc4);
            //42 35 05 ;60fps
            i2cWrite_SENSOR(0x3a, 0xb4); //<= 30fps
            i2cWrite_SENSOR(0x36, 0x3f);

            i2cWrite_SENSOR(0x04, 0x60);
            i2cWrite_SENSOR(0x27, 0x80);
            i2cWrite_SENSOR(0x3d, 0x0f); //08/0f/0f/0f for 60/30/15/7.5fps
            i2cWrite_SENSOR(0x3e, 0x82);
            i2cWrite_SENSOR(0x3f, 0x40);
            i2cWrite_SENSOR(0x40, 0x7f);
            i2cWrite_SENSOR(0x41, 0x6a);
            i2cWrite_SENSOR(0x42, 0x29);
            i2cWrite_SENSOR(0x44, 0xe5); //f5/e5/e5/e5 for 60/30/15/7.5fps
            i2cWrite_SENSOR(0x45, 0x41);
            i2cWrite_SENSOR(0x47, 0x42);
            i2cWrite_SENSOR(0x48, 0x00);
            i2cWrite_SENSOR(0x49, 0x61);
            i2cWrite_SENSOR(0x4a, 0xa1);
            i2cWrite_SENSOR(0x4b, 0x46);
            i2cWrite_SENSOR(0x4c, 0x18);
            i2cWrite_SENSOR(0x4d, 0x50);
            i2cWrite_SENSOR(0x4e, 0x13);
            i2cWrite_SENSOR(0x64, 0x00);
            i2cWrite_SENSOR(0x67, 0x88);
            i2cWrite_SENSOR(0x68, 0x1a);

            i2cWrite_SENSOR(0x14, 0x38); //AGC max to 16x
            i2cWrite_SENSOR(0x24, 0x3c);
            i2cWrite_SENSOR(0x25, 0x30);
            i2cWrite_SENSOR(0x26, 0x72);
            
            i2cWrite_SENSOR(0x50, 0x97); // 2e/97/4c/26 for 60/30/15/7.5fps; //Banding starting step for 50Hz (LSB 7:0)
            i2cWrite_SENSOR(0x51, 0x7e); //fc/7e/3f/20 for 60/30/15/7.5fps   //Banding starting step for 60Hz (LSB 7:0)
            i2cWrite_SENSOR(0x52, 0x00); //10/00/00/00 for 60/30/15/7.5fps   //MSB(9:8) part of banding starting steop.
            
            i2cWrite_SENSOR(0x20, 0x00);
            i2cWrite_SENSOR(0x21, 0x23); //01/23/57/cf for 60/30/15/7.5fps
            i2cWrite_SENSOR(0x38, 0x14);
            i2cWrite_SENSOR(0xe9, 0x00);

            //---AE weigth table: bellow 2/3 enhance--//
            i2cWrite_SENSOR(0x82, 0x32);  //Enable y average/FIFO/window cropping
            i2cWrite_SENSOR(0x83, 0x03);  //ISP control, Output RAW10 data
            
            i2cWrite_SENSOR(0x38, 0x12); //reserved
            i2cWrite_SENSOR(0xe9, 0x00); //AHS

            i2cWrite_SENSOR(0x38, 0x13); //reserved
            i2cWrite_SENSOR(0xe9, 0x00); //AVS
            
            i2cWrite_SENSOR(0x38, 0x14); //reserved
            i2cWrite_SENSOR(0xe9, 0x20); //AE window set by the register

            i2cWrite_SENSOR(0x38, 0x15); //reserved
            i2cWrite_SENSOR(0xe9, 0x50); //AHW  x8

            i2cWrite_SENSOR(0x38, 0x16); //reserved
            i2cWrite_SENSOR(0xe9, 0x78); //AVH  x4

            i2cWrite_SENSOR(0x53, 0x02);

            i2cWrite_SENSOR(0x56, 0x00);
            i2cWrite_SENSOR(0x57, 0x14);
            i2cWrite_SENSOR(0x58, 0x14);
            i2cWrite_SENSOR(0x59, 0x00);
            
            i2cWrite_SENSOR(0x5f, 0x04);

            i2cWrite_SENSOR(0x80, 0x7d);
            i2cWrite_SENSOR(0x81, 0x2f);  //CIP/CMX/UV_avg/yuv422/SDE enable, uv_adj disable.
            
            i2cWrite_SENSOR(0x38, 0x11);
            i2cWrite_SENSOR(0x84, 0x70);
            i2cWrite_SENSOR(0x85, 0x00);
            i2cWrite_SENSOR(0x86, 0x03);
            i2cWrite_SENSOR(0x87, 0x01);
            i2cWrite_SENSOR(0x88, 0x05);
            i2cWrite_SENSOR(0x89, 0x30);
            i2cWrite_SENSOR(0x8d, 0x30);
            i2cWrite_SENSOR(0x8f, 0x85);
            i2cWrite_SENSOR(0x93, 0x30);
            i2cWrite_SENSOR(0x95, 0x85);
            i2cWrite_SENSOR(0x99, 0x30);
            i2cWrite_SENSOR(0x9b, 0x85);

            i2cWrite_SENSOR(0x9c, 0x08);
            i2cWrite_SENSOR(0x9d, 0x12);
            i2cWrite_SENSOR(0x9e, 0x23);
            i2cWrite_SENSOR(0x9f, 0x45);
            i2cWrite_SENSOR(0xa0, 0x55);
            i2cWrite_SENSOR(0xa1, 0x64);
            i2cWrite_SENSOR(0xa2, 0x72);
            i2cWrite_SENSOR(0xa3, 0x7f);
            i2cWrite_SENSOR(0xa4, 0x8b);
            i2cWrite_SENSOR(0xa5, 0x95);
            i2cWrite_SENSOR(0xa6, 0xa7);
            i2cWrite_SENSOR(0xa7, 0xb5);
            i2cWrite_SENSOR(0xa8, 0xcb);
            i2cWrite_SENSOR(0xa9, 0xdd);
            i2cWrite_SENSOR(0xaa, 0xec);
            i2cWrite_SENSOR(0xab, 0x1a);

            i2cWrite_SENSOR(0xce, 0x78);

            //---sharpness & Denoise ---//
            i2cWrite_SENSOR(0xcc, 0x60); //manual setting for sharpness and denoise
            i2cWrite_SENSOR(0xcb, 0x00); //manual setting for denoise=0
            i2cWrite_SENSOR(0xcd, 0x06); 
            
            i2cWrite_SENSOR(0xcf, 0x6e);
            i2cWrite_SENSOR(0xd0, 0x0a);
            i2cWrite_SENSOR(0xd1, 0x0c);
            i2cWrite_SENSOR(0xd2, 0x84);
            i2cWrite_SENSOR(0xd3, 0x90);
            i2cWrite_SENSOR(0xd4, 0x1e);

            i2cWrite_SENSOR(0x5a, 0x24);
            i2cWrite_SENSOR(0x5b, 0x1f);
            i2cWrite_SENSOR(0x5c, 0x88);
            i2cWrite_SENSOR(0x5d, 0x60);

            i2cWrite_SENSOR(0xac, 0x6e);
            i2cWrite_SENSOR(0xbe, 0xff);
            i2cWrite_SENSOR(0xbf, 0x00);

            //50/60Hz auto detection is XCLK dependent
            //the following is based on XCLK = 24MHz
            i2cWrite_SENSOR(0x70, 0x00);
            i2cWrite_SENSOR(0x71, 0x34);
            i2cWrite_SENSOR(0x74, 0x28);
            i2cWrite_SENSOR(0x75, 0x98);
            i2cWrite_SENSOR(0x76, 0x00);
            i2cWrite_SENSOR(0x77, 0x08);
            i2cWrite_SENSOR(0x78, 0x01);
            i2cWrite_SENSOR(0x79, 0xc2);
            i2cWrite_SENSOR(0x7d, 0x02);
            i2cWrite_SENSOR(0x7a, 0x9c);
            i2cWrite_SENSOR(0x7b, 0x40);
            if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
               i2cWrite_SENSOR(0xec, 0x02); //02/82 for manual/auto
            else
               i2cWrite_SENSOR(0xec, 0x42); //02/82 for manual/auto
            i2cWrite_SENSOR(0x7c, 0x0c);
            
    
            i2cWrite_SENSOR( 0xda, 0x02); //Saturation disable
            i2cWrite_SENSOR( 0xdd, 0x40); //U saturation
            i2cWrite_SENSOR( 0xde, 0x40); //V saturation 
            i2cWrite_SENSOR(0x13, 0xff);

            break;
        }
    }
    //----------------//
#endif	
    
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
    u8 FID;

#if (DEBUG_SIU_FRAM_STR_INTR_USE_LED6 &&(FPGA_BOARD_A1018_SERIES))
    static u32 Debug_count=0;
#endif

#if(DEBUG_SIU_BOTFD_END_INTR_USE_LED7 &&(FPGA_BOARD_A1018_SERIES))
    static u32 Debug_count1=0;
#endif
    
#if FINE_TIME_STAMP // use IIS time + Timer3 to calculate frame time
	s32 IISTimeOffsetSIU, TimeOffset;
	u32 IISTime1;
#endif
	
	intStat = SiuIntStat;
    FID=(SiuSyncStat>>31) & 0x01;
    
 #if (ISU_OUT_BY_FID)
    isuOutputAddrArrange_TV();
 #endif

	    
    if(FID==1)
       return;
    
	if (intStat & SIU_INT_STAT_FIFO_OVERF)
	{
		/* Raw data FIFO overflow */
		DEBUG_SIU("\nSIU Data Overflow\n");
	}
	
	if ( (intStat & SIU_INT_STAT_FRAM) || (intStat & SIU_INT_STAT_FRAME_STR) || (intStat & SIU_INT_STAT_CCIR_TOP_STR) ) //Lucian: 由AP決定該開哪一個
	{
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
		#if ( ISU_OUT_BY_FID)

        #elif (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC)
             isuOutputAddrArrange_Sensor();
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

    if (intStat & SIU_INT_STAT_CCIR_BOT_END)
    {
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

	Set color tune.

Arguments:

	tune - Color tune.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetColorTune(u8 tune)
{
			
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

	siuPreviewInit(zoomFactor,SIUMODE_PREVIEW);
    /* SW 0222 E */
    siuAeEnable = 1;

	
	
	return 1;
}

s32 siuPreviewZoom(s8 zoomFactor)
{
    siuOpMode = SIUMODE_PREVIEW;
	
	// Set sensor control 
	SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |                      //D.pixel compesation disable
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_DISA_FRAM |
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
        #if(Sensor_OPTION ==Sensor_CCIR656)
        	  	SIU_DATA_10b |                          // data <<2, CCIR656 must be
        		SIU_PIX_CLK_ACT_FAL |                   //falling edge latch 
        #else
            #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
            #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
            #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
        #endif
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_DISA |                      //OB correct disable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                           //AE report disable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.


#if(Sensor_OPTION ==Sensor_CCIR656)
    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08362000;
                SIU_656_DATALATCH_FAL   |
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27           |
                SIU_FIELD_POL_POS;

#elif(Sensor_OPTION ==Sensor_CCIR601)
    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27;
#elif(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)
    if(sysVideoInSel==VIDEO_IN_TV)
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_27;
    else
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_36;
#endif
        
	
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
	
	
	//while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
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
    
  // Set sensor control 
	SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_DISA_FRAM |
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
        #if(Sensor_OPTION ==Sensor_CCIR656)
        	  	SIU_DATA_10b |                          // data <<2, CCIR656 must be
        		SIU_PIX_CLK_ACT_FAL |                   //falling edge latch 
        #else
           #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
           #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
           #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
        #endif
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_DISA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                            //AE report enable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.


#if(Sensor_OPTION ==Sensor_CCIR656)
    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08362000;
                SIU_656_DATALATCH_FAL   |
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27           |
                SIU_FIELD_POL_POS;

#elif(Sensor_OPTION ==Sensor_CCIR601)
    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_FRAME_STR_INT_EN    |
                SIU_YUVMAP_27;
#elif(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)
    if(sysVideoInSel==VIDEO_IN_TV)
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_27;
    else
       SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                   SIU_FRAME_STR_INT_EN    |
                   SIU_YUVMAP_36;
#endif

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

#define DAY_MODE_SWITCH_THR   0x7f
#define NIGHT_MODE_SWITCH_THR 0x7f
#define DENOISE_ON_THR        0x70
#define MODESWITCH_THR        3

void siuTask(void* pData)
{
	u8 err;
    u16 Again;
    static int  Mode=SIU_DAY_MODE;
    static unsigned int DayCount=0;
    static unsigned int NightCount=0;
    
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

    #if(Sensor_OPTION ==Sensor_CCIR601_MIX_OV7740YUV)
      if(sysVideoInSel==VIDEO_IN_SENSOR)
      {
        i2cRead_SENSOR(0x0, &Again);
        //DEBUG_SIU("Again=0x%x ",Again);
        if(Again>=DENOISE_ON_THR) // >= 8x
        {
            i2cWrite_SENSOR(0xcc, 0x62); //manual setting for sharpness and denoise,sharpness=2
            i2cWrite_SENSOR(0xcb, 0x04); //manual setting for denoise=4
            i2cWrite_SENSOR(0xcd, 0x06); //sharpness thr=6
        }
        else
        {
            i2cWrite_SENSOR(0xcc, 0x64); //manual setting for sharpness and denoise,,sharpness=4
            i2cWrite_SENSOR(0xcb, 0x02); //manual setting for denoise=1
            i2cWrite_SENSOR(0xcd, 0x06); //sharpness thr=6
        }
        
        //----Mode select----//
        if(Mode ==SIU_DAY_MODE)
        {
           if( Again>=DAY_MODE_SWITCH_THR )
              NightCount ++;
           else
              NightCount =0;

           if(NightCount> MODESWITCH_THR) 
           {
              Mode =SIU_NIGHT_MODE;  //switch to Night mode
              DayCount =0;
           }
        }
        else //Night mode
        {
           if( Again < NIGHT_MODE_SWITCH_THR )
              DayCount ++;
           else
              DayCount =0; 

           if(DayCount> MODESWITCH_THR)
           {
              Mode =SIU_DAY_MODE; //switch to Day mode
              NightCount=0;
           }
        }
		
		if(Mode ==SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
		{  //夜間模式
		   i2cWrite_SENSOR(0xcc, 0x60); //manual setting for sharpness and denoise,sharpness=0
           i2cWrite_SENSOR(0xcb, 0x08); //manual setting for denoise=8 
           i2cWrite_SENSOR(0xcd, 0x08); //sharpness thr=8
           i2cWrite_SENSOR( 0xdd, 0x00); //U saturation
           i2cWrite_SENSOR( 0xde, 0x00); //V saturation 
		}
        else
        { //日間模式
		    i2cWrite_SENSOR( 0xdd, 0x40); //U saturation
            i2cWrite_SENSOR( 0xde, 0x40); //V saturation 
        }
     }
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
s32 siuSetAwbWin()
{

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
    
    if(sysVideoInSel == VIDEO_IN_TV)
    {
    #if(Sensor_OPTION  == Sensor_CCIR656)
          #if(TV_DECODER == BIT1605) //use bit1605 tv decoder 
             if(sysTVinFormat == TV_IN_PAL)
             {
                Img_Width =684; 
                Img_Height=576/2;
             }
             else
             {
                Img_Width =684; 
                Img_Height=480/2;
             }
          #elif(TV_DECODER == TI5150)
             if(sysTVinFormat == TV_IN_PAL)
             {
                Img_Width =704; 
                Img_Height=576/2;
             }
             else
             {
                Img_Width =704; 
                Img_Height=480/2;
             }
          #elif(TV_DECODER == MI9V136)
             if(sysTVinFormat == TV_IN_PAL)
             {
                Img_Width =640; 
                Img_Height=576/2;
             }
             else
             {
                Img_Width =640; 
                Img_Height=480/2;
             }
	      #else
		     if(sysTVinFormat == TV_IN_PAL)
             {
                Img_Width =704; 
                Img_Height=576/2;
             }
             else
             {
                Img_Width =704; 
                Img_Height=480/2;
             }
          #endif
             sensor_validsize.imgStr.x = 2;
             sensor_validsize.imgStr.y = 0;
    #else
         #if(TV_DECODER == BIT1605) //use bit1605 tv decoder         
            if(sysTVinFormat == TV_IN_PAL)
            {
               Img_Width =684;//704;  //有些sensor 不支援704/270 width,Lucian: 暫時, 704
               Img_Height=576/2;	
			   sensor_validsize.imgStr.x = SENSOR_PAL_START_X;//163;
			   sensor_validsize.imgStr.y = SENSOR_PAL_START_Y;		  
            }
            else
            {
               Img_Width =684; 
               Img_Height=480/2;
			   sensor_validsize.imgStr.x = SENSOR_NTSC_START_X;//139;//139;
			   sensor_validsize.imgStr.y = SENSOR_NTSC_START_Y;	  
            } 
         #elif(TV_DECODER == TI5150) //use TI5150 tv decoder         
             
            if(sysTVinFormat == TV_IN_PAL)
            {
               Img_Width =704;//704;  //有些sensor 不支援704/270 width,Lucian: 暫時, 704
               Img_Height=576/2;
               sensor_validsize.imgStr.x = 166;//138;// 134
               sensor_validsize.imgStr.y = 17;//11        
            }
            else
            {
               Img_Width =704; 
               Img_Height=480/2;
               sensor_validsize.imgStr.x = 142;//138;// 134
               sensor_validsize.imgStr.y = 15;//11        
               
            } 
         #elif(TV_DECODER == MI9V136)
             if(sysTVinFormat == TV_IN_PAL)
             {
                Img_Width =640; 
                Img_Height=576/2;
             }
             else
             {
                Img_Width =640; 
                Img_Height=480/2;
             }
             sensor_validsize.imgStr.x = 2;
             sensor_validsize.imgStr.y = 0;
	     #else
	        if(sysTVinFormat == TV_IN_PAL)
            {
               Img_Width =704;//704;  //有些sensor 不支援704/270 width,Lucian: 暫時, 704
               Img_Height=576/2;
               sensor_validsize.imgStr.x = 166;//138;// 134
               sensor_validsize.imgStr.y = 17;//11        
            }
            else
            {
               Img_Width =704; 
               Img_Height=480/2;
               sensor_validsize.imgStr.x = 142;//138;// 134
               sensor_validsize.imgStr.y = 15;//11        
               
            } 
         
         #endif //#if(TV_DECODER == BIT1605) //use bit1605 tv decoder         
    #endif        
    		sensor_validsize.imgSize.x = Img_Width*2;
    		sensor_validsize.imgSize.y = Img_Height;
    		
	
    		SiuValidSize = 	(sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
			                (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);	
        
        	SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
        			        (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
                
            ipuSetIOSize(Img_Width, Img_Height);
         	isuSetIOSize(Img_Width, Img_Height); 
    }
    else
    {
            Img_Width =640; 
            Img_Height=480;
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
	uiDigZoomSetFlag = 0;

	return 1;
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
    *W=PreviewZoomParm_VGA[0].size.x;
    *H=PreviewZoomParm_VGA[0].size.y;
}


u16 getVideoZoomScale(s32 zoomFactor)
{
     return 0; 
}

u16 getSensorRawWidth(void)
{
    return   sensor_validsize.imgSize.x; 
}

/*

Routine Description:

	Get tv-in status. If Hsync and Vsync is locked, the related pins should be "1".

Arguments:

	None.
	
Return Value:

	tv-in status.

*/
u8 siuGetTVinStatus()
{
    u8 data;

    #if(TV_DECODER == BIT1605) //use bit1605 tv decoder

	#elif(TV_DECODER == TI5150)
      /* read the input video status from TVP5150 */
      i2cRead_TVP5150(0x88, &data,TV_CHECK_FORMAT_RD_ADDR);
	  data &= 0x06;    
	#else

    #endif
    return data;
}

void siu_FCINTE_ena(void)
{
   SiuSensCtrl |= SIU_INT_ENA_FRAM; 
}

void siu_FCINTE_disa(void)
{
   SiuSensCtrl &= (~SIU_INT_ENA_FRAM); 
}

  #if(TV_DECODER==BIT1605 )
    void siu_FID_INT_ena()
    {
    }

    void siu_FID_INT_disa()
    {
    }
  #else
    void siu_FID_INT_ena()
    {
    }

    void siu_FID_INT_disa()
    {
    }
  #endif        

/*

Routine Description:

	SIU control.

Arguments:

	ucCtrl - Enable/ Disable siu. 1 -> Enable, 0 -> Disable.
	
Return Value:

	None.

*/
void siuSiuCtrl(u8 ucCtrl)
{

	if (ucCtrl == GLB_ENA)
	{
		SiuSensCtrl |= SIU_ENA;
	}
	else
	{
		SiuSensCtrl &= ~SIU_ENA;
	}

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
  
#endif	//Lisa 5M 




