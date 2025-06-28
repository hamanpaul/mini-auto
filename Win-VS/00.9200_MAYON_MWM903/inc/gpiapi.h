/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpiapi.h

Abstract:

   	The application interface of the I2C controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __GPI_API_H__
#define __GPI_API_H__

#define FLAGGPI_LWIP_IP_READY   0x00000001
#define FLAGGPI_LWIP_IP_READY_2 0x00000002
#define FLAGGPI_TUTK_READY      0x00000004

extern OS_FLAG_GRP  *gpiNetStatusFlagGrp;

#endif
