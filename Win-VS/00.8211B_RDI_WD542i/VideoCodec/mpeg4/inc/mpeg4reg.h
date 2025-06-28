/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mpeg4reg.h

Abstract:

   	The registers of MPEG-4 encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __MPEG4_REG_H__
#define __MPEG4_REG_H__

/* constant */
#define mbWidthShft	0
#define mbHeightShft	8
#define mbNoShft	16


#define MPEG4_RESY_DIS   0x00010000
#define MPEG4_RESY_ENA   0x00000000
#define MPEG4_IMBREF_EN	 0x01000000
#define MPEG4_IMBREF_DIS 0x00000000

#define MPEG4_ENC_TRG    0x00000001
#define MPEG4_DEC_TRG    0x00000002
#define MPEG4_QM_H263    0x00000000
#define MPEG4_QM_MPEG    0x00000004
#define MPEG4_VDI_YC     0x00000000
#define MPEG4_VDI_YUV	 0x00000008
#define MPEG4_DB_FIELD   0x00000010
#define MPEG4_DBG_ENA	 0x00000020
#define	MPEG4_DBG_DISA	 0x00000000
#define	MPEG4_DS_ENA	 0x00010000
#define	MPEG4_DS_DISA	 0x00000000
#define	MPEG4_DS_TOP	 0x00000000
#define	MPEG4_DS_BOT	 0x00040000

#define MPEG4_ROT_ENA    0x10000000
#define MPEG4_ROT_DISA   0x00000000
#define MPEG4_ROT_90     0x20000000
#define MPEG4_ROT_270    0x00000000
#define MPEG4_ROTDS_ENA  0x40000000
#define MPEG4_ROTDS_DISA 0x00000000


//-------------------//
#define mpeg4IMbRefYpos_Shft	8
#define mpeg4RefSliceSize_Shft	16


#endif
