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
File        : fs_int.h
Purpose     : Internals used accross different layers of the file system
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_INT_H_
#define _FS_INT_H_

/*********************************************************************
*
*             Global data types
*
**********************************************************************
*/

#ifndef FS_USE_LB_READCACHE
#define FS_USE_LB_READCACHE 0
#endif

// Prevent the over loading in Dir entry Scan
#define FS_FAT_DIRCLUSTER_MAX 0x1000

#define FS_LB_MULTIPLE_BLCOK_NUMBER 128

#if FS_USE_LB_READCACHE
#ifndef FS_LB_BLOCKSIZE
#define FS_LB_BLOCKSIZE 0x200
#endif

typedef struct
{
    u32 BlockId;
    u32 Popularity;
    char aBlockData[FS_LB_BLOCKSIZE];
    u8 Dirty;
} FS__CACHE_BUFFER;

typedef struct
{
    const int MaxCacheNum;
    int CacheIndex;
    FS__CACHE_BUFFER *const pCache;
} FS__LB_CACHE;
#endif  /* FS_USE_LB_READCACHE */

typedef struct
{
    u32 FreeClusterCount;
    u32 NextFreeCluster;
} FS_FATFSINFO;

typedef struct
{
    const char *const devname;
    const FS__fsl_type *const fs_ptr;
    const FS__device_type *const devdriver;
#if FS_USE_LB_READCACHE
    FS__LB_CACHE *const pDevCacheInfo;
#endif /* FS_USE_LB_READCACHE */
    FS_FATFSINFO FSInfo;
    u32 TagOfFirstFreeClust;
}
FS__devinfo_type;

typedef struct _FS_MemoryAllocStruct
{
	u8 *Address;
	u32 Length;
	struct _FS_MemoryAllocStruct *Next;
	struct _FS_MemoryAllocStruct *Prev;
} FS_MemoryAllocStruct;

/*********************************************************************
*
*             Externals
*
**********************************************************************
*/

/* fs_info.c */
extern FS__devinfo_type *FS__pDevInfo;
extern const unsigned int FS__maxdev;
extern const unsigned int FS__fat_maxunit;

#endif  /* _FS_INT_H_ */

