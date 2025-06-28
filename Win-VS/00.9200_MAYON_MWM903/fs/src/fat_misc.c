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
File        : fat_misc.c
Purpose     : File system's FAT File System Layer misc routines
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
#include "rtcapi.h"
#include "mcpuapi.h"


/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

#ifndef FS_FAT_NOFAT32
#define FS_FAT_NOFAT32        0
#endif /* FS_FAT_NOFAT32 */

#ifndef FS_DIR_MAXOPEN
#define FS_DIR_MAXOPEN        0
#endif /* FS_DIR_MAXOPEN */



/*********************************************************************
*
*             Local data types
*
**********************************************************************
*/

typedef struct
{
#ifdef MMU_SUPPORT
    char* memory;
    int status;
#else
    char memory[FS_FAT_SEC_SIZE];
    int status;
    int rev1;
    int rev2;
    int rev3;
#endif
}
_FS_FAT_block_type;

/*********************************************************************
*
*             Global Variables
*
**********************************************************************
*/
__align(64) static _FS_FAT_block_type _FS_memblock[FS_MEMBLOCK_NUM];

unsigned long fsStorageSectorCount = 0; /*CY 1023*/

OS_EVENT *FSFATClustSemEvt;

// For Cluster list cache of file in Playback.
FS_ClusterListCache FSReadCLCache;
u8 FSReadCacheOfClusterList[FS_V_FAT_SECTOR_SIZE * FS_V_FAT_CLUSTER_NUMBER];

u8 *FSBuf;
// Data block info
u8 *FSTableBuf;
u8 *FSTableBufEdge;
FS_MemoryAllocStruct *FSTableHead;
FS_MemoryAllocStruct *FSTableTail;
//
u8 *FSDataBuf;

/*********************************************************************
*
*             Extern functions
*
**********************************************************************/
extern u32 GetTotalBlockCount(int Idx, int Unit);


/*********************************************************************
*
*             External Global Variables
*
**********************************************************************/
extern FS_DISKFREE_T global_diskInfo;


/*********************************************************************
*
*             Local Variables
*
**********************************************************************
*/



/*********************************************************************
*
*             Local functions section
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_block_init
*
  Description:
  FS internal function. Init FAT block memory management.

  Parameters:
  None.

  Return value:
  None.
*/

void FS__fat_block_init(void)
{
    int i;

    FS_X_OS_LockMem();
    for (i = 0; i < FS_MEMBLOCK_NUM; i++)
    {
        _FS_memblock[i].status = 0;
    }
    FS_X_OS_UnlockMem();
}


/*********************************************************************
*
*             FS__fat_malloc
*
  Description:
  FS internal function. Allocate a sector buffer.

  Parameters:
  Size        - Size of the sector buffer. Normally this is 512.
                Parameter is for future extension.

  Return value:
  ==0         - Cannot allocate a buffer.
  !=0         - Address of a buffer.
*/

#ifdef MMU_SUPPORT
__inline char *FS__fat_malloc(unsigned int Size)
{
    int i;
    extern u8* FS_internal_mem;
    FS_X_OS_LockMem();
    if (Size <= FS_FAT_SEC_SIZE)
    {
        for (i = 0; i < FS_MEMBLOCK_NUM; i++)
        {
            if (_FS_memblock[i].status == 0)
            {
                _FS_memblock[i].status = 1;

                _FS_memblock[i].memory=(u8*)(FS_internal_mem+ i*FS_FAT_SEC_SIZE);
                FS_X_OS_UnlockMem();
                return ((void*)_FS_memblock[i].memory);
            }
        }
    }
    FS_X_OS_UnlockMem();
    return 0;
}
#else
__inline char *FS__fat_malloc(unsigned int Size)
{
    int i;

    FS_X_OS_LockMem();
    if (Size <= FS_FAT_SEC_SIZE)
    {
        for (i = 0; i < FS_MEMBLOCK_NUM; i++)
        {
            if (_FS_memblock[i].status == 0)
            {
                _FS_memblock[i].status = 1;

                FS_X_OS_UnlockMem();
                return ((void*)_FS_memblock[i].memory);
            }
        }
    }
    FS_X_OS_UnlockMem();
    return 0;
}

#endif


/*********************************************************************
*
*             FS__fat_free
*
  Description:
  FS internal function. Free sector buffer.

  Parameters:
  pBuffer     - Pointer to a buffer, which has to be set free.

  Return value:
  None.
*/

__inline void FS__fat_free(void *pBuffer)
{
    int i;

    FS_X_OS_LockMem();
    for (i = 0; i < FS_MEMBLOCK_NUM; i++)
    {
        if (((void*)_FS_memblock[i].memory) == pBuffer)
        {
            _FS_memblock[i].status = 0;
            FS_X_OS_UnlockMem();
            return;
        }
    }
    FS_X_OS_UnlockMem();
}

int FSPlaybackCacheBufferReset(void)
{
	memset(FSReadCacheOfClusterList, 0x0, FS_V_FAT_SECTOR_SIZE * FS_V_FAT_CLUSTER_NUMBER);
	memset(&FSReadCLCache, 0x0, sizeof(FS_ClusterListCache));
	FSReadCLCache.CacheBuffer = FSReadCacheOfClusterList;
	return 1;
}

int FSMCacheBufInit(void)
{
    FSTableBuf = FSBuf;
    FSTableBufEdge = FSTableBuf;
    FSDataBuf = FSTableBuf + FS_V_MEMBUF_SIZE;
    memset(FSBuf, 0x0, FS_V_MEMBUF_SIZE * FS_V_TOTAL_CACHE_BLOCK);
    //DEBUG_YELLOW("FSDataBuf: %#x\n", FSDataBuf);

    FSPlaybackCacheBufferReset();

    return 1;
}

int FSCacheInfoInsert(FS_MemoryAllocStruct *FirstBlock, FS_MemoryAllocStruct *ins)
{
    FS_MemoryAllocStruct *cur;

    if(!FirstBlock)
    {
        // ins be the first one block.
        FSTableHead = ins;
        FSTableTail = ins;
        ins->Next = ins;
        ins->Prev = ins;
        return 1;
    }

    cur = FSTableHead;
    do
    {
        if(ins->Address < cur->Address)
        {
            if(cur == FSTableHead)
                FSTableHead = ins;
            break;
        }
        cur = cur->Next;
    }
    while(cur != FSTableHead);

    cur->Prev->Next = ins;
    ins->Prev = cur->Prev;
    ins->Next = cur;
    cur->Prev = ins;

    FSTableTail = FSTableHead->Prev;
    return 1;
}

int FSCacheInfoFindNewSpot(FS_MemoryAllocStruct *FirstBlock, FS_MemoryAllocStruct *mid)
{
    FS_MemoryAllocStruct *cur;
    u8 *boundary;
    if(FirstBlock == NULL)
    {
        mid->Address = FSDataBuf;
        return 1;
    }

    cur = FirstBlock;
    do
    {
    	if(cur == FSTableTail)
    		boundary = FSBuf + FS_V_MEMBUF_SIZE * FS_V_TOTAL_CACHE_BLOCK;
    	else
    		boundary = cur->Next->Address;
    		
        if((cur->Address + cur->Length + mid->Length) <= boundary)
        {
            mid->Address = cur->Address + cur->Length;
            return 1;
        }
        cur = cur->Next;
    }while(cur != FirstBlock);

    return -1;
}

int FSCacheInfoUnchain(FS_MemoryAllocStruct *unIns)
{
    unIns->Prev->Next = unIns->Next;
    unIns->Next->Prev = unIns->Prev;

    if((unIns == FSTableTail) && (unIns == FSTableHead))
    	FSTableTail = FSTableHead = NULL;
	else if(unIns == FSTableTail)
		FSTableTail = FSTableHead->Prev;
	else if(unIns == FSTableHead)
		FSTableHead = FSTableTail->Next;

    return 1;
}

u8 *FSMalloc(u32 SizeOfRequest)
{
    FS_MemoryAllocStruct *FirstBlock, *cur, *Loc;
    
    FS_X_OS_LockMem();
    FirstBlock = cur = Loc = NULL;
    
    cur = (FS_MemoryAllocStruct *) FSTableBuf;
    // Get the instance which can let new ob fit in.
    while((u8 *) cur < (FSTableBufEdge))
    {
        if(FirstBlock == NULL && cur->Address != NULL)
        {
            FirstBlock = cur;
            break;
        }
        cur++;
    }

    // Reassign first block to find empty one.
    cur = (FS_MemoryAllocStruct *) FSTableBuf;
    while((u8 *) cur < (FSTableBuf + FS_V_MEMBUF_SIZE))
    {
        if(cur->Address == NULL)
        {
            Loc = cur;
            Loc->Length = SizeOfRequest;
            
            if(FSCacheInfoFindNewSpot(FirstBlock, Loc) < 0)
            {
                DEBUG_FS("[WARM] No enough memory spaces can provide.\n");
                break;
            }
            FSCacheInfoInsert(FirstBlock, Loc);
			//DEBUG_GREEN("FSMalloc: %#x, %#x\n", Loc->Address, Loc->Length);
            if(((u8 *)(Loc+1)) > FSTableBufEdge)
                FSTableBufEdge = (u8 *)(Loc+1);
            FS_X_OS_UnlockMem();
            return Loc->Address;
        }
        cur++;
    }

    DEBUG_FS("[E] FS Malloc failed.\n");
    FS_X_OS_UnlockMem();
    return NULL;
}

int FSFree(u8 *MemoryAddress)
{
    FS_MemoryAllocStruct *cur, *Loc;
    FS_X_OS_LockMem();
    cur = (FS_MemoryAllocStruct *) FSTableBuf;
    // Get the instance which can let new ob fit in.
    while((u8 *) cur < (FSTableBufEdge))
    {
        if(cur->Address != NULL)
            break;
        cur++;
    }

    Loc = cur;
    do
    {
        if(Loc->Address == MemoryAddress)
        {
            FSCacheInfoUnchain(Loc);
            if(((u8 *)(Loc+1)) == FSTableBufEdge)
                FSTableBufEdge = (u8 *)Loc;
            //DEBUG_LIGHTGREEN("FSFree: %#x\n", Loc->Address);
            //memset(Loc->Address, 0x0, Loc->Length);
            memset(Loc, 0x0, sizeof(FS_MemoryAllocStruct));
            FS_X_OS_UnlockMem();
            return 1;
        }
        Loc = Loc->Next;
    }
    while(Loc != cur);
    
    DEBUG_FS("[E] FS Free empty.\n");
    FS_X_OS_UnlockMem();
    return -1;
}

#if FS_BIT_WISE_OPERATION
int FSFATCalculateSectorByCluster(int Idx, u32 Unit, u32 *pSrcCluster, u32 *pDestSector)
{
    //
    if(!pSrcCluster)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    if(!pDestSector)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    *pDestSector = 0;
    //
    switch(*pSrcCluster)
    {
        case 0:
        case 1:
            ERRD(FS_PARAM_VALUE_ERR);
            //return -1;
        case 2:	// Root cluter
            *pDestSector = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
            break;
        default:
            *pDestSector = *pSrcCluster;
            break;
    }

    *pDestSector = ((*pDestSector - 2) << FS__FAT_aBPBUnit[Idx][Unit].BitNumOfSPC);	// Cluster start from 2nd
    *pDestSector += FS__FAT_aBPBUnit[Idx][Unit].RootDirSec;
    *pDestSector += (((FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) << 5) >> FS__FAT_aBPBUnit[Idx][Unit].BitNumOfBPS);//* FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE; //at FAT32,FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt=0

    return 1;
}

int FSFATLWScanClusterLink(int Idx, u32 Unit, u32 StartClust)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, TmpVal;
	char *pMemCache;
	int ret;
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	CurCluster = StartClust;
	LastFATSector = 0xFFFFFFFF;

	do{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster << 2;
				break;
			default:	// FAT16
				FATIndex = CurCluster << 1;
				break;
		}
		// fatSec is the position of Sector number to locate the FAT1 table.
		// fatoffs is the position of FAT1 table within sector size when system read a sector.
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
		FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);

		if(FATSector != LastFATSector)
		{
			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FS__fat_free(pMemCache);
				return ret;
			}
			LastFATSector = FATSector;
		}

		//
		switch(pBPBUnit->FATType)
		{
			case 1:
				// The cluster fetch from FAT1 of FAT12
				TmpVal = *(pMemCache + FATOffset) & 0xFF;
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache);
					if(ret < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						return ret;
					}
					TmpVal |= (*(pMemCache) & 0xFF) << 8;
				}
				else
					TmpVal |= (*(pMemCache + FATOffset + 1) & 0xFF) << 8;

				if (CurCluster & 1)
					TmpVal = TmpVal >> 4;
				else
					TmpVal = TmpVal & 0x0fff;
				if(TmpVal >= 0x0ff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;

			case 2:
				// The cluster fetch from FAT1 of FAT32
				TmpVal = *(u32 *)(pMemCache + FATOffset);
				if(TmpVal >= 0x0ffffff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;

			default:
				// The cluster fetch from FAT1 of FAT16
				TmpVal = *(u16 *)(pMemCache + FATOffset);
				if(TmpVal >= 0xfff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;
		}
		
		if(TmpVal == 0)
			break;
			
		CurCluster = TmpVal;
	}while(1);

	DEBUG_FS("[ERR] FAT1 Link broken. StartCluster: %#x\n", StartClust);
	FS__fat_free(pMemCache);
	return -1;
}

int FSFATCollectClusterList(int Idx, u32 Unit, u32 StartClst, u32 NumOfCluster, u32 *pClustList, u8 BufType)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, Rounds, *pListPos, TmpVal;
	char *pBuffer;
	u8 LimitSwitch;
	int err;
	//
	if(!pClustList)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	CurCluster = StartClst;
	Rounds = NumOfCluster;
	LimitSwitch = (NumOfCluster != 0x0)? 1: 0;
	pListPos = pClustList;
	LastFATSector = 0xFFFFFFFF;

	do
	{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster << 2;
				break;
			default:	// FAT16
				FATIndex = CurCluster << 1;
				break;
		}
		// fatSec is the position of Sector number to locate the FAT1 table.
		// fatoffs is the position of FAT1 table within sector size when system read a sector.
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
		FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);

		if(FATSector != LastFATSector)
		{
			if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FS__fat_free(pBuffer);
				return err;
			}
			LastFATSector = FATSector;
		}
				
		FATOffset += ((FATSector - LastFATSector) << pBPBUnit->BitNumOfBPS);	// Pick the offset from 
		//
		switch(pBPBUnit->FATType)
		{
			case 1:
				// The cluster fetch from FAT1 of FAT12
				TmpVal = *(pBuffer + FATOffset) & 0xFF;
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
					if(err < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pBuffer);
						return err;
					}
					TmpVal |= (*(pBuffer) & 0xFF) << 8;
				}
				else
					TmpVal |= (*(pBuffer + FATOffset + 1) & 0xFF) << 8;

				if (CurCluster & 1)
					TmpVal = TmpVal >> 4;
				else
					TmpVal = TmpVal & 0x0fff;
				*pListPos = CurCluster;
				if(TmpVal >= 0x0ff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;

			case 2:
				// The cluster fetch from FAT1 of FAT32
				TmpVal = *(u32 *)(pBuffer + FATOffset);
				*pListPos = CurCluster;
				if(TmpVal >= 0x0ffffff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;

			default:
				// The cluster fetch from FAT1 of FAT16
				TmpVal = *(u16 *)(pBuffer + FATOffset);
				*pListPos = CurCluster;
				if(TmpVal >= 0xfff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;
		}

		if(BufType == FS_E_BUF_TYPE_LIST)
			pListPos++;
			
		if(TmpVal == 0)
			break;
			
		CurCluster = TmpVal;
		if(LimitSwitch)
		{
			--Rounds;
			// Check the cluster list is whether enough or not.
			if(Rounds == 0x0)
			{
				FS__fat_free(pBuffer);
				return 1;
			}
		}
	}
	while(1);

	DEBUG_FS("[ERR] FAT1 Link broken. StartCluster: %#x\n", StartClst);
	FS__fat_free(pBuffer);
	return -1;
}


int FSFATGetClusterList(int Idx, u32 Unit, u32 StrtClst, u32 *pClustList)
{
    return FSFATCollectClusterList(Idx, Unit, StrtClst, 0, pClustList, FS_E_BUF_TYPE_LIST);
}

int FSFATGoForwardCluster(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClust)
{
	return FSFATCollectClusterList(Idx, Unit, StrtClst, NumOfCluster, pClust, FS_E_BUF_TYPE_VALUE);
}

int FSFATGoForwardClusterList(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClustList)
{
	return FSFATCollectClusterList(Idx, Unit, StrtClst, NumOfCluster, pClustList, FS_E_BUF_TYPE_LIST);
}

int FSFAT32ScanFreeCluster(int Idx, u32 Unit, u32 *LastCluster)
{
	FS__FAT_BPB *pBPBUnit;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, ClusterSize, LimitOfClusters;
    u8 *pClustDataBuf;
    int ret;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    CurCluster = *LastCluster;
    LastFATSector = 0xFFFFFFFF;
    ClusterSize = pBPBUnit->SizeOfCluster;
    LimitOfClusters = pBPBUnit->LimitsOfCluster;

	if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	do
	{    	
		FATIndex = CurCluster << 2;
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
		FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);

		if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
        {
            if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
            {
                ERRD(FS_LB_MUL_READ_DAT_ERR);
                FSFree(pClustDataBuf);
                return ret;
            }
            LastFATSector = FATSector;
        }

        if(LimitOfClusters - CurCluster >= (ClusterSize >> 2))
        {
            if(mcpu_FATZeroScan(pClustDataBuf, ClusterSize) > 0)	// Only 32 bit mode can use.
            {
            	*LastCluster = CurCluster;
            	FSFree(pClustDataBuf);
                return 1;
            }
            CurCluster += (ClusterSize >> 2);
        }
        else
        {
            // The cluster fetch from FAT1 of FAT32
            if(*(u32 *)((u8 *)pClustDataBuf + FATOffset) == 0x0)
            {
            	*LastCluster = CurCluster;
            	FSFree(pClustDataBuf);
                return 1;
            }
            CurCluster++;
        }
	}while(CurCluster < LimitOfClusters);
	
	*LastCluster = LimitOfClusters;
	FSFree(pClustDataBuf);
	return 1;
}

int FSFATFindFreeCluster(int Idx, u32 Unit, u32 *ClusterList, u32 NumOfClust)
{
    FS__FAT_BPB *pBPBUnit;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, Rounds, TmpVal, LimitOfClusters, ActScanBounds;
    char *pBuffer;
    int err;
    //
    if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    CurCluster = *ClusterList;
    Rounds = NumOfClust;
    LimitOfClusters = pBPBUnit->LimitsOfCluster;
    LastFATSector = 0xFFFFFFFF;

    ActScanBounds = (FS__pDevInfo[Idx].FSInfo.NextFreeCluster + (pBPBUnit->SizeOfCluster >> 2));

    do
    {
        if(CurCluster >= LimitOfClusters)
        {
        	ActScanBounds = (pBPBUnit->SizeOfCluster >> 2);
        	CurCluster = 0;
        }
        
        switch(pBPBUnit->FATType)
        {
            case 1:	// FAT12
                FATIndex = CurCluster + (CurCluster >> 1);
                break;
            case 2:	// FAT32
            	// Use mcpu to speed up the scan process.
            	if(CurCluster > ActScanBounds)
            	{
            		DEBUG_FS("[I] FS Start scan FAT1: %#x\n", CurCluster);
            		if((err = FSFAT32ScanFreeCluster(Idx, Unit, &CurCluster)) < 0)
	            	{
	            		ERRD(FS_FAT_CLUT_FIND_ERR);
	                	FS__fat_free(pBuffer);
	                	return err;
	            	}
	            	// Reset new bounds
	            	ActScanBounds = CurCluster + (pBPBUnit->SizeOfCluster >> 2);
	            	DEBUG_FS("[I] FS End scan FAT1: %#x\n", CurCluster);
            	}
                FATIndex = CurCluster << 2;
                break;
            default:	// FAT16
                FATIndex = CurCluster << 1;
                break;
        }
        // fatSec is the position of Sector number to locate the FAT1 table.
        // fatoffs is the position of FAT1 table within sector size when system read a sector.
        FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
        FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);
        if(FATSector != LastFATSector)
        {
            if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
            {
                ERRD(FS_LB_READ_DAT_ERR);
                FS__fat_free(pBuffer);
                return err;
            }
            LastFATSector = FATSector;
        }
        switch(pBPBUnit->FATType)
        {
            case 1:
                // The cluster fetch from FAT1 of FAT12
                TmpVal = *(pBuffer + FATOffset) & 0xFF;
                if(FATOffset == (pBPBUnit->BytesPerSec - 1))
                {
                    err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_DAT_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }
                    LastFATSector = FATSector + 1;
                    TmpVal |= (*(pBuffer) & 0xFF) << 8;
                }
                else
                    TmpVal |= (*(pBuffer + FATOffset + 1) & 0xFF) << 8;

                if (CurCluster & 1)
                    TmpVal = TmpVal >> 4;
                else
                    TmpVal = TmpVal & 0x0fff;
                break;

            case 2:
                // The cluster fetch from FAT1 of FAT32
                TmpVal = *(u32 *)((pBuffer + FATOffset));
                break;

            default:
                // The cluster fetch from FAT1 of FAT16
                TmpVal = *(u16 *)((pBuffer + FATOffset));
                break;
        }
        
        if(TmpVal == 0)
        {
        	*ClusterList++ = CurCluster;
        	Rounds--;
        }
        
        if((FATSector + 1) >= pBPBUnit->FatEndSec)
            CurCluster = 0;
        else
            CurCluster++;
    }
    while(Rounds);

    FS__fat_free(pBuffer);
    return 1;
}

int FSFATBookFreeCluster(int Idx, u32 Unit, u32 *pClusterList, u32 NumberOfCluster)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, i, TmpVal, *pListPos, Cnt;
	char *pBuffer;
	int err;
	//
	pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
	if (!pBuffer)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	pListPos = pClusterList;
	CurCluster = *pClusterList;
	Cnt = NumberOfCluster;
	LastFATSector = 0xFFFFFFFF;
	i = 0;

	do
	{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = (CurCluster << 2);
				break;
			default:	// FAT16
				FATIndex = (CurCluster << 1);
				break;
		}
		// fatSec is the position of Sector number to locate the FAT1 table.
		// fatoffs is the position of FAT1 table within sector size when system read a sector.
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
		FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);
		if (FATSector != LastFATSector)
		{
			if(LastFATSector != 0xFFFFFFFF)
			{
				if (0)
				{
					u32 i;
					for(i = 0; i < 0x200;i++)
					{
						if(i && !(i%16))
							printf("\n");
						printf("%02x ", pBuffer[i]);
					}
					printf("\n");
				}
				if((err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBuffer)) < 0)
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pBuffer);
					return err;
				}
			}
			if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
			{
				ERRD(FS_LB_READ_DAT_ERR);
				FS__fat_free(pBuffer);
				return err;
			}
			LastFATSector = FATSector;
		}
		if (0)
		{
			u32 i;
			for(i = 0; i < 0x200;i++)
			{
				if(i && !(i%16))
					printf("\n");
				printf("%02x ", pBuffer[i]);
			}
			printf("\n");
		}
		// Prepare the data for write action.
		switch(pBPBUnit->FATType)
		{
			case 1:
				if(pListPos[i+1] == 0x0)
					TmpVal = 0xfff;
				else
					TmpVal = pListPos[i+1];

				if(CurCluster & 1)
					TmpVal = TmpVal << 4;

				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
					if(err < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pBuffer);
						return err;
					}

					if(CurCluster & 1)
						pBuffer[0] = (TmpVal >> 8) & 0xff;
					else
					{
						pBuffer[0] &= ~0x0f;
						pBuffer[0] |= (TmpVal >> 8) & 0x0f;
					}

					err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
					if(err < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pBuffer);
						return err;
					}
					err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer);
					if(err < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pBuffer);
						return err;
					}

					if(CurCluster & 1)
					{
						pBuffer[FATOffset] &= ~0xf0;
						pBuffer[FATOffset] |= TmpVal & 0xf0;
					}
					else
						pBuffer[FATOffset] = TmpVal & 0xff;
				}
				else
				{
					if(CurCluster & 1)
					{
						pBuffer[FATOffset] &= ~0xf0;
						pBuffer[FATOffset] |= TmpVal & 0xf0;
						pBuffer[FATOffset + 1] = (TmpVal >> 8) & 0xff;
					}
					else
					{
						pBuffer[FATOffset] = TmpVal & 0xff;
						pBuffer[FATOffset + 1] &= ~0x0f;
						pBuffer[FATOffset + 1] |= (TmpVal >> 8) & 0x0f;
					}
				}
				break;

			case 2:
				if(Cnt == 1)
					*(u32 *)(pBuffer + FATOffset) = 0x0fffffff;
				else
					*(u32 *)(pBuffer + FATOffset) = pListPos[i+1];
				break;

			default:
				if(Cnt == 1)
					*(u16 *)(pBuffer + FATOffset) = 0xffff;
				else
					*(u16 *)(pBuffer + FATOffset) = pListPos[i+1];
				break;
		}
		
		//DEBUG_LIGHTGREEN("BookCluster: %#x, %#x\n", pListPos[i+1], *(u32 *)(pBuffer + FATOffset));
		CurCluster = pListPos[i+1];
		Cnt--;
	}
	while(Cnt && (pListPos[++i] != 0x0));

	if (0)
	{
		u32 i;
		for(i = 0; i < 0x200;i++)
		{
			if(i && !(i%16))
				printf("\n");
			printf("%02x ", pBuffer[i]);
		}
		printf("\n");
	}
	if((err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
	{
		ERRD(FS_LB_WRITE_DAT_ERR);
		FS__fat_free(pBuffer);
		return err;
	}

	FS__fat_free(pBuffer);
	return 1;
}


int FSFATCleanCluster(int Idx, u32 Unit, u32 *ClusterList, u32 NumberOfCluster)
{
	FS__FAT_BPB *pBPBUnit;
	u32 CurSector;
	u8 *pClustDataBuf;
	int ret, i;
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustDataBuf, 0x0, pBPBUnit->SizeOfCluster);
	
	for(i = 0; i < NumberOfCluster; i++)
	{
		if(FSFATCalculateSectorByCluster(Idx, Unit, (ClusterList + i), &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree(pClustDataBuf);
			return -1;
		}
		
		if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_WRITE_DAT_ERR);
			FSFree(pClustDataBuf);
			return ret;
		}
	}
	
	FSFree(pClustDataBuf);
	return 1;
}

int FSFATSetFSInfo(int Idx, u32 Unit)
{
    FS__FAT_BPB *pBPBUnit;
    char *pBuffer;
    int err;
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    	
    if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    if (pBPBUnit->FATType == 2) //only for FAT32
    {
        // Modify FSInfo
        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, pBPBUnit->FSInfo, pBuffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(pBuffer);
            return err;
        }
        // Check for FSInfo structure in buffer
        // 0 = FAT_FSIS_I_SEC_SIGNATURE
        if((pBuffer[FAT_FSIS_I_SEC_SIGNATURE] == 0x52) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 1] == 0x52) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 2] == 0x61) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 3] == 0x41))
        {
            // 484 = 1E4 = FAT_FSIS_I_SEC_SIGNATURE2
            if((pBuffer[FAT_FSIS_I_SEC_SIGNATURE2] == 0x72) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 1] == 0x72) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 2] == 0x41) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 3] == 0x61))
            {
                // 508 = FAT_FSIS_I_BOOT_SIGNATURE
                if((pBuffer[FAT_FSIS_I_BOOT_SIGNATURE] == 0x00) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 1] == 0x00) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 2] == 0x55) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 3] == 0xaa))
                {
                	
                    // Invalidate last known free cluster count
                    memcpy(&pBuffer[FAT_FSIS_I_FREE_CLUS_COUNT], &FS__pDevInfo[Idx].FSInfo.FreeClusterCount, sizeof(u32));
                    // Give hint for free cluster search
                    memcpy(&pBuffer[FAT_FSIS_I_NEXT_FREE_CLUS], &FS__pDevInfo[Idx].FSInfo.NextFreeCluster, sizeof(u32));
                    
                    err  = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, pBPBUnit->FSInfo, pBuffer);
                    if (err < 0)
                    {
                        ERRD(FS_REAL_SEC_WRTIE_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }
                }
            }
        }	// buffer contains FSInfo structure
    }
    FS__fat_free(pBuffer);
    return 1;
}

int FSFATOrderFreeCluster(int Idx, u32 Unit, u32 *LastUsedCluster, u32 NumberOfCluster, u32 *pClusterList, u32 CleanFlag)
{
    u32 CurCluster, *pListPos;
    int ret;
    //
    if(0)//(*LastUsedCluster == 0x0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }
    if(NumberOfCluster == 0x0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }
    if(!pClusterList)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    CurCluster = (*LastUsedCluster != FS__pDevInfo[Idx].FSInfo.NextFreeCluster)? FS__pDevInfo[Idx].FSInfo.NextFreeCluster: *LastUsedCluster;
    pListPos = pClusterList;
    *pListPos = CurCluster;	// Set the first free cluster.
    //
    // Fetch the cluster from FAT1 of FAT32
    if((ret = FSFATFindFreeCluster(Idx, Unit, pListPos, NumberOfCluster)) < 0)
    {
        ERRD(FS_FAT_CLUT_FIND_ERR);
        return ret;
    }

    if((ret = FSFATBookFreeCluster(Idx, Unit, pListPos, NumberOfCluster)) < 0)
    {
        DEBUG_FS("[ERR] Book the FAT1 link fail.\n");
        return ret;
    }
	CurCluster = *pListPos;

    if(CleanFlag)
	{
		if((ret = FSFATCleanCluster(Idx, Unit, pClusterList, NumberOfCluster)) < 0)
		{
			ERRD(FS_DATA_WRITE_ERR);
			return ret;
		}
	}

    if((ret = FSFATFindFreeCluster(Idx, Unit, &CurCluster, 1)) < 0)
    {
        ERRD(FS_FAT_CLUT_FIND_ERR);
        return ret;
    }

	//DEBUG_CYAN("Free Cluster: %#x\n", CurCluster);
    FS__pDevInfo[Idx].FSInfo.NextFreeCluster = CurCluster;
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount -= NumberOfCluster;

    if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
    {
    	DEBUG_FS("[ERR] FSInfo update fail.\n");
		return -1;
    }
    
	// Update the storage space counter.
    global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

    return 1;
}

int FSFATSetTheClusterLinkToDestination(int Idx, u32 Unit, u32 SrcCluster, u32 DestCluster)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset;
	u32 tmpClusterVal;
	int ret;
	char *pBuffer;
	//
	pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!pBuffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    //
    switch(pBPBUnit->FATType)
    {
        case 1:	// FAT12
            FATIndex = SrcCluster + (SrcCluster >> 1);
            break;
        case 2:	// FAT32
            FATIndex = SrcCluster << 2;
            break;
        default:	// FAT16
            FATIndex = SrcCluster << 1;
            break;
    }
	FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
	FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);

	if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
	{
		ERRD(FS_LB_READ_DAT_ERR);
		FS__fat_free(pBuffer);
		return -1;
	}
	
    switch(pBPBUnit->FATType)
    {
    	case 1:
    		if(SrcCluster & 1)
    			tmpClusterVal = DestCluster << 4;
    		if(FATOffset == (pBPBUnit->BytesPerSec - 1))
    		{
    			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pBuffer);
					return -1;
				}

				if(SrcCluster & 1)
                    pBuffer[0] = (tmpClusterVal >> 8) & 0xff;
                else
                {
                    pBuffer[0] &= ~0x0f;
                    pBuffer[0] |= (tmpClusterVal >> 8) & 0x0f;
                }

				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer)) < 0)
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pBuffer);
					return ret;
				}

				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pBuffer);
					return ret;
				}

				if(SrcCluster & 1)
                {
                    pBuffer[FATOffset] &= ~0xf0;
                    pBuffer[FATOffset] |= tmpClusterVal & 0xf0;
                }
                else
                    pBuffer[FATOffset] = tmpClusterVal & 0xff;
    		}
    		else
    		{
    			if(SrcCluster & 1)
                {
                    pBuffer[FATOffset] &= ~0xf0;
                    pBuffer[FATOffset] |= tmpClusterVal & 0xf0;
                    pBuffer[FATOffset + 1] = (tmpClusterVal >> 8) & 0xff;
                }
                else
                {
                    pBuffer[FATOffset] = tmpClusterVal & 0xff;
                    pBuffer[FATOffset + 1] &= ~0x0f;
                    pBuffer[FATOffset + 1] |= (tmpClusterVal >> 8) & 0x0f;
                }
    		}
    		break;
    	case 2:
    		if((tmpClusterVal = *(u32 *)(pBuffer + FATOffset)) != 0x0fffffff)
    			DEBUG_FS("[W] The cluster isn't the EOF cluster will be link to new free cluster.\n");
    		*(u32 *)(pBuffer + FATOffset) = DestCluster;
    		break;
    	default:
    		if((tmpClusterVal = *(u16 *)(pBuffer + FATOffset)) != 0xffff)
    			DEBUG_FS("[W] The cluster isn't the EOF cluster will be link to new free cluster.\n");
    		*(u16 *)(pBuffer + FATOffset) = DestCluster;
    		break;
    }
    
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
    {
    	ERRD(FS_LB_WRITE_DAT_ERR);
    	FS__fat_free(pBuffer);
		return ret;
    }

    FS__fat_free(pBuffer);
    return 1;
}

int FSFATAllocateFreeCluster(int Idx, u32 Unit, u32 *LastUsedCluster, u32 NumberOfCluster, u32 CleanFlag, u32 LinkFlag)
{
    FS__FAT_BPB *pBPBUnit;
    u32 *pClustListBuf;
    int ret;
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    if((pClustListBuf = (u32 *)FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
    {
    	ERRD(FS_MEMORY_ALLOC_ERR);
    	return -1;
    }
    if((ret = FSFATOrderFreeCluster(Idx, Unit, LastUsedCluster, NumberOfCluster, pClustListBuf, CleanFlag)) < 0)
    {
    	ERRD(FS_FAT_CLUT_FIND_ERR);
    	FSFree((u8 *)pClustListBuf);
    	return ret;
    }

    if(*pClustListBuf == 0x0)
    {
    	ERRD(FS_FAT_CLUT_FIND_ERR);
    	FSFree((u8 *)pClustListBuf);
    	return -1;
    }

    if(LinkFlag)
    {
    	//DEBUG_MAGENTA("LastUsedCluster: %#x, pClustListBuf: %#x\n", *LastUsedCluster, *pClustListBuf);
    	if((ret = FSFATSetTheClusterLinkToDestination(Idx, Unit, *LastUsedCluster, *pClustListBuf)) < 0)
    	{
    		ERRD(FS_FAT_CLUT_LINK_ERR);
    		FSFree((u8 *)pClustListBuf);
    		return ret;
    	}
    }

	*LastUsedCluster = *pClustListBuf;
    FSFree((u8 *)pClustListBuf);
    return 1;
}

/*********************************************************************
*
*             FSFATIncEntry
*
  Description:
  FS internal function. Increase directory starting at DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory.
  NumberOfCluster - One cluster can provide 1024 short dir or file entry.
  pDirSize    - Pointer to an u32, which is used to return the new
                sector (not cluster) size of the directory.

  Return value:
  ==1         - Success.
  ==-1        - An error has occured.
*/
int FSFATIncEntry(int Idx, u32 Unit, u32 DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag)
{
	FS__FAT_BPB *pBPBUnit;
	u32 i, NewClusterVal;
	int ret;
	u8 err;
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	if((ret = FSFATGoForwardCluster(Idx, Unit, DirStart, 0, &NewClusterVal)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}
	
	if((ret = FSFATAllocateFreeCluster(Idx, Unit, &NewClusterVal, NumberOfCluster, CleanFlag, 1)) < 0)
	{
		ERRD(FS_FAT_CLUT_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
        return ret;
	}

	if(pSize != NULL)
		*pSize += (NumberOfCluster << pBPBUnit->BitNumOfSPC);
	OSSemPost(FSFATClustSemEvt);
    return 1;
}

int FSFATNewEntry(int Idx, u32 Unit, u32 *DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag)
{
	FS__FAT_BPB *pBPBUnit;
	u32 CurCluster;
	int ret;
	u8 err;
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	*pSize = 0;
	
	CurCluster = FS__pDevInfo[Idx].FSInfo.NextFreeCluster;
	if((ret = FSFATAllocateFreeCluster(Idx, Unit, &CurCluster, NumberOfCluster, CleanFlag, 0)) < 0)
	{
		ERRD(FS_FAT_CLUT_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	*DirStart = CurCluster;
	if(pSize != NULL)
		*pSize += (NumberOfCluster << pBPBUnit->BitNumOfSPC);
	OSSemPost(FSFATClustSemEvt);
	return 1;
}

/**********************************************************************
*
*             FSFATFreeFATLink
*
  Description:
  Delete FAT1 link of a file or directory.

  Parameters:
  
  Return value:
  = 1	- Success.
  < 0	- An error has occured.
************************************************************************/
//Just kill the FAT(FDB) but no kill the Data
int FSFATFreeFATLink(int Idx, u32 Unit, u32 StartCluster)
{
#define FS_FAT_FREE_SIN_MUL_SWITCH 0
#if FS_FAT_FREE_SIN_MUL_SWITCH
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, tmpVal, Count;
	int ret;
	u8 err, Loop;
	char *pMemCache;

	u32 time1, time2;
	//
	if(StartCluster == 0x0)
	{
		ERRD(FS_PARAM_VALUE_ERR);
		return -1;
	}
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pMemCache = FS__fat_malloc(pBPBUnit->BytesPerSec)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
		return -1;
	}
	
	CurCluster = StartCluster;
	Count = 0;
	Loop = 1;
	LastFATSector = 0xFFFFFFFF;

	time1 = OSTimeGet();
	do{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster << 2;
				break;
			default:	// FAT16
				FATIndex = CurCluster << 1;
				break;
		}
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
		FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);
		if(FATSector != LastFATSector)
		{
			if(Count != 0x0)
			{
				// Write back the modified data
				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pMemCache) < 0))
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
			}
		
			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_READ_DAT_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return ret;
			}
			LastFATSector = FATSector;
		}
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					tmpVal = (*pMemCache << 8);
					if(CurCluster & 1)
						*pMemCache = 0x0;
					else
						*pMemCache &= 0xf0;
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					
					if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					tmpVal |= *(pMemCache + FATOffset);
					if(CurCluster & 1)
					{
						*(pMemCache + FATOffset) &= 0x0f;
						tmpVal >>= 4;
					}
					else
						*(pMemCache + FATOffset) = 0x0;
					CurCluster = tmpVal & 0xfff;
				}
				else
				{
					tmpVal = *(u16 *)(pMemCache + FATOffset);
					if(CurCluster & 1)
					{
						*(pMemCache + FATOffset) &= 0x0f;
						*(pMemCache + FATOffset + 1) = 0x00;
						tmpVal >>= 4;
					}
					else
					{
						*(pMemCache + FATOffset) = 0x00;
						*(pMemCache + FATOffset + 1) &= 0xf0;
					}
					CurCluster = tmpVal & 0xfff;
				}
				break;
			case 2: // FAT32
				CurCluster = *(u32 *)(pMemCache + FATOffset);
				memset(pMemCache + FATOffset, 0x0, sizeof(u32));
				break;
			default:	// FAT16
				CurCluster = *(u16 *)(pMemCache + FATOffset);
				memset(pMemCache + FATOffset, 0x0, sizeof(u16));
				break;
		}
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_EOF_FIND_ERR);
			FS__fat_free(pMemCache);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		Count++;
		
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				if(CurCluster >= 0xFF8)
					Loop = 0;
				break;
			case 2: // FAT32
				if(CurCluster >= 0x0FFFFFF8)
					Loop = 0;
				break;
			default:	// FAT16
				if(CurCluster >= 0xFFF8)
					Loop = 0;
				break;
		}
	}while(Loop);

	// Write back the modified data
	if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache) < 0))
	{
		ERRD(FS_LB_WRITE_DAT_ERR);
		FS__fat_free(pMemCache);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	FS__pDevInfo[Idx].FSInfo.FreeClusterCount += Count;
	if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
	{
		ERRD(FS_FAT_FSIS_UPDATE_ERR);
		FS__fat_free(pMemCache);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	// Update the storage space counter.
	global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

	time2 = OSTimeGet();
	DEBUG_GREEN("[I] Free FAT link: %d (x50ms)\n", time2 - time1);

	FS__fat_free(pMemCache);
	OSSemPost(FSFATClustSemEvt);
	return 1;
#else
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, tmpVal, Count;
	int ret;
	u8 *pClustDataBuf;
	u8 err, Loop;
	char *pMemCache;

	u32 time1, time2;
	//
	if(StartCluster == 0x0)
	{
		ERRD(FS_PARAM_VALUE_ERR);
		return -1;
	}
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	CurCluster = StartCluster;
	Count = 0;
	Loop = 1;
	LastFATSector = 0xFFFFFFFF;

	time1 = OSTimeGet();
	if(pBPBUnit->FATType == 0x1)
	{
		if((pMemCache = FS__fat_malloc(pBPBUnit->BytesPerSec)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		
		do
		{
			FATIndex = CurCluster + (CurCluster >> 1);
			FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
			FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);
			if (FATSector != LastFATSector)
			{				
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				LastFATSector = FATSector;
			}

			if(FATOffset == (pBPBUnit->BytesPerSec - 1))
			{
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				tmpVal = (*pMemCache << 8);
				if(CurCluster & 1)
					*pMemCache = 0x0;
				else
					*pMemCache &= 0xf0;
				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				tmpVal |= *(pMemCache + FATOffset);
				if(CurCluster & 1)
				{
					*(pMemCache + FATOffset) &= 0x0f;
					tmpVal >>= 4;
				}
				else
					*(pMemCache + FATOffset) = 0x0;
				CurCluster = tmpVal & 0xfff;
			}
			else
			{
				tmpVal = *(u16 *)(pMemCache + FATOffset);
				if(CurCluster & 1)
				{
					*(pMemCache + FATOffset) &= 0x0f;
					*(pMemCache + FATOffset + 1) = 0x00;
					tmpVal >>= 4;
				}
				else
				{
					*(pMemCache + FATOffset) = 0x00;
					*(pMemCache + FATOffset + 1) &= 0xf0;
				}
				CurCluster = tmpVal & 0xfff;
			}
				
			if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_EOF_FIND_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return -1;
			}

			// Write back the modified data
			if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_WRITE_DAT_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return ret;
			}
			Count++;
			
			if(CurCluster >= 0xFF8)
				Loop = 0;
		}while(Loop);

		FS__fat_free(pMemCache);
	}
	else
	{
		if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		do
		{
			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					FATIndex = CurCluster << 2;
					break;
				default:	// FAT16
					FATIndex = CurCluster << 1;
					break;
			}
			FATSector = pBPBUnit->RsvdSecCnt + (FATIndex >> pBPBUnit->BitNumOfBPS);
			FATOffset = (FATIndex & pBPBUnit->BitRevrOfBPS);

			if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
			{
				if(Count != 0x0)
				{
					// Write back the modified data
					if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
					{
						ERRD(FS_LB_MUL_WRITE_DAT_ERR);
						FSFree(pClustDataBuf);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
				}
				
				if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
				{
					ERRD(FS_LB_MUL_READ_DAT_ERR);
					FSFree(pClustDataBuf);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				LastFATSector = FATSector;
			}

			FATOffset += ((FATSector - LastFATSector) << pBPBUnit->BitNumOfBPS);

			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					CurCluster = *(u32 *)(pClustDataBuf + FATOffset);
					memset(pClustDataBuf + FATOffset, 0x0, sizeof(u32));
					break;
				default:	// FAT16
					CurCluster = *(u16 *)(pClustDataBuf + FATOffset);
					memset(pClustDataBuf + FATOffset, 0x0, sizeof(u16));
					break;
			}
			
			if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_EOF_FIND_ERR);
				FSFree(pClustDataBuf);
				OSSemPost(FSFATClustSemEvt);
				return -1;
			}
			Count++;
			
			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					if(CurCluster >= 0x0FFFFFF8)
						Loop = 0;
					break;
				default:	// FAT16
					if(CurCluster >= 0xFFF8)
						Loop = 0;
					break;
			}
		}while(Loop);

		// Write back the modified data
		if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_WRITE_DAT_ERR);
			FSFree(pClustDataBuf);
			OSSemPost(FSFATClustSemEvt);
			return ret;
		}

		FSFree(pClustDataBuf);
	}

	FS__pDevInfo[Idx].FSInfo.FreeClusterCount += Count;
	if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
	{
		ERRD(FS_FAT_FSIS_UPDATE_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	// Update the storage space counter.
	global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

	time2 = OSTimeGet();
	DEBUG_GREEN("[I] Free FAT link: %d (x50ms)\n", time2 - time1);
	
	OSSemPost(FSFATClustSemEvt);
	return 1;
#endif
}

int FSFATFreeFATLink_bg(s32 Idx, s32 Unit, s32 StartCluster, s32 dummy)
{
	return FSFATFreeFATLink((int) Idx, (u32) Unit, (u32) StartCluster);
}

int FSFATFileEntryUpdate(FS_FILE *pFile)
{
	FS__FAT_BPB *pBPBUnit;
	RTC_DATE_TIME LocalTime;
	FS_FAT_ENTRY *pEntry;
	u32 CurCluster, CurSector, RescanSwitch, i, j;
	u32 *pClustListBuf;
	u16 TmpTimeVal;
    char *pMemCache;
    u8 *pClustDataBuf, *ppClustDataBuf;
    int ret;

    RescanSwitch = 0;

    if(pFile == NULL)
    {
    	ERRD(FS_PARAM_PTR_EXIST_ERR);
    	return -1;
    }

    if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

	pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FS__fat_free(pMemCache);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(pFile->dev_index, pFile->fileid_lo, pFile->fileid_ex, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		FS__fat_free(pMemCache);
		return ret;
	}

	if((CurCluster = pClustListBuf[pFile->FileEntrySect >> pBPBUnit->BitNumOfSPC]) == 0x0)
	{
		DEBUG_FS("[E] Entry offset over cluster list: %d\n", pFile->FileEntrySect);
		RescanSwitch = 1;
	}

	// Direct search by FileEntrySect
	if(!RescanSwitch)
	{
		if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
	    {
	    	ERRD(FS_FAT_SEC_CAL_ERR);
	    	FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
	    	return -1;
	    }

	    CurSector += (pFile->FileEntrySect & (pBPBUnit->SecPerClus - 1));
	    if((ret = FS__lb_sin_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pMemCache)) < 0)
	    {
	    	ERRD(FS_LB_READ_DAT_ERR);
	    	FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
			return ret;
	    }

	    pEntry = (FS_FAT_ENTRY *)pMemCache;
	    do{
	    	if((char *)pEntry >= (pMemCache + FS_FAT_SEC_SIZE))
	    	{
	    		DEBUG_FS("[E] Can not find the entry. Offset: %d\n", pFile->FileEntrySect);
	    		RescanSwitch = 1;
	    		break;
	    	}
	    	
	    	CurCluster = (pEntry->data[21] << 24) | (pEntry->data[20] << 16) | (pEntry->data[27] << 8) | pEntry->data[26];
	    	if(pEntry->data[0] != 0xe5 && (CurCluster == pFile->fileid_hi))
	    		break;
	    	pEntry++;
	    }while(1);
	}

	// Search entry from start cluster to EOF cluster
    if(RescanSwitch)
    {
		i = 0;
		if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
			return -1;
		}
    	do
    	{
    		CurCluster = pClustListBuf[i];
    		if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_CLUT_FIND_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return -1;
			}

			if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
			{
				ERRD(FS_FAT_SEC_CAL_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return -1;
			}

			if((ret = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return ret;
			}

			ppClustDataBuf = pClustDataBuf;
			for(j = 0; j < pBPBUnit->SecPerClus; j++)
			{
				pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
				do
				{
					if(pEntry->data[0] == 0x0)
					{
						DEBUG_FS("[INF] Entry end.\n");
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						FS__fat_free(pMemCache);
						return -1;
					}

					CurCluster = (pEntry->data[21] << 24) | (pEntry->data[20] << 16) | (pEntry->data[27] << 8) | pEntry->data[26];
			    	if(pEntry->data[0] != 0xe5 && (CurCluster == pFile->fileid_hi))
			    	{
			    		DEBUG_YELLOW("[W] Entry found. Name: %s, Offset: %d\n", pEntry->data, (i << pBPBUnit->BitNumOfSPC) + j);
			    		RescanSwitch = 0;
			    		// Copy the memory data and relocate pEntry point.
			    		memcpy(pMemCache, ppClustDataBuf, pBPBUnit->BytesPerSec);			    		
			    		pEntry = (FS_FAT_ENTRY *) (pMemCache + ((u8 *)pEntry - ppClustDataBuf));
			    		CurSector += j;
			    		break;
			    	}
					pEntry++;
				}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
				if(RescanSwitch == 0)
					break;
				ppClustDataBuf += pBPBUnit->BytesPerSec;
			}
    	}while(RescanSwitch);
		FSFree(pClustDataBuf);
    }
    FSFree((u8 *)pClustListBuf);

    memcpy(&pEntry->data[28], &pFile->size, sizeof(u32));
    //DEBUG_GREEN("%s, size: %#x\n", &pEntry->data, pFile->size);
    RTC_Get_Time(&LocalTime);
    TmpTimeVal = ((LocalTime.hour & 0x1F) << 11) | ((LocalTime.min & 0x3F) << 5) | ((LocalTime.sec >> 1) & 0x1F);
    memcpy(&pEntry->data[22], &TmpTimeVal, sizeof(u16));
    TmpTimeVal = (((LocalTime.year + 20) & 0x7F) << 9) | ((LocalTime.month & 0xF) << 5) | (LocalTime.day & 0x1F);
    memcpy(&pEntry->data[24], &TmpTimeVal, sizeof(u16));

    if((ret = FS__lb_sin_write(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pMemCache)) < 0)
    {
    	ERRD(FS_LB_WRITE_DAT_ERR);
    	FS__fat_free(pMemCache);
    	return ret;
    }

    FS__fat_free(pMemCache);
    return 1;
}

int FSFATFileDelete(FS_DIR *pDir, char *pFileName, FS_DeleteCondition *pCondition)
{
	FS__FAT_BPB *pBPBUnit;
	FS_DIR InDir;
	FS_FAT_ENTRY *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	u32 *pClustListBuf;
	int Idx, ret;
	u8 *pClustDataBuf, *ppClustDataBuf;

	if(pDir == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	if(pCondition == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	if(pCondition->DeleteMode == FS_E_DELETE_TYPE_ORDER)
	{
		if(pFileName == NULL)
		{
			ERRD(FS_PARAM_PTR_EXIST_ERR);
			return -1;
		}
	}

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(Idx, Unit, pDir->dirid_hi, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		return ret;
	}

	i = 0;
	if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FSFree((u8 *)pClustListBuf);
		return -1;
	}
	do
	{
		CurCluster = pClustListBuf[i];
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_CLUT_FIND_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if(FSFATCalculateSectorByCluster(Idx, Unit, &CurCluster, &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_READ_DAT_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return ret;
		}

		ppClustDataBuf = pClustDataBuf;
		for(j = 0; j < pBPBUnit->SecPerClus; j++)
		{
			pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
			do
			{
				if(pEntry->data[0] == 0x0)
				{
					DEBUG_FS("[INF] Entry end.\n");
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 0;
				}

				if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && (pEntry->data[0] != '.') &&
					(pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_DIRECTORY))
				{
					memset(&InDir, 0x0, sizeof(FS_DIR));
					InDir.dev_index = pDir->dev_index;
					InDir.dirid_hi = pDir->dirid_hi;
					InDir.dirid_lo = pDir->dirid_lo;
					InDir.dirid_ex = CurCluster;
					InDir.inuse = 1;
					// Release resource temporary
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);

					do
					{
						if((ret = FSFATFileDelete(&InDir, NULL, pCondition)) < 0)
						{
							ERRD(FS_FILE_DELETE_ERR);
							return ret;
						}
					}while(ret);

					if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
					{
						ERRD(FS_MEMORY_ALLOC_ERR);
						return -1;
					}
					if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
					{
						ERRD(FS_LB_MUL_READ_DAT_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}
					
					DEBUG_FS("[INF] InDir name: \\%s\\%s.\n", pDir->dirent.d_name, pEntry->data);
					// Mark the entry
					pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
					// Mark the entry
					pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
					// Write back the Entry info
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}
					// Free the FAT link
					TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
					if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
					{
						ERRD(FS_FAT_LINK_DELETE_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}

					FSFree(pClustDataBuf);
					return 1;
				}
				else if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && (pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_ARCHIVE))
				{
					switch(pCondition->DeleteMode)
					{
						case FS_E_DELETE_TYPE_ORDER:
							if(strncmp((char *)pEntry->data, pFileName, FS_V_FAT_ENTEY_SHORT_NAME) != 0)
								break;
						case FS_E_DELETE_TYPE_AUTO:
							DEBUG_FS("[INF] File name: \\%s\\%s.\n", pDir->dirent.d_name, pEntry->data);
							
							// Mark the entry
							pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
							// Mark the entry
							pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
							// Write back the Entry info
							if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
							{
								ERRD(FS_LB_WRITE_DAT_ERR);
								FSFree((u8 *)pClustListBuf);
								FSFree(pClustDataBuf);
								return ret;
							}
							// Free the FAT link
							TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
							if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
							{
								ERRD(FS_FAT_LINK_DELETE_ERR);
								FSFree((u8 *)pClustListBuf);
								FSFree(pClustDataBuf);
								return ret;
							}

							FSFree((u8 *)pClustListBuf);
							FSFree(pClustDataBuf);
							return 1;
								
						default: 
							DEBUG_FS("[ERR] Mission impossible.\n");
							break;
					}
				}
				pEntry++;
			}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
			ppClustDataBuf += pBPBUnit->BytesPerSec;
		}
	}while(pClustListBuf[++i] != 0x0);

	DEBUG_FS("[INF] Still not found the file entry which can be deleted.\n");
	FSFree((u8 *)pClustListBuf);
	FSFree(pClustDataBuf);
	return 0;
}

int FSFATDirDelete(FS_DIR *pDir)
{
	FS__FAT_BPB *pBPBUnit;
	FS_FAT_ENTRY *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	u32 *pClustListBuf;
	int Idx, ret;
	u8 *pClustDataBuf, *ppClustDataBuf;

	if(pDir == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(Idx, Unit, pDir->dirid_ex, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		return ret;
	}

	i = 0;
	if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FSFree((u8 *)pClustListBuf);
		return -1;
	}
	do
	{
		CurCluster = pClustListBuf[i];
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_CLUT_FIND_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if(FSFATCalculateSectorByCluster(Idx, Unit, &CurCluster, &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_READ_DAT_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return ret;
		}

		ppClustDataBuf = pClustDataBuf;
		for(j = 0; j < pBPBUnit->SecPerClus; j++)
		{
			pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
			do
			{
				if(pEntry->data[0] == 0x0)
				{
					DEBUG_FS("[INF] Entry end.\n");
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 0;
				}

				if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && 
					(pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_DIRECTORY) && 
					(strncmp((char *)pEntry->data, pDir->dirent.d_name, FS_V_FAT_ENTEY_SHORT_NAME) == 0))
				{
					DEBUG_FS("[INF] Dir name: %s.\n", pEntry->data);
					
					// Mark the entry
					pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
					// Mark the entry
					pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
					// Write back the Entry info
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						return ret;
					}
					// Free the FAT link
					TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
					if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
					{
						ERRD(FS_FAT_LINK_DELETE_ERR);
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						return ret;
					}

					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 1;
				}
				pEntry++;
			}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
			ppClustDataBuf += pBPBUnit->BytesPerSec;
		}
	}while(pClustListBuf[++i] != 0x0);

	DEBUG_FS("[INF] Still not found the Dir entry which can deleted.\n");
	FSFree((u8 *)pClustListBuf);
	FSFree(pClustDataBuf);
	return 0;
}
#else
int FSFATCalculateSectorByCluster(int Idx, u32 Unit, u32 *pSrcCluster, u32 *pDestSector)
{
    //
    if(!pSrcCluster)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    if(!pDestSector)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    *pDestSector = 0;
    //
    switch(*pSrcCluster)
    {
        case 0:
        case 1:
            ERRD(FS_PARAM_VALUE_ERR);
            //return -1;
        case 2:	// Root cluter
            *pDestSector = FS__FAT_aBPBUnit[Idx][Unit].RootClus;
            break;
        default:
            *pDestSector = *pSrcCluster;
            break;
    }

    *pDestSector = (*pDestSector - 2) * FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;	// Cluster start from 2nd
    *pDestSector += FS__FAT_aBPBUnit[Idx][Unit].RootDirSec;
    *pDestSector += ((u32)((u32)FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt) * FS_FAT_DENTRY_SIZE) / FS_FAT_SEC_SIZE; //at FAT32,FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt=0

    return 1;
}

int FSFATLWScanClusterLink(int Idx, u32 Unit, u32 StartClust)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, TmpVal;
	char *pMemCache;
	int ret;
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	CurCluster = StartClust;
	LastFATSector = 0xFFFFFFFF;

	do{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster * 4;
				break;
			default:	// FAT16
				FATIndex = CurCluster * 2;
				break;
		}
		// fatSec is the position of Sector number to locate the FAT1 table.
		// fatoffs is the position of FAT1 table within sector size when system read a sector.
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
		FATOffset = FATIndex % pBPBUnit->BytesPerSec;

		if(FATSector != LastFATSector)
		{
			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FS__fat_free(pMemCache);
				return ret;
			}
			LastFATSector = FATSector;
		}

		//
		switch(pBPBUnit->FATType)
		{
			case 1:
				// The cluster fetch from FAT1 of FAT12
				TmpVal = *(pMemCache + FATOffset) & 0xFF;
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache);
					if(ret < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						return ret;
					}
					TmpVal |= (*(pMemCache) & 0xFF) << 8;
				}
				else
					TmpVal |= (*(pMemCache + FATOffset + 1) & 0xFF) << 8;

				if (CurCluster & 1)
					TmpVal = TmpVal >> 4;
				else
					TmpVal = TmpVal & 0x0fff;
				if(TmpVal >= 0x0ff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;

			case 2:
				// The cluster fetch from FAT1 of FAT32
				TmpVal = *(u32 *)(pMemCache + FATOffset);
				if(TmpVal >= 0x0ffffff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;

			default:
				// The cluster fetch from FAT1 of FAT16
				TmpVal = *(u16 *)(pMemCache + FATOffset);
				if(TmpVal >= 0xfff8)
				{
					FS__fat_free(pMemCache);
					return 1;	// EOF found.
				}
				break;
		}
		
		if(TmpVal == 0)
			break;
			
		CurCluster = TmpVal;
	}while(1);

	DEBUG_FS("[ERR] FAT1 Link broken. StartCluster: %#x\n", StartClust);
	FS__fat_free(pMemCache);
	return -1;
}

int FSFATCollectClusterList(int Idx, u32 Unit, u32 StartClst, u32 NumOfCluster, u32 *pClustList, u8 BufType)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, Rounds, *pListPos, TmpVal;
	char *pBuffer;
	u8 LimitSwitch;
	int err;
	//
	if(!pClustList)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	CurCluster = StartClst;
	Rounds = NumOfCluster;
	LimitSwitch = (NumOfCluster != 0x0)? 1: 0;
	pListPos = pClustList;
	LastFATSector = 0xFFFFFFFF;

	do
	{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster * 4;
				break;
			default:	// FAT16
				FATIndex = CurCluster * 2;
				break;
		}
		// fatSec is the position of Sector number to locate the FAT1 table.
		// fatoffs is the position of FAT1 table within sector size when system read a sector.
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
		FATOffset = FATIndex % pBPBUnit->BytesPerSec;

		if(FATSector != LastFATSector)
		{
			if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FS__fat_free(pBuffer);
				return err;
			}
			LastFATSector = FATSector;
		}
				
		FATOffset += (FATSector - LastFATSector) * pBPBUnit->BytesPerSec;	// Pick the offset from 
		//
		switch(pBPBUnit->FATType)
		{
			case 1:
				// The cluster fetch from FAT1 of FAT12
				TmpVal = *(pBuffer + FATOffset) & 0xFF;
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
					if(err < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pBuffer);
						return err;
					}
					TmpVal |= (*(pBuffer) & 0xFF) << 8;
				}
				else
					TmpVal |= (*(pBuffer + FATOffset + 1) & 0xFF) << 8;

				if (CurCluster & 1)
					TmpVal = TmpVal >> 4;
				else
					TmpVal = TmpVal & 0x0fff;
				*pListPos = CurCluster;
				if(TmpVal >= 0x0ff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;

			case 2:
				// The cluster fetch from FAT1 of FAT32
				TmpVal = *(u32 *)(pBuffer + FATOffset);
				*pListPos = CurCluster;
				if(TmpVal >= 0x0ffffff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;

			default:
				// The cluster fetch from FAT1 of FAT16
				TmpVal = *(u16 *)(pBuffer + FATOffset);
				*pListPos = CurCluster;
				if(TmpVal >= 0xfff8)
				{
					FS__fat_free(pBuffer);
					return 1;	// EOF found.
				}
				break;
		}

		if(BufType == FS_E_BUF_TYPE_LIST)
			pListPos++;
			
		if(TmpVal == 0)
			break;
			
		CurCluster = TmpVal;
		if(LimitSwitch)
		{
			--Rounds;
			// Check the cluster list is whether enough or not.
			if(Rounds == 0x0)
			{
				FS__fat_free(pBuffer);
				return 1;
			}
		}
	}
	while(1);

	DEBUG_FS("[ERR] FAT1 Link broken. StartCluster: %#x\n", StartClst);
	FS__fat_free(pBuffer);
	return -1;
}


int FSFATGetClusterList(int Idx, u32 Unit, u32 StrtClst, u32 *pClustList)
{
    return FSFATCollectClusterList(Idx, Unit, StrtClst, 0, pClustList, FS_E_BUF_TYPE_LIST);
}

int FSFATGoForwardCluster(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClust)
{
	return FSFATCollectClusterList(Idx, Unit, StrtClst, NumOfCluster, pClust, FS_E_BUF_TYPE_VALUE);
}

int FSFATGoForwardClusterList(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClustList)
{
	return FSFATCollectClusterList(Idx, Unit, StrtClst, NumOfCluster, pClustList, FS_E_BUF_TYPE_LIST);
}

int FSFAT32ScanFreeCluster(int Idx, u32 Unit, u32 *LastCluster)
{
	FS__FAT_BPB *pBPBUnit;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, ClusterSize, LimitOfClusters;
    u8 *pClustDataBuf;
    int ret;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    CurCluster = *LastCluster;
    LastFATSector = 0xFFFFFFFF;
    ClusterSize = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
    LimitOfClusters = pBPBUnit->TotSec16;
    if(LimitOfClusters == 0)
        LimitOfClusters = pBPBUnit->TotSec32;
    LimitOfClusters = (LimitOfClusters - pBPBUnit->RootDirSec - pBPBUnit->Dsize) / pBPBUnit->SecPerClus;

	if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	do
	{    	
		FATIndex = CurCluster * 4;
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
		FATOffset = FATIndex % pBPBUnit->BytesPerSec;

		if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
        {
            if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
            {
                ERRD(FS_LB_MUL_READ_DAT_ERR);
                FSFree(pClustDataBuf);
                return ret;
            }
            LastFATSector = FATSector;
        }

        if(LimitOfClusters - CurCluster >= (ClusterSize >> 2))
        {
            if(mcpu_FATZeroScan(pClustDataBuf, ClusterSize) > 0)	// Only 32 bit mode can use.
            {
            	*LastCluster = CurCluster;
            	FSFree(pClustDataBuf);
                return 1;
            }
            CurCluster += (ClusterSize >> 2);
        }
        else
        {
            // The cluster fetch from FAT1 of FAT32
            if(*(u32 *)((u8 *)pClustDataBuf + FATOffset) == 0x0)
            {
            	*LastCluster = CurCluster;
            	FSFree(pClustDataBuf);
                return 1;
            }
            CurCluster++;
        }
	}while(CurCluster < LimitOfClusters);
	
	*LastCluster = LimitOfClusters;
	FSFree(pClustDataBuf);
	return 1;
}

int FSFATFindFreeCluster(int Idx, u32 Unit, u32 *ClusterList, u32 NumOfClust)
{
    FS__FAT_BPB *pBPBUnit;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, Rounds, TmpVal, LimitOfClusters, ActScanBounds;
    char *pBuffer;
    int err;
    //
    if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    CurCluster = *ClusterList;
    Rounds = NumOfClust;
    LimitOfClusters = pBPBUnit->TotSec16;
    if(LimitOfClusters == 0)
        LimitOfClusters = pBPBUnit->TotSec32;
    LimitOfClusters = (LimitOfClusters - pBPBUnit->RootDirSec - pBPBUnit->Dsize) / pBPBUnit->SecPerClus;
    LastFATSector = 0xFFFFFFFF;

    ActScanBounds = (FS__pDevInfo[Idx].FSInfo.NextFreeCluster + pBPBUnit->SecPerClus * (pBPBUnit->BytesPerSec >> 2));

    do
    {
        if(CurCluster >= LimitOfClusters)
        {
        	ActScanBounds = pBPBUnit->SecPerClus * (pBPBUnit->BytesPerSec >> 2);
        	CurCluster = 0;
        }
        
        switch(pBPBUnit->FATType)
        {
            case 1:	// FAT12
                FATIndex = CurCluster + (CurCluster >> 1);
                break;
            case 2:	// FAT32
            	// Use mcpu to speed up the scan process.
            	if(CurCluster > ActScanBounds)
            	{
            		DEBUG_FS("[INF] FS Start scan FAT1: %#x\n", CurCluster);
            		if((err = FSFAT32ScanFreeCluster(Idx, Unit, &CurCluster)) < 0)
	            	{
	            		ERRD(FS_FAT_CLUT_FIND_ERR);
	                	FS__fat_free(pBuffer);
	                	return err;
	            	}
	            	// Reset new bounds
	            	ActScanBounds = CurCluster + pBPBUnit->SecPerClus * (pBPBUnit->BytesPerSec >> 2);
	            	DEBUG_FS("[INF] FS End scan FAT1: %#x\n", CurCluster);
            	}
                FATIndex = CurCluster * 4;
                break;
            default:	// FAT16
                FATIndex = CurCluster * 2;
                break;
        }
        // fatSec is the position of Sector number to locate the FAT1 table.
        // fatoffs is the position of FAT1 table within sector size when system read a sector.
        FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
        FATOffset = FATIndex % pBPBUnit->BytesPerSec;
        if(FATSector != LastFATSector)
        {
            if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
            {
                ERRD(FS_LB_READ_DAT_ERR);
                FS__fat_free(pBuffer);
                return err;
            }
            LastFATSector = FATSector;
        }
        switch(pBPBUnit->FATType)
        {
            case 1:
                // The cluster fetch from FAT1 of FAT12
                TmpVal = *(pBuffer + FATOffset) & 0xFF;
                if(FATOffset == (pBPBUnit->BytesPerSec - 1))
                {
                    err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_DAT_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }
                    LastFATSector = FATSector + 1;
                    TmpVal |= (*(pBuffer) & 0xFF) << 8;
                }
                else
                    TmpVal |= (*(pBuffer + FATOffset + 1) & 0xFF) << 8;

                if (CurCluster & 1)
                    TmpVal = TmpVal >> 4;
                else
                    TmpVal = TmpVal & 0x0fff;
                break;

            case 2:
                // The cluster fetch from FAT1 of FAT32
                TmpVal = *(u32 *)((pBuffer + FATOffset));
                break;

            default:
                // The cluster fetch from FAT1 of FAT16
                TmpVal = *(u16 *)((pBuffer + FATOffset));
                break;
        }
        
        if(TmpVal == 0)
        {
        	*ClusterList++ = CurCluster;
        	Rounds--;
        }
        
        if((FATSector + 1) >= pBPBUnit->FatEndSec)
            CurCluster = 0;
        else
            CurCluster++;
    }
    while(Rounds);

    FS__fat_free(pBuffer);
    return 1;
}

int FSFATBookFreeCluster(int Idx, u32 Unit, u32 *pClusterList)
{
    FS__FAT_BPB *pBPBUnit;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, i, TmpVal, *pListPos;
    char *pBuffer;
    int err;
    //
    pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!pBuffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    pListPos = pClusterList;
    CurCluster = *pClusterList;
    LastFATSector = 0xFFFFFFFF;
    i = 0;

    do
    {
        switch(pBPBUnit->FATType)
        {
            case 1:	// FAT12
                FATIndex = CurCluster + (CurCluster >> 1);
                break;
            case 2:	// FAT32
                FATIndex = CurCluster * 4;
                break;
            default:	// FAT16
                FATIndex = CurCluster * 2;
                break;
        }
        // fatSec is the position of Sector number to locate the FAT1 table.
        // fatoffs is the position of FAT1 table within sector size when system read a sector.
        FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
        FATOffset = FATIndex % pBPBUnit->BytesPerSec;
        if (FATSector != LastFATSector)
        {
        	if(LastFATSector != 0xFFFFFFFF)
        	{
        		if (0)
		        {
		        	u32 i;
		        	for(i = 0; i < 0x200;i++)
		        	{
		        		if(i && !(i%16))
		        			printf("\n");
		        		printf("%02x ", pBuffer[i]);
		        	}
		        	printf("\n");
		        }
		        if((err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBuffer)) < 0)
		        {
		            ERRD(FS_LB_WRITE_DAT_ERR);
		            FS__fat_free(pBuffer);
		            return err;
		        }
        	}
            if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
            {
                ERRD(FS_LB_READ_DAT_ERR);
                FS__fat_free(pBuffer);
                return err;
            }
            LastFATSector = FATSector;
        }
        if (0)
        {
        	u32 i;
        	for(i = 0; i < 0x200;i++)
        	{
        		if(i && !(i%16))
        			printf("\n");
        		printf("%02x ", pBuffer[i]);
        	}
        	printf("\n");
        }
        // Prepare the data for write action.
        switch(pBPBUnit->FATType)
        {
            case 1:
                if(pListPos[i+1] == 0x0)
                    TmpVal = 0xfff;
                else
                    TmpVal = pListPos[i+1];

                if(CurCluster & 1)
                    TmpVal = TmpVal << 4;

                if(FATOffset == (pBPBUnit->BytesPerSec - 1))
                {
                    err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_DAT_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }

                    if(CurCluster & 1)
                        pBuffer[0] = (TmpVal >> 8) & 0xff;
                    else
                    {
                        pBuffer[0] &= ~0x0f;
                        pBuffer[0] |= (TmpVal >> 8) & 0x0f;
                    }

                    err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_DAT_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }
                    err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_DAT_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }

                    if(CurCluster & 1)
                    {
                        pBuffer[FATOffset] &= ~0xf0;
                        pBuffer[FATOffset] |= TmpVal & 0xf0;
                    }
                    else
                        pBuffer[FATOffset] = TmpVal & 0xff;
                }
                else
                {
                    if(CurCluster & 1)
                    {
                        pBuffer[FATOffset] &= ~0xf0;
                        pBuffer[FATOffset] |= TmpVal & 0xf0;
                        pBuffer[FATOffset + 1] = (TmpVal >> 8) & 0xff;
                    }
                    else
                    {
                        pBuffer[FATOffset] = TmpVal & 0xff;
                        pBuffer[FATOffset + 1] &= ~0x0f;
                        pBuffer[FATOffset + 1] |= (TmpVal >> 8) & 0x0f;
                    }
                }
                break;

            case 2:
                if(pListPos[i+1] == 0x0)
                    *(u32 *)(pBuffer + FATOffset) = 0x0fffffff;
                else
                    *(u32 *)(pBuffer + FATOffset) = pListPos[i+1];
                break;

            default:
                if(pListPos[i+1] == 0x0)
                	*(u16 *)(pBuffer + FATOffset) = 0xffff;
                else
                    *(u16 *)(pBuffer + FATOffset) = pListPos[i+1];
                break;
        }

        
        //DEBUG_LIGHTGREEN("BookCluster: %#x, %#x\n", pListPos[i+1], *(u32 *)(pBuffer + FATOffset));
        CurCluster = pListPos[i+1];
    }
    while(pListPos[++i] != 0x0);

    if (0)
    {
    	u32 i;
    	for(i = 0; i < 0x200;i++)
    	{
    		if(i && !(i%16))
    			printf("\n");
    		printf("%02x ", pBuffer[i]);
    	}
    	printf("\n");
    }
    if((err = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FS__fat_free(pBuffer);
        return err;
    }

    FS__fat_free(pBuffer);
    return 1;
}

int FSFATCleanCluster(int Idx, u32 Unit, u32 *ClusterList, u32 NumberOfCluster)
{
	FS__FAT_BPB *pBPBUnit;
	u32 CurSector;
	u8 *pClustDataBuf;
	int ret, i;
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustDataBuf, 0x0, pBPBUnit->SizeOfCluster);
	
	for(i = 0; i < NumberOfCluster; i++)
	{
		if(FSFATCalculateSectorByCluster(Idx, Unit, (ClusterList + i), &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree(pClustDataBuf);
			return -1;
		}
		
		if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_WRITE_DAT_ERR);
			FSFree(pClustDataBuf);
			return ret;
		}
	}
	
	FSFree(pClustDataBuf);
	return 1;
}

int FSFATSetFSInfo(int Idx, u32 Unit)
{
    FS__FAT_BPB *pBPBUnit;
    char *pBuffer;
    int err;
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    	
    if((pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    if (pBPBUnit->FATType == 2) //only for FAT32
    {
        // Modify FSInfo
        err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, pBPBUnit->FSInfo, pBuffer);
        if (err < 0)
        {
            ERRD(FS_REAL_SEC_READ_ERR);
            FS__fat_free(pBuffer);
            return err;
        }
        // Check for FSInfo structure in buffer
        // 0 = FAT_FSIS_I_SEC_SIGNATURE
        if((pBuffer[FAT_FSIS_I_SEC_SIGNATURE] == 0x52) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 1] == 0x52) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 2] == 0x61) &&
                (pBuffer[FAT_FSIS_I_SEC_SIGNATURE + 3] == 0x41))
        {
            // 484 = 1E4 = FAT_FSIS_I_SEC_SIGNATURE2
            if((pBuffer[FAT_FSIS_I_SEC_SIGNATURE2] == 0x72) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 1] == 0x72) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 2] == 0x41) &&
                    (pBuffer[FAT_FSIS_I_SEC_SIGNATURE2 + 3] == 0x61))
            {
                // 508 = FAT_FSIS_I_BOOT_SIGNATURE
                if((pBuffer[FAT_FSIS_I_BOOT_SIGNATURE] == 0x00) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 1] == 0x00) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 2] == 0x55) &&
                        (pBuffer[FAT_FSIS_I_BOOT_SIGNATURE + 3] == 0xaa))
                {
                	
                    // Invalidate last known free cluster count
                    memcpy(&pBuffer[FAT_FSIS_I_FREE_CLUS_COUNT], &FS__pDevInfo[Idx].FSInfo.FreeClusterCount, sizeof(u32));
                    // Give hint for free cluster search
                    memcpy(&pBuffer[FAT_FSIS_I_NEXT_FREE_CLUS], &FS__pDevInfo[Idx].FSInfo.NextFreeCluster, sizeof(u32));
                    
                    err  = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, pBPBUnit->FSInfo, pBuffer);
                    if (err < 0)
                    {
                        ERRD(FS_REAL_SEC_WRTIE_ERR);
                        FS__fat_free(pBuffer);
                        return err;
                    }
                }
            }
        }	// buffer contains FSInfo structure
    }
    FS__fat_free(pBuffer);
    return 1;
}

int FSFATOrderFreeCluster(int Idx, u32 Unit, u32 *LastUsedCluster, u32 NumberOfCluster, u32 *pClusterList, u32 CleanFlag)
{
    u32 CurCluster, *pListPos;
    int ret;
    //
    if(0)//(*LastUsedCluster == 0x0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }
    if(NumberOfCluster == 0x0)
    {
        ERRD(FS_PARAM_VALUE_ERR);
        return -1;
    }
    if(!pClusterList)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    //
    CurCluster = (*LastUsedCluster != FS__pDevInfo[Idx].FSInfo.NextFreeCluster)? FS__pDevInfo[Idx].FSInfo.NextFreeCluster: *LastUsedCluster;
    pListPos = pClusterList;
    *pListPos = CurCluster;	// Set the first free cluster.
    //
    // Fetch the cluster from FAT1 of FAT32
    if((ret = FSFATFindFreeCluster(Idx, Unit, pListPos, NumberOfCluster)) < 0)
    {
        ERRD(FS_FAT_CLUT_FIND_ERR);
        return ret;
    }

    if((ret = FSFATBookFreeCluster(Idx, Unit, pListPos)) < 0)
    {
        DEBUG_FS("[ERR] Book the FAT1 link fail.\n");
        return ret;
    }
	CurCluster = *pListPos;

    if(CleanFlag)
    {
    	if((ret = FSFATCleanCluster(Idx, Unit, pClusterList, NumberOfCluster)) < 0)
    	{
    		ERRD(FS_DATA_WRITE_ERR);
    		return ret;
    	}
    }

    if((ret = FSFATFindFreeCluster(Idx, Unit, &CurCluster, 1)) < 0)
    {
        ERRD(FS_FAT_CLUT_FIND_ERR);
        return ret;
    }

	//DEBUG_CYAN("Free Cluster: %#x\n", CurCluster);
    FS__pDevInfo[Idx].FSInfo.NextFreeCluster = CurCluster;
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount -= NumberOfCluster;

    if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
    {
    	DEBUG_FS("[ERR] FSInfo update fail.\n");
		return -1;
    }

    // Update the storage space counter.
    global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

    return 1;
}

int FSFATSetTheClusterLinkToDestination(int Idx, u32 Unit, u32 SrcCluster, u32 DestCluster)
{
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset;
	u32 tmpClusterVal;
	int ret;
	char *pBuffer;
	//
	pBuffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!pBuffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    //
    switch(pBPBUnit->FATType)
    {
        case 1:	// FAT12
            FATIndex = SrcCluster + (SrcCluster >> 1);
            break;
        case 2:	// FAT32
            FATIndex = SrcCluster * 4;
            break;
        default:	// FAT16
            FATIndex = SrcCluster * 2;
            break;
    }
	FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
	FATOffset = FATIndex % pBPBUnit->BytesPerSec;

	if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
	{
		ERRD(FS_LB_READ_DAT_ERR);
		FS__fat_free(pBuffer);
		return -1;
	}
	
    switch(pBPBUnit->FATType)
    {
    	case 1:
    		if(SrcCluster & 1)
    			tmpClusterVal = DestCluster << 4;
    		if(FATOffset == (pBPBUnit->BytesPerSec - 1))
    		{
    			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pBuffer);
					return -1;
				}

				if(SrcCluster & 1)
                    pBuffer[0] = (tmpClusterVal >> 8) & 0xff;
                else
                {
                    pBuffer[0] &= ~0x0f;
                    pBuffer[0] |= (tmpClusterVal >> 8) & 0x0f;
                }

				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pBuffer)) < 0)
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pBuffer);
					return ret;
				}

				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pBuffer);
					return ret;
				}

				if(SrcCluster & 1)
                {
                    pBuffer[FATOffset] &= ~0xf0;
                    pBuffer[FATOffset] |= tmpClusterVal & 0xf0;
                }
                else
                    pBuffer[FATOffset] = tmpClusterVal & 0xff;
    		}
    		else
    		{
    			if(SrcCluster & 1)
                {
                    pBuffer[FATOffset] &= ~0xf0;
                    pBuffer[FATOffset] |= tmpClusterVal & 0xf0;
                    pBuffer[FATOffset + 1] = (tmpClusterVal >> 8) & 0xff;
                }
                else
                {
                    pBuffer[FATOffset] = tmpClusterVal & 0xff;
                    pBuffer[FATOffset + 1] &= ~0x0f;
                    pBuffer[FATOffset + 1] |= (tmpClusterVal >> 8) & 0x0f;
                }
    		}
    		break;
    	case 2:
    		if((tmpClusterVal = *(u32 *)(pBuffer + FATOffset)) != 0x0fffffff)
    			DEBUG_FS("[WARNING] The cluster isn't the EOF cluster will be link to new free cluster.\n");
    		*(u32 *)(pBuffer + FATOffset) = DestCluster;
    		break;
    	default:
    		if((tmpClusterVal = *(u16 *)(pBuffer + FATOffset)) != 0xffff)
    			DEBUG_FS("[WARNING] The cluster isn't the EOF cluster will be link to new free cluster.\n");
    		*(u16 *)(pBuffer + FATOffset) = DestCluster;
    		break;
    }
    
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBuffer)) < 0)
    {
    	ERRD(FS_LB_WRITE_DAT_ERR);
    	FS__fat_free(pBuffer);
		return ret;
    }

    FS__fat_free(pBuffer);
    return 1;
}

int FSFATAllocateFreeCluster(int Idx, u32 Unit, u32 *LastUsedCluster, u32 NumberOfCluster, u32 CleanFlag, u32 LinkFlag)
{
    FS__FAT_BPB *pBPBUnit;
    u32 *pClustListBuf;
    int ret;
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    if((pClustListBuf = (u32 *)FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
    {
    	ERRD(FS_MEMORY_ALLOC_ERR);
    	return -1;
    }
    if((ret = FSFATOrderFreeCluster(Idx, Unit, LastUsedCluster, NumberOfCluster, pClustListBuf, CleanFlag)) < 0)
    {
    	ERRD(FS_FAT_CLUT_FIND_ERR);
    	FSFree((u8 *)pClustListBuf);
    	return ret;
    }

    if(*pClustListBuf == 0x0)
    {
    	ERRD(FS_FAT_CLUT_FIND_ERR);
    	FSFree((u8 *)pClustListBuf);
    	return -1;
    }

    if(LinkFlag)
    {
    	//DEBUG_MAGENTA("LastUsedCluster: %#x, pClustListBuf: %#x\n", *LastUsedCluster, *pClustListBuf);
    	if((ret = FSFATSetTheClusterLinkToDestination(Idx, Unit, *LastUsedCluster, *pClustListBuf)) < 0)
    	{
    		ERRD(FS_FAT_CLUT_LINK_ERR);
    		FSFree((u8 *)pClustListBuf);
    		return ret;
    	}
    }

	*LastUsedCluster = *pClustListBuf;
    FSFree((u8 *)pClustListBuf);
    return 1;
}

/*********************************************************************
*
*             FSFATIncEntry
*
  Description:
  FS internal function. Increase directory starting at DirStart.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  DirStart    - 1st cluster of the directory.
  NumberOfCluster - One cluster can provide 1024 short dir or file entry.
  pDirSize    - Pointer to an u32, which is used to return the new
                sector (not cluster) size of the directory.

  Return value:
  ==1         - Success.
  ==-1        - An error has occured.
*/
int FSFATIncEntry(int Idx, u32 Unit, u32 DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag)
{
	FS__FAT_BPB *pBPBUnit;
	u32 i, NewClusterVal;
	int ret;
	u8 err;
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	if((ret = FSFATGoForwardCluster(Idx, Unit, DirStart, 0, &NewClusterVal)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}
	
	if((ret = FSFATAllocateFreeCluster(Idx, Unit, &NewClusterVal, NumberOfCluster, CleanFlag, 1)) < 0)
	{
		ERRD(FS_FAT_CLUT_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
        return ret;
	}

	if(pSize != NULL)
		*pSize += pBPBUnit->SecPerClus * NumberOfCluster;
	OSSemPost(FSFATClustSemEvt);
    return 1;
}

int FSFATNewEntry(int Idx, u32 Unit, u32 *DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag)
{
	FS__FAT_BPB *pBPBUnit;
	u32 CurCluster;
	int ret;
	u8 err;
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	*pSize = 0;
	
	CurCluster = FS__pDevInfo[Idx].FSInfo.NextFreeCluster;
	if((ret = FSFATAllocateFreeCluster(Idx, Unit, &CurCluster, NumberOfCluster, CleanFlag, 0)) < 0)
	{
		ERRD(FS_FAT_CLUT_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	*DirStart = CurCluster;
	if(pSize != NULL)
		*pSize += pBPBUnit->SecPerClus * NumberOfCluster;
	OSSemPost(FSFATClustSemEvt);
	return 1;
}

/**********************************************************************
*
*             FSFATFreeFATLink
*
  Description:
  Delete FAT1 link of a file or directory.

  Parameters:
  
  Return value:
  = 1	- Success.
  < 0	- An error has occured.
************************************************************************/
//Just kill the FAT(FDB) but no kill the Data
int FSFATFreeFATLink(int Idx, u32 Unit, u32 StartCluster)
{
#define FS_FAT_FREE_SIN_MUL_SWITCH 0
#if FS_FAT_FREE_SIN_MUL_SWITCH
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, tmpVal, Count;
	int ret;
	u8 err, Loop;
	char *pMemCache;

	u32 time1, time2;
	//
	if(StartCluster == 0x0)
	{
		ERRD(FS_PARAM_VALUE_ERR);
		return -1;
	}
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pMemCache = FS__fat_malloc(pBPBUnit->BytesPerSec)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		OSSemPost(FSFATClustSemEvt);
		return -1;
	}
	
	CurCluster = StartCluster;
	Count = 0;
	Loop = 1;
	LastFATSector = 0xFFFFFFFF;

	time1 = OSTimeGet();
	do{
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				FATIndex = CurCluster + (CurCluster >> 1);
				break;
			case 2: // FAT32
				FATIndex = CurCluster * 4;
				break;
			default:	// FAT16
				FATIndex = CurCluster * 2;
				break;
		}
		FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
		FATOffset = FATIndex % pBPBUnit->BytesPerSec;
		if (FATSector != LastFATSector)
		{
			if(Count != 0x0)
			{
				// Write back the modified data
				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pMemCache) < 0))
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
			}
		
			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_READ_DAT_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return ret;
			}
			LastFATSector = FATSector;
		}
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				if(FATOffset == (pBPBUnit->BytesPerSec - 1))
				{
					if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					tmpVal = (*pMemCache << 8);
					if(CurCluster & 1)
						*pMemCache = 0x0;
					else
						*pMemCache &= 0xf0;
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					
					if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
					{
						ERRD(FS_LB_READ_DAT_ERR);
						FS__fat_free(pMemCache);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
					tmpVal |= *(pMemCache + FATOffset);
					if(CurCluster & 1)
					{
						*(pMemCache + FATOffset) &= 0x0f;
						tmpVal >>= 4;
					}
					else
						*(pMemCache + FATOffset) = 0x0;
					CurCluster = tmpVal & 0xfff;
				}
				else
				{
					tmpVal = *(u16 *)(pMemCache + FATOffset);
					if(CurCluster & 1)
					{
						*(pMemCache + FATOffset) &= 0x0f;
						*(pMemCache + FATOffset + 1) = 0x00;
						tmpVal >>= 4;
					}
					else
					{
						*(pMemCache + FATOffset) = 0x00;
						*(pMemCache + FATOffset + 1) &= 0xf0;
					}
					CurCluster = tmpVal & 0xfff;
				}
				break;
			case 2: // FAT32
				CurCluster = *(u32 *)(pMemCache + FATOffset);
				memset(pMemCache + FATOffset, 0x0, sizeof(u32));
				break;
			default:	// FAT16
				CurCluster = *(u16 *)(pMemCache + FATOffset);
				memset(pMemCache + FATOffset, 0x0, sizeof(u16));
				break;
		}
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_EOF_FIND_ERR);
			FS__fat_free(pMemCache);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		Count++;
		
		switch(pBPBUnit->FATType)
		{
			case 1: // FAT12
				if(CurCluster >= 0xFF8)
					Loop = 0;
				break;
			case 2: // FAT32
				if(CurCluster >= 0x0FFFFFF8)
					Loop = 0;
				break;
			default:	// FAT16
				if(CurCluster >= 0xFFF8)
					Loop = 0;
				break;
		}
	}while(Loop);

	// Write back the modified data
	if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache) < 0))
	{
		ERRD(FS_LB_WRITE_DAT_ERR);
		FS__fat_free(pMemCache);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	FS__pDevInfo[Idx].FSInfo.FreeClusterCount += Count;
	if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
	{
		ERRD(FS_FAT_FSIS_UPDATE_ERR);
		FS__fat_free(pMemCache);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	// Update the storage space counter.
	global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

	time2 = OSTimeGet();
	DEBUG_GREEN("[INF] Free FAT link: %d (x50ms)\n", time2 - time1);

	FS__fat_free(pMemCache);
	OSSemPost(FSFATClustSemEvt);
	return 1;
#else
	FS__FAT_BPB *pBPBUnit;
	u32 FATIndex, FATSector, FATOffset, LastFATSector;
	u32 CurCluster, tmpVal, Count;
	int ret;
	u8 *pClustDataBuf;
	u8 err, Loop;
	char *pMemCache;

	u32 time1, time2;
	//
	if(StartCluster == 0x0)
	{
		ERRD(FS_PARAM_VALUE_ERR);
		return -1;
	}
	//
	OSSemPend(FSFATClustSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	CurCluster = StartCluster;
	Count = 0;
	Loop = 1;
	LastFATSector = 0xFFFFFFFF;

	time1 = OSTimeGet();
	if(pBPBUnit->FATType == 0x1)
	{
		if((pMemCache = FS__fat_malloc(pBPBUnit->BytesPerSec)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		
		do
		{
			FATIndex = CurCluster + (CurCluster >> 1);
			FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
			FATOffset = FATIndex % pBPBUnit->BytesPerSec;
			if (FATSector != LastFATSector)
			{				
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				LastFATSector = FATSector;
			}

			if(FATOffset == (pBPBUnit->BytesPerSec - 1))
			{
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				tmpVal = (*pMemCache << 8);
				if(CurCluster & 1)
					*pMemCache = 0x0;
				else
					*pMemCache &= 0xf0;
				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
				{
					ERRD(FS_LB_WRITE_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				
				if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
				{
					ERRD(FS_LB_READ_DAT_ERR);
					FS__fat_free(pMemCache);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				tmpVal |= *(pMemCache + FATOffset);
				if(CurCluster & 1)
				{
					*(pMemCache + FATOffset) &= 0x0f;
					tmpVal >>= 4;
				}
				else
					*(pMemCache + FATOffset) = 0x0;
				CurCluster = tmpVal & 0xfff;
			}
			else
			{
				tmpVal = *(u16 *)(pMemCache + FATOffset);
				if(CurCluster & 1)
				{
					*(pMemCache + FATOffset) &= 0x0f;
					*(pMemCache + FATOffset + 1) = 0x00;
					tmpVal >>= 4;
				}
				else
				{
					*(pMemCache + FATOffset) = 0x00;
					*(pMemCache + FATOffset + 1) &= 0xf0;
				}
				CurCluster = tmpVal & 0xfff;
			}
				
			if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_EOF_FIND_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return -1;
			}

			// Write back the modified data
			if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				ERRD(FS_LB_WRITE_DAT_ERR);
				FS__fat_free(pMemCache);
				OSSemPost(FSFATClustSemEvt);
				return ret;
			}
			Count++;
			
			if(CurCluster >= 0xFF8)
				Loop = 0;
		}while(Loop);

		FS__fat_free(pMemCache);
	}
	else
	{
		if((pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			OSSemPost(FSFATClustSemEvt);
			return -1;
		}
		do
		{
			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					FATIndex = CurCluster * 4;
					break;
				default:	// FAT16
					FATIndex = CurCluster * 2;
					break;
			}
			FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
			FATOffset = FATIndex % pBPBUnit->BytesPerSec;

			if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
			{
				if(Count != 0x0)
				{
					// Write back the modified data
					if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
					{
						ERRD(FS_LB_MUL_WRITE_DAT_ERR);
						FSFree(pClustDataBuf);
						OSSemPost(FSFATClustSemEvt);
						return ret;
					}
				}
				
				if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
				{
					ERRD(FS_LB_MUL_READ_DAT_ERR);
					FSFree(pClustDataBuf);
					OSSemPost(FSFATClustSemEvt);
					return ret;
				}
				LastFATSector = FATSector;
			}

			FATOffset += (FATSector - LastFATSector) * pBPBUnit->BytesPerSec;

			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					CurCluster = *(u32 *)(pClustDataBuf + FATOffset);
					memset(pClustDataBuf + FATOffset, 0x0, sizeof(u32));
					break;
				default:	// FAT16
					CurCluster = *(u16 *)(pClustDataBuf + FATOffset);
					memset(pClustDataBuf + FATOffset, 0x0, sizeof(u16));
					break;
			}
			
			if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_EOF_FIND_ERR);
				FSFree(pClustDataBuf);
				OSSemPost(FSFATClustSemEvt);
				return -1;
			}
			Count++;
			
			switch(pBPBUnit->FATType)
			{
				case 1: break;
				case 2: // FAT32
					if(CurCluster >= 0x0FFFFFF8)
						Loop = 0;
					break;
				default:	// FAT16
					if(CurCluster >= 0xFFF8)
						Loop = 0;
					break;
			}
		}while(Loop);

		// Write back the modified data
		if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, LastFATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_WRITE_DAT_ERR);
			FSFree(pClustDataBuf);
			OSSemPost(FSFATClustSemEvt);
			return ret;
		}

		FSFree(pClustDataBuf);
	}

	FS__pDevInfo[Idx].FSInfo.FreeClusterCount += Count;
	if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
	{
		ERRD(FS_FAT_FSIS_UPDATE_ERR);
		OSSemPost(FSFATClustSemEvt);
		return ret;
	}

	// Update the storage space counter.
	global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;

	time2 = OSTimeGet();
	DEBUG_GREEN("[INF] Free FAT link: %d (x50ms)\n", time2 - time1);
	
	OSSemPost(FSFATClustSemEvt);
	return 1;
#endif
}

int FSFATFreeFATLink_bg(s32 Idx, s32 Unit, s32 StartCluster, s32 dummy)
{
	return FSFATFreeFATLink((int) Idx, (u32) Unit, (u32) StartCluster);
}

int FSFATFileEntryUpdate(FS_FILE *pFile)
{
	FS__FAT_BPB *pBPBUnit;
	RTC_DATE_TIME LocalTime;
	FS_FAT_ENTRY *pEntry;
	u32 CurCluster, CurSector, RescanSwitch, i, j;
	u32 *pClustListBuf;
	u16 TmpTimeVal;
    char *pMemCache;
    u8 *pClustDataBuf, *ppClustDataBuf;
    int ret;

    RescanSwitch = 0;

    if(pFile == NULL)
    {
    	ERRD(FS_PARAM_PTR_EXIST_ERR);
    	return -1;
    }

    if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

	pBPBUnit = &FS__FAT_aBPBUnit[pFile->dev_index][pFile->fileid_lo];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FS__fat_free(pMemCache);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(pFile->dev_index, pFile->fileid_lo, pFile->fileid_ex, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		FS__fat_free(pMemCache);
		return ret;
	}

	if((CurCluster = pClustListBuf[pFile->FileEntrySect / pBPBUnit->SecPerClus]) == 0x0)
	{
		DEBUG_FS("[ERR] Entry offset over cluster list: %d\n", pFile->FileEntrySect);
		RescanSwitch = 1;
	}

	// Direct search by FileEntrySect
	if(!RescanSwitch)
	{
		if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
	    {
	    	ERRD(FS_FAT_SEC_CAL_ERR);
	    	FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
	    	return -1;
	    }

	    CurSector += pFile->FileEntrySect % pBPBUnit->SecPerClus;
	    if((ret = FS__lb_sin_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pMemCache)) < 0)
	    {
	    	ERRD(FS_LB_READ_DAT_ERR);
	    	FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
			return ret;
	    }

	    pEntry = (FS_FAT_ENTRY *)pMemCache;
	    do{
	    	if((char *)pEntry >= (pMemCache + FS_FAT_SEC_SIZE))
	    	{
	    		DEBUG_FS("[ERR] Can not find the entry. Offset: %d\n", pFile->FileEntrySect);
	    		RescanSwitch = 1;
	    		break;
	    	}
	    	
	    	CurCluster = (pEntry->data[21] << 24) | (pEntry->data[20] << 16) | (pEntry->data[27] << 8) | pEntry->data[26];
	    	if(pEntry->data[0] != 0xe5 && (CurCluster == pFile->fileid_hi))
	    		break;
	    	pEntry++;
	    }while(1);
	}

	// Search entry from start cluster to EOF cluster
    if(RescanSwitch)
    {
		i = 0;
		if((pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
		{
			ERRD(FS_MEMORY_ALLOC_ERR);
			FSFree((u8 *)pClustListBuf);
			FS__fat_free(pMemCache);
			return -1;
		}
    	do
    	{
    		CurCluster = pClustListBuf[i];
    		if(CurCluster == 0x0)
			{
				ERRD(FS_FAT_CLUT_FIND_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return -1;
			}

			if(FSFATCalculateSectorByCluster(pFile->dev_index, pFile->fileid_lo, &CurCluster, &CurSector) < 0)
			{
				ERRD(FS_FAT_SEC_CAL_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return -1;
			}

			if((ret = FS__lb_mul_read(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
			{
				ERRD(FS_LB_MUL_READ_DAT_ERR);
				FSFree((u8 *)pClustListBuf);
				FSFree(pClustDataBuf);
				FS__fat_free(pMemCache);
				return ret;
			}

			ppClustDataBuf = pClustDataBuf;
			for(j = 0; j < pBPBUnit->SecPerClus; j++)
			{
				pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
				do
				{
					if(pEntry->data[0] == 0x0)
					{
						DEBUG_FS("[INF] Entry end.\n");
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						FS__fat_free(pMemCache);
						return -1;
					}

					CurCluster = (pEntry->data[21] << 24) | (pEntry->data[20] << 16) | (pEntry->data[27] << 8) | pEntry->data[26];
			    	if(pEntry->data[0] != 0xe5 && (CurCluster == pFile->fileid_hi))
			    	{
			    		DEBUG_YELLOW("[WARM] Entry found. Name: %s, Offset: %d\n", pEntry->data, i * pBPBUnit->SecPerClus + j);
			    		RescanSwitch = 0;
			    		// Copy the memory data and relocate pEntry point.
			    		memcpy(pMemCache, ppClustDataBuf, pBPBUnit->BytesPerSec);			    		
			    		pEntry = (FS_FAT_ENTRY *) (pMemCache + ((u8 *)pEntry - ppClustDataBuf));
			    		CurSector += j;
			    		break;
			    	}
					pEntry++;
				}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
				if(RescanSwitch == 0)
					break;
				ppClustDataBuf += pBPBUnit->BytesPerSec;
			}
    	}while(RescanSwitch);
		FSFree(pClustDataBuf);
    }
    FSFree((u8 *)pClustListBuf);

    memcpy(&pEntry->data[28], &pFile->size, sizeof(u32));
    //DEBUG_GREEN("%s, size: %#x\n", &pEntry->data, pFile->size);
    RTC_Get_Time(&LocalTime);
    TmpTimeVal = ((LocalTime.hour & 0x1F) << 11) | ((LocalTime.min & 0x3F) << 5) | ((LocalTime.sec >> 1) & 0x1F);
    memcpy(&pEntry->data[22], &TmpTimeVal, sizeof(u16));
    TmpTimeVal = (((LocalTime.year + 20) & 0x7F) << 9) | ((LocalTime.month & 0xF) << 5) | (LocalTime.day & 0x1F);
    memcpy(&pEntry->data[24], &TmpTimeVal, sizeof(u16));

    if((ret = FS__lb_sin_write(FS__pDevInfo[pFile->dev_index].devdriver, pFile->fileid_lo, CurSector, pMemCache)) < 0)
    {
    	ERRD(FS_LB_WRITE_DAT_ERR);
    	FS__fat_free(pMemCache);
    	return ret;
    }

    FS__fat_free(pMemCache);
    return 1;
}

int FSFATFileDelete(FS_DIR *pDir, char *pFileName, FS_DeleteCondition *pCondition)
{
	FS__FAT_BPB *pBPBUnit;
	FS_DIR InDir;
	FS_FAT_ENTRY *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	u32 *pClustListBuf;
	int Idx, ret;
	u8 *pClustDataBuf, *ppClustDataBuf;

	if(pDir == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	if(pCondition == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	if(pCondition->DeleteMode == FS_E_DELETE_TYPE_ORDER)
	{
		if(pFileName == NULL)
		{
			ERRD(FS_PARAM_PTR_EXIST_ERR);
			return -1;
		}
	}

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(Idx, Unit, pDir->dirid_hi, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		return ret;
	}

	i = 0;
	if((pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FSFree((u8 *)pClustListBuf);
		return -1;
	}
	
	do
	{
		CurCluster = pClustListBuf[i];
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_CLUT_FIND_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if(FSFATCalculateSectorByCluster(Idx, Unit, &CurCluster, &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_READ_DAT_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return ret;
		}

		ppClustDataBuf = pClustDataBuf;
		for(j = 0; j < pBPBUnit->SecPerClus; j++)
		{
			pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
			do
			{
				if(pEntry->data[0] == 0x0)
				{
					DEBUG_FS("[INF] Entry end.\n");
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 0;
				}

				if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && (pEntry->data[0] != '.') &&
					(pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_DIRECTORY))
				{
					memset(&InDir, 0x0, sizeof(FS_DIR));
					InDir.dev_index = pDir->dev_index;
					InDir.dirid_hi = pDir->dirid_hi;
					InDir.dirid_lo = pDir->dirid_lo;
					InDir.dirid_ex = CurCluster;
					InDir.inuse = 1;
					// Release resource temporary
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);

					do
					{
						if((ret = FSFATFileDelete(&InDir, NULL, pCondition)) < 0)
						{
							ERRD(FS_FILE_DELETE_ERR);
							return ret;
						}
					}while(ret);

					if((pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
					{
						ERRD(FS_MEMORY_ALLOC_ERR);
						return -1;
					}
					if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
					{
						ERRD(FS_LB_MUL_READ_DAT_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}
					
					DEBUG_FS("[INF] InDir name: \\%s\\%s.\n", pDir->dirent.d_name, pEntry->data);
					// Mark the entry
					pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
					// Mark the entry
					pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
					// Write back the Entry info
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}
					// Free the FAT link
					TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
					if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
					{
						ERRD(FS_FAT_LINK_DELETE_ERR);
						FSFree(pClustDataBuf);
						return ret;
					}

					FSFree(pClustDataBuf);
					return 1;
				}
				else if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && (pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_ARCHIVE))
				{
					switch(pCondition->DeleteMode)
					{
						case FS_E_DELETE_TYPE_ORDER:
							if(strncmp((char *)pEntry->data, pFileName, FS_V_FAT_ENTEY_SHORT_NAME) != 0)
								break;
						case FS_E_DELETE_TYPE_AUTO:
							DEBUG_FS("[INF] File name: \\%s\\%s.\n", pDir->dirent.d_name, pEntry->data);
							
							// Mark the entry
							pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
							// Mark the entry
							pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
							// Write back the Entry info
							if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
							{
								ERRD(FS_LB_WRITE_DAT_ERR);
								FSFree((u8 *)pClustListBuf);
								FSFree(pClustDataBuf);
								return ret;
							}
							// Free the FAT link
							TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
							if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
							{
								ERRD(FS_FAT_LINK_DELETE_ERR);
								FSFree((u8 *)pClustListBuf);
								FSFree(pClustDataBuf);
								return ret;
							}

							FSFree((u8 *)pClustListBuf);
							FSFree(pClustDataBuf);
							return 1;
								
						default: 
							DEBUG_FS("[ERR] Mission impossible.\n");
							break;
					}
				}
				pEntry++;
			}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
			ppClustDataBuf += pBPBUnit->BytesPerSec;
		}
	}while(pClustListBuf[++i] != 0x0);

	DEBUG_FS("[INF] Still not found the file entry which can be deleted.\n");
	FSFree((u8 *)pClustListBuf);
	FSFree(pClustDataBuf);
	return 0;
}

int FSFATDirDelete(FS_DIR *pDir)
{
	FS__FAT_BPB *pBPBUnit;
	FS_FAT_ENTRY *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	u32 *pClustListBuf;
	int Idx, ret;
	u8 *pClustDataBuf, *ppClustDataBuf;

	if(pDir == NULL)
	{
		ERRD(FS_PARAM_PTR_EXIST_ERR);
		return -1;
	}

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pClustListBuf = (u32 *) FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	memset(pClustListBuf, 0x0, pBPBUnit->SizeOfCluster);
	if((ret = FSFATGetClusterList(Idx, Unit, pDir->dirid_ex, pClustListBuf)) < 0)
	{
		ERRD(FS_FAT_CLUT_FIND_ERR);
		FSFree((u8 *)pClustListBuf);
		return ret;
	}

	i = 0;
	if((pClustDataBuf = FSMalloc(pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus)) == NULL)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		FSFree((u8 *)pClustListBuf);
		return -1;
	}
	
	do
	{
		CurCluster = pClustListBuf[i];
		if(CurCluster == 0x0)
		{
			ERRD(FS_FAT_CLUT_FIND_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if(FSFATCalculateSectorByCluster(Idx, Unit, &CurCluster, &CurSector) < 0)
		{
			ERRD(FS_FAT_SEC_CAL_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return -1;
		}

		if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
		{
			ERRD(FS_LB_MUL_READ_DAT_ERR);
			FSFree((u8 *)pClustListBuf);
			FSFree(pClustDataBuf);
			return ret;
		}

		ppClustDataBuf = pClustDataBuf;
		for(j = 0; j < pBPBUnit->SecPerClus; j++)
		{
			pEntry = (FS_FAT_ENTRY *) ppClustDataBuf;
			do
			{
				if(pEntry->data[0] == 0x0)
				{
					DEBUG_FS("[INF] Entry end.\n");
					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 0;
				}

				if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && 
					(pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_DIRECTORY) && 
					(strncmp((char *)pEntry->data, pDir->dirent.d_name, FS_V_FAT_ENTEY_SHORT_NAME) == 0))
				{
					DEBUG_FS("[INF] Dir name: %s.\n", pEntry->data);
					
					// Mark the entry
					pEntry->data[0] = FS_V_FAT_ENTRY_DELETE;
					// Mark the entry
					pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] = 0x0;
					// Write back the Entry info
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector + j, ppClustDataBuf)) < 0)
					{
						ERRD(FS_LB_WRITE_DAT_ERR);
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						return ret;
					}
					// Free the FAT link
					TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
					if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
					{
						ERRD(FS_FAT_LINK_DELETE_ERR);
						FSFree((u8 *)pClustListBuf);
						FSFree(pClustDataBuf);
						return ret;
					}

					FSFree((u8 *)pClustListBuf);
					FSFree(pClustDataBuf);
					return 1;
				}
				pEntry++;
			}while((u8 *) pEntry < (ppClustDataBuf + pBPBUnit->BytesPerSec));
			ppClustDataBuf += pBPBUnit->BytesPerSec;
		}
	}while(pClustListBuf[++i] != 0x0);

	DEBUG_FS("[INF] Still not found the Dir entry which can deleted.\n");
	FSFree((u8 *)pClustListBuf);
	FSFree(pClustDataBuf);
	return 0;
}

#endif
/*********************************************************************
*
*             Global functions section
*
**********************************************************************

  Functions in this section are used by FAT File System layer only

*/


/*********************************************************************
*
*             FS__fat_checkunit
*
  Description:
  FS internal function. Read Bios-Parameter-Block from a device and
  check, if it contains valid data.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.

  Return value:
  ==1         - BPB is okay.
  ==0         - An error has occured.
*/

int FS__fat_checkunit(int Idx, u32 Unit)
{
    FS__FAT_BPB *pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    int err, status;

    status = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);
    if (status < 0)
    {
        ERRD(FS_DEV_ACCESS_ERR);
        return status;
    }
    if (status == FS_LBL_MEDIACHANGED || pBPBUnit->Signature != 0xaa55)
    {
        err = _FS_ReadBPB(Idx, Unit);
        if (err < 0)
        {
            if(FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_SET_STATUS, 0, (void*)0) < 0)
                ERRD(FS_DEV_IOCTL_ERR);
            return err;
        }
    }
    return 1;
}


/*********************************************************************
*
*             FS__fat_which_type
*
  Description:
  FS internal function. Determine FAT type used on a media. This
  function is following the MS specification very closely.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.

  Return value:
  ==0         - FAT16.
  ==1         - FAT12.
  ==2         - FAT32
*/

int FS__fat_which_type(int Idx, u32 Unit)
{
    FS__FAT_BPB *pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    u32 coc;
    u32 fatsize;
    u32 totsec;
    u32 datasec;
    u32 bytespersec;
    u32 dsize;

    bytespersec = (u32)pBPBUnit->BytesPerSec;
    if (bytespersec != 0)
        dsize = ((u32)((u32)pBPBUnit->RootEntCnt) * FS_FAT_DENTRY_SIZE) / bytespersec;
    else
        return -1;

    fatsize = pBPBUnit->FATSz16;
    if (fatsize == 0)
        fatsize = pBPBUnit->FATSz32;
    totsec = (u32)pBPBUnit->TotSec16;
    if (totsec == 0)
        totsec = pBPBUnit->TotSec32;
    datasec = totsec - (pBPBUnit->RsvdSecCnt + pBPBUnit->NumFATs * fatsize + dsize);
    if (pBPBUnit->SecPerClus!=0)
        coc = datasec / pBPBUnit->SecPerClus;
    else
        return -1;

    if (coc < 4085)
    {
        return 1;  // FAT12
    }
    else if (coc < 65525)
    {
        return 0;  // FAT16
    }
    return 2;  // FAT32
}


/*********************************************************************
*
*             FS__fat_FAT_find_eof
*
  Description:
  FS internal function. Find the next EOF mark in the FAT1.

  	#因為建立檔案或資料夾都會預設擴增FAT1 table link，
  	那這裡就是尋找已經被延長的eof mark.
  	return value = 跳躍了幾個cluster，1 = 原地

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  StrtClst    - Starting cluster in FAT.
  pClstCnt    - If not zero, this is a pointer to an u32, which
                is used to return the number of clusters found
                between StrtClst and the next EOF mark.

  Return value:
  >=0         - Cluster, which contains the EOF mark.
  <0          - An error has occured.
*/

int FS__fat_FAT_find_eof(int Idx, u32 Unit, s32 StrtClst, u32 *pClstCnt, u32 *pClustNum)
{
    FS__FAT_BPB *pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    u32 bytesPerSec, rsvdSecCnt, fatSize;
    u32 curClust, lastSec, fatSec, fatindex, fatoffs, clstCount, maxClst, eofClst;
    u8 a, b;
    int err;
    char *buffer;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    // max Cluster is too much ??
    switch(pBPBUnit->FATType)
    {
        case 1: // FAT12
            maxClst = 4085UL;
            break;
        case 2: // FAT32
            maxClst = 0x0ffffff0UL;
            break;
        default:	// FAT16
            maxClst = 65525UL;
            break;
    }
    //
    curClust = StrtClst;	// Set the position of Clusters in FAT1 table
    clstCount = 0;
    *pClstCnt = 0;
    *pClustNum = 0;
    //
    bytesPerSec = pBPBUnit->BytesPerSec;	// Usually use 0x200
    fatSize = pBPBUnit->FATSz16;	// the size fo FAT1 or FAT2 table
    if(fatSize == 0)
        fatSize = pBPBUnit->FATSz32;
    rsvdSecCnt = pBPBUnit->RsvdSecCnt;

    while(clstCount < maxClst)
    {
        eofClst = curClust; // At the beginning, update the eofClust value
        clstCount++;
        // In FAT1 table, FAT12 use 1.5 bytes to store cluster info,
        //	FAT16 use 2 bytes, then FAT32 use 4 bytes.
        switch(pBPBUnit->FATType)
        {
            case 1: // FAT12
                fatindex = curClust + (curClust / 2);
                break;
            case 2: // FAT32
                fatindex = curClust * 4;
                break;
            default:	// FAT16
                fatindex = curClust * 2;
                break;
        }
        // fatSec is the position of Sector number to locate the FAT1 table.
        // fatoffs is the position of FAT1 table within sector size when system read a sector.
        fatSec = rsvdSecCnt + (fatindex / bytesPerSec);
        fatoffs = fatindex % bytesPerSec;

        if (fatSec != lastSec)
        {
            err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatSec, fatSize + fatSec, (void*)buffer);
            if(err < 0)
            {
                ERRD(FS_LB_READ_FAT_TBL_ERR);
                FS__fat_free(buffer);
                return err;
            }
            lastSec = fatSec;
        }
        switch(pBPBUnit->FATType)
        {
            case 1: // FAT12
                if (fatoffs == (bytesPerSec - 1))
                {
                    a	= buffer[fatoffs];
                    err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatSec + 1, fatSize + fatSec + 1, (void*)buffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_FAT_TBL_ERR);
                        FS__fat_free(buffer);
                        return err;
                    }
                    lastSec = fatSec + 1;
                    b = buffer[0];
                }
                else
                {
                    a = buffer[fatoffs];
                    b = buffer[fatoffs + 1];
                }
                if (curClust & 1)
                {
                    curClust = ((a & 0xf0) >> 4 ) + 16 * b;
                }
                else
                {
                    curClust = a + ((b & 0x0f) << 8);
                }
                curClust &= 0x0fffL;
                if (curClust >= 0x0ff8L)
                {
                    // EOF found
                    *pClstCnt = clstCount;
                    *pClustNum = eofClst;
                    FS__fat_free(buffer);
                    return 1;
                }
                break;
            case 2: // FAT32
                curClust = *((u32 *)(buffer + fatoffs));
                curClust &= 0x0fffffffL;
                if (curClust >= 0x0ffffff8L)
                {
                    // EOF found
                    *pClstCnt = clstCount;
                    *pClustNum = eofClst;
                    FS__fat_free(buffer);
                    return 1;
                }
                break;
            default:	// FAT16
                curClust = *((u16 *)(buffer+fatoffs));
                curClust &= 0xffffL;
                if (curClust >= 0xfff8L)
                {
                    // EOF found
                    *pClstCnt = clstCount;
                    *pClustNum = eofClst;
                    FS__fat_free(buffer);
                    return 1;
                }
                break;
        }

        if(curClust == 0)
        {
            //It is imposible value.
            DEBUG_FS("[Error] Find FAT1 Link broken in EOF function.\n");
            *pClstCnt = clstCount;
            FS__fat_free(buffer);
            return -1;
        }
    } // while (clstcount<maxclst)
    DEBUG_FS("Can't find the target cluster in EOF function.\n");
    FS__fat_free(buffer);
    return -1;
}

/*********************************************************************
*
*             FS__fat_diskclust
*
  Description:
  FS internal function. Walk through the FAT starting at StrtClst for
  ClstNum times. Return the found cluster number of the media. This is
  very similar to FS__fat_FAT_find_eof.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  StrtClst    - Starting point for FAT walk.
  ClstNum     - Number of steps.

  Return value:
  > 0         - Number of cluster found after ClstNum steps.
  ==0         - An error has occured.
***********************************************************************/

int FS__fat_diskclust(int Idx, u32 Unit, u32 StrtClst, s32 ClstNum, u32 *CurClust)
{
    FS__FAT_BPB *pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    u32 bytesPerSec, rsvdSecCnt, fatSize;
    u32 curClust, lastSec, fatSec, fatindex, fatoffs;
    s32 rounds;
    u8 a, b;
    int err;
    char *buffer;
    //
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    rounds = ClstNum;	// Set the Cluster number to be the loop counter
    curClust = StrtClst;	// Set the position of Clusters in FAT1 table
    //
    bytesPerSec = pBPBUnit->BytesPerSec;	// Usually use 0x200
    fatSize = pBPBUnit->FATSz16;	// the size fo FAT1 or FAT2 table
    if(fatSize == 0)
        fatSize = pBPBUnit->FATSz32;
    rsvdSecCnt = pBPBUnit->RsvdSecCnt;

    while (rounds)
    {
        // In FAT1 table, FAT12 use 1.5 bytes to store cluster info,
        //	FAT16 use 2 bytes, then FAT32 use 4 bytes.
        switch(pBPBUnit->FATType)
        {
            case 1:	// FAT12
                fatindex = curClust + (curClust / 2);
                break;
            case 2:	// FAT32
                fatindex = curClust * 4;
                break;
            default:	// FAT16
                fatindex = curClust * 2;
                break;
        }
        // fatSec is the position of Sector number to locate the FAT1 table.
        // fatoffs is the position of FAT1 table within sector size when system read a sector.
        fatSec = rsvdSecCnt + (fatindex / bytesPerSec);
        fatoffs = fatindex % bytesPerSec;

        if (fatSec != lastSec)
        {
            err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatSec, fatSize + fatSec, (void*)buffer);
            if(err < 0)
            {
                ERRD(FS_LB_READ_FAT_TBL_ERR);
                FS__fat_free(buffer);
                return err;
            }
            lastSec = fatSec;
        }
        switch(pBPBUnit->FATType)
        {
            case 1:	// FAT12
                if (fatoffs == (bytesPerSec - 1))	// (512 / 3) = never end. We need the next sector to cover.
                {
                    a = buffer[fatoffs];
                    err = FS__lb_read_FAT_table(FS__pDevInfo[Idx].devdriver, Unit, fatSec + 1, fatSize + fatSec + 1, (void*)buffer);
                    if(err < 0)
                    {
                        ERRD(FS_LB_READ_FAT_TBL_ERR);
                        FS__fat_free(buffer);
                        return err;
                    }
                    lastSec = fatSec + 1;
                    b = buffer[0];
                }
                else
                {
                    a = buffer[fatoffs];
                    b = buffer[fatoffs + 1];
                }
                if (curClust & 1)
                {
                    curClust = ((a & 0xf0) >> 4) + 16 * b;
                }
                else
                {
                    curClust = a + ((b & 0x0f) << 8);
                }
                curClust &= 0x0fffL;
                if (curClust >= 0x0ff8L)	// 0x0ff8-0x0fff means final cluster, file is closed.
                {
                    FS__fat_free(buffer);
                    ERRD(FS_FAT_CLUT_FIND_ERR);
                    DEBUG_FS("rounds: %d, StrtClst: %d, ClstNum: %d\n", rounds, StrtClst, ClstNum);
                    return -1;
                }
                break;
            case 2:	// FAT32
                curClust = *((u32 *)(buffer + fatoffs));
                curClust &= 0x0fffffffL;
                if (curClust >= 0x0ffffff8L)	// 0x0ffffff8-0x0fffffff means final cluster, file is closed.
                {
                    FS__fat_free(buffer);
                    ERRD(FS_FAT_CLUT_FIND_ERR);
                    DEBUG_FS("rounds: %d, StrtClst: %d, ClstNum: %d\n", rounds, StrtClst, ClstNum);
                    return -1;
                }
                break;
            default:	// FAT16
                curClust = *((u16 *)(buffer + fatoffs));
                curClust &= 0xffffL;
                if (curClust >= 0xfff8L)	// 0xfff8-0xffff means final cluster, file is closed.
                {
                    FS__fat_free(buffer);
                    ERRD(FS_FAT_CLUT_FIND_ERR);
                    DEBUG_FS("rounds: %d, StrtClst: %d, ClstNum: %d\n", rounds, StrtClst, ClstNum);
                    return -1;
                }
                break;
        }
        rounds--;
    }
    FS__fat_free(buffer);
    *CurClust = curClust;
    if(curClust == 0)	// it means impossible
    {
        ERRD(FS_FAT_CLUT_FIND_ERR);
        return -1;
    }
    return 1;
}

static int _FS_CheckBPB(int Idx, u32 Unit)
{
    int err;
    u8 *buffer;
    u32 sdctotalblockcount;

    sdctotalblockcount = GetTotalBlockCount(Idx,Unit);

    // Check BPB general setting
    if (FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec==0 || FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec!=FS_SECTOR_SIZE)
    {
        DEBUG_FS("Check BPB: BytesPerSec must be 512 \n");
        return -1;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus==0)
    {
        DEBUG_FS("Check BPB: SecPerClus cannot be 0 \n");
        return -1;
    }
    else
    {
        if (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus%2 && FS__FAT_aBPBUnit[Idx][Unit].SecPerClus!=1) // Must be power of 2
        {
            DEBUG_FS("Check BPB:SecPerClus must be even \n");
            return -1;
        }
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt==0)
    {
        DEBUG_FS("Check BPB:RsvdSecCnt cannot be zero \n");
        return -1;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].NumFATs!=2)
    {
        DEBUG_FS("Check BPB:NumFATs must be 2 \n");
        return -1;
    }

    if ((FS__FAT_aBPBUnit[Idx][Unit].TotSec16+ FS__FAT_aBPBUnit[Idx][Unit].TotSec32)==0)
    {
        DEBUG_FS("Check BPB:Total sector cannot be zero \n");
        return -1;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16 == 0)
    {
        if (FS__FAT_aBPBUnit[Idx][Unit].ExtFlags & 0x0080)
        {
            DEBUG_FS("Check BPB:ExtFlags error \n");
            return -1;  /* Only mirroring at runtime supported */
        }
    }
    // Check FAT and root clst
    buffer = (u8*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt >= sdctotalblockcount)
    {
        DEBUG_FS("Check BPB:RsvdSecCnt >= sdctotalblockcount \n");
        return -1;
    }
    err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt, (void*)buffer);
    if (err < 0)
    {
        ERRD(FS_REAL_SEC_READ_ERR);
        FS__fat_free(buffer);
        return err;
    }
    // Check Meida Type
#if 0	/* Media Type is no effect on our system */
    if (FS__FAT_aBPBUnit[Idx][Unit].MediaDesc!=buffer[0])
    {
        FS__fat_free(buffer);
        return BPB_SETTING_ERROR;
    }
#endif
    switch (FS__FAT_aBPBUnit[Idx][Unit].FATType)
    {
        case 0:
        case 1:
            if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16==0 && FS__FAT_aBPBUnit[Idx][Unit].FATSz32!=0)
            {
                FS__fat_free(buffer);
                DEBUG_FS("Check BPB:FATSz16 cannot be zero \n");
                return -1;
            }

            if (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt==0)
            {
                FS__fat_free(buffer);
                DEBUG_FS("Check BPB:RootEntCnt cannot be zero \n");
                return -1;
            }
            else
            {
                if (((FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt*0x20)/FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec)%2)
                {
#if 0
                    FS__fat_free(buffer);
                    return BPB_SETTING_ERROR;
#else
                    DEBUG_FS("Warning! Root Entry sector not muliple of even!\n");
#endif
                }
            }
            if (FS__FAT_aBPBUnit[Idx][Unit].FATType==0)
            {
                if (buffer[1]!=0xFF && buffer[2]!=0xFF)
                {
                    DEBUG_FS("Check BPB:FAT12 FAT table start error \n");
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            else
            {
                if (buffer[1]!=0xFF && buffer[2]!=0xFF && buffer[3]!=0xFF)
                {
                    DEBUG_FS("Check BPB:FAT16 FAT table start error \n");
                    FS__fat_free(buffer);
                    return -1;
                }
            }

            break;
        case 2:
            if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16 !=0 && FS__FAT_aBPBUnit[Idx][Unit].FATSz32==0)
            {
                DEBUG_FS("Check BPB:FAT32 FATSize error\n");
                FS__fat_free(buffer);
                return -1;
            }

            if (FS__FAT_aBPBUnit[Idx][Unit].RootClus==0)
            {
                DEBUG_FS("Check BPB:FAT32 RootClus cannot be zero \n");
                FS__fat_free(buffer);
                return -1;
            }

            if (buffer[1]!=0xFF && buffer[2]!=0xFF && buffer[3]!=0x0F && buffer[4]!=0xFF && buffer[5]!=0xFF && buffer[6]!=0xFF && buffer[7]!=0x0F)
            {
                DEBUG_FS("Check BPB:FAT32 FAT table start error \n");
                FS__fat_free(buffer);
                return -1;
            }

            if (FS__FAT_aBPBUnit[Idx][Unit].RootClus==2) // Just check rootclst while equaling 2
            {
                if (buffer[8]==0x0 && buffer[9]==0x00 && buffer[0xA]==0x00 && buffer[0xB]==0x00)
                {
                    DEBUG_FS("Check BPB:FAT32 RootClus=2 \n");
                    FS__fat_free(buffer);
                    return -1;
                }
            }
            break;
    }

    FS__fat_free(buffer);
    return 1;
}
/*********************************************************************
*
*             _FS_ReadBPB
*
  Description:
  FS internal function. Read Bios-Parameter-Block from a device and
  copy the relevant data to FS__FAT_aBPBUnit.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.

  Return value:
  >=0         - BPB successfully read.
  <0          - An error has occured.
*/

/*CY 0718*/
static int _FS_ReadBPB(int Idx, u32 Unit)
{
	FS__FAT_BPB *pBPBUnit;
	u32 TotalBlock, Partition1Start;
	int err, i;
	char *buffer;

	buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
	if (!buffer)
	{
		ERRD(FS_MEMORY_ALLOC_ERR);
		return -1;
	}
	//
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	Partition1Start = 0;
	TotalBlock = GetTotalBlockCount(Idx,Unit);
	//
	// cytsai: find a Partition Boot Sector for Bios-Parameter-Block (possibly indexed by Master Boot Record)
	// read first sector (it may be a PBS or MBR)
	err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, 0, buffer);
	if (err < 0)
	{
		ERRD(FS_LB_READ_DAT_ERR);
		FS__fat_free(buffer);
		return err;
	}

	// check if it's a MBR or PBS
	if ((buffer[MBR_I_BOOT_SIGNATURE] == MBR_V_BOOT_SIGN_0x55) && (buffer[MBR_I_BOOT_SIGNATURE + 1] == MBR_V_BOOT_SIGN_0xAA))
	{
		if(((buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_STATUS] == 0x80)) &&
			((buffer[MBR_I_PARTI_ENTRY_02 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_02 + MBR_O_PARTI_ENT_STATUS] == 0x80)) &&
			((buffer[MBR_I_PARTI_ENTRY_03 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_03 + MBR_O_PARTI_ENT_STATUS] == 0x80)) &&
			((buffer[MBR_I_PARTI_ENTRY_04 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_04 + MBR_O_PARTI_ENT_STATUS] == 0x80)))
		{
			// It's a MBR
			memcpy(&Partition1Start, buffer + MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC, sizeof(u32));
			if(Partition1Start >= TotalBlock)
			{
				ERRD(BPB_SETTING_ERROR);
				DEBUG_FS("[ERR] SectorOfPartition1 >= TotalBlock.\n");
				FS__fat_free(buffer);
				return -1;
			}

			// Load the P1 volume BPB
			if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, Partition1Start, buffer)) < 0)
			{
				ERRD(FS_LB_READ_DAT_ERR);
				FS__fat_free(buffer);
				return err;
			}
			if((buffer[MBR_I_BOOT_SIGNATURE] != MBR_V_BOOT_SIGN_0x55) || (buffer[MBR_I_BOOT_SIGNATURE + 1] != MBR_V_BOOT_SIGN_0xAA) ||
				(buffer[FAT_BPB_I_BYTES_PER_SEC] != 0x00) || (buffer[FAT_BPB_I_BYTES_PER_SEC + 1] != 0x02))
			{
				ERRD(BPB_SETTING_ERROR);
				FS__fat_free(buffer);
				return -1;
			}
		}
	}

	if((buffer[MBR_I_BOOT_SIGNATURE] != MBR_V_BOOT_SIGN_0x55) || (buffer[MBR_I_BOOT_SIGNATURE + 1] != MBR_V_BOOT_SIGN_0xAA))
	{
		DEBUG_FS("[ERR] Signature error!\n");
		ERRD(BPB_SETTING_ERROR);
		FS__fat_free(buffer);
		return -1;
	}

	// Reassign BPB table
	memcpy(&pBPBUnit->BytesPerSec, buffer + FAT_BPB_I_BYTES_PER_SEC, sizeof(u16));			// _512_, 1024, 2048, 4096
	memcpy(&pBPBUnit->SecPerClus, buffer + FAT_BPB_I_LOGI_SEC_PER_CLUS, sizeof(u8));		// 8, 32, 64, 128
	memcpy(&pBPBUnit->RsvdSecCnt, buffer + FAT_BPB_I_RESERV_SEC, sizeof(u16));
	pBPBUnit->RsvdSecCnt += Partition1Start;
	memcpy(&pBPBUnit->NumFATs, buffer + FAT_BPB_I_NUM_FATS, sizeof(u8));					// 2 usually 
	memcpy(&pBPBUnit->RootEntCnt, buffer + FAT_BPB_I_ROOT_DIR_ENTRY, sizeof(u16));
	memcpy(&pBPBUnit->TotSec16, buffer + FAT_BPB_I_TOTAL_LOGI_SEC, sizeof(u16));
	memcpy(&pBPBUnit->MediaDesc, buffer + FAT_BPB_I_MEDIA_DESC, sizeof(u8));
	memcpy(&pBPBUnit->FATSz16, buffer + FAT_BPB_I_1216_SIZE, sizeof(u16));
	memcpy(&pBPBUnit->TotSec32, buffer + FAT_BPB_I_LARGE_TOTAL_LOGI_SEC, sizeof(u32));

	if(pBPBUnit->FATSz16)
	{
		// FAT16 / FAT12
		// Number of Sector: if Size larger than 32M take 0x20, less than 32M take 0x13.
		pBPBUnit->FATSz32 = 0;
		pBPBUnit->ExtFlags = 0;
		pBPBUnit->RootClus = 0;
		pBPBUnit->FSInfo = 0;
		pBPBUnit->FatEndSec = pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz16;
		pBPBUnit->RootDirSec = pBPBUnit->RsvdSecCnt + pBPBUnit->NumFATs * pBPBUnit->FATSz16;
		pBPBUnit->Dsize = pBPBUnit->RootEntCnt * FS_FAT_DENTRY_SIZE / FS_FAT_SEC_SIZE;
		pBPBUnit->LimitsOfCluster = (pBPBUnit->TotSec32 - pBPBUnit->RootDirSec) / 
			((pBPBUnit->SecPerClus != 0x0)? pBPBUnit->SecPerClus: 1);	// Avoid to divide by 0
		FS__pDevInfo[Idx].FSInfo.FreeClusterCount = 0;
		FS__pDevInfo[Idx].FSInfo.NextFreeCluster = 0;
	}
	else
	{
		// FAT32
		memcpy(&pBPBUnit->FATSz32, buffer + FAT_BPB_I_32_SIZE, sizeof(u32));
		memcpy(&pBPBUnit->ExtFlags, buffer + FAT_BPB_I_MIRROR_FLAG, sizeof(u16));
		memcpy(&pBPBUnit->RootClus, buffer + FAT_BPB_I_ROOT_CLUS, sizeof(u32));
		memcpy(&pBPBUnit->FSInfo, buffer + FAT_BPB_I_FS_INFO, sizeof(u16));
		if(pBPBUnit->FSInfo != 0x1)
		{
			pBPBUnit->FSInfo = 0x1;
			DEBUG_YELLOW("[WARM] The shift of FSInfo was wrong.\n");
		}
		pBPBUnit->FSInfo += Partition1Start;
		// Saving time to speed up search first free cluster in old FS FSInfo.
		FS__pDevInfo[Idx].TagOfFirstFreeClust = 0x0;
		pBPBUnit->FatEndSec = pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz32;
		pBPBUnit->RootDirSec = pBPBUnit->RsvdSecCnt + pBPBUnit->NumFATs * pBPBUnit->FATSz32;
		pBPBUnit->Dsize = 0;
		pBPBUnit->LimitsOfCluster = (pBPBUnit->TotSec32 - pBPBUnit->RootDirSec) / 
			((pBPBUnit->SecPerClus != 0x0)? pBPBUnit->SecPerClus: 1);	// Avoid to divide by 0
		// Load the P1 volume BPB
		if((err = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, pBPBUnit->FSInfo, buffer)) < 0)
		{
			ERRD(FS_LB_READ_DAT_ERR);
			FS__fat_free(buffer);
			return err;
		}
		memcpy(&FS__pDevInfo[Idx].FSInfo.FreeClusterCount, buffer + FAT_FSIS_I_FREE_CLUS_COUNT, sizeof(u32)); 
		memcpy(&FS__pDevInfo[Idx].FSInfo.NextFreeCluster, buffer + FAT_FSIS_I_NEXT_FREE_CLUS, sizeof(u32));
		if(FS__pDevInfo[Idx].FSInfo.NextFreeCluster == 0xFFFFFFFF)
			FS__pDevInfo[Idx].FSInfo.NextFreeCluster = 0;
	}
	memcpy(&pBPBUnit->Signature, buffer + MBR_I_BOOT_SIGNATURE, sizeof(u16));
	FS__fat_free(buffer);	// Mission over.

	if((pBPBUnit->FATType = FS__fat_which_type(Idx, Unit)) == -1)
	{
		DEBUG_FS("Not FAT12/16/32!\n");
		ERRD(BPB_SETTING_ERROR);
		return -1;
	}	

	fsStorageSectorCount = pBPBUnit->TotSec16;
	if (!fsStorageSectorCount)
		fsStorageSectorCount = pBPBUnit->TotSec32;
	
	if((err = _FS_CheckBPB(Idx,Unit)) < 0)
	{
		ERRD(FS_FAT_DAT_FORM_ERR);
		return err; // return -1 would block the format function?
	}

#if FS_USE_LB_READCACHE
	FS_LB_Cache_Enable(Idx, Unit);
#endif

	pBPBUnit->SizeOfCluster = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
	// Record the Bitwise
	pBPBUnit->BitRevrOfBPS = pBPBUnit->BytesPerSec - 1;
	for(i = 0; i < 32; i++)
	{
		if((1 << i) & pBPBUnit->BytesPerSec)
			pBPBUnit->BitNumOfBPS = i;
		if((1 << i) & pBPBUnit->SecPerClus)
			pBPBUnit->BitNumOfSPC = i;
		if((1 << i) & pBPBUnit->SizeOfCluster)
			pBPBUnit->BitNumOfSOC = i;
	}

	FSPlaybackCacheBufferReset();

	return 1;
}


/*********************************************************************
*
*             Global Variables
*
**********************************************************************
*/

const FS__fsl_type FS__fat_functable =
{
#if (FS_FAT_NOFAT32==0)
    "FAT12/FAT16/FAT32",
#else
    "FAT12/FAT16",
#endif /* FS_FAT_NOFAT32==0 */
    FS__fat_fopen,        // open
    FS__fat_fclose,       // close
    FS__fat_fread,        // read
    FS__fat_fwrite,       // write
    0,                    // tell
    0,                    // seek
    FS__fat_ioctl,        // ioctl
#if FS_POSIX_DIR_SUPPORT
    FS__fat_opendir,      // opendir
    FS__fat_closedir,     // closedir
    FS__fat_readdir,      // readdir
    0,                    // rewinddir
    FS__fat_MkRmDir,      // mkdir
    FS__fat_MkRmDir,      // rmdir
#endif  /* FS_POSIX_DIR_SUPPORT */
};

