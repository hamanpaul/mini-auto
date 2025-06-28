/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	init_stmem_ram.c

Abstract:

   	The initialization routines of static memory for RAM boot.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"
#include "board.h"

/*

Routine Description:
	
	Initialize Shadow Register in Static Memory Control Register for RAM boot.

Arguments:

	None.

Return Value:

	None.

*/
void stmemInit(void)
{
	ShadowReset 	= SdramBase + 0x00000000;
	ShadowUndef 	= SdramBase + 0x00000004;
	ShadowSwi	    = SdramBase + 0x00000008;
	ShadowPrefAbt	= SdramBase + 0x0000000c;
	ShadowDataAbt	= SdramBase + 0x00000010;
	ShadowRsv	    = SdramBase + 0x00000014;
	ShadowIrq	    = SdramBase + 0x00000018;
	ShadowFiq	    = SdramBase + 0x0000001c;
	ShadowEnable	= 0x01; 

    *((volatile unsigned *)(StMemBase + 0x0000))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x0004))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x0008))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x000c))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x0010))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x0014))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x0018))=0xE59FF018;
    *((volatile unsigned *)(StMemBase + 0x001c))=0xE59FF018;

	*((volatile unsigned *)(StMemBase + 0x0020))=SdramBase + 0x00000000;
    *((volatile unsigned *)(StMemBase + 0x0024))=SdramBase + 0x00000004;
    *((volatile unsigned *)(StMemBase + 0x0028))=SdramBase + 0x00000008;
    *((volatile unsigned *)(StMemBase + 0x002c))=SdramBase + 0x0000000c;
    *((volatile unsigned *)(StMemBase + 0x0030))=SdramBase + 0x00000010;
    *((volatile unsigned *)(StMemBase + 0x0034))=SdramBase + 0x00000014;
    *((volatile unsigned *)(StMemBase + 0x0038))=SdramBase + 0x00000018;
    *((volatile unsigned *)(StMemBase + 0x003c))=SdramBase + 0x0000001c;
}


