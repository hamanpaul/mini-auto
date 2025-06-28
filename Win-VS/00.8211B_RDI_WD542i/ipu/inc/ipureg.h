/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ipureg.h

Abstract:

   	The declarations of Image Processing Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IPU_REG_H__
#define __IPU_REG_H__


/*IPU Engine enable*/
#define IPUEE_RST     0x00000001
#define IPUEE_EN      0x00000002
#define IPUEE_BUSRST  0x00000004

/*IPU Mode Control*/
#define IPUMCTL_FRAMMODE 0x00000001
#define IPUMCTL_BLKMODE  0x00000000
#define IPUMCTL_CAPMODE  0x00000002
#define IPUMCTL_PREVMODE 0x00000000
#define IPUMCTL_BLK_16   0x00000004    
#define IPUMCTL_BLK_8    0x00000000

/*IPU Function Enable*/
#define IPUFE_EGEN      0x000000001
#define IPUFE_DEEN      0x000000002
#define IPUFE_FCEN      0x000000004
#define IPUFE_YUVGAEN   0x000000008

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define IPUSRAM5K_EN    0x000000010
#elif(CHIP_OPTION == CHIP_PA9002D)

#else //PA9001D
#define IPUFE_AFREN     0x000000010
#define IPUFE_YREN      0x000000020
#define IPUFE_FTC_INTE  0x000000040
#define IPUFE_BLK_INTE  0x000000080
#define IPUFE_RGBGAEN   0x000000100
#endif

/* CFA RB gain */
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define IPU_R_GAIN_SHFT    0
#define IPU_B_GAIN_SHFT    16
#endif


/*  IPU Image Input and Output Size  */
#define IPU_SIZE_X_SHFT			0
#define IPU_SIZE_Y_SHFT			16

/* IpuCfaInterp */
#define IPU_CFAI_EDG_GTHRESH_SHFT	0
#define IPU_CFAI_EDG_SMOOTHHUE_SHFT	8
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define IPU_HSW_SHFT	16
#endif

/* IpuColorCorrMatrx1_2 */
/* IpuColorCorrMatrx3_4 */
/* IpuColorCorrMatrx5_6 */
/* IpuColorCorrMatrx7_8 */
/* IpuColorCorrMatrx9 */
#define IPU_CCM_NEGATIVE		0x00000400
#define IPU_CCM_EVEN_SHFT		0
#define IPU_CCM_ODD_SHFT		16

/* IpuEdgEnhance1 */
#define IPU_EDG_ENH_THRESH_L_SHFT	0
#define IPU_EDG_ENH_CURVSLOP1x32_SHFT	8
#define IPU_EDG_ENH_ADDOFFS1_SHFT	16
#define IPU_EDG_ENH_THRESH_CORNR1_SHFT	24
		
/* IpuEdgEnhance2 */
#define IPU_EDG_ENH_CURVSLOP2x32_SHFT	0
#define IPU_EDG_ENH_ADDOFFS2_SHFT	8
#define IPU_EDG_ENH_THRESH_CORNR2_SHFT	16
#define IPU_EDG_ENH_CURVSLOP3x32_SHFT	24

/* IpuEdgEnhance3 */
#define IPU_EDG_ENH_THRESH_H		0

#define IPU_EDG_ENH_DISA		0x00000000
#define IPU_EDG_ENH_ENA			0x00000100
//lisa_2008_0408_S YUVGamma 
#define IPU_YUV_GAMMA_Gain0_SHFT	0
#define IPU_YUV_GAMMA_Gain1_SHFT	8
#define IPU_YUV_GAMMA_Gain2_SHFT	16
#define IPU_YUV_GAMMA_Gain3_SHFT	24

#define IPU_YUV_GAMMA_SHFT_Y0	0
#define IPU_YUV_GAMMA_SHFT_Y1	4
#define IPU_YUV_GAMMA_SHFT_Y2	8
#define IPU_YUV_GAMMA_SHFT_Y3	12
#define IPU_YUV_GAMMA_SHFT_Y4	16
#define IPU_YUV_GAMMA_SHFT_Y5	20
#define IPU_YUV_GAMMA_SHFT_Y6	24
#define IPU_YUV_GAMMA_SHFT_Y7	28
//lisa_2008_0408_E YUVGamma 
#define IPU_LUM_GAMMA_X_EVEN_SHFT	8
#define IPU_LUM_GAMMA_Y_EVEN_SHFT	0
#define IPU_LUM_GAMMA_X_ODD_SHFT	24
#define IPU_LUM_GAMMA_Y_ODD_SHFT	16

/* IpuFalsColorSuppr */
#define IPU_FCS_EDG_THRESH_SHFT		0
#define IPU_FCS_DECSLOPx256_SHFT	8

#define IPU_FCS_DISA			0x00000000
#define IPU_FCS_ENA			0x00020000

/* IpuHueSaturAdj */
#define IPU_HUE_SIN_SHFT		0
#define IPU_HUE_COS_SHFT		8
#define IPU_SATUR_SHFT			16

#define IPU_HUE_SATUR_DISA		0x00000000
#define IPU_HUE_SATUR_ENA		0x00080000

/* IpuHueSaturAdj */
#define IPU_HUE_SIN_SHFT		0
#define IPU_HUE_COS_SHFT		8
#define IPU_SATUR_SHFT			16

//IpuDeNoise
#define IPU_DN_NOISE_SHFT		0
#define IPU_DN_DIFF_SHFT		16
#define IPU_ADF_DIFF_SHFT		24
#define IPU_DN_DIFFCNT_SHFT		0
#define IPU_ADF_DIFFCNT_SHFT		4			

//IpuYSumRpt
#define IPU_YRPT_W_SHFT			0
#define IPU_YRPT_H_SHFT			16
#define IPU_YRPT_YLTHD_SHFT		0
#define IPU_YRPT_YHTHD_SHFT		8
#define IPU_YRPT_YSCA_SHFT		16

//IpuAFRpt
#define IPU_AFRPR_W_SHFT		0
#define IPU_AFRPR_H_SHFT		16
#define IPU_AFRPT_SCA_SHFT		16
#define IPU_AFRPT_ETHD_SHFT		0
#define IPU_AFRPT_HBOND_SHFT		8			

// OSD_AFreport
#define IPU_MASK_0 0x00000fff
#define IPU_SHFT_0 0
#define IPU_MASK_1 0x0fff0000
#define IPU_SHFT_1 16

#endif
