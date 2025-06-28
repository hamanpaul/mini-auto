/*
    Implementation of the IMA ADPCM audio coding algorithm
*/


#include "general.h"

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)

#include <string.h> 
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
#include "iaureg.h"
#include "ima_adpcm.h"
#include "ima_adpcm_api.h"
#include "uiapi.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
static const unsigned char IMA_ADPCMStepTableInit[16] =
{ //7  7  7  7 12  25  50  97 190 371 796 1552 3024  5894  12635  24623
    0, 0, 0, 0, 5, 13, 20, 27, 34, 41, 49,  56,  63,   70,    78,    85, 
};


static const unsigned short IMA_ADPCMStepTable[89] =
{
    7,    8,    9,   10,   11,   12,   13,   14,
   16,   17,   19,   21,   23,   25,   28,   31,
   34,   37,   41,   45,   50,   55,   60,   66,
   73,   80,   88,   97,  107,  118,  130,  143,
  157,  173,  190,  209,  230,  253,  279,  307,
  337,  371,  408,  449,  494,  544,  598,  658,
  724,  796,  876,  963, 1060, 1166, 1282, 1411,
 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
 7132, 7845, 8630, 9493,10442,11487,12635,13899,
15289,16818,18500,20350,22385,24623,27086,29794,
32767
};

static const int IMA_ADPCMIndexTable[8] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
};
*/

/*
 *********************************************************************************************************
 * Global variable
 *********************************************************************************************************
 */

short                   PredictedValue;
BYTE                    StepIndex;
int                     curSize;
u8                      AdpcmBuff[IMA_ADPCM_BUF_SIZE];
s16                     PCM_Data_16bits[IMA_ADPCM_SAMPLE_PER_BLOCK];
u8                      PCM_Data_8bits[IMA_ADPCM_SAMPLE_PER_BLOCK];
WAV_IMA_ADPCM_HEADER    WavImaAdpcmHeader;
WAV_PCM_HEADER          WavPcmHeader;
IMA_ADPCM_Option        ImaAdpcmOpt;
OS_EVENT*               iauTrgSemEvt;
OS_EVENT*               iauCmpSemEvt;

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



/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */


/*

Initialise the PredictedValue and StepIndex members to the optimum values for encoding an audio stream whoes first two PCM samples have the values given. 
Use of this method at the start of audio stream encoding gives improved accuracy over a naive initialisation which sets PredictedValue and StepIndex to predetermined constant values.

Parameters:
    	sample1 	The first PCM sample in the audio stream.
    	sample2 	The second PCM sample in the audio stream. 

*/
/*
void IMA_ADPCM_EncodeInit(short sample1, short sample2)
{
    PredictedValue  = sample1;
    int delta       = sample2-sample1;
    if(delta < 0)
        delta = - delta;
    if(delta > 32767)
        delta = 32767;
    int stepIndex = 0;
#if 0   // full search
    while(IMA_ADPCMStepTable[stepIndex] < (unsigned)delta)
        stepIndex++;
    StepIndex   = stepIndex;
#else   // quick search
    while((delta >> stepIndex) > 0) {
        stepIndex++;
    }
    StepIndex   = IMA_ADPCMStepTableInit[stepIndex];
    //DEBUG_IIS("delta = 0x%02x, 0x%02x\n", delta, IMA_ADPCMStepTable[StepIndex]);
#endif
}

unsigned IMA_ADPCM_Encode_Sample(short pcm16)
{
    int predicedValue   = PredictedValue;
    int stepIndex       = StepIndex;

    int delta = pcm16 - predicedValue;

#if DEBUG_MESSAGE
    DEBUG_IIS("PredictedValue  = 0x%08x\n", PredictedValue);
    DEBUG_IIS("StepIndex       = 0x%08x\n", StepIndex);
    DEBUG_IIS("pcm16           = 0x%08x\n", pcm16);
    DEBUG_IIS("delta           = 0x%08x -> ", delta);
#endif

    unsigned value;
    if(delta >= 0)
        value = 0;
    else
    {
        value = 8;
        delta = -delta;
    }

    int step = IMA_ADPCMStepTable[stepIndex];
    int diff = step >> 3;

#if DEBUG_MESSAGE
    DEBUG_IIS("0x%08x\n", delta);
    DEBUG_IIS("diff            = 0x%08x -> ", diff);
#endif

    if(delta >= step)
    {
        value  |= 4;
        delta  -= step;
        diff   += step;
    }
    step >>= 1;
    if(delta >= step)
    {
        value  |= 2;
        delta  -= step;
        diff   += step;
    }
    step >>= 1;
    if(delta >= step)
    {
        value  |= 1;
        diff   += step;
    }

    if(value & 8)
        predicedValue -= diff;
    else
        predicedValue += diff;
    if(predicedValue < -0x8000)
        predicedValue   = -0x8000;
    else if(predicedValue > 0x7fff)
        predicedValue   = 0x7fff;
    PredictedValue  = predicedValue;

    stepIndex += IMA_ADPCMIndexTable[value & 7];
    if(stepIndex < 0)
        stepIndex   = 0;
    else if(stepIndex > 88)
        stepIndex   = 88;
    StepIndex   = stepIndex;

#if DEBUG_MESSAGE
    DEBUG_IIS("0x%08x\n", diff);
    DEBUG_IIS("value           = 0x%08x\n\n", value);
#endif

    return value;
}

int IMA_ADPCM_Decode_Sample(unsigned adpcm)
{
    int stepIndex   = StepIndex;
    int step        = IMA_ADPCMStepTable[stepIndex];

    stepIndex  += IMA_ADPCMIndexTable[adpcm & 7];
    if(stepIndex < 0)
        stepIndex = 0;
    else if(stepIndex > 88)
        stepIndex = 88;
    StepIndex   = stepIndex;

    int diff    = step >> 3;
    if(adpcm & 4)
        diff   += step;
    if(adpcm & 2)
        diff   += step >> 1;
    if(adpcm & 1)
        diff   += step >> 2;

    int predicedValue   = PredictedValue;
    if(adpcm & 8)
        predicedValue  -= diff;
    else
        predicedValue  += diff;
    if(predicedValue < -0x8000)
        predicedValue   = -0x8000;
    else if(predicedValue > 0x7fff)
        predicedValue   = 0x7fff;
    PredictedValue  = predicedValue;

    return predicedValue;
}
*/

/*

Encode a buffer of 16 bit uniform PCM values into ADPCM values.

Two ADPCM values are stored in each byte. 
The value stored in bits 0-3 corresponds to the sample preceding that stored in bits 4-7. 
Note, if the last encoded ADPCM value is stored in bits 0-3, then bits 4-7 will be cleared to zero.

Parameters:
        dst         Pointer to location to store ADPCM values.
        dstOffset   Offset from dst, in number-of-bits, at which the decoded values will be stored. 
                    I.e. the least significant bit of the first ADPCM value will be stored in byte dst[dstOffset>>3]   
                    at bit position (dstOffset & 7)
                    Where the bit 0 is the least significant bit in a byte and bit 7 is the most significant bit. 
                    The value of dstOffset must be a multiple of 4.
        src         Pointer to the buffer of PCM values to be converted.
        srcSize     The size, in bytes, of the buffer at src. Must be a multiple of 2.

Returns:
    The number of bits which were stored at dst. 

*/
/*
unsigned IMA_ADPCM_Encode_Block(BYTE* dst, int dstOffset, const short* src, unsigned int srcSize)
{
    // use given bit offset
    dst    += dstOffset >> 3;
    unsigned bitOffset  = dstOffset & 4;
#if DEBUG_MESSAGE
    int     SampleIndex = 0;
#endif

    // make sure srcSize represents a whole number of samples
    srcSize &= ~1;

    // calculate end of input buffer
    const short* end = (const short*)((const BYTE*)src + srcSize);

    while(src < end)
    {
#if DEBUG_MESSAGE
        SampleIndex++;
        DEBUG_IIS("Sample    %5d:\n", SampleIndex);
#endif
        // encode a pcm value from input buffer
        unsigned adpcm = IMA_ADPCM_Encode_Sample(*src++);

        // pick which nibble to write adpcm value to...
        if(!bitOffset)
            *dst = adpcm;       // write adpcm value to low nibble
        else
        {
            unsigned b  = *dst;      // get byte from ouput
            b          &= 0x0f;          // clear bits of high nibble
            b          |= adpcm << 4;      // or adpcm value into the high nibble
            *dst++      = (BYTE)b;   // write value back to output and move on to next byte
        }

        // toggle which nibble in byte to write to next
        bitOffset ^= 4;
    }

    // return number bits written to dst
    return srcSize * 2;
}
*/

/*

Decode a buffer of ADPCM values into 16 bit uniform PCM values.

Two ADPCM values are stored in each byte. 
The value stored in bits 0-3 corresponds to the sample preceding that stored in bits 4-7.

Parameters:
    	dst 	    Pointer to location to store PCM values.
    	src 	    Pointer to the buffer of ADPCM values to be converted.
    	srcOffset 	Offset from src, in number-of-bits, from which the ADPCM values will be read. 
                    I.e. the least significant bit of the first ADPCM value will be read from byte src[srcOffset>>3]
                    at bit position (srcOffset & 7)
                    Where the bit 0 is the least significant bit in a byte and bit 7 is the most significant bit. 
                    The value of srcOffset must be a multiple of 4.
    	srcSize 	The number of bits to be read from the buffer at src. Must be a multiple of the size of 4.

Returns:
    The number of bytes which were stored at dst. 

*/
/*
unsigned IMA_ADPCM_Decode_Block(short* dst, const BYTE* src, int srcOffset, unsigned srcSize)
{
    // use given bit offset
    src += srcOffset >> 3;

    // calculate pointers to iterate output buffer
    short* out  = dst;
    short* end  = out+(srcSize >> 2);

    while(out < end)
    {
        // get byte from src
        unsigned adpcm = *src;

        // pick which nibble holds a adpcm value...
        if(srcOffset & 4)
        {
            adpcm >>= 4;  // use high nibble of byte
            ++src;        // move on a byte for next sample
        }

        *out++ = IMA_ADPCM_Decode_Sample(adpcm);  // decode value and store it

        // toggle which nibble in byte to write to next
        srcOffset ^= 4;
    }

    // return number of bytes written to dst
    return (unsigned)out - (unsigned)dst;
}
*/

/*

Routine Description:

    Read PCM wave file data.

Arguments:

    szSrcFileName       - PCM wave file name.
    pWavPcmHeader       - PCM wave header.

Return Value:

    Buffer point of PCM data.

*/
/*
BYTE* ReadPcmFile(char *szSrcFileName, WAV_PCM_HEADER *pWavPcmHeader)
{
    FILE    *fp;
    BYTE    *PCM_Data;
    size_t  size;

    fp      = fopen(szSrcFileName, "rb");
    if(fp == 0) {
        DEBUG_IIS("Error: Open file %s fail!!!\n", szSrcFileName);
        return 0;
    }
    size    = fread(pWavPcmHeader, 1, sizeof(WAV_PCM_HEADER), fp); 
    if(size != sizeof(WAV_PCM_HEADER)) {
        DEBUG_IIS("Error: read %s WAV_PCM_HEADER fail!!!\n", szSrcFileName);
        fclose(fp);
        return 0;
    }
    if(pWavPcmHeader->dc.id == 0x61746164) {    // "data"
        if(pWavPcmHeader->fmtc.wBitsPerSample == 8)     // 8 bits
            size        = pWavPcmHeader->dc.size + IMA_ADPCM_SAMPLE_PER_BLOCK;
        else                                            // 16 bits
            size        = pWavPcmHeader->dc.size + IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
        PCM_Data        = (BYTE*)malloc(size);
        memset(PCM_Data, 0, size);
    } else {
        DEBUG_IIS("Error: read data chunk header fail!!!\n");
        fclose(fp);
        return 0;
    }
    size    = fread(PCM_Data, 1, pWavPcmHeader->dc.size, fp); 
    if(size != pWavPcmHeader->dc.size) {
        DEBUG_IIS("Error: read %s PCM data fail!!! size(%d) != dc.size(%d)\n", szSrcFileName, size, pWavPcmHeader->dc.size);
        free(PCM_Data);
        fclose(fp);
        return 0;
    }
    
    fclose(fp);
    return PCM_Data;
}
*/

/*

Routine Description:

    Read IMA ADPCM wave file data.

Arguments:

    szSrcFileName       - IMA ADPCM file name.
    pWavAdpcmHeader     - IMA ADPCM header.

Return Value:

    Buffer point of PCM data.

*/
/*
BYTE* ReadImaAdpcmFile(char *szSrcFileName, WAV_IMA_ADPCM_HEADER *pWavAdpcmHeader)
{
    FILE    *fp;
    BYTE    *ADPCM_Data;
    size_t  size;

    fp      = fopen(szSrcFileName, "rb");
    if(fp == 0) {
        DEBUG_IIS("Error: Open file %s fail!!!\n", szSrcFileName);
        return 0;
    }
    size    = fread(pWavAdpcmHeader, 1, sizeof(WAV_IMA_ADPCM_HEADER), fp); 
    if(size != sizeof(WAV_IMA_ADPCM_HEADER)) {
        DEBUG_IIS("Error: read %s WAV_IMA_ADPCM_HEADER fail!!!\n", szSrcFileName);
        fclose(fp);
        return 0;
    }
    if(pWavAdpcmHeader->dc.id == 0x61746164) {    // "data"
        ADPCM_Data  = (BYTE*)malloc(pWavAdpcmHeader->dc.size + pWavAdpcmHeader->fmtc.format.nBlockAlign);
        memset(ADPCM_Data, 0, pWavAdpcmHeader->dc.size + pWavAdpcmHeader->fmtc.format.nBlockAlign);
    } else {
        DEBUG_IIS("Error: read data chunk header fail!!!\n");
        fclose(fp);
        return 0;
    }
    size    = fread(ADPCM_Data, 1, pWavAdpcmHeader->dc.size, fp); 
    if(size != pWavAdpcmHeader->dc.size) {
        DEBUG_IIS("Error: read %s PCM data fail!!! size(%d) != dc.size(%d)\n", szSrcFileName, size, pWavAdpcmHeader->dc.size);
        free(ADPCM_Data);
        fclose(fp);
        return 0;
    }
    
    fclose(fp);
    return ADPCM_Data;
}
*/

/*

Routine Description:

    Write IMA ADPCM wave file header previous.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 Write_IMA_ADPCM_Header_Pre(FS_FILE* pFile)
{
    u32             size;

    WavImaAdpcmHeader.rc.id                        = 0x46464952;    // "RIFF"
    WavImaAdpcmHeader.rc.size                      = 0;             // update this item after voice record finishd
    WavImaAdpcmHeader.rc.format                    = 0x45564157;    // "WAVE"

    WavImaAdpcmHeader.fmtc.id                      = 0x20746d66;    // "fmt "
    WavImaAdpcmHeader.fmtc.size                    = sizeof(WavImaAdpcmHeader.fmtc) - 8; 
    WavImaAdpcmHeader.fmtc.format.wFormatTag       = 0x0011; 
    WavImaAdpcmHeader.fmtc.format.nChannels        = 1; 
    WavImaAdpcmHeader.fmtc.format.nSamplesPerSec   = 8000; 
    WavImaAdpcmHeader.fmtc.format.nAvgBytesPerSec  = 0;             // update this item after voice record finishd
    WavImaAdpcmHeader.fmtc.format.nBlockAlign      = IMA_ADPCM_BLOCK_SIZE; 
    WavImaAdpcmHeader.fmtc.format.wBitsPerSample   = 4; 
    WavImaAdpcmHeader.fmtc.format.cbSize           = 2; 
    WavImaAdpcmHeader.fmtc.wSamplesPerBlock        = IMA_ADPCM_SAMPLE_PER_BLOCK; 
    
    WavImaAdpcmHeader.fc.id                        = 0x74636166;    // "fact"
    WavImaAdpcmHeader.fc.size                      = 4; 
    WavImaAdpcmHeader.fc.nSamples                  = 0;             // Total number of samples be coded, update this item after voice record finishd

    WavImaAdpcmHeader.dc.id                        = 0x61746164;    // "data"
    WavImaAdpcmHeader.dc.size                      = 0;             // update this item after voice record finishd
    
    if (dcfWrite(pFile, (unsigned char*)&WavImaAdpcmHeader, sizeof(WavImaAdpcmHeader), &size) == 0) {
        DEBUG_IIS("Wav Write IMA ADPCM header error!!!\n");
        return 0;
    }

    return 1;
}

/*

Routine Description:

    Write IMA ADPCM wave file header post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 Write_IMA_ADPCM_Header_Post(FS_FILE* pFile, IMA_ADPCM_Option* pImaAdpcmOpt)
{
    u32                                 size;

    if(pImaAdpcmOpt->PcmBitPerSample == 8)
        WavImaAdpcmHeader.fc.nSamples               = WavPcmHeader.dc.size;
    else if(pImaAdpcmOpt->PcmBitPerSample == 16)
        WavImaAdpcmHeader.fc.nSamples               = WavPcmHeader.dc.size / 2;
    WavImaAdpcmHeader.dc.size                       = curSize;
    WavImaAdpcmHeader.rc.size                       = 4 + sizeof(WavImaAdpcmHeader.fmtc) + sizeof(WavImaAdpcmHeader.fc) + sizeof(WavImaAdpcmHeader.dc) + WavImaAdpcmHeader.dc.size; 
    WavImaAdpcmHeader.fmtc.format.nAvgBytesPerSec   = curSize * WavImaAdpcmHeader.fmtc.format.nSamplesPerSec / WavImaAdpcmHeader.fc.nSamples;
    
    dcfSeek(pFile, 0x00, FS_SEEK_SET);
    if (dcfWrite(pFile, (unsigned char*)&WavImaAdpcmHeader, sizeof(WavImaAdpcmHeader), &size) == 0) {
        DEBUG_IIS("Wav Write IMA ADPCM header error!!!\n");
        return 0;
    }

    return 1;
}

/*

Routine Description:

    Write PCM wave file header previous.

Arguments:

    pFile           - File handle.
    DecPcmFormat    - 0: 8 bit unsigned
                      1: 16 bit signed

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
s32 Write_PCM_Header_Pre(FILE* pFile, BYTE DecPcmFormat)
{
    u32                                 size;

    WavPcmHeader.rc.id                  = 0x46464952;    // "RIFF"
    WavPcmHeader.rc.size                = 0;             // update this item after voice record finishd
    WavPcmHeader.rc.format              = 0x45564157;    // "WAVE"

    WavPcmHeader.fmtc.id                = 0x20746d66;    // "fmt "
    WavPcmHeader.fmtc.size              = sizeof(WavPcmHeader.fmtc) - 8; 
    WavPcmHeader.fmtc.wFormatTag        = 0x0001; 
    WavPcmHeader.fmtc.nChannels         = 1;
    WavPcmHeader.fmtc.nSamplesPerSec    = 8000;
    if(DecPcmFormat) {  // 16 bit signed
        WavPcmHeader.fmtc.nAvgBytesPerSec   = 16000;
        WavPcmHeader.fmtc.nBlockAlign       = 2; 
        WavPcmHeader.fmtc.wBitsPerSample    = 16;
    } else {            // 8 bit unsigned
        WavPcmHeader.fmtc.nAvgBytesPerSec   = 8000;
        WavPcmHeader.fmtc.nBlockAlign       = 1; 
        WavPcmHeader.fmtc.wBitsPerSample    = 8;
    }
    
    WavPcmHeader.dc.id                  = 0x61746164;    // "data"
    WavPcmHeader.dc.size                = 0;             // update this item after voice record finishd
    
    if ((size = fwrite((unsigned char*)&WavPcmHeader, 1, sizeof(WavPcmHeader), pFile)) == 0) {
        DEBUG_IIS("Wav Write PCM header error!!!\n");
        return 0;
    }

    return 1;
}
*/

/*

Routine Description:

    Write PCM wave file header post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
s32 Write_PCM_Header_Post(FILE* pFile)
{
    u32         size;
    //u32         offset;

    WavPcmHeader.dc.size    = curSize;
    WavPcmHeader.rc.size    = sizeof(WavPcmHeader.rc.format) + sizeof(WavPcmHeader.fmtc) + sizeof(WavPcmHeader.dc) + WavPcmHeader.dc.size; 
    
    //dcfSeek(pFile, 0x00, FS_SEEK_SET);
    fseek(pFile, 0x00, SEEK_SET);
    if ((size = fwrite((unsigned char*)&WavPcmHeader, 1, sizeof(WavPcmHeader), pFile)) == 0) {
        DEBUG_IIS("Wav Write IMA ADPCM header error!!!\n");
        return 0;
    }

    return 1;
}
*/

/*

Routine Description:

    Encoder whole file PCM data to IMA ADPCM data.

Arguments:

    fpTgt       - Target file handle.
    pPCM_Data   - PCM data.

Return Value:

    The number of bytes which were stored to dst. 

*/
/*
u32 IMA_ADPCM_Encode(FILE *fpTgt, BYTE *pPCM_Data)
{
    int                 RemainSample;
    BYTE                *pPCM8;
    u16                 *pPCM16;
    IMA_ADPCM_BLOCK     *pbuff;
    u32                 storebits, WriteSize, WriteTotalSize, i, size;
    s16                 *pPCM;
#if DEBUG_MESSAGE
    int                 BlockIndex  = 0;
#endif

    if(WavPcmHeader.fmtc.wBitsPerSample == 8) {
        RemainSample    = WavPcmHeader.dc.size;
        pPCM8           = pPCM_Data;
    } else if(WavPcmHeader.fmtc.wBitsPerSample == 16) {
        RemainSample    = WavPcmHeader.dc.size / 2;
        pPCM16          = (u16*)pPCM_Data;
    } else {
        RemainSample    = 0;
    }

#if (MAKE_ROM_CODE || MAKE_ICE_SCRIPT)
    ImaAdpcmHWInit(&ImaAdpcmOpt, (BYTE*)ADPCM_STARTADDR, 0);
#endif

    WriteTotalSize      = 0;
    while(RemainSample > 0) {
#if (MAKE_ROM_CODE || MAKE_ICE_SCRIPT)
        ImaAdpcmEncSetRegister(&ImaAdpcmOpt);
    #if MAKE_ROM_CODE
        OutputEncASM(&ImaAdpcmOpt);
        OutputEncVH(&ImaAdpcmOpt);
    #endif
    #if MAKE_ICE_SCRIPT
        ICE_OutputEncInc(&ImaAdpcmOpt);
    #endif
#endif
#if DEBUG_MESSAGE
        DEBUG_IIS("=============================================================================================\n");
        DEBUG_IIS("BlockIndex%5d:\n", BlockIndex);
        BlockIndex++;
#endif

        if(WavPcmHeader.fmtc.wBitsPerSample == 8) {
            // u8 to s16 data format convert
            //for(i = 0; i < chunkSize; i++) {
            //size    = (RemainSample < IMA_ADPCM_SAMPLE_PER_BLOCK) ? RemainSample : IMA_ADPCM_SAMPLE_PER_BLOCK;
            size    = WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
#if MAKE_ROM_CODE
            DumpEncPCM(&ImaAdpcmOpt, pPCM8, size);
#endif
#if MAKE_ICE_SCRIPT
            ICE_DumpEncPCM(&ImaAdpcmOpt, pPCM8, size);
#endif
            for(i = 0; i < size; i++) {
                PCM_Data_16bits[i]  = ((s16)*pPCM8++ - 0x80) << 8;
            }
        } else if(WavPcmHeader.fmtc.wBitsPerSample == 16) {
            //for(i = 0; i < chunkSize; i++) {
            //size    = (RemainSample < IMA_ADPCM_SAMPLE_PER_BLOCK) ? RemainSample : IMA_ADPCM_SAMPLE_PER_BLOCK;
            size    = WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
#if MAKE_ROM_CODE
            DumpEncPCM(&ImaAdpcmOpt, (BYTE*)pPCM16, size * 2);
#endif
#if MAKE_ICE_SCRIPT
            ICE_DumpEncPCM(&ImaAdpcmOpt, (BYTE*)pPCM16, size * 2);
#endif
            for(i = 0; i < size; i++) {
                //PCM_Data_16bits[i]  = (s16)(*pPCM16++ - 0x8000);
                PCM_Data_16bits[i]  = (s16)*pPCM16++;
            }
        }
        IMA_ADPCM_EncodeInit(PCM_Data_16bits[0], PCM_Data_16bits[1]);
        pPCM            = (s16*)PCM_Data_16bits;
        pbuff           = (IMA_ADPCM_BLOCK*)AdpcmBuff;
        pbuff->sample0  = *pPCM++;
        pbuff->index    = StepIndex;
        pbuff->reserved = 0;
        IMA_ADPCM_Encode_Block((BYTE*)&pbuff->pNibbles, 0, pPCM, (WavImaAdpcmHeader.fmtc.wSamplesPerBlock - 1) * 2);
        if(RemainSample < IMA_ADPCM_SAMPLE_PER_BLOCK)
            //storebits       = IMA_ADPCM_Encode_Block((BYTE*)&pbuff->pNibbles, 0, pPCM, (RemainSample - 1) * 2);
            storebits       = (RemainSample - 1) * 4;
        else
            //storebits       = IMA_ADPCM_Encode_Block((BYTE*)&pbuff->pNibbles, 0, pPCM, (IMA_ADPCM_SAMPLE_PER_BLOCK - 1) * 2);
            storebits       = (WavImaAdpcmHeader.fmtc.wSamplesPerBlock - 1) * 4;
        WriteSize       = 4 + ((storebits + 4) >> 3);
        //WriteSize       = IMA_ADPCM_BLOCK_SIZE;
        WriteTotalSize += WriteSize;
        if ((size = fwrite((unsigned char*)pbuff, 1, WriteSize, fpTgt)) == 0) {
            DEBUG_IIS("Error: Wav Write IMA ADPCM block data fail!!!\n");
            return 0;
        }
        RemainSample   -= IMA_ADPCM_SAMPLE_PER_BLOCK;
#if MAKE_ROM_CODE
        //DumpEncADPCM(&ImaAdpcmOpt, (u8*)pbuff, WriteSize);
        DumpEncADPCM(&ImaAdpcmOpt, (u8*)pbuff, WavImaAdpcmHeader.fmtc.format.nBlockAlign);
#endif
#if MAKE_ICE_SCRIPT
        ICE_DumpEncADPCM(&ImaAdpcmOpt, (u8*)pbuff, WavImaAdpcmHeader.fmtc.format.nBlockAlign);
#endif
        ImaAdpcmOpt.Counter++;
    }
    return WriteTotalSize;
}
*/

/*

Routine Description:

    Encoder whole file IMA ADPCM data to PCM data.

Arguments:

    fpTgt           - Target file handle.
    pADPCM_Data     - IMA ADPCM data.

Return Value:

    The number of bytes which were stored to dst. 

*/
/*
u32 IMA_ADPCM_Decode(FILE *fpTgt, BYTE *pADPCM_Data)
{
    int                 RemainSample;
    s16                 *pPCM16;
    IMA_ADPCM_BLOCK     *pbuff;
    u32                 WriteSize, WriteTotalSize, i, size;
    s16                 *pPCM;

#if (MAKE_ROM_CODE || MAKE_ICE_SCRIPT)
    ImaAdpcmHWInit(&ImaAdpcmOpt, (BYTE*)ADPCM_STARTADDR, 0);
#endif

    RemainSample        = WavImaAdpcmHeader.fc.nSamples;
    WriteTotalSize      = 0;
    pbuff               = (IMA_ADPCM_BLOCK*)pADPCM_Data;
    while(RemainSample > 0) {
        pPCM16          = (s16*)PCM_Data_16bits;
        PredictedValue  = pbuff->sample0;
        StepIndex       = pbuff->index;
#if (MAKE_ROM_CODE || MAKE_ICE_SCRIPT)
        ImaAdpcmDecSetRegister(&ImaAdpcmOpt, PredictedValue, StepIndex);
    #if MAKE_ROM_CODE
        OutputDecASM(&ImaAdpcmOpt);
        OutputDecVH(&ImaAdpcmOpt);
        DumpDecADPCM(&ImaAdpcmOpt, (u8*)pbuff, WavImaAdpcmHeader.fmtc.format.nBlockAlign);
    #endif
    #if MAKE_ICE_SCRIPT
        ICE_OutputDecInc(&ImaAdpcmOpt);
        ICE_DumpDecADPCM(&ImaAdpcmOpt, (u8*)pbuff, WavImaAdpcmHeader.fmtc.format.nBlockAlign);
    #endif
#endif
        *pPCM16++       = PredictedValue;
        WriteSize       = IMA_ADPCM_Decode_Block(pPCM16, (u8*)&pbuff->pNibbles, 0, (WavImaAdpcmHeader.fmtc.format.nBlockAlign - 4) * 8);
        WriteSize      += 2;    // count first sample
        if(WavPcmHeader.fmtc.wBitsPerSample == 8) {     // s16 to u8 data format convert
            //for(i = 0; i < chunkSize; i++) {
            for(i = 0; i < WavImaAdpcmHeader.fmtc.wSamplesPerBlock; i++) {
                PCM_Data_8bits[i]   = (u8)(((PCM_Data_16bits[i] + 0x8000) >> 8) & 0xff);
            }
            pPCM        = (s16*)PCM_Data_8bits;
            if(RemainSample < WavImaAdpcmHeader.fmtc.wSamplesPerBlock)
                WriteSize   = RemainSample;
            else
                WriteSize  /= 2;
#if MAKE_ROM_CODE
            DumpDecPCM(&ImaAdpcmOpt, (BYTE*)pPCM, WavImaAdpcmHeader.fmtc.wSamplesPerBlock);
#endif
#if MAKE_ICE_SCRIPT
            ICE_DumpDecPCM(&ImaAdpcmOpt, (BYTE*)pPCM, WavImaAdpcmHeader.fmtc.wSamplesPerBlock);
#endif
        } else if(WavPcmHeader.fmtc.wBitsPerSample == 16) { // s16 data
            pPCM        = (s16*)PCM_Data_16bits;
            if(RemainSample < WavImaAdpcmHeader.fmtc.wSamplesPerBlock)
                WriteSize   = RemainSample * 2;
#if MAKE_ROM_CODE
            DumpDecPCM(&ImaAdpcmOpt, (BYTE*)pPCM, WavImaAdpcmHeader.fmtc.wSamplesPerBlock * 2);
#endif
#if MAKE_ICE_SCRIPT
            ICE_DumpDecPCM(&ImaAdpcmOpt, (BYTE*)pPCM, WavImaAdpcmHeader.fmtc.wSamplesPerBlock * 2);
#endif
        }
        if ((size = fwrite((unsigned char*)pPCM, 1, WriteSize, fpTgt)) == 0) {
            DEBUG_IIS("Error: Wav Write PCM block data fail!!!\n");
            return 0;
        }
        WriteTotalSize += WriteSize;
        RemainSample   -= WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
        pbuff           = (IMA_ADPCM_BLOCK*)((u8*)pbuff + WavImaAdpcmHeader.fmtc.format.nBlockAlign);
        ImaAdpcmOpt.Counter++;
    }
    return WriteTotalSize;
}
*/

/*

Routine Description:

    Convert PCM wave file to IMA ADPCM wave file.

Arguments:

    szTgtFileName   - Target file name.
    szSrcFileName   - Source file name.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
int IMA_ADPCM_Encode_File(char *szTgtFileName, char *szSrcFileName)
{
    FILE    *fpTgt;
    BYTE    *pPCM_Data;

    pPCM_Data   = ReadPcmFile(szSrcFileName, &WavPcmHeader);
    if(pPCM_Data == 0)
        return 0;
    
    fpTgt   = fopen(szTgtFileName, "wb");
    if(fpTgt == 0) {
        DEBUG_IIS("Error: Open file %s fail!!!\n", szTgtFileName);
        return 0;
    }

    if(Write_IMA_ADPCM_Header_Pre(fpTgt) == 0) {
        DEBUG_IIS("Error: Write_IMA_ADPCM_Header_Pre() fail!!!\n");
        free(pPCM_Data);
        fclose(fpTgt);
        return 0;
    }

    curSize    = IMA_ADPCM_Encode(fpTgt, pPCM_Data);
    if(curSize == 0) {
        DEBUG_IIS("Error: IMA_ADPCM_Encode() fail!!!\n");
        free(pPCM_Data);
        fclose(fpTgt);
        return 0;
    }

    if(Write_IMA_ADPCM_Header_Post(fpTgt) == 0) {
        DEBUG_IIS("Error: Write_IMA_ADPCM_Header_Post() fail!!!\n");
        free(pPCM_Data);
        fclose(fpTgt);
        return 0;
    }

    free(pPCM_Data);
    fclose(fpTgt);
    return 1;
}
*/

/*

Routine Description:

    Convert IMA ADPCM wave file to PCM wave file.

Arguments:

    szTgtFileName   - Target file name.
    szSrcFileName   - Source file name.
    DecPcmFormat    - 0: 8 bit unsigned
                      1: 16 bit signed

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
int IMA_ADPCM_Decode_File(char *szTgtFileName, char *szSrcFileName, BYTE DecPcmFormat)
{
    FILE    *fpTgt;
    BYTE    *pADPCM_Data;

    pADPCM_Data   = ReadImaAdpcmFile(szSrcFileName, &WavImaAdpcmHeader);
    if(pADPCM_Data == 0) {
        DEBUG_IIS("Error: ReadImaAdpcmFile() fail!!!\n");
        return 0;
    }
    
    fpTgt   = fopen(szTgtFileName, "wb");
    if(fpTgt == 0) {
        DEBUG_IIS("Error: Open file %s fail!!!\n", szTgtFileName);
        return 0;
    }

    if(Write_PCM_Header_Pre(fpTgt, DecPcmFormat) == 0) {
        DEBUG_IIS("Error: Write_IMA_ADPCM_Header_Pre() fail!!!\n");
        free(pADPCM_Data);
        fclose(fpTgt);
        return 0;
    }
    curSize    = IMA_ADPCM_Decode(fpTgt, pADPCM_Data);
    if(curSize == 0) {
        DEBUG_IIS("Error: IMA_ADPCM_Encode() fail!!!\n");
        free(pADPCM_Data);
        fclose(fpTgt);
        return 0;
    }
    if(Write_PCM_Header_Post(fpTgt) == 0) {
        DEBUG_IIS("Error: Write_IMA_ADPCM_Header_Post() fail!!!\n");
        free(pADPCM_Data);
        fclose(fpTgt);
        return 0;
    }

    free(pADPCM_Data);
    fclose(fpTgt);
    return 1;
}

int main(int argc, char** argv)
{
    char    *szTgtFileName, *szSrcFileName;
    int     mode, DecPcmFormat;

    if(argc < 4) {
        DEBUG_IIS("ima_adpcm source target mode DecPcmFormat\n"); 
        DEBUG_IIS("source          - source file name\n"); 
        DEBUG_IIS("target          - target file name\n"); 
        DEBUG_IIS("mode            - 0: encoder\n"); 
        DEBUG_IIS("                  1: decoder\n"); 
        DEBUG_IIS("DecPcmFormat    - decoder valid only\n"); 
        DEBUG_IIS("                  0: 8 bit unsigned\n"); 
        DEBUG_IIS("                  1: 16 bit signed\n"); 
        return 0; 
    }
    szSrcFileName   = argv[1];
    szTgtFileName   = argv[2];
    mode            = atoi(argv[3]);

    if(mode == 0) {         // Encode PCM to IMA ADPCM
        DEBUG_IIS("Convert PCM file(%s) to IMA ADPCM file(%s)\n", szSrcFileName, szTgtFileName); 
        IMA_ADPCM_Encode_File(szTgtFileName, szSrcFileName);
    } else if(mode == 1) {  // Decode ADPCM to PCM
        DecPcmFormat    = atoi(argv[4]);
        DEBUG_IIS("Convert IMA ADPCM file(%s) to PCM file(%s)\n", szSrcFileName, szTgtFileName);
        IMA_ADPCM_Decode_File(szTgtFileName, szSrcFileName, DecPcmFormat);
    } else
        DEBUG_IIS("mode(%d) not support!!!\n", mode); 

    return 1;
}
*/

/*

Initial IMA ADPCM.

Parameters:
    None.

Returns:
    0: Failure.
    1: Success. 

*/
s8 IMA_ADPCM_Init(void)
{
    iauTrgSemEvt = OSSemCreate(1);
    iauCmpSemEvt = OSSemCreate(0);
    
    return 1;
}

/*

Encode a buffer of PCM values into IMA ADPCM values.

Parameters:
    pImaAdpcmOpt    - Point to IMA ADPCM option.

Returns:
    0: Failure.
    1: Success. 

*/
s8 IMA_ADPCM_Encode_Block_HW(IMA_ADPCM_Option* pImaAdpcmOpt)
{
    u8      err;
    u32     CommandReg;

    OSSemPend(iauTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
    
    IAU_PcmSize1            = (unsigned int)pImaAdpcmOpt->PcmSize1;
    IAU_PcmTotalSize        = (unsigned int)pImaAdpcmOpt->PcmTotalSize;
    IAU_AdpcmSize           = (unsigned int)(pImaAdpcmOpt->AdpcmSize << 16 | pImaAdpcmOpt->AdpcmSamplePerBlock);
    IAU_PcmAddress1         = (unsigned int)pImaAdpcmOpt->PcmAddress1;
    IAU_PcmAddress2         = (unsigned int)pImaAdpcmOpt->PcmAddress2;
    IAU_AdpcmAddress        = (unsigned int)pImaAdpcmOpt->AdpcmAddress;
    IAU_PcmStrWord          = (unsigned int)pImaAdpcmOpt->PcmStrWord;
    IAU_AdpcmStrWord        = (unsigned int)pImaAdpcmOpt->AdpcmStrWord;
    IAU_AdpcmBlockHeader    = (unsigned int)(pImaAdpcmOpt->AdpcmIndex << 16 | (pImaAdpcmOpt->AdpcmSample & 0xffff));

    CommandReg              = ADPCM_ENC_EN | ADPCM_ENC_FIN_INT_EN;
    if(pImaAdpcmOpt->PcmBitPerSample == 16)
        CommandReg         |= PCM_BIT_PER_SAMPLE_16;
    if(pImaAdpcmOpt->PcmSigned)
        CommandReg         |= PCM_SIGNED;
    IAU_Command             = (unsigned int)CommandReg;

    OSSemPend(iauCmpSemEvt, IAU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        // reset IAU hardware
        IAU_Command        |= ADPCM_RST; 
        DEBUG_MP4("IAU Encoder Error: iauCmpSemEvt is %d.\n", err);
        OSSemPost(iauTrgSemEvt);
        return 0;
    }
    
    OSSemPost(iauTrgSemEvt);
    return  1;
}

/*

Encode a buffer of IMA ADPCM values into PCM values.

Parameters:
    pImaAdpcmOpt    - Point to IMA ADPCM option.

Returns:
    0: Failure.
    1: Success. 

*/
s8 IMA_ADPCM_Decode_Block_HW(IMA_ADPCM_Option* pImaAdpcmOpt)
{
    u8      err;
    u32     CommandReg;

    OSSemPend(iauTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
    
    IAU_PcmSize1            = (unsigned int)pImaAdpcmOpt->PcmSize1;
    IAU_PcmTotalSize        = (unsigned int)pImaAdpcmOpt->PcmTotalSize;
    IAU_AdpcmSize           = (unsigned int)(pImaAdpcmOpt->AdpcmSize << 16 | pImaAdpcmOpt->AdpcmSamplePerBlock);
    IAU_PcmAddress1         = (unsigned int)pImaAdpcmOpt->PcmAddress1;
    IAU_PcmAddress2         = (unsigned int)pImaAdpcmOpt->PcmAddress2;
    IAU_AdpcmAddress        = (unsigned int)pImaAdpcmOpt->AdpcmAddress;
    IAU_PcmStrWord          = (unsigned int)pImaAdpcmOpt->PcmStrWord;
    IAU_AdpcmStrWord        = (unsigned int)pImaAdpcmOpt->AdpcmStrWord;
    IAU_AdpcmBlockHeader    = (unsigned int)(pImaAdpcmOpt->AdpcmIndex << 16 | (pImaAdpcmOpt->AdpcmSample & 0xffff));

    CommandReg              = ADPCM_DEC_EN | ADPCM_DEC_FIN_INT_EN;
    if(pImaAdpcmOpt->PcmBitPerSample == 16)
        CommandReg         |= PCM_BIT_PER_SAMPLE_16;
    if(pImaAdpcmOpt->PcmSigned)
        CommandReg         |= PCM_SIGNED;
    IAU_Command             = (unsigned int)CommandReg;

    OSSemPend(iauCmpSemEvt, IAU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        // reset IAU hardware
        IAU_Command        |= ADPCM_RST; 
        DEBUG_MP4("IAU Encoder Error: iauCmpSemEvt is %d.\n", err);
        OSSemPost(iauTrgSemEvt);
        return 0;
    }
    
    OSSemPost(iauTrgSemEvt);
    return  1;
}

/*

Routine Description:

    Record wav file of IMA ADPCM format.

Arguments:

    none.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavRecVoiceFile_IMA_ADPCM(void)
{
    FS_FILE*            pFile;
    FS_DISKFREE_T*      diskInfo;
    u32                 free_size;
    u32                 bytes_per_cluster;
    u16                 audio_value;
    u16                 audio_value_max;
    u16                 width, chan;
    u32                 rate, data_size,data_size_pos;
    u32                 size;
    riff_wave_hdr_t     riff_wave_hdr;
    //int                 mode;
    s32                 GetPcmSize, UsePcmSize, PcmBytesForAdpcm;
    u32                 iisNextIdx, PcmOffset;
    IMA_ADPCM_Option    *pImaAdpcmOpt;
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

    if (Write_IMA_ADPCM_Header_Pre(pFile) == 0)
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

    diskInfo                            = &global_diskInfo;
    bytes_per_cluster                   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size                           = diskInfo->avail_clusters * bytes_per_cluster;
    curSize                             = 0;
    //AdpcmUsedBuff                       = 0;
    //mode                                = 0;
    GetPcmSize                          = 0;
    UsePcmSize                          = 0;
    iisNextIdx                          = 0;
    PcmOffset                           = 0;
    pImaAdpcmOpt                        = &ImaAdpcmOpt;
    pImaAdpcmOpt->PcmAddress1           = 0;
    pImaAdpcmOpt->PcmAddress2           = 0;
    pImaAdpcmOpt->AdpcmAddress          = ImaAdpcmBuf;
    pImaAdpcmOpt->PcmSize1              = 0;
    pImaAdpcmOpt->PcmSize2              = 0;
    pImaAdpcmOpt->PcmTotalSize          = IIS_CHUNK_SIZE;
    pImaAdpcmOpt->AdpcmSize             = IMA_ADPCM_BLOCK_SIZE;
    pImaAdpcmOpt->AdpcmSamplePerBlock   = IMA_ADPCM_SAMPLE_PER_BLOCK;
    pImaAdpcmOpt->PcmStrWord            = 0;
    pImaAdpcmOpt->AdpcmStrWord          = 0;
    pImaAdpcmOpt->AdpcmSample           = 0;
    pImaAdpcmOpt->AdpcmIndex            = 0;
    switch (Audio_formate) {
        case nomo_8bit_8k:
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
            DEBUG_IIS("Don't support Audio_formate %d\n", Audio_formate);
    }
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

        // Return the original semaphore count value
        audio_value = OSSemAccept(iisCmpSemEvt);
        if (audio_value > 0)
        {   //Finish one buffer record
            GetPcmSize             += iisSounBufMng[iisSounBufMngReadIdx].size;
            if(audio_value_max < audio_value)
                audio_value_max     = audio_value;
            while((GetPcmSize - UsePcmSize) >= PcmBytesForAdpcm) {
                if(PcmOffset <= (IIS_CHUNK_SIZE - PcmBytesForAdpcm)) {
                    pImaAdpcmOpt->PcmAddress1   = iisSounBufMng[iisSounBufMngReadIdx].buffer + PcmOffset;
                    pImaAdpcmOpt->PcmAddress2   = 0;
                    pImaAdpcmOpt->PcmSize1      = PcmBytesForAdpcm;
                    pImaAdpcmOpt->PcmSize2      = 0;
                    //PcmOffset                  += PcmBytesForAdpcm;
                } else {
                    pImaAdpcmOpt->PcmAddress1   = iisSounBufMng[iisSounBufMngReadIdx].buffer + PcmOffset;
                    iisNextIdx                  = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    pImaAdpcmOpt->PcmAddress2   = iisSounBufMng[iisNextIdx].buffer;
                    pImaAdpcmOpt->PcmSize1      = IIS_CHUNK_SIZE - PcmOffset;
                    pImaAdpcmOpt->PcmSize2      = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
                    //PcmOffset                   = pImaAdpcmOpt->PcmSize2;
                }
                if(IMA_ADPCM_Encode_Block_HW(pImaAdpcmOpt) == 0) {
                    DEBUG_IIS("Wav Write Data error!!!\n");
                    sysVoiceRecStop     = 1;
                    sysVoiceRecStart    = 0;
                    dcfCloseFileByIdx(pFile, 0, &tmp);
                    return 0;
                } else {
                    free_size      -= IMA_ADPCM_BLOCK_SIZE;   //Chunk size
                    if (dcfWrite(pFile, (u8*)pImaAdpcmOpt->AdpcmAddress, pImaAdpcmOpt->AdpcmSize, &size) == 0)
                    {
                        DEBUG_IIS("Rec voice File error \n");
                        dcfCloseFileByIdx(pFile, 0, &tmp);
                        return 0;
                    }
                    curSize        += IMA_ADPCM_BLOCK_SIZE;
                }
                UsePcmSize             += PcmBytesForAdpcm;
                PcmOffset              += PcmBytesForAdpcm;
                //if(iisNextIdx == ((iisSounBufMngReadIdx + 1) % IIS_BUF_NUM)) {
                if(PcmOffset >= IIS_CHUNK_SIZE) {
                    //iisSounBufMngReadIdx    = iisNextIdx;
                    iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    if(iisSounBufMngReadIdx != iisNextIdx)
                        DEBUG_IIS("iisSounBufMngReadIdx(%d) != iisNextIdx(%d)\n", iisSounBufMngReadIdx, iisNextIdx);
                    OSSemPost(iisTrgSemEvt);
                    PcmOffset              %= IIS_CHUNK_SIZE;
                    DEBUG_IIS("a");
                }
            }
        } else {
            OSTimeDly(1);
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
        audio_value = OSSemAccept(iisCmpSemEvt);
        if (audio_value > 0)
        {   //Finish one buffer record
            GetPcmSize             += iisSounBufMng[iisSounBufMngReadIdx].size;
            if(audio_value_max < audio_value)
                audio_value_max     = audio_value;
            while((GetPcmSize - UsePcmSize) >= PcmBytesForAdpcm) {
                if(PcmOffset <= (IIS_CHUNK_SIZE - PcmBytesForAdpcm)) {
                    pImaAdpcmOpt->PcmAddress1   = iisSounBufMng[iisSounBufMngReadIdx].buffer + PcmOffset;
                    pImaAdpcmOpt->PcmAddress2   = 0;
                    pImaAdpcmOpt->PcmSize1      = PcmBytesForAdpcm;
                    pImaAdpcmOpt->PcmSize2      = 0;
                    //PcmOffset                  += PcmBytesForAdpcm;
                } else {
                    pImaAdpcmOpt->PcmAddress1   = iisSounBufMng[iisSounBufMngReadIdx].buffer + PcmOffset;
                    iisNextIdx                  = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    pImaAdpcmOpt->PcmAddress2   = iisSounBufMng[iisNextIdx].buffer;
                    pImaAdpcmOpt->PcmSize1      = IIS_CHUNK_SIZE - PcmOffset;
                    pImaAdpcmOpt->PcmSize2      = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
                    //PcmOffset                   = pImaAdpcmOpt->PcmSize2;
                }
                if(IMA_ADPCM_Encode_Block_HW(pImaAdpcmOpt) == 0) {
                    DEBUG_IIS("Wav Write Data error!!!\n");
                    sysVoiceRecStop     = 1;
                    sysVoiceRecStart    = 0;
                    dcfCloseFileByIdx(pFile, 0, &tmp);
                    return 0;
                } else {
                    free_size      -= IMA_ADPCM_BLOCK_SIZE;   //Chunk size
                    if (dcfWrite(pFile, (u8*)pImaAdpcmOpt->AdpcmAddress, pImaAdpcmOpt->AdpcmSize, &size) == 0)
                    {
                        DEBUG_IIS("Rec voice File error \n");
                        dcfCloseFileByIdx(pFile, 0, &tmp);
                        return 0;
                    }
                    curSize            += IMA_ADPCM_BLOCK_SIZE;
                }
                UsePcmSize             += PcmBytesForAdpcm;
                PcmOffset              += PcmBytesForAdpcm;
                //if(iisNextIdx == ((iisSounBufMngReadIdx + 1) % IIS_BUF_NUM)) {
                if(PcmOffset >= IIS_CHUNK_SIZE) {
                    //iisSounBufMngReadIdx    = iisNextIdx;
                    iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    if(iisSounBufMngReadIdx != iisNextIdx)
                        DEBUG_IIS("iisSounBufMngReadIdx(%d) != iisNextIdx(%d)\n", iisSounBufMngReadIdx, iisNextIdx);
                    PcmOffset              %= IIS_CHUNK_SIZE;
                    DEBUG_IIS("b");
                }
            }
        }
    }   //end while
    
    if(GetPcmSize != UsePcmSize)    // the remnan of last chunk data
    {
        pImaAdpcmOpt->PcmAddress1   = iisSounBufMng[iisSounBufMngReadIdx].buffer + PcmOffset;
        iisNextIdx                  = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
        pImaAdpcmOpt->PcmAddress2   = iisSounBufMng[iisNextIdx].buffer;
        pImaAdpcmOpt->PcmSize1      = IIS_CHUNK_SIZE - PcmOffset;
        pImaAdpcmOpt->PcmSize2      = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
        PcmOffset                   = pImaAdpcmOpt->PcmSize2;
        memset(pImaAdpcmOpt->PcmAddress2, 0, pImaAdpcmOpt->PcmSize2);
        if(IMA_ADPCM_Encode_Block_HW(pImaAdpcmOpt) == 0) {
            DEBUG_IIS("Wav Write Data error!!!\n");
            sysVoiceRecStop     = 1;
            sysVoiceRecStart    = 0;
            dcfCloseFileByIdx(pFile, 0, &tmp);
            return 0;
        } else {
            if(pImaAdpcmOpt->PcmBitPerSample == 8)
                PcmBytesForAdpcm    = 4 + pImaAdpcmOpt->PcmSize1 / 2;
            else if(pImaAdpcmOpt->PcmBitPerSample == 16)
                PcmBytesForAdpcm    = 4 + (pImaAdpcmOpt->PcmSize1 + 1) / 4;
            free_size          -= IMA_ADPCM_BLOCK_SIZE;   //Chunk size
            if (dcfWrite(pFile, (u8*)pImaAdpcmOpt->AdpcmAddress, 4 + pImaAdpcmOpt->PcmSize1 / 2, &size) == 0)
            {
                DEBUG_IIS("Rec voice File error \n");
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }
            curSize            += 4 + pImaAdpcmOpt->PcmSize1 / 2;
        }
        UsePcmSize             += pImaAdpcmOpt->PcmSize1;
    }
    WavPcmHeader.dc.size    = GetPcmSize;

    // update the wav data size
    if (Write_IMA_ADPCM_Header_Post(pFile, pImaAdpcmOpt) == 0)
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

    The IRQ handler of IMA ADPCM.

Arguments:

    None.

Return Value:

    None.

*/
void iauIntHandler(void)
{
 
    u32  INT_SOURCE;  

    //gpioSetLevel(0, 0, 1);
    INT_SOURCE = IAU_IntStat;
    if(INT_SOURCE) {
        if(IISMode == 1) 
        {   // IMA ADPCM playback
            OSSemPost(iauCmpSemEvt);
        } 
        else if(IISMode == 0 || IISMode == 2) 
        {   // IMA ADPCM record or Receive and playback audio in preview mode
            OSSemPost(iauCmpSemEvt);
        }
    }
}

/*

Routine Description:

    Playback wav file of IMA ADPCM format.

Arguments:

    none.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavReadFile_IMA_ADPCM(FS_FILE* pFile)
{
    int                 data_size, i, VoicePlayCount;
    u32                 data_size_pos;
    u32                 size, rest_key, audio_value;
    u32                 AudioPlayback;
    IMA_ADPCM_BLOCK     *pbuff;
    s32                 CurrentPcmSize, UsePcmSize, PcmBytesForAdpcm;
    u32                 iisNextIdx, PcmOffset;
    IMA_ADPCM_Option    *pImaAdpcmOpt;
    IIS_BUF_MNG*        pMng;
    u32                 readsize;
    u8 tmp;
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;

    IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    // Current IIS playback time(micro second)
    IISTimeUnit             = 0;    // IIS playback time per DMA(micro second)
    iisPlayCount            = 0;    // IIS played chunk number
    iisTotalPlay            = 0;    // IIS total trigger playback number
    AudioPlayback           = 0;
    
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
    if (dcfRead(pFile, (char *)&WavImaAdpcmHeader, sizeof(WavImaAdpcmHeader), &size) == 0)
    {
        DEBUG_IIS("Wav Read IMA ADPCM header error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }  
    data_size                           = WavImaAdpcmHeader.fc.nSamples;
    DEBUG_IIS("Voice Data Size = %d\n", data_size);
    VoicePlayCount                      = data_size / IIS_CHUNK_SIZE;
    //iisResumeTask();
    
    iisNextIdx                          = 0;
    PcmOffset                           = 0;
    pbuff                               = (IMA_ADPCM_BLOCK*)ImaAdpcmBuf;
    PcmBytesForAdpcm                    = WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
    pImaAdpcmOpt                        = &ImaAdpcmOpt;
    pImaAdpcmOpt->PcmAddress1           = 0;
    pImaAdpcmOpt->PcmAddress2           = 0;
    pImaAdpcmOpt->AdpcmAddress          = ImaAdpcmBuf;
    pImaAdpcmOpt->PcmSize1              = 0;
    pImaAdpcmOpt->PcmSize2              = 0;
    pImaAdpcmOpt->PcmTotalSize          = IIS_CHUNK_SIZE;
    pImaAdpcmOpt->AdpcmSize             = WavImaAdpcmHeader.fmtc.format.nBlockAlign;
    pImaAdpcmOpt->AdpcmSamplePerBlock   = WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
    pImaAdpcmOpt->PcmStrWord            = 0;
    pImaAdpcmOpt->AdpcmStrWord          = 0;
    pImaAdpcmOpt->AdpcmSample           = 0;
    pImaAdpcmOpt->AdpcmIndex            = 0;
    pImaAdpcmOpt->PcmBitPerSample       = 8;
    pImaAdpcmOpt->PcmSigned             = 0;
    PcmBytesForAdpcm                    = WavImaAdpcmHeader.fmtc.wSamplesPerBlock;
    readsize                            = pImaAdpcmOpt->AdpcmSize;

    //while(CurrentPcmSize < data_size)
    while(iisPlayCount < VoicePlayCount )
    {
        if(sysVoicePlayStop == 1)
            break;
        audio_value = OSSemAccept(iisCmpSemEvt);
        
        if (audio_value > 0) 
        {
            rest_key = OSSemAccept(iisplayCmpEvt);
            if (rest_key==1)
            {
                OSSemPost(iisTrgSemEvt);
            }
            iisNextIdx          = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
            while(PcmOffset < IIS_CHUNK_SIZE)
            {
                if((CurrentPcmSize + PcmBytesForAdpcm) > data_size) {
                    readsize    = 4 + (data_size - CurrentPcmSize - 1) / 2;
                }
                if (dcfRead(pFile, (u8*)ImaAdpcmBuf, readsize, &size) == 0)
                {
                    DEBUG_IIS("Wav Read Data error!!!\n");
                    dcfClose(pFile, &tmp);
                    return 0;
                }
                pImaAdpcmOpt->PcmAddress1           = iisSounBufMng[iisSounBufMngWriteIdx].buffer + PcmOffset;
                pImaAdpcmOpt->PcmAddress2           = iisSounBufMng[iisNextIdx].buffer;
                pImaAdpcmOpt->PcmSize1              = IIS_CHUNK_SIZE - PcmOffset;
                pImaAdpcmOpt->PcmSize2              = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
                pImaAdpcmOpt->PcmStrWord            = *(u32*)((u32)(pImaAdpcmOpt->PcmAddress1) & ~3);
                pImaAdpcmOpt->AdpcmStrWord          = 0;
                pImaAdpcmOpt->AdpcmSample           = pbuff->sample0;
                pImaAdpcmOpt->AdpcmIndex            = pbuff->index;
                pImaAdpcmOpt->PcmBitPerSample       = 8;
                pImaAdpcmOpt->PcmSigned             = 0;
                IMA_ADPCM_Decode_Block_HW(pImaAdpcmOpt);
                CurrentPcmSize                     += PcmBytesForAdpcm;
                PcmOffset                          += PcmBytesForAdpcm;
            }

            //Advance the buffer pointer
            iisSounBufMng[iisSounBufMngWriteIdx].size   = IIS_CHUNK_SIZE;
            iisSounBufMngWriteIdx                       = iisNextIdx;
            PcmOffset                                  %= IIS_CHUNK_SIZE;
            DEBUG_IIS("a");
            
            if(AudioPlayback == 0)
            {
                AudioPlayback   = 1;
                iisResumeTask();
                OSSemPost(iisTrgSemEvt);
            }

            // Cal the rest data size
            //DEBUG_IIS("Rest Data Size = %d  -- %d\n", data_size, size);
        } else {
            OSTimeDly(1);
            DEBUG_IIS("b");
        }
    }
    
    DEBUG_IIS("\nCurrentPcmSize = %d\n", CurrentPcmSize);
    if(CurrentPcmSize >= data_size)
        DEBUG_IIS("Read wave file finished!!\n");

    while((iisPlayCount < VoicePlayCount) && (sysVoicePlayStop == 0)) {
        OSTimeDly(1);
        DEBUG_IIS("c");
    }

    dcfClose(pFile, &tmp);
    OSTimeDly(5);
    DEBUG_IIS("Wav Playback End!!!\n");
    // reset IIS hardware
    return 1;
}


#endif  // #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
