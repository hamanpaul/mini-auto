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
//#define __SPI_API_H__

#include "..\spi\inc\spi.h"

/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */

/* Debug Flag */
#define SPI_DEBUG_FLAG
#undef SPI_DEBUG_FLAG


/* SPI time out value */
#define SPI_INITED		1
#define SPI_UNINITED	0

#define SPI_TIMEOUT					40
#define SPI_MAX_BUF_SIZE			256

#if(SW_APPLICATION_OPTION == MR9670_DOORPHONE)
#define MAX_FRM_OBJ 500
#elif((UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_2MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_8MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2208p_16MB))
#define MAX_FRM_OBJ                         2208
#elif((UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_2MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_8MB) || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_2030p_16MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_2MB)    || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_4MB)||\
      (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_8MB)    || (UI_GRAPH_SIZE ==UI_GRAPH_SIZE_HA_16MB))
#define MAX_FRM_OBJ                         2030
#else
#define MAX_FRM_OBJ 1013
#endif

#define MAX_AUDIO_FRM_OBJ 16

#if (SYSTEM_CONFIG_ADDRESS_LIMIT == FIRMWARE_SIZE_1M)
#define FlASH_USING_END_ADDRESS	0x100000
#elif (SYSTEM_CONFIG_ADDRESS_LIMIT == FIRMWARE_SIZE_2M)
#define FlASH_USING_END_ADDRESS	0x200000
#elif (SYSTEM_CONFIG_ADDRESS_LIMIT == FIRMWARE_SIZE_8M)
#define FlASH_USING_END_ADDRESS	0x800000
#elif (SYSTEM_CONFIG_ADDRESS_LIMIT == FIRMWARE_SIZE_16M)
#define FlASH_USING_END_ADDRESS	0x1000000
#elif (SYSTEM_CONFIG_ADDRESS_LIMIT == FIRMWARE_SIZE_32M)
#define FlASH_USING_END_ADDRESS	0x2000000
#else
#define FlASH_USING_END_ADDRESS	0x200000	// Default: FIRMWARE_SIZE_2M
#endif


//#define SPI_UPDATE_MAX_CODE_SIZE	655104		/* max code size is 655104 Bytes, 655104 = 640K - 256 */
//#define SPI_RESERVED_CODE_BLOCK		655360
	#define SPI_BOOT_CODE_START_ADDR		0	/* 1024 KB */
	#define SPI_BOOT_CODE_END_ADDR		0x2000	/* 1024 KB */

	#define SPI_CODE_START_ADDR		0x2100	/*  The stored addr of boot code is started from the 4352 byte */

#if ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) ||\
     (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (UI_BIN_CODE_SIZE == UI_BIN_CODE_SIZE_1D5M))
    #define SPI_CODE_END_ADDR			0x180000	/* 1536 KB */
#else
    #define SPI_CODE_END_ADDR			0x100000	/* 1024 KB */
#endif

#define SPI_MAX_CODE_SIZE		(SPI_CODE_END_ADDR - SPI_CODE_START_ADDR)

#define SPI_SYS_PARA_AWB_SIZE			   		0x200 	/* 512B */
#define SPI_SYS_PARA_FILE_CNT_LIST_SIZE			0x100	/* 26 letters * 4 bytes (u32) */
#define SPI_SYS_PARA_UI_FB_SIZE					0x2000	/* MAX_FRM_OBJ * ((sizeof)FRAME_BUF_OBJECT) */
//#define SPI_SYS_PARA_RESERVED1_SIZE		   		0x400 	/* 1KB */
//#define SPI_SYS_PARA_RESERVED2_SIZE		   		0x400 	/* 1KB */
#define SPI_VERSION_ADDR        0x1F00

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)||\
    (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_HD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2)||\
    (SW_APPLICATION_OPTION == MR8211_IPCAM) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
    #define UI_LIB_PER_LANGUAGE_SIZE    0xC4D00        /* for SPI 2MB */
#endif

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
        #if ((HW_BOARD_OPTION == MR9200_RX_RDI_M1000))
        	#define UI_LIB_PER_LANGUAGE_SIZE   0xE6F000
        #elif ((UI_BIN_CODE_SIZE == UI_BIN_CODE_SIZE_1D5M) && (UI_VERSION == UI_VERSION_TRANWO))
            #define UI_LIB_PER_LANGUAGE_SIZE   0xE3DD00
    #else
        #define UI_LIB_PER_LANGUAGE_SIZE   0xEBDD00
    #endif
    #endif
#else
    /* Undefine UI_GRAPH_SIZE, Please Check Project.h */
#endif

#if MARSS_SUPPORT
#define MARSS_SYS_SIZE 512 // 512 is Marss sys data size; 256 is caild sensor size
#define MARSS_SENSOR_SIZE 256 // 512 is Marss sys data size; 256 is caild sensor size
#endif

#if(HOME_RF_SUPPORT)
#define SPI_HOMERF_RODATA_SIZE              0x1000   /*4KB*/
#define SPI_HOMERF_CONFIG_SIZE              0x1000   /*4KB*/
#define SPI_HOMERF_ROOM_SIZE                0x2000   /*8KB*/
#define SPI_HOMERF_SCENE_SIZE               0x2000   /*8KB*/
#define SPI_HOMERF_SENSOR_SIZE              0x8000   /*32KB*/
#define SPI_HOMERF_TOTOAL_SIZE              (0x10000) /*64KB*/
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

#define SPI_DEFECT_PIXEL_SIZE		0x4000	/* 16 KB */

#define SPI_UI_LIB_LANGUAGE_COUNT	UI_LIB_LANGUAGE_NUM	    /* supported language types */
#define SPI_UI_LIB_EACH_SIZE        UI_LIB_PER_LANGUAGE_SIZE
#define SPI_UI_AUDIO_LIB_SIZE       UI_AUDIO_LIB_SIZE
#define SPI_UI_CONFIG_SIZE          UI_CONFIG_TOTAL_SIZE
#if UI_ICONFLAG_BACKUP
#define SPI_UI_CONFIG_BACKUP_SIZE   SPI_UI_CONFIG_SIZE
#endif
#define SPI_UI_NETWORK_INFO_SIZE         0x1000
#define SPI_UI_RF_INFO_SIZE              0x1000
#define SPI_UI_RF_CAL_SIZE               0x1000 

#define SPI_BUF_SIZE                0x8000   /* 4KB*/

#define	MAX_WAVE_RING_COUNT		8

/*
*	Wave File For Ring
*/
#define	SPI_WAVE_FILE_COUNT		8
#define	SPI_WAVE_EACH_FILE_LENGTH	0x3C000		/* Mono quality, 30 Sec length, 8KHz sampling rate; FileLength = 8K*1(Byte)*30 = 240 KBytes */
#define SPI_MAX_WAVE_FILE_COUNT		0x0A
#define	SPI_WAVE_FILE_HEADSIZE		44

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
extern u32 unAddrForTest;

extern u32 spiTotalSize;
extern u32 spiBlockSize;
extern u32 spiSectorSize;
extern u32 spiPageSize;
extern u8	spiManufID, spiDevID;
extern u8	spiInitTable;
extern u8	spiInitIdx;

extern u32 spiCodeStartAddr;
extern u32 spiSysParaFileCntListStartAddr;
extern u32 spiSysParaUIFBStartAddr;
extern u32 spiSysParaAWBStartAddr;
extern u32 spiSysParaRsvdStartAddr1;
extern u32 spiSysParaRsvdStartAddr2;
extern u32 spiDefectPixelStartAddr;
extern u32 spiUILibStartAddr;
extern u32 spiSysParaWaveRingObjStartAddr;
extern u32 spiNetwrokStartAddr;


/* Driver Function */
extern s32 spiSemProcess(u8, u8);
extern void spiIntHandler(void);
extern void spiInit(void);
extern s32 spiSetReadDataDma(u8*, u32);
extern s32 spiSetWriteDataDma(u8*, u32);
extern s32 spiCheckDmaComplete(void);
extern s32 spiRead(u8*, u32, u32);
extern u32 spiReadID(void);
extern void spiReadRF_ID(void);

extern void spiReadNet(void);

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
extern s32 spiWriteRF(s32 dummy);

extern void spiReleaseFlashStatus(void);
/* API Function */
extern s32 spiArrangeSpace(void);
extern s32 spiStart(void);
extern int spiFirmwareCodeUpdate(u8 *pSrcCodeAddr, u32 CodeLen);
extern s32 spiCodeUpdate(u8*, u32);
extern s32 spiCmpWriteData(u8*, u32);
extern void spiReadFBSetting(void);
extern void spiReadUI(void);
extern void spiReadVersion(void);
void spiReadData(u8 *targetBuf, u32 StartAddr, u32 unLen );
extern s32 spiWriteUI(void);
extern s32 spiGet_UI_FB_Index(u8* targetStr);
extern u8 spiUI_OpenFB_ByIndex(s32 fb_index);
extern u8 spiUI_OpenFB_ByIndexToSpecificAddr(u8	*, s32);
extern s32 spiWriteHomeRF(u8 typeIdx);
extern void spiReadHomeRF(u8 typeIdx);
extern s32 spiWriteNet(void);

#if UI_ICONFLAG_BACKUP
extern void spiReadUIBackup(void);
extern s32 spiWriteUIBackup(void);
#endif

#if MARSS_SUPPORT
extern void spiWriteA7128Test(void);
extern void spiReadA7128Test(void);
#endif

#if ADAPTIVE_AE_BY_ADC
extern int spiReadAEData(u8 *targetBuf, u32 Length);
extern int spiWriteAEData(u8 *targetBuf, u32 Length);
#endif


extern void spiBlkSemProcess(void);
extern void spiReleSemProcess(void);

#endif

