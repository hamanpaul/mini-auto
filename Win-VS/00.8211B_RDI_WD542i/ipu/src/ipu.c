/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ipu.c

Abstract:

   	The routines of Image Processing Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"
#include "board.h"
#include "ipu.h"
#include "ipureg.h"
#include "siuapi.h"


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
 //---IPU control ---//
 IPU_SIZE   ipu_insize, ipu_outsize;

 //------- Color Correct Martix-------//
#if YUV_CCM_MERGE_ENA
    const s32 d50_IPU_CCM[10] = {256, 0, 0, 0, 256, 0, 0, 0, 256, 0};

    //----Color Transform Matrix----//
    #if(HW_BOARD_OPTION==SALIX_SDV)
      #if(Sensor_OPTION == Sensor_OV7725_VGA) //For SDV-1
          s16 ipuColorCorrTransform[10] = { 84, 73, 99, 3, -254, 252, 152, -118, -34, 0}; //from 蔡sir @2008/02/21        
      #else  //For SDV-2
          s16 ipuColorCorrTransform[10] = {64, 98, 94, -39,-168, 207, 191, -194, 2,0}; //from 蔡sir @2008/02/21
          //const s32 d50_IPU_CCM[10] = {392,-210,73,-96,374,-22,-29,-242,527,0}; //from 蔡sir @2008/08/30,More saturation
      #endif
    #elif(HW_BOARD_OPTION==HX_DH500)
          s16 ipuColorCorrTransform[10] = {64, 98, 94, -39,-168, 207, 191, -194, 2,0}; //from 蔡sir @2008/02/21 
    #elif(HW_BOARD_OPTION==AIPTEK_BOARD_V5o)      
      s16 ipuColorCorrTransform[10] = {64, 98, 94, -39, -168, 207, 191, -194, 2,0}; //from 蔡sir @2008/02/21
    #else
      s16 ipuColorCorrTransform[10] = {64, 98, 94, -39, -168, 207, 191, -194, 2,0}; //from 蔡sir @2008/02/21
    #endif     
#else
   #if(HW_BOARD_OPTION==SALIX_SDV)
     #if(Sensor_OPTION == Sensor_OV7725_VGA)
        const s32 d50_IPU_CCM[10] = {297, -92, 51, -25, 244, 36, 89, -378, 545, 0}; //from 蔡sir @2008/02/21
        //const s32 d50_IPU_CCM[10] = {350, -97, 3, -69, 340, -16, 80, -469, 645, 0}; //from 蔡sir @2008/08/04
        //const s32 d50_IPU_CCM[10] = {256, 0, 0, 0, 256, 0, 0, 0, 256, 0}; //from 蔡sir @2008/02/21
     #else
        const s32 d50_IPU_CCM[10] = {331,-173,97,-59,294,21,-5,-199,460,0}; //from 蔡sir @2008/02/21
        //const s32 d50_IPU_CCM[10] = {392,-210,73,-96,374,-22,-29,-242,527,0}; //from 蔡sir @2008/08/30,More saturation
     #endif
   #elif(HW_BOARD_OPTION==HX_DH500)
        const s32 d50_IPU_CCM[10] = {331,-173,97,-59,294,21,-5,-199,460,0}; //from 蔡sir @2008/02/21
   #elif(HW_BOARD_OPTION==AIPTEK_BOARD_V5o)
        const s32 d50_IPU_CCM[10] = {331,-173,97,-59,294,21,-5,-199,460,0}; //from 蔡sir @2008/02/21
   #elif(HW_BOARD_OPTION == D010_CARDVR )
       //const s32 d50_IPU_CCM[10] = {297, -92, 51, -25, 244, 36, 89, -378, 545, 0}; //from 蔡sir @2008/02/21
       //const s32 d50_IPU_CCM[10] = {244, -87, 99,  19, 148, 88, 98, -287, 445, 0};
       //const s32 d50_IPU_CCM[10] = { 271,-90, 75,  -3, 196, 62, 94, -333, 495, 0};
       const s32 d50_IPU_CCM[10] = { 233,-86, 109,  28, 128, 99, 100, -268, 424, 0};
   #else
        const s32 d50_IPU_CCM[10] = {331,-173,97,-59,294,21,-5,-199,460,0}; //from 蔡sir @2008/02/21
   #endif
    //----Color Transform Matrix----//
   s16 ipuColorCorrTransform[10] = {77, 150, 29, -43, -85, 128, 128, -107, -21, 0};//Lucian: modify @2008/02/01
#endif
s16 ipuCCM_WB_Matrix[10]= {85, 85, 85, 85, 85, 85, 85, 85, 85, 0};
s16 ipuCCM_I_Matrix[10] = {256, 0, 0, 0, 256, 0, 0, 0, 256, 0}; //Lucian: Modified for OV7725
s16 Preview_ipuCCM_DFT[10]={328, 0, 0, 0, 256, 0, 0, 0, 340, 0};


//---AWB Control ---//
#if(Sensor_OPTION==Sensor_MI1320_RAW)
  s16 AWBgain_Preview[3]= {1000, 1000, 1000};
  s16 AWBgain_Capture[3]= {1000, 1000, 1000};
#elif(Sensor_OPTION==Sensor_MI_5M)
  s16 AWBgain_Preview[3]= {1280, 1000, 1330};
  s16 AWBgain_Capture[3]= {1280, 1000, 1330};
#elif(Sensor_OPTION==Sensor_OV7740_RAW)
  s16 AWBgain_Preview[3]= {1280, 1000, 1330};
  s16 AWBgain_Capture[3]= {1280, 1000, 1330};
#else
  s16 AWBgain_Preview[3]= {1280, 1000, 1330};
  s16 AWBgain_Capture[3]= {1280, 1000, 1330};
#endif

//----YUV Gamma ---//lisa_2008_0408
//X[17]={0,1,3,7,11,15,23,31,39,47,63,79,95,127,159,191,255};
#if(HW_BOARD_OPTION == D010_CARDVR) //gamma=0.53,修正暗部
  //u8 YUV_Gamma[17]        = {0, 240, 224, 224, 192, 166, 131, 217, 182, 155, 238, 188, 151, 99, 64, 37, 1};
  #if 0
    u8 YUV_Gamma[17]        = {0, 131, 131, 131, 131, 131, 131, 217, 182, 155, 238, 188, 151, 99, 64, 37, 1};
    u8 YUV_Gamma_shft[17]   = {0,   6,   6,   6,   6,   6,   6,   7,   7,   7,   8,   8,   8,  8,  8,  8, 8};
  #else
    u8 YUV_Gamma[17]        = {0, 192, 160, 147, 138, 131, 245, 200, 169, 146, 225, 179, 143, 95, 61, 36, 1};
    u8 YUV_Gamma_shft[17]   = {0,   6,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,  8,  8,  8, 8};
  #endif
#else //gamma=0.6
  u8 YUV_Gamma[17]        = {0, 131, 158, 207, 162, 136, 208, 171, 144, 249,194,155,125,84,54,32, 1};
  u8 YUV_Gamma_shft[17]   = {0,   4,   5,   6,   6,   6,   7,   7,   7,   8,  8,  8,  8, 8, 8, 8, 8 };
#endif
//-----產線對焦校正------//
// OSD_AFreport
#if(Sensor_OPTION == Sensor_OV7725_VGA)
 u32 bar_max_value = 4000; // default max vlue, adjust value by case
#else
 u32 bar_max_value = 3000; // default max vlue, adjust value by case
#endif

u32 AFrpt_weight[5] = {1,1,4,1,1}; // setting AFreport weight
extern void osdDrawAFBarInfo(u8 buf_idx);
void ipuStop(void);
s32 ipuPreview(s8);


//-----CFA----//
IPU_CFAI_THRESH ipuCfaiThresh =
{
	11,	/* edgeGThresh:20 */  
	62	/* edgeSmoothHue:31 */
};

//---Edge Enhance---//
IPU_EDGE_ENHANCE ipuEdgeEnhance[3] = 
{ 
     {
      #if 0
        3	, // threshL 	
    	64  , // curvSlop1x32 
        24	, // addOffs1 	
        15	, // threshCornr1 	
        32  , // curvSlop2x32 	
        39  , // addOffs2 	
        30  , // threshCornr2 	
        194 , // curvSlop3x32 	
        60  , // threshH
      #else
        1	, // threshL 	
    	160  , // curvSlop1x32 
        10	, // addOffs1 	
        3	, // threshCornr1 	
        32  , // curvSlop2x32 	
        37  , // addOffs2 	
        30  , // threshCornr2 	
        194 , // curvSlop3x32 	
        60  , // threshH
      #endif
     },
     //Day mode
     {
      #if 0
        6	, // threshL 	
    	64, // curvSlop1x32 
        4	, // addOffs1 	
        8	, // threshCornr1 	
        64, // curvSlop2x32 	
        68, // addOffs2 	
        40, // threshCornr2 	
        0xB6, // curvSlop3x32 	
        80, // threshH
      #else
        3	, // threshL 	
    	64  , // curvSlop1x32 
        24	, // addOffs1 	
        15	, // threshCornr1 	
        32  , // curvSlop2x32 	
        39  , // addOffs2 	
        30  , // threshCornr2 	
        194 , // curvSlop3x32 	
        60  , // threshH
      #endif
     },
     //Night mode
     {
     #if 0
        12, // threshL 	
    	64, // curvSlop1x32 
        4	, // addOffs1 	
        14	, // threshCornr1 	
        64, // curvSlop2x32 	
        56, // addOffs2 	
        40, // threshCornr2 	
        0xAD, // curvSlop3x32 	
        80, // threshH 	
     #else
        6	, // threshL 	
    	64, // curvSlop1x32 
        4	, // addOffs1 	
        8	, // threshCornr1 	
        64, // curvSlop2x32 	
        68, // addOffs2 	
        40, // threshCornr2 	
        0xB6, // curvSlop3x32 	
        80, // threshH
     #endif
     },
};


//------- RGB gamma-----//
#define IPU_LUM_GAMMA_TBL_COUNT		18


#if(Sensor_OPTION == Sensor_OV7725_VGA)
/*
    IPU_LUM_GAMMA ipu_Y_GammaTbl[IPU_LUM_GAMMA_TBL_COUNT] = 
{
    // 0.7
    {	0	,	0	},
    {	4	,	14	},
    {	8	,	23	},
    {	12	,	30	},
    {	16	,	37	},
    {	24	,	49	},
    {	32	,	60	},
    {	40	,	70	},
    {	48	,	79	},
    {	64	,	97	},
    {	80	,	113	},
    {	96	,	129	},
    {	128	,	157	},
    {	160	,	184	},
    {	192	,	209	},
    {	224	,	233	},
    {	255	,	255	},
    {	0x00	,	0x00	}
};

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Prev[IPU_LUM_GAMMA_TBL_COUNT] = 
{
    // 0.7
    {	0	,	0	},
    {	4	,	14	},
    {	8	,	23	},
    {	12	,	30	},
    {	16	,	37	},
    {	24	,	49	},
    {	32	,	60	},
    {	40	,	70	},
    {	48	,	79	},
    {	64	,	97	},
    {	80	,	113	},
    {	96	,	129	},
    {	128	,	157	},
    {	160	,	184	},
    {	192	,	209	},
    {	224	,	233	},
    {	255	,	255	},
    {	0x00	,	0x00	}
};

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Cap[IPU_LUM_GAMMA_TBL_COUNT] = 
{
    // 0.7
    {	0	,	0	},
    {	4	,	14	},
    {	8	,	23	},
    {	12	,	30	},
    {	16	,	37	},
    {	24	,	49	},
    {	32	,	60	},
    {	40	,	70	},
    {	48	,	79	},
    {	64	,	97	},
    {	80	,	113	},
    {	96	,	129	},
    {	128	,	157	},
    {	160	,	184	},
    {	192	,	209	},
    {	224	,	233	},
    {	255	,	255	},
    {	0x00	,	0x00	}
};
*/

    IPU_LUM_GAMMA ipu_Y_GammaTbl[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Prev[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Cap[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };
#elif(Sensor_OPTION == Sensor_MI_5M)
    // -AC- 071127 [[
        IPU_LUM_GAMMA ipu_Y_GammaTbl[IPU_LUM_GAMMA_TBL_COUNT] = 
        {
                    {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	19	},
                    {	12	,	29	},
                    {	16	,	38	},
                    {	24	,	52	},
                    {	32	,	64	},
                    {	40	,	75	},
                    {	48	,	85	},
                    {	64	,	104	},
                    {	80	,	120	},
                    {	96	,	135	},
                    {	128	,	164	},
                    {	160	,	189	},
                    {	192	,	213	},
                    {	224	,	235	},
                    {	255	,	255	},
                    {	0x00,	0x00} // achi@2008/02/26
        };
                
        IPU_LUM_GAMMA ipuRGB_GammaTbl_Prev[IPU_LUM_GAMMA_TBL_COUNT] = 
        {
            {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	19	},
                    {	12	,	29	},
                    {	16	,	38	},
                    {	24	,	52	},
                    {	32	,	64	},
                    {	40	,	75	},
                    {	48	,	85	},
                    {	64	,	104	},
                    {	80	,	120	},
                    {	96	,	135	},
                    {	128	,	164	},
                    {	160	,	189	},
                    {	192	,	213	},
            {	224	,	235	},
            {	255	,	255	},
            {	0x00	,	0x00	} // achi@2008/02/26
        };

        IPU_LUM_GAMMA ipuRGB_GammaTbl_Cap[4][IPU_LUM_GAMMA_TBL_COUNT] = 
        {
                  { // index -0- : Gamma 0.65,Offset 4
                    {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	17	},
                    {	12	,	27	},
                    {	16	,	35	},
                    {	24	,	49	},
                    {	32	,	61	},
                    {	40	,	72	},
                    {	48	,	82	},
                    {	64	,	101	},
                    {	80	,	117	},
                    {	96	,	133	},
                    {	128	,	161	},
                    {	160	,	187	},
                    {	192	,	211	},
                    {	224	,	234	},
                    {	255	,	255	},
                    {	0x00,	0x00} 
                  },
                  { //index -1- : Gamma 0.68, Offset 8
                    {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	0	},
                    {	12	,	15	},
                    {	16	,	25	},
                    {	24	,	40	},
                    {	32	,	52	},
                    {	40	,	64	},
                    {	48	,	74	},
                    {	64	,	93	},
                    {	80	,	110	},
                    {	96	,	126	},
                    {	128	,	156	},
                    {	160	,	183	},
                    {	192	,	209	},
                    {	224	,	233	},
                    {	255	,	255	},
                    {	0x00,	0x00} 
                  },
                  { //index -2- : Gamma 0.68, offset 12 
                    {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	0	},
                    {	12	,	0	},
                    {	16	,	16	},
                    {	24	,	33	},
                    {	32	,	47	},
                    {	40	,	59	},
                    {	48	,	70	},
                    {	64	,	89	},
                    {	80	,	107	},
                    {	96	,	124	},
                    {	128	,	154	},
                    {	160	,	182	},
                    {	192	,	208	},
                    {	224	,	232	},
                    {	255	,	255	},
                    {	0x00,	0x00} 
                  },
                  { //index -3-: Gamma 0.7, offset 16 
                    {	0	,	0	},
                    {	4	,	0	},
                    {	8	,	0	},
                    {	12	,	0	},
                    {	16	,	0	},
                    {	24	,	24	},
                    {	32	,	38	},
                    {	40	,	51	},
                    {	48	,	62	},
                    {	64	,	83	},
                    {	80	,	101	},
                    {	96	,	119	},
                    {	128	,	150	},
                    {	160	,	179	},
                    {	192	,	206	},
                    {	224	,	231	},
                    {	255	,	255	},
                    {	0x00,	0x00} 
                  }
                };
#else
    IPU_LUM_GAMMA ipu_Y_GammaTbl[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Prev[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };

    IPU_LUM_GAMMA ipuRGB_GammaTbl_Cap[IPU_LUM_GAMMA_TBL_COUNT] = 
    {
                {	0	,	0	},
                {	4	,	0	},
                {	8	,	19	},
                {	12	,	29	},
                {	16	,	38	},
                {	24	,	52	},
                {	32	,	64	},
                {	40	,	75	},
                {	48	,	85	},
                {	64	,	104	},
                {	80	,	120	},
                {	96	,	135	},
                {	128	,	164	},
                {	160	,	189	},
                {	192	,	213	},
                {	224	,	235	},
                {	255	,	255	},
                {	0x00,	0x00} // achi@2008/02/26
    };
// -AC- 071127 ]]
#endif

//------ Faulse color supression------//
IPU_FALSE_COLOR_SUPPR ipuFalseColorSuppr = 
{
	2, 
	15,
	IPU_FCS_ENA	
};	

//---- DeNoise ----//
 IPU_DE_NOISE  ipuDeNoise = 
{	
	225,
	10, 
	3, 
	15,
	3 

};

//---- Y report -----//
IPU_YSUM_RPT ipuYSumRpt = 
{
	{0,0},
	{100,75},
	0,
	5,
	50
};

//---- AF report----//
IPU_AF_RPT	ipuAFRpt =
{

	{0,0},
	{105, 80},
	//{640, 480},
	0,
	10,
	100
};
	 

u16 OB_B = 0;
u16 OB_Gb = 0;
u16 OB_R = 0;
u16 OB_Gr = 0;	
	 
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u16 AECurSet;           //參考AE table index已決定Denoise參數.
extern u8 siuFlashLightShoot;  //參考閃光燈ON/OFF,已決定Denoise參數.
extern u8 siuAwbEnable;        //參考UI 選單,決定自動WB是否啟動.
	 
	 
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 ipuSetCfaiThresh(IPU_CFAI_THRESH*);
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
s32 ipuSetCfaAWBgain(s32 Rgain,s32 Bgain);
#endif
s32 ipuSetColorCorrMatrix(s16*);
s32 ipuSetColorTransMatrix(s16*);
s32 ipuSetEdgeEnhance(int index);
s32 ipuSetLumGamma(IPU_LUM_GAMMA*);
s32 ipuSetFalseColorSuppr(IPU_FALSE_COLOR_SUPPR*);
s32 ipuSetDeNoise(IPU_DE_NOISE*);
s32 ipuSetYsumReport(IPU_YSUM_RPT*);
s32 ipuSetAFReport(IPU_AF_RPT *);
s32 ipuGetOutputSize(u16*, u16*);
s32 ipuSetIOSize(u16, u16);
void ipuStop(void);
s32 ipuSetGamma(IPU_LUM_GAMMA*,IPU_LUM_GAMMA*,IPU_LUM_GAMMA*);
void ipuSetYUVGamma();
int EstimateLightLeak(unsigned char *Image,unsigned int W,unsigned int H);

void GetAwbGain(unsigned char *BayerImg,unsigned short *R_gain,unsigned short *B_gain); //Lucian 080103: For AWB gain

//OSD_AFreport
void ipuGetAFReport(void);
 
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
  
/*

Routine Description:

	Initialize the Image Processing Unit.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/

//lisa_2008_0408_no use

/*

Routine Description:

	The FIQ handler of Image Processing Unit.

Arguments:

	None.

Return Value:

	None.

*/
void ipuIntHandler(void)
{
	u32 status;
	
	status =IpuIntStrlStat;
}



/*

Routine Description:

	Set the threshold of Color Filter Array Interpolation.

Arguments:

	pThresh - The threshold of Color Filtre Array Interpolation.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuSetCfaiThresh(IPU_CFAI_THRESH* pThresh)
{
	IpuCfaInterp = (((u32)pThresh->edgeGThresh)   << IPU_CFAI_EDG_GTHRESH_SHFT) |
		           (((u32)pThresh->edgeSmoothHue) << IPU_CFAI_EDG_SMOOTHHUE_SHFT); 	
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
     IpuCfaInterp |= (0x0c8 <<IPU_HSW_SHFT);	
#endif
	
	return 1;	       
}

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
s32 ipuSetCfaAWBgain(s32 Rgain,s32 Bgain)
{
    Rgain=(Rgain*256+500)/1000; //Lucian: 轉為s2.8 format
    if(Rgain > 0x3ff)
        Rgain=0x3ff;
    Bgain=(Bgain*256+500)/1000;
    if(Bgain > 0x3ff)
        Bgain=0x3ff;
    
    IpuCfaRBgain = ((u32)Rgain << IPU_R_GAIN_SHFT) |
                   ((u32)Bgain << IPU_B_GAIN_SHFT);
}
#endif
/*

Routine Description:

	Set the Color Correction Matrix.

Arguments:

	pMatrix - The Color Correction Matrix.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuSetColorCorrMatrix(s16* pMatrix)
{
	volatile unsigned *ccm = &(IpuColorCorrMatrx1_2);
	u32 i;
	s32 even, odd;
	
	for (i = 0; i < 9; i += 2)
	{
		even = (s32)pMatrix[i];
		if (even < 0)
		{
			even = -even;
			if(even>1023) //Lucian: colore matrix coef. is only 10-bit, need to clip 
			  even=1023;
			even |= IPU_CCM_NEGATIVE;
		}
		else
		{
		   if(even>1023)
			  even=1023; 
		}
		//=======//
		odd = (s32)pMatrix[i+1];
		if (odd < 0)
		{
			odd = -odd;
			if(odd>1023)
			  odd=1023;
			odd |= IPU_CCM_NEGATIVE;
		}	
		else
		{
		    if(odd>1023)
			  odd=1023;
		}
		*ccm++ = (even << IPU_CCM_EVEN_SHFT) | (odd  << IPU_CCM_ODD_SHFT);	 
	}
 
	return 1; 	        		
}



/*

Routine Description:

	Set the parameters of Edge Enhancement.

Arguments:

	pEdgeEnhance - The parameter of Edge Enhancement.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuSetEdgeEnhance(int index)
{
    IPU_EDGE_ENHANCE* pEdgeEnhance;

    pEdgeEnhance=&ipuEdgeEnhance[index];
	IpuEdgEnhance1 = (((u32)pEdgeEnhance->threshL) 	    << IPU_EDG_ENH_THRESH_L_SHFT) |
			         (((u32)pEdgeEnhance->curvSlop1x32) << IPU_EDG_ENH_CURVSLOP1x32_SHFT) |
		       	     (((u32)pEdgeEnhance->addOffs1)     << IPU_EDG_ENH_ADDOFFS1_SHFT) |
		       	     (((u32)pEdgeEnhance->threshCornr1) << IPU_EDG_ENH_THRESH_CORNR1_SHFT);
	IpuEdgEnhance2 = (((u32)pEdgeEnhance->curvSlop2x32) << IPU_EDG_ENH_CURVSLOP2x32_SHFT) |
			         (((u32)pEdgeEnhance->addOffs2)     << IPU_EDG_ENH_ADDOFFS2_SHFT) |
			         (((u32)pEdgeEnhance->threshCornr2) << IPU_EDG_ENH_THRESH_CORNR2_SHFT) |
			         (((u32)pEdgeEnhance->curvSlop3x32) << IPU_EDG_ENH_CURVSLOP3x32_SHFT);
	IpuEdgEnhance3 = (((u32)pEdgeEnhance->threshH) 	    << IPU_EDG_ENH_THRESH_H);
		 
	return 1;		 
}

//lisa_2008_04_08  YUVGamma
#if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
void ipuSetYUVGamma()      
{                          
IpuYUVGamma1 = (((s32)YUV_Gamma[0]<< IPU_YUV_GAMMA_Gain0_SHFT) |
                            ((s32)YUV_Gamma[1] << IPU_YUV_GAMMA_Gain1_SHFT) |
                            ((s32)YUV_Gamma[2] << IPU_YUV_GAMMA_Gain2_SHFT) |
                            ((s32)YUV_Gamma[3] << IPU_YUV_GAMMA_Gain3_SHFT));

IpuYUVGamma2 = (((s32)YUV_Gamma[4]<< IPU_YUV_GAMMA_Gain0_SHFT) |
                            ((s32)YUV_Gamma[5] << IPU_YUV_GAMMA_Gain1_SHFT) |
                            ((s32)YUV_Gamma[6] << IPU_YUV_GAMMA_Gain2_SHFT) |
                            ((s32)YUV_Gamma[7] << IPU_YUV_GAMMA_Gain3_SHFT));

IpuYUVGamma3 = (((s32)YUV_Gamma[8] << IPU_YUV_GAMMA_Gain0_SHFT) |
                            ((s32)YUV_Gamma[9]  << IPU_YUV_GAMMA_Gain1_SHFT) |
                            ((s32)YUV_Gamma[10]<< IPU_YUV_GAMMA_Gain2_SHFT) |
                            ((s32)YUV_Gamma[11]<< IPU_YUV_GAMMA_Gain3_SHFT));

IpuYUVGamma4 = (((s32)YUV_Gamma[12]<< IPU_YUV_GAMMA_Gain0_SHFT) |
                            ((s32)YUV_Gamma[13] << IPU_YUV_GAMMA_Gain1_SHFT) |
                            ((s32)YUV_Gamma[14] << IPU_YUV_GAMMA_Gain2_SHFT) |
                            ((s32)YUV_Gamma[15] << IPU_YUV_GAMMA_Gain3_SHFT));  
     
IpuYUVGamma5 = ((s32)YUV_Gamma[16]<< IPU_YUV_GAMMA_Gain0_SHFT) ;
		  
 
IpuYUVGammaShift1=  (((s32)YUV_Gamma_shft[0]<<IPU_YUV_GAMMA_SHFT_Y0) |
                                    ((s32)YUV_Gamma_shft[1]<<IPU_YUV_GAMMA_SHFT_Y1) |
                                    ((s32)YUV_Gamma_shft[2]<<IPU_YUV_GAMMA_SHFT_Y2) |
                                    ((s32)YUV_Gamma_shft[3]<<IPU_YUV_GAMMA_SHFT_Y3) |
                                    ((s32)YUV_Gamma_shft[4]<<IPU_YUV_GAMMA_SHFT_Y4) |
                                    ((s32)YUV_Gamma_shft[5]<<IPU_YUV_GAMMA_SHFT_Y5) |
                                    ((s32)YUV_Gamma_shft[6]<<IPU_YUV_GAMMA_SHFT_Y6) |
                                    ((s32)YUV_Gamma_shft[7]<<IPU_YUV_GAMMA_SHFT_Y7)); 

IpuYUVGammaShift2=  (((s32)YUV_Gamma_shft[8]<<IPU_YUV_GAMMA_SHFT_Y0) |
                                    ((s32)YUV_Gamma_shft[9]<<IPU_YUV_GAMMA_SHFT_Y1) |
                                    ((s32)YUV_Gamma_shft[10]<<IPU_YUV_GAMMA_SHFT_Y2) |
                                    ((s32)YUV_Gamma_shft[11]<<IPU_YUV_GAMMA_SHFT_Y3) |
                                    ((s32)YUV_Gamma_shft[12]<<IPU_YUV_GAMMA_SHFT_Y4) |
                                    ((s32)YUV_Gamma_shft[13]<<IPU_YUV_GAMMA_SHFT_Y5) |
                                    ((s32)YUV_Gamma_shft[14]<<IPU_YUV_GAMMA_SHFT_Y6) |
                                    ((s32)YUV_Gamma_shft[15]<<IPU_YUV_GAMMA_SHFT_Y7)); 

IpuYUVGammaShift3=   ((s32)YUV_Gamma_shft[16]<<IPU_YUV_GAMMA_SHFT_Y0);
                           
/*
    IpuLumGammaCorr0_1	=0x00010001;
    IpuLumGammaCorr2_3	=0x00010001;
    IpuLumGammaCorr4_5	=0x00010001;
    IpuLumGammaCorr6_7	=0x00010001;
    IpuLumGammaCorr8_9	=0x00000001;
    IpuLumGammaCorr10_11=0x88888888;
    IpuLumGammaCorr12_13=0x88888888;	
    IpuLumGammaCorr14_15=0x00000008;
*/	
}                          
#else   // PA9001D
s32 ipuSetColorTransMatrix(s16* pMatrix)
{
	volatile unsigned *ctm = &(IpuColorTransMatrx1_2);
	u32 i;
	s32 even, odd;
	
	for (i = 0; i < 9; i += 2)
	{
		even = (s32)pMatrix[i];
		if (even < 0)
		{
			even = -even;
			even |= IPU_CCM_NEGATIVE;
		}
		odd = (s32)pMatrix[i+1];
		if (odd < 0)
		{
			odd = -odd;
			odd |= IPU_CCM_NEGATIVE;
		}	
		*ctm++ = (even << IPU_CCM_EVEN_SHFT) | (odd  << IPU_CCM_ODD_SHFT);	 
	}
 	
	return 1; 	        		
}

/*

Routine Description:

	Set the Luminance Gamma.

Arguments:

	pTbl - The Luminance Gamma Table.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuSetLumGamma(IPU_LUM_GAMMA* pLumGammaTable)
{
	volatile unsigned *lumGamma = &(IpuLumGammaCorr0_1);
	u32 i;
	
	for (i = 0; i < IPU_LUM_GAMMA_TBL_COUNT; i += 2)
	{
		*lumGamma++ = (((s32)pLumGammaTable[i].x)   << IPU_LUM_GAMMA_X_EVEN_SHFT) | 
			      (((s32)pLumGammaTable[i].y)   << IPU_LUM_GAMMA_Y_EVEN_SHFT) |
			      (((s32)pLumGammaTable[i+1].x) << IPU_LUM_GAMMA_X_ODD_SHFT)  | 
			      (((s32)pLumGammaTable[i+1].y) << IPU_LUM_GAMMA_Y_ODD_SHFT);
	}

	return 1; 	        		
}


s32 ipuSetRGBGamma(IPU_LUM_GAMMA* pRGB_GammaTable)
{
	volatile unsigned *RGamma = &(IpuRGammaCorr0_1);
	volatile unsigned *GGamma = &(IpuGGammaCorr0_1);
	volatile unsigned *BGamma = &(IpuBGammaCorr0_1);
	u32 i;
	
	for (i = 0; i < IPU_LUM_GAMMA_TBL_COUNT; i += 2)
	{
		*RGamma++ = (((s32)pRGB_GammaTable[i].x)   << IPU_LUM_GAMMA_X_EVEN_SHFT) | 
			        (((s32)pRGB_GammaTable[i].y)   << IPU_LUM_GAMMA_Y_EVEN_SHFT) |
			        (((s32)pRGB_GammaTable[i+1].x) << IPU_LUM_GAMMA_X_ODD_SHFT)  | 
			        (((s32)pRGB_GammaTable[i+1].y) << IPU_LUM_GAMMA_Y_ODD_SHFT);

		*GGamma++ = (((s32)pRGB_GammaTable[i].x)   << IPU_LUM_GAMMA_X_EVEN_SHFT) | 
			        (((s32)pRGB_GammaTable[i].y)   << IPU_LUM_GAMMA_Y_EVEN_SHFT) |
			        (((s32)pRGB_GammaTable[i+1].x) << IPU_LUM_GAMMA_X_ODD_SHFT)  | 
			        (((s32)pRGB_GammaTable[i+1].y) << IPU_LUM_GAMMA_Y_ODD_SHFT);

		*BGamma++ = (((s32)pRGB_GammaTable[i].x)   << IPU_LUM_GAMMA_X_EVEN_SHFT) | 
			        (((s32)pRGB_GammaTable[i].y)   << IPU_LUM_GAMMA_Y_EVEN_SHFT) |
			        (((s32)pRGB_GammaTable[i+1].x) << IPU_LUM_GAMMA_X_ODD_SHFT)  | 
			        (((s32)pRGB_GammaTable[i+1].y) << IPU_LUM_GAMMA_Y_ODD_SHFT);
	}

	return 1; 	        		
}
#endif
/*

Routine Description:

	Set the parameter of False Color Suppression

Arguments:

	pSuppr - The paramter of False Color Suppression.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuSetFalseColorSuppr(IPU_FALSE_COLOR_SUPPR* pSuppr)
{
	IpuFalsColorSuppr = (((u32)pSuppr->edgeThresh)   << IPU_FCS_EDG_THRESH_SHFT) |
		       	    (((u32)pSuppr->decSlopex256) << IPU_FCS_DECSLOPx256_SHFT) |
		       	    (((u32)pSuppr->enable)); 

	return 1;	       
}

/*

Routine Description:

	Set the parameter of Hue and Saturation

Arguments:

	pHueSatur - The paramter of Hue and Saturation.

Return Value:

	0 - Failure.
	1 - Success.

*/


s32	ipuSetDeNoise(IPU_DE_NOISE* pipuDeNoise)
{
	IpuDeNoise1 =(((u32)pipuDeNoise->adf_diffthd) << IPU_ADF_DIFF_SHFT) |
	       	     (((u32)pipuDeNoise->diff_thd)    << IPU_DN_DIFF_SHFT)  |
	       	     (((u32)pipuDeNoise->noisedeg)); 
	       	    
	IpuDeNoise2 =(((u32)pipuDeNoise->adf_diff_cnt)<< IPU_ADF_DIFFCNT_SHFT) |
	       	     (((u32)pipuDeNoise->diff_cnt));
	return 1;
}

s32 ipuSetYsumReport(IPU_YSUM_RPT* pipuYSumRpt)
{
	IpuYRptStr		= (((u32)pipuYSumRpt->Ysum_str.w) << IPU_YRPT_W_SHFT)  | (((u32)pipuYSumRpt->Ysum_str.h)<<IPU_YRPT_H_SHFT);
	IpuYRptWinSize	= (((u32)pipuYSumRpt->Ysum_size.w) << IPU_YRPT_W_SHFT) | (((u32)pipuYSumRpt->Ysum_size.h)<<IPU_YRPT_H_SHFT);
	IpuYRptCtl		= (((u32)pipuYSumRpt->Yhtgh_thd)<<IPU_YRPT_YHTHD_SHFT)| (((u32)pipuYSumRpt->Yhtgl_thd)<<IPU_YRPT_YLTHD_SHFT)| (((u32)pipuYSumRpt->Ysum_scale)<<IPU_YRPT_YSCA_SHFT);
	return 1;
}

s32 ipuSetAFReport(IPU_AF_RPT * pipuAFRpt)
{
	IpuAFRptStr		= (((u32)pipuAFRpt->AF_str.w)<<IPU_AFRPR_W_SHFT) | (((u32)pipuAFRpt->AF_str.h)<<IPU_AFRPR_H_SHFT);
	IpuAFRptSize		= (((u32)pipuAFRpt->AF_size.w)<<IPU_AFRPR_W_SHFT) | (((u32)pipuAFRpt->AF_size.h)<<IPU_AFRPR_H_SHFT);
	IpuAFRptCtl		= (((u32)pipuAFRpt->AF_scale)<<IPU_AFRPT_SCA_SHFT) | (((u32)pipuAFRpt->AF_edgethd)<<IPU_AFRPT_ETHD_SHFT) | (((u32)pipuAFRpt->AF_highbond)<<IPU_AFRPT_HBOND_SHFT);	
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
s32 ipuPreview(s8 zoomFactor)
{
    u16 i;
    s16 ipuCCM_DFT[10];
	// IPU reset
	IpuEna = 1;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;
    
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IpuFunEna = IPUFE_EGEN   |
	            IPUFE_YUVGAEN |
	            IPUSRAM5K_EN  |
   	            IPUFE_FCEN;  
#elif(CHIP_OPTION == CHIP_PA9002D) 
	IpuFunEna = IPUFE_EGEN   |
	            IPUFE_YUVGAEN |
   	            IPUFE_FCEN;      
#else   // PA9001D
    IpuFunEna = IPUFE_EGEN |
   	            IPUFE_FCEN |
   	            IPUFE_AFREN |
   	            IPUFE_RGBGAEN;
#endif    
    
    // Clear all INT sources in IPU
	IpuIntStrlStat = 0x00000000;

    // set IPU operate at preview mode
	IpuModeCtrl = 0x00000000;
	
	// Set IPU input resolution
	//siuGetOutputSize(&ipu_insize.h, &ipu_insize.w);
	IpuInSize = (ipu_insize.h << IPU_SIZE_Y_SHFT) | ipu_insize.w;   // Input size: 804 X 604    

    	// Set IPU output resolution
	//ipu_outsize.w = ipu_insize.w - 4;
	//ipu_outsize.h = ipu_insize.h - 4;
	IpuOutSize =(ipu_outsize.h << IPU_SIZE_Y_SHFT) | ipu_outsize.w; //IPU OUT_SIZE: 800 X 600	
	

	ipuSetCfaiThresh(&ipuCfaiThresh);
	
     //lisa_2008_0408  CTM Fix ,combine AWB and CCM for PA9002d
 #if (CHIP_OPTION == CHIP_PA9002D)
        #if(PA9002D_AWB_EN==1)    
            ipuCCM_DFT[0] = (s16)d50_IPU_CCM[0];
            ipuCCM_DFT[1] = (s16)d50_IPU_CCM[1];
            ipuCCM_DFT[2] = (s16)d50_IPU_CCM[2];
            ipuCCM_DFT[3] = (s16)d50_IPU_CCM[3];
            ipuCCM_DFT[4] = (s16)d50_IPU_CCM[4];
            ipuCCM_DFT[5] = (s16)d50_IPU_CCM[5];
            ipuCCM_DFT[6] = (s16)d50_IPU_CCM[6];
            ipuCCM_DFT[7] = (s16)d50_IPU_CCM[7];
            ipuCCM_DFT[8] = (s16)d50_IPU_CCM[8];
        #else
            ipuCCM_DFT[0] = (s16)((d50_IPU_CCM[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R));
        	ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1]*AWBgain_Preview[1]/1000);
        	ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2]*AWBgain_Preview[2]/1000);
        	ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3]*AWBgain_Preview[0]/1000);
        	ipuCCM_DFT[4] = (s16)((d50_IPU_CCM[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr));
        	ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5]*AWBgain_Preview[2]/1000);
        	ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6]*AWBgain_Preview[0]/1000);
        	ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7]*AWBgain_Preview[1]/1000);
        	ipuCCM_DFT[8] = (s16)((d50_IPU_CCM[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B));
        #endif

            ipuSetColorCorrMatrix(ipuCCM_DFT); 
            ipuSetEdgeEnhance(0);
            ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
            ipuSetDeNoise(&ipuDeNoise);
            ipuSetYUVGamma();
 #elif( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A )|| \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))   //PA9003A
            //CCM 與 CTM 合併運算: 做矩陣乘法.
            
            ipuCCM_DFT[0]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[6])/256;
            ipuCCM_DFT[1]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[7])/256;
            ipuCCM_DFT[2]= ( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[8])/256;

            ipuCCM_DFT[3]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[6])/256;
            ipuCCM_DFT[4]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[7])/256;
            ipuCCM_DFT[5]= ( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[8])/256;

            ipuCCM_DFT[6]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[6])/256;
            ipuCCM_DFT[7]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[7])/256;
            ipuCCM_DFT[8]= ( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[8])/256;

      #if  PA9003_AWB_GAIN_IN_SIU
            siuSetAWBGain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                            (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                         );
      #else
            ipuSetCfaAWBgain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                               (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                            );
      #endif
            ipuSetColorCorrMatrix(ipuCCM_DFT); 
            //ipuSetColorCorrMatrix(ipuColorCorrTransform);
            ipuSetEdgeEnhance(0);
            ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
            ipuSetDeNoise(&ipuDeNoise);
            ipuSetYUVGamma();

    
#else   // PA9001D
            ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
            
        	ipuSetColorTransMatrix(ipuColorCorrTransform);
        	ipuSetEdgeEnhance(0);
        	ipuSetLumGamma(ipu_Y_GammaTbl);
        	ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
        	ipuSetDeNoise(&ipuDeNoise);
        	ipuSetYsumReport(&ipuYSumRpt);
        	ipuSetAFReport(&ipuAFRpt);
        	ipuSetRGBGamma(ipuRGB_GammaTbl_Prev);
#endif
	
	// IPU engine reset
	IpuEna = 0;
	IpuEna = 1;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;
	
	// IPU bus reset
	for (i=0; i<50; i++)   ;
	IpuEna = 4;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;
	
	// IPU Enable
	IpuEna = 2; 
	
	return 1;
}

// OSD_AFreport 
#if(FACTORY_TOOL == TOOL_ON)
void ipuGetAFReport(void)
{
	u32 AFreport0_1, AFreport2_3, AFreport4_5, AFreport6_7, AFreport8;
	u32 AFrpt1, AFrpt2, AFrpt3, AFrpt4, AFrpt5, AFrpt6, AFrpt7, AFrpt8, AFrpt9;

	u32 bar_max = bar_max_value / 32; // 32 steps bar chart

	// for average report
	u32 AFrpt_avg = 0;
	u8 rptavg_string[10];
	u8 rptavg_bar;
	u8 i;
	u32 weight=0;

	AFreport0_1 = IpuAFRpt0_1;
	AFreport2_3 = IpuAFRpt2_3;
	AFreport4_5 = IpuAFRpt4_5;
	AFreport6_7 = IpuAFRpt6_7;
	AFreport8   = IpuAFRpt8;

	AFrpt1 = (u16)((AFreport0_1 & IPU_MASK_0) >> IPU_SHFT_0);
	AFrpt3 = (u16)((AFreport2_3 & IPU_MASK_0) >> IPU_SHFT_0);
	AFrpt5 = (u16)((AFreport4_5 & IPU_MASK_0) >> IPU_SHFT_0);
	AFrpt7 = (u16)((AFreport6_7 & IPU_MASK_0) >> IPU_SHFT_0);
	AFrpt9 = (u16)((AFreport8 & IPU_MASK_0) >> IPU_SHFT_0);

	for (i=0;i<=4;i++)
	{
		weight += AFrpt_weight[i];
	}

	AFrpt_avg = (AFrpt1*AFrpt_weight[0]+ AFrpt3*AFrpt_weight[1] + AFrpt5*AFrpt_weight[2] + AFrpt7*AFrpt_weight[3] +AFrpt9*AFrpt_weight[4])/weight;

	if(AFrpt_avg<10)
		sprintf(rptavg_string, "avg:%4d", AFrpt_avg);
	else if (AFrpt_avg<100)
	{
		AFrpt_avg = (AFrpt_avg/10)*10;
		sprintf(rptavg_string, "avg:%4d", AFrpt_avg);
	}
	else
	{
		AFrpt_avg = (AFrpt_avg/10)*10;
		sprintf(rptavg_string, "avg:%4d", AFrpt_avg);
	}

	rptavg_bar = AFrpt_avg/bar_max;

    if(sysTVOutOnFlag)
    {
        uiMenuOSDStringByColor(TVOSD_SizeX , rptavg_string, 8 , 16, 128 , 130 , 2 ,0xc1, 0xc6);
	    osdDrawAFBarAvg(TVOSD_SizeX , 256 , 24 , 32 ,160 , rptavg_bar, 2); //32*4 = 128	
	    osdDrawAFBarInfo(2);
    }
    else
    {
	    uiMenuOSDStringByColor(160 , rptavg_string, 8 , 16, 48 , 130 , 2 , 0xc1, 0xc6);
	    osdDrawAFBarAvg(160 , 128 , 8 , 12 ,160 , rptavg_bar, 2); //32*4 = 128	
    }	
}
#endif

s32 ipuGetOutputSize(u16 *height_size, u16 *width_size)
{
	*height_size = ipu_outsize.h;
	*width_size =  ipu_outsize.w;	
	return 1; 	
}

s32 ipuSetIOSize(u16 width, u16 height)
{
	ipu_insize.h  = height;
	ipu_insize.w  = width;	
	ipu_outsize.h = height-4;
	ipu_outsize.w = width-4;

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
s32 ipuCapturePrimary(void)
{
    u16 i;
    INT8U err;
    s16 Capture_ipuCCM_DFT[10];
    s16 ipuCCM_DFT[10];
    u32 status;
	s16 GammaTabIdx=0;
	/* raw datato yuv data */
	// IPU reset
	IpuEna = IPUEE_RST;
	for (i=0; i<30; i++)   ;
	IpuEna &= (~IPUEE_RST);

	// Enable all IPU function and disable INT

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        IpuFunEna = IPUFE_EGEN   |
    	            IPUFE_YUVGAEN |
    	            IPUSRAM5K_EN  |
       	            IPUFE_FCEN;  	
#elif(CHIP_OPTION == CHIP_PA9002D)
	    IpuFunEna = IPUFE_EGEN |
	   	            IPUFE_YUVGAEN |
	   	            IPUFE_FCEN;
#else   // PA9001D
    #if(Sensor_OPTION == Sensor_OV7725_VGA)
        IpuFunEna = IPUFE_EGEN |
       	            IPUFE_FCEN |
       	            IPUFE_RGBGAEN;
    #else
       	IpuFunEna = IPUFE_EGEN |
       	            IPUFE_FCEN |
       	            IPUFE_AFREN |
       	            IPUFE_RGBGAEN;
    #endif
#endif
    // Clear all INT sources in IPU
	status =IpuIntStrlStat;      //read clear
	IpuModeCtrl = IPUMCTL_BLKMODE |
	              IPUMCTL_CAPMODE |
	              IPUMCTL_BLK_16;  //Lucian: only support Block_16 mode

	// Set IPU input resolution
	//siuGetOutputSize(&ipu_insize.h, &ipu_insize.w);
	IpuInSize = (ipu_insize.h << IPU_SIZE_Y_SHFT) | ipu_insize.w;   

    // Set IPU output resolution
	//ipu_outsize.w = ipu_insize.w - 4;
	//ipu_outsize.h = ipu_insize.h - 4;
	IpuOutSize =(ipu_outsize.h << IPU_SIZE_Y_SHFT) | ipu_outsize.w; 
	
	// set ipu output buffers ----  block mode
	IpuSrcAddr  =  (u32) &(siuRawBuf[0]);
	IpuDst0Addr =  (u32)&(ipuDstBuf0[0]);
	IpuDst1Addr =  (u32)&(ipuDstBuf1[0]);
	IpuDst2Addr =  (u32)&(ipuDstBuf2[0]);	
		
	//===Set CFA Parameter==//
	ipuSetCfaiThresh(&ipuCfaiThresh);
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
     #if ADDAPP2TOJPEG
        #if( PA9003_AWB_GAIN_IN_SIU ) //Lucian: 選擇用preview awb gain. on 9002D 
            AWBgain_Capture[0]=AWBgain_Preview[0];
            AWBgain_Capture[1]=AWBgain_Preview[1];
            AWBgain_Capture[2]=AWBgain_Preview[2];
        #else
            if(siuAwbEnable==1)
            {
                //===Do Calculate AWB gain===//
                GetAwbGain(exifApp2Data->AwbImg,&AWBgain_Capture[0],&AWBgain_Capture[2]); //Lucian 080103: For AWB gain
                AWBgain_Capture[1]=1000;                  
            }
        #endif
               
        exifApp2Data->AWBgain_Capture[0]=AWBgain_Capture[0];
        exifApp2Data->AWBgain_Capture[1]=AWBgain_Capture[1];
        exifApp2Data->AWBgain_Capture[2]=AWBgain_Capture[2];
        
        exifApp2Data->AWBgain_Preview[0]=AWBgain_Preview[0];
        exifApp2Data->AWBgain_Preview[1]=AWBgain_Preview[1];
        exifApp2Data->AWBgain_Preview[2]=AWBgain_Preview[2];
     #else
        AWBgain_Capture[0]=AWBgain_Preview[0];
        AWBgain_Capture[1]=AWBgain_Preview[1];
        AWBgain_Capture[2]=AWBgain_Preview[2];
     #endif
		
		//CCM 與 CTM 合併運算: 做矩陣乘法.
        ipuCCM_DFT[0]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[6])/256;
        ipuCCM_DFT[1]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[7])/256;
        ipuCCM_DFT[2]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[8])/256;
    
        ipuCCM_DFT[3]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[6])/256;
        ipuCCM_DFT[4]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[7])/256;
        ipuCCM_DFT[5]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[8])/256;
    
        ipuCCM_DFT[6]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[6])/256;
        ipuCCM_DFT[7]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[7])/256;
        ipuCCM_DFT[8]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[8])/256;

        ipuSetColorCorrMatrix(ipuCCM_DFT);
      #if  PA9003_AWB_GAIN_IN_SIU
        siuSetAWBGain(  (s32)AWBgain_Capture[0] *(255)/(255-OB_R),
                            (s32)AWBgain_Capture[2] *(255)/(255-OB_B) 
                         );
      #else
        ipuSetCfaAWBgain(   (s32)AWBgain_Capture[0] *(255)/(255-OB_R),
                            (s32)AWBgain_Capture[2] *(255)/(255-OB_B) 
                        );
      #endif
            
    #if ADDAPP2TOJPEG	
    	for(i=0;i<9;i++)
    	  exifApp2Data->CCM[i]=d50_IPU_CCM[i];
    #endif  
    	ipuSetEdgeEnhance(0);
    	ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
        ipuSetDeNoise(&ipuDeNoise);            	       	
        ipuSetYUVGamma();   

#elif(CHIP_OPTION == CHIP_PA9002D)
     #if ADDAPP2TOJPEG
        #if( PA9002D_AWB_EN ) //Lucian: 選擇用preview awb gain. on 9002D 
            AWBgain_Capture[0]=AWBgain_Preview[0];
            AWBgain_Capture[1]=AWBgain_Preview[1];
            AWBgain_Capture[2]=AWBgain_Preview[2];
        #else
            if(siuAwbEnable==1)
            {
                //===Do Calculate AWB gain===//
                GetAwbGain(exifApp2Data->AwbImg,&AWBgain_Capture[0],&AWBgain_Capture[2]); //Lucian 080103: For AWB gain
                AWBgain_Capture[1]=1000;                  
            }
        #endif
               
        exifApp2Data->AWBgain_Capture[0]=AWBgain_Capture[0];
        exifApp2Data->AWBgain_Capture[1]=AWBgain_Capture[1];
        exifApp2Data->AWBgain_Capture[2]=AWBgain_Capture[2];
        
        exifApp2Data->AWBgain_Preview[0]=AWBgain_Preview[0];
        exifApp2Data->AWBgain_Preview[1]=AWBgain_Preview[1];
        exifApp2Data->AWBgain_Preview[2]=AWBgain_Preview[2];
     #else
        AWBgain_Capture[0]=AWBgain_Preview[0];
        AWBgain_Capture[1]=AWBgain_Preview[1];
        AWBgain_Capture[2]=AWBgain_Preview[2];
     #endif
     
      #if( PA9002D_AWB_EN )     
        ipuCCM_DFT[0] = (s16)d50_IPU_CCM[0];
        ipuCCM_DFT[1] = (s16)d50_IPU_CCM[1];
        ipuCCM_DFT[2] = (s16)d50_IPU_CCM[2];
        ipuCCM_DFT[3] = (s16)d50_IPU_CCM[3];
        ipuCCM_DFT[4] = (s16)d50_IPU_CCM[4];
        ipuCCM_DFT[5] = (s16)d50_IPU_CCM[5];
        ipuCCM_DFT[6] = (s16)d50_IPU_CCM[6];
        ipuCCM_DFT[7] = (s16)d50_IPU_CCM[7];
        ipuCCM_DFT[8] = (s16)d50_IPU_CCM[8];
      #else
        ipuCCM_DFT[0] = (s16)((d50_IPU_CCM[0]*AWBgain_Capture[0]/1000)*(255)/(255-OB_R));
		ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1]*AWBgain_Capture[1]/1000);
		ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2]*AWBgain_Capture[2]/1000);
		ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3]*AWBgain_Capture[0]/1000);
		ipuCCM_DFT[4] = (s16)((d50_IPU_CCM[4]*AWBgain_Capture[1]/1000)*(255)/(255-OB_Gr));
		ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5]*AWBgain_Capture[2]/1000);
		ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6]*AWBgain_Capture[0]/1000);
		ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7]*AWBgain_Capture[1]/1000);
		ipuCCM_DFT[8] = (s16)((d50_IPU_CCM[8]*AWBgain_Capture[2]/1000)*(255)/(255-OB_B));
      #endif
        ipuSetColorCorrMatrix(ipuCCM_DFT);
            
    #if ADDAPP2TOJPEG	
    	for(i=0;i<9;i++)
    	  exifApp2Data->CCM[i]=d50_IPU_CCM[i];
    #endif  
    	ipuSetEdgeEnhance(0);
    	ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
        ipuSetDeNoise(&ipuDeNoise);            	       	
        ipuSetYUVGamma();
        
#else   // PA9001D
         /* SW 0107 S */
         #if ADDAPP2TOJPEG
            if(siuAwbEnable==1)
         {
             //===Do Calculate AWB gain===//
                GetAwbGain(exifApp2Data->AwbImg,&AWBgain_Capture[0],&AWBgain_Capture[2]); //Lucian 080103: For AWB gain
                AWBgain_Capture[1]=1000;                  
            }
               
            GammaTabIdx=EstimateLightLeak(exifApp2Data->AwbImg,160,120);
            
            exifApp2Data->AWBgain_Capture[0]=AWBgain_Capture[0];
            exifApp2Data->AWBgain_Capture[1]=AWBgain_Capture[1];
            exifApp2Data->AWBgain_Capture[2]=AWBgain_Capture[2];
            
            exifApp2Data->AWBgain_Preview[0]=AWBgain_Preview[0];
            exifApp2Data->AWBgain_Preview[1]=AWBgain_Preview[1];
            exifApp2Data->AWBgain_Preview[2]=AWBgain_Preview[2];
                         
            exifApp2Data->GammaTabIdx=GammaTabIdx;
                         
         #else
            AWBgain_Capture[0]=AWBgain_Preview[0];
            AWBgain_Capture[1]=AWBgain_Preview[1];
            AWBgain_Capture[2]=AWBgain_Preview[2];
         #endif
        
    		Capture_ipuCCM_DFT[0] = (s16)((d50_IPU_CCM[0]*AWBgain_Capture[0]/1000)*(255)/(255-OB_R));
    		Capture_ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1]*AWBgain_Capture[1]/1000);
    		Capture_ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2]*AWBgain_Capture[2]/1000);
    		Capture_ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3]*AWBgain_Capture[0]/1000);
    		Capture_ipuCCM_DFT[4] = (s16)((d50_IPU_CCM[4]*AWBgain_Capture[1]/1000)*(255)/(255-OB_Gr));
    		Capture_ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5]*AWBgain_Capture[2]/1000);
    		Capture_ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6]*AWBgain_Capture[0]/1000);
    		Capture_ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7]*AWBgain_Capture[1]/1000);
    		Capture_ipuCCM_DFT[8] = (s16)((d50_IPU_CCM[8]*AWBgain_Capture[2]/1000)*(255)/(255-OB_B));
        	ipuSetColorCorrMatrix(Capture_ipuCCM_DFT);
	/* SW 0107 E */
    #if ADDAPP2TOJPEG	
    	for(i=0;i<9;i++)
    	  exifApp2Data->CCM[i]=d50_IPU_CCM[i];
    #endif

	ipuSetColorTransMatrix(ipuColorCorrTransform);
	ipuSetEdgeEnhance(0);
	ipuSetLumGamma(ipu_Y_GammaTbl); //Y gamma
	ipuSetFalseColorSuppr(&ipuFalseColorSuppr);

	ipuSetDeNoise(&ipuDeNoise);
	ipuSetYsumReport(&ipuYSumRpt); // Y report
	ipuSetAFReport(&ipuAFRpt);
	
    #if(Sensor_OPTION == Sensor_OV7725_VGA)
	  ipuSetRGBGamma(ipuRGB_GammaTbl_Cap);
    #elif(Sensor_OPTION == Sensor_MI_5M)
	  ipuSetRGBGamma(ipuRGB_GammaTbl_Cap[GammaTabIdx]);
	#else
	  ipuSetRGBGamma(ipuRGB_GammaTbl_Cap);
	#endif	
	
#endif
	
	//IPU reset//
	IpuEna = IPUEE_RST | IPUEE_BUSRST;
	for (i=0; i<50; i++)   ;
	IpuEna = 0;
	
	isuStatus_OnRunning = 1;/*BJ 0428*/
	IpuEna = IPUEE_EN; // IPU Enable
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))		
    OSSemPend(isuSemEvt, ISU_TIMEOUT*3 ,&err);
#else
    OSSemPend(isuSemEvt, ISU_TIMEOUT*2 ,&err);
#endif
    if (err != OS_NO_ERR)
    {
        DEBUG_IPU("Error: isuSemEvt(ipu+isu) is %d,IsuSCA_INTC=0x%x.\n", err,IsuSCA_INTC);
        return 0;   
    }      

	/* set related EXIF IFD ... */
		
	return 1;	
}

/*BJ 0530 S*/
/*

Routine Description:

	CaptureVideo.

Arguments:

	zoomFactor - Zoom factor.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ipuCaptureVideo(void)
{
    u16 i;
    s16 ipuCCM_DFT[10];
	// IPU reset
	IpuEna = 1;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;

	// Enable all IPU function and disable INT
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IpuFunEna = IPUFE_EGEN   |
	            IPUFE_YUVGAEN |
	            IPUSRAM5K_EN  |
   	            IPUFE_FCEN;  	
#elif(CHIP_OPTION == CHIP_PA9002D)
    IpuFunEna = IPUFE_EGEN |
   	            IPUFE_YUVGAEN |
   	            IPUFE_FCEN;
#else   // PA9001D
 #if(Sensor_OPTION  == Sensor_OV7725_VGA) //OV7725 ISP build-in, no need post-processing.                 
    IpuFunEna = IPUFE_EGEN |
   	            IPUFE_FCEN |
   	            IPUFE_RGBGAEN;
 #else
	IpuFunEna = IPUFE_EGEN |  //Lucian: Video 先拿掉Edge enhance,減輕鋸齒現象
   	            IPUFE_FCEN |
   	            IPUFE_AFREN |
   	            IPUFE_RGBGAEN;
 #endif       	            
#endif    
    
    	// Clear all INT sources in IPU
	IpuIntStrlStat = 0x00000000;

    	// set IPU operate at preview mode
	IpuModeCtrl = 0x00000000;
	
	// Set IPU input resolution
	//siuGetOutputSize(&ipu_insize.h, &ipu_insize.w);
	IpuInSize = (ipu_insize.h << IPU_SIZE_Y_SHFT) | ipu_insize.w;   // Input size: 804 X 604    

    	// Set IPU output resolution
	//ipu_outsize.w = ipu_insize.w - 4;
	//ipu_outsize.h = ipu_insize.h - 4;
	IpuOutSize =(ipu_outsize.h << IPU_SIZE_Y_SHFT) | ipu_outsize.w; //IPU OUT_SIZE: 800 X 600	
	
	ipuSetCfaiThresh(&ipuCfaiThresh);
    
#if (CHIP_OPTION == CHIP_PA9002D)
    #if( PA9002D_AWB_EN==1 )    
        ipuCCM_DFT[0] = (s16)d50_IPU_CCM[0];
        ipuCCM_DFT[1] = (s16)d50_IPU_CCM[1];
        ipuCCM_DFT[2] = (s16)d50_IPU_CCM[2];
        ipuCCM_DFT[3] = (s16)d50_IPU_CCM[3];
        ipuCCM_DFT[4] = (s16)d50_IPU_CCM[4];
        ipuCCM_DFT[5] = (s16)d50_IPU_CCM[5];
        ipuCCM_DFT[6] = (s16)d50_IPU_CCM[6];
        ipuCCM_DFT[7] = (s16)d50_IPU_CCM[7];
        ipuCCM_DFT[8] = (s16)d50_IPU_CCM[8];
    #else
        ipuCCM_DFT[0] = (s16)((d50_IPU_CCM[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R));
    	ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1] *AWBgain_Preview[1]/1000);
    	ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2] *AWBgain_Preview[2]/1000);
    	ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3] *AWBgain_Preview[0]/1000);
    	ipuCCM_DFT[4] = (s16)((d50_IPU_CCM[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr));
    	ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5] *AWBgain_Preview[2]/1000);
    	ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6] *AWBgain_Preview[0]/1000);
    	ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7] *AWBgain_Preview[1]/1000);
    	ipuCCM_DFT[8] = (s16)((d50_IPU_CCM[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B));
    #endif

        ipuSetColorCorrMatrix(ipuCCM_DFT);
        ipuSetEdgeEnhance(0);
        ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
        ipuSetDeNoise(&ipuDeNoise);
        ipuSetYUVGamma();
#elif( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))  
    //CCM 與 CTM 合併運算: 做矩陣乘法.
    ipuCCM_DFT[0]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[6])/256;
    ipuCCM_DFT[1]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[7])/256;
    ipuCCM_DFT[2]=( (s32)ipuColorCorrTransform[0]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[1]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[2]*d50_IPU_CCM[8])/256;

    ipuCCM_DFT[3]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[6])/256;
    ipuCCM_DFT[4]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[7])/256;
    ipuCCM_DFT[5]=( (s32)ipuColorCorrTransform[3]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[4]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[5]*d50_IPU_CCM[8])/256;

    ipuCCM_DFT[6]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[0]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[3]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[6])/256;
    ipuCCM_DFT[7]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[1]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[4]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[7])/256;
    ipuCCM_DFT[8]=( (s32)ipuColorCorrTransform[6]*d50_IPU_CCM[2]+ (s32)ipuColorCorrTransform[7]*d50_IPU_CCM[5]+  (s32)ipuColorCorrTransform[8]*d50_IPU_CCM[8])/256;

 #if  PA9003_AWB_GAIN_IN_SIU
        siuSetAWBGain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                        (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                     );
 #else
    ipuSetCfaAWBgain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                       (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                    );
 #endif
 
    ipuSetColorCorrMatrix(ipuCCM_DFT);
    ipuSetEdgeEnhance(0);
    ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
    ipuSetDeNoise(&ipuDeNoise);
    ipuSetYUVGamma();


#else   // PA9001D
    ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
    ipuSetColorTransMatrix(ipuColorCorrTransform);
    ipuSetEdgeEnhance(0);
    ipuSetLumGamma(ipu_Y_GammaTbl);
    ipuSetFalseColorSuppr(&ipuFalseColorSuppr);
    ipuSetDeNoise(&ipuDeNoise);
    ipuSetYsumReport(&ipuYSumRpt);
    ipuSetAFReport(&ipuAFRpt);
    ipuSetRGBGamma(ipuRGB_GammaTbl_Prev);
#endif	
	
	// IPU engine reset
	IpuEna = 0;
	IpuEna = 1;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;
	
	// IPU bus reset
	for (i=0; i<50; i++)   ;
	IpuEna = 4;
	for (i=0; i<10; i++)   ;
	IpuEna = 0;
	
	// IPU Enable
	IpuEna = 2; 
	
	return 1;
}
/*BJ 0530 E*/

void ipuStop(void)
{
	IpuEna = 0;
}

#define LIGHT_LEAK_THR  30

int EstimateLightLeak(unsigned char *Image,unsigned int W,unsigned int H)
{
    unsigned int Hist[4];
    unsigned int i,j;
	unsigned int idx;

	Hist[3]=Hist[2]=Hist[1]=Hist[0]=0;

	for(j=0;j<H;j++)
	{
	    for(i=0;i<W;i++)
		{
		    if( *Image<4)
			  Hist[0] +=1;
			else if( *Image<8)
		      Hist[1] +=1;
		    else if( *Image<12)
		      Hist[2] +=1;
			else if( *Image<16)
		      Hist[3] +=1;
			Image ++;
		}
	}
    
	Hist[1] += Hist[0];
    Hist[2] += Hist[1];
	Hist[3] += Hist[2];

	Hist[0]= (Hist[0]*1000)/(W*H);
	Hist[1]= (Hist[1]*1000)/(W*H);
	Hist[2]= (Hist[2]*1000)/(W*H);
	Hist[3]= (Hist[3]*1000)/(W*H);

	if(Hist[3]<LIGHT_LEAK_THR)
		return 3;
	else if(Hist[2]<LIGHT_LEAK_THR)
		return 2;
	else if(Hist[1]<LIGHT_LEAK_THR)
		return 1;
	else
		return 0;
}


void ipuDenoise_ON()
{
   IpuFunEna |= IPUFE_DEEN ;
}

void ipuDenoise_OFF()
{
   IpuFunEna &= (~IPUFE_DEEN);
}

void ipuYUVgamma_ON()
{
   IpuFunEna |= IPUFE_YUVGAEN ;
}

void ipuYUVgamma_OFF()
{
   IpuFunEna &= (~IPUFE_YUVGAEN) ;
}
