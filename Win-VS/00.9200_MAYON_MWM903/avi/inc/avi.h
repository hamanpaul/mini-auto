/*

Copyright (c) 2008   2008 Mars Semiconductor Corp.


Module Name:

	avi.h

Abstract:

   	The declarations of AVI file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2006/06/01	Peter Hsu Create	

*/

#ifndef __AVI_H__
#define __AVI_H__

/* type definition */

/***************************************************************************
 *
 * Object definition of AVI file
 *
 ***************************************************************************/

/***************************************************************************
 * General
 ***************************************************************************/

#define AVI_AUDIO /*Peter 1109 S*/
#define AVIEXTINFOLENS 32

typedef struct
{
   s32 kid;
   s32 Flags;
   s32 ChunkOffset;
   s32 ChunkLength;
} AVI_INDEXENTRY;

typedef struct
{
  u32 ID;
  u32 Size;
  u32 Type;
} WAVRIFFCHK;
//
typedef struct
{
  u32 ID;
  u32 Size;
  s16 FormatTag;
  u16 Channels;
  u32 SamplesPerSec;
  u32 AvgBytesPerSec;
  u16 BlockAlign;
  u16 BitsPerSamples;
} WAVFORMATCHK;
//
typedef struct
{
  u32 ID;
  u32 Size;
} WAVDATACHK;

/***************************************************************************
 * Entry definition
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
/* Header object							   */	
/*-------------------------------------------------------------------------*/
typedef struct
{
   s32 ID;
   s32 fileSize;
   s32 fileType;
}RIFFINFO;
//
typedef struct
{
   s32 listID;
   s32 listSize;
   s32 listType;
}LISTINFO;
//
typedef struct
{
   s32 ckID;
   s32 ckSize;
}CHUNKINFO;
//

//
typedef struct
{
   s32 MicroSecPerFrame;
   s32 MaxBytePerSec;
   s32 Reverved1;
   s32 Flags;
   s32 TotalFrames;
   s32 InitialFrames;
   s32 Streams;
   s32 SuggestedBufferSize;
   s32 Width;
   s32 Height;
   s32 Scale;
   s32 Rate;
   s32 Start;
   s32 Length;
} MAINAVIHEADER;
//
typedef struct
{
  s32 Type;
  s32 Handler;
  s32 Flags;
  s32 Reserved1;
  s32 InitialFrames;
  s32 Scale;
  s32 Rate;
  s32 Start;
  s32 Length;
  s32 SuggestedBufferSize;
  s32 Quality;
  s32 SampleSize;
  s32 Rec_UpLeft;
  s32 Rec_DownRight;
} AVISTREAMHEADER;
//
typedef struct
{
   s32 Size;
   s32 Width;
   s32 Height;
   s16 Planes;
   s16 BitCount;
   s32 Compression;
   s32 SizeImage;
   s32 XPelsPerMeter;
   s32 YPelsPerMeter;
   s32 ClrUsed;
   s32 ClrImportant; 
} DIBINFO;

typedef struct
{
  s16 FormatTag;
  s16 Channel;
  s32 SamplesPerSec;
  s32 AvgBytesPerSec;
  s16 BlockAlign;
  s16 BitsPerSample;
} WAVINFO;
//
typedef struct 
{
    s16 FormatTag;
    s16 Channel;
    s32   SamplesPerSec;
    s32   AvgBytesPerSec;
    s16 BlockAlign;
    s16 BitsPerSample;
    s16 SamplesPerBlock;
} DVIADPCMWAVEFORMAT;

//
typedef struct
{
    s16 valprev;    /* Previous output value */
    u8  index;      /* Index into stepsize table */
} ADPCMSTATE;
//
typedef struct
{
  s32 wordcnt;
  s32 *fillPos;
}PUTAVIBS_HEADER;

typedef struct
{
    RIFFINFO AviRiff;
    LISTINFO hdrl;

    /*** Main AVIHeader ***/
    CHUNKINFO AviHdrChunk;
    MAINAVIHEADER MainAviHdr;
    /*** Video stream Header ***/
    LISTINFO strl_vid;
    CHUNKINFO strh_vid;
    AVISTREAMHEADER StrHdr_vid;
    /*** Video stream format ***/
    CHUNKINFO strf_vid;
    DIBINFO StrFmt_vid;
    char ExtInfo[AVIEXTINFOLENS]; //now, dont care MPEG Quatization, 32 byte is enough//


#ifdef  AVI_AUDIO
    /*** Audio stream Header ***/
    LISTINFO strl_aud;
    CHUNKINFO strh_aud;
    AVISTREAMHEADER StrHdr_aud;
    /*** Audio stream format ***/           
    CHUNKINFO strf_aud;
    WAVINFO StrFmt_aud;
#endif
} HDRLIST;

/*-------------------------------------------------------------------------*/
/* Data object								   */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Index object								   */ 
/*-------------------------------------------------------------------------*/

/***************************************************************************
 * Object definition
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
/* Header object							   */	
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Data object								   */	
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Index object								   */	
/*-------------------------------------------------------------------------*/

#endif
