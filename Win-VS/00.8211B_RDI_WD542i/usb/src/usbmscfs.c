/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbmscfs.c

Abstract:

   	USB Mass Storage Class file system related routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbmsc.h"
#include "usbapi.h"
#include "sysapi.h"
#include "osapi.h"
#include "sdcapi.h"
#include "uiapi.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbMscFsInitLuns(void);
s32 usbMscFsLunGetInfo(u32);
s32 usbMscFsLunGetStat(u32);
s32 usbMscFsLunSetStat(u32);
s32 usbMscFsLunRead(u32, u8*, u32, u32);
s32 usbMscFsLunWrite(u32, u8*, u32, u32);
s32 usbMscFsLunLoad(u32);
s32 usbMscFsLunUnload(u32);
s32 usbMscFsLunStart(u32);
s32 usbMscFsLunStop(u32);
s32 usbMscFsUnInitLuns(void);


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

#define USB_MSC_FS_TIMEOUT		0x1000

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

extern u32 dcfStorageType; /*CY 1023*/
extern USB_MSC_FS_FUNC_TABLE ramDiskFuncTable;
extern USB_MSC_FS_FUNC_TABLE sdcFuncTable;

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern USB_MSC_FS_FUNC_TABLE smcFuncTable;
#endif

u32 usbMscCurLun = 0;

USB_MSC_LUN_INFO usbMscLunInfo[USB_MSC_MAX_LUN];
u8 usbMscLunStat[USB_MSC_MAX_LUN];

USB_MSC_FS_FUNC_TABLE *usbMscFsFuncTable[USB_MSC_MAX_LUN];

OS_EVENT* usbSemMscFs; /* semaphore to msc file system access */

extern OS_FLAG_GRP *gSdUsbProcFlagGrp;
extern OS_FLAG_GRP *gUiStateFlagGrp;

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize LUNs information.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsInitLuns(void)
{
    u8 i;

    /* initialize lun information */
    memset((void*)usbMscLunInfo, 0, sizeof(USB_MSC_LUN_INFO) * USB_MSC_MAX_LUN);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    /* cytsai: select current storage type; currently, only one lun is supported */
    switch (dcfStorageType) /*CY 1023*/
    {
    case STORAGE_MEMORY_RAMDISK:
        usbMscFsFuncTable[0] = &ramDiskFuncTable;
        break;

    case STORAGE_MEMORY_SD_MMC:
        usbMscFsFuncTable[0] = &sdcFuncTable;
        break;

    case STORAGE_MEMORY_SMC_NAND:
    default:

		#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
        usbMscFsFuncTable[0] = &smcFuncTable;
		#endif
        break;
    }

#else

    switch (dcfStorageType) /*CY 1023*/
    {
    case STORAGE_MEMORY_RAMDISK:
        usbMscFsFuncTable[0] = &ramDiskFuncTable;
        break;

    case STORAGE_MEMORY_SD_MMC:
    default:

        usbMscFsFuncTable[0] = &sdcFuncTable;

        break;
    }

#endif


    /* get lun information */
    for (i = 0; i < USB_MSC_MAX_LUN; i++)
        usbMscFsLunGetInfo(i);

    /* Create semaphore for accessing file system through usb msc */
    usbSemMscFs = OSSemCreate(1);

    return 1;
}



/*

Routine Description:

	UnInitialize MSC FS LUNs.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsUnInitLuns(void)
{
	u8	err;

	usbSemMscFs = OSSemDel(usbSemMscFs, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemMscFs Failed = %d\n", err);
		return 0;
	}

	return 1;
}

void usbInitLuns(void)
{
    u8 i;

    /* initialize lun information */
    memset((void*)usbMscLunInfo, 0, sizeof(USB_MSC_LUN_INFO) * USB_MSC_MAX_LUN);

    /* cytsai: select current storage type; currently, only one lun is supported */
    switch (dcfStorageType) /*CY 1023*/
    {
    case STORAGE_MEMORY_RAMDISK:
        usbMscFsFuncTable[0] = &ramDiskFuncTable;
        break;

    case STORAGE_MEMORY_SD_MMC:
        usbMscFsFuncTable[0] = &sdcFuncTable;
        break;

    case STORAGE_MEMORY_SMC_NAND:
    default:
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
        usbMscFsFuncTable[0] = &smcFuncTable;
#endif
        break;
    }

    /* get lun information */
    for (i = 0; i < USB_MSC_MAX_LUN; i++)
        usbMscFsLunGetInfo(i);
}
/*

Routine Description:

	Get LUN information.

Arguments:

	lun - Logical unit number.
	pInfo - LUN information.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunGetInfo(u32 lun)
{
	u8	err;
	u32	waitFlag;
	

	/* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;


	waitFlag = OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_WAIT_CLR_ANY, &err);

    if(waitFlag & FLAGUI_SD_INIT)
    {
		OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_CLR, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_USB("Error!!  gSdUsbProcFlagGrp is %d.\n", err);
		}

	    /* check if lun mounted */
	    if ((*usbMscFsFuncTable[lun]->checkMount)())
	    {	/* already mounted */
	        /* get lun status */
			(*usbMscFsFuncTable[lun]->getInfo)(&usbMscLunInfo[lun]);
			OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_SET, &err);
			return 1;
	    }
	    else
	    {	/* not yet mounted */
			OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_SET, &err);
	        return 0;
	    }
    }
	else
	{
		return 0;
	}
	
}

/*

Routine Description:

	Get LUN status.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunGetStat(u32 lun)
{
	u8	err;
	u32	waitFlag;
	
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

	waitFlag = OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_WAIT_CLR_ANY, &err);


    if(waitFlag & FLAGUI_SD_INIT)
    {
		OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_CLR, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_USB("Error!!  gSdUsbProcFlagGrp is %d.\n", err);
		}

	    /* check if lun mounted */
	    if ((*usbMscFsFuncTable[lun]->checkMount)())
	    {	/* already mounted */
	        /* get lun status */
			(*usbMscFsFuncTable[lun]->getStat)(&usbMscLunStat[lun]);
			OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_SET, &err);
			return 1;
	    }
	    else
	    {	/* not yet mounted */
			OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_SET, &err);
	        return 0;
	    }
    }
	else
	{
		return 0;
	}
	
}

/*

Routine Description:

	Set LUN status.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunSetStat(u32 lun)
{
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    /* check if lun mounted */
    if ((*usbMscFsFuncTable[lun]->checkMount)())
    {	/* already mounted */
        /* set lun status */
        return ((*usbMscFsFuncTable[lun]->setStat)(usbMscLunStat[lun]));
    }
    else
    {	/* not yet mounted */
        return 0;
    }
}

/*

Routine Description:

	Read LUN blocks.

Arguments:

	lun - Logical unit number.
	pBuf - Buffer to read to.
	blockStart - Start block to read.
	blockCount - Number of block to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunRead(u32 lun, u8* pBuf, u32 blockStart, u32 blockCount)
{
    u8 err;
    s32 ret;

    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
    {
        DEBUG_USB("Error: Invalid logical unit number %d.\n", lun);
        return 0;
    }

    /* acquire the semaphore */
    OSSemPend(usbSemMscFs, USB_MSC_FS_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemMscFs is %d.\n", err);
        return 0;
    }

    /* read from lun */
    ret = (*usbMscFsFuncTable[lun]->physRead)(pBuf, blockStart, blockCount);

    /* release the semaphore */
    OSSemPost(usbSemMscFs);

    return(ret);
}

/*

Routine Description:

	Write LUN blocks.

Arguments:

	lun - Logical unit number.
	pBuf - Buffer to write from.
	blockStart - Start block to write.
	blockCount - Number of block to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunWrite(u32 lun, u8* pBuf, u32 blockStart, u32 blockCount)
{
    u8 err;
    s32 ret;

    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    /* acquire the semaphore */
    OSSemPend(usbSemMscFs, USB_MSC_FS_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemMscFs is %d.\n", err);
        return 0;
    }

    /* read from lun */
    ret = (*usbMscFsFuncTable[lun]->physWrite)(pBuf, blockStart, blockCount);

    /* release the semaphore */
    OSSemPost(usbSemMscFs);
    /* write to lun */
    return(ret);
}

/*

Routine Description:

	Load LUN.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunLoad(u32 lun)
{
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    return 1;
}

/*

Routine Description:

	Unload LUN.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunUnload(u32 lun)
{
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    return 1;
}

/*

Routine Description:

	Start LUN.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunStart(u32 lun)
{
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    return 1;
}

/*

Routine Description:

	Stop LUN.

Arguments:

	lun - Logical unit number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscFsLunStop(u32 lun)
{
    /* check if lun is valid */
    if (lun >= USB_MSC_MAX_LUN)
        return 0;

    return 1;
}
