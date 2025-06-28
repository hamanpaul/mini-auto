/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    ciu_1.c

Abstract:

    The routines of CCIR656 Interface Unit-CH1.
    1. TV decoder 參數設定:
        a. Preview/Capture/Video clip mode.
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2010/07/21  Lucian Yuan  Create  
*/

#include "general.h"
#include "board.h"
#include "mcpureg.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif


#define MCPU_TIMEOUT    20
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_FLAG_GRP  *gMcpuFlagGrp;
OS_EVENT     *gMcpuOpenSem;
OS_EVENT     *gMcpu2OpenSem;

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
void Sw_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                        int Width, int Height,
                        int Stride
                      );

/*
 *********************************************************************************************************
 * Function Body
 *********************************************************************************************************
 */
void mcpuInit(void)
{
    unsigned char err;

    gMcpuOpenSem = OSSemCreate(1);
    gMcpu2OpenSem = OSSemCreate(1);
    gMcpuFlagGrp = OSFlagCreate(0x00000000, &err);

}

void mcpu_ByteMemcpy(u8 *DstAddr, u8 *SrcAddr, unsigned int ByteCnt)
{
   unsigned int pp;
   unsigned char err;


   if(ByteCnt<16)
   {
      memcpy(DstAddr,SrcAddr,ByteCnt);
      return;
   }
   //----//
   OSSemPend(gMcpuOpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU_SrcAddr    = (unsigned int)SrcAddr;
   MCPU_DstAddr    = (unsigned int)DstAddr;

   pp=((unsigned int )DstAddr) & (~0x03);
   MCPU_DstStrWord = *((unsigned int *)pp);

   pp=((unsigned int )(DstAddr+ByteCnt)) & (~0x03);
   MCPU_DstLastWord = *((unsigned int *)pp);

   MCPU_ByteNum=ByteCnt;

   MCPU_Command = MCP_CMD_MODE_LCPY | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu_ByteMemcpy time out!\n");
       DEBUG_MCPU("mcpu_ByteMemcpy(0x%08x, 0x%08x, %d)\n", (u32)DstAddr, (u32)SrcAddr, ByteCnt);
   }
   // release resource
   OSSemPost(gMcpuOpenSem);
}

void mcpu_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                           int Width, int Height,
                           int Stride
                         )
{
   unsigned char err;
   int ByteCnt;
   //=======================//
   ByteCnt =Width * Height;

   if( (ByteCnt<16) || ((int)SrcAddr & 0x03) || ((int)DstAddr & 0x03) || (Width & 0x03) )
   {
      DEBUG_MAIN("mcpu_BlockCpy's Paramater is illegle!\n");
      return;
   }
   //----//
   OSSemPend(gMcpuOpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU_SrcAddr    = (unsigned int)SrcAddr;
   MCPU_DstAddr    = (unsigned int)DstAddr;

   //pp=((unsigned int )DstAddr) & (~0x03);
   //MCPU_DstStrWord = *((unsigned int *)pp);

   //pp=((unsigned int )(DstAddr+ByteCnt)) & (~0x03);
   //MCPU_DstLastWord = *((unsigned int *)pp);

   MCPU_ByteNum=ByteCnt;

   MCPU_DstBlkSize= ((Stride)<<MCP_DST_BLK_STRIDE_SHFT) | ((Width)<<MCP_DST_BLK_WIDTH_SHFT);

   MCPU_Command = MCP_CMD_MODE_BCPY | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu_BlockCpy time out!\n");
   }
   // release resource
   OSSemPost(gMcpuOpenSem);
}


int mcpu_FATZeroScan(u8 *SrcAddr, int ByteCnt)
{
   unsigned char err;
   int ZeroCount;
   //=======================//

   if( (ByteCnt<16) || ((int)SrcAddr & 0x03))
   {
      DEBUG_MAIN("mcpu_FATZeroScan's Paramater is illegle!\n");
      return 0;
   }
   //----//
   OSSemPend(gMcpuOpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU_SrcAddr    = (unsigned int)SrcAddr;
   
   MCPU_ByteNum=ByteCnt;

   MCPU_Command = MCP_CMD_MODE_FATS | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu_FATZeroScan time out!\n");
   }
   // release resource
   ZeroCount=MCPU_FATZeroCnt;
   OSSemPost(gMcpuOpenSem);


   return ZeroCount;
}

void mcpu2_ByteMemcpy(u8 *DstAddr, u8 *SrcAddr, unsigned int ByteCnt)
{
   unsigned int pp;
   unsigned char err;


   if(ByteCnt<16)
   {
      memcpy(DstAddr,SrcAddr,ByteCnt);
      return;
   }
   //----//
   OSSemPend(gMcpu2OpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU2_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP2_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU2_SrcAddr    = (unsigned int)SrcAddr;
   MCPU2_DstAddr    = (unsigned int)DstAddr;

   pp=((unsigned int )DstAddr) & (~0x03);
   MCPU2_DstStrWord = *((unsigned int *)pp);

   pp=((unsigned int )(DstAddr+ByteCnt)) & (~0x03);
   MCPU2_DstLastWord = *((unsigned int *)pp);

   MCPU2_ByteNum=ByteCnt;

   MCPU2_Command = MCP_CMD_MODE_LCPY | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP2_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu2_ByteMemcpy time out!\n");
   }
   // release resource
   OSSemPost(gMcpu2OpenSem);
}

void mcpu2_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                           int Width, int Height,
                           int Stride
                         )
{
   unsigned char err;
   int ByteCnt;
   //=======================//
   ByteCnt =Width * Height;

   if( (ByteCnt<16) || ((int)SrcAddr & 0x03) || ((int)DstAddr & 0x03) || (Width & 0x03) )
   {
      DEBUG_MAIN("mcpu2_BlockCpy's Paramater is illegle!\n");
      return;
   }
   //----//
   OSSemPend(gMcpu2OpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU2_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP2_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU2_SrcAddr    = (unsigned int)SrcAddr;
   MCPU2_DstAddr    = (unsigned int)DstAddr;

   //pp=((unsigned int )DstAddr) & (~0x03);
   //MCPU_DstStrWord = *((unsigned int *)pp);

   //pp=((unsigned int )(DstAddr+ByteCnt)) & (~0x03);
   //MCPU_DstLastWord = *((unsigned int *)pp);

   MCPU2_ByteNum=ByteCnt;

   MCPU2_DstBlkSize= ((Stride)<<MCP_DST_BLK_STRIDE_SHFT) | ((Width)<<MCP_DST_BLK_WIDTH_SHFT);

   MCPU2_Command = MCP_CMD_MODE_BCPY | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP2_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu2_BlockCpy time out!\n");
   }
   // release resource
   OSSemPost(gMcpu2OpenSem);
}


int mcpu2_FATZeroScan(u8 *SrcAddr, int ByteCnt)
{
   unsigned char err;
   int ZeroCount;
   //=======================//

   if( (ByteCnt<16) || ((int)SrcAddr & 0x03))
   {
      DEBUG_MAIN("mcpu2_FATZeroScan's Paramater is illegle!\n");
      return 0;
   }
   //----//
   OSSemPend(gMcpu2OpenSem, OS_IPC_WAIT_FOREVER, &err);
   
   MCPU2_Command    = MCP_CMD_RST;
   OSFlagPost(gMcpuFlagGrp, MCP2_INT_STAT_MCPYFIN , OS_FLAG_CLR, &err);
   
   MCPU2_SrcAddr    = (unsigned int)SrcAddr;
   
   MCPU2_ByteNum=ByteCnt;

   MCPU2_Command = MCP_CMD_MODE_FATS | MCP_CMD_MCPYENA | MCP_CMD_MCPYINT_EN;
   
   OSFlagPend(gMcpuFlagGrp,MCP2_INT_STAT_MCPYFIN, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MCPU_TIMEOUT, &err);
   if (err != OS_NO_ERR)
   {
       DEBUG_MCPU("Error! mcpu2_FATZeroScan time out!\n");
   }
   // release resource
   OSSemPost(gMcpu2OpenSem);

   ZeroCount=MCPU2_FATZeroCnt;

   return ZeroCount;
}

void mcpu2IntHandler(void)
{
    u32 intStat;
    unsigned char err;

    intStat = MCPU2_IntStat;

    if(intStat)
        OSFlagPost(gMcpuFlagGrp, MCP2_INT_STAT_MCPYFIN, OS_FLAG_SET, &err);

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    iauIntHandler();
#endif
}





void mcpuIntHandler(void)
{
    u32 intStat;
    unsigned char err;

    intStat = MCPU_IntStat;
    if(intStat)
        OSFlagPost(gMcpuFlagGrp, MCP_INT_STAT_MCPYFIN , OS_FLAG_SET, &err);

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    iauIntHandler();
#endif
}



