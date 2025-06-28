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
#if 1
#include "general.h"
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))

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

extern u8 iconflag[UIACTIONNUM];  //civic 071001
extern s8 defaultvalue[]; //civic 071001
//extern SMC_REDUN_AREA smcRedunArea;
extern u8 userClickFormat;

extern s32 siuWBComp_RBRatio[4];
extern u8 cur_ev_value;

extern FRAME_BUF_OBJECT frame_buf_obj[MAX_FRM_OBJ];

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/* Application Function */

s32 smcSectorsRead(u32, u32, u8*);
s32 smcSectorsWrite(u32, u32, u8*);
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

    u8 i,j=0, check_val=0;
    BOOLEAN check = FALSE;
    if (ReadWrite)
    {		//read

        //Read icon flag from NAND

        if (smcSectorsRead(1,UI_logAddr,smcReadBuf)==0)
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
    #ifndef EBELL_UI
        iconflag[UI_MENU_SETIDX_VIDEO_FORMAT-LEVEL1_NODE_NUM] = UI_MENU_FORMAT_NO;
        iconflag[UI_MENU_SETIDX_DELETE-LEVEL1_NODE_NUM] = UI_MENU_DELETE_NO;
        iconflag[UI_MENU_SETIDX_DELETE_ALL-LEVEL1_NODE_NUM] = UI_MENU_DELETE_ALL_NO;
        iconflag[UI_MENU_SETIDX_FLASH_LIGHT-LEVEL1_NODE_NUM] = UI_MENU_FLASH_LIGHT_ALWAYS_OFF;
        cur_ev_value = iconflag[UI_MENU_SETIDX_EXPOSURE_VALUE - LEVEL1_NODE_NUM];
    #endif

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

	0 - Failure.
	1 - Success.

*/
u8 smcReadFBSetting(void)
{
    u32 i;
    u32 FB_addr;
    u8* frmptr= &smcWriteCache[0][0];
    u32* miptr;

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
        smcPage1ARead(FB_addr, frmptr);
        frmptr+=SMC_MAX_PAGE_SIZE;
        FB_addr+=SMC_MAX_PAGE_SIZE;
    }

    frmptr= &smcWriteCache[0][0];
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

    if (smcSectorsRead(SMC_AWB_SECTOR_SIZE,AWB_logAddr,smcReadBuf)==0)
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

    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        sysNAND_Disable();
        sysSD_Enable();
    }

    return flag;
}


u8 smcGetBitMap(void)
{
    u8 i;
    u8* readBuf=smcBitMap;
    u32 BitMap_logAddr;

    smcCheckTotalBlock();
    BitMap_logAddr = SMC_MAP_ADDR;	// Reservese size + 1 block

    for (i = 0; i < SMC_MAX_MAP_SIZE_IN_PAGE; i++)
    {
        if (smcSectorsRead(1, BitMap_logAddr, readBuf)==0)
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
    buffer[0x0B]=0x00;		//0x0200
    buffer[0x0C]=0x02;

    /* Sector Per Cluster */
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
    buffer[0x0D]=0x20;		//0x02
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
    buffer[0x0D]=0x04;		//0x04
#else
    buffer[0x0D]=0x20;		//0x02
#endif

    //RsvdSecCnt
    buffer[0x0E]=0x02;		//just reserve 20 sector for BPB
    buffer[0x0F]=0x00;
    //NUM_FATS
    buffer[0x10]=0x02;		//0x02
    //RootEntCnt
    buffer[0x11]=0x00;		//0x008 8 sectors
    buffer[0x12]=0x02;
    //TotSec16
    buffer[0x13]=TOSECT16 & 0xFF;		//0x7800-2 block 15MB
    buffer[0x14]=(TOSECT16>>8) & 0xFF;         //0x400 sector 32 block for bad block

    /* Media Type*/		/* 0xF8 -> Single Side, 80 track each side, 9 sectors each track */
    buffer[0x15]=0xF8;
    //FATSz16
    buffer[0x16]=0x0F;		//60 sectors for save FAT
    buffer[0x17]=0x00;
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
    NAND_FAT_PARAMETER.RsvdSecCnt = buffer[0x0E] + (buffer[0x0F]<<8);
    // Get the new FAT parameter
    NAND_FAT_PARAMETER.BytesPerSec = buffer[0x0B] + (buffer[0x0C]<<8);
    NAND_FAT_PARAMETER.FATSz16 = buffer[0x16] + (buffer[0x17]<<8);
    NAND_FAT_PARAMETER.NumFATs = buffer[0x10];
    NAND_FAT_PARAMETER.RootEntCnt = buffer[0x11] + (buffer[0x12]<<8);
    NAND_FAT_PARAMETER.SecPerClus = buffer[0xD];    // Important parameter it will affect NAND behavior
    NAND_FAT_PARAMETER.TotSec16= buffer[0x13] + (buffer[0x14]<<8);
    // calculate the essential NAND FAT cache parameter
    nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
    nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs*NAND_FAT_PARAMETER.FATSz16;
    nand_fat_size = NAND_FAT_PARAMETER.FATSz16;

    if (smcSectorsWrite(1, SMC_FAT_START_ADDR, (u8*)buffer)==0)
    {
        DEBUG_SMC("Write MBR error \n");
    }


}

u8 smcMakeBitMap(char jump_flag)        //0 --> Force format all NAND
{
    // Read Bit MAP
    u8 i;
    u8* readBuf=smcBitMap;
    u8* temp_buffer;
    u32 BitMap_logAddr;
    u32 fat_magic;
    u8 mbr_magic;
    u8* MbrBuf = smcMBRCache;
    u8* WriteBuf=&smcWriteCache[0][0];
#if 0
    memset((void*)smcBitMap,0xFF,SMC_MAX_MAP_SIZE_IN_BYTE);
    smcPageProgram(SMC_UI_LIBRARY_ADDR, smcBitMap, 0xAAAA);
    smcPageProgram(SMC_UI_LIBRARY_ADDR+0x100000, smcBitMap, 0xBBBB);
    smcPageProgram(SMC_UI_LIBRARY_ADDR+0x104000, smcBitMap, 0xBBBB);
    smcPageProgram(SMC_UI_LIBRARY_ADDR+0x108000, smcBitMap, 0xBBBB);
    smcPageProgram(SMC_UI_LIBRARY_ADDR+0x200000, smcBitMap, 0xCCCC);
    smcPageProgram(SMC_UI_LIBRARY_ADDR+0x280000, smcBitMap, 0xCCCC);
    smcPage2CRead(SMC_UI_LIBRARY_ADDR);
#endif
    if (jump_flag ==0)
        goto FORMAT;

    if (Got_BitMap==0)
    {
        Got_BitMap = smcGetBitMap();
    }

    BitMap_logAddr = SMC_FAT_START_ADDR;
    // read out MBR sector

    if (smcSectorsRead(1,BitMap_logAddr,MbrBuf)==0)
    {
        DEBUG_SMC("Read error on logical address %#x\n",BitMap_logAddr);
        return 0;
    }

    mbr_magic = *(u8*)smcMBRCache;
    if (mbr_magic==0xE9 ||mbr_magic==0xEB )
    {
        // Read The MBR and fill the NAND parameter
        NAND_FAT_PARAMETER.RsvdSecCnt = MbrBuf[14] + (MbrBuf[15]<<8);
        // Get the new FAT parameter
        NAND_FAT_PARAMETER.BytesPerSec = MbrBuf[11] + (MbrBuf[12]<<8);
        NAND_FAT_PARAMETER.FATSz16 = MbrBuf[22] + (MbrBuf[23]<<8);
        NAND_FAT_PARAMETER.NumFATs = MbrBuf[16];
        NAND_FAT_PARAMETER.RootEntCnt = MbrBuf[17] + (MbrBuf[18]<<8);
        NAND_FAT_PARAMETER.SecPerClus = MbrBuf[13];    // Important parameter it will affect NAND behavior
        NAND_FAT_PARAMETER.TotSec16= MbrBuf[19] + (MbrBuf[20]<<8);
        // calculate the essential NAND FAT cache parameter
        nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
        nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs*NAND_FAT_PARAMETER.FATSz16;
        nand_fat_size = NAND_FAT_PARAMETER.FATSz16;
        nand_data_start = nand_fat_end + NAND_FAT_PARAMETER.RootEntCnt*0x20/smcPageSize;

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
        DEBUG_SMC("nand_data_start = %#x\n", nand_data_start);

    }
    else
        jump_flag=0;  //force format

    // calculate the nand fat logical address
    BitMap_logAddr = SMC_FAT_START_ADDR + nand_fat_start*smcPageSize;
    // Read out the NAND FAT data in first sector
    temp_buffer=	&smcWriteCache[0][0];    //FAT

    if (smcSectorsRead(1,BitMap_logAddr,temp_buffer)==0)
    {
        DEBUG_SMC("Read error on logical address %#x\n",BitMap_logAddr);
        return 0;
    }

    // This is the initial if this is first run then it will record to nand
    fat_magic = *(u32*)temp_buffer;
    fat_magic = fat_magic & 0x00FFFFFF;		/* For FAT12, get the first 3 Bytes to determine if it is the end cluster or not */

    if (fat_magic!=0xFFFFF8 || jump_flag!=1)
    {
        fat_magic=fat_magic;
        jump_flag=0;
    }
    else
    {
        return 1;		// don't need to first initial
    }
FORMAT:
    DEBUG_SMC("Format SMC Media\n");

    smcTotalBlockErase(jump_flag);
    if (jump_flag!=1)
    {       // Make Bit Map
        memset((void*)smcBitMap,0xFF,SMC_MAX_MAP_SIZE_IN_BYTE);  // Need to modify
    }

    smcMakeMBR();

    for (i = 0; i < nand_fat_size; i++)
    {
        temp_buffer= &smcWriteCache[0][0];
        memset((void*)temp_buffer,0x00,smcPageSize);
        if (i==0)
        {
            *temp_buffer=0xF8;
            *(temp_buffer+1)=0xFF;
            *(temp_buffer+2)=0xFF;
            //*(temp_buffer+3)=0xFF;
        }
        BitMap_logAddr=nand_fat_start*smcPageSize + SMC_FAT_START_ADDR + i*smcPageSize;

        if (smcSectorsWrite(1,BitMap_logAddr , (u8*)temp_buffer)==0)
        {
            DEBUG_SMC("Write FAT error in MakeBitMap \n");
        }
    }

    // Clear The FDB region for Host PC writing
    for (i = 0; i < NAND_FAT_PARAMETER.RootEntCnt *0x20/smcPageSize; i++)
    {
        temp_buffer= &smcWriteCache[0][0];
        memset((void*)temp_buffer,0x00,smcPageSize);
        BitMap_logAddr=nand_fat_end*smcPageSize+ SMC_FAT_START_ADDR + i*smcPageSize;
        if (smcSectorsWrite(1, BitMap_logAddr , (u8*)temp_buffer)==0)
        {
            DEBUG_SMC("Write FAT error in MakeBitMap \n");
        }
    }
    return 2;
    //Read FAT to cache

}



u32 smcCheckBadBlock(u32 block_Addr)
{
    u8 i;

    if (Rerseved_Algo_Start)
    {
        for (i=0;i<SMC_RESERVED_BLOCK;i++)      //check whether this block is a bad one
        {
            if (SMC_BBM.Bad_Block_addr[i]==block_Addr)
            {
                return  SMC_BBM.New_Block_addr[i];
            }
        }

        return block_Addr;

    }
    else
        return block_Addr;

}

u8 smcCheckPageUsed(u32* BitMapAddr,u32 offset,u32 count)
{
    u32 i,copy_flag=0;
    u32 mask_data;
    u32 BitMapData=*BitMapAddr;

    for (i=0;i<count;i++)
    {
        mask_data=0x0;
        mask_data |= (1<<(offset+i));
        if (BitMapData & mask_data)
        {
            BitMapData = BitMapData & ~(1<<(offset+i));
            continue;
        }
        else
            copy_flag=1;
    }
    *BitMapAddr=BitMapData;
    return copy_flag;
}

u8 smcWriteDirectly(u32 addr,u32 count,u8* Srcbuf)
{
    u32 i;

    for (i=0;i<count;i++)
    {
        if (smcPageProgram(addr, Srcbuf, 0xFFFF))
        {
            addr += FS_SECTOR_SIZE;  // calculate each setor logical address
            Srcbuf+=smcPageSize;
        }
        else
            return 0;
    }

    return 1;
}

u8 smcCopyBlockTailHead(u32 BlockAddr,u32 Bit_map_dataed,u32 Offset,u32 count,u8* SrcBuf)
{
    u8 i,change_buffer=0;
    u8* read_buf = &smcWriteCache[0][0];
    u32 temp_addr=BlockAddr;

    if (Bit_map_dataed==0 && count==smcPagePerBlock && Offset==0)   // Clear all block without copy-back
    {
        change_buffer=1;
    }
    else
    {
        for (i = 0; i < smcPagePerBlock; i++)
        {
            if ( i>=Offset && i<(Offset+count))
            {		 //Update the write data to write cache
                memcpy(&smcWriteCache[i][0],SrcBuf,smcPageSize);
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
        read_buf= &smcWriteCache[0][0];
    // Re-write write cache to block
    temp_addr=BlockAddr;

    for (i = 0; i < smcPagePerBlock; i++)
    {
        if ((Bit_map_dataed & (1<<i))!=1)
        {	//page can't use
            if (smcPageProgram(temp_addr, read_buf, 0xFFFF)==0)
            {
                DEBUG_SMC("Write error on block byte address %#x",temp_addr);
                return 0;
            }
        }
        temp_addr += smcPageSize;
        read_buf += smcPageSize;
    }

    return 1;
}

u8 smcCopyBlockAction(u32 logical_addr,u32 Offset,u32 Count,u8* SrcBuf)
{
    u32 BlockByteAddr;
    u32 BlockAddr;
    u32 Bit_map_word_data;

    u8* read_buf = &smcWriteCache[0][0];
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
            DEBUG_SMC("Write directly error on address %x",logical_addr);
            return 0;
        }
    }
    else
    {
        if (BlockByteAddr<SMC_RESERVED_SIZE)
            return 0;

        if (smcCopyBlockTailHead(BlockByteAddr,Bit_map_word_data,Offset,Count,SrcBuf)==0)
        {
            DEBUG_SMC("Copy block back error on Logical address %x",BlockByteAddr);
            return 0;
        }
    }
    *bitmap_block=Bit_map_word_data;

    return 1;
}
/*

Routine Description:

	Read sectors. The sectors to read must be within the same block.

Arguments:

	readCnt - Count of pages to read.
	logAddr - Logical page address.
	readBuf - Buffer to read to.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSectorsRead(u32 readCnt, u32 logAddr, u8* readBuf)
{
    u32 i;
    u32 pageOffset,BlockAddr,BlockByteAddr;
    u32 bad_flag=0;

#if SMC_USE_LB_WRITE_CACHE /*CY 0907*/	/*CY 1023*/
    //u32 logBlockAddr;
#endif

    smcProcProt(SMC_SET_REQ, 0);	/* set to disable access */

    if (Rerseved_Algo_Start)
    {
        pageOffset = logAddr % smcBlockSize;		// offset of page in a block
        pageOffset = pageOffset / smcPageSize;

        BlockAddr = logAddr / smcBlockSize;  // offset of block
        BlockByteAddr = BlockAddr * smcBlockSize;

        for (i=0;i<SMC_RESERVED_BLOCK ;i++)      //check whether this block is a bad one
        {
            if (SMC_BBM.Bad_Block_addr[i] == BlockByteAddr)
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

    smcProcProt(SMC_GET_REQ, 0);	/* set to disable access */

    return 1;
}

/*

Routine Description:

	Write sectors.

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
s32 smcSectorsWrite(u32 writeCnt, u32 logAddr, u8* writeBuf)
{
    /*CY 0601 S*/
//#if SMC_USE_LB_WRITE_CACHE
#if 1


    u32 pageOffset,BlockByteAddr,temp;
    u32 i;
    u32 Bit_map_word_data;
    u32 block_head;		/* the page numbers in front of the write address within the same block */
    u32	block_tail;		/* the page numbers in back of the write address within the same block */

    smcProcProt(SMC_SET_REQ, 0);	/* set to disable access */
    if (logAddr<SMC_RESERVED_SIZE)
        return 0;

//    DEBUG_SMC("Logic Addr = %#x\n", logAddr);

    pageOffset = logAddr % smcBlockSize;		// offset of page in a block
    pageOffset= pageOffset /smcPageSize;

    if (pageOffset==0 && writeCnt<=smcPagePerBlock)		/* the write address is the first page of one block */
    {
        if (smcCopyBlockAction(logAddr, pageOffset, writeCnt, writeBuf)==0)
        {
            DEBUG_SMC("CopyBlock Action error on address %x",logAddr);
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
                DEBUG_SMC("CopyBlock Action error on address %x",logAddr);
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
                DEBUG_SMC("CopyBlock Action error on address %x",temp);
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
                DEBUG_SMC("CopyBlock Action error on address %x",temp);
                return 0;
            }
            writeBuf+=smcBlockSize;
        }

        if (block_tail!=0)
        {
            temp+=smcBlockSize; // because last for loop
            if (smcCopyBlockAction(temp,pageOffset,block_tail,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %x",temp);
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
                DEBUG_SMC("CopyBlock Action error on address %x",logAddr);
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
                DEBUG_SMC("CopyBlock Action error on address %x",temp);
                return 0;
            }
            writeBuf+=smcBlockSize;
        }

        if (block_tail!=0)
        {
            temp+=smcBlockSize; // because last for loop
            if (smcCopyBlockAction(temp,0,block_tail,writeBuf)==0)
            {
                DEBUG_SMC("CopyBlock Action error on address %x",temp);
                return 0;
            }
        }
    }

    smcProcProt(SMC_GET_REQ, 0);	/* set to disable access */

    return 1;

//---------------------------------------------
#else
    u32 pageOffset,BlockAddr,BlockByteAddr,temp;
    u8 i,j,k,bad_flag=0;
    u32 page_num;
    u32 Bit_map_move;
    u8 Bit_map_rest,Bit_map_data,change_buffer=0;
    u32 Bit_map_word_data;

    u8* bitmap_page = smcBitMap;	//0xFF means the eight pages can program
    u8* read_buf = &smcWriteCache[0][0];
    u32* bitmap_block = (u32* )smcBitMap; //0xFFFFFFFF means this block had erased

    k=0;

    for (j=0;j<writeCnt;j++)
    {
        read_buf = &smcWriteCache[0][0];
        bitmap_page = smcBitMap;
        bitmap_block = (u32* )smcBitMap;

        pageOffset = logAddr % smcBlockSize;		// offset of page in a block
        pageOffset= pageOffset /smcPageSize;
        BlockAddr = logAddr / smcBlockSize;  // offset of block
        BlockByteAddr = BlockAddr*smcBlockSize;
        temp=BlockByteAddr;
#if 1
        // Bad block replace
        if (Rerseved_Algo_Start)
        {
            for (i=0;i<SMC_RESERVED_BLOCK;i++)      //check whether this block is a bad one
            {
                if (SMC_BBM.Bad_Block_addr[i]==BlockByteAddr)
                {
                    bad_flag=1;
                    break;
                }
            }

            if (bad_flag)       //re-calculate the logAddr
            {
                logAddr = SMC_BBM.New_Block_addr[i] + pageOffset*smcPageSize;

                pageOffset = logAddr % smcBlockSize;		// offset of page in a block
                pageOffset= pageOffset /smcPageSize;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr*smcBlockSize;
                temp=BlockByteAddr;
                bad_flag=0;
            }
        }
#endif
        page_num = logAddr /smcPageSize;	// offset of pages in device
        Bit_map_move = page_num / 8;
        Bit_map_rest = page_num % 8;

        bitmap_page += Bit_map_move;
        Bit_map_data = *bitmap_page; 	// Get the eight page bit map data

        if ((Bit_map_data& (1<<Bit_map_rest)))
        {	//page can use without erasing block
            smcPageProgram(logAddr, writeBuf, BlockAddr);
            Bit_map_data = Bit_map_data & ~(1<<Bit_map_rest); // Update Bit Map Page info
            *bitmap_page = Bit_map_data;	//The Bit Map Info will write back after calling dcfclose
            logAddr += FS_SECTOR_SIZE;  // calculate each setor logical address
            writeBuf+=smcPageSize;
            k++;    //record how many times it program
            continue;
            //return 1;
        }
        else
        {	// We need to erase block so check the sector offset
            if (writeCnt==0x40)
                writeCnt=writeCnt;
            writeCnt=writeCnt-k;
            // If the sector + writecnt <= block align then do it at one time
            if ((pageOffset+writeCnt)<=(smcPagePerBlock))
                j=writeCnt;
            //If the sector + writecnt >block align then do n times (depend on writeCnt)
            if ((pageOffset+writeCnt)>(smcPagePerBlock))
            {
                j=(smcPagePerBlock-pageOffset-1);
                logAddr += (j)*FS_SECTOR_SIZE;
            }
            bitmap_block += BlockAddr;
            Bit_map_word_data = *bitmap_block;		// Show the bit map status of block

            if (Bit_map_word_data==0 && pageOffset==0 && writeCnt==smcPagePerBlock)
                change_buffer=1;
            else
            {    //Read all the data within a block to write cache
                for (i = 0; i < smcPagePerBlock; i++)
                {
                    if ( i>=pageOffset && i<(pageOffset+writeCnt))
                    {
                        //Update the write data to write cache
                        memcpy(&smcWriteCache[i][0],writeBuf,smcPageSize);
                        writeBuf+=smcPageSize;
                    }
                    else
                    {
                        if ((Bit_map_word_data & (1<<i))!=1)
                        { // If the page can use then it shows it had no data so memset 0xff
                            if (smcPage1ARead(temp,read_buf)==0)
                            {
                                DEBUG_SMC("read error on block address %#x",BlockAddr);
                                return 0;
                            }
                        }
                    }
                    temp += smcPageSize;
                    read_buf += smcPageSize;
                }

            }
            //erase the block
            smcBlockErase(BlockByteAddr);
            if (change_buffer)
                read_buf=writeBuf;
            else
                read_buf= &smcWriteCache[0][0];
            // Re-write write cache to block
            temp=BlockByteAddr;
            for (i = 0; i < smcPagePerBlock; i++)
            {
                if ((Bit_map_word_data & (1<<i))!=1)
                {	//page can't use
                    if (smcPageProgram(temp, read_buf, BlockAddr)==0)
                    {
                        DEBUG_SMC("Write error on block byte address %#x",BlockByteAddr);
                        return 0;
                    }
                }
                temp += smcPageSize;
                read_buf += smcPageSize;
            }
            //Won't need to update the Bit Map data
            return 1;
        }

    }

#endif
    /*CY 0601 E*/
}
#endif
#endif
