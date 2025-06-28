/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	isp.c

Abstract:

   	The routines of ISP.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

/*CY 1023*/

#include "general.h"
#include "board.h"

#include "isp.h"
#include "ispapi.h"
#include "rtcapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "smcapi.h"
#include "sysapi.h"
#include "spiapi.h"
#include "gpioapi.h"
#include "uiapi.h"
#include "p2pserver_api.h"
#include "rfiuapi.h"
#if LWIP2_SUPPORT
#include "encrptyapi.h"
#else
#include <../LwIP/netif/ppp/md5.h>
#endif

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define ISP_DEBUG_FLAG
#undef	ISP_DEBUG_FLAG

/*
*	ISP UI LIB
*/
#define ISP_UI_DIR  "\\ISP_UI"

#define PICS 11
#define PICS_ADDR 0xA0000
#define PICS_BLOCK_ADDR 0x2000

#define NOR_LOADER_ADDR 0x000  // Loader loade start loading addr  
#define NOR_FW_ADDR 0x10000  // Loader loade start loading addr  

#if(FLASH_OPTION == FLASH_NAND_9002_ADV)//Adv Nand's capacity is lager than 128Mb, so it may use "partial-file" to burn file
#define ISP_BURN_PARTIAL_FILE   1
#elif((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9001_NORMAL))
#define ISP_BURN_PARTIAL_FILE   0
#endif

#define ISP_FW_NAME_LENGTH 32

/*
*	ISP WAVE FILE
*/
#define ISP_WAV_DIR	"\\ISP_WAV"

typedef struct 
{
    char Version[ISP_FW_NAME_LENGTH + 1];
    u32 HWOP;
    u32 UIOP;
    u32 DATE;
} ISP_VERSION_INFO;

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

u8 ispParam[ISP_BUFFER_SIZE];
u8 ispAux[ISP_BUFFER_SIZE];

#if ((FLASH_OPTION == FLASH_SERIAL_EON) || (FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_WINBOND)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
s8 ispFWFileName[32]   = "\\pa9spi.bin";
s8 ispBootFileName[32] = "\\pa9spib.bin";
s8 ispUSBFileName[32]  = "\\pa9spiu.bin";
#elif ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
s8 ispFWFileName[32] = "\\pa9nand.bin";
#endif
u32 bad_block_Addr[32];

FS_DIRENT UIFileEnt[MAX_FRM_OBJ];

ISP_VERSION_INFO ispNewFWVersionInfo[1 + MAX_RFIU_UNIT];    // RX + TX * N

/**********************************************************************************************************
 * External Variable
 *********************************************************************************************************
 */
extern UI_FILE_CHAR_COUNT	uiFileListCharCount[26];	/* UI Lib updated files, the first character of files count list */
extern u32	spiWavStartAddr;
extern FRAME_BUF_OBJECT frame_buf_obj[MAX_FRM_OBJ];

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
extern SMC_REDUN_AREA smcRedunArea;
extern u32 smcBlockSize, smcPageSize;
extern u8 smcReadBuf[];
extern void smc_temp(void);
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
extern SMC_REDUN_AREA_ADV smcRedunArea;
extern u32 smcBlockSize, smcPageSize;
extern u8 smcReadBuf[];
extern void smc_temp(void);
#endif
#if TUTK_SUPPORT
extern u8 OnlineUpdateStatus;
#endif
#if USB_DONGLE_SUPPORT
extern u8* usbfwupgrade_buf;
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
s32 ispSwitchToSmcNand(void);
s32 ispUpdateCodeToSmcNand(s32, u8*);
s32 ispSwitchToSdMmc(void);
s32 ispUpdateUILIB_ToSmc(s32 , u8*,u32);
#endif


s32 find_frame_block(void);
s32 Cal_Bad_block(u32 );

/*
 *********************************************************************************************************
 * External Function prototype
 *********************************************************************************************************
 */
extern s32 dcfIntToStr(u32 , s8* , s32 );
extern s32 sysReadFile(void);
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern s32 smcPage1ARead(u32, u8*);
extern s32 smcPageProgram(u32, u8*, u16);
extern s32 smcBlockErase(u32);
#endif

/*

Routine Description:

	Compare FW code with SMC/NAND gate flash and SD

Arguments:

	codeSize - Code size.
	codeAddr - Code address.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 Cmp_FW_Data(u8* SrcAdr,u8* DstAdr,u32 length)
{
    while(length!=0)
    {
        if(*SrcAdr!=*DstAdr)
        {
            DEBUG_ISP("length %d\n",length);
            return 0;
        }
        SrcAdr++;
        DstAdr++;
        length--;
    }

    return 1;
}


/* allen 20081120 S */
#if(FLASH_OPTION == FLASH_NO_DEVICE)
s32 ispUpdate(u8 item)
{
    return 1;
}


s32 ispUpdate_UILIB(void)
{
    return 1;
}


#elif ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_WINBOND) ||(FLASH_OPTION == FLASH_SERIAL_EON))



/*

Routine Description:

	To update frame buffer object into the serial flash.

Arguments:

	unTotalSize - Total size to write.
	pucSrcUILibAddr - UI Lib location in memory.

Return Value:

	1 - Success
	0 - Failure

*/
s32 ispUpdateUILib_ToSpi(s32 unTotalSize, u8* pucSrcUILibAddr)
{
    u32 unAddr;
    u8* pucBuf = pucSrcUILibAddr;
    u8* pucBuf2 = PKBuf;
    s32 unTemp = unTotalSize;
    u8	ucCnt = 0;


    DEBUG_ISP("Program UI Lib...\n");
    /* program UI Lib into the serial flash */
    for (unAddr=spiUILibStartAddr; unTotalSize>0; unAddr+= spiPageSize, unTotalSize-=spiPageSize, pucSrcUILibAddr+= spiPageSize)
    {
        if (!spiWrite(unAddr, pucSrcUILibAddr, SPI_MAX_BUF_SIZE))
        {
            DEBUG_ISP("Error! SPI Write Error in UI Lib Update.\n");
            return 0;
        }

#ifdef DEBUG_PROGRAM_MSG
        DEBUG_ISP("\b");
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
    DEBUG_ISP("\b");
#endif

#ifdef ISP_DEBUG_FLAG
    pucSrcUILibAddr = pucBuf;
    unTotalSize = unTemp;
    /* For Test */
    for (unAddr=spiUILibStartAddr; unTotalSize>0; unAddr+= spiPageSize, unTotalSize-=spiPageSize, pucSrcUILibAddr+= spiPageSize)
    {
        if (!spiRead(PKBuf, unAddr, SPI_MAX_BUF_SIZE))
        {
            DEBUG_ISP("Error! SPI Read Error in UI Lib Update.\n");
            return 0;
        }
        DEBUG_ISP("cmp\n");
        if (memcmp(PKBuf, pucSrcUILibAddr, 256)!=0)
        {
            DEBUG_ISP("Error! compare error\n");
        }
    }

#endif
    return 1;

}
/*

Routine Description:

	To update frame buffer object into the serial flash.

Arguments:

	UIFileCount - UI File count which is related to Frame buffer object.

Return Value:

	1 - Success
	0 - Failure

*/
s32 ispUpdateFB(u32 UIFileCount)
{
    s32 unLen;
    u8* pucBuf;
    u32 unAddr;

    sysSD_Disable();
    sysSPI_Enable();

#ifdef ISP_DEBUG_FLAG
    /* Test */
    {
        u32 j;
        u8* pucBuf2 = PKBuf;
        pucBuf = spiReadBuf;

        unLen = sizeof(FRAME_BUF_OBJECT) * UIFileCount;
        memset(PKBuf, 0x5A, 256);

        for (unAddr=spiSysParaUIFBStartAddr; unLen>0 ; unAddr += spiPageSize, unLen -= spiPageSize)
        {
            if (spiRead(pucBuf2, unAddr, spiPageSize)== 0)
            {
                DEBUG_SPI("Error! SPI Read UI Frame Buffer Object Error\n");
                return 0;
            }
            for (j=0; j<spiPageSize/4; j++)
                DEBUG_ISP("spiReadBuf[%d] = %#x\n", j, *(u32*)(pucBuf2+j));
        }
    }
#endif

    pucBuf = (u8*) frame_buf_obj;
    unLen = sizeof(FRAME_BUF_OBJECT) * UIFileCount;

#ifdef ISP_DEBUG_FLAG
    DEBUG_ISP("sizeof(FRAME_BUF_OBJECT) = %#x\n", sizeof(FRAME_BUF_OBJECT));
    DEBUG_ISP("UIFileCount = %d\n", UIFileCount);
    DEBUG_ISP("unLen = %#x\n", unLen);
#endif

    DEBUG_ISP("Program Frame Buffer Objects...\n");
    for (unAddr=spiSysParaUIFBStartAddr; unLen>0 ; unAddr += spiPageSize, unLen -= spiPageSize,	pucBuf += spiPageSize)
    {
        if (spiWrite(unAddr, pucBuf, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_SPI("Error! SPI Write UI Frame Buffer Object Error\n");
            return 0;

        }
    }
#ifdef ISP_DEBUG_FLAG
    /* Test */
    {
        u32 j;
        u8* pucBuf2 = PKBuf;
        pucBuf = spiReadBuf;

        unLen = sizeof(FRAME_BUF_OBJECT) * UIFileCount;
        for (unAddr=spiSysParaUIFBStartAddr; unLen>0 ; unAddr += spiPageSize, unLen -= spiPageSize)
        {
            if (spiRead(pucBuf2, unAddr, spiPageSize)== 0)
            {
                DEBUG_SPI("Error! SPI Read UI Frame Buffer Object Error\n");
                return 0;

            }
            for (j=0; j<spiPageSize/4; j++)
                DEBUG_ISP("spiReadBuf[%d] = %#x\n", j, *(u32*)(pucBuf2+j));
        }
    }
#endif
    //sysSPI_Disable();
    sysSD_Enable();


}


/*

Routine Description:

	To update the file list character count table into the serial flash.

Arguments:

	None.

Return Value:

	1 - Success
	0 - Failure

*/
s32 ispUpdateFileListCharCountTable(void)
{
    u32 i;
    s32 unLen;
    u8* pucBuf = PKBuf;
    u8	ucCnt = 0;

    pucBuf = (u8*)uiFileListCharCount;
    unLen = sizeof(UI_FILE_CHAR_COUNT) * 26; 	/* 26 alphabets */

    sysSD_Disable();
    sysSPI_Enable();


    DEBUG_ISP("Erase Blocks...\n");
    /* erase all sectors of UI related area */
    for (i=spiSysParaFileCntListStartAddr; i<spiTotalSize; i+=spiBlockSize)
    {
        if (spiBlockErase(i)==0)
        {
            DEBUG_ISP("Error! SPI Block Erase error\n");
            return 0;
        }
#ifdef DEBUG_PROGRAM_MSG
        DEBUG_ISP("\b");
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
    DEBUG_ISP("\b");
#endif

    DEBUG_ISP("Program File List Table...\n");
    for (i = spiSysParaFileCntListStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
    {
        if (spiWrite(i, pucBuf, SPI_MAX_BUF_SIZE) == 0)
        {
            DEBUG_ISP("Error: SPI Write error!\n");
            return 0;
        }
        pucBuf = (u8*)uiFileListCharCount + SPI_MAX_BUF_SIZE;

    }


    unLen = sizeof(UI_FILE_CHAR_COUNT) * 26; 	/* 26 alphabets */
    {
        u8 *pucDstReadBuf = PKBuf0;

        for (i = spiSysParaFileCntListStartAddr; unLen > 0; i += spiPageSize, unLen -= spiPageSize)
        {
            if (spiRead(pucDstReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_ISP("Error: SPI Raed error!\n");
                return 0;
            }
            pucDstReadBuf += spiPageSize;
        }

        unLen = sizeof(UI_FILE_CHAR_COUNT) * 26;
        pucDstReadBuf = PKBuf0;

        /* compare and check the update results */
        if (memcmp(pucDstReadBuf, uiFileListCharCount, unLen)!=0)
        {
            DEBUG_ISP("Error! Update File List Char Count Error.\n");
        }
        else
        {
            DEBUG_ISP("Update File List Char Count Success.\n");
        }
    }

    //sysSPI_Disable();
    sysSD_Enable();

    return 1;

}

/*

Routine Description:

	To check the first character of UI Lib file name.
	To find out the first character and add the file count to its respond character.
	Because it doesn't restrict the letter case of file name, we have to consider both lower and upper cases.

Arguments:

	unUIFileCount - Check file total counts.

Return Value:

	None.

*/
void ispCheckUILibFileListCharCount(u32 unUIFileCount)
{
    u32	i, j;
    u8	ucRespondChar;
    u32 unSum;

    memset(uiFileListCharCount, 0, sizeof(UI_FILE_CHAR_COUNT)*26);

    for (j=2; j<unUIFileCount; j++)
    {
        if ((UIFileEnt[j].d_name[0] >= 'a') && (UIFileEnt[j].d_name[0] <= 'z'))
            ucRespondChar = UIFileEnt[j].d_name[0] - 97;

        else if ((UIFileEnt[j].d_name[0] >= 'A') && (UIFileEnt[j].d_name[0] <= 'Z'))
            ucRespondChar = UIFileEnt[j].d_name[0] - 65;

        else
        {
            DEBUG_ISP("Error! Invalid file character!\n");
            return;
        }
        uiFileListCharCount[ucRespondChar].stCount += 1;
    }

    /* update the file count list */
    i = 0;	/* temp for sum */
    for (j=1; j<26; j++)
    {
        uiFileListCharCount[j].stStart = i + uiFileListCharCount[j-1].stCount;
        i += uiFileListCharCount[j-1].stCount;
    }

#ifdef ISP_DEBUG_FLAG
    /* for debug */
    for (i=0; i<26; i++)
    {
        DEBUG_ISP("[%c] = (Start) = %#x, (Count) = %#x\n", i+65, uiFileListCharCount[i].stStart, uiFileListCharCount[i].stCount);
    }
#endif

}
/*

Routine Description:

	In-system programming update.

Arguments:

	item - No meaningful here.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ispUpdatebootload(void)
{
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - SPI_MAX_CODE_SIZE);

    FS_FILE* pFile;
    u32 codeSize;
    extern u8 ucNORInit;
    u8  tmp;


    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
    {
        if ((pFile = dcfBackupOpen((signed char*)ispBootFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        dcfBackupRead(pFile, codeAddr, pFile->size, &codeSize);
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfBackupClose(pFile);
    }
    else
    {
        if ((pFile = dcfOpen((signed char*)ispBootFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        if(dcfRead(pFile, codeAddr, pFile->size, &codeSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfClose(pFile, &tmp);
    }

    if (codeSize == 0)
    {
        DEBUG_ISP("ISP Err: Code Size is 0 Byte!\n");
        DEBUG_ISP("Quit Code Update!\n");
        return 0;
    }

    sysSD_Disable();
    sysSPI_Enable();

    osdDrawISPNow();

    if (spiMount()==0)
    {
        DEBUG_ISP("Error: Mounting Serial Flash Error!\n");
        return 0;
    }

    if (spibootCodeUpdate(codeAddr, codeSize)==0)
    {
        DEBUG_ISP("Error: Program to Serial Flash Failed.\n");
        return 0;
    }

    if (spiCmpBootWriteData(codeAddr, codeSize) == 0)
    {
        DEBUG_ISP("Error: Data compare error!\n");
        return 0;
    }
    else
        DEBUG_SPI("Programming Boot code to Serial Flash Success\n");

    //sysSPI_Disable();
    sysSD_Enable();
    return 1;

}

#if ISP_NEW_UPGRADE_FLOW_SUPPORT
typedef struct
{
	char *pCmpBuf;
	u32 BufLen;
	u32 Type;
	char *Version;
	u32 Date;
	u32 HWOP;
	u32 UIOP;
} VersionCmpInfo;

char *ispGetFWVersionStr(u32 Index)
{
	if(MAX_RFIU_UNIT < Index)
		return NULL;
		
	return ispNewFWVersionInfo[Index].Version;
}

void ispSetFWVersionStr(u32 Index, char *pStr)
{
	if(MAX_RFIU_UNIT < Index)
		return;
		
	strcpy(ispNewFWVersionInfo[Index].Version, pStr);
}

int ispChkFirmwareModel(char *LocalVer, char *ServerVer)
{
	char *pCh, *pDot, *pCh2, *pDot2;

	pCh = strstr(LocalVer, "-V") + 2;
	if(pCh == NULL)
		return ISP_ERR;

	if(strncmp(LocalVer, ServerVer, pCh - LocalVer) != 0)
		return ISP_ERR;
	
	pDot = strchr(pCh, '.') + 1;
	if(pDot == NULL)
		return ISP_ERR;

	pCh2 = ServerVer + (pCh - LocalVer);
	pDot2 = ServerVer + (pDot - LocalVer);
	
	// V'x'.xx
	if(strncmp(pCh, pCh2, 1) > 0)
		return ISP_ERR;

	// Vx.'xx'
	if(strncmp(pDot, pDot2, 2) >= 0)
		return ISP_ERR;

	return ISP_OK;
}

int ispParseUpgradeListInfo(VersionCmpInfo *pVCI)
{
	char Lebal[9], HWOP[9], UIOP[5], Date[9], Version[33];
	char *pBuf, *pCh;
	u32 i, Type;
	int ret;

	if(pVCI == NULL)
	{
		DEBUG_ISP("[E] ISP param doesn't exist. L:%d\n", __LINE__);
		return ISP_ERR;
	}

	if(pVCI->Version == NULL)
	{
		//DEBUG_ISP("[E] ISP version doesn't exist. L:%d\n", __LINE__);
		return ISP_ERR;
	}

	if(pVCI->Date == NULL)
	{
		DEBUG_ISP("[E] ISP date doesn't exist. L:%d\n", __LINE__);
		return ISP_ERR;
	}

	pBuf = pVCI->pCmpBuf;
	// special case first.
	for(i = 0; i < pVCI->BufLen; i++)
	{
		if(*(pBuf + i) == '[')
		{
			// Analyze in order
			// Lebal
			ret = strchr((pBuf + i), ']') - (pBuf + i) - 1;
			if((ret < 0) || (ret > 8))
			{
				DEBUG_ISP("[W] ISP analyze lebal failed.\n");
				return ISP_ERR;
			}

			memset(Lebal, 0x0, sizeof(char) * 9);
			strncpy(Lebal, (char *)(pBuf + i + 1), ret);
			//DEBUG_ISP("Lebal: '%s'\n", Lebal);

			i += ret + 1;	// jump over the lebal and ].

			if(strncmp(Lebal, "SP", 2) == 0)
			{
				Type = ISP_TYPE_SP;
			}
			else
			{
				continue;
			}

			if((pVCI->Type != Type) && (Type != ISP_TYPE_SP))
			{
				continue;
			}

			// HWOP
			pCh = strstr((pBuf + i), "HWOP=");
			pCh += strlen("HWOP=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 8))
			{
				DEBUG_ISP("[W] ISP analyze version failed.\n");
				return ISP_ERR;
			}

			memset(HWOP, 0x0, sizeof(char) * 9);
			strncpy(HWOP, (char *)pCh, ret);
			//DEBUG_ISP("HWOP: '%s'\n", HWOP);
			i += ret + 1;	// jump over the lebal and ].
			
			// UIOP
			pCh = strstr((pBuf + i), "UIOP=");
			pCh += strlen("UIOP=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 4))
			{
				DEBUG_ISP("[W] ISP analyze date failed.\n");
				return ISP_ERR;
			}

			memset(UIOP, 0x0, sizeof(char) * 5);
			strncpy(UIOP, (char *)pCh, ret);
			//DEBUG_ISP("UIOP: '%s'\n", UIOP);
			i += ret + 1;	// jump over the lebal and ].

			// VERSION
			pCh = strstr((pBuf + i), "VERSION=");
			pCh += strlen("VERSION=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 32))
			{
				DEBUG_ISP("[W] ISP analyze date failed.\n");
				return ISP_ERR;
			}

			memset(Version, 0x0, sizeof(char) * 33);
			strncpy(Version, (char *)pCh, ret);
			//DEBUG_ISP("VERSION: '%s'\n", Version);
			i += ret + 1;	// jump over the lebal and ].

			// DATE
			pCh = strstr((pBuf + i), "DATE=");
			pCh += strlen("DATE=");
			ret = 8;

			memset(Date, 0x0, sizeof(char) * 9);
			strncpy(Date, (char *)pCh, ret);
			//DEBUG_ISP("DATE: '%s'\n", Date);
			i += ret + 1;	// jump over the lebal and ].
			
            if(pVCI->HWOP != atoi(HWOP))
				continue;

		    if(pVCI->UIOP != atoi(UIOP))
				continue;

			if(pVCI->Date == atoi(Date))
				continue;

			netSetFirmwareName(Version, strlen(Version));
			DEBUG_ISP("Lebal: '%s'\n", Lebal);
			DEBUG_ISP("HWOP: '%s'\n", HWOP);
			DEBUG_ISP("UIOP: '%s'\n", UIOP);
			DEBUG_ISP("Version: '%s'\n", Version);
			DEBUG_ISP("DATE: '%s'\n", Date);
			return ISP_OK;
		}
	}

	// Normal rule, not including special case.
	for(i = 0; i < pVCI->BufLen; i++)
	{
		if(*(pBuf + i) == '[')
		{
			// Analyze in order
			// Lebal
			ret = strchr((pBuf + i), ']') - (pBuf + i) - 1;
			if((ret < 0) || (ret > 8))
			{
				DEBUG_ISP("[W] ISP analyze lebal failed.\n");
				return ISP_ERR;
			}

			memset(Lebal, 0x0, sizeof(char) * 9);
			strncpy(Lebal, (char *)(pBuf + i + 1), ret);
			//DEBUG_ISP("Lebal: '%s'\n", Lebal);

			i += ret + 1;	// jump over the lebal and ].

			if(strncmp(Lebal, "TX", 2) == 0)
			{
				Type = ISP_TYPE_TX;
			}
			else if(strncmp(Lebal, "RX", 2) == 0)
			{
				Type = ISP_TYPE_RX;
			}
			else
			{
				continue;
			}

			if(pVCI->Type != Type)
			{
				continue;
			}

			// HWOP
			pCh = strstr((pBuf + i), "HWOP=");
			pCh += strlen("HWOP=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 8))
			{
				DEBUG_ISP("[W] ISP analyze version failed.\n");
				return ISP_ERR;
			}

			memset(HWOP, 0x0, sizeof(char) * 9);
			strncpy(HWOP, (char *)pCh, ret);
			//DEBUG_ISP("HWOP: '%s'\n", HWOP);
			i += ret + 1;	// jump over the lebal and ].
			
			// UIOP
			pCh = strstr((pBuf + i), "UIOP=");
			pCh += strlen("UIOP=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 4))
			{
				DEBUG_ISP("[W] ISP analyze date failed.\n");
				return ISP_ERR;
			}

			memset(UIOP, 0x0, sizeof(char) * 5);
			strncpy(UIOP, (char *)pCh, ret);
			//DEBUG_ISP("UIOP: '%s'\n", UIOP);
			i += ret + 1;	// jump over the lebal and ].

			// VERSION
			pCh = strstr((pBuf + i), "VERSION=");
			pCh += strlen("VERSION=");
			ret = strchr(pCh, '\r') - pCh;
			if((ret < 0) || (ret > 32))
			{
				DEBUG_ISP("[W] ISP analyze date failed.\n");
				return ISP_ERR;
			}

			memset(Version, 0x0, sizeof(char) * 33);
			strncpy(Version, (char *)pCh, ret);
			//DEBUG_ISP("VERSION: '%s'\n", Version);
			i += ret + 1;	// jump over the lebal and ].

			// DATE
			pCh = strstr((pBuf + i), "DATE=");
			pCh += strlen("DATE=");
			ret = 8;

			memset(Date, 0x0, sizeof(char) * 9);
			strncpy(Date, (char *)pCh, ret);
			//DEBUG_ISP("DATE: '%s'\n", Date);
			i += ret + 1;	// jump over the lebal and ].

			if(pVCI->HWOP != atoi(HWOP))
				continue;

		    if(pVCI->UIOP != atoi(UIOP))
				continue;

			if(pVCI->Date >= atoi(Date))
				continue;

			netSetFirmwareName(Version, strlen(Version));
			DEBUG_ISP("Lebal: '%s'\n", Lebal);
			DEBUG_ISP("HWOP: '%s'\n", HWOP);
			DEBUG_ISP("UIOP: '%s'\n", UIOP);
			DEBUG_ISP("Version: '%s'\n", Version);
			DEBUG_ISP("DATE: '%s'\n", Date);
			return ISP_OK;
		}
	}

	return ISP_ERR;
}

/* 
 *  ispDriveUpgradeProcedure:
 *      
 *  @UpgradeList: bit[0]: RX, bit[1~4]: TX1~4, bit[28~31]: Drive type(SD or USB).
 */
int ispDriveUpgradeProcedure(u32 UpgradeList)
{
    u32 UnitSel = (UpgradeList >> 28) & 0xF;
    u32 StorageIndex;
    u32 i;

    StorageIndex = sysGetStorageIndex(UnitSel);

    if(sysGetStorageStatus(StorageIndex) == SYS_V_STORAGE_OFF)
        return 0;

    if(StorageIndex == SYS_I_STORAGE_MAIN)
    {
        for(i = 27; i >= 0; i--)
        {
            switch((1 << i) & UpgradeList)
            {
                case 1:
                    ispFirmwareUpdateFlow(0);
                    break;

                case 2:
    			case 4:
    			case 8:
    			case 16:
                default:
#if(RFIU_SUPPORT && TX_FW_UPDATE_SUPPORT)	// No RF, No Tx upgrade.
                    if((i - 1) >= MAX_RFIU_UNIT)
                        break;

                     if(dcfItemExist(ispTxFWFileName[i - 1], "rb") != 1)
                        break;

                     if(rfiuTxFwUpdateFromSD(i) == 1)
                     {
                        OSTimeDly(400);
                     }
                     OSTimeDly(40);
#endif
                    break;
            }
        }
    }
    
    return 1;
}

/*
 *	ispNetworkUpgradeProcedure:
 *
 *	UpgradeList: bit[0]: RX, bit[1~4]: TX1~4.
 */
int ispNetworkUpgradeProcedure(u32 UpgradeList)
{
	VersionCmpInfo VCI;
	u8 UpgradeInfoBuf[1024];
	char tmpBuf[33];
	u32 ReadSize;
	int i, ret;

	if(netGetVersionFileInfo(UpgradeInfoBuf, 1024, &ReadSize) < 1)
	{
		DEBUG_ISP("[W] ISP get firmware version info failed.\n");
		return ISP_ERR;
	}

	for(i = 31; i >= 0; i--)
	{
		switch((1 << i) & UpgradeList)
		{
			case 1:
				VCI.pCmpBuf = UpgradeInfoBuf;
				VCI.BufLen = ReadSize;
				VCI.Type = ISP_TYPE_RX;
				VCI.Version = uiVersion;
				VCI.Date = atoi(uiVersionTime);
                VCI.HWOP = HW_BOARD_OPTION;
#ifndef UI_PROJ_OPT			
#ifndef PROJ_OPT
                VCI.UIOP = 0;
#else
                VCI.UIOP = PROJ_OPT;
#endif
#else
                VCI.UIOP = UI_PROJ_OPT;
#endif

				if(ispParseUpgradeListInfo(&VCI) == ISP_OK)
				{
					DEBUG_ISP("Prepare %d upgrade sequence.\n", i);
					sysSetEvt(SYS_EVT_DOWNLOAD_BY_NET, 0);
				}
				break;

			case 2:
			case 4:
			case 8:
			case 16:
#if (RFIU_SUPPORT && TX_FW_UPDATE_SUPPORT)	// No RF, No Tx upgrade.
			    VCI.pCmpBuf = UpgradeInfoBuf;
			    VCI.BufLen = ReadSize;
			    VCI.Type = ISP_TYPE_TX;
			    VCI.Version = rfiuGetTxVersionName(i - 1);
			    VCI.Date = RXGetTXFWTime(i - 1);
			    VCI.HWOP = RXGetTXBorad(i - 1);
			    VCI.UIOP = RXGetTXPROJ(i - 1);

			    if(ispParseUpgradeListInfo(&VCI) == ISP_OK)
			    {
			        DEBUG_ISP("Prepare %d upgrade sequence.\n", i);
			        if(netDoDownloadFile("MD5SUM", rfiuGetTxUpgradeBuf()) < 0)
			        {
			            sysSetFWUpgradeStatus(0);   // Release status of upgrade.
			            break;
			        }

			        memset(tmpBuf, 0x0, 33);
			        sprintf(tmpBuf, "%s.bin", netGetFirmwareName());
			        if(netDoDownloadFile(tmpBuf, rfiuGetTxUpgradeBuf()) < 0)
			        {
			            DEBUG_ISP("[W] ISP file download failed.\n");
			            sysSetFWUpgradeStatus(0);   // Release status of upgrade.
			            break;
			        }

			        if(rfiuTxFwUpdateFromNet(i - 1) < 1)
			        {
			            DEBUG_ISP("[W] ISP tx upgrade failed.\n");
			            sysSetFWUpgradeStatus(0);   // Release status of upgrade.
			            break;
			        }

			        while(1)
			        {
			            ret = rfiuGetTxUpgradePercentage(i - 1);
			            if((ret < 0) || (ret == 100))
			                break;
			            OSTimeDly(20);
			        }
			        sysSetFWUpgradeStatus(0);   // Release status of upgrade.
			    }
#endif
			default:
				break;		
		}
	}

	return ISP_OK;
}

int ispUpdateFirmwareVersion(u32 UpgradeList)
{
	VersionCmpInfo VCI;
	u8 UpgradeInfoBuf[1024];
	char tmpBuf[33];
	u32 ReadSize;
	int i, ret;

	if(netGetVersionFileInfo(UpgradeInfoBuf, 1024, &ReadSize) < 1)
	{
		DEBUG_ISP("[W] ISP get firmware version info failed.\n");
		return ISP_ERR;
	}

	for(i = 31; i >= 0; i--)
	{
		switch((1 << i) & UpgradeList)
		{
			case 1:
				VCI.pCmpBuf = UpgradeInfoBuf;
				VCI.BufLen = ReadSize;
				VCI.Type = ISP_TYPE_RX;
				VCI.Version = uiVersion;
				VCI.Date = atoi(uiVersionTime);
                VCI.HWOP = HW_BOARD_OPTION;
#ifndef UI_PROJ_OPT			
#ifndef PROJ_OPT
                VCI.UIOP = 0;
#else
                VCI.UIOP = PROJ_OPT;
#endif
#else
                VCI.UIOP = UI_PROJ_OPT;
#endif

				// Clear fw version.
				netSetFirmwareName(NULL, 0);
				ispParseUpgradeListInfo(&VCI);
				ispSetFWVersionStr(i, netGetFirmwareName());
				break;

			case 2:
			case 4:
			case 8:
			case 16:
			default:
#if (RFIU_SUPPORT == 1)	// No RF, No Tx upgrade.
				VCI.pCmpBuf = UpgradeInfoBuf;
				VCI.BufLen = ReadSize;
				VCI.Type = ISP_TYPE_TX;
				VCI.Version = rfiuGetTxVersionName(i - 1);
				VCI.Date = RXGetTXFWTime(i - 1);
				VCI.HWOP = RXGetTXBorad(i - 1);
				VCI.UIOP = RXGetTXPROJ(i - 1);
				
				// Clear fw version.
				netSetFirmwareName(NULL, 0);
				ispParseUpgradeListInfo(&VCI);
				ispSetFWVersionStr(i, netGetFirmwareName());
#endif
				break;		
		}
	}

	return ISP_OK;
}
#endif

int ispFirmwareUpdateFlow(u32 Mode)
{
    int i, ret;
    u8 err, Prio;

    sysSetFWUpgradeStatus(1);

    uiCaptureVideoStop();	// close recording
#if APP_KEEP_ALIVE
    for(i = 0; i < 4; i++)	// Band SID users
        Clear_Session_Status(i);
#endif
    IOTC_DeInitialize();	// Block app connect in
#if I2C_SEM_PROTECT
    i2cWaitProcess();		// Block i2c request RTC
#endif
	spiBlkSemProcess();

    DEBUG_ISP("[I] ISP start to del useless tasks.\n");

    for(i = RFIU_TASK_PRIORITY_HIGH; i <= MAIN_TASK_PRIORITY_END; i++)
    {
        if((i == SYS_TASK_PRIORITY) ||
                (i == UI_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY))
            continue;

        DEBUG_ISP("[I] OSTaskDel %d!\n", i);
        OSTaskSuspend(i);
        OSTaskDel(i);

        WDT_Reset_Count();
    }
#if I2C_SEM_PROTECT
    i2cFinProcess();
#endif
	spiReleSemProcess();

    sysSD_Enable();
    // close WDT
    WDT_off();
    DEBUG_ISP("[I] ISP del tasks over.\n");

    //--------------------------------//
    ret = ispUpdateAllload();//usb name boot
    //-------------------------------//

    if(ret == 0)
        OSMboxPost(general_MboxEvt, "FAIL");
    else
        OSMboxPost(general_MboxEvt, "PASS");

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);

    // Wait for UI changed.
    for(i = 0; i < 5; i++)
        OSTimeDly(20);

    // open WDT
    DEBUG_ISP("[I] ISP Open WDT. Wait for rebooting.\n");
    WDT_init(); // WDT_EN

    return ret;
}


int ispFirmwareNetPrepare(u32 dummy)
{
    u32 i;
    int ret;
    u8 err;

    sysSetFWUpgradeStatus(1);

    uiCaptureVideoStop();	// close recording
#if APP_KEEP_ALIVE
    for(i = 0; i < 4; i++)	// Band SID users
        Clear_Session_Status(i);
#endif        
    IOTC_DeInitialize();	// Block app connect in
#if I2C_SEM_PROTECT
    i2cWaitProcess();		// Block i2c request RTC
#endif
	spiBlkSemProcess();

    for(i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
    {
        if ((i == SYSTIMER_TASK_PRIORITY) ||
                (i == SYS_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY) ||
#if (NIC_SUPPORT == 1)
                (i == T_LWIP_THREAD_START_PRIO)||
                (i == TCPIP_THREAD_PRIO_t)||
                (i == T_ETHERNETIF_INPUT_PRIO)||
                (i == T_LWIPENTRY_PRIOR)||
                (i == SYSBACK_NET_TASK_PRIORITY)||
#endif          
                (i == UI_TASK_PRIORITY)
           )
        {
            continue;
        }
        DEBUG_UI("ISP OSTaskDel %d!\n",i);
        OSTaskSuspend(i);
        OSTaskDel(i);
    }
#if I2C_SEM_PROTECT
    i2cFinProcess();
#endif
	spiReleSemProcess();

#if (NIC_SUPPORT)
    ret = sysback_Net_SetEvt(SYS_BACKRF_NTE_FW_UPDATE, 0, 0);
#endif
    if (ret == 0)
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
    return ISP_OK;
}


int ispFirmwareNetUpdateFlow(u32 codeSize)
{
    int i;
    u8 err;

    sysSetFWUpgradeStatus(1);

    DEBUG_ISP("[I] ISP start to del useless tasks.\n");

    for(i = RFIU_TASK_PRIORITY_HIGH; i <= MAIN_TASK_PRIORITY_END; i++)
    {
        if((i == SYS_TASK_PRIORITY) ||
                (i == UI_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY))
            continue;

        DEBUG_ISP("[I] OSTaskDel %d!\n", i);
        OSTaskSuspend(i);
        OSTaskDel(i);

        WDT_Reset_Count();
    }

    // close WDT
    WDT_off();
    DEBUG_ISP("[I] ISP del tasks over.\n");

    ispUpdateAllload_Net(codeSize);
    // Block flash
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);

    // Wait for UI changed.
    for(i = 0; i < 5; i++)
        OSTimeDly(20);

    // open WDT
    DEBUG_ISP("[I] ISP Open WDT. Wait for rebooting.\n");
    WDT_init(); // WDT_EN

    return 1;
}


s32 ispUpdateAllload(void)
{
	FS_FILE* pFile;
	MD5_CTX ctx;
    u8 *codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x1000000);
    u8 digest[16], val=0, err=0, tmp;
    char buf2[50], buf3[2], MD5_buf[33];
    u32 codeSize, waitFlag, filesize = 0;
    int i;
    extern u8 ucNORInit;

#if ((UI_VERSION == UI_VERSION_TRANWO) && (SD_AUTO_UPGRADE == 1))
    s8 RenameVerStr[32] = "finishsd.bin";
#endif

    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
    {
        if ((pFile = dcfBackupOpen((signed char*)ispUSBFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        if(dcfBackupRead(pFile, codeAddr, pFile->size, &codeSize) != 1)
        {
        	DEBUG_ISP("Error: dcf BackupRead error!\n");
        	return 0;
        }
        DEBUG_ISP("ispUSBFileName %s finded \n", ispUSBFileName);
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfBackupClose(pFile);
    }
    else
    {
        if ((pFile = dcfOpen((signed char*)ispUSBFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        if(dcfRead(pFile, codeAddr, pFile->size, &codeSize)!=1)
        {
        	DEBUG_ISP("Error: dcf Read error!\n");
        	return 0;
        }
        DEBUG_ISP("ispUSBFileName %s finded \n", ispUSBFileName);
        DEBUG_ISP("Trace: fileSize = %#x, readSize = %#x\n", pFile->size, codeSize);

        /* close file */
        dcfClose(pFile, &tmp);
    }

    filesize=pFile->size;

    if ((codeSize == 0) || (pFile->size != codeSize))
    {
        DEBUG_ISP("ISP Err: Code Size is not in right Byte!\n");
        DEBUG_ISP("Quit Code Update!\n");
        return 0;
    }

    sysSD_Disable();
#if 0//(HOME_RF_SUPPORT)
    val=homeRFGetNewFWAvailable(codeAddr, 0x8000);
    if(val == HOMERF_FLAG_NEW_FW)
    {
        waitFlag=OSFlagPend(gHomeRFStatusFlagGrp, HOMERF_FLAG_UPDATE_SUC | HOMERF_FLAG_UPDATE_FAIL, OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        printf("**** MCU update status %x \n",waitFlag);
        if(waitFlag & HOMERF_FLAG_UPDATE_FAIL)
        {
            DEBUG_ISP("MCU Update FAIL\n");
        }
        else if(waitFlag & HOMERF_FLAG_UPDATE_SUC)
        {
            DEBUG_ISP("MCU Update SUC\n");
        }

    }


#if 0
    for(i=0; i<0x8000; i++)
    {

        printf("%02x ",codeAddr[i]);
        if((i%4) == 0)
            printf("\n");
    }
#endif
    codeAddr += 0x10000;
    filesize -= 0x10000;
    codeSize -= 0x10000;
#endif

    sysSPI_Enable();

    osdDrawISPNow();

    if (spiMount()==0)
    {
        DEBUG_ISP("Error: Mounting Serial Flash Error!\n");
        return 0;
    }

    MD5Init(&ctx);

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    MD5Update(&ctx, (unsigned char*)codeAddr+0x100,filesize-0x100);
#else
    if(filesize > 0x800000)
        MD5Update(&ctx, (unsigned char*)codeAddr+0x100,0x800000-0x100);
    else
        MD5Update(&ctx, (unsigned char*)codeAddr+0x100,filesize-0x100);
#endif
    MD5Final(digest,&ctx);
    for (i = 0; i < 16; i++)
    {
        sprintf(buf3,"%02x",digest[i]);
        buf2[2*i]=buf3[0];
        buf2[2*i+1]=buf3[1];
    }
    buf2[32]='\0';

    for(i = 0; i < 32; i++)
    {
        MD5_buf[i]=(codeAddr+0xa0)[i];
    }
    MD5_buf[32]='\0';
    DEBUG_ISP("MD5:\n"\
              "PC     Compute :%s\n"\
              "Device Compute :%s\n",MD5_buf, buf2);
    if(!strncmp(MD5_buf,buf2,32))
    {
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
		if(spiFirmwareCodeUpdate(codeAddr, codeSize) < 1)
#else
        if (!spiallCodeUpdate(codeAddr, codeSize))
#endif        
        {
            DEBUG_ISP("Error: ALL Program to Serial Flash Failed.\n");
            return 0;
        }
        else
        {
            DEBUG_ISP("Programming Boot code to Serial Flash Success\n");
#if ((UI_VERSION == UI_VERSION_TRANWO) && (SD_AUTO_UPGRADE == 1))
            dcfRename((signed char*)RenameVerStr,(signed char*)ispUSBFileName);
            DEBUG_ISP("######## Rename SD Upgrade File Success !!!!! ########\n");
#endif
            //sysSPI_Disable();
            sysSD_Enable();
            return 1;
        }
    }
    else
    {
        DEBUG_ISP("SD MD5 check error!!\n");
        return 0;
    }
}
/*Firmware upgrade via internet*/
s32 ispUpdateAllload_Net(u32 codeSize)
{
    /*
    unsigned char digest[16];
    MD5_CTX ctx;
    int i;
    s32 idx;
    char buf2[50];
    char buf3[2];
    */
    u8  err;
#if USB_DONGLE_SUPPORT
    u8* codeAddr = usbfwupgrade_buf;//(u8*)(DRAM_MEMORY_END - 0x800000);
#else
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x800000);
#endif
    /*
    OSTimeDly(1);
    				MD5Init(&ctx);
    				MD5Update(&ctx, (unsigned char*)codeAddr,codeSize);
    				MD5Final(digest,&ctx);
    				for (i = 0; i < 16; i++)
    				{
    					sprintf(buf3,"%02x",digest[i]);
    					buf2[2*i]=buf3[0];
    					buf2[2*i+1]=buf3[1];
    				}
    				buf2[32]='\0';
    				printf("ISP FW MD5:%s\n",buf2);
    */


    sysSPI_Enable();
    osdDrawISPNow();
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
	if(spiFirmwareCodeUpdate(codeAddr, codeSize) < 1)
#else
	if (!spiallCodeUpdate(codeAddr, codeSize))
#endif 
    {
        DEBUG_ISP("Error: ALL Program to Serial Flash Failed.\n");
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
        return 0;
    }
    else
        DEBUG_SPI("Programming Boot code to Serial Flash Success\n");
#if TUTK_SUPPORT
    OnlineUpdateStatus=FW_UPGRADE_COMPLETE;
#endif
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
    //sysSPI_Disable();
    sysSD_Enable();
    return 1;

}
/*

Routine Description:

	In-system programming update.

Arguments:

	item - No meaningful here.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 ispUpdate(u8 item)
{
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - SPI_MAX_CODE_SIZE);

    FS_FILE* pFile;
    u32 codeSize;
    extern u8 ucNORInit;
    u8  tmp;


    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
    {
        if ((pFile = dcfBackupOpen((signed char*)ispFWFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        dcfBackupRead(pFile, codeAddr, pFile->size, &codeSize);
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);
        /* close file */
        dcfBackupClose(pFile);

    }
    else
    {
        if ((pFile = dcfOpen((signed char*)ispFWFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        if( dcfRead(pFile, codeAddr, pFile->size, &codeSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);
        /* close file */
        dcfClose(pFile, &tmp);
    }

    if (codeSize == 0)
    {
        DEBUG_ISP("ISP Err: Code Size is 0 Byte!\n");
        DEBUG_ISP("Quit Code Update!\n");
        return 0;
    }

    sysSD_Disable();
    sysSPI_Enable();

    osdDrawISPNow();

    if (spiMount()==0)
    {
        DEBUG_ISP("Error: Mounting Serial Flash Error!\n");
        return 0;
    }

    if (spiCodeUpdate(codeAddr, codeSize)==0)
    {
        DEBUG_ISP("Error: Program to Serial Flash Failed.\n");
        return 0;
    }

    if (spiCmpWriteData(codeAddr, codeSize) == 0)
    {
        DEBUG_ISP("Error: Data compare error!\n");
        return 0;
    }
    else
        DEBUG_SPI("Programming to Serial Flash Success\n");

    //sysSPI_Disable();
    sysSD_Enable();

    return 1;

}

s32 ispUpdate_UILIB(void)
{
    u8*	pucUILIbAddr = (u8*)((DRAM_MEMORY_END + 4) - spiTotalSize);
    u32 UIFileCount=0;
    s32 pictSize=0;
    s32 unTotalSize = 0;
    u32 frame_block;
    u8 i=0,Bad_block_nums;
    FS_FILE* pFile;
    u32 j;
    u8	ucRespondChar;
    u32	unSpiAddr;
    u32	unFileCountArray[26] = {0};
    u32	unFBO_Count = 0;
    u8  tmp;


    if (dcfChDir(ISP_UI_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory \\ISP_UI failed.\n");
        return 0;
    }
#if (UPDATE_LOAD_DEFAULT_SETTING==1)
    DEBUG_ISP("Reset default setting\r\n");
    uiSetDefaultSetting();
    Save_UI_Setting();
#endif
    memset((void *)&UIFileEnt, 0, sizeof(FS_DIRENT)*MAX_FRM_OBJ);
    memset((void *)&frame_buf_obj[0].stFileName, 0, sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ);

    if (dcfDir(NULL, UIFileEnt, &UIFileCount,1,1,MAX_FRM_OBJ) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");
        return 0;
    }

    /* to check UI Lib file list character count table to speed up ui refresh time */
    ispCheckUILibFileListCharCount(UIFileCount);
    /* program the table into the serial flash */
    ispUpdateFileListCharCountTable();

    /* the start address in SPI to store UI Lib */
    unSpiAddr = spiUILibStartAddr;
    /* Since 0 and 1 is 0x2E and 0x2E 0x2E */
    for(j=2; j<UIFileCount; j++)
    {
        if(j>=MAX_FRM_OBJ)
            break;

        if ((pFile = dcfOpen((signed char*)UIFileEnt[j].d_name, "rb")) == NULL)
            return 0;

        {

            if ((UIFileEnt[j].d_name[0] >= 'a') && (UIFileEnt[j].d_name[0] <= 'z'))
                ucRespondChar = UIFileEnt[j].d_name[0] - 97;

            else if ((UIFileEnt[j].d_name[0] >= 'A') && (UIFileEnt[j].d_name[0] <= 'Z'))
                ucRespondChar = UIFileEnt[j].d_name[0] - 65;

            unFBO_Count = uiFileListCharCount[ucRespondChar].stStart + unFileCountArray[ucRespondChar];

        }

        if(dcfRead(pFile, pucUILIbAddr, pFile->size, &pictSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, pictSize);

        frame_buf_obj[unFBO_Count].stFileLen = pFile->size;
        frame_buf_obj[unFBO_Count].stLenInSPI = ((pFile->size+ spiPageSize-1)/spiPageSize)*spiPageSize; //Align page
        frame_buf_obj[unFBO_Count].stPageStartAddr = unSpiAddr;
        memcpy ((void *)frame_buf_obj[unFBO_Count].stFileName, (void *)UIFileEnt[j].d_name, 12);

        unSpiAddr += frame_buf_obj[unFBO_Count].stLenInSPI;	 /* update the addr */
        pucUILIbAddr += frame_buf_obj[unFBO_Count].stLenInSPI;	 /* addr in memory */
        unTotalSize += frame_buf_obj[unFBO_Count].stLenInSPI;	 /* addr in memory */

        unFileCountArray[ucRespondChar]++;

        dcfClose(pFile, &tmp);

    }

    DEBUG_ISP("\n>>>> Update UI Lib, Please Wait....(%#x)\n", unTotalSize);
    /* update frame buffer object into serial flash */
    ispUpdateFB(UIFileCount);
    if(unTotalSize>SPI_UI_LIB_EACH_SIZE * SPI_UI_LIB_LANGUAGE_COUNT)
    {
        DEBUG_ISP("Error: UI real LIB SIZE (%#x) is larger than SPI_UI_LIBRARY_SIZE (%#x).\n", unTotalSize, SPI_UI_LIB_EACH_SIZE * SPI_UI_LIB_LANGUAGE_COUNT);
        return -1;
    }

    sysSD_Disable();
    sysSPI_Enable();

    pucUILIbAddr = (u8*)((DRAM_MEMORY_END + 4) - spiTotalSize);
    if (!ispUpdateUILib_ToSpi(unTotalSize, pucUILIbAddr))
    {
        //sysSPI_Disable();
        sysSD_Enable();
        return -1;
    }

    //sysSPI_Disable();
    sysSD_Enable();

    DEBUG_ISP(">>>> Update UI Lib Completed.\n\n");

    return 1;

}

#elif ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) || (FLASH_OPTION == FLASH_NAND_9002_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_ADV))



u8 check_region(u32 addr,u32 length,u8 bad_counts)
{
    u8 i,j=0;
    for(i=0; i<bad_counts; i++)
    {
        if(addr<=bad_block_Addr[i] && bad_block_Addr[i]<addr + length)
            j++;
    }

    return j;
}

void recheck_loop(u8 bad_nums,u8 index)
{
    u8 last_nums;
    u8 i=0;
    while(bad_nums)
    {
        //check how many bad block locate in original region
    RECHECK:
        last_nums=i;
        i=check_region(frame_buf_obj[index-2].sector_addr,frame_buf_obj[index-2].len_in_nand,bad_nums);
        if(i!=0)
        {
            if(i==last_nums)
                break;
            frame_buf_obj[index-2].len_in_nand += (i-last_nums)*smcBlockSize;
            goto RECHECK;
        }
        else
            break;
    }
}
#if ISP_BURN_PARTIAL_FILE

s32 ispFindStartValidFrameBlock(u32 start_addr)
{
    u32 i,paramAddr;

    //i = SMC_UI_LIBRARY_ADDR;
    i = start_addr;
    while (smcPage1ARead(i, ispParam) == 0)
    {
        /* invalid block */
        i += smcBlockSize;
        if (i >= (start_addr + SMC_MAX_BURN_SIZE))
        {
            DEBUG_ISP("Error: Too many invalid blocks to parameter block.\n");
            return 0;
        }
    }
    paramAddr = i;	/* Save address of parameter block */
    smcBlockErase(paramAddr);

    return paramAddr;
}

s32 ispBurningFrameObj(u32 paramAddr)
{
    u32 i,j;
    u8* frmptr= (u8*)frame_buf_obj[0].file_name;
    i=paramAddr;	/* Save address of parameter block */

    //smcStart();
    smcReset();

    for(j=0; j<(sizeof(FRAME_BUF_OBJECT)*(MAX_FRM_OBJ) + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE; j++)
    {

        if (smcPageProgram(i, frmptr, 0xffff)==0)
        {
            DEBUG_ISP("Program error\n");
            return 0;
        }

        frmptr+=SMC_MAX_PAGE_SIZE;
        i+=SMC_MAX_PAGE_SIZE;

    }
}

s32 ispBurningUILIB(s32 uiSize, u8* pictAddr,u32 paramAddr)
{
    s32 i,j;
    s32 auxAddr,validSize=0,tempaddr;

    //smcStart();
    smcReset();

    // Write pict
    auxAddr=paramAddr+smcBlockSize;

    for (i = auxAddr ; i < (auxAddr + SMC_MAX_BURN_SIZE); i += smcBlockSize)
    {
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (smcPage1ARead(i, ispAux))
        {
            /* valid block */
            smcBlockErase(i);
            validSize += smcBlockSize;
        }
#elif(FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(i, smcReadBuf);
        if (ispAux[0] == 0xff)		/* block status */
        {
            /* valid block */
            smcBlockErase(i);
            validSize += smcBlockSize;
        }
        else
        {
            DEBUG_ISP("ISP ERR: Page1A Read Error on Addr %#x\n", i);
            return 0;
        }
#endif
        if(validSize>=uiSize)        // We get enough space
            break;
    }

    tempaddr = auxAddr;
    for (i = 0 ; i <uiSize/SMC_MAX_PAGE_SIZE ; i++)
    {

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        while (smcPage1ARead(auxAddr, ispAux) == 0)
        {
            /* invalid block */
            DEBUG_ISP("Error: Encounter Bad Block %x.\n",auxAddr);
            auxAddr += smcBlockSize;
            // Modified the corresponding frame buffer object
            if (auxAddr >= (tempaddr + SMC_MAX_BURN_SIZE))
            {
                DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                return 0;
            }

        }
#elif(FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(auxAddr, smcReadBuf);
        while(smcReadBuf[0] != 0xff)
        {
            /* invalid block */
            DEBUG_ISP("Error: Encounter Bad Block %x.\n",auxAddr);
            auxAddr += smcBlockSize;
            // Modified the corresponding frame buffer object
            if (auxAddr >= (tempaddr + SMC_MAX_BURN_SIZE))
            {
                DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                return 0;
            }
            smcRedunAreaRead(auxAddr, smcReadBuf);
        }
#endif
        smcPageProgram(auxAddr, pictAddr, 0xffff);

        auxAddr += smcPageSize;
        pictAddr += smcPageSize;
    }

    ///smc_temp();
    DEBUG_ISP("ISP PICT Completed.\n");
    return 1;
}

s32 ispUpdate_UILIB(void)
{
    u8* pictAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_MAX_BURN_SIZE);
    u32 UIFileCount=0;
    //    FS_DIRENT UIFileEnt[MAX_FRM_OBJ];
    s32 pictSize=0;
    s32 total_size=0,uilibsize=0;
    u32 frame_block;
    u8 i=0,Bad_block_nums;
    FS_FILE* pFile;
    u8 j;
    u32 offset;
    u32 StartBurnAddr = SMC_UI_LIBRARY_ADDR;
    u8  tmp;


    if (dcfChDir(ISP_UI_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory \\DCIM failed.\n");
        return 0;
    }

    memset((void *)&UIFileEnt, 0, sizeof(FS_DIRENT)*MAX_FRM_OBJ);
    memset((void *)&frame_buf_obj[0].file_name, 0, sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ);
    if (dcfDir(NULL, UIFileEnt, &UIFileCount,1,1,MAX_FRM_OBJ) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");
        return 0;
    }

    // Reservefor record frame buffer object
    // pictAddr+=  ((sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE)*SMC_MAX_PAGE_SIZE;
    // Since 0 and 1 is 0x2E and 0x2E 0x2E
    for(j=2; j<UIFileCount; j++)
    {
        if(j>=MAX_FRM_OBJ)
            break;

        if ((pFile = dcfOpen((signed char*)UIFileEnt[j].d_name, "rb")) == NULL)
            return 0;

        // Fill the frame buffer object
        offset = ((pFile->size+ SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE)*SMC_MAX_PAGE_SIZE; //Align page
        frame_buf_obj[j-2].file_length=pFile->size;
        frame_buf_obj[j-2].len_in_nand=offset;
        memcpy ((void *)frame_buf_obj[j-2].file_name, (void *)UIFileEnt[j].d_name, 12);
        //DEBUG_ISP("frame_buf_obj[%d].filename = %s\n", j-2, frame_buf_obj[j-2].file_name);
        if((total_size + offset) > (SMC_MAX_BURN_SIZE - (8*smcBlockSize)))
        {
            DEBUG_ISP("Burning file\n");
            //Burning NAND
            if(ucNANDInit)
            {
                smcStart();
                ucNANDInit=0;
            }
            else
            {
                sysSD_Disable();
                sysNAND_Enable();
            }
            pictAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_MAX_BURN_SIZE);
            if(!ispBurningUILIB(total_size,pictAddr,StartBurnAddr))
                return -1;

            StartBurnAddr = ((StartBurnAddr + SMC_MAX_BURN_SIZE + smcBlockSize -1)/smcBlockSize)*smcBlockSize;
            Bad_block_nums=Cal_Bad_block(StartBurnAddr);
            sysNAND_Disable();
            sysSD_Enable();

            frame_buf_obj[j-2].sector_addr=StartBurnAddr+smcBlockSize;     // should consider bad block ststus
            recheck_loop(Bad_block_nums,j);
            uilibsize += total_size;
            total_size = 0;
            DEBUG_ISP("StartBurnAddr = %#x\n", StartBurnAddr);
            DEBUG_ISP("Bad_block_nums = %d\n", Bad_block_nums);
        }

        if(dcfRead(pFile, pictAddr, pFile->size, &pictSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, pictSize);
        pictAddr+=frame_buf_obj[j-2].len_in_nand;

        // Find the desire block for saving frame object
        if(j==2)
        {
            if(ucNANDInit)
            {
                smcStart();
                ucNANDInit=0;
            }
            else
            {
                sysSD_Disable();
                sysNAND_Enable();
            }

            /** frame buffer info addr at SMC_UI_LIBRARY_ADDR **/
            /** UILIB data at (SMC_UI_LIBRARY_ADDR + smcBlockSize + offset) **/
            frame_block=ispFindStartValidFrameBlock(StartBurnAddr);
            StartBurnAddr = frame_block + smcBlockSize;
            offset = sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ;
            StartBurnAddr = ((StartBurnAddr + offset + smcBlockSize -1)/smcBlockSize)*smcBlockSize;
            StartBurnAddr = ((StartBurnAddr + smcBlockSize -1)/smcBlockSize)*smcBlockSize;

            Bad_block_nums=Cal_Bad_block(StartBurnAddr);
            DEBUG_ISP("StartBurnAddr = %#x\n", StartBurnAddr);
            DEBUG_ISP("Bad_block_nums = %d\n", Bad_block_nums);
            sysNAND_Disable();
            sysSD_Enable();
            frame_buf_obj[j-2].sector_addr=StartBurnAddr+smcBlockSize;     // should consider bad block ststus
        }
        else
        {
            if(total_size!=0)
                frame_buf_obj[j-2].sector_addr= frame_buf_obj[j-3].sector_addr + frame_buf_obj[j-3].len_in_nand;
        }

        recheck_loop(Bad_block_nums,j);
        total_size+=frame_buf_obj[j-2].len_in_nand;
        frame_buf_obj[j-2].fb_magic=0xA5A55A5A;
        dcfClose(pFile, &tmp);

    }

    //Burning NAND
    if(total_size!=0)
    {
        if(ucNANDInit)
        {
            smcStart();
            ucNANDInit=0;
        }
        else
        {
            sysSD_Disable();
            sysNAND_Enable();
        }
        pictAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_MAX_BURN_SIZE);

        if(!ispBurningUILIB(total_size,pictAddr,StartBurnAddr))
            return -1;
    }

    ///DEBUG_ISP("Burning Frame Obj Data\n");
    ispBurningFrameObj(frame_block);
    sysNAND_Disable();
    sysSD_Enable();
    uilibsize += total_size;

    if(uilibsize>SMC_UI_LIBRARY_SIZE)
    {
        DEBUG_ISP("Error: UI real LIB SIZE is larger then SMC_UI_LIBRARY_SIZE %x.\n",uilibsize);
        return -1;
    }
    else
        DEBUG_ISP("Burning Frame Obj Data OK\n");
    /*
    {
    	u32 i;

    	for (i = 2; i< 65; i++)
    	{
    		printf("frame_buf_obj[%d].filename = %s\n", i-2, frame_buf_obj[i-2].file_name);
    		//printf("frame_buf_obj[%d].sector_addr = %d\n", i-2, frame_buf_obj[i-2].sector_addr);
    		//printf("frame_buf_obj[%d].file_length = %d\n", i-2, frame_buf_obj[i-2].file_length);
    		//printf("frame_buf_obj[%d].len_in_nand = %d\n", i-2, frame_buf_obj[i-2].len_in_nand);
    	}
    }
    */
    return 1;
}

s32 Cal_Bad_block(u32 start_addr)
{
    u32 i=0,addr;

    for (addr = start_addr; addr < (start_addr + SMC_MAX_BURN_SIZE); addr += smcBlockSize)
    {

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_block_Addr[i]=addr;
            i++;
            continue;
        }
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(addr, smcReadBuf);
        if (smcReadBuf[0] != 0xff)		/* block status */
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_block_Addr[i]=addr;
            i++;
            continue;
        }
#endif

    }

    return i;
}


#else
s32 ispUpdate_UILIB(void)
{
    u8* pictAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_RESERVED_SIZE);
    u32 UIFileCount=0;
    //    FS_DIRENT UIFileEnt[MAX_FRM_OBJ];
    s32 pictSize=0;
    s32 total_size=0;
    u32 frame_block;
    u8 i=0,Bad_block_nums;
    FS_FILE* pFile;
    u8 j;
    u8  tmp;


    if (dcfChDir(ISP_UI_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory \\DCIM failed.\n");
        return 0;
    }

    memset((void *)&UIFileEnt, 0, sizeof(FS_DIRENT)*MAX_FRM_OBJ);
    memset((void *)&frame_buf_obj[0].file_name, 0, sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ);
    if (dcfDir(NULL, UIFileEnt, &UIFileCount,1,1,MAX_FRM_OBJ) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");
        return 0;
    }
    // Reservefor record frame buffer object
    // pictAddr+=  ((sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE)*SMC_MAX_PAGE_SIZE;
    // Since 0 and 1 is 0x2E and 0x2E 0x2E
    for(j=2; j<UIFileCount; j++)
    {
        if(j>=MAX_FRM_OBJ)
            break;

        if ((pFile = dcfOpen((signed char*)UIFileEnt[j].d_name, "rb")) == NULL)
            return 0;

        if(dcfRead(pFile, pictAddr, pFile->size, &pictSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, pictSize);
        // Fill the frame buffer object
        frame_buf_obj[j-2].file_length=pFile->size;
        frame_buf_obj[j-2].len_in_nand=((pFile->size+ SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE)*SMC_MAX_PAGE_SIZE; //Align sector
        pictAddr+=frame_buf_obj[j-2].len_in_nand;

        memcpy ((void *)frame_buf_obj[j-2].file_name, (void *)UIFileEnt[j].d_name, 12);
        // Find the desire block for saving frame object
        if(j==2)
        {
            if(ucNANDInit)
            {
                smcStart();
                ucNANDInit=0;
            }
            else
            {
                sysSD_Disable();
                sysNAND_Enable();
            }
            frame_block=find_frame_block();
            Bad_block_nums=Cal_Bad_block(frame_block);
            sysNAND_Disable();
            sysSD_Enable();
            frame_buf_obj[j-2].sector_addr=frame_block+smcBlockSize;     // should consider bad block ststus
        }
        else
        {
            frame_buf_obj[j-2].sector_addr= frame_buf_obj[j-3].sector_addr + frame_buf_obj[j-3].len_in_nand;
        }

        recheck_loop(Bad_block_nums,j);
        total_size+=frame_buf_obj[j-2].len_in_nand;
        frame_buf_obj[j-2].fb_magic=0xA5A55A5A;
        dcfClose(pFile, &tmp);

    }

    if(total_size>SMC_UI_LIBRARY_SIZE)
    {
        DEBUG_ISP("Error: UI real LIB SIZE is larger then SMC_UI_LIBRARY_SIZE %x.\n",total_size);
        return -1;
    }

    //Burning NAND
    if(ucNANDInit)
    {
        smcStart();
        ucNANDInit=0;
    }
    else
    {
        sysSD_Disable();
        sysNAND_Enable();
    }
    pictAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_RESERVED_SIZE);


    //memcpy((void *)&UIFileEnt, , sizeof(FRAME_BUF_OBJECT)*MAX_FRM_OBJ);
    if(!ispUpdateUILIB_ToSmc(total_size,pictAddr,frame_block))
        return -1;

    return 1;

}

s32 find_frame_block(void)
{
    u32 i,paramAddr;

    i = SMC_UI_LIBRARY_ADDR;
    while (smcPage1ARead(i, ispParam) == 0)
    {
        /* invalid block */
        i += smcBlockSize;
        if (i >= SMC_RESERVED_SIZE)
        {
            DEBUG_ISP("Error: Too many invalid blocks to parameter block.\n");
            return 0;
        }
    }
    paramAddr = i;	/* Save address of parameter block */
    smcBlockErase(paramAddr);

    return paramAddr;
}

s32 Cal_Bad_block(u32 start_addr)
{
    u32 i=0,addr;

    for (addr = start_addr; addr < SMC_RESERVED_SIZE; addr += smcBlockSize)
    {

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_block_Addr[i]=addr;
            i++;
            continue;
        }
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(addr, smcReadBuf);
        if (smcReadBuf[0] != 0xff)		/* block status */
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_block_Addr[i]=addr;
            i++;
            continue;
        }
#endif

    }

    return i;
}

s32 ispUpdateUILIB_ToSmc(s32 uiSize, u8* pictAddr,u32 paramAddr )
{
    s32 i,j;
    s32 auxAddr,validSize=0;
    u32* ptr = (u32*) ispParam;
    u8* frmptr= (u8*)frame_buf_obj[0].file_name;

    /* load parameter block */

    i=paramAddr;	/* Save address of parameter block */

    //smcStart();
    smcReset();

    for(j=0; j<(sizeof(FRAME_BUF_OBJECT)*(MAX_FRM_OBJ) + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE; j++)
    {

        if (smcPageProgram(i, frmptr, 0xffff)==0)
        {
            DEBUG_ISP("Program error\n");
            return 0;
        }

        frmptr+=SMC_MAX_PAGE_SIZE;
        i+=SMC_MAX_PAGE_SIZE;

    }
    // Write pict
    auxAddr=paramAddr+smcBlockSize;

    for (i = auxAddr ; i < SMC_RESERVED_SIZE; i += smcBlockSize)
    {
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (smcPage1ARead(i, ispAux))
        {
            /* valid block */
            smcBlockErase(i);
            validSize += smcBlockSize;
        }
#elif(FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(i, smcReadBuf);
        if (ispAux[0] == 0xff)		/* block status */
        {
            /* valid block */
            smcBlockErase(i);
            validSize += smcBlockSize;
        }
        else
        {
            DEBUG_ISP("ISP ERR: Page1A Read Error on Addr %#x\n", i);
            return 0;
        }
#endif
        if(validSize>=uiSize)        // We get enough space
            break;
    }

    for (i = 0 ; i <uiSize/SMC_MAX_PAGE_SIZE ; i++)
    {

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        while (smcPage1ARead(auxAddr, ispAux) == 0)
        {
            /* invalid block */
            DEBUG_ISP("Error: Encounter Bad Block %x.\n",auxAddr);
            auxAddr += smcBlockSize;
            // Modified the corresponding frame buffer object
            if (auxAddr>= SMC_RESERVED_SIZE)
            {
                DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                return 0;
            }

        }
#elif(FLASH_OPTION == FLASH_NAND_9002_ADV)
        smcRedunAreaRead(auxAddr, smcReadBuf);
        while(smcReadBuf[0] != 0xff)
        {
            /* invalid block */
            DEBUG_ISP("Error: Encounter Bad Block %x.\n",auxAddr);
            auxAddr += smcBlockSize;
            // Modified the corresponding frame buffer object
            if (auxAddr>= SMC_RESERVED_SIZE)
            {
                DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                return 0;
            }
            smcRedunAreaRead(auxAddr, smcReadBuf);
        }
#endif
        smcPageProgram(auxAddr, pictAddr, 0xffff);

        auxAddr += smcPageSize;
        pictAddr += smcPageSize;
    }

    smc_temp();
    DEBUG_ISP("ISP PICT Completed.\n");
    return 1;
}
#endif

/*

Routine Description:

	Read FW code out from SMC/NAND gate flash to memory

Arguments:

	codeSize - Code size.
	codeAddr - Code address.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 ispRead_FW_Data(u8* codeAddr,s32 codeSize,u32 aux_addr)
{
    s32 i;
    u32 smcBlockMask=smcBlockSize-1;

    /* read code block */
    for (i = aux_addr+smcBlockSize; codeSize > 0; i += smcPageSize, codeAddr += smcPageSize, codeSize -= smcPageSize)
    {
        if ((i & smcBlockMask) == 0)
        {
            while (smcPage1ARead(i, codeAddr) == 0)
            {
                /* invalid block */
                i += smcBlockSize;
                if (i >= SMC_CODE_END_ADDR)
                {
                    //DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                    return 0;
                }
            }
        }
        else
        {
            smcPage1ARead(i, codeAddr);
        }
    }

}


/*

Routine Description:

	Update code to SMC/NAND gate flash.

Arguments:

	codeSize - Code size.
	codeAddr - Code address.

Return Value:

	0 - Failure.
	1 - Success.

*/
u32 ispUpdateCode(s32 codeSize, u8* codeAddr)
{
    s32 i;
    s32 paramAddr, auxAddr;
    s32 validSize = 0;
    u32* ptr = (u32*) ispParam;

    /* load parameter block */
    i = 0x00000000;
    while (smcPage1ARead(i, ispParam) == 0)
    {
        /* invalid block */
        i += smcBlockSize;
        if (i >= SMC_CODE_END_ADDR)
        {
            DEBUG_ISP("Error: Too many invalid blocks to parameter block.\n");
            return 0;
        }
    }
    paramAddr = i;	/* Save address of parameter block */

    /* load auxiliary block */
    do
    {
        /* invalid block */
        i += smcBlockSize;
        if (i >= SMC_CODE_END_ADDR)
        {
            DEBUG_ISP("Error: Too many invalid blocks to auxiliary block.\n");
            return 0;
        }
    }
    while (smcPage1ARead(i, ispAux) == 0);
    auxAddr = i;	/* Save address of auxiliary block */

    /* Erase parameter block and auxiliary block */
    smcBlockErase(paramAddr);
    smcBlockErase(auxAddr);

    /* update code size parameter */
    *ptr = codeSize;

    /* write parameter block */
    smcPageProgram(paramAddr, ispParam, 0xffff);

    /* erase code block */
    for (i = auxAddr + smcBlockSize; i < SMC_CODE_END_ADDR; i += smcBlockSize)
    {
        if (smcPage1ARead(i, ispAux))
        {
            /* valid block */
            smcBlockErase(i);
            validSize += smcBlockSize;
        }
    }

    /* check if valid size is greater than code size */
    if (validSize < codeSize)
    {
        DEBUG_ISP("Error: Reserved size is less than code size.\n");
        return 0;
    }

    /* write code block */
    for (i = auxAddr + smcBlockSize; codeSize > 0; i += smcPageSize, codeAddr += smcPageSize, codeSize -= smcPageSize)
    {
        if ((i % smcBlockSize) == 0)
        {
            while (smcPage1ARead(i, ispAux) == 0)
            {
                /* invalid block */
                i += smcBlockSize;
                if (i >= SMC_CODE_END_ADDR)
                {
                    DEBUG_ISP("Error: Too many invalid blocks to code block.\n");
                    return 0;
                }
            }
        }

        smcPageProgram(i, codeAddr, 0xffff);
    }
    return auxAddr;
}

/*

Routine Description:

	Updated FW while booting if exist FW file

Arguments:

	None.

Return Value:
     -2 - Compare fail
     -1 - Update FW data Fail
	0 - No file exit.
	1 - Success

*/
s32 ispUpdate(u8 item)
{
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_CODE_END_ADDR*2);
    u8* tempcodeAddr = (u8*)((DRAM_MEMORY_END + 4) - SMC_CODE_END_ADDR*3);
    FS_FILE* pFile;
    u32 codeSize;
    s32 Aux_start_address;
    u32 level;
    u8  ledon=0, i;
    u8  tmp;

    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
    {
        if ((pFile = dcfBackupOpen((signed char*)ispFWFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: Update File Not Found or File open error!\n");
            DEBUG_ISP("Quit FW Update\n");
            return 0;
        }
        DEBUG_ISP("Updating...Wait...\n");

        osdDrawISPNow();
        osdDrawWarningMessage("UPDATING.. WAIT..",2,TRUE, FALSE);
#if (UPDATE_LOAD_DEFAULT_SETTING==1)
        DEBUG_ISP("Reset default setting\r\n");
        uiSetDefaultSetting();
        Save_UI_Setting();
#endif
        dcfBackupRead(pFile, codeAddr, pFile->size, &codeSize);
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfBackupClose(pFile);
    }
    else
    {
        if ((pFile = dcfOpen((signed char*)ispFWFileName, "rb")) == NULL)
        {
            DEBUG_ISP("Error: Update File Not Found or File open error!\n");
            DEBUG_ISP("Quit FW Update\n");
            return 0;
        }
        DEBUG_ISP("Updating...Wait...\n");

        osdDrawISPNow();
        osdDrawWarningMessage("UPDATING.. WAIT..",2,TRUE, FALSE);
#if (UPDATE_LOAD_DEFAULT_SETTING==1)
        DEBUG_ISP("Reset default setting\r\n");
        uiSetDefaultSetting();
        Save_UI_Setting();
#endif
        if(dcfRead(pFile, codeAddr, pFile->size, &codeSize)==0)
        {
            return 0;
        }
        DEBUG_ISP("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfClose(pFile, &tmp);
    }

    if(ucNANDInit)
    {
        smcStart();
        ucNANDInit=0;
    }
    else
    {
        sysSD_Disable();
        sysNAND_Enable();
    }

    Aux_start_address=ispUpdateCode((s32)codeSize, codeAddr);

    if(!Aux_start_address)
        return -1;

    ispRead_FW_Data(tempcodeAddr,codeSize,Aux_start_address);

    if(!Cmp_FW_Data(codeAddr,tempcodeAddr,codeSize))
        return -2;

    sysNAND_Disable();
    sysSD_Enable();

    DEBUG_ISP("Update Completed!\n");

    return 1;
}
#endif



