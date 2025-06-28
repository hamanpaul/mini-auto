/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcusb.c

Abstract:

   	The routines of SMC related to file system.

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
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
#include	"osapi.h"
#include	"fsapi.h"
#include	"smcapi.h"
#include	"usbapi.h"

/*********************************************************************
*
*		Local Variables
*
**********************************************************************
*/

#define SMC_SECTOR_SIZE		512

/*********************************************************************
*
*		Local Variables
*
**********************************************************************
*/

static u8 smcCurrStat;

/*********************************************************************
*
*		Function Protoypes
*
**********************************************************************
*/

s32 smcCheckMount(void);
s32 smcGetInfo(USB_MSC_LUN_INFO*);
s32 smcGetStat(u8*);
s32 smcSetStat(u8);
s32 smcPhysRead(u8*, u32, u32);
s32 smcPhysWrite(u8*, u32, u32);

/*********************************************************************
*
*		Global Variables
*
**********************************************************************
*/

USB_MSC_FS_FUNC_TABLE smcFuncTable =
    {
        smcCheckMount,
        smcGetInfo,
        smcGetStat,
        smcSetStat,
        smcPhysRead,
        smcPhysWrite,
    };

extern u32 fsStorageSectorCount;

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
s32 smcCheckMount(void)
{
    smcCurrStat |= USB_MSC_LUN_MOUNT | USB_MSC_LUN_START;

    return 1;
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
s32 smcGetInfo(USB_MSC_LUN_INFO* pInfo)
{
    pInfo->sectorCount = fsStorageSectorCount;
    pInfo->sectorSize = SMC_SECTOR_SIZE;

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
s32 smcGetStat(u8* pStat)
{
    *pStat = smcCurrStat;

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
s32 smcSetStat(u8 stat)
{
    /* do any possible action if necessary */
    smcCurrStat = stat;

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
s32 smcPhysRead(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
    u32 i;
    u32 sectorIndex = sectorStart;
    u8* pDat = pBuf;

    for (i = 0; i < sectorCount; i++)
    {
        if (smcDevRead(0, sectorIndex, (void*)pDat) != 0)
        {
            return 0;
        }
        sectorIndex++;
        pDat += SMC_SECTOR_SIZE;
    }

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
s32 smcPhysWrite(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
    u32 i;
    u32 sectorIndex = sectorStart;
    u8* pDat = pBuf;

#if 1
    if (sectorCount==1 || sectorStart==0)
    {
        for (i = 0; i < sectorCount; i++)
        {
            if (smcDevWrite(0, sectorIndex, (void*)pDat) != 0)
            {
                return 0;
            }
            sectorIndex++;
//            pDat += SMC_SECTOR_SIZE;
            pDat += SMC_MAX_PAGE_SIZE;
        }
    }
    else
        smcDevMultiWrite(0, sectorIndex, sectorCount, pDat);
#else
    for (i = 0; i < sectorCount; i++)
    {
        if (smcDevWrite(0, sectorIndex, (void*)pDat) != 0)
        {
            return 0;
        }
        sectorIndex++;
        pDat += SMC_SECTOR_SIZE;
    }
#endif
    return 1;
}

#endif
