/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbvc.h

Abstract:

   	The declarations of USB Video Class related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_VC_H__
#define __USB_VC_H__

#if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
//test rf
typedef struct _CAMERA_STATUS_ {
    //u32 last_tick;
    u8  NeedUpdate;
    u8  battery;
    u8  motion;
    u8  ARM_en;
    
}CAMERA_STATUS;
extern CAMERA_STATUS CamStatus[8];
#endif
/* Constant */
//Device Mask of Interrupt Source Group 0 Register
#define USBD_MCX_COMABORT_INT	(1<<5)
#define USBD_MCX_COMFAIL_INT	(1<<4)
#define USBD_MCX_COMEND			(1<<3)
#define USBD_MCX_OUT_INT		(1<<2)
#define USBD_MCX_IN_INT			(1<<1)
#define USBD_MCX_SETUP_INT		(1<<0)



//Device Mask of Interrupt Source Group 1 Register
#define USBD_MF3_IN_INT			(1<<19)
#define USBD_MF2_IN_INT			(1<<18)
#define USBD_MF1_IN_INT			(1<<17)
#define USBD_MF0_IN_INT			(1<<16)
#define USBD_MF3_SPK_INT		(1<<7)
#define USBD_MF3_OUT_INT		(1<<6)
#define USBD_MF2_SPK_INT		(1<<5)
#define USBD_MF2_OUT_INT		(1<<4)
#define USBD_MF1_SPK_INT		(1<<3)
#define USBD_MF1_OUT_INT		(1<<2)
#define USBD_MF0_SPK_INT		(1<<1)
#define USBD_MF0_OUT_INT		(1<<0)

//Device Mask of Interrupt Source Group 2 Register

#define USBD_MDEV_WAKEUP_BYVBUS		(1<<10)
#define USBD_MDEV_IDLE				(1<<11)
#define USBD_MDMA_ERROR				(1<<8)
#define USBD_MDMA_CMPLT				(1<<7)
#define USBD_MRX0BYTE_INT			(1<<6)
#define USBD_MTX0BYTE_INT			(1<<5)
#define USBD_MSEQ_ABORT_INT			(1<<4)
#define USBD_MSEQ_ERR_INT			(1<<3)
#define USBD_MRESM_INT				(1<<2)
#define USBD_MSUSP_INT				(1<<1)
#define USBD_MUSBRST_INT			(1<<0)

//Device Receive Zero-Length Data Packet Register
#define USBD_RX0BYTE_EP8			(1<<7)
#define USBD_RX0BYTE_EP7			(1<<6)
#define USBD_RX0BYTE_EP6			(1<<5)
#define USBD_RX0BYTE_EP5			(1<<4)
#define USBD_RX0BYTE_EP4			(1<<3)
#define USBD_RX0BYTE_EP3			(1<<2)
#define USBD_RX0BYTE_EP2			(1<<1)
#define USBD_RX0BYTE_EP1			(1<<0)

//Device Transfer Zero-Length Data Packet Register
#define USBD_TX0BYTE_EP8			(1<<7)
#define USBD_TX0BYTE_EP7			(1<<6)
#define USBD_TX0BYTE_EP6			(1<<5)
#define USBD_TX0BYTE_EP5			(1<<4)
#define USBD_TX0BYTE_EP4			(1<<3)
#define USBD_TX0BYTE_EP3			(1<<2)
#define USBD_TX0BYTE_EP2			(1<<1)
#define USBD_TX0BYTE_EP1			(1<<0)


//Pipe type in Fifo config
#define USBD_PIPE_TYP_ISO	1
#define USBD_PIPE_TYP_BULK	2
#define USBD_PIPE_TYP_INT	3

typedef __packed struct _USB_ALT_SETTING
{
	USB_IF_DESC		interface;
	USB_EP_DESC		EP0;
	USB_EP_DESC		EP1;
	USB_EP_DESC		EP2;
	USB_EP_DESC		EP3;
	USB_EP_DESC		EP4;
	USB_EP_DESC		EP5;
	USB_EP_DESC		EP6;

}
USB_ALT_SETTING;

typedef __packed struct _USB_ISP_CONFIGURATION_DESC
{
	USB_CFG_DESC	cfg;
    USB_ALT_SETTING alt[9];
    
}USB_ISP_CONFIGURATION_DESC;

/* descriptor type */
#define	USB_DESC_TYPE_DEVICE			        0x01
#define USB_DESC_TYPE_CONFIGURATION		        0x02
#define USB_DESC_TYPE_STRING			        0x03
#define USB_DESC_TYPE_INTERFACE			        0x04
#define USB_DESC_TYPE_ENDPOINT			        0x05
#define USB_DESC_TYPE_DEVICE_QUALIFIER		    0x06
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION	0x07
#define USB_DESC_TYPE_INTERFACE_POWER		    0x08

/* descriptor length */
#define	USB_DESC_LEN_DEVICE			            0x12
#define USB_DESC_LEN_CONFIGURATION		        0x09
#define USB_DESC_LEN_STRING			            0x00		/* TBD */
#define USB_DESC_LEN_INTERFACE			        0x09
#define USB_DESC_LEN_ENDPOINT			        0x07
#define USB_DESC_LEN_DEVICE_QUALIFIER		    0x0a
#define USB_DESC_LEN_OTHER_SPEED_CONFIGURATION	0x09
#define USB_DESC_LEN_INTERFACE_POWER		    0x00		/* TBD */


/* bmRequestType */
#define USB_DEV_REQ_DIR_MASK		0x80	/* Data transfer direction */
#define USB_DEV_REQ_DIR_H2D		    0x00	/* Host-to-device */
#define USB_DEV_REQ_DIR_D2H		    0x80	/* Device-to-host */

#define USB_DEV_REQ_TYP_MASK		0x60	/* Type */
#define USB_DEV_REQ_TYP_STANDARD	0x00	/* Standard */
#define USB_DEV_REQ_TYP_CLASS		0x20	/* Class */
#define USB_DEV_REQ_TYP_VENDOR		0x40	/* Vendor */
#define USB_DEV_REQ_TYP_RESERVED	0x60	/* Reserved */

#define USB_DEV_REQ_REC_MASK		0x1f	/* Recipient */
#define USB_DEV_REQ_REC_DEV		    0x00	/* Device */
#define USB_DEV_REQ_REC_IF		    0x01	/* Interface */
#define USB_DEV_REQ_REC_EP		    0x02	/* Endpoint */

/* bRequest - Request Code (RC) */
#define USB_RC_GET_STATUS		    0x00	/* GET_STATUS */
#define USB_RC_CLEAR_FEATURE		0x01	/* CLEAR_FEATURE */
#define USB_RC_SET_FEATURE		    0x03	/* SET_FEATURE */
#define USB_RC_SET_ADDRESS		    0x05	/* SET_ADDRESS */
#define USB_RC_GET_DESCRIPTOR		0x06	/* GET_DESCRIPTOR */
#define USB_RC_SET_DESCRIPTOR		0x07	/* SET_DESCRIPTOR */
#define USB_RC_GET_CONFIGURATION	0x08	/* GET_CONFIGURATION */
#define USB_RC_SET_CONFIGURATION	0x09	/* SET_CONFIGURATION */
#define USB_RC_GET_INTERFACE		0x0a	/* GET_INTERFACE */
#define USB_RC_SET_INTERFACE		0x0b	/* SET_INTERFACE */
#define USB_RC_SYNCH_FRAME		    0x0c	/* SYNCH_FRAME */

#define PCAM_UPDATE_TX_START            0x01
#define PCAM_UPDATE_TX_CAM1             0x02
#define PCAM_UPDATE_TX_CAM2             0x03
#define PCAM_UPDATE_TX_CAM3             0x04
#define PCAM_UPDATE_TX_CAM4             0x05
#define PCAM_UPDATE_TX_CAM5             0x06
#define PCAM_UPDATE_TX_CAM6             0x07
#define PCAM_UPDATE_TX_CAM7             0x08
#define PCAM_UPDATE_TX_CAM8             0x09
#define PCAM_UPDATE_TX_FINISH           0x10

#define PCAM_UPDATE_RX_START            0x11
#define PCAM_UPDATE_RX_ERASE_FLASH      0x12
#define PCAM_UPDATE_RX_PROGRAM_FLASH    0x13
#define PCAM_UPDATE_RX_VERIFY_FLASH     0x14
#define PCAM_UPDATE_RX_FINISH           0x20


#define TEST_J          1
#define TEST_K          2
#define TEST_SE0_NAK    3
#define TEST_PACKET     4
#define TEST_FORCE_EN   5

/* language id */
#define USB_STR0_bLength			    0x04
#define USB_STR0_bDescriptorType		USB_DESC_TYPE_STRING
#define USB_STR0_wLANGID			    0x09, 0x04

/* string of manufacturer */
#define USB_STR1_bLength			0x0a
#define USB_STR1_bDescriptorType	USB_DESC_TYPE_STRING
#define USB_STR1_bString			'M', 0x00, 'A', 0x00, 'R', 0x00, 'S', 0x00
/* string of product */
#define USB_STR2_bLength			0x0c
#define USB_STR2_bDescriptorType	USB_DESC_TYPE_STRING
#define USB_STR2_bString			'A', 0x00, '1', 0x00, '0', 0x00, '1', 0x00, '6', 0x00

/* string of serial number */
#define USB_STR3_bLength			0x0a
#define USB_STR3_bDescriptorType	USB_DESC_TYPE_STRING
#define USB_STR3_bString			'0', 0x00, '1', 0x00, '0', 0x00, '0', 0x00

/* language id */

__align(4) const USB_DEV_DESC usb_isp_dev_desc =
{
    USB_DESC_LEN_DEVICE,		/* Size of this descriptor */
    USB_DESC_TYPE_DEVICE,		/* Device descriptor type */
    0x0200,			/* USB Specification Release Number in BCD */
    0xff,		    /* Class code */
    0xff,		    /* Subclass code */
    0xff,		    /* Protocol code */
    64,		        /* Max. packet size for endpoint zero */
    0x0d98,			/* Vendor ID */
#if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
    0x9600,			/* Product ID */
  #else
    0x8984,			/* Product ID */
  #endif
    0x1000,			/* Device release number in BCD */
    1,		        /* Index of string descriptor of manufacturer */
    2,			    /* Index of string descriptor of product */
    3,		        /* Index of string descriptor of serial number */
    1,		        /* Number of configurations */
};
#if USB_DONGLE_SUPPORT
__align(4) const USB_ISP_CONFIGURATION_DESC usb_isp_configuration_desc =
{
    /* cfg */
    {
        USB_DESC_LEN_CONFIGURATION,			/* Size of this descriptor */
        USB_DESC_TYPE_CONFIGURATION,		/* Configuration descriptor type */
      #if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
        USB_DESC_LEN_CONFIGURATION+(USB_DESC_LEN_INTERFACE+(USB_DESC_LEN_ENDPOINT*7))*4,		/* Total length of this configuration */
      #else
        USB_DESC_LEN_CONFIGURATION+(USB_DESC_LEN_INTERFACE+(USB_DESC_LEN_ENDPOINT*7))*9,		/* Total length of this configuration */
      #endif
        1,		    /* Number of interfaces */
        1,	        /* Value to select this configuration */
        0,		    /* Index of string descriptor of this configuration */
        0x80,		/* Configuration charateristics */
        0xFA,		/* Max. power consumption */
    },
    {
        {
            #include "USB_dev_Pcam_alt0.h"
        },
        {
            #include "USB_dev_Pcam_alt1.h"
        },
        {
            #include "USB_dev_Pcam_alt2.h"
        },
        {
            #include "USB_dev_Pcam_alt3.h"
        },
      #if((PROJECT_SELECT == 0) && (PROJECT_SELECT == 1))
        {
            #include "USB_dev_Pcam_alt4.h"
        },
        {
            #include "USB_dev_Pcam_alt5.h"
        },
        {
            #include "USB_dev_Pcam_alt6.h"
        },
        {
            #include "USB_dev_Pcam_alt7.h"
        },
        {
            #include "USB_dev_Pcam_alt8.h"
        }
      #endif
    }
};

#endif
typedef enum
{
	PCAM_CMD_NONE = 0,						//0x00
	PCAM_CMD_START_PREVIEW,					//0x01
	PCAM_CMD_STOP_PREVIEW,					//0x02
	PCAM_CMD_SET_RESOLUTION,				//0x03
	PCAM_CMD_SET_AC_FREQUENCY,				//0x04
	PCAM_CMD_GET_AC_FREQUENCY,				//0x05
	PCAM_CMD_SET_EXPOSURE_TARGET,			//0x06
	PCAM_CMD_GET_EXPOSURE_TARGET,			//0x07
	PCAM_CMD_SET_IN_OUT_DOOR_MODE,			//0x08
	PCAM_CMD_GET_IN_OUT_DOOR_MODE,			//0x09
	PCAM_CMD_SET_BRIGHTNESS,				//0x0A
	PCAM_CMD_GET_BRIGHTNESS,				//0x0B
	PCAM_CMD_SET_SATURATION,				//0x0C
	PCAM_CMD_GET_SATURATION,				//0x0D
	PCAM_CMD_SET_SHARPNESS,					//0x0E
	PCAM_CMD_GET_SHARPNESS,					//0x0F
	PCAM_CMD_SET_GAMMA,						//0x10
	PCAM_CMD_GET_GAMMA,						//0x11
	PCAM_CMD_SET_JPEG_QUALITY,				//0x12
	PCAM_CMD_GET_JPEG_QUALITY,				//0x13
	PCAM_CMD_CAMERA_SEL=0x14,				//0x14
	PCAM_CMD_CAMERA_ZOOM=0x15,				//0x15
	PCAM_CMD_CAMERA_PEN_U=0x16,				//0x16
	PCAM_CMD_CAMERA_PEN_D=0x17,				//0x17
	PCAM_CMD_CAMERA_PEN_L=0x18,				//0x18
	PCAM_CMD_CAMERA_PEN_R=0x19,				//0x19
	PCAM_CMD_GET_STATUS=0x1A,				//0x1A
	PCAM_CMD_PAIR=0x1B,				        //0x1B
	PCAM_CMD_GET_PAIR_STATUS=0x1C,			//0x1C
    PCAM_CMD_ENABLE_SMALL_STREAM=0x1E,
    PCAM_CMD_DISABLE_SMALL_STREAM=0x1F,
    
    PCAM_CMD_OS_IS_LINUX=0x20,              //0x20
    PCAM_CMD_RESET_LOCK_BUF,

    PCAM_CMD_SIGNAL_MODE=0x30,
    PCAM_CMD_QUAD_MODE,
    PCAM_CMD_CLR_TX_PIR,
    PCAM_CMD_GET_TX_PIR,

	PCAM_CMD_SET_TIME=0x40,					//0x40
	PCAM_CMD_SET_CAMSWITCH,
	PCAM_CMD_SET_CAMCONF,
  #if((PROJECT_SELECT == 0) || (PROJECT_SELECT == 1))
    PCAM_CMD_SET_MOTION,
  #endif
	PCAM_CMD_SET_TVSYSTEM=0x44,
	PCAM_CMD_SET_OSDMODE=0x45,
	PCAM_CMD_SET_SMALLSTREAM_Q,
	PCAM_CMD_STOP_PAIR,
  #if((PROJECT_SELECT == 2) || (PROJECT_SELECT == 3))
    PCAM_CMD_SET_MOTION=0x48,               //4M
  #else
    PCAM_CMD_SET_TXFLIPMIRROR,              //1072
  #endif
    
    PCAM_CMD_GET_TIME=0x60,
	PCAM_CMD_GET_CAMSWITCH,			
	PCAM_CMD_GET_CAMCONF,	
	PCAM_CMD_GET_MOTION,
	PCAM_CMD_GET_TVSYSTEM,
	PCAM_CMD_GET_OSDMODE,
	PCAM_CMD_GET_SMALLSTREAM_Q,
	PCAM_CMD_GET_VERSION,
  #if((PROJECT_SELECT == 2) || (PROJECT_SELECT == 3))
	PCAM_CMD_GET_TXVERSION,
  #else
	PCAM_CMD_GET_VERSION_TIME,
  #endif
	PCAM_CMD_GET_VIDEO_READY,
	
    PCAM_CMD_SET_CAM_ENABLE=0x70,           
    PCAM_CMD_GET_CAM_ENABLE=0x71,           
    PCAM_CMD_GET_RESOLUTION=0x72,
    PCAM_CMD_SET_PIR_TIME=0x73,
    PCAM_CMD_GET_PIR_TIME=0x74,           

    PCAM_CMD_SET_AUDIO=0x80,
    PCAM_CMD_ENABLE_2WAY_AUDIO=0x81,
    PCAM_CMD_DISABLE_2WAY_AUDIO=0x82,
    PCAM_CMD_ENABLE_PTT=0x83,
    PCAM_CMD_DISABLE_PTT=0x84,
    
  #if((PROJECT_SELECT == 2) || (PROJECT_SELECT == 3))
    PCAM_CMD_REBOOT=0x8F,
    PCAM_CMD_FW_UPDATE=0xB0,
    PCAM_CMD_DL_VER_BIN,
    PCAM_CMD_DL_FW_BIN,
    PCAM_CMD_FW_UPDATE_TX,
    PCAM_CMD_FW_UPDATE_STATUS,
  #else
    PCAM_CMD_FW_UPGRADE=0x90,
  #endif
    PCAM_CMD_USB_RESET,
    
}_PCAM_CMD;

/* vendor id */
#define USB_VC_DEV_VENDOR_ID			0x7654
/* product id */
#define USB_VC_DEV_PRODUCT_ID			0x0123

/* class code */
#define USB_VC_CLASS_VENDOR			0xff

/* subclass code */
#define USB_VC_SUBCLASS_VENDOR			0x00

/* protocol code */
#define USB_VC_PROTOCOL_VENDOR			0x00

#endif
