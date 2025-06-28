/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	MDreg.h

Abstract:

   	The declarations of Motion Detector Unit registers.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/11/03	Lucian Yuan	Create	

*/


/* MD_CTRL */  
#define MD_CTRL_TRIG    0x00000001
#define MD_CTRL_RST     0x00000002

#define MD_CTRL_DS1x1   0x00000020
#define MD_CTRL_DS2x2   0x00000000
#define MD_CTRL_DS4x4   0x00000010

/* MD_INT_ENA */  
#define MD_INTENA_ON    0x00000001
#define MD_INTENA_OFF   0x00000000

/* MD_INT_STA */              

