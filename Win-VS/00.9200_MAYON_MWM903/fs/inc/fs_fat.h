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
File        : fs_fat.h
Purpose     : FAT File System Layer header
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_FAT_H_
#define _FS_FAT_H_

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

#define FS_FAT_ATTR_READ_ONLY   0x01
#define FS_FAT_ATTR_HIDDEN      0x02
#define FS_FAT_ATTR_SYSTEM      0x04
#define FS_FAT_VOLUME_ID        0x08
#define FS_FAT_ATTR_DIRECTORY   0x10
#define FS_FAT_ATTR_ARCHIVE     0x20

#define FS_FAT_DENTRY_SIZE      0x20


/*********************************************************************
*
*             Global data types
*
**********************************************************************
*/

/* BIOS parameter block (FAT12/FAT16) */
typedef struct
{
    u16 BytesPerSec;    // 0x0B: _512_, 1024, 2048, 4096
    u8 SecPerClus;     // 0x0D: sec in allocation unit
    u16 RsvdSecCnt;     // 0x0E: 1 for FAT12 & FAT16, cytsai: 32 for FAT32
    u8 NumFATs;        // 0x10: 2
    u16 RootEntCnt;     // 0x11: number of root dir entries
    u16 TotSec16;       // 0x13: RSVD + FAT + ROOT + FATA (<64k)
    u8 MediaDesc;      // 0x15:
    u16 FATSz16;        // 0x16: number of FAT sectors
    // FAT32 Section
    u32 TotSec32;       // 0x20: RSVD + FAT + ROOT + FATA (>=64k)
    u32 FATSz32;        // 0x24: number of FAT sectors
    u16 ExtFlags;       // 0x28: mirroring info
    u32 RootClus;       // 0x2C: root dir clus for FAT32
    u16 FSInfo;         // 0x30: position of FSInfo structure
    // Self Section
    u32 SizeOfCluster;	// BytesPerSec * SecPerClus
    u8 BitNumOfBPS;
    u8 BitNumOfSPC;
    u8 BitNumOfSOC;
    u16 BitRevrOfBPS;
    u32 LimitsOfCluster;
    u32 FatEndSec;      // End sector of FAT table
    u32 RootDirSec;		// RSVD+ FAT
    u32 Dsize;          // ROOT
    u16 Signature;      // 0xAA55 Signature
    s8 FATType;        // (VCC) 1: FAT12 0: FAT16, 2:FAT32
}
FS__FAT_BPB;

/* FAT directory entry */
typedef struct
{
    u8 data[32];
}
FS__fat_dentry_type;

typedef struct
{
    u8 data[32];
}
FS_FAT_ENTRY;


/*********************************************************************
*
*             Externals
*
**********************************************************************
*/

extern FS__FAT_BPB FS__FAT_aBPBUnit[FS_MAXDEV][FS_FAT_MAXUNIT];


/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*             fat_misc
*/

void FS__fat_block_init(void);
char *FS__fat_malloc(unsigned int Size);
void FS__fat_free(void *pBuffer);
int FS__fat_diskclust(int Idx, u32 Unit, u32 StrtClst, s32 ClstNum, u32 *CurClust);
int FS__fat_FindClustList(int Idx, u32 Unit, s32 StrtClst, s32 ClstNum, u32 *ClustList, u32 *pCurrClust);

int FS__fat_FAT_allocOne(int Idx, u32 Unit, s32 LastClust,int LinkFlag);
int FS__fat_FAT_allocMulti(int Idx, u32 Unit, s32 LastClust, int AllocFatNum, u32 *pclstBuf);

int FS__fat_FindLastFreeCluster(int Idx, u32 Unit);

int FS__fat_FAT_find_eof(int Idx, u32 Unit, s32 StrtClst, u32 *pClstCnt, u32 *pClustNum);
int FS__fat_which_type(int Idx, u32 Unit);
int FS__fat_checkunit(int Idx, u32 Unit);



/*********************************************************************
*
*             fat_in
*/

int FS__fat_fread(FS_FILE *pFile, void *pData, u32 dataSize, u32 *pReadSize);


/*********************************************************************
*
*             fat_out
*/

int FS__fat_fwrite(FS_FILE *pFile, const void *pData, u32 dataSize, u32 *pWriteSize);
int FS__fat_fclose(FS_FILE *pFile);



/*********************************************************************
*
*             fat_open
*/

int FS__fat_fopen(const char *pFileName, const char *pMode, FS_FILE *pFile);
int FS__fat_dir_size(int Idx, u32 Unit, u32 DirStart, u32 *pDirSize);
int FS__fat_dir_realsec(int Idx, u32 Unit, u32 DirStart, u32 DirOffset, u32 *pDirSec);
void FS__fat_make_realname(char *pEntryName, const char *pOrgName);
int FS__fat_find_dir(int Idx, u32 Unit, char *pDirName, u32 DirStart, u32 DirSize, u32 *pDirSec);
int FS__fat_findpath(int Idx, const char *pFullName, FARCHARPTR *pFileName, u32 *pUnit, u32 *pDirStart, u32 *pDirSize);
int FS__fat_DeleteFileOrDir(int Idx, u32 Unit,  const char *pName, u32 DirStart, u32 DirSize, u16 FileEntrySect, char RmFile);



/*********************************************************************
*
*             fat_ioctl
*/

int FS__fat_ioctl(int Idx, u32 Unit, s32 Cmd, s32 Aux, void *pBuffer);



/*********************************************************************
*
*             fat_dir
*/

#if FS_POSIX_DIR_SUPPORT
int FS__fat_opendir(const char *pDirName, FS_DIR *pDir);
int FS__fat_closedir(FS_DIR *pDir);
int FS__fat_readdir(FS_DIR *pDir, FS_DIRENT *pEnt);
int FS__fat_MkRmDir(const char *pDirName, int Idx, char MkDir);
#endif /* FS_POSIX_DIR_SUPPORT */


#endif  /* _FS_FAT_H_ */


