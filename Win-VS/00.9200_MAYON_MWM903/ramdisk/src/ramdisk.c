/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ramdisk.c

Abstract:

   	The routines of RAM disk related to file system.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

/*********************************************************************
*
*		#include Section
*
**********************************************************************
*/

#include	"general.h"
#include	"osapi.h"
#include	"fsapi.h"
#include	"usbapi.h"
#include        "fsapi.h"	/*CY 0718*/

/*********************************************************************
*
*		Local Variables        
*
**********************************************************************
*/

#ifndef FS_RR_BLOCKNUM
#define FS_RR_BLOCKNUM      8        /* 14 KB RAM */
#endif

#ifndef FS_RR_BLOCKSIZE
#define FS_RR_BLOCKSIZE     0x200     /* do not change for FAT */
#endif

static char _array[(long)FS_RR_BLOCKNUM * FS_RR_BLOCKSIZE];

/* cytsai: mass storage */
static char _array1[(long)FS_RR_BLOCKNUM * FS_RR_BLOCKSIZE];

static u8 ramDiskStat;

extern int sysStorageOnlineStat[];

/*********************************************************************
*
*		Function Protoypes        
*
**********************************************************************
*/

s32 ramDiskCheckMount(void);
s32 ramDiskGetInfo(USB_MSC_LUN_INFO*);
s32 ramDiskGetStat(u8*);
s32 ramDiskSetStat(u8);
s32 ramDiskPhysRead(u8*, u32, u32);
s32 ramDiskPhysWrite(u8*, u32, u32);

/*********************************************************************
*
*		Global Variables        
*
**********************************************************************
*/

USB_MSC_FS_FUNC_TABLE ramDiskFuncTable = 
{
	ramDiskCheckMount,
	ramDiskGetInfo,
	ramDiskGetStat,
	ramDiskSetStat,
	ramDiskPhysRead,
	ramDiskPhysWrite,	
};

extern s32 dcfStorageSize[STORAGE_MEMORY_MAX];	/*CY 0718*/

/*********************************************************************
*
*       	Local functions
*
**********************************************************************
*/

/*

Routine Description:

	FS driver function. Flush RAM disk to file.

Arguments:

	Unit - The unit number.
  	Cmd - The command to be executed.
  	Aux - The parameter depending on command.
  	pBuffer - The pointer to a buffer used for the command.

Return Value:

  	0   - Dump success.
  	< 0 - Dump failure.

*/	
int ramDiskFlush(void)
{
  	return 0;
}

/*

Routine Description:

	FS driver function. Get status of RAM disk.

Arguments:

	Unit - The unit number.

Return Value:

	1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
	0                       - Device okay and ready for operation.
	< 0                     - An error has occured.

*/	
int ramDiskDevStatus(u32 Unit) 
{
 	int DevIdx;
    
  	if (Unit != 0) {
    		return -1;  /* Invalid unit number */
  	}

    DevIdx=DCF_GetDeviceIndex("ram1");
    
  	if (!sysStorageOnlineStat[DevIdx]) {
	    	/* 
       		   Make sure, the function returns FS_LBL_MEDIACHANGED when it is
	       	   called the first time
    		*/
	    	sysStorageOnlineStat[DevIdx] = 1;
    		return FS_LBL_MEDIACHANGED;
  	}
  	
  	ramDiskStat = USB_MSC_LUN_MOUNT | USB_MSC_LUN_START; 	/*CY 0718*/
  	dcfStorageSize[STORAGE_MEMORY_RAMDISK] = FS_MEDIA_RAM_16KB;
  	
  	return 0;
}

/*

Routine Description:

	FS driver function. Read a sector from RAM disk.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to buffer for storing the data.

Return Value:

	0   - The sector has been read and copied to pBuffer.
	< 0 - An error has occured.

*/	
int ramDiskDevRead(u32 Unit, u32 Sector, void *pBuffer) 
{
	if (Unit != 0) {
		return -1;  /* Invalid unit number */
  	}
  	if (Sector >= FS_RR_BLOCKNUM) {
    		return -1;  /* Out of physical range */
  	}
  	memcpy(pBuffer, ((char*)&_array[0]) + Sector * FS_RR_BLOCKSIZE,
                  	(size_t)FS_RR_BLOCKSIZE);
                  		
	return 0;
}

/*

Routine Description:

	FS driver function. Write a sector to RAM disk.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to data to be stored.

Return Value:

	0   - The sector has been written to the device.
	< 0 - An error has occured.

*/	
int ramDiskDevWrite(u32 Unit, u32 Sector, void *pBuffer) 
{
	if (Unit != 0) {
    		return -1;  /* Invalid unit number */
  	}
  	if (Sector >= FS_RR_BLOCKNUM) {
    		return -1;  /* Out of physical range */
  	}
  	memcpy(((char*)&_array[0]) + Sector * FS_RR_BLOCKSIZE, pBuffer,
        	    	(size_t)FS_RR_BLOCKSIZE);
  	ramDiskFlush();	//write to file
  
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
int ramDiskDevIoCtl(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
	u32 *info;

	Aux = Aux;  /* Get rid of compiler warning */
	if (Unit != 0) {
		return -1;  /* Invalid unit number */
	}
	switch (Cmd) {
		case FS_CMD_GET_DEVINFO:
			if (!pBuffer) {
				return -1;
			}
			info = pBuffer;
			*info = 0;  /* hidden */
			info++;
			*info = 2;  /* head */
			info++;
			*info = 4;  /* sec per track */
			info++;
			*info = FS_RR_BLOCKNUM;
      			break;
		default:
			break;
	}
	
	return 0;
}


/*********************************************************************
*
*		Global function
*
**********************************************************************
*/

/*

Routine Description:

	Initialize RAM disk from file:ram.img if existed.

Arguments:

	None.

Return Value:

	0    - Initialization success.
	!= 0 - Initialization failure.

*/	
int ramDiskInit(void)
{
	ramDiskDevStatus(0);	//set status
	
	return 0;
}

/*

Routine Description:

	Uninitialize RAM disk from file:ram.img if existed.

Arguments:

	None.

Return Value:

	0    - Initialization success.
	!= 0 - Initialization failure.

*/	
int ramDiskUninit(void)
{
	return 0;
		
}
/* cytsai: mass storage */

/*

Routine Description:

	FS driver function. Flush RAM disk 1 to file.

Arguments:

	Unit - The unit number.
  	Cmd - The command to be executed.
  	Aux - The parameter depending on command.
  	pBuffer - The pointer to a buffer used for the command.

Return Value:

  	0   - Dump success.
  	< 0 - Dump failure.

*/	
int ramDiskFlush1(void)
{
  	return 0;
}

/*

Routine Description:

	FS driver function. Get status of RAM disk 1.

Arguments:

	Unit - The unit number.

Return Value:

	1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
	0                       - Device okay and ready for operation.
	< 0                     - An error has occured.

*/	
int ramDiskDevStatus1(u32 Unit) 
{
 	static int online1[1];

  	if (Unit != 0) {
    		return -1;  /* Invalid unit number */
  	}
  	
  	if (!online1[Unit]) {
	    	/* 
       		   Make sure, the function returns FS_LBL_MEDIACHANGED when it is
	       	   called the first time
    		*/
	    	online1[Unit] = 1;
    		return FS_LBL_MEDIACHANGED;
  	}
  	
  	ramDiskStat = USB_MSC_LUN_MOUNT | USB_MSC_LUN_START; 	/*CY 0718*/
  	dcfStorageSize[STORAGE_MEMORY_RAMDISK] = FS_MEDIA_RAM_16KB;
  	
  	return 0;
}

/*

Routine Description:

	FS driver function. Read a sector from RAM disk 1.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to buffer for storing the data.

Return Value:

	0   - The sector has been read and copied to pBuffer.
	< 0 - An error has occured.

*/	
int ramDiskDevRead1(u32 Unit, u32 Sector, void *pBuffer) 
{
	if (Unit != 0) {
		return -1;  /* Invalid unit number */
  	}
  	if (Sector >= FS_RR_BLOCKNUM) {
    		return -1;  /* Out of physical range */
  	}
  	memcpy(pBuffer, ((char*)&_array1[0]) + Sector * FS_RR_BLOCKSIZE,
                  	(size_t)FS_RR_BLOCKSIZE);
                  		
	return 0;
}

/*

Routine Description:

	FS driver function. Write a sector to RAM disk 1.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to data to be stored.

Return Value:

	0   - The sector has been written to the device.
	< 0 - An error has occured.

*/	
int ramDiskDevWrite1(u32 Unit, u32 Sector, void *pBuffer) 
{
	if (Unit != 0) {
    		return -1;  /* Invalid unit number */
  	}
  	if (Sector >= FS_RR_BLOCKNUM) {
    		return -1;  /* Out of physical range */
  	}
  	memcpy(((char*)&_array1[0]) + Sector * FS_RR_BLOCKSIZE, pBuffer,
        	    	(size_t)FS_RR_BLOCKSIZE);
  	ramDiskFlush1();	//write to file
  
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
int ramDiskDevIoCtl1(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
	u32 *info;

	Aux = Aux;  /* Get rid of compiler warning */
	if (Unit != 0) {
		return -1;  /* Invalid unit number */
	}
	switch (Cmd) {
		case FS_CMD_GET_DEVINFO:
			if (!pBuffer) {
				return -1;
			}
			info = pBuffer;
			*info = 0;  /* hidden */
			info++;
			*info = 2;  /* head */
			info++;
			*info = 4;  /* sec per track */
			info++;
			*info = FS_RR_BLOCKNUM;
      			break;
		default:
			break;
	}
	
	return 0;
}


/*********************************************************************
*
*		Global function
*
**********************************************************************
*/

/*

Routine Description:

	Initialize RAM disk 1 from file:ram.img if existed.

Arguments:

	None.

Return Value:

	0    - Initialization success.
	!= 0 - Initialization failure.

*/	
int ramDiskInit1(void)
{
	ramDiskDevStatus1(0);	//set status
	
	return 0;
}

/*

Routine Description:

	Uninitialize RAM disk 1 from file:ram.img if existed.

Arguments:

	None.

Return Value:

	0    - Initialization success.
	!= 0 - Initialization failure.

*/	
int ramDiskUninit1(void)
{
	return 0;
		
}

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
s32 ramDiskCheckMount(void)
{
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
s32 ramDiskGetInfo(USB_MSC_LUN_INFO* pInfo)
{
	pInfo->sectorCount = FS_RR_BLOCKNUM;
	pInfo->sectorSize = FS_RR_BLOCKSIZE;
	
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
s32 ramDiskGetStat(u8* pStat)
{
	*pStat = ramDiskStat;
	
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
s32 ramDiskSetStat(u8 stat)
{
	/* do any possible action if necessary */
	ramDiskStat = stat;
	
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
s32 ramDiskPhysRead(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
	u32 i;
	u32 sectorIndex = sectorStart;
	
	for (i = 0; i < sectorCount; i++)
	{
		if (ramDiskDevRead(0, sectorIndex, (void*) *pBuf) != 0)
		{
			return 0;
		}
		sectorIndex++;
		pBuf += FS_RR_BLOCKSIZE;	
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
s32 ramDiskPhysWrite(u8* pBuf, u32 sectorStart, u32 sectorCount)
{
	u32 i;
	u32 sectorIndex = sectorStart;
	
	for (i = 0; i < sectorCount; i++)
	{
		if (ramDiskDevWrite(0, sectorIndex, (void*) *pBuf) != 0)
		{
			return 0;
		}
		sectorIndex++;
		pBuf += FS_RR_BLOCKSIZE;	
	}
	
	return 1;
}
