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
#include "..\..\fs\inc\fs_fat.h"
#include "ramdiskapi.h"
#include "sdcapi.h"
#include "smcapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "spiapi.h"
#include "sysapi.h"
#include "uiapi.h"
#include "gpioapi.h"
#include "ispapi.h"     // ispUSBFileName

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR6730_AFN)
#include "..\..\ui\inc\MainFlow.h"  // FW_findIspFilename
#endif

#if(UI_VERSION == UI_VERSION_TRANWO)
#include "..\..\ui\inc\uiact_project.h"
#endif


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* DCF related definition */
#define CURRENTFILEENTRY     0x00
#define DELFILEENTRY         0x01
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
#define CUR_PIC_FILE_ENTRY   0x02
#define CUR_ALB_FILE_ENTRY   0x03
#endif
#define PLAYFILEENTRY        0x04
#define SCANFILEENTRY        0x05

//-------------------------------------//
#define FILEENTRY_AGROUP	0x02
#define FILEENTRY_BGROUP	0x03
#define FILEENTRY_CGROUP	0x04
#define FILEENTRY_DGROUP    0x05

#define APP_NUMS 6
#define APP_BUF_SIZE 0x400000

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
#define ALB_DIR   "\\Album"
#define PIC_DIR   "\\Picture"
#define MOV_DIR   "\\Movie"
#endif

typedef struct _DCFFILETYPE_INFO
{
    const char *pSubFileStr;
    u8  uiStrLength;
} DCFFILETYPE_INFO, *PDCFFILETYPE_INFO;

typedef struct _SALIX_APP_LIST
{
    u8			name[16];
    u32			approx_size;	/* for saving buffer */
    u32         pc_real_size;
    u32         real_size;
    u8			copy_flag;          // 0: no copy 1: need copying
}
SALIX_APP_LIST;




/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
#if( RX_SNAPSHOT_SUPPORT || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) )
u16 dcfPhotoMonthDistr[DCF_PHOTO_DIST_MAX]; //search 20 years
#endif
s8 dcfDecChar[] = "0123456789";
u8 dcfDayMonth[13] = {   0,  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31  };
OS_EVENT    *dcfReadySemEvt;

#if CDVR_iHome_LOG_SUPPORT
s8 gsLogDirName[9]=GS_DCF_LOGDIR_NAME;
#endif

#if RX_SNAPSHOT_SUPPORT
s8 gsPhotoDirName[9]=GS_DCF_PHOTODIR_NAME;
#endif


//---used for Saving file information
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
struct FS_DIRENT dcfDelFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT dcfListDelFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT* dcfListDelFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListDelFileEntTail = NULL; //指向file link的最後一個.
u32 dcfListDelFileCount;
s32 dcfDelFileCount;
#if CDVR_REC_WITH_PLAY_SUPPORT
struct FS_DIRENT dcfReadFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT dcfListReadFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT* dcfListReadFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListReadFileEntTail = NULL; //指向file link的最後一個.
u32 dcfListReadFileCount;
s32 dcfReadFileCount;
#endif

#if (ROOTWORK==0)
s8 gsDirName[9]=GS_DCFDIRNAME;
#endif
u8 InitRecoverDir=0;
s8 gdcfTempStr[32]; /* Dummy. Jan. Feb. Mar. Apr. May. Jun. Jul. Aug. Sep. Oct. Nov. Dec. */
u32 dcfDelFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
DCF_LIST_DIRENT* dcfListDelDirEnt = NULL;   //指向Dir link的第一個.
DCF_PLAYBACK_DAY_INFO  dcfPlaybackDayInfo[31];

#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
u32 global_total_Pic_file_count;
u32 dcfPicFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
u32 dcfDelPicFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
u32 dcfPicFileCount;
u32 dcfListPicFileCount;
s32 dcfDelPicFileCount;
u32 dcfListDelPicFileCount;
DCF_LIST_FILEENT* dcfListPicFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListPicFileEntTail = NULL; //指向file link的最後一個.
DCF_LIST_FILEENT* dcfListDelPicFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListDelPicFileEntTail = NULL; //指向file link的最後一個.
DCF_LIST_FILEENT dcfListPicFileEnt[DCF_PIC_FILE_PER_DIR];
struct FS_DIRENT dcfPicFileEnt[DCF_PIC_FILE_PER_DIR];
struct FS_DIRENT dcfDelFileEnt[DCF_FILEENT_MAX];
s32 dcfDelFileCount;
DCF_LIST_FILEENT* dcfListDelFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListDelFileEntTail = NULL; //指向file link的最後一個.
u32 dcfDelFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
DCF_LIST_FILEENT dcfListDelFileEnt[DCF_FILEENT_MAX];
u32 dcfListDelFileCount;
DCF_LIST_DIRENT* dcfListDelDirEnt = NULL;   //指向Dir link的第一個.

u32 global_total_Alb_file_count;
u32 dcfAlbFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
u32 dcfAlbFileCount;
u32 dcfListAlbFileCount;
DCF_LIST_FILEENT* dcfListAlbFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListAlbFileEntTail = NULL; //指向file link的最後一個.
DCF_LIST_FILEENT dcfListAlbFileEnt[DCF_PIC_FILE_PER_DIR];
struct FS_DIRENT dcfAlbFileEnt[DCF_PIC_FILE_PER_DIR];

const char *DcfFileNameTag = "";
u8  *DcfDoorFileName[DCF_MAX_CHANNEL_IDX] =
{
    "C1",
    "C2",
    "D1",
    "D2",
    "P1",
    "P2"
};

u8  dcfPlaybackDirMode;

#endif

//---used for DIR/FILE LINK
struct FS_DIRENT dcfDirEnt[DCF_DIRENT_MAX];
DCF_LIST_DIRENT dcfListDirEnt[DCF_DIRENT_MAX];

struct FS_DIRENT dcfFileEnt[DCF_FILEENT_MAX];
DCF_LIST_FILEENT dcfListFileEnt[DCF_FILEENT_MAX];

u32 dcfDirRescanSwitch;
u32 dcfDirCount;
u32 dcfFileCount;
u32 dcfListDirCount;
u32 dcfListFileCount;
u32 dcfTotalFileCntInDir;

DCF_LIST_DIRENT* dcfListDirEntHead = NULL;   //指向Dir link的第一個.
DCF_LIST_DIRENT* dcfListDirEntTail = NULL;   //指向Dir link的第一個.
DCF_LIST_FILEENT* dcfListFileEntHead = NULL; //指向file link的第一個.
DCF_LIST_FILEENT* dcfListFileEntTail = NULL; //指向file link的最後一個.
DCF_LIST_FILEENT* dcfPlaybackTempFile; //Lsk 090430 : for ROULE_DOORPHONE cycle playback
DCF_LIST_FILEENT* dcfPlaybackCamHead[DCF_MAX_MULTI_FILE] = NULL; //指向file link的第一個.
DCF_LIST_DIRENT* dcfListPlaybackDirHead = NULL;   //指向Dir link的第一個.
DCF_LIST_DIRENT* dcfListPlaybackDirTail = NULL;   //指向Dir link的第一個.

u32 dcfDirNumLast;  //表示最新開目錄的編號,下次開目錄以此為基準以流水號編織.
u32 dcfFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.

#if CDVR_iHome_LOG_SUPPORT
struct FS_DIRENT dcfLogEnt[DCF_LOGENT_MAX];
u32 dctTotalLogCount;
u32 dcfLogCount;
u32 dcfLogFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
DCF_LIST_LOGENT* dcfListLogEntHead = NULL; //指向file link的第一個.
DCF_LIST_LOGENT* dcfListLogEntTail = NULL; //指向file link的最後一個.
DCF_LIST_LOGENT* dcfListLogEntRead = NULL;
u32 dcfListLogCount;
DCF_LIST_LOGENT dcfListLogEnt[DCF_LOGENT_MAX];
DEF_LOG_INFO dcfLogFileInfo;
int dcfLogWriteBufPos=0;
int dcfLogInitReady=0;
int dcfLogRefreshFlag=0;
#endif

#if RX_SNAPSHOT_SUPPORT
int dcfPhotoInitReady=0;
#endif

DCF_LIST_DIRENT* dcfPlaybackCurDir;
DCF_LIST_DIRENT* dcfPlaybackTmpDir;
DCF_LIST_FILEENT* dcfPlaybackCurFile;
DCF_LIST_FILEENT* dcfPlaybackTmpFile;
DCF_LIST_DIRENT* dcfRecCurDir;

u8 gfileType;
u8 ucSDCInit = 1;
u8 ucSDCUnInit=0;
u8 ucNANDInit = 1;
u8 ucNORInit = 1;

DCF_FILE_INFO dcfFileInfoTab[DCF_MAX_MULTI_FILE];
//Add by albert lee

u8 dcfFileGroup=FILEENTRY_ALL;
u8 dcfFileGroupbackup=FILEENTRY_ALL;

u32 global_totalfile_count;
u32 global_totalCamfile_count[DCF_MAX_MULTI_FILE];

u32 global_totaldir_count;
u32 gFileType_count[DCF_FILE_TYPE_MAX];
u32 ChDistrInDir;   //Lucian: 用於統計目錄夾內Channel的分布, bit0 -->CH1, bit1-->CH2

DCFFILETYPE_INFO  gdcfFileType_Info[DCF_FILE_TYPE_MAX] =
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

OS_FLAG_GRP  *gDcfFlagGrp = NULL;
u8  dcfNewFile = 0;
u8  dcfFileName[MULTI_CHANNEL_MAX][32];

/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */
extern s8 dcfPlayPath[];

extern u8  osdYShift;
extern u32 sdcTryInvertSDClk;

extern u32 exifThumbnailImageHeaderSize;
extern __align(4)EXIF_THUMBNAIL_IMAGE exifThumbnailImage;
extern u32 exifPrimaryImageHeaderSize;
extern __align(4)EXIF_PRIMARY_IMAGE exifPrimaryImage;
extern __align(4)EXIF_PRIMARY_IMAGE exifAPP3VGAImage;
extern DEF_APP3PREFIX exifAPP3Prefix;

extern u32 dcfStorageType; /*CY 1023*/
extern u8 userClickFormat;
extern u32 pagecount;
extern u8 fileitem;
extern u32 playback_location;
extern u8 system_busy_flag;
extern u8  gInsertNAND;
extern u8  gInsertCard;
extern int sysStorageOnlineStat[];
extern u32 estimate_dir_nums;

extern FS_DISKFREE_T global_diskInfo;
extern u32 sys_frequency;
extern int dcfLasfEofCluster;

#if ISFILE
#if (ROOTWORK==0)
extern s8 gsParseDirName[9];
#endif
#endif

extern u8 BandWidthControl;
extern BOOLEAN MemoryFullFlag;
#if(UI_VERSION == UI_VERSION_TRANWO)
extern u8  uiCurRecType[MULTI_CHANNEL_MAX];
#endif

#if (Get_sametime == 1)
extern RTC_DATE_TIME   same_localTime;
extern u8 sametime_Pair;
extern u16 sametime_UseCnt[];
extern RTC_DATE_TIME sametime_CurrTime[];
#endif

#if CDVR_iHome_LOG_SUPPORT
extern s32 dcfLogDirFileInit(void);
extern s32 dcfLogFileOWDel(void);

#endif

#if RX_SNAPSHOT_SUPPORT
extern s32 dcfPhotoDirInit(void);
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
s32 dcfCheckUnit(void);
s32 dcfDirInit(void);
s32 dcfFileInit(DCF_LIST_DIRENT*, u8);
s32 dcfListAllFile(DCF_LIST_DIRENT*);
void dcfInsertFileNode_ToTail(DCF_FILE_INFO* File);


s32 dcfIntToStr(u32, s8*, s32);
s32 dcfCheckDir(struct FS_DIRENT*, u32*);
s32 dcfCheckFile(struct FS_DIRENT*, u32*, u8*, u8);
static s32 dcfListDirEntInsert(DCF_LIST_DIRENT*);
static s32 dcfListFileEntInsert(DCF_LIST_FILEENT*);
static s32 dcfListFileEntInsert_ToTail(DCF_LIST_FILEENT*);
s32 dcfListDelFileEntInsert(DCF_LIST_FILEENT*);
s32 dcfCreateNextDir(void);
void dcfResetFileEntrySect(int sect,int offset);
void dcfGetFileEntrySect(int *psect,int *poffset);
#if ((UI_VERSION == UI_VERSION_RDI_4) || ((UI_VERSION == UI_VERSION_TRANWO) && (OPEN_ENDUSER_SDUPGRADE == 1)))
void dcfDirRenamePa9spiu(void);
#endif


/*
 *********************************************************************************************************
 *Extern Function prototype
 *********************************************************************************************************
 */
#if RX_SNAPSHOT_SUPPORT
extern int FS_MapPhotoDir( FS_DIR *pDir,
                           struct FS_DIRENT *dst_DirEnt,
                           unsigned char* buffer,
                           unsigned int DirEntMax,
                           unsigned int *pOldestEntry,
                           int Year,
                           int Month,
                           unsigned short *pMap
                         );

extern s32 dcfPlayPhotoDirSearch(  s8* pDirName,
                                   struct FS_DIRENT *pDirEnt,
                                   s32* pDirCount,
                                   unsigned int *pOldestEntry,
                                   int DoBadFile,
                                   int Year,
                                   int Month);

#endif

extern int dcfRename(s8* pNewFileName,s8* pOldFileName);

extern  s32 dcfDirScan(  s8* pDirName,
                         struct FS_DIRENT *pDirEnt,
                         s32* pDirCount,
                         unsigned int *pOldestEntry,
                         int DoBadFile);

extern s32 dcfCreateCurPath(void);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern void smcStart(void);
#endif

FS_FILE* dcfCreateNextFile(u8 fileType, u8 index);

extern s32 dcfDrive(s8*);
extern s32 dcfFormat(s8*);
extern s32 dcfDir(s8*, struct FS_DIRENT*, u32*,u8 IsUpdateEntrySect,int DoBadFile,unsigned int MaxEntry);
extern s32 dcfMkDir(s8*);
extern s32 dcfRmDir(s8*,u8 checkflag);
extern s32 dcfChDir(s8*);
extern FS_FILE* dcfOpen(s8*, s8*);
extern s32 dcfClose(FS_FILE*, u8*);
extern s32 dcfWrite(FS_FILE*, u8*, u32, u32*);
extern s32 dcfRead(FS_FILE*, u8*, u32, u32*);
extern s32 dcfDriveInfo(FS_DISKFREE_T*);
extern s32 dcfFindLastEofCluster(u32 StorageType);

extern s32 dcfGetDirEnt(s8*, struct FS_DIRENT*, s8*);
extern s32 dcfIncDirEnt(s8* pDirName, s32 IncEntNum);


extern s32 dcfFsInit(void); /*CY 1023*/
extern s32 PowerOnuiMenuSet(s8 setting);	//civic 071002
extern u8 smcReadWriteUI(u8 , u8);	//civic 071002

//albert lee
extern void init_serial_A(void);
extern void FS__fat_ResetFileEntrySect(int sect,int offset);
extern void FS__fat_GetFileEntrySect(int *psect,int *poffset);

extern s32 sysGetDiskFree(s32);

#if ISFILE
extern s32 InitSetProgram(u8 item);
#endif


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize DCF.

Arguments:

	None.

Return Value:

	<=0 - Failure.
	1   - Success.

*/

s32 dcfInit(u32 type)
{
    u8 BitMap_return, err;
    s32 lasteofcluster;
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
#endif
    int status;
    int count;
    s32 ret;

    if (gDcfFlagGrp == NULL)
        gDcfFlagGrp = OSFlagCreate(0x00000000, &err);

    /* initialize exif file */
    exifFileInit();

    FS_Init(); /* initialize file system: create semephore */

    dcfStorageType = type;

    if (dcfFsInit() == 0)
        return 0;

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
            sdcInit();  //Initialize SD controller and create semephore.
            if (ucSDCInit)
            {
                ucSDCInit = 0;
            }
            else
            {
                sysNAND_Disable();
                //sysSPI_Disable();
                sysSD_Enable();
            }

            gInsertCard = 1;  //此時SD card 已插入
            count=0;
            do
            {
                DEBUG_DCF("===>Try SDC Mount\n");
                if(count > 0)
                    sdcTryInvertSDClk ^= 0x01;
                status=sdcMount();
                count++;
            }
            while( (status<=0) && (count<=3) );

            if ( status == -2)
            {
                DEBUG_DCF("Error: No SD Card. #1\n");
                return -2;
            }
            else if(status <= 0)
            {
                DEBUG_DCF("Error: SD/MMC mount error.\n");
                return -1;
            }

            break;

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
            if ( BitMap_return== 0)
            {
                DEBUG_DCF("Error: SMC/NAND gate flash mount error.\n");
                return -1;
            }
#endif
            break;
    }

    if ( dcfCacheClear()== 0)
    {
        DEBUG_DCF("Warning : dcfCacheClear() fail \n");
    }
    //added by Albert Lee on 20090615
    // for initial setting
    if(dcfCheckUnit()==0)
        return 0;

#if ISFILE
    InitSetProgram(1);		 // Check whether drec.inf exists ot not. If exist then update initial setting

#if ((ROOTWORK==0)&&(FILE_SYSTEM_SEL==FILE_SYSTEM_CDVR))
    if(gsParseDirName[0]!=0x00)
    {
        strcpy(gsDirName,gsParseDirName);
    }
#endif
#endif

#if CDVR_iHome_LOG_SUPPORT
    dcfLogInitReady=0;
    ret = dcfLogDirFileInit();
    if ((ret == 0)||(ret == -1))
        return ret;
    dcfLogInitReady=1;
#endif

#if RX_SNAPSHOT_SUPPORT
    dcfPhotoInitReady=0;
    ret = dcfPhotoDirInit();
    if ((ret == 0)||(ret == -1))
        return ret;

    dcfPhotoInitReady=1;
#endif

#if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
    /*Renamse pa9spiu.bin*/
    dcfDirRenamePa9spiu();
    //RDI VBM 目前不支援Video 錄影.
  #if RX_SNAPSHOT_SUPPORT
    if(dcfPhotoFileInit(0) == 0)
        return 0;
  #endif

#else
    #if ((UI_VERSION == UI_VERSION_TRANWO) && (OPEN_ENDUSER_SDUPGRADE == 1))
    /*Renamse pa9spiu.bin*/
    dcfDirRenamePa9spiu();
    #endif

    ret = dcfDirInit();

    if ((ret == 0)||(ret == -1))
        return ret;

    if (dcfFileInit(dcfListDirEntTail, CURRENTFILEENTRY) == 0)
    {
        return 0;
    }
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfFileInit(NULL, CUR_PIC_FILE_ENTRY) == 0)
    {
        return 0;
    }
#endif
#endif

    if (dcfStorageType == STORAGE_MEMORY_SMC_NAND)
    {
        if (BitMap_return==2 &&  gInsertNAND==1)    // Smc Format while making BitMap
            userClickFormat=1;
    }
    //-----Lucian:新增function, Disk初始化後,Scan FAT table 找出free cluster----//
#if FILE_SYSTEM_DVF_TEST
    time1=OSTimeGet();
#endif
    lasteofcluster=dcfFindLastEofCluster(dcfStorageType);
#if FILE_SYSTEM_DVF_TEST
    time2=OSTimeGet();
    DEBUG_DCF("--->dcfFindLastEofCluster Time=%d\n",time2-time1);
#endif
    if(lasteofcluster>0)
    {
        DEBUG_DCF("Lasf EOF Cluster=%d\n",lasteofcluster);
    }
    else
    {
        DEBUG_DCF("Find free cluster is FAiL\n");
    }
    //------------//


    return 1;
}


#if CDVR_iHome_LOG_SUPPORT

void dcfLogTest(u8 *cmd)
{
    int Nday,Nblock,Rblock,Tblock;
    int ReadSize;

    DEBUG_UI("dcfLogTest: %s\n", cmd);
    if(!strncmp((char*)cmd,"INFO ", strlen("INFO ")))
    {
        sscanf((char*)cmd, "INFO %d", &Nday);
        Tblock=dcfGetLogFileInfo(Nday);
        DEBUG_DCF("Tblock=%d\n",Tblock);
    }
    else if(!strncmp((char*)cmd,"READ ", strlen("READ ")))
    {
        sscanf((char*)cmd, "READ %d %d", &Nday,&Nblock);
        ReadSize=dcfReadLogFile(Nday,Nblock,&Rblock,&Tblock);
        DEBUG_DCF("ReadSize=%d,Rblock=%d,Tblock=%d\n",ReadSize,Rblock,Tblock);
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

s32 dcfCheckLogFile(struct FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j,value;
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
    static int First=1;

    //--------------------//
    DEBUG_DCF("dcfLogDirInit: Begin \n");
    dcfLogFileNumLast = 0; /* 0001 - 1 */
    dcfListLogEntTail = dcfListLogEntHead =NULL;
    memset_hw((void*)dcfLogEnt, 0, sizeof(struct FS_DIRENT) * DCF_LOGENT_MAX);
    if(First)
    {
        memset(dcfLogBuf_Wr,0,DCF_LOGBUF_SIZE);
        First=0;
    }

    sprintf ((char*)DirName, "\\%s", gsLogDirName);

    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_LOGENT_MAX);

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
    DEBUG_DCF("--->dcfDir Log complete:%d.\n",dcfLogCount);

    // sort the directory and create the directory list
    memset_hw((void*)dcfListLogEnt, 0, sizeof(DCF_LIST_LOGENT) * DCF_LOGENT_MAX);
    dcfListLogCount = 0;
    dctTotalLogCount=0;

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
    DEBUG_DCF("--->dcfListLogCount=%d,dctTotalLogCount=%d\n",dcfListLogCount,dctTotalLogCount);


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
        dcfListDirEntTemp = dcfListLogEntHead;
        if (dcfListLogEntHead == NULL)
            return 1;

        do
        {
            DEBUG_DCF("Log Name: %s\n", dcfListDirEntTemp->pDirEnt->d_name);
            dcfListDirEntTemp = dcfListDirEntTemp->next;

        }
        while(dcfListDirEntTemp != dcfListLogEntHead );
    }
#endif

    return 1;
}


s32 dcfLogFileOWDel(void)
{
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;

    //---------------------------------------------//

    DEBUG_DCF("Enter dcfLogFileDel....\n");

    if (dcfListLogEntHead == NULL)
    {
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }

    if(dcfLogDel((s8*)dcfListLogEntHead->pDirEnt->d_name,dcfListLogEntHead->pDirEnt->FileEntrySect) <= 0)
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

    DEBUG_DCF("Leave dcfLogFileDel\n");
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
    int temp;
    int Temp_FileEntSect;
    int Temp_FileEntSect_offset;


    u8	tmp;
    //-------------------------//

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    dcfGetFileEntrySect(&Temp_FileEntSect,&Temp_FileEntSect_offset);
    if( (dcfListLogEntTail==NULL) && (dcfLogInitReady==1) )
    {
        DEBUG_DCF("Create Empty Log file\n");
        dcfResetFileEntrySect(0,0);
        if((pFile = dcfCreateNextLog())==NULL)
        {
            DEBUG_DCF("===dcfWriteLogFile Error!!===\n");
        }
        dcfClose(pFile, &tmp);
        dcfResetFileEntrySect(0,0);
    }

    RTC_Get_Time(&localTime);
    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    if ( (dcfListLogEntTail != NULL) && (dcfLogInitReady==1) && (dcfListLogEntTail->pDirEnt->fsFileCreateDate_YMD !=val) )
    {
        DEBUG_DCF("===Day change,Split to new log file===\n");
        dcfResetFileEntrySect(0,0);  //Lucian:讓 Estimate file etnry歸零 .

        strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);
        strcpy((char*)TargetName, "\\");
        strcat((char*)TargetName, (char*)gsLogDirName);
        strcat((char*)TargetName, "\\");
        strcat((char*)TargetName, LogName);

        DEBUG_DCF("Append old Log file:%s\n",TargetName);

        temp=dcfLasfEofCluster;
        dcfResetFileEntrySect(dcfListLogEntTail->pDirEnt->FileEntrySect,0);
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);

            memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
            dcfLogWriteBufPos += size;
            if(dcfLogWriteBufPos > DCF_LOGBUF_SIZE)
                dcfLogWriteBufPos= DCF_LOGBUF_SIZE;
            dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        if(dcfLogRefreshFlag)
            dcfSeek(pFile, -DCF_LOGBUF_SIZE, FS_SEEK_CUR);

        dcfWrite(pFile, dcfLogBuf_Wr, dcfLogWriteBufPos, &writesize);
        dcfClose(pFile, &tmp);
        //DEBUG_DCF("dcfLasfEofCluster=%d\n",dcfLasfEofCluster[0]);

        if((pFile = dcfCreateNextLog())==NULL)
        {
            DEBUG_DCF("===dcfWriteLogFile Error!!===\n");
        }
        dcfClose(pFile, &tmp);
        dcfLasfEofCluster=temp;

        dcfResetFileEntrySect(0,0);  //Lucian:讓 Estimate file etnry歸零 .

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
        DEBUG_DCF("### dcfLogDirFileInit is not ready,Drop this event!###\n");
    }
    else
    {
        dcfResetFileEntrySect(0,0);  //Lucian:讓 Estimate file etnry歸零 .

        strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);

        strcpy((char*)TargetName, "\\");
        strcat((char*)TargetName, (char*)gsLogDirName);
        strcat((char*)TargetName, "\\");
        strcat((char*)TargetName, LogName);

        DEBUG_DCF("Append old Log file:%s\n",TargetName);
        temp=dcfLasfEofCluster;
        dcfResetFileEntrySect(dcfListLogEntTail->pDirEnt->FileEntrySect,0);
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            memcpy(dcfLogBuf_Wr+dcfLogWriteBufPos,event,size);
            dcfLogWriteBufPos += size;
            if(dcfLogWriteBufPos > DCF_LOGBUF_SIZE)
                dcfLogWriteBufPos= DCF_LOGBUF_SIZE;
            dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }

        if(dcfLogRefreshFlag)
            dcfSeek(pFile, -DCF_LOGBUF_SIZE, FS_SEEK_CUR);

        dcfWrite(pFile, dcfLogBuf_Wr, dcfLogWriteBufPos, &writesize);
        dcfClose(pFile, &tmp);
        dcfResetFileEntrySect(0,0); //Lucian:讓 Estimate file etnry歸零 .
        dcfLasfEofCluster=temp;
        //DEBUG_DCF("dcfLasfEofCluster=%d\n",dcfLasfEofCluster[0]);

        DEBUG_DCF("Log file position:%d\n",pFile->filepos);

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
    dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
    OSSemPost(dcfReadySemEvt);

    return 1;
}


int dcfLogFileSaveRefresh()
{
    RTC_DATE_TIME   localTime;
    unsigned short val;
    INT8U   err;
    FS_FILE* pFile;
    u32 writesize;
    char LogName[32],TargetName[64];
    int temp;
    u8 	tmp;
    int Temp_FileEntSect;
    int Temp_FileEntSect_offset;

    //-------------------------//

    if(dcfLogWriteBufPos==0)
        return;

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    dcfGetFileEntrySect(&Temp_FileEntSect,&Temp_FileEntSect_offset);

    if( (dcfListLogEntTail==NULL) || (dcfLogInitReady==0) )
    {
        dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }


    strcpy(LogName, dcfListLogEntTail->pDirEnt->d_name);

    strcpy((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)gsLogDirName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, LogName);

    DEBUG_DCF("-->dcfLogFileSaveRefresh Refresh old Log file:%s\n",TargetName);
    temp=dcfLasfEofCluster;
    dcfResetFileEntrySect(dcfListLogEntTail->pDirEnt->FileEntrySect,0);

    if(dcfLogRefreshFlag)
    {
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
        dcfSeek(pFile, -DCF_LOGBUF_SIZE, FS_SEEK_CUR);

        dcfWrite(pFile, dcfLogBuf_Wr, DCF_LOGBUF_SIZE, &writesize);
        dcfClose(pFile, &tmp);
    }
    else
    {
        //dcfResetFileEntrySect(0,0);  //Lucian:讓 Estimate file etnry歸零 .
        if ((pFile = dcfOpen((s8 *)TargetName, "a")) == NULL)
        {
            /* create next file error */
            DEBUG_DCF("Error: create old Log %s error\n", TargetName);
            dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
        dcfWrite(pFile, dcfLogBuf_Wr, DCF_LOGBUF_SIZE, &writesize);
        dcfClose(pFile, &tmp);
    }

    dcfResetFileEntrySect(0,0); //Lucian:讓 Estimate file etnry歸零 .
    DEBUG_DCF("Log file position:%d\n",pFile->filepos);
    dcfLasfEofCluster=temp;
    dcfLogRefreshFlag=1;
    //dcfCacheClean();
    dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
    OSSemPost(dcfReadySemEvt);

    return 1;
}

FS_FILE* dcfCreateNextLog()
{
    char newLogName[32], newTargetName[64];
    u32 LastdirNum;
    int scan;
    s8	DirName[32];
    RTC_DATE_TIME   localTime;
    INT8U   err;
    int count;
    int datecode;
    unsigned short val;
    FS_FILE* pFile;
    //==============================================//

    DEBUG_DCF("\nEnter dcfCreateNextLog\n");

    dcfLogFileNumLast++;
    dcfIntToStr(dcfLogFileNumLast, newLogName, 8);
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
    DEBUG_DCF("Log name: %s \n",newTargetName);

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

    DEBUG_DCF("Log Name: %s\n",dcfListLogEntTail->pDirEnt->d_name);

    dctTotalLogCount ++;

    DEBUG_DCF("Leave dcfCreateNextLog\n\n");

    return pFile;
}
//-------------------------------------------//
int dcfGetLogFileInfo(int Nday)
{
    FS_FILE* pFile;
    int Tblock;
    u8	tmp;

    pFile=dcfOpenLogFile(Nday);
    dcfClose(pFile, &tmp);

    strcpy(dcfLogFileInfo.FileName,dcfListLogEntRead->pDirEnt->d_name);

    dcfLogFileInfo.DateTime.year=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD >> 9)+1980;
    dcfLogFileInfo.DateTime.month=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;
    dcfLogFileInfo.DateTime.day=(dcfListLogEntRead->pDirEnt->fsFileCreateDate_YMD & 0x001F);

    dcfLogFileInfo.DateTime.hour=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS>>11);
    dcfLogFileInfo.DateTime.min=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5;
    dcfLogFileInfo.DateTime.sec=(dcfListLogEntRead->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1;

    dcfLogFileInfo.FileSize=pFile->size;
    /*
        DEBUG_DCF("\ndcfGetLogFileInfo:%s %04d/%02d/%02d %02d:%02d:%02d FileSize=%d\n",
                      dcfLogFileInfo.FileName,
                      dcfLogFileInfo.DateTime.year,
                      dcfLogFileInfo.DateTime.month,
                      dcfLogFileInfo.DateTime.day,
                      dcfLogFileInfo.DateTime.hour,
                      dcfLogFileInfo.DateTime.min,
                      dcfLogFileInfo.DateTime.sec,
                      dcfLogFileInfo.FileSize

        );
    */
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
    u32 ReadSize,size,shift;
    INT8U   err;
    int filepos;
    u8 tmp;

    //----------------------//
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    filepos=0;
    ReadSize=0;
    pFile=dcfOpenLogFile(Nday);
    strcpy(dcfLogFileInfo.FileName,dcfListLogEntRead->pDirEnt->d_name);

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

    //DEBUG_DCF("\dcfReadLogFile: pos=%d,rdsz=%d,fsz=%d,Block=%d,isTail=%d\n",filepos,ReadSize,pFile->size,Nblock,(dcfListLogEntRead==dcfListLogEntTail));

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


    //DEBUG_DCF("\nEnter dcfOpenLogFile_Nday\n");
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
    //DEBUG_DCF("Open Log name: %s \n",newTargetName);
    //DEBUG_DCF("Leave dcfOpenLogFile\n\n");

    return pFile;
}

int dcfLogFileUsrDel(int Nday)
{
    int ShiftNum;
    int i;
    char newLogName[32], newTargetName[64];
    DCF_LIST_LOGENT* EntDel;


    DEBUG_DCF("\nEnter dcfLogFileUsrDel N day\n");

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

    if(dcfLogDel((s8*)EntDel->pDirEnt->d_name,EntDel->pDirEnt->FileEntrySect) <= 0)
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

    DEBUG_DCF("Leave dcfLogFileUsrDel\n\n");

    return 1;
}
#endif


#if RX_SNAPSHOT_SUPPORT

s32 dcfPhotoDirInit(void)
{
    u32 PhotoNum;
    u32 i;
    s8  DirName[32];
    u32 dirNum=0;

    //--------------------//
    DEBUG_DCF("dcfPhotoDirInit: Begin \n");

    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_DIRENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH1");
    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_FILEENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH2");
    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_FILEENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH3");
    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_FILEENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }
    //-----------------------------------------//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    strcat((char*)DirName, "\\CH4");
    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        dcfIncDirEnt((s8 *)DirName,DCF_FILEENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }
    //===========================================//
    sprintf ((char*)DirName, "\\%s", gsPhotoDirName);
    if (dcfDir(DirName, dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory %s failed.\n",DirName);
        //return 0;
    }

    // sort the directory and create the directory list
    memset_hw((void*)dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    dcfListDirCount = 0;
    // List the valid directory and make the linked list
    for (i = 0; i < dcfDirCount; i++)
    {
        if (dcfDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            if(dcfListDirCount>=DCF_DIRENT_MAX)
            {
                break;
            }
            // insert directory entry to the list
            dcfListDirEnt[dcfListDirCount].used_flag = DCF_FILE_USE_CLOSE;
            dcfListDirEnt[dcfListDirCount].dirNum = dirNum;
            dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[i];
            dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
            dcfListDirCount++;
            //Add on 20091208, for the playback of CDVR File system
            global_totaldir_count ++;
        }
    }


    if(dcfListDirEntHead != NULL)
    {
        /* set tail entry of directory list */
        dcfListDirEntTail = dcfListDirEntHead->prev;

        /* set current playback directory */
        dcfPlaybackCurDir = dcfListDirEntTail;
    }

    DEBUG_DCF("--->dcfPhotoDirInit complete:%d.\n",dcfDirCount);

    return 1;
}


int dcfWriteNextPhotoFile(int RFUnit,u8 *buffer,int size)
{
    RTC_DATE_TIME   localTime;
    INT8U   err;
    FS_FILE* pFile;
    u32 writesize;
    char PhotoName[32],TargetName[64],ChannelName[32];
    int temp;
    u8  tmp;
    FS_DISKFREE_T   *diskInfo;
    u32             free_size;
    u32             bytes_per_cluster;
    char MonthName[12][4]=
    {
        {'J','A','N','\0'},
        {'F','E','B','\0'},
        {'M','A','R','\0'},
        {'A','P','R','\0'},
        {'M','A','Y','\0'},
        {'J','U','N','\0'},
        {'J','U','L','\0'},
        {'A','U','G','\0'},
        {'S','E','P','\0'},
        {'O','C','T','\0'},
        {'N','O','V','\0'},
        {'D','E','C','\0'},
    };
    int Temp_FileEntSect;
    int Temp_FileEntSect_offset;
    //-------------------------//
    if( dcfPhotoInitReady==0)
    {
        DEBUG_DCF("===Error,Photo file system not Ready!===\n");
        return 0;
    }

    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit

    if(free_size <= 300) //Notice: K-Byte unit
    {
        DEBUG_DCF("Disk Full\n");
        MemoryFullFlag = TRUE;
        return 0;
    }

    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    dcfGetFileEntrySect(&Temp_FileEntSect,&Temp_FileEntSect_offset);
    DEBUG_DCF("--dcfWriteNextPhotoFile--\n");

    dcfResetFileEntrySect(0,0);
    RTC_Get_Time(&localTime);
    sprintf(PhotoName,"%s%02d-%02d",MonthName[localTime.month-1],localTime.day, localTime.year % 100);
    strcat((char*)PhotoName, ".JPG");
    sprintf(ChannelName,"CH%d",(RFUnit+1));
    strcpy((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)gsPhotoDirName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, (char*)ChannelName);
    strcat((char*)TargetName, "\\");
    strcat((char*)TargetName, PhotoName);
    if ((pFile = dcfOpen((s8 *)TargetName, "w")) == NULL)
    {
        /* create next file error */
        DEBUG_DCF("Error: create photo %s error\n", TargetName);
        dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }
    dcfWrite(pFile, buffer, size, &writesize);
    dcfClose(pFile, &tmp);
    DEBUG_DCF("dcfLasfEofCluster=%d,EOF=%d\n",dcfLasfEofCluster,pFile->EOFClust);
    dcfResetFileEntrySect(0,0);
    //-------------------------------------//
    dcfResetFileEntrySect(Temp_FileEntSect,Temp_FileEntSect_offset);
    OSSemPost(dcfReadySemEvt);

    return 1;
}


s32 dcfPhotoFileInit(int CHsel)
{
    u32 i;
    u32 fileNum;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;
    s8	DirName[32];
    //----------------------------//
    DEBUG_DCF("----dcfPhotoFileInit Begin:%d----\n",CHsel);

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfListReadFileEntHead = NULL;
    dcfListReadFileEntTail = NULL;
    dcfPlaybackCurFile = NULL;

    if (dcfPhotoInitReady == 0)
    {
        DEBUG_DCF("===Error,Photo file system not Ready!===\n");
        return 0;
    }

    dcfReadFileCount=0;
    memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX);
    sprintf ((char*)DirName, "\\%s\\CH%d", gsPhotoDirName,CHsel+1);
    if (dcfChPlayDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

        return 0;
    }

    /* list current directory */
    if (dcfPlayDirScan(NULL, dcfReadFileEnt, &dcfReadFileCount,&OldestEntry,0) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");
        return 0;
    }

    /* sort the file and create the file list */
    memset_hw((void*)dcfListReadFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

    dcfListReadFileCount = dcfReadFileCount;

    if(dcfReadFileCount==0)
    {
        dcfListReadFileEntHead=NULL;
    }
    else if(dcfReadFileCount==1)
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[0];
        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[0];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];
    }
    else
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[OldestEntry];

        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->next = &dcfListReadFileEnt[1];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];

        ins ++;

        for (i = 1; i < dcfReadFileCount-1; i++)
        {
            ins = &dcfListReadFileEnt[i];
            ins->prev = &dcfListReadFileEnt[i-1];
            ins->next = &dcfListReadFileEnt[i+1];

            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[i];
            ins ++;
        }

        ins = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-2];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[dcfReadFileCount-1];
    }


    /* set tail entry of file list */
    if( dcfListReadFileEntHead != NULL)
    {
        dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
        dcfListReadFileEntTail->next =dcfListReadFileEntHead;
    }
    else
    {
        dcfListReadFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile = dcfListReadFileEntTail;
    DEBUG_DCF("----dcfPhotoFileInit End:%d----\n",dcfReadFileCount);

    return 1;


}
s32 dcfGetStorageSizeInfo(void)
{
    FS_DISKFREE_T *pInfo;
    FS__FAT_BPB bpb;
    u32 val = 0, unit = 0, reservSpace;
    u32 DevIdx, avaClust, totClust;

    // if the device donesn't exist
    if(DevIdx = DCF_GetDeviceIndex("sdmmc") == -1)
        return -1;

    pInfo = &global_diskInfo;
    // It spend much time to load the Voulme Info
    //if(!_FS_fat_GetDiskVolumeInfo(pInfo, &dcfCurDrive))
    //return -1;

    // special condition for SD card
    if (pInfo->total_clusters  == 0)
        return -1;
    // u32 is not enough to support 32K of one cluster in 16G SD card.
    // I will set one percent by 6M that would allow calculate in 2TB SD card.
    // 6144K / 32K = 192
    avaClust = pInfo->avail_clusters / 192;
    totClust = pInfo->total_clusters / 192;

    val = (avaClust * 10000) / totClust;
    if(val <= 100)
    {
        if(!(_FS_fat_GetDiskBPBInfo(&bpb, DevIdx, unit)))
            return -1;

        if(bpb.FATSz16)
            reservSpace = bpb.FATSz16;
        else
            reservSpace = bpb.FATSz32;
        //reservSpace = bpb.RsvdSecCnt + reservSpace * 2;	// Get disk free had count already
        reservSpace += 2000;	// Another reserved place, 1000K
        if(pInfo->avail_clusters * 64 >= reservSpace)
            return val;	// 1%
        else
            return 0;	// 0%
    }
    else
        return val;
}
#endif





/*

Routine Description:

	Uninitialize DCF.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/

s32 dcfUninit(void)
{
    /*CY 1023*/
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
    }

    FS_Exit(); /* exit file system */
    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    dcfListDirEntTail = dcfListDirEntHead=dcfPlaybackCurDir=NULL;
    dcfListFileEntTail = dcfListFileEntHead=dcfPlaybackCurFile=NULL;

    return 1;
}


/*

Routine Description:

	Insert directory entry.

Arguments:

	ins - Directory entry to be inserted.
	會按照流水號排序

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListDirEntInsert(DCF_LIST_DIRENT* ins)
{
    DCF_LIST_DIRENT* cur;

    if (dcfListDirCount == 0)
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
            if (ins->dirNum < cur->dirNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListDirEntHead)
                {
                    /* insert before head */
                    dcfListDirEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListDirEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}


/*

Routine Description:

	Insert file entry.(forward)

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListFileEntInsert(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListFileEntHead = ins;
    }
    else
    {
        cur = dcfListFileEntHead;
        do
        {
            if (ins->fileNum < cur->fileNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListFileEntHead)
                {
                    /* insert before head */
                    dcfListFileEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListFileEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}



/*

Routine Description:

	Insert file entry.(backward)

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListFileEntInsert_ToTail(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListFileEntHead = ins;
    }
    else
    {
        //always insert to tail
        cur = dcfListFileEntHead->prev;
        cur->next = ins;
        ins->next = dcfListFileEntHead;
        dcfListFileEntHead->prev = ins;
        ins->prev = cur;
    }

    return 1;
}





s32 dcfCheckUnit(void)
{
    DEBUG_DCF("--Do DcfCheckUnit--\n");
    if (FS_IoCtl(dcfCurDrive,FS_CMD_CHECK_UNIT,0,0) != 0)
    {
        DEBUG_DCF("Error: Check %s failed\n", dcfCurDrive);
        return 0;
    }
    DEBUG_DCF("-->Check %s OK\n", dcfCurDrive);
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
s32 dcfPlaybackFormat(void)
{
    int ret;

    if (dcfFormat(dcfCurDrive) == 0)
    {
        OSMboxPost(general_MboxEvt, "FAIL");
        return 0;
    }

    dcfNewFile = 0;
    // It must be cleared before dir and file init, or it will disturb the FAT1 table
    dcfLasfEofCluster=-1;

#if CDVR_iHome_LOG_SUPPORT
    dcfLogInitReady=0;
    dcfLogRefreshFlag=0;
    dcfLogWriteBufPos=0;
    ret = dcfLogDirFileInit();
    if ((ret == 0)||(ret == -1))
        return ret;
    dcfLogInitReady=1;
#endif

#if RX_SNAPSHOT_SUPPORT
    dcfPhotoInitReady=0;
    ret = dcfPhotoDirInit();
    if ((ret == 0)||(ret == -1))
        return ret;

    dcfPhotoInitReady=1;
#endif


    /* directory initialization */
    if (dcfDirInit() == 0)
        return 0;

    /* file initialization */
#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    if (dcfFileInit(dcfListDirEntHead,CURRENTFILEENTRY) == 0)
        return 0;
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DSC)
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
        return 0;
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
        return 0;
    if (dcfFileInit(dcfListDirEntTail,CUR_PIC_FILE_ENTRY) == 0)
        return 0;
#if 0
    if (dcfFileInit(dcfListDirEntTail,CUR_ALB_FILE_ENTRY) == 0)
        return 0;
#endif
#endif

    // After format, DCIM system would be ready to record
    gSystemStroageReady = 1;
    sys_format = 1;
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
    unsigned char i;

    for(i=0; i<DCF_FILE_TYPE_MAX; i++)
    {
        if (!strncmp(pcsubfilename, gdcfFileType_Info[i].pSubFileStr, gdcfFileType_Info[i].uiStrLength))
        {
            break;
        }
    }

    //DEBUG_DCF("TRACE: file type =%s, %d\n",pcsubfilename, i);
    return i;


}

u8 dcfCheckFileChannel(char CHmap,char CHch)
{
    switch(CHch)
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

void dcfResetFileEntrySect(int sect,int offset)
{
    FS__fat_ResetFileEntrySect(sect,offset);
}

void dcfGetFileEntrySect(int *psect,int *poffset)
{
    FS__fat_GetFileEntrySect(psect,poffset);
}

//*******************************************************************************
//	DCF for FILE_SYSTEM_DVR and FILE_SYSTEM_DSC
//*******************************************************************************
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DSC)

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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFileNext....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    //DEBUG_DCF("dcfPlaybackFileNext: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No next file.\n");
        return 0;
    }

    if (dcfPlaybackCurFile->next == dcfListFileEntHead)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;

        if (dcfPlaybackDirBackward() == 0)
        {
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Trace: Change Dir Fail\n");
            return 0;
        }
    }
    else
    {
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }

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
    //DEBUG_DCF("dcfPlaybackFilePrev: begin ,dcfListDirCount=%d\n",dcfListDirCount);

    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFilePrev....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No previous file.\n");
        return 0;
    }

    if (dcfPlaybackCurFile == dcfListFileEntHead)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
        if (dcfPlaybackDirForward() == 0)
        {
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Trace: Change Dir Fail\n");
            return 0;
        }
    }
    else
    {
        dcfPlaybackCurFile = dcfPlaybackCurFile->prev;
    }
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFilePrev\n");
    return 1;
}

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
    u32 i;

    DEBUG_DCF("dcfPlaybackDirForward: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    /* forward traverse whole directories */
    for (i = 0; i < dcfListDirCount; i++)
    {
        /* file initialization */
        if (dcfFileInit(dcfPlaybackCurDir, 0))
        {
            /* check if file of list head is null */
            if (dcfListFileEntHead)
            {
                /* set current file */
                dcfPlaybackCurFile = dcfListFileEntHead;
                return 1;
            }
        }

        /* next directory */
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
    }

    DEBUG_DCF("Trace: No current file.\n");
    dcfPlaybackCurFile = NULL;

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
    u32 i;

    DEBUG_DCF("dcfPlaybackDirBackward: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    /* backward traverse whole directories */
    for (i = 0; i < dcfListDirCount; i++)
    {
        /* file initialization */
        if (dcfFileInit(dcfPlaybackCurDir, 0))
        {
            /* check if file of list head is null */
            if (dcfListFileEntHead)
            {
                /* set current file */
                dcfPlaybackCurFile = dcfListFileEntHead->prev;
                return 1;
            }
        }

        /* previous directory */
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
    }

    DEBUG_DCF("Trace: No current file.\n");
    dcfPlaybackCurFile = NULL;

    return 0;
}

/*

Routine Description:

	The directory initialization routine of DCF.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfDirInit(void)
{
    u32 i;
    u32 dirNum;

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count = 0;

    dcfDirNumLast = 99; /* 100 - 1; Lucian: 目錄從100XXXX 開始算起 */

    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir("\\DCIM") == 0)
    {
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        /* change directory \DCIM error */
        if (dcfMkDir("\\DCIM") == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory \\DCIM failed.\n");

            return -1;
        }

        if (dcfChDir("\\DCIM") == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory \\DCIM failed.\n");

            return -1;
        }
    }

    /* list directory \DCIM */
    // List all the directory which in DCIM civic
    if (dcfDir("\\DCIM", dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory \\DCIM failed.\n");

        return 0;
    }

    /* sort the directory and create the directory list */
    memset_hw((void*)dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    dcfListDirCount = 0;
    // List the valid directory and make the linked list
    for (i = 0; i < dcfDirCount; i++)
    {
        if (dcfDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            /* check if valid dcf directory */
            if (dcfCheckDir(&dcfDirEnt[i], &dirNum) == 0)
                continue;

            /* assign last directory number */
            dcfDirNumLast = (dcfDirNumLast < dirNum) ? dirNum : dcfDirNumLast;

            /* insert directory entry to the list */
            dcfListDirEnt[dcfListDirCount].dirNum = dirNum;
            dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[i];
            dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
            dcfListDirCount++;
            //Add on 20091208, for the playback of CDVR File system
            global_totaldir_count ++;
        }
    }

    /* create a subdirectory if none is found */
    if (dcfListDirCount == 0)
    {
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
        //Lucian: Only one Sub-directory
        for (i=0; i<estimate_dir_nums-1; i++)
        {
            if (dcfCreateNextDir() == 0)
            {
                /* create directory failed */
                DEBUG_DCF("Error: Create next direcotry failed.\n");

                return 0;
            }
        }
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DSC)
        if (dcfCreateNextDir() == 0)
        {
            /* create directory failed */
            DEBUG_DCF("Error: Create next direcotry failed.\n");

            return 0;
        }
#endif
    }

    /* set tail entry of directory list */
    dcfListDirEntTail = dcfListDirEntHead->prev;

    /* set current playback directory */
    dcfPlaybackCurDir = dcfListDirEntHead;

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
s32 dcfFileInit(DCF_LIST_DIRENT* pListDirEnt, u8 Type)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
//    DCF_LIST_DIRENT* temp_head=dcfListDirEntHead;
//    u32 local_dcfFileCount;
//    struct FS_DIRENT* local_dcfFileEnt = &dcfFileEnt[0];
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

    DEBUG_DCF("dcfFileInit: Begin ,Type=%d\n",Type);
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 1 =%d (x50ms)\n",time1);
#endif

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;
    dcfResetFileEntrySect(0,0);
    /* set tail entry of file list */

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfFileNumLast = 0; /* 0001 - 1 */
    dcfListFileEntHead = NULL;

    dcfFileCount=0; //civicwu 070926
    memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926
    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir("\\DCIM") == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory \\DCIM failed.\n");

        return 0;
    }

    /* change current directory */
    if (dcfChDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
    {
        /* change current directory error */
        DEBUG_DCF("Error: Change current directory failed.\n");

        return 0;
    }

    /* list current directory */
    if (dcfDir(NULL, dcfFileEnt, &dcfFileCount,1,1,DCF_FILEENT_MAX) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");

        return 0;
    }
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 2 =%d (x50ms)\n",time1);
#endif
    /* sort the file and create the file list */
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    dcfListFileCount = 0;

    for (i = 0; i < dcfFileCount; i++)
    {
        if ((dcfFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
        {
            /* check if valid dcf file */
#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB0) ||(FILE_SYSTEM_SEL == FILE_SYSTEM_DSC) )
            if (dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                continue;
#elif ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
            if (dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                continue;
#endif

#if FILE_REPAIR_AUTO //Repair Bad file
            if(fileType == 6) // AXF file
            {
                DEBUG_SYS("Repair Bad File:%s\n",dcfFileEnt[i].d_name);
                uiClearOSDBuf(2);
                uiMenuOSDStringByColor(TVOSD_SizeX , "Repair Bad File:" , OSD_STRING_W , OSD_STRING_H , 48, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                uiMenuOSDStringByColor(TVOSD_SizeX , dcfFileEnt[i].d_name , OSD_STRING_W , OSD_STRING_H , 176, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                asfRepairFile(dcfFileEnt[i].d_name);
                DEBUG_SYS("===Repair Complete===\n");

                memcpy(NewName,dcfFileEnt[i].d_name,13);
                NewName[10]='S';
                if(0 == dcfRename(NewName,dcfFileEnt[i].d_name))
                {
                    DEBUG_DCF("Warning! DCF Rename Fail!\n");
                }
                fileType=0;
                dcfFileEnt[i].d_name[10]='S';
            }
#endif
            dcfFileTypeCount_Inc(fileType);
            global_totalfile_count++;

            /* assign last file number */
            dcfFileNumLast = (dcfFileNumLast < fileNum) ? fileNum : dcfFileNumLast;

            /* insert file entry to the list */
            dcfListFileEnt[dcfListFileCount].fileNum = fileNum;
            dcfListFileEnt[dcfListFileCount].fileType = fileType;
            dcfListFileEnt[dcfListFileCount].pDirEnt = &dcfFileEnt[i];
            dcfListFileEntInsert(&dcfListFileEnt[dcfListFileCount]);
            dcfListFileCount++;
        }

        sysDeadLockMonitor_Reset();
    }

    /* set tail entry of file list */
    if( dcfListFileEntHead != NULL)
    {
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfListFileEntTail->next =dcfListFileEntHead;
    }
    else
    {
        dcfListFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile=dcfListFileEntHead->prev;

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 3 =%d (x50ms)\n",time1);
#endif

    return 1;
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
s32 dcfCheckDir(struct FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j;
    s8 dirNumStr[4];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".   ") != 0)
        return 0;

    /* get dcf directory number string */
    for (j = 0; j < 3; j++)
        dirNumStr[j] = pDirEnt->d_name[j];
    dirNumStr[3] = '\0';

    /* dcf directory number is from 100 to 999 */
    *pDirNum = (u32)atoi((const char*)dirNumStr);
    if ((*pDirNum < 100) || (*pDirNum > 999))
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
s32 dcfCheckFile(struct FS_DIRENT* pFileEnt, u32* pFileNum, u8* pFileType, u8 FileGroup)
{
    u32 j;
    u8 fileNumStr[5];

    //check sub file name
    for (j = 0; j < 3; j++)
        fileNumStr[j] = pFileEnt->d_name[j+9];
    fileNumStr[j] = '\0';

    if ((*pFileType=dcfGetFileType((char *)fileNumStr))==DCF_FILE_TYPE_MAX)
        return 0;

    /* get dcf file number string */
    for (j = 0; j < 4; j++)
        fileNumStr[j] = pFileEnt->d_name[j + 4];
    fileNumStr[4] = '\0';

    /* dcf file number is from 0001 to 9999 */
    *pFileNum = (u32)atoi((const char*)fileNumStr);
    if ((*pFileNum < 0001) || (*pFileNum > 9999))
        return 0;

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
s32 dcfIntToStr(u32 val, s8* str, s32 cnt)
{
    s8 i;
    s8 dig[5];

    for (i = 0; i < 4; i++)
    {
        dig[i] = dcfDecChar[val % 10];
        val /= 10;
    }

    for (i = (cnt - 1); i >= 0; i--)
        *str++ = dig[i];

    *str = '\0';

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
s32 dcfCreateNextDir(void)
{
    s8 newDirName[32];
    s8 newTargetName[64];
    u32 count = 900;

    DEBUG_DCF("Enter DSC dcfCreateNextDir\n");
    do
    {
        dcfDirNumLast++;
        if (dcfDirNumLast > 999)
            dcfDirNumLast = 100;

        dcfIntToStr(dcfDirNumLast, newDirName, 3);
        strcat((char*)newDirName, DCF_DIR_SUFFIX);
        strcat((char*)newDirName, ".   ");
        strcpy((char*)newTargetName, "\\DCIM\\");
        strcat((char*)newTargetName, (const char*)newDirName);
        if (dcfMkDir(newTargetName))
        {
            /* make next directory ok */
            break;
        }
    }
    while (count--);

    if (count == 0)
    {
        /* none valid directory name is found */
        DEBUG_DCF("dcfCreateNextDir no directory\n");
        return 0;
    }

    /* list directory newly created */
    if (dcfGetDirEnt("\\DCIM", &dcfDirEnt[dcfDirCount], newDirName) == 0)
    {
        /* list directory error */
        DEBUG_DCF("Error: Find directory entry of %s failed.\n", newDirName);
        return 0;
    }
//#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
    //Lucian: 增加direct FDB entry. 增加至4096 entry.
    dcfIncDirEnt(newTargetName,DCF_FILEENT_MAX);
//#endif
    /* insert directory entry to the list */
    dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[dcfDirCount];
    dcfListDirEnt[dcfListDirCount].dirNum = dcfDirNumLast;
    dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
    dcfDirCount++;

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count ++;

    dcfListDirEntTail = &dcfListDirEnt[dcfListDirCount++];
    DEBUG_DCF("Leave DSC dcfCreateNextDir\n");

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

FS_FILE* dcfCreateNextFile(u8 fileType, u8 index)
{
    s8 newFileName[32];
    FS_FILE* pFile;
    INT8U   err;

    DEBUG_DCF("Enter DSC dcfCreateNextFile\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (index >= DCF_MAX_MULTI_FILE)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("dcfCreateNextFile Error Index %d\n",index);
        return 0;
    }

    dcfFileNumLast++;
    //----若檔案目錄大於DCF_FILE_PER_DIR,則開下一個目錄-----//
    if (dcfFileNumLast > DCF_FILE_PER_DIR)
    {
        /* make next directory */
        if (dcfCreateNextDir() == 0)
        {
            /* create directory failed */
            //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
            OSSemPost(dcfReadySemEvt);
            DEBUG_DCF("Error: Create next direcotry failed.\n");
            return NULL;
        }

        /* file initialization */
        if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
        {
            /* file initialization failed */
            //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
            OSSemPost(dcfReadySemEvt);
            DEBUG_DCF("Error: File initialization failed.\n");
            return NULL;
        }
        dcfFileNumLast++;

    }

    strcpy((char*)newFileName, DCF_FILE_PREFIX);
    dcfIntToStr(dcfFileNumLast, &newFileName[4], 4);

    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: fileType=%d does not exist\n", fileType);
        return NULL;
    }
    strcat((char*)newFileName,".");
    strcat((char*)newFileName,gdcfFileType_Info[fileType].pSubFileStr);

    DEBUG_DCF("Over Write create Name: %s\n",newFileName);

    strcpy(&dcfFileName[index][0], newFileName);

    if ((pFile = dcfOpen(newFileName, "w")) == NULL)
    {
        /* create next file error */
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: create next file %s failed.\n", newFileName);
        return NULL;
    }

    /* list file newly created */
    if (dcfGetDirEnt(NULL, &dcfFileEnt[dcfFileCount], newFileName) == 0)
    {
        /* list file error */
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: Find file entry of %s failed.\n", newFileName);
        return NULL;
    }
    dcfFileEnt[dcfFileCount].FileEntrySect=0;

    gfileType=fileType;
    dcfFileInfoTab[index].dcfFileInfoCount = dcfFileCount;
    dcfFileInfoTab[index].dcfFileInfoListCount = dcfListFileCount;
    dcfFileInfoTab[index].dcfFileInfoNumLast = dcfFileNumLast;
    dcfFileInfoTab[index].dcfFileInfoFileType = fileType;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DSC dcfCreateNextFile\n");
    return pFile;
}



/***********************************************
Routine Description:
    將新增檔加入檔案鏈結.

Arguments:
	None.

Return Value:
	None
************************************************/



void dcfInsertFileNode(DCF_FILE_INFO* File)
{
    INT8U   err;

    DEBUG_DCF("Enter DSC dcfInsertFileNode\n");
    dcfListFileEnt[File->dcfFileInfoListCount].pDirEnt = &dcfFileEnt[File->dcfFileInfoCount];
    dcfListFileEnt[File->dcfFileInfoListCount].fileNum = File->dcfFileInfoNumLast;
    dcfListFileEnt[File->dcfFileInfoListCount].fileType = File->dcfFileInfoFileType;
    dcfListFileEntInsert(&dcfListFileEnt[File->dcfFileInfoListCount]);
    //dcfFileCount++;
    dcfListFileEntTail = &dcfListFileEnt[File->dcfFileInfoListCount];

    /* set current playback file */
    //dcfPlaybackCurFile=dcfListFileEntTail;

    dcfFileTypeCount_Inc(File->dcfFileInfoFileType);
    global_totalfile_count++;
    DEBUG_DCF("Leave DSC dcfInsertFileNode\n");
}

s32 dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile)
{
    s32 rel;
    INT8U   err;

    if (idx >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfClose Error Index %d\n",idx);
        return 0;
    }
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF("\ndcfCloseFileByIdx Index %d\n",idx);
    rel = dcfClose(pFile, pOpenFile);
    dcfInsertFileNode(&dcfFileInfoTab[idx]);
    dcfNewFile = 1;
    DEBUG_DCF("dcfCloseFileByIdx idx=%d, OSFlagPost Close %d\n\n", idx, rel);
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    return rel;
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
    DCF_LIST_FILEENT* curFile;
    INT8U err;

    DEBUG_DCF("dcfPlaybackDelDir Begin\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfPlaybackCurDir == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current Dir.\n");
        return 0;
    }
    else
    {
        if(dcfScanFileOnPlaybackDir() == 0)
        {
            DEBUG_DCF("DelDir do dcfScanFileOnPlaybackDir Fail.\n");
        }

        if(dcfListFileEntHead != NULL)
        {
            curFile = dcfListFileEntHead;
            /* check if null */
            if (curFile)
            {
                /* forward traverse whole files */
                do
                {
                    /* delete file */
                    dcfDel((s8*)curFile->pDirEnt->d_name,0);

                    /* next file */
                    curFile = curFile->next;
                }
                while (curFile != dcfListFileEntHead);
            }
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
        }
    }

    /* delete directory */
    dcfChDir("..");
    dcfRmDir((s8*)dcfPlaybackCurDir->pDirEnt->d_name,1);
    global_totaldir_count--;
    dcfListDirCount --;

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurDir->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurDir->used_flag = DCF_FILE_USE_NONE;
#endif

    if(dcfListDirEntHead != dcfListDirEntTail)
    {
        //above 2 files
        dcfPlaybackCurDir->prev->next = dcfPlaybackCurDir->next;
        dcfPlaybackCurDir->next->prev = dcfPlaybackCurDir->prev;
        if (dcfPlaybackCurDir == dcfListDirEntHead)
            dcfListDirEntHead = dcfPlaybackCurDir->next;
        dcfListDirEntTail = dcfListDirEntHead->prev;
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurDir = dcfListDirEntHead = dcfListDirEntTail = NULL;
        dcfDirNumLast =0;
        global_totaldir_count=0;
        dcfListDirCount=0;
    }
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelDir\n");

    return 1;

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
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    char LogFileName[32];
#endif

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
    {
        return dcfPlaybackDelPic();
    }
#endif
    DEBUG_DCF("Enter dcfPlaybackDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: file type doesn't exist, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;

    osdDrawDelMsg((s8*)dcfPlaybackCurFile->pDirEnt->d_name,0);
    dcfDel((s8*)dcfPlaybackCurFile->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->FileEntrySect);

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))

    strncpy(LogFileName, (s8*)dcfPlaybackCurFile->pDirEnt->d_name,9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if (dcfDel((s8*)LogFileName,dcfPlaybackCurFile->pDirEnt->FileEntrySect)==0)
    {
        DEBUG_DCF("log File: %s not found \n",LogFileName);
        //fix me =>Albert
        //return 0;
    }
#endif


#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;
#endif

    //Albert Modified on 20091209
    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListFileEntHead)
            dcfListFileEntHead = dcfPlaybackCurFile->next;
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListFileEntHead = dcfListFileEntTail = NULL;
        dcfFileNumLast =0;
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDel\n");
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
    DCF_LIST_DIRENT* curDir;
    DCF_LIST_FILEENT* curFile;
    u32 index=0;
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDelAll....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    /* delete all files */
    /* set current directory */
    curDir = dcfListDirEntHead;
    system_busy_flag=1;

    /* forward traverse whole directories */
    //Lucian: 從File Link 頭砍到尾. 先砍檔案再砍目錄. 再進下一個目錄. and so on..
    do
    {
        /* file initialization */
        if (dcfFileInit(curDir, CURRENTFILEENTRY) == 0)
        {
            system_busy_flag=0;
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
        /* set current file */
        curFile = dcfListFileEntHead;

        /* check if null */
        if (curFile)
        {
            /* forward traverse whole files */
            do
            {
                /* delete file */
                index++;
                osdDrawDelMsg((s8*)curFile->pDirEnt->d_name,index);
                dcfDel((s8*)curFile->pDirEnt->d_name,0);

                /* next file */
                curFile = curFile->next;
            }
            while (curFile != dcfListFileEntHead);
        }

        /* delete directory */
        dcfChDir("..");
        dcfRmDir((s8*)curDir->pDirEnt->d_name,1);

        global_totaldir_count--;

        /* next directory */
        curDir = curDir->next;
    }
    while (curDir != dcfListDirEntHead);

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    /* directory initialization */
    if (dcfDirInit() == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfDirInit() == 0\n");
        return 0;
    }
    /* file initialization */
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfFileInit Err\n");
        return 0;
    }
    playback_location=0xFF;
    system_busy_flag=0;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelAll....\n");
    return 1;

}

#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFileNext....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFileNext: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No next file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->next;

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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFilePrev....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    //DEBUG_DCF("dcfPlaybackFilePrev: begin ,dcfListDirCount=%d\n",dcfListDirCount);

    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No previous file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->prev;
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFilePrev\n");

    return 1;
}

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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirForward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirForward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirBackward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirBackward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
    return 0;
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

    DEBUG_DCF("Enter dcfScanFileOnPlaybackDir....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if (dcfFileInit(dcfPlaybackCurDir, dcfFileGroup))
    {
        /* check if file of list head is null */
        if (dcfListFileEntHead)
        {
            /* set current file */
            dcfPlaybackCurFile = dcfListFileEntHead;
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Leave dcfScanFileOnPlaybackDir\n");
            return 1;
        }
    }
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("dcfFileInit Fail\n");
    return 0;
}


/*

Routine Description:

	The directory initialization routine of DCF.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfDirInit(void)
{
    u32 i;
    u32 dirNum;

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count = 0;

    dcfDirNumLast = 99; /* 100 - 1; Lucian: 目錄從100XXXX 開始算起 */

    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir("\\DCIM") == 0)
    {
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        /* change directory \DCIM error */
        if (dcfMkDir("\\DCIM") == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory \\DCIM failed.\n");

            return -1;
        }

        if (dcfChDir("\\DCIM") == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory \\DCIM failed.\n");

            return -1;
        }
    }

    /* list directory \DCIM */
    // List all the directory which in DCIM civic
    if (dcfDir("\\DCIM", dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory \\DCIM failed.\n");

        return 0;
    }

    /* sort the directory and create the directory list */
    memset_hw((void*)dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    dcfListDirCount = 0;
    // List the valid directory and make the linked list
    for (i = 0; i < dcfDirCount; i++)
    {
        if (dcfDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            /* check if valid dcf directory */
            if (dcfCheckDir(&dcfDirEnt[i], &dirNum) == 0)
                continue;

            /* assign last directory number */
            dcfDirNumLast = (dcfDirNumLast < dirNum) ? dirNum : dcfDirNumLast;

            /* insert directory entry to the list */
            dcfListDirEnt[dcfListDirCount].dirNum = dirNum;
            dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[i];
            dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
            dcfListDirCount++;
            //Add on 20091208, for the playback of CDVR File system
            global_totaldir_count ++;
        }
    }

    /* create a subdirectory if none is found */
    if (dcfListDirCount == 0)
    {
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
        //Lucian: Only one Sub-directory
        for (i=0; i<estimate_dir_nums-1; i++)
        {
            if (dcfCreateNextDir() == 0)
            {
                /* create directory failed */
                DEBUG_DCF("Error: Create next direcotry failed.\n");

                return 0;
            }
        }
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DSC)
        if (dcfCreateNextDir() == 0)
        {
            /* create directory failed */
            DEBUG_DCF("Error: Create next direcotry failed.\n");

            return 0;
        }
#endif
    }

    /* set tail entry of directory list */
    dcfListDirEntTail = dcfListDirEntHead->prev;

    /* set current playback directory */
    dcfPlaybackCurDir = dcfListDirEntHead;

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
s32 dcfFileInit(DCF_LIST_DIRENT* pListDirEnt, u8 Type)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
//    DCF_LIST_DIRENT* temp_head=dcfListDirEntHead;
//    u32 local_dcfFileCount;
//    struct FS_DIRENT* local_dcfFileEnt = &dcfFileEnt[0];
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

    DEBUG_DCF("dcfFileInit: Begin ,Type=%d\n",Type);
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 1 =%d (x50ms)\n",time1);
#endif

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;
    dcfResetFileEntrySect(0,0);
    /* set tail entry of file list */

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfFileNumLast = 0; /* 0001 - 1 */
    dcfListFileEntHead = NULL;

    dcfFileCount=0; //civicwu 070926
    memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926
    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir("\\DCIM") == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory \\DCIM failed.\n");

        return 0;
    }

    /* change current directory */
    if (dcfChDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
    {
        /* change current directory error */
        DEBUG_DCF("Error: Change current directory failed.\n");

        return 0;
    }

    /* list current directory */
    if (dcfDir(NULL, dcfFileEnt, &dcfFileCount,1,1,DCF_FILEENT_MAX) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");

        return 0;
    }
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 2 =%d (x50ms)\n",time1);
#endif
    /* sort the file and create the file list */
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    dcfListFileCount = 0;

    for (i = 0; i < dcfFileCount; i++)
    {
        if ((dcfFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
        {
            /* check if valid dcf file */
#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB0) ||(FILE_SYSTEM_SEL == FILE_SYSTEM_DSC) )
            if (dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                continue;
#elif ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
            if (dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                continue;
#endif

#if FILE_REPAIR_AUTO //Repair Bad file
            if(fileType == 6) // AXF file
            {
                DEBUG_SYS("Repair Bad File:%s\n",dcfFileEnt[i].d_name);
                uiClearOSDBuf(2);
                uiMenuOSDStringByColor(TVOSD_SizeX , "Repair Bad File:" , OSD_STRING_W , OSD_STRING_H , 48, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                uiMenuOSDStringByColor(TVOSD_SizeX , dcfFileEnt[i].d_name , OSD_STRING_W , OSD_STRING_H , 176, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                asfRepairFile(dcfFileEnt[i].d_name);
                DEBUG_SYS("===Repair Complete===\n");

                memcpy(NewName,dcfFileEnt[i].d_name,13);
                NewName[10]='S';
                if(0 == dcfRename(NewName,dcfFileEnt[i].d_name))
                {
                    DEBUG_DCF("Warning! DCF Rename Fail!\n");
                }
                fileType=0;
                dcfFileEnt[i].d_name[10]='S';
            }
#endif
            dcfFileTypeCount_Inc(fileType);
            global_totalfile_count++;

            /* assign last file number */
            dcfFileNumLast = (dcfFileNumLast < fileNum) ? fileNum : dcfFileNumLast;

            /* insert file entry to the list */
            dcfListFileEnt[dcfListFileCount].used_flag=DCF_FILE_USE_CLOSE;
            dcfListFileEnt[dcfListFileCount].fileNum = fileNum;
            dcfListFileEnt[dcfListFileCount].fileType = fileType;
            dcfListFileEnt[dcfListFileCount].pDirEnt = &dcfFileEnt[i];
            dcfListFileEntInsert(&dcfListFileEnt[dcfListFileCount]);
            dcfListFileCount++;
        }

        sysDeadLockMonitor_Reset();
    }

    /* set tail entry of file list */
    if( dcfListFileEntHead != NULL)
    {
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfListFileEntTail->next =dcfListFileEntHead;
    }
    else
    {
        dcfListFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile=dcfListFileEntHead->prev;

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 3 =%d (x50ms)\n",time1);
#endif

    return 1;
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
s32 dcfCheckDir(struct FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j;
    s8 dirNumStr[4];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".   ") != 0)
        return 0;

    /* get dcf directory number string */
    for (j = 0; j < 3; j++)
        dirNumStr[j] = pDirEnt->d_name[j];
    dirNumStr[3] = '\0';

    /* dcf directory number is from 100 to 999 */
    *pDirNum = (u32)atoi((const char*)dirNumStr);
    if ((*pDirNum < 100) || (*pDirNum > 999))
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

s32 dcfCheckFile(struct FS_DIRENT* pFileEnt, u32* pFileNum, u8* pFileType, u8 FileGroup)
{
    u32 j;
    u8 fileNumStr[9];
#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1)	 || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3) )
    char cchar;
#endif

    //check sub file name
    for (j = 0; j < 3; j++)
        fileNumStr[j] = pFileEnt->d_name[j+9];
    fileNumStr[j] = '\0';

    if ((*pFileType=dcfGetFileType(fileNumStr))==DCF_FILE_TYPE_MAX)
        return 0;

//**** Albert Lee , modify the point to add other file system
#if (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB0)
    /* get dcf file number string */
    for (j = 0; j < 8; j++)
        fileNumStr[j] = pFileEnt->d_name[j];
    fileNumStr[8] = '\0';
#elif (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1)
    cchar = pFileEnt->d_name[0];
    if ((cchar != 'A') && (cchar != 'B'))
        return 0;


    if( ( FileGroup == FILEENTRY_ALL) ||
            ( (FileGroup == FILEENTRY_AGROUP) && (cchar == 'A') ) ||
            ( (FileGroup == FILEENTRY_BGROUP) && (cchar == 'B') ) )
    {
        for (j = 0; j < 7; j++)
            fileNumStr[j] = pFileEnt->d_name[j+1];
        fileNumStr[7] = '\0';

    }
    else
        return 0;
#elif (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2)
    cchar = pFileEnt->d_name[0];
    if ((cchar != 'A') && (cchar != 'B') && (cchar != 'C'))
        return 0;

    if( ( FileGroup == FILEENTRY_ALL) ||
            ( (FileGroup == FILEENTRY_AGROUP) && (cchar == 'A') ) ||
            ( (FileGroup == FILEENTRY_BGROUP) && (cchar == 'B') ) ||
            ( (FileGroup == FILEENTRY_CGROUP) && (cchar == 'C') ) )
    {
        for (j = 0; j < 7; j++)
            fileNumStr[j] = pFileEnt->d_name[j+1];
        fileNumStr[7] = '\0';

    }
    else
        return 0;
#elif(SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3)
    cchar = pFileEnt->d_name[0];
    if ((cchar != 'A') && (cchar != 'B') && (cchar != 'C') && (cchar != 'D'))
        return 0;

    if( ( FileGroup == FILEENTRY_ALL) ||
            ( (FileGroup == FILEENTRY_AGROUP) && (cchar == 'A') ) ||
            ( (FileGroup == FILEENTRY_BGROUP) && (cchar == 'B') ) ||
            ( (FileGroup == FILEENTRY_CGROUP) && (cchar == 'C') ) ||
            ( (FileGroup == FILEENTRY_DGROUP) && (cchar == 'D') ) )
    {
        for (j = 0; j < 7; j++)
            fileNumStr[j] = pFileEnt->d_name[j+1];
        fileNumStr[7] = '\0';

    }
    else
        return 0;
#endif

//	DEBUG_DCF("Trace: dcfCheckFile 5: Name= %s Type = %d.\n", fileNumStr,*pFileType);
    /* dcf file number is from 0001 to 9999 */
    *pFileNum = (u32)atoi((const char*)fileNumStr);
    if ((*pFileNum < 0000001) || (*pFileNum > DCF_FILENAME_MAX))
        return 0;

//	DEBUG_DCF("Trace: dcfCheckFile 6: pFileNum = %d.\n", *pFileNum);

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
s32 dcfIntToStr(u32 val, s8* str, s32 cnt)
{
    s8 i;
    s8 dig[9];

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


/*

Routine Description:

	Create next directory.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 dcfCreateNextDir(void)
{
    s8 newDirName[32];
    s8 newTargetName[64];
    u32 count = 900;
    INT8U   err;

    DEBUG_DCF("Enter DVR dcfCreateNextDir\n");
    do
    {
        dcfDirNumLast++;
        if (dcfDirNumLast > 999)
            dcfDirNumLast = 100;

        dcfIntToStr(dcfDirNumLast, newDirName, 3);
        strcat((char*)newDirName, DCF_DIR_SUFFIX);
        strcat((char*)newDirName, ".   ");
        strcpy((char*)newTargetName, "\\DCIM\\");
        strcat((char*)newTargetName, (const char*)newDirName);
        if (dcfMkDir(newTargetName))
        {
            /* make next directory ok */
            break;
        }
    }
    while (count--);

    if (count == 0)
    {
        /* none valid directory name is found */
        DEBUG_DCF("dcfCreateNextDir no directory\n");
        return 0;
    }

    /* list directory newly created */
    if (dcfGetDirEnt("\\DCIM", &dcfDirEnt[dcfDirCount], newDirName) == 0)
    {
        /* list directory error */
        DEBUG_DCF("Error: Find directory entry of %s failed.\n", newDirName);
        return 0;
    }
//#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
    //Lucian: 增加direct FDB entry. 增加至4096 entry.
    dcfIncDirEnt(newTargetName,DCF_FILEENT_MAX);
//#endif
    /* insert directory entry to the list */
    dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[dcfDirCount];
    dcfListDirEnt[dcfListDirCount].dirNum = dcfDirNumLast;
    dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
    dcfDirCount++;

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count ++;

    dcfListDirEntTail = &dcfListDirEnt[dcfListDirCount++];
    DEBUG_DCF("Leave DVR dcfCreateNextDir\n");
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
FS_FILE* dcfCreateNextFile(u8 fileType, u8 index)
{
    s8 newFileName[32];
    FS_FILE* pFile;
    int     scan;
    INT8U   err;
    unsigned short val;
    RTC_DATE_TIME   localTime;

    DEBUG_DCF("Enter DVR dcfCreateNextFile %d....\n",index);
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (global_totalfile_count > (DCF_FILE_PER_DIR-20))
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("file number is over: %d\n",global_totalfile_count);
        return 0;
    }

    if (index >= DCF_MAX_MULTI_FILE)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("dcfCreateNextFile Error Index %d\n",index);
        return 0;
    }

    dcfFileNumLast++;

    if (dcfFileNumLast > DCF_FILENAME_MAX)
    {
        dcfFileNumLast=0;
    }

    //------可在此做分類檔案-------//
//**** Albert Lee , modify the point to add other file system
#if (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB0)
    //strcpy((char*)newFileName, DCF_FILE_PREFIX);
    dcfIntToStr(dcfFileNumLast, &newFileName[0], 8);
#elif (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1)
    dcfIntToStr(dcfFileNumLast, &newFileName[1], 7);
    if( dcfFileGroup == FILEENTRY_AGROUP)
        newFileName[0]='A';
    else if ( dcfFileGroup == FILEENTRY_BGROUP)
        newFileName[0]='B';
#elif (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2)
    dcfIntToStr(dcfFileNumLast, &newFileName[1], 7);
    if( dcfFileGroup == FILEENTRY_AGROUP)
        newFileName[0]='A';
    else if ( dcfFileGroup == FILEENTRY_BGROUP)
        newFileName[0]='B';
    else if ( dcfFileGroup == FILEENTRY_CGROUP)
        newFileName[0]='C';

#elif (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3)
    dcfIntToStr(dcfFileNumLast, &newFileName[1], 7);
    if( dcfFileGroup == FILEENTRY_AGROUP)
        newFileName[0]='A';
    else if ( dcfFileGroup == FILEENTRY_BGROUP)
        newFileName[0]='B';
    else if ( dcfFileGroup == FILEENTRY_CGROUP)
        newFileName[0]='C';
    else if ( dcfFileGroup == FILEENTRY_DGROUP)
        newFileName[0]='D';
#endif
    //-----------------------------//

    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: fileType=%d does not exist\n", fileType);
        return NULL;
    }
    strcat((char*)newFileName,".");
    strcat((char*)newFileName,gdcfFileType_Info[fileType].pSubFileStr);

    strcpy(&dcfFileName[index][0], newFileName);

    if ((pFile = dcfOpen(newFileName, "w-")) == NULL) // "w-": Create file 前不scan FDB.
    {
        /* create next file error */
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: create Name: %s, error\n",newFileName);
        return NULL;
    }


    DEBUG_DCF("Over Write create Name: %s, %d\n",newFileName,global_totalfile_count);
    if(dcfListFileEntHead != NULL)
    {
        DEBUG_DCF("Over Write Head Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
    }

    /* list file newly created */
    scan=0;
    while(dcfFileEnt[dcfFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfFileCount ++;
        if(dcfFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfFileBuf is full!\n");
            dcfFileCount=0;
            break;
        }
    }
    dcfFileEnt[dcfFileCount].used_flag = DCF_FILE_USE_CREATE;

    scan=0;
    while(dcfListFileEnt[dcfListFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListFileCount ++;
        if(dcfListFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfListFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListFileBuf is full!\n");
            dcfListFileCount=0;
            break;
        }
    }
    dcfListFileEnt[dcfListFileCount].used_flag = DCF_FILE_USE_CREATE;

    DEBUG_DCF("dcfFileCount: %d\n",dcfFileCount);
    DEBUG_DCF("dcfListFileCount: %d\n",dcfListFileCount);

#if 0
    if (dcfGetDirEnt(NULL, &dcfFileEnt[dcfFileCount], newFileName) == 0)
    {
        /* list file error */
        DEBUG_DCF("Error: Find file entry of %s failed.\n", newFileName);

        return NULL;
    }
#else

    dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
    strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

    dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

    RTC_Get_Time(&localTime);
    val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
    dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
    dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;

#endif
#if 0
    DEBUG_ASF("dcfFileEnt:name:%s, %d, %X, %X, %X, %X\n"
              ,dcfFileEnt[dcfFileCount].d_name,dcfFileEnt[dcfFileCount].FAT_DirAttr
              ,dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS,dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD
              ,dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS,dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD);

#endif
    gfileType = fileType;
    dcfFileInfoTab[index].dcfFileInfoCount = dcfFileCount;
    dcfFileInfoTab[index].dcfFileInfoListCount = dcfListFileCount;
    dcfFileInfoTab[index].dcfFileInfoNumLast = dcfFileNumLast;
    dcfFileInfoTab[index].dcfFileInfoFileType = fileType;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DVR dcfCreateNextFile\n");
    return pFile;
}

#if FS_NEW_VERSION
s32 dcfOverWriteDel(void)
{
	int ret;
	u8 err;
	
	if(dcfListDirEntHead == NULL)
	{	
		DEBUG_DCF("[ERR] DCF Head of Dir entrys is NULL.\n");
		return 0;
	}
	
	DEBUG_DCF("Enter DVR dcfOverWriteDel....\n");
	OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

	if(dcfListDelDirEnt != dcfListDirEntHead)
		dcfListDelDirEnt = dcfListDirEntHead;

	if((ret = dcfOWDel(dcfListDelDirEnt->pDirEnt->d_name)) < 0)
	{
		DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
		OSSemPost(dcfReadySemEvt);
		return 0;
	}

	if(ret == 0)
		DEBUG_DCF("[INF] Directory is empty.\n");

	OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DVR dcfOverWriteDel\n");
    return 1;
}
#else
s32 dcfOverWriteDel(void)
{
    char* pExtName ;
    char extName[5];
    char j;
    u8 ucfiletype;
    u8  err;
    int ret;

    DEBUG_DCF("Enter DVR dcfOverWriteDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_ASF("Error: file type doesn't exist, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;


    // find number minum
    dcfListFileEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfListFileEntHead->used_flag = DCF_FILE_USE_NONE;
    DEBUG_ASF("Over Write delete Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
    strcpy((char*)dcfDelDir, (char*)dcfCurDir);
    if ((ret = dcfOWDel((s8*)dcfListFileEntHead->pDirEnt->d_name,dcfListFileEntHead->pDirEnt->FileEntrySect)) <=0)
    {
        DEBUG_ASF("Error: file delete fail %s\nError code: %d\n", dcfListFileEntHead->pDirEnt->d_name, ret);
        switch(ret)
        {
            case FS_FILE_ENT_FIND_ERROR:
                if(dcfListFileEntHead != dcfListFileEntTail)
                {
                    dcfListFileEntHead=dcfListFileEntHead->next;
                    dcfListFileEntTail->next =dcfListFileEntHead;
                    dcfListFileEntHead->prev=dcfListFileEntTail;
                }
                else
                    dcfListFileEntHead = dcfListFileEntTail = NULL;
                break;
            default:
                break;
        }
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListFileEntHead=dcfListFileEntHead->next;
        dcfListFileEntTail->next =dcfListFileEntHead;
        dcfListFileEntHead->prev=dcfListFileEntTail;
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X , count= %X \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }
    else
    {
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListFileEntHead = dcfListFileEntTail = NULL;
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X, count= %X  \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DVR dcfOverWriteDel\n");
    return 1;
}

#endif


/***********************************************
Routine Description:
    將新增檔加入檔案鏈結.

Arguments:
	None.

Return Value:
	None
************************************************/



//for the close file under  recording
void dcfInsertFileNode_ToTail(DCF_FILE_INFO* File)
{
    INT8U   err;

    dcfFileEnt[File->dcfFileInfoCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].pDirEnt = &dcfFileEnt[File->dcfFileInfoCount];
    dcfListFileEnt[File->dcfFileInfoListCount].fileNum = File->dcfFileInfoNumLast;
    dcfListFileEnt[File->dcfFileInfoListCount].fileType = File->dcfFileInfoFileType;
    dcfListFileEntInsert_ToTail(&dcfListFileEnt[File->dcfFileInfoListCount]);
    dcfListFileEntTail = dcfListFileEntHead->prev;

    dcfFileTypeCount_Inc(File->dcfFileInfoFileType);
    global_totalfile_count++;
}

s32 dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile)
{
    s32 rel;
    INT8U   err;

    if (idx >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfCloseFileByIdx Error Index %d\n",idx);
        return 0;
    }
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF("\ndcfCloseFileByIdx Index %d\n",idx);
    rel = dcfClose(pFile, pOpenFile);
    dcfInsertFileNode_ToTail(&dcfFileInfoTab[idx]);
    dcfNewFile = 1;
    DEBUG_DCF("dcfCloseFileByIdx idx=%d, OSFlagPost Close %d\n\n", idx, rel);
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    return rel;
}

#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
void dcfSetFileGroup(u8 ucmode)
{
    if(ucmode == FILEGROUP_A)
        dcfFileGroup = FILEENTRY_AGROUP;
    else if(ucmode == FILEGROUP_B)
        dcfFileGroup = FILEENTRY_BGROUP;
#if ( (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3) )
    else if(ucmode == FILEGROUP_C)
        dcfFileGroup = FILEENTRY_CGROUP;
#endif
#if (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3)
    else if(ucmode == FILEGROUP_D)
        dcfFileGroup = FILEENTRY_DGROUP;
#endif

}

u8 dcfGetFileGroup(void)
{
    u8 ucmode;

    if(dcfFileGroup == FILEENTRY_AGROUP)
        ucmode = FILEGROUP_A;
    else if(dcfFileGroup == FILEENTRY_BGROUP)
        ucmode = FILEGROUP_B;
#if ( (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3) )
    else if(dcfFileGroup == FILEENTRY_CGROUP)
        ucmode = FILEGROUP_C;
#endif
#if (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3)
    else if(dcfFileGroup == FILEENTRY_DGROUP)
        ucmode = FILEGROUP_D;
#endif
    return ucmode;
}
u8 dcfSaveFileGroup(void)
{

    dcfFileGroupbackup = dcfFileGroup;
    return dcfFileGroupbackup;
}

u8 dcfRestoreFileGroup(void)
{

    dcfFileGroup = dcfFileGroupbackup;
    return dcfFileGroup;
}
#endif

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
    DCF_LIST_FILEENT* curFile;
    INT8U err;

    DEBUG_DCF("dcfPlaybackDelDir Begin\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfPlaybackCurDir == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current Dir.\n");
        return 0;
    }
    else
    {
        if(dcfScanFileOnPlaybackDir() == 0)
        {
            DEBUG_DCF("DelDir do dcfScanFileOnPlaybackDir Fail.\n");
        }

        if(dcfListFileEntHead != NULL)
        {
            curFile = dcfListFileEntHead;
            /* check if null */
            if (curFile)
            {
                /* forward traverse whole files */
                do
                {
                    /* delete file */
                    dcfDel((s8*)curFile->pDirEnt->d_name,0);

                    /* next file */
                    curFile = curFile->next;
                }
                while (curFile != dcfListFileEntHead);
            }
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
        }
    }

    /* delete directory */
    dcfChDir("..");
    dcfRmDir((s8*)dcfPlaybackCurDir->pDirEnt->d_name,1);
    global_totaldir_count--;
    dcfListDirCount --;

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurDir->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurDir->used_flag = DCF_FILE_USE_NONE;
#endif

    if(dcfListDirEntHead != dcfListDirEntTail)
    {
        //above 2 files
        dcfPlaybackCurDir->prev->next = dcfPlaybackCurDir->next;
        dcfPlaybackCurDir->next->prev = dcfPlaybackCurDir->prev;
        if (dcfPlaybackCurDir == dcfListDirEntHead)
            dcfListDirEntHead = dcfPlaybackCurDir->next;
        dcfListDirEntTail = dcfListDirEntHead->prev;
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurDir = dcfListDirEntHead = dcfListDirEntTail = NULL;
        dcfDirNumLast =0;
        global_totaldir_count=0;
        dcfListDirCount=0;
    }
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelDir\n");

    return 1;

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
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    char LogFileName[32];
#endif

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
    {
        return dcfPlaybackDelPic();
    }
#endif
    DEBUG_DCF("Enter dcfPlaybackDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: file type doesn't exist, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;

    osdDrawDelMsg((s8*)dcfPlaybackCurFile->pDirEnt->d_name,0);
    dcfDel((s8*)dcfPlaybackCurFile->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->FileEntrySect);

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))

    strncpy(LogFileName, (s8*)dcfPlaybackCurFile->pDirEnt->d_name,9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if (dcfDel((s8*)LogFileName,dcfPlaybackCurFile->pDirEnt->FileEntrySect)==0)
    {
        DEBUG_DCF("log File: %s not found \n",LogFileName);
        //fix me =>Albert
        //return 0;
    }
#endif


#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;
#endif

    //Albert Modified on 20091209
    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListFileEntHead)
            dcfListFileEntHead = dcfPlaybackCurFile->next;
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListFileEntHead = dcfListFileEntTail = NULL;
        dcfFileNumLast =0;
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDel\n");
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
    DCF_LIST_DIRENT* curDir;
    DCF_LIST_FILEENT* curFile;
    u32 index=0;
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDelAll....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    /* delete all files */
    /* set current directory */
    curDir = dcfListDirEntHead;
    system_busy_flag=1;

    /* forward traverse whole directories */
    //Lucian: 從File Link 頭砍到尾. 先砍檔案再砍目錄. 再進下一個目錄. and so on..
    do
    {
        /* file initialization */
        if (dcfFileInit(curDir, CURRENTFILEENTRY) == 0)
        {
            system_busy_flag=0;
            return 0;
        }
        /* set current file */
        curFile = dcfListFileEntHead;

        /* check if null */
        if (curFile)
        {
            /* forward traverse whole files */
            do
            {
                /* delete file */
                index++;
                osdDrawDelMsg((s8*)curFile->pDirEnt->d_name,index);
                dcfDel((s8*)curFile->pDirEnt->d_name,0);

                /* next file */
                curFile = curFile->next;
            }
            while (curFile != dcfListFileEntHead);
        }

        /* delete directory */
        dcfChDir("..");
        dcfRmDir((s8*)curDir->pDirEnt->d_name,1);

        global_totaldir_count--;

        /* next directory */
        curDir = curDir->next;
    }
    while (curDir != dcfListDirEntHead);

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    /* directory initialization */
    if (dcfDirInit() == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfDirInit() == 0\n");
        return 0;
    }
    /* file initialization */
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfFileInit Err\n");
        return 0;
    }
    playback_location=0xFF;
    system_busy_flag=0;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelAll....\n");
    return 1;

}

#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
//*******************************************************************************
//	DCF for FILE_SYSTEM_CDVR
//*******************************************************************************

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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFileNext....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFileNext: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No next file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->next;

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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFilePrev....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFilePrev: begin ,dcfListDirCount=%d\n",dcfListDirCount);

    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No previous file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->prev;

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFilePrev\n");
    return 1;
}

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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirForward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirForward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirBackward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirBackward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
    return 0;
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

    DEBUG_DCF("Enter dcfScanFileOnPlaybackDir....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
#if CDVR_REC_WITH_PLAY_SUPPORT
    if (dcfFileInit(dcfPlaybackCurDir, PLAYFILEENTRY))
    {
        /* check if file of list head is null */
        if (dcfListReadFileEntHead)
        {
            /* set current file */
            dcfPlaybackCurFile = dcfListReadFileEntTail;
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Leave dcfScanFileOnPlaybackDir\n");
            return 1;
        }
    }
#else
    if (dcfFileInit(dcfPlaybackCurDir, PLAYFILEENTRY))
    {
        /* check if file of list head is null */
        if (dcfListFileEntHead)
        {
            /* set current file */
            dcfPlaybackCurFile = dcfListFileEntTail;
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Leave dcfScanFileOnPlaybackDir\n");
            return 1;
        }
    }
#endif
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("dcfFileInit Fail\n");
    return 0;
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
s32 dcfDirInit(void)
{
    u32 dirNum;
    u32 i;
#if (ROOTWORK==0)
    s8  DirName[32];
#endif


    DEBUG_DCF("dcfDirInit: Begin \n");

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count = 0;

    dcfDirNumLast=0;
    dcfFileNumLast = 0; /* 0001 - 1 */
    dcfListDirEntTail = dcfListDirEntHead = dcfPlaybackCurDir =NULL;
    dcfDelFileNumLast = 0; /* 0001 - 1 */
    dcfListDelFileEntHead = NULL;
    dcfListDelFileEntTail = NULL;
    dcfFileCount=0;
    dcfListFileCount=0;
    dcfTotalFileCntInDir=0;
    dcfListDelFileCount = dcfDelFileCount=0;

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;
    memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

    memset_hw((void*)dcfDirEnt, 0, sizeof(struct FS_DIRENT) * DCF_DIRENT_MAX);
#if ROOTWORK
    // list root directory
    // List all the directory in root directory
    if (dcfDir("\\", dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        // list root directory error
        DEBUG_DCF("Warning:List root directory empty\n");

        //return 0;
    }
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    uiCheckUpgradeFileName(dcfDirEnt, dcfDirCount);
#endif
#else
    if (dcfDir("\\", dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        // list root directory error
        DEBUG_DCF("Warning:List root directory empty\n");

        //return 0;
    }
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    uiCheckUpgradeFileName(dcfDirEnt, dcfDirCount);
#endif

    sprintf ((char*)DirName, "\\%s", gsDirName);

    if (dcfChDir(DirName) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(DirName) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",DirName);

            return -1;
        }

        //    //Lucian: 增加direct FDB entry. 增加至 512 entry.
        dcfIncDirEnt((s8 *)DirName,DCF_DIRENT_MAX);

        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return -1;
        }
    }


    /* list directory \DCIM */
    // List all the directory which in DCIM civic
    if (dcfDir(DirName, dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory %s failed.\n",DirName);
        //return 0;
    }
    DEBUG_DCF("--->dcfDir complete:%d.\n",dcfDirCount);
#endif


#if (HW_BOARD_OPTION == MR6730_AFN)
#if (FWUPG_FIND_SPEC_BIN)
    FW_findIspFilename(&ispUSBFileName[1]); // skip the "\\"
    DEBUG_DCF("searching ISP File:%s\n",ispUSBFileName);
#endif
#endif


    // sort the directory and create the directory list
    memset_hw((void*)dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    dcfListDirCount = 0;
    // List the valid directory and make the linked list
    for (i = 0; i < dcfDirCount; i++)
    {
        if (dcfDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            // check if valid dcf directory
            if (dcfCheckDir(&dcfDirEnt[i], &dirNum) == 0)
            {
                continue;
            }

            if(dcfListDirCount>=DCF_DIRENT_MAX)
            {
                break;
            }
            //DEBUG_DCF("(%d,%d) ",dirNum,i);
            // assign last directory number
            dcfDirNumLast = (dcfDirNumLast < dirNum) ? dirNum : dcfDirNumLast;

            // insert directory entry to the list
            dcfListDirEnt[dcfListDirCount].used_flag = DCF_FILE_USE_CLOSE;
            dcfListDirEnt[dcfListDirCount].dirNum = dirNum;
            dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[i];
            dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
            dcfListDirCount++;
            //Add on 20091208, for the playback of CDVR File system
            global_totaldir_count ++;
        }
    }


    if(dcfListDirEntHead != NULL)
    {
        /* set tail entry of directory list */
        dcfListDirEntTail = dcfListDirEntHead->prev;

        /* set current playback directory */
        dcfPlaybackCurDir = dcfListDirEntTail;
    }


#if 1
    {
        DCF_LIST_DIRENT* dcfListDirEntTemp = NULL;
        dcfListDirEntTemp = dcfListDirEntHead;
        if (dcfListDirEntHead == NULL)
            return 1;

        do //??
        {
            DEBUG_DCF("Dir Name: %s\n", dcfListDirEntTemp->pDirEnt->d_name);
            dcfListDirEntTemp = dcfListDirEntTemp->next;

        }
        while(dcfListDirEntTemp != dcfListDirEntHead );
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
s32 dcfFileInit(DCF_LIST_DIRENT* pListDirEnt, u8 Type)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;
    u8 CamerIdx;


#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

#if (ROOTWORK==0)
    s8	DirName[32];
#endif

    DEBUG_DCF("dcfFileInit: Begin %d\n", Type);

#if CDVR_REC_WITH_PLAY_SUPPORT
    if (Type == CURRENTFILEENTRY)
    {
        dcfResetFileEntrySect(0,0);

        dcfFileNumLast = 0; /* 0001 - 1 */
        dcfListFileEntHead = NULL;
        for (i = 0; i < DCF_MAX_MULTI_FILE; i++)
        {
            global_totalCamfile_count[i] = 0;
        }
        dcfListFileEntTail = NULL;
        dcfFileTypeCount_Clean();
        global_totalfile_count = 0;

        if (pListDirEnt == NULL)
        {
            DEBUG_DCF("Warning: Directory Empty\n");
            return 1;
        }
        /* set tail entry of file list */

        //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
        dcfFileCount=0; //civicwu 070926
        dcfTotalFileCntInDir=0;
        memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

#if ROOTWORK
        if (dcfChDir("\\") == 0)
        {
            /* change root directory error */
            DEBUG_DCF("Error: Change directory root failed.\n");

            return 0;
        }
#else
        //Albert Add
        //gsDirName
#if(SW_APPLICATION_OPTION ==  MR8100_DUALMODE_VBM)

#else
        sprintf ((char*)DirName, "\\%s", gsDirName);
#endif
        if (dcfChDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return 0;
        }
#endif


        DEBUG_DCF("Directory Name: %s\n",pListDirEnt->pDirEnt->d_name);

        /* change current directory */
        if (dcfChDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
        {
            /* change current directory error */
            DEBUG_DCF("Error: Change current directory failed.\n");

            return 0;
        }
#if 1
        //Lucian: 採用快速搜尋, 不對FDB 做排序. 可縮短開機時間. 只能用於封閉式系統.
        /* list current directory */
        if (dcfDirScan(NULL, dcfFileEnt, &dcfFileCount,&OldestEntry,1) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
        {
            /* list current directory error */
            DEBUG_DCF("Error: List current directory failed.\n");
            return 0;
        }

        /* sort the file and create the file list */
        memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

        dcfListFileCount = dcfFileCount;
        global_totalfile_count = dcfFileCount;
        dcfTotalFileCntInDir= dcfFileCount;

        if(dcfListFileCount==0)
        {
            dcfListFileCount=NULL;
        }
#if 1
        else if(dcfListFileCount==1)
        {
            dcfListFileEntHead = &dcfListFileEnt[0];

            ins = &dcfListFileEnt[0];
            ins->prev = &dcfListFileEnt[0];
            ins->next = &dcfListFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfFileEnt[0];
        }
#endif
        else
        {
            dcfListFileEntHead = &dcfListFileEnt[OldestEntry];

            ins = &dcfListFileEnt[0];
            ins->prev = &dcfListFileEnt[dcfFileCount-1];
            ins->next = &dcfListFileEnt[1];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfFileEnt[0];

            ins ++;

            for (i = 1; i < dcfFileCount-1; i++)
            {
                ins = &dcfListFileEnt[i];
                ins->prev = &dcfListFileEnt[i-1];
                ins->next = &dcfListFileEnt[i+1];

                ins->used_flag = DCF_FILE_USE_CLOSE;
                ins->pDirEnt = &dcfFileEnt[i];
                ins ++;
            }

            ins = &dcfListFileEnt[dcfFileCount-1];
            ins->prev = &dcfListFileEnt[dcfFileCount-2];
            ins->next = &dcfListFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfFileEnt[dcfFileCount-1];
        }
#else
        //Lucian:  一般收尋, 做檔名排序. 可做壞檔修復.
        /* list current directory */
        if (dcfDir(NULL, dcfFileEnt, &dcfFileCount,1,1,DCF_FILEENT_MAX) == 0)
        {
            /* list current directory error */
            DEBUG_DCF("Error: List current directory failed.\n");

            return 0;
        }

        /* sort the file and create the file list */
        memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
        dcfListFileCount = 0;

        for (i = 0; i < dcfFileCount; i++)
        {
            if ((dcfFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
            {
                /* check if valid dcf file */
                if (CamerIdx = dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                    continue;
#if FILE_REPAIR_AUTO //Repair Bad file
                if(fileType == 6) // AXF file
                {
                    DEBUG_SYS("Repair Bad File:%s\n",dcfFileEnt[i].d_name);
                    uiClearOSDBuf(2);
                    uiMenuOSDStringByColor(TVOSD_SizeX , "Repair Bad File:" , OSD_STRING_W , OSD_STRING_H , 48, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                    uiMenuOSDStringByColor(TVOSD_SizeX , dcfFileEnt[i].d_name , OSD_STRING_W , OSD_STRING_H , 176, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                    asfRepairFile(dcfFileEnt[i].d_name);
                    DEBUG_SYS("===Repair Complete===\n");

                    memcpy(NewName,dcfFileEnt[i].d_name,13);
                    NewName[10]='S';
                    if(0 == dcfRename(NewName,dcfFileEnt[i].d_name))
                    {
                        DEBUG_DCF("Warning! DCF Rename Fail!\n");
                    }
                    fileType=0;
                    dcfFileEnt[i].d_name[10]='S';
                }
#endif
                dcfFileTypeCount_Inc(fileType);
                if (CamerIdx != 0)
                    CamerIdx--;
                global_totalfile_count++;
                global_totalCamfile_count[CamerIdx]++;

                /* assign last file number */
                dcfFileNumLast = (dcfFileNumLast < fileNum) ? fileNum : dcfFileNumLast;

                /* insert file entry to the list */
                dcfListFileEnt[dcfListFileCount].used_flag = DCF_FILE_USE_CLOSE;
                dcfListFileEnt[dcfListFileCount].fileNum = fileNum;
                //DEBUG_DCF("fileNum=%d\n",fileNum);
                dcfListFileEnt[dcfListFileCount].fileType = fileType;
                dcfListFileEnt[dcfListFileCount].pDirEnt = &dcfFileEnt[i];
                dcfListFileEntInsert(&dcfListFileEnt[dcfListFileCount]);
                dcfListFileCount++;
            }
        }
#endif
        /* set tail entry of file list */
        if( dcfListFileEntHead != NULL)
        {
            dcfListFileEntTail = dcfListFileEntHead->prev;
            dcfListFileEntTail->next =dcfListFileEntHead;
#if 1
            dcfResetFileEntrySect(dcfListFileEntTail->pDirEnt->FileEntrySect,dcfListFileEntTail->pDirEnt->FileEntrySectOffset);
            DEBUG_DCF("====>dcfFileInit:%d,%d\n",dcfListFileEntTail->pDirEnt->FileEntrySect,dcfListFileEntTail->pDirEnt->FileEntrySectOffset);
#endif
        }
        else
        {
            dcfListFileEntTail =NULL;
        }

        return 1;
    }
#else
    if ((Type == CURRENTFILEENTRY) || (Type == PLAYFILEENTRY))
    {
        dcfResetFileEntrySect(0,0);

        dcfFileNumLast = 0; /* 0001 - 1 */
        dcfListFileEntHead = NULL;
        for (i = 0; i < DCF_MAX_MULTI_FILE; i++)
        {
            //dcfPlaybackCamHead[i] = NULL;
            global_totalCamfile_count[i] = 0;
        }
        dcfListFileEntTail = dcfPlaybackCurFile = NULL;
        dcfFileTypeCount_Clean();
        global_totalfile_count = 0;

        if (pListDirEnt == NULL)
        {
            DEBUG_DCF("Warning: Directory Empty\n");
            return 1;
        }
        /* set tail entry of file list */

        //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
        dcfFileCount=0; //civicwu 070926
        memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

#if ROOTWORK
        if (Type == CURRENTFILEENTRY)
        {
            if (dcfChDir("\\") == 0)
            {
                /* change root directory error */
                DEBUG_DCF("Error: Change directory root failed.\n");

                return 0;
            }
        }
        else
        {
            if (dcfChPlayDir("\\") == 0)
            {
                /* change root directory error */
                DEBUG_DCF("Error: Change directory root failed.\n");

                return 0;
            }

        }
#else
        //Albert Add
        //gsDirName
        sprintf ((char*)DirName, "\\%s", gsDirName);

        if (Type == CURRENTFILEENTRY)
        {
            if (dcfChDir(DirName) == 0)
            {
                /* change directory \DCIM error */
                DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

                return 0;
            }
        }
        else
        {
            if (dcfChPlayDir(DirName) == 0)
            {
                /* change directory \DCIM error */
                DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

                return 0;
            }
        }
#endif


        DEBUG_DCF("Directory Name: %s\n",pListDirEnt->pDirEnt->d_name);

        if (Type == CURRENTFILEENTRY)
        {
            /* change current directory */
            if (dcfChDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
            {
                /* change current directory error */
                DEBUG_DCF("Error: Change current directory failed.\n");

                return 0;
            }
        }
        else
        {
            /* change current directory */
            if (dcfChPlayDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
            {
                /* change current directory error */
                DEBUG_DCF("Error: Change current directory failed.\n");

                return 0;
            }
        }
#if 0
        //Lucian: 採用快速搜尋, 不對FDB 做排序. 可縮短開機時間. 只能用於封閉式系統.
        if (Type == CURRENTFILEENTRY)
        {
            /* list current directory */
            if (dcfDirScan(NULL, dcfFileEnt, &dcfFileCount,&OldestEntry,1) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
            {
                /* list current directory error */
                DEBUG_DCF("Error: List current directory failed.\n");
                return 0;
            }
        }
        else
        {
            /* list current directory */
            if (dcfPlayDirScan(NULL, dcfFileEnt, &dcfFileCount,&OldestEntry,1) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
            {
                /* list current directory error */
                DEBUG_DCF("Error: List current directory failed.\n");
                return 0;
            }
        }
        /* sort the file and create the file list */
        memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

        dcfListFileCount = dcfFileCount;
        global_totalfile_count = dcfFileCount;

        if(dcfListFileCount==0)
        {
            dcfListFileCount=NULL;
        }
        else
        {
            dcfListFileEntHead = &dcfListFileEnt[OldestEntry];

            ins = &dcfListFileEnt[0];
            ins->prev = &dcfListFileEnt[dcfFileCount-1];
            ins->next = &dcfListFileEnt[1];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfFileEnt[0];

            ins ++;

            for (i = 1; i < dcfFileCount-1; i++)
            {
                ins = &dcfListFileEnt[i];
                ins->prev = &dcfListFileEnt[i-1];
                ins->next = &dcfListFileEnt[i+1];

                ins->used_flag = DCF_FILE_USE_CLOSE;
                ins->pDirEnt = &dcfFileEnt[i];
                ins ++;
            }

            ins = &dcfListFileEnt[dcfFileCount-1];
            ins->prev = &dcfListFileEnt[dcfFileCount-2];
            ins->next = &dcfListFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfFileEnt[dcfFileCount-1];
        }
#else
        //Lucian:  一般收尋, 做檔名排序. 可做壞檔修復.
        /* list current directory */
        if (Type == CURRENTFILEENTRY)
        {
            if (dcfDir(NULL, dcfFileEnt, &dcfFileCount,1,1,DCF_FILEENT_MAX) == 0)
            {
                /* list current directory error */
                DEBUG_DCF("Error: List current directory failed.\n");

                return 0;
            }
        }
        else
        {
            if (dcfPlaybackDir(NULL, dcfFileEnt, &dcfFileCount,1,0) == 0)
            {
                /* list current directory error */
                DEBUG_DCF("Error: List current directory failed.\n");

                return 0;
            }
        }

        /* sort the file and create the file list */
        memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
        dcfListFileCount = 0;

        for (i = 0; i < dcfFileCount; i++)
        {
            if ((dcfFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
            {
                /* check if valid dcf file */
                if (CamerIdx = dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, Type) == 0)
                    continue;
#if FILE_REPAIR_AUTO //Repair Bad file
                if(fileType == 6) // AXF file
                {
                    DEBUG_SYS("Repair Bad File:%s\n",dcfFileEnt[i].d_name);
                    uiClearOSDBuf(2);
                    uiMenuOSDStringByColor(TVOSD_SizeX , "Repair Bad File:" , OSD_STRING_W , OSD_STRING_H , 48, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                    uiMenuOSDStringByColor(TVOSD_SizeX , dcfFileEnt[i].d_name , OSD_STRING_W , OSD_STRING_H , 176, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                    asfRepairFile(dcfFileEnt[i].d_name);
                    DEBUG_SYS("===Repair Complete===\n");

                    memcpy(NewName,dcfFileEnt[i].d_name,13);
                    NewName[10]='S';
                    if(0 == dcfRename(NewName,dcfFileEnt[i].d_name))
                    {
                        DEBUG_DCF("Warning! DCF Rename Fail!\n");
                    }
                    fileType=0;
                    dcfFileEnt[i].d_name[10]='S';
                }
#endif
                dcfFileTypeCount_Inc(fileType);
                if (CamerIdx != 0)
                    CamerIdx--;
                global_totalfile_count++;
                global_totalCamfile_count[CamerIdx]++;

                /* assign last file number */
                dcfFileNumLast = (dcfFileNumLast < fileNum) ? fileNum : dcfFileNumLast;

                /* insert file entry to the list */
                dcfListFileEnt[dcfListFileCount].used_flag = DCF_FILE_USE_CLOSE;
                dcfListFileEnt[dcfListFileCount].fileNum = fileNum;
                //DEBUG_DCF("fileNum=%d\n",fileNum);
                dcfListFileEnt[dcfListFileCount].fileType = fileType;
                dcfListFileEnt[dcfListFileCount].pDirEnt = &dcfFileEnt[i];
                dcfListFileEntInsert(&dcfListFileEnt[dcfListFileCount]);
                dcfListFileCount++;
            }
        }
#endif
        DEBUG_DCF("dcfFileNumLast = %u\n", dcfFileNumLast);
        /* set tail entry of file list */
        if( dcfListFileEntHead != NULL)
        {
            dcfListFileEntTail = dcfListFileEntHead->prev;
            dcfListFileEntTail->next =dcfListFileEntHead;
        }
        else
        {
            dcfListFileEntTail =NULL;
        }

        /* set current playback file */
        dcfPlaybackCurFile = dcfListFileEntTail;

        return 1;
    }
#endif
    else if ( Type == DELFILEENTRY)
    {
        //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
        dcfDelFileNumLast = 0; /* 0001 - 1 */
        dcfListDelFileEntHead = NULL;
        dcfListDelFileEntTail = NULL;

        BandWidthControl=1;
        if (pListDirEnt == NULL)
        {
            DEBUG_DCF("Warning: Directory Empty\n");
            return 1;
        }

        dcfDelFileCount=0; //civicwu 070926
        memset_hw((void*)dcfDelFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

#if ROOTWORK
        /* change root directory  */
        if (dcfChOWDelDir("\\") == 0)
        {
            /* change root directory error */
            DEBUG_DCF("Error: Change directory root failed.\n");
            return 0;
        }
#else
        sprintf ((char*)DirName, "\\%s", gsDirName);

        if (dcfChOWDelDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return 0;
        }
#endif
        /* change current directory */
        if (dcfChOWDelDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
        {
            /* change current directory error */
            DEBUG_DCF("Error: Change current directory failed.\n");

            return 0;
        }

        /* list current directory */
        if (dcfOWDelDirScan(NULL, dcfDelFileEnt, &dcfDelFileCount,&OldestEntry,0) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
        {
            /* list current directory error */
            DEBUG_DCF("Error: List current directory failed.\n");
            return 0;
        }
        /* sort the file and create the file list */
        memset_hw((void*)dcfListDelFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

        dcfListDelFileCount = dcfDelFileCount;

        if(dcfDelFileCount==0)
        {
            dcfListDelFileEntHead=NULL;
        }
        else if(dcfDelFileCount==1)
        {
            dcfListDelFileEntHead = &dcfListDelFileEnt[0];
            ins = &dcfListDelFileEnt[0];
            ins->prev = &dcfListDelFileEnt[0];
            ins->next = &dcfListDelFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfDelFileEnt[0];
        }
        else
        {
            dcfListDelFileEntHead = &dcfListDelFileEnt[OldestEntry];

            ins = &dcfListDelFileEnt[0];
            ins->prev = &dcfListDelFileEnt[dcfDelFileCount-1];
            ins->next = &dcfListDelFileEnt[1];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfDelFileEnt[0];

            ins ++;

            for (i = 1; i < dcfDelFileCount-1; i++)
            {
                ins = &dcfListDelFileEnt[i];
                ins->prev = &dcfListDelFileEnt[i-1];
                ins->next = &dcfListDelFileEnt[i+1];

                ins->used_flag = DCF_FILE_USE_CLOSE;
                ins->pDirEnt = &dcfDelFileEnt[i];
                ins ++;
            }

            ins = &dcfListDelFileEnt[dcfDelFileCount-1];
            ins->prev = &dcfListDelFileEnt[dcfDelFileCount-2];
            ins->next = &dcfListDelFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfDelFileEnt[dcfDelFileCount-1];
        }


        /* set tail entry of file list */
        if( dcfListDelFileEntHead != NULL)
        {
            dcfListDelFileEntTail = dcfListDelFileEntHead->prev;
            dcfListDelFileEntTail->next =dcfListDelFileEntHead;
        }
        else
        {
            dcfListDelFileEntTail =NULL;
        }
        return 1;

    }
#if CDVR_REC_WITH_PLAY_SUPPORT
    else if ( Type == PLAYFILEENTRY)
    {
        //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
        dcfListReadFileEntHead = NULL;
        dcfListReadFileEntTail = NULL;

        for (i = 0; i < DCF_MAX_MULTI_FILE; i++)
        {
            global_totalCamfile_count[i] = 0;
        }
        dcfPlaybackCurFile = NULL;
        dcfFileTypeCount_Clean();
        global_totalfile_count = 0;

        if (pListDirEnt == NULL)
        {
            DEBUG_DCF("Warning: Directory Empty\n");
            return 1;
        }

        dcfReadFileCount=0; //civicwu 070926
        memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

#if ROOTWORK
        /* change root directory  */
        if (dcfChPlayDir("\\") == 0)
        {
            /* change root directory error */
            DEBUG_DCF("Error: Change directory root failed.\n");

            return 0;
        }
#else
        //Albert Add
        //gsDirName
        sprintf ((char*)DirName, "\\%s", gsDirName);
        if (dcfChPlayDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return 0;
        }
#endif

        DEBUG_DCF("Directory Name: %s\n",pListDirEnt->pDirEnt->d_name);

        /* change current directory */
        if (dcfChPlayDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
        {
            /* change current directory error */
            DEBUG_DCF("Error: Change current directory failed.\n");
            return 0;
        }

        /* list current directory */
        if (dcfPlayDirScan(NULL, dcfReadFileEnt, &dcfReadFileCount,&OldestEntry,0) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
        {
            /* list current directory error */
            DEBUG_DCF("Error: List current directory failed.\n");
            return 0;
        }

        /* sort the file and create the file list */
        memset_hw((void*)dcfListReadFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

        dcfListReadFileCount = dcfReadFileCount;
        global_totalfile_count = dcfReadFileCount;

        if(dcfReadFileCount==0)
        {
            dcfListReadFileEntHead=NULL;
        }
        else if(dcfReadFileCount==1)
        {
            dcfListReadFileEntHead = &dcfListReadFileEnt[0];
            ins = &dcfListReadFileEnt[0];
            ins->prev = &dcfListReadFileEnt[0];
            ins->next = &dcfListReadFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[0];
        }
        else
        {
            dcfListReadFileEntHead = &dcfListReadFileEnt[OldestEntry];

            ins = &dcfListReadFileEnt[0];
            ins->prev = &dcfListReadFileEnt[dcfReadFileCount-1];
            ins->next = &dcfListReadFileEnt[1];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[0];

            ins ++;

            for (i = 1; i < dcfReadFileCount-1; i++)
            {
                ins = &dcfListReadFileEnt[i];
                ins->prev = &dcfListReadFileEnt[i-1];
                ins->next = &dcfListReadFileEnt[i+1];

                ins->used_flag = DCF_FILE_USE_CLOSE;
                ins->pDirEnt = &dcfReadFileEnt[i];
                ins ++;
            }

            ins = &dcfListReadFileEnt[dcfReadFileCount-1];
            ins->prev = &dcfListReadFileEnt[dcfReadFileCount-2];
            ins->next = &dcfListReadFileEnt[0];
            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[dcfReadFileCount-1];
        }


        /* set tail entry of file list */
        if( dcfListReadFileEntHead != NULL)
        {
            dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
            dcfListReadFileEntTail->next =dcfListReadFileEntHead;
        }
        else
        {
            dcfListReadFileEntTail =NULL;
        }

        /* set current playback file */
        dcfPlaybackCurFile = dcfListReadFileEntTail;

        return 1;

    }
    else if ( Type == SCANFILEENTRY)
    {
        //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
        dcfPlaybackCurFile = NULL;
        dcfReadFileCount=0;

        if (pListDirEnt == NULL)
        {
            DEBUG_DCF("Warning: Directory Empty\n");
            return 1;
        }

        memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

#if ROOTWORK
        /* change root directory  */
        if (dcfChPlayDir("\\") == 0)
        {
            /* change root directory error */
            DEBUG_DCF("Error: Change directory root failed.\n");
            return 0;
        }
#else
        sprintf ((char*)DirName, "\\%s", gsDirName);
        if (dcfChPlayDir(DirName) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

            return 0;
        }
#endif

        DEBUG_DCF("Directory Name: %s\n",pListDirEnt->pDirEnt->d_name);

        /* change current directory */
        if (dcfChPlayDir((s8*)pListDirEnt->pDirEnt->d_name) == 0)
        {
            /* change current directory error */
            DEBUG_DCF("Error: Change current directory failed.\n");
            return 0;
        }

        /* list current directory */
        if (dcfPlayDirScan(NULL, dcfReadFileEnt, &dcfReadFileCount,&OldestEntry,0) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
        {
            /* list current directory error */
            DEBUG_DCF("Error: List current directory failed.\n");
            return 0;
        }

        ChDistrInDir=0;
        global_totalfile_count = 0;
        for (i = 0; i < DCF_MAX_MULTI_FILE; i++)
        {
            global_totalCamfile_count[i] = 0;
        }
        dcfListReadFileCount = dcfReadFileCount;
        for(i=0; i<dcfReadFileCount; i++)
        {
            switch(dcfReadFileEnt[i].d_name[7])
            {
                //Lucian: 在此不check 副檔名,若有需要可增加.
                case '1':
                    global_totalCamfile_count[0] ++;
                    ChDistrInDir |= 0x01;
                    break;

                case '2':
                    global_totalCamfile_count[1] ++;
                    ChDistrInDir |= 0x02;
                    break;

                case '3':
                    global_totalCamfile_count[2] ++;
                    ChDistrInDir |= 0x04;
                    break;

                case '4':
                    global_totalCamfile_count[3] ++;
                    ChDistrInDir |= 0x08;
                    break;
            }
            global_totalfile_count ++;
        }

        return 1;

    }
#endif
    else
        return 0;

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
void dcfGetCurFileName (s8 *FileName)
{
    memcpy(FileName,dcfFileEnt[dcfFileCount].d_name,12);
}
void dcfSetCurFileName (s8 *FileName)
{
    memcpy(dcfFileEnt[dcfFileCount].d_name,FileName,12);
}
s32 dcfCheckDir(struct FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j,value;
    s8 dirNumStr[16];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".   ") != 0)
        return 0;

#if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)

#else
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
#endif

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

s32 dcfCheckFile(struct FS_DIRENT* pFileEnt, u32* pFileNum, u8* pFileType, u8 FileGroup)
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
s32 dcfIntToStr(u32 val, s8* str, s32 cnt)
{
    s8 i;
    s8 dig[16];

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

int dcfGetNewFileTime(u8 fileType, u8 index, char *pFileName, u32 fileNum, RTC_DATE_TIME *pLocalTime)
{
	RTC_Get_Time(pLocalTime);
#if (Get_sametime)

#if (HW_BOARD_OPTION == MR6730_AFN)
	if(sametime_Pair)
	{
		//sprintf(newFileName,"%02d%02d%02d-%d",(u32)localTime.hour, localTime.min, localTime.sec, (index));
		OS_ENTER_CRITICAL();
		memcpy(pLocalTime, &same_localTime, sizeof(RTC_Get_Time));
		sprintf(pFileName,"%02d%02d%02d-%d",(u32)same_localTime.hour, same_localTime.min, same_localTime.sec, (index));
		OS_EXIT_CRITICAL();
		fileNum=((u32)pLocalTime->hour)*1000000+((u32)pLocalTime->min)*10000+((u32)pLocalTime->sec)*100 + (index);
	}
	else
	{
		if(fileType == DCF_FILE_TYPE_JPG)
		{
			OS_ENTER_CRITICAL();
			memcpy(pLocalTime, &same_localTime, sizeof(RTC_Get_Time));
			sprintf(pFileName,"%02d%02d%02d-%d",(u32)same_localTime.hour, same_localTime.min, same_localTime.sec, (index));
			fileNum=((u32)same_localTime.hour)*1000000+((u32)same_localTime.min)*10000+((u32)same_localTime.sec)*100 + (index);
			OS_EXIT_CRITICAL();
		}
		else
		{
			sprintf(pFileName,"%02d%02d%02d-%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index));
			fileNum=((u32)pLocalTime->hour)*1000000+((u32)pLocalTime->min)*10000+((u32)pLocalTime->sec)*100 + (index);
		}
	}
#endif

#else

#if (HW_BOARD_OPTION == MR6730_AFN)

	sprintf(pFileName,"%02d%02d%02d-%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index));
	fileNum=((u32)pLocalTime->hour)*1000000+((u32)pLocalTime->min)*10000+((u32)pLocalTime->sec)*100 + (index);
	
#elif (UI_VERSION == UI_VERSION_TRANWO)

	if(uiCurRecType[index]== UI_REC_TYPE_MANUAL)
		sprintf(pFileName,"%02d%02d%02dM%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
	else if(uiCurRecType[index]== UI_REC_TYPE_MOTION)
		sprintf(pFileName,"%02d%02d%02dD%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
	else if(uiCurRecType[index]== UI_REC_TYPE_SCHEDULE)
		sprintf(pFileName,"%02d%02d%02dS%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
	else
		sprintf(pFileName,"%02d%02d%02dS%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
		
#elif ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))

#if ((UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM) || (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM) ||\
		(UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_DMY_APM))
	if (pLocalTime->hour > 12)
		sprintf(pFileName,"P%02d%02d%02d%d",(u32)(pLocalTime->hour-12), pLocalTime->min, pLocalTime->sec, (index+1));
	else if (pLocalTime->hour == 12)
		sprintf(pFileName,"P%02d%02d%02d%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
	else if (pLocalTime->hour == 0)
		sprintf(pFileName,"A%02d%02d%02d%d",(u32)pLocalTime->hour+12, pLocalTime->min, pLocalTime->sec, (index+1));
	else
		sprintf(pFileName,"A%02d%02d%02d%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
#else
	sprintf(pFileName,"%02d%02d%02d-%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
#endif

	fileNum=((u32)pLocalTime->hour)*1000000+((u32)pLocalTime->min)*10000+((u32)pLocalTime->sec)*100 + (index+1);
	
#else

	sprintf(pFileName,"%02d%02d%02d-%d",(u32)pLocalTime->hour, pLocalTime->min, pLocalTime->sec, (index+1));
	fileNum=((u32)pLocalTime->hour)*1000000+((u32)pLocalTime->min)*10000+((u32)pLocalTime->sec)*100 + (index+1);
	
#endif
	
#endif//#if (Get_sametime)
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
s32 dcfCreateNextDir(void)
{
	FS_DIR InDir;
    char newDirName[32], newTargetName[64];
    u32 LastdirNum;
    int scan;
#if (ROOTWORK==0)
    s8	DirName[32];
#endif
    RTC_DATE_TIME   localTime;
    INT8U err;
    int ret, count, datecode;
    //==============================================//

    DEBUG_DCF("\nEnter CDVR dcfCreateNextDir\n");

    /* list file newly created */
    scan=0;
    while(dcfDirEnt[dcfDirCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfDirCount ++;
        if(dcfDirCount >= DCF_DIRENT_MAX)
        {
            scan ++;
            dcfDirCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfDirEnt is full!\n");
            dcfDirCount=0;
            return 0;
        }
    }
    //dcfDirEnt[dcfDirCount].used_flag = DCF_FILE_USE_CREATE;

    scan=0;
    while(dcfListDirEnt[dcfListDirCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListDirCount ++;
        if(dcfListDirCount>=DCF_DIRENT_MAX)
        {
            scan ++;
            dcfListDirCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListDirEnt is full!\n");
            dcfListDirCount=0;
            return 0;
        }
    }
    //dcfListDirEnt[dcfListDirCount].used_flag = DCF_FILE_USE_CREATE;

    DEBUG_DCF("dcfDirCount: %d\n",dcfDirCount);
    DEBUG_DCF("dcfListDirCount: %d\n",dcfListDirCount);

#if 1 //CDVR_REC_WITH_PLAY_SUPPORT
    count=0;
    dcfDirNumLast++;
    while (1)
    {
        dcfIntToStr(dcfDirNumLast, newDirName, 8);
        strcat((char*)newDirName, ".   ");

#if ROOTWORK
        strcpy((char*)newTargetName, "\\");
        strcat((char*)newTargetName, (const char*)newDirName);
#else
        strcpy((char*)newTargetName, "\\");
        strcat((char*)newTargetName, (char*)gsDirName);
        strcat((char*)newTargetName, "\\");
        strcat((char*)newTargetName, (const char*)newDirName);
#endif

        DEBUG_DCF("Dir name: %s \n",newTargetName);
        if((ret = dcfMkDir(newTargetName)) == 0x0)
            DEBUG_DCF("[WARM] DCF Mkdir error: %#x\n", ret);
        if((ret = dcfCheckDirExist(newTargetName)) == 0x1)
        	break;
        DEBUG_DCF("[WARM] DCF dir mkdir error: %#x\n", ret);
        count++;
        if(count >= 3)
            return 0;
        DEBUG_DCF("[ERR] DCF make dir %s Failed, Retry: %d.\n", newTargetName, count);
        if(dcfCheckUnit()==0)
            return 0;
    }
#else
    RTC_Get_Time(&localTime);

#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    sprintf(newDirName,"%02d%02d%04d.   \0", localTime.month, localTime.day, (u32)localTime.year+2000);
#else
    sprintf(newDirName,"%04d%02d%02d.   \0",(u32)localTime.year+2000, localTime.month, localTime.day);
#endif
    LastdirNum=((u32)localTime.year+2000)*10000+((u32)localTime.month)*100+localTime.day;

#if ROOTWORK
    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newDirName);
#else
    strcpy((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (char*)gsDirName);
    strcat((char*)newTargetName, "\\");
    strcat((char*)newTargetName, (const char*)newDirName);
#endif
    DEBUG_DCF("Dir name: %s \n",newTargetName);

    if (dcfMkDir((s8 *)newTargetName)==0)
    {
        DEBUG_DCF("dcfMkDir %s Failed \n",newTargetName);
        return 0;
    }
    dcfDirNumLast = LastdirNum;

#endif

    // list directory newly created
    // using dcfDirCount(only increment), for delete(cannot decrement))

#if ROOTWORK
    if (dcfGetDirEnt("\\", &dcfDirEnt[dcfDirCount], (s8 *)newDirName) == 0)
    {
        /* list directory error */
        DEBUG_DCF("Error: Find directory entry of %s failed.\n", newDirName);
        return 0;
    }

#else
    //Albert Add
    //gsDirName
    sprintf ((char*)DirName, "\\%s", gsDirName);

    if (dcfGetDirEnt(DirName, &dcfDirEnt[dcfDirCount], (s8 *)newDirName) == 0)
    {
        /* list directory error */
        DEBUG_DCF("Error: Find directory entry of %s failed.\n", newDirName);
        return 0;
    }

#endif


//    //Lucian: 增加direct FDB entry. 增加至4096 entry.
    dcfIncDirEnt((s8 *)newTargetName,DCF_FILEENT_MAX);
    /* insert directory entry to the list */
    dcfDirEnt[dcfDirCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListDirEnt[dcfListDirCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[dcfDirCount];
    dcfListDirEnt[dcfListDirCount].dirNum = dcfDirNumLast;
    dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);

    dcfListDirEntTail = dcfListDirEntHead->prev;

    DEBUG_DCF("Directory Name: %s\n",dcfListDirEntTail->pDirEnt->d_name);

    //Add on 20091208, for the playback of CDVR File system
    global_totaldir_count ++;

    /* set current recording directory */
    //dcfPlaybackCurDir = dcfListDirEntTail;
    dcfRecCurDir = &dcfListDirEnt[dcfListDirCount];
    DEBUG_DCF("Leave CDVR dcfCreateNextDir\n\n");

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
FS_FILE* dcfCreateNextFile(u8 fileType, u8 index)
{
    char newFileName[32];
    u32 fileNum,dirNum;
    int scan;
    FS_FILE* pFile;
    u16 val;
    RTC_DATE_TIME   localTime;
#if (ROOTWORK==0)
    s8  DirName[32];
#endif
    u8 count, err;
    int ForceCreatNewDir;

#if (HW_BOARD_OPTION == MR6730_AFN)
#if (OS_CRITICAL_METHOD == 3)				  /* Allocate storage for CPU status register			*/
    unsigned int	  cpu_sr = 0;				  /* Prevent compiler warning							*/
#endif
#endif
    //----------------------------------//

    ForceCreatNewDir=0;
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF("\nEnter CDVR dcfCreateNextFile\n");
#if 1 //CDVR_REC_WITH_PLAY_SUPPORT
#if (CDVR_LOG || CDVR_TEST_LOG)
    if (dcfTotalFileCntInDir > (DCF_FILE_PER_DIR/2-20))
    {
        DEBUG_DCF("file number is over: %d\n",dcfTotalFileCntInDir);
        ForceCreatNewDir=1;
    }
#else
    if (dcfTotalFileCntInDir > (DCF_FILE_PER_DIR-20))
    {
        DEBUG_DCF("file number is over: %d\n",dcfTotalFileCntInDir);
        ForceCreatNewDir=1;
    }
#endif
#else
#if (CDVR_LOG || CDVR_TEST_LOG)
    if (global_totalfile_count > (DCF_FILE_PER_DIR/2-20))
    {
        DEBUG_DCF("file number is over: %d\n",global_totalfile_count);
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }
#else
    if (global_totalfile_count > (DCF_FILE_PER_DIR-20))
    {
        DEBUG_DCF("file number is over: %d\n",global_totalfile_count);
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }
#endif
#endif

    if (index >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfCreateNextFile Error Index %d\n", index);
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

	dcfGetNewFileTime(fileType, index, newFileName, fileNum, &localTime);
	val = (((localTime.year + 20) & 0x7F) << 9) |((localTime.month & 0xF) << 5) |((localTime.day) & 0x1F);

#if 1 //CDVR_REC_WITH_PLAY_SUPPORT
    if ((dcfListDirEntTail==NULL)  || (dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD !=val) || (ForceCreatNewDir==1) )
#else
    if ((dcfListDirEntTail==NULL) || (dirNum != dcfListDirEntTail->dirNum))
#endif
    {
#if 1  //for Debug!!
        if (dcfListDirEntTail==NULL)
            DEBUG_DCF("Change Dircetory 11 \n");
        else if ((dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD !=val))
            DEBUG_DCF("Change Dircetory 33,fsFileCreateDate_YMD= 0x%x, val= 0x%x \n", dcfListDirEntTail->pDirEnt->fsFileCreateDate_YMD, val);
#endif

        DEBUG_DCF("Change Dircetory \n");

        if (dcfCreateNextDir())
        {
        	dcfDirRescanSwitch = 1;
            dcfResetFileEntrySect(0,0);
            dcfFileNumLast = 0; /* 0001 - 1 */
            dcfListFileEntHead = NULL;
            dcfListFileEntTail = dcfPlaybackCurFile = NULL;
            dcfFileCount=0;
            dcfTotalFileCntInDir=0;
            dcfListFileCount=0;
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
            memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926
            memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

#if ROOTWORK
            /* change root directory  */
            if (dcfChDir("\\") == 0)
            {
                /* change root directory error */
                DEBUG_DCF("Error: Change directory root failed.\n");
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }
#else
            //Albert Add
            //gsDirName
            sprintf ((char*)DirName, "\\%s", gsDirName);

            if (dcfChDir(DirName) == 0)
            {
                /* change directory \DCIM error */
                DEBUG_DCF("Error: Change directory %s failed.\n",DirName);
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }
#endif


            DEBUG_DCF("Directory Name: %s\n",dcfRecCurDir->pDirEnt->d_name);

            /* change current directory */
            if (dcfChDir((s8*)dcfRecCurDir->pDirEnt->d_name) == 0)
            {
                /* change current directory error */
                DEBUG_DCF("Error: Change current directory failed.\n");
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }

        }
        else
        {
            DEBUG_DCF("===dcfCreateNextDir Error!!===\n");
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
    }
    else
    {
    }

    dcfFileNumLast = fileNum;

    //DEBUG_DCF("File name: %s \n",newFileName);

    //------可在此做分類檔案-------//
    //strcpy((char*)newFileName, DCF_FILE_PREFIX);
    //dcfIntToStr(dcfFileNumLast, (s8 *)&newFileName[0], 8);

    //-----------------------------//
    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        DEBUG_DCF("Error: fileType=%d does not exist\n", fileType);
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return NULL;
    }

	count = 0;
    do
    {
	    strcat((char*)newFileName, ".");
	    strcat((char*)newFileName, gdcfFileType_Info[fileType].pSubFileStr);

	    DEBUG_DCF("File name: %s\\%s\n", dcfRecCurDir->pDirEnt->d_name, newFileName);
	    if(pFile = dcfOpen((s8 *)newFileName, "w-"))
	    	break;
	    DEBUG_DCF("[WARM] DCF create file retry: %d\n", ++count);
	    if(!(count < 3))
	    {
	        DEBUG_DCF("Error: create next file %s error\n", newFileName);
	        OSSemPost(dcfReadySemEvt);
	        return NULL;
	    }

		// Fetch new file name
	    dcfGetNewFileTime(fileType, index, newFileName, fileNum, &localTime);
	}while(pFile == NULL);

#if !FS_NEW_VERSION
	DEBUG_DCF("Over Write create Name: %s\n", newFileName);
    DEBUG_DCF("Over Write Head Name: %s\n", dcfListFileEntHead->pDirEnt->d_name);
#endif

    /* list file newly created */
    scan=0;
    while(dcfFileEnt[dcfFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfFileCount ++;
        if(dcfFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfFileBuf is full!\n");
            dcfFileCount=0;
            break;
        }
    }
    dcfFileEnt[dcfFileCount].used_flag = DCF_FILE_USE_CREATE;
    dcfTotalFileCntInDir ++;

    scan=0;
    while(dcfListFileEnt[dcfListFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListFileCount ++;
        if(dcfListFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfListFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListFileBuf is full!\n");
            dcfListFileCount=0;
            break;
        }
    }
    dcfListFileEnt[dcfListFileCount].used_flag = DCF_FILE_USE_CREATE;

    DEBUG_DCF("dcfTotalFileCntInDir: %d\n",dcfTotalFileCntInDir);
    //DEBUG_ASF("dcfFileCount: %d\n",dcfFileCount);
    //DEBUG_ASF("dcfListFileCount: %d\n",dcfListFileCount);

#if 0
    if (dcfGetDirEnt(NULL, &dcfFileEnt[dcfFileCount], newFileName) == 0)
    {
        /* list file error */
        DEBUG_DCF("Error: Find file entry of %s failed.\n", newFileName);

        return NULL;
    }
#else


#if (Get_sametime)
#if (HW_BOARD_OPTION == MR6730_AFN)


    OS_ENTER_CRITICAL();
    if(sametime_Pair)
    {
        dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
        dcfFileEnt[dcfFileCount].d_ino = pFile->fileid_hi;
        strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

        dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

        //RTC_Get_Time(&same_localTime);
        val=((same_localTime.hour& 0x1F )<<11) |((same_localTime.min  & 0x3F)<< 5) |((same_localTime.sec/2) & 0x1F);

        dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
        dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

        val=(((same_localTime.year+ 20)& 0x7F) <<9) |((same_localTime.month & 0xF)<< 5) |((same_localTime.day) & 0x1F);

        dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
        dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;
    }
    else
    {
        dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
        dcfFileEnt[dcfFileCount].d_ino = pFile->fileid_hi;
        strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

        dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

        RTC_Get_Time(&localTime);
        val=((localTime.hour& 0x1F )<<11) |((localTime.min	& 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

        dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
        dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

        val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

        dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
        dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;

    }//if(sametime_Pair)
    OS_EXIT_CRITICAL();


#else


    dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
    dcfFileEnt[dcfFileCount].d_ino = pFile->fileid_hi;
    strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

    dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

    //RTC_Get_Time(&same_localTime);
    val=((same_localTime.hour& 0x1F )<<11) |((same_localTime.min  & 0x3F)<< 5) |((same_localTime.sec/2) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
    dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

    val=(((same_localTime.year+ 20)& 0x7F) <<9) |((same_localTime.month & 0xF)<< 5) |((same_localTime.day) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
    dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;
#endif


#else

    dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
    dcfFileEnt[dcfFileCount].d_ino = pFile->fileid_hi;
    strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

    dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

    RTC_Get_Time(&localTime);
    val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
    dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
    dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;

#endif//#if (Get_sametime)
#endif

    /* insert directory entry to the list */
    // move to dcfclose
    gfileType=fileType;
    dcfFileInfoTab[index].dcfFileInfoCount = dcfFileCount;
    dcfFileInfoTab[index].dcfFileInfoListCount = dcfListFileCount;
    dcfFileInfoTab[index].dcfFileInfoNumLast = dcfFileNumLast;
    dcfFileInfoTab[index].dcfFileInfoFileType = fileType;
    dcfInsertFileNode_ToTail(&dcfFileInfoTab[index]);
    DEBUG_DCF("Leave CDVR dcfCreateNextFile\n\n");
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);

    return pFile;
}

#if FS_NEW_VERSION
s32 dcfOverWriteDel(void)
{
	int ret;
	u8 err;

	if(dcfListDirEntHead == NULL)
	{	
		DEBUG_DCF("[ERR] DCF Head of Dir entrys is NULL.\n");
		return 0;
	}

	OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
	DEBUG_DCF("Enter CDVR dcfOverWriteDel.\n");

	if(dcfListDelDirEnt != dcfListDirEntHead)
		dcfListDelDirEnt = dcfListDirEntHead;

	if((ret = dcfOWDel(dcfListDelDirEnt->pDirEnt->d_name)) < 0)
	{
		DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
		OSSemPost(dcfReadySemEvt);
		return 0;
	}

	if(ret == 0)
	{
		DEBUG_DCF("[INF] Directory is empty.\n");
		if((ret = dcfOWDelDir(dcfListDelDirEnt->pDirEnt->d_name)) < 0)
		{
			DEBUG_DCF("[ERR] DCF delete dir fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
			OSSemPost(dcfReadySemEvt);
			return 0;
		}

		global_totaldir_count--;

		// next directory
        dcfListDirEntHead = dcfListDirEntHead->next;
        dcfListDirEntTail->next = dcfListDirEntHead;
        dcfListDirEntHead->prev = dcfListDirEntTail;

        dcfListDelDirEnt = dcfListDirEntHead;

        DEBUG_DCF("[INF] DCF switch del dir to %s.\n", dcfListDelDirEnt->pDirEnt->d_name);
        
        // Start rescan in next playback mode
        dcfDirRescanSwitch = 1;
	}

	DEBUG_DCF("Leave CDVR dcfOverWriteDel.\n");
	OSSemPost(dcfReadySemEvt);
	return 1;
}
#else
s32 dcfOverWriteDel(void)
{
    char *pExtName;
    char extName[5];
    char j;
    u8 ucfiletype, backupCurDir[64];
    u8 err;
    int ret;
    static int RunOn=0;
#if (CDVR_LOG || CDVR_TEST_LOG)
    char LogFileName[32];
#if CDVR_LOG_RESERVE_ENA
    char newname[16];
    int status;
#endif
#endif
    //================================//

    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (err != OS_NO_ERR)
    {
        DEBUG_DCF("dcfOverWriteDel Pend error!\n");
        RunOn=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;    // error
    }

    DEBUG_DCF("Enter CDVR dcfOverWriteDel....\n");
    //DEBUG_DCF("gDcfFlagGrp=0x%x\n",gDcfFlagGrp->OSFlagFlags);
    if(RunOn)
    {
        DEBUG_DCF("===>Warning!! dcfOverWriteDel is overrun!!\n");
    }
    RunOn=1;
    //DEBUG_DCF("dcfOverWriteDel 00: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
    //DEBUG_DCF("dcfOverWriteDel 00 : dcfListDelFileEntHead= %X, dcfListDelFileEntTail= %X \n",dcfListDelFileEntHead, dcfListDelFileEntTail);

    //case 1:  del file in the different directory (with current directory)
    if (dcfListDirEntHead != dcfListDirEntTail)
    {
        //more one directory: ------Delete 不同天檔案------

        strcpy((char*)backupCurDir, (const char*)dcfCurDir);

        //case 1:  	del file in the different directory (with current directory)
        //case 1.1 : 	First dcfOverWriteDel => dcfListDelFileEntHead = NULL
        //			then record the files of the dcfListDirEntHead directory
        //
        //			if dcfListDirEntHead still empty, then delete this Directory and re-scan next directory
        //			otherwise, delete dcfListDelFileEntHead file

        do
        {
            //DEBUG_DCF("dcfOverWriteDel 11: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
            //check if the newest directory exist
            if (dcfListDirEntHead == dcfListDirEntTail)
            {
                break;
            }
            if(dcfListDelFileEntHead == NULL)
            {
                if (dcfFileInit(dcfListDirEntHead, DELFILEENTRY) == 0)
                {
                    DEBUG_DCF("Error: dcfFileInit fail: %s\n",dcfListDelFileEntHead->pDirEnt->d_name);
                    dcfListDelDirEnt = NULL;
                    dcfListDelFileEntHead=NULL;
                    dcfListDelFileEntTail=NULL;
                    RunOn=0;
                    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                    OSSemPost(dcfReadySemEvt);
                    return 0;
                }
                dcfListDelDirEnt = dcfListDirEntHead;
            }

            //
            if (dcfListDelFileEntHead==NULL)  //Lucian: 判斷為空目錄時, delete directory.
            {
                /* delete directory */
                dcfChOWDelDir("..");
#if ( (CDVR_LOG || CDVR_TEST_LOG) && CDVR_LOG_RESERVE_ENA)
                memcpy(newname,dcfListDirEntHead->pDirEnt->d_name,16);
                newname[9] ='L';
                newname[10]='O';
                newname[11]='G';
                status=dcfOWDelRename(newname,(s8*)dcfListDirEntHead->pDirEnt->d_name);
                if(status>0)
                    DEBUG_DCF("Directory Rename Success!\n");
                else
                    DEBUG_DCF("Directory Rename Fail!\n");
#else
                DEBUG_DCF("remove Dir : %s\n",dcfListDirEntHead->pDirEnt->d_name);
                dcfOWDelRmDir((s8*)dcfListDirEntHead->pDirEnt->d_name,1);
#endif

                global_totaldir_count--;

                /* next directory */
                dcfListDirEntHead = dcfListDirEntHead->next;
                dcfListDirEntTail->next = dcfListDirEntHead;
                dcfListDirEntHead->prev = dcfListDirEntTail;

                dcfListDelFileEntHead=NULL;
                dcfListDelFileEntTail=NULL;
            }
        }
        while(dcfListDelFileEntHead==NULL);

        //DEBUG_DCF("dcfOverWriteDel 22: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);


        if (dcfListDirEntHead != dcfListDirEntTail)
        {
            //DEBUG_DCF("dcfOverWriteDel 66: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
            // find number minum
            dcfListDelFileEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
            dcfListDelFileEntHead->used_flag = DCF_FILE_USE_NONE;
            DEBUG_DCF("Over Write delete Dir: %s, Name: %s\n",dcfListDirEntHead->pDirEnt->d_name, dcfListDelFileEntHead->pDirEnt->d_name);

            dcfChOWDelDir("..");
            if (dcfChOWDelDir((s8 *)dcfListDirEntHead->pDirEnt->d_name)== 0)
            {
                DEBUG_DCF("Error: change directory  failed.\n");
                RunOn=0;
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }

            if ((ret = dcfOWDel((s8*)dcfListDelFileEntHead->pDirEnt->d_name,dcfListDelFileEntHead->pDirEnt->FileEntrySect)) <= 0)
            {
                DEBUG_ASF("Error: delete file delete failed %s\nError code: %d\n", dcfListDelFileEntHead->pDirEnt->d_name, ret);
                switch(ret)
                {
                    case FS_FILE_ENT_FIND_ERROR:
                        if(dcfListDelFileEntHead != dcfListDelFileEntTail)
                        {
                            dcfListDelFileEntHead=dcfListDelFileEntHead->next;
                            dcfListDelFileEntTail->next =dcfListDelFileEntHead;
                            dcfListDelFileEntHead->prev=dcfListDelFileEntTail;
                        }
                        else
                        {
                            dcfListDelFileEntHead = dcfListDelFileEntTail = NULL;

                            // delete directory
                            // should not check if delete directory is failed, because the directory still exist some user's file
                            dcfChOWDelDir("..");
#if ( (CDVR_LOG || CDVR_TEST_LOG) && CDVR_LOG_RESERVE_ENA)
                            memcpy(newname,dcfListDirEntHead->pDirEnt->d_name,16);
                            newname[9] ='L';
                            newname[10]='O';
                            newname[11]='G';
                            status=dcfOWDelRename(newname,(s8*)dcfListDirEntHead->pDirEnt->d_name);
                            if(status>0)
                                DEBUG_DCF("Directory Rename Success!\n");
                            else
                                DEBUG_DCF("Directory Rename Fail!\n");
#else
                            DEBUG_DCF("remove Dir : %s\n",dcfListDirEntHead->pDirEnt->d_name);
                            dcfOWDelRmDir((s8*)dcfListDirEntHead->pDirEnt->d_name,0); //Lucian: 為加速,不做空目錄確認. User 需確認為空目錄.
#endif

                            // next directory
                            global_totaldir_count--;
                            dcfListDirEntHead = dcfListDirEntHead->next;
                            dcfListDirEntTail->next = dcfListDirEntHead;
                            dcfListDirEntHead->prev = dcfListDirEntTail;

                            if (dcfChOWDelDir((s8 *)dcfListDirEntHead->pDirEnt->d_name)== 0)
                            {
                                DEBUG_DCF("Error2: change directory  failed.\n");
                                RunOn=0;
                                OSSemPost(dcfReadySemEvt);
                                return 0;
                            }
                            dcfListDelFileEntHead = dcfListDelFileEntTail = NULL;
                        }
                        break;
                    default:
                        break;
                }
                RunOn=0;
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }
#if (CDVR_LOG || CDVR_TEST_LOG)
#if CDVR_LOG_RESERVE_ENA
            //Lucian: 保留 Log file.
#else
            strncpy(LogFileName, (s8*)dcfListDelFileEntHead->pDirEnt->d_name,9);
            LogFileName[9]  = 0;
            strcat(LogFileName, "LOG");

            if ((ret = dcfOWDel((s8*)LogFileName,dcfListDelFileEntHead->pDirEnt->FileEntrySect)) <= 0)
            {
                DEBUG_DCF("log File: %s not found \nError code: %d\n",LogFileName, ret);
                //fix me =>Albert
                //return 0;
            }
#endif
#endif

            if(dcfListDelFileEntHead != dcfListDelFileEntTail)
            {
                //DEBUG_DCF("dcfOverWriteDel 77: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
                dcfListDelFileEntHead=dcfListDelFileEntHead->next;
                dcfListDelFileEntTail->next =dcfListDelFileEntHead;
                dcfListDelFileEntHead->prev=dcfListDelFileEntTail;
                //DEBUG_DCF("dcfOverWriteDel 77: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead->dirNum, dcfListDirEntTail->dirNum);
            }
            else
            {
                //DEBUG_DCF("dcfOverWriteDel 88: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
                dcfListDelFileEntHead = dcfListDelFileEntTail = NULL;
                //DEBUG_DCF("dcfOverWriteDel 88: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);

                // delete directory
                // should not check if delete directory is failed, because the directory still exist some user's file
                dcfChOWDelDir("..");

#if ( (CDVR_LOG || CDVR_TEST_LOG) && CDVR_LOG_RESERVE_ENA)
                memcpy(newname,dcfListDirEntHead->pDirEnt->d_name,16);
                newname[9] ='L';
                newname[10]='O';
                newname[11]='G';
                status=dcfOWDelRename(newname,(s8*)dcfListDirEntHead->pDirEnt->d_name);
                if(status>0)
                    DEBUG_DCF("Directory Rename Success!\n");
                else
                    DEBUG_DCF("Directory Rename Fail!\n");
#else
                DEBUG_DCF("remove Dir : %s\n",dcfListDirEntHead->pDirEnt->d_name);
                dcfOWDelRmDir((s8*)dcfListDirEntHead->pDirEnt->d_name,0); //Lucian: 為加速,不做空目錄確認. User 需確認為空目錄.
#endif

                global_totaldir_count--;

                /* next directory */
                dcfListDirEntHead = dcfListDirEntHead->next;
                dcfListDirEntTail->next = dcfListDirEntHead;
                dcfListDirEntHead->prev = dcfListDirEntTail;

                if (dcfChOWDelDir((s8 *)dcfListDirEntHead->pDirEnt->d_name)== 0)
                {
                    DEBUG_DCF("Error2: change directory  failed.\n");
                    RunOn=0;
                    OSSemPost(dcfReadySemEvt);
                    return 0;
                }

                //DEBUG_DCF("dcfOverWriteDel 881: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);

                dcfListDelFileEntHead=NULL;
                dcfListDelFileEntTail=NULL;
				// Start rescan in next playback mode
                dcfDirRescanSwitch = 1;
            }

            strcpy((char*)dcfDelDir, (const char*)backupCurDir);
            dcfCreateOWDelCurPath();

            //DEBUG_DCF("gDcfFlagGrp=0x%x\n",gDcfFlagGrp->OSFlagFlags);
            DEBUG_DCF("Leave CDVR dcfOverWriteDel!!!\n\n");
            RunOn=0;
            //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
            OSSemPost(dcfReadySemEvt);
            return 1;
        }
        else
        {
            strcpy((char*)dcfDelDir, (char*)dcfCurDir);
            dcfChOWDelDir("..");
            if (dcfChOWDelDir((s8*)dcfListDirEntTail->pDirEnt->d_name) == 0)
            {
                /* change current directory error */
                DEBUG_DCF("Error: Change current directory failed.\n");
                RunOn=0;
                //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
                OSSemPost(dcfReadySemEvt);
                return 0;
            }

        }

    }

    //----------Delete 同一天檔案-----------//
    //DEBUG_DCF("dcfOverWriteDel 33: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
#if 0//CDVR_REC_WITH_PLAY_SUPPORT
    //Lucian: 為避免 PlayDel,Rec,OWDel在同一個目錄而產生問題, 此時Overwrite delete 不做任何事,等到不同目錄時才做delete的動作.
    OSFlagPost(gDcfFlagGrp, waitFlag, OS_FLAG_CLR, &err);
    DEBUG_DCF("Leave CDVR dcfOverWriteDel: Do Nothing! \n");
    return 1;
#endif

    //only one directory
    if (dcfListFileEntHead == NULL)
    {
        DEBUG_DCF("Trace: No current file.\n");
        RunOn=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];

    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        DEBUG_ASF("Over Write delete Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
        DEBUG_ASF("Error: file type doesn't exist, %d\n", ucfiletype);
        RunOn=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;

    // find number minum
    dcfListFileEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfListFileEntHead->used_flag = DCF_FILE_USE_NONE;
    DEBUG_ASF("Over Write delete Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
    strcpy((char*)dcfDelDir, (char*)dcfCurDir);
    if ((ret = dcfOWDel((s8*)dcfListFileEntHead->pDirEnt->d_name,dcfListFileEntHead->pDirEnt->FileEntrySect)) <= 0)
    {
        DEBUG_ASF("Error: file delete fail %s\nError code: %d\n", dcfListFileEntHead->pDirEnt->d_name, ret);
        switch(ret)
        {
            case FS_FILE_ENT_FIND_ERROR:
                if(dcfListFileEntHead != dcfListFileEntTail)
                {
                    dcfListFileEntHead=dcfListFileEntHead->next;
                    dcfListFileEntTail->next =dcfListFileEntHead;
                    dcfListFileEntHead->prev=dcfListFileEntTail;
                }
                else
                    dcfListFileEntHead = dcfListFileEntTail = NULL;
                break;
            default:
                break;
        }
        RunOn=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

#if (CDVR_LOG || CDVR_TEST_LOG)
#if CDVR_LOG_RESERVE_ENA
    //Lucian: 保留 Log file.
#else
    strncpy(LogFileName, (s8*)dcfListFileEntHead->pDirEnt->d_name,9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if ((ret = dcfOWDel((s8*)LogFileName,dcfListFileEntHead->pDirEnt->FileEntrySect)) <= 0)
    {
        DEBUG_DCF("log File: %s not found\nError code: %d\n", LogFileName, ret);
        //fix me =>Albert
        //return 0;
    }
#endif
#endif
    //dcfListDelFileCount--;
    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
        dcfListFileEntHead=dcfListFileEntHead->next;
        dcfListFileEntTail->next =dcfListFileEntHead;
        dcfListFileEntHead->prev=dcfListFileEntTail;
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead->dirNum, dcfListDirEntTail->dirNum);
    }
    else
    {
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
        dcfListFileEntHead = dcfListFileEntTail = NULL;
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListDirEntHead= %X, dcfListDirEntTail= %X \n",dcfListDirEntHead, dcfListDirEntTail);
    }

    //DEBUG_DCF("gDcfFlagGrp=0x%x\n",gDcfFlagGrp->OSFlagFlags);
    DEBUG_DCF("Leave CDVR dcfOverWriteDel\n");
    RunOn=0;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
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
/***********************************************
Routine Description:
    將新增檔加入檔案鏈結.

Arguments:
	None.

Return Value:
	None
************************************************/


//for the close file under  recording


void dcfInsertFileNode_ToTail(DCF_FILE_INFO* File)
{
    INT8U   err;

    dcfFileEnt[File->dcfFileInfoCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].pDirEnt = &dcfFileEnt[File->dcfFileInfoCount];
    dcfListFileEnt[File->dcfFileInfoListCount].fileNum = File->dcfFileInfoNumLast;
    dcfListFileEnt[File->dcfFileInfoListCount].fileType = File->dcfFileInfoFileType;
    dcfListFileEntInsert_ToTail(&dcfListFileEnt[File->dcfFileInfoListCount]);
    dcfListFileEntTail = dcfListFileEntHead->prev;
    //dcfListFileEntTail = &dcfListFileEnt[dcfListFileCount];

    /* set current playback file */
    //dcfPlaybackCurFile=dcfListFileEntTail;

    dcfFileTypeCount_Inc(File->dcfFileInfoFileType);
    global_totalfile_count++;
}
/*

Routine Description:

	Insert file entry.

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
#if !FS_NEW_VERSION
s32 dcfListDelFileEntInsert(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListDelFileCount == 0)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListDelFileEntHead = ins;
    }
    else
    {
        cur = dcfListDelFileEntHead;
        do
        {
            if (ins->fileNum < cur->fileNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListDelFileEntHead)
                {
                    /* insert before head */
                    dcfListDelFileEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListDelFileEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}
#endif

s32 dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile)
{
    s32 rel;
    INT8U   err;

    if (idx >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfClose Error Index %d\n",idx);
        return 0;
    }
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF("\ndcfCloseFileByIdx Index %d\n",idx);
    rel = dcfClose(pFile, pOpenFile);
    //dcfInsertFileNode_ToTail(&dcfFileInfoTab[idx]);  //Lucian: 移到creat file 去做.
    dcfNewFile = 1;
    DEBUG_DCF("dcfCloseFileByIdx idx=%d, OSFlagPost Close %d\n\n", idx, rel);
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    return rel;
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
#if FS_NEW_VERSION
s32 dcfPlaybackDelDir(void)
{
	int ret;
	u8 err;
	
	DEBUG_DCF("dcfPlaybackDelDir Begin\n");
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
			OSSemPost(dcfReadySemEvt);
			return 0;
		}

		if(ret == 0)
		{
			DEBUG_DCF("[INF] Directory is empty.\n");
			if((ret = dcfOWDelDir(dcfPlaybackCurDir->pDirEnt->d_name)) < 0)
			{
				DEBUG_DCF("[ERR] DCF delete dir fail. %s\n", dcfPlaybackCurDir->pDirEnt->d_name);
				OSSemPost(dcfReadySemEvt);
				return 0;
			}

			global_totaldir_count--;

			if(dcfListDirEntHead == dcfListDirEntTail)
			{
				dcfPlaybackCurDir = dcfListDirEntHead = dcfListDirEntTail = NULL;
			}
			else
			{
				// Link
				dcfPlaybackCurDir->prev->next = dcfPlaybackCurDir->next;
	        	dcfPlaybackCurDir->next->prev = dcfPlaybackCurDir->prev;
	        	if (dcfPlaybackCurDir == dcfListDirEntHead)
		            dcfListDirEntHead = dcfPlaybackCurDir->next;
		        dcfListDirEntTail = dcfListDirEntHead->prev;
		        dcfPlaybackCurDir = NULL;
			}
			
			OSSemPost(dcfReadySemEvt);
		    DEBUG_DCF("Leave dcfPlaybackDelDir\n");
		    return 1;
		}
		OSSemPost(dcfReadySemEvt);
	}while(1);
}
#else
s32 dcfPlaybackDelDir(void)
{
    DCF_LIST_FILEENT* curFile;
    INT8U err;

    DEBUG_DCF("dcfPlaybackDelDir Begin\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfPlaybackCurDir == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current Dir.\n");
        return 0;
    }
    else
    {
        if(dcfScanFileOnPlaybackDir() == 0)
        {
            DEBUG_DCF("DelDir do dcfScanFileOnPlaybackDir Fail.\n");
        }
#if CDVR_REC_WITH_PLAY_SUPPORT
        if(dcfListReadFileEntHead != NULL)
        {
            curFile = dcfListReadFileEntHead;
            /* check if null */
            if (curFile)
            {
                /* forward traverse whole files */
                do
                {
                    /* delete file */
                    dcfPlayDel((s8*)curFile->pDirEnt->d_name,0);

                    /* next file */
                    curFile = curFile->next;
                }
                while (curFile != dcfListReadFileEntHead);
            }
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
        }
#else
        if(dcfListFileEntHead != NULL)
        {
            curFile = dcfListFileEntHead;
            /* check if null */
            if (curFile)
            {
                /* forward traverse whole files */
                do
                {
                    /* delete file */
                    dcfPlayDel((s8*)curFile->pDirEnt->d_name,0);

                    /* next file */
                    curFile = curFile->next;
                }
                while (curFile != dcfListFileEntHead);
            }
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
        }
#endif
    }

    /* delete directory */
    dcfChDir("..");
    dcfRmDir((s8*)dcfPlaybackCurDir->pDirEnt->d_name,1);
    global_totaldir_count--;
    dcfListDirCount --;

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurDir->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurDir->used_flag = DCF_FILE_USE_NONE;
#endif

    if(dcfListDirEntHead != dcfListDirEntTail)
    {
#if CDVR_REC_WITH_PLAY_SUPPORT
        //當 playDir 與 DelDir 相同時,強迫OverWriteDel 做dcfFileInite();
        if(dcfPlaybackCurDir == dcfListDirEntHead)
        {
            dcfListDelFileEntHead = NULL;
        }
#endif

        //above 2 dir
        dcfPlaybackCurDir->prev->next = dcfPlaybackCurDir->next;
        dcfPlaybackCurDir->next->prev = dcfPlaybackCurDir->prev;
        if (dcfPlaybackCurDir == dcfListDirEntHead)
            dcfListDirEntHead = dcfPlaybackCurDir->next;
        dcfListDirEntTail = dcfListDirEntHead->prev;
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
        dcfChDir((s8*)dcfListDirEntTail->pDirEnt->d_name);
    }
    else
    {
        //just a dir
        dcfPlaybackCurDir = dcfListDirEntHead = dcfListDirEntTail = NULL;
        dcfDirNumLast =0;
        global_totaldir_count=0;
        dcfListDirCount=0;
    }
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelDir\n");

    return 1;

}

#endif

/*

Routine Description:

	Playback delete current file.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
#if FS_NEW_VERSION
s32 dcfPlaybackDel(void)
{
	u8 err, ucfiletype;

    DEBUG_DCF("Enter dcfPlaybackDel....\n");
	OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

	if (dcfListReadFileEntHead == NULL)
    {
    	DEBUG_DCF("Trace: No current file.\n");
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    if((ucfiletype = dcfGetFileType(&dcfListReadFileEntHead->pDirEnt->d_name[9])) == DCF_FILE_TYPE_MAX)
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

    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;

    dcfPlaybackCurDir->ChTotal[dcfPlaybackCurFile->pDirEnt->d_name[7]-'1']--;
    dcfPlaybackCurDir->FileTotal--;

    
	if(dcfListReadFileEntHead != dcfListReadFileEntTail)
	{
		//above 2 files
		dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
		dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
		if (dcfPlaybackCurFile == dcfListReadFileEntHead)
			dcfListReadFileEntHead = dcfPlaybackCurFile->next;
			
		dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
		dcfPlaybackCurFile = dcfPlaybackCurFile->next;
	}
	else
	{
		dcfPlaybackCurFile = dcfListReadFileEntHead = dcfListReadFileEntTail = NULL;
		// Start rescan in next playback mode
		dcfDirRescanSwitch = 1;
	}
    
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDel\n");
    return 1;
}
#else
s32 dcfPlaybackDel(void)
{
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;
    u8 delOverwrite = 0;

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    char LogFileName[32];
#endif
    //---------------------------------------------//

    DEBUG_DCF("Enter dcfPlaybackDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (dcfPlaybackCurFile == dcfListDelFileEntHead)
        dcfListDelFileEntHead = NULL;
#if CDVR_REC_WITH_PLAY_SUPPORT
    if (dcfListReadFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListReadFileEntHead->pDirEnt->d_name[9];
#else
    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
#endif

    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: file type doesn't exist, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;
    dcfPlaybackCurDir->ChTotal[dcfPlaybackCurFile->pDirEnt->d_name[7] - '1']--;
    dcfPlaybackCurDir->FileTotal--;
#if(HW_BOARD_OPTION != MR6730_AFN)
    osdDrawDelMsg((s8*)dcfPlaybackCurFile->pDirEnt->d_name,0);
#endif
    if(dcfPlayDel((s8*)dcfPlaybackCurFile->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->FileEntrySect) <= 0)
    {
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))

    strncpy(LogFileName, (s8*)dcfPlaybackCurFile->pDirEnt->d_name,9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if (dcfPlayDel((s8*)LogFileName,dcfPlaybackCurFile->pDirEnt->FileEntrySect)<=0)
    {
        DEBUG_DCF("log File: %s not found \n",LogFileName);
        //fix me =>Albert
        //return 0;
    }
#endif

	if(dcfPlaybackCurDir->ChTotal[dcfPlaybackCurFile->pDirEnt->d_name[7]-'1'] == 0)
	{
		ChDistrInDir &= ~(DCF_DISTRIB_CH1 << dcfPlaybackCurFile->pDirEnt->d_name[7]-'1');
		dcfPlaybackCurDir->CH_Distr &= ~(DCF_DISTRIB_CH1 << dcfPlaybackCurFile->pDirEnt->d_name[7]-'1');
	}

    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;

#if CDVR_REC_WITH_PLAY_SUPPORT
    if(dcfListReadFileEntHead != dcfListReadFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListReadFileEntHead)
            dcfListReadFileEntHead = dcfPlaybackCurFile->next;
        dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListReadFileEntHead = dcfListReadFileEntTail = NULL;
        dcfFileNumLast = 0;
        // Start rescan in next playback mode
        dcfDirRescanSwitch = 1;
        
    }

    //當 playDir 與 DelDir 相同時,強迫OverWriteDel 做dcfFileInite();
    if(dcfPlaybackCurDir == dcfListDirEntHead)
    {
        dcfListDelFileEntHead = NULL;
    }
#else
    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListFileEntHead)
            dcfListFileEntHead = dcfPlaybackCurFile->next;
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListFileEntHead = dcfListFileEntTail = NULL;
        dcfFileNumLast =0;
        // Start rescan in next playback mode
        dcfDirRescanSwitch = 1;
    }
#endif

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDel\n");
    return 1;
}
#endif

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
    DCF_LIST_DIRENT* curDir;
    DCF_LIST_FILEENT* curFile;
    u32 index=0;
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDelAll....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    /* delete all files */
    /* set current directory */
    curDir = dcfListDirEntHead;
    system_busy_flag=1;

    /* forward traverse whole directories */
    //Lucian: 從File Link 頭砍到尾. 先砍檔案再砍目錄. 再進下一個目錄. and so on..
    do
    {
        /* file initialization */
        if (dcfFileInit(curDir, PLAYFILEENTRY) == 0)
        {
            system_busy_flag=0;
            OSSemPost(dcfReadySemEvt);
            return 0;
        }
        /* set current file */
#if CDVR_REC_WITH_PLAY_SUPPORT
        curFile = dcfListReadFileEntHead;
#else
        curFile = dcfListFileEntHead;
#endif

        /* check if null */
        if (curFile)
        {
            /* forward traverse whole files */
            do
            {
                /* delete file */
                index++;
                osdDrawDelMsg((s8*)curFile->pDirEnt->d_name,index);
                dcfPlayDel((s8*)curFile->pDirEnt->d_name,0);

                /* next file */
                curFile = curFile->next;
            }
#if CDVR_REC_WITH_PLAY_SUPPORT
            while (curFile != dcfListReadFileEntHead);
#else
            while (curFile != dcfListFileEntHead);
#endif
        }

        /* delete directory */
        dcfChDir("..");
        dcfRmDir((s8*)curDir->pDirEnt->d_name,1);

        global_totaldir_count--;

        /* next directory */
        curDir = curDir->next;
    }
    while (curDir != dcfListDirEntHead);

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    /* directory initialization */
    if (dcfDirInit() == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfDirInit() == 0\n");
        return 0;
    }
    /* file initialization */
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfFileInit Err\n");
        return 0;
    }
    playback_location=0xFF;
    system_busy_flag=0;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelAll....\n");
    return 1;

}


s32 dcfScanDiskAll(u8 year, u8 month,u32 CamList)
{
	DCF_LIST_DIRENT* curDir;
	u32 dirYear, dirMonth;
	u32 index=0;
	s8 DirName[32];
	INT8U err;
	unsigned int OldestEntry;
	int i;

	//----------------//
	DEBUG_DCF("Enter dcfScanDiskAll....\n");
	global_totaldir_count=0;
	dcfFileTypeCount_Clean();
	//==============//
	curDir = dcfListDirEntHead;
	do
	{
#if 1
		dirYear  = (curDir->pDirEnt->fsFileCreateDate_YMD >> 9)-20;
		//dirDay   =  curDir->pDirEnt->fsFileCreateDate_YMD & 0x001F;
		dirMonth = (curDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;

		if ((year == dirYear) && (month == dirMonth))
		{
			// light weight search
			/* file initialization */
			//----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
			dcfPlaybackCurFile = NULL;
			dcfReadFileCount=0;

			if (curDir == NULL)
			{
				DEBUG_DCF("Warning: Directory Empty\n");
				return 0;
			}
			memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

			sprintf ((char*)DirName, "\\%s", gsDirName);
			if (dcfChPlayDir(DirName) == 0)
			{
				/* change directory \DCIM error */
				DEBUG_DCF("Error: Change directory %s failed.\n",DirName);

				return 0;
			}

			DEBUG_DCF("Directory Name: %s\n",curDir->pDirEnt->d_name);
			sysDeadLockMonitor_Reset();  //Lsk: MR9300 search too long, avoid reboot.

			/* change current directory */
			if (dcfChPlayDir((s8*)curDir->pDirEnt->d_name) == 0)
			{
				/* change current directory error */
				DEBUG_DCF("Error: Change current directory failed.\n");
				return 0;
			}

			/* list current directory */
			if (dcfPlayDirSearch(NULL, dcfReadFileEnt, &dcfReadFileCount, &OldestEntry, CamList, 'A',0,24*60, 0) == 0)
			{
				/* list current directory error */
				DEBUG_DCF("Error: List current directory failed.\n");
				return 0;
			}

			ChDistrInDir=0;
			global_totalfile_count = 0;
			for (i = 0; i < DCF_MAX_MULTI_FILE; i++)
			{
				global_totalCamfile_count[i] = 0;
			}
			dcfListReadFileCount = dcfReadFileCount;
			for(i=0; i < dcfReadFileCount; i++)
			{
				switch(dcfReadFileEnt[i].d_name[7])
				{
					//Lucian: 在此不check 副檔名,若有需要可增加.
					case '1':
						global_totalCamfile_count[0] ++;
						ChDistrInDir |= DCF_DISTRIB_CH1;
						break;

					case '2':
						global_totalCamfile_count[1] ++;
						ChDistrInDir |= DCF_DISTRIB_CH2;
						break;

					case '3':
						global_totalCamfile_count[2] ++;
						ChDistrInDir |= DCF_DISTRIB_CH3;
						break;

					case '4':
						global_totalCamfile_count[3] ++;
						ChDistrInDir |= DCF_DISTRIB_CH4;
						break;
				}
				global_totalfile_count ++;
			}

			curDir->CH_Distr=ChDistrInDir;
			curDir->FileTotal=global_totalfile_count;
			for(i=0; i<DCF_MAX_MULTI_FILE; i++)
				curDir->ChTotal[i]=global_totalCamfile_count[i];
			OSTimeDly(1);
		}
		else
		{
			curDir->CH_Distr=0;
			curDir->FileTotal=0;
			for(i=0; i<DCF_MAX_MULTI_FILE; i++)
				curDir->ChTotal[i]=0;
		}

		global_totaldir_count ++;

		DEBUG_DCF("ChDistrInDir=0x%x\n",ChDistrInDir);
		//dcfChDir("..");
		/* next directory */
		curDir = curDir->next;
#else
		/* file initialization */
		if (dcfFileInit(curDir, SCANFILEENTRY) == 0)
		{
			return 0;
		}
		curDir->CH_Distr=ChDistrInDir;
		curDir->FileTotal=global_totalfile_count;
		for(i=0; i<DCF_MAX_MULTI_FILE; i++)
			curDir->ChTotal[i]=global_totalCamfile_count[i];
		global_totaldir_count ++;

		DEBUG_DCF("global_totalfile_count=%d\n",global_totalfile_count);
		DEBUG_DCF("ChDistrInDir=0x%x\n",ChDistrInDir);
		//dcfChDir("..");
		/* next directory */
		curDir = curDir->next;
		OSTimeDly(1);
#endif
	}
	while (curDir != dcfListDirEntHead);

	DEBUG_DCF("global_totaldir_count=%d\n",global_totaldir_count);
	DEBUG_DCF("Leave dcfScanDiskAll....\n");

	return 1;

}


s32 dcfPlaybackCalendarInit(u8 year, u8 month, u32 CamList, u32 bRescan)

{
    DCF_LIST_DIRENT* PlaytmpDir;
    u32     i, j,cnt = 0;
    u8      day, firstDir = 0;
    u32     dirYear, dirMonth, dirDay;

    if(bRescan)
    	dcfScanDiskAll(year, month, CamList);
    PlaytmpDir = dcfListDirEntHead;
    memset(dcfPlaybackDayInfo, 0, sizeof(dcfPlaybackDayInfo));
    dcfListPlaybackDirHead = NULL;
    dcfListPlaybackDirTail = NULL;
    DEBUG_DCF("dcfPlaybackCalendarInit %d \n", global_totaldir_count);
    for (i = 0; i < global_totaldir_count; i++)
    {
        dirYear  = (PlaytmpDir->pDirEnt->fsFileCreateDate_YMD >> 9)-20;
        dirDay   = PlaytmpDir->pDirEnt->fsFileCreateDate_YMD & 0x001F;
        dirMonth = (PlaytmpDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5;
        DEBUG_DCF("Dir Create Date %d:%d:%d\n", dirYear, dirMonth, dirDay);
        if ((year != dirYear) || (month != dirMonth))
        {
            PlaytmpDir = PlaytmpDir->next;
            continue;
        }
        if (PlaytmpDir->CH_Distr & CamList)
        {
            if (dcfPlaybackDayInfo[dirDay-1].DirNum == 0)
            {
                /*first one*/
                if (firstDir == 0)
                {
                    firstDir = 1;
                    dcfListPlaybackDirHead= PlaytmpDir;
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
            DEBUG_DCF("Find Dir %s in playback list search\n", PlaytmpDir->pDirEnt->d_name);
        }
        PlaytmpDir = PlaytmpDir->next;
    }
}

#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)

/*

Routine Description:

	Insert file entry.(forward)

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListAlbFileEntInsert(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListAlbFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListAlbFileEntHead = ins;
    }
    else
    {
        cur = dcfListAlbFileEntHead;
        do
        {
            if (ins->fileNum < cur->fileNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListAlbFileEntHead)
                {
                    /* insert before head */
                    dcfListAlbFileEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListAlbFileEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}

/*

Routine Description:

	Insert file entry.(forward)

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListPicFileEntInsert(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListPicFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListPicFileEntHead = ins;
    }
    else
    {
        cur = dcfListPicFileEntHead;
        do
        {
            if (ins->fileNum < cur->fileNum) //會按照流水號排序:由小排到大
            {
                if (cur == dcfListPicFileEntHead)
                {
                    /* insert before head */
                    dcfListPicFileEntHead = ins;
                }

                break;
            }

            cur = cur->next;
        }
        while (cur != dcfListPicFileEntHead);

        /* insert before cur */
        (cur->prev)->next = ins;
        ins->prev = cur->prev;
        ins->next = cur;
        cur->prev = ins;
    }

    return 1;
}

/*

Routine Description:

	Insert file entry.(backward)

Arguments:

	ins - File entry to be inserted.

Return Value:

	0 - Failure.
	1 - Success.

*/
static s32 dcfListPicFileEntInsert_ToTail(DCF_LIST_FILEENT* ins)
{
    DCF_LIST_FILEENT* cur;

    if (dcfListPicFileEntHead == NULL)
    {
        ins->prev = ins;
        ins->next = ins;

        dcfListPicFileEntHead = ins;
    }
    else
    {
        //always insert to tail
        cur = dcfListPicFileEntHead->prev;
        cur->next = ins;
        ins->next = dcfListPicFileEntHead;
        dcfListPicFileEntHead->prev = ins;
        ins->prev = cur;
    }

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

s32 dcfCheckFile(struct FS_DIRENT* pFileEnt, u32* pFileNum, u8* pFileType, u8 FileGroup)
{
    u32 i, j,value;
    u8 fileNumStr[7];
    u32 limit[4]= {24, 60, 60, 03};

    //check sub file name
    for (j = 0; j < 3; j++)
        fileNumStr[j] = pFileEnt->d_name[j+9];
    fileNumStr[j] = '\0';

    if ((*pFileType=dcfGetFileType((char *)fileNumStr))==DCF_FILE_TYPE_MAX)
        return 0;

    for (j = 0; j < 6; j++)
        fileNumStr[j] = pFileEnt->d_name[j];
    fileNumStr[6] = '\0';

    //DEBUG_DCF("Trace: dcfCheckFile 5: Name= %s Type = %d.\n", fileNumStr,*pFileType);
    /* dcf file number is from 0001 to 9999 */
    *pFileNum = (u32)atoi((const char*)fileNumStr);
    if ((*pFileNum < 0000001) || (*pFileNum > DCF_FILENAME_MAX))
        return 0;

    //DEBUG_DCF("Trace: dcfCheckFile 6: pFileNum = %d.\n", *pFileNum);

    return 1;
}

s32 dcfCheckDir(struct FS_DIRENT* pDirEnt, u32* pDirNum)
{
    u32 j,value;
    s8 dirNumStr[5];

    /* extension of dcf directory name is composed of space characters */
    if (strcmp(&pDirEnt->d_name[8], ".   ") != 0)
        return 0;

    /* get dcf directory number string(year) */
    for (j = 0; j < 4; j++)
        dirNumStr[j] = pDirEnt->d_name[j];
    dirNumStr[4] = '\0';

    /* dcf directory number(year) is from 2255 to 2000 */
    value = (u32)atoi((const char*)dirNumStr);
    if ((value < 2000) || (value > 2255))
        return 0;

    *pDirNum = value*10000;

    /* get dcf directory number string(month) */
    for (j = 0; j < 2; j++)
        dirNumStr[j] = pDirEnt->d_name[4+j];
    dirNumStr[2] = '\0';

    /* dcf directory number(month) is from 1 to 12 */
    value = (u32)atoi((const char*)dirNumStr);
    if ((value < 1) || (value > 12))
        return 0;

    *pDirNum += value*100;

    /* get dcf directory number string(day) */
    for (j = 0; j < 2; j++)
        dirNumStr[j] = pDirEnt->d_name[6+j];
    dirNumStr[2] = '\0';

    /* dcf directory number(month) is from 1 to 31 */
    value = (u32)atoi((const char*)dirNumStr);
    if ((value < 1) || (value > 31))
        return 0;

    *pDirNum += value;


    return 1;
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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFileNext....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFileNext: begin ,dcfListDirCount=%d\n",dcfListDirCount);
    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No next file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->next;

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
    INT8U err;

    //DEBUG_DCF("Enter dcfPlaybackFilePrev....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //DEBUG_DCF("dcfPlaybackFilePrev: begin ,dcfListDirCount=%d\n",dcfListDirCount);

    if (dcfPlaybackCurFile == NULL)
    {
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Trace: No previous file.\n");
        return 0;
    }

    dcfPlaybackCurFile = dcfPlaybackCurFile->prev;

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    //DEBUG_DCF("Leave dcfPlaybackFilePrev\n");
    return 1;
}


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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirForward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirForward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
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
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDirBackward....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    if(dcfPlaybackCurDir != NULL)
    {
        dcfPlaybackCurDir = dcfPlaybackCurDir->prev;
        OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
        DEBUG_DCF("Leave dcfPlaybackDirBackward\n");
        return 1;
    }

    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("No dcfPlaybackCurDir\n");
    return 0;
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

    DEBUG_DCF("Enter dcfScanFileOnPlaybackDir....\n");
    OSFlagPend(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if (dcfFileInit(dcfPlaybackCurDir, CURRENTFILEENTRY))
    {
        /* check if file of list head is null */
        if (dcfListFileEntHead)
        {
            /* set current file */
            dcfPlaybackCurFile = dcfListFileEntHead;
            OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
            DEBUG_DCF("Leave dcfScanFileOnPlaybackDir\n");
            return 1;
        }
    }
    OSFlagPost(gDcfFlagGrp, DCF_FLAG_PLAYBACK_CUR, OS_FLAG_CLR, &err);
    DEBUG_DCF("dcfFileInit Fail\n");
    return 0;
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
s32 dcfIntToStr(u32 val, s8* str, s32 cnt)
{
    s8 i;
    s8 dig[9];

    for (i = 0; i < 6; i++)
    {
        dig[i] = dcfDecChar[val % 10];
        val /= 10;
    }

    for (i = (cnt - 1); i >= 0; i--)
        *str++ = dig[i];

    *str = '\0';

    return 1;
}

void dcfInsertPicFileNode_ToTail(DCF_FILE_INFO* File)
{
    INT8U   err;

    DEBUG_DCF("Enter DOOR dcfInsertPicFileNode_ToTail\n");
    dcfPicFileEnt[File->dcfFileInfoCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListPicFileEnt[File->dcfFileInfoListCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListPicFileEnt[File->dcfFileInfoListCount].pDirEnt = &dcfPicFileEnt[File->dcfFileInfoCount];
    dcfListPicFileEnt[File->dcfFileInfoListCount].fileNum = File->dcfFileInfoNumLast;
    dcfListPicFileEnt[File->dcfFileInfoListCount].fileType = File->dcfFileInfoFileType;
    dcfListPicFileEntInsert_ToTail(&dcfListPicFileEnt[File->dcfFileInfoListCount]);
    dcfListPicFileEntTail = dcfListPicFileEntHead->prev;
    //dcfListFileEntTail = &dcfListFileEnt[dcfListFileCount];

    dcfFileTypeCount_Inc(File->dcfFileInfoFileType);
    global_total_Pic_file_count++;
    DEBUG_DCF("Leave Door dcfInsertPicFileNode_ToTail\n");
}

void dcfInsertFileNode_ToTail(DCF_FILE_INFO* File)
{
    INT8U   err;
    if (File->dcfFileInfoFileType == DCF_FILE_TYPE_JPG)
    {
        dcfInsertPicFileNode_ToTail(File);
        return;
    }

    dcfFileEnt[File->dcfFileInfoCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].used_flag = DCF_FILE_USE_CLOSE;
    dcfListFileEnt[File->dcfFileInfoListCount].pDirEnt = &dcfFileEnt[File->dcfFileInfoCount];
    dcfListFileEnt[File->dcfFileInfoListCount].fileNum = File->dcfFileInfoNumLast;
    dcfListFileEnt[File->dcfFileInfoListCount].fileType = File->dcfFileInfoFileType;
    dcfListFileEntInsert_ToTail(&dcfListFileEnt[File->dcfFileInfoListCount]);
    dcfListFileEntTail = dcfListFileEntHead->prev;

    dcfFileTypeCount_Inc(File->dcfFileInfoFileType);
    global_totalfile_count++;

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
s32 dcfDirInit(void)
{
    u32 dirNum;
    u32 i;


    DEBUG_DCF("dcfDirInit: Begin \n");

    /*Init Movie*/
    global_totaldir_count = 0;
    global_totalfile_count = 0;
    dcfFileNumLast = 0; /* 0001 - 1 */
    dcfDelFileNumLast = 0; /* 0001 - 1 */
#if !FS_NEW_VERSION    
    dcfListDelFileEntHead = NULL;
    dcfListDelFileEntTail = NULL;
#endif
    dcfFileCount=0;
    dcfListFileCount=0;
    dcfListDelFileCount = dcfDelFileCount=0;

    /*Init Picture*/
    global_total_Pic_file_count = 0;
    dcfPicFileNumLast = 0; /* 0001 - 1 */
    dcfDelPicFileNumLast = 0; /* 0001 - 1 */
    dcfListDelPicFileEntHead = NULL;
    dcfListDelPicFileEntTail = NULL;
    dcfPicFileCount=0;
    dcfListPicFileCount=0;
    dcfListDelPicFileCount = dcfDelPicFileCount=0;

    dcfFileTypeCount_Clean();
    memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    memset_hw((void*)dcfPicFileEnt, 0, sizeof(dcfPicFileEnt)); //civicwu 070926
    memset_hw((void*)dcfListPicFileEnt, 0, sizeof(dcfListPicFileEnt));

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    FW_findIspFilename(&ispUSBFileName[1]); // skip the "\\"
#endif

    /*check movie directory*/
    if (dcfChDir(MOV_DIR) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");

            return 0;
        }

        if (dcfMkDir(MOV_DIR) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",MOV_DIR);

            return -1;
        }

        //    //Lucian: 增加direct FDB entry. 增加至 512 entry.
        dcfIncDirEnt((s8 *)MOV_DIR,DCF_DIRENT_MAX);

        if (dcfChDir(MOV_DIR) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",MOV_DIR);

            return -1;
        }
    }


    /* list directory \DCIM */
    // List all the directory which in DCIM civic
    if (dcfDir(MOV_DIR, dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
    {
        /* list directory \DCIM error */
        DEBUG_DCF("Error: List directory %s failed.\n",MOV_DIR);
    }


    /*check picture directory*/
    if (dcfChDir(PIC_DIR) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");
            return 0;
        }

        if (dcfMkDir(PIC_DIR) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",PIC_DIR);
            return -1;
        }

        //    //Lucian: 增加direct FDB entry. 增加至 512 entry.
        dcfIncDirEnt((s8 *)PIC_DIR,DCF_DIRENT_MAX);

        if (dcfChDir(PIC_DIR) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",PIC_DIR);
            return -1;
        }
    }
#if 0
    /*check ALBUM directory*/
    if (dcfChDir(ALB_DIR) == 0)
    {
        /* change directory \DCIM error */
        if (Write_protet()==1)
        {
            // Write protect
            DEBUG_DCF("Error: Write protect\n");
            return 0;
        }

        if (dcfMkDir(ALB_DIR) == 0)
        {
            /* make directory \DCIM error */
            DEBUG_DCF("Error: Make directory %s failed.\n",ALB_DIR);
            return -1;
        }

        //    //Lucian: 增加direct FDB entry. 增加至 512 entry.
        dcfIncDirEnt((s8 *)ALB_DIR,DCF_DIRENT_MAX);

        if (dcfChDir(ALB_DIR) == 0)
        {
            /* change directory \DCIM error */
            DEBUG_DCF("Error: Change directory %s failed.\n",ALB_DIR);
            return -1;
        }
    }
#endif

    DEBUG_DCF("--->dcfDir complete.\n");

#if 0
    // sort the directory and create the directory list
    memset_hw((void*)dcfListDirEnt, 0, sizeof(DCF_LIST_DIRENT) * DCF_DIRENT_MAX);
    dcfListDirCount = 0;
    // List the valid directory and make the linked list
    for (i = 0; i < dcfDirCount; i++)
    {
        if (dcfDirEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY)
        {
            // check if valid dcf directory
            if (dcfCheckDir(&dcfDirEnt[i], &dirNum) == 0)
                continue;

            // assign last directory number
            dcfDirNumLast = (dcfDirNumLast < dirNum) ? dirNum : dcfDirNumLast;

            // insert directory entry to the list
            dcfListDirEnt[dcfListDirCount].used_flag = DCF_FILE_USE_CLOSE;
            dcfListDirEnt[dcfListDirCount].dirNum = dirNum;
            dcfListDirEnt[dcfListDirCount].pDirEnt = &dcfDirEnt[i];
            dcfListDirEntInsert(&dcfListDirEnt[dcfListDirCount]);
            dcfListDirCount++;
            //Add on 20091208, for the playback of CDVR File system
            global_totaldir_count ++;
        }
    }
    if(dcfListDirEntHead != NULL)
    {
        /* set tail entry of directory list */
        dcfListDirEntTail = dcfListDirEntHead->prev;

        /* set current playback directory */
        dcfPlaybackCurDir = dcfListDirEntTail;
    }

#if 1
    {
        DCF_LIST_DIRENT* dcfListDirEntTemp = NULL;
        dcfListDirEntTemp = dcfListDirEntHead;
        if (dcfListDirEntHead == NULL)
            return 1;

        do //??
        {
            DEBUG_DCF("Dir Name: %s\n", dcfListDirEntTemp->pDirEnt->d_name);
            dcfListDirEntTemp = dcfListDirEntTemp->next;

        }
        while(dcfListDirEntTemp != dcfListDirEntHead );

    }
#endif
#endif
    return 1;
}

static s32 dcfMovieFileInit(DCF_LIST_DIRENT* pListDirEnt)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

    dcfResetFileEntrySect(0,0);

    dcfFileNumLast = 0; /* 0001 - 1 */
    dcfListFileEntHead = NULL;
    dcfListFileEntTail = dcfPlaybackCurFile = NULL;
    global_totalfile_count = 0;

    /* set tail entry of file list */

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfFileCount=0; //civicwu 070926
    memset_hw((void*)dcfFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

    if (dcfChDir(MOV_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",MOV_DIR);

        return 0;
    }

    /* list current directory */
    if (dcfDir(NULL, dcfFileEnt, &dcfFileCount,1,1,DCF_FILEENT_MAX) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");

        return 0;
    }
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 2 =%d (x50ms)\n",time1);
#endif
    /* sort the file and create the file list */
    memset_hw((void*)dcfListFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);
    dcfListFileCount = 0;

    for (i = 0; i < dcfFileCount; i++)
    {
        if ((dcfFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
        {
            /* check if valid dcf file */
            if (dcfCheckFile(&dcfFileEnt[i], &fileNum, &fileType, 0) == 0)
                continue;

#if FILE_REPAIR_AUTO //Repair Bad file
            if(fileType == 6) // AXF file
            {
                DEBUG_SYS("Repair Bad File:%s\n",dcfFileEnt[i].d_name);
                uiClearOSDBuf(2);
                uiMenuOSDStringByColor(TVOSD_SizeX , "Repair Bad File:" , OSD_STRING_W , OSD_STRING_H , 48, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                uiMenuOSDStringByColor(TVOSD_SizeX , dcfFileEnt[i].d_name , OSD_STRING_W , OSD_STRING_H , 176, 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                asfRepairFile(dcfFileEnt[i].d_name);
                DEBUG_SYS("===Repair Complete===\n");

                memcpy(NewName,dcfFileEnt[i].d_name,13);
                NewName[10]='S';
                if(0 == dcfRename(NewName,dcfFileEnt[i].d_name))
                {
                    DEBUG_DCF("Warning! DCF Rename Fail!\n");
                }
                fileType=0;
                dcfFileEnt[i].d_name[10]='S';
            }
#endif
            dcfFileTypeCount_Inc(fileType);
            global_totalfile_count++;

            /* assign last file number */
            dcfFileNumLast = (dcfFileNumLast < fileNum) ? fileNum : dcfFileNumLast;

            /* insert file entry to the list */
            dcfListFileEnt[dcfListFileCount].used_flag=DCF_FILE_USE_CLOSE;
            dcfListFileEnt[dcfListFileCount].fileNum = fileNum;
            dcfListFileEnt[dcfListFileCount].fileType = fileType;
            dcfListFileEnt[dcfListFileCount].pDirEnt = &dcfFileEnt[i];
            dcfListFileEntInsert(&dcfListFileEnt[dcfListFileCount]);
            dcfListFileCount++;
        }

        sysDeadLockMonitor_Reset();
    }

    /* set tail entry of file list */
    if( dcfListFileEntHead != NULL)
    {
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfListFileEntTail->next =dcfListFileEntHead;
    }
    else
    {
        dcfListFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile=dcfListFileEntHead->prev;

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 3 =%d (x50ms)\n",time1);
#endif
    return 1;
}

static s32 dcfPicFileInit(DCF_LIST_DIRENT* pListDirEnt)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
//    DCF_LIST_DIRENT* temp_head=dcfListDirEntHead;
//    u32 local_dcfFileCount;
//    struct FS_DIRENT* local_dcfFileEnt = &dcfFileEnt[0];
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 1 =%d (x50ms)\n",time1);
#endif

    global_total_Pic_file_count = 0;
    dcfResetFileEntrySect(0,0);
    /* set tail entry of file list */

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfPicFileNumLast = 0; /* 0001 - 1 */
    dcfListPicFileEntHead = NULL;

    dcfPicFileCount=0;
    memset_hw((void*)dcfPicFileEnt, 0, sizeof(dcfPicFileEnt)); //civicwu 070926
    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir(PIC_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",PIC_DIR);

        return 0;
    }

    /* list current directory */
    if (dcfDir(NULL, dcfPicFileEnt, &dcfPicFileCount,1,1,DCF_PIC_FILE_PER_DIR + 3) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");

        return 0;
    }
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 2 =%d (x50ms)\n",time1);
#endif
    /* sort the file and create the file list */
    memset_hw((void*)dcfListPicFileEnt, 0, sizeof(dcfListPicFileEnt));
    dcfListPicFileCount = 0;

    for (i = 0; i < dcfPicFileCount; i++)
    {
        if ((dcfPicFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
        {
            /* check if valid dcf file */
            if (dcfCheckFile(&dcfPicFileEnt[i], &fileNum, &fileType, 0) == 0)
                continue;

            if(fileType != DCF_FILE_TYPE_JPG)
                continue;

            dcfFileTypeCount_Inc(fileType);
            global_total_Pic_file_count++;

            /* assign last file number */
            dcfPicFileNumLast = (dcfPicFileNumLast < fileNum) ? fileNum : dcfPicFileNumLast;

            /* insert file entry to the list */
            dcfListPicFileEnt[dcfListPicFileCount].used_flag=DCF_FILE_USE_CLOSE;
            dcfListPicFileEnt[dcfListPicFileCount].fileNum = fileNum;
            dcfListPicFileEnt[dcfListPicFileCount].fileType = fileType;
            dcfListPicFileEnt[dcfListPicFileCount].pDirEnt = &dcfPicFileEnt[i];
            dcfListPicFileEntInsert(&dcfListPicFileEnt[dcfListPicFileCount]);
            dcfListPicFileCount++;
        }

        sysDeadLockMonitor_Reset();
    }

    /* set tail entry of file list */
    if( dcfListPicFileEntHead != NULL)
    {
        dcfListPicFileEntTail = dcfListPicFileEntHead->prev;
        dcfListPicFileEntTail->next =dcfListPicFileEntHead;
    }
    else
    {
        dcfListPicFileEntTail =NULL;
    }

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 3 =%d (x50ms)\n",time1);
#endif

    return 1;
}

static s32 dcfAlbFileInit(DCF_LIST_DIRENT* pListDirEnt)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
//    DCF_LIST_DIRENT* temp_head=dcfListDirEntHead;
//    u32 local_dcfFileCount;
//    struct FS_DIRENT* local_dcfFileEnt = &dcfFileEnt[0];
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 1 =%d (x50ms)\n",time1);
#endif

    global_total_Alb_file_count = 0;
    dcfResetFileEntrySect(0,0);
    /* set tail entry of file list */

    //----------------Scan最後一個目錄,建立file link,找出寫檔起點----------------------
    dcfAlbFileNumLast = 0; /* 0001 - 1 */
    dcfListAlbFileEntHead = NULL;

    dcfAlbFileCount=0;
    memset_hw((void*)dcfAlbFileEnt, 0, sizeof(dcfAlbFileEnt)); //civicwu 070926
    /* change directory \DCIM (make diretory \DCIM if necessary) */
    if (dcfChDir(ALB_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",ALB_DIR);

        return 0;
    }

    /* list current directory */
    if (dcfDir(NULL, dcfAlbFileEnt, &dcfAlbFileCount,1,1,DCF_PIC_FILE_PER_DIR) == 0)
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");

        return 0;
    }
#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 2 =%d (x50ms)\n",time1);
#endif
    /* sort the file and create the file list */
    memset_hw((void*)dcfListAlbFileEnt, 0, sizeof(dcfListAlbFileEnt));
    dcfListAlbFileCount = 0;

    for (i = 0; i < dcfAlbFileCount; i++)
    {
        if ((dcfAlbFileEnt[i].FAT_DirAttr & DCF_FAT_ATTR_DIRECTORY) == 0)
        {
            /* check if valid dcf file */
            if (dcfCheckFile(&dcfAlbFileEnt[i], &fileNum, &fileType, 0) == 0)
                continue;

            if(fileType != DCF_FILE_TYPE_JPG)
                continue;

            dcfFileTypeCount_Inc(fileType);
            global_total_Alb_file_count++;

            /* assign last file number */
            dcfAlbFileNumLast = (dcfAlbFileNumLast < fileNum) ? fileNum : dcfAlbFileNumLast;

            /* insert file entry to the list */
            dcfListAlbFileEnt[dcfListAlbFileCount].used_flag=DCF_FILE_USE_CLOSE;
            dcfListAlbFileEnt[dcfListAlbFileCount].fileNum = fileNum;
            dcfListAlbFileEnt[dcfListAlbFileCount].fileType = fileType;
            dcfListAlbFileEnt[dcfListAlbFileCount].pDirEnt = &dcfAlbFileEnt[i];
            dcfListAlbFileEntInsert(&dcfListAlbFileEnt[dcfListAlbFileCount]);
            dcfListAlbFileCount++;
        }

        sysDeadLockMonitor_Reset();
    }

    /* set tail entry of file list */
    if( dcfListAlbFileEntHead != NULL)
    {
        dcfListAlbFileEntTail = dcfListAlbFileEntHead->prev;
        dcfListAlbFileEntTail->next =dcfListAlbFileEntHead;
    }
    else
    {
        dcfListAlbFileEntTail =NULL;
    }

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("DCF Time 3 =%d (x50ms)\n",time1);
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
s32 dcfFileInit(DCF_LIST_DIRENT* pListDirEnt, u8 Type)
{
    u32 i;
    u32 fileNum;
    u8 fileType;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;

#if FILE_REPAIR_AUTO
    char NewName[13]= {0};
#endif

#if (ROOTWORK==0)
    s8	DirName[32];
#endif

    DEBUG_DCF("dcfFileInit: Begin %d\n", Type);

    if ( Type == CURRENTFILEENTRY)
    {
        return dcfMovieFileInit(pListDirEnt);
    }
    else if ( Type == CUR_PIC_FILE_ENTRY)
    {
        return dcfPicFileInit(pListDirEnt);
    }
#if 0
    else if (Type == CUR_ALB_FILE_ENTRY)
    {
        return dcfAlbFileInit(pListDirEnt);
    }
#endif
    else
        return 0;

}

#if FS_NEW_VERSION
s32 dcfOverWriteDel(void)
{
	int ret;
	u8 err;
	
	if(dcfListDirEntHead == NULL)
	{	
		DEBUG_DCF("[ERR] DCF Head of Dir entrys is NULL.\n");
		return 0;
	}

	DEBUG_DCF("Enter DVR dcfOverWriteDel....\n");
	OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

	if(dcfListDelDirEnt != dcfListDirEntHead)
		dcfListDelDirEnt = dcfListDirEntHead;

	if((ret = dcfOWDel(MOV_DIR)) < 0)
	{
		DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
		OSSemPost(dcfReadySemEvt);
		return 0;
	}

	if(ret == 0)
		DEBUG_DCF("[INF] Directory is empty.\n");
	
	OSSemPost(dcfReadySemEvt);
	DEBUG_DCF("Leave DVR dcfOverWriteDel....\n");
	return 1;
}
#else
s32 dcfOverWriteDel(void)
{
    char* pExtName ;
    char extName[5];
    char j;
    u8 ucfiletype;
    u8  err;
    int ret;

    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    //OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    DEBUG_DCF("Enter DVR dcfOverWriteDel....\n");

    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        //OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        DEBUG_ASF("Error: file type doesn't exist, %d\n", ucfiletype);
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        //OSSemPost(dcfReadySemEvt);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;


    // find number minum
    dcfListFileEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfListFileEntHead->used_flag = DCF_FILE_USE_NONE;
    DEBUG_ASF("Over Write delete Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
    strcpy((char*)dcfDelDir, MOV_DIR);
    if ((ret = dcfOWDel((s8*)dcfListFileEntHead->pDirEnt->d_name,dcfListFileEntHead->pDirEnt->FileEntrySect)) <= 0)
    {
        DEBUG_ASF("Error: file delete fail %s\nError code: %d\n", dcfListFileEntHead->pDirEnt->d_name, ret);
        switch(ret)
        {
            case FS_FILE_ENT_FIND_ERROR:
                if(dcfListFileEntHead != dcfListFileEntTail)
                {
                    dcfListFileEntHead=dcfListFileEntHead->next;
                    dcfListFileEntTail->next =dcfListFileEntHead;
                    dcfListFileEntHead->prev=dcfListFileEntTail;
                }
                else
                    dcfListFileEntHead = dcfListFileEntTail = NULL;
                break;
            default:
                break;
        }
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        //OSSemPost(dcfReadySemEvt);
        return 0;
    }

    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListFileEntHead=dcfListFileEntHead->next;
        dcfListFileEntTail->next =dcfListFileEntHead;
        dcfListFileEntHead->prev=dcfListFileEntTail;
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X , count= %X \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }
    else
    {
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListFileEntHead = dcfListFileEntTail = NULL;
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X, count= %X  \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }

    DEBUG_DCF("Leave DVR dcfOverWriteDel\n");
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    //OSSemPost(dcfReadySemEvt);
    return 1;
}
#endif

#if FS_NEW_VERSION
s32 dcfOverWritePicDel(void)
{
	int ret;
	u8 err;
	
	if(dcfListDirEntHead == NULL)
	{	
		DEBUG_DCF("[ERR] DCF Head of Dir entrys is NULL.\n");
		return 0;
	}

	DEBUG_DCF("Enter DVR dcfOverWritePicDel....\n");
	OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

	if(dcfListDelDirEnt != dcfListDirEntHead)
		dcfListDelDirEnt = dcfListDirEntHead;

	if((ret = dcfOWDel(PIC_DIR)) < 0)
	{
		DEBUG_DCF("[ERR] DCF OverWrite fail. %s\n", dcfListDelDirEnt->pDirEnt->d_name);
		OSSemPost(dcfReadySemEvt);
		return 0;
	}

	if(ret == 0)
		DEBUG_DCF("[INF] Directory is empty.\n");
	
	OSSemPost(dcfReadySemEvt);
	DEBUG_DCF("Leave DVR dcfOverWritePicDel....\n");
	return 1;
}
#else
s32 dcfOverWritePicDel(void)
{
    char* pExtName ;
    char extName[5];
    char j;
    u8 ucfiletype;
    u8  err;
    int ret;

    DEBUG_DCF("Enter Door dcfOverWritePicDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    //OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    dcfResetFileEntrySect(0,0);
    dcfChDir(PIC_DIR);
    if (dcfListPicFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListPicFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype !=DCF_FILE_TYPE_JPG)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_ASF("Error: file type doesn't JPG %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_total_Pic_file_count--;

    // find number minum
    dcfListPicFileEntHead->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfListPicFileEntHead->used_flag = DCF_FILE_USE_NONE;
    DEBUG_ASF("Over Write delete Name: %s\n",dcfListPicFileEntHead->pDirEnt->d_name);
    strcpy((char*)dcfDelDir, PIC_DIR);
    if ((ret = dcfOWDel((s8*)dcfListPicFileEntHead->pDirEnt->d_name,dcfListPicFileEntHead->pDirEnt->FileEntrySect)) <=0)
    {
        DEBUG_DCF("Error: file delete fail %s\nError code: %d\n", dcfListPicFileEntHead->pDirEnt->d_name, ret);
        switch(ret)
        {
            case FS_FILE_ENT_FIND_ERROR:
                if(dcfListPicFileEntHead != dcfListPicFileEntTail)
                {
                    dcfListPicFileEntHead=dcfListPicFileEntHead->next;
                    dcfListPicFileEntTail->next =dcfListPicFileEntHead;
                    dcfListPicFileEntHead->prev=dcfListPicFileEntTail;
                }
                else
                    dcfListPicFileEntHead = dcfListPicFileEntTail = NULL;
                break;
            default:
                break;
        }
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        return 0;
    }

    if(dcfListPicFileEntHead != dcfListPicFileEntTail)
    {
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListPicFileEntHead=dcfListPicFileEntHead->next;
        dcfListPicFileEntTail->next =dcfListPicFileEntHead;
        dcfListPicFileEntHead->prev=dcfListPicFileEntTail;
        //DEBUG_DCF("dcfOverWriteDel 44: dcfListFileEntHead= %X, dcfListFileEntTail= %X , count= %X \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }
    else
    {
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X \n",dcfListFileEntHead, dcfListFileEntTail);
        dcfListPicFileEntHead = dcfListPicFileEntTail = NULL;
        //DEBUG_DCF("dcfOverWriteDel 55: dcfListFileEntHead= %X, dcfListFileEntTail= %X, count= %X  \n",dcfListFileEntHead, dcfListFileEntTail,global_totalfile_count);
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave Door dcfOverWritePicDel\n");
    return 1;
}
#endif
/*

Routine Description:

	Create next file.

Arguments:

	fileType - File type.

Return Value:

	File handle.

*/
FS_FILE* dcfCreateNextPicFile(u8 index)
{
    s8 newFileName[32];
    FS_FILE* pFile;
    int     scan;
    INT8U   err;
    unsigned short val;
    RTC_DATE_TIME   localTime;

    DEBUG_DCF("Enter Door dcfCreateNextPicFile %d....\n",index);
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    if (global_total_Pic_file_count >= DCF_PIC_FILE_PER_DIR)
    {
        dcfOverWritePicDel();
    }

    if (index >= DCF_MAX_CHANNEL_IDX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("dcfCreateNextFile Error Index %d\n",index);
        return 0;
    }
    dcfResetFileEntrySect(0,0);
    if (dcfChDir(PIC_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",MOV_DIR);

        return 0;
    }

    dcfPicFileNumLast++;

    if (dcfPicFileNumLast > DCF_FILENAME_MAX)
    {
        dcfPicFileNumLast=0;
    }

    if (DcfFileNameTag[0] != '\0')
    {
        char fmt[] = "%06d%s";
        sprintf(fmt, "%%0%dd%%s", 8 - strlen(DcfFileNameTag));
        sprintf(newFileName, fmt, dcfPicFileNumLast, DcfFileNameTag);
    }
    else
    {
        sprintf(newFileName, "%06d%s", dcfPicFileNumLast, DcfDoorFileName[index]);
    }

    //-----------------------------//
    strcat((char*)newFileName,".");
    strcat((char*)newFileName,gdcfFileType_Info[DCF_FILE_TYPE_JPG].pSubFileStr);

    strcpy(&dcfFileName[index][0], newFileName);

    if ((pFile = dcfOpen(newFileName, "w-")) == NULL) // "w-": Create file 前不scan FDB.
    {
        /* create next file error */
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: create Name: %s, error\n",newFileName);
        return NULL;
    }


    DEBUG_DCF("Over Write create Name: %s, %d\n",newFileName,global_total_Pic_file_count);
    if(dcfListPicFileEntHead != NULL)
    {
        DEBUG_DCF("Over Write Head Name: %s\n",dcfListPicFileEntHead->pDirEnt->d_name);
    }

    /* list file newly created */
    scan=0;
    while(dcfPicFileEnt[dcfPicFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfPicFileCount ++;
        if(dcfPicFileCount>=DCF_PIC_FILE_PER_DIR)
        {
            scan ++;
            dcfPicFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfFileBuf is full!\n");
            dcfPicFileCount=0;
            break;
        }
    }
    dcfPicFileEnt[dcfPicFileCount].used_flag = DCF_FILE_USE_CREATE;

    scan=0;
    while(dcfListPicFileEnt[dcfListPicFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListPicFileCount ++;
        if(dcfListPicFileCount>=DCF_PIC_FILE_PER_DIR)
        {
            scan ++;
            dcfListPicFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListFileBuf is full!\n");
            dcfListPicFileCount=0;
            break;
        }
    }
    dcfListPicFileEnt[dcfListPicFileCount].used_flag = DCF_FILE_USE_CREATE;

    DEBUG_DCF("dcfFileCount: %d\n",dcfPicFileCount);
    DEBUG_DCF("dcfListFileCount: %d\n",dcfListPicFileCount);

#if 0
    if (dcfGetDirEnt(NULL, &dcfFileEnt[dcfFileCount], newFileName) == 0)
    {
        /* list file error */
        DEBUG_DCF("Error: Find file entry of %s failed.\n", newFileName);

        return NULL;
    }
#else

    dcfPicFileEnt[dcfPicFileCount].FAT_DirAttr = 0x20;
    strcpy(dcfPicFileEnt[dcfPicFileCount].d_name, newFileName);

    dcfPicFileEnt[dcfPicFileCount].FileEntrySect=pFile->FileEntrySect;

    RTC_Get_Time(&localTime);
    val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

    dcfPicFileEnt[dcfPicFileCount].fsFileCreateTime_HMS =  val;
    dcfPicFileEnt[dcfPicFileCount].fsFileModifiedTime_HMS = val;

    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    dcfPicFileEnt[dcfPicFileCount].fsFileCreateDate_YMD = val;
    dcfPicFileEnt[dcfPicFileCount].fsFileModifiedDate_YMD = val;

#endif
#if 0
    DEBUG_ASF("dcfFileEnt:name:%s, %d, %X, %X, %X, %X\n"
              ,dcfFileEnt[dcfFileCount].d_name,dcfFileEnt[dcfFileCount].FAT_DirAttr
              ,dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS,dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD
              ,dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS,dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD);

#endif
    gfileType = DCF_FILE_TYPE_JPG;
    dcfFileInfoTab[index].dcfFileInfoCount = dcfPicFileCount;
    dcfFileInfoTab[index].dcfFileInfoListCount = dcfListPicFileCount;
    dcfFileInfoTab[index].dcfFileInfoNumLast = dcfPicFileNumLast;
    dcfFileInfoTab[index].dcfFileInfoFileType = DCF_FILE_TYPE_JPG;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DVR dcfCreateNextFile\n");
    return pFile;
}
/*

Routine Description:

	Create next file.

Arguments:

	fileType - File type.

Return Value:

	File handle.

*/
FS_FILE* dcfCreateNextFile(u8 fileType, u8 index)
{
    char newFileName[32];
    u32 fileNum,dirNum;
    int scan;
    FS_FILE* pFile;

    unsigned short val;
    RTC_DATE_TIME   localTime;
#if (ROOTWORK==0)
    s8  DirName[32];
#endif
    INT8U   err;

    if (fileType == DCF_FILE_TYPE_JPG)
        return dcfCreateNextPicFile(index);

    DEBUG_DCF("Enter Door dcfCreateNextFile\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    dcfResetFileEntrySect(0,0);
    if (dcfChDir(MOV_DIR) == 0)
    {
        /* change directory \DCIM error */
        DEBUG_DCF("Error: Change directory %s failed.\n",MOV_DIR);

        return 0;
    }

    if (global_totalfile_count > (DCF_FILE_PER_DIR-20))
    {
        // Find the oldest file pointer and delete it
        if (dcfOverWriteDel() == 0)
        {
            //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
            OSSemPost(dcfReadySemEvt);
            DEBUG_DCF("Over Write delete fail!!\n");
            return 0;
        }
        else
        {
            //DEBUG_DCF("Over Write delete Pass!!\n");
        }
    }
    if (global_totalfile_count > (DCF_FILE_PER_DIR-20))
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("file number is over: %d\n",global_totalfile_count);
        return 0;
    }

    if (index >= DCF_MAX_MULTI_FILE)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("dcfCreateNextFile Error Index %d\n",index);
        return 0;
    }

    dcfFileNumLast++;

    if (dcfFileNumLast > DCF_FILENAME_MAX)
    {
        dcfFileNumLast=0;
    }

    if (DcfFileNameTag[0] != '\0')
    {
        char fmt[] = "%06d%s";
        sprintf(fmt, "%%0%dd%%s", 8 - strlen(DcfFileNameTag));
        sprintf(newFileName, fmt, dcfFileNumLast, DcfFileNameTag);
    }
    else
    {
        sprintf(newFileName, "%06d%s", dcfFileNumLast, DcfDoorFileName[index]);
    }

    if (fileType >= DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: fileType=%d does not exist\n", fileType);
        return NULL;
    }
    strcat((char*)newFileName,".");
    strcat((char*)newFileName,gdcfFileType_Info[fileType].pSubFileStr);

    strcpy(&dcfFileName[index][0], newFileName);

    if ((pFile = dcfOpen(newFileName, "w-")) == NULL) // "w-": Create file 前不scan FDB.
    {
        /* create next file error */
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: create Name: %s, error\n",newFileName);
        return NULL;
    }


    DEBUG_DCF("Over Write create Name: %s, %d\n",newFileName,global_totalfile_count);
    if(dcfListFileEntHead != NULL)
    {
        DEBUG_DCF("Over Write Head Name: %s\n",dcfListFileEntHead->pDirEnt->d_name);
    }

    /* list file newly created */
    scan=0;
    while(dcfFileEnt[dcfFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfFileCount ++;
        if(dcfFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfFileBuf is full!\n");
            dcfFileCount=0;
            break;
        }
    }
    dcfFileEnt[dcfFileCount].used_flag = DCF_FILE_USE_CREATE;

    scan=0;
    while(dcfListFileEnt[dcfListFileCount].used_flag != DCF_FILE_USE_NONE)
    {
        dcfListFileCount ++;
        if(dcfListFileCount>=DCF_FILEENT_MAX)
        {
            scan ++;
            dcfListFileCount=0;
        }
        if(scan >1)
        {
            DEBUG_DCF("Warning! dcfListFileBuf is full!\n");
            dcfListFileCount=0;
            break;
        }
    }
    dcfListFileEnt[dcfListFileCount].used_flag = DCF_FILE_USE_CREATE;

    DEBUG_DCF("dcfFileCount: %d\n",dcfFileCount);
    DEBUG_DCF("dcfListFileCount: %d\n",dcfListFileCount);

#if 0
    if (dcfGetDirEnt(NULL, &dcfFileEnt[dcfFileCount], newFileName) == 0)
    {
        /* list file error */
        DEBUG_DCF("Error: Find file entry of %s failed.\n", newFileName);

        return NULL;
    }
#else

    dcfFileEnt[dcfFileCount].FAT_DirAttr = 0x20;
    dcfFileEnt[dcfFileCount].d_ino = pFile->fileid_hi;
    strcpy(dcfFileEnt[dcfFileCount].d_name, newFileName);

    dcfFileEnt[dcfFileCount].FileEntrySect=pFile->FileEntrySect;

    RTC_Get_Time(&localTime);
    val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS =  val;
    dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS = val;

    val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

    dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD = val;
    dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD = val;

#endif
#if 0
    DEBUG_ASF("dcfFileEnt:name:%s, %d, %X, %X, %X, %X\n"
              ,dcfFileEnt[dcfFileCount].d_name,dcfFileEnt[dcfFileCount].FAT_DirAttr
              ,dcfFileEnt[dcfFileCount].fsFileCreateTime_HMS,dcfFileEnt[dcfFileCount].fsFileCreateDate_YMD
              ,dcfFileEnt[dcfFileCount].fsFileModifiedTime_HMS,dcfFileEnt[dcfFileCount].fsFileModifiedDate_YMD);

#endif
    gfileType = fileType;
    dcfFileInfoTab[index].dcfFileInfoCount = dcfFileCount;
    dcfFileInfoTab[index].dcfFileInfoListCount = dcfListFileCount;
    dcfFileInfoTab[index].dcfFileInfoNumLast = dcfFileNumLast;
    dcfFileInfoTab[index].dcfFileInfoFileType = fileType;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave DVR dcfCreateNextFile\n");
    return pFile;
}

s32 dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile)
{
    s32 rel;
    INT8U   err;

    if (idx >= DCF_MAX_MULTI_FILE)
    {
        DEBUG_DCF("dcfClose Error Index %d\n",idx);
        return 0;
    }
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_DCF("\ndcfCloseFileByIdx Index %d\n",idx);
    rel = dcfClose(pFile, pOpenFile);
    dcfInsertFileNode_ToTail(&dcfFileInfoTab[idx]);
    dcfNewFile = 1;
    DEBUG_DCF("dcfCloseFileByIdx idx=%d, OSFlagPost Close %d\n\n", idx, rel);
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    return rel;
}

s32 dcfDoorChangeDir(u8  dirIdx)
{
    dcfPlaybackDirMode = dirIdx;
    switch (dirIdx)
    {
        case DCF_DOOR_DIR_MOVIE:
            dcfResetFileEntrySect(0,0);
            return dcfChDir(MOV_DIR);

        case DCF_DOOR_DIR_PICTURE:
            dcfResetFileEntrySect(0,0);
            return dcfChDir(PIC_DIR);

        case DCF_DOOR_DIR_ALBUM:
            dcfResetFileEntrySect(0,0);
            return dcfChDir(ALB_DIR);

        default:
            DEBUG_DCF("dcfDoorChangeDir Error index %d\n",dirIdx);
            return 0;
    }
}

s32 dcfPlaybackDelPic(void)
{
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDelPic....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfListPicFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListPicFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype != DCF_FILE_TYPE_JPG)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: file type doesn't JPG, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_total_Pic_file_count--;

    osdDrawDelMsg((s8*)dcfPlaybackCurFile->pDirEnt->d_name,0);
    dcfDel((s8*)dcfPlaybackCurFile->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->FileEntrySect);

    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;

    //Albert Modified on 20091209
    if(dcfListPicFileEntHead != dcfListPicFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListPicFileEntHead)
            dcfListPicFileEntHead = dcfPlaybackCurFile->next;
        dcfListPicFileEntTail = dcfListPicFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListPicFileEntHead = dcfListPicFileEntTail = NULL;
        dcfPicFileNumLast =0;
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelPic\n");
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
    DCF_LIST_FILEENT* curFile;
    INT8U err;

    DEBUG_DCF("dcfPlaybackDelDir Begin\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfPlaybackCurDir == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current Dir.\n");
        return 0;
    }
    else
    {
        if(dcfScanFileOnPlaybackDir() == 0)
        {
            DEBUG_DCF("DelDir do dcfScanFileOnPlaybackDir Fail.\n");
        }

        if(dcfListFileEntHead != NULL)
        {
            curFile = dcfListFileEntHead;
            /* check if null */
            if (curFile)
            {
                /* forward traverse whole files */
                do
                {
                    /* delete file */
                    dcfDel((s8*)curFile->pDirEnt->d_name,0);

                    /* next file */
                    curFile = curFile->next;
                }
                while (curFile != dcfListFileEntHead);
            }
            dcfFileTypeCount_Clean();
            global_totalfile_count = 0;
        }
    }

    /* delete directory */
    dcfChDir("..");
    dcfRmDir((s8*)dcfPlaybackCurDir->pDirEnt->d_name,1);
    global_totaldir_count--;
    dcfListDirCount --;

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurDir->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurDir->used_flag = DCF_FILE_USE_NONE;
#endif

    if(dcfListDirEntHead != dcfListDirEntTail)
    {
        //above 2 files
        dcfPlaybackCurDir->prev->next = dcfPlaybackCurDir->next;
        dcfPlaybackCurDir->next->prev = dcfPlaybackCurDir->prev;
        if (dcfPlaybackCurDir == dcfListDirEntHead)
            dcfListDirEntHead = dcfPlaybackCurDir->next;
        dcfListDirEntTail = dcfListDirEntHead->prev;
        dcfPlaybackCurDir = dcfPlaybackCurDir->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurDir = dcfListDirEntHead = dcfListDirEntTail = NULL;
        dcfDirNumLast =0;
        global_totaldir_count=0;
        dcfListDirCount=0;
    }
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelDir\n");

    return 1;

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
    char* pExtName ;
    char extName[4];
    char j;
    u8 ucfiletype;
    INT8U err;

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    char LogFileName[32];
#endif

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
    {
        return dcfPlaybackDelPic();
    }
#endif
    DEBUG_DCF("Enter dcfPlaybackDel....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (dcfListFileEntHead == NULL)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Trace: No current file.\n");
        return 0;
    }
    pExtName=&dcfListFileEntHead->pDirEnt->d_name[9];
    for (j = 0; j < 3; j++)
    {
        extName[j] = pExtName[j];
        if ((extName[j] >= 'a') && (extName[j] <= 'z'))
            extName[j] -= 0x20;
    }
    extName[j]= '\0' ;

    ucfiletype = dcfGetFileType(extName);

    if (ucfiletype ==DCF_FILE_TYPE_MAX)
    {
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("Error: file type doesn't exist, %d\n", ucfiletype);
        return 0;
    }

    dcfFileTypeCount_Dec(ucfiletype);
    global_totalfile_count--;

    osdDrawDelMsg((s8*)dcfPlaybackCurFile->pDirEnt->d_name,0);
    dcfDel((s8*)dcfPlaybackCurFile->pDirEnt->d_name,dcfPlaybackCurFile->pDirEnt->FileEntrySect);

#if ((CDVR_LOG || CDVR_TEST_LOG)&&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))

    strncpy(LogFileName, (s8*)dcfPlaybackCurFile->pDirEnt->d_name,9);
    LogFileName[9]  = 0;
    strcat(LogFileName, "LOG");

    if (dcfDel((s8*)LogFileName,dcfPlaybackCurFile->pDirEnt->FileEntrySect)<=0)
    {
        DEBUG_DCF("log File: %s not found \n",LogFileName);
        //fix me =>Albert
        //return 0;
    }
#endif


#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    dcfPlaybackCurFile->pDirEnt->used_flag = DCF_FILE_USE_NONE;
    dcfPlaybackCurFile->used_flag = DCF_FILE_USE_NONE;
#endif

    //Albert Modified on 20091209
    if(dcfListFileEntHead != dcfListFileEntTail)
    {
        //above 2 files
        dcfPlaybackCurFile->prev->next = dcfPlaybackCurFile->next;
        dcfPlaybackCurFile->next->prev = dcfPlaybackCurFile->prev;
        if (dcfPlaybackCurFile == dcfListFileEntHead)
            dcfListFileEntHead = dcfPlaybackCurFile->next;
        dcfListFileEntTail = dcfListFileEntHead->prev;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    else
    {
        //just a file
        dcfPlaybackCurFile = dcfListFileEntHead = dcfListFileEntTail = NULL;
        dcfFileNumLast =0;
    }

    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDel\n");
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
    DCF_LIST_DIRENT* curDir;
    DCF_LIST_FILEENT* curFile;
    u32 index=0;
    INT8U err;

    DEBUG_DCF("Enter dcfPlaybackDelAll....\n");
    //OSFlagPend(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    /* delete all files */
    /* set current directory */
    curDir = dcfListDirEntHead;
    system_busy_flag=1;

    /* forward traverse whole directories */
    //Lucian: 從File Link 頭砍到尾. 先砍檔案再砍目錄. 再進下一個目錄. and so on..
    do
    {
        /* file initialization */
        if (dcfFileInit(curDir, CURRENTFILEENTRY) == 0)
        {
            system_busy_flag=0;
            return 0;
        }
        /* set current file */
        curFile = dcfListFileEntHead;

        /* check if null */
        if (curFile)
        {
            /* forward traverse whole files */
            do
            {
                /* delete file */
                index++;
                osdDrawDelMsg((s8*)curFile->pDirEnt->d_name,index);
                dcfDel((s8*)curFile->pDirEnt->d_name,0);

                /* next file */
                curFile = curFile->next;
            }
            while (curFile != dcfListFileEntHead);
        }

        /* delete directory */
        dcfChDir("..");
        dcfRmDir((s8*)curDir->pDirEnt->d_name,1);

        global_totaldir_count--;

        /* next directory */
        curDir = curDir->next;
    }
    while (curDir != dcfListDirEntHead);

    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    /* directory initialization */
    if (dcfDirInit() == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfDirInit() == 0\n");
        return 0;
    }
    /* file initialization */
    if (dcfFileInit(dcfListDirEntTail,CURRENTFILEENTRY) == 0)
    {
        system_busy_flag=0;
        //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
        OSSemPost(dcfReadySemEvt);
        DEBUG_DCF("PlaybackDelAll dcfFileInit Err\n");
        return 0;
    }
    playback_location=0xFF;
    system_busy_flag=0;
    //OSFlagPost(gDcfFlagGrp, DCF_FLAG_CREATE_FILE, OS_FLAG_CLR, &err);
    OSSemPost(dcfReadySemEvt);
    DEBUG_DCF("Leave dcfPlaybackDelAll....\n");
    return 1;

}

#endif		//(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)


//-------Customize Issue: Salix 要求在NAND 放入AP used in PC. 在卡插入時,將AP in Nand Copy to SD card. --------//
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
//Lucian: 用於APP 存於NAND flash,開機後將APP 讀出到SD card.
void Backup_Path(void)
{
    memcpy(Temp_dcfCurDrive,dcfCurDrive,64);  // Recover path
    memcpy(Temp_dcfCurDir,dcfCurDir,64);
    memcpy(Temp_dcfCurPath,dcfCurPath,64);
    memcpy(Temp_dcfTargetPath,dcfTargetPath,64);
}

void Recover_Path(void)
{
    memcpy(dcfCurDrive,Temp_dcfCurDrive,64);  // Recover path
    memcpy(dcfCurDir,Temp_dcfCurDir,64);
    memcpy(dcfCurPath,Temp_dcfCurPath,64);
    memcpy(dcfTargetPath,Temp_dcfTargetPath,64);
}
#endif

#if RX_SNAPSHOT_SUPPORT
//Lucian: 以月為單位,搜尋該月份(Year-Month) 以前的所有檔案,符合的 dcfPhotoMonthDistr[] +=1.
int dcfPlaybackPhotoMapInit(int CHsel,int Year,int Month)
{
    u32 i;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;
    s8 DirName[32];
    s8 PlayTargetPath[64];
    FS_DIR* pDir;

    //------------------------------//
    dcfListReadFileEntHead = NULL;
    dcfListReadFileEntTail = NULL;
    dcfPlaybackCurFile = NULL;
    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    dcfReadFileCount=0; //civicwu 070926
    memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

    memset(dcfPhotoMonthDistr,0,sizeof(dcfPhotoMonthDistr));
    sprintf ((char*)DirName, "\\%s\\CH%d", gsPhotoDirName,CHsel+1);

    DEBUG_DCF("Directory Name: %s\n",DirName);

    /* change current directory */
    if (dcfChPlayDir(DirName) == 0)
    {
        /* change current directory error */
        DEBUG_DCF("Error: Change current directory failed.\n");
        return 0;
    }

    strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);

    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {
        /* pDirName exists */
        DEBUG_DCF("dcfPlaybackPhotoMapInit Path: %s \n",PlayTargetPath);
        dcfReadFileCount=FS_MapPhotoDir(pDir,dcfReadFileEnt,dcfBuf,DCF_FILEENT_MAX,&OldestEntry,Year,Month,dcfPhotoMonthDistr);
        FS_CloseDir(pDir);
    }
    else
    {
        /* pDirName not exists */
        DEBUG_DCF("Error: dcfPlaybackPhotoMapInit %s failed.\n", PlayTargetPath);
        return 0;
    }

    /* sort the file and create the file list */
    memset_hw((void*)dcfListReadFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

    dcfListReadFileCount = dcfReadFileCount;
    global_totalfile_count = dcfReadFileCount;

    if(dcfReadFileCount==0)
    {
        dcfListReadFileEntHead=NULL;
    }
    else if(dcfReadFileCount==1)
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[0];
        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[0];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];
    }
    else
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[OldestEntry];

        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->next = &dcfListReadFileEnt[1];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];

        ins ++;

        for (i = 1; i < dcfReadFileCount-1; i++)
        {
            ins = &dcfListReadFileEnt[i];
            ins->prev = &dcfListReadFileEnt[i-1];
            ins->next = &dcfListReadFileEnt[i+1];

            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[i];
            ins ++;
        }

        ins = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-2];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[dcfReadFileCount-1];
    }


    /* set tail entry of file list */
    if( dcfListReadFileEntHead != NULL)
    {
        dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
        dcfListReadFileEntTail->next =dcfListReadFileEntHead;
    }
    else
    {
        dcfListReadFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile = dcfListReadFileEntTail;
    return 1;

}

int dcfPlaybackPhotoFileInit(int CHsel,int Year,int Month)
{
    u32 i;
    unsigned int OldestEntry;
    DCF_LIST_FILEENT* ins;
    s8	DirName[32];
    s8 PlayTargetPath[64];

    //------------------------------//
    dcfListReadFileEntHead = NULL;
    dcfListReadFileEntTail = NULL;
    dcfPlaybackCurFile = NULL;
    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    dcfReadFileCount=0; //civicwu 070926
    memset_hw((void*)dcfReadFileEnt, 0, sizeof(struct FS_DIRENT) * DCF_FILEENT_MAX); //civicwu 070926

    sprintf ((char*)DirName, "\\%s\\CH%d", gsPhotoDirName,CHsel+1);

    DEBUG_DCF("Directory Name: %s\n",DirName);

    /* change current directory */
    if (dcfChPlayDir(DirName) == 0)
    {
        /* change current directory error */
        DEBUG_DCF("Error: Change current directory failed.\n");
        return 0;
    }
    //strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);

    /* list current directory */
    if (dcfPlayPhotoDirSearch(DirName, dcfReadFileEnt, &dcfReadFileCount,&OldestEntry,0,Year,Month) == 0) //Lucian: 搜尋不同目錄, 故不update section-entry position.
    {
        /* list current directory error */
        DEBUG_DCF("Error: List current directory failed.\n");
        return 0;
    }

    /* sort the file and create the file list */
    memset_hw((void*)dcfListReadFileEnt, 0, sizeof(DCF_LIST_FILEENT) * DCF_FILEENT_MAX);

    dcfListReadFileCount = dcfReadFileCount;
    global_totalfile_count = dcfReadFileCount;

    if(dcfReadFileCount==0)
    {
        dcfListReadFileEntHead=NULL;
    }
    else if(dcfReadFileCount==1)
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[0];
        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[0];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];
    }
    else
    {
        dcfListReadFileEntHead = &dcfListReadFileEnt[OldestEntry];

        ins = &dcfListReadFileEnt[0];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->next = &dcfListReadFileEnt[1];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[0];

        ins ++;

        for (i = 1; i < dcfReadFileCount-1; i++)
        {
            ins = &dcfListReadFileEnt[i];
            ins->prev = &dcfListReadFileEnt[i-1];
            ins->next = &dcfListReadFileEnt[i+1];

            ins->used_flag = DCF_FILE_USE_CLOSE;
            ins->pDirEnt = &dcfReadFileEnt[i];
            ins ++;
        }

        ins = &dcfListReadFileEnt[dcfReadFileCount-1];
        ins->prev = &dcfListReadFileEnt[dcfReadFileCount-2];
        ins->next = &dcfListReadFileEnt[0];
        ins->used_flag = DCF_FILE_USE_CLOSE;
        ins->pDirEnt = &dcfReadFileEnt[dcfReadFileCount-1];
    }


    /* set tail entry of file list */
    if( dcfListReadFileEntHead != NULL)
    {
        dcfListReadFileEntTail = dcfListReadFileEntHead->prev;
        dcfListReadFileEntTail->next =dcfListReadFileEntHead;
    }
    else
    {
        dcfListReadFileEntTail =NULL;
    }

    /* set current playback file */
    dcfPlaybackCurFile = dcfListReadFileEntTail;

    return 1;


}


int dcfPlaybackPhotoDelAll(void)
{
    DCF_LIST_FILEENT* curFile;
    u32 index=0;
    //----------------//

    curFile = dcfListReadFileEntHead;

    /* check if null */
    if (curFile)
    {
        /* forward traverse whole files */
        do
        {
            /* delete file */
            index++;
            osdDrawDelMsg((s8*)curFile->pDirEnt->d_name,index);
            dcfPlayDel((s8*)curFile->pDirEnt->d_name,0);

            /* next file */
            curFile = curFile->next;
            if(index > DCF_FILEENT_MAX)
                break;
        }
        while (curFile != dcfListReadFileEntHead);
    }

}

#endif

#if ((UI_VERSION == UI_VERSION_RDI_4) || ((UI_VERSION == UI_VERSION_TRANWO) && (OPEN_ENDUSER_SDUPGRADE == 1)))
void dcfDirRenamePa9spiu(void)
{
    DEBUG_DCF("dcfDirPa9spiuRename: Begin \n");
    memset_hw((void*)dcfDirEnt, 0, sizeof(struct FS_DIRENT) * DCF_DIRENT_MAX);
    if (dcfDir("\\", dcfDirEnt, &dcfDirCount,1,1,DCF_DIRENT_MAX) == 0)
        DEBUG_DCF("Warning:List root directory empty\n");
    uiCheckUpgradeFileName(dcfDirEnt, dcfDirCount);
    DEBUG_DCF("dcfDirPa9spiuRename: End \n");
}
#endif
