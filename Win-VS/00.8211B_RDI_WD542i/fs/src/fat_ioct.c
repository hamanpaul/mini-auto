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
#include "smcapi.h"

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
    FS_i32  media_id;
    FS_u32  totsec32;
    FS_u32  hiddsec;
    FS_u16  totsec16;
    FS_u16  rootentcnt;
    FS_u16  fatsz16;
    FS_u16  secpertrk;
    FS_u16  numheads;
    char    secperclus;
    char    media;
    char    fsystype;
    FS_u16 usReservedSize;
}
_FS_wd_format_media_type;


typedef struct
{
    FS_u32 SecNum;
    FS_u32 Num;
}
_FS_FAT_ROOTENTCNT ;


typedef struct
{
    FS_u32 SecNum;
    FS_u16 Num;
}
_FS_FAT_SECPERCLUST;


/*********************************************************************
*
*             Global Variables
*
**********************************************************************/
#if(FILE_SYSTEM_SEL ==  FILE_SYSTEM_DVR)  // FAT adjustment
FS_u32 estimate_dir_nums=2;
#endif

/*********************************************************************
*
*             Extern Global Variables
*
**********************************************************************/
extern unsigned char  gInsertNAND; //civic 071002
extern FS_DISKFREE_T global_diskInfo;
extern u8 *PKBuf;
extern char strVolName[];
extern char *cMulBlockBuffer;

/*********************************************************************
*
*             Extern Functions
*
**********************************************************************/
extern signed long dcfDriveInfo(FS_DISKFREE_T*);


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
    { FS_MEDIA_RAM_128KB, 0x00000000UL, 0x00000000UL, 0x0100,   0x0080,  			0x0001,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
    { FS_MEDIA_RAM_256KB, 0x00000000UL, 0x00000000UL, 0x0200,   0x0080,  			0x0002,  0x0004,    0x0004,   	0x01,       	(char) 0xf8,  0,        1 },
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

    //==Compact Flash==//
    /*  media_id            totsec32      hidsec        totsec16  rootent  			fatsz16  secpertrk  numheads    secperclus  		  media   fstype    reserved size*/
    { FS_MEDIA_CF_32MB,   0x00000000UL, 0x00000020UL, 0xf760,   0x0200,  			0x007c,  0x0020,    0x0004,   	0x02,           (char) 0xf8,  1,        1 },
    { FS_MEDIA_CF_64MB,   0x0001e860UL, 0x00000020UL, 0x0000,   0x0200,  			0x007b,  0x0020,    0x0004,   	0x04,           (char) 0xf8,  1,        1 }
};


/* table for getting number of root entries for a given media size */
static const _FS_FAT_ROOTENTCNT _FS_auto_rootcnt[] =
{
    {
        0x100,     0x40
    },
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


/*CY 0718*/
#if FS_NEW_VERSION
int _FS_fat_format(int Idx, u32 Unit, char SecPerClus,
                   FS_u16 RootEntCnt, FS_u16 TotSec16, FS_u32 TotSec32, char Media,
                   FS_u16 FATSz16, FS_u32 FATSz32, FS_u16 SecPerTrk,FS_u16 NumHeads,
                   FS_u32 HiddSec, char FSysType, FS_u16 usReservedSize)
{
    FS__FAT_BPB bpb;
    FS_u32 sector;
    FS_u32 value;
    char *buffer;
    int i;
    int j;
    int n;
    int err;
    unsigned int partitionStart = 0;
    unsigned int partitionSize = 0;
    int totalfatsec,numOfSector,fatsec;
    //===================================================//

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }

    /* cytsai: check Master Boot Record */
    /* read first sector and check if it's an MBR */
#if FS_RW_DIRECT
    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
#else
    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, 0, (void*)buffer);
#endif
    if (err < 0)
    {
        FS__fat_free(buffer);
        return -1;
    }
    /* check if it's a MBR */
    if ((buffer[510] == 0x55) && (buffer[511] == 0xaa))
    {
        /* check if it's a PBS; i.e. bytesPerSector == 0x0200 */
        if ((buffer[11] != 0x00) || (buffer[12] != 0x02))
        {
            /* it's a MBR */
            partitionStart = (((unsigned int)buffer[457]) << 24) |
                             (((unsigned int)buffer[456]) << 16) |
                             (((unsigned int)buffer[455]) <<  8) |
                             ((unsigned int)buffer[454]);
            partitionSize  = (((unsigned int)buffer[461]) << 24) |
                             (((unsigned int)buffer[460]) << 16) |
                             (((unsigned int)buffer[459]) <<  8) |
                             ((unsigned int)buffer[458]);

            if (TotSec16 == 0)
            {
                TotSec32 = partitionSize;
            }
            else
            {
                TotSec16 = (FS_u16)partitionSize;
            }
        }
    }

    /* Clear buffer */
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE); /* cytsai: FS__fat_malloc has already initialized the buffer to zero. */
    /* Setup BPB */
    FS__CLIB_memset(&bpb, 0x00, (FS_size_t)sizeof(bpb));
    bpb.BytesPerSec = 0x0200;		/* _512_,1024,2048,4096 		  */
    bpb.SecPerClus	= SecPerClus;	/* sec in allocation unit		  */

    if (FSysType != 2)
    {
//	  bpb.RsvdSecCnt  = 0x0001; 	  /* 1 for FAT12 & FAT16			*/
        bpb.RsvdSecCnt	= usReservedSize;		/* 1 for FAT12 & FAT16			  */
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        bpb.RsvdSecCnt	= usReservedSize;		/* 32 for FAT32 				  */
    }
#else
    /* FAT32 disabled */
    else
    {
        FS__fat_free(buffer);
        return -1;
    }
#endif /* FS_FAT_NOFAT32==0 */
    bpb.NumFATs 	= 0x02; 		/* 2							  */
    bpb.RootEntCnt	= RootEntCnt;	/* number of root dir entries	  */
    bpb.TotSec16	= TotSec16; 	/* RSVD+FAT+ROOT+DATA (<64k)	  */
    bpb.FATSz16 	= FATSz16;		/* number of FAT sectors		  */
    bpb.TotSec32	= TotSec32; 	/* RSVD+FAT+ROOT+FATA (>=64k)	  */
    bpb.Signature	= 0xaa55;		/* 0xAA55 Signature 			  */

    /* setup BPB specifics for FAT32 */
    bpb.FATSz32 	= FATSz32;		/* number of FAT sectors		  */
    bpb.ExtFlags	= 0x0000;		/* mirroring info				  */
    bpb.RootClus	= 0x00000002UL; /* root dir clus for FAT32		  */
    bpb.FSInfo		= 0x0001;		/* position of FSInfo structure   */

    /* Set the number of FAT sectors */
    if (FSysType != 2)
    {
        bpb.FATSz16 	= FATSz16;
        bpb.FATSz32 	= 0;
    }
    else
    {
        bpb.FATSz16 	= 0;
        bpb.FATSz32 	= FATSz16;
    }

    /*
       Prepare buffer with information of the BPB
       offset 0 - 35 is same for FAT12/FAT16 and FAT32
    */

    /* jmpBoot = 0xe9 0x0000 */
#if 0
    buffer[0]	= (char)0xe9;
    buffer[1]	= (char)0x00;
    buffer[2]	= (char)0x00;
#else
    buffer[0]	= (char)0xeb;
    buffer[1]	= (char)0x3c;
    buffer[2]	= (char)0x90;
#endif
    /* OEMName = '		  ' */
    for (i = 3; i < 11; i++)
    {
        buffer[i] = (char)0x20;
    }
    /* BytesPerSec */
    buffer[11]	= (char)(bpb.BytesPerSec & 0xff);
    buffer[12]	= (char)((unsigned int)(bpb.BytesPerSec & 0xff00) >> 8);
    /* SecPerClus */
    buffer[13]	= (char)bpb.SecPerClus;
    /* RsvdSecCnt */
    buffer[14]	= (char)(bpb.RsvdSecCnt & 0xff);
    buffer[15]	= (char)((unsigned int)(bpb.RsvdSecCnt & 0xff00) >> 8);
    /* NumFATs */
    buffer[16]	= (char)bpb.NumFATs;
    /* RootEntCnt */
    buffer[17]	= (char)(bpb.RootEntCnt & 0xff);
    buffer[18]	= (char)((unsigned int)(bpb.RootEntCnt & 0xff00) >> 8);
    /* TotSec16 */
    buffer[19]	= (char)(bpb.TotSec16 & 0xff);
    buffer[20]	= (char)((unsigned int)(bpb.TotSec16 & 0xff00) >> 8);
    /* Media */
    buffer[21]	= Media;
    /* FATSz16 */
    if (FSysType != 2)
    {
        buffer[22]	= (char)(bpb.FATSz16 & 0xff);
        buffer[23]	= (char)((unsigned int)(bpb.FATSz16 & 0xff00) >> 8);
    }
    else
    {
        /* FATSz16 = 0 when file system type = FAT32 */
        buffer[22]	= (char)0x00;
        buffer[23]	= (char)0x00;
    }
    /* SecPerTrk */
    buffer[24]	= (char)(SecPerTrk & 0xff);
    buffer[25]	= (char)((unsigned int)(SecPerTrk & 0xff00) >> 8);
    /* NumHeads */
    buffer[26]	= (char)(NumHeads & 0xff);
    buffer[27]	= (char)((unsigned int)(NumHeads & 0xff00) >> 8);
    /* HiddSec */
    //buffer[28]  = (char)(HiddSec & 0xff);
    //buffer[29]  = (char)((FS_u32)(HiddSec & 0x0000ff00UL) >> 8);
    //buffer[30]  = (char)((FS_u32)(HiddSec & 0x00ff0000UL) >> 16);
    //buffer[31]  = (char)((FS_u32)(HiddSec & 0xff000000UL) >> 24);
    buffer[28]	= (char)(partitionStart & 0xff);
    buffer[29]	= (char)((FS_u32)(partitionStart & 0x0000ff00UL) >> 8);
    buffer[30]	= (char)((FS_u32)(partitionStart & 0x00ff0000UL) >> 16);
    buffer[31]	= (char)((FS_u32)(partitionStart & 0xff000000UL) >> 24);
    /* TotSec32 */
    buffer[32]	= (char)(bpb.TotSec32 & 0xff);
    buffer[33]	= (char)((FS_u32)(bpb.TotSec32 & 0x0000ff00UL) >> 8);
    buffer[34]	= (char)((FS_u32)(bpb.TotSec32 & 0x00ff0000UL) >> 16);
    buffer[35]	= (char)((FS_u32)(bpb.TotSec32 & 0xff000000UL) >> 24);

    /* Offset 36 and above have different meanings for FAT12/FAT16 and FAT32 */
    if (FSysType != 2)
    {
        /* FAT12/FAT16 */
        /* DrvNum = 0x00 (floppy) */
        buffer[36]	= (char)0x00;
        /* Reserved1 = 0x00 (floppy) */
        buffer[37]	= (char)0x00;
        /* BootSig = 0x00 (next three fields are not 'present') */
        buffer[38]	= (char)0x00;
        /* VolID = 0x00000000 (serial number, e.g. date/time) */
        for (i = 39; i < 43; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* VolLab = ' ' */
#if 1
        FS__CLIB_strncpy(&buffer[43], "MARS16	  ", 11);
#else
        for (i = 43; i < 54; i++)
        {
            buffer[i] = (char)0x20;
        }
#endif

        /* FilSysType = 'FAT12' */
        if (FSysType == 0)
        {
            FS__CLIB_strncpy(&buffer[54], "FAT12   ", 8);
        }
        else
        {
            FS__CLIB_strncpy(&buffer[54], "FAT16   ", 8);
        }
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        /* FAT32 */
        /* FATSz32 */
        buffer[36]	= (char)(bpb.FATSz32 & 0xff);
        buffer[37]	= (char)((FS_u32)(bpb.FATSz32 & 0x0000ff00UL) >> 8);
        buffer[38]	= (char)((FS_u32)(bpb.FATSz32 & 0x00ff0000UL) >> 16);
        buffer[39]	= (char)((FS_u32)(bpb.FATSz32 & 0xff000000UL) >> 24);
        /* EXTFlags */
        buffer[40]	= (char)(bpb.ExtFlags & 0xff);
        buffer[41]	= (char)((unsigned int)(bpb.ExtFlags & 0xff00) >> 8);
        /* FSVer = 0:0 */
        buffer[42]	= 0x00;
        buffer[43]	= 0x00;
        /* RootClus */
        buffer[44]	= (char)(bpb.RootClus & 0xff);
        buffer[45]	= (char)((FS_u32)(bpb.RootClus & 0x0000ff00UL) >> 8);
        buffer[46]	= (char)((FS_u32)(bpb.RootClus & 0x00ff0000UL) >> 16);
        buffer[47]	= (char)((FS_u32)(bpb.RootClus & 0xff000000UL) >> 24);
        /* FSInfo */
        buffer[48]	= (char)( (bpb.FSInfo) & 0xff);
        buffer[49]	= (char)((unsigned int)( (bpb.FSInfo) & 0xff00) >> 8);
        /* BkBootSec = 0x0006; */
        buffer[50]	= 0x06;
        buffer[51]	= 0x00;
        /* Reserved */
        for (i = 52; i < 64; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* DrvNum = 0x00 (floppy) */
        buffer[64]	= (char)0x00;
        /* Reserved1 = 0x00 (floppy) */
        buffer[65]	= (char)0x00;
        /* BootSig = 0x00 (next three fields are not 'present') */
        buffer[66]	= (char)0x00;
        /* VolID = 0x00000000 (serial number, e.g. date/time) */
        for (i = 67; i < 71; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* VolLab = ' ' */
#if 1
        FS__CLIB_strncpy(&buffer[71], strVolName, 11);
#else
        for (i = 71; i < 82; i++)
        {
            buffer[i] = (char)0x20;
        }
#endif
        /* FilSysType = 'FAT12' */
        FS__CLIB_strncpy(&buffer[82], "FAT32   ", 8);
    }
#endif /* FS_FAT_NOFAT32==0 */
    /* Signature = 0xAA55 */
    buffer[510] = (char)0x55;
    buffer[511] = (char)0xaa;
    /* Write BPB to media */
#if FS_RW_DIRECT
    i = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + 0, (void*)buffer);
#else
    i = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + 0, (void*)buffer);
#endif
    if (i < 0)
    {
        FS__fat_free(buffer);
        return -1;
    }
    if (FSysType == 2)
    {
        /* Write backup BPB */
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + 6, (void*)buffer);
#else
        i = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + 6, (void*)buffer);
#endif
        if (i<0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    /* Init FAT 1 & 2 */
    /* (VCC) To fine-tune */
#if 0

    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

    for (sector = 0; sector < 2 * (bpb.FATSz16 + bpb.FATSz32); sector++)
    {
        value = sector % (bpb.FATSz16 + bpb.FATSz32);
        if (value != 0)
        {
#if FS_RW_DIRECT
            i = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + sector, (void*)buffer);
#else
            i = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + sector, (void*)buffer);
#endif
            if (i<0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#else
    FS__CLIB_memset(cMulBlockBuffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE*128);
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

    //===Clear FAT1 ===//
    totalfatsec= (bpb.FATSz16 + bpb.FATSz32);
    fatsec=partitionStart + bpb.RsvdSecCnt;
    while(totalfatsec>0)
    {
        if(totalfatsec>=128)
            numOfSector=128;
        else
            numOfSector=totalfatsec;
        err = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, numOfSector, cMulBlockBuffer);
        totalfatsec -= numOfSector;
        fatsec +=numOfSector;
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }

    //===Clear FAT2 ===//
    totalfatsec= (bpb.FATSz16 + bpb.FATSz32);
    fatsec=partitionStart + bpb.RsvdSecCnt + (bpb.FATSz16 + bpb.FATSz32);
    while(totalfatsec>0)
    {
        if(totalfatsec>=128)
            numOfSector=128;
        else
            numOfSector=totalfatsec;
        err = FS__lb_mul_write(FS__pDevInfo[Idx].devdriver, Unit,fatsec, numOfSector, cMulBlockBuffer);
        totalfatsec -=numOfSector;
        fatsec +=numOfSector;
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }


#endif

    buffer[0] = (char)Media;
    buffer[1] = (char)0xff;
    buffer[2] = (char)0xff;
    if (FSysType != 0)
    {
        buffer[3] = (char)0xff;
    }
#if (FS_FAT_NOFAT32==0)
    if (FSysType == 2)
    {
        buffer[3]	= (char)0x0f;
        buffer[4]	= (char)0xff;
        buffer[5]	= (char)0xff;
        buffer[6]	= (char)0xff;
#if 1
        buffer[7]	= (char)0x0f; /* (VCC)	*/
#else
        buffer[7]	= (char)0xf0; /* cytsai: should be 0xf0 instead of 0x0f */
#endif
        buffer[8]	= (char)0xff;
        buffer[9]	= (char)0xff;
        buffer[10]	= (char)0xff;
#if 1
        buffer[11]	 = (char)0x0f; /* (VCC)  */
#else
        buffer[11]	= (char)0xf0; /* cytsai: should be 0xf0 instead of 0x0f */
#endif
    }
#endif /* FS_FAT_NOFAT32==0 */
    for (i = 0; i < 2; i++)
    {
#if FS_RW_DIRECT
        j = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + (FS_u32)bpb.RsvdSecCnt + i * ((FS_u32)bpb.FATSz16+bpb.FATSz32), (void*)buffer);
#else
        j = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + (FS_u32)bpb.RsvdSecCnt + i * ((FS_u32)bpb.FATSz16+bpb.FATSz32), (void*)buffer);
#endif
        if (j < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }	// two fat table
    }
    /* Init root directory area */
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
    if (bpb.RootEntCnt != 0)
    {
        /* FAT12/FAT16 */
        n = (((FS_u32)bpb.RootEntCnt * 32) / (FS_u32)512);	//n show the sectors accupied by root directory entries
        for (i = 0; i < n; i++)
        {
#if FS_RW_DIRECT
            j = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz16 + i, (void*)buffer);	//reserved sectors under fat12/16 is 1
#else
            j = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz16 + i, (void*)buffer);	//reserved sectors under fat12/16 is 1
#endif
            if (j < 0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        /* FAT32 */
        n = bpb.SecPerClus;
        for (i = 0; i < n; i++)
        {
#if FS_RW_DIRECT
            j = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz32 + (bpb.RootClus - 2) * n + i, (void*)buffer);
#else
            j = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz32 + (bpb.RootClus - 2) * n + i, (void*)buffer);
#endif
            if (j < 0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#endif /* FS_FAT_NOFAT32==0 */

#if (FS_FAT_NOFAT32==0)
#if INIT_DISK_INFO
    if (FSysType == 2)
    {
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        FS__CLIB_strncpy(&buffer[0], strVolName, 11);
        buffer[11]	   = (char)0x08;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz32 * 2, (void*)buffer);
#else
        i = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz32 * 2, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }

        /* Init FSInfo */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        /* LeadSig = 0x41615252 */
        buffer[0]	  = (char)0x52;
        buffer[1]	  = (char)0x52;
        buffer[2]	  = (char)0x61;
        buffer[3]	  = (char)0x41;
        /* StructSig = 0x61417272 */
        buffer[484]   = (char)0x72;
        buffer[485]   = (char)0x72;
        buffer[486]   = (char)0x41;
        buffer[487]   = (char)0x61;
        /* Invalidate last known free cluster count */
        buffer[488]   = (char)0xff;
        buffer[489]   = (char)0xff;
        buffer[490]   = (char)0xff;
        buffer[491]   = (char)0xff;
        /* Give hint for free cluster search */
        buffer[492]   = (char)0xff;
        buffer[493]   = (char)0xff;
        buffer[494]   = (char)0xff;
        buffer[495]   = (char)0xff;
        /* TrailSig = 0xaa550000 */
        buffer[508]   = (char)0x00;
        buffer[509]   = (char)0x00;
        buffer[510]   = (char)0x55;
        buffer[511]   = (char)0xaa;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#else
        i = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    else if (FSysType != 2)
    {
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

        /* When the system format the disk, the vol. name should be blank to make SD boot correctly. */
        FS__CLIB_strncpy(&buffer[0], strVolName, 11);

        buffer[11]	   = (char)0x08;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz16 * 2, (void*)buffer);
#else
        i = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz16 * 2, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
#if 0
        /* Init FSInfo */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        /* LeadSig = 0x41615252 */
        buffer[0]	  = (char)0x52;
        buffer[1]	  = (char)0x52;
        buffer[2]	  = (char)0x61;
        buffer[3]	  = (char)0x41;
        /* StructSig = 0x61417272 */
        buffer[484]   = (char)0x72;
        buffer[485]   = (char)0x72;
        buffer[486]   = (char)0x41;
        buffer[487]   = (char)0x61;
        /* Invalidate last known free cluster count */
        buffer[488]   = (char)0xff;
        buffer[489]   = (char)0xff;
        buffer[490]   = (char)0xff;
        buffer[491]   = (char)0xff;
        /* Give hint for free cluster search */
        buffer[492]   = (char)0xff;
        buffer[493]   = (char)0xff;
        buffer[494]   = (char)0xff;
        buffer[495]   = (char)0xff;
        /* TrailSig = 0xaa550000 */
        buffer[508]   = (char)0x00;
        buffer[509]   = (char)0x00;
        buffer[510]   = (char)0x55;
        buffer[511]   = (char)0xaa;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, bpb.FSInfo+ partitionStart, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
#endif
    }
#endif
#endif /* FS_FAT_NOFAT32==0 */

    FS__pDevInfo[Idx].FSInfo.FreeClusterCount = (bpb.TotSec32 - (bpb.RsvdSecCnt + bpb.FATSz32 * bpb.NumFATs) / bpb.SecPerClus) - 3;
    FS__pDevInfo[Idx].FSInfo.NextFreeCluster = bpb.RootClus + 1;

    FS__fat_free(buffer);
    return 0;
}

#else
static int _FS_fat_format(const FS__device_type *pDriver,FS_u32 Unit, char SecPerClus,
                          FS_u16 RootEntCnt, FS_u16 TotSec16, FS_u32 TotSec32, char Media,
                          FS_u16 FATSz16, FS_u32 FATSz32, FS_u16 SecPerTrk,FS_u16 NumHeads,
                          FS_u32 HiddSec, char FSysType, FS_u16 usReservedSize)
{
    FS__FAT_BPB bpb;
    FS_u32 sector;
    FS_u32 value;
    char *buffer;
    int i;
    int j;
    int n;
    int err;
    unsigned int partitionStart = 0;
    unsigned int partitionSize = 0;
    int totalfatsec,numOfSector,fatsec;
    //===================================================//

    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }

    /* cytsai: check Master Boot Record */
    /* read first sector and check if it's an MBR */
#if FS_RW_DIRECT
    err = FS__lb_read_Direct(pDriver, Unit, 0, (void*)buffer);
#else
    err = FS__lb_read(pDriver, Unit, 0, (void*)buffer);
#endif
    if (err < 0)
    {
        FS__fat_free(buffer);
        return -1;
    }
    /* check if it's a MBR */
    if ((buffer[510] == 0x55) && (buffer[511] == 0xaa))
    {
        /* check if it's a PBS; i.e. bytesPerSector == 0x0200 */
        if ((buffer[11] != 0x00) || (buffer[12] != 0x02))
        {
            /* it's a MBR */
            partitionStart = (((unsigned int)buffer[457]) << 24) |
                             (((unsigned int)buffer[456]) << 16) |
                             (((unsigned int)buffer[455]) <<  8) |
                             ((unsigned int)buffer[454]);
            partitionSize  = (((unsigned int)buffer[461]) << 24) |
                             (((unsigned int)buffer[460]) << 16) |
                             (((unsigned int)buffer[459]) <<  8) |
                             ((unsigned int)buffer[458]);

            if (TotSec16 == 0)
            {
                TotSec32 = partitionSize;
            }
            else
            {
                TotSec16 = (FS_u16)partitionSize;
            }
        }
    }

    /* Clear buffer */
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE); /* cytsai: FS__fat_malloc has already initialized the buffer to zero. */
    /* Setup BPB */
    FS__CLIB_memset(&bpb, 0x00, (FS_size_t)sizeof(bpb));
    bpb.BytesPerSec = 0x0200;       /* _512_,1024,2048,4096           */
    bpb.SecPerClus  = SecPerClus;   /* sec in allocation unit         */

    if (FSysType != 2)
    {
//    bpb.RsvdSecCnt  = 0x0001;       /* 1 for FAT12 & FAT16            */
        bpb.RsvdSecCnt  = usReservedSize;       /* 1 for FAT12 & FAT16            */
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        bpb.RsvdSecCnt  = usReservedSize;       /* 32 for FAT32                   */
    }
#else
    /* FAT32 disabled */
    else
    {
        FS__fat_free(buffer);
        return -1;
    }
#endif /* FS_FAT_NOFAT32==0 */
    bpb.NumFATs     = 0x02;         /* 2                              */
    bpb.RootEntCnt  = RootEntCnt;   /* number of root dir entries     */
    bpb.TotSec16    = TotSec16;     /* RSVD+FAT+ROOT+DATA (<64k)      */
    bpb.FATSz16     = FATSz16;      /* number of FAT sectors          */
    bpb.TotSec32    = TotSec32;     /* RSVD+FAT+ROOT+FATA (>=64k)     */
    bpb.Signature   = 0xaa55;       /* 0xAA55 Signature               */

    /* setup BPB specifics for FAT32 */
    bpb.FATSz32     = FATSz32;      /* number of FAT sectors          */
    bpb.ExtFlags    = 0x0000;       /* mirroring info                 */
    bpb.RootClus    = 0x00000002UL; /* root dir clus for FAT32        */
    bpb.FSInfo      = 0x0001;       /* position of FSInfo structure   */

    /* Set the number of FAT sectors */
    if (FSysType != 2)
    {
        bpb.FATSz16     = FATSz16;
        bpb.FATSz32     = 0;
    }
    else
    {
        bpb.FATSz16     = 0;
        bpb.FATSz32     = FATSz16;
    }

    /*
       Prepare buffer with information of the BPB
       offset 0 - 35 is same for FAT12/FAT16 and FAT32
    */

    /* jmpBoot = 0xe9 0x0000 */
#if 0
    buffer[0]   = (char)0xe9;
    buffer[1]   = (char)0x00;
    buffer[2]   = (char)0x00;
#else
    buffer[0]   = (char)0xeb;
    buffer[1]   = (char)0x3c;
    buffer[2]   = (char)0x90;
#endif
    /* OEMName = '        ' */
    for (i = 3; i < 11; i++)
    {
        buffer[i] = (char)0x20;
    }
    /* BytesPerSec */
    buffer[11]  = (char)(bpb.BytesPerSec & 0xff);
    buffer[12]  = (char)((unsigned int)(bpb.BytesPerSec & 0xff00) >> 8);
    /* SecPerClus */
    buffer[13]  = (char)bpb.SecPerClus;
    /* RsvdSecCnt */
    buffer[14]  = (char)(bpb.RsvdSecCnt & 0xff);
    buffer[15]  = (char)((unsigned int)(bpb.RsvdSecCnt & 0xff00) >> 8);
    /* NumFATs */
    buffer[16]  = (char)bpb.NumFATs;
    /* RootEntCnt */
    buffer[17]  = (char)(bpb.RootEntCnt & 0xff);
    buffer[18]  = (char)((unsigned int)(bpb.RootEntCnt & 0xff00) >> 8);
    /* TotSec16 */
    buffer[19]  = (char)(bpb.TotSec16 & 0xff);
    buffer[20]  = (char)((unsigned int)(bpb.TotSec16 & 0xff00) >> 8);
    /* Media */
    buffer[21]  = Media;
    /* FATSz16 */
    if (FSysType != 2)
    {
        buffer[22]  = (char)(bpb.FATSz16 & 0xff);
        buffer[23]  = (char)((unsigned int)(bpb.FATSz16 & 0xff00) >> 8);
    }
    else
    {
        /* FATSz16 = 0 when file system type = FAT32 */
        buffer[22]  = (char)0x00;
        buffer[23]  = (char)0x00;
    }
    /* SecPerTrk */
    buffer[24]  = (char)(SecPerTrk & 0xff);
    buffer[25]  = (char)((unsigned int)(SecPerTrk & 0xff00) >> 8);
    /* NumHeads */
    buffer[26]  = (char)(NumHeads & 0xff);
    buffer[27]  = (char)((unsigned int)(NumHeads & 0xff00) >> 8);
    /* HiddSec */
    //buffer[28]  = (char)(HiddSec & 0xff);
    //buffer[29]  = (char)((FS_u32)(HiddSec & 0x0000ff00UL) >> 8);
    //buffer[30]  = (char)((FS_u32)(HiddSec & 0x00ff0000UL) >> 16);
    //buffer[31]  = (char)((FS_u32)(HiddSec & 0xff000000UL) >> 24);
    buffer[28]  = (char)(partitionStart & 0xff);
    buffer[29]  = (char)((FS_u32)(partitionStart & 0x0000ff00UL) >> 8);
    buffer[30]  = (char)((FS_u32)(partitionStart & 0x00ff0000UL) >> 16);
    buffer[31]  = (char)((FS_u32)(partitionStart & 0xff000000UL) >> 24);
    /* TotSec32 */
    buffer[32]  = (char)(bpb.TotSec32 & 0xff);
    buffer[33]  = (char)((FS_u32)(bpb.TotSec32 & 0x0000ff00UL) >> 8);
    buffer[34]  = (char)((FS_u32)(bpb.TotSec32 & 0x00ff0000UL) >> 16);
    buffer[35]  = (char)((FS_u32)(bpb.TotSec32 & 0xff000000UL) >> 24);

    /* Offset 36 and above have different meanings for FAT12/FAT16 and FAT32 */
    if (FSysType != 2)
    {
        /* FAT12/FAT16 */
        /* DrvNum = 0x00 (floppy) */
        buffer[36]  = (char)0x00;
        /* Reserved1 = 0x00 (floppy) */
        buffer[37]  = (char)0x00;
        /* BootSig = 0x00 (next three fields are not 'present') */
        buffer[38]  = (char)0x00;
        /* VolID = 0x00000000 (serial number, e.g. date/time) */
        for (i = 39; i < 43; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* VolLab = ' ' */
#if 1
        FS__CLIB_strncpy(&buffer[43], "MARS16     ", 11);
#else
        for (i = 43; i < 54; i++)
        {
            buffer[i] = (char)0x20;
        }
#endif

        /* FilSysType = 'FAT12' */
        if (FSysType == 0)
        {
            FS__CLIB_strncpy(&buffer[54], "FAT12   ", 8);
        }
        else
        {
            FS__CLIB_strncpy(&buffer[54], "FAT16   ", 8);
        }
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        /* FAT32 */
        /* FATSz32 */
        buffer[36]  = (char)(bpb.FATSz32 & 0xff);
        buffer[37]  = (char)((FS_u32)(bpb.FATSz32 & 0x0000ff00UL) >> 8);
        buffer[38]  = (char)((FS_u32)(bpb.FATSz32 & 0x00ff0000UL) >> 16);
        buffer[39]  = (char)((FS_u32)(bpb.FATSz32 & 0xff000000UL) >> 24);
        /* EXTFlags */
        buffer[40]  = (char)(bpb.ExtFlags & 0xff);
        buffer[41]  = (char)((unsigned int)(bpb.ExtFlags & 0xff00) >> 8);
        /* FSVer = 0:0 */
        buffer[42]  = 0x00;
        buffer[43]  = 0x00;
        /* RootClus */
        buffer[44]  = (char)(bpb.RootClus & 0xff);
        buffer[45]  = (char)((FS_u32)(bpb.RootClus & 0x0000ff00UL) >> 8);
        buffer[46]  = (char)((FS_u32)(bpb.RootClus & 0x00ff0000UL) >> 16);
        buffer[47]  = (char)((FS_u32)(bpb.RootClus & 0xff000000UL) >> 24);
        /* FSInfo */
        buffer[48]  = (char)( (bpb.FSInfo) & 0xff);
        buffer[49]  = (char)((unsigned int)( (bpb.FSInfo) & 0xff00) >> 8);
        /* BkBootSec = 0x0006; */
        buffer[50]  = 0x06;
        buffer[51]  = 0x00;
        /* Reserved */
        for (i = 52; i < 64; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* DrvNum = 0x00 (floppy) */
        buffer[64]  = (char)0x00;
        /* Reserved1 = 0x00 (floppy) */
        buffer[65]  = (char)0x00;
        /* BootSig = 0x00 (next three fields are not 'present') */
        buffer[66]  = (char)0x00;
        /* VolID = 0x00000000 (serial number, e.g. date/time) */
        for (i = 67; i < 71; i++)
        {
            buffer[i] = (char)0x00;
        }
        /* VolLab = ' ' */
#if 1
        FS__CLIB_strncpy(&buffer[71], strVolName, 11);
#else
        for (i = 71; i < 82; i++)
        {
            buffer[i] = (char)0x20;
        }
#endif
        /* FilSysType = 'FAT12' */
        FS__CLIB_strncpy(&buffer[82], "FAT32   ", 8);
    }
#endif /* FS_FAT_NOFAT32==0 */
    /* Signature = 0xAA55 */
    buffer[510] = (char)0x55;
    buffer[511] = (char)0xaa;
    /* Write BPB to media */
#if FS_RW_DIRECT
    i = FS__lb_write_Direct(pDriver, Unit, partitionStart + 0, (void*)buffer);
#else
    i = FS__lb_write(pDriver, Unit, partitionStart + 0, (void*)buffer);
#endif
    if (i < 0)
    {
        FS__fat_free(buffer);
        return -1;
    }
    if (FSysType == 2)
    {
        /* Write backup BPB */
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, partitionStart + 6, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, partitionStart + 6, (void*)buffer);
#endif
        if (i<0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    /* Init FAT 1 & 2 */
    /* (VCC) To fine-tune */
#if 0

    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

    for (sector = 0; sector < 2 * (bpb.FATSz16 + bpb.FATSz32); sector++)
    {
        value = sector % (bpb.FATSz16 + bpb.FATSz32);
        if (value != 0)
        {
#if FS_RW_DIRECT
            i = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + sector, (void*)buffer);
#else
            i = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + sector, (void*)buffer);
#endif
            if (i<0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#else
    FS__CLIB_memset(cMulBlockBuffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE*128);
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

    //===Clear FAT1 ===//
    totalfatsec= (bpb.FATSz16 + bpb.FATSz32);
    fatsec=partitionStart + bpb.RsvdSecCnt;
    while(totalfatsec>0)
    {
        if(totalfatsec>=128)
            numOfSector=128;
        else
            numOfSector=totalfatsec;
        err = FS__lb_mul_write(pDriver, Unit, fatsec, numOfSector, cMulBlockBuffer);
        totalfatsec -= numOfSector;
        fatsec +=numOfSector;
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }

    //===Clear FAT2 ===//
    totalfatsec= (bpb.FATSz16 + bpb.FATSz32);
    fatsec=partitionStart + bpb.RsvdSecCnt + (bpb.FATSz16 + bpb.FATSz32);
    while(totalfatsec>0)
    {
        if(totalfatsec>=128)
            numOfSector=128;
        else
            numOfSector=totalfatsec;
        err = FS__lb_mul_write(pDriver, Unit,fatsec, numOfSector, cMulBlockBuffer);
        totalfatsec -=numOfSector;
        fatsec +=numOfSector;
        if (err < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }


#endif

    buffer[0] = (char)Media;
    buffer[1] = (char)0xff;
    buffer[2] = (char)0xff;
    if (FSysType != 0)
    {
        buffer[3] = (char)0xff;
    }
#if (FS_FAT_NOFAT32==0)
    if (FSysType == 2)
    {
        buffer[3]   = (char)0x0f;
        buffer[4]   = (char)0xff;
        buffer[5]   = (char)0xff;
        buffer[6]   = (char)0xff;
#if 1
        buffer[7]   = (char)0x0f; /* (VCC)  */
#else
        buffer[7]   = (char)0xf0; /* cytsai: should be 0xf0 instead of 0x0f */
#endif
        buffer[8]   = (char)0xff;
        buffer[9]   = (char)0xff;
        buffer[10]  = (char)0xff;
#if 1
        buffer[11]   = (char)0x0f; /* (VCC)  */
#else
        buffer[11]  = (char)0xf0; /* cytsai: should be 0xf0 instead of 0x0f */
#endif
    }
#endif /* FS_FAT_NOFAT32==0 */
    for (i = 0; i < 2; i++)
    {
#if FS_RW_DIRECT
        j = FS__lb_write_Direct(pDriver, Unit, partitionStart + (FS_u32)bpb.RsvdSecCnt + i * ((FS_u32)bpb.FATSz16+bpb.FATSz32), (void*)buffer);
#else
        j = FS__lb_write(pDriver, Unit, partitionStart + (FS_u32)bpb.RsvdSecCnt + i * ((FS_u32)bpb.FATSz16+bpb.FATSz32), (void*)buffer);
#endif
        if (j < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }	// two fat table
    }
    /* Init root directory area */
    FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
    if (bpb.RootEntCnt != 0)
    {
        /* FAT12/FAT16 */
        n = (((FS_u32)bpb.RootEntCnt * 32) / (FS_u32)512);	//n show the sectors accupied by root directory entries
        for (i = 0; i < n; i++)
        {
#if FS_RW_DIRECT
            j = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz16 + i, (void*)buffer);	//reserved sectors under fat12/16 is 1
#else
            j = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz16 + i, (void*)buffer);	//reserved sectors under fat12/16 is 1
#endif
            if (j < 0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#if (FS_FAT_NOFAT32==0)
    else
    {
        /* FAT32 */
        n = bpb.SecPerClus;
        for (i = 0; i < n; i++)
        {
#if FS_RW_DIRECT
            j = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz32 + (bpb.RootClus - 2) * n + i, (void*)buffer);
#else
            j = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + 2 * bpb.FATSz32 + (bpb.RootClus - 2) * n + i, (void*)buffer);
#endif
            if (j < 0)
            {
                FS__fat_free(buffer);
                return -1;
            }
        }
    }
#endif /* FS_FAT_NOFAT32==0 */

#if (FS_FAT_NOFAT32==0)
#if INIT_DISK_INFO
    if (FSysType == 2)
    {
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        FS__CLIB_strncpy(&buffer[0], strVolName, 11);
        buffer[11]     = (char)0x08;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz32 * 2, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz32 * 2, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }

        /* Init FSInfo */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        /* LeadSig = 0x41615252 */
        buffer[0]     = (char)0x52;
        buffer[1]     = (char)0x52;
        buffer[2]     = (char)0x61;
        buffer[3]     = (char)0x41;
        /* StructSig = 0x61417272 */
        buffer[484]   = (char)0x72;
        buffer[485]   = (char)0x72;
        buffer[486]   = (char)0x41;
        buffer[487]   = (char)0x61;
        /* Invalidate last known free cluster count */
        buffer[488]   = (char)0xff;
        buffer[489]   = (char)0xff;
        buffer[490]   = (char)0xff;
        buffer[491]   = (char)0xff;
        /* Give hint for free cluster search */
        buffer[492]   = (char)0xff;
        buffer[493]   = (char)0xff;
        buffer[494]   = (char)0xff;
        buffer[495]   = (char)0xff;
        /* TrailSig = 0xaa550000 */
        buffer[508]   = (char)0x00;
        buffer[509]   = (char)0x00;
        buffer[510]   = (char)0x55;
        buffer[511]   = (char)0xaa;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
    }
    else if (FSysType != 2)
    {
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);

        /* When the system format the disk, the vol. name should be blank to make SD boot correctly. */
        FS__CLIB_strncpy(&buffer[0], strVolName, 11);

        buffer[11]     = (char)0x08;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz16 * 2, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, partitionStart + bpb.RsvdSecCnt + bpb.FATSz16 * 2, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
#if 0
        /* Init FSInfo */
        FS__CLIB_memset(buffer, 0x00, (FS_size_t)FS_FAT_SEC_SIZE);
        /* LeadSig = 0x41615252 */
        buffer[0]     = (char)0x52;
        buffer[1]     = (char)0x52;
        buffer[2]     = (char)0x61;
        buffer[3]     = (char)0x41;
        /* StructSig = 0x61417272 */
        buffer[484]   = (char)0x72;
        buffer[485]   = (char)0x72;
        buffer[486]   = (char)0x41;
        buffer[487]   = (char)0x61;
        /* Invalidate last known free cluster count */
        buffer[488]   = (char)0xff;
        buffer[489]   = (char)0xff;
        buffer[490]   = (char)0xff;
        buffer[491]   = (char)0xff;
        /* Give hint for free cluster search */
        buffer[492]   = (char)0xff;
        buffer[493]   = (char)0xff;
        buffer[494]   = (char)0xff;
        buffer[495]   = (char)0xff;
        /* TrailSig = 0xaa550000 */
        buffer[508]   = (char)0x00;
        buffer[509]   = (char)0x00;
        buffer[510]   = (char)0x55;
        buffer[511]   = (char)0xaa;
#if FS_RW_DIRECT
        i = FS__lb_write_Direct(pDriver, Unit, bpb.FSInfo+ partitionStart, (void*)buffer);
#else
        i = FS__lb_write(pDriver, Unit, bpb.FSInfo + partitionStart, (void*)buffer);
#endif
        if (i < 0)
        {
            FS__fat_free(buffer);
            return -1;
        }
#endif
    }
#endif
#endif /* FS_FAT_NOFAT32==0 */
    FS__fat_free(buffer);
    return 0;
}
#endif /* FS_FAT_NOFORMAT==0 */
#endif

#if FS_FAT_DISKINFO

u32 _FS_fat_GetDiskVolumeInfo(FS_DISKFREE_T *pInfo, s8 *curDrive)
{
    if (FS_IoCtl((const char*)curDrive, FS_CMD_GET_DISKFREE, 0, (void*)pInfo) != 0)
    {
        DEBUG_FS("Error: Get drive %s information failed.\n", curDrive);
        return 0;
    }
    return 1;
}

u32 _FS_fat_GetDiskBPBInfo(FS__FAT_BPB *pInfo, u32 DevIdx, u32 unit)
{
    u32 reservSpace;
    if((FS_MAXDEV <= DevIdx) || (FS_FAT_MAXUNIT <= unit))
    {
        DEBUG_FS("Error: aBPBUnit[%d][%d] doesn't exist.\n", DevIdx, unit);
        return 0;
    }
    pInfo = &FS__FAT_aBPBUnit[DevIdx][unit];
    return 1;
}

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

static int _FS_fat_GetTotalFree(int Idx, FS_u32 Unit, FS_DISKFREE_T *pDiskData)
{
    FS_u32 freeclust;
    FS_u32 usedclust;
    FS_u32 totclust;
    FS_u32 fatentry;
    FS_u32 fatsize;
    FS_i32 fatoffs;
    FS_i32 bytespersec;
    FS_i32 cursec;
    FS_i32 fatsec;
    FS_i32 lastsec;
    FS_i32 fatindex;

    unsigned long* fat32buf;
    unsigned short* fat16buf;
    unsigned char* tempbuf;
    FS_u32 k;
    FS_u32 numOfSector;
    int fattype;

    int err;
    char *buffer;
    unsigned char a;
    unsigned char b;

    int totalfatsec;
    unsigned int *pp;
    unsigned short *pp_short;
    //---------------//
    if (!pDiskData)
    {
        return -1;  /* No pointer to a FS_DISKFREE_T structure */
    }
    buffer = FS__fat_malloc(FS_FAT_SEC_SIZE);
    if (!buffer)
    {
        DEBUG_FS("FS__fat_malloc is Fail\n");
        return -1;
    }
    fattype = FS__FAT_aBPBUnit[Idx][Unit].FATType;

    fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz16;
    if (fatsize == 0)
    {
        fatsize = FS__FAT_aBPBUnit[Idx][Unit].FATSz32;
    }
    bytespersec = (FS_i32)FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;

    /*---- Calculate total allocation units on disk -----*/
    totclust = FS__FAT_aBPBUnit[Idx][Unit].TotSec16;
    if (!totclust)
    {
        totclust = FS__FAT_aBPBUnit[Idx][Unit].TotSec32;
    }
    totclust  -= FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
    totclust  -= 2*fatsize;
    usedclust  = FS__FAT_aBPBUnit[Idx][Unit].RootEntCnt;
    usedclust *= 0x20;
    usedclust /= bytespersec;
    totclust  -= usedclust;
    totclust  /= FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;

    /*--- Scan FAT for free and used entries ---*/
    cursec     = 0;
    fatsec     = 0;
    lastsec    = -1;
    fatentry   = 0xffffUL;
    freeclust  = 0;
    usedclust  = 0;

    if (fattype == 1) //FAT12
    {
        while (1)
        {
            if (cursec >= (FS_i32)totclust)
            {
                break;  /* Last cluster reached */
            }
            if (fatsec >= (FS_i32)fatsize + FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt)
            {
                break;  /* End of FAT reached */
            }

            fatindex = (cursec + 2) + ((cursec + 2) / 2);    /* FAT12 */
            fatsec = FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt + (fatindex / bytespersec);
            fatoffs = fatindex % bytespersec;
            if (fatsec != lastsec)
            {
#if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                }
#else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec, (void*)buffer);
                }
#endif
                lastsec = fatsec;
            }

            if (fatoffs == (bytespersec - 1))
            {
                a = buffer[fatoffs];
#if FS_RW_DIRECT
                err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec+1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }

#else
                err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsec+1, (void*)buffer);
                if (err < 0)
                {
                    err = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, fatsize + fatsec + 1, (void*)buffer);
                    if (err < 0)
                    {
                        FS__fat_free(buffer);
                        return -1;
                    }
                    /* Try to repair original FAT sector with contents of copy */
                    FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, fatsec + 1, (void*)buffer);
                }
#endif
                lastsec = fatsec + 1;
                b = buffer[0];
            }
            else
            {
                a = buffer[fatoffs];
                b = buffer[fatoffs + 1];
            }
            if (cursec & 1)
            {
                fatentry = ((a & 0xf0) >> 4 ) + 16 * b;
            }
            else
            {
                fatentry = a + 256 * (b & 0x0f);
            }
            fatentry &= 0x0fff;

            cursec++;
            if (fatentry == 0)
            {
                freeclust++;
            }
            else
            {
                usedclust++;
            }
        }
    }
    else //if (fattype==0 ||fattype==2)
    {
        tempbuf=(unsigned char*)cMulBlockBuffer;  //Lucian: 佔用cMulBlockBuffer (Common data),因 _FS_fat_GetTotalFree 只可使用於開機,熱插拔卡,Format 後.
        totalfatsec=fatsize;
        fatsec=FS__FAT_aBPBUnit[Idx][Unit].RsvdSecCnt;
        while(totalfatsec>0)
        {
            if(totalfatsec>128)
                numOfSector=128;
            else
                numOfSector=totalfatsec;

            err = FS__lb_mul_read(FS__pDevInfo[Idx].devdriver,Unit,fatsec,numOfSector,(void *)tempbuf);
            if (err < 0)
            {

                FS__fat_free(buffer);
                return -1;
            }
            fatsec +=numOfSector;
            totalfatsec -=numOfSector;

            if (fattype == 2) //FAT32
            {
                pp = (unsigned int *)tempbuf;
                while(pp< (unsigned int *)(tempbuf + 512*numOfSector))
                {
                    if (freeclust+usedclust>=totclust+2)
                        break;
                    if( (*pp & 0x0fffffff) ==  0)
                        freeclust++;
                    else
                        usedclust++;
                    pp ++;
                }
            }
            else
            {
                pp_short= (unsigned short *)tempbuf;
                while(pp_short<(unsigned short *)(tempbuf + 512*numOfSector))
                {
                    if (freeclust+usedclust>=totclust+2)
                        break;
                    if( *pp_short ==  0)
                        freeclust++;
                    else
                        usedclust++;
                    pp_short ++;
                }
            }

        }

    }

    pDiskData->total_clusters      = totclust;
    pDiskData->avail_clusters      = freeclust;
    pDiskData->sectors_per_cluster = FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
    pDiskData->bytes_per_sector    = (FS_u16)bytespersec;
#if FS_NEW_VERSION
    FS__pDevInfo[Idx].FSInfo.FreeClusterCount = freeclust;	// Update UnusedCluster number
#endif
    FS__fat_free(buffer);
    return 0;
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

int FS__fat_ioctl(int Idx, FS_u32 Unit, FS_i32 Cmd, FS_i32 Aux, void *pBuffer)
{
//  FS_u32 dsec;
    struct
    {
        FS_u32 hiddennum;
        FS_u32 headnum;
        FS_u32 secnum;
        FS_u32 partsize;
    }
    devinfo;
    int x = 0;
#if(FILE_SYSTEM_SEL ==  FILE_SYSTEM_DVR)  // FAT adjustment
    FS_u32 rootdirsize;
    FS_u32 totalsec;
    FS_u32 rest_sec;
    FS_u32 estimate_asf_nums;
#endif

#if ((FS_SUPPORT_SEC_ACCESS) || (FS_FAT_NOFORMAT==0))
    int i;
#endif
#if (FS_FAT_NOFORMAT==0)
    int j;
#endif
    int nClusEntriesNoPerFatCluster;
    int nReservedSize = 0;
    int nFSysType;

    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_INC_BUSYCNT, 0, (void*)0);  /* Turn on busy signal */
#if (FS_FAT_NOFORMAT==0)
    //------------------------------Optimized Format Command--------------------------//
    if ( (Cmd == FS_CMD_FORMAT_MEDIA) || (Cmd == FS_CMD_FORMAT_FAST) )
    {
        j = 0;
        while (1)
        {
            if (j >= FS_KNOWNMEDIA_NUM)
            {
                break;  /* Not a known media */
            }
            if (_FS_wd_format_media_table[j].media_id == Aux)
            {
                break;  /* Media found in the list */
            }
            j++;
        }	//find the right media
        if (j >= FS_KNOWNMEDIA_NUM)
        {
            //can't find the right media
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
            return -1;
        }
        i = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);	//get status of device, if ok return 1
        if (i >= 0)
        {
            if(Cmd == FS_CMD_FORMAT_MEDIA)
            {
                DEBUG_FS("====> Device erase start!\n");
                i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FORMAT_MEDIA, 0, (void*)0); //call device driver to erase data.
                if(i<0)
                {
                    DEBUG_FS("====> Device erase Failed!\n");
                    return -1;
                }
                DEBUG_FS("====> Device erase complete!\n");
            }

            if ( (Cmd == FS_CMD_FORMAT_FAST) || (Cmd==FS_CMD_FORMAT_MEDIA) )
            {
                i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_GET_DEVINFO, 0, (void*)&devinfo); //get card information

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

                nReservedSize = 8 - (((FS_u32)_FS_wd_format_media_table[j].fatsz16 << 30) >> 29); //??


                if (nFSysType == 2)
                {
                    if (nReservedSize== 8)
                        nReservedSize = 32;
                    else
                        nReservedSize += 32;
                }
                FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FLUSH_CACHE, 0, (void*)0);
                _FS_wd_format_media_table[j].usReservedSize = nReservedSize;

                DEBUG_FS("====> _FS_fat_format start!\n");
#if FS_NEW_VERSION
                x = _FS_fat_format(Idx,
                                   Unit,
                                   _FS_wd_format_media_table[j].secperclus,
                                   _FS_wd_format_media_table[j].rootentcnt,
                                   _FS_wd_format_media_table[j].totsec16,
                                   _FS_wd_format_media_table[j].totsec32,
                                   _FS_wd_format_media_table[j].media,
                                   _FS_wd_format_media_table[j].fatsz16,
                                   0,
                                   _FS_wd_format_media_table[j].secpertrk,
                                   _FS_wd_format_media_table[j].numheads,
                                   _FS_wd_format_media_table[j].hiddsec,
                                   _FS_wd_format_media_table[j].fsystype,
                                   _FS_wd_format_media_table[j].usReservedSize);

#else
                x = _FS_fat_format(FS__pDevInfo[Idx].devdriver,
                                   Unit,
                                   _FS_wd_format_media_table[j].secperclus,
                                   _FS_wd_format_media_table[j].rootentcnt,
                                   _FS_wd_format_media_table[j].totsec16,
                                   _FS_wd_format_media_table[j].totsec32,
                                   _FS_wd_format_media_table[j].media,
                                   _FS_wd_format_media_table[j].fatsz16,
                                   0,
                                   _FS_wd_format_media_table[j].secpertrk,
                                   _FS_wd_format_media_table[j].numheads,
                                   _FS_wd_format_media_table[j].hiddsec,
                                   _FS_wd_format_media_table[j].fsystype,
                                   _FS_wd_format_media_table[j].usReservedSize);

#endif
                DEBUG_FS("====> _FS_fat_format compete!\n");

            }

            i = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_FLUSH_CACHE, 0, (void*)0);
            if (i < 0)
            {
                x = i;
            }
            else
            {
                /* Invalidate BPB */
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
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */
            return -1;
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
        if (i > 0)
        {
            DEBUG_FS("======> _FS_fat_GetTotalFree start!\n");
            x = _FS_fat_GetTotalFree(Idx, Unit, (FS_DISKFREE_T*)pBuffer);
            DEBUG_FS("======> _FS_fat_GetTotalFree compete!\n");
        }
        else
        {
            x = -1;
        }
    }
#else /* FS_FAT_DISKINFO==0 */
    else if (Cmd == FS_CMD_GET_DISKFREE)
    {
        x = -1; /* Diskinfo command not supported */
    }
#endif /* FS_FAT_DISKINFO */
    //-----------------------------//
    else if(Cmd ==FS_CMD_CHECK_UNIT)
    {
        i = FS__fat_checkunit(Idx, Unit);
        if(i==1)
        {
            DEBUG_FS("======> FS_CMD_CHECK_UNIT is OK!\n");
            x = 0;
        }
        else
            x = -1;
        global_diskInfo.sectors_per_cluster=FS__FAT_aBPBUnit[Idx][Unit].SecPerClus;
        global_diskInfo.bytes_per_sector=FS__FAT_aBPBUnit[Idx][Unit].BytesPerSec;
    }
    //-------Read one Sector-------//
#if FS_SUPPORT_SEC_ACCESS
    else if ((Cmd == FS_CMD_READ_SECTOR) || (Cmd == FS_CMD_WRITE_SECTOR))
    {
        if (!pBuffer)
        {
            FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
            return -1;
        }
        i = FS__lb_status(FS__pDevInfo[Idx].devdriver, Unit);
        if (i >= 0)
        {
            if (Cmd == FS_CMD_READ_SECTOR)
            {
#if FS_RW_DIRECT
                x = FS__lb_read_Direct(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
#else
                x = FS__lb_read(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
#endif
            }
            else
            {
#if FS_RW_DIRECT
                x = FS__lb_write_Direct(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
#else
                x = FS__lb_write(FS__pDevInfo[Idx].devdriver, Unit, Aux, pBuffer);
#endif
            }
        }
        else
        {
            x = -1;
        }
    }
#else /* FS_SUPPORT_SEC_ACCESS */
    else if ((Cmd == FS_CMD_READ_SECTOR) || (Cmd == FS_CMD_WRITE_SECTOR))
    {
        FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);
        return -1;
    }
#endif /* FS_SUPPORT_SEC_ACCESS */
    else
    {
        /* Maybe command for driver */
        x = FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, Cmd, Aux, (void*)pBuffer);
    }
    FS__lb_ioctl(FS__pDevInfo[Idx].devdriver, Unit, FS_CMD_DEC_BUSYCNT, 0, (void*)0);  /* Turn off busy signal */

    return x;
}


