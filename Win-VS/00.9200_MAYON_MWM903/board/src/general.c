/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	general.c

Abstract:

   	The general routines for modules.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"

/******************************************************************************
* Function Prototype                                                          *
*******************************************************************************/

/******************************************************************************
* Function Body		                                                      *
*******************************************************************************/
/* hex to Bin */

u8* hex2bin(char ch)
{
    static u8 bin[5]={0}, error[]="####";
    static u8 mask[4]={8,4,2,1};
    u8 i;
    if ((ch>='0' && ch<='9') ||
        (ch>='A' && ch<='F') || (ch>='a' && ch<='f'))
    {
        (ch -= '0')>10 ? ch -= 7 : 0;
        for (i=0 ; i<4 ; i++)
            bin[i] = ch & mask[i]? '1' : '0';
        return bin;
    }
    else return error;
}

void PrintBin (u32 hex)
{
    char *result;
    char bin[4];  /* 4* 8  = 32 bit*/
    u8 i;
    for (i= 0; i<8;i++)
    {
        sprintf((char*)bin,"%x",hex & (0xF0000000>>i*4));    
		strtok(bin,"0");
        result = (char *)hex2bin(bin[0]);
        DEBUG_BOARD ("%s",result);
    }
    DEBUG_BOARD ("\r\n");
}

/*Hex to bin*/

/*

Routine Description:
	
	Byte swap of half word.
	
Arguments:

	data - Half word to be byte swapped.

Return Value:

	Half word of byte swapped.

*/
u16 bSwap16(u16 data)
{
	return (((data & 0xff00) >> 8 ) | ((data & 0xff) << 8));
	
}	

/*

Routine Description:
	
	Byte swap of word.
	
Arguments:

	data - Word to be byte swapped.

Return Value:

	Word of byte swapped.

*/
u32 bSwap32(u32 data)
{
	return (((data & 0x000000ff) << 24) | ((data & 0x0000ff00) << 8) | ((data & 0x00ff0000) >> 8) | ((data & 0xff000000) >> 24));
}	

/*

Routine Description:
	
	Byte swap of double word.
	
Arguments:

	data - Double word to be byte swapped.

Return Value:

	Double word of byte swapped.

*/


u64 bSwap64(u64 data)
{
	u32 temp;

	temp = data.hi;
	data.hi = bSwap32(data.lo);
	data.lo = bSwap32(temp);
	
	return (data);
}
