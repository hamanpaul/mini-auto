/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sensorapi.h

Abstract:

   	The declarations of Sensor API.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SENSOR_API_H__
#define __SENSOR_API_H__

/* use Micron MT9D011 as an example */

/* Type definition */

typedef struct _SENSOR_OPR {
	u8	opr;
	u8	addr;
	u16	data;
	u16	mask;
} SENSOR_OPR;

/* Constant */

/* use Micron MT9D011 as an example */

#define SENSOR_CHIP_VERSION		0x00
#define SENSOR_ROW_START		0x01
#define SENSOR_COLUMN_START		0x02
#define SENSOR_ROW_WIDTH		0x03
#define SENSOR_COLUMN_WIDTH		0x04
#define SENSOR_H_BLANK_B		0x05
#define SENSOR_V_BLANK_B		0x06
#define SENSOR_H_BLANK_A		0x07
#define SENSOR_V_BLANK_A		0x08
#define SENSOR_SHUTTER_WIDTH		0X09
#define SENSOR_ROW_SPEED		0x0a
#define SENSOR_EXTRA_DELAY		0x0b
#define SENSOR_SHUTTER_DELAY		0x0c
#define SENSOR_RESET			0x0d

#define SENSOR_FRAME_VALID_CONTROL	0x1f
#define SENSOR_REAL_MODE_B		0x20
#define SENSOR_REAL_MODE_A		0x21
#define SENSOR_DARK_COLUMNS_ROWS	0x22
#define SENSOR_FLASH			0x23
#define SENSOR_EXTRA_RESET		0x24
#define SENSOR_LINE_VALID_CONTROL	0x25
#define SENSOR_BOTTOM_DARK_ROWS		0x26

#define SENSOR_GREEN1_GAIN		0x2b
#define SENSOR_BLUE_GAIN		0x2c
#define SENSOR_RED_GAIN			0x2d
#define SENSOR_GREEN2_GAIN		0x2e
#define SENSOR_GLOBAL_GAIN		0x2f
#define SENSOR_ROW_NOISE		0x30

#define SENSOR_BLACK_ROWS		0x59

#define SENSOR_DARK_G1_AVERAGE		0x5b
#define SENSOR_DARK_B_AVERAGE		0x5c
#define SENSOR_DARK_R_AVERAGE		0x5d
#define SENSOR_DARK_G2_AVERAGE		0x5e
#define SENSOR_CALIB_THRESHOLD		0x5f
#define SENSOR_CALIB_CONTROL		0x60
#define SENSOR_CALIB_GREEN1		0x61
#define SENSOR_CALIB_BLUE		0x62
#define SENSOR_CALIB_RED		0x63
#define SENSOR_CALIB_GREEN2		0x64
#define SENSOR_CLOCK_CONTROL		0x65
#define SENSOR_PLL_CONTROL_1		0x66
#define SENSOR_PLL_CONTROL_2		0x67

#define SENSOR_GLOBAL_SHUTTER_CONTROL	0xc0
#define SENSOR_START_INTEGRATION	0xc1
#define SENSOR_START_READOUT		0xc2
#define SENSOR_ASSERT_STROBE		0xc3
#define SENSOR_DEASSERT_STROBE		0xc4
#define SENSOR_ASSERT_FLASH		0xc5
#define SENSOR_DEASSERT_FLASH		0xc6

#define SENSOR_EXT_SAMPLE_1		0xe0
#define SENSOR_EXT_SAMPLE_2		0xe1
#define SENSOR_EXT_SAMPLE_3		0xe2
#define SENSOR_EXT_SAMPLING_CONTROL	0xe3

#define SENSOR_BYTEWISE_ADDRESS		0xf1
#define SENSOR_CONTEXT_CONTROL		0xf2

#define SENSOR_CHIP_VERSION_2		0xff

#endif
