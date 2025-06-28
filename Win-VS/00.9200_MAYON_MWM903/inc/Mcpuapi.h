#ifndef __MCPU_API_H__
#define __MCPU_API_H__

extern void mcpuInit(void);
extern void mcpu_ByteMemcpy(u8 *DstAddr,u8 *SrcAddr,unsigned int ByteCnt);
extern void mcpu_BlockCpy(u8 *DstAddr, u8 *SrcAddr, int Width, int Height, int Stride);

extern int mcpu_FATZeroScan(u8 *SrcAddr, int ByteCnt);


extern void mcpu2_ByteMemcpy(u8 *DstAddr,u8 *SrcAddr,unsigned int ByteCnt);
extern void mcpu2_BlockCpy( u8 *DstAddr, u8 *SrcAddr, int Width, int Height, int Stride);

extern int mcpu2_FATZeroScan(u8 *SrcAddr, int ByteCnt);


extern void mcpuIntHandler(void);
extern void mcpu2IntHandler(void);

#if MCPU_TEST
extern  int marsMcpu_Test();
extern  int marsMcpu2_Test();
#endif


#endif
