/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	iisreg.h

Abstract:

   	The declarations of IIS.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IIS_REG_H__
#define __IIS_REG_H__

/* IisCtrl */
#define IIS_DISA                0x00000000
#define IIS_ENA                 0x00000001

/* yc: 2006.07.24 : S */
#define IIS_XMT_DISA			0x00000000
#define IIS_XMT_ENA				0x00000002

#define IIS_RCV_DISA			0x00000000
#define IIS_RCV_ENA				0x00000004

#define IIS_EXMT_DISA           0x00000000
#define IIS_EXMT_ENA            0x00000020

/* Toby: 2013.03.19 : s*/
//#define Non_stopTx_DISA         0x00000000
//#define Non_stopTx_ENA          0x00000020

#define clk_inv_IIS_WS_inverted         0x00000000
#define clk_inv_IIS_WS_notinverted      0x00000040
/* Toby: 2013.03.19 : e*/

/* Fiis = Fsys * (A/A+1) * (1/B) */
/* re-define */
//#define IIS_CLK_DIV_A_6			0x00000600
//#define IIS_CLK_DIV_A_8			0x00000800
//#define IIS_CLK_DIV_A_12		0x00000c00
//#define IIS_CLK_DIV_A_16		0x00001000
//#define IIS_CLK_DIV_A_30        0x00001e00
//#define IIS_CLK_DIV_A_53		0x00003500  /*Peter 1109 S*/

//#define IIS_CLK_DIV_B_1			0x00000000
//#define IIS_CLK_DIV_B_4			0x00040000  /* yc: 0808 */ /*Peter 1109 S*/
//#define IIS_CLK_DIV_B_8			0x00080000
//#define IIS_CLK_DIV_B_16        0x00100000
//#define IIS_CLK_DIV_B_22		0x00160000  /*Peter 1109 S*/
//#define IIS_CLK_DIV_B_256		0x00ff0000

/* IisMode */
#define IIS_IIS_COMPATIBLE		0x00000000
#define IIS_LSB_MSB_JUSTIFIED	0x00000001

#define IIS_WS_LO_LEFT_CH		0x00000000
#define IIS_WS_LO_RIGHT_CH		0x00000002

#define OUT_DIGITIAL_FILTER_DISA    0x00000000
#define OUT_DIGITIAL_FILTER_ENA     0x00000004

#define OUT_DIGITAL_FILTER_GAINX1   0x00000000
#define OUT_DIGITAL_FILTER_GAINX2   0x00000008
#define OUT_DIGITAL_FILTER_GAINX4   0x00000010

#define OUT_DIGITAL_FILTER_FORMAT_UNSIGNED      0x00000000 
#define OUT_DIGITAL_FILTER_FORMAT_SIGNED        0x00000020

#define R_MUTE_EN_DISA                0x00000000     
#define R_MUTE_EN_ENA                 0x00000040

#define L_MUTE_EN_DISA                0x00000000   
#define L_MUTE_EN_ENA                 0x00000080  

/* IisAudFormat	*/
#define IIS_PLAY_MASK			0x00000007

#define IIS_PLAY_8b_MONO		0x00000000
#define IIS_PLAY_16b_MONO		0x00000001
#define IIS_PLAY_16b_STEREO	    0x00000002

#define IIS_PLAY_DATA_UNSIGNED  0x00000000
#define IIS_PLAY_DATA_SIGNED	0x00000004 

#define IIS_REC_MASK			0x00000038

#define IIS_REC_8b_MONO		    0x00000000
#define IIS_REC_16b_MONO		0x00000008
#define IIS_REC_16b_STEREO		0x00000010

#define IIS_REC_DATA_UNSIGNED	0x00000000
#define IIS_REC_DATA_SIGNED	    0x00000020


/* IisAdvance */
/* Toby: 2013.05.13 : s*/
#define IIS_PLAY_DATA_REP_x1    0x00000000
#define IIS_PLAY_DATA_REP_x2    0x00000001
#define IIS_PLAY_DATA_REP_x4    0x00000002

#define IIS_WS_CLk_SEL_IIS1     0x00000000
#define IIS_WS_CLk_SEL_IIS2     0x00000004
#define IIS_WS_CLk_SEL_IIS3     0x00000008
#define IIS_WS_CLk_SEL_IIS4     0x0000000C
#define IIS_WS_CLk_SEL_IIS5     0x00000010
#if (CHIP_OPTION == CHIP_A1018B) 
#define IIS_PLAY_DATA_SEL_1L1R      0x00000000
#define IIS_PLAY_DATA_SEL_2L2R      0x00000020
#define IIS_PLAY_DATA_SEL_3L3R      0x00000040
#define IIS_PLAY_DATA_SEL_4L4R      0x00000060
#define IIS_PLAY_DATA_SEL_5L5R      0x00000080
#define IIS_PLAY_DATA_SEL_1L2L      0x000000A0
#define IIS_PLAY_DATA_SEL_3L4L      0x000000C0
#define IIS_PLAY_DATA_SEL_1L3L      0x000000E0
#else
#define IIS_PLAY_DATA_SEL_1L1R      0x00000000
#define IIS_PLAY_DATA_SEL_2L2R      0x00000020
#define IIS_PLAY_DATA_SEL_1L2L      0x00000040
#define IIS_PLAY_DATA_SEL_1R2R      0x00000060
#endif
#define IIS_OP_MODE_IIS1_ENA        0x00000100
#define IIS_OP_MODE_IIS1_DISA       0x00000000
#define IIS_OP_MODE_IIS2_ENA        0x00000200
#define IIS_OP_MODE_IIS2_DISA       0x00000000
#define IIS_OP_MODE_IIS3_ENA        0x00000400
#define IIS_OP_MODE_IIS3_DISA       0x00000000
#define IIS_OP_MODE_IIS4_ENA        0x00000800
#define IIS_OP_MODE_IIS4_DISA       0x00000000

#define IIS_4CH_MUX_ENABLE       0x00008000

/* Toby: 2013.05.13 : e*/

/* IisCtrlAlt */
/* Toby: 2013.05.13 : s*/
#define IIS_EN_ALT_DIS          0x00000000
#define IIS_EN_ALT_ENA          0x00000001

#define IIS_TX_EN_ALT_DIS       0x00000000
#define IIS_TX_EN_ALT_ENA       0x00000002

#define IIS_RX_EN_ALT_DIS       0x00000000
#define IIS_RX_EN_ALT_ENA       0x00000004

#define IIS_Au_EN_ALT_DISA      0x00000000
#define IIS_Au_EN_ALT_ENA       0x00000008

#define IIS_Au_LAW_ALT_uLAW     0x00000000
#define IIS_Au_LAW_ALT_ALAW     0x00000010

#define Non_stopTx_ALT_DISA     0x00000000
#define Non_stopTx_ALT_ENA      0x00000020

#define WS_CLK_INV_ALT_Notinverted      0x00000000
#define WS_CLK_INV_ALT_inverted         0x20000000

#define MCLK_CLK_INV_ALT_Notinverted      0x00000000
#define MCLK_CLK_INV_ALT_inverted         0x80000000
/* Toby: 2013.05.13 : e*/

/* IisModeAlt */
/* Toby: 2013.05.13 : s*/
#define R_MUTE2_EN_DISA_ALT                0x00000000     
#define R_MUTE2_EN_ENA_ALT                 0x00000040

#define L_MUTE2_EN_DISA_ALT                0x00000000   
#define L_MUTE2_EN_ENA_ALT                 0x00000080 
/* Toby: 2013.05.13 : e*/

/* IisAudFormatAlt */
/* Toby: 2013.05.13 : s*/
#define IIS_PLAY_ALT_8b_MONO		    0x00000000
#define IIS_PLAY_ALT_16b_MONO		    0x00000001
#define IIS_PLAY_ALT_16b_STEREO	        0x00000002

#define IIS_PLAY_DATA_ALT_UNSIGNED      0x00000000
#define IIS_PLAY_DATA_ALT_SIGNED	    0x00000004 

#define IIS_REC_ALT_8b_MONO		        0x00000000
#define IIS_REC_ALT_16b_MONO		    0x00000008
#define IIS_REC_ALT_16b_STEREO		    0x00000010

#define IIS_REC_DATA_ALT_UNSIGNED	    0x00000000
#define IIS_REC_DATA_ALT_SIGNED	        0x00000020
/* Toby: 2013.05.13 : e*/

/* IisAdvanceAlt */
/* Toby: 2013.05.13 : s*/
#define IIS_PLAY_DATA_REP_ALT_x1        0x00000000
#define IIS_PLAY_DATA_REP_ALT_x2        0x00000001
#define IIS_PLAY_DATA_REP_ALT_x4        0x00000002

#define IIS_WS_CLk_SEL_IIS1_ALT         0x00000000
#define IIS_WS_CLk_SEL_IIS2_ALT         0x00000004
#define IIS_WS_CLk_SEL_IIS3_ALT         0x00000008
#define IIS_WS_CLk_SEL_IIS4_ALT         0x0000000C
#define IIS_WS_CLk_SEL_IIS5_ALT         0x00000010

#define IIS_PLAY_DATA_SEL_1L1R_ALT      0x00000000
#define IIS_PLAY_DATA_SEL_2L2R_ALT      0x00000020
#define IIS_PLAY_DATA_SEL_3L3R_ALT      0x00000040
#define IIS_PLAY_DATA_SEL_4L4R_ALT      0x00000060
#define IIS_PLAY_DATA_SEL_5L5R_ALT      0x00000080
#define IIS_PLAY_DATA_SEL_1L2L_ALT      0x000000A0
#define IIS_PLAY_DATA_SEL_3L4L_ALT      0x000000C0
#define IIS_PLAY_DATA_SEL_1L3L_ALT      0x000000E0

/* Toby: 2013.05.13 : e*/

/*IIS Interrupt Mask*/
#define IIS_INT_OVER_MASK              0x00010000               
#define IIS_INT_UNDER_MASK             0x00020000
#define IIS_INT_PEND_MASK              0x00040000
#define IIS_INT_WDONE_MASK             0x00080000
#define IIS_INT_RDONE_MASK             0x00100000

/*IIS Interrupt Status*/
#define IIS_INT_OVER_DONE              0x01000000               
#define IIS_INT_UNDER_DONE             0x02000000
#define IIS_INT_PEND_DONE              0x04000000
#define IIS_INT_WDONE_DONE             0x08000000
#define IIS_INT_RDONE_DONE             0x10000000

/* IisTxData */
/* IisRxData */
/* yc: 2006.07.24 : E */

/* yc: 2007.01.24 : S*/
/* AC'97Ctrl */
#define AC97_DISA						0x00000000
#define AC97_ENA						0x01000000

#define AC97_CRST						0x10000000

#define AC97_XMT_DISA					0x00000000
#define AC97_XMT_ENA					0x00000002

#define AC97_RCV_DISA					0x00000000
#define AC97_RCV_ENA					0x00000004

/* Fiis = Fsys * (A/A+1) * (1/B+1) */
#define AC97_CLK_DIV_A_1				0x00000000
#define AC97_CLK_DIV_A_18               0x00001200
#define AC97_CLK_DIV_A_25				0x00001800

#define AC97_CLK_DIV_B_1				0x00000000
#define AC97_CLK_DIV_B_2				0x00010000
#define AC97_CLK_DIV_B_5				0x00040000
#define AC97_CLK_DIV_B_256				0x00ff0000

/* AC'97Mode */
#define AC97_COMPATIBLE				    0x00000000
#define AC97_EXCESS_CLK_DIV_9_DENOM	    0x00080000
#define AC97_EXCESS_CLK_DIV_8_NUMER	    0x07000000
#define AC97_EXCESS_CLK_DIV_1_DENOM     0x00000000

#define AC97_EXCESS_CLK_DIV_8_NUMER	    0x07000000
#define AC97_EXCESS_CLK_DIV_1_NUMER     0x00000000
/* AC'97AudFormat	*/
#define AC97_PLAY_MASK					0x00000007

#define AC97_PLAY_8b_MONO				0x00000000
#define AC97_PLAY_16b_MONO			    0x00000001
#define AC97_PLAY_16b_STEREO			0x00000002

#define AC97_PLAY_DATA_UNSIGNED 		0x00000000
#define AC97_PLAY_DATA_SIGNED			0x00000004 

#define AC97_REC_MASK					0x00000038
	
#define AC97_REC_8b_MONO				0x00000000
#define AC97_REC_16b_MONO				0x00000008
#define AC97_REC_16b_STEREO			    0x00000010

#define AC97_REC_DATA_UNSIGNED		    0x00000000
#define AC97_REC_DATA_SIGNED			0x00000020

/*AC97 Interrupt Mask*/
#define AC97_INT_OVER_MASK              0x00010000               
#define AC97_INT_UNDER_MASK             0x00020000
#define AC97_INT_PEND_MASK              0x00040000
#define AC97_INT_WDONE_MASK             0x00080000
#define AC97_INT_RDONE_MASK             0x00100000

/* AC'97 CODEC Interrupt */
#define AC97_INTS_WRITE_DONE			0x08000000
#define AC97_INTS_READ_DONE			    0x10000000

/* AC'97OCC - AC'97 Output Channel Configuration Register */
#define AC97_LEFT_OUT_CH_ENA			   0x00000001
#define AC97_LEFT_OUT_CH_SAMPLE_RATE	   0x00000002
#define AC97_RIGHT_OUT_CH_ENA			   0x00000010
#define AC97_RIGHT_OUT_CH_SAMPLE_RATE 0x00000020

/* AC'97ICC - AC'97 Input Channel Configuration Register */
#define AC97_LEFT_IN_CH_ENA			   0x00000001
#define AC97_LEFT_IN_CH_SAMPLE_RATE	   0x00000002
#define AC97_RIGHT_IN_CH_ENA			   0x00000010
#define AC97_RIGHT_IN_CH_SAMPLE_RATE	   0x00000020

/* AC'97CRAC - AC'97 Codec Register Access Command */
#define AC97_READ_REGISTER_SELECT		   0x80000000
#define AC97_WRITE_REGISTER_SELECT	   0x00000000
#define AC97_READ_DATA_MASK			   0x0000ffff
#define AC97_READ_DATA_MASK_LSB		   0x0000000f

/* Define AC'97 Codec Comman Register Address (CRA) */
#define AC97_CRA_RESET                 0x00000000
#define AC97_CRA_LINEOUT               0x00020000
#define AC97_CRA_HEADPHONE			   0x00040000
#define AC97_CRA_PCM_OUT_VOLUME		   0x00180000
#define AC97_CRA_EXT_AUD_STA_CON		   0x002a0000
#define AC97_CRA_PCM_DAC_RATE	                 0x002c0000
#define AC97_CRA_PCM_ADC_RATE		   	   0x00320000
#define AC97_CRA_LINE_IN_VOLUME		   0x00100000
#define AC97_CRA_REC_GAIN_STEREO_ADC	   0x001c0000
#define AC97_CRA_REC_SELECT			   0x001a0000
#define AC97_CRA_POWERDOWN             0x00260000

/* Define AC'97 Codec MX04 Headphone (HP) Control Bits */
#define AC97_HP_NORMAL	    			         0x00000000
#define AC97_HP_MUTE	          	   		  0x00008000

#define AC97_HP_LEFT_VOL_0_dB_ATT    	  0x00000000    // ATT: Attenuation
#define AC97_HP_LEFT_VOL_3_dB_ATT     	  0x00000200
#define AC97_HP_LEFT_VOL_6_dB_ATT     	  0x00000400
#define AC97_HP_LEFT_VOL_9_dB_ATT     	  0x00000600
#define AC97_HP_LEFT_VOL_94_5_dB_ATT 	  0x00003f00

#define AC97_HP_RIGHT_VOL_0_dB_ATT    	  0x00000000    // ATT: Attenuation
#define AC97_HP_RIGHT_VOL_3_dB_ATT     	  0x00000002
#define AC97_HP_RIGHT_VOL_6_dB_ATT     	  0x00000004
#define AC97_HP_RIGHT_VOL_9_dB_ATT     	  0x00000006
#define AC97_HP_RIGHT_VOL_94_5_dB_ATT 0x0000003f

/* Define AC'97 Codec MX18 PCM_OUT Volume Control Bits */
#define AC97_PCM_OUT_NORMAL	    		  0x00000000
#define AC97_PCM_OUT_MUTE		    		  0x00008000

#define AC97_PCM_LEFT_VOL_12_dB_GAIN   0x00000000
#define AC97_PCM_LEFT_VOL_9_dB_GAIN     0x00000200
#define AC97_PCM_LEFT_VOL_6_dB_GAIN     0x00000400
#define AC97_PCM_LEFT_VOL_3_dB_GAIN     0x00000600
#define AC97_PCM_LEFT_VOL_0_dB_GAIN     0x00000800
#define AC97_PCM_LEFT_VOL_3_dB_ATT  	  0x00000a00	// ATT: Attenuation
#define AC97_PCM_LEFT_VOL_6_dB_ATT  	  0x00000c00	// ATT: Attenuation
#define AC97_PCM_LEFT_VOL_34_5_dB_ATT 0x00001f00	// ATT: Attenuation

#define AC97_PCM_RIGHT_VOL_12_dB_GAIN   0x00000000
#define AC97_PCM_RIGHT_VOL_9_dB_GAIN     0x00000002
#define AC97_PCM_RIGHT_VOL_6_dB_GAIN     0x00000004
#define AC97_PCM_RIGHT_VOL_3_dB_GAIN     0x00000006
#define AC97_PCM_RIGHT_VOL_0_dB_GAIN     0x00000008
#define AC97_PCM_RIGHT_VOL_3_dB_ATT  	    0x0000000a	// ATT: Attenuation
#define AC97_PCM_RIGHT_VOL_6_dB_ATT  	    0x0000000c	// ATT: Attenuation
#define AC97_PCM_RIGHT_VOL_34_5_dB_ATT 0x0000001f	// ATT: Attenuation

/* Define AC'97 Codec MX2A Extended Audio Status and Control Bits */
#define AC97_EXT_AUD_STA_CON_VRA_ENA   0x00000001
#define AC97_EXT_AUD_STA_CON_VRA_DISA  0x00000000

/* Define AC'97 Codec MX2C PCM DAC Rate Control Bits */
#define AC97_PCM_DAC_RATE_8K_HZ	    	  0x00001f40
#define AC97_PCM_DAC_RATE_16K_HZ	    	  0x00003e80

/* Define AC'97 Codec MX2C PCM ADC Rate Control Bits */
#define AC97_PCM_ADC_RATE_8K_HZ	    	  0x00001f40
#define AC97_PCM_ADC_RATE_16K_HZ	    	  0x00003e80

/* Define AC'97 Codec MX10 Line In Volume Control Bits */
#define AC97_LINE_IN_NORMAL				0x00000000
#define AC97_LINE_IN_Mute					0x00008000

#define AC97_LINE_IN_LEFT_VOL_0dB_GAIN	0x00000800
#define AC97_LINE_IN_LEFT_VOL_6dB_GAIN	0x00000400
#define AC97_LINE_IN_LEFT_VOL_9dB_GAIN	0x00000200
#define AC97_LINE_IN_LEFT_VOL_12dB_GAIN	0x00000000

#define AC97_LINE_IN_RIGHT_VOL_0dB_GAIN	0x00000008
#define AC97_LINE_IN_RIGHT_VOL_6dB_GAIN	0x00000004
#define AC97_LINE_IN_RIGHT_VOL_9dB_GAIN	0x00000002
#define AC97_LINE_IN_RIGHT_VOL_12dB_GAIN	0x00000000

/* Define AC'97 Codec MX1C Record Gain for Stereo ADC Control Bits */
#define AC97_REC_ST_ADC_NORMAL			0x00000000
#define AC97_REC_ST_ADC_MUTE				0x00008000

#define AC97_REC_ST_ADC_LEFT_0dB_GAIN	0x00000000 
#define AC97_REC_ST_ADC_LEFT_3dB_GAIN	0x00000200 
#define AC97_REC_ST_ADC_LEFT_6dB_GAIN	0x00000400
#define AC97_REC_ST_ADC_LEFT_9dB_GAIN	0x00000600 
#define AC97_REC_ST_ADC_LEFT_12dB_GAIN	0x00000800
#define AC97_REC_ST_ADC_LEFT_15dB_GAIN	0x00000a00
#define AC97_REC_ST_ADC_LEFT_18dB_GAIN	0x00000c00
#define AC97_REC_ST_ADC_LEFT_21dB_GAIN	0x00000e00 

#define AC97_REC_ST_ADC_RIGHT_0dB_GAIN	0x00000000
#define AC97_REC_ST_ADC_RIGHT_3dB_GAIN	0x00000002
#define AC97_REC_ST_ADC_RIGHT_6dB_GAIN	0x00000004
#define AC97_REC_ST_ADC_RIGHT_9dB_GAIN	0x00000006
#define AC97_REC_ST_ADC_RIGHT_12dB_GAIN	0x00000008 
#define AC97_REC_ST_ADC_RIGHT_15dB_GAIN	0x0000000a
#define AC97_REC_ST_ADC_RIGHT_18dB_GAIN	0x0000000c
#define AC97_REC_ST_ADC_RIGHT_21dB_GAIN	0x0000000e

/* Define AC'97 Codec MX1A Record Select */
#define AC97_REC_SELECT_LEFT_MIC			0x00000000
#define AC97_REC_SELECT_LEFT_LINE			0x00000400

#define AC97_REC_SELECT_RIGHT_MIC			0x00000000
#define AC97_REC_SELECT_RIGHT_LINE		0x00000004

/* yc: 2007.01.24 : E*/

#endif
