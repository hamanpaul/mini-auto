/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	intapi.h

Abstract:

   	The application interface of the interrupt controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/


#ifndef __INT_API_H__
#define __INT_API_H__


/*----------------- IntFiqID ------------------*/
   #if(VIDEO_CODEC_OPTION==MPEG4_CODEC)
   #define INT_FIQID_MPEG4    0 // 0x00000001
   #elif(VIDEO_CODEC_OPTION==H264_CODEC)
   #define INT_FIQID_H264     0 // 0x00000001
   #endif
   #define INT_FIQID_JPEG     1 // 0x00000002
   #define INT_FIQID_SIU      2 // 0x00000004
   #define INT_FIQID_IPU      3 // 0x00000008
   #define INT_FIQID_ISU      4 // 0x00000010
   #define INT_FIQID_IDU      5 // 0x00000020
   #define INT_FIQID_HIU      6 // 0x00000040
   #define INT_FIQID_SUBTV    7 // 0x00000080
   #define INT_FIQID_CIU1     8 // 0x00000100
   #define INT_FIQID_CIU2     9 // 0x00000200
   #define INT_FIQID_CIU3     10// 0x00000400
   #define INT_FIQID_CIU4     11// 0x00000800
   #define INT_FIQID_DIU      14
   #define INT_FIQID_MAX      15

/*---------------- IntFiqMask ------------------*/
   #if(VIDEO_CODEC_OPTION==MPEG4_CODEC)
   #define INT_FIQMASK_MPEG4  0x00000001
   #elif(VIDEO_CODEC_OPTION==H264_CODEC)
   #define INT_FIQMASK_H264  0x00000001
   #endif

   #define INT_FIQMASK_JPEG   0x00000002
   #define INT_FIQMASK_SIU    0x00000004
   #define INT_FIQMASK_IPU    0x00000008
   #define INT_FIQMASK_ISU    0x00000010
   #define INT_FIQMASK_IDU    0x00000020
   #define INT_FIQMASK_HIU    0x00000040
   #define INT_FIQMASK_SUBTV  0x00000080
   #define INT_FIQMASK_CIU1   0x00000100
   #define INT_FIQMASK_CIU2   0x00000200
   #define INT_FIQMASK_CIU3   0x00000400
   #define INT_FIQMASK_CIU4   0x00000800
   #define INT_FIQMASK_DIU    0x00004000

   /*------------------- IntIrqID ------------------*/
   //Lucian: At A1018, SD2 is used on CPU2 platform.
   #define INT_IRQID_TIMER    0 // 0x00000001
   #define INT_IRQID_ADC      1 // 0x00000002
   #define INT_IRQID_GPIU     2 // 0x00000004
   #define INT_IRQID_MD       3 // 0x00000008
   #define INT_IRQID_CF       4 // 0x00000010
   #define INT_IRQID_DMA      5 // 0x00000020
   #define INT_IRQID_GPIO_1   6 // 0x00000040
   #define INT_IRQID_SPI      7 // 0x00000080
   #define INT_IRQID_UART_1   8 // 0x00000100
   #define INT_IRQID_I2C      9 // 0x00000200
   #define INT_IRQID_GPIO_0  10 // 0x00000400   //GPIO0
   #define INT_IRQID_IR      11 // 0x00000800
   #define INT_IRQID_SDC     12 // 0x00001000
   #define INT_IRQID_USB     13 // 0x00002000
   #define INT_IRQID_RTC     14 // 0x00004000
   #define INT_IRQID_IIS     15 // 0x00008000
   #define INT_IRQID_RFI     16 // 0x00010000
   #define INT_IRQID_UART_2  17 // 0x00020000
   #define INT_IRQID_UART_3  18 // 0x00040000
   #define INT_IRQID_MCP     19 // 0x00080000
   #define INT_IRQID_MAX     20 // 0x00100000

/*-------------- IntIrqMask --------------*/
   #define INT_IRQMASK_TIMER  0x00000001
   #define INT_IRQMASK_ADC    0x00000002
   #define INT_IRQMASK_GPIU   0x00000004
   #define INT_IRQMASK_MD     0x00000008
   #define INT_IRQMASK_CF     0x00000010
   #define INT_IRQMASK_DMA    0x00000020
   #define INT_IRQMASK_GPIO_1 0x00000040
   #define INT_IRQMASK_SPI    0x00000080
   #define INT_IRQMASK_UART   0x00000100
   #define INT_IRQMASK_I2C    0x00000200
   #define INT_IRQMASK_GPIO_0 0x00000400
   #define INT_IRQMASK_IR     0x00000800
   #define INT_IRQMASK_SDC    0x00001000
   #define INT_IRQMASK_USB    0x00002000
   #define INT_IRQMASK_RTC    0x00004000
   #define INT_IRQMASK_IIS    0x00008000
   #define INT_IRQMASK_RFI    0x00010000
   #define INT_IRQMASK_UART_2 0x00020000
   #define INT_IRQMASK_UART_3 0x00040000
   #define INT_IRQMASK_MCP    0x00080000


#endif
