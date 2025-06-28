/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usb_Host.c

Abstract:

   	USB routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2010/12/17	Griffy Liu	Create

*/


#include "general.h"
#include "board.h"
#include "task.h"
#include "usbHost.h"
#include "usb_Host_Event.h"
#include "gpioapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "sysapi.h"

#include "Ehic.h"
#include "usbHost_MSC.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
#if (USB_HOST == 1)
u32  usbOTGGetIntStat(void);
void USB_Delay(u32);
void usbHostTask(void*);
s32 usbHostManageInit(void);
s32 usbHostResourceInit(void);
s32 usbHostSetIntEvt(u8);
s32 usbHostGetIntEvt(u8*);
s32 usbHostInit(void);
u8 usbHostUninst(void);


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

OS_STK usbHostTaskStack[USB_HOST_TASK_STACK_SIZE]; /* Stack of task usbTask() */

OS_EVENT* usbHostSemEvt; /* semaphore to synchronize event processing */
//OS_EVENT* usbSemEp[USB_HOST_SEM_EP_MAX]; /* semaphore to synchronize endpoint access */

USB_INT_EVT usbHostIntEvt; /* Interrupt event queue */
static u32  enumeration_stage = STAGE_GET_DEVECE_DESCRIPTOR ; 
static u32  control_stage = SETUP_STAGE ;
static u8   enumeration_done = 0 ;
static u8   clear_feature = 0 ;
static u8   read_write_proccess = 0 ;
static u8   clear_halt = 0 ;

static u32  mass_storage_stage = COMMAND_STAGE ; 
static u32  mass_storage_command = SETUP_STAGE ;

USB_HOST_API_EVT usbHostApiEvt; /* API event queue */
u8               bus_reset = 0 ;

extern u32 usbEpMaxPktSize[USB_HOST_EP_MAX];
extern u32 usbEpMaxDmaSize[USB_HOST_EP_MAX];
extern void (*usbHostIntEvtFunc[])(void);

extern OS_FLAG_GRP  *gSdUsbProcFlagGrp;

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

void usbHost_Bus_Reset(void)
{
    u32 i ;
    bus_reset = 1 ;
    DEBUG_UHOST("Bus Reset\n");
    HCPortSC |= 0x00000100 ;
    for(i=0 ; i<10000 ; i++ );
    HCPortSC &= ~0x00000100 ;
}

void Set_Clear_Feature_Flag(u32 flag)
{
	clear_feature = flag ;
}

void Set_Control_Stage(u32 stage)
{
	control_stage = stage ;
}

u32 Get_Enumeration_Stage(void)
{
	u32 stage ;

	stage = enumeration_stage ;
	return stage ;
}

u32 Set_Enumeration_Stage(u32 stage)
{
	enumeration_stage = stage;
}

u32 Get_Mass_Storage_Command(void)
{
	u32 command ;

	command = mass_storage_command ;
	return command ;
}

u32 Set_Mass_Storage_Command(u32 command)
{
	mass_storage_command = command ;
}

u32 Set_Mass_Storage_Stage(u32 stage)
{
	mass_storage_stage = stage;
}

u32 Set_Read_Write_Proccess(u8 start)
{
	read_write_proccess = start;
}

u8 Get_Read_Write_Proccess()
{
	u8 status ;

	status = read_write_proccess ;
	return status ;
}


void usbHost_Asyc_QH_List_Init(void)
{
	USB_QUEUE_HEAD  *qh ;
    USB_QTD			*qtd;
	
	qtd = (USB_QTD *)usbHost_Get_qTD();
	//QH1
	qh = (USB_QUEUE_HEAD *)usb_qh_buf_1 ;
    qh->Horizontal_Link_Pointer = (u32)usb_qh_buf_2 ;
    qh->Endpoint_Characteristics_1 = ( HIGH_SPEED | H_BIT | DTC_BIT | (CONTROL_MAX_PKT_SIZE << 16));
    qh->Endpoint_Characteristics_2 = SINGLE_TRANSACTION ;
    qh->Next_qTD_Pointer = (u32)qtd ;
    qh->Alternate_Next_qTD_Pointer = 0x00000001 ;
    qh->Horizontal_Link_Pointer |= 0x00000002 ;

    
    qh->Current_qTD_Pointer = 0x00000000 ;
    qh->qTD_Token = 0x00000000 ;
    qh->Buffer_Pointer_0 = 0x00000000 ;
    qh->Buffer_Pointer_1 = 0x00000000 ;
    qh->Buffer_Pointer_2 = 0x00000000 ;
    qh->Buffer_Pointer_3 = 0x00000000 ;
    qh->Buffer_Pointer_4 = 0x00000000 ;
    

	qtd = (USB_QTD *)usbHost_Get_qTD();
	//QH2
	qh = (USB_QUEUE_HEAD *)usb_qh_buf_2 ;
    qh->Horizontal_Link_Pointer = (u32)usb_qh_buf_3 ;
    //qh->Endpoint_Characteristics_1 = ( HIGH_SPEED | DTC_BIT | (BULK_MAX_PKT_SIZE << 16));
    qh->Endpoint_Characteristics_1 = ( HIGH_SPEED | (BULK_MAX_PKT_SIZE << 16));
    qh->Endpoint_Characteristics_2 = SINGLE_TRANSACTION ;
    qh->Next_qTD_Pointer = (u32)qtd ;
    qh->Alternate_Next_qTD_Pointer = 0x00000001 ;
    qh->Horizontal_Link_Pointer |= 0x00000002 ;

    
    qh->Current_qTD_Pointer = 0x00000000 ;
    qh->qTD_Token = 0x00000000 ;
    qh->Buffer_Pointer_0 = 0x00000000 ;
    qh->Buffer_Pointer_1 = 0x00000000 ;
    qh->Buffer_Pointer_2 = 0x00000000 ;
    qh->Buffer_Pointer_3 = 0x00000000 ;
    qh->Buffer_Pointer_4 = 0x00000000 ;
    

    qtd = (USB_QTD *)usbHost_Get_qTD();
	//QH3
	qh = (USB_QUEUE_HEAD *)usb_qh_buf_3 ;
    qh->Horizontal_Link_Pointer = (u32)usb_qh_buf_1 ;
    qh->Endpoint_Characteristics_1 = ( HIGH_SPEED | DTC_BIT | (BULK_MAX_PKT_SIZE << 16));
    qh->Endpoint_Characteristics_2 = SINGLE_TRANSACTION ;
    qh->Next_qTD_Pointer = (u32)qtd ;
    qh->Alternate_Next_qTD_Pointer = 0x00000001 ;
    qh->Horizontal_Link_Pointer |= 0x00000002 ;

    
    qh->Current_qTD_Pointer = 0x00000000 ;
    qh->qTD_Token = 0x00000000 ;
    qh->Buffer_Pointer_0 = 0x00000000 ;
    qh->Buffer_Pointer_1 = 0x00000000 ;
    qh->Buffer_Pointer_2 = 0x00000000 ;
    qh->Buffer_Pointer_3 = 0x00000000 ;
    qh->Buffer_Pointer_4 = 0x00000000 ;
    

	
}

/*
void usbHost_Asyc_Qtd_List_Init(void)
{
	USB_QTD			*qtd;
	//QTD1
	qtd = (USB_QTD *)usb_qtd_buf_1 ;
	qtd->Next_qTD_Pointer = (u32)usb_qtd_buf_2 ;
	qtd->Alternate_Next_qTD_Pointer = 0x00000001 ;
	qtd->qTD_Token = 0x00000000 ;
	qtd->Buffer_Pointer_0 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_1 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_2 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_3 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_4 = (u32)usb_Page_buf_0 ;

    //QTD2
	qtd = (USB_QTD *)usb_qtd_buf_2 ;
	qtd->Next_qTD_Pointer = (u32)usb_qtd_buf_3 ;
	qtd->Alternate_Next_qTD_Pointer = 0x00000001 ;
	qtd->qTD_Token = 0x00000000 ;
	qtd->Buffer_Pointer_0 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_1 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_2 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_3 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_4 = (u32)usb_Page_buf_0 ;

    //QTD3
	qtd = (USB_QTD *)usb_qtd_buf_3 ;
	//qtd->Next_qTD_Pointer = 0x00000001 ;
	qtd->Next_qTD_Pointer = (u32)usb_qtd_buf_4 ;
	qtd->Alternate_Next_qTD_Pointer = 0x00000001 ;
	qtd->qTD_Token = 0x00000000 ;
	qtd->Buffer_Pointer_0 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_1 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_2 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_3 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_4 = (u32)usb_Page_buf_0 ;

	//QTD4
	qtd = (USB_QTD *)usb_qtd_buf_4 ;
	qtd->Next_qTD_Pointer = 0x00000001 ;
	qtd->Alternate_Next_qTD_Pointer = 0x00000001 ;
	qtd->qTD_Token = 0x00000000 ;
	qtd->Buffer_Pointer_0 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_1 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_2 = (u32)usb_Page_buf_0 ;
   	qtd->Buffer_Pointer_3 = (u32)usb_Page_buf_0 ;
    qtd->Buffer_Pointer_4 = (u32)usb_Page_buf_0 ;
     
}
*/

s32 usbHostInit(void)
{
    u32 i, temp ;
    /* Create the semaphore */
    usbHostSemEvt = OSSemCreate(0);
    memset((void *)&usbHostIntEvt, 0, sizeof(USB_INT_EVT));
    // Start Host Task
    OSTaskCreate(USB_HOST_TASK, USB_HOST_TASK_PARAMETER, USB_HOST_TASK_STACK, USB_HOST_TASK_PRIORITY);
    //usbHostMSCInit();

    //Queue Head Build up
    printf("usb_qh_buf_1 %x\n", usb_qh_buf_1);
	printf("usb_qh_buf_2 %x\n", usb_qh_buf_2);
    printf("usb_qh_buf_3 %x\n", usb_qh_buf_3);
    printf("usb_qtd_buf %x\n", usb_qtd_buf);
	printf("usb_itd_buf_1 %x\n", usb_itd_buf_1);
	printf("usb_Page_buf_0 %x\n", usb_Page_buf_0);

    HOST_DEVICE_SWITCH &= ~0x00000001 ;
    //proccess 1
    //while((OTGCtlS & BIT20) == BIT20);
    printf("OTGCtlS %x\n", OTGCtlS);
    OTGCtlS = 0x00360220 ;
    printf("HCMisc %x\n", HCMisc);
    HCMisc = 0x000001bd ;
    printf("OTGCtlS %x\n", OTGCtlS);
    OTGCtlS = 0x00360230 ;
    printf("OTGCtlS %x\n", OTGCtlS);
    OTGCtlS = 0x00360210 ;
    printf("OTGCtlS %x\n", OTGCtlS);
    printf("GLOBALInterruptMask %x\n", GLOBALInterruptMask);
    //GLOBALInterruptMask = 0x00000007 ;
    GLOBALInterruptMask = 0x00000001 ;
    //HCUSBSTS    |= 0x00000004 ;
    HCUSBINTR |= 0x00000007 ;
    printf("DeviceMainCtl %x\n", DeviceMainCtl);
    //DeviceMainCtl = 0x00000022 ;
    DeviceMainCtl = 0x00000020 ;

    HCUSBCMD &= ~0x00ff0000 ;
    HCUSBCMD |= 0x00010000 ;

    //Park mode count
    HCUSBCMD &= ~0x00000b00 ;
    HCUSBCMD |= 0x00000100 ;
    // Async Addr
    HCAsyncListAddr = (u32)usb_qh_buf_1 ;

	usbHost_qTD_Init();
	usbHost_Asyc_QH_List_Init();
	//usbHost_Asyc_Qtd_List_Init();
	
    
    HCUSBCMD |= BIT0 ;    //Run

    printf("# HCUSBCMD %x\n", HCUSBCMD);
    printf("Init done\n");
    
    return 1;
}

void usbHostTask(void* pData)
{
    u8 err;
    u8 cause;

    while (1)
    {
        if (usbHostGetIntEvt(&cause))
        {
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
            //printf("# usbHostSemEvt\n");
            if (err != OS_NO_ERR)
            {
                DEBUG_USB("Error: usbSemEvt is %d.\n", err);
                //return ;
            }
        }
    }
}

void usbHostIntHandler(void)
{
    //printf("GLOBALInterruptS %x\n", GLOBALInterruptS);
    //printf("HCPortSC %x\n", HCPortSC);
    static u32 once = 0 ;
    static u8 clear_once = 0 ;
    USB_QUEUE_HEAD  *qh ;
    //printf("H I \n");

    if((HCPortSC & 0x00000002) == 0x00000002)
    {
        HCPortSC &= ~0x00000002 ;
        if((HCUSBSTS & 0x00000004) == 0x00000004)
        {
            HCUSBSTS = 0x00000004 ;
            if(bus_reset == 0)
                usbHost_Bus_Reset();
        }
    }

    if((HCUSBSTS & 0x00000004) == 0x00000004)
    {
        HCUSBSTS = 0x00000004 ;
        printf("Bus Reset done\n");
        HCUSBCMD |= BIT5 ;
        //HCUSBCMD |= BIT0 ;    //Run
		usbHostSetIntEvt(USB_HOST_INT_EVT_DEVICE_PLUG_IN);
    }

    if((HCUSBSTS & 0x00000002) == 0x00000002)
    {
        printf("# Error I\n");
        HCUSBSTS = 0x00000002 ;
        qh = (USB_QUEUE_HEAD *)usb_qh_buf_1 ;
        printf("Err Type 1 %x\n", qh->qTD_Token);
        qh = (USB_QUEUE_HEAD *)usb_qh_buf_2 ;
        printf("Err Type 2 %x\n", qh->qTD_Token);
        if(clear_halt == 0)
        {
            clear_halt = 1 ;
            qh->qTD_Token &= ~0x000000ff ;
        }
        qh = (USB_QUEUE_HEAD *)usb_qh_buf_3 ;
        printf("Err Type 3 %x\n", qh->qTD_Token);
        
    }
    else if((HCUSBSTS & 0x00000001) == 0x00000001)
    {
        //printf("T I\n");
        HCUSBSTS = 0x00000001 ;
        if(enumeration_done == 0)
        {
            switch(enumeration_stage)
    	    {
        	    case STAGE_GET_DEVECE_DESCRIPTOR: 
				    if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
            	    break;    
			    case STAGE_SET_DEVECE_ADDRESS:    
            	    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_SET_ADDRESS);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
            	    break;
			    case STAGE_GET_DEVECE_DESCRIPTOR_2:
				    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_GET_DEVICE_DESCRIPTOR);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
            	    break;
			    case STAGE_GET_CONFIG_DESCRIPTOR:
				    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_GET_CONFIG_DESCRIPTOR);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
				    break;
			    case STAGE_GET_LANGUAGE_DESCRIPTOR:
				    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_GET_LANGUAGE_DESCRIPTOR);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
				    break;	
			    case STAGE_GET_CONFIG_DESCRIPTOR_ALL:
				    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_GET_CONFIG_DESCRIPTOR_ALL);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
				    {
				    	usbHost_Parse_Configuration_Info();
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
				    }
					
				    break;	
			    case STAGE_SET_CONFIGURATION:    
            	    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_SET_CONFIGURATION);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
            	    break;	
			    case STAGE_GET_MAX_LUN:
				    if(control_stage == SETUP_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_GET_MAX_LUN);
				    else if(control_stage == CONTROL_IN_STAGE)
            		    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
				    else if(control_stage == CONTROL_OUT_STAGE)
					    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_OUT);
				    break;		
			    case STAGE_ENUMERATION_DONE:
				    printf("Enumeration done\n");
                    enumeration_done = 1 ;
					mass_storage_stage = COMMAND_STAGE ;
					mass_storage_command = COMMAND_INQUIRY ;
				    break;
			    default :
            	    printf("Error stage\n");
            	    break;  	
		    }
        }
        if(enumeration_done == 1)
        {
            if(clear_feature == 1)
            {
                if(control_stage == CONTROL_IN_STAGE)
                    usbHostSetIntEvt(USB_HOST_INT_EVT_CONTROL_IN);
            }
        	else if(mass_storage_stage == COMMAND_STAGE)
        	{
        		switch(mass_storage_command)
    	    	{
    	    		case COMMAND_INQUIRY: 
            			usbHostSetIntEvt(USB_HOST_INT_EVT_INQUIRY);
						break;
                    case COMMAND_READ_FORMAT_CAPACITY: 
            			usbHostSetIntEvt(USB_HOST_INT_EVT_READ_FORMAT_CAPACITY);
						break;    
					case COMMAND_REQUEST_SENSE: 
            			usbHostSetIntEvt(USB_HOST_INT_EVT_REQUEST_SENSE);
            			printf("REQUEST_SENSE\n");
						break;	
					case COMMAND_WRITE10: 
                        if(once < 1)
                        {
                            once = once + 1 ;
            			    usbHostSetIntEvt(USB_HOST_INT_EVT_WRITE_10);
            			    printf("Write_10\n");
                        }
                    case COMMAND_READ10: 
                        if(once < 25600)
                        {
            			    usbHostSetIntEvt(USB_HOST_INT_EVT_READ_10);
            			    //printf("# Int Read_10\n");  
                            once = once + 1 ;
                        }
                        else 
                            printf("transfer done\n");
						break;
                        
                    case COMMAND_UNKNOWN:
                        printf("\n COMMAND_UNKNOWN, Set_Read_Write_Proccess\n");    
                        Set_Read_Write_Proccess(1);
                        OSTimeDly(1);
                        break ;
					default :
            	    	printf("Error Mass Storage Command\n");
            	    break;	
        		}
        	}
			else if(mass_storage_stage == DATA_STAGE)
			{
				if(mass_storage_command == COMMAND_WRITE10)
					usbHostSetIntEvt(USB_HOST_INT_EVT_BULK_OUT);
				else
					usbHostSetIntEvt(USB_HOST_INT_EVT_BULK_IN_DATA);
			}
			else if(mass_storage_stage == STATUS_STAGE)
			{
			    if(mass_storage_command == COMMAND_READ_FORMAT_CAPACITY && clear_once == 0)
			    {
			        clear_once = 1 ;
					usbHostSetIntEvt(USB_HOST_INT_EVT_SET_CLEAR_FEATURE);
			    }
                else
                {
                    //printf("# BULK_IN_STATUS\n");
				    usbHostSetIntEvt(USB_HOST_INT_EVT_BULK_IN_STATUS);
                }
			}
        }
    }
    
}

s32 usbHostManageInit(void)
{
    /* zero initialize structure */
    memset((void *)&usbHostIntEvt, 0, sizeof(USB_INT_EVT));
    //memset((void *)&usbApiEvt, 0, sizeof(USB_API_EVT));
    //memset((void *)&usbEpReq[0], 0, sizeof(USB_EP_REQ) * USB_EP_MAX);
    //memset((void *)&usbEpReqRet[0], 0, sizeof(USB_EP_REQ_RET) * USB_EP_MAX);

    return 1;
}

s32 usbHostSetIntEvt(u8 cause)
{
    /* check if cause is valid */
    if (cause >= USB_HOST_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    /* set the cause */
    usbHostIntEvt.cause[usbHostIntEvt.idxSet++] = cause;

    if (usbHostIntEvt.idxSet == USB_HOST_MAX_INT_EVT)
    {	/* wrap around the index */
        usbHostIntEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbHostIntEvt.idxSet == usbHostIntEvt.idxGet)
    {
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    //printf("# USB post %d\n", cause);
    OSSemPost(usbHostSemEvt);

    return 1;
}

s32 usbHostGetIntEvt(u8* pCause)
{
    /* check if event queue is empty */
    if (usbHostIntEvt.idxGet == usbHostIntEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pCause = usbHostIntEvt.cause[usbHostIntEvt.idxGet++];

    if (usbHostIntEvt.idxGet == USB_HOST_MAX_INT_EVT)
    {	/* wrap around the index */
        usbHostIntEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= USB_HOST_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    return 1;
}

u8 usbHostUninst(void)
{
	u8	err;

	DEBUG_USB("USB Uninst\n");

	/* del USB Task */
	err = OSTaskDel(USB_HOST_TASK_PRIORITY);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del USB_TASK_PRIORITY Failed = %d\n", err);
		return 0;
	}

	/* uninst Class resource */
	usbMscUnInit();

	/* uninst resource */
	//usbHostResourceUnInit();

	return 1;
}


#endif

