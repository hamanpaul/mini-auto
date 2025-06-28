/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	jpegreg.h

Abstract:

   	The registers of JPEG encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __JPEG_REG_H__
#define __JPEG_REG_H__

// Reg 00 
#define JPEG_ENC_DISA		0x00000000
#define JPEG_ENC_ENA		0x00000001

#define JPEG_DEC_DISA		0x00000000
#define JPEG_DEC_ENA		0x00000002

#define JPEG_DECBLK_TRG		    0x00000004
#define JPEG_DEBLK_MODE_DISA    0x00000000
#define JPEG_DEBLK_MODE_ENA     0x00000008

#define JPEG_IMG_422		0x00000000
#define JPEG_IMG_420		0x00000010
#define JPEG_IMG_440		0x00000040
#define JPEG_IMG_444		0x00000050
#define JPEG_IMG_SHFT		4

#define JPEG_FRAME_MODE		0x00000000
#define JPEG_SLICE_MODE		0x00000020  
#define JPEG_OPBUF_SHFT		5 

#define JPEG_MCU_NO_SHFT	16

// Reg 04
#define JPEG_IMAGE_SIZE_X_SHFT	0
#define JPEG_IMAGE_SIZE_Y_SHFT	16

// Reg 08
#define JPEG_DRI_EN	0x80000000/*BJ 0523 S*/

// Reg 20
#define JPEG_CMP_INT_ENA	0x00000001
#define JPEG_CMP_INT_DISA	0x00000000
#define JPEG_DCMP_INT_ENA	0x00000002
#define JPEG_DCMP_INT_DISA	0x00000000
#define JPEG_DCMP_ERR_INT_ENA	0x00000004
#define JPEG_DCMP_ERR_INT_DISA	0x00000000
#define JPEG_DCMP_BLK_INT_ENA	0x00000080
#define JPEG_DCMP_BLK_INT_DISA	0x00000000

// JPEG_BRI_CTRL 0xc014f000
#define JPEG_BRI_RST		        0x00000001
#define JPEG_BRI_MODE_ENC           0x00000000
#define JPEG_BRI_MODE_DEC           0x00000002
#define JPEG_BRI_DAT_TYPE_YUV422    0x00000000
#define JPEG_BRI_DAT_TYPE_YUV420    0x00000004
#define JPEG_BRI_DAT_FMT_YUV422     0x00000000
#define JPEG_BRI_DAT_FMT_YUV420     0x00000008
#define JPEG_BRI_DAT_FMT_YUV440     0x00000010
#define JPEG_BRI_DAT_FMT_YUV444     0x00000018


#endif
