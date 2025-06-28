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
   File        : fat_out.c
   Purpose     : FAT12/FAT16/FAT32 Filesystem file write routines
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

#ifndef FS_FARCHARPTR
#define FS_FARCHARPTR char *
#endif
#ifndef FS_FAT_FWRITE_UPDATE_DIR
#define FS_FAT_FWRITE_UPDATE_DIR 1
#endif

#include "general.h"
#include "Rtcapi.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_fat.h"
#include "fs_clib.h"
#include "board.h"
#include "i2capi.h"

/*********************************************************************
*
*             Global Variable
*
**********************************************************************/
char *cMulBlockBuffer;


/*********************************************************************
*
*             Extern Variable
*
**********************************************************************/
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern unsigned char gInsertNAND;
extern unsigned int smcBlockSize;
extern unsigned int smcPagePerBlock;
#endif

extern s32 dcfLasfEofCluster;
extern BOOLEAN MemoryFullFlag;

/*********************************************************************
*
*             Local functions Body
*
**********************************************************************/

/*********************************************************************
   *
   *             _FS_fat_write_dentry
   *
   Description:
   FS internal function. Write a directory entry.

   Parameters:
   Idx         - Index of device in the device information table
   referred by FS__pDevInfo.
   Unit        - Unit number.
   FirstClust  - First cluster of the file, which's directory entry
   will be written.
   pDirEntry   - Pointer to an FS__fat_dentry_type structure, which
   contains the new directory entry.
   DirSec      - Sector, which contains the directory entry.
   pBuffer     - Pointer to a buffer, which contains the sector with
   the old directory entry.

   Return value:
   ==1         - Directory entry has been written.
   ==0         - An error has occured.
   */

static int _FS_fat_write_dentry(int Idx, FS_u32 Unit, FS_u32 FirstClust, FS__fat_dentry_type *pDirEntry,
                                FS_u32 DirSec, char *pBuffer)
{
    FS__fat_dentry_type *s;
    FS_u32 value;
    int err;

    if (DirSec == 0)
    {
        return 0;  /* Not a valid directory sector */
    }
    if (pBuffer == 0)
    {
        return 0;  /* No buffer */
    }
    /* Scan for the directory entry with FirstClust in the directory sector */
    s = (FS__fat_dentry_type*)pBuffer;
    while (1)
    {
        if (s >= (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
        {
            break;  /* End of sector reached */
        }
        value = (FS_u32)s->data[26] + 0x100UL * s->data[27] + 0x10000UL * s->data[20] + 0x1000000UL * s->data[21];
        if (value == FirstClust && s->data[0]!=0xE5)
        {
            break;  /* Entry found */
        }
        s++;
    }
    if (s < (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
    {
        if (pDirEntry)
        {
            FS__CLIB_memcpy(s, pDirEntry, sizeof(FS__fat_dentry_type));
        #if FS_RW_DIRECT
            err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, DirSec, (void*)pBuffer);
        #else
            err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, DirSec, (void*)pBuffer);
        #endif
            if (err < 0)
            {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}


/*********************************************************************
   *
   *             _FS_fat_read_dentry
   *
   Description:
   FS internal function. Read a directory entry.

   Parameters:
   Idx           - Index of device in the device information table
                   referred by FS__pDevInfo.
   Unit          - Unit number.
   FirstClust    - First cluster of the file, which's directory entry
                   will be read.
   DirStart      - Start of directory, where to read the entry.
   FileEntrySect - Sector location of File Entry.
   pDirEntry     - Pointer to an FS__fat_dentry_type structure, which is
                   used to read the directory entry.
   pDirSec       - Pointer to an FS_u32, which is used to store the sector
                   number, in which the directory entry has been read.
   pBuffer       - Pointer to a buffer, which is used for reading the
                   directory.

   Return value:
   ==1         - Directory entry has been read.
   ==0         - An error has occured.
   */

static int _FS_fat_read_dentry(int Idx, FS_u32 Unit, FS_u32 FirstClust,
                                     FS_u32 DirStart,FS_u32 FileEntrySect,FS__fat_dentry_type *pDirEntry, 
                                     FS_u32 *pDirSec, char *pBuffer)
{
    FS_u32 i;
    FS_u32 dsize;
    FS_u32 value;
    FS__fat_dentry_type *s;
    int err;

    if (pBuffer == 0)
    {
        return 0;
    }
    
#if 1
    /* Read the directory */
    *pDirSec = FS__fat_dir_realsec(Idx, Unit, DirStart, FileEntrySect);
    if (*pDirSec == 0)
    {
        return 0;  /* Unable to translate relative directory sector to absolute setor */
    }
#if FS_RW_DIRECT
    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pDirSec, (void*)pBuffer);
#else
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, *pDirSec, (void*)pBuffer);
#endif
    if (err < 0)
    {
        return 0;
    }
    /* Scan for entry with FirstClus in the sector */
    s = (FS__fat_dentry_type*)pBuffer;
    while (1)
    {
        if (s >= (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
        {
            break;  /* End of sector reached */
        }
        value = (FS_u32)s->data[26] + 0x100UL * s->data[27] + 0x10000UL * s->data[20] + 0x1000000UL * s->data[21];
        if ( (value == FirstClust) && (s->data[0]!=0xE5) )
        {
            break;  /* Entry found */
        }
        s++;
    }
    if (s < (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
    {
        if (pDirEntry)
        {
            /* Read the complete directory entry from the buffer */
            FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
        }
        return 1;
    }
    DEBUG_FS("\n--->Error,Cannot find file entry:%d\n",FileEntrySect);
    return 0; // Error!
#else
    dsize  =  FS__fat_dir_size(Idx, Unit, DirStart);
    /* Read the directory */
    for (i = 0; i < dsize; i++)
    {
        *pDirSec = FS__fat_dir_realsec(Idx, Unit, DirStart, i);
        if (*pDirSec == 0)
        {
            return 0;  /* Unable to translate relative directory sector to absolute setor */
        }
    #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pDirSec, (void*)pBuffer);
    #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, *pDirSec, (void*)pBuffer);
    #endif
        if (err < 0)
        {
            return 0;
        }
        /* Scan for entry with FirstClus in the sector */
        s = (FS__fat_dentry_type*)pBuffer;
        while (1)
        {
            if (s >= (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
            {
                break;  /* End of sector reached */
            }
            value = (FS_u32)s->data[26] + 0x100UL * s->data[27] + 0x10000UL * s->data[20] + 0x1000000UL * s->data[21];
            if (value == FirstClust)
            {
                break;  /* Entry found */
            }
            s++;
        }
        if (s < (FS__fat_dentry_type*)(pBuffer + FS_FAT_SEC_SIZE))
        {
            if (pDirEntry)
            {
                /* Read the complete directory entry from the buffer */
                FS__CLIB_memcpy(pDirEntry, s, sizeof(FS__fat_dentry_type));
            }
            return 1;
        }
    }
    return 0;
#endif
}


/*********************************************************************
   *
   *             Global functions
   *
   **********************************************************************
   */

/*********************************************************************
   *
   *             FS__fat_fwrite
   *
   Description:
   FS internal function. Write data to a file.

   Parameters:
   pData       - Pointer to data, which will be written to the file.
   Size        - Size of an element to be transferred to a file.
   N           - Number of elements to be transferred to the file.
   pFile       - Pointer to a FS_FILE data structure.

   Return value:
   Number of elements written.
   */

FS_i32 Updated_Dentry(FS_FILE *pFile)
{
#if (FS_FAT_FWRITE_UPDATE_DIR)
    FS__fat_dentry_type s;
    FS_u32 dsec;
    FS_u16 val;
    int err;
#endif /* FS_FAT_FWRITE_UPDATE_DIR */
    char *buffer;

    buffer = cMulBlockBuffer;
    if (!buffer)
    {
        return -1;
    }
#if (FS_FAT_FWRITE_UPDATE_DIR)
    /* Modify directory entry */
    err = _FS_fat_read_dentry(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, pFile->fileid_ex, pFile->FileEntrySect,&s, &dsec, buffer);
    if (err == 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x00;
        return -1;
    }
    s.data[28] = (unsigned char)(pFile->size & 0xff);   /* FileSize */
    s.data[29] = (unsigned char)((pFile->size / 0x100UL) & 0xff);
    s.data[30] = (unsigned char)((pFile->size / 0x10000UL) & 0xff);
    s.data[31] = (unsigned char)((pFile->size / 0x1000000UL) & 0xff);
    val = FS_X_OS_GetTime();
    s.data[22] = (unsigned char)(val & 0xff);
    s.data[23] = (unsigned char)((val / 0x100) & 0xff);
    val = FS_X_OS_GetDate();
    s.data[24] = (unsigned char)(val & 0xff);
    s.data[25] = (unsigned char)((val / 0x100) & 0xff);
    err = _FS_fat_write_dentry(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, &s, dsec, buffer);
    if (err == 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x01;
    }
#endif /* FS_FAT_FWRITE_UPDATE_DIR */

    return 1;

}

FS_size_t fat_part_fwrite(const void *pData, FS_size_t N, FS_FILE *pFile,FS_u32 datastart)
{
    FS_size_t todo;
    FS_u32 bytesperclus;
    FS_u32 fileclustnum;
    FS_u32 diskclustnum;
    FS_u32 prevclust;
    FS_i32 last;
    FS_i32 i;
    FS_i32 j; /* which sector */
    FS_i32 k;
    FS_u32 wsize;
    int err;
    int lexp;
    int numOfSector;
    int nRemainder;
    int secPerClus = (FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus;
    char *buffer;
    FS_u32* clstBuf=(FS_u32*)cMulBlockBuffer; 



    buffer = cMulBlockBuffer;
    if (!buffer)
    {
        return 0;
    }

    todo = N ;  /* Number of bytes to be written */

    /* Alloc new clusters if required */
    bytesperclus = secPerClus * ((FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].BytesPerSec);
    
    /* Calculate number of clusters required */
    i = (pFile->filepos + todo+bytesperclus-1) / bytesperclus;       // Optimize

    /* Calculate clusters already allocated */
    j = (pFile->size + bytesperclus-1)/ bytesperclus;

    if (pFile->size == 0)
        lexp=1;
    else
        lexp=0;
    if (lexp)
    {
        j++;
    }
    i -= j; //i: 還需allocate 的 cluster.

    if (i > 0)
    {
        /* Alloc new clusters */
        last = pFile->EOFClust;
        if (last < 0)
        {
            /* Position of EOF is unknown, so we scan the whole file to find it */
            last = FS__fat_FAT_find_eof(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, 0);   // Find EOF if we don't know end of cluster
        }
        if (last < 0)
        {
            return 0;
        }
    #if 0
        while (i)
        {   // Write FAT data to FAT area
            last = FS__fat_FAT_allocOne(pFile->dev_index, pFile->fileid_lo, last,1);	/* Allocate new cluster */
            pFile->EOFClust = last;
            if (last < 0)
            {
                /* Cluster allocation failed */
                pFile->size += (N  - todo);
                pFile->error = FS_ERR_DISKFULL;
                return (N  - todo);
            }
            i--;
        }
    #else
        last = FS__fat_FAT_allocMulti(pFile->dev_index,pFile->fileid_lo, last,  i , clstBuf);
        pFile->EOFClust = last;
        if (last < 0)
        {
            /* Cluster allocation failed */
            return 0;
        }
        clstBuf +=i;
    #endif
        /* Update the FSInfo structure */
        /* Add codes here */
        //FS__fat_Update_FSInfo();
    }
    /* Get absolute postion of data area on the media */
    /* Write data to clusters */
    prevclust = 0;

    while (todo)
    {   /* Write data loop */
        /* Translate file ppinter position to cluster position*/
        fileclustnum = pFile->filepos / bytesperclus;
        // Check continuous cluster or not

        /*
        Translate the file relative cluster position to an absolute cluster
        position on the media. To avoid scanning the whole FAT of the file,
        we remember the current cluster position in the FS_FILE data structure.
        */
        if (prevclust == 0)
        {
            //diskclustnum = pFile->CurClust;
            /* No known current cluster position, we have to scan from the file's start cluster */
            //if (diskclustnum == 0)
            diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, fileclustnum);
        }
        else
        {
            /* Get next cluster of the file starting at the current cluster */
            diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
        }
        prevclust        = diskclustnum;
        pFile->CurClust  = diskclustnum;
        if (diskclustnum == 0)
        {
            /* Translation to absolute cluster failed */
			DEBUG_FS("_-->Error! FS__fat_diskclust=0\n");
            pFile->error = FS_ERR_WRITEERROR | 0x02;
            return (N  - todo);
        }
        diskclustnum -= 2;  //換算成 real cluster number.
        j = (pFile->filepos % bytesperclus) / FS_FAT_SEC_SIZE; //j: 

        while (1)
        {   /* Cluster loop */
            if (!todo)
            {
                break;  /* Nothing more to write */
            }


            if (j >= FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus)
                break; /* End of cluster reached */

            i = pFile->filepos % FS_FAT_SEC_SIZE;
            /*
            We only have to read the sector from the media, if we do not
            modify the whole sector. That is the case if

            a) Writing starts not at the first byte of the sector
            b) Less data than the sector contains is written
            */
            lexp = (i != 0);
            lexp = lexp || (todo < FS_FAT_SEC_SIZE);
            if (lexp)
            {
                /* We have to read the old sector */
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                  datastart +
                                  diskclustnum * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus + j,
                                  (void*)buffer);
            #else
                err = FS__lb_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                  datastart +
                                  diskclustnum * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus + j,
                                  (void*)buffer);
            #endif
                if (err < 0)
                {
                    pFile->error = FS_ERR_WRITEERROR | 0x03;
                    return (N  - todo);
                }
            }

            nRemainder = j % secPerClus;
            numOfSector = secPerClus - nRemainder;

            if (todo < FS_FAT_SEC_SIZE * numOfSector)
            {
                nRemainder = todo % FS_FAT_SEC_SIZE;
                numOfSector = todo / FS_FAT_SEC_SIZE;
                if (nRemainder)
                    numOfSector++;
            }

            { /* Sector loop: 使用temp beffer */
                wsize = (FS_FAT_SEC_SIZE * numOfSector - i) < todo ? (FS_FAT_SEC_SIZE * numOfSector - i) : todo;
                if (wsize)
                {
                    FS__CLIB_memcpy(&buffer[i], ((FS_FARCHARPTR)pData) + N  - todo, wsize);
                    i += wsize;
                    pFile->filepos += wsize;
                    if (pFile->filepos > pFile->size)
                    {
                        pFile->size = pFile->filepos;
                    }
                    todo -= wsize;
                }
            } /* Sector loop */
            {
                /* Write the modified sector */
                err = FS__lb_mul_write(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                       datastart +
                                       diskclustnum * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus + j,
                                       numOfSector,
                                       (void*)buffer);

                if (err < 0)
                {
                    pFile->error = FS_ERR_WRITEERROR | 0x04;
                    return (N  - todo);
                }
                j+=numOfSector;

            }
        }  /* Cluster loop */
    }	    /* Write data loop */
    if (i >= FS_FAT_SEC_SIZE)
    {
        if (j >= FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus)
        {
            /* File pointer is already in the next cluster */
            pFile->CurClust = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
        }
    }
    if (Updated_Dentry(pFile)!=1)
        return 0;

    return (N  - todo);


}


FS_i32 allocate_one_fat(FS_FILE *pFile)
{
    FS_i32 last;

    /* Alloc new clusters */
    last = pFile->EOFClust;
    if (last < 0)
    {
        /* Position of EOF is unknown, so we scan the whole file to find it */
        last = FS__fat_FAT_find_eof(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, 0);   // Find EOF if we don't know end of cluster
    }
    if (last < 0)
    {
        return 0;
    }

    last = FS__fat_FAT_allocOne(pFile->dev_index, pFile->fileid_lo, last,1);	/* Allocate new cluster */
    pFile->EOFClust = last;
    if (last < 0)
    {
        /* Cluster allocation failed */
        return 0;
    }
    /* Update the FSInfo structure */
    /* Add codes here */
    //FS__fat_Update_FSInfo();

    return last;


}

FS_i32* allocate_multi_fat(FS_FILE *pFile,FS_u32* clstBuf, FS_u32 AllocFatNum)
{
    FS_i32 last;
    int i;


    
    /* Alloc new clusters */
    last = pFile->EOFClust;
    if (last < 0)
    {
        /* Position of EOF is unknown, so we scan the whole file to find it */
        last = FS__fat_FAT_find_eof(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, 0);   // Find EOF if we don't know end of cluster
        pFile->EOFClust = last;
    }
    
    if (last < 0)
    {
        DEBUG_FS("Warning! FS__fat_FAT_find_eof(): EOF cannot be found!\n");
        return 0;
    }
  
    if(AllocFatNum==0)
    {
        return clstBuf;
    }        
    
  #if 0
        for(i=0;i<AllocFatNum;i++)
        {
            last = FS__fat_FAT_allocOne(pFile->dev_index, pFile->fileid_lo, last,1);	/* Allocate new cluster */
            pFile->EOFClust = last;
            pFile->CurClust = last;
            *clstBuf=last;
            if (last < 0)
            {
                /* Cluster allocation failed */
                return 0;
            }
            clstBuf ++;
            FS__fat_Update_FSInfo();
        }
  #else
        last = FS__fat_FAT_allocMulti(pFile->dev_index,pFile->fileid_lo, last,  AllocFatNum , clstBuf);
        pFile->EOFClust = last;
        pFile->CurClust = last;
        if (last < 0)
        {
            /* Cluster allocation failed */
            return 0;
        }
        clstBuf +=AllocFatNum;
        //FS__fat_Update_FSInfo();
  #endif
  
    /* Update the FSInfo structure */
    /* Add codes here */
    //FS__fat_Update_FSInfo();

    return clstBuf;


}
FS_i32 phys_clsts_write(FS_size_t data_len,FS_u32 clstSize,FS_FILE *pFile,const void *pData,FS_u32 DataStart,FS_u32 SecPerClus)  // Once enter this function , it must clst align
{   // Data address sec align and data greater than one clst
    FS_u32 data_head,data_tail,tail_secs;
    FS_u32 prevclust;
    FS_u32 nextclust;
    FS_i32 StartClst;
    FS_u32 total_clsts,alloctated_clsts=0;
    FS_u32 conti_times=1,total_wsize=0;
    int err;
    FS_i32 i;
    FS_u32* clstBuf=(FS_u32*)cMulBlockBuffer;       // 64K --> worst case 8MB per write

    if (clstSize & (FS_FAT_SEC_SIZE-1) ||pFile->filepos > pFile->size  )    // invalid clust size
        return 0;

    data_head = (pFile->filepos & (clstSize-1))/FS_FAT_SEC_SIZE;

    if ((data_head*FS_FAT_SEC_SIZE + data_len)<=clstSize)       // Head plus len within one clst
    {
        data_tail=((pFile->filepos + data_len) & (clstSize-1))/FS_FAT_SEC_SIZE;
        total_clsts=1;
        if (data_tail==0)
            tail_secs=0;
        else
            tail_secs= (SecPerClus-data_tail);
    }
    else   //cross clst
    {
        data_tail=((pFile->filepos + data_len) & (clstSize-1))/FS_FAT_SEC_SIZE;
        if (data_tail==0)
            tail_secs=0;
        else
            tail_secs= (SecPerClus-data_tail);
        total_clsts=(data_head*FS_FAT_SEC_SIZE + data_len + tail_secs*FS_FAT_SEC_SIZE)/clstSize;
    }

    if (pFile->size==0 ||data_head!=0 || pFile->filepos<pFile->size)
    {
        alloctated_clsts=1;
        if (get_curr_clst(pFile->filepos /clstSize,pFile)==-1)
            return 0;
        prevclust = pFile->CurClust;
        *clstBuf = pFile->CurClust;
        clstBuf++;
    }
    //------------------Allocat New Cluster-------------------//
#if 0    
    for (i=1;i<total_clsts-alloctated_clsts+1;i++)        // allocated first
    {
        if ((pFile->filepos + clstSize*i) <pFile->size)      // Find out corresponding clsts
        {
            *clstBuf = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1); 
            pFile->CurClust = *clstBuf ;
            prevclust=pFile->CurClust;
            clstBuf++;
            continue;
        }
        else      // allocate clsts
        {
            *clstBuf = allocate_one_fat(pFile);
            pFile->CurClust=*clstBuf;
            if (*clstBuf ==0)
                return 0;
            else
                clstBuf++;
        }
    }
#else //Optimize
    if(pFile->filepos<pFile->size)
    {
        for (i=1;i<total_clsts-alloctated_clsts+1;i++)        // allocated first
        {
            if ((pFile->filepos + clstSize*i) <pFile->size)      // Find out corresponding clsts
            {
                *clstBuf = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1); 
                pFile->CurClust = *clstBuf ;
                prevclust=pFile->CurClust;
                clstBuf++;
                continue;
            }
            else      // allocate clsts
            {
                *clstBuf = allocate_one_fat(pFile);
                pFile->CurClust=*clstBuf;
                if (*clstBuf ==0)
                    return 0;
                else
                    clstBuf++;
            }
        }
    }
    else
    {
       #if 0
        for (i=0;i<total_clsts-alloctated_clsts;i++)        // allocated first
        {
            *clstBuf = allocate_one_fat(pFile);
            pFile->CurClust=*clstBuf;
            if (*clstBuf ==0)
                return 0;
            else
                clstBuf++;
        }
       #else
         clstBuf = allocate_multi_fat(pFile,clstBuf,total_clsts-alloctated_clsts);
         if (clstBuf ==0)
         {
            DEBUG_FS("Allocat FAT Fail.\n");
            return 0;
         }
       #endif            
    }
#endif
    //-----------------------------------------------//
    if(clstBuf == (unsigned int *)cMulBlockBuffer)
    {
       clstBuf ++;
    }
    *clstBuf=0xF5A55A5A;  // Magic Number
    clstBuf=(FS_u32*)cMulBlockBuffer;
    StartClst=*clstBuf;
    
    while (1)
    {
        prevclust=*clstBuf;
        clstBuf ++;
        nextclust=*clstBuf;

        if (nextclust-prevclust==1)     // Continuous
        {
            conti_times++;
        }
        else        // Read out
        {
            if (nextclust==0xF5A55A5A) // End of reading
            {
                err = FS__lb_mul_write(FS__pDevInfo[pFile->dev_index].devdriver,
                                       pFile->fileid_lo,
                                       DataStart + (StartClst-2)* SecPerClus+data_head ,
                                       SecPerClus*conti_times-data_head-tail_secs,(void*)((char*)pData+total_wsize));
                if (err < 0)
                {
                    pFile->error = FS_ERR_READERROR | 0x06;
                    return total_wsize;
                }
                total_wsize+=(SecPerClus*conti_times-data_head-tail_secs)*FS_FAT_SEC_SIZE;
                pFile->filepos +=total_wsize;

                if (pFile->filepos>pFile->size)
                    pFile->size = pFile->filepos;
                /*
                if ((pFile->filepos & (clstSize-1))==0 )  //Move advance current clst
                {
                    pFile->CurClust = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->CurClust, 1);                    
                }
                */
                if (Updated_Dentry(pFile)!=1) // Updated Dentry
                    return 0;
                return total_wsize;
            }
            else
            {
                err = FS__lb_mul_write(FS__pDevInfo[pFile->dev_index].devdriver,
                                       pFile->fileid_lo,
                                       DataStart + (StartClst-2)* SecPerClus+data_head ,
                                       SecPerClus*conti_times-data_head,(void*)((char*)pData+total_wsize));
                StartClst=nextclust;
                total_wsize+=(SecPerClus*conti_times-data_head)*FS_FAT_SEC_SIZE;
                conti_times=1;
                data_head=0;
                if (err < 0)
                {
                    pFile->error = FS_ERR_READERROR | 0x07;
                    return total_wsize;
                }
            }
        }      
    }
    
}

FS_size_t FS__fat_fwrite(const void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile)
{
    FS_size_t todo;
    FS_u32 datastart;
    FS_u32 secPerClus = (FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus;
    FS_u32 ClusSize =secPerClus*FS_FAT_SEC_SIZE ;
    FS_u32 PosOffset;
    FS_u32 Data_align,main_rsize,rest_rsize;

    int err;

    if (!pFile)
        return 0;

    /*------ Check if media is OK -------*/
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        pFile->error = FS_ERR_DISKCHANGED | 0x01;
        return 0;
    }
    else if (err < 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x05;
        return 0;
    }

    //datastart= RSVD+FAT+ROOT
    datastart = FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].DirStartSec + FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].Dsize;
    todo=N*Size;

    if (!todo)
        return 0;

    PosOffset = pFile->filepos & (FS_FAT_SEC_SIZE-1);
    Data_align=todo & (FS_FAT_SEC_SIZE-1);

    switch (PosOffset)
    {
    case 0: //start point is section alignment.
        switch (Data_align)
        {
        case 0:     // No Head and no Tail: start point is sector alignment, and end point is sector alignment.
            if (todo>=FS_FAT_SEC_SIZE && (((int)pData & 0x0000000F)==0)) //至少one sector, buffer start address is 4 word alignment.
            {
                return phys_clsts_write(todo,ClusSize,pFile,pData,datastart,secPerClus);
            }
            else
                return  fat_part_fwrite(pData, N, pFile,datastart);
            break;
        default:    // Having Tail --> seperate tail: Start point is sector alignment. but end point is not sector alignment.
            if (todo>=FS_FAT_SEC_SIZE && (((int)pData & 0x0000000F)==0))
            {
                main_rsize=phys_clsts_write(todo-Data_align,ClusSize,pFile,pData,datastart,secPerClus);
                rest_rsize =fat_part_fwrite((void*)((char*)pData+main_rsize), Data_align, pFile,datastart);
                if (main_rsize!=todo-Data_align && rest_rsize!=Data_align )
                    return 0;
                else
                    return main_rsize+ rest_rsize;
            }
            else
                return  fat_part_fwrite(pData, N, pFile,datastart);
            break;
        }

        break;

    default:    // Head and Tail
        return fat_part_fwrite(pData, N, pFile,datastart);
        break;
    }

}
/*********************************************************************
*
*             FS__fat_fclose
*
Description:
FS internal function. Close a file referred by pFile.

Parameters:
pFile       - Pointer to a FS_FILE data structure.

Return value:
None.
*********************************************************************/

void FS__fat_fclose(FS_FILE *pFile)
{
#if (FS_FAT_FWRITE_UPDATE_DIR==0)
    FS__fat_dentry_type s;
    char *buffer;
    FS_u32 dsec;
    FS_u16 val;
#endif /* FS_FAT_FWRITE_UPDATE_DIR */
    int err;
    RTC_DATE_TIME   localTime;

    if (!pFile)
    {
        return;
    }
    /* Check if media is OK */
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        pFile->error = FS_ERR_DISKCHANGED | 0x02;
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        pFile->inuse = 0;
        return;
    }
    else if (err < 0)
    {
        pFile->error = FS_ERR_CLOSE | 0x00;
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
        pFile->inuse = 0;
        return;
    }

    if (pFile->mode_w)      // civic 071120 for enhance performance
    {
#if (FS_FAT_FWRITE_UPDATE_DIR==0)
        /* Modify directory entry */
        buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
        if (!buffer)
        {
            pFile->inuse = 0;
            pFile->error = FS_ERR_CLOSE | 0x01;
            DEBUG_FS("FS__fat_malloc is Fail\n");
            return;
        }
        // s: the 32 bytes of one FDB data
        // dsec: The sector number which record the
        err = _FS_fat_read_dentry(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, pFile->fileid_ex, pFile->FileEntrySect,&s, &dsec, buffer);

        if (err == 0)
        {
            pFile->inuse = 0;
            pFile->error = FS_ERR_CLOSE | 0x02;
            FS__fat_free(buffer);
            return;
        }

        //--Recording File size
        s.data[28] = (unsigned char)(pFile->size & 0xff);   /* FileSize */
        s.data[29] = (unsigned char)((pFile->size / 0x100UL) & 0xff);
        s.data[30] = (unsigned char)((pFile->size / 0x10000UL) & 0xff);
        s.data[31] = (unsigned char)((pFile->size / 0x1000000UL) & 0xff);

        //---Recording File time
        RTC_Get_Time(&localTime);
		val=((localTime.hour& 0x1F )<<11) |((localTime.min	& 0x3F)<< 5) |((localTime.sec/2) & 0x1F);

		s.data[22] = (unsigned char)(val & 0xff);
		s.data[23] = (unsigned char)((val / 0x100) & 0xff);

        val=(((localTime.year+ 20)& 0x7F) <<9) |((localTime.month & 0xF)<< 5) |((localTime.day) & 0x1F);

        s.data[24] = (unsigned char)(val & 0xff);
        s.data[25] = (unsigned char)((val / 0x100) & 0xff);
        //--Write Back to SD or Cache
        err = _FS_fat_write_dentry(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, &s, dsec, buffer);
        if (err == 0)
        {
            pFile->error = FS_ERR_CLOSE | 0x03;
        }
        FS__fat_free(buffer);
#endif /* FS_FAT_FWRITE_UPDATE_DIR */
        FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0); //clean cache

        //DEBUG_FS("FileClose:pFile->EOFClust=%d\n",pFile->EOFClust);
        if(MemoryFullFlag == TRUE)
           dcfLasfEofCluster=-1;
        else
           dcfLasfEofCluster=pFile->EOFClust;
        //DEBUG_FS("FileClose:dcfLasfEofCluster=%d\n",dcfLasfEofCluster);
    }
    if (err < 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x06;
    }
    pFile->inuse = 0;
    FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
}


