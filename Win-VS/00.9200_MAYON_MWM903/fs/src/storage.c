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
#include 	"cfapi.h"
#include    "ramdiskapi.h"
#include    "sdcapi.h"
#include    "smcapi.h"
#include    "fs_fsl.h"
#include    "fs_int.h"


extern int usbfsDevStatus(u32);
extern int usbfsDevRead(u32, u32, void*);
extern int usbfsDevMulRead(u32, u32, u32, void*);
extern int usbfsDevWrite(u32, u32, void*);
extern int usbfsDevMulWrite(u32, u32, u32, void *);
extern int usbfsDevIoCtl(u32, s32, s32, void*);
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
    NULL,	// avoid warning message
    ramDiskDevWrite,
    NULL,	// avoid warning message
    ramDiskDevIoCtl
};

/* cytsai: mass storage */
const FS__device_type FS__ramdevice_driver1 =
{
    "ram1",
    ramDiskDevStatus1,
    ramDiskDevRead1,
    NULL,	// avoid warning message
    ramDiskDevWrite1,
    NULL,	// avoid warning message
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
#if (USB_HOST == 1)
const FS__device_type FS__usbfsdevice_driver =
{
    "usbfs",
    usbfsDevStatus,
    usbfsDevRead,
    usbfsDevMulRead,
    usbfsDevWrite,
    usbfsDevMulWrite,
    usbfsDevIoCtl
};
#endif
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

/*********************************************************************
*
*             Extern Global variables
*
**********************************************************************
*/
extern u32 sdcTotalBlockCount;
extern u32 usbfsTotalBlockCount;


/*********************************************************************
*
*             Function Body
*
**********************************************************************
*/
u32 GetTotalBlockCount(int Idx, int Unit)
{

    if(0==strncmp(FS__pDevInfo[Idx].devname,"sdmmc",5))
    {
        return sdcTotalBlockCount;
    }
    else if(0==strncmp(FS__pDevInfo[Idx].devname,"usbfs",5))
    {
#if USB_HOST_MASS_SUPPORT
        return usbfsTotalBlockCount;
#endif
    }

    return 0;
}
