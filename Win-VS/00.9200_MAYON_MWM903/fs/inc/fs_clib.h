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
File        : fs_clib.h
Purpose     : Header file for the file system's CLIB functions.
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_CLIB_H_
#define _FS_CLIB_H_

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

/*CY 0601 S*/
#define FS__CLIB_IMPL   0
/*CY 0601 E*/

/*********************************************************************
*
*             Global function prototypes
*
**********************************************************************
*/

FARCHARPTR FS__CLIB_strchr(const char *s, int c);
u32 FS__CLIB_strlen(const char *s);
int FS__CLIB_strncmp(const char *s1, const char *s2, u32 n);
int FS__CLIB_strcmp(const char *s1, const char *s2);
int FS__CLIB_atoi(const char *s);
void *FS__CLIB_memset(void *s, int c, u32 n);
void *FS__CLIB_memcpy(void *s1, const void *s2, u32 n);
char *FS__CLIB_strncpy(char *s1, const char *s2, u32 n);
int FS__CLIB_toupper(int c);

#endif  /* _FS_CLIB_H_  */


