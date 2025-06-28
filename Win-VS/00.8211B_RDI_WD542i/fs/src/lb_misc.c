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

/*********************************************************************
*
*            Constant
*
**********************************************************************/
#define R_W_RETRY_COUNT_MAX   3

/*********************************************************************
*
*             Extern Global Variable
*
**********************************************************************/
extern unsigned char  format_Start;
extern u8  gInsertCard;
extern u32 sdcTryInvertSDClk;

u8 MulWrCnt, MulWrOTCnt, MulWrOnceCnt;


/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

#if FS_USE_LB_READCACHE

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

static int _FS_LB_GetDriverIndex(const FS__device_type *pDriver)
{
    unsigned int i;

    i = 0;
    while (1)
    {
        if (i >= FS__maxdev)
        {
            break;  /* Driver not found */
        }
        if (FS__pDevInfo[i].devdriver == pDriver)
        {
            break;  /* Driver found */
        }
        i++;
    }
    if (i >= FS__maxdev)
    {
        return -1;
    }
    return i;
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
static char _FS_LBW_Cache_Check(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int idx;
    int Cache_idx;
    int i,empty_flag;
    int x;
    int count;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if (idx < 0)
    {
        return -1;
    }

    if (Sector>= FS__FAT_aBPBUnit[idx][Unit].FSInfo && Sector<= FS__FAT_aBPBUnit[idx][Unit].FatEndSec)
    {
        Cache_idx=FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex;
        if (Cache_idx==0xFFFFFFFF)
            return 0;
        i=0;
        empty_flag=0xFFFFFFFF;
        while (1)
        {
            if (i >= FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum)
            {
                // Sector not in cache
                if (empty_flag!=0xFFFFFFFF)       // Having empty space
                {
                    FS__CLIB_memcpy(FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].aBlockData,pBuffer, FS_LB_BLOCKSIZE);
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=empty_flag;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].BlockId = Sector;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].Dirty=1;  // The data is async~
                }
                else     // LRU replacement
                {
                    Cache_idx++;
                    if (Cache_idx >= FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum)
                        Cache_idx=0;
                    if (FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].Dirty==1) // cache coherency
                    {
                        x=(pDriver->dev_write)(Unit,FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].BlockId, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].aBlockData);
                        if(gInsertCard)
                        {
                            count=0;
                            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                            {
                                count ++;
                                sdcTryInvertSDClk ^=0x01;
                                DEBUG_FS("SDC error, Re-Mount again!\n");
                                if (sdcMount() <= 0)
                                    continue;
                                DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                                x = (pDriver->dev_read)(Unit, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].BlockId, FatDummyBuf);
#endif
                                x = (pDriver->dev_write)(Unit,FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].BlockId, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].aBlockData);
                            }
                        }

                        if (x < 0)
                        {
                            return -1;
                        }
                    }

                    FS__CLIB_memcpy(FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].aBlockData,pBuffer, FS_LB_BLOCKSIZE);
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=Cache_idx;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].BlockId = Sector;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].Dirty=1;  // The data is async~
                }
                return 1;
            }

            if (Sector == FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId)
            {
                FS__CLIB_memcpy(FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].aBlockData,pBuffer ,FS_LB_BLOCKSIZE);
                FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=i;
                FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].Dirty=1;  // The data is async~
                return 1;  /* Sector found */
            }

            if (FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId==0xFFFFFFFF && empty_flag==0xFFFFFFFF) // Check having empty cache space or not
                empty_flag=i;

            i++;
        }

    }
    else
        return 0;  // directly write

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
static char _FS_LBR_Cache_Check(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int idx;
    int Cache_idx;
    int i,empty_flag;
    int x;
    int count;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if (idx < 0)
    {
        return -1;
    }

    if (Sector>= FS__FAT_aBPBUnit[idx][Unit].FSInfo && Sector<= FS__FAT_aBPBUnit[idx][Unit].FatEndSec)
    {
        Cache_idx=FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex;
        if (Cache_idx==0xFFFFFFFF)
            return 0;
        i=0;
        empty_flag=0xFFFFFFFF;
        while (1)
        {
            if (i >= FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum)
            {
                // Sector not in cache
                if (empty_flag!=0xFFFFFFFF)       // Having empty space
                {
                    x=(pDriver->dev_read)(Unit,Sector,pBuffer);
                    if(gInsertCard)
                    {
                        count=0;
                        while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                        {
                            count ++;
                            sdcTryInvertSDClk ^=0x01;
                            DEBUG_FS("SDC error, Re-Mount again!\n");
                            if (sdcMount() <= 0)
                                continue;
                            DEBUG_FS("Re-Mount OK!\n");
                            x = (pDriver->dev_read)(Unit, Sector, pBuffer);
                        }
                        if (x < 0)
                            return -1;
                    }
                    FS__CLIB_memcpy(FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].aBlockData,pBuffer, FS_LB_BLOCKSIZE);
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=empty_flag;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].BlockId = Sector;
                    FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[empty_flag].Dirty=0;
                }
                else     // Read Directly and choose victim
                {
                    Cache_idx++;
                    if (Cache_idx >= FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum)
                        Cache_idx=0;
                    if (FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].Dirty==1)
                    {
                        x=(pDriver->dev_read)(Unit,Sector,pBuffer);
                        if(gInsertCard)
                        {
                            count=0;
                            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                            {
                                count ++;
                                sdcTryInvertSDClk ^=0x01;
                                DEBUG_FS("SDC error, Re-Mount again!\n");
                                if (sdcMount() <= 0)
                                    continue;
                                DEBUG_FS("Re-Mount OK!\n");
                                x = (pDriver->dev_read)(Unit, Sector, pBuffer);
                            }
                            if (x < 0)
                                return -1;
                        }
                    }
                    else  // Cache not dirty so replacement
                    {
                        x=(pDriver->dev_read)(Unit,Sector,pBuffer);
                        if(gInsertCard)
                        {
                            count=0;
                            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                            {
                                count ++;
                                sdcTryInvertSDClk ^=0x01;
                                DEBUG_FS("SDC error, Re-Mount again!\n");
                                if (sdcMount() <= 0)
                                    continue;
                                DEBUG_FS("Re-Mount OK!\n");
                                x = (pDriver->dev_read)(Unit, Sector, pBuffer);
                            }
                            if (x < 0)
                                return -1;
                        }

                        FS__CLIB_memcpy(FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].aBlockData,pBuffer, FS_LB_BLOCKSIZE);
                        FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=Cache_idx;
                        FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].BlockId = Sector;
                        FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[Cache_idx].Dirty=0;
                    }
                }
                return 1;
            }

            if (Sector == FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId)
            {
                FS__CLIB_memcpy(pBuffer,FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].aBlockData ,FS_LB_BLOCKSIZE);
                FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex=i;
                return 1;  /* Sector found */
            }

            if (FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId==0xFFFFFFFF && empty_flag==0xFFFFFFFF) // Check having empty cache space or not
                empty_flag=i;

            i++;
        }

    }
    else
        return 0;  // directly read

}

static char _FS_LB_Cache_Clean(const FS__device_type *pDriver, FS_u32 Unit)
{
    int i,idx;
    int x;
    int count;

    i=0;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if (idx < 0)
    {
        return -1;
    }
    while (1)
    {
        if (i >= FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum)
            break;

        if (FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].Dirty==1) // Cache Coherency
        {
            x=(pDriver->dev_write)(Unit,FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].aBlockData);
            if(gInsertCard)
            {
                count=0;
                while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                {
                    count ++;
                    sdcTryInvertSDClk ^=0x01;
                    DEBUG_FS("SDC error, Re-Mount again!\n");
                    if (sdcMount() <= 0)
                        continue;
                    DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                    x = (pDriver->dev_read)(Unit, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId, FatDummyBuf);
#endif
                    x = (pDriver->dev_write)(Unit,FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId, FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].aBlockData);
                }
                if(x<0)
                    return -1;
            }

            FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].Dirty=0;
        }

        i++;
    }

    return 1;
}

char FS_LB_Cache_Clean(int idx, unsigned int Unit)
{
    char err;
    //
    FS_X_OS_LockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);
    err =_FS_LB_Cache_Clean(FS__pDevInfo[idx].devdriver, Unit);
    FS_X_OS_UnlockDeviceOp(FS__pDevInfo[idx].devdriver, Unit);
    return err;
}

static void _FS_LB_Cache_Init(const FS__device_type *pDriver, FS_u32 Unit)
{
    int i;
    int idx;
    MulWrCnt = 0;
    MulWrOTCnt = 0;
    MulWrOnceCnt = 0;
    idx = _FS_LB_GetDriverIndex(pDriver);
    if (idx<0)
    {
        return;
    }
    if (FS__pDevInfo[idx].pDevCacheInfo)
    {
        FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex = 0xffffffffUL;
        for (i = 0; i < FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum; i++)
        {
            FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId = 0xffffffffUL;
            FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].Dirty=0;
        }
    }
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

static void _FS_LB_ClearCache(const FS__device_type *pDriver, FS_u32 Unit)
{
    int i;
    int idx;

    idx = _FS_LB_GetDriverIndex(pDriver);
    if (idx<0)
    {
        return;
    }

    DEBUG_FS("\n----->_FS_LB_Cache_Clear\n");

    if (FS__pDevInfo[idx].pDevCacheInfo)
    {
        FS__pDevInfo[idx].pDevCacheInfo[Unit].CacheIndex = 0xffffffffUL;
        for (i = 0; i < FS__pDevInfo[idx].pDevCacheInfo[Unit].MaxCacheNum; i++)
        {
            FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].BlockId = 0xffffffffUL;
            FS__pDevInfo[idx].pDevCacheInfo[Unit].pCache[i].Dirty=0;
        }
    }
}

char FS_LB_Cache_Clear(int idx, unsigned int Unit)
{
    MulWrCnt = 0;
    MulWrOTCnt = 0;
    MulWrOnceCnt = 0;
    _FS_LB_ClearCache(FS__pDevInfo[idx].devdriver, Unit);
    return 1;
}
#endif  /* FS_USE_LB_READCACHE */


/*********************************************************************
*
*             Global functions
*
**********************************************************************

  Functions here are global, although their names indicate a local
  scope. They should not be called by user application.
*/

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

int FS__lb_status(const FS__device_type *pDriver, FS_u32 Unit)
{
    int x;

    if (pDriver->dev_status)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        x = (pDriver->dev_status)(Unit);
#if FS_USE_LB_READCACHE
        if (x != 0)
        {
            _FS_LB_ClearCache(pDriver, Unit);
        }
#endif  /* FS_USE_LB_READCACHE */
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return x;
    }
    return -1;
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

int FS__lb_read(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int x;
    int count;

    if (pDriver->dev_read && gInsertCard)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
#if FS_USE_LB_READCACHE
        x = _FS_LBR_Cache_Check(pDriver, Unit, Sector, pBuffer);
        if (x == 0)
        {
            x = (pDriver->dev_read)(Unit, Sector, pBuffer);
            if(gInsertCard)
            {
                count=0;
                while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                {
                    count ++;
                    sdcTryInvertSDClk ^=0x01;
                    DEBUG_FS("SDC error, Re-Mount again!\n");
                    if (sdcMount() <= 0)
                        continue;
                    DEBUG_FS("Re-Mount OK!\n");
                    x = (pDriver->dev_read)(Unit, Sector, pBuffer);
                }
            }
            if (x < 0)
            {
                FS_X_OS_UnlockDeviceOp(pDriver, Unit);
                return -1;
            }

        }
#else
        x = (pDriver->dev_read)(Unit, Sector, pBuffer);
        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
                x = (pDriver->dev_read)(Unit, Sector, pBuffer);
            }
        }

        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }
#endif  /* FS_USE_LB_READCACHE */
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return  x;
    }
    return -1;
}

int FS__lb_read_Direct(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int x;
    int count;

    if (pDriver->dev_read && gInsertCard)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        x = (pDriver->dev_read)(Unit, Sector, pBuffer);
        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
                x = (pDriver->dev_read)(Unit, Sector, pBuffer);
            }
        }

        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }

        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return  x;
    }
    return -1;
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

int FS__lb_mul_read(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, FS_u32 NumofSector, void *pBuffer)
{
    int x;
    int SectorNo;
    int count;

    if (pDriver->dev_mul_read && gInsertCard )
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        x = (pDriver->dev_mul_read)(Unit, Sector, NumofSector, pBuffer);
        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
                x = (pDriver->dev_mul_read)(Unit, Sector, NumofSector, pBuffer);
            }
        }

        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }

        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return  x;
    }
    return -1;
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

int FS__lb_write(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int x;
    int count;

    if (pDriver->dev_write && gInsertCard)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
#if FS_USE_LB_READCACHE
        x = _FS_LBW_Cache_Check(pDriver, Unit, Sector, pBuffer);
        if (x==0)
        {
            x = (pDriver->dev_write)(Unit, Sector, pBuffer);
            if(gInsertCard)
            {
                count=0;
                while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
                {
                    count ++;
                    sdcTryInvertSDClk ^=0x01;
                    DEBUG_FS("SDC error, Re-Mount again!\n");
                    if (sdcMount() <= 0)
                        continue;
                    DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                    x = (pDriver->dev_read)(Unit, Sector, FatDummyBuf);
#endif
                    x = (pDriver->dev_write)(Unit, Sector, pBuffer);
                }
            }

            if (x < 0)
            {
                FS_X_OS_UnlockDeviceOp(pDriver, Unit);
                return -1;
            }
        }
#else
        x = (pDriver->dev_write)(Unit, Sector, pBuffer);
        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                x = (pDriver->dev_read)(Unit, Sector, FatDummyBuf);
#endif
                x = (pDriver->dev_write)(Unit, Sector, pBuffer);
            }
        }

        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }
#endif  /* FS_USE_LB_READCACHE */
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return x;
    }
    return -1;
}

int FS__lb_write_Direct(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, void *pBuffer)
{
    int x;
    int count;

    if (pDriver->dev_write && gInsertCard)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        x = (pDriver->dev_write)(Unit, Sector, pBuffer);
        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                x = (pDriver->dev_read)(Unit, Sector, FatDummyBuf);
#endif
                x = (pDriver->dev_write)(Unit, Sector, pBuffer);
            }
        }

        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }

        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return x;
    }
    return -1;
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

int FS__lb_mul_write(const FS__device_type *pDriver, FS_u32 Unit, FS_u32 Sector, FS_u32 NumofSector, void *pBuffer)
{
    int x;
    int count;
    int t1, t2;

    if (pDriver->dev_mul_write && gInsertCard)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        t1 = OSTimeGet();
        x = (pDriver->dev_mul_write)(Unit, Sector, NumofSector, pBuffer);

        if(gInsertCard)
        {
            count=0;
            while( (x<0) && (count <R_W_RETRY_COUNT_MAX) && (gInsertCard==1) )
            {
                count ++;
                sdcTryInvertSDClk ^=0x01;
                DEBUG_FS("SDC error, Re-Mount again!\n");
                if (sdcMount() <= 0)
                    continue;
                DEBUG_FS("Re-Mount OK!\n");
#if SDC_ECC_DETECT
                x = (pDriver->dev_mul_read)(Unit, Sector, NumofSector,FatDummyBuf);
#endif
                x = (pDriver->dev_mul_write)(Unit, Sector, NumofSector, pBuffer);
            }
        }

        t2 = OSTimeGet();
        MulWrCnt++;
        if(t2 - t1 > 1)
            MulWrOTCnt++;

        // Heavy delay twice in five times. Remount now.
        if(t2 - t1 > 20)
        {
#if SD_LOWSPEED_REMOUNT
            if((MulWrOnceCnt > 0) && (MulWrOTCnt > 5))
            {
                DEBUG_FS("[W] SD remount! Critical Hit: %d\n", t2 - t1);
                for(t2 = 0; t2 < 3; t2++)
                {
                    sdcTryInvertSDClk ^= 0x01;
                    if((x = sdcMount()) < 1)
                        DEBUG_FS("[E] SD remount %d fail.\n", t2);
                    else
                    {
                        DEBUG_FS("[I] SD remount OK.\n");
                        break;
                    }
                }
                MulWrCnt = 0;
                MulWrOTCnt = 0;
                MulWrOnceCnt = 0;
                gpioSetLevel(GPIO_POWER_KEEP_GROUP, GPIO_POWER_KEEP, 0);
            }
            else
            {
                DEBUG_FS("[W] SD Critical Hit: %d\n", t2 - t1);
                MulWrOnceCnt++;
            }
#endif
        }

        if(MulWrCnt >= 20)
        {
            if(MulWrOTCnt > 15)
            {
#if SD_LOWSPEED_REMOUNT
                DEBUG_FS("[W] SD remount!\n");
                for(t2 = 0; t2 < 3; t2++)
                {
                    sdcTryInvertSDClk ^= 0x01;
                    if((x = sdcMount()) < 1)
                        DEBUG_FS("[E] SD remount %d fail.\n", t2);
                    else
                    {
                        DEBUG_FS("[I] SD remount OK.\n");
                        break;
                    }
                }
#endif
            }
            MulWrCnt = 0;
            MulWrOTCnt = 0;
            MulWrOnceCnt = 0;
        }
        if (x < 0)
        {
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return -1;
        }

        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return x;
    }
    return -1;
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

int FS__lb_ioctl(const FS__device_type *pDriver, FS_u32 Unit, FS_i32 Cmd, FS_i32 Aux, void *pBuffer)
{
    int x;

    if (pDriver->dev_ioctl)
    {
        FS_X_OS_LockDeviceOp(pDriver, Unit);
        if (Cmd==FS_CMD_CLEAN_CACHE)
        {
            x=_FS_LB_Cache_Clean(pDriver,Unit);
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return x;
        }
        else if (Cmd==FS_CMD_FLUSH_CACHE)
        {
            _FS_LB_ClearCache(pDriver,Unit);
            FS_X_OS_UnlockDeviceOp(pDriver, Unit);
            return 1;
        }
        x = (pDriver->dev_ioctl)(Unit, Cmd, Aux, pBuffer);
        FS_X_OS_UnlockDeviceOp(pDriver, Unit);
        return x;
    }
    return -1;
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
