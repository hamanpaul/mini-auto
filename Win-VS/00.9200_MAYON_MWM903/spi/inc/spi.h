/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	spi.h

Abstract:

   	The structures and constants of serial flash protocol.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2008/04/17	Chi-Lun Chen	Create

*/

#ifndef __SPI_H__
#define __SPI_H__

/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/

/* SMC Redundant Area */

#define SPI_OK	1
#define SPI_ERR	0

/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/

/* SPI Command Sets */
#define SPI_CMD_READ			0x03
#define SPI_CMD_HS_READ			0x0B
#define SPI_CMD_SEC_ERASE		0x20
#define SPI_CMD_BLK_ERASE		0xD8
#define SPI_CMD_CHIP_ERASE		0xC7
#define SPI_CMD_BYTE_PROGRAM	0x02
#define SPI_CMD_PAGE_PROGRAM	0x02		/* for winbond */
#define SPI_CMD_AAI_PROGRAM		0xAD
#define SPI_CMD_RDSR			0x05
#define SPI_CMD_EWSR			0x50
#define SPI_CMD_WRSR			0x01
#define SPI_CMD_WREN			0x06
#define SPI_CMD_WRDI			0x04
#define SPI_CMD_HWWDEN			0x70
#define SPI_CMD_HWWDDISA		0x80
#define SPI_CMD_RES				0xAB
#define SPI_CMD_READ_ID			0x90
#define SPI_CMD_EBSY			0x70
#define SPI_CMD_DBSY			0x80

#define SPI_CMD_ENTER_4BM		0xB7
#define SPI_CMD_EXIT_4BM		0xE9
#define SPI_CMD_READ_4BM		0x13
#define SPI_CMD_WRITE_4BM		0x12
#define SPI_CMD_SEC_ERASE_4BM	0x21
#define SPI_CMD_BLK_ERASE_4BM	0xDC



#define SPI_FREQ_MASK				16

/* SPI Status Register */
#if SWAP_SP1_SP3
#define SPI_SR_BUSY				    0x00400000
#else
#define SPI_SR_BUSY					0x00100000
#endif

#define SPI5_SR_BUSY				0x01000000
#define SPI3_SR_BUSY				0x00400000
#define SPI4_SR_BUSY				0x00800000
#define SPI2_SR_BUSY				0x00200000


#define SPI5_SR_DMA_END				0x00080000
#define SPI5_SR_RX_OVR				0x00040000
#define SPI5_SR_BUS_FIN				0x00020000
#define SPI5_SR_TX_EMPTY			0x00010000

#define SPI4_SR_DMA_END				0x00008000
#define SPI4_SR_RX_OVR				0x00004000
#define SPI4_SR_BUS_FIN				0x00002000
#define SPI4_SR_TX_EMPTY			0x00001000

#define SPI3_SR_DMA_END				0x00000800
#define SPI3_SR_RX_OVR				0x00000400
#define SPI3_SR_BUS_FIN				0x00000200
#define SPI3_SR_TX_EMPTY			0x00000100

#define SPI2_SR_DMA_END				0x00000080
#define SPI2_SR_RX_OVR				0x00000040
#define SPI2_SR_BUS_FIN				0x00000020
#define SPI2_SR_TX_EMPTY			0x00000010

#if SWAP_SP1_SP3
#define SPI_SR_DMA_END				0x00000800
#define SPI_SR_RX_OVR				0x00000400
#define SPI_SR_BUS_FIN				0x00000200
#define SPI_SR_TX_EMPTY			    0x00000100
#else
#define SPI_SR_DMA_END				0x00000008
#define SPI_SR_RX_OVR				0x00000004
#define SPI_SR_BUS_FIN				0x00000002
#define SPI_SR_TX_EMPTY				0x00000001
#endif

#define SPI_SR_TX_ENDI_BIG		(0 << 0)
#define SPI_SR_TX_ENDI_LITTLE	(1 << 0)
#define SPI_SR_RX_ENDI_BIG		(0 << 1)
#define SPI_SR_RX_ENDI_LITTLE	(1 << 1)

#define SPI2_SR_TX_ENDI_BIG		(0 << 2)
#define SPI2_SR_TX_ENDI_LITTLE	(1 << 2)
#define SPI2_SR_RX_ENDI_BIG		(0 << 3)
#define SPI2_SR_RX_ENDI_LITTLE	(1 << 3)

#define SPI3_SR_TX_ENDI_BIG		(0 << 4)
#define SPI3_SR_TX_ENDI_LITTLE	(1 << 4)
#define SPI3_SR_RX_ENDI_BIG		(0 << 5)
#define SPI3_SR_RX_ENDI_LITTLE	(1 << 5)

#define SPI4_SR_TX_ENDI_BIG		(0 << 6)
#define SPI4_SR_TX_ENDI_LITTLE	(1 << 6)
#define SPI4_SR_RX_ENDI_BIG		(0 << 7)
#define SPI4_SR_RX_ENDI_LITTLE	(1 << 7)

#define SPI5_SR_TX_ENDI_BIG		(0 << 8)
#define SPI5_SR_TX_ENDI_LITTLE	(1 << 8)
#define SPI5_SR_RX_ENDI_BIG		(0 << 9)
#define SPI5_SR_RX_ENDI_LITTLE	(1 << 9)



/* SPI Semaphore Flag */
enum
{
    SPI_SEM_FLG_BUSY=0x01,
    SPI_SEM_FLG_DMA_END,
    SPI_SEM_FLG_RX_OVR,
    SPI_SEM_FLG_BUS_FIN,
    SPI_SEM_FLG_TX_EMPTY,
    SPI_SEM_FLG_RW_HOOK, /* check read or write is hold the resource or not */
    
    SPI2_SEM_FLG_BUSY,
    SPI2_SEM_FLG_DMA_END,
    SPI2_SEM_FLG_RX_OVR,
    SPI2_SEM_FLG_BUS_FIN,
    SPI2_SEM_FLG_TX_EMPTY,
    SPI2_SEM_FLG_RW_HOOK,
    
    SPI3_SEM_FLG_BUSY,
    SPI3_SEM_FLG_DMA_END,
    SPI3_SEM_FLG_RX_OVR,
    SPI3_SEM_FLG_BUS_FIN,
    SPI3_SEM_FLG_TX_EMPTY,
    SPI3_SEM_FLG_RW_HOOK,

    SPI4_SEM_FLG_BUSY,
    SPI4_SEM_FLG_DMA_END,
    SPI4_SEM_FLG_RX_OVR,
    SPI4_SEM_FLG_BUS_FIN,
    SPI4_SEM_FLG_TX_EMPTY,
    SPI4_SEM_FLG_RW_HOOK,

    SPI5_SEM_FLG_BUSY,
    SPI5_SEM_FLG_DMA_END,
    SPI5_SEM_FLG_RX_OVR,
    SPI5_SEM_FLG_BUS_FIN,
    SPI5_SEM_FLG_TX_EMPTY,
    SPI5_SEM_FLG_RW_HOOK,
};

/* SPI command index */
enum
{
    SPI_SEM_CMD_IDX_READ=0x01,
    SPI_SEM_CMD_IDX_WRITE,
    SPI_SEM_CMD_IDX_READID,
    SPI_SEM_CMD_IDX_EWSR,
    SPI_SEM_CMD_IDX_WRSR,
    SPI_SEM_CMD_IDX_WREN,
    SPI_SEM_CMD_IDX_HWWDEN,
    SPI_SEM_CMD_IDX_HWWDDISA,
    SPI_SEM_CMD_IDX_RDSR,
    SPI_SEM_CMD_IDX_SECTOR_ERASE,
    SPI_SEM_CMD_IDX_BLOCK_ERASE,
    SPI_SEM_CMD_IDX_CHIP_ERASE,
    SPI_SEM_CMD_IDX_BYTE_PROGRAM,
    SPI_SEM_CMD_IDX_AAI_PROGRAM,
    SPI_SEM_CMD_IDX_ENTER_4BM,
    SPI_SEM_CMD_IDX_EXIT_4BM,
};

enum
{
    SPI_TEST_IDX_CHIPERASE	= 0x00,
    SPI_TEST_IDX_BLOCKERASE,
    SPI_TEST_IDX_SECTORERASE,
    SPI_TEST_IDX_BYTEPROGRAM,
};

enum
{
    SPI_MANUF_ESMT	= 0x00,
    SPI_MANUF_WINBOND,
    SPI_MANUF_EON,
};

enum
{
    SPI_AREA_NONE = 0x0,
    //SPI_AREA_USB_HEADER,
    SPI_AREA_BOOT_CODE,
    SPI_AREA_MAIN_CODE,
    SPI_AREA_CODE,
    SPI_AREA_UI_PICS,
    SPI_AREA_AUDIO_DATA,
    SPI_AREA_PIR_SENSOR,
    SPI_AREA_MARS_SENSOR,
    SPI_AREA_MARS_SYS,
    SPI_AREA_HA,
    SPI_AREA_UI_BACKUP,
    SPI_AREA_NETWORK,
    SPI_AREA_RFID,
    SPI_AREA_UI_CONFIG,

    SPI_AREA_BMP,	// boot, main, pics
    SPI_AREA_ALL,
    SPI_AREA_RFCAL,
};


#endif
