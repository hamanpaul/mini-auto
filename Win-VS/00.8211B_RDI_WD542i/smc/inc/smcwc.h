/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcwc.h

Abstract:

   	The structures and constants of write cache SMC and NAND gate flash.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SMC_WC_H__
#define __SMC_WC_H__

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))


/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/
//civic 070907 S
//#if (STORAGE_MEMORY_OPTION == STORAGE_MEMORY_SMC_NAND)

//#define SMC_USE_LB_WRITE_CACHE		1

//#else
#include "smcapi.h"
#define SMC_USE_LB_WRITE_CACHE		1

//#endif
//civic 070907 E
/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/

#if SMC_USE_LB_WRITE_CACHE



/* SMC write cache */
//typedef struct _SMC_WRITE_CACHE
//{
//unsigned long 	blockAddr;
//unsigned char	blockData[SMC_MAX_PAGE_PER_BLOCK][SMC_MAX_PAGE_SIZE];
//} SMC_WRITE_CACHE;

#endif

/*****************************************************************************/
/* External Variable			                                     */
/*****************************************************************************/

#if SMC_USE_LB_WRITE_CACHE
extern unsigned char	* smcMBRCache;
//extern unsigned char	*smcReadBuf;
extern u8	smcReadBuf[];
extern unsigned char smcWriteCache[SMC_MAX_PAGE_PER_BLOCK][SMC_MAX_PAGE_SIZE];
extern unsigned char	 smcBitMap[SMC_MAX_MAP_SIZE_IN_BYTE];
#endif


/*****************************************************************************/
/* Function Prototype			                                     */
/*****************************************************************************/

//signed long  smcCacheClear(void);
//signed long  smcCacheBlockRead(unsigned long);
//signed long  smcCachePageWrite(unsigned long, unsigned char*);
//signed long  smcCacheBlockWrite(void);

#endif /* __SMC_WC_H__ */
#endif
