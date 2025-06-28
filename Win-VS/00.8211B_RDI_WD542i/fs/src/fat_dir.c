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
unsigned char File_protect_on;


/*********************************************************************
*
*             Extern Global Variables
*
**********************************************************************/
extern unsigned char gInsertNAND;
extern int dcfLasfEofCluster;
extern unsigned char *exifDecBuf;

/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/
extern int _FS_fat_IncDir_One(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize);
extern void FS__fat_ResetFileEntrySect(int sect,int offset);
extern void FS__fat_GetFileEntrySect(int *psect,int *poffset);

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

static int _FS_fat_create_directory(int Idx, FS_u32 Unit, const char *pDirName,
                                    FS_u32 DirStart, FS_u32 DirSize)
{
    char *buffer;
    FS__fat_dentry_type *s;
    FS_u32 dirindex;
    FS_u32 dsec;
    FS_i32 cluster;
    FS_u16 val_time;
    FS_u16 val_date;
    int err;
    int len;
    int j;
    unsigned int dsec_temp;
    RTC_DATE_TIME   localTime;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    len = FS__CLIB_strlen(pDirName);
    if (len > 11)
    {
        len = 11;
    }
    /* Read directory */
    for (dirindex = 0; dirindex < DirSize; dirindex++)
    {
        dsec = FS__fat_dir_realsec(Idx, Unit, DirStart, dirindex);
        if (dsec == 0)
        {
            /* Translation of relativ directory sector to an absolute sector failed */
            FS__fat_free(buffer);
            return -1;
        }
#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Read directory sector */
#else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Read directory sector */
#endif
        if (err < 0)
        {
            /* Read error */
            FS__fat_free(buffer);
            return -1;
        }
        /* Scan the directory sector for a free or deleted entry */
        s = (FS__fat_dentry_type*)buffer;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }
            if (s->data[0] == 0x00)
            {
                break;  /* Found a free entry */
            }
            if (s->data[0] == (unsigned char)0xe5)
            {
                break;  /* Found a deleted entry */
            }
            // Add NAND 0xFF break events  civicfs
            if (gInsertNAND==1)
            {
                if (s->data[0] == (unsigned char)0xFF)
                {
                    break;  /* Found a free entry in NAND FDB*/
                }
            }
            s++;
        }
        if (s < (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
        {
            /* Free entry found. Make entry and return 1st block of the file. */
            FS__CLIB_strncpy((char*)s->data, pDirName, len);
            s->data[11] = FS_FAT_ATTR_DIRECTORY;
            cluster = FS__fat_FAT_allocOne(Idx, Unit, dcfLasfEofCluster,0);              /* Alloc block in FAT */
            dcfLasfEofCluster=cluster;
            if (cluster >= 0)
            {
                s->data[12]     = 0x00;                                /* Res */
                s->data[13]     = 0x00;                                /* CrtTimeTenth (optional, not supported) */
                RTC_Get_Time(&localTime);
                val_time=((localTime.hour& 0x1F )<<11) |((localTime.min & 0x3F)<< 5) |((localTime.sec/2) & 0x1F);
                s->data[14]     = (unsigned char)(val_time & 0xff);   /* WrtTime */                                /* CrtTime (optional, not supported) */
                s->data[15]     = (unsigned char)(val_time / 256);
                val_date=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);
                s->data[16]     = (unsigned char)(val_date & 0xff);   /* WrtDate */                                /* CrtDate (optional, not supported) */
                s->data[17]     = (unsigned char)(val_date / 256);
                s->data[18]     = 0x00;                                /* LstAccDate (optional, not supported) */
                s->data[19]     = 0x00;
                s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
                s->data[23]     = (unsigned char)(val_time / 256);
                s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
                s->data[25]     = (unsigned char)(val_date / 256);
                s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */
                s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
                s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
                s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
                s->data[28]     = 0x00;                                /* FileSize */
                s->data[29]     = 0x00;
                s->data[30]     = 0x00;
                s->data[31]     = 0x00;
#if FS_RW_DIRECT
                err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Write the modified directory sector */
#else
                err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec, (void*)buffer); /* Write the modified directory sector */
#endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Clear new directory and make '.' and '..' entries */
                /* Make "." entry */
                FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

                dsec = FS__fat_dir_realsec(Idx, Unit, cluster, 0); /* Find 1st absolute sector of the new directory */
                if (dsec == 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                dsec_temp = dsec;

                /* clear all sectors of one cluster first for performance improvement */
                /* for performance improvement, use a buffer with one page size */

                /* loop to clear single sector */
                for (j=0; j<FS__FAT_aBPBUnit[Idx][Unit].SecPerClus; j++)
                {
#if FS_RW_DIRECT
                    err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, dsec_temp++, (void*)buffer);
#else
                    err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, dsec_temp++, (void*)buffer);
#endif
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                }

                s = (FS__fat_dentry_type*)buffer;
                FS__CLIB_strncpy((char*)s->data, ".          ", 11);
                s->data[11]     = FS_FAT_ATTR_DIRECTORY;
                s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
                s->data[23]     = (unsigned char)(val_time / 256);
                s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
                s->data[25]     = (unsigned char)(val_date / 256);
                s->data[26]     = (unsigned char)(cluster & 0xff);    /* FstClusLo / FstClusHi */
                s->data[27]     = (unsigned char)((cluster / 256) & 0xff);
                s->data[20]     = (unsigned char)((cluster / 0x10000L) & 0xff);
                s->data[21]     = (unsigned char)((cluster / 0x1000000L) & 0xff);
                /* Make entry ".." */
                s++;
                FS__CLIB_strncpy((char*)s->data, "..         ", 11);
                s->data[11]     = FS_FAT_ATTR_DIRECTORY;
                s->data[22]     = (unsigned char)(val_time & 0xff);   /* WrtTime */
                s->data[23]     = (unsigned char)(val_time / 256);
                s->data[24]     = (unsigned char)(val_date & 0xff);   /* WrtDate */
                s->data[25]     = (unsigned char)(val_date / 256);
                s->data[26]     = (unsigned char)(DirStart & 0xff);    /* FstClusLo / FstClusHi */
                s->data[27]     = (unsigned char)((DirStart / 256) & 0xff);
                s->data[20]     = (unsigned char)((DirStart / 0x10000L) & 0xff);
                s->data[21]     = (unsigned char)((DirStart / 0x1000000L) & 0xff);


                /* Write "." & ".." entries into the new directory */
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
                FS__fat_free(buffer);
                return 1;

            }
            FS__fat_free(buffer);
            return -1;
        }
    }
    FS__fat_free(buffer);
    return -2;  /* Directory is full */
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

FS_DIR *FS__fat_opendir(const char *pDirName, FS_DIR *pDir)
{
    FS_size_t len;
    FS_u32 unit;
    FS_u32 dstart;
    FS_u32 dsize;
    FS_i32 i;
    char realname[12];
    char *filename;

    if (!pDir)
    {
        return 0;  /* No valid pointer to a FS_DIR structure */
    }
    /* Find path on the media and return file name part of the complete path */
    dsize = FS__fat_findpath(pDir->dev_index, pDirName, &filename, &unit, &dstart);
    if (dsize == 0)
    {
        return 0;  /* Directory not found */
    }
    FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0); /* Turn on busy signal */
    len = FS__CLIB_strlen(filename);
    if (len != 0)
    {
        /* There is a name in the complete path (it does not end with a '\') */
        FS__fat_make_realname(realname, filename);  /* Convert name to FAT real name */
        i =  FS__fat_find_dir(pDir->dev_index, unit, realname, dstart, dsize);  /* Search name in the directory */
        // i is the first cluster of file
        if (i == 0)
        {
            /* Directory not found */
            FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
            return 0;
        }
    }
    else
    {
        /*
           There is no name in the complete path (it does end with a '\'). In that
           case, FS__fat_findpath returns already start of the directory.
        */
        i = dstart;  /* Use 'current' path */
    }
    if (i)
    {
        dsize  =  FS__fat_dir_size(pDir->dev_index, unit, i);  /* Get size of the directory */
    }
    if (dsize == 0)
    {
        /* Directory not found */
        FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        return 0;
    }
    pDir->dirid_lo  = unit;
    pDir->dirid_hi  = i;
    pDir->dirid_ex  = dstart;	// start cluster of the  parent directory.
    pDir->error     = 0;
    pDir->size      = dsize;
    pDir->dirpos    = 0;
    pDir->inuse     = 1;
    return pDir;
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
    if (!pDir)
    {
        return -1;  /* No valid pointer to a FS_DIR structure */
    }
    FS__lb_ioctl(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
    pDir->inuse = 0;
    return 0;
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

struct FS_DIRENT *FS__fat_readdir(FS_DIR *pDir)
{
    FS__fat_dentry_type *s;
    FS_u32 dirindex;
    FS_u32 dsec;
    FS_u16 bytespersec;
    char *buffer;
    int err;
    FS_i32 cluster;
//added by Albert Lee on20090522
    FS_u32 unTemp;

    if (!pDir)
    {
        return 0;  /* No valid pointer to a FS_DIR structure */
    }
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return 0;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    dirindex = pDir->dirpos / bytespersec;
    while (dirindex < (FS_u32)pDir->size)
    {
        dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, dirindex);
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            FS__fat_free(buffer);
            return 0;
        }
        /* Read directory sector */
#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            return 0;
        }
        /* Scan for valid directory entry */
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
                if (s->data[0] != (unsigned char)0xe5)
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
            unTemp = s->data[14] + (s->data[15]<<8);
            pDir->dirent.fsFileCreateTime_HMS = unTemp;


            unTemp = s->data[16] + (s->data[17]<<8);
            pDir->dirent.fsFileCreateDate_YMD = unTemp;

            //---modified time--//
            unTemp = s->data[22] + (s->data[23]<<8);
            pDir->dirent.fsFileModifiedTime_HMS = unTemp;

            unTemp = s->data[24] + (s->data[25]<<8);
            pDir->dirent.fsFileModifiedDate_YMD = unTemp;

            //end
            FS__fat_free(buffer);
            return &pDir->dirent;
        }
        dirindex++;
    }
    FS__fat_free(buffer);
    return 0;
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

unsigned int FS__fat_readwholedir(FS_DIR *pDir,struct FS_DIRENT *dst_DirEnt,
                                  unsigned char* buffer, unsigned int DirEntMax,
                                  DEF_FILEREPAIR_INFO *pdcfBadFileInfo,
                                  unsigned char IsUpdateEntrySect,
                                  int DoBadFile)
{
    FS__fat_dentry_type *s;
    FS_u32 dirindex;
    FS_u32 dsec;
    FS_u16 bytespersec;
    FS_u16 numSector;
    FS_u16 i,sect,offset;
    FS_u16 numEntry=0;
    FS_u32 unTemp;
    FS_u32 size;
    FS_i32 cluster;
    FS_u16 val;
    FS_i32 last;
    unsigned int SecPerClus;

    char flag=1;
    int err;

    RTC_DATE_TIME   localTime;
    //-----------------//



    if (!pDir)
    {
        return 0;  /* No valid pointer to a FS_DIR structure */
    }
    if (!buffer)
    {
        return 0;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;


    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
            dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect);
        else
            dsec +=1;
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            return 0;
        }

#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            return 0;
        }
        s = (FS__fat_dentry_type*)buffer;
        offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
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

                        s->data[22] = (unsigned char)(val & 0xff);
                        s->data[23] = (unsigned char)((val / 0x100) & 0xff);

                        val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

                        s->data[24] = (unsigned char)(val & 0xff);
                        s->data[25] = (unsigned char)((val / 0x100) & 0xff);

#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }
                    }
                    else
                    {
                        s->data[0]= (unsigned char)0xe5;
                        DEBUG_FS("Repair Fail and Delete!\n");

#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }

                        s++;
                        offset ++;
                        continue;
                    }


                }

#else //Delete Bad file
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                        if(err<0)
                        {
                            DEBUG_FS("Delete file-link error!\n");
                        }

                    }

                    s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                    err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                    err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                    if (err < 0)
                    {
                        DEBUG_FS("Delete file-FDB error!\n");
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
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    last = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, 0);

                    if(last<0) //Cannot find EOF
                    {
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                            if(err<0)
                            {
                                DEBUG_FS("Delete file-link error!\n");
                            }
                        }
                        s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }
                        s++;
                        offset ++;

                        continue;
                    }
                }
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (unsigned char)0xe5)
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

                            if(IsUpdateEntrySect)
                                FS__fat_ResetFileEntrySect(sect,offset);
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
int FS__fat_ScanWholedir(       FS_DIR *pDir,
                                struct FS_DIRENT *dst_DirEnt,
                                unsigned char* buffer,
                                unsigned int DirEntMax,
                                unsigned int *pOldestEntry,
                                int DoBadFile
                        )
{
    FS__fat_dentry_type *s;
    FS_u32 dirindex;
    FS_u32 dsec;
    FS_u16 bytespersec;
    FS_u16 numSector;
    FS_u16 i,sect;
    unsigned int numEntry=0;
    FS_u32 unTemp;
    FS_u32 size;
    FS_i32 cluster;
    FS_u16 val;
    FS_i32 last;
    int Offset;

    unsigned int SecPerClus;
    int err;
    RTC_DATE_TIME   localTime;
    unsigned char prev_d_name[9]= {'9','9','9','9','9','9','9','9','\0'};
    unsigned char curr_d_name[9]= {0};
    //-----------------//

    //Lucian: Clear Bad File Record.

    if (!pDir)
    {
        return -1;  /* No valid pointer to a FS_DIR structure */
    }
    if (!buffer)
    {
        return -1;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;

    *pOldestEntry=0;
    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
            dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect);
        else
            dsec +=1;
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            return -1;
        }

#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        Offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }

            if(DoBadFile) //Delete Bad file
            {
                if( (  (s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                        if(err<0)
                        {
                            DEBUG_FS("Delete file-link error!\n");
                        }

                    }

                    s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                    err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                    err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                    if (err < 0)
                    {
                        DEBUG_FS("Delete file-FDB error!\n");
                    }
                    //===============//
                    s++;
                    Offset ++;
                    continue;
                }


                //Scan Directory. if find error directory, delete it//
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_DIRECTORY) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    last = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, 0);

                    if(last<0) //Cannot find EOF
                    {
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                            if(err<0)
                            {
                                DEBUG_FS("Delete file-link error!\n");
                            }
                        }
                        s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }
                        s++;
                        Offset ++;

                        continue;
                    }
                }
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (unsigned char)0xe5)
                {
                    /* not a deleted file */
                    if (s->data[11] == FS_FAT_ATTR_ARCHIVE)
                    {
                        if( (s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0 )
                        {
                            if( dcfGetFileType(&s->data[8]) != DCF_FILE_TYPE_MAX )
                            {
                                //----檔案名稱---//
                                FS__CLIB_memcpy(dst_DirEnt->d_name, s->data, 8);
                                dst_DirEnt->d_name[8] = '.';
                                FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &s->data[8], 3);
                                dst_DirEnt->d_name[12] = 0;

                                //---檔案屬性---//
                                dst_DirEnt->FAT_DirAttr = FS_FAT_ATTR_ARCHIVE;


                                //DEBUG_FS("-->ScanDIR: %s \n",dst_DirEnt->d_name);
                                memcpy(curr_d_name,s->data,8);
                                if(strcmp(curr_d_name,prev_d_name)<0)
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

                                if (numEntry>=DirEntMax)
                                {
                                    return numEntry;
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

int FS__fat_SearchWholedir(FS_DIR *pDir, struct FS_DIRENT *dst_DirEnt, unsigned char* buffer, unsigned int DirEntMax, unsigned int *pOldestEntry,
                           char CHmap, char Typesel, unsigned int StartMin,unsigned int EndMin, int DoBadFile)
{
    FS__fat_dentry_type *s;
    FS_u32 dsec = 0;
    FS_u16 sect;
    FS_u16 bytespersec;
    unsigned int numEntry=0;
    FS_u32 CreateTime,unTemp,ModifyTime,Duration;
    FS_i32 cluster;
    FS_i32 last;
    int Offset;
    int hour,min,sec;
    u8 lightWeight = 0;

    unsigned int SecPerClus;
    int err;
    char prev_d_name[9]= {'9','9','9','9','9','9','9','9','\0'};
    char curr_d_name[9]= {0};
    //-----------------//

    //Lucian: Clear Bad File Record.

    if (!pDir)
    {
        return -1;  /* No valid pointer to a FS_DIR structure */
    }
    if (!buffer)
    {
        return -1;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    bytespersec = bytespersec;	// avoid warming message
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;

    *pOldestEntry=0;
    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
            dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect);
        else
            dsec +=1;
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            return -1;
        }

#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        Offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }

            if(DoBadFile) //Delete Bad file
            {
                if( (  (s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                        if(err<0)
                        {
                            DEBUG_FS("Delete file-link error!\n");
                        }

                    }

                    s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                    err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                    err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                    if (err < 0)
                    {
                        DEBUG_FS("Delete file-FDB error!\n");
                    }
                    //===============//
                    s++;
                    Offset ++;
                    continue;
                }


                //Scan Directory. if find error directory, delete it//
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_DIRECTORY) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    last = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, 0);

                    if(last<0) //Cannot find EOF
                    {
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                            if(err<0)
                            {
                                DEBUG_FS("Delete file-link error!\n");
                            }
                        }
                        s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }
                        s++;
                        Offset ++;

                        continue;
                    }
                }
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (unsigned char)0xe5)
                {
                    /* not a deleted file */
                    if (s->data[11] == FS_FAT_ATTR_ARCHIVE)
                    {
                        if( (s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0 )
                        {
                            if( (s->data[6]==Typesel) || ('A'==Typesel)  )
                            {
                                if( dcfCheckFileChannel(CHmap,s->data[7]) )
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

                                    Duration= (ModifyTime - CreateTime)/60;
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


                                            DEBUG_FS("-->Search DIR: %s,%d min\n",dst_DirEnt->d_name,Duration);

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

                                            // light weight search to saving time
                                            return numEntry;

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


#if RX_SNAPSHOT_SUPPORT
int FS__fat_FS_MapPhotoDir(
    FS_DIR *pDir,
    struct FS_DIRENT *dst_DirEnt,
    unsigned char* buffer,
    unsigned int DirEntMax,
    unsigned int *pOldestEntry,
    int CheckYear,
    int CheckMonth,
    unsigned short *pMap
)
{
    FS__fat_dentry_type *s;
    FS_u32 dsec;
    FS_u16 bytespersec;
    FS_u16 numSector;
    FS_u16 i,sect;
    FS_u32 unTemp;
    FS_u32 size;
    FS_i32 cluster;
    FS_u16 val;
    FS_i32 last;
    int Offset;
    unsigned int SecPerClus;
    int err;
    int year,month,day;
    unsigned short CreateDate;
    unsigned int CheckYM,YM;
    unsigned int numEntry=0;
    unsigned int currYMD,prevYMD;


    //-----------------//
    prevYMD=9999999;
    //Lucian: Clear Bad File Record.
    if (!pDir)
    {
        return -1;  /* No valid pointer to a FS_DIR structure */
    }
    if (!buffer)
    {
        return -1;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;

    //*pOldestEntry=0;
    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
            dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect);
        else
            dsec +=1;
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            return -1;
        }

#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        Offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (unsigned char)0xe5)
                {
                    /* not a deleted file */
                    if (s->data[11] == FS_FAT_ATTR_ARCHIVE)
                    {
                        if( (s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0 )
                        {
                            if( dcfGetFileType(&s->data[8]) != DCF_FILE_TYPE_MAX )
                            {
                                //----------------//
                                CreateDate= s->data[16] + (s->data[17]<<8);
                                year=(CreateDate >> 9)+1980;
                                month=(CreateDate & 0x01E0)>>5;
                                day=(CreateDate & 0x001F);
                                YM=year*12+month;

                                CheckYM=CheckYear*12+CheckMonth;

                                if(YM <=CheckYM)
                                {
                                    if(CheckYM-YM< DCF_PHOTO_DIST_MAX)
                                    {
                                        DEBUG_FS("%d:%d-%d-%d\n",CheckYM-YM,year,month,day);
                                        pMap[CheckYM-YM] +=1;
                                    }
                                }
                                //----------------//
                                //----檔案名稱---//
                                FS__CLIB_memcpy(dst_DirEnt->d_name, s->data, 8);
                                dst_DirEnt->d_name[8] = '.';
                                FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &s->data[8], 3);
                                dst_DirEnt->d_name[12] = 0;

                                //---檔案屬性---//
                                dst_DirEnt->FAT_DirAttr = FS_FAT_ATTR_ARCHIVE;

                                //DEBUG_FS("-->ScanDIR: %s \n",dst_DirEnt->d_name);
                                currYMD= (year*12+month)*31+day;
                                if(currYMD<prevYMD)
                                {
                                    *pOldestEntry=numEntry;
                                    prevYMD=currYMD;
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

                                if (numEntry>=DirEntMax)
                                {
                                    return numEntry;
                                }

                                //----------------//
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


int FS__fat_SearchPhotoWholedir(     FS_DIR *pDir,
                                     struct FS_DIRENT *dst_DirEnt,
                                     unsigned char* buffer,
                                     unsigned int DirEntMax,
                                     unsigned int *pOldestEntry,
                                     int DoBadFile,
                                     int CheckYear,
                                     int CheckMonth
                               )
{
    FS__fat_dentry_type *s;
    FS_u32 dirindex;
    FS_u32 dsec;
    FS_u16 bytespersec;
    FS_u16 numSector;
    FS_u16 i,sect;
    unsigned int numEntry=0;
    FS_u32 unTemp;
    FS_u32 size;
    FS_i32 cluster;
    FS_u16 val;
    FS_i32 last;
    int Offset;

    unsigned int SecPerClus;
    int err;
    RTC_DATE_TIME   localTime;
    unsigned char prev_d_name[9]= {'9','9','9','9','9','9','9','9','\0'};
    unsigned char curr_d_name[9]= {0};

    int year,month,day;
    unsigned short CreateDate;

    //-----------------//

    //Lucian: Clear Bad File Record.

    if (!pDir)
    {
        return -1;  /* No valid pointer to a FS_DIR structure */
    }
    if (!buffer)
    {
        return -1;
    }
    bytespersec = FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].BytesPerSec;
    SecPerClus=(unsigned int)FS__FAT_aBPBUnit[pDir->dev_index][pDir->dirid_lo].SecPerClus;

    *pOldestEntry=0;
    for (sect = 0; sect < pDir->size; sect++)
    {
        if( (sect % SecPerClus)==0) //Lucian: optimize here..
            dsec = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, sect);
        else
            dsec +=1;
        if (dsec == 0)
        {
            /* Cannot convert logical sector */
            return -1;
        }

#if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
        err = FS__lb_read(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
        if (err < 0)
        {
            return -1;
        }
        s = (FS__fat_dentry_type*)buffer;
        Offset=0;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(buffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }

            if(DoBadFile) //Delete Bad file
            {
                if( (  (s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_ARCHIVE) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];  //Check file length
                    DEBUG_FS("======>Bad File Detect and Delete:%d \n",cluster);
                    if (cluster != 0)
                    {
                        err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                        if(err<0)
                        {
                            DEBUG_FS("Delete file-link error!\n");
                        }

                    }

                    s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                    err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                    err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                    if (err < 0)
                    {
                        DEBUG_FS("Delete file-FDB error!\n");
                    }
                    //===============//
                    s++;
                    Offset ++;
                    continue;
                }


                //Scan Directory. if find error directory, delete it//
                if( ((s->data[28] + s->data[29] + s->data[30] +s->data[31])==0) &&
                        (s->data[11] == FS_FAT_ATTR_DIRECTORY) &&
                        (s->data[0] != 0x00) &&
                        (s->data[0] != 0xe5) &&
                        (s->data[0] != 0x2e)
                  )
                {
                    cluster = s->data[26] + 0x100L * s->data[27] + 0x10000L * s->data[20] + 0x1000000L * s->data[21];
                    last = FS__fat_FAT_find_eof(pDir->dev_index,  pDir->dirid_lo, cluster, 0);

                    if(last<0) //Cannot find EOF
                    {
                        DEBUG_FS("======>Bad Directory Detect and Delete:%d \n",cluster);
                        if (cluster != 0)
                        {
                            err=_FS__fat_DeleteFATLink(pDir->dev_index,pDir->dirid_lo,0x0fffff,cluster);
                            if(err<0)
                            {
                                DEBUG_FS("Delete file-link error!\n");
                            }
                        }
                        s->data[0]= (unsigned char)0xe5;
#if FS_RW_DIRECT
                        err = FS__lb_write_Direct(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#else
                        err = FS__lb_write(FS__pDevInfo[pDir->dev_index].devdriver, pDir->dirid_lo, dsec, (void*)buffer);
#endif
                        if (err < 0)
                        {
                            DEBUG_FS("Delete file-FDB error!\n");
                        }
                        s++;
                        Offset ++;

                        continue;
                    }
                }
            }

            if (s->data[11] != 0x00)
            {
                /* not an empty entry */
                if (s->data[0] != (unsigned char)0xe5)
                {
                    /* not a deleted file */
                    if (s->data[11] == FS_FAT_ATTR_ARCHIVE)
                    {
                        if( (s->data[28] + s->data[29] + s->data[30] +s->data[31]) != 0 )
                        {
                            if( dcfGetFileType(&s->data[8]) != DCF_FILE_TYPE_MAX )
                            {
                                CreateDate= s->data[16] + (s->data[17]<<8);
                                year=(CreateDate >> 9)+1980;
                                month=(CreateDate & 0x01E0)>>5;
                                day=(CreateDate & 0x001F);
                                if( (CheckYear == year) && (CheckMonth==month) )
                                {
                                    //----檔案名稱---//
                                    FS__CLIB_memcpy(dst_DirEnt->d_name, s->data, 8);
                                    dst_DirEnt->d_name[8] = '.';
                                    FS__CLIB_memcpy(&dst_DirEnt->d_name[9], &s->data[8], 3);
                                    dst_DirEnt->d_name[12] = 0;

                                    //---檔案屬性---//
                                    dst_DirEnt->FAT_DirAttr = FS_FAT_ATTR_ARCHIVE;

                                    //DEBUG_FS("-->ScanDIR: %s \n",dst_DirEnt->d_name);
                                    memcpy(curr_d_name,s->data,8);
                                    if(strcmp(curr_d_name,prev_d_name)<0)
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
            s++;
            Offset ++;
        }

    }

    return numEntry;


}
#endif
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

int  FS__fat_MkRmDir(const char *pDirName, int Idx, char MkDir)
{
    FS_size_t len;
    FS_u32 dstart;
    FS_u32 dsize;
    FS_u32 unit;
    FS_i32 i;
    int lexp_a;
    int lexp_b;
    char realname[12];
    char *filename;


    if (Idx < 0)
    {
        return -1; /* Not a valid index */
    }
    // dsize --> directory size
    dsize = FS__fat_findpath(Idx, pDirName, &filename, &unit, &dstart);

    if (dsize == 0)
    {
        return -1;  /* Path not found */
    }
    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_INC_BUSYCNT, 0, (void*)0); /* Turn on busy signal */
    len = FS__CLIB_strlen(filename);
    if (len != 0)
    {
        FS__fat_make_realname(realname, filename);  /* Convert name to FAT real name */
        // realname: "DCIM        "  dsize:8
        i =  FS__fat_find_dir(Idx, unit, realname, dstart, dsize);
        lexp_a = (i!=0) && (MkDir);  /* We want to create a direcory , but it does already exist */
        lexp_b = (i==0) && (!MkDir); /* We want to remove a direcory , but it does not exist */
        lexp_a = lexp_a || lexp_b;
        if (lexp_a)
        {
            /* We want to create, but dir does already exist or we want to remove, but dir is not there */
            /* turn off busy signal */
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
            return -1;
        }
    }
    else
    {
        FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        return -1;
    }
    /*
        When you get here, variables have following values:
         dstart="current"
         dsize="size of current"
         realname="real dir name to create"
    */
    if (MkDir)
    {
        i = _FS_fat_create_directory(Idx, unit,realname, dstart, dsize);  /* Create the directory */
// Add by BJ (20060420)
        if (i < 0)
        {
            /* Could not create file */
            if (i == -2)
            {
                /* Directory is full, try to increase */
                i = _FS_fat_IncDir_One(Idx, unit, dstart, &dsize);
                if (i > 0)
                {
                    i = _FS_fat_create_directory(Idx, unit, realname, dstart, dsize);
                }
            }
            if (i < 0)
            {
                FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
                return 0;
            }
            else
                FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        }
        else
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
// End Add (20060420)
    }
    else
    {
        i = FS__fat_DeleteFileOrDir(Idx, unit, realname, dstart, dsize, 0, 0);  /* Remove the directory */
    }
    if (i > 0)
    {
        /* If the operation has been successfull, flush the cache.*/
        FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);
        return 0;
    }
    else
        FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_CLEAN_CACHE, 2, (void*)0);

    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn of busy signal */
    return -1;
}


#endif /* FS_POSIX_DIR_SUPPORT */

