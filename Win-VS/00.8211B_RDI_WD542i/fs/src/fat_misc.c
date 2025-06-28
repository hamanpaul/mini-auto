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
#include "General.h"



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

typedef struct
{
    int Idx;
    FS_u32 Unit;
    FS_i32 *pFATSector;
    FS_i32 *pLastSector;
    FS_i32 *pFATOffset;
    FS_i32 LastClust;
    unsigned char *pBuffer;
    int FSysType;
    FS_u32 FATSize;
    FS_i32 BytesPerSec;
}
_FS_FAT_FindFreeCluster_Para;

/*********************************************************************
*
*             Global Variables
*
**********************************************************************
*/
__align(16) static _FS_FAT_block_type		   _FS_memblock[FS_MEMBLOCK_NUM];

unsigned long fsStorageSectorCount = 0; /*CY 1023*/
FS_i32 fat_bpb_err;

OS_EVENT *FSSecIncSemEvt;

//---Used for updating Disk information---//
static FS_i32 gCurclst;
static int gIdx;
static FS_u32 gUnit;


/*********************************************************************
*
*             Extern functions
*
**********************************************************************/
extern u32 sdcGetTotalBlockCount();


/*********************************************************************
*
*             External Global Variables
*
**********************************************************************/
extern u8 *PKBuf;
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
#if FS_NEW_VERSION
int FSFATFreeFATLink(int Idx, u32 Unit, u32 StartCluster)
{
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
		DEBUG_FS("[ERR] FS_PARAM_VALUE_ERR (file %s line %d)\n", __FILE__, __LINE__);
		return -1;
	}
	//
	OSSemPend(FSSecIncSemEvt, OS_IPC_WAIT_FOREVER, &err);
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
	if((pMemCache = FS__fat_malloc(pBPBUnit->BytesPerSec)) == NULL)
	{
		DEBUG_FS("[ERR] FS_MEMORY_ALLOC_ERR (file %s line %d)\n", __FILE__, __LINE__);
		OSSemPost(FSSecIncSemEvt);
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
				FATIndex = CurCluster + (CurCluster / 2);
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
					DEBUG_FS("[ERR] FS_LB_WRITE_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
					FS__fat_free(pMemCache);
					OSSemPost(FSSecIncSemEvt);
					return ret;
				}
			}
		
			if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
			{
				DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
				FS__fat_free(pMemCache);
				OSSemPost(FSSecIncSemEvt);
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
						DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
						FS__fat_free(pMemCache);
						OSSemPost(FSSecIncSemEvt);
						return ret;
					}
					tmpVal = (*pMemCache << 8);
					if(CurCluster & 1)
						*pMemCache = 0x0;
					else
						*pMemCache &= 0xf0;
					if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pMemCache)) < 0)
					{
						DEBUG_FS("[ERR] FS_LB_WRITE_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
						FS__fat_free(pMemCache);
						OSSemPost(FSSecIncSemEvt);
						return ret;
					}
					
					if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pMemCache)) < 0)
					{
						DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
						FS__fat_free(pMemCache);
						OSSemPost(FSSecIncSemEvt);
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
			DEBUG_FS("[ERR] FS_FAT_EOF_FIND_ERR (file %s line %d)\n", __FILE__, __LINE__);
			FS__fat_free(pMemCache);
			OSSemPost(FSSecIncSemEvt);
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
		DEBUG_FS("[ERR] FS_LB_WRITE_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
		FS__fat_free(pMemCache);
		OSSemPost(FSSecIncSemEvt);
		return ret;
	}
	

	FS__pDevInfo[Idx].FSInfo.FreeClusterCount += Count;
#if 0
	if((ret = FSFATSetFSInfo(Idx, Unit)) < 0)
	{
		DEBUG_FS("[ERR] FS_FAT_FSIS_UPDATE_ERR (file %s line %d)\n", __FILE__, __LINE__);
		FS__fat_free(pMemCache);
		OSSemPost(FSSecIncSemEvt);
		return ret;
	}
#endif
	// Update the storage space counter.
	global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;


	time2 = OSTimeGet();
	DEBUG_GREEN("[INF] Free FAT link: %d (x50ms)\n", time2 - time1);

	FS__fat_free(pMemCache);
	OSSemPost(FSSecIncSemEvt);
	return 1;
}

int FSFATFileDelete(FS_DIR *pDir, char *pFileName, FS_DeleteCondition *pCondition)
{
	FS__FAT_BPB *pBPBUnit;
	FS__fat_dentry_type *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	int Idx, ret;
	u8 *pMemCache;
	
	if(pDir == NULL)
	{
		DEBUG_FS("[ERR] FS_PARAM_PTR_EXIST_ERR (file %s line %d)\n", __FILE__, __LINE__);
		return -1;
	}

	if(pCondition == NULL)
	{
		DEBUG_FS("[ERR] FS_PARAM_PTR_EXIST_ERR (file %s line %d)\n", __FILE__, __LINE__);
		return -1;
	}

	if(pCondition->DeleteMode == FS_E_DELETE_TYPE_ORDER)
	{
		if(pFileName == NULL)
		{
			DEBUG_FS("[ERR] FS_PARAM_PTR_EXIST_ERR (file %s line %d)\n", __FILE__, __LINE__);
			return -1;
		}
	}

	if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
    	DEBUG_FS("[ERR] FS_MEMORY_ALLOC_ERR (file %s line %d)\n", __FILE__, __LINE__);
        return -1;
    }

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	for(i = 0; i < pDir->size; i++)
	{
		if((i % pBPBUnit->SecPerClus) == 0x0)
			j = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_hi, i);
        else
            j += 1;
            
        if(j == 0)
        {
            DEBUG_FS("[ERR] Cannot convert logical sector.\n");
            FS__fat_free(pMemCache);
            return -1;
        }
        
		if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, j, (void *)pMemCache)) < 0)
        {
        	DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
        	FS__fat_free(pMemCache);
            return -1;
        }

        pEntry = (FS__fat_dentry_type *)pMemCache;
        do
        {
			if(pEntry->data[0] == 0x0)
			{
				DEBUG_FS("[INF] Entry end.\n");
				FS__fat_free(pMemCache);
				return 0;
			}

			if((pEntry->data[0] != FS_V_FAT_ENTRY_DELETE) && (pEntry->data[FAT_FAT_I_ENTEY_ATTRIBUTE] == FS_FAT_ATTR_ARCHIVE))
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
						if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, j, (void *)pMemCache)) < 0)
				        {
				        	DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
				        	FS__fat_free(pMemCache);
				            return ret;
				        }

						// Free the FAT link
						TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
						if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
						{
							DEBUG_FS("[ERR] FS_FAT_LINK_DELETE_ERR (file %s line %d)\n", __FILE__, __LINE__);
							FS__fat_free(pMemCache);
							return ret;
						}

						FS__fat_free(pMemCache);
						return 1;
					default: 
						DEBUG_FS("[ERR] Mission impossible.\n");
						break;
				}
			}
        	pEntry++;
        }while((u8 *) pEntry < (pMemCache + pBPBUnit->BytesPerSec));
	}

	DEBUG_FS("[INF] Still not found the file entry which can be deleted.\n");
	FS__fat_free(pMemCache);
	return 0;
}

int FSFATDirDelete(FS_DIR *pDir)
{
	FS__FAT_BPB *pBPBUnit;
	FS__fat_dentry_type *pEntry;
	u32 Unit, CurCluster, CurSector;
	u32 TmpVal, i, j;
	int Idx, ret;
	u8 *pMemCache;
	
	if(pDir == NULL)
	{
		DEBUG_FS("[ERR] FS_PARAM_PTR_EXIST_ERR (file %s line %d)\n", __FILE__, __LINE__);
		return -1;
	}

	if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
    	DEBUG_FS("[ERR] FS_MEMORY_ALLOC_ERR (file %s line %d)\n", __FILE__, __LINE__);
        return -1;
    }

	Idx = pDir->dev_index;
	Unit = pDir->dirid_lo;
	pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

	for(i = 0; i < pDir->size; i++)
	{
		if((i % pBPBUnit->SecPerClus) == 0x0)
			j = FS__fat_dir_realsec(pDir->dev_index, pDir->dirid_lo, pDir->dirid_ex, i);
        else
            j += 1;
            
        if(j == 0)
        {
            DEBUG_FS("[ERR] Cannot convert logical sector.\n");
            FS__fat_free(pMemCache);
            return -1;
        }
        
		if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, j, (void *)pMemCache)) < 0)
        {
        	DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
        	FS__fat_free(pMemCache);
            return -1;
        }

        pEntry = (FS__fat_dentry_type *)pMemCache;
        do
        {
			if(pEntry->data[0] == 0x0)
			{
				DEBUG_FS("[INF] Entry end.\n");
				FS__fat_free(pMemCache);
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
				if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, j, (void *)pMemCache)) < 0)
		        {
		        	DEBUG_FS("[ERR] FS_LB_READ_DAT_ERR (file %s line %d)\n", __FILE__, __LINE__);
		        	FS__fat_free(pMemCache);
		            return ret;
		        }

				// Free the FAT link
				TmpVal = pEntry->data[26] + (pEntry->data[27] << 8) + (pEntry->data[20] << 16) + (pEntry->data[21] << 24);
				if((ret = FSFATFreeFATLink(Idx, Unit, TmpVal)) < 0)
				{
					DEBUG_FS("[ERR] FS_FAT_LINK_DELETE_ERR (file %s line %d)\n", __FILE__, __LINE__);
					FS__fat_free(pMemCache);
					return ret;
				}

				FS__fat_free(pMemCache);
				return 1;
			}
        	pEntry++;
        }while((u8 *) pEntry < (pMemCache + pBPBUnit->BytesPerSec));
	}

	DEBUG_FS("[INF] Still not found the file entry which can be deleted.\n");
	FS__fat_free(pMemCache);
	return 0;
}
#endif
static int _FS_CheckBPB(int Idx, FS_u32 Unit)
{
    int err;
    unsigned char *buffer;
    u32 sdctotalblockcount;

    sdctotalblockcount=sdcGetTotalBlockCount();

    // Check BPB general setting
    if (FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec==0 || FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec!=FS_SECTOR_SIZE)
    {
        DEBUG_FS("Check BPB: BytesPerSec must be 512 \n");
        return BPB_SETTING_ERROR;
    }
    
    if (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus==0)
    {
        DEBUG_FS("Check BPB: SecPerClus cannot be 0 \n");
        return BPB_SETTING_ERROR;
    }
    else
    {
        if (FS__FAT_aBPBUnit[Idx][Unit].SecPerClus%2 && FS__FAT_aBPBUnit[Idx][Unit].SecPerClus!=1) // Must be power of 2
        {
            DEBUG_FS("Check BPB:SecPerClus must be even \n");
            return BPB_SETTING_ERROR;
        }
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt==0)
    { 
        DEBUG_FS("Check BPB:RsvdSecCnt cannot be zero \n");
        return BPB_SETTING_ERROR;
    }
    
    if (FS__FAT_aBPBUnit[Idx][Unit].NumFATs!=2)
    {
        DEBUG_FS("Check BPB:NumFATs must be 2 \n");
        return BPB_SETTING_ERROR;
    }

    if ((FS__FAT_aBPBUnit[Idx][Unit].TotSec16+ FS__FAT_aBPBUnit[Idx][Unit].TotSec32)==0)
    {
        DEBUG_FS("Check BPB:Total sector cannot be zero \n");
        return BPB_SETTING_ERROR;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16 == 0)
    {
        if (FS__FAT_aBPBUnit[Idx][Unit].ExtFlags & 0x0080)
        {
            DEBUG_FS("Check BPB:ExtFlags error \n");
            return BPB_SETTING_ERROR;  /* Only mirroring at runtime supported */
        }
    }
    // Check FAT and root clst
    buffer = (unsigned char*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("Check BPB:FS__fat_malloc is Fail\n");
        return -1;
    }

    if (FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt >= sdctotalblockcount)
    {
       DEBUG_FS("Check BPB:RsvdSecCnt >= sdctotalblockcount \n");
       FS__fat_free(buffer);
       return BPB_SETTING_ERROR;
    }
#if FS_RW_DIRECT
    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt, (void*)buffer);
#else
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt, (void*)buffer);
#endif

    if (err < 0)
    {
        FS__fat_free(buffer);
        return -1;
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
            return BPB_SETTING_ERROR;
        }

        if (FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt==0)
        {
            FS__fat_free(buffer);
            DEBUG_FS("Check BPB:RootEntCnt cannot be zero \n");
            return BPB_SETTING_ERROR;
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
                return FAT_SETTING_ERROR;
            }
        }
        else
        {
            if (buffer[1]!=0xFF && buffer[2]!=0xFF && buffer[3]!=0xFF)
            {
                DEBUG_FS("Check BPB:FAT16 FAT table start error \n");
                FS__fat_free(buffer);
                return FAT_SETTING_ERROR;
            }
        }

        break;
    case 2:
        if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16 !=0 && FS__FAT_aBPBUnit[Idx][Unit].FATSz32==0)
        {
            DEBUG_FS("Check BPB:FAT32 FATSize error\n");
            FS__fat_free(buffer);
            return BPB_SETTING_ERROR;
        }

        if (FS__FAT_aBPBUnit[Idx][Unit].RootClus==0)
        {
            DEBUG_FS("Check BPB:FAT32 RootClus cannot be zero \n");
            FS__fat_free(buffer);
            return BPB_SETTING_ERROR;
        }

        if (buffer[1]!=0xFF && buffer[2]!=0xFF && buffer[3]!=0x0F && buffer[4]!=0xFF && buffer[5]!=0xFF && buffer[6]!=0xFF && buffer[7]!=0x0F)
        {
            DEBUG_FS("Check BPB:FAT32 FAT table start error \n");
            FS__fat_free(buffer);
            return FAT_SETTING_ERROR;
        }

        if (FS__FAT_aBPBUnit[Idx][Unit].RootClus==2) // Just check rootclst while equaling 2
        {
            if (buffer[8]==0x0 && buffer[9]==0x00 && buffer[0xA]==0x00 && buffer[0xB]==0x00)
            {
                DEBUG_FS("Check BPB:FAT32 RootClus=2 \n");
                FS__fat_free(buffer);
                return FAT_SETTING_ERROR;
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
  ==0         - BPB successfully read.
  <0          - An error has occured.
*/

/*CY 0718*/
static int _FS_ReadBPB(int Idx, FS_u32 Unit)
{
    int err;
    unsigned char *buffer;
    unsigned int partitionStart = 0;
    unsigned int sdctotalblockcount;


    sdctotalblockcount=sdcGetTotalBlockCount();
  
    buffer = (unsigned char*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return BUFFER_ALLOC_ERROR;
    }

    /* cytsai: find a Partition Boot Sector for Bios-Parameter-Block (possibly indexed by Master Boot Record) */
    /* read first sector (it may be a PBS or MBR) */
#if FS_RW_DIRECT
    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
#else
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
#endif
    if (err < 0)
    {
        FS__fat_free(buffer);
        DEBUG_FS("---READ_SECTOR_ERROR---\n");
        return READ_SECTOR_ERROR;
    }

    // check if it's a MBR or PBS
    if ((buffer[510] == 0x55) && (buffer[511] == 0xaa))
    {
    	if(((buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_STATUS] == 0x80)) && 
    		((buffer[MBR_I_PARTI_ENTRY_02 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_02 + MBR_O_PARTI_ENT_STATUS] == 0x80)) && 
    		((buffer[MBR_I_PARTI_ENTRY_03 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_03 + MBR_O_PARTI_ENT_STATUS] == 0x80)) && 
    		((buffer[MBR_I_PARTI_ENTRY_04 + MBR_O_PARTI_ENT_STATUS] == 0x00) || (buffer[MBR_I_PARTI_ENTRY_04 + MBR_O_PARTI_ENT_STATUS] == 0x80)))
        { 
        	// it's a MBR
            partitionStart = ((buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 3]) << 24) |
                             ((buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 2]) << 16) |
                             ((buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 1]) <<  8) |
                             (buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC]);
            
            if (partitionStart >= sdctotalblockcount)
            {
               DEBUG_FS("partitionStart >= sdctotalblockcount\n");
               FS__fat_free(buffer);
               return BPB_SETTING_ERROR;
            }
            
         	if(partitionStart != 0)	// Avoid RX FAT32's format form, all is 0x0
         	{
         	#if FS_RW_DIRECT
	        	err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart, (void*)buffer);
	        #else
	        	err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, partitionStart, (void*)buffer);
	        #endif
	            if (err < 0)
	            {
	                FS__fat_free(buffer);
	                DEBUG_FS("---READ_SECTOR_ERROR---\n");
	                return READ_SECTOR_ERROR;
	            }
	            
	            if ((buffer[MBR_I_BOOT_SIGNATURE] != MBR_V_BOOT_SIGN_0x55) || (buffer[MBR_I_BOOT_SIGNATURE + 1] != MBR_V_BOOT_SIGN_0xAA) ||
	                    (buffer[0xB] != 0x00) || (buffer[0xB + 1] != 0x02))
	            {
	                FS__fat_free(buffer);
	                return FAT_SETTING_ERROR;
	            }
         	}
        }
    }
    else
    {
      #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
      #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
      #endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            DEBUG_FS("---READ_SECTOR_ERROR---\n");
            return READ_SECTOR_ERROR;
        }
        if ((buffer[510] == 0x55) && (buffer[511] == 0xaa))
        {
            err=err;
        }
        else
        {
            FS__fat_free(buffer);
            DEBUG_FS("Signature error!\n");
            return BPB_SETTING_ERROR;
        }
    }

    /* Assign FS__FAT_aBPBUnit */
    FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec   = buffer[11] + 256 * buffer[12];      /* _512_,1024,2048,4096             */
    FS__FAT_aBPBUnit[Idx][Unit].SecPerClus    = buffer[13];                         /* sec in allocation unit           */
    FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt    = buffer[14] + 256 * buffer[15] + partitionStart;
    /* 1 + partitionStart for FAT12 & FAT16; 32 + partitionStart for FAT32 */
    FS__FAT_aBPBUnit[Idx][Unit].NumFATs       = buffer[16];                         /* 2                                */
    FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt    = buffer[17] + 256 * buffer[18];      /* number of root dir entries       */
    FS__FAT_aBPBUnit[Idx][Unit].TotSec16      = buffer[19] + 256 * buffer[20];      /* RSVD + FAT + ROOT + FATA (<64k)  */
    FS__FAT_aBPBUnit[Idx][Unit].MediaDesc     =  buffer[21];
    FS__FAT_aBPBUnit[Idx][Unit].FATSz16       = buffer[22] + 256 * buffer[23];      /* number of FAT sectors            */
    FS__FAT_aBPBUnit[Idx][Unit].TotSec32      = buffer[32] + 0x100UL * buffer[33]   /* RSVD + FAT + ROOT + FATA (>=64k) */
                                                + 0x10000UL * buffer[34]
                                                + 0x1000000UL * buffer[35];
    if (FS__FAT_aBPBUnit[Idx][Unit].FATSz16 == 0)
    {	/* cytsai: FAT32 */
        FS__FAT_aBPBUnit[Idx][Unit].FATSz32       = buffer[36] + 0x100UL * buffer[37]   /* number of FAT sectors          */
                                                    + 0x10000UL * buffer[38] + 0x1000000UL * buffer[39];
        FS__FAT_aBPBUnit[Idx][Unit].ExtFlags      = buffer[40] + 256 * buffer[41];      /* mirroring info                 */
        FS__FAT_aBPBUnit[Idx][Unit].RootClus      = buffer[44] + 0x100UL * buffer[45]   /* root dir clus for FAT32        */
                                                    + 0x10000UL * buffer[46]
                                                    + 0x1000000UL * buffer[47];
        FS__FAT_aBPBUnit[Idx][Unit].FSInfo        = buffer[48] + 256 * buffer[49];      /* position of FSInfo structure   */
        FS__FAT_aBPBUnit[Idx][Unit].FatEndSec     = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
        FS__FAT_aBPBUnit[Idx][Unit].DirStartSec   = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs*FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
        FS__FAT_aBPBUnit[Idx][Unit].Dsize = 0;
    }
    else
    {						/* cytsai: FAT16 / FAT12 */
        FS__FAT_aBPBUnit[Idx][Unit].FATSz32       = 0;
        FS__FAT_aBPBUnit[Idx][Unit].ExtFlags      = 0;
        FS__FAT_aBPBUnit[Idx][Unit].RootClus      = 0;
        FS__FAT_aBPBUnit[Idx][Unit].FSInfo        = 0;
        FS__FAT_aBPBUnit[Idx][Unit].FatEndSec     = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
        FS__FAT_aBPBUnit[Idx][Unit].DirStartSec   = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + FS__FAT_aBPBUnit[Idx][Unit].NumFATs*FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
        FS__FAT_aBPBUnit[Idx][Unit].Dsize         = FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt * FS_FAT_DENTRY_SIZE /FS_FAT_SEC_SIZE;
    }
    FS__FAT_aBPBUnit[Idx][Unit].Signature     = buffer[FS_FAT_SEC_SIZE-2] + 256 * buffer[FS_FAT_SEC_SIZE-1];
    /* (VCC) Check FAT Type */
    FS__FAT_aBPBUnit[Idx][Unit].FATType = FS__fat_which_type(Idx, Unit);

    if ( FS__FAT_aBPBUnit[Idx][Unit].FATType==-1)
    {
        DEBUG_FS("Not FAT12/16/32! \n");
        FS__fat_free(buffer);
        return BPB_SETTING_ERROR;
    }

    /*CY 1023*/

    FS__pDevInfo[Idx].pDevCacheInfo[Unit].CacheIndex = 0;
    fsStorageSectorCount = FS__FAT_aBPBUnit[Idx][Unit].TotSec16;
    if (!fsStorageSectorCount)
    {
        fsStorageSectorCount = FS__FAT_aBPBUnit[Idx][Unit].TotSec32;
    }

    err=_FS_CheckBPB(Idx,Unit);
    if(err<0)
        DEBUG_FS("Error! BPB check Fail: 0x%x\n",err);

    FS__fat_free(buffer);
    return err;
}
/*********************************************************************
*
*             _FS__fat_FindFreeCluster
*
  Description:
  FS internal function. Find the next free entry in the FAT.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pFATSector  - Returns the sector number of the free entry.
  pLastSector - Returns the sector number of the sector in pBuffer.
  pFATOffset  - Returns the offset of the free FAT entry within the
                sector pFATSector.
  LastClust   - Here it is used at hint for where to start
                in the FAT.
  pBuffer     - Pointer to a sector buffer.
  FSysType    - ==1 => FAT12
                ==0 => FAT16
                ==2 => FAT32
  FATSize     - Size of one FAT ind sectors.
  BytesPerSec - Number of bytes in each sector.

  Return value:
  >=0         - Number of the free cluster.
  <0          - An error has occured.
*/

__inline static FS_i32 _FS__fat_FindFreeCluster(int Idx, FS_u32 Unit, FS_i32 *pFATSector,
                                                       FS_i32 *pLastSector, FS_i32 *pFATOffset,
                                                       FS_i32 LastClust, unsigned char *pBuffer,
                                                       int FSysType, FS_u32 FATSize, FS_i32 BytesPerSec)
{
    FS_u32 totclst;
    FS_u32 rootdirsize;
    FS_i32 curclst;
    FS_i32 fatindex;
    int err;
    int scan;
    unsigned char fatentry;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    FS__FAT_BPB *pFS__FAT_aBPBUnit;

    //-----------------//
    pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    if (LastClust > 0)
    {
        curclst = LastClust + 1;  /* Start scan after the previous allocated sector */
    }
    else
    {
        curclst = 0;  /*  Start scan at the beginning of the media */
    }
    scan          =  0;
    *pFATSector   =  0;
    *pLastSector  = -1;
    fatentry      = 0xff;
    /* Calculate total number of data clusters on the media */
    totclst = (FS_u32)pFS__FAT_aBPBUnit->TotSec16;
    if (totclst == 0)
    {
        totclst = pFS__FAT_aBPBUnit->TotSec32;
    }
    rootdirsize = ((FS_u32)((FS_u32)pFS__FAT_aBPBUnit->RootEntCnt) * FS_FAT_DENTRY_SIZE) / BytesPerSec;
    totclst     = totclst - (pFS__FAT_aBPBUnit->RsvdSecCnt + pFS__FAT_aBPBUnit->NumFATs * FATSize + rootdirsize);
    totclst    /= pFS__FAT_aBPBUnit->SecPerClus;
    while (1)
    {
        if (curclst >= (FS_i32)totclst)
        {
            scan++;
            if (scan > 1)
            {
                global_diskInfo.avail_clusters=0;
                return -1;  /* End of clusters reached after 2nd scan */
            }
            if (LastClust <= 0)
            {
                break;  /* 1st scan started already at zero */
            }
            curclst   = 0;  /* Try again starting at the beginning of the FAT */
            fatentry  = 0xff;
        }
        if (fatentry == 0)
        {
            break;  /* Free entry found */
        }
        if (FSysType == 1)
        {
            fatindex = curclst + (curclst / 2);    /* FAT12 */
        }
        else if (FSysType == 2)
        {
            fatindex = curclst * 4;               /* FAT32 */
        }
        else
        {
            fatindex = curclst * 2;               /* FAT16 */
        }
        *pFATSector = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / BytesPerSec);
        *pFATOffset = fatindex % BytesPerSec;
        if (*pFATSector != *pLastSector)
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector, (void*)pBuffer);
            if (err < 0)
            {
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSize + *pFATSector, (void*)pBuffer);
                if (err < 0)
                {
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector, (void*)pBuffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector, (void*)pBuffer);
            if (err < 0)
            {
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FATSize + *pFATSector, (void*)pBuffer);
                if (err < 0)
                {
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector, (void*)pBuffer);
            }
        #endif
            *pLastSector = *pFATSector;
        }
        if (FSysType == 1)//FAT12
        {
            if (*pFATOffset == (BytesPerSec - 1))
            {
                a = pBuffer[*pFATOffset];
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector + 1, (void*)pBuffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSize + *pFATSector + 1, (void*)pBuffer);
                    if (err < 0)
                    {
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector + 1, (void*)pBuffer);
                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector + 1, (void*)pBuffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FATSize + *pFATSector + 1, (void*)pBuffer);
                    if (err < 0)
                    {
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, *pFATSector + 1, (void*)pBuffer);
                }
            #endif
                *pLastSector = *pFATSector + 1;
                b = pBuffer[0];
            }
            else
            {
                a = pBuffer[*pFATOffset];
                b = pBuffer[*pFATOffset + 1];
            }
            if (curclst & 1)
            {
                fatentry = ((a & 0xf0) >> 4 ) | b;
            }
            else
            {
                fatentry = a | (b & 0x0f);
            }
        }
        else if (FSysType == 2) //FAT32
        {
            a = pBuffer[*pFATOffset];
            b = pBuffer[*pFATOffset + 1];
            c = pBuffer[*pFATOffset + 2];
            d = pBuffer[*pFATOffset + 3];
            fatentry = a | b | c | d;
        }
        else //FAT16
        {
            a = pBuffer[*pFATOffset];
            b = pBuffer[*pFATOffset + 1];
            fatentry = a | b;
        }
        if (fatentry != 0)
        {
            curclst++;  /* Cluster is in use or defect, so try the next one */
        }
    }
    if (fatentry == 0)
    {
        return curclst;  /* Free cluster found */
    }
    return -1;
}


/*********************************************************************
*
*             _FS__fat_SetEOFMark
*
  Description:
  FS internal function. Set the EOF mark in the FAT for a cluster.
  The function does not write the FAT sector. An exception is FAT12,
  if the FAT entry is in two sectors.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  FATSector   - FAT sector, where the cluster is located.
  pLastSector - Pointer to an FS_i32, which contains the number of the
                sector in pBuffer.
  FATOffset   - Offset of the cluster in the FAT sector.
  Cluster     - Cluster number, where to set the EOF mark.
  pBuffer     - Pointer to a sector buffer.
  FSysType    - ==1 => FAT12
                ==0 => FAT16
                ==2 => FAT32
  FATSize     - Size of one FAT ind sectors.
  BytesPerSec - Number of bytes in each sector.

  Return value:
  >=0         - EOF mark set.
  <0          - An error has occured.
*/

__inline static int _FS__fat_SetEOFMark(int Idx, FS_u32 Unit, FS_i32 FATSector,
                                        FS_i32 *pLastSector, FS_i32 FATOffset,
                                        FS_i32 Cluster, unsigned char *pBuffer,
                                        int FSysType, FS_u32 FATSize, FS_i32 BytesPerSec)
{
    int err1;
//    int err2;
    int lexp;

    if (FSysType == 1) //FAT12
    {
        if (FATOffset == (BytesPerSec - 1))
        {
            /* Entry in 2 sectors (we have 2nd sector in buffer) */
            if (Cluster & 1)
            {
                pBuffer[0]  = (char)0xff;
            }
            else
            {
                pBuffer[0] |= (char)0x0f;
            }
        #if FS_RW_DIRECT
            err1 = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, (void*)pBuffer);
            lexp = (err1 < 0);
            if (lexp)
            {
                return -1;
            }
            err1 = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSector, (void*)pBuffer);
            if (err1 < 0)
            {
                err1 = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSize + FATSector, (void*)pBuffer);
                if (err1 < 0)
                {
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSector, (void*)pBuffer);
            }
        #else
            err1 = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, (void*)pBuffer);
            lexp = (err1 < 0);
            if (lexp)
            {
                return -1;
            }
            err1 = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, (void*)pBuffer);
            if (err1 < 0)
            {
                err1 = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FATSize + FATSector, (void*)pBuffer);
                if (err1 < 0)
                {
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, FATSector, (void*)pBuffer);
            }
        #endif
            *pLastSector = FATSector;
            if (Cluster & 1)
            {
                pBuffer[FATOffset] |= (char)0xf0;
            }
            else
            {
                pBuffer[FATOffset]  = (char)0xff;
            }
        }
        else
        {
            if (Cluster & 1)
            {
                pBuffer[FATOffset]   |= (char)0xf0;
                pBuffer[FATOffset+1]  = (char)0xff;
            }
            else
            {
                pBuffer[FATOffset]    = (char)0xff;
                pBuffer[FATOffset+1] |= (char)0x0f;
            }
        }
    }
#if (FS_FAT_NOFAT32==0)
    else if (FSysType == 2) //FAT32
    { /* FAT32 */
        pBuffer[FATOffset]      = (char)0xff;
        pBuffer[FATOffset + 1]  = (char)0xff;
        pBuffer[FATOffset + 2]  = (char)0xff;
        pBuffer[FATOffset + 3]  = (char)0x0f;
    }
#endif /* FS_FAT_NOFAT32==0 */
    else
    { /* FAT16 */
        pBuffer[FATOffset]      = (char)0xff;
        pBuffer[FATOffset + 1]  = (char)0xff;
    }
    return 0;
}


/*********************************************************************
*
*             _FS__fat_LinkCluster
*
  Description:
  FS internal function. Link the new cluster with the EOF mark to the
  cluster list.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pLastSector - Pointer to an FS_i32, which contains the number of the
                sector in pBuffer.
  Cluster     - Cluster number of the new cluster with the EOF mark.
  LastClust   - Number of cluster, to which the new allocated cluster
                is linked to.
  pBuffer     - Pointer to a sector buffer.
  FSysType    - ==1 => FAT12
                ==0 => FAT16
                ==2 => FAT32
  FATSize     - Size of one FAT ind sectors.
  BytesPerSec - Number of bytes in each sector.

  Return value:
  >=0         - Link has been made.
  <0          - An error has occured.
*/

__inline static int _FS__fat_LinkCluster(int Idx, FS_u32 Unit, FS_i32 *pLastSector, FS_i32 Cluster,
                                              FS_i32 LastClust, unsigned char *pBuffer, int FSysType,
                                              FS_u32 FATSize, FS_i32 BytesPerSec)
{   //Link Cluster 與 LastClust
    FS_i32 fatindex;
    FS_i32 fatoffs;
    FS_i32 fatsec;
    int lexp;
    int err;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

    /* Link old last cluster to this one */
    if (FSysType == 1)
    {
        fatindex = LastClust + (LastClust / 2); /* FAT12 */
    }
    else if (FSysType == 2)
    {
        fatindex = LastClust * 4;               /* FAT32 */
    }
    else
    {
        fatindex = LastClust * 2;               /* FAT16 */
    }
    fatsec = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / BytesPerSec);
    fatoffs = fatindex % BytesPerSec;
    if (fatsec != *pLastSector)
    {
        /*
           FAT entry, which has to be modified is not in the same FAT sector, which is
           currently in the buffer. So write it to the media now.
        */
    #if FS_RW_DIRECT
        err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, *pLastSector, (void*)pBuffer);
        lexp = (err < 0);
        if (lexp)
        {
            return -1;
        }
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        if (err < 0)
        {
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, FATSize + fatsec, (void*)pBuffer);
            if (err<0)
            {
                return -1;
            }
            /* Try to repair original FAT sector with contents of copy */
            FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        }
    #else
        err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, *pLastSector, (void*)pBuffer);
        lexp = (err < 0);
        if (lexp)
        {
            return -1;
        }
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        if (err < 0)
        {
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, FATSize + fatsec, (void*)pBuffer);
            if (err<0)
            {
                return -1;
            }
            /* Try to repair original FAT sector with contents of copy */
            FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        }
    #endif
        *pLastSector = fatsec;
    }
    a =  Cluster & 0xff;
    b = (Cluster / 0x100L) & 0xff;
    c = (Cluster / 0x10000L) & 0xff;
    d = (Cluster / 0x1000000L) & 0x0f;
    
    if (FSysType == 1)
    {
        if (fatoffs == (BytesPerSec - 1))
        {
            /* Entry in 2 sectors (we have 2nd sector in buffer) */
            if (LastClust & 1)
            {
                pBuffer[fatoffs]   &= (char)0x0f;
                pBuffer[fatoffs]   |= (char)((a << 4) & 0xf0);
            }
            else
            {
                pBuffer[fatoffs]    = (char)(a & 0xff);
            }
         #if FS_RW_DIRECT
            err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
         #else
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
         #endif
            lexp = (err < 0);
            if (lexp)
            {
                return -1;
            }
         #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)pBuffer);
         #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)pBuffer);
         #endif
            if (err < 0)
            {
                return -1;
            }
            *pLastSector = fatsec + 1;
            if (LastClust & 1)
            {
                pBuffer[0]  = (char)(((a >> 4) & 0x0f) | ((b << 4) & 0xf0));
            }
            else
            {
                pBuffer[0] &= (char)0xf0;
                pBuffer[0] |= (char)(b & 0x0f);
            }
         #if FS_RW_DIRECT
           err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)pBuffer);
         #else
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)pBuffer);
         #endif
            lexp = (err < 0);
            if (lexp)
            {
                return -1;
            }
        }
        else
        {
            if (LastClust & 1)
            {
                pBuffer[fatoffs]     &= (char)0x0f;
                pBuffer[fatoffs]     |= (char)((a << 4) & 0xf0);
                pBuffer[fatoffs + 1]  = (char)(((a >> 4) & 0x0f) | ((b << 4) & 0xf0));
            }
            else
            {
                pBuffer[fatoffs]      = (char)(a & 0xff);
                pBuffer[fatoffs + 1] &= (char)0xf0;
                pBuffer[fatoffs + 1] |= (char)(b & 0x0f);
            }
        #if FS_RW_DIRECT
           err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        #else
            err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
        #endif
            lexp = (err < 0);
            if (lexp)
            {
                return -1;
            }
        }
    }
    else if (FSysType == 2)
    { /* FAT32 */
        pBuffer[fatoffs]      = a;
        pBuffer[fatoffs + 1]  = b;
        pBuffer[fatoffs + 2]  = c;
        pBuffer[fatoffs + 3]  = d;
    #if FS_RW_DIRECT
        err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
    #else
        err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
    #endif
        lexp = (err < 0) ;
        if (lexp)
        {
            return -1;
        }
    }
    else
    { /* FAT16 */
        pBuffer[fatoffs]      = a;
        pBuffer[fatoffs + 1]  = b;
    #if FS_RW_DIRECT
        err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
    #else
        err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)pBuffer);
    #endif
        lexp = (err < 0);
        if (lexp)
        {
            return -1;
        }
    }
    return 0;
}


/*********************************************************************
*
*             Global functions section
*
**********************************************************************

  Functions in this section are used by FAT File System layer only

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
    extern unsigned char* FS_internal_mem;
    FS_X_OS_LockMem();
    if (Size <= FS_FAT_SEC_SIZE)
    {
        for (i = 0; i < FS_MEMBLOCK_NUM; i++)
        {
            if (_FS_memblock[i].status == 0)
            {
                _FS_memblock[i].status = 1;

                _FS_memblock[i].memory=(unsigned char*)(FS_internal_mem+ i*FS_FAT_SEC_SIZE);
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

int FS__fat_checkunit(int Idx, FS_u32 Unit)
{
    int err;
    int status;
    int lexp;
    int retry_count=0;
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];


    status = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);
    if (status < 0)
    {
        return GET_STATUS_ERROR;
    }
    if (status == FS_LBL_MEDIACHANGED || pFS__FAT_aBPBUnit->Signature != 0xaa55)
    {
        /* Mount new volume */
        for (retry_count=0;retry_count<3;retry_count++)
        {
            err = _FS_ReadBPB(Idx, Unit);
            if (err == 1)
                break;
            else
            {
                DEBUG_FS("FS_ReadBPB error: 0x%x\n",err);
                continue;
            }
        }

        fat_bpb_err=err;
        if (err==1)
        {
            return 1;
        }
        else
        {
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_SET_STATUS, 0, (void*)0);
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

int FS__fat_which_type(int Idx, FS_u32 Unit)
{
    FS_u32 coc;
    FS_u32 fatsize;
    FS_u32 totsec;
    FS_u32 datasec;
    FS_u32 bytespersec;
    FS_u32 dsize;
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

    bytespersec   = (FS_u32)pFS__FAT_aBPBUnit->BytesPerSec;
    if (bytespersec!=0)
        dsize         = ((FS_u32)((FS_u32)pFS__FAT_aBPBUnit->RootEntCnt) * FS_FAT_DENTRY_SIZE) / bytespersec;
    else
        return BPB_SETTING_ERROR;
    fatsize       = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    totsec = (FS_u32)pFS__FAT_aBPBUnit->TotSec16;
    if (totsec == 0)
    {
        totsec = pFS__FAT_aBPBUnit->TotSec32;
    }
    datasec = totsec - (pFS__FAT_aBPBUnit->RsvdSecCnt +
                        pFS__FAT_aBPBUnit->NumFATs * fatsize + dsize);
    if (pFS__FAT_aBPBUnit->SecPerClus!=0)
        coc     = datasec / pFS__FAT_aBPBUnit->SecPerClus;
    else
        return -1;
    if (coc < 4085)
    {
        return 1;  /* FAT12 */
    }
    else if (coc < 65525)
    {
        return 0;  /* FAT16 */
    }
    return 2;  /* FAT32 */
}


/*********************************************************************
*
*             FS__fat_FAT_find_eof
*
  Description:
  FS internal function. Find the next EOF mark in the FAT.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  StrtClst    - Starting cluster in FAT.
  pClstCnt    - If not zero, this is a pointer to an FS_u32, which
                is used to return the number of clusters found
                between StrtClst and the next EOF mark.

  Return value:
  >=0         - Cluster, which contains the EOF mark.
  <0          - An error has occured.
*/

FS_i32 FS__fat_FAT_find_eof(int Idx, FS_u32 Unit, FS_i32 StrtClst, FS_u32 *pClstCnt)
{
    FS_u32 clstcount;
    FS_u32 fatsize;
    FS_u32 maxclst;
    FS_i32 fatindex;
    FS_i32 fatsec;
    FS_i32 fatoffs;
    FS_i32 lastsec;
    FS_i32 curclst;
    FS_i32 bytespersec;
    FS_i32 eofclst;
    int fattype;
    int err;
    char *buffer;
    unsigned int *fat32;
    unsigned short *fat16;
    unsigned char a;
    unsigned char b;
#if (FS_FAT_NOFAT32==0)
    unsigned char c;
    unsigned char d;
#endif /* FS_FAT_NOFAT32==0 */
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    if (fattype == 1)
    {
        maxclst = 4085UL;       /* FAT12 */
    }
    else if (fattype == 2)
    {
        maxclst = 0x0ffffff0UL; /* FAT32 */
    }
    else
    {
        maxclst = 65525UL;      /* FAT16 */
    }
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec   = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;
    curclst       = StrtClst;
    lastsec       = -1;
    clstcount     = 0;
    while (clstcount < maxclst)
    {
        eofclst = curclst;
        clstcount++;
        if (fattype == 1)
        {
            fatindex = curclst + (curclst / 2);   /* FAT12 */
        }
        else if (fattype == 2)
        {
            fatindex = curclst * 4;               /* FAT32 */
        }
        else
        {
            fatindex = curclst * 2;               /* FAT16 */
        }
        // fatsec: The sector number which record FAT
        fatsec  = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;
        if (fatsec != lastsec)
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return -1;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        if (fattype == 1) //FAT12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a   = buffer[fatoffs];
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);

                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);

                }
            #endif
                lastsec = fatsec + 1;
                b       = buffer[0];
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
            }
            if (curclst & 1)
            {
                curclst = ((a & 0xf0) >> 4 ) + 16 * b;
            }
            else
            {
                curclst = a + 256 * (b & 0x0f);
            }
            curclst &= 0x0fffL;
            if (curclst >= 0x0ff8L)
            {
                /* EOF found */
                if (pClstCnt)
                {
                    *pClstCnt = clstcount;
                }
                FS__fat_free(buffer);
                return eofclst;
            }
        }
        else if (fattype == 2)
        {
            fat32=(unsigned int*)(buffer+fatoffs);
            //a         = buffer[fatoffs];
            //b         = buffer[fatoffs + 1];
            //c         = buffer[fatoffs + 2];
            //d         = buffer[fatoffs + 3];
            //curclst   = a + 0x100L * b + 0x10000L * c + 0x1000000L * d;
            curclst = *fat32;
            curclst  &= 0x0fffffffL;
            if (curclst >= (FS_i32)0x0ffffff8L)
            {
                /* EOF found */
                if (pClstCnt)
                {
                    *pClstCnt = clstcount;
                }
                FS__fat_free(buffer);
                return eofclst;
            }
        }
        else
        {
            fat16=(unsigned short*)(buffer+fatoffs);
            //a         = buffer[fatoffs];
            //b         = buffer[fatoffs + 1];
            //curclst   = a + 256 * b;
            curclst = *fat16;
            curclst  &= 0xffffL;
            if (curclst >= (FS_i32)0xfff8L)
            {
                /* EOF found */
                if (pClstCnt)
                {
                    *pClstCnt = clstcount;
                }
                FS__fat_free(buffer);
                return eofclst;
            }
        }

        if(curclst ==0 )
        {  //It is imposible value.
           if (pClstCnt)
           {
              *pClstCnt = clstcount;
           }
           FS__fat_free(buffer);
           return -1;
        }
    } /* while (clstcount<maxclst) */
    FS__fat_free(buffer);
    return -1;
}

/*
   Update the FSInfo structure
*/
FS_i32 FS__fat_Update_FSInfo()
{
#if (FS_FAT_NOFAT32==0)

    unsigned char *buffer;
    int fattype;
    int err;
    int Idx = gIdx;
    FS_u32 Unit = gUnit;
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

    buffer = (unsigned char*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    /* Update the FSInfo structure */
    if (fattype == 2) //only for FAT32
    {
        /* Modify FSInfo */
    #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, pFS__FAT_aBPBUnit->FSInfo, (void*)buffer);
    #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, pFS__FAT_aBPBUnit->FSInfo, (void*)buffer);
    #endif
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
        /* Check for FSInfo structure in buffer */
        if (buffer[0] == (char)0x52)
        {
            if (buffer[1] == (char)0x52)
            {
                if (buffer[2] == (char)0x61)
                {
                    if (buffer[3] == (char)0x41)
                    {
                        if (buffer[484] == (char)0x72)
                        {
                            if (buffer[485] == (char)0x72)
                            {
                                if (buffer[486] == (char)0x41)
                                {
                                    if (buffer[487] == (char)0x61)
                                    {
                                        if (buffer[508] == (char)0x00)
                                        {
                                            if (buffer[509] == (char)0x00)
                                            {
                                                if (buffer[510] == (char)0x55)
                                                {
                                                    if (buffer[511] == (char)0xaa)
                                                    {
                                                        /* Invalidate last known free cluster count */
                                                        buffer[488] = (char)0xff;
                                                        buffer[489] = (char)0xff;
                                                        buffer[490] = (char)0xff;
                                                        buffer[491] = (char)0xff;
                                                        /* Give hint for free cluster search */
                                                        buffer[492] = gCurclst & 0xff;
                                                        buffer[493] = (gCurclst / 0x100L) & 0xff;
                                                        buffer[494] = (gCurclst / 0x10000L) & 0xff;
                                                        buffer[495] = (gCurclst / 0x1000000L) & 0x0f;
                                                     #if FS_RW_DIRECT
                                                        err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, pFS__FAT_aBPBUnit->FSInfo, (void*)buffer);
                                                     #else
                                                        err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, pFS__FAT_aBPBUnit->FSInfo, (void*)buffer);
                                                     #endif
                                                        if (err < 0)
                                                        {
                                                            FS__fat_free(buffer);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } /* buffer contains FSInfo structure */
    }
    FS__fat_free(buffer);
    return 0;

#endif /* FS_FAT_NOFAT32==0 */
}

/*********************************************************************
*
*             FS__fat_FAT_allocOne
*
  Description:
  FS internal function. Allocate a new cluster in the FAT and link it
  to LastClust. Assign an EOF mark to the new allocated cluster.
  The function has grown a lot, since it supports all FAT types (FAT12,
  FAT16 & FAT32). There is also room for performance improvement, when
  makeing the new FAT entry and the old entry is within the same FAT
  sector.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  LastClust   - Number of cluster, to which the new allocated cluster
                is linked to. If this is negative, the new cluster is
                not linked to anything and only the EOF mark is set.
  LinkFlag    - 1: FAT table link to LastClust
                0: FAT table not link to LastClust. Create a new linkage.

  Return value:
  >=0         - Number of new allocated cluster, which contains the
                EOF mark.
  <0          - An error has occured.
*********************************************************************/

FS_i32 FS__fat_FAT_allocOne(int Idx, FS_u32 Unit, FS_i32 LastClust,int LinkFlag)
{   //Lucian: 一次僅allocat one cluster
    FS_u32 fatsize;
    FS_i32 fatoffs;
    FS_i32 bytespersec;
    FS_i32 curclst;
    FS_i32 fatsec;
    FS_i32 lastsec;
    unsigned char *buffer;
    int fattype;
    int err;
    int lexp;
    u8 err2;
    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];

    OSSemPend(FSSecIncSemEvt, OS_IPC_WAIT_FOREVER, &err2);

    buffer = (unsigned char*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    //--Read FAT table size (sector unit)
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec   = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;
    
    /* Find a free cluster in the FAT */
    curclst       = _FS__fat_FindFreeCluster(Idx, Unit, &fatsec, &lastsec, &fatoffs, LastClust, buffer, fattype, fatsize, bytespersec);
    if (curclst < 0)
    {
        FS__fat_free(buffer);   /* No free cluster found. */
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    /* Make an EOF entry for the new cluster */
    err = _FS__fat_SetEOFMark(Idx, Unit, fatsec, &lastsec, fatoffs, curclst, buffer, fattype, fatsize, bytespersec);
    if (err < 0)
    {
        FS__fat_free(buffer);
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    /* Link the new cluster to the cluster list */
    if (LinkFlag)
    {
        err = _FS__fat_LinkCluster(Idx, Unit, &lastsec, curclst, LastClust, buffer, fattype, fatsize, bytespersec);
        if (err < 0)
        {
            FS__fat_free(buffer);
            OSSemPost(FSSecIncSemEvt);
            return -1;
        }
    }
    else
    {
    #if FS_RW_DIRECT
        err  = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, lastsec, (void*)buffer);
    #else
        err  = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, lastsec, (void*)buffer);
    #endif
        lexp = (err < 0);
        if (lexp)
        {
            FS__fat_free(buffer);
            OSSemPost(FSSecIncSemEvt);
            return -1;
        }
    }

    gCurclst = curclst;
    gUnit = Unit;
    gIdx = Idx;

#if FS_NEW_VERSION
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount -= 1;

    // Update the storage space counter.
    global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;
#endif
    FS__fat_free(buffer);
    OSSemPost(FSSecIncSemEvt);
    return curclst;
}

/*********************************************************************
*
*             FS__fat_FAT_allocMulti
*
  Description:
  FS internal function. Allocate a new cluster in the FAT and link it
  to LastClust. Assign an EOF mark to the new allocated cluster.
  The function has grown a lot, since it supports all FAT types (FAT12,
  FAT16 & FAT32). There is also room for performance improvement, when
  makeing the new FAT entry and the old entry is within the same FAT
  sector.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  LastClust   - Number of cluster, to which the new allocated cluster
                is linked to. If this is negative, the new cluster is
                not linked to anything and only the EOF mark is set.
  AllocFatNum - allocat Fat cluster.

  clstBuf     - save FAT link.

  Return value:
  >=0         - Number of new allocated cluster, which contains the
                EOF mark.
  <0          - An error has occured.
*********************************************************************/
FS_i32 FS__fat_FAT_allocMulti(int Idx, FS_u32 Unit, FS_i32 LastClust, int AllocFatNum,FS_u32 *pclstBuf)
{   //Lucian: allocat and link multi cluster
    FS_u32 fatsize;
    FS_i32 fatoffs,Lastfatoffs;
    FS_i32 bytespersec;
    FS_i32 curclst;
    FS_i32 fatsec;
    FS_i32 Lastfatsec,lastsec;
    unsigned char *buffer;
    int fattype;
    int err;
    int lexp;
    u8 err2;
    FS__FAT_BPB *pFS__FAT_aBPBUnit;
    FS_u32 totclst;
    FS_u32 rootdirsize;
    FS_i32 fatindex;
    int scan;
    int i;
    unsigned char fatentry;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    //-----------//

    OSSemPend(FSSecIncSemEvt, OS_IPC_WAIT_FOREVER, &err2);

    pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    buffer = (unsigned char*)FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    //--Read FAT table size (sector unit)
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec   = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;

    //=========================================================//
    lastsec=-1;

    if(LastClust<0)
    {
        FS__fat_free(buffer);
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    
    if (fattype == 1)
    {
        fatindex = LastClust + (LastClust / 2);    /* FAT12 */
    }
    else if (fattype == 2)
    {
        fatindex = LastClust * 4;               /* FAT32 */
    }
    else
    {
        fatindex = LastClust * 2;               /* FAT16 */
    }
    Lastfatsec = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / bytespersec);
    Lastfatoffs = fatindex % bytespersec;    
    if(Lastfatsec != lastsec)
    {
    #if FS_RW_DIRECT
        err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        if (err < 0)
        {
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + Lastfatsec, buffer);
            if (err < 0)
            {
                FS__fat_free(buffer);
                OSSemPost(FSSecIncSemEvt);
                return -1;
            }
            /* Try to repair original FAT sector with contents of copy */
            FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        }
    #else
        err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        if (err < 0)
        {
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + Lastfatsec, buffer);
            if (err < 0)
            {
                FS__fat_free(buffer);
                OSSemPost(FSSecIncSemEvt);
                return -1;
            }
            /* Try to repair original FAT sector with contents of copy */
            FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        }
    #endif
        lastsec=Lastfatsec;
    }

    //---//
    fatsec        =  0;
    scan          =  0;
    /*---- Calculate total number of data clusters on the media ----*/
    totclst = (FS_u32)pFS__FAT_aBPBUnit->TotSec16;
    if (totclst == 0)
    {
        totclst = pFS__FAT_aBPBUnit->TotSec32;
    }
    rootdirsize = ((FS_u32)((FS_u32)pFS__FAT_aBPBUnit->RootEntCnt) * FS_FAT_DENTRY_SIZE) / bytespersec;
    totclst     = totclst - (pFS__FAT_aBPBUnit->RsvdSecCnt + pFS__FAT_aBPBUnit->NumFATs * fatsize + rootdirsize);
    totclst    /= pFS__FAT_aBPBUnit->SecPerClus;

    for(i=0;i<AllocFatNum;i++)
    {
        /*------ Find a free cluster in the FAT ------*/
        if (LastClust > 0)
        {
            curclst = LastClust + 1;  /* Start scan after the previous allocated sector */
        }
        else
        {
            curclst = 0;  /*  Start scan at the beginning of the media */
        }
        fatentry      = 0xff;
        while (1)
        {
            if (curclst >= (FS_i32)totclst)
            {
                DEBUG_FS("\nFAT SCAN=%d\n",scan);
                scan++;
                if (scan > 1)
                {
                    global_diskInfo.avail_clusters=0;
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;  /* End of clusters reached after 2nd scan */
                }
                if (LastClust <= 0)
                {
                    break;  /* 1st scan started already at zero */
                }
                curclst   = 0;  /* Try again starting at the beginning of the FAT */
                fatentry  = 0xff;
            }
            if (fatentry == 0)
            {
                break;  /* Free entry found */
            }
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
            fatsec = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / bytespersec);
            fatoffs = fatindex % bytespersec;
            if (fatsec != lastsec)
            {
            #if FS_RW_DIRECT
                err=FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
            #else
                err=FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, buffer);
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
                lastsec = fatsec;
            }
            if (fattype == 1)//FAT12
            {
                if (fatoffs == (bytespersec - 1))
                {
                    a = buffer[fatoffs];
                #if FS_RW_DIRECT
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, buffer);
                #else
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, buffer);
                #endif
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        OSSemPost(FSSecIncSemEvt);
                        return -1;
                    }
                    lastsec = fatsec + 1;
                    b = buffer[0];
                }
                else
                {
                    a = buffer[fatoffs];
                    b = buffer[fatoffs + 1];
                }
                if (curclst & 1)
                {
                    fatentry = ((a & 0xf0) >> 4 ) | b;
                }
                else
                {
                    fatentry = a | (b & 0x0f);
                }
            }
            else if (fattype == 2) //FAT32
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
                c = buffer[fatoffs + 2];
                d = buffer[fatoffs + 3];
                fatentry = a | b | c | d;
            }
            else //FAT16
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
                fatentry = a | b;
            }
            if (fatentry != 0)
            {
                curclst++;  /* Cluster is in use or defect, so try the next one */
            }
        }
        *pclstBuf =curclst;
        pclstBuf ++;
        
        //---------- Set EOF Mark --------//
        if (fattype == 1)//FAT12
        {
            if (fatoffs == (bytespersec - 1))
            {
                if (LastClust & 1)
                {
                    buffer[fatoffs]   |= (char)0xf0;
                }
                else
                {
                    buffer[fatoffs]    = (char)0xff;
                }
            #if FS_RW_DIRECT
                err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, buffer);
            #else
                err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, buffer);
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
                lastsec = fatsec + 1;
                
                if (LastClust & 1)
                {
                    buffer[0]  = (char)0xff;
                }
                else
                {
                    buffer[0] |= (char)0x0f;
                }
            }
            else
            {
                if (LastClust & 1)
                {
                    buffer[fatoffs]     |= (char)0xf0;
                    buffer[fatoffs + 1]  = (char)0xff;
                }
                else
                {
                    buffer[fatoffs]      = (char)0xff;
                    buffer[fatoffs + 1] |= (char)0x0f;
                }
            }
        }
        else if (fattype == 2) //FAT32
        {
            buffer[fatoffs]    =0xff;
            buffer[fatoffs + 1]=0xff;
            buffer[fatoffs + 2]=0xff;
            buffer[fatoffs + 3]=0x0f;
        }
        else //FAT16
        {
            buffer[fatoffs]    =0xff;
            buffer[fatoffs + 1]=0xff;
        }

        /*----------- Link FAT ----------*/
        a =  curclst & 0xff;
        b = (curclst / 0x100L) & 0xff;
        c = (curclst / 0x10000L) & 0xff;
        d = (curclst / 0x1000000L) & 0x0f;
        
        if (fattype == 1)
        {
            fatindex = LastClust + (LastClust / 2);    /* FAT12 */
        }
        else if (fattype == 2)
        {
            fatindex = LastClust * 4;               /* FAT32 */
        }
        else
        {
            fatindex = LastClust * 2;               /* FAT16 */
        }
        Lastfatsec = pFS__FAT_aBPBUnit->RsvdSecCnt + (fatindex / bytespersec);
        Lastfatoffs = fatindex % bytespersec;
        if(Lastfatsec != lastsec)
        {
        #if FS_RW_DIRECT
           err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
        #else
            err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
        #endif
            if (err < 0)
            {
                FS__fat_free(buffer);
                OSSemPost(FSSecIncSemEvt);
                return -1;
            }
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, buffer);
        #endif
            if (err < 0)
            {
                FS__fat_free(buffer);
                OSSemPost(FSSecIncSemEvt);
                return -1;
            }
            lastsec=Lastfatsec;
        }
        if (fattype == 1)//FAT12
        {
            if (Lastfatoffs == (bytespersec - 1))
            {
                if (LastClust & 1)
                {
                    buffer[Lastfatoffs]   &= (char)0x0f;
                    buffer[Lastfatoffs]   |= (char)((a << 4) & 0xf0);
                }
                else
                {
                    buffer[Lastfatoffs]    = (char)(a & 0xff);
                }
            #if FS_RW_DIRECT
                err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, (void*)buffer);
            #else
                err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec, (void*)buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec + 1, (void*)buffer);
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, Lastfatsec + 1, (void*)buffer);
            #endif
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    OSSemPost(FSSecIncSemEvt);
                    return -1;
                }
                lastsec = Lastfatsec + 1;
                if (LastClust & 1)
                {
                    buffer[0]  = (char)(((a >> 4) & 0x0f) | ((b << 4) & 0xf0));
                }
                else
                {
                    buffer[0] &= (char)0xf0;
                    buffer[0] |= (char)(b & 0x0f);
                }
            }
            else
            {
                if (LastClust & 1)
                {
                    buffer[Lastfatoffs]     &= (char)0x0f;
                    buffer[Lastfatoffs]     |= (char)((a << 4) & 0xf0);
                    buffer[Lastfatoffs + 1]  = (char)(((a >> 4) & 0x0f) | ((b << 4) & 0xf0));
                }
                else
                {
                    buffer[Lastfatoffs]      = (char)(a & 0xff);
                    buffer[Lastfatoffs + 1] &= (char)0xf0;
                    buffer[Lastfatoffs + 1] |= (char)(b & 0x0f);
                }
            }
        }
        else if (fattype == 2) //FAT32
        {
            buffer[Lastfatoffs]=a;
            buffer[Lastfatoffs + 1]=b;
            buffer[Lastfatoffs + 2]=c;
            buffer[Lastfatoffs + 3]=d;
        }
        else //FAT16
        {
            buffer[Lastfatoffs]=a;
            buffer[Lastfatoffs + 1]=b;
        }

        LastClust=curclst;
    }
#if FS_RW_DIRECT
    err = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
#else
    err = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, lastsec, buffer);
#endif
    if (err < 0)
    {
        FS__fat_free(buffer);
        OSSemPost(FSSecIncSemEvt);
        return -1;
    }
    //--------------------------------------------//
    gCurclst = curclst;
    gUnit = Unit;
    gIdx = Idx;
#if FS_NEW_VERSION
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount -= AllocFatNum;

    // Update the storage space counter.
    global_diskInfo.avail_clusters = FS__pDevInfo[Idx].FSInfo.FreeClusterCount;
#endif
    FS__fat_free(buffer);
    OSSemPost(FSSecIncSemEvt);
    return curclst;
}
/*********************************************************************
*
*             FS__fat_FindLastFreeCluster
*
  Description:
     於FAT Table 找尋空置的Cluster,

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  LastClust   - Number of cluster, to which the new allocated cluster
                is linked to. If this is negative, the new cluster is
                not linked to anything and only the EOF mark is set.

  Return value:
  >=0         - Number of free cluster.
  <0          - An error has occured.
*********************************************************************/

int FS__fat_FindLastFreeCluster(int Idx, FS_u32 Unit)
{   
    FS_u32 fatsize;
    FS_i32 fatoffs;
    FS_i32 bytespersec;
    FS_i32 curclst;
    FS_i32 fatsec;
    FS_i32 lastsec;
    unsigned char *buffer;
    int fattype;
    int err;
    FS__FAT_BPB *pFS__FAT_aBPBUnit;
    FS_u32 totclst;
    FS_u32 rootdirsize;
    int i;
    int totalfatsec;
    int RsvdSecCnt;
    unsigned int *pp;
    unsigned short *pp_short;
    int numOfSector;
    

    err=FS_LB_Cache_Clean(Idx, Unit); //Clean cache first,write back to SD card.
    if(err<0)
    {
      DEBUG_FS("FS_LB_Cache_Clean is Fail\n");
    }
    pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    
    buffer = (unsigned char*)PKBuf;
    
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;
    
    if(fattype == 1)
      return -1;  //FAT12 not support  


    //--Read FAT table size (sector unit)
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec   = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;
    
    /*------ Find a free cluster in the FAT -------*/
    lastsec=-1;
    fatsec        =  0;
    totclst = (FS_u32)pFS__FAT_aBPBUnit->TotSec16;
    if (totclst == 0)
    {
        totclst = pFS__FAT_aBPBUnit->TotSec32;
    }
    rootdirsize = ((FS_u32)((FS_u32)pFS__FAT_aBPBUnit->RootEntCnt) * FS_FAT_DENTRY_SIZE) / bytespersec;
    totclst     = totclst - (pFS__FAT_aBPBUnit->RsvdSecCnt + pFS__FAT_aBPBUnit->NumFATs * fatsize + rootdirsize);
    totclst    /= pFS__FAT_aBPBUnit->SecPerClus;

    curclst  = 0;  /*  Start scan at the beginning of the media */
    RsvdSecCnt=pFS__FAT_aBPBUnit->RsvdSecCnt;
    
    //------------------------------------------------//
    //for optimize: 確定 one sector =512 bytes.
    if (fattype == 2) //FAT32
       totalfatsec=totclst*4/512;
    else
       totalfatsec=totclst*2/512;
    
    fatsec=RsvdSecCnt;
    while(totalfatsec>0)
    {
        if(totalfatsec>256)
           numOfSector=256;
        else
           numOfSector=totalfatsec;
        
        err = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver,Unit,fatsec,numOfSector,(void *)buffer);
        if (err < 0)
        {
            return -1;
        }
        fatsec +=numOfSector;
        totalfatsec -=numOfSector;
    
        if (fattype == 2) //FAT32
        {
            pp = (unsigned int *)buffer;
            while(pp< (unsigned int *)(buffer + 512*numOfSector))
            {
                if( (*pp & 0x0fffffff) ==  0)
                {
                   return curclst;
                }
                curclst ++;
                pp ++;
            }
        }
        else
        {
            pp_short= (unsigned short *)buffer; 
            while(pp_short<(unsigned short *)(buffer + 512*numOfSector))
            {
                if(*pp_short ==  0)
                {
                   return curclst;
                }
                curclst ++;
                pp_short ++;
            }
        }
        
    }
    //----------------------------------------------//
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

FS_i32 FS__fat_diskclust(int Idx, FS_u32 Unit, FS_i32 StrtClst, FS_i32 ClstNum)
{
    FS_u32 fatsize;
    FS_i32 fatindex;
    FS_i32 fatsec;
    FS_i32 fatoffs;
    FS_i32 lastsec;
    FS_i32 curclst;
    FS_i32 todo;
    FS_i32 bytespersec;
	
    int err;
    int fattype;
    char *buffer;
    int RsvdSecCnt;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;


    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return 0;
    }
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;
    todo        = ClstNum;
    curclst     = StrtClst;
    lastsec     = -1;
    RsvdSecCnt=pFS__FAT_aBPBUnit->RsvdSecCnt;
    while (todo)
    {
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
        fatsec  = RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;
        
        if (fatsec != lastsec)
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
					DEBUG_FS("FS__lb_read_Direct is Fail\n");
                    return 0;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
					DEBUG_FS("FS__lb_read is Fail\n");
                    return 0;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        if (fattype == 1) //FAT12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
						DEBUG_FS("FS__lb_read_Direct is Fail\n");
                        return 0;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
						DEBUG_FS("FS__lb_read is Fail\n");
                        return 0;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #endif
                lastsec = fatsec + 1;
                b = buffer[0];
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
            }
            if (curclst & 1)
            {
                curclst = ((a & 0xf0) >> 4) + 16 * b;
            }
            else
            {
                curclst = a + 256 * (b & 0x0f);
            }
            curclst &= 0x0fffL;
            if (curclst >= 0x0ff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_diskclust Fail:%d,%d,%d\n",todo,StrtClst,ClstNum);
                return 0;
				
            }
        }
        else if (fattype == 2) //FAT32
        {
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3];
            curclst = a + 0x100L * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;
            if (curclst >= (FS_i32)0x0ffffff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_diskclust Fail:%d,%d,%d\n",todo,StrtClst,ClstNum);
				return 0;
            }
        }
        else //Fat16
        {
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            curclst  = a + 256 * b;
            curclst &= 0xffffL;
            if (curclst >= (FS_i32)0xfff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_diskclust Fail:%d,%d,%d\n",todo,StrtClst,ClstNum);
				return 0;
            }
        }
        todo--;
    }
    FS__fat_free(buffer);
    return curclst;
}

/*********************************************************************
*
*             FS__fat_FindClustList
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
  ClustList   - Array of cluster list.

  Return value:
  > 0         - Number of cluster found after ClstNum steps.
  ==0         - An error has occured.
***********************************************************************/

FS_i32 FS__fat_FindClustList(int Idx, FS_u32 Unit, FS_i32 StrtClst, FS_i32 ClstNum,unsigned int *ClustList)
{
    FS_u32 fatsize;
    FS_i32 fatindex;
    FS_i32 fatsec;
    FS_i32 fatoffs;
    FS_i32 lastsec;
    FS_i32 curclst;
    FS_i32 todo;
    FS_i32 bytespersec;
    int err;
    int fattype;
    char *buffer;
    int RsvdSecCnt;
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;


    FS__FAT_BPB *pFS__FAT_aBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return 0;
    }
    fatsize = pFS__FAT_aBPBUnit->FATSz16;
    if (fatsize == 0)
    {
        fatsize = pFS__FAT_aBPBUnit->FATSz32;
    }
    bytespersec = (FS_i32)pFS__FAT_aBPBUnit->BytesPerSec;
    todo        = ClstNum;
    curclst     = StrtClst;
    lastsec     = -1;
    RsvdSecCnt=pFS__FAT_aBPBUnit->RsvdSecCnt;
    while (todo)
    {
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
        fatsec  = RsvdSecCnt + (fatindex / bytespersec);
        fatoffs = fatindex % bytespersec;
        
        if (fatsec != lastsec)
        {
        #if FS_RW_DIRECT
            err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return 0;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #else
            err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            if (err < 0)
            {
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                if (err < 0)
                {
                    FS__fat_free(buffer);
                    return 0;
                }
                /* Try to repair original FAT sector with contents of copy */
                FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
            }
        #endif
            lastsec = fatsec;
        }
        if (fattype == 1) //FAT12
        {
            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
            #if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return 0;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return 0;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
            #endif
                lastsec = fatsec + 1;
                b = buffer[0];
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
            }
            if (curclst & 1)
            {
                curclst = ((a & 0xf0) >> 4) + 16 * b;
            }
            else
            {
                curclst = a + 256 * (b & 0x0f);
            }
            curclst &= 0x0fffL;
            *ClustList=curclst;
            if (curclst >= 0x0ff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_FindClustList: Fail\n");
                return 0;
            }
        }
        else if (fattype == 2) //FAT32
        {
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            c = buffer[fatoffs + 2];
            d = buffer[fatoffs + 3];
            curclst = a + 0x100L * b + 0x10000L * c + 0x1000000L * d;
            curclst &= 0x0fffffffL;
            *ClustList=curclst;
            if (curclst >= (FS_i32)0x0ffffff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_FindClustList: Fail\n");
                return 0;
            }
        }
        else //Fat16
        {
            a = buffer[fatoffs];
            b = buffer[fatoffs + 1];
            curclst  = a + 256 * b;
            curclst &= 0xffffL;
            *ClustList=curclst;
            if (curclst >= (FS_i32)0xfff8L)
            {
                FS__fat_free(buffer);
                DEBUG_FS("FS__fat_FindClustList: Fail\n");
                return 0;
            }
        }
        ClustList ++;
        todo--;
    }
    FS__fat_free(buffer);
    return curclst;
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
        FS__fat_fopen,        /* open  */
        FS__fat_fclose,       /* close */
        FS__fat_fread,        /* read  */
        FS__fat_fwrite,       /* write */
        0,                    /* tell  */
        0,                    /* seek  */
        FS__fat_ioctl,        /* ioctl */
#if FS_POSIX_DIR_SUPPORT
        FS__fat_opendir,      /* opendir   */
        FS__fat_closedir,     /* closedir  */
        FS__fat_readdir,      /* readdir   */
        0,                    /* rewinddir */
        FS__fat_MkRmDir,      /* mkdir     */
        FS__fat_MkRmDir,      /* rmdir     */
#endif  /* FS_POSIX_DIR_SUPPORT */
    };

