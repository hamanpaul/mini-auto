
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
#if (Sensor_OPTION == Sensor_OV7725_YUV601)
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
 //----- Function Test -----//
 



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


#define SW_CAPTURE2PREVIEW_RATION      1000   //x1000, 




#define PULSE_TIME  250   //每輸出250次Hight Level 大約 花0.5ms


#define  SIU_OV7725_AIO     0
#define  SIU_OV7725_MARS2   1
#define  SIU_OV7725_RDI     2
#define  SIU_OV7725_MAYON   3


#define SIU_OV7725_PARAMETER    SIU_OV7725_AIO

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
#if 1        
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
#else   //Fit small pannel: 128x128
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
#endif        

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




u8 siu_dftAeWeight2[25] = {  //閃光燈用之 AE weight table.
		8, 8, 8, 8, 8,
		8, 9, 9, 9, 8,
		8, 9,10, 9, 8,
		8, 9,10, 9, 8,
		7, 8, 8, 8, 7
};

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
 

extern s8 gpio_IIC_Read(u8, u32, u8, u8 *);

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
        		SIU_OB_COMP_DISA |                      //OB correct disable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                           //AE report disable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.


    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_YUVMAP_27;
                //SIU_YUVMAP_63;
        
	
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
                SIU_YUVMAP_27;
    
    SiuSensCtrl |= SIU_CAPT_ENA;
    	 	    		
	return 1;	
}



#if 0
void SetOV7725_SUNIN1(void)
{
	u16 data;
    int i;

    i2cWrite_SENSOR(0x12, 0x0080); //SCCB Register reset
    for(i=0;i<10000;i++);
    i2cWrite_SENSOR( 0x12, 0x0000); //YUV out, VGA resolution

    i2cRead_SENSOR(0x0a, &data);
    DEBUG_SIU("PID_M=0x%x\n",data);
    i2cRead_SENSOR(0x0b, &data);
    DEBUG_SIU("PID_L=0x%x\n",data);

    i2cWrite_SENSOR( 0x67, 0x0000); //AEC before gamma, YUV selected.
    i2cWrite_SENSOR( 0x3d, 0x80);  //DC offset compensation for analog process


#if SENSOR_ROW_COL_MIRROR  
    i2cWrite_SENSOR(0x0c,0x00); //rotate 180 degree
#else
    i2cWrite_SENSOR( 0x0c, 0xc0); //rotate 0 degree
#endif
    //----AWB Control ----//
    i2cWrite_SENSOR(0x01, 0xa0);
    i2cWrite_SENSOR(0x6b, 0xaa);      //AWB mode select: simple AWB


    //i2cWrite_SENSOR(0x69, 0x54);      //AWB gain control
    //i2cWrite_SENSOR(0x4d, 0x00a0);    //Analog Fix Gain Amplifier: B x 1.5, R x1.5

    //--- AE Control window ---//
#if 1
    i2cWrite_SENSOR( 0x24, 0x0048); //Stable operating Region (Upper limit)
    i2cWrite_SENSOR( 0x25, 0x0038); //Stabel operation Region (Lower limit)
    i2cWrite_SENSOR( 0x26, 0x00b2); //AGC/AEC fast mode operating region 
#else
    i2cWrite_SENSOR( 0x24, 0x0040); //Stable operating Region (Upper limit)
    i2cWrite_SENSOR( 0x25, 0x0030); //Stabel operation Region (Lower limit)
    i2cWrite_SENSOR( 0x26, 0x00b6); //AGC/AEC fast mode operating region 
#endif


    //---Set Input window size ---//
    i2cWrite_SENSOR( 0x17, 0x0026);
    i2cWrite_SENSOR( 0x18, 0x00A2);
    i2cWrite_SENSOR( 0x19, 0x0007);
    i2cWrite_SENSOR( 0x1A, 0x00F2);

    //---Set Output window size---//
    i2cWrite_SENSOR( 0x32, 0x0000);  //HOutSize (LSB)
    i2cWrite_SENSOR( 0x29, 0x00A0);  //HOutSize (MSB) =640

    i2cWrite_SENSOR( 0x2C, 0x00F0);  //VOutSize (MSBS)=480
    i2cWrite_SENSOR( 0x2A, 0x0000);  //VOutSize (LSB) 
    //-------PLL Setting--------//

    //i2cWrite_SENSOR( 0x0d, 0x71);    //PLL 4x,AEC Low 2/3 window
    //i2cWrite_SENSOR( 0x0d, 0x61);    //PLL 4x,AEC Low 1/4 window
    //i2cWrite_SENSOR( 0x0d, 0x51);    //PLL 4x,AEC Low 1/2 window
    i2cWrite_SENSOR( 0x0d, 0x41);    //PLL 4x,AEC full window

    i2cWrite_SENSOR( 0x11, 0x0001);  // Internal clock= input_clock*4/((1+1)*2)=input_clock = 24MHz

    i2cWrite_SENSOR( 0x4D, 0x0000);  //Analog gain fix to 1x

    i2cWrite_SENSOR( 0x63, 0x00F0);
    i2cWrite_SENSOR( 0x64, 0x001F);  // 亮度不足時,設成 0XFB 關掉Gamma.

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 128);    
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x0000); //Insert dummy line. -->7
        i2cWrite_SENSOR(0x34, 0);
    }
    else //50Hz
    {
        i2cWrite_SENSOR(0x22, 153);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 102);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0);
    }
    //i2cWrite_SENSOR(0x14, 0x0010);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0090);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0020);  //Max AGC to 8x
    i2cWrite_SENSOR(0x14, 0x0030);  //Max AGC to 16x

    //---Signal setting---//
    i2cWrite_SENSOR( 0x32, 0x40); 
    i2cWrite_SENSOR( 0x66, 0x80);    //Y0V0 Y1U1 Y2U2 Y3U3
                  
    i2cWrite_SENSOR( 0x15, 0x02);    //Vsync negative
    //-- Denoise Strength--//

    i2cWrite_SENSOR( 0xac, 0xbf);    //Manual De-noise
    //i2cWrite_SENSOR( 0xac, 0xff);    //Automatic De-noise
    i2cWrite_SENSOR( 0x8e, 0x00);    // De-noise strength=0     
    //--Sharpness Strength--//
    i2cWrite_SENSOR( 0x90, 0x0a); //Threshold for edge detection
    i2cWrite_SENSOR( 0x92, 0x08); //Upper limit
    i2cWrite_SENSOR( 0x93, 0x06); //Lower limit

    //---UV adjustment---//
    i2cWrite_SENSOR( 0x13, 0xff);  //banding filter on, allow exposure time tobe less than 1/100 or  1/120.

    i2cWrite_SENSOR(0x90, 0x05);
    i2cWrite_SENSOR(0x91, 0x01);
    i2cWrite_SENSOR(0x92, 0x05);
    i2cWrite_SENSOR(0x93, 0x00);
#if 1
    i2cWrite_SENSOR(0x94, 0x78);
    i2cWrite_SENSOR(0x95, 0x64);
    i2cWrite_SENSOR(0x96, 0x14);
    i2cWrite_SENSOR(0x97, 0x12);
    i2cWrite_SENSOR(0x98, 0x72);
    i2cWrite_SENSOR(0x99, 0x84);
    i2cWrite_SENSOR(0x9a, 0x1e);
    i2cWrite_SENSOR(0x9b, 0x08);
    i2cWrite_SENSOR(0x9c, 0x20);
    i2cWrite_SENSOR(0x9e, 0x00);
    i2cWrite_SENSOR(0x9f, 0x00);
    i2cWrite_SENSOR(0xa6, 0x04);
#elif 0 // 韓國OV原廠參數
    i2cWrite_SENSOR(0x94, 0xb0);
    i2cWrite_SENSOR(0x95, 0x9d);
    i2cWrite_SENSOR(0x96, 0x13);
    i2cWrite_SENSOR(0x97, 0x16);
    i2cWrite_SENSOR(0x98, 0x7b);
    i2cWrite_SENSOR(0x99, 0x91);
    i2cWrite_SENSOR(0x9a, 0x1e);
    i2cWrite_SENSOR(0x9b, 0x38);
    i2cWrite_SENSOR(0x9c, 0x20);
    i2cWrite_SENSOR(0x9e, 0x81);
    i2cWrite_SENSOR(0x9f, 0xfa);
    i2cWrite_SENSOR(0xa6, 0x05);
#endif


    // gamma
#if 1 // VER_120821
    i2cWrite_SENSOR(0x7e, 0x04);    // 0x04
    i2cWrite_SENSOR(0x7f, 0x0e);    // 0x08
    i2cWrite_SENSOR(0x80, 0x20);    // 0x10
    i2cWrite_SENSOR(0x81, 0x43);    // 0x20
    i2cWrite_SENSOR(0x82, 0x53);    // 0x28
    i2cWrite_SENSOR(0x83, 0x61);    // 0x30
    i2cWrite_SENSOR(0x84, 0x6d);    // 0x38
    i2cWrite_SENSOR(0x85, 0x76);    // 0x40
    i2cWrite_SENSOR(0x86, 0x7e);    // 0x48
    i2cWrite_SENSOR(0x87, 0x86);    // 0x50
    i2cWrite_SENSOR(0x88, 0x94);    // 0x60
    i2cWrite_SENSOR(0x89, 0xa1);    // 0x70
    i2cWrite_SENSOR(0x8a, 0xba);    // 0x90
    i2cWrite_SENSOR(0x8b, 0xcf);    // 0xb0
    i2cWrite_SENSOR(0x8c, 0xe3);    // 0xd0
    i2cWrite_SENSOR(0x8d, 0x26);    // SLOP[7:0]=(0x100-GAM15[7:0])x4/3
#elif 0 // 韓國OV原廠參數
    i2cWrite_SENSOR(0x7e, 0x0c);
    i2cWrite_SENSOR(0x7f, 0x16);
    i2cWrite_SENSOR(0x80, 0x2a);
    i2cWrite_SENSOR(0x81, 0x4e);
    i2cWrite_SENSOR(0x82, 0x61);
    i2cWrite_SENSOR(0x83, 0x6f);
    i2cWrite_SENSOR(0x84, 0x7b);
    i2cWrite_SENSOR(0x85, 0x86);
    i2cWrite_SENSOR(0x86, 0x8e);
    i2cWrite_SENSOR(0x87, 0x97);
    i2cWrite_SENSOR(0x88, 0xa4);
    i2cWrite_SENSOR(0x89, 0xaf);
    i2cWrite_SENSOR(0x8a, 0xc5);
    i2cWrite_SENSOR(0x8b, 0xd7);
    i2cWrite_SENSOR(0x8c, 0xe8);
    i2cWrite_SENSOR(0x8d, 0x20);
#elif 0   // default
    i2cWrite_SENSOR(0x7e, 0x0e);    // 0x04
    i2cWrite_SENSOR(0x7f, 0x1a);    // 0x08
    i2cWrite_SENSOR(0x80, 0x31);    // 0x10
    i2cWrite_SENSOR(0x81, 0x5a);    // 0x20
    i2cWrite_SENSOR(0x82, 0x69);    // 0x28
    i2cWrite_SENSOR(0x83, 0x75);    // 0x30
    i2cWrite_SENSOR(0x84, 0x7e);    // 0x38
    i2cWrite_SENSOR(0x85, 0x88);    // 0x40
    i2cWrite_SENSOR(0x86, 0x8f);    // 0x48
    i2cWrite_SENSOR(0x87, 0x96);    // 0x50
    i2cWrite_SENSOR(0x88, 0xa3);    // 0x60
    i2cWrite_SENSOR(0x89, 0xaf);    // 0x70
    i2cWrite_SENSOR(0x8a, 0xc4);    // 0x90
    i2cWrite_SENSOR(0x8b, 0xd7);    // 0xb0
    i2cWrite_SENSOR(0x8c, 0xe8);    // 0xd0
    i2cWrite_SENSOR(0x8d, 0x20);    // SLOP[7:0]=(0x100-GAM15[7:0])x4/3
#else   // default
    i2cWrite_SENSOR(0x7e, 0x0e);    // 0x04
    i2cWrite_SENSOR(0x7f, 0x20);    // 0x08
    i2cWrite_SENSOR(0x80, 0x3d);    // 0x10
    i2cWrite_SENSOR(0x81, 0x70);    // 0x20
    i2cWrite_SENSOR(0x82, 0x82);    // 0x28
    i2cWrite_SENSOR(0x83, 0x98);    // 0x30
    i2cWrite_SENSOR(0x84, 0xa0);    // 0x38
    i2cWrite_SENSOR(0x85, 0xb8);    // 0x40
    i2cWrite_SENSOR(0x86, 0xcf);    // 0x48
    i2cWrite_SENSOR(0x87, 0xd6);    // 0x50
    i2cWrite_SENSOR(0x88, 0xe3);    // 0x60
    i2cWrite_SENSOR(0x89, 0xef);    // 0x70
    i2cWrite_SENSOR(0x8a, 0xf0);    // 0x90
    i2cWrite_SENSOR(0x8b, 0xf4);    // 0xb0
    i2cWrite_SENSOR(0x8c, 0xf8);    // 0xd0
    i2cWrite_SENSOR(0x8d, 0x0a);    // SLOP[7:0]=(0x100-GAM15[7:0])x4/3
#endif 

    // Setup file initial value
    for(i = 0; i < SensorInitLength; i++)
    {
        i2cWrite_SENSOR(SensorInitAddr[i], SensorInitData[i]);
        i2cRead_SENSOR(SensorInitAddr[i], &data);
        DEBUG_SIU("OV7725=0x%02x, 0x%02x\n", SensorInitAddr[i], data & 0xff);
    }
}

#endif  // #if(SIU_OV7725_PARAMETER == SIU_OV7725_SUNNIN)

#if 0
// OV 韓國原廠參數整合原本Mars參數
void SetOV7725_KoreaAndMars(void)
{
	u16 data;
    int i;

    i2cWrite_SENSOR(0x12, 0x0080); //SCCB Register reset
    for(i=0;i<10000;i++);
    i2cWrite_SENSOR( 0x12, 0x0000); //YUV out, VGA resolution
    i2cRead_SENSOR(0x0a, &data);
    DEBUG_SIU("PID_M=0x%x\n",data);
    i2cRead_SENSOR(0x0b, &data);
    DEBUG_SIU("PID_L=0x%x\n",data);

    i2cWrite_SENSOR( 0x67, 0x48); //AEC before gamma, YUV selected.
    i2cWrite_SENSOR( 0x3d, 0x03);  //DC offset compensation for analog process

#if SENSOR_ROW_COL_MIRROR  
    i2cWrite_SENSOR(0x0c,0x00); //rotate 180 degree
#else
    i2cWrite_SENSOR( 0x0c, 0xc0); //rotate 0 degree
#endif
    //----AWB Control ----//
    i2cWrite_SENSOR(0x01, 0x58);
    i2cWrite_SENSOR(0x6a, 0x11);
    i2cWrite_SENSOR(0x6b, 0xaa);      //AWB mode select: simple AWB

    //--- AE Control window ---//      
    i2cWrite_SENSOR( 0x24, 0x0040); //Stable operating Region (Upper limit)
    i2cWrite_SENSOR( 0x25, 0x0030); //Stabel operation Region (Lower limit)
    i2cWrite_SENSOR( 0x26, 0x00b2); //AGC/AEC fast mode operating region 


    //---Set Input window size ---//
    i2cWrite_SENSOR( 0x17, 0x0026);
    i2cWrite_SENSOR( 0x18, 0x00A2);
    i2cWrite_SENSOR( 0x19, 0x0007);
    i2cWrite_SENSOR( 0x1A, 0x00F2);
    //---Set Output window size---//
    i2cWrite_SENSOR( 0x32, 0x0000);  //HOutSize (LSB)
    i2cWrite_SENSOR( 0x29, 0x00A0);  //HOutSize (MSB) =640

    i2cWrite_SENSOR( 0x2C, 0x00F0);  //VOutSize (MSBS)=480
    i2cWrite_SENSOR( 0x2A, 0x0000);  //VOutSize (LSB) 
    //-------PLL Setting--------//

    //i2cWrite_SENSOR( 0x0d, 0x71);    //PLL 4x,AEC Low 2/3 window
    //i2cWrite_SENSOR( 0x0d, 0x61);    //PLL 4x,AEC Low 1/4 window
    //i2cWrite_SENSOR( 0x0d, 0x51);    //PLL 4x,AEC Low 1/2 window
    i2cWrite_SENSOR( 0x0d, 0x41);    //PLL 4x,AEC full window

    i2cWrite_SENSOR( 0x11, 0x01);  // Internal clock= input_clock*4/((1+1)*2)=input_clock = 24MHz

    i2cWrite_SENSOR( 0x4D, 0x09);  //Analog gain fix to 1x

    i2cWrite_SENSOR( 0x63, 0xe0);
    i2cWrite_SENSOR( 0x64, 0xff);
    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 128);    
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x0000); //Insert dummy line. -->7
        i2cWrite_SENSOR(0x34, 0);
    }
    else //50Hz
    {
        i2cWrite_SENSOR(0x22, 153);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 102);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0);
    }

    i2cWrite_SENSOR(0x14, 0x0091);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0090);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0020);  //Max AGC to 8x
    //i2cWrite_SENSOR(0x14, 0x0030);  //Max AGC to 16x

    //---Signal setting---//
    i2cWrite_SENSOR( 0x32, 0x40); 
    i2cWrite_SENSOR( 0x66, 0x80);    //Y0V0 Y1U1 Y2U2 Y3U3                            
    i2cWrite_SENSOR( 0x15, 0x02);    //Vsync negative
    //-- Denoise Strength--//               
    i2cWrite_SENSOR( 0x91, 0x01);

    //--Sharpness Strength--//                 
    i2cWrite_SENSOR(0x90, 0x05);
    i2cWrite_SENSOR(0x91, 0x01);
    i2cWrite_SENSOR(0x92, 0x03);
    i2cWrite_SENSOR(0x93, 0x00);

    //---UV adjustment---//                      
    i2cWrite_SENSOR(0xa7, 0x40);//20111205
    i2cWrite_SENSOR(0xa8, 0x40);//20111206    

    //--------DSP function Config----------//
    i2cWrite_SENSOR( 0x13, 0xff);  //banding filter on, allow exposure time tobe less than 1/100 or  1/120.
    i2cWrite_SENSOR( 0x64, 0xff);  //Saturation enable
    //i2cWrite_SENSOR( 0xac, 0x9f);    //Manual De-noise
    i2cWrite_SENSOR( 0x8f, 0x05);

    //-----Color Matrix-----//
    i2cWrite_SENSOR(0x94, 0xb0);
    i2cWrite_SENSOR(0x95, 0x9d);
    i2cWrite_SENSOR(0x96, 0x13);
    i2cWrite_SENSOR(0x97, 0x16);
    i2cWrite_SENSOR(0x98, 0x7b);
    i2cWrite_SENSOR(0x99, 0x91);
    i2cWrite_SENSOR(0x9a, 0x1e);

    //-----Brightnes and Contrast adjustment---//
    //i2cWrite_SENSOR(0x9b, 0x08);
    i2cWrite_SENSOR(0x9b, 0x38);
    i2cWrite_SENSOR(0x9c, 0x20);
    i2cWrite_SENSOR(0x9e, 0x81);
    i2cWrite_SENSOR(0x9f, 0xfa);
    //---Special Digital Effect Control---//
    i2cWrite_SENSOR(0xa6, 0x05);

    //-----gamma table-----//                           
    i2cWrite_SENSOR(0x7e, 0x0c);
    i2cWrite_SENSOR(0x7f, 0x16);
    i2cWrite_SENSOR(0x80, 0x2a);
    i2cWrite_SENSOR(0x81, 0x4e);
    i2cWrite_SENSOR(0x82, 0x61);
    i2cWrite_SENSOR(0x83, 0x6f);
    i2cWrite_SENSOR(0x84, 0x7b);
    i2cWrite_SENSOR(0x85, 0x86);
    i2cWrite_SENSOR(0x86, 0x8e);
    i2cWrite_SENSOR(0x87, 0x97);
    i2cWrite_SENSOR(0x88, 0xa4);
    i2cWrite_SENSOR(0x89, 0xaf);
    i2cWrite_SENSOR(0x8a, 0xc5);
    i2cWrite_SENSOR(0x8b, 0xd7);
    i2cWrite_SENSOR(0x8c, 0xe8);
    i2cWrite_SENSOR(0x8d, 0x20);
}
#endif

#if 0
// OV 韓國原廠參數
void SetOV7725_Korea(void)
{
    int i;


    i2cWrite_SENSOR(0x12, 0x00);
    i2cWrite_SENSOR(0x00, 0x04);
    i2cWrite_SENSOR(0x01, 0x58);
    i2cWrite_SENSOR(0x02, 0x44);
    i2cWrite_SENSOR(0x03, 0x40);
    i2cWrite_SENSOR(0x04, 0x00);
    i2cWrite_SENSOR(0x05, 0x7a);
    i2cWrite_SENSOR(0x06, 0x79);
    i2cWrite_SENSOR(0x07, 0x78);
    i2cWrite_SENSOR(0x08, 0x00);
    i2cWrite_SENSOR(0x09, 0x00);
    i2cWrite_SENSOR(0x0a, 0x77);
    i2cWrite_SENSOR(0x0b, 0x21);
    i2cWrite_SENSOR(0x0c, 0xC0);
    i2cWrite_SENSOR(0x0d, 0x41);
    i2cWrite_SENSOR(0x0e, 0x79);
    i2cWrite_SENSOR(0x0f, 0xc5);
    i2cWrite_SENSOR(0x10, 0xbd);
    i2cWrite_SENSOR(0x11, 0x01);
    i2cWrite_SENSOR(0x12, 0x00);
    i2cWrite_SENSOR(0x13, 0xff);
    i2cWrite_SENSOR(0x14, 0x91);
    i2cWrite_SENSOR(0x15, 0x00);
    i2cWrite_SENSOR(0x16, 0x00);
    i2cWrite_SENSOR(0x17, 0x22);
    i2cWrite_SENSOR(0x18, 0xa4);
    i2cWrite_SENSOR(0x19, 0x07);
    i2cWrite_SENSOR(0x1a, 0xf0);
    i2cWrite_SENSOR(0x1b, 0x40);
    i2cWrite_SENSOR(0x1c, 0x7f);
    i2cWrite_SENSOR(0x1d, 0xa2);
    i2cWrite_SENSOR(0x1e, 0x00);
    i2cWrite_SENSOR(0x1f, 0x00);
    i2cWrite_SENSOR(0x20, 0x10);
    i2cWrite_SENSOR(0x21, 0x00);
    i2cWrite_SENSOR(0x22, 0x3f);
    i2cWrite_SENSOR(0x23, 0x07);
    i2cWrite_SENSOR(0x24, 0x40);
    i2cWrite_SENSOR(0x25, 0xc0);
    i2cWrite_SENSOR(0x26, 0x81);
    i2cWrite_SENSOR(0x27, 0x00);
    i2cWrite_SENSOR(0x28, 0x00);
    i2cWrite_SENSOR(0x29, 0xa0);
    i2cWrite_SENSOR(0x2a, 0x00);
    i2cWrite_SENSOR(0x2b, 0xc0);
    i2cWrite_SENSOR(0x2c, 0xf0);
    i2cWrite_SENSOR(0x2d, 0x00);
    i2cWrite_SENSOR(0x2e, 0x00);
    i2cWrite_SENSOR(0x2f, 0x47);
    i2cWrite_SENSOR(0x30, 0x88);
    i2cWrite_SENSOR(0x31, 0x78);
    i2cWrite_SENSOR(0x32, 0x00);
    i2cWrite_SENSOR(0x33, 0x00);
    i2cWrite_SENSOR(0x34, 0x00);
    i2cWrite_SENSOR(0x35, 0x7e);
    i2cWrite_SENSOR(0x36, 0x81);
    i2cWrite_SENSOR(0x37, 0x83);
    i2cWrite_SENSOR(0x38, 0x82);
    i2cWrite_SENSOR(0x39, 0x80);
    i2cWrite_SENSOR(0x3a, 0x80);
    i2cWrite_SENSOR(0x3b, 0x80);
    i2cWrite_SENSOR(0x3c, 0x80);
    i2cWrite_SENSOR(0x3d, 0x03);
    i2cWrite_SENSOR(0x3e, 0xe2);
    i2cWrite_SENSOR(0x3f, 0x1f);
    i2cWrite_SENSOR(0x40, 0xe8);
    i2cWrite_SENSOR(0x41, 0x00);
    i2cWrite_SENSOR(0x42, 0x7f);
    i2cWrite_SENSOR(0x43, 0x80);
    i2cWrite_SENSOR(0x44, 0x80);
    i2cWrite_SENSOR(0x45, 0x80);
    i2cWrite_SENSOR(0x46, 0x01);
    i2cWrite_SENSOR(0x47, 0x00);
    i2cWrite_SENSOR(0x48, 0x00);
    i2cWrite_SENSOR(0x49, 0x50);
    i2cWrite_SENSOR(0x4a, 0x30);
    i2cWrite_SENSOR(0x4b, 0x50);
    i2cWrite_SENSOR(0x4c, 0x50);
    i2cWrite_SENSOR(0x4d, 0x09);
    //i2cWrite_SENSOR(0x4e, 0xef);
    i2cWrite_SENSOR(0x4f, 0x10);
    //i2cWrite_SENSOR(0x50, 0x60);
    //i2cWrite_SENSOR(0x51, 0x00);
    //i2cWrite_SENSOR(0x52, 0x00);
    //i2cWrite_SENSOR(0x53, 0x24);
    i2cWrite_SENSOR(0x54, 0x7a);
    //i2cWrite_SENSOR(0x55, 0xfc);
    //i2cWrite_SENSOR(0x56, 0xfc);
    //i2cWrite_SENSOR(0x57, 0xfc);
    i2cWrite_SENSOR(0x58, 0xfc);
    i2cWrite_SENSOR(0x59, 0xfc);
    i2cWrite_SENSOR(0x5a, 0xfc);
    i2cWrite_SENSOR(0x5b, 0xfc);
    i2cWrite_SENSOR(0x5c, 0xfc);
    i2cWrite_SENSOR(0x5d, 0xfc);
    i2cWrite_SENSOR(0x5e, 0xfc);
    i2cWrite_SENSOR(0x5f, 0xfc);
    i2cWrite_SENSOR(0x60, 0x00);
    i2cWrite_SENSOR(0x61, 0x05);
    i2cWrite_SENSOR(0x62, 0xff);
    i2cWrite_SENSOR(0x63, 0xe0);
    i2cWrite_SENSOR(0x64, 0xff);
    i2cWrite_SENSOR(0x65, 0x20);
    i2cWrite_SENSOR(0x66, 0x00);
    i2cWrite_SENSOR(0x67, 0x48);
    i2cWrite_SENSOR(0x68, 0x00);
    i2cWrite_SENSOR(0x69, 0x54);
    i2cWrite_SENSOR(0x6a, 0x11);
    i2cWrite_SENSOR(0x6b, 0xaa);
    i2cWrite_SENSOR(0x6c, 0x01);
    i2cWrite_SENSOR(0x6d, 0x50);
    i2cWrite_SENSOR(0x6e, 0x80);
    i2cWrite_SENSOR(0x6f, 0x80);
    i2cWrite_SENSOR(0x70, 0x0f);
    i2cWrite_SENSOR(0x71, 0x00);
    i2cWrite_SENSOR(0x72, 0x00);
    i2cWrite_SENSOR(0x73, 0x0f);
    i2cWrite_SENSOR(0x74, 0x0f);
    i2cWrite_SENSOR(0x75, 0xff);
    i2cWrite_SENSOR(0x76, 0x00);
    i2cWrite_SENSOR(0x77, 0x10);
    i2cWrite_SENSOR(0x78, 0x10);
    i2cWrite_SENSOR(0x79, 0x70);
    i2cWrite_SENSOR(0x7a, 0x70);
    i2cWrite_SENSOR(0x7b, 0xf0);
    i2cWrite_SENSOR(0x7c, 0xf0);
    i2cWrite_SENSOR(0x7d, 0xf0);
    i2cWrite_SENSOR(0x7e, 0x0c);
    i2cWrite_SENSOR(0x7f, 0x16);
    i2cWrite_SENSOR(0x80, 0x2a);
    i2cWrite_SENSOR(0x81, 0x4e);
    i2cWrite_SENSOR(0x82, 0x61);
    i2cWrite_SENSOR(0x83, 0x6f);
    i2cWrite_SENSOR(0x84, 0x7b);
    i2cWrite_SENSOR(0x85, 0x86);
    i2cWrite_SENSOR(0x86, 0x8e);
    i2cWrite_SENSOR(0x87, 0x97);
    i2cWrite_SENSOR(0x88, 0xa4);
    i2cWrite_SENSOR(0x89, 0xaf);
    i2cWrite_SENSOR(0x8a, 0xc5);
    i2cWrite_SENSOR(0x8b, 0xd7);
    i2cWrite_SENSOR(0x8c, 0xe8);
    i2cWrite_SENSOR(0x8d, 0x20);
    i2cWrite_SENSOR(0x8e, 0x00);
    i2cWrite_SENSOR(0x8f, 0x03);
    i2cWrite_SENSOR(0x90, 0x05);
    i2cWrite_SENSOR(0x91, 0x01);
    i2cWrite_SENSOR(0x92, 0x03);
    i2cWrite_SENSOR(0x93, 0x00);
    i2cWrite_SENSOR(0x94, 0xb0);
    i2cWrite_SENSOR(0x95, 0x9d);
    i2cWrite_SENSOR(0x96, 0x13);
    i2cWrite_SENSOR(0x97, 0x16);
    i2cWrite_SENSOR(0x98, 0x7b);
    i2cWrite_SENSOR(0x99, 0x91);
    i2cWrite_SENSOR(0x9a, 0x1e);
    //i2cWrite_SENSOR(0x9b, 0x08);
    i2cWrite_SENSOR(0x9b, 0x38);
    i2cWrite_SENSOR(0x9c, 0x20);
    i2cWrite_SENSOR(0x9d, 0x00);
    i2cWrite_SENSOR(0x9e, 0x81);
    i2cWrite_SENSOR(0x9f, 0xfa);
    i2cWrite_SENSOR(0xa0, 0x02);
    i2cWrite_SENSOR(0xa1, 0x50);
    i2cWrite_SENSOR(0xa2, 0x40);
    i2cWrite_SENSOR(0xa3, 0x06);
    i2cWrite_SENSOR(0xa4, 0x00);
    i2cWrite_SENSOR(0xa5, 0x00);
    i2cWrite_SENSOR(0xa6, 0x05);
    i2cWrite_SENSOR(0xa7, 0x40);
    i2cWrite_SENSOR(0xa8, 0x40);
    i2cWrite_SENSOR(0xa9, 0x7b);
    i2cWrite_SENSOR(0xaa, 0x21);
    i2cWrite_SENSOR(0xab, 0x05);
    //i2cWrite_SENSOR(0xac, 0x9f);
    i2cWrite_SENSOR(0x8f, 0x05);
    i2cWrite_SENSOR(0x8e, 0x00);
    i2cWrite_SENSOR(0x8f, 0x02);
    i2cWrite_SENSOR(0xa7, 0x60);
    i2cWrite_SENSOR(0xa8, 0x30);
    i2cWrite_SENSOR(0xa9, 0x80);
    i2cWrite_SENSOR(0xaa, 0x00);
    i2cWrite_SENSOR(0xab, 0x06);
    i2cWrite_SENSOR(0xa6, 0x06);
}

#endif

#if 0
void SetOV7725_GCT(void)
{
     
     i2cWrite_SENSOR(0x12, 0x00);
     i2cWrite_SENSOR(0x3d, 0x3);
     i2cWrite_SENSOR(0x17, 0x22);
     i2cWrite_SENSOR(0x18, 0xa4);
     i2cWrite_SENSOR(0x19, 0x7);
     i2cWrite_SENSOR(0x1a, 0xf0);
     i2cWrite_SENSOR(0x32, 0x0);
     i2cWrite_SENSOR(0x29, 0xa0);
     i2cWrite_SENSOR(0x2c, 0xf0);
     i2cWrite_SENSOR(0x2a, 0x0);
     i2cWrite_SENSOR(0x11, 0x1);
     i2cWrite_SENSOR(0x42, 0x7f);
     i2cWrite_SENSOR(0x4d, 0x9);
     i2cWrite_SENSOR(0x63, 0xe0);
     i2cWrite_SENSOR(0x64, 0xff);
     i2cWrite_SENSOR(0x65, 0x20);
     i2cWrite_SENSOR(0x66, 0x80);
     i2cWrite_SENSOR(0x67, 0x48);
     i2cWrite_SENSOR(0x13, 0xf0);
     i2cWrite_SENSOR(0xd, 0x41);
     i2cWrite_SENSOR(0xe, 0xc9);
     i2cWrite_SENSOR(0xf, 0xc5);
     i2cWrite_SENSOR(0x14, 0x40);
     i2cWrite_SENSOR(0x22, 0x7f);
     i2cWrite_SENSOR(0x23, 0x37);
     i2cWrite_SENSOR(0x24, 0x40);
     i2cWrite_SENSOR(0x25, 0x30);
     i2cWrite_SENSOR(0x26, 0xa1);
     i2cWrite_SENSOR(0x2b, 0x0);
     i2cWrite_SENSOR(0x6b, 0xaa);
     i2cWrite_SENSOR(0x13, 0xff);
     i2cWrite_SENSOR(0x90, 0x5);
     i2cWrite_SENSOR(0x91, 0x1);
     i2cWrite_SENSOR(0x92, 0x3);
     i2cWrite_SENSOR(0x93, 0x0);
     i2cWrite_SENSOR(0x94, 0xb0);
     i2cWrite_SENSOR(0x95, 0x9d);
     i2cWrite_SENSOR(0x96, 0x13);
     i2cWrite_SENSOR(0x97, 0x16);
     i2cWrite_SENSOR(0x98, 0x7b);
     i2cWrite_SENSOR(0x99, 0x91);
     i2cWrite_SENSOR(0x9a, 0x1e);
     i2cWrite_SENSOR(0x9b, 0x8);
     i2cWrite_SENSOR(0x9c, 0x20);
     i2cWrite_SENSOR(0x9e, 0x81);
     i2cWrite_SENSOR(0xa6, 0x4);
     i2cWrite_SENSOR(0x7e, 0xc);
     i2cWrite_SENSOR(0x7f, 0x16);
     i2cWrite_SENSOR(0x80, 0x2a);
     i2cWrite_SENSOR(0x81, 0x4e);
     i2cWrite_SENSOR(0x82, 0x61);
     i2cWrite_SENSOR(0x83, 0x6f);
     i2cWrite_SENSOR(0x84, 0x7b);
     i2cWrite_SENSOR(0x85, 0x86);
     i2cWrite_SENSOR(0x86, 0x8e);
     i2cWrite_SENSOR(0x87, 0x97);
     i2cWrite_SENSOR(0x88, 0xa4);
     i2cWrite_SENSOR(0x89, 0xaf);
     i2cWrite_SENSOR(0x8a, 0xc5);
     i2cWrite_SENSOR(0x8b, 0xd7);
     i2cWrite_SENSOR(0x8c, 0xe8);
     i2cWrite_SENSOR(0x8d, 0x20);
     i2cWrite_SENSOR(0x33, 0x66);
     i2cWrite_SENSOR(0x22, 0x98);
     i2cWrite_SENSOR(0x23, 0x4);
     i2cWrite_SENSOR(0x46, 0x5);
     i2cWrite_SENSOR(0x47, 0x10);
     i2cWrite_SENSOR(0x48, 0x0);
     i2cWrite_SENSOR(0x49, 0x8);
     i2cWrite_SENSOR(0x4a, 0x8);
     i2cWrite_SENSOR(0x4b, 0x8);
     i2cWrite_SENSOR(0x4c, 0x9);
     i2cWrite_SENSOR(0xa6, 0x6);
     i2cWrite_SENSOR(0xa7, 0x30);
     i2cWrite_SENSOR(0xa8, 0x30);
     i2cWrite_SENSOR(0x8d, 0x21);
     i2cWrite_SENSOR(0x7e, 0x8);
     i2cWrite_SENSOR(0x7f, 0x12);
     i2cWrite_SENSOR(0x80, 0x23);
     i2cWrite_SENSOR(0x81, 0x43);
     i2cWrite_SENSOR(0x82, 0x59);
     i2cWrite_SENSOR(0x83, 0x64);
     i2cWrite_SENSOR(0x84, 0x72);
     i2cWrite_SENSOR(0x85, 0x7f);
     i2cWrite_SENSOR(0x86, 0x8b);
     i2cWrite_SENSOR(0x87, 0x96);
     i2cWrite_SENSOR(0x88, 0xa7);
     i2cWrite_SENSOR(0x89, 0xb4);
     i2cWrite_SENSOR(0x8a, 0xc7);
     i2cWrite_SENSOR(0x8b, 0xd7);
     i2cWrite_SENSOR(0x8c, 0xe7);
     i2cWrite_SENSOR(0x64, 0xff);
     i2cWrite_SENSOR(0xac, 0xff);
     i2cWrite_SENSOR(0x9f, 0x0);
     i2cWrite_SENSOR(0x9e, 0x23);
     i2cWrite_SENSOR(0x75, 0xff);
     i2cWrite_SENSOR(0x76, 0x0);
     i2cWrite_SENSOR(0x77, 0x3b);
     i2cWrite_SENSOR(0x79, 0x4a);
     i2cWrite_SENSOR(0x78, 0x36);
     i2cWrite_SENSOR(0x7a, 0x24);
     i2cWrite_SENSOR(0x72, 0x4);
     i2cWrite_SENSOR(0x71, 0x0);
     i2cWrite_SENSOR(0x6e, 0x6d);
     i2cWrite_SENSOR(0x6f, 0x36);
     i2cWrite_SENSOR(0x74, 0x36);
     i2cWrite_SENSOR(0x73, 0xe);
     i2cWrite_SENSOR(0x70, 0x9);
     i2cWrite_SENSOR(0x6d, 0x50);
     i2cWrite_SENSOR(0x6c, 0x1);
     i2cWrite_SENSOR(0x6b, 0xaa);
     i2cWrite_SENSOR(0x6a, 0x11);
     i2cWrite_SENSOR(0x69, 0x5c);
     i2cWrite_SENSOR(0xc, 0x00);
     i2cWrite_SENSOR(0x15, 0x02);
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }



}
#endif
// Mars參數
#if 0
void SetOV7725_Mars_1(void)
{
	u16 data;
    int i;


    i2cWrite_SENSOR( 0x12, 0x0000); //YUV out, VGA resolution

    i2cRead_SENSOR(0x0a, &data);
    DEBUG_SIU("PID_M=0x%x\n",data);
    i2cRead_SENSOR(0x0b, &data);
    DEBUG_SIU("PID_L=0x%x\n",data);

    i2cWrite_SENSOR( 0x67, 0x0000); //AEC before gamma, YUV selected.
    i2cWrite_SENSOR( 0x3d, 0x80);  //DC offset compensation for analog process

#if SENSOR_ROW_COL_MIRROR  
    i2cWrite_SENSOR(0x0c,0x00); //rotate 180 degree
#else
    i2cWrite_SENSOR( 0x0c, 0xc0); //rotate 0 degree
#endif
    //----AWB Control ----//
    i2cWrite_SENSOR(0x01, 0xa0);
    i2cWrite_SENSOR(0x6b, 0xaa);      //AWB mode select: simple AWB
     
    //--- AE Control window ---//      
    i2cWrite_SENSOR( 0x24, 0x0040); //Stable operating Region (Upper limit)
    i2cWrite_SENSOR( 0x25, 0x0030); //Stabel operation Region (Lower limit)
    i2cWrite_SENSOR( 0x26, 0x00b2); //AGC/AEC fast mode operating region 


    //---Set Input window size ---//
    i2cWrite_SENSOR( 0x17, 0x0026);
    i2cWrite_SENSOR( 0x18, 0x00A2);
    i2cWrite_SENSOR( 0x19, 0x0007);
    i2cWrite_SENSOR( 0x1A, 0x00F2);

    //---Set Output window size---//
    i2cWrite_SENSOR( 0x32, 0x0000);  //HOutSize (LSB)
    i2cWrite_SENSOR( 0x29, 0x00A0);  //HOutSize (MSB) =640

    i2cWrite_SENSOR( 0x2C, 0x00F0);  //VOutSize (MSBS)=480
    i2cWrite_SENSOR( 0x2A, 0x0000);  //VOutSize (LSB) 
    //-------PLL Setting--------//

    //i2cWrite_SENSOR( 0x0d, 0x71);    //PLL 4x,AEC Low 2/3 window
    //i2cWrite_SENSOR( 0x0d, 0x61);    //PLL 4x,AEC Low 1/4 window
    //i2cWrite_SENSOR( 0x0d, 0x51);    //PLL 4x,AEC Low 1/2 window
    i2cWrite_SENSOR( 0x0d, 0x41);    //PLL 4x,AEC full window

    i2cWrite_SENSOR( 0x11, 0x0001);  // Internal clock= input_clock*4/((1+1)*2)=input_clock = 24MHz

    i2cWrite_SENSOR( 0x4D, 0x0000);  //Analog gain fix to 1x

    i2cWrite_SENSOR( 0x63, 0x00F0);
    i2cWrite_SENSOR( 0x64, 0x001F);

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 128);    
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x0000); //Insert dummy line. -->7
        i2cWrite_SENSOR(0x34, 0);
    }
    else //50Hz
    {
        i2cWrite_SENSOR(0x22, 153);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 3);      // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 102);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0);
    }

    i2cWrite_SENSOR(0x14, 0x0010);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0090);  //Max AGC to 4x
    //i2cWrite_SENSOR(0x14, 0x0020);  //Max AGC to 8x

    //---Signal setting---//
    i2cWrite_SENSOR( 0x32, 0x40); 
    i2cWrite_SENSOR( 0x66, 0x80);    //Y0V0 Y1U1 Y2U2 Y3U3                            
    i2cWrite_SENSOR( 0x15, 0x02);    //Vsync negative

    //-- Denoise Strength--//               
    i2cWrite_SENSOR( 0x91, 0x01);

    //--Sharpness Strength--//                 
    i2cWrite_SENSOR(0x90, 0x05);
    i2cWrite_SENSOR(0x91, 0x01);
    i2cWrite_SENSOR(0x92, 0x05);
    i2cWrite_SENSOR(0x93, 0x00);

    //---UV adjustment---//                      
    i2cWrite_SENSOR(0xa7, 0x56);//20111205
    i2cWrite_SENSOR(0xa8, 0x4f);//20111206    

    //--------DSP function Config----------//
    i2cWrite_SENSOR( 0x13, 0xff);  //banding filter on, allow exposure time tobe less than 1/100 or  1/120.
    i2cWrite_SENSOR( 0x64, 0xBF);  //Saturation enable
    i2cWrite_SENSOR( 0xac, 0xbf);    //Manual De-noise



    //-----Color Matrix-----//
    i2cWrite_SENSOR(0x94, 0x78);
    i2cWrite_SENSOR(0x95, 0x64);
    i2cWrite_SENSOR(0x96, 0x14);
    i2cWrite_SENSOR(0x97, 0x12);
    i2cWrite_SENSOR(0x98, 0x72);
    i2cWrite_SENSOR(0x99, 0x84);
    i2cWrite_SENSOR(0x9a, 0x1e);

    //-----Brightnes and Contrast adjustment---//
    i2cWrite_SENSOR(0x9b, 0x08);
    i2cWrite_SENSOR(0x9c, 0x20);
    i2cWrite_SENSOR(0x9e, 0x00);
    i2cWrite_SENSOR(0x9f, 0x00);

    //---Special Digital Effect Control---//
    i2cWrite_SENSOR(0xa6, 0x04);

    //-----gamma table-----//
#if 0
    i2cWrite_SENSOR(0x7e, 0x0c);
    i2cWrite_SENSOR(0x7f, 0x16);
    i2cWrite_SENSOR(0x80, 0x2a);
    i2cWrite_SENSOR(0x81, 0x4e);
    i2cWrite_SENSOR(0x82, 0x61);
    i2cWrite_SENSOR(0x83, 0x6f);
    i2cWrite_SENSOR(0x84, 0x7b);
    i2cWrite_SENSOR(0x85, 0x86);
    i2cWrite_SENSOR(0x86, 0x8e);
    i2cWrite_SENSOR(0x87, 0x97);
    i2cWrite_SENSOR(0x88, 0xa4);
    i2cWrite_SENSOR(0x89, 0xaf);
    i2cWrite_SENSOR(0x8a, 0xc5);
    i2cWrite_SENSOR(0x8b, 0xd7);
    i2cWrite_SENSOR(0x8c, 0xe8);
    i2cWrite_SENSOR(0x8d, 0x20);
#else
    i2cWrite_SENSOR(0x7e, 0x04);
    i2cWrite_SENSOR(0x7f, 0x0e);
    i2cWrite_SENSOR(0x80, 0x20);
    i2cWrite_SENSOR(0x81, 0x43);
    i2cWrite_SENSOR(0x82, 0x53);
    i2cWrite_SENSOR(0x83, 0x61);
    i2cWrite_SENSOR(0x84, 0x6d);
    i2cWrite_SENSOR(0x85, 0x76);
    i2cWrite_SENSOR(0x86, 0x7e);
    i2cWrite_SENSOR(0x87, 0x86);
    i2cWrite_SENSOR(0x88, 0x94);
    i2cWrite_SENSOR(0x89, 0xa1);
    i2cWrite_SENSOR(0x8a, 0xba);
    i2cWrite_SENSOR(0x8b, 0xcf);
    i2cWrite_SENSOR(0x8c, 0xe3);
    i2cWrite_SENSOR(0x8d, 0x26);
#endif 
}
#endif




#if (SIU_OV7725_PARAMETER == SIU_OV7725_AIO)
void SetOV7725_Init(void)
{
     i2cWrite_SENSOR(0x12, 0x0);
     i2cWrite_SENSOR(0x3d, 0x3);
     i2cWrite_SENSOR(0x17, 0x22);
     i2cWrite_SENSOR(0x18, 0xa4);
     i2cWrite_SENSOR(0x19, 0x7);
     i2cWrite_SENSOR(0x1a, 0xf0);
     i2cWrite_SENSOR(0x32, 0x0);
     i2cWrite_SENSOR(0x29, 0xa0);
     i2cWrite_SENSOR(0x2c, 0xf0);
     i2cWrite_SENSOR(0x42, 0x7f);
     i2cWrite_SENSOR(0x60, 0x00);
     i2cWrite_SENSOR(0x61, 0x00);
     i2cWrite_SENSOR(0x63, 0xe0);
     i2cWrite_SENSOR(0x65, 0x20);
     i2cWrite_SENSOR(0x67, 0x48);
     i2cWrite_SENSOR(0x0f, 0xc5);
     i2cWrite_SENSOR(0x14, 0x31);
     i2cWrite_SENSOR(0x24, 0x40);
     i2cWrite_SENSOR(0x25, 0x38);
     i2cWrite_SENSOR(0x26, 0x91);
     i2cWrite_SENSOR(0x91, 0x01);
     i2cWrite_SENSOR(0x94, 0xb0);
     i2cWrite_SENSOR(0x95, 0x9d);
     i2cWrite_SENSOR(0x96, 0x13);
     i2cWrite_SENSOR(0x97, 0x16);
     i2cWrite_SENSOR(0x98, 0x7b);
     i2cWrite_SENSOR(0x99, 0x91);
     i2cWrite_SENSOR(0x9a, 0x1e);
     
     i2cWrite_SENSOR(0x9b, 0x8);
     i2cWrite_SENSOR(0x9c, 0x20);
     i2cWrite_SENSOR(0x9e, 0x81);
     i2cWrite_SENSOR(0x13, 0xff);
     i2cWrite_SENSOR(0x66, 0x80);
     i2cWrite_SENSOR(0x15, 0x2);
     #if(SENSOR_ROW_COL_MIRROR)
     i2cWrite_SENSOR(0x0c, 0x00);   
     #else
     i2cWrite_SENSOR(0x0c, 0xc0);   //mirror image
     #endif
     i2cWrite_SENSOR(0x2d, 0x0);
     i2cWrite_SENSOR(0x2e, 0x0);
     i2cWrite_SENSOR(0x11, 0x1);
     i2cWrite_SENSOR(0x0d, 0x41);
     i2cWrite_SENSOR(0x2a, 0x0);
     i2cWrite_SENSOR(0x2b, 0x0);
     i2cWrite_SENSOR(0x0e, 0xed);
     i2cWrite_SENSOR(0x47, 0x10);
     i2cWrite_SENSOR(0x48, 0x10);
     i2cWrite_SENSOR(0x4a, 0x8);
     i2cWrite_SENSOR(0x49, 0x8);
     i2cWrite_SENSOR(0x4b, 0x8);
     i2cWrite_SENSOR(0x4c, 0x8);
     i2cWrite_SENSOR(0x46, 0x5);
     i2cWrite_SENSOR(0xa7, SENSOR_U_SATURATION_INDOOR);
     i2cWrite_SENSOR(0xa8, SENSOR_V_SATURATION_INDOOR);
     i2cWrite_SENSOR(0xa6, 0x2);
     i2cWrite_SENSOR(0x7e, 0xa);
     i2cWrite_SENSOR(0x7f, 0x14);
     i2cWrite_SENSOR(0x80, 0x28);
     i2cWrite_SENSOR(0x81, 0x4c);
     i2cWrite_SENSOR(0x82, 0x5c);
     i2cWrite_SENSOR(0x83, 0x6a);
     i2cWrite_SENSOR(0x84, 0x77);
     i2cWrite_SENSOR(0x85, 0x82);
     i2cWrite_SENSOR(0x86, 0x8b);
     i2cWrite_SENSOR(0x87, 0x93);
     i2cWrite_SENSOR(0x88, 0xa1);
     i2cWrite_SENSOR(0x89, 0xac);
     i2cWrite_SENSOR(0x8a, 0xc2);
     i2cWrite_SENSOR(0x8b, 0xd7);
     i2cWrite_SENSOR(0x8c, 0xe9);
     i2cWrite_SENSOR(0x8d, 0x1e);
     i2cWrite_SENSOR(0x75, 0xf0);
     i2cWrite_SENSOR(0x76, 0x10);
     i2cWrite_SENSOR(0x77, 0x28);
     i2cWrite_SENSOR(0x79, 0x4e);
     i2cWrite_SENSOR(0x78, 0x1e);
     i2cWrite_SENSOR(0x7a, 0x2d);
     i2cWrite_SENSOR(0x72, 0x98);
     i2cWrite_SENSOR(0x71, 0x9c);
     i2cWrite_SENSOR(0x6e, 0x6a);
     i2cWrite_SENSOR(0x6f, 0x5b);
     i2cWrite_SENSOR(0x74, 0x36);
     i2cWrite_SENSOR(0x73, 0x10);
     i2cWrite_SENSOR(0x70, 0xe);
     i2cWrite_SENSOR(0x6d, 0x50);
     i2cWrite_SENSOR(0x6c, 0x11);
     i2cWrite_SENSOR(0x6b, 0x15);
     i2cWrite_SENSOR(0x6a, 0x0);
     i2cWrite_SENSOR(0x69, 0x96);
     i2cWrite_SENSOR(0x4d, 0x9);
     i2cWrite_SENSOR(0x64, 0xbf);
     i2cWrite_SENSOR(0xac, 0x20);
     i2cWrite_SENSOR(0x90, 0x2);
     i2cWrite_SENSOR(0x92, 0x4);
     i2cWrite_SENSOR(0x93, 0x0);


    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }


}

#elif (SIU_OV7725_PARAMETER == SIU_OV7725_RDI)
void SetOV7725_Init(void)
{
    int i;

// 4x maximum gain

    i2cWrite_SENSOR(0x12, 0x00);

    i2cWrite_SENSOR(0x3d, 0x03);    //Gamma
    i2cWrite_SENSOR(0x17, 0x22);  
    i2cWrite_SENSOR(0x18, 0xa4);  
    i2cWrite_SENSOR(0x19, 0x07);  
    i2cWrite_SENSOR(0x1a, 0xf0);  
    i2cWrite_SENSOR(0x32, 0x00);  
    i2cWrite_SENSOR(0x29, 0xa0);  
    i2cWrite_SENSOR(0x2c, 0xf0);  
    i2cWrite_SENSOR(0x2a, 0x00);  
    i2cWrite_SENSOR(0x11, 0x01);  
    i2cWrite_SENSOR(0x42, 0x7f);  
    i2cWrite_SENSOR(0x4d, 0x09);  
    i2cWrite_SENSOR(0x63, 0xe0);  
 
    i2cWrite_SENSOR(0x65, 0x20);  
    i2cWrite_SENSOR(0x66, 0x00);  
    i2cWrite_SENSOR(0x67, 0x48);  
    i2cWrite_SENSOR(0x13, 0xf0);  
    i2cWrite_SENSOR(0x0d, 0x41); 
//0x51/0x61/0x71 for 
//different AEC/AGC window
    i2cWrite_SENSOR(0x0f, 0xc5); 
    i2cWrite_SENSOR(0x14, 0x11);  
    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  

    i2cWrite_SENSOR(0x2b, 0x00);  
  
    i2cWrite_SENSOR(0x13, 0xff);  

    i2cWrite_SENSOR(0x91, 0x01);  

    i2cWrite_SENSOR(0x94, 0xb0);  
    i2cWrite_SENSOR(0x95, 0x9d);  
    i2cWrite_SENSOR(0x96, 0x13);  
    i2cWrite_SENSOR(0x97, 0x16);  
    i2cWrite_SENSOR(0x98, 0x7b);  
    i2cWrite_SENSOR(0x99, 0x91);  
    i2cWrite_SENSOR(0x9a, 0x1e);  
    i2cWrite_SENSOR(0x9b, 0x08);  
    i2cWrite_SENSOR(0x9c, 0x20);  
    i2cWrite_SENSOR(0x9e, 0x81);  
    i2cWrite_SENSOR(0xa6, 0x06);  
    i2cWrite_SENSOR(0x7e, 0x0c);    //Gamma
    i2cWrite_SENSOR(0x7f, 0x16);  
    i2cWrite_SENSOR(0x80, 0x2a);  
    i2cWrite_SENSOR(0x81, 0x4e);  
    i2cWrite_SENSOR(0x82, 0x61);  
    i2cWrite_SENSOR(0x83, 0x6f);  
    i2cWrite_SENSOR(0x84, 0x7b);  
    i2cWrite_SENSOR(0x85, 0x86);  
    i2cWrite_SENSOR(0x86, 0x8e);  
    i2cWrite_SENSOR(0x87, 0x97);  
    i2cWrite_SENSOR(0x88, 0xa4);  
    i2cWrite_SENSOR(0x89, 0xaf);  
    i2cWrite_SENSOR(0x8a, 0xc5);  
    i2cWrite_SENSOR(0x8b, 0xd7);  
    i2cWrite_SENSOR(0x8c, 0xe8);  
    i2cWrite_SENSOR(0x8d, 0x20);  
    i2cWrite_SENSOR(0x33, 0x00);    //for 30 fps,60Hz
    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  
    i2cWrite_SENSOR(0x33, 0x66);    //for 25 fps,50Hz
    i2cWrite_SENSOR(0x22, 0x99);  
    i2cWrite_SENSOR(0x23, 0x03);                        

//Lens Correction,should be tuned 
//with real camera module            
    i2cWrite_SENSOR(0x4a, 0x10); 
    i2cWrite_SENSOR(0x49, 0x10);  
    i2cWrite_SENSOR(0x4b, 0x14);  
    i2cWrite_SENSOR(0x4c, 0x17);  
    i2cWrite_SENSOR(0x46, 0x05);  
    i2cWrite_SENSOR(0x0e, 0x65); 

//;;;;;;;;;;;;;;;
//start modify by mike for client

    i2cWrite_SENSOR(0x13, 0xff);

    i2cWrite_SENSOR(0x66, 0x80);
    i2cWrite_SENSOR(0x15, 0x02);
    i2cWrite_SENSOR(0x0c, 0xc0);
    i2cWrite_SENSOR(0x2d, 0x00);
    i2cWrite_SENSOR(0x2e, 0x00);


    i2cWrite_SENSOR(0x11, 0x01);    //25fps
    i2cWrite_SENSOR(0x0d, 0x41);
    i2cWrite_SENSOR(0x33, 0xb3);
    i2cWrite_SENSOR(0x34, 0x00);
    i2cWrite_SENSOR(0x2a, 0x00);
    i2cWrite_SENSOR(0x2b, 0x00);
    i2cWrite_SENSOR(0x22, 0xab);
    i2cWrite_SENSOR(0x23, 0x03);

    i2cWrite_SENSOR(0x0e, 0x65);
    // Lens Correction  
    i2cWrite_SENSOR(0x47, 0x10);             //x   
    i2cWrite_SENSOR(0x48, 0x10);    //14       ;y         
    i2cWrite_SENSOR(0x4a, 0x08);    //10   ;r
    i2cWrite_SENSOR(0x49, 0x08);    //0c   ;8;10     ;G
    i2cWrite_SENSOR(0x4b, 0x08);    //0c   ;10;8;14  ;B
    i2cWrite_SENSOR(0x4c, 0x08);    //0c   ;8;10;17  ;R
    i2cWrite_SENSOR(0x46, 0x05); 
                              
    //@@ 0.75x

    i2cWrite_SENSOR(0xa7, SENSOR_U_SATURATION_INDOOR);    //30;40
    i2cWrite_SENSOR(0xa8, SENSOR_V_SATURATION_INDOOR);    //30;40

    i2cWrite_SENSOR(0xa6, 0x02);
                              
    i2cWrite_SENSOR(0x7e, 0x0A);    //06   ; 0A
    i2cWrite_SENSOR(0x7f, 0x14);    //0C   ; 14
    i2cWrite_SENSOR(0x80, 0x28);    // 1A   ; 28
    i2cWrite_SENSOR(0x81, 0x4C);    //46   ; 4C
    i2cWrite_SENSOR(0x82, 0x5C);    //5E   ; 5C
    i2cWrite_SENSOR(0x83, 0x6A);    //74   ; 6A
    i2cWrite_SENSOR(0x84, 0x77);    //86   ; 77
    i2cWrite_SENSOR(0x85, 0x82);    //95   ; 82
    i2cWrite_SENSOR(0x86, 0x8B);    //A1   ; 8B
    i2cWrite_SENSOR(0x87, 0x93);    //AB   ; 93
    i2cWrite_SENSOR(0x88, 0xA1);    //BC   ; A1
    i2cWrite_SENSOR(0x89, 0xAC);    //C9   ; AC
    i2cWrite_SENSOR(0x8a, 0xC2);    //DD   ; C2
    i2cWrite_SENSOR(0x8b, 0xD7);    //EC   ; D7
    i2cWrite_SENSOR(0x8c, 0xE9);    //F5   ; E9
    i2cWrite_SENSOR(0x8d, 0x1E);    //0E   ; 1E
                              
    i2cWrite_SENSOR(0x24, 0x38);
    i2cWrite_SENSOR(0x25, 0x30);
    i2cWrite_SENSOR(0x26, 0x93);
                              
//awb-ver1.04-0321-1100        
    i2cWrite_SENSOR(0x75, 0xf0); 
    i2cWrite_SENSOR(0x76, 0x10);
    i2cWrite_SENSOR(0x77, 0x28);
    i2cWrite_SENSOR(0x79, 0x4e);
    i2cWrite_SENSOR(0x78, 0x1e);
    i2cWrite_SENSOR(0x7a, 0x2d);
    i2cWrite_SENSOR(0x72, 0x98);
    i2cWrite_SENSOR(0x71, 0x9c);
    i2cWrite_SENSOR(0x6e, 0x6a);
    i2cWrite_SENSOR(0x6f, 0x5b);
    i2cWrite_SENSOR(0x74, 0x36);
    i2cWrite_SENSOR(0x73, 0x10);
    i2cWrite_SENSOR(0x70, 0x0e); 
    i2cWrite_SENSOR(0x6d, 0x50);
    i2cWrite_SENSOR(0x6c, 0x11);
    i2cWrite_SENSOR(0x6b, 0x15);
    i2cWrite_SENSOR(0x6a, 0x00); 
    i2cWrite_SENSOR(0x69, 0x96);
                              
//42 4d 0d                     
                                                     
    i2cWrite_SENSOR(0x64, 0xbf);    //disable UVadjust
                              
    i2cWrite_SENSOR(0xac, 0x20);    //auto mode
    i2cWrite_SENSOR(0x90, 0x02);    //05      ;05->03 reduce denoise 
    i2cWrite_SENSOR(0x92, 0x04);    //03      ;03->05 enhance sharpness
    i2cWrite_SENSOR(0x93, 0x00);
                              
//end modify by mike      
                                             
                              
//init over                   

    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else //50Hz
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }

//AEC/night mode               
    //i2cWrite_SENSOR(0x0e, 0xed);
    //i2cWrite_SENSOR(0x14, 0x31);

}

#elif (SIU_OV7725_PARAMETER == SIU_OV7725_MAYON)
void SetOV7725_Init (void)
{

    i2cWrite_SENSOR(0x12, 0x00);

    i2cWrite_SENSOR(0x3d, 0x03);    //Gamma
    i2cWrite_SENSOR(0x17, 0x22);  
    i2cWrite_SENSOR(0x18, 0xa4);  
    i2cWrite_SENSOR(0x19, 0x07);  
    i2cWrite_SENSOR(0x1a, 0xf0);  
    i2cWrite_SENSOR(0x32, 0x00);  
    i2cWrite_SENSOR(0x29, 0xa0);  
    i2cWrite_SENSOR(0x2c, 0xf0);  
    i2cWrite_SENSOR(0x2a, 0x00);  
    i2cWrite_SENSOR(0x11, 0x01);  
    i2cWrite_SENSOR(0x42, 0x7f);  
    i2cWrite_SENSOR(0x4d, 0x09);  
    i2cWrite_SENSOR(0x63, 0xe0);  
 
    i2cWrite_SENSOR(0x65, 0x20);  
    i2cWrite_SENSOR(0x66, 0x00);  
    i2cWrite_SENSOR(0x67, 0x48);  
    i2cWrite_SENSOR(0x13, 0xff);  
    i2cWrite_SENSOR(0x0d, 0x41); 
//0x51/0x61/0x71 for 
//different AEC/AGC window
    i2cWrite_SENSOR(0x0f, 0xc5); 

    i2cWrite_SENSOR(0x14, 0x31);  

    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  

    i2cWrite_SENSOR(0x2b, 0x00);  
  

    i2cWrite_SENSOR(0x91, 0x01);  

    i2cWrite_SENSOR(0x94, 0x5f);  
    i2cWrite_SENSOR(0x95, 0x53);  
    i2cWrite_SENSOR(0x96, 0x11);  
    i2cWrite_SENSOR(0x97, 0x1a);  
    i2cWrite_SENSOR(0x98, 0x3d);  
    i2cWrite_SENSOR(0x99, 0x5a);  
    i2cWrite_SENSOR(0x9a, 0x1e);  
    i2cWrite_SENSOR(0x9b, 0x01);  
    i2cWrite_SENSOR(0x9c, 0x25);  
    i2cWrite_SENSOR(0x9e, 0x81);  
    i2cWrite_SENSOR(0xa6, 0x06);  
    i2cWrite_SENSOR(0x7e, 0x0c);    //Gamma
    i2cWrite_SENSOR(0x7f, 0x16);  
    i2cWrite_SENSOR(0x80, 0x2a);  
    i2cWrite_SENSOR(0x81, 0x4e);  
    i2cWrite_SENSOR(0x82, 0x61);  
    i2cWrite_SENSOR(0x83, 0x6f);  
    i2cWrite_SENSOR(0x84, 0x7b);  
    i2cWrite_SENSOR(0x85, 0x86);  
    i2cWrite_SENSOR(0x86, 0x8e);  
    i2cWrite_SENSOR(0x87, 0x97);  
    i2cWrite_SENSOR(0x88, 0xa4);  
    i2cWrite_SENSOR(0x89, 0xaf);  
    i2cWrite_SENSOR(0x8a, 0xc5);  
    i2cWrite_SENSOR(0x8b, 0xd7);  
    i2cWrite_SENSOR(0x8c, 0xe8);  
    i2cWrite_SENSOR(0x8d, 0x20);  
    i2cWrite_SENSOR(0x33, 0x00);    //for 30 fps,60Hz
    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);                         

//Lens Correction,should be tuned 
//with real camera module
	    i2cWrite_SENSOR(0x12, 0x00);

    i2cWrite_SENSOR(0x3d, 0x03);    //Gamma
    i2cWrite_SENSOR(0x17, 0x22);  
    i2cWrite_SENSOR(0x18, 0xa4);  
    i2cWrite_SENSOR(0x19, 0x07);  
    i2cWrite_SENSOR(0x1a, 0xf0);  
    i2cWrite_SENSOR(0x32, 0x00);  
    i2cWrite_SENSOR(0x29, 0xa0);  
    i2cWrite_SENSOR(0x2c, 0xf0);  
    i2cWrite_SENSOR(0x2a, 0x00);  
    i2cWrite_SENSOR(0x11, 0x01);  
    i2cWrite_SENSOR(0x42, 0x7f);  
    i2cWrite_SENSOR(0x4d, 0x09);  
    i2cWrite_SENSOR(0x63, 0xe0);  
 
    i2cWrite_SENSOR(0x65, 0x20);  
    i2cWrite_SENSOR(0x66, 0x00);  
    i2cWrite_SENSOR(0x67, 0x48);  
    i2cWrite_SENSOR(0x13, 0xff);  
    i2cWrite_SENSOR(0x0d, 0x41); 
//0x51/0x61/0x71 for 
//different AEC/AGC window
    i2cWrite_SENSOR(0x0f, 0xc5); 

    i2cWrite_SENSOR(0x14, 0x31);  

    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  

    i2cWrite_SENSOR(0x2b, 0x00);  
  

    i2cWrite_SENSOR(0x91, 0x01);  

    i2cWrite_SENSOR(0x94, 0x5f);  
    i2cWrite_SENSOR(0x95, 0x53);  
    i2cWrite_SENSOR(0x96, 0x11);  
    i2cWrite_SENSOR(0x97, 0x1a);  
    i2cWrite_SENSOR(0x98, 0x3d);  
    i2cWrite_SENSOR(0x99, 0x5a);  
    i2cWrite_SENSOR(0x9a, 0x1e);  
    i2cWrite_SENSOR(0x9b, 0x01);  
    i2cWrite_SENSOR(0x9c, 0x25);  
    i2cWrite_SENSOR(0x9e, 0x81);  
    i2cWrite_SENSOR(0xa6, 0x06);  
    i2cWrite_SENSOR(0x7e, 0x0c);    //Gamma
    i2cWrite_SENSOR(0x7f, 0x16);  
    i2cWrite_SENSOR(0x80, 0x2a);  
    i2cWrite_SENSOR(0x81, 0x4e);  
    i2cWrite_SENSOR(0x82, 0x61);  
    i2cWrite_SENSOR(0x83, 0x6f);  
    i2cWrite_SENSOR(0x84, 0x7b);  
    i2cWrite_SENSOR(0x85, 0x86);  
    i2cWrite_SENSOR(0x86, 0x8e);  
    i2cWrite_SENSOR(0x87, 0x97);  
    i2cWrite_SENSOR(0x88, 0xa4);  
    i2cWrite_SENSOR(0x89, 0xaf);  
    i2cWrite_SENSOR(0x8a, 0xc5);  
    i2cWrite_SENSOR(0x8b, 0xd7);  
    i2cWrite_SENSOR(0x8c, 0xe8);  
    i2cWrite_SENSOR(0x8d, 0x20);  
    i2cWrite_SENSOR(0x33, 0x00);    //for 30 fps,60Hz
    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);                         

//Lens Correction,should be tuned 
//with real camera module            
    i2cWrite_SENSOR(0x4a, 0x30); 
    i2cWrite_SENSOR(0x49, 0x50);  
    i2cWrite_SENSOR(0x4b, 0x50);  
    i2cWrite_SENSOR(0x4c, 0x50);  
    i2cWrite_SENSOR(0x46, 0x00);  
    i2cWrite_SENSOR(0x0e, 0xf5); 

//;;;;;;;;;;;;;;;
//start modify by mike for client

    i2cWrite_SENSOR(0x66, 0x80);
    i2cWrite_SENSOR(0x15, 0x02);

    i2cWrite_SENSOR(0x0c, 0xc0);

    i2cWrite_SENSOR(0x2d, 0x00);
    i2cWrite_SENSOR(0x2e, 0x00);


    i2cWrite_SENSOR(0x11, 0x01);    //25fps
    i2cWrite_SENSOR(0x0d, 0x41);
    i2cWrite_SENSOR(0x33, 0xb3);
    i2cWrite_SENSOR(0x34, 0x00);
    i2cWrite_SENSOR(0x2a, 0x00);
    i2cWrite_SENSOR(0x2b, 0x00);
    i2cWrite_SENSOR(0x22, 0xab);
    i2cWrite_SENSOR(0x23, 0x03);

    i2cWrite_SENSOR(0x0e, 0x65);
                         

    i2cWrite_SENSOR(0xa7, 0x65);//30;40
    i2cWrite_SENSOR(0xa8, 0x65);//30;40

    i2cWrite_SENSOR(0xa6, 0x06);
                              
    i2cWrite_SENSOR(0x7e, 0x0c);    //06   ; 0A
    i2cWrite_SENSOR(0x7f, 0x16);    //0C   ; 14
    i2cWrite_SENSOR(0x80, 0x2a);    // 1A   ; 28
    i2cWrite_SENSOR(0x81, 0x4e);    //46   ; 4C
    i2cWrite_SENSOR(0x82, 0x61);    //5E   ; 5C
    i2cWrite_SENSOR(0x83, 0x6f);    //74   ; 6A
    i2cWrite_SENSOR(0x84, 0x7b);    //86   ; 77
    i2cWrite_SENSOR(0x85, 0x86);    //95   ; 82
    i2cWrite_SENSOR(0x86, 0x8e);    //A1   ; 8B
    i2cWrite_SENSOR(0x87, 0x97);    //AB   ; 93
    i2cWrite_SENSOR(0x88, 0xa4);    //BC   ; A1
    i2cWrite_SENSOR(0x89, 0xAf);    //C9   ; AC
    i2cWrite_SENSOR(0x8a, 0xC5);    //DD   ; C2
    i2cWrite_SENSOR(0x8b, 0xD7);    //EC   ; D7
    i2cWrite_SENSOR(0x8c, 0xE8);    //F5   ; E9
    i2cWrite_SENSOR(0x8d, 0x20);    //0E   ; 1E
                              
    i2cWrite_SENSOR(0x24, 0x50);
    i2cWrite_SENSOR(0x25, 0x30);
    i2cWrite_SENSOR(0x26, 0xa1);
                              
//awb-ver1.04-0321-1100        
    i2cWrite_SENSOR(0x75, 0xff); 
    i2cWrite_SENSOR(0x76, 0x00);
    i2cWrite_SENSOR(0x77, 0x10);
    i2cWrite_SENSOR(0x79, 0x70);
    i2cWrite_SENSOR(0x78, 0x10);
    i2cWrite_SENSOR(0x7a, 0x70);
    i2cWrite_SENSOR(0x72, 0x00);
    i2cWrite_SENSOR(0x71, 0x00);
    i2cWrite_SENSOR(0x6e, 0x80);
    i2cWrite_SENSOR(0x6f, 0x80);
    i2cWrite_SENSOR(0x74, 0x0f);
    i2cWrite_SENSOR(0x73, 0x0f);
    i2cWrite_SENSOR(0x70, 0x0f); 
    i2cWrite_SENSOR(0x6d, 0x50);
    i2cWrite_SENSOR(0x6c, 0x01);
    i2cWrite_SENSOR(0x6b, 0xaa);
    i2cWrite_SENSOR(0x6a, 0x11); 
    i2cWrite_SENSOR(0x69, 0x5c);
                              
//42 4d 0d                     
                                                     
    i2cWrite_SENSOR(0x64, 0xff);    //disable UVadjust
                              
    i2cWrite_SENSOR(0xac, 0xff);    //auto mode
    i2cWrite_SENSOR(0x90, 0x05);    //05      ;05->03 reduce denoise 
    i2cWrite_SENSOR(0x92, 0x03);    //03      ;03->05 enhance sharpness
    i2cWrite_SENSOR(0x93, 0x00);            
    i2cWrite_SENSOR(0x4a, 0x30); 
    i2cWrite_SENSOR(0x49, 0x50);  
    i2cWrite_SENSOR(0x4b, 0x50);  
    i2cWrite_SENSOR(0x4c, 0x50);  
    i2cWrite_SENSOR(0x46, 0x00);  
    i2cWrite_SENSOR(0x0e, 0xf5); 

//;;;;;;;;;;;;;;;

                              

    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }
}

#elif(SIU_OV7725_PARAMETER == SIU_OV7725_MARS2)
void SetOV7725_Init(void)
{
    int i;

   // 4x maximum gain

    i2cWrite_SENSOR(0x12, 0x00);

    i2cWrite_SENSOR(0x3d, 0x03);    //Gamma
    i2cWrite_SENSOR(0x17, 0x22);  
    i2cWrite_SENSOR(0x18, 0xa4);  
    i2cWrite_SENSOR(0x19, 0x07);  
    i2cWrite_SENSOR(0x1a, 0xf0);  
    i2cWrite_SENSOR(0x32, 0x00);  
    i2cWrite_SENSOR(0x29, 0xa0);  
    i2cWrite_SENSOR(0x2c, 0xf0);  
    i2cWrite_SENSOR(0x2a, 0x00);  
    i2cWrite_SENSOR(0x11, 0x01);  
    i2cWrite_SENSOR(0x42, 0x7f);  
    i2cWrite_SENSOR(0x4d, 0x09);  
    i2cWrite_SENSOR(0x63, 0xe0);  
    i2cWrite_SENSOR(0x65, 0x20);  
    i2cWrite_SENSOR(0x66, 0x00);  
    i2cWrite_SENSOR(0x67, 0x48);  
    i2cWrite_SENSOR(0x0d, 0x41); 
    
    //0x51/0x61/0x71 for 
    //different AEC/AGC window
    i2cWrite_SENSOR(0x0f, 0xc5); 

    i2cWrite_SENSOR(0x14, 0x31);  

    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  
    i2cWrite_SENSOR(0x2b, 0x00);  
    i2cWrite_SENSOR(0x13, 0xff);  
    i2cWrite_SENSOR(0x90, 0x05);  
    i2cWrite_SENSOR(0x91, 0x01);  
    i2cWrite_SENSOR(0x92, 0x03);  
    i2cWrite_SENSOR(0x93, 0x00);  

#if 1
    i2cWrite_SENSOR(0x94, 0xb0);  
    i2cWrite_SENSOR(0x95, 0x9d);  
    i2cWrite_SENSOR(0x96, 0x13);  
    i2cWrite_SENSOR(0x97, 0x16);  
    i2cWrite_SENSOR(0x98, 0x7b);  
    i2cWrite_SENSOR(0x99, 0x91);  
    i2cWrite_SENSOR(0x9a, 0x1e); 
#else
    i2cWrite_SENSOR(0x94, 0x78);
    i2cWrite_SENSOR(0x95, 0x64);
    i2cWrite_SENSOR(0x96, 0x14);
    i2cWrite_SENSOR(0x97, 0x12);
    i2cWrite_SENSOR(0x98, 0x72);
    i2cWrite_SENSOR(0x99, 0x84);
    i2cWrite_SENSOR(0x9a, 0x1e);
#endif
    
    i2cWrite_SENSOR(0x9b, 0x08);  
    i2cWrite_SENSOR(0x9c, 0x20);  
    i2cWrite_SENSOR(0x9e, 0x81);  
        
    i2cWrite_SENSOR(0x33, 0x00);    //for 30 fps,60Hz
    i2cWrite_SENSOR(0x22, 0x7f);  
    i2cWrite_SENSOR(0x23, 0x03);  
    i2cWrite_SENSOR(0x33, 0x66);    //for 25 fps,50Hz
    i2cWrite_SENSOR(0x22, 0x99);  
    i2cWrite_SENSOR(0x23, 0x03);                        
//;;;;;;;;;;;;;;;
//start modify by mike for client

    i2cWrite_SENSOR(0x13, 0xff);

    i2cWrite_SENSOR(0x66, 0x80);
    i2cWrite_SENSOR(0x15, 0x02);


    i2cWrite_SENSOR(0x0c, 0x00);   //mirror image


    i2cWrite_SENSOR(0x2d, 0x00);
    i2cWrite_SENSOR(0x2e, 0x00);


    i2cWrite_SENSOR(0x11, 0x01);    //25fps
    i2cWrite_SENSOR(0x0d, 0x41);
    i2cWrite_SENSOR(0x33, 0xb3);
    i2cWrite_SENSOR(0x34, 0x00);
    i2cWrite_SENSOR(0x2a, 0x00);
    i2cWrite_SENSOR(0x2b, 0x00);
    i2cWrite_SENSOR(0x22, 0xab);
    i2cWrite_SENSOR(0x23, 0x03);

    i2cWrite_SENSOR(0x0e, 0xed);
    
    // Lens Correction  
    i2cWrite_SENSOR(0x47, 0x10);             //x   
    i2cWrite_SENSOR(0x48, 0x10);    //14       ;y         
    i2cWrite_SENSOR(0x4a, 0x08);    //10   ;r
    i2cWrite_SENSOR(0x49, 0x08);    //0c   ;8;10     ;G
    i2cWrite_SENSOR(0x4b, 0x08);    //0c   ;10;8;14  ;B
    i2cWrite_SENSOR(0x4c, 0x08);    //0c   ;8;10;17  ;R
    i2cWrite_SENSOR(0x46, 0x05); 
                              
    //@@ 0.75x

    i2cWrite_SENSOR(0xa7, 0x30);    // 0x18
    i2cWrite_SENSOR(0xa8, 0x30);    // 0x18

    i2cWrite_SENSOR(0xa6, 0x02);

    //Gamma      
#if 1
    /*
    i2cWrite_SENSOR(0x7e, 0x04);
    i2cWrite_SENSOR(0x7f, 0x0e);
    i2cWrite_SENSOR(0x80, 0x20);
    i2cWrite_SENSOR(0x81, 0x43);
    i2cWrite_SENSOR(0x82, 0x53);
    i2cWrite_SENSOR(0x83, 0x61);
    i2cWrite_SENSOR(0x84, 0x6d);
    i2cWrite_SENSOR(0x85, 0x76);
    i2cWrite_SENSOR(0x86, 0x7e);
    i2cWrite_SENSOR(0x87, 0x86);
    i2cWrite_SENSOR(0x88, 0x94);
    i2cWrite_SENSOR(0x89, 0xa1);
    i2cWrite_SENSOR(0x8a, 0xba);
    i2cWrite_SENSOR(0x8b, 0xcf);
    i2cWrite_SENSOR(0x8c, 0xe3);
    i2cWrite_SENSOR(0x8d, 0x26);
    */
    /*
    //Y3=[0,10,20,42,66,76, 86, 95,104,112,120,135,149,176,200,223,255];
    i2cWrite_SENSOR(0x7e, 10);
    i2cWrite_SENSOR(0x7f, 20);
    i2cWrite_SENSOR(0x80, 42);
    i2cWrite_SENSOR(0x81, 66);
    i2cWrite_SENSOR(0x82, 76);
    i2cWrite_SENSOR(0x83, 86);
    i2cWrite_SENSOR(0x84, 95);
    i2cWrite_SENSOR(0x85, 104);
    i2cWrite_SENSOR(0x86, 112);
    i2cWrite_SENSOR(0x87, 120);
    i2cWrite_SENSOR(0x88, 135);
    i2cWrite_SENSOR(0x89, 149);
    i2cWrite_SENSOR(0x8a, 176);
    i2cWrite_SENSOR(0x8b, 200);
    i2cWrite_SENSOR(0x8c, 233);
    i2cWrite_SENSOR(0x8d, 30);
    */
    //Y3=[0 14,23,37,60,70, 79  88, 97,105,113,129,143,171,197,221,255]
    i2cWrite_SENSOR(0x7e, 10);
    i2cWrite_SENSOR(0x7f, 20);
    i2cWrite_SENSOR(0x80, 37);
    i2cWrite_SENSOR(0x81, 60);
    i2cWrite_SENSOR(0x82, 70);
    i2cWrite_SENSOR(0x83, 79);
    i2cWrite_SENSOR(0x84, 88);
    i2cWrite_SENSOR(0x85, 97);
    i2cWrite_SENSOR(0x86, 105);
    i2cWrite_SENSOR(0x87, 113);
    i2cWrite_SENSOR(0x88, 129);
    i2cWrite_SENSOR(0x89, 143);
    i2cWrite_SENSOR(0x8a, 171);
    i2cWrite_SENSOR(0x8b, 197);
    i2cWrite_SENSOR(0x8c, 221);
    i2cWrite_SENSOR(0x8d, 46);
#else
    i2cWrite_SENSOR(0x7e, 0x0A);    //06   ; 0A
    i2cWrite_SENSOR(0x7f, 0x14);    //0C   ; 14
    i2cWrite_SENSOR(0x80, 0x28);    // 1A   ; 28
    i2cWrite_SENSOR(0x81, 0x4C);    //46   ; 4C
    i2cWrite_SENSOR(0x82, 0x5C);    //5E   ; 5C
    i2cWrite_SENSOR(0x83, 0x6A);    //74   ; 6A
    i2cWrite_SENSOR(0x84, 0x77);    //86   ; 77
    i2cWrite_SENSOR(0x85, 0x82);    //95   ; 82
    i2cWrite_SENSOR(0x86, 0x8B);    //A1   ; 8B
    i2cWrite_SENSOR(0x87, 0x93);    //AB   ; 93
    i2cWrite_SENSOR(0x88, 0xA1);    //BC   ; A1
    i2cWrite_SENSOR(0x89, 0xAC);    //C9   ; AC
    i2cWrite_SENSOR(0x8a, 0xC2);    //DD   ; C2
    i2cWrite_SENSOR(0x8b, 0xD7);    //EC   ; D7
    i2cWrite_SENSOR(0x8c, 0xE9);    //F5   ; E9
    i2cWrite_SENSOR(0x8d, 0x1E);    //0E   ; 1E
#endif
                                                            
    //awb-ver1.04-0321-1100        
    i2cWrite_SENSOR(0x75, 0xf0); 
    i2cWrite_SENSOR(0x76, 0x10);
    i2cWrite_SENSOR(0x77, 0x28);
    i2cWrite_SENSOR(0x79, 0x4e);
    i2cWrite_SENSOR(0x78, 0x1e);
    i2cWrite_SENSOR(0x7a, 0x2d);
    i2cWrite_SENSOR(0x72, 0x98);
    i2cWrite_SENSOR(0x71, 0x9c);
    i2cWrite_SENSOR(0x6e, 0x6a);
    i2cWrite_SENSOR(0x6f, 0x5b);
    i2cWrite_SENSOR(0x74, 0x36);
    i2cWrite_SENSOR(0x73, 0x10);
    i2cWrite_SENSOR(0x70, 0x0e); 
    i2cWrite_SENSOR(0x6d, 0x50);
    i2cWrite_SENSOR(0x6c, 0x11);
    i2cWrite_SENSOR(0x6b, 0x15);
    i2cWrite_SENSOR(0x6a, 0x00); 
    i2cWrite_SENSOR(0x69, 0x96);
                              
                              
    //----------------Modified-----------//                        
    i2cWrite_SENSOR(0x64, 0xbf);    //disable UVadjust
                              
    i2cWrite_SENSOR(0xac, 0x20);    //sharpness,denoise auto mode
    i2cWrite_SENSOR(0x90, 0x02);    //05      ;05->08 reduce denoise 
    i2cWrite_SENSOR(0x91, 0x01);
    i2cWrite_SENSOR(0x92, 0x04);    //03      ;03->05 enhance sharpness
    i2cWrite_SENSOR(0x93, 0x00);

    //Denoise filter
    i2cWrite_SENSOR(0x8e, 0x03);    //denoise off

    //AE windows
    i2cWrite_SENSOR(0x24, 0x48);
    i2cWrite_SENSOR(0x25, 0x38);
    i2cWrite_SENSOR(0x26, 0xc2);
                              
    //end modify by mike                           
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }
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

	u16 data;
    u8 level;
    int i,j;
    int FrameRate=30;
    static u8 isFirstBoot=0;
   // u32 sensorReadData[13]={0};
   // u32 sensorWriteData[13]={0xed,0xa4,0x07,0xf0,0x00,0xa0,0xf0,0x00,0x01,0x7f,0x09,0xe0,0x20};
   // u8 sensorReg[13]=      {0x0e,0x18,0x19,0x1a,0x32,0x29,0x2c,0x2a,0x11,0x42,0x4d,0x63,0x65};
 #if(FPGA_BOARD_A1018_SERIES)
    //---------Run on Mclk=32 MHz--------------//    
    #if(SYS_CPU_CLK_FREQ == 32000000)
	  SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=32/2=16MHz, 20 fps
	#elif(SYS_CPU_CLK_FREQ == 48000000)
      SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz, 30 fps
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
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=27MHz  
   #endif
 #endif

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 
        case SIUMODE_CAPTURE: 
        if(isFirstBoot==0)
        {
            i2cWrite_SENSOR(0x12, 0x80);    //Reset
            for(i=0;i<10000;i++);
            i2cWrite_SENSOR(0x12, 0x00);
            isFirstBoot++;
            DEBUG_SIU("sensor Reset \n");
        }
        
        SetOV7725_Init();
            break;
    }
    //----------------//
    /*
        Lucian:	當為VideoClip Zoom 模式時, 因錄影還在進行中,不可reset Mpeg control variable.
    */

    #if 0  //test sensor i2c 
    {
        for(i=0;i<10000;i++)
        {
            for(j=0;j<13;j++)
            {
                i2cWrite_SENSOR(sensorReg[j], sensorWriteData[j]); 
            }

            for(j=0;j<13;j++)
            {
                i2cRead_SENSOR(sensorReg[j], &data);  
                sensorReadData[j]=data;                    
            }
            
            for(j=0;j<13;j++)
            {
                if(sensorReadData[j] != sensorWriteData[j])
                {
                    DEBUG_SIU("Sensor data error \n");
                    while(1)
                        {
                        }
                    break;
                }
                else
                {
                    DEBUG_SIU("0x%x = 0x%x \n",sensorReg[j],sensorReadData[j]);
                }
    
     
            }
            for(j=0;j<13;j++)
            {
                 
                sensorReadData[j]=0;                    
            }
            DEBUG_SIU("Compare data No.%02d\n",i);  

            

        }
            
    }
    #endif

    
    if(siuSensorMode != SIUMODE_MPEGAVI_ZOOM)
    {
    	mp4_avifrmcnt = 0;
    	isu_avifrmcnt = 0;  //0;
    	avi_FrameSkip = 0;	
        unMp4SkipFrameCount = 0; /* mh@2006/11/22: for time stamp control */
    }
        
	return FrameRate;
}


void siuSensorSet_60fps()
{
    i2cWrite_SENSOR( 0x11, 0x0000);  // Internal clock= input_clock*4/((0+1)*2)=input_clock = 48MHz
    if(AE_Flicker_50_60_sel ==SENSOR_AE_FLICKER_60HZ )     //60Hz
    {
      i2cWrite_SENSOR(0x22, 128);
      i2cWrite_SENSOR(0x23, 3);
    }
    else //50Hz
    {
      i2cWrite_SENSOR(0x22, 153); 
      i2cWrite_SENSOR(0x23, 3);
    }
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
    
    //DEBUG_SIU("###test \n");
	if (intStat & SIU_INT_STAT_FIFO_OVERF)
	{
		/* Raw data FIFO overflow */
		DEBUG_SIU("\nSIU Data Overflow\n");
	}
	
	if (intStat & SIU_INT_STAT_FRAM)
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
                          
            siuFrameCount=0;
			
		}
    #endif		
        //No AE/AWB/AF , Do nothing
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

    i2cWrite_SENSOR(0x24, 0x48 + (expBiasValue - 4) * 12);
    i2cWrite_SENSOR(0x25, 0x38 + (expBiasValue - 4) * 12);
	
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


#define DAY_MODE_SWITCH_THR   (8*16)
#define NIGHT_MODE_SWITCH_THR (6*16)
#define DENOISE_ON_THR        (8*16)
#define MODESWITCH_THR        3



void siuTask(void* pData)
{
	u8 err;
    u8 level=0;
    u16 Again_x16,temp,AECH,AECL;
    u32 i;
    static int  Mode=SIU_NIGHT_MODE;
    u8 mdset;

    u8 pulse_time=250;   //每輸出250次Hight Level 大約 花0.5ms
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

s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    if(flicker_sel ==SENSOR_AE_FLICKER_60HZ )  //60Hz
    {
        i2cWrite_SENSOR(0x22, 0x7f);
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x00);
        i2cWrite_SENSOR(0x34, 0x00);    //Insert dummy line. -->7
    }
    else //50Hz
    {
        i2cWrite_SENSOR(0x22, 0x99);    // 132/99 for 50/25 fps
        i2cWrite_SENSOR(0x23, 0x03);    // 01/03/07/0f for 60/30/15/7.5fps
        i2cWrite_SENSOR(0x33, 0x66);    //Insert dummy line. -->102
        i2cWrite_SENSOR(0x34, 0x00);
    }

    return 0;
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

#if 1  // After Fine Tune @Lucian 20080729
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



/*
    60hz:

        510 x fps/120=a (banding value) to hex=b, 0x22=b

        510/a=c(banding step) c-1 to hex=d, 0x23=d

        Ex: 510x30/120=127.5 =0x7f, thus 0x22=7f

        510/127=4.016, 4-1=3, thus 0x23=03


    50hz: 

        510 x fps/100=a (banding value) to hex=b, 0x22=b

        510/a=c(banding step) c-1 to hex=d, 0x23=d

        Ex: 510x30/100=153=0x99, thus 0x22=99

        510/153=3.33, 3-1=2, thus 0x23=02


*/

void siu_SetD1_FPS(u8 res)
{
    #if 0
    switch(res)
    {
        case UI_MENU_VIDEO_SIZE_704x480:  /* 27 fps */
            i2cWrite_SENSOR(0x33,0x36);  /* dummy line */  /*  36/66/98 -> 27fps/25fps/23fps */
            
            break;

        case UI_MENU_VIDEO_SIZE_704x576:  /* 23 fps */
            i2cWrite_SENSOR(0x33,0x98);  /* dummy line */
            
            break;

        default:
            DEBUG_SIU("SetD1_FPS undefine resolution \n");
            break;
    }
    #endif
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






