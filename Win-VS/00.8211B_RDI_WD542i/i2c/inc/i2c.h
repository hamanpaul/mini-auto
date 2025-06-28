/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	i2c.h

Abstract:

   	The declarations of I2C bus protocol.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __I2C_H__
#define __I2C_H__


/** Configure Bits */
typedef enum {
    C_MEAS_TEMP0,     ///< Measure TEMP0
    C_MEAS_VBAT1,     ///< Measure VBAT1
    C_MEAS_IN1,       ///< Measure IN1
    // 3 is Reversed
    C_MEAS_TEMP1 = 4, ///< Measure TEMP1
    C_MEAS_VBAT2,     ///< Measure VBAT2
    C_MEAS_IN2,       ///< Measure IN2
    // 7 is Reversed
    C_ACTIVATE_NX_DRIVERS = 8,    ///< Activate X- Drivers
    C_ACTIVATE_NY_DRIVERS,        ///< Activate Y- Drivers
    C_ACTIVATE_YNX_DRIVERS,       ///< Activate Y+, X- Drivers
    // 11 is Reversed
    C_MEAS_X = 12,    ///< Measure X Position [v]
    C_MEAS_Y,         ///< Measure Y Position [v]
    C_MEAS_Z1,        ///< Measure Z1 Position
    C_MEAS_Z2,        ///< Measure Z2 Position
} ConfigureBits;


/** Power Down Bits */
typedef enum {
    PD_POWERDOWN = 0,       ///< penirq; power-down between conversions
    PD_IREFOFF_ADCON = 1,   ///< no penirq; internal reference OFF, ADC ON
    PD_IREFON_ADCOFF = 2,   ///< penirq; internal reference ON, ADC OFF
    PD_IREFON_ADCON = 3,    ///< no penirq; internal reference ON, ADC ON [v]
} PowerDownBits;


/** Mode Bit */
typedef enum {
    M_12BIT,    ///< 12-bit mode [v]
    M_8BIT      ///< 8-bit mode
} ModeBit;

#if(TV_DECODER == WT8861)
#define	VIDEO_525L60HZ		0
#define	VIDEO_625L50HZ		1
#define	VIDEO_PLA_COLOUR		0 
#define	VIDEO_SECAM_COLOUR	1 
#endif
#define TSC2003_ADC_MAX ((1 << 12) - 1)

/** Assembly a Command Byte
 * c -- configure bits
 * pd -- power-down bits
 * m -- mode bit
 */
#define TSC2003_CMD(c, pd, m) (((c) << 4) | ((pd) << 2) | ((m) << 1))



#endif
