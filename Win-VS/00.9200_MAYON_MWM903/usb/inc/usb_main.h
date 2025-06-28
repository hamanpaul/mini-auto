
#ifndef __USB_MAIN_H__
#define __USB_MAIN_H__

#include <usb_defs.h>
#include <usbdescriptors.h>

/********************************************************************************
 *
 * Functional defines
 *
 */

 /*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (2 bits)
 *  - max packet size (2 bits: 8, 16, 32 or 64)
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * unsigned int. The encoding is:
 *
 *  - max size:		bits 0-1	(00 = 8, 01 = 16, 10 = 32, 11 = 64)
 *  - direction:	bit 7		(0 = Host-to-Device [Out], 1 = Device-to-Host [In])
 *  - device:		bits 8-14
 *  - endpoint:		bits 15-18
 *  - Data0/1:		bit 19
 *  - speed:		bit 26		(0 = Full, 1 = Low Speed, 2 = High)
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt, 10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 */
/* Create various pipes... */
#define create_pipe(dev,endpoint)   ((dev->devnum << 8) | (endpoint << 15) | (dev->speed << 26) | dev->maxpacketsize)
#define default_pipe(dev)           ((dev)->speed << 26)

#define usb_sndctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_snddefctrl(dev)				((PIPE_CONTROL << 30) | default_pipe(dev))
#define usb_rcvdefctrl(dev)				((PIPE_CONTROL << 30) | default_pipe(dev) | USB_DIR_IN)

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out)         (((dev)->toggle[out] >> ep) & 1)
#define usb_dotoggle(dev, ep, out)          ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit)    ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir)	        (((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out)     ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out)  ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out)   ((dev)->halted[out] & (1 << (ep)))

#define usb_packetid(pipe)	    (((pipe) & USB_DIR_IN) ? USB_PID_IN : USB_PID_OUT)

#define usb_pipeout(pipe)	    ((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)	    (((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)	(((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe)	    (((pipe) >> 19) & 1)
#define usb_pipespeed(pipe)	    (((pipe) >> 26) & 3)
#define usb_pipeslow(pipe)	    (usb_pipespeed(pipe) == USB_SPEED_LOW)
#define usb_pipetype(pipe)	    (((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	    (usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	    (usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	    (usb_pipetype((pipe)) == PIPE_BULK)

/*
 * This is the timeout to allow for submitting an urb in ms. We allow more
 * time for a BULK device to react - some are slow.
 */
#define USB_TIMEOUT_MS(pipe) (usb_pipebulk(pipe) ? 5000 : 1000)

#define hc32_to_cpu(x)		(x)
#define cpu_to_hc32(x)		(x)

/*
 *
 * Ehci Host controller's functional defines
 *
 */

#define	ehci_readl(x)		(*((volatile u32 *)(x)))
#define ehci_writel(a, b)	(*((volatile u32 *)(a)) = ((volatile u32)b))

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest 		((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)
#define DeviceOutRequest 	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)
#define InterfaceRequest 	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)
#define EndpointRequest 	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)
#define EndpointOutRequest 	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

/*
 *
 * Register Space.
 *
 */
#define HC_LENGTH(p)		(((p) >> 0) & 0x00ff)
#define HC_VERSION(p)		(((p) >> 16) & 0xffff)
#define HCS_PPC(p)		    ((p) & (1 << 4))
#define HCS_INDICATOR(p)	((p) & (1 << 16)) /* Port indicators */
#define HCS_N_PORTS(p)		(((p) >> 0) & 0xf)

__packed struct ehci_hccr
{
	u32 cr_capbase;
	u32 cr_hcsparams;
	u32 cr_hccparams;
} ;

__packed struct ehci_hcor
{
	u32 or_usbcmd;
	u32 or_usbsts;
	u32 or_usbintr;
	u32 or_frindex;			    /*0x1C*/
	u32 or_reserved1;			/*0x20*/
	u32 or_periodiclistbase;	/*0x24*/
	u32 or_asynclistaddr;		/*0x28*/
	u32 or_reserved2;			/*0x2C*/
	u32 or_portsc[CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS];	/*0x30*/
	u32 or_reserved3[3];		/*0x34 -0x3C*/
	u32 or_misc;				/*0x40*/
} ;

/*******************************************************************************/

/********************************************************************************
 *
 * Data structure area
 *
 */

/* device request (setup) */
__packed struct devrequest
{
    u8	requesttype;
    u8	request;
    u16	value;
    u16	index;
    u16	length;
};

/* Interface data structure */
__packed struct usb_interface
{
    struct usb_interface_descriptor desc;
    u8 no_of_hid;   /* identify hid_desc is exist or not */
    struct usb_class_hid_descriptor hid_desc;
    struct usb_endpoint_descriptor ep_desc[USB_MAX_ENDPOINTS];
} ;

/* Configuration data structure */
__packed struct usb_config
{
    struct usb_configuration_descriptor desc;
    u8 no_of_if;	/* number of interfaces */
    struct usb_interface if_desc[USB_MAX_INTERFACES];
} ;

/* Maximum packet size; encoded as 0,1,2,3 = 8,16,32,64 */
enum
{
    PACKET_SIZE_8   = 0,
    PACKET_SIZE_16  = 1,
    PACKET_SIZE_32  = 2,
    PACKET_SIZE_64  = 3,
};

struct usb_device
{
    s8 enabled;        /* the device status: new & removed-> 0, using-> 1, */
    s8 devnum;         /* Device number on USB bus; udev_index + 1 = devnum */

    int speed;          /* full/low/high */
    int configno;       /* selected config number */
    int hid_report_len; /* the report descriptor length of selected boot interface */
    int have_langid;    /* whether string_langid is valid yet */
    int string_langid;  /* language ID for strings */

    char mf[32];        /* manufacturer */
    char prod[32];      /* product */
    char serial[32];    /* serial number */

    /* endpoint halts; one bit per endpoint # & direction ([0] = IN, [1] = OUT) */
    unsigned int halted[2];
    unsigned int toggle[2]; /* one bit for each endpoint ([0] = IN, [1] = OUT) */

    int maxpacketsize;      /* Maximum packet size; one of: PACKET_SIZE_* */
    int epmaxpacketin[16];  /* INput endpoint specific maximums */
    int epmaxpacketout[16]; /* OUTput endpoint specific maximums */

    struct usb_device_descriptor descriptor; /* Device Descriptor */
    struct usb_config config; /* config descriptor */

    /* ? */
    int (*irq_handle)(struct usb_device *dev);
    u32 irq_status;
    int irq_act_len;    /* transfered bytes */
    void *privptr;

    /*
     * Child devices -  if this is a hub device
     * Each instance needs its own set of data structures.
     */
    u32 status;		/* the qTD status after execute */
    int act_len;    /* transfered bytes */
    int maxchild;   /* Number of ports if hub */
    int portnr;
    struct usb_device *parent;
    struct usb_device *children[USB_MAX_CHILDREN];
};

/*
 *
 * Data structure of USB transaction
 *
 */
typedef void (*usb_complete_t)(void *);

 /* Queue Element Transfer Descriptor (qTD). */
struct qTD
{
	u32 qtd_next;		    /* see EHCI 3.5.1 */
	u32 qtd_altnext;		/* see EHCI 3.5.2 */
	u32 qtd_token;		    /* see EHCI 3.5.3 */
	u32 qtd_buffer[USB_MAX_QTD];	/* see EHCI 3.5.4 */
};

/* Queue Head (QH). */
struct QH
{
	u32 qh_link;
	u32 qh_endpt1;
	u32 qh_endpt2;
	u32 qh_curtd;
	struct qTD qh_overlay;
	usb_complete_t complete;	/* (in) completion routine */
};

#endif

/*******************************************************************************/

