/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	siuapi.h

Abstract:

   	The application interface of Sensor Interface Unit

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SIU_API_H__
#define __SIU_API_H__

#include "MotionDetect_API.h"

#if( (Sensor_OPTION  == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MI1320_RAW) || (Sensor_OPTION == Sensor_OV2643_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601) )
  #define MAX_PREVIEW_ZOOM_FACTOR	            45 //23
  #define MAX_VIDEOCLIP_ZOOM_FACTOR             45 //23
#else
  #define MAX_PREVIEW_ZOOM_FACTOR	            19
  #define MAX_VIDEOCLIP_ZOOM_FACTOR             19
#endif

/* Constant */

#define SIU_LIGHT_SOURCE_UNKNOWN		        0x00
#define SIU_LIGHT_SOURCE_DAYLIGHT		        0x01
#define SIU_LIGHT_SOURCE_FLUORESCENT		    0x02
#define SIU_LIGHT_SOURCE_TUNGSTEN		        0x03
#define SIU_LIGHT_SOURCE_FLASH			        0x04
#define SIU_LIGHT_SOURCE_FINE_WEATHER		    0x09
#define SIU_LIGHT_SOURCE_CLOUDY_WEATHER		    0x0a
#define SIU_LIGHT_SOURCE_SHADE			        0x0b
#define SIU_LIGHT_SOURCE_DAYLIGHT_FLUORESCENT	0x0c
#define SIU_LIGHT_SOURCE_DAY_WHITE_FLUORESCENT	0x0d
#define SIU_LIGHT_SOURCE_COOL_WHITE_FLUORESCENT	0x0e
#define SIU_LIGHT_SOURCE_WHITE_FLUORESCENT	    0x0f
#define SIU_LIGHT_SOURCE_STANDARD_LIGHT_A	    0x11
#define SIU_LIGHT_SOURCE_STANDARD_LIGHT_B	    0x12
#define SIU_LIGHT_SOURCE_STANDARD_LIGHT_C	    0x13
#define SIU_LIGHT_SOURCE_D55			        0x14
#define SIU_LIGHT_SOURCE_D65			        0x15
#define SIU_LIGHT_SOURCE_D75			        0x16
#define SIU_LIGHT_SOURCE_D50			        0x17
#define SIU_LIGHT_SOURCE_ISO_STUDIO_TUNGSTEN	0x18
#define SIU_LIGHT_SOURCE_OTHER_LIGHT_SOURCE	    0xff

#define SIU_COLOR_TUNE_NORMAL			        0x00
#define SIU_COLOR_TUNE_SEPIA 			        0x01
#define SIU_COLOR_TUNE_BLACK_AND_WHITE		    0x02
#define SIU_COLOR_TUNE_NEGATIVE			        0x03
#define SIU_COLOR_TUNE_SOLARIZE			        0x04

#define SIU_FLASH_LIGHT_ALWAYS_OFF		        0x00
#define SIU_FLASH_LIGHT_AUTOMATIC		        0x01
#define SIU_FLASH_LIGHT_ALWAYS_ON		        0x02
#define SIU_FLASH_LIGHT_RED_EYE_REDUCTION	    0x03

//--Scene mode, ISO --//
#define SIU_SCENEMODE_AUTO                      0
#define SIU_SCENEMODE_SPORT                     1
#define SIU_SCENEMODE_LANDSCAPE                 2

#define SIU_ISO_AUTO                            0
#define SIU_ISO_100                             1
#define SIU_ISO_200                             2

//------AE algorithm------//

#if (Sensor_OPTION == Sensor_MI_5M)
    #define AECurSet_Min           6   //Lucian: 12->4
    #define AEPrevCurSet_Max      72   //89
    #define AECapCurSet_Max       72   //限制Capture mode 時, AE index 最大增益.
    #define AEVideoCurSet_Max     72   //75
#elif( Sensor_OPTION == Sensor_OV7725_VGA)
    #define AECurSet_Min           1   //Lucian: 12->4
    #define AEPrevCurSet_Max      78   //89
    #define AECapCurSet_Max       78   //限制Capture mode 時, AE index 最大增益.
    #define AEVideoCurSet_Max     78   //75
#else
    #define AECurSet_Min           6   //Lucian: 12->4
    #define AEPrevCurSet_Max      72   //89
    #define AECapCurSet_Max       72   //限制Capture mode 時, AE index 最大增益.
    #define AEVideoCurSet_Max     72   //75
#endif

#define SIU_FRAME_DURATION_NUM		        8	/* Peter 070403 */

//------ OB Parameter ------//
#if (Sensor_OPTION == Sensor_MI_5M)
#define SIU_OB_R   0x0028
#define SIU_OB_Gr  0x0028
#define SIU_OB_Gb  0x0028
#define SIU_OB_B   0x0028 // achi@2008/01/28
#elif( Sensor_OPTION == Sensor_OV7725_VGA)
  #if(HW_BOARD_OPTION == D010_CARDVR)
    #define SIU_OB_R   0
	#define SIU_OB_Gr  0
	#define SIU_OB_Gb  0
	#define SIU_OB_B   0  // achi@2008/01/28
  #else
    #define SIU_OB_R   0x00
	#define SIU_OB_Gr  0x00
	#define SIU_OB_Gb  0x00
	#define SIU_OB_B   0x00 // achi@2008/01/28
  #endif
#else
    #define SIU_OB_R   0x00
	#define SIU_OB_Gr  0x00
	#define SIU_OB_Gb  0x00
	#define SIU_OB_B   0x00 // achi@2008/01/28    
#endif

#define IPU_OB_R  (SIU_OB_R/4)
#define IPU_OB_Gr (SIU_OB_Gr/4)
#define IPU_OB_Gb (SIU_OB_Gb/4)
#define IPU_OB_B  (SIU_OB_B/4)

//------Factory Mode use--------//
#define DETECT_OPTICAL_BLACK  0
#define DETECT_LIGHT_PIXEL 1
#define DETECT_DARK_PIXEL 2

#define SIU_AWB_NORMAL              0
#define SIU_AWB_FORCEGRAY           1

/* SiuValidStart */
#define SIU_VALID_START_X_SHFT		0
#define SIU_VALID_START_Y_SHFT		16

#define SENSOR_NTSC_START_X     (139+12)
#define SENSOR_NTSC_START_Y      14    
#define SENSOR_PAL_START_X      163
#define SENSOR_PAL_START_Y       16

/*Mirror Flip*/
#define GPIO_MIRROR_FLIP_ON   1
#define GPIO_MIRROR_FLIP_OFF  0
#if((HW_BOARD_OPTION == MR8120_TX_TRANWO)||(HW_BOARD_OPTION == MR8120_TX_GCT)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO2)||(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
    ((HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE))||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
  #define SIU_DAY_MODE     0
  #define SIU_NIGHT_MODE   1
#else
  #define SIU_DAY_MODE     1
  #define SIU_NIGHT_MODE   0
#endif


typedef struct _SIU_COORD {
	u16		x;
	u16		y;
} SIU_COORD; 

typedef struct _AE_WIN {
	SIU_COORD	winStart;
	u16		winSizeX;
	u16 	winSizeY;
	u8		histLoBond;
	u8		histHiBond;
	u32     winSize;
	u32		scaleSize;
	u16     weightacc;
} AE_WIN;	

typedef struct _SENSOR_VALIDSIZE {
	SIU_COORD	imgStr;
	SIU_COORD	imgSize;
} SENSOR_VALIDSIZE;



/*------------------------------ Function prototype -------------------------------- */

extern s32 siuSetSubjectDistance(u32);
extern s32 siuSetExposureValue(s8);
extern s32 siuSetLightSource(u16);
extern s32 siuSetColorTune(u8);
extern s32 siuSetFlicker50_60Hz(int flicker_sel);
extern s32 siuSetImageResolution(u16, u16);
extern s32 siuSetFlashLight(u8);
extern s32 siuSetVideoResolution(u16, u16);
extern s32 siuGetOutputSize(u16*, u16*);
extern s32 siuPreview(s8);
extern s32 siuPreviewZoom(s8);
extern s32 siuCapturePrimary(s8);
extern s32 siuCapture_DarkLightPixelDetect(s8 zoomFactor,u8 SensorMode,u8 OpMode);
extern s32 siuCapture_WBComp(s8 zoomFactor);
extern s32 siuCaptureVideo(s8);/*BJ 0530 S*/
extern s32 siuVideoZoom(s8 zoomFactor);

extern s32 siuResumeTask(void);
extern s32 siuSuspendTask(void);

extern s32 siuInit(void);
extern void siuIntHandler(void);
extern s32 setSensorWinSize(s8, s8);

extern u16 getPreviewZoomScale(s32);
extern void getPreviewZoomSize(s32 zoomFactor,u32* X,u32* Y);


extern u16 getVideoZoomScale(s32);
extern u16 getSensorRawWidth(void);

extern s32 siuGetOutputSize(u16*, u16*);
extern void siuStop(void);
extern void sysSIURst(void);

extern s32 siuSetAeWin(AE_WIN*);
extern s32 siuSetAeBlkWeight(u8* , u16*);

extern u8 siuGetTVinStatus(void);
extern void siuSiuCtrl(u8);
extern void siuCCIR656_Ena(void);
#if((HW_BOARD_OPTION == MR8120_TX_GCT) || (HW_BOARD_OPTION == MR8120_TX_RDI2)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI3) || (HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS) ||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO)|| (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) ||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA530) || (HW_BOARD_OPTION == MR8120_TX_FRESHCAM)||\
    (HW_BOARD_OPTION == MR8120_TX_HECHI)|| (HW_BOARD_OPTION == MR8120_TX_MAYON_LW604)||\
    (HW_BOARD_OPTION == MR8120_TX_MA8806) ||(HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
extern void siuSetSensorDayNight(u8 Level);
#endif

#if(Sensor_OPTION == Sensor_CCIR601)
extern void siu_FID_INT_ena(void);       
extern void siu_FID_INT_disa(void);        
#endif

/* Only for 8600 TX series */
#if(Sensor_OPTION == Sensor_OV7725_YUV601) 
#if((HW_BOARD_OPTION == MR8120_TX_RDI2) || (HW_BOARD_OPTION == MR8120_TX_RDI3))
extern void siu_SetD1_FPS(u8 res);
#endif
#endif


extern void siuSetAWBGain(s32 Rgain,s32 Bgain);
/*------------------------------ Extern Golbal Variable -------------------------------- */


extern u8 siu_dftAeWeight1[];
extern AE_WIN aeWin;

extern u16 siuFlashLightPreChargeTimeCount;
extern u8 siuFlashLightMode;
extern u8 AE_Flicker_50_60_sel;
extern u8 siuOpMode;
extern SENSOR_VALIDSIZE sensor_validsize;
extern void siuSetSensorChrome(u8 level);

#endif
