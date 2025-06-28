/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	dmaapi.h

Abstract:

   	The application interface of DMA.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __DMA_API_H__
#define __DMA_API_H__



/* Type definition */

typedef struct _DMA_CFG {
	u32		src;
	u32     dst;
	u32		cnt;
	u32		burst; /*CY 0907*/
} DMA_CFG;

typedef struct _DMA_CFG_AUTO {
	u32		src;
	u32     dst;
    u32     src_stride;
    u32     dst_stride;
	u32		datacnt;
    u32     linecnt;
	u32		burst; /*CY 0907*/
} DMA_CFG_AUTO;
/* Function prototype */

extern void* memcpy_hw(void *dest, const void *src, unsigned int count);
extern void* memset_hw(void *dest, unsigned char dataVal, unsigned int count);
extern void *memset_hw_Word(void *ori_dest, unsigned int dataVal, int ori_count);

extern void *memBlkCpyAuto_hw(unsigned char *dest, unsigned char *src, 
                                        unsigned int stride_dst, unsigned int stride_src, 
                                        unsigned int datacnt, unsigned int linecnt);

#endif
