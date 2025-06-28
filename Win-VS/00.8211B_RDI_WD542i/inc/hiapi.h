/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	hiuapi.h

Abstract:

   	The application interface of host interface unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __HIU_API_H__
#define __HIU_API_H__


#include "general.h"

//----- Constant definitions for HIU registers -----

// 0xc0080000
#define HIUC_DMA_EN			0x00000008   // Host want to read data.

#define HIUC_FIFO_SEND		0x00000000   //FIFO direction
#define HIUC_FIFO_RECV		0x00000010   //FIFO direction

#define HIUC_WR_RISE		0x00000000
#define HIUC_WR_FALL		0x00000020

#define HIUC_ER_RISE		0x00000000
#define HIUC_ER_FALL		0x00000040

#define HIUC_ENDIAN_BIG		0x00000000
#define HIUC_ENDIAN_LIT		0x00000080

#define HIUC_IF_M68			0x00000100
#define HIUC_IF_I80			0x00000200


	// The interface signals for host interface
#define HIUC_SGL_RS			0x00000000  // with RS signal

	// Data TYPE OF Hiu interface
#define HIUC_DT_1b			0x00000000 // 8 bits 
#define HIUC_DT_8b			0x00001000 // 8 bits
#define HIUC_DT_9b			0x00002000 // 8 bits
#define HIUC_DT_16b			0x00003000 // 16 bits
#define HIUC_DT_18b			0x00004000 // 16 bits

	// The data latch of HIU
#define HIUC_LATCH_SYSCLK	0x00000000

#define HIUC_IF_SEL			0x00400000


// 0xc0080004

#define HIUC_P_SIZE_256		0x00000000 //maximum data size is 256 bytes
#define HIUC_P_SIZE_512		0x00000001 //maximum data size is 512 bytes
#define HIUC_P_SIZE_1K		0x00000002 //maximum data size is 1K bytes
#define HIUC_P_SIZE_2K		0x00000003 //maximum data size is 2K bytes
#define HIUC_P_SIZE_4K		0x00000004 //maximum data size is 4K bytes
#define HIUC_P_SIZE_NOB		0x00000007 //maximum data size is no boundary

#define HIUC_CMD_SIZE_MASK	0x00000F00 //
#define HIUC_CMD_SIZE_SHIFT	8

#define HIUC_CMD_SIZE_1B	0
#define HIUC_CMD_SIZE_2B	1
#define HIUC_CMD_SIZE_3B	2
#define HIUC_CMD_SIZE_4B	3
#define HIUC_CMD_SIZE_5B	4
#define HIUC_CMD_SIZE_6B	5
#define HIUC_CMD_SIZE_7B	6
#define HIUC_CMD_SIZE_8B	7
#define HIUC_CMD_SIZE_9B	8
#define HIUC_CMD_SIZE_10B	9
#define HIUC_CMD_SIZE_11B	10
#define HIUC_CMD_SIZE_12B	11
#define HIUC_CMD_SIZE_13B	12
#define HIUC_CMD_SIZE_14B	13
#define HIUC_CMD_SIZE_15B	14
#define HIUC_CMD_SIZE_16B	15



#define HIUC_CLK_DIV_MASK	0x00FF0000 //
#define HIUC_CLK_DIV_SHIFT	16

#define HIUC_DMAMODE_CPU		0x00000000 // 	use CPU to RW data fifo
#define HIUC_DMAMODE_DMA		0x01000000 // 	use DMA to RW data fifo

//0xc0080008
#define HIUC_INT_CMD_MASK	0x00000004
#define HIUC_INT_DATA_MASK	0x00000008

//----- Constant definitions for HIU registers -----
typedef struct _HIU_CfgData{
	u32  CTL0;          // 
	u32  CTL1;       	//
	u32  IntEnable;     // Interrupt Enable
	
	
}HIU_CfgData;


/*-------------------------------------------------
** HIU OS constant
---------------------------------------------------*/


#define FLAHIU_CMD_READY	0x00000001
#define FLAHIU_DATA_READY	0x00000002



/*-------------------------------------------------
** HIU API
---------------------------------------------------*/

extern void Hiu_init(void);
extern void hiu_CmdInit(void);
extern u32 hiu_open(void);
extern void hiuIntHandler(void);


extern char  HIUCMDBUF[];


#if 0
extern s32 hiuInit(void);
extern u8 hiuWriteData(void);
extern void hiuREST(void);
extern void HiuCmdComp(u32 data);

extern void HiuDataPinDir(u8);
extern void HiuSetDataPinLevel(u8);
extern u8 HiuGetDataPinLevel(void);
extern void HiuSetDataSize(u32);

extern void HiuSendData(u8* , s32);
extern void HiuRecvData(u8* , s32);

/* SW 1113 S */
extern void HiuReflashStatus(void);
extern u32 hiuOpStatus;
/* SW 1113 E */

extern u8 HIU_CMD_SIZE;
extern u8 HIU_DMA_DRAM[];

/* SW 1011 S */
extern u16 DateFileFormat;
extern u16 TimeFileFormat;
/* SW 1011 E */
#endif
#endif

