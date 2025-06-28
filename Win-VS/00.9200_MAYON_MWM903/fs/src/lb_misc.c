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
File        : lb_misc.c
Purpose     : Logical Block Layer
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
#include "general.h"

#include "fsapi.h"
#include "fs_os.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_clib.h"
#include "fs_fat.h"

#include "sysapi.h"
#include "sdcapi.h"

/*********************************************************************
*
*            Constant
*
**********************************************************************/
#define R_W_RETRY_COUNT_MAX 3
#define FS_LB_CACHE_THRESHOLD 1000
#define FS_LB_CACHE_SHOWHIT	0

/*********************************************************************
*
*             Extern Global Variable
*
**********************************************************************/

extern u32 sdcTryInvertSDClk;

/*********************************************************************
*
*             Global Variable
*
**********************************************************************/

u32 TotalHits, HistoryTHits;
u32 Hits, HistoryHits;
u8 MulWrCnt, MulWrOTCnt;

/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/


/*********************************************************************
*
*             _FS_LB_GetDriverIndex
*
  Description:
  FS internal function. Get index of a driver in the device information
  table referred by FS__pDevInfo.

  Parameters:
  pDriver     - Pointer to a device driver structure

  Return value:
  =>0         - Index of pDriver in the device information table.
  <0          - An error has occured.
*/

int _FS_LB_GetDriverIndex(const FS__device_type *pDriver)
{
    u32 i = 0;

    while(1)
    {
        if (i >= FS__maxdev)
        {
            break;  // Driver not found
        }
        if (FS__pDevInfo[i].devdriver == pDriver)
        {
            break;  // Driver found
        }
        i++;
    }

    if (i >= FS__maxdev)
    {
        return -1;
    }
    return i;
}

#if FS_USE_LB_READCACHE
#if FS_LB_CACHE_SHOWHIT
int FSLBCacheHitPercent(void)
{
    if(HistoryTHits != 0x0)
        return HistoryHits * 100 / HistoryTHits;

    if(TotalHits != 0x0)
        return Hits * 100 / TotalHits;
    return 0;
}

int FSLBCacheMatchStart(int HitNumber)
{
    TotalHits += HitNumber;
    return 1;
}

int FSLBCacheHit(void)
{
    u32 Divisor;
    ++Hits;

    if(TotalHits > 1000)
    {
        HistoryTHits += TotalHits;
        HistoryHits += Hits;
        if(HistoryTHits >= 1000)
        {
            Divisor = HistoryTHits / 1000;
            HistoryTHits /= Divisor;
            HistoryHits /= Divisor;
        }

        DEBUG_MAGENTA("[I] Cache hit rate: H:%d N:(%d / %d)\n", FSLBCacheHitPercent(), Hits, TotalHits);

        TotalHits = Hits = 0;
    }
    return 1;
}
#endif

void FSLBCacheInit(int Idx, u32 Unit)
{
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;

    MulWrCnt = 0;
    MulWrOTCnt = 0;

    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];

    if(pDevCacheInfo)
    {
        pDevCacheInfo->CacheIndex = FS_LB_CACHE_SIGNFLAG;
        for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
        {
            pDevCacheInfo->pCache[i].BlockId = FS_LB_CACHE_SIGNFLAG;
            pDevCacheInfo->pCache[i].Dirty = 0x0;
            pDevCacheInfo->pCache[i].Popularity = 0x0;
        }
    }
}

/*********************************************************************
*
*             _FS_LBR_CopyToCache
*
  Description:
  FS internal function. Copy a sector to the cache of the device while Read.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be copied to the device's cache.
  pBuffer     - Pointer to a data buffer to be stored in the cache.

  Return value:
  None.
*/
int FSLBSyncSinReadCache(int Idx, u32 Unit, u32 Sector, void *pBuffer)
{
    const FS__device_type *pDriver;
    FS__FAT_BPB *pBPBUnit;
    FS__LB_CACHE *pDevCacheInfo;
    u32 EmptyFlag, CacheIndex;
    u32 i, LowestVal;
    int ret;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];
    pDriver = FS__pDevInfo[Idx].devdriver;

    // It means the cache module doesn't ready to cache memory.
    if(pDevCacheInfo->CacheIndex == FS_LB_CACHE_SIGNFLAG)
    {
        //ERRD(FS_LB_CACHE_INIT_ERR);
        return 0;
    }

    // Only cache the blocks of FAT1.
    if(!((pBPBUnit->FSInfo <= Sector) && (Sector < pBPBUnit->FatEndSec)))
        return 0;

#if FS_LB_CACHE_SHOWHIT
    FSLBCacheMatchStart(1);
#endif
    EmptyFlag = FS_LB_CACHE_SIGNFLAG;
    for(i = 0, ret = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        // Decreasing the pop
        if(pDevCacheInfo->pCache[i].Popularity != 0x0)
            pDevCacheInfo->pCache[i].Popularity--;

        if((pDevCacheInfo->pCache[i].BlockId == FS_LB_CACHE_SIGNFLAG) && (EmptyFlag == FS_LB_CACHE_SIGNFLAG))
            EmptyFlag = i;

        if(Sector == pDevCacheInfo->pCache[i].BlockId)
        {
#if FS_LB_CACHE_SHOWHIT
            FSLBCacheHit();
#endif
            FS__CLIB_memcpy(pBuffer, pDevCacheInfo->pCache[i].aBlockData, pBPBUnit->BytesPerSec);
            pDevCacheInfo->pCache[i].Popularity += 2;	// +2 -1 = 1
            if(pDevCacheInfo->pCache[i].Popularity > FS_LB_CACHE_THRESHOLD)
                pDevCacheInfo->pCache[i].Popularity = FS_LB_CACHE_THRESHOLD;
            ret++;
        }
    }

    // Sector has been found already.
    if(ret)
        return 1;

    if(EmptyFlag != FS_LB_CACHE_SIGNFLAG)
    {
        ret = (pDriver->dev_read)(Unit, Sector, pBuffer);
        ret = FS__lb_SD_RDretry(pDriver, Unit, Sector, 1, pBuffer, ret);
        if(ret < 0)
        {
            DEBUG_RED("[WARM] Unit: %d, Sector: %#x\n", Unit, Sector);
            ERRD(FS_LB_READ_DAT_ERR);
            return ret;
        }

        FS__CLIB_memcpy(pDevCacheInfo->pCache[EmptyFlag].aBlockData, pBuffer, pBPBUnit->BytesPerSec);
        pDevCacheInfo->CacheIndex = EmptyFlag;
        pDevCacheInfo->pCache[EmptyFlag].BlockId = Sector;
        pDevCacheInfo->pCache[EmptyFlag].Dirty = 0;
        pDevCacheInfo->pCache[EmptyFlag].Popularity = 1;
        return 1;
    }

    // Select the lowest pop block to be replaced by new one.
    CacheIndex = i = pDevCacheInfo->CacheIndex;
    LowestVal = pDevCacheInfo->pCache[i].Popularity;
    do
    {
        if(LowestVal > pDevCacheInfo->pCache[i].Popularity)
        {
            CacheIndex = i;
            LowestVal = pDevCacheInfo->pCache[i].Popularity;
        }

        i = (i + 1) % pDevCacheInfo->MaxCacheNum;
    }
    while(i != pDevCacheInfo->CacheIndex);

    ret = (pDriver->dev_read)(Unit, Sector, pBuffer);
    ret = FS__lb_SD_RDretry(pDriver, Unit, Sector, 1, pBuffer, ret);
    if(ret < 0)
    {
        DEBUG_RED("[WARM] Unit: %d, Sector: %#x\n", Unit, Sector);
        ERRD(FS_LB_READ_DAT_ERR);
        return ret;
    }

    if(pDevCacheInfo->pCache[CacheIndex].Dirty == 0x0)
    {
        FS__CLIB_memcpy(pDevCacheInfo->pCache[CacheIndex].aBlockData, pBuffer, pBPBUnit->BytesPerSec);
        pDevCacheInfo->pCache[CacheIndex].BlockId = Sector;
        pDevCacheInfo->pCache[CacheIndex].Dirty = 0;
        pDevCacheInfo->pCache[CacheIndex].Popularity = 1;
    }

    pDevCacheInfo->CacheIndex = (pDevCacheInfo->CacheIndex + 1) % pDevCacheInfo->MaxCacheNum;
    return 1;
}

/*********************************************************************
*
*             _FS_LBW_CopyToCache
*
  Description:
  FS internal function. Copy a sector to the cache of the device while write.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be copied to the device's cache.
  pBuffer     - Pointer to a data buffer to be stored in the cache.

  Return value:
  None.
*/
int FSLBSyncSinWriteCache(int Idx, u32 Unit, u32 Sector, void *pBuffer)
{
    const FS__device_type *pDriver;
    FS__FAT_BPB *pBPBUnit;
    FS__LB_CACHE *pDevCacheInfo;
    u32 EmptyFlag, CacheIndex;
    u32 i, LowestVal;
    int ret;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];
    pDriver = FS__pDevInfo[Idx].devdriver;

    // It means the cache module doesn't ready to cache memory.
    if(pDevCacheInfo->CacheIndex == FS_LB_CACHE_SIGNFLAG)
    {
        //ERRD(FS_LB_CACHE_INIT_ERR);
        return 0;
    }

    // Only cache the blocks of FAT1.
    if(!((pBPBUnit->FSInfo <= Sector) && (Sector < pBPBUnit->FatEndSec)))
        return 0;

#if FS_LB_CACHE_SHOWHIT
    FSLBCacheMatchStart(1);
#endif
    EmptyFlag = FS_LB_CACHE_SIGNFLAG;
    for(i = 0, ret = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        // Decreasing the pop
        if(pDevCacheInfo->pCache[i].Popularity != 0x0)
            pDevCacheInfo->pCache[i].Popularity--;

        if((pDevCacheInfo->pCache[i].BlockId == FS_LB_CACHE_SIGNFLAG) && (EmptyFlag == FS_LB_CACHE_SIGNFLAG))
            EmptyFlag = i;

        if(Sector == pDevCacheInfo->pCache[i].BlockId)
        {
#if FS_LB_CACHE_SHOWHIT
            FSLBCacheHit();
#endif
            FS__CLIB_memcpy(pDevCacheInfo->pCache[i].aBlockData, pBuffer, pBPBUnit->BytesPerSec);
            pDevCacheInfo->pCache[i].Dirty = 1;	// The data is async~
            pDevCacheInfo->pCache[i].Popularity += 2;	// -1 +2 = +1

            // Saving data timely to keep the data wasn't async too far
            if(pDevCacheInfo->pCache[i].Popularity > FS_LB_CACHE_THRESHOLD)
            {
                pDevCacheInfo->pCache[i].Popularity /= 10;	// Falling the pop, raising others
                ret = (pDriver->dev_write)(Unit, pDevCacheInfo->pCache[i].BlockId, pDevCacheInfo->pCache[i].aBlockData);
                ret = FS__lb_SD_WRretry(pDriver, Unit, pDevCacheInfo->pCache[i].BlockId, 1, pDevCacheInfo->pCache[i].aBlockData, ret);
                if(ret < 0)
                {
                    DEBUG_RED("[WARM] Unit: %d, BlockID: %#x\n", Unit, pDevCacheInfo->pCache[i].BlockId);
                    ERRD(FS_LB_WRITE_DAT_ERR);
                    return ret;
                }
                pDevCacheInfo->pCache[i].Dirty = 0;
            }
            ret++;	// Not got off right now, others pop need to dec.
        }
    }

    // Sector has been found already.
    if(ret)
        return 1;

    if(EmptyFlag != FS_LB_CACHE_SIGNFLAG)
    {
        FS__CLIB_memcpy(pDevCacheInfo->pCache[EmptyFlag].aBlockData, pBuffer, pBPBUnit->BytesPerSec);
        pDevCacheInfo->CacheIndex = EmptyFlag;
        pDevCacheInfo->pCache[EmptyFlag].BlockId = Sector;
        pDevCacheInfo->pCache[EmptyFlag].Dirty = 1;	// The data is async~
        pDevCacheInfo->pCache[EmptyFlag].Popularity = 1;
        return 1;
    }

    // Select the lowest pop block to be replaced by new one.
    CacheIndex = i = pDevCacheInfo->CacheIndex;
    LowestVal = pDevCacheInfo->pCache[i].Popularity;
    do
    {
        if(LowestVal > pDevCacheInfo->pCache[i].Popularity)
        {
            CacheIndex = i;
            LowestVal = pDevCacheInfo->pCache[i].Popularity;
        }

        i = (i + 1) % pDevCacheInfo->MaxCacheNum;
    }
    while(i != pDevCacheInfo->CacheIndex);

    if(pDevCacheInfo->pCache[CacheIndex].Dirty == 1)
    {
        ret = (pDriver->dev_write)(Unit, pDevCacheInfo->pCache[CacheIndex].BlockId, pDevCacheInfo->pCache[CacheIndex].aBlockData);
        ret = FS__lb_SD_WRretry(pDriver, Unit, pDevCacheInfo->pCache[CacheIndex].BlockId, 1, pDevCacheInfo->pCache[CacheIndex].aBlockData, ret);
        if(ret < 0)
        {
            DEBUG_RED("[WARM] Unit: %d, BlockID: %#x\n", Unit, pDevCacheInfo->pCache[CacheIndex].BlockId);
            ERRD(FS_LB_WRITE_DAT_ERR);
            return ret;
        }
    }

    FS__CLIB_memcpy(pDevCacheInfo->pCache[CacheIndex].aBlockData, pBuffer, pBPBUnit->BytesPerSec);
    pDevCacheInfo->CacheIndex = (pDevCacheInfo->CacheIndex + 1) % pDevCacheInfo->MaxCacheNum;
    pDevCacheInfo->pCache[CacheIndex].BlockId = Sector;
    pDevCacheInfo->pCache[CacheIndex].Dirty = 1;
    pDevCacheInfo->pCache[CacheIndex].Popularity = 1;

    return 1;
}

int FSLBSyncMulReadCache(int Idx, u32 Unit, u32 Sector, u32 NumOfSector, void *pBuffer)
{
    FS__FAT_BPB *pBPBUnit;
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];

    // It means the cache module doesn't ready to cache memory.
    if(pDevCacheInfo->CacheIndex == FS_LB_CACHE_SIGNFLAG)
    {
        //ERRD(FS_LB_CACHE_INIT_ERR);
        return 0;
    }

    // Only cache the blocks of FAT1. One in, all in.
    if(!((pBPBUnit->FSInfo <= Sector) && (Sector < pBPBUnit->FatEndSec)))	// One in, all in.
        return 0;

#if FS_LB_CACHE_SHOWHIT
    FSLBCacheMatchStart(NumOfSector);
#endif

    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        // Decreasing the pop
        if(pDevCacheInfo->pCache[i].Popularity != 0x0)
            pDevCacheInfo->pCache[i].Popularity--;
    }

    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        if((Sector <= pDevCacheInfo->pCache[i].BlockId) && (pDevCacheInfo->pCache[i].BlockId <= (Sector + NumOfSector)))
        {
#if FS_LB_CACHE_SHOWHIT
            FSLBCacheHit();
#endif
            // Sector <= BlockID <= (Sector + NumOfSector), So Shift value equals BlockID - Sector
            FS__CLIB_memcpy((u8 *)pBuffer + ((pDevCacheInfo->pCache[i].BlockId - Sector) * pBPBUnit->BytesPerSec),
                            pDevCacheInfo->pCache[i].aBlockData,
                            pBPBUnit->BytesPerSec);
            pDevCacheInfo->pCache[i].Popularity += 2;	// +2 -1 = 1
            if(pDevCacheInfo->pCache[i].Popularity > FS_LB_CACHE_THRESHOLD)
                pDevCacheInfo->pCache[i].Popularity = FS_LB_CACHE_THRESHOLD;
        }
    }

    return 1;
}

int FSLBSyncMulWriteCache(int Idx, u32 Unit, u32 Sector, u32 NumOfSector, void *pBuffer)
{
    FS__FAT_BPB *pBPBUnit;
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];

    // It means the cache module doesn't ready to cache memory.
    if(pDevCacheInfo->CacheIndex == FS_LB_CACHE_SIGNFLAG)
    {
        //ERRD(FS_LB_CACHE_INIT_ERR);
        return 0;
    }

    // Only cache the blocks of FAT1. One in, all in.
    if(!((pBPBUnit->FSInfo <= Sector) && (Sector < pBPBUnit->FatEndSec)))
        return 0;

#if FS_LB_CACHE_SHOWHIT
    FSLBCacheMatchStart(NumOfSector);
#endif
    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        // Decreasing the pop
        if(pDevCacheInfo->pCache[i].Popularity != 0x0)
            pDevCacheInfo->pCache[i].Popularity--;
    }

    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        if((Sector <= pDevCacheInfo->pCache[i].BlockId) && (pDevCacheInfo->pCache[i].BlockId <= (Sector + NumOfSector)))
        {
#if FS_LB_CACHE_SHOWHIT
            FSLBCacheHit();
#endif
            // Sector <= BlockID <= (Sector + NumOfSector), So Shift value equals BlockID - Sector
            FS__CLIB_memcpy(pDevCacheInfo->pCache[i].aBlockData,
                            (u8 *)pBuffer + ((pDevCacheInfo->pCache[i].BlockId - Sector) * pBPBUnit->BytesPerSec),
                            pBPBUnit->BytesPerSec);
            pDevCacheInfo->pCache[i].Popularity += 2;	// +2 -1 = 1
            if(pDevCacheInfo->pCache[i].Popularity > FS_LB_CACHE_THRESHOLD)
                pDevCacheInfo->pCache[i].Popularity = FS_LB_CACHE_THRESHOLD;
        }
    }

    return 1;
}

int FSLBUpdateCacheDirty(int Idx, u32 Unit, u32 Sector, u32 NumOfSector)
{
    FS__FAT_BPB *pBPBUnit;
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];

    // It means the cache module doesn't ready to cache memory.
    if(pDevCacheInfo->CacheIndex == FS_LB_CACHE_SIGNFLAG)
    {
        //ERRD(FS_LB_CACHE_INIT_ERR);
        return 0;
    }

    // Only cache the blocks of FAT1. One in, all in.
    if(!((pBPBUnit->FSInfo <= Sector) && (Sector < pBPBUnit->FatEndSec)))	// One in, all in.
        return 0;

    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        // Sector <= BlockID <= (Sector + NumOfSector), So Shift value equals BlockID - Sector
        if((Sector <= pDevCacheInfo->pCache[i].BlockId) && (pDevCacheInfo->pCache[i].BlockId <= (Sector + NumOfSector)))
            pDevCacheInfo->pCache[i].Dirty = 0x0;
    }

    return 1;
}

int FSLBSyncAllCache(int Idx, u32 Unit)
{
    const FS__device_type *pDriver;
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;
    int ret;

    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];
    pDriver = FS__pDevInfo[Idx].devdriver;

    for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
    {
        if(pDevCacheInfo->pCache[i].Dirty == 0x1)
        {
            ret = (pDriver->dev_write)(Unit, pDevCacheInfo->pCache[i].BlockId, pDevCacheInfo->pCache[i].aBlockData);
            ret = FS__lb_SD_WRretry(pDriver, Unit, pDevCacheInfo->pCache[i].BlockId, 1, pDevCacheInfo->pCache[i].aBlockData, ret);
            if(ret < 0)
            {
                DEBUG_RED("[WARM] Unit: %d, BlockID: %#x\n", Unit, pDevCacheInfo->pCache[i].BlockId);
                ERRD(FS_LB_WRITE_DAT_ERR);
                return ret;
            }
            pDevCacheInfo->pCache[i].Dirty = 0x0;
        }
    }

    return 1;
}

/*********************************************************************
*
*             _FS_LB_ClearCache
*
  Description:
  FS internal function. Clear cache of a device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.

  Return value:
  None.
*/
int FSLBClearCache(int Idx, u32 Unit)
{
    FS__LB_CACHE *pDevCacheInfo;
    u32 i;

    MulWrCnt = 0;
    MulWrOTCnt = 0;

    pDevCacheInfo = &FS__pDevInfo[Idx].pDevCacheInfo[Unit];

    if(FS__pDevInfo[Idx].pDevCacheInfo)
    {
        pDevCacheInfo->CacheIndex = FS_LB_CACHE_SIGNFLAG;
        for(i = 0; i < pDevCacheInfo->MaxCacheNum; i++)
        {
            pDevCacheInfo->pCache[i].BlockId = FS_LB_CACHE_SIGNFLAG;
            pDevCacheInfo->pCache[i].Dirty = 0;
            pDevCacheInfo->pCache[i].Popularity = 0;
        }
    }

    return 1;
}

int FS_LB_Cache_Init(int idx, u32 Unit)
{
    FS_X_OS_LockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);
    Hits = TotalHits = HistoryTHits = HistoryHits = 0;
    FSLBCacheInit(idx, Unit);
    FS_X_OS_UnlockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);

    return 1;
}

int FS_LB_Cache_Enable(int idx, u32 Unit)
{
    FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex = 0;
    return 1;
}

int FS_LB_Cache_Disable(int idx, u32 Unit)
{
    FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex = FS_LB_CACHE_SIGNFLAG;
    return 1;
}

int FS_LB_Cache_Clean(int idx, u32 Unit)
{
    int ret;

    FS_X_OS_LockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);
    ret = FSLBSyncAllCache(idx, Unit);
    FS_X_OS_UnlockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);

    return ret;
}

int FS_LB_Cache_Clear(int idx, u32 Unit)
{
    //int ret;
    FS_X_OS_LockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);
    //ret = FSLBClearCache(idx, Unit);
    FSLBCacheInit(idx, Unit);
    FS_X_OS_UnlockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);

    return 1;
}
#endif  // FS_USE_LB_READCACHE

/*********************************************************************
*
*             Global functions
*
**********************************************************************

  Functions here are global, although their names indicate a local
  scope. They should not be called by user application.
*/

int FS__LB_Init(void)
{
    return 1;
}

/*********************************************************************
*
*             FS__lb_status
*
  Description:
  FS internal function. Get status of a device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.

  Return value:
  ==1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
  ==0                       - Device okay and ready for operation.
  <0                        - An error has occured.
*/

int FS__lb_status(const FS__device_type *pDriver, u32 Unit)
{
    int result, idx;

    if(!pDriver->dev_status)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if ((idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    result = (pDriver->dev_status)(Unit);
    if(result < 0)
        ERRD(FS_LB_STATUS_GET_ERR);

#if FS_USE_LB_READCACHE
    if(result != 0)
        FSLBCacheInit(idx, Unit);
#endif
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    return result;
}


/*********************************************************************
*
*             FS__lb_read
*
  Description:
  FS internal function. Read sector from device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be read from the device.
  pBuffer     - Pointer to buffer for storing the data.

  Return value:
  ==0         - Sector has been read and copied to pBuffer.
  <0          - An error has occured.
*/

int FS__lb_read(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int result, idx;

    if(!pDriver->dev_read)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if((idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
#if FS_USE_LB_READCACHE
    result = FSLBSyncSinReadCache(idx, Unit, Sector, pBuffer);
    if(result != 0)
    {
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return result;
    }
#endif

    result = (pDriver->dev_read)(Unit, Sector, pBuffer);
    result = FS__lb_SD_RDretry(pDriver, Unit, Sector, 1, pBuffer, result);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if(result < 0)
        ERRD(FS_LB_READ_DAT_ERR);
    return result;
}

int FS__lb_read_Direct(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int result;

    if(!pDriver->dev_read)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    result = (pDriver->dev_read)(Unit, Sector, pBuffer);
    result = FS__lb_SD_RDretry(pDriver, Unit, Sector, 1, pBuffer, result);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if(result < 0)
        ERRD(FS_LB_DIR_READ_DAT_ERR);
    return result;
}

/*********************************************************************
*
*             FS__lb_mul_read
*
  Description:
  FS internal function. Read sector from device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be read from the device.
  pBuffer     - Pointer to buffer for storing the data.

  Return value:
  ==0         - Sector has been read and copied to pBuffer.
  <0          - An error has occured.
*/

int FS__lb_mul_read(const FS__device_type *pDriver, u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    int ret, Idx;

    if(!pDriver->dev_mul_read)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if((Idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    ret = (pDriver->dev_mul_read)(Unit, Sector, NumofSector, pBuffer);
    ret = FS__lb_SD_RDretry(pDriver, Unit, Sector, NumofSector, pBuffer, ret);
    FSLBSyncMulReadCache(Idx, Unit, Sector, NumofSector, pBuffer);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if (ret < 0)
        ERRD(FS_LB_MUL_READ_DAT_ERR);
    return ret;
}
/*********************************************************************
*
*             FS__lb_write
*
  Description:
  FS internal function. Write sector to device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be written to the device.
  pBuffer     - Pointer to data to be stored.

  Return value:
  ==0         - Sector has been written to the device.
  <0          - An error has occured.
*/

int FS__lb_write(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int result, idx;

    if(!pDriver->dev_write)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if((idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
#if FS_USE_LB_READCACHE
    result = FSLBSyncSinWriteCache(idx, Unit, Sector, pBuffer);
    if(result != 0)
    {
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return result;
    }
#endif
    result = (pDriver->dev_write)(Unit, Sector, pBuffer);
    result = FS__lb_SD_WRretry(pDriver, Unit, Sector, 1, pBuffer, result);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if (result < 0)
        ERRD(FS_LB_WRITE_DAT_ERR);
    return result;
}

int FS__lb_write_Direct(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int result;

    if(!pDriver->dev_write)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    result = (pDriver->dev_write)(Unit, Sector, pBuffer);
    result = FS__lb_SD_WRretry(pDriver, Unit, Sector, 1, pBuffer, result);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if (result < 0)
        ERRD(FS_LB_DIR_WRITE_DAT_ERR);
    return result;
}

/*********************************************************************
*
*             FS__lb_mul_write
*
  Description:
  FS internal function. Write sector to device.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Sector      - Sector to be written to the device.
  pBuffer     - Pointer to data to be stored.

  Return value:
  ==0         - Sector has been written to the device.
  <0          - An error has occured.
*/

int FS__lb_mul_write(const FS__device_type *pDriver, u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    int ret, Idx;
    int t1, t2;

    if(!pDriver->dev_mul_write)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if((Idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    t1 = OSTimeGet();
    
    // Update cache data
    FSLBSyncMulWriteCache(Idx, Unit, Sector, NumofSector, pBuffer);

    ret = (pDriver->dev_mul_write)(Unit, Sector, NumofSector, pBuffer);
    ret = FS__lb_SD_WRretry(pDriver, Unit, Sector, NumofSector, pBuffer, ret);

    // Update cache Dirty
    FSLBUpdateCacheDirty(Idx, Unit, Sector, NumofSector);

    t2 = OSTimeGet();
    MulWrCnt++;
    if(t2 - t1 > 1)
        MulWrOTCnt++;

    if(MulWrCnt >= 20)
    {
        if(MulWrOTCnt > 15)
        {
#if FS_SD_REMOUNT
            // Check the device whether is in or not
            if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
            {
                DEBUG_FS("[W] SD remount!\n");
                for(t2 = 0; t2 < 3; t2++)
                {
                    sdcTryInvertSDClk ^= 0x01;
                    if((ret = sdcMount()) < 0)
                        DEBUG_FS("[E] SD remount %d fail.\n", t2);
                    else
                    {
                        DEBUG_FS("[I] SD remount OK.\n");
                        break;
                    }
                }
            }
#endif
        }
        MulWrCnt = 0;
        MulWrOTCnt = 0;
    }

    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    if(ret < 0)
        ERRD(FS_LB_MUL_WRITE_DAT_ERR);
    return ret;
}


/*********************************************************************
*
*             FS__lb_ioctl
*
  Description:
  FS internal function. Execute device command.

  Parameters:
  pDriver     - Pointer to a device driver structure.
  Unit        - Unit number.
  Cmd         - Command to be executed.
  Aux         - Parameter depending on command.
  pBuffer     - Pointer to a buffer used for the command.

  Return value:
  Command specific. In general a negative value means an error.
*/
int FS__lb_ioctl(const FS__device_type *pDriver, u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
    int result, idx;

    if(!pDriver->dev_ioctl)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    if((idx = _FS_LB_GetDriverIndex(pDriver)) < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDeviceOp(pDriver, Unit);
    if (Cmd == FS_CMD_CLEAN_CACHE)
    {
#if FS_USE_LB_READCACHE
        result = FSLBSyncAllCache(idx, Unit);
#endif
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return result;
    }
    else if (Cmd == FS_CMD_FLUSH_CACHE)
    {
#if FS_USE_LB_READCACHE
        FSLBCacheInit(idx, Unit);
#endif
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return 1;
    }
    result = (pDriver->dev_ioctl)(Unit, Cmd, Aux, pBuffer);
    FS_X_OS_UnlockDeviceOp(pDriver, Unit);

    return result;
}


/*********************************************************************
*
*					Interface of distribution
*
*/

int FS__lb_SD_RDretry(const FS__device_type *pDriver, u32 Unit, u32 Sector, u32 NumOfSec, void *pBuffer, int result)
{
    int idx, tmpVal, count;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if(0 == strncmp(FS__pDevInfo[idx].devname, "sdmmc", 5))
    {
        count = 0;
        while((result < 0) && (count < R_W_RETRY_COUNT_MAX))
        {
            if(sdcErrorResultFilter(result) < 0)
                count = R_W_RETRY_COUNT_MAX;	// hardwave error happened, no retry action.

            // Check the device whether is in or not
            if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
                tmpVal = SYS_I_STORAGE_MAIN;
            else
                tmpVal = SYS_I_STORAGE_BACKUP;
            if(sysGetStorageInserted(tmpVal) == SYS_V_STORAGE_OFF)
                break;
            count ++;
            sdcTryInvertSDClk ^= 0x01;
            DEBUG_FS("SDC Error, Re-Mount again!\n");
            if(sdcMount() < 0)
                continue;
            DEBUG_FS("Re-Mount OK!\n");
            if(NumOfSec == 1)
            {
                result = (pDriver->dev_read)(Unit, Sector, pBuffer);
            }
            else
            {
                result = (pDriver->dev_mul_read)(Unit, Sector, NumOfSec, pBuffer);

            }
        }
    }
    return result;
}

int FS__lb_SD_WRretry(const FS__device_type *pDriver, u32 Unit, u32 Sector, u32 NumOfSec, void *pBuffer, int result)
{
    int idx, tmpVal, count;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if(0 == strncmp(FS__pDevInfo[idx].devname, "sdmmc", 5))
    {
        count = 0;
        while((result < 0) && (count < R_W_RETRY_COUNT_MAX))
        {
            if(sdcErrorResultFilter(result) < 0)
                count = R_W_RETRY_COUNT_MAX;	// hardwave error happened, no retry action.

            // Check the device whether is in or not
            if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
                tmpVal = SYS_I_STORAGE_MAIN;
            else
                tmpVal = SYS_I_STORAGE_BACKUP;
            if(sysGetStorageInserted(tmpVal) == SYS_V_STORAGE_OFF)
                break;
            count ++;
            sdcTryInvertSDClk ^= 0x01;
            DEBUG_FS("SDC Error, Re-Mount again!\n");
            if(sdcMount() < 0)
                continue;
            DEBUG_FS("Re-Mount OK!\n");

            if(NumOfSec == 1)
            {
#if SDC_ECC_DETECT
                result = (pDriver->dev_read)(Unit, Sector, FatDummyBuf);
#endif
                result = (pDriver->dev_write)(Unit, Sector, pBuffer);
            }
            else
            {
#if SDC_ECC_DETECT
                result = (pDriver->dev_mul_read)(Unit, Sector, NumOfSec, FatDummyBuf);
#endif
                result = (pDriver->dev_mul_write)(Unit, Sector, NumOfSec, pBuffer);
            }
        }
    }
    return result;
}

int FS__lb_sin_read(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int err;

#if FS_RW_DIRECT
    err = FS__lb_read_Direct(pDriver, Unit, Sector, pBuffer); // Read directory sector
#else
    err = FS__lb_read(pDriver, Unit, Sector, pBuffer); // Read directory sector
#endif

    return err;
}

int FS__lb_sin_write(const FS__device_type *pDriver, u32 Unit, u32 Sector, void *pBuffer)
{
    int err;

#if FS_RW_DIRECT
    err = FS__lb_write_Direct(pDriver, Unit, Sector, pBuffer);
#else
    err = FS__lb_write(pDriver, Unit, Sector, pBuffer);
#endif

    return err;
}


int FS__lb_read_FAT_table(const FS__device_type *pDriver, u32 Unit, u32 fat1Sec, u32 fat2Sec, void *pBuffer)
{
    int err;
    err = FS__lb_sin_read(pDriver, Unit, fat1Sec, (void*)pBuffer);	// FAT1 table
    if (err < 0)
    {
        ERRD(FS_REAL_SEC_READ_ERR);
        //
        err = FS__lb_sin_read(pDriver, Unit, fat2Sec, (void*)pBuffer);	// FAT2 table
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            return err;
        }
        // Try to repair original FAT sector with contents of copy
        err = FS__lb_sin_write(pDriver, Unit, fat1Sec, (void*)pBuffer);
        if(err < 0)
        {
            ERRD(FS_REAL_SEC_WRTIE_ERR);
            return err;
        }
    }
    return err;
}


