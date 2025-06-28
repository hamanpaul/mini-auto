/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbHost_msc.c

Abstract:

   	USB Mass Storage Class routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2010/12/17	Griffy Liu	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb_Device.h"
//#include "usbHost_MSC.h"
//#include "usbOTGreg.h"


#if (USB_DEVICE == 1)
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


s32 usbDeviceMSCSetEvt(u8);
s32 usbDeviceMSCGetEvt(u8*);
void USB_Device_Mass_Bulk_Out(void);
void USB_Device_Mass_Bulk_In(void);
void USB_Device_Mass_Storage_Init(void);
void usbDeviceMSCTask(void*);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK usbDeviceMscTaskStack[USB_DEVICE_MSC_TASK_STACK_SIZE]; /* Stack of task usbVcTask() */

OS_EVENT *USBDevice_Mass_Storage_Start_Evt;
OS_EVENT *USBDevice_Mass_Storage_Bulk_Trans_Done_Evt;
OS_EVENT *USBDevice_Mass_Storage_Command_Finish_Evt;

static u32 read_addr ;
static u32 write_addr ;
static u16 rw_blk_num ;

static u32 Mass_Storage_Tag ;
static u32 SD_Sec_Num = 0 ;
static u32 SD_Logical_Addr = 0 ;

USB_INT_EVT usbDeviceMSCEvt; /* Interrupt event queue */

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/* Interrupt event function */
void (*usbDeviceMSCIntEvtFunc[])(void) =
    {
        USB_Device_Mass_Bulk_Out,		    // 0x00  - Mass Storage init 
        USB_Device_Mass_Bulk_In,          // 0x04     
    };


void Enable_Device_Interrupt(void)
{
    GLOBALInterruptMask &= ~BIT0 ;
    //printf("GIM %x\n", GLOBALInterruptMask);
}

void Disable_Device_Interrupt(void)
{
    GLOBALInterruptMask |= BIT0 ;
}

void Set_MSC_SD_Sec_Num(u32 num)
{
    SD_Sec_Num = ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | ((num & 0x000000ff) << 24) ;
    num = num - 1 ;
    SD_Logical_Addr = ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | ((num & 0x000000ff) << 24) ;
    DEBUG_UHOST("SD_Sec_Num = %x\n", SD_Sec_Num);
    DEBUG_UHOST("SD_Logical_Addr = %x\n", SD_Logical_Addr);
}

void USB_Device_Bulk_In_Transfer(u32 len)
{
    DeviceDMACtlParam1 = ((len<<8)|(1<<1));
    DeviceDMATargetFIFONum = 0x00000001 ;  // Access FIFO0
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    //DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceInterruptSourceG2 = BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
    //printf("# B I done\n");
}

void USB_Device_Bulk_Out_Transfer(void)
{
    u32 len, i ;
    
    len = DeviceFIFO2ByteCnt & 0x000007ff ;
    DeviceDMACtlParam1 = (len<<8);
    DeviceDMATargetFIFONum = 0x00000004 ;  // Access FIFO2
    DeviceDMACtlParam2 = (u32)usb_device_buf;   // Dram Address
    DeviceDMACtlParam1 |= BIT0 ;    //Start
    while((DeviceInterruptSourceG2 & BIT7) == 0x00000000);
    //DeviceInterruptSourceG2 &= ~BIT7 ;
    DeviceInterruptSourceG2 = BIT7 ;
    DeviceDMATargetFIFONum = 0x00000000 ;
}

void USB_Device_Mass_Storage_Status(void)
{
    USB_MSC_STATUS status ;

    status.Signature = 0x53425355 ;
    status.Tag = Mass_Storage_Tag ;
    status.Data_Residue = 0x00000000 ;
    status.Status = 0x00 ;
    memcpy(usb_device_buf, &status, 13);
    USB_Device_Bulk_In_Transfer(13);
    Enable_Device_Interrupt();
    
    //Enable_Device_Interrupt();
}

void USB_Device_Inquiry_Response(void)
{
    USB_INQUIRY_RESPONSE inq_res ;
    
    DEBUG_UHOST("# Inquiry\n");
    inq_res.DeviceType = 0x00 ;
    inq_res.Removable = 0x80 ;
    inq_res.Version = 0x02 ;
    inq_res.Response_Format = 0x02 ;
    inq_res.Additional_Length = 0x1f ;
    inq_res.Reserve_0 = 0x00 ;
    inq_res.Reserve_1 = 0x00 ;
    inq_res.Reserve_2 = 0x00 ;
    inq_res.Vendor_Info0 = 'sraM';
    inq_res.Vendor_Info1 = 0x20202020 ;
    inq_res.Product_Info0 = '0000' ;
    inq_res.Product_Info1 = '1234' ;
    inq_res.Product_Info2 = '1234' ; 
    inq_res.Product_Info3 = '5678' ;
    inq_res.Product_Revision = 0x30302e31 ;
    memcpy(usb_device_buf, &inq_res, 36);

    USB_Device_Bulk_In_Transfer(36);
    USB_Device_Mass_Storage_Status();
    
}

void USB_Device_Read_Format_Capacity_Response(void)
{
    USB_FORMAT_CAPACITY_RESPONSE format_capacity_res ;
    
    //printf("# Read Format Capacity\n");

    format_capacity_res.Reserve_0 = 0x00 ;
    format_capacity_res.Reserve_1 = 0x00 ;
    format_capacity_res.Reserve_2 = 0x00 ;
    format_capacity_res.List_Length = 0x08 ;
    format_capacity_res.Num_Blocks = SD_Sec_Num ;
    //format_capacity_res.Descriptor_Code= 0x02 ;
    format_capacity_res.Descriptor_Code= 0x00 ;
    format_capacity_res.Block_Len_0= 0x00 ;
    format_capacity_res.Block_Len_1= 0x02 ;
    format_capacity_res.Block_Len_2= 0x00 ;
    memcpy(usb_device_buf, &format_capacity_res, 12);

    USB_Device_Bulk_In_Transfer(12);
    USB_Device_Mass_Storage_Status();
}

void USB_Device_Read_Capacity_Response(void)
{
    USB_CAPACITY_RESPONSE capacity_res ;
    
    //printf("# Read Capacity\n");

    capacity_res.Logical_BLK_Address = SD_Logical_Addr ;
    capacity_res.Block_Length= 0x00020000 ;
    memcpy(usb_device_buf, &capacity_res, 8);

    USB_Device_Bulk_In_Transfer(8);
    USB_Device_Mass_Storage_Status();
}

void USB_Device_Mode_Sense_Response(void)
{
    USB_MODE_SENSE_RESPONSE mode_sense_res ;
    u8 *ptr ;
    
    //printf("# Mode Sense\n");

    ptr = usb_device_buf ;    
    ptr = ptr + 17 ;

#if 1
    mode_sense_res.Mode_Data_Len = 0x000b ;
    mode_sense_res.Media_Type = 0x00 ;
    mode_sense_res.WP = 0x08 ;
    mode_sense_res.Page_Code = 0x00 ;
    mode_sense_res.Page_Length = 0x03 ;
    mode_sense_res.Tansfer_Rate = 0x11e3 ;
    mode_sense_res.Heads_Num = 0x00 ;
    mode_sense_res.Sec_per_Track = 0x00 ;
    mode_sense_res.Data_Bytes_per_Sec = 0x0008 ;
    memcpy(usb_device_buf, &mode_sense_res, 12);
    USB_Device_Bulk_In_Transfer(12);
    USB_Device_Mass_Storage_Status();
#else
    if(*ptr == 0x1c)
    {
        mode_sense_res.Mode_Data_Len = 0x0003 ;
        mode_sense_res.Media_Type = 0x00 ;
        mode_sense_res.WP = 0x00 ;
        memcpy(usb_device_buf, &mode_sense_res, 4);
        USB_Device_Bulk_In_Transfer(4);
        USB_Device_Mass_Storage_Status();
    }
    else if(*ptr == 0x3f)
    {
        mode_sense_res.Mode_Data_Len = 0x0023 ;
        mode_sense_res.Media_Type = 0x00 ;
        mode_sense_res.WP = 0x00 ;
        mode_sense_res.Page_Code = 0x05 ;
        mode_sense_res.Page_Length = 0x1e ;
        mode_sense_res.Tansfer_Rate = 0x00f0 ;
        mode_sense_res.Heads_Num = 0x10 ;
        mode_sense_res.Sec_per_Track = 0x20 ;
        mode_sense_res.Data_Bytes_per_Sec = 0x0002 ;
        mode_sense_res.Num_Cylinders = 0x5c0f ;
        mode_sense_res.Reserve_0 = 0x0000 ;
        mode_sense_res.Reserve_1 = 0x00000000 ;
        mode_sense_res.Reserve_2 = 0x00000000 ;
        mode_sense_res.Reserve_3 = 0x00000000 ;
        mode_sense_res.Reserve_4 = 0x00000000 ;
        mode_sense_res.Reserve_5 = 0x00000000 ;

        memcpy(usb_device_buf, &mode_sense_res, 36);
        USB_Device_Bulk_In_Transfer(36);
        USB_Device_Mass_Storage_Status();
    }
#endif    
}

void USB_Device_Test_Unit_Ready_Response(void)
{
    //printf("# Test Unit Ready\n");
    USB_Device_Mass_Storage_Status();
}

void USB_Device_Prevent_Remove_Response(void)
{
    USB_MSC_STATUS status ;

    status.Signature = 0x53425355 ;
    status.Tag = Mass_Storage_Tag ;
    status.Data_Residue = 0x00000000 ;
    status.Status = 0x01 ;
    memcpy(usb_device_buf, &status, 13);
    USB_Device_Bulk_In_Transfer(13);
    Enable_Device_Interrupt();
}

void USB_Device_Request_Sense_Response(void)
{
    USB_REQUEST_SENSE_RESPONSE req_sense ;

    DEBUG_UHOST("# Request_Sense\n");
    req_sense.data0 = 0x000500f0 ;
    req_sense.data1 = 0x0a000000 ;
    req_sense.data2 = 0x00000000 ;
    req_sense.data3 = 0x00000024 ;
    req_sense.data4 = 0x0000 ;

    memcpy(usb_device_buf, &req_sense, 18);
    USB_Device_Bulk_In_Transfer(18);
    USB_Device_Mass_Storage_Status();
}

void USB_Device_Read10_Response(void)
{
    u32 temp_addr, i, j ;
    u16 temp_num ;
    u8 *ptr ;
    
    //printf("# Read10\n");

    ptr = usb_device_buf ;    
    ptr = ptr + 17 ;
    memcpy(&temp_addr, ptr, 4);
    ptr = ptr + 5 ;
    memcpy(&temp_num, ptr, 2);

    read_addr = ((temp_addr & 0xff000000) >> 24) | ((temp_addr & 0x00ff0000) >> 8) | ((temp_addr & 0x0000ff00) << 8) | ((temp_addr & 0x000000ff) << 24) ;
    rw_blk_num = ((temp_num & 0xff00) >> 8) | ((temp_num & 0x00ff) << 8) ;
    //printf("%x, %x\n", read_addr, rw_blk_num);

    for(i=0 ; i<rw_blk_num ; i++)
    {
        sdcReadSingleBlock(((read_addr + i) * 512), usb_device_buf);
        ptr = usb_device_buf ;
        //printf("data %x, %x, %x\n", usb_device_buf[0], usb_device_buf[1], usb_device_buf[2]);
        USB_Device_Bulk_In_Transfer(512);
    }
    USB_Device_Mass_Storage_Status();
}

void USB_Device_Write10_Response(void)
{
    u32 temp_addr, i, j ;
    u16 temp_num ;
    u8 *ptr ;
    
    DEBUG_UHOST("# Write10\n");

    ptr = usb_device_buf ;    
    ptr = ptr + 17 ;
    memcpy(&temp_addr, ptr, 4);
    ptr = ptr + 5 ;
    memcpy(&temp_num, ptr, 2);

    write_addr = ((temp_addr & 0xff000000) >> 24) | ((temp_addr & 0x00ff0000) >> 8) | ((temp_addr & 0x0000ff00) << 8) | ((temp_addr & 0x000000ff) << 24) ;
    rw_blk_num = ((temp_num & 0xff00) >> 8) | ((temp_num & 0x00ff) << 8) ;
    //printf("%x, %x\n", read_addr, rw_blk_num);

    for(i=0 ; i<rw_blk_num ; i++)
    {
        USB_Device_Bulk_Out_Transfer();
        sdcWriteSingleBlock(((write_addr + i) * 512), usb_device_buf);
    }
    USB_Device_Mass_Storage_Status();
}



void USB_Device_Parse_CMD(void)
{
    u8 *ptr ;
    
    ptr = usb_device_buf ;
    ptr = ptr + 4 ;
    memcpy(&Mass_Storage_Tag, ptr, 4);
    ptr = usb_device_buf ;    
    ptr = ptr + 15 ;
    switch(*ptr)
    {
        case 0x00:    //Test Unit Ready
            USB_Device_Test_Unit_Ready_Response();
            break;
        case 0x03:    //Request Sense
            USB_Device_Request_Sense_Response();
            break;      
        case 0x12:    //Inquiry
            USB_Device_Inquiry_Response();
            break;
        case 0x1A:    //Mode Sense
            USB_Device_Mode_Sense_Response();
            break;    
        case 0x1E:    //Prevent Remove
            USB_Device_Prevent_Remove_Response();
            break;       
        case 0x23:    //Read Format Capacity
            USB_Device_Read_Format_Capacity_Response();
            break;    
        case 0x25:    //Read Capacity
            USB_Device_Read_Capacity_Response();
            break;      
        case 0x28:    //Read10
            USB_Device_Read10_Response();
            break;   
        case 0x2A:    //Write10
            USB_Device_Write10_Response();
            break; 
        default :
            printf("Unknown Command %x\n", *ptr);
            break;    
    }
}

void USB_Device_Mass_Bulk_Out(void)
{

    //printf("# Bulk Out\n");
    
    USB_Device_Bulk_Out_Transfer();
    USB_Device_Parse_CMD();
}

void USB_Device_Mass_Bulk_In(void)
{
}

/*
void usbHostMSCRead10(u32 blk_num)
{
    USB_HOST_CBW    *cbw ;
    USB_HOST_UFI_Command_Read10    *cmd ; 
    u32 i ;

    cbw  = (USB_HOST_CBW *)SRAMBUFFERBULKOUTBase ;
    cbw->Signature = 0x43425355;
    cbw->Tag = 0x8a020f00;
    cbw->Transfer_Len = 0x00000200;
    //cbw->Transfer_Len = 0x00001000;
    cbw->Flags = 0x80;
    cbw->Logic_Unit_Num = 0x00;
    cbw->CB_Len = 0x0a;

    cmd = (USB_HOST_UFI_Command_Read10 *)(SRAMBUFFERBULKOUTBase + sizeof(USB_HOST_CBW));
    cmd->OP_Code = 0x28 ;
    cmd->Logical_Unit_Num = 0x00 ;
    cmd->Logical_BLK_Addr = ((read_block_addr & 0x000000ff) << 24) | ((read_block_addr & 0x0000ff00) << 8) | ((read_block_addr & 0x00ff0000) >> 8) | ((read_block_addr & 0xff000000) >> 24);
    cmd->reserved1 = 0x00 ;
    cmd->Tran_Len = 0x0100 ;
    //cmd->Tran_Len = 0x0800 ;
    cmd->Control = 0x00 ;
    cmd->Padding1 = 0x0000 ;
    cmd->Padding2 = 0x00000000 ;
    
    MS_Bulk_Out_Transfer(31);

    MS_Bulk_In_Transfer();
    mcpu_ByteMemcpy(read_buff_addr, (u8 *)(SRAMBUFFERBULKINBase), 512);
    //Change_Bulk_In_Addr();
    MS_Bulk_In_Transfer();
    OSSemPost(USBHost_Mass_Storage_Command_Finish_Evt);
    
}


void usbHostMSCRead10_Multi(void)
{
    USB_HOST_CBW    *cbw ;
    USB_HOST_UFI_Command_Read10    *cmd ; 
    u32 i ;

    cbw  = (USB_HOST_CBW *)SRAMBUFFERBULKOUTBase ;
    cbw->Signature = 0x43425355;
    cbw->Tag = 0x8a020f00;
    //cbw->Transfer_Len = 0x00000200;
    //cbw->Transfer_Len = 0x00001000;
    cbw->Transfer_Len = (multi_read_sec_num<< 9);
    cbw->Flags = 0x80;
    cbw->Logic_Unit_Num = 0x00;
    cbw->CB_Len = 0x0a;

    cmd = (USB_HOST_UFI_Command_Read10 *)(SRAMBUFFERBULKOUTBase + sizeof(USB_HOST_CBW));
    cmd->OP_Code = 0x28 ;
    cmd->Logical_Unit_Num = 0x00 ;
    cmd->Logical_BLK_Addr = ((read_block_addr & 0x000000ff) << 24) | ((read_block_addr & 0x0000ff00) << 8) | ((read_block_addr & 0x00ff0000) >> 8) | ((read_block_addr & 0xff000000) >> 24);
    cmd->reserved1 = 0x00 ;
    //cmd->Tran_Len = 0x0100 ;
    //cmd->Tran_Len = 0x0800 ;
    cmd->Tran_Len = (((multi_read_sec_num & 0x000000ff) << 8) | ((multi_read_sec_num & 0x0000ff00) >> 8));
    cmd->Control = 0x00 ;
    cmd->Padding1 = 0x0000 ;
    cmd->Padding2 = 0x00000000 ;
    
    MS_Bulk_Out_Transfer(31);

    //MS_Bulk_In_Transfer();
    
    for(i=0 ; i<multi_read_sec_num ; i++)
    {
        MS_Bulk_In_Transfer();
        mcpu_ByteMemcpy((read_buff_addr+ (i * 512)), (u8 *)(SRAMBUFFERBULKINBase), 512);
    }
    //Change_Bulk_In_Addr();
    MS_Bulk_In_Transfer();
    OSSemPost(USBHost_Mass_Storage_Command_Finish_Evt);
    
}

void usbHostMSCWrite10(void)
{
    USB_HOST_CBW    *cbw ;
    USB_HOST_UFI_Command_Read10    *cmd ; 

    cbw  = (USB_HOST_CBW *)SRAMBUFFERBULKOUTBase ;
    cbw->Signature = 0x43425355;
    cbw->Tag = 0x8a020f00;
    cbw->Transfer_Len = 0x00000200;
    //cbw->Transfer_Len = 0x00001000;
    cbw->Flags = 0x00;
    cbw->Logic_Unit_Num = 0x00;
    cbw->CB_Len = 0x0a;

    cmd = (USB_HOST_UFI_Command_Read10 *)(SRAMBUFFERBULKOUTBase + sizeof(USB_HOST_CBW));
    cmd->OP_Code = 0x2a ;
    cmd->Logical_Unit_Num = 0x00 ;
    cmd->Logical_BLK_Addr = ((write_block_addr & 0x000000ff) << 24) | ((write_block_addr & 0x0000ff00) << 8) | ((write_block_addr & 0x00ff0000) >> 8) | ((write_block_addr & 0xff000000) >> 24);
    cmd->reserved1 = 0x00 ;
    cmd->Tran_Len = 0x0100 ;
    //cmd->Tran_Len = 0x0800 ;
    cmd->Control = 0x00 ;
    cmd->Padding1 = 0x0000 ;
    cmd->Padding2 = 0x00000000 ;

    
    MS_Bulk_Out_Transfer(31);
    mcpu_ByteMemcpy((u8 *)(SRAMBUFFERBULKOUTBase), write_buff_addr,  512);
    MS_Bulk_Out_Transfer(512);
    MS_Bulk_In_Transfer();
    OSSemPost(USBHost_Mass_Storage_Command_Finish_Evt);
    
}

void usbHostMSCWrite10_Multi(void)
{
    USB_HOST_CBW    *cbw ;
    USB_HOST_UFI_Command_Read10    *cmd ; 
    u32 i ;

    cbw  = (USB_HOST_CBW *)SRAMBUFFERBULKOUTBase ;
    cbw->Signature = 0x43425355;
    cbw->Tag = 0x8a020f00;
    //cbw->Transfer_Len = 0x00000200;
    cbw->Transfer_Len = (multi_write_sec_num<< 9);
    cbw->Flags = 0x00;
    cbw->Logic_Unit_Num = 0x00;
    cbw->CB_Len = 0x0a;

    cmd = (USB_HOST_UFI_Command_Read10 *)(SRAMBUFFERBULKOUTBase + sizeof(USB_HOST_CBW));
    cmd->OP_Code = 0x2a ;
    cmd->Logical_Unit_Num = 0x00 ;
    cmd->Logical_BLK_Addr = ((write_block_addr & 0x000000ff) << 24) | ((write_block_addr & 0x0000ff00) << 8) | ((write_block_addr & 0x00ff0000) >> 8) | ((write_block_addr & 0xff000000) >> 24);
    cmd->reserved1 = 0x00 ;
    //cmd->Tran_Len = 0x0100 ;
    cmd->Tran_Len = (((multi_write_sec_num & 0x000000ff) << 8) | ((multi_write_sec_num & 0x0000ff00) >> 8));
    cmd->Control = 0x00 ;
    cmd->Padding1 = 0x0000 ;
    cmd->Padding2 = 0x00000000 ;

    
    MS_Bulk_Out_Transfer(31);
    for(i=0 ; i<multi_write_sec_num ; i++)
    {
        mcpu_ByteMemcpy((u8 *)(SRAMBUFFERBULKOUTBase), write_buff_addr,  512);
        MS_Bulk_Out_Transfer(512);
        write_buff_addr = write_buff_addr + 512 ;
    }
    MS_Bulk_In_Transfer();
    OSSemPost(USBHost_Mass_Storage_Command_Finish_Evt);
    
}


void USB_Host_Mass_Storage_Read_Sector(u32 sec_num, u8 *buf)
{
    u8 err;
    read_block_addr= sec_num ;
    read_buff_addr = buf ;
    usbHostMSCSetEvt(USB_HOST_MSC_EVT_READ_10_SECTOR);
    OSSemPend(USBHost_Mass_Storage_Command_Finish_Evt, OS_IPC_WAIT_FOREVER, &err);
    //mcpu_ByteMemcpy(buf, (u8 *)(SRAMBUFFERBULKINBase), 512);
    //Undo_Bulk_In_Addr();
}

void USB_Host_Mass_Storage_Read_Multi(u32 start_sec, u8 *buf, u32 total_sec)
{
    u8 err;
    read_block_addr= start_sec ;
    read_buff_addr = buf ;
    multi_read_sec_num = total_sec ;
    usbHostMSCSetEvt(USB_HOST_MSC_EVT_READ_10_MULTI);
    OSSemPend(USBHost_Mass_Storage_Command_Finish_Evt, OS_IPC_WAIT_FOREVER, &err);
}

void USB_Host_Mass_Storage_Write_Sector(u32 sec_num, u8 *buf)
{
    u8 err;
    write_block_addr= sec_num ;
    write_buff_addr = buf ;
    usbHostMSCSetEvt(USB_HOST_MSC_EVT_WRITE_10_SECTOR);
    OSSemPend(USBHost_Mass_Storage_Command_Finish_Evt, OS_IPC_WAIT_FOREVER, &err);
}

void USB_Host_Mass_Storage_Write_Multi(u32 start_sec, u8 *buf, u32 total_sec)
{
    u8 err;
    write_block_addr= start_sec ;
    write_buff_addr = buf ;
    multi_write_sec_num = total_sec ;
    usbHostMSCSetEvt(USB_HOST_MSC_EVT_WRITE_10_MULTI);
    OSSemPend(USBHost_Mass_Storage_Command_Finish_Evt, OS_IPC_WAIT_FOREVER, &err);
}
*/

/*
void USB_Device_Mass_Storage_Init()
{
    printf("# Inquiry\n");
    usbHostMSCInquiry();
    //usbHostMSCInquiry();
}
*/

s32 usbDeviceMSCInit(void)
{
    /* Create the semaphore */
    USBDevice_Mass_Storage_Start_Evt = OSSemCreate(0);
    USBDevice_Mass_Storage_Bulk_Trans_Done_Evt = OSSemCreate(0);
    USBDevice_Mass_Storage_Command_Finish_Evt = OSSemCreate(0);
    memset((void *)&usbDeviceMSCEvt, 0, sizeof(USB_INT_EVT));
    /* Create the task */
    DEBUG_UHOST("MSC Task C\n");
    OSTaskCreate(USB_DEVICE_MSC_TASK, USB_DEVICE_MSC_TASK_PARAMETER, USB_DEVICE_MSC_TASK_STACK, USB_DEVICE_MSC_TASK_PRIORITY);

    return 1;
}


s32 usbDeviceMSCSetEvt(u8 cause)
{
    /* check if cause is valid */
    if (cause >= USB_DEVICE_MSC_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    /* set the cause */
    usbDeviceMSCEvt.cause[usbDeviceMSCEvt.idxSet++] = cause;

    if (usbDeviceMSCEvt.idxSet == USB_DEVICE_MAX_INT_EVT)
    {	/* wrap around the index */
        usbDeviceMSCEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbDeviceMSCEvt.idxSet == usbDeviceMSCEvt.idxGet)
    {
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    OSSemPost(USBDevice_Mass_Storage_Start_Evt);

    return 1;
}

s32 usbDeviceMSCGetEvt(u8* pCause)
{
    /* check if event queue is empty */
    if (usbDeviceMSCEvt.idxGet == usbDeviceMSCEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pCause = usbDeviceMSCEvt.cause[usbDeviceMSCEvt.idxGet++];

    if (usbDeviceMSCEvt.idxGet == USB_DEVICE_MAX_INT_EVT)
    {	/* wrap around the index */
        usbDeviceMSCEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= USB_DEVICE_MSC_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    return 1;
}


/*

Routine Description:

	The USB Video Class task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
void usbDeviceMSCTask(void* pData)
{
    u8 err;
    u8 cause;
    
    while (1)
    {
        if(usbDeviceMSCGetEvt(&cause))
        {
            (*usbDeviceMSCIntEvtFunc[cause])();
        }
        else
        {
            //printf("Pend\n");
            OSSemPend(USBDevice_Mass_Storage_Start_Evt, OS_IPC_WAIT_FOREVER, &err);
            //printf("Pend end\n");
            if (err != OS_NO_ERR)
            {
                DEBUG_USB("Error: usbSemEvt is %d.\n", err);
                //return ;
            }
        }
    }
}

#endif

