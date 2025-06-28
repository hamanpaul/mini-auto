/*
**********************************************************************
*							Micrium, Inc.
*						949 Crestview Circle
*					 Weston,	FL 33327-1848
*
*							uC/FS
*
*			 (c) Copyright 2001 - 2003, Micrium, Inc.
*						All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File	: fs_conf.h
Purpose	: File system configuration
	Usually all configuration of the file system for your system can 
	be done by changing this file. If you are using a big endian system 
	or a totally different architecture, you may have to modify the file
	"fs_port.h".
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef _FS_CONF_H_
#define _FS_CONF_H_
#include "sysopt.h"

/*********************************************************************
*
*			 #define constants
*
**********************************************************************
*/

#define FS_DEV_ONLINE_MAX	4


/*********************************************************************
*
*			 Number of file handles
*
	Set the maximum number of simultaneously open files in your system.
	Please be aware, that the file system requires one FS_FILE structure
	for each open file. If you are using FAT, each file will also require
	two sector buffers.
*/

#define FS_MAXOPEN 8	/* Maximum number of file handles */


/*********************************************************************
*
*			 POSIX 1003.1 like directory functions support
*
	Enables functions FS_OpenDir, FS_CloseDir, FS_ReadDir, FS_RewindDir,
	FS_MkDir and FS_RmDir.
*/

#define FS_POSIX_DIR_SUPPORT 1	/* POSIX 1003.1 like directory support */
#if FS_POSIX_DIR_SUPPORT
#define FS_DIR_MAXOPEN 8	/* Maximum number of directory handles */
#endif


/*********************************************************************
*
*			 OS Layer
*
	Set all to 0, if you do not want OS support.
*/

#define FS_OS_EMBOS 0	/* 1 = use embOS */
#define FS_OS_UCOS_II 1	/* 1 = use uC/OS-II */
#define FS_OS_WINDOWS 0	/* 1 = use WINDOWS */
#if ((FS_OS_WINDOWS == 1) && ((FS_OS_EMBOS == 1) || (FS_OS_UCOS_II == 1)))
#error You must not use Windows at the same time as embOS or uC/OS-II!
#endif


/*********************************************************************
*
*			 Time/Date support
*
	If your system does support ANSI C library functions for time/date,
	you can set this to 1. If it is set to 0, functions FS_OS_Get_Date
	and FS_OS_Get_Time will return the date 1.01.1980 0:00 unless you
	modify them.
*/

/* cytsai: modified */
#define FS_OS_TIME_SUPPORT 0	/* 1 = time()/date supported */


/*********************************************************************
*
*			 File System Layer
*
	You can turn on/off the file system layers used in your system.
	At least one layer has to be active. Because currently there is only
	one file system layer supported (FAT), you will usually not change
	this setting.
*/

#define FS_USE_FAT_FSL 1	/* FAT12/FAT16 file system */


/*********************************************************************
*
*			 Logical Block Layer
*
	You can turn on/off cache of the Logical Block Layer. To use the
	cache on specific device, you also have to set its size in the
	device driver settings below.
*/

/*CY 0601 S*/
#define FS_USE_LB_READCACHE 1	/* 1 enables cache */
#define FS_LB_CACHE_SIGNFLAG 0xFFFFFFFF
/*CY 0601 E*/

/*********************************************************************
*
*			 Device Driver Support
*
	You can turn on/off device driver supported in your system.
	If you turn on a driver, please check also its settings in this
	file below.
*/

/* cytsai: 0315 */
#define FS_USE_SMC_DRIVER		1	/* SmartMedia card driver		*/
#define FS_USE_IDE_DRIVER		0	/* IDE driver					*/
#define FS_USE_WINDRIVE_DRIVER	0	/* Windows Logical Drive driver */
#define FS_USE_RAMDISK_DRIVER	0	/* RAM Disk driver				*/
#define FS_USE_MMC_DRIVER		1	/* MMC/SD card driver			*/
#define FS_USE_FLASH_DRIVER		0	/* Generic flash driver		 */

#if USB_HOST_MASS_SUPPORT
#define FS_USE_USBFS_DRIVER	 1	/* USB host mass storage*/
#else
#define FS_USE_USBFS_DRIVER	 0	/* USB host mass storage*/
#endif

#if (!defined(_WIN32) && (FS_USE_WINDRIVE_DRIVER))
#error Windows Logical Drive driver needs Windows API
#endif

/*********************************************************************
*
*			 FAT File System Layer defines
*/

#if FS_USE_FAT_FSL
/*
	For each media in your system using FAT, the file system reserves
	memory to keep required information of the boot sector of that media.
	FS_MAXDEV is the number of device drivers in your system used
	with FAT, FS_FAT_MAXUNIT is the maximum number of logical units
	supported by one of the activated drivers.
*/
/* cytsai: mass storage */
#define FS_MAXDEV (FS_USE_SMC_DRIVER + FS_USE_WINDRIVE_DRIVER + FS_USE_RAMDISK_DRIVER + FS_USE_RAMDISK_DRIVER + FS_USE_MMC_DRIVER + FS_USE_IDE_DRIVER + FS_USE_FLASH_DRIVER)
#define FS_FAT_MAXUNIT	3		 /* max number of medias per device */
#define FS_FAT_NOFAT32	0		 /* 1 disables FAT32 support */
#define FS_FAT_NOFORMAT	0		 /* 1 disables code for formatting a media */
#define FS_FAT_DISKINFO	1		 /* 0 disables command FS_CMD_GET_DISKFREE */
#define FS_FAT_FWRITE_UPDATE_DIR 0		 /* 1 FS_FWrite modifies directory (default), 0 directory 
												 is modified by FS_FClose */
/*
	Do not change following defines !
	They may become configurable in future versions of the file system.
*/
#define FS_FAT_SEC_SIZE 0x200	 /* do not change for FAT */
#endif /* FS_USE_FAT_FSL */


/*********************************************************************
*
*			 RAMDISK_DRIVER defines
*/

#if FS_USE_RAMDISK_DRIVER
/*
	Define size of your RAM disk here.
	You specify the number of sectors (512 bytes) here.
*/
#define FS_RR_BLOCKNUM 32		/* 16KB RAM */
/*
	Do not change following define !
	It may become configurable in future versions of the file system.
*/
#define FS_RR_BLOCKSIZE 0x200	 /* do not change for FAT */
#endif	/* FS_USE_RAMDISK_DRIVER */


/*********************************************************************
*
*			 SMC_DRIVER defines
*
	Settings of the generic Smartmedia Card driver.
	For using SMC in your system, you will have to provide basic
	hardware access functions. Please check device\smc\hardware\XXX\smc_x_hw.c
	and device\smc\hardware\XXX\smc_x_hw.h for samples.
*/

#if FS_USE_SMC_DRIVER
/*
	Number of card readers in your system.
	Please note, that even if your system does have more than one
	SMC slot, it might be unable to access them both at the same
	time, because they share resources (e.g. same data port). If that
	is true for your system, you must ensure, that your implementation
	of "FS_OS_Lock_deviceop(&_FS_smcdevice_driver,id)" blocks the device
	driver for all values of "id"; the default implementation of the
	file system's OS Layer does so.
*/
#define FS_SMC_MAXUNIT 1	/* EP7312 SMC + on board NAND */
/*
	The following define does tell the generic driver, if your
	system can directly check the SMC RY/BY line.
*/
#define FS_SMC_HW_SUPPORT_BSYLINE_CHECK	0	/* EP7312 does not support */
/*
	FS_SMC_HW_NEEDS_POLL has to be set, if your SMC hardware driver
	needs to be called periodically for status check (e.g. diskchange).
	In such a case, the generic driver does provide a function
	"void FS_smc_check_card(u32 id)", which has to be called
	periodically by your system for each card reader.
*/
#define FS_SMC_HW_NEEDS_POLL 1	/* EP7312 needs poll for diskchange */
/*
	Logical Block Read Cache Settings for SMC driver.
	Options are not used if FS_USE_LB_READCACHE is 0.
*/
#define FS_SMC_CACHENUM 10	 /* Number of sector buffers; 0 disables cache for this device. */
/*
	The minimum erase unit on an SMC is a physical block, which consists
	of 16 or 32 pages. A page is 256+8 or 512+16 bytes. If the file system
	is going to change a single sector (512 bytes) on the media, the SMC
	driver usually has to copy the whole corresponding physical block to
	a new block. To avoid that time consuming task for every single sector
	write, you can provide the SMC driver with a number of sector caches
	(512 bytes each). The driver will use them to cache sequential writes
	to the media. If you set the value to 0, the SMC driver will not cache
	any write access. A value of 1 makes no sense. The maximum of the media's
	cluster size (number of logical sectors) and the number of bytes you
	write with FS_FWrites divided by 512 is typically the best speed
	performance value. Bigger values usually make sense only, if you plan to
	bypass the FAT system by e.g. using the FS_IoCtl command
	FS_CMD_WRITE_SECTOR for a large number of sequential sectors.
*/
#define FS_SMC_PAGE_BUFFER 0	/* Number of sector write caches; 0 disables write cache function. */
/*
	Do not change following define !
	It may become configurable in future versions of the file system.
*/
#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 0x200	 /* do not change for FAT */ /*CY 1023*/
#endif
#endif /* FS_USE_SMC_DRIVER */


/*********************************************************************
*
*			 MMC_DRIVER defines
*
	Settings of the generic MMC/SD card driver.
	For using MMC in your system, you will have to provide basic
	hardware access functions. Please check device\mmc_sd\hardware\XXX\mmc_x_hw.c
	and device\mmc_sd\hardware\XXX\mmc_x_hw.h for samples.
*/

#if FS_USE_MMC_DRIVER
/*
	Number of card readers in your system.
	Please note, that even if your system does have more than one
	MMC interface, it might be unable to access them both at the same
	time, because they share resources (e.g. same data line). If that
	is true for your system, you must ensure, that your implementation
	of "FS_OS_Lock_deviceop(&_FS_smcdevice_driver,id)" blocks the device
	driver for all values of "id"; the default implementation of the
	file system's OS Layer does so.
*/
#define FS_MMC_MAXUNIT 1
/*
	FS_MMC_HW_NEEDS_POLL has to be set, if your MMC hardware driver
	needs to be called periodically for status check (e.g. diskchange).
*/
#define FS_MMC_HW_NEEDS_POLL 1
/*
	Logical Block Read Cache Settings for MMC driver.
	Options are not used if FS_USE_LB_READCACHE is 0.
*/
/*CY 0601 S*/
#define FS_MMC_CACHENUM 40	 /* Number of sector buffers; 0 disables cache for this device. */
/*CY 0601 E*/
/*
	Do not change following define !
	It may become configurable in future versions of the file system.
*/
#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 0x200	 /* do not change for FAT */ /*CY 1023*/
#endif
#endif /* FS_USE_MMC_DRIVER */


/*********************************************************************
*
*			 IDE_DRIVER defines
*
	Settings of the generic IDE driver. This driver is also used to
	access CF cards. For using IDE in your system, you will have to provide basic
	hardware access functions. Please check device\ide\hardware\XXX\ide_x_hw.c
	and device\ide\hardware\XXX\ide_x_hw.h for samples.
*/

#if FS_USE_IDE_DRIVER
/*
	Number of card readers in your system.
	Please note, that even if your system does have more than one
	IDE interface, it might be unable to access them both at the same
	time, because they share resources (e.g. same data line). If that
	is true for your system, you must ensure, that your implementation
	of "FS_OS_Lock_deviceop(&_FS_idedevice_driver,id)" blocks the device
	driver for all values of "id"; the default implementation of the
	file system's OS Layer does so.
*/
#define FS_IDE_MAXUNIT 1
/*
	FS_IDE_HW_NEEDS_POLL has to be set, if your IDE hardware driver
	needs to be called periodically for status check (e.g. diskchange).
*/
#define FS_IDE_HW_NEEDS_POLL 1
/*
	Logical Block Read Cache Settings for IDE driver.
	Options are not used if FS_USE_LB_READCACHE is 0.
*/
#define FS_IDE_CACHENUM 10	 /* Number of sector buffers; 0 disables cache for this device. */
#endif /* FS_USE_IDE_DRIVER */


/*********************************************************************
*
*			 FLASH_DRIVER defines
*
	Settings of the generic flash memory driver.
	For using flash memory in your system, you will have set your
	chip model you would like to use in device\flash_config.h
*/

#if FS_USE_FLASH_DRIVER
/* Size of the used ram buffer, it must be equal to the greatest
	erasable flash sector. Set this size to 0 if you don愒 have
 enough space for a flash buffer, Buffering is the automaticly
 done with reserved flash sectors.
 */
#define FS_FLASH_RAMBUFFER	0x10000 /* must be equal to the biggest erase sector size */
/* Address range settings. It is suggested that you use an external
	flash space that is mapped into the memory range */
#define FLASH_BASEADR		0x01000000	/* base address in external address space		*/
#define FLASH_USER_START	0x01000000	/* Start adress of flash useable area (1st sector) */
#define FLASH_USER_LEN		0x00400000	/* Length of user flash area */
/* Enable/disable wear leveling for flash memory */
#define FS_FLASHWEARLEVELING 1 /* 1 = on, 0 = off */
#endif


/*********************************************************************
*
*			 WINDRIVE_DRIVER defines
*
	This driver does allow to use any Windows logical driver on a
	Windows NT system with the file system. Please be aware, that Win9X
	is not supported, because it cannot access logical drives with
	"CreateFile".
*/

#if FS_USE_WINDRIVE_DRIVER
/*
	The following define tells WINDRIVE, how many logical drives
	of your NT system you are going to access with the file system.
	if your are going to use more than 2 logical drives, you
	will have to modify function "_FS_wd_devstatus" of module
	device\windriver\wd_misc.c.
*/
#define FS_WD_MAXUNIT 2			 /* number of windows drives */
/*
	Specify names of logical Windows drives used with the file system. For example,
	"\\\\.\\A:" is usually the floppy of your computer.
*/
#define FS_WD_DEV0NAME "\\\\.\\A:"	/* Windows drive name for "windrv:0:" */
#define FS_WD_DEV1NAME "\\\\.\\B:"	/* Windows drive name for "windrv:1:" */
/*
	To improve performance of WINDRIVE, it does use sector caches
	for read/write operations to avoid real device operations for
	each read/write access. The number of caches can be specified
	below and must not be smaller than 1.
*/
#define FS_WD_CACHENUM 40			/* number of read caches per drive */
#define FS_WD_WBUFFNUM 20			/* number of write caches per drive */
/*
	Do not change following define !
	It may become configurable in future versions of the file system.
*/
#define FS_WD_BLOCKSIZE 0x200		 /* do not change for FAT */
#endif	/* FS_USE_WINDRIVE_DRIVER */


/*********************************************************************
*
*			 USBFS_DRIVER defines
*
*/
#if FS_USE_USBFS_DRIVER
/*
	Number of card readers in your system.
	Please note, that even if your system does have more than one
	MMC interface, it might be unable to access them both at the same
	time, because they share resources (e.g. same data line). If that
	is true for your system, you must ensure, that your implementation
	of "FS_OS_Lock_deviceop(&_FS_smcdevice_driver,id)" blocks the device
	driver for all values of "id"; the default implementation of the
	file system's OS Layer does so.
*/
#define FS_USBFS_MAXUNIT 1
/*
	FS_USBFS_HW_NEEDS_POLL has to be set, if your USB hardware driver
	needs to be called periodically for status check (e.g. diskchange).
*/
#define FS_USBFS_HW_NEEDS_POLL 1
/*
	Logical Block Read Cache Settings for MMC driver.
	Options are not used if FS_USE_LB_READCACHE is 0.
*/
/*CY 0601 S*/
#define FS_USBFS_CACHENUM 40	 /* Number of sector buffers; 0 disables cache for this device. */
/*CY 0601 E*/
/*
	Do not change following define !
	It may become configurable in future versions of the file system.
*/
#ifndef FS_SECTOR_SIZE
#define FS_SECTOR_SIZE 0x0200	 /* do not change for FAT */ /*CY 1023*/
#endif
#endif /* FS_USE_USB_DRIVER */


/*
--------------------------------------------------------------------------------------
|			Structure of a classical generic MBR				
--------------------------------------------------------------------------------------
|	Address						Description							Size
|	Hex		Dec														(bytes)
|	+000	+0		Bootstrap code area								446
|	+1BE	+446	Partition entry 1	Partition table				16
|	+1CE	+462	Partition entry 2	(for primary partitions)	16
|	+1DE	+478	Partition entry 3								16
|	+1EE	+494	Partition entry 4								16
|	+1FE	+510	0x55				Boot signature[a]			2
|	+1FF	+511	0xAA		
|					Total size: 446 + 4×16 + 2						512
|-------------------------------------------------------------------------------------
|	+000	+0		Status or physical drive (80h bootable, 00h inactive)
|	+001	+1		CHS address of first absolute sector in partition
|	+004	+4		Partition type
|	+005	+5		CHS address of last absolute sector in partition
|	+008	+8		LBA of first absolute sector in the partition
|	+00C	+11		Number of sectors in partition
|
|	CHS to LBA formula: #lba = (#c*H + #h)*S + #s - 1
|	H = 0 ~ 255, 8 bit
|	S = 0 ~ 63, 6 bit
|	C = 0 ~ 1024, 10 bit
|	---------------------------------------------------------------
|	| H = 8 (bit) | C = 2 (hi-bit) | S = 6 (bit) | C = 8 (lo-bit) |
|	---------------------------------------------------------------
|					 └----->------->------>---------┘
|
|	In WinHex, Surplus sector and end means the "Unpartitionable space" sector index
|	Surplus sector and end = Total sector - S*H*C
|	Cylinder = Total sector / 255(HEAD) / 63(Sector) (N integer)
|
|	Unpartitionable space still used by other Volume if it 
|	had been formated all parts in one partition.
--------------------------------------------------------------------------------------	
*/

// Structure of a classical generic MBR index
#define MBR_I_PARTI_ENTRY_01 0x1BE
#define MBR_I_PARTI_ENTRY_02 0x1CE
#define MBR_I_PARTI_ENTRY_03 0x1DE
#define MBR_I_PARTI_ENTRY_04 0x1EE
#define MBR_I_BOOT_SIGNATURE 0x1FE

#define MBR_O_PARTI_ENT_STATUS		0x0
#define MBR_O_CHS_ADDR_FIRST_SEC 	0x1
#define MBR_O_PARTI_TYPE			0x4
#define MBR_O_CHS_ADDR_LAST_SEC		0x5
#define MBR_O_PARTI_ENT_FIRST_SEC	0x8
#define MBR_O_PARTI_ENT_NUM_SEC		0xC
#define MBR_O_CHS_HEAD				0x00
#define MBR_O_CHS_CYLINDER_P1		0x08	// Two High bit save in the sector area
#define MBR_O_CHS_SECTOR			0x0A
#define MBR_O_CHS_CYLINDER_P2		0x10

#define MBR_V_CHS_HEAD_SIZE		0xFF		// 8 bits = 0 ~ 255
#define MBR_V_CHS_SECTOR_SIZE	0x3F		// 6 bits = 0 ~ 63
#define MBR_V_CHS_CYLINDER_SIZE	0x3FF		// 10 bits = 0 ~ 1023 
#define MBR_V_BOOT_SIGN_0x55	0x55
#define MBR_V_BOOT_SIGN_0xAA	0xAA

/*
--------------------------------------------------------------------------------------
|			DOS 2.0 BPB	(FAT12 / FAT16)
--------------------------------------------------------------------------------------
|	Sector offset	|	BPB offset	|	 length		|	Description
|		0x00B		|		0x00	|	2 BYTES		|	Bytes per logical sector
|		0x00D		|		0x02	|	1 BYTE		|	Logical sectors per cluster
|		0x00E		|		0x03	|	2 BYTES		|	Reserved logical sectors
|		0x010		|		0x05	|	1 BYTE		|	Number of FATs
|		0x011		|		0x06	|	2 BYTES		|	Root directory entries
|		0x013		|		0x08	|	2 BYTES		|	Total logical sectors
|		0x015		|		0x0A	|	1 BYTE		|	Media descriptor
|		0x016		|		0x0B	|	2 BYTES		|	Logical sectors per FAT
--------------------------------------------------------------------------------------
|			DOS 3.31 BPB
--------------------------------------------------------------------------------------
|#		0x00B		|		0x00	|	13 BYTES	|	DOS 2.0 BPB <=
|		0x018		|		0x0D	|	2 BYTES		|	Physical sectors per track (identical to DOS 3.0 BPB)
|		0x01A		|		0x0F	|	2 BYTES		|	Number of heads (identical to DOS 3.0 BPB)
|		0x01C		|		0x11	|	4 BYTES		|	Hidden sectors (incompatible with DOS 3.0 BPB)
|		0x020		|		0x15	|	4 BYTES		|	Large total logical sectors
|#		0x024		|		0x19	|	1 BYTE		|	Physical drive number
|		0x025		|		0x1A	|	1 BYTE		|	Flags etc.
|		0x026		|		0x1B	|	1 BYTE		|	Extended boot signature (0x28 aka "4.0") (similar to DOS 4.0 EBPB and NTFS EBPB)
|		0x027		|		0x1C	|	4 BYTES		|	Volume serial number
|		0x02B 		|		0x20 	|	11 BYTES	|	Volume label
|		0x036 		|		0x2B 	|	8 BYTES		|	File-system type
--------------------------------------------------------------------------------------
|			DOS 7.1 EBPB (FAT32)
--------------------------------------------------------------------------------------
|#		0x00B		|		0x00	|	25 BYTES	|	DOS 3.31 BPB <=
|		0x024		|		0x19	|	4 BYTES		|	Logical sectors per FAT
|		0x028		|		0x1D	|	2 BYTES		|	Mirroring flags etc.
|		0x02A		|		0x1F	|	2 BYTES		|	Version
|		0x02C		|		0x21	|	4 BYTES		|	Root directory cluster
|		0x030		|		0x25	|	2 BYTES		|	Location of FS Information Sector
|		0x032		|		0x27	|	2 BYTES		|	Location of backup sector(s)
|		0x034		|		0x29	|	12 BYTES	|	Reserved (Boot file name)
|		0x040		|		0x35	|	1 BYTE		|	Physical drive number
|		0x041		|		0x36	|	1 BYTE		|	Flags etc.
|		0x042		|		0x37	|	1 BYTE		|	Extended boot signature (0x28)
|		0x043		|		0x38	|	4 BYTES		|	Volume serial number
|#		0x047		|		0x3C	|	11 BYTES	|	Volume label
|		0x052		|		0x47	|	8 BYTES		|	File-system type
--------------------------------------------------------------------------------------
*/
#define FAT_BPB_I_1216_FI_SYS_TYPE		0x36
#define FAT_BPB_I_32_FI_SYS_TYPE		0x52

#define FAT_BPB_I_BYTES_PER_SEC			0x0B
#define FAT_BPB_I_LOGI_SEC_PER_CLUS		0x0D
#define FAT_BPB_I_RESERV_SEC			0x0E
#define FAT_BPB_I_NUM_FATS				0x10
#define FAT_BPB_I_ROOT_DIR_ENTRY		0x11
#define FAT_BPB_I_TOTAL_LOGI_SEC		0x13
#define FAT_BPB_I_MEDIA_DESC			0x15
#define FAT_BPB_I_1216_SIZE				0x16
#define FAT_BPB_I_PHYS_SEC_PER_TRACK	0x18
#define FAT_BPB_I_NUM_HEADS				0x1A
#define FAT_BPB_I_HIDDEN_SEC			0x1C
#define FAT_BPB_I_LARGE_TOTAL_LOGI_SEC	0x20
#define FAT_BPB_I_32_SIZE				0x24
// FAT 12 16 USED
#define FAT_BPB_I_1216_PHYS_DRIVE_NUM	0x24
#define FAT_BPB_I_1216_FLAGS			0x25
#define FAT_BPB_I_1216_EXTEND_BOOT_SIGN	0x26
#define FAT_BPB_I_1216_VOL_SERIAL_NUM	0x27
#define FAT_BPB_I_1216_VOL_LBL			0x2B
#define FAT_BPB_I_1216_FS_TYPE			0x36
//
#define FAT_BPB_I_MIRROR_FLAG			0x28
#define FAT_BPB_I_VERSION				0x2A
#define FAT_BPB_I_ROOT_CLUS				0x2C
#define FAT_BPB_I_FS_INFO				0x30
#define FAT_BPB_I_BACKUP_SEC			0x32
#define FAT_BPB_I_RESERVED				0x34
#define FAT_BPB_I_PHYS_DRIVE_NUM		0x40
#define FAT_BPB_I_FLAGS					0x41
#define FAT_BPB_I_EXTEND_BOOT_SIGN		0x42
#define FAT_BPB_I_VOL_SERIAL_NUM		0x43
#define FAT_BPB_I_VOL_LBL				0x47
#define FAT_BPB_I_FS_TYPE				0x52


#define FAT_FSIS_I_SEC_SIGNATURE		0x00
#define FAT_FSIS_I_SEC_SIGNATURE2		0x1E4
#define FAT_FSIS_I_FREE_CLUS_COUNT		0x1E8
#define FAT_FSIS_I_NEXT_FREE_CLUS		0x1EC
#define FAT_FSIS_I_BOOT_SIGNATURE		0x1FC

// Entry
#define FAT_FAT_I_ENTEY_ATTRIBUTE		0xB

#define FS_V_FAT_ENTEY_SHORT_NAME 0x8
#define FS_V_FAT_ENTRY_DELETE	0xE5


/*
--------------------------------------------------------------------------------------
|			exFAT BPB
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name				|	Description
|		0x000		|	3 BYTES		|	Jump Boot				|	0xEB7690
|		0x003		|	8 BYTES		|	File System Name		|	"EXFAT   "
|		0x00B		|	53 BYTES	|	Must Be Zero			|	fill with 0x00
|		0x040		|	8 BYTES		|	Partition Offset		|	Sector Address
|		0x048		|	8 BYTES		|	Volume Length			|	Size of total volume in sectors
|		0x050		|	4 BYTES		|	FAT Offset				|	Sector address of 1st FAT
|		0x054		|	4 BYTES		|	FAT Length				|	Size of FAT in Sectors
|		0x058		|	4 BYTES		|	Cluster Heap offset		|	Sector address of the Data Region
|		0x05C		|	4 BYTES		|	Cluster Count			|	Number of clusters in the Cluster Heap
|		0x060		|	4 BYTES		|	Root Dir First Cluster	|	Cluster address of the Root Directory
|		0x064		|	4 BYTES		|	Vol Serial Number		|	Volume Serial Number
|		0x068		|	2 BYTES		|	File System Revision	|	VV.MM (01.00 for this release)
|		0x06A		|	2 BYTES		|	Volume Flags			|
|					|				|							|	------------------------------------------------------------------------------------------
|					|				|							|	|	Field		Offset bits		Size bits		Description
|					|				|							|	|	Active FAT			0				1			0 = 1st, 2 = 2nd
|					|				|							|	|	Volume Dirty		1				1			0 = Clean, 1 = Dirty
|					|				|							|	|	Media Failure		2				1			0 = No Faulures, 1 = Failures Reported
|					|				|							|	|	Clear to to Zero	3				1			??
|					|				|							|	|	Reserved			4				12			
|					|				|							|	------------------------------------------------------------------------------------------
|		0x06C		|	1 BYTES		|	Bytes Per Sector		|	This is a power of 2. Range: min of 2^9 = 512 byte cluster size, and a max of 212 = 4096.
|		0x06D		|	1 BYTES		|	Sectors Per Cluster		|	This is a power of 2. Range: Min of 2^1 = '2' * BPS. The maximum Cluster size is 32 MiB, 
|					|				|							|	so the Values in Bytes per Sector + Sectors Per Cluster cannot exceed 25.
|		0x06E		|	1 BYTES		|	Number of FATS			|	This number is either 1 or 2, and is only 2 if TexFAT is in use.
|		0x06F		|	1 BYTES		|	Drive Select			|	Used by INT 13
|		0x070		|	1 BYTES		|	Percent In Use			|	Percentage of Heap in use
|		0x071		|	7 BYTES		|	Reserved				|	
|		0x078		|	390	BYTES	|	Boot Code				|	The Boot Program
|		0x1FE		|	2 BYTES		|	Boot Signature			|	0xAA55
|		0x200		|	BYTES		|	Excess					|	If the sector is larger than 512 bytes, extra padding may exist beyond the signature
|-------------------------------------------------------------------------------------
|		the Addr of Up-Case Table = VBR cluster * 1 + FAT Offset * 2
|		the Addr of Root dir = VBR cluster * 1 + FAT Offset * 2 + Up-Case Table cluster * 1
--------------------------------------------------------------------------------------
*/
#define EXFAT_BPB_I_JUMP_BOOT		0x00
#define EXFAT_BPB_I_FS_NAME			0x03
#define EXFAT_BPB_I_RESERVED		0x0B
#define EXFAT_BPB_I_PARTI_OFFSET	0x40
#define EXFAT_BPB_I_VOL_LENGTH		0x48
#define EXFAT_BPB_I_FAT_OFFSET		0x50
#define EXFAT_BPB_I_FAT_LENGTH		0x54
#define EXFAT_BPB_I_CLUST_HEAP_OFFSET	0x58
#define EXFAT_BPB_I_CLUST_COUNT		0x5C
#define EXFAT_BPB_I_ROOT_CLUST		0x60
#define EXFAT_BPB_I_VOL_SERIAL_NUM	0x64
#define EXFAT_BPB_I_FS_REVISION		0x68
#define EXFAT_BPB_I_VOL_FLAGS		0x6A
#define EXFAT_BPB_I_BYTE_PER_SEC	0x6C
#define EXFAT_BPB_I_SEC_PER_CLUST	0x6D
#define EXFAT_BPB_I_NUM_FATS		0x6E
#define EXFAT_BPB_I_DRIVE_SEL		0x6F
#define EXFAT_BPB_I_PERCENT_USE		0x70
#define EXFAT_BPB_I_RESERVED2		0x71
#define EXFAT_BPB_I_BOOT_CODE		0x78
#define EXFAT_BPB_I_BOOT_SIGNATURE	0x1FE

#define EXFAT_BPB_V_BOOT_PARAMS_NUM	0x09
#define EXFAT_BPB_V_OEM_PARAMS_NUM	0x01
#define EXFAT_BPB_V_RESERVED_NUM	0x01
#define EXFAT_BPB_V_VBR_HASH_NUM	0x01
#define EXFAT_BPB_V_VBR_BACKUP_NUM	0x0C	// 12 = 9 + 1 + 1 + 1.

// File Allocation Table (FAT)
#define EXFAT_FAT_LARGEST_VALUE		0xFFFFFFF6
#define EXFAT_FAT_BAD_BLOCK			0xFFFFFFF7
#define EXFAT_FAT_MEDIA_DESCRIPTOR	0xFFFFFFF8
	// 0xFFFFFFF9 - 0xFFFFFFFE Not defined
#define EXFAT_FAT_END_OF_FILE		0xFFFFFFFF

/*
--------------------------------------------------------------------------------------
|						Volume Label Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0x83
|					|				|					|	-----------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1		
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|0		
|					|				|					|	|Code		|0		|5		|00011
|					|				|					|	-----------------------------------------
|		0x01		|	1 BYTES		|	Character Count	|	Number of charaters in label
|		0x02		|	22 BYTES	|	Volume Label	|	Volume Label in Unicode
|		0x18		|	8 BYTES		|	Reserved		|	
--------------------------------------------------------------------------------------
#	If the Entry Type is 0x03 then there is no volume label.
#	In Use就像開關一樣，只要OFF就必須開新的Entry; 已經ON的Entry可local變更。
*/


/*
--------------------------------------------------------------------------------------
|						Allocation Bitmap Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0x81
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|0		
|					|				|					|	|Code		|0		|5		|00001		
|					|				|					|	---------------------------------------
|		0x01		|	1 BYTES		|	Bit Map Flags	|	
|					|				|					|	---------------------------------------
|					|				|					|	|Bit	|Size	|Value	|Purpose
|					|				|					|	|7-1	|		|		|Reserved
|					|				|					|	|0		|1		|0		|1st Bitmap
|					|				|					|	|0		|1		|1		|2nd Bitmap
|					|				|					|	---------------------------------------
|		0x02		|	18 BYTES	|	Reserved		|
|		0x14		|	4 BYTES		|	First Cluster	|	Cluster Address of First Data Block
|		0x18		|	8 BYTES		|	Data Length		|	Length of the Data	
--------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------
|						UP-Case Table Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0x82
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|0		
|					|				|					|	|Code		|0		|5		|00010
|					|				|					|	---------------------------------------
|		0x01		|	3 BYTES		|	Reserved1		|	
|		0x04		|	4 BYTES		|	Table Checksum	|
|		0x08		|	12 BYTES	|	Reserved2		|	
|		0x14		|	4 BYTES		|	First Cluster	|	Cluster Address of First Data Block
|		0x18		|	8 BYTES		|	Data Length		|	Length of the Data	
--------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------
|						Volume GUID Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0xA0
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|1	
|					|				|					|	|Code		|0		|5		|0		
|					|				|					|	---------------------------------------
|		0x01		|	1 BYTES		|	Secondary Count	|	Always Zero
|		0x02		|	2 BYTES		|	Set Checksum	|	
|		0x04		|	2 BYTES		|	General Primary Flags		|	
|					|				|					|	---------------------------------------
|					|				|					|	|Field				|Offset	|Size	|Value
|					|				|					|	|Allocation	Possible|0		|1		|0=No
|					|				|					|	|No FAT Chain		|1		|1		|0=Valid, 1=Invalid
|					|				|					|	|Custom				|2		|14		|
|					|				|					|	---------------------------------------
|		0x06		|	16 BYTES	|	Volume GUID		|
|		0x16		|	10 BYTES	|	Reserved		|
--------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------
|						TexFAT Padding Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0xA1
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|1	
|					|				|					|	|Code		|0		|5		|1		
|					|				|					|	---------------------------------------
|		0x01		|	31 BYTES	|	Reserved		|
--------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------
|						Windows CE Access Control Table Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0xE2
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|1
|					|				|					|	|Importance	|5		|1		|1	
|					|				|					|	|Code		|0		|5		|2		
|					|				|					|	---------------------------------------
|		0x01		|	31 BYTES	|	Reserved		|
--------------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------------
|						File Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0x85
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|0		
|					|				|					|	|Importance	|5		|1		|0	
|					|				|					|	|Code		|0		|5		|00101		
|					|				|					|	---------------------------------------
|		0x01		|	1 BYTES		|	Secondary Count	|	
|		0x02		|	2 BYTES		|	Set Checksum	|	
|		0x04		|	2 BYTES		|	File Arrtibutes |	
|					|				|					|	---------------------------------------
|					|				|					|	|Attribute	|Offset	|Size	|Mask
|					|				|					|	|Reserved2	|6		|10		|
|					|				|					|	|Archive	|5		|1		|0x20
|					|				|					|	|Directory	|4		|1		|0x10
|					|				|					|	|Reserved1	|3		|1		|
|					|				|					|	|System		|2		|1		|0x04
|					|				|					|	|Hidden		|1		|1		|0x02
|					|				|					|	|Read-Only	|0		|1		|0x01
|					|				|					|	---------------------------------------
|		0x06		|	2 BYTES		|	Reserved1		|
|		0x08		|	4 BYTES		|	Create			|	DOS Timestamp Format
|		0x0C		|	4 BYTES		|	Last Modified	|	DOS Timestamp Format
|		0x10		|	4 BYTES		|	Last Accessed	|	DOS Timestamp Format
|		0x14		|	1 BYTES		|	Create 10ms		|	10ms increments between 0-199
|		0x15		|	1 BYTES		|	Last Modified 10ms	|	10ms increments between 0-199
|		0x16		|	1 BYTES		|	Create TZ Offset	|	Time zone difference to UTC in 15 min increments
|		0x17		|	1 BYTES		|	Last Modified TZ1 Offset	|	Time zone difference to UTC in 15 min increments
|		0x18		|	1 BYTES		|	Last Accessed TZ1 Offset	|	Time zone difference to UTC in 15 min increments
|		0x19		|	7 BYTES		|	Reserved2		|
--------------------------------------------------------------------------------------
#	If the In Use bit is zero (0x05) then this is probably a deleted file, it will also occur when a
	file is renamed and the number of file name extension directory entries changes.
*/

/*
--------------------------------------------------------------------------------------
|						Stream Extension Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0xC0
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|1	
|					|				|					|	|Importance	|5		|1		|0	
|					|				|					|	|Code		|0		|5		|0
|					|				|					|	---------------------------------------
|		0x01		|	1 BYTES		|	General Secondary Flags |	
|					|				|					|	---------------------------------------
|					|				|					|	|Field				|Offset	|Size	|Value
|					|				|					|	|Allocation	Possible|0		|1		|0=No, 1=Yes
|					|				|					|	|No FAT Chain		|1		|1		|0=Valid, 1=Invalid
|					|				|					|	|Custom				|2		|14		|
|					|				|					|	---------------------------------------
|		0x02		|	1 BYTES		|	Reserved1		|
|		0x03		|	1 BYTES		|	Name Length		|	Length of file name in bytes ( Maximum of 255 Unicode characters)
|		0x04		|	2 BYTES		|	Name hash		|	Hash of the file name; Used while searching for fie/directory name
|		0x06		|	2 BYTES		|	Reserved2		|	
|		0x08		|	8 BYTES		|	Valid Data Len	|	The size of the file of directory in bytes
|		0x10		|	4 BYTES		|	Reserved3		|	
|		0x14		|	4 BYTES		|	First Cluster	|	Cluster Address of First Data Block
|		0x18		|	8 BYTES		|	Data Length		|	Length of the Data If this is a directory, then the maximum value for this field is 256M
--------------------------------------------------------------------------------------
#	1 Entry Per File, If the In Use bit is zero (0x40) then this is probably part of a deleted file set
*/

/*
--------------------------------------------------------------------------------------
|						File Name Extension Directory Entry
--------------------------------------------------------------------------------------
|	Sector offset	|	length		|	Field Name		|	Description
|		0x00		|	1 BYTES		|	Entry Type		|	0xC1
|					|				|					|	---------------------------------------
|					|				|					|	|Type Field	|Offset	|Size	|Value
|					|				|					|	|In Use		|7		|1		|1	
|					|				|					|	|Category	|6		|1		|1	
|					|				|					|	|Importance	|5		|1		|0	
|					|				|					|	|Code		|0		|5		|1
|					|				|					|	---------------------------------------
|		0x01		|	1 BYTES		|	General Secondary Flags |	
|					|				|					|	---------------------------------------
|					|				|					|	|Field				|Offset	|Size	|Value
|					|				|					|	|Allocation	Possible|0		|1		|0=No, 1=Yes
|					|				|					|	|No FAT Chain		|1		|1		|0=Valid, 1=Invalid
|					|				|					|	|Custom				|2		|14		|
|					|				|					|	---------------------------------------
|		0x02		|	30 BYTES	|	File Name		|	Unicode part of filename is 15 characters, for a maximum of 255 Special filenames of 
|					|				|					|	"." And ".." have special meanings of "this directory" and "containing directory" 
|					|				|					|	and shall not be recorded.
--------------------------------------------------------------------------------------
#	If the In Use bit is zero (0x40) then this is probably part of a deleted file set
*/




#endif	/* _FS_CONF_H_ */
