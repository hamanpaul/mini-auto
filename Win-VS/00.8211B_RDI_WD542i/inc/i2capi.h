/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	i2capi.h

Abstract:

   	The application interface of the I2C controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __I2C_API_H__
#define __I2C_API_H__

#include "rtcapi.h"
//------------//
#define I2C_TVP5150_WR_SLAV_ADDR_1    0x000000B8
#define I2C_TVP5150_RD_SLAV_ADDR_1    0x000000B9

#define I2C_TVP5150_WR_SLAV_ADDR_2    0x000000BA
#define I2C_TVP5150_RD_SLAV_ADDR_2    0x000000BB


#define I2C_BIT1605_WR_SLAV_ADDR      0x00000040
#define I2C_BIT1605_RD_SLAV_ADDR      0x00000041

#define I2C_BIT1605_WR_SLAV_2_ADDR    0x00000044
#define I2C_BIT1605_RD_SLAV_2_ADDR    0x00000045

#define I2C_WT8861_WR_SLAV_ADDR		  0x00000024
#define I2C_WT8861_RD_SLAV_ADDR		  0x00000025

#define I2C_WT8861_WR_SLAV_2_ADDR     0x00000026
#define I2C_WT8861_RD_SLAV_2_ADDR     0x00000027

#define I2C_DM5900_WR_SLAV_ADDR       0x000000ba
#define I2C_DM5900_RD_SLAV_ADDR       0x000000bb

#define I2C_DM5900_WR_SLAV_2_ADDR     0x000000B8
#define I2C_DM5900_RD_SLAV_2_ADDR     0x000000B9

#if (TV_DECODER == TW9900)
#define I2C_TW9900_WR_SLAV_ADDR       0x0000008a
#define I2C_TW9900_RD_SLAV_ADDR       0x0000008b

#define I2C_TW9900_WR_SLAV_2_ADDR     0x00000088
#define I2C_TW9900_RD_SLAV_2_ADDR     0x00000089
#elif (TV_DECODER == TW9910)
#define I2C_TW9910_WR_SLAV_ADDR       0x00000088
#define I2C_TW9910_RD_SLAV_ADDR       0x00000089
#elif (TV_DECODER == TW2866)
#define I2C_TW2866_WR_SLAV_ADDR       0x00000050
#define I2C_TW2866_RD_SLAV_ADDR       0x00000051
#endif
#if (ECHO_IC == FM1288)
#define I2C_FM1288_WR_SLAV_ADDR       0x000000C0
#define I2C_FM1288_RD_SLAV_ADDR       0x000000C1
#endif


#if((HW_BOARD_OPTION == MR8200_RX_GCT_LCD) || (HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD))
#define I2C_BS83B12_WR_SLAV_ADDR      0x000000A0
#define I2C_BS83B12_RD_SLAV_ADDR      0x000000A1
#else
#define I2C_BS83B12_WR_SLAV_ADDR      0x00000040
#define I2C_BS83B12_RD_SLAV_ADDR      0x00000041
#endif


#if((HW_BOARD_OPTION == MR8600_RX_GCT) || (HW_BOARD_OPTION == MR8600_RX_JIT) ||\
    (HW_BOARD_OPTION == MR8600_RX_JESMAY) || (HW_BOARD_OPTION == MR8600_RX_MAYON)||\
    (HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS))
#define I2C_BIT1201G_WR_SLAV_ADDR     0x00000002
#define I2C_BIT1201G_RD_SLAV_ADDR     0x00000003
#else
#define I2C_BIT1201G_WR_SLAV_ADDR     0x00000000
#define I2C_BIT1201G_RD_SLAV_ADDR     0x00000001
#endif
#define I2C_CS8556_WR_SLAV_ADDR       0x0000007A
#define I2C_CS8556_RD_SLAV_ADDR       0x0000007B

#define I2C_TSC2003_WR_SLAV_ADDR      0x00000094
#define I2C_TSC2003_RD_SLAV_ADDR      0x00000095

#define I2C_PIC16F1516_WR_SLAV_ADDR   0x000000B8   //Touch key
#define I2C_PIC16F1516_RD_SLAV_ADDR   0x000000B9


#define I2C_CH7025_WR_SLAV_ADDR		  0x000000EC
#define I2C_CH7025_RD_SLAV_ADDR  	  0x000000ED

#define I2C_CH7025_WR_SLAV_ADDR_2	  0x000000EA
#define I2C_CH7025_RD_SLAV_ADDR_2     0x000000EB

#define I2C_MA86P03_WR_SLAV_ADDR      0x00000074
#define I2C_MA86P03_RD_SLAV_ADDR      0x00000075

#define I2C_WT6853_WR_SLAV_ADDR       0x00000040
#define I2C_WT6852_RD_SLAV_ADDR       0x00000041


//Roy: 因為不同板子I2C_SEL不同 在check TV format時會出錯
#if(TV_DECODER == TI5150 )
    #define TV_CHECK_FORMAT_RD_ADDR  I2C_TVP5150_RD_SLAV_ADDR_1
    #define TV_CHECK_FORMAT_WR_ADDR  I2C_TVP5150_WR_SLAV_ADDR_1
#endif

#if (Melody_SNC7232_ENA)
    enum
	{
		Melody_NORMAL = 0,
		Melody_STOP,
		Melody_PAUSE,
		Melody_PLAY,
		Melody_PLAYALL,
		Melody_START,
		Melody_STARTALL,
		Melody_PLAYNEXT,
		Melody_SETVOLUME,
	};

#endif
//-------------//
extern s32 i2cWrite_SENSOR(u8, u16);
extern s32 i2cRead_SENSOR(u8, u16*);
#if (Sensor_OPTION ==Sensor_HM1375_YUV601)
s32 i2cWrite_HM1375(u16 addr, u8 data);
s32 i2cRead_HM1375(u16 addr, u8* pData);
s32 i2cRead_HM1375_Manual(u16 ucRegAddr, u8* pucData);
#endif
#if (Sensor_OPTION ==Sensor_NT99141_YUV601)
s32 i2cWrite_NT99141(u16 addr, u8 data);
s32 i2cRead_NT99141(u16 addr, u8* pData);
s32 i2cRead_NT99141_Manual(u16 ucRegAddr, u8* pucData);
#endif
#if (Sensor_OPTION == Sensor_PO3100K_YUV601)
s32 i2cWrite_PO3100K(u8 addr, u8 data);
s32 i2cRead_PO3100K(u8 addr, u8* pData);
#endif

extern s32 i2cInit(void);
extern s32 i2cDeviceInit(void);

extern s32 i2cUninit(void);
extern void i2cIntHandler(void);

s32 i2cWrite_Byte(u8 id, u8 addr, u8 data);
s32 i2cWrite_Data(u8 id, u8 addr, u8 bytes, u32 data);
s32 i2cRead_Byte(u8 id, u8 addr, u8* pData);
s32 i2cRead_Data(u8 id, u8 addr, u8 bytes, void* pData);
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
s32 IIC_Read_2Word(u8 id, u8 addr, u32* pData);
#endif

#if (G_SENSOR == G_SENSOR_LIS302DL)     // for ST LIS302DL
extern s32 i2cWrite_LIS302DL(u8 addr, u8 data);
extern s32 i2cRead_LIS302DL(u8 addr, u8* pData);
extern void i2cInit_LIS302DL(void);
extern u32 i2cPolling_LIS302DL(void);
extern void i2cGet_LIS302DL_XYZ(s8 *out_X, s8 *out_Y, s8 *out_Z);
#elif (G_SENSOR == G_SENSOR_H30CD)   // for Hitachi H30CD
extern s32 i2cWrite_H30CD(u8 addr, u8 data);
extern s32 i2cRead_H30CD(u8 addr, u8* pData);
extern s32 i2cRead_H30CD_Word(u8 addr, u32* pData);
extern s32 i2cRead_H30CD_NextWord(u32* pData);
extern s32 i2cRead_H30CD_2Word_Manual(u8 addr, u32* pData);
extern s32 i2cRead_H30CD_2Word(u8 addr, u32* pData);
s32 i2cWriteCommand_H30CD(u8 data);
extern void i2cInit_H30CD(void);
extern u32 i2cPolling_H30CD(void);
extern void i2cGet_H30CD_XYZ(s16 *out_X, s16 *out_Y, s16 *out_Z);
extern void i2cTest_H30CD(void);
#elif (G_SENSOR == G_SENSOR_DMARD03)   // for Domintech DMARD03
s32 i2cWrite_DMARD03(u8 addr, u8 data);
s32 i2cRead_DMARD03(u8 addr, u8* pData);
s32 i2cRead_DMARD03_HalfWord(u8 addr, u16* pData);
s32 i2cRead_DMARD03_Word(u8 addr, u32* pData);
s32 i2cRead_DMARD03_2Word_Manual(u8 addr, u32* pData);
s32 i2cWriteCommand_DMARD03(u8 data);
void i2cInit_DMARD03(void);
void i2cInit_DMARD03(void);
void i2cGet_DMARD03_TXYZ(s16 *out_T, s16 *out_X, s16 *out_Y, s16 *out_Z);
void i2cTest_DMARD03(void);
#elif (G_SENSOR == G_SENSOR_BMA150)   // for Bosch BMA150
s32 i2cWrite_BMA150(u8 addr, u8 data);
s32 i2cRead_BMA150(u8 addr, u8* pData);
s32 i2cRead_BMA150_HalfWord(u8 addr, u16* pData);
s32 i2cRead_BMA150_Word(u8 addr, u32* pData);
s32 i2cRead_BMA150_2Word_Manual(u8 addr, u32* pData);
s32 i2cWriteCommand_BMA150(u8 data);
void i2cInit_BMA150(void);
void i2cInit_BMA150(void);
void i2cGet_BMA150_TXYZ(s16 *out_T, s16 *out_X, s16 *out_Y, s16 *out_Z);
void i2cTest_BMA150(void);
#endif

#if(TV_DECODER == TI5150)
extern void i2cInit_TVP5150_1(void);
extern void i2cInit_TVP5150_2(void);

extern s32 i2cRead_TVP5150(u8 addr, u8* pData,u8 DeviceAddr);
extern s32 i2cWrite_TVP5150(u8 addr, u8 data,u8 DeviceAddr);

#elif (TV_DECODER == CJC5150 )

extern void i2cInit_CJC5150(void);
extern s32 i2cRead_TVP5150(u8 addr, u8* pData,u8 DeviceAddr);
extern s32 i2cWrite_TVP5150(u8 addr, u8 data,u8 DeviceAddr);
#elif(TV_DECODER == BIT1605)
extern void i2cInit_BIT1605(void);
extern void i2cInit_BIT1605_DV2(void);
extern s32 i2cRead_BIT1605(u8 ucRegAddr, u8* pucData);
extern s32 i2cRead_BIT1605_DV2(u8 ucRegAddr, u8* pucData);
extern s32 i2cWrite_BIT1605(u8 ucRegAddr, u8 ucData);
extern s32 i2cWrite_BIT1605_DV2(u8 ucRegAddr, u8 ucData);
#elif(TV_DECODER == MI9V136)
extern void i2cInit_MI9V136(void);
extern void HW_I2C_Word_addr_Write_Word(u8 dev_addr,u16 addr, u16 write_data);
extern u16 HW_I2C_Word_addr_Read_Word(u8 dev_addr,u16 addr);
#elif(TV_DECODER == WT8861)
extern void i2cInit_WT8861_1(void);
extern void i2cInit_WT8861_2(void);
extern void VIDEO_PortChange(u8 port);
extern void VIDEO_SetupColourStandard(void);
extern s32 i2cRead_WT8861(u8 ucRegAddr, u8* pucData,u32 DeviceAddr);
extern s32 i2cWrite_WT8861(u8 ucRegAddr, u8 ucData,u32 DeviceAddr);
extern void NTSCA(u32 Readaddr,u32 Writeaddr);
extern void VIDEO_OutputTriState(u8 bMode,u32 Readaddr,u32 Writeaddr);
extern void PALMA(u32 Readaddr,u32 Writeaddr);
extern void PALIA(u32 Readaddr,u32 Writeaddr);
extern void PALCNA(u32 Readaddr,u32 Writeaddr);
extern void NTSC443A(u32 Readaddr,u32 Writeaddr);
extern void PAL60A(u32 Readaddr,u32 Writeaddr);
extern void SECAMA(u32 Readaddr,u32 Writeaddr);



#elif(TV_DECODER == TW9900)
extern void i2cInit_TW9900_1(void);
extern void i2cInit_TW9900_2(void);
extern s32 i2cRead_TW9900(u8 ucRegAddr, u8* pucData,u32 DeviceAddr);
extern s32 i2cWrite_TW9900(u8 ucRegAddr, u8 ucData,u32 DeviceAddr);

#elif(TV_DECODER == DM5900)
extern void i2cInit_DM5900_1(void);
extern void i2cInit_DM5900_2(void);
extern s32 i2cRead_DM5900(u8 ucRegAddr, u8* pucData,u32 DeviceAddr);
extern s32 i2cWrite_DM5900(u8 ucRegAddr, u8 ucData,u32 DeviceAddr);

#elif(TV_DECODER == TW9910)
extern void i2cInit_TW9910(void);

extern s32 i2cRead_TW9910(u8 ucRegAddr, u8* pucData,u32 DeviceAddr);
extern s32 i2cWrite_TW9910(u8 ucRegAddr, u8 ucData,u32 DeviceAddr);
#elif(TV_DECODER == TW2866)
extern void i2cInit_TW2866(void);

extern s32 i2cWrite_TW2866(u16 addr, u8 data);
extern s32 i2cRead_TW2866_Manual(u16 addr, u8 *data);
#endif
#if(ECHO_IC == FM1288)
extern s32 i2cInit_FM1288_Rx(void);
extern s32 i2cInit_FM1288_Tx(void);
#endif
extern void contrast_write(u8 data);
extern u8 contrast_read(u8* data);
extern void brightness_write(u8 data);
extern u8 brightness_read(u8* data);
extern void saturation_write(u8 data);
extern u8 saturation_read(u8* data);
extern void CH_Channel_write(u8 ch,u8 data);

#if(TV_ENCODER== BIT1201G)
extern s32 i2cRead_BIT1201G(u8 ucRegAddr, u8 * pucData, u8 DeviceAddr);
extern s32 i2cWrite_BIT1201G(u8 ucRegAddr, u8 ucData, u8 DeviceAddr);
extern void i2cInit_BIT1201G(void);
#endif

#if(GPIO_I2C_ENA == 1)
extern s32 i2cWrite_CS8556(u8 ucRegAddr, u8 ucData,u8 DeviceAddr);
extern s32 i2cRead_CS8556(u8 addr, u8* pData, u8 DeviceAddr);
extern void i2cInit_CS8556(void);
#endif
extern s32 i2cRead_ISL1208(u8 addr, u8* pData);
extern s32 i2cWrite_ISL1208(u8 addr, u8 data);
extern BOOLEAN Get_ISL1208_RTC(RTC_DATE_TIME *rtc_time);
extern BOOLEAN Set_ISL1208_RTC(RTC_DATE_TIME* date);
extern void ISL1208_RTC_Init(void);
#if((USE_BUILD_IN_RTC == 0) && (EXT_RTC_SEL == RTC_SD2068))
extern BOOLEAN Get_SD2068_RTC(RTC_DATE_TIME *rtc_time);
extern BOOLEAN Set_SD2068_RTC(RTC_DATE_TIME* date);
extern void SD2068_RTC_Init(void);
#endif
extern s32 i2cRead_HM1375_Manual(u16 ucRegAddr, u8* pucData);

#if(TV_ENCODER == CH7025)
extern s32 i2cRead_CH7025(u8 ucRegAddr, u8 * pucData);
extern s32 i2cWrite_CH7025(u8 ucRegAddr, u8  pucData);
extern void i2cInit_CH7025(void);
#endif

#if(EXT_RTC_SEL == RTC_BQ32000)
s32 i2cWrite_BQ32000(u8 addr, u8 data);
s32 i2cWrite_BQ32000_Word(u8 addr, u32 data, u8 ByteLength);
s32 i2cRead_BQ32000(u8 addr, u8* pData);
s32 i2cRead_BQ32000_Word(u8 addr, u32* pData, u8 ByteLength);
void i2cTest_RTC_BQ32000(void);
#endif

#if 0//(LCM_OPTION == LCM_CT674_LF)
 extern void i2cInit_CT674LF(void);
 extern void CT674_LCD_PANEL_On(void);
 extern void CT674_LCD_PANEL_Off(void);
#endif

#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
extern s32 i2cWrite_ALC5621(u8 addr, u16 data);
extern s32 i2cRead_ALC5621(u8 addr, u16* pData);
extern void ALC5621_AdjustVolume(u8 Volume);
extern void ALC5621_AdjustMICVolume(u8 Volume);
extern void ALC5621_ChannelSwitch(u8 ch);
extern void Init_IIS_ALC5621_play(void);
extern void Init_IIS_ALC5621_bypass(void);
extern void Init_IIS_ALC5621_rec(void);
extern void Close_IIS_ALC5621(void);
#endif

#if (TOUCH_KEY == TOUCH_KEY_BS83B12)
extern s32 i2cRead_BS83B12(u8 id, u8* pData);
extern s32 i2cRead_BS83B12_2Byte(u8 id, u16* pData);
#elif (TOUCH_KEY == TOUCH_KEY_MA86P03)
extern s32 i2cRead_MA86P03(u8 reg , u8 *data );
extern s32 i2cWrite_MA86P03(u8 reg, u8 data);
extern u8 i2cReadTouchKey(u8 *data);

#endif

#if (IO_EXPAND == IO_EXPAND_WT6853)
extern s32 i2cRead_WT6853(u8 reg , u8 *data );
extern s32 i2cWrite_WT6853(u8 reg, u8 data);
#endif

#if IIC_TEST
 extern void i2cTest(void);
#endif

extern OS_EVENT* i2cWDTProtect;

extern bool i2c_Touch_getPosition(int *x, int *y);

extern s32 i2cRead_PIC16F1516(u8 id, u8* pData);
extern s32 i2cWrite_PIC16F1516(u8 id,u8 data);

#if(THERMOMETER_SEL == THERMO_MLX90615)
s32 i2cRead_MLX90615(u8 addr, u16* pData);
void i2cRead_MLX90615_TempA(f32* pData);
void i2cRead_MLX90615_TempO(f32* pData);
#endif

#endif
