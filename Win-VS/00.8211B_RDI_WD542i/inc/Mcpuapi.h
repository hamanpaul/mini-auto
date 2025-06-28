#ifndef __MCPU_API_H__
#define __MCPU_API_H__

   #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
     extern void mcpuInit();
     extern void mcpu_ByteMemcpy(u8 *DstAddr,u8 *SrcAddr,unsigned int ByteCnt);
     extern void mcpu_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                                       int Width, int Height,
                                       int Stride
                                     );

     extern int mcpu_FATZeroScan(u8 *SrcAddr, int ByteCnt);


     extern void mcpu2_ByteMemcpy(u8 *DstAddr,u8 *SrcAddr,unsigned int ByteCnt);
     extern void mcpu2_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                                       int Width, int Height,
                                       int Stride
                                     );

     extern int mcpu2_FATZeroScan(u8 *SrcAddr, int ByteCnt);

    
     extern void mcpuIntHandler(void);
     extern void mcpu2IntHandler(void);   

     #if MCPU_TEST
         extern  int marsMcpu_Test();
        extern  int marsMcpu2_Test(); 
     #endif

   #endif

#endif
