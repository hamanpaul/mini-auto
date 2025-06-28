
#ifndef _USB_DEFS_H_
#define _USB_DEFS_H_

/********************************************************************************
 * This is customized area
 */
#define USB_ALTSETTINGALLOC	4
#define USB_MAXALTSETTING	128	/* Hard limit */

#define USB_MAX_DEVICE		8
#define USB_MAX_CONFIG		8
#define USB_MAX_INTERFACES	8
#define USB_MAX_ENDPOINTS	16
#define USB_MAX_CHILDREN    8	/* This is arbitrary */
#define USB_MAX_HUB         4
#define USB_MAX_QTD	        5

#define USB_CNTL_TIMEOUT    100    /* 100ms timeout */

#define USB_DEFAULT_FLS_SIZE	1024
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 1
/*******************************************************************************/

/********************************************************************************
 *
 * USB constants
 *
 */

/* Device and/or Interface Class codes */
#define USB_CLASS_PER_INTERFACE 0	/* for DeviceClass */
#define USB_CLASS_AUDIO         1
#define USB_CLASS_COMM          2
#define USB_CLASS_HID           3
#define USB_CLASS_PRINTER       7
#define USB_CLASS_MASS_STORAGE  8
#define USB_CLASS_HUB           9
#define USB_CLASS_DATA          10
#define USB_CLASS_VENDOR_SPEC   0xff

/* some HID sub classes */
#define USB_SUB_HID_NONE 0
#define USB_SUB_HID_BOOT 1

/* some UID Protocols */
#define USB_PROT_HID_NONE       0
#define USB_PROT_HID_KEYBOARD   1
#define USB_PROT_HID_MOUSE      2

/* Sub STORAGE Classes */
#define US_SC_RBC   1		/* Typically, flash devices */
#define US_SC_8020  2		/* CD-ROM */
#define US_SC_QIC   3		/* QIC-157 Tapes */
#define US_SC_UFI   4		/* Floppy */
#define US_SC_8070  5		/* Removable media */
#define US_SC_SCSI  6		/* Transparent */
#define US_SC_MIN   US_SC_RBC
#define US_SC_MAX   US_SC_SCSI

/* STORAGE Protocols */
#define US_PR_CB    1		/* Control/Bulk w/o interrupt */
#define US_PR_CBI   0		/* Control/Bulk/Interrupt */
#define US_PR_BULK  0x50	/* bulk only */
#define US_PR_UAS  0x62	/* uasp*/

/* USB types */
#define USB_TYPE_STANDARD   (0 << 5)
#define USB_TYPE_CLASS      (1 << 5)
#define USB_TYPE_VENDOR     (2 << 5)
#define USB_TYPE_RESERVED   (3 << 5)

/* USB recipients */
#define USB_RECIP_DEVICE    0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT  0x02
#define USB_RECIP_OTHER     0x03

/* USB directions & endpoint modifiers
 * static struct usb_endpoint_description function_default_A_1[] = {
 *
 *     {this_endpoint: 0, attributes: CONTROL,	 max_size: 8,  polling_interval: 0 },
 *     {this_endpoint: 1, attributes: BULK,	 max_size: 64, polling_interval: 0, direction: IN},
 *     {this_endpoint: 2, attributes: BULK,	 max_size: 64, polling_interval: 0, direction: OUT},
 *     {this_endpoint: 3, attributes: INTERRUPT, max_size: 8,  polling_interval: 0},
 *
 */
/* USB transfer direction */
#define USB_DIR_OUT 0x00
#define USB_DIR_IN  0x80

/* USB transfer type */
#define USB_ISOCHRONOUS 0
#define USB_INTERRUPT   1
#define USB_CONTROL     2
#define USB_BULK        3

/* USB device speeds */
#define USB_SPEED_FULL      0x0	/* 12Mbps */
#define USB_SPEED_LOW       0x1	/* 1.5Mbps */
#define USB_SPEED_HIGH      0x2	/* 480Mbps */
#define USB_SPEED_RESERVED  0x3

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE       0x01
#define USB_DT_CONFIG       0x02
#define USB_DT_STRING       0x03
#define USB_DT_INTERFACE    0x04
#define USB_DT_ENDPOINT     0x05
#define USB_DT_HID          (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT       (USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL     (USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB          (USB_TYPE_CLASS | 0x09)

#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30
/* From the USB 3.1 spec */
#define	USB_DT_SSP_ISOC_ENDPOINT_COMP	0x31


/* Descriptor sizes per descriptor type */
#define USB_DT_DEVICE_SIZE          18
#define USB_DT_CONFIG_SIZE          9
#define USB_DT_INTERFACE_SIZE       9
#define USB_DT_ENDPOINT_SIZE        7
#define USB_DT_ENDPOINT_AUDIO_SIZE  9	/* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE      7
#define USB_DT_HID_SIZE             9

/* Endpoints */
#define USB_ENDPOINT_NUMBER_MASK    0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK       0x80
#define USB_ENDPOINT_XFERTYPE_MASK 0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL  0
#define USB_ENDPOINT_XFER_ISOC     1
#define USB_ENDPOINT_XFER_BULK     2
#define USB_ENDPOINT_XFER_INT      3

/* USB Packet IDs (PIDs) */
#define USB_PID_UNDEF_0     0xf0
#define USB_PID_OUT         0xe1
#define USB_PID_ACK         0xd2
#define USB_PID_DATA0       0xc3
#define USB_PID_UNDEF_4     0xb4
#define USB_PID_SOF         0xa5
#define USB_PID_UNDEF_6     0x96
#define USB_PID_UNDEF_7     0x87
#define USB_PID_UNDEF_8     0x78
#define USB_PID_IN          0x69
#define USB_PID_NAK         0x5a
#define USB_PID_DATA1       0x4b
#define USB_PID_PREAMBLE    0x3c
#define USB_PID_SETUP       0x2d
#define USB_PID_STALL       0x1e
#define USB_PID_UNDEF_F     0x0f

/* Standard requests */
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09
#define USB_REQ_GET_INTERFACE       0x0A
#define USB_REQ_SET_INTERFACE       0x0B
#define USB_REQ_SYNCH_FRAME         0x0C

/* HID requests */
#define USB_REQ_GET_REPORT      0x01
#define USB_REQ_GET_IDLE        0x02
#define USB_REQ_GET_PROTOCOL    0x03
#define USB_REQ_SET_REPORT      0x09
#define USB_REQ_SET_IDLE        0x0A
#define USB_REQ_SET_PROTOCOL    0x0B

/* HID report type */
#define USB_REQ_REPORT_TYPE_INPUT       0x01
#define USB_REQ_REPORT_TYPE_OUTPUT      0x02
#define USB_REQ_REPORT_TYPE_FEATURE     0x03
#define USB_REQ_REPORT_TYPE_RESERVED    0x04    //04-FF

/* "pipe" definitions */
#define PIPE_ISOCHRONOUS    0
#define PIPE_INTERRUPT      1
#define PIPE_CONTROL        2
#define PIPE_BULK           3
#define PIPE_DEVEP_MASK     0x0007ff00

/* USB-status codes: */
#define USB_ST_ACTIVE       0x1		/* TD is active */
#define USB_ST_STALLED      0x2		/* TD is stalled */
#define USB_ST_BUF_ERR      0x4		/* buffer error */
#define USB_ST_BABBLE_DET   0x8		/* Babble detected */
#define USB_ST_NAK_REC      0x10	/* NAK Received*/
#define USB_ST_CRC_ERR      0x20	/* CRC/timeout Error */
#define USB_ST_BIT_ERR      0x40	/* Bitstuff error */
#define USB_ST_NOT_PROC     0x80000000L	/* Not yet processed */

/*
 *
 * Hub defines
 *
 */

/* Hub request types */
#define USB_RT_HUB	(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT	(USB_TYPE_CLASS | USB_RECIP_OTHER)

/* Hub Class feature numbers */
#define C_HUB_LOCAL_POWER   0
#define C_HUB_OVER_CURRENT  1

/* Port feature numbers */
#define USB_PORT_FEAT_CONNECTION     0
#define USB_PORT_FEAT_ENABLE         1
#define USB_PORT_FEAT_SUSPEND        2
#define USB_PORT_FEAT_OVER_CURRENT   3
#define USB_PORT_FEAT_RESET          4
#define USB_PORT_FEAT_POWER          8
#define USB_PORT_FEAT_LOWSPEED       9
#define USB_PORT_FEAT_HIGHSPEED      10
#define USB_PORT_FEAT_C_CONNECTION   16
#define USB_PORT_FEAT_C_ENABLE       17
#define USB_PORT_FEAT_C_SUSPEND      18
#define USB_PORT_FEAT_C_OVER_CURRENT 19
#define USB_PORT_FEAT_C_RESET        20

/* wPortStatus bits */
#define USB_PORT_STAT_CONNECTION    0x0001
#define USB_PORT_STAT_ENABLE        0x0002
#define USB_PORT_STAT_SUSPEND       0x0004
#define USB_PORT_STAT_OVERCURRENT   0x0008
#define USB_PORT_STAT_RESET         0x0010
#define USB_PORT_STAT_POWER         0x0100
#define USB_PORT_STAT_LOW_SPEED     0x0200
#define USB_PORT_STAT_HIGH_SPEED    0x0400	/* support for EHCI */
#define USB_PORT_STAT_SPEED	        (USB_PORT_STAT_LOW_SPEED | USB_PORT_STAT_HIGH_SPEED)

/* wPortChange bits */
#define USB_PORT_STAT_C_CONNECTION  0x0001
#define USB_PORT_STAT_C_ENABLE      0x0002
#define USB_PORT_STAT_C_SUSPEND     0x0004
#define USB_PORT_STAT_C_OVERCURRENT 0x0008
#define USB_PORT_STAT_C_RESET       0x0010

/* wHubCharacteristics (masks) */
#define HUB_CHAR_LPSM       0x0003
#define HUB_CHAR_COMPOUND   0x0004
#define HUB_CHAR_OCPM       0x0018

/* Hub Status & Hub Change bit masks */
#define HUB_STATUS_LOCAL_POWER	0x0001
#define HUB_STATUS_OVERCURRENT	0x0002

#define HUB_CHANGE_LOCAL_POWER	0x0001
#define HUB_CHANGE_OVERCURRENT	0x0002

/*
 *
 * Bit defines of EHCI Registers
 *
 */

#define USBCMD_PARK	        (1 << 11)		    /* enable "park" */
#define USBCMD_PARK_CNT(c)	(((c) >> 8) & 3)	/* how many transfers to park */
#define USBCMD_LRESET	    (1 << 7)		    /* partial reset */
#define USBCMD_IAAD	        (1 << 6)		    /* "doorbell" interrupt */
#define USBCMD_ASE		    (1 << 5)		    /* async schedule enable */
#define USBCMD_PSE		    (1 << 4)		    /* periodic schedule enable */
#define USBCMD_FLS_RESERVED (3 << 2)            /* frame list size */
#define USBCMD_FLS_256      (2 << 2)            /* frame list size 256 elements */
#define USBCMD_FLS_512      (1 << 2)            /* frame list size 512 elements */
#define USBCMD_FLS_1024     (0 << 2)            /* frame list size 1024 elements */
#define USBCMD_RESET	    (1 << 1)		    /* reset HC not bus */
#define USBCMD_RUN		    (1 << 0)		    /* start/stop HC */

#define USBSTS_ASS          (1 << 15)   /* Async Schedule Status */
#define USBSTS_PSS	        (1 << 14)   /* Periodic Schedule Status */
#define USBSTS_RECL	        (1 << 13)   /* Reclamation */
#define USBSTS_HALT	        (1 << 12)	/* Not running (any reason) */
#define USBSTS_IAA	        (1 << 5)	/* Interrupted on async advance */
#define USBSTS_FATAL        (1 << 4)	/* such as some PCI access errors */
#define USBSTS_FLR		    (1 << 3)	/* frame list rolled over */
#define USBSTS_PCD		    (1 << 2)	/* port change detect */
#define USBSTS_ERR		    (1 << 1)	/* "error" completion (overflow, ...) */
#define USBSTS_INT		    (1 << 0)	/* "normal" completion (short, ...) */

#define USBINTR_AAE         (1 << 5)    /* Interrupt on async adavance enable */
#define USBINTR_SEE         (1 << 4)    /* system error enable */
#define USBINTR_PCE         (1 << 2)    /* Port change detect enable */
#define USBINTR_UEE         (1 << 1)    /* USB error interrupt enable */
#define USBINTR_UE          (1 << 0)    /* USB interrupt enable */

#define PORTSC_WKOC_E		(1 << 22)	/* RW wake on over current */
#define PORTSC_WKDSCNNT_E	(1 << 21)	/* RW wake on disconnect */
#define PORTSC_WKCNNT_E	    (1 << 20)	/* RW wake on connect */
#define PORTSC_PO		    (1 << 13)	/* RW port owner */
#define PORTSC_PP		    (1 << 12)	/* RW,RO port power */
#define PORTSC_LS		    (3 << 10)	/* RO line status */
#define PORTSC_PR		    (1 << 8)	/* RW port reset */
#define PORTSC_SUSP		    (1 << 7)	/* RW suspend */
#define PORTSC_FPR		    (1 << 6)	/* RW force port resume */
#define PORTSC_OCC		    (1 << 5)	/* RWC over current change */
#define PORTSC_OCA		    (1 << 4)	/* RO over current active */
#define PORTSC_PEC		    (1 << 3)	/* RWC port enable change */
#define PORTSC_PE		    (1 << 2)	/* RW port enable */
#define PORTSC_CSC		    (1 << 1)	/* RWC connect status change */
#define PORTSC_CS		    (1 << 0)	/* RO connect status */
#define PORTSC_CLEAR	    (PORTSC_OCC | PORTSC_PEC | PORTSC_CSC)

/* check */

#define PORTSC_IS_LOWSPEED(x)	(((x) & PORTSC_LS) == (1 << 10))

/*
 *
 * Bit defines of USB Transfer structure
 *
 */
/* Queue Head */
/* Dword 0 */
#define	USB_QH_LINK_TYPE_FSTN	        (3 << 1)
#define	USB_QH_LINK_TYPE_SITD	        (2 << 1)
#define	USB_QH_LINK_TYPE_QH		        (1 << 1)
#define	USB_QH_LINK_TYPE_ITD	        (0 << 1)
#define USB_QH_LINK_PTR_T			    (1 << 0)
/* Dword 1 */
#define USB_QH_EP_CHAR_RL(x)            (((x) & 0xF) << 28)
#define USB_QH_EP_CHAR_C(x)             (((x) & 0x1) << 27)	/* High-speed device: 1 = NO, 0 = Yes */
#define USB_QH_EP_CHAR_MAX_PACKET(x)    (((x) & 0xFFF) << 16)
#define USB_QH_EP_CHAR_H                (1 << 15)
#define USB_QH_EP_CHAR_DTC		        (1 << 14)
#define USB_QH_EP_CHAR_EPS(x)           (((x) & 0x3) << 12)
#define USB_QH_EP_CHAR_EP(x)   		    (((x) & 0xF) << 8)
#define USB_QH_EP_CHAR_I(x)             (((x) & 0x1) << 7)
#define USB_QH_EP_CHAR_DEV_ADDR(x)      (((x) & 0x7F) << 0)
/* Dword 2 */
#define USB_QH_EP_CAP_MULT_THREE  	    (3 << 30)
#define USB_QH_EP_CAP_MULT_TWO   	    (2 << 30)
#define USB_QH_EP_CAP_MULT_ONE   	    (1 << 30)
#define USB_QH_EP_CAP_PORT_NUM(x)  	    (((x) & 0x7F) << 23)
#define USB_QH_EP_CAP_HUB_ADDR(x)  	    (((x) & 0x7F) << 16)
#define USB_QH_EP_CAP_UFRAME_CMASK(x)	(((x) & 0xFF) << 8)
#define USB_QH_EP_CAP_UFRAME_SMASK(x)	(((x) & 0xFF) << 0)

/* Queue Element Transfer Descriptor (qTD)*/
/* Dword 0, 1 */
#define USB_QTD_LINK_PTR_T			    (1 << 0)
/* Dword 2 */
#define USB_QTD_TOKEN_DT(x)	   		    ((x) << 31)               /* data toggle, see QH's dtc */
#define USB_QTD_TOKEN_TRANS_SIZE(x)	    (((x) & 0x7FFF) << 16)
#define USB_QTD_TOKEN_IOC(x)			((x) << 15)               /* interrupt on complete */
#define USB_QTD_TOKEN_C_PAGE(x)		    (((x) & 0x7) << 12)
#define USB_QTD_TOKEN_CERR(x)		    (((x) & 0x3) << 10)
#define USB_QTD_TOKEN_PID_SETUP		    (2 << 8)
#define USB_QTD_TOKEN_PID_IN		    (1 << 8)
#define USB_QTD_TOKEN_PID_OUT		    (0 << 8)
#define USB_QTD_TOKEN_STAT_ACTIVE	    (1 << 7)                /* HC may execute this */
#define USB_QTD_TOKEN_STAT_HALTED	    (1 << 6)                /* halted on error */
#define USB_QTD_TOKEN_STAT_DBE		    (1 << 5)                /* data buffer error (in HC) */
#define USB_QTD_TOKEN_STAT_BABBLE	    (1 << 4)                /* device was babbling (qtd halted) */
#define USB_QTD_TOKEN_STAT_XACT_ERR	    (1 << 3)                /* device gave illegal response */
#define USB_QTD_TOKEN_STAT_MISS_UFRAME	(1 << 2)                /* incomplete split transaction */
#define USB_QTD_TOKEN_STAT_SPLITXSTATE	(1 << 1)                /* split transaction state */
#define USB_QTD_TOKEN_STAT_PING		    (1 << 0)                /* issue PING? */

/*
 *
 * Status of Device execution
 *
 */

#define HID_INITIAL_STATUS	0
#define HID_POLL_STATUS	1
#define HID_INT_STATUS	2

#define HUB_INITIAL_STATUS	0
#define HUB_POLL_STATUS	1
#define HUB_INT_STATUS	2

/* Interval of periodic frame list */
#define POLL_HUB_INTERVAL	1024
#define POLL_HID_INTERVAL	10

/*******************************************************************************/

/********************************************************************************
 *
 * HID report descriptor defines
 *
 */

/* Item format */
#define HID_ITEM_FORMAT_SHORT   0
#define HID_ITEM_FORMAT_LONG    1
/* Speacial tag */
#define HID_ITEM_TAG_LONG   15
/* Item type */
#define HID_ITEM_TYPE_MAIN      0
#define HID_ITEM_TYPE_GLOBAL    1
#define HID_ITEM_TYPE_LOCAL     2
#define HID_ITEM_TYPE_RESERVED  3
/* Main items */
#define HID_MAIN_ITEM_INPUT             0x8
#define HID_MAIN_ITEM_OUTPUT            0x9
#define HID_MAIN_ITEM_COLLECTION        0xA
#define HID_MAIN_ITEM_FEATURE           0xB
#define HID_MAIN_ITEM_END_COLLECTION    0xC
/* Collection item type */
#define HID_COLLECTION_PHYSICAL     0
#define HID_COLLECTION_APPLICATION  1
#define HID_COLLECTION_LOGICAL      2
/* Global items */
#define HID_GLOBAL_ITEM_USAGE_PAGE          0x0
#define HID_GLOBAL_ITEM_LOGICAL_MINIMUM     0x1
#define HID_GLOBAL_ITEM_LOGICAL_MAXIMUM     0x2
#define HID_GLOBAL_ITEM_PHYSICAL_MINIMUM    0x3
#define HID_GLOBAL_ITEM_PHYSICAL_MAXIMUM    0x4
#define HID_GLOBAL_ITEM_UNIT_EXPONENT       0x5
#define HID_GLOBAL_ITEM_UNIT                0x6
#define HID_GLOBAL_ITEM_REPORT_SIZE         0x7
#define HID_GLOBAL_ITEM_REPORT_ID           0x8
#define HID_GLOBAL_ITEM_REPORT_COUNT        0x9
#define HID_GLOBAL_ITEM_PUSH                0xA
#define HID_GLOBAL_ITEM_POP                 0xB
/* Local items */
#define HID_LOCAL_ITEM_USAGE                0x0
#define HID_LOCAL_ITEM_USAGE_MINIMUM        0x1
#define HID_LOCAL_ITEM_USAGE_MAXIMUM        0x2
#define HID_LOCAL_ITEM_DESIGNATOR_INDEX     0x3
#define HID_LOCAL_ITEM_DESIGNATOR_MINIMUM   0x4
#define HID_LOCAL_ITEM_DESIGNATOR_MAXIMUM   0x5
#define HID_LOCAL_ITEM_STRING_INDEX         0x7
#define HID_LOCAL_ITEM_STRING_MINIMUM       0x8
#define HID_LOCAL_ITEM_STRING_MAXIMUM       0x9
#define HID_LOCAL_ITEM_DELIMITER            0xA
/* Usage Pages table */
#define HID_USAGE_PAGE_GENERIC_DESKTOP_CONTROLS 0x1
#define HID_USAGE_PAGE_KEYBOARD_KEYPAD          0x7
#define HID_USAGE_PAGE_LEDS                     0x8
#define HID_USAGE_PAGE_BUTTON                   0x9
/* Generic Desktop Page table */
#define HID_GENERIC_DESKTOP_PAGE_MOUSE      0x02
#define HID_GENERIC_DESKTOP_PAGE_KEYBOARD   0x09
#define HID_GENERIC_DESKTOP_PAGE_X          0x30
#define HID_GENERIC_DESKTOP_PAGE_Y          0x31
#define HID_GENERIC_DESKTOP_PAGE_Z          0x32
#define HID_GENERIC_DESKTOP_PAGE_RX         0x33
#define HID_GENERIC_DESKTOP_PAGE_RY         0x34
#define HID_GENERIC_DESKTOP_PAGE_RZ         0x35
#define HID_GENERIC_DESKTOP_PAGE_WHEEL      0x38
#define HID_GENERIC_DESKTOP_PAGE_VX         0x40
#define HID_GENERIC_DESKTOP_PAGE_VY         0x41
#define HID_GENERIC_DESKTOP_PAGE_VZ         0x42
/* Keyboard Keypad Page */
#define HID_KEYBOARD_KEYPAD_PAGE_NEI			0x00
#define HID_KEYBOARD_KEYPAD_PAGE_ERRORROLLOVER	0x01
#define HID_KEYBOARD_KEYPAD_PAGE_POSTFAIL		0x02
#define HID_KEYBOARD_KEYPAD_PAGE_ERRORUNDEFINED	0x03
#define HID_KEYBOARD_KEYPAD_PAGE_RETURN			0x28
#define HID_KEYBOARD_KEYPAD_PAGE_ESCAPE			0x29
#define HID_KEYBOARD_KEYPAD_PAGE_DELETE			0x2A
#define HID_KEYBOARD_KEYPAD_PAGE_TAB			0x2B
#define HID_KEYBOARD_KEYPAD_PAGE_SPACEBAR		0x2C
#define HID_KEYBOARD_KEYPAD_PAGE_CAPSLOCK       0x39
#define HID_KEYBOARD_KEYPAD_PAGE_PRINTSCREEN	0x46
#define HID_KEYBOARD_KEYPAD_PAGE_SCROLLLOCK		0x47
#define HID_KEYBOARD_KEYPAD_PAGE_PAUSE			0x48
#define HID_KEYBOARD_KEYPAD_PAGE_INSERT			0x49
#define HID_KEYBOARD_KEYPAD_PAGE_HOME			0x4A
#define HID_KEYBOARD_KEYPAD_PAGE_PAGEUP			0x4B
#define HID_KEYBOARD_KEYPAD_PAGE_DELETEFORWARD	0x4C
#define HID_KEYBOARD_KEYPAD_PAGE_END			0x4D
#define HID_KEYBOARD_KEYPAD_PAGE_PAGEDOWN		0x4E
#define HID_KEYBOARD_KEYPAD_PAGE_RIGHTARROW		0x4F
#define HID_KEYBOARD_KEYPAD_PAGE_LEFTARROW		0x50
#define HID_KEYBOARD_KEYPAD_PAGE_DOWNARROW		0x51
#define HID_KEYBOARD_KEYPAD_PAGE_UPARROW		0x52
#define HID_KEYBOARD_KEYPAD_PAGE_NUMKOCK        0x53
#define HID_KEYBOARD_KEYPAD_PAGE_ENTER			0x58
#define HID_KEYBOARD_KEYPAD_PAGE_APPLICATION	0x65
#define HID_KEYBOARD_KEYPAD_PAGE_POWER			0x66
#define HID_KEYBOARD_KEYPAD_PAGE_EQUAL			0x67
/* too much, ignore...... */
#define HID_KEYBOARD_KEYPAD_PAGE_LEFTCONTROL	0xE0
#define HID_KEYBOARD_KEYPAD_PAGE_LEFTSHIFT		0xE1
#define HID_KEYBOARD_KEYPAD_PAGE_LEFTALT		0xE2
#define HID_KEYBOARD_KEYPAD_PAGE_LEFTGUI		0xE3
#define HID_KEYBOARD_KEYPAD_PAGE_RIGHTCONTROL	0xE4
#define HID_KEYBOARD_KEYPAD_PAGE_RIGHTSHIFT		0xE5
#define HID_KEYBOARD_KEYPAD_PAGE_RIGHTALT		0xE6
#define HID_KEYBOARD_KEYPAD_PAGE_RIGHTGUI		0xE7

static const u8 HID_KEYBOARD_KEYPAD_PAGE_VALUE[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
    0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B,
    0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
    0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9,
    0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD,
    0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1,
    0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB,
    0xDC, 0xDD, 0xDE, 0xEF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5,
    0xE6, 0xE7
};

static const char* HID_KEYBOARD_KEYPAD_PAGE_SCHAR[] =
{
    "(no event indicated)", "ErrorRollOver", "POSTFail", "ErrorUndefined", "a", "b", "c", "d", "e", "f",
    "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
    "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "Return (ENTER)", "ESCAPE", "DELETE (Backspace)", "Tab", "Spacebar", "-", "=", "[", "]", "\\",
    "Non-US # and ~", ";", "`", "Grave Accent and Tilde", ",", ".", "/", "Caps Lock", "F1",	"F2",
    "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11",	"F12",
    "PrintScreen", "Scroll Lock", "Pause", "Insert", "Home", "PageUp", "Delete Forward", "End", "PageDown",	"RightArrow",
    "LeftArrow", "DownArrow", "UpArrow", "Num Lock and Clear", "/", "*", "-", "+", "ENTER",	"1",
    "2", "3", "4", "5", "6", "7", "8", "9", "0", ".",
    "Non-US \\ and |", "Application", "Power", "=", "F13", "F14", "F15", "F16", "F17", "F18",
    "F19", "F20", "F21", "F22", "F23", "F24", "Execute", "Help", "Menu", "Select",
    "Stop", "Again", "Undo", "Cut", "Copy", "Paste", "Find", "Mute", "Volume Up", "Volume Down",
    "Locking Caps Lock", "Locking Num Lock", "Locking Scroll Lock", "Comma", "Equal Sign", "Keyboard International", "Keyboard International", "Keyboard International", "Keyboard International", "Keyboard International",
    "Keyboard International", "Keyboard International", "Keyboard International", "Keyboard International", "Keyboard LANG1", "Keyboard LANG2", "Keyboard LANG3", "Keyboard LANG4", "Keyboard LANG5", "Keyboard LANG6",
    "Keyboard LANG7", "Keyboard LANG8", "Keyboard LANG9", "Keyboard Alternate Erase", "Keyboard SysReq/Attention", "Keyboard Cancel", "Keyboard Clear", "Keyboard Prior", "Keyboard Return", "Keyboard Separator",
    "Keyboard Out", "Keyboard Oper", "Keyboard Clear/Again", "Keyboard CrSel/Props", "Keyboard ExSel", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",	"Keypad 00", "Keypad 000", "Thousands Separator", "Decimal Separator",
    "Currency Unit", "Currency Sub-unit34", "Keypad (", ")", "{", "}", "Tab", "Backspace", "A", "B",
    "C", "D", "E", "F", "XOR", "^", "%", "<", ">", "&",
    "&&", "|", "||", "", "#", "Space", "@", "!", "Memory Store", "Memory Recall",
    "Memory Clear", "Memory Add", "Memory Subtract", "Memory Multiply", "Memory Divide", "+/-", "Clear", "Clear Entry", "Binary", "Octal",
    "Decimal", "Keypad Hexadecimal", "Reserved", "Reserved", "Keyboard LeftControl", "Keyboard LeftShift", "Keyboard LeftAlt", "Keyboard Left GUI", "Keyboard RightControl", "Keyboard RightShift",
    "Keyboard RightAlt", "Keyboard Right GUI"
};

static const char* HID_KEYBOARD_KEYPAD_PAGE_BCHAR[] =
{
    "", "", "", "", "A", "B", "C", "D", "E", "F",
    "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
    "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
    "", "", "", "", "", "_", "+", "{", "}", "|",
    "~", ":", "", "", "<", ">", "?", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "End",
    "Down Arrow", "PageDn", "Left Arrow", "", "Right Arrow", "Home", "Up Arrow", "PageUp", "Insert", "Delete",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "", ""
};


/* LED Page */
#define HID_LED_PAGE_UNDEFINED      0x00
#define HID_LED_PAGE_NUMLOCK        0x01
#define HID_LED_PAGE_CAPSLOCK       0x02
#define HID_LED_PAGE_SCROLLLOCK     0x03
#define HID_LED_PAGE_COMPOSE        0x04
#define HID_LED_PAGE_KANA           0x05
#define HID_LED_PAGE_POWER          0x06
#define HID_LED_PAGE_SHIFT          0x07
#define HID_LED_PAGE_DONOTDISTURB   0x08
#define HID_LED_PAGE_MUTE           0x09
/* too much, ignore...... */
/* Consumer Page table */
#define HID_CONSUMER_PAGE_AC_PAN    0x0238
/* others */
#define HID_INFO_COLLECT_SIZE   5
#define HID_GLOBAL_ITEMS_NN     4

/*******************************************************************************/

#endif /*_USB_DEFS_H_ */
