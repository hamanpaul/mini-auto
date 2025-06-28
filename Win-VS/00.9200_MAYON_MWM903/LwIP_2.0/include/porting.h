/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _OS_PORTING_H_
#define _OS_PORTING_H_

#include <general.h>
//#include "os.h"
#include <UCOS_II.H>
                          
/*============Memory========================*/
OS_APIs void *OS_MemAlloc( u32 size );
OS_APIs void __OS_MemFree( void *m );
OS_APIs void OS_MemSET(void *pdest, u8 byte, u32 size);
OS_APIs void OS_MemCPY(void *pdest, const void *psrc, u32 size);
OS_APIs void DispLwipBufStat(void);
/*=========================================*/

#endif
