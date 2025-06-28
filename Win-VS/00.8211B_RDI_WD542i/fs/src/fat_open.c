/*
**********************************************************************
*                          Micrium, Inc.
*                      949 Crestview Circle
*                     Weston,  FL 33327-1848
*
*                            uC/FS
*
*             (c) Copyright 2001 - 2003, Micrium, Inc.
*                      All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File        : fat_open.c
Purpose     : FAT routines for open/delete files
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "fsapi.h"

#ifndef FS_FARCHARPTR
#define FS_FARCHARPTR char *
#endif

#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_fat.h"
#include "fs_clib.h"

#include "General.h"
#include "Rtcapi.h"
#include "smcapi.h"
#include "sysapi.h"
#include "siuapi.h"
#include "i2capi.h"

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/
#ifndef FS_FAT_NOFAT32
#define FS_FAT_NOFAT32        0
#endif /* FS_FAT_NOFAT32 */


/*********************************************************************
*
*             Global Variable
*
**********************************************************************/
static int FileEntSect=0;
static int FileEntSect_offset=0;
/*********************************************************************
*
*             Extern Global Variable
*
**********************************************************************/
extern char *cMulBlockBuffer;

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern FS_u32 smcTotalSize, smcBlockSize, smcPageSize, smcPagePerBlock;
extern u32 smcSecPerBlock, smcSecPerPage;
extern unsigned char	 smcBitMap[SMC_MAX_MAP_SIZE_IN_BYTE];
extern int smcErase(FS_u32, FS_u32 , FS_u32);
extern unsigned char  gInsertNAND;
#endif

extern FS_DISKFREE_T global_diskInfo;
extern s32 dcfLasfEofCluster;
extern u8 siuOpMode;
extern u8 sysDeleteFATLinkOnRunning;
/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/

int _FS__fat_DeleteFATLink( int Idx, int Unit,int todo,int curclst);
extern s32 sysbackLowSetEvt(s8 cause, s32 param1,s32 param2,s32 param3,s32 param4);



/*********************************************************************
*
*             Local functions body
*
**********************************************************************
*/

/*********************************************************************
*
*             _FS_fat_find_file
*
  Description:
  FS internal function. Find the file with name pFileName in directory
  DirStart. Copy its directory entry to pDirEntry.
     搜尋整個目錄(sector by sector), 直到找到該檔名為止.
  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pFileName   - File name.
  pDirEntry   - Pointer to an FS__fat_dentry_type data structure.
  DirStart    - 1st cluster of the directory.
  DirSize     - Sector (not cluster) size of the directory.
  pEmptyFileEntSec        -search for empty file entry sector; -1: not found; others: found out.
  pEmptyFileEntSectOffset -search for sector offset of the empty file entry.

  Return value:
  >=0         - File found. Value is the first cluster of the file.
  <0          - An error has occured.
**********************************************************************/

static FS_i32 _FS_fat_find_file(int Idx, FS_u32 Unit, const char *pFileName,
                                   FS__fat_dentry_type *pDirEntry,
                                   FS_u32 DirStart, FS_u32 DirSize
                                  )
{
    FS__fat_dentry_type *s;
    FS_u32 i;
    FS_u32 dsec;
    int len;
    int err;
    int c;
    char *buffer;
    int offset;
    unsigned int SecPerClus;

    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
    {
        len = 11;
    }
    /* Read directory */
    for (i = 0; i < DirSize; i++)   
    {
        if( (i % SecPerClus)==0) //Lucian: optimize here..
           dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
        else
           dsec +=1;
        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
     #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
     #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
     #endif
        if (err < 0)        
        {
            FS__fat_free(buffer);
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }
            c = FS__CLIB_strncmp((char*)s->data, pFileName, len);
            if (c == 0)
            {  /* Name does match */
                if (s->data[11] & FS_FAT_ATTR_ARCHIVE)
                {
                    break;  /* Entry found */
                }
            }
            offset ++;
            s++;
        }
        
        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            /* Entry found. Return number of 1st block of the file */

            if (pDirEntry)
            {
                FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
            }
            dsec  = (FS_u32)s->data[26];
            dsec += (FS_u32)s->data[27] * 0x100UL;
            dsec += (FS_u32)s->data[20] * 0x10000UL;
            dsec += (FS_u32)s->data[21] * 0x1000000UL;
            FS__fat_free(buffer);
            return ((FS_i32)dsec); //回傳該檔案的 first cluster
        }
    }
    FS__fat_free(buffer);
    return -1;
}


/*********************************************************************
*
*             _FS_fat_IncDir_One
*
  Description:
  FS internal function. Increase directory starting at DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory.
  pDirSize    - Pointer to an FS_u32, which is used to return the new
                sector (not cluster) size of the directory.

  Return value:
  ==1         - Success.
  ==-1        - An error has occured.
*/

int _FS_fat_IncDir_One(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize)
{
    FS_u32 i;
    FS_u32 dsec;
    FS_i32 last;
    char *buffer;
    int err;
    unsigned char SecPerClus;

    SecPerClus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    
    if (DirStart == 0)
    {
        /* Increase root directory only, if not FAT12/16  */
  
        i = FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt;
        if (i != 0)
        {   //FAT12/16 無法增加entry of  root directory 
            return -1;  /* Not FAT32 */
        }
    }
    last = FS__fat_FAT_find_eof(Idx, Unit, DirStart, 0);
    if (last < 0)
    {
        return -1;  /* No EOF marker found */
    }
    last = FS__fat_FAT_allocOne(Idx, Unit, last,1);  /* Allocate new cluster. allocate one cluse once */
    if (last < 0)
    {
        return -1;
    }

    *pDirSize = *pDirSize + SecPerClus; //update director size.
    
    /* Clean new directory cluster */
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }

    //將entry in the cluster都清為零(empty tag).
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
    for (i = *pDirSize - SecPerClus; i < *pDirSize; i++)
    {
        if( (i % SecPerClus)==0 ) //Lucian: optimize here..
           dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
        else
           dsec +=1;
        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    #if FS_RW_DIRECT
        err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #else
        err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    FS__fat_free(buffer);
    return 1;
}

/*********************************************************************
*
*             _FS_fat_IncDir_Multi
*
  Description:
  FS internal function. Increase directory starting at DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory.
  pDirSize    - Pointer to an FS_u32, which is used to return the new
                sector (not cluster) size of the directory.
  IncDirSize  - Want to increase Dir size (sector unit)

  Return value:
  ==1         - Success.
  ==-1        - An error has occured.
*/

int _FS_fat_IncDir_Multi(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize, unsigned int IncDirSize)
{
    FS_u32 i;
    FS_u32 dsec;
    FS_i32 last;
    char *buffer;
    int err;
    unsigned char SecPerClus;
    int AllocFatNum;
    FS_u32 *pclstBuf=(FS_u32*)cMulBlockBuffer; 
    //----------------------------------------------//

    SecPerClus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    AllocFatNum=(IncDirSize + SecPerClus -1 )/SecPerClus; //Lucian: 轉換成cluster unit.(無條件進位)

    if(AllocFatNum == 0)
        return 1;
    
    if (DirStart == 0)
    {
        /* Increase root directory only, if not FAT12/16  */
        i = FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt;
        if (i != 0)
        {   //FAT12/16 無法增加entry of  root directory 
            return -1;  /* Not FAT32 */
        }
    }
    last = FS__fat_FAT_find_eof(Idx, Unit, DirStart, 0);
    if (last < 0)
    {
        return -1;  /* No EOF marker found */
    }

    last = FS__fat_FAT_allocMulti(Idx, Unit, last, AllocFatNum, pclstBuf); //Lucian: allocat multi cluster
    if (last < 0)
    {
        return -1;
    }

    *pDirSize = *pDirSize + SecPerClus*AllocFatNum; //update director size.
    
    /* Clean new directory cluster */
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }

    //將entry in the cluster都清為零(empty tag).
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

    for (i = *pDirSize - SecPerClus*AllocFatNum; i < *pDirSize; i++)
    {
        if( (i % SecPerClus)==0 ) //Lucian: optimize here..
           dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
        else
           dsec +=1;
        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    #if FS_RW_DIRECT
        err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #else
        err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    FS__fat_free(buffer);

    return 1;
}


/*********************************************************************
*
*             _FS_fat_create_file
*
  Description:
  FS internal function. Create a file in the directory specified
  with DirStart. Do not call, if you have not checked before for
  existing file with name pFileName.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pFileName   - File name.
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Sector size of the directory starting at DirStart.
  EmptyFileEntSect       -to hint the location(sector unit) for searching emypty entry.
  Return value:
  >=0         - 1st cluster of the new file.
  ==-1        - An error has occured.
  ==-2        - Cannot create, because directory is full.
*/

static FS_i32 _FS_fat_create_file(int Idx, FS_u32 Unit,  const char *pFileName,
                                      FS_u32 DirStart, FS_u32 DirSize,
                                      int *pEmptyFileEntSect,
                                      int *pEmptyFileEntSect_offset)
{
    FS__fat_dentry_type *s;
    FS_u32 sect,offset;
    FS_u32 dsec;
    FS_i32 cluster;
    int len;
    int err;
    FS_u16 val;
    char *buffer;
    RTC_DATE_TIME   localTime;
    int scan,first,SecPerClus;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
    {
        len = 11;
    }

    //if( (*pEmptyFileEntSect<0) || (*pEmptyFileEntSect>=DirSize)) //代表之前沒找到,便從頭開始找
    {
         *pEmptyFileEntSect=0;
         *pEmptyFileEntSect_offset=0;
    }
    /*-------------------- Read directory ----------------------*/
    /*
        目前做法是: Scan all directory (sector by sector). 從cluster頭到尾,直到找到空的file entry.
        新的作法:
    */
    
    scan=0;
    first=1;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;

    sect = *pEmptyFileEntSect;

    while(1)
    {
        if(sect>=DirSize)
        {
           scan ++;
           DEBUG_FS("\nFile Entry SCAN=%d\n",scan);
           if (scan > 1)
           {
               DEBUG_FS("File Entry  is full\n");
               FS__fat_free(buffer);
               return -2;      /* Directory is full */
           }
           sect=0;

        }

        if( (first==1) || ((sect % SecPerClus)==0) )
        {
           dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, sect);
        }
        else
        {
           dsec +=1;
        }
        first=0;

        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
     #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
     #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
     #endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
        
        s = (FS__fat_dentry_type*)buffer;

        if(sect == *pEmptyFileEntSect)
        {
           offset=*pEmptyFileEntSect_offset;
           s += offset;
        }
        else
           offset=0;

        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }
            if (s->data[0] == 0x00)
            {
                break;  /* Empty entry found */
            }
            if(strncmp(s->data, pFileName, FS_V_FAT_ENTEY_SHORT_NAME) == 0)
            {
            	DEBUG_FS("[ERR] FS_FILE_NAME_REPEAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
            	FS__fat_free(buffer);
            	return FS_FILE_NAME_REPEAT_ERR;
            }
            s++;
            offset ++;
        }
        
        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            *pEmptyFileEntSect = sect;
            *pEmptyFileEntSect_offset=offset;
            
            DEBUG_FS("EntSect = (%d, %d), LasfEofClust = %d\n", sect, offset, dcfLasfEofCluster);
            
            /* Free entry found. Make entry and return 1st block of the file */
            FS__CLIB_strncpy((char*)s->data, pFileName, len);
            s->data[11] = FS_FAT_ATTR_ARCHIVE;
            /* Alloc block in FAT */
            cluster = FS__fat_FAT_allocOne(Idx, Unit, dcfLasfEofCluster,0);
            dcfLasfEofCluster=cluster;
            //DEBUG_FS("FileOpen:dcfLasfEofCluster=%d\n",dcfLasfEofCluster);
            //----紀錄 create time , modified time---//
            if (cluster >= 0)
            {
                s->data[12]     = 0x00;                           /* Res */
                s->data[13]     = 0x00;                           /* CrtTimeTenth (optional, not supported) */
                s->data[22]     = 0x00;                           /* CrtTime (optional, not supported) */
                s->data[23]     = 0x00;
                s->data[24]     = 0x00;                           /* CrtDate (optional, not supported) */
                s->data[25]     = 0x00;

                RTC_Get_Time(&localTime);
                val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

                s->data[14]     = (unsigned char)(val & 0xff);   /* WrtTime */
                s->data[15]     = (unsigned char)(val / 256);           
                // + 2000 -1980 =20
                val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

                s->data[16]     = (unsigned char)(val & 0xff);   /* WrtDate */
                s->data[17]     = (unsigned char)(val / 256);
                s->data[18]     = (unsigned char)(val & 0xff);		/* LstAccDate (optional, not supported) */
                s->data[19]     = (unsigned char)(val / 256);
                s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */
                s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
                s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
                s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
                s->data[28]     = 0x00;                           /* FileSize */
                s->data[29]     = 0x00;
                s->data[30]     = 0x00;
                s->data[31]     = 0x00;
            #if FS_RW_DIRECT
                err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
            #else
                err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            //======//
            FS__fat_free(buffer);
            return cluster;
        }

        sect ++;
    }
    FS__fat_free(buffer);
    return -2;      /* Directory is full */
}


/*********************************************************************
*
*             Global functions section 1
*
**********************************************************************

  Functions in this section are global, but are used inside the FAT
  File System Layer only.

*/

/**********************************************************************
*
*             FS__fat_DeleteFileOrDir
*
  Description:
  FS internal function. Delete a file or directory.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pName       - File or directory name.
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Sector size of the directory starting at DirStart.
  RmFile      - 1 => remove a file
                0 => remove a directory

  Return value:
  >=0         - Success.
  <0          - An error has occured.
************************************************************************/
//Just kill the FAT(FDB) but no kill the Data

FS_i32 FS__fat_DeleteFileOrDir(int Idx, FS_u32 Unit,  const char *pName,
                                    FS_u32 DirStart, FS_u32 DirSize, FS_u16 FileEntrySect,char RmFile)
{
    FS__fat_dentry_type *s;
    FS_u32 dsec;
    FS_u32 sect;
    FS_u32 value;
    FS_u32 filesize;
    FS_i32 len;
    FS_i32 curclst;
    FS_i32 todo;
    char *buffer;
    int ret;
    int lexp;
    int x;
    int bytespersec;
    int SecPerClus;
    int scan,first,offset;

    //--------//
    bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    
    len = FS__CLIB_strlen(pName);
    if (len > 11)
    {
        len = 11;
    }

    //DEBUG_FS("EstimateFileEntrySect=%d\n",FileEntrySect);
    if(FileEntrySect>=DirSize)
        FileEntrySect=0;

    scan=0;
    first=1;
    sect = FileEntrySect;
    /* Read directory */
    while (1) //DirSize: sector unit.
    {
        curclst = -1;

        if(sect>=DirSize)
        {
           scan ++;
           DEBUG_FS("\nFile Entry SCAN=%d\n",scan);
           if (scan > 1)
           {
               DEBUG_FS("Error! File Entry  is not found.\n");
               FS__fat_free(buffer);
               return FS_FILE_ENT_FIND_ERROR;      
           }
           sect=0;
        }
        
        if( (first==1) || ((sect % SecPerClus)==0) )
        {
           dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, sect);
        }
        else
        {
           dsec +=1;
        }
        first=0;
        
        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    #if FS_RW_DIRECT
        ret = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #else
        ret = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
    #endif
        if (ret < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
        /* Scan for pName in the directory sector */
        s = (FS__fat_dentry_type*) buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + bytespersec))
            {
                break;  /* End of sector reached */
            }
            x = FS__CLIB_strncmp((char*)s->data, pName, len);
            if (x == 0)
            { /* Name does match */
                if (s->data[11] != 0)
                {
                    break;  /* Entry found */
                }
            }
            s++;
            offset ++;
        }
        if (s < (FS__fat_dentry_type*)(buffer + bytespersec))
        {
            //DEBUG_FS("DeleteFileEntry=(%d,%d)\n",sect,offset);
            /* Entry has been found, delete directory entry */
            s->data[0]  = 0xe5;		//set file name E5,表示這個條目曾經被刪除不再有用.
            s->data[11] = 0;			//Set attribute 0
        #if FS_RW_DIRECT
            ret = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); //FDB修改後寫回
        #else
            ret = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); //FDB修改後寫回
        #endif
            if (ret < 0)
            {
                FS__fat_free(buffer);
                return -1;
            }
            /* Free blocks in FAT */
            /*
               For normal files, there are no more clusters freed than the entrie's filesize
               does indicate. That avoids corruption of the complete media in case there is
               no EOF mark found for the file (FAT is corrupt!!!). 
               If the function should remove a directory, filesize if always 0 and cannot
               be used for that purpose. To avoid running into endless loop, todo is set
               to 0x0ffffff8L, which is the maximum number of clusters for FAT32.
            */
            if (RmFile) //delete file
            {
                filesize  = s->data[28] + 0x100UL * s->data[29] + 0x10000UL * s->data[30] + 0x1000000UL * s->data[31];
                todo      = filesize / (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec); //todo --> how much cluster
                value     = filesize % (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec);
                if (value != 0)
                {
                    todo++;
                }

            }
            else //delete directory
            {
                
                todo = 256; //Lucian: 預估值, =DCF_FILEENT_MAX*32/512/SectPerCluster
            }

            curclst = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];

            
		  #if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)) 
            /*
                Lucian: 於DVR file system中,循環錄影模式下,將Delete FAT link放在background task 下 執行.
            */
            if(siuOpMode == SIUMODE_MPEGAVI)
            {
               sysbackLowSetEvt(SYSBACKLOW_EVT_DELETEFATLINK,Idx,Unit,todo,curclst);
#if !FS_NEW_VERSION 
               global_diskInfo.avail_clusters +=todo; // 由於在background 做, cluster. available cluster 先加回.
#endif 
               ret=1;
            }
            else
            {
               ret=_FS__fat_DeleteFATLink(Idx,Unit,todo,curclst);
               ret=1; //Lucian: 忽略 delete link error. 因為entry 已經砍掉
            }
          #else
            ret=_FS__fat_DeleteFATLink(Idx,Unit,todo,curclst);
            ret=1; //Lucian: 忽略 delete link error.  因為entry 已經砍掉
          #endif
#if !FS_NEW_VERSION                
            //DEBUG_FS("Available Cluster=%d\n",global_diskInfo.avail_clusters);
            if ( global_diskInfo.avail_clusters >global_diskInfo.total_clusters)
                global_diskInfo.avail_clusters = global_diskInfo.total_clusters;
#endif
            FS__fat_free(buffer);
            return ret;
        } /*  Delete entry */

        sect ++;
    } /* for */
    DEBUG_FS("Cannot Find the delete file\n");
    FS__fat_free(buffer);
    return curclst;
}

/**********************************************************************
*
*             _FS__fat_DeleteFATLink
*
  Description:
  FS internal function. Delete FAT link of a file or directory.

  Parameters:
  

  Return value:
  =1          - Not Search EOF.
  =0          - Success.
  <0          - An error has occured.
************************************************************************/
//Just kill the FAT(FDB) but no kill the Data
int _FS__fat_DeleteFATLink( int Idx, int Unit,int todo,int curclst)
{
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
#endif
    int lastsec,fatindex,fatsec,fatoffs,NextFatSec;
    int err;
    int fattype;
    unsigned int fatsize;
    int bytespersec;
    char *buffer;
    int RsvdSecCnt;

    unsigned int  i;
    unsigned char a;
    unsigned char b;
#if (FS_FAT_NOFAT32==0)
    unsigned char c;
    unsigned char d;
#endif 
    FS_i32 Data_start_sector;
    FS_i32 Root_start_sector;

    FS_u32* bitmap_block ; //0xFFFFFFFF means this block had erased
    FS_u32  logAddr_sector,logAddr; //magic
    FS_u32  BlockAddr,BlockByteAddr,bitmap_block_data;
    FS_u8   sector_per_clustrer,cluster_per_block,sector_offset,cluster_offset,wait_times,recalculate,j,k;
    FS_u32  first_temp_cluster,second_temp_cluster;
    FS_u8   bad_flag=0,m;
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    extern FS_u8 Rerseved_Algo_Start;
#endif

    //------------------------------------------------------------------//
    sysDeleteFATLinkOnRunning=1;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        sysDeleteFATLinkOnRunning=0;
        return -1;
    }

    if( (todo==0) || (curclst==0))
    {
        DEBUG_FS("Delete FAT-Link invalid parameter\n");
        FS__fat_free(buffer);
        return -1;
    }
    
    Root_start_sector = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    Data_start_sector = (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt*0x20)/ FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec + Root_start_sector;
    
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0)
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }
    bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    if (gInsertNAND)
    {
        recalculate=1;
        sector_per_clustrer= NAND_FAT_PARAMETER.SecPerClus;
        cluster_per_block = smcSecPerBlock / sector_per_clustrer;
    }
#endif
    lastsec = -1;

    


#if FILE_SYSTEM_DVF_TEST
    time1=OSTimeGet();
#endif
    DEBUG_FS("Delete FAT-Link: (%d,%d)\n",curclst,todo);
    RsvdSecCnt=FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
    while (todo)
    {
        first_temp_cluster= curclst;
        if (fattype == 1)
        {
            fatindex = curclst + (curclst / 2);    /* FAT12 */
        }
        else if (fattype == 2)
        {
            fatindex = curclst * 4;               /* FAT32 */
        }
        else
        {
            fatindex = curclst * 2;               /* FAT16 */
        }

        if(curclst == 0)
        {
           DEBUG_FS("Cannot Find EOF! #3 (%d,%d)\n",curclst,todo);
           sysDeleteFATLinkOnRunning=0;
           FS__fat_free(buffer);
           return 1;
        }
        //fatsec means the specify sector in FAT
        //用fatindex(byte unit)算出該file再FAT table 的位置.
        fatsec = RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;

    #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==0x20)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)      //check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[m]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1)       //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[m];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr*smcBlockSize;
            bitmap_block += BlockAddr;
            *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
        }
    #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==SEC_PER_CLS)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * FS_SECTOR_SIZE + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)		//check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[m]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1) 	  //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[m];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr * smcBlockSize;

            bitmap_block += (BlockAddr * (smcPagePerBlock / 32));	/* 32 represents 32 bits of one word. One word represents 32 pages. */

            {
                u8	k;

                for (k=0; k<(smcPagePerBlock / 32); k++)
                    *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
            }
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
            //civic 070910 E
        }
    #endif

        if (fatsec != lastsec) //若是在同一個sector,則不必再重覆讀取
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        if (fattype == 1)  //FAT-12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
                if (curclst & 1)
                {
                    buffer[fatoffs] &= 0x0f;
                }
                else
                {
                    buffer[fatoffs]  = 0x00;
                }
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
             #endif
                lastsec = fatsec + 1;
                b = buffer[0];
                if (curclst & 1)
                {
                    buffer[0]  = 0x00;
                    curclst = ((a & 0xf0) >> 4) + 16 * b;
                }
                else
                {
                    buffer[0] &= 0xf0;
                    curclst = a + 256 * (b & 0x0f);

                }
                NextFatSec= RsvdSecCnt + ( (curclst + (curclst / 2)) / bytespersec);

                if(NextFatSec != fatsec + 1)
                {
                #if FS_RW_DIRECT
                    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                #else
                    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                #endif
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                }
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
                if (curclst & 1)
                {
                    buffer[fatoffs]     &= 0x0f;
                    buffer[fatoffs + 1]  = 0x00;
                    curclst = ((a & 0xf0) >> 4) + 16 * b;

                }
                else
                {
                    buffer[fatoffs]      = 0x00;
                    buffer[fatoffs + 1] &= 0xf0;
                    curclst = a + 256 * (b & 0x0f);

                }

                NextFatSec= RsvdSecCnt + ( (curclst + (curclst / 2)) / bytespersec);

                if(NextFatSec != fatsec)
                {
                #if FS_RW_DIRECT
                    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                #else
                    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                #endif
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                }
            }
            
            global_diskInfo.avail_clusters ++;
            
            curclst &= 0x0fff;
            if (curclst >= 0x0ff8)
            {

            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif

            }
        }
        else if (fattype == 2) //FAT32
        { /* FAT32 */
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3] & 0x0f;
            buffer[fatoffs]      = 0x00; //FAT table 填零表示該cluster 不被佔用.也就是刪除的意思.
            buffer[fatoffs + 1]  = 0x00;
            buffer[fatoffs + 2]  = 0x00;
            buffer[fatoffs + 3]  = 0x00;
            curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;

            global_diskInfo.avail_clusters ++;

            NextFatSec= RsvdSecCnt + (curclst * 4 / bytespersec);
            if(NextFatSec != fatsec)
            {
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            if (curclst >= (FS_i32)0x0ffffff8L) //判斷是否為EOF
            {
                
            #if FILE_SYSTEM_DVF_TEST
                time2=OSTimeGet();
                //DEBUG_FS("--->Delete FAT-Link Time=%d (x50msec)\n",time2-time1);
            #endif
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif
            }
        }
        else
        {	//FAT 16
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            buffer[fatoffs]     = 0x00;
            buffer[fatoffs + 1] = 0x00;
            curclst  = a + 256 * b;
            curclst &= 0xffff;
            
            global_diskInfo.avail_clusters ++;
            
            NextFatSec= RsvdSecCnt + (curclst * 2 / bytespersec);
            if(NextFatSec != fatsec)
            {
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            if (curclst >= (FS_i32)0xfff8)
            {
                
            #if FILE_SYSTEM_DVF_TEST
                time2=OSTimeGet();
                //DEBUG_FS("--->Delete FAT-Link Time=%d (x50msec)\n",time2-time1);
            #endif
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                /* Attention!! sector_per_cluster will be changed due to different media size */
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif
            }
        }
        second_temp_cluster= curclst;

  #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
                logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcPagePerBlock;
                cluster_offset = sector_offset /sector_per_clustrer;
                wait_times =smcPagePerBlock/sector_per_clustrer;
                wait_times-=cluster_offset;
                if (todo < wait_times )
                    wait_times=todo;
                recalculate=0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j=0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr*smcBlockSize;
                bitmap_block += BlockAddr;
            }

            // Judge the erasing cluster is continue or not
            if (second_temp_cluster - first_temp_cluster==1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate=1;
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                first_temp_cluster= curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);
                    // Update The Bit map Info
                    *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    bitmap_block_data=*bitmap_block;
                    for (k=0;k<smcPagePerBlock;k++)
                    {
                        if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                            bitmap_block_data = bitmap_block_data | (1<<k);
                    }
                    *bitmap_block=bitmap_block_data;
                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j+wait_times)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            }
        }
  #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)

        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap

                logAddr = logAddr_sector * FS_SECTOR_SIZE+ SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcSecPerBlock;
                cluster_offset = sector_offset / sector_per_clustrer;

                wait_times = smcSecPerBlock / sector_per_clustrer;
                wait_times -= cluster_offset;
                if (todo < wait_times )
                    wait_times = todo;
                recalculate = 0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j = 0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr * smcBlockSize;
                bitmap_block += BlockAddr * (smcPagePerBlock / 32);		/* one word represents 32 pages */
            }

            // Judge the erasing cluster is continue or not
            if ((second_temp_cluster - first_temp_cluster) == 1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate = 1;
                smcErase(BlockByteAddr, logAddr_sector, logAddr_sector + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster

                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));

                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }

                first_temp_cluster = curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);

                    // Update The Bit map Info
                    for (k=0; k<(smcPagePerBlock/32); k++)
                        *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    {
                        u8	m;

                        for (m=0; m<(smcPagePerBlock/32); m++)
                        {
                            bitmap_block_data = *(bitmap_block + m);

                            for (k=0; k<smcPagePerBlock; k++)
                            {
                                if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                    bitmap_block_data = bitmap_block_data | (1<<k);
                            }
                            *(bitmap_block + m) = bitmap_block_data;
                        }
                    }


                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));
                
                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            }
        }
  #endif
        todo--;
    } /* Free cluster loop */

    DEBUG_FS("Cannot Find EOF! #4 (%d,%d)\n",curclst,todo);
#if FS_RW_DIRECT
    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
#else
    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
#endif

    sysDeleteFATLinkOnRunning=0;
    FS__fat_free(buffer);
    return -1;
}

int _FS__fat_DeleteFATLink_Back( int Idx, int Unit,int todo,int curclst)
{
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
#endif
    int lastsec,fatindex,fatsec,fatoffs,NextFatSec;
    int err;
    int fattype;
    unsigned int fatsize;
    int bytespersec;
    char *buffer;
    int RsvdSecCnt;

    unsigned int  i;
    unsigned char a;
    unsigned char b;
#if (FS_FAT_NOFAT32==0)
    unsigned char c;
    unsigned char d;
#endif 
    FS_i32 Data_start_sector;
    FS_i32 Root_start_sector;

    FS_u32* bitmap_block ; //0xFFFFFFFF means this block had erased
    FS_u32  logAddr_sector,logAddr; //magic
    FS_u32  BlockAddr,BlockByteAddr,bitmap_block_data;
    FS_u8   sector_per_clustrer,cluster_per_block,sector_offset,cluster_offset,wait_times,recalculate,j,k;
    FS_u32  first_temp_cluster,second_temp_cluster;
    FS_u8   bad_flag=0,m;
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    extern FS_u8 Rerseved_Algo_Start;
#endif

    //------------------------------------------------------------------//
    sysDeleteFATLinkOnRunning=1;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        sysDeleteFATLinkOnRunning=0;
        return -1;
    }

    if( (todo==0) || (curclst==0))
    {
        DEBUG_FS("Delete FAT-Link invalid parameter\n");
        FS__fat_free(buffer);
        return -1;
    }
    
    Root_start_sector = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    Data_start_sector = (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt*0x20)/ FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec + Root_start_sector;
    
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0)
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }
    bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    if (gInsertNAND)
    {
        recalculate=1;
        sector_per_clustrer= NAND_FAT_PARAMETER.SecPerClus;
        cluster_per_block = smcSecPerBlock / sector_per_clustrer;
    }
#endif
    lastsec = -1;

    


#if FILE_SYSTEM_DVF_TEST
    time1=OSTimeGet();
#endif
    //DEBUG_FS("Delete FAT-Link Back: (%d,%d)\n",curclst,todo);
    RsvdSecCnt=FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
    while (todo)
    {
        first_temp_cluster= curclst;
        if (fattype == 1)
        {
            fatindex = curclst + (curclst / 2);    /* FAT12 */
        }
        else if (fattype == 2)
        {
            fatindex = curclst * 4;               /* FAT32 */
        }
        else
        {
            fatindex = curclst * 2;               /* FAT16 */
        }

        if(curclst == 0)
        {
           DEBUG_FS("Cannot Find EOF! #1 (%d,%d)\n",curclst,todo);
           sysDeleteFATLinkOnRunning=0;
           FS__fat_free(buffer);
           return 1;
        }
        //fatsec means the specify sector in FAT
        //用fatindex(byte unit)算出該file再FAT table 的位置.
        fatsec = RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;

    #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==0x20)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)      //check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[i]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1)       //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[i];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr*smcBlockSize;
            bitmap_block += BlockAddr;
            *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
        }
    #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==SEC_PER_CLS)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * FS_SECTOR_SIZE + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)		//check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[i]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1) 	  //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[i];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr * smcBlockSize;

            bitmap_block += (BlockAddr * (smcPagePerBlock / 32));	/* 32 represents 32 bits of one word. One word represents 32 pages. */

            {
                u8	k;

                for (k=0; k<(smcPagePerBlock / 32); k++)
                    *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
            }
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
            //civic 070910 E
        }
    #endif

        if (fatsec != lastsec) //若是在同一個sector,則不必再重覆讀取
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        if (fattype == 1)  //FAT-12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
                if (curclst & 1)
                {
                    buffer[fatoffs] &= 0x0f;
                }
                else
                {
                    buffer[fatoffs]  = 0x00;
                }
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
             #endif
                lastsec = fatsec + 1;
                b = buffer[0];
                if (curclst & 1)
                {
                    buffer[0]  = 0x00;
                    curclst = ((a & 0xf0) >> 4) + 16 * b;
                }
                else
                {
                    buffer[0] &= 0xf0;
                    curclst = a + 256 * (b & 0x0f);

                }
                NextFatSec= RsvdSecCnt + ( (curclst + (curclst / 2)) / bytespersec);

                if(NextFatSec != fatsec + 1)
                {
                #if FS_RW_DIRECT
                    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                #else
                    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                #endif
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                }
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
                if (curclst & 1)
                {
                    buffer[fatoffs]     &= 0x0f;
                    buffer[fatoffs + 1]  = 0x00;
                    curclst = ((a & 0xf0) >> 4) + 16 * b;

                }
                else
                {
                    buffer[fatoffs]      = 0x00;
                    buffer[fatoffs + 1] &= 0xf0;
                    curclst = a + 256 * (b & 0x0f);

                }

                NextFatSec= RsvdSecCnt + ( (curclst + (curclst / 2)) / bytespersec);

                if(NextFatSec != fatsec)
                {
                #if FS_RW_DIRECT
                    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                #else
                    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                #endif
                    if (err < 0)
                    {
                        sysDeleteFATLinkOnRunning=0;
                        FS__fat_free(buffer);
                        return -1;
                    }
                }
            }
            
            //global_diskInfo.avail_clusters ++;
            
            curclst &= 0x0fff;
            if (curclst >= 0x0ff8)
            {               
                //DEBUG_FS("Delete FAT-Link Complete.\n");
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif

            }
        }
        else if (fattype == 2) //FAT32
        { /* FAT32 */
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3] & 0x0f;
            buffer[fatoffs]      = 0x00; //FAT table 填零表示該cluster 不被佔用.也就是刪除的意思.
            buffer[fatoffs + 1]  = 0x00;
            buffer[fatoffs + 2]  = 0x00;
            buffer[fatoffs + 3]  = 0x00;
            curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;

            //global_diskInfo.avail_clusters ++;

            NextFatSec= RsvdSecCnt + (curclst * 4 / bytespersec);
            if(NextFatSec != fatsec)
            {
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            if (curclst >= (FS_i32)0x0ffffff8L) //判斷是否為EOF
            {
                
            #if FILE_SYSTEM_DVF_TEST
                time2=OSTimeGet();
                //DEBUG_FS("--->Delete FAT-Link Time=%d (x50msec)\n",time2-time1);
            #endif
                //DEBUG_FS("Delete FAT-Link Complete.\n");
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif
            }
        }
        else
        {	//FAT 16
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            buffer[fatoffs]     = 0x00;
            buffer[fatoffs + 1] = 0x00;
            curclst  = a + 256 * b;
            curclst &= 0xffff;
            
            //global_diskInfo.avail_clusters ++;
            
            NextFatSec= RsvdSecCnt + (curclst * 2 / bytespersec);
            if(NextFatSec != fatsec)
            {
            #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            if (curclst >= (FS_i32)0xfff8)
            {
                
            #if FILE_SYSTEM_DVF_TEST
                time2=OSTimeGet();
                //DEBUG_FS("--->Delete FAT-Link Time=%d (x50msec)\n",time2-time1);
            #endif
                //DEBUG_FS("Delete FAT-Link Complete.\n");
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                /* Attention!! sector_per_cluster will be changed due to different media size */
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    sysDeleteFATLinkOnRunning=0;
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            #endif
            }
        }
        second_temp_cluster= curclst;

  #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
                logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcPagePerBlock;
                cluster_offset = sector_offset /sector_per_clustrer;
                wait_times =smcPagePerBlock/sector_per_clustrer;
                wait_times-=cluster_offset;
                if (todo < wait_times )
                    wait_times=todo;
                recalculate=0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j=0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr*smcBlockSize;
                bitmap_block += BlockAddr;
            }

            // Judge the erasing cluster is continue or not
            if (second_temp_cluster - first_temp_cluster==1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate=1;
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                first_temp_cluster= curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);
                    // Update The Bit map Info
                    *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    bitmap_block_data=*bitmap_block;
                    for (k=0;k<smcPagePerBlock;k++)
                    {
                        if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                            bitmap_block_data = bitmap_block_data | (1<<k);
                    }
                    *bitmap_block=bitmap_block_data;
                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j+wait_times)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            }
        }
  #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)

        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap

                logAddr = logAddr_sector * FS_SECTOR_SIZE+ SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcSecPerBlock;
                cluster_offset = sector_offset / sector_per_clustrer;

                wait_times = smcSecPerBlock / sector_per_clustrer;
                wait_times -= cluster_offset;
                if (todo < wait_times )
                    wait_times = todo;
                recalculate = 0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j = 0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr * smcBlockSize;
                bitmap_block += BlockAddr * (smcPagePerBlock / 32);		/* one word represents 32 pages */
            }

            // Judge the erasing cluster is continue or not
            if ((second_temp_cluster - first_temp_cluster) == 1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate = 1;
                smcErase(BlockByteAddr, logAddr_sector, logAddr_sector + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster

                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));

                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }

                first_temp_cluster = curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);

                    // Update The Bit map Info
                    for (k=0; k<(smcPagePerBlock/32); k++)
                        *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    {
                        u8	m;

                        for (m=0; m<(smcPagePerBlock/32); m++)
                        {
                            bitmap_block_data = *(bitmap_block + m);

                            for (k=0; k<smcPagePerBlock; k++)
                            {
                                if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                    bitmap_block_data = bitmap_block_data | (1<<k);
                            }
                            *(bitmap_block + m) = bitmap_block_data;
                        }
                    }


                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));
                
                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }
                sysDeleteFATLinkOnRunning=0;
                FS__fat_free(buffer);
                return 0;
            }
        }
  #endif
        todo--;
    } /* Free cluster loop */
    
    DEBUG_FS("Cannot Find EOF! #2 (%d,%d)\n",curclst,todo);

#if FS_RW_DIRECT
    err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
#else
    err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
#endif
    sysDeleteFATLinkOnRunning=0;
    FS__fat_free(buffer);
    return -1;
}

int _FS__fat_ScanFATLink( int Idx, int Unit,int todo,int curclst,unsigned int *pfilesize)
{
    int lastsec,fatindex,fatsec,fatoffs;
    int err;
    int fattype;
    unsigned int fatsize;
    int bytespersec;
    char *buffer;
    int RsvdSecCnt;

    unsigned int  i;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    FS_i32 Data_start_sector;
    FS_i32 Root_start_sector;

    FS_u32* bitmap_block ; //0xFFFFFFFF means this block had erased
    FS_u32  logAddr_sector,logAddr; //magic
    FS_u32  BlockAddr,BlockByteAddr,bitmap_block_data;
    FS_u8   sector_per_clustrer,cluster_per_block,sector_offset,cluster_offset,wait_times,recalculate,j,k;
    FS_u32  byte_per_clustrer;
    FS_u32  first_temp_cluster,second_temp_cluster;
    FS_u8   bad_flag=0,m;
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    extern FS_u8 Rerseved_Algo_Start;  //ToAllen
#endif

    //------------------------------------------------------------------//
    *pfilesize=0;
    if(curclst==0)
        return -1;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0)
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }
    
    Root_start_sector = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    Data_start_sector = (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt*0x20)/ FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec + Root_start_sector;
    
    
    bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    byte_per_clustrer = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec;

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    if (gInsertNAND)
    {
        recalculate=1;
        sector_per_clustrer= NAND_FAT_PARAMETER.SecPerClus;
        byte_per_clustrer= sector_per_clustrer * bytespersec;
        cluster_per_block = smcSecPerBlock / sector_per_clustrer;
    }
#endif
    lastsec = -1;

    RsvdSecCnt=FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
    while (todo)
    {
        first_temp_cluster= curclst;
        if (fattype == 1)
        {
            fatindex = curclst + (curclst / 2);    /* FAT12 */
        }
        else if (fattype == 2)
        {
            fatindex = curclst * 4;               /* FAT32 */
        }
        else
        {
            fatindex = curclst * 2;               /* FAT16 */
        }

        
        //fatsec means the specify sector in FAT
        //用fatindex(byte unit)算出該file再FAT table 的位置.
        fatsec = RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;

    #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==0x20)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)      //check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[m]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1)       //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[m];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr*smcBlockSize;
            bitmap_block += BlockAddr;
            *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
        }
    #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
        if (gInsertNAND==1 && NAND_FAT_PARAMETER.SecPerClus==SEC_PER_CLS)
        {
            /* Add the reserved size */
            bitmap_block = (FS_u32* )smcBitMap;
            logAddr_sector = (curclst-2)*FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
            logAddr = logAddr_sector * FS_SECTOR_SIZE + SMC_FAT_START_ADDR;

            //pageOffset = logAddr % smcBlockSize;		// offset of page in a block
            // Update The Bit map Info
            if (Rerseved_Algo_Start)
            {
                for (m=0;m<SMC_RESERVED_BLOCK;m++)		//check whether this block is a bad one
                {
                    if (SMC_BBM.Bad_Block_addr[i]==logAddr)
                    {
                        bad_flag=1;
                        break;
                    }
                }

                if (bad_flag==1) 	  //re-calculate the logAddr
                {
                    logAddr = SMC_BBM.New_Block_addr[i];
                }
            }
            BlockAddr = logAddr / smcBlockSize;  // offset of block
            BlockByteAddr = BlockAddr * smcBlockSize;

            bitmap_block += (BlockAddr * (smcPagePerBlock / 32));	/* 32 represents 32 bits of one word. One word represents 32 pages. */

            {
                u8	k;

                for (k=0; k<(smcPagePerBlock / 32); k++)
                    *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
            }
            if (smcBlockErase(logAddr)==0)
                DEBUG_FS("smcBlockErase error at %x \n",logAddr);
            //civic 070910 E
        }
    #endif

        if (fatsec != lastsec) //若是在同一個sector,則不必再重覆讀取
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {	//Second FAT
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        
        if (fattype == 1)  //FAT-12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
             #if FS_RW_DIRECT
                err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
             #else
                err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
             #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
             #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
             #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
             #endif
                lastsec = fatsec + 1;
                b = buffer[0];
                if (curclst & 1)
                {
                    curclst = ((a & 0xf0) >> 4) + 16 * b;
                }
                else
                {
                    curclst = a + 256 * (b & 0x0f);

                }
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
                if (curclst & 1)
                {
                    curclst = ((a & 0xf0) >> 4) + 16 * b;
                }
                else
                {
                    curclst = a + 256 * (b & 0x0f);
                }
            }
            
            
            curclst &= 0x0fff;
            *pfilesize += byte_per_clustrer;
            if (curclst >= 0x0ff8)
            {
                
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                FS__fat_free(buffer);
                return 0;
            #endif

            }

            if(curclst == 0)
            {
               DEBUG_FS("Cannot Find EOF! Mark EOF\n");
               FS__fat_free(buffer);
               return -1;
            }
        }
        else if (fattype == 2) //FAT32
        { /* FAT32 */
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3] & 0x0f;
           
            curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;

            *pfilesize += byte_per_clustrer;
            if (curclst >= (FS_i32)0x0ffffff8L) //判斷是否為EOF
            {       
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                FS__fat_free(buffer);
                return 0;
            #endif
            }

            if(curclst == 0)
            {
               DEBUG_FS("Cannot Find EOF! Mark EOF\n");
               buffer[fatoffs]=0xff;
               buffer[fatoffs + 1]=0xff;
               buffer[fatoffs + 2]=0xff;
               buffer[fatoffs + 3]=0xff;
               #if FS_RW_DIRECT
                  err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
               #else
                  err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
               #endif
               FS__fat_free(buffer);

               if(err<0)
                  return -1;
               else
                  return 1;
            }
            
        }
        else
        {	//FAT 16
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];

            curclst  = a + 256 * b;
            curclst &= 0xffff;

            *pfilesize += byte_per_clustrer;
            if (curclst >= (FS_i32)0xfff8)
            {
            #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
                /* Attention!! sector_per_cluster will be changed due to different media size */
                if (!(gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS))		 // for NAND delete function
                {
                    FS__fat_free(buffer);
                    return 0;
                }
            #else
                FS__fat_free(buffer);
                return 0;
            #endif
            }

            if(curclst == 0)
            {
               DEBUG_FS("Cannot Find EOF! Mark EOF\n");
               buffer[fatoffs]=0xff;
               buffer[fatoffs + 1]=0xff;
               #if FS_RW_DIRECT
                  err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
               #else
                  err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
               #endif
               FS__fat_free(buffer);

               if(err<0)
                  return -1;
               else
                  return 1;
            }
            
        }
        second_temp_cluster= curclst;

  #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap
                logAddr = logAddr_sector * smcPageSize + SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcPagePerBlock;
                cluster_offset = sector_offset /sector_per_clustrer;
                wait_times =smcPagePerBlock/sector_per_clustrer;
                wait_times-=cluster_offset;
                if (todo < wait_times )
                    wait_times=todo;
                recalculate=0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j=0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr*smcBlockSize;
                bitmap_block += BlockAddr;
            }

            // Judge the erasing cluster is continue or not
            if (second_temp_cluster - first_temp_cluster==1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate=1;
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                first_temp_cluster= curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);
                    // Update The Bit map Info
                    *bitmap_block=0xFFFFFFFF;		//  All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    bitmap_block_data=*bitmap_block;
                    for (k=0;k<smcPagePerBlock;k++)
                    {
                        if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                            bitmap_block_data = bitmap_block_data | (1<<k);
                    }
                    *bitmap_block=bitmap_block_data;
                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block_data=*bitmap_block;
                for (k=0;k<smcPagePerBlock;k++)
                {
                    if (k>=sector_offset && k<(sector_offset+ (j+wait_times)*sector_per_clustrer))
                        bitmap_block_data = bitmap_block_data | (1<<k);
                }
                *bitmap_block=bitmap_block_data;
                FS__fat_free(buffer);
                return 0;
            }
        }
  #elif (FLASH_OPTION == FLASH_NAND_9002_ADV)

        if (gInsertNAND==1 && sector_per_clustrer!=SEC_PER_CLS)
        {
            // calculate cluster offset of NAND block
            if (recalculate)
            {
                logAddr_sector = (first_temp_cluster-2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus + Data_start_sector ;	 // Remap

                logAddr = logAddr_sector * FS_SECTOR_SIZE+ SMC_FAT_START_ADDR;

                sector_offset = logAddr_sector % smcSecPerBlock;
                cluster_offset = sector_offset / sector_per_clustrer;

                wait_times = smcSecPerBlock / sector_per_clustrer;
                wait_times -= cluster_offset;
                if (todo < wait_times )
                    wait_times = todo;
                recalculate = 0; // Re-calculate while un-continuous or last clst reached or one block align reach
                j = 0;
                bitmap_block = (FS_u32* )smcBitMap;
                BlockAddr = logAddr / smcBlockSize;  // offset of block
                BlockByteAddr = BlockAddr * smcBlockSize;
                bitmap_block += BlockAddr * (smcPagePerBlock / 32);		/* one word represents 32 pages */
            }

            // Judge the erasing cluster is continue or not
            if ((second_temp_cluster - first_temp_cluster) == 1)
            {
                wait_times--;
                j++;
            }
            else
            {  // un-continunes
                recalculate = 1;
                smcErase(BlockByteAddr, logAddr_sector, logAddr_sector + (j+1)*sector_per_clustrer); // first_temp_cluster+j means the last continuous cluster

                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));

                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }

                first_temp_cluster = curclst;
            }

            if (wait_times==0 && recalculate!=1)   // match the block boarding address
            {
                if (j==4)
                {  // erasing a block
                    smcBlockErase(logAddr);

                    // Update The Bit map Info
                    for (k=0; k<(smcPagePerBlock/32); k++)
                        *(bitmap_block + k) = 0xFFFFFFFF;		//	All the page in the block erase
                }
                else
                {  // continunes but it's a part of block
                    smcErase(BlockByteAddr,sector_offset,sector_offset + (j)*sector_per_clustrer);
                    {
                        u8	m;

                        for (m=0; m<(smcPagePerBlock/32); m++)
                        {
                            bitmap_block_data = *(bitmap_block + m);

                            for (k=0; k<smcPagePerBlock; k++)
                            {
                                if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                    bitmap_block_data = bitmap_block_data | (1<<k);
                            }
                            *(bitmap_block + m) = bitmap_block_data;
                        }
                    }


                }
                first_temp_cluster= curclst;
                recalculate=1;
            }

            if (todo==1) // Last clst reached
            {
                smcErase(BlockByteAddr,sector_offset,sector_offset + (j+wait_times)*sector_per_clustrer);
                bitmap_block += ((BlockByteAddr/smcBlockSize) * (smcPagePerBlock/32));
                
                {
                    u8	m;

                    for (m=0; m<(smcPagePerBlock/32); m++)
                    {
                        bitmap_block_data = *(bitmap_block + m);

                        for (k=0; k<smcPagePerBlock; k++)
                        {
                            if (k>=sector_offset && k<(sector_offset+ (j)*sector_per_clustrer))
                                bitmap_block_data = bitmap_block_data | (1<<k);
                        }
                        *(bitmap_block + m) = bitmap_block_data;
                    }
                }
                FS__fat_free(buffer);
                return 0;
            }
        }
  #endif
        todo--;
    } /* Free cluster loop */

    DEBUG_FS("Cannot Find EOF\n");
    FS__fat_free(buffer);
    return -1;
}
/*********************************************************************
*
*             FS__fat_make_realname
*
  Description:
  FS internal function. Convert a given name to the format, which is
  used in the FAT directory.

  Parameters:
  pOrgName    - Pointer to name to be translated
  pEntryName  - Pointer to a buffer for storing the real name used
                in a directory.

  Return value:
  None.
*/

void FS__fat_make_realname(char *pEntryName, const char *pOrgName)
{
    FS_FARCHARPTR ext;
    FS_FARCHARPTR s;
    int i;

    s = (FS_FARCHARPTR)pOrgName;
    ext = (FS_FARCHARPTR) FS__CLIB_strchr(s, '.');
    if (!ext)
    {
        ext = &s[FS__CLIB_strlen(s)];
    }
    i=0;
    while (1)
    {
        if (s >= ext)
        {
            break;  /* '.' reached */
        }
        if (i >= 8)
        {
            break;  /* If there is no '.', this is the end of the name */
        }
        if (*s == (char)0xe5)
        {
            pEntryName[i] = 0x05;
        }
        else
        {
            pEntryName[i] = (char)FS__CLIB_toupper(*s); //將小寫字母換成大寫
        }
        i++;
        s++;
    }
    while (i < 8) //不足八個字元以 space 補足之
    {
        /* Fill name with spaces*/
        pEntryName[i] = ' ';
        i++;
    }
    if (*s == '.')
    {
        s++;
    }
    while (i < 11)
    {
        if (*s != 0)
        {
            if (*s == (char)0xe5)
            {
                pEntryName[i] = 0x05;
            }
            else
            {
                pEntryName[i] = (char)FS__CLIB_toupper(*s);
            }
            s++;
        }
        else
        {
            pEntryName[i] = ' ';
        }
        i++;
    }
    pEntryName[11]=0;
}


/*********************************************************************
*
*             FS__fat_find_dir
*
  Description:
  FS internal function. Find the directory with name pDirName in directory
  DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pDirName    - Directory name; if zero, return the root directory.
  DirStart    - 1st cluster of the directory.
  DirSize     - Sector (not cluster) size of the directory.

  Return value:
  >0          - Directory found. Value is the first cluster of the file.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_find_dir(int Idx, FS_u32 Unit, char *pDirName, FS_u32 DirStart,
                           FS_u32 DirSize)
{
    FS__fat_dentry_type *s;
    FS_u32 dstart;
    FS_u32 i;
    FS_u32 dsec;
    FS_u32 fatsize;
    int len;
    int err;
    int c;
    char *buffer;

    if (pDirName == 0)
    {
        /* Return root directory */
        if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16)
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
            dstart  = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        }
        else
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
            dstart  = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize
                      + (FS__FAT_aBPBUnit[Idx][Unit].RootClus - 2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        }
    }
    else
    {
        /* Find directory */
        buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
        if (!buffer)
        {
            DEBUG_FS("FS__fat_malloc is Fail\n");
            return 0;
        }
        len = FS__CLIB_strlen(pDirName);
        if (len > 11)
        {
            len = 11;
        }
        /* Read directory */
        for (i = 0; i < DirSize; i++) //Lucian: Scan all directory. sector by sector
        {
            dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
            if (dsec == 0)
            {
                FS__fat_free(buffer);
                return 0;
            }
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        #endif
            if (err < 0)
            {
                FS__fat_free(buffer);
                return 0;
            }
            s = (FS__fat_dentry_type*)buffer;
            while (1)
            {
                if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
                {
                    break;  /* End of sector reached */
                }
                c = FS__CLIB_strncmp((char*)s->data, pDirName, len);
                if (c == 0)
                { /* Name does match */
                    if (s->data[11] & FS_FAT_ATTR_DIRECTORY)
                    {
                        break;  /* Entry found */
                    }
                }
                s++;
            }
            if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                /* Entry found. Return number of 1st block of the directory */
                dstart  = (FS_u32)s->data[26];
                dstart += (FS_u32)0x100UL * s->data[27];
                dstart += (FS_u32)0x10000UL * s->data[20];
                dstart += (FS_u32)0x1000000UL * s->data[21];
                FS__fat_free(buffer);
                return dstart;
            }
        }
        dstart = 0;
        FS__fat_free(buffer);
    }
    return dstart;
}


/*********************************************************************
*
*             FS__fat_dir_realsec
*
  Description:
  FS internal function. Translate a directory relative sector number
  to a real sector number on the media.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory. This is zero to address
                the root directory.
  DirSec      - Sector in the directory.

  Return value:
  >0          - Directory found. Value is the sector number on the media.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_dir_realsec(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 DirSec)
{
    FS_u32 rootdir;
    FS_u32 rsec;
    FS_u32 dclust;
    FS_u32 fatsize;
    int fattype;
    int lexp;
    unsigned char secperclus;

    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    lexp = (0 == DirStart);
    lexp = lexp && (fattype != 2);
    if (lexp)
    {
        /* Sector in FAT12/FAT16 root directory */
        //rootdir:0x18
        rootdir = FS__fat_find_dir(Idx, Unit, 0, 0, 0);
        rsec = rootdir + DirSec;
    }
    else
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
        if (fatsize == 0)
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
        }
        secperclus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        dclust = DirSec / secperclus;
        if (0 == DirStart)
        {
            /* FAT32 root directory */
            rsec = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
        }
        else
        {
            rsec = FS__fat_diskclust(Idx, Unit, DirStart, dclust);
            if (rsec == 0)
            {
                DEBUG_FS("FS__fat_dir_realsec Fail\n");
                return 0;
            }
        }
        rsec -= 2;  //cluster start from 2nd
        rsec *= secperclus;
        rsec += FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        rsec += ((FS_u32)((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) * FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE; //at FAT32,FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt=0
        rsec += (DirSec % secperclus);
    }
    return rsec;
}


/*********************************************************************
*
*             FS__fat_dirsize
*
  Description:
  FS internal function. Return the sector size of the directory
  starting at DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory. This is zero to address
                the root directory.

  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_dir_size(int Idx, FS_u32 Unit, FS_u32 DirStart)
{
    FS_u32 dsize;
    FS_i32 value;

    if (DirStart == 0)
    {
        /* For FAT12/FAT16 root directory, the size can be found in BPB */
        dsize = ((FS_u32)((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt)
                 * FS_FAT_DENTRY_SIZE) / ((FS_u32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec);
        
        if (dsize == 0)
        {
            /* Size in BPB is 0, so it is a FAT32 (FAT32 does not have a real root dir) */
            value = FS__fat_FAT_find_eof(Idx, Unit, FS__FAT_aBPBUnit[Idx][Unit].RootClus, &dsize);
            if (value < 0)
            {
                dsize = 0;
            }
            else
            {
                dsize *= FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
            }
        }
    }
    else
    {
        /* Calc size of a sub-dir */
        value = FS__fat_FAT_find_eof(Idx, Unit, DirStart, &dsize);
        if (value < 0)
        {
            dsize = 0;
        }
        else
        {
            dsize *= FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        }
    }
    return dsize;
}


/*********************************************************************
*
*             FS__fat_findpath
*
  Description:
  FS internal function. Return start cluster and size of the directory
  of the file name in pFileName.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  pFullName   - Fully qualified file name w/o device name.
  pFileName   - Pointer to a pointer, which is modified to point to the
                file name part of pFullName.
                //paste civic
                	e.g. "0:\\dir1\\dir2",
		     "0:\\dir1\\dir2\\test.txt"
  pUnit       - Pointer to an FS_u32 for returning the unit number.
  pDirStart   - Pointer to an FS_u32 for returning the start cluster of
                the directory.

  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

FS_u32 FS__fat_findpath(int Idx, const char *pFullName, FS_FARCHARPTR *pFileName,
                        FS_u32 *pUnit, FS_u32 *pDirStart)
{
    FS_u32 dsize;
    FS_i32 i;
    FS_i32 j;
    FS_FARCHARPTR dname_start;
    FS_FARCHARPTR dname_stop;
    FS_FARCHARPTR chprt;
    int x;
    char dname[12];
    char realname[12];

    /* Find correct unit (unit:name) */
    *pFileName = (FS_FARCHARPTR)FS__CLIB_strchr(pFullName, ':');
    if (*pFileName)
    {
        /* Scan for unit number */
        *pUnit = FS__CLIB_atoi(pFullName);
        (*pFileName)++;
    }
    else
    {
        /* Use 1st unit as default */
        *pUnit = 0;
        *pFileName = (FS_FARCHARPTR) pFullName;
    }
    /* Check volume */
    x = FS__fat_checkunit(Idx, *pUnit);		//Read BPB(MBR)

    if (x!=1)
    {
        DEBUG_FS("--->FS__fat_checkunit Error!\n");
        return 0;
    }
    /* Setup pDirStart/dsize for root directory */
    *pDirStart = 0;
    dsize      = FS__fat_dir_size(Idx, *pUnit, 0);  //Lucian: 算出root directory 的size. (sector unit)

    //Lucian: 從Root directory 開始找到 destination 
    /* Find correct directory */
    do
    {
        dname_start = (FS_FARCHARPTR)FS__CLIB_strchr(*pFileName, '\\');
        if (dname_start)
        {
            dname_start++;
            *pFileName = dname_start;
            dname_stop = (FS_FARCHARPTR)FS__CLIB_strchr(dname_start, '\\');
        }
        else
        {
            dname_stop = 0;
        }
        if (dname_stop)
        {
            i = dname_stop-dname_start;
            if (i >= 12)
            {
                j = 0;
                for (chprt = dname_start; chprt < dname_stop; chprt++)
                {
                    if (*chprt == '.')
                    {
                        i--;
                    }
                    else if (j < 12)
                    {
                        realname[j] = *chprt;
                        j++;
                    }
                }
                if (i >= 12)
                {
                    return 0;
                }
            }
            else
            {
                FS__CLIB_strncpy(realname, dname_start, i);
            }
            realname[i] = 0;
            FS__fat_make_realname(dname, realname); //將realname-->dname,做大小寫轉換.
            *pDirStart =  FS__fat_find_dir(Idx, *pUnit, dname, *pDirStart, dsize);
            if (*pDirStart)
            {
                dsize  =  FS__fat_dir_size(Idx, *pUnit, *pDirStart);
            }
            else
            {
                dsize = 0;    /* Directory NOT found */
            }
        }
    }
    while (dname_start);

    return dsize;
}


/*********************************************************************
*
*             Global functions section 2
*
**********************************************************************

  These are real global functions, which are used by the API Layer
  of the file system.

*/

int FS_fat_rename(char *pOldFilePath,char *pNewFileName,int index)
{
    FS_FARCHARPTR fname;
    FS_u32 unit;
    FS_u32 dstart;
    FS_u32 dsize;
    char realname[12];
    char newname[12];
    
    FS__fat_dentry_type *s;
    unsigned int SecPerClus;
    char *buffer;
    int len;
    int i;
    FS_u32 dsec;
    int err;
    int offset;
    int c;
    //==========================//

    dsize = FS__fat_findpath( index, pOldFilePath, &fname, &unit, &dstart);
    if (dsize == 0)
    {
        return -1;  /* Directory not found */
    }
    FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之 */
    FS__fat_make_realname(newname, pNewFileName);

    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[index][unit].SecPerClus;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    len = FS__CLIB_strlen(realname);
    if (len > 11)
    {
        len = 11;
    }

    /* Read directory */
    for (i = 0; i < dsize; i++)   
    {
        if( (i % SecPerClus)==0) //Lucian: optimize here..
           dsec = FS__fat_dir_realsec(index, unit, dstart, i);
        else
           dsec +=1;
        
        if (dsec == 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
     #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
     #else
        err = FS__lb_read(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
     #endif
        if (err < 0)        
        {
            FS__fat_free(buffer);
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }
            c = FS__CLIB_strncmp((char*)s->data, realname, len);
            if (c == 0)
            {  /* Name does match */
                if (s->data[11] & (FS_FAT_ATTR_ARCHIVE | FS_FAT_ATTR_DIRECTORY))
                {
                    break;  /* Entry found */
                }
            }
            offset ++;
            s++;
        }
        
        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            memcpy(s->data,newname,11);

        #if FS_RW_DIRECT
            err = FS__lb_write_Direct(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
        #else
            err = FS__lb_write(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
        #endif
        
            FS__fat_free(buffer);
            return 1 ; 
        }
    }
    FS__fat_free(buffer);
    return -1;
}


/*********************************************************************
*
*             FS__fat_fopen
*
  Description:
  FS internal function. Open an existing file or create a new one.

  Parameters:
  pFileName   - File name.
  pMode       - Mode for opening the file.
  pFile       - Pointer to an FS_FILE data structure.

  Return value:
  ==0         - Unable to open the file.
  !=0         - Address of the FS_FILE data structure.
  //paste by civic
  pFilename   - Address of a pointer, which is modified to point to
                the file name part of pFullName.
		Output "unit:\\directory\\directory\\file-name"
		e.g. "0:\\dir1\\dir2",
		     "0:\\dir1\\dir2\\test.txt"

*/
// Give the pFileName and pMode then fill the pFile Structure

FS_i32 FS__fat_fopen(const char *pFileName, const char *pMode, FS_FILE *pFile)
{
    FS_u32 unit;
    FS_u32 dstart;
    FS_u32 dsize;
    FS_i32 i;
    FS_FARCHARPTR fname;
    FS__fat_dentry_type s;
    char realname[12];
    int lexp_a;
    int lexp_b;
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
#endif

    if (!pFile)
    {
        DEBUG_FS("--->Not a valid pointer to an FS_FILE structure!\n");
        return 0;  // Not a valid pointer to an FS_FILE structure
    }
    //FS_FOpen was filled by FS_FOpen function
    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    dsize = FS__fat_findpath(pFile->dev_index, pFileName, &fname, &unit, &dstart);
    if (dsize == 0)
    {
        DEBUG_FS("--->FS__fat_findpath error!\n");
        return 0;  /* Directory not found */
    }
    // No execution because never implement FS_CMD_INC_BUSYCNT ioctl command
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  /* Turn on busy signal */
    FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之 */
    /* FileSize = 0 */
    // Since offset 28--31 is the file size in FDB
    s.data[28] = 0x00;
    s.data[29] = 0x00;
    s.data[30] = 0x00;
    s.data[31] = 0x00;

#if 1
    /*---------------- Delete file Path--------------*/
    lexp_b = (FS__CLIB_strcmp(pMode, "del") == 0);    /* Delete file request */
    if (lexp_b)
    {   //delete file//
    #if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
    #endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect,1);
        if (i <= 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);   
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    #if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        //DEBUG_FS("--->Delete File Time=%d (x50msec)\n",time2-time1);
    #endif
        return i;
    }
    
    //-----------------------Create File Path--------------------//
    //dstart is the start cluster of directory, dsize is the size of directory
    //Lucian: 找到該目錄下該檔案的 start cluster.
    
    lexp_b = (FS__CLIB_strcmp(pMode, "w-") == 0);
    if(lexp_b)
       i=-1;
    else
    {
    #if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
    #endif
       i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize);
    
    #if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        //DEBUG_FS("--->_FS_fat_find_file = %d (x50msec)\n",time2-time1);
    #endif
    }
#else
    //Lucian: 找到該目錄下該檔案的 start cluster.
    i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize);
    
    /* Delete file */
    lexp_b = (FS__CLIB_strcmp(pMode, "del") == 0);    /* Delete file request */
    lexp_a = lexp_b && (i >= 0);                      /* File does exist,and want to delete */
    if (lexp_a)
    {   //delete file//
    #if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
    #endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect,1);
        if (i <= 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);   
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    #if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        DEBUG_FS("Delete File Time=%d (x50msec)\n",time2-time1);
    #endif
        return i;
    }
    else if (lexp_b)
    {   //delete file but file don't exist.
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        pFile->error = -1;
        return 0;
    }
#endif
    /* Check read only */
    lexp_a = ((i >= 0) && ((s.data[11] & FS_FAT_ATTR_READ_ONLY) != 0)) &&
             ((pFile->mode_w) || (pFile->mode_a) || (pFile->mode_c));
    if (lexp_a)
    {
        /* Files is RO and we try to create, write or append. Not allow */
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        return 0;
    }
    lexp_a = ( i>= 0) && (!pFile->mode_a) && 
             (((pFile->mode_w) && (!pFile->mode_r)) || ((pFile->mode_w) && (pFile->mode_c) && (pFile->mode_r)) );
    if (lexp_a)
    {
        /* Delete old file */
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect,1);
        /* FileSize = 0 */
        s.data[28] = 0x00;
        s.data[29] = 0x00;
        s.data[30] = 0x00;
        s.data[31] = 0x00;
        i=-1;
    }
    
    if ((!pFile->mode_c) && (i < 0))
    {
        /* File does not exist and we don't create */
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        return 0;
    }
    else if ((pFile->mode_c) && (i < 0))
    {
        /*File does not exist and Create new file */
        //DEBUG_FS("EmptyFileEntSect=(%d,%d)\n",FileEntSect,FileEntSect_offset);
    #if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
    #endif
        i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize,&FileEntSect,&FileEntSect_offset);
    #if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        DEBUG_FS("--->_FS_fat_create_file = %d (x50msec)\n",time2-time1);
    #endif

		switch(i)
		{
			case FS_FILE_NAME_REPEAT_ERR:
				return -1;
			case -2:
				DEBUG_FS("File Entry Full. Create new one.\n");
				if((i = _FS_fat_IncDir_One(pFile->dev_index, unit, dstart, &dsize)) >= 0)
					i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize,&FileEntSect,&FileEntSect_offset);
			default:
				if (i < 0)
	            {
	                FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
	                return 0;
	            }
				break;
				
		}
        pFile->EOFClust   = dcfLasfEofCluster;
    }
    else
    {
        pFile->EOFClust   = -1;     //  Append file
        //DEBUG_FS("Append:EmptyFileEntSect=(%d,%d)\n",FileEntSect,FileEntSect_offset);

    }

    pFile->fileid_lo  = unit;   // unit whre file is located.
    pFile->fileid_hi  = i;      // 1st clust of file location.(Low byte)
    pFile->fileid_ex  = dstart; // dstart is the start cluster of directory
    pFile->FileEntrySect=FileEntSect;
    pFile->CurClust   = 0;      // 未知
    pFile->error      = 0;      // 未知
    /* FileSize */
    pFile->size       = (FS_u32)s.data[28];   
    pFile->size      += (FS_u32)0x100UL * s.data[29];
    pFile->size      += (FS_u32)0x10000UL * s.data[30];
    pFile->size      += (FS_u32)0x1000000UL * s.data[31];
    if (pFile->mode_a)
    {
        pFile->filepos   = pFile->size; //從檔案尾開始接
    }
    else
    {
        pFile->filepos   = 0;//filepos: 是從檔案頭開始算
    }
    pFile->inuse     = 1;
    return 1;
}

void FS__fat_ResetFileEntrySect(int sect,int offset)
{
   FileEntSect=sect;
   FileEntSect_offset=offset;
}

void FS__fat_GetFileEntrySect(int *psect,int *poffset)
{
   *psect=FileEntSect;
   *poffset=FileEntSect_offset;
}

/*********************************************************************
*
*             FS__fat_FindLastFileCluster
*
  Description:
     於FAT Table 找尋最後寫入的檔案所屬的Cluster,

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  
  Return value:
  >=0         - Number of file first cluster.
  <0          - An error has occured.
*********************************************************************/

int FS__fat_FindLastFileCluster(int Idx, char *pFileName)
{
    FS_u32 unit;
    FS_u32 dstart;
    FS_u32 dsize;
    FS_FARCHARPTR fname;
    FS__fat_dentry_type s;
    char realname[12];
    FS_u32 dsec;
    
    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    dsize = FS__fat_findpath(Idx, pFileName, &fname, &unit, &dstart);
    if (dsize == 0)
    {
        DEBUG_FS("--->FS__fat_findpath error!\n");
        return -1;  /* Directory not found */
    }
    FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之 */
    
    dsec = _FS_fat_find_file(Idx, unit, realname, &s, dstart, dsize);

    return dsec;
    
}
