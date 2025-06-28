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

#define FS_FAT_FINAL_UPDATE 1	// Only Update file entry while execute file close operation.

#include "general.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_fat.h"
#include "fs_clib.h"

/*********************************************************************
*
*             Global Variable
*
**********************************************************************/


/*********************************************************************
*
*             Extern Variable
*
**********************************************************************/


/*********************************************************************
*
*             Local functions Body
*
**********************************************************************/
#if FS_BIT_WISE_OPERATION
int FS__fat_fwrite(FS_FILE *pFile, const void *pData, u32 DataSize, u32 *pWriteSize)
{
    FS__FAT_BPB *pBPBUnit;
    u32 CurCluster, CurSector;
    u32 NumOfClust, ContOfClust, WriteInSizeVal, MemCpySizeVal;
    u32 FilePosition, PosQuotient, PosRemainder, RestData;
    u32 TmpVal, Step, i, j;
    int err;
    u8 *pClustDataBuf, *pPClustDataBuf, *pDataBuf;
    //
    if(!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;	// No valid pointer to a FS_FILE structure
    }
    if(DataSize == 0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }

    // Check if media is OK
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        pFile->error = FS_ERR_DISKCHANGED | 0x01;
        return -1;
    }
    else if (err < 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x05;
        ERRD(FS_DEV_ACCESS_ERR);
        return err;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
    FilePosition = pFile->filepos;
    CurCluster = pFile->EOFClust;
    pDataBuf = (u8 *)pData;
    *pWriteSize = 0;
    RestData = (DataSize - *pWriteSize);    
    PosQuotient = (FilePosition >> pBPBUnit->BitNumOfSOC);
    if(PosQuotient)
        PosRemainder = FilePosition - (PosQuotient << pBPBUnit->BitNumOfSOC);
    else
        PosRemainder = FilePosition;

    // Count the allocated size.
    if(pFile->size == 0)
    {
        TmpVal = pBPBUnit->SizeOfCluster;
    }
    else
    {
        TmpVal = ((pFile->size & (pBPBUnit->SizeOfCluster - 1)) > 0)? 1: 0;
        TmpVal = (((pFile->size >> pBPBUnit->BitNumOfSOC) + TmpVal) << pBPBUnit->BitNumOfSOC);
    }
    
    if(FilePosition > TmpVal)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }

    // Check that whether is necessary to increase new cluster or not.
    Step = (FilePosition + RestData);   // Use parameter Step to be temporary parameter here.
    if(Step > TmpVal)
    {
        Step -= TmpVal;
        TmpVal = (Step >> pBPBUnit->BitNumOfSOC);
        TmpVal += (Step > (TmpVal << pBPBUnit->BitNumOfSOC))? 1:0;

        if(TmpVal > 0)
        {
            if((err = FSFATIncEntry(pFile->dev_index, pFile->fileid_lo, CurCluster, TmpVal, NULL, FS_E_CLUST_CLEAN_OFF)) < 0)
            {
                ERRD(FS_FAT_CLUT_ALLOC_ERR);
                return err;
            }

            // Move EOF cluster position to newest one.
            if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, CurCluster, 0, (u32 *)&pFile->EOFClust)) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                return err;
            }
        }
    } 

    // Calculate the relatively number of cluster in FAT link list.
    if(pFile->size == FilePosition)
    {
        // Two situation. 1. UsedSize = 0, 2. increase size.
        if((pFile->size == 0x0) || (PosRemainder != 0))
            Step = 1;   // Stay EOF cluster
        else
            Step = 2;   // Move to next one.
    }
    else
    {
        // There has some space at the end cluster or rewrite old data.
        if((pFile->NumOfPosClust != PosQuotient) || (CurCluster != pFile->CurClust))
        {
            CurCluster = pFile->fileid_hi;
            Step = PosQuotient + 1;   // Move to created one.
            //DEBUG_YELLOW("# reroll. Step: %d\n", Step);
        }
        else
            Step = 1;   // Stay EOF cluster
    }
    
    // Memory allocate
    if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    do
    {
        /* 
         *  Locate the current cluster, 
         *  Step = 0: last one.
         *  Step = 1: stay on current one. 
         *  Step 2 ~ ?: move next (N - 1).
         */
        if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, CurCluster, Step, &CurCluster)) < 0)
        {
            ERRD(FS_FAT_CLUT_FIND_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        /* 
         *  Count the continuous cluster value in FAT link.    
         *  PS: Cut the data into pieces if PosRemainder isn't align with the cluster.
         *      1. PosRemainder != 0, 
         *      2. PosRemainder == 0,
         *      3. left.
         */
        NumOfClust = 1;
        if(PosRemainder == 0x0)
        {
            for(j = 1, i = CurCluster; RestData > (j << pBPBUnit->BitNumOfSOC); j++)
            {
                TmpVal = i;
                if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, TmpVal, 2, &i)) < 0)
                {
                    ERRD(FS_FAT_CLUT_FIND_ERR);
                    FSFree(pClustDataBuf);
                    return err;
                }

                if((i - TmpVal) == 1)
                    NumOfClust++;
                else
                    break;
            }

            // Only handle the area which is continuously and not resting data from size of cluster.
            if((RestData < (NumOfClust << pBPBUnit->BitNumOfSOC)) && (NumOfClust > 1))
                NumOfClust--;
        }
        ContOfClust = (NumOfClust << pBPBUnit->BitNumOfSOC);

        if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        pFile->CurClust = CurCluster;
        //DEBUG_BLUE("[I] CC: %#x, FP: %#x, NC: %d\n", CurCluster, FilePosition, NumOfClust);
        // Calculate the memcpy size and the figure of accessed blk.
        if(PosRemainder > 0)
        {
            i = (PosRemainder >> pBPBUnit->BitNumOfBPS);
            pPClustDataBuf = pClustDataBuf;
            if(PosRemainder + RestData < ContOfClust)
            {
                // Data may not over the cluster boundary.
                // |--------Cluster-------|
                // |---------===wanted==--|
                MemCpySizeVal = RestData;
            }
            else
            {
                // Data may close the cluster boundary.
                // |--------Cluster-------|
                // |---------====wanted===|
                MemCpySizeVal = ContOfClust - PosRemainder;
            }
            WriteInSizeVal = ((PosRemainder & pBPBUnit->BitRevrOfBPS) + MemCpySizeVal);
            WriteInSizeVal = (WriteInSizeVal >> pBPBUnit->BitNumOfBPS) + ((WriteInSizeVal & pBPBUnit->BitRevrOfBPS)? 1: 0);
        }
        else
        {
            i = 0;
            if(RestData < ContOfClust)
            {
                pPClustDataBuf = pClustDataBuf;
                // The data we wanted may smaller than ClusterSize.
                // |--------Cluster-------|
                // |====wanted===---------|
                MemCpySizeVal = RestData;
                WriteInSizeVal = (MemCpySizeVal >> pBPBUnit->BitNumOfBPS) + ((MemCpySizeVal & pBPBUnit->BitRevrOfBPS)? 1: 0);
            }
            else
            {
                pPClustDataBuf = pDataBuf;
                // The data we wanted is equal to ClusterSize * n.
                // |--------Cluster-------|....|--------Cluster-------|
                // |========wanted========|....|========wanted========|
                MemCpySizeVal = ContOfClust;
                WriteInSizeVal = (MemCpySizeVal >> pBPBUnit->BitNumOfBPS);
            }
        }

        // Shift the Read address by Sector units.
        CurSector += i;
        if(PosRemainder || (MemCpySizeVal & pBPBUnit->BitRevrOfBPS))
        {
            //DEBUG_BLUE("[I] CS2: %#x, WS: %d\n", CurSector, WriteInSizeVal);
            if((err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, WriteInSizeVal, pPClustDataBuf)) < 0)
            {
                ERRD(FS_LB_MUL_READ_DAT_ERR);
                FSFree(pClustDataBuf);
                return err;
            }
        }

        if(PosRemainder > 0)
        {
            // |--------Cluster-------|....
            // |---------====wanted===|....
            memcpy(pPClustDataBuf + PosRemainder - (i << pBPBUnit->BitNumOfBPS), pDataBuf, MemCpySizeVal);
        }
        else
        {
            // |--------Cluster-------|....
            // |========wanted========|....
            if(RestData < ContOfClust)
                memcpy(pPClustDataBuf, pDataBuf, MemCpySizeVal);
            else
            {
                // We don't do the memcpy action if we use src buf to be the loading buf.
            }
        }

        pDataBuf += MemCpySizeVal;
        FilePosition += MemCpySizeVal;
        *pWriteSize += MemCpySizeVal;
        //DEBUG_BLUE("[I] CS3: %#x, WS: %d, MS: %d\n", CurSector, WriteInSizeVal, MemCpySizeVal);
        if((err = FS__lb_mul_write(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, WriteInSizeVal, pPClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_WRITE_DAT_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        // 2. 一邊寫cluster 一邊update entry info ?
        if(pFile->size < FilePosition)	// Update file size
            pFile->size = FilePosition;

#if (FS_FAT_FINAL_UPDATE == 0)
        if((err = FSFATFileEntryUpdate(pFile)) < 0)
        {
            ERRD(FS_FILE_ENT_UPDATE_ERR);
            FSFree(pClustDataBuf);
            return err;
        }
#endif

        Step = NumOfClust + (((PosRemainder + MemCpySizeVal) & (pBPBUnit->SizeOfCluster - 1))? 0: 1);	// + 2  go forward to next cluster, + 1 stay.

        // Calculate the new Quotient and Remainder.
        PosQuotient = (FilePosition >> pBPBUnit->BitNumOfSOC);
        if(PosQuotient)
            PosRemainder = FilePosition - (PosQuotient << pBPBUnit->BitNumOfSOC);
        else
            PosRemainder = FilePosition;
        RestData = (DataSize - *pWriteSize);
    }
    while(DataSize > *pWriteSize);

    pFile->filepos = FilePosition;
    pFile->NumOfPosClust = PosQuotient;

    FSFree(pClustDataBuf);
    return 1;
}
#else
int FS__fat_fwrite(FS_FILE *pFile, const void *pData, u32 DataSize, u32 *pWriteSize)
{
    FS__FAT_BPB *pBPBUnit;
    u32 CurCluster, CurSector, ClusterSize;
    u32 NumOfClust, WriteInSizeVal, MemCpySizeVal;
    u32 FilePosition, PosQuotient, PosRemainder, ClusterOfUnalloc, RestData;
    u32 TmpVal;
    int err, i, j;
    u8 *pClustDataBuf, *pPClustDataBuf, *pDataBuf;
    //
    if(!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;	// No valid pointer to a FS_FILE structure
    }
    if(DataSize == 0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }

    // Check if media is OK
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        pFile->error = FS_ERR_DISKCHANGED | 0x01;
        return -1;
    }
    else if (err < 0)
    {
        pFile->error = FS_ERR_WRITEERROR | 0x05;
        ERRD(FS_DEV_ACCESS_ERR);
        return err;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
    ClusterSize = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
    FilePosition = pFile->filepos;
    PosQuotient = FilePosition / ClusterSize;
    if(PosQuotient)
        PosRemainder = FilePosition - (PosQuotient * ClusterSize);
    else
        PosRemainder = FilePosition;
    pDataBuf = (u8 *)pData;
    *pWriteSize = 0;
    RestData = (DataSize - *pWriteSize);

    if(pFile->size > FilePosition)
    {
        // There has some space at the end cluster or rewrite old data.
        if((pFile->NumOfPosClust == PosQuotient) && (pFile->EOFClust == pFile->CurClust))
            CurCluster = pFile->EOFClust;
        else
            CurCluster = pFile->fileid_hi;
        ClusterOfUnalloc = PosQuotient + ((PosRemainder) ? 1: 0);
    }
    else
    {
        TmpVal = pFile->EOFClust;
        ClusterOfUnalloc = (RestData / ClusterSize);
        ClusterOfUnalloc += (RestData > (ClusterOfUnalloc * ClusterSize))? 1:0;	// Use comparison to saving src src more than use division
        if((pFile->size == 0x0) || PosRemainder)	// First time or there has some empty space
            ClusterOfUnalloc--;

        if(ClusterOfUnalloc > 0)
        {
            if((err = FSFATIncEntry(pFile->dev_index, pFile->fileid_lo, TmpVal, ClusterOfUnalloc, NULL, FS_E_CLUST_CLEAN_OFF)) < 0)
            {
                ERRD(FS_FAT_CLUT_ALLOC_ERR);
                return err;
            }

            // Update EOFCluster
            if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, pFile->EOFClust, 0, &pFile->EOFClust)) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                return err;
            }
        }

        CurCluster = TmpVal;
        if(pFile->size == 0x0)
            ClusterOfUnalloc = 1;
        else
            ClusterOfUnalloc = (PosRemainder) ? 1: 2;
    }

    if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    do
    {
        // Locate the current cluster
        if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, CurCluster, ClusterOfUnalloc, &CurCluster)) < 0)
        {
            ERRD(FS_FAT_CLUT_FIND_ERR);
            FSFree(pClustDataBuf);
            return err;
        }
        ClusterOfUnalloc = (RestData & (ClusterSize - 1)) ? 1: 2; // 2  go forward to next cluster, 1 stay.

        // Count the continuous value
        NumOfClust = 1;
        if(PosRemainder == 0x0)
        {
            for(j = 0, i = CurCluster; DataSize > (ClusterSize * (j + 1)); j++)
            {
                TmpVal = i;
                if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, TmpVal, 2, &i)) < 0)
                {
                    ERRD(FS_FAT_CLUT_FIND_ERR);
                    FSFree(pClustDataBuf);
                    return err;
                }

                if((i - TmpVal) == 1)
                    NumOfClust++;
                else
                    break;
            }

            // Only handle the area which is continuously and not resting data from size of cluster.
            if((RestData < (NumOfClust * ClusterSize)) && (NumOfClust > 1))
                NumOfClust--;
        }

        if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        pFile->CurClust = CurCluster;
        //DEBUG_BLUE("[I] CC: %#x, FP: %#x, NC: %d\n", CurCluster, FilePosition, NumOfClust);
        if(PosRemainder)
        {
            i = PosRemainder / pBPBUnit->BytesPerSec;
            pPClustDataBuf = pClustDataBuf;
            if(PosRemainder + DataSize < (NumOfClust * ClusterSize))
            {
                // Data may over the cluster boundary.
                // |--------Cluster-------|....|--------Cluster-------|		|--------Cluster-------|
                // |---------====wanted===|....|=======---------------|	or	|---------====wanted==-|
                MemCpySizeVal = DataSize;
            }
            else
            {
                // Data may over the cluster boundary.
                // |--------Cluster-------|....|		|--------Cluster-------|
                // |---------====wanted===|....|	or	|---------====wanted===|
                MemCpySizeVal = (NumOfClust * ClusterSize) - PosRemainder;
            }
            WriteInSizeVal = (MemCpySizeVal / pBPBUnit->BytesPerSec) + ((MemCpySizeVal % pBPBUnit->BytesPerSec)? 1: 0);
        }
        else
        {
            i = 0;
            if(RestData < (NumOfClust * ClusterSize))
            {
                pPClustDataBuf = pClustDataBuf;
                // The data we wanted may smaller than ClusterSize.
                // |--------Cluster-------|....|--------Cluster-------|		|--------Cluster-------|
                // |========wanted========|....|====wanted===---------|	or	|====wanted===---------|
                MemCpySizeVal = RestData;
                WriteInSizeVal = (MemCpySizeVal / pBPBUnit->BytesPerSec) + ((MemCpySizeVal % pBPBUnit->BytesPerSec)? 1: 0);
            }
            else
            {
                pPClustDataBuf = pDataBuf;
                // The data we wanted is equal to ClusterSize * n.
                // |--------Cluster-------|....|--------Cluster-------|
                // |========wanted========|....|========wanted========|
                MemCpySizeVal = (NumOfClust * ClusterSize);
                WriteInSizeVal = MemCpySizeVal / pBPBUnit->BytesPerSec;
            }
        }

        // Shift the Read address by Sector units.
        CurSector += i;
        if(PosRemainder || (MemCpySizeVal & (pBPBUnit->BytesPerSec - 1)))
        {
            //DEBUG_BLUE("[I] CS2: %#x, WS: %d\n", CurSector, WriteInSizeVal);
            if((err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, WriteInSizeVal, pPClustDataBuf)) < 0)
            {
                ERRD(FS_LB_MUL_READ_DAT_ERR);
                FSFree(pClustDataBuf);
                return err;
            }
        }

        if(PosRemainder)
        {
            // |--------Cluster-------|....
            // |---------====wanted===|....
            memcpy(pPClustDataBuf + PosRemainder - (i * pBPBUnit->BytesPerSec), pDataBuf, MemCpySizeVal);
        }
        else
        {
            // |--------Cluster-------|....
            // |========wanted========|....
            if(RestData < (NumOfClust * ClusterSize))
                memcpy(pPClustDataBuf, pDataBuf, MemCpySizeVal);
            else
            {
                // We don't do the memcpy action if we use src buf to be the loading buf.
            }
        }

        pDataBuf += MemCpySizeVal;
        FilePosition += MemCpySizeVal;
        *pWriteSize += MemCpySizeVal;
        //DEBUG_BLUE("[I] CS3: %#x, WS: %d, MS: %d\n", CurSector, WriteInSizeVal, MemCpySizeVal);
        if((err = FS__lb_mul_write(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, WriteInSizeVal, pPClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_WRITE_DAT_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        // 2. 一邊寫cluster 一邊update entry info ?
        if(pFile->size < FilePosition)	// Update file size
            pFile->size += MemCpySizeVal;

#if (FS_FAT_FINAL_UPDATE == 0)
        if((err = FSFATFileEntryUpdate(pFile)) < 0)
        {
            ERRD(FS_FILE_ENT_UPDATE_ERR);
            FSFree(pClustDataBuf);
            return err;
        }
#endif

        // Calculate the new Quotient and Remainder.
        PosQuotient = FilePosition / ClusterSize;
        if(PosQuotient)
            PosRemainder = FilePosition - (PosQuotient * ClusterSize);
        else
            PosRemainder = FilePosition;
        RestData = (DataSize - *pWriteSize);
    }
    while(DataSize > *pWriteSize);

    pFile->filepos = FilePosition;
    pFile->NumOfPosClust = PosQuotient;

    FSFree(pClustDataBuf);
    return 1;
}
#endif

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

int FS__fat_fclose(FS_FILE *pFile)
{
    int err;

    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }

    // Check if media is OK
    err = FS__lb_status(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo);
    if (err == FS_LBL_MEDIACHANGED)
    {
        pFile->error = FS_ERR_DISKCHANGED | 0x02;
        if(FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
            ERRD(FS_DEV_IOCTL_ERR);
        pFile->inuse = 0;
        return -1;
    }
    else if (err < 0)
    {
        pFile->error = FS_ERR_CLOSE | 0x00;
        if(FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
            ERRD(FS_DEV_IOCTL_ERR);
        pFile->inuse = 0;
        ERRD(FS_DEV_ACCESS_ERR);
        return err;
    }

    if (pFile->mode_w)      // civic 071120 for enhance performance
    {
#if FS_FAT_FINAL_UPDATE
        if((err = FSFATFileEntryUpdate(pFile)) < 0)
        {
            ERRD(FS_FILE_ENT_UPDATE_ERR);
            pFile->inuse = 0;
            return err;
        }
#endif

        if((err = FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_CLEAN_CACHE, 2, (void*)0)) < 0)
        {
            ERRD(FS_DEV_IOCTL_ERR);
            pFile->inuse = 0;
            return err;
        }

        //DEBUG_FS("FileClose:pFile->EOFClust=%d\n",pFile->EOFClust);
    }
    if (err < 0)
        pFile->error = FS_ERR_WRITEERROR | 0x06;
    pFile->inuse = 0;
    if(FS__lb_ioctl(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;
    }

    return 1;
}

