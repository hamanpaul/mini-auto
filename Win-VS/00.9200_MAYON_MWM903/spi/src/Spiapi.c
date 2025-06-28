/*

Copyright (c) 2005	MARS Semiconductor Corp

Module Name:

	smc.c

Abstract:

	The API Functions of Serial flash.

Environment:

		ARM RealView Developer Suite

Revision History:

	2008/12/31	Chi-Lun Chen	Create

*/

#include "general.h"
#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_EON)|| (FLASH_OPTION == FLASH_SERIAL_ESMT))
#include "board.h"
#include "osapi.h"
#include "spi.h"
#include "spireg.h"
#include "dmaapi.h"

#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "stdio.h"
#include "gpioapi.h"
#include "spiapi.h"
#include "uiapi.h"
#include "sysapi.h"
#include "asfapi.h"
#include "uiapi.h"
#include "uiKey.h"
#include "MotionDetect_API.h"

#ifdef NEW_UI_ARCHITECTURE
#include "..\..\ui\inc\SpiNor.h"
#endif
#define SPI_DEBUG_FLAG 1
/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
u32 spiCodeUpgradepercentage;
FRAME_BUF_OBJECT frame_buf_obj[MAX_FRM_OBJ];
UI_FILE_CHAR_COUNT	uiFileListCharCount[26];	/* UI Lib updated files, the first character of files count list */

u32 spiCodeStartAddr;
u32 spiSysParaFileCntListStartAddr;
u32 spiSysParaAudioFileCntListStartAddr;
u32 spiSysParaUIFBStartAddr;
u32 spiSysParaAudioFBStartAddr;
u32 spiSysParaAWBStartAddr;
u32 spiSysParaRsvdStartAddr1;
u32 spiSysParaRsvdStartAddr2;
u32 spiDefectPixelStartAddr;
u32 spiUIConfigStartAddr;
u32 spiUIConfigBackupStartAddr;
u32 spiUILibStartAddr;
u32 spiFirmwareEndAddr;
u32 spiAudioLibStartAddr;
u32 spiPIRDataStartAddr;
u32 spiNetwrokStartAddr;
u32 spiRFIDStartAddr;
u32 spiRFCalStartAddr;
u32 spiModelParaAddr;
u32 spiSysParaWaveRingObjStartAddr;
u32	spiWavStartAddr;
u8  spiInitTable = SPI_UNINITED;
u8  spiInitIdx = SPI_UNINITED;

u32 spiMarssSysStartAddr;
u32 spiMarssSensorStartAddr;
void spiWriteA7128Test(void);
void spiReadA7128Test(void);
u32 spiHomeRFROStartAddr;
u32 spiHomeRFConfigStartAddr;
u32 spiHomeRFRoomStartAddr;
u32 spiHomeRFSceneStartAddr;
u32 spiHomeRFSensorStartAddr;

#if UI_PIC_LIST_DYNAMIC_SUPPORT
u32 spiPicDetialNum;
u32 spiPicObjAddress;
#endif

extern FRAME_BUF_OBJECT frame_buf_obj[];
extern u8 iconflag[UIACTIONNUM];
extern u32 data_size;
#if ICOMMWIFI_SUPPORT
extern u8 *icommbuf4;
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

u32 spiGetFirmwareUpdatePercentage(void)
{
	return spiCodeUpgradepercentage;
}

void spiSetFirmwareUpdatePercentage(u32 Percent)
{
	spiCodeUpgradepercentage = Percent;
}

void spiExecuteWDTCntReset(void)
{
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
	if(sysGetFWUpgradeStatus())
		return;	// Blk the WDT when firmware is updating.
#endif
	WDT_Reset_Count();
}


/****************************************

	Middle Ware Function

****************************************/
#if ICOMMWIFI_SUPPORT

#if 1
static void conver_endianness_tx(u8 *dstbuf,u8 *srcbuf,u32 unSize)
{
	u8 *src_addr;
    u8 *dst_addr;
    u32 a;

	if((unSize%4)!= 0)
		unSize+=4-(unSize%4);

	dst_addr=dstbuf;
    src_addr=srcbuf;

	for(;(int)unSize>0;)
	{
	    a=*(u32 *)src_addr;
		*(u32 *)dst_addr=( (a>>24) & 0xff) |        // move byte 3 to byte 0
					     ( (a<<8)  & 0xff0000 ) |   // move byte 1 to byte 2
					     ( (a>>8)  & 0xff00) |      // move byte 2 to byte 1
					     ( (a<<24) & 0xff000000);   // byte 0 to byte 3

		(int)unSize=(int)unSize-4;			   
		src_addr=src_addr+4;
        dst_addr=dst_addr+4;
	}			
}

#else
static void conver_endianness_tx(u8 *buf,u32 unSize)
{
	u8 *tmp_addr;
    u32 a;

	if((unSize%4)!= 0)
		unSize+=4-(unSize%4);

	tmp_addr=buf;	

	for(;(int)unSize>0;)
	{
	    a=*(u32 *)tmp_addr;
		*(u32 *)tmp_addr=( (a>>24) & 0xff) |        // move byte 3 to byte 0
					     ( (a<<8)  & 0xff0000 ) |   // move byte 1 to byte 2
					     ( (a>>8)  & 0xff00) |      // move byte 2 to byte 1
					     ( (a<<24) & 0xff000000);   // byte 0 to byte 3

		(int)unSize=(int)unSize-4;			   
		tmp_addr=tmp_addr+4;
	}			
}
#endif

static void conver_endianness_rx(u8 *buf,u32 unSize)
{
	u8 *tmp_addr;
    u32 a;
    
	tmp_addr=buf;
	
	for(;(int)unSize>0;)
	{
	    a=*(u32 *)tmp_addr;
		*(u32 *)tmp_addr=((a>>24) & 0xff) | // move byte 3 to byte 0
					     ((a<<8) & 0xff0000) | // move byte 1 to byte 2
					     ((a>>8) & 0xff00) | // move byte 2 to byte 1
					     ((a<<24) & 0xff000000); // byte 0 to byte 3

		(int)unSize=(int)unSize-4;			   
		tmp_addr=tmp_addr+4;
	}			
}

s32 spi_master_out(u8* pucSrc, u32 unSize)
{
	bool ret=TRUE;
    int i;

#if PWIFI_SUPPORT
	if( ((u32)pucSrc & 0x03) || (unSize & 0x03))
	{
	    memcpy_hw(icommbuf4,pucSrc,unSize); //20171108 Sean: MUST 4-bytes align.
	    conver_endianness_tx(icommbuf4,icommbuf4,(unSize-(unSize%4)));
	    ret = spiWrite_Icomm_DMA(icommbuf4,(unSize));	//Sean: CAN NOT delay inside func. Will deadlock.
    }
    else
    {
       //for(i=0;i<0xffff;i++);
	   conver_endianness_tx(icommbuf4,pucSrc,(unSize-(unSize%4)));
	   ret = spiWrite_Icomm_DMA(icommbuf4,(unSize));	//Sean: CAN NOT delay inside func. Will deadlock.
    }   

#else
	memcpy_hw(icommbuf4,pucSrc,unSize); //20171108 Sean: MUST 4-bytes align.
	conver_endianness_tx(icommbuf4,icommbuf4,(unSize-(unSize%4)));
	ret = spiWrite_Icomm_DMA(icommbuf4,(unSize));	//Sean: CAN NOT delay inside func. Will deadlock.
#endif	
	return ret;
}


s32 spi_master_in(u8 cmd ,u8* pucDstBuf, u32 unSize)
{
	bool ret = TRUE;

	ret = spiRead_Icomm_DMA(cmd,pucDstBuf,unSize);	//Sean: CAN NOT delay inside func. Will deadlock.
	if(ret == 1)
		conver_endianness_rx(pucDstBuf,unSize);
	return ret;
}

s32 spi_master_out_cpu(u8* pucSrc, u32 unSize)
{
	bool ret = TRUE;
		
	ret = spiWrite_Icomm_cpu(pucSrc, unSize);  //CPU mode
	
	return ret;
}

s32 spi_master_in_cpu(u8 spi_cmd,u8* pucDstBuf, u32 unSize)
{
	bool ret = TRUE;
		
	ret = spiRead_Icomm_cpu(pucDstBuf,unSize);

	return ret;
}
#endif
/*

Routine Description:

	Serial flash mount detection.

Arguments:

	None.

Return Value:

	1 - successful
	0 - failed

*/
s32 spiMount(void)
{
    return spiIdentification();
}
/*

Routine Description:

	Arrange Serial Flash Space.

Arguments:

	None

Return Value:

	None

*/

u32 spiAddressLocate(u32 srcAddr, s32 operationStatus)
{
    switch(operationStatus)
    {
        case SPI_AREA_BOOT_CODE:
            if(SPI_BOOT_CODE_START_ADDR <= srcAddr && srcAddr < SPI_BOOT_CODE_END_ADDR)
                return 1;
            break;
        case SPI_AREA_MAIN_CODE:
            if(SPI_BOOT_CODE_END_ADDR <= srcAddr && srcAddr < SPI_CODE_END_ADDR)
                return 1;
            break;
        case SPI_AREA_CODE:
            if(SPI_BOOT_CODE_START_ADDR <= srcAddr && srcAddr < SPI_CODE_END_ADDR)
                return 1;
            break;
            
        case SPI_AREA_UI_PICS:
            if(SPI_CODE_END_ADDR <= srcAddr && srcAddr < spiFirmwareEndAddr)
                return 1;
            break;

		case SPI_AREA_PIR_SENSOR:
            if(spiPIRDataStartAddr <= srcAddr && srcAddr < spiMarssSensorStartAddr)
                return 1;
            break;

        case SPI_AREA_MARS_SENSOR:
        	if(spiMarssSensorStartAddr <= srcAddr && srcAddr < spiHomeRFROStartAddr)
                return 1;
        	break;
            
        case SPI_AREA_HA:
            if(spiHomeRFROStartAddr <= srcAddr && srcAddr < spiUIConfigBackupStartAddr)
                return 1;
            break;
            
        case SPI_AREA_UI_BACKUP:
            if(spiUIConfigBackupStartAddr <= srcAddr && srcAddr < spiNetwrokStartAddr)
                return 1;
            break;
		
        case SPI_AREA_NETWORK:
            if(spiNetwrokStartAddr <= srcAddr && srcAddr < spiRFIDStartAddr)
                return 1;
            break;
            
        case SPI_AREA_RFID:
            if(spiRFIDStartAddr <= srcAddr && srcAddr < spiUIConfigStartAddr)
                return 1;
            break;
                        
        case SPI_AREA_UI_CONFIG:
            if(spiUIConfigStartAddr <= srcAddr)
                return 1;
            break;
            
        case SPI_AREA_BMP:
            if(SPI_BOOT_CODE_START_ADDR <= srcAddr && srcAddr < spiFirmwareEndAddr)
                return 1;
                
            break;
        case SPI_AREA_ALL:
            return 1;

        default:
            break;
    }
    DEBUG_SPI("[Warming]: Wrong SPI flash area operate.\n" \
              "Status: %#x, unAddr: %#x\n", operationStatus, srcAddr);
    return 0;
}

#if SPI_NOR_FLASH_SETTING_ALIGNMENT
s32 spiArrangeSpace(void)
{
    u32 unSize, unAddr = 0, BackEndAddr;

	////////////////////////////////////////////////////////////////////
	// Front to End
    unAddr += SPI_CODE_START_ADDR;

    spiCodeStartAddr = unAddr;												// The first 256 byte stores code length
    unAddr += SPI_MAX_CODE_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;	// page alignemnt

    // Sector alognment for UI Lib update convenience
    spiSysParaFileCntListStartAddr = unAddr;
    unAddr += SPI_SYS_PARA_FILE_CNT_LIST_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;	// page alignemnt

    spiSysParaUIFBStartAddr = unAddr;
#if UI_PIC_LIST_DYNAMIC_SUPPORT
    unSize = sizeof(FRAME_BUF_OBJECT) * spiPicDetialNum;
    unSize = (unSize + spiPageSize) - (unSize & (spiPageSize - 1));

    if((unAddr + unSize) != spiPicObjAddress)
    {
        DEBUG_SPI("spiPicDetialNum: %x, %x\n", spiPicDetialNum, spiPicObjAddress);
        DEBUG_SPI("[W] SPI UI_PIC_LIST_DYNAMIC_SUPPORT info error\n");
        unSize = sizeof(FRAME_BUF_OBJECT) * MAX_FRM_OBJ;
    }
    unAddr += unSize;
#else
    unSize = sizeof(FRAME_BUF_OBJECT) * MAX_FRM_OBJ;
    unAddr += unSize;
#endif
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;	// page alignemnt

    spiUILibStartAddr = unAddr;
    unAddr += SPI_UI_LIB_EACH_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;	// page alignemnt
    
	spiFirmwareEndAddr = unAddr;

	////////////////////////////////////////////////////////////////////
    // Back to begin
    BackEndAddr = FlASH_USING_END_ADDRESS;

    // UI setting value
    BackEndAddr -= SPI_UI_CONFIG_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiUIConfigStartAddr = BackEndAddr;

    BackEndAddr -= SPI_UI_RF_INFO_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiRFIDStartAddr = BackEndAddr;

    BackEndAddr -= SPI_UI_NETWORK_INFO_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiNetwrokStartAddr = BackEndAddr;

#if UI_ICONFLAG_BACKUP
    BackEndAddr -= SPI_UI_CONFIG_BACKUP_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
#endif
    spiUIConfigBackupStartAddr = BackEndAddr;

#if HOME_RF_SUPPORT
    BackEndAddr -= SPI_HOMERF_RESERVE_SIZE;
    BackEndAddr -= SPI_HOMERF_SENSOR_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiHomeRFSensorStartAddr = BackEndAddr;

    BackEndAddr -= SPI_HOMERF_SCENE_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiHomeRFSceneStartAddr = BackEndAddr;

    BackEndAddr -= SPI_HOMERF_ROOM_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiHomeRFRoomStartAddr = BackEndAddr;

    BackEndAddr -= SPI_HOMERF_CONFIG_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiHomeRFConfigStartAddr = BackEndAddr;

    BackEndAddr -= SPI_HOMERF_RODATA_SIZE;
    BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
    spiHomeRFROStartAddr = BackEndAddr;
#else
	spiHomeRFSensorStartAddr = BackEndAddr;
	spiHomeRFSceneStartAddr = BackEndAddr;
	spiHomeRFRoomStartAddr = BackEndAddr;
	spiHomeRFConfigStartAddr = BackEndAddr;
	spiHomeRFROStartAddr = BackEndAddr;
#endif

#if MARSS_SUPPORT
	BackEndAddr -= MARSS_SYS_SIZE;
	BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
	spiMarssSysStartAddr = BackEndAddr;
	BackEndAddr -= MARSS_SENSOR_SIZE;
	BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
	spiMarssSensorStartAddr = BackEndAddr;
#else
	spiMarssSysStartAddr = BackEndAddr;
	spiMarssSensorStartAddr = BackEndAddr;
#endif

#if ADAPTIVE_AE_BY_ADC
	BackEndAddr -= SPI_BUF_SIZE;
	BackEndAddr = (BackEndAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
#endif
	spiPIRDataStartAddr = BackEndAddr;

    if(unAddr > BackEndAddr)
    {
        DEBUG_SPI("Flash space isn't enough for arrangement.\n");
        return 0;
    }
#ifdef SPI_DEBUG_FLAG
    else
        DEBUG_SPI("Residual Free Space is %d KBytes\n", ((BackEndAddr - unAddr) >> 10));

    DEBUG_SPI("spiCodeStartAddr = %#x\n", spiCodeStartAddr);
    DEBUG_SPI("spiSysParaAWBStartAddr = %#x\n", spiSysParaAWBStartAddr);
    DEBUG_SPI("spiSysParaRsvdStartAddr1 = %#x\n", spiSysParaRsvdStartAddr1);
    DEBUG_SPI("spiSysParaRsvdStartAddr2 = %#x\n", spiSysParaRsvdStartAddr2);
    DEBUG_SPI("spiDefectPixelStartAddr = %#x\n", spiDefectPixelStartAddr);
    DEBUG_SPI("spiSysParaFileCntListStartAddr = %#x\n", spiSysParaFileCntListStartAddr);
    DEBUG_SPI("spiSysParaUIFBStartAddr = %#x\n", spiSysParaUIFBStartAddr);
    DEBUG_SPI("spiUILibStartAddr = %#x\n", spiUILibStartAddr);
    DEBUG_SPI("spiFirmwareEndAddr = %#x\n", spiFirmwareEndAddr);
	DEBUG_SPI("spiPIRDataStartAddr = %#x\n", spiPIRDataStartAddr);
    DEBUG_SPI("spiMarssSensorStartAddr = %#x\n", spiMarssSensorStartAddr);
    DEBUG_SPI("spiMarssSysStartAddr = %#x\n", spiMarssSysStartAddr);
    DEBUG_SPI("spiHomeRFROStartAddr = %#x\n", spiHomeRFROStartAddr);
    DEBUG_SPI("spiHomeRFConfigStartAddr = %#x\n", spiHomeRFConfigStartAddr);
    DEBUG_SPI("spiHomeRFRoomStartAddr = %#x\n", spiHomeRFRoomStartAddr);
    DEBUG_SPI("spiHomeRFSceneStartAddr = %#x\n", spiHomeRFSceneStartAddr);
    DEBUG_SPI("spiHomeRFSensorStartAddr = %#x\n", spiHomeRFSensorStartAddr);
	DEBUG_SPI("spiUIConfigBackupStartAddr = %#x\n",spiUIConfigBackupStartAddr);
    DEBUG_SPI("spiNetwrokStartAddr = %#x\n", spiNetwrokStartAddr);
    DEBUG_SPI("spiRFIDStartAddr = %#x\n", spiRFIDStartAddr);
    DEBUG_SPI("spiUIConfigStartAddr = %#x\n",spiUIConfigStartAddr);
#endif
    return 1;
}
#else
s32 spiArrangeSpace(void)
{
	u32 unAddr = 0;
    u32 unSize;

    unAddr += SPI_CODE_START_ADDR;

    spiCodeStartAddr = unAddr;		// The first 256 byte stores code length
    unAddr += SPI_MAX_CODE_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    // Sector alognment for UI Lib update convenience
    spiSysParaFileCntListStartAddr = unAddr;
    unAddr += SPI_SYS_PARA_FILE_CNT_LIST_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    spiSysParaUIFBStartAddr = unAddr;
    unSize = sizeof(FRAME_BUF_OBJECT) * MAX_FRM_OBJ;
    unAddr += unSize;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    spiUILibStartAddr = unAddr;
    unAddr += SPI_UI_LIB_EACH_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

	spiSysParaAudioFileCntListStartAddr = unAddr;
    unAddr += SPI_SYS_PARA_FILE_CNT_LIST_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    spiSysParaAudioFBStartAddr = unAddr;
    unSize = sizeof(FRAME_BUF_OBJECT) * MAX_AUDIO_FRM_OBJ;
    unAddr += unSize;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt


	// Steal some space from SPI_UI_AUDIO_LIB_SIZE
	unSize = 0;	// UI_ICONFLAG_BACKUP ignored.
#if ADAPTIVE_AE_BY_ADC
	unSize += SPI_BUF_SIZE;
#endif	
#if MARSS_SUPPORT
//	unSize += SPI_BUF_SIZE * 2;

    unSize += SPI_HOMERF_SENSOR_SIZE;
#endif
#if HOME_RF_SUPPORT
	unSize += SPI_HOMERF_RODATA_SIZE + 
			SPI_HOMERF_CONFIG_SIZE + 
			SPI_HOMERF_ROOM_SIZE + 
			SPI_HOMERF_SCENE_SIZE + 
			SPI_HOMERF_RESERVE_SIZE + 
			SPI_HOMERF_SENSOR_SIZE;
#endif

	if(SPI_UI_AUDIO_LIB_SIZE < unSize)
	{
		DEBUG_SPI("SPI_UI_AUDIO_LIB_SIZE is not enough for arrangement. (%#x)\n", unSize);
		return 0;
	}
	
	spiAudioLibStartAddr = unAddr;
    unAddr += SPI_UI_AUDIO_LIB_SIZE - unSize;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

	// Firmware END
    spiFirmwareEndAddr = unAddr;

	// CONFIG AREA
#if ADAPTIVE_AE_BY_ADC
	spiPIRDataStartAddr = unAddr;
	unAddr += SPI_BUF_SIZE;
	unAddr = (unAddr / spiSectorSize) * spiSectorSize;		// Sector alignemnt
#else
	spiPIRDataStartAddr = unAddr;
#endif	

#if MARSS_SUPPORT
	spiMarssSensorStartAddr = unAddr;
    unAddr += MARSS_SENSOR_SIZE;
    unAddr = ((unAddr + (spiSectorSize - 1)) / spiSectorSize) * spiSectorSize;		// Sector alignemnt

    spiMarssSysStartAddr = unAddr;
    unAddr += MARSS_SYS_SIZE;
    unAddr = ((unAddr + (spiSectorSize - 1)) / spiSectorSize) * spiSectorSize;		// Sector alignemnt   
#else
	spiMarssSensorStartAddr = unAddr;
	spiMarssSysStartAddr = unAddr;
#endif

#if HOME_RF_SUPPORT
	spiHomeRFROStartAddr = unAddr;
	unAddr += SPI_HOMERF_RODATA_SIZE;
	unAddr = (unAddr / spiPageSize) * spiPageSize;		// page alignemnt

	spiHomeRFConfigStartAddr = unAddr;
	unAddr += SPI_HOMERF_CONFIG_SIZE;
    unAddr = (unAddr / spiPageSize) * spiPageSize;		// page alignemnt

	spiHomeRFRoomStartAddr = unAddr;
    unAddr += SPI_HOMERF_ROOM_SIZE;
    unAddr = (unAddr / spiPageSize) * spiPageSize;		// page alignemnt

    spiHomeRFSceneStartAddr = unAddr;
	unAddr += SPI_HOMERF_SCENE_SIZE;
    unAddr = (unAddr / spiPageSize) * spiPageSize;		// page alignemnt
    
	spiHomeRFSensorStartAddr = unAddr;
    unAddr += SPI_HOMERF_RESERVE_SIZE;
    unAddr += SPI_HOMERF_SENSOR_SIZE;
    unAddr = (unAddr / spiPageSize) * spiPageSize;		// page alignemnt
#else
	spiHomeRFROStartAddr = unAddr;
	spiHomeRFConfigStartAddr = unAddr;
	spiHomeRFRoomStartAddr = unAddr;
	spiHomeRFSceneStartAddr = unAddr;
	spiHomeRFSensorStartAddr = unAddr;
#endif

#if UI_ICONFLAG_BACKUP
    spiUIConfigBackupStartAddr = unAddr;
    unAddr += SPI_UI_CONFIG_BACKUP_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt
#else
	spiUIConfigBackupStartAddr = unAddr;
#endif

	spiNetwrokStartAddr = unAddr;
    unAddr += SPI_UI_NETWORK_INFO_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    spiRFIDStartAddr = unAddr;
    unAddr += SPI_UI_RF_INFO_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

    spiUIConfigStartAddr = unAddr;
    unAddr += SPI_UI_CONFIG_SIZE;
    unAddr = ((unAddr + (spiPageSize - 1)) / spiPageSize) * spiPageSize;		// page alignemnt

	if (unAddr > spiTotalSize)
    {
        DEBUG_SPI("Space(%#x) is not enough for arrangement.\n", unAddr);
        return 0;
    }

#ifdef SPI_DEBUG_FLAG
	DEBUG_SPI("Residual Free Space is %d KBytes\n", (spiTotalSize - unAddr) >> 10);

    DEBUG_SPI("spiCodeStartAddr = %#x\n", spiCodeStartAddr);
    DEBUG_SPI("spiSysParaAWBStartAddr = %#x\n", spiSysParaAWBStartAddr);
    DEBUG_SPI("spiSysParaRsvdStartAddr1 = %#x\n", spiSysParaRsvdStartAddr1);
    DEBUG_SPI("spiSysParaRsvdStartAddr2 = %#x\n", spiSysParaRsvdStartAddr2);
    DEBUG_SPI("spiDefectPixelStartAddr = %#x\n", spiDefectPixelStartAddr);
    DEBUG_SPI("spiSysParaFileCntListStartAddr = %#x\n", spiSysParaFileCntListStartAddr);
    DEBUG_SPI("spiSysParaUIFBStartAddr = %#x\n", spiSysParaUIFBStartAddr);
    DEBUG_SPI("spiUILibStartAddr = %#x\n", spiUILibStartAddr);
    DEBUG_SPI("spiSysParaAudioFileCntListStartAddr = %#x\n", spiSysParaAudioFileCntListStartAddr);
    DEBUG_SPI("spiSysParaAudioFBStartAddr = %#x\n", spiSysParaAudioFBStartAddr);
    DEBUG_SPI("spiAudioLibStartAddr = %#x\n", spiAudioLibStartAddr);
    DEBUG_SPI("spiFirmwareEndAddr = %#x\n", spiFirmwareEndAddr);
	DEBUG_SPI("spiPIRDataStartAddr = %#x\n", spiPIRDataStartAddr);
    DEBUG_SPI("spiMarssSensorStartAddr = %#x\n", spiMarssSensorStartAddr);
    DEBUG_SPI("spiMarssSysStartAddr = %#x\n", spiMarssSysStartAddr);
    DEBUG_SPI("spiHomeRFROStartAddr = %#x\n", spiHomeRFROStartAddr);
    DEBUG_SPI("spiHomeRFConfigStartAddr = %#x\n", spiHomeRFConfigStartAddr);
    DEBUG_SPI("spiHomeRFRoomStartAddr = %#x\n", spiHomeRFRoomStartAddr);
    DEBUG_SPI("spiHomeRFSceneStartAddr = %#x\n", spiHomeRFSceneStartAddr);
    DEBUG_SPI("spiHomeRFSensorStartAddr = %#x\n", spiHomeRFSensorStartAddr);
	DEBUG_SPI("spiUIConfigBackupStartAddr = %#x\n",spiUIConfigBackupStartAddr);
    DEBUG_SPI("spiNetwrokStartAddr = %#x\n", spiNetwrokStartAddr);
    DEBUG_SPI("spiRFIDStartAddr = %#x\n", spiRFIDStartAddr);
    DEBUG_SPI("spiUIConfigStartAddr = %#x\n",spiUIConfigStartAddr);
#endif

    return 1;
}
#endif

#if UI_PIC_LIST_DYNAMIC_SUPPORT
INT32U spiPreparePicSetParams(void)
{
    INT8U *targetBuf = SPIConfigBuf;
    INT32S unLen = 0x100;
    INT32U i;

    if (spiRead(targetBuf, SPI_CODE_END_ADDR, SPI_MAX_BUF_SIZE)==0)
    {
        DEBUG_SPI("Error: SPI Read error!\n");
        return 0;
    }
    
    spiPicDetialNum = *(INT32U *) (targetBuf + 0xF8);
    spiPicObjAddress = *(INT32U *) (targetBuf + 0xFC);
}
#endif

/*

Routine Description:

	Start SPI.

Arguments:

	None

Return Value:

	0 - Failure
	1 - Success

*/
s32 spiStart(void)
{
//	spiInit();

    if (spiIdentification() == 0)
    {
        DEBUG_SPI("Error! Identification Failed.\n");
        return 0;
    }

    /* Init the Flash space arrangement */
    if (spiInitTable == SPI_UNINITED)
    {
#if UI_PIC_LIST_DYNAMIC_SUPPORT
        spiPreparePicSetParams();
#endif
        if (spiArrangeSpace() == SPI_UNINITED)
            return 0;
        else
            spiInitTable = SPI_INITED;
    }
    spiInitIdx = SPI_INITED;

    return 1;
}

// No using WDT when firmware code is updating.
int spiFirmwareCodeUpdate(u8 *pSrcCodeAddr, u32 CodeLen)
{
	u32 unEAddr, unWAddr, tmpVal;
	u32 Length;
	u32 i, limit;
	u8 *pBufAddr = pSrcCodeAddr;
    u8 ucCnt;

    spiSetFirmwareUpdatePercentage(0);
    Length = CodeLen;
    tmpVal = Length / 100;
    limit = 3;	// the times of flash retry.
    ucCnt = 0;
	i = 0;

    DEBUG_SPI("In-System Update ++ALL MODE++ Code to Serial Flash...\n");

	DEBUG_SPI("SPI Write code Lenth: %d (bytes)\n", Length);

	for(unEAddr = SPI_BOOT_CODE_START_ADDR; unEAddr < Length; )
	{
		// Action erase
		for(; i < limit; )
		{
			if(spiSectorErase(unEAddr) == SPI_OK)
				break;

			DEBUG_SPI("[E] SPI sector erase %#x failed\n", unEAddr);
			i++;	// Action failed.
		}

		if(i >= limit)
		{
			DEBUG_SPI("[E] SPI Upgrade %#x failed\n", unEAddr);
            return SPI_ERR;
		}

		// Action write
		for(unWAddr = unEAddr; unWAddr < (unEAddr + spiSectorSize); )
		{
			if(spiWrite(unWAddr, pBufAddr + (unWAddr - unEAddr), SPI_MAX_BUF_SIZE) == 0)
			{
				i++;
				break;
			}
				
			unWAddr += SPI_MAX_BUF_SIZE;
		}

		if(unWAddr != (unEAddr + spiSectorSize))
		{
			DEBUG_SPI("[E] SPI Write code %#x error!\n", unEAddr);
			continue;
		}

		pBufAddr += spiSectorSize;
		unEAddr += spiSectorSize;

		spiSetFirmwareUpdatePercentage(unEAddr / tmpVal);

		switch(ucCnt & 0x3)
        {
            case 0:
                DEBUG_SPI("/");
                break;
                
            case 1:
                DEBUG_SPI("-");
                break;
                
            case 2:
                DEBUG_SPI("\\");
                break;
                
            case 3:
                DEBUG_SPI("|");
                break;
        }
        DEBUG_SPI("\b");
        ucCnt++;
	}

	DEBUG_SPI("Programming to Serial Flash Success.\n");
    return SPI_OK;
}


s32 spiallCodeUpdate(u8* pucCodeAddr, u32 unCodeSize)
{
    u32	unAddr;
    u32 i, tmp;
    u8* pucAddr = pucCodeAddr;
    s32	nCodeLen = (s32)unCodeSize;
    u32 count = 0;
    u8	ucCnt = 0;

    spiCodeUpgradepercentage = 0;
    tmp = nCodeLen*2 /100;

    DEBUG_SPI("In-System Update ++ALL MODE++ Code to Serial Flash...\n");

    /* SECTOR code block */
    for (unAddr = SPI_BOOT_CODE_START_ADDR; unAddr < nCodeLen; unAddr += spiSectorSize)
    {
#ifdef NEW_UI_ARCHITECTURE
        // skip over user data storing area (64K bytes; 1 block)
        if ((SETTING_BASE_ADDR <= unAddr) && (unAddr < SETTING_BASE_ADDR+SN_BLK_SIZE))
            continue;
#endif
        spiExecuteWDTCntReset();
        if (spiSectorErase(unAddr) == 0)
        {
            DEBUG_SPI("Error: sector erase failed\n");
            return 0;
        }
        count +=spiSectorSize;
        spiCodeUpgradepercentage = count / tmp;
    }

    DEBUG_SPI(" SPI Write code Lenth:%d \n",nCodeLen);
    /* write code */
    for (i=SPI_BOOT_CODE_START_ADDR; nCodeLen>0; i+=SPI_MAX_BUF_SIZE, nCodeLen-=SPI_MAX_BUF_SIZE, pucAddr+=SPI_MAX_BUF_SIZE)
    {
#ifdef NEW_UI_ARCHITECTURE
        // skip over user data storing area (64K bytes; 1 block)
        if ((SETTING_BASE_ADDR <= i) && (i < SETTING_BASE_ADDR+SN_BLK_SIZE))
            continue;
#endif
        spiExecuteWDTCntReset();
        if (spiWrite(i, pucAddr, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Write code error!\n");
            return 0;
        }
        count += SPI_MAX_BUF_SIZE;
        spiCodeUpgradepercentage = count / tmp;

#ifdef DEBUG_PROGRAM_MSG
        DEBUG_SPI("\b");
        switch(ucCnt % 4)
        {
            case 0:
                DEBUG_SPI("/");
                break;
            case 1:
                DEBUG_SPI("-");
                break;
            case 2:
                DEBUG_SPI("\\");
                break;
            case 3:
                DEBUG_SPI("|");
                break;
        }
        ucCnt++;
#endif
    }
#ifdef DEBUG_PROGRAM_MSG
    DEBUG_SPI("\b");
#endif

//    DEBUG_SPI("Programming to Serial Flash Success\n");
    return 1;
}

/*

Routine Description:

	SPI update code routine.

Arguments:

	pucCodeAddr - address pointer of code position to update.
	unCodeSize - update code size.

Return Value:

	1 - success
	0 - failed

*/
s32 spibootCodeUpdate(u8* pucCodeAddr, u32 unCodeSize)
{
    u32	unAddr;
    u32 i, tmp;
    u8* pucAddr = pucCodeAddr;
    s32	nCodeLen = (s32)unCodeSize;
    u32 count = 0;
    u8	ucCnt = 0;

    spiCodeUpgradepercentage = 0;
    tmp = nCodeLen*2 /100;

    DEBUG_SPI("In-System Update BootCode to Serial Flash...\n");

#if(CHIP_OPTION == CHIP_A1016A)
    /* check code size */
    if (0x1C00 < unCodeSize)
#else
    if (0x1000 < unCodeSize)
#endif
    {
        DEBUG_SPI("Error: Code size to update is over the range.\n");
        return 0;
    }

    /* SECTOR code block */
    for (unAddr =SPI_BOOT_CODE_START_ADDR; unAddr<SPI_BOOT_CODE_END_ADDR; unAddr+=spiSectorSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_BOOT_CODE))
        {
            DEBUG_SPI(" in spibootCodeUpdate.\n");
            return 0;
        }
        if (spiSectorErase(unAddr) == 0)
        {
            DEBUG_SPI("Error: sector erase failed\n");
            return 0;
        }
        count +=spiSectorSize;
        spiCodeUpgradepercentage = count / tmp;
    }
    DEBUG_SPI(" SPI Write code Lenth:%d \n",nCodeLen);
    /* write code */
    for (i=SPI_BOOT_CODE_START_ADDR; nCodeLen>0; i+=SPI_MAX_BUF_SIZE, nCodeLen-=SPI_MAX_BUF_SIZE, pucAddr+=SPI_MAX_BUF_SIZE)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(i, SPI_AREA_BOOT_CODE))
        {
            DEBUG_SPI(" in spibootCodeUpdate.\n");
            return 0;
        }
        if (spiWrite(i, pucAddr, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Write code error!\n");
            return 0;
        }
        count += SPI_MAX_BUF_SIZE;
        spiCodeUpgradepercentage = count / tmp;
#ifdef DEBUG_PROGRAM_MSG
        DEBUG_SPI("\b");
        switch(ucCnt % 4)
        {
            case 0:
                DEBUG_SPI("/");
                break;
            case 1:
                DEBUG_SPI("-");
                break;
            case 2:
                DEBUG_SPI("\\");
                break;
            case 3:
                DEBUG_SPI("|");
                break;
        }
        ucCnt++;
#endif
    }
#ifdef DEBUG_PROGRAM_MSG
    DEBUG_SPI("\b");
#endif

//    DEBUG_SPI("Programming to Serial Flash Success\n");
    return 1;
}

u32 spiGetUpgradePercentage()
{
    return spiCodeUpgradepercentage;
}

/*

Routine Description:

	SPI update code routine.

Arguments:

	pucCodeAddr - address pointer of code position to update.
	unCodeSize - update code size.

Return Value:

	1 - success
	0 - failed

*/
s32 spiCodeUpdate(u8* pucCodeAddr, u32 unCodeSize)
{
    u32	unAddr;
    u32 i, tmp;
    u8* pucAddr = pucCodeAddr;
    s32	nCodeLen = (s32)unCodeSize;
    u32 count = 0;
    u8	ucCnt = 0;

    spiCodeUpgradepercentage = 0;
    tmp = nCodeLen*2 /100;

    DEBUG_SPI("In-System A1013 Programming to Serial Flash...\n");

    /* check code size */
    if (SPI_MAX_CODE_SIZE < unCodeSize)
    {
        DEBUG_SPI("Error: Code size to update is over the range.\n");
        return 0;
    }

    /* sector code block */
    for (unAddr = SPI_BOOT_CODE_END_ADDR; unAddr<SPI_CODE_END_ADDR; unAddr+=spiSectorSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_MAIN_CODE))
        {
            DEBUG_SPI(" in spiCodeUpdate.\n");
            return 0;
        }
        if (spiSectorErase(unAddr) == 0)
        {
            DEBUG_SPI("Error: sector erase failed\n");
            return 0;
        }
        count +=spiSectorSize;
        spiCodeUpgradepercentage = count / tmp;
    }
    /* write code size at the first 256 bytes area */
    *(u32*)spiWriteBuf = unCodeSize;
    spiExecuteWDTCntReset();
    // Wrong place operation detect
    if(!spiAddressLocate(SPI_BOOT_CODE_END_ADDR, SPI_AREA_MAIN_CODE))
    {
        DEBUG_SPI(" in spiCodeUpdate.\n");
        return 0;
    }
    if (spiWrite(SPI_BOOT_CODE_END_ADDR, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
    {
        DEBUG_SPI("Error: SPI Write error!\n");
        return 0;
    }

    /* write code */
    DEBUG_SPI(" SPI Write code Lenth:%d \n",nCodeLen);
    for (i=SPI_CODE_START_ADDR; nCodeLen>0; i+=SPI_MAX_BUF_SIZE, nCodeLen-=SPI_MAX_BUF_SIZE, pucAddr+=SPI_MAX_BUF_SIZE)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(i, SPI_AREA_MAIN_CODE))
        {
            DEBUG_SPI(" in spiCodeUpdate.\n");
            return 0;
        }
        if (spiWrite(i, pucAddr, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Write code error!\n");
            return 0;
        }
        count += SPI_MAX_BUF_SIZE;
        spiCodeUpgradepercentage = count / tmp;
#ifdef DEBUG_PROGRAM_MSG
        DEBUG_SPI("\b");
        switch(ucCnt % 4)
        {
            case 0:
                DEBUG_SPI("/");
                break;
            case 1:
                DEBUG_SPI("-");
                break;
            case 2:
                DEBUG_SPI("\\");
                break;
            case 3:
                DEBUG_SPI("|");
                break;
        }
        ucCnt++;
#endif
    }
#ifdef DEBUG_PROGRAM_MSG
    DEBUG_SPI("\b");
#endif

//    DEBUG_SPI("Programming to Serial Flash Success\n");
    return 1;
}

/*

Routine Description:

	SPI compare write data routine.

Arguments:

	pucCodeAddr - address pointer of code position to update.
	unCodeSize - compare code size.

Return Value:

	1 - success
	0 - failed

*/
s32 spiCmpBootWriteData(u8* pucCodeAddr, u32 unCodeSize)
{
    u32 i, j;
    u8* pucAddr = pucCodeAddr;
    s32 nCodeLen = (s32) unCodeSize;

    for (i=0; nCodeLen>0; i+=SPI_MAX_BUF_SIZE, nCodeLen-=SPI_MAX_BUF_SIZE, pucAddr+=SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        spiExecuteWDTCntReset();
        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE)==0)
        {
            DEBUG_SPI("Error: SPI Read error!\n");
            return 0;
        }

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
        {
            if ((*(pucAddr+j))!= spiReadBuf[j])
            {
                DEBUG_SPI("Error: Compare error\n");
                return 0;
            }
        }
        sysDeadLockMonitor_Reset();
    }

    return 1;
}

/*

Routine Description:

	SPI compare write data routine.

Arguments:

	pucCodeAddr - address pointer of code position to update.
	unCodeSize - compare code size.

Return Value:

	1 - success
	0 - failed

*/
s32 spiCmpWriteData(u8* pucCodeAddr, u32 unCodeSize)
{
    u32 i, j;
    u8* pucAddr = pucCodeAddr;
    s32 nCodeLen = (s32) unCodeSize;

    for (i=SPI_CODE_START_ADDR; nCodeLen>0; i+=SPI_MAX_BUF_SIZE, nCodeLen-=SPI_MAX_BUF_SIZE, pucAddr+=SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        spiExecuteWDTCntReset();
        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE)==0)
        {
            DEBUG_SPI("Error: SPI Read error!\n");
            return 0;
        }

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
        {
            if ((*(pucAddr+j))!= spiReadBuf[j])
            {
                DEBUG_SPI("Error: Compare error\n");
                return 0;
            }
        }
        sysDeadLockMonitor_Reset();
    }

    return 1;
}

/*

Routine Description:

	Read AWB setting from serial flash.

Arguments:

	None.

Return Value:

	None.

*/
void spiReadAWBSetting(void)
{}


/*

Routine Description:

	Read UI setting from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
#if UI_ICONFLAG_BACKUP
void spiReadUIBackup(void)
{
    u32     i,j;
    u8      check_val=0;
    u8*     targetBuf = SPIConfigBuf;
    s32     unLen = SPI_UI_CONFIG_BACKUP_SIZE;
    BOOLEAN check = FALSE;

    for (i = spiUIConfigBackupStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Backup Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }
    uiReadSettingFromFlash(SPIConfigBuf);


    check_val = uiGetSaveChecksum();

    if (iconflag[UI_MENU_SETIDX_CHECK] == check_val)
        check = TRUE;

    /* used default setting*/
    if (check == FALSE)
    {
        DEBUG_SPI("iconflag %d check_val %d \n",iconflag[UI_MENU_SETIDX_CHECK],check_val);
        DEBUG_SPI("UI Setting Backup Check Fail, Set Default Value!\n");
        uiSetDefaultSetting();
    }
}

s32 spiWriteUIBackup(void)
{
    u8* targetBuf = SPIConfigBuf;
    u32 unAddr, startAddr = spiUIConfigBackupStartAddr;
    s32 unTotalSize = SPI_UI_CONFIG_BACKUP_SIZE;

    spiExecuteWDTCntReset();
    // Wrong place operation detect
    if(!spiAddressLocate(startAddr, SPI_AREA_UI_BACKUP))
    {
        DEBUG_SPI(" in spiWriteUIBackup.\n");
        return 0;
    }
    if (!spiSectorErase(startAddr))
    {
        DEBUG_SPI("Error! Block Erase Fail in spiWriteUIBackup.\n");
        return 0;
    }
    uiWriteSettingToFlash(targetBuf);
    for (unAddr = startAddr; unTotalSize>0; unAddr+= spiPageSize, unTotalSize-=spiPageSize, targetBuf+= spiPageSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_UI_BACKUP))
        {
            DEBUG_SPI(" in spiWriteUIBackup.\n");
            return 0;
        }
        if (!spiWrite(unAddr, targetBuf, SPI_MAX_BUF_SIZE))
        {
            DEBUG_SPI("Error! SPI Write Error in UI Config Bcakup Update.\n");
            return 0;
        }
    }

    DEBUG_SPI("SPI Write UI Config Backup Finish.\n");

    return 1;
}
#endif

void spiReadUI(void)
{
    u32     i;
    u8      check_val=0;
    u8*     targetBuf = SPIConfigBuf;
    s32     unLen = SPI_UI_CONFIG_SIZE;
    BOOLEAN check = FALSE;

    for (i = spiUIConfigStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }
    uiReadSettingFromFlash(SPIConfigBuf);


    check_val = uiGetSaveChecksum();

    if (iconflag[UI_MENU_SETIDX_CHECK] == check_val)
        check = TRUE;

    /* used default setting*/
#if UI_ICONFLAG_BACKUP
    if (check == FALSE)
    {
        DEBUG_SPI("iconflag %d check_val %d \n",iconflag[UI_MENU_SETIDX_CHECK],check_val);
        DEBUG_SPI("UI Setting Check Fail, Set UI Setting Backup Check!\n");
        spiReadUIBackup();
    }
#else
    if (check == FALSE)
    {
        DEBUG_SPI("UI Setting Check Fail, Set Default Value!\n");
        uiSetDefaultSetting();
    }
#endif
}

/*

Routine Description:

	Read RFID & RF Code from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
void spiReadRF_ID(void)
{
    u32     i;
    u8*     targetBuf = SPIConfigBuf;
    s32     unLen = SPI_UI_RF_INFO_SIZE;

#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

    for (i = spiRFIDStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }

    uiReadRFIDFromFlash(SPIConfigBuf);
}

#if NIC_SUPPORT
/*

Routine Description:

	Read MAC address and TUTK ID from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
void spiReadNet(void)
{
    u32     i;
    u8*     targetBuf = SPIConfigBuf;
    s32     unLen = SPI_UI_NETWORK_INFO_SIZE;

#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

    for (i = spiNetwrokStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }
    uiReadNetworkIDFromFlash(SPIConfigBuf);

}
#endif

/*

Routine Description:

	Read Version from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
void spiReadVersion(void)
{
    u32     i;
    u8*     targetBuf = SPIConfigBuf;
    s32     unLen = 0x100;

#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

    for (i = SPI_VERSION_ADDR; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;

    }
    uiReadVersionFromFlash(SPIConfigBuf);

}

void spiReleaseFlashStatus(void)
{
	if(spiTotalSize > 0x1000000)
		spiExit4ByteMode();
}

/*

Routine Description:

	Read Data from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
void spiReadData(u8 *targetBuf, u32 StartAddr, u32 unLen )
{
    u32 i;
    s32 length = unLen;

    sysSD_Disable();
    sysSPI_Enable() ;
#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

    for (i = StartAddr; length > 0; i += spiPageSize, length -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead(targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }
    sysSPI_Disable();
    sysSD_Enable();
}

void spiWriteData(u32 StartAddr, u8 *targetBuf, u32 unLen)
{
    u32 i;
    s32 length;

    sysSD_Disable();
    sysSPI_Enable() ;
#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

	for(i = StartAddr, length = unLen; length > 0; i += spiSectorSize, length -= spiSectorSize)
	{
		spiExecuteWDTCntReset();
		if(spiSectorErase(i) == 0)
		{
			DEBUG_SPI("Error: SPI Erase %#x error!\n", i);
			return;
		}
	}

	for(i = StartAddr, length = unLen; length > 0; i += spiPageSize, length -= spiPageSize)
	{
		spiExecuteWDTCntReset();
		if (spiWrite(i, targetBuf, SPI_MAX_BUF_SIZE) == 0)
		{
			DEBUG_SPI("Error: SPI Write %#x error!\n", i);
			return;
		}
		targetBuf += spiPageSize;
    }
    sysSPI_Disable();
    sysSD_Enable();
}

#if ADAPTIVE_AE_BY_ADC
int spiReadAEData(u8 *targetBuf, u32 Length)
{
	u32 i, StartAddr = spiPIRDataStartAddr;
	int unLen = Length;
	
	if(spiInitIdx == SPI_UNINITED)
	{
		if(spiStart() == 0)
        {
        	DEBUG_SPI("[E] SPI flash init.\n");
        	return 0;
        }
    }

    // Wrong place operation detect
    if(spiAddressLocate(StartAddr, SPI_AREA_PIR_SENSOR) == 0)
    {
        DEBUG_SPI(" in ReadAEData.\n");
        return 0;
    }

    for(i = StartAddr; unLen > 0; i += SPI_MAX_BUF_SIZE, unLen -= SPI_MAX_BUF_SIZE)
    {
        spiExecuteWDTCntReset();
        if (spiRead(targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("[E] SPI Raed %#x in ReadAEData.\n", i);
            return 0;
        }
        targetBuf += SPI_MAX_BUF_SIZE;
    }

    return 1;
}

int spiWriteAEData(u8 *targetBuf, u32 Length)
{
	u32 i, StartAddr = spiPIRDataStartAddr;
	int unLen = Length;
	
	if(spiInitIdx == SPI_UNINITED)
	{
		if(spiStart() == 0)
        {
        	DEBUG_SPI("[E] SPI flash init.\n");
        	return 0;
        }
    }

    // Wrong place operation detect
    if(spiAddressLocate(StartAddr, SPI_AREA_PIR_SENSOR) == 0)
    {
        DEBUG_SPI(" in WriteAEData.\n");
        return 0;
    }

	spiExecuteWDTCntReset();
    if(spiSectorErase(StartAddr) == 0)
    {
        DEBUG_SPI("[E] SPI Raed %#x in ReadAEData.\n", StartAddr);
        return 0;
    }

    for(i = StartAddr; unLen > 0; i += SPI_MAX_BUF_SIZE, unLen -= SPI_MAX_BUF_SIZE)
    {
    	// Wrong place operation detect
	    if(spiAddressLocate(StartAddr, SPI_AREA_PIR_SENSOR) == 0)
	    {
	        DEBUG_SPI(" in WriteAEData.\n");
	        return 0;
	    }
	    
        spiExecuteWDTCntReset();
        if (spiWrite(i, targetBuf, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("[E] SPI Write %#x in WriteAEData.\n", i);
            return 0;
        }
        targetBuf += SPI_MAX_BUF_SIZE;
    }
    
    return 1;
}
#endif

/*

Routine Description:

	Write UI setting to serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
s32 spiWriteUI(void)
{
    u8* targetBuf = SPIConfigBuf;
    u32 unAddr, startAddr = spiUIConfigStartAddr;
    s32 unTotalSize = SPI_UI_CONFIG_SIZE;

    spiExecuteWDTCntReset();
    // Wrong place operation detect
    if(!spiAddressLocate(startAddr, SPI_AREA_UI_CONFIG))
    {
        DEBUG_SPI(" in spiWriteUI.\n");
        return 0;
    }
    if (!spiSectorErase(startAddr))
    {
        DEBUG_SPI("Error! Block Erase Fail in spiWriteUI.\n");
        return 0;
    }
    uiWriteSettingToFlash(targetBuf);
    for (unAddr = startAddr; unTotalSize > 0; unAddr += spiPageSize, unTotalSize -= spiPageSize, targetBuf += spiPageSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_UI_CONFIG))
        {
            DEBUG_SPI(" in spiWriteUI.\n");
            return 0;
        }
        if (!spiWrite(unAddr, targetBuf, SPI_MAX_BUF_SIZE))
        {
            DEBUG_SPI("Error! SPI Write Error in UI Config Update.\n");
            return 0;
        }
    }
    //DEBUG_SPI("SPI Write UI Config Finish.\n");
    return 1;
}

#if NIC_SUPPORT
/*

Routine Description:

	Write RFID and RF code to serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
s32 spiWriteNet(void)
{
    u8* targetBuf = SPIConfigBuf;
    u32 unAddr, startAddr = spiNetwrokStartAddr;
    s32 unTotalSize = SPI_UI_NETWORK_INFO_SIZE;

    sysSD_Disable();
    sysSPI_Enable();
    spiExecuteWDTCntReset();
    // Wrong place operation detect
    if(!spiAddressLocate(startAddr, SPI_AREA_NETWORK))
    {
        DEBUG_SPI(" in spiWriteNet.\n");
        return 0;
    }
    if (!spiSectorErase(startAddr))
    {
        DEBUG_SPI("Error! Block Erase Fail in spiWriteUI.\n");
        return 0;
    }

    uiWriteNetworkIDFromFlash(targetBuf);

    for (unAddr = startAddr; unTotalSize > 0; unAddr += spiPageSize, unTotalSize -= spiPageSize, targetBuf += spiPageSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_NETWORK))
        {
            DEBUG_SPI(" in spiWriteNet.\n");
            return 0;
        }
        if (!spiWrite(unAddr, targetBuf, SPI_MAX_BUF_SIZE))
        {
            DEBUG_SPI("Error! SPI Write Error in UI Config Update.\n");
            return 0;
        }
    }
    sysSD_Enable();
    sysSPI_Disable();
    return 1;
}
#endif

//u8 targetBuf[MARSS_SYS_SIZE];

u8 csum(u8 *data, u8 length)
{
     u8 count;
     u32 Sum = 0;
     
     for (count = 0; count < length; count++)
         Sum = Sum + *(data+count);

//	 DEBUG_TIMER("Sum=0x%X, val = 0x%X\n", Sum, *(data+count));
     return (Sum & 0xFF);
}

#if MARSS_SUPPORT// A7128_FLASH_DEBUG
extern void MarssSysReadFromFlash(u8* bufAddr);
extern void MarssSysWriteToFlash(u8* bufAddr);
extern void MarssSensorReadFromFlash(u8* bufAddr);
extern void MarssSensorWriteToFlash(u8* bufAddr);

u8 targetBuf[MARSS_SYS_SIZE];

void spiWriteMarss()
{
    u8  err;
	u8 *targetBuf = SPIConfigBuf;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
	MarssSensorWriteToFlash(targetBuf);
	DEBUG_TIMER("MARSS sensor write checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SENSOR_SIZE-1), spiMarssSensorStartAddr);
	spiWriteData(spiMarssSensorStartAddr, targetBuf, MARSS_SENSOR_SIZE);

//	MarssSysDump();

	MarssSysWriteToFlash(targetBuf); //Copy data from MarssSys
	DEBUG_TIMER("MARSS sys write checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SYS_SIZE-1), spiMarssSysStartAddr);
	spiWriteData(spiMarssSysStartAddr, targetBuf, MARSS_SYS_SIZE); // Write to SPI flash

    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void spiReadMarss()
{
    u8  err;
	u8 *targetBuf = SPIConfigBuf;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
	
	spiReadData( targetBuf, spiMarssSensorStartAddr, MARSS_SENSOR_SIZE);
	MarssSensorReadFromFlash(targetBuf);
	DEBUG_TIMER("A7128 sensor read checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SENSOR_SIZE-1), spiMarssSensorStartAddr);

	spiReadData(targetBuf, spiMarssSysStartAddr, MARSS_SYS_SIZE);
	MarssSysReadFromFlash(targetBuf);
	DEBUG_TIMER("A7128 sys read checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SYS_SIZE-1), spiMarssSysStartAddr);


    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void spiWriteMarssTest()
{
    u8  err;
	
//A7128 coinfig in flash
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
	DEBUG_TIMER("\n============SPI write MARSS test\n");

	memset(targetBuf, 0x55, MARSS_SENSOR_SIZE);
	DEBUG_TIMER("A7128 sensor checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SENSOR_SIZE-1), spiMarssSensorStartAddr);
	spiWriteData(spiMarssSensorStartAddr, targetBuf, MARSS_SENSOR_SIZE);

	memset(targetBuf, 0, MARSS_SYS_SIZE);
	memset(targetBuf, 0xAA, MARSS_SYS_SIZE);
	DEBUG_TIMER("A7128 sys checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SYS_SIZE-1), spiMarssSysStartAddr);
	spiWriteData(spiMarssSysStartAddr, targetBuf, MARSS_SYS_SIZE);


    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void spiReadMarssTest()
{
    u8  err;
//A7128 coinfig in flash
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
	DEBUG_TIMER("\n============SPI read MARSS test\n");
	memset(targetBuf, 0, MARSS_SYS_SIZE);
	spiReadData( targetBuf, spiMarssSensorStartAddr, MARSS_SENSOR_SIZE);
	DEBUG_TIMER("A7128 sensor checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SENSOR_SIZE-1), spiMarssSensorStartAddr);

	memset(targetBuf, 0, MARSS_SYS_SIZE);
	spiReadData(targetBuf, spiMarssSysStartAddr, MARSS_SYS_SIZE);
	DEBUG_TIMER("A7128 sys checksum: 0x%X @ 0x%X\n", csum(targetBuf,MARSS_SYS_SIZE-1), spiMarssSysStartAddr);

    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}
#endif


#if(HOME_RF_SUPPORT)
/*

Routine Description:

	Write MAC address and UID to serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
extern u8 *homeRFWriteToFlash(u8* bufAddr, u8 type);
s32 spiWriteHomeRF(u8 typeIdx)
{
    u8* targetBuf = SPIConfigBuf;
    u32 unAddr;
    s32 unTotalSize = 0;
    u32 targetAddr=0;
    u8  err;
    u8  i;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysSD_Disable();
    sysSPI_Enable();

    if(typeIdx == SPI_HOMERF_RODATA)
    {
        targetAddr=spiHomeRFROStartAddr;
        unTotalSize=SPI_HOMERF_RODATA_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_CONFIG)
    {
        targetAddr=spiHomeRFConfigStartAddr;
        unTotalSize=SPI_HOMERF_CONFIG_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_ROOM)
    {
        targetAddr=spiHomeRFRoomStartAddr;
        unTotalSize=SPI_HOMERF_ROOM_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_SCENE)
    {
        targetAddr=spiHomeRFSceneStartAddr;
        unTotalSize=SPI_HOMERF_SCENE_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_SENSOR)
    {
        targetAddr=spiHomeRFSensorStartAddr;
        unTotalSize=SPI_HOMERF_SENSOR_SIZE;
    }


    for( i=0; i< unTotalSize/0x1000; i++)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(targetAddr+i*0x1000, SPI_AREA_HA))
        {
            DEBUG_SPI(" in spiWriteHomeRF.\n");
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
        if (!spiSectorErase(targetAddr+i*0x1000))
        {
            DEBUG_SPI("Error! Block(0x%x) Erase Fail in spiWriteUI.\n",targetAddr);
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
    }

    targetBuf = homeRFWriteToFlash(targetBuf, typeIdx);
    DEBUG_GREEN("[%s] tgAddr=0x%X size = %d @ gSeList 0x%p to 0x%p\n",__func__, targetAddr,
                sizeof(gHomeRFSensorList->sSensor), &gHomeRFSensorList->sSensor, targetBuf);

    for (unAddr=targetAddr; unTotalSize>0; unAddr+= spiPageSize, unTotalSize-=spiPageSize, targetBuf+= spiPageSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_HA))
        {
            DEBUG_SPI(" in spiWriteHomeRF.\n");
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
        if (!spiWrite(unAddr, targetBuf, SPI_MAX_BUF_SIZE))
        {
            DEBUG_SPI("Error! SPI Write Error in UI Config Update.\n");
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
    }
    
    DEBUG_SPI("SPI Write HOMERF Finish. %d %x %x\n",typeIdx, targetAddr, unTotalSize);
//    if(typeIdx == SPI_HOMERF_SENSOR)
//		DEBUG_RED("[%s] Checksum = 0x%X\n",__func__,csum(targetBuf,SPI_HOMERF_SENSOR_SIZE));
    sysSD_Enable();
    sysSPI_Disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    return 1;
}



/*

Routine Description:

	Read Home automation data from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
void spiReadHomeRF(u8 typeIdx)
{
    u32     i;
    u8      check_val=0;
    u8*     targetBuf = SPIConfigBuf;
    u32     targetAddr=0;
    s32     unLen = SPI_BUF_SIZE;
    BOOLEAN check = FALSE;

#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
#endif

    memset(SPIConfigBuf,0, SPI_BUF_SIZE);


    if(typeIdx == SPI_HOMERF_RODATA)
    {
        targetAddr=spiHomeRFROStartAddr;
        unLen=SPI_HOMERF_RODATA_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_CONFIG)
    {
        targetAddr=spiHomeRFConfigStartAddr;
        unLen=SPI_HOMERF_CONFIG_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_ROOM)
    {
        targetAddr=spiHomeRFRoomStartAddr;
        unLen=SPI_HOMERF_ROOM_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_SCENE)
    {
        targetAddr=spiHomeRFSceneStartAddr;
        unLen=SPI_HOMERF_SCENE_SIZE;
    }
    else if(typeIdx == SPI_HOMERF_SENSOR)
    {
        targetAddr=spiHomeRFSensorStartAddr;
        unLen=SPI_HOMERF_SENSOR_SIZE;
    }

    for (i=targetAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)targetBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        targetBuf += spiPageSize;
    }
    homeRFReadFromFlash(SPIConfigBuf, typeIdx);
//	if(SPI_HOMERF_SENSOR == typeIdx)
//		DEBUG_RED("[%s] Checksum = 0x%X\n",__func__,csum(SPIConfigBuf, sizeof(gHomeRFSensorList->sSensor)));
}




#endif

/*

Routine Description:

	Write HomeAutomation data to serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
s32 spiWriteRF(s32 dummy)
{
    u8* targetBuf = SPIConfigBuf;
    u32 unAddr, startAddr = spiRFIDStartAddr;
    s32 unTotalSize = SPI_UI_RF_INFO_SIZE;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysSD_Disable();
    sysSPI_Enable();
    spiExecuteWDTCntReset();
    // Wrong place operation detect
    if(!spiAddressLocate(startAddr, SPI_AREA_RFID))
    {
        DEBUG_SPI(" in spiWriteRF.\n");
        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
        return 0;
    }
    if (!spiSectorErase(startAddr))
    {
        DEBUG_SPI("Error! Block Erase Fail in spiWriteUI.\n");
        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
        return 0;
    }

    uiWriteRFIDFromFlash(targetBuf);
    for (unAddr = startAddr; unTotalSize>0; unAddr+= spiPageSize, unTotalSize-=spiPageSize, targetBuf+= spiPageSize)
    {
        spiExecuteWDTCntReset();
        // Wrong place operation detect
        if(!spiAddressLocate(unAddr, SPI_AREA_RFID))
        {
            DEBUG_SPI(" in spiWriteRF.\n");
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
        if (!spiWrite(unAddr, targetBuf, SPI_MAX_BUF_SIZE))
        {
            DEBUG_SPI("Error! SPI Write Error in UI Config Update.\n");
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            return 0;
        }
    }
    DEBUG_SPI("SPI Write RF Finish.\n");

    sysSD_Enable();
    sysSPI_Disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    return 1;
}


/*

Routine Description:

	Read Frame Block setting from serial flash.

Arguments:

	None.

Return Value:

	None.

Note:
    1.It must be set sysSPI_Enable() before call this function,
      and set sysSPI_Disable() after call this function
    2.Use PKBuf may cause potential risk when video decoding
*/

void spiReadFBSetting(void)
{
    u32 i,FileCount=0;
    s32 uLen;
    u8* pucBuf = PKBuf;

    /*read uiFileListCharCount table*/
//    pucBuf = (u8*)uiFileListCharCount;
    uLen = sizeof(UI_FILE_CHAR_COUNT) * 26;    /* 26 alphabets */
    for (i = spiSysParaFileCntListStartAddr; uLen > 0; i += spiPageSize, uLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)pucBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
//        pucBuf = pucBuf + SPI_MAX_BUF_SIZE;

        memcpy(uiFileListCharCount, pucBuf, sizeof(UI_FILE_CHAR_COUNT) * 26);

    }
    for (i = 0; i < 26; i++)
    {
        FileCount += uiFileListCharCount[i].stCount;
//        DEBUG_SPI("table %c stStart %d stCount %d \r\n",i+65,uiFileListCharCount[i].stStart, uiFileListCharCount[i].stCount);
    }
//    DEBUG_SPI("FileCount %d\r\n",FileCount);
    /*check uiFileListCharCount error*/
    if (FileCount != uiFileListCharCount[25].stCount+uiFileListCharCount[25].stStart)
    {
        memset(uiFileListCharCount, 0, sizeof(uiFileListCharCount));
        DEBUG_SPI("uiFileListCharCount read error in spiReadFBSetting\r\n");
        return;
    }
    /*read frame_buf_obj table*/
//    pucBuf = (u8*)frame_buf_obj;
    pucBuf = (u8*) PKBuf;
    uLen = sizeof(FRAME_BUF_OBJECT) * FileCount;
    for (i = spiSysParaUIFBStartAddr; uLen > 0; i += spiPageSize, uLen -= spiPageSize)
    {
        spiExecuteWDTCntReset();
        if (spiRead((u8*)pucBuf, i, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error: SPI Raed error!\n");
            return;
        }
        pucBuf+=SPI_MAX_BUF_SIZE;
    }
    memcpy(frame_buf_obj, PKBuf, sizeof(FRAME_BUF_OBJECT) * FileCount);
#if 0
    for (i = 0; i < FileCount; i++)
        DEBUG_SPI("file name %s \r\n",frame_buf_obj[i].stFileName);
#endif
}

/*

Routine Description:

    Get frame_buf_obj index

Arguments:

    targetStr - file name

Return Value:

    -1 - file not fund.
    -2 - file name errors.

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
s32 spiGet_UI_FB_Index(u8* targetStr)
{
    u8  Char_idx;
    u8  MainName[13];
    u32 i,index;
    u16 strLen;

    memset(MainName, 0, sizeof(MainName));
    strLen=strlen((char*)targetStr);
    for (i = 0; i < strLen; i++)     // Since we just support 8.3 format
    {
        MainName[i] = targetStr[i];
        if ((MainName[i] >= 'a') && (MainName[i] <= 'z'))
            MainName[i] -= 0x20;
    }

    if ((MainName[0] >= 'A') && (MainName[0] <= 'Z'))
        Char_idx = MainName[0] - 'A';
    else
        return -2;

    strLen=strLen-4;  // we didn't compare the .bin extention file name
//    DEBUG_SPI("spiGet_UI_FB_Index Char_idx %d, strLen %d\r\n",Char_idx,strLen);
//    DEBUG_SPI("serach for %s\r\n",MainName);
    for (i = 0; i < uiFileListCharCount[Char_idx].stCount; i++)
    {
        index = uiFileListCharCount[Char_idx].stStart+i;
//        if (strlen((char*)frame_buf_obj[index].stFileName) != (strLen+4))
        //          continue;
//        DEBUG_SPI("cmp MainName %s and %s\r\n", MainName,frame_buf_obj[index].stFileName);
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
        if (strncmp((const char *)MainName, (const char *)frame_buf_obj[index].stFileName, (strLen+1)) == 0)
#else
        if (strncmp((const char *)MainName, (const char *)frame_buf_obj[index].stFileName, strLen) == 0)
#endif
            return index;
    }
    //DEBUG_MAIN("get file %s from uiFileListCharCount fail\r\n",targetStr);
    return -1;
}

/*

Routine Description:

    Get frame_buf_obj index

Arguments:

    targetStr - file name

Return Value:

	1 - success
	0 - failed

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
u8 spiUI_OpenFB_ByIndex(s32 fb_index)
{
    u8* targetBuf = exifDecBuf;
    u32 CurReadSize;

    if (fb_index < 0)
        return 0;
    if (spiInitIdx == SPI_UNINITED)
    {
        spiStart();
        spiInitIdx = SPI_INITED;
    }
    spiExecuteWDTCntReset();
    CurReadSize = (frame_buf_obj[fb_index].stFileLen+3)&~3;
    spiRead(targetBuf, frame_buf_obj[fb_index].stPageStartAddr, CurReadSize);
    return 1;
}


/*

Routine Description:

    Get frame_buf_obj index

Arguments:

	pucDstAddr - Destination Buffer Address to store read out image.
	fb_index - frame buffer index.

Return Value:

	1 - success
	0 - failed

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
u8 spiUI_OpenFB_ByIndexToSpecificAddr(u8	*pucDstAddr, s32 fb_index)
{

    if (fb_index < 0)
        return 0;

    if (spiInitIdx == SPI_UNINITED)
    {
        spiStart();
        spiInitIdx = SPI_INITED;
    }
    spiExecuteWDTCntReset();
    spiRead(pucDstAddr, frame_buf_obj[fb_index].stPageStartAddr, frame_buf_obj[fb_index].stFileLen);
    return 1;
}

/*

Routine Description:

    Get Serial Number in serial flash.

Arguments:

	pucBufAddr - the buffer to store read-out data.

Return Value:

	1 - success
	0 - failed

Note:
    It must be set sysSPI_Enable() before call this function,
    and set sysSPI_Disable() after call this function
*/
u8 spiReadSerialNumber(u8 *pucBufAddr)
{
    spiExecuteWDTCntReset();
    /* read out and verify data */
    if (spiRead(pucBufAddr, spiModelParaAddr, SPI_MAX_BUF_SIZE)==0)
    {
        DEBUG_SPI("Error: SPI Read error!\n");
        return 0;
    }

    return 1;
}

#if SPI_ENDIAN_TEST
u32 spiEndianRunningTest(u32 uAddr, u32 uLen, u32 DMAEn)
{
    u32 TmpVal, i;
    u8 *OutputBuf = SPIConfigBuf;
    u8 *InputBuf = SPIConfigBuf + (SPI_BUF_SIZE >> 1);

    if(uLen > (SPI_BUF_SIZE >> 1))
    {
        DEBUG_SPI("uLen must less than %#x\n", (SPI_BUF_SIZE >> 1));
        return 0;
    }

    // 8 bit
    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Big mode
    SpiEndian = SPI_SR_RX_ENDI_BIG | SPI_SR_TX_ENDI_BIG;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 8 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 8 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 1) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 8 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 8 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 1) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 8 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("8 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI big endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }

    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Little mode
    SpiEndian = SPI_SR_RX_ENDI_LIT | SPI_SR_TX_ENDI_LIT;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 8 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 8 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 1) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 8 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 8 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 1) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 8 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("8 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI little endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }



    // 16 bit
    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Big mode
    SpiEndian = SPI_SR_RX_ENDI_BIG | SPI_SR_TX_ENDI_BIG;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 16 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 16 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 2) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 16 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 16 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 2) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 16 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("16 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI big endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }

    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Little mode
    SpiEndian = SPI_SR_RX_ENDI_LIT | SPI_SR_TX_ENDI_LIT;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 16 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 16 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 2) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 16 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 16 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 2) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 16 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("16 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI little endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }



    // 24 bit
    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;
        
    for(i = 0; i < 2; i++)
        InputBuf[i + uLen] = i;

    // Big mode
    SpiEndian = SPI_SR_RX_ENDI_BIG | SPI_SR_TX_ENDI_BIG;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 24 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 24 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 3) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 24 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 24 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 3) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 24 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("24 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI big endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }

    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;
    for(i = 0; i < 2; i++)
        InputBuf[i + uLen] = i;

    // Little mode
    SpiEndian = SPI_SR_RX_ENDI_LIT | SPI_SR_TX_ENDI_LIT;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 24 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 24 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 3) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 24 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 24 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 3) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 24 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("24 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI little endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }



    // 32 bit
    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Big mode
    SpiEndian = SPI_SR_RX_ENDI_BIG | SPI_SR_TX_ENDI_BIG;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 32 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 32 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 4) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 32 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 32 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 4) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 32 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("32 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI big endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }

    memset(OutputBuf, 0x00, uLen);
    for(i = 0; i < uLen; i++)
        InputBuf[i] = i;

    // Little mode
    SpiEndian = SPI_SR_RX_ENDI_LIT | SPI_SR_TX_ENDI_LIT;
    if(spiSectorErase(uAddr) == 0)
    {
        DEBUG_SPI("[E] SPI erase error at phase 32 bit!\n");
        return 0;
    }

    if(DMAEn)
    {
        if(spiWrite(uAddr, InputBuf, uLen) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 32 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiWriteCPU(uAddr, InputBuf, uLen, 4) == 0)
        {
            DEBUG_SPI("[E] SPI write error at phase 32 bit!\n");
            return 0;
        }
    }

    if(DMAEn)
    {
        if(spiRead(OutputBuf, uAddr, uLen) == 0)
        {
            DEBUG_SPI("[E]: SPI read error at phase 32 bit!\n");
            return 0;
        }
    }
    else
    {
        if(spiReadCPU(OutputBuf, uAddr, uLen, 4) == 0)
        {
            DEBUG_SPI("[E] SPI read error at phase 32 bit!\n");
            return 0;
        }
    }

    DEBUG_SPI("32 bit test.\n");
    DEBUG_SPI("Original:\n");
    for(i = 0; i < (uLen >> 4); i++)
    {
        if(i && !(i%16))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", InputBuf[i]);
    }
    DEBUG_SPI("\n......\n");
    DEBUG_SPI("SPI little endian read:\n");
    for(i = 0; i < uLen; i++)
    {
        if(i && !(i & 0xf))
            DEBUG_SPI("\n");
        DEBUG_SPI("%02x ", OutputBuf[i]);
    }
    DEBUG_SPI("\n");
    for(i = 0; i < uLen; i++)
    {
        if(OutputBuf[i] != InputBuf[i])
        {
            DEBUG_SPI("[Error]: buffer match error!\n");
            return 0;
        }
    }
    
    return 1;
}
#endif

#if INTERNAL_STORAGE_TEST

/*

Routine Description:

	SPI Block Erase Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiBlockEraseTest (u32 unAddr)
{
    u32 i, j, k;
    u8	ucCnt;

    unAddr &= 0x000F0000;

    DEBUG_SPI("The testing Block Address = %#x\n", unAddr);

    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        if (spiBlockErase(unAddr)==0)
            return 0;

        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Block Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<(k+10)) && (k<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("i = %#x\n", i);
                    DEBUG_SPI("Block Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)

                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
        ucCnt = 0;

        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Block Erase\n");

    unAddrForTest += spiBlockSize;

    return 1;
}


s32 spiBlockEraseSel(u32 unAddr,u8 SPIno)
{
    s32 EB_ok;

    switch (SPIno)
    {
        case 1: //spi1
            EB_ok = spiBlockErase(unAddr);
            break;
        case 2: //spi2
            EB_ok =spiBlockErase2(unAddr,1);
            break;
        case 3: //spi3
            EB_ok =spiBlockErase3(unAddr,0);
            break;
        case 4: //spi4
            EB_ok =spiBlockErase4(unAddr,1);
            break;
        case 5: //spi5
            EB_ok =spiBlockErase5(unAddr,0);
            break;
        default:
            DEBUG_SPI("spiBlockErase Fail!\n");
            DEBUG_SPI("SPIno = %d\n", SPIno);
            DEBUG_SPI("\n");
            return 0;
    }
    if(EB_ok ==0)
    {
        DEBUG_SPI("spiBlockErase Fail!\n");
        DEBUG_SPI("SPIno = %d\n", SPIno);
        DEBUG_SPI("\n");
        return 0;
    }
    else
    {
        DEBUG_SPI("spiBlockErase pass!\n");
        DEBUG_SPI("SPIno = %d\n", SPIno);
        DEBUG_SPI("\n");
        return 1;
    }
}

s32 spiReadSel(u8* pucDstBuf, u32 unAddr, u32 unSize, u8 SPIno)
{
    s32 RD_ok;

    switch (SPIno)
    {
        case 1: //spi1
            RD_ok = spiRead(pucDstBuf, unAddr, unSize);
            break;
        case 2: //spi2
            RD_ok = spiRead2(pucDstBuf, unAddr, unSize, 1);
            break;
        case 3: //spi3
            RD_ok = spiRead3(pucDstBuf, unAddr, unSize, 0);
            break;
        case 4: //spi4
            RD_ok = spiRead4(pucDstBuf, unAddr, unSize, 1);
            break;
        case 5: //spi5
            RD_ok = spiRead5(pucDstBuf, unAddr, unSize, 0);
            break;
        default:
            DEBUG_SPI("spiRead Fail!\n");
            DEBUG_SPI("SPIno = %d\n", SPIno);
            DEBUG_SPI("\n");
            return 0;
    }
    if(RD_ok ==0)
    {
        return 0;
    }
    else
    {

        return 1;
    }
}
s32 spiWriteSel(u32 unDst, u8* pucSrc, u32 unSize,u8 SPIno)
{
    s32 WT_ok;

    switch (SPIno)
    {
        case 1: //spi1
            WT_ok = spiWrite(unDst, pucSrc, unSize);
            break;
        case 2: //spi2
            WT_ok = spiWrite2(unDst, pucSrc, unSize, 1);
            break;
        case 3: //spi3
            WT_ok = spiWrite3(unDst, pucSrc, unSize, 0);
            break;
        case 4: //spi4
            WT_ok = spiWrite4(unDst, pucSrc, unSize, 1);
            break;
        case 5: //spi5
            WT_ok = spiWrite5(unDst, pucSrc, unSize, 0);
            break;
        default:
            DEBUG_SPI("spiWrite Fail!\n");
            DEBUG_SPI("SPIno = %d\n", SPIno);
            DEBUG_SPI("\n");
            return 0;
    }
    if(WT_ok ==0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

s32 spiBlockEraseTestSel(u32 unAddr,u8 SPIno)
{
    u32 i, j, k;
    u8	ucCnt;

    unAddr &= 0x000F0000;

    DEBUG_SPI("The testing Block Address = %#x\n", unAddr);

    DEBUG_SPI("SPI %d Block Erase\n",SPIno);
    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        if (spiBlockEraseSel(unAddr,SPIno)==0)
            return 0;

        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiReadSel(spiReadBuf, i, SPI_MAX_BUF_SIZE,SPIno) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Block Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<(k+10)) && (k<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("i = %#x\n", i);
                    DEBUG_SPI("Block Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)

                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
        ucCnt = 0;

        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWriteSel(i, spiWriteBuf, SPI_MAX_BUF_SIZE,SPIno) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiReadSel(spiReadBuf, i, SPI_MAX_BUF_SIZE,SPIno) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Block Erase\n");

    unAddrForTest += spiBlockSize;

    return 1;
}
/*

Routine Description:

	SPI Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s8	spiTest(void)
{
    u32 i, j, SPIno,ALL_pass;

    u8*punWriteBuf = (u8*)spiWriteBuf;


    DEBUG_SPI("***************************\n");
    DEBUG_SPI("*                         *\n");
    DEBUG_SPI("*    Serial Flash Test    *\n");
    DEBUG_SPI("*                         *\n");
    DEBUG_SPI("***************************\n\n");

    ALL_pass = 1;
    /* init spi controller */
    spiInit();
    GpioActFlashSelect |= (GPIO_ACT_FLASH_SPI|GPIO_SPI2_FrDISP|GPIO_SPI3_FrUARTDV1);
    for (SPIno = 1; SPIno <4; SPIno++)
    {
        DEBUG_SPI("Serial Flash %d TEST!\n",SPIno);
        /* mount and get ID */
        if (spiIdentificationSel(SPIno) == 0)
        {
            DEBUG_SPI("Serial Flash Mount Error!\n");
            ALL_pass=0;
        }
        else
        {
            /* use the last block for access testing */
            unAddrForTest = spiTotalSize - spiBlockSize;

            DEBUG_SPI("Addr For Test = %#x\n", unAddrForTest);

            /* set write data */
            for (i = 0; i < SPI_MAX_BUF_SIZE; i++)
                *punWriteBuf++= i;

            /* Block accessing test */

            if (spiBlockEraseTestSel(unAddrForTest,SPIno) == 1)
            {
                DEBUG_SPI("SPI %d Test Pass!\n",SPIno);
            }
            else
            {
                DEBUG_SPI("SPI %d  Test Failed!\n",SPIno);
                ALL_pass=0;
            }
        }
    }
    if(ALL_pass)
        return 1;
    return 0;
}
#endif

/*
 ****************************
 * Funcstions For Verification
 ****************************
*/

#if 0

/*

Routine Description:

	SPI Read ID Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiReadIDTest (void)
{

    if (spiIdentification() == 0)
        return 0;

    return 1;
}

/*

Routine Description:

	SPI Chip Erase Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiChipEraseTest (void)
{
    u32 i, j, k;
    u8	ucCnt;


    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);
        DEBUG_SPI("Wait about 8 Secs (Typical)\n");

        if (spiChipErase()==0)
            return 0;
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("spiReadBuf[%d] = %d \n", k, spiReadBuf[k]);
                    DEBUG_SPI("Chip Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }
        DEBUG_SPI("\n");
        ucCnt = 0;

        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Chip Erase\n");
    return 1;


}


/*

Routine Description:

	SPI Block Erase Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiBlockEraseTest (u32 unAddr)
{
    u32 i, j, k;
    u8	ucCnt;

    unAddr &= 0x000F0000;

    DEBUG_SPI("The testing Block Address = %#x\n", unAddr);

    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        if (spiBlockErase(unAddr)==0)
            return 0;

        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Block Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<(k+10)) && (k<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("i = %#x\n", i);
                    DEBUG_SPI("Block Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)

                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
        ucCnt = 0;

        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Block Erase\n");

    unAddrForTest += spiBlockSize;

    return 1;


}


/*

Routine Description:

	SPI Sector Erase Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiSectorEraseTest (u32 unAddr)
{
    u32 i, j, k;

    unAddr &= 0x000FF000;
    DEBUG_SPI("The testing Sector Address = %#x\n", unAddr);

    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        if (spiSectorErase(unAddr)==0)
            return 0;

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Sector Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<k+10) && (j<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("Sector Erase Failed!\n");
                    return 0;
                }
        }
        /* only for winbond and EON */
        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }
        }

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (j=k; (j<k+10) && (j<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    return 0;
                }
        }

    }

    DEBUG_SPI("[Pass]: Sector Erase\n");

    unAddrForTest += spiSectorSize;

    return 1;


}


/*

Routine Description:

	SPI Byte Program Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiByteProgramTest (u32 unAddr)
{
    u32 i;

    unAddr &= 0x000FF000;

    /* Sector Erase*/
    if (spiSectorErase(unAddr)==0)
        return 0;

    if (spiRead(spiReadBuf, unAddr, SPI_MAX_BUF_SIZE) == 0)
        return 0;

    for (i=0; i<SPI_MAX_BUF_SIZE; i++)
        if (spiReadBuf[i] != 0xFF)
        {
            DEBUG_SPI("Sector Erase Failed in Byte-Program Testing!\n");
            DEBUG_SPI("Finish The Verification\n");
            return 0;
        }

    /* Byte Program Test */
    if (spiByteProgram(0xA5, unAddr) == 0)
        return 0;
    if (spiByteProgram(0x5A, unAddr+1) == 0)
        return 0;
    if (spiByteProgram(0xF0, unAddr+2) == 0)
        return 0;
    if (spiByteProgram(0x0F, unAddr+3) == 0)
        return 0;

    if (spiRead(spiReadBuf, unAddr, 16) == 0)
        return 0;

    if (spiReadBuf[3] != 0xA5)
        return 0;
    if (spiReadBuf[2] != 0x5A)
        return 0;
    if (spiReadBuf[1] != 0xF0)
        return 0;
    if (spiReadBuf[0] != 0x0F)
        return 0;

    DEBUG_SPI("[Pass]: Byte Program\n");
    return 1;


}

/*

Routine Description:

	SPI Mass Data Read/ Write Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiMassRWTest (void)
{
    u32 i, j;
    u8	ucCnt;

    DEBUG_SPI("Chip Erase\n");
    DEBUG_SPI("Wait about 8 Secs (Typical)\n");
    /* erase the whole chip */
    if (spiChipErase()==0)
    {
        DEBUG_SPI("Chip erase failed.\n");
        return 0;
    }

    DEBUG_SPI("Read out to check chip erase results.\n");
    /* read out to verify erase process */
    for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
            if (spiReadBuf[j] != 0xFF)
            {
                DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                DEBUG_SPI("Quit The Verification!\n");
                DEBUG_SPI("i = %#x, j = %d\n", i, j);
                DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                return 0;
            }
    }

    ucCnt = 0;
    DEBUG_SPI("Program data to Serial Flash.\n");

    /* program the specific data into serial flash */
    for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);

        if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
            if (spiReadBuf[j] != j)
            {
                DEBUG_SPI("Program Failed in Mass RW Testing!\n");
                DEBUG_SPI("Quit The Verification!\n");
                DEBUG_SPI("i = %#x, j = %d\n", i, j);
                DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                return 0;
            }
        if (ucCnt == 5)
        {
            DEBUG_SPI("\b\b\b\b\b");
            ucCnt = 0;
        }
        else
        {
            DEBUG_SPI(".");
            ucCnt ++;
        }
    }

    DEBUG_SPI("\n[Pass]: Mass Read/ Write Test.\n");
    return 1;

}




/****************************
Funcstions For Verification
****************************/

/*

Routine Description:

	Serial flash Bit-wise Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiBitWiseTest (void)
{
    u32 i, j, k;
    u8	ucCnt;
    u8 	ucPattern;




    for (k=0; k<4; k++)
    {

        /* set the test pattern */
        switch (k)
        {
            case 0:
                ucPattern = 0;
                break;

            case 1:
                ucPattern = 0xff;
                break;

            case 2:
                ucPattern = 0xaa;
                break;

            case 3:
                ucPattern = 0x55;
                break;

        }

        DEBUG_SPI("\n@@ Cycle = %d @@\n", k);
        DEBUG_SPI("Test pattern [%#x]\n", ucPattern);

        DEBUG_SPI("Chip Erase....\n");
        DEBUG_SPI("Wait about 8 Secs (Typical)\n");

        /* erase the whole chip */
        if (spiChipErase()==0)
        {
            DEBUG_SPI("Chip erase failed.\n");
            return 0;
        }

        DEBUG_SPI("Read out to check chip erase results.\n");
        /* read out to verify erase process */
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (j=0; j<SPI_MAX_BUF_SIZE; j++)
                if (spiReadBuf[j] != 0xFF)
                {
                    DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                    DEBUG_SPI("Quit The Verification!\n");
                    DEBUG_SPI("i = %#x, j = %d\n", i, j);
                    DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                    return 0;
                }
        }

        ucCnt = 0;
        /* Set value to buffer */
        memset(spiWriteBuf, ucPattern, SPI_MAX_BUF_SIZE);

        DEBUG_SPI("Write and check the data.\n");
        /* program the specific data into serial flash */
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (j=0; j<SPI_MAX_BUF_SIZE; j++)
                if (spiReadBuf[j] != ucPattern)
                {
                    DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                    DEBUG_SPI("Quit The Verification!\n");
                    DEBUG_SPI("i = %#x, j = %d\n", i, j);
                    DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                    return 0;
                }

            if (ucCnt == 5)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                DEBUG_SPI(".");
                ucCnt ++;
            }
        }
        DEBUG_SPI("\n");

    }


    DEBUG_SPI("\n[Pass]: Serial Flash Bit-Wise Full Test.\n");

    return 1;

}

/*

Routine Description:

	Spi Read Address for test routine.

Arguments:

	None.

Return Value:

	Block Address.

*/
u32 spiReadAddr4Test (void)
{
    u8	pucBuf[4];
    u32	unLastBlk;
    u32	unRtnAddr;


    unLastBlk = spiTotalSize - spiBlockSize;

    /* Read 4 bytes */
    spiRead(pucBuf, unLastBlk, 4);

    unRtnAddr = *(u32*)pucBuf;

    unRtnAddr &= 0x000FF000;

    return unRtnAddr;

}


/*

Routine Description:

	Spi Write Address for the next test routine.

Arguments:

	unAdd - the addr will be tested next time.

Return Value:

	1 - Success
	0 - Failed

*/
u32 spiWriteAddr4Test (u32 unAddr)
{
    u8	pucBuf[4];
    u32	unLastBlk;		// = 0x000F0000;		/* start address of block-15 */
    u8 	ucWriteData, i;


    unLastBlk = spiTotalSize - spiBlockSize;

    unAddr += (spiBlockSize + spiSectorSize);

    if (unAddr >= spiTotalSize)
        unAddr = spiSectorSize;		/* start from sector 1 */

    if (spiBlockErase(unLastBlk) == 0)
    {
        DEBUG_SPI("Block erase failed in Write Block Addr 4 Test.\n");
        return 0;
    }

    /* store the next block address for test */
    for (i=0; i<4; i++)
    {
        ucWriteData = (u8)(unAddr >> (i<<3));
        if (spiByteProgram(ucWriteData, (u32)(unLastBlk +3 -i)) == 0)
        {
            DEBUG_SPI("Byte Program Failed in Write Block Addr 4 Test.\n");
            return 0;
        }

    }

    /* read out to check the result */
    if (spiRead(pucBuf, unLastBlk, 4) == 0)
    {
        DEBUG_SPI("spiRead Failed in Write Block Addr 4 Test.\n");
        return 0;
    }

    if (unAddr != *(u32*)pucBuf)
    {
        DEBUG_SPI("Write Error.\n");
        DEBUG_SPI("Saved Error\n");
        DEBUG_SPI("Saved Addr = %#x\n", *(u32*)pucBuf);
        return 0;
    }
    else
        DEBUG_SPI("The saved block address is [%#x].\n", unAddr);

    return 1;

}


void spiReadReg(u32 unAdd, u32 *unContent, u32 unLeng)
{
    u32 i;
    u32*	punAddr;

    punAddr = (u32*) unAdd;

    DEBUG_SPI("\n");
    for (i=0; i<unLeng; i++)
        DEBUG_SPI("Addr[%#x] = %#x\n", unAdd + i, *(punAddr+ i));

}

void spiWriteReg(u32 unAdd, u32 unContent)
{
    *(unsigned long *)unAdd = unContent;
}



/*

Routine Description:

	SPI verification main functions

Arguments:

	None.

Return Value:

	None.

*/
void spiVerify(void)
{

    u32 i, j;
    u8 ucVerifyIdx = 1;
    u8 ucTestItemSel;
    u8 ucMounted;

    u32 unScanAdd, unContent, unLeng;

    if (spiIdentification() == 0)
        DEBUG_SPI("ID Detection Error!\n");

    unAddrForTest = spiReadAddr4Test();
    DEBUG_SPI("Addr For Test = %#x\n", unAddrForTest);

    /* set write data */
    for (i = 0; i < SPI_MAX_BUF_SIZE; i++)
        spiWriteBuf[i]= i;

    while (ucVerifyIdx == 1)
    {
        DEBUG_SPI("\n");
        DEBUG_SPI("\n*****************************************\n");
        DEBUG_SPI("*					*\n");
        DEBUG_SPI("*	Serial Flash Verification 	*\n");
        DEBUG_SPI("*					*\n");
        DEBUG_SPI("*****************************************\n");
        DEBUG_SPI("\n");


        DEBUG_SPI("Please Select a item to test! \nKey-in a number to test.\n");
        DEBUG_SPI("[1] - 	Serial Flash - ReadID.\n");
        DEBUG_SPI("[2] -	Serial Flash - Chip Erase Test.\n");
        DEBUG_SPI("[3] -	Serial Flash - Block Erase Test.\n");
        DEBUG_SPI("[4] -	Serial Flash - Sector Erase Test.\n");
        DEBUG_SPI("[5] -	Serial Flash - Byte Program Test.\n");
        DEBUG_SPI("[6] -	Serial Flash - Full Test.\n");
        DEBUG_SPI("[7] -	Serial Flash - Mass Read/Write Test.\n");
        DEBUG_SPI("[8] -	Serial Flash - Read/Write [bit-wise] Test.\n");
        DEBUG_SPI("[9] -	Serial Flash - Adjust SPI Clock.\n");
        DEBUG_SPI("[10] -	Serial Flash - Sector Erase.\n");
        DEBUG_SPI("[11] -	Serial Flash - DMA Read.\n");
        DEBUG_SPI("[12] -	Serial Flash - DMA Write.\n");
        DEBUG_SPI("[13] -	Serial Flash - Routione Identification Test\n");
        DEBUG_SPI("[15] -	Serial Flash - Read Register.\n");
        DEBUG_SPI("[16] -	Serial Flash - Write Register.\n");
        DEBUG_SPI("[17] -	Serial Flash - Dump Write Buf.\n");
        DEBUG_SPI("[99] -	Finish verification.\n");


        scanf("%d", &ucTestItemSel);
        DEBUG_SPI("\n");

        switch (ucTestItemSel)
        {

            case	1:		/* ID Detection */
                if (spiReadIDTest() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    //ucVerifyIdx = 0;		/* End Verification */
                }

                ucMounted = 1;	/* Serial Flash is mounted. */

                break;

            case	2:
                DEBUG_SPI("Start to [Chip Erase Test]\n");
                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                if (spiChipEraseTest() == 0)
                    DEBUG_SPI("spiChipEraseTest failed\n");

                ucMounted = 1;	/* Serial Flash is mounted. */

                break;

            case	3:
                DEBUG_SPI("Start to [Block Erase Test]\n");
                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                if (spiBlockEraseTest(unAddrForTest) == 0)
                    DEBUG_SPI("spiBlockEraseTest failed\n");

                ucMounted = 1;	/* Serial Flash is mounted. */

                break;

            case	4:
                DEBUG_SPI("Start to [Secter Erase Test]\n");
                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                if (spiSectorEraseTest(unAddrForTest) == 0)
                    DEBUG_SPI("spiSectorEraseTest failed\n");

                ucMounted = 1;	/* Serial Flash is mounted. */

                break;

            case	5:
                DEBUG_SPI("Start to [Byet Program Test]\n");
                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                if (spiByteProgramTest(unAddrForTest) == 0)
                    DEBUG_SPI("spiByteProgramTest failed\n");

                ucMounted = 1;	/* Serial Flash is mounted. */

                break;

            case	6:
                DEBUG_SPI("\nStart to [Full Test]\n");

                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    return;
                }
                ucMounted = 1;	/* Serial Flash is mounted. */

                if (	(spiChipEraseTest() == 0) ||
                        (spiBlockEraseTest(unAddrForTest) == 0) ||
                        (spiSectorEraseTest(unAddrForTest) == 0) ||
                        (spiByteProgramTest(unAddrForTest) == 0) )
                {
                    DEBUG_SPI("Full Test Failed\n");
                }
                else
                    DEBUG_SPI("[Pass]: Full Test\n");

                break;

            case	7:
                DEBUG_SPI("\nStart to [Mass Read/ Write Test]\n");

                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        return;
                    }
                ucMounted = 1;	/* Serial Flash is mounted. */
                if (spiMassRWTest() == 0)
                    DEBUG_SPI("spiMassRWTest failed\n");

                break;

            case	8:
                DEBUG_SPI("\nStart to [Serial Flash Bit-Wise R/W Test]\n");

                if (ucMounted != 1)
                    if (spiIdentification() == 0)
                    {
                        DEBUG_SPI("ID Detection Error!\n");
                        DEBUG_SPI("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        return;
                    }
                ucMounted = 1;	/* Serial Flash is mounted. */
                if (spiBitWiseTest() == 0)
                    DEBUG_SPI("spiBitWiseTest failed\n");

                break;

            case	9:
                DEBUG_SPI("\nAdjust SPI Clock Frequency.\n");
                DEBUG_SPI("System Freq = %d MHz.\n", (SYS_CPU_CLK_FREQ / 1000000));
                DEBUG_SPI("Please Key-in the SPI Clock Divisor for required SPI Clock\n");
                DEBUG_SPI("[0] - [48 -> 24 MHz], [96 -> 48 MHz]\n");
                DEBUG_SPI("[1] - [48 -> 12 MHz], [96 -> 24 MHz]\n");
                DEBUG_SPI("[2] - [48 -> 8 MHz], [96 -> 16 MHz]\n");
                DEBUG_SPI("[3] - [48 -> 6 MHz], [96 -> 12 MHz]\n");
                scanf("%d", &unLeng);

                spiClkDiv = (u8) unLeng;

                if (SYS_CPU_CLK_FREQ == 48000000)
                {
                    DEBUG_SPI("System Freq = 48 MHz.\n");
                    DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (48/2/(spiClkDiv +1)));
                }
                else if (SYS_CPU_CLK_FREQ == 96000000)
                {
                    DEBUG_SPI("System Freq = 96 MHz.\n");
                    DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (96/2/(spiClkDiv +1)));
                }
                else
                    DEBUG_SPI("No Support System Freq: [%d] MHz.\n", (SYS_CPU_CLK_FREQ / 1000000));

                break;

            case	10:

                if (spiSectorErase(unAddrForTest) == 0)
                    DEBUG_SPI("Sector Erased Failed\n");

                else
                    DEBUG_SPI("Sector Erased CMP.\n");


                break;

            case	11:

                if (spiRead(spiReadBuf, unAddrForTest, SPI_MAX_BUF_SIZE) == 0)
                    DEBUG_SPI("Read Failed\n");

                {
                    u32 m;

                    for (m=0; m<16; m++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", m, spiReadBuf[m]);

                }

                break;

            case	12:

                if (spiWrite(unAddrForTest, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
                    DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                else
                    DEBUG_SPI("WinBondWrite Finished!\n");

                break;


            case	13:

            {
                u32 unTestCnt;
                u32	unID;
                u8 ucMenufID, ucDevID;
                u32	unErrIdx = 0;
                u32	unTestTimes = 100;


                DEBUG_SPI("Routione Identification Test %d Times.\n", unTestTimes);
                DEBUG_SPI("Starting....\n");
                for (unTestCnt=0; unTestCnt<unTestTimes; unTestCnt++)
                {
                    unID = spiReadID();
                    ucMenufID = (u8) (unID >> 8);
                    ucDevID = (u8) unID;

                    if ((ucMenufID != 0x8c && ucMenufID != 0xef) ||(ucDevID != 0x13 && ucDevID != 0x12))
                    {
                        DEBUG_SPI("Error! Identification Error!\n");
                        DEBUG_SPI("Test Count = %d\n", unTestCnt);
                        unErrIdx ++;
                    }

                }
                if (unErrIdx == 0)
                    DEBUG_SPI("Test %d Times --> All Pass\n", unTestTimes);

                DEBUG_SPI("Test Finished\n");

            }
            break;



            case	15:
                DEBUG_SPI("Please key-in by Hex format.\n");
                DEBUG_SPI("Address = ");
                scanf("%x", &unScanAdd);
                DEBUG_SPI("Length (word) = ");
                scanf("%x", &unLeng);
                spiReadReg(unScanAdd, &unContent, unLeng);

                break;

            case	16:
                DEBUG_SPI("Please key-in by Hex format.\n");
                DEBUG_SPI("Address = ");
                scanf("%x", &unScanAdd);
                DEBUG_SPI("\nContent = ");
                scanf("%x", &unContent);
                DEBUG_SPI("\n");
                spiWriteReg(unScanAdd, unContent);
                break;

            case 17:
                DEBUG_SPI("Dump spiWriteBuf data\n");
                {
                    u32 j;
                    for (j=0; j<16; j++)
                        DEBUG_SPI("spiWriteBuf[%d] = %#x\n", j, spiWriteBuf[j]);
                }
                break;

            case	99:
                DEBUG_SPI("Finish Verification!\n");
                ucVerifyIdx = 0;		/* End Verification */
                break;


            default:
                DEBUG_SPI("Error! Wrong item selected!\n");

                break;
        }

        if (unAddrForTest > spiTotalSize)
            unAddrForTest = 0;


    }

    spiWriteAddr4Test (unAddrForTest);


    DEBUG_SPI("\nEnd of SPI verification\n");

}
#endif
#endif /*end of #if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON))*/

