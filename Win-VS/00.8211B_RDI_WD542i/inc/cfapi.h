/*

Copyright (c) 2005  Himax Technologies, Inc.

Module Name:

	sdcapi.h

Abstract:

   	The application interface of the SecurityDisk controller

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __CF_API_H__
#define __CF_API_H__

#define VERIFYCF       0
#define BIGENDIAN      0   /* (VCC) 1: Big Endian  0: Little Endian */

#define REAL_CHIP      0 /* (VCC) 1: For Real-Chip  0: For FPGA board */
#define USE_INT        1 /* (VCC) 1:Use interrupt  0:Use polling. */

#define BURN_IN_TEST   0 /* (VCC) 1:FOR Burn-In Test  0: FOR Normal Test */

//#define USE_TX_INT     1 /* (VCC) 1:Use Tx_INT interrupt  0:Use polling. 
//                            Tx_INT interrupt was added in PA9001D */
// #define USE_MUL_TX_INT 1 /* (VCC) 1:Use Multiple Tx_INT interrupt*/

/* Constant */
#define CF_BLK_LEN	0x0200		/* CF block length */
// #define MMC_BLK_LEN	0x0200		/* MMC block length */

/* Function prototype */
#if 1
extern s32 cfInit(void);
extern s32 cfMount(void);
extern s32 cfUnmount(void);
extern void cfIntHandler(void);
extern s8 cfTest(void);


extern int ideDevStatus(u32); 
extern int ideDevRead(u32, u32, void*); 
extern int ideDevMulRead(u32, u32, u32, void*);
extern int ideDevWrite(u32, u32, void*); 
extern int ideDevMulWrite(u32, u32, u32, void *); 
extern int ideDevIoCtl(u32, s32, s32, void*);

#endif

#if VERIFYCF

#endif


u32 cfErrHandle(u32 ucErrorCode, u8 *ucOutputMessage);

#endif
