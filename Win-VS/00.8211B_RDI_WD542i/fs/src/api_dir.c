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
File        : api_dir.c
Purpose     : POSIX 1003.1 like directory support
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
#include "fs_os.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "api_int.h"
#include ".\dcf\dcf.h"




#if FS_POSIX_DIR_SUPPORT

/*********************************************************************
*
*             Local variables
*
**********************************************************************
*/

static const unsigned int _FS_dir_maxopen = FS_DIR_MAXOPEN;
static FS_DIR             _FS_dirhandle[FS_DIR_MAXOPEN];

extern unsigned int FS__fat_readwholedir(FS_DIR *pDir,struct FS_DIRENT *dst_DirEnt,
                                               unsigned char* buffer,unsigned int DirEntMax,
                                               DEF_FILEREPAIR_INFO *pdcfBadFileInfo,
                                               unsigned char IsUpdateEntrySect,
                                               int DoBadFile);

extern  int FS__fat_ScanWholedir( FS_DIR *pDir,
                                                 struct FS_DIRENT *dst_DirEnt,
                                                 unsigned char* buffer, 
                                                 unsigned int DirEntMax,
                                                 unsigned int *pOldestEntry,
                                                 int DoBadFile
                                                );

unsigned int FS_ReadWholeDir(FS_DIR *pDir,struct FS_DIRENT *dst_DirEnt,
                                     unsigned char* buffer, unsigned int DirEntMax,
                                     DEF_FILEREPAIR_INFO *pdcfBadFileInfo,
                                     unsigned char IsUpdateEntrySect,
                                     int DoBadFile);

 int FS_ScanWholeDir(  
                              FS_DIR *pDir,
                              struct FS_DIRENT *dst_DirEnt,
                              unsigned char* buffer, 
                              unsigned int DirEntMax,
                              unsigned int *pOldestEntry,
                              int DoBadFile
                           );

#if RX_SNAPSHOT_SUPPORT
extern int FS__fat_FS_MapPhotoDir(  FS_DIR *pDir,
                                              struct FS_DIRENT *dst_DirEnt,
                                              unsigned char* buffer, 
                                              unsigned int DirEntMax,
                                              unsigned int *pOldestEntry,
                                              int CheckYear,
                                              int CheckMonth,
                                              unsigned short *pMap
                                           );

extern int FS__fat_SearchPhotoWholedir(   FS_DIR *pDir,
                                                     struct FS_DIRENT *dst_DirEnt,
                                                     unsigned char* buffer, 
                                                     unsigned int DirEntMax,
                                                     unsigned int *pOldestEntry,
                                                     int DoBadFile,
                                                     int CheckYear,
                                                     int CheckMonth
                                           );

#endif
/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS_OpenDir
*
  Description:
  API function. Open an existing directory for reading.

  Parameters:
  pDirName    - Fully qualified directory name.

  Return value:
  ==0         - Unable to open the directory.
  !=0         - Address of an FS_DIR data structure.
*/

FS_DIR *FS_OpenDir(const char *pDirName)
{
    FS_DIR *handle;
    unsigned int i;
    int idx;
    char *s;

    /* Find correct FSL (device:unit:name) */
    idx = FS__find_fsl(pDirName, &s);
    if (idx < 0)
    {
        return 0;  /* Device not found */
    }
    if (FS__pDevInfo[idx].fs_ptr->fsl_opendir)
    {
        /*  Find next free entry in _FS_dirhandle */
        FS_X_OS_LockDirHandle();
        i = 0;
        while (1)
        {
            if (i >= _FS_dir_maxopen)
            {
                break;  /* No free entry in _FS_dirhandle */
            }
            if (!_FS_dirhandle[i].inuse)
            {
                break;  /* Free entry found */
            }
            i++;
        }
        if (i < _FS_dir_maxopen)
        {
            /* Execute open function of the found FSL */
            _FS_dirhandle[i].dev_index = idx;
            handle = (FS__pDevInfo[idx].fs_ptr->fsl_opendir)(s, &_FS_dirhandle[i]);
            FS_X_OS_UnlockDirHandle();
            return handle;
        }
        FS_X_OS_UnlockDirHandle();
    }
    return 0;
}


/*********************************************************************
*
*             FS_CloseDir
*
  Description:
  API function. Close a directory referred by pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  ==0         - Directory has been closed.
  ==-1        - Unable to close directory.
*/

int FS_CloseDir(FS_DIR *pDir)
{
    int i;

    if (!pDir)
    {
        return -1;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirHandle();
    if (!pDir->inuse)
    {
        /* FS_DIR structure is not in use and cannot be closed */
        FS_X_OS_UnlockDirHandle();
        return -1;
    }
    i = -1;
    if (pDir->dev_index >= 0)
    {
        if (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_closedir)
        {
            /* Execute close function of the corresponding FSL */
            i = (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_closedir)(pDir);
        }
    }
    FS_X_OS_UnlockDirHandle();
    return i;
}


/*********************************************************************
*
*             FS_ReadDir
*
  Description:
  API function. Read next directory entry in directory specified by
  pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  ==0         - No more directory entries or error.
  !=0         - Pointer to a directory entry.
*/

struct FS_DIRENT *FS_ReadDir(FS_DIR *pDir)
{
    struct FS_DIRENT *entry;

    if (!pDir)
    {
        return 0;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);
    entry = 0;
    if (pDir->dev_index >= 0)
    {
        if (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_readdir)
        {
            /* Execute FSL function */
            entry = (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_readdir)(pDir);
        }
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry;
}

/*********************************************************************
*
*             FS_ReadWholeDir
*
  Description:
  API function. Read whole directory entry in directory specified by
  pDir.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.
  dst_DirEnt  - dst_DirEnt - Pointer to destination dcf dir entry or file entry
  buffer      - DCF buffer for parsing FDB
  DirEntMax   - Maximun of file entry supporting.
  Return value:
  ==0         - No more directory entries or error.
  !=0         - Pointer to a directory entry.
*/

unsigned int FS_ReadWholeDir(FS_DIR *pDir,struct FS_DIRENT *dst_DirEnt,
                                   unsigned char* buffer, unsigned int DirEntMax,
                                   DEF_FILEREPAIR_INFO *pdcfBadFileInfo,
                                   unsigned char IsUpdateEntrySect,
                                   int DoBadFile)
{
    unsigned entry_num;
    if (!pDir)
    {
        return 0;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);

    if (pDir->dev_index >= 0)
    {
        entry_num=FS__fat_readwholedir(pDir,dst_DirEnt,buffer,DirEntMax,pdcfBadFileInfo, IsUpdateEntrySect,DoBadFile);
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}


/*********************************************************************
*
*             FS_ScanWholeDir
*
  Description:
  API function. Read whole directory entry in directory specified by
  pDir. Findout oldest file entry.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.
  dst_DirEnt  - dst_DirEnt - Pointer to destination dcf dir entry or file entry
  buffer      - DCF buffer for parsing FDB
  DirEntMax   - Maximun of file entry supporting.
  pOldestEntry- oldest file entry.
  
  Return value:
     total entry number.
*/
 int FS_ScanWholeDir( FS_DIR *pDir,
                             struct FS_DIRENT *dst_DirEnt,
                             unsigned char* buffer, 
                             unsigned int DirEntMax,
                             unsigned int *pOldestEntry,
                             int DoBadFile
                           )
{
    int entry_num;
    
    if (!pDir)
    {
        return -1;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);

    if (pDir->dev_index >= 0)
    {
        entry_num=FS__fat_ScanWholedir(pDir,dst_DirEnt,buffer,DirEntMax,pOldestEntry,DoBadFile);
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

int FS_SearchWholeDir(FS_DIR *pDir, struct FS_DIRENT *dst_DirEnt, u8* buffer, unsigned int DirEntMax, unsigned int *pOldestEntry,
                      char CHmap, char Typesel, unsigned int StartMin,   unsigned int EndMin, int DoBadFile)

{
    unsigned int entry_num = 0;

    if (!pDir)
    {
        return -1;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);

    if (pDir->dev_index >= 0)
    {
        entry_num = FS__fat_SearchWholedir(pDir, dst_DirEnt, buffer, DirEntMax, pOldestEntry, CHmap, Typesel,
                                         StartMin, EndMin, DoBadFile);
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}


#if RX_SNAPSHOT_SUPPORT
 int FS_MapPhotoDir( FS_DIR *pDir,
                            struct FS_DIRENT *dst_DirEnt,
                            unsigned char* buffer, 
                            unsigned int DirEntMax,
                            unsigned int *pOldestEntry,
                            int Year,
                            int Month,
                            unsigned short *pMap
                          )
{
    int entry_num;
    
    if (!pDir)
    {
        return -1;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);

    if (pDir->dev_index >= 0)
    {
        entry_num=FS__fat_FS_MapPhotoDir(pDir,dst_DirEnt,buffer,DirEntMax,pOldestEntry,Year,Month,pMap);
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

 int FS_SearchPhotoWholeDir(  FS_DIR *pDir,
                                         struct FS_DIRENT *dst_DirEnt,
                                         unsigned char* buffer, 
                                         unsigned int DirEntMax,
                                         unsigned int *pOldestEntry,
                                         int DoBadFile,
                                         int Year,
                                         int Month
                                     )
{
    int entry_num;
    
    if (!pDir)
    {
        return -1;  /* No pointer to a FS_DIR data structure */
    }
    FS_X_OS_LockDirOp(pDir);

    if (pDir->dev_index >= 0)
    {
        entry_num=FS__fat_SearchPhotoWholedir(pDir,dst_DirEnt,buffer,DirEntMax,pOldestEntry,DoBadFile,Year,Month);
    }
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

#endif
/*********************************************************************
*
*             FS_RewindDir
*
  Description:
  API function. Set pointer for reading the next directory entry to
  the first entry in the directory.

  Parameters:
  pDir        - Pointer to a FS_DIR data structure.

  Return value:
  None.
*/

void FS_RewindDir(FS_DIR *pDir)
{
    if (!pDir)
    {
        return;  /* No pointer to a FS_DIR data structure */
    }
    pDir->dirpos = 0;
}


/*********************************************************************
*
*             FS_MkDir
*
  Description:
  API function. Create a directory.

  Parameters:
  pDirName    - Fully qualified directory name.

  Return value:
  ==0         - Directory has been created.
  ==-1        - An error has occured.
*/

int FS_MkDir(const char *pDirName)
{
    int idx;
    int i;
    char *s;

    /* Find correct FSL (device:unit:name) */
    idx = FS__find_fsl(pDirName, &s);

    if (idx < 0)
    {
        return -1;  /* Device not found */
    }
    if (FS__pDevInfo[idx].fs_ptr->fsl_mkdir)
    {
        /* Execute the FSL function */
        FS_X_OS_LockDirHandle();
        i = (FS__pDevInfo[idx].fs_ptr->fsl_mkdir)(s, idx, 1);
        FS_X_OS_UnlockDirHandle();
        return i;
    }
    return -1;
}


/*********************************************************************
*
*             FS_RmDir
*
  Description:
  API function. Remove a directory.

  Parameters:
  pDirName    - Fully qualified directory name.
  checkflag   - 是否確認為空目錄.

  Return value:
  ==0         - Directory has been removed.
  ==-1        - An error has occured.
*/

int FS_RmDir(const char *pDirName,unsigned char checkflag)
{
    FS_DIR *dirp;
    struct FS_DIRENT *direntp;
    int idx;
    int i;
    char *s;

    /* Check if directory is empty */
    if(checkflag)
    {
        dirp = FS_OpenDir(pDirName);
        if (!dirp)
        {
            /* Directory not found */
            return -1;
        }
        i=0;
        while (1)
        {
            direntp = FS_ReadDir(dirp);
            i++;
            if (i >= 4)
            {
                break;  /* There is more than '..' and '.' */
            }
            if (!direntp)
            {
                break;  /* There is no more entry in this directory. */
            }
        }
        FS_CloseDir(dirp);
        if (i >= 4)
        {
            /*
                There is more than '..' and '.' in the directory, so you 
                must not delete it.
            */
            //DEBUG_FS("Warning!! Directory is not empty!!\n");
            return -1;
        }
    }
    //---------------------------------//
    /* Find correct FSL (device:unit:name) */
    idx = FS__find_fsl(pDirName, &s);
    if (idx < 0)
    {
        return -1;  /* Device not found */
    }
    if (FS__pDevInfo[idx].fs_ptr->fsl_rmdir)
    {
        /* Execute the FSL function */
        FS_X_OS_LockDirHandle();
        i = (FS__pDevInfo[idx].fs_ptr->fsl_rmdir)(s, idx, 0);
        FS_X_OS_UnlockDirHandle();
        return i;
    }
    return -1;
}


#endif  /* FS_POSIX_DIR_SUPPORT */

