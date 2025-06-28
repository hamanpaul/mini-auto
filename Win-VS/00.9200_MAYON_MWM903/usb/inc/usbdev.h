/*

Copyright (c) 2008 Mars Semiconductor Corp.
Module Name:

	usbdev.h

Abstract:

   	The declarations of USB device.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_DEV_H__
#define __USB_DEV_H__

/*-------- Constant --------*/
#define USB_TIMEOUT		100

/*---- Interface number ----*/
#define USB_INTERFACE_MAX		0x02

/*---- Endpoint index ----*/
#define USB_EP_CTRLIN			0x00
#define USB_EP_CTRLOUT			0x01
#define USB_EP_BULKIN			0x02
#define USB_EP_BULKOUT			0x03
#define USB_EP_INTRIN			0x04
#define USB_EP_ISOIN			0x05
#define USB_EP_MAX			0x06

/*---- Endpoint number ----*/
#define USB_ENDPOINT_CTRL		0x00
#define USB_ENDPOINT_BULKIN		0x01
#define USB_ENDPOINT_BULKOUT		0x02
#define USB_ENDPOINT_INTRIN		0x03
#define USB_ENDPOINT_ISOIN		0x04
#define USB_ENDPOINT_MAX		0x05

/*---- Semaphore index ----*/
#define USB_SEM_EP_CTRLIN			0x00
#define USB_SEM_EP_CTRLOUT			0x01
#define USB_SEM_EP_BULKIN			0x02
#define USB_SEM_EP_BULKOUT			0x03
#define USB_SEM_EP_INTRIN			0x04
#define USB_SEM_EP_ISOIN			0x05
#define USB_SEM_EP				0x06
#define USB_SEM_EP_MAX				0x07

#define USB_SEM_API_TERMINATE			0x00
#define USB_SEM_API_ATTACH			0x01
#define USB_SEM_API_DETACH			0x02
#define USB_SEM_API_SEND			0x03
#define USB_SEM_API_RECV			0x04
#define USB_SEM_API_EP_CANCEL			0x05
#define USB_SEM_API_EP_RESET			0x06
#define USB_SEM_API_EP_STALL			0x07
#define USB_SEM_API_EP_CLEAR_STALL		0x08
#define USB_SEM_API_REMOTE_WAKEUP		0x09
#define USB_SEM_API_QUERY_STATUS		0x0a
#define USB_SEM_API				0x0b
#define USB_SEM_API_MAX				0x0c

#define USB_SEM_API_NOTIFY_TERMINATE		0x00
#define USB_SEM_API_NOTIFY_ATTACH		0x01
#define USB_SEM_API_NOTIFY_DETACH		0x02
#define USB_SEM_API_NOTIFY_SEND			0x03
#define USB_SEM_API_NOTIFY_RECV			0x04
#define USB_SEM_API_NOTIFY_EP_CANCEL		0x05
#define USB_SEM_API_NOTIFY_EP_RESET		0x06
#define USB_SEM_API_NOTIFY_EP_STALL		0x07
#define USB_SEM_API_NOTIFY_EP_CLEAR_STALL	0x08
#define USB_SEM_API_NOTIFY_REMOTE_WAKEUP	0x09
#define USB_SEM_API_NOTIFY_QUERY_STATUS		0x0a
#define USB_SEM_API_NOTIFY			0x0b
#define USB_SEM_API_NOTIFY_MAX			0x0c

/*---- API request ----*/

/* timeout value */
#define USB_API_REQ_TIMEOUT		100
#define USB_API_NOTIFY_TIMEOUT		100

/* notify */
#define USB_API_OPT_NOTIFY		0x00000001

/* send / receive */
#define USB_API_RECV			0x00
#define USB_API_SEND			0x01

/*---- Endpoint request ----*/

/* timeout value */
#define USB_EP_REQ_TIMEOUT		0x0100

/* reqOption */
#define USB_EP_REQ_OPT_NOTIFY		0x01
#define USB_EP_REQ_OPT_FIRST_RCV	0x02
#define USB_EP_REQ_OPT_END_ZERO_PKT	0x04
#define USB_EP_REQ_OPT_ABORT		0x08

/* svcStat */
#define USB_EP_REQ_STAT_OK		0x01    /* success transfer */
#define USB_EP_REQ_STAT_ERR		0x02    /* error transfer */
#define USB_EP_REQ_STAT_ABORT		0x04    /* abort transfer */
#define USB_EP_REQ_STAT_DETACH		0x08    /* bus detach */
#define USB_EP_REQ_STAT_RESET		0x10    /* bus reset */
#define USB_EP_REQ_STAT_CANCEL		0x20    /* cancel transfer */
#define USB_EP_REQ_STAT_TIMEOUT		0x40    /* timeout transfer */
#define USB_EP_REQ_STAT_EXIST		0x80	/* transfer request exist (check during transfer) */

/* DMA source */
#define USB_DMA_NOT_FIRST_RCV		0x00
#define USB_DMA_FIRST_RCV		0x01


/*-------- Type definition --------*/

/* Device setup */
typedef struct _USB_DEV_SETTING
{
    u16		devAddr;			/* device address */
    u8		numCfg;				/* number of configuration */
    u8		numIf;				/* number of interface */
    u8		numEp;				/* number of endpoint */
    u8		cfgNum;				/* configuration number */
    u8   		ifNum;                     	/* interface number */
    u8		altSet[USB_INTERFACE_MAX];	/* alternate setting */
    u16		devStat;			/* device status */
    u16		ifStat;				/* interface status */
    u16		epStat[USB_ENDPOINT_MAX];	/* endpoint status */
}
USB_DEV_SETTING;

/* Interrupt event queue to signal usbTask() */
#define USB_MAX_INT_EVT		32			/* max. interrupt event queued */
typedef struct _USB_INT_EVT
{
    u8		cause[USB_MAX_INT_EVT];   	/* cause of interrupt event */
    u8	  	idxSet;                         /* index of set event */
    u8	  	idxGet;                         /* index of get event */
}
USB_INT_EVT;

/* API event queue to sigal usbTask() */
#define USB_MAX_API_EVT		32      		/* max. api event queued */
typedef struct _USB_API_EVT
{
    u8		cause[USB_MAX_API_EVT];		/* cause of api event */
    u8		epNum[USB_MAX_API_EVT];		/* endpoint number */
    u8		option[USB_MAX_API_EVT];	/* option */
    u8   		idxSet;                     	/* index of set event */
    u8		idxGet;                     	/* index of get event */
}
USB_API_EVT;

typedef struct _USB_API_EVT_SNAP
{
    u8		cause;				/* cause of api event */
    u8		epNum;				/* endpoint number */
    u8		option;				/* option */
}
USB_API_EVT_SNAP;

/* transfer request */
typedef struct _USB_EP_REQ
{
    u8*		pData;				/* data requested */
    u32		reqSize;        		/* data size requested */
    u8		reqOption;			/* option requested */
    u32		svcSize;			/* data size serviced */
    u8		svcStat;			/* status serviced */
}
USB_EP_REQ;

typedef struct _USB_EP_REQ_RET
{
    u32*		pRetSize;			/* data size to return */
    u8*		pRetStat;			/* status to return */
}
USB_EP_REQ_RET;

#endif
