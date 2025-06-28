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
File        : fat_in.c
Purpose     : FAT read routines
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

extern char *cMulBlockBuffer;
/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_fread
*
  Description:
  FS internal function. Read data from a file.

  Parameters:
  pData       - Pointer to a data buffer for storing data transferred
                from file.
  Size        - Size of an element to be transferred from file to data
                buffer
  N           - Number of elements to be transferred from the file.
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  Number of elements read.
*/


FS_i32 fat_part_read(void *pData, FS_size_t N, FS_FILE *pFile,FS_u32 datastart)
{
    FS_size_t todo;
    FS_u32 i;
    FS_u32 j;
    FS_u32 fileclustnum;
    FS_u32 diskclustnum;
    FS_u32 prevclust;
    FS_u32 numOfSector = (FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus;
    FS_u32 secPerClus = numOfSector;
    FS_u32 nRemainder;
    char *buffer;
    int err;

    buffer = cMulBlockBuffer;

    prevclust = 0;
    todo = N ;
    while (todo)
    {
        if (pFile->filepos >= pFile->size)
        {
            /* EOF has been reached */
            pFile->error = FS_ERR_EOF;
            //      FS__fat_free(buffer);
            return (N  - todo);
        }
        fileclustnum = pFile->filepos / (FS_FAT_SEC_SIZE * FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus);
        if (prevclust == 0)
        {
            diskclustnum = pFile->CurClust;
            if (diskclustnum == 0)
            {
                /* Find current cluster by starting at 1st cluster of the file */
                diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, fileclustnum);
            }
        }
        else
        {
            /* Get next cluster of the file */
            diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
        }
        prevclust       = diskclustnum;
        pFile->CurClust = diskclustnum;
        if (diskclustnum == 0)
        {
            /* Could not find current cluster */
            pFile->error = FS_ERR_READERROR | 0x00;
            //      FS__fat_free(buffer);
            return (N  - todo);
        }
        diskclustnum -= 2;

        j = (pFile->filepos % (FS_FAT_SEC_SIZE * secPerClus)) / FS_FAT_SEC_SIZE;
        nRemainder = j % secPerClus;
        numOfSector = secPerClus - nRemainder;

        if (todo < FS_FAT_SEC_SIZE * numOfSector)
        {
            nRemainder = todo % FS_FAT_SEC_SIZE;
            numOfSector = todo / FS_FAT_SEC_SIZE;
            if (nRemainder)
                numOfSector++;
        }

        while (1)
        {
            if (!todo)
            {
                break;  /* Nothing more to write */
            }
            if (j >= secPerClus)
            {
                break;  /* End of the cluster reached */
            }
            if (pFile->filepos >= pFile->size)
            {
                break;  /* End of the file reached */
            }

            err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                  datastart +
                                  diskclustnum * secPerClus + j,
                                  numOfSector,
                                  (void*)buffer);

            if (err < 0)
            {
                pFile->error = FS_ERR_READERROR | 0x01;
                //        FS__fat_free(buffer);
                return (N  - todo);
            }
            //      i = pFile->filepos % (FS_FAT_SEC_SIZE * numOfSector);
            i = pFile->filepos % (FS_FAT_SEC_SIZE * 1);
            /*CY 0621 S*/
            {  /* Sector loop */
                FS_u32 rsize;

                rsize = (FS_FAT_SEC_SIZE * numOfSector - i) < todo ? (FS_FAT_SEC_SIZE * numOfSector - i) : todo;
                rsize = (pFile->size - pFile->filepos) < rsize ? (pFile->size - pFile->filepos) : rsize;
                if (rsize)
                {
                    FS__CLIB_memcpy(((FS_FARCHARPTR)pData) + N  - todo, &buffer[i], rsize);
                    i += rsize;
                    pFile->filepos += rsize;
                    todo -= rsize;

                }
            }  /* Sector loop */
            /*CY 0621 E*/
            j += numOfSector;

            nRemainder = todo % FS_FAT_SEC_SIZE;
            if (nRemainder)
                nRemainder = 1;
            numOfSector = (FS_FAT_SEC_SIZE * secPerClus) <= todo ? secPerClus : (todo / FS_FAT_SEC_SIZE + nRemainder);

        }  /* Sector loop */
    }  /* Cluster loop */
    if ((j >= secPerClus) && ((pFile->filepos % (FS_FAT_SEC_SIZE * secPerClus)) == 0))
    {
        pFile->CurClust = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
    }
    //  FS__fat_free(buffer);
    return (N  - todo);


}

FS_i32 get_curr_clst(FS_u32 Totclustnum,FS_FILE *pFile)
{
    FS_u32 diskclustnum;

    diskclustnum = pFile->CurClust;//when use Fseek(),pFile->CurClust will be set "0"
    if (diskclustnum == 0)
    {
        /* Find current cluster by starting at 1st cluster of the file */
        diskclustnum = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->fileid_hi, Totclustnum);
        if (diskclustnum==0)
        {
            pFile->error = FS_ERR_READERROR | 0x02;
            return -1;
        }
    }

    pFile->CurClust=diskclustnum;
    //diskclustnum-=2;

    return diskclustnum;
}

FS_i32 phys_clsts_read(FS_size_t data_len,FS_u32 clstSize,FS_FILE *pFile,void *pData,FS_u32 DataStart,FS_u32 SecPerClus)  // Once enter this function , it must clst align
{       // Data address sec align and data greater than one clst
    FS_u32 i,data_head,data_tail,tail_secs;
    FS_u32 prevclust=pFile->CurClust;
    FS_u32 nextclust;
    FS_u32 StartClst;
    FS_u32 total_clsts;
    FS_u32 conti_times=1,total_rsize=0;
    FS_u32* clstBuf=(FS_u32*)cMulBlockBuffer;
    int err;

    if (clstSize & (FS_FAT_SEC_SIZE-1) )    // invalid clust size
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

    *clstBuf =pFile->CurClust;
    clstBuf++;
#if 0
    for (i=1;i<total_clsts;i++) // Get all clst path
    {
        *clstBuf = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, prevclust, 1);
        pFile->CurClust = *clstBuf;
        prevclust=*clstBuf;
        clstBuf++;
    }
#else
    pFile->CurClust = FS__fat_FindClustList(pFile->dev_index, pFile->fileid_lo, prevclust, total_clsts-1,clstBuf);
    clstBuf +=(total_clsts-1);
#endif
    *clstBuf=0xF5A55A5A;  // Magic Number
    clstBuf=(FS_u32*)cMulBlockBuffer;

    StartClst=*clstBuf;
    while (1)
    {
        prevclust=*clstBuf++;
        nextclust=*clstBuf;

        if (nextclust-prevclust==1)     // Continuous
        {
            conti_times++;
        }
        else        // Read out
        {
            if (nextclust==0xF5A55A5A) // End of reading
            {
                err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                      DataStart + (StartClst-2)* SecPerClus+data_head ,SecPerClus*conti_times-data_head-tail_secs,(void*)((char*)pData+total_rsize));

                if (err < 0)
                {
                    pFile->error = FS_ERR_READERROR | 0x03;
                    return total_rsize;
                }
                total_rsize+=(SecPerClus*conti_times-data_head-tail_secs)*FS_FAT_SEC_SIZE;
                pFile->filepos +=total_rsize;
                if ((pFile->filepos & (clstSize-1))==0 )  //Move advance current clst
                {
                    pFile->CurClust = FS__fat_diskclust(pFile->dev_index, pFile->fileid_lo, pFile->CurClust, 1);
                }
                return total_rsize;
            }
            else
            {
                err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo,
                                      DataStart + (StartClst-2)* SecPerClus+data_head ,SecPerClus*conti_times-data_head,(void*)((char*)pData+total_rsize));
                StartClst=nextclust;
                total_rsize+=(SecPerClus*conti_times-data_head)*FS_FAT_SEC_SIZE;
                conti_times=1;
                data_head=0;
                if (err < 0)
                {
                    pFile->error = FS_ERR_READERROR | 0x04;
                    return total_rsize;
                }
            }
        }
    }

}

FS_size_t FS__fat_fread(void *pData, FS_size_t Size, FS_size_t N, FS_FILE *pFile)
{
    FS_size_t todo;
    FS_u32 fileclustnum;
    FS_u32 datastart;
    FS_u32 secPerClus = (FS_u32)FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].SecPerClus;
    FS_u32 ClusSize =secPerClus*FS_FAT_SEC_SIZE ;
    FS_u32 PosOffset;
    FS_u32 Data_align,main_rsize,rest_rsize;

    int err;

    if (!pFile)
    {
        return 0;  /* No valid pointer to a FS_FILE structure */
    }
    /* Check if media is OK */
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        /* Media has changed */
        pFile->error = FS_ERR_DISKCHANGED | 0x00;
        return 0;
    }
    else if (err < 0)
    {
        /* Media cannot be accessed */
        pFile->error = FS_ERR_READERROR | 0x05;
        return 0;
    }

    datastart = FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].DirStartSec + FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo].Dsize;

    todo = N * Size;

    if (todo==0)
        return 0;
    // Decide partial read or conti-read
    PosOffset = pFile->filepos & (FS_FAT_SEC_SIZE-1);
    Data_align=todo & (FS_FAT_SEC_SIZE-1);


    switch (PosOffset)
    {
    case 0:
        switch (Data_align)
        {
        case 0:          // No Head and Tail
            if (todo>=FS_FAT_SEC_SIZE && (((int)pData & 0x0000000F)==0))
            {
                fileclustnum = pFile->filepos /ClusSize;
                if (get_curr_clst(fileclustnum,pFile)== -1)
                {
                    return 0;
                }
                return phys_clsts_read(todo,ClusSize,pFile,pData,datastart,secPerClus);
            }
            else
                return fat_part_read(pData, todo, pFile,datastart);
            break;
        default:    // Having Tail --> seperate tail
            if (todo>=FS_FAT_SEC_SIZE && (((int)pData & 0x0000000F)==0))
            {
                fileclustnum = pFile->filepos /ClusSize;
                if (get_curr_clst(fileclustnum,pFile)==-1)
                    return 0;
                main_rsize=phys_clsts_read(todo-Data_align,ClusSize,pFile,pData,datastart,secPerClus);
                rest_rsize =fat_part_read((void*)((char*)pData+main_rsize),Data_align, pFile,datastart);
                if (main_rsize!=todo-Data_align && rest_rsize!=Data_align )
                    return 0;
                else
                    return main_rsize+ rest_rsize;
            }
            else
                return fat_part_read(pData,todo, pFile,datastart);
            break;
        }

        break;

    default:    // Head and Tail
        return fat_part_read(pData, todo, pFile,datastart);
        break;
    }

}

