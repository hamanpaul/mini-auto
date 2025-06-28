/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	debug.c

Abstract:

   	The routines of debug loop.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"

#include "task.h"

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */
 
/* define debug print */
#define debugDebugPrint 			printf 

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
  
OS_STK debugTaskStack[DEBUG_TASK_STACK_SIZE];	/* debug task stack */
 
s8 debugHexChar[] = "0123456789abcdef";

/*
 *********************************************************************************************************
 *                                 		FUNCTION PROTOTYPE
 *********************************************************************************************************
 */
void debugTask(void*); 
void debugUsage(void); 
s8* debugGetToken(s8*, s8*, s32*);
s32 debugStrToInt(s8*, s32, u32*);
s32 debugIntToStr(u32, s8*, s32*);
s32 debugDumpMemory(u32, u32);
s32 debugFillMemory(u32, u32, u32);
s32 debugGetString(s8*);

/*
 *********************************************************************************************************
 *                                      	FUNCTION BODY 	
 *********************************************************************************************************
 */
 
/*

Routine Description:

	Initialize the debug module.

Arguments:

	None.

Return Value:

	None.

*/ 
void debugInit(void)
{   
	/* Create the task */
	//debugDebugPrint("Trace: Debug task creating.\n");
    	OSTaskCreate(DEBUG_TASK, DEBUG_TASK_PARAMETER, DEBUG_TASK_STACK, DEBUG_TASK_PRIORITY); 
}

/*

Routine Description:

	The test routine of debug module.

Arguments:

	None.

Return Value:

	None.

*/
void debugTest(void)
{
	debugInit();
}

/*

Routine Description:

	The task routine of the debug module.

Arguments:

	pData - The parameter of the task.

Return Value:

	None.

*/
void debugTask(void* pData)
{
	s8 string[80];
	s8 token[80];
	s8* pString;
	s32 count;
	u32 address, data, length;
	s32 i;
		
	while (1)
	{
		address = data = length = 0;
		printf("debug> ");
		debugGetString(string);
		pString = string;
		pString = debugGetToken(pString, token, &count);
		switch (*token)
		{
			case '?':	/* usage */
				debugUsage();
				break;
				
			case 'r':	/* read address */
				pString = debugGetToken(pString, token, &count);
				if (debugStrToInt(token, count, &address))
					debugDumpMemory(1, address);
				else
					printf("invalid address\n");	
				break;
			
			case 'w':	/* write address data */
				pString = debugGetToken(pString, token, &count);
				if (debugStrToInt(token, count, &address))
				{
					pString = debugGetToken(pString, token, &count);
					if (debugStrToInt(token, count, &data))
						debugFillMemory(1, address, data);
					else
						printf("invalid data\n");	
				}
				else
					printf("invalid address\n");	
				break;
				
			case 'd':	/* dump length address */
				pString = debugGetToken(pString, token, &count);
				if (debugStrToInt(token, count, &length))
				{
					pString = debugGetToken(pString, token, &count);
					if (debugStrToInt(token, count, &address))
						debugDumpMemory(length, address);
					else
						printf("invalid address\n");	
				}
				else
					printf("invalid length\n");	
				break;
				
			case 'f':	/* fill length address data */
				pString = debugGetToken(pString, token, &count);
				if (debugStrToInt(token, count, &length))
				{
					pString = debugGetToken(pString, token, &count);
					if (debugStrToInt(token, count, &address))
					{
						pString = debugGetToken(pString, token, &count);
						if (debugStrToInt(token, count, &data))
							debugFillMemory(length, address, data);
						else
							printf("invalid data\n");
					}
					else
						printf("invalid address\n");	
				}
				else
					printf("invalid length\n");
				break;

			case 'm':	/* mwrite length address data ... */
				pString = debugGetToken(pString, token, &count);
				if (debugStrToInt(token, count, &length))
				{
					pString = debugGetToken(pString, token, &count);
					if (debugStrToInt(token, count, &address))
					{
						for (i = 0; i < length; i++)
						{
							pString = debugGetToken(pString, token, &count);
							if (debugStrToInt(token, count, &data))
							{
								debugFillMemory(1, address, data);
								address += 4;
							}
							else
							{
								printf("invalid data\n");
								break;
							}
						}		
					}
					else
						printf("invalid address\n");	
				}
				else
					printf("invalid length\n");
				break;
									
			default:
				printf("invalid command\n");
		}					
	}
}

/*

Routine Description:

	The usage of the debug module.

Arguments:

	None.

Return Value:

	None.

*/
void debugUsage(void)
{
	printf("Usage:\n");
	printf("  r[ead] address\n"); 
	printf("  w[rite] address data\n"); 
	printf("  d[ump] length address\n"); 
	printf("  f[ill] length address data\n");
	printf("  m[write] length address data ...\n"); 
}

/*

Routine Description:

	Get the first token of the string.

Arguments:

	str - The string to get the token.
	tkn - The first token got from the string.
	cnt - The character count of the token.

Return Value:

	The remainning string after the first token.

*/
s8* debugGetToken(s8* str, s8* tkn, s32* cnt)
{
	s32 cntr = 0;
	
	while ((*str != '\0') && (*str == ' '))
		str++;
	
	while ((*str != '\0') && (*str != ' '))
	{
		*tkn++ = *str++;
		cntr++;	
	}
	
	*tkn = '\0';
	*cnt = cntr;

	return str;	
}		

/*

Routine Description:

	Convert a token to an integer.

Arguments:

	tkn - The token.
	cnt - The character count of the token.
	val - The integer value converted.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 debugStrToInt(s8* tkn, s32 cnt, u32* val)
{ 
	s32 i;
	s8 ch;
	s32 v = 0;
	
	if (cnt == 0)
		return 0;
	
	for (i = 0; i < cnt; i++)
	{
		ch = *tkn++;
		if ((ch >= '0') && (ch <= '9'))
			ch = (int) ch - '0';
		else if ((ch >= 'A') && (ch <= 'F'))
			ch = (int) ch - 'A' + 0xa;
		else if ((ch >= 'a') && (ch <= 'f'))
			ch = (int) ch - 'a' + 0xa;
		else
			return 0;	
		
		v = (v << 4) + (int) ch;
	}	

	*val = v;
	
	return 1;
}

/*

Routine Description:

	Convert an integer to a token.

Arguments:

	val - The integer value to be converted.
	tkn - The token.
	cnt - The character count of the token.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 debugIntToStr(u32 val, s8* tkn, s32* cnt)
{ 
	s32 i;
	s8 dig[8];
	
	for (i = 0; i < 8; i++)
	{
		dig[i] = debugHexChar[val % 16];
		val >>= 4;
	}	
	
	*cnt = i;
	
	for (i = 7; i >= 0; i--)
		*tkn++ = dig[i];

	*tkn = '\0';
	
	return 1;
}

/*

Routine Description:

	Dump a memory range.

Arguments:

	len - The number of words to be dumped.
	addr - The start address to be dumped.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 debugDumpMemory(u32 len, u32 addr)
{ 
	s32 i;
	u32 dat;
	s8 tok[80];
	s32 cnt;
	
	for (i = 0; i < len; i++)
	{
		if ((i % 4) == 0)
		{
			if (i != 0)
				printf("\n");
			
			debugIntToStr(addr, tok, &cnt);	
			printf("%s: ", tok);	
		}
						
		dat = *((volatile int *) addr);
		debugIntToStr(dat, tok, &cnt);	
		printf("%s ", tok);	
		addr += 4;
	}					
	printf("\n");
	
	return 1;	
}

/*

Routine Description:

	Fill a memory range with a wor of fixed data pattern.

Arguments:

	len - The number of words to be filled.
	addr - The start address to be filled.
	dat - The data pattern to be filled.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 debugFillMemory(u32 len, u32 addr, u32 dat)
{					
	s32 i;
	
	for (i = 0; i < len; i++)
	{
		*((volatile int *) addr) = dat;
		addr += 4;
	}
	
	return 1;
}		

/*

Routine Description:

	Get a string.

Arguments:

	str - The string.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 debugGetString(s8* str)
{ 
	s8  ch;
	u32 i = 0;

	do
	{		
	        ch = getchar();
	        str[i++] = ch;
	} while ((ch != '\n') && (ch != '\r'));
	str[--i] = '\0';
	
	return 1;
}
