/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usb_Host_EHCI.c

Abstract:

   	USB interrupt event handler.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2010/12/29	Griffy Liu	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usbHost.h"
#include "Ehic.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if (USB_HOST == 1)
void EHCI_Setup_Device_Descriptor(void);

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
u8				 Host_qTD_Manage[TOTAL_QTD_NUMBER];
u8				 Device_Address = 0 ;
static u8		 bulk_in_data_toggle = 0 ;
static u8		 bulk_out_data_toggle = 0 ;


void usbHost_qTD_Init(void)
{
	u32 i ;
	for(i=0 ; i<TOTAL_QTD_NUMBER ; i++)
		Host_qTD_Manage[i] = QTD_MEM_UNUSED ;
}

u32 usbHost_Get_qTD(void)
{
	u32 i ;
	u8 bFound = 0 ;
	USB_QTD			*qtd;

    //printf("# Get_qTD\n");
	for (i=0;i<TOTAL_QTD_NUMBER;i++)
	{
		if(Host_qTD_Manage[i]==0) 
		{
        	bFound=1;
            Host_qTD_Manage[i] = QTD_MEM_USED;
            break;     
        }
	}
	if (bFound==1)	
	{
		qtd = (USB_QTD *)(usb_qtd_buf + (i * QTD_SIZE)) ;
		qtd->Next_qTD_Pointer = 0x00000001 ;
		qtd->Alternate_Next_qTD_Pointer = 0x00000001 ;
		qtd->qTD_Token = 0x00000000 ;
		qtd->Buffer_Pointer_0 = (u32)usb_Page_buf_0 ;
   		qtd->Buffer_Pointer_1 = (u32)usb_Page_buf_0 ;
    	qtd->Buffer_Pointer_2 = (u32)usb_Page_buf_0 ;
   		qtd->Buffer_Pointer_3 = (u32)usb_Page_buf_0 ;
    	qtd->Buffer_Pointer_4 = (u32)usb_Page_buf_0 ;

        //printf("# qTd addr %x\n", qtd);            
        return (u32)qtd;
    }
}

u32 usbHost_Free_qTD(u32 pwAddress)
{
    u32 i;
    u32 wReleaseNum;
    
    if (pwAddress < (u32)usb_qtd_buf) 
    {
        DEBUG_UHOST("Free qTD fail...\n");
    }
             
    if ((pwAddress - (u32)usb_qtd_buf)==0)
        wReleaseNum=0;	
    else
        wReleaseNum=(pwAddress - (u32)usb_qtd_buf)/QTD_SIZE;        
    if (wReleaseNum > TOTAL_QTD_NUMBER) 
	{
    	DEBUG_UHOST("Unknown qTD address\n");
    }
    
    Host_qTD_Manage[wReleaseNum] = QTD_MEM_UNUSED;
}

void usbHost_Send_qTD(u32 pid, u32 size, u8 toggle)
{
	USB_QUEUE_HEAD  *qh ;
	USB_QTD			*qtd, *spOldDumyqTD, *spLastqTD;

	qh = (USB_QUEUE_HEAD *)usb_qh_buf_1 ;
	qtd = (USB_QTD *)usbHost_Get_qTD();

	//qTD
	qtd->qTD_Token = (0x00008000 | pid | (toggle<<31) | (size<<16));
	spOldDumyqTD = (USB_QTD *)(qh->Next_qTD_Pointer & 0xfffffffe);
	memcpy(spOldDumyqTD, qtd, QTD_SIZE);

	//Link to new dummy qTD
	spOldDumyqTD->Next_qTD_Pointer = ((u32)qtd);
	spOldDumyqTD->Alternate_Next_qTD_Pointer = ((u32)qtd);

	//Active
	spOldDumyqTD->qTD_Token |= 0x00000080 ;

    //Free qTD
    usbHost_Free_qTD((u32)spOldDumyqTD);
}


void usbHost_Bulk_In_Send_qTD(u32 pid, u32 size, u8 toggle)
{
	USB_QUEUE_HEAD  *qh ;
	USB_QTD			*qtd, *spOldDumyqTD, *spLastqTD;
    u32 mass_cmd ;
	
	

	qh = (USB_QUEUE_HEAD *)usb_qh_buf_2 ;
	qtd = (USB_QTD *)usbHost_Get_qTD();
        
    mass_cmd = Get_Mass_Storage_Command();
    //printf("## Bulk in qTd\n");
	//qTD
	qtd->qTD_Token = (0x00008000 | pid | (toggle<<31) | (size<<16));
	spOldDumyqTD = (USB_QTD *)(qh->Next_qTD_Pointer & 0xfffffffe);
	memcpy(spOldDumyqTD, qtd, QTD_SIZE);

	//Link to new dummy qTD
	spOldDumyqTD->Next_qTD_Pointer = ((u32)qtd);
	spOldDumyqTD->Alternate_Next_qTD_Pointer = ((u32)qtd);

	//Active
	/*
	if(mass_cmd == 0x03)
	{
	    printf("# Request sense data in\n");
        while(1);
	}
    else
    */
	    spOldDumyqTD->qTD_Token |= 0x00000080 ;

    //Free qTD
    usbHost_Free_qTD((u32)spOldDumyqTD);
}

void usbHost_Bulk_Out_Send_qTD(u32 pid, u32 size, u8 toggle)
{
	USB_QUEUE_HEAD  *qh ;
	USB_QTD			*qtd, *spOldDumyqTD, *spLastqTD;

	qh = (USB_QUEUE_HEAD *)usb_qh_buf_3 ;
	qtd = (USB_QTD *)usbHost_Get_qTD();

    //printf("## Bulk out qTd\n");
	//qTD
	qtd->qTD_Token = (0x00008000 | pid | (toggle<<31) | (size<<16));
	spOldDumyqTD = (USB_QTD *)(qh->Next_qTD_Pointer & 0xfffffffe);
	memcpy(spOldDumyqTD, qtd, QTD_SIZE);

	//Link to new dummy qTD
	spOldDumyqTD->Next_qTD_Pointer = ((u32)qtd);
	spOldDumyqTD->Alternate_Next_qTD_Pointer = ((u32)qtd);

	//Active
	spOldDumyqTD->qTD_Token |= 0x00000080 ;

    //Free qTD
    usbHost_Free_qTD((u32)spOldDumyqTD);
}

void usbHost_Set_Bulk_In_Data_Toggle(void)
{
	if(bulk_in_data_toggle == 0)
		bulk_in_data_toggle = 1 ;
	else if(bulk_in_data_toggle == 1)
		bulk_in_data_toggle = 0 ;
}

void usbHost_Set_Bulk_Out_Data_Toggle(void)
{
	if(bulk_out_data_toggle == 0)
		bulk_out_data_toggle = 1 ;
	else if(bulk_out_data_toggle == 1)
		bulk_out_data_toggle = 0 ;
}

u8 usbHost_Get_Bulk_In_Data_Toggle(void)
{
	return bulk_in_data_toggle ;
}

u8 usbHost_Get_Bulk_Out_Data_Toggle(void)
{
	return bulk_out_data_toggle ;
}

void EHCI_Setup_Device_Descriptor(void)
{
	
	u32 			*buff_ptr;

	DEBUG_UHOST("# Device Descriptor\n");
	// Command
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x01000680 ;
	buff_ptr = buff_ptr + 1 ;
	if(Device_Address == 0)
		*buff_ptr = 0x00400000 ;
	else
		*buff_ptr = 0x00120000 ;

	usbHost_Send_qTD(SETUP_PID, 8, 0);
	
}

void EHCI_Change_Address(u8 address)
{
	USB_QUEUE_HEAD  *qh ;
	
	qh = (USB_QUEUE_HEAD *)usb_qh_buf_1 ;
	qh->Endpoint_Characteristics_1 |= address ;
	qh = (USB_QUEUE_HEAD *)usb_qh_buf_2 ;
	qh->Endpoint_Characteristics_1 |= address ;
    qh = (USB_QUEUE_HEAD *)usb_qh_buf_3 ;
	qh->Endpoint_Characteristics_1 |= address ;
	Device_Address = address ;
}

void EHCI_Change_QH_Endpoint(u8 qh_num, u8 ept)
{
	USB_QUEUE_HEAD  *qh ;

    if(qh_num == 2)
    {
	    qh = (USB_QUEUE_HEAD *)usb_qh_buf_2 ;
	    qh->Endpoint_Characteristics_1 |= (ept << 8) ;
    }
    if(qh_num == 3)
    {
        qh = (USB_QUEUE_HEAD *)usb_qh_buf_3 ;
	    qh->Endpoint_Characteristics_1 |= (ept << 8) ;
    }
}

void EHCI_Control_In(u32 len, u8 toggle)
{
    usbHost_Send_qTD(IN_PID, len, toggle);
}

void EHCI_Control_Out(u32 len, u8 toggle)
{
    usbHost_Send_qTD(OUT_PID, len, toggle);
}

void EHCI_Bulk_In(u32 len, u8 toggle)
{
    usbHost_Bulk_In_Send_qTD(IN_PID, len, toggle);
}

void EHCI_Bulk_Out(u32 len, u8 toggle)
{
    usbHost_Bulk_Out_Send_qTD(OUT_PID, len, toggle);
}

void EHCI_Setup_Set_Address(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x00020500 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Config_Descriptor(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x02000680 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00090000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Language_Descriptor(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x03000680 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00ff0000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Config_All_Descriptor(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x02000680 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00ff0000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Set_Configuration(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x00010900 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Set_Clear_Feature(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x00000102 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000081 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Setup_Get_Max_Lun(void)
{
	u32 			*buff_ptr;
	
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x0000fea1 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00010000 ;
    
	usbHost_Send_qTD(SETUP_PID, 8, 0);
}

void EHCI_Inquiry(void)
{
	u32 			*buff_ptr;

    DEBUG_UHOST("# EHCI_Inquiry");
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x43425355 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12345678 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000024 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12060080 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x24000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Bulk_Out_Send_qTD(OUT_PID, 31, bulk_out_data_toggle);
	usbHost_Set_Bulk_Out_Data_Toggle();
}

void EHCI_Read_Format_Capacity(void)
{
	u32 			*buff_ptr;

    DEBUG_UHOST("# Read_Format_Capacity");
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x43425355 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12345678 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x000000fc ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x230a0080 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0xfc000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Bulk_Out_Send_qTD(OUT_PID, 31, bulk_out_data_toggle);
	usbHost_Set_Bulk_Out_Data_Toggle();
    usbHost_Set_Bulk_In_Data_Toggle();
}

void EHCI_Request_Sense(void)
{
	u32 			*buff_ptr;

    DEBUG_UHOST("# Request_Sense");
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x43425355 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12345678 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000012 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x030c0080 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Bulk_Out_Send_qTD(OUT_PID, 31, bulk_out_data_toggle);
	usbHost_Set_Bulk_Out_Data_Toggle();
}

void EHCI_Read_10(void)
{
	u32 			*buff_ptr;

    //printf("# EHCI_Read 10");
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x43425355 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12345678 ;
    buff_ptr = buff_ptr + 1 ;
	//*buff_ptr = 0x00000200 ;
	*buff_ptr = 0x00001000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x280a0080 ;
	// address
    buff_ptr = buff_ptr + 1 ;
	//*buff_ptr = 0xe8030000 ;
    *buff_ptr = 0x00000000 ;
	// one block
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x08000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Bulk_Out_Send_qTD(OUT_PID, 31, bulk_out_data_toggle);
	usbHost_Set_Bulk_Out_Data_Toggle();
}


void EHCI_Write_10(void)
{
	u32 			*buff_ptr;

    DEBUG_UHOST("# EHCI_Write 10");
	buff_ptr = usb_Page_buf_0 ;
	*buff_ptr = 0x43425355 ;
	buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x12345678 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000200 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x2a0a0000 ;
	// address 51200
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0xc8000000 ;
	// one block
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x01000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    buff_ptr = buff_ptr + 1 ;
	*buff_ptr = 0x00000000 ;
    
	usbHost_Bulk_Out_Send_qTD(OUT_PID, 31, bulk_out_data_toggle);
	usbHost_Set_Bulk_Out_Data_Toggle();
}

#endif
