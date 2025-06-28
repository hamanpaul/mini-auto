/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ispapi.h

Abstract:

   	The application interface of ISP.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

/*CY 1023*/

#ifndef __ISP_API_H__
#define __ISP_API_H__

/* Constant */
#define ISP_UPDATE_VARIABLE	0
#define ISP_UPDATE_CODE		1

/* Function prototype */
extern s32 ispUpdatebootload(void);
extern s32 ispUpdateAllload(void);
extern s32 ispUpdate(u8);
extern s32 ispUpdateAllload_Net(u32 codeSize);

#if (FLASH_OPTION == FLASH_NAND_9002_ADV)
#define ISP_BUFFER_SIZE		2048
#else
#define ISP_BUFFER_SIZE		512
#endif

extern u8 ispParam[ISP_BUFFER_SIZE];
extern s8 ispUSBFileName[32];
#endif
