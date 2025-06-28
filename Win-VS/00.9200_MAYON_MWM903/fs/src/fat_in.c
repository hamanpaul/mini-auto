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
#if FS_BIT_WISE_OPERATION
int FS__fat_fread(FS_FILE *pFile, void *pData, u32 DataSize, u32 *pReadSize)
{
    FS__FAT_BPB *pBPBUnit;
    u32 CurCluster, CurSector, ClusterSize, NumOfClust, ReadOutSizeVal, MemCpySizeVal;
    u32 LimitOfReadCache;
    u32 FilePosition, PosQuotient, PosRemainder, DataRest;
    u32 *pClustListBuf;
    int err, i, j;
    u8 *pClustDataBuf, *pPClustDataBuf, *pDataBuf;
    //
    if(!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_FILE structure
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
        // Media has changed
        pFile->error = FS_ERR_DISKCHANGED | 0x00;
        return -1;
    }
    else if (err < 0)
    {
        // Media cannot be accessed
        pFile->error = FS_ERR_READERROR | 0x05;
        ERRD(FS_DEV_ACCESS_ERR);
        return err;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
    ClusterSize = pBPBUnit->SizeOfCluster;
    FilePosition = pFile->filepos;
    LimitOfReadCache = (FS_V_FAT_SECTOR_SIZE * FS_V_FAT_CLUSTER_NUMBER) >> 1;
    pDataBuf = pData;
    *pReadSize = 0;

    // Using global array to cache cluster list in order to saving get list time in every read operation.
    // Do sort out action at first time.
    if(FSReadCLCache.StartCluster != pFile->fileid_hi)
    {
        // Check the Entry info
        if(pFile->size == 0x0)
        {
            ERRD(FS_FILE_ENT_READ_ERR);
            return -1;
        }

        //	The distribution of CacheBuffer. Total 32KB.
        //	|=====16K=====|	-->	Middle point cluster of whole file.
        //	|=====16K=====|	--> Part list of whole file's cluster list. fisrt cluster is Middle point cluster.
        //
        //	The distribution of CacheBuffer.
        //	|=====16K=====|	--> 1st part list
        //	|=====16K=====|	--> 2nd Part list
        //	|=====16K=====|	--> 3th Part list
        //	|.............|	--> ... Part list

        // Set the figure of mid turn point.
        NumOfClust = (pFile->size >> pBPBUnit->BitNumOfSOC) + ((pFile->size & (pBPBUnit->SizeOfCluster - 1))? 1: 0);
        FSReadCLCache.NumOfMidPoint = (NumOfClust / (LimitOfReadCache >> 2)) + ((NumOfClust % (LimitOfReadCache >> 2))? 1: 0);
        FSReadCLCache.PosiOfMidPoint = 0;
        // Clear the Middle point cluster list buffer
        memset(FSReadCLCache.CacheBuffer, 0x0, LimitOfReadCache);
        // Collect the middle cluster value
        pClustListBuf = (u32 *) FSReadCLCache.CacheBuffer;
        for(i = 0, CurCluster = pFile->fileid_hi; i < FSReadCLCache.NumOfMidPoint; i++)
        {
            pClustListBuf[i] = CurCluster;
            //DEBUG_YELLOW("pClustListBuf[%d]: %#x\n", i, pClustListBuf[i]);
            if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, CurCluster, (LimitOfReadCache >> 2) + 1, &CurCluster)) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                return err;
            }
        }
        FSReadCLCache.StartCluster = pFile->fileid_hi;
        FSReadCLCache.PosiOfMidPoint = FSReadCLCache.NumOfMidPoint;	// Set any other value to trigger the first time loading.
    }

    if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)	// Choose the bigger one to be the caching buffer.
    {
    	ERRD(FS_MEMORY_ALLOC_ERR);
    	return -1;
    }
    do
    {
        DataRest = DataSize - *pReadSize;
        PosQuotient = (FilePosition >> pBPBUnit->BitNumOfSOC);
        if(PosQuotient)
            PosRemainder = FilePosition - (PosQuotient << pBPBUnit->BitNumOfSOC);
        else
            PosRemainder = FilePosition;

        // Compare the PosiOfMidPoint with FilePosition whether it needs to switch the Part list or not.
        NumOfClust = (PosQuotient / (LimitOfReadCache >> 2));
        if(FSReadCLCache.PosiOfMidPoint != NumOfClust)
        {
            pClustListBuf = (u32 *) FSReadCLCache.CacheBuffer;
            //DEBUG_GREEN("pClustListBuf[%d]: %#x\n", NumOfClust, pClustListBuf[NumOfClust]);
            // Clear the part list buffer
            memset(FSReadCLCache.CacheBuffer + LimitOfReadCache, 0x0, LimitOfReadCache);
            if((err = FSFATGoForwardClusterList(pFile->dev_index, pFile->fileid_lo, pClustListBuf[NumOfClust],
                                                (LimitOfReadCache >> 2), (u32 *)(FSReadCLCache.CacheBuffer + LimitOfReadCache))) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                FSFree(pClustDataBuf);
                return err;
            }
            FSReadCLCache.PosiOfMidPoint = NumOfClust;
        }
        //DEBUG_MAGENTA("NumOfClust: %#d\n", NumOfClust);

        // Reuse "pClustListBuf" to be the Memory buf locate. Load the part of ordered cluster list.
        pClustListBuf = (u32 *) (FSReadCLCache.CacheBuffer + LimitOfReadCache);
        // Reuse "NumOfClust" to be the Count of times of continuous clusters.
        NumOfClust = 1;
        i = PosQuotient - ((LimitOfReadCache >> 2) * FSReadCLCache.PosiOfMidPoint);
        if(PosRemainder == 0x0)
        {
            // Count the times of continuous clusters if ClusterSize is smaller than dataSize.
            for(j = 0; DataRest > (ClusterSize * (j + 1)); j++)
            {
                if((pClustListBuf[i+(j+1)] - pClustListBuf[i+j]) == 1)
                    NumOfClust++;
                else
                    break;
            }

            // Only handle the area which is continuously and not resting data from size of cluster.
            if((DataRest < (NumOfClust << pBPBUnit->BitNumOfSOC)) && (NumOfClust > 1))
                NumOfClust--;
        }

        //DEBUG_CYAN("idx:%d\n", i);
        CurCluster = pClustListBuf[i];
        //DEBUG_BLUE("FilePos: %#x, CurCluster: %#x\n", FilePosition, CurCluster);
        if(CurCluster == 0x0)
        {
            ERRD(FS_FAT_CLUT_SHIFT_ERR);
            DEBUG_FS("[W] FilePos: %#x\n", FilePosition);
            FSFree(pClustDataBuf);
            return -1;
        }

        if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustDataBuf);
            return -1;
        }

        pFile->CurClust = CurCluster;
        // Calculate the minimum sector that can read from mass storage.
        if(PosRemainder)
        {
            pPClustDataBuf = pClustDataBuf;
            i = (PosRemainder >> pBPBUnit->BitNumOfBPS);

            if(PosRemainder + DataRest < (ClusterSize * NumOfClust))
            {
                // Data may over the cluster boundary.
                // |--------Cluster-------|....|--------Cluster-------|		|--------Cluster-------|
                // |---------====wanted===|....|=======---------------|	or	|---------====wanted==-|
                MemCpySizeVal = DataRest;
            }
            else
            {
                // Data may over the cluster boundary.
                // |--------Cluster-------|....|		|--------Cluster-------|
                // |---------====wanted===|....|	or	|---------====wanted===|
                MemCpySizeVal = (NumOfClust << pBPBUnit->BitNumOfSOC) - PosRemainder;
            }
            ReadOutSizeVal = ((PosRemainder & pBPBUnit->BitRevrOfBPS) + MemCpySizeVal);
            ReadOutSizeVal = (ReadOutSizeVal >> pBPBUnit->BitNumOfBPS) + ((ReadOutSizeVal & pBPBUnit->BitRevrOfBPS)? 1: 0);
        }
        else
        {
            i = 0;
            if(DataRest < (NumOfClust << pBPBUnit->BitNumOfSOC))
            {
                pPClustDataBuf = pClustDataBuf;
                // The data we wanted may smaller than ClusterSize.
                // |--------Cluster-------|....|--------Cluster-------|		|--------Cluster-------|
                // |========wanted========|....|====wanted===---------|	or	|====wanted===---------|
                MemCpySizeVal = DataRest;
                ReadOutSizeVal = (MemCpySizeVal >> pBPBUnit->BitNumOfBPS) + ((MemCpySizeVal & pBPBUnit->BitRevrOfBPS)? 1: 0);
            }
            else
            {
                pPClustDataBuf = pDataBuf;
                // The data we wanted is equal to ClusterSize * n.
                // |--------Cluster-------|....|--------Cluster-------|
                // |========wanted========|....|========wanted========|
                MemCpySizeVal = (NumOfClust << pBPBUnit->BitNumOfSOC);
                ReadOutSizeVal = MemCpySizeVal >> pBPBUnit->BitNumOfBPS;
            }
        }

        // Shift the Read address by Sector units.
        CurSector += i;
        if((err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, ReadOutSizeVal, pPClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_READ_DAT_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        if(PosRemainder)
        {
            // |--------Cluster-------|....
            // |---------====wanted===|....
            memcpy(pDataBuf, pPClustDataBuf + PosRemainder - (i << pBPBUnit->BitNumOfBPS), MemCpySizeVal);
        }
        else
        {
            // |--------Cluster-------|....
            // |========wanted========|....
            if(DataRest < (NumOfClust << pBPBUnit->BitNumOfSOC))
                memcpy(pDataBuf, pPClustDataBuf, MemCpySizeVal);
            else
            {
                // We don't do the memcpy action if we use src buf to be the loading buf.
            }
        }

        pDataBuf += MemCpySizeVal;
        FilePosition += MemCpySizeVal;
        *pReadSize += MemCpySizeVal;
    }
    while(DataSize > *pReadSize);

    pFile->filepos = FilePosition;
    FSFree(pClustDataBuf);
    return 1;
}

#else
int FS__fat_fread(FS_FILE *pFile, void *pData, u32 DataSize, u32 *pReadSize)
{
    FS__FAT_BPB *pBPBUnit;
    u32 CurCluster, CurSector, ClusterSize, NumOfClust, ReadOutSizeVal, MemCpySizeVal;
    u32 LimitOfReadCache;
    u32 FilePosition, PosQuotient, PosRemainder, DataRest;
    u32 *pClustListBuf;
    int err, i, j;
    u8 *pClustDataBuf, *pPClustDataBuf, *pDataBuf;
    //
    if(!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No valid pointer to a FS_FILE structure
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
        // Media has changed
        pFile->error = FS_ERR_DISKCHANGED | 0x00;
        return -1;
    }
    else if (err < 0)
    {
        // Media cannot be accessed
        pFile->error = FS_ERR_READERROR | 0x05;
        ERRD(FS_DEV_ACCESS_ERR);
        return err;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
    ClusterSize = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
    FilePosition = pFile->filepos;
    LimitOfReadCache = (FS_V_FAT_SECTOR_SIZE * FS_V_FAT_CLUSTER_NUMBER) >> 1;
    pDataBuf = pData;
    *pReadSize = 0;

    // Using global array to cache cluster list in order to saving get list time in every read operation.
    // Do sort out action at first time.
    if(FSReadCLCache.StartCluster != pFile->fileid_hi)
    {
        // Check the Entry info
        if(pFile->size == 0x0)
        {
            ERRD(FS_FILE_ENT_READ_ERR);
            return -1;
        }

        //	The distribution of CacheBuffer. Total 32KB.
        //	|=====16K=====|	-->	Middle point cluster of whole file.
        //	|=====16K=====|	--> Part list of whole file's cluster list. fisrt cluster is Middle point cluster.
        //
        //	The distribution of CacheBuffer.
        //	|=====16K=====|	--> 1st part list
        //	|=====16K=====|	--> 2nd Part list
        //	|=====16K=====|	--> 3th Part list
        //	|.............|	--> ... Part list

        // Set the figure of mid turn point.
        NumOfClust = (pFile->size / ClusterSize) + ((pFile->size % ClusterSize)? 1: 0);
        FSReadCLCache.NumOfMidPoint = (NumOfClust / (LimitOfReadCache >> 2)) + ((NumOfClust % (LimitOfReadCache >> 2))? 1: 0);
        FSReadCLCache.PosiOfMidPoint = 0;
        // Clear the Middle point cluster list buffer
        memset(FSReadCLCache.CacheBuffer, 0x0, LimitOfReadCache);
        // Collect the middle cluster value
        pClustListBuf = (u32 *) FSReadCLCache.CacheBuffer;
        for(i = 0, CurCluster = pFile->fileid_hi; i < FSReadCLCache.NumOfMidPoint; i++)
        {
            pClustListBuf[i] = CurCluster;
            //DEBUG_YELLOW("pClustListBuf[%d]: %#x\n", i, pClustListBuf[i]);
            if((err = FSFATGoForwardCluster(pFile->dev_index, pFile->fileid_lo, CurCluster, (LimitOfReadCache >> 2) + 1, &CurCluster)) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                return err;
            }
        }
        FSReadCLCache.StartCluster = pFile->fileid_hi;
        FSReadCLCache.PosiOfMidPoint = FSReadCLCache.NumOfMidPoint;	// Set any other value to trigger the first time loading.
    }

    if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)	// Choose the bigger one to be the caching buffer.
    {
    	ERRD(FS_MEMORY_ALLOC_ERR);
    	return -1;
    }
    do
    {
        DataRest = DataSize - *pReadSize;
        PosQuotient = FilePosition / ClusterSize;
        if(PosQuotient)
            PosRemainder = FilePosition - (PosQuotient * ClusterSize);
        else
            PosRemainder = FilePosition;

        // Compare the PosiOfMidPoint with FilePosition whether it needs to switch the Part list or not.
        NumOfClust = (PosQuotient / (LimitOfReadCache >> 2));
        if(FSReadCLCache.PosiOfMidPoint != NumOfClust)
        {
            pClustListBuf = (u32 *) FSReadCLCache.CacheBuffer;
            //DEBUG_GREEN("pClustListBuf[%d]: %#x\n", NumOfClust, pClustListBuf[NumOfClust]);
            // Clear the part list buffer
            memset(FSReadCLCache.CacheBuffer + LimitOfReadCache, 0x0, LimitOfReadCache);
            if((err = FSFATGoForwardClusterList(pFile->dev_index, pFile->fileid_lo, pClustListBuf[NumOfClust],
                                                (LimitOfReadCache >> 2), (u32 *)(FSReadCLCache.CacheBuffer + LimitOfReadCache))) < 0)
            {
                ERRD(FS_FAT_CLUT_FIND_ERR);
                FSFree(pClustDataBuf);
                return err;
            }
            FSReadCLCache.PosiOfMidPoint = NumOfClust;
        }
        //DEBUG_MAGENTA("NumOfClust: %#d\n", NumOfClust);

        // Reuse "pClustListBuf" to be the Memory buf locate. Load the part of ordered cluster list.
        pClustListBuf = (u32 *) (FSReadCLCache.CacheBuffer + LimitOfReadCache);
        // Reuse "NumOfClust" to be the Count of times of continuous clusters.
        NumOfClust = 1;
        i = PosQuotient - ((LimitOfReadCache >> 2) * FSReadCLCache.PosiOfMidPoint);
        if(PosRemainder == 0x0)
        {
            // Count the times of continuous clusters if ClusterSize is smaller than dataSize.
            for(j = 0; DataRest > (ClusterSize * (j + 1)); j++)
            {
                if((pClustListBuf[i+(j+1)] - pClustListBuf[i+j]) == 1)
                    NumOfClust++;
                else
                    break;
            }

            // Only handle the area which is continuously and not resting data from size of cluster.
            if((DataRest < (NumOfClust * ClusterSize)) && (NumOfClust > 1))
                NumOfClust--;
        }

        //DEBUG_CYAN("idx:%d\n", i);
        CurCluster = pClustListBuf[i];
        //DEBUG_BLUE("FilePos: %#x, CurCluster: %#x\n", FilePosition, CurCluster);
        if(CurCluster == 0x0)
        {
            ERRD(FS_FAT_CLUT_SHIFT_ERR);
            DEBUG_FS("[W] FilePos: %#x\n", FilePosition);
            FSFree(pClustDataBuf);
            return -1;
        }

        if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
        {
            ERRD(FS_FAT_SEC_CAL_ERR);
            FSFree(pClustDataBuf);
            return -1;
        }

        pFile->CurClust = CurCluster;
        // Calculate the minimum sector that can read from mass storage.
        if(PosRemainder)
        {
            pPClustDataBuf = pClustDataBuf;
            i = PosRemainder / pBPBUnit->BytesPerSec;

            if(PosRemainder + DataSize < (ClusterSize * NumOfClust))
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
                MemCpySizeVal = (ClusterSize * NumOfClust) - PosRemainder;
            }
            ReadOutSizeVal = (MemCpySizeVal / pBPBUnit->BytesPerSec) + ((MemCpySizeVal % pBPBUnit->BytesPerSec)? 1: 0);
        }
        else
        {
            i = 0;
            if(DataRest < (ClusterSize * NumOfClust))
            {
                pPClustDataBuf = pClustDataBuf;
                // The data we wanted may smaller than ClusterSize.
                // |--------Cluster-------|....|--------Cluster-------|		|--------Cluster-------|
                // |========wanted========|....|====wanted===---------|	or	|====wanted===---------|
                MemCpySizeVal = DataRest;
                ReadOutSizeVal = (MemCpySizeVal / pBPBUnit->BytesPerSec) + ((MemCpySizeVal % pBPBUnit->BytesPerSec)? 1: 0);
            }
            else
            {
                pPClustDataBuf = pDataBuf;
                // The data we wanted is equal to ClusterSize * n.
                // |--------Cluster-------|....|--------Cluster-------|
                // |========wanted========|....|========wanted========|
                MemCpySizeVal = (ClusterSize * NumOfClust);
                ReadOutSizeVal = MemCpySizeVal / pBPBUnit->BytesPerSec;
            }
        }

        // Shift the Read address by Sector units.
        CurSector += i;
        if((err = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, ReadOutSizeVal, pPClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_READ_DAT_ERR);
            FSFree(pClustDataBuf);
            return err;
        }

        if(PosRemainder)
        {
            // |--------Cluster-------|....
            // |---------====wanted===|....
            memcpy(pDataBuf, pPClustDataBuf + PosRemainder - (i * pBPBUnit->BytesPerSec), MemCpySizeVal);
        }
        else
        {
            // |--------Cluster-------|....
            // |========wanted========|....
            if(DataRest < (ClusterSize * NumOfClust))
                memcpy(pDataBuf, pPClustDataBuf, MemCpySizeVal);
            else
            {
                // We don't do the memcpy action if we use src buf to be the loading buf.
            }
        }

        pDataBuf += MemCpySizeVal;
        FilePosition += MemCpySizeVal;
        *pReadSize += MemCpySizeVal;
    }
    while(DataSize > *pReadSize);

    pFile->filepos = FilePosition;
    FSFree(pClustDataBuf);
    return 1;
}
#endif

