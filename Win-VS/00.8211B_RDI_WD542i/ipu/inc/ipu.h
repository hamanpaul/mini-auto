/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ipu.h

Abstract:

   	The declarations of Image Processing Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IPU_H__
#define __IPU_H__

/* Type definition */

typedef struct _IPU_SIZE {
	u16		w;
	u16 		h;
} IPU_SIZE;

typedef struct _IPU_CFAI_THRESH {
	u8		edgeGThresh;
	u8 		edgeSmoothHue;
} IPU_CFAI_THRESH;

typedef struct _IPU_EDGE_ENHANCE {
	u8		threshL;
	u8		curvSlop1x32;
	u8		addOffs1;
	u8		threshCornr1;
	u8		curvSlop2x32;
	u8		addOffs2;
	u8		threshCornr2;
	u8		curvSlop3x32;
	u8		threshH;
} IPU_EDGE_ENHANCE;

typedef struct _IPU_LUM_GAMMA {
	u8		x;
	u8		y;
} IPU_LUM_GAMMA;

typedef struct _IPU_FALSE_COLOR_SUPPR {
	u8		edgeThresh;
	u16		decSlopex256;
	u32		enable;
} IPU_FALSE_COLOR_SUPPR;

typedef struct _IPU_HUE_SATUR {
	u8		hueSin;
	u8		hueCos;
	u8		satur;
	u32		enable;
} IPU_HUE_SATUR;

typedef struct  _IPU_DE_NOISE {
	u16		noisedeg;
	u8		diff_thd;
	u8		diff_cnt;
	u8		adf_diffthd;
	u8		adf_diff_cnt;
} IPU_DE_NOISE;

typedef struct  _IPU_YSUM_RPT {
	IPU_SIZE	Ysum_str;
	IPU_SIZE	Ysum_size;
	u8		Ysum_scale;
	u8		Yhtgl_thd;
	u8		Yhtgh_thd; 
} IPU_YSUM_RPT;

typedef struct _IPU_AF_RPT{
	IPU_SIZE AF_str;
	IPU_SIZE AF_size;
	u8		 AF_scale;
	u8		 AF_edgethd;
	u8		 AF_highbond;
} IPU_AF_RPT;

#endif
