/*

Copyright (c) 2009  MARS Technologies, Inc.

Module Name:

	dcf.c

Abstract:

   	The routines of DCF.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "ramdiskapi.h"
#include "rtcapi.h"
#include "sysapi.h"
#include "uiapi.h"
#include "ispapi.h"     // ispUSBFileName
#include "SDK_recordapi.h"

#include "..\..\inc\mars_controller\mars_dma.h"

#if(UI_VERSION == UI_VERSION_TRANWO)
#include "..\..\ui\inc\uiact_project.h"
#endif

#if PIR_FALSETRIG_TEST
#include "rfiuapi.h"
#endif
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

typedef struct _DCFFILETYPE_INFO
{
    const char *pSubFileStr;
    u8  uiStrLength;
} DCFFILETYPE_INFO, *PDCFFILETYPE_INFO;

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_EVENT *dcfReadySemEvt, *dcfWriteSemEvt;
OS_FLAG_GRP *gDcfFlagGrp = NULL;

//DCF_FILE_INFO dcfFileInfoTab[DCF_MAX_MULTI_FILE];

//---used for DIR/FILE LINK
FS_DIRENT dcfDirEnt[DCF_DIRENT_MAX];	// For dir
FS_DIRENT dcfFileEnt[DCF_FILEENT_MAX];	// For playback
FS_DIRENT dcfChFileEnt[DCF_MAX_MULTI_FILE];	// For recording

DCF_LIST_DIRENT dcfListDirEnt[DCF_DIRENT_MAX];
DCF_LIST_FILEENT dcfListFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT dcfListChFileEnt[DCF_MAX_MULTI_FILE];

DCF_LIST_DIRENT *dcfListDirEntHead = NULL;
DCF_LIST_DIRENT *dcfListDirEntTail = NULL;
DCF_LIST_DIRENT *dcfListPlaybackDirHead = NULL;
DCF_LIST_DIRENT *dcfListPlaybackDirTail = NULL;
DCF_LIST_DIRENT *dcfPlaybackCurDir = NULL;
DCF_LIST_DIRENT *dcfVideoRecCurDir = NULL;
DCF_LIST_DIRENT *dcfListDelDirEnt = NULL;	// it means the same as dcfListFileEntHead.

DCF_LIST_FILEENT *dcfListFileEntHead = NULL;	// abs link
DCF_LIST_FILEENT *dcfListFileEntTail = NULL;	// abs link
DCF_LIST_FILEENT *dcfPlaybackListFileEntHead = NULL;	// playback link
DCF_LIST_FILEENT *dcfPlaybackListFileEntTail = NULL;	// playback link
DCF_LIST_FILEENT *dcfPlaybackCurFile = NULL;

// For normal
u32 dcfVideoDirNumLast;  //表示最新開目錄的編號,下次開目錄以此為基準以流水號編織.
u32 dcfVideoDirCnt;
// For search and playback
u32 dcfFileOldestIndex;
u32 dcfVideoFileCnt;
// For record
u8 dcfFileNewestIndex;

u32 dcfDirRescanSwitch;
u8 dcfNewFile = 0;
u8 dcfFileName[MULTI_CHANNEL_MAX][32];
u8 dcfChannelRecType[MULTI_CHANNEL_MAX];
s32 dcfOverWriteOP;
u8 dcfOWclk;

u8 dcfDayMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char dcfDecChar[] = "0123456789";

//---used for Saving file information
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
FS_DIRENT dcfFileEntBG[DCF_FILEENT_MAX];
DCF_LIST_FILEENT dcfListFileEntBG[DCF_FILEENT_MAX];
DCF_LIST_FILEENT *dcfListFileEntBGHead = NULL;
DCF_LIST_FILEENT *dcfListFileEntBGTail = NULL;
DCF_LIST_FILEENT *dcfPlaybackCurBGFile = NULL;

DCF_PLAYBACK_DAY_INFO dcfPlaybackDayInfo[31];
DCF_LIST_DIRENT *dcfListDirCurEntBGScan = NULL;

u32 dcfVideoFileBGCnt;
char gsDirName[9] = GS_DCFDIRNAME;
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
DCF_LIST_LOGENT dcfListLogEnt[DCF_LOGENT_MAX];
FS_DIRENT dcfLogEnt[DCF_LOGENT_MAX];
u32 dctTotalLogCount;
u32 dcfLogCount;
u32 dcfLogFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
DCF_LIST_LOGENT *dcfListLogEntHead = NULL; //指向file link的第一個.
DCF_LIST_LOGENT *dcfListLogEntTail = NULL; //指向file link的最後一個.
DCF_LIST_LOGENT *dcfListLogEntRead = NULL;

DEF_LOG_INFO dcfLogFileInfo;
u32 dcfListLogCount;
int dcfLogWriteBufPos = 0;
int dcfLogInitReady = 0;
int dcfLogRefreshFlag = 0;
s8 gsLogDirName[9] = GS_DCF_LOGDIR_NAME;
#endif

#if RX_SNAPSHOT_SUPPORT
int dcfPhotoInitReady = 0;
s8 gsPhotoDirName[9] = GS_DCF_PHOTODIR_NAME;
#endif

u8 ucSDCInit = 1;
u8 ucSDCUnInit = 0;
u8 ucNANDInit = 1;
u8 ucNORInit = 1;

u32 gFileType_count[DCF_FILE_TYPE_MAX];

DCFFILETYPE_INFO gdcfFileType_Info[DCF_FILE_TYPE_MAX] =
{
    {	"ASF",	3	},
    {	"JPG",	3	},
    {	"WAV",	3	},
    {	"RAW",	3	},
    {	"AVI",	3	},
    {	"MOV",	3	},
    {   "AXF",  3   },  //bad file of "ASF"
//  {   "TIF",  3	},
//  {   "THM",  3   },
//	{	"MP4",	3	},
};

/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */
#if PIR_FALSETRIG_TEST
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
#endif

extern u8 SystemLogData[SYSTEM_LOG_DATA_SIZE];
extern BOOLEAN MemoryFullFlag;

#if ISFILE
extern s8 gsParseDirName[9];
extern s32 InitSetProgram(u8 item);
#endif

#if(UI_VERSION == UI_VERSION_TRANWO)
extern u8 uiCurRecType[MULTI_CHANNEL_MAX];
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
s32 dcfLogDirFileInit(void);
s32 dcfLogFileOWDel(void);
#endif

#if RX_SNAPSHOT_SUPPORT
s32 dcfPhotoDirInit(void);
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 *Extern Function prototype
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

int dcfRWTest(void)
{
    FS_FILE* pFile;
    u32 tmp, i;
    u8 tmp1;
#if 0
    u32 time1, time2, size = 0x1000000, ws;
    u8 *codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x2000000);
    DEBUG_GREEN("[I] File size: %d (Bytes)\n", size);
    for(i = 0; i < 50; i++)
    {
        sysDeadLockMonitor_OFF();
        if((pFile = dcfOpen((signed char*)"\\MFG\\testBIG1.bin", "wb")) == NULL)
        {
            DEBUG_ISP("Error: dcf open error!\n");
            return 0;
        }

        time1 = OSTimeGet();
        for(ws = 0; ws < size; ws += 0x8000)
            dcfWrite(pFile, codeAddr, 0x8000, &tmp);
        time2 = OSTimeGet();
        dcfClose(pFile, &tmp1);
        DEBUG_YELLOW("[%d]Tick: %d (x50ms)\n", i, time2 - time1);
        time2 = ((time2 = ((time2 - time1) / 20)))?time2:1;
        DEBUG_CYAN("[%d] Speed: %d (KBs)\n", i, (size/1024) /time2);
        sysDeadLockMonitor_ON();
    }
#elif 0
    u32 time1, time2;
    u8 *codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x2000000);


    if((pFile = dcfOpen((signed char*)"\\testBIG.bin", "wb")) == NULL)
    {
        DEBUG_ISP("Error: dcf open error!\n");
        return 0;
    }
    time1 = OSTimeGet();
    dcfWrite(pFile, codeAddr, 0x2000000, &tmp);
    time2 = OSTimeGet();
    dcfClose(pFile, &tmp1);
    DEBUG_GREEN("file size: %d(Bytes)\n", 0x2000000);
    DEBUG_GREEN("dcfOpen-dcfWrite-dcfClose: %d\n", time2 - time1);
    DEBUG_CYAN("Speed: %d (KBs)\n", (0x2000000/((time2 - time1)/20)) / 1024);


    if((pFile = dcfOpen((signed char*)"\\testBIG.bin", "rb")) == NULL)
    {
        DEBUG_ISP("Error: dcf open error!\n");
        return 0;
    }
    time1 = OSTimeGet();
    dcfRead(pFile, codeAddr, 0x2000000, &tmp);
    time2 = OSTimeGet();
    dcfClose(pFile, &tmp1);

    DEBUG_GREEN("file size: %d(Bytes)\n", 0x2000000);
    DEBUG_GREEN("dcfOpen-dcfRead-dcfClose: %d\n", time2 - time1);
    DEBUG_CYAN("Speed: %d (KBs)\n", (0x2000000/((time2 - time1)/20)) / 1024);

    time1 = OSTimeGet();
    sdcDevMulWrite(0, 0, 0x2000000/0x200, codeAddr);
    time2 = OSTimeGet();
    DEBUG_GREEN("file size: %d(Bytes)\n", 0x2000000);
    DEBUG_GREEN("sdcDevMulWrite: %d\n", time2 - time1);
    DEBUG_CYAN("Speed: %d (KBs)\n", (0x2000000/((time2 - time1)/20)) / 1024);

    time1 = OSTimeGet();
    for(i = 0; i < 10; i++)
        sdcDevMulRead(0, 0, (0x2000000/0x200)/10, codeAddr);
    time2 = OSTimeGet();
    DEBUG_GREEN("file size: %d(Bytes)\n", 0x2000000);
    DEBUG_GREEN("sdcDevMulWrite: %d\n", time2 - time1);
    DEBUG_CYAN("Speed: %d (KBs)\n", (0x2000000/((time2 - time1)/20)) / 1024);
#else
    u8 dataBuf[0xFF];
    for(i = 0; i < 0xFF; i++)
        dataBuf[i] = i;

    if((pFile = dcfOpen((signed char*)"\\MFG\\test.bin", "wb")) == NULL)
    {
        DEBUG_ISP("Error: dcf open error!\n");
        return 0;
    }

    dcfWrite(pFile, dataBuf, 0xFF, &tmp);
    dcfClose(pFile, &tmp1);
    DEBUG_GREEN("[INF] DCF Write Test END\n");

    memset(&dataBuf, 0x0, 0xFF);
    if((pFile = dcfOpen((signed char*)"\\MFG\\test.bin", "rb")) == NULL)
    {
        DEBUG_ISP("Error: dcf open error!\n");
        return 0;
    }
    dcfRead(pFile, dataBuf, 0xFF, &tmp);
    for(i = 0; i < 0xFF; i++)
    {
        if(!(i%16) && i)
            DEBUG_DCF("\n");
        DEBUG_DCF("%02x ", dataBuf[i]);
    }
    DEBUG_DCF("\n");
    dcfClose(pFile, &tmp1);
#endif

    return 1;
}


/*
Routine Description:
	Initialize DCF.

Arguments:
	None.

Return Value:
	<=0 - Failure.
	1   - Success.
*/

int dcfInit(u32 type)
{
    u8 BitMap_return, err;
    int ret;
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
    int status, count;
#endif

    if (gDcfFlagGrp == NULL)
        gDcfFlagGrp = OSFlagCreate(0x00000000, &err);

    // initialize exif file
    exifFileInit();

    dcfStorageType = type;

    if ((ret = dcfFsInit()) < 0)
        return ret;

    switch (dcfStorageType)
    {
        case STORAGE_MEMORY_RAMDISK:
            ramDiskInit(); /* initialize ram disk */
            ramDiskInit1(); /* initialize ram disk 1 */
            dcfFormat("ram:0:");
            dcfFormat("ram1:0:");
            break;

        case STORAGE_MEMORY_SD_MMC:
            dcfStorageType = STORAGE_MEMORY_SD_MMC;
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
            sdcInit();  //Initialize SD controller and create semephore.
            if(ucSDCInit)
                ucSDCInit = 0;
            else
            {
                sysNAND_Disable();
                sysSD_Enable();
            }

            count=0;
            do
            {
                DEBUG_DCF("===>Try SDC Mount\n");
                if(count > 0)
                    sdcTryInvertSDClk ^=0x01;
                status = sdcMount();
                count++;
            }
            while((status < 0) && (count <= 3));

            if (status == -2)
            {
                DEBUG_DCF("Error: No SD Card. #1\n");
                return -2;
            }
            else if(status < 0)
            {
                DEBUG_DCF("Error: SD/MMC mount error.\n");
                return -1;
            }
#endif
            break;
#if USB_HOST_MASS_SUPPORT
        case STORAGE_MEMORY_USB_HOST:
            dcfStorageType = STORAGE_MEMORY_USB_HOST;
            break;
#endif

        case STORAGE_MEMORY_SMC_NAND:
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
            dcfStorageType = STORAGE_MEMORY_SMC_NAND;
            gInsertNAND= 1;  //此時無插入SD card, 初始化 NAND/SPI flash.
            if (ucNANDInit)
            {
                smcStart();
                ucNANDInit=0;
            }
            else
            {
                sysSD_Disable();
                sysNAND_Enable();
            }

            BitMap_return=smcMount();
            if(BitMap_return== 0)
            {
                DEBUG_DCF("Error: SMC/NAND gate flash mount error.\n");
                return -1;
            }
#endif
            break;
    }

    if((ret = dcfCacheInit()) < 0)
        return ret;

    // for initial setting
    if((ret = dcfCheckUnit()) < 0)
        return ret;

#if ISFILE
    //DEBUG_DCF("---InitSetProgram---\n");
    InitSetProgram(1);		 // Check whether drec.inf exists ot not. If exist then update initial setting

#if(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    if(gsParseDirName[0]!=0x00)
        strcpy(gsDirName, (char *) gsParseDirName);
#endif
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
    dcfLogInitReady = 0;
    if((ret = dcfLogDirFileInit()) < 0)
      return ret;
    dcfLogInitReady=1;
#endif

#if RX_SNAPSHOT_SUPPORT
    dcfPhotoInitReady = 0;
    if((ret = dcfPhotoDirInit()) < 0)
        return ret;
    dcfPhotoInitReady = 1;
#endif

    if((ret = dcfDirInit()) < 0)
        return ret;

    if(dcfFileInit(dcfListDirEntTail, CURRENTFILEENTRY) < 0)
    {
        return -1;
    }

    if(dcfStorageType == STORAGE_MEMORY_SMC_NAND)
    {
        if(BitMap_return == 2 && gInsertNAND == 1)    // Smc Format while making BitMap
            userClickFormat=1;
    }

    return 1;
}

s32 dcfBackupInit(u32 type)
{
    int ret;
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
    int count, status;
#endif
    //---------------------//

    if((ret = dcfBackupFsInit(type)) < 0)
        return ret;

    switch (type)
    {
        case STORAGE_MEMORY_SD_MMC:
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
            sdcInit();  //Initialize SD controller and create semephore.
            if (ucSDCInit)
            {
                ucSDCInit = 0;
            }
            else
            {
                sysNAND_Disable();
                sysSD_Enable();
            }

            count=0;
            do
            {
                DEBUG_DCF("===>Try SDC Mount\n");
                if(count > 0)
                    sdcTryInvertSDClk ^=0x01;
                status=sdcMount();
                count++;
            }
            while( (status < 0) && (count<=3) );

            if ( status == -2)
            {
                DEBUG_DCF("Error: No SD Card. #1\n");
                return -2;
            }
            else if(status < 0)
            {
                DEBUG_DCF("Error: SD/MMC mount error.\n");
                return -1;
            }
#endif
            break;
#if USB_HOST_MASS_SUPPORT
        case STORAGE_MEMORY_USB_HOST:
            break;
#endif
    }

    // for initial setting
    if((ret = dcfCheckBackupUnit()) < 0)
        return ret;

    if ((ret = dcfBackupDirInit()) < 0)
        return ret;

    return 1;
}

/*
Routine Description:
	Uninitialize DCF.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
int dcfUninit(void)
{
    /*CY 1023*/
    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
        dcfCacheClean();

    switch (dcfStorageType)
    {
        case STORAGE_MEMORY_RAMDISK:
            ramDiskUninit(); /* uninitialize ram disk */
            ramDiskUninit1(); /* uninitialize ram disk 1 */
            break;

        case STORAGE_MEMORY_SD_MMC:
            sdcUnmount();
            break;

        case STORAGE_MEMORY_SMC_NAND:
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
            smcUnmount();
#endif
            break;
        case STORAGE_MEMORY_USB_HOST:
            //usbhostUnmount();
            break;

    }

    //FS_Exit(); // exit file system; SEM Always ON, there is no need to close
    dcfFileTypeCount_Clean();

    dcfVideoDirCnt = dcfVideoFileCnt = 0;
    dcfListDirEntTail = dcfListDirEntHead = dcfPlaybackCurDir = NULL;
    dcfListFileEntTail = dcfListFileEntHead = NULL;
    dcfPlaybackListFileEntHead = dcfPlaybackListFileEntHead = dcfPlaybackCurFile = NULL;

    return 1;
}


s32 dcfBackupUninit(int BackupDeviceType)
{
    /*CY 1023*/
    switch (BackupDeviceType)
    {
        case STORAGE_MEMORY_SD_MMC:
            sdcUnmount();
            break;

        case STORAGE_MEMORY_SMC_NAND:
            break;

        case STORAGE_MEMORY_USB_HOST:
            //usbhostUnmount();
            break;
    }

    return 1;
}

/*
Routine Description:
	Convert integer to string.

Arguments:
	val - The integer value to be converted.
	str - The string.
	cnt - The character count of the string.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfIntToStr(u32 val, s8 *str, s32 cnt)
{
    s8 i, dig[16];

    for (i = 0; i < 8; i++)
    {
        dig[i] = dcfDecChar[val % 10];
        val /= 10;
    }

    for (i = (cnt - 1); i >= 0; i--)
        *str++ = dig[i];

    *str = '\0';

    return 1;
}

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
void dcfLogTest(u8 *cmd)
{
    int Nday,Nblock,Rblock,Tblock;
    RTC_DATE_TIME   localTime;

    DEBUG_UI("dcfLogTest: %s\n", cmd);
    if(!strncmp((char*)cmd,"INFO ", strlen("INFO ")))
    {
        sscanf((char*)cmd, "INFO %d", &Nday);
        Tblock=dcfGetLogFileInfo(Nday);
        DEBUG_DCF2("Tblock=%d\n",Tblock);
    }
    else if(!strncmp((char*)cmd,"READ ", strlen("READ ")))
    {
        int ReadSize;
        sscanf((char*)cmd, "READ %d %d", &Nday,&Nblock);
        ReadSize=dcfReadLogFile(Nday,Nblock,&Rblock,&Tblock);
        ReadSize = ReadSize;	// avoid warning message
        DEBUG_DCF2("ReadSize=%d,Rblock=%d,Tblock=%d\n",ReadSize,Rblock,Tblock);
    }
    else if(!strncmp((char*)cmd,"WRITE", strlen("WRITE")))
    {
        RTC_Get_Time(&localTime);
        memset(SystemLogData,0,SYSTEM_LOG_DATA_SIZE);
        sprintf((char *) SystemLogData,"%04d%02d%02d %02d%02d%02d",localTime.year+2000,localTime.month,localTime.day,(u32)localTime.hour, localTime.min, localTime.sec);
        dcfWriteLogFile(SystemLogData, SYSTEM_LOG_DATA_SIZE);
    }

}

static s32 dcfListLogEntInsert(DCF_LIST_LOGENT* ins)
{
    DCF_LIST_LOGENT* cur;

    if (dcfListLogCount == 0)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListLogEntHead = ins;
    }
    else
    {
        cur = dcfListLogEntHead;
        do
        {
            if (ins->LogNum < cur->LogNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListLogEntHead)
                {
                    /* insert before head */
                    dcfListLogEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListLogEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}

s32 dcfCheckLogFile(FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j;
    s8 dirNumStr[16];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".LOG") != 0)
        return 0;

    /* get dcf directory number string */
    for (j = 0; j < 8; j++)
    {
        dirNumStr[j] = pDirEnt->d_name[j];
        if( (dirNumStr[j] < '0') || (dirNumStr[j] > '9') )
            return 0;
    }
    dirNumStr[8] = '\0';

    *pDirNum = (u32)atoi((const char*)dirNumStr);

    if(*pDirNum < 1)
        return 0;

    return 1;
}

s32 dcfLogDirFileInit(void)
{
    u32 LogNum;
    u32 i;
    s8  DirName[32];
    DCF_LIST_LOGENT* dcfListDirEntTemp = NULL;

    //--------------------//
    DEBUG_DCF2("dcfLogDirInit: Begin \n");
    dcfLogFileNumLast = 0; /* 0001 - 1 */
    dcfListLogEntTail = dcfListLogEntHead =NULL;
    memset_hw((void*)dcfLogEnt, 0, sizeof(FS_DIRENT) * DCF_LOGENT_MAX);
    memset(dcfLogBuf_Wr,0,DCF_LOGBUF_SIZE);

    sprintf ((char*)DirName, "\\%s", gsLogDirName);

    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if (dcfMkDir(DirName) < 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n", DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_LOGENT_MAX);	// Not nesscessarily

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }

    /* list directory \DCIM */
    if (dcfDir(DirName, dcfLogEnt, &dcfLogCount,1,1,DCF_LOGENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory %s failed.\n",DirName);
        //return 0;
    }
    DEBUG_DCF2("--->dcfDir Log complete:%d.\n",dcfLogCount);

    // sort the directory and create the directory list
    memset_hw((void*)dcfListLogEnt, 0, sizeof(DCF_LIST_LOGENT) * DCF_LOGENT_MAX);

    dcfListLogCount = 0;
    dctTotalLogCount = 0;

    // List the valid directory and make the linked list

    for(i = 0; i < dcfLogCount; i++)
    {
        if( (dcfLogEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0 )
        {
            // check if valid dcf directory
            if (dcfCheckLogFile(&dcfLogEnt[i], &LogNum) == 0)
            {
                continue;
            }

            if(dcfListLogCount>=DCF_LOGENT_MAX)
            {
                break;
            }
            // assign last directory number
            dcfLogFileNumLast = (dcfLogFileNumLast < LogNum) ? LogNum : dcfLogFileNumLast;

            // insert directory entry to the list
            dcfListLogEnt[dcfListLogCount].used_flag = DCF_FILE_USE_CLOSE;
            dcfListLogEnt[dcfListLogCount].LogNum = LogNum;
            dcfListLogEnt[dcfListLogCount].pDirEnt = &dcfLogEnt[i];
            dcfListLogEntInsert(&dcfListLogEnt[dcfListLogCount]);
            dcfListLogCount++;
            dctTotalLogCount ++;
        }
    }
    DEBUG_DCF2("--->dcfListLogCount=%d,dctTotalLogCount=%d\n",dcfListLogCount,dctTotalLogCount);

    if(dcfListLogEntHead != NULL)
    {
        /* set tail entry of directory list */
        dcfListLogEntTail = dcfListLogEntHead->prev;
    }

    while(dctTotalLogCount >DCF_LOG_RESV_NUM)
    {
        dcfLogFileOWDel();
    }

#if 1
    {
        if (dcfListLogEntHead == NULL)
            return 1;

        dcfListDirEntTemp = dcfListLogEntHead;
        do
        {
            DEBUG_DCF2("Log Name: %s\n", dcfListDirEntTemp->pDirEnt->d_name);
            dcfListDirEntTemp = dcfListDirEntTemp->next;
        }
        while(dcfListDirEntTemp != dcfListLogEntHead );
    }
#endif

    return 1;
}

s32 dcfLogFileOWDel(void)
{
    //---------------------------------------------//

    DEBUG_DCF2("Enter dcfLogFileDel....\n");

    if (dcfListLogEntHead == NULL)
    {
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }

    if(0 == dcfLogDel(dcfListLogEntHead->pDirEnt->d_name))
    {
        DEBUG_DCF("Trace: dcfLogFileOWDel fail.\n");
        return 0;
    }

    dcfListLogEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfListLogEntHead->used_flag = DCF_FILE_USE_NONE;

    if(dcfListLogEntHead != dcfListLogEntTail)
    {
        //above 2 files
        dcfListLogEntHead->prev->next = dcfListLogEntHead->next;
        dcfListLogEntHead->next->prev = dcfListLogEntHead->prev;

        dcfListLogEntHead  = dcfListLogEntHead->next;
        dcfListLogEntTail = dcfListLogEntHead->prev;
        dctTotalLogCount --;
    }
    else
    {
        //just a file
        dcfListLogEntHead = dcfListLogEntTail = NULL;
        dcfLogFileNumLast =0;
        dctTotalLogCount=0;
    }

    DEBUG_DCF2("Leave dcfLogFileDel\n");
    return 1;
}

int dcfWriteLogFile(u8 *event,int size)
{
    RTC_DATE_TIME   localTime;
    unsigned short val;
    INT8U   err;
    FS_FILE* pFile;
    u32 writesize;
    char LogName[32],TargetName[64];
    int DevIdx;
    int temp;
    u8  tmp;
    int Temp_FileEntSect;
    int Temp_FileEntSect_offset;

    //-------------------------//

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
        DevIdx=DCF_GetDeviceIndex("sdmmc");
    else
        DevIdx=DCF_GetDeviceIndex("usbfs");

    if( (dcfListLogEntTail==NULL) && (dcfLogInitReady==1) )
    {
        DEBUG_DCF2("Create Empty Log file\n");
        if((pFile = dcfCreateNextLog())==NULL)
        {
            DEBUG_DCF("===dcfWriteLogFile Error!!===\n");
        }
        dcfClose(pFile, &tmp);
    }

    RTC_Get_Time(&localTime);
    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    if ( (dcfListLogEntTail != NULL) && (dcfLogInitReady==1) && (dcfListLogEntTail->pDirEnt->fsFileCreateDate_YMD !=val) )
    {
        DEBUG_DCF2("===Day change,Split to new log file===\n");

        strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);
        strcpy((char*)TargetName, "\\");
        strcat((char*)TargetName, (char*)gsLogDirName);
        strcat((char*)TargetName, "\\");
        strcat((char*)TargetName, LogName);

        DEBUG_DCF2("Append old Log file:%s\n",TargetName);

        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);

            memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
            dcfLogWriteBufPos += size;
            if(dcfLogWriteBufPos > DCF_LOGBUF_SIZE)
                dcfLogWriteBufPos= DCF_LOGBUF_SIZE;
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        if(dcfLogRefreshFlag)
            dcfSeek(pFile, -DCF_LOGBUF_SIZE, FS_SEEK_CUR);

        dcfWrite(pFile, dcfLogBuf_Wr, dcfLogWriteBufPos, &writesize);
        dcfClose(pFile, &tmp);

        if((pFile = dcfCreateNextLog())==NULL)
        {
            DEBUG_DCF("===dcfWriteLogFile Error!!===\n");
        }
        dcfClose(pFile, &tmp);

        dcfLogWriteBufPos=0;

        memset(dcfLogBuf_Wr,0,DCF_LOGBUF_SIZE);
        memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
        dcfLogWriteBufPos += size;

        dcfLogRefreshFlag=0;

    }
    else if(dcfLogWriteBufPos < DCF_LOGBUF_SIZE)
    {
        memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
        dcfLogWriteBufPos += size;
    }
    else if(dcfLogInitReady==0)
    {
        DEBUG_DCF2("### dcfLogDirFileInit is not ready,Drop this event!###\n");
    }
    else
    {

        strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);

        strcpy((char*)TargetName, "\\");
        strcat((char*)TargetName, (char*)gsLogDirName);
        strcat((char*)TargetName, "\\");
        strcat((char*)TargetName, LogName);

        DEBUG_DCF2("Append old Log file:%s\n",TargetName);


        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
            dcfLogWriteBufPos += size;
            if(dcfLogWriteBufPos > DCF_LOGBUF_SIZE)
                dcfLogWriteBufPos= DCF_LOGBUF_SIZE;
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        if(dcfLogRefreshFlag)
            dcfSeek(pFile, -DCF_LOGBUF_SIZE, FS_SEEK_CUR);

        dcfWrite(pFile, dcfLogBuf_Wr, dcfLogWriteBufPos, &writesize);
        dcfClose(pFile, &tmp);


        DEBUG_DCF2("Log file position:%d\n",pFile->filepos);

        dcfLogWriteBufPos=0;
        memset(dcfLogBuf_Wr,0,DCF_LOGBUF_SIZE);
        memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
        dcfLogWriteBufPos += size;

        dcfLogRefreshFlag=0;
    }
    //DEBUG_DCF("--->dctTotalLogCount=%d\n",dctTotalLogCount);
    while(dctTotalLogCount >DCF_LOG_RESV_NUM)
    {
        dcfLogFileOWDel();
    }
    //dcfCacheClean();
    OSSemPost(dcfReadySemEvt);

    return 1;
}


int dcfLogFileSaveRefresh(void)
{
    INT8U   err;
    FS_FILE* pFile;
    u32 writesize;
    char LogName[32],TargetName[64];
    int DevIdx;
    int offset;
    int temp;
    u8  tmp;
    int Temp_FileEntSect;
    int Temp_FileEntSect_offset;
    //-------------------------//

    if(dcfLogWriteBufPos==0)
        return 0;

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
        DevIdx=DCF_GetDeviceIndex("sdmmc");
    else
        DevIdx=DCF_GetDeviceIndex("usbfs");


    if( (dcfListLogEntTail==NULL) || (dcfLogInitReady==0) )
    {
        OSSemPost(dcfReadySemEvt);
        return 0;
    }


    strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);

    strcpy((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)gsLogDirName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, LogName);

    DEBUG_DCF2("-->dcfLogFileSaveRefresh Refresh old Log file:%s,%d\n",TargetName,dcfLogRefreshFlag);


    if(dcfLogRefreshFlag)
    {
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
#if 1
        offset = dcfTell(pFile); //scan address
        if(offset>=DCF_LOGBUF_SIZE)
            offset -=DCF_LOGBUF_SIZE;
        else
            offset=0;
        dcfSeek(pFile, offset, FS_SEEK_SET);
#endif
        dcfWrite(pFile, dcfLogBuf_Wr, DCF_LOGBUF_SIZE, &writesize);
        dcfClose(pFile, &tmp);


    }
    else
    {
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
        dcfWrite(pFile, dcfLogBuf_Wr, DCF_LOGBUF_SIZE, &writesize);
        dcfClose(pFile, &tmp);
    }

    DEBUG_DCF2("Log file position/size:%d,%d\n",pFile->filepos,pFile->size);

    dcfLogRefreshFlag=1;
    //dcfCacheClean();
    OSSemPost(dcfReadySemEvt);

    return 1;
}

FS_FILE* dcfCreateNextLog(void)
{
    char newLogName[32], newTargetName[64];
    int scan;
    RTC_DATE_TIME   localTime;
    unsigned short val;
    FS_FILE* pFile;
    //==============================================//

    DEBUG_DCF2("\nEnter dcfCreateNextLog\n");

    dcfLogFileNumLast++;
    dcfIntToStr(dcfLogFileNumLast, (s8 *)newLogName, 8);
    strcat((char*)newLogName, ".LOG");

    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (char*)gsLogDirName);
    strcat((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newLogName);

    if ((pFile = dcfOpen((s8 *)newTargetName, "w")) == NULL)
    {
        /* create next file error */
        DEBUG_DCF("Error: create next Log %s error\n", newTargetName);
        return NULL;
    }
    DEBUG_DCF2("Log name: %s \n",newTargetName);

    /* list file newly created */
    scan=0;
    while(dcfLogEnt[dcfLogCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfLogCount ++;
        if(dcfLogCount >= DCF_LOGENT_MAX)
        {
            scan ++;
            dcfLogCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfLogEnt is full!\n");
            dcfLogCount=0;
            return 0;
        }
    }
    dcfLogEnt[dcfLogCount].used_flag = DCF_FILE_USE_CREATE;

    scan=0;
    while(dcfListLogEnt[dcfListLogCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListLogCount ++;
        if(dcfListLogCount>=DCF_LOGENT_MAX)
        {
            scan ++;
            dcfListLogCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListDirEnt is full!\n");
            dcfListLogCount=0;
            return 0;
        }
    }
    dcfListLogEnt[dcfListLogCount].used_flag = DCF_FILE_USE_CREATE;

    dcfLogEnt[dcfLogCount].FAT_DirAttr = 0x20;
    strcpy(dcfLogEnt[dcfLogCount].d_name, newLogName);

    dcfLogEnt[dcfLogCount].FileEntrySect=pFile->FileEntrySect;

    RTC_Get_Time(&localTime);
    val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

    dcfLogEnt[dcfLogCount].fsFileCreateTime_HMS =  val;
    dcfLogEnt[dcfLogCount].fsFileModifiedTime_HMS = val;

    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    dcfLogEnt[dcfLogCount].fsFileCreateDate_YMD = val;
    dcfLogEnt[dcfLogCount].fsFileModifiedDate_YMD = val;

    /* insert directory entry to the list */
    dcfLogEnt[dcfLogCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListLogEnt[dcfListLogCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListLogEnt[dcfListLogCount].pDirEnt = &dcfLogEnt[dcfLogCount];
    dcfListLogEnt[dcfListLogCount].LogNum = dcfLogFileNumLast;
    dcfListLogEntInsert(&dcfListLogEnt[dcfListLogCount]);

    dcfListLogEntTail = dcfListLogEntHead->prev;

    DEBUG_DCF2("Log Name: %s\n",dcfListLogEntTail->pDirEnt->d_name);

    dctTotalLogCount ++;

    DEBUG_DCF2("Leave dcfCreateNextLog\n\n");

    return pFile;
}
//-------------------------------------------//
int dcfGetLogFileInfo(int Nday)
{
    FS_FILE* pFile;
    int Tblock;
    u8  tmp;

    pFile=dcfOpenLogFile(Nday);
    dcfClose(pFile, &tmp);

    strcpy((char *) dcfLogFileInfo.FileName,dcfListLogEntRead->pDirEnt->d_name);

    dcfLogFileInfo.DateTime.year=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD >> 9)+1980;
    dcfLogFileInfo.DateTime.month=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;
    dcfLogFileInfo.DateTime.day=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x001F);

    dcfLogFileInfo.DateTime.hour=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS>>11);
    dcfLogFileInfo.DateTime.min=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5;
    dcfLogFileInfo.DateTime.sec=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1;

    dcfLogFileInfo.FileSize=pFile->size;

    DEBUG_DCF2("\ndcfGetLogFileInfo:%s %04d/%02d/%02d %02d:%02d:%02d FileSize=%d\n",
               dcfLogFileInfo.FileName,
               dcfLogFileInfo.DateTime.year,
               dcfLogFileInfo.DateTime.month,
               dcfLogFileInfo.DateTime.day,
               dcfLogFileInfo.DateTime.hour,
               dcfLogFileInfo.DateTime.min,
               dcfLogFileInfo.DateTime.sec,
               dcfLogFileInfo.FileSize
              );

    if( (dcfListLogEntRead==dcfListLogEntTail) )
    {
        if( pFile->size % DCF_LOGBUF_SIZE)
            Tblock=pFile->size/DCF_LOGBUF_SIZE+1+1;
        else
            Tblock=pFile->size/DCF_LOGBUF_SIZE+1;

        if(dcfLogRefreshFlag)
            Tblock -=1;
    }
    else
    {
        if( pFile->size % DCF_LOGBUF_SIZE)
            Tblock=pFile->size/DCF_LOGBUF_SIZE+1;
        else
            Tblock=pFile->size/DCF_LOGBUF_SIZE;
    }


    return Tblock;
}

int dcfReadLogFile(int Nday,int Nblock,int *pRblock,int *pTblock)
{
    FS_FILE* pFile;
    u32 ReadSize,size;
    INT8U   err;
    int filepos;
    u8  tmp;

    //----------------------//
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    filepos=0;
    ReadSize=0;
    pFile=dcfOpenLogFile(Nday);
    strcpy((char *) dcfLogFileInfo.FileName,dcfListLogEntRead->pDirEnt->d_name);

    dcfLogFileInfo.DateTime.year=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD >> 9)+1980;
    dcfLogFileInfo.DateTime.month=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;
    dcfLogFileInfo.DateTime.day=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x001F);

    dcfLogFileInfo.DateTime.hour=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS>>11);
    dcfLogFileInfo.DateTime.min=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5;
    dcfLogFileInfo.DateTime.sec=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1;

    dcfLogFileInfo.FileSize=pFile->size;

    if( (dcfListLogEntRead==dcfListLogEntTail) )
    {
        if( (pFile->size % DCF_LOGBUF_SIZE) != 0)
            DEBUG_DCF("Warning! The size of tail Log is not alignment to 32KB\n");

        if( (Nblock==0) || (pFile->size == 0) )
        {
            memcpy(dcfLogBuf_Rd,dcfLogBuf_Wr,dcfLogWriteBufPos);
            ReadSize=dcfLogWriteBufPos;
            *pRblock=0;
        }
        else
        {
            if( pFile->size % DCF_LOGBUF_SIZE )
            {
                filepos= pFile->size -(pFile->size % DCF_LOGBUF_SIZE)-((Nblock-1)*DCF_LOGBUF_SIZE);

                if(dcfLogRefreshFlag)
                    filepos-=DCF_LOGBUF_SIZE;

                if(filepos<0)
                {
                    filepos=0;
                    *pRblock= (pFile->size -(pFile->size % DCF_LOGBUF_SIZE))/DCF_LOGBUF_SIZE +1;
                }
                else
                {
                    *pRblock=Nblock;
                }

                if(filepos+DCF_LOGBUF_SIZE > pFile->size)
                    ReadSize=pFile->size - filepos;
                else
                    ReadSize=DCF_LOGBUF_SIZE;
            }
            else
            {
                filepos= pFile->size- (Nblock*DCF_LOGBUF_SIZE);

                if(dcfLogRefreshFlag)
                    filepos-=DCF_LOGBUF_SIZE;

                if(filepos<0)
                {
                    filepos=0;
                    *pRblock=pFile->size/DCF_LOGBUF_SIZE;
                }
                else
                {
                    *pRblock=Nblock;
                }


                if(filepos+DCF_LOGBUF_SIZE > pFile->size)
                    ReadSize=pFile->size - filepos;
                else
                    ReadSize=DCF_LOGBUF_SIZE;
            }

            dcfSeek(pFile, filepos, FS_SEEK_SET);
            dcfRead(pFile, dcfLogBuf_Rd,ReadSize, &size);
        }

        if( pFile->size % DCF_LOGBUF_SIZE)
            *pTblock=pFile->size/DCF_LOGBUF_SIZE+1+1;
        else
            *pTblock=pFile->size/DCF_LOGBUF_SIZE+1;

    }
    else
    {
        if(pFile->size % DCF_LOGBUF_SIZE )
        {
            filepos= pFile->size - (pFile->size % DCF_LOGBUF_SIZE)- (Nblock*DCF_LOGBUF_SIZE);
        }
        else
        {
            filepos= pFile->size - ( (Nblock+1)*DCF_LOGBUF_SIZE);
        }

        if(filepos<0)
        {
            filepos=0;
            if(pFile->size % DCF_LOGBUF_SIZE)
                *pRblock= (pFile->size - (pFile->size % DCF_LOGBUF_SIZE))/DCF_LOGBUF_SIZE;
            else
                *pRblock= pFile->size/DCF_LOGBUF_SIZE -1;
        }
        else
        {
            *pRblock=Nblock;
        }

        if(filepos+DCF_LOGBUF_SIZE > pFile->size)
            ReadSize=pFile->size - filepos;
        else
            ReadSize=DCF_LOGBUF_SIZE;

        dcfSeek(pFile, filepos, FS_SEEK_SET);
        dcfRead(pFile, dcfLogBuf_Rd,ReadSize, &size);

        if( pFile->size % DCF_LOGBUF_SIZE)
            *pTblock=pFile->size/DCF_LOGBUF_SIZE+1;
        else
            *pTblock=pFile->size/DCF_LOGBUF_SIZE;
    }

    //DEBUG_DCF2("\dcfReadLogFile: pos=%d,rdsz=%d,fsz=%d,Block=%d,isTail=%d\n",filepos,ReadSize,pFile->size,Nblock,(dcfListLogEntRead==dcfListLogEntTail));

    dcfClose(pFile, &tmp);
    OSSemPost(dcfReadySemEvt);

    return ReadSize;
}

FS_FILE* dcfOpenLogFile(int Nday)
{
    int ShiftNum;
    int i;
    FS_FILE* pFile;
    char newLogName[32], newTargetName[64];


    //DEBUG_DCF2("\nEnter dcfOpenLogFile_Nday\n");
    if(dcfListLogEntTail == NULL)
        return NULL;

    if(Nday > dctTotalLogCount-1)
        ShiftNum=dctTotalLogCount-1;
    else
        ShiftNum=Nday;

    dcfListLogEntRead=dcfListLogEntTail;

    for(i=0; i<ShiftNum; i++)
    {
        dcfListLogEntRead=dcfListLogEntRead->prev;
    }
    //-------//
    strcpy(newLogName,dcfListLogEntRead->pDirEnt->d_name);
    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (char*)gsLogDirName);
    strcat((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newLogName);

    if ((pFile = dcfOpen((s8 *)newTargetName, "r")) == NULL)
    {
        /* create next file error */
        DEBUG_DCF("Error: create next Log %s error\n", newTargetName);
        return NULL;
    }
    //DEBUG_DCF2("Open Log name: %s \n",newTargetName);
    //DEBUG_DCF2("Leave dcfOpenLogFile\n\n");

    return pFile;
}

int dcfLogFileUsrDel(int Nday)
{
    int ShiftNum;
    int i;
    //char newLogName[32], newTargetName[64];
    DCF_LIST_LOGENT* EntDel;


    DEBUG_DCF2("\nEnter dcfLogFileUsrDel N day\n");

    if(dcfListLogEntTail == NULL)
        return NULL;

    if( (Nday==0) || (dctTotalLogCount<2) )
    {
        DEBUG_DCF("Cannot Delete current log!\n");
        return NULL;
    }

    if(Nday > dctTotalLogCount-1)
        ShiftNum=dctTotalLogCount-1;
    else
        ShiftNum=Nday;

    EntDel=dcfListLogEntTail;

    for(i=0; i<ShiftNum; i++)
    {
        EntDel=EntDel->prev;
    }

    if(0 == dcfLogDel(EntDel->pDirEnt->d_name))
    {
        DEBUG_DCF("Trace: dcfLogFileOWDel fail.\n");
        return 0;
    }

    EntDel->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    EntDel->used_flag = DCF_FILE_USE_NONE;

    if(dcfListLogEntHead != dcfListLogEntTail)
    {
        //above 2 files
        EntDel->prev->next = EntDel->next;
        EntDel->next->prev = EntDel->prev;
        if (EntDel == dcfListLogEntHead)
            dcfListLogEntHead = EntDel->next;
        dcfListLogEntTail = dcfListLogEntHead->prev;
        EntDel = EntDel->next;
    }
    else
    {
        //just a file
        EntDel = dcfListLogEntHead = dcfListLogEntTail = NULL;
    }
    dctTotalLogCount --;

    DEBUG_DCF2("Leave dcfLogFileUsrDel\n\n");

    return 1;
}

#endif

#if RX_SNAPSHOT_SUPPORT
s32 dcfPhotoDirInit(void)
{
    char DirName[32];

    //--------------------//
    DEBUG_DCF2("dcfPhotoDirInit: Begin \n");

    sprintf((char*)DirName, "\\%s", gsPhotoDirName);
    if(dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if(Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkDir(DirName) < 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_DIRENT_MAX);	// Not nesscessarily

        if(dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH1");
    if(dcfChDir(DirName) == 0)
    {
        if(Write_protet()==1)
        {
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkDir(DirName) < 0)
        {
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_FILEENT_MAX);	// Not nesscessarily

        if(dcfChDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH2");
    if(dcfChDir(DirName) == 0)
    {
        if(Write_protet()==1)
        {
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkDir(DirName) < 0)
        {
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_FILEENT_MAX); // Not nesscessarily

        if(dcfChDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH3");
    if(dcfChDir(DirName) == 0)
    {
        if(Write_protet()==1)
        {
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkDir(DirName) < 0)
        {
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_FILEENT_MAX);	// Not nesscessarily

        if(dcfChDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH4");
    if(dcfChDir(DirName) == 0)
    {
        if(Write_protet()==1)
        {
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkDir(DirName) < 0)
        {
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        //dcfIncDirEnt(DirName,DCF_FILEENT_MAX);	// Not nesscessarily

        if(dcfChDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    return 1;
}

s32 dcfPhotoFileInit(int CHsel)
{
    u32 OldestEntry, FileCount;
    char DirName[32];

    if(dcfPhotoInitReady == 0)
    {
        DEBUG_DCF("[ERR] DCF photo file system not ready.\n");
        return 0;
    }

    //----------------------------//
    DEBUG_DCF("----dcfPhotoFileInit Begin:%d----\n", CHsel);

    //----------------Scan最後一個目錄----------------------
    memset_hw((void*)dcfFileEntBG, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
    sprintf((char*)DirName, "\\%s\\CH%d", gsPhotoDirName, CHsel+1);
    if(dcfChPlayDir(DirName) == 0)
    {
        // change directory \DCIM error
        DEBUG_DCF("[ERR] DCF change dir to %s failed.\n", DirName);
        return 0;
    }

    // list current directory
    if(dcfPlayDirScan(NULL, dcfFileEntBG, &FileCount, &OldestEntry, 0) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
    {
        // list current directory error
        DEBUG_DCF("[ERR] DCF list current directory failed.\n");
        return 0;
    }

    DEBUG_DCF("----dcfPhotoFileInit End:%d----\n", FileCount);
    return 1;
}

int dcfWriteNextPhotoFile(int RFUnit, u8 *buffer, int size)
{
    FS_FILE* pFile;
    FS_DISKFREE_T *diskInfo;
    RTC_DATE_TIME localTime;
    u32 writesize, free_size, bytes_per_cluster;
    char PhotoName[32],TargetName[64],ChannelName[32];
    u8 err, tmp;

    //-------------------------//
    if(dcfPhotoInitReady==0)
    {
        DEBUG_DCF("[ERR] DCF photo file system isn't Ready.\n");
        return 0;
    }

    diskInfo = &global_diskInfo;
    bytes_per_cluster = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size = diskInfo->avail_clusters * (bytes_per_cluster/512) / 2; // KByte unit

    if(free_size <= 300) // Notice: K-Byte unit
    {
        DEBUG_DCF("Disk Full\n");
        MemoryFullFlag = TRUE;
        return 0;
    }

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    DEBUG_DCF("[INF] dcfWriteNextPhotoFile()\n");
    RTC_Get_Time(&localTime);
    sprintf(PhotoName,"%02d%02d%02dD%d",(u32)localTime.hour, localTime.min, localTime.sec, (RFUnit+1));
    strcat((char*)PhotoName, ".JPG");
    sprintf(ChannelName,"CH%d",(RFUnit+1));
    strcpy((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)gsPhotoDirName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)ChannelName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, PhotoName);

    if((pFile = dcfOpen(TargetName, "w")) == NULL)
    {
        DEBUG_DCF("[ERR] DCF create photo %s error\n", TargetName);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }
    dcfWrite(pFile, buffer, size, &writesize);
    dcfClose(pFile, &tmp);

    OSSemPost(dcfReadySemEvt);
    return 1;
}
#endif

int dcfCheckUnit(void)
{
    int ret;

    DEBUG_DCF2("--Do DcfCheckUnit--\n");
    if((ret = FS_IoCtl((char *) dcfCurDrive, FS_CMD_CHECK_UNIT,0,0)) < 0)
    {
        DEBUG_DCF("Error: Check %s failed\n", dcfCurDrive);
    }
    DEBUG_DCF2("-->Check %s OK\n", dcfCurDrive);
    return ret;
}

int dcfCheckBackupUnit(void)
{
    int ret;

    DEBUG_DCF2("--Do dcfCheckBackupUnit:%s--\n",dcfBackupDrive);
    if((ret = FS_IoCtl((char *) dcfBackupDrive, FS_CMD_CHECK_UNIT, 0, 0)) < 0)
    {
        DEBUG_DCF("Error: Check %s failed\n", dcfBackupDrive);
    }
    DEBUG_DCF2("-->Check %s OK\n", dcfBackupDrive);
    return ret;
}

int dcfBackupDirInit(void)
{
    char DirName[32];

    DEBUG_DCF2("dcfBackupDirInit: Begin \n");
    sprintf((char*)DirName, "\\%s", gsDirName);

    if(dcfChBackupDir(DirName) == 0)
    {
        // change directory \DCIM error
        if(Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");
            return -1;
        }

        if(dcfMkBackupDir(DirName) == 0)
        {
            // make directory \DCIM error
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return -1;
        }

        if(dcfChBackupDir(DirName) == 0)
        {
            // change directory \DCIM error
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return -1;
        }
    }
    return 1;
}

/*
Routine Description:
	Format drive.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
int dcfPlaybackFormat(void)
{
    int ret;

    sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_NREADY);
	
    dcfCacheClear();
    if(dcfFormat(dcfCurDrive) == 0)
    {
        OSMboxPost(general_MboxEvt, "FAIL");
        return 0;
    }

    dcfNewFile = 0;

    // It must be cleared before dir and file init, or it will disturb the FAT1 table
#if(CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
    dcfLogInitReady = 0;
    dcfLogRefreshFlag = 0;
    dcfLogWriteBufPos = 0;
    if((ret = dcfLogDirFileInit()) < 0)
       return ret;
    dcfLogInitReady = 1;
#endif

#if RX_SNAPSHOT_SUPPORT
    dcfPhotoInitReady = 0;
    if ((ret = dcfPhotoDirInit()) < 0)
        return ret;
    dcfPhotoInitReady = 1;
#endif

    // directory initialization
    if ((ret = dcfDirInit()) < 0)
        return ret;

    // file initialization
#if(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    if (dcfFileInit(dcfListDirEntHead, CURRENTFILEENTRY) < 0)
        return -1;
#endif

    sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_READY);
    return 1;
}


s32 dcfBackupFormat(void)
{
    if (dcfFormatBackup(dcfBackupDrive) == 0)
    {
        OSMboxPost(general_MboxEvt, "FAIL");
        return 0;
    }
    // directory initialization
    if (dcfBackupDirInit() == 0)
        return 0;

    return 1;
}

/*

Routine Description:

	Get file type from the Sub-file name

Arguments:

	the pointer string of the Sub-file name.

Return Value:

	<DCF_FILE_TYPE_MAX,  success
	=DCF_FILE_TYPE_MAX,  fail

*/
u8 dcfGetFileType(char* pcsubfilename)
{
    u8 i;

    for(i = 0; i < DCF_FILE_TYPE_MAX; i++)
    {
        if (!strncmp(pcsubfilename, gdcfFileType_Info[i].pSubFileStr, gdcfFileType_Info[i].uiStrLength))
        {
            break;
        }
    }

    //DEBUG_DCF("TRACE: file type =%s, %d\n",pcsubfilename, i);
    return i;
}


u8 dcfCheckFileChannel(char CHmap, char *pFileName)
{
    switch(pFileName[7])
    {
        case '1':
            if(CHmap & DCF_DISTRIB_CH1)
                return 1;
            break;

        case '2':
            if(CHmap & DCF_DISTRIB_CH2)
                return 1;
            break;

        case '3':
            if(CHmap & DCF_DISTRIB_CH3)
                return 1;
            break;

        case '4':
            if(CHmap & DCF_DISTRIB_CH4)
                return 1;
            break;
    }
    return 0;
}

/*
Routine Description:
	clean gFileType_count[DCF_FILE_TYPE_MAX] buffer and global_totalfile_count

Arguments:
	None;

Return Value:
	None
*/
void dcfFileTypeCount_Clean(void)
{
    int i;

    for(i=0; i<DCF_FILE_TYPE_MAX; i++)
    {
        gFileType_count[i] = 0;
    }
}


/*
Routine Description:
	increment by 1 the total count of  the specific file type  and global_totalfile_count

Arguments:
	the specific file type;

Return Value:
	None
*/
void dcfFileTypeCount_Inc(u8 ucFileType)
{
    if (ucFileType >= DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("Error: dcfFileTypeCount_Inc Fail, %d\n",ucFileType);
        return;
    }

    gFileType_count[ucFileType]++;
}

/*
Routine Description:
	decrement by 1 the total count of  the specific file type  and global_totalfile_count

Arguments:
	the specific file type;

Return Value:
	None
*/
void dcfFileTypeCount_Dec(u8 ucFileType)
{
    if (ucFileType >= DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("Error: dcfFileTypeCount_Dec Fail, %d\n",ucFileType);
        return;
    }

    gFileType_count[ucFileType]--;
}

FS_FILE *dcfCreateNextBackupFile(u8 fileType, u8 CHnum,u8 RecType,RTC_DATE_TIME  *pFileTime)
{
    char newFileName[32], newDirName[32], DirName[32];
    FS_FILE *pFile;
    u8 err;

    if (CHnum >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfCreateNextBackupFile incorrect Channel %d\n", CHnum);
        return 0;
    }

    //----------------------------------//
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF2("Enter dcfCreateNextBackupFile\n");

    if(RecType == DCF_REC_TYPE_MANUAL)
        sprintf(newFileName,"%02d%02d%02dM%d",(u32)pFileTime->hour, pFileTime->min, pFileTime->sec, (CHnum+1));
    else if(RecType == DCF_REC_TYPE_MOTION)
        sprintf(newFileName,"%02d%02d%02dD%d",(u32)pFileTime->hour, pFileTime->min, pFileTime->sec, (CHnum+1));
    else if(RecType == DCF_REC_TYPE_SCHEDULE)
        sprintf(newFileName,"%02d%02d%02dS%d",(u32)pFileTime->hour, pFileTime->min, pFileTime->sec, (CHnum+1));
    else
        sprintf(newFileName,"%02d%02d%02d-%d",(u32)pFileTime->hour, pFileTime->min, pFileTime->sec, (CHnum+1));

    sprintf(newDirName,"%04d%02d%02d",(u32)pFileTime->year+2000, pFileTime->month, pFileTime->day);

    //--------Create Backup Dir---------//
    sprintf ((char*)DirName, "\\%s\\%s", gsDirName,newDirName);
    DEBUG_DCF2("Backup Dir name: %s \n",DirName);

    if (dcfChBackupDir(DirName) == 0)
    {
        if (Write_protet()==1)
        {
            DEBUG_DCF("Error: Write protect\n");
            return 0;
        }

        if (dcfMkBackupDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);
            return 0;
        }

        if (dcfChBackupDir(DirName) == 0)
        {
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
            return 0;
        }
    }

    //------------Open Backup File-----------------//
    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("Error: fileType=%d does not exist\n", fileType);
        OSSemPost(dcfReadySemEvt);
        return NULL;
    }
    strcat((char*)newFileName,".");
    strcat((char*)newFileName,gdcfFileType_Info[fileType].pSubFileStr);


    DEBUG_DCF2("Backup File name: %s \n",newFileName);
    if((pFile = dcfBackupOpen(newFileName, "w-")) == NULL)
    {
        DEBUG_DCF("Error: create next backup file %s error\n", newFileName);
        OSSemPost(dcfReadySemEvt);
        return NULL;
    }

    DEBUG_DCF2("Leave dcfCreateNextBackupFile\n\n");
    OSSemPost(dcfReadySemEvt);

    return pFile;
}

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
//*******************************************************************************
//	DCF for FILE_SYSTEM_CDVR
//*******************************************************************************

void dcfGetCurFileName(s8 *FileName)
{
    memcpy(FileName, dcfChFileEnt[dcfFileNewestIndex].d_name, 12);
}

void dcfSetCurFileName(s8 *FileName, u8 index)
{
    memcpy(dcfChFileEnt[index].d_name, FileName, 12);
}

/*
Routine Description:
	Check if valid dcf directory.

Arguments:
	pDirEnt - Directory entry.
	pDirNum - Directory number.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfCheckDir(FS_DIRENT *pDirEnt, u32 *pDirNum)
{
    u32 j;
    s8 dirNumStr[16];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".   ") != 0)
        return 0;

    /* get dcf directory number string */
    for (j = 0; j < 8; j++)
    {
        dirNumStr[j] = pDirEnt->d_name[j];
        if( (dirNumStr[j] < '0') || (dirNumStr[j] > '9') )
            return 0;
    }
    dirNumStr[8] = '\0';

    *pDirNum = (u32)atoi((const char*)dirNumStr);

    if(*pDirNum < 1)
        return 0;

    return 1;
}

/*
Routine Description:
	Check if valid dcf file.

Arguments:
	pFileEnt - File entry.
	pFileNum - File number.

Return Value:
	0 - Failure.
	1 - Success.
*/

s32 dcfCheckFile(FS_DIRENT* pFileEnt, u32* pFileNum, u8* pFileType, u8 FileGroup)
{
    u32 i, j,value;
    u8 fileNumStr[5];
    u32 limit[4]= {24, 60, 60, 03};
    s32 CamId = 1;

    //check sub file name
    for (j = 0; j < 3; j++)
        fileNumStr[j] = pFileEnt->d_name[j+9];
    fileNumStr[j] = '\0';

    if ((*pFileType=dcfGetFileType((char *)fileNumStr))==DCF_FILE_TYPE_MAX)
        return 0;

    /* get dcf file number string(hour/min/sec/ch) */
    *pFileNum = 0;
    for( i=0 ; i<3 ; i++)
    {
        for (j = 0; j < 2; j++)
        {
            fileNumStr[j] = pFileEnt->d_name[i*2+j];
            if ((fileNumStr[j]<'0')||(fileNumStr[j]>'9'))
                return 0;
        }
        fileNumStr[3] = '\0';

        /* dcf directory number(hour) is from 00 to 23 */
        value = (u32)atoi((const char*)fileNumStr);
        if (value > limit[i])
            return 0;

        *pFileNum = *pFileNum *100 + value;
    }

#if(UI_VERSION == UI_VERSION_TRANWO)
    if ((pFileEnt->d_name[6] != 'M') && (pFileEnt->d_name[6] != 'D') && (pFileEnt->d_name[6] != 'S'))
        return 0;
#elif (UI_VERSION == UI_VERSION_RDI_3)
    if ((pFileEnt->d_name[6] != 'A') && (pFileEnt->d_name[6] != 'P'))
        return 0;
#else
    if (pFileEnt->d_name[6] != '-')
        return 0;
#endif

    fileNumStr[0] = pFileEnt->d_name[7];
    if ((fileNumStr[0]<'1')||(fileNumStr[0]>'4'))
        return 0;
    fileNumStr[1] = '\0';
    value = (u32)atoi((const char*)fileNumStr);
    CamId = value;
    *pFileNum = *pFileNum *100 + value;

    //DEBUG_DCF("dcfCheckFile: name= %s, number= %u\n",pFileEnt->d_name, *pFileNum);

    return CamId;
}

int dcfGetCurDirFileCount(void)
{
    if(dcfPlaybackCurDir == NULL)
    {
        DEBUG_DCF("[WARM] DCF current dir is NULL.\n");
        return 0;
    }

    return dcfVideoFileCnt;
}

int dcfGetTotalDirCount(void)
{
    if(dcfListDirEntHead == NULL)
    {
        DEBUG_DCF("[WARM] DCF Head of dir list is NULL.\n");
    }

    return dcfVideoDirCnt;
}

int dcfFindFreeDirEnt(FS_DIRENT **pDirEnt)
{
    u32 i;

    for(i = 0; i < DCF_DIRENT_MAX; i++)
    {
        if(dcfDirEnt[i].used_flag == DCF_FILE_USE_NONE)
        {
            *pDirEnt = &dcfDirEnt[i];
            //(*pDirEnt)->used_flag = DCF_FILE_USE_CREATE;
            return 1;
        }
    }

    DEBUG_DCF("[WARM] DCF dir ent list is full.\n");
    return -1;
}

/*
Routine Description:
	Insert directory entry.

Arguments:
	ins - Directory entry to be inserted. 會按照流水號排序

Return Value:
	0 - Failure, 1 - Success.
*/
int dcfInsertFreeDirEnt(DCF_LIST_DIRENT *ins)
{
    DCF_LIST_DIRENT *cur;

    if(dcfListDirEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;
        dcfListDirEntHead = ins;
    }
    else
    {
        cur = dcfListDirEntHead;
        do
        {
            if(ins->dirNum < cur->dirNum) 	// Compare the seq number
            {
                if (cur == dcfListDirEntHead)
                    dcfListDirEntHead = ins;	// insert before head
                break;
            }
            cur = cur->next;
        }
        while(cur != dcfListDirEntHead);

        // insert before cur
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    dcfListDirEntTail = dcfListDirEntHead->prev;
    return 1;
}

int dcfSetFreeDirEnt(DCF_LIST_DIRENT **pListDirEnt, FS_DIRENT *pDirEnt, u32 DirNumName)
{
    u32 i;

    for(i = 0; i < DCF_DIRENT_MAX; i++)
    {
        if(dcfListDirEnt[i].used_flag == DCF_FILE_USE_NONE)
        {
        	memset(dcfListDirEnt + i, 0x0, sizeof(DCF_LIST_DIRENT));
        	(*pListDirEnt) = &dcfListDirEnt[i];
        	(*pListDirEnt)->dirNum = DirNumName;
        	(*pListDirEnt)->pDirEnt = pDirEnt;
        	(*pListDirEnt)->used_flag = DCF_FILE_USE_CLOSE;
        	pDirEnt->used_flag = DCF_FILE_USE_CLOSE;	// the same as dcfDirEnt.
        	dcfInsertFreeDirEnt(*pListDirEnt);
        	return 1;
        }
    }

    DEBUG_DCF("[WARM] DCF set dir ent failed.\n");
    return -1;
}

int dcfFreeDirEnt(DCF_LIST_DIRENT **pListDirEnt)
{
    if(*pListDirEnt == NULL)
    {
        DEBUG_DCF("[WARM] DCF free dir ent failed.\n");
        return 1;
    }

    // Release the Entry of global array.
    (*pListDirEnt)->used_flag = DCF_FILE_USE_NONE;
    (*pListDirEnt)->pDirEnt->used_flag = DCF_FILE_USE_NONE;

    if((*pListDirEnt) == dcfListDirEntHead)
        dcfListDirEntHead = (*pListDirEnt)->next;

    (*pListDirEnt)->prev->next = (*pListDirEnt)->next;
    (*pListDirEnt)->next->prev = (*pListDirEnt)->prev;

    dcfListDirEntTail = dcfListDirEntHead->prev;

    if(dcfVideoDirCnt == 0x1)	// Last one
    {
        dcfListDirEntHead = dcfListDirEntTail = NULL;
        (*pListDirEnt) = NULL;
    }

    dcfVideoDirCnt--;

    return 1;
}

int dcfSetDirEntList(u32 NumOfDirs)
{
    FS_DIRENT *pDirEnt;
    DCF_LIST_DIRENT *pDirEntList;
    u32 i, dirNum;
    //
    dcfListDirEntHead = NULL;
    dcfListDirEntTail = NULL;
    dcfPlaybackCurDir = NULL;

    dcfVideoDirNumLast = 0;
    dcfVideoDirCnt = 0;

    if(NumOfDirs >= DCF_DIRENT_MAX)
        DEBUG_DCF("[WARM] DCF dir list over limit.\n");

    pDirEnt = dcfDirEnt;

    for(i = 0; (i < NumOfDirs) && (i < DCF_DIRENT_MAX); i++)
    {
        if(pDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            // check if valid dcf directory
            if (dcfCheckDir(&pDirEnt[i], &dirNum) == 0)
                continue;

            // Keep the bigger one value
            dcfVideoDirNumLast = (dcfVideoDirNumLast < dirNum) ? dirNum : dcfVideoDirNumLast;
            if(dcfSetFreeDirEnt(&pDirEntList, &pDirEnt[i], dirNum) < 0)
            {
                DEBUG_DCF("[ERR] DCF increase dir list failed.\n");
                return -1;
            }
            dcfVideoDirCnt++;
        }
    }

    if(dcfListDirEntHead != NULL)
    {
        // set tail entry of directory list
        dcfListDirEntTail = dcfListDirEntHead->prev;
        // set current playback directory
        dcfVideoRecCurDir = dcfListDirEntTail;
    }

    return 1;
}

/*
Routine Description:
	Insert file entry.

Arguments:
	ins - file entry to be inserted.

Return Value:
	0 - Failure, 1 - Success.
*/
int dcfInsertFreeFileEnt(DCF_LIST_FILEENT *ins)
{
    DCF_LIST_FILEENT *cur;

    if(dcfListFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;
        ins->CamPrev = ins;
        ins->CamNext = ins;
        dcfListFileEntHead = ins;
    }
    else
    {
        cur = dcfListFileEntHead;
        // insert before Head (tail-ins-head)
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
        (cur->CamPrev)->CamNext = ins;
        ins->CamPrev = cur->CamPrev;
        ins->CamNext = cur;
        cur->CamPrev = ins;
    }

    dcfListFileEntTail = dcfListFileEntHead->prev;
    return 1;
}

int dcfSetFreeFileEnt(DCF_LIST_FILEENT *pListFileEnt, FS_DIRENT *pFileEnt)
{
    u32 i;

    pListFileEnt = NULL;

    for(i = 0; i < DCF_FILEENT_MAX; i++)
    {
        if(dcfListFileEnt[i].used_flag == DCF_FILE_USE_NONE)
        {
            pListFileEnt = &dcfListFileEnt[i];
            pListFileEnt->pDirEnt = pFileEnt;
            pListFileEnt->used_flag = DCF_FILE_USE_CLOSE;
            pFileEnt->used_flag = DCF_FILE_USE_CLOSE;	// the same as dcfDirEnt.

            dcfInsertFreeFileEnt(pListFileEnt);
            return 1;
        }
    }
    DEBUG_DCF("[WARM] DCF set dir ent failed.\n");
    return -1;
}

int dcfSetFileEntList(u32 NumOfFiles)
{
    FS_DIRENT *pFileEnt;
    DCF_LIST_FILEENT *pFileEntList;
    u32 i;
    //
    dcfListFileEntHead = NULL;
    dcfListFileEntTail = NULL;
    dcfPlaybackCurFile = NULL;

    dcfVideoFileCnt = 0;

    if(NumOfFiles >= DCF_FILEENT_MAX)
        DEBUG_DCF("[WARM] DCF file list over limit.\n");

    pFileEnt = dcfFileEnt;

    for(i = 0; (i < NumOfFiles) && (i < DCF_FILEENT_MAX); i++)
    {
        if(pFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_ARCHIVE)
        {
            if(dcfSetFreeFileEnt(pFileEntList, &pFileEnt[i]) < 0)
            {
                DEBUG_DCF("[ERR] DCF increase dir list failed.\n");
                return -1;
            }
            dcfVideoFileCnt++;
        }
    }

    if(dcfListFileEntHead != NULL)
    {
        // Change the head to the Oldest one
        dcfListFileEntHead = &dcfListFileEnt[dcfFileOldestIndex];
        // set tail entry of file list
        dcfListFileEntTail = dcfListFileEntHead->prev;
        // set current playback file
        dcfPlaybackCurFile = dcfListFileEntTail;
    }

    return 1;
}

int dcfSetFileDistInDir(DCF_LIST_DIRENT *pListDirEnt, DCF_LIST_FILEENT *pListFileEntHead)
{
    DCF_LIST_FILEENT *Cur;
    u32 indexVal, hourVal;
    int tmpVal = pListDirEnt->FileTotal;
    //
    // 3 = (PosSchedule + PosDetect + PosManual)
    pListDirEnt->CH_Distr = 0x0;
    pListDirEnt->FileTotal = 0x0;
    memset(pListDirEnt->ChTotal, 0x0, sizeof(u32) * DCF_MAX_MULTI_FILE);
    memset(&pListDirEnt->PosSchedule, 0x0, sizeof(u16) * 3);
    memset(pListDirEnt->SumSchedule, 0x0, sizeof(u16) * DCF_MAX_MULTI_FILE * 3);
    memset(pListDirEnt->DistributionSchedule, 0x0, sizeof(u32) * DCF_MAX_MULTI_FILE * 3);
    //
    Cur = pListFileEntHead;
    do
    {
        if(Cur == NULL)
            break;

        // fetch the hour and channel value
        switch(Cur->pDirEnt->d_name[6])
        {
            case 'D':
            case 'S':
            case '-':
            case 'M':
            case 'R':
            case 'F':
                sscanf(Cur->pDirEnt->d_name, "%2d%*2d%*2d%*1s%1d*", &hourVal, &indexVal);
                break;
            default:	// PM, AM.
                sscanf(Cur->pDirEnt->d_name, "%*1s%2d%*2d%*2d%1d*", &hourVal, &indexVal);
                break;
        }
        indexVal -= 1;	// Channel name always begin from 1 to 4.
        switch(Cur->pDirEnt->d_name[6])
        {
            case 'D':
                if(!pListDirEnt->PosDetect)
                    pListDirEnt->PosDetect = 0x80 | (Cur->pDirEnt->FileEntrySect & 0x3F);
                pListDirEnt->DistributionDetect[indexVal] |= (1 << hourVal);
                pListDirEnt->SumDetect[indexVal]++;
                pListDirEnt->CH_Distr |= DCF_DISTRIB_MOTN;
                break;
            case 'S':
                if(!pListDirEnt->PosSchedule)
                    pListDirEnt->PosSchedule = 0x80 | (Cur->pDirEnt->FileEntrySect & 0x3F);
                pListDirEnt->DistributionSchedule[indexVal] |= (1 << hourVal);
                pListDirEnt->SumSchedule[indexVal]++;
                pListDirEnt->CH_Distr |= DCF_DISTRIB_SCHE;
                break;
            case '-':
            case 'M':
            case 'F':
                if(!pListDirEnt->PosManual)
                    pListDirEnt->PosManual = 0x80 | (Cur->pDirEnt->FileEntrySect & 0x3F);
                pListDirEnt->DistributionManual[indexVal] |= (1 << hourVal);
                pListDirEnt->SumManual[indexVal]++;
                pListDirEnt->CH_Distr |= DCF_DISTRIB_MANU;
                break;
            case 'R':
                if(!pListDirEnt->PosRing)
                    pListDirEnt->PosRing = 0x80 | (Cur->pDirEnt->FileEntrySect & 0x3F);
                pListDirEnt->DistributionRing[indexVal] |= (1 << hourVal);
                pListDirEnt->SumRing[indexVal]++;
                pListDirEnt->CH_Distr |= DCF_DISTRIB_RING;
                break;
            default:
                if((Cur->pDirEnt->d_name[0] == 'A') || Cur->pDirEnt->d_name[0] == 'P')
                {
                    if(!pListDirEnt->PosManual)
                        pListDirEnt->PosManual = 0x80 | (Cur->pDirEnt->FileEntrySect & 0x3F);
                    pListDirEnt->DistributionManual[indexVal] |= (1 << hourVal);
                    pListDirEnt->SumManual[indexVal]++;
                }
                break;
        }
        pListDirEnt->ChTotal[indexVal]++;
        pListDirEnt->FileTotal++;
        pListDirEnt->CH_Distr |= (1 << indexVal);
        // Move to next
        Cur = Cur->next;
    }
    while(Cur != pListFileEntHead);

    if(tmpVal > pListDirEnt->FileTotal)
        pListDirEnt->FileTotal = tmpVal;	// Keep the number of Recording files.

    return 1;
}

int dcfCamListFilter(DCF_LIST_FILEENT *ins, u32 CamList)
{
    DCF_LIST_FILEENT *Cur;
    int ret;

    dcfPlaybackListFileEntHead = NULL;
    dcfPlaybackListFileEntTail = NULL;
    dcfPlaybackCurFile = NULL;

    Cur = ins;
    do
    {
        if(Cur == NULL)
            break;

        switch((1 << (Cur->pDirEnt->d_name[7]-'1')) & CamList)
        {
            case 1:	// Ch 1
            case 2:	// Ch 2
            case 4:	// Ch 3
            case 8:	// Ch 4
                ret = 1;
                break;
            default:
                ret = 0;
                break;
        }

        switch(Cur->pDirEnt->d_name[6])
        {
            case 'D':
                if(!(DCF_DISTRIB_MOTN & CamList))
                    ret = 0;
                break;
            case 'S':
                if(!(DCF_DISTRIB_SCHE & CamList))
                    ret = 0;
                break;
            case 'M':
                if(!(DCF_DISTRIB_MANU & CamList))
                    ret = 0;
                break;
            case 'R':
                if(!(DCF_DISTRIB_RING & CamList))
                    ret = 0;
                break;
            default:
                break;
        }

        if(ret)
        {
            if(dcfPlaybackListFileEntHead == NULL)
                dcfPlaybackListFileEntHead = Cur;
        }
        else
        {
            if(Cur->CamNext != Cur)
            {
                Cur->CamPrev->CamNext = Cur->CamNext;
                Cur->CamNext->CamPrev = Cur->CamPrev;
            }
        }

        // Move to next
        Cur = Cur->CamNext;
    }
    while((Cur != ins) && (Cur != dcfPlaybackListFileEntHead));


    if(dcfPlaybackListFileEntHead != NULL)
    {
        // set tail entry of file list
        dcfPlaybackListFileEntTail = dcfPlaybackListFileEntHead->CamPrev;
        // set current playback file
        dcfPlaybackCurFile = dcfPlaybackListFileEntTail;
    }

    return 1;
}

int dcfInsertFreeFileEntBG(DCF_LIST_FILEENT *ins)
{
    DCF_LIST_FILEENT *cur;

    if(dcfListFileEntBGHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;
        dcfListFileEntBGHead = ins;
    }
    else
    {
        cur = dcfListFileEntBGHead;
        // insert before Head (tail-ins-head)
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    dcfListFileEntBGTail = dcfListFileEntBGHead->prev;
    return 1;
}

int dcfSetFreeFileEntBG(DCF_LIST_FILEENT *pListFileEnt, FS_DIRENT *pFileEnt)
{
    u32 i;

    pListFileEnt = NULL;

    for(i = 0; i < DCF_FILEENT_MAX; i++)
    {
        if(dcfListFileEntBG[i].used_flag == DCF_FILE_USE_NONE)
        {
            pListFileEnt = &dcfListFileEntBG[i];
            pListFileEnt->pDirEnt = pFileEnt;
            pListFileEnt->used_flag = DCF_FILE_USE_CLOSE;
            pFileEnt->used_flag = DCF_FILE_USE_CLOSE;	// the same as dcfDirEnt.

            dcfInsertFreeFileEntBG(pListFileEnt);
            return 1;
        }
    }
    DEBUG_DCF("[WARM] DCF set dir ent failed.\n");
    return -1;
}

int dcfSetFileEntListBG(u32 NumOfFiles)
{
    FS_DIRENT *pFileEnt;
    DCF_LIST_FILEENT *pFileEntList;
    u32 i;
    //
    dcfListFileEntBGHead = NULL;
    dcfListFileEntBGTail = NULL;
    dcfPlaybackCurBGFile = NULL;

    dcfVideoFileBGCnt = 0;

    if(NumOfFiles >= DCF_FILEENT_MAX)
        DEBUG_DCF("[WARM] DCF file list over limit.\n");

    pFileEnt = dcfFileEntBG;

    for(i = 0; (i < NumOfFiles) && (i < DCF_FILEENT_MAX); i++)
    {
        if(pFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_ARCHIVE)
        {
            if(dcfSetFreeFileEntBG(pFileEntList, &pFileEnt[i]) < 0)
            {
                DEBUG_DCF("[ERR] DCF increase dir list failed.\n");
                return -1;
            }
            dcfVideoFileBGCnt++;
        }
    }

    if(dcfListFileEntBGHead != NULL)
    {
        // Change the head to the Oldest one
        //dcfListFileEntBGHead = &dcfListFileEntBG[dcfFileOldestIndex];
        // set tail entry of file list
        dcfListFileEntBGTail = dcfListFileEntBGHead->prev;
        // set current playback file
        dcfPlaybackCurBGFile = dcfListFileEntBGTail;
    }

    return 1;
}

DCF_LIST_DIRENT *dcfGetVideoDirListHead(void)
{
    return dcfListDirEntHead;
}

DCF_LIST_DIRENT *dcfGetVideoDirListTail(void)
{
    return dcfListDirEntTail;
}

DCF_LIST_FILEENT *dcfGetPlaybackFileListHead(void)
{
    return dcfPlaybackListFileEntHead;
}

DCF_LIST_FILEENT *dcfGetPlaybackFileListTail(void)
{
    return dcfPlaybackListFileEntTail;
}

int dcfSetChannelRecType(u8 Idx, u8 RecType)
{
    if(!(Idx < MULTI_CHANNEL_MAX))
        return 0;

    dcfChannelRecType[Idx] = RecType;
    return 1;
}

int dcfGetNewFileTime(u8 index, char *pFileName, RTC_DATE_TIME *pLocalTime)
{
#if DCF_RECORD_TYPE_API
    u8 RecType;
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    u8 tmpSch;
#endif

    RTC_Get_Time(pLocalTime);

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    tmpSch = (pLocalTime->hour << 1) + (pLocalTime->min / 30);
    if(Schedule_Status[pLocalTime->week][index][tmpSch] & SDK_SCHEDULE_REC)
        RecType = DCF_I_FILE_NAME_SCHE;
    else if(Schedule_Status[pLocalTime->week][index][tmpSch] & SDK_SCHEDULE_MOTION)
        RecType = DCF_I_FILE_NAME_DYNAMIC;
    else
        RecType = DCF_I_FILE_NAME_MANUAL;
#else
    RecType = dcfChannelRecType[index];
#endif

#if PIR_FALSETRIG_TEST
    if(gRfiuUnitCntl[index].RFpara.TXPirFaulseTrig)
        RecType = DCF_I_FILE_NAME_FORCE_TRIGGER;
#endif

    switch(RecType)
    {
        case DCF_I_FILE_NAME_MANUAL:
            sprintf(pFileName, "%02d%02d%02dM%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_SCHE:
            sprintf(pFileName, "%02d%02d%02dS%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_DYNAMIC:
            sprintf(pFileName, "%02d%02d%02dD%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_APM:
            if(pLocalTime->hour > 12)
                sprintf(pFileName, "P%02d%02d%02d%d", (u32)(pLocalTime->hour-12), pLocalTime->min, pLocalTime->sec, (index+1));
            else if(pLocalTime->hour == 12)
                sprintf(pFileName, "P%02d%02d%02d%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            else if(pLocalTime->hour == 0)
                sprintf(pFileName, "A%02d%02d%02d%d", (u32)(pLocalTime->hour+12), pLocalTime->min, pLocalTime->sec, (index+1));
            else
                sprintf(pFileName, "A%02d%02d%02d%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_FORCE_TRIGGER:
            sprintf(pFileName,"%02d%02d%02dF%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_RING:
            sprintf(pFileName,"%02d%02d%02dR%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;

        case DCF_I_FILE_NAME_DASH:
        default:
            sprintf(pFileName, "%02d%02d%02d-%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
            break;
    }
#else

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    u8 tmpSch;
#endif

    RTC_Get_Time(pLocalTime);

#if PIR_FALSETRIG_TEST
    if(gRfiuUnitCntl[index].RFpara.TXPirFaulseTrig)
    {
        sprintf(pFileName,"%02d%02d%02dF%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
        return 1;
    }
#endif

#if (UI_VERSION == UI_VERSION_TRANWO)

    if(uiCurRecType[index]== UI_REC_TYPE_MANUAL)
        sprintf(pFileName, "%02d%02d%02dM%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else if(uiCurRecType[index]== UI_REC_TYPE_MOTION)
        sprintf(pFileName, "%02d%02d%02dD%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else if(uiCurRecType[index]== UI_REC_TYPE_SCHEDULE)
        sprintf(pFileName, "%02d%02d%02dS%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else
        sprintf(pFileName, "%02d%02d%02dM%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));

#else

#if ((UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM) || (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM))
    if (pLocalTime->hour > 12)
        sprintf(pFileName, "P%02d%02d%02d%d", (u32)(pLocalTime->hour-12), pLocalTime->min, pLocalTime->sec, (index+1));
    else if (pLocalTime->hour == 12)
        sprintf(pFileName, "P%02d%02d%02d%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else if (pLocalTime->hour == 0)
        sprintf(pFileName, "A%02d%02d%02d%d", (u32)pLocalTime->hour+12, pLocalTime->min, pLocalTime->sec, (index+1));
    else
        sprintf(pFileName, "A%02d%02d%02d%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
#else
    sprintf(pFileName, "%02d%02d%02d-%d", (u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
#endif

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    tmpSch = pLocalTime->hour*2+pLocalTime->min/30;
    if(Schedule_Status[pLocalTime->week][index][tmpSch] & SDK_SCHEDULE_REC == SDK_SCHEDULE_REC)
        sprintf(pFileName,"%02d%02d%02dS%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else if(Schedule_Status[pLocalTime->week][index][tmpSch] == SDK_SCHEDULE_MOTION)
        sprintf(pFileName,"%02d%02d%02dD%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
    else
        sprintf(pFileName,"%02d%02d%02dM%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
#endif

#endif

#endif
    return 1;
}


/*
Routine Description:
	Create next directory.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.

*/
int dcfCreateNextDir(void)
{
    DCF_LIST_DIRENT *pListDirEnt;
    FS_DIRENT *pDirEnt;
#if(UI_VERSION == UI_VERSION_MUXCOM)
    RTC_DATE_TIME localTime;
    u32 LastdirNum;
#endif
    char DirName[32], newDirName[32], newTargetName[64];
    int ret, count;
    //

    DEBUG_DCF2("Enter CDVR dcfCreateNextDir\n");
    // list file newly created
    if(dcfFindFreeDirEnt(&pDirEnt) < 0)
    {
        DEBUG_DCF("[ERR] DCF increase dir failed.\n");
        return -1;
    }

#if (UI_VERSION == UI_VERSION_MUXCOM)
    RTC_Get_Time(&localTime);

#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    sprintf(newDirName, "%02d%02d%04d.   \0", localTime.month, localTime.day, (u32)localTime.year + 2000);
#else
    sprintf(newDirName, "%04d%02d%02d.   \0", (u32)localTime.year + 2000, localTime.month, localTime.day);
#endif
    LastdirNum = ((u32)localTime.year + 2000) * 10000 + ((u32)localTime.month) * 100 + localTime.day;

#if ROOTWORK
    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newDirName);
#else
    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, gsDirName);
    strcat((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newDirName);
#endif

    DEBUG_DCF("[INF] New dir path: %s\n", newTargetName);
    if((ret = dcfMkDir(newTargetName)) < 0)
    {
        DEBUG_DCF("[ERR] DCF make dir %s failed.\n", newTargetName);
        return ret;
    }

    dcfVideoDirNumLast = LastdirNum;
#else
    count = 0;
    dcfVideoDirNumLast++;
    while (1)
    {
        strcpy((char*)newTargetName, "\\");
        strcat((char*)newTargetName, gsDirName);
        strcat((char*)newTargetName, "\\");

        dcfIntToStr(dcfVideoDirNumLast, (s8 *) newDirName, 8);
        strcat((char*)newDirName, ".   ");
        strcat((char*)newTargetName, (const char *) newDirName);

        DEBUG_DCF("[INF] New dir path: %s\n", newTargetName);
        if((ret = dcfMkDir(newTargetName)) < 0x0)
            DEBUG_DCF("[WARM] DCF Mkdir error: %#x\n", ret);
        if((ret = dcfCheckDirExist(newTargetName)) == 0x1)
            break;
        count++;
        if(count >= 3)
            return -1;
        DEBUG_DCF("[ERR] DCF make dir %s failed, Retry: %d.\n", newTargetName, count);
        if(dcfCheckUnit() < 0)
            return -1;
    }
#endif

    // list directory newly created
    // using dcfVideoDirCnt (only increment), for delete(cannot decrement))
    sprintf(DirName, "\\%s", gsDirName);
    if(dcfGetDirEnt(DirName, pDirEnt, newDirName) == 0)
    {
        // list directory error
        DEBUG_DCF("[ERR] Find dir ent of %s failed.\n", newDirName);
        return -1;
    }

    if(dcfSetFreeDirEnt(&pListDirEnt, pDirEnt, dcfVideoDirNumLast) < 0)
    {
        DEBUG_DCF("[ERR] DCF increase dir list failed.\n");
        return -1;
    }
    dcfVideoDirCnt++;

    // set current recording directory
    dcfVideoRecCurDir = pListDirEnt;
    DEBUG_DCF2("Leave CDVR dcfCreateNextDir\n");

    return 1;
}

/*
Routine Description:
	Create next file.

Arguments:
	fileType - File type.

Return Value:
	File handle.
*/
FS_FILE *dcfCreateNextFile(u8 fileType, u8 index)
{
    FS_FILE* pFile;
    RTC_DATE_TIME localTime;
    u16 val;
    char DirName[32], newFileName[32], ReNewFileName[32];
    u8 count, err;

    //
    if (index >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("[ERR] DCF Create File Error Index %d\n", index);
        return 0;
    }
    //
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF2("\nEnter CDVR dcfCreateNextFile\n");

    dcfGetNewFileTime(index, newFileName, &localTime);

    // Date value
    val = (((localTime.year + 20) & 0x7F) << 9) |((localTime.month & 0xF) << 5) |(localTime.day & 0x1F);
    if((dcfListDirEntTail == NULL) || (dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD != val))
    {
        if (dcfListDirEntTail == NULL)
            DEBUG_DCF("CD type: 11, new YMD: %#x\n", val);
        else if ((dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD != val))
            DEBUG_DCF("CD type: 33, Dir YMD(Last: New): (%#x:%#x)\n", dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD, val);

        if(dcfCreateNextDir() < 0)
        {
            DEBUG_DCF("[ERR] DCF create next dir error!\n");
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        // Create new dir successful
        dcfDirRescanSwitch = 1;

        dcfFileTypeCount_Clean();

        if(dcfVideoRecCurDir)
            DEBUG_DCF("[INF] Directory Name: %s\n", dcfVideoRecCurDir->pDirEnt->d_name);

        // The directory would be deleted when video recording processing.
        // It will disrupt FAT table and recording information.
        switch(dcfOverWriteOP)
        {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            case DCF_OVERWRITE_OP_01_DAYS:
                if(dcfGetTotalDirCount() < 1)	// First dir don't stop the video recording
                {
                    u32 tmp;
                    for(tmp = 0; tmp < DCF_MAX_MULTI_FILE; tmp++)
                    {
                        if(index != tmp)
                            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, tmp);
                    }
                }
                break;
#endif
            default:
                break;
        }
    }

    // Switch current path to recording dir
    sprintf ((char*)DirName, "\\%s", gsDirName);
    if(dcfChDir(DirName) == 0)
    {
        DEBUG_DCF("[ERR] DCF change dir to %s failed.\n", DirName);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    // change current directory
    if(dcfChDir(dcfVideoRecCurDir->pDirEnt->d_name) == 0)
    {
        DEBUG_DCF("[ERR] DCF change current dir failed.\n");
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    //------可在此做分類檔案-------//
    //strcpy((char*)newFileName, DCF_FILE_PREFIX);
    //dcfIntToStr(dcfFileNumLast, (s8 *)&newFileName[0], 8);
    //-----------------------------//
    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("[ERR] DCF fileType(%d) does not exist.\n", fileType);
        OSSemPost(dcfReadySemEvt);
        return NULL;
    }

    count = 0;
    do
    {
        strcat((char*)newFileName, ".");
        strcat((char*)newFileName, gdcfFileType_Info[fileType].pSubFileStr);

        DEBUG_DCF("[INF] New file: %s\\%s\n", dcfVideoRecCurDir->pDirEnt->d_name, newFileName);
        if((pFile = dcfOpen(newFileName, "w-")) != NULL)
            break;
        DEBUG_DCF("[WARM] DCF create file retry: %d\n", ++count);
        if(!(count < 3))
        {
            DEBUG_DCF("[ERR] DCF create next file %s error.\n", newFileName);
            OSSemPost(dcfReadySemEvt);
            return NULL;
        }

        do
        {
            OSTimeDly(2);
            // Fetch new file name
            dcfGetNewFileTime(index, ReNewFileName, &localTime);
            if(strncmp(newFileName, ReNewFileName, FS_V_FAT_ENTEY_SHORT_NAME) != 0x0)
            {
                memset(newFileName, 0x0, 32);
                memcpy(newFileName, ReNewFileName, FS_V_FAT_ENTEY_SHORT_NAME);
                break;
            }
        }
        while(1);
    }
    while(pFile == NULL);

    strcpy(dcfChFileEnt[index].d_name, newFileName);
    dcfChFileEnt[index].FAT_DirAttr = 0x20;
    dcfChFileEnt[index].FileEntrySect = pFile->FileEntrySect;
    RTC_Get_Time(&localTime);
    val = ((localTime.hour & 0x1F) << 11) | ((localTime.min & 0x3F) << 5) | ((localTime.sec / 2) & 0x1F);
    dcfChFileEnt[index].fsFileCreateTime_HMS = val;
    dcfChFileEnt[index].fsFileModifiedTime_HMS = val;
    val = (((localTime.year + 20)& 0x7F) << 9) | ((localTime.month & 0xF) << 5) | (localTime.day & 0x1F);
    dcfChFileEnt[index].fsFileCreateDate_YMD = val;
    dcfChFileEnt[index].fsFileModifiedDate_YMD = val;

    dcfChFileEnt[index].used_flag = DCF_FILE_USE_CREATE;
    dcfListChFileEnt[index].used_flag = DCF_FILE_USE_CREATE;
    dcfFileNewestIndex = index;

    // Prepare the timeline params to save time of Playback search
    switch(newFileName[6])
    {
        case 'D':
            if(!dcfVideoRecCurDir->PosDetect)
                dcfVideoRecCurDir->PosDetect = 0x80 | (pFile->FileEntrySect & 0x3F);
            dcfVideoRecCurDir->DistributionDetect[index] |= (1 << localTime.hour);
            dcfVideoRecCurDir->SumDetect[index]++;
            dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_MOTN;
            break;

        case 'S':
            if(!dcfVideoRecCurDir->PosSchedule)
                dcfVideoRecCurDir->PosSchedule = 0x80 | (pFile->FileEntrySect & 0x3F);
            dcfVideoRecCurDir->DistributionSchedule[index] |= (1 << localTime.hour);
            dcfVideoRecCurDir->SumSchedule[index]++;
            dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_SCHE;
            break;

        case '-':
        case 'M':
            if(!dcfVideoRecCurDir->PosManual)
                dcfVideoRecCurDir->PosManual = 0x80 | (pFile->FileEntrySect & 0x3F);
            dcfVideoRecCurDir->DistributionManual[index] |= (1 << localTime.hour);
            dcfVideoRecCurDir->SumManual[index]++;
            dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_MANU;
            break;

        case 'R':
            if(!dcfVideoRecCurDir->PosRing)
                dcfVideoRecCurDir->PosRing = 0x80 | (pFile->FileEntrySect & 0x3F);
            dcfVideoRecCurDir->DistributionRing[index] |= (1 << localTime.hour);
            dcfVideoRecCurDir->SumRing[index]++;
            dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_RING;
            break;

#if PIR_FALSETRIG_TEST
        case 'F':
            if(!dcfVideoRecCurDir->PosSchedule)
                dcfVideoRecCurDir->PosSchedule = 0x80 | (pFile->FileEntrySect & 0x3F);
            dcfVideoRecCurDir->DistributionSchedule[index] |= (1 << localTime.hour);
            dcfVideoRecCurDir->SumSchedule[index]++;
            dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_SCHE;
            break;
#endif

        default:
            if((newFileName[0] == 'A') || newFileName[0] == 'P')
            {
                if(!dcfVideoRecCurDir->PosManual)
                    dcfVideoRecCurDir->PosManual = 0x80 | (pFile->FileEntrySect & 0x3F);
                dcfVideoRecCurDir->DistributionManual[index] |= (1 << localTime.hour);
                dcfVideoRecCurDir->SumManual[index]++;
                dcfVideoRecCurDir->CH_Distr |= DCF_DISTRIB_MANU;
            }
            break;
    }
    dcfVideoRecCurDir->ChTotal[index]++;
    dcfVideoRecCurDir->FileTotal++;
    dcfVideoRecCurDir->CH_Distr |= (1 << localTime.hour);

    DEBUG_DCF2("[INF] NumOfFiles: %d, NumOfDirs: %d\n", dcfVideoRecCurDir->FileTotal, dcfGetTotalDirCount());	//

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    DEBUG_GREEN("hourVal: %#x\n", localTime.hour);
    DEBUG_GREEN("DistributionSchedule: %#x, %#x, %#x, %#x\n"
                "DistributionDetect: %#x, %#x, %#x, %#x\n"
                "DistributionManual: %#x, %#x, %#x, %#x\n",
                dcfVideoRecCurDir->DistributionSchedule[0], dcfVideoRecCurDir->DistributionSchedule[1],
                dcfVideoRecCurDir->DistributionSchedule[2], dcfVideoRecCurDir->DistributionSchedule[3],
                dcfVideoRecCurDir->DistributionDetect[0], dcfVideoRecCurDir->DistributionDetect[1],
                dcfVideoRecCurDir->DistributionDetect[2], dcfVideoRecCurDir->DistributionDetect[3],
                dcfVideoRecCurDir->DistributionManual[0], dcfVideoRecCurDir->DistributionManual[1],
                dcfVideoRecCurDir->DistributionManual[2], dcfVideoRecCurDir->DistributionManual[3]);
#endif

    DEBUG_DCF2("Leave CDVR dcfCreateNextFile\n\n");
    OSSemPost(dcfReadySemEvt);
    return pFile;
}

/*

Routine Description:

	The directory initialization routine of DCF.

1. check if the directory of date code exists
2. if not, create it
3. record all Directory in dcfDirEnt



Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
int dcfDirInit(void)
{
    FS_DIRENT *pDirEnt;
    u32 NumOfDirs;
    char DirName[32];

    DEBUG_DCF2("[INF] DCF Dir Init: Begin.\n");

    dcfVideoDirNumLast = 0;
    pDirEnt = dcfDirEnt;

    // list root directory error
    memset_hw((void*) pDirEnt, 0, sizeof(FS_DIRENT) * DCF_DIRENT_MAX);
    if (dcfDir("\\", pDirEnt, &NumOfDirs, 1, 1, DCF_DIRENT_MAX) == 0)
        DEBUG_DCF("[WARM] DCF list root dir empty.\n");

#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    uiCheckUpgradeFileName(dcfDirEnt, NumOfDirs);
#elif ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_MAYON))
    uiCheckTXUpgradeFileName();
#endif

    sprintf ((char*)DirName, "\\%s", gsDirName);
    if(dcfChDir(DirName) == 0)
    {
        // change directory \DCIM error
        if (Write_protet() == 1)
        {
            // Write protect
            DEBUG_DCF("[ERR] DCF Write protect.\n");
            return -1;
        }

        if (dcfMkDir(DirName) < 0)
        {
            // make directory \DCIM error
            DEBUG_DCF("[ERR] DCF make dir %s failed.\n", DirName);
            return -1;
        }

        if(dcfChDir(DirName) == 0)
        {
            // change directory \DCIM error
            DEBUG_DCF("[ERR] DCF Change dir to %s failed.\n", DirName);
            return -1;
        }
    }

    // list directory \gsDirName
    memset_hw((void *) pDirEnt, 0, sizeof(FS_DIRENT) * DCF_DIRENT_MAX);
    memset_hw((void *) dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    // List all the directory which in gsDirName civic
    if(dcfDir(DirName, pDirEnt, &NumOfDirs, 1, 1, DCF_DIRENT_MAX) == 0)
        DEBUG_DCF("[WARM] List dir %s failed.\n", DirName);
    DEBUG_DCF2("[INF] DCF Dir count: %d.\n", NumOfDirs);

    if(dcfSetDirEntList(NumOfDirs) < 0)
    {
        DEBUG_DCF("[ERR] DCF set dir list failed.\n");
        return -1;
    }

#if 1
    {
        // Print out the dir list.
        DCF_LIST_DIRENT *pTempListEnt = NULL;
        if (dcfListDirEntHead == NULL)
            return 1;

        for(pTempListEnt = dcfListDirEntHead; ; pTempListEnt = pTempListEnt->next)
        {
            DEBUG_DCF("Dir Name: %s\n", pTempListEnt->pDirEnt->d_name);
            if(pTempListEnt == dcfListDirEntHead)
                break;
        }
    }
#endif

    return 1;
}



/*
Routine Description:
	The file initialization routine of DCF.

Arguments:
	pListDirEnt - Current directory entry.

Return Value:
	0 - Failure.
	1 - Success.
*/
int dcfFileInit(DCF_LIST_DIRENT *pListDirEnt, u8 Type)
{
    u8 DelBadFile;
    char DirName[32];

    //
    DEBUG_DCF2("[INF] DCF File Init: Begin (%d).\n", Type);

    if(pListDirEnt == NULL)
    {
        DEBUG_DCF("[ERR] DCF directory empty.\n");
        return 1;
    }

    if((Type == PLAYFILEENTRY) || (Type == SCANFILEENTRY) || (Type == CURRENTFILEENTRY))
    {
        switch(Type)
        {
            case PLAYFILEENTRY:
            case SCANFILEENTRY:
                DelBadFile = 0;
                break;

            case CURRENTFILEENTRY:
            default:
                DelBadFile = 1;
                break;
        }

        dcfFileTypeCount_Clean();

        // Scaning the playback directory and remove the bad file before put into the entry list.
        // 1. Go to dir \gsDirName first.
        sprintf ((char*)DirName, "\\%s", gsDirName);
        if(dcfChPlayDir(DirName) == 0)
        {
            DEBUG_DCF("[ERR] DCF change dir to %s failed.\n", DirName);
            return -1;
        }

        DEBUG_DCF2("[INF] Directory name: %s\n", pListDirEnt->pDirEnt->d_name);
        // 2. Diving into the playback directory below \gsDirName.
        if(dcfChPlayDir(pListDirEnt->pDirEnt->d_name) == 0)
        {
            DEBUG_DCF("[ERR] DCF change current to dir failed.\n");
            return -1;
        }

        // 3. Listing the file entries in current directory
        memset_hw((void*)dcfFileEnt, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
        if(dcfPlayDirScan(NULL, dcfFileEnt, &dcfVideoFileCnt, &dcfFileOldestIndex, DelBadFile) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
        {
            DEBUG_DCF("[ERR] DCF list current dir failed.\n");
            return -1;
        }

        // 4. Sorting the list by the dcfFileOldestIndex(Head).
        memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
        if(dcfSetFileEntList(dcfVideoFileCnt) < 0)
        {
            DEBUG_DCF("[ERR] DCF file list sorting failed.\n");
            return -1;
        }

        // 5. Caching the distribution info of files in directory entry
        dcfSetFileDistInDir(pListDirEnt, dcfListFileEntHead);

        // 6. Filter the Cam List file
        dcfCamListFilter(dcfListFileEnt, sysPlaybackCamList | sysPlaybackType);

        return 1;

    }
    else if(Type == DELFILEENTRY)
        return 1;
    else
        return -1;
}

int dcfCloseFileByIdx(FS_FILE *pFile, u8 idx, u8 *pOpenFile)
{
    int rel;
    u8 err;

    if (idx >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfClose Error Index %d\n",idx);
        return 0;
    }

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    rel = dcfClose(pFile, pOpenFile);
    DEBUG_DCF2("[INF] DCF file is closed already. Ch: %d\n", idx);
    dcfNewFile = 1;

    OSSemPost(dcfReadySemEvt);
    return rel;
}

s32 dcfOverWriteDel(void)
{
    int ret;
    u8 err;

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(dcfListDelDirEnt != dcfListDirEntHead)
    {
        dcfListDelDirEnt = dcfListDirEntHead;
        if(dcfListDirEntHead == NULL)
        {
            DEBUG_DCF("[ERR] DCF Head of Dir entrys is NULL.\n");
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
    }

    if((ret = dcfOWDel(dcfListDelDirEnt->pDirEnt->d_name)) < 0)
    {
        DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    dcfListDelDirEnt->FileTotal--;
    if(dcfPlaybackCurDir == dcfListDelDirEnt)
    	dcfVideoFileCnt--;

    if(ret == 0)
    {
        DEBUG_DCF("[INF] Directory is empty.\n");
        if((ret = dcfOWDelDir(dcfListDelDirEnt->pDirEnt->d_name)) < 0)
        {
            DEBUG_DCF("[ERR] DCF delete dir fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        dcfFreeDirEnt(&dcfListDelDirEnt);
        // Reassign Dir pointer
        dcfListDelDirEnt = dcfListDirEntHead;
        if(dcfListDelDirEnt)
            DEBUG_DCF("[INF] DCF switch del dir to %s.\n", dcfListDelDirEnt->pDirEnt->d_name);
    }

    // Start rescan in next playback mode
    dcfDirRescanSwitch = 1;

    OSSemPost(dcfReadySemEvt);
    return 1;
}

#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
s32 dcfOverWriteDel_bgT(s32 OWDays, s32 dummy1, s32 dummy2, s32 dummy3)
{
    u32 NumOfDelDir, LastOneTotalDirCount;
    int ret;

    if(dcfOWclk == 1)
        return 1;

    DEBUG_DCF2("Enter dcfOverWriteDel bgt....\n");
    dcfOWclk = 1;
    NumOfDelDir = 0;
    LastOneTotalDirCount = dcfGetTotalDirCount();

    do
    {
        if((ret = dcfOverWriteDel()) <= 0)
        {
            DEBUG_DCF("[ERR] DCF OWDel bdt fail.\n");
            dcfOWclk = 0;
            return 0;
        }

        // Check the number of total directory whether was changeed or not.
        if(LastOneTotalDirCount != dcfGetTotalDirCount())
        {
            LastOneTotalDirCount = dcfGetTotalDirCount();
            NumOfDelDir++;
        }
    }
    while(NumOfDelDir < OWDays);

    dcfOWclk = 0;
    DEBUG_DCF2("Leave dcfOverWriteDel bgt....\n");
    return 1;
}
#endif

#if (CDVR_LOG || CDVR_TEST_LOG)
//s32 dcfCreatLogFile(u8* pucaddr, u32 u32bufsize)
FS_FILE* dcfCreatLogFile(void)
{
    FS_FILE* pFile;
    char LogFileName[32];
    u32 writesize;

    strncpy(LogFileName, (s8*)dcfListFileEntTail->pDirEnt->d_name, 9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if ((pFile = dcfOpen((s8 *)LogFileName, "w-")) == 0)
    {
        /* create next file error */
        DEBUG_DCF("dcfOpen(%s) error!!!\n", LogFileName);
        return 0;
    }

    /*
    if(dcfWrite(pFile, pucaddr, u32bufsize, &writesize) == 0)
    {
        DEBUG_DCF("dcfWrite(%s) error!!!\n", LogFileName);
        if(dcfClose(pFile) == 0)
            DEBUG_DCF("dcfClose(%s) error!!!\n", LogFileName);
        return 0;
    }

    if(dcfClose(pFile) == 0)
    {
        DEBUG_DCF("dcfClose(%s) error!!!\n", LogFileName);
    	return 0;
    }

    return  1;
    */

    return  pFile;
}
#endif

/*
Routine Description:
	Playback directory forward.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfPlaybackDirForward(void)
{
    u8 err;

    DEBUG_DCF2("Enter dcfPlaybackDirForward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF2("Leave dcfPlaybackDirForward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("No dcfPlaybackCurDir\n");
    return 0;
}


/*
Routine Description:
	Playback directory backward.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfPlaybackDirBackward(void)
{
    u8 err;

    DEBUG_DCF2("Enter dcfPlaybackDirBackward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF2("Leave dcfPlaybackDirBackward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("No dcfPlaybackCurDir\n");
    return 0;
}

/*
Routine Description:
	Playback next file.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfPlaybackFileNext(void)
{
    u8 err;

    //DEBUG_DCF("Enter dcfPlaybackFileNext....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFileNext: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No next file.\n");
        return 0;
    }
    dcfPlaybackCurFile = dcfPlaybackCurFile->CamNext;

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFileNext\n");
    return 1;
}

/*
Routine Description:
	Playback previous file.

Arguments:
	None.

Return Value:
	0 - Failure.
	1 - Success.
*/
s32 dcfPlaybackFilePrev(void)
{
    u8 err;

    //DEBUG_DCF("Enter dcfPlaybackFilePrev....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFilePrev: begin ,dcfListDirCount=%d\n",dcfListDirCount);

    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No previous file.\n");
        return 0;
    }
    dcfPlaybackCurFile = dcfPlaybackCurFile->CamPrev;

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFilePrev\n");
    return 1;
}

/*

Routine Description:

	Playback directory Scan File.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfScanFileOnPlaybackDir(void)
{
    INT8U err;

    DEBUG_DCF2("Enter dcfScanFileOnPlaybackDir....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if(dcfFileInit(dcfPlaybackCurDir, PLAYFILEENTRY) >= 0)
    {
        // check if file of list head is null
        if (dcfListFileEntHead)
        {
            // set current file
            //dcfPlaybackCurFile = dcfListFileEntTail;
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF2("Leave dcfScanFileOnPlaybackDir\n");
            return 1;
        }
    }
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("[ERR] DCF File Init failed.\n");
    return 0;
}

//Lucian: CHmap = 0x01,0x3,0x7,0xf, Typesel = 'D', 'M', 'S', 'A'
int dcfSearchFileOnPlaybackDir(char CHmap, u32 Typesel, u32 StartMin, u32 EndMin)
{
    DCF_LIST_DIRENT *pListDirEnt;
    u8 err;
    char DirName[32];

    if (dcfPlaybackCurDir == NULL)
    {
        DEBUG_DCF("[WARM] DCF directory empty.\n");
        return 0;
    }

    //
    DEBUG_DCF2("Enter dcSearchFileOnPlaybackDir....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY | OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    pListDirEnt = dcfPlaybackCurDir;
    dcfFileTypeCount_Clean();

    sprintf((char*)DirName, "\\%s", gsDirName);
    if(dcfChPlayDir(DirName) == 0)
    {
        DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    DEBUG_DCF("[INF] Dir name: %s\n", pListDirEnt->pDirEnt->d_name);
    // change current directory
    if(dcfChPlayDir(pListDirEnt->pDirEnt->d_name) == 0)
    {
        DEBUG_DCF("[ERR] Change current to dir failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    // list current directory
    memset_hw((void*)dcfFileEnt, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
    if(dcfPlayDirSearch(NULL, dcfFileEnt, &dcfVideoFileCnt, &dcfFileOldestIndex, CHmap, Typesel, StartMin, EndMin, 0) == 0)
    {
        DEBUG_DCF("[ERR] List current dir failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    // sort the file and create the file list
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    if(dcfSetFileEntList(dcfVideoFileCnt) < 0)
    {
        DEBUG_DCF("[ERR] DCF file list sorting failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    // 5. Caching the distribution info of files in directory entry
    dcfSetFileDistInDir(pListDirEnt, dcfListFileEntHead);

    // 6. Filter the Cam List file
    dcfCamListFilter(dcfListFileEnt, CHmap | DCF_DISTRIB_ALL_TYPE);

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("Leave dcfScanFileOnPlaybackDir\n");
    return 1;
}

int dcfPlaybackDirDetailCounts(DCF_LIST_DIRENT *CurDir)
{
    FS_SearchCondition SCondition;
    FS_DIRENT *pEnt;
    INT32U Idx, i;
    INT32U ChVal, HourVal;
    INT8U err;
    char DirName[32];

    if(CurDir == NULL)
    {
        DEBUG_DCF("[W] DCF parameter is null.\n");
        return 0;
    }

    DEBUG_DCF2("Enter dcfPlaybackDirDetailCounts\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY | OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    sprintf((char*)DirName, "\\%s", gsDirName);
    // change current directory
    if(dcfChPlayDir(DirName) == 0)
    {
        DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    DEBUG_DCF("[INF] Dir name: %s\n", CurDir->pDirEnt->d_name);
    // change current directory
    if(dcfChPlayDir(CurDir->pDirEnt->d_name) == 0)
    {
        DEBUG_DCF("[ERR] Change current to dir failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    // Clean parameters that about the statistics.
    CurDir->CH_Distr = 0x0;
    CurDir->FileTotal = 0x0;
    memset(CurDir->ChTotal, 0x0, sizeof(u32) * DCF_MAX_MULTI_FILE);
    memset(&CurDir->PosSchedule, 0x0, sizeof(u16) * 3);
    memset(CurDir->SumSchedule, 0x0, sizeof(u16) * DCF_MAX_MULTI_FILE * 3);
    memset(CurDir->DistributionSchedule, 0x0, sizeof(u32) * DCF_MAX_MULTI_FILE * 3);

    for(Idx = 0; ; )
    {
        // Set search condition
        memset(&SCondition, 0x0, sizeof(FS_SearchCondition));
        SCondition.EntryType = DCF_FAT_ATTR_ARCHIVE;
        SCondition.StartNumOfInOrder = Idx;
        SCondition.LimitOfEntSize = DCF_FILEENT_MAX;
        SCondition.BadFileAction = 1;

        // Do search
        if(dcfFetchDirItems(NULL, dcfFileEnt, &SCondition) == 0)
        {
            DEBUG_DCF("[ERR] List current dir %s failed.\n", CurDir->pDirEnt->d_name);
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            return 0;
        }

        for(i = 0; i < SCondition.CntOfResult; i++)
        {
            pEnt = (dcfFileEnt + i);

            switch(pEnt->d_name[6])
            {
                case 'D':
                case 'S':
                case '-':
                case 'M':
                case 'R':
                case 'F':
                    sscanf(pEnt->d_name, "%2d%*2d%*2d%*1s%1d*", &HourVal, &ChVal);
                    break;

                default:	// PM, AM.
                    sscanf(pEnt->d_name, "%*1s%2d%*2d%*2d%1d*", &HourVal, &ChVal);
                    break;
            }
            ChVal -= 1;

            switch(pEnt->d_name[6])
            {
                case 'D':
                    if(CurDir->PosDetect == 0)
                        CurDir->PosDetect = 0x80 | (CurDir->pDirEnt->FileEntrySect & 0x3F);
                    CurDir->DistributionDetect[ChVal] |= (1 << HourVal);
                    CurDir->SumDetect[ChVal]++;
                    CurDir->CH_Distr |= DCF_DISTRIB_MOTN;
                    break;
                    
                case 'S':
                    if(CurDir->PosSchedule == 0)
                        CurDir->PosSchedule = 0x80 | (CurDir->pDirEnt->FileEntrySect & 0x3F);
                    CurDir->DistributionSchedule[ChVal] |= (1 << HourVal);
                    CurDir->SumSchedule[ChVal]++;
                    CurDir->CH_Distr |= DCF_DISTRIB_SCHE;
                    break;
                    
                case '-':
                case 'M':
                case 'F':
                    if(CurDir->PosManual == 0)
                        CurDir->PosManual = 0x80 | (CurDir->pDirEnt->FileEntrySect & 0x3F);
                    CurDir->DistributionManual[ChVal] |= (1 << HourVal);
                    CurDir->SumManual[ChVal]++;
                    CurDir->CH_Distr |= DCF_DISTRIB_MANU;
                    break;

                case 'R':
                    if(CurDir->PosRing == 0)
                        CurDir->PosRing = 0x80 | (CurDir->pDirEnt->FileEntrySect & 0x3F);
                    CurDir->DistributionRing[ChVal] |= (1 << HourVal);
                    CurDir->SumRing[ChVal]++;
                    CurDir->CH_Distr |= DCF_DISTRIB_RING;
                    break;

                default:
                    if((pEnt->d_name[0] == 'A') || pEnt->d_name[0] == 'P')
                    {
                        if(CurDir->PosManual == 0)
                            CurDir->PosManual = 0x80 | (CurDir->pDirEnt->FileEntrySect & 0x3F);
                        CurDir->DistributionManual[ChVal] |= (1 << HourVal);
                        CurDir->SumManual[ChVal]++;
                    }
                    break;
            }
            
            CurDir->ChTotal[ChVal]++;
            CurDir->FileTotal++;
            CurDir->CH_Distr |= (1 << ChVal);
        }

        // End condition
        if(SCondition.CntOfResult < DCF_FILEENT_MAX)
        {
            break;
        }

        Idx += SCondition.CntOfResult;
    }    

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("Leave dcfPlaybackDirDetailCounts\n");
    return 1;
}

int dcfPlaybackDirDetailSearch(u32 *pDirCount, u32 NumOfMaxEnt, u32 IdxOfFirstItem, u8 CamList, u16 TypeSel, 
                                u32 StartSec, u32 EndSec, u8 DelBadFile, u8 LightingSearch)
{
    FS_SearchCondition SCondition;
    DCF_LIST_DIRENT *pListDirEnt;
    u8 err;
    char DirName[32];

    if(dcfPlaybackCurDir == NULL)
    {
        DEBUG_DCF("[WARM] DCF directory empty.\n");
        return 0;
    }

    //
    DEBUG_DCF2("Enter dcfPlaybackDirDetailSearch....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY | OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    pListDirEnt = dcfPlaybackCurDir;
    dcfFileTypeCount_Clean();

    sprintf((char*)DirName, "\\%s", gsDirName);
    if(dcfChPlayDir(DirName) == 0)
    {
        DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    DEBUG_DCF("[INF] Dir name: %s\n", pListDirEnt->pDirEnt->d_name);
    // change current directory
    if(dcfChPlayDir(pListDirEnt->pDirEnt->d_name) == 0)
    {
        DEBUG_DCF("[ERR] Change current to dir failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    // list current directory
    memset(&SCondition, 0x0, sizeof(FS_SearchCondition));
    SCondition.FilterEnable = 1;
    SCondition.EntryType = DCF_FAT_ATTR_ARCHIVE;
    SCondition.StartNumOfInOrder = IdxOfFirstItem;
    SCondition.LimitOfEntSize = NumOfMaxEnt;
    SCondition.BadFileAction = DelBadFile;
    SCondition.ChannelMap = CamList;
    SCondition.SearchMode = TypeSel;
    SCondition.StartTime = StartSec;
    SCondition.EndTime = EndSec;
    SCondition.NameTypeMask = dcfGetPlaybackNameMaskIndex();

    SCondition.LightWeightSearch = LightingSearch;
    
    memset_hw((void*)dcfFileEnt, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
    if(dcfFetchDirItems(NULL, dcfFileEnt, &SCondition) == 0)
    {
        DEBUG_DCF("[ERR] List current dir failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    dcfVideoFileCnt = SCondition.CntOfResult;
    // sort the file and create the file list
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    if(dcfSetFileEntList(dcfVideoFileCnt) < 0)
    {
        DEBUG_DCF("[ERR] DCF file list sorting failed.\n");
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        return 0;
    }

    *pDirCount = dcfVideoFileCnt;

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF2("Leave dcfPlaybackDirDetailSearch\n");
    return 1;
}


/*

Routine Description:

	Playback delete the all files in the current directory.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfPlaybackDelDir(void)
{
    int ret;
    u8 err;

    DEBUG_DCF2("Enter dcfPlaybackDelDir.\n");
    do
    {
        OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

        if(dcfPlaybackCurDir == NULL)
        {
            DEBUG_DCF("[ERR] DCF No current Dir.\n");
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        if((ret = dcfOWDel(dcfPlaybackCurDir->pDirEnt->d_name)) < 0)
        {
            DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfPlaybackCurDir->pDirEnt->d_name);
        }

        if((ret < -1) || (sysGetStorageInserted(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_OFF))
        {
        	OSSemPost(dcfReadySemEvt);
            return 0;
        }

        dcfPlaybackCurDir->FileTotal--;

        if(ret == 0)
        {
            DEBUG_DCF("[INF] Directory is empty.\n");
            if((ret = dcfOWDelDir(dcfPlaybackCurDir->pDirEnt->d_name)) < 0)
            {
                DEBUG_DCF("[ERR] DCF delete dir fail. %s\n", dcfPlaybackCurDir->pDirEnt->d_name);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }

            dcfFreeDirEnt(&dcfPlaybackCurDir);
            dcfPlaybackCurDir = dcfPlaybackCurDir->next;
            // Start rescan in next playback mode
            dcfDirRescanSwitch = 1;

            OSSemPost(dcfReadySemEvt);
            DEBUG_DCF2("Leave dcfPlaybackDelDir.\n");
            return 1;
        }
        OSSemPost(dcfReadySemEvt);
    }
    while(1);
}

/*

Routine Description:

	Playback delete current file.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfPlaybackDel(void)
{
    int ret;
    u8 err, CamId, ucfiletype;

    DEBUG_DCF2("Enter dcfPlaybackDel\n");
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (dcfListFileEntHead == NULL)
    {
        DEBUG_DCF("Trace: No current file.\n");
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    if((ucfiletype = dcfGetFileType(&dcfListFileEntHead->pDirEnt->d_name[9])) == DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("Error: file type doesn't exist, %d\n", ucfiletype);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);

    osdDrawDelMsg((s8 *)dcfPlaybackCurFile->pDirEnt->d_name, 0);
    if(dcfPlayDel(dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name) == 0)
    {
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    CamId = dcfPlaybackCurFile->pDirEnt->d_name[7]-'1';
    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;

    dcfPlaybackCurDir->ChTotal[CamId]--;
    dcfPlaybackCurDir->FileTotal--;	// ?
    dcfVideoFileCnt--;

    // Prepare the timeline params to save time of Playback search
    switch(dcfPlaybackCurFile->pDirEnt->d_name[6])
    {
        case 'D':
            if((--dcfPlaybackCurDir->SumDetect[CamId]) == 0x0)
            {
                dcfPlaybackCurDir->PosDetect = 0x0;
                dcfPlaybackCurDir->DistributionDetect[CamId] = 0x0;
                dcfPlaybackCurDir->CH_Distr &= ~DCF_DISTRIB_MOTN;
            }
            break;
        case 'S':
            if((--dcfPlaybackCurDir->SumSchedule[CamId]) == 0x0)
            {
                dcfPlaybackCurDir->PosSchedule = 0x0;
                dcfPlaybackCurDir->DistributionSchedule[CamId] = 0x0;
                dcfPlaybackCurDir->CH_Distr &= ~DCF_DISTRIB_SCHE;
            }
            break;
        case '-':
        case 'M':
            if((--dcfPlaybackCurDir->SumManual[CamId]) == 0x0)
            {
                dcfPlaybackCurDir->PosManual = 0x0;
                dcfPlaybackCurDir->DistributionManual[CamId] = 0x0;
                dcfPlaybackCurDir->CH_Distr &= ~DCF_DISTRIB_MANU;
            }
            break;
        case 'R':
            if((--dcfPlaybackCurDir->SumRing[CamId]) == 0x0)
            {
                dcfPlaybackCurDir->PosRing = 0x0;
                dcfPlaybackCurDir->DistributionRing[CamId] = 0x0;
                dcfPlaybackCurDir->CH_Distr &= ~DCF_DISTRIB_RING;
            }
            break;
        default:
            if((dcfPlaybackCurFile->pDirEnt->d_name[0] == 'A') || dcfPlaybackCurFile->pDirEnt->d_name[0] == 'P')
            {
                if((--dcfPlaybackCurDir->SumManual[CamId]) == 0x0)
                {
                    dcfPlaybackCurDir->PosManual = 0x0;
                    dcfPlaybackCurDir->DistributionManual[CamId] = 0x0;
                    dcfPlaybackCurDir->CH_Distr &= ~DCF_DISTRIB_MANU;
                }
            }
            break;
    }

    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        dcfPlaybackCurFile->CamPrev->CamNext = dcfPlaybackCurFile->CamNext;
        dcfPlaybackCurFile->CamNext->CamPrev = dcfPlaybackCurFile->CamPrev;
        if(dcfPlaybackCurFile == dcfListFileEntHead)
            dcfListFileEntHead = dcfPlaybackCurFile->next;
        if(dcfPlaybackCurFile == dcfPlaybackListFileEntHead)
            dcfPlaybackListFileEntHead = dcfPlaybackCurFile->CamNext;
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfPlaybackListFileEntTail = dcfPlaybackListFileEntHead->CamPrev;

        dcfPlaybackCurFile = dcfPlaybackCurFile->CamNext;
    }
    else
    {
        dcfPlaybackCurFile = dcfListFileEntHead = dcfPlaybackListFileEntTail = NULL;
        dcfListFileEntHead = dcfListFileEntTail = NULL;
    }

    if(dcfPlaybackCurDir->FileTotal == 0)
    {
        DEBUG_DCF("[INF] Directory is empty.\n");
        if((ret = dcfOWDelDir(dcfPlaybackCurDir->pDirEnt->d_name)) < 0)
        {
            DEBUG_DCF("[ERR] DCF delete dir fail. %s\n", dcfPlaybackCurDir->pDirEnt->d_name);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        dcfFreeDirEnt(&dcfPlaybackCurDir);
    }

    // Start rescan in next playback mode
    dcfDirRescanSwitch = 1;

    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF2("Leave dcfPlaybackDel\n");
    return 1;
}

/*

Routine Description:

	Playback delete all files.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfPlaybackDelAll(void)
{
    u32 tmp;
    int ret;

    DEBUG_DCF2("Enter dcfPlaybackDelAll.\n");

    system_busy_flag = 1;
    // Stop record
#if (MULTI_CHANNEL_VIDEO_REC)
    for(tmp = 0; tmp < DCF_MAX_MULTI_FILE; tmp++)
        sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, tmp);
#endif

    do
    {
        if((ret = dcfPlaybackDelDir()) <= 0)
        {
            system_busy_flag = 0;
            return ret;
        }
    }
    while(dcfGetTotalDirCount());

    system_busy_flag = 0;
    DEBUG_DCF2("Leave dcfPlaybackDelAll.\n");
    return 1;

}

int dcfScanDiskAll(u8 year, u8 month, u32 CamList, u32 TypeList)
{
    DCF_LIST_DIRENT *curDir;
    u32 dirYear, dirMonth;
    char DirName[32];

    //----------------//
    if(CamList == 0x0)
    {
        DEBUG_DCF("[INF] No cam in list.\n");
        return 1;
    }

    DEBUG_DCF("Enter dcfScanDiskAll....\n");
    dcfFileTypeCount_Clean();

    curDir = dcfListDirEntHead;
    do
    {
        if (curDir == NULL)
        {
            DEBUG_DCF("[WARM] Directory Empty\n");
            break;
        }

        dirYear  = (curDir->pDirEnt->fsFileCreateDate_YMD >> 9)-20;
        dirMonth = (curDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0) >> 5;
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
        if ((year == dirYear) && (month == dirMonth) &&
                (curDir->PosDetect || curDir->PosSchedule || curDir->PosManual))
        {
            DEBUG_DCF("@#Directory name: %s\n", curDir->pDirEnt->d_name);
            DEBUG_BLUE("DistributionSchedule: %#x, %#x, %#x, %#x\n" \
                       "DistributionDetect: %#x, %#x, %#x, %#x\n" \
                       "DistributionManual: %#x, %#x, %#x, %#x\n",
                       curDir->DistributionSchedule[0], curDir->DistributionSchedule[1],
                       curDir->DistributionSchedule[2], curDir->DistributionSchedule[3],
                       curDir->DistributionDetect[0], curDir->DistributionDetect[1],
                       curDir->DistributionDetect[2], curDir->DistributionDetect[3],
                       curDir->DistributionManual[0], curDir->DistributionManual[1],
                       curDir->DistributionManual[2], curDir->DistributionManual[3]);
        }
        else if ((year == dirYear) && (month == dirMonth))
#else
        if ((year == dirYear) && (month == dirMonth))
#endif
        {
            //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
            dcfPlaybackCurFile = NULL;
            dcfVideoFileCnt = 0;

            sprintf ((char*)DirName, "\\%s", gsDirName);
            if (dcfChPlayDir(DirName) == 0)
            {
                DEBUG_DCF("[ERR] DCF change dir to %s failed.\n", DirName);
                return 0;
            }

            DEBUG_DCF("Directory Name: %s\n",curDir->pDirEnt->d_name);
            sysDeadLockMonitor_Reset();  //Lsk: MR9300 search too long, avoid reboot.

            // change current directory
            if (dcfChPlayDir(curDir->pDirEnt->d_name) == 0)
            {
                DEBUG_DCF("[ERR] DCF change current directory failed.\n");
                return 0;
            }

            // list current directory
            memset_hw((void*)dcfFileEnt, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
            if (dcfPlayDirSearch(NULL, dcfFileEnt, &dcfVideoFileCnt, &dcfFileOldestIndex, CamList, TypeList, 0, 24*60, 0) == 0)
            {
                DEBUG_DCF("[ERR] list current directory failed.\n");
                return 0;
            }

            // Sorting the list by the dcfFileOldestIndex(Head).
            memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
            if(dcfSetFileEntList(dcfVideoFileCnt) < 0)
            {
                DEBUG_DCF("[ERR] DCF file list sorting failed.\n");
                return -1;
            }

            // Caching the distribution info of files in directory entry
            dcfSetFileDistInDir(curDir, dcfListFileEntHead);

            OSTimeDly(1);
        }

        // next directory
        DEBUG_DCF("ChDistrInDir = %#x\n", curDir->CH_Distr);
        curDir = curDir->next;
    }
    while(curDir != dcfListDirEntHead);

    DEBUG_DCF("Leave dcfScanDiskAll....\n");
    return 1;
}

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
s32 dcfScanDiskAll_bdT(s32 dummy)
{
    DCF_LIST_DIRENT *curDir;
    u32 fileCount;
    int ret;
    char DirName[32];


    //----------------//
    DEBUG_CYAN("Enter dcfScanDiskAll_bdT....\n");
    ret = 1;
    //==============//
    if(dcfListDirCurEntBGScan == NULL)
        dcfListDirCurEntBGScan = dcfListDirEntHead;

    curDir = dcfListDirCurEntBGScan;
    do
    {
        if(curDir == NULL)
        {
            DEBUG_DCF("[WARM] Directory Empty\n");
            ret = 0;
            break;
        }

        sprintf ((char*)DirName, "\\%s", gsDirName);
        if(dcfChPlayDir(DirName) == 0)
        {
            DEBUG_DCF("[ERR] DCF change dir to %s failed.\n", DirName);
            ret = 0;
            break;
        }
        sysDeadLockMonitor_Reset();

        // change current directory
        if(dcfChPlayDir(curDir->pDirEnt->d_name) == 0)
        {
            DEBUG_DCF("[ERR] DCF change current directory failed.\n");
            ret = 0;
            break;
        }

        // list current directory
        memset_hw((void*)dcfFileEntBG, 0, sizeof(FS_DIRENT) * DCF_FILEENT_MAX);
        if(dcfPlayDirScan(NULL, dcfFileEntBG, &fileCount, &dcfFileOldestIndex, 0) == 0)
        {
            DEBUG_DCF("[ERR] list current directory failed.\n");
            ret = 0;
            break;
        }

        DEBUG_DCF("[INF] Dir name: %s, NumOfFiles: %#x\n", curDir->pDirEnt->d_name, fileCount);

        // Sorting the list by the dcfFileOldestIndex(Head).
        memset_hw((void*)dcfListFileEntBG, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
        if(dcfSetFileEntListBG(fileCount) < 0)
        {
            DEBUG_DCF("[ERR] DCF file list sorting failed.\n");
            ret = 0;
        }

        // Caching the distribution info of files in directory entry
        dcfSetFileDistInDir(curDir, dcfListFileEntBGHead);

    }
    while(0);

    dcfListDirCurEntBGScan = dcfListDirCurEntBGScan->next;
    DEBUG_CYAN("Leave dcfScanDiskAll_bdT....\n");
    if(dcfListDirCurEntBGScan != dcfListDirEntHead)
        sysSetEvt(SYS_EVT_SCANFILE, 0);
    return ret;
}
#endif

s32 dcfPlaybackCalendarInit(u8 year, u8 month, u32 CamList, u32 TypeList, u8 Rescan)
{
    DCF_LIST_DIRENT *PlaytmpDir;
    u32 dirYear, dirMonth, dirDay;
    u32 i;
    u8 firstDir = 0;

    if(Rescan)
        dcfScanDiskAll(year, month, CamList, TypeList);

    memset(dcfPlaybackDayInfo, 0, sizeof(dcfPlaybackDayInfo));
    dcfListPlaybackDirHead = NULL;
    dcfListPlaybackDirTail = NULL;
    PlaytmpDir = dcfListDirEntHead;

    DEBUG_DCF("dcfPlaybackCalendarInit %d \n", dcfGetTotalDirCount());
    for (i = 0; i < dcfGetTotalDirCount(); i++)
    {
        dirYear  = (PlaytmpDir->pDirEnt->fsFileCreateDate_YMD >> 9) - 20;
        dirDay   = PlaytmpDir->pDirEnt->fsFileCreateDate_YMD & 0x001F;
        dirMonth = (PlaytmpDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;
        DEBUG_DCF("Dir Create Date %d:%d:%d\n", dirYear, dirMonth, dirDay);
        if((year != dirYear) || (month != dirMonth))
        {
            PlaytmpDir = PlaytmpDir->next;
            continue;
        }
        if((PlaytmpDir->CH_Distr & CamList) && ((PlaytmpDir->CH_Distr & TypeList) || (TypeList == DCF_DISTRIB_ALL_TYPE)))
        {
            if (dcfPlaybackDayInfo[dirDay-1].DirNum == 0)
            {
                // first one
                if (firstDir == 0)
                {
                    firstDir = 1;
                    dcfListPlaybackDirHead = PlaytmpDir;
                }
                dcfPlaybackDayInfo[dirDay-1].dirTail = PlaytmpDir;
                dcfPlaybackDayInfo[dirDay-1].dirHead = PlaytmpDir;
                PlaytmpDir->playbackNext = PlaytmpDir;
                PlaytmpDir->playbackPrev = PlaytmpDir;
            }
            else
            {
                PlaytmpDir->playbackNext = dcfPlaybackDayInfo[dirDay-1].dirHead;
                PlaytmpDir->playbackPrev = dcfPlaybackDayInfo[dirDay-1].dirTail;
                dcfPlaybackDayInfo[dirDay-1].dirHead->playbackPrev = PlaytmpDir;
                dcfPlaybackDayInfo[dirDay-1].dirTail->playbackNext = PlaytmpDir;
                dcfPlaybackDayInfo[dirDay-1].dirTail = PlaytmpDir;

            }
            dcfListPlaybackDirTail = PlaytmpDir;
            dcfPlaybackDayInfo[dirDay-1].DirNum++;
            DEBUG_DCF("[INF] Find Dir %s in PB list search.\n", PlaytmpDir->pDirEnt->d_name);
        }
        PlaytmpDir = PlaytmpDir->next;
    }
    return 1;
}
#endif
