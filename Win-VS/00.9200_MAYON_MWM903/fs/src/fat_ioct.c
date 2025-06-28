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
File        : fat_ioctc.c
Purpose     : FAT File System Layer IOCTL
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

#include "fsapi.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "fs_os.h"
#include "fs_fat.h"
#include "fs_clib.h"
#include "mcpuapi.h"

/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

#define FS_KNOWNMEDIA_NUM   sizeof(_FS_wd_format_media_table) / sizeof(_FS_wd_format_media_type)

#ifndef FS_FAT_NOFORMAT
#define FS_FAT_NOFORMAT       0
#endif

#ifndef FS_FAT_DISKINFO
#define FS_FAT_DISKINFO       1
#endif

#ifndef FS_SUPPORT_SEC_ACCESS
#define FS_SUPPORT_SEC_ACCESS 1
#endif

/*********************************************************************
*
*             Local data types
*
**********************************************************************
*/

#if (FS_FAT_NOFORMAT==0)

typedef struct
{
    s32  media_id;
    u32  totsec32;
    u32  hiddsec;
    u16  totsec16;
    u16  rootentcnt;
    u32  fatsz16;
    u16  secpertrk;
    u16  numheads;
    char secperclus;
    char media;
    char fsystype;
    u16 usReservedSize;
}
_FS_wd_format_media_type;


typedef struct
{
    u32 SecNum;
    u32 Num;
}
_FS_FAT_ROOTENTCNT;


typedef struct
{
    u32 SecNum;
    u16 Num;
}
_FS_FAT_SECPERCLUST;


/*********************************************************************
*
*             Global Variables
*
**********************************************************************/



/*********************************************************************
*
*             Extern Global Variables
*
**********************************************************************/
extern FS_DISKFREE_T global_diskInfo;
extern char strVolName[];

/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/


/*********************************************************************
*
*             Local Variables
*
**********************************************************************
*/

/*CY 0718*/
static _FS_wd_format_media_type _FS_wd_format_media_table[] =
{
    //==RAM DISK==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
    { FS_MEDIA_RAM_16KB,  0x00000000UL, 0x00000000UL, 0x0020,   0x0040,  			0x0001,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_32KB,  0x00000000UL, 0x00000000UL, 0x0040,   0x0040,  			0x0001,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_64KB,  0x00000000UL, 0x00000000UL, 0x0080,   0x0040,  			0x0001,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_128KB, 0x00000000UL, 0x00000000UL, 0x0100,   0x0080,				0x0001,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_256KB, 0x00000000UL, 0x00000000UL, 0x0200,   0x0080, 			0x0002,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_512KB, 0x00000000UL, 0x00000000UL, 0x0400,   0x00e0,  			0x0003,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_FD_144MB,  0x00000000UL, 0x00000000UL, 0x0b40,   0x00e0,  			0x0009,  0x0012,    0x0002,   	0x01,       	(char) 0xf0,  0,        1 },
    //==SMC==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    { FS_MEDIA_SMC_1MB,   0x00000000UL, 0x0000000dUL, 0x07c3,   0x0100,  			0x0001,  0x0004,    0x0004,   	0x08,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_2MB,   0x00000000UL, 0x0000000bUL, 0x0f95,   0x0100,  			0x0002,  0x0008,    0x0004,   	0x08,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_4MB,   0x00000000UL, 0x0000001bUL, 0x1f25,   0x0100,  			0x0002,  0x0008,    0x0004,   	0x10,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_8MB,   0x00000000UL, 0x00000019UL, 0x3e67,   0x0100,  			0x0003,  0x0010,    0x0004,   	0x10,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_16MB,  0x00000000UL, 0x00000029UL, 0x7cd7,   ROOT_ENTRY_CNT,		0x0003,  0x0010,    0x0004,   	SEC_PER_CLS,    (char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_32MB,  0x00000000UL, 0x00000023UL, 0xf9dd,   ROOT_ENTRY_CNT, 	0x0006,  0x0010,    0x0008,   	SEC_PER_CLS,	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_64MB,  0x0001f3c9UL, 0x00000037UL, 0x0000,   ROOT_ENTRY_CNT, 	0x000c,  0x0020,    0x0008,   	SEC_PER_CLS,	(char) 0xf8,  0,        1 },
    { FS_MEDIA_SMC_128MB, TOSECT32, 0x0000002fUL, 0x0000,   	ROOT_ENTRY_CNT,		FATSZ16,  0x0020,    0x0010,	SEC_PER_CLS,	(char) 0xf8,  1,        1 },
#endif
    //==MMC==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
    { FS_MEDIA_MMC_16MB,  0x00000000UL, 0x00000039UL, 0x7187,   0x0200,  			0x000b,  0x003f,    0x00ff,   	0x08,           (char) 0xf8,  0,        1 },
    { FS_MEDIA_MMC_32MB,  0x00000000UL, 0x00000020UL, 0xf460,   0x0200,  			0x003d,  0x0020,    0x0004,   	0x04,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_MMC_64MB,  0x0001d9d9UL, 0x00000027UL, 0x0000,   0x0200,  			0x00ec,  0x003f,    0x00ff,   	0x02,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_MMC_128MB, 0x0003ca00UL, 0x00000061UL, 0x0000,   0x0200,  			0x00f2,  0x003f,    0x00ff,   	0x04,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_MMC_256MB, 0x00079f9bUL, 0x00000065UL, 0x0000,   0x0200,  			0x00f4,  0x003f,    0x00ff,   	0x08,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_MMC_512MB, 0x000f1f00UL, 0x00000061UL, 0x0000,   0x0200,  			0x00f2,  0x003f,    0x00ff,   	0x10,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_MMC_1GB,   0x001e830bUL, 0x000000f5UL, 0x0000,   0x0200,  			0x00f5,  0x003f,    0x00ff,   	0x20,           (char) 0xf8,  1,        1 },
    //==SDC==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
    { FS_MEDIA_SD_16MB,   0x00000000UL, 0x00000039UL, 0x7187,   0x0200,  			0x000b,  0x003f,    0x00ff,   	0x08,           (char) 0xf8,  0,        1 },
    { FS_MEDIA_SD_32MB,   0x00000000UL, 0x00000020UL, 0xf460,   0x0200,  			0x003d,  0x0020,    0x0004,   	0x04,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_64MB,   0x0001d9d9UL, 0x00000027UL, 0x0000,   0x0200,  			0x00ec,  0x003f,    0x00ff,   	0x02,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_128MB,  0x0003ca00UL, 0x00000061UL, 0x0000,   0x0200,  			0x00f2,  0x003f,    0x00ff,   	0x04,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_256MB,  0x00079f9bUL, 0x00000065UL, 0x0000,   0x0200,  			0x00f4,  0x003f,    0x00ff,   	0x08,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_512MB,  0x000f1f00UL, 0x00000061UL, 0x0000,   0x0200,  			0x00f2,  0x003f,    0x00ff,   	0x10,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_1GB,    0x001e830bUL, 0x000000f5UL, 0x0000,   0x0200,  			0x00f5,  0x003f,    0x00ff,   	0x40,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_2GB,    0x003cbf07UL, 0x000000f5UL, 0x0000,   0x0200,  			0x00f5,  0x003f,    0x00ff,  	0x40,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_SD_4GB,    0x00771400UL, 0x000000f5UL, 0x0000,   0x0000,  			0x1DEA,  0x003f,    0x00ff,   	0x40,           (char) 0xf8,  2,       32 },
    { FS_MEDIA_SD_8GB,    0x00f4d400UL, 0x000000f5UL, 0x0000,   0x0000,  			0x3D17,  0x003f,    0x00ff,   	0x40,           (char) 0xf8,  2,       32 },
    { FS_MEDIA_SD_16GB,   0x00ffffffUL, 0x000000f5UL, 0x0000,   0x0000,  			0x00f5,  0x003f,    0x00ff,   	0x40,           (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_32GB,   0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_64GB,   0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_128GB,  0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_256GB,  0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_512GB,  0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_1TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_2TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_3TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_4TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_5TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??
    { FS_MEDIA_SD_6TB,    0x03CC0000UL, 0x000000f5UL, 0x0000,	0x0000, 			0x0000,  0x003f,	0x00ff, 	0x40,		    (char) 0xf8,  2,       32 },  //Lucian: ??


    //==Compact Flash==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
    { FS_MEDIA_CF_32MB,   0x00000000UL, 0x00000020UL, 0xf760,   0x0200,  			0x007c,  0x0020,    0x0004,   	0x02,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_CF_64MB,   0x0001e860UL, 0x00000020UL, 0x0000,   0x0200,  			0x007b,  0x0020,    0x0004,   	0x04,           (char) 0xf8,  1,        1 }
};

#if 0 // was declared but never referenced
/* table for getting number of root entries for a given media size */
static const _FS_FAT_ROOTENTCNT _FS_auto_rootcnt[] =
{
    {   	  0x100,     0x40 },
    {         0x200,     0x80 },
    {      0x0b40UL,     0xe0 },
    {  0x0001f3c9UL,    0x100 },
    {  0xffffffffUL,    0x200 }
};


/* table for calculating cluster size */
static const _FS_FAT_SECPERCLUST _FS_auto_secperclust[] =
{
    /* medias up to 512MB are formatted with FAT16 */
    {     0x0b40UL, 0x0001 },
    {      32680UL, 0x0002 },
    {     262144UL, 0x0004 },
    {     524288UL, 0x0008 },
    {    1048576UL, 0x0010 },
    /* medias bigger than 512MB are formatted with FAT32 */
    {   16777216UL, 0x0008 },
    {   33554432UL, 0x0010 },
    {   67108864UL, 0x0020 },
    { 0xffffffffUL, 0x0040 }
};
#endif

#endif /* FS_FAT_NOFORMAT==0 */

/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

#if (FS_FAT_NOFORMAT==0)
/*********************************************************************
 *
 *             _FS_MBR_format
 Description:
 	?FS internal function. Format a media using specified parameters.
 	Currently this function needs many parameters. The function will be
	improved.

 	Parameters:
 		pDriver     - Pointer to a device driver function table.
 		Unit		- Unit number.
 		mediaInfo	- Media fat info
 		pEnt1SecStart 	- if there exists MBR info in first sector,
						value shall be the partition entry 1 LBA location.
						0 = MBR doesn't exist.
 		pEnt1SecNum		- partition 1's total sector number. Same defination
 						as pEnt1SecStart.
 	Return value:
 		1			- formatted successful or it has been formatted.
  		0			- the device doesn't need the Partition Info.
  		-1			- An error has occured.
*/

int _FS_MBR_format(const FS__device_type *pDriver, u32 Unit, _FS_wd_format_media_type *mediaInfo, u32 *pEnt1SecStart, u32 *pEnt1SecNum)
{
    u8 *buffer;
    //u8 mbrSwitch = 0; // 0 = SD card, 1 = usb device
    s8 err;
    s32 idx;

    // parameters Initilization
    *pEnt1SecStart = *pEnt1SecNum = 0;

    // Get the device info
    idx = _FS_LB_GetDriverIndex(pDriver);
    if(idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    if(!strncmp(FS__pDevInfo[idx].devname, "usbfs", 5))
    {
        //mbrSwitch = 1;
    }
    else
        return 0;

    //===================================================//
    DEBUG_FS2("FSysType = %d\n", mediaInfo->fsystype);
    buffer = (u8 *) FS__fat_malloc(FS_FAT_SEC_SIZE);
    if(!buffer)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
#if 0	// the procedure here is to check disk has any fine MBR table we can use.
    // read the first sector data to know that is MBR or BPB
    err = FS__lb_sin_read(pDriver, Unit, 0, (void*)buffer);
    if(err < 0)
    {
        ERRD(FS_REAL_SEC_READ_ERR);
        FS__fat_free(buffer);
        return err;
    }
    // MBR or Partition entry exist
    if((buffer[MBR_I_BOOT_SIGNATURE] == MBR_V_BOOT_SIGN_0x55) && (buffer[MBR_I_BOOT_SIGNATURE + 1] == MBR_V_BOOT_SIGN_0xAA))
    {
        // MBR partition table don't has "FAT" string and "0200" at the same time, it also means there it's has partition table exist.
        if((buffer[BPB_I_BYTES_PER_SEC] == 0x00) && (buffer[BPB_I_BYTES_PER_SEC + 1] == 0x02) && (buffer[BPB_I_FAT1216_FI_SYS_TYPE] == 0x46) &&
                (buffer[BPB_I_FAT1216_FI_SYS_TYPE + 1] == 0x41) && (buffer[BPB_I_FAT1216_FI_SYS_TYPE + 2] == 0x54))
        {
            if(!mbrSwitch)
                return 0;
        }
        else if((buffer[BPB_I_BYTES_PER_SEC] == 0x00) && (buffer[BPB_I_BYTES_PER_SEC + 1] == 0x02) && (buffer[BPB_I_FAT32_FI_SYS_TYPE] == 0x46) &&
                (buffer[BPB_I_FAT32_FI_SYS_TYPE + 1] == 0x41) && (buffer[BPB_I_FAT32_FI_SYS_TYPE + 2] == 0x54))
        {
            if(!mbrSwitch)
                return 0;
        }
        else	// use the existed partition
        {
            *pEnt1SecStart = (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 3]) << 24) |
                             (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 2]) << 16) |
                             (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC + 1]) << 8) |
                             ((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC]);
            *pEnt1SecNum = (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_NUM_SEC + 3]) << 24) |
                           (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_NUM_SEC + 2]) << 16) |
                           (((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_NUM_SEC + 1]) << 8) |
                           ((u8) buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_NUM_SEC]);
            // unually, MBR used in large size harddrive disk
            mediaInfo->totsec32 = *pEnt1SecNum;
            DEBUG_FS("MBR establish.\n");
            return 1;
        }

    }
    else	// Other devices don't need the partition entry in MBR table
    {
        if(!mbrSwitch)
            return 0;
    }
#endif

    // prepare the MBR infomation
    memset(buffer, 0x0, FS_FAT_SEC_SIZE);
    // update pointer value
    *pEnt1SecStart = 0x800;
    *pEnt1SecNum = GetTotalBlockCount(idx, Unit);	// Source from the usb_storage
    *pEnt1SecNum = (*pEnt1SecNum - 0x800 * 2) & 0xFFFFFF00;
    mediaInfo->totsec32 = *pEnt1SecNum;
    // Prepare the MBR value, and make the partition 1 start at the sector 2048
    // Reserved about 2~3 MB including the MBR space
    // Start sector address: LBA 2048 = (0*255 + 32)*63 + 33 - 1
    // Numbers of partition 1: (Totla sector - 2048 * 2) & 0xFFFFFF00
    buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_STATUS] = 0x0;	// Only two type, 0x80 or 0x0
    buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_FIRST_SEC] = 0x20; // Head value = 32
    buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_FIRST_SEC + 1] = 0x21; // Sector value = 33
    buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_FIRST_SEC + 2] = 0x0;	// Cylinder value = 0
    buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_TYPE] = 0x0B; // FAT32 with CHS addressing
    if(*pEnt1SecNum >= 0xFAC53F)	// 0xFAC53F = 255 * 63 * 1023, CHS maximum addressing size
    {
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC] = 0xFE;	// Maximum is 0xFE, 0 ~ 254
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC + 1] = 0xFF;	// Maximum is 0x3F, 1 ~ 63
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC + 2] = 0xFF;	// Maximum is 0x3FF, 0 ~ 1023
    }
    else
    {
        u32 c, h, s;
        c = *pEnt1SecNum / (0x3EC1);	// 0x3F * 0xFF
        h = (*pEnt1SecNum / 0x3F) % 0xFF;
        s = (((*pEnt1SecNum % 0x3F) + 1) & 0x3f) & ((c >> 2) & 0xC0);
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC] = h & 0xFF;
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC + 1] = s & 0xFF;
        buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_CHS_ADDR_LAST_SEC + 2] = c & 0xFF;
    }
    memcpy(&buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_FIRST_SEC], pEnt1SecStart, sizeof(u32));
    memcpy(&buffer[MBR_I_PARTI_ENTRY_01 + MBR_O_PARTI_ENT_NUM_SEC], pEnt1SecNum, sizeof(u32));

    buffer[MBR_I_BOOT_SIGNATURE] = MBR_V_BOOT_SIGN_0x55;
    buffer[MBR_I_BOOT_SIGNATURE + 1] = MBR_V_BOOT_SIGN_0xAA;

    err = FS__lb_sin_write(pDriver, Unit, 0, (void *)buffer);
    if(err < 0)
    {
        ERRD(FS_REAL_SEC_WRTIE_ERR);
        FS__fat_free(buffer);
        return err;
    }
    return 1;
}



/*********************************************************************
*
*             _FS_fat_format
*
  Description:
  FS internal function. Format a media using specified parameters.
  Currently this function needs many parameters. The function will be
  improved.

  Parameters:
  pDriver     - Pointer to a device driver function table.
  Unit        - Unit number.
  SecPerClus  - Number of sector per allocation unit.
  RootEntCnt  - For FAT12/FAT16, this is the number of 32 byte root
                directory entries. 0 for FAT32.
  TotSec16    - 16-bit total count of sectors. If zero, TotSec32 must
                be none-zero.
  TotSec32    - 32-bit total count of sectors. If zero, TotSec16 must
                be none-zero.
  Media       - Media byte.
  FATSz16     - 16-bit count of sectors occupied by one FAT. 0 for
                FAT32 volumes, which use FATSz32.
  FATSz32     - 32-bit count of sectors occupied by one FAT. This is
                valid for FAT32 medias only.
  SecPerTrk   - Sectors per track.
  NumHeads    - Number of heads.
  HiddSec     - Count of hidden sectors preceding the partition.
  FSysType    - ==0 => FAT12
                ==1 => FAT16
                ==2 => FAT32

  Return value:
  >=0         - Media has been formatted.
  <0          - An error has occured.
*/

int _FS_fat_format(int Idx, u32 Unit, u32 *pEnt1SecStart, u32 *pEnt1SecNum)
{
    FS__FAT_BPB *pBPBUnit;
    u32 StartSectorOfVolume;
    u32 SectorSize, CurSector, TmpVal;
    u32 Dividend, Divisor;
    int ret, i;
    u8 *pClustDataBuf, Times;
    char *pMemCache;

    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    if((pMemCache = FS__fat_malloc(FS_FAT_SEC_SIZE)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }
    //
    Times = 1;
    StartSectorOfVolume = 0;
    memset(pMemCache, 0x0, FS_FAT_SEC_SIZE);
    memset(pBPBUnit, 0x0, sizeof(FS__FAT_BPB));
    //
    if(*pEnt1SecStart)
        StartSectorOfVolume = *pEnt1SecStart;
    if(!(*pEnt1SecNum))
        *pEnt1SecNum = GetTotalBlockCount(Idx, Unit);

    do
    {
        pBPBUnit->BytesPerSec = FS_V_FAT_SECTOR_SIZE;
        pBPBUnit->SecPerClus = FS_V_FAT_CLUSTER_NUMBER / Times;
        pBPBUnit->RsvdSecCnt = FS_V_FAT_RSVD_SECTOR;
        pBPBUnit->NumFATs = 0x2;

        pBPBUnit->RootEntCnt = 0x0;
        pBPBUnit->TotSec16 = 0x0;
        pBPBUnit->MediaDesc = 0xf8;
        pBPBUnit->FATSz16 = 0x0;
        pBPBUnit->TotSec32 = *pEnt1SecNum;
        //
        // TotSec32 >= RsvdSecCnt + NumFATs * FATSz32 + (FS_V_FAT_SECTOR_SIZE / sizeof(u32) * SecPerClus)
        Dividend = pBPBUnit->TotSec32 - pBPBUnit->RsvdSecCnt;
        Divisor = (FS_V_FAT_SECTOR_SIZE / sizeof(u32)) * pBPBUnit->SecPerClus + pBPBUnit->NumFATs;
        if(Dividend % Divisor)
            pBPBUnit->FATSz32 = Dividend / Divisor + 1;
        else
            pBPBUnit->FATSz32 = Dividend / Divisor;
        pBPBUnit->ExtFlags = 0x0;
        pBPBUnit->RootClus = 0x2;
        pBPBUnit->FSInfo = 0x1;
        pBPBUnit->FatEndSec = pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz32;
        pBPBUnit->RootDirSec = pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz32 * pBPBUnit->NumFATs;
        pBPBUnit->LimitsOfCluster = (pBPBUnit->TotSec32 - pBPBUnit->RootDirSec) / pBPBUnit->SecPerClus;

        TmpVal = pBPBUnit->TotSec32 - (pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz32 * pBPBUnit->NumFATs);
        TmpVal /= pBPBUnit->SecPerClus;

        pBPBUnit->SizeOfCluster = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
        pBPBUnit->BitRevrOfBPS = pBPBUnit->BytesPerSec - 1;
        for(i = 0; i < 32; i++)
        {
            if((1 << i) & pBPBUnit->BytesPerSec)
                pBPBUnit->BitNumOfBPS = i;
            if((1 << i) & pBPBUnit->SecPerClus)
                pBPBUnit->BitNumOfSPC = i;
            if((1 << i) & pBPBUnit->SizeOfCluster)
                pBPBUnit->BitNumOfSOC = i;
        }

        if(Times == 0x8)
        {
            DEBUG_FS("[ERR] The size of storage is too small.\n");
            FS__fat_free(pMemCache);
            return -1;
        }
        Times++;
    }
    while(TmpVal < 65525);

    // Prepare buffer with information of the BPB offset 0 - 35 is same for FAT12/FAT16 and FAT32
    // jmpBoot
    pMemCache[0] = 0xEB;
    pMemCache[1] = 0x3C;
    pMemCache[2] = 0x90;
    // OEMName = '		  '
    memset(&pMemCache[3], 0x20, 11);
    // BytesPerSec
    memcpy(&pMemCache[11], &pBPBUnit->BytesPerSec, sizeof(u16));
    // SecPerClus
    memcpy(&pMemCache[13], &pBPBUnit->SecPerClus, sizeof(u8));
    // RsvdSecCnt
    memcpy(&pMemCache[14], &pBPBUnit->RsvdSecCnt, sizeof(u16));
    // NumFATs
    memcpy(&pMemCache[16], &pBPBUnit->NumFATs, sizeof(u8));
    // RootEntCnt
    memcpy(&pMemCache[17], &pBPBUnit->RootEntCnt, sizeof(u16));
    // TotSec16
    memcpy(&pMemCache[19], &pBPBUnit->TotSec16, sizeof(u16));
    // Media
    memcpy(&pMemCache[21], &pBPBUnit->MediaDesc, sizeof(u8));
    // FATSz16
    memcpy(&pMemCache[22], &pBPBUnit->FATSz16, sizeof(u16));
    if(pBPBUnit->TotSec32 > (0x3F * 0xFF))
    {
        // SecPerTrk
        memset(&pMemCache[24], 0x3F, sizeof(u8));
        // NumHeads
        memset(&pMemCache[26], 0xFF, sizeof(u8));
    }
    else
    {
        // SecPerTrk
        memset(&pMemCache[24], 0x3F, sizeof(u8));
        // NumHeads
        memset(&pMemCache[26], (pBPBUnit->TotSec32 / 0x3F) & 0xFF, sizeof(u8));
    }
    // Start Sector Of Volume
    memcpy(&pMemCache[28], &StartSectorOfVolume, sizeof(u32));
    // TotSec32
    memcpy(&pMemCache[32], &pBPBUnit->TotSec32, sizeof(u32));
    // FATSz32
    memcpy(&pMemCache[36], &pBPBUnit->FATSz32, sizeof(u32));
    // EXTFlags
    memcpy(&pMemCache[40], &pBPBUnit->ExtFlags, sizeof(u16));
    // FSVer = 0:0
    //memset(pMemCache[42], 0x0, sizeof(u16));
    // RootClus
    memcpy(&pMemCache[44], &pBPBUnit->RootClus, sizeof(u32));
    // FSInfo
    memcpy(&pMemCache[48], &pBPBUnit->FSInfo, sizeof(u16));
    // Backup Boot Sec
    memset(&pMemCache[50], 0x6, sizeof(u8));
    // Reserved
    //memset(pMemCache[52], 0x0, 12);
    // DrvNum
    //memset(pMemCache[64], 0x0, 1);
    // Reserved1
    //memset(pMemCache[65], 0x0, 1);
    // BootSig
    //memset(pMemCache[66], 0x0, 1);
    // VolID
    //memset(pMemCache[67], 0x0, sizeof(u32));
    // VolLab =
    memcpy(&pMemCache[71], "           ", 11);
    // Fil System Type
    memcpy(&pMemCache[82], "FAT32   ", 8);
    // Signature = 0xAA55
    pMemCache[MBR_I_BOOT_SIGNATURE] = MBR_V_BOOT_SIGN_0x55;
    pMemCache[MBR_I_BOOT_SIGNATURE + 1] = MBR_V_BOOT_SIGN_0xAA;

    // Write BPB to media
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume, pMemCache)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FS__fat_free(pMemCache);
        return ret;
    }
    // Write backup BPB
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume + 0x6, pMemCache)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FS__fat_free(pMemCache);
        return ret;
    }

    // Init FAT 1 & 2
    if((pClustDataBuf = FSMalloc(pBPBUnit->SizeOfCluster)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        FS__fat_free(pMemCache);
        return -1;
    }
    memset(pClustDataBuf, 0x0, pBPBUnit->SizeOfCluster);
    // Clean FAT1
    SectorSize = pBPBUnit->FATSz32;
    SectorSize = (SectorSize % pBPBUnit->SecPerClus)? ((SectorSize / pBPBUnit->SecPerClus) + 1) * pBPBUnit->SecPerClus: SectorSize;
    CurSector = StartSectorOfVolume + pBPBUnit->RsvdSecCnt;
    do
    {
        if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_WRITE_DAT_ERR);
            FSFree(pClustDataBuf);
            FS__fat_free(pMemCache);
            return -1;
        }
        SectorSize -= pBPBUnit->SecPerClus;
        CurSector += pBPBUnit->SecPerClus;
    }
    while(SectorSize);
    // Clean FAT2
    SectorSize = pBPBUnit->FATSz32;
    SectorSize = (SectorSize % pBPBUnit->SecPerClus)? ((SectorSize / pBPBUnit->SecPerClus) + 1) * pBPBUnit->SecPerClus: SectorSize;
    CurSector = StartSectorOfVolume + pBPBUnit->RsvdSecCnt + pBPBUnit->FATSz32;
    do
    {
        if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, CurSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
        {
            ERRD(FS_LB_MUL_WRITE_DAT_ERR);
            FSFree(pClustDataBuf);
            FS__fat_free(pMemCache);
            return -1;
        }
        SectorSize -= pBPBUnit->SecPerClus;
        CurSector += pBPBUnit->SecPerClus;
    }
    while(SectorSize);

    // Prepare FAT1
    memset(pMemCache, 0x0, pBPBUnit->BytesPerSec);
    *(u32 *)(pMemCache) = 0x0FFFFFF8;
    *(u32 *)(pMemCache + 4) = 0x0FFFFFFF;
    *(u32 *)(pMemCache + 8) = 0x0FFFFFFF;	// Root directroy cluster

    // Write back to FAT1
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume + pBPBUnit->RsvdSecCnt, pMemCache)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FSFree(pClustDataBuf);
        FS__fat_free(pMemCache);
        return -1;
    }
    // Write back to FAT2
    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume + pBPBUnit->FatEndSec, pMemCache)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FSFree(pClustDataBuf);
        FS__fat_free(pMemCache);
        return -1;
    }

    // Clean Root directory
    if((ret = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume + pBPBUnit->RootDirSec, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
    {
        ERRD(FS_LB_MUL_WRITE_DAT_ERR);
        FSFree(pClustDataBuf);
        FS__fat_free(pMemCache);
        return -1;
    }
    FSFree(pClustDataBuf);

    // Prepare Disk FSInfo
    memset(pMemCache, 0x0, pBPBUnit->BytesPerSec);
    // LeadSig = 0x41615252
    *(u32 *)(pMemCache + FAT_FSIS_I_SEC_SIGNATURE) = 0x41615252;
    // StructSig = 0x61417272
    *(u32 *)(pMemCache + FAT_FSIS_I_SEC_SIGNATURE2) = 0x61417272;
    // Invalidate last known free cluster count
    *(u32 *)(pMemCache + FAT_FSIS_I_FREE_CLUS_COUNT) = pBPBUnit->LimitsOfCluster - 3;	// 3 = Original(2) + Root Dir
    // Give hint for free cluster search
    *(u32 *)(pMemCache + FAT_FSIS_I_NEXT_FREE_CLUS) = pBPBUnit->RootClus + 1;
    // TrailSig = 0xaa550000
    *(u32 *)(pMemCache + FAT_FSIS_I_BOOT_SIGNATURE) = 0xAA550000;

    if((ret = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, StartSectorOfVolume + pBPBUnit->FSInfo, pMemCache)) < 0)
    {
        ERRD(FS_LB_WRITE_DAT_ERR);
        FS__fat_free(pMemCache);
        return -1;
    }

    // Reset capacity Info
    FS__pDevInfo[Idx].TagOfFirstFreeClust = 0x0;
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount = pBPBUnit->LimitsOfCluster - 3;
    FS__pDevInfo[Idx].FSInfo.NextFreeCluster = pBPBUnit->RootClus + 1;

    // Reset playback cache
    FSPlaybackCacheBufferReset();

    // Restore Partion 1 Sector Info
    pBPBUnit->RsvdSecCnt += StartSectorOfVolume;
    pBPBUnit->FSInfo += StartSectorOfVolume;
    pBPBUnit->FatEndSec += StartSectorOfVolume;
    pBPBUnit->RootDirSec += StartSectorOfVolume;

    FS__fat_free(pMemCache);
    return 1;
}

#endif /* FS_FAT_NOFORMAT==0 */


#if FS_FAT_DISKINFO

/*********************************************************************
*
*             _FS_fat_GetTotalFree
*
  Description:
  FS internal function. Store information about used/unused clusters
  in a FS_DISKFREE_T data structure.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  pDiskData   - Pointer to a FS_DISKFREE_T data structure.

  Return value:
  ==0         - Information is stored in pDiskData.
  <0          - An error has occured.
*/

static int _FS_fat_GetTotalFree(int Idx, u32 Unit, FS_DISKFREE_T *pDiskData, int Simplified)
{
    FS__FAT_BPB *pBPBUnit;
    u32 UsedCluster, UnusedCluster;
    u32 FATIndex, FATSector, FATOffset, LastFATSector;
    u32 CurCluster, EndCluster, ClusterSize, Times, TmpVal;
    int ret;
    u16 *pShortVal;
    u8 *pClustDataBuf, MoveStep, LastPercnet;
    //
    pBPBUnit = &FS__FAT_aBPBUnit[Idx][Unit];
    ClusterSize = pBPBUnit->BytesPerSec * pBPBUnit->SecPerClus;
    Times = 1;
    LastFATSector = 0xffffffff;
    EndCluster = pBPBUnit->LimitsOfCluster;
    CurCluster = 0;
    UsedCluster = 0;
    UnusedCluster = 0;
    LastPercnet = 101;
    if((pClustDataBuf = FSMalloc(ClusterSize)) == NULL)
    {
        ERRD(FS_MEMORY_ALLOC_ERR);
        return -1;
    }

    if(pBPBUnit->FATType == 1)	// FAT12
    {
        do
        {
            FATIndex = CurCluster + CurCluster / 2;
            FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
            FATOffset = FATIndex % pBPBUnit->BytesPerSec;

            if(LastFATSector != FATSector)
            {
                if((LastFATSector != 0xffffffff) && !Simplified)	// 1: After format, we don't scan all the storage spaces
                {
                    UnusedCluster = EndCluster - UsedCluster;
                    break;
                }
                if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pClustDataBuf)) < 0)
                {
                    ERRD(FS_LB_READ_DAT_ERR);
                    FSFree(pClustDataBuf);
                    return ret;
                }
                LastFATSector = FATSector;
            }

            if(FATOffset == pBPBUnit->BytesPerSec - 1)
            {
                TmpVal = *(pClustDataBuf + FATOffset);
                if((ret = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector + 1, pClustDataBuf)) < 0)
                {
                    ERRD(FS_LB_READ_DAT_ERR);
                    FSFree(pClustDataBuf);
                    return ret;
                }
                LastFATSector = FATSector + 1;
                TmpVal |= *(pClustDataBuf) << 8;
            }
            else
            {
                memcpy(&TmpVal, pClustDataBuf + FATOffset, sizeof(u16));
            }
            if(CurCluster & 1)
                TmpVal >>= 4;
            TmpVal &= 0xfff;
            if(TmpVal)
                UsedCluster++;
            else
                UnusedCluster++;
            CurCluster++;

            TmpVal = CurCluster / (EndCluster / 100);
            if(!(TmpVal % 5) && LastPercnet != TmpVal && CurCluster)
            {
                LastPercnet = TmpVal / 5 * 5;
                DEBUG_FS("[INF] Scan free cluster: %d/100.\n", TmpVal);
            }
        }
        while(CurCluster < EndCluster);
    }
    else if(pBPBUnit->FATType == 0) // FAT16
    {
        MoveStep = sizeof(u16);
        do
        {
            FATIndex = CurCluster * MoveStep;
            FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
            FATOffset = FATIndex % pBPBUnit->BytesPerSec;

            if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
            {
                if((LastFATSector != 0xffffffff) && !Simplified)	// 1: After format, we don't scan all the storage spaces
                {
                    UnusedCluster = EndCluster - UsedCluster;
                    break;
                }
                if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
                {
                    ERRD(FS_LB_MUL_READ_DAT_ERR);
                    FSFree(pClustDataBuf);
                    return ret;
                }
                LastFATSector = FATSector;
            }

            for(pShortVal = (u16 *)((u8 *)pClustDataBuf + FATOffset);
                    ((u8 *)pShortVal < (pClustDataBuf + ClusterSize)) && (CurCluster < EndCluster);)
            {
                if(*pShortVal++)
                    UsedCluster++;
                else
                    UnusedCluster++;
                CurCluster++;
            }
            if(!(CurCluster % (0x10000 / Times)) && CurCluster)
            {
                //DEBUG_GREEN("[INF] (Used+Unused)/TotalClust: %#x/%#x\n", UsedCluster + UnusedCluster, EndCluster);
#if ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
                OSTimeDly(1);
#endif
            }

            TmpVal = CurCluster / (EndCluster / 100);
            if(!(TmpVal % 5) && LastPercnet != TmpVal && CurCluster)
            {
                LastPercnet = TmpVal / 5 * 5;
                DEBUG_FS("[INF] Scan free cluster: %d/100.\n", TmpVal);
            }
        }
        while(CurCluster < EndCluster);
    }
    else	// FAT32
    {
        MoveStep = sizeof(u32);
        do
        {
            FATIndex = CurCluster * MoveStep;
            FATSector = pBPBUnit->RsvdSecCnt + (FATIndex / pBPBUnit->BytesPerSec);
            FATOffset = FATIndex % pBPBUnit->BytesPerSec;

            if((FATSector >= (LastFATSector + pBPBUnit->SecPerClus)) || (FATSector < LastFATSector))
            {
                if((LastFATSector != 0xffffffff) && !Simplified)	// 1: After format, we don't scan all the storage spaces
                {
                    UnusedCluster = EndCluster - UsedCluster;
                    break;
                }
                if((ret = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver, Unit, FATSector, pBPBUnit->SecPerClus, pClustDataBuf)) < 0)
                {
                    ERRD(FS_LB_MUL_READ_DAT_ERR);
                    FSFree(pClustDataBuf);
                    return ret;
                }
                LastFATSector = FATSector;
            }

            // Use MPCU function till cluster less than 1024
            if(((EndCluster - CurCluster) < (ClusterSize / MoveStep)) && (Times < 8))
            {
                Times *= 2;
                //DEBUG_MAGENTA("[INF] Times: %#d\n", Times);
            }

            if(EndCluster - CurCluster >= (ClusterSize / MoveStep / Times))
            {
                TmpVal = mcpu_FATZeroScan(pClustDataBuf, ClusterSize / Times);	// Only 32 bit mode can use.
                UnusedCluster += TmpVal;
                UsedCluster += ClusterSize / MoveStep / Times - TmpVal;
                CurCluster += ClusterSize / MoveStep / Times;
                //DEBUG_YELLOW("[INF] CurCluster: %#x\n", CurCluster);
            }
            else
            {
                // The cluster fetch from FAT1 of FAT32
                if(*(u32 *)((u8 *)pClustDataBuf + FATOffset))
                    UsedCluster++;
                else
                    UnusedCluster++;
                CurCluster++;
            }
            if(!(CurCluster % (0x10000 / Times)) && CurCluster)
            {
                //DEBUG_GREEN("[INF] (Used+Unused)/TotalClust: %#x/%#x\n", UsedCluster + UnusedCluster, EndCluster);
#if ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
                OSTimeDly(1);
#endif
            }

            TmpVal = CurCluster / (EndCluster / 100);
            if(!(TmpVal % 5) && LastPercnet != TmpVal && CurCluster)
            {
                LastPercnet = TmpVal / 5 * 5;
                DEBUG_FS("[INF] Scan free cluster: %d/100.\n", TmpVal);
            }
        }
        while(CurCluster < EndCluster);
    }

    DEBUG_FS("[INF] Sum of The Storage Space: %#x, %#x, %#x\n", EndCluster, UsedCluster, UnusedCluster);

    pDiskData->total_clusters = EndCluster;
    pDiskData->avail_clusters = UnusedCluster;
    pDiskData->sectors_per_cluster = pBPBUnit->SecPerClus;
    pDiskData->bytes_per_sector = pBPBUnit->BytesPerSec;

    FS__pDevInfo[Idx].FSInfo.FreeClusterCount = UnusedCluster;	// Update UnusedCluster number

    FSFree(pClustDataBuf);
    return 1;
}


#endif /* FS_FAT_DISKINFO */


/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__fat_ioctl
*
  Description:
  FS internal function. Execute device command. The FAT layer checks
  first, if it has to process the command (e.g. format). Any other
  command is passed to the device driver.

  Parameters:
  Idx         - Index of device in the device information table
                referred by FS__pDevInfo.
  Unit        - Unit number.
  Cmd         - Command to be executed.
  Aux         - Parameter depending on command.
  pBuffer     - Pointer to a buffer used for the command.

  Return value:
  Command specific. In general a negative value means an error.
*/

int FS__fat_ioctl(int Idx, u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
    struct
    {
        u32 hiddennum;
        u32 headnum;
        u32 secnum;
        u32 partsize;
    }
    devinfo;

    int err, x = 0;
#if(FILE_SYSTEM_SEL ==  FILE_SYSTEM_DVR)  // FAT adjustment
    u32 rootdirsize;
    u32 totalsec;
    u32 rest_sec;
    u32 estimate_asf_nums;
#endif

#if ((FS_SUPPORT_SEC_ACCESS) || (FS_FAT_NOFORMAT==0))
    int i;
#endif
#if (FS_FAT_NOFORMAT==0)
    int j;
#endif
    int nClusEntriesNoPerFatCluster = 0;
    int nReservedSize = 0;
    int nFSysType;

    err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  // Turn on busy signal
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }
#if (FS_FAT_NOFORMAT==0)
    //------------------------------Optimized Format Command--------------------------//
    if((Cmd == FS_CMD_FORMAT_MEDIA) || (Cmd == FS_CMD_FORMAT_FAST) )
    {
        j = 0;
        while (1)
        {
            if (j >= FS_KNOWNMEDIA_NUM)
            {
                break;  // Not a known media
            }
            if (_FS_wd_format_media_table[j].media_id == Aux)
            {
                break;  // Media found in the list
            }
            j++;
        }	//find the right media
        if (j >= FS_KNOWNMEDIA_NUM)
        {
            //can't find the right media
            err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
            if(err < 0)
                ERRD(FS_DEV_IOCTL_ERR);
            return -1;
        }
        i = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);	//get status of device, if ok return 1
        if (i >= 0)
        {
            if(Cmd == FS_CMD_FORMAT_MEDIA)
            {
                DEBUG_FS2("====> Device erase start!\n");
                i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FORMAT_MEDIA, 0, (void*)0); //call device driver to erase data.
                if(i < 0)
                {
                    DEBUG_FS("====> Device erase Failed!\n");
                    ERRD(FS_DEV_IOCTL_ERR);
                    return i;
                }
                DEBUG_FS("====> Device erase complete!\n");
            }

            if ( (Cmd == FS_CMD_FORMAT_FAST) || (Cmd==FS_CMD_FORMAT_MEDIA) )
            {
                u32 pEnt1SecStart = 0, pEnt1SecNum = 0;
                err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_GET_DEVINFO, 0, (void*)&devinfo); //get card information
                if(err < 0)
                {
                    ERRD(FS_DEV_IOCTL_ERR);
                    return err;	//?
                }

                if (_FS_wd_format_media_table[j].totsec32)
                {
                    _FS_wd_format_media_table[j].totsec32 = devinfo.partsize;
                }
                else /* (_FS_wd_format_media_table[j].totsec16) */
                {
                    _FS_wd_format_media_table[j].totsec16 = devinfo.partsize;
                }

                nFSysType = _FS_wd_format_media_table[j].fsystype;
                if (nFSysType == 1) //FAT16
                    nClusEntriesNoPerFatCluster = 256;  //512/2=256 entry/sector
                else if (nFSysType == 2) //FAT32,
                    nClusEntriesNoPerFatCluster = 128; //3                                                 // -->512/4=128 entry/sector
                else //FAT12
                    DEBUG_FS("FAT12\n");
                _FS_wd_format_media_table[j].fatsz16 = devinfo.partsize / (_FS_wd_format_media_table[j].secperclus * nClusEntriesNoPerFatCluster); // 算出FAT 所佔的sector數
                if (devinfo.partsize % (_FS_wd_format_media_table[j].secperclus * nClusEntriesNoPerFatCluster))
                    _FS_wd_format_media_table[j].fatsz16++;

                nReservedSize = 8 - (((u32)_FS_wd_format_media_table[j].fatsz16 << 30) >> 29); //??

                if (nFSysType == 2)
                {
                    if (nReservedSize== 8)
                        nReservedSize = 32;
                    else
                        nReservedSize += 32;
                }
                err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FLUSH_CACHE, 0, (void*)0);
                if(err < 0)
                {
                    ERRD(FS_DEV_IOCTL_ERR);
                    return err;	//?
                }
                _FS_wd_format_media_table[j].usReservedSize = nReservedSize;
                DEBUG_FS2("====> FS MBR format start.\n");
                x = _FS_MBR_format(FS__pDevInfo[Idx].devdriver, Unit, &_FS_wd_format_media_table[j], &pEnt1SecStart, &pEnt1SecNum);
                if(x < 0)
                {
                    DEBUG_FS("[ERR] MBR format failed.\n");
                    return -1;
                }
                DEBUG_FS2("====> FS_fat_format start: 0x%x,0x%x,%d\n",_FS_wd_format_media_table[j].totsec32,_FS_wd_format_media_table[j].fatsz16,nReservedSize);
                x = _FS_fat_format(Idx, Unit, &pEnt1SecStart, &pEnt1SecNum);
                if(x < 0)
                {
                    DEBUG_FS("[ERR] fat format failed.\n");
                    return -1;
                }
                DEBUG_FS("====> FS fat format compete!\n");
            }

            i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FLUSH_CACHE, 0, (void*)0);
            if (i < 0)
            {
                ERRD(FS_DEV_IOCTL_ERR);
                return err;
            }
            else
            {
                // Invalidate BPB
                if ( (Cmd == FS_CMD_FORMAT_FAST) || (Cmd==FS_CMD_FORMAT_MEDIA) )
                {
                    for (i = 0; i < (int)FS__maxdev; i++)
                    {
                        for (j = 0; j < (int)FS__fat_maxunit; j++)
                        {
                            FS__FAT_aBPBUnit[i][j].Signature = 0x0000;
                        }
                    }
                }
            }
        }
        else
        {
            if(FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0) < 0)	// Turn off busy signal
                ERRD(FS_DEV_IOCTL_ERR);
            ERRD(FS_DEV_ACCESS_ERR);
            return i;
        }
    }

#else /* FS_FAT_NOFORMAT==0 */
    if ( (Cmd == FS_CMD_FORMAT_FAST) || (Cmd == FS_CMD_FORMAT_MEDIA))
    {
        x = -1;  /* Format command is not supported */
    }
#endif /* FS_FAT_NOFORMAT==0 */

    //---------- 統計目前device 使用情況----------//
#if FS_FAT_DISKINFO
    else if (Cmd == FS_CMD_GET_DISKFREE)
    {
        i = FS__fat_checkunit(Idx, Unit);
        if (i >= 0)
        {
            DEBUG_FS2("GetTotalFree start!\n");
            x = _FS_fat_GetTotalFree(Idx, Unit, (FS_DISKFREE_T*)pBuffer, Aux);
            DEBUG_FS2("GetTotalFree compete!\n");
        }
        else
        {
            ERRD(FS_DEV_UNIT_CHECK_ERR);
            x = i;
        }
    }
#else // FS_FAT_DISKINFO==0
    else if (Cmd == FS_CMD_GET_DISKFREE)
    {
        x = -1; // Diskinfo command not supported
    }
#endif // FS_FAT_DISKINFO
    //-----------------------------//
    else if(Cmd ==FS_CMD_CHECK_UNIT)
    {
        i = FS__fat_checkunit(Idx, Unit);
        if(i >= 0)
        {
            DEBUG_FS2("======> FS_CMD_CHECK_UNIT is OK!\n");
            x = 0;
        }
        else
        {
            ERRD(FS_DEV_UNIT_CHECK_ERR);
            x = i;
        }
        global_diskInfo.sectors_per_cluster=FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        global_diskInfo.bytes_per_sector=FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    }
    //-------Read one Sector-------//
#if FS_SUPPORT_SEC_ACCESS
    else if ((Cmd == FS_CMD_READ_SECTOR) || (Cmd == FS_CMD_WRITE_SECTOR))
    {
        if (!pBuffer)
        {
            err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
            if(err < 0)
                ERRD(FS_DEV_IOCTL_ERR);
            return -1;
        }
        i = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);
        if (i >= 0)
        {
            if (Cmd == FS_CMD_READ_SECTOR)
            {
                x = FS__lb_sin_read(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
                if(x < 0)
                    ERRD(FS_REAL_SEC_READ_ERR);

            }
            else
            {
                x = FS__lb_sin_write(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
                if(x < 0)
                    ERRD(FS_REAL_SEC_WRTIE_ERR);
            }
        }
        else
        {
            ERRD(FS_DEV_ACCESS_ERR);
            x = i;
        }
    }
#else // FS_SUPPORT_SEC_ACCESS 
    else if ((Cmd == FS_CMD_READ_SECTOR) || (Cmd == FS_CMD_WRITE_SECTOR))
    {
        err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
        if(err < 0)
            ERRD(FS_DEV_IOCTL_ERR);
        return -1;
    }
#endif // FS_SUPPORT_SEC_ACCESS
    else
    {
        // Maybe command for driver
        x = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, Cmd, Aux, (void*)pBuffer);
        if(x < 0)
            ERRD(FS_DEV_IOCTL_ERR);
    }
    err = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  // Turn off busy signal
    if(err < 0)
    {
        ERRD(FS_DEV_IOCTL_ERR);
        return -1;	//?
    }

    return x;
}

