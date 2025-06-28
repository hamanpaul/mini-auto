/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	stmemapi.h

Abstract:

   	The application interface of the static memory controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/


#ifndef __STMEM_API_H__
#define __STMEM_API_H__

extern void stmemInit(void);
extern void stmemInit(void);
extern u8 stmInit(void);
extern u8 stmReset(void);
extern u8 stmMount(void);
extern u32 stmIdentification(void);
extern u8 stmReadSilicon(u16*, u16*);
extern u8 stmDataPolling(u8, u8, u8*);
extern u8 stmProgram(u32, u8*, u32);
extern u8 stmProgramTest(u32, u8*, u32);
extern u8 stmChipErase(void);
extern u8 stmChipEraseTest(u8, u8*);
extern u8 stmSectorErase(u32);
extern u8 stmSectorEraseTest(u8, u32, u32, u8*);
extern u8 stmVerify(void);
extern u8 stmBurnCode(void);
extern u8 Nor_Mount(void);
extern u8 Nor_burning(u32,u8*,u32);
#endif
