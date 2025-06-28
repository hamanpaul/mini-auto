/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ciureg.h

Abstract:

   	The declarations of CCIR656 Interface Unit registers.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2010/07/23	Lucian Yuan	Create	

*/


 /*CIU_1_CTL1*/ 
 
 #define CIU_NEW_BURST16          0x00000001

 #define CIU_OUT_FORMAT_420       0x00000000
 #define CIU_OUT_FORMAT_422       0x00000040

 #define CIU_SP_OUT_FORMAT_420    0x00000000
 #define CIU_SP_OUT_FORMAT_422    0x00000080

 #define CIU_YUVMAP_36            0x00003600
 #define CIU_YUVMAP_27            0x00002700
 #define CIU_YUVMAP_72            0x00007200
 #define CIU_YUVMAP_87            0x00008700
 #define CIU_YUVMAP_2D            0x00002d00
 #define CIU_YUVMAP_4D            0x00004d00
 #define CIU_YUVMAP_63            0x00006300
 #define CIU_YUVMAP_78            0x00007800
 #define CIU_YUVMAP_C9            0x0000C900

 #define CIU_FIELD_POL_POS        0x00000000
 #define CIU_FIELD_POL_NEG        0x00010000

 #define CIU_656DATALATCH_RIS     0x00000000
 #define CIU_656DATALATCH_FAL     0x00020000
 
 #define CIU_656BOB_FSEL_TOP      0x00000000
 #define CIU_656BOB_FSEL_BOTTOM   0x00040000
 
 #define CIU_OSD_ADDR_MODE_LINEAR 0x00000000
 #define CIU_OSD_ADDR_MODE_FRAME  0x00080000

 #define CIU_SCA_DIS              0x00000000
 #define CIU_SCA_EN               0x00100000   //valid in A1016

 #define CIU_EDGE_DIS             0x00000000
 #define CIU_EDGE_EN              0x00400000

 #define CIU_DENOISE_DIS          0x00000000
 #define CIU_DENOISE_EN           0x00800000

 #define CIU_FRAMEDROP_DIS        0x00000000
 #define CIU_FRAMEDROP_EN         0x01000000

 #define CIU_SUBPATH_DIS          0x00000000
 #define CIU_SUBPATH_EN           0x02000000

 #define CIU_MP_BUR16_DIS         0x00000000
 #define CIU_MP_BUR16_EN          0x04000000

 #define CIU_SP_BUR16_DIS         0x00000000
 #define CIU_SP_BUR16_EN          0x08000000

 #define CIU_SP_DECI_1X1          0x00000000
 #define CIU_SP_DECI_2X1          0x10000000
 #define CIU_SP_DECI_2X2          0x20000000
 #define CIU_SP_DECI_4X2          0x30000000
 #define CIU_SP_DECI_4X4          0x40000000

 #define CIU_MAINPATH_DIS         0x80000000
 #define CIU_MAINPATH_EN          0x00000000

 /*CIU_1_CTL2*/     
 #define CIU_NORMAL			      0x00000000
 #define CIU_RESET			      0x00000001

 #define CIU_DISA			      0x00000000
 #define CIU_ENA				  0x00000002

 #define CIU_VSYNC_ACT_LO		  0x00000000
 #define CIU_VSYNC_ACT_HI		  0x00000004

 #define CIU_HSYNC_ACT_LO		  0x00000000
 #define CIU_HSYNC_ACT_HI		  0x00000008

 #define CIU_INT_DISA_FIFO_OVERF  0x00000000
 #define CIU_INT_ENA_FIFO_OVERF	  0x00000010

 #define CIU_INT_DISA_FRAME_END   0x00000000
 #define CIU_INT_ENA_FRAME_END	  0x00000020

 #define CIU_DATA_OUT_DISA        0x00000000
 #define CIU_DATA_OUT_ENA	      0x00000040

 #define CIU_YUVDATALATCH_NEG     0x00000000
 #define CIU_YUVDATALATCH_POS     0x00000080

 #define CIU_TESTIMG_DISA         0x00000000
 #define CIU_TESTIMG_01           0x00000100
 #define CIU_TESTIMG_02           0x00000200
 #define CIU_TESTIMG_03           0x00000300

 #define CIU_INT_DISA_FIELD_END   0x00000000
 #define CIU_INT_ENA_FIELD_END	  0x00000400

 #define CIU_EXT_FIELD_DISA       0x00000000
 #define CIU_EXT_FIELD_EN         0x00000800

 #define CIU_OSD_DISA             0x00000000
 #define CIU_OSD_ENA              0x00001000

 #define CIU_SP_OSD_DISA          0x00000000
 #define CIU_SP_OSD_ENA           0x00002000

 #define CIU_SPLITER_DIS          0x00000000
 #define CIU_SPLITER_ENA          0x00004000

 #define CIU_INT_DISA_TOPEND      0x00000000
 #define CIU_INT_ENA_TOPEND       0x00010000

 #define CIU_MASKAREA_EN          0x00400000

 #define CIU_DS_MODE_DISA         0x00000000
 #define CIU_DS_MODE_1_2          0x02000000
 #define CIU_DS_MODE_1_4          0x04000000
 #define CIU_DS_MODE_1_8          0x06000000

 #define CIU_MP_SEL_SENSOR        0x00000000
 #define CIU_MP_SEL_DOWNSAMPLE    0x08000000
 #define CIU_MP_SEL_SCALAR        0x10000000

 #define CIU_SP_SEL_SENSOR        0x00000000
 #define CIU_SP_SEL_DOWNSAMPLE    0x20000000
 #define CIU_SP_SEL_SCALAR        0x40000000

 #define CIU_SA_SEL_SENSOR        0x00000000
 #define CIU_SA_SEL_DOWNSAMPLE    0x80000000
 
 /*CIU_1_INTRPT*/        
 #define CIU_INT_STAT_FIFO_OVERF  0x00000001
 #define CIU_INT_STAT_FRAME_END	  0x00000002
 #define CIU_INT_STAT_FIELD_END	  0x00000004
 #define CIU_INT_STAT_SP_OVERF    0x00000010
 
 /* 0X:Interrupt only trig by sensor end
    10:Interrupt only trig by data_complete
    11:Interrupt trig by sesnsor_end or data_complete */
 #if(CHIP_OPTION == CHIP_A1018A)
 #define CIU_INT_SNESOR_END	      0x00000000
 #define CIU_INT_DATA_CMP         0x00000020
 #define CIU_INT_BOTH             0x00000030
 #endif

 /*CIU_1_SYNC_RPT*/ 
 #define CIU_STAT_VSYNC			  0x00000001
 #define CIU_STAT_HSYNC			  0x00000002

 #define CIU_LINE_NUM_MASK		  0x0fff0000
 #define CIU_LINE_NUM_SHFT		  16

 #define CIU_STAT_FIELD           0x80000000
 
 /*CIU_1_InputSize*/       
 #define CIU_INPUT_SIZE_X_SHFT	  0
 #define CIU_INPUT_SIZE_Y_SHFT	  16
 
 /*CIU_1_IMG_STR*/             
 #define CIU_IMG_H_STR_SHFT       0
 #define CIU_IMG_V_STR_SHFT       16
 
 /*CIU_1_TotalSize*/           
 #define CIU_TOTAL_SIZE_X_SHFT    0
 #define CIU_TOTAL_SIZE_Y_SHFT    16
 
 /*CIU_1_OutputSize*/         
 #define CIU_OUTPUT_SIZE_X_SHFT   16
 #define CIU_OUTPUT_SIZE_Y_SHFT   0
 
 /*CIU_1_FRAME_STRIDE*/       
 
 /*CIU_1_STR_YADDR*/       
 
 /*CIU_1_STR_CbCrADDR*/ 
 
 /*CIU_1_OVL_WSP*/    
 #define CIU_OVL_WSX_SHFT         0
 #define CIU_OVL_WSY_SHFT         16
 
 /*CIU_1_OVL_WEP*/        
 #define CIU_OVL_WEX_SHFT         0
 #define CIU_OVL_WEY_SHFT         16
 
 /*CIU_1_OVL_IADDR*/ 
 
 /*CIU_1_OVL_IDXCOLOR_Y*/   
 #define CIU_OVL_COL_Y01_SHFT     0
 #define CIU_OVL_COL_Y02_SHFT     8
 #define CIU_OVL_COL_Y03_SHFT     16
 
 /*CIU_1_OVL_IDXCOLOR_CB*/ 
 #define CIU_OVL_COL_CB01_SHFT    0
 #define CIU_OVL_COL_CB02_SHFT    8
 #define CIU_OVL_COL_CB03_SHFT    16
 
 /*CIU_1_OVL_IDXCOLOR_CR*/
 #define CIU_OVL_COL_CR01_SHFT    0
 #define CIU_OVL_COL_CR02_SHFT    8
 #define CIU_OVL_COL_CR03_SHFT    16
 
 /*CIU_1_OVL_MAXBYTECNTLIM*/   

 /*CIU_1_OVL_STRIDE*/
 #define CIU_OVL_STRIDE_SHFT      0
 #define CIU_OVL_WIDTH_SHFT       16

 /*EEREG1*/
 //----------------DIU ---------------//
 #define DIU_TRIG                 0x01
 #define DIU_FORCEBOB             0x10

 #define DIU_WITH_SHFT            16
 #define DIU_HEIGHT_SHFT          0
 
