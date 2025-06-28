/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mmu.h

Abstract:

   	MMU Create

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"

#ifdef MMU_SUPPORT

#ifdef MMU_SUPPORT
    #define WRITE_THROUGH
    #ifndef WRITE_THROUGH  //Lucian: Write back 不建議使用.
    #define WRITE_BACK
    #endif

    #define SECTION_PAGE
    #ifndef SECTION_PAGE   //Lucian: Coarse page 不建議使用.
    #define COARSE_PAGE
    #endif    
#endif
#define SZ_1K                           0x00000400
#define SZ_2K                           0x00000800
#define SZ_4K                           0x00001000
#define SZ_8K                           0x00002000
#define SZ_16K                          0x00004000
#define SZ_64K                          0x00010000
#define SZ_128K                         0x00020000
#define SZ_256K                         0x00040000
#define SZ_512K                         0x00080000

#define SZ_1M                           0x00100000
#define SZ_2M                           0x00200000
#define SZ_4M                           0x00400000
#define SZ_8M                           0x00800000
#define SZ_16M                          0x01000000
#define SZ_32M                          0x02000000
#define SZ_64M                          0x04000000
#define SZ_128M                         0x08000000
#define SZ_256M                         0x10000000
#define SZ_512M                         0x20000000

#define SZ_1G                           0x40000000
#define SZ_2G                           0x80000000
extern void flush_dcache(void);
extern void Test_Clean_Dcache();
extern u8 cehck_cache_by_address(u32);
#endif
