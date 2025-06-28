/*
**********************************************************************
*                          Micrium, Inc.
*                      949 Crestview Circle
*                     Weston,  FL 33327-1848
*
*                            uC/FS
*
*                (c) Copyright 2002, Micrium, Inc.
*                      All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File        : fs_dev.h
Purpose     : Define structures for Device Drivers
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_DEV_H_
#define _FS_DEV_H_

/*********************************************************************
*
*             Global data types
*
**********************************************************************
*/

typedef struct
{
    FARCHARPTR name;
    int (*dev_status)(u32 id);
    int (*dev_read)(u32 id, u32 block, void *buffer);
    int (*dev_mul_read)(u32 id, u32 block, u32 numofBlock, void *buffer);
    int (*dev_write)(u32 id, u32 block, void *buffer);
    int (*dev_mul_write)(u32 id, u32 block, u32 numofBlock, void *buffer);
    int (*dev_ioctl)(u32 id, s32 cmd, s32 aux, void *buffer);
} FS__device_type;


#endif
