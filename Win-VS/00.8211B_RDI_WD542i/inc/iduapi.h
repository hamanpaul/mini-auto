/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	iduapi.h

Abstract:

   	The application interface of Image Display Unit

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IDU_API_H__
#define __IDU_API_H__


//0x0c
  #define TV_INTC_FRMEND__ENA      0x00008000
  
  #define TV_INTC_BOTFDSTART_ENA   0x00000200
  #define TV_INTC_TOPFDSTART_ENA   0x00000400
  #define TV_INTC_ALLFDSTART_ENA   0x00000600
  #define TV_INTC_ALL_DISA         0x00000000

  // IDU_INTC 0x000C
  #define IDU_FTCINTS_MASK    0x00000001
  #define IDU_FTCINTS_SHFT    0

  #define IDU_VURINTS_MASK    0x00000010
  #define IDU_VURINTS_SHFT    4

  #define IDU_OURINTS_MASK    0x00000020
  #define IDU_OURINTS_SHFT    5

  #define IDU_FTCINT_ENA      0x00000100
  #define IDU_FTCINT_DISA     0x00000000

  #define IDU_VURINT_ENA      0x00001000
  #define IDU_VURINT_DISA 0x00000000

  #define IDU_OURINT_ENA      0x00002000
  #define IDU_OURINT_DISA 0x00000000

  #define IDU_HACT_MASK       0x01000000
  #define IDU_HACT_SHFT       24

  #define IDU_VACT_MASK       0x02000000
  #define IDU_VACT_SHFT       25

  #define IDU_VSYNC_MASK      0x04000000
  #define IDU_VSYNC_SHFT      26
/* Constant */
/*========For Roule Brightness and Ssturation adjustment========*/
#if ((HW_BOARD_OPTION == ROULE_DOORPHONE)|| (HW_BOARD_OPTION == ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N))
  #define MAX_LCD_BRIGHTNESS_LEVEL 18
  #define MAX_LCD_SATURATION_LEVEL 18

/* Brightness Parameters */
static u32 unIduParaLcdBrightness[MAX_LCD_BRIGHTNESS_LEVEL][3]=
{

	{  /* 0 -3.0 dB */
		0x00000010,
		0x00000010,
		0x00000010	
	},
	{  /* 1, -3.0 dB */
		0x00000010,
		0x00000010,
		0x00000010	
	},
	{  /* 2, -2.5 dB */
		0x00000011,
		0x00000011,
		0x00000011	
	},
	{  /* 3, -2.0 dB */
		0x00000014,
		0x00000014,
		0x00000014	
	},
	{  /* 4, -1.6 dB */
		0x00000016,
		0x00000016,
		0x00000016	
	},
	{  /* 5, -1.2 dB */
		0x00000018,
		0x00000018,
		0x00000018	
	},
	{  /* 6, -0.8 dB */
		0x0000001A,
		0x0000001A,
		0x0000001A	
	},	
	{  /* 7, -0.4 dB */
		0x0000001D,
		0x0000001D,
		0x0000001D	
	},
	{  /* 8,  0   dB */
		0x00000020,
		0x00000020,
		0x00000020	
	},	
	{  /* 9,  0.25 dB */
		0x00000021,
		0x00000021,
		0x00000021	
	},
	{  /* 10, 0.5 dB */
		0x00000023,
		0x00000023,
		0x00000023	
	},
	{  /* 11, 0.75 dB */
		0x00000026,
		0x00000026,
		0x00000026	
	},
	{  /* 12, 1.0 dB */
		0x00000028,
		0x00000028,
		0x00000028	
	},
	{  /* 13, 1.25 dB */
		0x0000002A,
		0x0000002A,
		0x0000002A	
	},
	{  /* 14, 1.5 dB */
		0x0000002D,
		0x0000002D,
		0x0000002D	
	},
	{  /* 15, 1.75 dB */
		0x0000002F,
		0x0000002F,
		0x0000002F	
	},
	{  /* 16  2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	},
	{  /* 17, 2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	}

};
/* Saturation Parameters */
static u32 unIduParaLcdSat[MAX_LCD_SATURATION_LEVEL][3]=
{

	{  /* 0, -4.0 dB */
		0x00110000,
		0x00898400,
		0x00001500	
	},
	{  /* 1, -4.0 dB */
		0x00110000,
		0x00898400,
		0x00001500	
	},
	{  /* 2, -3.5 dB */
		0x00140000,
		0x008A8400,
		0x00001800	
	},
	{  /* 3, -3.0 dB */
		0x00160000,
		0x008B8500,
		0x00001B00	
	},
	{  /* 4, -2.5 */
		0x00190000,
		0x008C8600,
		0x00001E00	
	},
	{  /* 5, -2.0 */
		0x001C0000,
		0x008E8600,
		0x00002200	
	},
	{  /* 6, -1.5 */
		0x001F0000,
		0x00908700,
		0x00002700	
	},
	{  /* 7, -1.0 dB */
		0x00230000,
		0x00928800,
		0x00002B00	
	},
	{  /* 8, -0.5 */
		0x00270000,
		0x00948900,
		0x00003000	
	},
	{  /* 9, 0.0 */
		0x002C0000,
		0x00968B00,
		0x00003700	
	},
	{  /* 10, 0.5 */
		0x00320000,
		0x00998C00,
		0x00003D00	
	},
	{  /* 11, 1.0 */
		0x00380000,
		0x009C8D00,
		0x00004500	
	},
	{  /* 12, 1.5 */
		0x003F0000,
		0x00A08F00,
		0x00004D00	
	},
	{  /* 13, 2.3 */
		0x004C0000,
		0x00A69200,
		0x00005D00	
	},	
	{  /* 14, 3.1 dB */
		0x005B0000,
		0x00AE9800,
		0x00007000	
	},
	{  /* 15, 3.8 dB */
		0x006B0000,
		0x00B69A00,
		0x00007F00	
	},
	{  /* 16, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	},
 	{  /* 17, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	}
	
};
  #if (HW_BOARD_OPTION==ROULE_SD8F)
  
  #define MAX_TVOUT_BRIGHTNESS_LEVEL 18
  #define MAX_TVOUT_SATURATION_LEVEL 18
  
  #elif(HW_BOARD_OPTION == ROULE_SD7N)
  
  #define MAX_TVOUT_BRIGHTNESS_LEVEL 64
  #define MAX_TVOUT_SATURATION_LEVEL 64
  #endif
  
#elif (HW_BOARD_OPTION == LEIYON_DOORPHONE)

  #define MAX_LCD_BRIGHTNESS_LEVEL 10
  #define MAX_LCD_SATURATION_LEVEL 10

/* Brightness Parameters */
static u32 unIduParaLcdBrightness[MAX_LCD_BRIGHTNESS_LEVEL][3]=
{

	{  /* 0 -3.0 dB */
		0x00000010,
		0x00000010,
		0x00000010	
	},
	{  /* 2, -2.5 dB */
		0x00000011,
		0x00000011,
		0x00000011	
	},
	{  /* 4, -1.6 dB */
		0x00000016,
		0x00000016,
		0x00000016	
	},
	{  /* 6, -0.8 dB */
		0x0000001A,
		0x0000001A,
		0x0000001A	
	},	
	{  /* 8,  0   dB */
		0x00000020,
		0x00000020,
		0x00000020	
	},	
	{  /* 10, 0.5 dB */
		0x00000023,
		0x00000023,
		0x00000023	
	},
	{  /* 12, 1.0 dB */
		0x00000028,
		0x00000028,
		0x00000028	
	},
	{  /* 14, 1.5 dB */
		0x0000002D,
		0x0000002D,
		0x0000002D	
	},
	{  /* 16  2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	},
	{  /* 16  2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	}
	

};
/* Saturation Parameters */
static u32 unIduParaLcdSat[MAX_LCD_SATURATION_LEVEL][3]=
{

	{  /* 0, -4.0 dB */
		0x00110000,
		0x00898400,
		0x00001500	
	},
	{  /* 2, -3.5 dB */
		0x00140000,
		0x008A8400,
		0x00001800	
	},
	{  /* 4, -2.5 */
		0x00190000,
		0x008C8600,
		0x00001E00	
	},
	{  /* 6, -1.5 */
		0x001F0000,
		0x00908700,
		0x00002700	
	},
	{  /* 8, -0.5 */
		0x00270000,
		0x00948900,
		0x00003000	
	},
	{  /* 10, 0.5 */
		0x00320000,
		0x00998C00,
		0x00003D00	
	},
	{  /* 12, 1.5 */
		0x003F0000,
		0x00A08F00,
		0x00004D00	
	},
	{  /* 14, 3.1 dB */
		0x005B0000,
		0x00AE9800,
		0x00007000	
	},
	{  /* 16, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	},
	{  /* 16, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	}	
	
	
};
#else
/* Brightness Parameters */
  #define MAX_LCD_BRIGHTNESS_LEVEL 18
  #define MAX_LCD_SATURATION_LEVEL 18
  
static u32 unIduParaLcdBrightness[MAX_LCD_BRIGHTNESS_LEVEL][3]=
{

	{  /* 0 -3.0 dB */
		0x00000010,
		0x00000010,
		0x00000010	
	},
	{  /* 1, -3.0 dB */
		0x00000010,
		0x00000010,
		0x00000010	
	},
	{  /* 2, -2.5 dB */
		0x00000011,
		0x00000011,
		0x00000011	
	},
	{  /* 3, -2.0 dB */
		0x00000014,
		0x00000014,
		0x00000014	
	},
	{  /* 4, -1.6 dB */
		0x00000016,
		0x00000016,
		0x00000016	
	},
	{  /* 5, -1.2 dB */
		0x00000018,
		0x00000018,
		0x00000018	
	},
	{  /* 6, -0.8 dB */
		0x0000001A,
		0x0000001A,
		0x0000001A	
	},	
	{  /* 7, -0.4 dB */
		0x0000001D,
		0x0000001D,
		0x0000001D	
	},
	{  /* 8,  0   dB */
		0x00000020,
		0x00000020,
		0x00000020	
	},	
	{  /* 9,  0.25 dB */
		0x00000021,
		0x00000021,
		0x00000021	
	},
	{  /* 10, 0.5 dB */
		0x00000023,
		0x00000023,
		0x00000023	
	},
	{  /* 11, 0.75 dB */
		0x00000026,
		0x00000026,
		0x00000026	
	},
	{  /* 12, 1.0 dB */
		0x00000028,
		0x00000028,
		0x00000028	
	},
	{  /* 13, 1.25 dB */
		0x0000002A,
		0x0000002A,
		0x0000002A	
	},
	{  /* 14, 1.5 dB */
		0x0000002D,
		0x0000002D,
		0x0000002D	
	},
	{  /* 15, 1.75 dB */
		0x0000002F,
		0x0000002F,
		0x0000002F	
	},
	{  /* 16  2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	},
	{  /* 17, 2.1 dB */
		0x00000033,
		0x00000033,
		0x00000033	
	}

};
/* Saturation Parameters */
static u32 unIduParaLcdSat[MAX_LCD_SATURATION_LEVEL][3]=
{

	{  /* 0, -4.0 dB */
		0x00110000,
		0x00898400,
		0x00001500	
	},
	{  /* 1, -4.0 dB */
		0x00110000,
		0x00898400,
		0x00001500	
	},
	{  /* 2, -3.5 dB */
		0x00140000,
		0x008A8400,
		0x00001800	
	},
	{  /* 3, -3.0 dB */
		0x00160000,
		0x008B8500,
		0x00001B00	
	},
	{  /* 4, -2.5 */
		0x00190000,
		0x008C8600,
		0x00001E00	
	},
	{  /* 5, -2.0 */
		0x001C0000,
		0x008E8600,
		0x00002200	
	},
	{  /* 6, -1.5 */
		0x001F0000,
		0x00908700,
		0x00002700	
	},
	{  /* 7, -1.0 dB */
		0x00230000,
		0x00928800,
		0x00002B00	
	},
	{  /* 8, -0.5 */
		0x00270000,
		0x00948900,
		0x00003000	
	},
	{  /* 9, 0.0 */
		0x002C0000,
		0x00968B00,
		0x00003700	
	},
	{  /* 10, 0.5 */
		0x00320000,
		0x00998C00,
		0x00003D00	
	},
	{  /* 11, 1.0 */
		0x00380000,
		0x009C8D00,
		0x00004500	
	},
	{  /* 12, 1.5 */
		0x003F0000,
		0x00A08F00,
		0x00004D00	
	},
	{  /* 13, 2.3 */
		0x004C0000,
		0x00A69200,
		0x00005D00	
	},	
	{  /* 14, 3.1 dB */
		0x005B0000,
		0x00AE9800,
		0x00007000	
	},
	{  /* 15, 3.8 dB */
		0x006B0000,
		0x00B69A00,
		0x00007F00	
	},
	{  /* 16, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	},
 	{  /* 17, 4.6 dB */
		0x00700000,
		0x00B99B00,
		0x00007F00	
	}
	
};

#endif

enum
{
    IDU_OSD_L0_WINDOW_0 = 0,
    IDU_OSD_L0_WINDOW_1,
    IDU_OSD_L0_WINDOW_2,
    IDU_OSD_L1_WINDOW_0,
    IDU_OSD_L2_WINDOW_0,
    IDU_OSD_MAX_NUM
};

enum
{
    IDU_OSD_TYPE_4ALPHA = 0,    /* 4-bit alpha index mode (2-bit alpha + 2-bit index data)*/
    IDU_OSD_TYPE_4COLOR,        /*4-bit color index mode (4-bit index data)*/
    IDU_OSD_TYPE_8COLOR,        /*8-bit alpha index mode (2-bit alpha + 2-bit dummy + 4-bit index data)*/
};

/* Function prototype */

extern s32 iduPreview422(int src_W,int src_H);
extern s32 iduPreview(int src_W,int src_H);
extern s32 subTV_Preview(int src_W,int src_H);
extern s32 iduCapturePrimary(void);
extern s32 iduCaptureVideo(int src_W,int src_H);

extern s32 LCM_IDUInit(void);
extern void iduIntHandler(void);
extern void subTVIntHandler(void);
extern void iduPlaybackMode(int SrcWidth,int SrcHeight,int SrcStride);
extern void OSD_Black_Color_Bar(u8 bEnable);

extern void iduRst(void);
extern void iduOSDDisable_All(void);
extern void iduOSDEnable_All(void);
extern void iduTVOSDDisable_All(void);
extern u8 iduGetOSDType(u8 buf_idx);
extern void idu_switch(void);
extern void iduSwitchNTSCPAL(int Mode);


/* BJ 0505 S */
extern void iduOSDDisable(u8 osd);
extern void iduOSDEnable(u8 osd);
extern void iduTVOSDDisable(u8 tvosd);
extern void iduTVOSDEnable(u8 tvosd);
extern void iduSetVBuff(u8* buf1 , u8 * buf2 , u8 * buf3);
extern void iduOSDDisplay1(u8 blk_idx, u32 OSD_SX, u32 OSD_SY, u32 OSD_EX, u32 OSD_EY);
extern void iduTVOSDDisplay(u8 blk_idx, u32 TVOSD_SX, u32 TVOSD_SY, u32 TVOSD_EX, u32 TVOSD_EY);
extern void iduTVOSDDisplay_D1(u8 blk_idx, u32 TVOSD_SX, u32 TVOSD_SY, u32 TVOSD_EX, u32 TVOSD_EY);
extern u32  iduOSDGetYStartEnd(u8 buf_idx, u32 *OSD_SY, u32 *OSD_EY);
extern void osdDrawPreviewIcon(void);
extern void osdDrawVideoIcon(void);
extern void uiClearOSDBuf(u8 blk_idx);
extern u32 uiGetOSDBufAdr(u8 buf_idx);
extern void iduPlaybackFrame(u8 *);/*BJ 0523 S*/
extern void IduVideo_ClearPKBuf(u8 bufinx);
extern void iduSetVidBufAddr(u8 bufidx, u8 *bufaddr);
extern void iduPlaybackToMenu(void);
extern void IduVideoEnable(u8 enable);


/* BJ 0505 E */
void TV_reset(void);
void TV_init(u8 TVOutMode,u32 DivFac,u8 RunMode);
void subTV_init(u8 TVOutMode,u32 DivFac,u8 RunMode);

void IduVideo_ClearBuf(void);
void IduVideo_ClearPartPKBuf(u32 start, u32 end, u8 bufinx);
void IDU_TVLayer(u16 pannel_w , u8 *scaler_data , u16 scaler_w , u16 scaler_h , u16 x_pos , u16 y_pos);
void IDU_TVLayer_Stride(u16 pannel_w , u8 *scaler_data , u16 scaler_w , u16 scaler_h , u16 x_pos , u16 y_pos, u16 scaler_stride_w,u8 *target_addr);
void IDU_TVMoveData(u16 pannel_w , u8 *dst_data , u8 *src_data , u16 scaler_w , u16 scaler_h , u8 type);
extern void iduSetLcdBrightness(s8);
extern void iduSetLcdSat(s8);
extern void iduSetVideoBufAddr(u8*, u8*, u8*);
extern void iduSetTVOutBrightness(s8);
extern void iduVdoCtrl(u8);
extern void iduTVOSDClear(void);
extern void iduOSDClear(void);
extern void IDU_Init(u8 version , u8 rotation);
extern void iduSetVideoBuf0Addr(u8* buf);
extern void iduChangeDispSrcWidthHeight(int Width, int Height);

#if (HW_BOARD_OPTION == VER100_CARDVR)
void iduSendBit_1621(u8 dt,u8 cnt); //first sent MSB.
void iduSendDataBit_1621(u8 dt,u8 cnt); //first sent LSB.
void iduSendHT1621Cmd(u8 command);
void iduWrite_1621(u8 addr,u8 dt);
void iduWriteAll_1621(u8 addr,u8 *p,u8 cnt);
void iduLCDDispAll(void);
void iduClearLCD(void);
void iduControlSymbol(u8 sd,u8 circle, u8 rec, u8 usb);
void iduControlSymbolSD(u8 sd);
void iduControlSymbolCircle(u8 circle);
void iduControlSymbolREC(u8 rec);
void iduControlSymbolUSB(u8 usb);
void iduDisplaySymbol(u8 sd,u8 circle, u8 rec, u8 usb);
void iduDispLCDString(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop);
void iduDisplayDigitNum(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop);
void iduDisplayDefaultSymbol(void);;
void iduInitHT1621LCD(void);
void iduCloseHT1621LCD(void);
#elif ((HW_BOARD_OPTION == SIYUAN_CVR)||(HW_BOARD_OPTION==NEW_SIYUAN))
extern void iduSendBit_1621(u8,u8); //first sent MSB.
extern void iduSendDataBit_1621(u8,u8); //first sent LSB.
extern void iduSendHT1621Cmd(u8);
extern void iduWrite_1621(u8,u8);
extern void iduWriteAll_1621(u8,u8 *,u8);
extern void iduLCDDispAll(void);
extern void iduClearLCD(void);
extern void iduControlSymbol(u8, u8, u8, u8);
extern void iduControlSymbolSD(u8);
extern void iduControlSymbolCircle(u8);
extern void iduControlSymbolREC(u8);
extern void iduControlSymbolUSB(u8);
extern void iduDisplaySymbol(u8,u8, u8, u8);
extern void iduDispLCDString(u8 *, u8, u8);
extern void iduDisplayDigitNum(u8 *, u8, u8);
extern void iduDisplayDefaultSymbol(void);;
extern void iduInitHT1621LCD(void);
extern void iduCloseHT1621LCD(void);
extern void iduSetDateTime(u8);
extern void iduDispTime(void);
extern void iduUpdateLCD(void);
#endif

#if (HW_BOARD_OPTION == NEW_SIYUAN)

typedef enum LCDFLASH_CLASS
{
    LCD_FLASH_DEFAULT = 0,
    LCD_FLASH_SET_DATE_TIME,
    LCD_FLASH_SET_VIDEO_SIZE,
    LCD_FLASH_SET_FRAME_RATE,
    LCD_FLASH_SET_DELET_FILE,
    LCD_FLASH_SET_VOICE,
    LCD_FLASH_SET_CALENDAR,
    LCD_FLASH_SET_OFF
} LCD_FLASH_CLASS;

typedef enum LCDFLASH_DATE
{
    LCD_SET_YEAR = 1,
    LCD_SET_MONTH,
    LCD_SET_DATE,
    LCD_SET_HOUR,
    LCD_SET_MINUTE
} LCD_FLASH_DATE;

typedef enum LCDFLASH_RESOLUTION
{
    LCD_RESOLUTION_BIG = 1,
    LCD_RESOLUTION_MEDIUM,
    LCD_RESOLUTION_SMALL,
    LCD_RESOLUTION_CLEAR
} LCD_FLASH_RESOLUTION;

typedef enum LCDFLASH_FRAME_RATE
{
    LCD_FRAME_RATE_30 = 1,
    LCD_FRAME_RATE_15,
    LCD_FRAME_RATE_CLEAR
} LCD_FLASH_FRAME_RATE;

typedef enum LCDFLASH_DELETE_FILE
{
    LCD_DELETE_LAST_FILE = 1,
    LCD_DELETE_ALL_FILE,
    LCD_DELETE_FILE_CLEAR
} LCD_FLASH_DELETE_FILE;

typedef enum LCDFLASH_YEAR_POSITION
{
    LCD_TEN_YEARS = 1,
    LCD_SINGLE_YEAR
} LCD_FLASH_YEAR_POSITION;


#define LCD_VOICE_ENABLE            0x01
#define LCD_VOICE_DISABLE           0x00

#define LCD_CALENDAR_ENABLE         0x01
#define LCD_CALENDAR_DISABLE        0x00

#define LCD_SD_ENABLE               0x01
#define LCD_SD_DISABLE              0x00

#define LCD_SYMBOL_Y_ENABLE         0x01
#define LCD_SYMBOL_Y_DISABLE        0x00

#define LCD_SYMBOL_M1_ENABLE        0x01
#define LCD_SYMBOL_M1_DISABLE       0x00

#define LCD_SYMBOL_M2_ENABLE        0x01
#define LCD_SYMBOL_M2_DISABLE       0x00

#define LCD_SYMBOL_H_ENABLE         0x01
#define LCD_SYMBOL_H_DISABLE        0x00

#define LCD_SYMBOL_D_ENABLE         0x01
#define LCD_SYMBOL_D_DISABLE        0x00

#define LCD_SYMBOL_S_ENABLE         0x01
#define LCD_SYMBOL_S_DISABLE        0x00

extern void iduSetFlashClass(LCD_FLASH_CLASS);
extern void iduSetTimeFlashClass(LCD_FLASH_DATE);
extern void iduSetSizeFlashClass(LCD_FLASH_RESOLUTION);
extern void iduSetFameRateFlashClass(LCD_FLASH_FRAME_RATE);
extern void iduSetDeleteFileFlashClass(LCD_FLASH_DELETE_FILE);
extern void iduSetYearPositionFlash(LCD_FLASH_YEAR_POSITION);
extern void IduDispWinSel(u8 index);


#endif

typedef struct
{
  u32        BlkSX;
  u32        BlkSY;
  u32        BlkEX;
  u32        BlkEY;
  u8         *BlkAddr;
} IDU_OSD_BLK_INFO;

extern u8  IsIduOsdEnable;    // 1: Enable, 0: Disable
extern IDU_OSD_BLK_INFO OsdBlkInfo[IDU_OSD_MAX_NUM];
#endif

