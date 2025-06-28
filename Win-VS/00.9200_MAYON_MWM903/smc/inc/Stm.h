/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	stm.h

Abstract:

   	The structures and constants of Static Memory or NOR gate flash protocol.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2006/11/17	Lori Chen	Create

*/

#ifndef __STM_H__
#define __STM_H__

/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/



/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/
#define	FUNC_PROG		0
#define	FUNC_ERASE		1

/*****************************************
* Functions Prototype Declaration
*****************************************/
#if 0
void stmemInit(void);
u8 stmInit(void);
u8 stmReset(void);
u8 stmMount(void);
u32 stmIdentification(void);
u8 stmReadSilicon(u16*, u16*);
u8 stmDataPolling(u8, u8, u8*);
u8 stmProgram(u32, u8*, u32);
u8 stmProgramTest(u32, u8*, u32);
u8 stmChipErase(void);
u8 stmChipEraseTest(u8);
u8 stmSectorErase(u32);
u8 stmSectorEraseTest(u8, u32);
u8 stmVerify(void);
#endif


/* Sector Address Mapping Table */
enum
{
    stmSecSA0 = 0x0,
    stmSecSA1,
    stmSecSA2,
    stmSecSA3,
    stmSecSA4,
    stmSecSA5,
    stmSecSA6,
    stmSecSA7,
    stmSecSA8,
    stmSecSA9,
    stmSecSA10,
    stmSecSA11,
    stmSecSA12,
    stmSecSA13,
    stmSecSA14,
    stmSecSA15,
    stmSecSA16,
    stmSecSA17,
    stmSecSA18,
    stmSecSA19,
    stmSecSA20,
    stmSecSA21,
    stmSecSA22,
    stmSecSA23,
    stmSecSA24,
    stmSecSA25,
    stmSecSA26,
    stmSecSA27,
    stmSecSA28,
    stmSecSA29,
    stmSecSA30,
    stmSecSA31,
    stmSecSA32,
    stmSecSA33,
    stmSecSA34,
};

#endif
