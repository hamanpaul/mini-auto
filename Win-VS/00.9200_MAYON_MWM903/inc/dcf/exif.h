/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	exif.h

Abstract:

   	The declarations of EXIF.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __EXIF_H__
#define __EXIF_H__

/* Cosntant */
/* Little Endian (ARM) -> Big Endian (JFIF) */
/* Start Of Frame markers, non-differential, Huffman coding */
#define EXIF_MARKER_SOF0	0xc0ff
#define EXIF_MARKER_SOF1	0xc1ff
#define EXIF_MARKER_SOF2	0xc2ff
#define EXIF_MARKER_SOF3	0xc3ff

/* Start Of Frame markers, differential, Huffman coding */
#define EXIF_MARKER_SOF5	0xc5ff
#define EXIF_MARKER_SOF6	0xc6ff
#define EXIF_MARKER_SOF7	0xc7ff

/* Start Of Frame markers, non-differential, arithmetic coding */
#define EXIF_MARKER_JPG		0xc8ff
#define EXIF_MARKER_SOF9	0xc9ff
#define EXIF_MARKER_SOF10	0xcaff
#define EXIF_MARKER_SOF11	0xcbff

/* Start Of Frame markers, differential, arithmetic coding */
#define EXIF_MARKER_SOF13	0xcdff
#define EXIF_MARKER_SOF14	0xceff
#define EXIF_MARKER_SOF15	0xcfff

/* Huffman table specification */
#define EXIF_MARKER_DHT		0xc4ff

/* Arithmetic coding conditioning specification */
#define EXIF_MARKER_DAC		0xccff

/* Restart interval termination */
#define EXIF_MARKER_RST0	0xd0ff
#define EXIF_MARKER_RST1	0xd1ff
#define EXIF_MARKER_RST2	0xd2ff
#define EXIF_MARKER_RST3	0xd3ff
#define EXIF_MARKER_RST4	0xd4ff
#define EXIF_MARKER_RST5	0xd5ff
#define EXIF_MARKER_RST6	0xd6ff
#define EXIF_MARKER_RST7	0xd7ff

/* Other markers */
#define EXIF_MARKER_SOI		0xd8ff
#define EXIF_MARKER_EOI		0xd9ff
#define EXIF_MARKER_SOS		0xdaff
#define EXIF_MARKER_DQT		0xdbff
#define EXIF_MARKER_DNL		0xdcff
#define EXIF_MARKER_DRI		0xddff
#define EXIF_MARKER_DHP		0xdeff
#define EXIF_MARKER_EXP		0xdfff
#define EXIF_MARKER_APP0	0xe0ff
#define EXIF_MARKER_APP1	0xe1ff
#define EXIF_MARKER_APP2	0xe2ff
#define EXIF_MARKER_APP3	0xe3ff
#define EXIF_MARKER_APP4	0xe4ff
#define EXIF_MARKER_APP5	0xe5ff
#define EXIF_MARKER_APP6	0xe6ff
#define EXIF_MARKER_APP7	0xe7ff
#define EXIF_MARKER_APP8	0xe8ff
#define EXIF_MARKER_APP9	0xe9ff
#define EXIF_MARKER_APP10	0xeaff
#define EXIF_MARKER_APP11	0xebff
#define EXIF_MARKER_APP12	0xecff
#define EXIF_MARKER_APP13	0xedff
#define EXIF_MARKER_APP14	0xeeff
#define EXIF_MARKER_APP15	0xefff
#define EXIF_MARKER_JPG0	0xf0ff
#define EXIF_MARKER_JPG1	0xf1ff
#define EXIF_MARKER_JPG2	0xf2ff
#define EXIF_MARKER_JPG3	0xf3ff
#define EXIF_MARKER_JPG4	0xf4ff
#define EXIF_MARKER_JPG5	0xf5ff
#define EXIF_MARKER_JPG6	0xf6ff
#define EXIF_MARKER_JPG7	0xf7ff
#define EXIF_MARKER_JPG8	0xf8ff
#define EXIF_MARKER_JPG9	0xf9ff
#define EXIF_MARKER_JPG10	0xfaff
#define EXIF_MARKER_JPG11	0xfbff
#define EXIF_MARKER_JPG12	0xfcff
#define EXIF_MARKER_JPG13	0xfdff
#define EXIF_MARKER_COM  	0xfeff

/* Reserved markers */
#define EXIF_MARKER_TEM  	0xff01
#define EXIF_MARKER_RES  	0xff02	/* 0xff02 - 0xffbf */

/* Type definition */

/* COM */
typedef __packed struct _EXIF_COM
{
	u16			COM;	/* bSwap16() */
	u16			Lc;	/* bSwap16() */
	u8			Cm[16];
	//u8          Data[19712];
} EXIF_COM;

/* DRI */
typedef __packed struct _EXIF_DRI
{
	u16			DRI;    /* bSwap16() */
	u16			Lr;	    /* bSwap16() */
	u16			Ri;	    /* bSwap16() */
} EXIF_DRI;

/* DQT */
typedef __packed struct _EXIF_DQT
{
	u16			DQT;	/* bSwap16() */
	u16			Lq;	/* bSwap16() */
	u8			lumPqTq;
	u8			lumQ[64];
	u8			chrPqTq;
	u8			chrQ[64];
} EXIF_DQT;

/* DHT */
typedef __packed struct _EXIF_DHT
{
	u16			DHT;	/* bSwap16() */
	u16			Lh;	/* bSwap16() */
	u8			lumDcTcTh;
	u8			lumDcL[16];
	u8			lumDcV[12];
	u8			lumAcTcTh;
	u8			lumAcL[16];
	u8			lumAcV[162];
	u8			chrDcTcTh;
	u8			chrDcL[16];
	u8			chrDcV[12];
	u8			chrAcTcTh;
	u8			chrAcL[16];
	u8			chrAcV[162];
} EXIF_DHT;

/* SOF0 */
typedef __packed struct _EXIF_SOF0
{
	u16			SOF0;	/* bSwap16() */
	u16			Lf;	/* bSwap16() */
	u8			P;
	u16			Y;	/* bSwap16() */
	u16			X;	/* bSwap16() */
	u8			Nf;
	u8			yC;
	u8			yHV;
	u8			yTq;
	u8			cbC;
	u8			cbHV;
	u8			cbTq;
	u8			crC;
	u8			crHV;
	u8			crTq;
} EXIF_SOF0;

/* SOS */
typedef __packed struct _EXIF_SOS
{
	u16			SOS;	/* bSwap16() */
	u16			Ls;	/* bSwap16() */
	u8			Ns;
	u8			yCs;
	u8			yTdTa;
	u8			cbCs;
	u8			cbTdTa;
	u8			crCs;
	u8			crTdTa;
	u8			Ss;
	u8			Se;
	u8			AhAl;	
} EXIF_SOS;

/* Thumbnail */
typedef __packed struct _EXIF_THUMBNAIL
{
	u16			SOI;	/* bSwap16() */
	EXIF_DQT		dqt;
	EXIF_DHT		dht;
	EXIF_SOF0		sof0;
	EXIF_SOS		sos;	
} EXIF_THUMBNAIL;


/* APP1 */
typedef __packed struct _EXIF_APP1
{
	u16			APP1;	/* bSwap16() */
	u16			Lp;	/* bSwap16() */
	u8			identifier[5];
	u8			pad;
	EXIF_TIFF_HEADER	tiffHeader;
	EXIF_IFD0		ifd0;
	EXIF_IFD0_VALUE		ifd0Value;
	EXIF_IFD0E		ifd0e;
	EXIF_IFD0E_VALUE	ifd0eValue;
	EXIF_IFD1		ifd1;
	EXIF_IFD1_VALUE		ifd1Value;	
	EXIF_THUMBNAIL		thumbnail;
} EXIF_APP1;

/* EXIF thumbnail image */
//#define EXIF_THUMBNAIL_MAX	(160*120)
typedef __packed struct _EXIF_THUMBNAIL_IMAGE
{
	u16			SOI;	/* bSwap16() */		/* SOI 					*/
	EXIF_APP1		app1;			     	/* APP1 				*/ 	
	//u8			bitStream[EXIF_THUMBNAIL_MAX];	/* 	thumbnail bitstream + eoiMarker */
	u8			*bitStream;	/* 	thumbnail bitstream + eoiMarker */
} EXIF_THUMBNAIL_IMAGE;	

/* Primary */
typedef __packed struct _EXIF_PRIMARY
{
	EXIF_COM		com;
#if JPEG_DRI_ENABLE
    EXIF_DRI        dri;
#endif
	EXIF_DQT		dqt;
	EXIF_DHT		dht;
	EXIF_SOF0		sof0;
	EXIF_SOS		sos;	
} EXIF_PRIMARY;

#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
typedef __packed struct _EXIF_APP0
{
	u16			APP0;	/* bSwap16() */
	u16			Length;	    /* bSwap16() */
	u8			identifier[5];
	u8			version[2];
	u8          DensityUnit;
  	u16         X_Density;
   	u16         Y_Density;
    u8          ThuWidth;
    u8          ThuHeight; 
    u8          ThuData; 
} EXIF_APP0;
typedef __packed struct _EXIF_MJPG  //same as EXIF_PRIMARY
{
#if MJPG_APP0_ENABLE    
    EXIF_APP0       app0;
#endif    
	EXIF_COM		com;
#if MJPG_DRI_ENABLE    
    EXIF_DRI        dri;
#endif
	EXIF_DQT		dqt;
    EXIF_DHT		dht;
	EXIF_SOF0		sof0;
	EXIF_SOS		sos;	
} EXIF_MJPG;
#endif

/* EXIF primary image */
//#define EXIF_PRIMARY_MAX	(1600*1200)
typedef __packed struct _EXIF_PRIMARY_IMAGE
{
	EXIF_PRIMARY		primary;			/* primary				*/	
	//u8			bitStream[EXIF_PRIMARY_MAX];	/* primary bitstream + eoiMarker	*/
	u8			*bitStream;	/* primary bitstream + eoiMarker	*/
} EXIF_PRIMARY_IMAGE;

#endif

