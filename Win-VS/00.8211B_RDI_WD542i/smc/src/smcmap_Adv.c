/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcmap.c

Abstract:

   	The routines of SMC and NAND gate flash related to logical to physical mapping.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#if (FLASH_OPTION == FLASH_NAND_9002_ADV)

#include "board.h"
#include "smc.h"
#include "smcreg.h"
#include "smcwc.h"
#include "smcapi.h" /*CY 1023*/
#include "fsapi.h"
#include "ispapi.h"
#include "uiapi.h"
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* Max. zone */
#define SMC_MAX_ZONE				8
#define SMC_MAX_ZONE_ENTRY			1024
#define SMC_MAX_ZONE_BLOCK			1000

#define FAT_READ_DEBUG
#undef	FAT_READ_DEBUG

#define FAT_WRITE_DEBUG
#undef	FAT_WRITE_DEBUG

#define GET_MIN(a, b)	(a<b? a:b)	

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

// DefectPixel
// SSDV_1




u8 UI_update;
u8 Rerseved_Algo_Start=0;
u8 Got_BitMap=0;

extern u32 smcTotalSize, smcBlockSize, smcPageSize, smcPagePerBlock;
extern u32 smcPageRedunSize;
extern u32 smcTotalZone;
extern u32 smcAddrCycle;
extern u32	smcSecPerBlock;
extern u32	smcSecPerPage;
extern u32 smcSectionNum;
extern u8*	smcGeneralBuf;

extern u8 iconflag[UIACTIONNUM];  //civic 071001
extern s8 defaultvalue[]; //civic 071001
//extern SMC_REDUN_AREA smcRedunArea;
extern u8 userClickFormat;

extern s32 siuWBComp_RBRatio[4];
extern u8 cur_ev_value;

extern FRAME_BUF_OBJECT frame_buf_obj[MAX_FRM_OBJ];

extern u8	ucTestIdx;


/*--------For Adv NAND--------*/
//u32	unSecOffsetInBlock;

/*========For Adv NAND========*/

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/* Application Function */

s32 smcSectorsRead(u32, u32, u8*);
s32 smcPagesRead(u32, u32, u8*);
s32 smcSectorsWrite(u32, u32, u32, u8*);
s32 smcPagesWrite(u32, u32, u8*);
u8	smcGetBitMap(void);
u32 smcCheckBadBlock(u32);

/* Driver Function */

void CoordinateConvert(void);



extern s32 smcSetReadWriteAddr(u32);
extern s32 smcSetEraseAddr(u32);
//extern s32 smcGetRedunArea(SMC_REDUN_AREA*);
extern s32 smcSetRedunArea(u16);

/* Middleware Function */
extern s32 smcIdentification(void);
extern s32 smcReset(void);
extern s32 smcReadStatus(u8*);
extern s32 smcIdRead(u8*, u8*);
extern s32 smcPage1ARead(u32, u8*);
extern s32 smcPage1BRead(u32, u8*);
extern s32 smcPage2CRead(u32);
extern s32 smcPageProgram(u32, u8*, u16);
extern s32 smcBlockErase(u32);
extern s32 smcMultiBlockErase(u32, u32);

/*
 *********************************************************************************************************
 *  Application function
 *********************************************************************************************************
 */

// default_flag is only valid for write mode
// smcReadWriteUI(0,1) : Read UI setting from NAND to buffer if it's empty, it will set default state
//smcReadWriteUI(1,0) : Write UI default setting from buffer to NAND.
//smcReadWriteUI(0,0) : Write UI setting from buffer to NAND. If it wil update when power off.
u8 smcReadWriteUI(u8 default_flag, u8 ReadWrite)
{
    u32 UI_logAddr = SMC_SYS_PARAMETER_ADDR + SMC_UI_SECTOR_ADDR*SMC_MAX_PAGE_SIZE;	// Reservese size for UI

    u8 i, check_val=0;
    BOOLEAN check = FALSE;
    if (ReadWrite)
    {		//read

        //Read icon flag from NAND
#if 0
        if (smcSectorsRead(1,UI_logAddr,smcReadBuf)==0)
#else
        if (smcPagesRead(1,UI_logAddr,smcReadBuf)==0)
#endif
        {
            DEBUG_SMC("Read error on logical address %#x\n",UI_logAddr);
            return 0;
        }
        // Read UI setting from NAND
        memcpy ((void *)iconflag, (void *)smcReadBuf, UIACTIONNUM);

        for (i = 0; i < UIACTIONNUM-1; i++)
            check_val += iconflag[i];
        if (iconflag[UI_MENU_SETIDX_CHECK] == (check_val + UI_SET_CHECKSUM))
            check = TRUE;

        /* used default setting*/
        if (check == FALSE)
        {
            for (i = 0; i < UIACTIONNUM; i++)
                iconflag[i] = defaultvalue[i+LEVEL1_NODE_NUM];
        }
    
        iconflag[UI_MENU_SETIDX_VIDEO_FORMAT-LEVEL1_NODE_NUM] = UI_MENU_FORMAT_NO;
        iconflag[UI_MENU_SETIDX_DELETE-LEVEL1_NODE_NUM] = UI_MENU_DELETE_NO;
        iconflag[UI_MENU_SETIDX_DELETE_ALL-LEVEL1_NODE_NUM] = UI_MENU_DELETE_ALL_NO;
        iconflag[UI_MENU_SETIDX_FLASH_LIGHT-LEVEL1_NODE_NUM] = UI_MENU_FLASH_LIGHT_ALWAYS_OFF;
        cur_ev_value = iconflag[UI_MENU_SETIDX_EXPOSURE_VALUE - LEVEL1_NODE_NUM];

        UI_update=0xAB;	//Magic number
    }
    else
    {		//write
        if (default_flag)
            // Set All UI to default state
        {
            memset((char*)smcReadBuf,0x00,smcPageSize);
            for (i=0;i<26;i++)
                iconflag[i] = defaultvalue[i+4];

            memcpy ((void *)smcReadBuf,(void *)iconflag,  UIACTIONNUM);
            UI_update=0xAA;	//Magic number
        }
        else
        {	//update icon flag to NAND
            memset((char*)smcReadBuf,0x00,smcPageSize);
            memcpy ((void *)smcReadBuf,(void *)iconflag,  UIACTIONNUM);
            UI_update=0xAA;	//Magic number

        }

    }


    return 1;

}

/*

Routine Description:

	Refresh and use the updated UI library.

Arguments:

	None.

Return Value:

	None

*/
s8 smcReadFBSetting(void)
{
    u32 i;
    u32 FB_addr;
    u8* frmptr= smcGeneralBuf;

    if (Got_BitMap==0)
    {
        Got_BitMap = smcGetBitMap();
    }

    i = SMC_UI_LIBRARY_ADDR;
    while (smcPage1ARead(i, smcReadBuf) == 0)
    {	/* invalid block */
        i += smcBlockSize;
        DEBUG_SMC("Page Read Error in Read FB Setting.\n");
        if (i >= SMC_RESERVED_SIZE)
        {
            DEBUG_SMC("Error: Too many invalid blocks to parameter block.\n");
            return 0;
        }
    }

    FB_addr=i;

    for (i=0;i<(sizeof(FRAME_BUF_OBJECT)*(MAX_FRM_OBJ) + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE;i++)
    {
        if (smcPage1ARead(FB_addr, frmptr) == 0)
        {
            DEBUG_SMC("Error! Read error at FB_addr address %#x\n", FB_addr);
            return 0;
        }
        frmptr+=SMC_MAX_PAGE_SIZE;
        FB_addr+=SMC_MAX_PAGE_SIZE;
    }

    frmptr= smcGeneralBuf;
    memcpy( (void*)frame_buf_obj[0].file_name,frmptr,sizeof(FRAME_BUF_OBJECT)*(MAX_FRM_OBJ));

    // Check total file in ISP_UI  SDV2 41: latest SDV1 40
    for (i=0;i<MAX_FRM_OBJ;i++)
    {
        if (frame_buf_obj[i].fb_magic!=0xA5A55A5A)
            break;
    }

    // DefectPixel // read -> DEFECT_PIXEL_COORDINATE_DATA data
    CoordinateConvert();

    return 1;

}

void CoordinateConvert(void)
{

}

void sysCaptureImage_DefectPixel_Default(void)
{

}




u8 smcReadAWBSetting(void)
{
    u32 AWB_logAddr = SMC_SYS_PARAMETER_ADDR + SMC_AWB_SECTOR_ADDR*SMC_MAX_PAGE_SIZE;

#if 0
    if (smcSectorsRead(SMC_AWB_SECTOR_SIZE, AWB_logAddr, smcReadBuf)==0)
#else
    if (smcPagesRead(SMC_AWB_SECTOR_SIZE, AWB_logAddr, smcReadBuf)==0)
#endif
    {
        DEBUG_SMC("Read error on logical address %#x\n",AWB_logAddr);
        return 0;
    }

    // Read UI setting from NAND
    memcpy ((void *)siuWBComp_RBRatio, (void *)smcReadBuf, 16);

    if (siuWBComp_RBRatio[3] != 0x12345678) //確認內部值是否有效
    {
        siuWBComp_RBRatio[0]=256; //R_CompRatio
        siuWBComp_RBRatio[1]=256; //B_CompRatio
        siuWBComp_RBRatio[2]=0;   //Version
    }

    return 1;
}

u8 smcUIopen(u8* targetStr)
{
    u16 strLen;
    u16 i,j;
    u8 MainName[8],flag=0;
    u8* targetBuf=exifDecBuf;
    u32 targetSec;
    strLen=strlen((char*)targetStr);
    strLen=strLen-4;  // we didn't compare the .bin extention file name

    for (i = 0; i < 8; i++)     // Since we just support 8.3 format
    {
        MainName[i] = targetStr[i];
        if ((MainName[i] >= 'a') && (MainName[i] <= 'z'))
            MainName[i] -= 0x20;
    }

    for (i=0;i<MAX_FRM_OBJ;i++) // Get data from frame object
    {
        if (frame_buf_obj[i].fb_magic!=0xA5A55A5A)
            continue;

        if (strncmp(MainName, frame_buf_obj[i].file_name, strLen) == 0)
        {       //Got it exifDecBuf
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

            targetSec=frame_buf_obj[i].sector_addr;
            DEBUG_SMC("Trace: fileSize = 0x%08x,\n", frame_buf_obj[i].file_length);
            for (j=0;j<frame_buf_obj[i].len_in_nand/SMC_MAX_PAGE_SIZE;j++)
            {
                if ((targetSec % smcBlockSize) == 0)
                {
                    if (smcPage1ARead(targetSec, targetBuf) == 0)
                    {	/* invalid block */
                        targetSec += smcBlockSize;
                        continue;
                    }
                    else
                    {
                        targetBuf+=SMC_MAX_PAGE_SIZE;
                        targetSec+=SMC_MAX_PAGE_SIZE;
                        continue;
                    }
                }
                smcPage1ARead(targetSec, targetBuf);
                targetBuf+=SMC_MAX_PAGE_SIZE;
                targetSec+=SMC_MAX_PAGE_SIZE;
            }
            flag=1;
            break;
        }

    }

    if (gInsertCard==1)
    {
        sysNAND_Disable();
        sysSD_Enable();
    }

    return flag;
}


u8 smcGetBitMap(void)
{
    u8	i;
    u8* readBuf=smcBitMap;
    u32 BitMap_logAddr;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: GET BIT MAP\n");
#endif

    smcCheckTotalBlock();
    BitMap_logAddr = SMC_MAP_ADDR;	// Reservese size + 1 block

    for (i=0; i<SMC_MAX_MAP_SIZE_IN_PAGE; i++)
    {
        if (smcPagesRead(1, BitMap_logAddr, readBuf)==0)
        {
            DEBUG_SMC("Read error on logical address %#x\n",BitMap_logAddr);
            return 0;
        }
        BitMap_logAddr += smcPageSize;
        readBuf += smcPageSize;
    }

    return 1;
}

void smcMakeMBR(void)
{
    u8* buffer=smcMBRCache;
    //u32 logAddr;
    memset((void*)buffer,0x00,smcPageSize);

    buffer[0]=0xE9;

    // String Identifies MSDOS 5.0
    buffer[3]=0x4D;		/* character 'M' */
    buffer[4]=0x53;		/* character 'S' */
    buffer[5]=0x44;		/* character 'D' */
    buffer[6]=0x4F;		/* character 'O' */
    buffer[7]=0x53;		/* character 'S' */
    buffer[8]=0x35;		/* character '5' */
    buffer[9]=0x2E;		/* character '.' */
    buffer[0x0A]=0x30;		/* character '0' */

    /* Bytes Per Sector */
    buffer[0x0B]= FS_SECTOR_SIZE & 0xFF;
    buffer[0x0C]=(FS_SECTOR_SIZE>>8) & 0xFF;

    /* Sector Per Cluster */
    buffer[0x0D]=SEC_PER_CLS;

    //RsvdSecCnt
    buffer[0x0E]=RSVD_SEC_CNT & 0xFF;		//just reserve 2 sector for BPB
    buffer[0x0F]=(RSVD_SEC_CNT >> 8) & 0xFF;
    //NUM_FATS
    buffer[0x10]=NUM_FATS;		//0x02

    //RootEntCnt
    buffer[0x11]=ROOT_ENTRY_CNT & 0xFF;
    buffer[0x12]=(ROOT_ENTRY_CNT>>8) & 0xFF;

    //TotSec16
    buffer[0x13]=TOSECT16 & 0xFF;
    buffer[0x14]=(TOSECT16>>8) & 0xFF;

    /* Media Type*/		/* 0xF8 -> Single Side, 80 track each side, 9 sectors each track */
    buffer[0x15]=0xF8;

    //FATSz16
    buffer[0x16]=FATSZ16 & 0xFF;
    buffer[0x17]=(FATSZ16>>8) & 0xFF;

    //TotSec32
    buffer[0x20]= (TOSECT32) & 0xFF;
    buffer[0x21]=(TOSECT32>>8) & 0xFF;
    buffer[0x22]=(TOSECT32>>16) & 0xFF;
    buffer[0x23]=(TOSECT32>>24) & 0xFF;
    // Extended Boot Signature
    buffer[0x26]=0x29;

    // File System Type      FAT12 or FAT16
    buffer[0x36]=0x46;		/* character 'F' */
    buffer[0x37]=0x41;		/* character 'A' */
    buffer[0x38]=0x54;		/* character 'T' */

    if (smcTotalSize > SMC_32MB)
    {
        buffer[0x39]=0x31;		/* character '1' */
        buffer[0x3A]=0x36;		/* character '6' */
    }
    else
    {
        buffer[0x39]=0x31;		/* character '1' */
        buffer[0x3A]=0x32;		/* character '2' */
    }

    //Signature
    buffer[FS_FAT_SEC_SIZE-2]=0x55;
    buffer[FS_FAT_SEC_SIZE-1]=0xAA;

    // Read The MBR and fill the NAND parameter
    NAND_FAT_PARAMETER.RsvdSecCnt = RSVD_SEC_CNT;
    // Get the new FAT parameter
    NAND_FAT_PARAMETER.BytesPerSec = FS_SECTOR_SIZE;
    NAND_FAT_PARAMETER.FATSz16 = FATSZ16;
    NAND_FAT_PARAMETER.NumFATs = NUM_FATS;
    NAND_FAT_PARAMETER.RootEntCnt = ROOT_ENTRY_CNT;
    NAND_FAT_PARAMETER.SecPerClus = SEC_PER_CLS;    // Important parameter it will affect NAND behavior
    NAND_FAT_PARAMETER.TotSec16 = TOSECT16;

    // calculate the essential NAND FAT cache parameter
    nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
    nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs*NAND_FAT_PARAMETER.FATSz16;
    nand_fat_size = NAND_FAT_PARAMETER.FATSz16;

    /* Data start sector */
    nand_data_start = nand_fat_end + ((NAND_FAT_PARAMETER.RootEntCnt * 0x20) / FS_SECTOR_SIZE);


#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: Write MBR.\n");
#endif

    if (smcSectorsWrite(1, 0, SMC_FAT_START_ADDR, (u8*)buffer)==0)
    {
        DEBUG_SMC("SMC ERR: Write MBR error!\n");
    }

#ifdef FAT_WRITE_DEBUG
    {
        u32 i;
        u8*	pucDst = (u8*) smcReadBuf;
        //DEBUG
        DEBUG_SMC("\n-----MBR Read Test-----\n");
        if (smcSectorsRead(1, 0, pucDst) == 0)
        {
            DEBUG_SMC("Read error\n");
        }
        for (i=0; i<512; i++)
            if (*(pucDst+i) != *(buffer+i))
            {
                DEBUG_SMC("buffer[%#x] = %#x\n", i, *(buffer+i));
                DEBUG_SMC("pucDst[%#x] = %#x\n", i, *(pucDst+i));
            }

    }
#endif
}

u8 smcMakeBitMap(char jump_flag)        //0 --> Force format all NAND
{
    // Read Bit MAP
    u8	i;
    u32	unSecNumTemp;
    u32 fat_magic;
    u8*	pucBufTemp;
    u8*	pucMBRBuf = smcMBRCache;		/* smcMBRCahce is for following FAT using to reduce process time, it is neede to be stored here. */


    if (jump_flag == 0)
        goto FORMAT;

    if (Got_BitMap == 0)
    {
        Got_BitMap = smcGetBitMap();
    }

    /* read out MBR sector */
    /* MBR is stored in the first sector. Read one page can reduce the read cycle time. */
    if (smcSectorsRead(1, 0, pucMBRBuf) == 0)
    {
        DEBUG_SMC("Read error on physical address %#x\n", SMC_FAT_START_ADDR);
        return 0;
    }


    if ((*(u8*)pucMBRBuf)==0xE9 || *(u8*)pucMBRBuf==0xEB )
    {	/* The MRB are stored valid data. The media are not needed to be formated. */

        // Read The MBR and fill the NAND parameter
        NAND_FAT_PARAMETER.RsvdSecCnt = pucMBRBuf[0x0E] + (pucMBRBuf[0x0F]<<8);
        // Get the new FAT parameter
        NAND_FAT_PARAMETER.BytesPerSec = pucMBRBuf[0x0B] + (pucMBRBuf[0x0C]<<8);
        NAND_FAT_PARAMETER.FATSz16 = pucMBRBuf[0x16] + (pucMBRBuf[0x17]<<8);
        NAND_FAT_PARAMETER.NumFATs = pucMBRBuf[0x10];
        NAND_FAT_PARAMETER.RootEntCnt = pucMBRBuf[0x11] + (pucMBRBuf[0x12]<<8);
        NAND_FAT_PARAMETER.SecPerClus = pucMBRBuf[0x0D];	 // Important parameter it will affect NAND behavior
        NAND_FAT_PARAMETER.TotSec16= pucMBRBuf[0x13] + (pucMBRBuf[0x14]<<8);
        NAND_FAT_PARAMETER.TotSec32= ((pucMBRBuf[0x23]<<24) + (pucMBRBuf[0x22]<<16) + (pucMBRBuf[0x21]<<8) + pucMBRBuf[0x20]);			/* not support SMC size greater than 2GB */

        // calculate the essential NAND FAT cache parameter
        nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
        nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs * NAND_FAT_PARAMETER.FATSz16;
        nand_fat_size = NAND_FAT_PARAMETER.FATSz16;		/* Sector Counts of FAT */

        unSecNumTemp = ((NAND_FAT_PARAMETER.RootEntCnt * 0x20) / FS_SECTOR_SIZE);

        /* Data start sector */
        nand_data_start = nand_fat_end + unSecNumTemp;



#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("\nMakeBitMap\n");
        DEBUG_SMC("NAND_FAT_PARAMETER.RsvdSecCnt = %#x\n", NAND_FAT_PARAMETER.RsvdSecCnt);
        DEBUG_SMC("NAND_FAT_PARAMETER.BytesPerSec = %#x\n", NAND_FAT_PARAMETER.BytesPerSec);
        DEBUG_SMC("NAND_FAT_PARAMETER.FATSz16 = %#x\n", NAND_FAT_PARAMETER.FATSz16);
        DEBUG_SMC("NAND_FAT_PARAMETER.NumFATs = %#x\n", NAND_FAT_PARAMETER.NumFATs);
        DEBUG_SMC("NAND_FAT_PARAMETER.RootEntCnt = %#x\n", NAND_FAT_PARAMETER.RootEntCnt);
        DEBUG_SMC("NAND_FAT_PARAMETER.SecPerClus = %#x\n", NAND_FAT_PARAMETER.SecPerClus);
        DEBUG_SMC("NAND_FAT_PARAMETER.TotSec16 = %#x\n", NAND_FAT_PARAMETER.TotSec16);

        DEBUG_SMC("nand_fat_start = %#x\n", nand_fat_start);
        DEBUG_SMC("nand_fat_end = %#x\n", nand_fat_end);
        DEBUG_SMC("nand_fat_size = %#x\n", nand_fat_size);
        DEBUG_SMC("nand_data_start = %#x\n\n", nand_data_start);

        DEBUG_SMC("MBR START ADDR = %#x\n", SMC_FAT_START_ADDR);
        DEBUG_SMC("FAT1 START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * nand_fat_start);
        DEBUG_SMC("FAT2 START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16));
        DEBUG_SMC("ROOT Entry START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16*2));
        DEBUG_SMC("DATA START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16 * 2) + NAND_FAT_PARAMETER.RootEntCnt * 0x20);
#endif

#if 0
        DEBUG_SMC("SMC_CODE_END_ADDR = %#x\n", SMC_CODE_END_ADDR);
        DEBUG_SMC("SMC_UI_LIBRARY_ADDR = %#x\n", SMC_UI_LIBRARY_ADDR);
        DEBUG_SMC("SMC_SYS_PARAMETER_ADDR = %#x\n", SMC_SYS_PARAMETER_ADDR);
        DEBUG_SMC("SMC_DEFECT_PIXEL_ADDR = %#x\n", SMC_DEFECT_PIXEL_ADDR);
        DEBUG_SMC("SMC_MAP_ADDR = %#x\n", SMC_MAP_ADDR);
        DEBUG_SMC("SMC_FAT_START_ADDR (MBR) = %#x\n", SMC_FAT_START_ADDR);
        DEBUG_SMC("MBR START ADDR = %#x\n", SMC_FAT_START_ADDR);
        DEBUG_SMC("FAT1 START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * nand_fat_start);
        DEBUG_SMC("FAT2 START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16));
        DEBUG_SMC("ROOT Entry START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16*2));
        DEBUG_SMC("DATA START ADDR = %#x\n", SMC_FAT_START_ADDR + FS_SECTOR_SIZE * (nand_fat_start + NAND_FAT_PARAMETER.FATSz16*2+NAND_FAT_PARAMETER.RootEntCnt) );
#endif
    }
    else
    {
        /* Neede to be formated */
        jump_flag = 0;
    }

    // Read out the NAND FAT data in first sector
    pucBufTemp = &smcWriteCache[0][0];    //FAT

    if (smcSectorsRead(1, nand_fat_start, pucBufTemp)==0)
    {
        DEBUG_SMC("Read error on The FAT-1 address %#x\n", SMC_FAT_START_ADDR + nand_fat_start*FS_SECTOR_SIZE);
        return 0;
    }

    // This is the initial if this is first run then it will record to nand
    fat_magic = *(u32*)pucBufTemp;

#ifdef FAT_WRITE_DEBUG
#if 0
    DEBUG_SMC("DEBUG: fat_magic = %x\n", fat_magic);
    DEBUG_SMC("DEBUG: jump_flag = %d\n", jump_flag);
#endif
#endif
    /* this fat_magic is for FAT16 */
    if (fat_magic!=0xFFFFFFF8 || jump_flag!=1)
    {
        jump_flag = 0;
    }
    else
    {
        return 1;		// don't need to first initial
    }

FORMAT:
    DEBUG_SMC("Format SMC Media\n");

    smcTotalBlockErase(jump_flag);
    if (jump_flag != 1)
    {       // Make Bit Map
        memset((void*)smcBitMap, 0xFF, SMC_MAX_MAP_SIZE_IN_BYTE);  // Need to modify
    }

    smcMakeMBR();

    pucBufTemp = &smcWriteCache[0][0];
    memset(pucBufTemp, 0x00, FS_SECTOR_SIZE * FATSZ16);

    *(u32*)pucBufTemp = 0xFFFFFFF8;	/* This is for FAT16 */

    /* write two FATs into flash */
    for (i=0; i<NUM_FATS; i++)
    {
#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("\nDEBUG: write FAT-%d\n", i+1);
#endif
        if (smcSectorsWrite(nand_fat_size, nand_fat_start + i*nand_fat_size, SMC_FAT_START_ADDR, (u8*)pucBufTemp) == 0)		/* In usual, only the FAT-1 is used. */
        {
            DEBUG_SMC("Write FAT-%d error in MakeBitMap, address is %#x.\n", (i+1), SMC_FAT_START_ADDR + (nand_fat_start + i*nand_fat_size)*FS_SECTOR_SIZE);
        }

//		smcPage1ARead(SMC_FAT_START_ADDR + (nand_fat_start + i*nand_fat_size)*FS_SECTOR_SIZE, smcReadBuf);
//		smcSectorsRead(1, nand_fat_start + i*nand_fat_size, smcReadBuf);
//		DEBUG_SMC("FAT-%d pucBufTemp[0] = %#x\n", (i+1),*(u32*)smcReadBuf);
    }
#ifdef FAT_WRITE_DEBUG
    //DEBUG
    pucBufTemp = &smcReadBuf[0];
    smcSectorsRead(1, nand_fat_start, pucBufTemp);
    DEBUG_SMC("DEBUG: FAT-1 pucBufTemp[0] = %#x\n", *(u32*)pucBufTemp);

    smcSectorsRead(1, nand_fat_start + nand_fat_size, pucBufTemp);
    DEBUG_SMC("DEBUG: FAT-2 pucBufTemp[0] = %#x\n", *(u32*)pucBufTemp);


    DEBUG_SMC("\n\n\n---DEBUG: WRITE FDB.---\n");

    DEBUG_SMC("DEBUG: nand_fat_start = %#x\n", nand_fat_start);
    DEBUG_SMC("DEBUG: nand_fat_end = %#x\n", nand_fat_end);
    DEBUG_SMC("DEBUG: nand_fat_size = %#x\n", nand_fat_size);
    DEBUG_SMC("DEBUG: nand_data_start = %#x\n\n", nand_data_start);
#endif


    /* Clear The FDB region for Host PC writing */
    pucBufTemp = &smcWriteCache[0][0];

    unSecNumTemp = ((NAND_FAT_PARAMETER.RootEntCnt*0x20)/FS_SECTOR_SIZE);
    memset(pucBufTemp, 0x00, FS_SECTOR_SIZE * unSecNumTemp);

    if (smcSectorsWrite(unSecNumTemp, nand_fat_end, SMC_FAT_START_ADDR, pucBufTemp) == 0)
    {
        DEBUG_SMC("Write FAT error in MakeBitMap \n");
        return 0;
    }
#ifdef FAT_WRITE_DEBUG
    //DEBUG
    smcSectorsRead(1, nand_fat_end, pucBufTemp);
    for (i=0; i<4; i++)
        DEBUG_SMC("ROOT ENTRY pucBufTemp[%d] = %#x\n", i, *((u32*)pucBufTemp+i));

    DEBUG_SMC("===DEBUG: WRITE FDB.===\n\n\n");
#endif
    return 2;
    //Read FAT to cache

}


/*

Routine Description:

	Check the block if a bad one. If it is, assign a new good one to use.

Arguments:

	unBlockAddr - the block address to check the block is a valid one or not.

Return Value:

	A valid BLOCK address.

*/
u32 smcCheckBadBlock(u32 unBlockAddr)
{
    u8 i;


    if (Rerseved_Algo_Start)
    {
        for (i=0; i<SMC_RESERVED_BLOCK; i++)      //check whether this block is a bad one
        {
            if (SMC_BBM.Bad_Block_addr[i] == unBlockAddr)
            {
                return  SMC_BBM.New_Block_addr[i];
            }
        }
        return unBlockAddr;
    }
    else
        return unBlockAddr;


}

/*

Routine Description:

	Action to check each page used status.

Arguments:

	punBitMapAddr - The bit map addr which to be checked.
	unStartPage - The start page to check in one block.
	unCount - The page count will be chceked. This count would be limited one block.

Return Value:

	0 - Not need to copy.
	1 - Need to copy.

*/
u8 smcCheckPageUsedAction(u32* punBitMapAddr, u32 unStartPage, u32 unCount)
{
    u8	i, j;
    u8	ucCopyFlag = 0;
    u32 unMaskData;
    u32 unBitMapData = *punBitMapAddr;

    for (i=0; i<unCount; i++)
    {
        unMaskData = 0x0;
        unMaskData |= (1<<(unStartPage+i));

        if (unBitMapData & unMaskData)
        {
            /* mask the page used status and will be updated before quit this sub-function */
            unBitMapData = unBitMapData & ~(1<<(unStartPage+i));

            continue;
        }
        else
            ucCopyFlag = 1; 	/* Need to copy */
    }

    /* update the page used status */
    *punBitMapAddr = unBitMapData;

    return ucCopyFlag;

}


/*

Routine Description:

	Check each page used status.

Arguments:

	punBitMapAddr - The bit map addr which to be checked.
	unPageOffset - The page offset within one block.
	unPageCount - The page count will be chceked. This count would be limited one block.

Return Value:

	0 - Not need to copy.
	1 - Need to copy.

*/
u8 smcCheckPageUsed(u32* punBitMapAddr, u32 unPageOffset, u32 unPageCount)
{
    u32 i, j;
    u8	ucCopyFlag = 0;
    u32	unChkCnt;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\n------DEBUG: smcCheckPageUsed------\n");
    DEBUG_SMC("DEBUG: unPageOffset = %#x\n", unPageOffset);
    DEBUG_SMC("DEBUG: unPageCount = %#x\n", unPageCount);
#endif

    /* The maximum section should be 4, when there is 128 pages in one block. */
    j = unPageOffset / 32;		/* the init start section */

    unPageOffset %= 32;

    if ((unPageCount + unPageOffset) > 32)
        unChkCnt = 32-(unPageOffset%32);

    if (unChkCnt > unPageCount)
        unChkCnt = unPageCount;		/* insufficient pages to be fill of one block */

    for (i=j; i<smcSectionNum; i++)
    {
        ucCopyFlag |= smcCheckPageUsedAction((punBitMapAddr+i), unPageOffset, unChkCnt);

        unPageCount -= unChkCnt;

        if (unPageCount == 0)
            break;

        unPageOffset = 0;		/* Except the first section, the start page should be zero in other pages */

        if (unPageCount > 32)
            unChkCnt = 32;
        else
            unChkCnt = unPageCount;

    }
#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: ucCopyFlag = %d\n", ucCopyFlag);
    DEBUG_SMC("======DEBUG: smcCheckPageUsed======\n\n");
#endif
    return ucCopyFlag;

}

/*

Routine Description:

	Write directly by sector base.

Arguments:

	unWritePhyAddr - The physical addr of NAND flash to write. But this is a sector base addr, it should map to page addr.
	unWriteSecCnt - Count of SECTOR to write. In advanced NAND flash, this argument indicates SECTOR base operation.
	unSecOffsetInBlock - The sector offset in one block.
	pucSrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcWriteDirectlySector(u32 unWritePhyAddr, u32 unSecOffsetInBlock, u32 unWriteSecCnt, u8* pucSrcBuf)
{
    u32 i;
    u32	unSecOffsetInPage;
    u8* pucWriteBuf = (u8*)smcReadBuf;
    u8	unWritePageCnt;
    u32	unSecHead, unSecTail;
    u32	unWritePageAddr;
    u32	unStartSecAddr, unEndSecAddr;
    u32	unTemp;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\n--------smcWriteDirectlySector-------\n");
    DEBUG_SMC("DEBUG: smcWriteDirectlySector!\n");
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
#endif

    /*------------------------Get the sector offset in the first page------------------------*/
    unSecOffsetInPage = unSecOffsetInBlock % smcSecPerPage;
    unSecHead = (smcSecPerPage - unSecOffsetInPage) % smcSecPerPage;
    unSecTail = (unWriteSecCnt - unSecHead) % smcSecPerPage;

    unStartSecAddr = unWritePhyAddr;
    unEndSecAddr = unStartSecAddr + (FS_SECTOR_SIZE * unWriteSecCnt);
    unWritePageAddr = (unWritePhyAddr/smcPageSize) * smcPageSize;
    pucWriteBuf = smcGeneralBuf;

    unWritePageCnt =  (unWriteSecCnt - unSecHead - unSecTail) / smcSecPerPage;


#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\nDEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
    DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
    DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
    DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
    DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
    DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
    DEBUG_SMC("DEBUG: unWritePageAddr = %#x\n", unWritePageAddr);
    DEBUG_SMC("DEBUG: unWritePageCnt = %#x\n", unWritePageCnt);
#endif

    /*  write within one page */
    if ((unStartSecAddr/smcPageSize) == (unEndSecAddr/smcPageSize))
    {
        /* init the write buf to 0xFF */
        memset(pucWriteBuf, 0xff, smcPageSize);
        unTemp = (unSecOffsetInPage * FS_SECTOR_SIZE);
        memcpy((pucWriteBuf + unTemp), pucSrcBuf, (smcPageSize - unTemp));

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("\nDEBUG: write within one page\n");
        DEBUG_SMC("DEBUG: 1 unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
        DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
        DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
        DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
        DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
        DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
        DEBUG_SMC("DEBUG: unWritePageAddr = %#x\n", unWritePageAddr);
        DEBUG_SMC("DEBUG: unWritePageCnt = %#x\n", unWritePageCnt);
#endif

//		unWritePageAddr = (unWritePhyAddr/smcPageSize) * smcPageSize;
        if (smcPageProgram(unWritePageAddr, pucWriteBuf, 0xFFFF) == 0)
        {
            DEBUG_SMC("SMC ERR: [smcWriteDirectlySector] Page Program error at address %#x\n", unWritePageAddr);
            return 0;
        }
    }
    else
    {
        if (unSecHead != 0)
        {
#ifdef FAT_WRITE_DEBUG
            DEBUG_SMC("\nDEBUG: 2 unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
            DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
            DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
            DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
            DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
            DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
            DEBUG_SMC("DEBUG: unWritePageAddr = %#x\n", unWritePageAddr);
            DEBUG_SMC("DEBUG: unWritePageCnt = %#x\n", unWritePageCnt);
#endif

            /*------------------------Write the prior sectors which are not align to a page------------------------*/
            /* init the write buf to 0xFF */
            memset(pucWriteBuf, 0xff, smcPageSize);
            unTemp = (unSecOffsetInPage * FS_SECTOR_SIZE);
            memcpy((pucWriteBuf + unTemp), pucSrcBuf, (smcPageSize - unTemp));

//			unWritePageAddr = (unWritePhyAddr/smcPageSize) * smcPageSize;
            if (smcPageProgram(unWritePageAddr, pucWriteBuf, 0xFFFF))
            {
                pucSrcBuf += (unSecHead * FS_SECTOR_SIZE);
                unWritePageAddr += smcPageSize;
                unWriteSecCnt -= unSecHead;	/* cut the head sectors according to page alignment */
            }
            else
            {
                DEBUG_SMC("SMC ERR: [smcWriteDirectlySector] Page Program error at address %#x\n", unWritePageAddr);
                return 0;
            }
        }

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("\nDEBUG: 3 unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
        DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
        DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
        DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
        DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
        DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
        DEBUG_SMC("DEBUG: unWritePageAddr = %#x\n", unWritePageAddr);
        DEBUG_SMC("DEBUG: unWritePageCnt = %#x\n", unWritePageCnt);
#endif

        /* write the multiple pages */
        for (i=0; i<unWritePageCnt; i++)
        {
            if (smcPageProgram(unWritePageAddr, pucSrcBuf, 0xFFFF) == 0)
            {
                DEBUG_SMC("SMC ERR: [smcWriteDirectlySector] Page Program error at address %#x\n", unWritePageAddr);
                return 0;
            }
            unWritePageAddr += smcPageSize;
            pucSrcBuf += smcPageSize;
        }

        if (unSecTail != 0)
        {

#ifdef FAT_WRITE_DEBUG
            DEBUG_SMC("\nDEBUG: 4 unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
            DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
            DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
            DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
            DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
            DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
            DEBUG_SMC("DEBUG: unWritePageAddr = %#x\n", unWritePageAddr);
            DEBUG_SMC("DEBUG: unWritePageCnt = %#x\n", unWritePageCnt);
#endif

            /*------------------------Write the last pages which are multiples of one page------------------------*/
            pucWriteBuf = (u8*)smcReadBuf;
            memset(pucWriteBuf, 0xff, smcPageSize);
            memcpy(pucWriteBuf, pucSrcBuf, (unSecTail * FS_SECTOR_SIZE));
            if (smcPageProgram(unWritePageAddr, pucWriteBuf, 0xFFFF) == 0)
            {
                DEBUG_SMC("SMC ERR: [smcWriteDirectlySector] Page Program error at address %#x\n", unWritePageAddr);
                return 0;
            }
        }
    }
#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("======smcWriteDirectlySector======\n");
#endif
    return 1;


}

/*

Routine Description:

	Write directly by page base.

Arguments:

	unWritePhyAddr - The physical addr of NAND flash to write.
	unWriteCnt - Count of PAGEs to write.
	pucSrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcWriteDirectly(u32 unWritePhyAddr,u32 unWriteCnt,u8* pucSrcBuf)
{
    u32 i;

    for (i=0; i<unWriteCnt; i++)
    {
        if (smcPageProgram(unWritePhyAddr, pucSrcBuf, 0xFFFF))
        {
            unWritePhyAddr += FS_SECTOR_SIZE;  // calculate each setor logical address
            pucSrcBuf += smcPageSize;
        }
        else
            return 0;
    }

    return 1;
}
#if 0

/*

Routine Description:

	Copy section head and tail by sector base action. There are former data exist the areas of the front and back of the address which we will write.
	It ought to copy them out and merge new data, then re-write into the block again. There are at least two sections in one block. This routine will
	process one section.

Arguments:

	unSectionAddr - The physical SECTION addr of NAND flash to process.
	unBitMapDataed - Bit-Map data which has been updated.
	unSecOffset - The sector offset in the block.
	unWriteSecCnt - Counts of SECTOR to process.
	ucSectionNum - The section num in block to process.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcCopySectionTailHeadSectorAction(u32 unSectionAddr, u32 unBitMapDataed, u32 unSecOffset, u32 unWriteSecCnt, u8 ucSectionNum)
{
    u8	i, j;
    u8	ucChgBuf = 0;
    u8	*pucReadBuf = &smcWriteCache[0][0];
    u32	unAddrTmp;
    u32	unSecPerBlock = (smcBlockSize / FS_SECTOR_SIZE);
//	u32 unBitMapDataed;
//	u32	unBlockAddr;


    unBlockAddr = (unWritePhyAddr / smcBlockSize) * smcBlockSize;
    unAddrTmp = unSectionAddr;

//	unBitMapDataed = *punBitMapDataed;







    if (unBitMapDataed==0 && unWriteSecCnt==unSecPerBlock && unSecOffset==0)   // Clear all block without copy-back
    {
        ucChgBuf = 1;
    }
    else
    {
        pucReadBuf += (ucSectionNum * 32 * smcPageSize);

        for (i=0; i<32; i++)		/* 32 indicates 32 pages in one section of block.*/
        {
            if ((unBitMapDataed & (1<<i))!=1)
            {
                /* read out a used page for backup purpose */
                if (smcPage1ARead(unAddrTmp, pucReadBuf)==0)
                {
                    DEBUG_SMC("read error at address %#x", unAddrTmp);
                    return 0;
                }
            }
            unAddrTmp += smcPageSize;
            pucReadBuf += smcPageSize;
        }

        unAddrTmp = unSectionAddr;
        pucReadBuf -= (ucSectionNum * 32 * smcPageSize);






        for (i = 0; i < smcPagePerBlock; i++)
        {
            if ( i>=unPageOffset && i<(unPageOffset + unPageCnt))
            {		 //Update the write data to write cache
                memcpy(&smcWriteCache[i][0], pucSrcBuf, smcPageSize);
                pucSrcBuf += smcPageSize;
            }
            else
            {
                if ((unBitMapDataed & (1<<i))!=1)
                {
                    if (smcPage1ARead(temp_addr, read_buf)==0)
                    {
                        DEBUG_SMC("read error on block address %#x", unBlockAddr);
                        return 0;
                    }
                }
            }
            temp_addr += smcPageSize;
            read_buf += smcPageSize;
        }
    }

    if (smcBlockErase(unBlockAddr) == 0)
    {
        DEBUG_SMC("Erase error on block address %#x", unBlockAddr);
        return 0;
    }

    if (ucChgBuf)
        read_buf = pucSrcBuf;
    else
        read_buf= &smcWriteCache[0][0];
    // Re-write write cache to block
    temp_addr = unBlockAddr;

    for (i = 0; i < smcPagePerBlock; i++)
    {
        if ((unBitMapDataed & (1<<i))!=1)
        {	//page can't use
            if (smcPageProgram(temp_addr, read_buf, 0xFFFF)==0)
            {
                DEBUG_SMC("Write error on block byte address %#x\n",temp_addr);
                return 0;
            }
        }
        temp_addr += smcPageSize;
        read_buf += smcPageSize;
    }

    return 1;
}
#endif

/*

Routine Description:

	Copy block head and tail by sector base. There are former data exist the areas of the front and back of the address which we will write.
	It ought to copy them out and merge new data, then re-write into the block again.

Arguments:

	unBlockAddr - The physical addr of NAND flash to write.
	punBitMapDataed - Pointer to Bit-Map data which has been updated.
	unWriteSecCnt - Counts of SECTOR to write.
	pucSrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcCopyBlockTailHeadSector(u32 unWritePhyAddr, u32* punBitMapDataed, u32 unWriteSecCnt, u8* pucSrcBuf)
{
    u8	i, j;
    u8	ucChgBuf = 0;
    u8	*pucReadBuf = smcGeneralBuf;
    u32 unBitMapDataed;
    u32	unSecOffsetInBlock;
    u32	unWriteAddr;


    unSecOffsetInBlock = ((unWritePhyAddr - SMC_FAT_START_ADDR) / FS_SECTOR_SIZE);
    unSecOffsetInBlock %= smcSecPerBlock;
    unWriteAddr = (unWritePhyAddr / smcBlockSize) * smcBlockSize;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: -----smcCopyBlockTailHeadSector-----\n");
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
    DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
    DEBUG_SMC("DEBUG: unWriteAddr = %#x\n", unWriteAddr);
    DEBUG_SMC("DEBUG: unWriteSecCnt = %#x\n", unWriteSecCnt);
    DEBUG_SMC("DEBUG: pucWriteBuf[0] 1= %#x\n\n", *(u32*)pucSrcBuf);
#endif

    if (unBitMapDataed==0 && unWriteSecCnt==smcSecPerBlock && unSecOffsetInBlock==0)   // Clear all block without copy-back
    {
        ucChgBuf = 1;
    }
    else
    {
        for (j=0; j<smcSectionNum; j++)
        {
            unBitMapDataed = *(punBitMapDataed+j);
#ifdef FAT_WRITE_DEBUG
            DEBUG_SMC("DEBUG: unBitMapDataed [%d]= %#x\n", j, unBitMapDataed);
            DEBUG_SMC("DEBUG: smcSectionNum = %#x\n", j);
            DEBUG_SMC("DEBUG: pucWriteBuf[0] 3 = %#x\n\n", *(u32*)pucSrcBuf);
#endif
            for (i=0; i<32; i++)
            {
                if ((unBitMapDataed & (1<<i))!=1)
                {
                    if (smcPage1ARead(unWriteAddr, pucReadBuf) == 0)
                    {
                        DEBUG_SMC("Error! Read error at address %#x", unWriteAddr);
                        return 0;
                    }
                }
                unWriteAddr += smcPageSize;
                pucReadBuf += smcPageSize;
            }
        }

        /*------------------update the data to write------------------*/
        unWriteAddr -= smcBlockSize;	/* init the address for next program steps */
        pucReadBuf = smcGeneralBuf;		/* init the address for next program steps */

        pucReadBuf += (unSecOffsetInBlock * FS_SECTOR_SIZE);
        memcpy(pucReadBuf, pucSrcBuf, unWriteSecCnt*FS_SECTOR_SIZE);

    }

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: Block Erase Addr= %#x\n", unWriteAddr);
#endif
    if (smcBlockErase(unWriteAddr) == 0)
    {
        DEBUG_SMC("Erase error on block address %#x", unWriteAddr);
        return 0;
    }

    if (ucChgBuf)
        pucReadBuf = pucSrcBuf;
    else
        pucReadBuf = smcGeneralBuf;

    /*---------------------Re-Write cache to flash---------------------*/
    for (j=0; j<smcSectionNum; j++)
    {
        unBitMapDataed = *(punBitMapDataed+j);
        for (i=0; i<32; i++)
        {
            if ((unBitMapDataed & (1<<i))!=1)
            {
                if (smcPageProgram(unWriteAddr, pucReadBuf, 0xFFFF) == 0)
                {
                    DEBUG_SMC("Write error on address %#x", unWriteAddr);
                    return 0;
                }
            }
            unWriteAddr += smcPageSize;
            pucReadBuf += smcPageSize;
        }
    }

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: =====smcCopyBlockTailHeadSector=====\n");
#endif

    return 1;
}

/*

Routine Description:

	Copy block head and tail. There are former data exist the areas of the front and back of the address which we will write.
	It ought to copy them out and merge with new data, then re-write into the block again. This routine is called by page base.

Arguments:

	BlockAddr - The physical addr of NAND flash to write.
	Bit_map_dataed - Has been updated Bit-Map data.
	Offset - The page offset in the block.
	count - Counts of PAGE to write.
	SrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcCopyBlockTailHead(u32 BlockAddr,u32 Bit_map_dataed,u32 Offset,u32 count,u8* SrcBuf)
{
    u8 i,change_buffer=0;
    u8* read_buf = smcGeneralBuf;
    u32 temp_addr=BlockAddr;

    if (Bit_map_dataed==0 && count==smcPagePerBlock && Offset==0)	// Clear all block without copy-back
    {
        change_buffer=1;
    }
    else
    {
        for (i = 0; i < smcPagePerBlock; i++)
        {
            if ( i>=Offset && i<(Offset+count))
            {		 //Update the write data to write cache
//				memcpy(&smcWriteCache[i][0],SrcBuf,smcPageSize);
                memcpy(smcGeneralBuf+(i*smcPageSize), SrcBuf, smcPageSize);
                SrcBuf+=smcPageSize;
            }
            else
            {
                if ((Bit_map_dataed & (1<<i))!=1)
                {
                    if (smcPage1ARead(temp_addr,read_buf)==0)
                    {
                        DEBUG_SMC("read error on block address %#x",BlockAddr);
                        return 0;
                    }
                }
            }
            temp_addr += smcPageSize;
            read_buf += smcPageSize;
        }
    }

    if (smcBlockErase(BlockAddr)==0)
    {
        DEBUG_SMC("Erase error on block address %#x",BlockAddr);
        return 0;
    }

    if (change_buffer)
        read_buf=SrcBuf;
    else
        read_buf= smcGeneralBuf;
    // Re-write write cache to block
    temp_addr=BlockAddr;

    for (i = 0; i < smcPagePerBlock; i++)
    {
        if ((Bit_map_dataed & (1<<i))!=1)
        {	//page can't use
            if (smcPageProgram(temp_addr, read_buf, 0xFFFF)==0)
            {
                DEBUG_SMC("Write error on block byte address %#x\n",temp_addr);
                return 0;
            }
        }
        temp_addr += smcPageSize;
        read_buf += smcPageSize;
    }

    return 1;
}


/*

Routine Description:

	Copy the exist block data and re-write to the original address after new data is merged. This function is performed by sector base.

Arguments:

	unWritePhyAddr - The physical address to write.
	unSecOffsetInBlock - The offset SECTOR in one block.
	unSecOffsetInPage - The offset SECTOR in one page.
	unWriteSecCnt - Count of sectors to write.
	pucSrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

Remark:

	The sector base is for FAT R/W used. The first sector addr is SMC_FAT_START_ADDR.

*/
u8 smcCopyBlockActionSector(u32 unWritePhyAddr, u32 unSecOffsetInBlock, u32 unSecOffsetInPage, u32 unWriteSecCnt, u8* pucSrcBuf)
{
    u32	unPageCnt;
    u32	unBlockAddr;
    u32 unPageOffset;
    u32	unSecHead, unSecTail;
    u32	unTempAddr;
    u32	unBlockAddrTemp;
    u32*	punBitMapBlock = (u32* )smcBitMap; /* 0xFFFFFFFF means this block is available to write */


#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\nDEBUG: ------smcCopyBlockActionSector------\n");
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
#endif
    /* ------------------check the block if is a valid one ------------------*/
    unBlockAddr = (unWritePhyAddr / smcBlockSize) * smcBlockSize;
    unTempAddr = unWritePhyAddr - unBlockAddr;
    unBlockAddrTemp = smcCheckBadBlock(unBlockAddr);
    if (unBlockAddrTemp !=  unBlockAddr)
    {
        unWritePhyAddr = unBlockAddrTemp + unTempAddr;
        unBlockAddr = unBlockAddrTemp;
    }

    unPageOffset = unTempAddr / smcPageSize;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
#endif

    /* Get the whole bit map of the block which will be processed */
    /* Each bit indicates one page used status */
    /* Offset the bit-map address by the address will be processed */
    if (SMC_MAX_PAGE_PER_BLOCK == 32)
        punBitMapBlock += (unWritePhyAddr / smcBlockSize);
    else if (SMC_MAX_PAGE_PER_BLOCK == 64)
        punBitMapBlock += ((unWritePhyAddr / smcBlockSize)*2);
    else if (SMC_MAX_PAGE_PER_BLOCK == 128)
        punBitMapBlock += ((unWritePhyAddr / smcBlockSize)*4);
    else
        DEBUG_SMC("SMC ERR: Unknown type NAND Flash! No Supported!\n");

    /* ------------------Get the page counts to write------------------ */
//	unSecOffsetInBlock = unSectorNum % smcSecPerBlock;
//	unSecOffsetInPage = (unSecOffsetInBlock % smcSecPerPage);

    /* To reduce process time */
    if (unSecOffsetInPage == 0)
    {
        unSecHead = 0;
        unSecTail = unWriteSecCnt % smcSecPerPage;
        unPageCnt = ((unWriteSecCnt - unSecTail) / smcSecPerPage);

        if (unSecTail != 0)
            unPageCnt ++;
    }
    else
    {
        unSecHead = (smcSecPerPage - unSecOffsetInPage) % smcSecPerPage;
        unSecTail= (unWriteSecCnt - unSecHead) % smcSecPerPage;
        unPageCnt = ((unWriteSecCnt - unSecHead- unSecTail)/ smcSecPerPage);
        if (unSecHead != 0)
            unPageCnt ++;
        if (unSecTail != 0)
            unPageCnt ++;
    }


#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
    DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
    DEBUG_SMC("DEBUG: unWriteSecCnt = %#x\n", unWriteSecCnt);
    DEBUG_SMC("DEBUG: unPageOffset = %#x\n", unPageOffset);
    DEBUG_SMC("DEBUG: pucWriteBuf[0] = %#x\n", *(u32*)pucSrcBuf);
    DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
    DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
    DEBUG_SMC("DEBUG: unPageCnt = %#x\n", unPageCnt);
#endif

    if (smcCheckPageUsed(punBitMapBlock, unPageOffset, unPageCnt) == 0)
    {	/* The area which will be written is new. So it doesn't need to replace and erase. */
#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: smcCheckPageUsed and WriteDirectly!\n");
#endif
        if (unWritePhyAddr < SMC_RESERVED_SIZE)		/* The area in front of SMC_RESERVED_SIZE is under protection. */
            return 0;

        /* Operated by page base */
        if (smcWriteDirectlySector(unWritePhyAddr, unSecOffsetInBlock, unWriteSecCnt, pucSrcBuf) == 0)
        {
            DEBUG_SMC("SMC ERR: Write directly error on Physical address %x", unWritePhyAddr);
            return 0;
        }
    }
    else
    {	/* Because the page which will be written is a used one, it should copy the head and tail ones before write into new data. */

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("\nDEBUG: Head and Tail!\n");
#endif
        if (unBlockAddr < SMC_RESERVED_SIZE)
            return 0;

        if (smcCopyBlockTailHeadSector(unWritePhyAddr, punBitMapBlock, unWriteSecCnt, pucSrcBuf) == 0)
        {
            DEBUG_SMC("SMC ERR: Copy block back error on Logical address %x", unBlockAddr);
            return 0;
        }
    }
#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\nDEBUG: ======smcCopyBlockActionSector======\n");
#endif
    return 1;

}

/*

Routine Description:

	Copy the exist block data and re-write to the original address after new data is merged. This function is performed by page base.
	It could be called by smcPagesWrite();

Arguments:

	unWritePhyAddr - The physical address to write.
	unSecOffset - The offset SECTOR in one block.
	unWriteSecCnt - Count of sectors to write.
	pucSrcBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

Remark:

	The sector base is for FAT R/W used. The first sector addr is SMC_FAT_START_ADDR.

*/

u8 smcCopyBlockAction(u32 logical_addr,u32 Offset,u32 Count,u8* SrcBuf)
{
    u32 BlockByteAddr;
    u32 BlockAddr;
    u32 Bit_map_word_data;

    u32* bitmap_block = (u32* )smcBitMap; //0xFFFFFFFF means this block had erased
    u32 temp;
    u8 copy_flag;

    temp = logical_addr - Offset*smcPageSize;
    BlockByteAddr = smcCheckBadBlock(temp);

    if (temp!=BlockByteAddr) // Bad block occur
        logical_addr=BlockByteAddr +Offset*smcPageSize;

    BlockAddr = BlockByteAddr / smcBlockSize;  // offset of block
    bitmap_block += BlockAddr;              // Get whole BitMap data of writeCnt
    Bit_map_word_data = *bitmap_block;		// Show the bit map status of block

    copy_flag=smcCheckPageUsed(&Bit_map_word_data,Offset,Count);

    if (copy_flag==0) // We don't need to replace and erase
    {
        if (logical_addr<SMC_RESERVED_SIZE)
            return 0;

        if (smcWriteDirectly(logical_addr,Count,SrcBuf)==0)
        {
            DEBUG_SMC("Write directly error on address %#x\n",logical_addr);
            return 0;
        }
    }
    else
    {
        if (BlockByteAddr<SMC_RESERVED_SIZE)
            return 0;

        if (smcCopyBlockTailHead(BlockByteAddr,Bit_map_word_data,Offset,Count,SrcBuf)==0)
        {
            DEBUG_SMC("Copy block back error on Logical address %#x\n",BlockByteAddr);
            return 0;
        }
    }
    *bitmap_block=Bit_map_word_data;

    return 1;
}

/*

Routine Description:

	Read sectors.

Arguments:

	unReadSecCnt - Counts of sector to read.
	unSecNum - Sector number to read.
	pucReadBuf - Buffer to read to.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSectorsRead(u32 unReadSecCnt, u32 unSecNum, u8* pucReadBuf)
{
    u32	i;
    u32	unTempAddr;
    u32	unSecOffsetInBlock;
    u32	unSecOffsetInPage;
    u32	unPageOffsetInBlock;
    u32	unPageAddr;
    u32	unOffsetTemp;
    u32	unReadPageCnt;
    u32	unSecHead, unSecTail;
    u32	unStartSecAddr, unEndSecAddr;
    u32	unPageCntInFirstBlock;
    u32	unBlockAddr;
    u8*	pucBufTemp = (u8*) smcReadBuf;
    bool	ucFirstBlock = TRUE;
    bool	ucHeadTailInSameBlock;	/* index of the head sector and the tail sector are in the same block or not */
    u32	unTempDebug;
	u32	unReadSectorPosition = unSecNum;	/* indicator of the sector position where is read out */

    smcProcProt(SMC_SET_REQ, 0);	/* set to disable access */


    /* check the head and the tail are in the same block or not */
    unStartSecAddr = SMC_FAT_START_ADDR + FS_SECTOR_SIZE * unSecNum;
    /* ------------------Get the page counts to read--------------- */
    unBlockAddr = (unStartSecAddr / smcBlockSize) * smcBlockSize;
    unTempAddr = unStartSecAddr - unBlockAddr;
	
    unStartSecAddr = smcCheckBadBlock(unBlockAddr) + unTempAddr;
    unEndSecAddr = unStartSecAddr + (unReadSecCnt * FS_SECTOR_SIZE) - 1;

#ifdef FAT_READ_DEBUG
    DEBUG_SMC("\n------DEBUG: smcSectorsRead------\n");
    DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
    DEBUG_SMC("DEBUG: unEndSecAddr = %#x\n", unEndSecAddr);
    DEBUG_SMC("DEBUG: unReadSecCnt = %#x\n", unReadSecCnt);
    DEBUG_SMC("DEBUG: unSecNum = %#x\n", unSecNum);
#endif




    if ( (unStartSecAddr / smcBlockSize) == (unEndSecAddr / smcBlockSize))
        ucHeadTailInSameBlock = TRUE;
    else
        ucHeadTailInSameBlock = FALSE;


    /* the head and the tail are in the same block */
    if (ucHeadTailInSameBlock == TRUE)
    {
        unPageAddr = (unStartSecAddr/smcPageSize)*smcPageSize;	/* align the start of one page */

        /* ------------------Get the page counts to read--------------- */
        unSecOffsetInBlock = unTempAddr / FS_SECTOR_SIZE;
        unSecOffsetInPage = (unSecOffsetInBlock % smcSecPerPage);
        unSecHead = (smcSecPerPage - unSecOffsetInPage) % smcSecPerPage;
		unSecHead = GET_MIN(unSecHead, unReadSecCnt);
        unSecTail= (unReadSecCnt - unSecHead) % smcSecPerPage;
        unReadPageCnt = ((unReadSecCnt - unSecHead - unSecTail) / smcSecPerPage);

        /* the start sector and the end sector are in the same page */
        if ( (unStartSecAddr/smcPageSize) == (unEndSecAddr/smcPageSize) )
        {
#ifdef FAT_READ_DEBUG
            DEBUG_SMC("DEBUG: the start sector and the end sector are in the same page\n");
            DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
            DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
            DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
            DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
            DEBUG_SMC("DEBUG: unReadPageCnt = %#x\n", unReadPageCnt);
#endif
            if (smcPage1ARead(unPageAddr, pucBufTemp) == 0)
            {
                DEBUG_SMC("SMC ERR: 1 Read error at addr %#x\n", unPageAddr);
                DEBUG_SMC("SMC ERR: 1 unPageAddr = %#x\n", unPageAddr);
                DEBUG_SMC("SMC ERR: 1 pucBufTemp = %#x\n", pucBufTemp);
                return 0;
            }
            memcpy(pucReadBuf, pucBufTemp + (unSecOffsetInPage*FS_SECTOR_SIZE), unReadSecCnt*FS_SECTOR_SIZE);
        }
        else	/* the start sector and the end sector are NOT in the same page */
        {
#ifdef FAT_READ_DEBUG
            DEBUG_SMC("DEBUG: the start sector and the end sector are NOT in the same page\n");
            DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
            DEBUG_SMC("DEBUG: smcBlockSize = %#x\n", smcBlockSize);

            DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
            DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
            DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
            DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
            DEBUG_SMC("DEBUG: unReadPageCnt = %#x\n", unReadPageCnt);
#endif
            /* copy the head sectors if exist */
            if (unSecHead != 0)
            {
#ifdef FAT_READ_DEBUG
                DEBUG_SMC("DEBUG: copy the head sectors if exist\n");
#endif
                if (smcPage1ARead(unPageAddr, pucBufTemp) == 0)
                {
                    DEBUG_SMC("SMC ERR: 2 Read error at addr %#x\n", unPageAddr);
                    DEBUG_SMC("SMC ERR: 2 unPageAddr = %#x\n", unPageAddr);
                    DEBUG_SMC("SMC ERR: 2 pucBufTemp = %#x\n", pucBufTemp);
                    return 0;
                }
                unOffsetTemp = (FS_SECTOR_SIZE * unSecHead);

                pucBufTemp += (smcPageSize - unOffsetTemp);	/* offset to the start address */
                memcpy(pucReadBuf, pucBufTemp, unOffsetTemp);
                unPageAddr += smcPageSize;
                pucReadBuf += unOffsetTemp;
            }


            /* copy the whole pages, not include tail sectors */
            for (i=0; i<unReadPageCnt; i++)
            {
                if (smcPage1ARead(unPageAddr, pucReadBuf) == 0)
                {
	                DEBUG_SMC("DEBUG: i = %d\n", i);
                    DEBUG_SMC("SMC ERR: 3 Read error at addr %#x\n", unPageAddr);
                    DEBUG_SMC("SMC ERR: 3 unPageAddr = %#x\n", unPageAddr);
                    DEBUG_SMC("SMC ERR: 3 pucBufTemp = %#x\n", pucReadBuf);
                    return 0;
                }
                pucReadBuf += smcPageSize;
                unPageAddr += smcPageSize;

            }

            /* copy the tail sectors if exist */
            if (unSecTail != 0)
            {
                pucBufTemp = &smcReadBuf[0];

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("DEBUG: copy the tail sectors if exist\n");
#endif
                if (smcPage1ARead(unPageAddr, pucBufTemp) == 0)
                {
                    DEBUG_SMC("SMC ERR: 4 Read error at addr %#x\n", unPageAddr);
                    return 0;
                }
                memcpy(pucReadBuf, pucBufTemp, (FS_SECTOR_SIZE * unSecTail));
            }
        }
    }
    else	/* the head and the tail sectors are not in the same block */
    {

        unSecOffsetInBlock = unTempAddr / FS_SECTOR_SIZE;
        unSecOffsetInPage = (unSecOffsetInBlock % smcSecPerPage);
        unPageOffsetInBlock = unTempAddr / smcPageSize;
		unSecHead = (smcSecPerPage - unSecOffsetInPage) % smcSecPerPage;
		unSecHead = GET_MIN(unSecHead, unReadSecCnt);
		unSecTail= (unReadSecCnt - unSecHead) % smcSecPerPage;
		unReadPageCnt = ((unReadSecCnt - unSecHead - unSecTail) / smcSecPerPage);			


#ifdef FAT_READ_DEBUG
        DEBUG_SMC("DEBUG: ucHeadTailInSameBlock = FALSE\n");
        DEBUG_SMC("DEBUG: unStartSecAddr = %#x\n", unStartSecAddr);
        DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
        DEBUG_SMC("DEBUG: unSecOffsetInPage = %#x\n", unSecOffsetInPage);
        DEBUG_SMC("DEBUG: unPageOffsetInBlock = %#x\n", unPageOffsetInBlock);
        DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
        DEBUG_SMC("DEBUG: unSecTail = %#x\n", unSecTail);
        DEBUG_SMC("DEBUG: unReadSecCnt = %#x\n", unReadSecCnt);
        DEBUG_SMC("DEBUG: unReadPageCnt = %#x\n\n", unReadPageCnt);
#endif

        while (unReadSecCnt != 0)
        {
#ifdef FAT_READ_DEBUG
            DEBUG_SMC("\n\nDEBUG: while\n");
            DEBUG_SMC("DEBUG: unReadSecCnt = %#x\n", unReadSecCnt);
#endif
			unStartSecAddr = SMC_FAT_START_ADDR + FS_SECTOR_SIZE * unReadSectorPosition;
            unBlockAddr = (unStartSecAddr / smcBlockSize) * smcBlockSize;
            unTempAddr = unStartSecAddr - unBlockAddr;
            unStartSecAddr = smcCheckBadBlock(unBlockAddr) + unTempAddr;
            unPageAddr = (unStartSecAddr/smcPageSize)*smcPageSize;

            /*------------------Process the first block---------------*/
            if (ucFirstBlock == TRUE)
            {
#ifdef FAT_READ_DEBUG
                DEBUG_SMC("DEBUG: ucFirstBlock = TRUE\n");
#endif
                ucFirstBlock = FALSE;

                /* get the multiple page counts in the first block */
                unPageCntInFirstBlock = (smcSecPerBlock - unSecOffsetInBlock) / smcSecPerPage;

                if (unSecHead != 0)
                {	/* copy the head sectors if it exist */

#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: copy the head sectors.\n");
                    DEBUG_SMC("DEBUG: unSecHead = %#x\n", unSecHead);
                    DEBUG_SMC("DEBUG: unPageCntInFirstBlock = %#x\n", unPageCntInFirstBlock);
#endif

                    pucBufTemp = &smcReadBuf[0];
                    if (smcPage1ARead(unPageAddr, pucBufTemp) == 0)
                    {
                        DEBUG_SMC("SMC ERR: 5 Read error at addr %#x\n", unPageAddr);
                        return 0;
                    }
                    unOffsetTemp = (FS_SECTOR_SIZE * unSecHead);

                    pucBufTemp += smcPageSize - unOffsetTemp;
                    memcpy(pucReadBuf, pucBufTemp, unOffsetTemp);
                    unPageAddr += smcPageSize;
                    pucReadBuf += unOffsetTemp;
                    unReadSecCnt -= unSecHead;
                    unStartSecAddr += unOffsetTemp;
					unReadSectorPosition += unSecHead;	/* move the sector position */
#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: 1 unStartSecAddr = %#x\n", unStartSecAddr);
#endif
                }

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("DEBUG: 2 unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
                DEBUG_SMC("DEBUG: 2 unSecOffsetInPage = %#x\n", unSecOffsetInPage);
                DEBUG_SMC("DEBUG: 2 unPageOffsetInBlock = %#x\n", unPageOffsetInBlock);
                DEBUG_SMC("DEBUG: 2 unSecTail = %#x\n", unSecTail);
                DEBUG_SMC("DEBUG: 2 unReadSecCnt = %#x\n", unReadSecCnt);
                DEBUG_SMC("DEBUG: 2 unReadPageCnt = %#x\n", unReadPageCnt);
                DEBUG_SMC("DEBUG: 2 unPageCntInFirstBlock = %#x\n", unPageCntInFirstBlock);
#endif
                /* Read out the multiple pages after the head sectors */
                for (i=0; i<unPageCntInFirstBlock; i++)
                {
                    if (smcPage1ARead(unPageAddr, pucReadBuf) == 0)
                    {
                        DEBUG_SMC("SMC ERR: 6 Read error at addr %#x\n", unPageAddr);
		                DEBUG_SMC("DEBUG: i = %d\n", i);
                        return 0;
                    }
                    pucReadBuf += smcPageSize;
                    unPageAddr += smcPageSize;
                }
				unReadSectorPosition += (unPageCntInFirstBlock * smcSecPerPage);		/* move the sector position */

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("DEBUG: i = %d\n", i-1);
                DEBUG_SMC("unReadSecCnt = %#x\n", unReadSecCnt);
#endif

                unReadPageCnt -= unPageCntInFirstBlock;
                unReadSecCnt -= (unPageCntInFirstBlock*smcSecPerPage);

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("unReadSecCnt = %#x\n", unReadSecCnt);
                DEBUG_SMC("DEBUG: 2 unStartSecAddr = %#x\n", unStartSecAddr);
#endif

                /* init for next block processing */
                unStartSecAddr += (unPageCntInFirstBlock * smcPageSize);
                unSecOffsetInBlock = 0;
                unSecOffsetInPage = 0;
                unSecHead = 0;

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("unReadSecCnt = %#x\n", unReadSecCnt);
                DEBUG_SMC("DEBUG: 3 unStartSecAddr = %#x\n", unStartSecAddr);
#endif


#ifdef FAT_READ_DEBUG
                //For Debug
                if ((unStartSecAddr % smcBlockSize) != 0)
                {
                    DEBUG_SMC("unStartSecAddr = %#x\n", unStartSecAddr);
                    DEBUG_SMC("unReadPageCnt = %#x\n", unReadPageCnt);
                    DEBUG_SMC("unSecTail = %#x\n", unSecTail);
                }
#endif

            }
            else		/* process the remainder blocks */
            {

#ifdef FAT_READ_DEBUG
                DEBUG_SMC("\nDEBUG: remainder blocks\n");
                DEBUG_SMC("DEBUG: 4 unStartSecAddr = %#x\n", unStartSecAddr);
                DEBUG_SMC("DEBUG: 4 unReadSecCnt = %#x\n", unReadSecCnt);
#endif
                /* read a whole block */
                if (unReadSecCnt > smcSecPerBlock)
                {
#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: read a whole block\n");
#endif
                    for (i=0; i<smcPagePerBlock; i++)
                    {
                        if (smcPage1ARead(unStartSecAddr, pucReadBuf) == 0)
                        {
                            DEBUG_SMC("SMC ERR: 7 Read error at addr %#x\n", unStartSecAddr);
                            return 0;
                        }
                        pucReadBuf += smcPageSize;
                        unStartSecAddr += smcPageSize;
                    }
					unReadSectorPosition += smcSecPerBlock;		/* update the read out sector address */
                    unReadSecCnt -= smcSecPerBlock;
                    unReadPageCnt -= smcPagePerBlock;
#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: 2 unReadSecCnt = %#x\n", unReadSecCnt);
                    DEBUG_SMC("DEBUG: 2 unReadPageCnt = %#x\n", unReadPageCnt);
#endif

                }
                else		/* read the last sectors */
                {
#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: read the remainder sectors\n");
#endif
                    for (i=0; i<unReadPageCnt; i++)
                    {
                        if (smcPage1ARead(unStartSecAddr, pucReadBuf) == 0)
                        {
                            DEBUG_SMC("SMC ERR: 8 Read error at addr %#x\n", unStartSecAddr);
                            return 0;
                        }
                        pucReadBuf += smcPageSize;
                        unStartSecAddr += smcPageSize;
                        unReadSecCnt -= smcSecPerPage;
                    }
					unReadSectorPosition += (unReadPageCnt * smcSecPerPage);

#ifdef FAT_READ_DEBUG
                    DEBUG_SMC("DEBUG: 4 unReadSecCnt = %#x\n", unReadSecCnt);
#endif

                    /* copy the tail sectors */
                    if (unSecTail != 0)
                    {
                        pucBufTemp = &smcReadBuf[0];
#ifdef FAT_READ_DEBUG
                        DEBUG_SMC("DEBUG: copy the tail sectors\n");
                        DEBUG_SMC("DEBUG: 3 unStartSecAddr = %#x\n", unStartSecAddr);
                        DEBUG_SMC("DEBUG: 3 unReadSecCnt = %#x\n", unReadSecCnt);
#endif
                        /* copy the tail sectors in the temp buf */
                        if (smcPage1ARead(unStartSecAddr, pucBufTemp) == 0)
                        {
                            DEBUG_SMC("SMC ERR: 9 Read error at addr %#x\n", unStartSecAddr);
                            return 0;
                        }
                        memcpy(pucReadBuf, pucBufTemp, (FS_SECTOR_SIZE * unSecTail));
                        unReadSecCnt -= unSecTail;

#ifdef FAT_READ_DEBUG
                        //For Debug
                        if (unReadSecCnt != 0)
                            DEBUG_SMC("DEBUG: Error! 4 unReadSecCnt = %d\n", unReadSecCnt);
#endif
                    }
                }
            }
        }
    }
#ifdef FAT_READ_DEBUG
    DEBUG_SMC("pucReadBuf [0] = %#x\n", *(u32*)pucReadBuf);
    DEBUG_SMC("======DEBUG: smcSectorsRead======\n");
#endif

    smcProcProt(SMC_SET_REQ, 1);	/* set to Enable access */

    return 1;
}

/*

Routine Description:

	Read PAGES. The pages to read must be within the same block.

Arguments:

	readCnt - Count of pages to read.
	logAddr - Logical page address.
	readBuf - Buffer to read to.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPagesRead(u32 readCnt, u32 logAddr, u8* readBuf)
{
    u32 i;
    u32 pageOffset,BlockAddr,BlockByteAddr;
    u32 bad_flag=0;

    smcProcProt(SMC_SET_REQ, 0);	/* set to disable access */

    if (Rerseved_Algo_Start)
    {
        pageOffset = logAddr % smcBlockSize;		// offset of page in a block
        pageOffset = pageOffset /smcPageSize;

        BlockAddr = logAddr / smcBlockSize;  // offset of block
        BlockByteAddr = BlockAddr*smcBlockSize;

        for (i=0;i<SMC_RESERVED_BLOCK ;i++)      //check whether this block is a bad one
        {
            if (SMC_BBM.Bad_Block_addr[i]==BlockByteAddr)
            {
                bad_flag=1;
                break;
            }
        }

        if (bad_flag)
        {	/* assign a new addr for the bad one */
            logAddr = SMC_BBM.New_Block_addr[i] + pageOffset*smcPageSize;
        }

    }

    // Read data
    /*
    		if the read addr is over one block, it should occur error 
    the read count should be limited 
    */
    for (i = 0; i < readCnt; i++)
    {
        if (smcPage1ARead(logAddr, readBuf)==0)
        {
            DEBUG_SMC("Read error on logical address %#x\n",logAddr);
            return 0;       // civictest
        }
        logAddr += smcPageSize;
        readBuf += smcPageSize;
    }

    smcProcProt(SMC_SET_REQ, 1);	/* set to Enable access */

    return 1;
}

/*

Routine Description:

	Write sectors. In advanced NAND flash, this function will operate by sector base.

Arguments:

	unWriteSecCnt - Count of SECTOR to write. In advanced NAND flash, this argument indicates SECTOR base operation.
	unWritePhyAddr - The physical addr of NAND flash to write.
	pucWriteBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
/*

Routine Description:

	Write sectors. In advanced NAND flash, this function will operate by sector base.

Arguments:

	unWriteSecCnt - Count of SECTOR to write. In advanced NAND flash, this argument indicates SECTOR base operation.
	unSectorNum - The start sector number to write.
	unWriteAddr - The physical addr of NAND flash to write. It must include sector addr.
	pucWriteBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
//Neet to Implement the FDB and FAT buffer for speed-up and ECC
s32 smcSectorsWrite(u32 unWriteSecCnt, u32 unSectorNum, u32 unWriteAddr, u8* pucWriteBuf)
{
    u32 i;
    u32	unAddrTemp;
    u32	unSecOffsetInBlock;
    u32	unSecOffsetInPage;
    u32 unBlockHead;		/* the page numbers in front of the write address within the same block */
    u32	unBlockTail;		/* the page numbers in back of the write address within the same block */
    u32	unWritePhyAddr;
    u32	unTemp;

    smcProcProt(SMC_SET_REQ, 0); /* set to Disable access */

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("\nDEBUG: -------smcSectorsWrite!-------\n");
#endif

    unWritePhyAddr = unSectorNum * FS_SECTOR_SIZE + SMC_FAT_START_ADDR;

    /* The area in front of SMC_RESERVED_SIZE is protected. In normal operation, it could not be modified. */
    if (unWritePhyAddr < SMC_RESERVED_SIZE)
        return 0;

    unSecOffsetInBlock = unSectorNum % smcSecPerBlock;
    unSecOffsetInPage = (unSecOffsetInBlock % smcSecPerPage);

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: unWritePhyAddr = %#x\n", unWritePhyAddr);
    DEBUG_SMC("DEBUG: unWriteSecCnt = %#x\n", unWriteSecCnt);
    DEBUG_SMC("DEBUG: unSectorNum = %#x\n", unSectorNum);
    DEBUG_SMC("DEBUG: unSecOffsetInBlock = %#x\n", unSecOffsetInBlock);
    DEBUG_SMC("DEBUG: pucWriteBuf[0] = %#x\n", *(u32*)pucWriteBuf);
#endif


    if (unSecOffsetInBlock==0 && unWriteSecCnt<=smcSecPerBlock)
    {	/* the write address is the first page of one block */

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: the write address is the first page of one block\n");
#endif
        if (smcCopyBlockActionSector(unWritePhyAddr, unSecOffsetInBlock, unSecOffsetInPage, unWriteSecCnt, pucWriteBuf)==0)
        {
            DEBUG_SMC("CopyBlock Action error on address %#x\n", unWritePhyAddr);
            return 0;
        }
    }
    else if (unSecOffsetInBlock!=0 && unWriteSecCnt<=smcSecPerBlock)
    {
        unBlockHead = smcSecPerBlock - unSecOffsetInBlock;

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: Less than smcSecPerBlock.\n");
        DEBUG_SMC("DEBUG: 1 unBlockHead = %#x\n", unBlockHead);
#endif

        /* avoid block head is too larger than write sector count */
        if (unBlockHead > unWriteSecCnt)
            unBlockHead = unWriteSecCnt;

#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: 2 unBlockHead = %#x\n", unBlockHead);
#endif
        if (smcCopyBlockActionSector(unWritePhyAddr, unSecOffsetInBlock, unSecOffsetInPage, unBlockHead, pucWriteBuf) == 0)
        {
            DEBUG_SMC("Copy Block Action error on Physical address %x", unWritePhyAddr);
            return 0;
        }
        unTemp = unBlockHead * FS_SECTOR_SIZE;

        pucWriteBuf += unTemp;
        unAddrTemp = unWritePhyAddr + unTemp;
        unBlockTail = unWriteSecCnt - unBlockHead;


#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: 2 unAddrTemp = %#x\n", unAddrTemp);
        DEBUG_SMC("DEBUG: 2 unWriteSecCnt = %d\n", unWriteSecCnt);
        DEBUG_SMC("DEBUG: 2 unBlockTail = %d\n", unBlockTail);
#endif
        if (unBlockTail!=0)
        {
            if (smcCopyBlockActionSector(unAddrTemp, 0, 0, unBlockTail, pucWriteBuf) == 0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n", unAddrTemp);
                return 0;
            }
        }
    }
    else if (unSecOffsetInBlock==0 && unWriteSecCnt>smcSecPerBlock)		/* write sectors are over one block */
    {        // Check Tail
#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: Check Tail\n");
#endif
        unBlockTail = unWriteSecCnt%smcSecPerBlock;

        unTemp = unWriteSecCnt/smcSecPerBlock;
        unAddrTemp = unWritePhyAddr;
        /* process whole blocks */
        for (i=0; i<unTemp; i++)
        {
            if (smcCopyBlockActionSector(unAddrTemp, 0, 0, smcSecPerBlock, pucWriteBuf) == 0)
            {
                DEBUG_SMC("Copy Block Action error on Physical address %x", unAddrTemp);
                return 0;
            }
            pucWriteBuf += smcBlockSize;
            unAddrTemp += smcBlockSize;
        }

        /* process the last pages in block */
        if (unBlockTail!=0)
        {
            if (smcCopyBlockActionSector(unAddrTemp, 0, 0, unBlockTail, pucWriteBuf) == 0)
            {
                DEBUG_SMC("Copy Block Action error on physical address %x", unAddrTemp);
                return 0;
            }
        }
    }
    else if (unSecOffsetInBlock!=0 && unWriteSecCnt>smcSecPerBlock)
    {    /* Check Head and Tail */
#ifdef FAT_WRITE_DEBUG
        DEBUG_SMC("DEBUG: Check Head and Tail\n");
#endif
        unBlockHead = smcSecPerBlock - unSecOffsetInBlock;
        if (unBlockHead != 0)
        {
            if (smcCopyBlockActionSector(unWritePhyAddr, unSecOffsetInBlock, unSecOffsetInPage, unBlockHead, pucWriteBuf) == 0)
            {
                DEBUG_SMC("Copy Block Action error on Physical address %x", unWritePhyAddr);
                return 0;
            }
            unTemp = unBlockHead * FS_SECTOR_SIZE;
            pucWriteBuf += unTemp;
            unWritePhyAddr = unWritePhyAddr + unTemp;
            unWriteSecCnt -= unBlockHead;
        }
        unBlockTail = unWriteSecCnt % smcSecPerBlock;

        unTemp = unWriteSecCnt/smcSecPerBlock;
        unAddrTemp = unWritePhyAddr;
        /* process whole blocks */
        for (i=0; i<unTemp; i++)
        {
            if (smcCopyBlockActionSector(unAddrTemp, 0, 0, smcSecPerBlock, pucWriteBuf) == 0)
            {
                DEBUG_SMC("Copy Block Action error on Physical address %x", unAddrTemp);
                return 0;
            }
            pucWriteBuf += smcBlockSize;
            unAddrTemp += smcBlockSize;
        }

        /* process the last pages in block */
        if (unBlockTail != 0)
        {
            if (smcCopyBlockActionSector(unAddrTemp, 0, 0, unBlockTail, pucWriteBuf) == 0)
            {
                DEBUG_SMC("Copy Block Action error on physical address %x", unAddrTemp);
                return 0;
            }
        }
    }

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("DEBUG: ======smcSectorsWrite!======\n\n\n");
#endif

    smcProcProt(SMC_SET_REQ, 1);	/* set to Enable access */

    return 1;

}



/*

Routine Description:

	Write by page base. The pages to write must be within the same block. This procedure will independt with FAT related R/W.

Arguments:

	writeCnt - Count of pages to write.
	logAddr - Logical page address. (This indicates the sector number in file system in advanced NAND.)
//	unSectorNum - The sector number in file system. It should be mapped to the responded page in advanced NAND.
	writeBuf - Buffer to write from.

Return Value:

	0 - Failure.
	1 - Success.

*/
//Neet to Implement the FDB and FAT buffer for speed-up and ECC
s32 smcPagesWrite(u32 writeCnt, u32 logAddr, u8* writeBuf)
{
    u32 pageOffset,temp;
    u32 i;
    u32 block_head;		/* the page numbers in front of the write address within the same block */
    u32	block_tail;		/* the page numbers in back of the write address within the same block */

    smcProcProt(SMC_SET_REQ, 0);	/* set to Disable access */

    if (logAddr<SMC_RESERVED_SIZE)
        return 0;

    pageOffset = logAddr % smcBlockSize;		// offset of page in a block
    pageOffset= pageOffset /smcPageSize;

    if (pageOffset==0 && writeCnt<=smcPagePerBlock)		/* the write address is the first page of one block */
    {
        if (smcCopyBlockAction(logAddr, pageOffset, writeCnt, writeBuf)==0)
        {
            DEBUG_SMC("CopyBlock Action error on address %#x\n",logAddr);
            return 0;
        }
    }
    else if (pageOffset!=0 && writeCnt<=smcPagePerBlock)		/* indicates the write address is within the any page of one block */
    {
        block_head = smcPagePerBlock - pageOffset;
        if (block_head > writeCnt)
            block_head = writeCnt;

        if (block_head != 0)
        {
            if (smcCopyBlockAction(logAddr, pageOffset, block_head, writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",logAddr);
                return 0;
            }
            writeBuf += block_head*smcPageSize;
            temp = logAddr + block_head*smcPageSize;
            writeCnt -= block_head;
        }
        block_tail = writeCnt%smcPagePerBlock;

        if (block_tail!=0)
        {
            if (smcCopyBlockAction(temp,0,block_tail,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",temp);
                return 0;
            }
        }

    }
    else if (pageOffset==0 && writeCnt>smcPagePerBlock)
    {        // Check Tail
        block_tail=writeCnt%smcPagePerBlock;
        for (i=0;i<writeCnt/smcPagePerBlock;i++)
        {
            temp=logAddr +i*smcBlockSize;
            if (smcCopyBlockAction(temp,pageOffset,smcPagePerBlock,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",temp);
                return 0;
            }
            writeBuf+=smcBlockSize;
        }

        if (block_tail!=0)
        {
            temp+=smcBlockSize; // because last for loop
            if (smcCopyBlockAction(temp,pageOffset,block_tail,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",temp);
                return 0;
            }
        }
    }
    else if (pageOffset!=0 && writeCnt>smcPagePerBlock)
    {    // Check Head and tail
        block_head=smcPagePerBlock-pageOffset;
        if (block_head!=0)
        {
            if (smcCopyBlockAction(logAddr,pageOffset,block_head,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",logAddr);
                return 0;
            }
            writeBuf += block_head*smcPageSize;
            logAddr = logAddr + block_head*smcPageSize;
            writeCnt -= block_head;
        }
        block_tail = writeCnt%smcPagePerBlock;

        for (i=0;i<writeCnt/smcPagePerBlock;i++)
        {
            temp=logAddr +i*smcBlockSize;
            if (smcCopyBlockAction(temp,0,smcPagePerBlock,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",temp);
                return 0;
            }
            writeBuf+=smcBlockSize;
        }

        if (block_tail!=0)
        {
            temp+=smcBlockSize; // because last for loop
            if (smcCopyBlockAction(temp,0,block_tail,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %#x\n",temp);
                return 0;
            }
        }
    }

    smcProcProt(SMC_SET_REQ, 1);	/* set to Enable access */

    return 1;

}

#endif

