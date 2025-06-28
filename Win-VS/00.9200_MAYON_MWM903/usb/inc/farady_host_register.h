/*

Copyright (c) 2012 Mars Semiconductor Corp.

Module Name:

	farady_host_register.h

Abstract:

   	The declarations of FARADY USB host register.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2012/07/26	Civic Wu	Create

*/


#define OTG210_OTG_STATUS_SPD_TYPE_LS   (1<<22)  /* Host Speed Type */
#define OTG210_OTG_STATUS_SPD_TYPE_FS   (0<<22)  /* Host Speed Type */
#define OTG210_OTG_STATUS_SPD_TYPE_HS   (2<<22)  /* Host Speed Type */
#define OTG210_OTG_STATUS_SPD_TYPE   (3<<22)  /* Host Speed Type */
#define OTG210_OTG_ID      (1<<21)  /* Current ID, 0:A-device, 1:B-Device */
#define OTG210_OTG_STATUS_CROLE      (1<<20)  /* Current Role, 0:Host, 1:Device */

#define OTG210_OTG_VBUS_VLD      (1<<19)  
#define OTG210_OTG_A_SESS_VLD      (1<<18)  
#define OTG210_OTG_B_SESS_VLD      (1<<17)  
#define OTG210_OTG_B_SESS_END      (1<<16)  


#define OTG210_OTG_HDISCON      (1<<11)  
#define OTG210_OTG_VBUS_FLT_SEL      (1<<10) 

#define OTG210_OTG_ID_FLT_DEL      (1<<9) 
#define OTG210_OTG_A_SRP_RESP_TYP      (1<<8) 
#define OTG210_OTG_A_SRP_DET_EN      (1<<7) 
#define OTG210_OTG_A_SET_B_HNP_EN      (1<<6) 
#define OTG210_OTG_A_BUS_DROP      (1<<5) 
#define OTG210_OTG_A_BUS_REQ      (1<<4) 

#define OTG210_OTG_B_DSCHRG_VBUS      (1<<2) 
#define OTG210_OTG_B_HNP_EN      (1<<1) 
#define OTG210_OTG_B_BUS_REQ      (1<<0) 

#define MARS_OTG_CTLS_DEFAULT	( OTG210_OTG_STATUS_SPD_TYPE_HS |OTG210_OTG_VBUS_VLD | OTG210_OTG_A_SESS_VLD | \
								OTG210_OTG_B_SESS_VLD | OTG210_OTG_ID_FLT_DEL | OTG210_OTG_A_BUS_DROP| (0<<12) )


#define R_HPROT_DEFULT	(3<<7)
#define OTG210_MISC_EOF2_Time_default        (3<<4)   /* EOF2_Timing points */
#define OTG210_MISC_EOF1_Time_default        (3<<2)   /* EOF1_Timing points */
#define OTG210_MISC_ASYN_SCH_SLPT_default    (1<<0)   /* Asyn Schedule sleep timer  10us*/

#define MARS_OTG_HCMISC_DEFAULT	(R_HPROT_DEFULT | OTG210_MISC_EOF2_Time_default | OTG210_MISC_EOF1_Time_default  |OTG210_MISC_ASYN_SCH_SLPT_default)

#define	OTG210_INT_MASK_DEV	(1<<0)
#define	OTG210_INT_MASK_OTG	(1<<1)
#define	OTG210_INT_MASK_HC	(1<<2)
#define	OTG210_INT_POLARITY	(1<<3)

#define MARS_OTG_INTMASK_DEFAUL	OTG210_INT_MASK_DEV|OTG210_INT_MASK_OTG


#define USB_CHIP_ENABLE	(0x20)

#define IRQ_INTERVAL_MASK	0xFF0000
#define ONE_MICRO_FRAME		0x1
#define TWO_MICRO_FRAME		0x2
#define FOR_MICRO_FRAME		0x4
#define EIG_MICRO_FRAME		0x8


#define ASYNC_PARK_MASK		0xb00
#define ASYNC_PARK_COUNT_VALUE		0x100

