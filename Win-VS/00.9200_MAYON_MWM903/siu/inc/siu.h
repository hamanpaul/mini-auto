/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	siu.h

Abstract:

   	The declarations of Sensor Interface Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SIU_H__
#define __SIU_H__

/* Type definition */




typedef struct _ZOOM_FACTOR{
   	SIU_COORD	size;
   	u16		scale;
}ZOOM_FACTOR;


typedef struct _SIU_HIST {
	u16		redLoBond;
	u16		redHiBond;
	u16		greenLoBond;
	u16		greenHiBond;
	u16		blueLoBond;
	u16		blueHiBond;
} SIU_HIST;

typedef struct _SIU_LENS_SHADING {
	SIU_COORD	centerOffset;
	u32		    cornerX2;
	u32 		cornerY2;
	u32		    shading_G_Gain;
	u32         shading_R_Gain;
	u32         shading_B_Gain;
	u32		    rType;
	u32		    scaleSize;
	u32		    outputInvert;
} SIU_LENS_SHADING;	



typedef struct _Analog_Gain {
	u8	gain;
	u8	mul;
}Analog_Gain;

typedef struct _AE_Tab {
	u16	EL;
	Analog_Gain	AG;
	u8	DG;
	u8  F_num;
	u16	DB_p; // this : next
	u16	DB_n; // this : next
}DEF_AE_Tab;




typedef struct _AWB_GAIN {
	u32	RGain;
	u32	GGainX256;	
	u32	BGain;
	u32	RGainX256_Adj;
	u32	BGainX256_Adj;	
	u8 	status;
} AWB_GAIN;

typedef struct _SIU_AEINFO{
	s32 	Ysum;
	u32	SensorPck;
	u32	SensorGainX64;
	u32     SensorExpX128;
	u16	Exposure_Line;
	u16	Gain_Code;
	s32	dGS;
	u32 	pGS;  			// will remove when debug OK
	u32	cGS;
	s32 	yerror;
	s32	stepsizeX64;

} SIU_AEINFO;

/* Constant */

#define SIU_COMP_B	0x00
#define SIU_COMP_Gb	0x01
#define SIU_COMP_Gr	0x02
#define SIU_COMP_R	0x03

#define SIU_AE_HIST_OUT	0x00
#define SIU_AE_HIST_MID	0x01
#define SIU_AE_HIST_INN 0x02

#define SIU_FLASH_LIGHT_ON_6EV_BOND	0x34
#define SIU_FLASH_LIGHT_ON_7EV_BOND	0x31
#define SIU_FLASH_LIGHT_ON_8EV_BOND	0x2C
#define SIU_FLASH_LIGHT_ON_9EV_BOND	0x29
#define SIU_FLASH_LIGHT_ON_10EV_BOND	0x24

// SDV1 ############################################
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_preCOORD_SDV1;

#define Capture_TargetNum_SDV1	512
typedef struct _DEFECT_PIXEL_COORDINATE_SDV1{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_COORDINATE_SDV1;

// SDV2 ############################################
// --------------------------------------------------------------C
#define preCapture_MaxNum_SDV2	2000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_preCOORD_SDV2;

#define Capture_TargetNum_SDV2	1001
typedef struct _DEFECT_PIXEL_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_COORDINATE_SDV2;
typedef struct _DEFECT_PIXEL_nCOORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_nCOORDINATE_SDV2;

// --------------------------------------------------------------1
#define Preview_1_preCapture_MaxNum_SDV2	1000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_Preview_1_preCOORD_SDV2;

#define Preview_1_Capture_TargetNum_SDV2	50
typedef struct _DEFECT_PIXEL_Preview_1_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_Preview_1_COORDINATE_SDV2;
// --------------------------------------------------------------2
#define Preview_2_preCapture_MaxNum_SDV2	1000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_Preview_2_preCOORD_SDV2;

#define Preview_2_Capture_TargetNum_SDV2	50
typedef struct _DEFECT_PIXEL_Preview_2_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_Preview_2_COORDINATE_SDV2;

// --------------------------------------------------------------1
#define Avi_1_preCapture_MaxNum_SDV2	1000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_Avi_1_preCOORD_SDV2;

#define Avi_1_Capture_TargetNum_SDV2	100
typedef struct _DEFECT_PIXEL_Avi_1_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_Avi_1_COORDINATE_SDV2;
// --------------------------------------------------------------2
#define Avi_2_preCapture_MaxNum_SDV2	1000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_Avi_2_preCOORD_SDV2;

#define Avi_2_Capture_TargetNum_SDV2	100
typedef struct _DEFECT_PIXEL_Avi_2_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_Avi_2_COORDINATE_SDV2;
//---------------------------------------------------------------3
#define Avi_3_preCapture_MaxNum_SDV2	1000
typedef struct {
	u16	X;
	u16	Y;
	u16	DIFF;
	u16 KIND;
}DEFECT_PIXEL_Avi_3_preCOORD_SDV2;

#define Avi_3_Capture_TargetNum_SDV2	100
typedef struct _DEFECT_PIXEL_Avi_3_COORDINATE_SDV2{
	u16	X;
	u16	Y;
}DEFECT_PIXEL_Avi_3_COORDINATE_SDV2;

#endif
