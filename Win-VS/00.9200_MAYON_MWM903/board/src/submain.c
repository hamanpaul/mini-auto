/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	submain.c

Abstract:

   	The routine of $Sub$$main().
   	
	$Sub$$main() is executed immediately before the user defined main() function.  
	Compile this with -DUSE_SERIAL_PORT to initialize serial port on Integrator.
	For information on $Sub and $Super, see the Linker and Utilities Guide

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

extern void $Super$$main(void);
extern void init_serial_A(void);

/*

Routine Description:
	
	$Sub$$main() called before $Super$$main() == main().      
	
Arguments:

	None.

Return Value:

	None.

*/
void $Sub$$main(void)
{
    init_serial_A();                 // initialize the AP serial port A

    $Super$$main();                  // calls the original function main()
}

