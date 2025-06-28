/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	isureg.h

Abstract:

   	The declarations of Image Scaling Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __ISU_REG_H__
#define __ISU_REG_H__

// SCA_ENA  0x0000
#define ISU_NORMAL			0x00000000
#define ISU_RESET			0x00000001

#define ISU_DISA			0x00000000
#define ISU_ENA				0x00000002

#define ISU_Conti_ENA		0x00000000
#define ISU_Single_ENA		0x00000004


// SCA_MODE  0x0004
#define ISU_Manual_Mode		0x00000000
#define ISU_Auto_Mode0		0x00000001
#define ISU_Auto_Mode1		0x00000002
#define ISU_Auto_Mode2		0x00000003
#define ISU_Auto_Mode3		0x00000004
#define ISU_Auto_Mode4		0x00000005


#define ISU_PROC_LBMode	0x00000000
#define ISU_PROC_2DMode	0x00000008

#define ISU_SRC_DRAM		0x00000000
#define ISU_SRC_IPU			0x00000010

#define ISU_PKSRGB		    0x00000040

#define ISU_BLKSEL_8L		0x00000000
#define ISU_BLKSEL_16L		0x00000080

#define ISU_FBRSEL_MASK     0x00000300
#define ISU_FBRSEL_SHFT     8
#define ISU_FBR_SEL_0		0x00000000
#define ISU_FBR_SEL_1		0x00000100
#define ISU_FBR_SEL_2		0x00000200

#define ISU_PKWSEL_MASK     0x00000c00
#define ISU_PKWSEL_SHFT     10
#define ISU_PKW_SEL_0		0x00000000
#define ISU_PKW_SEL_1		0x00000400
#define ISU_PKW_SEL_2		0x00000800

#define ISU_PNWSEL_MASK     0x00003000
#define ISU_PNWSEL_SHFT     12
#define ISU_PNW_SEL_0		0x00000000
#define ISU_PNW_SEL_1		0x00001000
#define ISU_PNW_SEL_2		0x00002000

#define ISU_TRIPLE_Dual		0x00000000
#define ISU_TRIPLE_Trip		0x00008000


#define ISU_FBRCUR_MASK     0x00030000
#define ISU_PKWCUR_MASK     0x000c0000
#define ISU_PNWCUR_MASK     0x00300000

#define ISU_FBRCUR_SHFT     16
#define ISU_PKWCUR_SHFT     18
#define ISU_PNWCUR_SHFT     20

#define ISU_OVL_TYPE_16Bit		0x00000000
#define ISU_OVL_TYPE_1Bit		0x10000000
#define ISU_OVL_TYPE_2Bit		0x20000000
#define ISU_OVL_OPA_25          0x00000000
#define ISU_OVL_OPA_50          0x40000000
#define ISU_OVL_OPA_75          0x80000000
#define ISU_OVL_OPA_100         0xC0000000


// SCA_FUN  0x0008
#define ISU_PKOUT_DISA		0x00000000
#define ISU_PKOUT_ENA		0x00000001

#define ISU_PNOUT_DISA		0x00000000
#define ISU_PNOUT_ENA		0x00000002 // This is not allowed if (PROC =1)

#define ISU_OVLEN_DISA		0x00000000
#define ISU_OVLEN_ENA		0x00000004

#define ISU_OVLSEL_MAIN	0x00000000
#define ISU_OVLSEL_BOTH	0x00000008

#define ISU_MSFEN_DISA		0x00000000
#define ISU_MSFEN_ENA		0x00000040

#define ISU_SSFEN_DISA		0x00000000
#define ISU_SSFEN_ENA		0x00000080

//SCA_INTC 0x000C

#define ISU_FCINTS_Mask		0x00000001
#define ISU_PKOINTS_Mask	0x00000010
#define ISU_PYOINTS_Mask	0x00000020
#define ISU_PCOINTS_Mask	0x00000040
#define ISU_SRUINTS_Mask	0x00000080

#define ISU_FCINTE_DISA		0x00000000
#define ISU_FCINTE_ENA		0x00000100

#define ISU_PKOINTE_DISA	0x00000000
#define ISU_PKOINTE_ENA	0x00001000

#define ISU_PYOINTE_DISA	0x00000000
#define ISU_PYOINTE_ENA	0x00002000

#define ISU_PCOINTE_DISA	0x00000000
#define ISU_PCOINTE_ENA	0x00004000

#define ISU_SRUINTE_DISA	0x00000000
#define ISU_SRUINTE_ENA	0x00008000

// SRC_SIZE 0x0020
#define ISU_SRC_W_SHFT		0
#define ISU_SRC_H_SHFT		16

// PK_SIZE 0x0024
#define ISU_PK_W_SHFT		0
#define ISU_PK_H_SHFT		16

//PN_SIZE 0x0028
#define ISU_PN_W_SHFT		0
#define ISU_PN_H_SHFT		16

//MSF_STEP 0x0030
#define ISU_MSTEPX_SHFT	0
#define ISU_MSTEPY_SHFT	16

//MSF_PHA 0x0034
#define ISU_MPHASEX_SHFT	0
#define ISU_MPHASEY_SHFT	16

//SSF_STEP 0x0038
#define ISU_SSTEPX_SHFT	0
#define ISU_SSTEPY_SHFT	16

//SSF_PHA 0x003C
#define ISU_SPHASEX_SHFT	0
#define ISU_SPHASEY_SHFT	16

//PN_STRIDE 0x0078
#define ISU_PNYSTRIDE_SHFT	0
#define ISU_PNCSTRIDE_SHFT	16

//OVL_WSP 0x0080
#define ISU_OVL_WSY_SHFT	16
#define ISU_OVL_WSX_SHFT	0

//OVL_WEP 0x0084
#define ISU_OVL_WEY_SHFT	16
#define ISU_OVL_WEX_SHFT	0

//OVL_IADDR 0x0088
#define ISU_OVL_IADDR_MASK		0x00FFFFFF

//OVL_IADDR 0x008C
#define ISU_OVL_STRIDE_MASK	0x00FFFFFF

//FIFO_THD 0x0090
#define ISU_PK_LT_MASK		0x00000007
#define ISU_PK_HT_MASK		0x00000070
#define ISU_PK_HEN_MASK		0x00000080
#define ISU_PNY_LT_MASK		0x00000700
#define ISU_PNY_HT_MASK		0x00007000
#define ISU_PNY_HEN_MASK	0x00008000
#define ISU_PNC_LT_MASK		0x00070000
#define ISU_PNC_HT_MASK		0x00700000
#define ISU_PNC_HEN_MASK	0x00800000
#define ISU_SRC_LT_MASK		0x07000000
#define ISU_SRC_HT_MASK		0x70000000
#define ISU_SRC_HEN_MASK	0x80000000


//Address Mask
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#define ISU_BaseAdr_Mask	0xff000000
#define ISU_OffAdr_Mask		0xffffffff
#else
#define ISU_BaseAdr_Mask	0xff000000
#define ISU_OffAdr_Mask		0x00ffffff
#endif

//SCUP Controller 0x00d0
#define ISU_SCUP_EN               0x00000003
#define ISU_SCUP_LINESIZE_SHFT    16

#define ISU_SCUP_WAIT_PK          0x00000004
#define ISU_SCUP_WAIT_PNY         0x00000008
#define ISU_SCUP_OVL_DUMY_0       0x00000000
#define ISU_SCUP_OVL_DUMY_4       0x00000020
#define ISU_SCUP_OVL_DUMY_8       0x00000030
#define ISU_SCUP_WAIT_OVL         0x00000040
#define ISU_SCUP_WAIT_PNC         0x00000080

//SCUP Input Window 0x00d4
#define ISU_SCUP_SRCWIN_W_SHFT    0
#define ISU_SCUP_SRCWIN_H_SHFT    16

//SCUP Input Window 0x00d8
#define ISU_SCUP_OUTWIN_W_SHFT    0
#define ISU_SCUP_OUTWIN_H_SHFT    16

//SCUP Input Start  0x00dc
#define ISU_SCUP_SRCSTART_X_SHFT  0
#define ISU_SCUP_SRCSTART_Y_SHFT  16

#endif
