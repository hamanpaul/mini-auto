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
File        : storage.c
Purpose     : Storage API declaration
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

#include    "general.h"

#include    "osapi.h"

#include    "fsapi.h"
#include    "fs_clib.h"
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#include 	"cfapi.h"
#endif
#include    "ramdiskapi.h"
#include    "sdcapi.h"
#include        "smcapi.h"

/*********************************************************************
*
*             Global variables
*
**********************************************************************
*/

/* cytsai: 0315 */

const FS__device_type FS__ramdevice_driver =
    {
        "ram",
        ramDiskDevStatus,
        ramDiskDevRead,
        ramDiskDevRead,
        ramDiskDevWrite,
        ramDiskDevWrite,
        ramDiskDevIoCtl
    };

/* cytsai: mass storage */
const FS__device_type FS__ramdevice_driver1 =
    {
        "ram1",
        ramDiskDevStatus1,
        ramDiskDevRead1,
        ramDiskDevRead1,
        ramDiskDevWrite1,
        ramDiskDevWrite1,
        ramDiskDevIoCtl1
    };

const FS__device_type FS__mmcdevice_driver =
    {
        "sdmmc",
        sdcDevStatus,
        sdcDevRead,
        sdcDevMulRead,
        sdcDevWrite,
        sdcDevMulWrite,
        sdcDevIoCtl
    };

#if ( (FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

const FS__device_type FS__smcdevice_driver =
    {
        "smc",
        smcDevStatus,
        smcDevRead,
        smcDevMultiRead,
        smcDevWrite,
        smcDevMultiWrite,
        smcDevIoCtl
    };
#endif
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
const FS__device_type FS__idedevice_driver =
    {
        "ide",
        ideDevStatus,
        ideDevRead,
        ideDevMulRead,
        ideDevWrite,
        ideDevMulWrite,
        ideDevIoCtl
    };
#endif
