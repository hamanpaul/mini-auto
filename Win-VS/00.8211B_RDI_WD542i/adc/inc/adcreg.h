/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	adcreg.h

Abstract:

   	The declarations of ADC.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __ADC_REG_H__
#define __ADC_REG_H__

/* AdcRxCtrl */

#define ADC_PWON                0x00000001
#define ADC_PWOFF               0x00000000
#define ADC_RX_8bit             0x00000000
#define ADC_RX_16bit            0x00000002
#define ADC_RX_INVERTED         0x00000040
#define ADC_REC_CONV_ENA        0x00000080
#define ADC_RX_BYPASS           0x00000000
#define ADC_RX_ENA              0x00000004
#define ADC_RX_DISA             0x00000000
#define ADC_REF_TC_0PERCENT     0x00000080
#define ADC_CLK_DIV_A_0         0x00000000
#define ADC_CLK_DIV_A_2         0x00000200
#define ADC_CLK_DIV_A_3         0x00000300
#define ADC_CLK_DIV_A_4         0x00000400
#define ADC_CLK_DIV_A_24        0x00001800
#define ADC_CLK_DIV_A_33        0x00002100
#define ADC_CLK_DIV_A_53        0x00003500
#define ADC_CLK_DIV_A_65        0x00004100
#define ADC_CLK_DIV_A_128       0x00008000
#define ADC_CLK_DIV_A_227       0x0000E300
#define ADC_CLK_DIV_A_254       0x0000FE00

#define ADC_CLK_DIV_B_0         0x00000000
#define ADC_CLK_DIV_B_1         0x00010000
#define ADC_CLK_DIV_B_2         0x00020000
#define ADC_CLK_DIV_B_3         0x00030000
#define ADC_CLK_DIV_B_4         0x00040000
#define ADC_CLK_DIV_B_5         0x00050000
#define ADC_CLK_DIV_B_7         0x00070000
#define ADC_CLK_DIV_B_9         0x00090000
#define ADC_CLK_DIV_B_22        0x00160000
#define ADC_CLK_DIV_B_24        0x00180000
#define ADC_CLK_DIV_B_39        0x00270000
#define ADC_CLK_DIV_B_59        0x003B0000
#define ADC_CLK_DIV_B_69        0x00450000
#define ADC_CLK_DIV_B_74        0x004A0000
#define ADC_CLK_DIV_B_82        0x00520000

#define ADC_PWD_PGA_ENA         0x00000000
#define ADC_PWD_PGA_DISA        0x01000000

#define ADC_PGINM_20DB          0x04000000
#define ADC_PGINM_10DB          0x02000000
#define ADC_PGINM_0DB           0x00000000

#define ADC_PGINL_21DB          0x38000000
#define ADC_PGINL_18DB          0x30000000
#define ADC_PGINL_15DB          0x28000000
#define ADC_PGINL_12DB          0x20000000
#define ADC_PGINL_9DB           0x18000000
#define ADC_PGINL_6DB           0x10000000
#define ADC_PGINL_3DB           0x08000000
#define ADC_PGINL_0DB           0x00000000

#define ADC_REC_CH0             0x00000000
#define ADC_REC_CH1             0x40000000
#define ADC_REC_CH2             0x80000000
#define ADC_REC_CH3             0xC0000000

/*** 9003 ***/
#define ADC_REC_G0              0x00000004
#define ADC_REC_G1              0x00000008
#define ADC_REC_G2              0x00000010
#define ADC_REC_G3              0x00000020

#define ADC_REC_G0_CH0          0x00000000
#define ADC_REC_G0_CH1          0x01000000
#define ADC_REC_G0_CH2          0x02000000
#define ADC_REC_G0_CH3          0x03000000
#define ADC_REC_G1_CH0          0x00000000
#define ADC_REC_G1_CH1          0x04000000
#define ADC_REC_G1_CH2          0x08000000
#define ADC_REC_G1_CH3          0x0C000000
#define ADC_REC_G2_CH0          0x00000000
#define ADC_REC_G2_CH1          0x10000000
#define ADC_REC_G2_CH2          0x20000000
#define ADC_REC_G2_CH3          0x30000000
#define ADC_REC_G3_CH0          0x00000000
#define ADC_REC_G3_CH1          0x40000000
#define ADC_REC_G3_CH2          0x80000000
#define ADC_REC_G3_CH3          0xC0000000

#define ADC_REC_CH_0            0x00000000
#define ADC_REC_CH_1            0x00000001
#define ADC_REC_CH_2            0x00000002
#define ADC_REC_CH_3            0x00000003
#define ADC_REC_CH_4            0x00000004
#define ADC_REC_CH_5            0x00000005
#define ADC_REC_CH_6            0x00000006
#define ADC_REC_CH_7            0x00000007
#define ADC_REC_CH_8            0x00000008
#define ADC_REC_CH_9            0x00000009
#define ADC_REC_CH_10           0x0000000A
#define ADC_REC_CH_11           0x0000000B
#define ADC_REC_CH_12           0x0000000C
#define ADC_REC_CH_13           0x0000000D
#define ADC_REC_CH_14           0x0000000E
#define ADC_REC_CH_15           0x0000000F
#define ADC_REC_CH_16           0x00000010

#define ADC_REC_CONV_RATE_5     0x00000005
#define ADC_REC_CONV_RATE_7     0x00000007
#define ADC_REC_CONV_RATE_9     0x00000009
#define ADC_REC_CONV_RATE_14    0x0000000E
#define ADC_REC_CONV_RATE_15    0x0000000F
#define ADC_REC_CONV_RATE_17    0x00000011
#define ADC_REC_CONV_RATE_19    0x00000013
#define ADC_REC_CONV_RATE_21    0x00000015
#define ADC_REC_CONV_RATE_23    0x00000017
#define ADC_REC_CONV_RATE_24    0x00000018
#define ADC_REC_CONV_RATE_26    0x0000001A
#define ADC_REC_CONV_RATE_29    0x0000001D
#define ADC_REC_CONV_RATE_30    0x0000001E
#define ADC_REC_CONV_RATE_31    0x0000001F
#define ADC_REC_CONV_RATE_44    0x0000002C
#define ADC_REC_CONV_RATE_47    0x0000002F
#define ADC_REC_CONV_RATE_53    0x00000035
#define ADC_REC_CONV_RATE_59    0x0000003B
#define ADC_REC_CONV_RATE_66    0x00000042
#define ADC_REC_CONV_RATE_74    0x0000004A
#define ADC_REC_CONV_RATE_79    0x0000004F
#define ADC_REC_CONV_RATE_95    0x0000005F

#define ADC_BUS_SFT_0               0x00000000
#define ADC_BUS_SFT_1               0x00010000
#define ADC_BUS_SFT_2               0x00020000
#define ADC_BUS_SFT_FRACTION_1000   0x00000000
#define ADC_BUS_SFT_FRACTION_1125   0x00040000
#define ADC_BUS_SFT_FRACTION_1250   0x00080000
#define ADC_BUS_SFT_FRACTION_1375   0x000C0000
#define ADC_BUS_SFT_FRACTION_1500   0x00100000
#define ADC_BUS_SFT_FRACTION_1625   0x00140000
#define ADC_BUS_SFT_FRACTION_1750   0x00180000
#define ADC_BUS_SFT_FRACTION_1875   0x001C0000
#define ADC_AVG_G0_ENA              0x00000001
#define ADC_AVG_G1_ENA              0x00000002
#define ADC_AVG_G2_ENA              0x00000004
#define ADC_AVG_G3_ENA              0x00000008

/* DacTxCtrl */

#if(CHIP_OPTION <= CHIP_PA9002D)
#define DAC_PWON                0x00000000
#define DAC_PWOFF               0x00000001
#define DAC_MUTE_ENA            0x00000002
#define DAC_MUTE_DISA           0x00000000
#define DAC_MODE_BINARY         0x00000004
#define DAC_MODE_COMPLEMENT     0x00000000
#define DAC_PGO_0DB		        0x00000000
#define DAC_PGO_2_5DB		    0x00000010
#define DAC_PGO_5DB		        0x00000020
#define DAC_PGO_7_5DB		    0x00000030
#define DAC_PGO_10DB		    0x00000040
#define DAC_PGO_20DB		    0x00000080
#elif((CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || \
	(CHIP_OPTION == CHIP_A1026A))
#define DAC_ENVREF_ON           0x00000001
#define DAC_ENVREF_OFF          0x00000000
#define DAC_VREFSM_WEAK         0x00000002
#define DAC_VREFSM_STRONG       0x00000000
#define DAC_ENMICBIAS_ON        0x00000004
#define DAC_ENMICBIAS_OFF       0x00000000
#define DAC_ENMIC_ON            0x00000008
#define DAC_ENMIC_OFF           0x00000000
#define DAC_ENBST_ON            0x00000010
#define DAC_ENBST_OFF           0x00000000
#define DAC_PGAG_MAX            0x00000000
#define DAC_PGAG_MUTE           0x00001f00
#define DAC_ENZCD_ON            0x00002000
#define DAC_ENZCD_OFF           0x00000000
#define DAC_ZCDHS_ON            0x00004000
#define DAC_ZCDHS_OFF           0x00000000
#define DAC_TMAD_SERIAL         0x00008000
#define DAC_TMAD_PARALLEL       0x00000000
#define DAC_ENDAR_ON            0x00010000
#define DAC_ENDAR_OFF           0x00000000
#define DAC_ENDAL_ON            0x00020000
#define DAC_ENDAL_OFF           0x00000000
#define DAC_PWON                0x00030000
#define DAC_PWOFF               0x00000000
#define DAC_DARG_MAX            0x00000000
#define DAC_DARG_MUTE           0x007C0000
#define DAC_DALG_MAX            0x00000000
#define DAC_DALG_MUTE           0x0f800000
#define DAC_MUTE_ENA            0x0ffc0000
#define DAC_MUTE_DISA           0x00000000
#define DAC_TADDA_NORMAL        0x00000000
#define DAC_TADDA_MICAMP        0x10000000
#define DAC_TADDA_PGA           0x20000000
#define DAC_TADDA_AAF           0x30000000
#define DAC_TADDA_ADC           0x40000000
#define DAC_TADDA_DAC           0x50000000
#define DAC_TADDA_OLC           0x60000000

#endif

#endif
