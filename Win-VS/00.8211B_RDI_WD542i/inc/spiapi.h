/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	spiapi.h

Abstract:

   	The application interface of the SPI controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2008/04/17	Chi-Lun Chen	Create	

*/

#ifndef __SPI_API_H__
#define __SPI_API_H__

#include "..\spi\inc\spi.h"

/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */

/* Debug Flag */
//#define SPI_DEBUG_FLAG
//#undef	SPI_DEBUG_FLAG


/* SPI time out value */
#define SPI_INITED		                    1
#define SPI_UNINITED	                    0

#define SPI_TIMEOUT					        40
#define SPI_MAX_BUF_SIZE		            256

#if(SW_APPLICATION_OPTION == MR9670_DOORPHONE)
#define MAX_FRM_OBJ                         500
#elif((UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_2MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_8MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_16MB))
#define MAX_FRM_OBJ                         2208 
#elif((UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_2MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_8MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_16MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_2MB)    || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_8MB)    || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_16MB))
#define MAX_FRM_OBJ                         2030   
#else
#define MAX_FRM_OBJ                         1013
#endif

#define MAX_AUDIO_FRM_OBJ                   16


#define SPI_BOOT_CODE_START_ADDR            0     /* 1024 KB */
#define SPI_VERSION_ADDR                    0x1F00
#define SPI_BOOT_CODE_END_ADDR		        0x2000	/* 1024 KB */
#define SPI_CODE_START_ADDR		            0x2100	   /*  The stored addr of boot code is started from the 4352 byte */
#define SPI_CODE_END_ADDR			        0x100000   /* 1024 KB */


#define SPI_MAX_CODE_SIZE		            SPI_CODE_END_ADDR - SPI_CODE_START_ADDR
#define SPI_SYS_PARA_FILE_CNT_LIST_SIZE	    0x100    /* 26 letters * 4 bytes (u32) */

#if(UI_GRAPH_SIZE != UI_GRAPH_SIZE_NONE)
    #if(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2MB)
        #define UI_LIB_PER_LANGUAGE_SIZE   0xC4D00  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_4MB)
        #define UI_LIB_PER_LANGUAGE_SIZE   0x2C4D00  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_8MB)    
        #define UI_LIB_PER_LANGUAGE_SIZE   0x6C4D00  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_16MB)    
        #define UI_LIB_PER_LANGUAGE_SIZE   0xEC4D00  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_HA_2MB)    
        #define UI_LIB_PER_LANGUAGE_SIZE   0xA4000  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_HA_4MB)    
        #define UI_LIB_PER_LANGUAGE_SIZE   0x2A4000  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_HA_8MB)    
        #define UI_LIB_PER_LANGUAGE_SIZE   0x6A4000  
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_HA_16MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0xEA4000
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2030p_2MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0xBED00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2030p_4MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0x2BED00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2030p_8MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0x6BED00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2030p_16MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0xEBED00        
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2208p_2MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0xBDD00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2208p_4MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0x2BDD00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2208p_8MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0x6BDD00
    #elif(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2208p_16MB)   
        #define UI_LIB_PER_LANGUAGE_SIZE   0xEBDD00  
    #endif
#endif



#if(HOME_RF_SUPPORT)
#define SPI_HOMERF_RODATA_SIZE              0x1000   /*4KB*/
#define SPI_HOMERF_CONFIG_SIZE              0x1000   /*4KB*/
#define SPI_HOMERF_ROOM_SIZE                0x2000   /*8KB*/
#define SPI_HOMERF_SCENE_SIZE               0x2000   /*8KB*/
#define SPI_HOMERF_SENSOR_SIZE              0x8000   /*32KB*/
#define SPI_HOMERF_TOTOAL_SIZE              (0x10000)+(0xAD00)  /*64KB*/
#define SPI_HOMERF_RESERVE_SIZE             (SPI_HOMERF_TOTOAL_SIZE) - (SPI_HOMERF_RODATA_SIZE) - (SPI_HOMERF_CONFIG_SIZE) -\
                                            (SPI_HOMERF_ROOM_SIZE) - (SPI_HOMERF_SCENE_SIZE) - (SPI_HOMERF_SENSOR_SIZE)

enum
{
    SPI_HOMERF_RODATA=0,
    SPI_HOMERF_CONFIG,
    SPI_HOMERF_ROOM,
    SPI_HOMERF_SCENE,
    SPI_HOMERF_SENSOR,
};


#endif

#define SPI_UI_LIB_EACH_SIZE                UI_LIB_PER_LANGUAGE_SIZE
#define SPI_UI_AUDIO_LIB_SIZE               UI_AUDIO_LIB_SIZE
#define SPI_UI_CONFIG_SIZE                  0x1000
#if UI_ICONFLAG_BACKUP
#define SPI_UI_CONFIG_BACKUP_SIZE   SPI_UI_CONFIG_SIZE
#endif
#define SPI_UI_NETWORK_INFO_SIZE            0x1000
#define SPI_UI_RF_INFO_SIZE                 0x1000
#define SPI_BUF_SIZE                        0x8000   /* 32KB*/


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
extern __align(4) u8	spiReadBuf[SPI_MAX_BUF_SIZE];
extern __align(4) u8	spiWriteBuf[SPI_MAX_BUF_SIZE];

extern u32 	spiClkDiv;
extern u8	cnt;
extern u8	ucSpiTestResult[8];
extern u32  unAddrForTest;

extern u32  spiTotalSize;
extern u32  spiBlockSize;
extern u32  spiSectorSize;
extern u32  spiPageSize;
extern u8	spiManufID, spiDevID;
extern u8	spiInitTable;
extern u8	spiInitIdx;

extern u32  spiCodeStartAddr;
extern u32  spiSysParaFileCntListStartAddr;
extern u32  spiSysParaUIFBStartAddr;
extern u32  spiSysParaAWBStartAddr;
extern u32  spiSysParaRsvdStartAddr1;
extern u32  spiSysParaRsvdStartAddr2;
extern u32  spiDefectPixelStartAddr;
extern u32  spiUILibStartAddr;
extern u32  spiSysParaWaveRingObjStartAddr;
extern u32  spiNetwrokStartAddr;



/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */

/* Driver Function */
extern s32 spiSemProcess(u8, u8);
extern void spiIntHandler(void);
extern void spiInit(void);
extern s32 spiSetReadDataDma(u8*, u32);
extern s32 spiSetWriteDataDma(u8*, u32);
extern s32 spiCheckDmaComplete(void);
extern s32 spiRead(u8*, u32, u32);
extern u32 spiReadID(void);
extern s32 spiIdentification(void);
extern s32 spiMount(void);
extern s32 spiWREN(void);
extern s32 spiWRDI(void);
extern u8 spiHWWDEN(void);
extern u8 spiHWWDDISA(void);
extern u8 spiRDSR(void);
extern s32 spiSectorErase(u32);
extern s32 spiBlockErase(u32);
extern s32 spiChipErase(void);
extern s32 spiWrite(u32, u8*, u32);
extern s32 spiByteProgram(u8, u32);
extern void spiVerify(void);



/* API Function */
extern s32 spiArrangeSpace(void);
extern s32 spiStart(void);
extern s32 spiCodeUpdate(u8*, u32);
extern s32 spiCmpWriteData(u8*, u32);
extern void spiReadFBSetting(void);
extern void spiReadUI(void);
extern void spiReadVersion(void);
extern void spiReadNet(void);
extern void spiReadRF(void);

void spiReadData(u8 *targetBuf, u32 StartAddr, u32 unLen );
extern s32 spiWriteUI(void);
extern s32 spiGet_UI_FB_Index(u8* targetStr);
extern u8 spiUI_OpenFB_ByIndex(s32 fb_index);
extern u8 spiUI_OpenFB_ByIndexToSpecificAddr(u8	*, s32);
extern s32 spiWriteRF(void);
extern s32 spiWriteHomeRF(u8 typeIdx);
extern void spiReadHomeRF(u8 typeIdx);
#if UI_ICONFLAG_BACKUP
extern void spiReadUIBackup(void);
extern s32 spiWriteUIBackup(void);
#endif

#endif

