/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sdcusb.c

Abstract:

   	The routines of SDC to file system.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

/*CY 1023*/

/*********************************************************************
*
*		#include Section
*
**********************************************************************
*/

#include	"general.h"
#include	"osapi.h"
#include	"fsapi.h"
#include	"sdcapi.h"
#include	"usbapi.h"

/*********************************************************************
*
*		Constant
*
**********************************************************************
*/

#define SDC_SECTOR_SIZE		512

/*********************************************************************
*
*		Local Variables
*
**********************************************************************
*/

u8 sdcCurrStat;

/*********************************************************************
*
*		Function Protoypes
*
**********************************************************************
*/

s32 sdcCheckMount(void);
s32 sdcGetInfo(USB_MSC_LUN_INFO*);
s32 sdcGetStat(u8*);
s32 sdcSetStat(u8);
s32 sdcPhysRead(u8*, u32, u32);
s32 sdcPhysWrite(u8*, u32, u32);

/* Driver Function */
extern s32 sdcDetectCard(void);

/*********************************************************************
*
*		Global Variables
*
**********************************************************************
*/
OS_FLAG_GRP  *gSdUsbProcFlagGrp;

USB_MSC_FS_FUNC_TABLE sdcFuncTable =
    {
        sdcCheckMount,
        sdcGetInfo,
        sdcGetStat,
        sdcSetStat,
        sdcPhysRead,
        sdcPhysWrite,
    };

//extern u32 fsStorageSectorCount;
extern u32 sdcTotalBlockCount;

/*********************************************************************
*
*		Global function
*
**********************************************************************
*/

/*

Routine Description:

	Check if mounted.

Arguments:

	None.

Return Value:

	0 - Not yet mounted.
	1 - Already mounted.

*/
s32 sdcCheckMount(void)
{
    if (sdcDetectCard() == 1)
    {
//        sdcCurrStat |= USB_MSC_LUN_MOUNT | USB_MSC_LUN_START;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*

Routine Description:

	Get information.

Arguments:

	pInfo - Information.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 sdcGetInfo(USB_MSC_LUN_INFO* pInfo)
{
//    pInfo->sectorCount = fsStorageSectorCount;
    pInfo->sectorCount = sdcTotalBlockCount;
    pInfo->sectorSize = SDC_SECTOR_SIZE;
//    DEBUG_SDC("     sdcTotalBlockCount %d \n",sdcTotalBlockCount);
    return 1;
}

/*

Routine Description:

	Get status.

Arguments:

	pStat - The status.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 sdcGetStat(u8* pStat)
{
    *pStat = sdcCurrStat;
    return 1;
}

/*

Routine Description:

	Set status.

Arguments:

	stat - The status.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 sdcSetStat(u8 stat)
{
    /* do any possible action if necessary */

	u8	err;


	OSFlagPend(gSdUsbProcFlagGrp, FLAGSD_SET_SD_STAT, (OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), TIMEOUT_SDC, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SDC("Error SDC_USB: Wait sdcSetStat error!\n");
		return 0;
	}
	
	sdcCurrStat = stat;

	OSFlagPost(gSdUsbProcFlagGrp, FLAGSD_SET_SD_STAT, OS_FLAG_SET, &err);

    return 1;
}

/*

Routine Description:

	Read physical sectors.

Arguments:

	pBuf - Buffer to read to.
	sectorStart - Start sector index.
	sectorCount - Count of sectors to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 sdcPhysRead(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
    u32 i;
    u32 sectorIndex = sectorStart;
    u8* pDat = pBuf;

#if USB_MUL_RW /* For Multiple Read */
    sdcDevMulRead(0, sectorIndex, sectorCount, (void*)pDat);
#else /* For Single Read */
    for (i = 0; i < sectorCount; i++)
    {
        if (sdcDevRead(0, sectorIndex, (void*)pDat) != 0)
            return 0;

        sectorIndex++;
        pDat += SDC_SECTOR_SIZE;
    }
#endif

    return 1;
}

/*

Routine Description:

	Write physical sectors.

Arguments:

	pBuf - Buffer to write from.
	sectorStart - Start sector index.
	sectorCount - Count of sectors to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 sdcPhysWrite(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
    u32 i;
    u32 sectorIndex = sectorStart;
    u8* pDat = pBuf;

#if USB_MUL_RW /* For Multiple Write */
    sdcDevMulWrite(0, sectorIndex, sectorCount, (void*)pDat);
#else /* For Single Write */
    for (i = 0; i < sectorCount; i++)
    {
        if (sdcDevWrite(0, sectorIndex, (void*)pDat) != 0)
        {
            return 0;
        }
        sectorIndex++;
        pDat += SDC_SECTOR_SIZE;
    }
#endif

    return 1;
}
