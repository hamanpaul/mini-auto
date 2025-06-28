
#include "general.h"
#include "board.h"
#include "task.h"
#include "osapi.h"
#include "sysapi.h"
#include "uiapi.h"
#include "usbapi.h"

#include <usb_main.h>
#include "ehci.h"
#include "farady_host_api.h"
#if (USB_HOST == 1)

#define USB_BUFSIZ	512
#define ALIGN_SIZE	0x20

__align(4) static struct usb_device usb_dev[USB_MAX_DEVICE];

/*
 * for dev's hid qTD format and record
 * usb_dev <=> hid_item_list -> same index, devnum - 1
 */
struct usb_hid_simple_rd_item hid_item_list[USB_MAX_DEVICE][HID_INFO_COLLECT_SIZE];
u8 usb_mouse_hid_format[HID_INFO_COLLECT_SIZE*2]; //btn, constant, xy, wheel and one more byte
u8 usb_keyboard_hid_format[HID_INFO_COLLECT_SIZE*2]; //Control keycode, constant, LEDs, constant, keycode
u8 usb_keyboard_hid_leds_switch;    //prevent USB 1.1 in LED keycoed double shoot
u8 usb_keyboard_leds_status = 0x1;


struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
volatile struct ehci_hcor *hcor;

int udev_index; /* point to the newest device */
//static int running;
static int asynch_allowed;
u32	gEhci_status=0;
u32	gEhci_periodic_status=0;
u32	bus_reset=0;

u32 hub_running=0;
u32 mouse_running=HID_INITIAL_STATUS;
u32 keyboard_running=HID_INITIAL_STATUS;
u32 hub_mode=HUB_INITIAL_STATUS; //@POLL
u32 mouse_packet_size=0, keyboard_packet_size = 0;
u32 ghub_addr=0;
u32 ghub_port=0;
char usb_started,usb_init_flag=0; /* flag for the started/stopped USB status */

//u8 usb_msc_mode;



/*OS related global variable*/
USB_INT_EVT usbHostIntEvt; /* Interrupt event queue */
OS_EVENT* ehci_INT_SemEvt; /* semaphore to synchronize event processing */
OS_EVENT* ehci_Atomic_SemEvt; /* semaphore to synchronize event processing */
OS_STK usbHostTaskStack[USB_HOST_TASK_STACK_SIZE]; /* Stack of task usbTask() */
OS_EVENT* usbHostSemEvt; /* semaphore to synchronize event processing */

OS_EVENT* SCSI_SemEvt; /* semaphore to usb write / read commamd */
/*OS related global variable end*/

#define USB_MAX_STOR_DEV 3

extern struct usb_device *ghub_dev;
extern struct usb_device *ghid_dev_mouse, *ghid_dev_keyboard;
extern char hub_port;
extern struct us_data usb_stor[USB_MAX_STOR_DEV];
extern int usb_stor_scan(int);
extern int usb_stor_info(void);
extern void Mouse_update(int moveX, int moveY, u8 btns);

//u8 gucUsbPlugInStat;
u8 gUSBDevOn;
/*
void usbInitLuns(void)
{
}

void usbDevEnaCtrl(u8 ucResCtrl)
{
}

u8 usbUninst(void)
{
}

u8 usbSetUsbPluginStat(u8 test)
{
}
*/
/**********************************************************************
 * some forward declerations...
 */
static void usb_scan_devices(void);
int usb_mouse_hid_probe(struct usb_device *dev, struct usb_interface *iface, struct usb_endpoint_descriptor *ep);
int usb_keyboard_hid_probe(struct usb_device *dev, struct usb_interface *iface, struct usb_endpoint_descriptor *ep);
void usb_set_hid_format(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *usb_dev_hid_format, u8 *buf, int *length);
void usb_hid_bit_format(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *usb_dev_hid_format);
int usb_hid_report_descriptor_handler(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *buffer, int bufindex, struct hid_item *item);

/***************************************************************************
 * Init USB Device
 */
extern int ehci_hcd_init(void);
extern  int ehci_reset(void);

s32 usbHostInit(void)
{
#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) &&(HW_BOARD_OPTION== MR9300_RX_RDI))
    UsbHubRst();//hub reset
#endif
    /* Create the semaphore */
    ehci_Atomic_SemEvt = OSSemCreate(1);
    SCSI_SemEvt= OSSemCreate(1);
    ehci_INT_SemEvt = OSSemCreate(0);
    usbHostSemEvt = OSSemCreate(0);
    
#if !USB1_SWITCH_USB2  
    sysUSB_enable();
    DEBUG_UHOST("SYS_ANA_TEST1=%x\n",SYS_ANA_TEST1);
    SYS_ANA_TEST1|=0X80;
    SYS_ANA_TEST1&=(~0x01);
    DEBUG_UHOST("SYS_ANA_TEST1=%x\n",SYS_ANA_TEST1);
#else //usb2
    
    sysUSB2_enable();
    SYS_ANA_TEST1_usb2|=0X80;
    SYS_ANA_TEST1_usb2&=(~0x01);
    DEBUG_STORAGE("SYS_ANA_TEST1_usb2=%x\n",SYS_ANA_TEST1_usb2);
#endif        
    DEBUG_UHOST("SYS_CPU_PLLCTL=%x\n",SYS_CPU_PLLCTL);
    DEBUG_UHOST("SYS_CLK2=%x\n",SYS_CLK2);
#if 0
    printf("SYS_ANA_TEST1=%x\n",SYS_ANA_TEST1);
    printf("HCCapbility=%x\n",HCCapbility);
    printf(" HCSparams=%x\n",HCSparams);
    printf("HCCparams=%x\n",HCCparams);
    printf("HCUSBCMD=%x\n",HCUSBCMD);
    printf("HCUSBSTS=%x\n",HCUSBSTS);
    printf("HCUSBINTR=%x\n",HCUSBINTR);
    printf("HCFRindex=%x\n",HCFRindex);
    printf("HCPeriodicListBS=%x\n",HCPeriodicListBS);
    printf("HCAsyncListAddr=%x\n",  HCAsyncListAddr);
    printf("HCPortSC=%x\n",HCPortSC);
    printf("HCMisc=%x\n",HCMisc);

    printf("GLOBALInterruptS=%x\n",GLOBALInterruptS);
    printf("GLOBALInterruptMask=%x\n",GLOBALInterruptMask);

    printf("OTGInterruptS=%x\n",OTGInterruptS);
    printf("OTGInterruptEnable=%x\n",OTGInterruptEnable);
    printf("OTGCtlS=%x\n",OTGCtlS);
#endif
    memset((void *)&usbHostIntEvt, 0, sizeof(USB_INT_EVT));
    if(0==OSTaskCreate(USB_HOST_TASK, USB_HOST_TASK_PARAMETER, USB_HOST_TASK_STACK, USB_HOST_TASK_PRIORITY))
    {
        DEBUG_UHOST("USB_HOST_TASK OK\n ");
    }
    //DEBUG_UHOST("EHCI HC reset ");
    /* EHCI spec section 4.1 */
    if (ehci_reset() != 0)
    {
        //	return -1;
    }
    //DEBUG_UHOST("done \n");
    DEBUG_UHOST("Platform HC reset\n ");
    if (ehci_hcd_init() != 0)
        return -1;
    DEBUG_UHOST("done \n");
    return 1;
}

int usb_init(void)
{
    int result;

    udev_index = 0;
    asynch_allowed = 1;
    usb_hub_reset();
    /* init low_level USB */


    result = usb_lowlevel_init();
    /* if lowlevel init is OK, scan the bus for devices
     * i.e. search HUBs and configure them */
    //  OSTimeDly(400);
    if (result == 0)
    {
        DEBUG_UHOST("scanning bus for devices...\n");
        usb_scan_devices();

        //OSTimeDly(400);
        usb_started = 1;
        if( usb_stor_scan(1)==-1)
        {
            DEBUG_STORAGE("USB:usb_stor_scan retry\n");
            //OSTimeDly(60);
            if( usb_stor_scan(1)==-1)
            {
                DEBUG_STORAGE("Error, scan No storage \n");
                usb_started = 0;
                return -1;
            }
        }
        if(usb_stor_info())
        {
            DEBUG_STORAGE("Error, No storage devices\n");
            usb_started = 0;
            return -1;
        }
        return 0;
    }
    else
    {
        DEBUG_STORAGE("Error, couldn't init Lowlevel part\n");
        usb_started = 0;
        return -1;
    }


}


/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
int usb_stop(void)
{
    int res = 0;

    if (usb_started)
    {
        asynch_allowed = 1;
        usb_started = 0;
        usb_hub_reset();
        res = usb_lowlevel_stop();
    }
    return res;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
int usb_disable_asynch(int disable)
{
    int old_value = asynch_allowed;

    asynch_allowed = !disable;
    return old_value;
}


/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message
 */
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int transfer_len, int interval)
{
    return submit_int_msg(dev, pipe, buffer, transfer_len, interval);
}

/*
 * submits a control message and waits for comletion (at least timeout * 1ms)
 * If timeout is 0, we don't wait for completion (used as example to set and
 * clear keyboards LEDs). For data transfers, (storage transfers) we don't
 * allow control messages with 0 timeout, by previousely resetting the flag
 * asynch_allowed (usb_disable_asynch(1)).
 * returns the transfered length if OK or -1 if error. The transfered length
 * and the current status are stored in the dev->act_len and dev->status.
 */
int usb_control_msg(struct usb_device *dev, unsigned int pipe, unsigned char request, unsigned char requesttype,
                    unsigned short value, unsigned short index, void *data, unsigned short size, int timeout)
{
    __align(4) struct devrequest setup_packet[1];
	u8 ret;

    if ((timeout == 0) && (!asynch_allowed))
    {
        /* request for a asynch control pipe is not allowed */
		DEBUG_STORAGE("@request for a asynch control pipe is not allowed \n");
        return -1;
    }
ret=3;
do
{
    /* set setup command */
    setup_packet->requesttype = requesttype;		//USB_DIR_IN
    setup_packet->request = request;
    setup_packet->value = cpu_to_le16(value);
    setup_packet->index = cpu_to_le16(index);
    setup_packet->length = cpu_to_le16(size);
    DEBUG_UHOST("usb_control_msg : request: 0x%X, requesttype: 0x%X, value 0x%X index 0x%X length 0x%X\n",
                setup_packet->request, setup_packet->requesttype, setup_packet->value, setup_packet->index, size);
    dev->status = USB_ST_NOT_PROC; /*not yet processed */
    // Data is the buffer where store device decriptor from device
    submit_control_msg(dev, pipe, data, size, setup_packet);
    if (timeout == 0)
        return (int)size;
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
       if ((HCPortSC & PORTSC_CS)==0)
        {
            printf("\n@gUSBDevOn out\n");
            dev->status = USB_ST_BUF_ERR;
           return -1;
        }
#endif	   
	if(dev->status & USB_ST_NOT_PROC)
		{
		   DEBUG_STORAGE("not yet retry %d",ret);  
		}
	        if (ret==0)
                   break;
		ret--;
}while(dev->status & USB_ST_NOT_PROC);
			
    /*
     * Wait for status to update until timeout expires, USB driver
     * interrupt handler may set the status when the USB operation has
     * been completed.
     */
    while (timeout--)
    {
        //if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
        if (!(dev->status & USB_ST_NOT_PROC))
            break;
        if (gUSBDevOn== 0)
        {
            printf("\n@out\n");
            dev->status = USB_ST_BUF_ERR;
            break;
        }
	//ehci_mdelay(1);
        OSTimeDly(1);
     if(timeout==0)
     DEBUG_STORAGE("@timeout 0x%X \n",timeout);
    }
    if (dev->status)
        return -1;

    return dev->act_len;

}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * -1 if Error.
 * synchronous behavior
 */
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe, void *data, int len, int *actual_length, int timeout)
{
    if (len < 0)
        return -1;
    dev->status = USB_ST_NOT_PROC; /*not yet processed */
    submit_bulk_msg(dev, pipe, data, len);
    while (timeout--)
    {
        //if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
        if (!(dev->status & USB_ST_NOT_PROC))
            break;
	if (gUSBDevOn== 0)
        {
            printf("\n@1out\n");
            dev->status = USB_ST_BUF_ERR;
            break;
        }
       // OSTimeDly(1);	
        ehci_mdelay(1);
    if(timeout==0)
     DEBUG_STORAGE("@bulk_timeout 0x%X \n",timeout);
    }
    *actual_length = dev->act_len;
    if (dev->status == 0)
        return 0;
    else
        return -1;
}


/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
int usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
    /* direction is out -> use emaxpacket out */
    if ((pipe & USB_DIR_IN) == 0)
        return dev->epmaxpacketout[((pipe>>15) & 0xf)];
    else
        return dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

/*
 * The routine usb_set_maxpacket_ep() is extracted from the loop of routine
 * usb_set_maxpacket(), because the optimizer of GCC 4.x chokes on this routine
 * when it is inlined in 1 single routine. What happens is that the register r3
 * is used as loop-count 'i', but gets overwritten later on.
 * This is clearly a compiler bug, but it is easier to workaround it here than
 * to update the compiler (Occurs with at least several GCC 4.{1,2},x
 * CodeSourcery compilers like e.g. 2007q3, 2008q1, 2008q3 lite editions on ARM)
 *
 * NOTE: Similar behaviour was observed with GCC4.6 on ARMv5.
 */
static void
usb_set_maxpacket_ep(struct usb_device *dev, int if_idx, int ep_idx)
{
    int b;
    struct usb_endpoint_descriptor *ep;
    u16 ep_wMaxPacketSize;

    ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

    b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
#ifdef USB_UNDO
    ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);
#endif
    ep_wMaxPacketSize = ep->wMaxPacketSize;
    if ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL)
    {
        /* Control => bidirectional */
        dev->epmaxpacketout[b] = ep_wMaxPacketSize;
        dev->epmaxpacketin[b] = ep_wMaxPacketSize;
        DEBUG_UHOST("##Control EP epmaxpacketout/in[%d] = %d\n", b, dev->epmaxpacketin[b]);
    }
    else
    {
        if ((ep->bEndpointAddress & 0x80) == 0)
        {
            /* OUT Endpoint */
            if (ep_wMaxPacketSize > dev->epmaxpacketout[b])
            {
                dev->epmaxpacketout[b] = ep_wMaxPacketSize;
                DEBUG_UHOST("##EP epmaxpacket_out[%d] = %d\n", b, dev->epmaxpacketout[b]);
            }
        }
        else
        {
            /* IN Endpoint */
            if (ep_wMaxPacketSize > dev->epmaxpacketin[b])
            {
                dev->epmaxpacketin[b] = ep_wMaxPacketSize;
                DEBUG_UHOST("##EP epmaxpacket_in[%d] = %d\n", b, dev->epmaxpacketin[b]);
            }
        } /* if out */
    } /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
static int usb_set_maxpacket(struct usb_device *dev)
{
    int i, ii;

    for (i = 0; i < dev->config.desc.bNumInterfaces; i++)
        for (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
            usb_set_maxpacket_ep(dev, i, ii);

    return 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 */
static int usb_parse_config(struct usb_device *dev, unsigned char *buffer, int cfgno)
{
    struct usb_descriptor_header *head;
    struct usb_interface_descriptor *if_desc;
    struct usb_class_hid_descriptor *hid_desc;
    struct usb_endpoint_descriptor *ep_desc;
    int index, epindex;
    //u16 ep_wMaxPacketSize;  //HID input的最大Byte數，假如mouse那邊切割有問題，就需要注意這個值

    if (((struct usb_descriptor_header *)&buffer[0])->bDescriptorType != USB_DT_CONFIG)
    {
        DEBUG_UHOST(" ERROR: NOT USB_CONFIG_DESC %x\n", head->bDescriptorType);
        return -1;
    }

    memcpy(&dev->config.desc, buffer, buffer[0]);    //因為__packed貼齊關係，所以把config的desc貼滿
    dev->configno = cfgno;
    dev->config.no_of_if = -1;   //接下來會依照順序加入Intefcae, HID, EP資訊，所以這裡順便做Counter
    index = dev->config.desc.bLength;   //設定config後的起始位置，config的wTotalLength，會接著Inferface、HID、Endpoint

#if USB_UNDO
    le16_to_cpus(&(dev->config.desc.wTotalLength));
#endif

    /* Ok the first entry must be a configuration entry, now process the others */
    while(index + 1 < dev->config.desc.wTotalLength)
    {
        head = (struct usb_descriptor_header *)&buffer[index];
        switch(head->bDescriptorType)
        {
            case USB_DT_INTERFACE:
                dev->config.no_of_if++;
                if_desc = (struct usb_interface_descriptor *) head;
                memcpy(&dev->config.if_desc[dev->config.no_of_if], &buffer[index], if_desc->bLength);
               
                /*init the desc counter*/
                
//  DEBUG_UHOST(//"********usb interface*********\n"\
                DEBUG_UHOST( "bLength: %#x\n", if_desc->bLength);
                DEBUG_UHOST( "bDescriptorType:%#x\n", if_desc->bDescriptorType);
                DEBUG_STORAGE("if %d, no_of_if %d\n", if_desc->bAlternateSetting, dev->config.no_of_if);
				
                // if(dev->config.if_desc[dev->config.no_of_if].desc.bInterfaceClass == USB_CLASS_MASS_STORAGE)
                // 	{
				DEBUG_UHOST( "bInterfaceNumber: %#x\n", if_desc->bInterfaceNumber);
                            DEBUG_UHOST( "bAlternateSetting: %#x\n",if_desc->bAlternateSetting);
                            DEBUG_UHOST( "bNumEndpoints: %#x\n",if_desc->bNumEndpoints);
                            DEBUG_UHOST("bInterfaceClass: %#x\n", if_desc->bInterfaceClass);
                            DEBUG_UHOST( "bInterfaceSubClass: %#x\n",if_desc->bInterfaceSubClass);
                            DEBUG_UHOST("bInterfaceProtocol: %#x\n",if_desc->bInterfaceProtocol);
                            DEBUG_UHOST("iInterface: %#x\n",if_desc->iInterface);
				
                           DEBUG_UHOST("==================\n");			
                 	//}	 
                if(dev->config.if_desc[dev->config.no_of_if].desc.bInterfaceClass == USB_CLASS_HID)
                    dev->config.if_desc[dev->config.no_of_if].no_of_hid = 0;
                epindex = 0;   //在Interface底下會有複數個Endpoint，所以設定Interface時，順便Init epindex
                 
                break;
            case USB_DT_HID:
                hid_desc = (struct usb_class_hid_descriptor *)head;
                memcpy(&dev->config.if_desc[dev->config.no_of_if].hid_desc, &buffer[index], hid_desc->bLength);
                dev->config.if_desc[dev->config.no_of_if].no_of_hid++;
                if(if_desc->bInterfaceSubClass == USB_SUB_HID_BOOT)
                    dev->hid_report_len = hid_desc->wDescriptorLength;  //簡單類型的device也只會使用Boot interface所屬的HID
                DEBUG_UHOST("********usb_hub_probe*******\n"\
                            "hid_desc->[attribute]: value\n"\
                            "****************************\n"\
                            "bLength: 0x%02x\n"\
                            "bDescriptorType: 0x%02x\n"\
                            "bcdHID: 0x%04x\n"\
                            "bCountryCode: 0x%02x\n"\
                            "bNumDescriptor: 0x%02x\n"\
                            "bReportDescriptorType: 0x%02x\n"\
                            "bReportLength: 0x%02x\n"\
                            "****************************\n"
                            , hid_desc->bLength
                            , hid_desc->bDescriptorType
                            , hid_desc->bcdHID
                            , hid_desc->bCountryCode
                            , hid_desc->bNumDescriptors
                            , hid_desc->bDescriptorType0
                            , hid_desc->wDescriptorLength);
                break;
            case USB_DT_ENDPOINT:
                ep_desc = (struct usb_endpoint_descriptor *)head;
                memcpy(&dev->config.if_desc[dev->config.no_of_if].ep_desc[epindex], &buffer[index], ep_desc->bLength);
                /*
                #if USB_UNDO
                ep_wMaxPacketSize = get_unaligned(&dev->config.if_desc[if_desc->bInterfaceNumber].ep_desc[epindex].wMaxPacketSize);
                put_unaligned(le16_to_cpu(ep_wMaxPacketSize), &dev->config.if_desc[if_desc->bInterfaceNumber].ep_desc[epindex].wMaxPacketSize);
                #else
                ep_wMaxPacketSize = dev->config.if_desc[if_desc->bInterfaceNumber].ep_desc[epindex].wMaxPacketSize;
                #endif
                */
                DEBUG_UHOST("ifno %d, ep %d\n", dev->config.no_of_if, epindex);
                epindex++;
                break;
            default:
                if (head->bLength == 0)
                    return 1;
                DEBUG_UHOST("Unrecognized Class-Specific Description Type : %x\n", head->bDescriptorType);
                {
                    //#ifdef USB_DEBUG
                    u8 *ch = (unsigned char *)head;
                    int i;
                    //#endif
                    for (i = 0; i < head->bLength; i++)
                        DEBUG_UHOST("%02X ", *ch++);
                    ch = ch;	// avoid warming msg
                    DEBUG_UHOST("\n\n\n");
                }
                break;
        }
        index += head->bLength;
    }
    return 1;
}
#if 0
/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
static int usb_parse_config_1(struct usb_device *dev,unsigned char *buffer, int cfgno)
{
	struct usb_descriptor_header *head;
	int index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize;
	struct usb_interface *if_desc = NULL;

	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (struct usb_descriptor_header *) &buffer[0];
	if (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		return -EINVAL;
	}
	if (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		return -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (struct usb_descriptor_header *) &buffer[index];
	while (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		switch (head->bDescriptorType) {
		case USB_DT_INTERFACE:
			if (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				break;
			}
			if (((struct usb_interface_descriptor *) head)->bInterfaceNumber != curr_if_num)
				{
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				if (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					return -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num = if_desc->desc.bInterfaceNumber;
			      } 
			else
				{
				/* found alternate setting for the interface */
				if (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			if (head->bLength != USB_DT_ENDPOINT_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_ENDPOINT_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				break;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			if (epno > USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				return -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			memcpy(&if_desc->ep_desc[epno], head,
				USB_DT_ENDPOINT_SIZE);
			ep_wMaxPacketSize = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							wMaxPacketSize);
			put_unaligned(le16_to_cpu(ep_wMaxPacketSize),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					wMaxPacketSize);
			debug("if %d, ep %d\n", ifno, epno);
			break;
		case USB_DT_SS_ENDPOINT_COMP:
			if (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				break;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			break;
		default:
			if (head->bLength == 0)
				return -EINVAL;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned char *ch = (unsigned char *)head;
				int i;

				for (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	return 0;
}
#endif
/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
int usb_clear_halt(struct usb_device *dev, int pipe)
{
    int result;
    int endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    DEBUG_STORAGE("@usb_clear_halt\n");
    result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0, endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

    /* don't clear if failed */
    if (result < 0)
        return result;

    /*
     * NOTE: we do not get status and verify reset was successful
     * as some devices are reported to lock up upon this check..
     */

    usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

    /* toggle is reset on clear */
    usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
    return 0;
}


/**********************************************************************
 * get_descriptor type
 */
// buf is  &dev->descriptor
static int usb_get_descriptor(struct usb_device *dev, unsigned char type, unsigned char index, void *buf, int size)
{
    int res;
    DEBUG_UHOST("usb_get_descriptor\n");
    res = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
                          (type << 8) + index, 0, buf, size, USB_CNTL_TIMEOUT);
    return res;
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
int usb_get_configuration_no(struct usb_device *dev, unsigned char *buffer, int cfgno)
{
    int result;
    unsigned int tmp;
    struct usb_configuration_descriptor *config;

    config = (struct usb_configuration_descriptor *)&buffer[0];
    result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
    if (result < 9)
    {
        if (result < 0)
            DEBUG_STORAGE("unable to get descriptor, error %lX\n", dev->status);
        else
            DEBUG_STORAGE("config descriptor too short (expected %i, got %i)\n", 9, result);
        return -1;
    }
    tmp = le16_to_cpu(config->wTotalLength);

    if (tmp > USB_BUFSIZ)
    {
        DEBUG_STORAGE("usb_get_configuration_no: failed to get descriptor - too long: %d\n", tmp);
        return -1;
    }

    result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, tmp);
    DEBUG_UHOST("**************************\n"\
                "config->[attribute]: value\n"\
                "**************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "wTotalLength: 0x%04x\n"\
                "bNumInterfaces: 0x%02x\n"\
                "bConfigurationValue: 0x%02x\n"\
                "iConfiguration: 0x%02x\n"\
                "bmAttributes: 0x%02x\n"\
                "bMaxPower: 0x%02x\n"\
                "**************************\n"
                , config->bLength
                , config->bDescriptorType
                , config->wTotalLength
                , config->bNumInterfaces
                , config->bConfigurationValue
                , config->iConfiguration
                , config->bmAttributes
                , config->bMaxPower);
    DEBUG_UHOST("get_conf_no %d Result %d, wLength %d\n", cfgno, result, tmp);
    return result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
static int usb_set_address(struct usb_device *dev)
{
    int res;

    DEBUG_UHOST("set address(dev->devnum value) %d\n", dev->devnum);
    res = usb_control_msg(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS, 0, (dev->devnum), 0, NULL, 0, USB_CNTL_TIMEOUT);
    return res;
}

/********************************************************************
 * set interface number to interface
 */
int usb_set_interface(struct usb_device *dev, int interface, int alternate)
{
    struct usb_interface *if_face = NULL;
    int ret, i;

    for (i = 0; i < dev->config.desc.bNumInterfaces; i++)
    {
        if (dev->config.if_desc[i].desc.bInterfaceNumber == interface)
        {
            if_face = &dev->config.if_desc[i];
            break;
        }
    }
    if (!if_face)
    {
        DEBUG_UHOST("selecting invalid interface %d\n", interface);
        return -1;
    }
    /*
     * We should return now for devices with only one alternate setting.
     * According to 9.4.10 of the Universal Serial Bus Specification
     * Revision 2.0 such devices can return with a STALL. This results in
     * some USB sticks timeouting during initialization and then being
     * unusable in U-Boot.
     */

    if(dev->config.no_of_if ==0)
    {
        DEBUG_STORAGE(" only one alternate setting\n");
        return 0;
    }

    DEBUG_STORAGE("usb_set_interface %d\n", interface);
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),	USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
                          alternate, interface, NULL, 0, USB_CNTL_TIMEOUT * 5);
    if (ret < 0)
        return ret;

    return 0;
}

/********************************************************************
 * set configuration number to configuration
 */
static int usb_set_configuration(struct usb_device *dev, int configuration)
{
    int res;
    DEBUG_UHOST("set configuration %d\n", configuration);
    /* set setup command */
    res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0), USB_REQ_SET_CONFIGURATION, 0, configuration, 0, NULL,
                          0, USB_CNTL_TIMEOUT);
    if (res == 0)
    {
        dev->toggle[0] = 0;
        dev->toggle[1] = 0;
        return 0;
    }
    else
        return -1;
}

/********************************************************************
 * set report
 */
int usb_set_report(struct usb_device *dev, int ifnum, unsigned char type, unsigned char id, void *buf, int size)
{
    u8 *report = (u8 *) buf;
    DEBUG_UHOST("usb_set_report %d\n", *report);
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0), USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set protocol to protocol
 */
int usb_set_protocol(struct usb_device *dev, int ifnum, int protocol)
{
    DEBUG_UHOST("usb_set_protocol %d\n", protocol);
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0), USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
int usb_set_idle(struct usb_device *dev, int ifnum, int duration, int report_id)
{
    DEBUG_UHOST("usb_set_idle \n");
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                           (duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
int usb_get_report(struct usb_device *dev, int ifnum, unsigned char type, unsigned char id, void *buf, int size)
{
    DEBUG_UHOST("usb_get_report \n");
    return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_REPORT,
                           USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE, (type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}


/********************************************************************
 * get class descriptor
 */
int usb_get_class_descriptor(struct usb_device *dev, int ifnum, unsigned char type, unsigned char id, void *buf, int size)
{
    DEBUG_UHOST("usb_get_class_descriptor\n");
    return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
                           (type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
static int usb_get_string(struct usb_device *dev, unsigned short langid, unsigned char index, void *buf, int size)
{
    int i;
    int result;

    for (i = 0; i < 3; ++i)
    {
        /* some devices are flaky */
        DEBUG_UHOST("usb_get_string \n");
        result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
                                 (USB_DT_STRING << 8) + index, langid, buf, size, USB_CNTL_TIMEOUT);

        if (result > 0)
            break;

        if(gUSBDevOn == 0)
        {
            DEBUG_STORAGE("Fail:DEV OUT\n");
            break;
        }
    }

    return result;
}


static void usb_try_string_workarounds(unsigned char *buf, int *length)
{
    int newlength, oldlength = *length;

    for (newlength = 2; newlength + 1 < oldlength; newlength += 2)
        if (!isprint(buf[newlength]) || buf[newlength + 1])
            break;

    if (newlength > 2)
    {
        buf[0] = newlength;
        *length = newlength;
    }
}


static int usb_string_sub(struct usb_device *dev, unsigned int langid, unsigned int index, unsigned char *buf)
{
    int rc;

    /* Try to read the string descriptor by asking for the maximum
     * possible number of bytes */
    rc = usb_get_string(dev, langid, index, buf, 255);

    /* If that failed try to read the descriptor length, then
     * ask for just that many bytes */
    if (rc < 2)
    {
        rc = usb_get_string(dev, langid, index, buf, 2);
        if (rc == 2)
            rc = usb_get_string(dev, langid, index, buf, buf[0]);
    }

    if (rc >= 2)
    {
        if (!buf[0] && !buf[1])
            usb_try_string_workarounds(buf, &rc);

        /* There might be extra junk at the end of the descriptor */
        if (buf[0] < rc)
            rc = buf[0];

        rc = rc - (rc & 1); /* force a multiple of two */
    }

    if (rc < 2)
        rc = -1;

    return rc;
}


/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
int usb_string(struct usb_device *dev, int index, char *buf, u32 size)
{
    unsigned char mybuf[USB_BUFSIZ];
    unsigned char *tbuf;
    int err;
    unsigned int u, idx;

    if (size <= 0 || !buf || !index)
        return -1;
    buf[0] = 0;
    tbuf = &mybuf[0];

    /* get langid for strings if it's not yet known */
    if (!dev->have_langid)
    {
        err = usb_string_sub(dev, 0, 0, tbuf);
        if (err < 0)
        {
            DEBUG_UHOST("error getting string descriptor 0 (error=%lx)\n", dev->status);
            return -1;
        }
        else if (tbuf[0] < 4)
        {
            DEBUG_UHOST("string descriptor 0 too short\n");
            return -1;
        }
        else
        {
            dev->have_langid = -1;
            dev->string_langid = tbuf[2] | (tbuf[3] << 8);
            /* always use the first langid listed */
            DEBUG_UHOST("USB device number %d default language ID 0x%x\n", dev->devnum, dev->string_langid);
        }
    }

    err = usb_string_sub(dev, dev->string_langid, index, tbuf);
    if (err < 0)
        return err;

    size--;		/* leave room for trailing NULL char in output buffer */
    for (idx = 0, u = 2; u < err; u += 2)
    {
        if (idx >= size)
            break;
        if (tbuf[u+1])			/* high byte */
            buf[idx++] = '?';  /* non-ASCII character */
        else
            buf[idx++] = tbuf[u];
    }
    buf[idx] = 0;
    err = idx;
    return err;
}


/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */


/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
struct usb_device *usb_get_dev_index(int index)
{
    if (usb_dev[index].devnum == -1)
        return NULL;
    else
        return &usb_dev[index];
}


/* returns a pointer of a new device structure or NULL, if
 * no device struct is available
 */
struct usb_device *usb_alloc_new_device(void)
{
    u8 i;
    //check USB_dev array has any space or not
    for(i = 0; i < USB_MAX_DEVICE; i++)
    {
        if(!usb_dev[i].enabled)
        {
            udev_index = i;
            break;
        }
        else if(i+1 == USB_MAX_DEVICE)
        {
            DEBUG_STORAGE("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
            return NULL;
        }
    }
    //renew this device parameter
    memset(&usb_dev[udev_index], 0, sizeof(struct usb_device));
    usb_dev[udev_index].enabled = 1;    //0 = disabled; 1 = enabled;
    /* default Address is 0, real addresses start with 1 */
    DEBUG_UHUB("New Device index number: %d\n", udev_index + 1);
    usb_dev[udev_index].devnum = udev_index + 1;
    usb_dev[udev_index].maxchild = 0;
    for (i = 0; i < USB_MAX_CHILDREN; i++)
        usb_dev[udev_index].children[i] = NULL;
    usb_dev[udev_index].parent = NULL;
    return &usb_dev[udev_index];
}

int usb_hid_probe(struct usb_device *dev, int ifnum)
{
    struct usb_interface *iface;
    struct usb_endpoint_descriptor *ep;
    int ret;

    if (dev->descriptor.bNumConfigurations != 1)
        return 0;

    iface = &dev->config.if_desc[ifnum];
    if (iface->desc.bInterfaceClass != USB_CLASS_HID)
        return 0;
    if (iface->desc.bInterfaceSubClass != USB_SUB_HID_BOOT)
        return 0;
    if (iface->desc.bNumEndpoints != 1)
        return 0;

    ep = &iface->ep_desc[iface->desc.bNumEndpoints-1];
    /* Check if endpoint 1 is interrupt endpoint */
    if (!(ep->bEndpointAddress & 0x80))
        return 0;
    if ((ep->bmAttributes & 3) != 3)
        return 0;
    DEBUG_UHOST("********usb_hid_probe*********\n"\
                "iface->desc.[attribute]: value\n"\
                "******************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "bInterfaceNumber: 0x%02x\n"\
                "bAlternateSetting: 0x%02x\n"\
                "bNumEndpoints: 0x%02x\n"\
                "bInterfaceClass: 0x%02x\n"\
                "bInterfaceSubClass: 0x%02x\n"\
                "bInterfaceProtocol: 0x%02x\n"\
                "iInterface: 0x%02x\n"\
                "******************************\n",
                iface->desc.bLength,
                iface->desc.bDescriptorType,
                iface->desc.bInterfaceNumber,
                iface->desc.bAlternateSetting,
                iface->desc.bNumEndpoints,
                iface->desc.bInterfaceClass,
                iface->desc.bInterfaceSubClass,
                iface->desc.bInterfaceProtocol,
                iface->desc.iInterface);
    DEBUG_UHOST("********usb_hid_probe********\n"\
                "ep->[attribute]: value\n"\
                "******************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "bEndpointAddress: 0x%02x\n"\
                "bmAttributes: 0x%02x\n"\
                "wMaxPacketSize: 0x%04x\n"\
                "bInterval: 0x%02x\n"\
                "******************************\n",
                ep->bLength,
                ep->bDescriptorType,
                ep->bEndpointAddress,
                ep->bmAttributes,
                ep->wMaxPacketSize,
                ep->bInterval);

    DEBUG_UHOST("ep->wMaxPacketSize: %x\n",ep->wMaxPacketSize);
    DEBUG_UHOST("ep->bEndpointAddress: %x\n",ep->bEndpointAddress);
    DEBUG_UHOST("ep->bInterval: %x\n",ep->bInterval );
    DEBUG_UHOST("dev->devnum: %x\n",dev->devnum);

    switch(iface->desc.bInterfaceProtocol)
    {
        case USB_PROT_HID_KEYBOARD:
            usb_keyboard_hid_probe(dev, iface, ep);
            break;
        case USB_PROT_HID_MOUSE:
            usb_mouse_hid_probe(dev, iface, ep);
            break;
        default:
            break;
    }

    return ret;
}

int usb_mouse_hid_probe(struct usb_device *dev, struct usb_interface *iface, struct usb_endpoint_descriptor *ep)
{
    int ret;
    //int pipe, maxp;
    /* We found a hub */
    DEBUG_UHOST("USB HID Mouse found\n");

    //pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
    //maxp = usb_maxpacket(dev, pipe);

    //usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);
    //usb_set_idle(dev, iface->desc.bInterfaceNumber, 10, 0);
    //usb_get_report(dev, iface->desc.bInterfaceNumber, 1, 0, (void *)&hid_buf_mouse[0], 16);
    //format_printf(0,32,(int)&hid_buf_mouse[0]);
    //usb_submit_int_msg(dev, pipe,&hid_buf_mouse[0], maxp > 8 ? 8 : maxp, ep->bInterval);
    usb_set_configuration(dev, 1);
    usb_get_class_descriptor(dev, iface->desc.bInterfaceNumber, USB_DT_REPORT, 0, (void *)&hid_buf_mouse[0], dev->hid_report_len);

    mouse_packet_size = ep->wMaxPacketSize;
    ghid_dev_mouse = dev;
    //check device already, set the mouse qTD input
    memset(&hid_item_list[ghid_dev_mouse->devnum-1], 0, sizeof(struct usb_hid_simple_rd_item) * HID_INFO_COLLECT_SIZE);
    memset(&usb_mouse_hid_format, 0, sizeof(usb_mouse_hid_format));
    usb_set_hid_format(hid_item_list[ghid_dev_mouse->devnum-1], usb_mouse_hid_format, (void *)&hid_buf_mouse[0], &dev->hid_report_len);

    DEBUG_UHOST("usb_mouse_hid_format: %d, %d, %d, %d, %d\n"
                , usb_mouse_hid_format[1], usb_mouse_hid_format[3], usb_mouse_hid_format[5]
                , usb_mouse_hid_format[7], usb_mouse_hid_format[9]);
    //usb_set_idle(dev, iface->desc.bInterfaceNumber, POLL_HID_INTERVAL, 0);
    //usb_set_idle(dev, iface->desc.bInterfaceNumber, 10, 0);
    //format_printf(0,0x57,(int)&hid_buf_mouse[0]);

    hid_qh_init(dev, ep, mouse_qh_addr, mouse_qtd, 0x1);
    ehci_mdelay(50);
    mouse_running = HID_POLL_STATUS;
    return ret;
}

int usb_keyboard_hid_probe(struct usb_device *dev, struct usb_interface *iface, struct usb_endpoint_descriptor *ep)
{
    int ret;
    /* We found a hub */
    DEBUG_UHOST("USB HID keyboard found\n");

    usb_set_configuration(dev, 1);
    usb_get_class_descriptor(dev, iface->desc.bInterfaceNumber, USB_DT_REPORT, 0, (void *)&hid_buf_keyboard[0], dev->hid_report_len);

    keyboard_packet_size = ep->wMaxPacketSize;
    ghid_dev_keyboard = dev;
    //check device already, set the mouse qTD input
    memset(&hid_item_list[ghid_dev_keyboard->devnum-1], 0, sizeof(struct usb_hid_simple_rd_item) * HID_INFO_COLLECT_SIZE);
    memset(&usb_keyboard_hid_format, 0, sizeof(usb_keyboard_hid_format));
    usb_set_hid_format(hid_item_list[ghid_dev_keyboard->devnum-1], usb_keyboard_hid_format, (void *)&hid_buf_keyboard[0], &dev->hid_report_len);
    //First, set the default LEDs Light
    usb_set_report(ghid_dev_keyboard, 0, USB_REQ_REPORT_TYPE_OUTPUT, 0, (void *)&usb_keyboard_leds_status, 1);

    DEBUG_UHOST("usb_keyboard_hid_format: %d, %d, %d, %d, %d\n"
                , usb_keyboard_hid_format[1], usb_keyboard_hid_format[3], usb_keyboard_hid_format[5]
                , usb_keyboard_hid_format[7], usb_keyboard_hid_format[9]);
    //for(ret = 0; ret < 0xE7; ret++)	printf("0x%02x\t%s\t\t%s\n", HID_KEYBOARD_KEYPAD_PAGE_VALUE[ret], HID_KEYBOARD_KEYPAD_PAGE_SCHAR[ret], HID_KEYBOARD_KEYPAD_PAGE_BCHAR[ret]);

    hid_qh_init(dev, ep, keyboard_qh_addr, keyboard_qtd, 0x1);
    ehci_mdelay(50);
    keyboard_running = HID_POLL_STATUS;
    return ret;
}

void usb_set_hid_format(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *usb_dev_hid_format, u8 *buf, int *length)
{
    struct hid_item item;
    u8 i, index = 0;
    for(i = 0; i < *length; )
    {
        /* check hid item format */
        item.tag = buf[i] >> 4 & 0xf;
        item.type = buf[i] >> 2 & 0x3;
        //long item
        if(item.tag == HID_ITEM_TAG_LONG)
        {
            item.format = HID_ITEM_FORMAT_LONG;
            item.size = buf[i+1];
            if((*length - i) < 2 || (*length - i) < item.size)
            {
                i += item.size;
                continue;
            }
        }
        //short item
        item.format = HID_ITEM_FORMAT_SHORT;
        item.size = buf[i] & 0x3;
        if(usb_hid_report_descriptor_handler(&hid_item_inlist[index], buf, i, &item) == -1)
        {
            s8 j;
            //avoid type, bReportSize or bReportCount = 0, it will fetch last one type, bReportSize or bReportCount would not equals 0
            if(!hid_item_inlist[index].type && hid_item_inlist[index].type != CONSTANT)
            {
                for(j = index-1; j >= 0; j--)
                {
                    //avoid CONSTANT
                    if(hid_item_inlist[j].type && hid_item_inlist[j].type != CONSTANT)
                    {
                        hid_item_inlist[index].type = hid_item_inlist[j].type;
                        break;
                    }
                }
            }
            if(!hid_item_inlist[index].bReportSize)
            {
                for(j = index-1; j >= 0; j--)
                {
                    if(hid_item_inlist[j].bReportSize)
                    {
                        hid_item_inlist[index].bReportSize = hid_item_inlist[j].bReportSize;
                        break;
                    }
                }
            }
            if(!hid_item_inlist[index].bReportCount)
            {
                for(j = index-1; j >= 0; j--)
                {
                    if(hid_item_inlist[j].bReportCount)
                    {
                        hid_item_inlist[index].bReportCount = hid_item_inlist[j].bReportCount;
                        break;
                    }
                }
            }
            index++;
        }
        i += item.size + 1;
    }
    usb_hid_bit_format(hid_item_inlist, usb_dev_hid_format);
}

/* simple analyzer */
int usb_hid_report_descriptor_handler(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *buffer, int bufindex, struct hid_item *item)
{
    s8 result = 0;
    u8 dataleng = item->size == 3? 4:item->size;
    s32 tmp;
    switch(item->type)
    {
        /* main */
        case HID_ITEM_TYPE_MAIN:
            switch(item->tag)
            {
                case HID_MAIN_ITEM_INPUT:
                    hid_item_inlist->bMainItemType = HID_MAIN_ITEM_INPUT;
                case HID_MAIN_ITEM_OUTPUT:
                    if(hid_item_inlist->bMainItemType == 0)
                        hid_item_inlist->bMainItemType = HID_MAIN_ITEM_OUTPUT;
                    result = -1;
                    if(buffer[bufindex+dataleng] & 0x1)
                        hid_item_inlist->type = CONSTANT;
                    break;
                case HID_MAIN_ITEM_COLLECTION:
                    hid_item_inlist->bMainItemType = HID_MAIN_ITEM_COLLECTION;
                    break;
                case HID_MAIN_ITEM_FEATURE:
                    hid_item_inlist->bMainItemType = HID_MAIN_ITEM_FEATURE;
                    break;
                case HID_MAIN_ITEM_END_COLLECTION:
                    hid_item_inlist->bMainItemType = HID_MAIN_ITEM_END_COLLECTION;
                    break;
                default:
                    break;
            }
            break;

        /* global */
        case HID_ITEM_TYPE_GLOBAL:
            switch(item->tag)
            {
                case HID_GLOBAL_ITEM_REPORT_ID:
                    hid_item_inlist->type = REPORTID;
                    hid_item_inlist->bMainItemType = HID_MAIN_ITEM_INPUT;
                    hid_item_inlist->bReportID = buffer[bufindex+1];
                    hid_item_inlist->bReportSize = 0x8;
                    hid_item_inlist->bReportCount = 0x1;
                    result = -1;
                    break;
                case HID_GLOBAL_ITEM_USAGE_PAGE:
                    if(buffer[bufindex+dataleng] == HID_USAGE_PAGE_BUTTON)
                        hid_item_inlist->type = BUTTON;
                    else if(buffer[bufindex+dataleng] == HID_USAGE_PAGE_KEYBOARD_KEYPAD)
                        hid_item_inlist->type = KEYCODE;
                    else if(buffer[bufindex+dataleng] == HID_USAGE_PAGE_LEDS)
                        hid_item_inlist->type = LED;
                    break;
                case HID_GLOBAL_ITEM_LOGICAL_MINIMUM:
                    if((s8)buffer[bufindex+dataleng] < 0)
                        memset(&tmp, 0xff, sizeof(tmp));
                    memcpy(&tmp, &buffer[bufindex+1], sizeof(buffer[bufindex])*dataleng);
                    hid_item_inlist->bLogicalMinimum = tmp;
                    break;
                case HID_GLOBAL_ITEM_LOGICAL_MAXIMUM:
                    if((s8)buffer[bufindex+dataleng] < 0)
                        memset(&tmp, 0xff, sizeof(tmp));
                    memcpy(&tmp, &buffer[bufindex+1], sizeof(buffer[bufindex])*dataleng);
                    hid_item_inlist->bLogicalMaximum = tmp;
                    break;
                case HID_GLOBAL_ITEM_REPORT_SIZE:   //一般mouse, keyboard很少用超過u8的長度，所以這裡只算+1
                    hid_item_inlist->bReportSize = buffer[bufindex+1];
                    break;
                case HID_GLOBAL_ITEM_REPORT_COUNT:
                    hid_item_inlist->bReportCount = buffer[bufindex+1];
                    break;
                default:
                    break;
            }
            break;

        /* local */
        case HID_ITEM_TYPE_LOCAL:
            switch(item->tag)
            {
                case HID_LOCAL_ITEM_USAGE:
                    //因為是簡單的滑鼠配置，所以這裡先不管有沒有確認是不是在GD的Collection底下。
                    if(buffer[bufindex+dataleng] == HID_GENERIC_DESKTOP_PAGE_X || buffer[bufindex+dataleng] == HID_GENERIC_DESKTOP_PAGE_Y)
                        hid_item_inlist->type = COORDINATE;
                    if(buffer[bufindex+dataleng] == HID_GENERIC_DESKTOP_PAGE_WHEEL)
                        hid_item_inlist->type = WHEEL;
                    break;
                case HID_LOCAL_ITEM_USAGE_MINIMUM:
                    if((s8)buffer[bufindex+dataleng] < 0)
                        memset(&tmp, 0xff, sizeof(tmp));
                    memcpy(&tmp, &buffer[bufindex+1], sizeof(buffer[bufindex])*dataleng);
                    hid_item_inlist->bUsageMinimum = tmp;
                    break;
                case HID_LOCAL_ITEM_USAGE_MAXIMUM:
                    if((s8)buffer[bufindex+dataleng] < 0)
                        memset(&tmp, 0xff, sizeof(tmp));
                    memcpy(&tmp, &buffer[bufindex+1], sizeof(buffer[bufindex])*dataleng);
                    hid_item_inlist->bUsageMaximum = tmp;
                    break;
                default:
                    /* consumer */
                    if((s8)buffer[bufindex+dataleng] < 0)
                        memset(&tmp, 0xff, sizeof(tmp));
                    memcpy(&tmp, &buffer[bufindex+1], sizeof(buffer[bufindex])*dataleng);
                    if(tmp == HID_CONSUMER_PAGE_AC_PAN)
                        hid_item_inlist->type = AC_PAN;
                    break;
            }
            break;
        default:
            break;
    }
    return result;
}


void usb_hid_bit_format(struct usb_hid_simple_rd_item *hid_item_inlist, u8 *usb_dev_hid_format)
{
    u8 i;
    for(i = 0; i < HID_INFO_COLLECT_SIZE; i++)
    {
        DEBUG_UHOST("dev->bReportSize: %d, bReportCount: %d\n", hid_item_inlist[i].bReportSize, hid_item_inlist[i].bReportCount);
        usb_dev_hid_format[i*2+1] = hid_item_inlist[i].bReportSize * hid_item_inlist[i].bReportCount;
        switch(hid_item_inlist[i].type)
        {
            case BUTTON:
                usb_dev_hid_format[i*2] = BUTTON;
                break;
            case CONSTANT:
                usb_dev_hid_format[i*2] = CONSTANT;
                break;
            case COORDINATE:
                if(hid_item_inlist[i].bReportCount == 0x03)  //Wheel, X, Y together
                {
                    usb_dev_hid_format[i*2] = WHEEL;
                    usb_dev_hid_format[i*2+1] = hid_item_inlist[i].bReportSize*1;
                    usb_dev_hid_format[i*2] |= (hid_item_inlist[i].bMainItemType << 6);
                    ++i;
                    usb_dev_hid_format[i*2] = COORDINATE;
                    usb_dev_hid_format[i*2+1] = hid_item_inlist[i].bReportSize*2;
                }
                else
                    usb_dev_hid_format[i*2] = COORDINATE;
                break;
            case WHEEL:
                if(hid_item_inlist[i].bReportCount == 0x03)  //X, Y, Wheel together
                {
                    usb_dev_hid_format[i*2] = COORDINATE;
                    usb_dev_hid_format[i*2+1] = hid_item_inlist[i].bReportSize*2;
                    usb_dev_hid_format[i*2] |= (hid_item_inlist[i].bMainItemType << 6);
                    ++i;
                    usb_dev_hid_format[i*2] = WHEEL;
                    usb_dev_hid_format[i*2+1] = hid_item_inlist[i].bReportSize*1;
                }
                else
                    usb_dev_hid_format[i*2] = WHEEL;
                break;
            case KEYCODE:
                if(usb_dev_hid_format[i*2+1] == 8) //兩個都是使用keyboard/keypad，所以這裡使用數量來判別隔開
                    usb_dev_hid_format[i*2] = CONTROLKEYCODE;
                else
                    usb_dev_hid_format[i*2] = KEYCODE;
                break;
            case LED:
                usb_dev_hid_format[i*2] = LED;
                break;
            case REPORTID:
                usb_dev_hid_format[i*2] = REPORTID;
                break;
            default:
                usb_dev_hid_format[i*2] = 0;
                usb_dev_hid_format[i*2+1] = 0;
                break;
        }
        usb_dev_hid_format[i*2] |= (hid_item_inlist[i].bMainItemType << 6);
    }
}

/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
int usb_new_device(struct usb_device *dev)
{
    int addr, err;
    int tmp;
    __align(4) unsigned char tmpbuf[USB_BUFSIZ];

    struct usb_device_descriptor *desc;
    struct usb_device *parent = dev->parent;

    u16 portstatus;
    u32 cmd;


    cmd = ehci_readl(&hcor->or_usbcmd);
    /*
     * Philips, Intel, and maybe others need USBCMD_RUN before the
     * root hub will detect new devices (why?); NEC doesn't
     */
    cmd &= ~(USBCMD_LRESET|USBCMD_IAAD|USBCMD_ASE|USBCMD_RESET);
    cmd |= USBCMD_RUN;
    //ehci_writel(&hcor->or_usbcmd, cmd);
    hcor->or_usbcmd = cmd;

    /* We still haven't set the Address yet */
    DEBUG_UHOST("dev->devnum value: %d\n", dev->devnum);
    addr = dev->devnum;
    dev->devnum = 0;

    /* This is a Windows scheme of initialization sequence, with double
     * reset of the device (Linux uses the same sequence)
     * Some equipment is said to work only with such init sequence; this
     * patch is based on the work by Alan Stern:
     * http://sourceforge.net/mailarchive/forum.php?
     * thread_id=5729457&forum_id=5398
     */



    /* send 64-byte GET-DEVICE-DESCRIPTOR request.  Since the descriptor is
     * only 18 bytes long, this will terminate with a short packet.  But if
     * the maxpacket size is 8 or 16 the device may be waiting to transmit
     * some more, or keeps on retransmitting the 8 byte header. */

    //tmpbuf 使用記憶體位置進行多結構類型的指派，一對多
    desc = (struct usb_device_descriptor *)tmpbuf;
    dev->descriptor.bMaxPacketSize0 = 64;	    /* Start off at 64 bytes  */
    /* Default to 64 byte max packet size */
    dev->maxpacketsize = PACKET_SIZE_64;
    dev->epmaxpacketin[0] = 64;
    dev->epmaxpacketout[0] = 64;

    err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, 64);
    if (err < 0)
    {
        DEBUG_STORAGE("usb_new_device: usb_get_descriptor() failed\n");
        return 1;
    }

    dev->descriptor.bMaxPacketSize0 = desc->bMaxPacketSize0;

    /* find the port number we're at */
    if (parent)
    {
        int port = -1;
        port = port; // avoid warming msg.
        /*int j;
        for (j = 0; j < parent->maxchild; j++)
        {
            if (parent->children[j] == dev)
            {
                port = j;
                break;
            }
        }

        if (port < 0)
        {
            DEBUG_UHOST("usb_new_device:cannot locate device's port.\n");
            return 1;
        }*/

        /* reset the port for the second time */
        err = hub_port_reset(dev->parent, dev->portnr, &portstatus);
        if (err < 0)
        {
            DEBUG_STORAGE("\n  Couldn't reset port %i\n", port);
            return 1;
        }
    }


    dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
    dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;
    DEBUG_UHOST("dev->descriptor.bMaxPacketSize0 value: %d\n", dev->descriptor.bMaxPacketSize0);
    switch (dev->descriptor.bMaxPacketSize0)
    {
        case 8:
            dev->maxpacketsize  = PACKET_SIZE_8;
            break;
        case 16:
            dev->maxpacketsize = PACKET_SIZE_16;
            break;
        case 32:
            dev->maxpacketsize = PACKET_SIZE_32;
            break;
        case 64:
            dev->maxpacketsize = PACKET_SIZE_64;
            break;
    }
    //把原先借放於addr的值放回來
    DEBUG_UHOST("dev->devnum value: %d\n", dev->devnum);
    dev->devnum = addr;

    err = usb_set_address(dev); /* set address */

    if (err < 0)
    {
        DEBUG_STORAGE("\nUSB device not accepting new address (error=%lX)\n", dev->status);
        return 1;
    }

    ehci_mdelay(10);	/* Let the SET_ADDRESS settle */

    tmp = sizeof(dev->descriptor);

    err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, &dev->descriptor, sizeof(dev->descriptor));
    if (err < tmp)
    {
        if (err < 0)
            DEBUG_STORAGE("unable to get device descriptor (error=%d)\n", err);
        else
            DEBUG_STORAGE("USB device descriptor short read (expected %i, got %i)\n", tmp, err);
        return 1;
    }
    DEBUG_UHOST("**********************************\n"\
                "dev->descriptor.[attribute]: value\n"\
                "**********************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "bcdUSB: 0x%04x\n"\
                "bDeviceClass: 0x%02x\n"\
                "bDeviceSubClass: 0x%02x\n"\
                "bDeviceProtocol: 0x%02x\n"\
                "bMaxPacketSize0: 0x%02x\n"\
                "idVendor: 0x%04x\n"\
                "idProduct: 0x%04x\n"\
                "bcdDevice: 0x%04x\n"\
                "iManufacturer: 0x%02x\n"\
                "iProduct: 0x%02x\n"\
                "iSerialNumber: 0x%02x\n"\
                "bNumConfigurations: 0x%02x\n"\
                "**********************************\n"
                , dev->descriptor.bLength
                , dev->descriptor.bDescriptorType
                , dev->descriptor.bcdUSB
                , dev->descriptor.bDeviceClass
                , dev->descriptor.bDeviceSubClass
                , dev->descriptor.bDeviceProtocol
                , dev->descriptor.bMaxPacketSize0
                , dev->descriptor.idVendor
                , dev->descriptor.idProduct
                , dev->descriptor.bcdDevice
                , dev->descriptor.iManufacturer
                , dev->descriptor.iProduct
                , dev->descriptor.iSerialNumber
                , dev->descriptor.bNumConfigurations);

    /* correct le values */
#if USB_UNDO
    le16_to_cpus(&dev->descriptor.bcdUSB);
    le16_to_cpus(&dev->descriptor.idVendor);
    le16_to_cpus(&dev->descriptor.idProduct);
    le16_to_cpus(&dev->descriptor.bcdDevice);
#endif
    /* only support for one config for now */

    usb_get_configuration_no(dev, tmpbuf, 0);

    usb_parse_config(dev, tmpbuf, 0);
    //用來判斷該EP是用在control、in、out哪一種
    usb_set_maxpacket(dev);
    /* we set the default configuration here */
    if (usb_set_configuration(dev, dev->config.desc.bConfigurationValue))
    {
        DEBUG_STORAGE("failed to set default configuration len %d, status %lX\n", dev->act_len, dev->status);
        return -1;
    }
    DEBUG_UHOST("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
                dev->descriptor.iManufacturer, dev->descriptor.iProduct, dev->descriptor.iSerialNumber);
    memset(dev->mf, 0, sizeof(dev->mf));
    memset(dev->prod, 0, sizeof(dev->prod));
    memset(dev->serial, 0, sizeof(dev->serial));

    if (dev->descriptor.iManufacturer)
        usb_string(dev, dev->descriptor.iManufacturer, dev->mf, sizeof(dev->mf));
    if (dev->descriptor.iProduct)
        usb_string(dev, dev->descriptor.iProduct, dev->prod, sizeof(dev->prod));
    if (dev->descriptor.iSerialNumber)
        usb_string(dev, dev->descriptor.iSerialNumber, dev->serial, sizeof(dev->serial));
    DEBUG_UHOST("Manufacturer %s\n", dev->mf);
    DEBUG_UHOST("Product      %s\n", dev->prod);
    DEBUG_UHOST("SerialNumber %s\n", dev->serial);
    /* now prode if the device is a hub */
    //把usb_parse_config抓到的數值進行檢查
    usb_hub_probe(dev, 0);
    usb_hid_probe(dev, 0);
    return 0;
}

/* build device Tree  */
static void usb_scan_devices(void)
{
    int i;
    struct usb_device *dev;

    /* first make all devices unknown */
    for (i = 0; i < USB_MAX_DEVICE; i++)
    {
        memset(&usb_dev[i], 0, sizeof(struct usb_device));
        usb_dev[i].devnum = -1;
    }
    /* device 0 is always present (root hub, so let it analyze) */
    dev = usb_alloc_new_device();
    if (usb_new_device(dev))
        DEBUG_STORAGE("No USB Device found\n");
    else
        DEBUG_STORAGE("%d USB Device(s) found\n", udev_index);
    /* insert "driver" if possible */
#ifdef USB_UNDO
    drv_usb_kbd_init();
#endif
    DEBUG_UHOST("scan end\n");
}

s32 usbHostSetIntEvt(u8 cause)
{
    /* check if cause is valid */
    if (cause >= USB_HOST_DEV_EVT_UNDEF)
    {
        /* cause out of range */
        DEBUG_UHUB("# SetIntEvt OVER\n");
        return 0;
    }

    /* set the cause */
    usbHostIntEvt.cause[usbHostIntEvt.idxSet++] = cause;
    DEBUG_UHOST("# SetIntEvt %d index %d\n", cause,usbHostIntEvt.idxSet-1);
    if (usbHostIntEvt.idxSet == USB_HOST_MAX_INT_EVT)
    {
        /* wrap around the index */
        usbHostIntEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbHostIntEvt.idxSet == usbHostIntEvt.idxGet)
    {
        DEBUG_UHUB("# SetIntEvt FULL\n");
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    DEBUG_UHUB("# SetIntEvt %d\n", cause);
	
    OSSemPost(usbHostSemEvt);

    return 1;
}

s32 usbHostGetIntEvt(u8* pCause)
{
    /* check if event queue is empty */
    if (usbHostIntEvt.idxGet == usbHostIntEvt.idxSet)
    {
        /* event queue is empty */
        DEBUG_UHUB("# empty\n");
        return 0;
    }
  //  while(SCSI_ON)
  //  {
 //      DEBUG_UHUB("#");
 //       OSTimeDly(1);
 //    }
    /* get the cause */
    *pCause = usbHostIntEvt.cause[usbHostIntEvt.idxGet++];
    DEBUG_UHOST("# GetIntEvt %d index %d\n", *pCause,usbHostIntEvt.idxGet-1);
    if (usbHostIntEvt.idxGet == USB_HOST_MAX_INT_EVT)
    {
        /* wrap around the index */
        usbHostIntEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= USB_HOST_DEV_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    return 1;
}

void usb_scan(void)
{
	int ret;
	
	usb_init_flag=1;
	
#if USB_HOST_MASS_SUPPORT
#if MASS_STORAGE_INSERT_SHOW	
	sysSentUiKeyTilOK(UI_KEY_USB_INSERT);	// for USB prepare icon
#endif
#endif
	ret = usb_init();
	usb_init_flag=0;

#if USB_HOST_MASS_SUPPORT
	if(ret >= 0)
	{
#if SD_TASK_INSTALL_FLOW_SUPPORT
		usbHostSetIntEvt(USB_HOST_DEV_EVT_MOUNT);
#else
		sysUSBCD_IN(0);
#endif
	}
#endif
}

void usb_hub_connect_change(void)
{
    /*Need to configure Hub port*/
    char port = hub_buffer[0];
    int i;
    for (i = 1; i < 4; i++)
    {
        if((port >> i) == 1)
            break;
    }
    ghub_port=i;
    hub_event_handle(i);
}


void usb_dev_mount_seq(void)
{
#if SD_TASK_INSTALL_FLOW_SUPPORT
    if(!usb_hdd_removed)
        sysSentMountSeq(SYS_V_STORAGE_USBMASS);
#endif       
}

void usb_dev_unmount_seq(void)
{
#if SD_TASK_INSTALL_FLOW_SUPPORT
	// Maybe call from int handler
	if(HCPortSC & PORTSC_CS)
	{
		DEBUG_STORAGE("UHOST W: execute unmout seq.\n");
		usb_hdd_removed = 1;
	}
	// if usb_hdd_removed flag has raised, it means the unmount progress doesn't need to exexcute.
	//if(!usb_hdd_removed)
		sysSentMountSeq(SYS_V_STORAGE_USBMASS);
#endif
}

void usb_dev_mass_scan_free(void)
{
	sysGetDiskFree(0);
}

void usb_dev_mass_scan_free_extend(void)
{
	usbHostSetIntEvt(USB_HOST_DEV_EVT_MASS_FREE_SCAN);
}

void usb_end()
{
    int i=0;

    i++;
}

void (*usbHostIntEvtFunc[])(void) =
{
    usb_scan,					// 0x00  -Start First scan
    usb_hub_connect_change,
    usb_dev_mount_seq,
    usb_dev_unmount_seq,
    usb_dev_mass_scan_free,
    usb_end
};

void usbHostTask(void* pData)
{
    u8 err;
    u8 cause;

    while (1)
    {
        if (usbHostGetIntEvt(&cause))
        {
            DEBUG_UHUB("# cause Evt %d\n", cause);
            (*usbHostIntEvtFunc[cause])();
        }
        /*
        else if (usbHostGetApiEvt(&usbApiCurEvt))
        {
            (*usbApiEvtFunc[usbApiCurEvt.cause])();
        }
        */
        else
        {
            OSSemPend(usbHostSemEvt, OS_IPC_WAIT_FOREVER, &err);
            //DEBUG_UHOST("# usbHostSemEvt\n");
            if (err != OS_NO_ERR)
            {
                DEBUG_USB("Error: usbSemEvt is %d.\n", err);
                return ;
            }
        }
    }
}

int poll_hid(void)
{
    struct qTD *init_qtd;
    struct QH  *init_qh ;
    u32 token;

    if(mouse_running == HID_POLL_STATUS)
    {
        init_qtd =(struct qTD *) mouse_qtd ;
        init_qh =(struct QH *) mouse_qh_addr ;
        token = init_qtd->qtd_token;

        if ((token & USB_QTD_TOKEN_STAT_ACTIVE) == 0)
        {
#if USB_HOST_DVR
            u8 mouseKey=0;
            s16 mouseMoveX=0;
            s16 mouseMoveY=0;
            s8 mouseWheel=0;
            unsigned long long qTD_buf = 0;
            u8 i, times;
            s8 index = 0;
            s16 tmp = 0;
#if 0
            for(i = 0; i < mouse_packet_size; i++)
            {
                if(!i) printf("HID mouse qTD:");
                printf("%02x ", hid_buf_mouse[i]);
                if(i+1 == mouse_packet_size) printf("\n");
            }
#endif
            for(index = mouse_packet_size - 1; index >= 0; index--)
            {
                memcpy(&qTD_buf, &hid_buf_mouse[index], sizeof(hid_buf_mouse[index]));
                if(index > 0)
                    qTD_buf <<= 8;
            }
            //separate the qTD raw data
            for(i = 0, index = 0; i < HID_INFO_COLLECT_SIZE; i++)
            {
                // only handle Input, detail from struct usb_hid_simple_rd_item
                if((usb_mouse_hid_format[i*2] >> 6) && (HID_MAIN_ITEM_OUTPUT & 0x3))
                    continue;
                switch(usb_mouse_hid_format[i*2] & 0x3f)
                {
                    case COORDINATE:
                        times = 2;	// it contain X、Y axis。
                        break;
                    default:
                        times = 1;
                        break;
                }
                tmp = qTD_buf >> index;
                switch(usb_mouse_hid_format[i*2] & 0x3f)
                {
                    case BUTTON:
                        tmp <<= (16 - usb_mouse_hid_format[i*2+1] / times);
                        tmp >>= (16 - usb_mouse_hid_format[i*2+1] / times);
                        index += usb_mouse_hid_format[i*2+1] / times;
                        memcpy(&mouseKey, &tmp, sizeof(mouseKey));
                        break;
                    case COORDINATE:
                        tmp <<= (16 - usb_mouse_hid_format[i*2+1] / times);
                        tmp >>= (16 - usb_mouse_hid_format[i*2+1] / times);
                        index += usb_mouse_hid_format[i*2+1] / times;
                        memcpy(&mouseMoveX, &tmp, sizeof(mouseMoveX));
                        tmp = qTD_buf >> index;
                        tmp <<= (16 - usb_mouse_hid_format[i*2+1] / times);
                        tmp >>= (16 - usb_mouse_hid_format[i*2+1] / times);
                        index += usb_mouse_hid_format[i*2+1] / times;
                        memcpy(&mouseMoveY, &tmp, sizeof(mouseMoveY));
                        break;
                    case WHEEL:
                        tmp <<= (16 - usb_mouse_hid_format[i*2+1] / times);
                        tmp >>= (16 - usb_mouse_hid_format[i*2+1] / times);
                        index += usb_mouse_hid_format[i*2+1] / times;
                        memcpy(&mouseWheel, &tmp, sizeof(mouseWheel));
                        break;
                    case REPORTID:
                    default:
                        index += usb_mouse_hid_format[i*2+1] / times;
                        break;
                }
            }
            Mouse_update(mouseMoveX, mouseMoveY, mouseKey);
            //DEBUG_UHOST("Mouse Move X=%d Y=%d Wheel=%d\n", mouseMoveX, mouseMoveY, mouseWheel);
#endif
            init_qtd->qtd_token |= (USB_QTD_TOKEN_TRANS_SIZE(mouse_packet_size) | USB_QTD_TOKEN_STAT_ACTIVE);
            init_qtd->qtd_buffer[0] = (u32) &hid_buf_mouse[0];
            init_qh->qh_overlay.qtd_next = (u32) mouse_qtd;
            //format_printf2(0,sizeof(struct QH)/4,(int)mouse_qh_addr);
            //DEBUG_UHOST("---------------------------\n");
            //format_printf2(0,sizeof(struct qTD)/4,(int)mouse_qtd);

        }
    }

    if(keyboard_running == HID_POLL_STATUS)
    {
        init_qtd =(struct qTD *) keyboard_qtd ;
        init_qh =(struct QH *) keyboard_qh_addr ;
        token = init_qtd->qtd_token;

        if ((token & USB_QTD_TOKEN_STAT_ACTIVE) == 0)
        {
#if USB_HOST_DVR
            unsigned long long qTD_buf = 0;
            u8 controlKey, led_light;
            u8 i, j, tmp;
            s8 index = 0;
#if 0
            for(i = 0; i < keyboard_packet_size; i++)
            {
                if(!i)	printf("HID keyboard qTD:");
                printf("%02x ", hid_buf_keyboard[i]);
                if(i+1 == keyboard_packet_size) printf("\n");
            }
#endif
            for(index = keyboard_packet_size - 1; index >= 0; index--)
            {
                memcpy(&qTD_buf, &hid_buf_keyboard[index], sizeof(hid_buf_keyboard[index]));
                if(index > 0)
                    qTD_buf <<= 8;
            }

            //separate the qTD raw data
            for(i = 0, index = 0; i < HID_INFO_COLLECT_SIZE; i++)
            {
                if((usb_keyboard_hid_format[i*2] >> 6) && (HID_MAIN_ITEM_OUTPUT & 0x3))
                    continue;
                tmp = qTD_buf >> index;
                switch(usb_keyboard_hid_format[i*2] & 0x3f)
                {
                    case CONTROLKEYCODE:
                        controlKey = tmp;
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_LEFTCONTROL & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_LEFTCONTROL]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_LEFTSHIFT & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_LEFTSHIFT]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_LEFTALT & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_LEFTALT]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_LEFTGUI & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_LEFTGUI]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_RIGHTCONTROL & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_RIGHTCONTROL]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_RIGHTSHIFT & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_RIGHTSHIFT]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_RIGHTALT & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_RIGHTALT]);
                        if(tmp & (1 << (HID_KEYBOARD_KEYPAD_PAGE_RIGHTGUI & 0xf)))
                            printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[HID_KEYBOARD_KEYPAD_PAGE_RIGHTGUI]);
                        break;
                    case KEYCODE:
                        for(j = 0; tmp != 0 && j < hid_item_list[ghid_dev_keyboard->devnum-1][i].bReportCount; j++)
                        {
                            switch(tmp)
                            {
                                case HID_KEYBOARD_KEYPAD_PAGE_NEI:
                                case HID_KEYBOARD_KEYPAD_PAGE_ERRORROLLOVER:
                                case HID_KEYBOARD_KEYPAD_PAGE_POSTFAIL:
                                case HID_KEYBOARD_KEYPAD_PAGE_ERRORUNDEFINED:
                                    tmp = 0;
                                    break;
                                case HID_KEYBOARD_KEYPAD_PAGE_NUMKOCK:
                                    led_light = HID_LED_PAGE_NUMLOCK;
                                case HID_KEYBOARD_KEYPAD_PAGE_CAPSLOCK:
                                    if(!led_light)
                                        led_light = HID_LED_PAGE_CAPSLOCK;
                                case HID_KEYBOARD_KEYPAD_PAGE_SCROLLLOCK:
                                    //check Version to filter the double shoot
                                    if(ghid_dev_keyboard->descriptor.bcdUSB & 0x110)
                                    {
                                        usb_keyboard_hid_leds_switch = (++usb_keyboard_hid_leds_switch)%2;
                                        if(usb_keyboard_hid_leds_switch)
                                            break;
                                    }
                                    if(!led_light)
                                        led_light = HID_LED_PAGE_SCROLLLOCK;
                                    //Adjust LEDs Status
                                    usb_keyboard_leds_status ^= (1 << (led_light-1));
                                    printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[tmp]);
                                    break;
                                default:
                                {
                                    u8 charCase = 0;    //0: Small case, 1: Big Case
                                    if((((HID_KEYBOARD_KEYPAD_PAGE_LEFTSHIFT+1) & 0xf) & controlKey)
                                            || (((HID_KEYBOARD_KEYPAD_PAGE_RIGHTSHIFT+1) & 0xf) & controlKey))
                                        charCase ^= 1;
                                    if(HID_LED_PAGE_CAPSLOCK & usb_keyboard_leds_status)
                                        charCase ^= 1;
                                    if(charCase)
                                        printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_BCHAR[tmp]);
                                    else
                                        printf("keyboard keycode: %s\n", HID_KEYBOARD_KEYPAD_PAGE_SCHAR[tmp]);
                                }
                                break;
                            }
                            index += hid_item_list[ghid_dev_keyboard->devnum-1][i].bReportSize;
                            tmp = qTD_buf >> index;
                        }
                        break;
                    default:
                        break;
                }
                index += usb_keyboard_hid_format[i*2+1];
            }

            if(led_light && !usb_keyboard_hid_leds_switch)
                usb_set_report(ghid_dev_keyboard, 0, USB_REQ_REPORT_TYPE_OUTPUT, 0, (void *)&usb_keyboard_leds_status, 1);
#endif
            init_qtd->qtd_token |= (USB_QTD_TOKEN_TRANS_SIZE(keyboard_packet_size) | USB_QTD_TOKEN_STAT_ACTIVE);
            init_qtd->qtd_buffer[0] = (u32) &hid_buf_keyboard[0];
            init_qh->qh_overlay.qtd_next = (u32) keyboard_qtd;
            //format_printf2(0,sizeof(struct QH)/4,(int)keyboard_qh_addr);
            //DEBUG_UHOST("---------------------------\n");
            //format_printf2(0,sizeof(struct qTD)/4,(int)keyboard_qtd);
        }
    }
    return 0;
}


int poll_hub(void)
{
    struct qTD *init_qtd;
    struct QH  *init_qh ;
    u32 token;

    if(hub_mode==HUB_POLL_STATUS)
    {
        init_qtd =(struct qTD *) hub_qtd ;
        init_qh =(struct QH *) hub_qh_addr ;
        token = init_qtd->qtd_token;

        if ((token & USB_QTD_TOKEN_STAT_ACTIVE) == 0)
        {

            printf("Poll Hub data: %x\n",hub_buffer[0]);
            usbHostSetIntEvt(USB_HOST_INT_EVT_HUB_CONNECT_CHANGE);
            init_qtd->qtd_token |= (USB_QTD_TOKEN_TRANS_SIZE(1) | USB_QTD_TOKEN_STAT_ACTIVE );
            init_qtd->qtd_buffer[0] = (u32) &hub_buffer[0];
            init_qh->qh_overlay.qtd_next = (u32) hub_qtd;
            format_printf2(0,sizeof(struct QH)/4,(int)hub_qh_addr);
            printf("---------------------------\n");
            format_printf2(0,sizeof(struct qTD)/4,(int)hub_qtd);
            return 0;
        }
    }
    return 1;
}


int check_irq(void)
{
    //struct qTD *init_qtd;
    //struct QH  *init_qh ;
    //u32 token;
    //DEBUG_UHOST("check_irq");
    /*if(0)  //if(hub_running)
    {
        init_qtd =(struct qTD *) hub_qtd ;
        init_qh =(struct QH *) hub_qh_addr ;
        token = init_qtd->qtd_token;

        if ((token & USB_QTD_TOKEN_STAT_ACTIVE) == 0)
        {
            if (gEhci_periodic_status)
            {
                DEBUG_UHOST("Error: gEhci_periodic_status is %d.\n", gEhci_periodic_status);
                gEhci_periodic_status = 0;
                //  DEBUG_UHOST("TOKEN=%#x\n",hc32_to_cpu(init_qh->qh_overlay.qtd_token));
            }

            DEBUG_UHOST("Hub data: %x\n",hub_buffer[0]);
            usbHostSetIntEvt(USB_HOST_INT_EVT_HUB_CONNECT_CHANGE);
            init_qtd->qtd_token |= (USB_QTD_TOKEN_TRANS_SIZE(1) | USB_QTD_TOKEN_STAT_ACTIVE );
            init_qtd->qtd_buffer[0] = (u32) &hub_buffer[0];
            init_qh->qh_overlay.qtd_next = (u32) hub_qtd;

            format_printf2(0,sizeof(struct QH)/4,(int)hub_qh_addr);
            DEBUG_UHOST("---------------------------\n");
            format_printf2(0,sizeof(struct qTD)/4,(int)hub_qtd);
            return 1;
        }
    }

    //if(mouse_running)
    if(0)
    {
        init_qtd =(struct qTD *) mouse_qtd ;
        init_qh =(struct QH *) mouse_qh_addr ;
        token = init_qtd->qtd_token;

        if ((token & USB_QTD_TOKEN_STAT_ACTIVE) == 0)
        {
            if (gEhci_periodic_status)
            {
                DEBUG_UHOST("Error: gEhci_periodic_status is %d.\n", gEhci_periodic_status);
                gEhci_periodic_status = 0;
                // DEBUG_UHOST("TOKEN=%#x\n",hc32_to_cpu(init_qh->qh_overlay.qtd_token));
            }
            //printf("HID data 2: %02x %02x %02x %02x %02x %02x\n",hid_buf_mouse[0],hid_buf_mouse[1],hid_buf_mouse[2],hid_buf_mouse[3],hid_buf_mouse[4],hid_buf_mouse[5]);
        }
    }*/
    return 1;
}

//usbHostIntEvtFunc
u32 pre_port_status;


void usbHostIntHandler(void)
{
	u32 port_status;
	u32 ehci_status;
	int async_flag=0;

	port_status = HCPortSC;
	ehci_status = HCUSBSTS;
	//DEBUG_UHOST("Port Status: %x\n",port_status);
	//DEBUG_UHOST("ehci_status : %x\n",ehci_status);

	if(port_status & PORTSC_CSC ||ehci_status & USBSTS_PCD )
	{
		/*Port Change Detecct*/
		//	HCPortSC= (PORTSC_CSC|PORTSC_PEC);
		HCUSBINTR&=~(USBINTR_PCE);
		//	HCUSBSTS = USBSTS_PCD;
		DEBUG_STORAGE(" -----------USB Port Change Detect----------\n");
#if USB_HOST_DVR
		if(bus_reset == 0)	/*First enumeration*/
			usbHostSetIntEvt(USB_HOST_INT_EVT_SCAN);
#else
		DEBUG_UHOST("gUSBDevOn = %d, Port Status: %x\n", gUSBDevOn, port_status);
		if(port_status & PORTSC_CS)
		{
			if(gUSBDevOn == 0)
			{
				DEBUG_STORAGE(" -----------USB_gInsertCard----------\n");
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
#if AUTO_STORAGE_CHANGE
				// Call directly instead of put the mission into system queue
				if(sysCheckSDCD() == SDC_CD_IN)
					sysSDCD_OFF(0);
				sysKeepSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_USBMASS, SYS_I_STORAGE_BACKUP);
#endif
#endif
				ehci_Atomic_SemEvt->OSEventCnt = 1;
				ehci_INT_SemEvt->OSEventCnt = 0;
				SCSI_SemEvt->OSEventCnt = 1;
				if(usb_init_flag==0)
				{
					usbHostSetIntEvt(USB_HOST_INT_EVT_SCAN);
					gUSBDevOn =2;
				}
				else
				{
					DEBUG_STORAGE("don't scan,when usb init \n");
				}
			}
			pre_port_status=port_status;
			HCPortSC = (PORTSC_PEC|PORTSC_CSC); 
		}
		else
		{
			// if(gUSBDevOn == 1)
			{
#if USB_HOST_MASS_SUPPORT
				DEBUG_STORAGE("***------SYS_EVT_USBCD_OFF----------\n");
				
				if(ehci_Atomic_SemEvt->OSEventCnt == 0)
					OSSemPost(ehci_Atomic_SemEvt);
				if(ehci_INT_SemEvt->OSEventCnt == 0)
					OSSemPost(ehci_INT_SemEvt);
				if(SCSI_SemEvt->OSEventCnt == 0)
					OSSemPost(SCSI_SemEvt);

#if SD_TASK_INSTALL_FLOW_SUPPORT
				usbHostSetIntEvt(USB_HOST_DEV_EVT_UNMOUNT);
#else
				if(!usb_hdd_removed)
				{
					// Follow UI FLOW to raise USB file system init
					if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)
						sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_NREADY);
					else
						sysSetStorageStatus(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_NREADY);
#if((SW_APPLICATION_OPTION != MR9300_RFDVR_RX1RX2) && (SW_APPLICATION_OPTION != MR9300_NETBOX_RX1RX2))
					if(!uiSentKeyToUi(UI_KEY_USBCD))
						sysSetUIKeyRetry(SYS_V_STORAGE_USBMASS);
#endif
				}
#endif
				gUSBDevOn = 0;
				HCPortSC = (PORTSC_PEC|PORTSC_CSC);
#endif
			}
		}
		HCUSBINTR|=USBINTR_PCE	;
		HCUSBSTS = USBSTS_PCD ; 
#endif
	}

	if((port_status & (PORTSC_CS)) == (PORTSC_CS))
	{
		/*Port connect and Port enable*/
		if(ehci_status & USBSTS_INT)
		{
			HCUSBSTS = USBSTS_INT;
			//if(check_irq())
			{
				async_flag=1;
				OSSemPost(ehci_INT_SemEvt);
			}
		}

		if(ehci_status & USBSTS_ERR)
		{
		 DEBUG_STORAGE("***------SYS_USBSTS_ERR----------\n");
			if(async_flag)
				gEhci_status |= USBSTS_ERR;
			else
				gEhci_periodic_status |= USBSTS_ERR;
			HCUSBSTS = USBSTS_ERR;
			OSSemPost(ehci_INT_SemEvt);
		}

		if(ehci_status & USBSTS_FLR)
		{
#if 0 /*We will ignore frame list roll over status*/
			if(async_flag)
				gEhci_status |= STS_FLR;
			else
				gEhci_periodic_status |= STS_FLR;
#endif
			HCUSBSTS = USBSTS_FLR;
		}

		if(ehci_status & USBSTS_FATAL)
		{
		 DEBUG_STORAGE("***------SYS_USBSTS_FATAL----------\n");
			if(async_flag)
				gEhci_status |= USBSTS_FATAL;
			else
				gEhci_periodic_status |= USBSTS_FATAL;
			HCUSBSTS = USBSTS_FLR;
				OSSemPost(ehci_INT_SemEvt); 
		}
	}
}


/* EOF */
#endif
