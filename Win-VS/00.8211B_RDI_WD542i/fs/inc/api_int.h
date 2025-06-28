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
File        : api_int.h
Purpose     : Internals used accross different modules of API layer
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_API_INT_H_
#define _FS_API_INT_H_

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

extern int FS__find_fsl(const char *pFullName, FS_FARCHARPTR *pFileName);
extern int FS__fat_FindLastFreeCluster(int Idx, FS_u32 Unit);
extern int FS__fat_FindLastFileCluster(int Idx, char *pFileName);

extern FS_u32 FS__fat_findpath(int Idx, const char *pFullName, FS_FARCHARPTR *pFileName, FS_u32 *pUnit, FS_u32 *pDirStart);
extern int _FS_fat_IncDir_One(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize);
extern int _FS_fat_IncDir_Multi(int Idx, FS_u32 Unit, FS_u32 DirStart, FS_u32 *pDirSize, unsigned int IncDirSize);


#endif  /* _FS_API_INT_H_ */

