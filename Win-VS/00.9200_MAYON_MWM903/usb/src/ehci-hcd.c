
#include "general.h"
#include "board.h"
#include "task.h"
#include "gpioapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "sysapi.h"
#include "usbapi.h"

#include "usb_main.h"
#include "ehci.h"
#include "farady_host_api.h"
#include "timerapi.h"
#include "../timer/inc/timer.h"

#if (USB_HOST == 1)

extern OS_EVENT* ehci_INT_SemEvt; /* semaphore to synchronize event processing */
extern OS_EVENT* ehci_Atomic_SemEvt; /* semaphore to synchronize event processing */
extern struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
extern volatile struct ehci_hcor *hcor;
extern u32 gEhci_status,pre_port_status;
extern u32 gEhci_periodic_status;
extern INT32U marsTimerCountRead(INT32U uiTimerId, INT32U* pCount);
INT32U timestamp;
INT32U  lastdec;

#define READ_TIMER (marsTimerCountRead(guiIRTimerId, TIMER1_COUNT))


int rootdev;


static u16 portreset;

static __packed struct descriptor
{
    struct usb_hub_descriptor hub;
    struct usb_device_descriptor device;
    struct usb_configuration_descriptor config;
    struct usb_interface_descriptor interface;
    struct usb_endpoint_descriptor endpoint;
} descriptor =
{
    {
        0x8,		/* bDescLength */
        0x29,		/* bDescriptorType: hub descriptor */
        1,		/* bNrPorts -- runtime modified */
        0,		/* wHubCharacteristics */
        10,		/* bPwrOn2PwrGood */
        0,		/* bHubCntrCurrent */
        {0xFF,0xFF},		/* Device not removable */
        {0xFF,0xFF}		/* at most 7 ports! XXX */
    },
    {
        0x12,		/* bLength */
        1,		/* bDescriptorType: UDESC_DEVICE */
        cpu_to_le16(0x0200), /* bcdUSB: v2.0 */
        9,		/* bDeviceClass: UDCLASS_HUB */
        0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
        1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
        64,		/* bMaxPacketSize: 64 bytes */
        0x0000,		/* idVendor */
        0x0000,		/* idProduct */
        cpu_to_le16(0x0100), /* bcdDevice */
        1,		/* iManufacturer */
        2,		/* iProduct */
        0,		/* iSerialNumber */
        1		/* bNumConfigurations: 1 */
    },
    {
        0x9,
        2,		/* bDescriptorType: UDESC_CONFIG */
        cpu_to_le16(0x19),
        1,		/* bNumInterface */
        1,		/* bConfigurationValue */
        0,		/* iConfiguration */
        0x40,		/* bmAttributes: UC_SELF_POWER */
        0		/* bMaxPower */
    },
    {
        0x9,		/* bLength */
        4,		/* bDescriptorType: UDESC_INTERFACE */
        0,		/* bInterfaceNumber */
        0,		/* bAlternateSetting */
        1,		/* bNumEndpoints */
        9,		/* bInterfaceClass: UICLASS_HUB */
        0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
        0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
        0		/* iInterface */
    },
    {
        0x7,		/* bLength */
        5,		/* bDescriptorType: UDESC_ENDPOINT */
        0x81,		/* bEndpointAddress:
				 * UE_DIR_IN | EHCI_INTR_ENDPT
				 */
        3,		/* bmAttributes: UE_INTERRUPT */
        8,		/* wMaxPacketSize */
        255		/* bInterval */
    },
};



void reset_timer_masked (void);
u32 get_timer_masked (void);

void reset_timer (void)
{
    reset_timer_masked ();
}

u32 get_timer (u32 base)
{
    return get_timer_masked () - base;
}

void set_timer (u32 t)
{
    timestamp = t;
}

void __udelay (unsigned long usec)
{
    u32 tmo;

    if(usec>=10)
        tmo = usec / 10;
    else
        tmo=1;


    tmo += get_timer (0);

//while (get_timer_masked () < tmo)
        /*NOP*/;
}

void reset_timer_masked (void)
{
    /* reset time */
    marsTimerCountRead(guiIRTimerId, &lastdec);
    timestamp = 0;
}

u32 get_timer_masked (void)
{
    INT32U now;

    marsTimerCountRead(guiIRTimerId, &now);

    if (lastdec >= now)
    {
        /* normal mode */
        timestamp += lastdec - now;
    }
    else
    {
        /* we have an overflow ... */
        timestamp += lastdec + TIMER1_COUNT - now;
    }
    lastdec = now;

    return timestamp;
}

void udelay_masked (unsigned long usec)
{
    u32 tmo;
    u32 endtime;
    signed long diff;

    if (usec >= 10)
    {
        tmo = usec / 10;

    }
    else
    {
        tmo = 1;
    }

    endtime = get_timer(0) + tmo;

    do
    {
        u32 now = get_timer_masked ();
        diff = endtime - now;
    }
    while (diff >= 0);
}


void ehci_udelay(unsigned long usec)
{
    u32 kv;

    do
    {
        kv = usec;

        __udelay (kv);
        usec -= kv;
    }
    while(usec);

}

void ehci_mdelay(unsigned long msec)
{
    while (msec--)
        ehci_udelay(1000);
}




static int handshake(u32 *ptr, u32 mask, u32 done, int usec)
{
    u32 result;
    do
    {
        result = ehci_readl(ptr);
        ehci_udelay(5);
        if (result == ~(u32)0)
            return -1;
        result &= mask;
        if (result == done)
            return 0;
        usec--;
    }
    while (usec > 0);
    return -1;
}

int ehci_reset(void)
{
    u32 cmd;
    //u32 tmp;
    //u32 *reg_ptr;
    int ret = 0;

    cmd = ehci_readl(&hcor->or_usbcmd);
    cmd = (cmd & ~USBCMD_RUN) | USBCMD_RESET;
    //ehci_writel(&hcor->or_usbcmd, cmd);
    hcor->or_usbcmd = cmd;
    ret = handshake((u32 *)&hcor->or_usbcmd, USBCMD_RESET, 0, 250 * 1000);
    if (ret < 0)
    {
        DEBUG_STORAGE("EHCI fail to reset\n");
    }
    return ret;
}

static int ehci_td_buffer(struct qTD *td, void *buf, u32 sz)
{
    u32 delta, next;
    u32 addr = (u32)buf;
    //u32 rsz = roundup(sz, 32);
    int idx;

#if 0
    if (sz != rsz)
        DEBUG_UHOST("EHCI-HCD: Misaligned buffer size (%08x)\n", sz);

    if (addr & 31)
        DEBUG_UHOST("EHCI-HCD: Misaligned buffer address (%p)\n", buf);
#endif
    idx = 0;
    while (idx < 5)
    {
#if USB_UNDO
        flush_dcache_range(addr, addr + rsz);
#endif

        td->qtd_buffer[idx] = cpu_to_hc32(addr);
        next = (addr + 0x1000) & ~0xFFF;
        delta = next - addr;
        if (delta >= sz)
            break;
        sz -= delta;
        addr = next;
        idx++;
    }

    if (idx == 5)
    {
        DEBUG_STORAGE("out of buffer pointers (%u bytes left)\n", sz);
        return -1;
    }

    return 0;
}


#define DISP_LINE_LEN 16

void format_printf(unsigned long offset,unsigned long length,unsigned int buf)
{
    unsigned int	addr;
    unsigned long	i,linebytes,nbytes,eeprom_offset;
    unsigned char	*cp;
    int	size;  //Byte order
    char	linebuf[DISP_LINE_LEN];

    size=4;
    addr=buf;
    eeprom_offset=offset;

    nbytes = length * size;

 //   do
 //   {
        //unsigned int *uip = (unsigned int *)linebuf;
        //unsigned short *usp = (unsigned short *)linebuf;
        //unsigned char	*ucp = (unsigned char *)linebuf;
        DEBUG_UHOST("%04lx:", eeprom_offset);
        linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

        for (i=0; i<linebytes; i+= size)
        {
            if (size == 4)
            {
                DEBUG_UHOST(" %08x", *((unsigned int *)addr)); //(*uip++ = *((unsigned int *)addr)));
            }
            else if (size == 2)
            {
                DEBUG_UHOST(" %04x", *((unsigned int *)addr)); //(*usp++ = *((unsigned short *)addr)));
            }
            else
            {
                DEBUG_UHOST(" %02x", *((unsigned int *)addr)); //(*ucp++ = *((unsigned char *)addr)));
            }
            addr += size;
            eeprom_offset += size;
        }
#if 0
        DEBUG_UHOST ("    ");
        cp = (unsigned char *)linebuf;
        for (i=0; i<linebytes; i++)
        {
            if ((*cp < 0x20) || (*cp > 0x7e))
                DEBUG_UHOST(".");
            else
                DEBUG_UHOST("%c", *cp);
            cp++;
        }
#endif		
        DEBUG_UHOST ("\n");
  //      nbytes -= linebytes;

  //  }
   // while (nbytes > 0);
}

int g_cnt=0;

#if 0
#define NAK_COUNT   15
#define CONTROL_MAX_PKT_SIZE    64
#define BULK_MAX_PKT_SIZE       512
#define H_BIT                   0x00008000
#define DTC_BIT                 0x00004000
#define HIGH_SPEED              0x00002000
#define I_BIT                   0x00000080
#define SINGLE_TRANSACTION      0x40000000
#define	OUT_PID					0x00000000
#define	IN_PID					0x00000100
#define	SETUP_PID				0x00000200
ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
                  int length, struct devrequest *req)
{
    struct QH *qh = (struct QH *) qh_addr;
    struct qTD *qtd[USB_MAX_QTD];
    int qtd_index = 0;

    volatile struct qTD *vtd;
    unsigned long ts;
    u32 *tdp;
    u32 endpt, token, usbsts;
    u32 c, toggle;
    u32 cmd;
    int timeout;
    int ret = 0;
    int i;
    u8 ucErr;

    DEBUG_UHOST("dev 2=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe,
                buffer, length, req);
    if (req != NULL)
        DEBUG_UHOST("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
                    req->request, req->request,
                    req->requesttype, req->requesttype,
                    le16_to_cpu(req->value), le16_to_cpu(req->value),
                    le16_to_cpu(req->index));


    memset(qh, 0, sizeof(struct QH));

    for(i=0; i<USB_MAX_QTD; i++)
    {
        qtd[i]=(struct qTD *) qtd_addr[i];
        memset(qtd[i], 0, sizeof(struct qTD));
    }

    cmd = ehci_readl(&hcor->or_usbcmd);
    cmd |= USBCMD_ASE ;
    ehci_writel(&hcor->or_usbcmd, cmd);

    HCUSBCMD = 0x00010121;
    HCUSBSTS = 0x00008000;
    HCUSBINTR = 0x00000007;
    HCAsyncListAddr = (unsigned int) qh;
    qh->qh_link = cpu_to_hc32((u32)qh_list_addr | USB_QH_LINK_TYPE_QH);
    qh->qh_endpt1 =  ( HIGH_SPEED | H_BIT | DTC_BIT | (CONTROL_MAX_PKT_SIZE << 16));
    qh->qh_endpt2 = SINGLE_TRANSACTION;
    qh->qh_overlay.qtd_next = (unsigned long) qtd[0];
    qh->qh_overlay.qtd_altnext = 1;

    qtd[0]->qtd_next = (unsigned long) qtd[1];
    qtd[0]->qtd_altnext = (unsigned long) qtd[1];
    qtd[0]->qtd_buffer[0] = (unsigned long) req;
    qtd[0]->qtd_buffer[1] = (unsigned long) req;
    qtd[0]->qtd_buffer[2] = (unsigned long) req;
    qtd[0]->qtd_buffer[3] = (unsigned long) req;
    qtd[0]->qtd_buffer[4] = (unsigned long) req;
    qtd[0]->qtd_token |=  (0x00008000 | 0x00000200 | (0<<31) | (length<<16));

    qtd[1]->qtd_next = 1;
    qtd[1]->qtd_altnext =1;
    qtd[1]->qtd_buffer[0] = (unsigned long) req;
    qtd[1]->qtd_buffer[1] = (unsigned long) req;
    qtd[1]->qtd_buffer[2] = (unsigned long) req;
    qtd[1]->qtd_buffer[3] = (unsigned long) req;
    qtd[1]->qtd_buffer[4] = (unsigned long) req;
    qtd[1]->qtd_token = 0;

    qtd[0]->qtd_token |= 0x80;

    ts = get_timer(0);

    timeout = USB_TIMEOUT_MS(pipe);
    do
    {

        token = hc32_to_cpu(qtd[0]->qtd_token);
        if (!(token & 0x80))
            break;
    }
    while (get_timer(ts) < timeout*10);

    if (token & 0x80)
    {
        printf("EHCI timed out on TD - token=%#x\n", token);
    }
    format_printf(0, sizeof(struct ehci_hcor), (int) hcor);
    DEBUG_UHOST("QH 1: %x\n",qh_list_addr);
    format_printf(0, sizeof(struct QH) ,(int) qh_list_addr);
    DEBUG_UHOST("QH 2: %x\n",qh_addr);
    format_printf(0, sizeof(struct QH) ,(int) qh_addr);
    DEBUG_UHOST("QTD 1: %x\n", qtd[0]);
    format_printf(0, sizeof(struct qTD) , (int) qtd[0]);
    DEBUG_UHOST("QTD 2: %x\n", qtd[1]);
    format_printf(0, sizeof(struct qTD) , (int) qtd[1]);
    DEBUG_UHOST("QTD 3: %x\n", qtd[2]);
    format_printf(0, sizeof(struct qTD) , (int) qtd[2]);


}

if (length > 0 || req == NULL)
{

    int pdebug=0;
    /*
     * Setup request qTD (3.5 in ehci-r10.pdf)
     *
     *   qtd_next ................ 03-00 H
     *   qtd_altnext ............. 07-04 H
     *   qtd_token ............... 0B-08 H
     *
     *   [ buffer, buffer_hi ] loaded with "buffer".
     */
    do
    {

        xfr_bytes = 5 * 4096;

        xfr_bytes -= (u32)buf_ptr & (4096 - 1);

        xfr_bytes &= ~(0X200 - 1);

        if(xfr_bytes <left_length)
            xfr_bytes=xfr_bytes;
        else
            xfr_bytes = left_length;

        if(xfr_bytes <left_length)
        {
            pdebug=1;
        }
        if(pdebug)
        {
            DEBUG_UHOST("maxpacket: %x\n",maxpacket);
            DEBUG_UHOST("length: %x\n",length);
            DEBUG_UHOST("xfr_bytes: %x\n",xfr_bytes);
            DEBUG_UHOST("left_length: %x\n",left_length);
            DEBUG_UHOST("toggle: %x\n",toggle);
        }
        qtd[qtd_index]->qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
#if 1
        token = (toggle << 31) |
                (xfr_bytes << 16) |
                ((req == NULL ? 1 : 0) << 15) |
                (0 << 12) |
                (3 << 10) |
                ((usb_pipein(pipe) ? 1 : 0) << 8) | (0x80 << 0);
#else
        token = (toggle << 31) |
                (length << 16) |
                ((req == NULL ? 1 : 0) << 15) |
                (0 << 12) |
                (3 << 10) |
                ((usb_pipein(pipe) ? 1 : 0) << 8) | (0x80 << 0);
#endif
        qtd[qtd_index]->qtd_token = cpu_to_hc32(token);
        if (ehci_td_buffer(qtd[qtd_index], buf_ptr, xfr_bytes) != 0)
        {
            DEBUG_UHOST("unable construct DATA td\n");
            goto fail;
        }
        /* Update previous qTD! */
        *tdp = cpu_to_hc32((u32)qtd[qtd_index]);
        tdp = &qtd[qtd_index++]->qtd_next;

        //if ((xfr_bytes / maxpacket) & 1)
        toggle ^= 1;
        buf_ptr += xfr_bytes;
        left_length -= xfr_bytes;

    }
    while(left_length > 0);
}
#else
static int ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer, int length, struct devrequest *req)
{
    struct QH *qh = (struct QH *) qh_addr;
    struct qTD *qtd[USB_MAX_QTD];
    int qtd_index = 0;

    u32 *tdp;
    u32 token, toggle;
    u32 cmd, usbsts;
    //int timeout;
    int ret = 0;
    int i;
    u8 ucErr;
  // DEBUG_STORAGE("ehci_submit_async OSSemPend");
    OSSemPend(ehci_Atomic_SemEvt, EHCI_TIMEOUT, &ucErr);
  if (ucErr != OS_NO_ERR)
        {
            DEBUG_STORAGE("Error:ehci_Atomic_SemEvt is %d.\n", ucErr);
  	}

#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
    if(gUSBDevOn == 0)
    	{
		  DEBUG_STORAGE("ehci_Atomic_Error:Dev pull out .\n");
	     return -1;
       }
#endif
	
  // DEBUG_STORAGE("#");
    DEBUG_UHOST("dev 2=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe, buffer, length, req);
    if (req != NULL)
        DEBUG_UHOST("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
                    req->request, req->request, req->requesttype, req->requesttype,
                    le16_to_cpu(req->value), le16_to_cpu(req->value), le16_to_cpu(req->index));

    memset(qh, 0, sizeof(struct QH));
    for(i=0; i < USB_MAX_QTD; i++)
    {
        qtd[i]=(struct qTD *) qtd_addr[i];
        memset(qtd[i], 0, sizeof(struct qTD));
    }

    toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

    /*
     * Setup QH (3.6 in ehci-r10.pdf)
     *
     *   qh_link ................. 03-00 H
     *   qh_endpt1 ............... 07-04 H
     *   qh_endpt2 ............... 0B-08 H
     * - qh_curtd
     *   qh_overlay.qtd_next ...... 13-10 H
     * - qh_overlay.qtd_altnext
     */
    qh->qh_link = cpu_to_hc32((u32)qh_list_addr | USB_QH_LINK_TYPE_QH);
    qh->qh_endpt1 = cpu_to_hc32(
                        USB_QH_EP_CHAR_RL(8) |
                        USB_QH_EP_CHAR_C((usb_pipespeed(pipe) != USB_SPEED_HIGH) && (usb_pipeendpoint(pipe) == 0)) |
                        USB_QH_EP_CHAR_MAX_PACKET(usb_maxpacket(dev, pipe)) |
                        USB_QH_EP_CHAR_DTC |
                        USB_QH_EP_CHAR_EPS(usb_pipespeed(pipe)) |
                        USB_QH_EP_CHAR_EP(usb_pipeendpoint(pipe)) |
                        USB_QH_EP_CHAR_I(0) |
                        USB_QH_EP_CHAR_DEV_ADDR(usb_pipedevice(pipe)));
    qh->qh_endpt2 = cpu_to_hc32(
                        USB_QH_EP_CAP_MULT_ONE |
                        USB_QH_EP_CAP_PORT_NUM(dev->portnr + 1) |
                        USB_QH_EP_CAP_HUB_ADDR(dev->parent->devnum) |
                        USB_QH_EP_CAP_UFRAME_CMASK(0) |
                        USB_QH_EP_CAP_UFRAME_SMASK(0));
    qh->qh_overlay.qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);

    tdp = &qh->qh_overlay.qtd_next;

    /* this function is call by submit_bulk_msg, submit_control_msg, submit_int_msg
     * so, it has to be separated into three pieces
     */
    //for submit_control_msg
    if (req != NULL)
    {
        /*
         * Setup request qTD (3.5 in ehci-r10.pdf)
         *
         *   qtd_next ................ 03-00 H
         *   qtd_altnext ............. 07-04 H
         *   qtd_token ............... 0B-08 H
         *
         *   [ buffer, buffer_hi ] loaded with "req".
         */
        qtd[qtd_index]->qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_token = cpu_to_hc32(
                                        USB_QTD_TOKEN_DT(toggle) |
                                        USB_QTD_TOKEN_TRANS_SIZE(sizeof(*req)) |
                                        USB_QTD_TOKEN_C_PAGE(0) |
                                        USB_QTD_TOKEN_CERR(3) |
                                        USB_QTD_TOKEN_PID_SETUP |
                                        USB_QTD_TOKEN_STAT_ACTIVE);
        /* set the buffer pointer data  */
        if (ehci_td_buffer(qtd[qtd_index], req, sizeof(*req)) != 0)
        {
            DEBUG_STORAGE("unable construct SETUP td\n");
            goto fail;
        }
        /* Update previous qTD! */
        *tdp = cpu_to_hc32((u32)qtd[qtd_index]);
        tdp = &qtd[qtd_index++]->qtd_next;
        toggle = 1;
    }
    //for submit_bulk_msg, submit_control_msg, submit_int_msg
    if (length > 0 || req == NULL)
    {
        /*
         * Setup request qTD (3.5 in ehci-r10.pdf)
         *
         *   qtd_next ................ 03-00 H
         *   qtd_altnext ............. 07-04 H
         *   qtd_token ............... 0B-08 H
         *
         *   [ buffer, buffer_hi ] loaded with "buffer".
         */
        qtd[qtd_index]->qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_token = cpu_to_hc32(
                                        USB_QTD_TOKEN_DT(toggle) |
                                        USB_QTD_TOKEN_TRANS_SIZE(length) |
                                        USB_QTD_TOKEN_IOC(req == NULL) |
                                        USB_QTD_TOKEN_C_PAGE(0) |
                                        USB_QTD_TOKEN_CERR(3) |
                                        (usb_pipein(pipe) ? USB_QTD_TOKEN_PID_IN : USB_QTD_TOKEN_PID_OUT) |
                                        USB_QTD_TOKEN_STAT_ACTIVE);

        if (ehci_td_buffer(qtd[qtd_index], buffer, length) != 0)
        {
            DEBUG_STORAGE("unable construct DATA td\n");
            goto fail;
        }
        /* Update previous qTD! */
        *tdp = cpu_to_hc32((u32)qtd[qtd_index]);
        tdp = &qtd[qtd_index++]->qtd_next;
    }
    //for submit_control_msg
    if (req != NULL)
    {
        /*
         * Setup request qTD (3.5 in ehci-r10.pdf)
         *
         *   qtd_next ................ 03-00 H
         *   qtd_altnext ............. 07-04 H
         *   qtd_token ............... 0B-08 H
         */
        qtd[qtd_index]->qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
        qtd[qtd_index]->qtd_token = cpu_to_hc32(
                                        USB_QTD_TOKEN_DT(toggle) |
                                        USB_QTD_TOKEN_TRANS_SIZE(0) |
                                        USB_QTD_TOKEN_IOC(1) |
                                        USB_QTD_TOKEN_C_PAGE(0) |
                                        USB_QTD_TOKEN_CERR(3) |
                                        (usb_pipein(pipe) ? USB_QTD_TOKEN_PID_OUT : USB_QTD_TOKEN_PID_IN) |
                                        USB_QTD_TOKEN_STAT_ACTIVE);

        /* Update previous qTD! */
        *tdp = cpu_to_hc32((u32)qtd[qtd_index]);
        tdp = &qtd[qtd_index++]->qtd_next;
    }

    //(struct QH *) qh_list_addr->qh_link = cpu_to_hc32((u32)qh | USB_QH_LINK_TYPE_QH);

    usbsts = ehci_readl(&hcor->or_usbsts);
    //ehci_writel(&hcor->or_usbsts, (usbsts & 0x3f));	//?
    hcor->or_usbsts = (usbsts & 0x3f);
#if 0
    if(g_cnt <=1)
    {
        if(g_cnt==0)
        {
            DEBUG_UHOST("\n");
            format_printf(0, sizeof(struct ehci_hcor), (int) hcor);
        }
        DEBUG_UHOST("QH 1: %x\n",qh_list_addr);
        format_printf(0, sizeof(struct QH) ,(int) qh_list_addr);
        DEBUG_UHOST("QH 2: %x\n",qh_addr);
        format_printf(0, sizeof(struct QH) ,(int) qh_addr);
        DEBUG_UHOST("QTD 1: %x\n", qtd[0]);
        format_printf(0, sizeof(struct qTD) , (int) qtd[0]);
        DEBUG_UHOST("QTD 2: %x\n", qtd[1]);
        format_printf(0, sizeof(struct qTD) , (int) qtd[1]);
        DEBUG_UHOST("QTD 3: %x\n", qtd[2]);
        format_printf(0, sizeof(struct qTD) , (int) qtd[2]);
        g_cnt++;
    }
#endif
    /* Enable async. schedule. */
    cmd = ehci_readl(&hcor->or_usbcmd);
    cmd |= USBCMD_ASE ;
    //ehci_writel(&hcor->or_usbcmd, cmd);
    hcor->or_usbcmd = cmd;

    ret = handshake((u32 *)&hcor->or_usbsts, USBSTS_ASS, USBSTS_ASS, 100 * 1000);
    if (ret < 0)
    {
        DEBUG_UHOST("EHCI fail timeout USBSTS_ASS set\n");
        DEBUG_UHOST("USBCMD: %x\n",ehci_readl(&hcor->or_usbcmd));
        DEBUG_UHOST("USBSTS: %x\n",ehci_readl(&hcor->or_usbsts));
        DEBUG_UHOST("PORTSC: %x\n",ehci_readl(&hcor->or_portsc[0]));
        goto fail;
    }

#if 0
    /* Wait for TDs to be processed. */
    ts = get_timer(0);
    vtd = qtd[qtd_index - 1];
    timeout = USB_TIMEOUT_MS(pipe);
    do
    {

        token = hc32_to_cpu(vtd->qtd_token);
        if (!(token & 0x80))
            break;
    }
    while (get_timer(ts) < timeout*10);

    /* Check that the TD processing happened */
    if (token & 0x80)
    {
        DEBUG_UHOST("EHCI timed out on TD - token=%#x\n", token);
    }
#else
    OSSemPend(ehci_INT_SemEvt, 100*EHCI_TIMEOUT, &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_STORAGE("TimeOut: Async\n");
        DEBUG_STORAGE("dev=%u, usbsts=%#x, p[1]=%#x,\n", dev->devnum, ehci_readl(&hcor->or_usbsts), ehci_readl(&hcor->or_portsc));
    }

    if (gEhci_status)
    {
        DEBUG_UHOST("Error: ehci_INT_SemEvt is %d.\n", gEhci_status);
        gEhci_status = 0;
        // DEBUG_UHOST("TOKEN=%#x\n",hc32_to_cpu(qh->qh_overlay.qtd_token));
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)|| (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
    if(gUSBDevOn == 0)
    	{
		  DEBUG_STORAGE("ehci_INT_Error:Dev pull out O.\n", gEhci_status);
	     return -1;
       }
#endif	
#endif

    /* Disable async schedule. */
    cmd = ehci_readl(&hcor->or_usbcmd);
    cmd &= ~USBCMD_ASE;
    //ehci_writel(&hcor->or_usbcmd, cmd);
    hcor->or_usbcmd = cmd;

    ret = handshake((u32 *)&hcor->or_usbsts, USBSTS_ASS, 0,100 * 1000);
    if (ret < 0)
    {
        DEBUG_UHOST("EHCI fail timeout USBSTS_ASS reset\n");
        goto fail;
    }

    //qh_list->qh_link = cpu_to_hc32((u32)qh_addr | USB_QH_LINK_TYPE_QH);

    token = hc32_to_cpu(qh->qh_overlay.qtd_token);
    if (!(token & USB_QTD_TOKEN_STAT_ACTIVE))
    {
        //	DEBUG_UHOST("TOKEN=%#x\n", token);
        switch (token & 0xfc)
        {
            case 0:
                toggle = token >> 31;
                usb_settoggle(dev, usb_pipeendpoint(pipe),usb_pipeout(pipe), toggle);
                dev->status = 0;
                break;
            case USB_QTD_TOKEN_STAT_ACTIVE:
                dev->status = USB_ST_STALLED;
		  printf("\033[31mUSB_ST_STALLED\n\033[0m");	
                break;
            case 0xa0:
            case USB_QTD_TOKEN_STAT_DBE:
                dev->status = USB_ST_BUF_ERR;
		   printf("\033[31mUSB_ST_BUF_ERR\n\033[0m");				
                break;
            case USB_QTD_TOKEN_STAT_HALTED | USB_QTD_TOKEN_STAT_BABBLE:
            case USB_QTD_TOKEN_STAT_BABBLE:
		  printf("\033[31mUSB_QTD_TOKEN_STAT_BABBLE\n\033[0m");			
                dev->status = USB_ST_BABBLE_DET;
                break;
            default:
                dev->status = USB_ST_CRC_ERR;
                if ((token & USB_QTD_TOKEN_STAT_HALTED) == USB_QTD_TOKEN_STAT_HALTED)
                    dev->status |= USB_ST_STALLED;
		printf("\033[31mUSB_ST_CRC_ERR & USB_ST_STALLED\n\033[0m");			
                break;
        }
        dev->act_len = length - ((token >> 16) & 0x7fff);
    }
    else
    {
        dev->act_len = 0;
        DEBUG_STORAGE("token= %x dev=%u, usbsts=%#x, p[1]=%#x,\n",token,dev->devnum, ehci_readl(&hcor->or_usbsts),ehci_readl(&hcor->or_portsc));
	 DEBUG_STORAGE("#dev->status 0x%X \n",dev->status);	
    }

    OSSemPost(ehci_Atomic_SemEvt);

    return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:

    OSSemPost(ehci_Atomic_SemEvt);
    return -1;
}

#endif

static int min3(int a, int b, int c)
{

    if (b < a)
        a = b;
    if (c < a)
        a = c;
    return a;
}

extern u8 bus_reset;
void ehci_bus_reset(void)
{
    //u32 i;
    u32 *status_reg;
    u32 reg;
    status_reg = (u32 *)&hcor->or_portsc[0];

    DEBUG_UHOST("ehci_Bus Reset\n");


    bus_reset=1;
    //reg = ehci_readl(status_reg);
    reg = *status_reg;
    reg |= PORTSC_PR;

    //ehci_writel(status_reg, reg);
    *status_reg = reg;
   DEBUG_RED("ehci_Bus Reset\n");
   //ehci_mdelay(100);
    OSTimeDly(2);
   DEBUG_YELLOW("ehci_Bus Reset\n");
  // OSTimeDly(4);
    reg &= ~PORTSC_PR;
    //ehci_writel(status_reg, reg);
    *status_reg = reg;
	 OSTimeDly(2);
//DEBUG_RED("ehci_Bus Reset\n");
}

int ehci_submit_root(struct usb_device *dev, unsigned long pipe, void *buffer, int length, struct devrequest *req)
{
    u8 tmpbuf[4];
    u16 typeReq;
    void *srcptr = NULL;
    int len, srclen;
    u32 reg;
    u32 *status_reg,*W_status_reg;
    //int ret;

    if (le16_to_cpu(req->index) > CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS)
    {
        DEBUG_STORAGE("The request port(%d) is not configured\n", le16_to_cpu(req->index) - 1);
        return -1;
    }
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
     W_status_reg = (u32 *)&hcor->or_portsc[0];
     status_reg = (u32 *)&pre_port_status;
#else
    status_reg = (u32 *)&hcor->or_portsc[0];
#endif
    srclen = 0;

    DEBUG_UHOST("req=%u (%#x), type=%u (%#x), value=%u, index=%u\n",
                req->request, req->request, req->requesttype, req->requesttype,
                le16_to_cpu(req->value), le16_to_cpu(req->index));

    typeReq = req->request | req->requesttype << 8;

    switch (typeReq)
    {
        case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
            switch (le16_to_cpu(req->value) >> 8)
            {
                case USB_DT_DEVICE:
                    DEBUG_UHOST("USB_DT_DEVICE request\n");
                    srcptr = &descriptor.device;
                    srclen = 0x12;
                    break;
                case USB_DT_CONFIG:
                    DEBUG_UHOST("USB_DT_CONFIG config\n");
                    srcptr = &descriptor.config;
                    srclen = 0x19;
                    break;
                case USB_DT_STRING:
                    DEBUG_UHOST("USB_DT_STRING config\n");
                    switch (le16_to_cpu(req->value) & 0xff)
                    {
                        case 0:	/* Language */
                            srcptr = "\4\3\1\0";
                            srclen = 4;
                            break;
                        case 1:	/* Vendor */
                            srcptr = "\16\3M\0A\0R\0S\0";
                            srclen = 14;
                            break;
                        case 2:	/* Product */
                            srcptr = "\52\3E\0H\0C\0I\0 "
                                     "\0H\0o\0s\0t\0 "
                                     "\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
                            srclen = 42;
                            break;
                        default:
                            DEBUG_UHOST("unknown value DT_STRING %x\n",
                                        le16_to_cpu(req->value));
                            goto unknown;
                    }
                    break;
                default:
                    DEBUG_UHOST("unknown value %x\n", le16_to_cpu(req->value));
                    goto unknown;
            }
            break;
        case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
            switch (le16_to_cpu(req->value) >> 8)
            {
                case USB_DT_HUB:
                    DEBUG_UHOST("USB_DT_HUB config\n");
                    srcptr = &descriptor.hub;
                    srclen = 0x8;
                    break;
                default:
                    DEBUG_UHOST("unknown value %x\n", le16_to_cpu(req->value));
                    goto unknown;
            }
            break;
        case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
            DEBUG_UHOST("USB_REQ_SET_ADDRESS\n");
            rootdev = le16_to_cpu(req->value);
            break;
        case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
            DEBUG_UHOST("USB_REQ_SET_CONFIGURATION\n");
            /* Nothing to do */
            break;
        case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
            tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
            tmpbuf[1] = 0;
            srcptr = tmpbuf;
            srclen = 2;
            break;
        case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
            memset(tmpbuf, 0, 4);
            reg = ehci_readl(status_reg);
            if (reg & PORTSC_CS)
                tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
            if (reg & PORTSC_PE)
                tmpbuf[0] |= USB_PORT_STAT_ENABLE;
            if (reg & PORTSC_SUSP)
                tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
            if (reg & PORTSC_OCA)
                tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
            if (reg & PORTSC_PR)
                tmpbuf[0] |= USB_PORT_STAT_RESET;
            if (reg & PORTSC_PP)
                tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;


            tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;


            if (reg & PORTSC_CSC)
                tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
            if (reg & PORTSC_PEC)
                tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
            if (reg & PORTSC_OCC)
                tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
            if (portreset & (1 << le16_to_cpu(req->index)))
                tmpbuf[2] |= USB_PORT_STAT_C_RESET;

            srcptr = tmpbuf;
            srclen = 4;
            break;
        case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
            reg = ehci_readl(status_reg);
            reg &= ~PORTSC_CLEAR;
            switch (le16_to_cpu(req->value))
            {
                case USB_PORT_FEAT_ENABLE:
                    reg |= PORTSC_PE;
                    //ehci_writel(status_reg, reg);
                    *status_reg = reg;
                    break;
                case USB_PORT_FEAT_POWER:
#ifdef USB_UNDO
                    if (HCS_PPC(ehci_readl(&hccr->cr_hcsparams)))
                    {
                        reg |= PORTSC_PP;
                        //ehci_writel(status_reg, reg);
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test ) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX)|| (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
                          *W_status_reg = reg;
#else
                        *status_reg = reg;
#endif
                    }
#endif
                    break;
                case USB_PORT_FEAT_RESET:
#if 0



                    reg |= PORTSC_PR;
                    reg &= ~PORTSC_PE;
                    ehci_writel(status_reg, reg);


                    ehci_writel(status_reg, reg & ~PORTSC_PR);
                    /*
                     * A host controller must terminate the reset
                     * and stabilize the state of the port within
                     * 2 milliseconds
                     */
                    ret = handshake(status_reg, PORTSC_PR, 0,3 * 100000);
                    ehci_mdelay(2000);
                    if (!ret)
                        portreset |=
                            1 << le16_to_cpu(req->index);
                    else
                        DEBUG_UHOST("port(%d) reset error\n",
                                    le16_to_cpu(req->index) - 1);

#else
                    ehci_bus_reset();
                    portreset |=1 << le16_to_cpu(req->index);
#endif
                    break;
                default:
                    DEBUG_STORAGE("unknown feature %x\n", le16_to_cpu(req->value));
                    goto unknown;
            }
            /* unblock posted writes */
            (void) ehci_readl(&hcor->or_usbcmd);
            break;
        case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
            reg = ehci_readl(status_reg);
            switch (le16_to_cpu(req->value))
            {
                case USB_PORT_FEAT_ENABLE:
                    reg &= ~PORTSC_PE;
                    break;
                case USB_PORT_FEAT_C_ENABLE:
                    reg = (reg & ~PORTSC_CLEAR) | PORTSC_PE;
                    break;
                case USB_PORT_FEAT_POWER:
#ifdef USB_UNDO
                    if (HCS_PPC(ehci_readl(&hccr->cr_hcsparams)))
                        reg = reg & ~(PORTSC_CLEAR | PORTSC_PP);
#endif
                case USB_PORT_FEAT_C_CONNECTION:
                    reg = (reg & ~PORTSC_CLEAR) | PORTSC_CSC;
                    break;
                case USB_PORT_FEAT_OVER_CURRENT:
                    reg = (reg & ~PORTSC_CLEAR) | PORTSC_OCC;
                    break;
                case USB_PORT_FEAT_C_RESET:
                    portreset &= ~(1 << le16_to_cpu(req->index));
                    break;
                default:
                    DEBUG_STORAGE("unknown feature %x\n", le16_to_cpu(req->value));
                    goto unknown;
            }
            //ehci_writel(status_reg, reg);
#if( (SW_APPLICATION_OPTION== MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test) ||\
	(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
            *W_status_reg = reg;
#else
            *status_reg = reg;
#endif
            /* unblock posted write */
            (void) ehci_readl(&hcor->or_usbcmd);
            break;
        default:
            DEBUG_STORAGE("Unknown request\n");
            goto unknown;
    }

    ehci_mdelay(1);
    len = min3(srclen, le16_to_cpu(req->length), length);
    if (srcptr != NULL && len > 0)
        memcpy(buffer, srcptr, len);
    //else
    //DEBUG_UHOST("Len is 0\n");

    dev->act_len = len;
    dev->status = 0;
    return 0;

unknown:
    DEBUG_STORAGE("requesttype=%x, request=%x, value=%x, index=%x, length=%x\n",
                req->requesttype, req->request, le16_to_cpu(req->value),
                le16_to_cpu(req->index), le16_to_cpu(req->length));

    dev->act_len = 0;
    dev->status = USB_ST_STALLED;
    return -1;
}

int usb_lowlevel_stop(void)
{
    return ehci_hcd_stop();
}

#define DISP_LINE_LEN2 16

void format_printf2(unsigned long offset,unsigned long length,unsigned int buf)
{
    unsigned int	addr;
    unsigned long	i,linebytes,nbytes,eeprom_offset;
    unsigned char	*cp;
    int	size;  //Byte order
    char	linebuf[DISP_LINE_LEN2];

    size=4;
    addr=buf;
    eeprom_offset=offset;

    nbytes = length * size;

    do
    {
        //unsigned int	*uip = (unsigned int   *)linebuf;
        //unsigned short	*usp = (unsigned short *)linebuf;
        //unsigned char	*ucp = (unsigned char *)linebuf;
        DEBUG_UHOST("%04lx:", eeprom_offset);
        linebytes = (nbytes>DISP_LINE_LEN2)?DISP_LINE_LEN2:nbytes;

        for (i=0; i<linebytes; i+= size)
        {
            if (size == 4)
            {
                DEBUG_UHOST(" %08x", *((unsigned int *)addr)); //(*uip++ = *((unsigned int *)addr)));
            }
            else if (size == 2)
            {
                DEBUG_UHOST(" %04x", *((unsigned int *)addr)); //(*usp++ = *((unsigned short *)addr)));
            }
            else
            {
                DEBUG_UHOST(" %02x", *((unsigned int *)addr)); //(*ucp++ = *((unsigned char *)addr)));
            }
            addr += size;
            eeprom_offset += size;
        }

        DEBUG_UHOST ("    ");
        cp = (unsigned char *)linebuf;
        for (i=0; i<linebytes; i++)
        {
            if ((*cp < 0x20) || (*cp > 0x7e))
                DEBUG_UHOST(".");
            else
                DEBUG_UHOST("%c", *cp);
            cp++;
        }
        DEBUG_UHOST ("\n");
        nbytes -= linebytes;

    }
    while (nbytes > 0);
}
u32 periodic_schedule_init(u32 frame_list_size)
{
    u32 i;
    //u32 malloc_addr;

    /* Disable the asynchronous schedule */
    //    MCF_USB_USBCMD(port) &= ~MCF_USB_USBCMD_ASE;
#if 0
    /* Initialize the USBCMD register for the desired size of the frame list */
    switch (frame_list_size)
    {
        case 1024:
            HCUSBCMD |= USBCMD_FS_1024;
            break;
        case 512:
            HCUSBCMD |= USBCMD_FS_512;
            break;
        case 256:
            HCUSBCMD |= USBCMD_FS_256;
            break;
        default:
            DEBUG_UHOST("ERR!! Invalid frame list size\n");
            HCUSBCMD |= USBCMD_FS_256; /* Use the min size by default */
            break;
    }

#endif

    /* Fill the frame list with link pointers marked as invalid
     * since we don't have any traffic to send yet.
     */
    for ( i=0; i<(frame_list_size); i=i++)
        *(u32 *)(periodic_base+i) = 1;

    /* Initialize the Periodic base address register */
    HCPeriodicListBS = (unsigned long) periodic_base;

    //format_printf2(0, 256, (unsigned int )periodic_base);
    /* Enable the periodic schedule */
    HCUSBCMD |= USBCMD_PSE;

    DEBUG_UHOST("Periodic schedule is ");
    /* Wait for periodic schedule to enable */
    while (!(HCUSBSTS & USBSTS_PSS));


    DEBUG_UHOST("enabled.\n");

    return 0;
}


__align(4) char hub_buffer[8];
__align(4) char hid_buf_mouse[128];
__align(4) char hid_buf_keyboard[128];

#define OUT_PID  0
#define IN_PID  1
#define SETUP_PID  2


u32 hub_qtd_init(u32 trans_sz, u32 ioc)
{
    struct qTD *init_qtd =(struct qTD *) hub_qtd ;
    u32 token;
    //u32 malloc_addr;
    /*
    * The USB requires qTDs to be aligned on a 64 byte boundary.
    * In order to accomplish this, the data is over-allocated and
    * adjusted.
    */


    init_qtd->qtd_next = 0xDEAD0001;
    init_qtd->qtd_altnext = 0x1;
    token = (USB_QTD_TOKEN_DT(1) | USB_QTD_TOKEN_PID_IN);

    //token |= USB_QTD_TOKEN_IOC; //@POLL

    init_qtd->qtd_token = (token
                           | USB_QTD_TOKEN_TRANS_SIZE(trans_sz)
                           | USB_QTD_TOKEN_CERR(0x3)
                           | USB_QTD_TOKEN_STAT_ACTIVE );

    init_qtd->qtd_buffer[0] = (u32) &hub_buffer[0];
    init_qtd->qtd_buffer[1]  = 0;
    init_qtd->qtd_buffer[2]  = 0;
    init_qtd->qtd_buffer[3]  = 0;
    init_qtd->qtd_buffer[4]  = 0;

    return 0;
}

u32 hid_qtd_init(struct usb_interface_descriptor *if_desc, u8 *qtd_addr, u32 trans_sz, u32 ioc)
{
    struct qTD *init_qtd =(struct qTD *) qtd_addr ;
    u32 token;
    //u32 malloc_addr;
    /*
    * The USB requires qTDs to be aligned on a 64 byte boundary.
    * In order to accomplish this, the data is over-allocated and
    * adjusted.
    */

    init_qtd->qtd_next = 0xDEAD0001;
    init_qtd->qtd_altnext = 0x1;
    token =  (USB_QTD_TOKEN_DT(1) | USB_QTD_TOKEN_PID_IN);

    //token |= USB_QTD_TOKEN_IOC; //@POLL

    init_qtd->qtd_token = (token
                           | USB_QTD_TOKEN_TRANS_SIZE(trans_sz)
                           | USB_QTD_TOKEN_CERR(0x3)
                           | USB_QTD_TOKEN_STAT_ACTIVE );

    if(if_desc->bInterfaceProtocol == USB_PROT_HID_KEYBOARD)
        init_qtd->qtd_buffer[0] = (u32) &hid_buf_keyboard[0];
    else if (if_desc->bInterfaceProtocol == USB_PROT_HID_MOUSE)
        init_qtd->qtd_buffer[0] = (u32) &hid_buf_mouse[0];
    init_qtd->qtd_buffer[1]  = 0;
    init_qtd->qtd_buffer[2]  = 0;
    init_qtd->qtd_buffer[3]  = 0;
    init_qtd->qtd_buffer[4]  = 0;

    return 0;
}

u32 hub_qh_init(u32 max_packet, u32 epnum, u32 dev_addr, u32 smask)
{
    struct QH* hub_qh	= (struct QH*) hub_qh_addr;
    //u32 token;
    //u32 malloc_addr;
    int i;
    /*
    * The USB requires queue heads to be aligned on a 64 byte boundary.
    * In order to accomplish this, the data is over-allocated and
    * adjusted.
    */
    memset(hub_qh, 0, sizeof(struct QH));

    hub_qh->qh_link = cpu_to_hc32((u32)hub_qh | USB_QH_LINK_TYPE_QH);
    hub_qh->qh_endpt1 = cpu_to_hc32( (USB_SPEED_HIGH << 12));
    hub_qh->qh_endpt1 |=   	(USB_QH_EP_CHAR_MAX_PACKET(max_packet)
                             | USB_QH_EP_CHAR_DTC
                             | USB_QH_EP_CHAR_EP(epnum)
                             | USB_QH_EP_CHAR_DEV_ADDR(dev_addr) );
    hub_qh->qh_endpt2 =  (USB_QH_EP_CAP_MULT_ONE | USB_QH_EP_CAP_UFRAME_SMASK(smask));
    hub_qh->qh_curtd	= 0;
    hub_qh->qh_overlay.qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    hub_qh->qh_overlay.qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    hub_qh->qh_overlay.qtd_token = 0;

    /*Invidiate QH, will add mouse qh later*/
    hub_qh->qh_link |= USB_QH_LINK_PTR_T;

    for ( i=0; i<(USB_DEFAULT_FLS_SIZE); i=i+ POLL_HUB_INTERVAL)
    {
        *(u32 *)(periodic_base+i) =  (u32)hub_qh + 0x002;
    }

    hub_qtd_init(max_packet,1);
    hub_qh->qh_overlay.qtd_next = (u32) hub_qtd;
    DEBUG_UHOST("**************************\n"\
                "hub_qh->[attribute]: value\n"\
                "**************************\n"\
                "qh_link: 0x%08x\n"\
                "qh_endpt1: 0x%08x\n"\
                "qh_endpt2: 0x%08x\n"\
                "qh_curtd: 0x%08x\n"\
                "qh_overlay.qtd_next: 0x%08x\n"\
                "qh_overlay.qtd_altnext: 0x%08x\n"\
                "qh_overlay.qtd_token: 0x%08x\n"\
                "qh_overlay.qtd_buffer: %s\n"\
                "**************************\n"
                , hub_qh->qh_link
                , hub_qh->qh_endpt1
                , hub_qh->qh_endpt2
                , hub_qh->qh_curtd
                , hub_qh->qh_overlay.qtd_next
                , hub_qh->qh_overlay.qtd_altnext
                , hub_qh->qh_overlay.qtd_token
                , hub_qh->qh_overlay.qtd_buffer);
    return 0;
}

u32 hid_qh_init(struct usb_device *dev, struct usb_endpoint_descriptor *ep, u8 *qh_addr, u8 *qtd_addr, u32 smask)
{
    struct QH* hid_qh = (struct QH*) qh_addr;
    //u32 token;
    //u32 malloc_addr;
    int i;
    /*
    * The USB requires queue heads to be aligned on a 64 byte boundary.
    * In order to accomplish this, the data is over-allocated and adjusted.
    */
    memset(hid_qh, 0, sizeof(struct QH));

    hid_qh->qh_link = cpu_to_hc32((u32)hid_qh | USB_QH_LINK_TYPE_QH);
    hid_qh->qh_endpt1 = cpu_to_hc32( (dev->speed << 12));
    hid_qh->qh_endpt1 |=   	(USB_QH_EP_CHAR_MAX_PACKET(ep->wMaxPacketSize)
                             | USB_QH_EP_CHAR_DTC
                             | USB_QH_EP_CHAR_EP(ep->bEndpointAddress)
                             | USB_QH_EP_CHAR_DEV_ADDR(dev->devnum) );
    hid_qh->qh_endpt2 =  (USB_QH_EP_CAP_MULT_ONE | USB_QH_EP_CAP_UFRAME_SMASK(smask));
    hid_qh->qh_curtd	= 0;
    hid_qh->qh_overlay.qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    hid_qh->qh_overlay.qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    hid_qh->qh_overlay.qtd_token = 0;

    if(hub_running)
    {
        hid_qh->qh_endpt2 |= ((ghub_port<<23) |(ghub_addr<<16) |USB_QH_EP_CAP_UFRAME_CMASK(0x04));
    }
    /*Invidiate QH, will add mouse qh later*/
    hid_qh->qh_link |= USB_QH_LINK_PTR_T;

    for ( i=0; i<(USB_DEFAULT_FLS_SIZE); i=i+ POLL_HID_INTERVAL)
    {
        *(u32 *)(periodic_base +i + dev->devnum) =  (u32)hid_qh + 0x002;
    }

    hid_qtd_init(&dev->config.if_desc[0].desc, qtd_addr, ep->wMaxPacketSize,1);
    hid_qh->qh_overlay.qtd_next = (u32) qtd_addr;

    DEBUG_UHOST("****************************\n"\
                "mouse_qh->[attribute]: value\n"\
                "****************************\n"\
                "qh_link: 0x%08x\n"\
                "qh_endpt1: 0x%08x\n"\
                "qh_endpt2: 0x%08x\n"\
                "qh_curtd: 0x%08x\n"\
                "qh_overlay.qtd_next: 0x%08x\n"\
                "qh_overlay.qtd_altnext: 0x%08x\n"\
                "qh_overlay.qtd_token: 0x%08x\n"\
                "qh_overlay.qtd_buffer: %s\n"\
                "****************************\n"
                , hid_qh->qh_link
                , hid_qh->qh_endpt1
                , hid_qh->qh_endpt2
                , hid_qh->qh_curtd
                , hid_qh->qh_overlay.qtd_next
                , hid_qh->qh_overlay.qtd_altnext
                , hid_qh->qh_overlay.qtd_token
                , hid_qh->qh_overlay.qtd_buffer);

    return 0;
}

int usb_lowlevel_init(void)
{
    struct QH* qh_list_initial = (struct QH*) qh_list_addr;
    u32 reg, cmd;

    /* Set head of reclaim list */
    memset(qh_list_initial, 0, sizeof(struct QH));
    //qh_list_initial->qh_link = cpu_to_hc32((u32)qh_list | USB_QH_LINK_TYPE_QH);
    // 1 << 15 make a queue head being the head of reclamation
    qh_list_initial->qh_link = cpu_to_hc32((u32)qh_addr | USB_QH_LINK_TYPE_QH);
    qh_list_initial->qh_endpt1 = cpu_to_hc32(USB_QH_EP_CHAR_H | USB_QH_EP_CHAR_EPS(USB_SPEED_HIGH));
    qh_list_initial->qh_curtd = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    qh_list_initial->qh_overlay.qtd_next = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    qh_list_initial->qh_overlay.qtd_altnext = cpu_to_hc32(USB_QTD_LINK_PTR_T);
    qh_list_initial->qh_overlay.qtd_token = cpu_to_hc32(USB_QTD_TOKEN_STAT_HALTED);

    /* Set async(HCAsyncListAddr). queue head pointer. */
    //ehci_writel(&hcor->or_asynclistaddr, (u32)qh_addr);
    hcor->or_asynclistaddr = (u32)qh_addr;

    reg = ehci_readl(&hccr->cr_hcsparams);
    descriptor.hub.bNbrPorts = HCS_N_PORTS(reg);
    DEBUG_UHOST("EhCI:Register %x NbrPorts %d\n", reg, descriptor.hub.bNbrPorts);
    /* Port Indicators */
    //if (HCS_INDICATOR(reg))
    //	descriptor.hub.wHubCharacteristics |= 0x80;
    /* Port Power Control */
    //if (HCS_PPC(reg))
    //descriptor.hub.wHubCharacteristics |= 0x01;

    /* Start the host controller. */
    cmd = ehci_readl(&hcor->or_usbcmd);
    /*
     * Philips, Intel, and maybe others need USBCMD_RUN before the
     * root hub will detect new devices (why?); NEC doesn't
     */
    cmd &= ~(USBCMD_LRESET|USBCMD_IAAD|USBCMD_PSE|USBCMD_ASE|USBCMD_RESET);
    cmd |= USBCMD_RUN;
    //ehci_writel(&hcor->or_usbcmd, cmd);
    hcor->or_usbcmd = cmd;
    periodic_schedule_init(USB_DEFAULT_FLS_SIZE);
    DEBUG_UHOST("CMD at usb_lowlevel_init: %x\n",ehci_readl(&hcor->or_usbcmd));



    /* unblock posted write */
    cmd = ehci_readl(&hcor->or_usbcmd);
    ehci_mdelay(5);
    reg = HC_VERSION(ehci_readl(&hccr->cr_capbase));
    DEBUG_UHOST("USB EHCI %x.%02x\n", reg >> 8, reg & 0xff);

    rootdev = 0;

    return 0;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length)
{

    if (usb_pipetype(pipe) != PIPE_BULK)
    {
        DEBUG_STORAGE("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
        return -1;
    }
    return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length, struct devrequest *setup)
{

    if (usb_pipetype(pipe) != PIPE_CONTROL)
    {
        DEBUG_STORAGE("@non-control pipe (type=%lu)", usb_pipetype(pipe));
        return -1;
    }
    /*devnum*/
    if (usb_pipedevice(pipe) == rootdev)
    {
        if (rootdev == 0)
            dev->speed = USB_SPEED_HIGH;
        return ehci_submit_root(dev, pipe, buffer, length, setup);
    }
    // buffer is the buffer where store device decriptor from device
    return ehci_submit_async(dev, pipe, buffer, length, setup);
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length, int interval)
{

    DEBUG_UHOST("dev=%p, pipe=%lu, buffer=%p, length=%d, interval=%d", dev, pipe, buffer, length, interval);
    return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

#endif
