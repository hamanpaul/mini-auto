/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    asf.h

Abstract:

    The declarations of ASF file.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#ifndef __ASF_H__
#define __ASF_H__

/* type definition */

/***************************************************************************
 *
 * Object definition of ASF file
 *
 ***************************************************************************/

/***************************************************************************
 * General
 ***************************************************************************/
#define ASF_WAVE_FORMAT_PCM         0x0001
#define ASF_WAVE_FORMAT_DVI_ADPCM   0x0011

typedef struct _ASF_AUDIO_FORMAT 
{
    u16     codec_id_format_tag;        /* 0x0001 = ASF_WAVE_FORMAT_PCM */
    u16     number_of_channels;     /* channel */
    u32     samples_per_second;     /* sample/sec */
    u32     avg_num_of_bytes_per_sec;   /* byte/sec */
    u16     block_alignment;        /* byte */
    u16     bits_per_sample;        /* bit/sample */
} ASF_AUDIO_FORMAT;

/***************************************************************************
 * Entry definition
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
/* Header object                               */   
/*-------------------------------------------------------------------------*/


#if (VIDEO_CODEC_OPTION == H264_CODEC)
typedef __packed struct _ASF_HDR_VIDE_FORMAT_DATA
{
    u32 format_data_size;       /* 0x00000045 */
    s32 image_width;            /* 0x00000000 = 0x???????? */
    s32 image_height;           /* 0x00000000 = 0x???????? */
    u16 reserved;           /* 0x0001 */
    u16 bits_per_pixel_count;       /* 0x0018 */
    u32 compression_id;         /* 0x3253344d = "2S4M" (M4S2) */
    u32 image_size;         /* 0x???????? = image_width * image_height * 3 */
    s32 horz_pixels_per_meter;      /* 0x00000000 */
    s32 vert_pixels_per_meter;      /* 0x00000000 */
    u32 colors_used_count;      /* 0x00000000 */
    u32 important_colors_count;     /* 0x00000000 */
    u8  codec_specific_data[0x0A];  /* e.g.             */
                        /* 0x00, 0x00, 0x01, 0xb0,  */
                        /* 0x01,            */
                        /* 0x00, 0x00, 0x01, 0xb5,  */
                        /* 0x09,            */
                        /* 0x00, 0x00, 0x01, 0x00,  */
                        /* 0x00, 0x00, 0x01, 0x20,  */
                        /* 0x00, 0xc8, 0x88, 0x80,  */
                        /* 0x0f, 0x50, 0xb0, 0x42,  */
                        /* 0x41, 0x41, 0x41,        */
} ASF_HDR_VIDE_FORMAT_DATA;
typedef __packed struct _ASF_HDR_VIDE_TYPE_SPECIFIC_DATA 
{
    u32 encoded_image_width;        /* 0x00000000 = 0x???????? */
    u32 encoded_image_height;       /* 0x00000000 = 0x???????? */
    u8  reserved_flags;         /* 0x02 */
    u16 format_data_size;       /* 0x0045 */
    ASF_HDR_VIDE_FORMAT_DATA        asf_hdr_vide_format_dta;
    
} ASF_HDR_VIDE_TYPE_SPECIFIC_DATA;
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
typedef __packed struct _ASF_HDR_VIDE_FORMAT_DATA
{
    u32 format_data_size;       /* 0x00000045 */
    s32 image_width;            /* 0x00000000 = 0x???????? */
    s32 image_height;           /* 0x00000000 = 0x???????? */
    u16 reserved;           /* 0x0001 */
    u16 bits_per_pixel_count;       /* 0x0018 */
    u32 compression_id;         /* 0x3253344d = "2S4M" (M4S2) */
    u32 image_size;         /* 0x???????? = image_width * image_height * 3 */
    s32 horz_pixels_per_meter;      /* 0x00000000 */
    s32 vert_pixels_per_meter;      /* 0x00000000 */
    u32 colors_used_count;      /* 0x00000000 */
    u32 important_colors_count;     /* 0x00000000 */
    u8  codec_specific_data[0x0A];  /* e.g.             */
                        /* 0x00, 0x00, 0x01, 0xb0,  */
                        /* 0x01,            */
                        /* 0x00, 0x00, 0x01, 0xb5,  */
                        /* 0x09,            */
                        /* 0x00, 0x00, 0x01, 0x00,  */
                        /* 0x00, 0x00, 0x01, 0x20,  */
                        /* 0x00, 0xc8, 0x88, 0x80,  */
                        /* 0x0f, 0x50, 0xb0, 0x42,  */
                        /* 0x41, 0x41, 0x41,        */
} ASF_HDR_VIDE_FORMAT_DATA;
typedef __packed struct _ASF_HDR_VIDE_TYPE_SPECIFIC_DATA 
{
    u32 encoded_image_width;        /* 0x00000000 = 0x???????? */
    u32 encoded_image_height;       /* 0x00000000 = 0x???????? */
    u8  reserved_flags;         /* 0x02 */
    u16 format_data_size;       /* 0x0045 */
    ASF_HDR_VIDE_FORMAT_DATA        asf_hdr_vide_format_dta;
    
} ASF_HDR_VIDE_TYPE_SPECIFIC_DATA;
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
#if(ASF_FORMATE==0)
typedef __packed struct _ASF_HDR_VIDE_FORMAT_DATA
{
    u32 format_data_size;       /* 0x00000045 */
    s32 image_width;            /* 0x00000000 = 0x???????? */
    s32 image_height;           /* 0x00000000 = 0x???????? */
    u16 reserved;           /* 0x0001 */
    u16 bits_per_pixel_count;       /* 0x0018 */
    u32 compression_id;         /* 0x3253344d = "2S4M" (M4S2) */
    u32 image_size;         /* 0x???????? = image_width * image_height * 3 */
    s32 horz_pixels_per_meter;      /* 0x00000000 */
    s32 vert_pixels_per_meter;      /* 0x00000000 */
    u32 colors_used_count;      /* 0x00000000 */
    u32 important_colors_count;     /* 0x00000000 */
} ASF_HDR_VIDE_FORMAT_DATA;
typedef __packed struct _ASF_HDR_VIDE_TYPE_SPECIFIC_DATA 
{
    u32 encoded_image_width;        /* 0x00000000 = 0x???????? */
    u32 encoded_image_height;       /* 0x00000000 = 0x???????? */
    u8  reserved_flags;             /* 0x00000000              */    
    u16 format_data_size;       /* 0x0045 */
    ASF_HDR_VIDE_FORMAT_DATA        asf_hdr_vide_format_dta;

} ASF_HDR_VIDE_TYPE_SPECIFIC_DATA;
#endif

#if(ASF_FORMATE==1)
typedef __packed struct _ASF_HDR_VIDE_TYPE_SPECIFIC_DATA 
{
    u32 encoded_image_width;        /* 0x00000000 = 0x???????? */
    u32 encoded_image_height;       /* 0x00000000 = 0x???????? */
    u32 reserved_flags;             /* 0x00000000              */    
} ASF_HDR_VIDE_TYPE_SPECIFIC_DATA;

#endif

#endif


typedef __packed struct _ASF_HDR_AUDI_TYPE_SPECIFIC_DATA { 
    u16 codec_id_format_tag;        /* 0x0045 = 0x???? WAVE_FORMAT_G726 */
    u16 number_of_channels;     /* 0x0001 = 0x???? channel */
    u32 samples_per_second;     /* 0x00001f40 = 0x???????? sample/sec */
    u32 avg_num_of_bytes_per_sec;   /* 0x00000fa0 = 0x???????? byte/sec */
    u16 block_alignment;        /* 0x0001 = 0x???? byte */
    u16 bits_per_sample;        /* 0x0004 = 0x???? bit */
    u16 codec_specific_data_size;   /* 0x0000 */
#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
    //u8    codec_specific_data[0x00];
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    u16 wSamplesPerBlock; 
#endif
} ASF_HDR_AUDI_TYPE_SPECIFIC_DATA;

typedef __packed struct _ASF_HDR_AUDI_ERROR_CORRECTION_DATA 
{
    u8  span;               /* 0x01 */
    u16 virtual_packet_length;      /* 0x0190 */
    u16 virtual_chunk_length;       /* 0x0190 */
    u16 silence_data_length;        /* 0x0001 */
    u8  silence_data;           /* 0x00 */
} ASF_HDR_AUDI_ERROR_CORRECTION_DATA;

typedef __packed struct _ASF_HDR_VIDE_CODEC_ENTRY 
{
    u16 type;               /* 0x0001 = video codec */
    u16 codec_name_length;      /* 0x000b */
    u16 codec_name[0x0B];       /* 0x0049, 0x0053, 0x004f, 0x0020, = "ISO MPEG-4\0" */
                        /* 0x004d, 0x0050, 0x0045, 0x0047,  */
                        /* 0x002d, 0x0034, 0x0000, */
    u16 codec_description_length;   /* 0x0000 */
    //u16   codec_description[0x00];    
    u16 codec_information_length;   /* 0x0004 */
    u8  codec_information[0x04];    /* 0x4d, 0x34, 0x53, 0x32, = "M4S2" */
} ASF_HDR_VIDE_CODEC_ENTRY;

typedef __packed struct _ASF_HDR_AUDI_CODEC_ENTRY
{
    u16 type;               /* 0x0002 = audio codec */
    u16 codec_name_length;      /* 0x0006 */
    u16 codec_name[0x06];       /* 0x0047, 0x002e, 0x0037, 0x0032, = "G.723\0" */
                        /* 0x0036, 0x0000 */
    u16 codec_description_length;   /* 0x0013 */
    u16 codec_description[0x13];    /* 0x0033, 0x0032, 0x006b, 0x0062, = "32kb/s, 8kHz, Mono\0" */
                        /* 0x002f, 0x0073, 0x002c, 0x0020, */
                        /* 0x0038, 0x006b, 0x0048, 0x007a, */
                        /* 0x002c, 0x0020, 0x004d, 0x006f, */
                        /* 0x006e, 0x006f, 0x0000 */
    u16 codec_information_length;   /* 0x0002 */ 
    u8  codec_information[0x02];    /* 0x45, 0x00 = WAVE_FORMAT_G726 */
} ASF_HDR_AUDI_CODEC_ENTRY;

/*-------------------------------------------------------------------------*/
/* Data object                                 */
/*-------------------------------------------------------------------------*/
typedef __packed struct _ASF_DTA_ERROR_CORRECTION_DATA
{
    u8  error_correction_flags;     /* 0x82,    ErrorCorrectionDataLength = 2,      */
                        /*      OpaqueDataPresent = 0,          */
                        /*      ErrorCorrectionLengthType = 0,      */
                        /*      ErrorCorrectionPresent = 1,     */
    u8  error_correction_data[0x02];    /* 0x00, 0x00,  Type.Type = 0 (data is uncorrected),    */
                        /*      Type.Number = 0,            */  
                        /*      Cycle = 0,              */
} ASF_DTA_ERROR_CORRECTION_DATA;

typedef __packed struct _ASF_DTA_PAYLOAD_PARSING_INFO 
{
    u8  payload_flags;          /* 0x11,    MultiplePayloadsPresent = 1,        */
                        /*      SequenceType = 0 (X),           */
                        /*      PaddingLengthType = 2 (WORD),       */
                        /*      PacketLengthType = 0 (X),       */
                        /*      ErrorCorrectionPresent = 0,     */
    u8  property_flags;         /* 0x5d,    ReplicatedDataLength = 1 (BYTE),    */
                        /*      OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                        /*      MediaObjectNumberLengthType = 1 (BYTE), */
                        /*      StreamNumberLengthType = 1 (BYTE),  */
    //u??   packet_length;          /* 0x???????? */
    //u??   sequence;           /* 0x???????? */
    u16 padding_length;         /* 0x0000 */    
    u32 send_time;          /* 0x00000000 */
    u16 duration;           /* 0x0000 */
} ASF_DTA_PAYLOAD_PARSING_INFO;

typedef __packed struct _ASF_DTA_AUDIO_PAYLOAD 
{
    u8  stream_number;          /* 0x81 = 0x??, StreamNumber = 1, KeyFrameBit = 1, (video) */
                        /* 0x02 = 0x??, StreamNumber = 2, KeyFrameBit = 0, (audio) */
    u8  media_object_number;        /* 0x00 = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video) */
                        /* 0x00 = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio) */
    u32 offset_into_media_object;   /* 0x00000000, byte offset of Vn/An corresponding to start of this payload */
                        /*         non-zero when Vn/An across multiple payloads            */
    u8  replicated_data_length;     /* 0x08 */
    u64 replicated_data;        /* {0x????????, 0x????????}                              */ 
                        /*  = {size of Vn/An, byte offset of of Vn/An}               */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads */
    u16 payload_length;         /* 0x????, = size of this payload                                */ 
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads */
    //u8    payload_data[0x????????];   
} ASF_DTA_AUDIO_PAYLOAD;

typedef __packed struct _ASF_DTA_VIDEO_PAYLOAD 
{
    u8  stream_number;          /* 0x81 = 0x??, StreamNumber = 1, KeyFrameBit = 1, (video) */
                        /* 0x02 = 0x??, StreamNumber = 2, KeyFrameBit = 0, (audio) */
    u8  media_object_number;        /* 0x00 = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video) */
                        /* 0x00 = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio) */
    u32 offset_into_media_object;   /* 0x00000000, byte offset of Vn/An corresponding to start of this payload */
                        /*         non-zero when Vn/An across multiple payloads            */
    u8  replicated_data_length;     /* 0x08 */
    u64 replicated_data;        /* {0x????????, 0x????????}                              */ 
                        /*  = {size of Vn/An, byte offset of of Vn/An}               */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads */
	u8 flag;			/* asf_payload_extension_system_content_type */ 
    u16 payload_length;         /* 0x????, = size of this payload                                */ 
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads */
    //u8    payload_data[0x????????];   
} ASF_DTA_VIDEO_PAYLOAD;



typedef __packed struct _ASF_DTA_AUDIO_PAYLOAD_SINGLE 
{
    u8  stream_number;          /* 0x81 = 0x??, StreamNumber = 1, KeyFrameBit = 1, (video) */
                        /* 0x02 = 0x??, StreamNumber = 2, KeyFrameBit = 0, (audio) */
    u8  media_object_number;        /* 0x00 = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video) */
                        /* 0x00 = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio) */
    u32 offset_into_media_object;   /* 0x00000000, byte offset of Vn/An corresponding to start of this payload */
                        /*         non-zero when Vn/An across multiple payloads            */
    u8  replicated_data_length;     /* 0x08 */
    u64 replicated_data;        /* {0x????????, 0x????????}                              */ 
                        /*  = {size of Vn/An, byte offset of of Vn/An}               */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads */
    //u16 payload_length;         /* 0x????, = size of this payload                                */ 
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads */
    //u8    payload_data[0x????????];   
} ASF_DTA_AUDIO_PAYLOAD_SINGLE;

typedef __packed struct _ASF_DTA_VIDEO_PAYLOAD_SINGLE 
{
    u8  stream_number;          /* 0x81 = 0x??, StreamNumber = 1, KeyFrameBit = 1, (video) */
                        /* 0x02 = 0x??, StreamNumber = 2, KeyFrameBit = 0, (audio) */
    u8  media_object_number;        /* 0x00 = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video) */
                        /* 0x00 = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio) */
    u32 offset_into_media_object;   /* 0x00000000, byte offset of Vn/An corresponding to start of this payload */
                        /*         non-zero when Vn/An across multiple payloads            */
    u8  replicated_data_length;     /* 0x08 */
    u64 replicated_data;        /* {0x????????, 0x????????}                              */ 
                        /*  = {size of Vn/An, byte offset of of Vn/An}               */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads */
	u8 flag;			/* asf_payload_extension_system_content_type */
    //u16 payload_length;         /* 0x????, = size of this payload                                */ 
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads */
    //u8    payload_data[0x????????];   
} ASF_DTA_VIDEO_PAYLOAD_SINGLE;

typedef __packed struct _ASF_DTA_PAYLOAD_SUBSEQ 
{
    u8  stream_number;          /* 0x81 = 0x??, StreamNumber = 1, KeyFrameBit = 1, (video) */
                        /* 0x02 = 0x??, StreamNumber = 2, KeyFrameBit = 0, (audio) */
    u8  media_object_number;        /* 0x00 = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video) */
                        /* 0x00 = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio) */
    u32 offset_into_media_object;   /* 0x00000000, byte offset of Vn/An corresponding to start of this payload */
                        /*         non-zero when Vn/An across multiple payloads            */
    u8  replicated_data_length;     /* 0x00 */
    u16 payload_length;         /* 0x????, = size of this payload                                */ 
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads */
    //u8    payload_data[0x????????];   
} ASF_DTA_PAYLOAD_SUBSEQ;

typedef __packed struct _ASF_DTA_M_PAYLOAD_DATA
{
    u8  payload_flags;          /* 0x80,    NumberOfPayloads,       */
                        /*      PayloadLengthType = 2 (WORD),   */
    //ASF_DTA_PAYLOAD_PARSING_INFO      /* asf_dta_payld[0x??] */  
} ASF_DTA_M_PAYLOAD_DATA;



typedef __packed struct _ASF_DTA_DATA_PACKET {
    ASF_DTA_ERROR_CORRECTION_DATA       asf_dta_error_correction_dta;
    ASF_DTA_PAYLOAD_PARSING_INFO        asf_dta_payload_parsing_inf;
    ASF_DTA_M_PAYLOAD_DATA          asf_dta_m_payload_dta;
    //ASF_DTA_PADDING_DATA          asf_dta_padding_dta;
} ASF_DTA_DATA_PACKET;

typedef __packed struct _ASF_DATA_PACKET_MULTIPAYLOAD{
    ASF_DTA_ERROR_CORRECTION_DATA       asf_dta_error_correction_dta;
    ASF_DTA_PAYLOAD_PARSING_INFO        asf_dta_payload_parsing_inf;
    ASF_DTA_M_PAYLOAD_DATA          asf_dta_m_payload_dta;
    //ASF_DTA_PADDING_DATA          asf_dta_padding_dta;
} ASF_DATA_PACKET_MULTIPAYLOAD;

typedef __packed struct _ASF_DATA_PACKET_SINGLEPAYLOAD{
    ASF_DTA_ERROR_CORRECTION_DATA       asf_dta_error_correction_dta;
    ASF_DTA_PAYLOAD_PARSING_INFO        asf_dta_payload_parsing_inf;
    //ASF_DTA_M_PAYLOAD_DATA          asf_dta_m_payload_dta;
    //ASF_DTA_PADDING_DATA          asf_dta_padding_dta;
} ASF_DATA_PACKET_SINGLEPAYLOAD;

/*-------------------------------------------------------------------------*/
/* Index object                                */ 
/*-------------------------------------------------------------------------*/
typedef __packed struct _ASF_IDX_SIMPLE_INDEX_ENTRY 
{
    u32 packet_number;          /* 0x00000000, = the closest key frame (I-VOP) prior to current time interval */
    u16 packet_count;           /* 0x0000, = number of packets spans for the closest key frame (I-VOP). */
} ASF_IDX_SIMPLE_INDEX_ENTRY;

/***************************************************************************
 * Object definition
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
/* Header object                               */   
/*-------------------------------------------------------------------------*/
typedef __packed struct _ASF_HEADER_OBJECT
{
    u128    object_id;          /* {0x30, 0x26, 0xb2, 0x75,                 */
                        /*  0x8e, 0x66, 0xcf, 0x11,             */
                        /*  0xa6, 0xd9, 0x00, 0xaa,                 */
                        /*  0x00, 0x62, 0xce, 0x6c} = ASF_Header_Object */
    u64 object_size;            /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u32 number_of_header_object;    /* 0x00000007, (video) */
                        /* 0x00000008, (video and audio) */
    u8  reserved1;          /* 0x01 */
    u8  reserved2;          /* 0x02 */
} ASF_HEADER_OBJECT;

typedef __packed struct _ASF_FILE_PROPERTIES_OBJECT
{
    u128    object_id;          /* {0xa1, 0xdc, 0xab, 0x8c,                     */ 
                        /*  0x47, 0xa9, 0xcf, 0x11,                     */
                        /*  0x8e, 0xe4, 0x00, 0xC0,                     */
                        /*  0x0c, 0x20, 0x53, 0x65} = ASF_File_Properties_Object    */
    u64 object_size;            /* {0x00000068, 0x00000000} */
    u128    file_id;            /* {0x00, 0x00, 0x00, 0x00,                                 */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00} = file_id of asf_data_object and asf_simple_index_object    */
    u64 file_size;          /* {0x00000000, 0x00000000}, = 0x???????????????? */    
    u64 creation_date;          /* {0xb22f4000, 0x01c3d255}, = 00:00:00 2004/01/01 */
    u64 data_packets_count;     /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u64 play_duration;          /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u64 send_duration;          /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u64 preroll;            /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u32 flags;              /* 0x00000002, BroadcastFlag = 0, SeekableFlag = 1 */ 
    u32 minimum_data_packet_size;   /* 0x00008000 */
    u32 maximum_data_packet_size;   /* 0x00008000 */
    u32 maximum_bitrate;        /* 0x00012c00 = 384K * 2 */
} ASF_FILE_PROPERTIES_OBJECT;

typedef __packed struct _ASF_FILE_PROPERTIES_OBJECT_POST
{
    u64 file_size;          /* {0x00000000, 0x00000000}, = 0x???????????????? */    
    u64 creation_date;          /* {0xb22f4000, 0x01c3d255}, = 00:00:00 2004/01/01 */
    u64 data_packets_count;     /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u64 play_duration;          /* {0x00000000, 0x00000000}, = 0x???????????????? */
    u64 send_duration;          /* {0x00000000, 0x00000000}, = 0x???????????????? */
} ASF_FILE_PROPERTIES_OBJECT_POST;

typedef __packed struct _ASF_VIDE_STREAM_PROPERTIES_OBJECT 
{
    u128    object_id;          /* {0x91, 0x07, 0xdc, 0xb7,                 */ 
                        /*  0xb7, 0xa9, 0xcf, 0x11,                 */
                        /*  0x8e, 0xe6, 0x00, 0xc0,                     */
                        /*  0x0c, 0x20, 0x53, 0x65} = ASF_Stream_Properties_Object  */
    u64 object_size;            /* {0x0000009e, 0x00000000} */
    u128    steam_type;         /* {0xc0, 0xef, 0x19, 0xbc,         */   
                        /*  0x4d, 0x5b, 0xcf, 0x11,         */
                        /*  0xa8, 0xfd, 0x00, 0x80,         */
                        /*  0x5f, 0x5c, 0x44, 0x2b} = ASF_Video_Media   */
    u128    error_correction_type;      /* {0x00, 0x57, 0xfb, 0x20,             */   
                        /*  0x55, 0x5b, 0xcf, 0x11,             */
                        /*  0xa8, 0xfd, 0x00, 0x80,             */
                        /*  0x5f, 0x5c, 0x44, 0x2b} = ASF_No_Error_Correction   */
    u64 time_offset;            /* {0x00000000, 0x00000000} */
    u32 type_specific_data_length;  /* 0x00000050 */
    u32 error_correction_data_length;   /* 0x00000000 */
    u16 flags;              /* 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
    u32 reserved;           /* 0x00000000 */
    ASF_HDR_VIDE_TYPE_SPECIFIC_DATA     asf_hdr_vide_type_specific_dta;
    //ASF_HDR_VIDE_ERROR_CORRECTION_DATA    asf_hdr_vide_error_correction_dta;
} ASF_VIDE_STREAM_PROPERTIES_OBJECT;

typedef __packed struct _ASF_AUDI_STREAM_PROPERTIES_OBJECT 
{
    u128    object_id;          /* {0x91, 0x07, 0xdc, 0xb7,                     */ 
                        /*  0xb7, 0xa9, 0xcf, 0x11,                     */
                        /*  0x8e, 0xe6, 0x00, 0xc0,                     */ 
                        /*  0x0c, 0x20, 0x53, 0x65} = ASF_Stream_Properties_Object  */
    u64 object_size;            /* {0x00000068, 0x00000000} */
    u128    stream_type;            /* {0x40, 0x9e, 0x69, 0xf8,             */
                        /*  0x4d, 0x5b, 0xcf, 0x11,         */
                        /*  0xa8, 0xfd, 0x00, 0x80,         */
                        /*  0x5f, 0x5c, 0x44, 0x2b} = ASF_Audio_Media   */
    u128    error_correction_type;      /* {0x50, 0xcd, 0xc3, 0xbf,             */
                        /*  0x8f, 0x61, 0xcf, 0x11,         */
                        /*  0x8b, 0xb2, 0x00, 0xaa,         */  
                        /*  0x00, 0xb4, 0xe2, 0x20} = ASF_Audio_Spread  */
    u64 time_offset;            /* {0x00000000, 0x00000000} */  
    u32 type_specific_data_length;  /* 0x00000012 */
    u32 error_correction_data_length;   /* 0x00000008 */
    u16 flags;              /* 0x0002, StreamNumber = 2, EncryptedContentFlag = 0 */
    u32 reserved;           /* 0x00000000 */
    ASF_HDR_AUDI_TYPE_SPECIFIC_DATA     asf_hdr_audi_type_specific_dta;
    ASF_HDR_AUDI_ERROR_CORRECTION_DATA  asf_hdr_audi_error_correction_dta;
} ASF_AUDI_STREAM_PROPERTIES_OBJECT;

typedef __packed struct _ASF_HEADER_EXTENSION_OBJECT
{
    u128    object_id;          /* {0xb5, 0x03, 0xbf, 0x5f,                     */
                        /*  0x2e, 0xa9, 0xcf, 0x11,                 */
                        /*  0x8e, 0xe3, 0x00, 0xc0,                     */
                        /*  0x0c, 0x20, 0x53, 0x65} = ASF_Header_Extension_Object   */ 
    u64 object_size;            /* {0x0000002e, 0x00000000} */
    u128    reserved_field_1;       /* {0x11, 0xd2, 0xd3, 0xab,             */
                        /*  0xba, 0xa9, 0xcf, 0x11,         */
                        /*  0x8E, 0xE6, 0x00, 0xC0,             */
                        /*  0x0C, 0x20, 0x53, 0x65} = ASF_Reserved_1    */
    u16 reserved_field_2;       /* 0x0006 */
    u32 header_extension_data_size; /* 0x00000000 */
    //u8     header_extension_data[0x00];
} ASF_HEADER_EXTENSION_OBJECT;

typedef __packed struct _ASF_CODEC_LIST_OBJECT {
    u128    object_id;          /* {0x40, 0x52, 0xD1, 0x86,             */ 
                        /*  0x1D, 0x31, 0xD0, 0x11,             */
                        /*  0xA3, 0xA4, 0x00, 0xA0,                 */
                        /*  0xC9, 0x03, 0x48, 0xF6} = ASF_Codec_List_Object */
    u64 object_size;            /* {0x0000004E, 0x00000000}, (video only) */        
                        /* {0x0000008A, 0x00000000}, (video and audio) */
    u128    reserved;           /* {0x41, 0x52, 0xD1, 0x86,             */
                        /*  0x1D, 0x31, 0xD0, 0x11,         */
                        /*  0xA3, 0xA4, 0x00, 0xA0,             */
                        /*  0xC9, 0x03, 0x48, 0xF6} = ASF_Reserved_2    */
    u32 codec_entries_count;        /* 0x00000001, (video only) */  
                        /* 0x00000002, (video and audio) */
    //ASF_HDR_VIDE_CODEC_ENTRY      asf_hdr_vide_codec_ent;
    //ASF_HDR_AUDI_CODEC_ENTRY      asf_hdr_audi_codec_ent;
} ASF_CODEC_LIST_OBJECT;

typedef __packed struct _ASF_CONTENT_DESCRIPTION_OBJECT
{
    u128    object_id;          /* {0x33, 0x26, 0xB2, 0x75,                 */ 
                        /*  0x8E, 0x66, 0xCF, 0x11,                 */
                        /*  0xA6, 0xD9, 0x00, 0xAA,                 */   
                        /*  0x00, 0x62, 0xCE, 0x6C} = ASF_Content_Description_Object    */
    u64 object_size;            /* {0x00000062, 0x00000000} */
    u16 title_length;           /* 0x0000 */
    u16 author_length;          /* 0x0000 */
    u16 copyright_length;       /* 0x0000 */
    u16 description_length;     /* 0x0040 */
    u16 rating_length;          /* 0x0000 */

    u16 description[0x20];      /* 0x0048, 0x0049, 0x004d, 0x0041, = "Mars PA9001\0..."    */
                        /* 0x0058, 0x0020, 0x0050, 0x0041,              */
                        /* 0x0039, 0x0030, 0x0030, 0x0031,              */
                        /* 0x0000, 0x0000, 0x0000, 0x0000,              */
                        /* 0x0000, 0x0000, 0x0000, 0x0000,              */
                        /* 0x0000, 0x0000, 0x0000, 0x0000,              */
                        /* 0x0000, 0x0000, 0x0000, 0x0000,              */
                        /* 0x0000, 0x0000, 0x0000, 0x0000,              */
} ASF_CONTENT_DESCRIPTION_OBJECT;

typedef __packed struct _ASF_HDR_PADDING_OBJECT {
    u128    object_id;          /* {0x74, 0xD4, 0x06, 0x18,                 */
                        /*  0xDF, 0xCA, 0x09, 0x45,             */
                        /*  0xA4, 0xBA, 0x9A, 0xAB,                 */
                        /*  0xCB, 0x96, 0xAA, 0xE8} = ASF_Padding_Object    */
    u64 object_size;            /* {0x00000000, 0x00000000} = 0x???????????????? byte */
    //u8    padding_data[0x????????????????];
} ASF_HDR_PADDING_OBJECT;
/*-------------------------------------------------------------------------*/
/* Header Extension object                               */   
/*-------------------------------------------------------------------------*/ 
typedef __packed struct _ASF_HDR_EXT_STREAM_PROPERITY
{
	u128 object_id;  /* {0xCB, 0xA5, 0xE6, 0x14,                 */
                        /*  0x72, 0xC6, 0x32, 0x43,             */
                        /*  0x83, 0x99, 0xA9, 0x69,                 */
                        /*  0x52, 0x06, 0x5B, 0x5A} = ASF_Extended_Stream_Properties_Object    */
	u64 object_size;            /* {0x????????, 0x????????} = 0x???????????????? */
	u64 start_time;
	u64 end_time;
	u32 data_bitrate;
	u32 buffer_size;
	u32 initial_buffer_fullness;
	u32 alternate_data_bitrate;
	u32 alternate_buffer_size;
	u32 alternate_initial_buffer_fullness;
	u32 max_object_size;
	u32 flag;
	u16 stream_num;
	u16 stream_language;
	u64 average_time_per_frame;
	u16 stream_name_count;
	u16 payload_ext_sys_cnt;
	
}ASF_HDR_EXT_STREAM_PROPERTY;

typedef __packed struct _ASF_PAYLOAD_EXT_SYSTEMS
{
	u128 object_id;  	/* {0x20, 0xDC, 0x90, 0xD5,                 */
                        /*  0xBC, 0x07, 0x6C, 0x43,             */
                        /*  0x9C, 0xF7, 0xF3, 0xBB,                 */
                        /*  0xFB, 0xF1, 0xA4, 0xDC} = ASF_Payload_Extension_System_Content_Type    */
	u16 data_size;            /* {0x????????, 0x????????} = 0x???????????????? */
	u32 info_length;
}ASF_PAYLOAD_EXT_SYSTEMS;
/*-------------------------------------------------------------------------*/
/* Data object                                 */   
/*-------------------------------------------------------------------------*/
typedef __packed struct _ASF_DATA_OBJECT 
{
    u128    object_id;          /* {0x36, 0x26, 0xB2, 0x75,             */
                        /*  0x8E, 0x66, 0xCF, 0x11,         */
                        /*  0xA6, 0xD9, 0x00, 0xAA,             */
                        /*  0x00, 0x62, 0xCE, 0x6C} = ASF_Data_Object   */
    u64 object_size;            /* {0x????????, 0x????????} = 0x???????????????? */ 
    u128    file_id;            /* {0x00, 0x00, 0x00, 0x00,                                 */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00} = file_id of asf_data_object and asf_simple_index_object    */
    u64 total_data_packets;     /* {0x00000000, 0x00000000} = 0x???????????????? */
    u16 reserved;           /* 0x0101 */
    //ASF_DTA_DATA_PACKET           asf_dta_data_pkt[0x????????????????];
} ASF_DATA_OBJECT;  

typedef __packed struct _ASF_HEADER
{
    u128    object_id;
    u64     object_size;            /* {0x????????, 0x????????} = 0x???????????????? */ 
} ASF_HEADER;   

/*-------------------------------------------------------------------------*/
/* Index object                                */   
/*-------------------------------------------------------------------------*/
typedef __packed struct _ASF_SIMPLE_INDEX_OBJECT {
    u128    object_id;          /* {0x90, 0x08, 0x00, 0x33,             */ 
                        /*  0xB1, 0xE5, 0xCF, 0x11,             */
                        /*  0x89, 0xF4, 0x00, 0xA0,                 */
                        /*  0xC9, 0x03, 0x49, 0xCB} = ASF_Simple_Index_Object   */
    u64 object_size;            /* {0x00000000, 0x00000000} = 0x???????????????? */
    u128    file_id;            /* {0x00, 0x00, 0x00, 0x00,                                 */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00,                             */
                        /*  0x00, 0x00, 0x00, 0x00} = file_id of asf_data_object and asf_simple_index_object    */
    u64 index_entry_time_interval;  /* {0x00989680, 0x00000000} = 10000000 100-nanosecond = 1 second */
    u32 max_packet_count;       /* 0x00000005 = maximum 5 packets (1 sec) for each index_entry */
    u32 index_entry_count;      /* 0x???????? */
    //ASF_IDX_SIMPLE_INDEX_ENTRY        asf_idx_simple_index_ent[0x????????];
} ASF_SIMPLE_INDEX_OBJECT;






#define NoMatch_Guid                    0
//Top-level ASF Object GUIDS
#define ASF_Header_Object_Guid          1
#define ASF_Data_Object_Guid            2
#define ASF_Simple_Index_Object_Guid    3
#define ASF_Index_Object_Guid           4
#define ASF_Media_Object_Guid           5
#define ASF_Timecode_Index_Guid     6

//Header Object GUIDS
#define ASF_File_Properties_Object_GUID     7
#define ASF_Stream_Properties_Object_GUID   8
#define ASF_Header_Extension_Object_GUID    9
#define ASF_Codec_List_Object_GUID          0xA
#define ASF_Content_Description_Object_GUID 0xB
#define ASF_Padding_Object_GUID         0xC


// Stream Type GUIDS
#define ASF_Audio_Media_GUID    0x10
#define ASF_Video_Media_GUID    0x11

// for NEWKEN NK2400
//#define ASF_SIZE_PER_FILE       3145728    // 3M Bytes per asf file
#define ASF_SIZE_PER_FILE       10485760    // 10M Bytes per asf file
//#define ASF_SIZE_PER_FILE       20971520    // 20M Bytes per asf file

#define ASF_REC_TIME_LEN	150


#endif
