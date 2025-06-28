/*

Copyright (c) 2012 Mars Semiconductor Corp.

Module Name:

	farady_host_register.h

Abstract:

   	The declarations of FARADY USB host register.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2012/07/26	David Tsai	Create

*/

#ifndef __USB_FARADY_HOST_H__
#define __USB_FARADY_HOST_H__

#include "usb_main.h"

#define BIT0    0x00000001
#define BIT1    0x00000002
#define BIT2    0x00000004
#define BIT3    0x00000008
#define BIT4    0x00000010
#define BIT5    0x00000020
#define BIT6    0x00000040
#define BIT7    0x00000080
#define BIT8    0x00000100
#define BIT9    0x00000200
#define BIT10   0x00000400
#define BIT11   0x00000800
#define BIT12   0x00001000
#define BIT13   0x00002000
#define BIT14   0x00004000
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000


extern int rootdev;
//extern struct ehci_hccr *hccr;
//extern volatile struct ehci_hcor *hcor;

extern u8* qh_list_addr;
extern u8* qh_addr;
extern u8* qtd_addr[5];
extern u8* hub_qh_addr;
extern u8* mouse_qh_addr;
extern u8* keyboard_qh_addr;
extern u8* hub_qtd;
extern u8* mouse_qtd;
extern u8* keyboard_qtd;
extern u32* periodic_base;
extern u8* test_buf;
extern u32 hub_running;
extern u32 mouse_running;
extern u32 keyboard_running;
extern u32 hub_mode;
extern u32 mouse_packet_size;
extern u32 ghub_addr;
extern u32 ghub_port;

extern char hub_buffer[8];
extern char hid_buf_mouse[128];
extern char hid_buf_keyboard[128];

extern int usb_lowlevel_init(void);
extern int usb_lowlevel_stop(void);
extern int hub_port_reset(struct usb_device *dev, int port, unsigned short *portstat);
extern int usb_hub_probe(struct usb_device *dev, int ifnum);
extern u32 hid_qh_init(struct usb_device *dev, struct usb_endpoint_descriptor *ep, u8 *qh_addr, u8 *qtd_addr, u32 smask);
extern int hub_event_handle(char port);
extern int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length);
extern int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length, int interval);
extern int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length, struct devrequest *setup);
extern void format_printf2(unsigned long offset,unsigned long length,unsigned int buf);
extern void usb_hub_reset(void);
extern int usb_set_interface(struct usb_device *dev, int interface, int alternate);
extern int usb_bulk_msg(struct usb_device *dev, unsigned int pipe, void *data, int len, int *actual_length, int timeout);
extern int usb_maxpacket(struct usb_device *dev, unsigned long pipe);
extern int usb_disable_asynch(int disable);
extern u32 hub_qh_init(u32 max_packet, u32 epnum, u32 dev_addr, u32 smask);
extern int usb_new_device(struct usb_device *dev);
extern int usb_control_msg(struct usb_device *dev, unsigned int pipe, unsigned char request, unsigned char requesttype,
                           unsigned short value, unsigned short index, void *data, unsigned short size, int timeout);

extern int usbfsDevStatus(u32);
extern int usbfsDevRead(u32, u32, void*);
extern int usbfsDevMulRead(u32, u32, u32, void*);
extern int usbfsDevWrite(u32, u32, void*);
extern int usbfsDevMulWrite(u32, u32, u32, void *);
extern int usbfsDevIoCtl(u32, s32, s32, void*);
extern int usb_clear_halt(struct usb_device * , int );
extern void UsbHubRst(void);

#define uswap_16(x) ((((x) & 0xff00) >> 8) | \
                     (((x) & 0x00ff) << 8))
#define uswap_32(x) ((((x) & 0xff000000) >> 24) | \
                     (((x) & 0x00ff0000) >>  8) | \
                     (((x) & 0x0000ff00) <<  8) | \
                     (((x) & 0x000000ff) << 24))

#define cpu_to_le16(x)	(x)
#define cpu_to_le32(x)	(x)
#define cpu_to_le64(x)	(x)
#define le16_to_cpu(x)	(x)
#define le32_to_cpu(x)	(x)
#define le64_to_cpu(x)	(x)
#define cpu_to_be16(x)	uswap_16(x)
#define cpu_to_be32(x)	uswap_32(x)
#define be16_to_cpu(x)	uswap_16(x)
#define be32_to_cpu(x)	uswap_32(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ROUND(a,b)			(((a) + (b) - 1) & ~((b) - 1))
#define DIV_ROUND(n,d)		(((n) + ((d)/2)) / (d))
#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))
#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))
#define max(a,b) (((a)>(b))? (a):(b))


extern void ehci_udelay(unsigned long usec);
extern void ehci_mdelay(unsigned long msec);
extern int poll_hid(void);

#define EHCI_TIMEOUT		20
/*end USB Host definition*/

#define USB_MAX_INT_EVT		32			/* max. interrupt event queued */
typedef struct _USB_INT_EVT
{
    u8		cause[USB_MAX_INT_EVT];   	/* cause of interrupt event */
    u8	  	idxSet;                         /* index of set event */
    u8	  	idxGet;                         /* index of get event */
}
USB_INT_EVT;

#define USB_HOST_MAX_INT_EVT		32

/* Interrupt event cause index */
enum
{
    USB_HOST_INT_EVT_SCAN = 0x00,	// 0x00 - attach event
    USB_HOST_INT_EVT_HUB_CONNECT_CHANGE,
    USB_HOST_DEV_EVT_MOUNT,
    USB_HOST_DEV_EVT_UNMOUNT,
    USB_HOST_DEV_EVT_MASS_FREE_SCAN,
    USB_HOST_DEV_EVT_UNDEF
};

#endif

