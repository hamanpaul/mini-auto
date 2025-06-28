
#include "general.h"
#include "board.h"
#include "task.h"
#include "osapi.h"
#include "sysapi.h"


#include "usb_main.h"
#include "ehci.h"
#include "farady_host_api.h"

#if (USB_HOST == 1)

#define USB_BUFSIZ	512

//extern int udev_index;

static struct usb_hub_device hub_dev[USB_MAX_HUB];
static int usb_hub_index;
struct usb_device *ghub_dev;
struct usb_device *ghid_dev_mouse, *ghid_dev_keyboard;
char hub_port;
extern char usb_started;	/* flag for the started/stopped USB status */
extern struct usb_device *usb_alloc_new_device(void);



static int usb_get_hub_descriptor(struct usb_device *dev, void *data, int size)
{
    DEBUG_UHUB("usb_get_hub_descripto\n");
    return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB, USB_DT_HUB << 8, 0, data, size, USB_CNTL_TIMEOUT);

}

static int usb_clear_port_feature(struct usb_device *dev, int port, int feature)
{
    DEBUG_UHUB("usb_clear_port%d_feature\n",port);
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                           USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature, port, NULL, 0, USB_CNTL_TIMEOUT);
}

static int usb_set_port_feature(struct usb_device *dev, int port, int feature)
{
    DEBUG_UHUB("usb_set_port%d_feature \n",port);
    return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                           USB_REQ_SET_FEATURE, USB_RT_PORT, feature, port, NULL, 0, USB_CNTL_TIMEOUT);
}

static int usb_get_hub_status(struct usb_device *dev, void *data)
{
    DEBUG_UHUB("usb_get_hub_status\n");
    return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0, data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}

static int usb_get_port_status(struct usb_device *dev, int port, void *data)
{
    DEBUG_UHUB("usb_get_port %d_status \n",port);
    return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
                           USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port, data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}


static void usb_hub_power_on(struct usb_hub_device *hub)
{
    int i;
    struct usb_device *dev;
    //unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2; // it never referenced

    dev = hub->pusb_dev;
    /* Enable power to the ports */
    DEBUG_UHOST("enabling power on all ports\n");
    for (i = 0; i < dev->maxchild; i++)
    {
        usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
        DEBUG_UHOST("port %d returns %lX\n", i + 1, dev->status);
    }

    /* Wait at least 100 msec for power to become stable */
    //ehci_mdelay(max(pgood_delay, (unsigned)100));
    ehci_mdelay(10);
}

void usb_hub_reset(void)
{
    usb_hub_index = 0;
}

static struct usb_hub_device *usb_hub_allocate(void)
{
    if (usb_hub_index < USB_MAX_HUB)
        return &hub_dev[usb_hub_index++];

    DEBUG_UHUB("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
    return NULL;
}

#define MAX_TRIES 5

static char *portspeed(int portstatus)
{
    if (portstatus & (1 << USB_PORT_FEAT_HIGHSPEED))
        return "480 Mb/s";
    else if (portstatus & (1 << USB_PORT_FEAT_LOWSPEED))
        return "1.5 Mb/s";
    else
        return "12 Mb/s";
}

int hub_port_reset(struct usb_device *dev, int port, unsigned short *portstat)
{
    int tries;
    struct usb_port_status portsts[1];
    unsigned short portstatus, portchange;

    DEBUG_UHUB("hub_port_reset: resetting port %d...\n", port);
    for (tries = 0; tries < MAX_TRIES; tries++)
    {

        usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
        ehci_mdelay(20);

        if (usb_get_port_status(dev, port + 1, portsts) < 0)
        {
            DEBUG_UHOST("get_port_status failed status %lX\n", dev->status);
            return -1;
        }
        portstatus = le16_to_cpu(portsts->wPortStatus);
        portchange = le16_to_cpu(portsts->wPortChange);

        DEBUG_UHUB("portstatus %x, change %x, %s\n",portstatus, portchange,portspeed(portstatus));

        DEBUG_UHOST("STAT_C_CONNECTION = %d STAT_CONNECTION = %d  USB_PORT_STAT_ENABLE %d\n",
                    (portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
                    (portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
                    (portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);

        if ((portchange & USB_PORT_STAT_C_CONNECTION) || !(portstatus & USB_PORT_STAT_CONNECTION))
            return -1;

        if (portstatus & USB_PORT_STAT_ENABLE)
            break;

        ehci_mdelay(20);
    }

    if (tries == MAX_TRIES)
    {
        DEBUG_UHOST("Cannot enable port %i after %i retries, disabling port.\n", port + 1, MAX_TRIES);
        DEBUG_UHOST("Maybe the USB cable is bad?\n");
        return -1;
    }

    usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
    *portstat = portstatus;
    return 0;
}

void usb_hub_and_port_reset(struct usb_device *dev, u8 port)
{
    dev->children[port]->enabled = 0;
    dev->children[port] = NULL;
}

extern u8 gUSBDevOn;

void usb_hub_port_connect_change(struct usb_device *dev, int port)
{
    struct usb_device *usb;
    struct usb_port_status portsts[1];
    unsigned short portstatus;

    /* Check status */
    if (usb_get_port_status(dev, port + 1, portsts) < 0)
    {
        DEBUG_UHUB("get_port_status failed\n");
        return;
    }

    portstatus = le16_to_cpu(portsts->wPortStatus);
    DEBUG_UHUB("portstatus %x, change %x, %s\n",portstatus,le16_to_cpu(portsts->wPortChange),portspeed(portstatus));

    /* Clear the connection change status */
    usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

    /* Disconnect any existing devices under this port */
    if (((!(portstatus & USB_PORT_STAT_CONNECTION)) && (!(portstatus & USB_PORT_STAT_ENABLE))) || (dev->children[port]))
    {
        DEBUG_UHUB("usb_disconnect(&hub->children[port]);\n");
        if(dev->children[port]->config.if_desc[0].desc.bInterfaceProtocol == USB_PROT_HID_KEYBOARD)
        {
            keyboard_running = HID_INITIAL_STATUS;
            usb_hub_and_port_reset(dev, port);
            DEBUG_UHOST("ghid_dev_keyboard->enabled = 0\n");
        }
        else if(dev->children[port]->config.if_desc[0].desc.bInterfaceProtocol == USB_PROT_HID_MOUSE)
        {
            mouse_running = HID_INITIAL_STATUS;
            usb_hub_and_port_reset(dev, port);
            DEBUG_UHOST("ghid_dev_mouse->enabled = 0\n");
        }
        /* Return now if nothing is connected */
        if (!(portstatus & USB_PORT_STAT_CONNECTION))
            return;
    }
    ehci_mdelay(10);

    /* Reset the port */
    if (hub_port_reset(dev, port, &portstatus) < 0)
    {
        DEBUG_UHUB("cannot reset port %i!?\n", port + 1);
        return;
    }

    ehci_mdelay(10);

    /* Allocate a new device struct for it */
    usb = usb_alloc_new_device();

    if (portstatus & USB_PORT_STAT_HIGH_SPEED)
        usb->speed = USB_SPEED_HIGH;
    else if (portstatus & USB_PORT_STAT_LOW_SPEED)
        usb->speed = USB_SPEED_LOW;
    else
        usb->speed = USB_SPEED_FULL;

    dev->children[port] = usb;
    usb->parent = dev;
    usb->portnr = port;
    /* Run it through the hoops (find a driver, etc) */
    if (usb_new_device(usb))
    {
        /* Woops, disable the port */
        usb_hub_and_port_reset(dev, port);
        DEBUG_UHUB("hub: disabling port %d\n", port + 1);
        usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
    }
}



static int usb_hub_configure(struct usb_device *dev)
{
    int i;
    u8 buffer [USB_BUFSIZ];
    u8 *bitmap;
    struct usb_hub_descriptor *descriptor;
    struct usb_hub_device *hub;

    struct usb_hub_status *hubsts;

    /* "allocate" Hub device */
    hub = usb_hub_allocate();
    if (hub == NULL)
        return -1;
    hub->pusb_dev = dev;
    /* Get the the hub descriptor */
    // Get USB HUB ¸s²Õdescriptor
    if (usb_get_hub_descriptor(dev, buffer, 4) < 0)
    {
        DEBUG_UHOST("usb_hub_configure: failed to get hub descriptor, giving up %lX\n", dev->status);
        return -1;
    }
    descriptor = (struct usb_hub_descriptor *)buffer;
    DEBUG_UHOST("******************************\n"\
                "descriptor->[attribute]: value\n"\
                "******************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "bNbrPorts: 0x%02x\n"\
                "wHubCharacteristics: 0x%04x\n"\
                "bPwrOn2PwrGood: 0x%02x\n"\
                "bHubContrCurrent: 0x%02x\n"\
                "DeviceRemovable: 0x%02x\n"\
                "PortPowerCtrlMask: 0x%02x\n"\
                "*******************************\n"
                , descriptor->bLength
                , descriptor->bDescriptorType
                , descriptor->bNbrPorts
                , descriptor->wHubCharacteristics
                , descriptor->bPwrOn2PwrGood
                , descriptor->bHubContrCurrent
                , descriptor->DeviceRemovable
                , descriptor->PortPowerCtrlMask);
    /* silence compiler warning if USB_BUFSIZ is > 256 [= sizeof(char)] */
    i = descriptor->bLength;
    if (i > USB_BUFSIZ)
    {
        DEBUG_UHOST("usb_hub_configure: failed to get hub descriptor - too long: %d\n", descriptor->bLength);
        return -1;
    }

    if (usb_get_hub_descriptor(dev, buffer, descriptor->bLength) < 0)
    {
        DEBUG_UHOST("usb_hub_configure: failed to get hub descriptor 2nd giving up %lX\n", dev->status);
        return -1;
    }
    memcpy((unsigned char *)&hub->desc, buffer, descriptor->bLength);
    /* adjust 16bit values */
    hub->desc.wHubCharacteristics = le16_to_cpu(descriptor->wHubCharacteristics);
    /* set the bitmap */
    bitmap = (unsigned char *)&hub->desc.DeviceRemovable[0];
    /* devices not removable by default */
    memset(bitmap, 0xff, (USB_MAX_CHILDREN+1+7)/8);
    bitmap = (unsigned char *)&hub->desc.PortPowerCtrlMask[0];
    memset(bitmap, 0xff, (USB_MAX_CHILDREN+1+7)/8); /* PowerMask = 1B */

    for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
        hub->desc.DeviceRemovable[i] = descriptor->DeviceRemovable[i];

    for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
        hub->desc.PortPowerCtrlMask[i] = descriptor->PortPowerCtrlMask[i];

    dev->maxchild = descriptor->bNbrPorts;
    DEBUG_UHUB("%d ports detected\n", dev->maxchild);

    switch (hub->desc.wHubCharacteristics & HUB_CHAR_LPSM)
    {
        case 0x00:
            DEBUG_UHUB("ganged power switching\n");
            break;
        case 0x01:
            DEBUG_UHUB("individual port power switching\n");
            break;
        case 0x02:
        case 0x03:
            DEBUG_UHUB("unknown reserved power switching mode\n");
            break;
    }

    if (hub->desc.wHubCharacteristics & HUB_CHAR_COMPOUND)
        DEBUG_UHUB("part of a compound device\n");
    else
        DEBUG_UHUB("standalone hub\n");

    switch (hub->desc.wHubCharacteristics & HUB_CHAR_OCPM)
    {
        case 0x00:
            DEBUG_UHUB("global over-current protection\n");
            break;
        case 0x08:
            DEBUG_UHUB("individual port over-current protection\n");
            break;
        case 0x10:
        case 0x18:
            DEBUG_UHUB("no over-current protection\n");
            break;
    }

    DEBUG_UHOST("power on to power good time: %dms\n",
                descriptor->bPwrOn2PwrGood * 2);
    DEBUG_UHOST("hub controller current requirement: %dmA\n",
                descriptor->bHubContrCurrent);

    for (i = 0; i < dev->maxchild; i++)
        DEBUG_UHUB("port %d is%s removable\n", i + 1, hub->desc.DeviceRemovable[(i + 1) / 8] & (1 << ((i + 1) % 8)) ? " not" : "");

    if (sizeof(struct usb_hub_status) > USB_BUFSIZ)
    {
        DEBUG_UHUB("usb_hub_configure: failed to get Status - too long: %d\n", descriptor->bLength);
        return -1;
    }

    if (usb_get_hub_status(dev, buffer) < 0)
    {
        DEBUG_UHUB("usb_hub_configure: failed to get Status %lX\n",	dev->status);
        return -1;
    }


    hubsts = (struct usb_hub_status *)buffer;

    DEBUG_UHUB("get_hub_status returned status %X, change %X\n", le16_to_cpu(hubsts->wHubStatus), le16_to_cpu(hubsts->wHubChange));
    DEBUG_UHUB("local power source is %s\n", (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? "lost (inactive)" : "good");
    DEBUG_UHUB("%sover-current condition exists\n", (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? "" : "no ");
    usb_hub_power_on(hub);

    for (i = 0; i < dev->maxchild; i++)
    {
        struct usb_port_status portsts[1];
        unsigned short portstatus, portchange;

        if (usb_get_port_status(dev, i + 1, portsts) < 0)
        {
            DEBUG_UHUB("get_port_status failed: %d\n",i);
            continue;
        }

        portstatus = le16_to_cpu(portsts->wPortStatus);
        portchange = le16_to_cpu(portsts->wPortChange);
        DEBUG_UHUB("HUB:Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

        if (portchange & USB_PORT_STAT_C_CONNECTION)
        {
            DEBUG_UHUB("port %d connection change\n", i + 1);
            usb_hub_port_connect_change(dev, i);
        }
        if (portchange & USB_PORT_STAT_C_ENABLE)
        {
            DEBUG_UHUB("port %d enable change, status %x\n",i + 1, portstatus);
            usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);

            /* EM interference sometimes causes bad shielded USB
             * devices to be shutdown by the hub, this hack enables
             * them again. Works at least with mouse driver */
            if (!(portstatus & USB_PORT_STAT_ENABLE) && (portstatus & USB_PORT_STAT_CONNECTION) && ((dev->children[i])))
            {
                DEBUG_UHUB("already running port %i disabled by hub (EMI?), re-enabling...\n", i + 1);
                usb_hub_port_connect_change(dev, i);
            }
        }
        if (portstatus & USB_PORT_STAT_SUSPEND)
        {
            DEBUG_UHUB("port %d suspend change\n", i + 1);
            usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
        }

        if (portchange & USB_PORT_STAT_C_OVERCURRENT)
        {
            DEBUG_UHUB("port %d over-current change\n", i + 1);
            usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_OVER_CURRENT);
            usb_hub_power_on(hub);
        }

        if (portchange & USB_PORT_STAT_C_RESET)
        {
            DEBUG_UHUB("port %d reset change\n", i + 1);
            usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
        }
    } /* end for i all ports */

    return 0;
}

int hub_event_handle(char port)
{
    int i = port-1;
    struct usb_device *dev = ghub_dev;
    struct usb_port_status portsts[1];
    unsigned short portstatus, portchange;

    if (usb_get_port_status(dev, port, portsts) < 0)
    {
        DEBUG_UHUB("get_port_status failed: %d\n",i);
        return -1;
    }

    portstatus = le16_to_cpu(portsts->wPortStatus);
    portchange = le16_to_cpu(portsts->wPortChange);
    DEBUG_UHUB(" hub Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

    if (portchange & USB_PORT_STAT_C_CONNECTION)
    {
  
        DEBUG_UHUB(" hub port %d change\n", i + 1);
        usb_hub_port_connect_change(dev, i);
       if((port==2))
       	{
//       	  DEBUG_SHOW_START();
			DEBUG_STORAGE("USB:hdd remount\n");
		usb_started = 1; 
       if( usb_stor_scan(1)==-1)
       	{
        		DEBUG_STORAGE("USB:Scan retry\n");		
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
//		DEBUG_SHOW_END();
       	}
	
    }
    if (portchange & USB_PORT_STAT_C_ENABLE)
    {
        DEBUG_UHUB("port %d enable change, status %x\n", i + 1, portstatus);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);

        /* EM interference sometimes causes bad shielded USB
         * devices to be shutdown by the hub, this hack enables
         * them again. Works at least with mouse driver */
        if (!(portstatus & USB_PORT_STAT_ENABLE) && (portstatus & USB_PORT_STAT_CONNECTION) && ((dev->children[i])))
        {
            DEBUG_UHUB("already running port %i disabled by hub (EMI?), re-enabling...\n", i + 1);
            usb_hub_port_connect_change(dev, i);
        }
    }
    if (portstatus & USB_PORT_STAT_SUSPEND)
    {
        DEBUG_UHUB("port %d suspend change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
    }

    if (portchange & USB_PORT_STAT_C_OVERCURRENT)
    {
        DEBUG_UHUB("port %d over-current change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_OVER_CURRENT);
        //usb_hub_power_on(hub);
    }

    if (portchange & USB_PORT_STAT_C_RESET)
    {
        DEBUG_UHUB("port %d reset change\n", i + 1);
        usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
    }

    return 0;
}

int usb_hub_probe(struct usb_device *dev, int ifnum)
{
    struct usb_interface *iface;
    struct usb_endpoint_descriptor *ep;
    int ret;

    iface = &dev->config.if_desc[ifnum];
    /* Is it a hub? */
    if (iface->desc.bInterfaceClass != USB_CLASS_HUB)
        return 0;
    /* Some hubs have a subclass of 1, which AFAICT according to the */
    /*  specs is not defined, but it works */
    if ((iface->desc.bInterfaceSubClass != 0) && (iface->desc.bInterfaceSubClass != 1))
        return 0;
    /* Multiple endpoints? What kind of mutant ninja-hub is this? */
    if (iface->desc.bNumEndpoints != 1)
        return 0;

    ep = &iface->ep_desc[0];
    /* Output endpoint? Curiousier and curiousier.. */
    if (!(ep->bEndpointAddress & USB_DIR_IN))
        return 0;
    /* If it's not an interrupt endpoint, we'd better punt! */
    if ((ep->bmAttributes & 3) != 3)
        return 0;
    DEBUG_UHOST("*********usb_hub_probe********\n"\
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
                "******************************\n"
                , iface->desc.bLength
                , iface->desc.bDescriptorType
                , iface->desc.bInterfaceNumber
                , iface->desc.bAlternateSetting
                , iface->desc.bNumEndpoints
                , iface->desc.bInterfaceClass
                , iface->desc.bInterfaceSubClass
                , iface->desc.bInterfaceProtocol
                , iface->desc.iInterface);
    DEBUG_UHOST("*********usb_hub_probe*********\n"\
                "ep->[attribute]: value\n"\
                "*******************************\n"\
                "bLength: 0x%02x\n"\
                "bDescriptorType: 0x%02x\n"\
                "bEndpointAddress: 0x%02x\n"\
                "bmAttributes: 0x%02x\n"\
                "wMaxPacketSize: 0x%04x\n"\
                "bInterval: 0x%02x\n"\
                "*******************************\n"
                , ep->bLength
                , ep->bDescriptorType
                , ep->bEndpointAddress
                , ep->bmAttributes
                , ep->wMaxPacketSize
                , ep->bInterval);
    /* We found a hub */
    DEBUG_UHUB("USB hub found\n");
    if(dev->devnum>1)
    {
        DEBUG_UHUB("USB Really hub found\n");
        DEBUG_UHUB("ep->wMaxPacketSize: %x\n",ep->wMaxPacketSize);
        DEBUG_UHUB("ep->bEndpointAddress: %x\n",ep->bEndpointAddress);
        DEBUG_UHUB("dev->devnum: %x\n",dev->devnum);
        hub_running=1;
        ghub_dev=dev;
        ghub_addr=dev->devnum;
        hub_qh_init(ep->wMaxPacketSize, ep->bEndpointAddress, dev->devnum, 1);
        hub_mode=HUB_POLL_STATUS; //@POLL
    }
    ret = usb_hub_configure(dev);
    return ret;
}
#if( ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) &&(HW_BOARD_OPTION== MR9300_RX_RDI))
void UsbHubRst(void)
{

     //   gpioSetLevel(GPIO_GROUP_USB, GPIO_HUB_RESET,1);
  //      _nop_();
	//OSTimeDly(1);
   //    gpioSetLevel(GPIO_GROUP_USB, GPIO_HUB_RESET,0);
//	_nop_();
	//OSTimeDly(1);
     gpioSetLevel(GPIO_GROUP_USB, GPIO_HUB_RESET,1);
     gpioSetLevel(GPIO_GROUP_USB,GPIO_USB_POWER_CONTROL,1);
}
#endif
#endif
