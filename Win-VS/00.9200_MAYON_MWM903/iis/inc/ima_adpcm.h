#ifndef __IMA_ADPCM_H__
#define __IMA_ADPCM_H__

#include "iisapi.h"

/*
 *********************************************************************************************************
 * Definition
 *********************************************************************************************************
 */

#if (IIS_CHUNK_SIZE == 1024)
#define IMA_ADPCM_BLOCK_SIZE        512
#elif (IIS_CHUNK_SIZE == 2048)
#define IMA_ADPCM_BLOCK_SIZE        1024
#elif (IIS_CHUNK_SIZE == 4096)
#define IMA_ADPCM_BLOCK_SIZE        2048
#endif
#define IMA_ADPCM_SAMPLE_PER_BLOCK  ((IMA_ADPCM_BLOCK_SIZE - 4) * 2 + 1)      // ((nBlockAlign - 4) * 2 + 1)
//#define IMA_ADPCM_BLOCK_SIZE        (4 + (IMA_ADPCM_SAMPLE_PER_BLOCK - 1) / 2)
//#define IMA_ADPCM_BLOCK_SIZE        (4 + IMA_ADPCM_SAMPLE_PER_BLOCK / 2)
//#define IMA_ADPCM_BLOCK_SIZE        (((4 + (IMA_ADPCM_SAMPLE_PER_BLOCK - 1) / 2) + 0xff) & ~0xff)
#define IMA_ADPCM_BUF_SIZE          (IMA_ADPCM_BLOCK_SIZE * 4)
#define IAU_TIMEOUT                 20

/*
typedef unsigned short  WORD; 
typedef unsigned int    DWORD; 
typedef unsigned char   BYTE; 
typedef char            s8; 
typedef unsigned char   u8; 
typedef short           s16; 
typedef unsigned short  u16; 
typedef int             s32; 
typedef unsigned int    u32; 
*/

typedef __packed struct _RIFFWAVECHUNK 
{ 
    DWORD id; 
    DWORD size; 
    DWORD format; 
} RIFFWAVE_CHUNK; 

typedef __packed struct _WAVEFORMATEX 
{ 
    WORD    wFormatTag; 
    WORD    nChannels; 
    DWORD   nSamplesPerSec; 
    DWORD   nAvgBytesPerSec; 
    WORD    nBlockAlign; 
    WORD    wBitsPerSample; 
    WORD    cbSize; 
} WAVEFORMAT_EX; 

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
} FORMAT_CHUNK; 

typedef __packed struct _IMA_ADPCM_FORMAT 
{ 
    DWORD           id; 
    DWORD           size; 
    WAVEFORMAT_EX   format; 
    WORD            wSamplesPerBlock; 
} IMA_ADPCM_FORMAT; 

typedef __packed struct _DATA_CHUNK 
{ 
    DWORD   id; 
    DWORD   size; 
    BYTE    data[1]; 
} DATA_CHUNK; 

typedef __packed struct 
{ 
    DWORD   id; 
    DWORD   size; 
    DWORD   nSamples; 
} FACT_CHUNK; 

typedef __packed struct{ 
    DWORD   id; 
    DWORD   size; 
} CHUNK; 

typedef __packed struct _WAV_PCM_HEADER
{
    RIFFWAVE_CHUNK          rc; 
    FORMAT_CHUNK            fmtc; 
    CHUNK                   dc;
} WAV_PCM_HEADER;

typedef __packed struct _WAV_IMA_ADPCM_HEADER
{
    RIFFWAVE_CHUNK          rc; 
    IMA_ADPCM_FORMAT        fmtc; 
    FACT_CHUNK              fc; 
    CHUNK                   dc;
} WAV_IMA_ADPCM_HEADER;

typedef __packed struct
{ 
    short   sample0; 
    BYTE    index;
    BYTE    reserved;
    BYTE    pNibbles[1]; 
} IMA_ADPCM_BLOCK; 



extern  short                   PredictedValue;
extern  BYTE                    StepIndex;
extern  int                     curSize; 
extern  u8                      AdpcmBuff[IMA_ADPCM_BUF_SIZE];
extern  s16                     PCM_Data_16bits[IMA_ADPCM_SAMPLE_PER_BLOCK];
extern  u8                      PCM_Data_8bits[IMA_ADPCM_SAMPLE_PER_BLOCK];
extern  WAV_IMA_ADPCM_HEADER    WavImaAdpcmHeader;
extern  WAV_PCM_HEADER          WavPcmHeader;




#endif  //  #ifndef __IMA_ADPCM_H__



