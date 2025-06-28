/*

Copyright (c) 2012 Mars Semiconductor Corp.

Module Name:

    MultiChannelASF.c

Abstract:

    The routines of multiple channel asf file format packer.

Environment:

        ARM RealView Developer Suite

Revision History:

    2012/05/14  Peter Hsu  Create

*/

#include "general.h"

#if MULTI_CHANNEL_VIDEO_REC

#include "general.h"
#include "board.h"
#include "task.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "asf.h"
#include "asfapi.h"
#include "mpeg4api.h"
#include "iisapi.h"
#include "../../iis/inc/iis.h"  /* Peter 070104 */
#include "iduapi.h" /* BJ: 0718 */
#include "isuapi.h" /* BJ: 0718 */
#include "ipuapi.h"    /* Peter 070104 */
#include "siuapi.h"    /* Peter 070104 */
#include "sysapi.h"
#include "timerapi.h"

#include "osapi.h"
#include "gpioapi.h"
#include "adcapi.h"
#include "dmaapi.h"
#include "uiapi.h"
#include "rfiuapi.h"
#include "../../ui/inc/ui_project.h"
#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
#include "jpegapi.h" //Lsk : 090312
#elif (VIDEO_CODEC_OPTION == H264_CODEC)
#include "VideoCodecAPI.h" //Lsk : 090312
#endif
#include "ciuapi.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif
#include "GlobalVariable.h"
#if TUTK_SUPPORT
#include "p2pserver_api.h"
#if LWIP2_SUPPORT
#include "../LwIP_2.0/include/tutk_P2P/AVIOCTRLDEFs.h"
#else
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif
#endif

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

//#undef  ASF_AUDIO
#define DUMMY_FRAME_DURATION    100 //ms

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

static FS_DISKFREE_T   *diskInfo;  //Lsk 090715
//static u32             free_size;
//static u32             bytes_per_cluster;

#if MULTI_CH_SDC_WRITE_TEST
__align(16) u8  TestBuf[4][0x8000];
#endif

#if ASF_DEBUG_ENA
u32 RX_time_A = 0, RX_time_V = 0 ;
u32 RX_skip_A = 0, RX_skip_V = 0;
u32 RX_sem_A = 0, RX_sem_V = 0;
u32 ASF_sem_A = 0, ASF_sem_V = 0;
u32 skip_I=0, skip_P=0, skip_A=0;
#endif

#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
u8 V_flag[MULTI_CHANNEL_MAX],A_flag[MULTI_CHANNEL_MAX];
u32 Audio_RF_index[MULTI_CHANNEL_MAX] = {0,0,0,0};
u32 Video_RF_index[MULTI_CHANNEL_MAX] = {0,0,0,0};
#endif

#if VIDEO_STARTCODE_DEBUG_ENA
int monitor_RX[MAX_RFIU_UNIT];
int monitor_decode[MAX_RFIU_UNIT];
int monitor_ASF_1[MAX_RFIU_UNIT];
int monitor_ASF_2[MAX_RFIU_UNIT];
#endif


/*
 *********************************************************************************************************
 * Extern Varaibel
 *********************************************************************************************************
 */

extern u32 sysVideoInSel;
extern u32 EventTrigger;  //用於Buffer moniting.
extern u32 asfVopCount;   //用於Buffer moniting.

extern s32 mp4_avifrmcnt, isu_avifrmcnt;
extern u32 IsuIndex;

extern u8* mpeg4outputbuf[3];
extern u8  sysCaptureVideoStop;
extern u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */
extern u32 isu_int_status;
extern BOOLEAN MemoryFullFlag;
extern s32 asfRecFileNum;         // less than 0: infinite, more than 0: video file number want to record?

// rfiu
extern VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM];
extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern IIS_BUF_MNG rfiuRxIIsSounBufMng[MAX_RFIU_UNIT][IIS_BUF_NUM];
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufPlay_idx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufFill_idx[MAX_RFIU_UNIT];
extern u8 *rfiuMainAudioPlayDMANextBuf[IISPLAY_BUF_NUM];

extern u8  dcfFileName[MULTI_CHANNEL_MAX][32];

extern s8  isPIRsenSent[MAX_RFIU_UNIT];
#if INSERT_NOSIGNAL_FRAME
extern u8 Record_flag[MULTI_CHANNEL_MAX];
#endif

/*
 *********************************************************************************************************
 * External Function
 *********************************************************************************************************
 */



/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if MULTI_CH_SDC_WRITE_TEST

s32 TestFileSystem(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    FS_FILE*    pFile;
    int         i, j, k, l, m;
    s8          newFileName[32];
    u8          *pTestBuf;
    u32         size;
    u8  tmp;
	
    DEBUG_ASF("TestFileSystem(%d) begin...\n", pVideoClipOption->VideoChannelID);

    //for(m = 0; m < 10 && sysCaptureVideoStop == 0; m++)
    while(pVideoClipOption->sysCaptureVideoStop == 0)
    {
    #if 1
        if ((pVideoClipOption->pFile = dcfCreateNextFile(DCF_FILE_TYPE_ASF, pVideoClipOption->VideoChannelID)) == NULL) {
            DEBUG_ASF("Ch%d ASF create file error!!!\n", pVideoClipOption->VideoChannelID);
            return 0;
        }
    #else
        sprintf((char *)newFileName, "Test%04d.txt", pVideoClipOption->VideoChannelID);
        if ((pFile = dcfOpen(newFileName, "w")) == NULL) // "w-": Create file 前不scan FDB.
        {	/* create next file error */
            DEBUG_ASF("Create file %s fail.\n", newFileName);
            return NULL;
        }
        DEBUG_ASF("Create file %s success.\n", newFileName);
    #endif

        j           = sizeof(TestBuf[pVideoClipOption->VideoChannelID]) / 128;
        k           = 0;
        for(l = 0; l < 100 && pVideoClipOption->sysCaptureVideoStop == 0; l++)
        {
            pTestBuf    = (char *)TestBuf[pVideoClipOption->VideoChannelID];
            memset(pTestBuf, ' ', sizeof(TestBuf[pVideoClipOption->VideoChannelID]));
            OSTimeDly(1);
            for(i = 0; i < j; i++, k++)
            {
                sprintf(pTestBuf, "\r\nCh%d %s line %5d", pVideoClipOption->VideoChannelID, &dcfFileName[pVideoClipOption->VideoChannelID][0], k);
                pTestBuf   += 128;
                if((i & 0x3f) == 0)
                    OSTimeDly(1);
            }
            OSTimeDly(1);
            if(dcfWrite(pVideoClipOption->pFile, (char *)TestBuf[pVideoClipOption->VideoChannelID], sizeof(TestBuf[pVideoClipOption->VideoChannelID]), &size) == 0)
            {
                DEBUG_ASF("Ch%d TestFileSystem write error!!!\n", pVideoClipOption->VideoChannelID);
                DEBUG_ASF("Ch%d TestFileSystem close file!!!\n", pVideoClipOption->VideoChannelID);
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                return 0;
            }
            //DEBUG_ASF(" %d ", pVideoClipOption->VideoChannelID);
            OSTimeDly(1);
        }
        DEBUG_ASF("Ch%d TestFileSystem close file!!!\n", pVideoClipOption->VideoChannelID);
        //dcfClose(pFile);
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
    }

    DEBUG_ASF("TestFileSystem Ch%d finish!!!\n", pVideoClipOption->VideoChannelID);
    return 1;
}
#endif  // #if 0


/*-------------------------------------------------------------------------*/
/* Header object                               */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Multiple channel write header object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 MultiChannelAsfWriteHeaderObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    u32 hdrSize;
    __align(4) ASF_HEADER_OBJECT  asfHeaderObject =
    {
        {0x30, 0x26, 0xb2, 0x75,    /* object_id = ASF_Header_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? */
        0x00000007,         /* number_of_header_object = 0x00000007 (video only)        */
                        /*               0x00000008 (video and audio)   */
        0x01,               /* reserved1 = 0x01 */
        0x02,               /* reserved2 = 0x02 */
    };

    #if (VIDEO_CODEC_OPTION == H264_CODEC)
    pVideoClipOption->asfVideHeaderSize = 0;
    #elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    MultiChannelMpeg4EncodeVolHeader(pVideoClipOption);
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    pVideoClipOption->asfVideHeaderSize = 0;
    #endif

    hdrSize =
        sizeof(ASF_HEADER_OBJECT) +
        sizeof(ASF_FILE_PROPERTIES_OBJECT) +
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + pVideoClipOption->asfVideHeaderSize +
#ifdef ASF_AUDIO
        sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT) +
#endif
        #if( (VIDEO_CODEC_OPTION == H264_CODEC)||(VIDEO_CODEC_OPTION == MPEG4_CODEC))
        sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS) + //Lsk 090515
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
        sizeof(ASF_HEADER_EXTENSION_OBJECT) +
        #endif
        sizeof(ASF_CODEC_LIST_OBJECT) +
        sizeof(ASF_HDR_VIDE_CODEC_ENTRY) +
#ifdef ASF_AUDIO
        sizeof(ASF_HDR_AUDI_CODEC_ENTRY) +
#endif
        sizeof(ASF_CONTENT_DESCRIPTION_OBJECT) +
        sizeof(ASF_HDR_PADDING_OBJECT);
    pVideoClipOption->asfHeaderPaddingSize = ASF_HEADER_SIZE_CEIL - hdrSize - sizeof(ASF_DATA_OBJECT);
    asfHeaderObject.object_size.lo =
        pVideoClipOption->asfHeaderSize =
        hdrSize + pVideoClipOption->asfHeaderPaddingSize;
#ifdef ASF_AUDIO
    asfHeaderObject.number_of_header_object = 0x00000007;
#else
    asfHeaderObject.number_of_header_object = 0x00000006;
#endif

#if ASF_MASS_WRITE_HEADER
    CopyMemory(pVideoClipOption->asfMassWriteData, &asfHeaderObject, sizeof(ASF_HEADER_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint   = pVideoClipOption->asfMassWriteData + sizeof(ASF_HEADER_OBJECT);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderObject, sizeof(ASF_HEADER_OBJECT), &size) == 0)
            return 0;
#endif

    if (MultiChannelAsfWriteFilePropertiesObjectPre(pVideoClipOption) == 0)
            return 0;
    if (MultiChannelAsfWriteVideStreamPropertiesObject(pVideoClipOption) == 0)
            return 0;
#ifdef ASF_AUDIO
    if (MultiChannelAsfWriteAudiStreamPropertiesObject(pVideoClipOption) == 0)
            return 0;
#endif
    if (MultiChannelAsfWriteHeaderExtensionObject(pVideoClipOption) == 0)
            return 0;
    if (MultiChannelAsfWriteCodecListObject(pVideoClipOption) == 0)
            return 0;
    if (MultiChannelAsfWriteContentDescriptionObject(pVideoClipOption) == 0)
            return 0;
    if (MultiChannelAsfWriteHdrPaddingObject(pVideoClipOption) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Multiple channel write file properties object pre.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteFilePropertiesObjectPre(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject =
    {
        {0xa1, 0xdc, 0xab, 0x8c,    /* object_id = ASF_File_Properties_Object */
         0x47, 0xa9, 0xcf, 0x11,
         0x8e, 0xe4, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x00000068, 0x00000000},   /* object_size = 0x0000000000000068 */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0x00000000, 0x00000000},   /* file_size = 0x???????????????? */
        {0xffffffff, 0xffffffff},   /* creation_date = 0x???????????????? */
        {0x00000000, 0x00000000},   /* data_packets_count= 0x???????????????? */
        {0x00000000, 0x00000000},   /* play_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000000, 0x00000000},   /* send_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000bb8, 0x00000000},   /* preroll = 0x0bb8000000000000 */  //Lsk : preroll 3sec
        0x00000002,         /* flags = 0x02, BroadcastFlag = 0, SeekableFlag = 1 */
        0x00000800,         /* minimum_data_packet_size = 0x???????? */
        0x00000800,         /* maximum_data_packet_size = 0x???????? */
        //0x00012c00,           /* maximum_bitrate = 384K * 2 */
        0x002547AE,         /* maximum_bitrate = 0 kbps, for Media Player 6.4 */ /* Peter 070104 */
    };

    asfFilePropertiesObject.minimum_data_packet_size =
        asfFilePropertiesObject.maximum_data_packet_size =
            ASF_DATA_PACKET_SIZE;
    asfFilePropertiesObject.object_size.lo = sizeof(ASF_FILE_PROPERTIES_OBJECT);

#if ASF_MASS_WRITE_HEADER
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfFilePropertiesObject, sizeof(ASF_FILE_PROPERTIES_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_FILE_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfFilePropertiesObject, sizeof(ASF_FILE_PROPERTIES_OBJECT), &size) == 0)
            return 0;
#endif

    return 1;
}

/*

Routine Description:

    Multiple channel write file properties object post.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteFilePropertiesObjectPost(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    u32 offset;
    __align(4) ASF_FILE_PROPERTIES_OBJECT_POST asfFilePropertiesObjectPost =
    {
        {0x00000000, 0x00000000},   /* file_size = 0x???????????????? */
        {0xb22f4000, 0x01c3d255},   /* creation_date = 00:00:00 2004/01/01 */
        {0x00000000, 0x00000000},   /* data_packets_count= 0x???????????????? */
        {0x00000000, 0x00000000},   /* play_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000000, 0x00000000},   /* send_duration = 0x???????????????? in unit of 100 nanosecond */
    };

    offset = dcfTell(pVideoClipOption->pFile);

    dcfSeek(pVideoClipOption->pFile, 0x46, FS_SEEK_SET);

    pVideoClipOption->asfDataSize =
        sizeof(ASF_DATA_OBJECT) +
        pVideoClipOption->asfDataPacketCount * ASF_DATA_PACKET_SIZE;
    pVideoClipOption->asfIndexSize =
        sizeof(ASF_SIMPLE_INDEX_OBJECT) +
        pVideoClipOption->asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY);
    asfFilePropertiesObjectPost.file_size.lo = pVideoClipOption->asfHeaderSize + pVideoClipOption->asfDataSize + pVideoClipOption->asfIndexSize;
    asfFilePropertiesObjectPost.data_packets_count.lo = pVideoClipOption->asfDataPacketCount;
    //Lsk 090309
    asfFilePropertiesObjectPost.play_duration.lo = pVideoClipOption->asfVidePresentTime * 10000;
    asfFilePropertiesObjectPost.play_duration.hi = ((pVideoClipOption->asfVidePresentTime >> 4) * (10000 >> 4)) >> 24;

    asfFilePropertiesObjectPost.send_duration.lo = (pVideoClipOption->asfVidePresentTime - PREROLL) * 10000;
    asfFilePropertiesObjectPost.send_duration.hi = (((pVideoClipOption->asfVidePresentTime - PREROLL) >> 4) * (10000 >> 4)) >> 24;

    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfFilePropertiesObjectPost, sizeof(ASF_FILE_PROPERTIES_OBJECT_POST), &size) == 0)
        return 0;

    dcfSeek(pVideoClipOption->pFile, offset, FS_SEEK_SET);

    return 1;
}


/*

Routine Description:

    Multiple channel write video stream properties object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if (VIDEO_CODEC_OPTION == H264_CODEC)
s32 MultiChannelAsfWriteVideStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000050,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */
                0x34363248,         /* compression_id = "462H" (H264) */
                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */
            },
        },
    };
    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            pVideoClipOption->asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            pVideoClipOption->asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        pVideoClipOption->asfVopWidth * pVideoClipOption->asfVopHeight * 3;

#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &pVideoClipOption->asfVideHeader, pVideoClipOption->asfVideHeaderSize);
    pVideoClipOption->asfMassWriteDataPoint  += pVideoClipOption->asfVideHeaderSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&pVideoClipOption->asfVideHeader, pVideoClipOption->asfVideHeaderSize, &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
s32 MultiChannelAsfWriteVideStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000050,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */
                0x3253344d,         /* compression_id = "2S4M" (M4S2) */
                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */
                0x00, 0x00, 0x01, 0xb0,     /* codec_specific_data[0x0a] */
                //0x01,
                0x03,                       /* Simple profile level 3 */ /* Peter 070104 */
                0x00, 0x00, 0x01, 0xb5,
                0x09,
                        /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00,       */
            },
        },
    };
    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            pVideoClipOption->asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            pVideoClipOption->asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        pVideoClipOption->asfVopWidth * pVideoClipOption->asfVopHeight * 3;

#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &pVideoClipOption->asfVideHeader, pVideoClipOption->asfVideHeaderSize);
    pVideoClipOption->asfMassWriteDataPoint  += pVideoClipOption->asfVideHeaderSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&pVideoClipOption->asfVideHeader, pVideoClipOption->asfVideHeaderSize, &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 MultiChannelAsfWriteVideStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
#if(ASF_FORMATE==0)
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        /*** steam_type test start***/
        /***  Lsk pc : media player 11 ok, media player 6.4 ok,
            other pc : fail ***/
        #if 1
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        #if 0
		{0xe0, 0x7d, 0x90,0x35,		 //steam_type = ASF_degradable_JPEG_Media
         0x15, 0xe4, 0xcf, 0x11,
         0xa9, 0x17, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /***  Lsk pc : media player 9 no video, media player 6.4 no video,
          ivy/Lab pc : media player 9 OK, media player 6.4 bug message,***/
		#if 0
		{0x00, 0xe1,0x1b,0xb6,		 //steam_type = ASF_JFIF_Media
         0x4e, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd,0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /*** steam_type test end***/
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000000,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        /* type specific data */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */

                /* compression_id =
	            0x67706a6d,         //mjpg
	            0x47504a4d,         //MJPG
   	            0x47504a6d,         //mJPG
                0x4649464a,         //JFIF
                0x4745504a,         //JPEG
                */
	            0x47504a4d,         //MJPG

                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */
            },
        },
    };
    pVideoClipOption->asfVopWidth  = MJPEG_WIDTH;
	pVideoClipOption->asfVopHeight = MJPEG_HEIGHT;

    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + pVideoClipOption->asfVideHeaderSize;

    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize);



    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + pVideoClipOption->asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            pVideoClipOption->asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            pVideoClipOption->asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        pVideoClipOption->asfVopWidth * pVideoClipOption->asfVopHeight * 3;
#endif

#if(ASF_FORMATE==1)
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        /*** steam_type test start***/
        /***  Lsk pc : media player 11 ok, media player 6.4 ok,
            other pc : fail ***/
        #if 0
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        #if 0
		{0xe0, 0x7d, 0x90,0x35,		 //steam_type = ASF_degradable_JPEG_Media
         0x15, 0xe4, 0xcf, 0x11,
         0xa9, 0x17, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /***  Lsk pc : media player 9 no video, media player 6.4 no video,
          ivy/Lab pc : media player 9 OK, media player 6.4 bug message,***/
		#if 1
		{0x00, 0xe1,0x1b,0xb6,		 //steam_type = ASF_JFIF_Media
         0x4e, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd,0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /*** steam_type test end***/
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000000,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        /* type specific data */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x00000000,               /* reserved_flags */
        },
    };
    pVideoClipOption->asfVopWidth  = MJPEG_WIDTH;
	pVideoClipOption->asfVopHeight = MJPEG_HEIGHT;

    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =
        pVideoClipOption->asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        pVideoClipOption->asfVopHeight;
#endif


#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&pVideoClipOption->asfVideHeader, pVideoClipOption->asfVideHeaderSize, &size) == 0)
            return 0;
#endif

    return 1;
}
#endif
#ifdef ASF_AUDIO

/*

Routine Description:

    Multiple channel write audio stream properties object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteAudiStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_AUDI_STREAM_PROPERTIES_OBJECT asfAudiStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x00000068, 0x00000000},   /* object_size */
        {0x40, 0x9e, 0x69, 0xf8,    /* steam_type = ASF_Audio_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x50, 0xcd, 0xc3, 0xbf,    /* error_correction_type = ASF_No_Audio_Spread */
         0x8f, 0x61, 0xcf, 0x11,
         0x8b, 0xb2, 0x00, 0xaa,
         0x00, 0xb4, 0xe2, 0x20},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000012,         /* type_specific_data_length */
        0x00000008,         /* error_correction_data_length */
        0x0002,             /* flags = 0x0001, StreamNumber = 2, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x0001,             /* codec_id_format_tag = 0x0001 (ASF_WAVE_FORMAT_PCM) */
            0x0001,             /* number_of_channels = 0x0001 channel */
            0x00001f40,         /* samples_per_second = 0x00001F40 = 8000 sample/sec */
            0x00001f40,         /* avg_num_of_bytes_per_sec = 8000 byte/sec */
            0x0001,             /* block_alignment = 0x0001 byte */
            0x0008,             /* bits_per_sample = 0x0008 bit */
#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
            0x0000,             /* codec_specific_data_size */
                                /* codec_specific_data[0x00] */
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            0x0002,             /* codec_specific_data_size */
            IMA_ADPCM_SAMPLE_PER_BLOCK, /* wSamplesPerBlock */
#endif
        },
        {
            0x01,               /* span */
            0x00c8,             /* virtual_packet_length */
            0x00c8,             /* virtual_chunk_length */
            0x0001,             /* silence_data_length */
            0x00,               /* silence_data */
        },
    };

    asfAudiStreamPropertiesObject.object_size.lo =
        sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT);
    asfAudiStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_AUDI_TYPE_SPECIFIC_DATA);
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.codec_id_format_tag =
        asfAudiFormat.codec_id_format_tag;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.number_of_channels =
        asfAudiFormat.number_of_channels;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.samples_per_second =
        asfAudiFormat.samples_per_second;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.avg_num_of_bytes_per_sec =
        asfAudiFormat.avg_num_of_bytes_per_sec;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.block_alignment =
        asfAudiFormat.block_alignment;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.bits_per_sample =
        asfAudiFormat.bits_per_sample;
    asfAudiStreamPropertiesObject.asf_hdr_audi_error_correction_dta.virtual_packet_length =
        asfAudiStreamPropertiesObject.asf_hdr_audi_error_correction_dta.virtual_chunk_length =
            //ASF_AUDIO_VIRTUAL_PACKET_SIZE;
            IIS_CHUNK_SIZE; /* Peter 070104 */

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfAudiStreamPropertiesObject, sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfAudiStreamPropertiesObject, sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}

#endif

/*

Routine Description:

    Multiple channel write header extension object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if (VIDEO_CODEC_OPTION == H264_CODEC)
s32 MultiChannelAsfWriteHeaderExtensionObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };


    __align(4) ASF_HDR_EXT_STREAM_PROPERTY asfHeaderExtStreamPropertyObject =
    {
        {0xCB, 0xA5, 0xE6, 0x14, 	/* object_id = ASF_Extended_Stream_Properties_Object */
         0x72, 0xC6, 0x32, 0x43,
         0x83, 0x99, 0xA9, 0x69,
         0x52, 0x06, 0x5B, 0x5A},
        {0x00000000, 0x00000000},   /* object_size */
		{0x00000000, 0x00000000},   /* start_time */
		{0x00000000, 0x00000000},   /* end_time */
		 0x00000000,				/* data_bitrate = 350000 */
		 //0x004c4b40,				/* data_bitrate = 350000 */
		 0x00000bb8,				/* buffer_size */
		 0x00000000,				/* initial_buffer_fullness */
		 0x00000000,
		 //0x004c4b40,				/* alternate_data_bitrate */
		 0x00000bb8,				/* alternate_buffer_size */
		 0x00000000,				/* alternate_initial_buffer_fullness */
		 0x00000000,
		 //0x00008b11,				/* max_object_size */
		 //0xffffffff,				/* max_object_size */
		 0x00000002,				/* flag */
		 0x0001,					/* stream_num */
		 0x0000,					/* stream_language */
		 {0x00051615, 0x00000000},  /* average_time_per_frame */
		 0x0000,					/* stream_name_count */
		 0x0001,					/* payload_ext_sys_cnt */
	};

	__align(4) ASF_PAYLOAD_EXT_SYSTEMS asfPayloadExtSystem =
	{
		{0x20, 0xDC, 0x90, 0xD5,    /* object_id = ASF_Payload_Extension_System_Content_Type    */
         0xBC, 0x07, 0x6C, 0x43,
         0x9C, 0xF7, 0xF3, 0xBB,
         0xFB, 0xF1, 0xA4, 0xDC},
         0x0001,					/* data_size */
         0x00000000, 				/* info_length */
	};
    asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
	asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
        return 0;
	if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
s32 MultiChannelAsfWriteHeaderExtensionObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };


    __align(4) ASF_HDR_EXT_STREAM_PROPERTY asfHeaderExtStreamPropertyObject =
    {
        {0xCB, 0xA5, 0xE6, 0x14, 	/* object_id = ASF_Extended_Stream_Properties_Object */
         0x72, 0xC6, 0x32, 0x43,
         0x83, 0x99, 0xA9, 0x69,
         0x52, 0x06, 0x5B, 0x5A},
        {0x00000000, 0x00000000},   /* object_size */
		{0x00000000, 0x00000000},   /* start_time */
		{0x00000000, 0x00000000},   /* end_time */
		 0x00000000,				/* data_bitrate = 350000 */
		 //0x004c4b40,				/* data_bitrate = 350000 */
		 0x00000bb8,				/* buffer_size */
		 0x00000000,				/* initial_buffer_fullness */
		 0x00000000,
		 //0x004c4b40,				/* alternate_data_bitrate */
		 0x00000bb8,				/* alternate_buffer_size */
		 0x00000000,				/* alternate_initial_buffer_fullness */
		 0x00000000,
		 //0x00008b11,				/* max_object_size */
		 //0xffffffff,				/* max_object_size */
		 0x00000002,				/* flag */
		 0x0001,					/* stream_num */
		 0x0000,					/* stream_language */
		 {0x00051615, 0x00000000},  /* average_time_per_frame */
		 0x0000,					/* stream_name_count */
		 0x0001,					/* payload_ext_sys_cnt */
	};

	__align(4) ASF_PAYLOAD_EXT_SYSTEMS asfPayloadExtSystem =
	{
		{0x20, 0xDC, 0x90, 0xD5,    /* object_id = ASF_Payload_Extension_System_Content_Type    */
         0xBC, 0x07, 0x6C, 0x43,
         0x9C, 0xF7, 0xF3, 0xBB,
         0xFB, 0xF1, 0xA4, 0xDC},
         0x0001,					/* data_size */
         0x00000000, 				/* info_length */
	};
    asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
	asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
        return 0;
	if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 MultiChannelAsfWriteHeaderExtensionObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };

   asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) ;//+ sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

   //asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	//asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	//CopyMemory(asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    //asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	//CopyMemory(asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    //asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	//if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
    //    return 0;
	//if (dcfWrite(pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
    //    return 0;
#endif

    return 1;
}
#endif

/*

Routine Description:

    Multiple channel write codec list object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteCodecListObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_CODEC_LIST_OBJECT asfCodecListObject =
    {
        {0x40, 0x52, 0xd1, 0x86,    /* object_id = ASF_Codec_List_Object */
         0x1d, 0x31, 0xd0, 0x11,
         0xa3, 0xa4, 0x00, 0xa0,
         0xc9, 0x03, 0x48, 0xf6},
        {0x0000004e, 0x00000000},   /* object_size = 0x000000000000004e (video only)    */
                        /*         = 0x000000000000008a (video and audio)   */
        {0x41, 0x52, 0xD1, 0x86,    /* reserved = = ASF_Reserved_2 */
         0x1d, 0x31, 0xd0, 0x11,
         0xa3, 0xa4, 0x00, 0xa0,
         0xc9, 0x03, 0x48, 0xf6},
        0x00000001,         /* codec_entries_count = 0x00000001 (video only)    */
                        /*             = 0x00000002 (video and audio)   */
    };
    __align(4) ASF_HDR_VIDE_CODEC_ENTRY asfHdrVideCodecEntry =
    {
        0x0001,             /* type = video codec */
        0x000b,             /* codec_name_length */
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        0x0049, 0x0053,         /* codec_name[0x0B] = "ISO MPEG-4\0" */
        0x004f, 0x0020,
        0x004d, 0x0050,
        0x0045, 0x0047,
        0x002d, 0x0034,
        0x0000,
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
   		0x0000, 0x0000,         /* codec_name[0x0B] = "ISO MPEG-4\0" */
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000,
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
   		0x0048, 0x002E,         /* codec_name[0x0B] = "H.264 " */
        0x0032, 0x0036,
        0x0034, 0x0020,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000,
        #endif
        0x0000,             /* codec_description_length */
                            /* codec_description[0x00] */
        0x0004,             /* codec_information_length */
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        0x4d, 0x34, 0x53, 0x32,     /* codec_information[0x04] = "M4S2" */
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
        0x48, 0x32, 0x36, 0x34,     /* codec_information[0x04] = "H264" */
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
   		//0x6d,0x6a,0x70,0x67,  //mjpg
		0x4d,0x4a,0x50,0x47,  //MJPG
    	//0x6d,0x4a,0x50,0x47,  //mJPG
		//0x4a,0x46,0x49,0x46,  //JFIF
		//0x4a,0x50,0x45,0x47,    //JPEG

        #endif

    };
#ifdef ASF_AUDIO
    #if(IIS_SAMPLE_RATE == 8000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0036, 0x0034,         /* codec_description[0x13] = "64kb/s, 8kHz, Mono\0" */
        0x006b, 0x0062,
        0x002f, 0x0073,
        0x002c, 0x0020,
        0x0038, 0x006b,
        0x0048, 0x007a,
        0x002c, 0x0020,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #elif(IIS_SAMPLE_RATE==16000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0031, 0x0032,     /* codec_description[0x13] = "128kb/s,16kHz,Mono\0" */
        0x0038, 0x006b,
        0x0062, 0x002f,
        0x0073, 0x002c,
        0x0031, 0x0036,
        0x006b, 0x0048,
        0x007a, 0x002c,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #elif(IIS_SAMPLE_RATE==32000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0032, 0x0035,     /* codec_description[0x13] = "256kb/s,32kHz,Mono\0" */
        0x0036, 0x006b,
        0x0062, 0x002f,
        0x0073, 0x002c,
        0x0033, 0x0032,
        0x006b, 0x0048,
        0x007a, 0x002c,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #endif
#endif

    asfCodecListObject.object_size.lo =
        sizeof(ASF_CODEC_LIST_OBJECT) +
            sizeof(ASF_HDR_VIDE_CODEC_ENTRY)
#ifdef ASF_AUDIO
            +
            sizeof(ASF_HDR_AUDI_CODEC_ENTRY);
#else
            ;
#endif
    asfCodecListObject.codec_entries_count =
#ifdef ASF_AUDIO
        0x00000002;
#else
        0x00000001;
#endif

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfCodecListObject, sizeof(ASF_CODEC_LIST_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_CODEC_LIST_OBJECT);
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHdrVideCodecEntry, sizeof(ASF_HDR_VIDE_CODEC_ENTRY));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HDR_VIDE_CODEC_ENTRY);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfCodecListObject, sizeof(ASF_CODEC_LIST_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHdrVideCodecEntry, sizeof(ASF_HDR_VIDE_CODEC_ENTRY), &size) == 0)
        return 0;
#endif

#ifdef ASF_AUDIO
    #if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHdrAudiCodecEntry, sizeof(ASF_HDR_AUDI_CODEC_ENTRY));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HDR_AUDI_CODEC_ENTRY);
    #else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHdrAudiCodecEntry, sizeof(ASF_HDR_AUDI_CODEC_ENTRY), &size) == 0)
        return 0;
#endif
#endif

    return 1;
}

/*

Routine Description:

    Multiple channel write content description object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteContentDescriptionObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_CONTENT_DESCRIPTION_OBJECT asfContentDescriptionObject =
    {
        {0x33, 0x26, 0xb2, 0x75,    /* object_id = ASF_Content_Description_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0x00000062, 0x00000000},   /* object_size */
        0x0000,             /* title_length */
        0x0000,             /* author_length */
        0x0000,             /* copyright_length */
        0x0040,             /* description_length */
        0x0000,             /* rating_length */
                        /* title[0x00] */
                        /* author[0x00] */
                        /* copyright[0x00] */
        0x0000, 0x0000,         /* description[0x20] = "HIMAX PA9001\0..." */
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
                        /* rating[0x00] */
    };

    asfContentDescriptionObject.object_size.lo = sizeof(ASF_CONTENT_DESCRIPTION_OBJECT);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfContentDescriptionObject, sizeof(ASF_CONTENT_DESCRIPTION_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_CONTENT_DESCRIPTION_OBJECT);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfContentDescriptionObject, sizeof(ASF_CONTENT_DESCRIPTION_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}

/*

Routine Description:

    Multiple channel write header padding object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteHdrPaddingObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    u32 thumbnailSize, i;
    u8 *thumbnailPos;
    u32 delay_cnt = 0;
    u32 foundIFrame = 0;
	unsigned char buf[16]={0};
    unsigned char *sizeBuf;
    //__align(4) u8 paddingBytes[0x200] = { 0x00 };
    unsigned char subStreamH264Header[24] = 
    {
        0x00, 0x00, 0x00, 0x01, 0x67,
        0x42, 0x00, 0x1E, 0xDA, 0x02,
        0x80, 0xB6, 0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x68,
        0xCE, 0x38, 0x80,
    };
    __align(4) ASF_HDR_PADDING_OBJECT asfHdrPaddingObject =
    {
        {0x74, 0xd4, 0x06, 0x18,    /* object_id = ASF_Padding_Object */
         0xdf, 0xca, 0x09, 0x45,
         0xa4, 0xba, 0x9a, 0xab,
         0xcb, 0x96, 0xaa, 0xe8},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? byte */
    };

    asfHdrPaddingObject.object_size.lo =
        sizeof(ASF_HDR_PADDING_OBJECT) +
        pVideoClipOption->asfHeaderPaddingSize;	
	
#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfHdrPaddingObject, sizeof(ASF_HDR_PADDING_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_HDR_PADDING_OBJECT);

    #if (THUMBNAIL_PREVIEW_ENA == 1)
    while(delay_cnt <= 216)
    {
        delay_cnt++;
        if(pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].flag == FLAG_I_VOP)
        {
            foundIFrame = 1;
            DEBUG_ASF("Found I frame for thumbnail: %d, W:%d\n", pVideoClipOption->SearchThumbnailIdx, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
            break;
        }
        if((rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]) != ((pVideoClipOption->SearchThumbnailIdx + 1) % VIDEO_BUF_NUM))
        {
            pVideoClipOption->SearchThumbnailIdx = (pVideoClipOption->SearchThumbnailIdx+1) % VIDEO_BUF_NUM;
        }
        OSTimeDly(1);
    }

    if(foundIFrame == 1)
    {
        thumbnailSize = pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].size - pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].offset;
        thumbnailPos  = pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].offset;

        printf("delayCnt:%d, thumbnail Size:%d, BindSize:%d, BigSize:%d, (Buf: R:0x%x, W:0x%x)\n", delay_cnt, thumbnailSize, 
            pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].size, 
            pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].offset, 
            pVideoClipOption->VideoBufMng[pVideoClipOption->SearchThumbnailIdx].buffer, 
            pVideoClipOption->VideoBufMng[rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]].buffer );

    /* record Thumbnail ID for playback recongizing*/
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &thumbnailID, sizeof(thumbnailID));
        pVideoClipOption->asfMassWriteDataPoint  += sizeof(thumbnailID);
    	pVideoClipOption->asfHeaderPaddingSize   -= sizeof(thumbnailID);

    /* save Thumbnail I frame size */
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &thumbnailSize, sizeof(thumbnailSize));
        pVideoClipOption->asfMassWriteDataPoint  += sizeof(thumbnailSize);
    	pVideoClipOption->asfHeaderPaddingSize   -= sizeof(thumbnailSize);

    /* save Thumbnail SPS PPS header */    
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, subStreamH264Header, sizeof(subStreamH264Header));
        pVideoClipOption->asfMassWriteDataPoint  += sizeof(subStreamH264Header);
    	pVideoClipOption->asfHeaderPaddingSize   -= sizeof(subStreamH264Header);

    /* save Thumbnail bitstream data */
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, thumbnailPos, thumbnailSize);    
        pVideoClipOption->asfMassWriteDataPoint  += thumbnailSize;
    	pVideoClipOption->asfHeaderPaddingSize   -= thumbnailSize;    
    }
    #endif
    
	//Capture mode
	if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, "Trigger ", 8);
	else
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, "Normal  ", 8);
    pVideoClipOption->asfMassWriteDataPoint  += 8;
	pVideoClipOption->asfHeaderPaddingSize   -= 8;	
	//Display mode
	if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, "Full Screen     ", 16);
	else
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, "Quad view       ", 16);
    pVideoClipOption->asfMassWriteDataPoint  += 16;
	pVideoClipOption->asfHeaderPaddingSize   -= 16;	
	//TX version
	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, gRfiuUnitCntl[pVideoClipOption->VideoChannelID].RFpara.TxCodeVersion, sizeof(gRfiuUnitCntl[pVideoClipOption->VideoChannelID].RFpara.TxCodeVersion));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(gRfiuUnitCntl[pVideoClipOption->VideoChannelID].RFpara.TxCodeVersion);
	pVideoClipOption->asfHeaderPaddingSize   -= sizeof(gRfiuUnitCntl[pVideoClipOption->VideoChannelID].RFpara.TxCodeVersion);
	//RX version
	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, uiVersion, sizeof(uiVersion));	
	pVideoClipOption->asfMassWriteDataPoint  += sizeof(uiVersion);
	pVideoClipOption->asfHeaderPaddingSize   -= sizeof(uiVersion);
	//Monitor value
	#if VIDEO_STARTCODE_DEBUG_ENA
	{
		unsigned char buf[16]={0};
		
		sprintf(buf, "%016d", monitor_ASF_1[pVideoClipOption->VideoChannelID]);
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, buf, sizeof(buf));
	    pVideoClipOption->asfMassWriteDataPoint  += sizeof(buf);
		pVideoClipOption->asfHeaderPaddingSize   -= sizeof(buf);

		sprintf(buf, "%016d", monitor_ASF_2[pVideoClipOption->VideoChannelID]);
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, buf, sizeof(buf));
	    pVideoClipOption->asfMassWriteDataPoint  += sizeof(buf);
		pVideoClipOption->asfHeaderPaddingSize   -= sizeof(buf);

		sprintf(buf, "%016d", monitor_decode[pVideoClipOption->VideoChannelID]);
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, buf, sizeof(buf));
	    pVideoClipOption->asfMassWriteDataPoint  += sizeof(buf);
		pVideoClipOption->asfHeaderPaddingSize   -= sizeof(buf);

		sprintf(buf, "%016d", monitor_RX[pVideoClipOption->VideoChannelID]);
		CopyMemory(pVideoClipOption->asfMassWriteDataPoint, buf, sizeof(buf));
	    pVideoClipOption->asfMassWriteDataPoint  += sizeof(buf);
		pVideoClipOption->asfHeaderPaddingSize   -= sizeof(buf);		
	}
	#endif

	
	CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pVideoClipOption->paddingBytes, pVideoClipOption->asfHeaderPaddingSize);
    pVideoClipOption->asfMassWriteDataPoint  += pVideoClipOption->asfHeaderPaddingSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfHdrPaddingObject, sizeof(ASF_HDR_PADDING_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->paddingBytes, pVideoClipOption->asfHeaderPaddingSize, &size) == 0)
            return 0;
#endif
	
    return 1;
}

/*-------------------------------------------------------------------------*/
/* Data object                                 */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Write data object pre.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteDataObjectPre(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_DATA_OBJECT asfDataObject =
    {
        {0x36, 0x26, 0xb2, 0x75,    /* object_id = ASF_Data_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0xffffffff, 0xffffffff},   /* object_size = 0x???????????????? */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0xffffffff, 0xffffffff},   /* total_data_packets = 0x???????????????? */
        0x0101,             /* reserved */
                        /* asf_dta_data_pkt[0x????????????????] */
    };

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDataObject, sizeof(ASF_DATA_OBJECT));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_DATA_OBJECT);
    /*** encrypt File ***/
    #if(ASF_ENCRYPTION==1)
    CopyMemory(pVideoClipOption->EncryptBuf, pVideoClipOption->asfMassWriteData, pVideoClipOption->asfMassWriteDataPoint - pVideoClipOption->asfMassWriteData);
    #endif
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->asfMassWriteData, pVideoClipOption->asfMassWriteDataPoint - pVideoClipOption->asfMassWriteData, &size) == 0) {
        DEBUG_ASF("Write file error!!!\n");
        return 0;
    }
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDataObject, sizeof(ASF_DATA_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}

/*

Routine Description:

    Write data object post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteDataObjectPost(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    u32 offset;
    u64 objectSize = {0x00000000, 0x00000000};
    u64 totalDataPackets = {0x00000000, 0x00000000};

    offset = dcfTell(pVideoClipOption->pFile);

    dcfSeek(pVideoClipOption->pFile, pVideoClipOption->asfHeaderSize+16, FS_SEEK_SET);

    objectSize.lo = pVideoClipOption->asfDataSize;

    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&objectSize, sizeof(u64), &size) == 0)
            return 0;

    dcfSeek(pVideoClipOption->pFile, pVideoClipOption->asfHeaderSize+40, FS_SEEK_SET);

    totalDataPackets.lo = pVideoClipOption->asfDataPacketCount;

    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&totalDataPackets, sizeof(u64), &size) == 0)
            return 0;

    dcfSeek(pVideoClipOption->pFile, offset, FS_SEEK_SET);

    return 1;
}

/*

Routine Description:

    Multiple channel write data packet pre.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteDataPacketPre(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    __align(4) ASF_DATA_PACKET_MULTIPAYLOAD asfDtaDataPacket =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x11,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0xffff,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0xffff,             /* duration = 0x???? */
        },
        {
            0xff,               /* payload_flags = 0x??, NumberOfPayloads,      */
                            /*           PayloadLengthType = 2 (WORD),  */
                            /* asf_dta_payld[0x??] */
        },
    };


#if ASF_MASS_WRITE
    CopyMemory(pVideoClipOption->asfMassWriteData, &asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD));
    pVideoClipOption->asfMassWriteDataPoint   = pVideoClipOption->asfMassWriteData + sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), &size) == 0)
        return 0;
#endif

    pVideoClipOption->asfDataPacketCount++;
#if ASF_MASS_WRITE
#else
    pVideoClipOption->asfDataPacketOffset = dcfTell(pVideoClipOption->pFile) - sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
#endif
    if(pVideoClipOption->asfDataPacketFormatFlag)
    {
        pVideoClipOption->asfDataPacketPreSendTime  = pVideoClipOption->asfDataPacketSendTime;
        pVideoClipOption->asfDataPacketFormatFlag   = 0;
        pVideoClipOption->asfDataPacketSendTime     = pVideoClipOption->asfVidePresentTime - PREROLL;
    }
    pVideoClipOption->asfDataPacketNumPayload   = 0;
    pVideoClipOption->asfDataPacketLeftSize     = ASF_DATA_PACKET_SIZE - sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
    return 1;
}

/*

Routine Description:

    Multiple channel write data packet post.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    paddingLength       - Padding length.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteDataPacketPost(VIDEO_CLIP_OPTION *pVideoClipOption, u32 paddingLength)
{
    u32             size;
    u32             offset;
    u32             padding_offset;
    u16             payload_length;
    u32             tempBufLeftSize;
    u32             timestamp;

//

    //u8 paddingBytes[0x200] = { 0x00 };
    __align(4) ASF_DATA_PACKET_MULTIPAYLOAD asfDtaDataPacket =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x11,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0x0000,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0x0000,             /* duration = 0x???? */
        },
        {
            0x00,               /* payload_flags = 0x??, NumberOfPayloads,      */
                            /*           PayloadLengthType = 2 (WORD),  */
                            /* asf_dta_payld[0x??] */
        },
    };

    __align(4) ASF_DATA_PACKET_SINGLEPAYLOAD asfDtaDataPacket_single =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x10,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0x0000,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0x0000,             /* duration = 0x???? */
        }
    };

    //DEBUG_ASF("A");
    if(pVideoClipOption->asfDataPacketFormatFlag)
        timestamp = pVideoClipOption->asfDataPacketSendTime;
    else
        timestamp = pVideoClipOption->asfDataPacketPreSendTime;
	if(pVideoClipOption->PayloadType == 1)
		padding_offset = sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)-sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE);
	else if(pVideoClipOption->PayloadType == 2)
    	padding_offset = sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)-sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE);

#if ASF_MASS_WRITE
    if(pVideoClipOption->asfDataPacketNumPayload > 1)
    {
        asfDtaDataPacket.asf_dta_payload_parsing_inf.padding_length = paddingLength;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.duration = 1;
        asfDtaDataPacket.asf_dta_m_payload_dta.payload_flags = 0x80 | pVideoClipOption->asfDataPacketNumPayload;
        memset_hw(pVideoClipOption->asfMassWriteDataPoint, 0, pVideoClipOption->asfDataPacketLeftSize);
        pVideoClipOption->asfMassWriteDataPoint  += pVideoClipOption->asfDataPacketLeftSize;
        CopyMemory(pVideoClipOption->asfMassWriteData, &asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD));
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->asfMassWriteData, pVideoClipOption->asfMassWriteDataPoint - pVideoClipOption->asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
    }
    else
    {
        /* re-write fields of data packet header */
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.padding_length = paddingLength + padding_offset;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.duration = 1;

        CopyMemory(pVideoClipOption->asfMassWriteData, &asfDtaDataPacket_single, sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));

        if(pVideoClipOption->PayloadType == 1)
        {
            CopyMemory(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), 
                       pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), 
                       sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)); //ASF_DTA_VIDEO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
            CopyMemory(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE), 
                       pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD), 
                       (int)(pVideoClipOption->asfMassWriteDataPoint-(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD))));//Lsk : payload data shift left
                       //(int)(pVideoClipOption->asfMassWriteDataPoint-(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE))));//Lsk : payload data shift left
        }
		else if(pVideoClipOption->PayloadType == 2)
		{
            CopyMemory(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), 
                       pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), 
                       sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)); //ASF_DTA_AUDIO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
            CopyMemory(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE), 
                       pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD), 
                       (int)(pVideoClipOption->asfMassWriteDataPoint-(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD))));//Lsk : payload data shift left
                       //(int)(pVideoClipOption->asfMassWriteDataPoint-(pVideoClipOption->asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE))));//Lsk : payload data shift left
		}
        memset_hw(pVideoClipOption->asfMassWriteDataPoint-padding_offset, 0, pVideoClipOption->asfDataPacketLeftSize+padding_offset); //Lsk : padding data
        pVideoClipOption->asfMassWriteDataPoint  += pVideoClipOption->asfDataPacketLeftSize;
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->asfMassWriteData, pVideoClipOption->asfMassWriteDataPoint - pVideoClipOption->asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
    }
#else
    if(pVideoClipOption->asfDataPacketNumPayload > 1)
    {
        asfDtaDataPacket.asf_dta_payload_parsing_inf.padding_length = paddingLength;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.duration = 1;
        asfDtaDataPacket.asf_dta_m_payload_dta.payload_flags = 0x80 | pVideoClipOption->asfDataPacketNumPayload;
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->paddingBytes, pVideoClipOption->asfDataPacketLeftSize, &size) == 0)
            return 0;
        offset = dcfTell(pVideoClipOption->pFile);
        dcfSeek(pVideoClipOption->pFile, pVideoClipOption->asfDataPacketOffset, FS_SEEK_SET);
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), &size) == 0)
            return 0;
        dcfSeek(pVideoClipOption->pFile, offset, FS_SEEK_SET);
    }
    else
    {
        offset = dcfTell(pVideoClipOption->pFile);
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->paddingBytes, pVideoClipOption->asfDataPacketLeftSize, &size) == 0)
            return 0;
        dcfSeek(pVideoClipOption->pFile, pVideoClipOption->asfDataPacketOffset, FS_SEEK_SET);

        if (dcfRead(pVideoClipOption->pFile, (unsigned char*)(pVideoClipOption->tempbuf), ASF_DATA_PACKET_SIZE, &size) == 0)
            return 0;

        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.padding_length  = paddingLength + padding_offset;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.send_time       = timestamp;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.duration        = 1;

		if(pVideoClipOption->PayloadType == 1)
		{
			payload_length = offset-pVideoClipOption->asfDataPacketOffset-sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DTA_VIDEO_PAYLOAD);
			CopyMemory(pVideoClipOption->tempbuf, (u8*)(&asfDtaDataPacket_single), sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));
			CopyMemory(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)); //ASF_DTA_VIDEO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
			CopyMemory(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE), pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD), payload_length);//Lsk : payload data shift left
			memset_hw((u8*)(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)+payload_length), 0, padding_offset); //Lsk : padding data
		}
		else if(pVideoClipOption->PayloadType == 2)
		{
        	payload_length = offset-pVideoClipOption->asfDataPacketOffset-sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DTA_AUDIO_PAYLOAD);
			CopyMemory(pVideoClipOption->tempbuf, (u8*)(&asfDtaDataPacket_single), sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));
			CopyMemory(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)); //ASF_DTA_AUDIO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
			CopyMemory(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE), pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD), payload_length);//Lsk : payload data shift left
			memset_hw((u8*)(pVideoClipOption->tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)+payload_length), 0, padding_offset); //Lsk : padding data
		}

        dcfSeek(pVideoClipOption->pFile, pVideoClipOption->asfDataPacketOffset, FS_SEEK_SET);
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->tempbuf, ASF_DATA_PACKET_SIZE, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
    }
#endif
  
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
        //due to only update global_diskInfo when clos file, so we must calculate when open file
		//curr_free_space -= (ASF_DATA_PACKET_SIZE / 512) / 2;
		//curr_record_space += (ASF_DATA_PACKET_SIZE / 512) / 2;

		switch(dcfOverWriteOP)
        {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            case DCF_OVERWRITE_OP_OFF:
                break;
            case DCF_OVERWRITE_OP_01_DAYS:
            case DCF_OVERWRITE_OP_07_DAYS:
            case DCF_OVERWRITE_OP_30_DAYS:
            case DCF_OVERWRITE_OP_60_DAYS:
            	/*while(curr_free_space < DCF_OVERWRITE_THR_KBYTE)
                {
                    if(dcfOverWriteDel()==0)
                    {
                        DEBUG_DCF("Over Write delete fail!!\n");
                        return 0;
                    }
                    else
                    {
                        //DEBUG_ASF("Over Write delete Pass!!\n");
                    }
                    //due to only update global_diskInfo when clos file, so we must calculate when open file
                    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
                    curr_free_space = free_size - curr_record_space;
                }*/
                if(dcfOverWriteOP >= dcfGetTotalDirCount())
                    break;
                sysbackLowSetEvt(SYSBACKLOW_EVT_OVERWRITEDEL, dcfOverWriteOP, 0, 0, 0);
                break;
#endif
            default:
		        while(dcfGetMainStorageFreeSize() < DCF_OVERWRITE_THR_KBYTE)
				{
					#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel start, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif
		            if(dcfOverWriteDel()==0)
		            {
		                DEBUG_DCF("Over Write delete fail!!\n");
		                return 0;
		            }
		            else
		            {
		                //DEBUG_ASF("Over Write delete Pass!!\n");
		            }
					#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel end, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif
		            //due to only update global_diskInfo when clos file, so we must calculate when open file
				}
				break;
		}
	}
    return 1;
}

#if (AUDIO_CODEC == AUDIO_CODEC_PCM)

/*

Routine Description:

    Multiple channel write audio payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng                - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteAudiPayload(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng,int flag)
{
    u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif


    __align(4) ASF_DTA_AUDIO_PAYLOAD asfDtaPayload =
    {
        0x02,               /* stream_number = 0x02, StreamNumber = 2, KeyFrameBit = 0, (audio) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x08,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */
        0x0000,             /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

	if(pVideoClipOption->ResetPayloadPresentTime == 0)
	{

        //DEBUG_ASF("1. asfAudiPresentTime = %010d\n",pVideoClipOption->asfAudiPresentTime);
        //DEBUG_ASF("1. asfVidePresentTime = %010d\n",pVideoClipOption->asfVidePresentTime);
        pVideoClipOption->ResetPayloadPresentTime = 1;
        pVideoClipOption->asfVidePresentTime   -= DUMMY_FRAME_DURATION;  //otherwise the Start Time of Video stream will keep adding 0.1sec for every file.
        
	    if(pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime)
        {
    		pVideoClipOption->asfVidePresentTime   -= (pVideoClipOption->asfAudiPresentTime - pVideoClipOption->AV_TimeBase);
	    	pVideoClipOption->asfAudiPresentTime    = pVideoClipOption->AV_TimeBase;
        }
        else
        {
            pVideoClipOption->asfAudiPresentTime     -= (pVideoClipOption->asfVidePresentTime - pVideoClipOption->AV_TimeBase);
    	    pVideoClipOption->asfVidePresentTime      = pVideoClipOption->AV_TimeBase;
        }
        //DEBUG_ASF("2. asfAudiPresentTime = %010d\n",pVideoClipOption->asfAudiPresentTime);
        //DEBUG_ASF("2. asfVidePresentTime = %010d\n",pVideoClipOption->asfVidePresentTime);
	}
    if(flag == 0)
    {
    	if(pMng->size == 0)
    	{
    		pMng->size = 2048;
			DEBUG_ASF("Warning audio size equal 0, reset 2048\n");
    	}
    
        chunkTime = pMng->time;
        chunkSize = pMng->size;
        
    	if(pVideoClipOption->MuteRec)
    		pChunkBuf = AsfAudioZeroBuf;
    	else	
    	    pChunkBuf = pMng->buffer;    
  
    }
    else if((flag == 1) || (flag == 2))
    {
        chunkTime = 128;
        chunkSize = 2048;
        pChunkBuf = AsfAudioZeroBuf;
    }
    

    //if(EnableStreaming)
    //    send_PCM_frame(chunkTime,chunkSize,pChunkBuf);

    payloadSize = sizeof(ASF_DTA_AUDIO_PAYLOAD) + chunkSize;
    if (pVideoClipOption->asfDataPacketLeftSize < payloadSize || pVideoClipOption->asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET) //Lsk 090410
    {
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
                return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
                return 0;
    }

    pVideoClipOption->asfDataPacketNumPayload++;
    /* write audio payload header */
    asfDtaPayload.stream_number = 0x02;
    asfDtaPayload.media_object_number = (u8) pVideoClipOption->asfAudiChunkCount;
    asfDtaPayload.offset_into_media_object = 0x00000000;
    asfDtaPayload.replicated_data_length = 0x08;
    asfDtaPayload.replicated_data.lo = chunkSize;
    asfDtaPayload.replicated_data.hi = pVideoClipOption->asfAudiPresentTime;
    asfDtaPayload.payload_length = (u16) chunkSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_DTA_AUDIO_PAYLOAD);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD), &size) == 0)
            return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize -= sizeof(ASF_DTA_AUDIO_PAYLOAD);
    /* payload data */
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, chunkSize);
    pVideoClipOption->asfMassWriteDataPoint                  += chunkSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
        return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize -= chunkSize;

    /* advance the index */
    pVideoClipOption->asfAudiChunkCount++;
    pVideoClipOption->asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
    OS_ENTER_CRITICAL();
    pVideoClipOption->CurrentAudioSize   -= chunkSize;
    OS_EXIT_CRITICAL();
    //}
    if((flag == 0) || (flag == 2))
    {
        pVideoClipOption->sysAudiPresentTime += chunkTime;
  #if ASF_DEBUG_ENA
    ASF_sem_A++;
  #endif
    }
    return 1;
}

#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)

/*

Routine Description:

    Multiple channel write ima adpcm audio payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng1               - Buffer manager 1.
    pMng2               - Buffer manager 2.

Return Value:

    0 - Failure.
    1 - Success and only using pMng1 PCM data.
    2 - Success and using both pMng1 and pMng2 PCM data.

*/
s32 MultiChannelAsfWriteAudiPayload_IMA_ADPCM_1Payload(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng1, IIS_BUF_MNG* pMng2, s32 *pPcmOffset)
{
    u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    IMA_ADPCM_Option    ImaAdpcmOpt;
    IMA_ADPCM_Option    *pImaAdpcmOpt;
    s32                 PcmBytesForAdpcm;
    s32                 PcmOffset;

    if(IISMode == 1)
        pVideoClipOption->Audio_formate = Audio_formate_Out;
    else
        pVideoClipOption->Audio_formate = Audio_formate_In;
    
    __align(4) ASF_DTA_AUDIO_PAYLOAD asfDtaPayload =
    {
        0x02,               /* stream_number = 0x02, StreamNumber = 2, KeyFrameBit = 0, (audio) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x08,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */
        0x0000,             /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

	if(pVideoClipOption->ResetPayloadPresentTime == 0)
	{

        //DEBUG_ASF("1. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("1. asfVidePresentTime = %010d\n",asfVidePresentTime);
        pVideoClipOption->ResetPayloadPresentTime = 1;
        pVideoClipOption->asfVidePresentTime   -= DUMMY_FRAME_DURATION;  //otherwise the Start Time of Video stream will keep adding 0.1sec for every file.        
        
	    if(pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime)
        {
    		pVideoClipOption->asfVidePresentTime   -= pVideoClipOption->(pVideoClipOption->asfAudiPresentTime - pVideoClipOption->AV_TimeBase);
	    	pVideoClipOption->asfAudiPresentTime    = pVideoClipOption->AV_TimeBase;
        }
        else
        {
            pVideoClipOption->asfAudiPresentTime   -= (pVideoClipOption->asfVidePresentTime - pVideoClipOption->AV_TimeBase);
    	    pVideoClipOption->asfVidePresentTime    = pVideoClipOption->AV_TimeBase;
        }
        //DEBUG_ASF("2. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("2. asfVidePresentTime = %010d\n",asfVidePresentTime);
	}
    //chunkTime = pMng->time;
    //chunkSize = pMng->size;
    //pChunkBuf = pMng->buffer;
    chunkTime   = (IMA_ADPCM_SAMPLE_PER_BLOCK * 1000 / IIS_SAMPLE_RATE) + pVideoClipOption->AdpcmPayloadTimeLose[asfAudiChunkCount & 0xf];
    chunkSize   = IMA_ADPCM_BLOCK_SIZE;
    pChunkBuf   = (u8*)pVideoClipOption->ImaAdpcmBuf;

    //if(EnableStreaming)
    //    send_PCM_frame(chunkTime,chunkSize,pChunkBuf);

    payloadSize = sizeof(ASF_DTA_AUDIO_PAYLOAD) + chunkSize;
    if (pVideoClipOption->asfDataPacketLeftSize < payloadSize || pVideoClipOption->asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET) //Lsk 090410
    {
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
                return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
                return 0;
    }

    pImaAdpcmOpt                        = &ImaAdpcmOpt;
    pImaAdpcmOpt->AdpcmAddress          = pVideoClipOption->ImaAdpcmBuf;
    pImaAdpcmOpt->AdpcmSize             = IMA_ADPCM_BLOCK_SIZE;
    pImaAdpcmOpt->AdpcmSamplePerBlock   = IMA_ADPCM_SAMPLE_PER_BLOCK;
    pImaAdpcmOpt->PcmStrWord            = 0;
    pImaAdpcmOpt->AdpcmStrWord          = 0;
    pImaAdpcmOpt->AdpcmSample           = 0;
    pImaAdpcmOpt->AdpcmIndex            = 0;
    switch (pVideoClipOption->Audio_formate) {
        case nomo_8bit_8k:
        case nomo_8bit_16k:
            pImaAdpcmOpt->PcmBitPerSample   = 8;
            pImaAdpcmOpt->PcmSigned         = 0;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            break;
        case nomo_16bit_8k:
            pImaAdpcmOpt->PcmBitPerSample   = 16;
            pImaAdpcmOpt->PcmSigned         = 1;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
            break;
        default:
            pImaAdpcmOpt->PcmBitPerSample   = 8;
            pImaAdpcmOpt->PcmSigned         = 0;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            DEBUG_ASF("Don't support Audio_formate %d\n", pVideoClipOption->Audio_formate);
    }
    pImaAdpcmOpt->PcmTotalSize      = PcmBytesForAdpcm;
    PcmOffset                       = *pPcmOffset;
    if(PcmOffset <= (IIS_CHUNK_SIZE - PcmBytesForAdpcm)) {
        pImaAdpcmOpt->PcmAddress1   = pMng1->buffer + PcmOffset;
        pImaAdpcmOpt->PcmAddress2   = 0;
        pImaAdpcmOpt->PcmSize1      = PcmBytesForAdpcm;
        pImaAdpcmOpt->PcmSize2      = 0;
        *pPcmOffset                += PcmBytesForAdpcm;
    } else {
        pImaAdpcmOpt->PcmAddress1   = pMng1->buffer + PcmOffset;
        pImaAdpcmOpt->PcmAddress2   = pMng2->buffer;
        pImaAdpcmOpt->PcmSize1      = pMng1->size - PcmOffset;
        pImaAdpcmOpt->PcmSize2      = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
        *pPcmOffset                 = pImaAdpcmOpt->PcmSize2;
    }
    if(IMA_ADPCM_Encode_Block_HW(pImaAdpcmOpt) == 0) {
        DEBUG_IIS("Wav Write Data error!!!\n");
        pVideoClipOption->sysVoiceRecStop     = 1;
        pVideoClipOption->sysVoiceRecStart    = 0;
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }

    pVideoClipOption->asfDataPacketNumPayload++;
    /* write audio payload header */
    asfDtaPayload.stream_number             = 0x02;
    asfDtaPayload.media_object_number       = (u8) pVideoClipOption->asfAudiChunkCount;
    asfDtaPayload.offset_into_media_object  = 0x00000000;
    asfDtaPayload.replicated_data_length    = 0x08;
    asfDtaPayload.replicated_data.lo        = chunkSize;
    asfDtaPayload.replicated_data.hi        = pVideoClipOption->asfAudiPresentTime;
    asfDtaPayload.payload_length            = (u16) chunkSize;
#if ASF_MASS_WRITE
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD));
    pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_DTA_AUDIO_PAYLOAD);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD), &size) == 0)
            return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize -= sizeof(ASF_DTA_AUDIO_PAYLOAD);
    /* payload data */
#if ASF_MASS_WRITE
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, chunkSize);
    pVideoClipOption->asfMassWriteDataPoint                  += chunkSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize -= chunkSize;

    /* advance the index */
    pVideoClipOption->asfAudiChunkCount++;
    pVideoClipOption->asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
        OS_ENTER_CRITICAL();
        pVideoClipOption->CurrentAudioSize   -= chunkSize;
        OS_EXIT_CRITICAL();
    //}

    if(pImaAdpcmOpt->PcmSize2)
        return 2;
    else
        return 1;
}

/*

Routine Description:

    Multiple channel write ima adpcm audio payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng                - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteAudiPayload_IMA_ADPCM(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng, s32 *pPcmOffset)
{
    s32 nRtn, PcmBytesForAdpcm;
    
    if(IISMode == 1)
        pVideoClipOption->Audio_formate = Audio_formate_Out;
    else
        pVideoClipOption->Audio_formate = Audio_formate_In;
    
    switch (pVideoClipOption->Audio_formate) {
        case nomo_8bit_8k:
        case nomo_8bit_16k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            break;
        case nomo_16bit_8k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
            break;
        default:
            DEBUG_ASF("Audio channel %d Don't support Audio_formate %d\n", pVideoClipOption->AudioChannelID, pVideoClipOption->Audio_formate);
    }
    if(*pPcmOffset == 0)    // no temp payload
    {
        do {
            nRtn    = asfWriteAudiPayload_IMA_ADPCM_1Payload(pVideoClipOption->pFile, pMng, 0, pPcmOffset);
        } while ((nRtn == 1) && ((*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE));
        if(nRtn == 1)
        {
            memcpy(pVideoClipOption->iisBufMngTemp.buffer + *pPcmOffset, pMng->buffer + *pPcmOffset, IIS_CHUNK_SIZE - *pPcmOffset);
            pVideoClipOption->iisBufMngTemp.size  = pMng->size;
            pVideoClipOption->iisBufMngTemp.time  = pMng->time;
            return 1;
        }
    } else {    // 前一個audio chunk data還沒編完
        nRtn    = MultiChannelAsfWriteAudiPayload_IMA_ADPCM_1Payload(pVideoClipOption, &pVideoClipOption->iisBufMngTemp, pMng, pPcmOffset);
        if((nRtn == 2) && ((*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE))
        {
            do {
                nRtn    = MultiChannelAsfWriteAudiPayload_IMA_ADPCM_1Payload(pVideoClipOption, pMng, 0, pPcmOffset);
            } while ((nRtn == 1) && (*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE);
        }
        if(nRtn)
        {
            memcpy(pVideoClipOption->iisBufMngTemp.buffer + *pPcmOffset, pMng->buffer + *pPcmOffset, IIS_CHUNK_SIZE - *pPcmOffset);
            pVideoClipOption->iisBufMngTemp.size  = pMng->size;
            pVideoClipOption->iisBufMngTemp.time  = pMng->time;
            return 1;
        }
    }
    return 0;
}

#endif  // #if (AUDIO_CODEC == AUDIO_CODEC_PCM)

/*

Routine Description:

    Multiple channel write video payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng                - Buffer manager.
    flag                - 0: write video playlaod, 1: write "no single" 1 FPS.
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteVidePayload(VIDEO_CLIP_OPTION *pVideoClipOption, VIDEO_BUF_MNG* pMng,u8 flag)
{
    u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;
    u32 SPS_PPS_WriteSize   = 0;
    u8  SPS_PPS_NotReady    = 0;
    u32 time_shift          = 0;
#if (OS_CRITICAL_METHOD == 3)                       /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr    = 0;                    /* Prevent compiler warning                           */
#endif

    __align(4) ASF_DTA_VIDEO_PAYLOAD asfDtaPayload =
    {
        0x81,               /* stream_number = 0x81, StreamNumber = 1, KeyFrameBit = 1, (video) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x09,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */


		0x00,           /* interlace and top field first */


        0x0000,         /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };
    if(pMng->time > 3000 || pMng->time < 20)
    {
        DEBUG_ASF("ASF Video time error, ori: %d\n",(u32)pMng->time);
        pMng->time = 66;
        DEBUG_ASF("ASF Video time error %d\n",(u32)pMng->time);
    }

	#if CHECK_VIDEO_BITSTREAM
	if(flag==0)
	{
		if((*(unsigned int*)(pMng->buffer) == 0x01000000)
			&&(*(unsigned int*)(pMng->buffer + pMng->size - 4) == 0x00000001)
			&&((pVideoClipOption->SkipToIframe==0)||((pVideoClipOption->SkipToIframe==1)&&(pMng->asfflag==FLAG_I_VOP)))
			)																					  		
		{
			pVideoClipOption->SkipToIframe  = 0;
		}
		else
		{
			//Lsk: video payload error occur
			if(pVideoClipOption->SkipToIframe==0)
			{
				DEBUG_ASF("Warning!!! CH%d Error VideoPayload\n", pVideoClipOption->VideoChannelID);
				DEBUG_ASF("<0x%08x>\n", *(unsigned int*)(pMng->buffer));
				DEBUG_ASF("<0x%08x>\n", *(unsigned int*)(pMng->buffer + pMng->size - 4));
				DEBUG_ASF("ReadIdx: %d,  ReadBuffer:<0x%08x>, frame size:%x\n", pVideoClipOption->VideoBufMngReadIdx,  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer - rfiuRxVideoBuf[pVideoClipOption->RFUnit],  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size);
				DEBUG_ASF("WriteIdx:%d, WriteBuffer:<0x%08x>\n", rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit],  pVideoClipOption->VideoBufMng[rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]].buffer - rfiuRxVideoBuf[pVideoClipOption->RFUnit]);                      
				//DEBUG_ASF("<%d, %d, %d>\n", pVideoClipOption->SkipToIframe, pMng->asfflag, flag);
			}
				

			chunkTime = pMng->time;
		    #if (VIDEO_CODEC_OPTION == H264_CODEC)
		    chunkSize = pMng->offset;
		    #else
		    chunkSize = pMng->size;
		    #endif
		    pMng->asfflag = 2;
		    /* advance the index */
		    pVideoClipOption->asfVidePresentTime += chunkTime;

		    OS_ENTER_CRITICAL();
		    pVideoClipOption->CurrentVideoSize   -= chunkSize;
		    pVideoClipOption->CurrentBufferSize  -= pMng->size;
		    pVideoClipOption->CurrentVideoTime   -= chunkTime;
		    OS_EXIT_CRITICAL();
		    
		    pVideoClipOption->sysVidePresentTime += chunkTime;
			pVideoClipOption->asfTimeStatistics  += chunkTime;

			pVideoClipOption->SkipToIframe  = 1;
			return 1;			
		}
	}
	#endif

  	#if VIDEO_STARTCODE_DEBUG_ENA
	if((*(pMng->buffer) == 0x00) && (*(pMng->buffer+1) == 0x00) && (*(pMng->buffer+2) == 0x00) 
        && (*(pMng->buffer+3) == 0x01) && ((*(pMng->buffer+4) == 0x41) || (*(pMng->buffer+4) == 0x45) || (*(pMng->buffer+4) == 0x65)))
	{
	}
	else
	{
		monitor_ASF_1[pVideoClipOption->VideoChannelID]++;
		DEBUG_ASF("Warning!!! WriteVidePayload H264 start code error - %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);
	}
  	#endif

    if(pVideoClipOption->ASF_set_interlace_flag)
    {
        asfDtaPayload.flag =  0xc0; 			/* interlace and top field first */
    }

	pVideoClipOption->PayloadType               = 1;
    pVideoClipOption->asfDataPacketFormatFlag   = 1;

    if(flag == 0)
        chunkFlag                                   = pMng->asfflag;
  #if INSERT_NOSIGNAL_FRAME
    else
        chunkFlag                                   = 1; //always I frame.
  #endif  
  #ifdef ASF_AUDIO
    if(flag == 0)
        chunkTime                                   = pMng->time;
   #if INSERT_NOSIGNAL_FRAME
    else
        chunkTime                                   = 1000; //timestamp 1s.
   #endif
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if(Lose_video_time[pVideoClipOption->RFUnit] > 0)
    {
        chunkTime += Lose_video_time[pVideoClipOption->RFUnit];
        time_shift = Lose_video_time[pVideoClipOption->RFUnit];
        DEBUG_ASF("Lose %d time %d %d\n",Lose_video_time[pVideoClipOption->RFUnit],pMng->time,pVideoClipOption->asfVopCount);
        Lose_video_time[pVideoClipOption->RFUnit] = 0;
    }
   #endif
  #else
    #if(RECORD_SOURCE == LOCAL_RECORD)
    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
        if(pVideoClipOption->asfVopWidth > 720) // 720P
            chunkTime   = 66;   // 67ms, 15fps.
        else                                    // VGA
            chunkTime   = 33;   // 33ms, 30fps.
    } 
    #elif(RECORD_SOURCE == RX_RECEIVE)
        if(pVideoClipOption->AV_Source == RX_RECEIVE) 
    {
        chunkTime   = pMng->time;
    }
    #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
  #endif



  #if FORCE_FPS
    if(pVideoClipOption->DummyChunkTime && (chunkTime > pVideoClipOption->DummyChunkTime))
    {
        chunkTime                          -= pVideoClipOption->DummyChunkTime;
        pVideoClipOption->DummyChunkTime    = 0;
    }
  #endif
    if(flag == 0)
    {    	
        if(pVideoClipOption->AV_Source == RX_RECEIVE)
            #if(VIDEO_CODEC_OPTION == H264_CODEC)
		    chunkSize = pMng->offset;
            #else
            chunkSize = pMng->size;
            #endif
        else
            chunkSize   = pMng->size;
        pChunkBuf   = pMng->buffer;
    }
  #if INSERT_NOSIGNAL_FRAME
    else
    {
      #if((HW_BOARD_OPTION  == MR9200_RX_RDI_UDR777) && (PROJ_OPT ==6))
        if(CurrLanguage == UI_MENU_SETTING_LANGUAGE_GERMAN)
        {
            if((pVideoClipOption->mpeg4Width == 640) && (pVideoClipOption->mpeg4Height == 352))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_German_QHD);
           		pChunkBuf = H264_NoSignal_German_QHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1280) && (pVideoClipOption->mpeg4Height == 720)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_German_HD);
           		pChunkBuf = H264_NoSignal_German_HD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1072)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_German_FHD);
           		pChunkBuf = H264_NoSignal_German_FHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1080)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_German_FHD_1080);
           		pChunkBuf = H264_NoSignal_German_FHD_1080;
            }
        }
        else
        {
            if((pVideoClipOption->mpeg4Width == 640) && (pVideoClipOption->mpeg4Height == 352))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_QHD);
           		pChunkBuf = H264_NoSignal_QHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1280) && (pVideoClipOption->mpeg4Height == 720)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_HD);
           		pChunkBuf = H264_NoSignal_HD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1072)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_FHD);
           		pChunkBuf = H264_NoSignal_FHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1080)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_FHD_1080);
           		pChunkBuf = H264_NoSignal_FHD_1080;
            }
        }
      #elif ((HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
             (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8897H))
      #if (UI_OTHER_LANGUAGE == 1)
        if (CurrLanguage == UI_MENU_SETTING_LANGUAGE_JAPANSE)
        {
            if((pVideoClipOption->mpeg4Width == 640) && (pVideoClipOption->mpeg4Height == 352))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_Japan_QHD);
           		pChunkBuf = H264_NoSignal_Japan_QHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1280) && (pVideoClipOption->mpeg4Height == 720)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_Japan_HD);
           		pChunkBuf = H264_NoSignal_Japan_HD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1072)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_Japan_FHD);
           		pChunkBuf = H264_NoSignal_Japan_FHD;
            }
        }
        else
      #endif
        {
            if((pVideoClipOption->mpeg4Width == 640) && (pVideoClipOption->mpeg4Height == 352))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_QHD);
           		pChunkBuf = H264_NoSignal_QHD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1280) && (pVideoClipOption->mpeg4Height == 720)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_HD);
           		pChunkBuf = H264_NoSignal_HD;
            }
            else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1072)))
            {
      	  	    chunkSize = sizeof(H264_NoSignal_FHD);
           		pChunkBuf = H264_NoSignal_FHD;
            }
        }
      #else
        if((pVideoClipOption->mpeg4Width == 640) && (pVideoClipOption->mpeg4Height == 352))
        {
  	  	    chunkSize = sizeof(H264_NoSignal_QHD);
       		pChunkBuf = H264_NoSignal_QHD;
        }
        else if(((pVideoClipOption->mpeg4Width == 1280) && (pVideoClipOption->mpeg4Height == 720)))
        {
  	  	    chunkSize = sizeof(H264_NoSignal_HD);
       		pChunkBuf = H264_NoSignal_HD;
        }
        else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1072)))
        {
  	  	    chunkSize = sizeof(H264_NoSignal_FHD);
       		pChunkBuf = H264_NoSignal_FHD;
        }
        else if(((pVideoClipOption->mpeg4Width == 1920) && (pVideoClipOption->mpeg4Height == 1080)))
        {
  	  	    chunkSize = sizeof(H264_NoSignal_FHD_1080);
       		pChunkBuf = H264_NoSignal_FHD_1080;
        }
      #endif
    }
  #endif
      //printf("size %d %d\n",chunkSize,pMng->offset);

  #if(VIDEO_CODEC_OPTION == H264_CODEC)   // 無線的第一張 I frame 需加上 NAL header
    if((pVideoClipOption->AV_Source == RX_RECEIVE) && (pVideoClipOption->asfVopCount == 0))
        chunkSize  += pVideoClipOption->SPS_PPS_Length;
#endif

    //if(EnableStreaming)
    //    send_mpeg4_frame(chunkTime,chunkSize,pChunkBuf);
	if(pVideoClipOption->ResetPayloadPresentTime == 0)
	{
        //DEBUG_ASF("3. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("3. asfVidePresentTime = %010d\n",asfVidePresentTime);

		pVideoClipOption->ResetPayloadPresentTime = 1;
        pVideoClipOption->asfVidePresentTime   -= DUMMY_FRAME_DURATION;  //otherwise the Start Time of Video stream will keep adding 0.1sec for every file.
        
	    if(pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime)
        {
    		pVideoClipOption->asfVidePresentTime -= (pVideoClipOption->asfAudiPresentTime - pVideoClipOption->AV_TimeBase);
	    	pVideoClipOption->asfAudiPresentTime = pVideoClipOption->AV_TimeBase;
        }
        else
        {
            pVideoClipOption->asfAudiPresentTime     -= (pVideoClipOption->asfVidePresentTime - pVideoClipOption->AV_TimeBase);
    		pVideoClipOption->asfVidePresentTime      = pVideoClipOption->AV_TimeBase;
        }
        //DEBUG_ASF("4. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("4. asfVidePresentTime = %010d\n",asfVidePresentTime);

	}
	pVideoClipOption->asfVidePresentTime   += chunkTime;    //Lsk 090409 : add chunkTime before write video payload
    pVideoClipOption->asfTimeStatistics    += chunkTime;

    /* corresponding to payload header, if left size of data packet is not efficiently used, just padding the left size */
    if (pVideoClipOption->asfDataPacketLeftSize < (sizeof(ASF_DTA_VIDEO_PAYLOAD) + ASF_PADDING_THRESHOLD) || pVideoClipOption->asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET)
    {
        /* new a data packet */
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
            return 0;
    }

    /* save next index entry of 1 sec boundary */
    streamNumber = 0x01;
    #if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    chunkFlag = 1;
    #endif
    if (chunkFlag & FLAG_I_VOP)
    {
        pVideoClipOption->asfIndexEntryPacketNumber   = pVideoClipOption->asfDataPacketCount - 1; /* data packet index */
        pVideoClipOption->asfIndexEntryPacketCount    = 1;
        streamNumber = 0x81;
        //DEBUG_ASF("I");
    }

    pVideoClipOption->asfDataPacketNumPayload++;
    /* write video payload */
    payloadSize             = sizeof(ASF_DTA_VIDEO_PAYLOAD) + chunkSize;
    subPayloadSize          = (payloadSize <= pVideoClipOption->asfDataPacketLeftSize) ? payloadSize : pVideoClipOption->asfDataPacketLeftSize; /* subpayload size with header */
    payloadSize            -= subPayloadSize; /* left size of payload */
    subPayloadSize         -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
    offsetIntoMediaObject   = 0;
    /* write video payload header */
    asfDtaPayload.stream_number = streamNumber;
#if FORCE_FPS
    asfDtaPayload.media_object_number       = (u8) (pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount);
#else
    asfDtaPayload.media_object_number       = (u8) pVideoClipOption->asfVopCount;
#endif
    asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
    asfDtaPayload.replicated_data_length    = 0x09;
    asfDtaPayload.replicated_data.lo        = chunkSize;
    asfDtaPayload.replicated_data.hi        = pVideoClipOption->asfVidePresentTime;
    asfDtaPayload.payload_length            = (u16) subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
    pVideoClipOption->asfMassWriteDataPoint                  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
        return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize    -= sizeof(ASF_DTA_VIDEO_PAYLOAD);
    /* write video payload data */
    offsetIntoMediaObject                      += subPayloadSize;
#if ASF_MASS_WRITE
  #if(VIDEO_CODEC_OPTION == H264_CODEC)   // 無線的第一張 I frame 需加上 NAL header
    if((pVideoClipOption->AV_Source == RX_RECEIVE) && (pVideoClipOption->asfVopCount == 0))
    {
        if(subPayloadSize >= pVideoClipOption->SPS_PPS_Length)
        {
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pVideoClipOption->SPS_PPS_Buffer, pVideoClipOption->SPS_PPS_Length);
            pVideoClipOption->asfMassWriteDataPoint    += pVideoClipOption->SPS_PPS_Length;
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize - pVideoClipOption->SPS_PPS_Length);
          #if VIDEO_STARTCODE_DEBUG_ENA
        	if((*(pVideoClipOption->asfMassWriteDataPoint) == 0x00) && (*(pVideoClipOption->asfMassWriteDataPoint+1) == 0x00) && (*(pVideoClipOption->asfMassWriteDataPoint+2) == 0x00) 
                && (*(pVideoClipOption->asfMassWriteDataPoint+3) == 0x01) && ((*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x41) || (*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x45) || (*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x65)))
        	{
        	}
        	else
        	{
        		monitor_ASF_2[pVideoClipOption->VideoChannelID]++;
        		DEBUG_ASF("Warning!!! %x %x %x %x %x \n", *(pVideoClipOption->asfMassWriteDataPoint),*(pVideoClipOption->asfMassWriteDataPoint+1),*(pVideoClipOption->asfMassWriteDataPoint+2),*(pVideoClipOption->asfMassWriteDataPoint+3),*(pVideoClipOption->asfMassWriteDataPoint+4));
        		DEBUG_ASF("Warning!!! WriteData H264 start code error - %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);
        	}
          #endif
            pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize - pVideoClipOption->SPS_PPS_Length;
            pChunkBuf                                  += subPayloadSize - pVideoClipOption->SPS_PPS_Length;
        }
        else
        {
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pVideoClipOption->SPS_PPS_Buffer, subPayloadSize);
            pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
            SPS_PPS_NotReady                            = 1;
            SPS_PPS_WriteSize                           = subPayloadSize;
        }
    }
    else
    {
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
      #if VIDEO_STARTCODE_DEBUG_ENA
    	if((*(pVideoClipOption->asfMassWriteDataPoint) == 0x00) && (*(pVideoClipOption->asfMassWriteDataPoint+1) == 0x00) && (*(pVideoClipOption->asfMassWriteDataPoint+2) == 0x00) 
            && (*(pVideoClipOption->asfMassWriteDataPoint+3) == 0x01) && ((*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x41) || (*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x45) || (*(pVideoClipOption->asfMassWriteDataPoint+4) == 0x65)))
    	{
    	}
    	else
    	{
    		monitor_ASF_2[pVideoClipOption->VideoChannelID]++;
    		DEBUG_ASF("#2 Warning!!! %x %x %x %x %x \n", *(pVideoClipOption->asfMassWriteDataPoint),*(pVideoClipOption->asfMassWriteDataPoint+1),*(pVideoClipOption->asfMassWriteDataPoint+2),*(pVideoClipOption->asfMassWriteDataPoint+3),*(pVideoClipOption->asfMassWriteDataPoint+4));
    		DEBUG_ASF("#2 Warning!!! WriteData H264 start code error - %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);
    	}
      #endif
        pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
        pChunkBuf                                  += subPayloadSize;
    }
  #else
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
    pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
    pChunkBuf                                  += subPayloadSize;
  #endif
#else
  #if(VIDEO_CODEC_OPTION == H264_CODEC)   // 無線的第一張 I frame 需加上 NAL header
    if((pVideoClipOption->AV_Source == RX_RECEIVE) && (pVideoClipOption->asfVopCount == 0))
    {
        if(subPayloadSize >= pVideoClipOption->SPS_PPS_Length)
        {
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->SPS_PPS_Buffer, pVideoClipOption->SPS_PPS_Length, &size) == 0)
                return 0;
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize - pVideoClipOption->SPS_PPS_Length, &size) == 0)
                return 0;
            pChunkBuf                                  += subPayloadSize - pVideoClipOption->SPS_PPS_Length;
        }
        else
        {
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->SPS_PPS_Buffer, subPayloadSize, &size) == 0)
                return 0;
            SPS_PPS_NotReady                            = 1;
            SPS_PPS_WriteSize                           = subPayloadSize;
        }
    }
    else
    {
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
            return 0;
        pChunkBuf                                  += subPayloadSize;
    }
  #else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
        return 0;
    pChunkBuf                                  += subPayloadSize;
  #endif
#endif
    pVideoClipOption->asfDataPacketLeftSize    -= subPayloadSize;
    while (payloadSize > 0)
    {
        /* new a data packet */
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
            return 0;
        pVideoClipOption->asfDataPacketFormatFlag             = 1;
        if (chunkFlag & FLAG_I_VOP)
            pVideoClipOption->asfIndexEntryPacketCount++;

        pVideoClipOption->asfDataPacketNumPayload++;
        /* write subsequent video payload */
        payloadSize    += sizeof(ASF_DTA_VIDEO_PAYLOAD);
        subPayloadSize  = (payloadSize <= pVideoClipOption->asfDataPacketLeftSize) ? payloadSize : pVideoClipOption->asfDataPacketLeftSize; /* subpayload size with header */
        payloadSize    -= subPayloadSize; /* left size of payload */
        subPayloadSize -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
        /* write video payload header */

        asfDtaPayload.stream_number             = streamNumber;
#if FORCE_FPS
        asfDtaPayload.media_object_number       = (u8) (pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount);
#else
        asfDtaPayload.media_object_number       = (u8) pVideoClipOption->asfVopCount;
#endif
        asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
        asfDtaPayload.replicated_data_length    = 0x09;
        asfDtaPayload.replicated_data.lo        = chunkSize;
        asfDtaPayload.replicated_data.hi        = pVideoClipOption->asfVidePresentTime;
        asfDtaPayload.payload_length            = (u16) subPayloadSize;

#if ASF_MASS_WRITE
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
        pVideoClipOption->asfMassWriteDataPoint                  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
            return 0;
#endif

        pVideoClipOption->asfDataPacketLeftSize    -= sizeof(ASF_DTA_VIDEO_PAYLOAD);


        /* write video payload data */
        offsetIntoMediaObject                      += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    #if(VIDEO_CODEC_OPTION == H264_CODEC)  
        if(SPS_PPS_NotReady)    // 無線的第一張 I frame 需加上 NAL header
        {
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pVideoClipOption->SPS_PPS_Buffer + SPS_PPS_WriteSize, pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize);
            pVideoClipOption->asfMassWriteDataPoint    += pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize;
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize - (pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize));
            pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize - (pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize);
            pChunkBuf                                  += subPayloadSize - (pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize);
            SPS_PPS_NotReady                            = 0;
        }
        else
        {
            CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
            pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
            pChunkBuf                                  += subPayloadSize;
        }
    #else
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
        pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
        pChunkBuf                                  += subPayloadSize;        
    #endif
#else
    #if(VIDEO_CODEC_OPTION == H264_CODEC)
        if(SPS_PPS_NotReady)    // 無線的第一張 I frame 需加上 NAL header
        {
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->SPS_PPS_Buffer + SPS_PPS_WriteSize, pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize, &size) == 0)
                return 0;
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize - (pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize), &size) == 0)
                return 0;
            pChunkBuf                                  += subPayloadSize - (pVideoClipOption->SPS_PPS_Length - SPS_PPS_WriteSize);
            SPS_PPS_NotReady                            = 0;
        }
        else
        {
            if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
                return 0;
            pChunkBuf                                  += subPayloadSize;
        }
    #else
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
            return 0;
        pChunkBuf                                  += subPayloadSize;        
    #endif
#endif
        pVideoClipOption->asfDataPacketLeftSize    -= subPayloadSize;
    }

    /* set index entry on each 1 sec interval */
    while (pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfIndexEntryTime)    //Lsk 090309 preroll index object
    {
        pVideoClipOption->asfIndexTable[pVideoClipOption->asfIndexTableIndex].packet_number     = pVideoClipOption->asfIndexEntryPacketNumber;
        pVideoClipOption->asfIndexTable[pVideoClipOption->asfIndexTableIndex++].packet_count    = pVideoClipOption->asfIndexEntryPacketCount;
        pVideoClipOption->asfIndexEntryTime        += 1000; /* next index after 1 s = 1000 ms */
/*CY 0629 S*/
        if (pVideoClipOption->asfIndexTableIndex >= ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
        {
            DEBUG_ASF2("Ch%d Trace: Video time (%d sec) reaches limit.\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfIndexTableIndex);
            DEBUG_ASF2("Ch%d asfVidePresentTime(%d) asfIndexEntryTime(%d)\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfIndexEntryTime);
            return 0;
        }
/*CY 0629 E*/
    }

    /* advance the index */
    pVideoClipOption->asfVopCount++;
    //asfVidePresentTime += chunkTime;    //Lsk 090409
    //if(chunkTime <= 1)    /* Peter 070104 */
    //if(chunkTime < IISTimeUnit)    /* Peter 070104 */
        //DEBUG_ASF("Video chunkTime == %d, asfVopCount = %d\n", chunkTime, asfVopCount);

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
    OS_ENTER_CRITICAL();
  #if INSERT_NOSIGNAL_FRAME
    if(flag == 0)
    {
        pVideoClipOption->sysVidePresentTime    += chunkTime;
      #if ASF_DEBUG_ENA
        ASF_sem_V++;
      #endif
    }
  #endif
    if(flag == 0)
    {
#if(VIDEO_CODEC_OPTION == H264_CODEC)      
    	if((pVideoClipOption->AV_Source == RX_RECEIVE) && ((pVideoClipOption->asfVopCount -1) == 0))
            pVideoClipOption->CurrentVideoSize   -= (chunkSize - pVideoClipOption->SPS_PPS_Length);
        else
#endif            
            pVideoClipOption->CurrentVideoSize   -= chunkSize;
		
    	pVideoClipOption->CurrentBufferSize   -= pMng->size;		        
      #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
        if(time_shift > 0)
        {
            chunkTime -= time_shift;
            pVideoClipOption->CurrentVideoTime   -= chunkTime;
            time_shift = 0;
        }
        else
            pVideoClipOption->CurrentVideoTime   -= chunkTime;
      #else
        pVideoClipOption->CurrentVideoTime   -= chunkTime;
      #endif
    }
    OS_EXIT_CRITICAL();
    
        //DEBUG_ASF("Ch%d CurrentVideoSize = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->CurrentVideoSize);
    //}
    pMng->asfflag = 2;
    return 1;
}

#if 1//FORCE_FPS

/*

Routine Description:

    Multiple channel write dummy video payload with MPEG-4 no coded frame.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteDummyVidePayload(VIDEO_CLIP_OPTION *pVideoClipOption,int time,u8 frame_num)
{
    u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;

    __align(4) ASF_DTA_VIDEO_PAYLOAD asfDtaPayload =
    {
        0x81,               /* stream_number = 0x81, StreamNumber = 1, KeyFrameBit = 1, (video) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x09,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */

		0x00,           /* interlace and top field first */

        0x0000,         /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

#if 0
    u8 mpeg4VOPHeader[7] =
    {
        0x00, 0x00, 0x01, 0xB6,
        0x5e, 0xa6, 0x13
    };
#else
#if(VIDEO_CODEC_OPTION == H264_CODEC)
    u8 h264SliceHeader[12] =
    {
        0x00
    };
#endif

#if(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    u8 mpeg4VOPHeader[512] =
    {
        0x00
    };
#endif


#endif

    //DEBUG_ASF("D");

    chunkSize   = 0;
#if (VIDEO_CODEC_OPTION == H264_CODEC)
    H264PutDummyHeader(pVideoClipOption->mpeg4Width,pVideoClipOption->mpeg4Height,(u8*)h264SliceHeader, &chunkSize,frame_num);
#endif

#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    mpeg4PutDummyVOPHeader(pVideoClipOption->mpeg4Width,pVideoClipOption->mpeg4Height,(u8*)mpeg4VOPHeader, &chunkSize);
#endif


    if(pVideoClipOption->ASF_set_interlace_flag==0)
    {
        asfDtaPayload.flag =  0xc0; 			/* interlace and top field first */
    }

    pVideoClipOption->PayloadType             = 1;
    pVideoClipOption->asfDataPacketFormatFlag = 1;                        //Lsk 090309
    chunkFlag               = !FLAG_I_VOP;
    chunkTime               = time;
#if (VIDEO_CODEC_OPTION == H264_CODEC)    
    pChunkBuf               = (u8*)h264SliceHeader;
#endif

#if(VIDEO_CODEC_OPTION == MPEG4_CODEC)    
    chunkSize               = sizeof(mpeg4VOPHeader);
#endif
	if(pVideoClipOption->ResetPayloadPresentTime == 0)
	{
		pVideoClipOption->ResetPayloadPresentTime = 1;
        pVideoClipOption->asfVidePresentTime   -= DUMMY_FRAME_DURATION;  //otherwise the Start Time of Video stream will keep adding 0.1sec for every file.
        
	    if(pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime)
        {
    		pVideoClipOption->asfVidePresentTime -= (pVideoClipOption->asfAudiPresentTime - pVideoClipOption->AV_TimeBase);
	    	pVideoClipOption->asfAudiPresentTime = pVideoClipOption->AV_TimeBase;
        }
        else
        {
            pVideoClipOption->asfAudiPresentTime     -= (pVideoClipOption->asfVidePresentTime - pVideoClipOption->AV_TimeBase);
    		pVideoClipOption->asfVidePresentTime      = pVideoClipOption->AV_TimeBase;
        }
	}
	pVideoClipOption->asfVidePresentTime     += chunkTime;    //Lsk 090409 : add chunkTime before write video payload
    pVideoClipOption->asfTimeStatistics      += chunkTime;
    /* corresponding to payload header, if left size of data packet is not efficiently used, just padding the left size */
    if (pVideoClipOption->asfDataPacketLeftSize < (sizeof(ASF_DTA_VIDEO_PAYLOAD) + ASF_PADDING_THRESHOLD) || pVideoClipOption->asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET)  //Lsk 090410
    {
        /* new a data packet */
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
            return 0;
    }

    streamNumber = 0x01;

    pVideoClipOption->asfDataPacketNumPayload++;
    /* write video payload */
    payloadSize                             = sizeof(ASF_DTA_VIDEO_PAYLOAD) + chunkSize;
    subPayloadSize                          = (payloadSize <= pVideoClipOption->asfDataPacketLeftSize) ? payloadSize : pVideoClipOption->asfDataPacketLeftSize; /* subpayload size with header */
    payloadSize                            -= subPayloadSize; /* left size of payload */
    subPayloadSize                         -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
    offsetIntoMediaObject                   = 0;
    /* write video payload header */
    asfDtaPayload.stream_number             = streamNumber;		
    asfDtaPayload.media_object_number       = (u8) (pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount);
    asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
    asfDtaPayload.replicated_data_length    = 0x09;
    asfDtaPayload.replicated_data.lo        = chunkSize;
    asfDtaPayload.replicated_data.hi        = pVideoClipOption->asfVidePresentTime;
    asfDtaPayload.payload_length            = (u16) subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
    pVideoClipOption->asfMassWriteDataPoint    += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
        return 0;
#endif
    pVideoClipOption->asfDataPacketLeftSize    -= sizeof(ASF_DTA_VIDEO_PAYLOAD);
    /* write video payload data */
    offsetIntoMediaObject                      += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
    pVideoClipOption->asfMassWriteDataPoint    += subPayloadSize;
#else
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
        return 0;
#endif
    pChunkBuf                                  += subPayloadSize;
    pVideoClipOption->asfDataPacketLeftSize    -= subPayloadSize;

    while (payloadSize > 0)
    {
        /* new a data packet */
        if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) /* new a data packet */
            return 0;
        pVideoClipOption->asfDataPacketFormatFlag = 1;                        //Lsk 090309
        if (chunkFlag & FLAG_I_VOP)
            pVideoClipOption->asfIndexEntryPacketCount++;

        pVideoClipOption->asfDataPacketNumPayload++;
        /* write subsequent video payload */
        payloadSize                        += sizeof(ASF_DTA_VIDEO_PAYLOAD);
        subPayloadSize                      = (payloadSize <= pVideoClipOption->asfDataPacketLeftSize) ? payloadSize : pVideoClipOption->asfDataPacketLeftSize; /* subpayload size with header */
        payloadSize                        -= subPayloadSize; /* left size of payload */
        subPayloadSize                     -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
        /* write video payload header */

        asfDtaPayload.stream_number             = streamNumber;
        asfDtaPayload.media_object_number       = (u8) (pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount);
        asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
        asfDtaPayload.replicated_data_length    = 0x09;
        asfDtaPayload.replicated_data.lo        = chunkSize;
        asfDtaPayload.replicated_data.hi        = pVideoClipOption->asfVidePresentTime;
        asfDtaPayload.payload_length            = (u16) subPayloadSize;

#if ASF_MASS_WRITE
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
        pVideoClipOption->asfMassWriteDataPoint  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
            return 0;
#endif

        pVideoClipOption->asfDataPacketLeftSize  -= sizeof(ASF_DTA_VIDEO_PAYLOAD);


        /* write video payload data */
        offsetIntoMediaObject  += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
        CopyMemory(pVideoClipOption->asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
        pVideoClipOption->asfMassWriteDataPoint  += subPayloadSize;
#else
        if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
            return 0;
#endif
        pChunkBuf              += subPayloadSize;
        pVideoClipOption->asfDataPacketLeftSize  -= subPayloadSize;
    }

    /* set index entry on each 1 sec interval */
    while (pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfIndexEntryTime)    //Lsk 090309 preroll index object
    {
        pVideoClipOption->asfIndexTable[pVideoClipOption->asfIndexTableIndex].packet_number     = pVideoClipOption->asfIndexEntryPacketNumber;
        pVideoClipOption->asfIndexTable[pVideoClipOption->asfIndexTableIndex++].packet_count    = pVideoClipOption->asfIndexEntryPacketCount;
        pVideoClipOption->asfIndexEntryTime                                                    += 1000; /* next index after 1 s = 1000 ms */
        if (pVideoClipOption->asfIndexTableIndex >= ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
        {
            DEBUG_ASF2("Trace: Video time (%d sec) reaches limit.\n", pVideoClipOption->asfIndexTableIndex);
            return 0;
        }
    }

    /* advance the index */
    pVideoClipOption->asfDummyVopCount++;
    pVideoClipOption->DummyChunkTime++;

    return 1;
}

#endif  // FORCE_FPS

/*

Routine Description:

    Multiple channel write virtual video payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng                - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteVirtualVidePayload(VIDEO_CLIP_OPTION *pVideoClipOption, VIDEO_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    if(pMng->time > 3000 || pMng->time < 20)
    {
        DEBUG_ASF("ASF VirtualVideo time error, ori: %d\n",(u32)pMng->time);
        pMng->time = 66;
        DEBUG_ASF("ASF VirtualVideo time error %d\n",(u32)pMng->time);
    }
    chunkTime = pMng->time;
    #if(VIDEO_CODEC_OPTION == H264_CODEC)
    chunkSize = pMng->offset;
    #else
    chunkSize = pMng->size;
    #endif
    pMng->asfflag = 2;
    /* advance the index */
    pVideoClipOption->asfVidePresentTime += chunkTime;

    OS_ENTER_CRITICAL();
    pVideoClipOption->CurrentVideoSize   -= chunkSize;	
    pVideoClipOption->CurrentBufferSize  -= pMng->size;
    pVideoClipOption->CurrentVideoTime   -= chunkTime;
    OS_EXIT_CRITICAL();
    pVideoClipOption->sysVidePresentTime    += chunkTime;
  #if ASF_DEBUG_ENA
    ASF_sem_V++;
  #endif
    return 1;
}

/*

Routine Description:

    Multiple channel write virtual audio payload.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.
    pMng                - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteVirtualAudiPayload(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    chunkTime = pMng->time;
    chunkSize = pMng->size;

    /* advance the index */
    pVideoClipOption->asfAudiChunkCount++;
    pVideoClipOption->asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
    OS_ENTER_CRITICAL();
    pVideoClipOption->CurrentAudioSize   -= chunkSize;
    OS_EXIT_CRITICAL();
    //}
    pVideoClipOption->sysAudiPresentTime += chunkTime;
  #if ASF_DEBUG_ENA
    ASF_sem_A++;
    if(chunkTime != 128 || chunkSize != 2048)
        DEBUG_ASF("<AAA> 1.over duration <%d, %d>\n", chunkTime, chunkSize);
  #endif
    return 1;
}

/*-------------------------------------------------------------------------*/
/* Index object                                */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Multiple channel write index object.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfWriteIndexObject(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 size;
    u32 length;
    __align(4) ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject =
    {
        {0x90, 0x08, 0x00, 0x33,    /* object_id = ASF_Simple_Index_Object */
         0xb1, 0xe5, 0xcf, 0x11,
         0x89, 0xf4, 0x00, 0xa0,
         0xc9, 0x03, 0x49, 0xcb},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0x00989680, 0x00000000},   /* index_entry_time_interval = 0x0000000000989680 100-nanosecond = 1 second */
        0x00000005,         /* max_packet_count = 0x00000005 = maximum packet_count */
        0x00000000,         /* index_entry_count = 0x???????? */
                        /* asf_idx_simple_index_ent[0x????????] */
    };

    asfSimpleIndexObject.object_size.lo     = pVideoClipOption->asfIndexSize;
    asfSimpleIndexObject.index_entry_count  = pVideoClipOption->asfIndexTableIndex;
#if 0
    if (dcfWrite(pFile, (unsigned char*)&asfSimpleIndexObject, sizeof(ASF_SIMPLE_INDEX_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pFile, (unsigned char*)asfIndexTable, asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY), &size) == 0) //Lsk 090303
            return 0;
#else //Lucian optimize: 將index table 一次寫入,以避免破碎寫入
    memcpy((unsigned char*)pVideoClipOption->mpeg4IndexBuf, (unsigned char*)&asfSimpleIndexObject, sizeof(ASF_SIMPLE_INDEX_OBJECT));
    length=pVideoClipOption->asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY)+sizeof(ASF_SIMPLE_INDEX_OBJECT);
    memset((pVideoClipOption->mpeg4IndexBuf+length),0,512-(length%512));
    length= ((length+511)/512)*512;  //sector alignment
    if (dcfWrite(pVideoClipOption->pFile, (unsigned char*)pVideoClipOption->mpeg4IndexBuf, length, &size) == 0) //Lsk 090303
            return 0;
#endif
    return 1;
}

/*

Routine Description:
    FSM : Check if Event trigger occur in multiple channel record mode.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

*/
void MultiChannelCheckEventTrigger(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    /***********************************
    *** Check if Event trigger occur ***
    ***********************************/
    if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA) && pVideoClipOption->AlarmDetect)
    {
        DEBUG_ASF("Ch%d Alarm detect event trigger\n", pVideoClipOption->VideoChannelID);
        pVideoClipOption->EventTrigger          = CAPTURE_STATE_TRIGGER;
    }
    #if (MOTIONDETEC_ENA || HW_MD_SUPPORT || MOTION_TRIGGRT_REC)
    else if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && pVideoClipOption->MD_Diff)
    {
        DEBUG_ASF("Ch%d Motion detect event trigger\n", pVideoClipOption->VideoChannelID);
        pVideoClipOption->EventTrigger  = CAPTURE_STATE_TRIGGER;
        //MotionlessSecond                = 0;
    }
    #endif
    #if G_SENSOR_DETECT
    else if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA) && pVideoClipOption->GSensorEvent)
    {
        DEBUG_ASF("Ch%d G-Sensor detect event trigger\n", pVideoClipOption->VideoChannelID);
        pVideoClipOption->EventTrigger  = CAPTURE_STATE_TRIGGER;
    }
    #endif

    if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
    {
        //Start_MPEG4TimeStatistics = 1;
        /***********************
        *** Reset time count ***
        ***********************/
        pVideoClipOption->asfTimeStatistics     = 0;
         pVideoClipOption->LocalTimeInSec        = g_LocalTimeInSec;
         pVideoClipOption->RTCseconds            = 0;
        //MotionlessSecond  = 0;
        pVideoClipOption->asfRecTimeLenTotal    = pVideoClipOption->asfRecTimeLen;
        pVideoClipOption->asfEventTriggerTime   = 0;
        
        #if (THUMBNAIL_PREVIEW_ENA == 1)
        pVideoClipOption->SearchThumbnailIdx = (rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - 3) % VIDEO_BUF_NUM;
        rfiuRX_OpMode = rfiuRX_OpMode | RfIU_RX_OPMODE_FORCE_I;
        rfiu_SetRXOpMode_1(pVideoClipOption->RFUnit);
        rfiuRX_OpMode = rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
        #endif        
    }
}

/*

Routine Description:
    FSM : Check if Time's up in multiple channel record mode.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

*/
void MultiChannelCheckRecordTimeUP(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 CurrentTime, LocalTimeInSec;

    /*********************************************************
    ***             延長 trigger 後的錄影時間              ***
    *********************************************************/
    CurrentTime     = pVideoClipOption->asfTimeStatistics / 1000;
    if(g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec)
        LocalTimeInSec  = g_LocalTimeInSec  - pVideoClipOption->LocalTimeInSec;
    else
        LocalTimeInSec  = 0;
    if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA) && pVideoClipOption->AlarmDetect)
    {
        if((LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (pVideoClipOption->asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > pVideoClipOption->asfRecTimeLenTotal))
        {
            pVideoClipOption->asfEventTriggerTime   = CurrentTime;
            pVideoClipOption->asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(pVideoClipOption->asfRecTimeLenTotal > asfSectionTime)
                pVideoClipOption->asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF2("Ch%d extend record time to %d sec\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfRecTimeLenTotal);
        }
        pVideoClipOption->AlarmDetect           = 0;
    }
    #if (MOTIONDETEC_ENA || HW_MD_SUPPORT || MOTION_TRIGGRT_REC)
    else if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && pVideoClipOption->MD_Diff)
    {
        if((LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (pVideoClipOption->asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > pVideoClipOption->asfRecTimeLenTotal))
        {
            pVideoClipOption->asfEventTriggerTime   = CurrentTime;
            pVideoClipOption->asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(pVideoClipOption->asfRecTimeLenTotal > asfSectionTime)
                pVideoClipOption->asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF2("Ch%d extend record time to %d sec\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfRecTimeLenTotal);
        }
        pVideoClipOption->MD_Diff               = 0;
    }
    #endif
    #if G_SENSOR_DETECT
    else if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA) && pVideoClipOption->GSensorEvent)
    {
        if((LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (pVideoClipOption->asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > pVideoClipOption->asfRecTimeLenTotal))
        {
            pVideoClipOption->asfEventTriggerTime   = CurrentTime;
            pVideoClipOption->asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(pVideoClipOption->asfRecTimeLenTotal > asfSectionTime)
                pVideoClipOption->asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF2("Ch%d extend record time to %d sec\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfRecTimeLenTotal);
        }
        pVideoClipOption->GSensorEvent          = 0;
    }
    #endif

    /*********************************************************
    *** Time's up!  Store current MPEG4 Encoder WriteIndex ***
    *********************************************************/
    #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
    if((CurrentTime >= pVideoClipOption->asfRecTimeLenTotal) || (LocalTimeInSec >= (pVideoClipOption->asfRecTimeLenTotal + 5)))
    {
        pVideoClipOption->EventTrigger = CAPTURE_STATE_TIMEUP;
    }
    #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)
    if((MotionlessSecond > MotionlessRecTimeLen)||(RTCseconds > asfRecTimeLen))
    {
        Last_VideoBufMngReadIdx = VideoBufMngWriteIdx;
        EventTrigger            = CAPTURE_STATE_TIMEUP;
    }
    #endif
}

/*

Routine Description:
    FSM : Check if Write finish in multiple channel record mode.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

*/
void MultiChannelCheckWriteFinish(VIDEO_CLIP_OPTION *pVideoClipOption)
{

    pVideoClipOption->EventTrigger  = CAPTURE_STATE_WAIT;
    pVideoClipOption->AlarmDetect   = 0;

#if (MOTIONDETEC_ENA)
    pVideoClipOption->MD_Diff       = 0;
    MD_FullTVRun                    = 1;
    MD_HalfTVRun                    = 1;
    MD_PanelRun                     = 1;
#elif HW_MD_SUPPORT
    pVideoClipOption->MD_Diff       = 0;
#elif MOTION_TRIGGRT_REC
    pVideoClipOption->MD_Diff       = 0;
#endif

#if (G_SENSOR_DETECT)
    pVideoClipOption->GSensorEvent  = 0;
#endif

#if PREVIEW_MD_TRIGGER_REC
    uiCaptureVideoStop();
#endif

    if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
    {
        pVideoClipOption->end_idx = pVideoClipOption->VideoBufMngReadIdx;
    }
}


/*

Routine Description:

    Multiple channel create ASF file.

Arguments:

    flag                - 1: reset audio,video timestamp for pre-reord, 0: Otherwise.
    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0           - Failure.
    Otherwose   - File point.

*/
FS_FILE* MultiChannelAsfCreateFile(u8 flag, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8              level;
    u32             i;
  #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int    cpu_sr = 0;                    /* Prevent compiler warning                           */
  #endif

    DEBUG_ASF2("Ch%d Create ASF file\n", pVideoClipOption->VideoChannelID);

#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    uiOsdDrawSysPerRec(pVideoClipOption);
#endif
    
    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
    {
        DEBUG_ASF("Main storage not ready.\n");
        /*
        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
        #endif
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        }
        */
        return 0;
    }

    OS_ENTER_CRITICAL();
    pVideoClipOption->RTCseconds                = 0;
    OS_EXIT_CRITICAL();

    pVideoClipOption->Start_asfTimeStatistics   = 0;
    if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
    {
        pVideoClipOption->asfTimeStatistics         = 0;
        pVideoClipOption->LocalTimeInSec            = g_LocalTimeInSec;
    }

    pVideoClipOption->asfDataPacketCount        = 0;
    pVideoClipOption->asfIndexTableIndex        = 0;
    pVideoClipOption->asfIndexEntryTime         = 0;
    pVideoClipOption->asfVopCount               = 0;
	pVideoClipOption->asfDummyVopCount          = 0;
#if FORCE_FPS
    pVideoClipOption->asfDummyVopCount          = 0;
    pVideoClipOption->DummyChunkTime            = 0;
#endif
	pVideoClipOption->asfDataPacketPreSendTime  = 0;    //Lsk 090519
    pVideoClipOption->asfDataPacketSendTime     = 0;
    pVideoClipOption->asfDataPacketFormatFlag   = 0;
#ifdef ASF_AUDIO
	if(flag)  //Lsk 090519  : reset audio,video timestamp for pre-reord
#else
	if(1)  //Lsk 090519  : reset audio,video timestamp for pre-reord
#endif
	{
        if(!((pVideoClipOption->asfVidePresentTime == PREROLL) && (pVideoClipOption->asfAudiPresentTime == PREROLL))){ //the first file don't need to -100ms
            pVideoClipOption->asfVidePresentTime   -= DUMMY_FRAME_DURATION;  //otherwise the Start Time of Video stream will keep adding 0.1sec for every file. 
        }

        if(pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfAudiPresentTime){
    		pVideoClipOption->asfVidePresentTime   -= (pVideoClipOption->asfAudiPresentTime - PREROLL);
	    pVideoClipOption->asfAudiPresentTime        = PREROLL;
        }
        else if(pVideoClipOption->asfVidePresentTime < pVideoClipOption->asfAudiPresentTime){
            pVideoClipOption->asfAudiPresentTime     -= (pVideoClipOption->asfVidePresentTime - PREROLL);
    	    pVideoClipOption->asfVidePresentTime      = PREROLL;
        }
            
		pVideoClipOption->ResetPayloadPresentTime   = 1;
	}
	else
		pVideoClipOption->ResetPayloadPresentTime   = 0;
    pVideoClipOption->asfAudiChunkCount         = 0;
    pVideoClipOption->MPEG4_Error               = 0;

    //------------- for disk full control-------------//

    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (dcfGetMainStorageFreeSize() <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX) / 1024)) //Notice: K-Byte unit
    {
        DEBUG_ASF("2.Disk full!!!\n");
        DEBUG_ASF("free_size = %d KBytes\n", dcfGetMainStorageFreeSize());
         system_busy_flag    = 1;
        //uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 98 + osdYShift / 2 , OSD_Blk2, 0x00 , alpha_3);
        //osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126 + osdYShift / 2, OSD_Blk2, 0xC0, 0x00);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
        osdDrawMemFull(UI_OSD_DRAW);
#elif((SW_APPLICATION_OPTION == Standalone_Test)||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))

#elif RFIU_TEST
 
#else
        uiOsdDrawSDCardFULL(UI_OSD_DRAW);
#endif
        MemoryFullFlag      = TRUE;
        // 偵測到卡滿就全部關閉錄影
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if(MultiChannelGetCaptureVideoStatus(i))
                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
        }
        OSTimeDly(15);
        return 0;
    }

    //----Storge capacity control------//
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
        //Check filesystem capacity
        switch(dcfOverWriteOP)
        {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            case DCF_OVERWRITE_OP_OFF:
                break;
            case DCF_OVERWRITE_OP_01_DAYS:
            case DCF_OVERWRITE_OP_07_DAYS:
            case DCF_OVERWRITE_OP_30_DAYS:
            case DCF_OVERWRITE_OP_60_DAYS:
            	/*while(curr_free_space < DCF_OVERWRITE_THR_KBYTE)
                {
                    if(dcfOverWriteDel()==0)
                    {
                        DEBUG_DCF("Over Write delete fail!!\n");
                        return 0;
                    }
                    else
                    {
                        //DEBUG_ASF("Over Write delete Pass!!\n");
                    }
                    //due to only update global_diskInfo when clos file, so we must calculate when open file
                    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
                    curr_free_space = free_size - curr_record_space;
                }*/
                if(dcfOverWriteOP >= dcfGetTotalDirCount())
                    break;
                sysbackLowSetEvt(SYSBACKLOW_EVT_OVERWRITEDEL, dcfOverWriteOP, 0, 0, 0);
                break;
#endif
            default:
		        while((dcfGetMainStorageFreeSize() < DCF_OVERWRITE_THR_KBYTE) || (dcfGetTotalDirCount() > (DCF_DIRENT_MAX - 2)) )
		        {   // Find the oldest file pointer and delete it
			        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel start, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif
		            if(dcfOverWriteDel()==0)
		            {
		                DEBUG_DCF("Over Write delete fail!!\n");
		                return 0;
		            }
		            else
		            {
		                //DEBUG_ASF("Over Write delete Pass!!\n");
		            }
					#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel end, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif

		            DEBUG_DCF("Free Space=%d (KBytes) \n", dcfGetMainStorageFreeSize());
		        }
		        break;
		}
    }

    /*------------ create next file------------*/
    if ((pVideoClipOption->pFile = dcfCreateNextFile(DCF_FILE_TYPE_ASF, pVideoClipOption->VideoChannelID)) == NULL) {
        DEBUG_ASF("Ch%d ASF create file error!!!\n", pVideoClipOption->VideoChannelID);
        return 0;
    }

	#if VIDEO_STARTCODE_DEBUG_ENA
	monitor_RX[pVideoClipOption->VideoChannelID]=0;
	monitor_decode[pVideoClipOption->VideoChannelID]=0;
	monitor_ASF_1[pVideoClipOption->VideoChannelID]=0;
	monitor_ASF_2[pVideoClipOption->VideoChannelID]=0;
	#endif

	
	pVideoClipOption->asfHeaderSize = 0;
	pVideoClipOption->asfDataSize = 0;
	pVideoClipOption->asfIndexSize = 0;
	
    /* write header object */
    if (MultiChannelAsfWriteHeaderObject(pVideoClipOption) == 0) {
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        return 0;
    }

    /* write data object pre */
    if (MultiChannelAsfWriteDataObjectPre(pVideoClipOption) == 0) {
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        return 0;
    }

    /* capture and write data */

    /* write first data packet */
    if (MultiChannelAsfWriteDataPacketPre(pVideoClipOption) == 0) {
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        return 0;
    }

    pVideoClipOption->OpenFile  = 1;

    return pVideoClipOption->pFile;
}


#if (VIDEO_CODEC_OPTION == H264_CODEC)
/*

Routine Description:

    Multiple channel asf file format packer.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfCaptureVideo(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u16             video_value;
    u16             video_value_max;
    u32             monitor_value;
    u32             timetick;

#ifdef  ASF_AUDIO
    u16             audio_value;
    u16             audio_value_max;
#endif

    u32             CurrentFileSize;
    u32             DummySize;
    u8              err;
  #if INSERT_NOSIGNAL_FRAME
    u8				audio_index,video_index;
  #endif
#if FINE_TIME_STAMP
    s32             TimeOffset;
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    u32             time1;
#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    s32             PcmOffset                   = 0;
#endif

    //s32             SkipFrameNum;
    //u32             PreRecordFrameNum;

    u32             MaxPacketCount;
    u8              TriggerModeFirstFileFlag    = 0;
    u8              InitDACPlayFlag             = 0;

    u32             SingleFrameSize;
    u32             FreeSpaceThreshold;
    u32             FreeSpace;
    u32             timeoutcnt                  = 0;
    u8              level;
    u32             size;
    u32             wait_Iframe_delay_cnt       = 0;
  #if (OS_CRITICAL_METHOD == 3)                 /* Allocate storage for CPU status register           */
    unsigned int    cpu_sr = 0;                 /* Prevent compiler warning                           */
  #endif
    u32             i;
    u32             RunVideo                    = 0;
    u32             RunAudio                    = 0;
    
    u8              write_audio_back_cnt        = 0;
    
    pVideoClipOption->sysAudiPresentTime = 0;
    pVideoClipOption->sysVidePresentTime = 0;
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    Lose_audio_time[pVideoClipOption->RFUnit] = 0;
    Lose_video_time[pVideoClipOption->RFUnit] = 0;
    Audio_RF_index[pVideoClipOption->RFUnit] = 0;
    Video_RF_index[pVideoClipOption->RFUnit] = 0;
    V_flag[pVideoClipOption->RFUnit] = 0;
    A_flag[pVideoClipOption->RFUnit] = 0;
  #endif
#if MULTI_CH_SDC_WRITE_TEST
    return TestFileSystem(pVideoClipOption);
#endif

    if (pVideoClipOption->asfCaptureMode == ASF_CAPTURE_NORMAL)
        DEBUG_ASF("Ch%d ASF_CAPTURE_NORMAL\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_OVERWRITE_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_GSENSOR_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_MOTION_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_ALARM_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_DUMMY_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_DUMMY_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_SCHEDULE_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_SCHEDULE_ENA\n", pVideoClipOption->VideoChannelID);
    #if  RFIU_RX_WAKEUP_TX_SCHEME
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_PIRTIGGER_ENA){
        DEBUG_ASF("Ch%d ASF_CAPTURE_PIRTIGGER_ENA %d\n", pVideoClipOption->VideoChannelID,gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BatCam_PIRMode );
        if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
        {
                pVideoClipOption->sysCaptureVideoStop = 1;
                pVideoClipOption->sysCaptureVideoStart = 0;
        }
    }
    #endif
    /***********************************************************
    *** calculate max packet count : file size / packet size ***
    ***********************************************************/
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
        #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = TriggerModeGetMaxPacketCount();
        #endif
    } else {
        #if(MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = ManualModeGetMaxPacketCount();
        #endif
    }

#if 0
    /****************************************************************
    *** calculate how many frames need for pre-record             ***
    *** PreRecordFrameNum = PreRecordTime * FPS                   ***
    ****************************************************************/
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
    {
        if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            PreRecordFrameNum = PreRecordTime * 30;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            PreRecordFrameNum = PreRecordTime * 15;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            PreRecordFrameNum = PreRecordTime * 5;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_60)
            PreRecordFrameNum = PreRecordTime * 60;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
            PreRecordFrameNum = PreRecordTime * 10;

    #if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
        PreRecordFrameNum = PreRecordTime * 15;
    #endif
    }
#endif
    /*********************
    *** reset variable ***
    *********************/

    /////////////////////////////////////////////////////////////////////////////
    // asfCaptureVideo() initial value

    pVideoClipOption->asfVideoFrameCount        = 0;
    pVideoClipOption->asfDataPacketPreSendTime  = 0;
    pVideoClipOption->asfDataPacketSendTime     = 0;
    pVideoClipOption->asfDataPacketFormatFlag   = 0;
    pVideoClipOption->VideoPictureIndex         = 0;
    //pVideoClipOption->VideoBufMngReadIdx        = 0;
    //pVideoClipOption->VideoBufMngWriteIdx       = 0;
    pVideoClipOption->asfVopCount               = 0;
#if FORCE_FPS
    pVideoClipOption->asfDummyVopCount          = 0;
    pVideoClipOption->DummyChunkTime            = 0;
#endif
    pVideoClipOption->asfVidePresentTime        = PREROLL; //Lsk 090309
    pVideoClipOption->MPEG4_Mode                = 0;    // 0: record, 1: playback
    OS_ENTER_CRITICAL();
    pVideoClipOption->CurrentVideoSize          = 0;
    pVideoClipOption->CurrentBufferSize         = 0;
    pVideoClipOption->CurrentVideoTime          = 0;
    pVideoClipOption->VideoTimeStatistics       = 0;
    OS_EXIT_CRITICAL();
    pVideoClipOption->asfIndexTable             = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(pVideoClipOption->mpeg4IndexBuf+sizeof(ASF_SIMPLE_INDEX_OBJECT));
	/*
    for(i = 0; i < VIDEO_BUF_NUM; i++) {
        pVideoClipOption->VideoBufMng[i].buffer = pVideoClipOption->VideoBuf;
    }
    */
    //mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);   /*Peter 1116 S*/
    memset(pVideoClipOption->ISUFrameDuration, 0, sizeof(pVideoClipOption->ISUFrameDuration));

#ifdef ASF_AUDIO
    /* audio */
    pVideoClipOption->asfAudiChunkCount         = 0;
    pVideoClipOption->asfAudiPresentTime        = PREROLL; //Lsk 090309
    pVideoClipOption->iisSounBufMngReadIdx      = 0;
    pVideoClipOption->iisSounBufMngWriteIdx     = 0;
    pVideoClipOption->IISMode                   = 0;    // 0: record, 1: playback
    pVideoClipOption->IISTime                   = 0;    /* Peter 070104 */
    pVideoClipOption->IISTimeUnit               = (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE;  /* milliscends */ /* Peter 070104 */
    pVideoClipOption->CurrentAudioSize          = 0;
    /* initialize sound buffer */
	/*
    for(i = 0; i < IIS_BUF_NUM; i++) {
        pVideoClipOption->iisSounBufMng[i].buffer   = pVideoClipOption->iisSounBuf[i];
    }
    */
#endif  // ASF_AUDIO

    /* file */
    pVideoClipOption->asfDataPacketCount        = 0;
    pVideoClipOption->asfIndexTableIndex        = 0;
    pVideoClipOption->asfIndexEntryTime         = 0;
    pVideoClipOption->OpenFile                  = 0;
	pVideoClipOption->asfHeaderSize = 0;
	pVideoClipOption->asfDataSize = 0;
	pVideoClipOption->asfIndexSize = 0;
#if CDVR_LOG
    pVideoClipOption->LogFileStart                                  = 0;
    pVideoClipOption->LogFileNextStart                              = 0;
    pVideoClipOption->LogFileCurrent                                = 0;
    pVideoClipOption->pLogFileEnd                                   = (u8*)0;
    pVideoClipOption->pLogFileMid                                   = (u8*)0;
    pVideoClipOption->LogFileIndex[pVideoClipOption->LogFileStart]  = pVideoClipOption->LogFileBuf;
#endif

    /////////////////////////////////////////////////////////////////////////////
    // asfCaptureVideoFile() initial value

    pVideoClipOption->asfTimeStatistics         = 0;
    pVideoClipOption->LocalTimeInSec            = g_LocalTimeInSec;
    pVideoClipOption->DirectlyTimeStatistics    = 0;
    pVideoClipOption->ResetPayloadPresentTime   = 1;
    pVideoClipOption->WantChangeFile            = 0;
    pVideoClipOption->LastAudio                 = 0;
    pVideoClipOption->LastVideo                 = 0;
    pVideoClipOption->GetLastAudio              = 0;
    pVideoClipOption->GetLastVideo              = 0;
    pVideoClipOption->EventTrigger              = CAPTURE_STATE_WAIT;
    pVideoClipOption->MD_Diff                   = 0;
    pVideoClipOption->MPEG4_Error               = 0;
    pVideoClipOption->sysReady2CaptureVideo     = 0;
    pVideoClipOption->WantToExitPreRecordMode   = 0;
    video_value                                 = 0;
    video_value_max                             = 0;
    monitor_value                               = 0;
#ifdef ASF_AUDIO
    audio_value                                 = 0;
    audio_value_max                             = 0;
#endif
    siuOpMode                                   = SIUMODE_MPEGAVI;  // fix dcfWrite() write wrong data bug

#if(RECORD_SOURCE == LOCAL_RECORD)
    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
        pVideoClipOption->VideoBufMngReadIdx        = 0;
        pVideoClipOption->VideoBufMngWriteIdx       = 0;
        for(i = 0; i < VIDEO_BUF_NUM; i++) {
            pVideoClipOption->VideoBufMng[i].buffer = pVideoClipOption->VideoBuf;
        }
#ifdef ASF_AUDIO
        pVideoClipOption->iisSounBufMngReadIdx      = 0;
        pVideoClipOption->iisSounBufMngWriteIdx     = 0;
        for(i = 0; i < IIS_BUF_NUM; i++) {
            pVideoClipOption->iisSounBufMng[i].buffer   = pVideoClipOption->iisSounBuf[i];
        }
#endif
    } 
#elif(RECORD_SOURCE == RX_RECEIVE)
    if(pVideoClipOption->AV_Source == RX_RECEIVE) {
    /*
        pVideoClipOption->VideoBufMngReadIdx        = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->VideoBufMngWriteIdx       = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
#ifdef ASF_AUDIO
        pVideoClipOption->iisSounBufMngReadIdx      = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->iisSounBufMngWriteIdx     = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
#endif
    */

    #ifdef ASF_AUDIO
        while(OSSemAccept(pVideoClipOption->iisCmpSemEvt));
        OS_ENTER_CRITICAL();

    #if  RFIU_RX_WAKEUP_TX_SCHEME
        if(gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BatCam_PIRMode == 1)
            pVideoClipOption->iisSounBufMngReadIdx  = 0;
        else
            pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
    #else
        pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
    #endif
        
        pVideoClipOption->CurrentAudioSize      = 0;
        OS_EXIT_CRITICAL();
        while(pVideoClipOption->iisCmpSemEvt->OSEventCnt || pVideoClipOption->CurrentAudioSize || (pVideoClipOption->iisSounBufMngReadIdx != rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]))
        {
            OS_ENTER_CRITICAL();
            pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
            pVideoClipOption->CurrentAudioSize      = 0;
            OS_EXIT_CRITICAL();
            OSSemAccept(pVideoClipOption->iisCmpSemEvt);
        };
    #endif
        while(OSSemAccept(pVideoClipOption->VideoCmpSemEvt));
        OS_ENTER_CRITICAL();

    #if  RFIU_RX_WAKEUP_TX_SCHEME
        if(gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BatCam_PIRMode == 1)
            pVideoClipOption->VideoBufMngReadIdx    = 0;
        else
            pVideoClipOption->VideoBufMngReadIdx = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
    #else
        pVideoClipOption->VideoBufMngReadIdx = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
    #endif
        pVideoClipOption->CurrentVideoSize      = 0;
        pVideoClipOption->CurrentBufferSize         = 0;
        pVideoClipOption->CurrentVideoTime      = 0;
        OS_EXIT_CRITICAL();
    #if  RFIU_RX_WAKEUP_TX_SCHEME
        if(gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[pVideoClipOption->RFUnit].RFpara.BatCam_PIRMode == 1)
        {
        }
        else
    #endif
        {
            while(pVideoClipOption->VideoCmpSemEvt->OSEventCnt || pVideoClipOption->CurrentBufferSize || pVideoClipOption->CurrentVideoTime || (pVideoClipOption->VideoBufMngReadIdx != rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]))
            {
                OS_ENTER_CRITICAL();
                pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                pVideoClipOption->CurrentVideoSize      = 0;
                pVideoClipOption->CurrentBufferSize     = 0;            
                pVideoClipOption->CurrentVideoTime      = 0;
                OS_EXIT_CRITICAL();
                OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
            };
        }
        // find I frame, 第一張必須是 I frame
        //DEBUG_ASF("Ch%d iisSounBufMngReadIdx        = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->iisSounBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxIIsSounBufMngWriteIdx = %d\n", pVideoClipOption->VideoChannelID, rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d VideoBufMngReadIdx          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxVideoBufMngWriteIdx   = %d\n", pVideoClipOption->VideoChannelID, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d finding I frame...\n", pVideoClipOption->VideoChannelID);
        while(pVideoClipOption->sysCaptureVideoStop == 0)
        {
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            if(Record_flag[pVideoClipOption->RFUnit] == 1)
            {
                pVideoClipOption->iisSounBufMngReadIdx = 0;
                pVideoClipOption->VideoBufMngReadIdx =0;
            }
          #endif
          #ifdef ASF_AUDIO
            if(pVideoClipOption->iisSounBufMngReadIdx != rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit])
            {
                //DEBUG_ASF(" #%d ", __LINE__);
                RunAudio        = 1;
                audio_value     = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                if(audio_value)
                    pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                else    // something wrong
                {
                    //pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
                    //DEBUG_ASF(" #A%d ", pVideoClipOption->VideoChannelID);
                    OSTimeDly(1);
                }
            } else {
                RunAudio        = 0;
            }
        #endif
            if(pVideoClipOption->VideoBufMngReadIdx != rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit])
            {
                if(wait_Iframe_delay_cnt %10 == 0)
                    DEBUG_ASF(" #%d \n", __LINE__);
                RunVideo        = 1;
                if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    //DEBUG_ASF("#");
                    if(video_value){
                        OS_ENTER_CRITICAL();
                        if(pVideoClipOption->CurrentVideoSize >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset)
                            pVideoClipOption->CurrentVideoSize   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset;
                        else 
                        {
                            DEBUG_ASF("VideoSize error %d, %d\n", pVideoClipOption->CurrentVideoSize, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset);
                            pVideoClipOption->CurrentVideoSize = 0;
                        }
                        
                        if(pVideoClipOption->CurrentBufferSize >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size)
                            pVideoClipOption->CurrentBufferSize   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size;
                        else
    			        {
                            DEBUG_ASF("BufferSize error %d, %d\n", pVideoClipOption->CurrentBufferSize, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size);
                            pVideoClipOption->CurrentBufferSize = 0;
                        }

                        if(pVideoClipOption->CurrentVideoTime >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time)
                            pVideoClipOption->CurrentVideoTime   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time;
                        else
                        {
                            DEBUG_ASF("Time error %d, %d\n", pVideoClipOption->CurrentVideoTime, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time);
                            pVideoClipOption->CurrentVideoTime = 0;
                        }

                        OS_EXIT_CRITICAL();                        
                        pVideoClipOption->VideoBufMngReadIdx    = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                    }
                    else    // something wrong
                    {
                        //pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                        DEBUG_ASF(" #V%d ", pVideoClipOption->VideoChannelID);
                        OSTimeDly(1);
                    }
                } else {
                    DEBUG_ASF("\nCh%d find I frame ok\n", pVideoClipOption->VideoChannelID);
                    break;
                }
            } else {
                RunVideo        = 0;
            }
            if(RunAudio == 0 && RunVideo == 0) {
                if(wait_Iframe_delay_cnt %10 == 0)
                    DEBUG_ASF(" #%d \n", __LINE__);
                OSTimeDly(1);
            }
            wait_Iframe_delay_cnt++;
        }
        #if (THUMBNAIL_PREVIEW_ENA == 1)
        pVideoClipOption->SearchThumbnailIdx = pVideoClipOption->VideoBufMngReadIdx;
        #endif
        DEBUG_ASF("BufSize:%d, BufTime:%d, Vcnt:%d, (%d, %d)\n", pVideoClipOption->CurrentVideoSize, pVideoClipOption->CurrentVideoTime, pVideoClipOption->VideoCmpSemEvt->OSEventCnt, pVideoClipOption->VideoBufMngReadIdx, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d iisSounBufMngReadIdx        = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->iisSounBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxIIsSounBufMngWriteIdx = %d\n", pVideoClipOption->VideoChannelID, rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d VideoBufMngReadIdx          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxVideoBufMngWriteIdx   = %d\n", pVideoClipOption->VideoChannelID, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);

    #if (VIDEO_CODEC_OPTION == H264_CODEC)
        if(pVideoClipOption->asfVopHeight == 1080)
        {
            rfiuH264Encode_I_Header(&pVideoClipOption->H264Enc_cfg,
                                    pVideoClipOption->asfVopWidth,
                                    1088);
        }
        else
        {
            rfiuH264Encode_I_Header(&pVideoClipOption->H264Enc_cfg,
                                    pVideoClipOption->asfVopWidth,
                                    pVideoClipOption->asfVopHeight);
        }
        pVideoClipOption->SPS_PPS_Length = GenerateParameterSets_SW(&pVideoClipOption->H264Enc_cfg, pVideoClipOption->SPS_PPS_Buffer, &pVideoClipOption->active_pps, &pVideoClipOption->active_sps, &pVideoClipOption->datastream);
        /*
        DEBUG_ASF("SPS_PPS_Buffer(%d):\n", pVideoClipOption->SPS_PPS_Length);
        for(i = 0; i < pVideoClipOption->SPS_PPS_Length; i++)
            DEBUG_ASF("%02X ", pVideoClipOption->SPS_PPS_Buffer[i]);
        DEBUG_ASF("\n");
        */
    #endif

        if(pVideoClipOption->sysCaptureVideoStop)
        {
          #if INSERT_NOSIGNAL_FRAME
            OS_ENTER_CRITICAL();
            Record_flag[pVideoClipOption->VideoChannelID] = 0;
            OS_EXIT_CRITICAL();
          #endif
            DEBUG_ASF("Ch%d stop video record!!!\n", pVideoClipOption->VideoChannelID);
            return 1;
        }
    }
#endif //end of if(RECORD_SOURCE == RX_RECEIVE)

    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL))
    {
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (dcfGetMainStorageFreeSize() <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX) / 1024)) //Notice: K-Byte unit
        {
            DEBUG_ASF("1.Disk full!!!\n");
            DEBUG_ASF("free_size = %d KBytes\n", dcfGetMainStorageFreeSize());
             system_busy_flag    = 1;
            //uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 98 + osdYShift / 2 , OSD_Blk2, 0x00 , alpha_3);
            //osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126 + osdYShift / 2, OSD_Blk2, 0xC0, 0x00);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            osdDrawMemFull(UI_OSD_DRAW);
#elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))

#elif RFIU_TEST

#else
            uiOsdDrawSDCardFULL(UI_OSD_DRAW);
#endif
            MemoryFullFlag      = TRUE;
            // 偵測到卡滿就全部關閉錄影
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                if(MultiChannelGetCaptureVideoStatus(i))
                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
            }
            OSTimeDly(15);
            return 0;
        }
        pVideoClipOption->SetIVOP = 1;
        // first file for Continuous Record
        if(MultiChannelAsfCreateFile(1, pVideoClipOption) == 0){

            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
					DEBUG_ASF("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    if(MultiChannelGetCaptureVideoStatus(i))
                    {
                    	DEBUG_ASF("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                }

        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
        #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))


        #elif RFIU_TEST

        #else
                if(MemoryFullFlag == TRUE)
                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                else
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
        #endif
             }
            return 0;
        }
    }

#if(RECORD_SOURCE == LOCAL_RECORD)
	if(pVideoClipOption->AV_Source == LOCAL_RECORD)
	{
	#if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
	    timerCountRead(2, (u32*) &TimeOffset);
    	pVideoClipOption->IISTimeOffset = TimeOffset >> 8;
	#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
		timerCountRead(1, (u32*) &TimeOffset);
	    pVideoClipOption->IISTimeOffset = TimeOffset / 100;
	#endif

	#ifdef ASF_AUDIO
        MultiChannelIISRecordTaskCreate(pVideoClipOption);
        adcInit(1);
      #if AUDIO_IN_TO_OUT
        iisResumePlaybackTask();
      #endif
    #endif  // #ifdef ASF_AUDIO

	    switch(pVideoClipOption->VideoChannelID)
	    {
	        case 0:
	            isuCaptureVideo(0);
	            ipuCaptureVideo();
	            siuCaptureVideo(0);
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(isu_avifrmcnt < 4)
	            {
	               DEBUG_ASF("asf w01\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t01\n");
	                   break;
	               }

	            }
	            isu_avifrmcnt           = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(isuSemEvt, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            isu_avifrmcnt           = 0;
	            timeoutcnt              = 0;
	            while(isu_avifrmcnt < 1)
	            {
	                DEBUG_ASF("asf w02\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt > 5)
	                {
	                    DEBUG_ASF("asf t02\n");
	                    DEBUG_ASF("Error: timeout 02\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(isu_avifrmcnt < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 1:
	            #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                 #if(IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE)  // 避免CIU Bob mode切換時 size會有變化
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, 800 * 2);
                    else
                       ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, 800 * 2);
                    #else
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(640, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                #endif
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(704, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(704, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(720, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(720, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
	            #else
	            ciuCaptureVideo_CH1(isuSrcImg.w, isuSrcImg.h, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height, CIU1_OSD_EN, pVideoClipOption->mpeg4Width);
	            #endif
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch1 < 4)
	            {
	               DEBUG_ASF("asf w11\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t11\n");
	                   break;
	               }

	            }
	            ciu_idufrmcnt_ch1       = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(ciuCapSemEvt_CH1, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            ciu_idufrmcnt_ch1       = 0;
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch1 < 1)
	            {
	                DEBUG_ASF("asf w2\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt > 5)
	                {
	                    DEBUG_ASF("asf t12\n");
	                    DEBUG_ASF("Error: timeout 12\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(ciu_idufrmcnt_ch1 < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 2:
	            #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                    if(sysTVinFormat == TV_IN_PAL)
                    #if(IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE )  // 避免CIU Bob mode切換時 size會有變化
                       ciuCaptureVideo_CH2(640, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    #else
                       ciuCaptureVideo_CH2(640, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    #endif
                    else
    	               ciuCaptureVideo_CH2(640, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH2(704, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH2(704, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH2(720, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH2(720, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
	            #else
	            ciuCaptureVideo_CH2(isuSrcImg.w, isuSrcImg.h, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height, CIU2_OSD_EN, pVideoClipOption->mpeg4Width);
	            #endif
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch2 < 4)
	            {
	               DEBUG_ASF("asf w21\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t21\n");
	                   break;
	               }

	            }
	            ciu_idufrmcnt_ch2       = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(ciuCapSemEvt_CH2, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            ciu_idufrmcnt_ch2       = 0;
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch2 < 1)
	            {
	                DEBUG_ASF("asf w22\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt>5)
	                {
	                    DEBUG_ASF("asf t22\n");
	                    DEBUG_ASF("Error: timeout 22\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(ciu_idufrmcnt_ch2 < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 3:
	        default:
	            DEBUG_ASF("Errpr: MultiChannelAsfCaptureVideo can't support Ch%d !!!\n", pVideoClipOption->VideoChannelID);
	    }

    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        MultiChannelMPEG4EncoderTaskCreate(pVideoClipOption);
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
        MultiChannelVideoEncoderTaskCreate(pVideoClipOption);
    #endif
	} 
#else
    {    // if(pVideoClipOption->AV_Source != LOCAL_RECORD)
    	siuOpMode   = SIUMODE_MPEGAVI;
    }
#endif //end of if(RECORD_SOURCE == RX_RECEIVE)
    while (pVideoClipOption->sysCaptureVideoStop == 0)
    {
      //update the lastest capture mode, such as overwrite, motion when UI has changed its settings
      if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) != (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA)){
          DEBUG_ASF("[%s] Ch:%d, CaptureMode: %d -> %d\n", __FUNCTION__, pVideoClipOption->VideoChannelID, pVideoClipOption->asfCaptureMode, sysCaptureVideoMode);
          if(sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA)
            pVideoClipOption->asfCaptureMode |= ASF_CAPTURE_OVERWRITE_ENA;
          else
            pVideoClipOption->asfCaptureMode &= !ASF_CAPTURE_OVERWRITE_ENA;
      }
      
      #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
        if((pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT) && (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && (Record_flag[pVideoClipOption->RFUnit] == 1))
        {
            pVideoClipOption->sysCaptureVideoStart	= 0;
            pVideoClipOption->sysCaptureVideoStop	= 1;
            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
            DEBUG_ASF("EventTrigger  %d asfCaptureMode %d\n ", pVideoClipOption->EventTrigger,pVideoClipOption->asfCaptureMode);
            break;
        }
      #endif

      #if(RECORD_SOURCE == LOCAL_RECORD)
        if((pVideoClipOption->AV_Source == LOCAL_RECORD) && (pVideoClipOption->MPEG4_Error == 1))
        {
            MultiChannelAsfCloseFile(pVideoClipOption);
          #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
          #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
          #endif
          #ifdef  ASF_AUDIO
            //MultiChannelIIsStopRec(pVideoClipOption);
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
          #endif
            return 0;
        }
      #endif
        /**********************************************************************************************************************
        *** Trigger mode FSM                                                                                                ***
        *** CAPTURE_STATE_WAIT --------> CAPTURE_STATE_TRIGGER --------> CAPTURE_STATE_TIMEUP --------> CAPTURE_STATE_WAIT  ***
        *** WAIT    -> TRIGGER : event trigger, start wrtite A/V bitstream                                                  ***
        *** TRIGGER -> TIMEUP  : Time's up, store lastest video payload index                                               ***
        *** TIMEUP  -> WAIT    : Write Finish film slice, return to wating state                                            ***
        **********************************************************************************************************************/
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
    		{
    		    //Start_MPEG4TimeStatistics = 0;
    		    MultiChannelCheckEventTrigger(pVideoClipOption);
                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_TRIGGER\n\n", pVideoClipOption->VideoChannelID);
                  #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    Lose_audio_time[pVideoClipOption->RFUnit]=0;
                    Lose_video_time[pVideoClipOption->RFUnit]=0;
                  #endif
                  #if 0
                    /****************************************************************
                    *** Calculate how many VOP in SDRAM need to drop              ***
                    ****************************************************************/
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        OS_ENTER_CRITICAL();
                        if(pVideoClipOption->VideoBufMngWriteIdx >= pVideoClipOption->VideoBufMngReadIdx)
                        {
                            //---R---s--W---
                            //if((pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum >= pVideoClipOption->VideoBufMngReadIdx)&&(pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum <= pVideoClipOption->VideoBufMngWriteIdx))
                            if(pVideoClipOption->VideoBufMngWriteIdx >= (pVideoClipOption->VideoBufMngReadIdx + PreRecordFrameNum))
                            {
                                SkipFrameNum = pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---R---W--s---
                            //---s---R--W---
                            else
                                SkipFrameNum = 0;
                        }
                        else
                        {
                            //---s---w--R---
                            //---w---R--s---
                            if( (VIDEO_BUF_NUM + pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum) >= pVideoClipOption->VideoBufMngReadIdx)
                            {
                                SkipFrameNum = VIDEO_BUF_NUM + pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---w---s--R---
                            else
                                SkipFrameNum = 0;
                        }
                        OS_EXIT_CRITICAL();
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        OS_ENTER_CRITICAL();
                        if(rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] >= pVideoClipOption->VideoBufMngReadIdx)
                        {
                            //---R---s--W---
                            if(rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] >= (pVideoClipOption->VideoBufMngReadIdx + PreRecordFrameNum))
                            {
                                SkipFrameNum = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---R---W--s---
                            //---s---R--W---
                            else
                                SkipFrameNum = 0;
                        }
                        else
                        {
                            //---s---w--R---
                            //---w---R--s---
                            if( (VIDEO_BUF_NUM + rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum) >= pVideoClipOption->VideoBufMngReadIdx)
                            {
                                SkipFrameNum = VIDEO_BUF_NUM + rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---w---s--R---
                            else
                                SkipFrameNum = 0;
                        }
                        OS_EXIT_CRITICAL();
                    }
                  #endif
                    /******************************************
                    *** Force Mpeg4 Engine compress I frame ***
                    ******************************************/
                    if(pVideoClipOption->DirectlyTimeStatistics == 0)
                    {
                        pVideoClipOption->SetIVOP = 1;
                        OSTimeDly(2);
                    }
                    /***************************
                    *** drop Audio and video ***
                    ***************************/
                    if(pVideoClipOption->CurrentBufferSize && pVideoClipOption->FreeSpaceControl)
                    {
                        SingleFrameSize     = pVideoClipOption->CurrentBufferSize / (pVideoClipOption->VideoCmpSemEvt->OSEventCnt);
                        FreeSpaceThreshold  = 4 * 30 * SingleFrameSize + MPEG4_MIN_BUF_SIZE;
                        FreeSpace           = MPEG4_MAX_BUF_SIZE - pVideoClipOption->CurrentBufferSize;
                         DEBUG_ASF("FreeSpace control (%d,%d)\n", FreeSpaceThreshold, FreeSpace);
                    }
                    else
                        pVideoClipOption->FreeSpaceControl = 0;

                    //DEBUG_ASF("SkipFrameNum = %d\n", SkipFrameNum);

                    
                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                        DEBUG_ASF("Main storage not ready.\n");
                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #endif
                        return 0;
                    }
                  #if ASF_DEBUG_ENA
                    DEBUG_ASF("************************ Trigger occur - 001 ************************\n");
                    {
                    	RTC_DATE_TIME   dateTime;					
                    	RTC_Get_Time(&dateTime);			
                    	DEBUG_ASF("<TTT> Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
                    }
                    skip_I=0;
                    skip_P=0;
                    skip_A=0;

                    DEBUG_ASF("<TTT>1. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                    DEBUG_ASF("<TTT>1. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                    DEBUG_ASF("<TTT>1. pVideoClipOption->CurrentBufferSize           = %d\n", pVideoClipOption->CurrentBufferSize);		
                    DEBUG_ASF("<TTT>1. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                    DEBUG_ASF("<TTT>1. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                  #endif
                    //while((pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag != FLAG_I_VOP) || (SkipFrameNum > 0)
                    while((pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
                        || (pVideoClipOption->CurrentBufferSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE))
                        || (pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000))
                        #if CHECK_VIDEO_BITSTREAM
						|| (*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer) != 0x01000000)
						|| (*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4) != 0x00000001)
						#else
						|| (!((*(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer)==0x00)&&(*(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer+1)==0x00)&&(*(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer+2)==0x00)&&(*(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer+3)==0x01)))                                               
						#endif
                        || (pVideoClipOption->FreeSpaceControl && (FreeSpace < FreeSpaceThreshold)))
                    {
                        //SkipFrameNum--;                        
                        if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
							DEBUG_ASF("CH%d, %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
                        if(pVideoClipOption->CurrentBufferSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE))
                            DEBUG_ASF("CH%d, %d, size:%d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,(u32)pVideoClipOption->CurrentBufferSize);
                        if(pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000))
                            DEBUG_ASF("CH%d, %d, time:%d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,pVideoClipOption->CurrentVideoTime);
                        if(pVideoClipOption->FreeSpaceControl && (FreeSpace < FreeSpaceThreshold))
		                    DEBUG_ASF("CH%d, %d, %d %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,FreeSpace,FreeSpaceThreshold);

						#if CHECK_VIDEO_BITSTREAM
						if((pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP) || (pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_P_VOP))
						{
							if(*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer) != 0x01000000)
								DEBUG_ASF("CH%d, %d, %d, 0x%08x\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag, *(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer));
							if(*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4) != 0x00000001)
								DEBUG_ASF("CH%d, %d, %d, 0x%08x\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag, *(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4));										
						}
						#endif

                      #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                        if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                            Record_flag[pVideoClipOption->RFUnit] = 1;
                        
                        if(Record_flag[pVideoClipOption->RFUnit] == 1)
                        {
                            pVideoClipOption->sysCaptureVideoStart	= 0;
                            pVideoClipOption->sysCaptureVideoStop	= 1;
                            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                            break;
                        }
                      #endif
                        video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                        if (video_value > 0) 
                        {
                          #if ASF_DEBUG_ENA
                            if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                            	skip_I++;
                            else
                            	skip_P++;
                          #endif
                            if(video_value_max < video_value)
                                video_value_max = video_value; ////Lsk : for what ?
                            #if CDVR_LOG
                            if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                                pVideoClipOption->LogFileStart      = (pVideoClipOption->LogFileStart + 1) % LOG_INDEX_NUM;
                            #endif
                            MultiChannelAsfWriteVirtualVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx]);
                            pVideoClipOption->VideoBufMngReadIdx    = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                                OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                         #endif
                            if(pVideoClipOption->FreeSpaceControl)
                                FreeSpace = MPEG4_MAX_BUF_SIZE - pVideoClipOption->CurrentBufferSize;
                        }
                        else
                        {
                        	#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
        			    DEBUG_ASF("\n\nCH%d Can't start from I frame %d, <%d, %d, %d> (%d) !!!\n\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->PackerTaskCreated, pVideoClipOption->sysCaptureVideoStart, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->VideoChannelID], pVideoClipOption->VideoCmpSemEvt->OSEventType);							
				#else
        		           DEBUG_ASF("\n\nCH%d Can't start from I frame (ReadIdx:%d, WriteIdx:%d)!!!\n\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
				#endif
                            OS_ENTER_CRITICAL();
                            //reset video
                            while(OSSemAccept(pVideoClipOption->VideoCmpSemEvt));                
                            pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                            pVideoClipOption->CurrentVideoSize      = 0;
                            pVideoClipOption->CurrentVideoTime      = 0;

                            //reset audio
                            while(OSSemAccept(pVideoClipOption->iisCmpSemEvt));
                            pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
                            pVideoClipOption->CurrentAudioSize      = 0;
                            
                            pVideoClipOption->CurrentBufferSize     = 0;                            
                            OS_EXIT_CRITICAL();

                            pVideoClipOption->EventTrigger  = CAPTURE_STATE_WAIT;
                            break;
                        }


                        
                        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                        {
                            DEBUG_ASF("Main storage not ready.\n");
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #endif
                            return 0;
                        }
                    }
                  #if ASF_DEBUG_ENA
                    DEBUG_ASF("<TTT>2. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);										
                    DEBUG_ASF("<TTT>2. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                    DEBUG_ASF("<TTT>2. pVideoClipOption->CurrentBufferSize           = %d\n", pVideoClipOption->CurrentBufferSize);	
                    DEBUG_ASF("<TTT>2. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                    DEBUG_ASF("<TTT>2. skip <%d, %d>\n"                                    , skip_I, skip_P);	
                    DEBUG_ASF("<TTT>2. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                  #endif
                    pVideoClipOption->start_idx     = pVideoClipOption->VideoBufMngReadIdx;

                    if(pVideoClipOption->start_idx != pVideoClipOption->end_idx)
                    {
                        DEBUG_ASF("\n Warning!!! Ch%d lose video slice....\n", pVideoClipOption->VideoChannelID);
                    }
               #ifdef ASF_AUDIO                   
                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                        DEBUG_ASF("Main storage not ready.\n");
                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #endif
                        return 0;
                    }
                  #if INSERT_NOSIGNAL_FRAME
                    while ((pVideoClipOption->sysAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->sysVidePresentTime)
                  #else
                    while ((pVideoClipOption->asfAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->asfVidePresentTime)
                  #endif
                    {
                      #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                        if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                            Record_flag[pVideoClipOption->RFUnit] = 1;
                        if(Record_flag[pVideoClipOption->RFUnit] == 1)
                        {
                            pVideoClipOption->sysCaptureVideoStart	= 0;
                            pVideoClipOption->sysCaptureVideoStop	= 1;
                            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                            break;
                        }
                      #endif
                        audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                        if (audio_value > 0) {
                            if(audio_value_max < audio_value)
                                audio_value_max = audio_value; //Lsk : for what ?
                            MultiChannelAsfWriteVirtualAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx]);
                            pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                            OSSemPost(pVideoClipOption->iisTrgSemEvt);
                        } else {
                            break;
                        }
                        
	                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
	                    {
	                        DEBUG_ASF("Main storage not ready.\n");
                          #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                          #endif
	                        return 0;
	                    }
                    }
                  #if ASF_DEBUG_ENA
                    DEBUG_ASF("<TTT>3. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);															
                    DEBUG_ASF("<TTT>3. skip <%d>\n"                                    , skip_A);	
                    DEBUG_ASF("<TTT>3. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);				
                    DEBUG_ASF("************************ Trigger sync end ************************\n");
                  #endif
                #endif
                }
              #if INSERT_NOSIGNAL_FRAME
                if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                {
                    DEBUG_ASF("===== RFIU_RX_STA_LINK_BROKEN =====\n");
                    return 0;
                }
              #endif
                /*******************************************
                *** seek to I frame, open a new asf file  ***
                *******************************************/

                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        pVideoClipOption->AV_TimeBase  = PREROLL;
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0) {
                          #if(RECORD_SOURCE == LOCAL_RECORD)  
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                          #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
									DEBUG_ASF("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    				if(MultiChannelGetCaptureVideoStatus(i))
                                    {
                                    	DEBUG_ASF("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        				sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                }
                        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))


                        #elif RFIU_TEST

                        #else
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #endif
                            }
                          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                    }
                    #elif (TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0)
                        {
                        #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                        #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
			                     	DEBUG_ASF("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    				if(MultiChannelGetCaptureVideoStatus(i))
                                    {
                                    	DEBUG_ASF("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        				sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                }
                        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))


                        #elif RFIU_TEST

                        #else
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #endif
                            }
                         #endif
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                      #if ASF_DEBUG_ENA
                        DEBUG_ASF("<FFF> ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, 
                                                                pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                        DEBUG_ASF("<FFF> RX time <%d, %d>\n", RX_time_A, RX_time_V);						
                        DEBUG_ASF("<FFF> RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);						
                        DEBUG_ASF("<FFF> RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V,
                                                                                     RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                      #endif
                    }
                    else
                    {
                        /*** Reset Audio/Video time biase ***/
                        pVideoClipOption->ResetPayloadPresentTime = 0; //reset video time base
                        pVideoClipOption->AV_TimeBase  = PREROLL;
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0) 
                        {
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                	DEBUG_RED("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    				if(MultiChannelGetCaptureVideoStatus(i))
                                    {
	                                    DEBUG_RED("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        				sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                }
                        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))


                        #elif RFIU_TEST

                        #else
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        #endif
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                      #if ASF_DEBUG_ENA
                        DEBUG_ASF("<FFF> ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime,
                                                                pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                        DEBUG_ASF("<FFF> RX time <%d, %d>\n", RX_time_A, RX_time_V);						
                        DEBUG_ASF("<FFF> RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);						
                        DEBUG_ASF("<FFF> RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V,
                                                                                     RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                      #endif
                    }
                    #endif
                }
            }
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
            {
                /*** check record time period ***/
                MultiChannelCheckRecordTimeUP(pVideoClipOption);

                /*** TODO ***/
                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP)
                {
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_TIMEUP\n\n", pVideoClipOption->VideoChannelID);
                }
            }
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP)
            {
                write_audio_back_cnt = 0;
                while((pVideoClipOption->asfVidePresentTime > pVideoClipOption->asfAudiPresentTime) && pVideoClipOption->iisCmpSemEvt->OSEventCnt > 0)
                    //write audio payload until PTS chase to video
                {
                    write_audio_back_cnt++;
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                    if (audio_value > 0) 
                    {
                    #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
                        if (MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],0) == 0)
                    #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
                        if (MultiChannelAsfWriteAudiPayload_IMA_ADPCM(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx], &PcmOffset) == 0)
                    #endif
                        {
                            DEBUG_ASF("ASF write audio payload error!!!\n");
                            /* write header object post */
                            if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                                DEBUG_ASF("ASF write file properties object post error!!!\n");
                                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                                pVideoClipOption->OpenFile  = 0;
                              #if(RECORD_SOURCE == LOCAL_RECORD)
                                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                                {
                                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                                #endif
                                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                                }
                              #elif(RECORD_SOURCE == RX_RECEIVE)
                                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                                {
                                    // 偵測到卡壞掉就全部關閉錄影
                                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                    {
                                        if(MultiChannelGetCaptureVideoStatus(i))
                                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                                }
                              #endif
                                return 0;
                            }

                            /* write data object post */
                            if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                                DEBUG_ASF("ASF write data object post error!!!\n");
                                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                                pVideoClipOption->OpenFile  = 0;
                              #if(RECORD_SOURCE == LOCAL_RECORD)
                                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                                {
                                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                                #endif
                                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                                }
                              #elif(RECORD_SOURCE == RX_RECEIVE)
                                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                                {
                                    // 偵測到卡壞掉就全部關閉錄影
                                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                    {
                                        if(MultiChannelGetCaptureVideoStatus(i))
                                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                                }
                              #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                                return 0;
                            }
                        #if 0
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                        #else   // 修正index不見的問題
                            // write index object //
                            if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                                DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                                //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                                //pVideoClipOption->OpenFile  = 0;
                                //return 0;
                            }

                            /* close file */
                            if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                                DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                                //pVideoClipOption->OpenFile  = 0;
                                //return 0;
                            }
                            DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                            pVideoClipOption->OpenFile  = 0;
                        #endif
                          #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                          #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                        pVideoClipOption->iisSounBufMngReadIdx = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        //OSSemPost(iisTrgSemEvt);
                    }
                }                    
            
                MultiChannelCheckWriteFinish(pVideoClipOption);

                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
                {
                    monitor_value = 0;

                    /*** Reset Audio/Video time biase ***/
                    #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    pVideoClipOption->ResetPayloadPresentTime = 0; //reset video time base
                    if(pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfAudiPresentTime )
                    {
                		pVideoClipOption->AV_TimeBase   = pVideoClipOption->asfVidePresentTime + 100; // 0.1s suspend
                    }
                    else
                    {
                        pVideoClipOption->AV_TimeBase   = pVideoClipOption->asfAudiPresentTime + 100; // 0.1s suspend
                    }
                    /*** Close ASF file ***/
                    #elif(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfAudiPresentTime )
                        printf("[Close file], V:%d, A:%d, diff:%d, Audiowriteback:%d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime, pVideoClipOption->asfVidePresentTime - pVideoClipOption->asfAudiPresentTime, write_audio_back_cnt);
                    else
                        printf("[Close file], V:%d, A:%d, diff:%d, Audiowriteback:%d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime, pVideoClipOption->asfAudiPresentTime - pVideoClipOption->asfVidePresentTime, write_audio_back_cnt);
                    #if INSERT_NOSIGNAL_FRAME
                    OS_ENTER_CRITICAL();
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                    {
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                      #if (NOSIGNAL_MODE == 3)
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                      #endif
                    }
                    OS_EXIT_CRITICAL();
                    #endif
                    if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) 
                    {
                      #if(RECORD_SOURCE == LOCAL_RECORD)  
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                      #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                        return 0;
                    }
                    #endif
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_WAIT\n\n", pVideoClipOption->VideoChannelID);

                #if (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)
                    uiOsdDrawSysAfterRec(pVideoClipOption);
                #endif
                    if(pVideoClipOption->WantToExitPreRecordMode)
                    {
						DEBUG_ASF("@@@ Ch%d WantToExitPreRecordMode <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);                                            	
                        pVideoClipOption->sysCaptureVideoStart    = 0;
                        pVideoClipOption->sysCaptureVideoStop     = 1;
                    }
                }
            }
        }

        /**********************************
        **** Write Audio/Video Payload ****
        **********************************/
        if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            #ifdef ASF_AUDIO
            // ------Write audio payload------//
            //To Write an Audio payload, check PTS, or check if no video frame and audio buffer is going to be full
            if(((video_value == 0)&&(pVideoClipOption->iisCmpSemEvt->OSEventCnt > 200)) || (pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime))
            {
              #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                if(Record_flag[pVideoClipOption->RFUnit] == 1)
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                    if(audio_value == 0)
                    {
                        A_flag[pVideoClipOption->RFUnit] = 1;
                    }
                    else
                    {
                        A_flag[pVideoClipOption->RFUnit] = 0;
                        Audio_RF_index[pVideoClipOption->RFUnit] = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
                    }
                }
                else
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                    if((audio_value > 0) && (A_flag[pVideoClipOption->RFUnit] == 1))
                    {
                        pVideoClipOption->iisSounBufMngReadIdx = 0;
                        A_flag[pVideoClipOption->RFUnit] = 0;
                        
                        if(Lose_audio_time[pVideoClipOption->RFUnit] > 0)
                        {
                            pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time -= 128;
                            Lose_audio_time[pVideoClipOption->RFUnit] += pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time;
                            Lose_audio_time[pVideoClipOption->RFUnit] = (Lose_audio_time[pVideoClipOption->RFUnit]/128);
                            for(i=0; i < Lose_audio_time[pVideoClipOption->RFUnit]; i++)
                                MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],2);
                            Lose_audio_time[pVideoClipOption->RFUnit] = 0;
                        }
                        Audio_RF_index[pVideoClipOption->RFUnit] = 2000000;
                        pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time = 128;
                    }
                    else if((audio_value > 0) && (A_flag[pVideoClipOption->RFUnit] == 0))
                    {
                        if(Audio_RF_index[pVideoClipOption->RFUnit] == pVideoClipOption->iisSounBufMngReadIdx)
                        {
                            pVideoClipOption->iisSounBufMngReadIdx = 0;
                            A_flag[pVideoClipOption->RFUnit] = 0;
                            if(Lose_audio_time[pVideoClipOption->RFUnit] > 0)
                            {
                                pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time -= 128;
                                Lose_audio_time[pVideoClipOption->RFUnit] += pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time;
                                Lose_audio_time[pVideoClipOption->RFUnit] = (Lose_audio_time[pVideoClipOption->RFUnit]/128);
                                for(i=0; i<Lose_audio_time[pVideoClipOption->RFUnit]; i++)
                                    MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],2);
                                Lose_audio_time[pVideoClipOption->RFUnit] = 0;
                            }
                            pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time = 128;
                            Audio_RF_index[pVideoClipOption->RFUnit] = 2000000;
                        }
                    }
                }
              #else
                audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
              #endif                
                if (audio_value > 0)
                {
                    if(audio_value_max < audio_value)
                        audio_value_max = audio_value;

                #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
                    if (MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],0) == 0)
                #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
                    if (MultiChannelAsfWriteAudiPayload_IMA_ADPCM(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx], &PcmOffset) == 0)
                #endif
                    {
                        DEBUG_ASF("Ch%d ASF write audio payload error!!!\n", pVideoClipOption->VideoChannelID);
                        /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                    #if 0
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    #else   // 修正index不見的問題
                        // write index object //
                        if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                            //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }

                        /* close file */
                        if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                            DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                        pVideoClipOption->OpenFile  = 0;
                    #endif
                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                      #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                        return 0;
                    }

                    pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    OSSemPost(pVideoClipOption->iisTrgSemEvt);
                }
            }

            //------ Write video payload------//
            //To Write a Video payload, check PTS, or check if no audio frame and video buffer is going to be full            
            if(((audio_value == 0)&&(pVideoClipOption->CurrentBufferSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE))) || (pVideoClipOption->asfAudiPresentTime >= pVideoClipOption->asfVidePresentTime))
            {
            #endif      // ASF_AUDIO
              #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                if(Record_flag[pVideoClipOption->RFUnit] == 1)
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    if(video_value == 0)
                        V_flag[pVideoClipOption->RFUnit] = 1;
                    else
                    {
                        V_flag[pVideoClipOption->RFUnit] = 0;
                        Video_RF_index[pVideoClipOption->RFUnit] = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                    }
                }
                else
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    if((video_value > 0) && (V_flag[pVideoClipOption->RFUnit] == 1))
                    {
                        pVideoClipOption->VideoBufMngReadIdx = 0;
                        V_flag[pVideoClipOption->RFUnit] = 0;
                        Video_RF_index[pVideoClipOption->RFUnit] = 2000000;
                    }
                    else if((video_value > 0) && (V_flag[pVideoClipOption->RFUnit] == 0))
                    {
                        if(pVideoClipOption->VideoBufMngReadIdx == Video_RF_index[pVideoClipOption->RFUnit])
                        {
                            pVideoClipOption->VideoBufMngReadIdx = 0;
                            Video_RF_index[pVideoClipOption->RFUnit] = 2000000;
                            V_flag[pVideoClipOption->RFUnit] = 0;
                        }
                    }
                }
              #else
                video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
              #endif
                if (video_value > 0)
                {
                    if(video_value_max < video_value)
                        video_value_max = video_value;

                    //Start asfTimeStatistics at create file (manual/trigger mode)
                    if(!pVideoClipOption->Start_asfTimeStatistics)
                    {
                        /*** TODO
                        calculat pre-record part VideoTimeStatistics
                        if(Cal_FileTime_Start_Idx == VideoBufMngReadIdx)
                        {
                        }
                        ***/
                        pVideoClipOption->Start_asfTimeStatistics   = 1;
                        pVideoClipOption->asfTimeStatistics         = 0;
                        pVideoClipOption->LocalTimeInSec            = g_LocalTimeInSec;
                    }
                #if 0
                    if(dcfWrite(pVideoClipOption->pFile, (u8*)pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size, &size) == 0) {
                        DEBUG_ASF("Ch%d write video frame data error!!!\n", pVideoClipOption->VideoChannelID);
        	    		dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    #endif
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        	    	    return 0;
        			}
                #else
                    if (MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],0) == 0)
                    {
                        DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
                         /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                         #if(RECORD_SOURCE == LOCAL_RECORD)  
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                    #if 0
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                    #else   // 修正index不見的問題
                        // write index object //
                        if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                            //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }

                        /* close file */
                        if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                            DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                        pVideoClipOption->OpenFile  = 0;
                    #endif
                     #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                     #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                     #endif
                        return 0;
                    }
                #endif
                    pVideoClipOption->VideoBufMngReadIdx = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                    //pVideoClipOption->asfVopCount++;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                  #endif
                    //DEBUG_ASF(" %d ", pVideoClipOption->VideoChannelID);
                }

                #if FORCE_FPS
                if(((pVideoClipOption->asfVidePresentTime - PREROLL) * FORCE_FPS) > ((pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount) * 1000))
                {
                    if (MultiChannelAsfWriteDummyVidePayload(pVideoClipOption,1,0) == 0)
                    {
                        DEBUG_ASF("Ch%d ASF write video dummy payload error!!!\n", pVideoClipOption->VideoChannelID);
                         /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                          #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                          #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                        pVideoClipOption->OpenFile  = 0;
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                     #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                     #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                     #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                        return 0;
                    }
                }
                #endif

            #ifdef  ASF_AUDIO
            }
            // Skip siu frames for release bandwidth to SD card writing
            monitor_value   = (video_value > audio_value) ? video_value : audio_value;
            #else
            monitor_value   = video_value;
            #endif

            // if there is no frame in buffer, release CPU to low priority task
            if(video_value == 0 || audio_value == 0)
                OSTimeDly(1);

            if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {

                #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                /**********************************
                **** Check File Size           ****
                **********************************/
                if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP)
                {
                    if(pVideoClipOption->asfDataPacketCount > MaxPacketCount)
                    {
                        DEBUG_ASF("\n\n\nCh%d File Size reach limit\n\n\n", pVideoClipOption->VideoChannelID);
                        if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) {
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                        /*** Reset Audio/Video time biase ***/
                        pVideoClipOption->ResetPayloadPresentTime   = 0; //reset video time base
                        pVideoClipOption->AV_TimeBase               = PREROLL;
                        //DEBUG_ASF("MPEG4 UseSem :%04d, IIS UseSem :%04d\n", VideoCmpSemEvt->OSEventCnt,iisCmpSemEvt->OSEventCnt);
                        //DEBUG_ASF("=====================================\n");
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0)
                        {
                         #if(RECORD_SOURCE == LOCAL_RECORD)
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                         #elif(RECORD_SOURCE == RX_RECEIVE)
                            if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
									DEBUG_ASF("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    				if(MultiChannelGetCaptureVideoStatus(i))
                                    {
                                    	DEBUG_ASF("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        				sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                    }
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                         #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                            return 0;
                        }
                    }
                }
                #endif
            }
        }   //if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
     #if(RECORD_SOURCE == LOCAL_RECORD)
        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
            if(video_value < 3)
                OSTimeDly(1);  //Lucian: release resource to low piority task.
        } else {    // 確保RF錄影時第一張為I frame
            if(video_value < 5)
                OSTimeDly(1);  //Lucian: release resource to low piority task.
        }
     #endif
      #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
        if(Record_flag[pVideoClipOption->RFUnit] == 0)
        {
            for(i = 0; i < 10; i++) // 確保錄影時第一張為I frame, 但若video buffer空掉會無法換檔.
            {
                if(pVideoClipOption->VideoCmpSemEvt->OSEventCnt == 0)
                    OSTimeDly(1);
            }
        }
      #else
        for(i = 0; i < 10; i++) // 確保錄影時第一張為I frame, 但若video buffer空掉會無法換檔.
        {
            if(pVideoClipOption->VideoCmpSemEvt->OSEventCnt == 0)
                OSTimeDly(1);
        }
      #endif

        //------------------- Bitstream buffer control---------------------------------//
        /*
             Lucian: 以Audio/Video bitstream buffer 內的index剩餘個數為偵測點,若大於 ASF_DROP_FRAME_THRESHOLD
                     則為SD 寫入速度過慢,需drop frame.

        */
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) //Event trigger mode
        {
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT &&
               (pVideoClipOption->CurrentBufferSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE) ||
                (pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000)) ||
                pVideoClipOption->VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 60) ||
                pVideoClipOption->iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 16) ||
                (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_DUMMY_ENA))) {
              #if ASF_DEBUG_ENA
                DEBUG_ASF("************************ Bitstream control ************************\n");
                skip_I=0;
                skip_P=0;
                skip_A=0;
                DEBUG_ASF("<BBB>1. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                DEBUG_ASF("<BBB>1. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>1. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>1. pVideoClipOption->CurrentBufferSize           = %d\n", pVideoClipOption->CurrentBufferSize);	
                DEBUG_ASF("<BBB>1. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                DEBUG_ASF("<BBB>1. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                DEBUG_ASF("<BBB>1. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
              #endif
                do 
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                  #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                    if(Record_flag[pVideoClipOption->RFUnit] == 1)
                    {
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Record_flag[pVideoClipOption->RFUnit]   = 0;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                        DEBUG_ASF("VideoBufMngReadIdx %d\n ", pVideoClipOption->VideoBufMngReadIdx);
                        break;
                    }
                  #endif
                    //DEBUG_ASF(" v%d=%d ", pVideoClipOption->VideoChannelID, video_value);
                    if (video_value > 0) 
                    {
                        if(video_value_max < video_value)
                            video_value_max = video_value;
                      #if ASF_DEBUG_ENA
                        if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                        	skip_I++;
                        else
                        	skip_P++;
                      #endif
                        MultiChannelAsfWriteVirtualVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx]);
                        pVideoClipOption->VideoBufMngReadIdx  = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                      #endif
                    }
                    else
                    {
                        break;
                    }
                } while(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP);
              #if ASF_DEBUG_ENA
                DEBUG_ASF("<BBB>2. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                DEBUG_ASF("<BBB>2. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>2. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>2. pVideoClipOption->CurrentBufferSize           = %d\n", pVideoClipOption->CurrentBufferSize);	
                DEBUG_ASF("<BBB>2. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                DEBUG_ASF("<BBB>2. skip <%d, %d>\n"                                    , skip_I, skip_P);	
                DEBUG_ASF("<BBB>2. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                DEBUG_ASF("<BBB>2. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
              #endif

              #if CDVR_LOG
                pVideoClipOption->LogFileStart    = (pVideoClipOption->LogFileStart + 1) % LOG_INDEX_NUM;
              #endif

              #ifdef  ASF_AUDIO
               #if INSERT_NOSIGNAL_FRAME
                while ((pVideoClipOption->sysAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->sysVidePresentTime)
               #else
                while ((pVideoClipOption->asfAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->asfVidePresentTime)
               #endif
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                  #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                    if(Record_flag[pVideoClipOption->RFUnit] == 1)
                    {
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Record_flag[pVideoClipOption->RFUnit]   = 0;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                        DEBUG_ASF("iisSounBufMngReadIdx %d\n ", pVideoClipOption->iisSounBufMngReadIdx);
                        break;
                    }
                  #endif
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;
                        MultiChannelAsfWriteVirtualAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx]);
                        pVideoClipOption->iisSounBufMngReadIdx    = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        OSSemPost(pVideoClipOption->iisTrgSemEvt);
                    } else {
                        break;
                    }
                }
                #endif
              #if ASF_DEBUG_ENA
                DEBUG_ASF("<BBB>3. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                DEBUG_ASF("<BBB>3. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>3. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                DEBUG_ASF("<BBB>3. pVideoClipOption->CurrentBufferSize           = %d\n", pVideoClipOption->CurrentBufferSize);	
                DEBUG_ASF("<BBB>3. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                DEBUG_ASF("<BBB>3. skip <%d>\n"                                    , skip_A);	
                DEBUG_ASF("<BBB>3. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                DEBUG_ASF("<BBB>3. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);				
                DEBUG_ASF("************************ Bitstream sync end ************************\n");
              #endif
            }
            else if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
            {
                OSTimeDly(1);  //Lucian: release resource to low piority task.
            }
        }
#if 0   // 取消由 packer 控制 image sensor frame rate
        else // Normal mode or overwrite mode
        {   // asfCaptureMode != ASF_CAPTURE_EVENT
        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )

            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {
                siuSkipFrameRate    = 0;
            }
            else
            {
               //DEBUG_ASF("z",monitor_value);
               DEBUG_ASF("z");
            }
        #else
            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {       // not skip siu frame
                if(siuSkipFrameRate != 0)
                {
                    siuSkipFrameRate    = 0;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 10)) {
                if(siuSkipFrameRate != 2) {
                    siuSkipFrameRate    = 2;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 20)) {
                if(siuSkipFrameRate != 4) {
                    siuSkipFrameRate    = 4;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 30)) {
                if(siuSkipFrameRate != 6) {
                    siuSkipFrameRate    = 6;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 40)) {
                if(siuSkipFrameRate != 8) {
                    siuSkipFrameRate    = 8;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 50)) {
                if(siuSkipFrameRate != 10) {
                    siuSkipFrameRate    = 10;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 60)) {
                if(siuSkipFrameRate != 12) {
                    siuSkipFrameRate    = 12;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 70)) {
                if(siuSkipFrameRate != 16) {
                    siuSkipFrameRate    = 16;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 80)) {
                if(siuSkipFrameRate != 20) {
                    siuSkipFrameRate    = 20;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 90)) {
                if(siuSkipFrameRate != 24) {
                    siuSkipFrameRate    = 24;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 100)) {
                if(siuSkipFrameRate != 28) {
                    siuSkipFrameRate    = 28;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else {
                if(siuSkipFrameRate != 32) {
                    siuSkipFrameRate    = 32;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            }
        #endif

        }   // asfCaptureMode != ASF_CAPTURE_EVENT
#endif

        /********************************************************
        *** asf index table detection / SD capacity detection ***
        ********************************************************/
        if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            //-------------Detect ASF index table: 因 ASF index table 是暫存於DRAM有容量限制; 錄影結束後才寫入SD card. ------//
            #ifdef  ASF_AUDIO
            if((pVideoClipOption->asfIndexTableIndex + VIDEO_BUF_NUM + IIS_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #else
            if((pVideoClipOption->asfIndexTableIndex + VIDEO_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #endif
    		{
                DEBUG_ASF("Ch%d asfIndexTableIndex =  %d, index buffer limit to %d, finish!!!\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfIndexTableIndex, ASF_IDX_SIMPLE_INDEX_ENTRY_MAX);
                pVideoClipOption->sysCaptureVideoStart    = 0;
                pVideoClipOption->sysCaptureVideoStop     = 1;
                break;
            }

            //-------------Detect Disk Full---------------------//
            // for disk full control
            if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA))
            {
                u32 TotalSize, i;
                pVideoClipOption->asfDataSize   = sizeof(ASF_DATA_OBJECT) +
                                                  pVideoClipOption->asfDataPacketCount * ASF_DATA_PACKET_SIZE;
                pVideoClipOption->asfIndexSize  = sizeof(ASF_SIMPLE_INDEX_OBJECT) +
                                                  pVideoClipOption->asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY);
                CurrentFileSize                 = pVideoClipOption->asfHeaderSize +
                                                  pVideoClipOption->asfDataSize +
                                                  pVideoClipOption->asfIndexSize;

                TotalSize                       = 0;
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if((VideoClipOption[i].PackerTaskCreated) && (VideoClipOption[i].OpenFile))
                    {
                    	#if 0//Lsk: FIXME may CurrentAudioSize<0;  
                        TotalSize  += (VideoClipOption[i].asfDataSize +
                                      VideoClipOption[i].asfIndexSize +
                                      VideoClipOption[i].asfHeaderSize +
                                      VideoClipOption[i].CurrentVideoSize +
                     	                 VideoClipOption[i].CurrentAudioSize) / 1024;
						#else
                        TotalSize  += (VideoClipOption[i].asfDataSize +
                                      VideoClipOption[i].asfIndexSize +
                                      VideoClipOption[i].asfHeaderSize +
                                      VideoClipOption[i].CurrentVideoSize
                                      ) / 1024;
						#endif

                    }
                }

                //if((((CurrentFileSize + pVideoClipOption->CurrentVideoSize + pVideoClipOption->CurrentAudioSize) / 1024) >= (free_size - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) / 1024)) )               
                if((dcfGetMainStorageFreeSize() < (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX / 1024))
                {
                    DEBUG_ASF("Ch%d Disk full!!!\n", pVideoClipOption->VideoChannelID);
                    DEBUG_ASF("free_size        = %d KBytes, CurrentFileSize = %d bytes.\n", dcfGetMainStorageFreeSize(), CurrentFileSize);
                    DEBUG_ASF("TotalSize        = %d bytes.\n", TotalSize);
                    DEBUG_ASF("asfHeaderSize    = %d bytes.\n", pVideoClipOption->asfHeaderSize);
                    DEBUG_ASF("asfDataSize      = %d bytes.\n", pVideoClipOption->asfDataSize);
                    DEBUG_ASF("asfIndexSize     = %d bytes.\n", pVideoClipOption->asfIndexSize);
                    DEBUG_ASF("CurrentVideoSize = %d bytes.\n", pVideoClipOption->CurrentVideoSize);
                    DEBUG_ASF("CurrentAudioSize = %d bytes.\n", pVideoClipOption->CurrentAudioSize);
					for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                	{
                		{	
	                    	DEBUG_ASF("VideoClipOption[%d] OpenFile, PackerTaskCreated = %d, %d\n", i, VideoClipOption[i].OpenFile, VideoClipOption[i].PackerTaskCreated); 
							DEBUG_ASF("VideoClipOption[%d].asfHeaderSize    = %d bytes.\n", i, VideoClipOption[i].asfHeaderSize);
		                    DEBUG_ASF("VideoClipOption[%d].asfDataSize      = %d bytes.\n", i, VideoClipOption[i].asfDataSize);
		                    DEBUG_ASF("VideoClipOption[%d].asfIndexSize     = %d bytes.\n", i, VideoClipOption[i].asfIndexSize);
		                    DEBUG_ASF("VideoClipOption[%d].CurrentVideoSize = %d bytes.\n", i, VideoClipOption[i].CurrentVideoSize);
		                    DEBUG_ASF("VideoClipOption[%d].CurrentAudioSize = %d bytes.\n", i, VideoClipOption[i].CurrentAudioSize);
	                    }	                    
	                }
                    pVideoClipOption->sysCaptureVideoStart  = 0;
                    pVideoClipOption->sysCaptureVideoStop   = 1;
                    MemoryFullFlag                          = TRUE;
                     //Warning_SDFull();
                #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                    osdDrawMemFull(UI_OSD_DRAW);
                #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))

                #elif RFIU_TEST

                #else
                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                #endif
                    // 偵測到卡滿就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    break;
                }
            }
        }

        /************************************
        *** Change file by size or slice ***
        ************************************/
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
          #if (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfDataPacketCount  > MaxPacketCount) && ((pVideoClipOption->AV_Source == LOCAL_RECORD) || pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
            if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfDataPacketCount  > MaxPacketCount) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
          #elif (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && ((pVideoClipOption->AV_Source == LOCAL_RECORD) || pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
            //if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 5))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
           #if INSERT_NOSIGNAL_FRAME
            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec) && ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 20)))) && ((pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP) || (gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)))
//            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
           #else
            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec) && ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 5)))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
           #endif
          #endif
            {
                if(asfRecFileNum != 0)
                {
                    pVideoClipOption->WantChangeFile            = 1;
                    DEBUG_ASF("Ch%d Time's up!!!\n", pVideoClipOption->VideoChannelID);
                    DEBUG_ASF("RTCseconds == %d\n", pVideoClipOption->RTCseconds);
                    DEBUG_ASF("asfRecFileNum  = %d\n", asfRecFileNum);
                  #if INSERT_NOSIGNAL_FRAME
                    OS_ENTER_CRITICAL();
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                    {
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                      #if(NOSIGNAL_MODE == 3)
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                      #endif
                        DEBUG_ASF("RFIU_RX_STA_LINK_BROKEN\n\n\n");
                    }
                    OS_EXIT_CRITICAL();
                  #endif

                    if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) {

                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                      #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                        return 0;
                    }
                  #if INSERT_NOSIGNAL_FRAME
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                        return 0;
                  #endif

                    /*** Reset Audio/Video time biase ***/
                    pVideoClipOption->ResetPayloadPresentTime   = 0; //reset video time base
                    pVideoClipOption->AV_TimeBase               = PREROLL;

                  #if (THUMBNAIL_PREVIEW_ENA == 1)
                  pVideoClipOption->SearchThumbnailIdx = pVideoClipOption->VideoBufMngReadIdx;
                  #endif
                  
                  #if INSERT_NOSIGNAL_FRAME
                    if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0) 
                  #else
                    if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(0, pVideoClipOption)) == 0) 
                  #endif
                    {

                      #if(RECORD_SOURCE == LOCAL_RECORD)
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                            #endif
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                      #elif(RECORD_SOURCE == RX_RECEIVE)
                        if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
								DEBUG_ASF("######## CH%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, __LINE__, pVideoClipOption->EventTrigger);
                    			if(MultiChannelGetCaptureVideoStatus(i))
                                {
	                                DEBUG_ASF("######## CH%d, stop ch%d <%d>, mode = %d", pVideoClipOption->VideoChannelID, i, __LINE__, pVideoClipOption->EventTrigger);
                        			sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                            }
                    #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    #elif((SW_APPLICATION_OPTION == Standalone_Test) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))


                    #elif RFIU_TEST

                    #else
                            if(MemoryFullFlag == TRUE)
                                uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                            else
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    #endif
                        }
                      #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                        return 0;
                    }

                    OS_ENTER_CRITICAL();
                    pVideoClipOption->WantChangeFile    = 0;
                    pVideoClipOption->LastAudio         = 0;
                    pVideoClipOption->LastVideo         = 0;
                    pVideoClipOption->GetLastAudio      = 0;
                    pVideoClipOption->GetLastVideo      = 0;
                    OS_EXIT_CRITICAL();
                    CurrentFileSize                     = 0;
                }
                else
                {
                    pVideoClipOption->sysCaptureVideoStart  = 0;
                    pVideoClipOption->sysCaptureVideoStop   = 1;
                    DEBUG_ASF("Ch%d asfRecFileNum  = %d\n", pVideoClipOption->VideoChannelID, asfRecFileNum);
                    break;
                }
            }
        }
        //-------------Check Power-off: 偵測到Power-off,須結束錄影 ------------------------//
        if(pwroff == 1) {   //prepare for power off
            pVideoClipOption->sysCaptureVideoStart  = 0;
            pVideoClipOption->sysCaptureVideoStop   = 1;
            break;
        }

        //------- Indicator of REC (LED ON/OFF): 以LED 閃爍提示---------------//
        //timetick =  IndicateRecordStatus(timetick);

    }

    DEBUG_ASF("Ch%d exit video capture while loop...\n", pVideoClipOption->VideoChannelID);

  #if(RECORD_SOURCE == LOCAL_RECORD)
    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
    #ifdef  ASF_AUDIO
        while(pVideoClipOption->iisTrgSemEvt->OSEventCnt > 0) {
            OSSemAccept(pVideoClipOption->iisTrgSemEvt);
        }
    #endif

        while(pVideoClipOption->VideoTrgSemEvt->OSEventCnt > 0) {
            OSSemAccept(pVideoClipOption->VideoTrgSemEvt);
        }
        OSTimeDly(6);
    }
  #endif
    if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && ((pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TEMP)))
	{
	  #if(TRIGGER_MODE_CLOSE_FILE_METHOD==CLOSE_FILE_BY_SIZE)
        if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) {
          #if(RECORD_SOURCE == LOCAL_RECORD)
            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
            {
            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
            #endif
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
            }
         #elif(RECORD_SOURCE == RX_RECEIVE)
            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
         #endif
            return 0;
        }
      #endif
        DEBUG_ASF("Ch%d Event mode finish!!\n", pVideoClipOption->VideoChannelID);
        DEBUG_ASF("Ch%d video_value_max = %d\n", pVideoClipOption->VideoChannelID, video_value_max);
      #ifdef  ASF_AUDIO
        DEBUG_ASF("Ch%d audio_value_max = %d\n", pVideoClipOption->VideoChannelID, audio_value_max);
      #endif
      #if(RECORD_SOURCE == LOCAL_RECORD)
        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
        #endif
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        }
      #endif
      #if INSERT_NOSIGNAL_FRAME
        OS_ENTER_CRITICAL();
        Record_flag[pVideoClipOption->VideoChannelID] = 0;
        OS_EXIT_CRITICAL();
      #endif
        return 1;
    }

    DEBUG_ASF("Ch%d write %d remaining data...\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoCmpSemEvt->OSEventCnt);

  #ifdef  ASF_AUDIO
    // write redundance audio payload data
   #if INSERT_NOSIGNAL_FRAME
    audio_index = 0;
    while((pVideoClipOption->iisCmpSemEvt->OSEventCnt > 0) && (audio_index < 20)) 
   #else
    while(pVideoClipOption->iisCmpSemEvt->OSEventCnt > 0) 
   #endif
    {
        Output_Sem();
        audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0) 
        {
          #if INSERT_NOSIGNAL_FRAME
            audio_index++;
          #endif
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
        #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
            if (MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],0) == 0)
        #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            if (MultiChannelAsfWriteAudiPayload_IMA_ADPCM(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx], &PcmOffset) == 0)
        #endif
            {
                DEBUG_ASF("ASF write audio payload error!!!\n");
                /* write header object post */
                if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("ASF write file properties object post error!!!\n");
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    #endif
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif
                    return 0;
                }

                /* write data object post */
                if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("ASF write data object post error!!!\n");
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    #endif
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                    return 0;
                }
            #if 0
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
            #else   // 修正index不見的問題
                // write index object //
                if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                    //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }

                /* close file */
                if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                    DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                pVideoClipOption->OpenFile  = 0;
            #endif
              #if(RECORD_SOURCE == LOCAL_RECORD)
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                #endif
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
              #elif(RECORD_SOURCE == RX_RECEIVE)
                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
              #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                return 0;
            }
            pVideoClipOption->iisSounBufMngReadIdx = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //OSSemPost(iisTrgSemEvt);
        }
    }
#endif
    
  #if INSERT_NOSIGNAL_FRAME
    video_index = 0;
    while((pVideoClipOption->VideoCmpSemEvt->OSEventCnt > 0) && (video_index < 10)) 
  #else
    while(pVideoClipOption->VideoCmpSemEvt->OSEventCnt > 0) 
  #endif
    {
        video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
        if (video_value > 0) 
        {
          #if INSERT_NOSIGNAL_FRAME
            video_index++;
          #endif
            if(video_value_max < video_value)
                video_value_max = video_value;
        #if 0
            if(dcfWrite(pVideoClipOption->pFile, (u8*)pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size, &size) == 0) {
                DEBUG_AVI("Ch%d ASF write video chunk error!!!\n", pVideoClipOption->VideoChannelID);
	    		dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
            #endif
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
	    	    return 0;
			}
        #else
            if (MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],0) == 0)
            {
                DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
                /* write header object post */
                if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    #endif
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                    return 0;
                }

                /* write data object post */
                if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    #endif
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                    return 0;
                }
            #if 0
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
            #else   // 修正index不見的問題
                // write index object //
                if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                    //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }

                /* close file */
                if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                    DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                pVideoClipOption->OpenFile  = 0;
            #endif
              #if(RECORD_SOURCE == LOCAL_RECORD)
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                #endif
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
              #elif(RECORD_SOURCE == RX_RECEIVE)
                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
              #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                return 0;
            }
        #endif
            pVideoClipOption->VideoBufMngReadIdx = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            //pVideoClipOption->asfVopCount++;
            //OSSemPost(pVideoClipOption->VideoTrgSemEvt);
            //DEBUG_ASF(" %d ", pVideoClipOption->VideoChannelID);
        }
    }
  #if(RECORD_SOURCE == LOCAL_RECORD)
    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
    #endif
    #ifdef  ASF_AUDIO
        //MultiChannelIIsStopRec(pVideoClipOption);
        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
    #endif
    }
 #endif
    MultiChannelAsfCloseFile(pVideoClipOption);

    DEBUG_ASF("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
    DEBUG_ASF("audio_value_max = %d\n", audio_value_max);
#endif

    return 1;
}


#else

/*

Routine Description:

    Multiple channel asf file format packer.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfCaptureVideo(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u16             video_value;
    u16             video_value_max;
    u32             monitor_value;
    u32             timetick;
#ifdef  ASF_AUDIO
    u16             audio_value;
    u16             audio_value_max;
#endif

    u32             CurrentFileSize;
    u32             DummySize;
    u8              err,audio_index,video_index;
#if FINE_TIME_STAMP
    s32             TimeOffset;
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    u32             time1;
#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    s32             PcmOffset                   = 0;
#endif

    //s32             SkipFrameNum;
    //u32             PreRecordFrameNum;

    u32             MaxPacketCount;
    u8              TriggerModeFirstFileFlag    = 0;
    u8              InitDACPlayFlag             = 0;

    u32             SingleFrameSize;
    u32             FreeSpaceThreshold;
    u32             FreeSpace;
    u32             timeoutcnt                  = 0;
    u8              level;
    u32             size;
  #if (OS_CRITICAL_METHOD == 3)                 /* Allocate storage for CPU status register           */
    unsigned int    cpu_sr = 0;                 /* Prevent compiler warning                           */
  #endif
    u32             i;
    u32             RunVideo                    = 0;
    u32             RunAudio                    = 0;
#if (HW_BOARD_OPTION == MR6730_AFN)
		u8				MD_TrigCnt=0;
#endif
#if AUDIO_DEBUG_ENA
    u32 ASFWRV4iiscnt = 0;
#endif
	pVideoClipOption->sysAudiPresentTime = 0;
	pVideoClipOption->sysVidePresentTime = 0;

#if MULTI_CH_SDC_WRITE_TEST
    return TestFileSystem(pVideoClipOption);
#endif
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    Lose_audio_time[pVideoClipOption->RFUnit] = 0;
    Lose_video_time[pVideoClipOption->RFUnit] = 0;
    Audio_RF_index[pVideoClipOption->RFUnit] = 0;
    Video_RF_index[pVideoClipOption->RFUnit] = 0;
#endif

    if (pVideoClipOption->asfCaptureMode == ASF_CAPTURE_NORMAL)
        DEBUG_ASF("Ch%d ASF_CAPTURE_NORMAL\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_OVERWRITE_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_GSENSOR_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_MOTION_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_ALARM_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_DUMMY_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_EVENT_DUMMY_ENA\n", pVideoClipOption->VideoChannelID);
    if (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_SCHEDULE_ENA)
        DEBUG_ASF("Ch%d ASF_CAPTURE_SCHEDULE_ENA\n", pVideoClipOption->VideoChannelID);

    /***********************************************************
    *** calculate max packet count : file size / packet size ***
    ***********************************************************/
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
        #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = TriggerModeGetMaxPacketCount();
        #endif
    } else {
        #if(MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = ManualModeGetMaxPacketCount();
        #endif
    }

#if 0
    /****************************************************************
    *** calculate how many frames need for pre-record             ***
    *** PreRecordFrameNum = PreRecordTime * FPS                   ***
    ****************************************************************/
    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
    {
        if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            PreRecordFrameNum = PreRecordTime * 30;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            PreRecordFrameNum = PreRecordTime * 15;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            PreRecordFrameNum = PreRecordTime * 5;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_60)
            PreRecordFrameNum = PreRecordTime * 60;
        else if(pVideoClipOption->VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
            PreRecordFrameNum = PreRecordTime * 10;

    #if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
        PreRecordFrameNum = PreRecordTime * 15;
    #endif
    }
#endif

    /*********************
    *** reset variable ***
    *********************/

    /////////////////////////////////////////////////////////////////////////////
    // asfCaptureVideo() initial value

    pVideoClipOption->asfVideoFrameCount        = 0;
    pVideoClipOption->asfDataPacketPreSendTime  = 0;
    pVideoClipOption->asfDataPacketSendTime     = 0;
    pVideoClipOption->asfDataPacketFormatFlag   = 0;
    pVideoClipOption->VideoPictureIndex         = 0;
    //pVideoClipOption->VideoBufMngReadIdx        = 0;
    //pVideoClipOption->VideoBufMngWriteIdx       = 0;
    pVideoClipOption->asfVopCount               = 0;
#if FORCE_FPS
    pVideoClipOption->asfDummyVopCount          = 0;
    pVideoClipOption->DummyChunkTime            = 0;
#endif
    pVideoClipOption->asfVidePresentTime        = PREROLL; //Lsk 090309
    pVideoClipOption->MPEG4_Mode                = 0;    // 0: record, 1: playback
    OS_ENTER_CRITICAL();
    pVideoClipOption->CurrentVideoSize          = 0;
    pVideoClipOption->CurrentVideoTime          = 0;
    pVideoClipOption->VideoTimeStatistics       = 0;
    OS_EXIT_CRITICAL();
    pVideoClipOption->asfIndexTable             = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(pVideoClipOption->mpeg4IndexBuf+sizeof(ASF_SIMPLE_INDEX_OBJECT));
	/*
    for(i = 0; i < VIDEO_BUF_NUM; i++) {
        pVideoClipOption->VideoBufMng[i].buffer = pVideoClipOption->VideoBuf;
    }
    */
    //mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);   /*Peter 1116 S*/
    memset(pVideoClipOption->ISUFrameDuration, 0, sizeof(pVideoClipOption->ISUFrameDuration));

#ifdef ASF_AUDIO
    /* audio */
    pVideoClipOption->asfAudiChunkCount         = 0;
    pVideoClipOption->asfAudiPresentTime        = PREROLL; //Lsk 090309
    pVideoClipOption->iisSounBufMngReadIdx      = 0;
    pVideoClipOption->iisSounBufMngWriteIdx     = 0;
    pVideoClipOption->IISMode                   = 0;    // 0: record, 1: playback
    pVideoClipOption->IISTime                   = 0;    /* Peter 070104 */
    pVideoClipOption->IISTimeUnit               = (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE;  /* milliscends */ /* Peter 070104 */
    pVideoClipOption->CurrentAudioSize          = 0;
    /* initialize sound buffer */
	/*
    for(i = 0; i < IIS_BUF_NUM; i++) {
        pVideoClipOption->iisSounBufMng[i].buffer   = pVideoClipOption->iisSounBuf[i];
    }
    */
#endif  // ASF_AUDIO

    /* file */
    pVideoClipOption->asfDataPacketCount        = 0;
    pVideoClipOption->asfIndexTableIndex        = 0;
    pVideoClipOption->asfIndexEntryTime         = 0;
    pVideoClipOption->OpenFile                  = 0;
	pVideoClipOption->asfHeaderSize = 0;
	pVideoClipOption->asfDataSize = 0;
	pVideoClipOption->asfIndexSize = 0;

#if CDVR_LOG
    pVideoClipOption->LogFileStart                                  = 0;
    pVideoClipOption->LogFileNextStart                              = 0;
    pVideoClipOption->LogFileCurrent                                = 0;
    pVideoClipOption->pLogFileEnd                                   = (u8*)0;
    pVideoClipOption->pLogFileMid                                   = (u8*)0;
    pVideoClipOption->LogFileIndex[pVideoClipOption->LogFileStart]  = pVideoClipOption->LogFileBuf;
#endif

    /////////////////////////////////////////////////////////////////////////////
    // asfCaptureVideoFile() initial value

    pVideoClipOption->asfTimeStatistics         = 0;
    pVideoClipOption->LocalTimeInSec            = g_LocalTimeInSec;
    pVideoClipOption->DirectlyTimeStatistics    = 0;
    pVideoClipOption->ResetPayloadPresentTime   = 1;
    pVideoClipOption->WantChangeFile            = 0;
    pVideoClipOption->LastAudio                 = 0;
    pVideoClipOption->LastVideo                 = 0;
    pVideoClipOption->GetLastAudio              = 0;
    pVideoClipOption->GetLastVideo              = 0;
    pVideoClipOption->EventTrigger              = CAPTURE_STATE_WAIT;
    pVideoClipOption->MD_Diff                   = 0;
    pVideoClipOption->MPEG4_Error               = 0;
    pVideoClipOption->sysReady2CaptureVideo     = 0;
    pVideoClipOption->WantToExitPreRecordMode   = 0;
    video_value                                 = 0;
    video_value_max                             = 0;
    monitor_value                               = 0;
#ifdef ASF_AUDIO
    audio_value                                 = 0;
    audio_value_max                             = 0;
#endif
    siuOpMode                                   = SIUMODE_MPEGAVI;  // fix dcfWrite() write wrong data bug
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    v_flag[pVideoClipOption->RFUnit] = 0;
    a_flag[pVideoClipOption->RFUnit] = 0;
#endif    
    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
        pVideoClipOption->VideoBufMngReadIdx        = 0;
        pVideoClipOption->VideoBufMngWriteIdx       = 0;
        for(i = 0; i < VIDEO_BUF_NUM; i++) {
            pVideoClipOption->VideoBufMng[i].buffer = pVideoClipOption->VideoBuf;
        }
#ifdef ASF_AUDIO
        pVideoClipOption->iisSounBufMngReadIdx      = 0;
        pVideoClipOption->iisSounBufMngWriteIdx     = 0;
        for(i = 0; i < IIS_BUF_NUM; i++) {
            pVideoClipOption->iisSounBufMng[i].buffer   = pVideoClipOption->iisSounBuf[i];
        }
#endif
    } else if(pVideoClipOption->AV_Source == RX_RECEIVE) {
    /*
        pVideoClipOption->VideoBufMngReadIdx        = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->VideoBufMngWriteIdx       = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
#ifdef ASF_AUDIO
        pVideoClipOption->iisSounBufMngReadIdx      = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->iisSounBufMngWriteIdx     = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
#endif
    */
	
    #ifdef ASF_AUDIO
		//Lsk: FIXME cause may CurrentAudioSize<0;  
        while(OSSemAccept(pVideoClipOption->iisCmpSemEvt));
        OS_ENTER_CRITICAL();
        pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->CurrentAudioSize      = 0;
        OS_EXIT_CRITICAL();

        while(pVideoClipOption->iisCmpSemEvt->OSEventCnt || pVideoClipOption->CurrentAudioSize || (pVideoClipOption->iisSounBufMngReadIdx != rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]))
        {
            OS_ENTER_CRITICAL();
            pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
            pVideoClipOption->CurrentAudioSize      = 0;
            OS_EXIT_CRITICAL();
            OSSemAccept(pVideoClipOption->iisCmpSemEvt);
        };
    #endif
        while(OSSemAccept(pVideoClipOption->VideoCmpSemEvt));
        OS_ENTER_CRITICAL();
        pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
        pVideoClipOption->CurrentVideoSize      = 0;
        pVideoClipOption->CurrentVideoTime      = 0;
        OS_EXIT_CRITICAL();
        while(pVideoClipOption->VideoCmpSemEvt->OSEventCnt || pVideoClipOption->CurrentVideoSize || pVideoClipOption->CurrentVideoTime || (pVideoClipOption->VideoBufMngReadIdx != rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]))
        {
            OS_ENTER_CRITICAL();
            pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
            pVideoClipOption->CurrentVideoSize      = 0;
            pVideoClipOption->CurrentVideoTime      = 0;
            OS_EXIT_CRITICAL();
            OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
        };

        // find I frame, 第一張必須是 I frame
        //DEBUG_ASF("Ch%d iisSounBufMngReadIdx        = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->iisSounBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxIIsSounBufMngWriteIdx = %d\n", pVideoClipOption->VideoChannelID, rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d VideoBufMngReadIdx          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxVideoBufMngWriteIdx   = %d\n", pVideoClipOption->VideoChannelID, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d finding I frame...\n", pVideoClipOption->VideoChannelID);
        while(pVideoClipOption->sysCaptureVideoStop == 0)
        {
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            if(Record_flag[pVideoClipOption->RFUnit] == 1)
            {
                pVideoClipOption->iisSounBufMngReadIdx = 0;
                pVideoClipOption->VideoBufMngReadIdx =0;
            }
          #endif
        #ifdef ASF_AUDIO
            if(pVideoClipOption->iisSounBufMngReadIdx != rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit])
            {
                DEBUG_ASF(" #%d ", __LINE__);
                RunAudio        = 1;
                audio_value     = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                if(audio_value)
                    pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                else    // something wrong
                {
                    //pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
                    //DEBUG_ASF(" #A%d ", pVideoClipOption->VideoChannelID);
                    OSTimeDly(1);
                }
            } else {
                RunAudio        = 0;
            }
        #endif
            if(pVideoClipOption->VideoBufMngReadIdx != rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit])
            {
                DEBUG_ASF(" #%d ", __LINE__);
                RunVideo        = 1;
                if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    //DEBUG_ASF("#");
                    if(video_value){
                        OS_ENTER_CRITICAL();
                        if(pVideoClipOption->CurrentVideoSize >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset)
                            pVideoClipOption->CurrentVideoSize   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset;
                        else 
                        {
                            DEBUG_ASF("VideoSize error %d, %d\n", pVideoClipOption->CurrentVideoSize, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].offset);
                            pVideoClipOption->CurrentVideoSize = 0;
                        }
                        
                        if(pVideoClipOption->CurrentBufferSize >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size)
                            pVideoClipOption->CurrentBufferSize   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size;
                        else
    			        {
                            DEBUG_ASF("BufferSize error %d, %d\n", pVideoClipOption->CurrentBufferSize, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size);
                            pVideoClipOption->CurrentBufferSize = 0;
                        }

                        if(pVideoClipOption->CurrentVideoTime >=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time)
                            pVideoClipOption->CurrentVideoTime   -=  pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time;
                        else
                        {
                            DEBUG_ASF("Time error %d, %d\n", pVideoClipOption->CurrentVideoTime, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].time);
                            pVideoClipOption->CurrentVideoTime = 0;
                        }

                        OS_EXIT_CRITICAL();                        
                        pVideoClipOption->VideoBufMngReadIdx    = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                    }
                    else    // something wrong
                    {
                        //pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                        DEBUG_ASF(" #V%d ", pVideoClipOption->VideoChannelID);
                        OSTimeDly(1);
                    }
                } else {
                    DEBUG_ASF("\nCh%d find I frame ok\n", pVideoClipOption->VideoChannelID);
                    break;
                }
            } else {
                RunVideo        = 0;
            }
            if(RunAudio == 0 && RunVideo == 0) {
                DEBUG_ASF(" #%d ", __LINE__);
                OSTimeDly(1);
            }
        }
        DEBUG_ASF("BufSize:%d, BufTime:%d, Vcnt:%d, (%d, %d)\n", pVideoClipOption->CurrentVideoSize, pVideoClipOption->CurrentVideoTime, pVideoClipOption->VideoCmpSemEvt->OSEventCnt, pVideoClipOption->VideoBufMngReadIdx, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d iisSounBufMngReadIdx        = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->iisSounBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxIIsSounBufMngWriteIdx = %d\n", pVideoClipOption->VideoChannelID, rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit]);
        //DEBUG_ASF("Ch%d VideoBufMngReadIdx          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
        //DEBUG_ASF("Ch%d rfiuRxVideoBufMngWriteIdx   = %d\n", pVideoClipOption->VideoChannelID, rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit]);
        if(pVideoClipOption->sysCaptureVideoStop)
        {
          #if INSERT_NOSIGNAL_FRAME
            OS_ENTER_CRITICAL();
            Record_flag[pVideoClipOption->VideoChannelID] = 0;
            OS_EXIT_CRITICAL();
          #endif
            DEBUG_ASF("Ch%d stop video record!!!\n", pVideoClipOption->VideoChannelID);
            return 1;
        }
    }


    #if (HW_BOARD_OPTION == MR6730_AFN)
      #if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	// multichannel
        #if(MULTI_CH_DEGRADE_1CH)	
            //only one channel supportted
        #else
         u8 RecingChNum=0;

         RecingChNum=(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_RECON))+(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON));
         if(RecingChNum)
         {//if there are more than one channel recording, waiting for its buddy ready then to go on

             int Ch_Id=0;
             int Ch_Id_buddy=0;
             
             ASSERT_OP (pVideoClipOption->VideoChannelID, > , 0);
             ASSERT_OP (pVideoClipOption->VideoChannelID, <= , 2);
             Ch_Id=(pVideoClipOption->VideoChannelID-1);	
             Ch_Id_buddy=(Ch_Id==0)?1:0;
             if(Ch_Id<2)
             {						
                 DEBUG_ASF("<REC_SYNC> CH%d REC mode:%d capture initial.(t:%d)\n",(Ch_Id+1),setUI.RecMode,OSTimeGet());//debug			
               #if (USE_REC_TERM_MON)	
                 if(MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECTERM|UI_SET_STATUS_BITMSK_CH2_RECTERM)))			
                 {//exit if detect recording terminated
                     pVideoClipOption->sysCaptureVideoStart	= 0;
                     pVideoClipOption->sysCaptureVideoStop	= 1;
                     DEBUG_ASF("<REC_SYNC> CH%d exit REC due to detect terminated.\n",(Ch_Id+1));//debug
                     return 0;
                 }
               #endif 
             }//if(Ch_Id<2)
         }//if(RecingChNum)
        #endif
      #endif //#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	// multichannel
    #endif //#if (HW_BOARD_OPTION == MR6730_AFN)

    // for disk full control

    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL))
    {
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (dcfGetMainStorageFreeSize() <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX) / 1024)) //Notice: K-Byte unit
        {
            DEBUG_ASF("1.Disk full!!!\n");
            DEBUG_ASF("free_size = %d KBytes\n", dcfGetMainStorageFreeSize());
            system_busy_flag    = 1;
            //uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 98 + osdYShift / 2 , OSD_Blk2, 0x00 , alpha_3);
            //osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126 + osdYShift / 2, OSD_Blk2, 0xC0, 0x00);
            uiOsdDrawSDCardFULL(UI_OSD_DRAW);
            MemoryFullFlag      = TRUE;
            // 偵測到卡滿就全部關閉錄影
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                if(MultiChannelGetCaptureVideoStatus(i))
                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
            }
            OSTimeDly(15);
            return 0;
        }

		#if (HW_BOARD_OPTION == MR6730_AFN)
		if(pVideoClipOption->sysCaptureVideoStop)
		{
			DEBUG_ASF("Ch%d stop video record!(a)\n", pVideoClipOption->VideoChannelID);
			return 1;
		}
		#endif //#if (HW_BOARD_OPTION == MR6730_AFN)
		
        pVideoClipOption->SetIVOP = 1;
      #if INSERT_NOSIGNAL_FRAME
        if(MultiChannelAsfCreateFile(1, pVideoClipOption) == 0)
      #else
        if(MultiChannelAsfCreateFile(0, pVideoClipOption) == 0)
      #endif
        {
            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                if(MemoryFullFlag == TRUE)
                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                else
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
            return 0;
        }
    }

	#if (HW_BOARD_OPTION == MR6730_AFN)
	if(pVideoClipOption->sysCaptureVideoStop)
	{
		DEBUG_ASF("Ch%d stop video record!(b)\n", pVideoClipOption->VideoChannelID);
		MultiChannelAsfCloseFile(pVideoClipOption);
		return 1;
	}
	#endif //#if (HW_BOARD_OPTION == MR6730_AFN)
	
	if(pVideoClipOption->AV_Source == LOCAL_RECORD)
	{
	#if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
	    timerCountRead(2, (u32*) &TimeOffset);
    	pVideoClipOption->IISTimeOffset = TimeOffset >> 8;
	#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
		timerCountRead(1, (u32*) &TimeOffset);
	    pVideoClipOption->IISTimeOffset = TimeOffset / 100;
	#endif

	#ifdef ASF_AUDIO
        MultiChannelIISRecordTaskCreate(pVideoClipOption);
        adcInit(1);
      #if AUDIO_IN_TO_OUT
        iisResumePlaybackTask();
      #endif
    #endif  // #ifdef ASF_AUDIO

	    switch(pVideoClipOption->VideoChannelID)
	    {
	        case 0:
	            isuCaptureVideo(0);
	            ipuCaptureVideo();
	            siuCaptureVideo(0);
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(isu_avifrmcnt < 4)
	            {
	               DEBUG_ASF("asf w01\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t01\n");
	                   break;
	               }

	            }
	            isu_avifrmcnt           = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(isuSemEvt, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            isu_avifrmcnt           = 0;
	            timeoutcnt              = 0;
	            while(isu_avifrmcnt < 1)
	            {
	                DEBUG_ASF("asf w02\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt > 5)
	                {
	                    DEBUG_ASF("asf t02\n");
	                    DEBUG_ASF("Error: timeout 02\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(isu_avifrmcnt < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 1:
	            #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                 #if(IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE)  // 避免CIU Bob mode切換時 size會有變化
                    #if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
                       ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, 800 * 2);
                    #else
                       ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    #endif
				 #elif (HW_BOARD_OPTION == MR6730_AFN && CIU_BOB_MODE) 
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
									
                 #else
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(640, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(640, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                #endif
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(704, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(704, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH1(720, 576/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH1(720, 480/2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height/2, CIU1_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
	            #else
	            ciuCaptureVideo_CH1(isuSrcImg.w, isuSrcImg.h, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height, CIU1_OSD_EN, pVideoClipOption->mpeg4Width);
	            #endif
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch1 < 4)
	            {
	               DEBUG_ASF("asf w11\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t11\n");
	                   break;
	               }

	            }
	            ciu_idufrmcnt_ch1       = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(ciuCapSemEvt_CH1, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            ciu_idufrmcnt_ch1       = 0;
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch1 < 1)
	            {
	                DEBUG_ASF("asf w2\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt > 5)
	                {
	                    DEBUG_ASF("asf t12\n");
	                    DEBUG_ASF("Error: timeout 12\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(ciu_idufrmcnt_ch1 < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 2:
	            #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                    if(sysTVinFormat == TV_IN_PAL)
                    #if(IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN))  // 避免CIU Bob mode切換時 size會有變化
                       ciuCaptureVideo_CH2(640, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    #else
                       ciuCaptureVideo_CH2(640, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    #endif
                    else
    	               ciuCaptureVideo_CH2(640, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH2(704, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH2(704, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
                {
                    if(sysTVinFormat == TV_IN_PAL)
                       ciuCaptureVideo_CH2(720, 576 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                    else
    	               ciuCaptureVideo_CH2(720, 480 / 2, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height / 2, CIU2_OSD_EN, pVideoClipOption->mpeg4Width * 2);
                }
	            #else
	            ciuCaptureVideo_CH2(isuSrcImg.w, isuSrcImg.h, pVideoClipOption->mpeg4Width, pVideoClipOption->mpeg4Height, CIU2_OSD_EN, pVideoClipOption->mpeg4Width);
	            #endif
	        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch2 < 4)
	            {
	               DEBUG_ASF("asf w21\n");

	               OSTimeDly(1);
	               timeoutcnt++;
	               if (timeoutcnt > 10)
	               {
	                   DEBUG_ASF("asf t21\n");
	                   break;
	               }

	            }
	            ciu_idufrmcnt_ch2       = 0;
	            mp4_avifrmcnt           = 0;
	            OSSemSet(ciuCapSemEvt_CH2, 0, &err);
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            ciu_idufrmcnt_ch2       = 0;
	            timeoutcnt              = 0;
	            while(ciu_idufrmcnt_ch2 < 1)
	            {
	                DEBUG_ASF("asf w22\n");

	                OSTimeDly(1);
	                timeoutcnt++;
	                if (timeoutcnt>5)
	                {
	                    DEBUG_ASF("asf t22\n");
	                    DEBUG_ASF("Error: timeout 22\n");
	                    break;
	                }
	            }
	        #else
	            pVideoClipOption->sysReady2CaptureVideo   = 1;
	            while(ciu_idufrmcnt_ch2 < 1)
	            {
	                OSTimeDly(1);
	            }
	        #endif
	            break;
	        case 3:
	        default:
	            DEBUG_ASF("Errpr: MultiChannelAsfCaptureVideo can't support Ch%d !!!\n", pVideoClipOption->VideoChannelID);
	    }

        MultiChannelMPEG4EncoderTaskCreate(pVideoClipOption);
	} else {    // if(pVideoClipOption->AV_Source != LOCAL_RECORD)
    	siuOpMode   = SIUMODE_MPEGAVI;
    }

	#if (HW_BOARD_OPTION==MR6730_AFN)//#if (USE_UI_TASK_WDT)

	if(pVideoClipOption->VideoChannelID<MULTI_CHANNEL_LOCAL_MAX)
		MulChASF_Error[pVideoClipOption->VideoChannelID]=0;//reset

	#if (USE_REC_TERM_MON)
	if(pVideoClipOption->sysCaptureVideoStop==0)
	{
		if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_RECON))
		{	
			if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_ISREC))
				MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH1_TSKRDY);
		}
		#if(!MULTI_CH_DEGRADE_1CH)	
		if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON))
		{	
			if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_ISREC))
				MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH2_TSKRDY);
		}	
		#endif
	}
	#endif//#if (USE_REC_TERM_MON)	
	//
	//DEBUG_ASF("CH%d enter loop...(%d)\n", pVideoClipOption->VideoChannelID,pVideoClipOption->sysCaptureVideoStop);
	if(pVideoClipOption->sysCaptureVideoStop==0)
		DEBUG_ASF("CH%d enter loop...\n", pVideoClipOption->VideoChannelID);
	else
		DEBUG_ASF("CH%d skip loop,\n", pVideoClipOption->VideoChannelID);	
	#endif

    #if AUDIO_DEBUG_ENA
    ASFiiscnt = *pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer;
    ASFiiscnt = ASFiiscnt<<8;

    ASFiiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+1);
    ASFiiscnt = ASFiiscnt<<8;

    ASFiiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+2);
    ASFiiscnt = ASFiiscnt<<8;

    ASFiiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+3);

    DEBUG_ASF("#1 idx %d,stream %02x%02x%02x%02x, cnt %d\n",pVideoClipOption->iisSounBufMngReadIdx,*(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer),
                                                *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+1), 
                                                *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+2), 
                                                *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+3),
                                                ASFiiscnt);
    #endif
    
    while (pVideoClipOption->sysCaptureVideoStop == 0)
    {
      #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
        if((pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT) && (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && (Record_flag[pVideoClipOption->RFUnit] == 1))
        {
            pVideoClipOption->sysCaptureVideoStart	= 0;
            pVideoClipOption->sysCaptureVideoStop	= 1;
            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
            DEBUG_ASF("EventTrigger  %d asfCaptureMode %d\n ", pVideoClipOption->EventTrigger,pVideoClipOption->asfCaptureMode);
            break;
        }
      #endif            
        if((pVideoClipOption->AV_Source == LOCAL_RECORD) && (pVideoClipOption->MPEG4_Error == 1))
        {
            MultiChannelAsfCloseFile(pVideoClipOption);
            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
        #ifdef  ASF_AUDIO
            //MultiChannelIIsStopRec(pVideoClipOption);
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        #endif
            return 0;
        }

        /**********************************************************************************************************************
        *** Trigger mode FSM                                                                                                ***
        *** CAPTURE_STATE_WAIT --------> CAPTURE_STATE_TRIGGER --------> CAPTURE_STATE_TIMEUP --------> CAPTURE_STATE_WAIT  ***
        *** WAIT    -> TRIGGER : event trigger, start wrtite A/V bitstream                                                  ***
        *** TRIGGER -> TIMEUP  : Time's up, store lastest video payload index                                               ***
        *** TIMEUP  -> WAIT    : Write Finish film slice, return to wating state                                            ***
        **********************************************************************************************************************/
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
    		{
    		    //Start_MPEG4TimeStatistics = 0;
		      #if (HW_BOARD_OPTION==MR6730_AFN)
 			   #if (USE_REC_TERM_MON)
 				if(MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECTERM|UI_SET_STATUS_BITMSK_CH2_RECTERM)))			
 				{//exit if detect recording terminated
 					pVideoClipOption->sysCaptureVideoStart	= 0;
 					pVideoClipOption->sysCaptureVideoStop	= 1;
 					break;
 				}
 			   #endif     		    
		      #endif    		    
    		    MultiChannelCheckEventTrigger(pVideoClipOption);
                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_TRIGGER\n\n", pVideoClipOption->VideoChannelID);
                  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    Lose_audio_time[pVideoClipOption->RFUnit]=0;
                    Lose_video_time[pVideoClipOption->RFUnit]=0;
                  #endif
                   #if 0
                    /****************************************************************
                    *** Calculate how many VOP in SDRAM need to drop              ***
                    ****************************************************************/
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        OS_ENTER_CRITICAL();
                        if(pVideoClipOption->VideoBufMngWriteIdx >= pVideoClipOption->VideoBufMngReadIdx)
                        {
                            //---R---s--W---
                            //if((pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum >= pVideoClipOption->VideoBufMngReadIdx)&&(pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum <= pVideoClipOption->VideoBufMngWriteIdx))
                            if(pVideoClipOption->VideoBufMngWriteIdx >= (pVideoClipOption->VideoBufMngReadIdx + PreRecordFrameNum))
                            {
                                SkipFrameNum = pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---R---W--s---
                            //---s---R--W---
                            else
                                SkipFrameNum = 0;
                        }
                        else
                        {
                            //---s---w--R---
                            //---w---R--s---
                            if( (VIDEO_BUF_NUM + pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum) >= pVideoClipOption->VideoBufMngReadIdx)
                            {
                                SkipFrameNum = VIDEO_BUF_NUM + pVideoClipOption->VideoBufMngWriteIdx - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---w---s--R---
                            else
                                SkipFrameNum = 0;
                        }
                        OS_EXIT_CRITICAL();
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        OS_ENTER_CRITICAL();
                        if(rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] >= pVideoClipOption->VideoBufMngReadIdx)
                        {
                            //---R---s--W---
                            if(rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] >= (pVideoClipOption->VideoBufMngReadIdx + PreRecordFrameNum))
                            {
                                SkipFrameNum = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---R---W--s---
                            //---s---R--W---
                            else
                                SkipFrameNum = 0;
                        }
                        else
                        {
                            //---s---w--R---
                            //---w---R--s---
                            if( (VIDEO_BUF_NUM + rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum) >= pVideoClipOption->VideoBufMngReadIdx)
                            {
                                SkipFrameNum = VIDEO_BUF_NUM + rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit] - PreRecordFrameNum - pVideoClipOption->VideoBufMngReadIdx;
                            }
                            //---w---s--R---
                            else
                                SkipFrameNum = 0;
                        }
                        OS_EXIT_CRITICAL();
                    }
                  #endif
                    /******************************************
                    *** Force Mpeg4 Engine compress I frame ***
                    ******************************************/
                    if(pVideoClipOption->DirectlyTimeStatistics == 0)
                    {
                        pVideoClipOption->SetIVOP = 1;
                        OSTimeDly(2);
                    }
                    /***************************
                    *** drop Audio and video ***
                    ***************************/
                    if(pVideoClipOption->CurrentVideoSize && pVideoClipOption->FreeSpaceControl)
                    {
                        SingleFrameSize     = pVideoClipOption->CurrentVideoSize / (pVideoClipOption->VideoCmpSemEvt->OSEventCnt);
                        FreeSpaceThreshold  = 4 * 30 * SingleFrameSize + MPEG4_MIN_BUF_SIZE;
                        FreeSpace           = MPEG4_MAX_BUF_SIZE - pVideoClipOption->CurrentVideoSize;
                        DEBUG_ASF("FreeSpace control (%d,%d)\n", FreeSpaceThreshold, FreeSpace);
                    }
                    else
                        pVideoClipOption->FreeSpaceControl = 0;

                    //DEBUG_ASF("SkipFrameNum = %d\n", SkipFrameNum);

                    level= sysCheckSDCD();
                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                        DEBUG_ASF("SDCD OFF\n");
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        return 0;
                    }
                  #if ASF_DEBUG_ENA
                    printf("************************ Trigger occur - 001 ************************\n");
                    {
                    	RTC_DATE_TIME   dateTime;					
                    	RTC_Get_Time(&dateTime);			
                    	printf("<TTT> Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
                    }
                    skip_I=0;
                    skip_P=0;
                    skip_A=0;

                    printf("<TTT>1. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                    printf("<TTT>1. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                    printf("<TTT>1. pVideoClipOption->CurrentVideoSize           = %d\n", pVideoClipOption->CurrentVideoSize);	
                    printf("<TTT>1. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                    printf("<TTT>1. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                  #endif
                    //while((pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag != FLAG_I_VOP) || (SkipFrameNum > 0)
                    while((pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
                      #if (HW_BOARD_OPTION==MR6730_AFN)
                        || (pVideoClipOption->CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE * 4)) //??
                      #else
                        || (pVideoClipOption->CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE))
                      #endif	
                        || (pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000))
                        #if CHECK_VIDEO_BITSTREAM
						|| (*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer) != 0xB6010000)
						|| (*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4) != 0x000001B6)
						#endif
                        || (pVideoClipOption->FreeSpaceControl && (FreeSpace < FreeSpaceThreshold)))
                    {
                        //SkipFrameNum--;
                        if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP)
                            DEBUG_ASF("CH%d, %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx);
                        if(pVideoClipOption->CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE))
							DEBUG_ASF("CH%d, %d, %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,(u32)pVideoClipOption->CurrentVideoSize);
                        if(pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000))
                            DEBUG_ASF("CH%d, %d, %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,pVideoClipOption->CurrentVideoTime);
                        if(pVideoClipOption->FreeSpaceControl && (FreeSpace < FreeSpaceThreshold))
                            DEBUG_ASF("CH%d, %d, %d %d\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx,FreeSpace,FreeSpaceThreshold);
						#if CHECK_VIDEO_BITSTREAM
						if(*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer) != 0xB6010000)
							DEBUG_RED("CH%d, %d, %d, 0x%08x\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag, *(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer));
						if(*(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4) != 0x000001B6)
							DEBUG_RED("CH%d, %d, %d, 0x%08x\n",pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag, *(unsigned int*)(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer + pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size - 4));		
						#endif
                        video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                      #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                        if(Record_flag[pVideoClipOption->RFUnit] == 1)
                        {
                            pVideoClipOption->sysCaptureVideoStart	= 0;
                            pVideoClipOption->sysCaptureVideoStop	= 1;
                            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                            break;
                        }
                      #endif
                        if (video_value > 0) 
                        {
                          #if ASF_DEBUG_ENA
                            if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                            	skip_I++;
                            else
                            	skip_P++;
                          #endif
                            if(video_value_max < video_value)
                                video_value_max = video_value; ////Lsk : for what ?
                            #if CDVR_LOG
                            if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                                pVideoClipOption->LogFileStart      = (pVideoClipOption->LogFileStart + 1) % LOG_INDEX_NUM;
                            #endif
                            MultiChannelAsfWriteVirtualVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx]);
                            pVideoClipOption->VideoBufMngReadIdx    = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                                OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                            if(pVideoClipOption->FreeSpaceControl)
                                FreeSpace = MPEG4_MAX_BUF_SIZE - pVideoClipOption->CurrentVideoSize;
                        } 
                        else 
                        {
        				    DEBUG_ASF("\n\nCH%d Can't start from I frame %d, %d!!!\n\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngReadIdx, pVideoClipOption->VideoBufMngWriteIdx);
                            pVideoClipOption->EventTrigger  = CAPTURE_STATE_WAIT;
                            break;
                        }

                        level= sysCheckSDCD();
                        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                        {
                            DEBUG_ASF("SDCD OFF\n");
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            return 0;
                        }
                    }
                  #if ASF_DEBUG_ENA
                    printf("<TTT>2. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);										
                    printf("<TTT>2. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                    printf("<TTT>2. pVideoClipOption->CurrentVideoSize           = %d\n", pVideoClipOption->CurrentVideoSize);	
                    printf("<TTT>2. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                    printf("<TTT>2. skip <%d, %d>\n"                                    , skip_I, skip_P);	
                    printf("<TTT>2. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                  #endif
                    pVideoClipOption->start_idx     = pVideoClipOption->VideoBufMngReadIdx;

                    if(pVideoClipOption->start_idx != pVideoClipOption->end_idx)
                    {
                        DEBUG_ASF("\n Warning!!! Ch%d lose video slice....\n", pVideoClipOption->VideoChannelID);
                    }
               #ifdef ASF_AUDIO

                    level= sysCheckSDCD();
                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                        DEBUG_ASF("SDCD OFF\n");
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        return 0;
                    }
                  #if INSERT_NOSIGNAL_FRAME
                    while ((pVideoClipOption->sysAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->sysVidePresentTime)
                  #else
                    while ((pVideoClipOption->asfAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->asfVidePresentTime)
                  #endif
                    {
                        audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                      #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                        if(Record_flag[pVideoClipOption->RFUnit] == 1)
                        {
                            pVideoClipOption->sysCaptureVideoStart	= 0;
                            pVideoClipOption->sysCaptureVideoStop	= 1;
                            Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                            break;
                        }
                      #endif
                        if (audio_value > 0) {
                            if(audio_value_max < audio_value)
                                audio_value_max = audio_value; //Lsk : for what ?
                          #if ASF_DEBUG_ENA
                            skip_A++;
                          #endif
                            MultiChannelAsfWriteVirtualAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx]);
                          #if AUDIO_DEBUG_ENA
                            ASFiiscnt++;
                          #endif
                            pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                            OSSemPost(pVideoClipOption->iisTrgSemEvt);
                        } else {
                            break;
                        }
                        level= sysCheckSDCD();
	                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
	                    {
	                        DEBUG_ASF("SDCD OFF\n");
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
	                        return 0;
	                    }
                    }
                  #if ASF_DEBUG_ENA
                    printf("<TTT>3. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);															
                    printf("<TTT>3. skip <%d>\n"                                    , skip_A);	
                    printf("<TTT>3. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);				
                    printf("************************ Trigger sync end ************************\n");
                  #endif
                #endif
                }

                /*******************************************
                *** seek to I fram, open a new asf file  ***
                *******************************************/
                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
                {
					
                  #if (HW_BOARD_OPTION == MR6730_AFN)//sss1
                    if(MD_TrigCnt==0)	
                    {//do this once at first triggered
                      #if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	// multichannel	
                        u8 TimeChg=1;		
                        u8 RecingChNum=0;
                        										
                        RecingChNum=(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_RECON))+(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON));
                        if(RecingChNum)
                        {//if there are more than one channel recording, waiting for its buddy ready then to go on
                        						
                            ASSERT_OP (pVideoClipOption->VideoChannelID, > , 0);
                            ASSERT_OP (pVideoClipOption->VideoChannelID, <= , 2);
                          #if (USE_REC_TERM_MON)	
                            if(MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECTERM|UI_SET_STATUS_BITMSK_CH2_RECTERM)))			
                            {//exit if detect recording terminated
                            	pVideoClipOption->sysCaptureVideoStart	= 0;
                            	pVideoClipOption->sysCaptureVideoStop	= 1;									
                            	break;
                            }
                          #endif 
                        }//if(RecingChNum)
                      #endif //#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	
                    }
                    MD_TrigCnt++;
                  #endif	//#if (HW_BOARD_OPTION == MR6730_AFN)  
                  #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        pVideoClipOption->AV_TimeBase  = PREROLL;
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(0, pVideoClipOption)) == 0) {
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                            if(MemoryFullFlag == TRUE)
                                uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                            else
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                    }
                  #elif (TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0)
                        {
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                      #if ASF_DEBUG_ENA
                        printf("<FFF> ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, 
                                                                pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                        printf("<FFF> RX time <%d, %d>\n", RX_time_A, RX_time_V);						
                        printf("<FFF> RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);						
                        printf("<FFF> RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V,
                                                                                     RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                      #endif
                    }
                    else
                    {
                        /*** Reset Audio/Video time biase ***/
                        pVideoClipOption->ResetPayloadPresentTime = 0; //reset video time base
                        pVideoClipOption->AV_TimeBase  = PREROLL;
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0) 
                        {

                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                      #if ASF_DEBUG_ENA
                        printf("<FFF> ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime,
                                                                pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                        printf("<FFF> RX time <%d, %d>\n", RX_time_A, RX_time_V);						
                        printf("<FFF> RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);						
                        printf("<FFF> RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V,
                                                                                     RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
                      #endif
                    }
                  #endif
                }
            }
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
            {
                /*** check record time period ***/
                MultiChannelCheckRecordTimeUP(pVideoClipOption);

                /*** TODO ***/
                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP)
                {
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_TIMEUP\n\n", pVideoClipOption->VideoChannelID);					
                }
            }
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP)
            {
                MultiChannelCheckWriteFinish(pVideoClipOption);

                if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
                {
                    monitor_value = 0;

                    /*** Reset Audio/Video time biase ***/
                    #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    pVideoClipOption->ResetPayloadPresentTime = 0; //reset video time base
                    if(pVideoClipOption->asfVidePresentTime >= pVideoClipOption->asfAudiPresentTime )
                    {
                		pVideoClipOption->AV_TimeBase   = pVideoClipOption->asfVidePresentTime + 100; // 0.1s suspend
                    }
                    else
                    {
                        pVideoClipOption->AV_TimeBase   = pVideoClipOption->asfAudiPresentTime + 100; // 0.1s suspend
                    }
                    /*** Close ASF file ***/
                    #elif(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    #if INSERT_NOSIGNAL_FRAME
                    OS_ENTER_CRITICAL();
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                    {
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                      #if (NOSIGNAL_MODE == 3)
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                      #endif
                    }
                    OS_EXIT_CRITICAL();
                    #endif
                    if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) 
                    {
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }
                    #endif
                    DEBUG_ASF("\n\nFSM : Ch%d CAPTURE_STATE_WAIT\n\n", pVideoClipOption->VideoChannelID);

				#if (HW_BOARD_OPTION == MR6730_AFN)
				#if (USE_REC_TERM_MON)	
				if(MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECTERM|UI_SET_STATUS_BITMSK_CH2_RECTERM)))			
				{//exit if detect recording terminated
					pVideoClipOption->sysCaptureVideoStart	= 0;
					pVideoClipOption->sysCaptureVideoStop	= 1;
					break;
				}
				#endif 
				#endif //#if (HW_BOARD_OPTION == MR6730_AFN)
				
                #if (UI_VERSION == UI_VERSION_RDI) || (HW_BOARD_OPTION == MR6730_AFN) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)
                    uiOsdDrawSysAfterRec(pVideoClipOption);
                #endif
                    if(pVideoClipOption->WantToExitPreRecordMode)
                    {
                        pVideoClipOption->sysCaptureVideoStart    = 0;
                        pVideoClipOption->sysCaptureVideoStop     = 1;
                    }
                }
            }
		#if (HW_BOARD_OPTION == MR6730_AFN)
			MD_TrigCnt=0;
		#endif 				
        }

        /**********************************
        **** Write Audio/Video Payload ****
        **********************************/
        if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            #ifdef ASF_AUDIO
            // ------Write audio payload------//
            if((video_value == 0) || (pVideoClipOption->asfAudiPresentTime <= pVideoClipOption->asfVidePresentTime))
            {
              #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                if(Record_flag[pVideoClipOption->RFUnit] == 1)
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                    if(audio_value == 0)
                    {
                        a_flag[pVideoClipOption->RFUnit] = 1;
                    }
                    else
                    {
                        a_flag[pVideoClipOption->RFUnit] = 0;
                        Audio_RF_index[pVideoClipOption->RFUnit] = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
                    }
                }
                else
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                    if((audio_value > 0) && (a_flag[pVideoClipOption->RFUnit] == 1))
                    {
                        pVideoClipOption->iisSounBufMngReadIdx = 0;
                        a_flag[pVideoClipOption->RFUnit] = 0;
                        
                        if(Lose_audio_time[pVideoClipOption->RFUnit] > 0)
                        {
                            pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time -= 128;
                            Lose_audio_time[pVideoClipOption->RFUnit] += pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time;
                            Lose_audio_time[pVideoClipOption->RFUnit] = (Lose_audio_time[pVideoClipOption->RFUnit]/128);
                            for(i=0; i<Lose_audio_time[pVideoClipOption->RFUnit]; i++)
                                MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],2);
                            Lose_audio_time[pVideoClipOption->RFUnit] = 0;
                        }
                        Audio_RF_index[pVideoClipOption->RFUnit] = 2000000;
                        pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time = 128;
                    }
                    else if((audio_value > 0) && (a_flag[pVideoClipOption->RFUnit] == 0))
                    {
                        if(Audio_RF_index[pVideoClipOption->RFUnit] == pVideoClipOption->iisSounBufMngReadIdx)
                        {
                            pVideoClipOption->iisSounBufMngReadIdx = 0;
                            a_flag[pVideoClipOption->RFUnit] = 0;
                            
                            if(Lose_audio_time[pVideoClipOption->RFUnit] > 0)
                            {
                                pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time -= 128;
                                Lose_audio_time[pVideoClipOption->RFUnit] += pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time;
                                Lose_audio_time[pVideoClipOption->RFUnit] = (Lose_audio_time[pVideoClipOption->RFUnit]/128);
                                for(i=0; i<Lose_audio_time[pVideoClipOption->RFUnit]; i++)
                                    MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],2);
                                Lose_audio_time[pVideoClipOption->RFUnit] = 0;
                            }
                            pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].time = 128;
                            Audio_RF_index[pVideoClipOption->RFUnit] = 2000000;
                        }
                    }
                }
              #else
                audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
              #endif
                if (audio_value > 0)
                {
                    if(audio_value_max < audio_value)
                        audio_value_max = audio_value;
                #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
                    if (MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],0) == 0)
                #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
                    if (MultiChannelAsfWriteAudiPayload_IMA_ADPCM(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx], &PcmOffset) == 0)
                #endif
                    {
                        DEBUG_ASF("Ch%d ASF write audio payload error!!!\n", pVideoClipOption->VideoChannelID);
                        /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                    #if 0
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    #else   // 修正index不見的問題
                        // write index object //
                        if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                            //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }

                        /* close file */
                        if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                            DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                        pVideoClipOption->OpenFile  = 0;
                    #endif
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }

					#if AFN_DEBUG
					#if 0//(AFN_DEBUG_MORE_CHECK>0)
						if(pVideoClipOption->VideoChannelID<MULTI_CHANNEL_LOCAL_MAX)
						{
							if(MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]>MULCH_IIS_ERRCHK_THRES)
							{
							#if( HW_BOARD_OPTION == MR6730_AFN )							
								DEBUG_ASF("Ch%d Audio too much error(%d), ", pVideoClipOption->VideoChannelID, MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]);
									#if 1
								//stop recording only
									DEBUG_ASF("terminate recording.\n");
									//terminate recording if detect audio stop
									pVideoClipOption->sysCaptureVideoStart	= 0;
									pVideoClipOption->sysCaptureVideoStop	= 1;
						
								#endif
							#endif
							}
						}
					#endif		
					#endif //#if AFN_DEBUG
                    #if AUDIO_DEBUG_ENA
                    ASFiiscnt++;
                    #endif
                    pVideoClipOption->iisSounBufMngReadIdx  = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    OSSemPost(pVideoClipOption->iisTrgSemEvt);
                }
            }

            //------ Write video payload------//
            if((audio_value == 0) || (pVideoClipOption->asfAudiPresentTime >= pVideoClipOption->asfVidePresentTime))
            {
            #endif      // ASF_AUDIO

              #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                if(Record_flag[pVideoClipOption->RFUnit] == 1)
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    if(video_value == 0)
                        v_flag[pVideoClipOption->RFUnit] = 1;
                    else
                    {
                        v_flag[pVideoClipOption->RFUnit] = 0;
                        Video_RF_index[pVideoClipOption->RFUnit] = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                    }
                }
                else
                {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                    if((video_value > 0) && (v_flag[pVideoClipOption->RFUnit] == 1))
                    {
                        pVideoClipOption->VideoBufMngReadIdx = 0;
                        v_flag[pVideoClipOption->RFUnit] = 0;
                        Video_RF_index[pVideoClipOption->RFUnit] = 2000000;
                    }
                    else if((video_value > 0) && (v_flag[pVideoClipOption->RFUnit] == 0))
                    {
                        if(pVideoClipOption->VideoBufMngReadIdx == Video_RF_index[pVideoClipOption->RFUnit])
                        {
                            pVideoClipOption->VideoBufMngReadIdx = 0;
                            Video_RF_index[pVideoClipOption->RFUnit] = 2000000;
                            v_flag[pVideoClipOption->RFUnit] = 0;
                        }
                    }
                }
              #else
                video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
              #endif
                if (video_value > 0)
                {
                    if(video_value_max < video_value)
                        video_value_max = video_value;

                    //Start asfTimeStatistics at create file (manual/trigger mode)
                    if(!pVideoClipOption->Start_asfTimeStatistics)
                    {
                        /*** TODO
                        calculat pre-record part VideoTimeStatistics
                        if(Cal_FileTime_Start_Idx == VideoBufMngReadIdx)
                        {
                        }
                        ***/
                        pVideoClipOption->Start_asfTimeStatistics   = 1;
                        pVideoClipOption->asfTimeStatistics         = 0;
                        pVideoClipOption->LocalTimeInSec            = g_LocalTimeInSec;
                    }
                #if 0
                    if(dcfWrite(pVideoClipOption->pFile, (u8*)pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size, &size) == 0) {
                        DEBUG_ASF("Ch%d write video frame data error!!!\n", pVideoClipOption->VideoChannelID);
        	    		dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        	    	    return 0;
        			}
                #else					
                    if (MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],0) == 0)
                    {
                        DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
                         /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                    #if 0
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                    #else   // 修正index不見的問題
                        // write index object //
                        if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                            //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }

                        /* close file */
                        if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                            DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                            //pVideoClipOption->OpenFile  = 0;
                            //return 0;
                        }
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                        pVideoClipOption->OpenFile  = 0;
                    #endif
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }
					
					#if AFN_DEBUG
					#if 0//(AFN_DEBUG_MORE_CHECK>1)
						if(pVideoClipOption->VideoChannelID<MULTI_CHANNEL_LOCAL_MAX)
						{
							if(MultiMP4_ErrCnt[pVideoClipOption->VideoChannelID]>MULCH_MP4_ERRCHK_THRES)
							{
							#if( HW_BOARD_OPTION == MR6730_AFN )							
								DEBUG_ASF("Ch%d Video too much error(%d), ", pVideoClipOption->VideoChannelID, MultiMP4_ErrCnt[pVideoClipOption->VideoChannelID]);
								#if 1
								//stop recording only
									DEBUG_ASF("terminate recording.\n");
									//terminate recording if detect audio stop
									pVideoClipOption->sysCaptureVideoStart	= 0;
									pVideoClipOption->sysCaptureVideoStop	= 1;
						
								#endif
							#endif
							}
						}
					#endif		
					#endif //#if AFN_DEBUG	
					
                #endif					
                    pVideoClipOption->VideoBufMngReadIdx = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                    //pVideoClipOption->asfVopCount++;
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                    //DEBUG_ASF(" %d ", pVideoClipOption->VideoChannelID);
                }
                #if FORCE_FPS
                if(((pVideoClipOption->asfVidePresentTime - PREROLL) * FORCE_FPS) > ((pVideoClipOption->asfVopCount + pVideoClipOption->asfDummyVopCount) * 1000))
                {
                    if (MultiChannelAsfWriteDummyVidePayload(pVideoClipOption,1) == 0)
                    {
                        DEBUG_ASF("Ch%d ASF write video dummy payload error!!!\n", pVideoClipOption->VideoChannelID);
                         /* write header object post */
                        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }

                        /* write data object post */
                        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                            pVideoClipOption->OpenFile  = 0;
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                        pVideoClipOption->OpenFile  = 0;
                        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }
                }
                #endif

            #ifdef  ASF_AUDIO
            }
            // Skip siu frames for release bandwidth to SD card writing
            monitor_value   = (video_value > audio_value) ? video_value : audio_value;
            #else
            monitor_value   = video_value;
            #endif

            if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {

                #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                /**********************************
                **** Check File Size           ****
                **********************************/
                if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                {
                    if(pVideoClipOption->asfDataPacketCount > MaxPacketCount)
                    {
                        DEBUG_ASF("\n\n\nCh%d File Size reach limit\n\n\n", pVideoClipOption->VideoChannelID);
                        if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) 
                        {
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                        /*** Reset Audio/Video time biase ***/
                        pVideoClipOption->ResetPayloadPresentTime   = 0; //reset video time base
                        pVideoClipOption->AV_TimeBase               = PREROLL;
                        //DEBUG_ASF("MPEG4 UseSem :%04d, IIS UseSem :%04d\n", VideoCmpSemEvt->OSEventCnt,iisCmpSemEvt->OSEventCnt);
                        //DEBUG_ASF("=====================================\n");
                        if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(0, pVideoClipOption)) == 0)
                        {
                            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            {
                                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                            }
                            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                            {
                                // 偵測到卡壞掉就全部關閉錄影
                                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                                {
                                    if(MultiChannelGetCaptureVideoStatus(i))
                                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                                }
                                if(MemoryFullFlag == TRUE)
                                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                                else
                                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                            }
                            return 0;
                        }
                    }
                }
                #endif
            }
        }   //if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))

        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
            if(video_value < 3)
                OSTimeDly(1);  //Lucian: release resource to low piority task.
        } else {    // 確保RF錄影時第一張為I frame
            if(video_value < 5)
                OSTimeDly(1);  //Lucian: release resource to low piority task.
        }
      #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
        if(Record_flag[pVideoClipOption->RFUnit] == 0)
        {
        for(i = 0; i < 10; i++) // 確保錄影時第一張為I frame, 但若video buffer空掉會無法換檔.
        {
            if(pVideoClipOption->VideoCmpSemEvt->OSEventCnt == 0)
                OSTimeDly(1);
        }
        }
      #else
        for(i = 0; i < 10; i++) // 確保錄影時第一張為I frame, 但若video buffer空掉會無法換檔.
        {
            if(pVideoClipOption->VideoCmpSemEvt->OSEventCnt == 0)
                OSTimeDly(1);
        }
      #endif
        //------------------- Bitstream buffer control---------------------------------//
        /*
             Lucian: 以Audio/Video bitstream buffer 內的index剩餘個數為偵測點,若大於 ASF_DROP_FRAME_THRESHOLD
                     則為SD 寫入速度過慢,需drop frame.

        */
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) //Event trigger mode
        {
            if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT &&
              #if (HW_BOARD_OPTION==MR6730_AFN)
            	(pVideoClipOption->CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE * 4) ||
              #else
            	(pVideoClipOption->CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE) ||
              #endif	            	
                (pVideoClipOption->CurrentVideoTime > (PreRecordTime * 1000)) || 
                pVideoClipOption->VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 60) ||
                pVideoClipOption->iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 16) ||
                (pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_DUMMY_ENA))) {
              #if ASF_DEBUG_ENA
                printf("************************ Bitstream control ************************\n");
                skip_I=0;
                skip_P=0;
                skip_A=0;
                printf("<BBB>1. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                printf("<BBB>1. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                printf("<BBB>1. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                printf("<BBB>1. pVideoClipOption->CurrentVideoSize           = %d\n", pVideoClipOption->CurrentVideoSize);	
                printf("<BBB>1. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                printf("<BBB>1. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                printf("<BBB>1. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
              #endif
                do {
                    video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
                  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    if(Record_flag[pVideoClipOption->RFUnit] == 1)
                    {
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Record_flag[pVideoClipOption->RFUnit]   = 0;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                        DEBUG_ASF("VideoBufMngReadIdx %d\n ", pVideoClipOption->VideoBufMngReadIdx);
                        break;
                    }
                  #endif
                    //DEBUG_ASF(" v%d=%d ", pVideoClipOption->VideoChannelID, video_value);
                    if (video_value > 0) {
                        if(video_value_max < video_value)
                            video_value_max = video_value;
                      #if ASF_DEBUG_ENA
                        if(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP)
                        	skip_I++;
                        else
                        	skip_P++;
                      #endif
                        MultiChannelAsfWriteVirtualVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx]);
                        pVideoClipOption->VideoBufMngReadIdx  = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                            OSSemPost(pVideoClipOption->VideoTrgSemEvt);
                    } else {
                        break;
                    }
                } while(pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag != FLAG_I_VOP);
              #if ASF_DEBUG_ENA
                printf("<BBB>2. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                printf("<BBB>2. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                printf("<BBB>2. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                printf("<BBB>2. pVideoClipOption->CurrentVideoSize           = %d\n", pVideoClipOption->CurrentVideoSize);	
                printf("<BBB>2. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                printf("<BBB>2. skip <%d, %d>\n"                                    , skip_I, skip_P);	
                printf("<BBB>2. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                printf("<BBB>2. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);
              #endif
                #if CDVR_LOG
                pVideoClipOption->LogFileStart    = (pVideoClipOption->LogFileStart + 1) % LOG_INDEX_NUM;
                #endif

                #ifdef  ASF_AUDIO
              #if INSERT_NOSIGNAL_FRAME
                while ((pVideoClipOption->sysAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->sysVidePresentTime)
              #else
                while ((pVideoClipOption->asfAudiPresentTime + IIS_CHUNK_TIME) <= pVideoClipOption->asfVidePresentTime)
              #endif
                {
                    audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
                  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                    if(Record_flag[pVideoClipOption->RFUnit] == 1)
                    {
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Record_flag[pVideoClipOption->RFUnit]   = 0;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                        DEBUG_ASF("iisSounBufMngReadIdx %d\n ", pVideoClipOption->iisSounBufMngReadIdx);
                        break;
                    }
                  #endif
//                    DEBUG_ASF(" A%d=%d ", pVideoClipOption->VideoChannelID, video_value);
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;
                      #if ASF_DEBUG_ENA
                        skip_A++;
                      #endif
                      #if AUDIO_DEBUG_ENA
                        ASFWRV4iiscnt = *pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer;
                        ASFWRV4iiscnt = ASFWRV4iiscnt<<8;

                        ASFWRV4iiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+1);
                        ASFWRV4iiscnt = ASFWRV4iiscnt<<8;

                        ASFWRV4iiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+2);
                        ASFWRV4iiscnt = ASFWRV4iiscnt<<8;

                        ASFWRV4iiscnt += *(pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx].buffer+3);
                        DEBUG_ASF("#2 ASFiiscnt %d,ASFWRV4iiscnt %d,iisidx %d,address 0x%08x\n",ASFiiscnt,ASFWRV4iiscnt,pVideoClipOption->iisSounBufMngReadIdx, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx]);
                      #endif
                        MultiChannelAsfWriteVirtualAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx]);
                      #if AUDIO_DEBUG_ENA
                        ASFiiscnt++;
                      #endif
                        pVideoClipOption->iisSounBufMngReadIdx    = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        
                        OSSemPost(pVideoClipOption->iisTrgSemEvt);
                    } else {
                        break;
                    }
                }
                #endif
              #if ASF_DEBUG_ENA
                printf("<BBB>3. asfVidePresentTime=%d, asfAudiPresentTime    = %d\n", pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);						
                printf("<BBB>3. pVideoClipOption->iisCmpSemEvt->OSEventCnt   = %d\n", pVideoClipOption->iisCmpSemEvt->OSEventCnt);								
                printf("<BBB>3. pVideoClipOption->VideoCmpSemEvt->OSEventCnt = %d\n", pVideoClipOption->VideoCmpSemEvt->OSEventCnt);								
                printf("<BBB>3. pVideoClipOption->CurrentVideoSize           = %d\n", pVideoClipOption->CurrentVideoSize);	
                printf("<BBB>3. pVideoClipOption->CurrentVideoTime           = %d\n", pVideoClipOption->CurrentVideoTime);						
                printf("<BBB>3. skip <%d>\n"                                    , skip_A);	
                printf("<BBB>3. ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
                printf("<BBB>3. RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);				
                printf("************************ Bitstream sync end ************************\n");
              #endif
            }
            else if(pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT)
            {
                OSTimeDly(1);  //Lucian: release resource to low piority task.
            }
        }
#if 0   // 取消由 packer 控制 image sensor frame rate
        else // Normal mode or overwrite mode
        {   // asfCaptureMode != ASF_CAPTURE_EVENT
        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )

            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {
                siuSkipFrameRate    = 0;
            }
            else
            {
               //DEBUG_ASF("z",monitor_value);
               DEBUG_ASF("z");
            }
        #else
            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {       // not skip siu frame
                if(siuSkipFrameRate != 0)
                {
                    siuSkipFrameRate    = 0;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 10)) {
                if(siuSkipFrameRate != 2) {
                    siuSkipFrameRate    = 2;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 20)) {
                if(siuSkipFrameRate != 4) {
                    siuSkipFrameRate    = 4;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 30)) {
                if(siuSkipFrameRate != 6) {
                    siuSkipFrameRate    = 6;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 40)) {
                if(siuSkipFrameRate != 8) {
                    siuSkipFrameRate    = 8;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 50)) {
                if(siuSkipFrameRate != 10) {
                    siuSkipFrameRate    = 10;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 60)) {
                if(siuSkipFrameRate != 12) {
                    siuSkipFrameRate    = 12;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 70)) {
                if(siuSkipFrameRate != 16) {
                    siuSkipFrameRate    = 16;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 80)) {
                if(siuSkipFrameRate != 20) {
                    siuSkipFrameRate    = 20;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 90)) {
                if(siuSkipFrameRate != 24) {
                    siuSkipFrameRate    = 24;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 100)) {
                if(siuSkipFrameRate != 28) {
                    siuSkipFrameRate    = 28;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else {
                if(siuSkipFrameRate != 32) {
                    siuSkipFrameRate    = 32;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            }
        #endif

        }   // asfCaptureMode != ASF_CAPTURE_EVENT
#endif

        /********************************************************
        *** asf index table detection / SD capacity detection ***
        ********************************************************/
        if( (pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TIMEUP) || (!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            //-------------Detect ASF index table: 因 ASF index table 是暫存於DRAM有容量限制; 錄影結束後才寫入SD card. ------//
            #ifdef  ASF_AUDIO
            if((pVideoClipOption->asfIndexTableIndex + VIDEO_BUF_NUM + IIS_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #else
            if((pVideoClipOption->asfIndexTableIndex + VIDEO_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #endif
    		{
                DEBUG_ASF("Ch%d asfIndexTableIndex =  %d, index buffer limit to %d, finish!!!\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfIndexTableIndex, ASF_IDX_SIMPLE_INDEX_ENTRY_MAX);
                pVideoClipOption->sysCaptureVideoStart    = 0;
                pVideoClipOption->sysCaptureVideoStop     = 1;
                break;
            }

            //-------------Detect Disk Full---------------------//
            // for disk full control
            if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA))
            {
                u32 TotalSize, i;
                pVideoClipOption->asfDataSize   = sizeof(ASF_DATA_OBJECT) +
                                                  pVideoClipOption->asfDataPacketCount * ASF_DATA_PACKET_SIZE;
                pVideoClipOption->asfIndexSize  = sizeof(ASF_SIMPLE_INDEX_OBJECT) +
                                                  pVideoClipOption->asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY);
                CurrentFileSize                 = pVideoClipOption->asfHeaderSize +
                                                  pVideoClipOption->asfDataSize +
                                                  pVideoClipOption->asfIndexSize;
                // for disk full control
                TotalSize                       = 0;
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if((VideoClipOption[i].PackerTaskCreated) && (VideoClipOption[i].OpenFile))
                    {
                    	#if 0//Lsk: FIXME may CurrentAudioSize<0;  
                        TotalSize  += (VideoClipOption[i].asfDataSize +
                                      VideoClipOption[i].asfIndexSize +
                                      VideoClipOption[i].asfHeaderSize +
                                      VideoClipOption[i].CurrentVideoSize +
                     	                 VideoClipOption[i].CurrentAudioSize) / 1024;
						#else
                        TotalSize  += (VideoClipOption[i].asfDataSize +
                                      VideoClipOption[i].asfIndexSize +
                                      VideoClipOption[i].asfHeaderSize +
                                      VideoClipOption[i].CurrentVideoSize
                                      ) / 1024;
						#endif

                    }
                }

                //if((((CurrentFileSize + pVideoClipOption->CurrentVideoSize + pVideoClipOption->CurrentAudioSize) / 1024) >= (free_size - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) / 1024)) )
                if((dcfGetMainStorageFreeSize() < (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX / 1024) ||
                    (TotalSize >= (dcfGetMainStorageFreeSize() - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX / 1024)))
                {
                    DEBUG_ASF("Ch%d Disk full!!!\n", pVideoClipOption->VideoChannelID);
                    DEBUG_ASF("free_size        = %d KBytes, CurrentFileSize = %d bytes.\n", dcfGetMainStorageFreeSize(), CurrentFileSize);
                    DEBUG_ASF("TotalSize        = %d KBytes.\n", TotalSize);
                    DEBUG_ASF("asfHeaderSize    = %d bytes.\n", pVideoClipOption->asfHeaderSize);
                    DEBUG_ASF("asfDataSize      = %d bytes.\n", pVideoClipOption->asfDataSize);
                    DEBUG_ASF("asfIndexSize     = %d bytes.\n", pVideoClipOption->asfIndexSize);
                    DEBUG_ASF("CurrentVideoSize = %d bytes.\n", pVideoClipOption->CurrentVideoSize);
                    DEBUG_ASF("CurrentAudioSize = %d bytes.\n", pVideoClipOption->CurrentAudioSize);
					for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                	{
                		{	
	                    	DEBUG_ASF("VideoClipOption[%d] OpenFile, PackerTaskCreated = %d, %d\n", i, VideoClipOption[i].OpenFile, VideoClipOption[i].PackerTaskCreated); 
							DEBUG_ASF("VideoClipOption[%d].asfHeaderSize    = %d bytes.\n", i, VideoClipOption[i].asfHeaderSize);
		                    DEBUG_ASF("VideoClipOption[%d].asfDataSize      = %d bytes.\n", i, VideoClipOption[i].asfDataSize);
		                    DEBUG_ASF("VideoClipOption[%d].asfIndexSize     = %d bytes.\n", i, VideoClipOption[i].asfIndexSize);
		                    DEBUG_ASF("VideoClipOption[%d].CurrentVideoSize = %d bytes.\n", i, VideoClipOption[i].CurrentVideoSize);
		                    DEBUG_ASF("VideoClipOption[%d].CurrentAudioSize = %d bytes.\n", i, VideoClipOption[i].CurrentAudioSize);
	                    }	                    
	                }
                    pVideoClipOption->sysCaptureVideoStart  = 0;
                    pVideoClipOption->sysCaptureVideoStop   = 1;
                    MemoryFullFlag                          = TRUE;
                    //Warning_SDFull();
                    uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                    // 偵測到卡滿就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    break;
                }
            }
        }

        /************************************
        *** Change file by size or slice ***
        ************************************/
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
          #if (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfDataPacketCount  > MaxPacketCount) && ((pVideoClipOption->AV_Source == LOCAL_RECORD) || pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
            if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfDataPacketCount  > MaxPacketCount) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
          #elif (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && ((pVideoClipOption->AV_Source == LOCAL_RECORD) || pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
            //if((pVideoClipOption->WantChangeFile == 0) && (pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
            //if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 5))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].flag == FLAG_I_VOP))
           #if INSERT_NOSIGNAL_FRAME
            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec) && ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 20)))) && ((pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP) || (gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)))
//            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) && ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
           #else
            if((pVideoClipOption->WantChangeFile == 0) && ((pVideoClipOption->asfTimeStatistics >= asfSectionTime * 1000) || ((g_LocalTimeInSec >= pVideoClipOption->LocalTimeInSec) && ((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (asfSectionTime + 5)))) && (pVideoClipOption->VideoCmpSemEvt->OSEventCnt && pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].asfflag == FLAG_I_VOP))
           #endif
          #endif
            {
                printf("asfTimeStatistics %d, LocalTimeInSec %d, g_LocalTimeInSec %d\n",pVideoClipOption->asfTimeStatistics,pVideoClipOption->LocalTimeInSec,g_LocalTimeInSec);
                if(asfRecFileNum != 0)
                {
                    pVideoClipOption->WantChangeFile            = 1;
                    DEBUG_ASF("Ch%d Time's up!!!\n", pVideoClipOption->VideoChannelID);
                    DEBUG_ASF("RTCseconds == %d\n", pVideoClipOption->RTCseconds);
                    DEBUG_ASF("asfRecFileNum  = %d\n", asfRecFileNum);
                    #if INSERT_NOSIGNAL_FRAME
                    OS_ENTER_CRITICAL();
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                    {
                      #if(NOSIGNAL_MODE == 3)
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                        pVideoClipOption->sysCaptureVideoStart	= 0;
                        pVideoClipOption->sysCaptureVideoStop	= 1;
                        Motion_Error_ststus[pVideoClipOption->RFUnit] = 1;
                        printf("RFIU_RX_STA_LINK_BROKEN\n\n\n");
                      #else
                        Record_flag[pVideoClipOption->RFUnit] = 1;
                      #endif
                    }
                    OS_EXIT_CRITICAL();
                    #endif
                    if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) 
                    {
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }
                  #if (HW_BOARD_OPTION == MR6730_AFN)//sss2
                   #if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	// multichannel
                    u8 TimeChg=1;		
                    u8 RecingChNum=0;

                    RecingChNum=(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_RECON))+(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON));
                    if(RecingChNum)
                    {//if there are more than one channel recording, waiting for its buddy ready then to go on
                        ASSERT_OP (pVideoClipOption->VideoChannelID, > , 0);
                        ASSERT_OP (pVideoClipOption->VideoChannelID, <= , 2);
                        //without REC_SYNC
                        //...
                      #if (USE_REC_TERM_MON)	
                        if(MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECTERM|UI_SET_STATUS_BITMSK_CH2_RECTERM)))			
                        {//exit if detect recording terminated
                            pVideoClipOption->sysCaptureVideoStart	= 0;
                            pVideoClipOption->sysCaptureVideoStop	= 1;							
                            break;
                        }
                      #endif 
                    }//if(RecingChNum)
                   #endif //#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)
                  #endif //#if (HW_BOARD_OPTION == MR6730_AFN)	
                  #if INSERT_NOSIGNAL_FRAME
                    if(gRfiu_Op_Sta[pVideoClipOption->RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                        return 0;
                  #endif

                    /*** Reset Audio/Video time biase ***/
                    pVideoClipOption->ResetPayloadPresentTime   = 0; //reset video time base
                    pVideoClipOption->AV_TimeBase               = PREROLL;
                   #if INSERT_NOSIGNAL_FRAME
                    if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(1, pVideoClipOption)) == 0)
                   #else
                    if((pVideoClipOption->pFile = MultiChannelAsfCreateFile(0, pVideoClipOption)) == 0)
                   #endif
                    {
                        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        {
                            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                        }
                        else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        {
                            // 偵測到卡壞掉就全部關閉錄影
                            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                            {
                                if(MultiChannelGetCaptureVideoStatus(i))
                                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                            }
                            if(MemoryFullFlag == TRUE)
                                uiOsdDrawSDCardFULL(UI_OSD_DRAW);
                            else
                                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                        }
                        return 0;
                    }

                    OS_ENTER_CRITICAL();
                    pVideoClipOption->WantChangeFile    = 0;
                    pVideoClipOption->LastAudio         = 0;
                    pVideoClipOption->LastVideo         = 0;
                    pVideoClipOption->GetLastAudio      = 0;
                    pVideoClipOption->GetLastVideo      = 0;
                    OS_EXIT_CRITICAL();
                    CurrentFileSize                     = 0;
                }
                else
                {
                    pVideoClipOption->sysCaptureVideoStart  = 0;
                    pVideoClipOption->sysCaptureVideoStop   = 1;
                    DEBUG_ASF("Ch%d asfRecFileNum  = %d\n", pVideoClipOption->VideoChannelID, asfRecFileNum);
                    break;
                }
            }
        }
        //-------------Check Power-off: 偵測到Power-off,須結束錄影 ------------------------//
        if(pwroff == 1) {   //prepare for power off
            pVideoClipOption->sysCaptureVideoStart  = 0;
            pVideoClipOption->sysCaptureVideoStop   = 1;
            break;
        }

        //------- Indicator of REC (LED ON/OFF): 以LED 閃爍提示---------------//
        //timetick =  IndicateRecordStatus(timetick);

    }

        
    DEBUG_ASF("Ch%d exit video capture while loop...\n", pVideoClipOption->VideoChannelID);

    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
    #ifdef  ASF_AUDIO
        while(pVideoClipOption->iisTrgSemEvt->OSEventCnt > 0) {
            OSSemAccept(pVideoClipOption->iisTrgSemEvt);
        }
    #endif

        while(pVideoClipOption->VideoTrgSemEvt->OSEventCnt > 0) {
            OSSemAccept(pVideoClipOption->VideoTrgSemEvt);
        }
        OSTimeDly(6);
    }

    if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && ((pVideoClipOption->EventTrigger == CAPTURE_STATE_WAIT) || (pVideoClipOption->EventTrigger == CAPTURE_STATE_TEMP)))
	{
	    #if(TRIGGER_MODE_CLOSE_FILE_METHOD==CLOSE_FILE_BY_SIZE)
        if(MultiChannelAsfCloseFile(pVideoClipOption) == 0) 
        {
            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
            {
                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
            }
            else if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
            return 0;
        }
        #endif
        DEBUG_ASF("Ch%d Event mode finish!!\n", pVideoClipOption->VideoChannelID);
        DEBUG_ASF("Ch%d video_value_max = %d\n", pVideoClipOption->VideoChannelID, video_value_max);
#ifdef  ASF_AUDIO
        DEBUG_ASF("Ch%d audio_value_max = %d\n", pVideoClipOption->VideoChannelID, audio_value_max);
#endif
        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
            MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        }
      #if INSERT_NOSIGNAL_FRAME
        OS_ENTER_CRITICAL();
        Record_flag[pVideoClipOption->VideoChannelID] = 0;
        OS_EXIT_CRITICAL();
      #endif
        return 1;
    }

    DEBUG_ASF("Ch%d write remaining audio data <%d, %d>...\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoCmpSemEvt->OSEventCnt, pVideoClipOption->iisCmpSemEvt->OSEventCnt);

#ifdef  ASF_AUDIO
    // write redundance audio payload data
    audio_index = 0;
  #if INSERT_NOSIGNAL_FRAME
    while((pVideoClipOption->iisCmpSemEvt->OSEventCnt > 0) && (audio_index < 20)) 
  #else
    while(pVideoClipOption->iisCmpSemEvt->OSEventCnt > 0) 
  #endif
    {
        Output_Sem();
        audio_value = OSSemAccept(pVideoClipOption->iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0) 
        {
            audio_index++;
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
        #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
            if (MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],0) == 0)
        #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            if (MultiChannelAsfWriteAudiPayload_IMA_ADPCM(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx], &PcmOffset) == 0)
        #endif
            {
                DEBUG_ASF("ASF write audio payload error!!!\n");
                /* write header object post */
                if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("ASF write file properties object post error!!!\n");
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                    return 0;
                }

                /* write data object post */
                if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("ASF write data object post error!!!\n");
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                    return 0;
                }
            #if 0
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
            #else   // 修正index不見的問題
                // write index object //
                if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                    //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }

                /* close file */
                if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                    DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                pVideoClipOption->OpenFile  = 0;
            #endif
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
                else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
                return 0;
            }
            pVideoClipOption->iisSounBufMngReadIdx = (pVideoClipOption->iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            #if AUDIO_DEBUG_ENA
            ASFiiscnt++;
            #endif
            //OSSemPost(iisTrgSemEvt);
        }
    }
#endif

    DEBUG_ASF("Ch%d write remaining video data <%d, %d>...\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoCmpSemEvt->OSEventCnt, pVideoClipOption->iisCmpSemEvt->OSEventCnt);

    video_index = 0;
  #if INSERT_NOSIGNAL_FRAME
    while((pVideoClipOption->VideoCmpSemEvt->OSEventCnt > 0) && (video_index < 10)) 
  #else
    while(pVideoClipOption->VideoCmpSemEvt->OSEventCnt > 0) 
  #endif
    {
        video_value = OSSemAccept(pVideoClipOption->VideoCmpSemEvt);
        if (video_value > 0) 
        {
            video_index++;
            if(video_value_max < video_value)
                video_value_max = video_value;
        #if 0
            if(dcfWrite(pVideoClipOption->pFile, (u8*)pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer, pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].size, &size) == 0) {
                DEBUG_AVI("Ch%d ASF write video chunk error!!!\n", pVideoClipOption->VideoChannelID);
	    		dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
	    	    return 0;
			}
        #else
            if (MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],0) == 0)
            {
                DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
                /* write header object post */
                if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                    return 0;
                }

                /* write data object post */
                if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                    else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                    return 0;
                }
            #if 0
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
            #else   // 修正index不見的問題
                // write index object //
                if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                    //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }

                /* close file */
                if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                    DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                pVideoClipOption->OpenFile  = 0;
            #endif
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
                else if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
                return 0;
            }
        #endif
            pVideoClipOption->VideoBufMngReadIdx = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            //pVideoClipOption->asfVopCount++;
            //OSSemPost(pVideoClipOption->VideoTrgSemEvt);
            //DEBUG_ASF(" %d ", pVideoClipOption->VideoChannelID);
        }
    }

    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
    {
        MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
    #ifdef  ASF_AUDIO
        //MultiChannelIIsStopRec(pVideoClipOption);
        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
    #endif
    }

	DEBUG_ASF("MultiChannelAsfCloseFile\n");
    MultiChannelAsfCloseFile(pVideoClipOption);
    DEBUG_ASF("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
    DEBUG_ASF("audio_value_max = %d\n", audio_value_max);
#endif

    return 1;
}
#endif
/*

Routine Description:

    Multiple channel close ASF file.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelAsfCloseFile(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32     CurrentFileSize;
    s32     DummySize;
    u32     size,i;
	u8      frame_num = 0;
  #if INSERT_NOSIGNAL_FRAME	
    u8      a_cnt = 0;
    u16     repeat_frame_cnt = 0;
    u32     audio_insert_cnt = 0, video_insert_cnt = 0, CurrentTime = 0;
  #endif
  #if (OS_CRITICAL_METHOD == 3)                 /* Allocate storage for CPU status register           */
    unsigned int    cpu_sr = 0;                 /* Prevent compiler warning                           */
  #endif
    //u8              FileName[FS_DIRNAME_MAX];
    //u32             FileEntrySect;
  #if ASF_DEBUG_ENA
    DEBUG_ASF("<fff> 1.ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
    DEBUG_ASF("<fff> 1.RX time <%d, %d>\n", RX_time_A, RX_time_V);						
    DEBUG_ASF("<fff> 1.RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);		
    DEBUG_ASF("<fff> 1.RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);	
    DEBUG_ASF("<fff> 1.End <%d>,  asfVidePresentTime=%d, asfAudiPresentTime=%d\n", (pVideoClipOption->asfVidePresentTime > pVideoClipOption->asfAudiPresentTime), pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);
  #endif

  	if(pVideoClipOption->VideoBufMngReadIdx == 0)
        i = VIDEO_BUF_NUM;
    else
        i = pVideoClipOption->VideoBufMngReadIdx - 1;
    
    if(pVideoClipOption->VideoBufMng[i].flag == FLAG_I_VOP)
        frame_num = ((*(pVideoClipOption->VideoBufMng[i].buffer+6) & 0x78) >> 3);
    else
        frame_num = ((*(pVideoClipOption->VideoBufMng[i].buffer+5) & 0x1) << 3) + ((*(pVideoClipOption->VideoBufMng[i].buffer+6) & 0xE0) >> 5);

    //printf("%d %x %x %d\n",frame_num,*(pVideoClipOption->VideoBufMng[i].buffer+5),*(pVideoClipOption->VideoBufMng[i].buffer+6),(*(pVideoClipOption->VideoBufMng[i].buffer+6) & 0xE0));
  #if INSERT_NOSIGNAL_FRAME
    if(Record_flag[pVideoClipOption->RFUnit] == 1)
    {
        a_cnt = 0;
      #if (NOSIGNAL_MODE == 1)
        MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],1);
        MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],1);
        if (MultiChannelAsfWriteVidePayload(pVideoClipOption, &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx],1) == 0)
        {
            DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
             /* write header object post */
            if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                pVideoClipOption->OpenFile  = 0;
             #if(RECORD_SOURCE == LOCAL_RECORD)
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                #endif
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
             #elif(RECORD_SOURCE == RX_RECEIVE)
                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
             #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                return 0;
            }

            /* write data object post */
            if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                pVideoClipOption->OpenFile  = 0;
              #if(RECORD_SOURCE == LOCAL_RECORD)
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
                #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                #endif
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
              #elif(RECORD_SOURCE == RX_RECEIVE)
                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
              #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                return 0;
            }
        #if 0
            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
            DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
        #else   // 修正index不見的問題
            // write index object //
            if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                //pVideoClipOption->OpenFile  = 0;
                //return 0;
            }

            /* close file */
            if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                //pVideoClipOption->OpenFile  = 0;
                //return 0;
            }
            DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
            pVideoClipOption->OpenFile  = 0;
        #endif
          #if(RECORD_SOURCE == LOCAL_RECORD)
            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
            {
            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                MultiChannelMPEG4EncoderTaskDestroy(pVideoClipOption);
            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
            #endif
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
            }
          #elif(RECORD_SOURCE == RX_RECEIVE)
            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
            return 0;
        }      
      #elif ((NOSIGNAL_MODE == 2) || (NOSIGNAL_MODE == 3))
        CurrentTime = pVideoClipOption->asfTimeStatistics / 1000;
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA)
        {
            if((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) >= (pVideoClipOption->asfRecTimeLenTotal + 5))    
                repeat_frame_cnt = pVideoClipOption->asfRecTimeLenTotal - CurrentTime;
            else
                repeat_frame_cnt = (g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) - CurrentTime;
        }
        else
        {
            if((g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) > asfSectionTime)
                repeat_frame_cnt = asfSectionTime - CurrentTime;
            else
                repeat_frame_cnt = (g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec) - CurrentTime;
        }
        //printf("%d %d %d\n",repeat_frame_cnt,(g_LocalTimeInSec - pVideoClipOption->LocalTimeInSec),(pVideoClipOption->asfTimeStatistics / 1000));            

        for(i = 0; i < repeat_frame_cnt; i++)
        {
        	frame_num = (frame_num+1) % 0x10;
            if (MultiChannelAsfWriteDummyVidePayload(pVideoClipOption, 1000,frame_num) == 0)
            {
                DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
                /* write header object post */
                if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                    return 0;
                }

                /* write data object post */
                if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
                    dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
                    pVideoClipOption->OpenFile  = 0;
                  #if(RECORD_SOURCE == LOCAL_RECORD)
                    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                    {
                        MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                        MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                        // 偵測到卡壞掉就全部關閉錄影
                        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                        {
                            if(MultiChannelGetCaptureVideoStatus(i))
                                sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                        }
                        uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    }
                  #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                    return 0;
                }
            #if 0
                dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
            #else   // 修正index不見的問題
                // write index object //
                if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
                    DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
                    //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }

                /* close file */
                if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
                    DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
                    //pVideoClipOption->OpenFile  = 0;
                    //return 0;
                }
                DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
                pVideoClipOption->OpenFile  = 0;
            #endif
              #if(RECORD_SOURCE == LOCAL_RECORD)
                if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                {
                    MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                    MultiChannelIISRecordTaskDestroy(pVideoClipOption);
                }
              #elif(RECORD_SOURCE == RX_RECEIVE)
                if(pVideoClipOption->AV_Source == RX_RECEIVE)
                {
                    // 偵測到卡壞掉就全部關閉錄影
                    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                    {
                        if(MultiChannelGetCaptureVideoStatus(i))
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                    }
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                }
              #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
                return 0;
            }        
        }
      #endif
        while(pVideoClipOption->asfAudiPresentTime < pVideoClipOption->asfVidePresentTime)
        {
            //a_cnt++;
            MultiChannelAsfWriteAudiPayload(pVideoClipOption, &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngReadIdx],1);
        }
        OS_ENTER_CRITICAL();
        Record_flag[pVideoClipOption->RFUnit] = 0;
        OS_EXIT_CRITICAL();
    }
  #endif
   	frame_num = (frame_num+1) % 0x10;
    if (MultiChannelAsfWriteDummyVidePayload(pVideoClipOption, DUMMY_FRAME_DURATION ,frame_num) == 0)//Lsk: avoid last frame disapper
    {
        DEBUG_ASF("Ch%d ASF write video payload error!!!\n", pVideoClipOption->VideoChannelID);
        /* write header object post */
        if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
            DEBUG_ASF("Ch%d ASF write file properties object post error!!!\n", pVideoClipOption->VideoChannelID);
            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
            pVideoClipOption->OpenFile  = 0;
          #if(RECORD_SOURCE == LOCAL_RECORD)
            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
            {
                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
            }
          #elif(RECORD_SOURCE == RX_RECEIVE)
            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
            return 0;
        }

        /* write data object post */
        if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
            DEBUG_ASF("Ch%d ASF write data object post error!!!\n", pVideoClipOption->VideoChannelID);
            dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
            pVideoClipOption->OpenFile  = 0;
          #if(RECORD_SOURCE == LOCAL_RECORD)
            if(pVideoClipOption->AV_Source == LOCAL_RECORD)
            {
                MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
                MultiChannelIISRecordTaskDestroy(pVideoClipOption);
            }
          #elif(RECORD_SOURCE == RX_RECEIVE)
            if(pVideoClipOption->AV_Source == RX_RECEIVE)
            {
                // 偵測到卡壞掉就全部關閉錄影
                for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if(MultiChannelGetCaptureVideoStatus(i))
                        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
                }
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
            }
          #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
            return 0;
        }
    #if 0
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
    #else   // 修正index不見的問題
        // write index object //
        if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
            DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
            //dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID);
            //pVideoClipOption->OpenFile  = 0;
            //return 0;
        }

        /* close file */
        if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
            DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
            //pVideoClipOption->OpenFile  = 0;
            //return 0;
        }
        DEBUG_ASF("Ch%d Leave dcfclose()!\n", pVideoClipOption->VideoChannelID);
        pVideoClipOption->OpenFile  = 0;
    #endif
       #if(RECORD_SOURCE == LOCAL_RECORD)
        if(pVideoClipOption->AV_Source == LOCAL_RECORD)
        {
            MultiChannelVideoEncoderTaskDestroy(pVideoClipOption);
            MultiChannelIISRecordTaskDestroy(pVideoClipOption);
        }
      #elif(RECORD_SOURCE == RX_RECEIVE)
        if(pVideoClipOption->AV_Source == RX_RECEIVE)
        {
            // 偵測到卡壞掉就全部關閉錄影
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                if(MultiChannelGetCaptureVideoStatus(i))
                    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, i);
            }
            uiOsdDrawSDCardFail(UI_OSD_DRAW);
        }
      #endif //end of if(RECORD_SOURCE == RX_RECEIVE)
        return 0;
    }    
    DEBUG_ASF("Ch%d close ASF file....\n", pVideoClipOption->VideoChannelID);

    if(pVideoClipOption->OpenFile == 0)
    {
        DEBUG_ASF("Ch%d file isn't exist, can't close it!!!\n", pVideoClipOption->VideoChannelID);
        return  1;
    }

  #if (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)
    uiOsdDrawSysAfterRec(pVideoClipOption);
  #endif

    /* write data packet post */
    //DEBUG_ASF("==1==\n");
    if (MultiChannelAsfWriteDataPacketPost(pVideoClipOption, pVideoClipOption->asfDataPacketLeftSize) == 0) {
        DEBUG_ASF("ASF write data packet post error!!!\n");
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }

    /* write header object post */
    //DEBUG_ASF("==2==\n");
    if (MultiChannelAsfWriteFilePropertiesObjectPost(pVideoClipOption) == 0) {
        DEBUG_ASF("ASF write file properties object post error!!!\n");
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }

    /* write data object post */
    //DEBUG_ASF("==3==\n");
    if (MultiChannelAsfWriteDataObjectPost(pVideoClipOption) == 0) {
        DEBUG_ASF("ASF write data object post error!!!\n");
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }
    // write index object //
    //DEBUG_ASF("==4==\n");
    if (MultiChannelAsfWriteIndexObject(pVideoClipOption) == 0) {
        DEBUG_ASF("Ch%d ASF write index object error!!!\n", pVideoClipOption->VideoChannelID);
        dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile));
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }

    //dcfGetCurFileName((s8*)FileName);
    //FileEntrySect   = pVideoClipOption->pFile->FileEntrySect;

    /* close file */
    if(dcfCloseFileByIdx(pVideoClipOption->pFile, pVideoClipOption->VideoChannelID, &(pVideoClipOption->OpenFile)) == 0) {
        DEBUG_ASF("Ch%d Close file error!!!\n", pVideoClipOption->VideoChannelID);
        pVideoClipOption->OpenFile  = 0;
        return 0;
    }
    pVideoClipOption->OpenFile  = 0;

    uiOsdDrawNewFile();
    //DEBUG_ASF("Ch%d FileName           = %s\n", pVideoClipOption->VideoChannelID, FileName);
    //DEBUG_ASF("Ch%d asfDataPacketCount = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfDataPacketCount);
    //DEBUG_ASF("Ch%d asfIndexTableIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfIndexTableIndex);
    //DEBUG_ASF("Ch%d asfIndexEntryTime  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfIndexEntryTime);
    //DEBUG_ASF("Ch%d asfVidePresentTime = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfVidePresentTime);
    //DEBUG_ASF("Ch%d asfAudiPresentTime = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfAudiPresentTime);
    //DEBUG_ASF("Ch%d asfVopCount        = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfVopCount);
#if FORCE_FPS
    //DEBUG_ASF("Ch%d asfDummyVopCount   = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfDummyVopCount);
#endif
    //DEBUG_ASF("Ch%d asfAudiChunkCount  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfAudiChunkCount);
    //DEBUG_ASF("finish\n");



    if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
        //Check filesystem capacity
        switch(dcfOverWriteOP)
        {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            case DCF_OVERWRITE_OP_OFF:
                break;
            case DCF_OVERWRITE_OP_01_DAYS:
            case DCF_OVERWRITE_OP_07_DAYS:
            case DCF_OVERWRITE_OP_30_DAYS:
            case DCF_OVERWRITE_OP_60_DAYS:
            	/*while(curr_free_space < DCF_OVERWRITE_THR_KBYTE)
                {
                    if(dcfOverWriteDel()==0)
                    {
                        DEBUG_DCF("Over Write delete fail!!\n");
                        return 0;
                    }
                    else
                    {
                        //DEBUG_ASF("Over Write delete Pass!!\n");
                    }
                    //due to only update global_diskInfo when clos file, so we must calculate when open file
                    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
                    curr_free_space = free_size - curr_record_space;
                }*/
                if(dcfOverWriteOP >= dcfGetTotalDirCount())
                    break;
                sysbackLowSetEvt(SYSBACKLOW_EVT_OVERWRITEDEL, dcfOverWriteOP, 0, 0, 0);
                break;
#endif
            default:
		        DEBUG_DCF("Free Space=%d (KBytes) \n", dcfGetMainStorageFreeSize());
		        while(dcfGetMainStorageFreeSize() < DCF_OVERWRITE_THR_KBYTE)
		        {   // Find the oldest file pointer and delete it
			        #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel start, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif
		            if(dcfOverWriteDel() == 0)
		            {
		                DEBUG_DCF("Over Write delete fail!!\n");
		                return 0;
		            }
		            else
		            {
		                //DEBUG_ASF("Over Write delete Pass!!\n");
		            }
					#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
					DEBUG_ASF("Lsk: CH%d dcfOverWriteDel end, %d !!!\n", pVideoClipOption->VideoChannelID, __LINE__);
					#endif

		            DEBUG_DCF("Free Space=%d (KBytes) \n", dcfGetMainStorageFreeSize());
		        }
		        break;
		}
    }
  #if ASF_DEBUG_ENA
    DEBUG_ASF("<fff> 2.ASF time <%d, %d>, %d\n", pVideoClipOption->sysAudiPresentTime, pVideoClipOption->sysVidePresentTime, pVideoClipOption->sysVidePresentTime-pVideoClipOption->sysAudiPresentTime);						
    DEBUG_ASF("<fff> 2.RX time <%d, %d>\n", RX_time_A, RX_time_V);						
    DEBUG_ASF("<fff> 2.RX skip <%d, %d>\n", RX_skip_A, RX_skip_V);		
    DEBUG_ASF("<fff> 2.RX, ASF sem = <%d, %d>, <%d, %d>, <%d, %d>\n", RX_sem_A, RX_sem_V, ASF_sem_A, ASF_sem_V, RX_sem_A-ASF_sem_A, RX_sem_V-ASF_sem_V);	
    DEBUG_ASF("<fff> 2.End <%d>,  asfVidePresentTime=%d, asfAudiPresentTime=%d\n", (pVideoClipOption->asfVidePresentTime > pVideoClipOption->asfAudiPresentTime), pVideoClipOption->asfVidePresentTime, pVideoClipOption->asfAudiPresentTime);
  #endif

    return 1;
}

/*

Routine Description:

    Multiple channel stop video capture.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    1: Success.
    2: Not in video capture mode
    0: Otherwise

*/
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) ) //Lsk: To debug pending stop record timeout issue
s32 MultiChannelAsfCaptureVideoStop(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    if(pVideoClipOption->sysCaptureVideoStart && !pVideoClipOption->sysCaptureVideoStop)
    {
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
        {
            if(!pVideoClipOption->EventTrigger && !pVideoClipOption->DoCaptureVideo)  //Lsk: ‥SA2μo?y?v!Aa?±μ°±?i
            {
                pVideoClipOption->sysCaptureVideoStart    = 0;
                pVideoClipOption->sysCaptureVideoStop     = 1;
                DEBUG_ASF("MultiChannelAsfCaptureVideoStop(%d) success!!!\n", pVideoClipOption->VideoChannelID);
                DEBUG_ASF("Ch%d EventTrigger = %d, DoCaptureVideo = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger, pVideoClipOption->DoCaptureVideo);
                return  1;
            }
			else
			{
                if(pVideoClipOption->DoCaptureVideo)
                {
                	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
                    if(pVideoClipOption->DoCaptureVideo == 1)
                    {
                		DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
                        pVideoClipOption->DoCaptureVideo  = 2;
                    }
					
                }
				else if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER) //Lsk: |3|bA2μo?y?v!A￥yTimeup
				{	
                	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
                  #if(RECORD_SOURCE == LOCAL_RECORD)
				    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
				    {
	                	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
                        pVideoClipOption->Last_VideoBufMngReadIdx   = pVideoClipOption->VideoBufMngWriteIdx;
				    }
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                    {
                    	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
                        pVideoClipOption->Last_VideoBufMngReadIdx   = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                    }
                  #endif
                    pVideoClipOption->WantToExitPreRecordMode   = 1;
                    pVideoClipOption->EventTrigger              = CAPTURE_STATE_TIMEUP;
					
                    return  1;
                }
                DEBUG_ASF("Ch%d EventTrigger = %d, DoCaptureVideo = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger, pVideoClipOption->DoCaptureVideo);
            }
        }
    #ifdef ASF_AUDIO
		else //if(pVideoClipOption->asfVopCount > 10 && pVideoClipOption->asfAudiChunkCount > 12) // A×§Kdead lock,?ERE￥y?￡§PA_.
		{
           	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
            pVideoClipOption->sysCaptureVideoStart    = 0;
            pVideoClipOption->sysCaptureVideoStop     = 1;
            DEBUG_ASF("Ch%d asfCaptureVideoStop() success!!!\n", pVideoClipOption->VideoChannelID);
            return  1;
        }
    #else
		else //if(pVideoClipOption->asfVopCount > 10)  // A×§Kdead lock,?ERE￥y?￡§PA_.
		{
           	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
            pVideoClipOption->sysCaptureVideoStart    = 0;
            pVideoClipOption->sysCaptureVideoStop     = 1;
            DEBUG_ASF("Ch%d asfCaptureVideoStop() success!!!\n", pVideoClipOption->VideoChannelID);
            return  1;
        }
    #endif
    }
    else if(!pVideoClipOption->sysCaptureVideoStart && pVideoClipOption->sysCaptureVideoStop)
    {
       	DEBUG_ASF("@@@ Ch%d <%d>\n", pVideoClipOption->VideoChannelID,__LINE__);
        DEBUG_ASF("Ch%d not in video capture mode!!!\n", pVideoClipOption->VideoChannelID);
        return  2;
    }
    DEBUG_ASF("MultiChannelAsfCaptureVideoStop(%d) fail!!!\n", pVideoClipOption->VideoChannelID);
    DEBUG_ASF("Ch%d sysCaptureVideoStart = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->sysCaptureVideoStart);
    DEBUG_ASF("Ch%d sysCaptureVideoStop  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->sysCaptureVideoStop);
    DEBUG_ASF("Ch%d asfCaptureMode       = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfCaptureMode);
    DEBUG_ASF("Ch%d EventTrigger         = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger);
    DEBUG_ASF("Ch%d asfVopCount          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfVopCount);
    DEBUG_ASF("Ch%d asfAudiChunkCount    = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfAudiChunkCount);
    return 0;
}
#else
s32 MultiChannelAsfCaptureVideoStop(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    if(pVideoClipOption->sysCaptureVideoStart && !pVideoClipOption->sysCaptureVideoStop)
    {
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
        {
            if(!pVideoClipOption->EventTrigger && !pVideoClipOption->DoCaptureVideo)
            {
                pVideoClipOption->sysCaptureVideoStart    = 0;
                pVideoClipOption->sysCaptureVideoStop     = 1;
                DEBUG_ASF("MultiChannelAsfCaptureVideoStop(%d) success!!!\n", pVideoClipOption->VideoChannelID);
                DEBUG_ASF("Ch%d EventTrigger = %d, DoCaptureVideo = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger, pVideoClipOption->DoCaptureVideo);
                return  1;
            }
			else
			{
                if(pVideoClipOption->DoCaptureVideo)
                {
                    if(pVideoClipOption->DoCaptureVideo == 1)
                        pVideoClipOption->DoCaptureVideo  = 2;
                }
				else if(pVideoClipOption->EventTrigger == CAPTURE_STATE_TRIGGER)
				{
                  #if(RECORD_SOURCE == LOCAL_RECORD)
				    if(pVideoClipOption->AV_Source == LOCAL_RECORD)
                        pVideoClipOption->Last_VideoBufMngReadIdx   = pVideoClipOption->VideoBufMngWriteIdx;
                  #elif(RECORD_SOURCE == RX_RECEIVE)
                    if(pVideoClipOption->AV_Source == RX_RECEIVE)
                        pVideoClipOption->Last_VideoBufMngReadIdx   = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
                  #endif
                    pVideoClipOption->WantToExitPreRecordMode   = 1;
                    pVideoClipOption->EventTrigger              = CAPTURE_STATE_TIMEUP;
                    return  1;
                }
                DEBUG_ASF("Ch%d EventTrigger = %d, DoCaptureVideo = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger, pVideoClipOption->DoCaptureVideo);
            }
        }
    #ifdef ASF_AUDIO
		else //if(pVideoClipOption->asfVopCount > 10 && pVideoClipOption->asfAudiChunkCount > 12) // 避免dead lock,暫時先不判斷.
		{
            pVideoClipOption->sysCaptureVideoStart    = 0;
            pVideoClipOption->sysCaptureVideoStop     = 1;
            DEBUG_ASF("Ch%d asfCaptureVideoStop() success!!!\n", pVideoClipOption->VideoChannelID);
            return  1;
        }
    #else
		else //if(pVideoClipOption->asfVopCount > 10)  // 避免dead lock,暫時先不判斷.
		{
            pVideoClipOption->sysCaptureVideoStart    = 0;
            pVideoClipOption->sysCaptureVideoStop     = 1;
            DEBUG_ASF("Ch%d asfCaptureVideoStop() success!!!\n", pVideoClipOption->VideoChannelID);
            return  1;
        }
    #endif
    }
    else if(!pVideoClipOption->sysCaptureVideoStart && pVideoClipOption->sysCaptureVideoStop)
    {
        DEBUG_ASF("Ch%d not in video capture mode!!!\n", pVideoClipOption->VideoChannelID);
        return  2;
    }
    DEBUG_ASF("MultiChannelAsfCaptureVideoStop(%d) fail!!!\n", pVideoClipOption->VideoChannelID);
    DEBUG_ASF("Ch%d sysCaptureVideoStart = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->sysCaptureVideoStart);
    DEBUG_ASF("Ch%d sysCaptureVideoStop  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->sysCaptureVideoStop);
    DEBUG_ASF("Ch%d asfCaptureMode       = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfCaptureMode);
    DEBUG_ASF("Ch%d EventTrigger         = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->EventTrigger);
    DEBUG_ASF("Ch%d asfVopCount          = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfVopCount);
    DEBUG_ASF("Ch%d asfAudiChunkCount    = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->asfAudiChunkCount);
    return 0;
}
#endif
/*

Routine Description:

    Multiple channel stop all video capture channels.

Arguments:

    None.

Return Value:

    None

*/
void MultiChannelAsfCaptureVideoStopAll(void)
{
    int     i, Ret;

    Ret     = 0;
    while(Ret != MULTI_CHANNEL_SEL)
    {
        for(i = 0; i < MULTI_CHANNEL_LOCAL_MAX; i++)
        {
            if((MULTI_CHANNEL_SEL & (1 << i)) && !(Ret & (1 << i)))
            {
                if(MultiChannelAsfCaptureVideoStop(&VideoClipOption[i]))
                    Ret    |= 1 << i;
                else
                    OSTimeDly(1);
            }
        }
    }
}

/*

Routine Description:

    Multiple channel stop all video capture channels.

Arguments:

    None.

Return Value:

    None

*/
void MultiChannelAsfLinkBrokenCloseFile(int ch)
{
    int i;
    #if (INSERT_NOSIGNAL_FRAME)
    if (gRfiu_Op_Sta[ch] != RFIU_RX_STA_LINK_OK)
    {
        isPIRsenSent[ch] = 0; //for next link up, sending PIR sensitivity from RX to TX       
    }
    #else
    // stop recording and closed file when link broken
    if (gRfiu_Op_Sta[ch] != RFIU_RX_STA_LINK_OK)
    {
        if (MultiChannelGetCaptureVideoStatus(ch))
        {
            DEBUG_ASF("[%s] Cam %d stop rec in link broken\n", __FUNCTION__, ch);
            uiCaptureVideoStopByChannel(ch);
        }
        isPIRsenSent[ch] = 0; //for next link up, sending PIR sensitivity from RX to TX
    }     
    #endif
}
/*
Routine Description:

    Multiple channel check video recording channel

Arguments:

    None.

Return Value:
    bitmap for recording channel. e.g. 0110b, ch1, ch2 is recording, ch0, ch3 is not recording
    

*/
u8 MultiChannelCheckRecordChannel()
{
    int i;
    u8 recording_channel = 0;
    for(i=0;i<MULTI_CHANNEL_MAX;i++)
         recording_channel |= VideoClipOption[i].OpenFile << i;
    return recording_channel;
}

#endif  // #if MULTI_CHANNEL_VIDEO_REC

