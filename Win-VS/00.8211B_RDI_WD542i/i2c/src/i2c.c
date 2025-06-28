/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	i2c.c

Abstract:

   	The routines of I2C controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"

#include "i2c.h"
#include "i2capi.h"
#include "i2creg.h"
#include "sensorapi.h"
#include "gpioapi.h"  //lisa 070514
#include "siuapi.h"
#include "uiapi.h"
#include "sysapi.h"


#include <assert.h>

#if((HW_BOARD_OPTION==JSW_DVRBOX)||(HW_BOARD_OPTION==JSY_DVRBOX))
#include "uiapi.h"
#endif
#if IS_COMMAX_DOORPHONE
#include "..\..\ui\inc\PortControl.h"
#include "..\..\ui\inc\MainFlow.h"
#include "..\..\ui\inc\Menu.h"
extern void Menu_setColor(MenuID);  // MenuFunc.c

#elif (HW_BOARD_OPTION==MR6730_AFN) 
#include "..\..\ui\inc\MainFlow.h"

#endif

/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/

/* I2C time out value */
#define I2C_TIMEOUT		20	/*CY 1023*/

/* I2C slave address */
#if (Sensor_OPTION ==Sensor_MI_5M)
    #define I2C_SENSOR_WR_SLAV_ADDR	0x000000BA
    #define I2C_SENSOR_RD_SLAV_ADDR	0x000000BB
#elif( (Sensor_OPTION == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MI1320_RAW))
  #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION == A1018B_FPGA_BOARD) ||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define I2C_SENSOR_WR_SLAV_ADDR	0x000000BA
    #define I2C_SENSOR_RD_SLAV_ADDR	0x000000BB
  #else
    #define I2C_SENSOR_WR_SLAV_ADDR	0x000000BA
    #define I2C_SENSOR_RD_SLAV_ADDR	0x000000BB
  #endif

#elif (Sensor_OPTION == Sensor_OV2643_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR	0x00000060
#define I2C_SENSOR_RD_SLAV_ADDR	0x00000061

#elif (Sensor_OPTION == Sensor_MT9M131_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR	0x000000BA
#define I2C_SENSOR_RD_SLAV_ADDR	0x000000BB

#elif (Sensor_OPTION == Sensor_OV7725_VGA)
#define I2C_SENSOR_WR_SLAV_ADDR	0x00000042
#define I2C_SENSOR_RD_SLAV_ADDR	0x00000043

#elif(Sensor_OPTION == Sensor_OV7725_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR	0x00000042
#define I2C_SENSOR_RD_SLAV_ADDR	0x00000043

#elif( (Sensor_OPTION == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) || (Sensor_OPTION == Sensor_OV7740_RAW) )
#define I2C_SENSOR_WR_SLAV_ADDR	0x00000042
#define I2C_SENSOR_RD_SLAV_ADDR	0x00000043

#elif(Sensor_OPTION == Sensor_MI9V136_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR	0x00000090
#define I2C_SENSOR_RD_SLAV_ADDR	0x00000091

#elif(Sensor_OPTION == Sensor_PC1089_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR    0x00000066
#define I2C_SENSOR_RD_SLAV_ADDR    0x00000067

#elif(Sensor_OPTION == Sensor_HM1375_YUV601)
#if 1   // SADR = 0
#define I2C_SENSOR_WR_SLAV_ADDR    0x00000048
#define I2C_SENSOR_RD_SLAV_ADDR    0x00000049
#else   // SADR = 1
#define I2C_SENSOR_WR_SLAV_ADDR    0x00000068
#define I2C_SENSOR_RD_SLAV_ADDR    0x00000069
#endif

#elif(Sensor_OPTION == Sensor_HM5065_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR    0x0000003E
#define I2C_SENSOR_RD_SLAV_ADDR    0x0000003F

#elif(Sensor_OPTION == Sensor_NT99141_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR    0x00000054
#define I2C_SENSOR_RD_SLAV_ADDR    0x00000055

#elif(Sensor_OPTION == Sensor_PO3100K_YUV601)
#define I2C_SENSOR_WR_SLAV_ADDR    0x00000064
#define I2C_SENSOR_RD_SLAV_ADDR    0x00000065

#endif

#define I2C_ALC5621_WR_SLAV_ADDR  0x00000034
#define I2C_ALC5621_RD_SLAV_ADDR  0x00000035

// for ST LIS302DL G sensor
#if 0   // SDO == 0
#define I2C_LIS302DL_WR_SLAV_ADDR   0x00000038
#define I2C_LIS302DL_RD_SLAV_ADDR   0x00000039
#else   // SDO  = 1 or floating
#define I2C_LIS302DL_WR_SLAV_ADDR   0x0000003a
#define I2C_LIS302DL_RD_SLAV_ADDR   0x0000003b
#endif

// for Hitachi H30CD G sensor
#if 0   // A0 == 0
#define I2C_H30CD_WR_SLAV_ADDR      0x000000d0
#define I2C_H30CD_RD_SLAV_ADDR      0x000000d1
#else   // A0 == 1
#define I2C_H30CD_WR_SLAV_ADDR      0x000000d2
#define I2C_H30CD_RD_SLAV_ADDR      0x000000d3
#endif

// for Domintech DMARD03 G sensor
#if 1   // SDO == 0
#define I2C_DMARD03_WR_SLAV_ADDR    0x00000038
#define I2C_DMARD03_RD_SLAV_ADDR    0x00000039
#else   // SDO  = 1
#define I2C_DMARD03_WR_SLAV_ADDR    0x0000003a
#define I2C_DMARD03_RD_SLAV_ADDR    0x0000003b
#endif

// for Bosch BMA150 G sensor
#define I2C_BMA150_WR_SLAV_ADDR     0x00000070
#define I2C_BMA150_RD_SLAV_ADDR     0x00000071

#define I2C_PT2257_WR_SLAV_ADDR    0x00000088
#define I2C_PT2257_RD_SLAV_ADDR    0x00000089


#define I2C_MI9V136_WR_SLAV_ADDR    0x90
#define I2C_MI9V136_RD_SLAV_ADDR    0x91


#if(THERMOMETER_SEL == THERMO_MLX90615)
#define I2C_MLX90615_WR_SLAV_ADDR   0xB6
#define I2C_MLX90615_RD_SLAV_ADDR   0xB7
#endif

#if(EXT_RTC_SEL == RTC_ISL1208)
#define I2C_RTC_WR_SLAV_ADDR    0x000000DE
#define I2C_RTC_RD_SLAV_ADDR    0x000000DF
#define RTC_STATUS				0x07
#define STATUS_WRTC				0x10
#define STATUS_ARST				0x80
#endif

#if(EXT_RTC_SEL == RTC_SD2068)
#define I2C_RTC_WR_SLAV_ADDR    0x00000064
#define I2C_RTC_RD_SLAV_ADDR    0x00000065
#endif

#if(EXT_RTC_SEL == RTC_BQ32000)
#define I2C_BQ32000_WR_SLAV_ADDR    0x000000d0
#define I2C_BQ32000_RD_SLAV_ADDR    0x000000d1
#endif


#if(TOUCH_KEY == TOUCH_KEY_CBM7320)
#define I2C_CBM7320_RD_SLAV_ADDR    0x000000A1
#endif

#if(DOOR_BELL_SUPPORT)
#define I2C_433RFMod_R_SLAV_ADDR  0x00000041
#define I2C_433RFMod_W_SLAV_ADDR  0x01000040
#endif

#if (Melody_SNC7232_ENA)
#define Melody_SNC7232_SDA_W(Value)     gpioSetLevel(1, 18, Value)
#define Melody_SNC7232_SDA_R(Value)     gpioGetLevel(1, 18, Value)
#define Melody_SNC7232_SCK_W(Value)     gpioSetLevel(1, 5, Value)
#define Melody_SNC7232_SDA_DirI()       gpioSetDir(1,18,GPIO_DIR_IN)
#define Melody_SNC7232_SDA_DirO()       gpioSetDir(1,18,GPIO_DIR_OUT)
#define Melody_SNC7232_SCK_DirI()       gpioSetDir(1,5,GPIO_DIR_IN)
#define Melody_SNC7232_SCK_DirO()       gpioSetDir(1,5,GPIO_DIR_OUT)

extern u8 Melody_play;
extern u8 Melody_play_num;
extern u8 Melody_retry;

#define SNC7232delay 4000//2050

#endif

#if( (Sensor_OPTION == Sensor_MI9V136_YUV601) || (TV_DECODER == MI9V136) )
typedef __packed struct _IICDATA
{
    unsigned short addr;
    unsigned short data;
}DEF_IICDATA;
#endif

#if(Sensor_OPTION == Sensor_PC1089_YUV601)
typedef struct _IICDATA
{
 unsigned char Addr;
 unsigned short Value;
}DEF_IICDATA;

#endif
#if(TV_DECODER==WT8861)
     u8 bVideoColourSelect,bYCDelay,b3DParameter;
     u8 fChangeMode;
     u8 bVideoResetTimer;
     u8 bVideoField;
    #define ON        1
    #define OFF       0
    #define	VIDEO_RESET_TIMER		50 //100  // 3 // 5     //Minimun value 3
    enum
	{
		NTSC_COLOUR = 0,
		PALI_COLOUR,
		PALM_COLOUR,
		PALCN_COLOUR,
		SECAM_COLOUR,
		PAL60_COLOUR,
		NTSC443_COLOUR,
		NONE_COLOUR
	};
	enum
	{
		INPUT_SOURCE_TUNER,
		INPUT_SOURCE_AV1,  // CVBS
		INPUT_SOURCE_YPBPR,
		INPUT_SOURCE_HDTV,
		INPUT_SOURCE_ANALOG,
		INPUT_SOURCE_LAST
	};

#endif

static u32 _i2cBytesTbl[] = {
    0,  // for offset
    I2C_1B, I2C_2B, I2C_3B, I2C_4B,
    I2C_5B, I2C_6B, I2C_7B, I2C_8B
};


/*****************************************************************************/
/*Extern  Variable 	   		                                     	     */
/*****************************************************************************/
#if GPIO_I2C_ENA
/* Gpio Pin Config */
extern GPIO_IIC_CFG GPIO_IIC_CFG_PT2257;
extern GPIO_IIC_CFG GPIO_IIC_CFG_TVP5150;
#endif

#if(TV_DECODER == WT8861)
extern u8	cam_check;
#endif



#if(HW_BOARD_OPTION==SALIX_SDV)
    extern u8 sysTVOutOnFlag;
    extern u8 TVorEarphone;
#endif

/*****************************************************************************/
/* Variable 	   		                                     	     */
/*****************************************************************************/

OS_EVENT* i2cSemReq;
OS_EVENT* i2cSemFin;
OS_EVENT* i2cWDTProtect;
#if SUPPORT_TOUCH
int uiTouchLevel=8000;
#endif
/*
   1.5dB step

   0x00: 12dB gain
   0x08: 0dB attenuation
   0x1F: 34.5dB attenuation

*/

const u8 DACVolumnTable[11]={0x1C,   // -28.5dB
                            0x19,   // -24  dB
                            0x16,   // -19.5dB
                            0x13,   // -15  dB
                            0x0F,   // -10.5dB
							0x0D,   // -7.5 dB
							0x0B,   // -4.5 dB
							0x09,   // -1.5 dB
							0x07,   // 	1.5 dB
							0x05,   //  4.5 dB
							0x03    //  7.5 dB
						};

/*
   1.5dB step

   0x00: 12dB gain
   0x08: 0dB attenuation
   0x1F: 34.5dB attenuation

*/

const u8 MICVolumeTable[10]={0x1f,   // -34.5dB
                            0x13,   // -15  dB
                            0x0F,   // -10.5dB
							0x0D,   // -7.5 dB
							0x0B,   // -4.5 dB
							0x09,   // -1.5 dB
							0x07,   // 	1.5 dB
							0x05,   //  4.5 dB
							0x03,   //  7.5 dB
							0x01    //  7.5 dB
						};

#if( (Sensor_OPTION == Sensor_MI9V136_YUV601) || (TV_DECODER == MI9V136) )
const DEF_IICDATA i2cpara_MI9V136[1507]=
            {
                 {0x0982, 0x0080}, {0x098A, 0x804D}, {0x098E, 0x000C}, {0x0990, 0x0000},
                 {0x0992, 0x0000}, {0x098E, 0x0008}, {0x0990, 0x0000}, {0x0992, 0x0000},
                 {0x098E, 0x7C00}, {0xFC00, 0x1000}, {0x0040, 0x8801}, {0x0040, 0x8305},
                 {0x0040, 0x8305}, {0x098E, 0x7C00}, {0xFC00, 0x0100}, {0xFC02, 0x0000},
                 {0x0040, 0x8300}, {0x098E, 0x7C00}, {0xFC00, 0x3000}, {0xFC02, 0x0000},
                 {0x0040, 0x8801}, {0x098E, 0x48CE}, {0xC8CE, 0x01EC}, {0xC8D0, 0xFF80},
                 {0xC8D2, 0x0026}, {0xC8D4, 0xFFC0}, {0xC8D6, 0x015B}, {0xC8D8, 0x004D},
                 {0xC8DA, 0xFFE6}, {0xC8DC, 0xFEB3}, {0xC8DE, 0x0247}, {0xC8E0, 0x0017},
                 {0xC8E2, 0x0052}, {0xC8E4, 0x0003}, {0xC8E6, 0xFFD2}, {0xC8E8, 0x0000},
                 {0xC8EA, 0x0017}, {0xC8EC, 0x006B}, {0xC8EE, 0xFFB3}, {0xC8F0, 0x003B},
                 {0xC8F2, 0x00A1}, {0xC8F4, 0xFF83}, {0xC8F6, 0x0013}, {0xC8F8, 0xFFD8},
                 {0xC910, 0x0003}, {0xC912, 0x0002}, {0xC914, 0x48C3}, {0xC916, 0x0CAD},
                 {0xC918, 0xFF97}, {0xC91A, 0x003D}, {0xC91C, 0x9103}, {0xC91E, 0xAFFE},
                 {0xC920, 0x402F}, {0xC922, 0x0000}, {0xC924, 0x004B}, {0xC926, 0x0039},
                 {0xC962, 0x0080}, {0xC964, 0x0088}, {0xC966, 0x0090}, {0xC968, 0x0080},
                 {0xC96A, 0x0088}, {0xC96C, 0x0088}, {0xAC40, 0x0032}, {0xAC42, 0x00C8},
                 {0xAC44, 0x001E}, {0xAC46, 0x00C8}, {0x301A, 0x10D0}, {0x3ED8, 0x0999},
                 {0x3E14, 0x6886}, {0x3E1A, 0x8507}, {0x3E1C, 0x8705}, {0x3E24, 0x9A10},
                 {0x3E26, 0x8F09}, {0x3E2A, 0x8060}, {0x3E2C, 0x6169}, {0x3ED0, 0x8F7F},
                 {0x301A, 0x10D4}, {0xA814, 0x0006}, {0xC89A, 0x0002}, {0x337C, 0x0007},
                 {0xC92E, 0x000B}, {0xC930, 0x000F}, {0xC932, 0x0078}, {0xC958, 0x000F},
                 {0xC95A, 0x0078}, {0xC95C, 0x005A}, {0xC95E, 0x01F4}, {0xC816, 0x0031},
                 {0xDC28, 0x0003}, {0xC950, 0x0064}, {0xC952, 0x0064}, {0xC954, 0x0064},
                 {0xC956, 0x0064}, {0xBC5C, 0x000F}, {0xBC5E, 0x0078}, {0xBC02, 0x0007},
                 {0xBC34, 0x0023}, {0xBC36, 0x002D}, {0x0982, 0x0001}, {0x098A, 0x69AC},
                 {0x0990, 0xC0F1}, {0x098A, 0x69AE}, {0x0990, 0x0AAE}, {0x098A, 0x69B0},
                 {0x0990, 0x0780}, {0x098A, 0x69B2}, {0x0990, 0x7508}, {0x098A, 0x69B4},
                 {0x0990, 0x8801}, {0x098A, 0x69B6}, {0x0990, 0xB872}, {0x098A, 0x69B8},
                 {0x0990, 0x0853}, {0x098A, 0x69BA}, {0x0990, 0x0135}, {0x098A, 0x69BC},
                 {0x0990, 0xC1A5}, {0x098A, 0x69BE}, {0x0990, 0x8D00}, {0x098A, 0x69C0},
                 {0x0990, 0x768B}, {0x098A, 0x69C2}, {0x0990, 0x0AF2}, {0x098A, 0x69C4},
                 {0x0990, 0x06A0}, {0x098A, 0x69C6}, {0x0990, 0x71C9}, {0x098A, 0x69C8},
                 {0x0990, 0x8D01}, {0x098A, 0x69CA}, {0x0990, 0xB872}, {0x098A, 0x69CC},
                 {0x0990, 0x0835}, {0x098A, 0x69CE}, {0x0990, 0x0115}, {0x098A, 0x69D0},
                 {0x0990, 0x2740}, {0x098A, 0x69D2}, {0x0990, 0x7282}, {0x098A, 0x69D4},
                 {0x0990, 0x7A15}, {0x098A, 0x69D6}, {0x0990, 0x7A20}, {0x098A, 0x69D8},
                 {0x0990, 0x9522}, {0x098A, 0x69DA}, {0x0990, 0x0013}, {0x098A, 0x69DC},
                 {0x0990, 0x0000}, {0x098A, 0x69DE}, {0x0990, 0x0015}, {0x098A, 0x69E0},
                 {0x0990, 0x0000}, {0x098A, 0x69E2}, {0x0990, 0x0017}, {0x098A, 0x69E4},
                 {0x0990, 0x0000}, {0x098A, 0x69E6}, {0x0990, 0x0019}, {0x098A, 0x69E8},
                 {0x0990, 0x0000}, {0x098A, 0x69EA}, {0x0990, 0x1C08}, {0x098A, 0x69EC},
                 {0x0990, 0x3044}, {0x098A, 0x69EE}, {0x0990, 0xF00A}, {0x098A, 0x69F0},
                 {0x0990, 0x1C0A}, {0x098A, 0x69F2}, {0x0990, 0x3044}, {0x098A, 0x69F4},
                 {0x0990, 0xF006}, {0x098A, 0x69F6}, {0x0990, 0x1C0C}, {0x098A, 0x69F8},
                 {0x0990, 0x3044}, {0x098A, 0x69FA}, {0x0990, 0xF004}, {0x098A, 0x69FC},
                 {0x0990, 0x1C0E}, {0x098A, 0x69FE}, {0x0990, 0x3044}, {0x098A, 0x6A00},
                 {0x0990, 0x8D00}, {0x098A, 0x6A02}, {0x0990, 0x0A4A}, {0x098A, 0x6A04},
                 {0x0990, 0x06A0}, {0x098A, 0x6A06}, {0x0990, 0x71C9}, {0x098A, 0x6A08},
                 {0x0990, 0xF004}, {0x098A, 0x6A0A}, {0x0990, 0x0996}, {0x098A, 0x6A0C},
                 {0x0990, 0x0320}, {0x098A, 0x6A0E}, {0x0990, 0x70A9}, {0x098A, 0x6A10},
                 {0x0990, 0x02B5}, {0x098A, 0x6A12}, {0x0990, 0x07A0}, {0x098A, 0x6A14},
                 {0x0990, 0xC0A5}, {0x098A, 0x6A16}, {0x0990, 0x78E0}, {0x098A, 0x6A18},
                 {0x0990, 0xC0F1}, {0x098A, 0x6A1A}, {0x0990, 0x0A3E}, {0x098A, 0x6A1C},
                 {0x0990, 0x07A0}, {0x098A, 0x6A1E}, {0x0990, 0xD804}, {0x098A, 0x6A20},
                 {0x0990, 0x1101}, {0x098A, 0x6A22}, {0x0990, 0x0084}, {0x098A, 0x6A24},
                 {0x0990, 0x7628}, {0x098A, 0x6A26}, {0x0990, 0x71CF}, {0x098A, 0x6A28},
                 {0x0990, 0x0008}, {0x098A, 0x6A2A}, {0x0990, 0x609A}, {0x098A, 0x6A2C},
                 {0x0990, 0xDA02}, {0x098A, 0x6A2E}, {0x0990, 0x08B6}, {0x098A, 0x6A30},
                 {0x0990, 0x0760}, {0x098A, 0x6A32}, {0x0990, 0x8E60}, {0x098A, 0x6A34},
                 {0x0990, 0x8E20}, {0x098A, 0x6A36}, {0x0990, 0x77CF}, {0x098A, 0x6A38},
                 {0x0990, 0x8000}, {0x098A, 0x6A3A}, {0x0990, 0x0504}, {0x098A, 0x6A3C},
                 {0x0990, 0xE184}, {0x098A, 0x6A3E}, {0x0990, 0x00A4}, {0x098A, 0x6A40},
                 {0x0990, 0x0029}, {0x098A, 0x6A42}, {0x0990, 0x21CA}, {0x098A, 0x6A44},
                 {0x0990, 0x0329}, {0x098A, 0x6A46}, {0x0990, 0x0D06}, {0x098A, 0x6A48},
                 {0x0990, 0x0760}, {0x098A, 0x6A4A}, {0x0990, 0x6974}, {0x098A, 0x6A4C},
                 {0x0990, 0x75CF}, {0x098A, 0x6A4E}, {0x0990, 0x8000}, {0x098A, 0x6A50},
                 {0x0990, 0x0080}, {0x098A, 0x6A52}, {0x0990, 0x8500}, {0x098A, 0x6A54},
                 {0x0990, 0xE001}, {0x098A, 0x6A56}, {0x0990, 0xA500}, {0x098A, 0x6A58},
                 {0x0990, 0x2756}, {0x098A, 0x6A5A}, {0x0990, 0x1280}, {0x098A, 0x6A5C},
                 {0x0990, 0x607A}, {0x098A, 0x6A5E}, {0x0990, 0x8A43}, {0x098A, 0x6A60},
                 {0x0990, 0x6078}, {0x098A, 0x6A62}, {0x0990, 0x0A27}, {0x098A, 0x6A64},
                 {0x0990, 0x0090}, {0x098A, 0x6A66}, {0x0990, 0x8862}, {0x098A, 0x6A68},
                 {0x0990, 0xE380}, {0x098A, 0x6A6A}, {0x0990, 0x21CA}, {0x098A, 0x6A6C},
                 {0x0990, 0x0061}, {0x098A, 0x6A6E}, {0x0990, 0xF234}, {0x098A, 0x6A70},
                 {0x0990, 0xE283}, {0x098A, 0x6A72}, {0x0990, 0xDB01}, {0x098A, 0x6A74},
                 {0x0990, 0x7BC0}, {0x098A, 0x6A76}, {0x0990, 0x0A17}, {0x098A, 0x6A78},
                 {0x0990, 0x00D1}, {0x098A, 0x6A7A}, {0x0990, 0x17A0}, {0x098A, 0x6A7C},
                 {0x0990, 0x1082}, {0x098A, 0x6A7E}, {0x0990, 0xE285}, {0x098A, 0x6A80},
                 {0x0990, 0x21CC}, {0x098A, 0x6A82}, {0x0990, 0x8082}, {0x098A, 0x6A84},
                 {0x0990, 0xF203}, {0x098A, 0x6A86}, {0x0990, 0xD909}, {0x098A, 0x6A88},
                 {0x0990, 0xF026}, {0x098A, 0x6A8A}, {0x0990, 0x8E21}, {0x098A, 0x6A8C},
                 {0x0990, 0x0919}, {0x098A, 0x6A8E}, {0x0990, 0x0110}, {0x098A, 0x6A90},
                 {0x0990, 0x091B}, {0x098A, 0x6A92}, {0x0990, 0x0390}, {0x098A, 0x6A94},
                 {0x0990, 0x0931}, {0x098A, 0x6A96}, {0x0990, 0x03D0}, {0x098A, 0x6A98},
                 {0x0990, 0xEB1B}, {0x098A, 0x6A9A}, {0x0990, 0x70E9}, {0x098A, 0x6A9C},
                 {0x0990, 0x0FE6}, {0x098A, 0x6A9E}, {0x0990, 0x02E0}, {0x098A, 0x6AA0},
                 {0x0990, 0x71C9}, {0x098A, 0x6AA2}, {0x0990, 0xF019}, {0x098A, 0x6AA4},
                 {0x0990, 0x9622}, {0x098A, 0x6AA6}, {0x0990, 0xB020}, {0x098A, 0x6AA8},
                 {0x0990, 0xF008}, {0x098A, 0x6AAA}, {0x0990, 0x8E24}, {0x098A, 0x6AAC},
                 {0x0990, 0x6139}, {0x098A, 0x6AAE}, {0x0990, 0x7935}, {0x098A, 0x6AB0},
                 {0x0990, 0x262F}, {0x098A, 0x6AB2}, {0x0990, 0xF048}, {0x098A, 0x6AB4},
                 {0x0990, 0xB024}, {0x098A, 0x6AB6}, {0x0990, 0xF404}, {0x098A, 0x6AB8},
                 {0x0990, 0xD900}, {0x098A, 0x6ABA}, {0x0990, 0xF00E}, {0x098A, 0x6ABC},
                 {0x0990, 0x9045}, {0x098A, 0x6ABE}, {0x0990, 0xEAFE}, {0x098A, 0x6AC0},
                 {0x0990, 0xB025}, {0x098A, 0x6AC2}, {0x0990, 0xF1FC}, {0x098A, 0x6AC4},
                 {0x0990, 0x8E24}, {0x098A, 0x6AC6}, {0x0990, 0x6139}, {0x098A, 0x6AC8},
                 {0x0990, 0x7935}, {0x098A, 0x6ACA}, {0x0990, 0xB023}, {0x098A, 0x6ACC},
                 {0x0990, 0xF1F6}, {0x098A, 0x6ACE}, {0x0990, 0x70C9}, {0x098A, 0x6AD0},
                 {0x0990, 0xFFB7}, {0x098A, 0x6AD2}, {0x0990, 0x7108}, {0x098A, 0x6AD4},
                 {0x0990, 0x8500}, {0x098A, 0x6AD6}, {0x0990, 0x2080}, {0x098A, 0x6AD8},
                 {0x0990, 0x8FFF}, {0x098A, 0x6ADA}, {0x0990, 0xA500}, {0x098A, 0x6ADC},
                 {0x0990, 0x0C9C}, {0x098A, 0x6ADE}, {0x0990, 0x0741}, {0x098A, 0x6AE0},
                 {0x0990, 0x01DD}, {0x098A, 0x6AE2}, {0x0990, 0x07A0}, {0x098A, 0x6AE4},
                 {0x0990, 0x7028}, {0x098A, 0x6AE6}, {0x0990, 0x78E0}, {0x098A, 0x6AE8},
                 {0x0990, 0xC0F1}, {0x098A, 0x6AEA}, {0x0990, 0x096A}, {0x098A, 0x6AEC},
                 {0x0990, 0x07A0}, {0x098A, 0x6AEE}, {0x0990, 0xDA02}, {0x098A, 0x6AF0},
                 {0x0990, 0xC1A6}, {0x098A, 0x6AF2}, {0x0990, 0x1101}, {0x098A, 0x6AF4},
                 {0x0990, 0x0084}, {0x098A, 0x6AF6}, {0x0990, 0x701A}, {0x098A, 0x6AF8},
                 {0x0990, 0x7528}, {0x098A, 0x6AFA}, {0x0990, 0xD804}, {0x098A, 0x6AFC},
                 {0x0990, 0x71CF}, {0x098A, 0x6AFE}, {0x0990, 0x0008}, {0x098A, 0x6B00},
                 {0x0990, 0x6105}, {0x098A, 0x6B02}, {0x0990, 0x0FE2}, {0x098A, 0x6B04},
                 {0x0990, 0x0720}, {0x098A, 0x6B06}, {0x0990, 0x8D60}, {0x098A, 0x6B08},
                 {0x0990, 0x8D00}, {0x098A, 0x6B0A}, {0x0990, 0x768B}, {0x098A, 0x6B0C},
                 {0x0990, 0xE084}, {0x098A, 0x6B0E}, {0x0990, 0x00C4}, {0x098A, 0x6B10},
                 {0x0990, 0x0029}, {0x098A, 0x6B12}, {0x0990, 0x20CA}, {0x098A, 0x6B14},
                 {0x0990, 0x2329}, {0x098A, 0x6B16}, {0x0990, 0x0C36}, {0x098A, 0x6B18},
                 {0x0990, 0x0740}, {0x098A, 0x6B1A}, {0x0990, 0x77CF}, {0x098A, 0x6B1C},
                 {0x0990, 0x8000}, {0x098A, 0x6B1E}, {0x0990, 0x0080}, {0x098A, 0x6B20},
                 {0x0990, 0x8740}, {0x098A, 0x6B22}, {0x0990, 0x71CF}, {0x098A, 0x6B24},
                 {0x0990, 0x8000}, {0x098A, 0x6B26}, {0x0990, 0x0556}, {0x098A, 0x6B28},
                 {0x0990, 0x6A01}, {0x098A, 0x6B2A}, {0x0990, 0xA700}, {0x098A, 0x6B2C},
                 {0x0990, 0x8D00}, {0x098A, 0x6B2E}, {0x0990, 0x6874}, {0x098A, 0x6B30},
                 {0x0990, 0x6169}, {0x098A, 0x6B32}, {0x0990, 0xE180}, {0x098A, 0x6B34},
                 {0x0990, 0x20CA}, {0x098A, 0x6B36}, {0x0990, 0x2061}, {0x098A, 0x6B38},
                 {0x0990, 0xF23C}, {0x098A, 0x6B3A}, {0x0990, 0x8D21}, {0x098A, 0x6B3C},
                 {0x0990, 0xB971}, {0x098A, 0x6B3E}, {0x0990, 0x0983}, {0x098A, 0x6B40},
                 {0x0990, 0x0155}, {0x098A, 0x6B42}, {0x0990, 0x0972}, {0x098A, 0x6B44},
                 {0x0990, 0x06A0}, {0x098A, 0x6B46}, {0x0990, 0xC181}, {0x098A, 0x6B48},
                 {0x0990, 0x701A}, {0x098A, 0x6B4A}, {0x0990, 0x8D01}, {0x098A, 0x6B4C},
                 {0x0990, 0xB871}, {0x098A, 0x6B4E}, {0x0990, 0x085B}, {0x098A, 0x6B50},
                 {0x0990, 0x0155}, {0x098A, 0x6B52}, {0x0990, 0x2740}, {0x098A, 0x6B54},
                 {0x0990, 0x7281}, {0x098A, 0x6B56}, {0x0990, 0x7915}, {0x098A, 0x6B58},
                 {0x0990, 0x7900}, {0x098A, 0x6B5A}, {0x0990, 0x0037}, {0x098A, 0x6B5C},
                 {0x0990, 0x0000}, {0x098A, 0x6B5E}, {0x0990, 0x0013}, {0x098A, 0x6B60},
                 {0x0990, 0x0000}, {0x098A, 0x6B62}, {0x0990, 0x0017}, {0x098A, 0x6B64},
                 {0x0990, 0x0000}, {0x098A, 0x6B66}, {0x0990, 0x001B}, {0x098A, 0x6B68},
                 {0x0990, 0x0000}, {0x098A, 0x6B6A}, {0x0990, 0x001F}, {0x098A, 0x6B6C},
                 {0x0990, 0x0000}, {0x098A, 0x6B6E}, {0x0990, 0x140C}, {0x098A, 0x6B70},
                 {0x0990, 0x3100}, {0x098A, 0x6B72}, {0x0990, 0xB600}, {0x098A, 0x6B74},
                 {0x0990, 0xF019}, {0x098A, 0x6B76}, {0x0990, 0x140E}, {0x098A, 0x6B78},
                 {0x0990, 0x3100}, {0x098A, 0x6B7A}, {0x0990, 0xB600}, {0x098A, 0x6B7C},
                 {0x0990, 0xF015}, {0x098A, 0x6B7E}, {0x0990, 0x1410}, {0x098A, 0x6B80},
                 {0x0990, 0x3100}, {0x098A, 0x6B82}, {0x0990, 0xB600}, {0x098A, 0x6B84},
                 {0x0990, 0xF011}, {0x098A, 0x6B86}, {0x0990, 0x1412}, {0x098A, 0x6B88},
                 {0x0990, 0x3100}, {0x098A, 0x6B8A}, {0x0990, 0xB600}, {0x098A, 0x6B8C},
                 {0x0990, 0xF00D}, {0x098A, 0x6B8E}, {0x0990, 0xC005}, {0x098A, 0x6B90},
                 {0x0990, 0xB8FE}, {0x098A, 0x6B92}, {0x0990, 0xB83D}, {0x098A, 0x6B94},
                 {0x0990, 0x20D2}, {0x098A, 0x6B96}, {0x0990, 0x0022}, {0x098A, 0x6B98},
                 {0x0990, 0x20D3}, {0x098A, 0x6B9A}, {0x0990, 0x0022}, {0x098A, 0x6B9C},
                 {0x0990, 0x20C0}, {0x098A, 0x6B9E}, {0x0990, 0x0062}, {0x098A, 0x6BA0},
                 {0x0990, 0x20CA}, {0x098A, 0x6BA2}, {0x0990, 0x0021}, {0x098A, 0x6BA4},
                 {0x0990, 0xAE00}, {0x098A, 0x6BA6}, {0x0990, 0x70A9}, {0x098A, 0x6BA8},
                 {0x0990, 0x71C9}, {0x098A, 0x6BAA}, {0x0990, 0x0ECA}, {0x098A, 0x6BAC},
                 {0x0990, 0x0760}, {0x098A, 0x6BAE}, {0x0990, 0xDA04}, {0x098A, 0x6BB0},
                 {0x0990, 0x8700}, {0x098A, 0x6BB2}, {0x0990, 0x2080}, {0x098A, 0x6BB4},
                 {0x0990, 0x8FFF}, {0x098A, 0x6BB6}, {0x0990, 0xA700}, {0x098A, 0x6BB8},
                 {0x0990, 0x0BC0}, {0x098A, 0x6BBA}, {0x0990, 0x0741}, {0x098A, 0x6BBC},
                 {0x0990, 0xF00A}, {0x098A, 0x6BBE}, {0x0990, 0xE280}, {0x098A, 0x6BC0},
                 {0x0990, 0xA740}, {0x098A, 0x6BC2}, {0x0990, 0x0BB8}, {0x098A, 0x6BC4},
                 {0x0990, 0x0741}, {0x098A, 0x6BC6}, {0x0990, 0x700A}, {0x098A, 0x6BC8},
                 {0x0990, 0x098A}, {0x098A, 0x6BCA}, {0x0990, 0x0320}, {0x098A, 0x6BCC},
                 {0x0990, 0x71A9}, {0x098A, 0x6BCE}, {0x0990, 0x701A}, {0x098A, 0x6BD0},
                 {0x0990, 0x700A}, {0x098A, 0x6BD2}, {0x0990, 0x00E5}, {0x098A, 0x6BD4},
                 {0x0990, 0x07A0}, {0x098A, 0x6BD6}, {0x0990, 0xC0A6}, {0x098A, 0x6BD8},
                 {0x0990, 0xC0F1}, {0x098A, 0x6BDA}, {0x0990, 0x0882}, {0x098A, 0x6BDC},
                 {0x0990, 0x0780}, {0x098A, 0x6BDE}, {0x0990, 0x0AB6}, {0x098A, 0x6BE0},
                 {0x0990, 0x0120}, {0x098A, 0x6BE2}, {0x0990, 0xC1A4}, {0x098A, 0x6BE4},
                 {0x0990, 0x0FD6}, {0x098A, 0x6BE6}, {0x0990, 0x03C0}, {0x098A, 0x6BE8},
                 {0x0990, 0x260A}, {0x098A, 0x6BEA}, {0x0990, 0x9000}, {0x098A, 0x6BEC},
                 {0x0990, 0x26CC}, {0x098A, 0x6BEE}, {0x0990, 0x91A2}, {0x098A, 0x6BF0},
                 {0x0990, 0xF41B}, {0x098A, 0x6BF2}, {0x0990, 0xE680}, {0x098A, 0x6BF4},
                 {0x0990, 0xDD01}, {0x098A, 0x6BF6}, {0x0990, 0x71CF}, {0x098A, 0x6BF8},
                 {0x0990, 0xFFFF}, {0x098A, 0x6BFA}, {0x0990, 0xEA18}, {0x098A, 0x6BFC},
                 {0x0990, 0x70CF}, {0x098A, 0x6BFE}, {0x0990, 0x0000}, {0x098A, 0x6C00},
                 {0x0990, 0x8203}, {0x098A, 0x6C02}, {0x0990, 0x0D4A}, {0x098A, 0x6C04},
                 {0x0990, 0x0120}, {0x098A, 0x6C06}, {0x0990, 0x7DC0}, {0x098A, 0x6C08},
                 {0x0990, 0x71CF}, {0x098A, 0x6C0A}, {0x0990, 0xFFFF}, {0x098A, 0x6C0C},
                 {0x0990, 0xEAE8}, {0x098A, 0x6C0E}, {0x0990, 0x70CF}, {0x098A, 0x6C10},
                 {0x0990, 0x0000}, {0x098A, 0x6C12}, {0x0990, 0x8204}, {0x098A, 0x6C14},
                 {0x0990, 0x0D36}, {0x098A, 0x6C16}, {0x0990, 0x0100}, {0x098A, 0x6C18},
                 {0x0990, 0x7608}, {0x098A, 0x6C1A}, {0x0990, 0xED07}, {0x098A, 0x6C1C},
                 {0x0990, 0x0A76}, {0x098A, 0x6C1E}, {0x0990, 0x0100}, {0x098A, 0x6C20},
                 {0x0990, 0x0FD2}, {0x098A, 0x6C22}, {0x0990, 0x03E0}, {0x098A, 0x6C24},
                 {0x0990, 0xD900}, {0x098A, 0x6C26}, {0x0990, 0xEE13}, {0x098A, 0x6C28},
                 {0x0990, 0x758B}, {0x098A, 0x6C2A}, {0x0990, 0x70A9}, {0x098A, 0x6C2C},
                 {0x0990, 0x71CF}, {0x098A, 0x6C2E}, {0x0990, 0xFFFF}, {0x098A, 0x6C30},
                 {0x0990, 0xEC54}, {0x098A, 0x6C32}, {0x0990, 0x0E42}, {0x098A, 0x6C34},
                 {0x0990, 0x0760}, {0x098A, 0x6C36}, {0x0990, 0xDA10}, {0x098A, 0x6C38},
                 {0x0990, 0x70CF}, {0x098A, 0x6C3A}, {0x0990, 0xFFFF}, {0x098A, 0x6C3C},
                 {0x0990, 0xE050}, {0x098A, 0x6C3E}, {0x0990, 0x8003}, {0x098A, 0x6C40},
                 {0x0990, 0xE080}, {0x098A, 0x6C42}, {0x0990, 0x0C18}, {0x098A, 0x6C44},
                 {0x0990, 0x0722}, {0x098A, 0x6C46}, {0x0990, 0x20CA}, {0x098A, 0x6C48},
                 {0x0990, 0x0342}, {0x098A, 0x6C4A}, {0x0990, 0x70C9}, {0x098A, 0x6C4C},
                 {0x0990, 0x0079}, {0x098A, 0x6C4E}, {0x0990, 0x07A0}, {0x098A, 0x6C50},
                 {0x0990, 0xC0A4}, {0x098A, 0x6C52}, {0x0990, 0x0000}, {0x098A, 0x6C54},
                 {0x0990, 0x4F56}, {0x098A, 0x6C56}, {0x0990, 0x7074}, {0x098A, 0x6C58},
                 {0x0990, 0x2066}, {0x098A, 0x6C50}, {0x0990, 0x3272}, {0x098A, 0x6C5C},
                 {0x0990, 0x6567}, {0x098A, 0x6C5E}, {0x0990, 0x2063}, {0x098A, 0x6C60},
                 {0x0990, 0x6D64}, {0x098A, 0x6C62}, {0x0990, 0x7300}, {0x098E, 0x7C37},
                 {0x098E, 0x7C00}, {0x0990, 0x0BD8}, {0x0992, 0x0211}, {0x0994, 0x0103},
                 {0x0996, 0x0611}, {0x0998, 0x02B8}, {0x0040, 0x8702}, {0x0982, 0x0001},
                 {0x098A, 0x753C}, {0x0990, 0x71CF}, {0x098A, 0x753E}, {0x0990, 0x8000},
                 {0x098A, 0x7540}, {0x0990, 0x03B4}, {0x098A, 0x7542}, {0x0990, 0x810A},
                 {0x098A, 0x7544}, {0x0990, 0x72CF}, {0x098A, 0x7546}, {0x0990, 0xFFFF},
                 {0x098A, 0x7548}, {0x0990, 0xF670}, {0x098A, 0x754A}, {0x0990, 0xB8BE},
                 {0x098A, 0x754C}, {0x0990, 0xB89E}, {0x098A, 0x754E}, {0x0990, 0xA10A},
                 {0x098A, 0x7550}, {0x0990, 0xD800}, {0x098A, 0x7552}, {0x0990, 0x2215},
                 {0x098A, 0x7554}, {0x0990, 0x0001}, {0x098A, 0x7556}, {0x0990, 0x8161},
                 {0x098A, 0x7558}, {0x0990, 0xE002}, {0x098A, 0x755A}, {0x0990, 0x8120},
                 {0x098A, 0x755C}, {0x0990, 0x08F7}, {0x098A, 0x755E}, {0x0990, 0x80B4},
                 {0x098A, 0x7560}, {0x0990, 0xA160}, {0x098A, 0x7562}, {0x0990, 0x7FE0},
                 {0x098A, 0x7564}, {0x0990, 0xD800}, {0x098A, 0x7566}, {0x0990, 0x78E0},
                 {0x098A, 0x7568}, {0x0990, 0xC0F1}, {0x098A, 0x756A}, {0x0990, 0x0EF2},
                 {0x098A, 0x756C}, {0x0990, 0x0720}, {0x098A, 0x756E}, {0x0990, 0xDA00},
                 {0x098A, 0x7570}, {0x0990, 0x7608}, {0x098A, 0x7572}, {0x0990, 0x71CF},
                 {0x098A, 0x7574}, {0x0990, 0x0008}, {0x098A, 0x7576}, {0x0990, 0xC04D},
                 {0x098A, 0x7578}, {0x0990, 0x0D6A}, {0x098A, 0x757A}, {0x0990, 0x06E0},
                 {0x098A, 0x757C}, {0x0990, 0xD808}, {0x098A, 0x757E}, {0x0990, 0x09CE},
                 {0x098A, 0x7580}, {0x0990, 0x0700}, {0x098A, 0x7582}, {0x0990, 0x75CF},
                 {0x098A, 0x7584}, {0x0990, 0x8000}, {0x098A, 0x7586}, {0x0990, 0x0080},
                 {0x098A, 0x7588}, {0x0990, 0x8500}, {0x098A, 0x758A}, {0x0990, 0xE001},
                 {0x098A, 0x758C}, {0x0990, 0x862A}, {0x098A, 0x758E}, {0x0990, 0xA500},
                 {0x098A, 0x7590}, {0x0990, 0xB9FB}, {0x098A, 0x7592}, {0x0990, 0x29C1},
                 {0x098A, 0x7594}, {0x0990, 0x0721}, {0x098A, 0x7596}, {0x0990, 0x20CA},
                 {0x098A, 0x7598}, {0x0990, 0x0061}, {0x098A, 0x759A}, {0x0990, 0x0AF8},
                 {0x098A, 0x759C}, {0x0990, 0x05E1}, {0x098A, 0x759E}, {0x0990, 0x21D3},
                 {0x098A, 0x75A0}, {0x0990, 0x0021}, {0x098A, 0x75A2}, {0x0990, 0x860A},
                 {0x098A, 0x75A4}, {0x0990, 0x081D}, {0x098A, 0x75A6}, {0x0990, 0x061E},
                 {0x098A, 0x75A8}, {0x0990, 0xD802}, {0x098A, 0x75AA}, {0x0990, 0x0BA6},
                 {0x098A, 0x75AC}, {0x0990, 0x05E0}, {0x098A, 0x75AE}, {0x0990, 0xD900},
                 {0x098A, 0x75B0}, {0x0990, 0xD802}, {0x098A, 0x75B2}, {0x0990, 0x0B42},
                 {0x098A, 0x75B4}, {0x0990, 0x05E0}, {0x098A, 0x75B6}, {0x0990, 0xD901},
                 {0x098A, 0x75B8}, {0x0990, 0x0B22}, {0x098A, 0x75BA}, {0x0990, 0x05E0},
                 {0x098A, 0x75BC}, {0x0990, 0xD801}, {0x098A, 0x75BE}, {0x0990, 0xF006},
                 {0x098A, 0x75C0}, {0x0990, 0x70C9}, {0x098A, 0x75C2}, {0x0990, 0x0C0A},
                 {0x098A, 0x75C4}, {0x0990, 0x0160}, {0x098A, 0x75C6}, {0x0990, 0xD909},
                 {0x098A, 0x75C8}, {0x0990, 0x8500}, {0x098A, 0x75CA}, {0x0990, 0x2080},
                 {0x098A, 0x75CC}, {0x0990, 0x8FFF}, {0x098A, 0x75CE}, {0x0990, 0xA500},
                 {0x098A, 0x75D0}, {0x0990, 0x09A8}, {0x098A, 0x75D2}, {0x0990, 0x0701},
                 {0x098A, 0x75D4}, {0x0990, 0x06F1}, {0x098A, 0x75D6}, {0x0990, 0x0700},
                 {0x098A, 0x75D8}, {0x0990, 0xC0F1}, {0x098A, 0x75DA}, {0x0990, 0x8829},
                 {0x098A, 0x75DC}, {0x0990, 0x090F}, {0x098A, 0x75DE}, {0x0990, 0x0211},
                 {0x098A, 0x75E0}, {0x0990, 0xA828}, {0x098A, 0x75E2}, {0x0990, 0xD900},
                 {0x098A, 0x75E4}, {0x0990, 0xA829}, {0x098A, 0x75E6}, {0x0990, 0xFFE1},
                 {0x098A, 0x75E8}, {0x0990, 0xF003}, {0x098A, 0x75EA}, {0x0990, 0x0FB2},
                 {0x098A, 0x75EC}, {0x0990, 0x0140}, {0x098A, 0x75EE}, {0x0990, 0xC0D1},
                 {0x098A, 0x75F0}, {0x0990, 0x7EE0}, {0x098A, 0x75F2}, {0x0990, 0x78E0},
                 {0x098A, 0x75F4}, {0x0990, 0xC0F1}, {0x098A, 0x75F6}, {0x0990, 0xC5E1},
                 {0x098A, 0x75F8}, {0x0990, 0x0AD6}, {0x098A, 0x75FA}, {0x0990, 0x03A0},
                 {0x098A, 0x75FC}, {0x0990, 0x7508}, {0x098A, 0x75FE}, {0x0990, 0x8521},
                 {0x098A, 0x7600}, {0x0990, 0xE907}, {0x098A, 0x7602}, {0x0990, 0x080F},
                 {0x098A, 0x7604}, {0x0990, 0x0041}, {0x098A, 0x7606}, {0x0990, 0x0B6A},
                 {0x098A, 0x7608}, {0x0990, 0x0160}, {0x098A, 0x760A}, {0x0990, 0x70A9},
                 {0x098A, 0x760C}, {0x0990, 0xF029}, {0x098A, 0x760E}, {0x0990, 0x8520},
                 {0x098A, 0x7610}, {0x0990, 0x082D}, {0x098A, 0x7612}, {0x0990, 0x0041},
                 {0x098A, 0x7614}, {0x0990, 0x1520}, {0x098A, 0x7616}, {0x0990, 0x1080},
                 {0x098A, 0x7618}, {0x0990, 0x080D}, {0x098A, 0x761A}, {0x0990, 0x00D1},
                 {0x098A, 0x761C}, {0x0990, 0x0AD6}, {0x098A, 0x761E}, {0x0990, 0x01A0},
                 {0x098A, 0x7620}, {0x0990, 0x70A9}, {0x098A, 0x7622}, {0x0990, 0xF01F},
                 {0x098A, 0x7624}, {0x0990, 0x8D30}, {0x098A, 0x7626}, {0x0990, 0x70A9},
                 {0x098A, 0x7628}, {0x0990, 0x0911}, {0x098A, 0x762A}, {0x0990, 0x00D1},
                 {0x098A, 0x762C}, {0x0990, 0x0912}, {0x098A, 0x762E}, {0x0990, 0x0180},
                 {0x098A, 0x7630}, {0x0990, 0x0BE6}, {0x098A, 0x7632}, {0x0990, 0x0160},
                 {0x098A, 0x7634}, {0x0990, 0x70A9}, {0x098A, 0x7636}, {0x0990, 0xF015},
                 {0x098A, 0x7638}, {0x0990, 0xFFE8}, {0x098A, 0x763A}, {0x0990, 0xF013},
                 {0x098A, 0x763C}, {0x0990, 0xE88B}, {0x098A, 0x763E}, {0x0990, 0xD801},
                 {0x098A, 0x7640}, {0x0990, 0xAD08}, {0x098A, 0x7642}, {0x0990, 0xD808},
                 {0x098A, 0x7644}, {0x0990, 0x71CF}, {0x098A, 0x7646}, {0x0990, 0x0008},
                 {0x098A, 0x7648}, {0x0990, 0xC0C7}, {0x098A, 0x764A}, {0x0990, 0x0C9A},
                 {0x098A, 0x764C}, {0x0990, 0x06E0}, {0x098A, 0x764E}, {0x0990, 0xDA00},
                 {0x098A, 0x7650}, {0x0990, 0xF007}, {0x098A, 0x7652}, {0x0990, 0x71CF},
                 {0x098A, 0x7654}, {0x0990, 0xFFFF}, {0x098A, 0x7656}, {0x0990, 0xF66C},
                 {0x098A, 0x7658}, {0x0990, 0x0BCA}, {0x098A, 0x765A}, {0x0990, 0x0060},
                 {0x098A, 0x765C}, {0x0990, 0xD8B0}, {0x098A, 0x765E}, {0x0990, 0x70A9},
                 {0x098A, 0x7660}, {0x0990, 0x0C8A}, {0x098A, 0x7662}, {0x0990, 0x03A0},
                 {0x098A, 0x7664}, {0x0990, 0xD902}, {0x098A, 0x7666}, {0x0990, 0x0669},
                 {0x098A, 0x7668}, {0x0990, 0x0700}, {0x098A, 0x766A}, {0x0990, 0x0000},
                 {0x098A, 0x766C}, {0x0990, 0x0990}, {0x098A, 0x766E}, {0x0990, 0x0000},
                 {0x098A, 0x7670}, {0x0990, 0x0000}, {0x098A, 0x7672}, {0x0990, 0x8000},
                 {0x098A, 0x7674}, {0x0990, 0x013C}, {0x098A, 0x7676}, {0x0990, 0xFFFF},
                 {0x098A, 0x7678}, {0x0990, 0xF5F4}, {0x098E, 0x7C00}, {0x098E, 0x7C00},
                 {0x0990, 0x153C}, {0x0992, 0x0511}, {0x0994, 0x0103}, {0x0996, 0x0611},
                 {0x0998, 0x013C}, {0x0040, 0x8702}, {0x0982, 0x0001}, {0x098A, 0x7678},
                 {0x0990, 0x72CF}, {0x098A, 0x767A}, {0x0990, 0x8000}, {0x098A, 0x767C},
                 {0x0990, 0x02B0}, {0x098A, 0x767E}, {0x0990, 0x8240}, {0x098A, 0x7680},
                 {0x0990, 0x9269}, {0x098A, 0x7682}, {0x0990, 0x080B}, {0x098A, 0x7684},
                 {0x0990, 0x00C2}, {0x098A, 0x7686}, {0x0990, 0x6B09}, {0x098A, 0x7688},
                 {0x0990, 0x7810}, {0x098A, 0x768A}, {0x0990, 0x9268}, {0x098A, 0x768C},
                 {0x0990, 0x2109}, {0x098A, 0x768E}, {0x0990, 0x00C1}, {0x098A, 0x7690},
                 {0x0990, 0x9267}, {0x098A, 0x7692}, {0x0990, 0x2108}, {0x098A, 0x7694},
                 {0x0990, 0x00C1}, {0x098A, 0x7696}, {0x0990, 0x72CF}, {0x098A, 0x7698},
                 {0x0990, 0xFFFF}, {0x098A, 0x769A}, {0x0990, 0xE118}, {0x098A, 0x769C},
                 {0x0990, 0x9258}, {0x098A, 0x769E}, {0x0990, 0xE280}, {0x098A, 0x76A0},
                 {0x0990, 0x21CA}, {0x098A, 0x76A2}, {0x0990, 0x00C2}, {0x098A, 0x76A4},
                 {0x0990, 0x0041}, {0x098A, 0x76A6}, {0x0990, 0x0540}, {0x098A, 0x76A8},
                 {0x0990, 0xD800}, {0x098A, 0x76AA}, {0x0990, 0x73CF}, {0x098A, 0x76AC},
                 {0x0990, 0xFFFF}, {0x098A, 0x76AE}, {0x0990, 0xF6D4}, {0x098A, 0x76B0},
                 {0x0990, 0x2315}, {0x098A, 0x76B2}, {0x0990, 0x0001}, {0x098A, 0x76B4},
                 {0x0990, 0x8141}, {0x098A, 0x76B6}, {0x0990, 0xE002}, {0x098A, 0x76B8},
                 {0x0990, 0x8120}, {0x098A, 0x76BA}, {0x0990, 0x08F9}, {0x098A, 0x76BC},
                 {0x0990, 0x80B4}, {0x098A, 0x76BE}, {0x0990, 0xA140}, {0x098A, 0x76C0},
                 {0x0990, 0x70CF}, {0x098A, 0x76C2}, {0x0990, 0x8000}, {0x098A, 0x76C4},
                 {0x0990, 0x02B0}, {0x098A, 0x76C6}, {0x0990, 0x8000}, {0x098A, 0x76C8},
                 {0x0990, 0x9027}, {0x098A, 0x76CA}, {0x0990, 0x70CF}, {0x098A, 0x76CC},
                 {0x0990, 0xFF00}, {0x098A, 0x76CE}, {0x0990, 0x300C}, {0x098A, 0x76D0},
                 {0x0990, 0xB024}, {0x098A, 0x76D2}, {0x0990, 0x7EE0}, {0x098A, 0x76D4},
                 {0x0990, 0x8000}, {0x098A, 0x76D6}, {0x0990, 0x0238}, {0x098A, 0x76D8},
                 {0x0990, 0xFFFF}, {0x098A, 0x76DA}, {0x0990, 0xF678}, {0x098E, 0x7C00},
                 {0x098E, 0x7C00}, {0x0990, 0x16A8}, {0x0992, 0x0611}, {0x0994, 0x0103},
                 {0x0996, 0x0611}, {0x0998, 0x0064}, {0x0040, 0x8702}, {0x0982, 0x0001},
                 {0x098A, 0x76DC}, {0x0990, 0xC0F1}, {0x098A, 0x76DE}, {0x0990, 0x0D7A},
                 {0x098A, 0x76E0}, {0x0990, 0x0700}, {0x098A, 0x76E2}, {0x0990, 0x70CF},
                 {0x098A, 0x76E4}, {0x0990, 0x0000}, {0x098A, 0x76E6}, {0x0990, 0x836A},
                 {0x098A, 0x76E8}, {0x0990, 0x7840}, {0x098A, 0x76EA}, {0x0990, 0x77CF},
                 {0x098A, 0x76EC}, {0x0990, 0xFFFF}, {0x098A, 0x76EE}, {0x0990, 0xE15C},
                 {0x098A, 0x76F0}, {0x0990, 0x97B9}, {0x098A, 0x76F2}, {0x0990, 0x258C},
                 {0x098A, 0x76F4}, {0x0990, 0x9002}, {0x098A, 0x76F6}, {0x0990, 0xF698},
                 {0x098A, 0x76F8}, {0x0990, 0xD900}, {0x098A, 0x76FA}, {0x0990, 0x2714},
                 {0x098A, 0x76FC}, {0x0990, 0x104E}, {0x098A, 0x76FE}, {0x0990, 0x72CF},
                 {0x098A, 0x7700}, {0x0990, 0x0000}, {0x098A, 0x7702}, {0x0990, 0xE44C},
                 {0x098A, 0x7704}, {0x0990, 0x22F5}, {0x098A, 0x7706}, {0x0990, 0x0042},
                 {0x098A, 0x7708}, {0x0990, 0x9E05}, {0x098A, 0x770A}, {0x0990, 0xE101},
                 {0x098A, 0x770C}, {0x0990, 0x792F}, {0x098A, 0x770E}, {0x0990, 0x7842},
                 {0x098A, 0x7710}, {0x0990, 0x78AC}, {0x098A, 0x7712}, {0x0990, 0x2942},
                 {0x098A, 0x7714}, {0x0990, 0x7183}, {0x098A, 0x7716}, {0x0990, 0xBB39},
                 {0x098A, 0x7718}, {0x0990, 0x7327}, {0x098A, 0x771A}, {0x0990, 0xBB47},
                 {0x098A, 0x771C}, {0x0990, 0x627A}, {0x098A, 0x771E}, {0x0990, 0x09DF},
                 {0x098A, 0x7720}, {0x0990, 0x8272}, {0x098A, 0x7722}, {0x0990, 0xB645},
                 {0x098A, 0x7724}, {0x0990, 0x0599}, {0x098A, 0x7726}, {0x0990, 0x0700},
                 {0x098A, 0x7728}, {0x0990, 0xD800}, {0x098A, 0x772A}, {0x0990, 0x73CF},
                 {0x098A, 0x772C}, {0x0990, 0xFFFF}, {0x098A, 0x772E}, {0x0990, 0xF744},
                 {0x098A, 0x7730}, {0x0990, 0x2315}, {0x098A, 0x7732}, {0x0990, 0x0001},
                 {0x098A, 0x7734}, {0x0990, 0x8141}, {0x098A, 0x7736}, {0x0990, 0xE002},
                 {0x098A, 0x7738}, {0x0990, 0x8120}, {0x098A, 0x773A}, {0x0990, 0x08F9},
                 {0x098A, 0x773C}, {0x0990, 0x80B4}, {0x098A, 0x773E}, {0x0990, 0xA140},
                 {0x098A, 0x7740}, {0x0990, 0x7EE0}, {0x098A, 0x7742}, {0x0990, 0x0000},
                 {0x098A, 0x7744}, {0x0990, 0x8000}, {0x098A, 0x7746}, {0x0990, 0x0208},
                 {0x098A, 0x7748}, {0x0990, 0xFFFF}, {0x098A, 0x774A}, {0x0990, 0xF6DC},
                 {0x098E, 0x7C00}, {0x098E, 0x7C00}, {0x0990, 0x1728}, {0x0992, 0x0711},
                 {0x0994, 0x0103}, {0x0996, 0x0611}, {0x0998, 0x0070}, {0x0040, 0x8702},
                 {0x0982, 0x0001}, {0x098A, 0x77A8}, {0x0990, 0xC0F1}, {0x098A, 0x77AA},
                 {0x0990, 0x0CAE}, {0x098A, 0x77AC}, {0x0990, 0x0700}, {0x098A, 0x77AE},
                 {0x0990, 0x70CF}, {0x098A, 0x77B0}, {0x0990, 0x0000}, {0x098A, 0x77B2},
                 {0x0990, 0x8462}, {0x098A, 0x77B4}, {0x0990, 0x7840}, {0x098A, 0x77B6},
                 {0x0990, 0x77CF}, {0x098A, 0x77B8}, {0x0990, 0xFFFF}, {0x098A, 0x77BA},
                 {0x0990, 0xE340}, {0x098A, 0x77BC}, {0x0990, 0x9725}, {0x098A, 0x77BE},
                 {0x0990, 0x76CF}, {0x098A, 0x77C0}, {0x0990, 0xFFFF}, {0x098A, 0x77C2},
                 {0x0990, 0xF820}, {0x098A, 0x77C4}, {0x0990, 0x6E02}, {0x098A, 0x77C6},
                 {0x0990, 0x75CF}, {0x098A, 0x77C8}, {0x0990, 0xFFFF}, {0x098A, 0x77CA},
                 {0x0990, 0xE198}, {0x098A, 0x77CC}, {0x0990, 0x0FA2}, {0x098A, 0x77CE},
                 {0x0990, 0x04E0}, {0x098A, 0x77D0}, {0x0990, 0x9546}, {0x098A, 0x77D2},
                 {0x0990, 0x9725}, {0x098A, 0x77D4}, {0x0990, 0xB506}, {0x098A, 0x77D6},
                 {0x0990, 0x70C9}, {0x098A, 0x77D8}, {0x0990, 0x0F96}, {0x098A, 0x77DA},
                 {0x0990, 0x04E0}, {0x098A, 0x77DC}, {0x0990, 0x9547}, {0x098A, 0x77DE},
                 {0x0990, 0x04E1}, {0x098A, 0x77E0}, {0x0990, 0x0720}, {0x098A, 0x77E2},
                 {0x0990, 0xB507}, {0x098A, 0x77E4}, {0x0990, 0xD800}, {0x098A, 0x77E6},
                 {0x0990, 0x73CF}, {0x098A, 0x77E8}, {0x0990, 0xFFFF}, {0x098A, 0x77EA},
                 {0x0990, 0xF824}, {0x098A, 0x77EC}, {0x0990, 0x2315}, {0x098A, 0x77EE},
                 {0x0990, 0x0001}, {0x098A, 0x77F0}, {0x0990, 0x8141}, {0x098A, 0x77F2},
                 {0x0990, 0xE002}, {0x098A, 0x77F4}, {0x0990, 0x8120}, {0x098A, 0x77F6},
                 {0x0990, 0x08F9}, {0x098A, 0x77F8}, {0x0990, 0x80B4}, {0x098A, 0x77FA},
                 {0x0990, 0xA140}, {0x098A, 0x77FC}, {0x0990, 0x70CF}, {0x098A, 0x77FE},
                 {0x0990, 0xFFFF}, {0x098A, 0x7800}, {0x0990, 0xE340}, {0x098A, 0x7802},
                 {0x0990, 0xD920}, {0x098A, 0x7804}, {0x0990, 0xB025}, {0x098A, 0x7806},
                 {0x0990, 0x72CF}, {0x098A, 0x7808}, {0x0990, 0xFFFF}, {0x098A, 0x780A},
                 {0x0990, 0xE198}, {0x098A, 0x780C}, {0x0990, 0x9207}, {0x098A, 0x780E},
                 {0x0990, 0x71CF}, {0x098A, 0x7810}, {0x0990, 0xFFFF}, {0x098A, 0x7812},
                 {0x0990, 0xF820}, {0x098A, 0x7814}, {0x0990, 0xB805}, {0x098A, 0x7816},
                 {0x0990, 0xB100}, {0x098A, 0x7818}, {0x0990, 0x9206}, {0x098A, 0x781A},
                 {0x0990, 0xB805}, {0x098A, 0x781C}, {0x0990, 0x7FE0}, {0x098A, 0x781E},
                 {0x0990, 0xB101}, {0x098A, 0x7820}, {0x0990, 0x0000}, {0x098A, 0x7822},
                 {0x0990, 0x0000}, {0x098A, 0x7824}, {0x0990, 0x8000}, {0x098A, 0x7826},
                 {0x0990, 0x020C}, {0x098A, 0x7828}, {0x0990, 0xFFFF}, {0x098A, 0x782A},
                 {0x0990, 0xF7A8}, {0x098E, 0x7C00}, {0x098E, 0x7C00}, {0x0990, 0x17E4},
                 {0x0992, 0x0911}, {0x0994, 0x0103}, {0x0996, 0x0611}, {0x0998, 0x0084},
                 {0x0040, 0x8702}, {0x0016, 0x587F}, {0x0030, 0x0207}, {0x0030, 0x0207},
                 {0x337C, 0x000F}, {0x337E, 0x1000}, {0xC8BE, 0x0010}
            };
#endif

#if(Sensor_OPTION == Sensor_PC1089_YUV601)
const DEF_IICDATA PC_1089_IIC_reg_table[]=
{
 #include "PC1089_init.h"
};
#endif
/*****************************************************************************/
/* Function Prototype 		                                     	     */
/*****************************************************************************/
extern u32 sysBack_Check_VideoinSource(u32 dummy);

#if (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
extern u32 OS_tickcounter;
#endif

s32 i2cWrite_SENSOR(u8, u16);
s32 i2cRead_SENSOR(u8, u16*);

void gpio_SensorInSwithc2TVIn(void);
void gpio_TVInSwithc2SensorIn(void);

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
  s32 i2cWrite_WM8974(u8 addr, u16 data);
  s32 i2cRead_WM8974(u8 addr, u16* pData);
  void Init_IIS_WM8974_play();
  void Init_IIS_WM8974_rec(void);
  void IIS_WM8974_reset();
#endif

#if (AUDIO_OPTION == AUDIO_IIS_ALC5621)
  s32 i2cWrite_ALC5621(u8 addr, u16 data);
  s32 i2cRead_ALC5621(u8 addr, u16* pData);
  void Init_IIS_ALC5621_play();
  void Init_IIS_ALC5621_rec(void);
  void Init_IIS_ALC5621_bypass(void);
  void Close_IIS_ALC5621(void);
  void ALC5621_AdjustVolume(u8 Volume);
  void ALC5621_AdjustMICVolume(u8 Volume);
  void ALC5621_ChannelSwitch(u8 ch);
#endif

#if(Sensor_OPTION ==  Sensor_PC1089_YUV601)
   void i2cInit_PC1089(void);
#endif
#if Melody_SNC7232_ENA
void Melody_SNC7232_init(void);
#endif


#if (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
    void CTP_init(void);
    bool CTP_getPosition(int *x, int *y);
#endif

/*****************************************************************************/
/* Driver Routine 		                                     	     */
/*****************************************************************************/

/*

Routine Description:

	Write data through I2C.

Arguments:

	addr - The address to write.
	data - The data to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 i2cWrite_SENSOR(u8 addr, u16 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_SENSOR i2cSemReq is %d.\n", err);
		return 0;
	}

//	I2cData = ((u32)data) << 16;
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    //I2cMISC=I2C_CLK_DIV_1920;
   #endif
#endif

	I2cData = ((u32)data);
#if (Sensor_OPTION ==Sensor_MI_5M)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif( (Sensor_OPTION == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MI1320_RAW) ||  (Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) )
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif(Sensor_OPTION ==Sensor_OV7725_VGA)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif(Sensor_OPTION == Sensor_OV7725_YUV601)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif(Sensor_OPTION == Sensor_OV2643_YUV601)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif(Sensor_OPTION == Sensor_PC1089_YUV601)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif( (Sensor_OPTION == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_OV7740_RAW) )
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;

#elif(Sensor_OPTION ==Sensor_CCIR601_MIX_OV7740YUV)
      I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
#endif
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}
#if 0
	if (I2cCtrl & I2C_NACK)
		return 0;
#endif


    OSSemPost(i2cSemReq);
	return 1;
}


/*

Routine Description:

	Read data through I2C.

Arguments:

	addr - The address to write.
	pData - The data to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 i2cRead_SENSOR(u8 addr, u16* pData)
{
	u8 err;

   #if ( (Sensor_OPTION == Sensor_OV7725_VGA) || (Sensor_OPTION ==Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7740_YUV601) || (Sensor_OPTION ==Sensor_CCIR601_MIX_OV7740YUV) || (Sensor_OPTION == Sensor_OV7740_RAW) || (Sensor_OPTION == Sensor_PC1089_YUV601) || (Sensor_OPTION == Sensor_OV2643_YUV601))
        //---First step---//
        OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    	if (err != OS_NO_ERR)
    	{
    		DEBUG_I2C("Error: i2cRead_SENSOR i2cSemReq is %d.\n", err);
    		return 0;
    	}
      #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
        (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A))
        #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
          I2cMISC=I2C_CLK_DIV_3840;
        #else
          //I2cMISC=I2C_CLK_DIV_1920;
        #endif
      #endif
        I2cData = ((u32)addr);
        I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2; // modify by BJ
    	I2cCtrl |= I2C_TRIG;

    	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    	if (err != OS_NO_ERR)
    	{
    		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
            OSSemPost(i2cSemReq);
    		return 0;
    	}
        //---Second step---//


        I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2; // modify by BJ
    	I2cCtrl |= I2C_TRIG;

    	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    	if (err != OS_NO_ERR)
    	{
    		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
            OSSemPost(i2cSemReq);
    		return 0;
    	}

    	*pData = (u16)(I2cData);
  #else
    	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    	if (err != OS_NO_ERR)
    	{
    		DEBUG_I2C("Error: i2cRead_SENSOR i2cSemReq is %d.\n", err);
    		return 0;
    	}
      #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
        (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A))
        #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
          I2cMISC=I2C_CLK_DIV_3840;
        #else
          //I2cMISC=I2C_CLK_DIV_1920;
        #endif
      #endif

      #if (Sensor_OPTION ==Sensor_MI_5M)
    	   I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2; // modify by BJ
      #elif( (Sensor_OPTION == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MI1320_RAW) ||(Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) )
    	   I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2; // modify by BJ
      #endif
    	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    	I2cCtrl |= I2C_TRIG;

    	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    	if (err != OS_NO_ERR)
    	{
    	    OSSemPost(i2cSemReq);
    		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
    		return 0;
    	}

    	*pData = (u16)(I2cData);
  #endif
    OSSemPost(i2cSemReq);
	return 1;
}

#if (Sensor_OPTION ==Sensor_HM1375_YUV601)

#if (GPIO_I2C_ENA)
s32 i2cWrite_HM1375(u16 addr, u8 data)
{
    u8  err;
	u32 i;
	GPIO_IIC_CFG    stGpio_Iic;
    u8  sub_reg1; /* bit 0-7  */
    u8  sub_reg2; /* bit 8-15 */

    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_HM1375, sizeof (GPIO_IIC_CFG));

    sub_reg1 = addr & 0x00ff;
    sub_reg2 = (addr >> 8 ) & 0xff;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_CS8556 i2cSemReq is %d.\n", err);
		return 0;
	}

     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);


    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_SENSOR_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;


    /* I2C Write register offset */

    gpio_IIC_W_Byte(&stGpio_Iic, sub_reg2);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, sub_reg1);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;


    /* I2C Write data */
    gpio_IIC_W_Byte(&stGpio_Iic, data);
    /* Acknowledge */
    gpio_IIC_Ack_R(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    }
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
	DEBUG_I2C("Error: i2cWrite_HM1375() gpio IIC Write Addr [%#x] Error!!!\n", addr);

    return 0;


}

s32 i2cRead_HM1375(u16 addr, u8* pData)
{
    u8  err;
	u32 data, i;
    u8  sub_reg1; /* bit 0-7  */
    u8  sub_reg2; /* bit 8-15 */
    u32 ucDevReadAddr;
    GPIO_IIC_CFG    stGpio_Iic;

    sub_reg1 = addr & 0x00ff;
    sub_reg2 = (addr >> 8 ) & 0xff;

    //DEBUG_I2C(" read reg: %x_%x \n",sub_reg2, sub_reg1);
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_HM1375, sizeof (GPIO_IIC_CFG));

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_HM1375 i2cSemReq is %d.\n", err);
		return 0;
	}

   /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_SENSOR_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;



    /* I2C Write register offset */
    gpio_IIC_W_Byte(&stGpio_Iic, sub_reg2);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;



    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, sub_reg1);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

#if 0
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

     /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
#endif

    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);



    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_SENSOR_RD_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pData);
    /* Inverse Acknowledge */
    //gpio_IIC_nAck_W(&stGpio_Iic);
    if (!gpio_IIC_nAck_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
    DEBUG_I2C("Error: i2cRead_HM1375() gpio IIC Read Addr [%#x] Error!!!\n", addr);

    return 0;
}



#else
s32 i2cWrite_HM1375(u16 addr, u8 data)
{
	u8 err;
    u8 data2;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_HM1375 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((((u32)addr) << 8) | data);
	I2cCtrl = I2C_ENA | I2C_3B | I2C_CLK_DIV_1920 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_HM1375 i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);

#if 0
    i2cRead_HM1375_Manual(addr,&data2);
    if(data2 != data )
    {
        DEBUG_I2C("HM1375 0x%x reg:data error write(0x%x) != read(0x%x) \n",addr,data,data2);
    }
#endif

	return 1;
}

s32 i2cRead_HM1375(u16 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_HM1375 i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
    // write 2 bytes register address
	I2cData = (u32)addr;
	I2cCtrl = I2C_ENA | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375 write 2 bytes register address i2cSemFin is %d.\n", err);
		return 0;
	}

    // read 1 byte data
	I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375 read 1 byte data i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read HM1375 i2c addr = 0x%04x, i2c data = 0x%02x\n", addr, *pData);
	OSSemPost(i2cSemReq);
	return 1;
#else
	if (!gpio_IIC_Read(IIC_DEV_HM1375, I2C_HM1375_WR_SLAV_ADDR, addr, pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif

}


s32 i2cRead_HM1375_Manual(u16 ucRegAddr, u8* pucData)
{
    u8 err;
    u32 i;
    u8 subReg=0;
    u8 subReg2=0;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_HM1375_Manual(0x%02x, 0x%02x), i2cSemReq is %d.\n",  ucRegAddr, *pucData, err);
        return 0;
    }

    I2cCtrl     = I2C_MANUAL_EN  |I2C_2B| I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_3840  | (I2C_SENSOR_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) | ( subReg << I2C_SUB_ADDR_SHFT);

    /* Start */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_SENSOR_WR_SLAV_ADDR);
		return 0;
	}

    /* Reg Addr (bit 15:8) */
    I2cData     = (u32)ucRegAddr;
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData2;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData2)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData2))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);
		return 0;
	}

    /* Reg Addr (bit 0:7) */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData1;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);
		return 0;
	}


    /* Start */
    I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_3840  | (I2C_SENSOR_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_SENSOR_RD_SLAV_ADDR);
		return 0;
	}

    /* Read Data */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Read Data Error!!!\n");
		return 0;
	}


    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
    *pucData  = (u8)I2cData;
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_HM1375_Manual() IIC Stop Error!!!\n");
		return 0;
	}
    else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}
#endif /* end of GPIO_I2C_ENA */
#endif


#if (Sensor_OPTION ==Sensor_NT99141_YUV601)

s32 i2cWrite_NT99141(u16 addr, u8 data)
{
	u8 err;
    u8 data2;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		/*CY 0907 */
		/* signal event semaphore */
		OSSemPost(i2cSemReq);
		OSSemPost(i2cSemFin);

		DEBUG_I2C("Error: i2cWrite_NT99141 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((((u32)addr) << 8) | data);
	I2cCtrl = I2C_ENA | I2C_3B | I2C_CLK_DIV_1920 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_NT99141 i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);

#if 0
    i2cRead_NT99141_Manual(addr,&data2);
    if(data2 != data )
    {
        DEBUG_I2C("NT99141 0x%x reg:data error write(0x%x) != read(0x%x) \n",addr,data,data2);
    }
#endif

	return 1;
}

s32 i2cRead_NT99141(u16 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_NT99141 i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
    // write 2 bytes register address
	I2cData = (u32)addr;
	I2cCtrl = I2C_ENA | I2C_2B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141 write 2 bytes register address i2cSemFin is %d.\n", err);
		return 0;
	}

    // read 1 byte data
	I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141 read 1 byte data i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read NT99141 i2c addr = 0x%04x, i2c data = 0x%02x\n", addr, *pData);
	OSSemPost(i2cSemReq);
	return 1;
#else
	if (!gpio_IIC_Read(IIC_DEV_NT99141, I2C_NT99141_WR_SLAV_ADDR, addr, pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif

}


s32 i2cRead_NT99141_Manual(u16 ucRegAddr, u8* pucData)
{
    u8 err;
    u32 i;
    u8 subReg=0;
    u8 subReg2=0;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_NT99141_Manual(0x%02x, 0x%02x), i2cSemReq is %d.\n",  ucRegAddr, *pucData, err);
        return 0;
    }

    I2cCtrl     = I2C_MANUAL_EN  |I2C_2B| I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_3840  | (I2C_SENSOR_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) | ( subReg << I2C_SUB_ADDR_SHFT);

    /* Start */
    I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_SENSOR_WR_SLAV_ADDR);
		return 0;
	}

    /* Reg Addr (bit 15:8) */
    I2cData     = (u32)ucRegAddr;
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData2;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData2)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData2))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);
		return 0;
	}

    /* Reg Addr (bit 0:7) */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData1;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);
		return 0;
	}


    /* Start */
    I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_3840  | (I2C_SENSOR_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_SENSOR_RD_SLAV_ADDR);
		return 0;
	}

    /* Read Data */
    I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Read Data Error!!!\n");
		return 0;
	}


    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
    *pucData  = (u8)I2cData;
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_NT99141_Manual() IIC Stop Error!!!\n");
		return 0;
	}
    else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#endif

#if (Sensor_OPTION == Sensor_PO3100K_YUV601)

s32 i2cWrite_PO3100K(u8 addr, u8 data)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_PO3100K(0x%02x, 0x%02x), i2cSemReq is %d.\n", addr, data, err);
        return 0;
    }

//  DEBUG_I2C("I2C : %x  %x\n",addr,data);


    I2cData = ((u32)data);

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cWrite_PO3100K(0x%02x, 0x%02x), i2cSemFin is %d.\n", addr, data, err);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
}

s32 i2cRead_PO3100K(u8 addr, u8* pData)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_PO3100K(0x%02x, 0x%02x), i2cSemReq is %d.\n", addr, *pData, err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_SENSOR_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_PO3100K(0x%02x, 0x%02x), i2cSemFin is %d.\n", addr, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    return 1;
}

#endif



/*****************************************************************************/
/* Application Routine	 		                                     */
/*****************************************************************************/

/*

Routine Description:

	Initialize I2C.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 i2cInit(void)
{
#if((HW_BOARD_OPTION==ES_LIGHTING))
    u32 i;
#endif
	/* Create the semaphore */
	i2cSemReq = OSSemCreate(1);
	i2cSemFin = OSSemCreate(0);
	i2cWDTProtect = OSSemCreate(1);

#if !USE_BUILD_IN_RTC
  #if(EXT_RTC_SEL == RTC_HT1381)
    RTC_HT1381_Init();
  #elif(EXT_RTC_SEL == RTC_ISL1208)
    ISL1208_RTC_Init();
  #elif(EXT_RTC_SEL == RTC_BQ32000)
    RTC_BQ32000_Init();
    //i2cTest_RTC_BQ32000();
  #elif(EXT_RTC_SEL == RTC_SD2068)
    SD2068_RTC_Init();
  #endif
#endif

	return 1;
}


void i2cEnableSecondI2C(void)
{
    GpioActFlashSelect |= GPIO_ACT_FLASH_IIC2;
    /* GPIO0[31]*/
    Gpio0Ena   |=  0x80000000;
    Gpio0Dir   &= ~0x80000000;
    Gpio0Level &= ~0x80000000;
    /* GPIO3[27]*/
    Gpio3Ena   |=  0x08000000;
    Gpio3Dir   &= ~0x08000000;
    Gpio3Level &= ~0x08000000;
}


s32 i2cDeviceInit(void)
{
    //----Sensor Reset ---//
#if(  (HW_BOARD_OPTION == A1013_FPGA_BOARD)||(HW_BOARD_OPTION == A1016_FPGA_BOARD) ||(HW_BOARD_OPTION == A1016B_FPGA_BOARD) || \
      (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || \
      (HW_BOARD_OPTION  == A1026A_FPGA_BOARD) ||\
      (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8211_IPCAM) || \
      (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU) )
   int i;
   u16 data;
   
  #if( ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) ) && (CIU1_OPTION == Sensor_HM1375_YUV601) )
    gpioSetLevel(GPIO_ENA_SENSOR_GROUP2, GPIO_ENA_SENSOR2, 1 ); //sensor enable
    OSTimeDly(1);
    gpioSetLevel(GPIO_ENA_SENSOR_GROUP2, GPIO_ENA_SENSOR2, 0 ); //sensor enable
  #endif

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );

    gpioSetLevel(GPIO_RST_SENSOR_GROUP2, GPIO_RST_SENSOR2, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP2, GPIO_RST_SENSOR2, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP2, GPIO_RST_SENSOR2, 1 );

    gpioSetLevel(GPIO_RST_SENSOR_GROUP3, GPIO_RST_SENSOR3, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP3, GPIO_RST_SENSOR3, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP3, GPIO_RST_SENSOR3, 1 );

    gpioSetLevel(GPIO_RST_SENSOR_GROUP4, GPIO_RST_SENSOR4, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP4, GPIO_RST_SENSOR4, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP4, GPIO_RST_SENSOR4, 1 );

#else


#endif

/* A1016A second I2C should flow step that can it be enabled */
#if( ( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) ) && (CHIP_A1016A ))
    i2cEnableSecondI2C();
#endif

   //---Sensor init---//
#if(Sensor_OPTION ==  Sensor_MI9V136_YUV601)
   data= HW_I2C_Word_addr_Read_Word(I2C_MI9V136_RD_SLAV_ADDR,0x3000);
   DEBUG_SIU("Sensor ID=%d\n",data);
   HW_I2C_Word_addr_Write_Word(I2C_MI9V136_WR_SLAV_ADDR,0x0016, 0x0587f);
#endif

#if(Sensor_OPTION ==  Sensor_PC1089_YUV601)
   i2cInit_PC1089();
#endif
   //----TV decoder init----//
#if MULTI_CHANNEL_SUPPORT
         #if(TV_DECODER == TI5150)
           i2cInit_TVP5150_1();
           i2cInit_TVP5150_2();
         #elif(TV_DECODER == BIT1605)
           i2cInit_BIT1605();
		   i2cInit_BIT1605_DV2();
         #elif(TV_DECODER == MI9V136)
           i2cInit_MI9V136();
         #elif(TV_DECODER == CJC5150)
           i2cInit_CJC5150();
         #endif
#else
     #if ( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR656) )
         #if(TV_DECODER == TI5150)
           i2cInit_TVP5150_1();
          #if MULTI_CHANNEL_SUPPORT
           i2cInit_TVP5150_2();
          #endif
         #elif(TV_DECODER == BIT1605)
           i2cInit_BIT1605();
         #elif(TV_DECODER == MI9V136)
           i2cInit_MI9V136();
         #elif(TV_DECODER == CJC5150)
           i2cInit_CJC5150();
         #endif
     #endif
#endif

#if(Sensor_OPTION ==Sensor_CCIR656)
    siuCCIR656_Ena();
#endif
   //----G sensor init-----//
#if (G_SENSOR == G_SENSOR_LIS302DL)     // for ST LIS302DL G sensor control
	i2cInit_LIS302DL();
#elif (G_SENSOR == G_SENSOR_H30CD)      // for Hitachi H30CD G sensor control
	i2cInit_H30CD();
#elif (G_SENSOR == G_SENSOR_DMARD03)      // for Hitachi H30CD G sensor control
    i2cInit_DMARD03();
#elif (G_SENSOR == G_SENSOR_BMA150)      // for Bosch BMA150 G sensor control
    i2cInit_BMA150();
#endif

#if((GPIO_I2C_ENA == 1) && (TV_ENCODER== CS8556) )
    i2cInit_CS8556();
#endif

#if(TV_DECODER == WT8861)
	VIDEO_OutputTriState(ON,I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
    i2cInit_WT8861_1();
	VIDEO_PortChange(INPUT_SOURCE_AV1);
	VIDEO_SetupColourStandard();
#if IS_COMMAX_DOORPHONE
    i2cInit_WT8861_2();
#endif
#endif

#if(TV_DECODER == TW9900)

#if (HW_BOARD_OPTION==MR6730_AFN)
	//delay for TW9900 stable??
	OSTimeDlyHMSM(0,0,0,300);//it maybe need G.T. 200ms 
#endif 

    i2cInit_TW9900_1();
//#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN)
#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION==MR6730_AFN) 
    #if( (HW_BOARD_OPTION==MR6730_AFN)&&(!MULTI_CH_DEGRADE_1CH) )
    i2cInit_TW9900_2(); 
    #else
    i2cInit_TW9900_2();    
    #endif
#endif
#endif
#if(TV_DECODER == TW9910)
	i2cInit_TW9910();
#endif
#if(TV_DECODER == TW2866)
    i2cInit_TW2866();
#endif
#if(TV_ENCODER == CH7025)
    i2cInit_CH7025();
#endif
#if(ECHO_IC == FM1288)
#if (HW_BOARD_OPTION == MR8120_TX_HECHI)
    i2cInit_FM1288_Tx();
#else
    i2cInit_FM1288_Rx();
#endif
#endif

#if((HW_BOARD_OPTION == MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) ||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO2) ||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||(HW_BOARD_OPTION == MR8100_GCT_VM9710)||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)||\
    (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) ||\
    (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS)||\
    (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
    gpioIRLEDInit();
#endif

#if (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
    CTP_init();
#endif

	return 1;
}

/*

Routine Description:

	Uninitialize I2C.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 i2cUninit(void)
{
	u8 err;

	/* Delete the semaphore */
	i2cSemReq = OSSemDel(i2cSemReq, OS_DEL_ALWAYS, &err);
	i2cSemFin = OSSemDel(i2cSemFin, OS_DEL_ALWAYS, &err);

	return 1;
}

/*

Routine Description:

	The IRQ handler of I2C.

Arguments:

	None.

Return Value:

	None.

*/
void i2cIntHandler(void)
{
/*BJ 061020 S*/
	u32 temp;
	temp = I2cCtrl;
	temp = temp;
/*BJ 061020 E*/
	/* signal event semaphore */

	OSSemPost(i2cSemFin);
}

/*

Routine Description:

	The test routine of I2C.

Arguments:

	None.

Return Value:

	None.

*/
#if IIC_TEST
	void i2cTest(void)
	{
		u16 data;

	   #if 1
	        /* use Micron MT9D011 as an example */
		i2cRead_SENSOR(SENSOR_PLL_CONTROL_1, &data);
		i2cWrite_SENSOR(SENSOR_PLL_CONTROL_1, 0x5555);
		i2cRead_SENSOR(SENSOR_PLL_CONTROL_1, &data);
		i2cRead_SENSOR(SENSOR_PLL_CONTROL_1, &data);
	   #else
	         /*use Wolfson WM8974 as an example*/
	         i2cWrite_WM8974(0, 0x00); //reset WM8974

	         i2cWrite_WM8974(1, 0x0d);
	         i2cWrite_WM8974(2, 0x00);
	         i2cWrite_WM8974(3, 0xed);
	         i2cWrite_WM8974(4, 0x1ff);
	         //==No read mode=//
                 i2cRead_WM8974(1,&data);
                 i2cRead_WM8974(2,&data);
                 i2cRead_WM8974(3,&data);
                 i2cRead_WM8974(4,&data);

	   #endif
	}
#endif


#if (AUDIO_OPTION == AUDIO_IIS_WM8974)

#define I2C_WM8974_WR_SLAV_ADDR  0x00000034
#define I2C_WM8974_RD_SLAV_ADDR  0x00000035

s32 i2cWrite_WM8974(u8 addr, u16 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_WM8974 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cData = ((u32)data) & 0x0ff;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
      I2cMISC=I2C_CLK_DIV_3840;
    #else
      I2cMISC=I2C_CLK_DIV_1920;
    #endif
#endif

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_WM8974_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;

	I2cCtrl = I2cCtrl | (((u32)addr & 0x0ff) << 17) | ( ((u32)data & 0x0100)<<8 );
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}


/*

Routine Description:

	Read data through I2C.

Arguments:

	addr - The address to write.
	pData - The data to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 i2cRead_WM8974(u8 addr, u16* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_WM8974 i2cSemReq is %d.\n", err);
		return 0;
	}

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
      I2cMISC=I2C_CLK_DIV_3840;
    #else
      I2cMISC=I2C_CLK_DIV_1920;
    #endif
#endif

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_WM8974_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;

	I2cCtrl = I2cCtrl | ( ((u32)addr & 0x0ff) << 17 );
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u16)(I2cData);
	*pData |= ((I2cCtrl & 0x10000)>>8);
    OSSemPost(i2cSemReq);
	return 1;
}

void Init_IIS_WM8974_play(void)
{

    i2cWrite_WM8974(0,0x0);
    i2cWrite_WM8974(1,0xd);
    i2cWrite_WM8974(2,0x0);
    i2cWrite_WM8974(3,0xed);
    i2cWrite_WM8974(4,0x10);          // IIS Format, word length = 16bits
    i2cWrite_WM8974(5,0x0);
    i2cWrite_WM8974(6,0x0);
    i2cWrite_WM8974(7,0xa);           // 8k sampling rate
    i2cWrite_WM8974(8,0x0);
    i2cWrite_WM8974(10,0x0);
    i2cWrite_WM8974(11,0xff);
    i2cWrite_WM8974(14,0x100);
    i2cWrite_WM8974(15,0xff);
    i2cWrite_WM8974(18,0x12c);
    i2cWrite_WM8974(19,0x2c);
    i2cWrite_WM8974(20,0x2c);
    i2cWrite_WM8974(21,0x2c);
    i2cWrite_WM8974(22,0x2c);
    i2cWrite_WM8974(24,0x32);
    i2cWrite_WM8974(25,0x0);
    i2cWrite_WM8974(27,0x0);
    i2cWrite_WM8974(28,0x0);
    i2cWrite_WM8974(29,0x0);
    i2cWrite_WM8974(30,0x0);
    i2cWrite_WM8974(32,0x38);
    i2cWrite_WM8974(33,0xb);
    i2cWrite_WM8974(34,0x32);
    i2cWrite_WM8974(35,0x0);
    i2cWrite_WM8974(36,0x8);
    i2cWrite_WM8974(37,0xc);
    i2cWrite_WM8974(38,0x93);
    i2cWrite_WM8974(39,0xe9);
    i2cWrite_WM8974(44,0x3);
    i2cWrite_WM8974(45,0x10);
    i2cWrite_WM8974(47,0x100);
    i2cWrite_WM8974(49,0x2);
    i2cWrite_WM8974(50,0x1);
    i2cWrite_WM8974(54,0x3f);        // Speaker Volume: 6dB
    i2cWrite_WM8974(56,0x1);
}

void Init_IIS_WM8974_rec(void)
{
    i2cWrite_WM8974(0,0x0);
    i2cWrite_WM8974(1,0x11d);          // Buffer Enabled,Microphone Bias Enable
    i2cWrite_WM8974(2,0x15);           // Boost stage ON, Microphone PGA enable, ADC enable
    i2cWrite_WM8974(3,0x0);            // MONOOUT,SPKOUTN,SPKOUTP disable.
    i2cWrite_WM8974(4,0x12);           // Word length = 16bits, IIS Format, ADC data appear in "left" phase of Frame
    i2cWrite_WM8974(5,0x0);            // ADC/DAC companding off, Digital loopback off.
    i2cWrite_WM8974(6,0x0);
    i2cWrite_WM8974(7,0xa);            // 8k Sampling Rate
    i2cWrite_WM8974(8,0x0);
    i2cWrite_WM8974(10,0x0);
    i2cWrite_WM8974(11,0xff);
    i2cWrite_WM8974(14,0x100);
    i2cWrite_WM8974(15,0xff);
    i2cWrite_WM8974(18,0x12c);
    i2cWrite_WM8974(19,0x2c);
    i2cWrite_WM8974(20,0x2c);
    i2cWrite_WM8974(21,0x2c);
    i2cWrite_WM8974(22,0x2c);
    i2cWrite_WM8974(24,0x32);
    i2cWrite_WM8974(25,0x0);
    i2cWrite_WM8974(27,0x0);
    i2cWrite_WM8974(28,0x0);
    i2cWrite_WM8974(29,0x0);
    i2cWrite_WM8974(30,0x0);
    i2cWrite_WM8974(32,0x038);      // original 0x038, ALC disable
    i2cWrite_WM8974(33,0xb);
    i2cWrite_WM8974(34,0x32);
    i2cWrite_WM8974(35,0x0);
    i2cWrite_WM8974(36,0x8);
    i2cWrite_WM8974(37,0xc);
    i2cWrite_WM8974(38,0x93);
    i2cWrite_WM8974(39,0xe9);
    i2cWrite_WM8974(44,0x3);        // 2, MICN conn.to input PGA amp Negative terminal, MICP conn.to input PGA amp to VNID
    i2cWrite_WM8974(45,0x3f);       // 10, Input PGA Volume: +35.25dB
    i2cWrite_WM8974(47,0x121);      // 121, input Boost 20dB, MICP: -9dB gain through BOOST stage,auxilliary amp -12dB gain
    i2cWrite_WM8974(49,0x2);
    i2cWrite_WM8974(50,0x0);
    i2cWrite_WM8974(54,0x3f);        // Speaker Volume: 6dB
    i2cWrite_WM8974(56,0x0);
}

void IIS_WM8974_reset()
{   //For Power down
    i2cWrite_WM8974(0,0x0);
}

#endif

#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)

s32 i2cWrite_ALC5621(u8 addr, u16 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_ALC5621 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_ALC5621_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_ALC5621(u8 addr, u16* pData)

{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_ALC5621 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_480 | (I2C_ALC5621_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = I2cData;
	//DEBUG_I2C("read ALC5621 i2c addr=%d,  i2c data=%x\n",addr ,*pData);
    OSSemPost(i2cSemReq);
	return 1;
}

void ALC5621_AdjustVolume(u8 Volume)
{
    u16 volumecontrol;
    volumecontrol = (DACVolumnTable[Volume]<<8) | (DACVolumnTable[Volume]);
    i2cWrite_ALC5621(12,volumecontrol); //  0ch, Stereo DAC Volume
}

void ALC5621_AdjustMICVolume(u8 Volume)
{
    u16 volumecontrol;
    if(Volume == 0)
    {
        i2cWrite_ALC5621(0x1C,0xC000); // HPR Volume output input select
    }
    else
    {
        i2cWrite_ALC5621(0x1C,0xC300); // HPR Volume output input select
        volumecontrol = (MICVolumeTable[Volume]<<8) | (MICVolumeTable[Volume]);
        i2cWrite_ALC5621(0x0e,volumecontrol); //  0eh, MIC Volume
    }
}

void ALC5621_ChannelSwitch(u8 ch)
{
    if(ch == 1)
        i2cWrite_ALC5621(0x14,0x3f3f);	// 14h, ADC Record Mixer Control
    else if(ch == 2)
 		i2cWrite_ALC5621(0x14,0x5f5f);	// 14h, ADC Record Mixer Control

    #if (AUDIO_BYPASS == 1)
    if(ch == 1)
    {
        i2cWrite_ALC5621(0x10,0x60e0);	 // 10h, Enable MIC1 Single End Input
        i2cWrite_ALC5621(0x3e,0xb61a); // 3eh, power management
    }
    else if(ch == 2)
    {
        i2cWrite_ALC5621(0x10,0xe060);	 // 10h, Enable MIC2 Single End Input
        i2cWrite_ALC5621(0x3e,0xb615);
    }
    #endif
}
void Init_IIS_ALC5621_play(void)
{

	i2cWrite_ALC5621(52,0x8000);   // 34h, Audio Interface, slave mode, bclk, sclk from IIS controller, 16bi// bit15, 0: master, 1: slave
	i2cWrite_ALC5621(58,0x8177);	//	3ah, power management
	i2cWrite_ALC5621(60,0xf730);	  //  3ch, power management
	i2cWrite_ALC5621(62,0xb610);	  //  3eh, power management


	//i2cWrite_ALC5621(52,0x8000);   // 34h, Audio Interface, slave mode, bclk, sclk from IIS controller, 16bit												   // bit15, 0: master, 1: slave
    i2cWrite_ALC5621(106,0x0054);	 // 6a, Index54  --- for DAC power saving
    i2cWrite_ALC5621(108,0xe184);	 // 6c, E184h  --- for DAC power saving
	i2cWrite_ALC5621(54,0x0668);   //  36h, Stereo AD/DA Clock Control, ?k sampling rate
	i2cWrite_ALC5621(12,0x0000);   //  0ch, Stereo DAC Volume

	//i2cWrite_ALC5621(28,0x8040);   //  1ch, Output Mixer Control
	i2cWrite_ALC5621(28,0x8740);   //  1ch, Output Mixer Control
	i2cWrite_ALC5621(2,0x0000); 	//	02h, Speaker Output Volume
	i2cWrite_ALC5621(106,0x0046);  //  6ah, Hidden Control Index Port  (ClassD only)
	i2cWrite_ALC5621(108,0xffff);	  //  6ch, Hidden Control Data Port   (ClassD only)
	i2cWrite_ALC5621(4,0x0000); 	//	04h, Headphone Output Volume

	i2cWrite_ALC5621(6,0x0000);   //  06h, Monoout/Auxout volume, Enable Right Channel
	i2cWrite_ALC5621(64,0xd300);   //  40h, Monoout/Auxout Single-End Mode

//	i2cWrite_ALC5621(56,0x8000);   // 38h, Device works at 8 bits mode when PCM mode B

}


#if 1

void Init_IIS_ALC5621_bypass(void)
{

		  i2cWrite_ALC5621(52,0x8000);	  // 34h, Audio Interface, slave mode, bclk, sclk from IIS controller, 16bit
										// bit15, 0: master, 1: slave
										// bit5: ADC data appear at right phase of LRCK, 0: left phase
										//												 1: right phase
		  //i2cRead_ALC5621(52,&data);

		  i2cWrite_ALC5621(106,0x0054);   // 6a, Index54  --- for DAC power saving
		  i2cWrite_ALC5621(108,0xe184);   // 6c, E184h	--- for DAC power saving


          i2cWrite_ALC5621(0x3a,0xcd77); // 3ah, power management
   		  i2cWrite_ALC5621(0x3c,0xf7f3); // 3ch, power management

          i2cWrite_ALC5621(0x3e,0xb61a);

	// AVC Function start

	// i2cWrite_ALC5621(104,0x000b);		// 68h, 1100_0000_0000_1011 ,moitor window = 2^(11+1)=4096, window is smaller, resopnse is faster


	 i2cWrite_ALC5621(106,0x0021);		 // 6ah, The Max PCM absolute level after AVC, Thmax (= 0~2^15-1)
	 i2cWrite_ALC5621(108,0x1000);



	 i2cWrite_ALC5621(106,0x0022);		 // 6ah, The Min PCM absolute level after AVC, Thmin (= 0~2^15-1)
	 i2cWrite_ALC5621(108,0x0950);


	 i2cWrite_ALC5621(106,0x0023);		 // 6ah, The Non-active PCM absolute level AVC will keep analog unit gain, Thnonact (= 0~2^15-1)
	// i2cWrite_ALC5621(108,0x0001);
	 i2cWrite_ALC5621(108,0x0450);


	 i2cWrite_ALC5621(106,0x0024);		 // 6ah, The CNTMAXTH1 to control the sensitivity to increase Gain (unit: 2^1)
	// i2cWrite_ALC5621(108,0x005f);
    // i2cWrite_ALC5621(108,0x01ff);
	// i2cWrite_ALC5621(108,0x007f);
     i2cWrite_ALC5621(108,0x0001);       // if fill 0x0000, besides mic won't saturation
	// i2cWrite_ALC5621(108,0x00ff);


	 i2cWrite_ALC5621(106,0x0025);		 // 6ah, The CNTMAXTH2 to control the sensitivity to decrease Gain (unit: 2^1)
	// i2cWrite_ALC5621(108,0x0060);     // CNTMAXTH2 = moitor window/2 = 8/2=4
	// i2cWrite_ALC5621(108,0x0200);
	// i2cWrite_ALC5621(108,0x0080);
	 i2cWrite_ALC5621(108,0x0001);
	// i2cWrite_ALC5621(108,0x0100);


	// i2cWrite_ALC5621(104,0x9005);	   // 68h, 1011_0000_0000_0111 ,moitor window = 2^(7+1)=256, window is smaller, resopnse is faster
	// i2cWrite_ALC5621(104,0xb005);       // bit13: o: keep unit gain
	// i2cWrite_ALC5621(104,0x900b);             1: keep previous gain
     i2cWrite_ALC5621(104,0x1005);         // disable AVC ************************











				// i2cWrite_ALC5621(98,0xc81f);	  // 62h, EQ Control     All
				// i2cWrite_ALC5621(98,0xc810);	  // 62h, EQ Control     HP
				 i2cWrite_ALC5621(98,0xc811);	  // 62h, EQ Control     LP+HP

				 i2cWrite_ALC5621(106,0x0000);	  // 6ah, index 00       LP0=100Hz, -12dB
				 i2cWrite_ALC5621(108,0x1757);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0001);	  // 6ah, index 01
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0002);	  // 6ah, index 02
				 i2cWrite_ALC5621(108,0xcf6c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0003);	  // 6ah, index 03
				 i2cWrite_ALC5621(108,0x10bb);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0004);	  // 6ah, index 04
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0005);	  // 6ah, index 05
				 i2cWrite_ALC5621(108,0xcf6c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0006);	  // 6ah, index 06
				 i2cWrite_ALC5621(108,0x10bb);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0007);	  // 6ah, index 07
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0008);	  // 6ah, index 08
				 i2cWrite_ALC5621(108,0xdb82);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0009);	  // 6ah, index 09
				 i2cWrite_ALC5621(108,0x139c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x000a);	  // 6ah, index 0a
				 i2cWrite_ALC5621(108,0x0c73);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x000b);	  // 6ah, index 0b
			//	 i2cWrite_ALC5621(108,0x139c);	  // 6ch, index data      HP0=600Hz
			//	 i2cWrite_ALC5621(108,0x193a);	  // 6ch, index data
			//	 i2cWrite_ALC5621(108,0x1c6f);	  // 6ch, index data      HP0=150Hz
				 i2cWrite_ALC5621(108,0x173f);	  // 6ch, index data      HP0=400Hz


				 i2cWrite_ALC5621(102,0x0000);	  // 66h, EQ Mode Change Enable
				 i2cWrite_ALC5621(102,0x001f);	  // 66h, EQ Mode Change Enable


				 i2cWrite_ALC5621(106,0x0012);	  // 6ah, index12 --- EQ output volume control
				 i2cWrite_ALC5621(108,0x0007);	  // 6ch, 18dB
			//	 i2cWrite_ALC5621(108,0x0006);	  // 6ch, 15dB,  default
			//	 i2cWrite_ALC5621(108,0x0005);	  // 6ch, 12dB
			//   i2cWrite_ALC5621(108,0x0004);	  // 6ch, 9dB




			// EQ Function end



			// i2cWrite_ALC5621(54,0x0668);	 // 36h, Stereo AD/DA Clock Control, ?k sampling rate
			 i2cWrite_ALC5621(54,0x0a11);
			// i2cWrite_ALC5621(16,0x60f0);  // 10h, Enable MIC1 Single End Input


    			i2cWrite_ALC5621(0x10,0x60e0);	 // 10h, Enable MIC1 Single End Input


             #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                i2cWrite_ALC5621(10,0xe808);	 // 0ah, Line-In
                i2cWrite_ALC5621(8,0xe808);	 // 0ah, Line-In
             #else
			 i2cWrite_ALC5621(10,0x6808);	 // 0ah, Line-In
             #endif
			// i2cWrite_ALC5621(34,0x0a00);    // 22h, MIC1,2 Boost 30dB	--- very important
	//		 i2cWrite_ALC5621(34,0x0000);  // 22h, MIC1,2 Boost 20dB	--- very important //albert on 20090713
			 i2cWrite_ALC5621(34,0x0000);  // 22h, MIC1,2 Boost 20dB	--- very important //albert on 20090713
			// i2cWrite_ALC5621(34,0x0000);	 // 22h, MIC1,2 Boost Bypass  --- very important


			i2cWrite_ALC5621(20,0x3f3f);	 // 14h, ADC Record Mixer Control


            // i2cWrite_ALC5621(20,0x3b3b);  // 14h, ADC Record Mixer Control
			// i2cWrite_ALC5621(20,0x7b7b);  // 14h, ADC Record Mixer Control
			i2cWrite_ALC5621(14,0x0202);  // 0eh, Mic volume  07/10//albert on 20090713

		    // i2cWrite_ALC5621(18,0xffff);	 // 12h, ADC Record Gain 30  dB Gain  [1111][11111][11][11111]
		    // i2cWrite_ALC5621(18,0xfe7c);	 // 12h, ADC Record Gain 25.5dB Gain  [1111][11100][11][11100]
		    // i2cWrite_ALC5621(18,0xfdfb);	 // 12h, ADC Record Gain 24  dB Gain  [1111][11011][11][11011]
		    // i2cWrite_ALC5621(18,0xfd7a);	 // 12h, ADC Record Gain 22.5dB Gain  [1111][11010][11][11010]
		     i2cWrite_ALC5621(18,0xf6eB);	 // 12h, ADC Record Gain 19.5dB Gain  [1111][11000][11][11000]//albert on 20090713
			// i2cWrite_ALC5621(18,0xf7ef);	 // 12h, ADC Record Gain 6dB Gain     [1111][01111][11][01111]
			// i2cWrite_ALC5621(18,0xf5eb);	 // 12h, ADC Record Gain 0dB Gain     [1111][01011][11][01011]


            i2cWrite_ALC5621(0x04,0x0000);
            i2cWrite_ALC5621(0x1C,0xc300);
            ALC5621_AdjustMICVolume(sysVolumnControl);
            ALC5621_AdjustVolume(sysVolumnControl);
}

void Init_IIS_ALC5621_rec(void)
{

		  i2cWrite_ALC5621(52,0x8000);	  // 34h, Audio Interface, slave mode, bclk, sclk from IIS controller, 16bit
										// bit15, 0: master, 1: slave
										// bit5: ADC data appear at right phase of LRCK, 0: left phase
										//												 1: right phase
		  //i2cRead_ALC5621(52,&data);  //Lucian: Cannt read it here. confused!

          //DEBUG_I2C("ALC5621 data test=0x%x\n",data);

		  i2cWrite_ALC5621(106,0x0054);   // 6a, Index54  --- for DAC power saving
		  i2cWrite_ALC5621(108,0xe184);   // 6c, E184h	--- for DAC power saving

	#if (AUDIO_BYPASS==1)
          i2cWrite_ALC5621(0x3a,0xcd77); // 3ah, power management
   		  i2cWrite_ALC5621(0x3c,0xf7f3); // 3ch, power management


          #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
            (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
          i2cWrite_ALC5621(0x3e,0x80f0);
          #else
          i2cWrite_ALC5621(0x3e,0xb61a);
          #endif
    #else

	// i2cWrite_ALC5621(60,0xaff7); 	// 3ch, power management										   // 0010 1000 1100 0011
    //albert added on 20100126 for power sequence
		  i2cWrite_ALC5621(60,0x2000);
          i2cWrite_ALC5621(62,0x8000);


	// i2cWrite_ALC5621(58,0xbe88);   // 3ah, power management		   // 1000 1100 0000 0000
		  i2cWrite_ALC5621(58,0xcc00);
		  //i2cRead_ALC5621(58,&data);
										// bit15: I2S Digital Interface Enable						 ---
												// bit14: All Zero Cross Detect Power Down
										// bit13: Reserved
										// bit12: Reserved
									    // bit11: Enable Microphone 1 Bias						  ---
										// bit10: MICBIAS Short Current Detector Control		---
										// bit09: Reserved
										// bit08: Power on softgen
										// bit07: Reserved
										// bit06: Power on depop-buffer of HP
										// bit05: Enable HP output buffer for normal loading
										// bit04: Enable HP output buffer for small R loading
										// bit03: Reserved
										// bit02: Power on depop-buffer of AUX
										// bit01: Enable AUX Output buffer for normal loading
										// bit00: Enable AUX Output buffer for small R loading


	// i2cWrite_ALC5621(60,0xaff7); 	// 3ch, power management										   // 0010 1000 1100 0011
		  i2cWrite_ALC5621(60,0x27c3);
		  //i2cRead_ALC5621(60,&data);
												// bit15: Enable All Class_AB Power
										// bit14: Enable All Class_D Power
										// bit13: Enable VREF for All Analog ckt				   ---
										// bit12: Enable PLL
										// bit11: Enable Thermal Shutdown							---
										// bit10: Enable DAC ref Ckt
										// bit09: Enable Left STEREO DAC filter clock
										// bit08: Enable Right STEREO DAC filter clock
										// bit07: Enable Left STEREO ADC filter clock and input gain			  ---
										// bit06: Enable Right STEREO ADC filter clock and input gain			 ---
										// bit05: Enable Left HP Mixer
										// bit04: Enable Right HP Mixer
										// bit03: Enable Speaker Mixer
										// bit02: Enable Mono Mixer
										// bit01: Enable Left ADC Record Mixer											---
										// bit00: Enable Right ADC Record Mixer 									   ---


	// i2cWrite_ALC5621(62,0x9f0a);    // 3eh, power management 											// 1000 0000 0000 1010



    #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
        (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
          i2cWrite_ALC5621(0x3e,0x80f0);
    #else
        i2cWrite_ALC5621(62,0x8002);
    #endif
		  //i2cRead_ALC5621(62,&data);
										// bit15: Enable Main Bias of ALC5621 Analog ckt											  ---
										// bit14: AUXOUT_L(Mono_P) Volume Control & AUX_L Amp
										// bit13: AUXOUT_R(Mono_N) Volume Control & AUX_R Amp
										// bit12: Enable SPK Output
										// bit11: Reserved
										// bit10: Enable HP_OUT_L Volume Control & HP_L Amp
										// bit09: Enable HP_OUT_R Volume Control & HP_R Amp
										// bit08: Reserved
										// bit07: Enable LINE_IN Left Volume Control
										// bit06: Enable LINE_IN Right Volume Control
										// bit05: Enable AUXIN Left Volume Control
										// bit04: Enable AUXIN Right Volume Control
										// bit03: Enable MIC1 Boost + differential mixer + volume Amp control				  ---
										// bit02: Enable MIC2 Boost + differential mixer + volume Amp control
												// bit01: Enable MIC1 adboost + addifferential mixer								  ---
										// bit00: Enable MIC2 adboost + addifferential mixer
	#endif
	// AVC Function start

	// i2cWrite_ALC5621(104,0x000b);		// 68h, 1100_0000_0000_1011 ,moitor window = 2^(11+1)=4096, window is smaller, resopnse is faster


	 i2cWrite_ALC5621(106,0x0021);		 // 6ah, The Max PCM absolute level after AVC, Thmax (= 0~2^15-1)
	 i2cWrite_ALC5621(108,0x1000);



	 i2cWrite_ALC5621(106,0x0022);		 // 6ah, The Min PCM absolute level after AVC, Thmin (= 0~2^15-1)
	 i2cWrite_ALC5621(108,0x0950);


	 i2cWrite_ALC5621(106,0x0023);		 // 6ah, The Non-active PCM absolute level AVC will keep analog unit gain, Thnonact (= 0~2^15-1)
	// i2cWrite_ALC5621(108,0x0001);
	 i2cWrite_ALC5621(108,0x0450);


	 i2cWrite_ALC5621(106,0x0024);		 // 6ah, The CNTMAXTH1 to control the sensitivity to increase Gain (unit: 2^1)
	// i2cWrite_ALC5621(108,0x005f);
    // i2cWrite_ALC5621(108,0x01ff);
	// i2cWrite_ALC5621(108,0x007f);
     i2cWrite_ALC5621(108,0x0001);       // if fill 0x0000, besides mic won't saturation
	// i2cWrite_ALC5621(108,0x00ff);


	 i2cWrite_ALC5621(106,0x0025);		 // 6ah, The CNTMAXTH2 to control the sensitivity to decrease Gain (unit: 2^1)
	// i2cWrite_ALC5621(108,0x0060);     // CNTMAXTH2 = moitor window/2 = 8/2=4
	// i2cWrite_ALC5621(108,0x0200);
	// i2cWrite_ALC5621(108,0x0080);
	 i2cWrite_ALC5621(108,0x0001);
	// i2cWrite_ALC5621(108,0x0100);


	// i2cWrite_ALC5621(104,0x9005);	   // 68h, 1011_0000_0000_0111 ,moitor window = 2^(7+1)=256, window is smaller, resopnse is faster
	// i2cWrite_ALC5621(104,0xb005);       // bit13: o: keep unit gain
	// i2cWrite_ALC5621(104,0x900b);             1: keep previous gain

    i2cWrite_ALC5621(104,0x1005);         // disable AVC ************************


	/* use AVC

	1. reg22 : 0500 (boost 20dB)
	2. index21 : 1400
	3. index22 : 1200
	4. index23 : 0001
	5. index24 : 000F
	6. index25 : 0010
	7. reg68 : B007

	none use AVC
	1. reg22 : 0500 (boost 20dB)
	2. reg12 : FE9D (ADC gain 27dB)

	*/


	// AVC Function end


			// EQ Function start

#if 0
			i2cWrite_ALC5621(98,0x8800);	// 62h, EQ Control

			i2cWrite_ALC5621(106,0x0012);   // 6ah, index12 --- EQ output volume control
			//i2cWrite_ALC5621(108,0x0007);   // 6ch, 18dB
			i2cWrite_ALC5621(108,0x0005);   // 6ch, 12dB
           #endif


           #if 0
            //==============       cutoff low frequency noise S     =============//

            i2cWrite_ALC5621(98,0xc810);     // 62h, EQ Control
            i2cWrite_ALC5621(106,0x000b);
				 i2cWrite_ALC5621(108,0x0d41);	  // fc=1000Hz

            i2cWrite_ALC5621(102,0x0000);    // 66h, EQ Mode Change Enable
            i2cWrite_ALC5621(102,0x001f);    // 66h, EQ Mode Change Enable

			i2cWrite_ALC5621(106,0x0012);    // 6ah, index12 --- EQ output volume control
			i2cWrite_ALC5621(108,0x0007);    // 6ch, 18dB



            //==============       cutoff low frequency noise E     =============//
			#endif


#if 1

				// i2cWrite_ALC5621(98,0xc81f);	  // 62h, EQ Control     All
				// i2cWrite_ALC5621(98,0xc810);	  // 62h, EQ Control     HP
				 i2cWrite_ALC5621(98,0xc811);	  // 62h, EQ Control     LP+HP

				 i2cWrite_ALC5621(106,0x0000);	  // 6ah, index 00       LP0=100Hz, -12dB
				 i2cWrite_ALC5621(108,0x1757);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0001);	  // 6ah, index 01
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0002);	  // 6ah, index 02
				 i2cWrite_ALC5621(108,0xcf6c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0003);	  // 6ah, index 03
				 i2cWrite_ALC5621(108,0x10bb);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0004);	  // 6ah, index 04
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0005);	  // 6ah, index 05
				 i2cWrite_ALC5621(108,0xcf6c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0006);	  // 6ah, index 06
				 i2cWrite_ALC5621(108,0x10bb);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0007);	  // 6ah, index 07
				 i2cWrite_ALC5621(108,0xf405);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0008);	  // 6ah, index 08
				 i2cWrite_ALC5621(108,0xdb82);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x0009);	  // 6ah, index 09
				 i2cWrite_ALC5621(108,0x139c);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x000a);	  // 6ah, index 0a
				 i2cWrite_ALC5621(108,0x0c73);	  // 6ch, index data

				 i2cWrite_ALC5621(106,0x000b);	  // 6ah, index 0b
			//	 i2cWrite_ALC5621(108,0x139c);	  // 6ch, index data      HP0=600Hz
			//	 i2cWrite_ALC5621(108,0x193a);	  // 6ch, index data
			//	 i2cWrite_ALC5621(108,0x1c6f);	  // 6ch, index data      HP0=150Hz
				 i2cWrite_ALC5621(108,0x173f);	  // 6ch, index data      HP0=400Hz


				 i2cWrite_ALC5621(102,0x0000);	  // 66h, EQ Mode Change Enable
				 i2cWrite_ALC5621(102,0x001f);	  // 66h, EQ Mode Change Enable


				 i2cWrite_ALC5621(106,0x0012);	  // 6ah, index12 --- EQ output volume control
				 i2cWrite_ALC5621(108,0x0007);	  // 6ch, 18dB
			//	 i2cWrite_ALC5621(108,0x0006);	  // 6ch, 15dB,  default
			//	 i2cWrite_ALC5621(108,0x0005);	  // 6ch, 12dB
			//   i2cWrite_ALC5621(108,0x0004);	  // 6ch, 9dB


#endif


			// EQ Function end



			// i2cWrite_ALC5621(54,0x0668);	 // 36h, Stereo AD/DA Clock Control, ?k sampling rate
			 i2cWrite_ALC5621(54,0x0a11);
			// i2cWrite_ALC5621(16,0x60f0);  // 10h, Enable MIC1 Single End Input



             #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
                (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
               i2cWrite_ALC5621(0x10,0xe0e0);	 //10h, Aux-in, Aux-out ,Disable MIC1,MIC2 Single End Input
             #else
			    #if(AUDIO_BYPASS ==1)
    			i2cWrite_ALC5621(0x10,0x60e0);	 // 10h, Enable MIC1 Single End Input
                #else
                i2cWrite_ALC5621(0x10,0xe0e0);	 // 10h, Disable MIC1,MIC2 Single End Input
                #endif
             #endif
             i2cWrite_ALC5621(10,0x6808);	 // 0ah, Line-In

			// i2cWrite_ALC5621(34,0x0a00);    // 22h, MIC1,2 Boost 30dB	--- very important
			 i2cWrite_ALC5621(34,0x0000);  // 22h, MIC1,2 Boost 20dB	--- very important //albert on 20090713
			// i2cWrite_ALC5621(34,0x0000);	 // 22h, MIC1,2 Boost Bypass  --- very important


#if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
    (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
            i2cWrite_ALC5621(20,0x7777);	 // 14h, ADC Record Mixer Control
#else
			 i2cWrite_ALC5621(20,0x3f3f);	 // 14h, ADC Record Mixer Control
#endif
			// i2cWrite_ALC5621(20,0x3b3b);  // 14h, ADC Record Mixer Control
			// i2cWrite_ALC5621(20,0x7b7b);  // 14h, ADC Record Mixer Control
			i2cWrite_ALC5621(14,0x0202);  // 0eh, Mic volume  07/10//albert on 20090713

		    // i2cWrite_ALC5621(18,0xffff);	 // 12h, ADC Record Gain 30  dB Gain  [1111][11111][11][11111]
		    // i2cWrite_ALC5621(18,0xfe7c);	 // 12h, ADC Record Gain 25.5dB Gain  [1111][11100][11][11100]
		    // i2cWrite_ALC5621(18,0xfdfb);	 // 12h, ADC Record Gain 24  dB Gain  [1111][11011][11][11011]
		    // i2cWrite_ALC5621(18,0xfd7a);	 // 12h, ADC Record Gain 22.5dB Gain  [1111][11010][11][11010]

            i2cWrite_ALC5621(18,0xf6eB);    // 12h, ADC Record Gain 19.5dB Gain  [1111][11000][11][11000]//albert on 20090713
            // i2cWrite_ALC5621(18,0xf7ef);	 // 12h, ADC Record Gain 6dB Gain     [1111][01111][11][01111]
			// i2cWrite_ALC5621(18,0xf5eb);	 // 12h, ADC Record Gain 0dB Gain     [1111][01011][11][01011]

            #if(AUDIO_BYPASS == 1)
            i2cWrite_ALC5621(0x1C,0xc300);
            i2cWrite_ALC5621(0x04,0x0000);
            ALC5621_AdjustMICVolume(sysVolumnControl);
            #endif

}

#endif



#if 0

void Init_IIS_ALC5621_rec()
{
	 i2cWrite_ALC5621(52,0x8000);      // 34h, Audio Interface, slave mode, bclk, sclk from IIS controller, 16bit
                                                 // bit15, 0: master, 1: slave
                                                 // bit5: ADC data appear at right phase of LRCK, 0: left phase
                                                 //                                                                     1: right phase

     #if 0

	 i2cWrite_ALC5621(58,0xbfff);      // 3ah, power management

	 i2cWrite_ALC5621(60,0xffff);      // 3ch, power management

	 i2cWrite_ALC5621(62,0xffff);      // 3eh, power management

     #endif



     #if 1

	 i2cWrite_ALC5621(58,0xcc00);      // 3ah, power management

	 i2cWrite_ALC5621(60,0x27c3);      // 3ch, power management

	 i2cWrite_ALC5621(62,0x8002);      // 3eh, power management

     #endif


  // i2cWrite_ALC5621(54,0x0a11);      // 36h, Stereo AD/DA Clock Control, ?k sampling rate
	 i2cWrite_ALC5621(54,0x0668);

  // i2cWrite_ALC5621(16,0x70f0);      // 10h, Enable MIC1 Diff. Input
  	 i2cWrite_ALC5621(16,0xe0f0);	   // 10h, Enable MIC1 Single End Input


	 i2cWrite_ALC5621(10,0x6808);      // 0ah, Line-In

	 // i2cWrite_ALC5621(34,0x0a00);	// 22h, MIC1,2 Boost 30dB	 --- very important
	 // i2cWrite_ALC5621(34,0x0500);	// 22h, MIC1,2 Boost 20dB	 --- very important
	 i2cWrite_ALC5621(34,0x0000);  		// 22h, MIC1,2 Boost Bypass  --- very important

	 i2cWrite_ALC5621(20,0x3f3f);      // 14h, ADC Record Mixer Control
	 i2cWrite_ALC5621(18,0xffff);      // 12h, ADC Record Gain 30dB Gain

}

#endif



void Close_IIS_ALC5621()
{
    #if (AUDIO_BYPASS == 1)
    i2cWrite_ALC5621(0x10,0xe0e0); // 10h, Disable MIC1/MIC2 Single End Input
    #endif

    i2cWrite_ALC5621(58,0);
    i2cWrite_ALC5621(60,0);
    i2cWrite_ALC5621(62,0);
}

#endif

#if (G_SENSOR == G_SENSOR_LIS302DL)     // for ST LIS302DL G sensor control

s32 i2cWrite_LIS302DL(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_LIS302DL i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_LIS302DL_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);

	return 1;
}

s32 i2cRead_LIS302DL(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_LIS302DL i2cSemReq is %d.\n", err);
		return 0;
	}


	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_60 | (I2C_LIS302DL_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

void i2cInit_LIS302DL(void)
{
    u8  data;

#if 0
    i2cWrite_LIS302DL(0x21, 0x27);
    i2cWrite_LIS302DL(0x22, 0xff);
    i2cWrite_LIS302DL(0x38, 0x95);
    i2cRead_LIS302DL (0x39, &data);
#endif

    // for data polling mode by IRQ
    i2cWrite_LIS302DL(0x20, 0x67);
    i2cWrite_LIS302DL(0x21, 0x00);
    i2cWrite_LIS302DL(0x22, 0x84);
    i2cRead_LIS302DL (0x27, &data);

    // check write data
    i2cRead_LIS302DL (0x20, &data);
    DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x(0x67)\n", 20, data);
    i2cRead_LIS302DL (0x21, &data);
    DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x(0x00)\n", 21, data);
    i2cRead_LIS302DL (0x22, &data);
    DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x(0x84)\n", 22, data);

}

u32 i2cPolling_LIS302DL(void)
{
    u8  status;
    s8  out_X, out_Y, out_Z;
    u32 GSquare, GThreshold;
    u8  GSensor_SetVal;

    i2cRead_LIS302DL(0x27, &status);
    i2cRead_LIS302DL(0x29, &out_X);
    i2cRead_LIS302DL(0x2b, &out_Y);
    i2cRead_LIS302DL(0x2d, &out_Z);

#if G_SENSOR_DETECT
    // GSquare = 200 x G value suqare, 1G = 200, 2G = 800, 3G = 1800...etc
    GSquare     = out_X * out_X + out_Y * out_Y + out_Z * out_Z;
    GThreshold  = 512;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        GSensorEvent    = 1;
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
/*
#if 1
    DEBUG_I2C("LIS302DL(S, X, Y, Z) = (%5d, %4d, %4d, %4d)\n",
                GSquare, out_X, out_Y, out_Z);
    return  0;
#else
    GSensor_SetVal  = gpio_Get_GSensor_SetVal();
    GThreshold      = 50 * GSensor_SetVal * GSensor_SetVal;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
*/
    return 0;
}

void i2cGet_LIS302DL_XYZ(s8 *out_X, s8 *out_Y, s8 *out_Z)
{
    u8  status;

    i2cRead_LIS302DL(0x27, &status);
    i2cRead_LIS302DL(0x29, out_X);
    i2cRead_LIS302DL(0x2b, out_Y);
    i2cRead_LIS302DL(0x2d, out_Z);

    //DEBUG_I2C("LIS302DL(X, Y, Z) = (%4d, %4d, %4d)\n", *out_X, *out_Y, *out_Z);
}

/*
void i2cTest_LIS302DL(void)
{
    u8  i, data;

    DEBUG_I2C("IIC write testing...\n");
    for(i = 0; i < 0x40; i++) {
        i2cWrite_LIS302DL(i, i);
    }

    DEBUG_I2C("IIC read testing...\n");
    for(i = 0; i < 0x40; i++) {
        i2cRead_LIS302DL(i, &data);
    }
}
*/

#elif (G_SENSOR == G_SENSOR_H30CD)   // for Hitachi H30CD

s32 i2cWrite_H30CD(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_H30CD i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_240 | (I2C_H30CD_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_H30CD i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_H30CD(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_H30CD i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_240 | (I2C_H30CD_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);

    OSSemPost(i2cSemReq);
	return 1;
#else
	if (!gpio_IIC_Read(IIC_DEV_H30CD, I2C_H30CD_WR_SLAV_ADDR, addr, pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif

}

s32 i2cRead_H30CD_Word(u8 addr, u32* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_H30CD_Word i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_4B | I2C_CLK_DIV_60 | (I2C_H30CD_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u32)I2cData;
	//DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
	OSSemPost(i2cSemReq);
 	return 1;
#else
	if (!gpio_IIC_Read_Word(IIC_DEV_H30CD, I2C_H30CD_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: gpio_IIC_Read_Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif
}

s32 i2cRead_H30CD_2Word_Manual(u8 addr, u32* pData)
{
	u8  err;
	u32 i;
	u8  *pucData;

	pucData = (u8*)pData;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual i2cSemReq is %d.\n", err);
		return 0;
	}

	// write register address for read two word data
#if 0   // Normal mode IIC
	I2cCtrl = I2C_ENA | I2C_R_ACK | I2C_1B | I2C_CLK_DIV_240 | (I2C_H30CD_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cData = ((u32)addr);
	I2cCtrl = I2C_TRIG | I2cCtrl;
	for(i = 0; (i < 100) && (I2cCtrl & I2C_BUSY); i++);
	if(I2cCtrl & I2C_BUSY)
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Write Register Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#else   // Manual mode IIC
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_240 | (I2C_H30CD_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_H30CD_WR_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSubaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Write Sub Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#endif

    // Read 8 bytes data
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_240 | (I2C_H30CD_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_H30CD_RD_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 3);
		return 0;
	}
	pucData[3]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 2);
		return 0;
	}
	pucData[2]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 1);
		return 0;
	}
	pucData[1]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 0);
		return 0;
	}
	pucData[0]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 7);
		return 0;
	}
	pucData[7]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 6);
		return 0;
	}
	pucData[6]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 5);
		return 0;
	}
	pucData[5]  = (u8)I2cData;
	I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 4);
		return 0;
	}
	pucData[4]  = (u8)I2cData;
	I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word_Manual() IIC Stop Error!!!\n");
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}


s32 i2cRead_H30CD_2Word(u8 addr, u32* pData)
{
#if (GPIO_I2C_ENA )
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_H30CD_2Word i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Read_2Word(IIC_DEV_H30CD, I2C_H30CD_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_2Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#else
	return IIC_Read_2Word(I2C_H30CD_RD_SLAV_ADDR, addr, pData);
#endif

	//DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
}

s32 i2cRead_H30CD_NextWord(u32* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_H30CD_NextWord i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_4B | I2C_CLK_DIV_240 | (I2C_H30CD_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_H30CD_NextWord i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u32)I2cData;
	//DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWriteCommand_H30CD(u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWriteCommand_H30CD i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_240 | (I2C_H30CD_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWriteCommand_H30CD i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

void i2cInit_H30CD(void)
{
    DEBUG_I2C("i2cWrite_H30CD(0x0f, 0x00)\n");
    i2cWrite_H30CD(0x0f, 0x00);     // Temporal CTRL_REG0

    DEBUG_I2C("i2cWrite_H30CD(0x14, 0x00)\n");
    i2cWrite_H30CD(0x14, 0x00);     // Temporal CTRL_REG1

    DEBUG_I2C("i2cWrite_H30CD(0x15, 0x00)\n");
    i2cWrite_H30CD(0x15, 0x00);     // Temporal CTRL_REG2

    DEBUG_I2C("i2cWriteCommand_H30CD(2) R_Mode\n");
    i2cWriteCommand_H30CD(2);       // R_Mode

    //DEBUG_I2C("i2cWriteCommand_H30CD(5) F3_Mode\n");
    //i2cWriteCommand_H30CD(5);       // F3_Mode
}

u32 i2cPolling_H30CD(void)
{
    s16 out_X, out_Y, out_Z;
    u32 GSquare, GThreshold;
    u8  GSensor_SetVal;

    i2cGet_H30CD_XYZ(&out_X, &out_Y, &out_Z);

#if G_SENSOR_DETECT
    // GSquare = 200 x G value suqare, 1G = 200, 2G = 800, 3G = 1800...etc
    GSquare     = out_X * out_X + out_Y * out_Y + out_Z * out_Z;
    GThreshold  = 1000000;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        GSensorEvent    = 1;
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
/*
#if 1
    DEBUG_I2C("H30CD(S, X, Y, Z) = (%5d, %4d, %4d, %4d)\n",
                GSquare, out_X, out_Y, out_Z);
    return  0;
#else
    GSensor_SetVal  = gpio_Get_GSensor_SetVal();
    GThreshold      = 50 * GSensor_SetVal * GSensor_SetVal;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
*/
}

void i2cGet_H30CD_XYZ(s16 *out_X, s16 *out_Y, s16 *out_Z)
{
    u8          bData1, bData2;
    s16         hwData;
    u32         wData1, wData2;
    u32         wData[2];
    static  u8  EvenOdd = 0;;

#if 0   // read by byte
    i2cRead_H30CD(0x80, &bData1);
    OSTimeDly(1);
    i2cRead_H30CD(0x81, &bData2);
    OSTimeDly(1);
    hwData  = (((u16)bData1) << 8) | bData2;
    *out_X  = hwData >> 4;

    i2cRead_H30CD(0x82, &bData1);
    OSTimeDly(1);
    i2cRead_H30CD(0x83, &bData2);
    OSTimeDly(1);
    hwData  = (((u16)bData1) << 8) | bData2;
    *out_Y  = hwData >> 4;

    i2cRead_H30CD(0x84, &bData1);
    OSTimeDly(1);
    i2cRead_H30CD(0x85, &bData2);
    OSTimeDly(1);
    hwData  = (((u16)bData1) << 8) | bData2;
    *out_Z  = hwData >> 4;
#elif 0   // read by word
    //EvenOdd++;
    //if(EvenOdd & 1)
    {
        //i2cRead_H30CD_Word(0x80, &wData1);
        //*out_X  = (s16)((s32)(wData1 & 0xfff00000) >> 20);
        //*out_Y  = (s16)((s16)(wData1 & 0x0000fff0) >>  4);
        i2cRead_H30CD_Word(0x80, &wData[0]);
        OSTimeDly(1);
        //i2cRead_H30CD_NextWord(&wData[1]);
        *out_X  = (s16)((s32)(wData[0] & 0xfff00000) >> 20);
        *out_Y  = (s16)((s16)(wData[0] & 0x0000fff0) >>  4);
        //DEBUG_I2C("0x%08x  ", wData);
        //DEBUG_I2C("          \r");
    }
    //else
    {
        //i2cRead_H30CD_Word(0x84, &wData2);
        //i2cRead_H30CD_NextWord(&wData2);
        //*out_Z  = (s16)((s32)(wData2 & 0xfff00000) >> 20);
        i2cRead_H30CD_Word(0x84, &wData[1]);
        OSTimeDly(1);
        *out_Z  = (s16)((s32)(wData[1] & 0xfff00000) >> 20);
        //hwData  = (s16)((s16)(wData2 & 0x0000fff0) >>  4);
        //DEBUG_I2C("0x%08x\n", wData);
    }
#elif((CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B) ||(CHIP_OPTION == CHIP_A1016A)||(CHIP_OPTION == CHIP_A1018A)||(CHIP_OPTION == CHIP_A1018B)||\
(CHIP_OPTION == CHIP_A1026A))  // read 2 word
    i2cRead_H30CD_2Word(0x80, &wData[0]);
    *out_X  = (s16)((s32)(wData[0] & 0xfff00000) >> 20);
    *out_Y  = (s16)((s16)(wData[0] & 0x0000fff0) >>  4);
    *out_Z  = (s16)((s32)(wData[1] & 0xfff00000) >> 20);
#else   // read 2 word by I2C manual mode
    i2cRead_H30CD_2Word_Manual(0x80, &wData[0]);
    *out_X  = (s16)((s32)(wData[0] & 0xfff00000) >> 20);
    *out_Y  = (s16)((s16)(wData[0] & 0x0000fff0) >>  4);
    *out_Z  = (s16)((s32)(wData[1] & 0xfff00000) >> 20);
#endif

    //DEBUG_I2C("H30CD(X, Y, Z) = (%5d, %5d, %5d), %.2fC\n", *out_X, *out_Y, *out_Z, (float)hwData / 32);
}

void i2cTest_H30CD(void)
{
    u16 i;
    u8  data;
    s16 out_X, out_Y, out_Z;

    DEBUG_I2C("IIC read testing...\n");
    //while(1)
    //{
    //    for(i = 0; i <= 0xff; i++) {
    //        i2cRead_H30CD((u8)i, &data);
    //        DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%02x\n", i, data);
    //    }
    //}

    //DEBUG_I2C("i2cWrite_H30CD(0x14, 0x00)\n");
    //i2cWrite_H30CD(0x14, 0x00);     // Temporal CTRL_REG1

    /*
    i2cRead_H30CD(0x14, &data);
    DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%02x\n", 0x14, data);
    i2cRead_H30CD(0x54, &data);
    DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%02x\n", 0x54, data);
    i2cRead_H30CD(0xd4, &data);
    DEBUG_I2C("read H30CD i2c addr = 0x%02x, i2c data = 0x%02x\n", 0xd4, data);
    */
    DEBUG_I2C("i2cWriteCommand_H30CD(2) R_Mode\n");
    i2cWriteCommand_H30CD(2);   // R_Mode
    //while(1);
    /*
    while(1)
    {
        i2cWriteCommand_H30CD(2);   // R_Mode
    }
    */

    while(1)
    {
        i2cGet_H30CD_XYZ(&out_X, &out_Y, &out_Z);
        DEBUG_I2C("H30CD(X, Y, Z) = (%5d, %5d, %5d)\n", out_X, out_Y, out_Z);
        OSTimeDly(1);
    }

}

#elif (G_SENSOR == G_SENSOR_DMARD03)   // for Domintech DMARD03

s32 i2cWrite_DMARD03(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_DMARD03 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_240 | (I2C_DMARD03_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_DMARD03 i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_DMARD03(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_DMARD03 i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_240 | (I2C_DMARD03_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03 i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read DMARD03 i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	OSSemPost(i2cSemReq);
	return 1;
#else
	if (!gpio_IIC_Read(IIC_DEV_DMARD03, I2C_DMARD03_WR_SLAV_ADDR, addr, pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif

}

s32 i2cRead_DMARD03_HalfWord(u8 addr, u16* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_DMARD03_Word i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_240 | (I2C_DMARD03_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u16)(I2cData & 0xffff);
	//DEBUG_I2C("read DMARD03 i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
	OSSemPost(i2cSemReq);
 	return 1;
#else
	if (!gpio_IIC_Read_Word(IIC_DEV_DMARD03, I2C_DMARD03_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: gpio_IIC_Read_Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif
}

s32 i2cRead_DMARD03_Word(u8 addr, u32* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_DMARD03_Word i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_4B | I2C_CLK_DIV_240 | (I2C_DMARD03_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u32)I2cData;
	//DEBUG_I2C("read DMARD03 i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
	OSSemPost(i2cSemReq);
 	return 1;
#else
	if (!gpio_IIC_Read_Word(IIC_DEV_DMARD03, I2C_DMARD03_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: gpio_IIC_Read_Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif
}

s32 i2cRead_DMARD03_2Word_Manual(u8 addr, u32* pData)
{
	u8  err;
	u32 i;
	u8  *pucData;

	pucData = (u8*)pData;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual i2cSemReq is %d.\n", err);
		return 0;
	}

	// write register address for read two word data
#if 0   // Normal mode IIC
	I2cCtrl = I2C_ENA | I2C_R_ACK | I2C_1B | I2C_CLK_DIV_240 | (I2C_DMARD03_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cData = ((u32)addr);
	I2cCtrl = I2C_TRIG | I2cCtrl;
	for(i = 0; (i < 100) && (I2cCtrl & I2C_BUSY); i++);
	if(I2cCtrl & I2C_BUSY)
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Write Register Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#else   // Manual mode IIC
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_240 | (I2C_DMARD03_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_DMARD03_WR_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSubaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Write Sub Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#endif

    // Read 8 bytes data
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_240 | (I2C_DMARD03_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_DMARD03_RD_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 3);
		return 0;
	}
	pucData[3]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 2);
		return 0;
	}
	pucData[2]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 1);
		return 0;
	}
	pucData[1]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 0);
		return 0;
	}
	pucData[0]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 7);
		return 0;
	}
	pucData[7]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 6);
		return 0;
	}
	pucData[6]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 5);
		return 0;
	}
	pucData[5]  = (u8)I2cData;
	I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 4);
		return 0;
	}
	pucData[4]  = (u8)I2cData;
	I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DMARD03_2Word_Manual() IIC Stop Error!!!\n");
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}


s32 i2cWriteCommand_DMARD03(u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWriteCommand_DMARD03 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_240 | (I2C_DMARD03_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
        OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWriteCommand_DMARD03 i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

void i2cInit_DMARD03(void)
{

}

u32 i2cPolling_DMARD03(void)
{
    s16 out_T, out_X, out_Y, out_Z;
    u32 GSquare, GThreshold;
    u8  GSensor_SetVal;

    i2cGet_DMARD03_TXYZ(&out_T, &out_X, &out_Y, &out_Z);

#if G_SENSOR_DETECT
    // GSquare = 200 x G value suqare, 1G = 200, 2G = 800, 3G = 1800...etc
    GSquare     = out_X * out_X + out_Y * out_Y + out_Z * out_Z;
    GThreshold  = 1000000;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        GSensorEvent    = 1;
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
/*
#if 1
    DEBUG_I2C("DMARD03(S, X, Y, Z) = (%5d, %4d, %4d, %4d)\n",
                GSquare, out_X, out_Y, out_Z);
    return  0;
#else
    GSensor_SetVal  = gpio_Get_GSensor_SetVal();
    GThreshold      = 50 * GSensor_SetVal * GSensor_SetVal;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
*/
}

void i2cGet_DMARD03_TXYZ(s16 *out_T, s16 *out_X, s16 *out_Y, s16 *out_Z)
{
    s16     data1;

    i2cRead_DMARD03_HalfWord(0, (u16*)&data1);
    *out_T  = (data1 >> 5) | (data1 & 0x0003);
    i2cRead_DMARD03_HalfWord(2, (u16*)&data1);
    *out_X  = (data1 >> 5) | (data1 & 0x0003);
    i2cRead_DMARD03_HalfWord(4, (u16*)&data1);
    *out_Y  = (data1 >> 5) | (data1 & 0x0003);
    i2cRead_DMARD03_HalfWord(6, (u16*)&data1);
    *out_Z  = (data1 >> 5) | (data1 & 0x0003);

    //DEBUG_I2C("DMARD03(T, X, Y, Z) = (%5d, %5d, %5d, %5d)\n", *out_T, *out_X, *out_Y, *out_Z);
}

void i2cTest_DMARD03(void)
{
    DEBUG_I2C("i2cTest_DMARD03...\n");
    /*
#if 0
    u16 i;
    u8  data;
    s16 out_T, out_X, out_Y, out_Z, data1;

    while(1)
    {
        for(i = 0; i <= 0x0d; i++) {
            i2cRead_DMARD03((u8)i, &data);
            DEBUG_I2C("read DMARD03 i2c addr = 0x%02x, i2c data = 0x%02x\n", i, data);
        }
    }
#else
    while(1)
    {
        i2cRead_DMARD03_HalfWord(0, (u16*)&data1);
        out_T   = (data1 >> 5) | (data1 & 0x0003);
        i2cRead_DMARD03_HalfWord(2, (u16*)&data1);
        out_X   = (data1 >> 5) | (data1 & 0x0003);
        i2cRead_DMARD03_HalfWord(4, (u16*)&data1);
        out_Y   = (data1 >> 5) | (data1 & 0x0003);
        i2cRead_DMARD03_HalfWord(6, (u16*)&data1);
        out_Z   = (data1 >> 5) | (data1 & 0x0003);
        DEBUG_I2C("DMARD03(T, X, Y, Z) = (%5d, %5d, %5d, %5d)\n", out_T, out_X, out_Y, out_Z);
    }
#endif
    */

    while(1)
    {
        i2cGet_DMARD03_TXYZ(&out_T, &out_X, &out_Y, &out_Z);
        DEBUG_I2C("DMARD03(T, X, Y, Z) = (%5d, %5d, %5d, %5d)\n", out_T, out_X, out_Y, out_Z);
        OSTimeDly(1);
    }

}

#elif (G_SENSOR == G_SENSOR_BMA150)   // for Bosch BMA150

s32 i2cWrite_BMA150(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BMA150 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_60 | (I2C_BMA150_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
        OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BMA150 i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_BMA150(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BMA150 i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_60 | (I2C_BMA150_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
    	OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150 i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read BMA150 i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	OSSemPost(i2cSemReq);
	return 1;
#else
	if (!gpio_IIC_Read(IIC_DEV_BMA150, I2C_BMA150_WR_SLAV_ADDR, addr, pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif

}

s32 i2cRead_BMA150_HalfWord(u8 addr, u16* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BMA150_Word i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_2B | I2C_CLK_DIV_60 | (I2C_BMA150_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u16)(I2cData & 0xffff);
	//DEBUG_I2C("read BMA150 i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
	OSSemPost(i2cSemReq);
 	return 1;
#else
	if (!gpio_IIC_Read_Word(IIC_DEV_BMA150, I2C_BMA150_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: gpio_IIC_Read_Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif
}

s32 i2cRead_BMA150_Word(u8 addr, u32* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BMA150_Word i2cSemReq is %d.\n", err);
		return 0;
	}
#if 1
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_4B | I2C_CLK_DIV_60 | (I2C_BMA150_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u32)I2cData;
	//DEBUG_I2C("read BMA150 i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
	OSSemPost(i2cSemReq);
 	return 1;
#else
	if (!gpio_IIC_Read_Word(IIC_DEV_BMA150, I2C_BMA150_WR_SLAV_ADDR, addr, (u8*)pData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: gpio_IIC_Read_Word() gpio IIC Read Addr [%#x] Error!!!\n", addr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
#endif
}

s32 i2cRead_BMA150_2Word_Manual(u8 addr, u32* pData)
{
	u8  err;
	u32 i;
	u8  *pucData;

	pucData = (u8*)pData;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual i2cSemReq is %d.\n", err);
		return 0;
	}

	// write register address for read two word data
#if 0   // Normal mode IIC
	I2cCtrl = I2C_ENA | I2C_R_ACK | I2C_1B | I2C_CLK_DIV_60 | (I2C_BMA150_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cData = ((u32)addr);
	I2cCtrl = I2C_TRIG | I2cCtrl;
	for(i = 0; (i < 100) && (I2cCtrl & I2C_BUSY); i++);
	if(I2cCtrl & I2C_BUSY)
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Write Register Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#else   // Manual mode IIC
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_60 | (I2C_BMA150_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_BMA150_WR_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSubaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Write Sub Addr [%#x] Error!!!\n", addr);
		return 0;
	}
#endif

    // Read 8 bytes data
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_60 | (I2C_BMA150_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Write Slave Addr [%#x] Error!!!\n", I2C_BMA150_RD_SLAV_ADDR);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 3);
		return 0;
	}
	pucData[3]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 2);
		return 0;
	}
	pucData[2]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 1);
		return 0;
	}
	pucData[1]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 0);
		return 0;
	}
	pucData[0]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 7);
		return 0;
	}
	pucData[7]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 6);
		return 0;
	}
	pucData[6]  = (u8)I2cData;
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 5);
		return 0;
	}
	pucData[5]  = (u8)I2cData;
	I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Read Register Addr [%#x] Error!!!\n", addr + 4);
		return 0;
	}
	pucData[4]  = (u8)I2cData;
	I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BMA150_2Word_Manual() IIC Stop Error!!!\n");
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}


s32 i2cWriteCommand_BMA150(u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWriteCommand_BMA150 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_60 | (I2C_BMA150_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWriteCommand_BMA150 i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

void i2cInit_BMA150(void)
{

}

u32 i2cPolling_BMA150(void)
{
    s16 out_T, out_X, out_Y, out_Z;
    u32 GSquare, GThreshold;
    u8  GSensor_SetVal;

    i2cGet_BMA150_TXYZ(&out_T, &out_X, &out_Y, &out_Z);

#if G_SENSOR_DETECT
    // GSquare = 200 x G value suqare, 1G = 200, 2G = 800, 3G = 1800...etc
    GSquare     = out_X * out_X + out_Y * out_Y + out_Z * out_Z;
    GThreshold  = 1000000;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        GSensorEvent    = 1;
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
/*
#if 1
    DEBUG_I2C("BMA150(S, X, Y, Z) = (%5d, %4d, %4d, %4d)\n",
                GSquare, out_X, out_Y, out_Z);
    return  0;
#else
    GSensor_SetVal  = gpio_Get_GSensor_SetVal();
    GThreshold      = 50 * GSensor_SetVal * GSensor_SetVal;
    if(GSquare >= GThreshold) {    // G value >= G threshold, a traffic accident.
        DEBUG_I2C("GSquare(%d) >= GThreshold(%d)\n",  GSquare, GThreshold);
        return  1;
    } else {                                    // G value <  threshold
        return  0;
    }
#endif
*/
}

void i2cGet_BMA150_TXYZ(s16 *out_T, s16 *out_X, s16 *out_Y, s16 *out_Z)
{
    u16     data1;
    u8      data2;
    u32     data3;

#if 0
    i2cRead_BMA150_HalfWord(2, &data1);
    *out_X  = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
    i2cRead_BMA150_HalfWord(4, &data1);
    *out_Y  = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
    i2cRead_BMA150_HalfWord(6, &data1);
    *out_Z  = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
    i2cRead_BMA150(8, &data2);
    *out_T  = (s16)data2;
#else
    i2cRead_BMA150_Word(2, &data3);
    *out_X  = ((s16)(((data3 >> 24) | (data3 >> 8)) & 0xffff)) >> 6;
    *out_Y  = ((s16)((((data3 & 0x0000ff00) >> 8) | (data3 << 8)) & 0xffff)) >> 6;
    i2cRead_BMA150_Word(6, &data3);
    *out_Z  = ((s16)(((data3 >> 24) | (data3 >> 8)) & 0xffff)) >> 6;
    *out_T  = (s16)((data3 & 0x0000ff00) >> 8);
#endif

    //DEBUG_I2C("BMA150(T, X, Y, Z) = (%5d, %5d, %5d, %5d)\n", *out_T, *out_X, *out_Y, *out_Z);
}

void i2cTest_BMA150(void)
{
    u16 i;
    s16 out_T, out_X, out_Y, out_Z;
    u16 data1;

    DEBUG_I2C("i2cTest_BMA150...\n");

#if 1
    while(1)
    {
        for(i = 0; i <= 0x7f; i++) {
            i2cRead_BMA150((u8)i, &data);
            DEBUG_I2C("read BMA150 i2c addr = 0x%02x, i2c data = 0x%02x\n", i, data);
        }
    }
#else
    while(1)
    {
        i2cRead_BMA150_HalfWord(2, &data1);
        out_X   = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
        i2cRead_BMA150_HalfWord(4, &data1);
        out_Y   = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
        i2cRead_BMA150_HalfWord(6, &data1);
        out_Z   = ((s16)((data1 >> 8) | (data1 << 8))) >> 6;
        DEBUG_I2C("BMA150(X, Y, Z) = (%5d, %5d, %5d)\n", out_X, out_Y, out_Z);
    }
#endif

    while(1)
    {
        i2cGet_BMA150_TXYZ(&out_T, &out_X, &out_Y, &out_Z);
        DEBUG_I2C("BMA150(T, X, Y, Z) = (%5d, %5d, %5d, %5d)\n", out_T, out_X, out_Y, out_Z);
    }

}

#endif  // #if (G_SENSOR == G_SENSOR_LIS302DL)


//#if((HW_BOARD_OPTION==JSW_DVRBOX)||(HW_BOARD_OPTION==JSY_DVRBOX)) //use Bit1605 TV decoder
#if GPIO_I2C_ENA

s32 i2cWrite_BIT1605(u8 ucRegAddr, u8 ucData)
{
	u8  err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Write(IIC_DEV_BIT1605, I2C_BIT1605_WR_SLAV_ADDR, ucRegAddr, ucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1605() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

s32 i2cWrite_BIT1605_DV2(u8 ucRegAddr, u8 ucData)
{
	u8  err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Write(IIC_DEV_BIT1605, I2C_BIT1605_WR_SLAV_2_ADDR, ucRegAddr, ucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1605() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}


#else

s32 i2cWrite_BIT1605(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif
//	DEBUG_I2C("I2C : %x  %x\n",addr,data);
//   DEBUG_I2C("I2C write slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_WR_SLAV_ADDR, addr, data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_BIT1605_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1605(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_BIT1605_DV2(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif
//	DEBUG_I2C("I2C : %x  %x\n",addr,data);
//	DEBUG_I2C("I2C write slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_WR_SLAV_2_ADDR, addr, data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_BIT1605_WR_SLAV_2_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
        OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1605(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

#endif

s32 i2cWrite_WT8861(u8 addr, u8 data,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_WT8861 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
        OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_WT8861(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}


s32 i2cRead_WT8861(u8 addr, u8* pData,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_WT8861 i2cSemReq is %d.\n", err);
		return 0;
	}

   	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_WT8861(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_DM5900(u8 addr, u8 data,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TW9900 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_DM5900(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_DM5900(u8 addr, u8* pData,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_DM5900 i2cSemReq is %d.\n", err);
		return 0;
	}

   	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_DM5900(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_TW9900(u8 addr, u8 data,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TW9900 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TW9900(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_TW9900(u8 addr, u8* pData,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_TW9900 i2cSemReq is %d.\n", err);
		return 0;
	}

   	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW9900(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}
s32 i2cWrite_TW9910(u8 addr, u8 data,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TW9910 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TW9910(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_TW9910(u8 addr, u8* pData,u32 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_TW9910 i2cSemReq is %d.\n", err);
		return 0;
	}

   	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW9910(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;
#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}
#if (TV_DECODER == TW2866)

s32 i2cWrite_TW2866(u16 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TW2866 i2cSemReq is %d.\n", err);
		return 0;
	}
    
    I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_7680 | (I2C_TW2866_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TW2866 i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);

	return 1;
}

s32 i2cRead_TW2866_Manual(u16 ucRegAddr, u8* pucData)
{
    u8 err;
    u32 i;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_TW2866_Manual(0x%02x, 0x%02x), i2cSemReq is %d.\n",  ucRegAddr, *pucData, err);
        return 0;
    }

    I2cCtrl     = I2C_MANUAL_EN  |I2C_1B| I2C_ENA | I2C_CLK_DIV_3840  | (I2C_TW2866_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
    /* Start */
    I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_TW2866_WR_SLAV_ADDR);
		return 0;
	}
   
    I2cData     = (u32)ucRegAddr;
    /* Reg Addr (bit 0:7) */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData1;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);
		return 0;
	}

    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Stop 1 Error!!!\n");
		return 0;
	}
 

    I2cCtrl     = I2C_MANUAL_EN |I2C_ENA | I2C_1B | I2C_CLK_DIV_3840  | (I2C_TW2866_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
    /* Start */
	I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_TW2866_RD_SLAV_ADDR);
		return 0;
	}

    /* Read Data */
    I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Read Data Error!!!\n");
		return 0;
	}

    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
    *pucData  = (u8)I2cData;
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TW2866_Manual() IIC Stop 2 Error!!!\n");
		return 0;
	}
    else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}
#endif
#if GPIO_I2C_ENA

s32 i2cRead_BIT1605(u8 ucRegAddr, u8* pucData)
{
	u8  err;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Read(IIC_DEV_BIT1605, I2C_BIT1605_WR_SLAV_ADDR, ucRegAddr, pucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1605() gpio IIC Read Addr [%#x] Error\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

s32 i2cRead_BIT1605_DV2(u8 ucRegAddr, u8* pucData)
{
	u8  err;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Read(IIC_DEV_BIT1605, I2C_BIT1605_WR_SLAV_2_ADDR, ucRegAddr, pucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1605() gpio IIC Read Addr [%#x] Error\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#else

s32 i2cRead_BIT1605(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}
   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_BIT1605_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1605(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_BIT1605_DV2(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BIT1605 i2cSemReq is %d.\n", err);
		return 0;
	}
   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_BIT1605_RD_SLAV_2_ADDR<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1605(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_2_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

#endif


//#else

#if GPIO_I2C_ENA

s32 i2cWrite_TVP5150(u8 ucRegAddr, u8 ucData,u8 DeviceAddr)
{
	u8  err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TVP5150 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Write(IIC_DEV_TVP5150, DeviceAddr, ucRegAddr, ucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#else

s32 i2cWrite_TVP5150(u8 addr, u8 data,u8 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TVP5150 i2cSemReq is %d.\n", err);
		return 0;
	}
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif
#endif

	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150(), i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_TVP5150_Manual(u8 addr, u8 data,u8 DeviceAddr)
{
	u8 err;
    u32 i;
    
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual i2cSemReq is %d.\n", err);
		return 0;
	}
    
	I2cData = ((u32)data);

	//I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
    I2cCtrl = I2C_MANUAL_EN | I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr << I2C_SLAV_ADDR_SHFT) | I2C_RD_REG_ADDR;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);

    /* Start */
    I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", DeviceAddr);
		return 0;
	}

    /* Data Addr */
    I2cManu     = I2C_ManuSubaddr;
    for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", addr);
		return 0;
	}

    /* Write Data 1B*/
    I2cManu     = I2C_ManuWriteData1;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual() IIC Write Data Error!!!\n");
		return 0;
	}

    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TVP5150_Manual() IIC Stop 2 Error!!!\n");
		return 0;
	}
    else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
    
}
#endif

#if GPIO_I2C_ENA

s32 i2cRead_TVP5150(u8 ucRegAddr, u8* pucData,u8 DeviceAddr)
{
	u8  err;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TVP5150 i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Read(IIC_DEV_TVP5150, DeviceAddr & 0xfe, ucRegAddr, pucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150() gpio IIC Read Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#else

s32 i2cRead_TVP5150(u8 addr, u8* pData, u8 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_TVP5150 i2cSemReq is %d.\n", err);
		return 0;
	}
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
   #if(HW_BOARD_OPTION == A1013_REALCHIP_A)
    I2cMISC=I2C_CLK_DIV_3840;
   #else
    I2cMISC=I2C_CLK_DIV_1920;
   #endif
#endif

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (DeviceAddr << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_TVP5150_Manual(u8 addr, u8* pData, u8 DeviceAddr)
{
    u8 err;
    u32 i;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_TVP5150_Manual(0x%02x, 0x%02x), i2cSemReq is %d.\n",  addr, *pData, err);
        return 0;
    }

    //I2cCtrl     = I2C_MANUAL_EN  |I2C_1B| I2C_ENA | I2C_CLK_DIV_3840  | (I2C_TW2866_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);  // TW2866
    I2cCtrl = I2C_MANUAL_EN | I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | ((DeviceAddr-1) << I2C_SLAV_ADDR_SHFT) | I2C_RD_REG_ADDR;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);

    /* Start */
    I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", DeviceAddr);
		return 0;
	}
   
#if 0
    I2cData     = (u32)addr;
    /* Reg Addr (bit 0:7) */
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData1;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", addr);
		return 0;
	}
#else
    I2cManu     = I2C_ManuSubaddr;
    for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", addr);
		return 0;
	}
#endif

    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Stop 1 Error!!!\n");
		return 0;
	}
 

    I2cCtrl     = I2C_MANUAL_EN |I2C_ENA | I2C_1B | I2C_CLK_DIV_480  | (DeviceAddr<< I2C_SLAV_ADDR_SHFT);
    /* Start */
	I2cManu     = I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}

    /* Slave Addr */
    I2cManu     = I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", DeviceAddr);
		return 0;
	}

    /* Read Data */
    I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Read Data Error!!!\n");
		return 0;
	}

    /* Stop */
    I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
    *pData  = (u8)I2cData;
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_TVP5150_Manual() IIC Stop 2 Error!!!\n");
		return 0;
	}
    else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}
}
#endif


#if GPIO_I2C_ENA
s32 i2cRead_BIT1201G(u8 ucRegAddr, u8* pucData,u8 DeviceAddr)
{
	u8  err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1201G i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Read(IIC_DEV_BIT1201G, DeviceAddr , ucRegAddr, pucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1201G() gpio IIC Read Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

s32 i2cWrite_BIT1201G(u8 ucRegAddr, u8 ucData,u8 DeviceAddr)
{
	u8  err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		/* signal event semaphore */
		OSSemPost(i2cSemReq);

		DEBUG_I2C("Error: i2cWrite_BIT1201G i2cSemReq is %d.\n", err);
		return 0;
	}

	if (!gpio_IIC_Write(IIC_DEV_BIT1201G, DeviceAddr, ucRegAddr, ucData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1201G() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#else
s32 i2cRead_BIT1201G(u8 ucRegAddr, u8* pucData,u8 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BIT1201G i2cSemReq is %d.\n", err);
		return 0;
	}

   	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BIT1201G(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pucData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_BIT1605_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;

}

s32 i2cWrite_BIT1201G(u8 ucRegAddr, u8 ucData,u8 DeviceAddr)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BIT1201G i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)ucData);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (DeviceAddr<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BIT1201G(), i2cSemFin is %d.\n", err);
		return 0;
	}
    OSSemPost(i2cSemReq);
	return 1;
}

#endif



#if(TV_ENCODER == BIT1201G)
void i2cInit_BIT1201G(void)
{
    int i;

    DEBUG_I2C("\n---i2cInit_BIT1201G---\n");
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //BIT1201G reset
    for(i=0;i<0xff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0xff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(1);

    #if(TV_D1_OUT_FULL)
    if(sysTVinFormat == TV_IN_NTSC)
    {
        DEBUG_I2C("***** NTSC \n");
        i2cWrite_BIT1201G(0x90, 0x01, I2C_BIT1201G_WR_SLAV_ADDR); //Horizontal phase adjustment    
    }
    else
    {
        DEBUG_I2C("***** PAL \n");
        i2cWrite_BIT1201G(0x90, 0x11, I2C_BIT1201G_WR_SLAV_ADDR); //Horizontal phase adjustment     
    }
    
    #endif


    i2cWrite_BIT1201G(0x9a, 0x70, I2C_BIT1201G_WR_SLAV_ADDR); //use CCIR601 format
    i2cWrite_BIT1201G(0x1e, 0x0a, I2C_BIT1201G_WR_SLAV_ADDR); //color burst offset  

    if(sysTVinFormat==TV_IN_PAL)
    {
        //i2cWrite_BIT1201G(0x02, 0xc2, I2C_BIT1201G_WR_SLAV_ADDR); //PAL mode 
        #if(HW_BOARD_OPTION == MR8600_RX_RDI)
        i2cWrite_BIT1201G(0x04, 0x7c, I2C_BIT1201G_WR_SLAV_ADDR); //PAL Y gain 
        i2cWrite_BIT1201G(0x06, 0x55, I2C_BIT1201G_WR_SLAV_ADDR); //PAL U gain 
        i2cWrite_BIT1201G(0x08, 0x84, I2C_BIT1201G_WR_SLAV_ADDR); //PAL V gain
        #elif(HW_BOARD_OPTION == MR8600_RX_MAYON)
        i2cWrite_BIT1201G(0x04, 0xC0, I2C_BIT1201G_WR_SLAV_ADDR); //PAL Y gain 
        i2cWrite_BIT1201G(0x06, 0x9d, I2C_BIT1201G_WR_SLAV_ADDR); //PAL U gain 
        i2cWrite_BIT1201G(0x07, 0x9d, I2C_BIT1201G_WR_SLAV_ADDR); //PAL V gain 
        i2cWrite_BIT1201G(0x14, 0x10, I2C_BIT1201G_WR_SLAV_ADDR); //PAL Y offset
        #else
        i2cWrite_BIT1201G(0x04, 0x98, I2C_BIT1201G_WR_SLAV_ADDR); //PAL Y gain 
        i2cWrite_BIT1201G(0x06, 0x80, I2C_BIT1201G_WR_SLAV_ADDR); //PAL U gain 
        i2cWrite_BIT1201G(0x07, 0xb0, I2C_BIT1201G_WR_SLAV_ADDR); //PAL V gain 
        i2cWrite_BIT1201G(0x14, 0x10, I2C_BIT1201G_WR_SLAV_ADDR); //PAL Y offset
        #endif
    }
    else if(sysTVinFormat == TV_IN_NTSC)
    {
        #if(HW_BOARD_OPTION == MR8600_RX_GCT)
        i2cWrite_BIT1201G(0x0a, 0x90, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y gain 
        i2cWrite_BIT1201G(0x0c, 0xa0, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC U gain 
        i2cWrite_BIT1201G(0x0e, 0xb0, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC V gain 
        #elif(HW_BOARD_OPTION == MR8600_RX_RDI)
        i2cWrite_BIT1201G(0x0a, 0x90, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y gain 
        i2cWrite_BIT1201G(0x0c, 0xa0, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC U gain 
        i2cWrite_BIT1201G(0x0e, 0xb0, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC V gain 
        #elif(HW_BOARD_OPTION == MR8600_RX_MAYON)
        i2cWrite_BIT1201G(0x0a, 0xa5, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y gain 
        i2cWrite_BIT1201G(0x0c, 0xa9, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC U gain 
        i2cWrite_BIT1201G(0x0e, 0xcf, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC V gain 
        i2cWrite_BIT1201G(0x12, 0x05, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y offset 
        #else
        i2cWrite_BIT1201G(0x0a, 0xb0, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y gain 
        i2cWrite_BIT1201G(0x0c, 0x80, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC U gain 
        i2cWrite_BIT1201G(0x0e, 0x80, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC V gain 
        i2cWrite_BIT1201G(0x12, 0x90, I2C_BIT1201G_WR_SLAV_ADDR); //NTSC Y offset
        #endif

    }   
     
}

#endif


#if (ECHO_IC == FM1288)
#if GPIO_I2C_ENA
u8 FM1288_mode0_Rx[] =
{
    0xC0,
    0xFC, 0xF3, 0x68, 0x64, 0x00,
    0xFC, 0xF3, 0x3B, 0x3F, 0xE8, 0x00, 0x14,
    0xFC, 0xF3, 0x0D, 0x3F, 0x80, 0x90, 0x94, 0x3E,
    0xFC, 0xF3, 0x89,
    0x93, 0x83, 0xDE, 0x19, 0x2A, 0xBF, 0x40, 0x05,
    0x08, 0x19, 0x3E, 0x7F, 0x80, 0x95, 0x2A, 0x2A,
    0x7A, 0xAA, 0x19, 0x28, 0x4F,
    0xFC, 0xF3, 0x68, 0x64, 0x00,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA0, 0x92, 0xAA,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB0, 0x3F, 0x80,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA1, 0x93, 0xE6,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB1, 0x3F, 0x83,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA2, 0x92, 0x82,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB2, 0x3F, 0x85,
    0xFC, 0xF3, 0x3B, 0x22, 0xE0, 0x3A, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xF6, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xF8, 0x80, 0x02,
    0xFC, 0xF3, 0x3B, 0x23, 0x07, 0xFC, 0xFC,
    0xFC, 0xF3, 0x3B, 0x23, 0x0D, 0x03, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xC8, 0x00, 0x29,
    0xFC, 0xF3, 0x3B, 0x22, 0xEE, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x0C, 0x11, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x2F, 0x00, 0x60,
    0xFC, 0xF3, 0x3B, 0x22, 0xF9, 0x00, 0x5F,
    0xFC, 0xF3, 0x3B, 0x22, 0xE5, 0x02, 0x88,
    0xFC, 0xF3, 0x3B, 0x23, 0x03, 0x0D, 0xE1,
    0xFC, 0xF3, 0x3B, 0x23, 0x04, 0x03, 0xCF,
    0xFC, 0xF3, 0x3B, 0x23, 0x05, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x23, 0x39, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x23, 0x48, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x49, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0xB7, 0x00, 0x10,
    0xFC, 0xF3, 0x3B, 0x23, 0xBA, 0x60, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x10, 0x12, 0x05,
    0xFC, 0xF3, 0x3B, 0x23, 0x32, 0x00, 0x30,
    0xFC, 0xF3, 0x3B, 0x23, 0x33, 0x00, 0x04,
    0xFC, 0xF3, 0x3B, 0x23, 0xB8, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x01, 0x00, 0x02,
    0xFC, 0xF3, 0x3B, 0x23, 0xF1, 0x3A, 0x11,
    0xFC, 0xF3, 0x3B, 0x23, 0xF2, 0xC5, 0xF8,
    0xFC, 0xF3, 0x3B, 0x23, 0xF3, 0x3A, 0x11,
    0xFC, 0xF3, 0x3B, 0x23, 0xF4, 0x91, 0x5C,
    0xFC, 0xF3, 0x3B, 0x23, 0xF5, 0x61, 0xAF,
    0xFC, 0xF3, 0x3B, 0x23, 0xF6, 0x73, 0x65,
    0xFC, 0xF3, 0x3B, 0x23, 0xF7, 0x8C, 0xFD,
    0xFC, 0xF3, 0x3B, 0x23, 0xF8, 0x73, 0x65,
    0xFC, 0xF3, 0x3B, 0x23, 0xF9, 0x83, 0xA9,
    0xFC, 0xF3, 0x3B, 0x23, 0xFA, 0x7A, 0xA4,
    0xFC, 0xF3, 0x3B, 0x23, 0xFB, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xFC, 0x80, 0x9D,
    0xFC, 0xF3, 0x3B, 0x23, 0xFD, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xFE, 0x81, 0x3D,
    0xFC, 0xF3, 0x3B, 0x23, 0xFF, 0x7F, 0x16,
    0xFC, 0xF3, 0x3B, 0x23, 0xC0, 0x01, 0xA1,
    0xFC, 0xF3, 0x3B, 0x23, 0xC1, 0xFE, 0x72,
    0xFC, 0xF3, 0x3B, 0x23, 0xC2, 0x01, 0xA1,
    0xFC, 0xF3, 0x3B, 0x23, 0xC3, 0x84, 0xC5,
    0xFC, 0xF3, 0x3B, 0x23, 0xC4, 0x76, 0xD3,
    0xFC, 0xF3, 0x3B, 0x23, 0xC5, 0x31, 0xDB,
    0xFC, 0xF3, 0x3B, 0x23, 0xC6, 0xCE, 0x93,
    0xFC, 0xF3, 0x3B, 0x23, 0xC7, 0x31, 0xDB,
    0xFC, 0xF3, 0x3B, 0x23, 0xC8, 0x82, 0x54,
    0xFC, 0xF3, 0x3B, 0x23, 0xC9, 0x7C, 0x48,
    0xFC, 0xF3, 0x3B, 0x23, 0xCA, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xCB, 0x80, 0xC9,
    0xFC, 0xF3, 0x3B, 0x23, 0xCC, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xCD, 0x81, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0xCE, 0x7F, 0x33,
    0xFC, 0xF3, 0x3B, 0x22, 0xF5, 0x80, 0x03,
    0xFC, 0xF3, 0x3B, 0x23, 0xB2, 0x00, 0x03,
    0xFC, 0xF3, 0x3B, 0x23, 0xB5, 0x50, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0xB3, 0x00, 0x08,
    0xFC, 0xF3, 0x3B, 0x3F, 0xD2, 0x06, 0x22,
    0xFC, 0xF3, 0x3B, 0x22, 0xF2, 0x00, 0x34,
    0xFC, 0xF3, 0x3B, 0x22, 0xE9, 0x00, 0x61,
    0xFC, 0xF3, 0x3B, 0x22, 0xFB, 0x00, 0x00
};
u8 FM1288_mode0_Tx[] =
{
    0xC0,
    0xFC, 0xF3, 0x68, 0x64, 0x00,
    0xFC, 0xF3, 0x3B, 0x3F, 0xE8, 0x00, 0x14,
    0xFC, 0xF3, 0x0D, 0x3F, 0x80, 0x90, 0x94, 0x3E,
    0xFC, 0xF3, 0x89,
    0x93, 0x83, 0xDE, 0x19, 0x2A, 0xBF, 0x40, 0x05,
    0x08, 0x19, 0x3E, 0x7F, 0x80, 0x95, 0x2A, 0x2A,
    0x7A, 0xAA, 0x19, 0x28, 0x4F,
    0xFC, 0xF3, 0x68, 0x64, 0x00,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA0, 0x92, 0xAA,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB0, 0x3F, 0x80,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA1, 0x93, 0xE6,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB1, 0x3F, 0x83,
    0xFC, 0xF3, 0x3B, 0x3F, 0xA2, 0x92, 0x82,
    0xFC, 0xF3, 0x3B, 0x3F, 0xB2, 0x3F, 0x85,
    0xFC, 0xF3, 0x3B, 0x22, 0xE0, 0x3A, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xF6, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xF8, 0x80, 0x02,
    0xFC, 0xF3, 0x3B, 0x23, 0x07, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x0D, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xC8, 0x00, 0x29,
    0xFC, 0xF3, 0x3B, 0x22, 0xEE, 0x00, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x0C, 0x20, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xF9, 0x00, 0x5F,
    0xFC, 0xF3, 0x3B, 0x22, 0xE5, 0x02, 0x88,
    0xFC, 0xF3, 0x3B, 0x23, 0x03, 0x0D, 0xE1,
    0xFC, 0xF3, 0x3B, 0x23, 0x04, 0x03, 0xCF,
    0xFC, 0xF3, 0x3B, 0x23, 0x05, 0x00, 0x01,
    0xFC, 0xF3, 0x3B, 0x23, 0x48, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x49, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0xB7, 0x00, 0x10,
    0xFC, 0xF3, 0x3B, 0x23, 0xBA, 0x60, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x10, 0x12, 0x05,
    0xFC, 0xF3, 0x3B, 0x23, 0x32, 0x00, 0x30,
    0xFC, 0xF3, 0x3B, 0x23, 0x33, 0x00, 0x04,
    0xFC, 0xF3, 0x3B, 0x23, 0xB8, 0x08, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0x01, 0x00, 0x02,
    0xFC, 0xF3, 0x3B, 0x23, 0xF1, 0x3A, 0x11,
    0xFC, 0xF3, 0x3B, 0x23, 0xF2, 0xC5, 0xF8,
    0xFC, 0xF3, 0x3B, 0x23, 0xF3, 0x3A, 0x11,
    0xFC, 0xF3, 0x3B, 0x23, 0xF4, 0x91, 0x5C,
    0xFC, 0xF3, 0x3B, 0x23, 0xF5, 0x61, 0xAF,
    0xFC, 0xF3, 0x3B, 0x23, 0xF6, 0x73, 0x65,
    0xFC, 0xF3, 0x3B, 0x23, 0xF7, 0x8C, 0xFD,
    0xFC, 0xF3, 0x3B, 0x23, 0xF8, 0x73, 0x65,
    0xFC, 0xF3, 0x3B, 0x23, 0xF9, 0x83, 0xA9,
    0xFC, 0xF3, 0x3B, 0x23, 0xFA, 0x7A, 0xA4,
    0xFC, 0xF3, 0x3B, 0x23, 0xFB, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xFC, 0x80, 0x9D,
    0xFC, 0xF3, 0x3B, 0x23, 0xFD, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xFE, 0x81, 0x3D,
    0xFC, 0xF3, 0x3B, 0x23, 0xFF, 0x7F, 0x16,
    0xFC, 0xF3, 0x3B, 0x23, 0xC0, 0x01, 0xA1,
    0xFC, 0xF3, 0x3B, 0x23, 0xC1, 0xFE, 0x72,
    0xFC, 0xF3, 0x3B, 0x23, 0xC2, 0x01, 0xA1,
    0xFC, 0xF3, 0x3B, 0x23, 0xC3, 0x84, 0xC5,
    0xFC, 0xF3, 0x3B, 0x23, 0xC4, 0x76, 0xD3,
    0xFC, 0xF3, 0x3B, 0x23, 0xC5, 0x31, 0xDB,
    0xFC, 0xF3, 0x3B, 0x23, 0xC6, 0xCE, 0x93,
    0xFC, 0xF3, 0x3B, 0x23, 0xC7, 0x31, 0xDB,
    0xFC, 0xF3, 0x3B, 0x23, 0xC8, 0x82, 0x54,
    0xFC, 0xF3, 0x3B, 0x23, 0xC9, 0x7C, 0x48,
    0xFC, 0xF3, 0x3B, 0x23, 0xCA, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xCB, 0x80, 0xC9,
    0xFC, 0xF3, 0x3B, 0x23, 0xCC, 0x7F, 0xFF,
    0xFC, 0xF3, 0x3B, 0x23, 0xCD, 0x81, 0x00,
    0xFC, 0xF3, 0x3B, 0x23, 0xCE, 0x7F, 0x33,
    0xFC, 0xF3, 0x3B, 0x22, 0xF5, 0x80, 0x03,
    0xFC, 0xF3, 0x3B, 0x23, 0xB2, 0x00, 0x03,
    0xFC, 0xF3, 0x3B, 0x3F, 0xD2, 0x06, 0x22,
    0xFC, 0xF3, 0x3B, 0x22, 0xF2, 0x00, 0x34,
    0xFC, 0xF3, 0x3B, 0x22, 0xE9, 0x00, 0x61,
    0xFC, 0xF3, 0x3B, 0x23, 0x2F, 0x00, 0x60,
    0xFC, 0xF3, 0x3B, 0x23, 0x39, 0x00, 0x03,
    0xFC, 0xF3, 0x3B, 0x23, 0xB3, 0x00, 0x08,
    0xFC, 0xF3, 0x3B, 0x23, 0xB5, 0x50, 0x00,
    0xFC, 0xF3, 0x3B, 0x22, 0xFB, 0x00, 0x00                                     
     
};
s32 i2cInit_FM1288_Tx(void)
{
	u8  err,bData;
	u32 x;
 	GPIO_IIC_CFG    stGpio_Iic;
    u16 data_size=0;
    
    gpioSetLevel(1, 18, 1);	//reset  FM1288
    OSTimeDly(1);
    
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_FM1288, sizeof (GPIO_IIC_CFG));
    data_size=sizeof(FM1288_mode0_Tx);

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_FM1288 i2cSemReq is %d.\n", err);
		return 0;
	}

     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    for(x=0;x<data_size;x++)
    {
        bData = FM1288_mode0_Tx[x];
        //i2cWrite_FM1288(x, bData,I2C_FM1288_WR_SLAV_ADDR);
        gpio_IIC_W_Byte(&stGpio_Iic, bData);

         /* Acknowledge */
        if (!gpio_IIC_Ack_R(&stGpio_Iic))
        {
            DEBUG_I2C("%d %x\n",x,bData);
            goto RECOVER_GPIO;
            
        }
    }

     /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
	DEBUG_I2C("Error: i2cWrite_FM1288() gpio IIC Write Error!!!\n");

    return 0;

}

s32 i2cInit_FM1288_Rx(void)
{
	u8  err,bData;
	u32 x;
	GPIO_IIC_CFG    stGpio_Iic;
    u16 data_size=0;
    
    gpioSetLevel(0, 10, 1);	//reset  FM1288
    OSTimeDly(1);
    
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_FM1288, sizeof (GPIO_IIC_CFG));
    data_size=sizeof(FM1288_mode0_Rx);

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_FM1288 i2cSemReq is %d.\n", err);
		return 0;
	}

     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);


    for(x=0;x<data_size;x++)
    {
        bData = FM1288_mode0_Rx[x];
        //i2cWrite_FM1288(x, bData,I2C_FM1288_WR_SLAV_ADDR);
        gpio_IIC_W_Byte(&stGpio_Iic, bData);

         /* Acknowledge */
        if (!gpio_IIC_Ack_R(&stGpio_Iic))
        {
            DEBUG_I2C("%d %x\n",x,bData);
            goto RECOVER_GPIO;
            
        }
    }

     /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;


    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
	DEBUG_I2C("Error: i2cWrite_FM1288() gpio IIC Write Error!!!\n");

    return 0;

}
#endif
#endif
#if GPIO_I2C_ENA
s32 i2cRead_CS8556(u8 ucRegAddr, u8* pucData,u8 DeviceAddr)
{
	u8  err;
	u32 i;
    GPIO_IIC_CFG    stGpio_Iic;
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_CS8556 i2cSemReq is %d.\n", err);
		return 0;
	}

   /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_CS8556_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;



    /* I2C Write register offset */
    gpio_IIC_W_Byte(&stGpio_Iic, 0x00);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;



    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

     /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);



    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_CS8556_RD_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData);
    /* Inverse Acknowledge */
    gpio_IIC_nAck_W(&stGpio_Iic);

    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
    DEBUG_I2C("Error: i2cRead_CS8556() gpio IIC Read Addr [%#x] Error!!!\n", ucRegAddr);

    return 0;


}


s32 i2cWrite_CS8556(u8 ucRegAddr, u8 ucData,u8 DeviceAddr)
{
	u8  err;
	u32 i;
	GPIO_IIC_CFG    stGpio_Iic;

    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_CS8556 i2cSemReq is %d.\n", err);
		return 0;
	}

     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);


    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_CS8556_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;


        /* I2C Write register offset */

        gpio_IIC_W_Byte(&stGpio_Iic, 0x00);
        /* Acknowledge */
        if (!gpio_IIC_Ack_R(&stGpio_Iic))
            goto RECOVER_GPIO;

    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;


    /* I2C Write data */
    gpio_IIC_W_Byte(&stGpio_Iic, ucData);
    /* Acknowledge */
    gpio_IIC_Ack_R(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    }
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
	DEBUG_I2C("Error: i2cWrite_CS8556() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);

    return 0;



}



void i2cInit_CS8556(void)
{

    i2cWrite_CS8556(0x0000,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0001,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0002,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0003,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0004,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0005,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0006,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0007,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0008,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0009,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000a,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000b,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x000f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0010,0x3d,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0011,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0012,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0013,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0014,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0015,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0016,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0017,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0018,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0019,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x001f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0020,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0021,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0022,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0023,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0024,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0025,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0026,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0027,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0028,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0029,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x002f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0030,0xb3,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0031,0x06,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0032,0x7f,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0033,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0034,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0035,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0036,0xa4,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0037,0x06,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0038,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0039,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003b,0x0a,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x003f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0040,0x0c,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0041,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0042,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0043,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0044,0x11,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0045,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0046,0x03,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0047,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0048,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0049,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x004f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0050,0x22,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0051,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0052,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0053,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0054,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0055,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0056,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0057,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0058,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0059,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x005f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0060,0xb3,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0061,0x06,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0062,0x7f,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0063,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0064,0x04,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0065,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0066,0xa4,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0067,0x06,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0068,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0069,0xe4,I2C_CS8556_WR_SLAV_ADDR);   /* Horizontal Scale  */
    i2cWrite_CS8556(0x006a,0x05,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x006b,0x10,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x006c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x006d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x006e,0x0c,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x006f,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0070,0x0c,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0071,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0072,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0073,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0074,0x11,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0075,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0076,0x03,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0077,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0078,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0079,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x007f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0080,0x22,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0081,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0082,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0083,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0084,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0085,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0086,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0087,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0088,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0089,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x008f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0090,0x84,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0091,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0092,0x18,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0093,0x0d,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0094,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0095,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0096,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0097,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0098,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x0099,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009a,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009b,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009c,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009d,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009e,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x009f,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a0,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a1,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a2,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a3,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a4,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a5,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a6,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a7,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a8,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00a9,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00aa,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ab,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ac,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ad,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ae,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00af,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b0,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b1,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b2,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b3,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b4,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b5,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b6,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b7,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b8,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00b9,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ba,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00bb,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00bc,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00bd,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00be,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00bf,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c0,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c1,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c2,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c3,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c4,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c5,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c6,0xff,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c7,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c8,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00c9,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ca,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00cb,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00cc,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00cd,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ce,0x80,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00cf,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d0,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d1,0x44,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d2,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d3,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d4,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d5,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d6,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d7,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d8,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00d9,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00da,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00db,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00dc,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00dd,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00de,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00df,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e0,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e1,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e2,0x08,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e3,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e4,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e5,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e6,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e7,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e8,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00e9,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ea,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00eb,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ec,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ed,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ee,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ef,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f0,0x60,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f1,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f2,0xe0,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f3,0x02,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f4,0x10,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f5,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f6,0xf0,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f7,0x01,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f8,0xf0,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00f9,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00fa,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00fb,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00fc,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00fd,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00fe,0x00,I2C_CS8556_WR_SLAV_ADDR);
    i2cWrite_CS8556(0x00ff,0x00,I2C_CS8556_WR_SLAV_ADDR);

}


#endif







#if(Sensor_OPTION ==  Sensor_PC1089_YUV601)
void i2cInit_PC1089(void)
{
    int i;

  #define num (sizeof(PC_1089_IIC_reg_table) / sizeof(PC_1089_IIC_reg_table[0]))
    //HW reset//
    for(i=0; i<num; i++)
    {
       i2cWrite_SENSOR(PC_1089_IIC_reg_table[i].Addr, PC_1089_IIC_reg_table[i].Value);
    }
  #undef num
}
#endif

#if(TV_DECODER == TI5150)
void i2cInit_TVP5150_1(void)
{
    u8  data;

    DEBUG_I2C("---Init TVP5150_1---\n");
    //----------Reset TI5150-------------//
    i2cWrite_TVP5150(0x00, 0x00,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x00, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x00, data);

    i2cWrite_TVP5150(0x03, 0x0d,I2C_TVP5150_WR_SLAV_ADDR_1);
    //i2cWrite_TVP5150(0x03, 0x0f,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x03, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x0d)\n", 0x03, data);

    i2cWrite_TVP5150(0x04, 0xc0,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x04, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0xc0)\n", 0x04, data);

  #if( (Sensor_OPTION == Sensor_CCIR656) || MULTI_CHANNEL_SUPPORT )
    i2cWrite_TVP5150(0x0d, 0x47,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x0d, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x47)\n", 0x0d, data);
  #else
    i2cWrite_TVP5150(0x0d, 0x40,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x0d, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x40)\n", 0x0d, data);
  #endif

    i2cWrite_TVP5150(0x0f, 0x00,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x0f, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x0f, data);

    i2cWrite_TVP5150(0x28, 0x00,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x28, &data ,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x28, data);

    i2cWrite_TVP5150(0x08, 0x0c,I2C_TVP5150_WR_SLAV_ADDR_1);
    i2cRead_TVP5150(0x08, &data ,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x08, data);


#if(TV_PARAMETER_SEL == TVPARA_CWELL)
    i2cWrite_TVP5150(0x0c, 117,I2C_TVP5150_WR_SLAV_ADDR_1); //Contrast
    i2cWrite_TVP5150(0x09, 100,I2C_TVP5150_WR_SLAV_ADDR_1); //Brightness
#else
    i2cWrite_TVP5150(0x0c, 128,I2C_TVP5150_WR_SLAV_ADDR_1); //Contrast
    i2cRead_TVP5150(0x0c, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x80)\n", 0x0c, data);
    i2cWrite_TVP5150(0x09, 128,I2C_TVP5150_WR_SLAV_ADDR_1); //Brightness
    i2cRead_TVP5150(0x09, &data,I2C_TVP5150_RD_SLAV_ADDR_1);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x80)\n", 0x09, data);
#endif
}

void i2cInit_TVP5150_2(void)
{
    u8  data;

    DEBUG_I2C("---Init TVP5150_2---\n");
    //----------------------//
    i2cWrite_TVP5150(0x00, 0x00,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x00, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x00, data);

    i2cWrite_TVP5150(0x03, 0x0d,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x03, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x0d)\n", 0x03, data);

    i2cWrite_TVP5150(0x04, 0xc0,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x04, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0xc0)\n", 0x04, data);

  #if((Sensor_OPTION == Sensor_CCIR656) || MULTI_CHANNEL_SUPPORT )
    i2cWrite_TVP5150(0x0d, 0x47,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x0d, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x47)\n", 0x0d, data);
  #else
    i2cWrite_TVP5150(0x0d, 0x40,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x0d, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x40)\n", 0x0d, data);
  #endif

    i2cWrite_TVP5150(0x0f, 0x00,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x0f, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x0f, data);

    i2cWrite_TVP5150(0x28, 0x00,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x28, &data ,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x28, data);

    i2cWrite_TVP5150(0x08, 0x0c,I2C_TVP5150_WR_SLAV_ADDR_2);
    i2cRead_TVP5150(0x08, &data ,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x00)\n", 0x08, data);

#if(TV_PARAMETER_SEL == TVPARA_CWELL)
    i2cWrite_TVP5150(0x0c, 117,I2C_TVP5150_WR_SLAV_ADDR_2); //Contrast
    i2cWrite_TVP5150(0x09, 100,I2C_TVP5150_WR_SLAV_ADDR_2); //Brightness
#else
    i2cWrite_TVP5150(0x0c, 128,I2C_TVP5150_WR_SLAV_ADDR_2); //Contrast
    i2cRead_TVP5150(0x0c, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x80)\n", 0x0c, data);
    i2cWrite_TVP5150(0x09, 128,I2C_TVP5150_WR_SLAV_ADDR_2); //Brightness
    i2cRead_TVP5150(0x09, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    DEBUG_I2C("I2C 0x%02x = 0x%02x(0x80)\n", 0x09, data);
#endif
}


#elif(TV_DECODER == CJC5150 )
u8 regValue[][2]=
{
    {0x00,0x00},
    {0x01,0x15},
    {0x02,0x00},
    {0x03,0xaf},
    {0x04,0xc0},
    {0x28,0x00},
    {0x06,0x10},
    {0x0f,0x02},
    {0x0d,0x47},
    {0x11,0x00},
    {0x12,0x00},
    {0x13,0x00},
    {0x14,0x00},
    {0x16,0x80},
    {0x18,0x00},
    {0x19,0x00},
    {0x09,0x80},
    {0x0a,0x80},
    {0x0b,0x00},
    {0x0c,0x80},
};

void i2cInit_CJC5150(void)
{
    u8 i;
    u8 reg;
    u8 val;
    u8 num;


    DEBUG_I2C("!@#@! %d \n",sizeof(regValue));
    num=sizeof(regValue)/2;
    for(i=0;i<num;i++)
    {
        reg=regValue[i][0];
        val=regValue[i][1];
        i2cWrite_Byte(I2C_TVP5150_WR_SLAV_ADDR_2,reg,val);
    }


}


#elif(TV_DECODER == BIT1605)
void i2cInit_BIT1605(void)
{
    u8  data;
    u32 i;
    u32 bitSet;


    DEBUG_I2C(" i2cInit_BIT1605 \n\n\n");
    //Reset BIT1605
    gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  TV decoder
    for(i=0; i<500 ;i++);
    gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  TV decoder
    for(i=0; i<500 ;i++);


    i2cWrite_BIT1605(0x0a, 0xf1);
    i2cRead_BIT1605 (0x0a, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x0a, data);
#if (HW_BOARD_OPTION==ITS_CARDVR)
    i2cWrite_BIT1605(0x6d, 0x01); //output V-sync, FID
    i2cRead_BIT1605 (0x6d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6d, data);

    i2cWrite_BIT1605(0x6e, 0x40); //output V-sync, FID
    i2cRead_BIT1605 (0x6e, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6e, data);

    i2cWrite_BIT1605(0x6f, 0x6e); //set clock polarity invert
    i2cRead_BIT1605 (0x6f, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6f, data);
#else

    i2cWrite_BIT1605(0x6d, 0x14); //output V-sync, FID
    i2cRead_BIT1605 (0x6d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6d, data);

    i2cWrite_BIT1605(0x6f, 0x6f); //set clock polarity invert
    i2cRead_BIT1605 (0x6f, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6f, data);
#endif
    i2cWrite_BIT1605(0x22, 0xf1); //Horizontal delay
    i2cRead_BIT1605 (0x22, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x22, data);



    i2cRead_BIT1605 (0x5d, &data);
    data = data & 0xe3;
    data = data | 0x10;
    i2cWrite_BIT1605(0x5d, data); //blank screen function enable
    i2cRead_BIT1605 (0x5d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x5d, data);

    i2cRead_BIT1605 (0x0d, &data);    //standard setting
    data = data & 0xf0;
    data = data | 0x05;
    i2cWrite_BIT1605(0x0d, data);
    i2cRead_BIT1605 (0x0d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x0d, data);

    i2cRead_BIT1605 (0x5e, &data);
    i2cWrite_BIT1605(0x5e, data&0x80); //blank screen function enable
    i2cRead_BIT1605 (0x5e, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x5E, data);

    i2cRead_BIT1605 (0x27, &data);
    i2cWrite_BIT1605(0x27, data|0x80); //VCR Mode
    i2cRead_BIT1605 (0x27, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x27, data);

	i2cWrite_BIT1605(0x65, 0xc4); //edison add for test DV1
    i2cRead_BIT1605 (0x65, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x65, data);

	i2cWrite_BIT1605(0x08, 0x00); //edison add for test DV1
    i2cRead_BIT1605 (0x08, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x08, data);
}

void i2cInit_BIT1605_DV2(void)
{
    u8  data;
    u32 i;
    u32 bitSet;


    DEBUG_I2C(" i2cInit_BIT1605 \n\n\n");
    //Reset BIT1605
    gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV2_VIDEO_RESET, 0);	//reset  TV decoder
    for(i=0; i<500 ;i++);
    gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV2_VIDEO_RESET, 1);	//reset  TV decoder
    for(i=0; i<500 ;i++);


    i2cWrite_BIT1605_DV2(0x0a, 0xf1);
    i2cRead_BIT1605_DV2 (0x0a, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x0a, data);
#if (HW_BOARD_OPTION==ITS_CARDVR)
    i2cWrite_BIT1605_DV2(0x6d, 0x01); //output V-sync, FID
    i2cRead_BIT1605_DV2 (0x6d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6d, data);

    i2cWrite_BIT1605_DV2(0x6e, 0x40); //output V-sync, FID
    i2cRead_BIT1605_DV2 (0x6e, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6e, data);

    i2cWrite_BIT1605_DV2(0x6f, 0x6e); //set clock polarity invert
    i2cRead_BIT1605_DV2 (0x6f, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6f, data);
#else

    i2cWrite_BIT1605_DV2(0x6d, 0x14); //output V-sync, FID
    i2cRead_BIT1605_DV2 (0x6d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6d, data);

    i2cWrite_BIT1605_DV2(0x6f, 0x6f); //set clock polarity invert
    i2cRead_BIT1605_DV2 (0x6f, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x6f, data);
#endif
    i2cWrite_BIT1605_DV2(0x22, 0xf1); //Horizontal delay
    i2cRead_BIT1605_DV2 (0x22, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x22, data);



    i2cRead_BIT1605_DV2 (0x5d, &data);
    data = data & 0xe3;
    data = data | 0x10;
    i2cWrite_BIT1605_DV2(0x5d, data); //blank screen function enable
    i2cRead_BIT1605_DV2 (0x5d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x5d, data);

    i2cRead_BIT1605_DV2 (0x0d, &data);    //standard setting
    data = data & 0xf0;
    data = data | 0x05;
    i2cWrite_BIT1605_DV2(0x0d, data);
    i2cRead_BIT1605_DV2 (0x0d, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x0d, data);

    i2cRead_BIT1605_DV2 (0x5e, &data);
    i2cWrite_BIT1605_DV2(0x5e, data&0x80); //blank screen function enable
    i2cRead_BIT1605_DV2 (0x5e, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x5E, data);

    i2cRead_BIT1605_DV2 (0x27, &data);
    i2cWrite_BIT1605_DV2(0x27, data|0x80); //VCR Mode
    i2cRead_BIT1605_DV2 (0x27, &data);
    DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x27, data);
}

#elif(TV_DECODER == MI9V136)
void i2cInit_MI9V136(void)
{
    u16 data;
    int i;

    //HW reset//
    gpioSetLevel(0, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(0, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(0, GPIO_RST_SENSOR, 1 );

    data= HW_I2C_Word_addr_Read_Word(I2C_MI9V136_RD_SLAV_ADDR,0x3000);
    DEBUG_SIU("MI9V136 ID= 0x%x\n",data);

    for(i=0;i<1507 ;i++)
      HW_I2C_Word_addr_Write_Word(I2C_MI9V136_WR_SLAV_ADDR,i2cpara_MI9V136[i].addr, i2cpara_MI9V136[i].data);
}
#elif(TV_DECODER == WT8861)
u8 VIDEO_DEFAULT[] =
{
    //20101105 0xDB, 0x00, //0xA4,
	0xDB, 0x64,
	0x00, 0x00,
	0x01, 0x09, //0x01,
	0x02, 0X3f,
	0x03, 0x10,
	0x04, 0xDD,
	0x05, 0x32,
	0x06, 0x0A,
	0x07, 0xA0, //0xA2,
	0x08, 0x70,
	0x09, 0x20,
	0x0A, 0x80,
	0x0B, 0x00,
	0x0C, 0x8A,
	0x0D, 0x07,
	0x0E, 0x06,
	0x0F, 0x2C,
	0x10, 0x0A,
	0x11, 0x89,
	0x12, 0x04,
	0x13, 0xA6,
	0x14, 0x2E,
	0x15, 0x47,
	0x16, 0x74,
	0x17, 0xCB,
	0x18, 0x2D,
	0x19, 0x2E, //0xD1,
	0x1A, 0x74,
	0x1B, 0x5D,
	0x1C, 0x2B,
	0x1D, 0x33,
	0x1E, 0x33,
	0x1F, 0x33,
	0x20, 0x3E,
	0x21, 0x3E,
	0x22, 0x00,
	0x23, 0x80,
	0x24, 0xE9,
	0x25, 0x0F,
	0x26, 0x2D,
	0x27, 0x50,
	0x28, 0x22,
	0x29, 0x4E,
	0x2A, 0xD6,
	0x2B, 0x4E,
	0x2C, 0x32,
	0x2D, 0x46,
	0x2E, 0x82, //82
	0x2F, 0x53, //0X50 //20101103
	0x30, 0x22,
	0x31, 0x61, 	//0x61
	0x32, 0x70,
	0x33, 0X01F,	//0x0E  //For VCR mode ,
	0x34, 0x6C,
	0x35, 0x90,
	0x36, 0x7A,
	0x37, 0x20,
	0x38, 0x00,
	0x39, 0x0A,
	0x3D, 0x00,
	0x3E, 0x00,
	0x3F, 0x01,
	0x40, 0x05, //0x04, //20051003 video power on NG
	0x41, 0xE7,
	0x42, 0x5A,
	0x43, 0x00,
	0x44, 0x00,
	0x45, 0x00,
	0x46, 0x00,
	0x47, 0x00,
	0x48, 0x00,
	0x49, 0x00,
	0x4A, 0x00,
	0x4B, 0x00,
	0x4C, 0x00,
	0x4D, 0x00,
	0x4E, 0x00,
	0x4F, 0x00,
	0x50, 0x00,
	0x51, 0x11,	//20051228 // 0x11,
	0x52, 0x00,
	0x53, 0x00,
	0x54, 0x00,
	0x55, 0x44,
	0x56, 0x04,
	0x58, 0x1A,
	0x59, 0xCF,
	0x5A, 0x0C,
	0x5B, 0xE5,
	0x5C, 0x78,
	0x5D, 0xCE,
	0x5E, 0xBE,
	0x5F, 0xB4,
	0x60, 0x7C,
	0x66, 0x64,
	0x67, 0x64,
	0x68, 0x5A,
    0x80, 0x23, //0x04,
	0x82, 0x82,
	0x83, 0x67,
	0x8A, 0x0A,
	0x8B, 0x01,
	0x8D, 0x0A,
	0x8E, 0xC8,
	0x8F, 0xB9,
	0xAF, 0x0A,
	0xB0, 0xFA,
	0xB1, 0xFF,
	0xB2, 0x50,
	0xB3, 0x03,
	0xB4, 0x21,
	0xB5, 0xC3,
	0xB6, 0x02,
	0xB7, 0x00,
	0xB8, 0x00,
	0xBA, 0x21,
	0xBB, 0x61,
	0xBC, 0x0D,
	//20101105 0xBD, 0x00,
	0xBD, 0x01,
	0xC0, 0x00,
	0xC1, 0x03, //0x29, //0x00, For S-Video
	0xC2, 0x00,
	0xC3, 0x04,
	0xC4, 0x00,
	0xC5, 0x00,	//0x05
	0xC6, 0x00,
	0xC7, 0x00,
	0xC8, 0x00,
	0xC9, 0xC8, //0xC9,
//========auto sog detect=====93 8 4===
	0xCA, 0x14,
	0xCB, 0xA6,
	0xCC, 0x07, //0x10,
	0xCD, 0x1F, //0x5F,
	0xCE, 0x1F,
	0xCF, 0x1F,
	0xD0, 0x20, //0x1D
	0xD1, 0x1F, //0x1D,
	0xD2, 0xFF,
//===================================
	0xD3, 0x00,
	0xD4, 0x00,
	0xD5, 0x84,
	0xD6, 0x00,
	0xD7, 0x00,	//0x03
	0xD8, 0x00,
	0xD9, 0xB4,
	0xDA, 0x02,
	0xDC, 0x00, //0x9F,
	0xDE, 0x81, //0x00
    0xEC, 0x01,	//20051108 For VCR mode
	0xFF, 0xFF,
};
u8 NTSCA_24MHz[] =		//XTL = 24
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	 0xCB, 0x10,   //AD CLAMP PLACEMENT
//	 0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x00,   //525 LINE
	0x04, 0xDD,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x26,   //CDTO            //tint bug start        //060413
	0x19, 0x2d,   //CDTO    ???
	0x1A, 0x8B,   //CDTO
	0x1B, 0xA2,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO             //tint bug end
	0x2E, 0x82,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x22,   //ACTIVE LINE START IN FIELD
	0x31, 0x61,   //ACTIVE LINE TOTAL
	0x39, 0x0A,
	0x03, 0x10,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0xc2,//0xA2,	//YC DELAY
//-----mars MR9670_WI2-----
	//0x09, 0x20,
	0x0A, 0xb0,
	//0x08, 0x70,
	0x0b, 0x00,
	0x01, 0x11,
	0x80, 0x7F,
	0xC3, 0x3F,
//-----mars MR9670_WI2-----
//------------------------------------
	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
 };

u8 PALIA_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x32,  	//525 LINE                     -
	0x04, 0xDC,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x2F,   //CDTO
	0x19, 0x4a,   //CDTO  ???
	0x1A, 0xBC,   //CDTO
	0x1B, 0x24,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x84,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x2A,   //ACTIVE LINE START IN FIELD
	0x31, 0xC1,   //ACTIVE LINE TOTAL
	0x39, 0x8A,
 	0x03, 0x16,//080328,BIT0 SET "H" to avoid pal bug//0x12,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0xc2, //0xA2,	//YC DELAY
//-----mars MR9670_WI2-----
//NTSC
	//0x09, 0x20,
	0x0A, 0x50,
	//0x08, 0x70,
	0x0b, 0x00,
	0x01, 0x09,
	0x80, 0x15,
	0xC3, 0x04,
//PAL
	0x80, 0x15,
	0x01, 0x00,
	0xCD, 0x1F,
	0xD0, 0x21,
//-----mars MR9670_WI2-----
//------------------------------------
	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
 };

u8 PALMA_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x04,   //525 LINE
	0x04, 0xDD,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x26,   //CDTO
	0x19, 0x23,   //CDTO    ???
	0x1A, 0xCD,   //CDTO
	0x1B, 0x98,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x82,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x22,   //ACTIVE LINE START IN FIELD
	0x31, 0x61,   //ACTIVE LINE TOTAL

	0x39, 0x8A,
	0x03, 0x16,//080328,BIT0 SET "H" to avoid pal bug//0x12,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0xc2, //0xA2,	//YC DELAY
//-----mars MR9670_WI2-----
//NTSC
	//0x09, 0x20,
	0x0A, 0x50,
	//0x08, 0x70,
	0x0b, 0x00,
	0x01, 0x09,
	0x80, 0x15,
	0xC3, 0x04,
//PAL
	0x80, 0x15,
	0x01, 0x00,
	0xCD, 0x1F,
	0xD0, 0x21,
//-----mars MR9670_WI2-----

//------------------------------------
	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
};

u8 PALCNA_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x36,   //525 LINE
	0x04, 0xDC,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x26,   //CDTO
	0x19, 0x35,   //CDTO    ???
	0x1A, 0x66,   //CDTO
	0x1B, 0xCF,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x84,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x2A,   //ACTIVE LINE START IN FIELD
	0x31, 0xC1,   //ACTIVE LINE TOTAL
	0x39, 0x8A,
	0x03, 0x16,//080328,BIT0 SET "H" to avoid pal bug//0x12,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0xc2, //0xA2,	//YC DELAY
//------------------------------------	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
};

u8 NTSC443_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x00,  	//525 LINE                     -
	0x04, 0xDD,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x2F,   //CDTO
	0x19, 0x4a,   //CDTO  ???
	0x1A, 0xBC,   //CDTO
	0x1B, 0x24,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x82,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x22,   //ACTIVE LINE START IN FIELD
	0x31, 0x61,   //ACTIVE LINE TOTAL
	0x39, 0x8A,
 	0x03, 0x92,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0x8A, //0xAA,	//YC DELAY
//------------------------------------
	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
};

u8 PAL60A_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	  0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x02,  	//525 LINE                     -
	0x04, 0xDC,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x2F,   //CDTO
	0x19, 0x4a,   //CDTO  ???
	0x1A, 0xBC,   //CDTO
	0x1B, 0x24,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x82,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x22,   //ACTIVE LINE START IN FIELD
	0x31, 0x61,   //ACTIVE LINE TOTAL
	0x39, 0x8A,
	0x03, 0x13,//080328,BIT0 SET "H" to avoid pal bug//0x12,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0xc2, //0xA2,	//YC DELAY
//------------------------------------
	0x2C, 0x32,   //SECAM only
	0x2d, 0x46,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
};

u8 SECAMA_24MHz[] =
{
//20050505	0xCB, 0x00,
	0x0B, 0x00,   //for hue set max no color bug, 070614, teresa
	0xDB, 0xA4,
//	  0xCB, 0x10,   //AD CLAMP PLACEMENT
//	  0xCC, 0x10,   //AD CLAMP DURATION
	0x35, 0xD0,   //V SYNC AGC VCR NOISE ENABLE
//------------------------------------
//	  0x00, 0x38,   //525 LINE
	0x04, 0xDC,   //AGC VALUE
	0x12, 0x05,   //AGC GATE START
	0x13, 0xC8,   //AGC GATE START
	0x14, 0x38,   //AGC GATE WIDTH
	0x15, 0x58,   //AGC TIME DELAY
	0x18, 0x2D,   //CDTO
	0x19, 0xB7,   //CDTO
	0x1A, 0xA3,   //CDTO
	0x1B, 0x28,   //CDTO
	0x1C, 0x24,   //HDTO
	0x1D, 0x00,   //HDTO
	0x1E, 0x00,   //HDTO
	0x1F, 0x00,   //HDTO
	0x2E, 0x84,   //ACTIVE PIXEL START IN H LINE
	0x30, 0x2A,   //ACTIVE LINE START IN FIELD
	0x31, 0xC1,   //ACTIVE LINE TOTAL
	0x39, 0x8A,
	0x03, 0x00,   //YC FILTER TYPE
//20050420	0xB2, 0x10,   // 2D COME ONLY?
	0xD8, 0x75,
	0x07, 0x87, //0xA7,	//YC DELAY
//------------------------------------
	0x2C, 0x40,   //SECAM only
	0x2d, 0x66,   //SECAM only
//------------------------------------
	0x3F, 0x01,   //SOFT RESET
	0xFF, 0xFF    //STOP
};
void i2cInit_WT8861_1(void)
{
    u8  bAddress,bData;
    u32 i,x;
#if IS_COMMAX_DOORPHONE
	CamPower_On();
#endif
    DEBUG_I2C("---Init WT8861_1---\n");
    //----------Reset WT8861-------------//
#if IS_COMMAX_DOORPHONE
	VideoDecoder_reset();
#else
	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
#endif

	x=0;
    while (VIDEO_DEFAULT[x] != 0xFF)
	{
		bAddress = VIDEO_DEFAULT[x++];
		bData = VIDEO_DEFAULT[x++];
		i2cWrite_WT8861(bAddress, bData,I2C_WT8861_WR_SLAV_ADDR);
    }
    OSTimeDly(5);
	i2cWrite_WT8861(0x3F,0x00,I2C_WT8861_WR_SLAV_ADDR);
	NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);

}

void i2cInit_WT8861_2(void)
{
    u8  bAddress,bData;
    u32 i,x;

    DEBUG_I2C("---Init WT8861_2---\n");
    //----------Reset WT8861-------------//
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);

    x=0;
    while (VIDEO_DEFAULT[x] != 0xFF)
	{
		bAddress = VIDEO_DEFAULT[x++];
		bData = VIDEO_DEFAULT[x++];
		i2cWrite_WT8861(bAddress, bData,I2C_WT8861_WR_SLAV_2_ADDR);
	}
	OSTimeDly(5);
	i2cWrite_WT8861(0x3F,0x00,I2C_WT8861_WR_SLAV_2_ADDR);
	NTSCA(I2C_WT8861_RD_SLAV_2_ADDR,I2C_WT8861_WR_SLAV_2_ADDR);

}
void VIDEO_PortChange(u8 port)
{
u8 data;
	switch(port)
	{
		#ifdef TUNER
			case INPUT_SOURCE_TUNER:
				i2cWrite_WT8861(0xC0,0x0C,I2C_WT8861_WR_SLAV_ADDR);
				i2cRead_WT8861(0x00,&data,I2C_WT8861_RD_SLAV_ADDR);
				data = data&0xFE;
				i2cWrite_WT8861(0x00,data,I2C_WT8861_WR_SLAV_ADDR);
				break;
		#endif
		case INPUT_SOURCE_AV1:
			i2cWrite_WT8861(0xC0,0x00,I2C_WT8861_WR_SLAV_ADDR);
			i2cRead_WT8861(0x00,&data,I2C_WT8861_RD_SLAV_ADDR);
			data = data&0xFE;
			i2cWrite_WT8861(0x00,data,I2C_WT8861_WR_SLAV_ADDR);
			break;
	#if 0
		#ifdef VIDEO_AV2
			case INPUT_SOURCE_AV2:
			i2cWrite_WT8861(0xC0,0x19,I2C_WT8861_WR_SLAV_ADDR);
			i2cRead_WT8861(0x00,&data,I2C_WT8861_RD_SLAV_ADDR);
			data = data | 0x01;
			i2cWrite_WT8861(0x00,data,I2C_WT8861_WR_SLAV_ADDR);
		//	I2CByteWrite(ID_WT8851, 0xC0, 0x19); //AV2_INPUT_MULTIPLEXER_C0);
		//	I2CByteWrite(ID_WT8851, 0x00,(I2CByteRead(ID_WT8851, 0x00)|0x01));
			break;
		#endif
		#ifdef VIDEO_AV3
			case INPUT_SOURCE_AV3:
			i2cWrite_WT8861(0xC0,AV3_INPUT_MULTIPLEXER_C0,I2C_WT8861_WR_SLAV_ADDR);
			i2cRead_WT8861(0x00,&data,I2C_WT8861_RD_SLAV_ADDR);
			data = data & 0xFE;
			i2cWrite_WT8861(0x00,data,I2C_WT8861_WR_SLAV_ADDR);
		//	I2CByteWrite(ID_WT8851, 0xC0, AV3_INPUT_MULTIPLEXER_C0);
		//	I2CByteWrite(ID_WT8851, 0x00,(I2CByteRead(ID_WT8851, 0x00)&0xFE));
			break;
		#endif
		#ifdef VIDEO_YCbCr
			case INPUT_SOURCE_YPBPR:
			i2cWrite_WT8861(0x00,HDTV_INPUT_MULTIPLEXER_C0,I2C_WT8861_WR_SLAV_ADDR);
		//	I2CByteWrite(ID_WT8851, 0xC0, HDTV_INPUT_MULTIPLEXER_C0);
			break;
		#endif
	#endif
	}
}

	#define	CordicFreq_1	0xEA
	#define	CordicFreq_2	0xD0
	#define	CordicFreq_3	0x40
	#define	CordicFreq_4	0x15
void VIDEO_SetupColourStandard(void)
{
	u8 bCordicFreq, bStatusReg3, bStatusReg1, bChromaGain;
	u8 bfFcMore,bfFcLess,bfFcSame,bfLine625,bfChromaLock,bfPal,bfSecam;
	#ifdef TUNER
		uCHAR bHVNLOCK,bData;
	#endif

	i2cRead_WT8861(0x7D, &bCordicFreq, I2C_WT8861_RD_SLAV_ADDR);	//Cordic frequency status
	i2cRead_WT8861(0x3C, &bStatusReg3, I2C_WT8861_RD_SLAV_ADDR);	//Check line 625/525, pal, secam
	i2cRead_WT8861(0x3A, &bStatusReg1, I2C_WT8861_RD_SLAV_ADDR);	//Check chroma Lock
	i2cRead_WT8861(0x7B, &bChromaGain, I2C_WT8861_RD_SLAV_ADDR);	//Check chroma gain
	bfFcMore = bfFcLess = bfFcSame = 0;
	if ((abs(bCordicFreq)>CordicFreq_2) && (abs(bCordicFreq)<CordicFreq_1))
		bfFcMore = 1;
	else if ((abs(bCordicFreq)>CordicFreq_4) && (abs(bCordicFreq)<CordicFreq_3))
		bfFcLess = 1;
	else if ((abs(bCordicFreq)<=CordicFreq_4) || (abs(bCordicFreq)>=CordicFreq_1))
		bfFcSame = 1;

	if(bStatusReg3 & 0x04)
	{
		bfLine625 = 1;		//PAL
		bVideoField = VIDEO_625L50HZ;
	}
	else
	{
		bfLine625 = 0;		//NTSC
		bVideoField = VIDEO_525L60HZ;
	}
	if(bStatusReg1 & 0x08)
		bfChromaLock = 1;
	else
		bfChromaLock = 0;

	if(bStatusReg3 & 0x01)
		bfPal = 1;
	else
		bfPal = 0;

	if(bStatusReg3 & 0x02)
		bfSecam = 1;
	else
		bfSecam = 0;
//20041203 if(bChromaGain>0x30)		//Color killer too large 0x30, Is black-white sync ?
	if(bChromaGain >= 0x06)	//WT8851setting to 0x20,WT8861 setting to 0x06 	//Color killer too large 0x20, Is black-white sync ?
	{
		if(bfLine625 && (bVideoColourSelect != PALI_COLOUR))
        {
			//if(bVideoColourSelect != PALI_COLOUR)
			PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
        }
		else if(!bfLine625 && (bVideoColourSelect != NTSC_COLOUR))
		{
			NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
		}
	}
	else
	{
		switch(bVideoColourSelect)
		{
			case NTSC_COLOUR:
					//if(bfLine625 && bfFcMore)	//20041202
				if(bfLine625 && (bfFcMore || bfFcSame))
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfChromaLock)
					PALMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcLess)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcMore)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(!bfLine625 && bfFcSame && !bfChromaLock)
				{
					NTSC443A();
				}
				*/
		//		DEBUG_I2C("NTSC_COLOUR \n");
			break;
			case PALI_COLOUR:
				if(!bfLine625 && bfFcLess)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfChromaLock)
					PALMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcLess)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame && !bfChromaLock) //(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && bfPal)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfPal)
					NTSC443A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
		//		DEBUG_I2C("PALI_COLOUR \n");
			break;
			case PALM_COLOUR:
				if(!bfLine625 && bfFcSame && !bfPal)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcMore)
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcMore && !bfChromaLock && !bfPal) //(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcMore)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(!bfLine625 && bfFcSame && !bfPal)
				{
					NTSC443A();
				}
				*/
		//		DEBUG_I2C("PALI_COLOUR \n");
			break;
			case PALCN_COLOUR:
				if(!bfLine625 && bfFcSame)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcMore)
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(bfLine625 && bfFcSame)
				{
					PALMA();
				}
				*/
				else	 if(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(!bfLine625 && bfFcMore)
				{
					PAL60A();
				}
				*/
				else	 if(!bfLine625 && bfFcMore)
					NTSC443A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
	//			DEBUG_I2C("PALCN_COLOUR \n");
			break;
			case SECAM_COLOUR:
				if(!bfLine625 && bfFcLess && !bfPal)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame &&!bfChromaLock && !bfSecam)
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(bfLine625 && bfFcSame)
				{
					PALMA();
				}
				*/
				else	 if(bfLine625 && bfFcLess && !bfSecam)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfSecam)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(!bfLine625 && bfFcMore)
				{
					NTSC443A();
				}
				*/
		//		DEBUG_I2C("SECAM_COLOUR \n");
			break;
			case PAL60_COLOUR:
				if(!bfLine625 && bfFcLess)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame)
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(bfLine625 && bfFcLess)
				{
					PALMA();
				}
				*/
				else	 if(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcMore)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfPal)
					NTSC443A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
		//		DEBUG_I2C("PAL60_COLOUR \n");
			break;
			case NTSC443_COLOUR:
				if(!bfLine625 && bfFcLess)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame)
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				/*
				else	 if(bfLine625 && bfFcLess)
				{
					PALMA();
				}
				*/
				else	 if(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcMore && bfPal)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcLess)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
		//		DEBUG_I2C("NTSC443_COLOUR \n");
			break;
			case NONE_COLOUR:
				if(!bfLine625 && bfFcLess)
					NTSCA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else if(bfLine625 && (bfFcMore || bfFcSame))
					PALIA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfChromaLock)
					PALMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcLess)
					PALCNA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(bfLine625 && bfFcSame && !bfChromaLock) //(bfSecam)
					SECAMA(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && bfPal)
					PAL60A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);
				else	 if(!bfLine625 && bfFcSame && !bfPal)
					NTSC443A(I2C_WT8861_RD_SLAV_ADDR,I2C_WT8861_WR_SLAV_ADDR);

			break;

		 }

	}
#if IS_COMMAX_DOORPHONE
    Menu_setColor(MENU_Color);
#endif
}

void NTSCA(u32 Readaddr,u32 Writeaddr)
{
	u16 bIndex;
	u8 bAddress,bData;
	if(cam_check==1)
	{
		if(sysTVinFormat != TV_IN_NTSC)
			return;
	}
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);
	bVideoColourSelect = NTSC_COLOUR;
	fChangeMode = ON;
	bVideoResetTimer = VIDEO_RESET_TIMER;

	DEBUG_I2C("NTSCA \n");
	bIndex=0;
	while (NTSCA_24MHz[bIndex] != 0xFF)
	{
		bAddress = NTSCA_24MHz[bIndex++];
		bData = NTSCA_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);

	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
//	OSTimeDly(1);//PARA_DEL_XUS(2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);
}
void PALIA(u32 Readaddr,u32 Writeaddr)

 {
	u16 bIndex;
	u8 bAddress,bData;
	if(cam_check==1)
	{
		if(sysTVinFormat != TV_IN_PAL)
			return;
	}
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);

	bVideoColourSelect = PALI_COLOUR;

	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("PALI_COLOUR \n");
		bIndex=0;
	while (PALIA_24MHz[bIndex] != 0xFF)
	{
		bAddress = PALIA_24MHz[bIndex++];
		bData = PALIA_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x32);
	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);
	i2cWrite_WT8861(0x01,0x09,Writeaddr); //test_626

}
void PALMA(u32 Readaddr,u32 Writeaddr)
{
	u16 bIndex;
	u8 bAddress,bData;
return ;
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);
	bVideoColourSelect = PALM_COLOUR;
	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("PALM_COLOUR \n");
	bIndex=0;
	while (PALMA_24MHz[bIndex] != 0xFF)
	{
		bAddress = PALMA_24MHz[bIndex++];
		bData = PALMA_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}

//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x04);

	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
 //   OSTimeDly(1);//PARA_DEL_XUS(2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);
	i2cWrite_WT8861(0x01,0x09,Writeaddr); //test_626
}
void PALCNA(u32 Readaddr,u32 Writeaddr)
 {
	u16 bIndex;
	u8 bAddress,bData;
return ;
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);

	bVideoColourSelect = PALCN_COLOUR;

	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("PALCN_COLOUR \n");
	bIndex=0;
	while (PALCNA_24MHz[bIndex] != 0xFF)
	{
		bAddress = PALCNA_24MHz[bIndex++];
		bData = PALCNA_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x36);
	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);

}
void NTSC443A(u32 Readaddr,u32 Writeaddr)
 {
	u16 bIndex;
	u8 bAddress,bData;
return ;
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);

	bVideoColourSelect = NTSC443_COLOUR;

	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("NTSC443_COLOUR \n");
	bIndex=0;
	while (NTSC443_24MHz[bIndex] != 0xFF)
	{
		bAddress = NTSC443_24MHz[bIndex++];
		bData = NTSC443_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x00);
	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);

}


void PAL60A(u32 Readaddr,u32 Writeaddr)
 {
	u16 bIndex;
	u8 bAddress,bData;
return ;
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);

	bVideoColourSelect = PAL60_COLOUR;

	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("PAL60_COLOUR \n");
	bIndex=0;
	while (PAL60A_24MHz[bIndex] != 0xFF)
	{
		bAddress = PAL60A_24MHz[bIndex++];
		bData = PAL60A_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x02);
	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);

}
void SECAMA(u32 Readaddr,u32 Writeaddr)
 {
	u16 bIndex;
	u8 bAddress,bData;
return ;
	//DISPLAY_MUTE(MUTE_ENABLE);
	VIDEO_OutputTriState(ON,Readaddr,Writeaddr);

	bVideoColourSelect = SECAM_COLOUR;

	fChangeMode = ON;
	//bSyncChangeTimer = SOURCECHANGETIME;	//20050627
	bVideoResetTimer = VIDEO_RESET_TIMER;
	DEBUG_I2C("SECAM_COLOUR \n");
	bIndex=0;
	while (SECAMA_24MHz[bIndex] != 0xFF)
	{
		bAddress = SECAMA_24MHz[bIndex++];
		bData = SECAMA_24MHz[bIndex++];
/*		if(bAddress == 0x07)
		{
			I2CByteWrite(ID_WT8851, bAddress, I2CByteRead(ID_WT8851, bAddress)|bData);
		}
		else
*/		i2cWrite_WT8861(bAddress, bData,Writeaddr);
		//I2CByteWrite(ID_WT8851, bAddress, bData);
	}
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Readaddr);//IICDevice_readByte(port,device,addr);
	bData=(bData&0x01);
	bData=(bData|0x38);
	i2cWrite_WT8861(0x00,bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
//============COLOR MODE AND S/CVBS SETTING=====================
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cRead_WT8861(0x00,&bData,Writeaddr);//IICDevice_writeByte(port,device,addr,data2);
	i2cWrite_WT8861(0x3F,0x00,Writeaddr);

}
void VIDEO_OutputTriState(u8 bMode,u32 Readaddr,u32 Writeaddr)
{
	u8 bVal;
    i2cRead_WT8861(0xDB,&bVal,Readaddr);
	if(OFF == bMode)
	{
		bVal &= 0xFD;
	}
	else
	{
		bVal |= 0x02;
	}
	i2cWrite_WT8861(0xDB,bVal,Writeaddr);
}

#elif(TV_DECODER == DM5900)
u8 DM5900_init_value[] =
{
    0x51, 0x01,
    0x32, 0x00,
    0x3b, 0x00,
    0x3c, 0x05,
    0x08, 0x30,
    0x14, 0x16,  //#always tracking clkoffset
    0x17, 0x0a,
    0x05, 0x20,   
    0x18, 0x23,  //#Reg 0x18 bit 0 auto update HSYNCTH
    0x39, 0xd2,  //#increase color AGC threshold
    0x3d, 0x40,  //#CCIR buffer reset  
    0x3a, 0x90,  //#CAGC enable
    0x1A, 0x15,
    0x00, 0x8a

};

void i2cInit_DM5900_1(void)
{
    u8  bAddress,bData;
    u32 x,bIndex;

    DEBUG_I2C("---Init DM5900_1---\n");

    //----------Reset DM5900-------------//
/*	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
*/
    OSTimeDly(2);
	x=0;
    while (DM5900_init_value[x] != 0xFF)
	{
		bAddress = DM5900_init_value[x++];
		bData = DM5900_init_value[x++];
		i2cWrite_DM5900(bAddress, bData,I2C_DM5900_WR_SLAV_ADDR);
    }

}

void i2cInit_DM5900_2(void)
{
    u8  bAddress,bData;
    u32 x,bIndex;

    DEBUG_I2C("---Init DM5900_2---\n");

    //----------Reset DM5900-------------//
    /*
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
	*/

    x=0;
    while (DM5900_init_value[x] != 0xFF)
	{
		bAddress = DM5900_init_value[x++];
		bData = DM5900_init_value[x++];
		i2cWrite_DM5900(bAddress, bData,I2C_DM5900_WR_SLAV_2_ADDR);
	}
   
}



#elif(TV_DECODER == TW9900)
u8 TW9900_init_value[] =
{
    0x03, 0xa2,
    0x05, 0x01,
    0x08, 0x13,
    0x0c, 0xdc,
    0x10, 0x00,
    0x11, 0x60,
    0x12, 0x07,
   	0x13, 0x80,
	0x14, 0x65,
    0x17, 0x00,
    0x18, 0x00,
    0x19, 0x57,
    0x1a, 0x0f,
    0x29, 0x03,
    0x2a, 0x78,
    0x2d, 0x07,
    0x2c, 0x35,
    0x31, 0x10,
    0x33, 0x35,
    0x6b, 0x06,
    0x6c, 0x24,
    0x6d, 0x0a,
    0xFF, 0xFF
};
u8 TW9900A_CCIR_PAL_DataSet[] =
{
	0x03, 0xA2,
	0x05, 0x00,
	0x07, 0x12,
	0x08, 0x19,
	0x09, 0x20,
	0x0a, 0x09,
	0x10, 0x00,
    0x11, 0x60,
    0x12, 0x07,
   	0x13, 0x80,
	0x14, 0x65,
	0x1a, 0x0f,
	0x1c, 0x17,
	0x1d, 0x7e,
	0x17, 0x02,
	0x18, 0x00,
	0x19, 0x4e,
	0x2c, 0x03,
	0x2e, 0xee,
	0x33, 0x20,
	0x06, 0x80,
	0x6E, 0x70,
	0xFF, 0xFF
};
u8 TW9900A_CCIR_NTSC_DataSet[] =
{
	0x03, 0xA2,
	0x05, 0x00,
	0x08, 0x15,
	0x0c, 0xdc,
	0x10, 0x00,
    0x11, 0x60,
    0x12, 0x07,
   	0x13, 0x80,
	0x14, 0x65,
	0x17, 0x02,
	0x18, 0x00,
	0x19, 0x4e,
	0x1a, 0x0f,
	0x1c, 0x0f,
	0x1d, 0x09,
	0x2a, 0x78,
	0x2c, 0x33,
	0x31, 0x10,
	0x33, 0x20,
	0x6b, 0x06,
	0x6c, 0x24,
	0x6d, 0x0a,
	0xb7, 0x06,
	0x06, 0x80,
	0xba, 0x77,
	0x6E, 0x70,
	0xFF, 0xFF
};
void i2cInit_TW9900_1(void)
{
    u8  data,bAddress,bData;
    u32 x,bIndex;

    DEBUG_I2C("---Init TW9900_1---\n");
#if IS_COMMAX_DOORPHONE
    VideoDecoder_reset();
#endif
    //----------Reset TW9900-------------//
/*	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
*/
    OSTimeDly(2);
	x=0;
    while (TW9900_init_value[x] != 0xFF)
	{
		bAddress = TW9900_init_value[x++];
		bData = TW9900_init_value[x++];
		i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_ADDR);
    }
    OSTimeDly(1);
    i2cRead_TW9900(0x1C, &data,I2C_TW9900_RD_SLAV_ADDR);
    DEBUG_I2C("bData=%x \n\n",data);
    data &= 0x70;
    if(data==0)//NTSC
    {
		bIndex=0;
		while (TW9900A_CCIR_NTSC_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9900A_CCIR_NTSC_DataSet[bIndex++];
			bData = TW9900A_CCIR_NTSC_DataSet[bIndex++];
			i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_ADDR);
		}
    //    i2cWrite_TW9900(0x08, 0x13,I2C_TW9900_WR_SLAV_ADDR);
    //    i2cWrite_TW9900(0x09, 0xF2,I2C_TW9900_WR_SLAV_ADDR);
        DEBUG_I2C("NTSC \n\n");
    }
    else if(data==0x10)
    {
	    bIndex=0;
		while (TW9900A_CCIR_PAL_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9900A_CCIR_PAL_DataSet[bIndex++];
			bData = TW9900A_CCIR_PAL_DataSet[bIndex++];
			i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_ADDR);
		}
    //    i2cWrite_TW9900(0x08, 0x19,I2C_TW9900_WR_SLAV_ADDR);
    //    i2cWrite_TW9900(0x09, 0x1F,I2C_TW9900_WR_SLAV_ADDR);
        DEBUG_I2C("PAL \n\n");
    }else
    {
        i2cWrite_TW9900(0x08, 0x13,I2C_TW9900_WR_SLAV_ADDR);
        i2cWrite_TW9900(0x09, 0xF2,I2C_TW9900_WR_SLAV_ADDR);
        DEBUG_I2C("other \n\n");
    }
}

void i2cInit_TW9900_2(void)
{
    u8  data,bAddress,bData;
    u32 i,x,bIndex;

    DEBUG_I2C("---Init TW9900_2---\n");
#if IS_COMMAX_DOORPHONE
    //VideoDecoder_reset();
#endif
#if (HW_BOARD_OPTION == MR9670_WOAN)
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV2_VIDEO_RESET, 0);
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV2_VIDEO_RESET, 1);
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
#endif
    //----------Reset TW9900-------------//
    /*
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV2_VIDEO_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
	*/

    x=0;
    while (TW9900_init_value[x] != 0xFF)
	{
		bAddress = TW9900_init_value[x++];
		bData = TW9900_init_value[x++];
		i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_2_ADDR);
	}
    OSTimeDly(5);
#if 0
    i2cWrite_TW9900(0x02,0x44,I2C_TW9900_WR_SLAV_2_ADDR);
    OSTimeDly(3);
#endif
    i2cRead_TW9900(0x1C, &data,I2C_TW9900_RD_SLAV_2_ADDR);
    DEBUG_I2C("bData=%x \n\n",data);
    data &= 0x70;
    if(data==0)//NTSC
    {
		bIndex=0;
		while (TW9900A_CCIR_NTSC_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9900A_CCIR_NTSC_DataSet[bIndex++];
			bData = TW9900A_CCIR_NTSC_DataSet[bIndex++];
			i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_2_ADDR);
		}
    //    i2cWrite_TW9900(0x08, 0x13,I2C_TW9900_WR_SLAV_ADDR);
    //    i2cWrite_TW9900(0x09, 0xF2,I2C_TW9900_WR_SLAV_ADDR);
        DEBUG_I2C("NTSC \n\n");
    }
    else if(data==0x10)
    {
	    bIndex=0;
		while (TW9900A_CCIR_PAL_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9900A_CCIR_PAL_DataSet[bIndex++];
			bData = TW9900A_CCIR_PAL_DataSet[bIndex++];
			i2cWrite_TW9900(bAddress, bData,I2C_TW9900_WR_SLAV_2_ADDR);
		}
    //    i2cWrite_TW9900(0x08, 0x19,I2C_TW9900_WR_SLAV_ADDR);
    //    i2cWrite_TW9900(0x09, 0x1F,I2C_TW9900_WR_SLAV_ADDR);
        DEBUG_I2C("PAL \n\n");
    }else
    {
        i2cWrite_TW9900(0x08, 0x13,I2C_TW9900_WR_SLAV_2_ADDR);
        i2cWrite_TW9900(0x09, 0xF2,I2C_TW9900_WR_SLAV_2_ADDR);
        DEBUG_I2C("other \n\n");
    }
}
#elif(TV_DECODER == TW9910)
#if 0
u8 TW9910A_CCIR_init_DataSet[] =
{
	0x00, 0x59, 0x02, 0x40, 0x03, 0xA2, 0x04, 0x00, 0x05, 0x01, 0x06, 0x00,
	0x0B, 0xD0, 0x0C, 0xCC, 0x0D, 0x00,
	0x0E, 0x11, 0x0F, 0x00, 0x10, 0x00, 0x11, 0x64, 0x12, 0x11, 0x13, 0x80, 0x14, 0x80,
	0x15, 0x00, 0x16, 0x00, 0x17, 0x30, 0x18, 0x44, 0x19, 0x57, 0x1A, 0x0F, 0x1B, 0xC0,
	0x1D, 0x7F, 0x1F, 0x00, 0x20, 0x50, 0x21, 0x42, 0x22, 0xF0,
	0x23, 0xD8, 0x24, 0xBC, 0x25, 0xB8, 0x26, 0x44, 0x27, 0x2A, 0x28, 0x00, 0x29, 0x03,
	0x2A, 0x78, 0x2B, 0x44, 0x2C, 0x30, 0x2D, 0x07, 0x2E, 0xA5, 0x2F, 0xE0, 0x30, 0x00,
	0x31, 0x10, 0x32, 0xFF, 0x33, 0x05, 0x34, 0x1A, 0x35, 0x00, 0x36, 0x00, 0x37, 0x00,
	0x38, 0x00, 0x39, 0x00, 0x3A, 0x00, 0x3B, 0x00, 0x3C, 0x00, 0x3D, 0x00, 0x3E, 0x00,
	0x3F, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00,
	0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x4A, 0x00, 0x4B, 0x00, 0x4C, 0x0D,
	0x4D, 0x40, 0x4E, 0x00, 0x4F, 0x00, 0x50, 0xA0, 0x51, 0x22, 0x52, 0x31, 0x53, 0x80,
	0x54, 0x00, 0x55, 0x00, 0x56, 0x00, 0x57, 0x00, 0x58, 0x00, 0x59, 0x00, 0x5A, 0x00,
	0x5B, 0x00, 0x5C, 0x00, 0x5D, 0x00, 0x5E, 0x00, 0x5F, 0x00, 0x60, 0x00, 0x61, 0x00,
	0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x65, 0x00, 0x66, 0x00, 0x67, 0x00, 0x68, 0x00,
	0x69, 0x00, 0x6A, 0x00, 0x6B, 0x26, 0x6C, 0x36, 0x6D, 0xF0, 0x6E, 0x28, 0x6F, 0x24,
	0xFF, 0xFF
};
u8 TW9910A_CCIR_NTSC_DataSet[] =
{
	0x01, 0x68,
	0x07, 0x02, 0x08, 0x13, 0x09, 0xF2, 0x0A, 0x10,
	0x1C, 0x07, 0x1E, 0x08,0xFF, 0xFF
};
u8 TW9910A_CCIR_PAL_DataSet[] =
{
	0x01, 0x69,
	0x07, 0x12, 0x08, 0x19, 0x09, 0x1F, 0x0A, 0x0C,
	0x1C, 0x17, 0x1E, 0x18, 0xFF, 0xFF
};
#else
u8 TW9910A_CCIR_init_DataSet[] =
{
	0x00, 0x58, 0x02, 0x40, 0x03, 0x80, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00,
	0x0B, 0xD0, 0x0C, 0xDC, 0x0D, 0x00,
	0x0E, 0x11, 0x0F, 0x00,
//	0x10, 0x00, 0x11, 0x5C, 0x12, 0x15, 0x13, 0x80, 0x14, 0x80,
	0x10, 0x00, 0x11, 0x60, 0x12, 0x07,	0x13, 0x80, 0x14, 0x65,
	0x15, 0x00, 0x16, 0x53, 0x17, 0x80, 0x18, 0x44, 0x19, 0x58, 0x1A, 0x0A, 0x1B, 0x00,
	0x1D, 0x7F, 0x1F, 0x00, 0x20, 0x50, 0x21, 0x22, 0x22, 0xF0,
	0x23, 0xD8, 0x24, 0xBC, 0x25, 0xB8, 0x26, 0x44, 0x27, 0x38, 0x28, 0x00, 0x29, 0x00,
	0x2A, 0x78, 0x2B, 0x44, 0x2C, 0x30, 0x2D, 0x14, 0x2E, 0xA5, 0x2F, 0xE4, 0x30, 0x00,
	0x31, 0x10, 0x32, 0x00, 0x33, 0x05, 0x34, 0x1E, 0x35, 0x00, 0x36, 0x00, 0x37, 0x00,
	0x38, 0x00, 0x39, 0x00, 0x3A, 0x00, 0x3B, 0x00, 0x3C, 0x00, 0x3D, 0x00, 0x3E, 0x00,
	0x3F, 0x00, 0x40, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00,
	0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x4A, 0x00, 0x4B, 0x00, 0x4C, 0x00,
	0x4D, 0x00, 0x4E, 0x00, 0x4F, 0x00, 0x50, 0xA0, 0x51, 0x22, 0x52, 0x31, 0x53, 0x80,
	0x54, 0x00, 0x55, 0x00, 0x56, 0x00, 0x57, 0x00, 0x58, 0x00, 0x59, 0x00, 0x5A, 0x00,
	0x5B, 0x00, 0x5C, 0x00, 0x5D, 0x00, 0x5E, 0x00, 0x5F, 0x00, 0x60, 0x00, 0x61, 0x00,
	0x62, 0x00, 0x63, 0x00, 0x64, 0x00, 0x65, 0x00, 0x66, 0x00, 0x67, 0x00, 0x68, 0x00,
	0x69, 0x00, 0x6A, 0x00, 0x6B, 0x2C, 0x6C, 0x60, 0x6D, 0x00, 0x6E, 0x00, 0x6F, 0x24,
	0x70, 0x00, 0x71, 0x00, 0x72, 0x00, 0x73, 0x00, 0x74, 0x00, 0x75, 0x00, 0x76, 0x00,
	0x77, 0x00, 0x78, 0x00, 0x79, 0x00, 0x7A, 0x00, 0x7B, 0x00, 0x7C, 0x00, 0x7D, 0x00,
	0x7E, 0x00, 0x7F, 0x00, 0x80, 0x00, 0x81, 0x00, 0x82, 0x00, 0x83, 0x00, 0x84, 0x00,
	0x85, 0x00, 0x86, 0x00, 0x87, 0x00, 0x88, 0x00, 0x89, 0x00, 0x8A, 0x00, 0x8B, 0x00,
	0x8C, 0x00, 0x8D, 0x00, 0x8E, 0x00, 0x8F, 0x00, 0x90, 0x00, 0x91, 0x00, 0x92, 0x00,
	0x93, 0x00, 0x94, 0x00, 0x95, 0x00, 0x96, 0x00, 0x97, 0x00, 0x98, 0x00, 0x99, 0x00,
	0x9A, 0x00, 0x9B, 0x00, 0x9C, 0x00, 0x9D, 0x00, 0x9E, 0x00, 0x9F, 0x00, 0xA0, 0x00,
	0xA1, 0x00, 0xA2, 0x00, 0xA3, 0x00, 0xA4, 0x00, 0xA5, 0x00, 0xA6, 0x00, 0xA7, 0x00,
	0xA8, 0x00, 0xA9, 0x00, 0xAA, 0x00, 0xAB, 0x00, 0xAC, 0x00, 0xAD, 0x00, 0xAE, 0x00,
	0xAF, 0x00, 0xB0, 0x00, 0xB1, 0x00, 0xB2, 0x00, 0xB3, 0x00, 0xB4, 0x00, 0xB5, 0x00,
	0xB6, 0x00, 0xB7, 0x00, 0xB8, 0x00, 0xB9, 0x00, 0xBA, 0x00, 0xBB, 0x00, 0xBC, 0x00,
	0xBD, 0x00, 0xBE, 0x00, 0xBF, 0x00, 0xC0, 0x00, 0xC1, 0x00, 0xC2, 0x00, 0xC3, 0x00,
	0xC4, 0x00, 0xC5, 0x00, 0xC6, 0x00, 0xC7, 0x00, 0xC8, 0x00, 0xC9, 0x00, 0xCA, 0x00,
	0xCB, 0x00, 0xCC, 0x00, 0xCD, 0x00, 0xCE, 0x00, 0xCF, 0x00, 0xD0, 0x00, 0xD1, 0x00,
	0xD2, 0x00, 0xD3, 0x00, 0xD4, 0x00, 0xD5, 0x00, 0xD6, 0x00, 0xD7, 0x00, 0xD8, 0x00,
	0xD9, 0x00, 0xDA, 0x00, 0xDB, 0x00, 0xDC, 0x00, 0xDD, 0x00, 0xDE, 0x00, 0xDF, 0x00,
	0xE0, 0x00, 0xE1, 0x00, 0xE2, 0x00, 0xE3, 0x00, 0xE4, 0x00, 0xE5, 0x00, 0xE6, 0x00,
	0xE7, 0x00, 0xE8, 0x00, 0xE9, 0x00, 0xEA, 0x00, 0xEB, 0x00, 0xEC, 0x00, 0xED, 0x00,
	0xEE, 0x00, 0xEF, 0x00, 0xF0, 0x00, 0xF1, 0x00, 0xF2, 0x00, 0xF3, 0x00, 0xF4, 0x00,
	0xF5, 0x00, 0xF6, 0x00, 0xF7, 0x00, 0xF8, 0x00, 0xF9, 0x00, 0xFA, 0x00, 0xFB, 0x00,
	0xFC, 0x00, 0xFD, 0x00, 0xFE, 0x00, 0xFF, 0xFF
};
u8 TW9910A_CCIR_NTSC_DataSet[] =
{
	0x01, 0x78,
	0x07, 0x02, 0x08, 0x14, 0x09, 0xF0, 0x0A, 0x18,
	0x1C, 0x07, 0x1E, 0x18, 0xFF, 0xFF
};
u8 TW9910A_CCIR_PAL_DataSet[] =
{
	0x01, 0x79,
	0x07, 0x12, 0x08, 0x18, 0x09, 0x20, 0x0A, 0x0E,
	0x1C, 0x17, 0x1E, 0x08, 0xFF, 0xFF
};
#endif
void i2cInit_TW9910(void)
{
    u8  data,bAddress,bData;
    u32 x,bIndex;

    DEBUG_I2C("--- Init TW9910 ---\n");
#if IS_COMMAX_DOORPHONE
    VideoDecoder_reset();
#endif
    //----------Reset TW9900-------------//
/*	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 0);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//OSTimeDly(1);
	gpioSetLevel(GPIO_DV1_RESET_GROUP, GPIO_DV1_RESET, 1);	//reset  WT8861
    for(i=0; i<300 ;i++);
	//	OSTimeDly(1);
*/
    OSTimeDly(2);
	x=0;
    while (TW9910A_CCIR_init_DataSet[x] != 0xFF)
	{
		bAddress = TW9910A_CCIR_init_DataSet[x++];
		bData = TW9910A_CCIR_init_DataSet[x++];
		i2cWrite_TW9910(bAddress, bData,I2C_TW9910_WR_SLAV_ADDR);
    }
    OSTimeDly(1);
    i2cRead_TW9910(0x1C, &data,I2C_TW9910_RD_SLAV_ADDR);
//	DEBUG_I2C("orl bData=%x \n\n",data);
    data &= 0x70;
//	DEBUG_I2C("bData=%x \n\n",data);
	bIndex=0;
    if(data==0x00)//NTSC
    {
		while (TW9910A_CCIR_NTSC_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9910A_CCIR_NTSC_DataSet[bIndex++];
			bData = TW9910A_CCIR_NTSC_DataSet[bIndex++];
			i2cWrite_TW9910(bAddress, bData,I2C_TW9910_WR_SLAV_ADDR);
		}
        DEBUG_I2C("NTSC \n\n");
    }
    else if(data==0x10)//PAL
    {
		while (TW9910A_CCIR_PAL_DataSet[bIndex] != 0xFF)
		{
			bAddress = TW9910A_CCIR_PAL_DataSet[bIndex++];
			bData = TW9910A_CCIR_PAL_DataSet[bIndex++];
			i2cWrite_TW9910(bAddress, bData,I2C_TW9910_WR_SLAV_ADDR);
		}
        DEBUG_I2C("PAL \n\n");
    }
}

#elif(TV_DECODER == TW2866)

u8 TW2866_init_value[] =
{
    0x80,0x3f, // reset all
    0xFA,0x40,	
	0xFB,0x2F,	// For Version A, C dev, enable IRQ
	0xFC,0xFF,	
	0xDB,0xC1,	
	0x73,0x01,	//A5DET_ENA
	0x9C,0xA0,	 
	0x9E,0x52,	
	0xD2,0x01,	
	0xDD,0x00,	//Audio Mix Ratio
	0xDE,0x00,	
	0xE1,0xC0,	//Audio Detection Period and Threshold
	0xE2,0xAA,	
	0xE3,0xAA,	
	0xF8,0x64,	
	0xF9,0x11,	//Video misc 
	0xAA,0x00,	//Video AGC
    //0x60,0x05,	// 2X XTI input(54MHz)
    0x60,0x07,	// 2X XTI input(54MHz) test timer
    //0x60,0x15,	// 4X XTI input(27MHz)
    //0x60,0x17,	// 4X XTI input(27MHz) test timer
	0x70,0x08,	//AFAUTO=1
	0xF8,0xC4, //ACPL=1(loop open),ACPL=0(loop closed) makes oscilating sound 
	0xF9,0x51, //Reset default is better   
	0xDF,0x80, // AOGAIN=8h
	0x7F,0x80, //AIGAIN5=8h
	0x6B,0x0f, //CLKPO3/CLKNO3 off
	0x6C,0x0f, //CLKPO4/CLKNO4 off
	0xCF,0x80,	//cascade ALINKI/ALINKO enable
	0xCF,0x00,
	0x89,0x05,
	0x70,0x08,
	0xD0,0x88,
	0xD1,0x88,
	0x7F,0x80,
	0xFC,0xFF,
	0x73,0x01,
	0xFB,0x3F,
	0xE1,0xE0,
	0xE2,0x22,
	0xE3,0x22,
	0x7E,0xA2,
	0xB3,0x00,
	0xB4,0x1C,
	0xB5,0x1D,
	0xB6,0x1A,
	0xB7,0x1B,
	0x75,0x00,
	0x76,0x19,
    0xFF,0xFF
};

u8 tbl_ntsc_TW2866_common[] = {  
//CH1			CH2			CH3			CH4
	0x00,0x00,0x64,0x11,		//...		0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33	Register Addr.
	0x80,0x80,0x00,0x02,		//...		0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37	Register Addr.
	0x12,0xF0,0x0C,0xD0,		//...		0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b	Register Addr.
	0x00,0x00,0x07,0x7F,		//...		0x0c~0x0f	0x1c~0x1f	0x2c~0x2f	0x3c~0x3f	Register Addr.
	0xFF                       // end
};
u8 tbl_pal_TW2866_common[] = {  
//CH1			CH2			CH3			CH4
	0x00,0x00,0x64,0x11,		//...		0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33	Register Addr.
	0x80,0x80,0x00,0x12,		//...		0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37	Register Addr.
	0x12,0x20,0x0C,0xD0,		//...		0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b	Register Addr.
	0x00,0x00,0x07,0x7F,		//...		0x0c~0x0f	0x1c~0x1f	0x2c~0x2f	0x3c~0x3f	Register Addr.
	0xFF                       // end
};

void i2cInit_TW2866(void)
{
    u8  bAddress,bData;
    u32 x,bIndex;

    DEBUG_I2C("--- Init TW2866 ---\n");

	x=0;
    while (TW2866_init_value[x] != 0xFF)
	{
		bAddress = TW2866_init_value[x++];
		bData = TW2866_init_value[x++];
		i2cWrite_TW2866(bAddress, bData);
    }
//----------------------		 TW2867 without internal analog PLL, OSC. 27MHz input to XTI---//    
    //i2cWrite_TW2866(0x60,0x40);
    //i2cWrite_TW2866(0x61,0x10);
#if 1
//----------------------		 TW2867 internal analog PLL, Crystal 54MHz to XTI & XTO---//      
    i2cWrite_TW2866(0x60,0x07);
    i2cWrite_TW2866(0x61,0x03);
#else
//----------------------		 TW2867 internal analog PLL, Crystal 27MHz to XTI & XTO---//
    i2cWrite_TW2866(0x60,0x17);
    i2cWrite_TW2866(0x61,0x03);
#endif
#if 1
//-- TW2867 VD1 (ch0/ch1/ch2/ch3) /VD2 (ch0/ch1/ch2/ch3) /VD3 (ch0/ch1/ch2/ch3) /VD4 (ch0/ch1/ch2/ch3) Four D1 108MHz 656 output-//    
    i2cWrite_TW2866(0xCA,0XAA);
    i2cWrite_TW2866(0xFA,0x4A);
    i2cWrite_TW2866(0xFB,0x2F);
    i2cWrite_TW2866(0x6A,0x0A);
    i2cWrite_TW2866(0x6B,0x0A);
    i2cWrite_TW2866(0x6C,0x0A);
    
#else
//----------------------		 TW2867 VD1/VD2/VD3/VD4 D1 27MHz 656 output-------------//
    i2cWrite_TW2866(0xCA,0X00);
    i2cWrite_TW2866(0xFA,0x40);
    i2cWrite_TW2866(0xFB,0x2F);
    i2cWrite_TW2866(0x6A,0x00);
    i2cWrite_TW2866(0x6B,0x00);
    i2cWrite_TW2866(0x6C,0x00);

    i2cWrite_TW2866(0x9E,0x42);   // no CHID
#endif

    i2cWrite_TW2866(0x9F,0x33);//Clock Output delay
#if 1
    bIndex=0;
		while (tbl_ntsc_TW2866_common[bIndex] != 0xFF)
		{
			bAddress = bIndex;
			bData = tbl_ntsc_TW2866_common[bIndex++];
			i2cWrite_TW2866(bAddress, bData);         // ch1
			i2cWrite_TW2866(bAddress + 0x10, bData);  // ch2
			i2cWrite_TW2866(bAddress + 0x20, bData);  // ch3
			i2cWrite_TW2866(bAddress + 0x30, bData);  // ch4
    }
        DEBUG_I2C("TW2866 NTSC MODE \n\n");

#else 
    bIndex=0;
    while (tbl_pal_TW2866_common[bIndex] != 0xFF)
		{
			bAddress = bIndex;
			bData = tbl_pal_TW2866_common[bIndex++];
			i2cWrite_TW2866(bAddress, bData);         // ch1
			i2cWrite_TW2866(bAddress + 0x10, bData);  // ch2
			i2cWrite_TW2866(bAddress + 0x20, bData);  // ch3
			i2cWrite_TW2866(bAddress + 0x30, bData);  // ch4
		}
        DEBUG_I2C("TW2866 PAL MODE \n\n");
#endif

}

#endif
void contrast_write(u8 data)
{
#if(TV_DECODER == WT8861)
    i2cWrite_WT8861(0x08,data,I2C_WT8861_WR_SLAV_ADDR);
    i2cWrite_WT8861(0x08,data,I2C_WT8861_WR_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cWrite_TW9900(0x11,data,I2C_TW9900_WR_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
        i2cWrite_TW9900(0x11,data,I2C_TW9900_WR_SLAV_2_ADDR);
    #endif
#elif(TV_DECODER == TW9910)
	i2cWrite_TW9910(0x11,data,I2C_TW9910_WR_SLAV_ADDR);

#endif
}
u8 contrast_read(u8* data)
{
    u8 bVal;
#if(TV_DECODER == WT8861)
    i2cRead_WT8861(0x08,&bVal,I2C_WT8861_RD_SLAV_ADDR);
    i2cRead_WT8861(0x08,&bVal,I2C_WT8861_RD_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cRead_TW9900(0x11,&bVal,I2C_TW9900_RD_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
    i2cRead_TW9900(0x11,&bVal,I2C_TW9900_RD_SLAV_2_ADDR);
#endif
#elif(TV_DECODER == TW9910)
    i2cRead_TW9910(0x11,&bVal,I2C_TW9910_RD_SLAV_ADDR);
#endif
    *data=bVal;
    return 0;
}
void brightness_write(u8 data)
{
#if(TV_DECODER == WT8861)
    i2cWrite_WT8861(0x09,data,I2C_WT8861_WR_SLAV_ADDR);
    i2cWrite_WT8861(0x09,data,I2C_WT8861_WR_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cWrite_TW9900(0x10,data,I2C_TW9900_WR_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
    i2cWrite_TW9900(0x10,data,I2C_TW9900_WR_SLAV_2_ADDR);
#endif
#elif(TV_DECODER == TW9910)
    i2cWrite_TW9910(0x10,data,I2C_TW9910_WR_SLAV_ADDR);
#endif
}
u8 brightness_read(u8* data)
{
    u8 bVal;
#if(TV_DECODER == WT8861)
    i2cRead_WT8861(0x09,&bVal,I2C_WT8861_RD_SLAV_ADDR);
    i2cRead_WT8861(0x09,&bVal,I2C_WT8861_RD_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cRead_TW9900(0x10,&bVal,I2C_TW9900_RD_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
    i2cRead_TW9900(0x10,&bVal,I2C_TW9900_RD_SLAV_2_ADDR);
#endif
#elif(TV_DECODER == TW9910)
    i2cRead_TW9910(0x10,&bVal,I2C_TW9910_RD_SLAV_ADDR);
#endif
    *data=bVal;
    return 0;
}
void saturation_write(u8 data)
{
#if(TV_DECODER == WT8861)
    i2cWrite_WT8861(0x0A,data,I2C_WT8861_WR_SLAV_ADDR);
    i2cWrite_WT8861(0x0A,data,I2C_WT8861_WR_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cWrite_TW9900(0x13,data,I2C_TW9900_WR_SLAV_ADDR);
	i2cWrite_TW9900(0x14,data,I2C_TW9900_WR_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
	i2cWrite_TW9900(0x13,data,I2C_TW9900_WR_SLAV_2_ADDR);
    i2cWrite_TW9900(0x14,data,I2C_TW9900_WR_SLAV_2_ADDR);
#endif
#elif(TV_DECODER == TW9910)
    i2cWrite_TW9910(0x13,data,I2C_TW9910_WR_SLAV_ADDR);
    i2cWrite_TW9910(0x14,data,I2C_TW9910_WR_SLAV_ADDR);
#endif
}
u8 saturation_read(u8* data)
{
    u8 bVal;
#if(TV_DECODER == WT8861)
    i2cRead_WT8861(0x0A,&bVal,I2C_WT8861_RD_SLAV_ADDR);
    i2cRead_WT8861(0x0A,&bVal,I2C_WT8861_RD_SLAV_2_ADDR);
#elif(TV_DECODER == TW9900)
    i2cRead_TW9900(0x13,&bVal,I2C_TW9900_RD_SLAV_ADDR);
    i2cRead_TW9900(0x14,&bVal,I2C_TW9900_RD_SLAV_ADDR);
    #if IS_COMMAX_DOORPHONE
	i2cRead_TW9900(0x13,&bVal,I2C_TW9900_RD_SLAV_2_ADDR);
    i2cRead_TW9900(0x14,&bVal,I2C_TW9900_RD_SLAV_2_ADDR);
#endif
#elif(TV_DECODER == TW9910)
   	i2cRead_TW9910(0x13,&bVal,I2C_TW9910_RD_SLAV_ADDR);
    i2cRead_TW9910(0x14,&bVal,I2C_TW9910_RD_SLAV_ADDR);
#endif
    *data=bVal;
    return 0;
}
void CH_Channel_write(u8 ch,u8 data)
{
#if(TV_DECODER == WT8861)
	if(ch==1)
    {
        if(data==1)
            i2cWrite_WT8861(0xC0,0x00,I2C_WT8861_WR_SLAV_ADDR);
        else if (data==2)
            i2cWrite_WT8861(0xC0,0x01,I2C_WT8861_WR_SLAV_ADDR);
		else if(data==3)
            i2cWrite_WT8861(0xC0,0x04,I2C_WT8861_WR_SLAV_ADDR);
        else if (data==4)
            i2cWrite_WT8861(0xC0,0x0C,I2C_WT8861_WR_SLAV_ADDR);
    }
    else
    {
        if(data==1)
            i2cWrite_WT8861(0x02,0x40,I2C_WT8861_WR_SLAV_2_ADDR);
        else if (data==2)
            i2cWrite_WT8861(0x02,0x44,I2C_WT8861_WR_SLAV_2_ADDR);
    }
#elif(TV_DECODER == TW9900)
    if(ch==1)
    {
        if(data==1)
        {
            data=0x40;
            i2cWrite_TW9900(0x02,data,I2C_TW9900_WR_SLAV_ADDR);
        }
        else if (data==2)
        {
            data=0x44;
            i2cWrite_TW9900(0x02,data,I2C_TW9900_WR_SLAV_ADDR);
        }
    }
#if IS_COMMAX_DOORPHONE
    else
    {
        if(data==1)
        {
            data=0x40;
            i2cWrite_TW9900(0x02,data,I2C_TW9900_WR_SLAV_2_ADDR);
        }
        else if (data==2)
        {
            data=0x44;
            i2cWrite_TW9900(0x02,data,I2C_TW9900_WR_SLAV_2_ADDR);
        }
    }
#endif
#elif(TV_DECODER == TW9910)
    if(ch==1)
    {
        if(data==1)
        {
            data=0x40;
            i2cWrite_TW9910(0x02,data,I2C_TW9910_WR_SLAV_ADDR);
        }
        else if (data==2)
        {
            data=0x44;
            i2cWrite_TW9910(0x02,data,I2C_TW9910_WR_SLAV_ADDR);
        }
    }
    else
    {
        if(data==1)
        {
            data=0x48;
            i2cWrite_TW9910(0x02,data,I2C_TW9910_WR_SLAV_ADDR);
		}
        else if (data==2)
		{
            data=0x4C;
            i2cWrite_TW9910(0x02,data,I2C_TW9910_WR_SLAV_ADDR);
        }
    }
#endif
}

#if (TOUCH_KEY == TOUCH_KEY_BS83B12)

// JESMAY 使用的BS83B12 I2C比較特殊
#if (HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD || (HW_BOARD_OPTION == MR8120_RX_GCT_SC7700))
s32 i2cRead_BS83B12(u8 id, u8* pData)
{
    u8 err;

	OSSemPend(i2cWDTProtect, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("rst Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemReq is %d.\n", id, *pData, err);
        sysI2cRst();
        OSSemPost(i2cWDTProtect);
        return 0;
    }
    /* clk_div= 3840 */
    I2cCtrl = I2C_ENA  | I2C_1B | I2C_CLK_DIV_3840 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        OSSemPost(i2cWDTProtect);
        sysI2cRst();
        DEBUG_I2C("rst Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", id, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    OSSemPost(i2cWDTProtect);
    return 1;
}
#else
s32 i2cRead_BS83B12(u8 id, u8* pData)
{
    u8 err;

    OSSemPend(i2cWDTProtect, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemReq is %d.\n", id, *pData, err);
        sysI2cRst();
        OSSemPost(i2cWDTProtect);
        return 0;
    }
    /* clk_div= 3840 */
    I2cCtrl = I2C_ENA  | I2C_1B | I2C_CLK_DIV_3840 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
    I2cCtrl |= I2C_TRIG;
    I2cCtrl |= I2C_R_ACK;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        OSSemPost(i2cWDTProtect);
        sysI2cRst();
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", id, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    if(*pData == 0xff)
    {
        *pData=0;
    }
    OSSemPost(i2cSemReq);
    OSSemPost(i2cWDTProtect);
    return 1;
}
#endif

s32 i2cRead_BS83B12_2Byte(u8 id, u16* pData)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemReq is %d.\n", id, *pData, err);
        return 0;
    }
    /* clk_div= 3840 */
    I2cCtrl = I2C_ENA  | I2C_2B | I2C_CLK_DIV_3840 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
    I2cCtrl |= I2C_TRIG;
    I2cCtrl |= I2C_R_ACK;


    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", id, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    return 1;
}
#elif(TOUCH_KEY == TOUCH_KEY_CBM7320)

s32 i2cRead_CBM7320_Manual(u8* pucData)
{                                                                                                                                                                   
    u8 err;                                                                                                                                                         
    u32 i;                                                                                                                                                          
    u8 subReg=0;                                                                                                                                                    
    u8 subReg2=0;                                                                                                                                                   

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);                                                                                                                        
    if (err != OS_NO_ERR)                                                                                                                                           
    {                                                                                                                                                               
        DEBUG_I2C("Error: i2cRead_CBM7320_Manual(0x%02x), i2cSemReq is %d.\n", *pucData, err);                                                  
        return 0;                                                                                                                                                   
    }                                                                                                                                                               
                                                                                                                                                                    
    I2cCtrl     = I2C_MANUAL_EN  |I2C_1B| I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_480  | (I2C_CBM7320_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
                                                                                                                                                                    
    /* Start */                                                                                                                                                     
    I2cManu     = I2C_ManuSBIT;                                                                                                                                     
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);                                                                                
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))                                                                                                              
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Start 1 Error!!!\n");                                                                                            
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                                    
    /* Slave Addr */                                                                                                                                                
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;                                                                                                              
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);                                                                           
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))                                                                                                         
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_CBM7320_RD_SLAV_ADDR);                                                   
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                               
                                                                                                                                                                    
    /* Reg Addr (bit 0:7)                                                                                                                                         
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuWriteData1;                                                                                                             
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1)); i++);                                                                          
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuWriteData1))                                                                                                        
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", ucRegAddr);                                                         
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
         */                                                                                                                                                           

    /* Read Data */                                                                                                                                                 
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;                                                                                                                                 
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);                                                                            
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))                                                                                                          
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Read Data Error!!!\n");                                                                                          
		return 0;                                                                                                                                                       
	}
    *pucData  = (u8)I2cData;

    /* Read Data */                                                                                                                                                 
    I2cManu     = I2C_ManuReadData;                                                                                                                                 
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);                                                                            
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))                                                                                                          
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Read Data Error!!!\n");                                                                                          
		return 0;                                                                                                                                                       
	}                                                                                                                                                       
                                                                                                                                                                    
    /* Stop */                                                                                                                                                      
    I2cManu     = I2C_ManuPBIT;                                                                                                                                     
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);                                                                                                                                                                                                                   
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))                                                                                                              
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Stop Error!!!\n");                                                                                               
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
    else                                                                                                                                                            
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
        //DEBUG_GREEN("pucData %x \n", *pucData);
		return 1;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                                    
}                                                                                                       



#elif (TOUCH_KEY == TOUCH_KEY_MA86P03)


s32 i2cRead_MA86P03(u8 reg , u8 *data )
{
    u8 err;

    OSSemPend(i2cWDTProtect, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
      //  DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemReq is %d.\n", id, addr, *pData, err);
      	OSSemPost(i2cWDTProtect);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_7680 | (((u32)I2C_MA86P03_RD_SLAV_ADDR) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
    I2cCtrl = I2cCtrl | (((u32)reg) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        OSSemPost(i2cWDTProtect);
       // DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemFin is %d.\n", id, addr, *pData, err);
        return 0;
    }

    *data = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    OSSemPost(i2cWDTProtect);
    return 1;

}

s32 i2cWrite_MA86P03(u8 reg, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_TW9900 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (I2C_MA86P03_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)reg) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_TW9900(), i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}


u8 i2cReadTouchKey(u8 *data)
{
    static u8 time_count=0;
    static u8 led_state=0;  //0: off 1: on  2: read data
    static u8 temp_data;
    u8 level =0;

    time_count++;


    gpioGetLevel(GPIO_GROUP_TOUCH_KEY,GPIO_BIT_TOUCH_KEY,&level);


    i2cRead_MA86P03(0x01, &temp_data);
    *data=temp_data;

    return 1;

}

#endif
#if((USE_BUILD_IN_RTC == 0) && (EXT_RTC_SEL == RTC_SD2068))
s32 i2cRead_SD2068(u8 ucRegAddr, u8* pucData)
{
#if(!GPIO_I2C_ENA)
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_SD2068(0x%02x), i2cSemReq is %d.\n", ucRegAddr,err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_RTC_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_SENS_CLK_DIV_1| I2C_INT_ENA;
    I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;
        
    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_SD2068(0x%02x), i2cSemFin is %d.\n",  ucRegAddr, err);
        OSSemPost(i2cSemReq);
        return 0;
    }

    *pucData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    return 1;

#else

	u8  err;
	u32 i;
    GPIO_IIC_CFG    stGpio_Iic;
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_SD2068, sizeof (GPIO_IIC_CFG));

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_SD2068 i2cSemReq is %d.\n", err);
		return 0;
	}

   /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

     /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_RD_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData);
    /* Inverse Acknowledge */
    gpio_IIC_nAck_W(&stGpio_Iic);

    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
    DEBUG_I2C("Error: i2cRead_SD2068() gpio IIC Read Addr [%#x] Error!!!\n", ucRegAddr);

    return 0;
#endif


}
#if(GPIO_I2C_ENA)
void WriteTimeOn(void)
{
    u8 i;
	GPIO_IIC_CFG    stGpio_Iic;
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_SD2068, sizeof (GPIO_IIC_CFG));
    
     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    gpio_IIC_W_Byte(&stGpio_Iic, 0x10);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    gpio_IIC_W_Byte(&stGpio_Iic, 0x80); ///  WRTC1 = 1
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
   
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    
    for(i = 0; i < 20; i++);
        
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    gpio_IIC_W_Byte(&stGpio_Iic, 0x0F);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    gpio_IIC_W_Byte(&stGpio_Iic, 0x84); //  WRTC2, WRTC3 = 1
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    return ;
    
    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
	DEBUG_I2C("WriteTimeOn   Error!!!\n");
    return;

}
void WriteTimeOff(void)
{
	GPIO_IIC_CFG    stGpio_Iic;
    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_SD2068, sizeof (GPIO_IIC_CFG));
    
     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    gpio_IIC_W_Byte(&stGpio_Iic, 0x0F);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    gpio_IIC_W_Byte(&stGpio_Iic, 0x0);  //  WRTC2, WRTC3 = 0
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    

    gpio_IIC_W_Byte(&stGpio_Iic, 0x0);  //  WRTC1 =0
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    return;
    
    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
	DEBUG_I2C("WriteTimeOff Error!!!\n");
    return;

}
#endif
s32 i2cWrite_SD2068(u8 ucRegAddr, u8 ucData)
{
#if(!GPIO_I2C_ENA)
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_SD2068(0x%02x, 0x%02x), i2cSemReq is %d.\n", ucRegAddr, ucData, err);
        return 0;
    }

    I2cData = ((u32)ucData);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_RTC_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT)| I2C_INT_ENA;
    I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;
        
    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", ucRegAddr, ucData, err);
        OSSemPost(i2cSemReq);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
#else
	u8  err;
	u32 i;
	GPIO_IIC_CFG    stGpio_Iic;

    memcpy(&stGpio_Iic, &GPIO_IIC_CFG_SD2068, sizeof (GPIO_IIC_CFG));

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		/* signal event semaphore */
		OSSemPost(i2cSemReq);

		DEBUG_I2C("Error: i2cWrite_SD2068 i2cSemReq is %d.\n", err);
		return 0;
	}
    WriteTimeOn();
    
     /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);


    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, I2C_RTC_WR_SLAV_ADDR);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;

    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;


    /* I2C Write data */
    gpio_IIC_W_Byte(&stGpio_Iic, ucData);
    /* Acknowledge */
    gpio_IIC_Ack_R(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    }
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);

    WriteTimeOff();

    OSSemPost(i2cSemReq);
    return 1;

    RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    OSSemPost(i2cSemReq);
	DEBUG_I2C("Error: i2cWrite_SD2068() gpio IIC Write Addr [%#x] Error!!!\n", ucRegAddr);

    return 0;
#endif



}
static BOOLEAN IsTimeValid(RTC_DATE_TIME* ptime)
{
  if ((ptime->year>99) || (ptime->year < 10))  return FALSE;

  if (ptime->month>12) return FALSE;

  if (ptime->day>32) return FALSE;

  if (ptime->hour>24) return FALSE;

  if (ptime->min>60) return FALSE;

  if (ptime->sec>60) return FALSE;

  return TRUE;
}

__inline u8 GetBCDtoDecimal(u8 data )
{
	return  ( (data>>4)&0xF) *10 + ( (data)&0xF);
}
__inline u8 ConvertDecimaltoBCD( u8 data )
{
	return  ((data/10)<<4) | (data %10 );
}

BOOLEAN Get_SD2068_RTC(RTC_DATE_TIME *rtc_time)
{
	BOOLEAN bRet = FALSE;
	u8 data[7] = {0};
	u8 i;

	for(i=0;i<7;i++)
	{
		if(i2cRead_SD2068(i, &data[i]) == 0)
		{
			bRet = FALSE;
			DEBUG_I2C("SD2068 I2C Read 0x%02x Error\r\n", i);
            break;
		}
        else
            bRet = TRUE;
	}

    if(bRet){
    	rtc_time->sec   = GetBCDtoDecimal(data[0]);
    	rtc_time->min   = GetBCDtoDecimal(data[1]);
    	rtc_time->hour  = GetBCDtoDecimal(data[2] & 0x3F);
        rtc_time->week  = GetBCDtoDecimal(data[3]);
    	rtc_time->day   = GetBCDtoDecimal(data[4]);
    	rtc_time->month = GetBCDtoDecimal(data[5]);
    	rtc_time->year  = GetBCDtoDecimal(data[6]);
    }
    //DEBUG_I2C("Get_SD2068_RTC Date:%04d/%02d/%02d %02d:%02d:%02d\n",(rtc_time->year+2000), rtc_time->month, rtc_time->day, rtc_time->hour, rtc_time->min, rtc_time->sec);
    return bRet;
}
BOOLEAN Set_SD2068_RTC(RTC_DATE_TIME* date)
{
	///u8 data;
	BOOLEAN bRet = FALSE;
	u8 data[7] = {0};
	u8 i;

#if(!GPIO_I2C_ENA)
    u8 reg10,reg0F;
    i2cRead_SD2068(0x10, &reg10);
    i2cRead_SD2068(0x0F, &reg0F);
    
    i2cWrite_SD2068(0x10, reg10 | 0x80); //WRTC1 bit[7]
    i2cWrite_SD2068(0x0F, reg0F | 0x84); //WRTC2 bit[2], WRTC3 bit[7] => set 1, enter write-mode (WriteTimeOn)
#endif

	data[0] = ConvertDecimaltoBCD(date->sec);
	data[1] = ConvertDecimaltoBCD(date->min);
	data[2] = (ConvertDecimaltoBCD(date->hour)|0x80);  //Set to 24H Format
	data[3] = ConvertDecimaltoBCD(date->week);  //Set to 24H Format
	data[4] = ConvertDecimaltoBCD(date->day);
	data[5] = ConvertDecimaltoBCD(date->month);
	data[6] = ConvertDecimaltoBCD(date->year);
	for(i=0;i<7;i++)
	{
		if(i2cWrite_SD2068(i, data[i]) == 0)
		{
			bRet = FALSE;
			DEBUG_I2C("I2C Write 0x%02x Error\r\n", i);
            break;
		}
        else
            bRet = TRUE;
	}
    i2cWrite_SD2068(0x12, 0);
//    DEBUG_I2C("Set_SD2068_RTC Date:%04d/%02d/%02d %02d:%02d:%02d\n",data[0], data[1], data[2], data[4], data[5], data[6]);
#if(!GPIO_I2C_ENA)
    i2cWrite_SD2068(0x0F, reg0F);
    i2cWrite_SD2068(0x10, reg10); //WRTC1,2,3 => set 0, enter read-mode (WriteTimeOff)
#endif
    return bRet;
}

void SD2068_RTC_Init(void)
{
    u8 u8RTCF, batterybit = 0x01;
    u8 RTCValid;
    u8 cnt;
    BOOLEAN bRet=TRUE;
    RTC_DATE_TIME RtcTime;
    printf("## SD2068_RTC_Init\n");

	if(i2cRead_SD2068(0x0F, &u8RTCF) == 0)  // 判斷 SD2068 是否正常
	{
		DEBUG_I2C("Test SD2068 RTC, failed to read\r\n");
		return;
	}

    if(u8RTCF & batterybit)  //no pwr
    {
        RTCValid=1;
        DEBUG_I2C("no power 0x0F = %x \n", u8RTCF);
    }
    else   // success
    {
        RTCValid=0;
        DEBUG_I2C("RTC Valid\n\r");
    }
	if ( RTCValid == 0 )
	{
		for (cnt=1; cnt<=10; cnt++)
        {
			bRet=Get_SD2068_RTC(&RtcTime);
			//Check if this is a valid time. If yes, break the for loop.
			if ( (bRet == TRUE) && (IsTimeValid(&RtcTime) == TRUE) ) break;

			//If it is not a valid time, read RTC again.
			DEBUG_I2C("Called Get_SD2068_RTC() the %dth time. \r\n",cnt);
		}
        DEBUG_I2C("#1 RTC_SD2068 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (RtcTime.year + 2000), RtcTime.month, RtcTime.day, RtcTime.hour, RtcTime.min, RtcTime.sec);
        if(cnt >= 10 || (RtcTime.month == 0) || (RtcTime.day == 0))
        {
            bRet=Set_SD2068_RTC(&rtcBase);
        }
	}
	else
	{
        bRet=Set_SD2068_RTC(&rtcBase);
    }
    DEBUG_I2C("#2 RTC_SD2068 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (RtcTime.year + 2000), RtcTime.month, RtcTime.day, RtcTime.hour, RtcTime.min, RtcTime.sec);
    RTCTime_Gmt_To_Local(&rtcBase, &g_LocalTime);
    DEBUG_I2C("g_LocalTime:    20%02d/%02d/%02d %02d:%02d:%02d\n", g_LocalTime.year, g_LocalTime.month, g_LocalTime.day, g_LocalTime.hour, g_LocalTime.min, g_LocalTime.sec);
}
#endif
#if((USE_BUILD_IN_RTC == 0) && (EXT_RTC_SEL == RTC_ISL1208))

s32 i2cRead_ISL1208(u8 addr, u8* pData)
{
#if 1
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_ISL1208 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_RTC_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pData = I2cData;
	//DEBUG_I2C("read ISL1208 i2c addr=%d,  i2c data=%x\n",addr ,*pData);
    OSSemPost(i2cSemReq);
	return 1;

#else
	u8  err;
	u32 data, i;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		/*CY 0907 */
		/* signal event semaphore */
		OSSemPost(i2cSemReq);

		DEBUG_I2C("Error: i2cRead_TVP5150 i2cSemReq is %d.\n", err);
		return 0;
	}

    gpio_IIC_Enable();

    data    = 0;

	// Start
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);
    gpio_IIC_SDA_W(0);
	gpio_IIC_SCK_W(0);

	// I2C General address
	gpio_IIC_W_Byte(I2C_RTC_WR_SLAV_ADDR);

    // Acknowledge
	gpio_IIC_Ack_R();

	// I2C Write register address
	gpio_IIC_W_Byte(addr);

    // Acknowledge
	gpio_IIC_Ack_R();

	// delay max 64 us
	for(i = 0; i < 10; i++)
	{
	    gpio_IIC_SCK_W(0);
	}

	// Stop
    gpio_IIC_SDA_W(0);
    gpio_IIC_SCK_W(0);
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);

	// Start
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);
    gpio_IIC_SDA_W(0);
	gpio_IIC_SCK_W(0);

	// I2C General address
	gpio_IIC_W_Byte(I2C_RTC_RD_SLAV_ADDR);

    // Acknowledge
	gpio_IIC_Ack_R();

	// I2C read data
	gpio_IIC_R_Byte(&data);

    // Inverse Acknowledge
	gpio_IIC_nAck_W();

	// delay max 64 us
	for(i = 0; i < 10; i++)
	{
	    gpio_IIC_SCK_W(0);
	}

	// Stop
    gpio_IIC_SDA_W(0);
    gpio_IIC_SCK_W(0);
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);

    gpio_IIC_Disable();

	*pData = (u8)(data & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);

	OSSemPost(i2cSemReq);

	return 1;
#endif
}

s32 i2cWrite_ISL1208(u8 addr, u8 data)
{
#if 1
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_ISL1208_RTC i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (I2C_RTC_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;

#else
	u8  err;
	u32 i;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		/*CY 0907 */
		/* signal event semaphore */
		OSSemPost(i2cSemReq);

		DEBUG_I2C("Error: i2cWrite_ISL1208 i2cSemReq is %d.\n", err);
		return 0;
	}

    gpio_IIC_Enable();

	// Start
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);
    gpio_IIC_SDA_W(0);
	gpio_IIC_SCK_W(0);

	// I2C General address
	gpio_IIC_W_Byte(I2C_RTC_WR_SLAV_ADDR);

    // Acknowledge
	gpio_IIC_Ack_R();

	// I2C Write register address
	gpio_IIC_W_Byte(addr);

    // Acknowledge
	gpio_IIC_Ack_R();

	// I2C Write data
	gpio_IIC_W_Byte(data);

    // Acknowledge
	gpio_IIC_Ack_R();

	// delay max 64 us
	for(i = 0; i < 10; i++)
	{
	    gpio_IIC_SCK_W(0);
	}

	// Stop
    gpio_IIC_SDA_W(0);
    gpio_IIC_SCK_W(0);
    gpio_IIC_SCK_W(1);
    gpio_IIC_SDA_W(1);

    gpio_IIC_Disable();

	OSSemPost(i2cSemReq);

	return 1;
#endif
}

static BOOLEAN IsTimeValid(RTC_DATE_TIME* ptime)
{
  if ((ptime->year>99) || (ptime->year < 10))  return FALSE;

  if (ptime->month>12) return FALSE;

  if (ptime->day>32) return FALSE;

  if (ptime->hour>24) return FALSE;

  if (ptime->min>60) return FALSE;

  if (ptime->sec>60) return FALSE;

  return TRUE;
}

__inline u8 GetBCDtoDecimal(u8 data )
{
	return  ( (data>>4)&0xF) *10 + ( (data)&0xF);
}
__inline u8 ConvertDecimaltoBCD( u8 data )
{
	return  ((data/10)<<4) | (data %10 );
}

BOOLEAN Get_ISL1208_RTC(RTC_DATE_TIME *rtc_time)
{
	BOOLEAN bRet=TRUE;
	u8 data[6];
	u8 i;
	for(i=0;i<6;i++)
	{
		if(i2cRead_ISL1208(i, &data[i]) == 0)
		{
			bRet=FALSE;
			DEBUG_I2C("I2C Read 0x%02x Error\r\n", i);
		}
	}
	rtc_time->sec   = GetBCDtoDecimal(data[0]);
	rtc_time->min   = GetBCDtoDecimal(data[1]);
	rtc_time->hour  = GetBCDtoDecimal(data[2] & 0x3F);
	rtc_time->day   = GetBCDtoDecimal(data[3]);
	rtc_time->month = GetBCDtoDecimal(data[4]);
	rtc_time->year  = GetBCDtoDecimal(data[5]);
	//DEBUG_I2C("Date:%04d/%02d/%02d %02d:%02d:%02d\n",(rtc_time.year+2000), rtc_time.month, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
#if 0
    if(i2cRead_ISL1208(0x00, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.sec = GetBCDtoDecimal(data);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x00, rtc_time.sec);

    if(i2cRead_ISL1208(0x01, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.min = GetBCDtoDecimal(data);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x01, rtc_time.min);

    if(i2cRead_ISL1208(0x02, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.hour = GetBCDtoDecimal(data & 0x3F);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x02, rtc_time.hour);

    if(i2cRead_ISL1208(0x03, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.day = GetBCDtoDecimal(data);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x03, rtc_time.day);

    if(i2cRead_ISL1208(0x04, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.month = GetBCDtoDecimal(data);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x04, rtc_time.month);

    if(i2cRead_ISL1208(0x05, &data) == 0)
    {
    	bRet=FALSE;
    	return bRet;
    }
    rtc_time.year = GetBCDtoDecimal(data);
    DEBUG_I2C("I2C 0x%02x = %02d\n", 0x05, rtc_time.year);
#endif
    return bRet;
}

BOOLEAN Set_ISL1208_RTC(RTC_DATE_TIME* date)
{
	///u8 data;
	BOOLEAN bRet=TRUE;
	u8 data[6];
	u8 i;

	data[0] = ConvertDecimaltoBCD(date->sec);
	data[1] = ConvertDecimaltoBCD(date->min);
	data[2] = (ConvertDecimaltoBCD(date->hour)|0x80);  //Set to 24H Format
	data[3] = ConvertDecimaltoBCD(date->day);
	data[4] = ConvertDecimaltoBCD(date->month);
	data[5] = ConvertDecimaltoBCD(date->year);

	for(i=0;i<6;i++)
	{
		if(i2cWrite_ISL1208(i, data[i]) == 0)
		{
			bRet=FALSE;
			DEBUG_I2C("I2C Write 0x%02x Error\r\n", i);
		}
	}

#if 0
	data = ConvertDecimaltoBCD(date->sec);
    i2cWrite_ISL1208(0x00, data);
    //i2cRead_ISL1208(0x00, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x00, data);

	data = ConvertDecimaltoBCD(date->min);
    i2cWrite_ISL1208(0x01, data);
    //i2cRead_ISL1208(0x01, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x01, data);

	data = ConvertDecimaltoBCD(date->hour);
	data |= 0x80;
    i2cWrite_ISL1208(0x02, data);
    //i2cRead_ISL1208(0x02, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x02, data);

    data = ConvertDecimaltoBCD(date->day);
    i2cWrite_ISL1208(0x03, data);
    //i2cRead_ISL1208(0x03, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x03, data);

    data = ConvertDecimaltoBCD(date->month);
    i2cWrite_ISL1208(0x04, data);
    //i2cRead_ISL1208(0x04, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x04, data);

    data = ConvertDecimaltoBCD(date->year);
    i2cWrite_ISL1208(0x05, data);
    //i2cRead_ISL1208(0x05, &data);
    //DEBUG_I2C("I2C 0x%02x = %02d\n", 0x05, data);
#endif
    return bRet;
}

void ISL1208_RTC_Init(void)
{
    u8 status;
    u8 RTCValid;
    u8 cnt;
    BOOLEAN bRet=TRUE;
    RTC_DATE_TIME RtcTime;
    //i2cWrite_ISL1208(0x08, 0x0A);
    //i2cRead_ISL1208(0x08, &data);
    //DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x08, data);

    //i2cWrite_ISL1208(0x07, 0x10);
    //i2cRead_ISL1208(0x07, &status);
    //DEBUG_I2C("I2C 0x%02x = 0x%02x\n", 0x07, status);

	if(i2cRead_ISL1208(RTC_STATUS, &status) == 0)
	{
		DEBUG_I2C("Test ISL1208 RTC, failed to read\r\n");
		return;
	}
	DEBUG_I2C("RTC Status 0x%02x\n", status);

    if( status & 0x1 )
    {
        RTCValid=0;

    }else{
        RTCValid=1;
        DEBUG_I2C("RTC Valid\n\r");
    }

    if( !(status&STATUS_WRTC ) ){

        status=STATUS_WRTC |STATUS_ARST;

        if( i2cWrite_ISL1208(RTC_STATUS,status) == 0 ){

            DEBUG_I2C("Init RTC failed\r\n");
            RTCValid=0;
        }
    }

	if( RTCValid ==1 )
	{
		for (cnt=1; cnt<=10; cnt++)
        {
			bRet=Get_ISL1208_RTC(&RtcTime);
			//Check if this is a valid time. If yes, break the for loop.
			if ( (bRet==TRUE) && (IsTimeValid(&RtcTime)==TRUE) ) break;

			//If it is not a valid time, read RTC again.
			DEBUG_I2C("Called Get_ISL1208_RTC() the %dth time. \r\n",cnt);
		}
		if((RtcTime.month   == 0) ||
		   (RtcTime.day     == 0))
        {
 	        RtcTime.sec     = 0;
	        RtcTime.min     = 0;
	        RtcTime.hour    = 0;
	        RtcTime.day     = 1;
	        RtcTime.month   = 1;
	        RtcTime.year    = 9;
	        bRet=Set_ISL1208_RTC(&RtcTime);
        }
	}
	else
	{
        bRet=Set_ISL1208_RTC(&RtcTime);
    }
    DEBUG_I2C("RTC_ISL1208 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (RtcTime.year + 2000), RtcTime.month, RtcTime.day, RtcTime.hour, RtcTime.min, RtcTime.sec);
    RTCTime_Gmt_To_Local(&RtcTime, &g_LocalTime);
    DEBUG_I2C("g_LocalTime:    20%02d/%02d/%02d %02d:%02d:%02d\n", g_LocalTime.year, g_LocalTime.month, g_LocalTime.day, g_LocalTime.hour, g_LocalTime.min, g_LocalTime.sec);
}

#endif      // #if(USE_BUILD_IN_RTC == 0 && EXT_RTC_SEL == RTC_ISL1208)

#if(DOOR_BELL_SUPPORT)

s32 i2cWrite_433RFModule_MCU(u8 pucData)
{
/*pucData Value 
  4: Unpair
  5: SOS Piar 
  6: DoorBell Pair
  7: Leave Pair Mode 
  */

    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_PO3100K(0x%02x), i2cSemReq is %d.\n",pucData, err);
        return 0;
    }

//  DEBUG_I2C("I2C : %x  %x\n",addr,data);


    I2cData = ((u32)pucData);

    I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (I2C_433RFMod_W_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
    //I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cWrite_PO3100K(0x%02x), i2cSemFin is %d.\n", pucData, err);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
}

s32 i2cRead_433RFModule_MCU(u8* pucData)
{                                                                                                                                                                   
    u8 err;                                                                                                                                                         
    u32 i;                                                                                                                                                          
    u8 subReg=0;                                                                                                                                                    
    u8 subReg2=0;                                                                                                                                                   

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);                                                                                                                        
    if (err != OS_NO_ERR)                                                                                                                                           
    {                                                                                                                                                               
        DEBUG_I2C("Error: i2cRead_CBM7320_Manual(0x%02x), i2cSemReq is %d.\n", *pucData, err);                                                  
        return 0;                                                                                                                                                   
    }                                                                                                                                                               
    //DEBUG_YELLOW("Read val %d \n",*pucData);                                                                                                                                                                
    I2cCtrl     = I2C_MANUAL_EN  |I2C_1B| I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_480  | (I2C_433RFMod_R_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT);
                                                                                                                                                                    
    /* Start */                                                                                                                                                     
    I2cManu     = I2C_ManuSBIT;                                                                                                                                     
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);                                                                                
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))                                                                                                              
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Start 1 Error!!!\n");                                                                                            
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                                    
    /* Slave Addr */                                                                                                                                                
    I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;                                                                                                              
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);                                                                           
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))                                                                                                         
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", I2C_433RFMod_R_SLAV_ADDR);                                                   
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                               
    /* Read Data */                                                                                                                                                 
    I2cManu     =  I2C_ManuReadData;                                                                                                                                 
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);                                                                            
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))                                                                                                          
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Read Data Error!!!\n");                                                                                          
		return 0;                                                                                                                                                       
	}
    *pucData  = (u8)I2cData;                                                                                                                                                     
                                                                                                                                                                    
    /* Stop */                                                                                                                                                      
    I2cManu     = I2C_ManuPBIT;                                                                                                                                     
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);                                                                                                                                                                                                                   
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))                                                                                                              
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
		DEBUG_I2C("Error: i2cRead_CBM7320_Manual() IIC Stop Error!!!\n");                                                                                               
		return 0;                                                                                                                                                       
	}                                                                                                                                                                 
    else                                                                                                                                                            
	{                                                                                                                                                                 
		OSSemPost(i2cSemReq);                                                                                                                                           
        //DEBUG_GREEN("pucData %x \n", *pucData);
		return 1;                                                                                                                                                       
	}                                                                                                                                                                 
                                                                                                                                                                    
}

#endif


#if(EXT_RTC_SEL == RTC_BQ32000)

s32 i2cWrite_BQ32000(u8 addr, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BQ32000 i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_60 | (I2C_BQ32000_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_BQ32000_Word(u8 addr, u32 data, u8 ByteLength)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_BQ32000_Word i2cSemReq is %d.\n", err);
		return 0;
	}

//	DEBUG_I2C("I2C : %x  %x\n",addr,data);


	I2cData = ((u32)data);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | ((ByteLength - 1) << 4) | I2C_CLK_DIV_240 | (I2C_BQ32000_WR_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_BQ32000_Word i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_BQ32000(u8 addr, u8* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BQ32000 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_240 | (I2C_BQ32000_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BQ32000 i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read BQ32000 i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cRead_BQ32000_Word(u8 addr, u32* pData, u8 ByteLength)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_BQ32000_Word i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | ((ByteLength - 1) << 4) | I2C_CLK_DIV_240 | (I2C_BQ32000_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_BQ32000_Word i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u32)I2cData;
	//DEBUG_I2C("read BQ32000 i2c addr = 0x%02x, i2c data = 0x%08x\n", addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

void i2cTest_RTC_BQ32000(void)
{
    u32 wdata;
    u8  second, minute, hour, day, date, month, year;

    //DEBUG_I2C("IIC write BQ32000 testing...\n");
    //i2cWrite_BQ32000_Word(0x00, 0x00201506, 4);
    //i2cWrite_BQ32000_Word(0x04, 0x28051000, 4);

    DEBUG_I2C("IIC read BQ32000 testing...\n");
    /*
    for(i = 0; i <= 0xff; i++) {
        i2cRead_Byte(I2C_BQ32000_RD_SLAV_ADDR, (u8)i, &data);
        DEBUG_I2C("read BQ32000 i2c addr = 0x%02x, i2c data = 0x%02x\n", i, data);
    }
    */

    while(1)
    {
        /*
        i2cRead_BQ32000_Word(0x00, &wdata, 4);
        day     = wdata & 0x07;
        hour    = ((wdata >> 12) & 0x03) * 10 + ((wdata >>  8) & 0x0f);
        minute  = ((wdata >> 20) & 0x07) * 10 + ((wdata >> 16) & 0x0f);
        second  = ((wdata >> 28) & 0x07) * 10 + ((wdata >> 24) & 0x0f);
        i2cRead_BQ32000_Word(0x04, &wdata, 4);
        year    = ((wdata >> 12) & 0x0f) * 10 + ((wdata >>  8) & 0x0f);
        month   = ((wdata >> 20) & 0x01) * 10 + ((wdata >> 16) & 0x0f);
        date    = ((wdata >> 28) & 0x03) * 10 + ((wdata >> 24) & 0x0f);
        DEBUG_I2C("0x%08x 20%02d/%02d/%02d D%d %02d:%02d:%02d\r", wdata, year, month, date, day, hour, minute, second);
        */
        i2cRead_BQ32000_Word(0x00, &wdata, 3);
        hour    = ((wdata >>  4) & 0x03) * 10 + ((wdata >>  0) & 0x0f);
        minute  = ((wdata >> 12) & 0x07) * 10 + ((wdata >>  8) & 0x0f);
        second  = ((wdata >> 20) & 0x07) * 10 + ((wdata >> 16) & 0x0f);
        i2cRead_BQ32000_Word(0x04, &wdata, 3);
        year    = ((wdata >>  4) & 0x0f) * 10 + ((wdata >>  0) & 0x0f);
        month   = ((wdata >> 12) & 0x01) * 10 + ((wdata >>  8) & 0x0f);
        date    = ((wdata >> 20) & 0x03) * 10 + ((wdata >> 16) & 0x0f);
        DEBUG_I2C("0x%08x 20%02d/%02d/%02d D%d %02d:%02d:%02d\r", wdata, year, month, date, day, hour, minute, second);

        //i2cRead_BQ32000(0x00, &data);
        //DEBUG_I2C("%d\n", (data >> 4) * 10 + (data & 0x0f));
    }

}

#endif  // #if(EXT_RTC_SEL == RTC_BQ32000)

// touch key
s32 i2cRead_PIC16F1516(u8 id, u8* pData)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_PIC16F1516(0x%02x, 0x%02x), i2cSemReq is %d.\n", id, *pData, err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_960 | (((u32)I2C_PIC16F1516_RD_SLAV_ADDR) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
    I2cCtrl = I2cCtrl | (((u32)id) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", id, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    return 1;
}

s32 i2cWrite_PIC16F1516(u8 id,u8 data)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_Byte(0x%02x, 0x%02x), i2cSemReq is %d.\n", id, data, err);
        return 0;
    }

//  DEBUG_I2C("I2C : %x  %x\n",addr,data);


    I2cData = ((u32)data);

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_1920 | (((u32)I2C_PIC16F1516_WR_SLAV_ADDR) << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
    I2cCtrl = I2cCtrl | (((u32)id) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cWrite_Byte(0x%02x, 0x%02x), i2cSemFin is %d.\n", id, data, err);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
}


#if(TV_ENCODER == CH7025)
s32 i2cRead_CH7025(u8 ucRegAddr, u8* pucData)
{
	u8 err;


	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_CH7025 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_7680 | (I2C_CH7025_RD_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_CH7025(), i2cSemFin is %d.\n", err);
		return 0;
	}

#if 0
	if (I2cCtrl & I2C_NACK)
          return 0;

#endif

	*pucData = (u8)(I2cData & 0xff);
	//DEBUG_I2C("read LIS302DL i2c addr = 0x%02x, i2c data = 0x%02x\n", addr, *pData);
	//DEBUG_I2C("I2C read slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_CH7025_RD_SLAV_ADDR, addr, *pData);
    OSSemPost(i2cSemReq);
	return 1;
}

s32 i2cWrite_CH7025(u8 ucRegAddr, u8 pucData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_CH7025 i2cSemReq is %d.\n", err);
		return 0;
	}


//	DEBUG_I2C("I2C : %x  %x\n",addr,data);
//   DEBUG_I2C("I2C write slave_addr=0x%02x addr=0x%02x data=0x%02x\n", I2C_CH7025_WR_SLAV_ADDR, addr, data);


	I2cData = ((u32)pucData);

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_7680 | (I2C_CH7025_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA;
	I2cCtrl = I2cCtrl | (((u32)ucRegAddr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_CH7025(), i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}


void i2cInit_CH7025(void)
{
	#if 1 //VGA800x600

    i2cWrite_CH7025( 0x02, 0x01 );
	i2cWrite_CH7025( 0x02, 0x03 );  //reset
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x04, 0x39 );
	i2cWrite_CH7025( 0x08, 0x08 );
    i2cWrite_CH7025( 0x07, 0x18 );
	i2cWrite_CH7025( 0x09, 0x40 );
	i2cWrite_CH7025( 0x0D, 0x08 );
	i2cWrite_CH7025( 0x0F, 0x1B );
	i2cWrite_CH7025( 0x10, 0x20 );
	i2cWrite_CH7025( 0x11, 0x98 );
	i2cWrite_CH7025( 0x12, 0x40 );
    i2cWrite_CH7025( 0x13, 0x44 );
	i2cWrite_CH7025( 0x15, 0x11 );
	i2cWrite_CH7025( 0x16, 0xE0 );
	i2cWrite_CH7025( 0x17, 0x0D );  //Input timing Register 9
    i2cWrite_CH7025( 0x19, 0x0C );
	i2cWrite_CH7025( 0x1B, 0x23 );
	i2cWrite_CH7025( 0x1C, 0x20 );
	i2cWrite_CH7025( 0x1D, 0x20 );
	i2cWrite_CH7025( 0x1F, 0x28 );
	i2cWrite_CH7025( 0x20, 0x80 );
	i2cWrite_CH7025( 0x21, 0x52 );  //Output timing Register 7
	i2cWrite_CH7025( 0x22, 0x58 );
	i2cWrite_CH7025( 0x23, 0x74 );
	i2cWrite_CH7025( 0x25, 0x01 );
	i2cWrite_CH7025( 0x26, 0x04 );
    i2cWrite_CH7025( 0x30, 0x2F );  //Contrast adjustment
    i2cWrite_CH7025( 0x31, 0x96 );  //Brightness adjustment
    i2cWrite_CH7025( 0x32, 0xB7 );  //Text enhancement
	i2cWrite_CH7025( 0x37, 0x28 );  //AX + B adjustment Register 1
	i2cWrite_CH7025( 0x39, 0x29 );  //AX + B adjustment Register 3
	i2cWrite_CH7025( 0x3B, 0x27 );  //AX + B adjustment Register 5
	i2cWrite_CH7025( 0x3D, 0xBD );  //Filter setting Register 1
	i2cWrite_CH7025( 0x3E, 0xA3 );  //Filter setting Register 2
	i2cWrite_CH7025( 0x40, 0x28 );  //Burst setting Register
	i2cWrite_CH7025( 0x4D, 0x02 );
	i2cWrite_CH7025( 0x4E, 0xC7 );
	i2cWrite_CH7025( 0x4F, 0x6D );
	i2cWrite_CH7025( 0x50, 0xD1 );
	i2cWrite_CH7025( 0x51, 0x59 );
	i2cWrite_CH7025( 0x52, 0x12 );
	i2cWrite_CH7025( 0x53, 0x13 );
	i2cWrite_CH7025( 0x55, 0xE5 );
    i2cWrite_CH7025( 0x58, 0x80 );  //MV Control Register 2
	i2cWrite_CH7025( 0x5E, 0x80 );
	i2cWrite_CH7025( 0x69, 0x7E );  //MV Control Register 19
	i2cWrite_CH7025( 0x70, 0x15 );  //MV Control Register 25
	i2cWrite_CH7025( 0x71, 0x15 );  //MV Control Register 26

	i2cWrite_CH7025( 0x7D, 0x62 );
	i2cWrite_CH7025( 0x04, 0x38 );
	i2cWrite_CH7025( 0x06, 0x71 );

    /*
    NOTE:
    The following five repeated sentences are used here to
    wait memory initial complete,

    please don't remove...

    (you could refer to Appendix A of programming guide document
    (CH7025(26)B Programming Guide Rev2.03.pdf or later version)
    for detailed information about memory initialization!
    */
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x03, 0x00 );
	i2cWrite_CH7025( 0x06, 0x70 );
	i2cWrite_CH7025( 0x02, 0x02 );
	i2cWrite_CH7025( 0x02, 0x03 );
	i2cWrite_CH7025( 0x04, 0x00 );

	#else  //CVBS

    i2cWrite_CH7025( 0x02, 0x01 );
    i2cWrite_CH7025( 0x02, 0x03 );
    i2cWrite_CH7025( 0x03, 0x00 );
    i2cWrite_CH7025( 0x04, 0x39 );
    i2cWrite_CH7025( 0x0F, 0x1B );
    i2cWrite_CH7025( 0x10, 0x20 );
    i2cWrite_CH7025( 0x11, 0x98 );
    i2cWrite_CH7025( 0x12, 0x40 );
    i2cWrite_CH7025( 0x15, 0x11 );
    i2cWrite_CH7025( 0x16, 0xE0 );
    i2cWrite_CH7025( 0x17, 0x0D );
    i2cWrite_CH7025( 0x1C, 0xA0 );
    i2cWrite_CH7025( 0x4D, 0x03 );
    i2cWrite_CH7025( 0x4E, 0xC5 );
    i2cWrite_CH7025( 0x4F, 0x7F );
    i2cWrite_CH7025( 0x50, 0x7B );
    i2cWrite_CH7025( 0x51, 0x59 );
    i2cWrite_CH7025( 0x52, 0x12 );
    i2cWrite_CH7025( 0x53, 0x1B );
    i2cWrite_CH7025( 0x55, 0xE5 );
    i2cWrite_CH7025( 0x5E, 0x80 );
    i2cWrite_CH7025( 0x69, 0x64 );
    i2cWrite_CH7025( 0x7D, 0x62 );
    i2cWrite_CH7025( 0x04, 0x38 );
    i2cWrite_CH7025( 0x06, 0x71 );

    /*
    NOTE: The following five repeated sentences are used here to wait memory initial complete, please don't remove...(you could refer to Appendix A of programming guide document (CH7025(26)B Programming Guide Rev2.03.pdf or later version) for detailed information about memory initialization!
    */
    i2cWrite_CH7025( 0x03, 0x00 );
    i2cWrite_CH7025( 0x03, 0x00 );
    i2cWrite_CH7025( 0x03, 0x00 );
    i2cWrite_CH7025( 0x03, 0x00 );
    i2cWrite_CH7025( 0x03, 0x00 );

    i2cWrite_CH7025( 0x06, 0x70 );
    i2cWrite_CH7025( 0x02, 0x02 );
    i2cWrite_CH7025( 0x02, 0x03 );
    i2cWrite_CH7025( 0x04, 0x00 );
	#endif
}

#endif

s32 i2cWrite_Byte(u8 id, u8 addr, u8 data)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemReq is %d.\n", id, addr, data, err);
        return 0;
    }

//  DEBUG_I2C("I2C : %x  %x\n",addr,data);


    I2cData = ((u32)data);

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (((u32)id) << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cWrite_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemFin is %d.\n", id, addr, data, err);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
}


s32 i2cWrite_Data(u8 id, u8 addr, u8 bytes, u32 data)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cWrite_Data(0x%02x, 0x%02x, 0x%02x, 0x%08x), i2cSemReq is %d.\n", id, addr, bytes, data, err);
        return 0;
    }

//  DEBUG_I2C("I2C : %x  %x\n",addr,data);


    I2cData = ((u32)data);

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | ((bytes - 1) << 4) | I2C_CLK_DIV_480 | (((u32)id) << I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA | I2C_SENS_CLK_DIV_10;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl = I2C_TRIG | I2cCtrl;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cWrite_Data(0x%02x, 0x%02x, 0x%02x, 0x%08x), i2cSemFin is %d.\n", id, addr, bytes, data, err);
        return 0;
    }

    OSSemPost(i2cSemReq);
    return 1;
}

s32 i2cRead_Byte(u8 id, u8 addr, u8* pData)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemReq is %d.\n", id, addr, *pData, err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_480 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_Byte(0x%02x, 0x%02x, 0x%02x), i2cSemFin is %d.\n", id, addr, *pData, err);
        return 0;
    }

    *pData = (u8)(I2cData & 0xff);
    OSSemPost(i2cSemReq);
    return 1;
}

s32 i2cRead_Data(u8 id, u8 addr, u8 bytes, void* pData)
{
    u8 err;

    //---First step---//
    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_Data(0x%02x, 0x%02x, 0x%02x), i2cSemReq is %d.\n", id, addr, bytes, err);
        return 0;
    }

    I2cData = ((u32)addr);
    I2cCtrl = I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | (((u32)id & ~1) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2;
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_Data(0x%02x, 0x%02x, 0x%02x), i2cSemFin is %d.\n", id, addr, bytes, err);
        return 0;
    }

    //---Second step---//
    I2cCtrl = I2C_ENA | ((bytes - 1) << 4) | I2C_CLK_DIV_480 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_2; // modify by BJ
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_Data(0x%02x, 0x%02x, 0x%02x), i2cSemFin is %d.\n", id, addr, bytes, err);
		return 0;
	}

    switch(bytes)
    {
    case 1:
        *(u8*)pData     = (u8)(I2cData & 0xff);
        break;
    case 2:
        *(u16*)pData    = (u16)(I2cData & 0xffff);
        break;
    case 3:
        *(u32*)pData    = (u32)(I2cData & 0xffffff);
        break;
    case 4:
        *(u32*)pData    = (u32)I2cData;
        break;
    }
    OSSemPost(i2cSemReq);
    return 1;
}

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))

s32 IIC_Read_2Word(u8 id, u8 addr, u32* pData)
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: IIC_Read_2Word(0x%02x, 0x%02x, 0x%02x), i2cSemReq is %d.\n", id, addr, *pData, err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_8B | I2C_CLK_DIV_240 | (((u32)id) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA | I2C_SENS_CLK_DIV_1;
    I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: IIC_Read_2Word(0x%02x, 0x%02x, 0x%08x, 0x%08x), i2cSemFin is %d.\n", id, addr, I2cData2, I2cData, err);
        return 0;
    }

    pData[0]    = I2cData;
    pData[1]    = I2cData2;
    OSSemPost(i2cSemReq);
    return 1;
}

#endif

//------------------CT674 ----------------------------------//
#if 0//(LCM_OPTION == LCM_CT674_LF)

#if 1   // Use HW IIC

#define i2cWrite_Byte_CT674LF(a, b, c)  i2cWrite_Byte(a, b, c)
#define i2cRead_Byte_CT674LF(a, b, c)   i2cRead_Byte(a, b, c)

#else   // Use GPIO IIC

s32 i2cWrite_Byte_CT674LF(u8 id, u8 addr, u8 data)
{
    u32 i;

    // Start
    //DEBUG_I2C("Start\n");
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);

    // I2C General address
    //DEBUG_I2C("I2C General address\n");
    gpio_IIC_W_Byte_CT674LF(id);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // I2C Write register address
    //DEBUG_I2C("I2C Write register address\n");
    gpio_IIC_W_Byte_CT674LF(addr);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // I2C Write data
    //DEBUG_I2C("I2C Write data\n");
    gpio_IIC_W_Byte_CT674LF(data);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // delay max 64 us
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(0);
    }

    // Stop
    //DEBUG_I2C("Stop\n");
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);

    return 1;
}

s32 i2cRead_Byte_CT674LF(u8 id, u8 addr, u8* pData)
{
    u32 data, i;

    data    = 0;

    // Start
    //DEBUG_I2C("Start\n");
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);

    // I2C General address
    //DEBUG_I2C("I2C General address\n");
    gpio_IIC_W_Byte_CT674LF(id);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // I2C Write register address
    //DEBUG_I2C("I2C Write register address\n");
    gpio_IIC_W_Byte_CT674LF(addr);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // delay max 64 us
    //DEBUG_I2C("delay max 64 us\n");
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W_CT674LF(0);
    }

    // Stop
    //DEBUG_I2C("Stop\n");
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);

    // Start
    //DEBUG_I2C("Start\n");
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);

    // I2C General address
    //DEBUG_I2C("I2C General address\n");
    gpio_IIC_W_Byte_CT674LF(id);

    // Acknowledge
    //DEBUG_I2C("Acknowledge\n");
    gpio_IIC_Ack_CT674LF();

    // I2C read data
    //DEBUG_I2C("I2C read data\n");
    gpio_IIC_R_Byte_CT674LF(&data);

    // Inverse Acknowledge
    //DEBUG_I2C("Inverse Acknowledge\n");
    gpio_IIC_nAck_CT674LF();

    // delay max 64 us
    //DEBUG_I2C("delay max 64 us\n");
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(0);
    }

    // Stop
    //DEBUG_I2C("Stop\n");
    gpio_IIC_SDA_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(0);
    gpio_IIC_SCK_W_CT674LF(1);
    gpio_IIC_SDA_W_CT674LF(1);

    return 1;
}

#endif  // Use GPIO IIC

void i2cInit_CT674LF(void)
{
    u32 i;
    u8  data;
    u8  CCIR656 = 0;    // 1: 656, 0: 601

    //--------------------------------------------------------------
    //  CT675 I2C SETTING
    //  Input Source: CVBS NTSC
    //--------------------------------------------------------------
    //  0x40 : MVD
    //  0x42 : ADC
    //  0xF4 : Scaler
    //  0xF6 : TCON
    //--------------------------------------------------------------

    DEBUG_I2C("i2cInit_CT674LF()\n");
    i2cWrite_Byte_CT674LF(0xF4, 0x0A, 0x40);

    i2cWrite_Byte_CT674LF(0xF4, 0x0B, 0x00);
    i2cWrite_Byte_CT674LF(0xF4, 0x0C, 0x72);
    // ****** Software Reset ******
    i2cWrite_Byte_CT674LF(0xF4, 0x08, 0x05);
    i2cWrite_Byte_CT674LF(0xF4, 0x08, 0x00);

    // AV Input
    if(CCIR656)
        i2cWrite_Byte_CT674LF(0xF4, 0x01, 0x69);   //CCIR 656
    else
        i2cWrite_Byte_CT674LF(0xF4, 0x01, 0x49);   // 01:Input Ctrl, bit7:Dual ADC mode

    //i2cRead_Byte_CT674LF(0xf5, 0x01, &data);
    //DEBUG_I2C("IIC data=0x%x\n",data);

    // ****** ADC ******
    CT674_42();

    // ****** Scaler ******
    // *** Input window
    CT674_F4();

    // DC-DC  1  2  3  1  2  3
    // ****** Timing controller ******     //?????
    CT674_F6();

#if (EnGamma)
    // Gamma Table Initial
    i2cWrite_Byte_CT674LF(0xF4, 0x80, 0x00);    // Define address
    i2cWrite_Byte_CT674LF(0xF4, 0x05, 0x40); // Reset for Gamma Table
    i2cWrite_Byte_CT674LF(0xF4, 0x05, 0x00);

    // Load Gammma Table
    CT674_LoadGammaTable();

    // Enable Gamma correction
    i2cWrite_Byte_CT674LF(0xF4, 0x05, 0x80);

#endif

#if 0
   #define num (sizeof(CT674_ID_42) / sizeof(CT674_ID_42[0]))
        for(i = 0; i < num; i++)
        {
            i2cRead_Byte_CT674LF(0x43, CT674_ID_42[i].Addr, &data);
            if(data != CT674_ID_42[i].Data)
                DEBUG_I2C("(0x%02x, 0x%02x) = 0x%02x != 0x%02x\n", 0x43, (u32)CT674_ID_42[i].Addr, data, (u32)CT674_ID_42[i].Data);
        }
   #undef num
   #define num (sizeof(CT674_ID_F4) / sizeof(CT674_ID_F4[0]))
        for(i = 0; i < num; i++)
        {
            i2cRead_Byte_CT674LF(0xf5, CT674_ID_F4[i].Addr, &data);
            if(data != CT674_ID_F4[i].Data)
                DEBUG_I2C("(0x%02x, 0x%02x) = 0x%02x != 0x%02x\n", 0xf5, (u32)CT674_ID_F4[i].Addr, data, (u32)CT674_ID_F4[i].Data);
        }
   #undef num
   #define num (sizeof(CT674_ID_F6) / sizeof(CT674_ID_F6[0]))
        for(i = 0; i < num; i++)
        {
            i2cRead_Byte_CT674LF(0xf7, CT674_ID_F6[i].Addr, &data);
            if(data != CT674_ID_F6[i].Data)
                DEBUG_I2C("(0x%02x, 0x%02x) = 0x%02x != 0x%02x\n", 0xf7, (u32)CT674_ID_F6[i].Addr, data, (u32)CT674_ID_F6[i].Data);
        }
   #undef num
#endif

}

void CT674_LoadGammaTable(void)
{

    WORD j;

    for(j = 0; j < 256; j++)
    {
        i2cWrite_Byte_CT674LF(0xF4, 0x81, GammaTable[j * 3]);
        i2cWrite_Byte_CT674LF(0xF4, 0x82, GammaTable[(j * 3) + 1]);
        i2cWrite_Byte_CT674LF(0xF4, 0x83, GammaTable[(j * 3) + 2]);
        /*
        i2cWrite_Byte_CT674LF(0xF4, 0x81, j);
        i2cWrite_Byte_CT674LF(0xF4, 0x82, j);
        i2cWrite_Byte_CT674LF(0xF4, 0x83, j);
        */
    }
}

void CT674_F4(void)
{
#define num (sizeof(CT674_ID_F4) / sizeof(CT674_ID_F4[0]))
    BYTE i;
    for(i=0; i<num; i++){
        i2cWrite_Byte_CT674LF(0xF4, CT674_ID_F4[i].Addr, CT674_ID_F4[i].Data);
    }
#undef num
}

void CT674_42(void)
{
#define num (sizeof(CT674_ID_42) / sizeof(CT674_ID_42[0]))
    BYTE i;
    for(i=0; i<num; i++){
        i2cWrite_Byte_CT674LF(0x42, CT674_ID_42[i].Addr, CT674_ID_42[i].Data);
    }
#undef num

}

void CT674_F6(void)
{
#define num (sizeof(CT674_ID_F6) / sizeof(CT674_ID_F6[0]))
    BYTE i;
    for(i=0; i<num; i++){
        i2cWrite_Byte_CT674LF(0xF6, CT674_ID_F6[i].Addr, CT674_ID_F6[i].Data);
    }
#undef num
}

void CT674_LCD_PANEL_On(void)
{
    DEBUG_I2C("CT674_LCD_PANEL_On()\n");
    i2cWrite_Byte_CT674LF(0xF4, 0x07, 0x08);
}

void CT674_LCD_PANEL_Off(void)
{
    DEBUG_I2C("CT674_LCD_PANEL_Off()\n");
    i2cWrite_Byte_CT674LF(0xF4, 0x07, 0x7F);
}

#endif  // #if (LCM_OPTION == LCM_CT674_LF)


#if( (Sensor_OPTION == Sensor_MI9V136_YUV601) || (TV_DECODER == MI9V136) )

s32 i2cRead_R16D16_Manual(u8 id, u16 addr, u16* pData)
{
	u8  err, idr;
	u32 i;
	u8  *pucData, *pAddr;

	pucData = (u8*)pData;
	idr     = id + 1;       // slave address for read
	pAddr   = (u8*)&addr;
	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() i2cSemReq is %d.\n", err);
		return 0;
	}

	// write register address for read two word data
	//I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_240 | (id << I2C_SLAV_ADDR_SHFT) | (((u32)pAddr[1]) << I2C_SUB_ADDR_SHFT);
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_480 | I2C_SENS_CLK_DIV_10 | (id << I2C_SLAV_ADDR_SHFT) | (((u32)pAddr[1]) << I2C_SUB_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Start 1 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", id);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSubaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Write Sub Addr 0x%04x high byte Error!!!\n", addr);
		return 0;
	}
	//I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_240 | (id << I2C_SLAV_ADDR_SHFT) | (((u32)pAddr[0]) << I2C_SUB_ADDR_SHFT);
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_CLK_DIV_480 | I2C_SENS_CLK_DIV_10 | (id << I2C_SLAV_ADDR_SHFT) | (((u32)pAddr[0]) << I2C_SUB_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSubaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSubaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Write Sub Addr 0x%04x low byte Error!!!\n", addr);
		return 0;
	}

    // Read 2 bytes data
	//I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_240 | (idr << I2C_SLAV_ADDR_SHFT);
	I2cCtrl     = I2C_MANUAL_EN | I2C_R_ACK | I2C_ENA | I2C_1B | I2C_CLK_DIV_480 | I2C_SENS_CLK_DIV_10 | (idr << I2C_SLAV_ADDR_SHFT);
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Start 2 Error!!!\n");
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuSlaveaddr;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuSlaveaddr))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Write Slave Addr 0x%02x Error!!!\n", idr);
		return 0;
	}
	I2cManu     = I2C_ManuAck2Bus | I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Read Register 0x%04x high byte Error!!!\n", addr);
		return 0;
	}
	pucData[1]  = (u8)I2cData;
	I2cManu     = I2C_ManuReadData;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuReadData))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Read Register 0x%04x low byte Error!!!\n", addr);
		return 0;
	}
	pucData[0]  = (u8)I2cData;
	I2cManu     = I2C_ManuPBIT;
	for(i = 0; (i < 1000) && ((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT)); i++);
	if((I2cCtrl & I2C_BUSY) || (I2cManu & I2C_ManuPBIT))
	{
		OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_R16D16_Manual() IIC Stop Error!!!\n");
		return 0;
	}
	else
	{
		OSSemPost(i2cSemReq);
		return 1;
	}

}

#define I2C_CLK_DIV_SETTING         I2C_CLK_DIV_480
#define I2C_SENSCLK_DIV_SETTING     I2C_SENS_CLK_DIV_10

void HW_I2C_Word_addr_Write_Word(u8 dev_addr,u16 addr, u16 write_data)
{
 dev_addr&=0xfe;
 I2cCtrl=I2C_ENA|I2C_4B|I2C_CLK_DIV_SETTING|I2C_SENSCLK_DIV_SETTING|((u32)dev_addr<<8);
 I2cData=((u32)addr<<16)|((u32)write_data);
 I2cCtrl=I2C_ENA|I2C_4B|I2C_CLK_DIV_SETTING|I2C_SENSCLK_DIV_SETTING|((u32)dev_addr<<8)|I2C_TRIG;
 while ((I2cCtrl&I2C_BUSY));
}

u16 HW_I2C_Word_addr_Read_Word(u8 dev_addr,u16 addr)
{
 dev_addr&=0xfe;
 I2cCtrl=I2C_ENA|I2C_2B|I2C_CLK_DIV_SETTING|I2C_SENSCLK_DIV_SETTING|((u32)dev_addr<<8);
 I2cData=(u32)addr;
 I2cCtrl=I2C_ENA|I2C_2B|I2C_CLK_DIV_SETTING|I2C_SENSCLK_DIV_SETTING|((u32)dev_addr<<8)|I2C_TRIG;
 while ((I2cCtrl&I2C_BUSY));
 dev_addr|=1;
 I2cCtrl=I2C_ENA|I2C_2B|I2C_CLK_DIV_SETTING|I2C_SENSCLK_DIV_SETTING|((u32)dev_addr<<8)|I2C_TRIG;
 while ((I2cCtrl&I2C_BUSY));
 return (I2cData&0xffff);
}


#endif

//-----------------------------------------------------------------------------
// Common Read/Write Functions
//-----------------------------------------------------------------------------




#if (TOUCH_PANEL ==  TOUCH_PANEL_DRIVER_TSC2003)

/** Reads bytes from a slave I2C device.
 * @param slave address including the R/nW bit
 * @param byte the pointer to store the read bytes
 * @param n the number of the bytes
 * @retval true in the case of success to read the bytes;
 * @retval false otherwise
 */
bool I2C_TSC2003_read(u8 addr, u8 *byte, u8 n)
{
    u8 err;
    u32 data;
    int i;

    assert ((0 < n) && (n <= 8));
    assert ((addr & 0x01) == 1);    // check R/nW bit

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: I2C_read: i2cSemReq is %d.\n", err);
        return FALSE;
    }

    I2cCtrl =  _i2cBytesTbl[n] | I2C_CLK_DIV_960 | (addr<<I2C_SLAV_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG | I2C_ENA | I2C_INT_ENA;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR) 
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: I2C_read, i2cSemFin is %d.\n", err);
        return FALSE;
    }

    data = (u32)I2cData;

    for (i=n-1; i>=0; --i, data>>=8)
        byte[i] = data & 0xFF;
    OSSemPost(i2cSemReq);
    return TRUE;
}


/** Writes bytes to a slave I2C device.
 * @param slave address including the R/nW bit
 * @param byte the pointer to the bytes to write
 * @param n the number of bytes to write
 * @retval true in the case of success to write the bytes;
 * @retval false otherwise
 */
bool I2C_TSC2003_write(u8 addr, const u8 *byte, u8 n)
{
     u8 err;
     u32 data;
     int i;

    assert ((0 < n) && (n <= 8));
    assert ((addr & 0x01) == 0);    // check R/nW bit

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR) 
    {
        DEBUG_I2C("Error: I2C_write i2cSemReq is %d.\n", err);
        return 0;
    }

    for (i=0, data=0;;) {
        data |= byte[i];
        ++i;
        if (i == n)
            break;
        data <<= 8;
    }
    I2cData = (u32)data;


    I2cCtrl =  _i2cBytesTbl[n] | I2C_CLK_DIV_960 | (addr<<I2C_SLAV_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG | I2C_ENA | I2C_INT_ENA;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR) 
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: I2C_write(), i2cSemFin is %d.\n", err);
        return FALSE;
    }

    OSSemPost(i2cSemReq);
    return TRUE;
}


//-----------------------------------------------------------------------------
// TSC2003 -- a 4-wire I2C Touch Screen Controller
//-----------------------------------------------------------------------------


static bool TSC2003_read(ConfigureBits cf, PowerDownBits pd, ModeBit m,
                         int *val)
{
    u8 c;
    u8 d[2];
    bool ret;

    c = TSC2003_CMD(cf, pd, m);
    ret = I2C_TSC2003_write(I2C_TSC2003_WR_SLAV_ADDR, &c, 1);

    //udelay(20);   // needed only on high-speed mode
    ret = I2C_TSC2003_read(I2C_TSC2003_RD_SLAV_ADDR, d, m == M_12BIT ? 2 : 1);

    *val = d[0];
    *val <<= 4;
    if (m == M_12BIT)
        *val += (d[1] >> 4);

    return ret;
}


static bool TSC2003_readX(int *x)
{

    return TSC2003_read(C_MEAS_X, PD_IREFON_ADCOFF , M_12BIT, x);
}


static bool TSC2003_readY(int *y)
{

    return TSC2003_read(C_MEAS_Y, PD_IREFON_ADCOFF , M_12BIT, y);
}


static bool TSC2003_readZ1(int *z1)
{

    return TSC2003_read(C_MEAS_Z1, PD_IREFON_ADCOFF , M_12BIT, z1);
}


static bool TSC2003_readZ2(int *z2)
{

    return TSC2003_read(C_MEAS_Z2, PD_IREFON_ADCOFF , M_12BIT, z2);
}


/**Gets the physical position of the touch panel.
 * @retval true in the case of success to read the position;
 * @retval false otherwise.
 */
static bool TSC2003_getPosition(int *x, int *y)
{
    int z1, z2;
    int r_touch;
    bool ret;
    static bool hasRun = FALSE;
    u8 level=1;
    enum {
        // The threshold of the touch resistance.
        // The value is normalized with R_XPlate == TSC2003_ADC_MAX,
        // and is evaluted from actual measurement.
        #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
            R_th = 15000
        #else
            R_th = 8000
        #endif
    };
    if (!hasRun) {

        TSC2003_readX(x);
        hasRun = TRUE;
    }
    if (gpioGetLevel(GPIO_GROUP_TOUCH_PANNEL_INT,GPIO_PIN_TOUCH_PANNEL_INT, &level) == 1)
    {
        if(level == 1)
            return FALSE;
    }


    ret = TSC2003_readX(x);
    ret |= TSC2003_readY(y);

    // The default position on no any touching is (4095, 0).
    // The touchabled bound is about from (100, 160) to (3950, 3930)
    if ((*x > 4050) && (*y < 50))
        return FALSE;

    ret |= TSC2003_readZ1(&z1);
    ret |= TSC2003_readZ2(&z2);

    //assert (z2 >= z1);
    if (z2 < z1)
        return FALSE;

    if (z1 == 0) {
        ++z1;
        ++z2;
    }
    r_touch = (int)*x * z2/z1 - *x; // assume R_XPlate == TSC2003_ADC_MAX
    //printf("x:%d, y:%d, z1:%d, z2:%d, Rt:%d\n", *x, *y, z1, z2, r_touch);
    if (r_touch > uiTouchLevel) //if (r_touch > R_th)
        return FALSE;

    return ret;
}


#endif


#if(SUPPORT_TOUCH)
/**Gets the logical position of the touch panel.
 * @retval true in the case of success to read the position;
 * @retval false otherwise.
 */

#if(HW_BOARD_OPTION == MR8200_RX_COMMAX)

bool i2c_Touch_getPosition(int *x, int *y)
{
    bool ret;
    enum {
        DISP_WIDTH = 640,   // width of the display buffer
        DISP_HEIGHT = 480   // height of the display buffer
    };

#if (TOUCH_PANEL ==  TOUCH_PANEL_DRIVER_TSC2003)
    if (ret = TSC2003_getPosition(x, y)) {
        *x = (u32)DISP_WIDTH * *x/TSC2003_ADC_MAX;
        *y = (u32)DISP_HEIGHT * *y/TSC2003_ADC_MAX;
    }
#endif

    return ret;
}

#else

#ifndef min
#define min(a,b) (((a)<(b))? (a):(b))
#endif

#ifndef max
#define max(a,b) (((a)>(b))? (a):(b))
#endif

#if (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589)
enum
{
    PHY_MIN_X = 21,
    PHY_MIN_Y = 1,
    PHY_MAX_X = 786,
    PHY_MAX_Y = 479
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N)
enum
{
    PHY_MIN_X = 21,
    PHY_MIN_Y = 1,
    PHY_MAX_X = 786,
    PHY_MAX_Y = 479
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};

#elif (HW_BOARD_OPTION == MR8100_GCT_LCD)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
enum
{
    PHY_MIN_X = 150,
    PHY_MIN_Y = 300,
    PHY_MAX_X = 3920,
    PHY_MAX_Y = 3780
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M930)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M742)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M731)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8120_RX_RDI_M733)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M706)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M742_HA)
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 200,
    PHY_MAX_X = 4000,
    PHY_MAX_Y = 3800
};
#else
enum
{
    PHY_MIN_X = 100,
    PHY_MIN_Y = 400,
    PHY_MAX_X = 3800,
    PHY_MAX_Y = 3700
};
#endif


bool i2c_Touch_getPosition(int *x, int *y)
{
    bool ret;
#if (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
    static u32 u32LastTick = 0;
#endif
   // DEBUG_I2C("***** 1\n");
    enum {
    DISP_WIDTH = 640,   // width of the display buffer
    DISP_HEIGHT = 480,  // height of the display buffer

    };
    //DEBUG_I2C("***** 2\n");
#if (TOUCH_PANEL ==  TOUCH_PANEL_DRIVER_TSC2003)
    if ((ret = TSC2003_getPosition(x, y)) == true)
#elif (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
    if ((ret = CTP_getPosition(x, y)) == true)
#endif
    {

    #if (TOUCH_PANEL_DRIVER_CAPACITIVE == TOUCH_PANEL)
    if(*x == 900)
    {
        if(*y == 80)
        {
            if (OS_tickcounter - u32LastTick > 35 || (OS_tickcounter - u32LastTick) < 0)
            {
                u32LastTick = OS_tickcounter;
                uiSentKeyToUi(UI_KEY_MODE);
            }
        }
        else if(*y == 240)
            uiSentKeyToUi(UI_KEY_DOWN);
        else if(*y == 400)
            uiSentKeyToUi(UI_KEY_UP);
        return false;
    }
    #endif
         //DEBUG_I2C("***** 1\n");
        //*x = (int32_t)DISP_WIDTH * *x/TSC2003_ADC_MAX;
        //*y = (int32_t)DISP_HEIGHT * *y/TSC2003_ADC_MAX;
        *x = min(*x, PHY_MAX_X);
        *y = min(*y, PHY_MAX_Y);
        *x = max(*x, PHY_MIN_X);
        *y = max(*y, PHY_MIN_Y);
        *x =  (int)DISP_WIDTH * (*x - PHY_MIN_X + 1) /
                                    (PHY_MAX_X - PHY_MIN_X + 1);
        *y =  (int)DISP_HEIGHT * (*y - PHY_MIN_Y + 1) /
                                     (PHY_MAX_Y - PHY_MIN_Y + 1);
    }
   // DEBUG_I2C("***** 3\n");
    return ret;
}
#endif  //#if(HW_BOARD_OPTION == MR8200_RX_COMMAX)
#endif  //#if(SUPPORT_TOUCH)

#if (IO_EXPAND == IO_EXPAND_WT6853)
s32 i2cRead_WT6853(u8 reg , u8 *data )
{
    u8 err;

    OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_I2C("Error: i2cRead_WT6853(0x%02x, 0x%02x), i2cSemReq is %d.\n", reg, *data, err);
        return 0;
    }

    I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_960 | (((u32)I2C_WT6852_RD_SLAV_ADDR) << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA ;
    I2cCtrl = I2cCtrl | (((u32)reg) << I2C_SUB_ADDR_SHFT);
    I2cCtrl |= I2C_TRIG;

    OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        OSSemPost(i2cSemReq);
        DEBUG_I2C("Error: i2cRead_WT6853(0x%02x, 0x%02x), i2cSemFin is %d.\n",  reg, *data, err);
        return 0;
    }

    *data = (u8)(I2cData & 0xff);

    OSSemPost(i2cSemReq);
    return 1;

}

s32 i2cWrite_WT6853(u8 reg, u8 data)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cWrite_WT6853 i2cSemReq is %d.\n", err);
		return 0;
	}
	I2cData = ((u32)data);
	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_1B | I2C_CLK_DIV_3840 | (I2C_WT6853_WR_SLAV_ADDR<< I2C_SLAV_ADDR_SHFT) |I2C_INT_ENA ;
	I2cCtrl = I2cCtrl | (((u32)reg) << I2C_SUB_ADDR_SHFT);
	I2cCtrl = I2C_TRIG | I2cCtrl;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cWrite_WT6853(), i2cSemFin is %d.\n", err);
		return 0;
	}

    OSSemPost(i2cSemReq);
	return 1;
}


#endif

#if(THERMOMETER_SEL == THERMO_MLX90615)
s32 i2cRead_MLX90615(u8 addr, u16* pData)
{
	u8 err;

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: i2cRead_MLX90615 i2cSemReq is %d.\n", err);
		return 0;
	}

	I2cCtrl = I2C_ENA | I2C_RD_REG_ADDR | I2C_3B | I2C_CLK_DIV_1920 | (I2C_MLX90615_RD_SLAV_ADDR << I2C_SLAV_ADDR_SHFT) | I2C_INT_ENA;
	I2cCtrl = I2cCtrl | (((u32)addr) << I2C_SUB_ADDR_SHFT);
	I2cCtrl |= I2C_TRIG;

	OSSemPend(i2cSemFin, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
	    OSSemPost(i2cSemReq);
		DEBUG_I2C("Error: i2cRead_MLX90615 i2cSemFin is %d.\n", err);
		return 0;
	}

	*pData = (u16)(((I2cData & 0x00ff0000) >> 16) | (I2cData & 0x0000ff00));
	//DEBUG_I2C("read MLX90615 i2c addr = 0x%02x, i2c data = 0x%08x, T_reg = 0x%04x\n", addr, I2cData, *pData);
	OSSemPost(i2cSemReq);
 	return 1;

}

// 環境溫度
void i2cRead_MLX90615_TempA(f32* pData)
{
    u16 Data;

    i2cRead_MLX90615(0x26, &Data);
    *pData  = ((f32)Data) * 0.02 - 273.15;
    //*pData  = (Data << 4) / 50 - (273.15 * 16);
}

// 目標溫度
void i2cRead_MLX90615_TempO(f32* pData)
{
    u16 Data;

    i2cRead_MLX90615(0x27, &Data);
    *pData  = ((f32)Data) * 0.02 - 273.15;
    //*pData  = (Data << 4) / 50 - (273.15 * 16);
}

#endif  // #if(THERMOMETER_SEL == THERMO_MLX90615)
#if (Melody_SNC7232_ENA)
void Melody_SNC7232_R_Byte(u8 Addr,    u8  Data)
{   
    s32 i;
    u8  Value;
    u8  LDPC_cont=0;
    s32 j;
    u16 delay=6000;
    u8  tData;
    
    Value   = (Addr);
    Melody_SNC7232_SCK_W(0);
    Melody_SNC7232_SDA_DirO();
    // write command
    for(i = 3; i >= 0; i--)
    {
        if((Value >> i) & 1)
            LDPC_cont++;
        Melody_SNC7232_SCK_W(0);
        Melody_SNC7232_SDA_W((Value >> i) & 1);
        for(j = 0; j < delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < delay; j++);
    }

    // write data
    for(i = 4; i >= 0; i--)
    {
        if((Data >> i) & 1)
            LDPC_cont++;
        Melody_SNC7232_SCK_W(0);
        Melody_SNC7232_SDA_W((Data >> i) & 1);
        for(j = 0; j < delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < delay; j++);
    }

    // LDPC
    Melody_SNC7232_SCK_W(0);
    if((LDPC_cont%2)==0)
    {
        Melody_SNC7232_SDA_W(0);
    }
    else
    {
        Melody_SNC7232_SDA_W(1);
    }
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);
    for(j = 0; j < delay; j++);
    
        for(j = 0; j < delay; j++);
    // ACK
    Melody_SNC7232_SCK_W(0);
//    Melody_SNC7232_SDA_W(1);
    Melody_SNC7232_SDA_DirI();
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);
    Data=1;
    while(Data)
    {
        Melody_SNC7232_SDA_R(&Data);
        printf("%d ",Data);
    }
    for(j = 0; j < delay; j++);

    Melody_SNC7232_SCK_W(0);
    // read data
    Data=0;
    for(i = 4; i >= 0; i--)
    {
        Melody_SNC7232_SCK_W(0);
        Melody_SNC7232_SDA_R(&tData);
        Data |= tData << i ;
        if(tData)
            LDPC_cont++;
        for(j = 0; j < delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < delay; j++);
    }

    // read LDPC
    Melody_SNC7232_SCK_W(0);
    Melody_SNC7232_SDA_R(&tData);
    Data |= tData << 5 ;
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);
    for(j = 0; j < delay; j++);

    // ACK
    Melody_SNC7232_SCK_W(0);
//    Melody_SNC7232_SDA_W(1);
    Melody_SNC7232_SDA_DirI();
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);
    Data=1;
    while(Data)
    {
        Melody_SNC7232_SDA_R(&Data);
        printf("%d ",Data);
    }
    
    
}

s8 Melody_SNC7232_W_Byte(u8 Addr, u8 Data)
{
    s32 i;
    u8  Value;
    u8  LDPC_cont=0;
    s32 j;
    u8  ack;
    u8 retry=0;
    u8 err;
    
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
     unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
#if 1

	OSSemPend(i2cSemReq, I2C_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_I2C("Error: Melody_SNC7232_W_Byte is %d.\n", err);
		return ;
	}

    while(retry<5)
    {
        Value   = (Addr);
        OS_ENTER_CRITICAL();	
        Melody_SNC7232_SCK_W(0);
        Melody_SNC7232_SDA_DirO();
        // write command
        for(i = 3; i >= 0; i--)
        {
            if((Value >> i) & 1)
                LDPC_cont++;
            Melody_SNC7232_SCK_W(0);
            Melody_SNC7232_SDA_W((Value >> i) & 1);
            for(j = 0; j < SNC7232delay; j++);
            Melody_SNC7232_SCK_W(1);
            for(j = 0; j < SNC7232delay; j++);
        }

        // write data
        for(i = 4; i >= 0; i--)
        {
            if((Data >> i) & 1)
                LDPC_cont++;
            Melody_SNC7232_SCK_W(0);
            Melody_SNC7232_SDA_W((Data >> i) & 1);
            for(j = 0; j < SNC7232delay; j++);
            Melody_SNC7232_SCK_W(1);
            for(j = 0; j < SNC7232delay; j++);
        }

        // LDPC
        Melody_SNC7232_SCK_W(0);
        if((LDPC_cont%2)==0)
        {
            Melody_SNC7232_SDA_W(0);
        }
        else
        {
            Melody_SNC7232_SDA_W(1);
        }
        for(j = 0; j < SNC7232delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < SNC7232delay; j++);
        
        // ACK
//        for(j = 0; j < delay; j++); //delay CLK high
        Melody_SNC7232_SCK_W(0);
        Melody_SNC7232_SDA_DirI();
        for(j = 0; j < SNC7232delay; j++);
        Melody_SNC7232_SCK_W(1);
        OS_EXIT_CRITICAL();
        for(j = 0, ack = 1; (j < 3000) && ack; j++)
        {
            gpioGetLevel(1,18,&ack);
        }
        if(ack)
            retry++;
        else 
            break;
        
        DEBUG_I2C("####### rty=%d cmd:%x data:%x %d\n",retry,Addr, Data,SNC7232delay);
        OSTimeDly(1);

    }
    OSTimeDly(1);

	OSSemPost(i2cSemReq);
    if(ack)
    {
        DEBUG_I2C("Error: Melody_SNC7232_W_Byte() cmd:%x data:%x time out!!!\n",Addr, Data);
        return 0;
    }
    return 1;
#else   
    Value   = (Addr);
    Melody_SNC7232_SCK_W(0);
    Melody_SNC7232_SDA_DirO();
    // write command
    for(i = 3; i >= 0; i--)
    {
        if((Value >> i) & 1)
            LDPC_cont++;
        Melody_SNC7232_SDA_W((Value >> i) & 1);
        Melody_SNC7232_SCK_W(0);
        for(j = 0; j < delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < delay; j++);
    }

    // write data
    for(i = 4; i >= 0; i--)
    {
        if((Data >> i) & 1)
            LDPC_cont++;
        Melody_SNC7232_SDA_W((Data >> i) & 1);
        Melody_SNC7232_SCK_W(0);
        for(j = 0; j < delay; j++);
        Melody_SNC7232_SCK_W(1);
        for(j = 0; j < delay; j++);
    }

    // LDPC
    if((LDPC_cont%2)==0)
    {
        Melody_SNC7232_SDA_W(0);
    }
    else
    {
        Melody_SNC7232_SDA_W(1);
    }
    Melody_SNC7232_SCK_W(0);
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);
    for(j = 0; j < delay; j++);
    
    for(j = 0; j < delay; j++);
    // ACK
    Melody_SNC7232_SDA_W(1);
    Melody_SNC7232_SDA_DirI();
    Melody_SNC7232_SCK_W(0);
    for(j = 0; j < delay; j++);
    Melody_SNC7232_SCK_W(1);


#endif

        
//    for(j = 0; j < delay*10; j++);
}

s8 Melody_SNC7232_W2_Byte(u8 Addr1, u8 Data1,u8 Addr2, u8 Data2)
{
    u8 res;
    res = Melody_SNC7232_W_Byte(Addr1, Data1);
    if(!res) return 0;
    res = Melody_SNC7232_W_Byte(Addr2, Data2);
    if(!res) return 0;

}

void Melody_SNC7232_SETDELAY( u8 Data)
{
//    SNC7232delay = Data;
}
void Melody_SNC7232_init(void)
{
    u8 level;
//    DEBUG_I2C("Melody_SNC7232_init \n");

//    Melody_SNC7232_W_Byte(0x01,0x0A); //reset
//    gpioGetLevel(1,18,&level);
//    if(level==0)
//    {
//        Melody_SNC7232_W_Byte(0x02,0x01);
//        Melody_SNC7232_W_Byte(0x06,0x03);
//    }
}
void Melody_SNC7232_Stop(void)
{
    Melody_play=0;
    if(!(Melody_SNC7232_W_Byte(0x04,0x03)))
        Melody_retry=Melody_STOP;
    else
        Melody_retry=Melody_NORMAL;
}
void Melody_SNC7232_Play_All(void)
{
    s32 j;
    Melody_play=1;
    if(!(Melody_SNC7232_W2_Byte(0x03,0x01,0x04,0x01)))
    {
        Melody_retry=Melody_PLAYALL;
    }
    else
    {
        Melody_play=3;
        Melody_retry=Melody_NORMAL;
    }

    //    Melody_SNC7232_W_Byte(0x03,0x01);
    //    Melody_SNC7232_W_Byte(0x04,0x01);
}
void Melody_SNC7232_Play(u8 cmd)
{ 
    s32 j;
    static u8 test=4;
    
    if(cmd >4)
        cmd=1;
    Melody_play=1;
#if 1
    if(!(Melody_SNC7232_W2_Byte(0x03,cmd,0x04,0x01)))
    {
        Melody_retry=Melody_PLAY;
    }
    else
    {
        Melody_retry=Melody_NORMAL;
        Melody_play=2;
        Melody_play_num=cmd;
        test=2;
    }

#else //test rty
    if (test == 0)
    {
        if(!(Melody_SNC7232_W2_Byte(0x03,cmd,0x04,0x01)))
        {
            Melody_retry=Melody_PLAY;
        }
        else
        {
            Melody_retry=Melody_NORMAL;
            Melody_play=1;
            Melody_play_num=cmd;
            test=2;
        }
    }
    else
    {
        Melody_retry=Melody_PLAY;
        printf("###Melody_SNC7232_Play %d \n",test--);
    }
#endif
}

void Melody_SNC7232_Start()
{ 
    s32 j;
    
    Melody_play=1;
    if(!(Melody_SNC7232_W_Byte(0x04,0x01)))
    {
        Melody_retry=Melody_START;
    }
    else
    {
        Melody_retry=Melody_NORMAL;
        Melody_play=2;
    }

}

void Melody_SNC7232_Start_ALL()
{ 
    s32 j;
    
    Melody_play=1;
    if(!(Melody_SNC7232_W_Byte(0x04,0x01)))
    {
        Melody_retry=Melody_STARTALL;
    }
    else
    {
        Melody_retry=Melody_NORMAL;
        Melody_play=3;
    }

}

void Melody_SNC7232_PAUSE()
{ 
    s32 j;
    
    Melody_play=1;
    if(!(Melody_SNC7232_W_Byte(0x04,0x02)))
    {
        Melody_retry=Melody_PAUSE;
    }
    else
    {
        Melody_retry=Melody_NORMAL;
        Melody_play=4;
    }

}

void Melody_SNC7232_PlayNext(u8 cmd)
{
    cmd +=1;
    if(cmd >4)
        cmd=1;
    Melody_play=1;
    if(!(Melody_SNC7232_W2_Byte(0x03,cmd,0x04,0x01)))
    {
        Melody_retry=Melody_PLAYNEXT;
    }
    else
    {
        Melody_retry=Melody_NORMAL;
        Melody_play=3;
        Melody_play_num=cmd;
    }
//    Melody_SNC7232_W_Byte(0x03, cmd);
//    Melody_SNC7232_W_Byte(0x04,0x01);
}
void Melody_SNC7232_AudioVolume(u8 level)
{
    if(!(Melody_SNC7232_W_Byte(0x06,level)))
        Melody_retry=Melody_SETVOLUME;
    else
        Melody_retry=Melody_NORMAL;
}

#endif

#if (TOUCH_PANEL == TOUCH_PANEL_DRIVER_CAPACITIVE)
//-----------------------------------------------------------------------------

#define FT5406_IIC_ADDR 0x38
#define FT5406_IIC_WR_ADDR (FT5406_IIC_ADDR << 1)
#define FT5406_IIC_RD_ADDR (FT5406_IIC_WR_ADDR | 0x01)


bool FT5406_writeByte(u8 reg, u8 val)
{
    return i2cWrite_Byte(FT5406_IIC_WR_ADDR, reg, val);
}


u8 FT5406_readByte(u8 reg)
{
    u8 pData;
    i2cRead_Byte(FT5406_IIC_RD_ADDR, reg, &pData);
    return pData;
}

//-----------------------------------------------------------------------------

typedef enum {
    ADDR_DeviceMode,
    ADDR_GestID,
    ADDR_TdStatus,
    ADDR_Touch1XH,
    ADDR_Touch1XL,
    ADDR_Touch1YH,
    ADDR_Touch1YL,
    ADDR_VersionH = 0xA1,       ///< Firmware Library Version H byte
    ADDR_VersionL = 0xA2,       ///< Firmware LIbrary Version L byte
    ADDR_ChipVendorID = 0xA3,
    ADDR_FirmwareID = 0xA6,
} DataAddr;


typedef enum {
    MODE_Op = 0x00,     ///< Read touch point and gesture
    MODE_Test = 0x04,   ///< Read raw data
    MODE_SysInfo = 0x01 ///< Read system information related Reserved
} DeviceMode;

static void FT5406_reset(void)
{
    gpioSetDir(GPIO_GROUP_TOUCH_PANNEL_INT, GPIO_BIT_TOUCH_PANNEL_INT, GPIO_DIR_IN);

    gpioSetDir(GPIO_GROUP_TOUCH_PANNEL_RST, GPIO_BIT_TOUCH_PANNEL_RST, GPIO_DIR_OUT);
    gpioSetLevel(GPIO_GROUP_TOUCH_PANNEL_RST, GPIO_BIT_TOUCH_PANNEL_RST, GPIO_LEVEL_LO);    
    OSTimeDly(1);
    gpioSetLevel(GPIO_GROUP_TOUCH_PANNEL_RST, GPIO_BIT_TOUCH_PANNEL_RST, GPIO_LEVEL_HI);
}


static void FT5406_printInfo(void)
{
    u8 version;

    FT5406_writeByte(ADDR_DeviceMode, MODE_Op << 4);

    DEBUG_I2C("== FT5406 Information ==\n");
    version = FT5406_readByte(ADDR_VersionH) << 8;
    version += FT5406_readByte(ADDR_VersionL);
    DEBUG_I2C("Firmware Version: %Xh\n", version);
    DEBUG_I2C("Chip Vendor ID: %Xh\n", FT5406_readByte(ADDR_ChipVendorID));
    DEBUG_I2C("Firmeare ID: %Xh\n", FT5406_readByte(ADDR_FirmwareID));
}


void CTP_init(void)
{
    FT5406_reset();
    FT5406_printInfo();
    FT5406_writeByte(ADDR_DeviceMode, MODE_Op << 4);
}

//-----------------------------------------------------------------------------

bool CTP_getPosition(int *x, int *y)
{
    typedef enum {
        ID_NoGesture = 0x00,
        ID_MoveUp = 0x10,
        ID_MoveLeft = 0x14,
        ID_MoveDown = 0x18,
        ID_MoveRight = 0x48,
        ID_ZoomIn = 0x49,
        ID_ZoomOut = 0x10,
    } GestureID;

    typedef enum {
        EV_PutDown,
        EV_PutUp,
        EV_Contact
    } EventFlag;

    u8 touches;
    u8 u8level;
    EventFlag event;

    gpioGetLevel(GPIO_GROUP_TOUCH_PANNEL_INT, GPIO_BIT_TOUCH_PANNEL_INT, &u8level);
    if (u8level == 1)
        return false;

    touches = FT5406_readByte(ADDR_TdStatus) & 0x0F;
    if (touches == 0)
        return false;

    *y = FT5406_readByte(ADDR_Touch1XH);
    event = (EventFlag)(*y >> 6);
    *y = (*y & 0x0F) << 8;
    *y += FT5406_readByte(ADDR_Touch1XL);
    *x = FT5406_readByte(ADDR_Touch1YH) << 8;
    *x += FT5406_readByte(ADDR_Touch1YL);
    //printf("Event Flag: %d\n", event);
    DEBUG_I2C("Point Coord: %d, %d\n", *x, *y);

    return true;
}

//-----------------------------------------------------------------------------

#endif




//-----------------------------------------------------------------------------

