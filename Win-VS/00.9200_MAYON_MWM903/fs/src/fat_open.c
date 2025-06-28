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

#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_fat.h"
#include "fs_clib.h"

#include "general.h"
#include "Rtcapi.h"
#include "smcapi.h"
#include "sysapi.h"
#include "siuapi.h"
#include "i2capi.h"
#include "dcfapi.h"


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

/*********************************************************************
*
*             Extern Global Variable
*
**********************************************************************/
extern FS_DISKFREE_T global_diskInfo;
extern u8 siuOpMode;
extern u8 sysDeleteFATLinkOnRunning;
/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/

extern s32 sysbackLowSetEvt(s8 cause, s32 param1,s32 param2,s32 param3,s32 param4);



/*********************************************************************
*
*             Local functions body
*
**********************************************************************
*/
#if FS_BIT_WISE_OPERATION
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

s32 _FS_fat_find_file(int Idx, u32 Unit, const char *pFileName, FS__fat_dentry_type *pDirEntry, u32 DirStart, u32 DirSize, u32 *pFileCluster)
{
	FS__FAT_BPB *pBPBUnit; 
    FS__fat_dentry_type *s;
    u32 i, dsec;
    int len, offset, err, c;
    char *buffer;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    dsec = 0;
    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
    {
        len = 11;
    }

	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    // Read directory
    for (i = 0; i < DirSize; i++)
    {
        if( (i & (pBPBUnit->SecPerClus - 1)) == 0) // Lucian: optimize here..
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, i, &dsec);
        else
            dsec += 1;
        if (err < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset = 0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }
            c = FS__CLIB_strncmp((char*)s->data, pFileName, len);
            if (c == 0)
            {
                // Name does match
                if (s->data[11] & FS_FAT_ATTR_ARCHIVE)
                {
                    break;  // Entry found
                }
            }
            offset ++;
            s++;
        }

        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            // Entry found. Return number of 1st block of the file
            if (pDirEntry)
            {
                FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
            }
            *pFileCluster = (s->data[26]) +
                            (s->data[27] << 8) +
                            (s->data[20] << 16) +
                            (s->data[21] << 24);	// set 該檔案的 first cluster
            FS__fat_free(buffer);
            return 1;
        }
    }
    FS__fat_free(buffer);
    return -1;
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

int _FS_fat_create_file(int Idx, u32 Unit, const char *pFileName, u32 DirStart, u32 DirSize, u32 *pEntSect, u32 *pEntSectOffset, u32 *pFileCluster)
{
	FS__FAT_BPB *pBPBUnit; 
    FS__fat_dentry_type *s;
    u32 sect, offset, dsec = 0;
    u32 cluster, Size;
    int len, err;
    u16 val;
    char *buffer;
    RTC_DATE_TIME localTime;
    int scan, first, SecPerClus;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
        len = 11;

    *pEntSect = 0;
    *pEntSectOffset = 0;
    //-------------------- Read directory ----------------------
    //	目前做法是: Scan all directory (sector by sector). 從cluster頭到尾,直到找到空的file entry.
    //    新的作法:

    SecPerClus = (u32) FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    scan = 0;
    first = 1;
    offset = 0;
    sect = *pEntSect;

    while(1)
    {
        if(sect >= DirSize)
        {
            scan ++;
            DEBUG_FS("File Entry SCAN = %d\n", scan);
            if (scan > 1)
            {
                DEBUG_FS("File Entry is full\n");
                FS__fat_free(buffer);
                return -2;	// Directory is full
            }
            sect=0;
        }

        if((first == 1) || ((sect % SecPerClus) == 0))
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, sect, &dsec);
        else
            dsec +=1;
        first = 0;
        if (err < 0)
        {
            DEBUG_RED("DirStart: %#x, sect: %d, DirSize: %d\n", DirStart, sect, DirSize);
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }

        s = (FS__fat_dentry_type*)buffer;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }
            if (s->data[0] == 0x00)
            {
                break;  // Empty entry found
            }
            if(strncmp((const char *)s->data, pFileName, FS_V_FAT_ENTEY_SHORT_NAME) == 0)
            {
            	ERRD(FS_FILE_NAME_REPEAT_ERR);
            	FS__fat_free(buffer);
            	return FS_FILE_NAME_REPEAT_ERR;
            }
            s++;
            offset ++;
        }

        if (s < (FS__fat_dentry_type* )(buffer + FS_FAT_SEC_SIZE))
        {
            *pEntSect = sect;
            *pEntSectOffset = offset & ((FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec >> 5) - 1);
            DEBUG_FS2("NextFreeCluster = %d\n", FS__pDevInfo[Idx].FSInfo.NextFreeCluster);

            // Free entry found. Make entry and return 1st block of the file
            FS__CLIB_strncpy((char*)s->data, pFileName, len);
            s->data[11] = FS_FAT_ATTR_ARCHIVE;
            // Alloc block in FAT
            if((err = FSFATNewEntry(Idx, Unit, &cluster, 1, &Size, FS_E_CLUST_CLEAN_OFF)) < 0)
            {
            	ERRD(FS_FAT_CLUT_ALLOC_ERR);
            	FS__fat_free(buffer);
            	return err;
            }
            //----紀錄 create time , modified time---//
            if (cluster > 0)
            {
                s->data[12] = 0x00;	// Res
                s->data[13] = 0x00;	// CrtTimeTenth (optional, not supported)
                s->data[22] = 0x00;	// CrtTime (optional, not supported)
                s->data[23] = 0x00;
                s->data[24] = 0x00;	// CrtDate (optional, not supported)
                s->data[25] = 0x00;

                RTC_Get_Time(&localTime);
                val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec>>1) & 0x1F);
                s->data[14] = (u8)(val & 0xff);	// WrtTime
                s->data[15] = (u8)(val >> 8);
                // + 2000 -1980 =20
                val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);
                s->data[16] = (val & 0xff);	// WrtDate
                s->data[17] = (val >> 8);
                s->data[18] = (val & 0xff);	// LstAccDate (optional, not supported)
                s->data[19] = (val >> 8);
                s->data[26] = (cluster & 0xff);	// FstClusLo / FstClusHi
                s->data[27] = ((cluster >> 8) & 0xff);
                s->data[20] = ((cluster >> 16) & 0xff);
                s->data[21] = ((cluster >> 24) & 0xff);
                s->data[28] = 0x00;	// FileSize
                s->data[29] = 0x00;
                s->data[30] = 0x00;
                s->data[31] = 0x00;

                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    return err;
                }
            }
            else
                ERRD(FS_FAT_CLUT_LINK_ERR);
            //======//
            FS__fat_free(buffer);
            *pFileCluster = cluster;
            return 1;	//???
        }
        sect++;
    }
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

int FS__fat_DeleteFileOrDir(int Idx, u32 Unit,  const char *pName, u32 DirStart, u32 DirSize, u16 FileEntrySect, char RmFile)
{
	FS__FAT_BPB *pBPBUnit;
    FS__fat_dentry_type *s;
    u32 offset, scan, first, value, sect, filesize, dsec = 0, curclst;
    s32 len, todo;
    char *buffer;
    int result;
    //--------//
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    len = FS__CLIB_strlen(pName);
    if (len > 11)
    {
        len = 11;
    }
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    //
    DEBUG_FS2("Estimate File Entry Secter: %d\n", FileEntrySect);
    if(FileEntrySect >= DirSize)
    {
        ERRD(FS_FAT_SEC_CAL_ERR);
        DEBUG_FS("FileEntrySect: %#x, DirSize: %#x\n", FileEntrySect, DirSize);
        FileEntrySect = 0;
    }

    scan = 0;
    first = 1;	// the switch to read data in first round
    sect = FileEntrySect;
    // Read directory
    while (1) //DirSize: sector unit.
    {
        curclst = 0;
        if(sect >= DirSize)
        {
            scan ++;
            DEBUG_FS("File Entry SCAN = %d\n", scan);
            if (scan > 1)
            {
                DEBUG_FS("Error! File Entry is not found.\n");
                FS__fat_free(buffer);
                return -1;
            }
            sect = 0;
        }

        if((first == 1) || ((sect & (pBPBUnit->SecPerClus - 1)) == 0))
        {
            result= FS__fat_dir_realsec(Idx, Unit, DirStart, sect, &dsec);
            first = 0;
        }
        else
        {
            dsec +=1;
        }
        if (result < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return result;
        }

        result = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (result < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return result;
        }
        // Scan for pName in the directory sector
        s = (FS__fat_dentry_type*) buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + pBPBUnit->BytesPerSec))
            {
                break;  // End of sector reached
            }
            result = FS__CLIB_strncmp((char*)s->data, pName, len);
            if (result == 0)
            {
                // Name does match
                if (s->data[11] != 0)
                {
                    break;  // Entry found
                }
            }
            s++;
            offset++;
        }
        if (s < (FS__fat_dentry_type*)(buffer + pBPBUnit->BytesPerSec))
        {
            DEBUG_FS2("Delete File Entry = (%d, %d)\n", sect, offset);
            // Entry has been found, delete directory entry
            s->data[0] = 0xe5;	// set file name E5,表示這個條目曾經被刪除不再有用.
            s->data[11] = 0;	// set attribute 0

            result = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); //FDB修改後寫回
            if (result < 0)
            {
                ERRD(FS_REAL_SEC_WRTIE_ERR);
                FS__fat_free(buffer);
                return result;
            }

            // Free blocks in FAT
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
                todo      = filesize >> pBPBUnit->BitNumOfSOC; //todo --> how much cluster
                value     = filesize & (pBPBUnit->SizeOfCluster - 1);
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
            // Lucian: 於DVR file system中,循環錄影模式下,將Delete FAT link放在background task 下 執行.
            switch(dcfOverWriteOP)
            {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                case DCF_OVERWRITE_OP_OFF:
                    break;
                case DCF_OVERWRITE_OP_01_DAYS:
                case DCF_OVERWRITE_OP_07_DAYS:
                case DCF_OVERWRITE_OP_30_DAYS:
                case DCF_OVERWRITE_OP_60_DAYS:
                    result = FSFATFreeFATLink(Idx, Unit, curclst);
                    result = 0; //Lucian: 忽略 delete link error. 因為entry 已經砍掉
                    break;
#endif
                default:
                    if(siuOpMode == SIUMODE_MPEGAVI)
                    {
                        sysbackLowSetEvt(SYSBACKLOW_EVT_DELETEFATLINK,Idx, Unit, curclst, 0);
                        result = 0;
                    }
                    else
                    {
                        result = FSFATFreeFATLink(Idx, Unit, curclst);
                        result = 0; //Lucian: 忽略 delete link error. 因為entry 已經砍掉
                    }
                    break;
            }
#else
            result = FSFATFreeFATLink(Idx, Unit, curclst);
            result = 0; //Lucian: 忽略 delete link error.  因為entry 已經砍掉
#endif
            DEBUG_FS2("Available Cluster = %d\n", global_diskInfo.avail_clusters);
            if (global_diskInfo.avail_clusters > global_diskInfo.total_clusters)
                global_diskInfo.avail_clusters = global_diskInfo.total_clusters;
            FS__fat_free(buffer);
            return result;
        } //  Delete entry
        sect ++;
    }	// for
    //DEBUG_FS("Cannot Find the delete file\n");
    //FS__fat_free(buffer);
    //return curclst;
}

int _FS__fat_ScanFATLink( int Idx, int Unit,int todo,int curclst,unsigned int *pfilesize)
{
    int lastsec,fatindex,fatsec,fatoffs;
    int err;
    int fattype;
    unsigned int fatsize;
    char *buffer;
    u8 semErr;
    //------------------------------------------------------------------//
    *pfilesize = 0;
    if(curclst == 0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }

    OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &semErr);
    if((buffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        OSSemPost(FSFATClustSemEvt);
        return -1;
    }

    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if(fatsize == 0)
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;

    lastsec = -1;
    while(todo)
    {
        if (FS__FAT_aBPBUnit[Idx][Unit].FATType == 1)
            fatindex = curclst + (curclst >> 1);   // FAT12
        else if (FS__FAT_aBPBUnit[Idx][Unit].FATType == 2)
            fatindex = curclst << 2;               // FAT32
        else
            fatindex = curclst << 1;               // FAT16

        //fatsec means the specify sector in FAT
        //用fatindex(byte unit)算出該file再FAT table 的位置.
        fatsec = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + (fatindex >> FS__FAT_aBPBUnit[Idx][Unit].BitNumOfBPS);
        fatoffs = (fatindex & FS__FAT_aBPBUnit[Idx][Unit].BitRevrOfBPS);

        if (fatsec != lastsec) //若是在同一個sector,則不必再重覆讀取
        {
            err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatsec, fatsize + fatsec, (void*)buffer);
            if(err < 0)
            {
                ERRD(FS_LB_READ_FAT_TBL_ERR);
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return err;
            }
            lastsec = fatsec;
        }

        if (FS__FAT_aBPBUnit[Idx][Unit].FATType == 1)  //FAT-12
        {
            if (fatoffs == (FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec - 1))
            {
                u8 a, b;
                a = buffer[fatoffs];
                err  = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, fatsize + fatsec + 1, (void*)buffer);
                if(err < 0)
                {
                    ERRD(FS_LB_READ_FAT_TBL_ERR);
                    FS__fat_free(buffer);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
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
                u8 a, b;
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
            *pfilesize += FS__FAT_aBPBUnit[Idx][Unit].SizeOfCluster;
            if (curclst >= 0x0ff8)
            {
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return -1;
            }
        }
        else if (FS__FAT_aBPBUnit[Idx][Unit].FATType == 2) //FAT32
        {
            /* FAT32 */
            u8 a, b, c, d;
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3] & 0x0f;

            curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;

            *pfilesize += FS__FAT_aBPBUnit[Idx][Unit].SizeOfCluster;
            if (curclst >= (s32)0x0ffffff8L) //判斷是否為EOF
            {
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                buffer[fatoffs]=0xff;
                buffer[fatoffs + 1]=0xff;
                buffer[fatoffs + 2]=0xff;
                buffer[fatoffs + 3]=0xff;

                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                FS__fat_free(buffer);
                if(err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }
        }
        else
        {
            //FAT 16
            u8 a, b;
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];

            curclst  = a + 256 * b;
            curclst &= 0xffff;

            *pfilesize += FS__FAT_aBPBUnit[Idx][Unit].SizeOfCluster;
            if (curclst >= (s32)0xfff8)
            {
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                buffer[fatoffs]=0xff;
                buffer[fatoffs + 1]=0xff;
                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                FS__fat_free(buffer);
                if(err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }
        }
        todo--;
    } /* Free cluster loop */

    DEBUG_FS("Cannot Find EOF\n");
    FS__fat_free(buffer);
    OSSemPost(FSFATClustSemEvt);
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
    FARCHARPTR ext;
    FARCHARPTR s;
    int i;

    s = (FARCHARPTR)pOrgName;
    ext = (FARCHARPTR) FS__CLIB_strchr(s, '.');
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

int FS__fat_find_dir(int Idx, u32 Unit, char *pDirName, u32 DirStart, u32 DirSize, u32 *pDirSec)
{
    FS__fat_dentry_type *s;
    u32 fatsize, dsec, dstart;
    u32 i;
    int len, err, c;
    char *buffer;
    //
    *pDirSec = 0;
    //
    if (pDirName == 0)
    {
        // Return root directory
        if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16)
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
            dstart = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        }
        else
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
            dstart = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize
                     + ((FS__FAT_aBPBUnit[Idx][Unit].RootClus - 2) << FS__FAT_aBPBUnit[Idx][Unit].BitNumOfSPC);
        }
        *pDirSec = dstart;
        return 1;
    }
    else
    {
        // Find directory
        buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
        if (!buffer)
        {
            ERRD(FS_MEMORY_ALLOC_ERR);
            return 0;
        }
        len = FS__CLIB_strlen(pDirName);
        if (len > 11)
        {
            len = 11;
        }
        // Read directory
        for (i = 0; i < DirSize; i++) // Lucian: Scan all directory. sector by sector
        {
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, i, &dsec);
            if (err < 0)
            {
                ERRD(FS_FAT_SEC_CAL_ERR);
                FS__fat_free(buffer);
                return err;
            }

            err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
            if (err < 0)
            {
                ERRD(FS_REAL_SEC_READ_ERR);
                FS__fat_free(buffer);
                return err;
            }
            // Locate the s ptr at buffer[0]
            s = (FS__fat_dentry_type*)buffer;
            while (1)
            {
                if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
                {
                    break;	// End of sector reached
                }
                c = FS__CLIB_strncmp((char*)s->data, pDirName, len);
                if (c == 0)
                {
                    // Name does match
                    if (s->data[11] & FS_FAT_ATTR_DIRECTORY)
                    {
                        break;	// Entry found
                    }
                }
                s++;
            }
            if (s < (FS__fat_dentry_type *)(buffer + FS_FAT_SEC_SIZE))
            {
                // Entry found. Return "cluster number" of 1st block of the directory
                dstart	= (u32)s->data[26];
                dstart += (u32)0x100UL * s->data[27];
                dstart += (u32)0x10000UL * s->data[20];
                dstart += (u32)0x1000000UL * s->data[21];
                FS__fat_free(buffer);
                *pDirSec = dstart;
                return 1;
            }
        }
        dstart = 0;
        FS__fat_free(buffer);
        return -1;
    }
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

int FS__fat_dir_realsec(int Idx, u32 Unit, u32 DirStart, u32 DirOffset, u32 *pDirSec)
{
    u32 rootdir, rsec, dclust, fatsize;
    int result, lexp;
    u8 secperclus;
    //
    *pDirSec = 0;
    //
    lexp = (0 == DirStart);
    lexp = lexp && (FS__FAT_aBPBUnit[Idx][Unit].FATType != 2);
    if (lexp)
    {
        // Sector in FAT12/FAT16 root directory
        //rootdir:0x18
        result = FS__fat_find_dir(Idx, Unit, 0, 0, 0, &rootdir);
        if(result < 0)
        {
            ERRD(FS_DIR_FIND_ERR);
            return result;
        }
        rsec = rootdir + DirOffset;
    }
    else
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
        if (fatsize == 0)
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
        secperclus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        dclust = (DirOffset >> FS__FAT_aBPBUnit[Idx][Unit].BitNumOfSPC);
        if (0 == DirStart)
        {
            // FAT32 root directory
            rsec = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
        }
        else
        {
            result = FS__fat_diskclust(Idx, Unit, DirStart, dclust, &rsec);
            if(result < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);	// resc = 0
                return result;
            }
        }
        rsec -= 2;  //cluster start from 2nd
        rsec *= secperclus;
        rsec += FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        rsec += ((u32)((u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) << 5) >> FS__FAT_aBPBUnit[Idx][Unit].BitNumOfBPS; //at FAT32,FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt=0
        rsec += (DirOffset & (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus - 1));	// likes offset
    }
    *pDirSec = rsec;
    return 1;
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

int FS__fat_dir_size(int Idx, u32 Unit, u32 DirStart, u32 *pDirSize)
{
    u32 clustNum, dsize;
    s32 result;
    //
    *pDirSize = 0;
    //
    if(DirStart == 0)
    {
        // For FAT12/FAT16 root directory, the size can be found in BPB
        dsize = (((FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) << 5) >> FS__FAT_aBPBUnit[Idx][Unit].BitNumOfBPS);
        if(dsize)
        {
            *pDirSize = dsize;
            return 1;
        }
        // Size in BPB is 0, so it is a FAT32 (FAT32 does not have a real root dir)
        result = FS__fat_FAT_find_eof(Idx, Unit, FS__FAT_aBPBUnit[Idx][Unit].RootClus, &dsize, &clustNum);
        if (result < 0)
        {
            ERRD(FS_FAT_EOF_FIND_ERR);
            return result;
        }
    }
    else
    {
        // Calc size of a sub-dir
        result = FS__fat_FAT_find_eof(Idx, Unit, DirStart, &dsize, &clustNum);
        if (result < 0)
        {
            ERRD(FS_FAT_EOF_FIND_ERR);
            return result;
        }
    }
    *pDirSize = (dsize << FS__FAT_aBPBUnit[Idx][Unit].BitNumOfSPC);
    return 1;
}


/*********************************************************************
*
*             FS__fat_findpath
*
  Description:
  FS internal function. Return start cluster and size of the directory
  of the file name in pFileName.

  	#找到被指派路徑的最底層資料夾，並回傳最底層資料夾所占的dir entry length，
  		這length，可能會是存放dir entry的length 或是存放file entry的length

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  pFullName   - Fully qualified file name w/o device name.
  pFileName   - Pointer to a pointer, which is modified to point to the
                file name part of pFullName.
                //paste civic
                	e.g. "0:\\dir1\\dir2",
		     "0:\\dir1\\dir2\\test.txt"
  pUnit       - Pointer to an u32 for returning the unit number.
  pDirStart   - Pointer to an u32 for returning the start cluster of
                the directory.

  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

int FS__fat_findpath(int Idx, const char *pFullName, FARCHARPTR *pFileName, u32 *pUnit, u32 *pDirStart, u32 *pDirSize)
{
    u32 dsize;
    char *dname_start, *dname_stop, *chprt;
    int x, i, j;
    char dname[12], realname[12];
    //
    *pDirSize = 0;
    // Find correct unit (unit:name)
    *pFileName = (FARCHARPTR)FS__CLIB_strchr(pFullName, ':');
    if (*pFileName)
    {
        // Scan for unit number
        *pUnit = FS__CLIB_atoi(pFullName);
        (*pFileName)++;
    }
    else
    {
        // Use 1st unit as default
        *pUnit = 0;
        *pFileName = (FARCHARPTR) pFullName;
    }
    // Check volume
    x = FS__fat_checkunit(Idx, *pUnit);	//Read BPB(MBR)
    if (x < 0)
    {
        ERRD(FS_DEV_UNIT_CHECK_ERR);
        return x;
    }
    // Setup pDirStart/dsize for root directory
    *pDirStart = 0;
    // 利用root dir的dir entry size來決定接下來要掃描的dir entry length，
    //	但這裡同時也限制了folder的最大上限值
    x = FS__fat_dir_size(Idx, *pUnit, 0, &dsize);  // Lucian: 算出root directory 的size. (sector unit)
    if(x < 0)
    {
        ERRD(FS_DIR_FIND_ERR);
        return x;
    }
    // Lucian: 從Root directory 開始找到 destination
    // Find correct directory
    do
    {
        dname_start = (FARCHARPTR)FS__CLIB_strchr(*pFileName, '\\');
        if (dname_start)
        {
            dname_start++;
            *pFileName = dname_start;
            dname_stop = (FARCHARPTR)FS__CLIB_strchr(dname_start, '\\');
        }
        else
        {
            dname_stop = 0;
        }
        if (dname_stop)
        {
            i = dname_stop - dname_start;
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
                    DEBUG_FS("[Error]: Path length is longer than 12.\nstart: %s\nstop: %s\n", dname_start, dname_stop);
                    return -1;
                }
            }
            else
            {
                FS__CLIB_strncpy(realname, dname_start, i);
            }
            realname[i] = 0;
            FS__fat_make_realname(dname, realname); //將realname-->dname,做大小寫轉換.
            x =  FS__fat_find_dir(Idx, *pUnit, dname, *pDirStart, dsize, pDirStart);
            if (x >= 0)
            {
                x = FS__fat_dir_size(Idx, *pUnit, *pDirStart, &dsize);
                if(x < 0)
                {
                    ERRD(FS_DIR_FIND_ERR);
                    return x;
                }
            }
            else
            {
                ERRD(FS_DIR_FIND_ERR);
                dsize = 0;    // Directory NOT found
            }
        }
    }
    while (dname_start);

    if(x < 0)
        return -1;
    *pDirSize = dsize;
    return 1;
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
    FARCHARPTR fname;
    u32 unit;
    u32 dstart;
    u32 dsize;
    char realname[12];
    char newname[12];

    FS__fat_dentry_type *s;
    unsigned int SecPerClus;
    char *buffer;
    int len;
    int i;
    u32 dsec = 0;
    int err;
    int offset;
    int c;
    //==========================//

    err = FS__fat_findpath(index, pOldFilePath, &fname, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", index, pOldFilePath);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Directory not found
    }
    FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之 */
    FS__fat_make_realname(newname, pNewFileName);

    SecPerClus = (unsigned int)FS__FAT_aBPBUnit[index][unit].SecPerClus;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
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
            err = FS__fat_dir_realsec(index, unit, dstart, i, &dsec);
        else
            dsec +=1;

        if (err < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }
        err = FS__lb_sin_read(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
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
            {
                /* Name does match */
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

            err = FS__lb_sin_write(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
            FS__fat_free(buffer);
            if(err < 0)
            {
                ERRD(FS_REAL_SEC_WRTIE_ERR);
                return err;
            }
            return 1;
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

int FS__fat_fopen(const char *pFileName, const char *pMode, FS_FILE *pFile)
{
    FS__fat_dentry_type s;
    u32 unit, dstart, dsize, fileClust;
    u32 EntSect, EntSectOffset;
    char *fname, realname[12];
    int i, err, lexp_a, lexp_b;
    //
    EntSect = 0;
    fileClust = 0;
    //
    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // Not a valid pointer to an FS_FILE structure
    }
    //FS_FOpen was filled by FS_FOpen function
    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    err = FS__fat_findpath(pFile->dev_index, pFileName, &fname, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", pFile->dev_index, pFileName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Directory not found
    }
    // No execution because never implement FS_CMD_INC_BUSYCNT ioctl command
    err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  /* Turn on busy signal */
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }
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
    {
        //delete file//
#if FILE_SYSTEM_DVF_TEST
        u32 time1,time2;
        time1 = OSTimeGet();
#endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect, 1);
        if (i < 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return err;	//?
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return -1;	//?
        }
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        //time1 = time1;	// avoid warming message
        //time2 = time2;	// avoid warming message
        DEBUG_FS("--->Delete File Time=%d (x50msec)\n",time2-time1);
#endif
        return i;
    }

    //-----------------------Create File Path--------------------//
    //dstart is the start cluster of directory, dsize is the size of directory
    //Lucian: 找到該目錄下該檔案的 start cluster.
    lexp_b = (FS__CLIB_strcmp(pMode, "w-") == 0);
    if(lexp_b)
    {
        i =- 1;
    }
    else
    {
#if FILE_SYSTEM_DVF_TEST
        u32 time1,time2;
        time1=OSTimeGet();
#endif
        i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize, &fileClust);
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        time1 = time1;	// avoid warming message
        time2 = time2;	// avoid warming message
        DEBUG_FS2("--->_FS_fat_find_file = %d (x50msec)\n",time2-time1);
#endif
    }
#else
    //Lucian: 找到該目錄下該檔案的 start cluster.
    i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize);

    /* Delete file */
    lexp_b = (FS__CLIB_strcmp(pMode, "del") == 0);    /* Delete file request */
    lexp_a = lexp_b && (i >= 0);                      /* File does exist,and want to delete */
    if (lexp_a)
    {
        //delete file//
#if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
#endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect,1);
        if (i != 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return 0;	//?
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return 0;	//?
        }
#if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        DEBUG_FS("Delete File Time=%d (x50msec)\n",time2-time1);
#endif
        return 0;
    }
    else if (lexp_b)
    {
        //delete file but file don't exist.
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        pFile->error = -1;
        return 0;
    }
#endif
    // Check read only
    lexp_a = ((i >= 0) && ((s.data[11] & FS_FAT_ATTR_READ_ONLY) != 0)) && ((pFile->mode_w) || (pFile->mode_a) || (pFile->mode_c));
    if (lexp_a)
    {
        // Files is RO and we try to create, write or append. Not allow
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        return -1;
    }
    lexp_a = (i >= 0) && (!pFile->mode_a) &&
             (((pFile->mode_w) && (!pFile->mode_r)) || ((pFile->mode_w) && (pFile->mode_c) && (pFile->mode_r)));
    if (lexp_a)
    {
        // Delete old file
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect, 1);
        // FileSize = 0
        s.data[28] = 0x00;
        s.data[29] = 0x00;
        s.data[30] = 0x00;
        s.data[31] = 0x00;
        i =- 1;
    }

    if ((!pFile->mode_c) && (i < 0))
    {
        // File does not exist and we don't create
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        return -1;
    }
    else if ((pFile->mode_c) && (i < 0))
    {
        // File does not exist and Create new file
#if FILE_SYSTEM_DVF_TEST
        u32 time1, time2;
        time1 = OSTimeGet();
#endif
        i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize, &EntSect, &EntSectOffset, &fileClust);
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        time1 = time1;
        time2 = time2;
        DEBUG_FS2("--->_FS_fat_create_file = %d (x50msec)\n",time2-time1);
#endif

		switch(i)
		{
			case FS_FILE_NAME_REPEAT_ERR:
				return -1;
				
			case -2:
				DEBUG_FS("File Entry Full. Create new one.\n");
				// Directory is full, try to increase
                if((i = FSFATIncEntry(pFile->dev_index, unit, dstart, 1, &dsize, FS_E_CLUST_CLEAN_ON)) >= 0)
 					i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize, &EntSect, &EntSectOffset, &fileClust);
 			default:
 				if (i < 0)
	            {
	                err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
	                if(err < 0)
	                    ERRD(FS_DEV_IOCTL_ERR);
	                return -1;
	            }
	            DEBUG_FS("[INF] EntSect = (%d, %d)\n", EntSect, EntSectOffset);
	            break;
		}
    }
    else
    {
        pFile->EOFClust = 0;//   檔案在,append
    }

    pFile->fileid_lo = unit;	// unit whre file is located.
    pFile->fileid_hi = fileClust;	// 1st clust of file location.(Low byte)
    pFile->fileid_ex = dstart;	// dstart is the start cluster of directory
    pFile->FileEntrySect = EntSect;
    pFile->CurClust = 0;	// 未知
    pFile->EOFClust = fileClust;
    pFile->error = 0;	// 未知
    // FileSize
    pFile->size = s.data[28] + 0x100UL * s.data[29] + 0x10000UL * s.data[30] + 0x1000000UL * s.data[31];
    if (pFile->mode_a)
    {
        pFile->filepos = pFile->size; //從檔案尾開始接
        if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, fileClust, 0, &fileClust)) < 0)
        {
         ERRD(FS_FAT_CLUT_FIND_ERR);
            return err;
        }
        pFile->EOFClust = fileClust;
    }
    else
        pFile->filepos = 0;//filepos: 是從檔案頭開始算
    pFile->inuse = 1;
    return 1;
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
    FS__fat_dentry_type s;
    u32 unit, dstart, dsize, dsec;
    int result;
    char *fname, realname[12];

    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    result = FS__fat_findpath(Idx, pFileName, &fname, &unit, &dstart, &dsize);
    if (result < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", Idx, pFileName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return result;  // Directory not found
    }
    FS__fat_make_realname(realname, fname);  // Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之

    result = _FS_fat_find_file(Idx, unit, realname, &s, dstart, dsize, &dsec);
    if(result < 0)
    {
        ERRD(FS_FILE_FIND_ERR);
        return result;
    }

    return dsec;

}
#else
static s32 _FS_fat_find_file(int Idx, u32 Unit, const char *pFileName, FS__fat_dentry_type *pDirEntry,
                             u32 DirStart, u32 DirSize, u32 *pFileCluster)
{
    FS__fat_dentry_type *s;
    u32 SecPerClus;
    u32 i, dsec;
    int len, offset, err, c;
    char *buffer;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    dsec = 0;
    SecPerClus = (u32)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
    {
        len = 11;
    }

    // Read directory
    for (i = 0; i < DirSize; i++)
    {
        if( (i % SecPerClus)==0) // Lucian: optimize here..
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, i, &dsec);
        else
            dsec += 1;
        if (err < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset = 0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }
            c = FS__CLIB_strncmp((char*)s->data, pFileName, len);
            if (c == 0)
            {
                // Name does match
                if (s->data[11] & FS_FAT_ATTR_ARCHIVE)
                {
                    break;  // Entry found
                }
            }
            offset ++;
            s++;
        }

        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            // Entry found. Return number of 1st block of the file
            if (pDirEntry)
            {
                FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
            }
            *pFileCluster = (s->data[26]) +
                            (s->data[27] << 8) +
                            (s->data[20] << 16) +
                            (s->data[21] << 24);	// set 該檔案的 first cluster
            FS__fat_free(buffer);
            return 1;
        }
    }
    FS__fat_free(buffer);
    return -1;
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

static int _FS_fat_create_file(int Idx, u32 Unit, const char *pFileName, u32 DirStart, u32 DirSize, 
							u32 *pEntSect, u32 *pEntSectOffset, u32 *pFileCluster)
{
    FS__fat_dentry_type *s;
    u32 sect, offset, dsec = 0;
    u32 cluster, Size;
    int len, err;
    u16 val;
    char *buffer;
    RTC_DATE_TIME localTime;
    int scan, first, SecPerClus;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    len = FS__CLIB_strlen(pFileName);
    if (len > 11)
        len = 11;

    *pEntSect = 0;
    *pEntSectOffset = 0;
    //-------------------- Read directory ----------------------
    //	目前做法是: Scan all directory (sector by sector). 從cluster頭到尾,直到找到空的file entry.
    //    新的作法:

    SecPerClus = (u32) FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    scan = 0;
    first = 1;
    offset = 0;
    sect = *pEntSect;

    while(1)
    {
        if(sect >= DirSize)
        {
            scan ++;
            DEBUG_FS("File Entry SCAN = %d\n", scan);
            if (scan > 1)
            {
                DEBUG_FS("File Entry is full\n");
                FS__fat_free(buffer);
                return -2;	// Directory is full
            }
            sect=0;
        }

        if((first == 1) || ((sect % SecPerClus) == 0))
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, sect, &dsec);
        else
            dsec +=1;
        first = 0;
        if (err < 0)
        {
            DEBUG_RED("DirStart: %#x, sect: %d, DirSize: %d\n", DirStart, sect, DirSize);
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }

        s = (FS__fat_dentry_type*)buffer;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }
            if (s->data[0] == 0x00)
            {
                break;  // Empty entry found
            }
            if(strncmp((const char *)s->data, pFileName, FS_V_FAT_ENTEY_SHORT_NAME) == 0)
            {
            	ERRD(FS_FILE_NAME_REPEAT_ERR);
            	FS__fat_free(buffer);
            	return FS_FILE_NAME_REPEAT_ERR;
            }
            s++;
            offset ++;
        }

        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            *pEntSect = sect;
            *pEntSectOffset = offset % (FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec / FS_FAT_DENTRY_SIZE);
            DEBUG_FS2("NextFreeCluster = %d\n", FS__pDevInfo[Idx].FSInfo.NextFreeCluster);

            // Free entry found. Make entry and return 1st block of the file
            FS__CLIB_strncpy((char*)s->data, pFileName, len);
            s->data[11] = FS_FAT_ATTR_ARCHIVE;
            // Alloc block in FAT
            if((err = FSFATNewEntry(Idx, Unit, &cluster, 1, &Size, FS_E_CLUST_CLEAN_OFF)) < 0)
            {
            	ERRD(FS_FAT_CLUT_ALLOC_ERR);
            	FS__fat_free(buffer);
            	return err;
            }
            //----紀錄 create time , modified time---//
            if (cluster > 0)
            {
                s->data[12] = 0x00;	// Res
                s->data[13] = 0x00;	// CrtTimeTenth (optional, not supported)
                s->data[22] = 0x00;	// CrtTime (optional, not supported)
                s->data[23] = 0x00;
                s->data[24] = 0x00;	// CrtDate (optional, not supported)
                s->data[25] = 0x00;

                RTC_Get_Time(&localTime);
                val=((localTime.hour& 0x1F )<<11) |((localTime.min  & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);
                s->data[14] = (u8)(val & 0xff);	// WrtTime
                s->data[15] = (u8)(val / 256);
                // + 2000 -1980 =20
                val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);
                s->data[16] = (u8)(val & 0xff);	// WrtDate
                s->data[17] = (u8)(val / 256);
                s->data[18] = (u8)(val & 0xff);	// LstAccDate (optional, not supported)
                s->data[19] = (u8)(val / 256);
                s->data[26] = (u8)(cluster & 0xff);	// FstClusLo / FstClusHi
                s->data[27] = (u8)((cluster / 256) & 0xff);
                s->data[20] = (u8)((cluster / 0x10000L) & 0xff);
                s->data[21] = (u8)((cluster / 0x1000000L) & 0xff);
                s->data[28] = 0x00;	// FileSize
                s->data[29] = 0x00;
                s->data[30] = 0x00;
                s->data[31] = 0x00;

                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    return err;
                }
            }
            else
                ERRD(FS_FAT_CLUT_LINK_ERR);
            //======//
            FS__fat_free(buffer);
            *pFileCluster = cluster;
            return 1;	//???
        }
        sect++;
    }
    //FS__fat_free(buffer);
    //return -2;      /* Directory is full */
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

int FS__fat_DeleteFileOrDir(int Idx, u32 Unit,  const char *pName, u32 DirStart, u32 DirSize, u16 FileEntrySect, char RmFile)
{
    FS__fat_dentry_type *s;
    u32 bytespersec, SecPerClus;
    u32 offset, scan, first, value, sect, filesize, dsec = 0, curclst;
    s32 len, todo;
    char *buffer;
    int result;
    //--------//
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    bytespersec = (s32) FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    SecPerClus = (u32) FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    //
    len = FS__CLIB_strlen(pName);
    if (len > 11)
    {
        len = 11;
    }
    //
    DEBUG_FS2("Estimate File Entry Secter: %d\n", FileEntrySect);
    if(FileEntrySect >= DirSize)
    {
        ERRD(FS_FAT_SEC_CAL_ERR);
        DEBUG_FS("FileEntrySect: %#x, DirSize: %#x\n", FileEntrySect, DirSize);
        FileEntrySect = 0;
    }

    scan = 0;
    first = 1;	// the switch to read data in first round
    sect = FileEntrySect;
    // Read directory
    while (1) //DirSize: sector unit.
    {
        curclst = 0;
        if(sect >= DirSize)
        {
            scan ++;
            DEBUG_FS("File Entry SCAN = %d\n", scan);
            if (scan > 1)
            {
                DEBUG_FS("Error! File Entry is not found.\n");
                FS__fat_free(buffer);
                return -1;
            }
            sect = 0;
        }

        if((first == 1) || ((sect % SecPerClus) == 0))
        {
            result= FS__fat_dir_realsec(Idx, Unit, DirStart, sect, &dsec);
            first = 0;
        }
        else
        {
            dsec +=1;
        }
        if (result < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return result;
        }

        result = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
        if (result < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return result;
        }
        // Scan for pName in the directory sector
        s = (FS__fat_dentry_type*) buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + bytespersec))
            {
                break;  // End of sector reached
            }
            result = FS__CLIB_strncmp((char*)s->data, pName, len);
            if (result == 0)
            {
                // Name does match
                if (s->data[11] != 0)
                {
                    break;  // Entry found
                }
            }
            s++;
            offset++;
        }
        if (s < (FS__fat_dentry_type*)(buffer + bytespersec))
        {
            DEBUG_FS2("Delete File Entry = (%d, %d)\n", sect, offset);
            // Entry has been found, delete directory entry
            s->data[0] = 0xe5;	// set file name E5,表示這個條目曾經被刪除不再有用.
            s->data[11] = 0;	// set attribute 0

            result = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); //FDB修改後寫回
            if (result < 0)
            {
                ERRD(FS_REAL_SEC_WRTIE_ERR);
                FS__fat_free(buffer);
                return result;
            }

            // Free blocks in FAT
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
            // Lucian: 於DVR file system中,循環錄影模式下,將Delete FAT link放在background task 下 執行.
            switch(dcfOverWriteOP)
            {
            #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                case DCF_OVERWRITE_OP_OFF:
                    break;
                case DCF_OVERWRITE_OP_01_DAYS:
                case DCF_OVERWRITE_OP_07_DAYS:
                case DCF_OVERWRITE_OP_30_DAYS:
                case DCF_OVERWRITE_OP_60_DAYS:
                    result = FSFATFreeFATLink(Idx, Unit, curclst);
                    result = 0; //Lucian: 忽略 delete link error. 因為entry 已經砍掉
                    break;
            #endif
                default:
                    if(siuOpMode == SIUMODE_MPEGAVI)
                    {
                        sysbackLowSetEvt(SYSBACKLOW_EVT_DELETEFATLINK,Idx, Unit, curclst, 0);
                        global_diskInfo.avail_clusters +=todo; // 由於在background 做, cluster. available cluster 先加回.
                        result = 0;
                    }
                    else
                    {
                        result = FSFATFreeFATLink(Idx, Unit, curclst);
                        result = 0; //Lucian: 忽略 delete link error. 因為entry 已經砍掉
                    }
                    break;
            }
#else
            result = FSFATFreeFATLink(Idx, Unit, curclst);
            result = 0; //Lucian: 忽略 delete link error.  因為entry 已經砍掉
#endif
            DEBUG_FS2("Available Cluster = %d\n", global_diskInfo.avail_clusters);
            if (global_diskInfo.avail_clusters > global_diskInfo.total_clusters)
                global_diskInfo.avail_clusters = global_diskInfo.total_clusters;
            FS__fat_free(buffer);
            return result;
        } //  Delete entry
        sect ++;
    }	// for
    //DEBUG_FS("Cannot Find the delete file\n");
    //FS__fat_free(buffer);
    //return curclst;
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
    //s32 Data_start_sector, Root_start_sector;
    u32  byte_per_clustrer;
    //u32  first_temp_cluster, second_temp_cluster;
    u8 semErr;


    //------------------------------------------------------------------//
    *pfilesize = 0;
    if(curclst == 0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }

    OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &semErr);

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        OSSemPost(FSFATClustSemEvt);
        return -1;
    }

    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0)
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }

    //Root_start_sector = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
    //Data_start_sector = (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt*0x20)/ FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec + Root_start_sector;

    bytespersec = (s32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    byte_per_clustrer = (s32)FS__FAT_aBPBUnit[Idx][Unit].SecPerClus * bytespersec;

    lastsec = -1;

    RsvdSecCnt=FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
    while (todo)
    {
        //first_temp_cluster = curclst;
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

        if (fatsec != lastsec) //若是在同一個sector,則不必再重覆讀取
        {
            err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatsec, fatsize + fatsec, (void*)buffer);
            if(err < 0)
            {
                ERRD(FS_LB_READ_FAT_TBL_ERR);
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return err;
            }
            lastsec = fatsec;
        }

        if (fattype == 1)  //FAT-12
        {
            if (fatoffs == (bytespersec - 1))
            {
                u8 a, b;
                a = buffer[fatoffs];
                err  = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, fatsize + fatsec + 1, (void*)buffer);
                if(err < 0)
                {
                    ERRD(FS_LB_READ_FAT_TBL_ERR);
                    FS__fat_free(buffer);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
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
                u8 a, b;
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
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return -1;
            }
        }
        else if (fattype == 2) //FAT32
        {
            /* FAT32 */
            u8 a, b, c, d;
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3] & 0x0f;

            curclst = a + 0x100 * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;

            *pfilesize += byte_per_clustrer;
            if (curclst >= (s32)0x0ffffff8L) //判斷是否為EOF
            {
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                buffer[fatoffs]=0xff;
                buffer[fatoffs + 1]=0xff;
                buffer[fatoffs + 2]=0xff;
                buffer[fatoffs + 3]=0xff;

                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                FS__fat_free(buffer);
                if(err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }
        }
        else
        {
            //FAT 16
            u8 a, b;
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];

            curclst  = a + 256 * b;
            curclst &= 0xffff;

            *pfilesize += byte_per_clustrer;
            if (curclst >= (s32)0xfff8)
            {
                FS__fat_free(buffer);
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }

            if(curclst == 0)
            {
                DEBUG_FS("Cannot Find EOF! Mark EOF\n");
                buffer[fatoffs]=0xff;
                buffer[fatoffs + 1]=0xff;
                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                FS__fat_free(buffer);
                if(err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    OSSemPost(FSFATClustSemEvt);
                    return err;
                }
                OSSemPost(FSFATClustSemEvt);
                return 1;
            }
        }
        //second_temp_cluster = curclst;
        todo--;
    } /* Free cluster loop */

    DEBUG_FS("Cannot Find EOF\n");
    FS__fat_free(buffer);
    OSSemPost(FSFATClustSemEvt);
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
    FARCHARPTR ext;
    FARCHARPTR s;
    int i;

    s = (FARCHARPTR)pOrgName;
    ext = (FARCHARPTR) FS__CLIB_strchr(s, '.');
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

int FS__fat_find_dir(int Idx, u32 Unit, char *pDirName, u32 DirStart, u32 DirSize, u32 *pDirSec)
{
    FS__fat_dentry_type *s;
    u32 fatsize, dsec, dstart;
    u32 i;
    int len, err, c;
    char *buffer;
    //
    *pDirSec = 0;
    //
    if (pDirName == 0)
    {
        // Return root directory
        if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16)
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
            dstart = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        }
        else
        {
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
            dstart = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize
                     + (FS__FAT_aBPBUnit[Idx][Unit].RootClus - 2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        }
        *pDirSec = dstart;
        return 1;
    }
    else
    {
        // Find directory
        buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
        if (!buffer)
        {
            ERRD(FS_MEMORY_ALLOC_ERR);
            return 0;
        }
        len = FS__CLIB_strlen(pDirName);
        if (len > 11)
        {
            len = 11;
        }
        // Read directory
        for (i = 0; i < DirSize; i++) // Lucian: Scan all directory. sector by sector
        {
            err = FS__fat_dir_realsec(Idx, Unit, DirStart, i, &dsec);
            if (err < 0)
            {
                ERRD(FS_FAT_SEC_CAL_ERR);
                FS__fat_free(buffer);
                return err;
            }

            err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
            if (err < 0)
            {
                ERRD(FS_REAL_SEC_READ_ERR);
                FS__fat_free(buffer);
                return err;
            }
            // Locate the s ptr at buffer[0]
            s = (FS__fat_dentry_type*)buffer;
            while (1)
            {
                if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
                {
                    break;	// End of sector reached
                }
                c = FS__CLIB_strncmp((char*)s->data, pDirName, len);
                if (c == 0)
                {
                    // Name does match
                    if (s->data[11] & FS_FAT_ATTR_DIRECTORY)
                    {
                        break;	// Entry found
                    }
                }
                s++;
            }
            if (s < (FS__fat_dentry_type *)(buffer + FS_FAT_SEC_SIZE))
            {
                // Entry found. Return "cluster number" of 1st block of the directory
                dstart	= (u32)s->data[26];
                dstart += (u32)0x100UL * s->data[27];
                dstart += (u32)0x10000UL * s->data[20];
                dstart += (u32)0x1000000UL * s->data[21];
                FS__fat_free(buffer);
                *pDirSec = dstart;
                return 1;
            }
        }
        dstart = 0;
        FS__fat_free(buffer);
        return -1;
    }
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

int FS__fat_dir_realsec(int Idx, u32 Unit, u32 DirStart, u32 DirOffset, u32 *pDirSec)
{
    u32 rootdir, rsec, dclust, fatsize;
    int result, lexp;
    u8 secperclus;
    //
    *pDirSec = 0;
    //
    lexp = (0 == DirStart);
    lexp = lexp && (FS__FAT_aBPBUnit[Idx][Unit].FATType != 2);
    if (lexp)
    {
        // Sector in FAT12/FAT16 root directory
        //rootdir:0x18
        result = FS__fat_find_dir(Idx, Unit, 0, 0, 0, &rootdir);
        if(result < 0)
        {
            ERRD(FS_DIR_FIND_ERR);
            return result;
        }
        rsec = rootdir + DirOffset;
    }
    else
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
        if (fatsize == 0)
            fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
        secperclus = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        dclust = DirOffset / secperclus;
        if (0 == DirStart)
        {
            // FAT32 root directory
            rsec = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
        }
        else
        {
            result = FS__fat_diskclust(Idx, Unit, DirStart, dclust, &rsec);
            if(result < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);	// resc = 0
                return result;
            }
        }
        rsec -= 2;  //cluster start from 2nd
        rsec *= secperclus;
        rsec += FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs * fatsize;
        rsec += ((u32)((u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) * FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE; //at FAT32,FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt=0
        rsec += (DirOffset % secperclus);	// likes offset
    }
    *pDirSec = rsec;
    return 1;
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

int FS__fat_dir_size(int Idx, u32 Unit, u32 DirStart, u32 *pDirSize)
{
    u32 clustNum, dsize;
    s32 result;
    //
    *pDirSize = 0;
    //
    if (DirStart == 0)
    {
        // For FAT12/FAT16 root directory, the size can be found in BPB
        dsize = ((u32)((u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) * FS_FAT_DENTRY_SIZE) / ((u32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec);
        if(dsize)
        {
            *pDirSize = dsize;
            return 1;
        }
        // Size in BPB is 0, so it is a FAT32 (FAT32 does not have a real root dir)
        result = FS__fat_FAT_find_eof(Idx, Unit, FS__FAT_aBPBUnit[Idx][Unit].RootClus, &dsize, &clustNum);
        if (result < 0)
        {
            ERRD(FS_FAT_EOF_FIND_ERR);
            return result;
        }
    }
    else
    {
        // Calc size of a sub-dir
        result = FS__fat_FAT_find_eof(Idx, Unit, DirStart, &dsize, &clustNum);
        if (result < 0)
        {
            ERRD(FS_FAT_EOF_FIND_ERR);
            return result;
        }
    }
    *pDirSize = dsize * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    return 1;
}


/*********************************************************************
*
*             FS__fat_findpath
*
  Description:
  FS internal function. Return start cluster and size of the directory
  of the file name in pFileName.

  	#找到被指派路徑的最底層資料夾，並回傳最底層資料夾所占的dir entry length，
  		這length，可能會是存放dir entry的length 或是存放file entry的length

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  pFullName   - Fully qualified file name w/o device name.
  pFileName   - Pointer to a pointer, which is modified to point to the
                file name part of pFullName.
                //paste civic
                	e.g. "0:\\dir1\\dir2",
		     "0:\\dir1\\dir2\\test.txt"
  pUnit       - Pointer to an u32 for returning the unit number.
  pDirStart   - Pointer to an u32 for returning the start cluster of
                the directory.

  Return value:
  >0          - Sector (not cluster) size of the directory.
  ==0         - An error has occured.
*/

int FS__fat_findpath(int Idx, const char *pFullName, FARCHARPTR *pFileName, u32 *pUnit, u32 *pDirStart, u32 *pDirSize)
{
    u32 dsize;
    char *dname_start, *dname_stop, *chprt;
    int x, i, j;
    char dname[12], realname[12];
    //
    *pDirSize = 0;
    // Find correct unit (unit:name)
    *pFileName = (FARCHARPTR)FS__CLIB_strchr(pFullName, ':');
    if (*pFileName)
    {
        // Scan for unit number
        *pUnit = FS__CLIB_atoi(pFullName);
        (*pFileName)++;
    }
    else
    {
        // Use 1st unit as default
        *pUnit = 0;
        *pFileName = (FARCHARPTR) pFullName;
    }
    // Check volume
    x = FS__fat_checkunit(Idx, *pUnit);	//Read BPB(MBR)
    if (x < 0)
    {
        ERRD(FS_DEV_UNIT_CHECK_ERR);
        return x;
    }
    // Setup pDirStart/dsize for root directory
    *pDirStart = 0;
    // 利用root dir的dir entry size來決定接下來要掃描的dir entry length，
    //	但這裡同時也限制了folder的最大上限值
    x = FS__fat_dir_size(Idx, *pUnit, 0, &dsize);  // Lucian: 算出root directory 的size. (sector unit)
    if(x < 0)
    {
        ERRD(FS_DIR_FIND_ERR);
        return x;
    }
    // Lucian: 從Root directory 開始找到 destination
    // Find correct directory
    do
    {
        dname_start = (FARCHARPTR)FS__CLIB_strchr(*pFileName, '\\');
        if (dname_start)
        {
            dname_start++;
            *pFileName = dname_start;
            dname_stop = (FARCHARPTR)FS__CLIB_strchr(dname_start, '\\');
        }
        else
        {
            dname_stop = 0;
        }
        if (dname_stop)
        {
            i = dname_stop - dname_start;
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
                    DEBUG_FS("[Error]: Path length is longer than 12.\nstart: %s\nstop: %s\n", dname_start, dname_stop);
                    return -1;
                }
            }
            else
            {
                FS__CLIB_strncpy(realname, dname_start, i);
            }
            realname[i] = 0;
            FS__fat_make_realname(dname, realname); //將realname-->dname,做大小寫轉換.
            x =  FS__fat_find_dir(Idx, *pUnit, dname, *pDirStart, dsize, pDirStart);
            if (x >= 0)
            {
                x = FS__fat_dir_size(Idx, *pUnit, *pDirStart, &dsize);
                if(x < 0)
                {
                    ERRD(FS_DIR_FIND_ERR);
                    return x;
                }
            }
            else
            {
                ERRD(FS_DIR_FIND_ERR);
                dsize = 0;    // Directory NOT found
            }
        }
    }
    while (dname_start);

    if(x < 0)
        return -1;
    *pDirSize = dsize;
    return 1;
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
    FARCHARPTR fname;
    u32 unit;
    u32 dstart;
    u32 dsize;
    char realname[12];
    char newname[12];

    FS__fat_dentry_type *s;
    unsigned int SecPerClus;
    char *buffer;
    int len;
    int i;
    u32 dsec = 0;
    int err;
    int offset;
    int c;
    //==========================//

    err = FS__fat_findpath(index, pOldFilePath, &fname, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", index, pOldFilePath);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Directory not found
    }
    FS__fat_make_realname(realname, fname);  /* Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之 */
    FS__fat_make_realname(newname, pNewFileName);

    SecPerClus = (unsigned int)FS__FAT_aBPBUnit[index][unit].SecPerClus;
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
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
            err = FS__fat_dir_realsec(index, unit, dstart, i, &dsec);
        else
            dsec +=1;

        if (err < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }
        err = FS__lb_sin_read(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
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
            {
                /* Name does match */
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

            err = FS__lb_sin_write(FS__pDevInfo[index].devdriver, unit, dsec, (void*)buffer);
            FS__fat_free(buffer);
            if(err < 0)
            {
                ERRD(FS_REAL_SEC_WRTIE_ERR);
                return err;
            }
            return 1;
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

int FS__fat_fopen(const char *pFileName, const char *pMode, FS_FILE *pFile)
{
    FS__fat_dentry_type s;
    u32 unit, dstart, dsize, fileClust;
    u32 EntSect, EntSectOffset;
    char *fname, realname[12];
    int i, err, lexp_a, lexp_b;
    //
    EntSect = 0;
    fileClust = 0;
    //
    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // Not a valid pointer to an FS_FILE structure
    }
    //FS_FOpen was filled by FS_FOpen function
    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    err = FS__fat_findpath(pFile->dev_index, pFileName, &fname, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", pFile->dev_index, pFileName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Directory not found
    }
    // No execution because never implement FS_CMD_INC_BUSYCNT ioctl command
    err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  /* Turn on busy signal */
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }
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
    {
        //delete file//
#if FILE_SYSTEM_DVF_TEST
        u32 time1,time2;
        time1 = OSTimeGet();
#endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect, 1);
        if (i < 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return err;	//?
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return -1;	//?
        }
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        //time1 = time1;	// avoid warming message
        //time2 = time2;	// avoid warming message
        DEBUG_FS("--->Delete File Time=%d (x50msec)\n",time2-time1);
#endif
        return i;
    }

    //-----------------------Create File Path--------------------//
    //dstart is the start cluster of directory, dsize is the size of directory
    //Lucian: 找到該目錄下該檔案的 start cluster.
    lexp_b = (FS__CLIB_strcmp(pMode, "w-") == 0);
    if(lexp_b)
    {
        i =- 1;
    }
    else
    {
#if FILE_SYSTEM_DVF_TEST
        u32 time1,time2;
        time1=OSTimeGet();
#endif
        i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize, &fileClust);
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        time1 = time1;	// avoid warming message
        time2 = time2;	// avoid warming message
        DEBUG_FS2("--->_FS_fat_find_file = %d (x50msec)\n",time2-time1);
#endif
    }
#else
    //Lucian: 找到該目錄下該檔案的 start cluster.
    i = _FS_fat_find_file(pFile->dev_index, unit, realname, &s, dstart, dsize);

    /* Delete file */
    lexp_b = (FS__CLIB_strcmp(pMode, "del") == 0);    /* Delete file request */
    lexp_a = lexp_b && (i >= 0);                      /* File does exist,and want to delete */
    if (lexp_a)
    {
        //delete file//
#if FILE_SYSTEM_DVF_TEST
        time1=OSTimeGet();
#endif
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect,1);
        if (i != 0)
        {
            pFile->error = -1;
        }
        else
        {
            pFile->error = 0;
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return 0;	//?
        }
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return 0;	//?
        }
#if FILE_SYSTEM_DVF_TEST
        time2=OSTimeGet();
        DEBUG_FS("Delete File Time=%d (x50msec)\n",time2-time1);
#endif
        return 0;
    }
    else if (lexp_b)
    {
        //delete file but file don't exist.
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        pFile->error = -1;
        return 0;
    }
#endif
    // Check read only
    lexp_a = ((i >= 0) && ((s.data[11] & FS_FAT_ATTR_READ_ONLY) != 0)) && ((pFile->mode_w) || (pFile->mode_a) || (pFile->mode_c));
    if (lexp_a)
    {
        // Files is RO and we try to create, write or append. Not allow
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        return -1;
    }
    lexp_a = (i >= 0) && (!pFile->mode_a) &&
             (((pFile->mode_w) && (!pFile->mode_r)) || ((pFile->mode_w) && (pFile->mode_c) && (pFile->mode_r)));
    if (lexp_a)
    {
        // Delete old file
        i = FS__fat_DeleteFileOrDir(pFile->dev_index, unit, realname, dstart, dsize, pFile->FileEntrySect, 1);
        // FileSize = 0
        s.data[28] = 0x00;
        s.data[29] = 0x00;
        s.data[30] = 0x00;
        s.data[31] = 0x00;
        i =- 1;
    }

    if ((!pFile->mode_c) && (i < 0))
    {
        // File does not exist and we don't create
        err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
        }
        return -1;
    }
    else if ((pFile->mode_c) && (i < 0))
    {
        // File does not exist and Create new file
#if FILE_SYSTEM_DVF_TEST
        u32 time1, time2;
        time1 = OSTimeGet();
#endif
        i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize, &EntSect, &EntSectOffset, &fileClust);
#if FILE_SYSTEM_DVF_TEST
        time2 = OSTimeGet();
        time1 = time1;
        time2 = time2;
        DEBUG_FS2("--->_FS_fat_create_file = %d (x50msec)\n",time2-time1);
#endif

		switch(i)
		{
			case FS_FILE_NAME_REPEAT_ERR:
				return -1;
				
			case -2:
				DEBUG_FS("File Entry Full. Create new one.\n");
				// Directory is full, try to increase
                if((i = FSFATIncEntry(pFile->dev_index, unit, dstart, 1, &dsize, FS_E_CLUST_CLEAN_ON)) >= 0)
 					i = _FS_fat_create_file(pFile->dev_index, unit, realname, dstart, dsize, &EntSect, &EntSectOffset, &fileClust);
 			default:
 				if (i < 0)
	            {
	                err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
	                if(err < 0)
	                    ERRD(FS_DEV_IOCTL_ERR);
	                return -1;
	            }
	            DEBUG_FS("[INF] EntSect = (%d, %d)\n", EntSect, EntSectOffset);
	            break;
		}
    }
    else
    {
        pFile->EOFClust = 0;//   檔案在,append
    }

    pFile->fileid_lo = unit;	// unit whre file is located.
    pFile->fileid_hi = fileClust;	// 1st clust of file location.(Low byte)
    pFile->fileid_ex = dstart;	// dstart is the start cluster of directory
    pFile->FileEntrySect = EntSect;
    pFile->CurClust = 0;	// 未知
    pFile->EOFClust = fileClust;
    pFile->error = 0;	// 未知
    // FileSize
    pFile->size = s.data[28] + 0x100UL * s.data[29] + 0x10000UL * s.data[30] + 0x1000000UL * s.data[31];
    if (pFile->mode_a)
    {
        pFile->filepos = pFile->size; //從檔案尾開始接
        if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, fileClust, 0, &fileClust)) < 0)
        {
         ERRD(FS_FAT_CLUT_FIND_ERR);
            return err;
        }
        pFile->EOFClust = fileClust;
    }
    else
        pFile->filepos = 0;//filepos: 是從檔案頭開始算
    pFile->inuse = 1;
    return 1;
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
    FS__fat_dentry_type s;
    u32 unit, dstart, dsize, dsec;
    int result;
    char *fname, realname[12];

    //Lucian: 找到該目錄下(Ex. unit:\\DCIM\\100VIDEO\\xxxxxxx.asf) 的start cluster(dstart),該目錄佔了幾個sector (dsize).
    result = FS__fat_findpath(Idx, pFileName, &fname, &unit, &dstart, &dsize);
    if (result < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", Idx, pFileName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return result;  // Directory not found
    }
    FS__fat_make_realname(realname, fname);  // Convert name to FAT real name,將小寫字母換成大寫,不足八個字元以 space 補足之

    result = _FS_fat_find_file(Idx, unit, realname, &s, dstart, dsize, &dsec);
    if(result < 0)
    {
        ERRD(FS_FILE_FIND_ERR);
        return result;
    }

    return dsec;

}

#endif
