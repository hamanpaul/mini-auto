/*

Copyright (c) 2010  MARS Semicondutor, Inc.

Module Name:

    cffs.c

Abstract:

    The routines of IDE related to file system.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/06/22  Raymond Liu  Create

*/

#include    "general.h"
#include    "osapi.h"
#include    "fsapi.h"
#include    "cfapi.h"
#include    "cferr.h"
#include    "cf.h"
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

extern u32 cfTotalBlockCount; 

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

/* Driver Function */
extern s32 sdcDetectCard(void);

/* Middleware Function */

extern void dcfFileTypeCount_Clean(void);

/*
 *********************************************************************************************************
 *                                                Application Function
 *********************************************************************************************************
 */


/*

Routine Description:

    FS driver function. Get status of IDE.

Arguments:

    Unit - The unit number.

Return Value:

    1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
    0                       - Device okay and ready for operation.
    < 0                     - An error has occured.

*/

int ideDevStatus(u32 Unit)
{
    

    return 0;
}

/*

Routine Description:

    FS driver function. Read a sector from SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to buffer for storing the data.

Return Value:

    0   - The sector has been read and copied to pBuffer.
    < 0 - An error has occured.

*/
int ideDevRead(u32 Unit, u32 Sector, void *pBuffer)
{
  if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (Sector >= cfTotalBlockCount)
    {
        return -1; /* Out of physical range */
    }

    if (!sdcDetectCard())
        return -1;
        cfReadSectors(Sector * CF_BLK_LEN,1, ((u8 *)pBuffer));
    return 0;
}

/*

Routine Description:

    FS driver function. Read a sector from SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to buffer for storing the data.

Return Value:

    0   - The sector has been read and copied to pBuffer.
    < 0 - An error has occured.

*/
int ideDevMulRead(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
   if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (Sector >= cfTotalBlockCount)
    {
        return -1; /* Out of physical range */
    }  
    if (!sdcDetectCard())
        return -1;
        cfReadSectors(Sector * CF_BLK_LEN, NumofSector, ((u8 *)pBuffer));
    return 0;
}

/*

Routine Description:

    FS driver function. Write a sector to SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to data to be stored.

Return Value:

    0   - The sector has been written to the device.
    < 0 - An error has occured.

*/
int ideDevWrite(u32 Unit, u32 Sector, void *pBuffer)
{

   if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (Sector >= cfTotalBlockCount)
    {
        return -1; /* Out of physical range */
    }  
        cfWriteSectors(Sector * CF_BLK_LEN, 1, ((u8 *)pBuffer));
    return 0;
}

/*

Routine Description:

    FS driver function. Write a sector to SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to data to be stored.

Return Value:

    0   - The sector has been written to the device.
    < 0 - An error has occured.

*/
int ideDevMulWrite(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
   if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (Sector >= cfTotalBlockCount)
    {
        return -1; /* Out of physical range */
    }  
        cfWriteSectors(Sector * CF_BLK_LEN, NumofSector, ((u8 *)pBuffer));
    return 0;
}

/*

Routine Description:

    FS driver function. Execute device command.

Arguments:

    Unit - The unit number.
    Cmd - The command to be executed.
    Aux - The parameter depending on command.
    pBuffer - The pointer to a buffer used for the command.

Return Value:

    Command specific. In general a negative value means an error.

*/
int ideDevIoCtl(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
   u32 *info;

    Aux = Aux; /* Get rid of compiler warning */

    if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }


    switch (Cmd)
    {
        case FS_CMD_GET_DEVINFO:
            if (!pBuffer)
            {
                return -1;
            }

            info = pBuffer;

            *info = cfTotalBlockCount; /* total block count */
            break;

        default:
            break;
    }
    return 0;
}
#endif
