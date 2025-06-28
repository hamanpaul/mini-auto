/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	i2creg.h

Abstract:

   	The registers of I2C controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __I2C_REG_H__
#define __I2C_REG_H__

/* I2cCtrl */
#define I2C_TRIG        0x00000001
#define I2C_ENA         0x00000002
#define I2C_WAIT_VSYNC      0x00000004
#define I2C_RD_REG_ADDR     0x00000008

#define I2C_1B          0x00000000
#define I2C_2B          0x00000010
#define I2C_3B          0x00000020
#define I2C_4B          0x00000030
#define I2C_5B          0x04000000
#define I2C_6B          0x04000010
#define I2C_7B          0x04000020
#define I2C_8B          0x04000030

#define I2C_CLK_DIV_60      0x00000000
#define I2C_CLK_DIV_120     0x00000040
#define I2C_CLK_DIV_240     0x00000080
#define I2C_CLK_DIV_480     0x000000c0
#define I2C_CLK_DIV_960     0x02000000
#define I2C_CLK_DIV_1920    0x02000040
#define I2C_CLK_DIV_3840    0x02000080
#define I2C_CLK_DIV_7680    0x020000c0

#define I2C_SLAV_ADDR_SHFT  8
#define I2C_SUB_ADDR_SHFT   16

#define I2C_INT_ENA     0x01000000

#define I2C_SENS_CLK_DIV_MASK   0x06000000
#define I2C_SENS_CLK_DIV_SHFT   25

#define I2C_SENS_CLK_DIV_1  0x00000000
#define I2C_SENS_CLK_DIV_2  0x02000000
//#define I2C_SENS_CLK_DIV_10	0x04000000
#define I2C_SENS_CLK_DIV_10	0x02000000
#define I2C_SENS_CLK_DIV_30 0x06000000

#define I2C_R_ACK       0x08000000
#define I2C_MANUAL_EN   0x20000000
#define I2C_ACK         0x40000000
#define I2C_BUSY        0x80000000

/* I2cData */
#define I2C_DATA3_SHFT      24
#define I2C_DATA2_SHFT      16
#define I2C_DATA1_SHFT      8
#define I2C_DATA0_SHFT      0

/* I2cManu */
#define I2C_ManuSBIT        0x00000001
#define I2C_ManuSlaveaddr   0x00000002
#define I2C_ManuSubaddr     0x00000004
#define I2C_ManuWriteData1  0x00000008
#define I2C_ManuWriteData2  0x00000010
#define I2C_ManuWriteData3  0x00000020
#define I2C_ManuWriteData4  0x00000040
#define I2C_ManuPBIT        0x00000080
#define I2C_ManuReadData    0x00000100
#define I2C_ManuAck2Bus     0x00000200


#endif
