

#ifndef __MARS_DMA_H__
#define __MARS_DMA_H__

#include	<osapi.h>
#include    "dmaapi.h"

#ifndef FP_VOID
typedef void   (*FP_VOID)(void);    // Function Point
#endif

#ifndef FP_INT
typedef void   (*FP_INT)(int);    // Function Point
#endif

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define DMA_ID_0     0
#define DMA_ID_1     1
#define DMA_ID_2     2
#define DMA_ID_3     3

#define DMA_ID_4     4
#define DMA_ID_5     5
#define DMA_ID_6     6
#define DMA_ID_7     7

#define DMA_ID_NUM   8
#else
#define DMA_ID_0     0
#define DMA_ID_1     1
#define DMA_ID_2     2
#define DMA_ID_3     3
#define DMA_ID_NUM   4
#endif

#ifdef  TCM_DMA_SUPPORT
// for TCM_CTRL
#define TCM_DMA_TRIG        0x00000001
#define TCM_FINISH          0x00000002
#define TCM_INT_EN          0x00000004
#define TCM_BURST_01        0x00000000
#define TCM_BURST_04        0x00000008
#define TCM_BURST_08        0x00000010
#define TCM_BURST_16        0x00000018
#define TCM_SRC_INC         0x00000100
#define TCM_REQ_SEL_SDRAM   0x00000000
#define TCM_REQ_SEL_SDC     0x02000000
#define TCM_REQ_SEL_SPI     0x07000000
#define TCM_RST             0x80000000
#endif


#ifndef DMA_INT_ENA_FINISH

/* DmaChXCmd */ 
#define DMA_STOP            0x00000000
#define DMA_START           0x00000001

#define DMA_INT_ENA         0x00000004

/* DmaIntStat */
#define DMA_INT_STAT_0          0x00000003
#define DMA_INT_STAT_0_FINISH   0x00000001
#define DMA_INT_STAT_0_ERROR    0x00000002
#define DMA_INT_STAT_1          0x0000000c
#define DMA_INT_STAT_1_FINISH   0x00000004
#define DMA_INT_STAT_1_ERROR    0x00000008
#define DMA_INT_STAT_2          0x00000030
#define DMA_INT_STAT_2_FINISH   0x00000010
#define DMA_INT_STAT_2_ERROR    0x00000020
#define DMA_INT_STAT_3          0x000000c0
#define DMA_INT_STAT_3_FINISH   0x00000040
#define DMA_INT_STAT_3_ERROR    0x00000080
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define DMA_INT_STAT_4          0x00000300
#define DMA_INT_STAT_4_FINISH   0x00000100
#define DMA_INT_STAT_4_ERROR    0x00000200
#define DMA_INT_STAT_5          0x00000c00
#define DMA_INT_STAT_5_FINISH   0x00000400
#define DMA_INT_STAT_5_ERROR    0x00000800
#define DMA_INT_STAT_6          0x00003000
#define DMA_INT_STAT_6_FINISH   0x00001000
#define DMA_INT_STAT_6_ERROR    0x00002000
#define DMA_INT_STAT_7          0x0000c000
#define DMA_INT_STAT_7_FINISH   0x00004000
#define DMA_INT_STAT_7_ERROR    0x00008000
#endif

//----DMA Flag ID----//
#define FLAGDMA_INT_CH0   0x00000001
#define FLAGDMA_INT_CH1   0x00000002
#define FLAGDMA_INT_CH2   0x00000004
#define FLAGDMA_INT_CH3   0x00000008
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define FLAGDMA_INT_CH4   0x00000010
#define FLAGDMA_INT_CH5   0x00000020
#define FLAGDMA_INT_CH6   0x00000040
#define FLAGDMA_INT_CH7   0x00000080
#endif
#define DMA_CANCEL   1<<DMA_ID_NUM

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A)) //for DMA auto mode
#define FLAGDMA_AUTO_INT_CH0  0x00010000
#define FLAGDMA_AUTO_INT_CH1  0x00020000
#define FLAGDMA_AUTO_INT_CH2  0x00040000
#define FLAGDMA_AUTO_INT_CH3  0x00080000
#define FLAGDMA_AUTO_INT_CH4  0x00100000
#define FLAGDMA_AUTO_INT_CH5  0x00200000
#define FLAGDMA_AUTO_INT_CH6  0x00400000
#define FLAGDMA_AUTO_INT_CH7  0x00800000
#endif

//---DMA command---//
#define DMA_INT_ENA_FINISH  0x00000004
#define DMA_SINGLE          0x00000000
#define DMA_BURST4          0x00000008

#define DMA_HIGH_PRI        0x00000010
#define DMA_INT_ENA_ERROR   0x00000020

#define DMA_SRC_APB         0x00000000
#define DMA_SRC_AHB         0x00000040

#define DMA_SDRAM_CH1       0x00000010

#define DMA_DST_APB         0x00000000
#define DMA_DST_AHB         0x00000080

#define DMA_SRC_INC0        0x00000000
#define DMA_SRC_INC1        0x00000100
#define DMA_SRC_INC2        0x00000200
#define DMA_SRC_INC4        0x00000300

#define DMA_DST_INC0        0x00000000
#define DMA_DST_INC1        0x00001000
#define DMA_DST_INC2        0x00002000
#define DMA_DST_INC4        0x00003000

#define DMA_DATA_WORD       0x00000000
#define DMA_DATA_HALFWORD   0x00100000
#define DMA_DATA_BYTE       0x00200000

#define DMA_NONE            0x00000000
#define DMA_SMC             0x01000000
#define DMA_SD              0x02000000
#define DMA_USB             0x03000000
#define DMA_SPI_4           0x03000000
#define DMA_IIS_PLAY        0x04000000
#define DMA_IIS_REC         0x05000000
#define DMA_IIS2_PLAY       0x0D000000
#define DMA_IIS2_REC        0x0E000000
#if ((CHIP_OPTION == CHIP_A1018A)||(CHIP_OPTION == CHIP_A1018B))
#define DMA_IIS3_PLAY       0x08000000
#define DMA_IIS3_REC        0x0F000000
#define DMA_IIS4_PLAY       0x10000000
#define DMA_IIS4_REC        0x11000000
#define DMA_IIS5_PLAY       0x12000000
#define DMA_IIS5_REC        0x13000000
#endif
#define DMA_ADCRX_REC       0x06000000
#define DMA_SPI             0x07000000
#define DMA_CF				0x08000000
#define DMA_GPI				0x09000000

#define DMA_SPI_2           0x0a000000
#define DMA_SPI_3           0x0b000000

#define DMA_ADCRX_G1_REC    0x0C000000
#define DMA_ADCRX_G2_REC    0x0D000000
#define DMA_ADCRX_G3_REC    0x0E000000
#define DMA_SPI_5           0x0F000000
#define DMA_AUTOMODE_EN     0x20000000
#define DMA_INT_ENA_AUTOFIN 0x40000000
#define DMA_AUTONONSTOP_EN  0x80000000
#endif


/* channel request */
enum
{
   DMA_REQ_IIS_RECORD =0,
   DMA_REQ_IIS_PLAY,
#if ((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B)|| (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) ) // A1020DIFF1016
   DMA_REQ_IIS2_RECORD,
   DMA_REQ_IIS2_PLAY,
#endif
#if ((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
   DMA_REQ_IIS3_RECORD,
   DMA_REQ_IIS3_PLAY,
   DMA_REQ_IIS4_RECORD,
   DMA_REQ_IIS4_PLAY,
#endif
   DMA_REQ_SD_READ,        
   DMA_REQ_SD_WRITE,       
   DMA_REQ_SMC_READ,       
   DMA_REQ_SMC_WRITE,      
   DMA_REQ_USB_READ,       
   DMA_REQ_USB_WRITE,      
#if SWAP_SP1_SP3
   DMA_REQ_SPI0_READ,       
   DMA_REQ_SPI0_WRITE,      
#else
   DMA_REQ_SPI_READ,       
   DMA_REQ_SPI_READ_P,   
   DMA_REQ_SPI_WRITE,      
   DMA_REQ_SPI_WRITE_P,
#endif   
   DMA_REQ_MEM_COPY_BYTE,  
   DMA_REQ_MEM_COPY_WORD,  
   DMA_REQ_MEM_SET_BYTE,   
   DMA_REQ_MEM_SET_WORD,   
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
(CHIP_OPTION == CHIP_A1026A))
   DMA_REQ_CF_READ,        
   DMA_REQ_CF_WRITE,       
   DMA_REQ_GPI_READ,       
   DMA_REQ_GPI_WRITE,      
   DMA_REQ_SPI2_READ,       
   DMA_REQ_SPI2_READ_P,
   DMA_REQ_SPI2_WRITE, 
   DMA_REQ_SPI2_WRITE_P, 
   DMA_REQ_SPI3_READ,       
   DMA_REQ_SPI3_READ_P,
   DMA_REQ_SPI3_WRITE,  
   DMA_REQ_SPI3_WRITE_P,  

   DMA_REQ_ADC_G1_RECORD,  
   DMA_REQ_ADC_G2_RECORD,  
   DMA_REQ_ADC_G3_RECORD,  
   DMA_REQ_SPI5_READ,       
   DMA_REQ_SPI5_WRITE,  
   //--auto mode--//
   DMA_REQ_MEM_BLKCOPY_WORD_AUTO,
   DMA_REQ_MEM_BLKCOPY_BYTE_AUTO,
   //DMA_REQ_ADC_G0_REC_AUTO,
   //DMA_REQ_ADC_G1_REC_AUTO,
   //DMA_REQ_ADC_G2_REC_AUTO,
   //DMA_REQ_ADC_G3_REC_AUTO,   
#endif
   DMA_REQ_MAX 
};

#if SWAP_SP1_SP3
#define   DMA_REQ_SPI_READ   DMA_REQ_SPI3_READ       
#define  DMA_REQ_SPI_WRITE   DMA_REQ_SPI3_WRITE  
#endif  
#define DMA_INTSTAT_FINISH   0x00000001
#define DMA_INTSTAT_ERROR    0x00000002

#ifndef REGDMA_CFG
typedef struct _REGDMA_CFG {
    INT32U    src;
    INT32U    dst;
    INT32U    cnt;
    INT32U    cmmd;
} REGDMA_CFG;
#endif

#ifndef REGDMA_CFG_AUTO
typedef struct _REGDMA_CFG_AUTO {
    INT32U    src;
    INT32U    dst;
    INT32U    src_alt;
    INT32U    dst_alt;
    INT32U    cnt;
    INT32U    cmmd;
    INT32U    datacnt;
    INT32U    linecnt;
} REGDMA_CFG_AUTO;
#endif


//=================================================================
extern void marsDMAInit(void);
extern void marsDMAOpen(INT32U *pDMAId, FP_INT pEventHdl);
extern void marsDMAClose(INT32U uiDMAId);
extern INT32S marsDMACheckCycleCnt(INT32U uiDMAId);
extern INT32U marsDMAGetIntSta(INT32U uiDMAId);
extern INT32U marsDMAConfig(INT32U uiDMAId, REGDMA_CFG* pCfg);
extern INT32U marsDMAWaitForInt(INT32U uiDMAId);
extern INT32U marsDMAWaitForIntOrCancel(INT32U uiDMAId, INT16U Timeout, INT32U* CycleCnt);
extern INT32U marsDMACancel(void);
extern INT32U marsDMAClearCancelFlag(void);
extern void marsDMACloseReleaseSource(INT32U uiDMAId);


extern INT32U marsDMAReq(INT32U uiReqId, DMA_CFG* pCfg);
extern INT32U marsDMACheckReady(INT32U uiDMAId);
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
extern void isr_marsDMAAuto(int DmaID,u8* buf);
extern void isr_marsDMA_PlayAuto(int DmaID,u8* buf);
extern void isr_marsDMA_RecAuto(int DmaID,u8* buf);
extern INT32U marsDMAConfig_auto(INT32U uiDMAId, REGDMA_CFG_AUTO* pCfg);
extern INT32U marsDMAWaitForInt_auto(INT32U uiDMAId);
extern INT32U marsDMACheckReady_auto(INT32U uiDMAId);
#endif
extern void isr_marsDMA_StopAuto(INT32U uiDMAId);
#ifdef  TCM_DMA_SUPPORT
void TCM_DMAInit(void);
void TCM_Reset(void);
u32 TCM_DMA_SDRAM(void* pSrc, void* pDes, u32 lenth);
void TCM_DMAIntHandler(void);
#endif

//=================================================================

#endif    // __MARS_DMA_H__
