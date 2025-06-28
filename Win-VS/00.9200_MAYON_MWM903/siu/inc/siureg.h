/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	siureg.h

Abstract:

   	The declarations of Sensor Interface Unit registers.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SIU_REG_H__
#define __SIU_REG_H__

/* SiuRawAddr */
	
/* SiuSensCtrl*/
#define SIU_NORMAL			0x00000000
#define SIU_RESET			0x00000001

#define SIU_DISA			0x00000000
#define SIU_ENA				0x00000002

#define SIU_SLAVE			0x00000000
#define SIU_MASTER			0x00000010

#define SIU_VSYNC_ACT_LO		0x00000000
#define SIU_VSYNC_ACT_HI		0x00000020

#define SIU_HSYNC_ACT_LO		0x00000000
#define SIU_HSYNC_ACT_HI		0x00000040

#define SIU_DEF_PIX_DISA		0x00000000
#define SIU_DEF_PIX_ENA			0x00000080

#define SIU_INT_DISA_FIFO_OVERF		0x00000000
#define SIU_INT_ENA_FIFO_OVERF		0x00000100

#define SIU_INT_DISA_FRAM		0x00000000
#define SIU_INT_ENA_FRAM		0x00000200

#define SIU_INT_DISA_16LINE		0x00000000
#define SIU_INT_ENA_16LINE		0x00000008

#define SIU_INT_DISA_8LINE		0x00000000
#define SIU_INT_ENA_8LINE		0x00000400

#define SIU_INT_DISA_LINE		0x00000000
#define SIU_INT_ENA_LINE		0x00000800

#define SIU_INT_DISA_DEF_PIX_FAIL	0x00000000
#define SIU_INT_ENA_DEF_PIX_FAIL	0x00000000

#define SIU_DST_SRAM			0x00000000
#define SIU_DST_SDRAM			0x00001000

#define SIU_CAPT_DISA			0x00000000
#define SIU_CAPT_ENA			0x00002000

#define SIU_PDSEL_1x1           0x00000000   // new function for PA9003, other(PA9002D) has bug inside.
#define SIU_PDSEL_3x3           0x00004000
#define SIU_PDSEL_5x5           0x00008000



#define SIU_DATA_12b			0x00000000
#define SIU_DATA_11b			0x00010000
#define SIU_DATA_10b			0x00020000
#define SIU_DATA_9b			0x00030000
#define SIU_DATA_8b			0x00040000

#define SIU_PIX_CLK_ACT_FAL		0x00000000
#define SIU_PIX_CLK_ACT_RIS		0x00080000

#define SIU_FRAM_VSYNC			0x00000000
#define SIU_FRAM_DATA_END		0x00100000   //此設定於PA9002D 會有bug(SIU intr 消失) 產生,不建議使用. PA9003 已修正

#define SIU_OB_COMP_DISA		0x00000000
#define SIU_OB_COMP_ENA			0x00200000   

#define SIU_LENS_SHAD_DISA		0x00000000
#define SIU_LENS_SHAD_ENA		0x00400000

#define SIU_RGB_GAMMA_DISA		0x00000000
#define SIU_RGB_GAMMA_ENA		0x00800000

#define SIU_AE_DISA			0x00000000
#define SIU_AE_ENA			0x01000000

#define SIU_TEST_PATRN_DISA		0x00000000
#define SIU_TEST_PATRN_1		0x02000000
#define SIU_TEST_PATRN_2		0x04000000
#define SIU_TEST_PATEN_3        0x06000000

#define SIU_FRANME_SCALING_DISA		0x00000000
#define SIU_FRANME_SCALING_ENA		0x08000000

#define SIU_DO_THD_4WORD            0x00000000
#define SIU_DO_THD_8WORD            0x40000000

#define SIU_SINGLE_CAPTURE_ENA		0x80000000
#define SIU_SINGLE_CAPTURE_DISA		0x00000000

/* SiuIntStat */
#define SIU_INT_STAT_FIFO_OVERF		0x00000001
#define SIU_INT_STAT_FRAM		    0x00000002
#define SIU_INT_STAT_8LINE          0x00000004
#define SIU_INT_STAT_LINE		    0x00000008
#define SIU_INT_STAT_16LINE		    0x00000010

#define SIU_INT_STAT_CCIR_BOT_END   0x00000020
#define SIU_INT_STAT_CCIR_TOP_STR   0x00000040
#define SIU_INT_STAT_FRAME_STR      0x00000080

#define SIU_INT_STAT_DEF_PIX_FAIL	0x80000000
/* SiuSyncStat */
#define SIU_STAT_VSYNC			0x00000001
#define SIU_STAT_HSYNC			0x00000002

#define SIU_LINE_NUM_MASK		0x0fff0000
#define SIU_LINE_NUM_SHFT		16

/* SiuValidSize */
#define SIU_VALID_SIZE_X_SHFT		0
#define SIU_VALID_SIZE_Y_SHFT		16
 
/* SiuTotalSize */
#define SIU_TOTAL_SIZE_X_SHFT		0
#define SIU_TOTAL_SIZE_Y_SHFT		16

/* SiuOb */
#define SIU_OB_B_MASK			0x000000ff
#define SIU_OB_B_SHFT			0
#define SIU_OB_Gb_MASK			0x0000ff00
#define SIU_OB_Gb_SHFT			8
#define SIU_OB_Gr_MASK			0x00ff0000
#define SIU_OB_Gr_SHFT			16
#define SIU_OB_R_MASK			0xff000000
#define SIU_OB_R_SHFT			24

/* SiuBGbGain */
#define SIU_GAIN_B_MASK			0x0000ffff
#define SIU_GAIN_B_SHFT			0
#define SIU_GAIN_Gb_MASK		0xffff0000
#define SIU_GAIN_Gb_SHFT		16

/* SiuRGrGain */
#define SIU_GAIN_R_MASK			0x0000ffff
#define SIU_GAIN_R_SHFT			0
#define SIU_GAIN_Gr_MASK		0xffff0000
#define SIU_GAIN_Gr_SHFT		16

/* SiuLensCornerX2 */

/* SiuLensCornerY2 */

/* SiuLensCentOffs */
#define SIU_LENS_CENT_X_SHFT		0
#define SIU_LENS_CENT_Y_SHFT		16

/* SiuLensCompGain */
#define SIU_LC_CORNER_GAIN_SHFT		0

#define SIU_LC_RTYPE_SK7		0x00000000
#define SIU_LC_RTYPE_SK8		0x00010000
#define SIU_LC_RTYPE_SK9		0x00020000
#define SIU_LC_RTYPE_SK10		0x00030000

#define SIU_LC_SCAL_1_1			0x00000000
#define SIU_LC_SCAL_1_4			0x00100000
#define SIU_LC_SCAL_1_16		0x00200000

#define SIU_LC_OUT_INVERT_DISA		0x00000000
#define SIU_LC_OUT_INVERT_ENA		0x01000000

/* SiuDefPixTbl */ 


#define SIU_B_GAMMA_X_SHFT		0
#define SIU_B_GAMMA_Y_SHFT		16

/* SiuAeWinStart */
#define SIU_AE_WIN_START_X_SHFT		0
#define SIU_AE_WIN_START_Y_SHFT		16

/* SiuAeWinSize */
#define SIU_AE_WIN_SIZE_X_SHFT		0
#define SIU_AE_WIN_SIZE_Y_SHFT		16

/* SiuAeCtrl */
#define SIU_AE_LO_BOND_SHFT		0
#define SIU_AE_HI_BOND_SHFT		8

#define SIU_AE_SCAL_1_4			0x00000000
#define SIU_AE_SCAL_1_16		0x00010000
#define SIU_AE_SCAL_1_64		0x00020000
#define SIU_AE_SCAL_1_256		0x00030000	

/* SiuAeWin0_1 */
/* SiuAeWin2_3 */
/* SiuAeWin4_5 */
/* SiuAeWin6_7 */
/* SiuAeWin8_9 */
/* SiuAeWin10_11 */
/* SiuAeWin12_13 */
/* SiuAeWin14_15 */
/* SiuAeWin16_17 */
/* SiuAeWin18_19 */
/* SiuAeWin20_21 */
/* SiuAeWin22_23 */
/* SiuAeWin24 */
#define SIU_AE_SUM_WIN_EVEN_MASK	0x00000fff
#define SIU_AE_SUM_WIN_EVEN_SHFT	0
#define SIU_AE_WGT_WIN_EVEN_MASK	0x0000f000
#define SIU_AE_WGT_WIN_EVEN_SHFT	12

#define SIU_AE_SUM_WIN_ODD_MASK		0x0fff0000
#define SIU_AE_SUM_WIN_ODD_SHFT		16
#define SIU_AE_WGT_WIN_ODD_MASK		0xf0000000
#define SIU_AE_WGT_WIN_ODD_SHFT		28

/* SiuAeRBondOut */
/* SiuAeGBondOut */
/* SiuAeBBondOut */
/* SiuAeRBondMid */
/* SiuAeGBondMid */
/* SiuAeBBondMid */
/* SiuAeRBondInn */
/* SiuAeGBondInn */
/* SiuAeBBondInn */
#define SIU_AE_LO_BOND_HIST_MASK	0x0000ffff
#define SIU_AE_LO_BOND_HIST_SHFT	0
#define SIU_AE_HI_BOND_HIST_MASK	0xffff0000
#define SIU_AE_HI_BOND_HIST_SHFT	16

/* SiuAeWgtSum */


/*SIU debug  offset: 0x0110*/
#define SIU_BT1120_MODE        0x00000008 
#define SIU_INSEL_CCIR601      0x00004000   
#define SIU_INSEL_CCIR656      0x00002000
#define SIU_INSEL_BAYERRAW     0x00000000

#define SIU_DEINTERLACE_ENA    0x00008000
#define SIU_DEINTERLACE_DISA   0x00000000


#define SIU_656_DATALATCH_RIS  0x00000000
#define SIU_656_DATALATCH_FAL  0x08000000

#define SIU_YUVMAP_36          0x00360000
#define SIU_YUVMAP_27          0x00270000
#define SIU_YUVMAP_72          0x00720000
#define SIU_YUVMAP_87          0x00870000
#define SIU_YUVMAP_2D          0x002d0000
#define SIU_YUVMAP_4D          0x004d0000
#define SIU_YUVMAP_63          0x00630000
#define SIU_YUVMAP_78          0x00780000
#define SIU_YUVMAP_C9          0x00c90000

#define SIU_4GAMMATab_EN       0x01000000
#define SIU_4GAMMATab_DIS      0x00000000

#define SIU_BOTFD_END_INT_ENA  0x02000000
#define SIU_BOTFD_STR_INT_DISA 0x00000000

#define SIU_FIELD_POL_POS      0x00000000
#define SIU_FIELD_POL_NEG      0x04000000

#define SIU_TOPFD_STR_INT_ENA  0x10000000
#define SIU_TOPFD_STR_INT_DISA 0x00000000

#define SIU_FRAME_STR_INT_EN   0x40000000
#define SIU_FRAME_STR_INT_DISA 0x00000000

/*SiuPreGammaGain*/
#define SIU_R_GAIN_SHFT         0
#define SIU_Gr_GAIN_SHFT        8
#define SIU_Gb_GAIN_SHFT        16
#define SIU_B_GAIN_SHFT         24

#endif
