/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcapi.h

Abstract:

   	The application interface of the SMC controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SMC_API_H__
#define __SMC_API_H__

#include "general.h"

#define SMC_SET_REQ		1
#define SMC_GET_REQ		0

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

/* Constant */
#define SMC_16MB  0x1000000
#define SMC_32MB  0x2000000
#define SMC_64MB  0x4000000
#define SMC_128MB 0x8000000
#define SMC_256MB 0x10000000
#define SMC_512MB 0x20000000
#define SMC_1024MB 0x40000000

//-------規劃 NAND  記憶體配置: Code / Parameter / UI LIB Size-------//
	#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
		#define SMC_TOTAL_SIZE                  SMC_32MB     //32MB
		#define SMC_RESERVED_SIZE			    0x00460000   //reserved 4.375 MB
		#define SMC_CODE_END_ADDR               0x00100000   //Code: 1MB
	#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
		    #define SMC_TOTAL_SIZE				SMC_128MB    //0x1 GByte
		    #define SMC_RESERVED_SIZE			0x00460000   //reserved 4.375 MB
		    #define SMC_CODE_END_ADDR			0x00100000   //Code: 1 MB

			#if ((DRAM_MEMORY_END + 4) - SdramBase <= 0x01000000)		/* 16 MB */
			#define SMC_MAX_BURN_SIZE			0x00300000   //用來燒 UI Libery.
			#else
			#define SMC_MAX_BURN_SIZE			0x00100000	/* 1 MB is a temporary option, it should be modified to a correct value. */
			#endif

	#endif



//---------配置 PAGE/Spare, Block size for NAND -----------//
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
   #define SMC_MAX_PAGE_SIZE		512	
   #define SMC_MAX_REDUN_SIZE       16
   #define SMC_MAX_PAGE_PER_BLOCK	32	
   #define SMC_BLOCK_SIZE           SMC_MAX_PAGE_SIZE*SMC_MAX_PAGE_PER_BLOCK
   #define SMC_ADDR_CYCLE		    3	
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
   #define SMC_MAX_PAGE_SIZE		4096	
   #define SMC_MAX_REDUN_SIZE		128
   #define SMC_MAX_PAGE_PER_BLOCK	64	
   #define SMC_BLOCK_SIZE			SMC_MAX_PAGE_SIZE * SMC_MAX_PAGE_PER_BLOCK
   #define SMC_ADDR_CYCLE			5
#else
   #define SMC_MAX_PAGE_SIZE		512	
   #define SMC_MAX_REDUN_SIZE       16
   #define SMC_MAX_PAGE_PER_BLOCK	32	
   #define SMC_BLOCK_SIZE           SMC_MAX_PAGE_SIZE*SMC_MAX_PAGE_PER_BLOCK
   #define SMC_ADDR_CYCLE		    3	
#endif

//------- 配置 Code / Parameter / UI Lib addressing-----//
//==MBR==/
#define SMC_MAX_MBR_PAGES		             0x01	

//==UI Lib==//
#define SMC_UI_LIBRARY_ADDR                  SMC_CODE_END_ADDR
#define SMC_UI_LIBRARY_SIZE                  (SMC_RESERVED_SIZE-SMC_CODE_END_ADDR)

//==Parameter==//
#define SMC_SYS_PARAMETER_ADDR	    	     SMC_RESERVED_SIZE
#define SMC_SYS_PARAMETER_SIZE               SMC_BLOCK_SIZE

//==Defect pixel==//
#define SMC_DEFECT_PIXEL_ADDR	            SMC_SYS_PARAMETER_ADDR + SMC_SYS_PARAMETER_SIZE	
#define SMC_DEFECT_PIXEL_SIZE	            SMC_BLOCK_SIZE	

//------Data allocation of  Paramete section--------// 
#define SMC_UI_SECTOR_ADDR   0          //in SYS parameter block
#define SMC_UI_SECTOR_SIZE   1 

#define SMC_AWB_SECTOR_ADDR   (SMC_UI_SECTOR_ADDR + SMC_UI_SECTOR_SIZE)          //in SYS parameter block
#define SMC_AWB_SECTOR_SIZE   1 

#define SMC_FB_SECTOR_ADDR		1           //in SYS parameter block
#define SMC_FB_SECTOR_SIZE		1 

//-------------------------------------------------//
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) || (FLASH_OPTION == FLASH_NAND_9002_NORMAL))
#define SMC_TOTAL_PAGE_CNT                    ((SMC_TOTAL_SIZE )/SMC_MAX_PAGE_SIZE)
#define SMC_MAX_MAP_SIZE_IN_BYTE              ((((SMC_TOTAL_PAGE_CNT)/8 + SMC_MAX_PAGE_SIZE-1)/SMC_MAX_PAGE_SIZE)*SMC_MAX_PAGE_SIZE)	// Align sector
#define SMC_MAX_MAP_SIZE_IN_PAGE              (SMC_MAX_MAP_SIZE_IN_BYTE/ SMC_MAX_PAGE_SIZE)
#define SMC_MAX_MAP_BLOCKS                    ((SMC_MAX_MAP_SIZE_IN_PAGE+SMC_MAX_PAGE_PER_BLOCK-1)/SMC_MAX_PAGE_PER_BLOCK)
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
#define SMC_TOTAL_PAGE_CNT					  SMC_TOTAL_SIZE/SMC_MAX_PAGE_SIZE		/* total pages in flash */
#define SMC_MAX_MAP_SIZE_IN_PAGE			  SMC_TOTAL_PAGE_CNT/(SMC_MAX_PAGE_SIZE * 8)	/* one byte (8 bits) represents 8 pages used status */
#define SMC_MAX_MAP_SIZE_IN_BYTE			  SMC_MAX_MAP_SIZE_IN_PAGE * SMC_MAX_PAGE_SIZE
#define SMC_MAX_MAP_BLOCKS					  ((SMC_MAX_MAP_SIZE_IN_PAGE+SMC_MAX_PAGE_PER_BLOCK-1)/SMC_MAX_PAGE_PER_BLOCK)
#endif

#define SMC_MAP_ADDR                          (SMC_DEFECT_PIXEL_ADDR + SMC_DEFECT_PIXEL_SIZE)          
#define SMC_FAT_START_ADDR                    (SMC_MAP_ADDR + SMC_MAX_MAP_BLOCKS* SMC_BLOCK_SIZE)      /* Align Block is needed. It will affect the FAT behavior on NAND */
#define SMC_RESERVED_BLOCK			          32

#if (SMC_TOTAL_SIZE==SMC_16MB)
//We hope Data start sector will align block
#define SEC_PER_CLS			0x20
#define TOSECT16			(SMC_TOTAL_SIZE -SMC_FAT_START_ADDR)/SMC_MAX_PAGE_SIZE - SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK
#define TOSECT32			0
#define RSVD_SEC_CNT		2
#define FATSZ16				0x0F  
#define ROOT_ENTRY_CNT		0x0200
#define NUM_FATS			2
#define	DISK_SIZE_IN_BYTE	SMC_TOTAL_SIZE - SMC_RESERVED_SIZE - (SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK*SMC_MAX_PAGE_SIZE) - FS_SECTOR_SIZE*(RSVD_SEC_CNT + FATSZ16*NUM_FATS + ROOT_ENTRY_CNT)
#define DISK_SIZE_IN_SECTOR	(DISK_SIZE_IN_BYTE / FS_SECTOR_SIZE)
#elif(SMC_TOTAL_SIZE==SMC_32MB)
#define SEC_PER_CLS			0x20
#define TOSECT16			(SMC_TOTAL_SIZE -SMC_FAT_START_ADDR)/SMC_MAX_PAGE_SIZE - SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK
#define TOSECT32			0
#define RSVD_SEC_CNT		2
#define FATSZ16				0x0F  
#define ROOT_ENTRY_CNT      0x0200
#define NUM_FATS			2
#define	DISK_SIZE_IN_BYTE	SMC_TOTAL_SIZE - SMC_RESERVED_SIZE - (SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK*SMC_MAX_PAGE_SIZE) - FS_SECTOR_SIZE*(RSVD_SEC_CNT + FATSZ16*NUM_FATS + ROOT_ENTRY_CNT)
#define DISK_SIZE_IN_SECTOR	(DISK_SIZE_IN_BYTE / FS_SECTOR_SIZE)
#elif(SMC_TOTAL_SIZE==SMC_64MB)
#define SEC_PER_CLS			0x20
#define TOSECT16			0		/* 0 indicates the total size is greater than 32 MB */
#define TOSECT32			((SMC_TOTAL_SIZE -SMC_FAT_START_ADDR)/SMC_MAX_PAGE_SIZE - SMC_RESERVED_BLOCK * SMC_MAX_PAGE_PER_BLOCK) * (SMC_MAX_PAGE_SIZE/ FS_SECTOR_SIZE)
#define RSVD_SEC_CNT		4		/* set to be 4 is helpful to acess FATs */
#define FATSZ16				0xF3	/* refer from windows FAT-16 */  
#define ROOT_ENTRY_CNT      0x0200
#define NUM_FATS			2
#define	DISK_SIZE_IN_BYTE	SMC_TOTAL_SIZE - SMC_RESERVED_SIZE - (SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK*SMC_MAX_PAGE_SIZE) - FS_SECTOR_SIZE*(RSVD_SEC_CNT + FATSZ16*NUM_FATS + ROOT_ENTRY_CNT)
#define DISK_SIZE_IN_SECTOR	(DISK_SIZE_IN_BYTE / FS_SECTOR_SIZE)
#elif(SMC_TOTAL_SIZE==SMC_128MB)
#define SEC_PER_CLS			0x20
#define TOSECT16			0		/* 0 indicates the total size is greater than 32 MB */
#define TOSECT32			((SMC_TOTAL_SIZE -SMC_FAT_START_ADDR)/SMC_MAX_PAGE_SIZE - SMC_RESERVED_BLOCK * SMC_MAX_PAGE_PER_BLOCK) * (SMC_MAX_PAGE_SIZE/ FS_SECTOR_SIZE)
#define RSVD_SEC_CNT		4		/* set to be 4 is helpful to acess FATs */
#define FATSZ16				0xFE	/* refer from windows FAT-16 */  
#define ROOT_ENTRY_CNT      0x0200
#define NUM_FATS			2
#define	DISK_SIZE_IN_BYTE	SMC_TOTAL_SIZE - SMC_RESERVED_SIZE - (SMC_RESERVED_BLOCK*SMC_MAX_PAGE_PER_BLOCK*SMC_MAX_PAGE_SIZE) - FS_SECTOR_SIZE*(RSVD_SEC_CNT + FATSZ16*NUM_FATS + ROOT_ENTRY_CNT)
#define DISK_SIZE_IN_SECTOR	(DISK_SIZE_IN_BYTE / FS_SECTOR_SIZE)
#endif

#define SMC_RESERVED_SEC_START				(TOSECT32 + TOSECT16 + (SMC_FAT_START_ADDR/SMC_MAX_PAGE_SIZE))  // sectors unit;  contain one block for bit map
#define SMC_RESERVED_LOGICAL_START			SMC_FAT_START_ADDR + (TOSECT32 + TOSECT16) * FS_SECTOR_SIZE		//SMC_RESERVED_SEC_START*SMC_MAX_PAGE_SIZE

/* Function prototype */

extern s32 smcInit(void);
extern s32 smcMount(void);
extern s32 smcUnmount(void);
extern void smcStart(void);
extern void smcIntHandler(void);
extern s32 smcReset(void);
extern s32 smcIdentification(void);
//extern void smcTest(void);

extern int smcDevStatus(u32); 
extern int smcDevRead(u32, u32, void*); 
extern int smcDevWrite(u32, u32, void*); 
extern int smcDevMultiWrite(u32, u32, u32,void*);  //civic 070907
extern int smcDevMultiRead(u32, u32, u32,void*);  //civic 070907
extern int smcDevIoCtl(u32, s32, s32, void*);
extern u8	smcProcProt(u8, u8);

#if INTERNAL_STORAGE_TEST
extern s8	smcTest(void);
#endif

extern u16 nand_fat_start ,nand_fat_end ,nand_fat_size,nand_data_start;     

typedef struct {
  u32          TotSec32;         /* RSVD + FAT + ROOT + FATA (>=64k) */
  u16          BytesPerSec;      /* _512_,1024,2048,4096           */
  u16          RootEntCnt;       /* number of root dir entries     */
  u16          TotSec16;         /* RSVD + FAT + ROOT + FATA (<64k) */
  u16          FATSz16;          /* number of FAT sectors          */
  u16          RsvdSecCnt;       /* 1 for FAT12 & FAT16            */	
  unsigned char   SecPerClus;       /* sec in allocation unit         */
  unsigned char   NumFATs;          /* 2                              */
} NAND_FAT_BPB;

typedef struct {
 u32 Bad_Block_addr[SMC_RESERVED_BLOCK];	
 u32 New_Block_addr[SMC_RESERVED_BLOCK];
 u8 index;      //indicate the pointer in reserved block
 u8 b_index;    //indicate bad block index
 u8 bbm_flag;
} NAND_BBM;

extern NAND_FAT_BPB NAND_FAT_PARAMETER;
extern NAND_BBM SMC_BBM;

#define MI_MAGIC_ADDR (SMC_FAT_START_ADDR+0x100000)
#define MI_MAGIC_DATA 0x87A72028
extern u8 ucNANDInit ;
#else //FLASH_NO_DEVICE

#define SMC_MAX_PAGE_SIZE		512	

#endif


#endif
