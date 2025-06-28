/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	init_stmem_nand.c

Abstract:

   	The initialization routines of static memory for nand gate flash boot.

	This file contains re-implementations of functions whose
	C library implementations rely on semihosting.  
	This includes I/O and clocking functionality.

	Defining USE_SERIAL_PORT targets the I/O
	to the Integrator AP serial port A.  Otherwise, I/O is targeted
	to the debugger console.  
 
	Defining USE_TARGET_CLOCK targets the clocking mechanism to the
	Integrator core module reference clock.  Otherwise, clocking is 
	timed off the host system clock.  

	In an image built to run on a standalone target USE_SERIAL_PORT
	USE_HOST_CLOCK must be defined.  

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"
#include "uartapi.h"

#define TRUE 1
#define FALSE 0
/*
** Importing __use_no_semihosting_swi ensures that our image doesn't link
** with any C Library code that makes direct use of semihosting. 
**
** Build with STANDALONE to include this symbol.
**
*/

#ifdef STANDALONE
//#ifdef 0
#define USE_SERIAL_PORT
#pragma import(__use_no_semihosting_swi)
#endif


/*
** Retargeted I/O
** ==============
** The following C library functions make use of semihosting
** to read or write characters to the debugger console: fputc(),
** fgetc(), and _ttywrch().  They must be retargeted to write to
** the UART.  __backspace() must also be retargeted
** with this layer to enable scanf().  See the Compiler and
** Libraries Guide.
*/

/*
** These must be defined to avoid linking in stdio.o from the
** C Library
*/
struct __FILE { int handle;   /* Add whatever you need here */};
#if 1
FILE __stdout;
FILE __stdin;
#endif

#ifdef USE_SERIAL_PORT
/*
** __backspace must return the last char read to the stream
** fgetc() needs to keep a record of whether __backspace was
** called directly before it
*/
int last_char_read;
int backspace_called;

/*

Routine Description:
	
	Retarget the fputc function to UART.      
	
Arguments:

	ch - The character to put to the file.
	f - The file handle.

Return Value:

	None.

*/
int fputc(int ch, FILE *f)
{
	unsigned char tempch = ch;

	sendchar(DEBUG_UART_ID, &tempch);

	return ch;
}

/*

Routine Description:
	
	Retarget the fgetc function to UART.      
	
Arguments:

	f - The file handle.

Return Value:

	The character got from file.

*/
int fgetc(FILE *f)
{
	unsigned char tempch;

	/* if we just backspaced, then return the backspaced character */
	/* otherwise output the next character in the stream */
	if (backspace_called == TRUE)
	{
		backspace_called = FALSE;
		return last_char_read;
	}

	tempch = receive_char(DEBUG_UART_ID);
	if (tempch == '\r')
		tempch = '\n';
	//sendchar(DEBUG_UART_ID, &tempch);		    /* echo the charater just received */
	last_char_read = (int)tempch;       /* backspace must return this value */
	
	return tempch;
}

/*

Routine Description:
	
	Retarget the __ttywrch function to UART.      
	
Arguments:

	ch - The character to write to TTY.

Return Value:

	None.

*/
void _ttywrch(int ch)
{
	unsigned char tempch = ch;

	sendchar(DEBUG_UART_ID, &tempch);
}

/*

Routine Description:
	
	The effect of __backspace() should be to return the last character
	read from the stream, such that a subsequent fgetc() will
	return the same character again.     
	
Arguments:

	f - The file handle.

Return Value:

	The last character read?

*/
int __backspace(FILE *f)
{
	backspace_called = TRUE;
	
	return 1;
}

/* END of Retargeted I/O */
#endif        //end of '#ifdef USE_SERIAL_PORT'

/*
** Exception Signaling and Handling
** ================================
** The C library implementations of ferror() uses semihosting directly
** and must therefore be retargeted.  This is a minimal reimplementation.  
** _sys_exit() is called after the user's main() function has exited.  The C library
** implementation uses semihosting to report to the debugger that the application has
** finished executing.  
*/


#ifdef STANDALONE

/*

Routine Description:
	
	Retarget the ferror function. 
	
	This is a minimal re-implementation.      
	
Arguments:

	f - The file handle.

Return Value:

	The constant of EOF (End Of File).

*/
int ferror(FILE *f)
{
	return EOF;
}

/*

Routine Description:
	
	Retarget the _sys_exit function.
	
	Just enter an infinite loop      
	
Arguments:

	return_code - The return code of system exit.

Return Value:

	None.

*/
void _sys_exit(int return_code)
{
	while(1);
}

#endif        //end of '#ifdef STANDALONE'
