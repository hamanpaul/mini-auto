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
File        : fat_dir.c
Purpose     : POSIX 1003.1 like directory support
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
**********************************************************************/


#include "fsapi.h"
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
#include "dcfapi.h"




#if FS_POSIX_DIR_SUPPORT
/*********************************************************************
*
*             Global Variables
*
**********************************************************************/


/*********************************************************************
*
*             Extern Global Variables
*
**********************************************************************/
extern u8 gInsertNAND;

/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/
extern void FS__fat_ResetFileEntrySect(int sect,int offset,int index);
extern u8 dcfGetFileType(char* pcsubfilename);

/*********************************************************************
*
*             _FS_fat_create_directory
*
  Description:
  FS internal function. Create a directory in the directory specified
  with DirStart. Do not call, if you have not checked before for
  existing directory with name pDirName.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number, which is passed to the device driver.
  pDirName    - Directory name.
  DirStart    - Start of directory, where to create pDirName.
  DirSize     - Size of the directory starting at DirStart.

  Return value:
  >=0         - Directory has been created.
  <0          - An error has occured.
*/
static int _FS_fat_create_directory(int Idx, u32 Unit, const char *pDirName, u32 DirStart, u32 DirSize)
{
    FS__fat_dentry_type *s;
    RTC_DATE_TIME localTime;
    u32 dirindex, dsec, dsec_temp;
    u32 cluster, Size;
    u16 val_time, val_date;
    int err, len, j;
    char *buffer;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    len = FS__CLIB_strlen(pDirName);
    if (len > 11)
    {
        len = 11;
    }
    // Read directory
    for (dirindex = 0; dirindex < DirSize; dirindex++)
    {
        err = FS__fat_dir_realsec(Idx, Unit, DirStart, dirindex, &dsec);
        if (err < 0)
        {
            // Translation of relativ directory sector to an absolute sector failed
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); // Read directory sector
        if (err < 0)
        {
            // Read error
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }
        // Scan the directory sector for a free or deleted entry
        s = (FS__fat_dentry_type*)buffer;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }
            if (s->data[0] == 0x00)
            {
                break;  // Found a free entry
            }
            // Add NAND 0xFF break events  civicfs
            if (gInsertNAND==1)
            {
                if (s->data[0] == (u8)0xFF)
                {
                    break;  // Found a free entry in NAND FDB
                }
            }
            s++;
        }
        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            /* Free entry found. Make entry and return 1st block of the file. */
            FS__CLIB_strncpy((char*)s->data, pDirName, len);
            s->data[11] = FS_FAT_ATTR_DIRECTORY;
            if((err = FSFATNewEntry(Idx, Unit, &cluster, 1, &Size, FS_E_CLUST_CLEAN_ON)) < 0)
            {
                ERRD(FS_FAT_CLUT_ALLOC_ERR);
                FS__fat_free(buffer);
                return err;
            }
            if (cluster > 0)
            {
                s->data[12] = 0x00;	// Res
                s->data[13] = 0x00;	// CrtTimeTenth (optional, not supported)
                RTC_Get_Time(&localTime);
                val_time = ((localTime.hour & 0x1F) << 11) |((localTime.min & 0x3F) << 5) |((localTime.sec/2) & 0x1F);
                val_date = (((localTime.year + 20) & 0x7F) << 9) |((localTime.month & 0xF) << 5) |((localTime.day) & 0x1F);
                s->data[14] = (u8)(val_time & 0xff);	// WrtTime, CrtTime (optional, not supported)
                s->data[15] = (u8)(val_time / 256);
                s->data[16] = (u8)(val_date & 0xff);	// WrtDate, CrtDate (optional, not supported)
                s->data[17] = (u8)(val_date / 256);
                s->data[18] = 0x00;						// LstAccDate (optional, not supported)
                s->data[19] = 0x00;
                s->data[22] = (u8)(val_time & 0xff);	// WrtTime
                s->data[23] = (u8)(val_time / 256);
                s->data[24] = (u8)(val_date & 0xff);	// WrtDate
                s->data[25] = (u8)(val_date / 256);
                s->data[26] = (u8)(cluster & 0xff);		// FstClusLo / FstClusHi
                s->data[27] = (u8)((cluster / 256) & 0xff);
                s->data[20] = (u8)((cluster / 0x10000L) & 0xff);
                s->data[21] = (u8)((cluster / 0x1000000L) & 0xff);
                s->data[28] = 0x00;						// FileSize
                s->data[29] = 0x00;
                s->data[30] = 0x00;
                s->data[31] = 0x00;

                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); // Write the modified directory sector
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    return err;
                }

                // Sub directory, Clear new directory and make '.' and '..' entries
                FS__CLIB_memset(buffer, 0x00, (u32) FS_FAT_SEC_SIZE);
                err = FS__fat_dir_realsec(Idx, Unit, cluster, 0, &dsec); /* Find 1st absolute sector of the new directory */
                //DEBUG_CYAN(". cluster: %#x, dsec: %#x\n", cluster, dsec);
                if (err < 0)
                {
                    ERRD(FS_FAT_SEC_CAL_ERR);
                    FS__fat_free(buffer);
                    return err;
                }
                dsec_temp = dsec;

                // clear all sectors of one cluster first for performance improvement
                // for performance improvement, use a buffer with one page size

                // loop to clear single sector
                for (j = 0; j < FS__FAT_aBPBUnit[Idx][Unit].SecPerClus; j++)
                {
                    err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec_temp++, (void*)buffer);
                    if (err < 0)
                    {
                        ERRD(FS_REAL_SEC_WRTIE_ERR);
                        FS__fat_free(buffer);
                        return err;
                    }
                }
                // Make "." entry
                s = (FS__fat_dentry_type*)buffer;
                FS__CLIB_strncpy((char*)s->data, ".          ", 11);
                s->data[11] = FS_FAT_ATTR_DIRECTORY;
                s->data[22] = (u8)(val_time & 0xff);   // WrtTime
                s->data[23] = (u8)(val_time / 256);
                s->data[24] = (u8)(val_date & 0xff);   // WrtDate
                s->data[25] = (u8)(val_date / 256);
                s->data[26] = (u8)(cluster & 0xff);    // FstClusLo / FstClusHi
                s->data[27] = (u8)((cluster / 256) & 0xff);
                s->data[20] = (u8)((cluster / 0x10000L) & 0xff);
                s->data[21] = (u8)((cluster / 0x1000000L) & 0xff);
                // Make entry ".."
                s++;
                FS__CLIB_strncpy((char*)s->data, "..         ", 11);
                s->data[11] = FS_FAT_ATTR_DIRECTORY;
                s->data[22] = (u8)(val_time & 0xff);	// WrtTime
                s->data[23] = (u8)(val_time / 256);
                s->data[24] = (u8)(val_date & 0xff);	// WrtDate
                s->data[25] = (u8)(val_date / 256);
                s->data[26] = (u8)(DirStart & 0xff);	// FstClusLo / FstClusHi
                s->data[27] = (u8)((DirStart / 256) & 0xff);
                s->data[20] = (u8)((DirStart / 0x10000L) & 0xff);
                s->data[21] = (u8)((DirStart / 0x1000000L) & 0xff);
                // Write "." & ".." entries into the new directory
                err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer);
                if (err < 0)
                {
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
                    FS__fat_free(buffer);
                    return err;
                }
                FS__fat_free(buffer);
                return 1;
            }
            ERRD(FS_FAT_CLUT_LINK_ERR);
            FS__fat_free(buffer);
            return -1;
        }
    }
    FS__fat_free(buffer);
    return -2;  // Directory is full
}


/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_opendir
*
  Description:
  FS internal function. Open an existing directory for reading.

  Parameters:
  pDirName    - Directory name.
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  ==0         - Unable to open the directory.
  !=0         - Address of an FS_DIR data structure.
*/

int FS__fat_opendir(const char *pDirName, FS_DIR *pDir)
{
    u32 len;
    u32 unit, dstart, dsize, dsec;
    char realname[12], *filename;
    int err;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    // Find path on the media and return file name part of the complete path
    err = FS__fat_findpath(pDir->dev_index, pDirName, &filename, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", pDir->dev_index, pDirName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Directory not found
    }
    if(FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0) < 0)	// Turn on busy signal
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }

    len = FS__CLIB_strlen(filename);
    if (len != 0)
    {
        // There is a name in the complete path (it does not end with a '\')
        FS__fat_make_realname(realname, filename);  // Convert name to FAT real name
        err =  FS__fat_find_dir(pDir->dev_index, unit, realname, dstart, dsize, &dsec);  // Search name in the directory
        // i is the first cluster of file
        if (err < 0)
        {
            // Directory not found
            ERRD(FS_DIR_FIND_ERR);
            if(FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
                ERRD(FS_DEV_IOCTL_ERR);
            return err;
        }
    }
    else
    {
        // There is no name in the complete path (it does end with a '\'). In that
        //   case, FS__fat_findpath returns already start of the directory.
        dsec = dstart;  // Use 'current' path
    }

    err = FS__fat_dir_size(pDir->dev_index, unit, dsec, &dsize);  // Get size of the directory
    if (err < 0)
    {
        ERRD(FS_DIR_FIND_ERR);
        // Directory not found
        if(FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
            ERRD(FS_DEV_IOCTL_ERR);
        return err;
    }
    pDir->dirid_lo = unit;
    pDir->dirid_hi = dsec;
    pDir->dirid_ex = dstart;	// start cluster of the  parent directory.
    pDir->error = 0;
    pDir->size = dsize;	// the size of
    pDir->dirpos = 0;
    pDir->inuse = 1;
    return 1;
}


/*********************************************************************
*
*             FS__fat_closedir
*
  Description:
  FS internal function. Close a directory referred by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  ==0         - Directory has been closed.
  ==-1        - Unable to close directory.
*/

int FS__fat_closedir(FS_DIR *pDir)
{
    int err;
    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    err = FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;
    }
    pDir->inuse = 0;
    return 1;
}


/*********************************************************************
*
*             FS__fat_readdir
*
  Description:
  FS internal function. Read next directory entry in directory
  specified by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  ==0         - No more directory entries or error.
  !=0         - Pointer to a directory entry.
*/

int FS__fat_readdir(FS_DIR *pDir, FS_DIRENT *pDirEnt)
{
    FS__fat_dentry_type *s;
    u32 dirindex, dsec;
    u16 bytespersec;
    char *buffer;
    int err;
    //added by Albert Lee on 20090522
    u32 unTemp;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    dirindex = pDir->dirpos / bytespersec;
    while (dirindex < (u32)pDir->size)
    {
        err = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, dirindex, &dsec);
        if (err < 0)
        {
            // Cannot convert logical sector
            ERRD(FS_FAT_SEC_CAL_ERR);
            FS__fat_free(buffer);
            return err;
        }
        // Read directory sector
        err = FS__lb_sin_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(buffer);
            return err;
        }
        // Scan for valid directory entry
        s = (FS__fat_dentry_type*)&buffer[pDir->dirpos % bytespersec];
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (u8)0xe5)
                {
                    /* not a deleted file */
                    if (s->data[11] != (FS_FAT_ATTR_READ_ONLY | FS_FAT_ATTR_HIDDEN | FS_FAT_ATTR_SYSTEM | FS_FAT_VOLUME_ID))
                    {
                        break;  /* Also not a long entry, so it is a valid entry */
                    }
                }
            }
            s++;
            pDir->dirpos += 32;
        }

        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            /* Valid entry found, copy it.*/
            pDir->dirpos += 32;
            FS__CLIB_memcpy(pDir->dirent.d_name, s->data, 8);
            pDir->dirent.d_name[8] = '.';
            FS__CLIB_memcpy(&pDir->dirent.d_name[9], &s->data[8], 3);
            pDir->dirent.d_name[12] = 0;
            pDir->dirent.FAT_DirAttr = s->data[11];
#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
            pDir->dirent.used_flag=0xAA;
#endif
            //added by Albert Lee on20090522
            //begin
            //---create time---//
            unTemp = s->data[14] + (s->data[15] << 8);
            pDir->dirent.fsFileCreateTime_HMS = unTemp;


            unTemp = s->data[16] + (s->data[17] << 8);
            pDir->dirent.fsFileCreateDate_YMD = unTemp;

            //---modified time--//
            unTemp = s->data[22] + (s->data[23] << 8);
            pDir->dirent.fsFileModifiedTime_HMS = unTemp;

            unTemp = s->data[24] + (s->data[25] << 8);
            pDir->dirent.fsFileModifiedDate_YMD = unTemp;

            //end
            *pDirEnt = pDir->dirent;
            FS__fat_free(buffer);
            return 1;
        }
        dirindex++;
    }
    FS__fat_free(buffer);
    return -1;
}




/*********************************************************************
*
*             FS__fat_readwholedir
*
  Description:
  FS internal function. Read whole directory entry in directory
  specified by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.
  dst_DirEnt - Pointer to destination dcf dir entry or file entry
  buffer      - DCF buffer for parsing FDB
  DirEntMax   -
  Return value:
  ==0         - No more directory entries or error.
  !=0         - Pointer to a directory entry.
*/

int FS__fat_readwholedir(FS_DIR *pDir,FS_DIRENT *dst_DirEnt, u8* buffer, unsigned int DirEntMax,
                         DEF_FILEREPAIR_INFO *pdcfBadFileInfo, u8 IsUpdateEntrySect, int DoBadFile)
{
    FS__fat_dentry_type *s;
    u32 clustNum, clustCount, dsec = 0;
    u16 bytesPerSec, secPerClus;

    u16 sect, offset;
    u16 numEntry = 0;

    u32 unTemp;
    s32 cluster;
    s32 result;
    int err;
    u8 WarmingFlag;
    //
    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    if (!buffer)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    WarmingFlag = 0;
    clustCount = 0;
    bytesPerSec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    secPerClus = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;
    bytesPerSec = bytesPerSec;	// avoid warning message
    //
    for (sect = 0; sect < pDir->size; sect++)
    {
        if((sect % secPerClus)==0) //Lucian: optimize here..
        {
        	if(WarmingFlag)
        	{
        		DEBUG_FS("[W] Entry end or over. %d\n", __LINE__);
        		return -1;
        	}
            err = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect, &dsec);
        }
        else
            dsec +=1;
        if (err < 0)
        {
            // Cannot convert logical sector
            ERRD(FS_FAT_SEC_CAL_ERR);
            return err;
        }

        err = FS__lb_sin_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            return err;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }

            if(DoBadFile)
            {
#if FILE_REPAIR_AUTO //Repair Bad file 
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Repair:%d \n",cluster);

                    err=_FS__fat_ScanFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster,&size);
                    if(err<0)
                    {
                        DEBUG_FS("Warnging! Scan file-link error!\n");
                    }
                    if ( (cluster != 0) && (err >=0))
                    {
                        if( (s->data[8]=='A') && (s->data[9]=='S')&& (s->data[10]=='F') )
                            s->data[9]='X'; //Lucian: 將ASF 壞檔更改副檔名為 .AXF

                        s->data[28]=size & 0x0ff;
                        s->data[29]=(size>>8) & 0x0ff;
                        s->data[30]=(size>>16) & 0x0ff;
                        s->data[31]=(size>>24) & 0x0ff;

                        RTC_Get_Time(&localTime);
                        val=((localTime.hour& 0x1F )<<11) |((localTime.min	& 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

                        s->data[22] = (u8)(val & 0xff);
                        s->data[23] = (u8)((val / 0x100) & 0xff);

                        val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

                        s->data[24] = (u8)(val & 0xff);
                        s->data[25] = (u8)((val / 0x100) & 0xff);

                        err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                        if (err < 0)
                        {
                            ERRD(FS_REAL_SEC_WRTIE_ERR);
                            DEBUG_FS("Delete file-FDB error!\n");
                            return err;	//?
                        }
                    }
                    else
                    {
                        s->data[0]= (u8)0xe5;
                        DEBUG_FS("Repair Fail and Delete!\n");

                        err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                        if (err < 0)
                        {
                            ERRD(FS_REAL_SEC_WRTIE_ERR);
                            DEBUG_FS("Delete file-FDB error!\n");
                            return err;	//?
                        }

                        s++;
                        offset ++;
                        continue;
                    }
                }
#else //Delete Bad file
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) && (s->data[0] != 0xe5) && (s->data[0] != 0x2e))
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        if((err = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, cluster)) < 0)
                            DEBUG_FS("Delete file-link error!\n");
                    }

                    s->data[0]= (u8)0xe5;
                    err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                    if (err < 0)
                    {
                        ERRD(FS_REAL_SEC_WRTIE_ERR);
                        DEBUG_FS("Delete file-FDB error!\n");
                        return err;	//?
                    }
                    //===============//
                    s++;
                    offset ++;
                    continue;
                }
#endif

                //Scan Directory. if find error directory, delete it//
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_DIRECTORY) &&
                        (s->data[0] != 0x00) && (s->data[0] != 0xe5) && (s->data[0] != 0x2e))
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    result = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, &clustCount, &clustNum);
                    if(result < 0) //Cannot find EOF
                    {
                        ERRD(FS_FAT_EOF_FIND_ERR);
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            if((err = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, cluster)) < 0)
                                DEBUG_FS("Delete file-link error!\n");
                        }
                        s->data[0]= (u8)0xe5;

                        err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                        if (err < 0)
                        {
                            ERRD(FS_REAL_SEC_WRTIE_ERR);
                            DEBUG_FS("Delete file-FDB error!\n");
                            return err;	//?
                        }
                        s++;
                        offset++;
                        continue;
                    }
                }
            }

            if(s->data[11] & 0xc0)
            {
            	DEBUG_FS("[W] Entry item attribute.\n");
            	WarmingFlag++;
            }

            if(s->data[0] == 0x0)
            {
            	//DEBUG_FS("[I] Entry end.\n");
            	//return numEntry;
            	WarmingFlag++;
            }
            else if(s->data[0] != (u8)0xe5)
            {
                /* not an empty entry */
                if(s->data[11] != 0x00)
                {
                    /* not a deleted file */
                    if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0) || (s->data[11] != FS_FAT_ATTR_ARCHIVE) )
                    {
                        if (s->data[11] != (FS_FAT_ATTR_READ_ONLY | FS_FAT_ATTR_HIDDEN | FS_FAT_ATTR_SYSTEM | FS_FAT_VOLUME_ID))
                        {
                            /* Also not a long entry, so it is a valid entry */
                            numEntry++;

                            if (numEntry>=DirEntMax)
                            {
                                return numEntry;
                            }

                            //----檔案名稱---//
                            FS__CLIB_memcpy(dst_DirEnt->d_name, s->data, 8);
                            dst_DirEnt->d_name[8] = '.';
                            FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &s->data[8], 3);
                            dst_DirEnt->d_name[12] = 0;

                            //---檔案屬性---//
                            dst_DirEnt->FAT_DirAttr = s->data[11];

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
                            dst_DirEnt->used_flag=0xAA;
#endif

                            //---記錄建檔時間---//
                            unTemp = s->data[14] + (s->data[15]<<8);
                            dst_DirEnt->fsFileCreateTime_HMS=unTemp;

                            unTemp = s->data[16] + (s->data[17]<<8);
                            dst_DirEnt->fsFileCreateDate_YMD=unTemp;

                            //---記錄最後修改時間---//
                            unTemp = s->data[22] + (s->data[23]<<8);
                            dst_DirEnt->fsFileModifiedTime_HMS=unTemp;

                            unTemp = s->data[24] + (s->data[25]<<8);
                            dst_DirEnt->fsFileModifiedDate_YMD=unTemp;

                            //-- 紀錄File entry location on DIR--//
                            dst_DirEnt->FileEntrySect=sect;

                            //if(IsUpdateEntrySect)
                            //FS__fat_ResetFileEntrySect(sect,offset,pDir->dev_index);
                            //DEBUG_FS("sect=%d\n",sect);

                            dst_DirEnt++;     // Forward address
                        }
                    }
                }
            }
            s++;;
            offset ++;
        }
    }
    return numEntry;
}

/*
*
*             FS__fat_ScanWholedir
*
  Description:
  FS internal function. Read whole directory entry in directory
  specified by pDir. Scan out oldest entry.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.
  dst_DirEnt - Pointer to destination dcf dir entry or file entry
  buffer      - DCF buffer for parsing FDB
  pOldestEntry- Oldest file entry position.
  DirEntMax   -
  Return value:
                 Total entry number. if error, return -1.

*/
int FS__fat_ScanWholedir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8 *buffer, u32 DirEntMax, u32 *pOldestEntry, int DoBadFile)
{
    FS__FAT_BPB *pBPBUnit;
    FS_FAT_ENTRY *pEntry;
    u32 i, j;
    u32 CurCluster, CurSector, TmpVal, NumOfEntrys;
    u16 TimeVal;
    int ret;
    u8 WarmingFlag;
    u8 *pClustDataBuf, *ppClustDataBuf;
    char prev_d_name[9]= {'9','9','9','9','9','9','9','9','\0'};
    char curr_d_name[9]= {0};
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo];
    //
    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    if (!buffer)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }

    // Set the Start cluster address.
    CurCluster = pDir->dirid_hi;
    // The cluster link of Dir is shorter than file's clusters usually.
    if((ret = FSFATGoForwardCluster(pDir->dev_index, pDir->dirid_lo, CurCluster, 1, &TmpVal)) < 0)
    {
        ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
        return ret;
    }

	if(CurCluster != TmpVal)
    {
        ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
    	return -1;
    }

    i = 0;
    WarmingFlag = 0;
    NumOfEntrys = 0;
    pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus);
    do
    {
        if(CurCluster == 0x0)
        {
            ERRD(FS_FAT_CLUT_SHIFT_ERR);
            FSFree(pClustDataBuf);
            return -1;
        }

        if(FSFATCalculateSectorByCluster(pDir->dev_index, pDir->dirid_lo, &CurCluster, &CurSector) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustDataBuf);
            return -1;
        }

        ret = FS__lb_mul_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, CurSector,	pBPBUnit->SecPerClus, pClustDataBuf);
        if(ret < 0)
        {
            ERRD(FS_LB_MUL_READ_DAT_ERR);
            FSFree(pClustDataBuf);
            return ret;
        }

        ppClustDataBuf = pClustDataBuf;
        for(j = 0; j < pBPBUnit->SecPerClus; j++)
        {
            pEntry = (FS_FAT_ENTRY *)ppClustDataBuf;
            do
            {
                if((NumOfEntrys >= DirEntMax))
                {
                    FSFree(pClustDataBuf);
                    return NumOfEntrys;
                }
            	
                if(DoBadFile)	// Delete Bad file
                {
                    if(((pEntry->data[0] != 0x2e) && (pEntry->data[0] != FS_V_FAT_ENTRY_DELETE)) &&
                            ((pEntry->data[28] + pEntry->data[29] + pEntry->data[30] + pEntry->data[31]) == 0))
                    {
                        TmpVal = (pEntry->data[21] << 24) + (pEntry->data[20] << 16) + (pEntry->data[27] << 8) + pEntry->data[26];
                        switch(pEntry->data[11])
                        {
                            case FS_FAT_ATTR_DIRECTORY:
                                if(FSFATLWScanClusterLink(pDir->dev_index, pDir->dirid_lo, TmpVal) >= 0)
                                    break;
                                ERRD(FS_FAT_EOF_FIND_ERR);
                            case FS_FAT_ATTR_ARCHIVE:
                                DEBUG_FS("[INF] FS bad entry detect and delete: %#x\n", TmpVal);
                                if(TmpVal != 0x0)
                                {
                                    if((ret = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, TmpVal)) < 0)
                                        DEBUG_FS("[ERR] FS delete file link error.\n");
                                }

                                pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;

                                if((ret = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, CurSector + j, ppClustDataBuf)) < 0)
                                {
                                    ERRD(FS_LB_WRITE_DAT_ERR);
                                    FSFree(pClustDataBuf);
                                    return ret;
                                }
                                break;
                            default:
                                break;
                        }
                        pEntry++;
                        continue;
                    }
                }

				// Warming check
                if(pEntry->data[0] == 0x0)
					WarmingFlag++;

				if(pEntry->data[11] & 0xc0)
	            {
	            	DEBUG_FS("[W] Entry item attribute.\n");
	            	WarmingFlag++;
	            }
	            

                if((pEntry->data[0] != 0x2E) && (pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) &&
                        (pEntry->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        ((pEntry->data[28] + pEntry->data[29] + pEntry->data[30] + pEntry->data[31]) != 0))
                {
                    char *dP = (char *) &pEntry->data[8];
                    if(dcfGetFileType(dP) != DCF_FILE_TYPE_MAX)
                    {
                        //----檔案名稱---//
                        FS__CLIB_memcpy(dst_DirEnt->d_name, pEntry->data, 8);
                        dst_DirEnt->d_name[8] = '.';
                        FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &pEntry->data[8], 3);
                        dst_DirEnt->d_name[12] = 0;

                        //---檔案屬性---//
                        dst_DirEnt->FAT_DirAttr = FS_FAT_ATTR_ARCHIVE;

                        memcpy(curr_d_name, pEntry->data,8);
                        if(strcmp(curr_d_name, prev_d_name)<0)
                        {
                            *pOldestEntry = NumOfEntrys;
                            memcpy(prev_d_name, curr_d_name,8);
                        }

                        dst_DirEnt->used_flag = DCF_FILE_USE_CLOSE;

                        //---記錄建檔時間---//
                        TimeVal = pEntry->data[14] + (pEntry->data[15] << 8);
                        dst_DirEnt->fsFileCreateTime_HMS = TimeVal;

                        TimeVal = pEntry->data[16] + (pEntry->data[17] << 8);
                        dst_DirEnt->fsFileCreateDate_YMD = TimeVal;

                        //---記錄最後修改時間---//
                        TimeVal = pEntry->data[22] + (pEntry->data[23] << 8);
                        dst_DirEnt->fsFileModifiedTime_HMS = TimeVal;

                        TimeVal = pEntry->data[24] + (pEntry->data[25] << 8);
                        dst_DirEnt->fsFileModifiedDate_YMD = TimeVal;

                        //-- 紀錄File entry location on DIR--//
                        dst_DirEnt->FileEntrySect = i * pBPBUnit->SecPerClus + j;	// Cluster number + Offset Secter
                        dst_DirEnt->FileEntrySectOffset = (((u8 *)pEntry) - ppClustDataBuf) / FS_FAT_DENTRY_SIZE;
                      
                        dst_DirEnt++;
                        NumOfEntrys++;
                    }
                }
                pEntry++;
            }
            while((u8 *)pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
            ppClustDataBuf += pBPBUnit->BytesPerSec;
        }

		// The cluster link of Dir is shorter than file's clusters usually.
		if((ret = FSFATGoForwardCluster(pDir->dev_index, pDir->dirid_lo, CurCluster, 2, &TmpVal)) < 0)
		{
			ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
			FSFree(pClustDataBuf);
			return ret;
    }

		if(CurCluster == TmpVal)
		{
			//DEBUG_FS("[I] Entry end.\n");
			break;
    	}

    	if(WarmingFlag)
    	{
    		DEBUG_FS("[W] Entry end or over. %d\n", __LINE__);
    		FSFree(pClustDataBuf);
			return -1;
    }

		CurCluster = TmpVal;
		i++;
	}
	while(1);

    FSFree(pClustDataBuf);
    return NumOfEntrys;
}

int FS__fat_FetchItems(FS_DIR *pDir, FS_DIRENT *pDstEnt, FS_SearchCondition *pCondition)
{
    FS__FAT_BPB *pBPBUnit;
    FS_FAT_ENTRY *pEntry;
    u32 CurClust, CurSect, Found;
    u32 CreateTime, ModifyTime;
    u32 i, j, TmpVal, WarmingFlag;
    int ret;
    u8 *pClustData, *pSLClustData;
    //
    if(pDir == NULL)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    if(pDstEnt == NULL)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    if(pCondition == NULL)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo];
    CurClust = pDir->dirid_hi;  // Set the Start cluster address of Parent dir.

    // The cluster link of Dir is shorter than file's clusters usually. Check the original cluster whether is exist or not.
    if((ret = FSFATGoForwardCluster(pDir->dev_index, pDir->dirid_lo, CurClust, 1, &TmpVal)) < 0)
    {
        ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
        return ret;
    }

    if(CurClust != TmpVal)
    {
        ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
        return -1;
    }

    // reset parameters before search action.
    pCondition->CntOfResult = 0;
    if((pClustData = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    for(i = 0, Found = 0, WarmingFlag = 0; ; i++)
    {
        if(CurClust == 0x0)
        {
            ERRD(FS_FAT_CLUT_SHIFT_ERR);
            FSFree(pClustData);
            return -1;
        }

        if(FSFATCalculateSectorByCluster(pDir->dev_index, pDir->dirid_lo, &CurClust, &CurSect) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustData);
            return -1;
        }

        if((ret = FS__lb_mul_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, CurSect, pBPBUnit->SecPerClus, pClustData)) < 0)
        {
            ERRD(FS_LB_MUL_READ_DAT_ERR);
            FSFree(pClustData);
            return ret;
        }

        pSLClustData = pClustData;
        for(j = 0; j < pBPBUnit->SecPerClus; j++)
        {
            for(pEntry = (FS_FAT_ENTRY *)pSLClustData; ((u8 *)pEntry) < (pSLClustData + pBPBUnit->BytesPerSec); pEntry++)
            {
                switch(pEntry->data[0])
                {
                    case 0x0:
                        DEBUG_FS("[I] Entry end. %d\n", pCondition->CntOfResult);
                        FSFree(pClustData);
                        return pCondition->CntOfResult;

                    case 0x2E:  // '.'
                    case 0xE5:
                        continue;

                    default:
                        break;
                }

                // Handle rule of bad files. Bad file: ModifyTime is empty.
                if(pCondition->BadFileAction > 0)
                {
                    ret = 1;
                    switch(pEntry->data[0xB])
                    {
                        case FS_FAT_ATTR_ARCHIVE:
                            if(*(u16 *) (pEntry->data + 0x16) == 0)
                                ret = 0;
                            break;

                        case FS_FAT_ATTR_DIRECTORY:
                        default:
                            break;
                    }

                    if(ret == 0)
                    {
                        TmpVal = (*(u16 *)(pEntry->data + 0x14) << 16) + (*(u16 *)(pEntry->data + 0x1A));
                        ERRD(FS_FAT_EOF_FIND_ERR);
                        if((ret = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, TmpVal)) < 0)
                        {
                            DEBUG_FS("[ERR] FS delete file link error.\n");
                        }

                        pEntry->data[0] = 0xE5;
                        pEntry->data[0xB] = 0x0;

                        if((ret = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, CurSect + j, pSLClustData)) < 0)
                        {
                            ERRD(FS_LB_WRITE_DAT_ERR);
                            FSFree(pClustData);
                            return ret;
                        }

                        // Delete action finished.
                        continue;
                    }
                }

                if(pEntry->data[11] & 0xc0)
                {
                    DEBUG_FS("[W] FS attribute of entry item can't recognize.\n");
                    WarmingFlag++;
                }

                if((pEntry->data[11] & pCondition->EntryType) == 0)
                    continue;

                if(pCondition->FilterEnable > 0)
                {
                    if(dcfFilterNameFormat((const char *)pEntry->data, pCondition->SearchMode, pCondition->NameTypeMask) != 1)
                        continue;

                    switch(pCondition->EntryType)
                    {
                        case FS_FAT_ATTR_ARCHIVE:
                            if(dcfCheckFileChannel(pCondition->ChannelMap, pEntry->data) == 1)
                                break;
                            continue;

                        case FS_FAT_ATTR_DIRECTORY:
                        default:
                            break;
                    }

                    if(pCondition->StartTime || pCondition->EndTime)    // 0 = ALL.
                    {
                        CreateTime = *(u16 *) (pEntry->data + 0xE);
                        CreateTime = ((((CreateTime >> 11) * 60 + ((CreateTime & 0x7E0) >> 5))) * 60) + ((CreateTime & 0x1F) << 1);
                        ModifyTime = *(u16 *) (pEntry->data + 0x16);
                        ModifyTime = ((((ModifyTime >> 11) * 60 + ((ModifyTime & 0x7E0) >> 5))) * 60) + ((ModifyTime & 0x1F) << 1);

                        // Check Create time only.
                        if(!((pCondition->StartTime <= CreateTime) && (CreateTime <= pCondition->EndTime)))
                            continue;
                    }
                }

                if(pCondition->CntOfResult < pCondition->LimitOfEntSize)
                {
                    if(pCondition->StartNumOfInOrder <= Found)
                    {
                        pDstEnt->FAT_DirAttr = pEntry->data[0xB];
                        switch(pDstEnt->FAT_DirAttr)
                        {
                            case FS_FAT_ATTR_ARCHIVE:
                                // MARS RULE, Modified time = 0, ignored.
                                if(*(u16 *) (pEntry->data + 0x16) == 0)
                                    continue;

                                FS__CLIB_memcpy(pDstEnt->d_name, pEntry->data, 8);
                                pDstEnt->d_name[8] = '.';
                                FS__CLIB_memcpy(&pDstEnt->d_name[9], &pEntry->data[8], 3);
                                pDstEnt->d_name[12] = '\0';
                                break;

                            case FS_FAT_ATTR_DIRECTORY:
                            default:
                                FS__CLIB_memcpy(pDstEnt->d_name, pEntry->data, 0xB);
                                pDstEnt->d_name[0xB + 1] = '\0';
                                break;
                        }

                        pDstEnt->used_flag = DCF_FILE_USE_CLOSE;

                        pDstEnt->FileEntrySect = (i * pBPBUnit->SecPerClus) + j;    // Cluster number + Offset Secter
                        pDstEnt->FileEntrySectOffset = (((u8 *)pEntry) - pSLClustData) / FS_FAT_DENTRY_SIZE;

                        pDstEnt->fsFileCreateTime_HMS = *(u16 *) (pEntry->data + 0xE);
                        pDstEnt->fsFileCreateDate_YMD = *(u16 *) (pEntry->data + 0x10);
                        pDstEnt->fsFileModifiedTime_HMS = *(u16 *) (pEntry->data + 0x16);
                        pDstEnt->fsFileModifiedDate_YMD = *(u16 *) (pEntry->data + 0x18);

                        pCondition->CntOfResult++;
                        //DEBUG_FS("[I] Item name: %s, Duration: %d min.\n", pDestEnt->EntryName, ModifyTime - CreateTime);

                        if(pCondition->LightWeightSearch > 0)
                        {
                            FSFree(pClustData);
                            return pCondition->CntOfResult;
                        }

                        pDstEnt++;
                    }
                    Found++;
                }
                else
                {
                    DEBUG_FS("[I] FS entry collect over. %d\n", pCondition->CntOfResult);
                    FSFree(pClustData);
                    return pCondition->CntOfResult;
                }
            }
            // Move to next round.
            pSLClustData += pBPBUnit->BytesPerSec;
        }

        // The cluster link of Dir is shorter than file's clusters usually.
        if((ret = FSFATGoForwardCluster(pDir->dev_index, pDir->dirid_lo, CurClust, 2, &TmpVal)) < 0)
        {
            ERRD(FS_FAT_FIND_EOF_CLUS_ERR);
            FSFree(pClustData);
            return ret;
        }

        if(CurClust == TmpVal)
        {
            //DEBUG_FS("[I] Entry end.\n");
            break;
        }

        if(WarmingFlag)
        {
            DEBUG_FS("[W] FS entry end or over. %d\n", __LINE__);
            FSFree(pClustData);
            return -1;
        }

        CurClust = TmpVal;
    }

    FSFree(pClustData);
    return pCondition->CntOfResult;
}


int FS__fat_SearchWholedir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry,
                           char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile)
{
    FS__fat_dentry_type *s;
    u32 clustNum, clustCount, dsec = 0;
    u32 SecPerClus;
    u16 sect;
    u32 numEntry = 0;
    u32 CreateTime, unTemp, ModifyTime, Duration;
    s32 cluster;
    s32 result;
    int Offset;
    int hour, min, sec;
    int err;
    char prev_d_name[9] = {'9','9','9','9','9','9','9','9','\0'};
    char curr_d_name[9] = {0};
    u8 WarmingFlag;
    //-----------------//

    //Lucian: Clear Bad File Record.
    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_DIR structure
    }
    if (!buffer)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    SecPerClus = (u32)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;
    clustCount = 0;
    *pOldestEntry = 0;
    WarmingFlag = 0;
    //
    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
        {
        	if(WarmingFlag)
        	{
        		DEBUG_FS("[W] Entry end or over. %d\n", __LINE__);
        		return -1;
        	}
            result = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect, &dsec);
        }
        else
            dsec +=1;
        if (result < 0)
        {
            // Cannot convert logical sector
            ERRD(FS_FAT_SEC_CAL_ERR);
            return result;
        }

        err = FS__lb_sin_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            return err;
        }
        s = (FS__fat_dentry_type*)buffer;
        Offset = 0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  // End of sector reached
            }

            if(DoBadFile) //Delete Bad file
            {
                if(((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) && (s->data[0] != 0xe5) && (s->data[0] != 0x2e))
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        if((err = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, cluster)) < 0)
                            DEBUG_FS("Delete file-link error!\n");
                    }

                    s->data[0]= (u8)0xe5;

                    err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                    if (err < 0)
                    {
                        ERRD(FS_REAL_SEC_WRTIE_ERR);
                        DEBUG_FS("Delete file-FDB error!\n");
                        return 0;	//?
                    }
                    //===============//
                    s++;
                    Offset ++;
                    continue;
                }

                //Scan Directory. if find error directory, delete it//
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_DIRECTORY) &&
                        (s->data[0] != 0x00) && (s->data[0] != 0xe5) && (s->data[0] != 0x2e))
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    result = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, &clustCount, &clustNum);
                    if(result < 0) //Cannot find EOF
                    {
                        ERRD(FS_FAT_EOF_FIND_ERR);
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            if((err = FSFATFreeFATLink(pDir->dev_index, pDir->dirid_lo, cluster)) < 0)
                                DEBUG_FS("Delete file-link error!\n");
                        }
                        s->data[0]= (u8)0xe5;

                        err = FS__lb_sin_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
                        if (err < 0)
                        {
                            ERRD(FS_REAL_SEC_WRTIE_ERR);
                            DEBUG_FS("Delete file-FDB error!\n");
                            return 0;	//?
                        }
                        s++;
                        Offset ++;
                        continue;
                    }
                }
            }

            if(s->data[11] & 0xc0)
            {
            	DEBUG_FS("[W] Entry item attribute.\n");
            	WarmingFlag++;
            }

            if(s->data[0] == 0x0)
            {
            	//DEBUG_FS("[I] Entry end.\n");
            	//return numEntry;
            	WarmingFlag++;
            }
            else if (s->data[0] != (u8)0xe5)
            {
                /* not an empty entry */
                if (s->data[11] != 0x00)
                {
                    /* not a deleted file */
                    if (s->data[11] == FS_FAT_ATTR_ARCHIVE)
                    {
                        if( (s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0 )
                        {
                        	err = 0;
                        	switch(s->data[6])
                        	{
                        		case 'D':
                        			if(Typesel & DCF_DISTRIB_MOTN)
                        				err = 1;
                        			break;
                        		case 'S':
                        			if(Typesel & DCF_DISTRIB_SCHE)
                        				err = 1;
                        			break;
                        		case 'M':
                        			if(Typesel & DCF_DISTRIB_MANU)
                        				err = 1;
                        			break;
                        		case 'R':
                        			if(Typesel & DCF_DISTRIB_RING)
                        				err = 1;
                        			break;
                        		default:
                        			err = 1;
                        			break;
                        	}
                        
                            if(err)
                            {
                                if(dcfCheckFileChannel(CHmap,s->data))
                                {
                                    CreateTime = s->data[14] + (s->data[15]<<8);
                                    hour=CreateTime>>11;
                                    min=(CreateTime & 0x07E0)>>5;
                                    sec= (CreateTime & 0x001F)<<1;
                                    CreateTime=(hour*60+min)*60+sec;

                                    ModifyTime=s->data[22] + (s->data[23]<<8);
                                    hour=ModifyTime>>11;
                                    min=(ModifyTime & 0x07E0)>>5;
                                    sec= (ModifyTime & 0x001F)<<1;
                                    ModifyTime=(hour*60+min)*60+sec;
                                    if(ModifyTime<CreateTime)
                                        ModifyTime += 60*24*60;

                                    Duration = (ModifyTime - CreateTime)/60;

                                    //if( (CreateTime >= StartMin) && (CreateTime<=EndMin) )
                                    if( (ModifyTime >= StartMin*60) && (CreateTime<=EndMin*60) )
                                    {
                                        char *dPointer = (char *)&s->data[8];
                                        if( dcfGetFileType(dPointer) != DCF_FILE_TYPE_MAX )
                                        {
                                            //----檔案名稱---//
                                            FS__CLIB_memcpy(dst_DirEnt->d_name, s->data, 8);
                                            dst_DirEnt->d_name[8] = '.';
                                            FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &s->data[8], 3);
                                            dst_DirEnt->d_name[12] = 0;

                                            //---檔案屬性---//
                                            dst_DirEnt->FAT_DirAttr = FS_FAT_ATTR_ARCHIVE;

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                                            // Depend on FS_DEBUG2
                                            Duration = Duration;
                                            DEBUG_FS2("-->Search DIR: %s, %d min\n", dst_DirEnt->d_name, Duration);
#else
                                            DEBUG_FS("-->Search DIR: %s, %d min\n", dst_DirEnt->d_name, Duration);
#endif
                                            memcpy(curr_d_name,s->data,8);
                                            if(strcmp(curr_d_name, prev_d_name)<0)
                                            {
                                                *pOldestEntry=numEntry;
                                                memcpy(prev_d_name,curr_d_name,8);
                                            }

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
                                            dst_DirEnt->used_flag=0xAA;
#endif
                                            //---記錄建檔時間---//
                                            unTemp = s->data[14] + (s->data[15]<<8);
                                            dst_DirEnt->fsFileCreateTime_HMS=unTemp;

                                            unTemp = s->data[16] + (s->data[17]<<8);
                                            dst_DirEnt->fsFileCreateDate_YMD=unTemp;

                                            //---記錄最後修改時間---//
                                            unTemp = s->data[22] + (s->data[23]<<8);
                                            dst_DirEnt->fsFileModifiedTime_HMS=unTemp;

                                            unTemp = s->data[24] + (s->data[25]<<8);
                                            dst_DirEnt->fsFileModifiedDate_YMD=unTemp;

                                            //-- 紀錄File entry location on DIR--//
                                            dst_DirEnt->FileEntrySect=sect;
                                            dst_DirEnt->FileEntrySectOffset=Offset;

                                            dst_DirEnt++;     // Forward address

                                            /* Also not a long entry, so it is a valid entry */
                                            numEntry++;

                                            //MR9200 format: HHMMSS-ch.ASF
                                            if(s->data[6] == '-')
                                                return numEntry;
                                            else if(s->data[0] == 'A' || s->data[0] == 'P')
                                                return numEntry;
                                            else
                                            	return numEntry;

                                            // light weight search for 9300
                                            /*if('A' != Typesel)
                                            {
                                                return numEntry;
                                            }

                                            if('S' == Typesel)
                                                lightWeight |= 0x1;
                                            if('D' == Typesel)
                                                lightWeight |= 0x2;
                                            if(lightWeight & 0x3)
                                                return numEntry;*/

                                            if (numEntry>=DirEntMax)
                                            {
                                                return numEntry;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            s++;
            Offset ++;
        }
    }
    return numEntry;
}
/*********************************************************************
*
*             FS__fat_MkRmDir
*
  Description:
  FS internal function. Create or remove a directory. If you call this
  function to remove a directory (MkDir==0), you must make sure, that
  it is already empty.

  Parameters:
  pDirName    - Directory name.
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  MkDir       - ==0 => Remove directory.
                !=0 => Create directory.

  Return value:
  ==0         - Directory has been created.
  ==-1        - An error has occured.
*/

int FS__fat_MkRmDir(const char *pDirName, int Idx, char MkDir)
{
    u32 unit, dstart, len, dsize, dsec;
    s32 i;
    int err, lexp_a, lexp_b;
    char realname[12], *filename;

    if (Idx < 0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1; // Not a valid index
    }
    // dsize --> directory size
    err = FS__fat_findpath(Idx, pDirName, &filename, &unit, &dstart, &dsize);
    if (err < 0)
    {
        DEBUG_FS("[Error]: Find path: %d, %s\n", Idx, pDirName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;  // Path not found
    }

    err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);	// Turn on busy signal
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }

    len = FS__CLIB_strlen(filename);
    if (len != 0)
    {
        FS__fat_make_realname(realname, filename);  // Convert name to FAT real name
        // realname: "DCIM        "  dsize:8
        err =  FS__fat_find_dir(Idx, unit, realname, dstart, dsize, &dsec);
        if(err < 0)
            ERRD(FS_DIR_FIND_ERR);
        lexp_a = (dsec!=0) && (MkDir);  // We want to create a direcory , but it does already exist
        lexp_b = (dsec==0) && (!MkDir); // We want to remove a direcory , but it does not exist
        lexp_a = lexp_a || lexp_b;
        if (lexp_a)
        {
            // We want to create, but dir does already exist or we want to remove, but dir is not there
            // turn off busy signal
            if(FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)
                ERRD(FS_DEV_IOCTL_ERR);
            return err;
        }
    }
    else
    {
        err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
        if(err < 0)
            ERRD(FS_DEV_IOCTL_ERR);
        return -1;
    }

    // When you get here, variables have following values:
    //	dstart="current", dsize="size of current", realname="real dir name to create"
    if (MkDir)
    {
        DEBUG_YELLOW("realname: %s, dstart A: %#x, dsize: %#d\n", realname, dstart, dsize);
        i = _FS_fat_create_directory(Idx, unit,realname, dstart, dsize);  /* Create the directory */
        // Add by BJ (20060420)
        if (i < 0)
        {
            // Could not create file
            if (i == -2)
            {
                // Directory is full, try to increase
                i = FSFATIncEntry(Idx, unit, dstart, 1, &dsize, FS_E_CLUST_CLEAN_ON);
                if (i > 0)
                {
                    i = _FS_fat_create_directory(Idx, unit, realname, dstart, dsize);
                }
            }
            if (i < 0)
            {
                if(FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
                    ERRD(FS_DEV_IOCTL_ERR);
                return i;	//? original value is 0?
            }
            else
            {
                err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
                if(err < 0)
                {
                    ERRD(FS_DEV_IOCTL_ERR);
                    return err;	//?
                }
            }
        }
        else
        {
            err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
            if(err < 0)
            {
                ERRD(FS_DEV_IOCTL_ERR);
                return err;	//?
            }
        }
        // End Add (20060420)
    }
    else
    {
        i = FS__fat_DeleteFileOrDir(Idx, unit, realname, dstart, dsize, 0, 0);  /* Remove the directory */
    }
    if (i >= 0)
    {
        // If the operation has been successfull, flush the cache.
        err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return err;	//?
        }
        return 1;
    }
    else
    {
        err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        if(err < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            return err;	//?
        }
        return i;
    }

    //err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn of busy signal
    //if(err < 0)
    //ERRD(FS_DEV_IOCTL_ERR);
    //return -1;
}
#endif // FS_POSIX_DIR_SUPPORT
