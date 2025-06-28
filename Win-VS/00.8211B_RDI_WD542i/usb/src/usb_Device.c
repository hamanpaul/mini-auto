/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbOTG_Device.c

Abstract:

   	USB routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2011/10/17	Griffy Liu	Create

*/


#include "general.h"
#include "board.h"
#include "task.h"
#include "usb.h"
#include "usbreg.h"
#include "usbOTGreg.h"
#include "usbapievt.h"
#include "usbHostintevt.h"
#include "usbapi.h"
#include "gpioapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "sysapi.h"

#include "usb_Device.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
#if (USB_DEVICE == 1)
void usbOTGReset(void);
void USB_Delay(u32);
void usbDeviceTask(void*);
s32 usbHostResourceInit(void);

s32 usbDeviceSetIntEvt(u8);
s32 usbDeviceGetIntEvt(u8*);
s32 usbDeviceInit(void);


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbDebugPrint 			DEBUG_UHOST

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK usbDeviceTaskStack[USB_DEVICE_TASK_STACK_SIZE]; /* Stack of task usbTask() */

OS_EVENT* usbDeviceSemEvt; /* semaphore to synchronize event processing */
//OS_EVENT* usbSemEp[USB_HOST_SEM_EP_MAX]; /* semaphore to synchronize endpoint access */

USB_INT_EVT usbDeviceIntEvt; /* Interrupt event queue */
static u8 device_addrss = 0 ;

u8 *ptr ;
extern void (*usbHostIntEvtFunc[])(void);
extern void (*usbApiEvtFunc[])(void);

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

void USB_Delay(u32 dly)
{
    u32 loop ;
    for(loop = 0 ; loop < dly ; loop++);
}

void Device_Descriptor(void)
{
    USB_DEVICE_DES des ;
    
    //printf("Device Descriptor %x\n", usb_device_buf);
    des.bLength = 18 ;
    des.bDescriptorType = 0x01 ;
    des.bcdUSB = 0x0200 ;
    des.bDeviceClass = 0 ;
    des.bDeviceSubClass = 0 ;
    des.bDeviceProtocol = 0 ;
    des.bMaxPacketSize0 = 64 ;
    des.idVendor = 0x0930 ;
    des.idProduct = 0x6544 ;
    des.bcdDevice = 0x0100 ;
    des.iManufacturer = 0x01 ;
    des.iProduct = 0x02 ;
    des.iSerialNumber = 0x03 ;
    des.bNumConfiguration = 0x01 ;
    memcpy(usb_device_buf, &des, 18);
  
    DeviceDMACtlParam1 = ((18<<8)|(1<<1));
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
    //printf("Device Descriptor done\n");
}

void Configuration_Descriptor(u16 wLength)
{
    USB_CONFIGURATION_DES config_des ;
    USB_INTERFACE_DES     interface_des ; 
    USB_ENDPOINT_DES      endpoint1_des, endpoint2_des ;  
    
    //printf("Configuration Descriptor %x\n", wLength);
    config_des.bLength = 0x09 ;
    config_des.bDescriptorType = 0x02 ;
    config_des.wTotalLength = 0x0020 ;
    config_des.bNumInterface = 0x01 ;
    config_des.bConfigValue = 0x01 ;
    config_des.iConfiguration = 0x00 ;
    config_des.bmAttribute = 0x80 ;
    config_des.bMaxPower = 0x32 ;

    interface_des.bLength = 0x09 ;
    interface_des.bDescriptorType = 0x04 ;
    interface_des.bInterfaceNum = 0x00 ;
    interface_des.bAlternateSetting = 0x00 ;
    interface_des.bNumEndpoints = 0x02 ;
    interface_des.bInterfaceClass = 0x08 ;
    interface_des.bInterfaceSubClass = 0x06 ;
    interface_des.bInterfaceProtocol = 0x50 ;
    interface_des.iInterface = 0x00 ;

    endpoint1_des.bLength = 0x07 ;
    endpoint1_des.bDescriptorType = 0x05 ;
    endpoint1_des.bEndpointAddress = 0x81 ;
    endpoint1_des.bmAttribute = 0x02 ;
    endpoint1_des.wMaxPacketSize = 0x0200 ;
    endpoint1_des.bInterval = 0xff ;

    endpoint2_des.bLength = 0x07 ;
    endpoint2_des.bDescriptorType = 0x05 ;
    endpoint2_des.bEndpointAddress = 0x02 ;
    endpoint2_des.bmAttribute = 0x02 ;
    endpoint2_des.wMaxPacketSize = 0x0200 ;
    endpoint2_des.bInterval = 0xff ;

    if(wLength == 0x09)
    {
        //printf("1 \n");
        memcpy(usb_device_buf, &config_des, 9); 
        DeviceDMACtlParam1 = ((9<<8)|(1<<1));
        DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
        DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
        DeviceDMACtlParam1 |= BIT0 ;    //Start
        while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
        DeviceInterruptSourceG2 &= ~BIT7 ;
        DeviceDMATargetFIFONum = 0x00000000 ;
        DeviceCXConfigFIFOEmpS |= BIT0 ;
    }
    else
    {
        //printf("2 \n");
        memcpy(usb_device_buf, &config_des, 9);
        memcpy((usb_device_buf+9), &interface_des, 9);
        memcpy((usb_device_buf+18), &endpoint1_des, 7);
        memcpy((usb_device_buf+25), &endpoint2_des, 7);
        DeviceDMACtlParam1 = ((32<<8)|(1<<1));
        DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
        DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
        DeviceDMACtlParam1 |= BIT0 ;    //Start
        while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
        DeviceInterruptSourceG2 &= ~BIT7 ;
        DeviceDMATargetFIFONum = 0x00000000 ;
        DeviceCXConfigFIFOEmpS |= BIT0 ;
    }
    //printf("Configuration Descriptor done\n");
}

void Language_Descriptor(u16 wLength)
{
    USB_LANGUAGE_DES lang_des ;

    //printf("Language Descriptor\n");
    lang_des.bLength = 0x04 ;
    lang_des.bDescriptorType = 0x03 ;
    lang_des.wLangID = 0x0409 ;

    if(wLength == 2)
    {
        memcpy(usb_device_buf, &lang_des, 2);
        DeviceDMACtlParam1 = ((2<<8)|(1<<1));
    }
    else
    {
        memcpy(usb_device_buf, &lang_des, 4);
        DeviceDMACtlParam1 = ((4<<8)|(1<<1));
    }
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
    //printf("Language Descriptor done\n");
}

void Product_Descriptor(void)
{
    USB_PRODUCT_DES product_des ;

    //printf("Language Descriptor\n");
    // Content is "I love SNSD"
    product_des.bLength = 0x18 ;
    product_des.bDescriptorType = 0x03 ;
    product_des.Content0 = 0x00200049 ;
    product_des.Content1 = 0x006f006c ;
    product_des.Content2 = 0x00650076 ;
    product_des.Content3 = 0x00530020 ;
    product_des.Content4 = 0x0053004e ;
    product_des.Content5 = 0x00200044 ;

    memcpy(usb_device_buf, &product_des, 24);
  
    DeviceDMACtlParam1 = ((24<<8)|(1<<1));
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
    //printf("Language Descriptor done\n");
}


void Serial_Descriptor(u16 wLength)
{
    USB_SERIAL_DES serial_des ;

    //printf("Serial Descriptor\n");
    if(wLength == 2)
        serial_des.bLength = 0x02 ;
    else
        serial_des.bLength = 0x32 ;
    serial_des.bDescriptorType = 0x03 ;
    serial_des.Content0 = 0x00300030 ;
    serial_des.Content1 = 0x00300031 ;
    serial_des.Content2 = 0x00300030 ;
    serial_des.Content3 = 0x00300030 ;
    serial_des.Content4 = 0x00300030 ;
    serial_des.Content5 = 0x00300030 ;
    serial_des.Content6 = 0x00300030 ;
    serial_des.Content7 = 0x00300030 ;
    serial_des.Content8 = 0x00300030 ;
    serial_des.Content9 = 0x00300030 ;
    serial_des.Content10 = 0x00320030 ;
    serial_des.Content11 = 0x00350030 ;

    if(wLength == 2)
    {
        memcpy(usb_device_buf, &serial_des, 2);
        DeviceDMACtlParam1 = ((2<<8)|(1<<1));
    }
    else
    {
        memcpy(usb_device_buf, &serial_des, 50);
        DeviceDMACtlParam1 = ((50<<8)|(1<<1));
    }
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
    //printf("Serial Descriptor done\n");
}


void Get_Descriptor(u16 wValue, u16 wLength)
{
    
    
    u32 tmp ;

    //printf("wValue %x\n", wValue);
    switch(wValue)
    {
        case 0x0100:    //Device
            Device_Descriptor();
            break;
        case 0x0200:
            Configuration_Descriptor(wLength);
            break;
        case 0x0300:
            Language_Descriptor(wLength); 
            break;
        case 0x0302:
            Product_Descriptor();    
            break;
        case 0x0303:
            Serial_Descriptor(wLength);    
            break;
        default :
            printf("wValue %x\n", wValue);
            break;    
    }
}

void Set_Address(u16 wValue)
{
    //printf("Set Address %x\n", wValue);
    DeviceAddress = wValue ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
}

void Set_Configuration()
{
    //printf("Set Configuration\n");
    DeviceAddress |= BIT7 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
}

void Get_Max_Lun()
{
    u8 *ptr ;
    
    printf("Get Max Lun\n");

    ptr = usb_device_buf ;
    *ptr = 0x00 ; 
    DeviceDMACtlParam1 = ((1<<8)|(1<<1));
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    DeviceCXConfigFIFOEmpS |= BIT0 ;
}

void RxSetupCommand(void)
{
    u32 cnt = 0 ;
    u32 setup_c0, setup_c1 ;
    
    //printf("Read Setup command\n");
    while((DeviceInterruptSourceG0 & 0x00000001) == 0x00000000)
    {
        cnt++ ;
    }
    //printf("cnt = %d\n", cnt);
    DeviceDMATargetFIFONum |= 0x00000010 ;  // Access Control transfer FIFO
    setup_c0 = DeviceDMACtlParam3 ;
    setup_c1 = DeviceDMACtlParam3 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    //printf("0 Setup Command : %x\n", setup_c0);
    //printf("1 Setup Command : %x\n", setup_c1);

    if((setup_c0 & 0x0000ff00) == 0x00000600)
    {
        //printf("Get_Descriptor !!\n");
        Get_Descriptor(((setup_c0 & 0xffff0000)>>16), ((setup_c1 & 0xffff0000)>>16));
    }
    else if((setup_c0 & 0x0000ff00) == 0x00000500)
        Set_Address(((setup_c0 & 0xffff0000)>>16));
    else if((setup_c0 & 0x0000ff00) == 0x00000900)
        Set_Configuration();
    else if((setup_c0 & 0x0000ffff) == 0x0000fea1)
        Get_Max_Lun();
}

void Bulk_In_Transfer(void)
{
    USB_INQUIRY_RESPONSE inq_res ;
    
    printf("# Bulk In\n");
    inq_res.DeviceType = 0x00 ;
    inq_res.Removable = 0x80 ;
    inq_res.Version = 0x02 ;
    inq_res.Response_Format = 0x02 ;
    inq_res.Additional_Length = 0x1f ;
    inq_res.Reserve_0 = 0x00 ;
    inq_res.Reserve_1 = 0x00 ;
    inq_res.Reserve_2 = 0x00 ;
    inq_res.Vendor_Info0 = 0x66697247 ;
    inq_res.Vendor_Info1 = 0x20207966 ;
    inq_res.Product_Info0 = 0x656b6f50 ;
    inq_res.Product_Info1 = 0x72756f59 ;
    inq_res.Product_Info2 = 0x68737341 ; 
    inq_res.Product_Info3 = 0x20656c6f ;
    inq_res.Product_Revision = 0x30302e31 ;

    memcpy(usb_device_buf, &inq_res, 36);
    DeviceDMACtlParam1 = ((36<<8)|(1<<1));
    DeviceDMATargetFIFONum = 0x00000001 ;  // Access FIFO0
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    printf("# Bulk S\n");
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    printf("# Bulk F\n");
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;

    printf("# Bulk In Done\n");
}

void Bulk_Out_Transfer(void)
{
    u32 len, i ;
    u8 *ptr ;

    //printf("# Bulk Out\n");
    len = DeviceFIFO2ByteCnt & 0x000007ff ;
    DeviceDMACtlParam1 = (len<<8);
    DeviceDMATargetFIFONum = 0x00000004 ;  // Access FIFO2
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    //DeviceCXConfigFIFOEmpS |= BIT0 ;
    /*
    ptr = usb_device_buf;
    for(i=0 ; i<36 ;i++)
    {
        printf("data %x\n", *ptr);
        ptr++ ;
    }
    */
    Bulk_In_Transfer();
}


/*

Routine Description:

	Initialize USB.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usb_DeviceInit(void)
{

    DEBUG_UHOST("Device Start !!!\n");
    /* Create the semaphore */
    usbDeviceSemEvt = OSSemCreate(0);
    memset((void *)&usbDeviceIntEvt, 0, sizeof(USB_INT_EVT));
    // Start Device Task
    OSTaskCreate(USB_DEVICE_TASK, USB_DEVICE_TASK_PARAMETER, USB_DEVICE_TASK_STACK, USB_DEVICE_TASK_PRIORITY);
    usbDeviceMSCInit();
    DeviceMaskInterrupt = 0x00000007 ;  // Disable Interrupt mask for G0~G2
    DeviceMaskInterrupt &= 0xfffffff8 ; // Enable Interrupt mask for G0 and G2
    DeviceMaskInterruptG0 = 0xfffffffe; // Enable Setup Interrupt mask
    DeviceMaskInterruptG1 = 0x000e00cf; // 
    DeviceMaskInterruptG2 = 0xfffffffe; // Enable Bus reset Interrupt mask
    DeviceEP1to4Map = 0x33332330 ;
    DeviceFIFOMap = 0x0f020f11 ;
    DeviceFIFOConfig = 0x06260626 ;
    DeviceInEP1MaxPktSize = 0x00000200 ;
    DeviceOutEP2MaxPktSize = 0x00000200 ;
    DeviceMainCtl = 0x00000026 ;        // Chip Enable & Global Interrupt Enable
//    DeviceMainCtl = 0x00000024 ;        // Chip Enable & Global Interrupt Enable


    DevicePHYTestMode = 0x00000000 ;    // PHY drive D+	
    DEBUG_UHOST("usb_DeviceInit done !!!\n");
    DEBUG_UHOST("Device buff addr %x\n", usb_device_buf);

    return 1;
}

/*

Routine Description:

	The USB driver task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
void usbDeviceTask(void* pData)
{
    u8 err;
    u8 cause;

    while (1)
    {
        if (usbDeviceGetIntEvt(&cause))
        {
            //(*usbHostIntEvtFunc[cause])();
        }
        /*
        else if (usbHostGetApiEvt(&usbApiCurEvt))
        {
            (*usbApiEvtFunc[usbApiCurEvt.cause])();
        }
        */
        else
        {
            OSSemPend(usbDeviceSemEvt, OS_IPC_WAIT_FOREVER, &err);
            //printf("# usbHostSemEvt\n");
            if (err != OS_NO_ERR)
            {
                DEBUG_USB("Error: usbSemEvt is %d.\n", err);
                //return ;
            }
        }
    }
}

/*

Routine Description:

	The IRQ handler of USB.

Arguments:

	None.

Return Value:

	None.

*/
void usbDeviceIntHandler(void)
{
    u32 Interrupt_Src ;
    u32 Interrupt_G0_Src, Interrupt_G1_Src, Interrupt_G2_Src ;
    
    Interrupt_Src = DeviceInterruptGP ;

    // Group 0
    if(Interrupt_Src & BIT0)
    {
        //printf("G0 \n");
        DeviceMaskInterrupt |= BIT0 ;
        Interrupt_G0_Src = DeviceInterruptSourceG0 ;
        if(Interrupt_G0_Src & BIT0)
        {
            RxSetupCommand();
        }
        
        DeviceMaskInterrupt &= ~BIT0 ;
    }

    // Group 1
    if(Interrupt_Src & BIT1)
    {
        //printf("G1 \n");
        DeviceMaskInterrupt |= BIT1 ;
        //GLOBALInterruptMask |= BIT0 ;
        Interrupt_G1_Src = DeviceInterruptSourceG1 ;
        if(Interrupt_G1_Src & BIT4)
        {
            //DeviceMaskInterrupt = 0x00000007 ; 
            GLOBALInterruptMask |= BIT0 ;
            //printf("S \n");
            usbDeviceMSCSetEvt(USB_DEVICE_MSC_EVT_BULK_OUT);
            //Bulk_Out_Transfer();
        }
        /*
        if(Interrupt_G1_Src & BIT4)
        {
            printf("# short\n");
        }
        */
        DeviceMaskInterrupt &= ~BIT1 ;
    }

    // Group 2
    if(Interrupt_Src & BIT2)
    {
        //printf("G2 \n");
        Interrupt_G2_Src = DeviceInterruptSourceG2 ;
        if(Interrupt_G2_Src & BIT0)
        {
            DEBUG_UHOST("Bus reset \n");
            //DeviceInterruptSourceG2 &= ~BIT0 ;
            DeviceInterruptSourceG2 = BIT0 ;
        }
    }
}

/*

Routine Description:

	Set interrupt event.

Arguments:

	cause - Cause of the event to set.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDeviceSetIntEvt(u8 cause)
{
    /* check if cause is valid */
    if (cause >= USB_HOST_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    /* set the cause */
    usbDeviceIntEvt.cause[usbDeviceIntEvt.idxSet++] = cause;

    if (usbDeviceIntEvt.idxSet == USB_HOST_MAX_INT_EVT)
    {	/* wrap around the index */
        usbDeviceIntEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbDeviceIntEvt.idxSet == usbDeviceIntEvt.idxGet)
    {
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    DEBUG_UHOST("# USB post %d\n", cause);
    OSSemPost(usbDeviceSemEvt);

    return 1;
}

/*

Routine Description:

	Get interrupt event.

Arguments:

	pCause - Cause of the event got.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDeviceGetIntEvt(u8* pCause)
{
    /* check if event queue is empty */
    if (usbDeviceIntEvt.idxGet == usbDeviceIntEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pCause = usbDeviceIntEvt.cause[usbDeviceIntEvt.idxGet++];

    if (usbDeviceIntEvt.idxGet == USB_HOST_MAX_INT_EVT)
    {	/* wrap around the index */
        usbDeviceIntEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= USB_HOST_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    return 1;
}

#endif

