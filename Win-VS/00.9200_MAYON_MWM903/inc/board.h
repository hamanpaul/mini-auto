/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    board.h

Abstract:

    The memory map of the board (chip).

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#ifndef __BOARD_H__
#define __BOARD_H__

#include "general.h"

/*****************************************************************************/
/*  Base addresses for standard memory-mapped peripherals                    */
/*****************************************************************************/
/* AHB Processing Unit */
#define StMemBase       0x00000000  /* Static Memory Base                   */
#define TcmSramBase     0x60000000  /* TCM SRAM Base                        */
#define CacheCtrlBase   0x60010000  /* Cache and TCM Base                   */
#define	DualCoreCtrlBase   0x60020000
#define SdramBase       0x80000000  /* SDRAM Base                           */
#define AHBARBBase      0xc0000000  /* AHB Arbit config, Lucian added       */
#define SdramCtrlBase   0xc0040000  /* SDRAM Control Register Base          */
#define DmaCtrlBase     0xc0050000  /* DMA Control Register Base            */
#define EmbedSramBase   0xc0070000  /* Embedded SRAM Base                   */
#define HiuCtrlBase     0xc0080000  /* Host Interface Unit Control Register Base    */

#define RFI_1_CtrlBase  0xc0090000  /* RF Interface Unit Control Register Base */
#define RFI_2_CtrlBase  0xc0091000  /* RF Interface Unit Control Register Base */
#define RFI_3_CtrlBase  0xc0092000  /* RF Interface Unit Control Register Base */
#define RFI_4_CtrlBase  0xc0093000  /* RF Interface Unit Control Register Base */

#define DIU_CtrlBase    0xc0210000  /* Deinterlace Unit Control Register Base */

#define SiuCtrlBase     0xc0100000  /* Sensor Input Unit Control Register Base  */
#define IpuCtrlBase     0xc0110000  /* Image Processing Unit Control Register Base  */
#define IduCtrlBase     0xc0120000  /* Image Display Unit Control Register Base */
#define TVCtrBase       0xc0120000  /* TV Unit Control Register Base*/
#define IsuCtrlBase     0xc0130000  /* Image Scaling Unit Control Register Base     */
#define JpegCtrlBase    0xc0140000  /* JPEG Control Register Base           */
#define Mpeg4CtrlBase   0xc0150000  /* MPEG4 Control Register Base          */

#define H264Enc_CtrlBase    0xc0158000  /* H264 Encoder Control Register Base          */
#define H264Dec_CtrlBase    0xc0150000  /* H264 Decoder Control Register Base          */

#define DES_CtrlBase    0xC0080000  /* DES/AES Control Register Base       */

#define CIU_1_CtrlBase  0xc0160000  /* CCIR656 Control Register Base       */
#define CIU_2_CtrlBase  0xc0160200  /* CCIR656 Control Register Base       */
#define CIU_3_CtrlBase  0xc0160400  /* CCIR656 Control Register Base       */
#define CIU_4_CtrlBase  0xc0160800  /* CCIR656 Control Register Base       */

#define CIU_5_CtrlBase  0xc0130000  /* scaling engine for SIU/IPU       */

#define TV2CtrBase      0xc0170000  /* 2nd TV Unit Control Register Base   */
#define MCPUCtrlBase    0xc0180000  /* Memory-copy Process Unit Base*/
#define IAUCtrlBase     0xc0181000  /* IMA ADPCM Unit Base*/
#define MDUCtrlBase     0xc0190000  /* Motion Detector Control Register Base*/
#define MDUSRAMBase     0xc0191000  /**MDU SRAM base address */
#define FTMACBase       0xc01A0000  /* FTMAC110 Base */
#define FTMACBase2      0xc01B0000  /* FTMAC110 Base */
#define DualCoreCfgBase 0xc01D0000	/* C2 Register Base		*/
#define MCPU_2_CtrlBase 0xc01e0000  /* 2nd Memory-copy Process Unit Base*/

/* APB Processing Unit */
#define IntCtrlBase     0xd0000000  /* INT Control Register Base            */
#define StMemCtrlBase   0xd0010000  /* Static Memory Control Register Base      */
#define GpioCtrlBase    0xd0020000  /* GPIO Control Register Base           */
#define UartCtrlBase    0xd0030000  /* UART Control Register Base           */
#define I2cCtrlBase     0xd0040000  /* I2C Control Register Base            */
#define IisCtrlBase     0xd0050000  /* IIS Control Register Base            */
#define UsbCtrlBase     0xd0060000  /* USB Control Register Base            */
#if !SD1_SWITCH_SD2
#define SdcCtrlBase     0xd0070000  /* SecurityDisk Card Control Register Base  */
#define Sdc_2_CtrlBase  0xd0080000  /* SD2 Control Register*/
#else
#define SdcCtrlBase     0xd0080000  /* SecurityDisk Card Control Register Base  */
#define Sdc_2_CtrlBase  0xd0070000  /* SD2 Control Register*/
#endif

#define SmcCtrlBase     0xd0080000  /* SmartMedia Card Control Register Base    */
#define TimerCtrlBase   0xd0090000  /* Timer Control Register Base          */
#define RtcCtrlBase     0xd00a0000  /* RTC Control Register Base            */
#define SysCtrlBase     0xd00b0000  /* System Control Register Base         */
#define WdtCtrlBase     0xd00c0000  /* WDT Control Register Base            */
#define AdcCtrlBase     0xd00d0000  /* ADC Control Register Base            */

#define IdeCtrlBase     0xd00e0000  /* IDE control Register Base  */
#define GPIUCtrlBase    0xd00f0000  /* General Processor Interface Control Register*/
#if !USB1_SWITCH_USB2
#define FD_USBHOSTBase           0xc00c0000  /* USB Host */
#define FD_USBHOSTCorBase        0xc00c0010  /* USB Host */
#define FD_USBOTGBase            0xc00c0080  /* USB OTG */
#define FD_USBGLOBALBase         0xc00c00C0  /* USB Global */
#define FD_USBDEVICEBase         0xc00c0100  /* USB Device */
#else //usb_2nd
#define FD_USBHOSTBase          0xe8000000  /* USB2 Host */
#define FD_USBHOSTCorBase       0xe8000010  /* USB2 Host */
#define FD_USBOTGBase           0xe8000080  /* USB2 OTG */
#define FD_USBGLOBALBase        0xe80000C0  /* USB2 Global */
#define FD_USBDEVICEBase        0xe8000100  /* USB2 Device */
#endif
#define FD_USBHOSTBase_2nd           0xe8000000  /* USB Host */
#define FD_USBHOSTCorBase_2nd        0xe8000010  /* USB Host */
#define FD_USBOTGBase_2nd            0xe8000080  /* USB OTG */
#define FD_USBGLOBALBase_2nd         0xe80000C0  /* USB Global */
#define FD_USBDEVICEBase_2nd         0xe8000100  /* USB Device */

#define IRCtrlBase               0xd0100000  /* IR control Register Base */
#define GFUCtrlBase              0xe0000000  /* 2D-Graphic Register Base */
/*****************************************************************************/
/* AHB Processing Unit                                               */
/*****************************************************************************/
#define AHB_ARBCtrl             *((volatile unsigned *)(AHBARBBase))
/*****************************************************************************/
/* Static Memory Address                                                 */
/*****************************************************************************/
#define StMemAddr               *((volatile unsigned *)(StMemBase))

/*****************************************************************************/
/* SDRAM Address                                                         */
/*****************************************************************************/
#define SdramAddr               *((volatile unsigned *)(SdramBase))

/*****************************************************************************/
/* SDRAM Control Register                                        */
/*****************************************************************************/
#define SdramTimeParm0          *((volatile unsigned *)(SdramCtrlBase))
#define SdramTimeParm1          *((volatile unsigned *)(SdramCtrlBase + 0x0004))
#define SdramConfig             *((volatile unsigned *)(SdramCtrlBase + 0x0008))
#define SdramExtBank            *((volatile unsigned *)(SdramCtrlBase + 0x000c))
#define SdramExtMode            *((volatile unsigned *)(SdramCtrlBase + 0x0010))
#define SdramDFI_PHYB           *((volatile unsigned *)(SdramCtrlBase + 0x001c))
#define SdramPerf_Cyc1          *((volatile unsigned *)(SdramCtrlBase + 0x002c))
#define SdramPerf_Cyc2          *((volatile unsigned *)(SdramCtrlBase + 0x0030))
#define SdramArbit              *((volatile unsigned *)(SdramCtrlBase + 0x0034))
#define SdramTimeCtrl           *((volatile unsigned *)(SdramCtrlBase + 0x0038))

/*****************************************************************************/
/* DMA Control Register                                              */
/*****************************************************************************/
#define REG_DMACH0SRCADDR     (DmaCtrlBase)
#define REG_DMACH0DSTADDR     (DmaCtrlBase + 0x0004)
#define REG_DMACH0CYCCNT      (DmaCtrlBase + 0x0008)
#define REG_DMACH0CMD         (DmaCtrlBase + 0x000c)

#define REG_DMACH1SRCADDR     (DmaCtrlBase + 0x0010)
#define REG_DMACH1DSTADDR     (DmaCtrlBase + 0x0014)
#define REG_DMACH1CYCCNT      (DmaCtrlBase + 0x0018)
#define REG_DMACH1CMD         (DmaCtrlBase + 0x001c)

#define REG_DMACH2SRCADDR     (DmaCtrlBase + 0x0020)
#define REG_DMACH2DSTADDR     (DmaCtrlBase + 0x0024)
#define REG_DMACH2CYCCNT      (DmaCtrlBase + 0x0028)
#define REG_DMACH2CMD         (DmaCtrlBase + 0x002c)

#define REG_DMACH3SRCADDR     (DmaCtrlBase + 0x0030)
#define REG_DMACH3DSTADDR     (DmaCtrlBase + 0x0034)
#define REG_DMACH3CYCCNT      (DmaCtrlBase + 0x0038)
#define REG_DMACH3CMD         (DmaCtrlBase + 0x003c)

#define REG_DMAINTSTAT        (DmaCtrlBase + 0x0040)

#define REG_DMACH4SRCADDR     (DmaCtrlBase + 0x0050)
#define REG_DMACH4DSTADDR     (DmaCtrlBase + 0x0054)
#define REG_DMACH4CYCCNT      (DmaCtrlBase + 0x0058)
#define REG_DMACH4CMD         (DmaCtrlBase + 0x005c)

#define REG_DMACH5SRCADDR     (DmaCtrlBase + 0x0060)
#define REG_DMACH5DSTADDR     (DmaCtrlBase + 0x0064)
#define REG_DMACH5CYCCNT      (DmaCtrlBase + 0x0068)
#define REG_DMACH5CMD         (DmaCtrlBase + 0x006c)

#define REG_DMACH6SRCADDR     (DmaCtrlBase + 0x0070)
#define REG_DMACH6DSTADDR     (DmaCtrlBase + 0x0074)
#define REG_DMACH6CYCCNT      (DmaCtrlBase + 0x0078)
#define REG_DMACH6CMD         (DmaCtrlBase + 0x007c)

#define REG_DMACH7SRCADDR     (DmaCtrlBase + 0x0080)
#define REG_DMACH7DSTADDR     (DmaCtrlBase + 0x0084)
#define REG_DMACH7CYCCNT      (DmaCtrlBase + 0x0088)
#define REG_DMACH7CMD         (DmaCtrlBase + 0x008c)

#define REG_DMACH0SRCADDRAlt  (DmaCtrlBase + 0x0090)
#define REG_DMACH0DSTADDRAlt  (DmaCtrlBase + 0x0094)
#define REG_DMACH0AutoNum     (DmaCtrlBase + 0x0098)

#define REG_DMACH1SRCADDRAlt  (DmaCtrlBase + 0x009c)
#define REG_DMACH1DSTADDRAlt  (DmaCtrlBase + 0x00a0)
#define REG_DMACH1AutoNum     (DmaCtrlBase + 0x00a4)

#define REG_DMACH2SRCADDRAlt  (DmaCtrlBase + 0x00a8)
#define REG_DMACH2DSTADDRAlt  (DmaCtrlBase + 0x00ac)
#define REG_DMACH2AutoNum     (DmaCtrlBase + 0x00b0)


#define REG_DMACH3SRCADDRAlt  (DmaCtrlBase + 0x00b4)
#define REG_DMACH3DSTADDRAlt  (DmaCtrlBase + 0x00b8)
#define REG_DMACH3AutoNum     (DmaCtrlBase + 0x00bc)

#define REG_DMACH4SRCADDRAlt  (DmaCtrlBase + 0x00c0)
#define REG_DMACH4DSTADDRAlt  (DmaCtrlBase + 0x00c4)
#define REG_DMACH4AutoNum     (DmaCtrlBase + 0x00c8)

#define REG_DMACH5SRCADDRAlt  (DmaCtrlBase + 0x00cc)
#define REG_DMACH5DSTADDRAlt  (DmaCtrlBase + 0x00d0)
#define REG_DMACH5AutoNum     (DmaCtrlBase + 0x00d4)

#define REG_DMACH6SRCADDRAlt  (DmaCtrlBase + 0x00d8)
#define REG_DMACH6DSTADDRAlt  (DmaCtrlBase + 0x00dc)
#define REG_DMACH6AutoNum     (DmaCtrlBase + 0x00e0)

#define REG_DMACH7SRCADDRAlt  (DmaCtrlBase + 0x00e4)
#define REG_DMACH7DSTADDRAlt  (DmaCtrlBase + 0x00e8)
#define REG_DMACH7AutoNum     (DmaCtrlBase + 0x00ec)

/*****************************************************************************/
/* Embedded SRAM                                             */
/*****************************************************************************/
#define EmbedSram           *((volatile unsigned *)(EmbedSramBase))

/*****************************************************************************/
/* Host Interface Unit Control Register                                  */
/*****************************************************************************/
#define HiuCtrl0            *((volatile unsigned *)(HiuCtrlBase))
#define HiuCtrl1            *((volatile unsigned *)(HiuCtrlBase + 0x0004))
#define HiuIntEna           *((volatile unsigned *)(HiuCtrlBase + 0x0008))
#define HiuIntStat          *((volatile unsigned *)(HiuCtrlBase + 0x000c))
#define HiuCmd              *((volatile unsigned *)(HiuCtrlBase + 0x0010))
#define HiuData             *((volatile unsigned *)(HiuCtrlBase + 0x0020))
#define HiuDramStart        *((volatile unsigned *)(HiuCtrlBase + 0x0024))

/*****************************************************************************/
/* Sensor Interface Unit Control Register                                */
/*****************************************************************************/
#define SiuRawAddr          *((volatile unsigned *)(SiuCtrlBase))
#define SiuSensCtrl         *((volatile unsigned *)(SiuCtrlBase + 0x0004))
#define SiuIntStat          *((volatile unsigned *)(SiuCtrlBase + 0x0008))
#define SiuSyncStat         *((volatile unsigned *)(SiuCtrlBase + 0x000c))
#define SiuValidSize        *((volatile unsigned *)(SiuCtrlBase + 0x0010))
#define SiuValidStart       *((volatile unsigned *)(SiuCtrlBase + 0x0014))
#define SiuTotalSize        *((volatile unsigned *)(SiuCtrlBase + 0x0018))
#define SiuOb               *((volatile unsigned *)(SiuCtrlBase + 0x001c))
#define SiuBGbGain          *((volatile unsigned *)(SiuCtrlBase + 0x0020))
#define SiuRGrGain          *((volatile unsigned *)(SiuCtrlBase + 0x0024))
#define SiuLensCornerX2     *((volatile unsigned *)(SiuCtrlBase + 0x0028))
#define SiuLensCornerY2     *((volatile unsigned *)(SiuCtrlBase + 0x002c))
#define SiuLensCentOffs     *((volatile unsigned *)(SiuCtrlBase + 0x0030))
#define SiuLensCompGain     *((volatile unsigned *)(SiuCtrlBase + 0x0034))
#define SiuDefPixTbl        *((volatile unsigned *)(SiuCtrlBase + 0x0038))

#define Siu_B_Gamma1        *((volatile unsigned *)(SiuCtrlBase + 0x003c))
#define Siu_B_Gamma2        *((volatile unsigned *)(SiuCtrlBase + 0x0040))
#define Siu_B_Gamma3        *((volatile unsigned *)(SiuCtrlBase + 0x0044))
#define Siu_B_Gamma4        *((volatile unsigned *)(SiuCtrlBase + 0x0048))
#define Siu_B_Gamma5        *((volatile unsigned *)(SiuCtrlBase + 0x004c))
#define Siu_B_Gamma6        *((volatile unsigned *)(SiuCtrlBase + 0x0050))
#define Siu_B_Gamma7        *((volatile unsigned *)(SiuCtrlBase + 0x0054))
#define Siu_B_Gamma8        *((volatile unsigned *)(SiuCtrlBase + 0x0058))
#define Siu_B_Gamma9        *((volatile unsigned *)(SiuCtrlBase + 0x005c))
#define Siu_B_Gamma10       *((volatile unsigned *)(SiuCtrlBase + 0x0060))
#define Siu_B_Gamma11       *((volatile unsigned *)(SiuCtrlBase + 0x0064))
#define Siu_B_Gamma12       *((volatile unsigned *)(SiuCtrlBase + 0x0068))
#define Siu_B_Gamma13       *((volatile unsigned *)(SiuCtrlBase + 0x006c))
#define Siu_B_Gamma14       *((volatile unsigned *)(SiuCtrlBase + 0x0070))
#define Siu_B_Gamma15       *((volatile unsigned *)(SiuCtrlBase + 0x0074))
#define Siu_B_Gamma16       *((volatile unsigned *)(SiuCtrlBase + 0x0078))

#define Siu_Gb_Gamma1       *((volatile unsigned *)(SiuCtrlBase + 0x0114))
#define Siu_Gb_Gamma2       *((volatile unsigned *)(SiuCtrlBase + 0x0118))
#define Siu_Gb_Gamma3       *((volatile unsigned *)(SiuCtrlBase + 0x011c))
#define Siu_Gb_Gamma4       *((volatile unsigned *)(SiuCtrlBase + 0x0120))

#define Siu_R_Gamma1        *((volatile unsigned *)(SiuCtrlBase + 0x0124))
#define Siu_R_Gamma2        *((volatile unsigned *)(SiuCtrlBase + 0x0128))
#define Siu_R_Gamma3        *((volatile unsigned *)(SiuCtrlBase + 0x012c))
#define Siu_R_Gamma4        *((volatile unsigned *)(SiuCtrlBase + 0x0130))

#define Siu_Gr_Gamma1       *((volatile unsigned *)(SiuCtrlBase + 0x0134))
#define Siu_Gr_Gamma2       *((volatile unsigned *)(SiuCtrlBase + 0x0138))
#define Siu_Gr_Gamma3       *((volatile unsigned *)(SiuCtrlBase + 0x013c))
#define Siu_Gr_Gamma4       *((volatile unsigned *)(SiuCtrlBase + 0x0140))

#define Siu_GbRGb_Gamma5    *((volatile unsigned *)(SiuCtrlBase + 0x0144))



#define SiuAeWinStart       *((volatile unsigned *)(SiuCtrlBase + 0x007c))
#define SiuAeWinSize        *((volatile unsigned *)(SiuCtrlBase + 0x0080))
#define SiuAeCtrl           *((volatile unsigned *)(SiuCtrlBase + 0x0084))
#define SiuAeWin0_1         *((volatile unsigned *)(SiuCtrlBase + 0x0088))
#define SiuAeWin2_3         *((volatile unsigned *)(SiuCtrlBase + 0x008c))
#define SiuAeWin4_5         *((volatile unsigned *)(SiuCtrlBase + 0x0090))
#define SiuAeWin6_7         *((volatile unsigned *)(SiuCtrlBase + 0x0094))
#define SiuAeWin8_9         *((volatile unsigned *)(SiuCtrlBase + 0x0098))
#define SiuAeWin10_11       *((volatile unsigned *)(SiuCtrlBase + 0x009c))
#define SiuAeWin12_13       *((volatile unsigned *)(SiuCtrlBase + 0x00a0))
#define SiuAeWin14_15       *((volatile unsigned *)(SiuCtrlBase + 0x00a4))
#define SiuAeWin16_17       *((volatile unsigned *)(SiuCtrlBase + 0x00a8))
#define SiuAeWin18_19       *((volatile unsigned *)(SiuCtrlBase + 0x00ac))
#define SiuAeWin20_21       *((volatile unsigned *)(SiuCtrlBase + 0x00b0))
#define SiuAeWin22_23       *((volatile unsigned *)(SiuCtrlBase + 0x00b4))
#define SiuAeWin24          *((volatile unsigned *)(SiuCtrlBase + 0x00b8))
#define SiuAeRBondOut       *((volatile unsigned *)(SiuCtrlBase + 0x00bc))
#define SiuAeGBondOut       *((volatile unsigned *)(SiuCtrlBase + 0x00c0))
#define SiuAeBBondOut       *((volatile unsigned *)(SiuCtrlBase + 0x00c4))
#define SiuAeRBondMid       *((volatile unsigned *)(SiuCtrlBase + 0x00c8))
#define SiuAeGBondMid       *((volatile unsigned *)(SiuCtrlBase + 0x00cc))
#define SiuAeBBondMid       *((volatile unsigned *)(SiuCtrlBase + 0x00d0))
#define SiuAeRBondInn       *((volatile unsigned *)(SiuCtrlBase + 0x00d4))
#define SiuAeGBondInn       *((volatile unsigned *)(SiuCtrlBase + 0x00d8))
#define SiuAeBBondInn       *((volatile unsigned *)(SiuCtrlBase + 0x00dc))
#define SiuAeWgtSum         *((volatile unsigned *)(SiuCtrlBase + 0x00e0))
#define SiuFrameScal        *((volatile unsigned *)(SiuCtrlBase + 0x00e4))
#define SiuAwbWinStart      *((volatile unsigned *)(SiuCtrlBase + 0x00e8))
#define SiuAwbWinSize       *((volatile unsigned *)(SiuCtrlBase + 0x00ec))
#define SiuAwbCtrl          *((volatile unsigned *)(SiuCtrlBase + 0x00f0))
#define SiuAwbThresh        *((volatile unsigned *)(SiuCtrlBase + 0x00f4))
#define SiuAwbGain1         *((volatile unsigned *)(SiuCtrlBase + 0x00f8))
#define SiuAwbGain2         *((volatile unsigned *)(SiuCtrlBase + 0x00fc))
#define SiuAwbRSum          *((volatile unsigned *)(SiuCtrlBase + 0x0100))
#define SiuAwbGSum          *((volatile unsigned *)(SiuCtrlBase + 0x0104))
#define SiuAwbBSum          *((volatile unsigned *)(SiuCtrlBase + 0x0108))
#define SiuAwbPixCnt        *((volatile unsigned *)(SiuCtrlBase + 0x010c))
#define SiuDebugSel         *((volatile unsigned *)(SiuCtrlBase + 0x0110))
#define SiuLineStride       *((volatile unsigned *)(SiuCtrlBase + 0x0148))
#define SiuDefPixIndex      *((volatile unsigned *)(SiuCtrlBase + 0x014c))  //write defect pixel to SRAM
#define SiuDefPixData       *((volatile unsigned *)(SiuCtrlBase + 0x0150))
#define SiuPreGammaGain     *((volatile unsigned *)(SiuCtrlBase + 0x0154))  //new function for PA9003, AWB gain for pregamma.

/*****************************************************************************/
/* Image Processing Unit Control Register                                */
/*****************************************************************************/
#define IpuEna                  *((volatile unsigned *)(IpuCtrlBase))
#define IpuModeCtrl             *((volatile unsigned *)(IpuCtrlBase + 0x0004))
#define IpuFunEna               *((volatile unsigned *)(IpuCtrlBase + 0x0008))
#define IpuIntStrlStat          *((volatile unsigned *)(IpuCtrlBase + 0x000c))
#define IpuInSize               *((volatile unsigned *)(IpuCtrlBase + 0x0010))
#define IpuOutSize              *((volatile unsigned *)(IpuCtrlBase + 0x0014))
#define IpuSrcAddr              *((volatile unsigned *)(IpuCtrlBase + 0x0018))
#define IpuDst0Addr             *((volatile unsigned *)(IpuCtrlBase + 0x001c))
#define IpuDst1Addr             *((volatile unsigned *)(IpuCtrlBase + 0x0020))
#define IpuDst2Addr             *((volatile unsigned *)(IpuCtrlBase + 0x0024))
#define IpuCfaInterp            *((volatile unsigned *)(IpuCtrlBase + 0x0028))
#define IpuColorCorrMatrx1_2    *((volatile unsigned *)(IpuCtrlBase + 0x002c))
#define IpuColorCorrMatrx3_4    *((volatile unsigned *)(IpuCtrlBase + 0x0030))
#define IpuColorCorrMatrx5_6    *((volatile unsigned *)(IpuCtrlBase + 0x0034))
#define IpuColorCorrMatrx7_8    *((volatile unsigned *)(IpuCtrlBase + 0x0038))
#define IpuColorCorrMatrx9      *((volatile unsigned *)(IpuCtrlBase + 0x003c))
#define IpuEdgEnhance1          *((volatile unsigned *)(IpuCtrlBase + 0x0040))
#define IpuEdgEnhance2          *((volatile unsigned *)(IpuCtrlBase + 0x0044))
#define IpuEdgEnhance3          *((volatile unsigned *)(IpuCtrlBase + 0x0048))



#define IpuYUVGamma1            *((volatile unsigned *)(IpuCtrlBase + 0x004C))
#define IpuYUVGamma2            *((volatile unsigned *)(IpuCtrlBase + 0x0050))
#define IpuYUVGamma3            *((volatile unsigned *)(IpuCtrlBase + 0x0054))
#define IpuYUVGamma4            *((volatile unsigned *)(IpuCtrlBase + 0x0058))
#define IpuYUVGamma5            *((volatile unsigned *)(IpuCtrlBase + 0x005C))
#define IpuYUVGammaShift1       *((volatile unsigned *)(IpuCtrlBase + 0x0060))
#define IpuYUVGammaShift2       *((volatile unsigned *)(IpuCtrlBase + 0x0064))
#define IpuYUVGammaShift3       *((volatile unsigned *)(IpuCtrlBase + 0x0068))
#define IpuThroughputCntr   *((volatile unsigned *)(IpuCtrlBase + 0x006c))


#define IpuFalsColorSuppr       *((volatile unsigned *)(IpuCtrlBase + 0x0070))
#define IpuCfaRBgain        *((volatile unsigned *)(IpuCtrlBase + 0x0074))
#define IpuColorTransMatrx1_2   *((volatile unsigned *)(IpuCtrlBase + 0x0078))
#define IpuColorTransMatrx3_4   *((volatile unsigned *)(IpuCtrlBase + 0x007c))
#define IpuColorTransMatrx5_6   *((volatile unsigned *)(IpuCtrlBase + 0x0080))
#define IpuColorTransMatrx7_8   *((volatile unsigned *)(IpuCtrlBase + 0x0084))
#define IpuColorTransMatrx9 *((volatile unsigned *)(IpuCtrlBase + 0x0088))
///////////////////////////////////////////////////////////////////////////////
#define IpuDeNoise1         *((volatile unsigned *)(IpuCtrlBase + 0x00a0))
#define IpuDeNoise2         *((volatile unsigned *)(IpuCtrlBase + 0x00a4))
#define IpuYRptStr          *((volatile unsigned *)(IpuCtrlBase + 0x00a8))
#define IpuYRptWinSize      *((volatile unsigned *)(IpuCtrlBase + 0x00ac))
#define IpuYRptCtl          *((volatile unsigned *)(IpuCtrlBase + 0x00b0))
#define IpuYRptUpHist       *((volatile unsigned *)(IpuCtrlBase + 0x00b4))
#define IpuYRptMidHist      *((volatile unsigned *)(IpuCtrlBase + 0x00b8))
#define IpuYRptCenHist      *((volatile unsigned *)(IpuCtrlBase + 0x00bc))
#define IpuYRptLowHist      *((volatile unsigned *)(IpuCtrlBase + 0x00c0))
#define IpuYSumRpt0_1       *((volatile unsigned *)(IpuCtrlBase + 0x00c4))
#define IpuYSumRpt2_3       *((volatile unsigned *)(IpuCtrlBase + 0x00c8))
#define IpuYSumRpt4_5       *((volatile unsigned *)(IpuCtrlBase + 0x00cc))
#define IpuYSumRpt6_7       *((volatile unsigned *)(IpuCtrlBase + 0x00d0))
#define IpuYSumRpt8         *((volatile unsigned *)(IpuCtrlBase + 0x00d4))
#define IpuAFRptStr         *((volatile unsigned *)(IpuCtrlBase + 0x00d8))
#define IpuAFRptSize        *((volatile unsigned *)(IpuCtrlBase + 0x00dc))
#define IpuAFRptCtl         *((volatile unsigned *)(IpuCtrlBase + 0x00e0))
#define IpuAFRptHistUp_Mid  *((volatile unsigned *)(IpuCtrlBase + 0x00e4))
#define IpuYRptHistCen_Low  *((volatile unsigned *)(IpuCtrlBase + 0x00e8))
#define IpuAFRpt0_1         *((volatile unsigned *)(IpuCtrlBase + 0x00ec))
#define IpuAFRpt2_3         *((volatile unsigned *)(IpuCtrlBase + 0x00f0))
#define IpuAFRpt4_5         *((volatile unsigned *)(IpuCtrlBase + 0x00f4))
#define IpuAFRpt6_7         *((volatile unsigned *)(IpuCtrlBase + 0x00f8))
#define IpuAFRpt8           *((volatile unsigned *)(IpuCtrlBase + 0x00fc))
#define IpuRGammaCorr0_1    *((volatile unsigned *)(IpuCtrlBase + 0x0100))
#define IpuRGammaCorr2_3    *((volatile unsigned *)(IpuCtrlBase + 0x0104))
#define IpuRGammaCorr4_5    *((volatile unsigned *)(IpuCtrlBase + 0x0108))
#define IpuRGammaCorr6_7    *((volatile unsigned *)(IpuCtrlBase + 0x010C))
#define IpuRGammaCorr8_9    *((volatile unsigned *)(IpuCtrlBase + 0x0110))
#define IpuRGammaCorr10_11  *((volatile unsigned *)(IpuCtrlBase + 0x0114))
#define IpuRGammaCorr12_13  *((volatile unsigned *)(IpuCtrlBase + 0x0118))
#define IpuRGammaCorr14_15  *((volatile unsigned *)(IpuCtrlBase + 0x011C))
#define IpuRGammaCorr16 *((volatile unsigned *)(IpuCtrlBase + 0x0120))

#define IpuGGammaCorr0_1    *((volatile unsigned *)(IpuCtrlBase + 0x0124))
#define IpuGGammaCorr2_3    *((volatile unsigned *)(IpuCtrlBase + 0x0128))
#define IpuGGammaCorr4_5    *((volatile unsigned *)(IpuCtrlBase + 0x012C))
#define IpuGGammaCorr6_7    *((volatile unsigned *)(IpuCtrlBase + 0x0130))
#define IpuGGammaCorr8_9    *((volatile unsigned *)(IpuCtrlBase + 0x0134))
#define IpuGGammaCorr10_11  *((volatile unsigned *)(IpuCtrlBase + 0x0138))
#define IpuGGammaCorr12_13  *((volatile unsigned *)(IpuCtrlBase + 0x013C))
#define IpuGGammaCorr14_15  *((volatile unsigned *)(IpuCtrlBase + 0x0140))
#define IpuGGammaCorr16     *((volatile unsigned *)(IpuCtrlBase + 0x0144))

#define IpuBGammaCorr0_1    *((volatile unsigned *)(IpuCtrlBase + 0x0148))
#define IpuBGammaCorr2_3    *((volatile unsigned *)(IpuCtrlBase + 0x014C))
#define IpuBGammaCorr4_5    *((volatile unsigned *)(IpuCtrlBase + 0x0150))
#define IpuBGammaCorr6_7    *((volatile unsigned *)(IpuCtrlBase + 0x0154))
#define IpuBGammaCorr8_9    *((volatile unsigned *)(IpuCtrlBase + 0x0158))
#define IpuBGammaCorr10_11  *((volatile unsigned *)(IpuCtrlBase + 0x015C))
#define IpuBGammaCorr12_13  *((volatile unsigned *)(IpuCtrlBase + 0x0160))
#define IpuBGammaCorr14_15  *((volatile unsigned *)(IpuCtrlBase + 0x0164))
#define IpuBGammaCorr16     *((volatile unsigned *)(IpuCtrlBase + 0x0168))


/*****************************************************************************/
/* Image Display Unit Control Register                                   */
/*****************************************************************************/
#define IduEna              *((volatile unsigned *)(IduCtrlBase))
#define IduWinCtrl          *((volatile unsigned *)(IduCtrlBase + 0x0004))
#define IduDispCfg          *((volatile unsigned *)(IduCtrlBase + 0x0008))
#define IduIntCtrl          *((volatile unsigned *)(IduCtrlBase + 0x000c))
#define IduHorzTimeCfg0     *((volatile unsigned *)(IduCtrlBase + 0x0010))
#define IduHorzTimeCfg1     *((volatile unsigned *)(IduCtrlBase + 0x0014))
#define IduHorzTimeCfg2     *((volatile unsigned *)(IduCtrlBase + 0x0018))
#define IduVertTimeCfg0     *((volatile unsigned *)(IduCtrlBase + 0x001c))
#define IduVertTimeCfg1     *((volatile unsigned *)(IduCtrlBase + 0x0020))
#define IduMpuCmdCfg        *((volatile unsigned *)(IduCtrlBase + 0x0024))
#define IduMpuCmd           *((volatile unsigned *)(IduCtrlBase + 0x0028))
#define IduMpuRead          *((volatile unsigned *)(IduCtrlBase + 0x002c))
#define IduOsdCtrl          *((volatile unsigned *)(IduCtrlBase + 0x0030))
#define IduVidWinStart      *((volatile unsigned *)(IduCtrlBase + 0x0034))
#define IduVidWinEnd        *((volatile unsigned *)(IduCtrlBase + 0x0038))
#define IduOsdWin0Start     *((volatile unsigned *)(IduCtrlBase + 0x003c))
#define IduOsdWin0End       *((volatile unsigned *)(IduCtrlBase + 0x0040))
#define IduOsdWin1Start     *((volatile unsigned *)(IduCtrlBase + 0x0044))
#define IduOsdWin1End       *((volatile unsigned *)(IduCtrlBase + 0x0048))
#define IduOsdWin2Start     *((volatile unsigned *)(IduCtrlBase + 0x004c))
#define IduOsdWin2End       *((volatile unsigned *)(IduCtrlBase + 0x0050))
#define IduOsdColorKey      *((volatile unsigned *)(IduCtrlBase + 0x0054))
#define IduDefBgColor       *((volatile unsigned *)(IduCtrlBase + 0x0058))
#define IduDVTIME2          *((volatile unsigned *)(IduCtrlBase + 0x005c))
#define IduOsdPal0          *((volatile unsigned *)(IduCtrlBase + 0x0060))
#define IduOsdPal1          *((volatile unsigned *)(IduCtrlBase + 0x0064))
#define IduOsdPal2          *((volatile unsigned *)(IduCtrlBase + 0x0068))
#define IduOsdPal3          *((volatile unsigned *)(IduCtrlBase + 0x006c))
#define IduOsdPal4          *((volatile unsigned *)(IduCtrlBase + 0x0070))
#define IduOsdPal5          *((volatile unsigned *)(IduCtrlBase + 0x0074))
#define IduOsdPal6          *((volatile unsigned *)(IduCtrlBase + 0x0078))
#define IduOsdPal7          *((volatile unsigned *)(IduCtrlBase + 0x007c))
#define IduOsdPal8          *((volatile unsigned *)(IduCtrlBase + 0x0080))
#define IduOsdPal9          *((volatile unsigned *)(IduCtrlBase + 0x0084))
#define IduOsdPal10         *((volatile unsigned *)(IduCtrlBase + 0x0088))
#define IduOsdPal11         *((volatile unsigned *)(IduCtrlBase + 0x008c))
#define IduOsdPal12         *((volatile unsigned *)(IduCtrlBase + 0x0090))
#define IduOsdPal13         *((volatile unsigned *)(IduCtrlBase + 0x0094))
#define IduOsdPal14         *((volatile unsigned *)(IduCtrlBase + 0x0098))
#define IduOsdPal15         *((volatile unsigned *)(IduCtrlBase + 0x009c))
#define IduOsdPal16         *((volatile unsigned *)(IduCtrlBase + 0x01a0))
#define IduOsdPal17         *((volatile unsigned *)(IduCtrlBase + 0x01a4))
#define IduOsdPal18         *((volatile unsigned *)(IduCtrlBase + 0x01a8))
#define IduOsdPal19         *((volatile unsigned *)(IduCtrlBase + 0x01ac))
#define IduOsdPal20         *((volatile unsigned *)(IduCtrlBase + 0x01b0))
#define IduOsdPal21         *((volatile unsigned *)(IduCtrlBase + 0x01b4))
#define IduOsdPal22         *((volatile unsigned *)(IduCtrlBase + 0x01b8))
#define IduOsdPal23         *((volatile unsigned *)(IduCtrlBase + 0x01bc))
#define IduOsdPal24         *((volatile unsigned *)(IduCtrlBase + 0x01C0))
#define IduOsdPal25         *((volatile unsigned *)(IduCtrlBase + 0x01c4))
#define IduOsdPal26         *((volatile unsigned *)(IduCtrlBase + 0x01c8))
#define IduOsdPal27         *((volatile unsigned *)(IduCtrlBase + 0x01cc))
#define IduOsdPal28         *((volatile unsigned *)(IduCtrlBase + 0x01d0))
#define IduOsdPal29         *((volatile unsigned *)(IduCtrlBase + 0x01d4))
#define IduOsdPal30         *((volatile unsigned *)(IduCtrlBase + 0x01d8))
#define IduOsdPal31         *((volatile unsigned *)(IduCtrlBase + 0x01dc))
#define IduOsdPal32         *((volatile unsigned *)(IduCtrlBase + 0x01e0))
#define IduOsdPal33         *((volatile unsigned *)(IduCtrlBase + 0x01e4))
#define IduOsdPal34         *((volatile unsigned *)(IduCtrlBase + 0x01e8))
#define IduOsdPal35         *((volatile unsigned *)(IduCtrlBase + 0x01ec))
#define IduOsdPal36         *((volatile unsigned *)(IduCtrlBase + 0x01f0))
#define IduOsdPal37         *((volatile unsigned *)(IduCtrlBase + 0x01f4))
#define IduOsdPal38         *((volatile unsigned *)(IduCtrlBase + 0x01f8))
#define IduOsdPal39         *((volatile unsigned *)(IduCtrlBase + 0x01fc))
#define IduOsdPal40         *((volatile unsigned *)(IduCtrlBase + 0x0200))
#define IduOsdPal41         *((volatile unsigned *)(IduCtrlBase + 0x0204))
#define IduOsdPal42         *((volatile unsigned *)(IduCtrlBase + 0x0208))
#define IduOsdPal43         *((volatile unsigned *)(IduCtrlBase + 0x020c))
#define IduOsdPal44         *((volatile unsigned *)(IduCtrlBase + 0x0210))
#define IduOsdPal45         *((volatile unsigned *)(IduCtrlBase + 0x0214))
#define IduOsdPal46         *((volatile unsigned *)(IduCtrlBase + 0x0218))
#define IduOsdPal47         *((volatile unsigned *)(IduCtrlBase + 0x021c))
#define IduOsdPal48         *((volatile unsigned *)(IduCtrlBase + 0x0220))
#define IduOsdPal49         *((volatile unsigned *)(IduCtrlBase + 0x0224))
#define IduOsdPal50         *((volatile unsigned *)(IduCtrlBase + 0x0228))
#define IduOsdPal51         *((volatile unsigned *)(IduCtrlBase + 0x022c))
#define IduOsdPal52         *((volatile unsigned *)(IduCtrlBase + 0x0230))
#define IduOsdPal53         *((volatile unsigned *)(IduCtrlBase + 0x0234))
#define IduOsdPal54         *((volatile unsigned *)(IduCtrlBase + 0x0238))
#define IduOsdPal55         *((volatile unsigned *)(IduCtrlBase + 0x023c))
#define IduOsdPal56         *((volatile unsigned *)(IduCtrlBase + 0x0240))
#define IduOsdPal57         *((volatile unsigned *)(IduCtrlBase + 0x0244))
#define IduOsdPal58         *((volatile unsigned *)(IduCtrlBase + 0x0248))
#define IduOsdPal59         *((volatile unsigned *)(IduCtrlBase + 0x024c))
#define IduOsdPal60         *((volatile unsigned *)(IduCtrlBase + 0x0250))
#define IduOsdPal61         *((volatile unsigned *)(IduCtrlBase + 0x0254))
#define IduOsdPal62         *((volatile unsigned *)(IduCtrlBase + 0x0258))
#define IduOsdPal63         *((volatile unsigned *)(IduCtrlBase + 0x025c))

#define IduVidBuf0Addr      *((volatile unsigned *)(IduCtrlBase + 0x00a0))
#define IduVidBuf1Addr      *((volatile unsigned *)(IduCtrlBase + 0x00a4))
#define IduVidBuf2Addr      *((volatile unsigned *)(IduCtrlBase + 0x00a8))
#define IduVidBufStride     *((volatile unsigned *)(IduCtrlBase + 0x00ac))
#define IduOsdBuf0Addr      *((volatile unsigned *)(IduCtrlBase + 0x00b0))
#define IduOsdBuf1Addr      *((volatile unsigned *)(IduCtrlBase + 0x00b4))
#define IduOsdBuf2Addr      *((volatile unsigned *)(IduCtrlBase + 0x00b8))
#define IduOsdBufStride     *((volatile unsigned *)(IduCtrlBase + 0x00bc))
#define IduFifoThresh       *((volatile unsigned *)(IduCtrlBase + 0x00c0))
#define IduCrcCtrl          *((volatile unsigned *)(IduCtrlBase + 0x00c4))
#define IduCrcData0         *((volatile unsigned *)(IduCtrlBase + 0x00c8))
#define IduCrcData1         *((volatile unsigned *)(IduCtrlBase + 0x00cc))
#define IduYCbCr2R          *((volatile unsigned *)(IduCtrlBase + 0x00d0))
#define IduYCbCr2G          *((volatile unsigned *)(IduCtrlBase + 0x00d4))
#define IduYCbCr2B          *((volatile unsigned *)(IduCtrlBase + 0x00d8))
#define IduGammaX0          *((volatile unsigned *)(IduCtrlBase + 0x00e0))
#define IduGammaX1          *((volatile unsigned *)(IduCtrlBase + 0x00e4))
#define IduGammaY0          *((volatile unsigned *)(IduCtrlBase + 0x00e8))
#define IduGammaY1          *((volatile unsigned *)(IduCtrlBase + 0x00ec))
#define IduGammaOffset      *((volatile unsigned *)(IduCtrlBase + 0x00f0))
#define IduBypassCtrl       *((volatile unsigned *)(IduCtrlBase + 0x00f4))
#define IduDVTIME3          *((volatile unsigned *)(IduCtrlBase + 0x00f8))
#define IduDVTIME4          *((volatile unsigned *)(IduCtrlBase + 0x00fc))
#define IduSRC_SIZE         *((volatile unsigned *)(IduCtrlBase + 0x0100))
#define IduROT_IADDR0       *((volatile unsigned *)(IduCtrlBase + 0x0104))
#define IduROT_IADDR1       *((volatile unsigned *)(IduCtrlBase + 0x0108))
#define IduROT_STRIDE       *((volatile unsigned *)(IduCtrlBase + 0x010c))
#define IduROTD_IADDR0      *((volatile unsigned *)(IduCtrlBase + 0x0110))
#define IduROTD_IADDR1      *((volatile unsigned *)(IduCtrlBase + 0x0114))

#define IduOsdL1Ctrl        *((volatile unsigned *)(IduCtrlBase + 0x0130))
#define IduOsdL2Ctrl        *((volatile unsigned *)(IduCtrlBase + 0x0134))
#define IduOsdL1Win0Start   *((volatile unsigned *)(IduCtrlBase + 0x013c))
#define IduOsdL1Win0End     *((volatile unsigned *)(IduCtrlBase + 0x0140))
#define IduOsdL2Win0Start   *((volatile unsigned *)(IduCtrlBase + 0x0154))
#define IduOsdL2Win0End     *((volatile unsigned *)(IduCtrlBase + 0x0158))
#define IduOsdL1Buf0Addr    *((volatile unsigned *)(IduCtrlBase + 0x0170))
#define IduOsdL1BufStride   *((volatile unsigned *)(IduCtrlBase + 0x017C))
#define IduOsdL2Buf0Addr    *((volatile unsigned *)(IduCtrlBase + 0x0180))
#define IduOsdL2BufStride   *((volatile unsigned *)(IduCtrlBase + 0x018C))
#define IduL1L2FifoThresh   *((volatile unsigned *)(IduCtrlBase + 0x0190))
#define IduUV_AddrOffset    *((volatile unsigned *)(IduCtrlBase + 0x0194))
#define IduBRIOutSize       *((volatile unsigned *)(IduCtrlBase + 0x0198))
//-----------------------------//
#define tvTVE_EN            *((volatile unsigned *)(IduCtrlBase))
#define tvFRAME_CTL         *((volatile unsigned *)(IduCtrlBase + 0x0004))
#define tvTV_CONF           *((volatile unsigned *)(IduCtrlBase + 0x0008))
#define tvTVE_INTC          *((volatile unsigned *)(IduCtrlBase + 0x000c))
#define tvTV_CTRL           *((volatile unsigned *)(IduCtrlBase + 0x0010))
#define tvCORING            *((volatile unsigned *)(IduCtrlBase + 0x0014))
#define tvHTOTAL            *((volatile unsigned *)(IduCtrlBase + 0x0018))
#define tvCSYNCCFG          *((volatile unsigned *)(IduCtrlBase + 0x001c))
#define tvBURSTCFG          *((volatile unsigned *)(IduCtrlBase + 0x0020))
#define tvBLANKCFG          *((volatile unsigned *)(IduCtrlBase + 0x0034))
#define tvHSYNC_WIDTH       *((volatile unsigned *)(IduCtrlBase + 0x0038))
#define tvOSD0_WSP          *((volatile unsigned *)(IduCtrlBase + 0x003c))
#define tvOSD0_WEP          *((volatile unsigned *)(IduCtrlBase + 0x0040))
#define tvOSD1_WSP          *((volatile unsigned *)(IduCtrlBase + 0x0044))
#define tvOSD1_WEP          *((volatile unsigned *)(IduCtrlBase + 0x0048))
#define tvOSD2_WSP          *((volatile unsigned *)(IduCtrlBase + 0x004c))
#define tvOSD2_WEP          *((volatile unsigned *)(IduCtrlBase + 0x0050))
#define tvVACTSTAEND        *((volatile unsigned *)(IduCtrlBase + 0x0054))
#define tvBURSTSTAEND       *((volatile unsigned *)(IduCtrlBase + 0x0058))
#define tvACTSTAEND         *((volatile unsigned *)(IduCtrlBase + 0x005c))
#define tvOSD_PAL0          *((volatile unsigned *)(IduCtrlBase + 0x0060))
#define tvOSD_PAL1          *((volatile unsigned *)(IduCtrlBase + 0x0064))
#define tvOSD_PAL2          *((volatile unsigned *)(IduCtrlBase + 0x0068))
#define tvOSD_PAL3          *((volatile unsigned *)(IduCtrlBase + 0x006c))
#define tvOSD_PAL4          *((volatile unsigned *)(IduCtrlBase + 0x0070))
#define tvOSD_PAL5          *((volatile unsigned *)(IduCtrlBase + 0x0074))
#define tvOSD_PAL6          *((volatile unsigned *)(IduCtrlBase + 0x0078))
#define tvOSD_PAL7          *((volatile unsigned *)(IduCtrlBase + 0x007c))
#define tvOSD_PAL8          *((volatile unsigned *)(IduCtrlBase + 0x0080))
#define tvOSD_PAL9          *((volatile unsigned *)(IduCtrlBase + 0x0084))
#define tvOSD_PAL10         *((volatile unsigned *)(IduCtrlBase + 0x0088))
#define tvOSD_PAL11         *((volatile unsigned *)(IduCtrlBase + 0x008c))
#define tvOSD_PAL12         *((volatile unsigned *)(IduCtrlBase + 0x0090))
#define tvOSD_PAL13         *((volatile unsigned *)(IduCtrlBase + 0x0094))
#define tvOSD_PAL14         *((volatile unsigned *)(IduCtrlBase + 0x0098))
#define tvOSD_PAL15         *((volatile unsigned *)(IduCtrlBase + 0x009c))
#define tvOSD_PAL16         *((volatile unsigned *)(IduCtrlBase + 0x01a0))
#define tvOSD_PAL17         *((volatile unsigned *)(IduCtrlBase + 0x01a4))
#define tvOSD_PAL18         *((volatile unsigned *)(IduCtrlBase + 0x01a8))
#define tvOSD_PAL19         *((volatile unsigned *)(IduCtrlBase + 0x01ac))
#define tvOSD_PAL20         *((volatile unsigned *)(IduCtrlBase + 0x01b0))
#define tvOSD_PAL21         *((volatile unsigned *)(IduCtrlBase + 0x01b4))
#define tvOSD_PAL22         *((volatile unsigned *)(IduCtrlBase + 0x01b8))
#define tvOSD_PAL23         *((volatile unsigned *)(IduCtrlBase + 0x01bc))
#define tvOSD_PAL24         *((volatile unsigned *)(IduCtrlBase + 0x01C0))
#define tvOSD_PAL25         *((volatile unsigned *)(IduCtrlBase + 0x01c4))
#define tvOSD_PAL26         *((volatile unsigned *)(IduCtrlBase + 0x01c8))
#define tvOSD_PAL27         *((volatile unsigned *)(IduCtrlBase + 0x01cc))
#define tvOSD_PAL28         *((volatile unsigned *)(IduCtrlBase + 0x01d0))
#define tvOSD_PAL29         *((volatile unsigned *)(IduCtrlBase + 0x01d4))
#define tvOSD_PAL30         *((volatile unsigned *)(IduCtrlBase + 0x01d8))
#define tvOSD_PAL31         *((volatile unsigned *)(IduCtrlBase + 0x01dc))
#define tvOSD_PAL32         *((volatile unsigned *)(IduCtrlBase + 0x01e0))
#define tvOSD_PAL33         *((volatile unsigned *)(IduCtrlBase + 0x01e4))
#define tvOSD_PAL34         *((volatile unsigned *)(IduCtrlBase + 0x01e8))
#define tvOSD_PAL35         *((volatile unsigned *)(IduCtrlBase + 0x01ec))
#define tvOSD_PAL36         *((volatile unsigned *)(IduCtrlBase + 0x01f0))
#define tvOSD_PAL37         *((volatile unsigned *)(IduCtrlBase + 0x01f4))
#define tvOSD_PAL38         *((volatile unsigned *)(IduCtrlBase + 0x01f8))
#define tvOSD_PAL39         *((volatile unsigned *)(IduCtrlBase + 0x01fc))
#define tvOSD_PAL40         *((volatile unsigned *)(IduCtrlBase + 0x0200))
#define tvOSD_PAL41         *((volatile unsigned *)(IduCtrlBase + 0x0204))
#define tvOSD_PAL42         *((volatile unsigned *)(IduCtrlBase + 0x0208))
#define tvOSD_PAL43         *((volatile unsigned *)(IduCtrlBase + 0x020c))
#define tvOSD_PAL44         *((volatile unsigned *)(IduCtrlBase + 0x0210))
#define tvOSD_PAL45         *((volatile unsigned *)(IduCtrlBase + 0x0214))
#define tvOSD_PAL46         *((volatile unsigned *)(IduCtrlBase + 0x0218))
#define tvOSD_PAL47         *((volatile unsigned *)(IduCtrlBase + 0x021c))
#define tvOSD_PAL48         *((volatile unsigned *)(IduCtrlBase + 0x0220))
#define tvOSD_PAL49         *((volatile unsigned *)(IduCtrlBase + 0x0224))
#define tvOSD_PAL50         *((volatile unsigned *)(IduCtrlBase + 0x0228))
#define tvOSD_PAL51         *((volatile unsigned *)(IduCtrlBase + 0x022c))
#define tvOSD_PAL52         *((volatile unsigned *)(IduCtrlBase + 0x0230))
#define tvOSD_PAL53         *((volatile unsigned *)(IduCtrlBase + 0x0234))
#define tvOSD_PAL54         *((volatile unsigned *)(IduCtrlBase + 0x0238))
#define tvOSD_PAL55         *((volatile unsigned *)(IduCtrlBase + 0x023c))
#define tvOSD_PAL56         *((volatile unsigned *)(IduCtrlBase + 0x0240))
#define tvOSD_PAL57         *((volatile unsigned *)(IduCtrlBase + 0x0244))
#define tvOSD_PAL58         *((volatile unsigned *)(IduCtrlBase + 0x0248))
#define tvOSD_PAL59         *((volatile unsigned *)(IduCtrlBase + 0x024c))
#define tvOSD_PAL60         *((volatile unsigned *)(IduCtrlBase + 0x0250))
#define tvOSD_PAL61         *((volatile unsigned *)(IduCtrlBase + 0x0254))
#define tvOSD_PAL62         *((volatile unsigned *)(IduCtrlBase + 0x0258))
#define tvOSD_PAL63         *((volatile unsigned *)(IduCtrlBase + 0x025c))
#define tvOSDL1_CTL         *((volatile unsigned *)(IduCtrlBase + 0x0260))
#define tvOSDL1W0_WSP       *((volatile unsigned *)(IduCtrlBase + 0x0264))
#define tvOSDL1W0_WEP       *((volatile unsigned *)(IduCtrlBase + 0x0268))
#define tvOSDL1W1_WSP       *((volatile unsigned *)(IduCtrlBase + 0x026C))
#define tvOSDL1W1_WEP       *((volatile unsigned *)(IduCtrlBase + 0x0270))
#define tvOSDL1W2_WSP       *((volatile unsigned *)(IduCtrlBase + 0x0274))
#define tvOSDL1W2_WEP       *((volatile unsigned *)(IduCtrlBase + 0x0278))
#define tvTOFBL1_IDDR0      *((volatile unsigned *)(IduCtrlBase + 0x027C))
#define tvTOFBL1_IDDR1      *((volatile unsigned *)(IduCtrlBase + 0x0280))
#define tvTOFBL1_IDDR2      *((volatile unsigned *)(IduCtrlBase + 0x0284))
#define tvTOFBL1_STRIDE     *((volatile unsigned *)(IduCtrlBase + 0x0288))
#define tvTVFB_IADDR0       *((volatile unsigned *)(IduCtrlBase + 0x00a0))
#define tvTVFB_IADDR1       *((volatile unsigned *)(IduCtrlBase + 0x00a4))
#define tvTVFB_IADDR2       *((volatile unsigned *)(IduCtrlBase + 0x00a8))
#define tvTVFB_STRIDE       *((volatile unsigned *)(IduCtrlBase + 0x00ac))
#define tvTOFB_IDDR0        *((volatile unsigned *)(IduCtrlBase + 0x00b0))
#define tvTOFB_IDDR1        *((volatile unsigned *)(IduCtrlBase + 0x00b4))
#define tvTOFB_IDDR2        *((volatile unsigned *)(IduCtrlBase + 0x00b8))
#define tvTOFB_STRIDE       *((volatile unsigned *)(IduCtrlBase + 0x00bc))
#define tvFIFO_TH           *((volatile unsigned *)(IduCtrlBase + 0x00c0))
#define tvYUVGAIN_REG       *((volatile unsigned *)(IduCtrlBase + 0x00d0))
#define tvSUB_CAR_FR        *((volatile unsigned *)(IduCtrlBase + 0x00e8))
#define tvDACVAL            *((volatile unsigned *)(IduCtrlBase + 0x00ec))
#define tvDACVAL1           *((volatile unsigned *)(IduCtrlBase + 0x00f0))
#define tvWHITEYEL          *((volatile unsigned *)(IduCtrlBase + 0x00f8))
#define tvCYANGRN           *((volatile unsigned *)(IduCtrlBase + 0x00fc))
#define tvMAGRED            *((volatile unsigned *)(IduCtrlBase + 0x0100))
#define tvBLUEBLACK         *((volatile unsigned *)(IduCtrlBase + 0x0104))
#define tvBT656CONF         *((volatile unsigned *)(IduCtrlBase + 0x0108))

#define tvUV_AddrOffset    *((volatile unsigned *)(IduCtrlBase + 0x0194))
#define tvBRIOutSize       *((volatile unsigned *)(IduCtrlBase + 0x0198))
/*****************************************************************************/
/* 2nd TV Encoder Unit Control Register                                   */
/*****************************************************************************/
#define tv2TVE_EN            *((volatile unsigned *)(TV2CtrBase))
#define tv2FRAME_CTL         *((volatile unsigned *)(TV2CtrBase + 0x0004))
#define tv2TV_CONF           *((volatile unsigned *)(TV2CtrBase + 0x0008))
#define tv2TVE_INTC          *((volatile unsigned *)(TV2CtrBase + 0x000c))
#define tv2TV_CTRL           *((volatile unsigned *)(TV2CtrBase + 0x0010))
#define tv2CORING            *((volatile unsigned *)(TV2CtrBase + 0x0014))
#define tv2HTOTAL            *((volatile unsigned *)(TV2CtrBase + 0x0018))
#define tv2CSYNCCFG          *((volatile unsigned *)(TV2CtrBase + 0x001c))
#define tv2BURSTCFG          *((volatile unsigned *)(TV2CtrBase + 0x0020))
#define tv2BLANKCFG          *((volatile unsigned *)(TV2CtrBase + 0x0034))
#define tv2HSYNC_WIDTH       *((volatile unsigned *)(TV2CtrBase + 0x0038))
#define tv2OSD0_WSP          *((volatile unsigned *)(TV2CtrBase + 0x003c))
#define tv2OSD0_WEP          *((volatile unsigned *)(TV2CtrBase + 0x0040))
#define tv2OSD1_WSP          *((volatile unsigned *)(TV2CtrBase + 0x0044))
#define tv2OSD1_WEP          *((volatile unsigned *)(TV2CtrBase + 0x0048))
#define tv2OSD2_WSP          *((volatile unsigned *)(TV2CtrBase + 0x004c))
#define tv2OSD2_WEP          *((volatile unsigned *)(TV2CtrBase + 0x0050))
#define tv2VACTSTAEND        *((volatile unsigned *)(TV2CtrBase + 0x0054))
#define tv2BURSTSTAEND       *((volatile unsigned *)(TV2CtrBase + 0x0058))
#define tv2ACTSTAEND         *((volatile unsigned *)(TV2CtrBase + 0x005c))
#define tv2OSD_PAL0          *((volatile unsigned *)(TV2CtrBase + 0x0060))
#define tv2OSD_PAL1          *((volatile unsigned *)(TV2CtrBase + 0x0064))
#define tv2OSD_PAL2          *((volatile unsigned *)(TV2CtrBase + 0x0068))
#define tv2OSD_PAL3          *((volatile unsigned *)(TV2CtrBase + 0x006c))
#define tv2OSD_PAL4          *((volatile unsigned *)(TV2CtrBase + 0x0070))
#define tv2OSD_PAL5          *((volatile unsigned *)(TV2CtrBase + 0x0074))
#define tv2OSD_PAL6          *((volatile unsigned *)(TV2CtrBase + 0x0078))
#define tv2OSD_PAL7          *((volatile unsigned *)(TV2CtrBase + 0x007c))
#define tv2OSD_PAL8          *((volatile unsigned *)(TV2CtrBase + 0x0080))
#define tv2OSD_PAL9          *((volatile unsigned *)(TV2CtrBase + 0x0084))
#define tv2OSD_PAL10         *((volatile unsigned *)(TV2CtrBase + 0x0088))
#define tv2OSD_PAL11         *((volatile unsigned *)(TV2CtrBase + 0x008c))
#define tv2OSD_PAL12         *((volatile unsigned *)(TV2CtrBase + 0x0090))
#define tv2OSD_PAL13         *((volatile unsigned *)(TV2CtrBase + 0x0094))
#define tv2O2SD_PAL14        *((volatile unsigned *)(TV2CtrBase + 0x0098))
#define tv2OSD_PAL15         *((volatile unsigned *)(TV2CtrBase + 0x009c))
#define tv2TVFB_IADDR0       *((volatile unsigned *)(TV2CtrBase + 0x00a0))
#define tv2TVFB_IADDR1       *((volatile unsigned *)(TV2CtrBase + 0x00a4))
#define tv2TVFB_IADDR2       *((volatile unsigned *)(TV2CtrBase + 0x00a8))
#define tv2TVFB_STRIDE       *((volatile unsigned *)(TV2CtrBase + 0x00ac))
#define tv2TOFB_IDDR0        *((volatile unsigned *)(TV2CtrBase + 0x00b0))
#define tv2TOFB_IDDR1        *((volatile unsigned *)(TV2CtrBase + 0x00b4))
#define tv2TOFB_IDDR2        *((volatile unsigned *)(TV2CtrBase + 0x00b8))
#define tv2TOFB_STRIDE       *((volatile unsigned *)(TV2CtrBase + 0x00bc))
#define tv2FIFO_TH           *((volatile unsigned *)(TV2CtrBase + 0x00c0))
#define tv2YUVGAIN_REG       *((volatile unsigned *)(TV2CtrBase + 0x00d0))
#define tv2SUB_CAR_FR        *((volatile unsigned *)(TV2CtrBase + 0x00e8))
#define tv2DACVAL            *((volatile unsigned *)(TV2CtrBase + 0x00ec))
#define tv2DACVAL1           *((volatile unsigned *)(TV2CtrBase + 0x00f0))
#define tv2WHITEYEL          *((volatile unsigned *)(TV2CtrBase + 0x00f8))
#define tv2CYANGRN           *((volatile unsigned *)(TV2CtrBase + 0x00fc))
#define tv2MAGRED            *((volatile unsigned *)(TV2CtrBase + 0x0100))
#define tv2BLUEBLACK         *((volatile unsigned *)(TV2CtrBase + 0x0104))

#define tv2UV_AddrOffset    *((volatile unsigned *)(IduCtrlBase + 0x0194))
#define tv2BRIOutSize       *((volatile unsigned *)(IduCtrlBase + 0x0198))

/*****************************************************************************/
/* IDU Display Bridge Unit Control Register                                   */
/*****************************************************************************/
#define BRI_CTRL_REG         *((volatile unsigned *)(IduCtrlBase + 0x6000))
#define BRI_IADDR_Y          *((volatile unsigned *)(IduCtrlBase + 0x6004))
#define BRI_IADDR_C          *((volatile unsigned *)(IduCtrlBase + 0x6008))
#define BRI_STRIDE           *((volatile unsigned *)(IduCtrlBase + 0x600c))
#define BRI_IN_SIZE          *((volatile unsigned *)(IduCtrlBase + 0x6010))
#define BRI_OUT_SIZE         *((volatile unsigned *)(IduCtrlBase + 0x6014))
#define BRI_SCCTRL_MODE      *((volatile unsigned *)(IduCtrlBase + 0x601c))

#define BRI_VIDEO_TESTDATA   *((volatile unsigned *)(IduCtrlBase + 0x6038))

/*****************************************************************************/
/* PIP                                           */
/*****************************************************************************/
#define OSD_ADDR_Y           *((volatile unsigned *)(IduCtrlBase + 0x6020))
#define OSD_ADDR_C           *((volatile unsigned *)(IduCtrlBase + 0x6024))
#define OSD_STRIDE           *((volatile unsigned *)(IduCtrlBase + 0x6028))
#define OSD_BFAddrShift      *((volatile unsigned *)(IduCtrlBase + 0x602A))
#define OSD_IN_WIDTH         *((volatile unsigned *)(IduCtrlBase + 0x602C))
#define OSD_END_X            *((volatile unsigned *)(IduCtrlBase + 0x6030))
#define OSD_START_X          *((volatile unsigned *)(IduCtrlBase + 0x6034))
#define TEST_COLOR_DATA      *((volatile unsigned *)(IduCtrlBase + 0x6038))
#define OSD_EDGE_COLOR       *((volatile unsigned *)(IduCtrlBase + 0x603C))
#define OSD_BRI_CTRL         *((volatile unsigned *)(IduCtrlBase + 0x6040))
#define OSD_BRI_ADDR         *((volatile unsigned *)(IduCtrlBase + 0x6044))
#define OSD_BRI_SREIDE       *((volatile unsigned *)(IduCtrlBase + 0x6048))
#define OSD_BRI_WIDTH        *((volatile unsigned *)(IduCtrlBase + 0x604C))
#define OSD_BRI_SIZE         *((volatile unsigned *)(IduCtrlBase + 0x6050))

/*****************************************************************************/
/* IDU2 Display Bridge Unit Control Register                                 */
/*****************************************************************************/
#define BRI2_CTRL_REG         *((volatile unsigned *)(TV2CtrBase + 0x6000))
#define BRI2_IADDR_Y          *((volatile unsigned *)(TV2CtrBase + 0x6004))
#define BRI2_IADDR_C          *((volatile unsigned *)(TV2CtrBase + 0x6008))
#define BRI2_STRIDE           *((volatile unsigned *)(TV2CtrBase + 0x600c))
#define BRI2_IN_SIZE          *((volatile unsigned *)(TV2CtrBase + 0x6010))
#define BRI2_OUT_SIZE         *((volatile unsigned *)(TV2CtrBase + 0x6014))
#define BRI2_SCCTRL_MODE      *((volatile unsigned *)(TV2CtrBase + 0x601c))


/*****************************************************************************/
/* Deinterlace Unit Control(DIU) Register                                    */
/*****************************************************************************/
#define DIU_CTL              *((volatile unsigned *)(DIU_CtrlBase + 0x0000))
#define DIU_INTRPT           *((volatile unsigned *)(DIU_CtrlBase + 0x0004))
#define DIU_PREVADDR_Y       *((volatile unsigned *)(DIU_CtrlBase + 0x0008))
#define DIU_PREVADDR_C       *((volatile unsigned *)(DIU_CtrlBase + 0x000c))
#define DIU_CURRADDR_Y       *((volatile unsigned *)(DIU_CtrlBase + 0x0010))
#define DIU_CURRADDR_C       *((volatile unsigned *)(DIU_CtrlBase + 0x0014))
#define DIU_FRAME_STRIDE     *((volatile unsigned *)(DIU_CtrlBase + 0x0018))
#define DIU_WINSIZE          *((volatile unsigned *)(DIU_CtrlBase + 0x001c))
#define DIU_MOTIONPARAM      *((volatile unsigned *)(DIU_CtrlBase + 0x0020))
#define DIU_MVCOUNT          *((volatile unsigned *)(DIU_CtrlBase + 0x0024))
#define DIU_Y_DIFSUM         *((volatile unsigned *)(DIU_CtrlBase + 0x0028))
#define DIU_CB_DIFSUM        *((volatile unsigned *)(DIU_CtrlBase + 0x002c))
#define DIU_CR_DIFSUM        *((volatile unsigned *)(DIU_CtrlBase + 0x0030))

/*****************************************************************************/
/* Image Scaling Unit Control Register                                   */
/*****************************************************************************/
#define IsuSCA_EN           *((volatile unsigned *)(IsuCtrlBase))
#define IsuSCA_MODE         *((volatile unsigned *)(IsuCtrlBase + 0x0004))
#define IsuSCA_FUN          *((volatile unsigned *)(IsuCtrlBase + 0x0008))
#define IsuSCA_INTC         *((volatile unsigned *)(IsuCtrlBase + 0x000c))
#define IsuSRC_SIZE         *((volatile unsigned *)(IsuCtrlBase + 0x0020))
#define IsuPK_SIZE          *((volatile unsigned *)(IsuCtrlBase + 0x0024))
#define IsuPN_SIZE          *((volatile unsigned *)(IsuCtrlBase + 0x0028))
#define IsuMSF_STEP         *((volatile unsigned *)(IsuCtrlBase + 0x0030))
#define IsuMSF_PHA          *((volatile unsigned *)(IsuCtrlBase + 0x0034))
#define IsuSSF_STEP         *((volatile unsigned *)(IsuCtrlBase + 0x0038))
#define IsuSSF_PHA          *((volatile unsigned *)(IsuCtrlBase + 0x003C))
#define IsuSRC_IADDR0       *((volatile unsigned *)(IsuCtrlBase + 0x0040))
#define IsuSRC_IADDR1       *((volatile unsigned *)(IsuCtrlBase + 0x0044))
#define IsuSRC_IADDR2       *((volatile unsigned *)(IsuCtrlBase + 0x0048))
#define IsuSRC_STRIDE       *((volatile unsigned *)(IsuCtrlBase + 0x004C))
#define IsuPK_IADDR0        *((volatile unsigned *)(IsuCtrlBase + 0x0050))
#define IsuPK_IADDR1        *((volatile unsigned *)(IsuCtrlBase + 0x0054))
#define IsuPK_IADDR2        *((volatile unsigned *)(IsuCtrlBase + 0x0058))
#define IsuPK_STRIDE        *((volatile unsigned *)(IsuCtrlBase + 0x005C))
#define IsuPN_YIADDR0       *((volatile unsigned *)(IsuCtrlBase + 0x0060))
#define IsuPN_CIADDR0       *((volatile unsigned *)(IsuCtrlBase + 0x0064))
#define IsuPN_YIADDR1       *((volatile unsigned *)(IsuCtrlBase + 0x0068))
#define IsuPN_CIADDR1       *((volatile unsigned *)(IsuCtrlBase + 0x006C))
#define IsuPN_YIADDR2       *((volatile unsigned *)(IsuCtrlBase + 0x0070))
#define IsuPN_CIADDR2       *((volatile unsigned *)(IsuCtrlBase + 0x0074))
#define IsuPN_STRIDE        *((volatile unsigned *)(IsuCtrlBase + 0x0078))
#define IsuOVL_WSP          *((volatile unsigned *)(IsuCtrlBase + 0x0080))
#define IsuOVL_WEP          *((volatile unsigned *)(IsuCtrlBase + 0x0084))
#define IsuOVL_IADDR        *((volatile unsigned *)(IsuCtrlBase + 0x0088))
#define IsuOVL_STRIDE       *((volatile unsigned *)(IsuCtrlBase + 0x008C))
#define IsuFIFO_TH          *((volatile unsigned *)(IsuCtrlBase + 0x0090))
#define IsuOVLPAL1          *((volatile unsigned *)(IsuCtrlBase + 0x00A4))
#define IsuOVLPAL2          *((volatile unsigned *)(IsuCtrlBase + 0x00A8))
#define IsuOVLPAL3          *((volatile unsigned *)(IsuCtrlBase + 0x00AC))

#define IsuScUpCntr         *((volatile unsigned *)(IsuCtrlBase + 0x00D0))
#define IsuScUpInWindow     *((volatile unsigned *)(IsuCtrlBase + 0x00D4))
#define IsuScUpOutWindow    *((volatile unsigned *)(IsuCtrlBase + 0x00D8))
#define IsuScUpStartPos     *((volatile unsigned *)(IsuCtrlBase + 0x00DC))
#define IsuScUpDebugInfo    *((volatile unsigned *)(IsuCtrlBase + 0x00E0))
#define IsuScUpFIFOCntr     *((volatile unsigned *)(IsuCtrlBase + 0x00E4))
#define IsuScUpFIFODepth    *((volatile unsigned *)(IsuCtrlBase + 0x00F0)) //scalling performance sp_cbcr[31:24], sp_y[23:16], cbcr[15:8], y[7:0]

/*****************************************************************************/
/* JPEG Control Register                                     */
/*****************************************************************************/
#define JpegCtrl                *((volatile unsigned *)(JpegCtrlBase))
#define JpegImageSize           *((volatile unsigned *)(JpegCtrlBase + 0x0004))
#define JpegRestartInterval     *((volatile unsigned *)(JpegCtrlBase + 0x0008))
#define JpegStreamAddr          *((volatile unsigned *)(JpegCtrlBase + 0x000c))
#define JpegStreamSize          *((volatile unsigned *)(JpegCtrlBase + 0x0010))
#define JpegImageAddr0          *((volatile unsigned *)(JpegCtrlBase + 0x0014))
#define JpegImageAddr1          *((volatile unsigned *)(JpegCtrlBase + 0x0018))
#define JpegImageAddr2          *((volatile unsigned *)(JpegCtrlBase + 0x001c))
#define JpegIntEna              *((volatile unsigned *)(JpegCtrlBase + 0x0020))
#define JpegIntStat             *((volatile unsigned *)(JpegCtrlBase + 0x0024))
#define JpegHufAc0MinTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0028))
#define JpegHufAc0MinTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x002c))
#define JpegHufAc0MinTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x0030))
#define JpegHufAc0MinTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x0034))
/*BJ 0523 S*/
#define JpegHufAc0DifTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0038))
#define JpegHufAc0DifTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x003c))
#define JpegHufAc0DifTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x0040))
#define JpegHufAc0DifTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x0044))
#define JpegHufAc0DifTblAddr4   *((volatile unsigned *)(JpegCtrlBase + 0x0048))
#define JpegHufAc0DifTblAddr5   *((volatile unsigned *)(JpegCtrlBase + 0x004c))
#define JpegHufAc0DifTblAddr6   *((volatile unsigned *)(JpegCtrlBase + 0x0050))
#define JpegHufAc0DifTblAddr7   *((volatile unsigned *)(JpegCtrlBase + 0x0054))
#define JpegHufAc1MinTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0058))
#define JpegHufAc1MinTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x005c))
#define JpegHufAc1MinTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x0060))
#define JpegHufAc1MinTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x0064))
#define JpegHufAc1DifTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0068))
#define JpegHufAc1DifTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x006c))
#define JpegHufAc1DifTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x0070))
#define JpegHufAc1DifTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x0074))
#define JpegHufAc1DifTblAddr4   *((volatile unsigned *)(JpegCtrlBase + 0x0078))
#define JpegHufAc1DifTblAddr5   *((volatile unsigned *)(JpegCtrlBase + 0x007c))
#define JpegHufAc1DifTblAddr6   *((volatile unsigned *)(JpegCtrlBase + 0x0080))
#define JpegHufAc1DifTblAddr7   *((volatile unsigned *)(JpegCtrlBase + 0x0084))
#define JpegHufDc0MinTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0088))
#define JpegHufDc0MinTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x008c))
#define JpegHufDc0MinTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x0090))
#define JpegHufDc0MinTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x0094))
#define JpegHufDc0DifTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x0098))
#define JpegHufDc0DifTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x009c))
#define JpegHufDc0DifTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x00a0))
#define JpegHufDc0DifTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x00a4))
#define JpegHufDc0DifTblAddr4   *((volatile unsigned *)(JpegCtrlBase + 0x00a8))
#define JpegHufDc0DifTblAddr5   *((volatile unsigned *)(JpegCtrlBase + 0x00ac))
#define JpegHufDc0DifTblAddr6   *((volatile unsigned *)(JpegCtrlBase + 0x00b0))
#define JpegHufDc0DifTblAddr7   *((volatile unsigned *)(JpegCtrlBase + 0x00b4))
#define JpegHufDc1MinTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x00b8))
#define JpegHufDc1MinTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x00bc))
#define JpegHufDc1MinTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x00c0))
#define JpegHufDc1MinTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x00c4))
#define JpegHufDc1DifTblAddr0   *((volatile unsigned *)(JpegCtrlBase + 0x00c8))
#define JpegHufDc1DifTblAddr1   *((volatile unsigned *)(JpegCtrlBase + 0x00cc))
#define JpegHufDc1DifTblAddr2   *((volatile unsigned *)(JpegCtrlBase + 0x00d0))
#define JpegHufDc1DifTblAddr3   *((volatile unsigned *)(JpegCtrlBase + 0x00d4))
#define JpegHufDc1DifTblAddr4   *((volatile unsigned *)(JpegCtrlBase + 0x00d8))
#define JpegHufDc1DifTblAddr5   *((volatile unsigned *)(JpegCtrlBase + 0x00dc))
#define JpegHufDc1DifTblAddr6   *((volatile unsigned *)(JpegCtrlBase + 0x00e0))
#define JpegHufDc1DifTblAddr7   *((volatile unsigned *)(JpegCtrlBase + 0x00e4))



#define JpegQuantLum            *((volatile unsigned *)(JpegCtrlBase + 0xe000))
#define JpegQuantChr            *((volatile unsigned *)(JpegCtrlBase + 0xe100))
#define JPEGHuffmanEncTb1Addr   *((volatile unsigned *)(JpegCtrlBase + 0xc000))
#define JPEG_HUFF_DEC_DC0_BASE  *((volatile unsigned *)(JpegCtrlBase + 0xc000))
#define JPEG_HUFF_DEC_AC0_BASE  *((volatile unsigned *)(JpegCtrlBase + 0xc040))
#define JPEG_HUFF_DEC_DC1_BASE  *((volatile unsigned *)(JpegCtrlBase + 0xc300))
#define JPEG_HUFF_DEC_AC1_BASE  *((volatile unsigned *)(JpegCtrlBase + 0xc340))

/*****************************************************************************/
/* JPEG Bridge Unit Control Register                                     */
/*****************************************************************************/
#define JPEG_BRI_CTRL           *((volatile unsigned *)(JpegCtrlBase + 0xf000))
#define JPEG_BRI_ADDR_Y         *((volatile unsigned *)(JpegCtrlBase + 0xf004))
#define JPEG_BRI_ADDR_C         *((volatile unsigned *)(JpegCtrlBase + 0xf008))
#define JPEG_BRI_STRIDE         *((volatile unsigned *)(JpegCtrlBase + 0xf00c))
#define JPEG_BRI_SIZE           *((volatile unsigned *)(JpegCtrlBase + 0xf010))

/*BJ 0523 E*/
/*****************************************************************************/
/* MPEG4 Control Register                                    */
/*****************************************************************************/
#define Mpeg4Ctrl               *((volatile unsigned *)(Mpeg4CtrlBase))
#define Mpeg4FrameSize          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0100))
#define Mpeg4MbParam            *((volatile unsigned *)(Mpeg4CtrlBase + 0x0104))
#define Mpeg4SourceStride       *((volatile unsigned *)(Mpeg4CtrlBase + 0x0108))
#define Mpeg4ErrResil           *((volatile unsigned *)(Mpeg4CtrlBase + 0x0200))
#define Mpeg4DecVidPkt          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0204))
#define Mpeg4MeThresh1          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0300))
#define Mpeg4MeThresh2          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0304))
#define Mpeg4SadSum             *((volatile unsigned *)(Mpeg4CtrlBase + 0x0308))
#define Mpeg4IntraMbRefresh     *((volatile unsigned *)(Mpeg4CtrlBase + 0x030c))
#define Mpeg4IntEna             *((volatile unsigned *)(Mpeg4CtrlBase + 0x0400))
#define Mpeg4IntStat            *((volatile unsigned *)(Mpeg4CtrlBase + 0x0404))
#define Mpeg4StreamAddr         *((volatile unsigned *)(Mpeg4CtrlBase + 0x0500))
#define Mpeg4StreamStartBit     *((volatile unsigned *)(Mpeg4CtrlBase + 0x0504))
#define Mpeg4StreamStartWord    *((volatile unsigned *)(Mpeg4CtrlBase + 0x0508))
#define Mpeg4EncStreamSize      *((volatile unsigned *)(Mpeg4CtrlBase + 0x050c))
#define Mpeg4VopParam           *((volatile unsigned *)(Mpeg4CtrlBase + 0x0510))
#define Mpeg4DecStreamSize      *((volatile unsigned *)(Mpeg4CtrlBase + 0x0514))
#define Mpeg4MvBufAddr          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0600))
#define Mpeg4CurrRawYAddr       *((volatile unsigned *)(Mpeg4CtrlBase + 0x0604))
#define Mpeg4CurrRawCbAddr      *((volatile unsigned *)(Mpeg4CtrlBase + 0x0608))
#define Mpeg4CurrRawCrAddr      *((volatile unsigned *)(Mpeg4CtrlBase + 0x060c))
#define Mpeg4CurrRecInYAddr     *((volatile unsigned *)(Mpeg4CtrlBase + 0x0610))
#define Mpeg4CurrRecInCbAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x0614))
#define Mpeg4CurrRecInCrAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x0618))
#define Mpeg4CurrRecOutYAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x061c))
#define Mpeg4CurrRecOutCbAddr   *((volatile unsigned *)(Mpeg4CtrlBase + 0x0620))
#define Mpeg4CurrRecOutCrAddr   *((volatile unsigned *)(Mpeg4CtrlBase + 0x0624))
#define Mpeg4PrevRecInYAddr     *((volatile unsigned *)(Mpeg4CtrlBase + 0x0628))
#define Mpeg4PrevRecInCbAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x062c))
#define Mpeg4PrevRecInCrAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x0630))
#define Mpeg4PrevRecOutYAddr    *((volatile unsigned *)(Mpeg4CtrlBase + 0x0634))
#define Mpeg4PrevRecOutCbAddr   *((volatile unsigned *)(Mpeg4CtrlBase + 0x0638))
#define Mpeg4PrevRecOutCrAddr   *((volatile unsigned *)(Mpeg4CtrlBase + 0x063c))
#define Mpeg4DbgSwitch          *((volatile unsigned *)(Mpeg4CtrlBase + 0x0700))
#define Mpeg4DbgPredAddr        *((volatile unsigned *)(Mpeg4CtrlBase + 0x0704))
#define Mpeg4DbgMidBufAddr      *((volatile unsigned *)(Mpeg4CtrlBase + 0x0708))

/*****************************************************************************/
/* H264 Encoder Control Register                                    */
/*****************************************************************************/
#define H264ENC_ADDR_SR0            *((volatile unsigned *)(H264Enc_CtrlBase))
#define H264ENC_ADDR_SH0            *((volatile unsigned *)(H264Enc_CtrlBase + 0x0008))
#define H264ENC_ADDR_SH1            *((volatile unsigned *)(H264Enc_CtrlBase + 0x000c))
#define H264ENC_ADDR_ENCODE_CTL0    *((volatile unsigned *)(H264Enc_CtrlBase + 0x0018))
#define H264ENC_ADDR_FME_CTL        *((volatile unsigned *)(H264Enc_CtrlBase + 0x001c))
#define H264ENC_ADDR_SLICE_REG0     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0020))
#define H264ENC_ADDR_SLICE_REG1     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0024))
#define H264ENC_ADDR_SLICE_REG2     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0028))
#define H264ENC_INTERRUPT           *((volatile unsigned *)(H264Enc_CtrlBase + 0x002c))
#define H264ENC_INT_MASK            *((volatile unsigned *)(H264Enc_CtrlBase + 0x0030))
#define H264ENC_ADDR_BASE_Y_CUR     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0104))
#define H264ENC_ADDR_BASE_U_CUR     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0108))
#define H264ENC_ADDR_BASE_V_CUR     *((volatile unsigned *)(H264Enc_CtrlBase + 0x010c))
#define H264ENC_ADDR_BASE_Y_REF     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0110))
#define H264ENC_ADDR_BASE_UV_REF    *((volatile unsigned *)(H264Enc_CtrlBase + 0x0114))
#define H264ENC_ADDR_BASE_Y_DP      *((volatile unsigned *)(H264Enc_CtrlBase + 0x0118))
#define H264ENC_ADDR_BASE_UV_DP     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0124))
#define H264ENC_ADDR_BASE_ILF       *((volatile unsigned *)(H264Enc_CtrlBase + 0x0128))
#define H264ENC_ADDR_BASE_INTRA     *((volatile unsigned *)(H264Enc_CtrlBase + 0x012c))
#define H264ENC_ADDR_BASE_BS        *((volatile unsigned *)(H264Enc_CtrlBase + 0x0130))
#define H264ENC_ADDR_OFFSET_ILF     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0134))
#define H264ENC_ADDR_OFFSET_INTRA   *((volatile unsigned *)(H264Enc_CtrlBase + 0x0138))
#define H264ENC_EN_ENC_SWRST        *((volatile unsigned *)(H264Enc_CtrlBase + 0x013c))
#define H264ENC_ADDR_SEL_UV         *((volatile unsigned *)(H264Enc_CtrlBase + 0x0140))
#define H264ENC_ADDR_OFFSET_LINE    *((volatile unsigned *)(H264Enc_CtrlBase + 0x0144))
#define H264ENC_ADDR_SPECIAL_CMD    *((volatile unsigned *)(H264Enc_CtrlBase + 0x0148))
#define H264ENC_ADDR_INTRA_MB       *((volatile unsigned *)(H264Enc_CtrlBase + 0x014c))
#define H264ENC_ADDR_TEXTURE_BIT    *((volatile unsigned *)(H264Enc_CtrlBase + 0x0180))
#define H264ENC_ADDR_HEADER_BIT     *((volatile unsigned *)(H264Enc_CtrlBase + 0x0184))
#define H264ENC_ADDR_BEST_COST      *((volatile unsigned *)(H264Enc_CtrlBase + 0x0188))
#define H264ENC_ADDR_TOTAL_BIT      *((volatile unsigned *)(H264Enc_CtrlBase + 0x018c))
/*****************************************************************************/
/* H264 Decoder Control Register                                             */
/*****************************************************************************/
#define H264DEC_AVC_MIB             *((volatile unsigned *)(H264Dec_CtrlBase))
#define H264DEC_AVC_MIO             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0004))
#define H264DEC_AVC_ICB             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0008))
#define H264DEC_AVC_ICO             *((volatile unsigned *)(H264Dec_CtrlBase + 0x000c))
#define H264DEC_AVC_IPB             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0010))
#define H264DEC_AVC_IPO             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0014))
#define H264DEC_AVC_CFY             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0018))
#define H264DEC_AVC_CFUV            *((volatile unsigned *)(H264Dec_CtrlBase + 0x001c))
#define H264DEC_AVC_DSY             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0028))
#define H264DEC_AVC_DSUV            *((volatile unsigned *)(H264Dec_CtrlBase + 0x002c))
#define H264DEC_AVC_STRP            *((volatile unsigned *)(H264Dec_CtrlBase + 0x0030))
#define H264DEC_AVC_CFO             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0038))
#define H264DEC_AVC_FCS             *((volatile unsigned *)(H264Dec_CtrlBase + 0x003c))
#define H264DEC_AVC_FB0             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0040))
#define H264DEC_AVC_IS              *((volatile unsigned *)(H264Dec_CtrlBase + 0x0100))
#define H264DEC_AVC_IC              *((volatile unsigned *)(H264Dec_CtrlBase + 0x0104))
#define H264DEC_AVC_IRS             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0108))
#define H264DEC_AVC_IMS             *((volatile unsigned *)(H264Dec_CtrlBase + 0x010c))
#define H264DEC_AVC_VSR             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0200))
#define H264DEC_AVC_SOD             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0204))
#define H264DEC_AVC_SFB             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0208))
#define H264DEC_AVC_BDC             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0300))
#define H264DEC_AVC_BDO             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0304))
#define H264DEC_AVC_UVL             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0308))
#define H264DEC_AVC_BCS             *((volatile unsigned *)(H264Dec_CtrlBase + 0x030c))
#define H264DEC_AVC_BSA             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0310))
#define H264DEC_AVC_BSL             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0314))
#define H264DEC_AVC_CBP             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0328))
#define H264DEC_AVC_DIS0            *((volatile unsigned *)(H264Dec_CtrlBase + 0x0400))
#define H264DEC_AVC_DIS1            *((volatile unsigned *)(H264Dec_CtrlBase + 0x0404))
#define H264DEC_AVC_DIS2            *((volatile unsigned *)(H264Dec_CtrlBase + 0x0408))
#define H264DEC_AVC_SL0             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0500))
#define H264DEC_AVC_SL1             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0504))
#define H264DEC_AVC_SL2             *((volatile unsigned *)(H264Dec_CtrlBase + 0x0508))

/*****************************************************************************/
/* H1 H264 Encoder Control Register                                    */
/*****************************************************************************/
/* HW ID register, read-only */
#define H1H264_SWREG0               *((volatile unsigned *)(H264Enc_CtrlBase))
/* Encoder interrupt register */
#define H1H264_SWREG1               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0004))     
/* Encoder configuration register */
#define H1H264_SWREG2               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0008))
#define H1H264_SWREG3               *((volatile unsigned *)(H264Enc_CtrlBase + 0x000C))
/* External memory base addresses for encoder input/outputput */
#define H1H264_SWREG5               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0014))     
#define H1H264_SWREG6               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0018))
#define H1H264_SWREG7               *((volatile unsigned *)(H264Enc_CtrlBase + 0x001C))
#define H1H264_SWREG8               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0020))
#define H1H264_SWREG9               *((volatile unsigned *)(H264Enc_CtrlBase + 0x0024))
#define H1H264_SWREG10              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0028))
#define H1H264_SWREG11              *((volatile unsigned *)(H264Enc_CtrlBase + 0x002C))
#define H1H264_SWREG12              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0030))
#define H1H264_SWREG13              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0034))
#define H1H264_SWREG14              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0038))
#define H1H264_SWREG15              *((volatile unsigned *)(H264Enc_CtrlBase + 0x003C))
/* VP8 / H.264 mixed definitions */
#define H1H264_SWREG16              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0040))
#define H1H264_SWREG17              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0044))
#define H1H264_SWREG18              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0048))
#define H1H264_SWREG19              *((volatile unsigned *)(H264Enc_CtrlBase + 0x004C))
/* Mixed definitions JPEG / video */
#define H1H264_SWREG20              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0050))
#define H1H264_SWREG21              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0054))
#define H1H264_SWREG22              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0058))
#define H1H264_SWREG23              *((volatile unsigned *)(H264Enc_CtrlBase + 0x005C))
#define H1H264_SWREG24              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0060))
#define H1H264_SWREG25              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0064))
#define H1H264_SWREG26              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0068))
/* H.264 Rate control registers */
#define H1H264_SWREG27              *((volatile unsigned *)(H264Enc_CtrlBase + 0x006C))
#define H1H264_SWREG28              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0070))
#define H1H264_SWREG29              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0074))
#define H1H264_SWREG30              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0078))
#define H1H264_SWREG31              *((volatile unsigned *)(H264Enc_CtrlBase + 0x007C))
#define H1H264_SWREG32              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0080))
#define H1H264_SWREG33              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0084))
#define H1H264_SWREG34              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0088))
#define H1H264_SWREG35              *((volatile unsigned *)(H264Enc_CtrlBase + 0x008C))
#define H1H264_SWREG36              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0090))
#define H1H264_SWREG37              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0094))
#define H1H264_SWREG38              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0098))
#define H1H264_SWREG39              *((volatile unsigned *)(H264Enc_CtrlBase + 0x009C))
#define H1H264_SWREG40              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00A0))
#define H1H264_SWREG41              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00A4))
#define H1H264_SWREG42              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00A8))
#define H1H264_SWREG43              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00AC))
#define H1H264_SWREG44              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00B0))
#define H1H264_SWREG45              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00B4))
#define H1H264_SWREG46              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00B8))
#define H1H264_SWREG47              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00BC))
#define H1H264_SWREG48              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00C0))
#define H1H264_SWREG49              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00C4))
#define H1H264_SWREG50              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00C8))
#define H1H264_SWREG51              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00CC))
#define H1H264_SWREG52              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00D0))
#define H1H264_SWREG53              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00D4))
#define H1H264_SWREG54              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00D8))
#define H1H264_SWREG55              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00DC))
#define H1H264_SWREG56              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00E0))
#define H1H264_SWREG57              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00E4))
#define H1H264_SWREG58              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00E8))
#define H1H264_SWREG59              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00EC))
#define H1H264_SWREG60              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00F0))
#define H1H264_SWREG61              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00F4))
#define H1H264_SWREG62              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00F8))
#define H1H264_SWREG63              *((volatile unsigned *)(H264Enc_CtrlBase + 0x00FC)) //RO
#define H1H264_SWREG64              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0100))
#define H1H264_SWREG65              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0104))
#define H1H264_SWREG66              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0108))
#define H1H264_SWREG67              *((volatile unsigned *)(H264Enc_CtrlBase + 0x010C))
#define H1H264_SWREG68              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0110))
#define H1H264_SWREG69              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0114))
#define H1H264_SWREG70              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0118))
#define H1H264_SWREG71              *((volatile unsigned *)(H264Enc_CtrlBase + 0x011C))
#define H1H264_SWREG72              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0120))
#define H1H264_SWREG73              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0124))
#define H1H264_SWREG74              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0128))
#define H1H264_SWREG75              *((volatile unsigned *)(H264Enc_CtrlBase + 0x012C))
#define H1H264_SWREG76              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0130))
#define H1H264_SWREG77              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0134))
#define H1H264_SWREG78              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0138))
#define H1H264_SWREG79              *((volatile unsigned *)(H264Enc_CtrlBase + 0x013C))
#define H1H264_SWREG80              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0140))
#define H1H264_SWREG81              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0144))
#define H1H264_SWREG82              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0148))
#define H1H264_SWREG83              *((volatile unsigned *)(H264Enc_CtrlBase + 0x014C))
#define H1H264_SWREG84              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0150))
#define H1H264_SWREG85              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0154))
#define H1H264_SWREG86              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0158))
#define H1H264_SWREG87              *((volatile unsigned *)(H264Enc_CtrlBase + 0x015C))
#define H1H264_SWREG88              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0160))
#define H1H264_SWREG89              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0164))
#define H1H264_SWREG90              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0168))
#define H1H264_SWREG91              *((volatile unsigned *)(H264Enc_CtrlBase + 0x016C))
#define H1H264_SWREG92              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0170))
#define H1H264_SWREG93              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0174))
#define H1H264_SWREG94              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0178))
#define H1H264_SWREG95              *((volatile unsigned *)(H264Enc_CtrlBase + 0x017C))
#define H1H264_SWREG96              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0180))
#define H1H264_SWREG97              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0184))
#define H1H264_SWREG98              *((volatile unsigned *)(H264Enc_CtrlBase + 0x0188))
#define H1H264_SWREG99              *((volatile unsigned *)(H264Enc_CtrlBase + 0x018C))
#define H1H264_SWREG100             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0190))
#define H1H264_SWREG101             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0194))
#define H1H264_SWREG102             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0198))
#define H1H264_SWREG103             *((volatile unsigned *)(H264Enc_CtrlBase + 0x019C))
#define H1H264_SWREG104             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01A0))
#define H1H264_SWREG105             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01A4))
#define H1H264_SWREG106             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01A8))
#define H1H264_SWREG107             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01AC))
#define H1H264_SWREG108             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01B0))
#define H1H264_SWREG109             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01B4))
#define H1H264_SWREG110             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01B8))
#define H1H264_SWREG111             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01BC))
#define H1H264_SWREG112             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01C0))
#define H1H264_SWREG113             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01C4))
#define H1H264_SWREG114             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01C8))
#define H1H264_SWREG115             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01CC))
#define H1H264_SWREG116             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01D0))
#define H1H264_SWREG117             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01D4))
#define H1H264_SWREG118             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01D8))
#define H1H264_SWREG119             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01DC))
#define H1H264_SWREG120             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01E0))
#define H1H264_SWREG121             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01E4))
#define H1H264_SWREG122             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01E8))
#define H1H264_SWREG123             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01EC))
#define H1H264_SWREG124             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01F0))
#define H1H264_SWREG125             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01F4))
#define H1H264_SWREG126             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01F8))
#define H1H264_SWREG127             *((volatile unsigned *)(H264Enc_CtrlBase + 0x01FC))
#define H1H264_SWREG128             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0200))
#define H1H264_SWREG129             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0204))
#define H1H264_SWREG130             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0208))
#define H1H264_SWREG131             *((volatile unsigned *)(H264Enc_CtrlBase + 0x020C))
#define H1H264_SWREG132             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0210))
#define H1H264_SWREG133             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0214))
#define H1H264_SWREG134             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0218))
#define H1H264_SWREG135             *((volatile unsigned *)(H264Enc_CtrlBase + 0x021C))
#define H1H264_SWREG136             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0220))
#define H1H264_SWREG137             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0224))
#define H1H264_SWREG138             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0228))
#define H1H264_SWREG139             *((volatile unsigned *)(H264Enc_CtrlBase + 0x022C))
#define H1H264_SWREG140             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0230))
#define H1H264_SWREG141             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0234))
#define H1H264_SWREG142             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0238))
#define H1H264_SWREG143             *((volatile unsigned *)(H264Enc_CtrlBase + 0x023C))
#define H1H264_SWREG144             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0240))
#define H1H264_SWREG145             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0244))
#define H1H264_SWREG146             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0248))
#define H1H264_SWREG147             *((volatile unsigned *)(H264Enc_CtrlBase + 0x024C))
#define H1H264_SWREG148             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0250))
#define H1H264_SWREG149             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0254))
#define H1H264_SWREG150             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0258))
#define H1H264_SWREG151             *((volatile unsigned *)(H264Enc_CtrlBase + 0x025C))
#define H1H264_SWREG152             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0260))
#define H1H264_SWREG153             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0264))
#define H1H264_SWREG154             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0268))
#define H1H264_SWREG155             *((volatile unsigned *)(H264Enc_CtrlBase + 0x026C))
#define H1H264_SWREG156             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0270))
#define H1H264_SWREG157             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0274))
#define H1H264_SWREG158             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0278))
#define H1H264_SWREG159             *((volatile unsigned *)(H264Enc_CtrlBase + 0x027C))
#define H1H264_SWREG160             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0280))
#define H1H264_SWREG161             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0284))
#define H1H264_SWREG162             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0288))
#define H1H264_SWREG163             *((volatile unsigned *)(H264Enc_CtrlBase + 0x028C))
#define H1H264_SWREG164             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0290))
#define H1H264_SWREG165             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0294))
#define H1H264_SWREG166             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0298))
#define H1H264_SWREG167             *((volatile unsigned *)(H264Enc_CtrlBase + 0x029C))
#define H1H264_SWREG168             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02A0))
#define H1H264_SWREG169             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02A4))
#define H1H264_SWREG170             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02A8))
#define H1H264_SWREG171             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02AC))
#define H1H264_SWREG172             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02B0))
#define H1H264_SWREG173             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02B4))
#define H1H264_SWREG174             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02B8))
#define H1H264_SWREG175             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02BC))
#define H1H264_SWREG176             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02C0))
#define H1H264_SWREG177             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02C4))
#define H1H264_SWREG178             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02C8))
#define H1H264_SWREG179             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02CC))
#define H1H264_SWREG180             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02D0))
#define H1H264_SWREG181             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02D4))
#define H1H264_SWREG182             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02D8))
#define H1H264_SWREG183             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02DC))
#define H1H264_SWREG184             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02E0))
#define H1H264_SWREG185             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02E4))
#define H1H264_SWREG186             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02E8))
#define H1H264_SWREG187             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02EC))
#define H1H264_SWREG188             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02F0))
#define H1H264_SWREG189             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02F4))
#define H1H264_SWREG190             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02F8))
#define H1H264_SWREG191             *((volatile unsigned *)(H264Enc_CtrlBase + 0x02FC))
#define H1H264_SWREG192             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0300))
#define H1H264_SWREG193             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0304))
#define H1H264_SWREG194             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0308))
#define H1H264_SWREG195             *((volatile unsigned *)(H264Enc_CtrlBase + 0x030C))
#define H1H264_SWREG196             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0310))
#define H1H264_SWREG197             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0314))
#define H1H264_SWREG198             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0318))
#define H1H264_SWREG199             *((volatile unsigned *)(H264Enc_CtrlBase + 0x031C))
#define H1H264_SWREG200             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0320))
#define H1H264_SWREG201             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0324))
#define H1H264_SWREG202             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0328))
#define H1H264_SWREG203             *((volatile unsigned *)(H264Enc_CtrlBase + 0x032C))
#define H1H264_SWREG204             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0330))
#define H1H264_SWREG205             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0334))
#define H1H264_SWREG206             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0338))
#define H1H264_SWREG207             *((volatile unsigned *)(H264Enc_CtrlBase + 0x033C))
#define H1H264_SWREG208             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0340))
#define H1H264_SWREG209             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0344))
#define H1H264_SWREG210             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0348))
#define H1H264_SWREG211             *((volatile unsigned *)(H264Enc_CtrlBase + 0x034C))
#define H1H264_SWREG212             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0350))
#define H1H264_SWREG213             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0354))
#define H1H264_SWREG214             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0358))
#define H1H264_SWREG215             *((volatile unsigned *)(H264Enc_CtrlBase + 0x035C))
#define H1H264_SWREG216             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0360))
#define H1H264_SWREG217             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0364))
#define H1H264_SWREG218             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0368))
#define H1H264_SWREG219             *((volatile unsigned *)(H264Enc_CtrlBase + 0x036C))
#define H1H264_SWREG220             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0370))
#define H1H264_SWREG221             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0374))
#define H1H264_SWREG222             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0378))
#define H1H264_SWREG223             *((volatile unsigned *)(H264Enc_CtrlBase + 0x037C))
#define H1H264_SWREG224             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0380))
#define H1H264_SWREG225             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0384))
#define H1H264_SWREG226             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0388))
#define H1H264_SWREG227             *((volatile unsigned *)(H264Enc_CtrlBase + 0x038C))
#define H1H264_SWREG228             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0390))
#define H1H264_SWREG229             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0394))
#define H1H264_SWREG230             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0398))
#define H1H264_SWREG231             *((volatile unsigned *)(H264Enc_CtrlBase + 0x039C))
#define H1H264_SWREG232             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03A0))
#define H1H264_SWREG233             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03A4))
#define H1H264_SWREG234             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03A8))
#define H1H264_SWREG235             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03AC))
#define H1H264_SWREG236             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03B0))
#define H1H264_SWREG237             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03B4))
#define H1H264_SWREG238             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03B8))
#define H1H264_SWREG239             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03BC))
#define H1H264_SWREG240             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03C0))
#define H1H264_SWREG241             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03C4))
#define H1H264_SWREG242             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03C8))
#define H1H264_SWREG243             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03CC))
#define H1H264_SWREG244             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03D0))
#define H1H264_SWREG245             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03D4))
#define H1H264_SWREG246             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03D8))
#define H1H264_SWREG247             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03DC))
#define H1H264_SWREG248             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03E0))
#define H1H264_SWREG249             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03E4))
#define H1H264_SWREG250             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03E8))
#define H1H264_SWREG251             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03EC))
#define H1H264_SWREG252             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03F0))
#define H1H264_SWREG253             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03F4))
#define H1H264_SWREG254             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03F8))
#define H1H264_SWREG255             *((volatile unsigned *)(H264Enc_CtrlBase + 0x03FC))
#define H1H264_SWREG256             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0400))
#define H1H264_SWREG257             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0404))
#define H1H264_SWREG258             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0408))
#define H1H264_SWREG259             *((volatile unsigned *)(H264Enc_CtrlBase + 0x040C))
#define H1H264_SWREG260             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0410))
#define H1H264_SWREG261             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0414))
#define H1H264_SWREG262             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0418))
#define H1H264_SWREG263             *((volatile unsigned *)(H264Enc_CtrlBase + 0x041C))
#define H1H264_SWREG264             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0420))
#define H1H264_SWREG265             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0424))
#define H1H264_SWREG266             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0428))
#define H1H264_SWREG267             *((volatile unsigned *)(H264Enc_CtrlBase + 0x042C))
#define H1H264_SWREG268             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0430))
#define H1H264_SWREG269             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0434))
#define H1H264_SWREG270             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0438))
#define H1H264_SWREG271             *((volatile unsigned *)(H264Enc_CtrlBase + 0x043C))
#define H1H264_SWREG272             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0440))
#define H1H264_SWREG273             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0444))
#define H1H264_SWREG274             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0448))
#define H1H264_SWREG275             *((volatile unsigned *)(H264Enc_CtrlBase + 0x044C))
#define H1H264_SWREG276             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0450))
#define H1H264_SWREG277             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0454))
#define H1H264_SWREG278             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0458))
#define H1H264_SWREG279             *((volatile unsigned *)(H264Enc_CtrlBase + 0x045C))
#define H1H264_SWREG280             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0460))
#define H1H264_SWREG281             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0464))
#define H1H264_SWREG282             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0468))
#define H1H264_SWREG283             *((volatile unsigned *)(H264Enc_CtrlBase + 0x046C))
#define H1H264_SWREG284             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0470))
#define H1H264_SWREG285             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0474))
#define H1H264_SWREG286             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0478))
#define H1H264_SWREG287             *((volatile unsigned *)(H264Enc_CtrlBase + 0x047C))
#define H1H264_SWREG288             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0480))
#define H1H264_SWREG289             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0484))
#define H1H264_SWREG290             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0488))
#define H1H264_SWREG291             *((volatile unsigned *)(H264Enc_CtrlBase + 0x048C))
#define H1H264_SWREG292             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0490))
#define H1H264_SWREG293             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0494))
#define H1H264_SWREG294             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0498))
#define H1H264_SWREG295             *((volatile unsigned *)(H264Enc_CtrlBase + 0x049C))
#define H1H264_SWREG296             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04A0))
#define H1H264_SWREG297             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04A4))
#define H1H264_SWREG298             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04A8))
#define H1H264_SWREG299             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04AC))
#define H1H264_SWREG300             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04B0))
#define H1H264_SWREG301             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04B4))
#define H1H264_SWREG302             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04B8))
#define H1H264_SWREG303             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04BC))
#define H1H264_SWREG304             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04C0))
#define H1H264_SWREG305             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04C4))
#define H1H264_SWREG306             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04C8))
#define H1H264_SWREG307             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04CC))
#define H1H264_SWREG308             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04D0))
#define H1H264_SWREG309             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04D4))
#define H1H264_SWREG310             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04D8))
#define H1H264_SWREG311             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04DC))
#define H1H264_SWREG312             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04E0))
#define H1H264_SWREG313             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04E4))
#define H1H264_SWREG314             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04E8))
#define H1H264_SWREG315             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04EC))
#define H1H264_SWREG316             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04F0))
#define H1H264_SWREG317             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04F4))
#define H1H264_SWREG318             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04F8))
#define H1H264_SWREG319             *((volatile unsigned *)(H264Enc_CtrlBase + 0x04FC))
#define H1H264_SWREG320             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0500))
#define H1H264_SWREG321             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0504))
#define H1H264_SWREG322             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0508))
#define H1H264_SWREG323             *((volatile unsigned *)(H264Enc_CtrlBase + 0x050C))
#define H1H264_SWREG324             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0510))
#define H1H264_SWREG325             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0514))
#define H1H264_SWREG326             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0518))
#define H1H264_SWREG327             *((volatile unsigned *)(H264Enc_CtrlBase + 0x051C))
#define H1H264_SWREG328             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0520))
#define H1H264_SWREG329             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0524))
#define H1H264_SWREG330             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0528))
#define H1H264_SWREG331             *((volatile unsigned *)(H264Enc_CtrlBase + 0x052C))
#define H1H264_SWREG332             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0530))
#define H1H264_SWREG333             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0534))
#define H1H264_SWREG334             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0538))
#define H1H264_SWREG335             *((volatile unsigned *)(H264Enc_CtrlBase + 0x053C))
#define H1H264_SWREG336             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0540))
#define H1H264_SWREG337             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0544))
#define H1H264_SWREG338             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0548))
#define H1H264_SWREG339             *((volatile unsigned *)(H264Enc_CtrlBase + 0x054C))
#define H1H264_SWREG340             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0550))
#define H1H264_SWREG341             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0554))
#define H1H264_SWREG342             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0558))
#define H1H264_SWREG343             *((volatile unsigned *)(H264Enc_CtrlBase + 0x055C))
#define H1H264_SWREG344             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0560))
#define H1H264_SWREG345             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0564))
#define H1H264_SWREG346             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0568))
#define H1H264_SWREG347             *((volatile unsigned *)(H264Enc_CtrlBase + 0x056C))
#define H1H264_SWREG348             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0570))
#define H1H264_SWREG349             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0574))
#define H1H264_SWREG350             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0578))
#define H1H264_SWREG351             *((volatile unsigned *)(H264Enc_CtrlBase + 0x057C))
#define H1H264_SWREG352             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0580))
#define H1H264_SWREG353             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0584))
#define H1H264_SWREG354             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0588))
#define H1H264_SWREG355             *((volatile unsigned *)(H264Enc_CtrlBase + 0x058C))
#define H1H264_SWREG356             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0590))
#define H1H264_SWREG357             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0594))
#define H1H264_SWREG358             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0598))
#define H1H264_SWREG359             *((volatile unsigned *)(H264Enc_CtrlBase + 0x059C))
#define H1H264_SWREG360             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05A0))
#define H1H264_SWREG361             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05A4))
#define H1H264_SWREG362             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05A8))
#define H1H264_SWREG363             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05AC))
#define H1H264_SWREG364             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05B0))
#define H1H264_SWREG365             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05B4))
#define H1H264_SWREG366             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05B8))
#define H1H264_SWREG367             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05BC))
#define H1H264_SWREG368             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05C0))
#define H1H264_SWREG369             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05C4))
#define H1H264_SWREG370             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05C8))
#define H1H264_SWREG371             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05CC))
#define H1H264_SWREG372             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05D0))
#define H1H264_SWREG373             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05D4))
#define H1H264_SWREG374             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05D8))
#define H1H264_SWREG375             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05DC))
#define H1H264_SWREG376             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05E0))
#define H1H264_SWREG377             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05E4))
#define H1H264_SWREG378             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05E8))
#define H1H264_SWREG379             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05EC))
#define H1H264_SWREG380             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05F0))
#define H1H264_SWREG381             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05F4))
#define H1H264_SWREG382             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05F8))
#define H1H264_SWREG383             *((volatile unsigned *)(H264Enc_CtrlBase + 0x05FC))
#define H1H264_SWREG384             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0600))
#define H1H264_SWREG385             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0604))
#define H1H264_SWREG386             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0608))
#define H1H264_SWREG387             *((volatile unsigned *)(H264Enc_CtrlBase + 0x060C))
#define H1H264_SWREG388             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0610))
#define H1H264_SWREG389             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0614))
#define H1H264_SWREG390             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0618))
#define H1H264_SWREG391             *((volatile unsigned *)(H264Enc_CtrlBase + 0x061C))
#define H1H264_SWREG392             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0620))
#define H1H264_SWREG393             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0624))
#define H1H264_SWREG394             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0628))
#define H1H264_SWREG395             *((volatile unsigned *)(H264Enc_CtrlBase + 0x062C))
#define H1H264_SWREG396             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0630))
#define H1H264_SWREG397             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0634))
#define H1H264_SWREG398             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0638))
#define H1H264_SWREG399             *((volatile unsigned *)(H264Enc_CtrlBase + 0x063C))
#define H1H264_SWREG400             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0640))
#define H1H264_SWREG401             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0644))
#define H1H264_SWREG402             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0648))
#define H1H264_SWREG403             *((volatile unsigned *)(H264Enc_CtrlBase + 0x064C))
#define H1H264_SWREG404             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0650))
#define H1H264_SWREG405             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0654))
#define H1H264_SWREG406             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0658))
#define H1H264_SWREG407             *((volatile unsigned *)(H264Enc_CtrlBase + 0x065C))
#define H1H264_SWREG408             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0660))
#define H1H264_SWREG409             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0664))
#define H1H264_SWREG410             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0668))
#define H1H264_SWREG411             *((volatile unsigned *)(H264Enc_CtrlBase + 0x066C))
#define H1H264_SWREG412             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0670))
#define H1H264_SWREG413             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0674))
#define H1H264_SWREG414             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0678))
#define H1H264_SWREG415             *((volatile unsigned *)(H264Enc_CtrlBase + 0x067C))
#define H1H264_SWREG416             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0680))
#define H1H264_SWREG417             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0684))
#define H1H264_SWREG418             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0688))
#define H1H264_SWREG419             *((volatile unsigned *)(H264Enc_CtrlBase + 0x068C))
#define H1H264_SWREG420             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0690))
#define H1H264_SWREG421             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0694))
#define H1H264_SWREG422             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0698))
#define H1H264_SWREG423             *((volatile unsigned *)(H264Enc_CtrlBase + 0x069C))
#define H1H264_SWREG424             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06A0))
#define H1H264_SWREG425             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06A4))
#define H1H264_SWREG426             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06A8))
#define H1H264_SWREG427             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06AC))
#define H1H264_SWREG428             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06B0))
#define H1H264_SWREG429             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06B4))
#define H1H264_SWREG430             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06B8))
#define H1H264_SWREG431             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06BC))
#define H1H264_SWREG432             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06C0))
#define H1H264_SWREG433             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06C4))
#define H1H264_SWREG434             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06C8))
#define H1H264_SWREG435             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06CC))
#define H1H264_SWREG436             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06D0))
#define H1H264_SWREG437             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06D4))
#define H1H264_SWREG438             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06D8))
#define H1H264_SWREG439             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06DC))
#define H1H264_SWREG440             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06E0))
#define H1H264_SWREG441             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06E4))
#define H1H264_SWREG442             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06E8))
#define H1H264_SWREG443             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06EC))
#define H1H264_SWREG444             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06F0))
#define H1H264_SWREG445             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06F4))
#define H1H264_SWREG446             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06F8))
#define H1H264_SWREG447             *((volatile unsigned *)(H264Enc_CtrlBase + 0x06FC))
#define H1H264_SWREG448             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0700))
#define H1H264_SWREG449             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0704))
#define H1H264_SWREG450             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0708))
#define H1H264_SWREG451             *((volatile unsigned *)(H264Enc_CtrlBase + 0x070C))
#define H1H264_SWREG452             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0710))
#define H1H264_SWREG453             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0714))
#define H1H264_SWREG454             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0718))
#define H1H264_SWREG455             *((volatile unsigned *)(H264Enc_CtrlBase + 0x071C))
#define H1H264_SWREG456             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0720))
#define H1H264_SWREG457             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0724))
#define H1H264_SWREG458             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0728))
#define H1H264_SWREG459             *((volatile unsigned *)(H264Enc_CtrlBase + 0x072C))
#define H1H264_SWREG460             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0730))
#define H1H264_SWREG461             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0734))
#define H1H264_SWREG462             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0738))
#define H1H264_SWREG463             *((volatile unsigned *)(H264Enc_CtrlBase + 0x073C))
#define H1H264_SWREG464             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0740))
#define H1H264_SWREG465             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0744))
#define H1H264_SWREG466             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0748))
#define H1H264_SWREG467             *((volatile unsigned *)(H264Enc_CtrlBase + 0x074C))
#define H1H264_SWREG468             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0750))
#define H1H264_SWREG469             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0754))
#define H1H264_SWREG470             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0758))
#define H1H264_SWREG471             *((volatile unsigned *)(H264Enc_CtrlBase + 0x075C))
#define H1H264_SWREG472             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0760))
#define H1H264_SWREG473             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0764))
#define H1H264_SWREG474             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0768))
#define H1H264_SWREG475             *((volatile unsigned *)(H264Enc_CtrlBase + 0x076C))
#define H1H264_SWREG476             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0770))
#define H1H264_SWREG477             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0774))
#define H1H264_SWREG478             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0778))
#define H1H264_SWREG479             *((volatile unsigned *)(H264Enc_CtrlBase + 0x077C))
#define H1H264_SWREG480             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0780))
#define H1H264_SWREG481             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0784))
#define H1H264_SWREG482             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0788))
#define H1H264_SWREG483             *((volatile unsigned *)(H264Enc_CtrlBase + 0x078C))
#define H1H264_SWREG484             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0790))
#define H1H264_SWREG485             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0794))
#define H1H264_SWREG486             *((volatile unsigned *)(H264Enc_CtrlBase + 0x0798))
#define H1H264_SWREG487             *((volatile unsigned *)(H264Enc_CtrlBase + 0x079C))
#define H1H264_SWREG488             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07A0))
#define H1H264_SWREG489             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07A4))
#define H1H264_SWREG490             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07A8))
#define H1H264_SWREG491             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07AC))
#define H1H264_SWREG492             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07B0))
#define H1H264_SWREG493             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07B4))
#define H1H264_SWREG494             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07B8))
#define H1H264_SWREG495             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07BC))
#define H1H264_SWREG496             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07C0))
#define H1H264_SWREG497             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07C4))
#define H1H264_SWREG498             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07C8))
#define H1H264_SWREG499             *((volatile unsigned *)(H264Enc_CtrlBase + 0x07CC))



/*****************************************************************************/
/* APB Processing Unit                                               */
/*****************************************************************************/

/*****************************************************************************/
/* 	Dual Core Interface Control Register                          	     */
/*****************************************************************************/
#define DC_C2STAADDR *((volatile unsigned *)(0x60010050))
#define DC_C2_RST  *((volatile unsigned *)(DualCoreCtrlBase))
#define DC_C2_UDF  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0004))
#define DC_C2_SWI  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0008))
#define DC_C2_PAE  *((volatile unsigned *)(DualCoreCtrlBase+ 0x000C))
#define	DC_C2_DAE  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0010))
#define	DC_C2_RSV  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0014))
#define DC_C2_IRQ  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0018))
#define DC_C2_FIQ  *((volatile unsigned *)(DualCoreCtrlBase+ 0x001C))
#define	DC_C2_FIQ_REG   *((volatile unsigned *)(DualCoreCtrlBase+ 0x0020))
#define	DC_C2_FIQ_MASK  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0024))
#define DC_C2_IRQ_REG  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0028))
#define DC_C2_IRQ_MASK  *((volatile unsigned *)(DualCoreCtrlBase+ 0x002C))
#define	DC_C2_MASK_ALL_FIQ   *((volatile unsigned *)(DualCoreCtrlBase+ 0x0030))
#define	DC_C2_MASK_ALL_IRQ  *((volatile unsigned *)(DualCoreCtrlBase+ 0x0034))

/*****************************************************************************/
/* Dual Core control register                           		     */
/*****************************************************************************/
#define DC_C2_Ctrl		*((volatile unsigned *)(DualCoreCfgBase))
#define DC_C2_INTR		*((volatile unsigned *)(DualCoreCfgBase + 0x0004))
#define DC_MAILBOX_0	*((volatile unsigned *)(DualCoreCfgBase + 0x0008))
#define DC_MAILBOX_1	*((volatile unsigned *)(DualCoreCfgBase + 0x000c))
#define DC_MAILBOX_2	*((volatile unsigned *)(DualCoreCfgBase + 0x0010))
#define DC_MAILBOX_3	*((volatile unsigned *)(DualCoreCfgBase + 0x0014))
#define DC_MAILBOX_4	*((volatile unsigned *)(DualCoreCfgBase + 0x0018))
#define DC_MAILBOX_5	*((volatile unsigned *)(DualCoreCfgBase + 0x001C))
/*****************************************************************************/
/* INT/WDOG Control Register                                     */
/*****************************************************************************/

#define IntFiqInput             *((volatile unsigned *)(IntCtrlBase))
#define IntFiqMask              *((volatile unsigned *)(IntCtrlBase + 0x0004))
#define IntIrqInput             *((volatile unsigned *)(IntCtrlBase + 0x000c))
#define IntIrqMask              *((volatile unsigned *)(IntCtrlBase + 0x0010))
#define GlobalIntMask           *((volatile unsigned *)(IntCtrlBase + 0x0014))

/*****************************************************************************/
/* Static Memory Control Register                                */
/*****************************************************************************/
#define StMemTacc               *((volatile unsigned *)(StMemCtrlBase))
#define StMemTacch              *((volatile unsigned *)(StMemCtrlBase + 0x0004))
#define StMemTwp                *((volatile unsigned *)(StMemCtrlBase + 0x0008))
#define StMemTwph               *((volatile unsigned *)(StMemCtrlBase + 0x000c))

#define ShadowReset             *((volatile unsigned *)(StMemCtrlBase + 0x0020))
#define ShadowUndef             *((volatile unsigned *)(StMemCtrlBase + 0x0024))
#define ShadowSwi               *((volatile unsigned *)(StMemCtrlBase + 0x0028))
#define ShadowPrefAbt           *((volatile unsigned *)(StMemCtrlBase + 0x002c))
#define ShadowDataAbt           *((volatile unsigned *)(StMemCtrlBase + 0x0030))
#define ShadowRsv               *((volatile unsigned *)(StMemCtrlBase + 0x0034))
#define ShadowIrq               *((volatile unsigned *)(StMemCtrlBase + 0x0038))
#define ShadowFiq               *((volatile unsigned *)(StMemCtrlBase + 0x003c))
#define ShadowEnable            *((volatile unsigned *)(StMemCtrlBase + 0x0040))
/*****************************************************************************/
/* Serial Flash Control Register                                 */
/*****************************************************************************/
#define SpiStat                 *((volatile unsigned *)(StMemCtrlBase + 0x0054))
#define SpiIntEn                *((volatile unsigned *)(StMemCtrlBase + 0x0058))
#if ((CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1021A) || (CHIP_OPTION == CHIP_A1022A) || (CHIP_OPTION == CHIP_A1025A))
#define SpiEndian               *((volatile unsigned *)(StMemCtrlBase + 0x00EC))
#endif

#if SWAP_SP1_SP3
#define SpiCtrl                *((volatile unsigned *)(StMemCtrlBase + 0x0090))
#define SpiTxData              *((volatile unsigned *)(StMemCtrlBase + 0x009C))
#define SpiRxData              *((volatile unsigned *)(StMemCtrlBase + 0x00A0))
#define SpiDmaLen              *((volatile unsigned *)(StMemCtrlBase + 0x00A4))
#else
#define SpiCtrl                 *((volatile unsigned *)(StMemCtrlBase + 0x0050))
#define SpiTxData               *((volatile unsigned *)(StMemCtrlBase + 0x005C))
#define SpiRxData               *((volatile unsigned *)(StMemCtrlBase + 0x0060))
#define SpiDmaLen               *((volatile unsigned *)(StMemCtrlBase + 0x0064))
#endif
#define Spi2Ctrl                *((volatile unsigned *)(StMemCtrlBase + 0x0070))
#define Spi2TxData              *((volatile unsigned *)(StMemCtrlBase + 0x007C))
#define Spi2RxData              *((volatile unsigned *)(StMemCtrlBase + 0x0080))
#define Spi2DmaLen              *((volatile unsigned *)(StMemCtrlBase + 0x0084))
#define Spi2CTL2                *((volatile unsigned *)(StMemCtrlBase + 0x0088))

#define Spi3Ctrl                *((volatile unsigned *)(StMemCtrlBase + 0x0090))
#define Spi3TxData              *((volatile unsigned *)(StMemCtrlBase + 0x009C))
#define Spi3RxData              *((volatile unsigned *)(StMemCtrlBase + 0x00A0))
#define Spi3DmaLen              *((volatile unsigned *)(StMemCtrlBase + 0x00A4))
#define Spi3CTL2                *((volatile unsigned *)(StMemCtrlBase + 0x00A8))

#define Spi4Ctrl                *((volatile unsigned *)(StMemCtrlBase + 0x00B0))
#define Spi4TxData              *((volatile unsigned *)(StMemCtrlBase + 0x00BC))
#define Spi4RxData              *((volatile unsigned *)(StMemCtrlBase + 0x00C0))
#define Spi4DmaLen              *((volatile unsigned *)(StMemCtrlBase + 0x00C4))
#define Spi4CTL2                *((volatile unsigned *)(StMemCtrlBase + 0x00C8))

#define Spi5Ctrl                *((volatile unsigned *)(StMemCtrlBase + 0x00D0))
#define Spi5TxData              *((volatile unsigned *)(StMemCtrlBase + 0x00DC))
#define Spi5RxData              *((volatile unsigned *)(StMemCtrlBase + 0x00E0))
#define Spi5DmaLen              *((volatile unsigned *)(StMemCtrlBase + 0x00E4))
#define Spi5CTL2                *((volatile unsigned *)(StMemCtrlBase + 0x00E8))
/*****************************************************************************/
/* GPIO Control Register                                     */
/*****************************************************************************/
/*CY 0718*/
#define Gpio0Dir                *((volatile unsigned *)(GpioCtrlBase))
#define Gpio0Level              *((volatile unsigned *)(GpioCtrlBase + 0x0004))
#define Gpio0IntStat            *((volatile unsigned *)(GpioCtrlBase + 0x0008))
#define Gpio0IntEna             *((volatile unsigned *)(GpioCtrlBase + 0x000c))
#define Gpio0InPullUp           *((volatile unsigned *)(GpioCtrlBase + 0x0010))
#define Gpio0InIntFallEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0014))
#define Gpio0InIntRiseEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0018))
#define Gpio0Ena                *((volatile unsigned *)(GpioCtrlBase + 0x001c))
#define GpioActFlashSelect      *((volatile unsigned *)(GpioCtrlBase + 0x0020))
#define Gpio1InIntFallEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0024))
#define Gpio1InIntRiseEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0028))
#define Gpio1IntStat            *((volatile unsigned *)(GpioCtrlBase + 0x002c))
#define Gpio1Ena                *((volatile unsigned *)(GpioCtrlBase + 0x0030))
#define Gpio1Dir                *((volatile unsigned *)(GpioCtrlBase + 0x0034))
#define Gpio1Level              *((volatile unsigned *)(GpioCtrlBase + 0x0038))
#define Gpio1LevelInt           *((volatile unsigned *)(GpioCtrlBase + 0x003c))
#define Gpio1InPullUp           *((volatile unsigned *)(SysCtrlBase + 0x0064))

#define Gpio2Ena                *((volatile unsigned *)(GpioCtrlBase + 0x0040))
#define Gpio2Dir                *((volatile unsigned *)(GpioCtrlBase + 0x0044))
#define Gpio2Level              *((volatile unsigned *)(GpioCtrlBase + 0x0048))
#define Gpio2InPullUp           *((volatile unsigned *)(GpioCtrlBase + 0x004c))
#define Gpio3Ena                *((volatile unsigned *)(GpioCtrlBase + 0x0050))
#define Gpio3Dir                *((volatile unsigned *)(GpioCtrlBase + 0x0054))
#define Gpio3Level              *((volatile unsigned *)(GpioCtrlBase + 0x0058))
#define Gpio3InPullUp           *((volatile unsigned *)(GpioCtrlBase + 0x005c))

#define Gpio1aLevelInt          *((volatile unsigned *)(GpioCtrlBase + 0x0060))  //gpio1 [31:16]
#define Gpio2IntStat            *((volatile unsigned *)(GpioCtrlBase + 0x008C))
#define Gpio2InIntFallEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0090))
#define Gpio2InIntRiseEdge      *((volatile unsigned *)(GpioCtrlBase + 0x0094))
#define Gpio2LevelInt_Lo        *((volatile unsigned *)(GpioCtrlBase + 0x0098))
#define Gpio2LevelInt_Hi        *((volatile unsigned *)(GpioCtrlBase + 0x009C))
#define Gpio3IntStat            *((volatile unsigned *)(GpioCtrlBase + 0x00AC))
#define Gpio3InIntFallEdge      *((volatile unsigned *)(GpioCtrlBase + 0x00B0))
#define Gpio3InIntRiseEdge      *((volatile unsigned *)(GpioCtrlBase + 0x00B4))
#define Gpio3LevelInt_Lo        *((volatile unsigned *)(GpioCtrlBase + 0x00B8))
#define Gpio3LevelInt_Hi        *((volatile unsigned *)(GpioCtrlBase + 0x00BC))


/*****************************************************************************/
/* UART Control Register                                     */
/*****************************************************************************/
#define UartDr                  (UartCtrlBase)
#define UartDll                 (UartCtrlBase)
#define UartIer                 (UartCtrlBase + 0x0004)
#define UartDlh                 (UartCtrlBase + 0x0004)
#define UartIir                 (UartCtrlBase + 0x0008)
#define UartFcr                 (UartCtrlBase + 0x0008)
#define UartLcr                 (UartCtrlBase + 0x000c)
#define UartMcr                 (UartCtrlBase + 0x0010)
#define UartLsr                 (UartCtrlBase + 0x0014)
#define UartMsr                 (UartCtrlBase + 0x0018)
#define UartScr                 (UartCtrlBase + 0x001c)
#define Uart2Dr                 (UartCtrlBase + 0x0020)
#define Uart2Dll                (UartCtrlBase + 0x0020)
#define Uart2Ier                (UartCtrlBase + 0x0024)
#define Uart2Dlh                (UartCtrlBase + 0x0024)
#define Uart2Iir                (UartCtrlBase + 0x0028)
#define Uart2Fcr                (UartCtrlBase + 0x0028)
#define Uart2Lcr                (UartCtrlBase + 0x002c)
#define Uart2Lsr                (UartCtrlBase + 0x0034)
#define Uart3Dr                 (UartCtrlBase + 0x0040)
#define Uart3Dll                (UartCtrlBase + 0x0040)
#define Uart3Ier                (UartCtrlBase + 0x0044)
#define Uart3Dlh                (UartCtrlBase + 0x0044)
#define Uart3Iir                (UartCtrlBase + 0x0048)
#define Uart3Fcr                (UartCtrlBase + 0x0048)
#define Uart3Lcr                (UartCtrlBase + 0x004c)
#define Uart3Lsr                (UartCtrlBase + 0x0054)

/*****************************************************************************/
/* I2C Control Register                                      */
/*****************************************************************************/
#define I2cCtrl                 *((volatile unsigned *)(I2cCtrlBase))
#define I2cData                 *((volatile unsigned *)(I2cCtrlBase + 0x0004))
#define I2cMISC                 *((volatile unsigned *)(I2cCtrlBase + 0x0008))
#define I2cManu                 *((volatile unsigned *)(I2cCtrlBase + 0x000c))
#define I2cData2                *((volatile unsigned *)(I2cCtrlBase + 0x0010))

/*****************************************************************************/
/* IIS Control Register                                  */
/*****************************************************************************/
#define IisCtrl                 *((volatile unsigned *)(IisCtrlBase))
#define IisMode                 *((volatile unsigned *)(IisCtrlBase + 0x0004))
#define IisAudFormat            *((volatile unsigned *)(IisCtrlBase + 0x0008))
#define IisAdvance              *((volatile unsigned *)(IisCtrlBase + 0x000C))
#define IisCtrlAlt              *((volatile unsigned *)(IisCtrlBase + 0x0050))
#define IisModeAlt              *((volatile unsigned *)(IisCtrlBase + 0x0054))
#define IisAudFormatAlt         *((volatile unsigned *)(IisCtrlBase + 0x0058))
#define IisAdvanceAlt           *((volatile unsigned *)(IisCtrlBase + 0x005C))

/* yc: 2006.07.21 : S */
#define IisTxData               *((volatile unsigned *)(IisCtrlBase + 0x0010))
#define IisTx2Data               *((volatile unsigned *)(IisCtrlBase + 0x0014))
#define IisTx3Data               *((volatile unsigned *)(IisCtrlBase + 0x0018))
#define IisTx4Data              *((volatile unsigned *)(IisCtrlBase + 0x001C))
#define IisTx5Data              *((volatile unsigned *)(IisCtrlBase + 0x0060))
#define IisRxData               *((volatile unsigned *)(IisCtrlBase + 0x0020))
#define IisRx2Data               *((volatile unsigned *)(IisCtrlBase + 0x0024))
#define IisRx3Data               *((volatile unsigned *)(IisCtrlBase + 0x0028))
#define IisRx4Data              *((volatile unsigned *)(IisCtrlBase + 0x002C))
#define IisRx5Data              *((volatile unsigned *)(IisCtrlBase + 0x0070))
/* yc: 2006.07.21 : E */

/*****************************************************************************/
/* USB Control Register                                      */
/*****************************************************************************/
#define USBPHYRST               *((volatile unsigned *)(USBPHYRSTBase))
#define USBConf                 *((volatile unsigned *)(USBCommonCtrlBase))
#define USBMode                 *((volatile unsigned *)(USBCommonCtrlBase + 0x0004))
#define USBIntEn                *((volatile unsigned *)(USBCommonCtrlBase + 0x0008))
#define USBIntS                 *((volatile unsigned *)(USBCommonCtrlBase + 0x000c))
#define EPCMD0                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0040))
#define EPCMD1                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0044))
#define EPCMD2                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0048))
#define EPCMD3                  *((volatile unsigned *)(USBCommonCtrlBase + 0x004C))
#define EPCMD4                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0050))
#define EPCMD5                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0054))
#define EPCMD6                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0058))
#define EPCMD7                  *((volatile unsigned *)(USBCommonCtrlBase + 0x005C))
#define EPCMD8                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0060))
#define EPCMD9                  *((volatile unsigned *)(USBCommonCtrlBase + 0x0064))
#define EPCMD10                 *((volatile unsigned *)(USBCommonCtrlBase + 0x0068))
#define EPCMD11                 *((volatile unsigned *)(USBCommonCtrlBase + 0x006C))
#define EPCMD12                 *((volatile unsigned *)(USBCommonCtrlBase + 0x0070))
#define EPCMD13                 *((volatile unsigned *)(USBCommonCtrlBase + 0x0074))
#define EPCMD14                 *((volatile unsigned *)(USBCommonCtrlBase + 0x0078))
#define EPCMD15                 *((volatile unsigned *)(USBCommonCtrlBase + 0x007C))

#define OTGC                    *((volatile unsigned *)(USBOTGCtrlBase))
#define OTGSTS                  *((volatile unsigned *)(USBOTGCtrlBase + 0x0010))
#define OTGSTSC                 *((volatile unsigned *)(USBOTGCtrlBase + 0x0014))
#define OTGSTSFALL              *((volatile unsigned *)(USBOTGCtrlBase + 0x0018))
#define OTGSTSRISE              *((volatile unsigned *)(USBOTGCtrlBase + 0x001C))
#define OTGTC                   *((volatile unsigned *)(USBOTGCtrlBase + 0x0020))
#define OTGT                    *((volatile unsigned *)(USBOTGCtrlBase + 0x0024))

#define PORTSC                  *((volatile unsigned *)(USBHostCtrlBase))
#define PORTSTSC                *((volatile unsigned *)(USBHostCtrlBase + 0x0004))
#define HOSTEVENTSRC            *((volatile unsigned *)(USBHostCtrlBase + 0x0008))
#define HOSTINTEN               *((volatile unsigned *)(USBHostCtrlBase + 0x000C))
#define HCFRMIDX                *((volatile unsigned *)(USBHostCtrlBase + 0x0010))
#define HCFRMINIT               *((volatile unsigned *)(USBHostCtrlBase + 0x0014))
#define HCCTRL                  *((volatile unsigned *)(USBHostCtrlBase + 0x0018))
#define HCSTLINK                *((volatile unsigned *)(USBHostCtrlBase + 0x001C))

#define DEVC                    *((volatile unsigned *)(USBDeviceCtrlBase))
#define DEVS                    *((volatile unsigned *)(USBDeviceCtrlBase + 0x0004))
#define FADDR                   *((volatile unsigned *)(USBDeviceCtrlBase + 0x0008))
#define TSTAMP                  *((volatile unsigned *)(USBDeviceCtrlBase + 0x000C))

#define DMAC1                    *((volatile unsigned *)(USBDMACtrlBase))
#define DMAS1                    *((volatile unsigned *)(USBDMACtrlBase + 0x0004))
#define DMATCI1                  *((volatile unsigned *)(USBDMACtrlBase + 0x0008))
#define DMATC1                   *((volatile unsigned *)(USBDMACtrlBase + 0x000C))
#define DMAC2                    *((volatile unsigned *)(USBDMACtrlBase + 0x0020))
#define DMAS2                    *((volatile unsigned *)(USBDMACtrlBase + 0x0024))
#define DMATCI2                  *((volatile unsigned *)(USBDMACtrlBase + 0x0028))
#define DMATC2                   *((volatile unsigned *)(USBDMACtrlBase + 0x002C))

#define HCEPCTRL1_0             *((volatile unsigned *)(RAMATHOSTBase))
#define HCEPCTRL2_0             *((volatile unsigned *)(RAMATHOSTBase + 0x0004))
#define HCEPCTRL1_1             *((volatile unsigned *)(RAMATHOSTBase + 0x0008))
#define HCEPCTRL2_1             *((volatile unsigned *)(RAMATHOSTBase + 0x000C))
#define HCEPCTRL1_2             *((volatile unsigned *)(RAMATHOSTBase + 0x0010))
#define HCEPCTRL2_2             *((volatile unsigned *)(RAMATHOSTBase + 0x0014))
#define HCEPCTRL1_3             *((volatile unsigned *)(RAMATHOSTBase + 0x0018))
#define HCEPCTRL2_3             *((volatile unsigned *)(RAMATHOSTBase + 0x001C))
#define HCEPCTRL1_4             *((volatile unsigned *)(RAMATHOSTBase + 0x0020))
#define HCEPCTRL2_4             *((volatile unsigned *)(RAMATHOSTBase + 0x0024))
#define HCEPCTRL1_5             *((volatile unsigned *)(RAMATHOSTBase + 0x0028))
#define HCEPCTRL2_5             *((volatile unsigned *)(RAMATHOSTBase + 0x002C))
#define HCEPCTRL1_6             *((volatile unsigned *)(RAMATHOSTBase + 0x0030))
#define HCEPCTRL2_6             *((volatile unsigned *)(RAMATHOSTBase + 0x0034))
#define HCEPCTRL1_7             *((volatile unsigned *)(RAMATHOSTBase + 0x0038))
#define HCEPCTRL2_7             *((volatile unsigned *)(RAMATHOSTBase + 0x003C))

#define EPCTRL0                  *((volatile unsigned *)(RAMATHOSTBase))
#define EPCTRL1                  *((volatile unsigned *)(RAMATHOSTBase + 0x0004))
#define EPCTRL2                  *((volatile unsigned *)(RAMATHOSTBase + 0x0008))
#define EPCTRL3                  *((volatile unsigned *)(RAMATHOSTBase + 0x000C))
#define EPCTRL4                  *((volatile unsigned *)(RAMATHOSTBase + 0x0010))
#define EPCTRL5                  *((volatile unsigned *)(RAMATHOSTBase + 0x0014))
#define EPCTRL6                  *((volatile unsigned *)(RAMATHOSTBase + 0x0018))
#define EPCTRL7                  *((volatile unsigned *)(RAMATHOSTBase + 0x001C))
#define EPCTRL8                  *((volatile unsigned *)(RAMATHOSTBase + 0x0020))
#define EPCTRL9                  *((volatile unsigned *)(RAMATHOSTBase + 0x0024))
#define EPCTRL10                 *((volatile unsigned *)(RAMATHOSTBase + 0x0028))
#define EPCTRL11                 *((volatile unsigned *)(RAMATHOSTBase + 0x002C))
#define EPCTRL12                 *((volatile unsigned *)(RAMATHOSTBase + 0x0030))
#define EPCTRL13                 *((volatile unsigned *)(RAMATHOSTBase + 0x0034))
#define EPCTRL14                 *((volatile unsigned *)(RAMATHOSTBase + 0x0038))
#define EPCTRL15                 *((volatile unsigned *)(RAMATHOSTBase + 0x003C))




#define EPCONF0                 *((volatile unsigned *)(RAMATHOSTBase + 0x0040))
#define EPCONF1                 *((volatile unsigned *)(RAMATHOSTBase + 0x0044))
#define EPCONF2                 *((volatile unsigned *)(RAMATHOSTBase + 0x0048))
#define EPCONF3                 *((volatile unsigned *)(RAMATHOSTBase + 0x004C))
#define EPCONF4                 *((volatile unsigned *)(RAMATHOSTBase + 0x0050))
#define EPCONF5                 *((volatile unsigned *)(RAMATHOSTBase + 0x0054))
#define EPCONF6                 *((volatile unsigned *)(RAMATHOSTBase + 0x0058))
#define EPCONF7                 *((volatile unsigned *)(RAMATHOSTBase + 0x005C))
#define EPCONF8                 *((volatile unsigned *)(RAMATHOSTBase + 0x0060))
#define EPCONF9                 *((volatile unsigned *)(RAMATHOSTBase + 0x0064))
#define EPCONF10                *((volatile unsigned *)(RAMATHOSTBase + 0x0068))
#define EPCONF11                *((volatile unsigned *)(RAMATHOSTBase + 0x006C))
#define EPCONF12                *((volatile unsigned *)(RAMATHOSTBase + 0x0070))
#define EPCONF13                *((volatile unsigned *)(RAMATHOSTBase + 0x0074))
#define EPCONF14                *((volatile unsigned *)(RAMATHOSTBase + 0x0078))
#define EPCONF15                *((volatile unsigned *)(RAMATHOSTBase + 0x007C))

#define EPCOUNT0                *((volatile unsigned *)(RAMATHOSTBase + 0x0080))
#define EPCOUNT1                *((volatile unsigned *)(RAMATHOSTBase + 0x0084))
#define EPCOUNT2                *((volatile unsigned *)(RAMATHOSTBase + 0x0088))
#define EPCOUNT3                *((volatile unsigned *)(RAMATHOSTBase + 0x008C))
#define EPCOUNT4                *((volatile unsigned *)(RAMATHOSTBase + 0x0090))
#define EPCOUNT5                *((volatile unsigned *)(RAMATHOSTBase + 0x0094))
#define EPCOUNT6                *((volatile unsigned *)(RAMATHOSTBase + 0x0098))
#define EPCOUNT7                *((volatile unsigned *)(RAMATHOSTBase + 0x009C))
#define EPCOUNT8                *((volatile unsigned *)(RAMATHOSTBase + 0x00A0))
#define EPCOUNT9                *((volatile unsigned *)(RAMATHOSTBase + 0x00A4))
#define EPCOUNT10               *((volatile unsigned *)(RAMATHOSTBase + 0x00A8))
#define EPCOUNT11               *((volatile unsigned *)(RAMATHOSTBase + 0x00AC))
#define EPCOUNT12               *((volatile unsigned *)(RAMATHOSTBase + 0x00B0))
#define EPCOUNT13               *((volatile unsigned *)(RAMATHOSTBase + 0x00B4))
#define EPCOUNT14               *((volatile unsigned *)(RAMATHOSTBase + 0x00B8))
#define EPCOUNT15               *((volatile unsigned *)(RAMATHOSTBase + 0x00BC))

#define HOST_DEVICE_SWITCH      *((volatile unsigned *)(SysCtrlBase + 0X002C))
// Host
#define HCCapbility             *((volatile unsigned *)(FD_USBHOSTBase))
#define HCSparams               *((volatile unsigned *)(FD_USBHOSTBase + 0x0004))
#define HCCparams               *((volatile unsigned *)(FD_USBHOSTBase + 0x0008))
#define HCUSBCMD                *((volatile unsigned *)(FD_USBHOSTBase + 0x0010))
#define HCUSBSTS                *((volatile unsigned *)(FD_USBHOSTBase + 0x0014))
#define HCUSBINTR               *((volatile unsigned *)(FD_USBHOSTBase + 0x0018))
#define HCFRindex               *((volatile unsigned *)(FD_USBHOSTBase + 0x001C))
#define HCPeriodicListBS        *((volatile unsigned *)(FD_USBHOSTBase + 0x0024))
#define HCAsyncListAddr         *((volatile unsigned *)(FD_USBHOSTBase + 0x0028))
#define HCPortSC                *((volatile unsigned *)(FD_USBHOSTBase + 0x0030))
#define HCMisc                  *((volatile unsigned *)(FD_USBHOSTBase + 0x0040))

// OTG
#define OTGCtlS                 *((volatile unsigned *)(FD_USBOTGBase))
#define OTGInterruptS           *((volatile unsigned *)(FD_USBOTGBase + 0x0004))
#define OTGInterruptEnable      *((volatile unsigned *)(FD_USBOTGBase + 0x0008))

// Global
#define GLOBALInterruptS        *((volatile unsigned *)(FD_USBGLOBALBase))
#define GLOBALInterruptMask     *((volatile unsigned *)(FD_USBGLOBALBase + 0x0004))

// Device
#define DeviceMainCtl           *((volatile unsigned *)(FD_USBDEVICEBase))
#define DeviceAddress           *((volatile unsigned *)(FD_USBDEVICEBase + 0x0004))
#define DeviceTest              *((volatile unsigned *)(FD_USBDEVICEBase + 0x0008))
#define DeviceSOF               *((volatile unsigned *)(FD_USBDEVICEBase + 0x000C))
#define DeviceSOFMaskTimer      *((volatile unsigned *)(FD_USBDEVICEBase + 0x0010))
#define DevicePHYTestMode       *((volatile unsigned *)(FD_USBDEVICEBase + 0x0014))
#define DeviceVendorIOCtl       *((volatile unsigned *)(FD_USBDEVICEBase + 0x0018))
#define DeviceCXConfigS         *((volatile unsigned *)(FD_USBDEVICEBase + 0x001C))
#define DeviceCXConfigFIFOEmpS  *((volatile unsigned *)(FD_USBDEVICEBase + 0x0020))
#define DeviceIdleCnt           *((volatile unsigned *)(FD_USBDEVICEBase + 0x0024))
#define DeviceMaskInterrupt     *((volatile unsigned *)(FD_USBDEVICEBase + 0x0030))
#define DeviceMaskInterruptG0   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0034))
#define DeviceMaskInterruptG1   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0038))
#define DeviceMaskInterruptG2   *((volatile unsigned *)(FD_USBDEVICEBase + 0x003C))
#define DeviceInterruptGP       *((volatile unsigned *)(FD_USBDEVICEBase + 0x0040))
#define DeviceInterruptSourceG0 *((volatile unsigned *)(FD_USBDEVICEBase + 0x0044))
#define DeviceInterruptSourceG1 *((volatile unsigned *)(FD_USBDEVICEBase + 0x0048))
#define DeviceInterruptSourceG2 *((volatile unsigned *)(FD_USBDEVICEBase + 0x004C))
#define DeviceRxZeroData        *((volatile unsigned *)(FD_USBDEVICEBase + 0x0050))
#define DeviceTxZeroData        *((volatile unsigned *)(FD_USBDEVICEBase + 0x0054))
#define DeviceIsoSeqErr         *((volatile unsigned *)(FD_USBDEVICEBase + 0x0058))
#define DeviceInEP1MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0060))
#define DeviceInEP2MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0064))
#define DeviceInEP3MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0068))
#define DeviceInEP4MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x006C))
#define DeviceInEP5MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0070))
#define DeviceInEP6MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0074))
#define DeviceInEP7MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0078))
#define DeviceInEP8MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x007C))
#define DeviceOutEP1MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0080))
#define DeviceOutEP2MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0084))
#define DeviceOutEP3MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0088))
#define DeviceOutEP4MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x008C))
#define DeviceOutEP5MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0090))
#define DeviceOutEP6MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0094))
#define DeviceOutEP7MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x0098))
#define DeviceOutEP8MaxPktSize   *((volatile unsigned *)(FD_USBDEVICEBase + 0x009C))
#define DeviceEP1to4Map          *((volatile unsigned *)(FD_USBDEVICEBase + 0x00A0))
#define DeviceEP5to8Map          *((volatile unsigned *)(FD_USBDEVICEBase + 0x00A4))
#define DeviceFIFOMap            *((volatile unsigned *)(FD_USBDEVICEBase + 0x00A8))
#define DeviceFIFOConfig         *((volatile unsigned *)(FD_USBDEVICEBase + 0x00AC))
#define DeviceFIFO0ByteCnt       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00B0))
#define DeviceFIFO1ByteCnt       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00B4))
#define DeviceFIFO2ByteCnt       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00B8))
#define DeviceFIFO3ByteCnt       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00BC))
#define DeviceDMATargetFIFONum   *((volatile unsigned *)(FD_USBDEVICEBase + 0x00C0))
#define DeviceDMACtlParam1       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00C8))
#define DeviceDMACtlParam2       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00CC))
#define DeviceDMACtlParam3       *((volatile unsigned *)(FD_USBDEVICEBase + 0x00D0))

// Host 2nd
#define HCCapbility_U2             *((volatile unsigned *)(FD_USBHOSTBase_2nd))
#define HCSparams_U2               *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0004))
#define HCCparams_U2               *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0008))
#define HCUSBCMD_U2                *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0010))
#define HCUSBSTS_U2                *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0014))
#define HCUSBINTR_U2               *((volatile unsigned *)(FD_USBHOSTBase_2nd +0x0018))
#define HCFRindex_U2               *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x001C))
#define HCPeriodicListBS_U2        *((volatile unsigned *)(FD_USBHOSTBase_2nd +0x0024))
#define HCAsyncListAddr_U2         *((volatile unsigned *)(FD_USBHOSTBase_2nd +0x0028))
#define HCPortSC_U2                *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0030))
#define HCMisc_U2                  *((volatile unsigned *)(FD_USBHOSTBase_2nd + 0x0040))

// OTG 2nd
#define OTGCtlS_U2                 *((volatile unsigned *)(FD_USBOTGBase_2nd))
#define OTGInterruptS_U2           *((volatile unsigned *)(FD_USBOTGBase_2nd + 0x0004))
#define OTGInterruptEnable_U2      *((volatile unsigned *)(FD_USBOTGBase_2nd + 0x0008))

// Global 2nd
#define GLOBALInterruptS_U2        *((volatile unsigned *)(FD_USBGLOBALBase_2nd))
#define GLOBALInterruptMask_U2     *((volatile unsigned *)(FD_USBGLOBALBase_2nd+ 0x0004))

// Device 2nd
#define DeviceMainCtl_U2           *((volatile unsigned *)(FD_USBDEVICEBase_2nd))
#define DeviceAddress_U2           *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0004))
#define DeviceTest_U2              *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0008))
#define DeviceSOF_U2               *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x000C))
#define DeviceSOFMaskTimer_U2      *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0010))
#define DevicePHYTestMode_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0014))
#define DeviceVendorIOCtl_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0018))
#define DeviceCXConfigS_U2         *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x001C))
#define DeviceCXConfigFIFOEmpS_U2  *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0020))
#define DeviceIdleCnt_U2           *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0024))
#define DeviceMaskInterrupt_U2     *((volatile unsigned *)(FD_USBDEVICEBase_2nd +0x0030))
#define DeviceMaskInterruptG0_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0034))
#define DeviceMaskInterruptG1_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0038))
#define DeviceMaskInterruptG2_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x003C))
#define DeviceInterruptGP_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0040))
#define DeviceInterruptSourceG0_U2 *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0044))
#define DeviceInterruptSourceG1_U2 *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0048))
#define DeviceInterruptSourceG2_U2 *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x004C))
#define DeviceRxZeroData_U2        *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0050))
#define DeviceTxZeroData_U2        *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0054))
#define DeviceIsoSeqErr_U2         *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0058))
#define DeviceInEP1MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0060))
#define DeviceInEP2MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0064))
#define DeviceInEP3MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0068))
#define DeviceInEP4MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x006C))
#define DeviceInEP5MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0070))
#define DeviceInEP6MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0074))
#define DeviceInEP7MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0078))
#define DeviceInEP8MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x007C))
#define DeviceOutEP1MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0080))
#define DeviceOutEP2MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0084))
#define DeviceOutEP3MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0088))
#define DeviceOutEP4MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x008C))
#define DeviceOutEP5MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0090))
#define DeviceOutEP6MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0094))
#define DeviceOutEP7MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x0098))
#define DeviceOutEP8MaxPktSize_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x009C))
#define DeviceEP1to4Map_U2          *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00A0))
#define DeviceEP5to8Map_U2          *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00A4))
#define DeviceFIFOMap_U2            *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00A8))
#define DeviceFIFOConfig_U2         *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00AC))
#define DeviceFIFO0ByteCnt_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00B0))
#define DeviceFIFO1ByteCnt_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00B4))
#define DeviceFIFO2ByteCnt_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00B8))
#define DeviceFIFO3ByteCnt_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00BC))
#define DeviceDMATargetFIFONum_U2   *((volatile unsigned *)(FD_USBDEVICEBase_2nd +0x00C0))
#define DeviceDMACtlParam1_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00C8))
#define DeviceDMACtlParam2_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00CC))
#define DeviceDMACtlParam3_U2       *((volatile unsigned *)(FD_USBDEVICEBase_2nd + 0x00D0))

#define UsbIntEna               *((volatile unsigned *)(UsbCtrlBase))
#define UsbIntStat              *((volatile unsigned *)(UsbCtrlBase + 0x0004))
#define UsbDevAddr              *((volatile unsigned *)(UsbCtrlBase + 0x0008))
#define UsbCtrl                 *((volatile unsigned *)(UsbCtrlBase + 0x000c))
#define UsbEp0CtrlStat          *((volatile unsigned *)(UsbCtrlBase + 0x0010))
#define UsbEp0RxData            *((volatile unsigned *)(UsbCtrlBase + 0x0014))
#define UsbEp0TxData            *((volatile unsigned *)(UsbCtrlBase + 0x0018))
#define UsbEp0DataLen           *((volatile unsigned *)(UsbCtrlBase + 0x001c))
#define UsbEp1CtrlStat          *((volatile unsigned *)(UsbCtrlBase + 0x0020))
#define UsbEp1TxData            *((volatile unsigned *)(UsbCtrlBase + 0x0024))
#define UsbEp1DataLen           *((volatile unsigned *)(UsbCtrlBase + 0x0028))
#define UsbEp2CtrlStat          *((volatile unsigned *)(UsbCtrlBase + 0x002c))
#define UsbEp2RxData            *((volatile unsigned *)(UsbCtrlBase + 0x0030))
#define UsbEp2DataLen           *((volatile unsigned *)(UsbCtrlBase + 0x0034))
#define UsbEp2TotalDataLen      *((volatile unsigned *)(UsbCtrlBase + 0x0038))
#define UsbEp3CtrlStat          *((volatile unsigned *)(UsbCtrlBase + 0x003c))
#define UsbEp3TxData            *((volatile unsigned *)(UsbCtrlBase + 0x0040))
#define UsbEp3DataLen           *((volatile unsigned *)(UsbCtrlBase + 0x0044))
#define UsbEp4CtrlStat          *((volatile unsigned *)(UsbCtrlBase + 0x0048))
#define UsbEp4TxData            *((volatile unsigned *)(UsbCtrlBase + 0x004c))
#define UsbEp4DataLen           *((volatile unsigned *)(UsbCtrlBase + 0x0050))
#define UsbEp4TotalDataLen      *((volatile unsigned *)(UsbCtrlBase + 0x0054))
#define UsbSofFramNum           *((volatile unsigned *)(UsbCtrlBase + 0x0058))
#define UsbTestMode             *((volatile unsigned *)(UsbCtrlBase + 0x0078))
#define UsbParm                 *((volatile unsigned *)(UsbCtrlBase + 0x007c))

/*****************************************************************************/
/* SecurityDisk Card Control Register                                */
/*****************************************************************************/
#define SdcCmd                  *((volatile unsigned *)(SdcCtrlBase))
#define SdcParm                 *((volatile unsigned *)(SdcCtrlBase + 0x0004))
#define SdcRsp1                 *((volatile unsigned *)(SdcCtrlBase + 0x0008))
#define SdcRsp2                 *((volatile unsigned *)(SdcCtrlBase + 0x000c))
#define SdcRsp3                 *((volatile unsigned *)(SdcCtrlBase + 0x0010))
#define SdcRsp4                 *((volatile unsigned *)(SdcCtrlBase + 0x0014))
#define SdcRsp5                 *((volatile unsigned *)(SdcCtrlBase + 0x0018))
#define SdcData                 *((volatile unsigned *)(SdcCtrlBase + 0x001c))
#define SdcDmaTrig              *((volatile unsigned *)(SdcCtrlBase + 0x0020))
#define SdcCfg                  *((volatile unsigned *)(SdcCtrlBase + 0x0024))
#define SdcCtrl0                *((volatile unsigned *)(SdcCtrlBase + 0x0028))
#define SdcCtrl1                *((volatile unsigned *)(SdcCtrlBase + 0x002c))
#define SdcStat                 *((volatile unsigned *)(SdcCtrlBase + 0x0030))
#define SdcIntMask              *((volatile unsigned *)(SdcCtrlBase + 0x0034))
#define SdcIntStat              *((volatile unsigned *)(SdcCtrlBase + 0x0038))
#define SdcMulBlkNum            *((volatile unsigned *)(SdcCtrlBase + 0x003c))


#define Sdc_2_Cmd               *((volatile unsigned *)(Sdc_2_CtrlBase))
#define Sdc_2_Parm              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0004))
#define Sdc_2_Rsp1              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0008))
#define Sdc_2_Rsp2              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x000c))
#define Sdc_2_Rsp3              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0010))
#define Sdc_2_Rsp4              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0014))
#define Sdc_2_Rsp5              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0018))
#define Sdc_2_Data              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x001c))
#define Sdc_2_DmaTrig           *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0020))
#define Sdc_2_Cfg               *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0024))
#define Sdc_2_Ctrl0             *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0028))
#define Sdc_2_Ctrl1             *((volatile unsigned *)(Sdc_2_CtrlBase + 0x002c))
#define Sdc_2_Stat              *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0030))
#define Sdc_2_IntMask           *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0034))
#define Sdc_2_IntStat           *((volatile unsigned *)(Sdc_2_CtrlBase + 0x0038))
#define Sdc_2_MulBlkNum         *((volatile unsigned *)(Sdc_2_CtrlBase + 0x003c))

/*****************************************************************************/
/* SmartMedia Card Control Register                          */
/*****************************************************************************/
#define SmcCfg                  *((volatile unsigned *)(SmcCtrlBase))
#define SmcTimeCtrl             *((volatile unsigned *)(SmcCtrlBase + 0x0004))
#define SmcCtrl                 *((volatile unsigned *)(SmcCtrlBase + 0x0008))
#define SmcCmd                  *((volatile unsigned *)(SmcCtrlBase + 0x000c))
#define SmcAddr                 *((volatile unsigned *)(SmcCtrlBase + 0x0010))
#define SmcStat                 *((volatile unsigned *)(SmcCtrlBase + 0x0014))
#define SmcData                 *((volatile unsigned *)(SmcCtrlBase + 0x0018))
#define SmcIntMask              *((volatile unsigned *)(SmcCtrlBase + 0x001c))
#define SmcIntStat              *((volatile unsigned *)(SmcCtrlBase + 0x0020))
/*  REG_SM_REDUNDANT_AREA1 (SMC redundant area 1st ~ 4th  word)*/
#define SmcRedun0               *((volatile unsigned *)(SmcCtrlBase + 0x0024))
#define SmcRedun1               *((volatile unsigned *)(SmcCtrlBase + 0x0028))
#define SmcRedun2               *((volatile unsigned *)(SmcCtrlBase + 0x002c))
#define SmcRedun3               *((volatile unsigned *)(SmcCtrlBase + 0x0030))

#define SmcEcc1                 *((volatile unsigned *)(SmcCtrlBase + 0x0034))
#define SmcEcc2                 *((volatile unsigned *)(SmcCtrlBase + 0x0038))
/*  REG_SM_REDUNDANT_AREA2 (SMC redundant area 5th ~ 16th  word)*/
#define SmcRedun4               *((volatile unsigned *)(SmcCtrlBase + 0x003C))
#define SmcRedun5               *((volatile unsigned *)(SmcCtrlBase + 0x0040))
#define SmcRedun6               *((volatile unsigned *)(SmcCtrlBase + 0x0044))
#define SmcRedun7               *((volatile unsigned *)(SmcCtrlBase + 0x0048))
#define SmcRedun8               *((volatile unsigned *)(SmcCtrlBase + 0x004C))
#define SmcRedun9               *((volatile unsigned *)(SmcCtrlBase + 0x0050))
#define SmcRedun10              *((volatile unsigned *)(SmcCtrlBase + 0x0054))
#define SmcRedun11              *((volatile unsigned *)(SmcCtrlBase + 0x0058))
#define SmcRedun12              *((volatile unsigned *)(SmcCtrlBase + 0x005C))
#define SmcRedun13              *((volatile unsigned *)(SmcCtrlBase + 0x0060))
#define SmcRedun14              *((volatile unsigned *)(SmcCtrlBase + 0x0064))
#define SmcRedun15              *((volatile unsigned *)(SmcCtrlBase + 0x0068))

#define SmcEcc3                 *((volatile unsigned *)(SmcCtrlBase + 0x006C))
#define SmcEcc4                 *((volatile unsigned *)(SmcCtrlBase + 0x0070))
#define SmcEcc5                 *((volatile unsigned *)(SmcCtrlBase + 0x0074))
#define SmcEcc6                 *((volatile unsigned *)(SmcCtrlBase + 0x0078))
#define SmcEcc7                 *((volatile unsigned *)(SmcCtrlBase + 0x007C))
#define SmcEcc8                 *((volatile unsigned *)(SmcCtrlBase + 0x0080))
#define SmcEcc9                 *((volatile unsigned *)(SmcCtrlBase + 0x0084))
#define SmcEcc10                *((volatile unsigned *)(SmcCtrlBase + 0x0088))
#define SmcEcc11                *((volatile unsigned *)(SmcCtrlBase + 0x008C))
#define SmcEcc12                *((volatile unsigned *)(SmcCtrlBase + 0x0090))
#define SmcEcc13                *((volatile unsigned *)(SmcCtrlBase + 0x0094))
#define SmcEcc14                *((volatile unsigned *)(SmcCtrlBase + 0x0098))
#define SmcEcc15                *((volatile unsigned *)(SmcCtrlBase + 0x009C))
#define SmcEcc16                *((volatile unsigned *)(SmcCtrlBase + 0x00A0))
/*  REG_SM_REDUNDANT_AREA2 (SMC redundant area 17th ~ 32th  word) */
#define SmcRedun16              *((volatile unsigned *)(SmcCtrlBase + 0x00A4))
#define SmcRedun17              *((volatile unsigned *)(SmcCtrlBase + 0x00A8))
#define SmcRedun18              *((volatile unsigned *)(SmcCtrlBase + 0x00AC))
#define SmcRedun19              *((volatile unsigned *)(SmcCtrlBase + 0x00B0))
#define SmcRedun20              *((volatile unsigned *)(SmcCtrlBase + 0x00B4))
#define SmcRedun21              *((volatile unsigned *)(SmcCtrlBase + 0x00B8))
#define SmcRedun22              *((volatile unsigned *)(SmcCtrlBase + 0x00BC))
#define SmcRedun23              *((volatile unsigned *)(SmcCtrlBase + 0x00C0))
#define SmcRedun24              *((volatile unsigned *)(SmcCtrlBase + 0x00C4))
#define SmcRedun25              *((volatile unsigned *)(SmcCtrlBase + 0x00C8))
#define SmcRedun26              *((volatile unsigned *)(SmcCtrlBase + 0x00CC))
#define SmcRedun27              *((volatile unsigned *)(SmcCtrlBase + 0x00D0))
#define SmcRedun28              *((volatile unsigned *)(SmcCtrlBase + 0x00D4))
#define SmcRedun29              *((volatile unsigned *)(SmcCtrlBase + 0x00D8))
#define SmcRedun30              *((volatile unsigned *)(SmcCtrlBase + 0x00DC))
#define SmcRedun31              *((volatile unsigned *)(SmcCtrlBase + 0x00E0))

/*RS Function */
/* Error position 1st ~ 8th 0x00E4 ~ 0x0120  */
#define SmcCheErrPos1st         *((volatile unsigned *)(SmcCtrlBase + 0x00E4))

/* Correct symbol 1st ~ 8th 0x0124 ~ 0x0160  */
#define SmcCorrtSym1st          *((volatile unsigned *)(SmcCtrlBase + 0x0124))

#define SmcChkErrNum            *((volatile unsigned *)(SmcCtrlBase + 0x0164))
#define SmcChkErrAddr           *((volatile unsigned *)(SmcCtrlBase + 0x0168))

/* Error position 9th ~ 16th 0x016C ~ 0x01A8  */
#define SmcCheErrPos9th         *((volatile unsigned *)(SmcCtrlBase + 0x016C))

/* Correct symbol 1st ~ 8th 0x01AC ~ 0x01E8  */
#define SmcCorrtSym9th          *((volatile unsigned *)(SmcCtrlBase + 0x01AC))

/*  REG_SM_REDUNDANT_AREA3 (SMC redundant area 33th ~109th word)0x01EC ~ 0x031C */
#define SmcRedun32              *((volatile unsigned *)(SmcCtrlBase + 0x01EC))
#define SmcRedun33              *((volatile unsigned *)(SmcCtrlBase + 0x01F0))
#define SmcRedun34              *((volatile unsigned *)(SmcCtrlBase + 0x01F4))
#define SmcRedun35              *((volatile unsigned *)(SmcCtrlBase + 0x01F8))
#define SmcRedun36              *((volatile unsigned *)(SmcCtrlBase + 0x01FC))
#define SmcRedun37              *((volatile unsigned *)(SmcCtrlBase + 0x0200))

/*****************************************************************************/
/* Timer Control Register                                */
/*****************************************************************************/
#define Timer0Count             *((volatile unsigned *)(TimerCtrlBase))
#define Timer0Ctrl              *((volatile unsigned *)(TimerCtrlBase + 0x0004))
#define Timer0IntEna            *((volatile unsigned *)(TimerCtrlBase + 0x0008))
#define Timer0IntStat           *((volatile unsigned *)(TimerCtrlBase + 0x000c))
#define Timer1Count             *((volatile unsigned *)(TimerCtrlBase + 0x0010))
#define Timer1Ctrl              *((volatile unsigned *)(TimerCtrlBase + 0x0014))
#define Timer1IntEna            *((volatile unsigned *)(TimerCtrlBase + 0x0018))
#define Timer1IntStat           *((volatile unsigned *)(TimerCtrlBase + 0x001c))
#define Timer2Count             *((volatile unsigned *)(TimerCtrlBase + 0x0020))
#define Timer2Ctrl              *((volatile unsigned *)(TimerCtrlBase + 0x0024))
#define Timer2IntEna            *((volatile unsigned *)(TimerCtrlBase + 0x0028))
#define Timer2IntStat           *((volatile unsigned *)(TimerCtrlBase + 0x002c))
#define Timer3Count             *((volatile unsigned *)(TimerCtrlBase + 0x0030))
#define Timer3Ctrl              *((volatile unsigned *)(TimerCtrlBase + 0x0034))
#define Timer3IntEna            *((volatile unsigned *)(TimerCtrlBase + 0x0038))
#define Timer3IntStat           *((volatile unsigned *)(TimerCtrlBase + 0x003c))
#define Timer4Count             *((volatile unsigned *)(TimerCtrlBase + 0x0040))
#define Timer4Ctrl              *((volatile unsigned *)(TimerCtrlBase + 0x0044))
#define Timer4IntEna            *((volatile unsigned *)(TimerCtrlBase + 0x0048))
#define Timer4IntStat           *((volatile unsigned *)(TimerCtrlBase + 0x004c))
#define Timer0123IntStat        *((volatile unsigned *)(TimerCtrlBase + 0x0050))

#define PWM5_CVREG             *((volatile unsigned *)(TimerCtrlBase + 0x00B0))
#define PWM5_CTLREG            *((volatile unsigned *)(TimerCtrlBase + 0x00B4))
#define PWM5_PAUSE_INTEN       *((volatile unsigned *)(TimerCtrlBase + 0x00B8))


/*****************************************************************************/
/* RTC Control Register                                  */
/*****************************************************************************/
#define RtcBase                 *((volatile unsigned *)(RtcCtrlBase))
#define RtcCount                *((volatile unsigned *)(RtcCtrlBase + 0x0004))
#define RtcCountCtrl            *((volatile unsigned *)(RtcCtrlBase + 0x0008))
#define RtcIntCtrl              *((volatile unsigned *)(RtcCtrlBase + 0x000c))
#define RtcIntStat              *((volatile unsigned *)(RtcCtrlBase + 0x0010))
#define RtcTmpCnt               *((volatile unsigned *)(RtcCtrlBase + 0x0014))

/*****************************************************************************/
/* ADC Control Register                                  */
/*****************************************************************************/
#define AdcCtrlReg              *((volatile unsigned *)(AdcCtrlBase))
#define DacTxCtrl               *((volatile unsigned *)(AdcCtrlBase + 0x0004))
#define AdcRecData_G1G2         *((volatile unsigned *)(AdcCtrlBase + 0x0008))
#define AdcRecData_G3G4         *((volatile unsigned *)(AdcCtrlBase + 0x000C))
#define AdcRecConvRate          *((volatile unsigned *)(AdcCtrlBase + 0x0028))
#define AdcRXRecChMap_G0        *((volatile unsigned *)(AdcCtrlBase + 0x002C))
#define AdcRXRecChMap_G1        *((volatile unsigned *)(AdcCtrlBase + 0x0030))
#define AdcRXRecChMap_G2        *((volatile unsigned *)(AdcCtrlBase + 0x0034))
#define AdcRXRecChMap_G3        *((volatile unsigned *)(AdcCtrlBase + 0x0038))
#define AdcRecData_G0           *((volatile unsigned *)(AdcCtrlBase + 0x003C))
#define AdcRecData_G1           *((volatile unsigned *)(AdcCtrlBase + 0x0040))
#define AdcRecData_G2           *((volatile unsigned *)(AdcCtrlBase + 0x0044))
#define AdcRecData_G3           *((volatile unsigned *)(AdcCtrlBase + 0x0048))
#define ADCRX_AVG_FIFO_INT      *((volatile unsigned *)(AdcCtrlBase + 0x004C))
#define ADCRX_AVG_VOLUMN        *((volatile unsigned *)(AdcCtrlBase + 0x0050))
#define ADCRX_TOUCH_XY          *((volatile unsigned *)(AdcCtrlBase + 0x0060))
#define ADCRX_TOUCH_Z           *((volatile unsigned *)(AdcCtrlBase + 0x0064))

/*****************************************************************************/
/* CF Host Control Register                                  */
/*****************************************************************************/
#define CfIntEn                 *((volatile unsigned *)(IdeCtrlBase))            /* CF Interrupt Enable Register */
#define CfIntStatus             *((volatile unsigned *)(IdeCtrlBase + 0x0004))   /* CF Interrupt Status Register */
#define CfSelAccmode            *((volatile unsigned *)(IdeCtrlBase + 0x0008))   /* CF Select Access Mode Register */
#define CfWaitTime              *((volatile unsigned *)(IdeCtrlBase + 0x000c))   /* CF Card WAIT# Signal Time Out Register */
#define CfHostCtrlReg           *((volatile unsigned *)(IdeCtrlBase + 0x0010))   /* CF Host Control Register */
#define CfSectorSize            *((volatile unsigned *)(IdeCtrlBase + 0x0014))   /* CF Sector Size Register */
#define CfAddr                  *((volatile unsigned *)(IdeCtrlBase + 0x0018))   /* CF Address Register */
#define CfReserved              *((volatile unsigned *)(IdeCtrlBase + 0x001c))
#define CfSectorCount           *((volatile unsigned *)(IdeCtrlBase + 0x0020))   /* CF Sector Count Register */
#define CfAttmodeReadTime       *((volatile unsigned *)(IdeCtrlBase + 0x0024))   /* CF Attribute Mode Read Timing Control Register */
#define CfCommodeReadTime       *((volatile unsigned *)(IdeCtrlBase + 0x0028))   /* CF Common Memory Mode Read Timing Control Register */
#define CfIdemodeReadTime       *((volatile unsigned *)(IdeCtrlBase + 0x002c))   /* CF True-IDE Mode Read Timing Control Register */
#define CfErrorData             *((volatile unsigned *)(IdeCtrlBase + 0x0030))   /* CF Error Data Register */
#define CfStatusChkloopcnt      *((volatile unsigned *)(IdeCtrlBase + 0x0034))   /* CF Status Check Loop Count Register */
#define CfCardPinlevel          *((volatile unsigned *)(IdeCtrlBase + 0x0038))   /* CF Card Pin Level Register */
#define CfCardTrigen            *((volatile unsigned *)(IdeCtrlBase + 0x003c))   /* CF Card Trigger Enable Register */
#define CfCardReset             *((volatile unsigned *)(IdeCtrlBase + 0x0040))   /* CF Card Reset Register */
#define CfDataport              *((volatile unsigned *)(IdeCtrlBase + 0x0044))   /* CF Data Port Register */
#define CfHostrdy               *((volatile unsigned *)(IdeCtrlBase + 0x0048))   /* CF Host Ready Register */
#define CfCardResetTime         *((volatile unsigned *)(IdeCtrlBase + 0x004c))   /* CF Card Reset Time Register */
#define CfAutoCheckCardStatus   *((volatile unsigned *)(IdeCtrlBase + 0x0050))   /* CF Card -> DMA Mode Auto Check Status Register */
#define CfAttmodeWriteTime      *((volatile unsigned *)(IdeCtrlBase + 0x0058))   /* CF Attribute Mode Write Timing Control Register */
#define CfCommodeWriteTime      *((volatile unsigned *)(IdeCtrlBase + 0x005C))   /* CF Common Memory Mode Write Timing Control Register */
#define CfIdemodeWriteTime      *((volatile unsigned *)(IdeCtrlBase + 0x0060))   /* CF True-IDE Mode Write Timing Control Register */
/*****************************************************************************/
/* General Processor Interface controller                            */
/*****************************************************************************/
#define GPICtrlReg              *((volatile unsigned *)(GPIUCtrlBase))
#define GPILength               *((volatile unsigned *)(GPIUCtrlBase + 0X0004))
#define GPICount                *((volatile unsigned *)(GPIUCtrlBase + 0X0008))
#define GPIIntCtrl              *((volatile unsigned *)(GPIUCtrlBase + 0X000C))
#define GPIOutput               *((volatile unsigned *)(GPIUCtrlBase + 0X0010))
#define GPIInput                *((volatile unsigned *)(GPIUCtrlBase + 0X0014))

/*****************************************************************************/
/* System Control Register                               */
/*****************************************************************************/
#define SYS_CTL0                *((volatile unsigned *)(SysCtrlBase))
#define SYS_CLK1                *((volatile unsigned *)(SysCtrlBase + 0X0004))
#define SYS_DBGIF_SEL           *((volatile unsigned *)(SysCtrlBase + 0X0008))
#define SYS_RSTCTL              *((volatile unsigned *)(SysCtrlBase + 0X000C))
#define SYS_CPU_PLLCTL          *((volatile unsigned *)(SysCtrlBase + 0X0010))
#define SYS_PWON_CFG            *((volatile unsigned *)(SysCtrlBase + 0X0014))
#define SYS_CLK2                *((volatile unsigned *)(SysCtrlBase + 0X0018))
#define SYS_DDR_PADCTL1         *((volatile unsigned *)(SysCtrlBase + 0X001C))
#define SYS_DDR_PLLCTL          *((volatile unsigned *)(SysCtrlBase + 0X0020))

#define SYS_TV_PLLCTL           *((volatile unsigned *)(SysCtrlBase + 0X0024))
#define SYS_INTERNAL_DBUG       *((volatile unsigned *)(SysCtrlBase + 0X0028))
#define SYS_ANA_TEST1           *((volatile unsigned *)(SysCtrlBase + 0X002c))
#define SYS_ANA_TEST2           *((volatile unsigned *)(SysCtrlBase + 0X0030))

#define SYS_CLK4                *((volatile unsigned *)(SysCtrlBase + 0X0034))


#define SYS_PIN_MUX_SEL         *((volatile unsigned *)(SysCtrlBase + 0X0050))
#define SYS_PAD_SR_CTL          *((volatile unsigned *)(SysCtrlBase + 0X0054))
#define SYS_DRV_E4_CTL          *((volatile unsigned *)(SysCtrlBase + 0X0058))
#define SYS_DRV_E8_CTL          *((volatile unsigned *)(SysCtrlBase + 0X005C))

#define  SYS_CHIP_IO_CFG2       *((volatile unsigned *)(SysCtrlBase + 0X0068))
#define  SYS_CTL0_EXT           *((volatile unsigned *)(SysCtrlBase + 0X006c))
#define  SYS_RSTCTL_EXT         *((volatile unsigned *)(SysCtrlBase + 0X0070))
#define  SYS_ANA_TEST1_usb2     *((volatile unsigned *)(SysCtrlBase + 0X0074))
/*****************************************************************************/
/* AC97 Control Register                                     */
/*****************************************************************************/
#define Ac97CtrlBase            IisCtrlBase /* AC97 Control Register Base           */
#define Ac97Ctrl                *((volatile unsigned *)(Ac97CtrlBase))
#define Ac97Mode                *((volatile unsigned *)(Ac97CtrlBase + 0x0004))
#define Ac97AudFormat           *((volatile unsigned *)(Ac97CtrlBase + 0x0008))
#define Ac97TxData              *((volatile unsigned *)(Ac97CtrlBase + 0x0010))
#define Ac97RxData              *((volatile unsigned *)(Ac97CtrlBase + 0x0020))
#define Ac97OCC                 *((volatile unsigned *)(Ac97CtrlBase + 0x0034))
#define Ac97ICC                 *((volatile unsigned *)(Ac97CtrlBase + 0x003c))
#define Ac97CRAC                *((volatile unsigned *)(Ac97CtrlBase + 0x0040))


/*****************************************************************************/
/* CIU Control Register                                                          */
/*****************************************************************************/
#if NEW_CIU1_EN

#define CIU_1_CTL1              *((volatile unsigned *)(CIU_1_CtrlBase))
#define CIU_1_CTL2              *((volatile unsigned *)(CIU_1_CtrlBase + 0x0004))
#define CIU_1_INTRPT            *((volatile unsigned *)(CIU_1_CtrlBase + 0x0008))
#define CIU_1_SYNC_RPT          *((volatile unsigned *)(CIU_1_CtrlBase + 0x000c))
#define CIU_1_InputSize         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0010))
#define CIU_1_IMG_STR           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0014))
#define CIU_1_InputStride       *((volatile unsigned *)(CIU_1_CtrlBase + 0x0018))
#define CIU_1_OutputSize        *((volatile unsigned *)(CIU_1_CtrlBase + 0x001c))
#define CIU_1_FRAME_STRIDE      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0020))
#define CIU_1_STR_YADDR         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0024))
#define CIU_1_STR_CbCrADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0028))
#define CIU_1_OVL_WSP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x002c))
#define CIU_1_OVL_WEP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0030))
#define CIU_1_OVL_TOPIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0034))
#define CIU_1_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_1_CtrlBase + 0x0038))
#define CIU_1_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_1_CtrlBase + 0x003c))
#define CIU_1_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_1_CtrlBase + 0x0040))
#define CIU_1_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_1_CtrlBase + 0x0044))
#define CIU_1_OVL_BOTIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0048))
#define CIU_1_OVL_STRIDE        *((volatile unsigned *)(CIU_1_CtrlBase + 0x004c))

#define CIU_1_EEREG1               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0050))
#define CIU_1_EEREG2               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0054))
#define CIU_1_EEREG3               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0058))
#define CIU_1_DropReg              *((volatile unsigned *)(CIU_1_CtrlBase + 0x005c))
#define CIU_1_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0060))
#define CIU_1_SP_STR_YADDR         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0064))
#define CIU_1_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0068))
#define CIU_1_SP_OVL_WSP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x006c))
#define CIU_1_SP_OVL_WEP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0070))
#define CIU_1_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0074))
#define CIU_1_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_1_CtrlBase + 0x0078))
#define CIU_1_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_1_CtrlBase + 0x007c))
#define CIU_1_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_1_CtrlBase + 0x0080))
#define CIU_1_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_1_CtrlBase + 0x0084))
#define CIU_1_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0088))
#define CIU_1_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_1_CtrlBase + 0x008c))
#define CIU_1_ISP_CTL              *((volatile unsigned *)(CIU_1_CtrlBase + 0x0090))
#define CIU_1_SPILTER_CTL          *((volatile unsigned *)(CIU_1_CtrlBase + 0x0094))

#define CIU_1_MSAKAREA_Yaddr       *((volatile unsigned *)(CIU_1_CtrlBase + 0x00a0))
#define CIU_1_MSAKAREA_COLOR       *((volatile unsigned *)(CIU_1_CtrlBase + 0x00a4))
#define CIU_1_Scaling_InSize       *((volatile unsigned *)(CIU_1_CtrlBase + 0x00a8))

#else

#define CIU_1_CTL1              *((volatile unsigned *)(CIU_1_CtrlBase))
#define CIU_1_CTL2              *((volatile unsigned *)(CIU_1_CtrlBase + 0x0004))
#define CIU_1_INTRPT            *((volatile unsigned *)(CIU_1_CtrlBase + 0x0008))
#define CIU_1_SYNC_RPT          *((volatile unsigned *)(CIU_1_CtrlBase + 0x000c))
#define CIU_1_InputSize         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0010))
#define CIU_1_IMG_STR           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0014))
#define CIU_1_TotalSize         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0018))
#define CIU_1_OutputSize        *((volatile unsigned *)(CIU_1_CtrlBase + 0x001c))
#define CIU_1_FRAME_STRIDE      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0020))
#define CIU_1_STR_YADDR         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0024))
#define CIU_1_STR_CbCrADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0028))
#define CIU_1_OVL_WSP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x002c))
#define CIU_1_OVL_WEP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0030))
#define CIU_1_OVL_TOPIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0034))
#define CIU_1_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_1_CtrlBase + 0x0038))
#define CIU_1_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_1_CtrlBase + 0x003c))
#define CIU_1_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_1_CtrlBase + 0x0040))
#define CIU_1_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_1_CtrlBase + 0x0044))
#define CIU_1_OVL_BOTIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0048))
#define CIU_1_OVL_STRIDE        *((volatile unsigned *)(CIU_1_CtrlBase + 0x004c))

#define CIU_1_EEREG1               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0050))
#define CIU_1_EEREG2               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0054))
#define CIU_1_EEREG3               *((volatile unsigned *)(CIU_1_CtrlBase + 0x0058))
#define CIU_1_DropReg              *((volatile unsigned *)(CIU_1_CtrlBase + 0x005c))
#define CIU_1_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0060))
#define CIU_1_SP_STR_YADDR         *((volatile unsigned *)(CIU_1_CtrlBase + 0x0064))
#define CIU_1_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0068))
#define CIU_1_SP_OVL_WSP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x006c))
#define CIU_1_SP_OVL_WEP           *((volatile unsigned *)(CIU_1_CtrlBase + 0x0070))
#define CIU_1_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0074))
#define CIU_1_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_1_CtrlBase + 0x0078))
#define CIU_1_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_1_CtrlBase + 0x007c))
#define CIU_1_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_1_CtrlBase + 0x0080))
#define CIU_1_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_1_CtrlBase + 0x0084))
#define CIU_1_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_1_CtrlBase + 0x0088))
#define CIU_1_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_1_CtrlBase + 0x008c))
#define CIU_1_ISP_CTL              *((volatile unsigned *)(CIU_1_CtrlBase + 0x0090))
#define CIU_1_SPILTER_CTL          *((volatile unsigned *)(CIU_1_CtrlBase + 0x0094))
#endif

//=====================================================================================//
#define CIU_2_CTL1              *((volatile unsigned *)(CIU_2_CtrlBase))
#define CIU_2_CTL2              *((volatile unsigned *)(CIU_2_CtrlBase + 0x0004))
#define CIU_2_INTRPT            *((volatile unsigned *)(CIU_2_CtrlBase + 0x0008))
#define CIU_2_SYNC_RPT          *((volatile unsigned *)(CIU_2_CtrlBase + 0x000c))
#define CIU_2_InputSize         *((volatile unsigned *)(CIU_2_CtrlBase + 0x0010))
#define CIU_2_IMG_STR           *((volatile unsigned *)(CIU_2_CtrlBase + 0x0014))
#define CIU_2_TotalSize         *((volatile unsigned *)(CIU_2_CtrlBase + 0x0018))
#define CIU_2_OutputSize        *((volatile unsigned *)(CIU_2_CtrlBase + 0x001c))
#define CIU_2_FRAME_STRIDE      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0020))
#define CIU_2_STR_YADDR         *((volatile unsigned *)(CIU_2_CtrlBase + 0x0024))
#define CIU_2_STR_CbCrADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0028))
#define CIU_2_OVL_WSP           *((volatile unsigned *)(CIU_2_CtrlBase + 0x002c))
#define CIU_2_OVL_WEP           *((volatile unsigned *)(CIU_2_CtrlBase + 0x0030))
#define CIU_2_OVL_TOPIADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0034))
#define CIU_2_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_2_CtrlBase + 0x0038))
#define CIU_2_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_2_CtrlBase + 0x003c))
#define CIU_2_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_2_CtrlBase + 0x0040))
#define CIU_2_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_2_CtrlBase + 0x0044))
#define CIU_2_OVL_BOTIADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0048))
#define CIU_2_OVL_STRIDE        *((volatile unsigned *)(CIU_2_CtrlBase + 0x004c))

#define CIU_2_EEREG1               *((volatile unsigned *)(CIU_2_CtrlBase + 0x0050))
#define CIU_2_EEREG2               *((volatile unsigned *)(CIU_2_CtrlBase + 0x0054))
#define CIU_2_EEREG3               *((volatile unsigned *)(CIU_2_CtrlBase + 0x0058))
#define CIU_2_DropReg              *((volatile unsigned *)(CIU_2_CtrlBase + 0x005c))
#define CIU_2_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0060))
#define CIU_2_SP_STR_YADDR         *((volatile unsigned *)(CIU_2_CtrlBase + 0x0064))
#define CIU_2_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0068))
#define CIU_2_SP_OVL_WSP           *((volatile unsigned *)(CIU_2_CtrlBase + 0x006c))
#define CIU_2_SP_OVL_WEP           *((volatile unsigned *)(CIU_2_CtrlBase + 0x0070))
#define CIU_2_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0074))
#define CIU_2_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_2_CtrlBase + 0x0078))
#define CIU_2_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_2_CtrlBase + 0x007c))
#define CIU_2_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_2_CtrlBase + 0x0080))
#define CIU_2_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_2_CtrlBase + 0x0084))
#define CIU_2_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_2_CtrlBase + 0x0088))
#define CIU_2_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_2_CtrlBase + 0x008c))
#define CIU_2_ISP_CTL              *((volatile unsigned *)(CIU_2_CtrlBase + 0x0090))
#define CIU_2_SPILTER_CTL          *((volatile unsigned *)(CIU_2_CtrlBase + 0x0094))

//=====================================================================================//
#define CIU_3_CTL1              *((volatile unsigned *)(CIU_3_CtrlBase))
#define CIU_3_CTL2              *((volatile unsigned *)(CIU_3_CtrlBase + 0x0004))
#define CIU_3_INTRPT            *((volatile unsigned *)(CIU_3_CtrlBase + 0x0008))
#define CIU_3_SYNC_RPT          *((volatile unsigned *)(CIU_3_CtrlBase + 0x000c))
#define CIU_3_InputSize         *((volatile unsigned *)(CIU_3_CtrlBase + 0x0010))
#define CIU_3_IMG_STR           *((volatile unsigned *)(CIU_3_CtrlBase + 0x0014))
#define CIU_3_TotalSize         *((volatile unsigned *)(CIU_3_CtrlBase + 0x0018))
#define CIU_3_OutputSize        *((volatile unsigned *)(CIU_3_CtrlBase + 0x001c))
#define CIU_3_FRAME_STRIDE      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0020))
#define CIU_3_STR_YADDR         *((volatile unsigned *)(CIU_3_CtrlBase + 0x0024))
#define CIU_3_STR_CbCrADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0028))
#define CIU_3_OVL_WSP           *((volatile unsigned *)(CIU_3_CtrlBase + 0x002c))
#define CIU_3_OVL_WEP           *((volatile unsigned *)(CIU_3_CtrlBase + 0x0030))
#define CIU_3_OVL_TOPIADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0034))
#define CIU_3_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_3_CtrlBase + 0x0038))
#define CIU_3_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_3_CtrlBase + 0x003c))
#define CIU_3_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_3_CtrlBase + 0x0040))
#define CIU_3_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_3_CtrlBase + 0x0044))
#define CIU_3_OVL_BOTIADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0048))
#define CIU_3_OVL_STRIDE        *((volatile unsigned *)(CIU_3_CtrlBase + 0x004c))

#define CIU_3_EEREG1               *((volatile unsigned *)(CIU_3_CtrlBase + 0x0050))
#define CIU_3_EEREG2               *((volatile unsigned *)(CIU_3_CtrlBase + 0x0054))
#define CIU_3_EEREG3               *((volatile unsigned *)(CIU_3_CtrlBase + 0x0058))
#define CIU_3_DropReg              *((volatile unsigned *)(CIU_3_CtrlBase + 0x005c))
#define CIU_3_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0060))
#define CIU_3_SP_STR_YADDR         *((volatile unsigned *)(CIU_3_CtrlBase + 0x0064))
#define CIU_3_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0068))
#define CIU_3_SP_OVL_WSP           *((volatile unsigned *)(CIU_3_CtrlBase + 0x006c))
#define CIU_3_SP_OVL_WEP           *((volatile unsigned *)(CIU_3_CtrlBase + 0x0070))
#define CIU_3_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0074))
#define CIU_3_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_3_CtrlBase + 0x0078))
#define CIU_3_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_3_CtrlBase + 0x007c))
#define CIU_3_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_3_CtrlBase + 0x0080))
#define CIU_3_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_3_CtrlBase + 0x0084))
#define CIU_3_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_3_CtrlBase + 0x0088))
#define CIU_3_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_3_CtrlBase + 0x008c))
#define CIU_3_ISP_CTL              *((volatile unsigned *)(CIU_3_CtrlBase + 0x0090))
#define CIU_3_SPILTER_CTL          *((volatile unsigned *)(CIU_3_CtrlBase + 0x0094))

//=====================================================================================//
#define CIU_4_CTL1              *((volatile unsigned *)(CIU_4_CtrlBase))
#define CIU_4_CTL2              *((volatile unsigned *)(CIU_4_CtrlBase + 0x0004))
#define CIU_4_INTRPT            *((volatile unsigned *)(CIU_4_CtrlBase + 0x0008))
#define CIU_4_SYNC_RPT          *((volatile unsigned *)(CIU_4_CtrlBase + 0x000c))
#define CIU_4_InputSize         *((volatile unsigned *)(CIU_4_CtrlBase + 0x0010))
#define CIU_4_IMG_STR           *((volatile unsigned *)(CIU_4_CtrlBase + 0x0014))
#define CIU_4_TotalSize         *((volatile unsigned *)(CIU_4_CtrlBase + 0x0018))
#define CIU_4_OutputSize        *((volatile unsigned *)(CIU_4_CtrlBase + 0x001c))
#define CIU_4_FRAME_STRIDE      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0020))
#define CIU_4_STR_YADDR         *((volatile unsigned *)(CIU_4_CtrlBase + 0x0024))
#define CIU_4_STR_CbCrADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0028))
#define CIU_4_OVL_WSP           *((volatile unsigned *)(CIU_4_CtrlBase + 0x002c))
#define CIU_4_OVL_WEP           *((volatile unsigned *)(CIU_4_CtrlBase + 0x0030))
#define CIU_4_OVL_TOPIADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0034))
#define CIU_4_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_4_CtrlBase + 0x0038))
#define CIU_4_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_4_CtrlBase + 0x003c))
#define CIU_4_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_4_CtrlBase + 0x0040))
#define CIU_4_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_4_CtrlBase + 0x0044))
#define CIU_4_OVL_BOTIADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0048))
#define CIU_4_OVL_STRIDE        *((volatile unsigned *)(CIU_4_CtrlBase + 0x004c))

#define CIU_4_EEREG1               *((volatile unsigned *)(CIU_4_CtrlBase + 0x0050))
#define CIU_4_EEREG2               *((volatile unsigned *)(CIU_4_CtrlBase + 0x0054))
#define CIU_4_EEREG3               *((volatile unsigned *)(CIU_4_CtrlBase + 0x0058))
#define CIU_4_DropReg              *((volatile unsigned *)(CIU_4_CtrlBase + 0x005c))
#define CIU_4_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0060))
#define CIU_4_SP_STR_YADDR         *((volatile unsigned *)(CIU_4_CtrlBase + 0x0064))
#define CIU_4_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0068))
#define CIU_4_SP_OVL_WSP           *((volatile unsigned *)(CIU_4_CtrlBase + 0x006c))
#define CIU_4_SP_OVL_WEP           *((volatile unsigned *)(CIU_4_CtrlBase + 0x0070))
#define CIU_4_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0074))
#define CIU_4_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_4_CtrlBase + 0x0078))
#define CIU_4_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_4_CtrlBase + 0x007c))
#define CIU_4_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_4_CtrlBase + 0x0080))
#define CIU_4_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_4_CtrlBase + 0x0084))
#define CIU_4_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_4_CtrlBase + 0x0088))
#define CIU_4_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_4_CtrlBase + 0x008c))
#define CIU_4_ISP_CTL              *((volatile unsigned *)(CIU_4_CtrlBase + 0x0090))
#define CIU_4_SPILTER_CTL          *((volatile unsigned *)(CIU_4_CtrlBase + 0x0094))

//=====================================================================================//
#define CIU_5_CTL1              *((volatile unsigned *)(CIU_5_CtrlBase))
#define CIU_5_CTL2              *((volatile unsigned *)(CIU_5_CtrlBase + 0x0004))
#define CIU_5_INTRPT            *((volatile unsigned *)(CIU_5_CtrlBase + 0x0008))
#define CIU_5_SYNC_RPT          *((volatile unsigned *)(CIU_5_CtrlBase + 0x000c))
#define CIU_5_InputSize         *((volatile unsigned *)(CIU_5_CtrlBase + 0x0010))
#define CIU_5_IMG_STR           *((volatile unsigned *)(CIU_5_CtrlBase + 0x0014))
#define CIU_5_InputStride       *((volatile unsigned *)(CIU_5_CtrlBase + 0x0018))
#define CIU_5_OutputSize        *((volatile unsigned *)(CIU_5_CtrlBase + 0x001c))
#define CIU_5_FRAME_STRIDE      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0020))
#define CIU_5_STR_YADDR         *((volatile unsigned *)(CIU_5_CtrlBase + 0x0024))
#define CIU_5_STR_CbCrADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0028))
#define CIU_5_OVL_WSP           *((volatile unsigned *)(CIU_5_CtrlBase + 0x002c))
#define CIU_5_OVL_WEP           *((volatile unsigned *)(CIU_5_CtrlBase + 0x0030))
#define CIU_5_OVL_TOPIADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0034))
#define CIU_5_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_5_CtrlBase + 0x0038))
#define CIU_5_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_5_CtrlBase + 0x003c))
#define CIU_5_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_5_CtrlBase + 0x0040))
#define CIU_5_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_5_CtrlBase + 0x0044))
#define CIU_5_OVL_BOTIADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0048))
#define CIU_5_OVL_STRIDE        *((volatile unsigned *)(CIU_5_CtrlBase + 0x004c))

#define CIU_5_EEREG1               *((volatile unsigned *)(CIU_5_CtrlBase + 0x0050))
#define CIU_5_EEREG2               *((volatile unsigned *)(CIU_5_CtrlBase + 0x0054))
#define CIU_5_EEREG3               *((volatile unsigned *)(CIU_5_CtrlBase + 0x0058))
#define CIU_5_DropReg              *((volatile unsigned *)(CIU_5_CtrlBase + 0x005c))
#define CIU_5_SP_FRAME_STRIDE      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0060))
#define CIU_5_SP_STR_YADDR         *((volatile unsigned *)(CIU_5_CtrlBase + 0x0064))
#define CIU_5_SP_STR_CbCrADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0068))
#define CIU_5_SP_OVL_WSP           *((volatile unsigned *)(CIU_5_CtrlBase + 0x006c))
#define CIU_5_SP_OVL_WEP           *((volatile unsigned *)(CIU_5_CtrlBase + 0x0070))
#define CIU_5_SP_OVL_TOPIADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0074))
#define CIU_5_SP_OVL_IDXCOLOR_Y    *((volatile unsigned *)(CIU_5_CtrlBase + 0x0078))
#define CIU_5_SP_OVL_IDXCOLOR_CB   *((volatile unsigned *)(CIU_5_CtrlBase + 0x007c))
#define CIU_5_SP_OVL_IDXCOLOR_CR   *((volatile unsigned *)(CIU_5_CtrlBase + 0x0080))
#define CIU_5_SP_OVL_MAXBYTECNTLIM *((volatile unsigned *)(CIU_5_CtrlBase + 0x0084))
#define CIU_5_SP_OVL_BOTIADDR      *((volatile unsigned *)(CIU_5_CtrlBase + 0x0088))
#define CIU_5_SP_OVL_STRIDE        *((volatile unsigned *)(CIU_5_CtrlBase + 0x008c))
#define CIU_5_ISP_CTL              *((volatile unsigned *)(CIU_5_CtrlBase + 0x0090))
#define CIU_5_SPILTER_CTL          *((volatile unsigned *)(CIU_5_CtrlBase + 0x0094))

#define CIU_5_MSAKAREA_Yaddr       *((volatile unsigned *)(CIU_5_CtrlBase + 0x00a0))
#define CIU_5_MSAKAREA_COLOR       *((volatile unsigned *)(CIU_5_CtrlBase + 0x00a4))
#define CIU_5_Scaling_InSize       *((volatile unsigned *)(CIU_5_CtrlBase + 0x00a8))

/*****************************************************************************/
/* Motion Detect Control Register                                                     */
/*****************************************************************************/
#define MD_CTRL                 *((volatile unsigned *)(MDUCtrlBase + 0x0000))
#define MD_THRESHOLD            *((volatile unsigned *)(MDUCtrlBase + 0x0004))
#define MD_IMGWIDTH             *((volatile unsigned *)(MDUCtrlBase + 0x0008))
#define MD_BLOCKNUM             *((volatile unsigned *)(MDUCtrlBase + 0x000c))
#define MD_Y_BASEADDR           *((volatile unsigned *)(MDUCtrlBase + 0x0010))
#define MD_INT_ENA              *((volatile unsigned *)(MDUCtrlBase + 0x0014))
#define MD_INT_STA              *((volatile unsigned *)(MDUCtrlBase + 0x0018))

#define MD_TRIG_MAP0            *((volatile unsigned *)(MDUCtrlBase + 0x0020))
#define MD_TRIG_MAP1            *((volatile unsigned *)(MDUCtrlBase + 0x0024))
#define MD_TRIG_MAP2            *((volatile unsigned *)(MDUCtrlBase + 0x0028))
#define MD_TRIG_MAP3            *((volatile unsigned *)(MDUCtrlBase + 0x002c))
#define MD_TRIG_MAP4            *((volatile unsigned *)(MDUCtrlBase + 0x0030))
#define MD_TRIG_MAP5            *((volatile unsigned *)(MDUCtrlBase + 0x0034))
#define MD_TRIG_MAP6            *((volatile unsigned *)(MDUCtrlBase + 0x0038))
#define MD_TRIG_MAP7            *((volatile unsigned *)(MDUCtrlBase + 0x003c))
#define MD_TRIG_MAP8            *((volatile unsigned *)(MDUCtrlBase + 0x0040))
#define MD_TRIG_MAP9            *((volatile unsigned *)(MDUCtrlBase + 0x0044))
#define MD_TRIG_MAP10           *((volatile unsigned *)(MDUCtrlBase + 0x0048))
#define MD_TRIG_MAP11           *((volatile unsigned *)(MDUCtrlBase + 0x004c))
#define MD_TRIG_MAP12           *((volatile unsigned *)(MDUCtrlBase + 0x0050))

#define REG_MD_MEANSRAM         (MDUCtrlBase + 0x1000)


/*****************************************************************************/
/* RFIU Control Register                                                     */
/*****************************************************************************/
#define  RFIU_INT_EN            *((volatile unsigned *)(RFI_1_CtrlBase + 0x0008))   //Lucian: Interrupt ¦@¥Î
#define  RFIU_INT_STA           *((volatile unsigned *)(RFI_1_CtrlBase + 0x000c))

#define  RFIU_DEBUG_PORT1        *((volatile unsigned *)(RFI_1_CtrlBase + 0x0300))
#define  RFIU_DEBUG_PORT2        *((volatile unsigned *)(RFI_1_CtrlBase + 0x0304))
#define  RFIU_DEBUG_PORT3        *((volatile unsigned *)(RFI_1_CtrlBase + 0x0308))


#define  REG_RFIU_CNTL_0            (RFI_1_CtrlBase)
#define  REG_RFIU_CNTL_1            (RFI_1_CtrlBase + 0x0004)
#define  REG_RFIU_SyncWord_L        (RFI_1_CtrlBase + 0x0010)
#define  REG_RFIU_SyncWord_H        (RFI_1_CtrlBase + 0x0014)
#define  REG_RFIU_RxClockAdjust     (RFI_1_CtrlBase + 0x0018)
#define  REG_RFIU_DclkConfig        (RFI_1_CtrlBase + 0x001c)
#define  REG_RFIU_BurstPktCNTL      (RFI_1_CtrlBase + 0x0020)
#define  REG_RFIU_PktGrp_0_Addr     (RFI_1_CtrlBase + 0x0024)
#define  REG_RFIU_TxRxOpBaseAddr    (RFI_1_CtrlBase + 0x0028)
#define  REG_RFIU_RxTimingFineTune  (RFI_1_CtrlBase + 0x002c)
#define  REG_RFIU_PacketMap_0       (RFI_1_CtrlBase + 0x0030)
#define  REG_RFIU_PacketMap_1       (RFI_1_CtrlBase + 0x0034)
#define  REG_RFIU_PacketMap_2       (RFI_1_CtrlBase + 0x0038)
#define  REG_RFIU_PacketMap_3       (RFI_1_CtrlBase + 0x003c)
#define  REG_RFIU_PacketMap_4       (RFI_1_CtrlBase + 0x0040)
#define  REG_RFIU_PacketMap_5       (RFI_1_CtrlBase + 0x0044)
#define  REG_RFIU_PacketMap_6       (RFI_1_CtrlBase + 0x0048)
#define  REG_RFIU_PacketMap_7       (RFI_1_CtrlBase + 0x004c)
#define  REG_RFIU_PktGrp_1_Addr     (RFI_1_CtrlBase + 0x0050)
#define  REG_RFIU_PktGrp_2_Addr     (RFI_1_CtrlBase + 0x0054)
#define  REG_RFIU_PktGrp_3_Addr     (RFI_1_CtrlBase + 0x0058)
#define  REG_RFIU_PktGrp_4_Addr     (RFI_1_CtrlBase + 0x005c)
#define  REG_RFIU_PktGrp_5_Addr     (RFI_1_CtrlBase + 0x0060)
#define  REG_RFIU_PktGrp_6_Addr     (RFI_1_CtrlBase + 0x0064)
#define  REG_RFIU_PktGrp_7_Addr     (RFI_1_CtrlBase + 0x0068)
#define  REG_RFIU_PktGrp_8_Addr     (RFI_1_CtrlBase + 0x006c)
#define  REG_RFIU_PktGrp_9_Addr     (RFI_1_CtrlBase + 0x0070)
#define  REG_RFIU_PktGrp_10_Addr    (RFI_1_CtrlBase + 0x0074)
#define  REG_RFIU_PktGrp_11_Addr    (RFI_1_CtrlBase + 0x0078)
#define  REG_RFIU_PktGrp_12_Addr    (RFI_1_CtrlBase + 0x007c)
#define  REG_RFIU_PktGrp_13_Addr    (RFI_1_CtrlBase + 0x0080)
#define  REG_RFIU_PktGrp_14_Addr    (RFI_1_CtrlBase + 0x0084)
#define  REG_RFIU_PktGrp_15_Addr    (RFI_1_CtrlBase + 0x0088)
#define  REG_RFIU_PacketMap_8       (RFI_1_CtrlBase + 0x008c)
#define  REG_RFIU_PacketMap_9       (RFI_1_CtrlBase + 0x0090)
#define  REG_RFIU_PacketMap_10      (RFI_1_CtrlBase + 0x0094)
#define  REG_RFIU_PacketMap_11      (RFI_1_CtrlBase + 0x0098)
#define  REG_RFIU_PacketMap_12      (RFI_1_CtrlBase + 0x009c)
#define  REG_RFIU_PacketMap_13      (RFI_1_CtrlBase + 0x00a0)
#define  REG_RFIU_PacketMap_14      (RFI_1_CtrlBase + 0x00a4)
#define  REG_RFIU_PacketMap_15      (RFI_1_CtrlBase + 0x00a8)
#define  REG_RFIU_PacketMap_16      (RFI_1_CtrlBase + 0x00ac)
#define  REG_RFIU_PacketMap_17      (RFI_1_CtrlBase + 0x00b0)
#define  REG_RFIU_PacketMap_18      (RFI_1_CtrlBase + 0x00b4)
#define  REG_RFIU_PacketMap_19      (RFI_1_CtrlBase + 0x00b8)
#define  REG_RFIU_PacketMap_20      (RFI_1_CtrlBase + 0x00bc)
#define  REG_RFIU_PacketMap_21      (RFI_1_CtrlBase + 0x00c0)
#define  REG_RFIU_PacketMap_22      (RFI_1_CtrlBase + 0x00c4)
#define  REG_RFIU_PacketMap_23      (RFI_1_CtrlBase + 0x00c8)
#define  REG_RFIU_PacketMap_24      (RFI_1_CtrlBase + 0x00cc)
#define  REG_RFIU_PacketMap_25      (RFI_1_CtrlBase + 0x00d0)
#define  REG_RFIU_PacketMap_26      (RFI_1_CtrlBase + 0x00d4)
#define  REG_RFIU_PacketMap_27      (RFI_1_CtrlBase + 0x00d8)
#define  REG_RFIU_PacketMap_28      (RFI_1_CtrlBase + 0x00dc)
#define  REG_RFIU_PacketMap_29      (RFI_1_CtrlBase + 0x00e0)
#define  REG_RFIU_PacketMap_30      (RFI_1_CtrlBase + 0x00e4)
#define  REG_RFIU_PacketMap_31      (RFI_1_CtrlBase + 0x00e8)



#define  REG_RFIU_ErrReport_1       (RFI_1_CtrlBase + 0x00f0)
#define  REG_RFIU_ErrReport_2       (RFI_1_CtrlBase + 0x00f4)
#define  REG_RFIU_DRAMADDRLIMIT     (RFI_1_CtrlBase + 0x00f8)
#define  REG_RFIU_BITSTUFF_DECT     (RFI_1_CtrlBase + 0x00fc)




/*****************************************************************************/
/* Memory Copy Unit Control Register                                         */
/*****************************************************************************/
#define MCPU_Command         *((volatile unsigned *)(MCPUCtrlBase + 0x0000))
#define MCPU_IntStat         *((volatile unsigned *)(MCPUCtrlBase + 0x0004))
#define MCPU_SrcAddr         *((volatile unsigned *)(MCPUCtrlBase + 0x0008))
#define MCPU_DstAddr         *((volatile unsigned *)(MCPUCtrlBase + 0x000c))
#define MCPU_DstStrWord      *((volatile unsigned *)(MCPUCtrlBase + 0x0010))
#define MCPU_DstLastWord     *((volatile unsigned *)(MCPUCtrlBase + 0x0014))
#define MCPU_ByteNum         *((volatile unsigned *)(MCPUCtrlBase + 0x0018))

#define MCPU_DstBlkSize      *((volatile unsigned *)(MCPUCtrlBase + 0x0020))
#define MCPU_FATZeroCnt      *((volatile unsigned *)(MCPUCtrlBase + 0x0024))


/*****************************************************************************/
/* Memory Copy Unit 2 Control Register                                         */
/*****************************************************************************/
#define MCPU2_Command         *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0000))
#define MCPU2_IntStat         *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0004))
#define MCPU2_SrcAddr         *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0008))
#define MCPU2_DstAddr         *((volatile unsigned *)(MCPU_2_CtrlBase + 0x000c))
#define MCPU2_DstStrWord      *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0010))
#define MCPU2_DstLastWord     *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0014))
#define MCPU2_ByteNum         *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0018))
#define MCPU2_DstBlkSize      *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0020))
#define MCPU2_FATZeroCnt      *((volatile unsigned *)(MCPU_2_CtrlBase + 0x0024))


/*****************************************************************************/
/* IMA ADPCM Unit Control Register                                           */
/*****************************************************************************/
#define IAU_COMMAND             (IAUCtrlBase + 0x0000)
#define IAU_INT_STAT            (IAUCtrlBase + 0x0004)
#define IAU_PCM_SIZE1           (IAUCtrlBase + 0x0008)
#define IAU_PCM_TOTAL_SIZE      (IAUCtrlBase + 0x000c)
#define IAU_ADPCM_SIZE          (IAUCtrlBase + 0x0010)
#define IAU_PCM_ADDRESS1        (IAUCtrlBase + 0x0014)
#define IAU_PCM_ADDRESS2        (IAUCtrlBase + 0x0018)
#define IAU_ADPCM_ADDRESS       (IAUCtrlBase + 0x001c)
#define IAU_PCM_STR_WORD        (IAUCtrlBase + 0x0020)
#define IAU_ADPCM_STR_WORD      (IAUCtrlBase + 0x0024)
#define IAU_ADPCM_BLOCK_HEADER  (IAUCtrlBase + 0x0028)

#define IAU_Command             *((volatile unsigned *)IAU_COMMAND)
#define IAU_IntStat             *((volatile unsigned *)IAU_INT_STAT)
#define IAU_PcmSize1            *((volatile unsigned *)IAU_PCM_SIZE1)
#define IAU_PcmTotalSize        *((volatile unsigned *)IAU_PCM_TOTAL_SIZE)
#define IAU_AdpcmSize           *((volatile unsigned *)IAU_ADPCM_SIZE)
#define IAU_PcmAddress1         *((volatile unsigned *)IAU_PCM_ADDRESS1)
#define IAU_PcmAddress2         *((volatile unsigned *)IAU_PCM_ADDRESS2)
#define IAU_AdpcmAddress        *((volatile unsigned *)IAU_ADPCM_ADDRESS)
#define IAU_PcmStrWord          *((volatile unsigned *)IAU_PCM_STR_WORD)
#define IAU_AdpcmStrWord        *((volatile unsigned *)IAU_ADPCM_STR_WORD)
#define IAU_AdpcmBlockHeader    *((volatile unsigned *)IAU_ADPCM_BLOCK_HEADER)

/*****************************************************************************/
/* Cache and TCM Control Register                                            */
/*****************************************************************************/
#define CACHE_CTL               *((volatile unsigned *)(CacheCtrlBase))
#define TCM_SRC_ADDR            *((volatile unsigned *)(CacheCtrlBase + 0X0040))
#define TCM_DST_ADDR            *((volatile unsigned *)(CacheCtrlBase + 0X0044))
#define TCM_CYCLE               *((volatile unsigned *)(CacheCtrlBase + 0X0048))
#define TCM_CTRL                *((volatile unsigned *)(CacheCtrlBase + 0X004c))

//---------------------------------------------------------------------

#define WDTctrBase                  *((volatile unsigned *)(WdtCtrlBase))

/*****************************************************************************/
/* IR Control Register                                            */
/*****************************************************************************/
#define IR_CTL                  *((volatile unsigned *)(IRCtrlBase))
#define IR_CUSTOM_CODE          *((volatile unsigned *)(IRCtrlBase + 0x0004))
#define IR_RECE_CUSTOM_CODE     *((volatile unsigned *)(IRCtrlBase + 0x0008))
#define IR_DATA                 *((volatile unsigned *)(IRCtrlBase + 0x000C))
#define IR_INT_EN               *((volatile unsigned *)(IRCtrlBase + 0x0010))
#define IR_INT_STATUS           *((volatile unsigned *)(IRCtrlBase + 0x0014))

/*****************************************************************************/
/* 2D-Graphic Control Register                                            */
/*****************************************************************************/
#define GFU_CTL                 *((volatile unsigned *)(GFUCtrlBase))
#define GFU_QUE_RP              *((volatile unsigned *)(GFUCtrlBase + 0x0004))
#define GFU_QUE_WP              *((volatile unsigned *)(GFUCtrlBase + 0x0008))
#define GFU_QUE_ADDR            *((volatile unsigned *)(GFUCtrlBase + 0x000c))
#define GFU_QUE_SIZE            *((volatile unsigned *)(GFUCtrlBase + 0x0010))

/*****************************************************************************/
/* DES/AES Control Register                                                  */
/*****************************************************************************/
#define DES_CTRL                *((volatile unsigned *)(DES_CtrlBase))
#define DES_SOURCE_ADDR         *((volatile unsigned *)(DES_CtrlBase + 0x0004))
#define DES_OUTPUT_ADDR         *((volatile unsigned *)(DES_CtrlBase + 0x0008))
#define SHA_CS                  *((volatile unsigned *)(DES_CtrlBase + 0x000C))
#define DES_IV                  *((volatile unsigned *)(DES_CtrlBase + 0x0020))
#define DES_KEY1                *((volatile unsigned *)(DES_CtrlBase + 0x0028))
#define DES_KEY2                *((volatile unsigned *)(DES_CtrlBase + 0x0030))
#define DES_KEY3                *((volatile unsigned *)(DES_CtrlBase + 0x0038))
#define SHA_K1                  *((volatile unsigned *)(DES_CtrlBase + 0x0040))
#define SHA_PMAC_HEAD           *((volatile unsigned *)(DES_CtrlBase + 0x0080))
#define SHA_K2                  *((volatile unsigned *)(DES_CtrlBase + 0x0090))
#define AUTH1_CS                *((volatile unsigned *)(DES_CtrlBase + 0x00D0))

/*****************************************************************************/
/* MAC Control Register                                            */
/*****************************************************************************/
#define FTMAC110_OFFSET_ISR             (FTMAC110_BASE)
#define FTMAC110_OFFSET_IMR             (FTMAC110_BASE + 0x0004)
#define FTMAC110_OFFSET_MAC_MADR        (FTMAC110_BASE + 0x0008)
#define FTMAC110_OFFSET_MAC_LADR        (FTMAC110_BASE + 0x000C)
#define FTMAC110_OFFSET_MAHT0           (FTMAC110_BASE + 0x0010)
#define FTMAC110_OFFSET_MAHT1           (FTMAC110_BASE + 0x0014)

#define FTMAC110_OFFSET_TXPD            (FTMAC110_BASE + 0x0018)

#define FTMAC110_OFFSET_RXPD            (FTMAC110_BASE + 0x001C)
#define FTMAC110_OFFSET_TXR_BADR        (FTMAC110_BASE + 0x0020)
#define FTMAC110_OFFSET_RXR_BADR        (FTMAC110_BASE + 0x0024)
#define FTMAC110_OFFSET_ITC             (FTMAC110_BASE + 0x0028)
#define FTMAC110_OFFSET_APTC            (FTMAC110_BASE + 0x002C)
#define FTMAC110_OFFSET_DBLAC           (FTMAC110_BASE + 0x0030)
#define FTMAC110_OFFSET_FPGA_VER        (FTMAC110_BASE + 0x0034)
#define FTMAC110_OFFSET_MACCR           (FTMAC110_BASE + 0x0088)
#define FTMAC110_OFFSET_MACSR           (FTMAC110_BASE + 0x008C)
#define FTMAC110_OFFSET_PHYCR           (FTMAC110_BASE + 0x0090)
#define FTMAC110_OFFSET_PHYWDATA        (FTMAC110_BASE + 0x0094)
#define FTMAC110_OFFSET_FCR             (FTMAC110_BASE + 0x0098)
#define FTMAC110_OFFSET_BPR             (FTMAC110_BASE + 0x009C)
#define FTMAC110_OFFSET_WOLCR           (FTMAC110_BASE + 0x00A0)
#define FTMAC110_OFFSET_WOLSR           (FTMAC110_BASE + 0x00A4)
#define FTMAC110_OFFSET_WFCRC           (FTMAC110_BASE + 0x00A8)
#define FTMAC110_OFFSET_WFBM1           (FTMAC110_BASE + 0x00B0)
#define FTMAC110_OFFSET_WFBM2           (FTMAC110_BASE + 0x00B4)
#define FTMAC110_OFFSET_WFBM3           (FTMAC110_BASE + 0x00B8)
#define FTMAC110_OFFSET_WFBM4           (FTMAC110_BASE + 0x00BC)
#define FTMAC110_OFFSET_TS              (FTMAC110_BASE + 0x00C4)
#define FTMAC110_OFFSET_DMAFIFOS        (FTMAC110_BASE + 0x00C8)
#define FTMAC110_OFFSET_TM              (FTMAC110_BASE + 0x00CC)
#define FTMAC110_OFFSET_TX_MCOL_SCOL    (FTMAC110_BASE + 0x00D4)
#define FTMAC110_OFFSET_RPF_AEP         (FTMAC110_BASE + 0x00D8)
#define FTMAC110_OFFSET_XM_PG           (FTMAC110_BASE + 0x00DC)
#define FTMAC110_OFFSET_RUNT_TLCC       (FTMAC110_BASE + 0x00E0)
#define FTMAC110_OFFSET_CRCER_FTL       (FTMAC110_BASE + 0x00E4)
#define FTMAC110_OFFSET_RLC_RCC         (FTMAC110_BASE + 0x00E8)
#define FTMAC110_OFFSET_BROC            (FTMAC110_BASE + 0x00EC)
#define FTMAC110_OFFSET_MULCA           (FTMAC110_BASE + 0x00F0)
#define FTMAC110_OFFSET_RP              (FTMAC110_BASE + 0x00F4)
#define FTMAC110_OFFSET_XP              (FTMAC110_BASE + 0x00F8)
#define SYS_PLL_SEL_MASK            0x00000003
#define SYS_PLL_SEL_48M             0x00000000
#define SYS_PLL_SEL_64M             0x00000001
#define SYS_PLL_SEL_162M            0x00000002
#define SYS_PLL_SEL_216M            0x00000003

#define SYS_PLL_SEL_IDU_MASK        0x00000c00
#define SYS_PLL_SEL_IDU_48_36       0x00000000
#define SYS_PLL_SEL_IDU_86_64       0x00000400
#define SYS_PLL_SEL_IDU_216_162     0x00000800
//#define SYS_PLL_SEL_IDU_216_162     0x00000c00

#define SYS_PLL_SEL_ADC_MASK        0x00003000
#define SYS_PLL_SEL_ADC_48_36       0x00000000
#define SYS_PLL_SEL_ADC_86_64       0x00001000
#define SYS_PLL_SEL_ADC_216_162     0x00002000
//#define SYS_PLL_SEL_ADC_216_162     0x00003000

#define SYS_CLK_SEL_MASK            0x00000004
#define SYS_CLK_SEL_XIN             0x00000000
#define SYS_CLK_SEL_PLL             0x00000004

#define IDU_CLK_DIV_MASK            0x07000000
#define IDU_CLK_DIV_5               0x04000000
#define IDU_CLK_DIV_6               0x05000000
#define IDU_CLK_DIV_8               0x07000000

#define IIS_CLK_DIV_A_MASK          0x0000ff00
#define IIS_CLK_DIV_A_1             0x00000100
#define IIS_CLK_DIV_A_2             0x00000200
#define IIS_CLK_DIV_A_3             0x00000300
#define IIS_CLK_DIV_A_4             0x00000400
#define IIS_CLK_DIV_A_5             0x00000500
#define IIS_CLK_DIV_A_6             0x00000600
#define IIS_CLK_DIV_A_7             0x00000700
#define IIS_CLK_DIV_A_8             0x00000800
#define IIS_CLK_DIV_A_9             0x00000900
#define IIS_CLK_DIV_A_10            0x00000a00
#define IIS_CLK_DIV_A_11            0x00000b00
#define IIS_CLK_DIV_A_12            0x00000c00
#define IIS_CLK_DIV_A_13            0x00000d00
#define IIS_CLK_DIV_A_14            0x00000e00
#define IIS_CLK_DIV_A_15            0x00000f00
#define IIS_CLK_DIV_A_16            0x00001000
#define IIS_CLK_DIV_A_17            0x00001100
#define IIS_CLK_DIV_A_18            0x00001200
#define IIS_CLK_DIV_A_19            0x00001300
#define IIS_CLK_DIV_A_20            0x00001400
#define IIS_CLK_DIV_A_21            0x00001500
#define IIS_CLK_DIV_A_22            0x00001600  /*Toby 0123 S*/
#define IIS_CLK_DIV_A_23            0x00001700
#define IIS_CLK_DIV_A_24            0x00001800
#define IIS_CLK_DIV_A_25            0x00001900
#define IIS_CLK_DIV_A_26            0x00001a00
#define IIS_CLK_DIV_A_27            0x00001b00  /*Toby 0123 S*/
#define IIS_CLK_DIV_A_28            0x00001c00
#define IIS_CLK_DIV_A_29            0x00001d00   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_30            0x00001e00
#define IIS_CLK_DIV_A_31            0x00001f00
#define IIS_CLK_DIV_A_32            0x00002000
#define IIS_CLK_DIV_A_33            0x00002100  /*Toby 0125 S*/
#define IIS_CLK_DIV_A_34            0x00002200
#define IIS_CLK_DIV_A_35            0x00002300
#define IIS_CLK_DIV_A_37            0x00002500
#define IIS_CLK_DIV_A_38            0x00002600
#define IIS_CLK_DIV_A_40            0x00002800  /*Toby 0125 S*/
#define IIS_CLK_DIV_A_41            0x00002900
#define IIS_CLK_DIV_A_42            0x00002a00
#define IIS_CLK_DIV_A_46            0x00002e00
#define IIS_CLK_DIV_A_47            0x00002f00
#define IIS_CLK_DIV_A_49            0x00003100   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_50            0x00003200   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_51            0x00003300   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_52            0x00003400   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_53            0x00003500  /*Peter 1109 S*/
#define IIS_CLK_DIV_A_57            0x00003900  /*Peter 1109 S*/
#define IIS_CLK_DIV_A_61            0x00003d00
#define IIS_CLK_DIV_A_68            0x00004400
#define IIS_CLK_DIV_A_71            0x00004700
#define IIS_CLK_DIV_A_72            0x00004800
#define IIS_CLK_DIV_A_73            0x00004900
#define IIS_CLK_DIV_A_80            0x00005000
#define IIS_CLK_DIV_A_86            0x00005600  /*Toby 0125 S*/
#define IIS_CLK_DIV_A_88            0x00005800
#define IIS_CLK_DIV_A_91            0x00005B00  /*Toby 0123 S*/
#define IIS_CLK_DIV_A_102           0x00006600
#define IIS_CLK_DIV_A_112           0x00007000
#define IIS_CLK_DIV_A_113           0x00007100
#define IIS_CLK_DIV_A_124           0x00007c00
#define IIS_CLK_DIV_A_138           0x00008A00
#define IIS_CLK_DIV_A_156           0x00009c00
#define IIS_CLK_DIV_A_157           0x00009d00   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_177           0x0000b100   /*Toby 0123 S*/
#define IIS_CLK_DIV_A_183           0x0000b700  /*Toby 0125 S*/
#define IIS_CLK_DIV_A_198           0x0000c600  /*Toby 0123 S*/
#define IIS_CLK_DIV_A_214           0x0000d600  /*Toby 0123 S*/
#define IIS_CLK_DIV_A_225           0x0000E100
#define IIS_CLK_DIV_A_233           0x0000E900   /*Toby 0125 S*/
#define IIS_CLK_DIV_A_245           0x0000F500   /*Toby 0123 S*/


#define IIS_CLK_DIV_B_MASK          0x00ff0000
#define IIS_CLK_DIV_B_0             0x00000000
#define IIS_CLK_DIV_B_1             0x00010000
#define IIS_CLK_DIV_B_2             0x00020000
#define IIS_CLK_DIV_B_3             0x00030000
#define IIS_CLK_DIV_B_4             0x00040000  /* yc: 0808 */ /*Peter 1109 S*/
#define IIS_CLK_DIV_B_5             0x00050000
#define IIS_CLK_DIV_B_6             0x00060000
#define IIS_CLK_DIV_B_7             0x00070000
#define IIS_CLK_DIV_B_8             0x00080000
#define IIS_CLK_DIV_B_9             0x00090000  /*Toby 0123 S*/
#define IIS_CLK_DIV_B_10            0x000a0000
#define IIS_CLK_DIV_B_11            0x000b0000   /*Toby 0123 S*/
#define IIS_CLK_DIV_B_12            0x000c0000
#define IIS_CLK_DIV_B_13            0x000d0000
#define IIS_CLK_DIV_B_14            0x000e0000
#define IIS_CLK_DIV_B_15            0x000f0000
#define IIS_CLK_DIV_B_16            0x00100000
#define IIS_CLK_DIV_B_20            0x00140000 /*Toby 0125 S*/
#define IIS_CLK_DIV_B_21            0x00150000 /*Toby 0123 S*/
#define IIS_CLK_DIV_B_22            0x00160000  /*Peter 1109 S*/
#define IIS_CLK_DIV_B_23            0x00170000 /*Toby 0125 S*/
#define IIS_CLK_DIV_B_24            0x00180000 /*Toby 0123 S*/
#define IIS_CLK_DIV_B_25            0x00190000
#define IIS_CLK_DIV_B_26            0x001a0000  /*Toby 0123 S*/
#define IIS_CLK_DIV_B_29            0x001d0000
#define IIS_CLK_DIV_B_30            0x001e0000
#define IIS_CLK_DIV_B_256           0x00ff0000
#endif

