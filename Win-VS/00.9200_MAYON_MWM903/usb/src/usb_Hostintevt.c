/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbint.c

Abstract:

   	USB interrupt event handler.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2010/12/17	Griffy Liu	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usbHost.h"
#include "usbHost_MSC.h"


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if 0//(USB_HOST == 1)
void usbHost_EvtAttach(void);
void usbHost_Control_In(void);
void usbHost_Control_Out(void);
void usbHost_Bulk_In_Data(void);
void usbHost_Bulk_In_Status(void);
void usbHost_Bulk_Out_Data(void);
void usbHost_Set_Address(void);
void usbHost_Get_Device_Descriptor(void);
void usbHost_Get_Config_Descriptor(void);
void usbHost_Get_Language_Descriptor(void);
void usbHost_Get_Config_All_Descriptor(void);
void usbHost_Set_Configuration(void);
void usbHost_Set_Clear_Feature(void);
void usbHost_Get_Max_Lun(void);
void usbHost_Inquiry(void);
void usbHost_Read_Format_Capacity(void);
void usbHost_Request_Sense(void);
void usbHost_Read_10(void);
void usbHost_Write_10(void);


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
extern OS_EVENT* USBHost_Mass_Storage_Start_Evt;


/* Interrupt event function */
void (*usbHostIntEvtFunc[])(void) =
    {
        usbHost_EvtAttach,					// 0x00  - attach event 
        usbHost_Control_In,					// 0x01  - Control-in 
        usbHost_Control_Out,				// 0x02  - Control-out 
        usbHost_Set_Address,  				// 0x03  - Set Address
        usbHost_Get_Device_Descriptor,		// 0x04	- Device Descriptor 
        usbHost_Get_Config_Descriptor,		// 0x05	- Config Descriptor
        usbHost_Get_Language_Descriptor,	// 0x06	- Language Descriptor
        usbHost_Get_Config_All_Descriptor,	// 0x07	- Config All Descriptor
        usbHost_Set_Configuration,
        usbHost_Set_Clear_Feature,
        usbHost_Get_Max_Lun,
        usbHost_Bulk_In_Data,
        usbHost_Bulk_In_Status,
        usbHost_Bulk_Out_Data,
        usbHost_Inquiry,
        usbHost_Read_Format_Capacity,
        usbHost_Request_Sense,
        usbHost_Read_10,
        usbHost_Write_10
    };


void usbHost_EvtAttach(void)
{
	DEBUG_UHOST("# USB Device Attach\n");
	
    Set_Control_Stage(CONTROL_IN_STAGE);

	EHCI_Setup_Device_Descriptor();    
}

void Bulk_Out_Data_Prepare(void)
{
	u32 			*buff_ptr;
	u32 i, data ;

	data = 0x00000000 ;
	buff_ptr = usb_Page_buf_0 ;
	for(i = 0 ; i < 128 ; i++)
	{
		*buff_ptr = data ;
		buff_ptr = buff_ptr + 1 ;
		data = data + 1 ;
	}
}

void usbHost_Parse_Configuration_Info(void)
{
	u32 *buf ;
	u8 	in_ept_num, out_ept_num ;
	
	buf = usb_Page_buf_0;
	buf = buf + 5 ;
	
	if((*buf & 0x00000080) == 0x00000080)
	{
		in_ept_num = *buf & 0x0000000f ;
		EHCI_Change_QH_Endpoint(2, in_ept_num);
		buf = buf + 1 ;
		//out_ept_num = *buf & 0x0f000000 ;
        out_ept_num = ((*buf & 0x0f000000)>>24) ;
		EHCI_Change_QH_Endpoint(3, out_ept_num);
        DEBUG_UHOST("# 1 in_ept_num = %x, out_ept_num = %x\n", in_ept_num, out_ept_num);
	}
	else
	{
		out_ept_num = *buf & 0x0000000f ;
		EHCI_Change_QH_Endpoint(3, out_ept_num);
		buf = buf + 1 ;
		in_ept_num = ((*buf & 0x0f000000)>>24) ;
		EHCI_Change_QH_Endpoint(2, in_ept_num);
        DEBUG_UHOST("# 2 in_ept_num = %x, out_ept_num = %x\n", in_ept_num, out_ept_num);
	}
    
}

void usbHost_Control_In(void)
{
    u32 enumer_stage ;
    
    DEBUG_UHOST("# Control In\n");

	enumer_stage = Get_Enumeration_Stage();
    if((enumer_stage == STAGE_SET_DEVECE_ADDRESS) || (enumer_stage == STAGE_SET_CONFIGURATION))		
		Set_Control_Stage(SETUP_STAGE);
	else
		Set_Control_Stage(CONTROL_OUT_STAGE);
	
	switch(enumer_stage)
    {
        case STAGE_GET_DEVECE_DESCRIPTOR:
		case STAGE_GET_DEVECE_DESCRIPTOR_2:	
			EHCI_Control_In(0x12, 1);
           	break;
		case STAGE_SET_DEVECE_ADDRESS:  
            Set_Enumeration_Stage(enumer_stage + 1);
            EHCI_Control_In(0x00, 1);
            break;	
		case STAGE_GET_CONFIG_DESCRIPTOR:    
            EHCI_Control_In(0x09, 1);
            break;
		case STAGE_GET_LANGUAGE_DESCRIPTOR:   
            EHCI_Control_In(0x04, 1);
            break;	
		case STAGE_GET_CONFIG_DESCRIPTOR_ALL:   
            EHCI_Control_In(0x20, 1);
            break;		
		case STAGE_SET_CONFIGURATION:  
            Set_Enumeration_Stage(enumer_stage + 1);
            EHCI_Control_In(0x00, 1);
            break;
        case STAGE_SET_CLEAR_FEATURE:  
            //Set_Enumeration_Stage(enumer_stage + 1);
            Set_Clear_Feature_Flag(0);
            EHCI_Control_In(0x00, 1);
            break;    
		case STAGE_GET_MAX_LUN:    
            EHCI_Control_In(0x01, 1);
            break;	
		default:
            DEBUG_UHOST("Error stage\n");
            break;  	
	}
}

void usbHost_Control_Out(void)
{
	u32 enumer_stage ;
	
	DEBUG_UHOST("# Ctl-Out\n");

	enumer_stage = Get_Enumeration_Stage();
	Set_Enumeration_Stage(enumer_stage + 1);
	Set_Control_Stage(SETUP_STAGE);
	EHCI_Control_Out(0x00, 1);
}

void usbHost_Bulk_In_Data(void)
{
	u32 mass_cmd ;
	
	mass_cmd = Get_Mass_Storage_Command();
	Set_Mass_Storage_Stage(STATUS_STAGE);
	switch(mass_cmd)
    {
        case COMMAND_INQUIRY:
			EHCI_Bulk_In(0x24, usbHost_Get_Bulk_In_Data_Toggle());
			usbHost_Set_Bulk_In_Data_Toggle();
			break;
        case COMMAND_READ_FORMAT_CAPACITY:
			EHCI_Bulk_In(0x0c, usbHost_Get_Bulk_In_Data_Toggle());
			usbHost_Set_Bulk_In_Data_Toggle();
			break;    
        case COMMAND_REQUEST_SENSE:
			EHCI_Bulk_In(0x12, usbHost_Get_Bulk_In_Data_Toggle());
			usbHost_Set_Bulk_In_Data_Toggle();
			break;    
        case COMMAND_READ10:
			//EHCI_Bulk_In(0x200, usbHost_Get_Bulk_In_Data_Toggle());
            EHCI_Bulk_In(0x1000, usbHost_Get_Bulk_In_Data_Toggle());
			usbHost_Set_Bulk_In_Data_Toggle();
			break;    
		default:
            DEBUG_UHOST("Error command\n");
            break;  	
	}
	
}

void usbHost_Bulk_In_Status(void)
{
	u32 mass_cmd ;
    
    //DEBUG_UHOST("## usbHost_Bulk_In_Status\n");
	mass_cmd = Get_Mass_Storage_Command();
	Set_Mass_Storage_Stage(COMMAND_STAGE);
	switch(mass_cmd)
    {
    	case COMMAND_INQUIRY:
			Set_Mass_Storage_Command(COMMAND_READ_FORMAT_CAPACITY);
			break;
        case COMMAND_READ_FORMAT_CAPACITY:
			Set_Mass_Storage_Command(COMMAND_REQUEST_SENSE);
			break;    
        case COMMAND_REQUEST_SENSE:
			Set_Mass_Storage_Command(COMMAND_READ10);
            //Set_Mass_Storage_Command(COMMAND_UNKNOWN);
			break;    
	}
	//if(mass_cmd != COMMAND_REQUEST_SENSE)
	{
	    EHCI_Bulk_In(0x0d, usbHost_Get_Bulk_In_Data_Toggle());
	    usbHost_Set_Bulk_In_Data_Toggle();
	}
}

void usbHost_Bulk_Out_Data(void)
{
	u32 mass_cmd ;

    DEBUG_UHOST("# usbHost_Bulk_Out_Data\n");
	mass_cmd = Get_Mass_Storage_Command();
	Set_Mass_Storage_Stage(STATUS_STAGE);
	switch(mass_cmd)
    {
        case COMMAND_WRITE10:
			Bulk_Out_Data_Prepare();
			EHCI_Bulk_Out(0x200, usbHost_Get_Bulk_Out_Data_Toggle());
			usbHost_Set_Bulk_Out_Data_Toggle();
			break;
		default:
            DEBUG_UHOST("Error command\n");
            break;  	
	}
}

void usbHost_Get_Device_Descriptor(void)
{
    DEBUG_UHOST("## Get_Device_Descriptor\n");

	EHCI_Change_Address(2);
	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Device_Descriptor();
}

void usbHost_Set_Address(void)
{
    DEBUG_UHOST("# Set_Address\n");
    
	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Set_Address();
}

void usbHost_Get_Config_Descriptor(void)
{
	DEBUG_UHOST("# Get_Config_Descriptor\n");

	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Config_Descriptor();
}

void usbHost_Get_Language_Descriptor(void)
{
	DEBUG_UHOST("# Get_Language_Descriptor\n");

	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Language_Descriptor();
}

void usbHost_Get_Config_All_Descriptor(void)
{
	DEBUG_UHOST("# Get_All_Config_Descriptor\n");

	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Config_All_Descriptor();
}

void usbHost_Set_Configuration(void)
{
    DEBUG_UHOST("# Set_Configuration\n");
    
	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Set_Configuration();
}

void usbHost_Set_Clear_Feature(void)
{
    DEBUG_UHOST("# Set_Clear_Feature\n");

    Set_Clear_Feature_Flag(1);
    Set_Enumeration_Stage(STAGE_SET_CLEAR_FEATURE);
	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Set_Clear_Feature();
}

void usbHost_Get_Max_Lun(void)
{
	DEBUG_UHOST("# Get Max Lun\n");

	Set_Control_Stage(CONTROL_IN_STAGE);
	EHCI_Setup_Get_Max_Lun();
}

void usbHost_Inquiry(void)
{
	Set_Mass_Storage_Stage(DATA_STAGE);
    EHCI_Inquiry();
}

void usbHost_Read_Format_Capacity(void)
{
	Set_Mass_Storage_Stage(DATA_STAGE);
    EHCI_Read_Format_Capacity();
}

void usbHost_Request_Sense(void)
{
    Set_Mass_Storage_Stage(DATA_STAGE);
    EHCI_Request_Sense();
}

void usbHost_Read_10(void)
{
	Set_Mass_Storage_Stage(DATA_STAGE);
    EHCI_Read_10();
}

void usbHost_Write_10(void)
{
	Set_Mass_Storage_Stage(DATA_STAGE);
    EHCI_Write_10();
}

#endif
