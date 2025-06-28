/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcwc.c

Abstract:

   	The routines of SMC and NAND gate flash related to write cache.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"
#include "smc.h"
#include "smcreg.h"
#include "smcwc.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

#if SMC_USE_LB_WRITE_CACHE

u8	smcWriteCache[SMC_MAX_PAGE_PER_BLOCK][SMC_MAX_PAGE_SIZE];

//u8	*smcMBRCache;		/* allocate space in memorypool.c */
//u8	*smcReadBuf;


extern u32 smcPageSize;
extern u32 smcBlockSize;
extern u8	smcReadBuf[];

#endif

/*
 *********************************************************************************************************
 * Function Prototype
 *********************************************************************************************************
 */

/* Application Function */
//extern s32 smcMakeTable(void);

//extern s32 smcSectorsRead(u32, u32, u8*);
//extern s32 smcSectorsWrite(u32, u32, u8*);

/* Middleware Function */
extern s32 smcIdentification(void);
extern s32 smcReset(void);
extern s32 smcReadStatus(u8*);
extern s32 smcIdRead(u8*, u8*);
extern s32 smcPage1ARead(u32, u8*);
extern s32 smcPage1BRead(u32, u8*);
extern s32 smcPage2CRead(u32);
extern s32 smcPageProgram(u32, u8*, u16);
extern s32 smcBlockErase(u32);
extern s32 smcMultiBlockErase(u32, u32);

/*
 *********************************************************************************************************
 * Function Body
 *********************************************************************************************************
 */


