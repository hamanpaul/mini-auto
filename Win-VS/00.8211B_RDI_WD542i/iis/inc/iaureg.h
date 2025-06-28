#ifndef __IAUREG_H__
#define __IAUREG_H__


//IAU_Command
#define ADPCM_RST                   0x00000001
#define ADPCM_ENC_EN                0x00000002
#define ADPCM_DEC_EN                0x00000004
#define ADPCM_ENC_FIN_INT_EN        0x00000008
#define ADPCM_DEC_FIN_INT_EN        0x00000010
#define PCM_BIT_PER_SAMPLE_8        0x00000000
#define PCM_BIT_PER_SAMPLE_16       0x00000020
#define PCM_UNSIGNED                0x00000000
#define PCM_SIGNED                  0x00000040


//IAU_IntStat
#define ADPCM_ENC_FIN_INT           0x00000001
#define ADPCM_DEC_FIN_INT           0x00000002


//IAU_PcmSize1


//IAU_PcmTotalSize


//IAU_AdpcmSize
#define ADPCM_BYTE_SIZE_SHIFT       16

//IAU_PcmAddress1


//IAU_PcmAddress2


//IAU_AdpcmAddress


//IAU_PcmStrWord


//IAU_AdpcmStrWord


//IAU_AdpcmBlockHeader
#define ADPCM_INDEX_SHIFT       16


#endif  // #ifndef __IAUREG_H__

