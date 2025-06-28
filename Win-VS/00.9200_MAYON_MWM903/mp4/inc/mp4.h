/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mp4.h

Abstract:

   	The declarations of 3GPP/MP4 file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __MP4_H__
#define __MP4_H__

/* type definition */

/***************************************************************************
 *
 * QuickTime definition of MP4 file
 *
 ***************************************************************************/

/***************************************************************************
 *  General
 ***************************************************************************/

typedef struct _MP4_AUDIO_FORMAT
{
	u16	number_of_channels;		/* number of channel */		
	u16	sample_size;			/* bits resolution of sample */
	u32	sample_rate;			/* sample per sec */
} MP4_AUDIO_FORMAT;

/* Peter: 0727 S*/
typedef __packed struct _MP4_BOX_HEADER
{
	u32	size;				/* Box size */
	u32	type;				/* Box type */
} MP4_BOX_HEADER_T;
/* Peter: 0727 E*/

/*-------------------------------------------------------------------------*/

/* Peter: 0727 S*/
/* common track								   */
/*-------------------------------------------------------------------------*/
/* depth = 1 */
typedef __packed struct _MP4_TRAK_BOX 
{
	u32	size;				/* 0x000001B6 */
	u32	type;				/* 0x7472616B = "trak" */
} MP4_TRAK_BOX_T;

/* depth = 2 */
typedef __packed struct _MP4_TKHD_BOX 
{
	u32	size;				/* 0x0000005C */
	u32	type;				/* 0x746B6864 = "tkhd" */
	u32	version_flags;			/* 0x00000001 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	track_id;			/* 0x00000001 */
	u32	reserved1;			/* 0x00000000 */
	u32	duration;			/* 0x00000000 */
	u8	reserved2[8];			/* 0x0000000000000000 */
	u16	layer;				/* 0x0000 = spatial priority for overlay */
	u16	alternate_group;		/* 0x0000 = group of movie tracks for QoS choice */
	u16	vloume;				/* 0x0000 = 0.0 */
	u16	reserved3;			/* 0x0000 */
	u32	matrix_structure[9];		/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						/* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						/* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */
	u32	track_width;			/* 0x00000000 */
	u32	track_height;			/* 0x00000000 */
} MP4_TKHD_BOX_T;

/* depth = 2 */
typedef __packed struct _MP4_MDIA_BOX 
{
	u32	size;				/* 0x0000013E */
	u32	type;				/* 0x6D646961 = "mdia" */
} MP4_MDIA_BOX_T;
    
/* depth = 3 */
typedef __packed struct _MP4_MDHD_BOX 
{
	u32	size;				/* 0x00000020 */
	u32	type;				/* 0x6D646864 = "mdhd" */
	u32	version_flags;			/* 0x00000000 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x???????? time_scale_uint/sec e.g. 0x0000001E time_scale_unit/sec for 30 frame/sec */
	u32	duration;			/* 0x???????? time_scale_unit e.g. VOP count time_scale_unit for VOP count frame */
	u16	language;			/* 0x0000 */
	u16	quality;			/* 0x0000 */
} MP4_MDHD_BOX_T;

/* depth = 3 */
typedef __packed struct _MP4_HDLR_BOX 
{
	u32	size;				/* 0x0000002D */
	u32	type;				/* 0x68646C72 = "hdlr" */
	u32	version_flags;			/* 0x00000000 */
	u32	component_type;			/* 0x00000000 */
	u32	component_subtype;		/* 0x76696465 = "vide" (video) */
	u32	component_manufacturer;		/* 0x00000000 */
	u32	component_flags;		/* 0x00000000 */
	u32	component_flags_mask;		/* 0x00000000 */
	u8	component_name[13];		/* 0x56697375616C53747265616D00 = "VisualStream\0" */
} MP4_HDLR_BOX_T;

/*-------------------------------------------------------------------------*/
/* Peter: 0727 E*/











/*-------------------------------------------------------------------------*/
/* soun track								   */
/*-------------------------------------------------------------------------*/
/* soun es descriptor for soun esds */
typedef __packed struct _MP4_SOUN_ES_DESCRIPTOR
{
	/* ES_Descriptor */
	u8	esd_tag;			/* 0x03 */
	u8	esd_tag_length;			/* 0x19 */
	u16	esd_tag_es_id;			/* 0x0000 */
	u8	esd_tag_flag;			/* 0x00 */
    	/* 	DecoderConfigDescriptor */                                    
	u8	dcd_tag;			/* 0x04 */
	u8	dcd_tag_length;			/* 0x11 */
	u8	dcd_tag_obj_type_ind;		/* 0x40 */
	u32	dcd_tag_flag_buf_size;		/* 0x15 + 0x00C000 */
	u32	dcd_tag_max_bitrate;		/* 0x00000000 */
	u32	dcd_tag_avg_bitrate;		/* 0x00000000 */
	/*		DecoderSpecificInfo = 0x1F byte */
	u8	dsi_tag;			/* 0x05 */
	u8	dsi_tag_length;			/* 0x02 */
	u8	dsi_tag_data[2];		/* 0x12 0x10 */
	/*	SLConfigDescriptor */
	u8	slcd_tag;			/* 0x06 */
	u8	slcd_tag_length;		/* 0x01 */
	u8	slcd_tag_data;			/* 0x02 */
} MP4_SOUN_ES_DESCRIPTOR;

/* soun esds atom for soun object sample entry */ 
typedef __packed struct _MP4_SOUN_ESDS_ATOM
{
	u32	size;				/* 0x00000027 */
	u32	type;				/* 0x65736473 = "esds" */
	u32	version_flags;			/* 0x00000000 */
    	MP4_SOUN_ES_DESCRIPTOR	mp4_soun_es_descriptor;
} MP4_SOUN_ESDS_ATOM;

/* soun audio sample entry of sample description table for "stsd" */
typedef __packed struct _MP4_SOUN_AUDIO_SAMPLE_ENTRY
{
	u32	sample_description_size;	/* 0x0000004B */ 
	u32	data_format;			/* 0x6D703461 = "mp4a" */
	u8	reserved1[6];			/* 0x000000000000 */
	u16	data_reference_index;		/* 0x0001 = data_reference of this track */
	u32	version_revision;		/* 0x00000000 */
	u32	vendor;				/* 0x00000000 */
	u16	number_of_channels;		/* 0x0002 */
	u16	sample_size;			/* 0x0010 */
	u16	compression_id;			/* 0x0000 */
	u16	packet_size;			/* 0x0000 */
	u32	sample_rate;			/* 0xAC440000 sample/sec = 44100.0000 sample/sec = 44.1K sample/sec */
	MP4_SOUN_ESDS_ATOM	mp4_soun_esds_atom;
} MP4_SOUN_AUDIO_SAMPLE_ENTRY;

/* soun sample size entry of sample size table for "stsz" - VOP size */
typedef unsigned long	MP4_SOUN_SAMPLE_SIZE_ENTRY;

/* soun time-to-sample entry of time-to-sample table for "stts" */
typedef __packed struct _MP4_SOUN_SAMPLE_TIME_ENTRY 
{
	u32	sample_count;			/* 0x???????? e.g. audioSampleEntryCount */
	u32	sample_duration;		/* 0x???????? e.g. audioSampleEntryDuration */
} MP4_SOUN_SAMPLE_TIME_ENTRY;

/* soun sample-to-chunk entry of sample-to-chunk table for "stsc" */ 
typedef __packed struct _MP4_SOUN_SAMPLE_TO_CHUNK_ENTRY 
{
	u32	first_chunk;			/* 0x00000001 */
	u32	samples_per_chunk;		/* 0x???????? = audioSampleEntryCount */
	u32	sample_description_id;		/* 0x00000001 = audio_sample_entry of this track */
} MP4_SOUN_SAMPLE_TO_CHUNK_ENTRY;

/* soun chunk offset entry of chunk offset table for "stco" */
typedef unsigned long	MP4_SOUN_CHUNK_OFFSET_ENTRY;

/***************************************************************************/
/*                        Box definition							       */	
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*                                 common								   */	
/*-------------------------------------------------------------------------*/

/* File Type Box (depth = 0) 
   Box Type: 'ftyp'         
   Container: File */
typedef __packed struct _MP4_FTYP_BOX
{
	u32	size;				    /* 0x00000018 */
	u32	type;				    /* 0x66747970 = "ftyp" */
	u32	major_brand;			/* 0x48494D41 = "HIMA" */
	u32	minor_version;			/* 0x00000000 */
	u32	compatible_brands[2];   /* 0x69736F6D 6D703432 = "isom" "mp42" */
} MP4_FTYP_BOX_T;

/* Media Box (depth = 0) 
   Box Type: 'mdat'         
   Container: File */
typedef __packed struct _MP4_MDAT_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D646174 = "mdat" */
} MP4_MDAT_BOX_T;

/* Movie Box (depth = 0) 
   Box Type: 'moov'         
   Container: File */
typedef __packed struct _MP4_MOOV_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D6F6F76 = "moov" */
} MP4_MOOV_BOX_T;

/* Movie Header Box (depth = 1) 
   Box Type: 'mvhd'         
   Container: Movie Box ('moov') */
typedef __packed struct _MP4_MVHD_BOX
{
	u32	size;				/* 0x0000006C */
	u32	type;				/* 0x6D766864 = "mvhd" */
	u32	version_flags;		/* 0x00000000 */
	u32	creation_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;	/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x000003E8 = 1000 time_scale_unit/sec = 1 millisec/time_scale_unit */
	u32	duration;			/* 0x???????? time_scale_unit */
	u32	preferred_rate;			/* 0x00010000 = 1.0 (normal rate) */
	u16	preferred_volume;		/* 0x0100 = 1.0 (full volume) */
	u8	reserved[10];			/* 0x00000000000000000000 */
	u32	matrix_structure[9];	/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						        /* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						        /* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */

    /* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */ 
#if 0
	u32	preview_time;			/* 0x00000000 */
	u32	preview_duration;		/* 0x00000000 */
	u32	poster_time;			/* 0x00000000 */
	u32	selection_time;			/* 0x00000000 */
	u32	selection_duration;		/* 0x00000000 */
	u32	current_time;			/* 0x00000000 */
#else
    u32 pre_defined[6];         /* 0x00000000 0x00000000 0x00000000 
                                   0x00000000 0x00000000 0x00000000 */
#endif
    /* mh@2006/10/13: END */

	u32	next_track_id;			/* 0x00000004 */
} MP4_MVHD_BOX_T;

/* Object Descriptor Box (depth = 1) 
   Box Type: 'iods'         
   Container: Movie Box ('moov') */

/* initial object descriptor for "iods" */
typedef __packed struct _MP4_IODS_ENTRY
{
	u8	profile_level_tag;		/* 0x10 */
	u8	profile_level_length;	/* 0x13 */
	u16	profile_level_flag;		/* 0x004F */
	u8	od_prf_lvl_ind;			/* 0xFE = not specify */
	u8	scene_prf_lvl_ind;		/* 0xFE = not specify */
	u8	audio_prf_lvl_ind;		/* 0xFE = not specify */
	u8	visual_prf_lvl_ind;		/* 0xFE = not specify */
	u8	graphics_prf_lvl_ind;	/* 0xFE = not specify */
	u8	sdsm_track_id_tag;		/* 0x0E */
	u8	sdsm_track_id_length;	/* 0x04 */
	u32	sdsm_track_id;			/* 0x00000002 */
	u8	odsm_track_id_tag;		/* 0x0E */
	u8	odsm_trck_id_length;	/* 0x04 */
	u32	odsm_track_id;			/* 0x00000001 */
} MP4_IODS_ENTRY;

typedef __packed struct _MP4_IODS_BOX
{
	u32	size;				/* 0x00000021 */
	u32	type;				/* 0x696F6473 = "iods" */
	u32	version_flags;		/* 0x00000000 */
	MP4_IODS_ENTRY	iods_entry;
} MP4_IODS_BOX_T;

/*-------------------------------------------------------------------------*/
/*                            vide track								   */	
/*-------------------------------------------------------------------------*/

/* Track Box (depth = 1) 
   Box Type: 'trak'         
   Container: Movie Box ('moov') */
typedef __packed struct _MP4_VIDE_TRAK_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7472616B = "trak" */
} MP4_VIDE_TRAK_BOX_T;

/* Track Header Box (depth = 2) 
   Box Type: 'tkhd'         
   Container: Track Box ('trak') */
typedef __packed struct _MP4_VIDE_TKHD_BOX
{
	u32	size;				/* 0x0000005C */
	u32	type;				/* 0x746B6864 = "tkhd" */
	u32	version_flags;		/* 0x00000001 */
	u32	creation_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;	/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	track_id;			/* 0x00000003 */
	u32	reserved1;			/* 0x00000000 */
	u32	duration;			/* 0x???????? */	
	u8	reserved2[8];		/* 0x0000000000000000 */
	u16	layer;				/* 0x0000 = spatial priority for overlay */
	u16	alternate_group;	/* 0x0000 = group of movie tracks for QoS choice */
	u16	vloume;				/* 0x0000 = 0.0 */
	u16	reserved3;			/* 0x0000 */
	u32	matrix_structure[9];/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						    /* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						    /* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */
	u32	track_width;		/* 0x???????? e.g. 0x01600000 = 352 */
	u32	track_height;		/* 0x???????? e.g. 0x01200000 = 288 */
} MP4_VIDE_TKHD_BOX_T;

/* Media Box (depth = 2) 
   Box Type: 'mdia'         
   Container: Track Box ('trak') */
   
/* Peter: 0727 S*/
typedef __packed struct _MP4_VIDE_MDIA_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D646961 = "mdia" */
} MP4_VIDE_MDIA_BOX_T;
/* Peter: 0727 E*/

    
/* Media Header Box (depth = 3) 
   Box Type: 'mdhd'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_VIDE_MDHD_BOX 
{
	u32	size;				/* 0x00000020 */
	u32	type;				/* 0x6D646864 = "mdhd" */
	u32	version_flags;		/* 0x00000000 */
	u32	creation_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;	/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x???????? time_scale_uint/sec e.g. 0x0000001E time_scale_unit/sec for 30 frame/sec */
	u32	duration;			/* 0x???????? time_scale_unit e.g. VOP count time_scale_unit for VOP count frame */
	u16	language;			/* 0x0000 */
	
	/* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0
	u16	quality;			/* 0x0000 */
#else
    u16 pre_defined;        /* 0x0000 */
#endif
    /* mh@2006/10/13: END */
} MP4_VIDE_MDHD_BOX_T;

/* Handler Reference Box (depth = 3) 
   Box Type: 'hdlr'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_VIDE_HDLR_BOX 
{
	u32	size;				    /* 0x0000002D */
	u32	type;				    /* 0x68646C72 = "hdlr" */
	u32	version_flags;			/* 0x00000000 */

    /* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0    
	u32	component_type;			/* 0x00000000 */
	u32	component_subtype;		/* 0x76696465 = "vide" (video) */
	u32	component_manufacturer;	/* 0x00000000 */
	u32	component_flags;		/* 0x00000000 */
	u32	component_flags_mask;	/* 0x00000000 */
	u8	component_name[13];		/* 0x56697375616C53747265616D00 = "VisualStream\0" */
#else
    u32	pre_defined;			/* 0x00000000 */
    u32 handler_type;           /* 0x76696465 = "vide" (video) */
	u32 reserved[3];            /* 0x00000000 0x00000000 0x00000000 */
	u8	name[13];		/* 0x56697375616C53747265616D00 = "VisualStream\0" */
#endif
    /* mh@2006/10/13: END */
} MP4_VIDE_HDLR_BOX_T;

/* Media Information Box (depth = 3) 
   Box Type: 'minf'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_VIDE_MINF_BOX 
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D696E66 = "minf" */
} MP4_VIDE_MINF_BOX_T;


/* Video Media Header Box (depth = 4) 
   Box Type: 'vmhd'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_VIDE_VMHD_BOX 
{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x766D6864 = "vmhd" */
	u32	version_flags;		/* 0x00000001 = no lean ahead */
	u16	graphics_mode;		/* 0x0000 = copy */
	u16	opcolor[3];			/* 0x0000 0000 0000 */
} MP4_VIDE_VMHD_BOX_T;


/* Data Information Box (depth = 4) 
   Box Type: 'dinf'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_VIDE_DINF_BOX
{
	u32	size;				/* 0x00000024 */
	u32	type;				/* 0x64696E66 = "dinf" */
} MP4_VIDE_DINF_BOX_T;
    
/* Data Reference Box (depth = 5) 
   Box Type: 'dref'         
   Container: Data Information Box ('dinf') */

/* data reference entry of data references for "dref" */
typedef __packed struct _MP4_DATA_REFERENCE
{
	u32	size;				/* 0x0000000C */
	u32	type;				/* 0x75726C20 = "url " */
	u32	version_flags;		/* 0x00000001 (self reference) */ 
} MP4_DATA_REFERENCE;

typedef __packed struct _MP4_VIDE_DREF_BOX 
{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x64726566 = "dref" */
	u32	version_flags;		/* 0x00000000 */
	u32	number_of_entries;	/* 0x00000001 */
	MP4_DATA_REFERENCE	mp4_data_reference[1];
} MP4_VIDE_DREF_BOX_T;

/* Sample Table Box (depth = 4) 
   Box Type: 'stbl'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_VIDE_STBL_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7374626C = "stbl" */
} MP4_VIDE_STBL_BOX_T;

/* Sample Description Box (depth = 5) 
   Box Type: 'stsd'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_VIDE_STSD_BOX 
{
	u32	size;				/* 0x000000A8 */
	u32	type;				/* 0x73747364 = "stsd" */
	u32	version_flags;		/* 0x00000000 */
	u32	number_of_entries;	/* 0x00000001 */
	//MP4_VIDE_VISUAL_SAMPLE_ENTRY	mp4_vide_visual_sample_entry[1];
} MP4_VIDE_STSD_BOX_T;

/* vide visual sample entry of sample description table for "stsd" */ 
typedef __packed struct _MP4_VIDE_VISUAL_SAMPLE_ENTRY
 {
	u32	sample_description_size;	/* 0x00000098 */
	u32	data_format;			/* 0x6D703476 = "mp4v" */
    u8	reserved[6];			/* 0x000000000000 */
	u16	data_reference_index;		/* 0x0001 = data_reference of this track */

	/* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0		
	u32	version_revision;		/* 0x00000000 */
	u32	vendor;				/* 0x00000000 */
	u32	temporal_quality;		/* 0x00000000 */
	u32	spatial_quality;		/* 0x00000000 */
	u32	width_height;			/* 0x???????? e.g. 0x01600120 = 352x288 */
	u32	horiz_res;			/* 0x00480000 e.g. 72 dpi */
	u32	vert_res;			/* 0x00480000 e.g. 72 dpi */
	u32	data_size;			/* 0x00000000 */
	u16	frame_count;			/* 0x0001 */
	u8	compressor_name[32];		/* 0x00-00 */
	u16	depth;				/* 0x0018 */
	u16	color_table_id;			/* 0xFFFF */
#else
    u16 pre_defined1;    /* 0x0000 */
    u16 reserved1;       /* 0x0000 */
	u32 pre_defined2[3]; /* 0x00000000 0x00000000 0x00000000 */
	u32	width_height;	 /* 0x???????? e.g. 0x01600120 = 352x288 */
	u32	horizresolution; /* 0x00480000 e.g. 72 dpi */
	u32	vertresolution;	 /* 0x00480000 e.g. 72 dpi */
	u32 reserved2;       /* 0x00000000 */
	u16	frame_count;	 /* 0x0001 */
	u8 compressor_name[32];/* 0x00-00 */
	u16	depth;			 /* 0x0018 */
	u16 pre_defined3;    /* 0xFFFF */
#endif
    /* mh@2006/10/13: END */

	//MP4_VIDE_ESDS_BOX_T	mp4_vide_esds_atom;
} MP4_VIDE_VISUAL_SAMPLE_ENTRY;

/* ES Descriptor Box */
typedef __packed struct _MP4_VIDE_ESDS_BOX
{
	u32	size;				/* 0x00000042 */
	u32	type;				/* 0x65736473 = "esds" */
	u32	version_flags;			/* 0x00000000 */
	//MP4_VIDE_ES_DESCRIPTOR	mp4_vide_es_descriptor;
} MP4_VIDE_ESDS_BOX_T;

/* vide es descriptor for vide visual esds */
/* ES_Descriptor = 0x36 byte */
typedef __packed struct _MP4_VIDE_ES_DESCRIPTOR
{
	u8	esd_tag;			/* 0x03   */
	u8	esd_tag_length;		/* 0x34   */
	u16	esd_tag_es_id;		/* 0x0000 */
	u8	esd_tag_flag;		/* 0x00   */
} MP4_VIDE_ES_DESCRIPTOR;

/* DecoderConfigDescriptor = 0x2E byte */
typedef __packed struct _MP4_VIDE_DECODER_CONFIG_DESCRIPTOR
{
	u8	dcd_tag;			/* 0x04 */
	u8	dcd_tag_length;			/* 0x2C */
	u8	dcd_tag_obj_type_ind;		/* 0x20 */
	u32	dcd_tag_flag_buf_size;		/* 0x11 + 0x0036A6 */
	u32	dcd_tag_max_bitrate;		/* 0x0011A0F0 */
	u32	dcd_tag_avg_bitrate;		/* 0x000C3754 */
} MP4_VIDE_DECODER_CONFIG_DESCRIPTOR;

/* DecoderSpecificInfo = 0x1F byte */
typedef __packed struct _MP4_VIDE_DECODER_SPECIFIC_INFO
{
	u8	dsi_tag;			/* 0x05 */
	u8	dsi_tag_length;			/* 0x1D */
	u8	dsi_tag_data[10];		/* e.g. */
						/*	0x00, 0x00, 0x01, 0xB0,	*/
						/*	0x01,			*/	
						/*	0x00, 0x00, 0x01, 0xB5,	*/
						/*	0x09,			*/ 	
						/*	0x00, 0x00, 0x01, 0x00,	*/
						/*	0x00, 0x00, 0x01, 0x20,	*/
						/*	0x00, 0xC8, 0x88, 0x80,	*/ 
						/*	0x0F, 0x50, 0xB0, 0x42,	*/
						/*	0x41, 0x41, 0x41,	*/
} MP4_VIDE_DECODER_SPECIFIC_INFO;
                                           
/* SLConfigDescriptor = 0x03 byte */
typedef __packed struct _MP4_VIDE_SL_CONFIG_DESCRIPTOR
{
	u8	slcd_tag;			    /* 0x06 */
	u8	slcd_tag_length;		/* 0x01 */
	u8	slcd_tag_data;			/* 0x02 */
} MP4_VIDE_SL_CONFIG_DESCRIPTOR;

/* Decoding Time to Sample Box (depth = 5) 
   Box Type: 'stts'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_VIDE_STTS_BOX
{
	u32	size;				/* 0x00000018 */
	u32	type;				/* 0x73747473 = "stts" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */                                  
} MP4_VIDE_STTS_BOX_T;

/* vide time-to-sample entry of time-to-sample table for "stts" */
typedef __packed struct _MP4_VIDE_SAMPLE_TIME_ENTRY
{
	u32	sample_count;			/* 0x???????? = vop_count */
	u32	sample_duration;		/* 0x00000001 */
} MP4_VIDE_SAMPLE_TIME_ENTRY;

/* Sample to Chunk Box (depth = 5) 
   Box Type: 'stsc'         
   Container: Sample Table Box ('stbl') */

/* vide sample-to-chunk entry of sample-to-chunk table for "stsc" */ 
typedef __packed struct _MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY 
{
	u32	first_chunk;			/* 0x00000001 */
	u32	samples_per_chunk;		/* 0x???????? = vop_count */
	u32	sample_description_id;		/* 0x00000001 = visual_sample_entry of this track */
} MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY;

typedef __packed struct _MP4_VIDE_STSC_BOX
{
	u32	size;				    /* 0x0000001C */
	u32	type;				    /* 0x73747363 = "stsc" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
/*	MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY	mp4_vide_sample_to_chunk_entry[1]; */                                      
} MP4_VIDE_STSC_BOX_T;


/* Sample Size Box (depth = 5) 
   Box Type: 'stsz'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_VIDE_STSZ_BOX
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7374737A = "stsz" */
	u32	version_flags;			/* 0x00000000 */
	u32	sample_size;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x???????? = VOP count */
	/* sample size table */                                    
} MP4_VIDE_STSZ_BOX_T;

/* vide sample size entry of sample size table for "stsz" - VOP size */
typedef	unsigned long	MP4_VIDE_SAMPLE_SIZE_ENTRY;


/* Chunk Offset Box (depth = 5) 
   Box Type: 'stco'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_VIDE_STCO_BOX 
{
	u32	size;				/* 0x00000000 */
	u32	type;				/* 0x7374636F = "stco" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000000 */                                    
} MP4_VIDE_STCO_BOX_T;

/* vide chunk offset entry of chunk offset table for "stco" */
typedef unsigned long	MP4_VIDE_CHUNK_OFFSET_ENTRY;

/* Sync Sample Box (depth = 5) 
   Box Type: 'stss'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_VIDE_STSS_BOX 
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x73747373 = "stss" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x???????? = I-VOP count */
	/* sync sample table */
} MP4_VIDE_STSS_BOX_T;

/* vide sync sample entry of sync sample table for "stss" - index of I-VOP */
typedef unsigned long	MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY;

/*-------------------------------------------------------------------------*/
/*                            soun track								   */	
/*-------------------------------------------------------------------------*/
/* depth = 1 */
typedef __packed struct _MP4_SOUN_TRAK_ATOM 
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7472616B = "trak" */
} MP4_SOUN_TRAK_ATOM;

/* depth = 2 */
typedef __packed struct _MP4_SOUN_TKHD_ATOM 
{
	u32	size;				/* 0x0000005C */
	u32	type;				/* 0x746B6864 = "tkhd" */
	u32	version_flags;			/* 0x00000001 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	track_id;			/* 0x00000004 */
	u32	reserved1;			/* 0x00000000 */
	u32	duration;			/* 0x???????? millisecond */
	u8	reserved2[8];			/* 0x0000000000000000 */
	u16	layer;				/* 0x0000 = spatial priority for overlay */
	u16	alternate_group;		/* 0x0000 = group of movie tracks for QoS choice */
	u16	vloume;				/* 0x0100 = 1.0 */
	u16	reserved3;			/* 0x0000 */
	u32	matrix_structure[9];		/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						/* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						/* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */
	u32	track_width;			/* 0x00000000 */
	u32	track_height;			/* 0x00000000 */
} MP4_SOUN_TKHD_ATOM;

/* depth = 2 */
/* Peter: 0727 S*/
typedef __packed struct _MP4_SOUN_MDIA_ATOM 
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D646961 = "mdia" */
} MP4_SOUN_MDIA_ATOM;
/* Peter: 0727 E*/
    
/* depth = 3 */
typedef __packed struct _MP4_SOUN_MDHD_ATOM 
{
	u32	size;				/* 0x00000020 */
	u32	type;				/* 0x6D646864 = "mdhd" */
	u32	version_flags;			/* 0x00000000 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x???????? time_scale_uint/sec = sampleRate time_scale_unit/sec e.g 44100 time_scale_unit/sec = 44100 sample/sec */
	u32	duration;			/* 0x???????? time_scale_unit = sampleCount time_scale_unit */
	u16	language;			/* 0x0000 */
	u16	quality;			/* 0x0000 */
} MP4_SOUN_MDHD_ATOM;

/* depth = 3 */
typedef __packed struct _MP4_SOUN_HDLR_ATOM 
{
	u32	size;				/* 0x0000002C */
	u32	type;				/* 0x68646C72 = "hdlr" */
	u32	version_flags;			/* 0x00000000 */
	u32	component_type;			/* 0x00000000 */
	u32	component_subtype;		/* 0x736F756E = "soun" (sound) */
	u32	component_manufacturer;		/* 0x00000000 */
	u32	component_flags;		/* 0x00000000 */
	u32	component_flags_mask;		/* 0x00000000 */
	u8	component_name[12];		/* 0x417564696F53747265616D00 = "AudioStream\0" */
} MP4_SOUN_HDLR_ATOM;

/* depth = 3 */
typedef __packed struct _MP4_SOUN_MINF_ATOM 
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x6D696E66 = "minf" */
} MP4_SOUN_MINF_ATOM;

/* depth = 4 */
typedef __packed struct _MP4_SOUN_DINF_ATOM 
{
	u32	size;				/* 0x00000024 */
	u32	type;				/* 0x64696E66 = "dinf" */
} MP4_SOUN_DINF_ATOM;
    
/* depth = 5 */
typedef __packed struct _MP4_SOUN_DREF_ATOM 
{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x64726566 = "dref" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_DATA_REFERENCE	mp4_data_reference[1];
} MP4_SOUN_DREF_ATOM;

/* depth = 4 */
typedef __packed struct _MP4_SOUN_STBL_ATOM
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7374626C = "stbl" */
} MP4_SOUN_STBL_ATOM;

/* depth = 5 */
typedef __packed struct _MP4_SOUN_STSD_ATOM
{
	u32	size;				/* 0x0000005B */
	u32	type;				/* 0x73747364 = "stsd" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SOUN_AUDIO_SAMPLE_ENTRY	mp4_soun_audio_sample_entry[1];
} MP4_SOUN_STSD_ATOM;

/* depth = 5 */
typedef __packed struct _MP4_SOUN_STSZ_ATOM
{
	u32	size;				/* 0x???????? */
	u32	type;				/* 0x7374737A = "stsz" */
	u32	version_flags;			/* 0x00000000 */
	u32	sample_size;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x???????? = VOP count */
	/* sample size table */                              
} MP4_SOUN_STSZ_ATOM;

/* depth = 5 */
typedef __packed struct _MP4_SOUN_STTS_ATOM 
{
	u32	size;				/* 0x00000018 */
	u32	type;				/* 0x73747473 = "stts" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SOUN_SAMPLE_TIME_ENTRY	mp4_soun_sample_time_entry[1];                                       
} MP4_SOUN_STTS_ATOM;

/* depth = 5 */
typedef __packed struct _MP4_SOUN_STSC_ATOM 
{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x73747363 = "stsc" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SOUN_SAMPLE_TO_CHUNK_ENTRY	mp4_soun_sample_to_chunk_entry[1];                                       
} MP4_SOUN_STSC_ATOM;

/* depth = 5 */
typedef __packed struct _MP4_SOUN_STCO_ATOM 
{
	u32	size;				/* 0x00000000 */
	u32	type;				/* 0x7374636F = "stco" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000000 */                                    
} MP4_SOUN_STCO_ATOM;

/* depth = 4 */
typedef __packed struct _MP4_SOUN_SMHD_ATOM 
{
	u32	size;				/* 0x00000010 */
	u32	type;				/* 0x736D6864 = "smhd" */
	u32	version_flags;			/* 0x00000000 */
	u16	balance;			/* 0x0000 = (0.0) */
	u16	reserved;			/* 0x0000 */
} MP4_SOUN_SMHD_ATOM;

/*-------------------------------------------------------------------------*/
/*                            odsm track								   */	
/*-------------------------------------------------------------------------*/

/* Track Box (depth = 1) 
   Box Type: 'trak'         
   Container: Movie Box ('moov') */
typedef __packed struct _MP4_ODSM_TRAK_BOX 
{
	u32	size;				/* 0x000001B6 */
	u32	type;				/* 0x7472616B = "trak" */
} MP4_ODSM_TRAK_BOX_T;

/* Track Header Box (depth = 2) 
   Box Type: 'tkhd'         
   Container: Track Box ('trak') */
typedef __packed struct _MP4_ODSM_TKHD_BOX 
{
	u32	size;				/* 0x0000005C */
	u32	type;				/* 0x746B6864 = "tkhd" */
	u32	version_flags;			/* 0x00000001 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	track_id;			/* 0x00000001 */
	u32	reserved1;			/* 0x00000000 */
	u32	duration;			/* 0x00000000 */
	u8	reserved2[8];			/* 0x0000000000000000 */
	u16	layer;				/* 0x0000 = spatial priority for overlay */
	u16	alternate_group;		/* 0x0000 = group of movie tracks for QoS choice */
	u16	vloume;				/* 0x0000 = 0.0 */
	u16	reserved3;			/* 0x0000 */
	u32	matrix_structure[9];		/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						/* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						/* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */
	u32	track_width;			/* 0x00000000 */
	u32	track_height;			/* 0x00000000 */
} MP4_ODSM_TKHD_BOX_T;


/* Track Reference Box (depth = 2) 
   Box Type: 'tref'         
   Container: Track Box ('trak') */

/* odsm tref type entry of tref type list for 'tref' */ 
typedef __packed struct _MP4_ODSM_TREF_TYPE_ENTRY
{
	u32	size;				/* 0x0000000C */
	u32	type;				/* 0x6D706F64 = "mpod" */
	u32	track_ids[2];			/* 0x00000003 = vide track id */
						/* 0x00000004 = soun track id */
} MP4_ODSM_TREF_TYPE_ENTRY;

typedef __packed struct _MP4_ODSM_TREF_BOX 
{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x74726566 = "tref" */
	MP4_ODSM_TREF_TYPE_ENTRY	mp4_odsm_tref_type_entry[1];
} MP4_ODSM_TREF_BOX_T;


/* Media Box (depth = 2) 
   Box Type: 'mdia'         
   Container: Track Box ('trak') */
/* Peter: 0727 S*/
typedef __packed struct _MP4_ODSM_MDIA_BOX 
{
	u32	size;				/* 0x0000013E */
	u32	type;				/* 0x6D646961 = "mdia" */
} MP4_ODSM_MDIA_BOX_T;
/* Peter: 0727 E*/
    
/* Media Header Box (depth = 3) 
   Box Type: 'mdhd'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_ODSM_MDHD_BOX
{
	u32	size;				/* 0x00000020 */
	u32	type;				/* 0x6D646864 = "mdhd" */
	u32	version_flags;			/* 0x00000000 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x000003E8 = 1000 time_scale_unit/sec = 1 millisec/time_scale_unit */
	u32	duration;			/* 0x00000000 time_scale_unit */
	u16	language;			/* 0x0000 */
	
	/* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0
	u16	quality;			/* 0x0000 */
#else
    u16 pre_defined;        /* 0x0000 */
#endif
    /* mh@2006/10/13: END */

} MP4_ODSM_MDHD_BOX_T;

/* Handler Reference Box (depth = 3) 
   Box Type: 'hdlr'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_ODSM_HDLR_BOX
{
	u32	size;				/* 0x00000037 */
	u32	type;				/* 0x68646C72 = "hdlr" */
	u32	version_flags;			/* 0x00000000 */

    /* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0    
	u32	component_type;			/* 0x00000000 */
	u32	component_subtype;		/* 0x76696465 = "vide" (video) */
	u32	component_manufacturer;	/* 0x00000000 */
	u32	component_flags;		/* 0x00000000 */
	u32	component_flags_mask;	/* 0x00000000 */
	u8	component_name[13];		/* 0x56697375616C53747265616D00 = "VisualStream\0" */
#else
    u32	pre_defined;			/* 0x00000000 */
    u32 handler_type;           /* 0x6F64736D = "odsm" (object descriptor) */
	u32 reserved[3];            /* 0x00000000 0x00000000 0x00000000 */
	u8	name[23];		/* 0x4F626A65637444657363726970746F7253747265616D00 = */
						/* "ObjectDescriptorStream\0" */
#endif
    /* mh@2006/10/13: END */
		
} MP4_ODSM_HDLR_BOX_T;

/* Media Information Box (depth = 3) 
   Box Type: 'minf'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_ODSM_MINF_BOX
{
	u32	size;				/* 0x000000DF */
	u32	type;				/* 0x6D696E66 = "minf" */
} MP4_ODSM_MINF_BOX_T;

/* Null Media Header Box (depth = 4) 
   Box Type: 'nmhd'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_ODSM_NMHD_BOX
{
	u32	size;				/* 0x0000000C */
	u32	type;				/* 0x6E6D6864 = "nmhd" */
	u32	version_flags;			/* 0x00000000 */
} MP4_ODSM_NMHD_BOX_T;

/* Data Information Box (depth = 4) 
   Box Type: 'dinf'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_ODSM_DINF_BOX
{
	u32	size;				/* 0x00000024 */
	u32	type;				/* 0x64696E66 = "dinf" */
} MP4_ODSM_DINF_BOX_T;
    
/* Data Reference Box (depth = 5) 
   Box Type: 'dref'         
   Container: Data Information Box ('dinf') */
typedef __packed struct _MP4_ODSM_DREF_BOX 
{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x64726566 = "dref" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_DATA_REFERENCE	mp4_data_reference[1];
} MP4_ODSM_DREF_BOX_T;

/* Sample Table Box (depth = 4) 
   Box Type: 'stbl'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_ODSM_STBL_BOX
{
	u32	size;				/* 0x000000A7 */
	u32	type;				/* 0x7374626C = "stbl" */
} MP4_ODSM_STBL_BOX_T;

/* Sample Description Box (depth = 5) 
   Box Type: 'stsd'         
   Container: Sample Table Box ('stbl') */

/* odsm es descriptor for odsm esds */
typedef __packed struct _MP4_ODSM_ES_DESCRIPTOR
{
	/* ES_Descriptor */
	u8	esd_tag;			/* 0x03 */
	u8	esd_tag_length;			/* 0x15 */
	u16	esd_tag_es_id;			/* 0x0000 */
	u8	esd_tag_flag;			/* 0x00 */
    		/* DecoderConfigDescriptor */                                   
		u8	dcd_tag;			/* 0x04 */
		u8	dcd_tag_length;			/* 0x0D */
		u8	dcd_tag_obj_type_ind;		/* 0x02 */
		u32	dcd_tag_flag_buf_size;		/* 0x05 + 0x000084 */
		u32	dcd_tag_max_bitrate;		/* 0x00000000 */
		u32	dcd_tag_avg_bitrate;		/* 0x00000000 */
		/* SLConfigDescriptor */
		u8	slcd_tag;			/* 0x06 */
		u8	slcd_tag_length;		/* 0x01 */
		u8	slcd_tag_data;			/* 0x02 */
} MP4_ODSM_ES_DESCRIPTOR;

/* odsm esds atom for odsm object sample entry */
typedef __packed struct _MP4_ODSM_ESDS_BOX
{
	u32	size;				/* 0x00000023 */
	u32	type;				/* 0x65736473 = "esds" */
	u32	version_flags;			/* 0x00000000 */
	MP4_ODSM_ES_DESCRIPTOR	mp4_odsm_es_descriptor;
} MP4_ODSM_ESDS_BOX_T;

/* odsm object sample entry of sample description table for "stsd" */
typedef __packed struct _MP4_ODSM_OBJECT_SAMPLE_ENTRY
{
	u32	sample_description_size;	/* 0x00000033 */
	u32	data_format;			/* 0x6D703473 = "mp4s" */
	u8	reserved[6];			/* 0x000000000000 */
	u16	data_reference_index;		/* 0x0001 = data_reference of this track */
	MP4_ODSM_ESDS_BOX_T	mp4_odsm_esds_atom;
} MP4_ODSM_OBJECT_SAMPLE_ENTRY;

typedef __packed struct _MP4_ODSM_STSD_BOX 
{
	u32	size;				/* 0x00000043 */
	u32	type;				/* 0x73747364 = "stsd" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_ODSM_OBJECT_SAMPLE_ENTRY	mp4_odsm_object_sample_entry[1];
} MP4_ODSM_STSD_BOX_T;

/* Decoding Time to Sample Box (depth = 5) 
   Box Type: 'stts'         
   Container: Sample Table Box ('stbl') */
   
/* osdm time-to-sample entry of time-to-sample table for "stts" */
typedef __packed struct _MP4_ODSM_SAMPLE_TIME_ENTRY
{
	u32	sample_count;			/* 0x00000001 */
	u32	sample_duration;		/* 0x00000000 */
} MP4_ODSM_SAMPLE_TIME_ENTRY;

typedef __packed struct _MP4_ODSM_STTS_BOX 
{
	u32	size;				/* 0x00000018 */
	u32	type;				/* 0x73747473 = "stts" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_ODSM_SAMPLE_TIME_ENTRY	mp4_odsm_sample_time_entry[1];                                       
} MP4_ODSM_STTS_BOX_T;

/* Sample to Chunk Box (depth = 5) 
   Box Type: 'stsc'         
   Container: Sample Table Box ('stbl') */

/* odsm sample-to-chunk entry of sample-to-chunk table for "stsc" */
typedef __packed struct _MP4_ODSM_SAMPLE_TO_CHUNK_ENTRY 
{
	u32	first_chunk;			/* 0x00000001 */
	u32	samples_per_chunk;		/* 0x00000001 */
	u32	sample_description_id;		/* 0x00000001 = object_sample_entry of this track */
} MP4_ODSM_SAMPLE_TO_CHUNK_ENTRY;

typedef __packed struct _MP4_ODSM_STSC_BOX
{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x73747363 = "stsc" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_ODSM_SAMPLE_TO_CHUNK_ENTRY	mp4_odsm_sample_to_chunk_entry[1];                                       
} MP4_ODSM_STSC_BOX_T;



/* Sample Size Box (depth = 5) 
   Box Type: 'stsz'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_ODSM_STSZ_BOX 
{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x7374737A = "stsz" */
	u32	version_flags;			/* 0x00000000 */
	u32	sample_size;			/* 0x???????? = VOP count */
	u32	number_of_entries;		/* 0x00000001 */       
} MP4_ODSM_STSZ_BOX_T;

/* Chunk Offset Box (depth = 5) 
   Box Type: 'stco'         
   Container: Sample Table Box ('stbl') */

/* odsm chunk offset entry of chunk offset table for "stco" */ 
typedef unsigned long MP4_ODSM_CHUNK_OFFSET_ENTRY;

typedef __packed struct _MP4_ODSM_STCO_BOX 
{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x7374636F = "stco" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_ODSM_CHUNK_OFFSET_ENTRY	mp4_odsm_chunk_offset_entry[1];                                       
} MP4_ODSM_STCO_BOX_T;

/*-------------------------------------------------------------------------*/
/*                            sdsm track								   */
/*-------------------------------------------------------------------------*/

/* Track Box (depth = 1) 
   Box Type: 'trak'         
   Container: Movie Box ('moov') */
typedef __packed struct _MP4_SDSM_TRAK_BOX 
{
	u32	size;				/* 0x000001A7 */
	u32	type;				/* 0x7472616B = "trak" */
} MP4_SDSM_TRAK_BOX_T;

/* Track Header Box (depth = 2) 
   Box Type: 'tkhd'         
   Container: Track Box ('trak') */
typedef __packed struct _MP4_SDSM_TKHD_BOX
{
	u32	size;				/* 0x0000005C */
	u32	type;				/* 0x746B6864 = "tkhd" */
	u32	version_flags;			/* 0x00000001 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	track_id;			/* 0x00000002 */
	u32	reserved1;			/* 0x00000000 */
	u32	duration;			/* 0x00000000 */
	u8	reserved2[8];			/* 0x0000000000000000 */
	u16	layer;				/* 0x0000 = spatial priority for overlay */
	u16	alternate_group;		/* 0x0000 = group of movie tracks for QoS choice */
	u16	vloume;				/* 0x0000 = 0.0 */
	u16	reserved3;			/* 0x0000 */
	u32	matrix_structure[9];		/* 0x00010000 0x00000000 0x00000000   1.0 0.0 0.0 */
						/* 0x00000000 0x00010000 0x00000000 = 0.0 0.0 0.0 */
						/* 0x00000000 0x00000000 0x40000000   0.0 0.0 1.0 */
	u32	track_width;			/* 0x00000000 */
	u32	track_height;			/* 0x00000000 */
} MP4_SDSM_TKHD_BOX_T;

/* Media Box (depth = 2) 
   Box Type: 'mdia'         
   Container: Track Box ('trak') */
   
/* Peter: 0727 S*/
typedef __packed struct _MP4_SDSM_MDIA_BOX
{
	u32	size;				/* 0x00000143 */
	u32	type;				/* 0x6D646961 = "mdia" */
} MP4_SDSM_MDIA_BOX_T;
/* Peter: 0727 E*/
    
/* Media Header Box (depth = 3) 
   Box Type: 'mdhd'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_SDSM_MDHD_BOX 
{
	u32	size;				/* 0x00000020 */
	u32	type;				/* 0x6D646864 = "mdhd" */
	u32	version_flags;			/* 0x00000000 */
	u32	creation_time;			/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	modification_time;		/* 0xBC191380 = 00:00:00 2004/01/01 */
	u32	time_scale;			/* 0x000003E8 = 1000 time_scale_unit/sec = 1 millisec/time_scale_unit */
	u32	duration;			/* 0x00000000 time_scale_unit */
	u16	language;			/* 0x0000 */

	/* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0
	u16	quality;			/* 0x0000 */
#else
    u16 pre_defined;        /* 0x0000 */
#endif
    /* mh@2006/10/13: END */

} MP4_SDSM_MDHD_BOX_T;

/* Handler Reference Box (depth = 3) 
   Box Type: 'hdlr'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_SDSM_HDLR_BOX 
{
	u32	size;				/* 0x00000037 */
	u32	type;				/* 0x68646C72 = "hdlr" */
	u32	version_flags;			/* 0x00000000 */

    /* mh@2006/10/13: Replace the data elements of QTFF by that of ISO14496-12 */
#if 0    
	u32	component_type;			/* 0x00000000 */
	u32	component_subtype;		/* 0x76696465 = "vide" (video) */
	u32	component_manufacturer;	/* 0x00000000 */
	u32	component_flags;		/* 0x00000000 */
	u32	component_flags_mask;	/* 0x00000000 */
	u8	component_name[13];		/* 0x56697375616C53747265616D00 = "VisualStream\0" */
#else
    u32	pre_defined;			/* 0x00000000 */
    u32 handler_type;           /* 0x7364736D = "sdsm" (scene descriptor) */
	u32 reserved[3];            /* 0x00000000 0x00000000 0x00000000 */
	u8	name[23];		/* 0x5363656E654465736372697074696F6E53747265616D00 */
						/* "SceneDescriptionStream\0" */
#endif
    /* mh@2006/10/13: END */
		
} MP4_SDSM_HDLR_BOX_T;


/* Media Information Box (depth = 3) 
   Box Type: 'minf'         
   Container: Media Box ('mdia') */
typedef __packed struct _MP4_SDSM_MINF_BOX 

{
	u32	size;				/* 0x000000E4 */
	u32	type;				/* 0x6D696E66 = "minf" */
} MP4_SDSM_MINF_BOX_T;


/* Null Media Header Box (depth = 4) 
   Box Type: 'nmhd'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_SDSM_NMHD_BOX 

{
	u32	size;				/* 0x0000000C */
	u32	type;				/* 0x6E6D6864 = "nmhd" */
	u32	version_flags;			/* 0x00000000 */
} MP4_SDSM_NMHD_BOX_T;



/* Data Information Box (depth = 4) 
   Box Type: 'dinf'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_SDSM_DINF_BOX

{
	u32	size;				/* 0x00000024 */
	u32	type;				/* 0x64696E66 = "dinf" */
} MP4_SDSM_DINF_BOX_T;

    
/* Data Reference Box (depth = 5) 
   Box Type: 'dref'         
   Container: Data Information Box ('dinf') */
typedef __packed struct _MP4_SDSM_DREF_BOX 

{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x64726566 = "dref" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_DATA_REFERENCE	mp4_data_reference[1];
} MP4_SDSM_DREF_BOX_T;


/* Sample Table Box (depth = 4) 
   Box Type: 'stbl'         
   Container: Media Information Box ('minf') */
typedef __packed struct _MP4_SDSM_STBL_BOX 

{
	u32	size;				/* 0x000000AC */
	u32	type;				/* 0x7374626C = "stbl" */
} MP4_SDSM_STBL_BOX_T;


/* Sample Description Box (depth = 5) 
   Box Type: 'stsd'         
   Container: Sample Table Box ('stbl') */

/* sdsm es descriptor for odsm esds */
typedef __packed struct _MP4_SDSM_ES_DESCRIPTOR
{
	/* ES_Descriptor */
	u8	esd_tag;			/* 0x03 */
	u8	esd_tag_length;			/* 0x1A */
	u16	esd_tag_es_id;			/* 0x0000 */
	u8	esd_tag_flag;			/* 0x00 */
		/* DecoderConfigDescriptor */                                   
		u8	dcd_tag;			/* 0x04 */
		u8	dcd_tag_length;			/* 0x12 */
		u8	dcd_tag_obj_type_ind;		/* 0x02 */
		u32	dcd_tag_flag_buf_size;		/* 0x0D + 0x000014 */
		u32	dcd_tag_max_bitrate;		/* 0x00000000 */
		u32	dcd_tag_avg_bitrate;		/* 0x00000000 */
			/* DecoderSpecificInfo */
			u8	dsi_tag;			/* 0x05 */
			u8	dsi_tag_length;			/* 0x03 */
			u8	dsi_tag_data[3];		/* 0x000040 */
		/* SLConfigDescriptor */
		u8	slcd_tag;			/* 0x06 */
		u8	slcd_tag_length;		/* 0x01 */
		u8	slcd_tag_data;			/* 0x02 */
} MP4_SDSM_ES_DESCRIPTOR;

/* sdsm esds atom for sdsm object sample entry */ 
typedef __packed struct _MP4_SDSM_ESDS_BOX

{
	u32	size;				/* 0x00000028 */
	u32	type;				/* 0x65736473 = "esds" */
	u32	version_flags;			/* 0x00000000 */
	MP4_SDSM_ES_DESCRIPTOR	mp4_sdsm_es_descriptor;
} MP4_SDSM_ESDS_BOX_T;


/* sdsm scene sample entry of sample description table for "stsd" */
typedef __packed struct _MP4_SDSM_SCENE_SAMPLE_ENTRY
{
	u32	sample_description_size;	/* 0x00000038 */
	u32	data_format;			/* 0x6D703473 = "mp4s" */
	u8	reserved[6];			/* 0x000000000000 */
	u16	data_reference_index;		/* 0x0001 = data_reference of this track */
	MP4_SDSM_ESDS_BOX_T	mp4_sdsm_esds_atom;

} MP4_SDSM_SCENE_SAMPLE_ENTRY;
   
typedef __packed struct _MP4_SDSM_STSD_BOX 

{
	u32	size;				/* 0x00000048 */
	u32	type;				/* 0x73747364 = "stsd" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SDSM_SCENE_SAMPLE_ENTRY	mp4_sdsm_scene_sample_entry[1];
} MP4_SDSM_STSD_BOX_T;


/* Decoding Time to Sample Box (depth = 5) 
   Box Type: 'stts'         
   Container: Sample Table Box ('stbl') */

/* sdsm time-to-sample entry of time-to-sample table for "stts" */
typedef __packed struct _MP4_SDSM_SAMPLE_TIME_ENTRY
{
	u32	sample_count; 			/* 0x00000001 */
	u32	sample_duration;		/* 0x00000000 */
} MP4_SDSM_SAMPLE_TIME_ENTRY;

typedef __packed struct _MP4_SDSM_STTS_BOX 

{
	u32	size;				/* 0x00000018 */
	u32	type;				/* 0x73747473 = "stts" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SDSM_SAMPLE_TIME_ENTRY	mp4_sdsm_sample_time_entry[1];                                       
} MP4_SDSM_STTS_BOX_T;


/* Sample to Chunk Box (depth = 5) 
   Box Type: 'stsc'         
   Container: Sample Table Box ('stbl') */


/* sdsm sample-to-chunk entry of sample-to-chunk table for "stsc" */ 
typedef __packed struct _MP4_SDSM_SAMPLE_TO_CHUNK_ENTRY
{
	u32	first_chunk;			/* 0x00000001 */
	u32	samples_per_chunk;		/* 0x00000001 */
	u32	sample_description_id;		/* 0x00000001 = scene_sample_entry of this track */
} MP4_SDSM_SAMPLE_TO_CHUNK_ENTRY;

typedef __packed struct _MP4_SDSM_STSC_BOX 

{
	u32	size;				/* 0x0000001C */
	u32	type;				/* 0x73747363 = "stsc" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SDSM_SAMPLE_TO_CHUNK_ENTRY	mp4_sdsm_sample_to_chunk_entry[1];                                       
} MP4_SDSM_STSC_BOX_T;


/* Sample Size Box (depth = 5) 
   Box Type: 'stsz'         
   Container: Sample Table Box ('stbl') */
typedef __packed struct _MP4_SDSM_STSZ_BOX

{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x7374737A = "stsz" */
	u32	version_flags;			/* 0x00000000 */
	u32	sample_size;			/* 0x0000000A */
	u32	number_of_entries;		/* 0x00000001 */           
} MP4_SDSM_STSZ_BOX_T;


/* Chunk Offset Box (depth = 5) 
   Box Type: 'stco'         
   Container: Sample Table Box ('stbl') */

/* sdsm chunk offset entry of chunk offset table for "stco" */
typedef unsigned long	MP4_SDSM_CHUNK_OFFSET_ENTRY;

typedef __packed struct _MP4_SDSM_STCO_BOX 

{
	u32	size;				/* 0x00000014 */
	u32	type;				/* 0x7374636F = "stco" */
	u32	version_flags;			/* 0x00000000 */
	u32	number_of_entries;		/* 0x00000001 */
	MP4_SDSM_CHUNK_OFFSET_ENTRY	mp4_sdsm_chunk_offset_entry[1];                                       
} MP4_SDSM_STCO_BOX_T;


#endif