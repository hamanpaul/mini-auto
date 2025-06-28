
#include "general.h"
#include "board.h"
#include "task.h"
#include "gpioapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "sysapi.h"


#if (USB_HOST == 1)

/*USB Host individual definition*/
#include "usb_main.h"
#include "ehci.h"
#include "farady_host_register.h"
#include "farady_host_api.h"
/*USB Host individual definition end*/


/*USB Host definition area*/
#define HOST_ROLE	0
#define DEVICE_ROLE	1
/*USB Host definition area end*/

extern struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
extern volatile struct ehci_hcor *hcor;

#if 0
s32 farady_ehci_init(void)
{
    u32 temp ;
    /* Create the semaphore */
    Ehci_SemEvt = OSSemCreate(0);

    hccr = (volatile  ehci_hccr * )FD_USBHOSTBase;
    hcor = (volatile  ehci_hcor * )FD_USBHOSTCorBase;
    HOST_DEVICE_SWITCH &= ~DEVICE_ROLE ;

    /*Wait until switch to HOST role*/
    while((OTGCtlS & BIT20) == BIT20);

    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);
    OTGCtlS = MARS_OTG_CTLS_DEFAULT ;


    DEBUG_UHOST("HCMisc %x\n", HCMisc);
    ehci_writel(&hcor->or_misc, MARS_OTG_HCMISC_DEFAULT);

    DEBUG_UHOST("OTGCtlS %x\n", HCMisc);
    OTGCtlS |= OTG210_OTG_A_BUS_REQ ;
    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);

    OTGCtlS &= ~OTG210_OTG_A_BUS_DROP ;
    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);

    DEBUG_UHOST("GLOBALInterruptMask %x\n", GLOBALInterruptMask);
    GLOBALInterruptMask = MARS_OTG_INTMASK_DEFAUL ;

    temp = ehci_readl(&hcor->or_usbintr);
    temp |= USBINTR_UE | USBINTR_UEE |USBINTR_PCE;
    ehci_writel(&hcor->or_usbintr, temp);

    DEBUG_UHOST("DeviceMainCtl %x\n", DeviceMainCtl);

    DeviceMainCtl = USB_CHIP_ENABLE ;

    /*clear interrupt interval*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp &= ~ IRQ_INTERVAL_MASK;
    temp |= ONE_MICRO_FRAME;
    /*Set 1 micro frame*/
    ehci_writel(&hcor->or_usbcmd, temp);


    /*clear park mode count*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp &= ~ ASYNC_PARK_MASK;
    temp |= ASYNC_PARK_COUNT_VALUE;
    /*Set 1 micro frame*/
    ehci_writel(&hcor->or_usbcmd, temp);

    ehci_writel(&hcor->or_asynclistaddr, (u32)usb_qh_buf_1);

    usbHost_qTD_Init();
    usbHost_Asyc_QH_List_Init();
    //usbHost_Asyc_Qtd_List_Init();

    /*RUN EHCIt*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp |= CMD_RUN;
    ehci_writel(&hcor->or_usbcmd, temp);

    DEBUG_UHOST("# HCUSBCMD %x\n", HCUSBCMD);
    DEBUG_UHOST("Init done\n");

    return 1;
}
#endif



/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */


int ehci_hcd_init(void)
{
    u32 temp ;

#if 0
    volatile unsigned int* test_usb=(volatile unsigned int* ) 0xC0040038;
    volatile unsigned int* test_clk=(volatile unsigned int* ) 0xd00b0000;
    volatile unsigned int* usb_grand=(volatile unsigned int* ) 0xC0040034;
    int temp2;
#endif
    DEBUG_UHOST("ehci_hcd_init\n");
    hccr = (struct ehci_hccr * )FD_USBHOSTBase;
    hcor = (volatile  struct ehci_hcor * )FD_USBHOSTCorBase;
    //DEBUG_UHOST("=>u1\n");
    //HOST_DEVICE_SWITCH &= ~DEVICE_ROLE ;
    // DEBUG_UHOST("=>u2\n");

    //temp2 = *usb_grand;
    //temp2 |= 0xFC0000;
    //*usb_grand = temp2;
    //printf("USB grand ...: %x\n",*usb_grand);

    //printf("Ori ARIBITER: %x\n",*test_usb);
    //printf("Ori CLK: %x\n",*test_clk);
    //temp2 =  *test_usb;
    //temp2 &= ~(0xC00000);
    //temp2 |= 0x400000;
    //*test_usb = temp2;
    //printf("AFT ARIBITER: %x\n",*test_usb);
    /*Wait until switch to HOST role*/
    //while((OTGCtlS & BIT20) == BIT20);
    // DEBUG_UHOST("=>u3\n");
    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);
    OTGCtlS = MARS_OTG_CTLS_DEFAULT ;
    // DEBUG_UHOST("=>u4\n");

    DEBUG_UHOST("HCMisc %x\n", HCMisc);
    //ehci_writel(&hcor->or_misc, MARS_OTG_HCMISC_DEFAULT);
    hcor->or_misc = MARS_OTG_HCMISC_DEFAULT;

    DEBUG_UHOST("OTGCtlS %x\n", HCMisc);
    OTGCtlS |= OTG210_OTG_A_BUS_REQ ;
    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);

    OTGCtlS &= ~OTG210_OTG_A_BUS_DROP ;
    DEBUG_UHOST("OTGCtlS %x\n", OTGCtlS);

    DEBUG_UHOST("GLOBALInterruptMask %x\n", GLOBALInterruptMask);
    GLOBALInterruptMask = MARS_OTG_INTMASK_DEFAUL ;

    temp = ehci_readl(&hcor->or_usbintr);
    temp |= USBINTR_UE | USBINTR_UEE |USBINTR_PCE;
    //ehci_writel(&hcor->or_usbintr, temp);
    hcor->or_usbintr = temp;

    DEBUG_UHOST("DeviceMainCtl %x\n", DeviceMainCtl);

    DeviceMainCtl = USB_CHIP_ENABLE ;
#if 0
    /*clear interrupt interval*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp &= ~ IRQ_INTERVAL_MASK;
    temp |= ONE_MICRO_FRAME;
    /*Set 1 micro frame*/
    ehci_writel(&hcor->or_usbcmd, temp);


    /*clear park mode count*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp &= ~ ASYNC_PARK_MASK;
    temp |= ASYNC_PARK_COUNT_VALUE;
    /*Set 1 micro frame*/
    ehci_writel(&hcor->or_usbcmd, temp);

    //	ehci_writel(&hcor->or_asynclistaddr, (u32)usb_qh_buf_1);

    //	usbHost_qTD_Init();
    //	usbHost_Asyc_QH_List_Init();
    //usbHost_Asyc_Qtd_List_Init();

    /*RUN EHCIt*/
    temp = ehci_readl(&hcor->or_usbcmd);
    temp |= CMD_RUN;
    ehci_writel(&hcor->or_usbcmd, temp);
#endif
    DEBUG_UHOST("# HCUSBCMD %x\n", HCUSBCMD);
    DEBUG_UHOST("Init done\n");
    return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
    DEBUG_UHOST("???ehci_hcd_stop???\n");

    return 0;
}


#endif
