/*
 * Most of this source has been derived from the Linux USB
 * project:
 *   (c) 1999-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *   (c) 2000 David L. Brown, Jr. (usb-storage@davidb.org)
 *   (c) 1999 Michael Gee (michael@linuxspecific.com)
 *   (c) 2000 Yggdrasil Computing, Inc.
 *
 *
 * Adapted for U-Boot:
 *   (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 *
 * For BBB support (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * BBB support based on /sys/dev/usb/umass.c from
 * FreeBSD.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/* Note:
 * Currently only the CBI transport protocoll has been implemented, and it
 * is only tested with a TEAC USB Floppy. Other Massstorages with CBI or CB
 * transport protocoll may work as well.
 */
/*
 * New Note:
 * Support for USB Mass Storage Devices (BBB) has been added. It has
 * only been tested with USB memory sticks.
 */


#include "general.h"
#include "board.h"
#include "task.h"
#include "osapi.h"
#include "sysapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "uiapi.h"

#include <usb_main.h>
#include "ehci.h"
#include "farady_host_api.h"

#include <part.h>


#undef BBB_COMDAT_TRACE
#undef BBB_XPORT_TRACE




#include <scsi_cmd.h>
extern int DCF_GetDeviceIndex(char *DevName);
#if (USB_HOST == 1)
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
extern int sysStorageOnlineStat[];
extern s32 dcfStorageSize[STORAGE_MEMORY_MAX];

static const unsigned char us_direction[256/8] =
{
    0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
    0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

__align(8) static ccb usb_ccb;

/*
 * CBI style
 */

#define US_CBI_ADSC		0

/*
 * BULK only
 */
#define US_BBB_RESET		0xff
#define US_BBB_GET_MAX_LUN	0xfe

/* Command Block Wrapper */
typedef struct
{
    u32		dCBWSignature;
#	define CBWSIGNATURE	0x43425355
    u32		dCBWTag;
    u32		dCBWDataTransferLength;
    u8		bCBWFlags;
#	define CBWFLAGS_OUT	0x00
#	define CBWFLAGS_IN	0x80
    u8		bCBWLUN;
    u8		bCDBLength;
#	define CBWCDBLENGTH	16
    u8		CBWCDB[CBWCDBLENGTH];
} umass_bbb_cbw_t;
#define UMASS_BBB_CBW_SIZE	31
static u32 CBWTag;

/* Command Status Wrapper */
typedef struct
{
    u32		dCSWSignature;
#	define CSWSIGNATURE	0x53425355
    u32		dCSWTag;
    u32		dCSWDataResidue;
    u8		bCSWStatus;
#	define CSWSTATUS_GOOD	0x0
#	define CSWSTATUS_FAILED 0x1
#	define CSWSTATUS_PHASE	0x2
} umass_bbb_csw_t;
#define UMASS_BBB_CSW_SIZE	13

#define USB_MAX_STOR_DEV 3
static int usb_max_devs; /* number of highest available usb device */
static int usb_hdd_dev_num; /* number of usb hdd device */
int usb_hdd_removed; /* remove on*/
static u32	keepCount = 0;
static block_dev_desc_t usb_dev_desc[USB_MAX_STOR_DEV];

block_dev_desc_t *gusb_dev_desc;


u32 usbfsTotalBlockCount;
//extern u32 sdcTotalBlockCount; /* ??? */
extern u8  gUSBDevOn;

#if 1//(SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)
//xxxxxxx.y MB
u32 g_usbfsCapicity=0;
#endif

struct us_data;
typedef int (*trans_cmnd)(ccb *cb, struct us_data *data);
typedef int (*trans_reset)(struct us_data *data);

struct us_data
{
    struct usb_device *pusb_dev;	 /* this usb_device */

    unsigned int	flags;			/* from filter initially */
    unsigned char	ifnum;			/* interface number */
    unsigned char	ep_in;			/* in endpoint */
    unsigned char	ep_out;			/* out ....... */
    unsigned char	ep_int;			/* interrupt . */
    unsigned char	subclass;		/* as in overview */
    unsigned char	protocol;		/* .............. */
    unsigned char	attention_done;		/* force attn on first cmd */
    unsigned short	ip_data;		/* interrupt data */
    int		action;			/* what to do */
    int		ip_wanted;		/* needed */
    int		*irq_handle;		/* for USB int requests */
    unsigned int	irqpipe;	 	/* pipe for release_irq */
    unsigned char	irqmaxp;		/* max packed for irq Pipe */
    unsigned char	irqinterval;		/* Intervall for IRQ Pipe */
    unsigned long	max_xfer_blk;		/* Max blocks per xfer */
    ccb		*srb;			/* current srb */
    trans_reset	transport_reset;	/* reset routine */
    trans_cmnd	transport;		/* transport routine */
};

struct us_data usb_stor[USB_MAX_STOR_DEV];


#define USB_STOR_TRANSPORT_GOOD	   0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2

int usb_stor_get_info(struct usb_device *dev, struct us_data *us,
                      block_dev_desc_t *dev_desc);
int usb_storage_probe(struct usb_device *dev, unsigned int ifnum, struct us_data *ss);
unsigned long usb_stor_read(int device, unsigned long blknr,
                            unsigned long blkcnt, void *buffer);
unsigned long usb_stor_write(int device, unsigned long blknr,
                             unsigned long blkcnt, const void *buffer);
struct usb_device * usb_get_dev_index(int index);

extern OS_EVENT* SCSI_SemEvt; /* semaphore to synchronize event processing */

extern char usb_init_flag;


block_dev_desc_t *usb_stor_get_dev(int index)
{
    return (index < usb_max_devs) ? &usb_dev_desc[index] : NULL;
}


void usb_show_progress(void)
{
    DEBUG_STORAGE(".");
}



/*
 * Overflowless variant of (block_count * mul_by / div_by)
 * when div_by > mul_by
 */
static lba512_t lba512_muldiv (lba512_t block_count, lba512_t mul_by, lba512_t div_by)
{
    lba512_t bc_quot, bc_rem;

    /* x * m / d == x / d * m + (x % d) * m / d */
    bc_quot = block_count / div_by;
    bc_rem  = block_count - div_by * bc_quot;
    return bc_quot * mul_by + (bc_rem * mul_by) / div_by;
}

void dev_print (block_dev_desc_t *dev_desc)
{
    lba512_t lba512; /* number of blocks if 512bytes block size */
    u32 capacity;

    if (dev_desc->type == DEV_TYPE_UNKNOWN)
    {
        DEBUG_STORAGE ("not available\n");
        return;
    }

    switch (dev_desc->if_type)
    {
        case IF_TYPE_SCSI:
            DEBUG_STORAGE ("(%d:%d) Vendor: %s Prod.: %s Rev: %s\n",
                           dev_desc->target,dev_desc->lun,
                           dev_desc->vendor,
                           dev_desc->product,
                           dev_desc->revision);
            break;
        case IF_TYPE_ATAPI:
        case IF_TYPE_IDE:
        case IF_TYPE_SATA:
            DEBUG_STORAGE ("Model: %s Firm: %s Ser#: %s\n",
                           dev_desc->vendor,
                           dev_desc->revision,
                           dev_desc->product);
            break;
        case IF_TYPE_SD:
        case IF_TYPE_MMC:
        case IF_TYPE_USB:
            DEBUG_STORAGE ("Vendor: %s Rev: %s Prod: %s\n",
                           dev_desc->vendor,
                           dev_desc->revision,
                           dev_desc->product);
            break;
        case IF_TYPE_DOC:
            DEBUG_STORAGE("device type DOC\n");
            return;
        case IF_TYPE_UNKNOWN:
            DEBUG_STORAGE("device type unknown\n");
            return;
        default:
            DEBUG_STORAGE("Unhandled device type: %i\n", dev_desc->if_type);
            return;
    }
    DEBUG_STORAGE ("            Type: ");
    if (dev_desc->removable)
        DEBUG_STORAGE ("Removable ");

    switch (dev_desc->type & 0x1F)
    {
        case DEV_TYPE_HARDDISK:
            DEBUG_STORAGE ("Hard Disk");
            break;
        case DEV_TYPE_CDROM:
            DEBUG_STORAGE ("CD ROM");
            break;
        case DEV_TYPE_OPDISK:
            DEBUG_STORAGE ("Optical Device");
            break;
        case DEV_TYPE_TAPE:
            DEBUG_STORAGE ("Tape");
            break;
        default:
            DEBUG_STORAGE ("# %02X #", dev_desc->type & 0x1F);
            break;
    }
    DEBUG_STORAGE ("\n");

    if ((dev_desc->lba * dev_desc->blksz)>0L)
    {
        u32 mb, mb_quot, mb_rem, gb, gb_quot, gb_rem;
        lbaint_t lba;

        lba = dev_desc->lba;

        lba512 = (lba * (dev_desc->blksz/512));
        usbfsTotalBlockCount = lba512;
        //sdcTotalBlockCount = usbfsTotalBlockCount;

        capacity=lba512/2; //unit: KB

        if (capacity > 0xffffffff) /* 4GB tolerance for 5T GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_5TB;
        }
        else if (capacity > 3800000000u) /* 4GB tolerance for 4T GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_4TB;
        }
        else if (capacity > 2900000000u) /* 4GB tolerance for 3T GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_3TB;
        }
        else if (capacity > 1900000000u) /* 4GB tolerance for 2T GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_2TB;
        }
        else if (capacity > 910046848) /* 4GB tolerance for 1T GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_1TB;
        }
        else if (capacity > 510046848) /* 4GB tolerance for 512 GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_512GB;
        }
        else if (capacity > 252046848) /* 4GB tolerance for 256GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_256GB;
        }
        else if (capacity > 125023424) /* 4GB tolerance for 128GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_128GB;
        }
        else if (capacity > 61962087) /* 2048MB tolerance for 64GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_64GB;
        }
        else if (capacity > 31459280) /* 2048MB tolerance for 32GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_32GB;
        }
        else if (capacity > 14680064) /* 2048MB tolerance for 16GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_16GB;
        }
        else if (capacity > 7562500) /* 256MB tolerance for 8GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_8GB;
        }
        else if (capacity > 3656250) /* 256MB tolerance for 4GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_4GB;
        }
        else if (capacity > 1703125) /* 256MB tolerance for 2GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_2GB;
        }
        else if (capacity > 726562) /* 256MB tolerance for 1GB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_1GB;
        }
        else if (capacity > 375000) /* 128MB tolerance for 512MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_512MB;
        }
        else if (capacity > 187500) /* 64MB tolerance for 256MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_256MB;
        }
        else if (capacity > 93750) /* 32MB tolerance for 128MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_128MB;
        }
        else if (capacity > 46875) /* 16MB tolerance for 64MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_64MB;
        }
        else if (capacity > 23437) /* 8MB tolerance for 32MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_32MB;
        }
        else if (capacity > 11718) /* 4MB tolerance for 16MB */
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = FS_MEDIA_SD_16MB;
        }
        else
        {
            dcfStorageSize[STORAGE_MEMORY_USB_HOST] = 0;
        }

        DEBUG_USB("==>USB dcfStorageSize=%d,%d\n",dcfStorageSize[STORAGE_MEMORY_USB_HOST],capacity);
        /* round to 1 digit */
        mb = lba512_muldiv(lba512, 10, 2048);	/* 2048 = (1024 * 1024) / 512 MB */

#if 1//(SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)
        g_usbfsCapicity=mb;
#endif

        mb_quot	= mb / 10;
        mb_rem	= mb - (10 * mb_quot);

        gb = mb / 1024;
        gb_quot	= gb / 10;
        gb_rem	= gb - (10 * gb_quot);
#ifdef CONFIG_LBA48
        if (dev_desc->lba48)
            DEBUG_STORAGE (" Supports 48-bit addressing\n");
#endif
#if defined(CONFIG_SYS_64BIT_LBA)
        DEBUG_STORAGE (" Capacity: %ld.%ld MB = %ld.%ld GB (%Ld x %ld)\n",
                       mb_quot, mb_rem,
                       gb_quot, gb_rem,
                       lba,
                       dev_desc->blksz);
#else
        DEBUG_STORAGE (" Capacity: %ld.%ld MB = %ld.%ld GB (%ld x %ld)\n",
                       mb_quot, mb_rem,
                       gb_quot, gb_rem,
                       (u32)lba,
                       dev_desc->blksz);
#endif
    }
    else
    {
#if 1//(SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)
        g_usbfsCapicity=0;
#endif
        DEBUG_STORAGE ("  Capacity: not available\n");
    }
}

u32 usbfsGetTotalBlockCount()
{
    return usbfsTotalBlockCount;

}
extern void format_printf(unsigned long offset,unsigned long length,unsigned int buf);
//extern s32 sys_USBHOST_STORAGE_IN(s32 dummy);
#if 0
s32 usb_stor_read_file(void) //Raymond Added for test
{
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x100000);
    char raytest[]="raymond test file use usb write 123456789";
    FS_FILE* pFile;
    u32 codeSize,Size;
    extern u8 ucNORInit;
    u8  tmp;

    DEBUG_STORAGE("usb_stor_read_file\n");

    if(dcfStorageType != STORAGE_MEMORY_USB_HOST)
        return 0;

    if((pFile = dcfOpen("\\raymond.txt","r")) == NULL)
    {
        DEBUG_STORAGE("Error: dcf open error!\n");
        return 0;
    }

    dcfRead(pFile, codeAddr, pFile->size, &codeSize);
    DEBUG_STORAGE("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

    /* close file */
    dcfClose(pFile, &tmp);
    if (codeSize == 0)
    {
        DEBUG_STORAGE("USB read Err: Code Size is 0 Byte!\n");
        return 0;
    }
    DEBUG_STORAGE("==================\n");

    if ((pFile = dcfOpen("\\raymond2.txt", "w")) == NULL)
    {
        /* create next file error */
        return NULL;
    }
    if (dcfWrite(pFile, (unsigned char*)&raytest, sizeof(raytest), &Size) == 0)
    {
        DEBUG_STORAGE(" Write test error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    DEBUG_STORAGE("*************************\n");
    /* close file */
    dcfClose(pFile, &tmp);
    if (codeSize == 0)
    {
        DEBUG_STORAGE("USB write Err: Code Size is 0 Byte!\n");
        return 0;
    }

    return 1;

}

s32 usb_test_read_file(void) //Raymond Added for test
{
    u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x100000);
    //char raytest[]="raymond test file use usb write 123456789 abcdefghijklmnopqrstuvwxyz";
    FS_FILE* pFile;
    u32 codeSize;
    extern u8 ucNORInit;
    u8 i;
    u8  tmp;
    DEBUG_STORAGE("usb_stor_read_file\n");

    if(dcfStorageType != STORAGE_MEMORY_USB_HOST)
        return 0;
    for(i=0; i<10; i++)
    {
        if((pFile = dcfOpen("\\raymond2.txt","r")) == NULL)
        {
            DEBUG_STORAGE("Error: dcf open error!\n");
            //return 0;
        }

        dcfRead(pFile, codeAddr, pFile->size, &codeSize);
        DEBUG_STORAGE("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfClose(pFile, &tmp);
        DEBUG_STORAGE("%s\n",codeAddr);
        if (codeSize == 0)
        {
            DEBUG_STORAGE("USB read Err: Code Size is 0 Byte!\n");
            //return 0;
        }
        DEBUG_STORAGE("==================\n");

        if((pFile = dcfOpen("\\raymond3.txt","r")) == NULL)
        {
            DEBUG_STORAGE("Error: dcf open error!\n");
            //return 0;
        }

        dcfRead(pFile, codeAddr, pFile->size, &codeSize);
        DEBUG_STORAGE("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

        /* close file */
        dcfClose(pFile, &tmp);
        DEBUG_STORAGE("%s\n",codeAddr);
        if (codeSize == 0)
        {
            DEBUG_STORAGE("USB read Err: Code Size is 0 Byte!\n");
            //return 0;
        }
        DEBUG_STORAGE("==================\n");
    }
    return 1;

}
#endif
/*******************************************************************************
 * show info on storage devices; 'usb start/init' must be invoked earlier
 * as we only retrieve structures populated during devices initialization
 */
int usb_stor_info(void)
{
	int i;

	if (usb_max_devs > 0)
	{
		for (i = 0; i < usb_max_devs; i++)
		{
			DEBUG_UHOST("  Device %d: ", i);
			dev_print(&usb_dev_desc[i]);
			gusb_dev_desc=&usb_dev_desc[i];
		}
		return 0;
	}

	DEBUG_UHOST("No storage devices, perhaps not 'usb start'ed..?\n");
	return 1;
}

static unsigned int usb_get_max_lun(struct us_data *us)
{
    int len;
    __align(8) unsigned char result[1];

    DEBUG_STORAGE("Get Max LUN \n");
    len = usb_control_msg(us->pusb_dev,
                          usb_rcvctrlpipe(us->pusb_dev, 0),
                          US_BBB_GET_MAX_LUN,
                          USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
                          0, us->ifnum,
                          result, sizeof(char),
                          USB_CNTL_TIMEOUT * 5);
    DEBUG_STORAGE("Get Max LUN -> len = %i, result = %i\n",
                  len, (int) *result);
    return (len > 0) ? *result : 0;
}

/*******************************************************************************
 * scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
int usb_stor_scan(int mode)
{
    unsigned char i;
    struct usb_device *dev;

    if (mode == 1)
        DEBUG_UHOST("       scanning bus for storage devices... ");

    usb_disable_asynch(1); /* asynch transfer not allowed */

    for (i = 0; i < USB_MAX_STOR_DEV; i++)
    {
        memset(&usb_dev_desc[i], 0, sizeof(block_dev_desc_t));
        usb_dev_desc[i].if_type = IF_TYPE_USB;
        usb_dev_desc[i].dev = i;
        usb_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
        usb_dev_desc[i].target = 0xff;
        usb_dev_desc[i].type = DEV_TYPE_UNKNOWN;
        usb_dev_desc[i].block_read = usb_stor_read;
        usb_dev_desc[i].block_write = usb_stor_write;
    }

    usb_max_devs = 0;
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        dev = usb_get_dev_index(i); /* get device */
       // DEBUG_STORAGE("i=%d\n", i);
        if (dev == NULL)
            break; /* no more devices available */

        if (usb_storage_probe(dev, 0, &usb_stor[usb_max_devs]))
        {
            /* OK, it's a storage device.  Iterate over its LUNs
             * and populate `usb_dev_desc'.
             */
            int lun, max_lun, start = usb_max_devs;

            max_lun = usb_get_max_lun(&usb_stor[usb_max_devs]);
            for (lun = 0;
                    lun <= max_lun && usb_max_devs < USB_MAX_STOR_DEV;
                    lun++)
            {
                usb_dev_desc[usb_max_devs].lun = lun;
                if (usb_stor_get_info(dev, &usb_stor[start], &usb_dev_desc[usb_max_devs]) == 1)
                {
                    usb_max_devs++;
                }
            }
          usb_hdd_dev_num=start;
	  usb_hdd_removed =0;  
		DEBUG_STORAGE("usb_hdd_dev_num=%d\n", usb_hdd_dev_num);			
        }
        /* if storage device */
        if (usb_max_devs == USB_MAX_STOR_DEV)
        {
            DEBUG_UHOST("max USB Storage Device reached: %d stopping\n",
                        usb_max_devs);
            break;
        }
    } /* for */

    usb_disable_asynch(0); /* asynch transfer allowed */
    DEBUG_UHOST("%d Storage Device(s) found\n", usb_max_devs);
    if (usb_max_devs > 0)
        return 0;
    return -1;
}

static int usb_stor_irq(struct usb_device *dev)
{
    struct us_data *us;
    us = (struct us_data *)dev->privptr;

    if (us->ip_wanted)
        us->ip_wanted = 0;
    return 0;
}


#ifdef	USB_STOR_DEBUG

static void usb_show_srb(ccb *pccb)
{
    int i;
    DEBUG_UHOST("SRB: len %d datalen 0x%lX\n ", pccb->cmdlen, pccb->datalen);
    for (i = 0; i < 12; i++)
        DEBUG_UHOST("%02X ", pccb->cmd[i]);
    DEBUG_UHOST("\n");
}

static void display_int_status(unsigned long tmp)
{
    DEBUG_UHOST("Status: %s %s %s %s %s %s %s\n",
                (tmp & USB_ST_ACTIVE) ? "Active" : "",
                (tmp & USB_ST_STALLED) ? "Stalled" : "",
                (tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
                (tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
                (tmp & USB_ST_NAK_REC) ? "NAKed" : "",
                (tmp & USB_ST_CRC_ERR) ? "CRC Error" : "",
                (tmp & USB_ST_BIT_ERR) ? "Bitstuff Error" : "");
}
#endif
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

static int us_one_transfer(struct us_data *us, int pipe, char *buf, int length)
{
    int max_size;
    int this_xfer;
    int result;
    int partial;
    int maxtry;
    int stat;

    /* determine the maximum packet size for these transfers */
    max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

    /* while we have data left to transfer */
    while (length)
    {

        /* calculate how long this will be -- maximum or a remainder */
        this_xfer = length > max_size ? max_size : length;
        length -= this_xfer;

        /* setup the retry counter */
        maxtry = 10;

        /* set up the transfer loop */
        do
        {
            /* transfer the data */
            DEBUG_STORAGE("Bulk xfer 0x%x(%d) try #%d\n",
                          (unsigned int)buf, this_xfer, 11 - maxtry);
            result = usb_bulk_msg(us->pusb_dev, pipe, buf, this_xfer, &partial,
                                  USB_CNTL_TIMEOUT * 5);
            DEBUG_STORAGE("bulk_msg returned %d xferred %d/%d\n",
                          result, partial, this_xfer);
            if (us->pusb_dev->status != 0)
            {
                /* if we stall, we need to clear it before
                 * we go on
                 */
#ifdef USB_STOR_DEBUG
                display_int_status(us->pusb_dev->status);
#endif
                if (us->pusb_dev->status & USB_ST_STALLED)
                {
                    DEBUG_STORAGE("stalled ->clearing endpoint halt for pipe 0x%x\n", pipe);
                    stat = us->pusb_dev->status;
                    usb_clear_halt(us->pusb_dev, pipe);
                    us->pusb_dev->status = stat;
                    if (this_xfer == partial)
                    {
                        DEBUG_STORAGE("bulk xfer with error %lX, but data ok\n", us->pusb_dev->status);
                        return 0;
                    }
                    else
                        return result;
                }
                if (us->pusb_dev->status & USB_ST_NAK_REC)
                {
                    DEBUG_STORAGE("Dev NAKed bulk_msg\n");
                    return result;
                }
                DEBUG_STORAGE("bulk xfer with error");
                if (this_xfer == partial)
                {
                    DEBUG_STORAGE(" %ld, but data ok\n",
                                  us->pusb_dev->status);
                    return 0;
                }
                /* if our try counter reaches 0, bail out */
                DEBUG_STORAGE(" %ld, data %d\n",
                              us->pusb_dev->status, partial);
                if (!maxtry--)
                    return result;
            }
            /* update to show what data was transferred */
            this_xfer -= partial;
            buf += partial;
            /* continue until this transfer is done */
        }
        while (this_xfer);
    }

    /* if we get here, we're done and successful */
    return 0;
}

static int usb_stor_BBB_reset(struct us_data *us)
{
    int result;
    unsigned int pipe;

    /*
     * Reset recovery (5.3.4 in Universal Serial Bus Mass Storage Class)
     *
     * For Reset Recovery the host shall issue in the following order:
     * a) a Bulk-Only Mass Storage Reset
     * b) a Clear Feature HALT to the Bulk-In endpoint
     * c) a Clear Feature HALT to the Bulk-Out endpoint
     *
     * This is done in 3 steps.
     *
     * If the reset doesn't succeed, the device should be port reset.
     *
     * This comment stolen from FreeBSD's /sys/dev/usb/umass.c.
     */
    DEBUG_STORAGE("BBB_reset\n");
    result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
                             US_BBB_RESET,
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0, us->ifnum, 0, 0, USB_CNTL_TIMEOUT );

    if ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG_STORAGE("RESET:stall\n");
        return -1;
    }
    if (gUSBDevOn== 0)
        {
            printf("@Dev out\n");
            return -1;
        }
    /* long wait for reset */
    //ehci_mdelay(150);
    DEBUG_STORAGE("BBB_reset result %d: status %lX reset\n", result,us->pusb_dev->status);
    pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
    result = usb_clear_halt(us->pusb_dev, pipe);
    /* long wait for reset */
    //ehci_mdelay(150);
        if (gUSBDevOn== 0)
        {
            printf("@Dev out\n");
            return -1;
        }
    DEBUG_STORAGE("BBB_reset result %d: status %lX clearing IN endpoint\n",	result, us->pusb_dev->status);
    /* long wait for reset */
    pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
    result = usb_clear_halt(us->pusb_dev, pipe);
    //ehci_mdelay(150);
        if (gUSBDevOn== 0)
        {
            printf("@Dev out\n");
            return -1;
        }
    DEBUG_STORAGE("BBB_reset result %d: status %lXd clearing OUT endpoint\n", result,us->pusb_dev->status);
    DEBUG_STORAGE("BBB_reset done\n");
    return 0;
}

#if 0
/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
static int usb_stor_CB_reset(struct us_data *us)
{
    unsigned char cmd[12];
    int result;

    DEBUG_STORAGE("CB_reset\n");
    memset(cmd, 0xff, sizeof(cmd));
    cmd[0] = SCSI_SEND_DIAG;
    cmd[1] = 4;
    result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
                             US_CBI_ADSC,
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0, us->ifnum, cmd, sizeof(cmd),
                             USB_CNTL_TIMEOUT * 5);

    /* long wait for reset */
    mdelay(1500);
    DEBUG_STORAGE("CB_reset result %d: status %lX"
                  " clearing endpoint halt\n", result,
                  us->pusb_dev->status);
    usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
    usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

    DEBUG_STORAGE("CB_reset done\n");
    return 0;
}
#endif
/*
 * Set up the command for a BBB device. Note that the actual SCSI
 * command is copied into cbw.CBWCDB.
 */
int usb_stor_BBB_comdat(ccb *srb, struct us_data *us)
{
    int result=0;
    int actlen;
    int dir_in;
    unsigned int pipe;
    umass_bbb_cbw_t cbw[1];

    dir_in = US_DIRECTION(srb->cmd[0]);

#ifdef BBB_COMDAT_TRACE
    DEBUG_UHOST("dir %d lun %d cmdlen %d cmd %p datalen %d pdata %p\n",dir_in, srb->lun, srb->cmdlen, srb->cmd, srb->datalen,srb->pdata);
    if (srb->cmdlen)
    {
        for (result = 0; result < srb->cmdlen; result++)
            DEBUG_UHOST("cmd[%d] %#x ", result, srb->cmd[result]);
        DEBUG_UHOST("\n");
    }
#endif
    /* sanity checks */
    if (!(srb->cmdlen <= CBWCDBLENGTH))
    {
        DEBUG_STORAGE("usb_stor_BBB_comdat:cmdlen too large\n");
        return -1;
    }

    /* always OUT to the ep */
    pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

    cbw->dCBWSignature = cpu_to_le32(CBWSIGNATURE);
    cbw->dCBWTag = cpu_to_le32(CBWTag++);
    cbw->dCBWDataTransferLength = cpu_to_le32(srb->datalen);
    cbw->bCBWFlags = (dir_in ? CBWFLAGS_IN : CBWFLAGS_OUT);
    cbw->bCBWLUN = srb->lun;
    cbw->bCDBLength = srb->cmdlen;
    /* copy the command data into the CBW command data buffer */
    /* DST SRC LEN!!! */
    memcpy(cbw->CBWCDB, srb->cmd, srb->cmdlen);
    result = usb_bulk_msg(us->pusb_dev, pipe, cbw, UMASS_BBB_CBW_SIZE,&actlen, USB_CNTL_TIMEOUT * 5);
    if (result < 0)
        DEBUG_STORAGE("usb_stor_BBB_comdat:usb_bulk_msg error\n");
    return result;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
int usb_stor_CB_comdat(ccb *srb, struct us_data *us)
{
    int result = 0;
    int dir_in, retry;
    unsigned int pipe;
    unsigned long status;

    retry = 5;
    dir_in = US_DIRECTION(srb->cmd[0]);

    if (dir_in)
        pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
    else
        pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

    while (retry--)
    {
        DEBUG_STORAGE("CBI gets a command: Try %d\n", 5 - retry);
#ifdef USB_STOR_DEBUG
        usb_show_srb(srb);
#endif
        /* let's send the command via the control pipe */
        result = usb_control_msg(us->pusb_dev,
                                 usb_sndctrlpipe(us->pusb_dev , 0),
                                 US_CBI_ADSC,
                                 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                                 0, us->ifnum,
                                 srb->cmd, srb->cmdlen,
                                 USB_CNTL_TIMEOUT * 5);
        DEBUG_STORAGE("CB_transport: control msg returned %d,"
                      " status %lX\n", result, us->pusb_dev->status);
        /* check the return code for the command */
        if (result < 0)
        {
            if (us->pusb_dev->status & USB_ST_STALLED)
            {
                status = us->pusb_dev->status;
                DEBUG_STORAGE(" stall during command found,"
                              " clear pipe\n");
                usb_clear_halt(us->pusb_dev,
                               usb_sndctrlpipe(us->pusb_dev, 0));
                us->pusb_dev->status = status;
            }
            DEBUG_STORAGE(" error during command %02X"
                          " Stat = %lX\n", srb->cmd[0],
                          us->pusb_dev->status);
            return result;
        }
        /* transfer the data payload for this command, if one exists*/

        DEBUG_STORAGE("CB_transport: control msg returned %d,"
                      " direction is %s to go 0x%lx\n", result,
                      dir_in ? "IN" : "OUT", srb->datalen);
        if (srb->datalen)
        {
            result = us_one_transfer(us, pipe, (char *)srb->pdata,
                                     srb->datalen);
            DEBUG_STORAGE("CBI attempted to transfer data,"
                          " result is %d status %lX, len %d\n",
                          result, us->pusb_dev->status,
                          us->pusb_dev->act_len);
            if (!(us->pusb_dev->status & USB_ST_NAK_REC))
                break;
        } /* if (srb->datalen) */
        else
            break;
    }
    /* return result */

    return result;
}

#if 0
int usb_stor_CBI_get_status(ccb *srb, struct us_data *us)
{
    int timeout;

    us->ip_wanted = 1;
    submit_int_msg(us->pusb_dev, us->irqpipe,
                   (void *) &us->ip_data, us->irqmaxp, us->irqinterval);
    timeout = 1000;
    while (timeout--)
    {
        if ((volatile int *) us->ip_wanted == 0)
            break;
        mdelay(10);
    }
    if (us->ip_wanted)
    {
        DEBUG_UHOST("	Did not get interrupt on CBI\n");
        us->ip_wanted = 0;
        return USB_STOR_TRANSPORT_ERROR;
    }
    DEBUG_STORAGE
    ("Got interrupt data 0x%x, transfered %d status 0x%lX\n",
     us->ip_data, us->pusb_dev->irq_act_len,
     us->pusb_dev->irq_status);
    /* UFI gives us ASC and ASCQ, like a request sense */
    if (us->subclass == US_SC_UFI)
    {
        if (srb->cmd[0] == SCSI_REQ_SENSE ||
                srb->cmd[0] == SCSI_INQUIRY)
            return USB_STOR_TRANSPORT_GOOD; /* Good */
        else if (us->ip_data)
            return USB_STOR_TRANSPORT_FAILED;
        else
            return USB_STOR_TRANSPORT_GOOD;
    }
    /* otherwise, we interpret the data normally */
    switch (us->ip_data)
    {
        case 0x0001:
            return USB_STOR_TRANSPORT_GOOD;
        case 0x0002:
            return USB_STOR_TRANSPORT_FAILED;
        default:
            return USB_STOR_TRANSPORT_ERROR;
    }			/* switch */
    return USB_STOR_TRANSPORT_ERROR;
}

#endif

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

/* clear a stall on an endpoint - special for BBB devices */
int usb_stor_BBB_clear_endpt_stall(struct us_data *us, u8 endpt)
{
    int result;

    /* ENDPOINT_HALT = 0, so set value to 0 */
    DEBUG_STORAGE("usb_stor_BBB_clear_endpt_stall \n");
    result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0),
                             USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT,
                             0, endpt, 0, 0, USB_CNTL_TIMEOUT * 5);
    return result;
}

int usb_stor_BBB_transport(ccb *srb, struct us_data *us)
{
    int result, retry;
    int dir_in;
    int actlen, data_actlen;
    unsigned int pipe, pipein, pipeout;
    __align(4) umass_bbb_csw_t csw[1];
#ifdef BBB_XPORT_TRACE
    unsigned char *ptr;
    int index;
#endif
    //     DEBUG_STORAGE("COMMAND phase\n");
    dir_in = US_DIRECTION(srb->cmd[0]);

    /* COMMAND phase */
    result = usb_stor_BBB_comdat(srb, us);
    if (result < 0)
    {
        DEBUG_STORAGE("COMMAND phase\n");

        DEBUG_STORAGE("failed to send CBW status %ld\n",us->pusb_dev->status);
        usb_stor_BBB_reset(us);
        DEBUG_STORAGE("return send CBW fAIL n");
        return USB_STOR_TRANSPORT_FAILED;
    }
    //	ehci_mdelay(5);
    pipein = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
    pipeout = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
    /* DATA phase + error handling */
    data_actlen = 0;
    /* no data, go immediately to the STATUS phase */
    if (srb->datalen == 0)
        goto st;
    //DEBUG_STORAGE("DATA phase\n");
    if (dir_in)
        pipe = pipein;
    else
        pipe = pipeout;
    result = usb_bulk_msg(us->pusb_dev, pipe, srb->pdata, srb->datalen, &data_actlen, USB_CNTL_TIMEOUT * 5);
    /* special handling of STALL in DATA phase */
    if ((result < 0) && (us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG_STORAGE("DATA:stall\n");
        /* clear the STALL on the endpoint */
        result = usb_stor_BBB_clear_endpt_stall(us,	dir_in ? us->ep_in : us->ep_out);
        if (result >= 0)
            /* continue on to STATUS phase */
            goto st;
    }
    if (result < 0)
    {
        DEBUG_STORAGE("usb_bulk_msg error status %ld\n",us->pusb_dev->status);
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
#ifdef BBB_XPORT_TRACE
    for (index = 0; index < data_actlen; index++)
        DEBUG_UHOST("pdata[%d] %#x ", index, srb->pdata[index]);
    DEBUG_UHOST("\n");
#endif
    /* STATUS phase + error handling */
st:
    retry = 0;
again:
    //DEBUG_STORAGE("STATUS phase\n");
    result = usb_bulk_msg(us->pusb_dev, pipein, csw, UMASS_BBB_CSW_SIZE, &actlen, USB_CNTL_TIMEOUT*5);

    /* special handling of STALL in STATUS phase */
    if ((result < 0) && (retry < 1) &&
            (us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG_STORAGE("STATUS:stall\n");
        /* clear the STALL on the endpoint */
        result = usb_stor_BBB_clear_endpt_stall(us, us->ep_in);
        if (result >= 0 && (retry++ < 1))
            /* do a retry */
            goto again;
    }
    if (result < 0)
    {
        DEBUG_STORAGE("usb_bulk_msg error status %x\n",us->pusb_dev->status);
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
#ifdef BBB_XPORT_TRACE
    ptr = (unsigned char *)csw;
    for (index = 0; index < UMASS_BBB_CSW_SIZE; index++)
        DEBUG_UHOST("ptr[%d] %#x ", index, ptr[index]);
    DEBUG_UHOST("\n");
#endif
    /* misuse pipe to get the residue */
    pipe = le32_to_cpu(csw->dCSWDataResidue);
    if (pipe == 0 && srb->datalen != 0 && srb->datalen - data_actlen != 0)
        pipe = srb->datalen - data_actlen;
    if (CSWSIGNATURE != le32_to_cpu(csw->dCSWSignature))
    {
        DEBUG_STORAGE("!CSWSIGNATURE\n");
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if ((CBWTag - 1) != le32_to_cpu(csw->dCSWTag))
    {
        DEBUG_STORAGE("!Tag\n");
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus > CSWSTATUS_PHASE)
    {
        DEBUG_STORAGE(">PHASE1\n");
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus == CSWSTATUS_PHASE)
    {
        DEBUG_STORAGE("=PHASE2\n");
        usb_stor_BBB_reset(us);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (data_actlen > srb->datalen)
    {
        DEBUG_STORAGE("transferred %dB instead of %ldB\n",
                      data_actlen, srb->datalen);
        return USB_STOR_TRANSPORT_FAILED;
    }
    else if (csw->bCSWStatus == CSWSTATUS_FAILED)
    {
        DEBUG_STORAGE("BBB:FAILED\n");
        return USB_STOR_TRANSPORT_FAILED;
    }

    return result;
}

#if 0
int usb_stor_CB_transport(ccb *srb, struct us_data *us)
{
    int result, status;
    ccb *psrb;
    ccb reqsrb;
    int retry, notready;

    psrb = &reqsrb;
    status = USB_STOR_TRANSPORT_GOOD;
    retry = 0;
    notready = 0;
    /* issue the command */
do_retry:
    result = usb_stor_CB_comdat(srb, us);
    DEBUG_STORAGE("command / Data returned %d, status %lX\n",
                  result, us->pusb_dev->status);
    /* if this is an CBI Protocol, get IRQ */
    if (us->protocol == US_PR_CBI)
    {
        status = usb_stor_CBI_get_status(srb, us);
        /* if the status is error, report it */
        if (status == USB_STOR_TRANSPORT_ERROR)
        {
            DEBUG_STORAGE(" USB CBI Command Error\n");
            return status;
        }
        srb->sense_buf[12] = (unsigned char)(us->ip_data >> 8);
        srb->sense_buf[13] = (unsigned char)(us->ip_data & 0xff);
        if (!us->ip_data)
        {
            /* if the status is good, report it */
            if (status == USB_STOR_TRANSPORT_GOOD)
            {
                DEBUG_STORAGE(" USB CBI Command Good\n");
                return status;
            }
        }
    }
    /* do we have to issue an auto request? */
    /* HERE we have to check the result */
    if ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG_STORAGE("ERROR %lX\n", us->pusb_dev->status);
        us->transport_reset(us);
        return USB_STOR_TRANSPORT_ERROR;
    }
    if ((us->protocol == US_PR_CBI) &&
            ((srb->cmd[0] == SCSI_REQ_SENSE) ||
             (srb->cmd[0] == SCSI_INQUIRY)))
    {
        /* do not issue an autorequest after request sense */
        DEBUG_STORAGE("No auto request and good\n");
        return USB_STOR_TRANSPORT_GOOD;
    }
    /* issue an request_sense */
    memset(&psrb->cmd[0], 0, 12);
    psrb->cmd[0] = SCSI_REQ_SENSE;
    psrb->cmd[1] = srb->lun << 5;
    psrb->cmd[4] = 18;
    psrb->datalen = 18;
    psrb->pdata = &srb->sense_buf[0];
    psrb->cmdlen = 12;
    /* issue the command */
    result = usb_stor_CB_comdat(psrb, us);
    DEBUG_STORAGE("auto request returned %d\n", result);
    /* if this is an CBI Protocol, get IRQ */
    if (us->protocol == US_PR_CBI)
        status = usb_stor_CBI_get_status(psrb, us);

    if ((result < 0) && !(us->pusb_dev->status & USB_ST_STALLED))
    {
        DEBUG_STORAGE(" AUTO REQUEST ERROR %ld\n",
                      us->pusb_dev->status);
        return USB_STOR_TRANSPORT_ERROR;
    }
    DEBUG_STORAGE("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
                  srb->sense_buf[0], srb->sense_buf[2],
                  srb->sense_buf[12], srb->sense_buf[13]);
    /* Check the auto request result */
    if ((srb->sense_buf[2] == 0) &&
            (srb->sense_buf[12] == 0) &&
            (srb->sense_buf[13] == 0))
    {
        /* ok, no sense */
        return USB_STOR_TRANSPORT_GOOD;
    }

    /* Check the auto request result */
    switch (srb->sense_buf[2])
    {
        case 0x01:
            /* Recovered Error */
            return USB_STOR_TRANSPORT_GOOD;
            break;
        case 0x02:
            /* Not Ready */
            if (notready++ > USB_TRANSPORT_NOT_READY_RETRY)
            {
                DEBUG_UHOST("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
                            " 0x%02X (NOT READY)\n", srb->cmd[0],
                            srb->sense_buf[0], srb->sense_buf[2],
                            srb->sense_buf[12], srb->sense_buf[13]);
                return USB_STOR_TRANSPORT_FAILED;
            }
            else
            {
                mdelay(100);
                goto do_retry;
            }
            break;
        default:
            if (retry++ > USB_TRANSPORT_UNKNOWN_RETRY)
            {
                DEBUG_UHOST("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X"
                            " 0x%02X\n", srb->cmd[0], srb->sense_buf[0],
                            srb->sense_buf[2], srb->sense_buf[12],
                            srb->sense_buf[13]);
                return USB_STOR_TRANSPORT_FAILED;
            }
            else
                goto do_retry;
            break;
    }
    return USB_STOR_TRANSPORT_FAILED;
}

#endif
static int usb_inquiry(ccb *srb, struct us_data *ss)
{
    int retry, i;
    retry = 3;
    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0] = SCSI_INQUIRY;
        srb->cmd[1] = srb->lun << 5;
        srb->cmd[4] = 36;
        srb->datalen = 36;
        srb->cmdlen = 12;
        i = ss->transport(srb, ss);
        DEBUG_STORAGE("inquiry returns %d\n", i);
        if (i == 0)
            break;
	 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
                break;
            }	
    }
    while (--retry);

    if (!retry)
    {
        DEBUG_UHOST("error in inquiry\n");
        return -1;
    }
    return 0;
}

static int usb_request_sense(ccb *srb, struct us_data *ss)
{
    char *ptr;

    ptr = (char *)srb->pdata;
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_REQ_SENSE;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[4] = 18;
    srb->datalen = 18;
    srb->pdata = &srb->sense_buf[0];
    srb->cmdlen = 12;
    ss->transport(srb, ss);
    DEBUG_STORAGE("Request Sense returned %02X %02X %02X\n", srb->sense_buf[2], srb->sense_buf[12],	srb->sense_buf[13]);
    srb->pdata = (u8 *)ptr;
    return 0;
}

static int usb_test_unit_ready(ccb *srb, struct us_data *ss)
{
    int retries = 3;

    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0] = SCSI_TST_U_RDY;
        srb->cmd[1] = srb->lun << 5;
        srb->datalen = 0;
        srb->cmdlen = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
        usb_request_sense(srb, ss);
		 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
                break;
            }
      if( srb->sense_buf[2]==02&& srb->sense_buf[12]==04 && srb->sense_buf[13]==01)
      	{
            //  DEBUG_STORAGE("[usb] Become Rdy\n");
		retries ++;
		OSTimeDly(10);	
	}
      else  
        ehci_mdelay(100);
    }
    while (retries--);

    return -1;
}

static int usb_read_capacity(ccb *srb, struct us_data *ss)
{
    int retry;
    /* XXX retries */
    retry = 3;
    do
    {
        memset(&srb->cmd[0], 0, 12);
        srb->cmd[0] = SCSI_RD_CAPAC;
        srb->cmd[1] = srb->lun << 5;
        srb->datalen = 8;
        srb->cmdlen = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
	 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
                break;
            }	
    }
    while (retry--);

    return -1;
}
#if 0
static int usb_remove(ccb *srb, struct us_data *ss)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_MED_REMOVL;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] =0;
    srb->cmd[3] = 0;
    srb->cmd[4] = 0;
    srb->cmd[5] = 0;
    srb->cmd[7] = 0;
    srb->cmd[8] = 0;
    srb->cmdlen = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
	 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
            }	
	  return -1;
}
#endif
static int usb_start_stop(ccb *srb, struct us_data *ss)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_START_STP;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] =0;
    srb->cmd[3] = 0;
    srb->cmd[4] = 0;
    srb->cmd[5] = 0;
    srb->cmd[7] = 0;
    srb->cmd[8] = 0;
    srb->cmdlen = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
	 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
            }	
	  return -1;
}
/*
static int usb_start_up(ccb *srb, struct us_data *ss)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_START_STP;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = 0;
    srb->cmd[3] = 0;
    srb->cmd[4] = 1;
    srb->cmd[5] = 0;
    srb->cmd[7] = 0;
    srb->cmd[8] = 0;
    srb->cmdlen = 12;
        if (ss->transport(srb, ss) == USB_STOR_TRANSPORT_GOOD)
            return 0;
	 if(gUSBDevOn == 0)
            {
                DEBUG_STORAGE("Fail:Card OUT\n");
            }	
	  return -1;
}
*/
static int usb_seek_10(ccb *srb, struct us_data *ss, unsigned long start)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_READ10;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;

    srb->cmdlen = 12;
   DEBUG_STORAGE("seek10: start %lx blocks %x\n", start);
    return ss->transport(srb, ss);
}

static int usb_read_10(ccb *srb, struct us_data *ss, unsigned long start,unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_READ10;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;
    srb->cmd[7] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[8] = (unsigned char) blocks & 0xff;
    srb->cmdlen = 12;
    //DEBUG_STORAGE("read10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}

/*
// was declared but never referenced
static int usb_read_16(ccb *srb, struct us_data *ss, unsigned long start,unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_READ16;
    srb->cmd[1] = 0x00;
    srb->cmd[2] =  0x00;
    srb->cmd[3] =  0x00;
    srb->cmd[4] =  0x00;
    srb->cmd[5] =  0x00;
    srb->cmd[6] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[7] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[8] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[9] = ((unsigned char) (start)) & 0xff;
    srb->cmd[10] =  0x00;
    srb->cmd[11] =  0x00;
    srb->cmd[12] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[13] = (unsigned char) blocks & 0xff;
    // srb->cmd[14] =  0x00;
    // srb->cmd[15] =  0x00;
    srb->cmdlen = 16;
    //DEBUG_STORAGE("read10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}
*/


static int usb_write_10(ccb *srb, struct us_data *ss, unsigned long start,unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_WRITE10;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;
    srb->cmd[7] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[8] = (unsigned char) blocks & 0xff;
    srb->cmdlen = 12;
    //DEBUG_STORAGE("write10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}
/*
// was declared but never referenced
static int usb_write_12(ccb *srb, struct us_data *ss, unsigned long start,unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_WRITE12;
    srb->cmd[1] = srb->lun << 5;
    srb->cmd[2] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[3] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[4] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[5] = ((unsigned char) (start)) & 0xff;

    srb->cmd[8] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[9] = (unsigned char) blocks & 0xff;
    srb->cmdlen = 12;
    //DEBUG_STORAGE("write10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}
*/
/*
// was declared but never referenced
static int usb_write_16(ccb *srb, struct us_data *ss, unsigned long start,unsigned short blocks)
{
    memset(&srb->cmd[0], 0, 12);
    srb->cmd[0] = SCSI_WRITE16;
    srb->cmd[1] = 0x00;
    srb->cmd[2] =  0x00;
    srb->cmd[3] =  0x00;
    srb->cmd[4] =  0x00;
    srb->cmd[5] =  0x00;
    srb->cmd[6] = ((unsigned char) (start >> 24)) & 0xff;
    srb->cmd[7] = ((unsigned char) (start >> 16)) & 0xff;
    srb->cmd[8] = ((unsigned char) (start >> 8)) & 0xff;
    srb->cmd[9] = ((unsigned char) (start)) & 0xff;
    srb->cmd[10] =  0x00;
    srb->cmd[11] =  0x00;
    srb->cmd[12] = ((unsigned char) (blocks >> 8)) & 0xff;
    srb->cmd[13] = (unsigned char) blocks & 0xff;
    // srb->cmd[14] =  0x00;
    // srb->cmd[15] =  0x00;
    srb->cmdlen = 16;
    //DEBUG_STORAGE("write10: start %lx blocks %x\n", start, blocks);
    return ss->transport(srb, ss);
}
*/

unsigned long usb_stor_read(int device, unsigned long blknr, unsigned long blkcnt, void *buffer)
{
    unsigned long start, blks, buf_addr;
    unsigned short smallblks;
    struct usb_device *dev;
    struct us_data *ss;
    int retry=0, i,flag=0;
    ccb *srb = &usb_ccb;

    if (blkcnt == 0)
    	{
    	DEBUG_STORAGE("lenth zero\n");
        return 0;
    	}
    device &= 0xff;
    /* Setup  device */
    //DEBUG_STORAGE("\nusb_read: dev %d \n", device);
    dev = NULL;
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        dev = usb_get_dev_index(i);
        if (dev == NULL)
            return 0;
        if (dev->devnum == usb_dev_desc[device].target)
            break;
    }
    ss = (struct us_data *)dev->privptr;

    usb_disable_asynch(1); /* asynch transfer not allowed */
    srb->lun = usb_dev_desc[device].lun;
    buf_addr = (unsigned long)buffer;
    start = blknr;
    blks = blkcnt;
#if 0
    if (usb_test_unit_ready(srb, ss))
    {
        DEBUG_UHOST("Device NOT ready\n   Request Sense returned %02X %02X %02X\n",
                    srb->sense_buf[2], srb->sense_buf[12], srb->sense_buf[13]);
        return 0;
    }
#endif
    //DEBUG_STORAGE("\nusb_read: dev %d startblk %lx, blccnt %lx"
    //" buffer %lx\n", device, start, blks, buf_addr);

    do
    {
        /* XXX need some comment here */
        retry = 2;
        srb->pdata = (unsigned char *)buf_addr;
        if (blks > ss->max_xfer_blk)
            smallblks = ss->max_xfer_blk;
        else
            smallblks = (unsigned short) blks;
    retry_it:
        //if (smallblks == ss->max_xfer_blk)
        //usb_show_progress();
        srb->datalen = usb_dev_desc[device].blksz * smallblks;
        srb->pdata = (unsigned char *)buf_addr;
        if (usb_read_10(srb, ss, start, smallblks))
            // if (usb_read_16(srb, ss, start, smallblks))
        {
            DEBUG_STORAGE("Read ERROR retry: %d\n",retry);
            if(gUSBDevOn == 0)
            {
                flag=-1;
                DEBUG_STORAGE("Read Fail:Card OUT\n");
                break;
            }
            usb_request_sense(srb, ss);
            if (retry--)
                goto retry_it;
            blkcnt -= blks;
            DEBUG_STORAGE("Read Fail\n");
            flag=-1;
            break;
        }
      if(gUSBDevOn == 0)
            {
                flag=-1;
                DEBUG_STORAGE("Read Fail:Card OUT\n");
                break;
            }	
        start += smallblks;
        blks -= smallblks;
        buf_addr += srb->datalen;
    }
    while (blks != 0);

    //DEBUG_STORAGE("usb_read: end startblk %lx, blccnt %x buffer %lx\n",start, smallblks, buf_addr);

    usb_disable_asynch(0); /* asynch transfer allowed */
    if (retry <= 1)
        DEBUG_STORAGE("retry ok?\n");
    if (flag < 0)
    {
        DEBUG_STORAGE("return  Read err\n");
        return (unsigned long)-1;
    }
    return blkcnt;
}

unsigned long usb_stor_write(int device, unsigned long blknr,unsigned long blkcnt, const void *buffer)
{
    unsigned long start, blks, buf_addr;
    unsigned short smallblks;
    struct usb_device *dev;
    struct us_data *ss;
    int retry, i,flag=0;
    ccb *srb = &usb_ccb;

    if (blkcnt == 0)
        return 0;

    device &= 0xff;
    /* Setup  device */
  //	DEBUG_STORAGE("\n##usb_write: dev %d \n", device);
    dev = NULL;
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        dev = usb_get_dev_index(i);
        if (dev == NULL)
            return 0;
        if (dev->devnum == usb_dev_desc[device].target)
            break;
    }
    ss = (struct us_data *)dev->privptr;

    usb_disable_asynch(1); /* asynch transfer not allowed */

    srb->lun = usb_dev_desc[device].lun;
    buf_addr = (unsigned long)buffer;
    start = blknr;
    blks = blkcnt;
#if 1
    if (usb_test_unit_ready(srb, ss))
    {
        DEBUG_UHOST("Device NOT ready\n   Request Sense returned %02X %02X"
                    " %02X\n", srb->sense_buf[2], srb->sense_buf[12],
                    srb->sense_buf[13]);
        return 0;
    }
#endif
    //	DEBUG_STORAGE("\nusb_write: dev %d startblk %lx, blccnt %lx"
    //			" buffer %lx\n", device, start, blks, buf_addr);

    do
    {
        /* If write fails retry for max retry count else
         * return with number of blocks written successfully.
         */
        retry = 2;
        srb->pdata = (unsigned char *)buf_addr;
        if (blks > ss->max_xfer_blk)
            smallblks = ss->max_xfer_blk;
        else
            smallblks = (unsigned short) blks;
    retry_it:
        //if (smallblks == ss->max_xfer_blk)
        //usb_show_progress();
        srb->datalen = usb_dev_desc[device].blksz * smallblks;
        srb->pdata = (unsigned char *)buf_addr;
        if (usb_write_10(srb, ss, start, smallblks))
            //if (usb_write_12(srb, ss, start, smallblks))
        {
            DEBUG_STORAGE("Write ERROR retry: %d\n",retry);
            // DEBUG_STORAGE("Write ERROR\n");
            if(gUSBDevOn == 0)
            {
                flag=-1;
                DEBUG_STORAGE("Write Fail:Card OUT\n");
                break;
            }
            usb_request_sense(srb, ss);
            if (retry--)
                goto retry_it;
            blkcnt -= blks;
            flag=-1;
            break;
        }
       if(gUSBDevOn == 0)
            {
                flag=-1;
                DEBUG_STORAGE("Write Fail:Card OUT\n");
                break;
            }	
        start += smallblks;
        blks -= smallblks;
        buf_addr += srb->datalen;
    }
    while (blks != 0);

    //DEBUG_STORAGE("usb_write: end startblk %lx, blccnt %x buffer %lx\n",
    //		start, smallblks, buf_addr);

    usb_disable_asynch(0); /* asynch transfer allowed */
    //if (blkcnt >= ss->max_xfer_blk)
    //DEBUG_UHOST("blkcnt >= ss->max_xfer_blk\n");
    if (retry <= 1)
        DEBUG_STORAGE("retry ok?\n");
    if (flag < 0)
    {
        DEBUG_STORAGE("return Write err\n");
        return (unsigned long)-1;
    }
    return blkcnt;

}

int usbfsDevStatus(u32 Unit)
{
    int DevIdx;

    DevIdx=DCF_GetDeviceIndex("usbfs");

    if (Unit != 0)
    {
        DEBUG_STORAGE("USB Err: Unit=%d\n",Unit);
        return -1; /* Invalid unit number */
    }

    if (!sysStorageOnlineStat[DevIdx])
    {
        /*
           Make sure, the function returns FS_LBL_MEDIACHANGED when it is
           called the first time
        */
        sysStorageOnlineStat[DevIdx] = 1;
        return FS_LBL_MEDIACHANGED;
    }

    return 0;
}

int test_cnt=0;
int usbfsDevRead(u32 Unit, u32 Sector, void *pBuffer)
{
    int retValue = 0;
    u8 ucErr;
  //  printf("{R");
  	
    OSSemPend(SCSI_SemEvt, 100 , &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("usbfsDevRead: SCSI_SemEvt is time-out");
      //  OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */
    }
    if (Unit != 0)
    {
        DEBUG_STORAGE("UHOST Err: Unit=%d\n",Unit);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */
    }
    if (Sector >= usbfsTotalBlockCount)
    {
        DEBUG_STORAGE("UHOST Err: Sector=%d,usbfsTotalBlockCount=%d\n",Sector,usbfsTotalBlockCount);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Out of physical range */
    }
    if (gUSBDevOn == 0)
    {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
   if((usb_init_flag)||(usb_hdd_removed))
   {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: usb device  initial or removed \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#endif
    //DEBUG_UHOST("usbfsDevRead: %x  %x\n",Sector,pBuffer);
    retValue=gusb_dev_desc->block_read(gusb_dev_desc->dev, Sector, 1, pBuffer);
//   printf("}");
    OSSemPost(SCSI_SemEvt);
    if(retValue<0)
    {
        DEBUG_STORAGE("UHOST Err:Read Error %d \n",retValue);
	 DEBUG_STORAGE("usbfsDevRead: %x  %x\n",Sector,pBuffer);
        return -1;
    }
	keepCount=0;
    return 0;
}

int usbfsDevMulRead(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    int retValue = 0;
    u8 ucErr;
 //  printf("{MR");
    OSSemPend(SCSI_SemEvt, 100 , &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("usbfsDevMulRead: SCSI_SemEvt is time-out");
           // OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */ ;
    }
    //DEBUG_UHOST("usbfsDevMulRead: %x  %x\n",NumofSector,pBuffer);
    if (Unit != 0)
    {
        DEBUG_STORAGE("UHOST Err: Unit=%d\n",Unit);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */
    }
    if (Sector >= usbfsTotalBlockCount)
    {
        DEBUG_STORAGE("UHOST Err: Sector=%d,usbfsTotalBlockCount=%d\n",Sector,usbfsTotalBlockCount);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Out of physical range */
    }
    if (gUSBDevOn == 0)
    {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)|| (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
   if((usb_init_flag)||(usb_hdd_removed))
   {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: usb device  initial or removed \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#endif
    retValue=gusb_dev_desc->block_read(gusb_dev_desc->dev, Sector, NumofSector, pBuffer);
//    printf("}");
    OSSemPost(SCSI_SemEvt);
    if(retValue<0)
    {
        DEBUG_STORAGE("UHOST Err: MURead fail \n");
	 DEBUG_STORAGE("usbfsDevMulRead: %x  %x\n",Sector,pBuffer);
        return -1;
    }
	keepCount=0;
    return 0;
}

int usbfsDevWrite(u32 Unit, u32 Sector, void *pBuffer)
{
    int retValue = 0;
    u8 ucErr;
  //  printf("[W");
    OSSemPend(SCSI_SemEvt, 100 , &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("usbfsDevWrite: SCSI_SemEvt is time-out");
            //OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */ ;
    }
    if (Unit != 0)
    {
        DEBUG_STORAGE("UHOST Err: Unit=%d\n",Unit);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */
    }
    if (Sector >= usbfsTotalBlockCount)
    {
        DEBUG_STORAGE("UHOST Err: Sector=%d,usbfsTotalBlockCount=%d\n",Sector,usbfsTotalBlockCount);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Out of physical range */
    }
    if (gUSBDevOn==0)
    {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)|| (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
   if((usb_init_flag)||(usb_hdd_removed))
   {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: usb device  initial or removed \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#endif
    //DEBUG_UHOST("usbfsDevWrite: %x  %x\n",Sector,pBuffer);
    retValue=gusb_dev_desc->block_write(gusb_dev_desc->dev, Sector, 1, pBuffer);
  //  printf("]");
    OSSemPost(SCSI_SemEvt);
    if(retValue<0)
    {
        DEBUG_STORAGE("UHOST Err: wite fail \n");
	 DEBUG_STORAGE("usbfsDevWrite: %x  %x\n",Sector,pBuffer);
        return -1;
    }
    keepCount=0;
    return 0;
}


int usbfsDevMulWrite(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    int retValue = 0;
    u8 ucErr;
//    printf("[MW");
    OSSemPend(SCSI_SemEvt, 100 , &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("usbfsDevMulWrite: SCSI_SemEvt is time-out");
           // OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */ ;
    }
    if (Unit != 0)
    {
        DEBUG_STORAGE("UHOST Err: Unit=%d\n",Unit);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Invalid unit number */
    }
    if (Sector >= usbfsTotalBlockCount)
    {
        DEBUG_STORAGE("UHOST Err: Sector=%d,usbfsTotalBlockCount=%d\n",Sector,usbfsTotalBlockCount);
        OSSemPost(SCSI_SemEvt);
        return -1; /* Out of physical range */
    }
    if(gUSBDevOn==0)
    {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)||(SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
   if((usb_init_flag)||(usb_hdd_removed))
   {
        gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: usb device  initial or removed \n");
        OSSemPost(SCSI_SemEvt);
        return -1;
    }
#endif
    //	DEBUG_UHOST("usbfsDevMulWrite: %x  %x\n",NumofSector,pBuffer);
    retValue=gusb_dev_desc->block_write(gusb_dev_desc->dev, Sector, NumofSector, pBuffer);

//    printf("]");
    OSSemPost(SCSI_SemEvt);
    if(retValue<0)
    {
        DEBUG_STORAGE("UHOST Err: MUwite fail \n");
	 DEBUG_STORAGE("usbfsDevMUWrite: %x  %x\n",Sector,pBuffer);
        return -1;
    }
    keepCount=0;
    return 0;
}

int usbfsDevIoCtl(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
    u32 *info;
    int DevIdx;

    DevIdx=DCF_GetDeviceIndex("usbfs");
    Aux = Aux;  /* Get rid of compiler warning */
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
            *info = 0;                  /* hidden */
            info++;
            *info = 2;                  /* head */
            info++;
            *info = 4;                  /* sec per track */
            info++;
            *info = usbfsTotalBlockCount; /* total block count */
            break;

        case FS_CMD_FORMAT_MEDIA:
            /* Format the SD card */

            break;
        case FS_CMD_SET_STATUS:
            sysStorageOnlineStat[DevIdx] = Aux;
            break;

        default:
            break;
    }

    return 0;
}

/* Probe to see if a new device is actually a Storage device */
int usb_storage_probe(struct usb_device *dev, unsigned int ifnum,struct us_data *ss)
{
    struct usb_interface *iface;
    int i;
    unsigned int flags = 0;

    int protocol = 0;
    int subclass = 0;

    ifnum=0;

    if(dev->config.no_of_if>0)
    {
         for(i=0;i<=dev->config.no_of_if;i++)
           {
            iface = &dev->config.if_desc[ifnum];
	     if (iface->desc.bInterfaceProtocol !=US_PR_BULK)
            {
                ifnum++;
            }
            }
        DEBUG_STORAGE("US_PR_BULK ifnum In %d \n",ifnum);
   }
  //     DEBUG_STORAGE("ray_ifnum In %d \n",ifnum);
    /* let's examine the device now */
    iface = &dev->config.if_desc[ifnum];

#if 0
    /* this is the place to patch some storage devices */
    DEBUG_STORAGE("iVendor %X iProduct %X\n", dev->descriptor.idVendor,
                  dev->descriptor.idProduct);

    if ((dev->descriptor.idVendor) == 0x066b &&
            (dev->descriptor.idProduct) == 0x0103)
    {
        DEBUG_STORAGE("patched for E-USB\n");
        protocol = US_PR_CB;
        subclass = US_SC_UFI;	    /* an assumption */
    }
#endif

    if (dev->descriptor.bDeviceClass != 0 ||
            iface->desc.bInterfaceClass != USB_CLASS_MASS_STORAGE ||
            iface->desc.bInterfaceSubClass < US_SC_MIN ||
            iface->desc.bInterfaceSubClass > US_SC_MAX)
    {
        /* if it's not a mass storage, we go no further */
		 DEBUG_STORAGE("\n\n if it's not a mass storage\n");
        return 0;
    }

    memset(ss, 0, sizeof(struct us_data));

    /* At this point, we know we've got a live one */
    DEBUG_STORAGE("\n\nUSB Mass Storage device detected\n");

    /* Initialize the us_data structure with some useful info */
    ss->flags = flags;
    ss->ifnum = ifnum;
    ss->pusb_dev = dev;
    ss->attention_done = 0;

    /* If the device has subclass and protocol, then use that.  Otherwise,
     * take data from the specific interface.
     */
    if (subclass)
    {
        ss->subclass = subclass;
        ss->protocol = protocol;
    }
    else
    {
        ss->subclass = iface->desc.bInterfaceSubClass;
        ss->protocol = iface->desc.bInterfaceProtocol;
    }

    /* set the handler pointers based on the protocol */
    DEBUG_STORAGE("Transport: %X ++\n",ss->protocol);
    switch (ss->protocol)
    {
        case US_PR_CB:
            DEBUG_STORAGE("Control/Bulk\n");
            //ss->transport = usb_stor_CB_transport;
            //ss->transport_reset = usb_stor_CB_reset;
            DEBUG_STORAGE("Not Support Control/Bulk Transfer!!!\n");
            break;

        case US_PR_CBI:
            DEBUG_STORAGE("Control/Bulk/Interrupt\n");
            //ss->transport = usb_stor_CB_transport;
            //ss->transport_reset = usb_stor_CB_reset;
            DEBUG_STORAGE("Not Support Control/Bulk Transfer/Interrupt!!!\n");
            break;
        case US_PR_BULK:
            DEBUG_STORAGE("Bulk/Bulk/Bulk\n");
            ss->transport = usb_stor_BBB_transport;
            ss->transport_reset = usb_stor_BBB_reset;
            break;
        case US_PR_UAS:
            DEBUG_STORAGE("USB Attached SCSI Protocol\n");
            //ss->transport = usb_stor_UASP_transport;
            //ss->transport_reset = usb_stor_UASP_reset;
            DEBUG_STORAGE("Not Support USB Attached SCSI Protocol!!!\n");
            return 0;
        default:
            DEBUG_STORAGE("USB Storage Transport unknown / not yet implemented\n");
            return 0;
    }

    /*
     * We are expecting a minimum of 2 endpoints - in and out (bulk).
     * An optional interrupt is OK (necessary for CBI protocol).
     * We will ignore any others.
     */
    for (i = 0; i < iface->desc.bNumEndpoints; i++)
    {
        /* is it an BULK endpoint? */
        if ((iface->ep_desc[i].bmAttributes &
                USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK)
        {
            if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
                ss->ep_in = iface->ep_desc[i].bEndpointAddress &
                            USB_ENDPOINT_NUMBER_MASK;
            else
                ss->ep_out =
                    iface->ep_desc[i].bEndpointAddress &
                    USB_ENDPOINT_NUMBER_MASK;
        }

        /* is it an interrupt endpoint? */
        if ((iface->ep_desc[i].bmAttributes &
                USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)
        {
            ss->ep_int = iface->ep_desc[i].bEndpointAddress &
                         USB_ENDPOINT_NUMBER_MASK;
            ss->irqinterval = iface->ep_desc[i].bInterval;
        }
    }
    DEBUG_STORAGE("Endpoints In %d Out %d Int %d\n",
                  ss->ep_in, ss->ep_out, ss->ep_int);

    /* Do some basic sanity checks, and bail if we find a problem */
  if (usb_set_interface(dev, iface->desc.bInterfaceNumber,  iface->desc.bAlternateSetting) ||
            !ss->ep_in || !ss->ep_out ||
            (ss->protocol == US_PR_CBI && ss->ep_int == 0))
    {
        DEBUG_STORAGE("error:Problems with device\n");
        return 0;
    }
    /* set class specific stuff */
    /* We only handle certain protocols.  Currently, these are
     * the only ones.
     * The SFF8070 accepts the requests used in u-boot
     */
    if (ss->subclass != US_SC_UFI && ss->subclass != US_SC_SCSI &&
            ss->subclass != US_SC_8070)
    {
        DEBUG_STORAGE("error:Sorry, protocol %d not yet supported.\n", ss->subclass);
        return 0;
    }
    if (ss->ep_int)
    {
        /* we had found an interrupt endpoint, prepare irq pipe
         * set up the IRQ pipe and handler
         */
        ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
        ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
        ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
        dev->irq_handle = usb_stor_irq;
    }
    dev->privptr = (void *)ss;
    return 1;
}

int usb_stor_get_info(struct usb_device *dev, struct us_data *ss, block_dev_desc_t *dev_desc)
{
    unsigned char perq, modi;
    __align(4) unsigned long cap[2];
    __align(4) unsigned char usb_stor_buf[36];

    unsigned long *capacity, *blksz;
    ccb *pccb = &usb_ccb;

    pccb->pdata = usb_stor_buf;

    dev_desc->target = dev->devnum;
    pccb->lun = dev_desc->lun;
    DEBUG_STORAGE(" address %d\n", dev_desc->target);

    if (usb_inquiry(pccb, ss))
        return -1;

    perq = usb_stor_buf[0];
    modi = usb_stor_buf[1];

    if ((perq & 0x1f) == 0x1f)
    {
        /* skip unknown devices */
        return 0;
    }
    if ((modi&0x80) == 0x80)
    {
        /* drive is removable */
        dev_desc->removable = 1;
    }
    memcpy(&dev_desc->vendor[0], (const void *) &usb_stor_buf[8], 8);
    memcpy(&dev_desc->product[0], (const void *) &usb_stor_buf[16], 16);
    memcpy(&dev_desc->revision[0], (const void *) &usb_stor_buf[32], 4);
    dev_desc->vendor[8] = 0;
    dev_desc->product[16] = 0;
    dev_desc->revision[4] = 0;
#ifdef CONFIG_USB_BIN_FIXUP
    usb_bin_fixup(dev->descriptor, (uchar *)dev_desc->vendor,
                  (uchar *)dev_desc->product);
#endif /* CONFIG_USB_BIN_FIXUP */
    DEBUG_STORAGE("ISO Vers %X, Response Data %X\n", usb_stor_buf[2],
                  usb_stor_buf[3]);
    if (usb_test_unit_ready(pccb, ss))
    {
        DEBUG_UHOST("Dev NOT ready\n"
                    "   Request Sense  %02X %02X %02X\n",
                    pccb->sense_buf[2], pccb->sense_buf[12],
                    pccb->sense_buf[13]);
        if (dev_desc->removable == 1)
        {
            dev_desc->type = perq;
            return 1;
        }
        DEBUG_UHOST("Device ready\n");
        return 0;
    }
    pccb->pdata = (unsigned char *)&cap[0];
    memset(pccb->pdata, 0, 8);
    if (usb_read_capacity(pccb, ss) != 0)
    {
        if(cap[0]== 0xffffffff)
        {
            DEBUG_STORAGE(">2T_E\n");
            cap[0] = 0xaf88e0e8; // 2T
            //cap[0] = 0xaf6d7074; // 1T
            cap[1] = 0x00020000;
        }
        else
        {
            DEBUG_STORAGE("READ_CAP ERROR\n");
            cap[0] = 2880;
            cap[1] = 0x200;
	     return 0;
        }
    }
    if(cap[0]== 0xffffffff)
    {
        DEBUG_STORAGE(">2T\n");
        cap[0] = 0xaf88e0e8; // 2T
        //cap[0] = 0xaf6d7074; // 1T
        cap[1] = 0x00020000;
    }
    DEBUG_STORAGE("Read_Capacity returns: 0x%lx, 0x%lx\n", cap[0],cap[1]);
#if 0
    if (cap[0] > (0x200000 * 10)) /* greater than 10 GByte */
        cap[0] >>= 16;
#endif
    cap[0] = cpu_to_be32(cap[0]);
    cap[1] = cpu_to_be32(cap[1]);

    /* this assumes bigendian! */
    cap[0] += 1;
    capacity = &cap[0];
    blksz = &cap[1];
    DEBUG_STORAGE("Capacity = 0x%lx, blocksz = 0x%lx\n",*capacity, *blksz);
    dev_desc->lba = *capacity;
    dev_desc->blksz = *blksz;
    dev_desc->type = perq;
    DEBUG_STORAGE(" address %d\n", dev_desc->target);
    DEBUG_STORAGE("partype: %d\n", dev_desc->part_type);

    /*
     * The U-Boot EHCI driver cannot handle more than 4096 * 5 bytes in a
     * transfer without running itself out of qt_buffers.
     */
    //ss->max_xfer_blk = (4096 * 4) / dev_desc->blksz;
    ss->max_xfer_blk = USB_MAX_XFER_BLK;
    //init_part(dev_desc);
    DEBUG_STORAGE("max_xfer_blk: %d\n", ss->max_xfer_blk);
    DEBUG_STORAGE("partype: %d\n", dev_desc->part_type);
    return 1;
}

int usb_stor_remove(ccb *srb, struct us_data *ss)
{
    unsigned char perq, modi;
    __align(4) unsigned long cap[2];
    __align(4) unsigned char usb_stor_buf[36];

    unsigned long *capacity, *blksz;
    ccb *pccb = &usb_ccb;



    pccb->pdata = usb_stor_buf;

    if (usb_test_unit_ready(pccb, ss))
    {
        DEBUG_STORAGE("Device NOT ready\n"
                    "   Request Sense returned %02X %02X %02X\n",
                    pccb->sense_buf[2], pccb->sense_buf[12],
                    pccb->sense_buf[13]);

        return 0;
    }
    pccb->pdata = (unsigned char *)&cap[0];
    memset(pccb->pdata, 0, 8);
//    if (usb_remove(pccb, ss) != 0)
//    {
//          DEBUG_STORAGE("usb_stor_remove fail\n");
//	return 0;
//    }
	
    if (usb_start_stop(pccb, ss) != 0)
    {
         DEBUG_STORAGE("usb_stor_stop fail\n");
		return 0; 
    }
	
if (usb_test_unit_ready(pccb, ss))
    {
        DEBUG_STORAGE("Device NOT ready\n"
                    "   Request Sense returned %02X %02X %02X\n",
                    pccb->sense_buf[2], pccb->sense_buf[12],
                    pccb->sense_buf[13]);

        return 0;
    }
    DEBUG_STORAGE("usb_stor_remove done\n");
    return 1;
}


int usb_stop_remove()
{
#if SD_TASK_INSTALL_FLOW_SUPPORT
	u8 ucErr;
	int retValue = 1;
	
	DEBUG_STORAGE("usb_stor_remove start\n");

	OSSemPend(SCSI_SemEvt, 100 , &ucErr);
	if (ucErr != OS_NO_ERR)
	{
		DEBUG_STORAGE("usb_stop_remove: SCSI_SemEvt is time-out");
		//  OSSemPost(SCSI_SemEvt);
		return 0;	// Invalid unit number
	}

	// Catch the semphore successfully.
	if((HCPortSC & PORTSC_CS)==0)
	{
		gUSBDevOn = 0;
		retValue = 0;
		DEBUG_STORAGE("UHOST Err: no usb device \n");
    }  

    if(retValue)
    {
    	// Execute stop action when device is exist.
    	retValue = usb_stor_remove(&usb_ccb, &usb_stor[usb_hdd_dev_num]);
    }
    
    if(retValue)
        usb_hdd_removed = 1;
    else
        DEBUG_STORAGE("UHOST Err:stop_remove Error %d \n", retValue);
    OSSemPost(SCSI_SemEvt);

    usbHostSetIntEvt(USB_HOST_DEV_EVT_UNMOUNT);
#else
	//  ccb *pccb = &usb_ccb;
	u8 ucErr;
	int retValue = 0;
	u32 StorageIndex;
	DEBUG_STORAGE("usb_stor_remove start\n");

	StorageIndex = sysGetStorageIndex(SYS_V_STORAGE_USBMASS);

	// Follow UI FLOW to raise USB file system init
	sysSetStorageStatus(StorageIndex, SYS_V_STORAGE_NREADY);

	OSSemPend(SCSI_SemEvt, 100 , &ucErr);
	if (ucErr != OS_NO_ERR)
	{
		DEBUG_STORAGE("usb_stop_remove: SCSI_SemEvt is time-out");
		//  OSSemPost(SCSI_SemEvt);
		return 0; /* Invalid unit number */
	}

	if((HCPortSC & PORTSC_CS)==0)
	{
		gUSBDevOn = 0;
        DEBUG_STORAGE("UHOST Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return 0;
    }

    retValue= usb_stor_remove(  &usb_ccb,&usb_stor[usb_hdd_dev_num]);

    OSSemPost(SCSI_SemEvt);

    if(retValue==0)
    {
        DEBUG_STORAGE("UHOST Err:stop_remove Error %d \n",retValue);
        sysSetStorageStatus(StorageIndex, SYS_V_STORAGE_READY);
        return 0;
    }
    usb_hdd_removed=1;

#if USB_HOST_MASS_SUPPORT
	sysUSBCD_OFF(1);
#endif
#endif
   return 1;
}
/*
int usb_seek(ccb *srb, struct us_data *ss)
{
    unsigned char perq, modi;
    __align(4) unsigned long cap[2];
    __align(4) unsigned char usb_stor_buf[36];

    unsigned long *capacity, *blksz,start;
    ccb *pccb = &usb_ccb;

    pccb->pdata = usb_stor_buf;
#if 0
    if (usb_request_sense(srb, ss))
    {
        DEBUG_STORAGE("Device NOT ready\n"
                    "   Request Sense returned %02X %02X %02X\n",
                    pccb->sense_buf[2], pccb->sense_buf[12],
                    pccb->sense_buf[13]);

        return 0;
    }
#endif
    start=0x00020000;
    pccb->pdata = (unsigned char *)&cap[0];
    memset(pccb->pdata, 0, 8);
    if (usb_seek_10(pccb, ss, start) != 0)
   {
         DEBUG_STORAGE("usb_seek_10 fail\n");
		return 0; 
   }
	
if (usb_test_unit_ready(pccb, ss))
    {
        DEBUG_STORAGE("Device NOT ready\n"
                    "   Request Sense returned %02X %02X %02X\n",
                    pccb->sense_buf[2], pccb->sense_buf[12],
                    pccb->sense_buf[13]);

        return 0;
    }
    DEBUG_STORAGE("usb_start_up done\n");
    return 1;
}
*/
int usb_dev_in()
{
   if (HCPortSC & PORTSC_CS) //usb Dev in
    {
    	return 1;
    }
   else
   {
   	return 0;
   }
}
int usb_keep_on()
{
#if 0
 //   ccb *pccb = &usb_ccb;
    u8 ucErr;
   int retValue = 0;
   u32 StorageIndex;
   DEBUG_STORAGE("usb_seek\n");
   
   if((usb_init_flag)||(usb_hdd_removed))
   {
        gUSBDevOn = 0;
        DEBUG_STORAGE("usb_seek Err: usb device  initial or removed \n");
        return -1;
    }

   OSSemPend(SCSI_SemEvt, 100 , &ucErr);
   if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("usb_seek: SCSI_SemEvt is time-out");
      //  OSSemPost(SCSI_SemEvt);
        return 0; /* Invalid unit number */
    }
   
  if ((HCPortSC & PORTSC_CS)==0)
    {
        gUSBDevOn = 0;
        DEBUG_STORAGE("usb_seek Err: no usb device \n");
        OSSemPost(SCSI_SemEvt);
        return 0;
    }  
  
  retValue= usb_seek_10( &usb_ccb,&usb_stor[usb_hdd_dev_num]);

   
   OSSemPost(SCSI_SemEvt);

       if(retValue==0)
    {
        DEBUG_STORAGE("usb_seek Err:usb_seek Error %d \n",retValue);
        return 0;
    }
	
   return 1;
#else
  u32 Sector;
  u8 dummy[512];
  
  keepCount++;
  
if((keepCount%8)==0)
{
// 	DEBUG_STORAGE("K");
	 Sector=0x80000;
 	usbfsDevRead(0,  Sector, dummy);
}
#endif
}

#endif
