/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	iis.h

Abstract:

   	The declarations of IIS.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IIS_H__
#define __IIS_H__



/* Type definition */

typedef struct _WAVEFORMAT {
	u16	wFormatTag;
        u16  	nChannels;
        u32	nSamplesPerSec;
        u32	nAvgBytesPerSec;
        u16	nBlockAlign;
        u16	wBitsPerSample;
} WAVEFORMAT;


#define RIFF_MAGIC		{'R','I','F','F'}
#define WAVE_MAGIC		{'W','A','V','E'}
#define FMT_CHUNK_MAGIC		{'f','m','t',' '}
#define FACT_CHUNK_MAGIC	{'f','a','c','t'}
#define DATA_CHUNK_MAGIC	{'d','a','t','a'}

/* size of a standard RIFF/WAVE header created by this tool */
#define RIFF_WAVE_HDR_SZ		(sizeof (riff_wave_hdr_t))

#define WAVE_FORMAT_UNKNOWN	    (0x0000)
#define WAVE_FORMAT_PCM		    (0x0001)
#define WAVE_FORMAT_ADPCM	    (0x0002)
#define WAVE_FORMAT_ALAW	    (0x0006)
#define WAVE_FORMAT_MULAW	    (0x0007)
#define WAVE_FORMAT_OKI_ADPCM	(0x0010)
#define WAVE_FORMAT_IMA_ADPCM   (0x0011)
#define WAVE_FORMAT_DIGISTD	    (0x0015)
#define WAVE_FORMAT_DIGIFIX	    (0x0016)
#define IBM_FORMAT_MULAW	    (0x0101)
#define IBM_FORMAT_ALAW		    (0x0102)
#define IBM_FORMAT_ADPCM	    (0x0103)

#define RIFF_OPEN_OK			0
#define RIFF_OPEN_FILE_OPEN_ERROR	1
#define RIFF_OPEN_FILE_READ_ERROR	2
#define RIFF_OPEN_NO_HDR		3
#define RIFF_OPEN_OUT_OF_MEM		4

#define PLAY_NONE			0
#define PLAY_BEEP			1				
#define PLAY_POWER_ON		2
#define PLAY_POWER_OFF		3
#define PLAY_WARNING		4

/*** TYPEDEFS ***/

/* RIFF/WAVE header */
typedef struct {
	char			riff_magic[4];		/* magic, "RIFF" */
	u32 		riff_size;		/* size of riff */
	char			wave_magic[4];		/* magic, "WAVE" */
} riff_hdr_t;

/* chunk header */
typedef struct {
	char			chunk_magic[4];		/* magic */
	u32 		chunk_size;		/* size of chunk */
} riff_chunk_t;

/* format chunk */
typedef struct {
	u16 		fmt_format;		/* format type */
	u16 		fmt_chan;		/* number of channels */
	u32 		fmt_rate;		/* sampling rate */
	u32		fmt_bytes_per_second;	/* bytes per second */
	u16 		fmt_bytes_per_sample;	/* bytes per sample */
	u16 		fmt_width;		/* width [bit] */
} riff_wave_fmt_t;

typedef struct {
	riff_hdr_t		riff_hdr_hdr;
	riff_chunk_t		riff_hdr_fmt_chunk_hdr;
	riff_wave_fmt_t		riff_hdr_fmt_chunk;
	riff_chunk_t		riff_hdr_data_chunk_hdr;
} riff_wave_hdr_t;

/* chunk node used to return a linked list of chunk headers */
typedef struct riff_chunk_node {
	struct riff_chunk_node	*next;
	long			chunk_offset;
	riff_chunk_t		chunk;
} riff_chunk_node_t;

/* handle */
typedef struct {
	int			status;
	int			fd;
	u32		data_start;
	u32		data_size;
	riff_chunk_node_t	*chunks;
	riff_wave_fmt_t		*format;
} riff_handle_t;

/*** PROTOTYPES ***/

riff_handle_t * riff_open (const char *fname, int flags);
void riff_close (riff_handle_t *rh);
int riff_create_header (riff_wave_hdr_t *buffer, u16 chan, u32 rate, u16 width, u32 data_size);
int riff_modify_header (riff_handle_t *rh, u16 chan, u32 rate, u16 width, u32 data_size);
riff_chunk_node_t *riff_find_chunk_node (riff_handle_t *rh, char *chunk_magic);

/*civic 070829 E*/
s32 iisSetRecDma_ch(u8* buf, u32 siz, u8 ch);
s32 iisStopRec_ch(u8 ch);
s32 iisCheckRecDmaReady_ch(u8 ch);
//void iisRecDMA_ISR_ch0(int);
void iisSetNextRecDMA_ch(u8* buf, u32 siz,u8 ch);

s32 iis2StopPlay(void);
s32 iis2SetPlayDma(u8*, u32);
s32 iis2CheckPlayDmaReady(void);
void iis2SetNextPlayDMA(u8* buf, u32 siz);
s32 iis3StopPlay(void);
s32 iis3SetPlayDma(u8*, u32);
s32 iis3CheckPlayDmaReady(void);
void iis3SetNextPlayDMA(u8* buf, u32 siz);
s32 iis4StopPlay(void);
s32 iis4SetPlayDma(u8*, u32);
s32 iis4CheckPlayDmaReady(void);
void iis4SetNextPlayDMA(u8* buf, u32 siz);
s32 iis5StopPlay(void);
s32 iis5SetPlayDma(u8*, u32);
s32 iis5CheckPlayDmaReady(void);
void iis5SetNextPlayDMA(u8* buf, u32 siz);
#endif
