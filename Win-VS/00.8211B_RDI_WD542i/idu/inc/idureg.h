/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    idureg.h

Abstract:

    The declarations of Image Display Unit.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#ifndef __IDU_REG_H__
#define __IDU_REG_H__


// IDU_CTL  0x0000
#define IDU_NORMAL                      0x00000000
#define IDU_RESET                       0x00000001

#define IDU_DISA                        0x00000000
#define IDU_ENA                         0x00000002

#define IDU_DATA_EN                     0x00000004
#define IDU_DCLK_EN                     0x00000008
#define IDU_SINGLE                      0x00000010
#define IDU_ROT_DEN                     0x00000020
#define IDU_DAC_PWN_ON                  0x00000040
#define IDU_TV_MODE_ENA                 0x00000080

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#define IDU_YUV420BRI_EN                0x00000100

#define IDU_BRI_DECI_1x1                0x00000000
#define IDU_BRI_DECI_1x2                0x00000200
#define IDU_BRI_DECI_1x4                0x00000400
#define IDU_BRI_DECI_2x1                0x00000600

#define IDU_BRI_DECI_2x2                0x00000800
#define IDU_BRI_DECI_2x4                0x00000a00
#define IDU_BRI_DECI_4x2                0x00000c00
#define IDU_BRI_DECI_4x4                0x00000e00

#define IDU_BRI_DECI_FIELD_TOP          0x00000000
#define IDU_BRI_DECI_FIELD_BOT          0x00001000
#endif

// WIN_CTL 0x0004
#define IDU_CTL_VDO_DISA                0x00000000
#define IDU_CTL_VDO_ENA                 0x00000001

#define IDU_CTL_OSD0_DISA               0x00000000
#define IDU_CTL_OSD0_ENA                0x00000002

#define IDU_CTL_OSD1_DISA               0x00000000
#define IDU_CTL_OSD1_ENA                0x00000004

#define IDU_CTL_OSD2_DISA               0x00000000
#define IDU_CTL_OSD2_ENA                0x00000008

#define IDU_CTL_FBAUTO_DISA             0x00000000
#define IDU_CTL_FBAUTO_ENA              0x00000100

#define IDU_CTL_TRIPLE_DISA             0x00000000
#define IDU_CTL_TRIPLE_ENA              0x00000200

#define IDU_CTL_IDU_WAIT_ENA            0x00000080
#define IDU_CTL_IDU_WAIT_DISA           0x00000080


#define IDU_CTL_FB_SEL_0                0x00000000
#define IDU_CTL_FB_SEL_1                0x00001000
#define IDU_CTL_FB_SEL_2                0x00002000

#define IDU_CTL_FB_CUR_MASk             0x0000C000
#define IDU_CTL_FB_CUR_SHFT                     14

#define IDU_CTL_SRC_FMT_YUV422          0x00000000
#define IDU_CTL_SRC_FMT_YUV420          0x01000000
#define IDU_CTL_SRC_FMT_SRGB            0x02000000

#define IDU_CTL_HISH_SPEED_EN           0x08000000
// DISP_CONF 0x0008
#define IDU_INTF_RGB            0x00000000
#define IDU_INTF_RGB_CF         0x00000001
#define IDU_INTF_80             0x00000002
#define IDU_INTF_68             0x00000003
#define IDU_INTF_MPU            0x00000004
#define IDU_INTF_SPI            0x00000005
#define IDU_INTF_CCIR601        0x00000006
#define IDU_INTF_RGB_CFII       0x00000007

#define IDU_DB_W_18         0x00000000
#define IDU_DB_W_16         0x00000010
#define IDU_DB_W_9              0x00000020
#define IDU_DB_W_8              0x00000030
#define IDU_DB_W_24         0x00000040

#define IDU_RGBTYPE_1       0x00000000
#define IDU_RGBTYPE_2       0x00000100
#define IDU_RGBTYPE_3       0x00000200
#define IDU_RGBTYPE_4       0x00000400
#define IDU_RGBTYPE_5       0x00000500
#define IDU_RGBTYPE_6       0x00000600
#define IDU_RGBTYPE_7       0x00000700

#define IDU_YUV_SWAP_0      0x00000000
#define IDU_YUV_SWAP_1      0x00004000
#define IDU_YUV_SWAP_2      0x00008000
#define IDU_YUV_SWAP_3      0x0000c000

#define IDU_D_POL_FE        0x00000000
#define IDU_D_POL_RE        0x00010000

#define IDU_HS_POL_LOW      0x00000000
#define IDU_HS_POL_HIGH 0x00020000

#define IDU_VS_POL_LOW      0x00000000
#define IDU_VS_POL_HIGH 0x00040000

#define IDU_DEN_POL_0       0x00000000
#define IDU_DEN_POL_1       0x00800000

#define IDU_CFT_0           0x00000000
#define IDU_CFT_1           0x00100000
#define IDU_CFT_2           0x00200000
#define IDU_CFT_3           0x00300000
#define IDU_CFT_4           0x00400000
#define IDU_CFT_5           0x00500000
#define IDU_CFT_6           0x00600000
#define IDU_CFT_7           0x00700000

#define IDU_CLKDIV_MASK 0x07000000
#define IDU_CLKDIV_SHFT     24




//DHTIME0 0x0010
#define IDU_HDEND_MASK      0x000007FF
#define IDU_HDEND_SHFT      0

#define IDU_HPD_MASK        0x07FF0000
#define IDU_HPD_SHFT        16

//DHTIME1 0x0014
#define IDU_HSYNC_S_MASK    0x000007FF
#define IDU_HSYNC_S_SHFT    0

#define IDU_HSYNC_E_MASK    0x07FF0000
#define IDU_HSYNC_E_SHFT    16

//DHTIME2 0x0018
#define IDU_HFECTH_S_MASK   0x000007FF
#define IDU_HFECTH_S_SHFT   0

//DVTIME0 0x001C
#define IDU_VDEND_MASK      0x000007FF
#define IDU_VDEND_SHFT      0

#define IDU_VPD_MASK        0x07FF0000
#define IDU_VPD_SHFT        16

//DVTIME1 0x0020
#define IDU_VSYNC_S_MASK    0x000007FF
#define IDU_VSYNC_S_SHFT    0

#define IDU_VSYNC_E_MASK    0x07FF0000
#define IDU_VSYNC_E_SHFT    16

//MPU_CMD_CONF 0x0024
#define IDU_CMD_RS_0        0x00000000
#define IDU_CMD_RS_1        0x00000001

#define IDU_NWR_WRITE       0x00000000
#define IDU_NWR_READ        0x00000002

#define IDU_CMD_SPI_0       0x00000000
#define IDU_CMD_SPI_1       0x00000004
#define IDU_CMD_SPI_2       0x00000008

#define IDU_CMD_W_18        0x00000000      
#define IDU_CMD_W_16        0x00000020
#define IDU_CMD_W_9     0x00000040
#define IDU_CMD_W_8     0x00000060
#define IDU_CMD_W_4     0x00000080
#define IDU_CMD_W_6     0x000000a0
#define IDU_CMD_W_10        0x000000c0
#define IDU_CMD_W_12        0x000000e0

#define IDU_CMD_BUSY_MASK   0x00008000
#define IDU_CMD_BUSY_SHFT   15

//MPU_CMD 0x0028
#define IDU_MPU_CMD_MASK    0x0003FFFF
#define IDU_MPU_CMD_SHFT    0

//MPU_READ 0x002C
#define IDU_MPU_READ_MASK   0x0003FFFF
#define IDU_MPU_READ_SHFT   0

//OSD_OLAY 0x0030
#define IDU_CKEY_ON_ENA     0x00000001
#define IDU_CKEY_ON_DISA        0x00000000

#define IDU_ALPHA_ON_ENA        0x00000002
#define IDU_ALPHA_ON_DISA       0x00000000

#define IDU_OSD_YUV             0x00000020
#define IDU_OSD_RGB             0x00000000

#define IDU_CKEY_MAT_0          0x00000000
#define IDU_CKEY_MAT_1          0x00000100
#define IDU_CKEY_MAT_2          0x00000200

#define IDU_CKEY_UNM_0          0x00000000
#define IDU_CKEY_UNM_1          0x00000400
#define IDU_CKEY_UNM_2          0x00000800

#define IDU_CKEY_BLEND_MASK 0x00003000
#define IDU_CKEY_BLEND_SHFT 12

//VDO_WSP 0x0034
#define IDU_VDO_WSX_MASK        0x000003FF
#define IDU_VDO_WSX_SHFT        0

#define IDU_VDO_WSY_MASK        0x03FF0000
#define IDU_VDO_WSY_SHFT        16

//VDO_WEP 0x0038
#define IDU_VDO_WEX_MASK        0x000003FF
#define IDU_VDO_WEX_SHFT        0

#define IDU_VDO_WEY_MASK        0x03FF0000
#define IDU_VDO_WEY_SHFT        16

//OSD0_WSP 0x003C
#define IDU_OSD0_WSX_MASK       0x000003FF
#define IDU_OSD0_WSX_SHFT       0

#define IDU_OSD0_WSY_MASK       0x03FF0000
#define IDU_OSD0_WSY_SHFT       16

//OSD0_WEP 0x0040
#define IDU_OSD0_WEX_MASK       0x000003FF
#define IDU_OSD0_WEX_SHFT       0

#define IDU_OSD0_WEY_MASK       0x03FF0000
#define IDU_OSD0_WEY_SHFT       16

//OSD1_WSP 0x0044
#define IDU_OSD1_WSX_MASK       0x000003FF
#define IDU_OSD1_WSX_SHFT       0

#define IDU_OSD1_WSY_MASK       0x03FF0000
#define IDU_OSD1_WSY_SHFT       16

//OSD1_WEP 0x0048
#define IDU_OSD1_WEX_MASK       0x000003FF
#define IDU_OSD1_WEX_SHFT       0

#define IDU_OSD1_WEY_MASK       0x03FF0000
#define IDU_OSD1_WEY_SHFT       16

//OSD2_WSP 0x004C
#define IDU_OSD2_WSX_MASK       0x000003FF
#define IDU_OSD2_WSX_SHFT       0

#define IDU_OSD2_WSY_MASK       0x03FF0000
#define IDU_OSD2_WSY_SHFT       16

//OSD2_WEP 0x0050
#define IDU_OSD2_WEX_MASK       0x000003FF
#define IDU_OSD2_WEX_SHFT       0

#define IDU_OSD2_WEY_MASK       0x03FF0000
#define IDU_OSD2_WEY_SHFT       16

//OSD_CKEY 0x0054
#define IDU_OSD_CKEY_MASK       0x0000000F
#define IDU_OSD_CKEY_SHFT       0

//BG_COLOR 0x0058
#define IDU_VDO_BG_Y_MASLK  0x000000FF
#define IDU_VDO_BG_Y_SHFT       0

#define IDU_VDO_BG_CB_MASLK 0x0000FF00
#define IDU_VDO_BG_CB_SHFT  8

#define IDU_VDO_BG_CR_MASLK 0x00FF0000
#define IDU_VDO_BG_CR_SHFT  16

//OSD_PAL0~ OSD_PAL15  0x0060 ~ 0x009C
#define IDU_OSD_PAL_R_MASK  0x000000FF
#define IDU_OSD_PAL_R_SHFT      0

#define IDU_OSD_PAL_G_MASK  0x0000FF00
#define IDU_OSD_PAL_G_SHFT  8

#define IDU_OSD_PAL_B_MASK  0x00FF0000
#define IDU_OSD_PAL_B_SHFT      16

//ADDR
#define IDU_BASE_ADDR_MASK  0xFF000000
#define IDU_BASE_ADDR_SHFT  24

#define IDU_ADDR_MASK           0x00FFFFFF
#define IDU_ADDR_SHFT           0

//VDO_STRIDE    0x00AC
#define IDU_VDO_STRIKE_MASK 0x000003FF
#define IDU_VDO_STRIKE_SHFT 0

//OSD_STRIDE    0x00BC
#define IDU_OSD0_STRIKE_MASK    0x000000FF
#define IDU_OSD0_STRIKE_SHFT    0

#define IDU_OSD1_STRIKE_MASK    0x0000FF00
#define IDU_OSD1_STRIKE_SHFT    8

#define IDU_OSD2_STRIKE_MASK    0x00FF0000
#define IDU_OSD2_STRIKE_SHFT    16

// FIFO_TH  0x00C0
#define IDU_VDO_LT_4            0x00000000
#define IDU_VDO_LT_8            0x00000001
#define IDU_VDO_LT_12           0x00000002
#define IDU_VDO_LT_16           0x00000003
#define IDU_VDO_LT_20           0x00000004
#define IDU_VDO_LT_24           0x00000005
#define IDU_VDO_LT_28           0x00000006
#define IDU_VDO_LT_32           0x00000007

#define IDU_OSD_LT_No           0x00000000
#define IDU_OSD_LT_6            0x00000200
#define IDU_OSD_LT_8            0x00000300
#define IDU_OSD_LT_10           0x00000400
#define IDU_OSD_LT_12           0x00000500
#define IDU_OSD_LT_14           0x00000600
#define IDU_OSD_LT_16           0x00000700

#define IDU_OSD_HT_No           0x00000000
#define IDU_OSD_HT_6            0x00002000
#define IDU_OSD_HT_8            0x00003000
#define IDU_OSD_HT_10           0x00004000
#define IDU_OSD_HT_12           0x00005000
#define IDU_OSD_HT_14           0x00006000
#define IDU_OSD_HT_16           0x00007000

//CRC_CTL   0x00C4
#define IDU_CRC_DISA            0x00000000
#define IDU_CRC_ENA             0x00000001

#define IDU_CRC_NORMAL          0x00000000
#define IDU_CRC_RST             0x00000002

#define IDU_CRC_BUSY_DISA       0x00000000
#define IDU_CRC_BUSY_ENA        0x00000080

//CRC_DATA0 0x00C8
#define IDU_CRC_DATA_G_MASK 0xFFFF0000
#define IDU_CRC_DATA_G_SHFT 16
#define IDU_CRC_DATA_R_MASK 0x0000FFFF
#define IDU_CRC_DATA_R_SHFT 0

//CRC_DATA0 0x00CC
#define IDU_CRC_DATA_B_MASK 0x0000FFFF
#define IDU_CRC_DATA_B_SHFT 0

//CSC_R     0x00D0
#define IDU_CSC_CR2R_MASK       0x00FF0000
#define IDU_CSC_CR2R_SHFT       16

#define IDU_CSC_CB2R_MASK       0x0000FF00
#define IDU_CSC_CB2R_SHFT       8

#define IDU_CSC_Y2R_MASK        0x000000FF
#define IDU_CSC_Y2R_SHFT        0

//CSC_G     0x00D4
#define IDU_CSC_CR2G_MASK       0x00FF0000
#define IDU_CSC_CR2G_SHFT       16

#define IDU_CSC_CB2G_MASK       0x0000FF00
#define IDU_CSC_CB2G_SHFT       8

#define IDU_CSC_Y2G_MASK        0x000000FF
#define IDU_CSC_Y2G_SHFT        0

//CSC_B     0x00D8
#define IDU_CSC_CR2B_MASK       0x00FF0000
#define IDU_CSC_CR2B_SHFT       16

#define IDU_CSC_CB2B_MASK       0x0000FF00
#define IDU_CSC_CB2B_SHFT       8

#define IDU_CSC_Y2B_MASK        0x000000FF
#define IDU_CSC_Y2B_SHFT        0

//GC_ESEG0      0x00E0
#define IDU_GC_SEG2_MASK        0x1F000000
#define IDU_GC_SEG2_SHFT        24

#define IDU_GC_SEG1_MASK        0x001F0000
#define IDU_GC_SEG1_SHFT        16

#define IDU_GC_SEG0_MASK        0x00001F00
#define IDU_GC_SEG0_SHFT        8

#define IDU_GC_EN_DISA          0x00000000
#define IDU_GC_EN_ENA           0x00000001

//GC_ESEG1      0x00E4
#define IDU_GC_SEG6_MASK        0x1F000000
#define IDU_GC_SEG6_SHFT        24

#define IDU_GC_SEG5_MASK        0x001F0000
#define IDU_GC_SEG5_SHFT        16

#define IDU_GC_SEG4_MASK        0x00001F00
#define IDU_GC_SEG4_SHFT        8

#define IDU_GC_SEG3_MASK        0x0000001F
#define IDU_GC_SEG3_SHFT        0

//GC_TAB0   0x00E8
#define IDU_GC_LUT3_MASK        0xFF000000
#define IDU_GC_LUT3_SHFT        24

#define IDU_GC_LUT2_MASK        0x00FF0000
#define IDU_GC_LUT2_SHFT        16

#define IDU_GC_LUT1_MASK        0x0000FF00
#define IDU_GC_LUT1_SHFT        8

#define IDU_GC_LUT0_MASK        0x000000FF
#define IDU_GC_LUT0_SHFT        0

//GC_TAB1   0x00EC
#define IDU_GC_LUT7_MASK        0xFF000000
#define IDU_GC_LUT7_SHFT        24

#define IDU_GC_LUT6_MASK        0x00FF0000
#define IDU_GC_LUT6_SHFT        16

#define IDU_GC_LUT5_MASK        0x0000FF00
#define IDU_GC_LUT5_SHFT        8

#define IDU_GC_LUT4_MASK        0x000000FF
#define IDU_GC_LUT4_SHFT        0

//GC_OFFSET     0x00F0
#define GC_OFFSET_B_MASK        0x00FF0000
#define GC_OFFSET_B_SHFT        16

#define GC_OFFSET_G_MASK        0x0000FF00
#define GC_OFFSET_G_SHFT        8

#define GC_OFFSET_R_MASK        0x000000FF
#define GC_OFFSET_R_SHFT        0

//BYPASS    0x00F4
#define IDU_CS_SEL_CS1          0x00000000
#define IDU_CS_SEL_CS2          0x00000001

#define IDU_SPI_CS_0            0x00000000
#define IDU_SPI_CS_1            0x00000002

#define IDU_WR_OE_0         0x00000000
#define IDU_WR_OE_1         0x00000004

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
/*OSD L1 Ctl  0x0130*/
#define IDU_L1_CKEY_ON_ENA      0x00000001
#define IDU_L1_CKEY_ON_DISA     0x00000000

#define IDU_L1_ALPHA_ON_ENA     0x00000002
#define IDU_L1_ALPHA_ON_DISA    0x00000000
#define IDU_L1_4BIT_INDEX_OSD   0x00000001
#define IDU_L1_MK_DIS_VIDEO     0x00000000
#define IDU_L1_MK_DIS_OSD       0x00000100
#define IDU_L1_MK_DIS_MIX       0x00000200
#define IDU_L1_UMK_DIS_VIDEO    0x00000000
#define IDU_L1_UMK_DIS_OSD      0x00000400
#define IDU_L1_UMK_DIS_MIX      0x00000800
#define IDU_L1_COLOR_KEY        0xF0000000


#define IDU_L1_OSD0_ENA         0x00000010
#define IDU_L1_OSD0_DISA        0x00000000

#define IDU_L1_LINEAR_ADDR      0x00000000
#define IDU_L1_FRAME_ADDR       0x00000080

#define IDU_L1_ALPHA_MODE_1     0x00000000  /*0%, 25%, 50%, 100%*/
#define IDU_L1_ALPHA_MODE_2     0x00001000  /*0%, 50%, 75%, 100%*/
#define IDU_L1_ALPHA_MODE_3     0x00002000  /*0%, 25%, 50%, 75%*/

/*OSD L1 Ctl  0x0134*/
#define IDU_L2_CKEY_ON_ENA      0x00000001
#define IDU_L2_CKEY_ON_DISA     0x00000000

#define IDU_L2_ALPHA_ON_ENA     0x00000002
#define IDU_L2_ALPHA_ON_DISA    0x00000000
#define IDU_L2_4BIT_INDEX_OSD   0x00000001

#define IDU_L2_OSD0_ENA         0x00000010
#define IDU_L2_OSD0_DISA        0x00000000

#define IDU_L2_LINEAR_ADDR      0x00000000
#define IDU_L2_FRAME_ADDR       0x00000080
#endif
//====== TV Setting====//
  //0x00
  #define TV_RST              0x00000001
  #define TVE_EN              0x00000002
  #define TVE_VX2             0x00000010
  #define TVE_OX2             0x00000020
  #define TVDAC_POWON         0x00000040
  #define TVMODE_SEL          0x00000080

  //0x04
  #define TV_FRMCTL_VDO_EN    0x00000001
  
  #define TV_FRMCTL_OSD0_EN   0x00000002  
  #define TV_FRMCTL_OSD1_EN   0x00000004
  #define TV_FRMCTL_OSD2_EN   0x00000008
  
  #define TV_FRMCTL_FB_AUTO   0x00000100
  #define TV_FRMCTL_FBAUTO_DISA   0x00000000
  
  #define TV_FRMCTL_TRIPLE    0x00000200
  #define TV_FRMCTL_FB_SEL0   0x00000000
  #define TV_FRMCTL_FB_SEL1   0x00001000
  #define TV_FRMCTL_FB_SEL2   0x00002000

  //0x08
  #define TV_DV_SEL_YUV       0x00000000
  #define TV_DV_SEL_601       0x00004000
  #define TV_DV_SEL_656_MODE0 0x00008000
  #define TV_DV_SEL_656_MODE1 0x0000c000
  
  #define TV_D_POL_FAL        0x00000000
  #define TV_D_POL_RISE       0x00010000

  #define TV_HS_POL_LOW       0x00000000
  #define TV_HS_POL_HIGH      0x00020000

  #define TV_VS_POL_LOW       0x00040000
  #define TV_VS_POL_HIGH      0x00000000

  #define TV_DIGITAL_OUT_EN   0x00400000
  #define TV_CLPF_ON          0x00000020  /*UV Low Pass Filter On*/
  //0x108
  #define TV_656_FIX_DIS      0x00000000
  #define TV_656_FIX_ENA      0x00010000
  
  #define TV_656_MUX_DIS      0x00000000
  #define TV_656_MUX_ENA      0x00020000

  #define TV_656_MAIN_OUT     0x00000000
  #define TV_656_SUB_OUT      0x00040000

  #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
  // BRI_CTL  0x6000
  #define BRI_CTRL_RST        0x00000001
  
  #define BRI_CTRL_SC_EN      0x00000002
  #define BRI_CTRL_SC_DISA    0x00000000
  
  #define BRI_CTRL_P_SCAN     0x00000000
  #define BRI_CTRL_I_SCAN     0x00000010  

  #define BRI_CTRL_DAT422     0x00000000
  #define BRI_CTRL_DAT420     0x00000020

  #define BRI_OSD_EN          0x00000040
  #define BRI_OSD_DISA        0x00000000

  #define OSD_DOWNSAMPLE_1    0x00000000  // 1/1
  #define OSD_DOWNSAMPLE_2    0x00000100  // 1/2
  #define OSD_DOWNSAMPLE_4    0x00000200  // 1/4

  #define OSD_BLENDING_100    0x00000000
  #define OSD_BLENDING_075    0x00000400
  #define OSD_BLENDING_050    0x00000800
  #define OSD_BLENDING_025    0x00000C00

  #define EDGE_COLOR_EN       0x00001000
  #define EDGE_COLOR_DISA     0x00000000

  #define BURST_8_OSD_8       0x00010000
  #define BURST_8_OSD_16      0x00000000
  #define BURST_8_VIDEO_8     0x00020000
  #define BURST_8_VIDEO_16    0x00000000
    
  #define BITWIDTH64_EN       0x00000000
  #define BITWIDTH64_DISA     0x00040000

  // BRI_STRIDE 0x600c
  #define BRI_BFA_DIV1        0x00000000
  #define BRI_BFA_DIV2        0x00010000
  #define BRI_BFA_DIV4        0x00020000

  //BRI_IN_SIZE   0x6010
  #define BRI_IN_WIDTH_SHFT   0
  #define BRI_IN_HEIGHT_SHFT  16

  //BRI_OUT_SIZE  0x6014
  #define BRI_OUT_WIDTH_SHFT  0
  #define BRI_OUT_HEIGHT_SHFT 16

  #endif
  #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) )
  #define TV_FRMCTL_OSDL1W0_EN   0x00000002  
  #define TV_FRMCTL_OSDL1W1_EN   0x00000004
  #define TV_FRMCTL_OSDL1W2_EN   0x00000008
  #endif

  //------------2D Grahpic-----------------//
  #define GFU_DISA               0x00
  #define GFU_ENA                0x01
  #define GFU_RESET              0x02

  //---------------------------------------//
#endif
