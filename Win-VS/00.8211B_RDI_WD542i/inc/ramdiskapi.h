/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ramdiskapi.h

Abstract:

   	The application interface of the RAM disk.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __RAM_DISK_API_H__
#define __RAM_DISK_API_H__

extern int ramDiskInit(void);
extern int ramDiskUninit(void);
extern int ramDiskDevStatus(u32); 
extern int ramDiskDevRead(u32, u32, void*); 
extern int ramDiskDevWrite(u32, u32, void*); 
extern int ramDiskDevIoCtl(u32, s32, s32, void*);

extern int ramDiskInit1(void);
extern int ramDiskUninit1(void);
extern int ramDiskDevStatus1(u32); 
extern int ramDiskDevRead1(u32, u32, void*); 
extern int ramDiskDevWrite1(u32, u32, void*); 
extern int ramDiskDevIoCtl1(u32, s32, s32, void*);

#endif
