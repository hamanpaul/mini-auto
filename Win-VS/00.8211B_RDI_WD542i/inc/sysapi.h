/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sysapi.h

Abstract:

   	The application interface of mode change.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SYS_API_H__
#define __SYS_API_H__

#if (HOME_RF_SUPPORT)
#include "../LwIP/include/tutk_P2P/MR8200def_homeautomation.h"
#endif


#define TVPARA_CWELL           0
#define TV_PARAMETER_SEL       TVPARA_CWELL
//----------------Storage Bus selection--------------//
/* GpioActFlashSelect */
#if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
      #define GPIO_ACT_FLASH_MASK               (~0x0003)
      #define GPIO_ACT_FLASH_SD                 0x0000
      #define GPIO_ACT_FLASH_SPI		        0x0000

      #define GPIO_ACT_FLASH_SMC                0x0001    //reserved
      #define GPIO_ACT_FLASH_CF                 0x0002    //reserved
      #define GPIO_ACT_FLASH_IIC2               0x00800000
      #define GPIO_DISP_FrDV1_EN                0x0004    //replace GPIU
      #define GPIO_GPIU2_FrXX_EN                0x0008

      #define GPIO_DEBUGE_EN                    0x0010
      #define GPIO_RF12_FrSP2_EN                0x0020
      #define GPIO_RF2_FrIIC_EN                 0x0040
      #define GPIO_DV2_FrGPI_EN2                0x0080

      #define GPIO_GPIU_FrDISP_EN               0x0100
      #define GPIO_EXROM_EN                     0x0200    //replace GPIU
      #define GPIO_PWM_EN                       0x0400


      #define GPIO_SPI2_FrDISP		            0x1000
      #define GPIO_DV2FrDISP_EN                 0x2000
      #define GPIO_IISFrDISP_EN                 0x4000


      #define GPIO_SPI3_FrUARTDV1		        0x100000



#elif(CHIP_OPTION == CHIP_A1013A)
      #define GPIO_ACT_FLASH_MASK               (~0x0003)

      #define GPIO_ACT_FLASH_SMC                0x0001    //reserved
      #define GPIO_ACT_FLASH_CF                 0x0002    //reserved
      #define GPIO_DISP_D8_15_EXT               0x0004    //replace DV1 data
      #define GPIO_EXROM_EN                     0x0008    //replace GPIU

      #define GPIO_DEBUGE_EN                    0x0010
      #define GPIO_RF12_FrSP2_EN                0x0020
      #define GPIO_RF2_FrIIC_EN                 0x0040
      #define GPIO_ACT_FLASH_SD                 0x0080
      #define GPIO_GPIU_FrDISP_EN               0x0100

     #if SWAP_SP1_SP3
        #define GPIO_ACT_FLASH_SPI		        0x0800
     #else
        #define GPIO_ACT_FLASH_SPI		        0x0200
     #endif
      #define GPIO_PWM_EN                       0x0400
      #define GPIO_SPI3_FrUARTDV1		        0x0800
      #define GPIO_SPI2_FrDISP		            0x1000
      #define GPIO_DV2FrDISP_EN                 0x2000
      #define GPIO_IISFrDISP_EN                 0x4000

#elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )

      #define GPIO_ACT_FLASH_MASK               (~0x0000)
      #define GPIO_ACT_FLASH_SD                 0x0000
      #define GPIO_ACT_FLASH_SPI		        0x0000

      #define GPIO_ACT_FLASH_SMC                0x0000    //reserved
      #define GPIO_ACT_FLASH_CF                 0x0000    //reserved
      #define GPIO_DISP_FrDV1_EN                0x0004    //replace GPIU
      #define GPIO_GPIU2_FrXX_EN                0x0008

      #define GPIO_DEBUGE_EN                    0x0010
      #define GPIO_RF12_FrSP2_EN                0x0020
      #define GPIO_RF2_FrIIC_EN                 0x0040
      #define GPIO_DV2_FrGPI_EN2                0x0080

      #define GPIO_GPIU_FrDISP_EN               0x0100
      #define GPIO_EXROM_EN                     0x0200    //replace GPIU
      #define GPIO_PWM_EN                       0x0400


      #define GPIO_SPI2_FrDISP		            0x1000
      #define GPIO_DV2FrDISP_EN                 0x2000
      #define GPIO_IISFrDISP_EN                 0x4000


      #define GPIO_SPI3_FrUARTDV1		        0x100000
 //chip io configureation register
      
      #define CHIP_IO_DV2_EN3                   0x00000001
      #define CHIP_IO_DISP2_EN                  0x00000002
      #define CHIP_IO_DISP_EN                   0x00000004
      #define CHIP_IO_GPI2_EN                   0x00000008
      #define CHIP_IO_DEBUG_EN                  0x00000010      
      #define CHIP_IO_RFI_EN_1                  0x00000020
      #define CHIP_IO_RF3_EN                    0x00000040
      #define CHIP_IO_DV2_EN2                   0x00000080
      #define CHIP_IO_GPI1_EN                   0x00000100
      #define CHIP_IO_DIS_ROM_EN                0x00000200
      #define CHIP_IO_PWM0_EN                   0x00000400
      #define CHIP_IO_UARTB_EN                  0x00000800
      #define CHIP_IO_SPI2_EN                   0x00001000
      #define CHIP_IO_DV2_EN                    0x00002000
      #define CHIP_IO_IIS_EN                    0x00004000
      #define CHIP_IO_RFI_ALT                   0x00008000
      #define CHIP_IO_DV2_EN4                   0x00010000
      #define CHIP_IO_FB_MODE1                  0x00020000
      #define CHIP_IO_FB_MODE2                  0x00040000
      #define CHIP_IO_CCIR2_EN                  0x00080000
      #define CHIP_IO_SPI3_EN                   0x00100000
      #define CHIP_IO_SERDISP_EN                0x00200000  
      
      
      #define CHIP_IO_RMII_EN                   0x00400000
      #define CHIP_IO_SEN_EN                    0x00800000
      #define CHIP_IO_RMII2_EN                  0x01000000
      #define CHIP_IO_RMII_EN2                  0x02000000
      #define CHIP_IO_SPI3_EN2                  0x04000000
      #define CHIP_IO_RMII_EN3                  0x08000000
      #define CHIP_IO_RFI2_EN                   0x10000000
      #define CHIP_IO_RFI2_EN2                  0x20000000
      #define CHIP_IO_I2C_EN                    0x40000000
      #define CHIP_IO_SMPTE2_EN                 0x80000000

 //chip io configureation register 2
      
      #define CHIP_IO2_MII_EN                   0x00000001
      #define CHIP_IO2_DISP_EN2                 0x00000002
      #define CHIP_IO2_RFI_ALT3                 0x00000004
      #define CHIP_IO2_ALT_NTX20E               0x00000008
      #define CHIP_IO2_ALT_NTX10E               0x00000010      
      #define CHIP_IO2_RFI_ALT2                 0x00000020
      #define CHIP_IO2_RFI_EN2                  0x00000040
      #define CHIP_IO2_RFI1012_EN               0x00000080
      #define CHIP_IO2_CCIR4CH_EN               0x00000100
      #define CHIP_IO2_ALT_NTX30E               0x00000200
      #define CHIP_IO2_RFI3_EN2                 0x00000400



#endif

//-----------------Device Reset/Enable-------------//
#if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
/*
 * SYS_CTL0
 */
  #define SYS_CTL0_SDRAM_CKEN         0x00000001
  #define SYS_CTL0_SRAM_CKEN          0x00000002

  #define SYS_CTL0_HIU_CKEN           0x00000000
  #define SYS_CTL0_RF1012_CKEN        0x00000004

  #define SYS_CTL0_SIU_CKEN           0x00000008

  #define SYS_CTL0_IPU_CKEN           0x00000010

  #define SYS_CTL0_IDU_CKEN           0x00000020
  #define SYS_CTL0_TVE_CKEN           0x00000000

  #define SYS_CTL0_ISU_CKEN           0x00000040
  #define SYS_CTL0_JPEG_CKEN          0x00000080
  #define SYS_CTL0_MPEG4_CKEN         0x00000100
  #define SYS_CTL0_STM_CKEN           0x00000200
  #define SYS_CTL0_SPI_CKEN			  0x00000200
  #define SYS_CTL0_GPIO0_CKEN         0x00000400
  #define SYS_CTL0_GPIO1_CKEN         0x00000800
  #define SYS_CTL0_GPIO2_CKEN         0x00001000
  #define SYS_CTL0_UART_CKEN          0x00002000
  #define SYS_CTL0_I2C_CKEN           0x00004000
  #define SYS_CTL0_IIS_CKEN           0x00008000
  #define SYS_CTL0_USB_CKEN           0x00010000
  #define SYS_CTL0_SD_CKEN            0x00020000
  #define SYS_CTL0_CF_CKEN            0x00020000
  #define SYS_CTL0_NAND_CKEN          0x00000000
  #define SYS_CTL0_MD_CKEN            0x00040000
  #define SYS_CTL0_TIMER0_CKEN        0x00080000
  #define SYS_CTL0_TIMER1_CKEN        0x00080000
  #define SYS_CTL0_TIMER2_CKEN        0x00080000
  #define SYS_CTL0_TIMER3_CKEN        0x00080000
  #define SYS_CTL0_TIMER4_CKEN        0x00080000
  #define SYS_CTL0_TIMER5_CKEN        0x00080000
  #define SYS_CTL0_TIMER6_CKEN        0x00080000
  #define SYS_CTL0_TIMER7_CKEN        0x00080000
  #define SYS_CTL0_RF1013_CKEN        0x00100000
  #define SYS_CTL0_SCUP_CKEN          0x00000000
  #define SYS_CTL0_CIU_CKEN           0x00400000
  #define SYS_CTL0_IR_CKEN            0x00800000


  #define SYS_CTL0_RTC_CKEN           0x01000000
  #define SYS_CTL0_WDT_CKEN           0x02000000
  #define SYS_CTL0_GPIO3_CKEN         0x04000000
  #define SYS_CTL0_CIU2_CKEN          0x08000000


  #define SYS_CTL0_GPIU_CKEN          0x10000000
  #define SYS_CTL0_MCP_CKEN           0x20000000
  #define SYS_CTL0_SER_MCKEN          0x40000000
  #define SYS_CTL0_ADC_CKEN           0x80000000

  //------SYS RESET----//
  #define SYS_RSTCTL_SDRAM_RST        0x00000001
  #define SYS_RSTCTL_SSRAM_RST        0x00000002
  #define SYS_RSTCTL_HIU_RST          0x00000000
  #define SYS_RSTCTL_RF1012_RST       0x00000004
  #define SYS_RSTCTL_SIU_RST          0x00000008
  #define SYS_RSTCTL_IPU_RST          0x00000010
  #define SYS_RSTCTL_IDU_RST          0x00000020
  #define SYS_RSTCTL_ISU_RST          0x00000040
  #define SYS_RSTCTL_JPEG_RST         0x00000080
  #define SYS_RSTCTL_MPEG4_RST        0x00000100
  #define SYS_RSTCTL_STM_RST          0x00000200
  #define SYS_RSTCTL_GPIO0_RST        0x00000400
  #define SYS_RSTCTL_GPIO1_RST        0x00000800
  #define SYS_RSTCTL_GPIO2_RST        0x00001000
  #define SYS_RSTCTL_UART_RST         0x00002000
  #define SYS_RSTCTL_I2C_RST          0x00004000
  #define SYS_RSTCTL_IIS_RST          0x00008000
  #define SYS_RSTCTL_USB_RST          0x00010000
  #define SYS_RSTCTL_SD_RST           0x00020000
  #define SYS_RSTCTL_CF_RST           0x00020000
  #define SYS_RSTCTL_NAND_RST         0x00040000
  #define SYS_RSTCTL_MD_RST           0x00040000
  #define SYS_RSTCTL_TIMER0_RST       0x00080000
  #define SYS_RSTCTL_TIMER1_RST       0x00080000
  #define SYS_RSTCTL_TIMER2_RST       0x00080000
  #define SYS_RSTCTL_TIMER3_RST       0x00080000
  #define SYS_RSTCTL_TIMER4_RST       0x00080000
  #define SYS_RSTCTL_RF1013_RST       0x00100000
  #define SYS_RSTCTL_SCUP_RST         0x00200000
  #define SYS_RSTCTL_CIU_RST          0x00400000
  #define SYS_RSTCTL_IR_RST           0x00800000
  #define SYS_RSTCTL_RTC_RST          0x01000000
  #define SYS_RSTCTL_WDT_RST          0x02000000
  #define SYS_RSTCTL_GPIO3_RST        0x04000000
  #define SYS_RSTCTL_TVE_RST          0x08000000
  #define SYS_RSTCTL_GPI_RST          0x10000000
  #define SYS_RSTCTL_MCP_RST          0x20000000

  #define SYS_RSTCTL_BIT30_RST        0x40000000
  #define SYS_RSTCTL_ADCRX_RST        0x80000000
#elif( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
/*
 * SYS_CTL0
 */
  #define SYS_CTL0_SDRAM_CKEN         0x00000001
  #define SYS_CTL0_SRAM_CKEN          0x00000002
  #define SYS_CTL0_HIU_CKEN           0x00000000
  #define SYS_CTL0_RF1012_CKEN        0x00000004
  #define SYS_CTL0_SIU_CKEN           0x00000008
  #define SYS_CTL0_IPU_CKEN           0x00000010
  #define SYS_CTL0_IDU_CKEN           0x00000020
  #define SYS_CTL0_ISU_CKEN           0x00000040
  #define SYS_CTL0_JPEG_CKEN          0x00000080
 #if (VIDEO_CODEC_OPTION == H264_CODEC)
  #define SYS_CTL0_H264_CKEN          0x00000100
  #define SYS_CTL0_MPEG4_CKEN         0x00000100
 #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
  #define SYS_CTL0_H264_CKEN          0x00000100
  #define SYS_CTL0_MPEG4_CKEN         0x00000100
 #endif
  #define SYS_CTL0_STM_CKEN           0x00000200
  #define SYS_CTL0_SPI_CKEN	          0x00000200
  #define SYS_CTL0_GPIO0_CKEN         0x00000400
  #define SYS_CTL0_GPIO1_CKEN         0x00000800
  #define SYS_CTL0_GPIO2_CKEN         0x00001000
  #define SYS_CTL0_UART_CKEN          0x00002000
  #define SYS_CTL0_I2C_CKEN           0x00004000
  #define SYS_CTL0_IIS_CKEN           0x00008000
  #define SYS_CTL0_USB_CKEN           0x00010000
  #define SYS_CTL0_SD_CKEN            0x00020000
  #define SYS_CTL0_CF_CKEN            0x00020000
  #define SYS_CTL0_NAND_CKEN          0x00000000
  #define SYS_CTL0_MD_CKEN            0x00040000
  #define SYS_CTL0_TIMER0_CKEN        0x00080000
  #define SYS_CTL0_TIMER1_CKEN        0x00080000
  #define SYS_CTL0_TIMER2_CKEN        0x00080000
  #define SYS_CTL0_TIMER3_CKEN        0x00080000
  #define SYS_CTL0_TIMER4_CKEN        0x00080000
  #define SYS_CTL0_TIMER5_CKEN        0x00080000
  #define SYS_CTL0_TIMER6_CKEN        0x00080000
  #define SYS_CTL0_TIMER7_CKEN        0x00080000
  #define SYS_CTL0_RF1013_CKEN        0x00100000
  #define SYS_CTL0_SCUP_CKEN          0x00200000
  #define SYS_CTL0_CIU_CKEN           0x00400000
  #define SYS_CTL0_IR_CKEN            0x00800000

  #define SYS_CTL0_RTC_CKEN           0x01000000
  #define SYS_CTL0_WDT_CKEN           0x02000000
  #define SYS_CTL0_GPIO3_CKEN         0x04000000
  #define SYS_CTL0_CIU2_CKEN          0x08000000  //A1018A

  #define SYS_CTL0_TVE_CKEN           0x00000000
  #define SYS_CTL0_GPIU_CKEN          0x10000000
  #define SYS_CTL0_MAC_CKEN           0x10000000
  #define SYS_CTL0_MCP_CKEN           0x20000000

  #define SYS_CTL0_SER_MCKEN          0x40000000
  #define SYS_CTL0_ADC_CKEN           0x80000000
  #define SYS_CTL0_EXT_CIU3_CKEN      0x00000001
  #define SYS_CTL0_EXT_CIU4_CKEN      0x00000002
  #define SYS_CTL0_EXT_DES_CKEN       0x00000004
  #define SYS_CTL0_EXT_MAC2_CKEN      0x00000008
  #define SYS_CTL0_EXT_IDU2_CKEN      0x00000010
  #define SYS_CTL0_EXT_IDU3_CKEN      0x00000020
  #define SYS_CTL0_EXT_IDU4_CKEN      0x00000040
  #define SYS_CTL0_EXT_MCP2_CKEN      0x00000080
  #define SYS_CTL0_EXT1_RF1013_CKEN    0x00000100
  #define SYS_CTL0_EXT2_RF1013_CKEN    0x00000200
  //------SYS RESET----//
  #define SYS_RSTCTL_SDRAM_RST        0x00000001
  #define SYS_RSTCTL_SSRAM_RST        0x00000002
  #define SYS_RSTCTL_HIU_RST          0x00000000
  #define SYS_RSTCTL_RF1012_RST       0x00000004
  #define SYS_RSTCTL_SIU_RST          0x00000008
  #define SYS_RSTCTL_IPU_RST          0x00000010
  #define SYS_RSTCTL_IDU_RST          0x00000020
  #define SYS_RSTCTL_ISU_RST          0x00000040
  #define SYS_RSTCTL_JPEG_RST         0x00000080
  #define SYS_RSTCTL_MPEG4_RST        0x00000100
  #define SYS_RSTCTL_STM_RST          0x00000200
  #define SYS_RSTCTL_GPIO0_RST        0x00000400
  #define SYS_RSTCTL_GPIO1_RST        0x00000800
  #define SYS_RSTCTL_GPIO2_RST        0x00001000
  #define SYS_RSTCTL_UART_RST         0x00002000
  #define SYS_RSTCTL_I2C_RST          0x00004000
  #define SYS_RSTCTL_IIS_RST          0x00008000
  #define SYS_RSTCTL_USB_RST          0x00010000
  #define SYS_RSTCTL_SD_RST           0x00020000
  #define SYS_RSTCTL_CF_RST           0x00020000
  #define SYS_RSTCTL_NAND_RST         0x00040000
  #define SYS_RSTCTL_MD_RST           0x00040000
  #define SYS_RSTCTL_TIMER0_RST       0x00080000
  #define SYS_RSTCTL_TIMER1_RST       0x00080000
  #define SYS_RSTCTL_TIMER2_RST       0x00080000
  #define SYS_RSTCTL_TIMER3_RST       0x00080000
  #define SYS_RSTCTL_TIMER4_RST       0x00080000
  #define SYS_RSTCTL_RF1013_RST       0x00100000
  #define SYS_RSTCTL_SCUP_RST         0x00200000
  #define SYS_RSTCTL_CIU_RST          0x00400000
  #define SYS_RSTCTL_IR_RST           0x00800000
  #define SYS_RSTCTL_RTC_RST          0x01000000
  #define SYS_RSTCTL_WDT_RST          0x02000000
  #define SYS_RSTCTL_GPIO3_RST        0x04000000
  //#define SYS_RSTCTL_TVE_RST          0x08000000
  #define SYS_RSTCTL_CIU2_RST         0x08000000
  #define SYS_RSTCTL_GPI_RST          0x10000000
  #define SYS_RSTCTL_MCP_RST          0x20000000

  #define SYS_RSTCTL_BIT30_RST        0x40000000
  #define SYS_RSTCTL_ADCRX_RST        0x80000000

  #define SYS_CTL0_EXT_CIU3_RST      0x00000001
  #define SYS_CTL0_EXT_CIU4_RST      0x00000002
  #define SYS_CTL0_EXT_DES_RST       0x00000004
  #define SYS_CTL0_EXT_MAC2_RST      0x00000008
  #define SYS_CTL0_EXT_IDU2_RST      0x00000010
  #define SYS_CTL0_EXT_IDU3_RST      0x00000020
  #define SYS_CTL0_EXT_IDU4_RST      0x00000040
  #define SYS_CTL0_EXT_MCP2_RST      0x00000080
  #define SYS_CTL0_EXT1_RF1013_RST    0x00000100
  #define SYS_CTL0_EXT2_RF1013_RST    0x00000200

#else //9001/9002
/*
 * SYS_CTL0
 */
  #define SYS_CTL0_SDRAM_CKEN         0x00000001
  #define SYS_CTL0_SRAM_CKEN          0x00000002
  #define SYS_CTL0_HIU_CKEN           0x00000004
  #define SYS_CTL0_SIU_CKEN           0x00000008
  #define SYS_CTL0_IPU_CKEN           0x00000010
  #define SYS_CTL0_IDU_CKEN           0x00000020
  #define SYS_CTL0_ISU_CKEN           0x00000040
  #define SYS_CTL0_JPEG_CKEN          0x00000080
 #if (VIDEO_CODEC_OPTION == H264_CODEC)
  #define SYS_CTL0_H264_CKEN          0x00000100
  #define SYS_CTL0_MPEG4_CKEN         0x00000100
 #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
  #define SYS_CTL0_H264_CKEN          0x00000100
  #define SYS_CTL0_MPEG4_CKEN         0x00000100
 #endif
  #define SYS_CTL0_STM_CKEN           0x00000200
  #define SYS_CTL0_SPI_CKEN			  0x00000200
  #define SYS_CTL0_GPIO0_CKEN         0x00000400
  #define SYS_CTL0_GPIO1_CKEN         0x00000800
  #define SYS_CTL0_GPIO2_CKEN         0x00001000
  #define SYS_CTL0_UART_CKEN          0x00002000
  #define SYS_CTL0_I2C_CKEN           0x00004000
  #define SYS_CTL0_IIS_CKEN           0x00008000
  #define SYS_CTL0_USB_CKEN           0x00010000
  #define SYS_CTL0_SD_CKEN            0x00020000
  #define SYS_CTL0_NAND_CKEN          0x00040000
  #define SYS_CTL0_TIMER0_CKEN        0x00080000
  #define SYS_CTL0_TIMER1_CKEN        0x00100000
  #define SYS_CTL0_TIMER2_CKEN        0x00200000
  #define SYS_CTL0_TIMER3_CKEN        0x00400000
  #define SYS_CTL0_TIMER4_CKEN        0x00800000
  #define SYS_CTL0_RTC_CKEN           0x01000000
  #define SYS_CTL0_WDT_CKEN           0x02000000
  #define SYS_CTL0_GPIO3_CKEN         0x04000000
  #define SYS_CTL0_TVE_CKEN           0x08000000
  #define SYS_CTL0_SER_MCKEN          0x40000000
  #define SYS_CTL0_ADC_CKEN           0x80000000

                    //------SYS RESET----//
  #define SYS_RSTCTL_SDRAM_RST        0x00000001
  #define SYS_RSTCTL_SSRAM_RST        0x00000002
  #define SYS_RSTCTL_HIU_RST          0x00000004
  #define SYS_RSTCTL_SIU_RST          0x00000008
  #define SYS_RSTCTL_IPU_RST          0x00000010
  #define SYS_RSTCTL_IDU_RST          0x00000020
  #define SYS_RSTCTL_ISU_RST          0x00000040
  #define SYS_RSTCTL_JPEG_RST         0x00000080
  #define SYS_RSTCTL_MPEG4_RST        0x00000100
  #define SYS_RSTCTL_STM_RST          0x00000200
  #define SYS_RSTCTL_GPIO0_RST        0x00000400
  #define SYS_RSTCTL_GPIO1_RST        0x00000800
  #define SYS_RSTCTL_GPIO2_RST        0x00001000
  #define SYS_RSTCTL_UART_RST         0x00002000
  #define SYS_RSTCTL_I2C_RST          0x00004000
  #define SYS_RSTCTL_IIS_RST          0x00008000
  #define SYS_RSTCTL_USB_RST          0x00010000
  #define SYS_RSTCTL_SD_RST           0x00020000
  #define SYS_RSTCTL_NAND_RST         0x00040000
  #define SYS_RSTCTL_TIMER0_RST       0x00080000
  #define SYS_RSTCTL_TIMER1_RST       0x00100000
  #define SYS_RSTCTL_TIMER2_RST       0x00200000
  #define SYS_RSTCTL_TIMER3_RST       0x00400000
  #define SYS_RSTCTL_TIMER4_RST       0x00800000
  #define SYS_RSTCTL_RTC_RST          0x01000000
  #define SYS_RSTCTL_WDT_RST          0x02000000
  #define SYS_RSTCTL_GPIO3_RST        0x04000000
  #define SYS_RSTCTL_TVE_RST          0x08000000
  #define SYS_RSTCTL_BIT30_RST        0x40000000
  #define SYS_RSTCTL_ADCRX_RST        0x80000000
#endif
//-----------AHB ARBIT Piority Config----------//
#if(CHIP_OPTION <= CHIP_A1013A)
#define SYS_ARBHIPIR_DEFAULT         0x00000001
#define SYS_ARBHIPIR_DMALOW          0x00000002
#define SYS_ARBHIPIR_CPU_D           0x00000004
#define SYS_ARBHIPIR_CPU_I           0x00000008
#define SYS_ARBHIPIR_IPU             0x00000010
#define SYS_ARBHIPIR_SIU             0x00000020
#define SYS_ARBHIPIR_JPGVLC          0x00000040
#define SYS_ARBHIPIR_JPGDCT          0x00000080
#define SYS_ARBHIPIR_RESV8           0x00000100
#define SYS_ARBHIPIR_RESV9           0x00000200
#define SYS_ARBHIPIR_RESV10          0x00000400
#define SYS_ARBHIPIR_IDUROT          0x00000800
#define SYS_ARBHIPIR_MCPY            0x00001000
#define SYS_ARBHIPIR_RESV13          0x00002000
#define SYS_ARBHIPIR_RESV14          0x00004000
#define SYS_ARBHIPIR_RESV15          0x00008000

#define SYS_ARBHIPIR_ISU             0x00010000
#define SYS_ARBHIPIR_IDUVID          0x00020000
#define SYS_ARBHIPIR_DMAHIGH         0x00040000
#define SYS_ARBHIPIR_RESV19          0x00080000
#define SYS_ARBHIPIR_IDUOSD          0x00100000
#define SYS_ARBHIPIR_SUBTV           0x00200000
#define SYS_ARBHIPIR_RESV22          0x00400000
#define SYS_ARBHIPIR_RESV23          0x00800000
#define SYS_ARBHIPIR_RESV24          0x01000000
#define SYS_ARBHIPIR_RESV25          0x02000000
#define SYS_ARBHIPIR_RESV26          0x04000000
#define SYS_ARBHIPIR_CIU1            0x08000000
#define SYS_ARBHIPIR_RESV28          0x10000000
#define SYS_ARBHIPIR_RESV29          0x20000000
#define SYS_ARBHIPIR_RF1013          0x40000000
#define SYS_ARBHIPIR_RESV31          0x80000000

#elif( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
#define SYS_ARBHIPIR_DEFAULT         0x00000001
#define SYS_ARBHIPIR_DMALOW          0x00000002
#define SYS_ARBHIPIR_CPU_D           0x00000004
#define SYS_ARBHIPIR_CPU_I           0x00000008
#define SYS_ARBHIPIR_IPU             0x00000010
#define SYS_ARBHIPIR_SIU             0x00000020
#define SYS_ARBHIPIR_JPGVLC          0x00000040
#define SYS_ARBHIPIR_JPGDCT          0x00000080
#define SYS_ARBHIPIR_RESV8           0x00000100
#define SYS_ARBHIPIR_MD              0x00000200
#define SYS_ARBHIPIR_RESV10          0x00000400
#define SYS_ARBHIPIR_IDUROT          0x00000800
#define SYS_ARBHIPIR_MCPY            0x00001000
#define SYS_ARBHIPIR_RESV13          0x00002000
#define SYS_ARBHIPIR_RESV14          0x00004000
#define SYS_ARBHIPIR_RESV15          0x00008000

#define SYS_ARBHIPIR_ADPCM           0x00010000
#define SYS_ARBHIPIR_IDUVID          0x00020000
#define SYS_ARBHIPIR_DMAHIGH         0x00040000
#define SYS_ARBHIPIR_RESV19          0x00080000
#define SYS_ARBHIPIR_IDUOSD          0x00100000
#define SYS_ARBHIPIR_SUBTV           0x00200000
#define SYS_ARBHIPIR_RESV22          0x00400000
#define SYS_ARBHIPIR_RESV23          0x00800000
#define SYS_ARBHIPIR_ISU             0x01000000
#define SYS_ARBHIPIR_RESV25          0x02000000
#define SYS_ARBHIPIR_RESV26          0x04000000
#define SYS_ARBHIPIR_CIU1            0x08000000
#define SYS_ARBHIPIR_RESV28          0x10000000
#define SYS_ARBHIPIR_CIU2            0x20000000
#define SYS_ARBHIPIR_RF1013          0x40000000
#define SYS_ARBHIPIR_RESV31          0x80000000


#elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define SYS_ARBHIPIR_DEFAULT         0x00000001
#define SYS_ARBHIPIR_DMALOW          0x00000002
#define SYS_ARBHIPIR_CPU_D           0x00000004
#define SYS_ARBHIPIR_CPU_I           0x00000008
#define SYS_ARBHIPIR_USB             0x00000010
#define SYS_ARBHIPIR_MD              0x00000020
#define SYS_ARBHIPIR_JPGVLC          0x00000040
#define SYS_ARBHIPIR_JPGDCT          0x00000080
#define SYS_ARBHIPIR_RESV8           0x00000100
#define SYS_ARBHIPIR_RESV9          0x00000200
#define SYS_ARBHIPIR_RESV10          0x00000400
#define SYS_ARBHIPIR_IDUROT          0x00000800
#define SYS_ARBHIPIR_MCPY            0x00001000
#define SYS_ARBHIPIR_RESV13          0x00002000
#define SYS_ARBHIPIR_RESV14          0x00004000
#define SYS_ARBHIPIR_RESV15          0x00008000

#define SYS_ARBHIPIR_ADPCM           0x00010000
#define SYS_ARBHIPIR_IDUVID          0x00020000
#define SYS_ARBHIPIR_DMAHIGH         0x00040000
#define SYS_ARBHIPIR_RESV19          0x00080000
#define SYS_ARBHIPIR_IDUOSD          0x00100000
#define SYS_ARBHIPIR_SUBTV           0x00200000
#define SYS_ARBHIPIR_RESV22          0x00400000
#define SYS_ARBHIPIR_RESV23          0x00800000
#define SYS_ARBHIPIR_RESV24          0x01000000
#define SYS_ARBHIPIR_RF1012          0x02000000
#define SYS_ARBHIPIR_RESV26          0x04000000
#define SYS_ARBHIPIR_CIU1            0x08000000
#define SYS_ARBHIPIR_RESV28          0x10000000
#define SYS_ARBHIPIR_CIU2            0x20000000
#define SYS_ARBHIPIR_RF1013          0x40000000
#define SYS_ARBHIPIR_RESV31          0x80000000
#else

#endif

/* Structure definition */

/* sys event queue to signal sysTask() */
#define SYS_EVT_MAX			8		/* max. sys event queued. Lucian: optimize 4-->8 */
typedef struct _SYS_EVT
{
	u8		cause[SYS_EVT_MAX];   		        /* cause of sys event */
	u32		param[SYS_EVT_MAX];		            /* parameter of sys event */
	u8	  	idxSet;                             /* index of set event */
	u8	  	idxGet;                             /* index of get event */
} SYS_EVT;


#define SYSBACK_RFEVT_MAX   96
typedef struct _SYSBACK_RF_EVT
{
	u8		cause[SYSBACK_RFEVT_MAX];   		/* cause of sys event */
	u32		param[SYSBACK_RFEVT_MAX];		    /* parameter of sys event */
	u8	  	idxSet;                             /* index of set event */
	u8	  	idxGet;                             /* index of get event */
} SYSBACK_RF_EVT;

#define SYSBACKLOW_EVT_MAX  8                   /* max. sysback low event queued*/
typedef struct _SYSBACKLOW_EVT
{
	u8		cause[SYSBACKLOW_EVT_MAX];   		/* cause of sys event */
	u32		param1[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32		param2[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32		param3[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32		param4[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */

	u8	  	idxSet;                             /* index of set event */
	u8	  	idxGet;                             /* index of get event */
} SYSBACKLOW_EVT;

#if (NIC_SUPPORT == 1)
#define SYSBACK_NET_EVT_MAX   10
typedef struct _SYSBACK_NET_EVT
{
	u8		cause[SYSBACK_NET_EVT_MAX];   		/* cause of sys event */
	u32		param1[SYSBACK_NET_EVT_MAX];		/* parameter 1 of sys event */
	u32		param2[SYSBACK_RFEVT_MAX];		    /* parameter 2 of sys event */
	u8	  	idxSet;                             /* index of set event */
	u8	  	idxGet;                             /* index of get event */
} SYSBACK_NET_EVT;
#endif

typedef struct _SYS_THUMBNAIL
{
	s8 tname[20];
	u8 type;
	u8 bufinx;
}SYS_THUMBNAIL;

typedef struct _SYS_CONFIG_REC
{
    u8      Overwrite;
    u32     Seccion;
    u32     Duration;
    u32     EventExtendTime;

} SYS_CONFIG_REC;

typedef struct _SYS_CONFIG_CAMERA
{
    u8      RecMode;
    u8      CamerOnOff;
    u8      Resoultion;
    u8      Brightness;
} SYS_CONFIG_CAMERA;

typedef struct _SYS_CONFIG_SCHEDULE
{
    u8      SchEnable;
} SYS_CONFIG_SCHEDULE;

typedef struct _SYS_CONFIG_EVENT
{
    u8      MotionEnable;
    u8      MotionDayLevel;
    u8      MotionNeightLevel;
    u8      PIREnable;
    u32     EventRECTime;
} SYS_CONFIG_EVENT;

typedef struct _SYS_CONFIG_TIME
{
    u8      TimeZoneSign;
    u8      TimeZoneHour;
    u8      TimeZoneMin;
    u8      DSTEnable;
} SYS_CONFIG_TIME;

typedef struct _SYS_CONFIG_ALARM
{
    u8      AlarmEnable;
    u8      AlarmVal;
    u8      AlarmRange;
} SYS_CONFIG_ALARM;

typedef struct _SYS_CONFIG_NETWORK
{
    u8      DHCPEnable;
    u8      IPAddr[4];
    u8      NetMask[4];
    u8      Gateway[4];
} SYS_CONFIG_NETWORK;

typedef struct _SYS_CONFIG_SYSTEM
{
    u8      TVOut;
    u8      Language;
    u8      Flicker;
    u8      Volume;
} SYS_CONFIG_SYSTEM;


typedef struct _SYS_CONFIG_SETTING
{
	SYS_CONFIG_REC		    RecSetting;
	SYS_CONFIG_CAMERA		CamSetting[MULTI_CHANNEL_MAX];
	SYS_CONFIG_SCHEDULE		SchSetting[MULTI_CHANNEL_MAX];
	SYS_CONFIG_EVENT		EventSetting[MULTI_CHANNEL_MAX];
	SYS_CONFIG_TIME		    TimeSetting;
	SYS_CONFIG_ALARM	    AlarmSetting;
	SYS_CONFIG_NETWORK	    NetSetting;
	SYS_CONFIG_SYSTEM	    SysSetting;
} SYS_CONFIG_SETTING;


/* Constant */

/* Event */

enum
{
    SYS_EVT_PREVIEW_INIT = 0,
    SYS_EVT_PREVIEW_RESET,
    SYS_EVT_SNAPSHOT,
    SYS_EVT_PLAYBACK_INIT,
    SYS_EVT_PLAYBACK_ZOOM,
    SYS_EVT_PLAYBACK_PAN,
    SYS_EVT_PLAYBACK_MOVE_FORWARD,
    SYS_EVT_PLAYBACK_MOVE_BACKWARD,
    SYS_EVT_PLAYBACK_DELETE,
    SYS_EVT_PLAYBACK_DELETE_ALL,
    SYS_EVT_PLAYBACK_FORMAT,
    SYS_EVT_PLAYBACK_ISP,
    SYS_EVT_VIDEOCAPTURE,
    SYS_EVT_PWROFF,
    SYS_EVT_MACRO,
    SYS_EVT_LCDROT,
    SYS_EVT_SDCD_IN,
    SYS_EVT_USB,
    SYS_EVT_SDCD_OFF,
    SYS_EVT_WhiteLight,
    SYS_EVT_FlashLight,
    SYS_EVT_VOICERECORD,
    SYS_EVT_ReadFile,
    SYS_EVT_PREVIEWZOOMINOUT,
    SYS_EVT_TVPLAYBACK_DELETE,
    SYS_EVT_VIDEOZOOMINOUT,
    SYS_EVT_PLAYBACK_DELETE_DIR,
	SYS_EVT_USB_REMOVED,
    SYS_EVT_SET_UI_KEY,
    SYS_EVT_UPGRADE_FW,
#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)
    SYS_EVT_VIDEOCAPTURE_STOP,
    SYS_EVT_VIDEOCAPTURE_RESTART,    
#endif
    SYS_EVT_SET_P2P_PLAYBACK,
    SYS_EVT_PLAYBACK_CALENDAR,
    SYS_EVT_DRAW_WAIT_LOAD,
#if RX_SNAPSHOT_SUPPORT
    SYS_EVT_RX_DATASAVE,
#endif  
    SYS_EVT_UNDEF
};
enum
{
    SYS_BACK_EVT_W_SD = 0,
#if PLAYBEEP_TEST
    SYS_BACK_PLAY_BEEP,
#endif
    SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP,
    SYS_BACK_SENSOR_FLIP,
    SYS_BACK_GET_DISK_FREE,
    SYS_BACK_VIDEOZOOMINOUT,
    SYS_BACK_TVIN_CHANELCHANGE_PREVIEW,
    SYS_BACK_TVIN_CHANELCHANGE_CAPTUREVIDEO,
    SYS_BACK_PLAYBACK_FORMAT,
    SYS_BACK_DRAWTIMEONVIDEOCLIP,
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT)
    SYS_BACK_RFI_RX_CH_RESTART,
    SYS_BACK_RFI_TX_CH_DEL,
    SYS_BACK_RFI_TX_CH_CREATE,
    SYS_BACK_RFI_TX_CHANGE_RESO,
    SYS_BACK_RFI_TX_SNAPSHOT,
  #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    SYS_BACK_RFI_TX_ENT_WIFI,
    SYS_BACK_RFI_TX_LEV_WIFI,
  #endif
#endif
#if (MOTIONDETEC_ENA==1)
    SYS_BACK_DRAW_MOTION_AREA_ONTV,
    SYS_BACK_DRAW_MOTION_AREA_ONPANEL,
#endif
#if HW_MD_SUPPORT
    SYS_BACK_DRAW_MOTION_AREA_ONTV,
#endif
    SYS_BACK_DRAW_BATTERY,
    SYS_BACK_CHECK_UI_CONTROL, /*1sec*/
    SYS_BACK_CHECK_UI_CONTROL_500MS, /*500ms*/
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)||(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
    SYS_BACK_CHECK_TVIN_FORMAT,
#endif
    SYS_BACK_CHECK_VIDEOIN_SOURCE,
    SYS_BACL_SET_SENSOR_COLOR,
    SYS_BACK_DRAW_BIT_RATE,
    SYS_BACK_DRAW_FRAME_RATE,
    SYS_BACK_DRAW_OSD_STRING,
    SYS_BACK_DRAW_SD_ICON,
#if(HOME_RF_SUPPORT)
    SYS_BACK_CHECK_HOMERF,
#endif
#if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    SYS_BACK_TURN_SPK_GPIO,
#elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    SYS_BACK_TURN_SPK_GPIO,
#elif((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    SYS_BACK_DRAW_SOUND_BAR,
#endif
#if RX_SNAPSHOT_SUPPORT
    SYS_BACK_RX_DATASAVE,
#endif
#if (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
    SYS_BACK_BLE_SYNCTIME,
#endif
    SYSBACK_EVT_UNDEF,

};

enum
{
    SYSBACKLOW_EVT_DELETEFATLINK = 0,
    SYSBACKLOW_EVT_UI_KEY_SDCD,
    SYSBACKLOW_EVT_SYN_RF,
 #if RX_SNAPSHOT_SUPPORT
    SYSBACKLOW_EVT_RXSNAP_ALL,
    SYSBACKLOW_EVT_RXSNAP_ONE,
 #endif  
    SYSBACKLOW_EVT_UNDEF
};

enum
{
    SYS_BACKRF_RFI_RX_CH_RESTART=0,
    SYS_BACKRF_RFI_TX_CH_DEL,
    SYS_BACKRF_RFI_TX_CH_CREATE,
    SYS_BACKRF_RFI_TX_CHANGE_RESO,
    SYS_BACKRF_RFI_TX_SNAPSHOT,
  #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    SYS_BACKRF_RFI_TX_ENT_WIFI,
    SYS_BACKRF_RFI_TX_LEV_WIFI,
  #endif   
    SYS_BACKRF_RFI_SAVE_RF_SETTING,
    SYS_BACKRF_FCC_DIRECT_TXRX,
    SYS_BACKRF_RFI_RX_SETOPMODE,
    SYS_BACKRF_RFI_TX_SETGPO,
    SYS_BACKRF_RFI_FORCERESYNC,
    SYS_BACKRF_RFI_CLEAR_QUADBUF,
    SYS_BACKRF_RFI_ENTER_WOR_B1,
    SYS_BACKRF_RFI_RESENDTXMDCFG,
    SYS_BACKRF_RFI_SENDTXMDSENS,
    SYS_BACKRF_RFI_TURBO_ON,
    SYS_BACKRF_RFI_TURBO_OFF,
    SYS_BACKRF_RFI_CAP_PHOTO,
    SYS_BACKRF_RFI_TX_SETPWM,
    SYS_BACKRF_RFI_TX_SETMOTORCTRL,
    SYS_BACKRF_RFI_TX_SETMELODYNUM,
    SYS_BACKRF_RFI_RX_VOXTRIG,
    SYS_BACKRF_RFI_RX_VOXCFG,
    SYS_BACKRF_RFI_SAVE_UI_SETTING,
    SYS_BACKRF_RFI_RX_LIGHTSTA,
    SYSBACK_RF_EVT_UNDEF
};

#if (NIC_SUPPORT == 1)
enum
{
    SYS_BACKRF_NTE_SEND_EVENT=0,
    SYS_BACKRF_NTE_NTP_UPDATE,
    SYS_BACKRF_NTE_FW_UPDATE,
#if CLOUD_SUPPORT
	SYS_BACKRF_NTE_SEND_CLOUD,
#endif
    SYSBACK_NTE_EVT_UNDEF
};
#endif

typedef enum dev_status
{
    DEV_SD_FULL = 0,
	DEV_SD_NOT_FULL,
	DEV_USB_PLUG_IN,
    DEV_NO_SD_CARD,
} DEV_STATUS;

/* Pan direction */
#define SYS_PAN_UP			    0x00
#define SYS_PAN_DOWN			0x01
#define SYS_PAN_LEFT			0x02
#define SYS_PAN_RIGHT			0x03

/* Untouched Panel Arrow keys */
#define SYS_UNTOUCHDE_PAN_ARROW_UP			     7
#define SYS_UNTOUCHDE_PAN_ARROW_DOWN			 8
#define SYS_UNTOUCHDE_PAN_ARROW_LEFT			 9
#define SYS_UNTOUCHDE_PAN_ARROW_RIGHT			10

/* Playback mode */
#define SYS_PLAYBACK_FORWARD_X1 0x00
#define SYS_PLAYBACK_FORWARD_X4 0x01
#define SYS_PLAYBACK_FORWARD_X8 0x02

#define CAP_PREVIEW_SCAL_UP_X1    0
#define CAP_PREVIEW_SCAL_UP_X2    1
#define CAP_PREVIEW_SCAL_UP_X3    2
#define CAP_PREVIEW_SCAL_DOWN_D2  3
#define CAP_PREVIEW_SCAL_DOWN_D4  4

#define FLAGSYS_RDYSTAT_MASK        0xFFFFFFFF

#define FLAGSYS_RDYSTAT_REC_START   0x00000001
#define FLAGSYS_RDYSTAT_REC_STOP    0x00000002
#define FLAGSYS_RDYSTAT_PLAY_START  0x00000004
#define FLAGSYS_RDYSTAT_PLAY_STOP   0x00000008

#define FLAGSYS_RDYSTAT_FORMAT      0x00000010
#define FLAGSYS_RDYSTAT_SET_REC     0x00000020
#define FLAGSYS_RDYSTAT_SET_PLAY    0x00000040
#define FLAGSYS_RDYSTAT_CARD_ERROR  0x00000080
#define FLAGSYS_RDYSTAT_CARD_READY  0x00000100
#define FLAGSYS_RDYSTAT_CARD_STATUS 0x00000200
#define FLAGSYS_RDYSTAT_PLAY_FINISH 0x00000400
#define FLAGSYS_RDYSTAT_DRAW_STOP	0x00000800
#define FLAGSYS_RDYSTAT_UPDATE_SEC	0x00001000
#define FLAGSYS_RDYSTAT_CH_SWITCH	0x00002000
#define FLAGSYS_RDYSTAT_DELETE      0x00004000
#define FLAGSYS_RDYSTAT_PWM_BEEP    0x00008000
#define FLAGSYS_RDYSTAT_PYBK_SEARCH 0x00010000
#define FLAGSYS_RDYSTAT_DRAW_LOAD   0x00020000

#define FLAGSYS_RDYSTAT_TVinFormat  0x00000200

#if MULTI_CHANNEL_VIDEO_REC
#define FLAGSYS_SUB_RDYSTAT_REC_ALL 0x0000000f
#define FLAGSYS_SUB_RDYSTAT_REC_CH0 0x00000001
#define FLAGSYS_SUB_RDYSTAT_REC_CH1 0x00000002
#define FLAGSYS_SUB_RDYSTAT_REC_CH2 0x00000004
#define FLAGSYS_SUB_RDYSTAT_REC_CH3 0x00000008
#endif

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_ALL 0x0000000f
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 0x00000001
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH1 0x00000002
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH2 0x00000004
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH3 0x00000008
#endif

/*when systask is deltetd, some flag must be set to default*/
#define FLAGSYS_SYS_INIT_RESET    (FLAGSYS_RDYSTAT_REC_START|FLAGSYS_RDYSTAT_REC_STOP|FLAGSYS_RDYSTAT_PLAY_START|\
                                    FLAGSYS_RDYSTAT_PLAY_STOP|FLAGSYS_RDYSTAT_FORMAT|FLAGSYS_RDYSTAT_SET_REC|\
                                    FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_CARD_ERROR|FLAGSYS_RDYSTAT_CARD_READY|\
                                    FLAGSYS_RDYSTAT_PLAY_FINISH)


#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
#define MR8211_QUIT_WIFI      0
#define MR8211_LEAVING_WIFI   1
#define MR8211_ENTER_WIFI     2
#endif
/*-----------Extern Variable -----------*/

#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
extern u32 sys8211TXWifiStat;
extern u32 sys8211TXWifiUserNum;
#endif

extern s32 gSystemStroageReady;	// 0: Storage is not ready to record films, 1: OK, -1: not support
extern u8  sys_format;  //  1 : success, 0 : error.
extern s32 sysFSType;	// -1: Error, 0: None, 1: can use

extern u8 siuFlashReady;

extern u8 sysForceWdt2Reboot;
extern u32 sysLifeTime;
extern u32 sysLifeTime_prev;
extern u8  sysDeadLockCheck_ena;

extern OS_EVENT* sysSemEvt;
extern SYS_EVT sysEvt;
extern OS_EVENT* speciall_MboxEvt;
extern OS_EVENT* general_MboxEvt;


extern  s32 sysPreviewZoomFactor;
extern  u8  sysCaptureImageStart;
extern  u8  sysCaptureVideoStart;
extern  u8  sysCaptureVideoStop;
extern  u32 sysCaptureVideoMode;    // ASF_CAPTURE_NORMAL, ASF_CAPTURE_OVERWRITE, ASF_CAPTURE_EVENT
extern  u8  sysPlaybackVideoStart;
extern  u8  sysPlaybackVideoStop;
extern  u8  sysPlaybackVideoPause;
#if (AVSYNC == VIDEO_FOLLOW_AUDIO)
extern  u8  sysPlaybackForward;
extern  u8  sysPlaybackBackward;
#elif (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern  s8  sysPlaybackForward;
extern  s8  sysPlaybackBackward;
#endif
extern  u8  sysPlaybackThumbnail;
extern  u32 u32PacketStart;
extern  u32 sysUSBPlugInFlag;
extern  u32	sysTVinFormat;
extern  u8 sysCaptureImageStart;
extern  BOOLEAN sysTVInFormatLocked;
extern  BOOLEAN sysTVInFormatLocked1;
extern u8 sysReady2CaptureVideo;
extern u32 sysVideoInSel;
extern u8 system_busy_flag;
extern u8 Main_Init_Ready;
extern u8 got_disk_info;
extern u8 Iframe_flag;
extern SYS_THUMBNAIL *sysThumnailPtr;
extern u32 playback_location;
extern u8  pwroff;
extern u8 SelfTimer;
extern u8 sysPlaybackCamList;
extern u8 sysPlaybackYear, sysPlaybackMonth, sysPlaybackDay;

#if MULTI_CHANNEL_VIDEO_REC
extern OS_STK          sysSubTaskStack0[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK          sysSubTaskStack1[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK          sysSubTaskStack2[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK          sysSubTaskStack3[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_FLAG_GRP     *gSysSubReadyFlagGrp;
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
extern OS_FLAG_GRP     *gRfRxVideoPackerSubReadyFlagGrp;
#endif
#endif
extern BOOLEAN MemoryFullFlag;
extern BOOLEAN SysOverwriteFlag;
extern u8 sysDisplaySDCardFail;
extern u8  sysEnMenu;
extern u8  sysEnZoom;
extern u8  sysEnSnapshot;

/*-----------------Extern Function prototype -------------*/
#if TX_SNAPSHOT_SUPPORT
  extern int sysTXSnapshotCheck(void);        
#endif

extern s32 sysSetEvt(s8, s32);
extern s32 sysbackSetEvt(s8, s32);	//civic 070828
extern s32 sysback_RF_SetEvt(s8 cause, s32 param);
#if (NIC_SUPPORT == 1)
extern s32 sysback_Net_SetEvt(s8 cause, u32 param1, u32 param2);
#endif
extern s32 sysbackLowSetEvt(s8 cause, s32 param1,s32 param2,s32 param3,s32 param4);

extern s32 sysDeadLockMonitor(void);
extern s32 sysDeadLockMonitor_ON(void);
extern s32 sysDeadLockMonitor_OFF(void);
extern s32 sysDeadLockMonitor_Reset(void);
extern u8 sysCheckSDCD(void);
extern s32 sysPreviewInit(s32);


extern s32 sysGetEvt(s8*, s32*);
extern s32 sysbackGetEvt(s8*, s32*);		//civic 070828
extern s32 sysbackLowGetEvt(s8* pCause, s32* pParam1,s32* pParam2,s32* pParam3,s32* pParam4);
extern s32 sysback_RF_GetEvt(s8* pCause, s32* pParam);
#if (NIC_SUPPORT == 1)
extern s32 sysback_Net_GetEvt(s8* pCause, u32* pParam, u32* pParam2);
#endif
extern s32 sysInit(void);
extern void InitsysEvt(void);
extern void InitsysbackEvt(void);
extern void sysTest(void);
extern s32 sysCheckNextEvtIsPrevOrNext(void);
extern s32 sysCheckNextEvtIsEmpty(void);
extern s32 sysbackCheckNextEvtIsEmpty(void);

extern void sysJPEG_enable(void);
extern void sysJPEG_disable(void);
extern void sysMPEG_enable(void);
extern void sysMPEG_disable(void);
extern void sysISU_enable(void);
extern void sysISU_disable(void);
extern void sysSIU_enable(void);
extern void sysSIU_disable(void);
extern void sysIDU_enable(void);
extern void sysIDU_disable(void);
extern void sysTVOUT_enable(void);
extern void sysTVOUT_disable(void);
extern void sysUSB_enable(void);
extern void sysUSB_disable(void);
extern void sysIIS_enable(void);
extern void sysIIS_disable(void);
extern void sysWDT_disable(void);
extern void sysWDT_enable(void);
extern void sysNAND_Enable(void);
extern void sysNAND_Disable(void);
extern void sysSD_Enable(void);
extern void sysSD_Disable(void);
extern void sysSPI_Disable(void);
extern void sysSPI_Enable(void);
extern s32 sysPreviewReset(s32);
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
extern s32 sysCiu_1_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_2_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_3_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_4_PreviewReset(s32 zoomFactor);

#endif

extern s32 sysPreviewStop(void);
extern s32 sysPreviewZoomInOut(s32);
extern s32 sysVideoZoomInOut(s32);
extern s32 sysSnapshot_OnPreview(s32);

extern s32 sysTVInChannelChange_Preview(s32 zoomFactor);
extern s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor);

extern s32 sysReadWaveRing(u8 ucRingWaveNum);
extern s32 sysReadWaveRingwithBuf(u8* pucWaveFileBuf, u8 ucRingWaveNum);
extern s32 sysPlayRingWave(u8);
extern s32 sysWriteRingWave2SDwithBuf(u8*	pucWaveFileBuf, u8 ucRingWaveNum);

extern void SYSClkEnable(u32 uiClkEnable);
extern void SYSClkDisable(u32 uiClkDisable);
extern void SYSReset(u32 uiReset);
extern void SYSReset_EXT(u32 uiReset);

extern void idu_switch(void);
extern s32 gpioSetLevel(u8, u8, u8);
extern s32 gpioGetLevel(u8, u8, u8*);
extern void iduWaitCmdBusy(void);
extern void osdDrawVideoOn(u8 on);
extern u8  sysProjectDeviceStatus(DEV_STATUS status);
extern u32 getTVinFormat(void);
extern s32 sysSDCD_OFF(s32);
extern s32 sysSDCD_IN(s32);
extern s32 sysPowerOff(s32);


extern s32 sys_background_init(void);
extern s32 sys_backLowTask_init(void);
extern s32 sys_back_RF_Task_init(void);  //處理RF task 所發出的Event

extern u8 sysSet_Overwrite(u8 pData);
extern void sysGet_Overwrite(u8* pData);
extern u8 sysSet_Seccion(u32 pData);
extern void sysGet_Seccion(u32* pData);
extern u8 sysSet_Duration(u32 pData);
extern void sysGet_Duration(u32* pData);
extern u8 sysSet_EventExtendTime(u32 pData);
extern void sysGet_EventExtendTime(u32* pData);
extern u8 sysSet_RecMode(u8 nCam, u8 pData);
extern void sysGet_RecMode(u8 nCam, u8* pData);
extern u8 sysSet_CamerOnOff(u8 nCam, u8 pData);
extern void sysGet_CamerOnOff(u8 nCam, u8* pData);
extern u8 sysSet_FormatSD(void);
extern void sysGet_Resoultion(u8 nCam, u8* pData);
extern u8 sysSet_Resoultion(u8 nCam, u8 pData);
extern void sysGet_Brightness(u8 nCam, u8* pData);
extern u8 sysSet_Brightness(u8 nCam, u8 pData);
extern void sysGet_SchEnable(u8 nCam, u8* pData);
extern u8 sysSet_SchEnable(u8 nCam, u8 pData);
extern void sysGet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8* pData);
extern u8 sysSet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8 pData);
extern void sysGet_MotionEnable(u8 nCam, u8* pEnable, u8* pDay, u8* pNight);
extern u8 sysSet_MotionEnable(u8 nCam, u8 pEnable, u8 pDay, u8 pNight);
extern void sysGet_PIREnable(u8 nCam, u8* pData);
extern u8 sysSet_PIREnable(u8 nCam, u8 pData);
extern void sysGet_TimeZone(u8* pSign, u8* pHour, u8* pMin);
extern u8 sysSet_TimeZone(u8 pSign, u8 pHour, u8 pMin);
extern void sysGet_DSTEnable(u8* pData);
extern u8 sysSet_DSTEnable(u8 pData);
extern void sysGet_AlarmEnable(u8* pData);
extern u8 sysSet_AlarmEnable(u8 pData);
extern void sysGet_AlarmVal(u8* pData);
extern u8 sysSet_AlarmVal(u8 pData); 
extern void sysGet_NetworkData(SYS_CONFIG_NETWORK *sNetData);
extern u8 sysSet_NetworkData(SYS_CONFIG_NETWORK *sNetData);
extern void sysGet_TVOut(u8* pData);
extern u8 sysSet_TVOut(u8 pData);
extern void sysGet_Language(u8* pData);
extern u8 sysSet_Language(u8 pData);
extern void sysGet_Flicker(u8* pData);
extern u8 sysSet_Flicker(u8 pData);
extern void sysGet_Volume(u8* pData);
extern u8 sysSet_Volume(u8 pData);
extern void sysGet_EventTime(u8 nCam, u32 *pData);
extern u8 sysSet_EventTime(u8 nCam, u32 pData);
extern u8 sysSet_Pair(u8 nCam);
extern u8 sysSet_FormatSD(void);
extern void sysGet_Time(RTC_DATE_TIME *pTime);
extern u8 sysSet_Time(RTC_DATE_TIME *pTime);
extern u8 sysSet_UpgradeFW(void);
extern u8 sysSet_Reset(void);
extern u8 sysGet_CurrTemp(float *temp);
extern u8 sysSetNightMode(unsigned char mode);
extern u8 sysGetNightMode(unsigned char *mode);
extern u8 sysGetLight(unsigned char *CurrentValueR,unsigned char *CurrentValueG,unsigned char *CurrentValueB,unsigned char *CurrentValueL,unsigned char *status);
extern u8 sysSetLight(unsigned char CurrentValueR,unsigned char CurrentValueG,unsigned char CurrentValueB,unsigned char CurrentValueL,unsigned char status);
extern u8 sysSetRecordMode(unsigned int mode);
extern u8 sysGetRecordMode(unsigned int *mode);
extern u8 sysSetMountSD(unsigned char mode);
extern u8 sysGetMountSD(unsigned char *mode);
extern u8 sysGetFileRecycle(unsigned char  *status);
extern u8 sysSetFileRecycle(unsigned char  status);
extern u8 sysGetFWver(char *version);
extern u8 sysGetMDSensitivity(unsigned int channel,unsigned int *sensitivity);	
extern u8 sysSetMDSensitivity(unsigned int channel,unsigned int sensitivity);	
extern u8 sysSetFrequency(unsigned int channel,unsigned char mode);
extern u8 sysGetFrequency(unsigned int channel,unsigned char *mode);
extern u8 sysSet_ResetToDef(void);
extern u8 sysGet_TempHighMargin(unsigned int channel,float *temp);
extern u8 sysSet_TempHighMargin(unsigned int channel,float temp);
extern u8 sysGet_TempLowMargin(unsigned int channel,float *temp);
extern u8 sysSet_TempLowMargin(unsigned int channel,float temp);
extern u8 sysGet_NoiseAlert(unsigned int channel,unsigned char *status);
extern u8 sysSet_NoiseAlert(unsigned int channel,unsigned char status);
extern u8 sysGet_TempAlert(unsigned int channel,unsigned char *status);
extern u8 sysSet_TempAlert(unsigned int channel,unsigned char status);
extern void sysSet_DefaultValue(void);
extern s32 uiSysMenuAction(s8 setidx);
extern void sysProjectSetSensorChrome(u8 level);
extern void sysProjectSetAudioVolume(u8 level);
extern void sysProjectSetLightOnOff(u8 level);
extern void InitsysbackLowEvt(void);


  #if (MOTIONDETEC_ENA==1)
     extern void DrawMotionArea_OnTV(unsigned int Diff);
     extern void DrawMotionArea_OnPanel(unsigned int Diff);
  #endif

  #if HW_MD_SUPPORT
     extern void DrawMotionArea_OnTV(unsigned int Diff);
  #endif




extern OS_FLAG_GRP  *gSysReadyFlagGrp;


#if(HOME_RF_SUPPORT)
extern u8 sysAppGetSensor(u32 sID, SMsgAVIoctrlGetSensorResp * response);
extern u32 sysAppAddRoom(SMsgAVIoctrlSetAddRoomReq * request);
extern u8 sysAppDeleteRoom(SMsgAVIoctrlSetDelRoomReq * request);
extern u8 sysAppDeleteSensor(SMsgAVIoctrlSetDelSensorReq * request);
extern u8 sysAppEditRoom(SMsgAVIoctrlSetEditRoomReq *request);
extern u8 sysAppEditSensor(SMsgAVIoctrlSetEditSensorReq *request);
extern void  sysAppGetRoomList(SMsgAVIoctrlGetRoomLstResp * response, u8 order);
extern void  sysAppGetSensorList(SMsgAVIoctrlGetSensorLstResp * response,u8 order);
extern void sysAppGetSceneList(SMsgAVIoctrlGetSceneLstResp * response, u8 order);
extern void sysAppAddSensor(SMsgAVIoctrlSetAddSensorResp * response);
extern u8 sysAppExecuteScene(u32 sceneID);
extern u8 gAppPairFlag; 
extern s32 sysBack_Check_HOMERF(s32 level);
extern void sysAppGetSensorName(u32 camidx);
#if CDVR_iHome_LOG_SUPPORT
extern u8 sysAppGetSensorLog(u32 sID, SMsgAVIoctrlGetSensorLogResp * response, u32 order);
extern u8 sysGetSensorLog(u8 day, HOMERF_SensorLogList * response, unsigned int order);
extern u8 sysGetSensorLogDayList(HOMERF_SensorLogDayList * response, u8 order);
#endif
enum
{
    APP_PAIR_NONE=0,
    APP_PAIR_SUCCESS,
    APP_PAIR_FAIL,
};

enum
{
    APP_IDTYPE_ROOM=0,
    APP_IDTYPE_SCENE,
};

enum
{
    HOMERF_EVENT_LOG=0,
    SYSTEM_EVENT_LOG,
};
#endif



#endif

