/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	SmcECCSW.h

Abstract:

   	The prototype functions declaration of NAND-Gate flash controller ECC S/W algorithm.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2006/11/15	Lori Chen	Create

*/


#include "general.h"



u32 smcRsCorrection(u8*);
u32	smcECC_SW(u32*);
u8	smcCPN_Process(u8, u8);
u8	smcOneBitXOR(u16, u16, u8);
u16 smcLPN_Process(u8, u16, u8);



