/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	jpeg.h

Abstract:

   	The declarations of JPEG standard.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __JPEG_H__
#define __JPEG_H__

typedef struct _JPEG_SIZE {
	u16		width;
	u16		height;
} JPEG_SIZE;

/*BJ 0523 S*/
typedef struct {
	unsigned min;
    unsigned diff;   
} Huffman_table;
/*BJ 0523 E*/

#endif
