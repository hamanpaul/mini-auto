#ifndef __IMA_ADPCM_API_H__
#define __IMA_ADPCM_API_H__

#include "..\iis\inc\ima_adpcm.h"

typedef struct
{
    BYTE    Mode;           // 0: Encoder, 1: Decoder
    s32     Counter;
    s32     AdpcmBufSize;
    s32     PcmBufSize;
    BYTE    *pAdpcmBuf[4];
    BYTE    *pPcmBuf[2];
    BYTE    DecPcmFormat;

    // Register
    BYTE    *PcmAddress1;
    BYTE    *PcmAddress2;
    BYTE    *AdpcmAddress;
    s32     PcmSize1;
    s32     PcmSize2;
    s32     PcmTotalSize;
    s32     AdpcmSize;
    s32     AdpcmSamplePerBlock;
    u32     PcmStrWord;
    u32     AdpcmStrWord;
    s16     AdpcmSample;        // block sample 0
    BYTE    AdpcmIndex;         // block index
    s32     PcmBitPerSample;
    s32     PcmSigned;
} IMA_ADPCM_Option; 

s8 IMA_ADPCM_Encode_Block_HW(IMA_ADPCM_Option* pImaAdpcmOpt);
s32 wavRecVoiceFile_IMA_ADPCM(void);
void iauIntHandler(void);
s8 IMA_ADPCM_Init(void);


//  for workstation test pattern
void DumpEncPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void DumpDecPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void DumpEncADPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void DumpDecADPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void ASM_SW(FILE *pfTask, int Address, int Data, char *strComment);
void ImaAdpcmHWInit(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE *pStartAddress, int mode);
void ImaAdpcmEncSetRegister(IMA_ADPCM_Option *pImaAdpcmOpt);
void ImaAdpcmDecSetRegister(IMA_ADPCM_Option *pImaAdpcmOpt, s16 AdpcmSample, u8 AdpcmIndex);
void OutputEncASM(IMA_ADPCM_Option *pImaAdpcmOpt);
void OutputDecASM(IMA_ADPCM_Option *pImaAdpcmOpt);
void OutputEncVH(IMA_ADPCM_Option *pImaAdpcmOpt);
void OutputDecVH(IMA_ADPCM_Option *pImaAdpcmOpt);

// for FPGA test pattern
void ICE_DumpEncPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void ICE_DumpDecPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void ICE_DumpEncADPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void ICE_DumpDecADPCM(IMA_ADPCM_Option *pImaAdpcmOpt, BYTE* pData, int length);
void ICE_OutputEncInc(IMA_ADPCM_Option *pImaAdpcmOpt);
void ICE_OutputDecInc(IMA_ADPCM_Option *pImaAdpcmOpt);

extern  IMA_ADPCM_Option        ImaAdpcmOpt;


#endif  // #ifndef __IMA_ADPCM_API_H__

