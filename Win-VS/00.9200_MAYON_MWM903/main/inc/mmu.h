/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mmu.h

Abstract:

   	MMU Create

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "general.h"
#include "mmuapi.h"
#ifdef MMU_SUPPORT

/* handy sizes */

#ifdef SECTION_PAGE
struct map_desc {
	unsigned long virtual;
	unsigned long physical;
	unsigned long length;
	int domain:4,  // MMU domain
	    prot_read:1,        // Access Permission
	    prot_write:1,     // Access Permission
	    cacheable:1,  // cache
	    bufferable:1, // write buffer
	    last:1;
};
#else
struct map_desc {
	unsigned long virtual;
	unsigned long physical;
	unsigned long length;
       unsigned short cacheable;  // cache
       unsigned short bufferable; // write buffer
	int domain:4,  // MMU domain
	    prot_read:1,        // Access Permission
	    prot_write:1,     // Access Permission
	    last:1;
};


#endif
#define LAST_DESC \
  { last: 1 }

#define DOMAIN_NOACCESS	0
#define DOMAIN_CLIENT	1
#define DOMAIN_MANAGER	3

#define ROM_PROTECTION (1 << 9)

#define ICACHE_ENAB (1 << 12)
#define DCACHE_ENAB (1 << 2)
#define ALIGNT_CHECK (2)
#define MPU_ENAB (1)
#define MMU_ENAB MPU_ENAB

#define VA_StMemBase		0x00000000	/* Static Memory Base				*/
#define VA_CpuTCMBase       0x60000000
#define VA_SdramBase		0x80000000	/* SDRAM Base					*/
#define VA_AHBARBBase      0xc0000000  /* AHB Arbit config, Lucian added*/
#define VA_SdramCtrlBase   	0xc0040000	/* SDRAM Control Register Base			*/
#define VA_DmaCtrlBase		0xc0050000	/* DMA Control Register Base			*/
#define VA_SysCfgBase     		0xc0060000	/* System Configuration Register Base		*/
#define VA_EmbedSramBase   	0xc0070000	/* Embedded SRAM Base				*/
#define VA_HiuCtrlBase		0xc0080000	/* Host Interface Unit Control Register Base	*/

#define VA_SiuCtrlBase		0xc0100000	/* Sensor Input Unit Control Register Base 	*/
#define VA_IpuCtrlBase		0xc0110000	/* Image Processing Unit Control Register Base	*/
#define VA_IduCtrlBase		0xc0120000	/* Image Display Unit Control Register Base	*/
#define VA_TVCtrBase       0xc0120000  /* TV Unit Control Register Base*/
#define VA_IsuCtrlBase		0xc0130000	/* Image Scaling Unit Control Register Base 	*/
#define VA_JpegCtrlBase		0xc0140000	/* JPEG Control Register Base			*/
#define VA_Mpeg4CtrlBase		0xc0150000	/* MPEG4 Control Register Base			*/


/* APB Processing Unit */
#define VA_IntCtrlBase		0xd0000000	/* INT Control Register Base			*/
#define VA_StMemCtrlBase    0xd0010000	/* Static Memory Control Register Base		*/
#define VA_GpioCtrlBase		0xd0020000	/* GPIO Control Register Base			*/
#define VA_UartCtrlBase		0xd0030000	/* UART Control Register Base			*/
#define VA_I2cCtrlBase		0xd0040000	/* I2C Control Register Base			*/
#define VA_IisCtrlBase		0xd0050000	/* IIS Control Register Base			*/
#define VA_UsbCtrlBase		0xd0060000	/* USB Control Register Base			*/
#define VA_SdcCtrlBase		0xd0070000	/* SecurityDisk Card Control Register Base	*/
#define VA_SmcCtrlBase		0xd0080000	/* SmartMedia Card Control Register Base	*/
#define VA_TimerCtrlBase	0xd0090000	/* Timer Control Register Base			*/
#define VA_RtcCtrlBase		0xd00a0000	/* RTC Control Register	Base			*/
#define VA_SysCtrlBase		0xd00b0000	/* System Control Register Base			*/
#define VA_WdtCtrlBase		0xd00c0000	/* WDT Control Register Base			*/
#define VA_AdcCtrlBase		0xd00d0000	/* ADC Control Register Base			*/

#define VA_GfuCtrlBase      0xe0000000  /* 2D GFU Control Register base*/

#define MMUTT_DESCTYPE_SHIFT 0
#define MMUTT_DESCTYPE_NBITS 5
#define MMUTT_DESCTYPE_MASK  (((1<<MMUTT_DESCTYPE_NBITS)-1)<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_FAULT_UNSHFT          0
#define MMUTT_DESCTYPE_TBL_COARSE_UNSHFT  0x11 // for 64KB and 4KB pages
#define MMUTT_DESCTYPE_SECTION_UNSHFT     0x12 // 1MB page
#define MMUTT_DESCTYPE_TBL_FINE_UNSHFT    0x13 // for 1KB pages
#define MMUTT_DESCTYPE_PAGE_LARGE_UNSHFT     1 // 64KB page
#define MMUTT_DESCTYPE_PAGE_SMALL_UNSHFT     2 // 4KB page
#define MMUTT_DESCTYPE_PAGE_TINY_UNSHFT      3 // 4KB page
#define MMUTT_DESCTYPE_FAULT              (MMUTT_DESCTYPE_FAULT_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_TBL_COARSE         (MMUTT_DESCTYPE_TBL_COARSE_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_SECTION            (MMUTT_DESCTYPE_SECTION_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_TBL_FINE           (MMUTT_DESCTYPE_TBL_FINE_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_PAGE_LARGE         (MMUTT_DESCTYPE_PAGE_LARGE_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_PAGE_SMALL         (MMUTT_DESCTYPE_PAGE_SMALL_UNSHFT<<MMUTT_DESCTYPE_SHIFT)
#define MMUTT_DESCTYPE_PAGE_TINY          (MMUTT_DESCTYPE_PAGE_TINY_UNSHFT<<MMUTT_DESCTYPE_SHIFT)

// Physical base address pointed to by descriptor
#define MMUTT_SECTIONBASE_SHIFT    20
#define MMUTT_SECTIONBASE_NBITS    (32-MMUTT_SECTIONBASE_SHIFT)
#define MMUTT_SECTIONBASE_MASK     (((1<<MMUTT_SECTIONBASE_NBITS)-1)<<MMUTT_SECTIONBASE_SHIFT)
#define MMUTT_COARSETBLBASE_SHIFT  10
#define MMUTT_COARSETBLBASE_NBITS  (32-MMUTT_COARSETBLBASE_SHIFT)
#define MMUTT_COARSETBLBASE_MASK   (((1<<MMUTT_COARSETBLBASE_NBITS)-1)<<MMUTT_COARSETBLBASE_SHIFT)
#define MMUTT_FINETBLBASE_SHIFT    12
#define MMUTT_FINETBLBASE_NBITS    (32-MMUTT_FINETBLBASE_SHIFT)
#define MMUTT_FINETBLBASE_MASK     (((1<<MMUTT_FINETBLBASE_NBITS)-1)<<MMUTT_FINETBLBASE_SHIFT)

// Domain control
#define MMUTT_DOMAIN_SHIFT 5
#define MMUTT_DOMAIN_NBITS 4
#define MMUTT_DOMAIN_MASK  (((1<<MMUTT_DOMAIN_NBITS)-1)<<MMUTT_DOMAIN_SHIFT)

// Cachability
#define MMUTT_CACHATTR_SHIFT   2
#define MMUTT_CACHATTR_NBITS   2
#define MMUTT_CACHATTR_MASK    (((1<<MMUTT_CACHATTR_NBITS)-1)<<MMUTT_CACHATTR_SHIFT)
#define MMUTT_CACHATTR_B       (1<<MMUTT_CACHATTR_SHIFT)
#define MMUTT_CACHATTR_C       (2<<MMUTT_CACHATTR_SHIFT)

// Access permission
#define MMUTT_AP_NBITS              2
#define MMUTT_AP_SHIFT             10
#define MMUTT_AP3_SHIFT            MMUTT_AP_SHIFT
#define MMUTT_AP2_SHIFT           (MMUTT_AP_SHIFT-2)
#define MMUTT_AP1_SHIFT           (MMUTT_AP_SHIFT-4)
#define MMUTT_AP0_SHIFT           (MMUTT_AP_SHIFT-6)
#define MMUTT_AP_MASK             (((1<<MMUTT_AP_NBITS)-1)<<MMUTT_AP_SHIFT)
#define MMUTT_AP_DEFAULT_UNSHFT   0
#define MMUTT_AP_SUPERV_UNSHFT    1
#define MMUTT_AP_RO_USER_UNSHFT   2
#define MMUTT_AP_ALL_UNSHFT       3
#define MMUTT_AP_DEFAULT          (MMUTT_AP_DEFAULT_UNSHFT<<MMUTT_AP_SHIFT)
#define MMUTT_AP_SUPERV           (MMUTT_AP_SUPERV_UNSHFT<<MMUTT_AP_SHIFT)
#define MMUTT_AP_RO_USER          (MMUTT_AP_RO_USER_UNSHFT<<MMUTT_AP_SHIFT)
#define MMUTT_AP_ALL              (MMUTT_AP_ALL_UNSHFT<<MMUTT_AP_SHIFT)
#endif
