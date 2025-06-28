/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    ms_adpcm.c

Abstract:

    The routines of Microsoft ADPCM codec.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2011/02/14  Peter Hsu  Create  

*/

#include "general.h"

#if (AUDIO_CODEC == AUDIO_CODEC_MS_ADPCM)

#include "board.h"
#include "task.h"
#include "iis.h"
#include "iisreg.h"
#include "iisapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "mp4api.h"
#include "asfapi.h"
#include "sysapi.h"
#include "aviapi.h" 
#include "mpeg4api.h"
#include "sysapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "adcapi.h"
#include "osapi.h"
#include "timerapi.h"
#include "uiapi.h"

/*
 *********************************************************************************************************
 * Definition
 *********************************************************************************************************
 */

//#define MS_ADPCM_SAMPLE_PER_BLOCK   500
#define MS_ADPCM_SAMPLE_PER_BLOCK   IIS_CHUNK_SIZE
#define MS_ADPCM_BLOCK_SIZE         (7 + (MS_ADPCM_SAMPLE_PER_BLOCK - 2) / 2)
#define MS_ADPCM_BUF_SIZE           (MS_ADPCM_BLOCK_SIZE * 4)


typedef __packed struct _ADPCM_COEF_SET 
{ 
    short coef1; 
    short coef2; 
}ADPCM_COEF_SET; 

//typedef unsigned short  WORD; 
//typedef unsigned int    DWORD; 
//typedef unsigned char   BYTE; 

typedef __packed struct _RIFFWAVECHUNK 
{ 
    DWORD id; 
    DWORD size; 
    DWORD format; 
}RIFFWAVE_CHUNK; 

typedef __packed struct _WAVEFORMATEX 
{ 
    WORD    wFormatTag; 
    WORD    nChannels; 
    DWORD   nSamplesPerSec; 
    DWORD   nAvgBytesPerSec; 
    WORD    nBlockAlign; 
    WORD    wBitsPerSample; 
    WORD    cbSize; 
}WAVEFORMAT_EX; 

typedef __packed struct _WAVEFORMATCHUNK 
{ 
    DWORD   id; 
    DWORD   size; 
    WORD    wFormatTag; 
    WORD    nChannels; 
    DWORD   nSamplesPerSec; 
    DWORD   nAvgBytesPerSec; 
    WORD    nBlockAlign; 
    WORD    wBitsPerSample; 
}FORMAT_CHUNK; 

typedef __packed struct _MICROSOFT_ADPCM_FORMAT 
{ 
    DWORD           id; 
    DWORD           size; 
    WAVEFORMAT_EX   format; 
    WORD            wSamplesPerBlock; 
    WORD            wNumCoef; 
    ADPCM_COEF_SET  coeffs[7]; 
}MICROSOFT_ADPCM_FORMAT; 

typedef __packed struct _DATA_CHUNK 
{ 
    DWORD   id; 
    DWORD   size; 
    BYTE    data[1]; 
}DATA_CHUNK; 

typedef __packed struct 
{ 
    DWORD   id; 
    DWORD   size; 
    DWORD   nSamples; 
}FACT_CHUNK; 

typedef __packed struct{ 
    DWORD   id; 
    DWORD   size; 
}CHUNK; 

typedef __packed struct _WAV_MS_ADPCM_HEADER
{
    RIFFWAVE_CHUNK          rc; 
    MICROSOFT_ADPCM_FORMAT  fmtc; 
    FACT_CHUNK              fc; 
    CHUNK                   dc;
} WAV_MS_ADPCM_HEADER;


typedef __packed struct{ 
    BYTE    predictor; 
    short   iDelta; 
    short   iSample1; 
    short   iSample2; 
    BYTE    pNibbles[1]; 
}BLOCK; 

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

int AdaptationTable [] = { 
    230, 230, 230, 230, 307, 409, 512, 614, 
    768, 614, 512, 409, 307, 230, 230, 230 
}; 

ADPCM_COEF_SET CoeffTable[] = { 
    256, 0, 
    512, -256, 
    0,   0, 
    192, 64, 
    240, 0, 
    460, -208, 
    392, -232 
}; 

/*
 *********************************************************************************************************
 * Global variable
 *********************************************************************************************************
 */

int                                 curSize; 
u8                                  AdpcmBuff[MS_ADPCM_BUF_SIZE];
s16                                 PCM_Data_16bits[IIS_CHUNK_SIZE];
int                                 AdpcmUsedBuff;
__align(4)  WAV_MS_ADPCM_HEADER     WavMsAdpcmHeader;

/*
 *********************************************************************************************************
 * External variable
 *********************************************************************************************************
 */

extern u8           uiMenuEnable;
extern u32          iisBufferCmpCount;
extern WAVEFORMAT   iisPlayFormat;
extern WAVEFORMAT   iisRecFormat;
extern u32          IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
extern s64          IISTime;        // Current IIS playback time(micro second)
extern u32          IISTimeUnit;    // IIS playback time per DMA(micro second)
extern u32          iisPlayCount;   // IIS played chunk number
extern u32          iisTotalPlay;   // IIS total trigger playback number

/*
 *********************************************************************************************************
 * External function
 *********************************************************************************************************
 */

s32 debugDumpMemory(u32 len, u32 addr);

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

#if 0
void calculateCoefficient(ADPCM_COEF_SET *coeff, short* pdata, int len) 
{ 
    double alpha    = 0; 
    double beta     = 0; 
    double gama     = 0; 
    double m        = 0, n = 0; 
    
    int             i; 
    for( i = 2; i < len; ++i ) 
    { 
        alpha   += (double)pdata[i-1] * pdata[i-1]; 
        beta    += (double)pdata[i-1] * pdata[i-2]; 
        gama    += (double)pdata[i-2] * pdata[i-2]; 
        m       += (double)pdata[i]   * pdata[i-1]; 
        n       += (double)pdata[i]   * pdata[i-2]; 
    } 
    
    coeff->coef1 = (m * gama - n * beta ) * 256 / (alpha * gama - beta  * beta); 
    coeff->coef2 = (m * beta - n * alpha) * 256 / (beta  * beta - alpha * gama); 
} 

#else

void calculateCoefficient(ADPCM_COEF_SET *coeff, short* pdata, int len) 
{ 
    s64 alpha    = 0; 
    s64 beta     = 0; 
    s64 gama     = 0; 
    s64 m        = 0, n = 0; 
    s64 a, b, c, d;
    
    int             i; 
    for( i = 2; i < len; ++i ) 
    { 
        alpha   += (s64)((s32)pdata[i-1] * pdata[i-1]); 
        beta    += (s64)((s32)pdata[i-1] * pdata[i-2]); 
        gama    += (s64)((s32)pdata[i-2] * pdata[i-2]); 
        m       += (s64)((s32)pdata[i]   * pdata[i-1]); 
        n       += (s64)((s32)pdata[i]   * pdata[i-2]); 
    } 
    a   = alpha * gama;
    b   = beta  * beta;
    c   = a - b;
    d   = b - a;
    if(c == 0)
        c   = 1;
    if(d == 0)
        d   = 1;
    coeff->coef1 = (m * gama - n * beta ) * 256 / c; 
    coeff->coef2 = (m * beta - n * alpha) * 256 / d; 
} 

#endif

int getClosestCoefficientIndex(ADPCM_COEF_SET *coeff) 
{ 
    int         i;
    DWORD       diff    = (DWORD)-1; 
    int         index   = 0; 
    
    for( i = 0; i < sizeof(CoeffTable) / sizeof(CoeffTable[0]); ++i) 
    { 
        int dx = (CoeffTable[i].coef1 - coeff->coef1); 
        int dy = (CoeffTable[i].coef2 - coeff->coef2); 
        
        if( dx * dx + dy * dy < diff ) 
            diff = dx * dx + dy * dy, index = i; 
    } 
    return index; 
} 
BYTE trimToNibble(char c) 
{ 
    if( c > 7 ) 
        c = 7; 
    if( c < -8 ) 
        c = -8; 
    if( c < 0 ) 
        c = c & 0x0f; 
    return c; 
} 

/*

Routine Description:

    Write Microsoft ADPCM wave file header previous.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 Write_MS_ADPCM_Header_Pre(FS_FILE* pFile)
{
    //__align(4)  WAV_MS_ADPCM_HEADER     Header;
    u32                                 size;

    WavMsAdpcmHeader.rc.id                        = 0x46464952;    // "RIFF"
    WavMsAdpcmHeader.rc.size                      = 0;             // update this item after voice record finishd
    WavMsAdpcmHeader.rc.format                    = 0x45564157;    // "WAVE"

    WavMsAdpcmHeader.fmtc.id                      = 0x20746d66;    // "fmt "
    WavMsAdpcmHeader.fmtc.size                    = sizeof(WavMsAdpcmHeader.fmtc) - 8; 
    WavMsAdpcmHeader.fmtc.format.wFormatTag       = 0x02; 
    WavMsAdpcmHeader.fmtc.format.nChannels        = 1; 
    WavMsAdpcmHeader.fmtc.format.nSamplesPerSec   = 8000; 
    WavMsAdpcmHeader.fmtc.format.nAvgBytesPerSec  = 4096; 
    //WavMsAdpcmHeader.fmtc.format.nBlockAlign      = 256; 
    WavMsAdpcmHeader.fmtc.format.nBlockAlign      = MS_ADPCM_BLOCK_SIZE; 
    WavMsAdpcmHeader.fmtc.format.wBitsPerSample   = 4; 
    WavMsAdpcmHeader.fmtc.format.cbSize           = 32; 
    //WavMsAdpcmHeader.fmtc.wSamplesPerBlock        = 500; 
    WavMsAdpcmHeader.fmtc.wSamplesPerBlock        = MS_ADPCM_SAMPLE_PER_BLOCK; 
    WavMsAdpcmHeader.fmtc.wNumCoef                = 7; 
    memcpy( &WavMsAdpcmHeader.fmtc.coeffs[0], &CoeffTable[0], sizeof(CoeffTable) ); 
    
    WavMsAdpcmHeader.fc.id                        = 0x74636166;    // "fact"
    WavMsAdpcmHeader.fc.size                      = 4; 
    WavMsAdpcmHeader.fc.nSamples                  = 0;             // Total number of samples be coded, update this item after voice record finishd

    WavMsAdpcmHeader.dc.id                        = 0x61746164;    // "data"
    WavMsAdpcmHeader.dc.size                      = 0;             // update this item after voice record finishd
    
    if (dcfWrite(pFile, (unsigned char*)&WavMsAdpcmHeader, sizeof(WavMsAdpcmHeader), &size) == 0) {
        DEBUG_IIS("Wav Write Microsoft ADPCM header error!!!\n");
        return 0;
    }

    return 1;
}

/*

Routine Description:

    Write Microsoft ADPCM wave file header post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 Write_MS_ADPCM_Header_Post(FS_FILE* pFile)
{
    //__align(4)  WAV_MS_ADPCM_HEADER     Header;
    u32                                 size;
    u32                                 offset;

    WavMsAdpcmHeader.fc.nSamples          = iisBufferCmpCount;
    WavMsAdpcmHeader.dc.size              = curSize;
    WavMsAdpcmHeader.rc.size              = 4 + sizeof(WavMsAdpcmHeader.fmtc) + sizeof(WavMsAdpcmHeader.fc) + sizeof(WavMsAdpcmHeader.dc.id) + sizeof(WavMsAdpcmHeader.dc.size) + WavMsAdpcmHeader.dc.size; 
    
    //offset                      = dcfTell(pFile);

    /*
    dcfSeek(pFile, 0x04, FS_SEEK_SET);
    if (dcfWrite(pFile, (unsigned char*)&WavMsAdpcmHeader.rc.size, sizeof(WavMsAdpcmHeader.rc.size), &size) == 0) {
        DEBUG_IIS("Wav Write Microsoft ADPCM header error(WavMsAdpcmHeader.rc.size)!!!\n");
        return 0;
    }

    dcfSeek(pFile, 0x4e, FS_SEEK_SET);
    if (dcfWrite(pFile, (unsigned char*)&WavMsAdpcmHeader.fc.nSamples, sizeof(WavMsAdpcmHeader.fc.nSamples), &size) == 0) {
        DEBUG_IIS("Wav Write Microsoft ADPCM header error(WavMsAdpcmHeader.fc.nSamples)!!!\n");
        return 0;
    }

    dcfSeek(pFile, 0x56, FS_SEEK_SET);
    if (dcfWrite(pFile, (unsigned char*)&WavMsAdpcmHeader.dc.size, sizeof(WavMsAdpcmHeader.dc.size), &size) == 0) {
        DEBUG_IIS("Wav Write Microsoft ADPCM header error(WavMsAdpcmHeader.dc.size)!!!\n");
        return 0;
    }
    */
    dcfSeek(pFile, 0x00, FS_SEEK_SET);
    if (dcfWrite(pFile, (unsigned char*)&WavMsAdpcmHeader, sizeof(WavMsAdpcmHeader), &size) == 0) {
        DEBUG_IIS("Wav Write Microsoft ADPCM header error!!!\n");
        return 0;
    }

    //dcfSeek(pFile, offset, FS_SEEK_SET); 
    
    return 1;
}

/*

Routine Description:

    Compress 16 bits PCM audio data to Microsoft ADPCM format.

Arguments:

    pChunk      - 16 bits PCM audio data.
    pAdpcmBuff  - compressed microsoft adpcm data.
    SampleNum   - want compressing sample number
    mode        - 0: encode first audio data of a file.
                  1: encode middle audio data of a file.
                  2: encode last audio data of a file.

Return Value:

    Used buffer number.

*/
s32 MS_ADPCM_Encoder(s16* pChunk, u8* pAdpcmBuff, s32 SampleNum, int mode)
{
    //static  BYTE            blockbuff[256]; 
    static  BYTE            blockbuff[MS_ADPCM_BLOCK_SIZE]; 
    static  ADPCM_COEF_SET  coeff;
    static  ADPCM_COEF_SET* pCoeff;
    static  BLOCK*          pbuff; 
    static  short*          p; 
    static  short*          pend; 
    static  int             index;
    static  int             iDelta;
    static  short           iSample1; 
    static  short           iSample2; 
    static  int             flag; 
    static  BYTE            bytebuff; 
    static  int             cnt; 
    
    int                     remnant;
    int                     iPredSamp; 
    int                     iErrorDelta; 
    int                     remainder; 
    int                     iNewSample;
    
    remnant                     = SampleNum;
    
    if(mode == 0)   // 只有每個檔案開始才要跑這一段
    {
        pbuff                   = (BLOCK*)blockbuff; 
        p                       = pChunk; 
        pend                    = p + SampleNum; 
        //calculateCoefficient(&coeff, p, (remnant > 500) ? 500 : remnant); 
        //calculateCoefficient(&coeff, p, (remnant > MS_ADPCM_SAMPLE_PER_BLOCK) ? MS_ADPCM_SAMPLE_PER_BLOCK : remnant); 
        calculateCoefficient(&coeff, p, (remnant > 10) ? 10 : remnant); 
        index                   = getClosestCoefficientIndex(&coeff); 
        pCoeff                  = &CoeffTable[index]; 
        
        pbuff->iSample2         = *p++; 
        pbuff->iSample1         = *p++; 
        remnant                -= 2;
        
        //iDelta                  = (pCoeff->coef1 * pbuff->iSample1 + pCoeff->coef2 * pbuff->iSample2) / 256 - *p; 
        iDelta                  = ((int)pCoeff->coef1 * pbuff->iSample1 + (int)pCoeff->coef2 * pbuff->iSample2) / 256 - *p; 
        iDelta                 /= 4; 
        if( iDelta <= 0 ) 
            iDelta  = (-iDelta) + 1; 
        if(iDelta < 16)
            iDelta              = 16;
        pbuff->iDelta           = iDelta; 
        pbuff->predictor        = index; 
        
        iSample1                = pbuff->iSample1; 
        iSample2                = pbuff->iSample2; 
        
        flag                    = 0; 
        bytebuff                = 0; 
        cnt                     = 0; 
    } else if((mode == 1) || (mode == 2)) {
        p                       = pChunk; 
        pend                    = p + SampleNum; 
    }
    
    while( p < pend ) 
    {
        //iPredSamp   = (pCoeff->coef1 * iSample1 + pCoeff->coef2 * iSample2) / 256; 
        iPredSamp   = ((int)pCoeff->coef1 * iSample1 + (int)pCoeff->coef2 * iSample2) / 256; 
        
        //iErrorDelta = (*p - iPredSamp) / iDelta; 
        iErrorDelta = ((int)*p - iPredSamp) / iDelta; 
        //remainder   = (*p - iPredSamp) % iDelta; 
        remainder   = ((int)*p - iPredSamp) % iDelta; 
        if( remainder > iDelta / 2 ) 
            ++iErrorDelta; 
        
        if( iErrorDelta > 7 ) 
            iErrorDelta = 7; 
        if( iErrorDelta < -8 ) 
            iErrorDelta = -8; 
        
        iNewSample          = iPredSamp + iErrorDelta * iDelta; 
        if(iNewSample > 32767)
            iNewSample      = 32767;
        if(iNewSample < -32768)
            iNewSample      = -32768;
        iDelta              = iDelta * AdaptationTable[trimToNibble(iErrorDelta)] / 256; 
        if(iDelta < 16)
            iDelta              = 16;
        iSample2            = iSample1; 
        iSample1            = iNewSample; 
        
        ++p; 
        --remnant;
        if( !flag ) 
            flag = 1, bytebuff = trimToNibble(iErrorDelta); 
        else 
        { 
            flag                    = 0; 
            bytebuff                = (bytebuff << 4) | trimToNibble(iErrorDelta); 
            pbuff->pNibbles[cnt++]  = bytebuff; 
            //if( !(cnt < 249) )
            if( cnt >= ((MS_ADPCM_SAMPLE_PER_BLOCK - 2) / 2) ) 
            { 
                //addBlock((char*)pbuff, 256); 
                memcpy(pAdpcmBuff, pbuff, MS_ADPCM_BLOCK_SIZE); 
                curSize        += MS_ADPCM_BLOCK_SIZE;
                pAdpcmBuff     += MS_ADPCM_BLOCK_SIZE;
                AdpcmUsedBuff  += MS_ADPCM_BLOCK_SIZE;

                cnt                 = 0; 
                if( !(p < pend) ) 
                    break; 
                //calculateCoefficient(&coeff, p, (remnant > 500) ? 500 : remnant); 
                //calculateCoefficient(&coeff, p, (remnant > MS_ADPCM_SAMPLE_PER_BLOCK) ? MS_ADPCM_SAMPLE_PER_BLOCK : remnant); 
                calculateCoefficient(&coeff, p, (remnant > 10) ? 10 : remnant); 
                index               = getClosestCoefficientIndex(&coeff); 
                pCoeff              = &CoeffTable[index]; 
                pbuff->predictor    = index; 
                pbuff->iDelta       = iDelta; 
                pbuff->iSample2     = *p++; 
                pbuff->iSample1     = *p++; 
                remnant            -= 2;
                iSample1            = pbuff->iSample1; 
                iSample2            = pbuff->iSample2; 
            }
        }
    }
    //if((mode == 2) && (cnt < 249))      // 只有每個檔案結束才要跑這一段
    if((mode == 2) && (cnt > 0) && (cnt < ((MS_ADPCM_SAMPLE_PER_BLOCK - 2) / 2)))      // 只有每個檔案結束才要跑這一段
    { 
        while( cnt < ((MS_ADPCM_SAMPLE_PER_BLOCK - 2) / 2) ) 
            pbuff->pNibbles[cnt++]  = 0; 
        //addBlock( (char*)pbuff, 256 ); 
        memcpy(pAdpcmBuff, pbuff, MS_ADPCM_BLOCK_SIZE); 
        curSize        += MS_ADPCM_BLOCK_SIZE;
        pAdpcmBuff     += MS_ADPCM_BLOCK_SIZE;
        AdpcmUsedBuff  += MS_ADPCM_BLOCK_SIZE;
    } 
    return cnt; 
}

/*

Routine Description:

    Write Microsoft ADPCM audio chunk.

Arguments:

    pFile       - File handle.
    pMng        - Buffer manager.
    mode        - 0: encode first audio data of a file.
                  1: encode middle audio data of a file.
                  2: encode last audio data of a file.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavWriteVoiceDataChunkMsAdpcm(FS_FILE* pFile, IIS_BUF_MNG* pMng, int mode)
{
    u32     size;
    //u32     chunkTime;
    u32     chunkSize;
    u8*     pChunkBuf;
    s32     i;

    //chunkTime   = pMng->time;
    chunkSize   = pMng->size;
    pChunkBuf   = pMng->buffer;
    
    // u8 to s16 data format convert
    for(i = 0; i < chunkSize; i++) {
        PCM_Data_16bits[i]  = ((s16)pChunkBuf[i] - 0x80) << 8;
        //PCM_Data_16bits[i]  = 0x7fff;
    }

    //while(chunkSize > 0)
    {
        if(mode == 0)
        {
            MS_ADPCM_Encoder((s16*) PCM_Data_16bits, (u8*)AdpcmBuff, chunkSize, 0);
            mode        = 1;
            //debugDumpMemory(chunkSize, (u32)AdpcmBuff);
        } else {
            MS_ADPCM_Encoder((s16*) PCM_Data_16bits, (u8*)AdpcmBuff, chunkSize, mode);
        }
        
        if (dcfWrite(pFile, (u8*)AdpcmBuff, AdpcmUsedBuff, &size) == 0)
        {
            DEBUG_IIS("Rec voice File error \n");
            return 0;
        }
        AdpcmUsedBuff   = 0;
    }
    /* Record the data chunk */ 
    // One size for IIS record is 1024 bytes (256x4 bytes (IIS DMA))
    //DEBUG_IIS("chunkSize = %d\n", chunkSize);
    iisBufferCmpCount   += chunkSize;
        
    return 1;
}

/*

Routine Description:

    Record wav file of Microsoft ADPCM format.

Arguments:

    none.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavRecVoiceFile_MS_ADPCM(void)
{
    FS_FILE*        pFile;
    FS_DISKFREE_T*  diskInfo;
    u32             free_size;
    u32             bytes_per_cluster;
    u16             audio_value;
    u16             audio_value_max;
    u16             width, chan;
    u32             rate, data_size,data_size_pos;
    u32             size;
    riff_wave_hdr_t riff_wave_hdr;
    int             mode;
	u8 tmp;
	
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * bytes_per_cluster;

    if(free_size <= 0x8000)      //reserve for  redundance voice data
    {
        DEBUG_IIS("Memory full \n");
        diskInfo->avail_clusters    = 0; //for Tome -> 0
        sysVoiceRecStop             = 1;
        sysVoiceRecStart            = 0;
        
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        
        return 0;
    }
   
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_WAV, 0)) == NULL){
        DEBUG_IIS("Wav create file error!!!\n");
        return 0;
    }
    
    DEBUG_IIS("iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);
    DEBUG_IIS("iisTrgSemEvt = %d\n", iisTrgSemEvt->OSEventCnt);
    audio_value_max = 0;

    if (Write_MS_ADPCM_Header_Pre(pFile) == 0)
    {
        DEBUG_IIS("Wav Write header error!!!\n");
        sysVoiceRecStop     = 1;
        sysVoiceRecStart    = 0;
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }
    // Resume IIS task
    iisResumeTask();
    iisStartRec();

    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * bytes_per_cluster;
    curSize             = 0;
    AdpcmUsedBuff       = 0;
    mode                = 0;
    while (sysVoiceRecStop == 0)
    {    
    
        if(free_size <= 0x8000)     //reserve for  redundance voice data
        {
            DEBUG_IIS("Memory full \n");
            diskInfo->avail_clusters    = 0; //for Tome -> 0
            sysVoiceRecStop             = 1;
            sysVoiceRecStart            = 0;
            
            osdDrawMemFull(UI_OSD_DRAW);
            OSTimeDly(20);
            
            break;
        }
        OSTimeDly(1);

        // Return the original semaphore count value
        audio_value = OSSemAccept(iisCmpSemEvt);
        if (audio_value > 0)
        {   //Finish one buffer record
            if(audio_value_max < audio_value)
                audio_value_max     = audio_value;
            if (wavWriteVoiceDataChunkMsAdpcm(pFile, &iisSounBufMng[iisSounBufMngReadIdx], mode) == 0) {
                DEBUG_IIS("Wav Write Data error!!!\n");
                sysVoiceRecStop     = 1;
                sysVoiceRecStart    = 0;
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            } else {
                free_size      -= IIS_CHUNK_SIZE / 2;   //Chunk size
                mode            = 1;
            }
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //asfDebugPrint("Trace: IIS frame written.\n");
            OSSemPost(iisTrgSemEvt);
            DEBUG_IIS("a");
        }

    }   //end while
    // Revoke the semaphore key
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
    // mark below because we decrease avail_clusters in dcfclose
    //diskInfo  =   &global_diskInfo;
    //bytes_per_cluster =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    //used_cluster=(iisBufferCmpCount+bytes_per_cluster-1) / bytes_per_cluster; 
    //diskInfo->avail_clusters -= used_cluster;

   
    /* delay until IIS task reach pend state */
    OSTimeDly(3);

    // write redundance voice data
    while(iisCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();   
        audio_value = OSSemAccept(iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0 && mode != 2 && free_size > 0x8000)
        {   
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
            if(audio_value == 1)
                mode    = 2;    // last data
            if (wavWriteVoiceDataChunkMsAdpcm(pFile, &iisSounBufMng[iisSounBufMngReadIdx], mode) == 0) {
                DEBUG_IIS("Wav Write Data error!!!\n");
                sysVoiceRecStop     = 1;
                sysVoiceRecStart    = 0;
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            } else {
                free_size      -= IIS_CHUNK_SIZE / 4;   //Chunk size
            }
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //OSSemPost(iisTrgSemEvt);
            DEBUG_IIS("b");
        }
    }   //end while
    
    if(mode != 2)
    {
        mode    = 2;
        iisSounBufMng[iisSounBufMngReadIdx].size    = 0;
        if (wavWriteVoiceDataChunkMsAdpcm(pFile, &iisSounBufMng[iisSounBufMngReadIdx], mode) == 0) {
            DEBUG_IIS("Wav Write Data error!!!\n");
            sysVoiceRecStop     = 1;
            sysVoiceRecStart    = 0;
            dcfCloseFileByIdx(pFile, 0, &tmp);
            return 0;
        } else {
            free_size          -= IIS_CHUNK_SIZE / 4;   //Chunk size
        }
    }

    // update the wav data size
    if (Write_MS_ADPCM_Header_Post(pFile) == 0)
    {
        DEBUG_IIS("Wav Write header error!!!\n");
        sysVoiceRecStop     = 1;
        sysVoiceRecStart    = 0;
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }
	DEBUG_IIS("global_Wav_count++\n");
    dcfCloseFileByIdx(pFile, 0, &tmp);
    uiMenuEnable        = 2;
    //if(sysTVOutOnFlag)
     // uiTVKeyMenu();
    //else
    //uiKeyMenu();

    return 1;
}

/*

Routine Description:

    Decompress Microsoft ADPCM format audio to 16 bits PCM audio data.

Arguments:

    pChunk      - 16 bits PCM audio data.
    pAdpcmBuff  - compressed microsoft adpcm data.
    SampleNum   - want compressing sample number

Return Value:

    Decoded sample number.

*/
s32 MS_ADPCM_Decoder(s16* pChunk, u8* pAdpcmBuff, s32 BitLen)
{
    static  ADPCM_COEF_SET  coeff;
    static  ADPCM_COEF_SET* pCoeff;
    static  BLOCK*          pbuff; 
    static  short*          p; 
    static  short*          pend; 
    static  int             index;
    static  int             iDelta;
    static  short           iSample1; 
    static  short           iSample2; 
    static  int             flag; 
    static  BYTE            bytebuff; 
    static  int             cnt; 
    static  s32             SampleNum;
    
    int                     iPredSamp; 
    int                     iErrorDelta; 
    int                     remainder; 
    int                     iNewSample;
    
    
    //pbuff                   = (BLOCK*)blockbuff; 
    pbuff                   = (BLOCK*)pAdpcmBuff; 
    p                       = pChunk; 
    SampleNum               = (BitLen - 7) * 2 + 2;
    pend                    = p + SampleNum; 
    iDelta                  = pbuff->iDelta; 
    if(iDelta < 16)
        iDelta              = 16;
    index                   = pbuff->predictor; 
    pCoeff                  = &CoeffTable[index]; 
    
    *p++                    = pbuff->iSample2; 
    *p++                    = pbuff->iSample1; 
    
    
    iSample1                = pbuff->iSample1; 
    iSample2                = pbuff->iSample2; 
    
    flag                    = 0; 
    bytebuff                = 0; 
    cnt                     = 0; 
    
    while( p < pend ) 
    {
        if( !flag ) 
        {
            flag            = 1; 
            bytebuff        = pbuff->pNibbles[cnt++];
            iErrorDelta     = (((int)bytebuff) << 24) >> 28; 
        } else {
            flag            = 0; 
            iErrorDelta     = (((int)bytebuff) << 28) >> 28;
        }
        iPredSamp           = ((int)pCoeff->coef1 * iSample1 + (int)pCoeff->coef2 * iSample2) / 256; 
        iNewSample          = iPredSamp + iErrorDelta * iDelta; 
        if(iNewSample > 32767)
            iNewSample      = 32767;
        if(iNewSample < -32768)
            iNewSample      = -32768;
        iDelta              = iDelta * AdaptationTable[trimToNibble(iErrorDelta)] / 256; 
        if(iDelta < 16)
            iDelta          = 16;
        iSample2            = iSample1; 
        iSample1            = iNewSample; 
        
        *p++                = (s16)iNewSample; 
    }
    return SampleNum; 
}

/*

Routine Description:

    Read Microsoft ADPCM audio chunk.

Arguments:

    pFile       - File handle.
    pMng        - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavReadVoiceDataChunkMsAdpcm(FS_FILE* pFile, IIS_BUF_MNG* pMng, u32 BitLen)
{
    u32     size;
    //u32     chunkTime;
    u32     chunkSize;
    u8*     pChunkBuf;
    s32     i;
	u8	tmp;

    //chunkTime   = pMng->time;
    //chunkSize   = pMng->size    = WavMsAdpcmHeader.fmtc.wSamplesPerBlock;
    pChunkBuf   = pMng->buffer;

    if (dcfRead(pFile, (u8*)AdpcmBuff, BitLen, &size) == 0)
    {
        DEBUG_IIS("Wav Read Data error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    
    chunkSize   = MS_ADPCM_Decoder((s16*) PCM_Data_16bits, (u8*)AdpcmBuff, BitLen);
    pMng->size  = chunkSize;

    // u8 to s16 data format convert
    for(i = 0; i < chunkSize; i++) {
        pChunkBuf[i]    = (PCM_Data_16bits[i] >> 8) + 0x80;
    }

    return 1;
}


/*

Routine Description:

    Playback wav file of Microsoft ADPCM format.

Arguments:

    none.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavReadFile_MS_ADPCM(FS_FILE* pFile)
{
    int data_size, i, VoicePlayCount    = 0;
    u32 data_size_pos;
    u32 size, rest_key, audio_value;
    u32 AudioPlayback       = 0;
	u8	tmp;
	
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;

    IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    // Current IIS playback time(micro second)
    IISTimeUnit             = 0;    // IIS playback time per DMA(micro second)
    iisPlayCount            = 0;    // IIS played chunk number
    iisTotalPlay            = 0;    // IIS total trigger playback number

    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
    //Initialize the IIS semaphore
    // Different from REC mode --> The count of iisTrgSemEvt -->0 iisCmpSemEvt -->126
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }

    while(iisplayCmpEvt->OSEventCnt > 0) {
        OSSemAccept(iisplayCmpEvt);
    }
    Output_Sem();
    while(iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisCmpSemEvt);
    }
    while(iisCmpSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisCmpSemEvt);
    }
    Output_Sem();
    
    //Find the data size
    dcfSeek(pFile, 0x00, FS_SEEK_SET); 
    if (dcfRead(pFile, (char *)&WavMsAdpcmHeader, sizeof(WavMsAdpcmHeader), &size) == 0)
    {
        DEBUG_IIS("Wav Read Microsoft ADPCM header error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }  
    data_size   = WavMsAdpcmHeader.dc.size;
    DEBUG_IIS("Voice Data Size = %d\n", data_size);
    VoicePlayCount  = data_size / MS_ADPCM_BLOCK_SIZE;
    //iisResumeTask();
    
    while(iisPlayCount < VoicePlayCount )
    {
        if(sysVoicePlayStop==1)
            break;
        audio_value = OSSemAccept(iisCmpSemEvt);
        
        if (audio_value > 0) 
        {
            rest_key = OSSemAccept(iisplayCmpEvt);
            if (rest_key==1)
            {
                OSSemPost(iisTrgSemEvt);
            }
            //OSSemPost(wdtSemEvt);
            if (wavReadVoiceDataChunkMsAdpcm(pFile, &iisSounBufMng[iisSounBufMngWriteIdx], WavMsAdpcmHeader.fmtc.format.nBlockAlign) == 0)
            {
                DEBUG_IIS("Wav Read Data error!!!\n");
                dcfClose(pFile, &tmp);
                return 0;
            }

            //Advance the buffer pointer
            iisSounBufMng[iisSounBufMngWriteIdx].size   = WavMsAdpcmHeader.fmtc.wSamplesPerBlock;
            iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
            
            if(AudioPlayback == 0)
            {
                AudioPlayback   = 1;
                iisResumeTask();
                OSSemPost(iisTrgSemEvt);
            }

            // Cal the rest data size
            //DEBUG_IIS("Rest Data Size = %d  -- %d\n", data_size, size);
        }
    }

    dcfClose(pFile, &tmp);
    OSTimeDly(5);
    DEBUG_IIS("Wav Playback End!!!\n");
    // reset IIS hardware
    return 1;
}

#endif  //#if (AUDIO_CODEC == AUDIO_CODEC_MS_ADPCM)
