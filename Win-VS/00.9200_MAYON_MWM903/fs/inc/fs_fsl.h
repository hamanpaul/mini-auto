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
File        : fs_fsl.h
Purpose     : Define structures for File-System-Layer
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_FSL_H_
#define _FS_FSL_H_

/*********************************************************************
*
*             Global data types
*
**********************************************************************
*/

typedef struct
{
    FARCHARPTR name;
    int		(*fsl_fopen)(const char *pFileName, const char *pMode, FS_FILE *pFile);
    int		(*fsl_fclose)(FS_FILE *pFile);
    int		(*fsl_fread)(FS_FILE *pFile, void *pData, u32 dataSize, u32 *pReadSize);
    int		(*fsl_fwrite)(FS_FILE *pFile, const void *pData, u32 dataSize, u32 *pWriteSize);
    long	(*fsl_ftell)(FS_FILE *pFile);
    int		(*fsl_fseek)(FS_FILE *pFile, long int Offset, int Whence);
    int		(*fsl_ioctl)(int Idx, u32 Id, s32 Cmd, s32 Aux, void *pBuffer);
#if FS_POSIX_DIR_SUPPORT
    int		(*fsl_opendir)(const char *pDirName, FS_DIR *pDir);
    int		(*fsl_closedir)(FS_DIR *pDir);
    int		(*fsl_readdir)(FS_DIR *pDir, FS_DIRENT *pDirEnt);
    void 	(*fsl_rewinddir)(FS_DIR *pDir);
    int		(*fsl_mkdir)(const char *pDirName, int DevIndex, char Aux);
    int		(*fsl_rmdir)(const char *pDirName, int DevIndex, char Aux);
#endif  /* FS_POSIX_DIR_SUPPORT */
}
FS__fsl_type;


#endif  /* _FS_FSL_H_ */

