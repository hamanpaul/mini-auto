
/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu_NONE.c

Abstract:

    The routines of Sensor Interface Unit.
    Control NT99230 (FHD) sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2015/011/27 Amon Li Create  
*/


#include "general.h"
#if ((Sensor_OPTION == Sensor_NONE_HD) || (Sensor_OPTION == Sensor_NONE_FHD))
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




u8 siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_OFF;
u8 siuFlashLightTestFlag = 0;
u8 siuFlashLightShoot = 0;      //決定是否打閃光燈.
u8 siuShootMainFlash=0;         //決定是否打出主閃: 用於User強迫閃光, 但環境又不需打閃光燈,旨在防紅眼.
u8  IsSensorInit;
u8 siuY_TargetIndex     = 4;
u8 AE_Flicker_50_60_sel = SENSOR_AE_FLICKER_60HZ;
SIU_AEINFO siu_aeinfo;
AE_WIN aeWin;
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


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 


s32 siuInit(void)
{
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
     (HW_BOARD_OPTION  == MR9200_RX_ROULE) || (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018) ||\
     (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
    // 2-11 pinmux set to sen_clk, senclk to WM8940 27M for 44.1k, then WM8940 I2S to HDMI
    u32 temp;
    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x000000ff)) | 0x00000011; // 486/18=27 MHz
#endif
    return 1;
}



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
#if(TV_DECODER == AHD6124B)

    //while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.     //開了會reboot

    // initialize sensor
    //rfiuVideoInFrameRate=siuSensorInit(mode,zoomFactor);

    // initialize the siu module
    // reset the SIU module
    SiuSensCtrl = SIU_RESET;

    // Set start address of raw buffer
    SiuRawAddr =    (u32)siuRawBuf;

    // Set sensor control
    SiuSensCtrl =
                SIU_NORMAL |
                SIU_ENA |                               //enable SIU module
                SIU_SLAVE |                             //For CMOS sensor
                SIU_VSYNC_ACT_HI |                      //Vsync active low
                SIU_HSYNC_ACT_HI |                      //Hsync active low
                SIU_DEF_PIX_DISA |                      //D.pixel compesation disable
                SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                //SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable //開了會reboot
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
                SIU_SINGLE_CAPTURE_DISA;
                //Single capture disable.

    SiuDebugSel=SIU_INSEL_CCIR656       |  //0x08364000;
//                SIU_BT1120_MODE         |             //this is for 1080P video, 720p no need
                SIU_YUVMAP_36;

    SiuSensCtrl |= SIU_CAPT_ENA;

#endif
    return 1;
}

s32 siuVideoClipInit(u8 zoomFactor)
{
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
#if(TV_DECODER == AHD6124B)
    int FrameRate;
    
    I2C_init_nvp6124B();
    //nvp6124b_switch_mode();
    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
    {
        FrameRate=10;
    }
    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
    {
        FrameRate=27;
    }
	return FrameRate;

#endif
    return 1;
}
void siuSetSensorDayNight(u8 Level)
{}
s32 siuAeInit(void)  
{
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
{}



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
	               Valid value : 0~8
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetExposureValue(s8 expBiasValue)
{
    return 1;
}


s32 siuSetFlicker50_60Hz(int flicker_sel)
{
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
    return 1;
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
    return 1;
}

s32 siuVideoZoom(s8 zoomFactor)
{
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
{}


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
                #if (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5)
                Img_Height  = 1072;
                #else
                Img_Height  = 1080;
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
{}



void siuStop(void)
{}

u16 getPreviewZoomScale(s32 zoomFactor)
{
    return 1;
}

void getPreviewZoomSize(s32 zoomFactor,u32* X,u32* Y)
{}


void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H)
{}


u16 getVideoZoomScale(s32 zoomFactor)
{
    return 1;
}

u16 getSensorRawWidth(void)
{
    return 1;
}

void siu_FCINTE_ena(void)
{}

void siu_FCINTE_disa(void)
{}

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

