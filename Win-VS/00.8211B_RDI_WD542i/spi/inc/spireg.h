/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	spireg.h

Abstract:

   	The registers of Serial flash controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2008/04/17	Chi-Lun Chen	Create

*/

#ifndef __SPI_REG_H__
#define __SPI_REG_H__


/* SPI clock enable */
#define 	SYS_CTL0_SPI_CK_EN   		0x00000200
/* spi ctrl */
#define	SPI_SMP_MODE_HIGH			0x00000100	/* Sample mode select bit */
#define	SPI_TX_LEN_8BIT				0x00000000
#define	SPI_TX_LEN_16BIT			0x00000040
#define	SPI_TX_LEN_24BIT			0x00000080
#define	SPI_TX_LEN_32BIT			0x000000C0

#define 	SPI_DMA_MODE_TX			0x00000020
#define	SPI_SSOE_EN					0x00000010
#define	SPI_CPOL_HIGH				0x00000008
#define	SPI_CPHA_HIGH				0x00000004
#define	SPI_DMA_EN					0x00000002
#define	SPI_EN						0x00000001

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
/* spi int */
#if SWAP_SP1_SP3
#define	SPI_INT_DMA_END_EN				0x00000800
#define	SPI_INT_RX_OVR_EN				0x00000400
#define	SPI_INT_BUS_FIN_EN				0x00000200
#define	SPI_INT_TX_EMPTY_EN		     	0x00000100
#else
#define	SPI_INT_DMA_END_EN				0x00000008
#define	SPI_INT_RX_OVR_EN				0x00000004
#define	SPI_INT_BUS_FIN_EN				0x00000002
#define	SPI_INT_TX_EMPTY_EN			0x00000001
#endif
#define	SPI_SSOE2_EN					0x00000001

#define	SPI2_INT_DMA_END_EN				0x00000080
#define	SPI2_INT_RX_OVR_EN				0x00000040
#define	SPI2_INT_BUS_FIN_EN				0x00000020
#define	SPI2_INT_TX_EMPTY_EN			0x00000010

#define	SPI3_INT_DMA_END_EN				0x00000800
#define	SPI3_INT_RX_OVR_EN				0x00000400
#define	SPI3_INT_BUS_FIN_EN				0x00000200
#define	SPI3_INT_TX_EMPTY_EN			0x00000100

#define	SPI4_INT_DMA_END_EN				0x00008000
#define	SPI4_INT_RX_OVR_EN				0x00004000
#define	SPI4_INT_BUS_FIN_EN				0x00002000
#define	SPI4_INT_TX_EMPTY_EN		    0x00001000

#define	SPI5_INT_DMA_END_EN				0x00080000
#define	SPI5_INT_RX_OVR_EN				0x00040000
#define	SPI5_INT_BUS_FIN_EN				0x00020000
#define	SPI5_INT_TX_EMPTY_EN			0x00010000



#else
/* spi int */
#define	SPI_INT_DMA_END_EN				0x00000008
#define	SPI_INT_RX_OVR_EN				0x00000004
#define	SPI_INT_BUS_FIN_EN				0x00000002
#define	SPI_INT_TX_EMPTY_EN			    0x00000001




#endif
#endif
