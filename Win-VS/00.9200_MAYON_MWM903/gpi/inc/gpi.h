/*

Copyright (c) 2010  Himax Technologies, Inc.

Module Name:

    gpi.h

Abstract:

    The structures and constants of GPI.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/08/02  Raymond Creates             

*/

#ifndef __GPI_H__
#define __GPI_H__

/* Error codes definitions */                           
#define GPI_OK  1

/*****************************************************************************/
/* Structure Definition                                              */
/*****************************************************************************/



/*****************************************************************************/
/* Constant Definition                                               */
/*****************************************************************************/
void GPIReset(void);
void GPIIntHandler(void);
/*
 * Command Index for GPI
 */
 


#endif
