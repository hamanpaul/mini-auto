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

#if FS_POSIX_DIR_SUPPORT
/*********************************************************************
*
*             Local variables
*
**********************************************************************
*/

static const unsigned int _FS_dir_maxopen = FS_DIR_MAXOPEN;
static FS_DIR             _FS_dirhandle[FS_DIR_MAXOPEN];



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
    u32 i;
    int idx, result;
    char *s;

    // Find correct FSL (device:unit:name)
    idx = FS__find_fsl(pDirName, &s);
    if (idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return 0;  // Device not found
    }
    if(!FS__pDevInfo[idx].fs_ptr->fsl_opendir)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return 0;
    }

    //  Find next free entry in _FS_dirhandle
    FS_X_OS_LockDirHandle();
    i = 0;
    while (1)
    {
        if (i >= _FS_dir_maxopen)
        {
            ERRD(FS_DIR_OVER_OPEN_ERR);
            break;  // No free entry in _FS_dirhandle
        }
        if (!_FS_dirhandle[i].inuse)
        {
            break;  // Free entry found
        }
        i++;
    }
    if (i < _FS_dir_maxopen)
    {
        // Execute open function of the found FSL
        _FS_dirhandle[i].dev_index = idx;
        result = (FS__pDevInfo[idx].fs_ptr->fsl_opendir)(s, &_FS_dirhandle[i]);
        FS_X_OS_UnlockDirHandle();
        if(result < 0)
            return NULL;
        return &_FS_dirhandle[i];
    }
    FS_X_OS_UnlockDirHandle();
    return NULL;
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
        // No valid pointer to a FS_DIR structure
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }

    if (pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    if (!pDir->inuse)
    {
        ERRD(FS_DIR_PTR_USE_ERR);
        // FS_DIR structure is not in use and cannot be closed
        return -1;
    }

    if(!FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_closedir)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    FS_X_OS_LockDirHandle();
    // Execute close function of the corresponding FSL
    i = (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_closedir)(pDir);
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

int FS_ReadDir(FS_DIR *pDir, FS_DIRENT *pDirEnt)
{
    int result;

    if (!pDir)
    {
        // No valid pointer to a FS_DIR structure
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }

    if (pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    if(!FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_readdir)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    FS_X_OS_LockDirOp(pDir);
    // Execute FSL function
    result = (FS__pDevInfo[pDir->dev_index].fs_ptr->fsl_readdir)(pDir, pDirEnt);
    FS_X_OS_UnlockDirOp(pDir);
    if(result < 0)
    {
        pDirEnt = NULL;
        return result;
    }
    return 1;
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

int FS_ReadWholeDir(FS_DIR *pDir,FS_DIRENT *dst_DirEnt, u8* buffer, unsigned int DirEntMax,
                    DEF_FILEREPAIR_INFO *pdcfBadFileInfo, u8 IsUpdateEntrySect, int DoBadFile)
{
    int entry_num = 0;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_DIR data structure
    }

    if(pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDirOp(pDir);
    // If entry_num < 0, it means something
    entry_num = FS__fat_readwholedir(pDir, dst_DirEnt, buffer, DirEntMax, pdcfBadFileInfo, IsUpdateEntrySect, DoBadFile);
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
int FS_ScanWholeDir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax,
                    u32 *pOldestEntry, int DoBadFile)
{
    int entry_num = 0;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_DIR data structure
    }

    if(pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDirOp(pDir);
    entry_num = FS__fat_ScanWholedir(pDir, dst_DirEnt, buffer, DirEntMax, pOldestEntry, DoBadFile);
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

int FS_FetchItems(FS_DIR *pDir, FS_DIRENT *pDstEnt, FS_SearchCondition *pCondition)
{
    int entry_num = 0;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_DIR data structure
    }

    if(pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDirOp(pDir);
    entry_num = FS__fat_FetchItems(pDir, pDstEnt, pCondition);
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

int FS_SearchWholeDir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry,
                      char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile)
{
    int entry_num = 0;

    if (!pDir)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_DIR data structure
    }

    if(pDir->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    FS_X_OS_LockDirOp(pDir);
    entry_num = FS__fat_SearchWholedir(pDir, dst_DirEnt, buffer, DirEntMax, pOldestEntry, CHmap, Typesel,
                                       StartMin, EndMin, DoBadFile);
    FS_X_OS_UnlockDirOp(pDir);
    return entry_num;
}

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
        ERRD(FS_PARAM_PTR_EXIST_ERR);
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
    int i, idx;
    char *s;

    // Find correct FSL (device:unit:name)
    idx = FS__find_fsl(pDirName, &s);
    if (idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;  // Device not found
    }

    if(!FS__pDevInfo[idx].fs_ptr->fsl_mkdir)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    // Execute the FSL function
    FS_X_OS_LockDirHandle();
    i = (FS__pDevInfo[idx].fs_ptr->fsl_mkdir)(s, idx, 1);
    FS_X_OS_UnlockDirHandle();
    return i;
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

int FS_RmDir(const char *pDirName, u8 checkflag)
{
    FS_DIR* dirp;
    FS_DIRENT direntp;
    int result, idx, i;
    char *s;

    // Check if directory is empty
    if(checkflag)
    {
        dirp = FS_OpenDir(pDirName);
        if (!dirp)
        {
            ERRD(FS_DIR_OPEN_ERR);
            return -1;	// Directory not found
        }
        i = 0;
        while (1)
        {
            result = FS_ReadDir(dirp, &direntp);
            if(result < 0)
            {
                ERRD(FS_DIR_READ_ERR);
                break;	// There is no more entry or error happened in this directory.
            }
            i++;
            if (i >= 4)
            {
                break;  // There is more than '..' and '.'
            }
        }
        FS_CloseDir(dirp);
        if (i >= 4)
        {
            // There is more than '..' and '.' in the directory, so you must not delete it.
            ERRD(FS_DIR_DELETE_ERR);
            return -1;
        }
    }
    //---------------------------------//
    // Find correct FSL (device:unit:name)
    idx = FS__find_fsl(pDirName, &s);
    if (idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;  // Device not found
    }

    if(!FS__pDevInfo[idx].fs_ptr->fsl_rmdir)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    // Execute the FSL function
    FS_X_OS_LockDirHandle();
    i = (FS__pDevInfo[idx].fs_ptr->fsl_rmdir)(s, idx, 0);
    FS_X_OS_UnlockDirHandle();
    return i;
}


#endif  /* FS_POSIX_DIR_SUPPORT */

