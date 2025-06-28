/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    adc.c

Abstract:

    The routines of ADC.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#include "general.h"
#include "board.h"
#include "adcreg.h"
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


/*

Routine Description:

    The test routine of ADC.

Arguments:

    None.

Return Value:

    None.

*/
void adcTest(void)
{
    
}

void init_DAC_play(u8 start)
{
}

/*
Routine Description:

	Set DAC output level zero.

Arguments:

	None.

Return Value:

	None.
*/
void adcDacOutputZero(void)
{	
	DacTxCtrl = DAC_PWOFF;
}


