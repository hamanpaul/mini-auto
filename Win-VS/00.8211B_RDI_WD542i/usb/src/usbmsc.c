/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbmsc.c

Abstract:

   	USB Mass Storage Class routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "osapi.h"
#include "usbreg.h"
#include "board.h"
#include "usb.h"
#include "usbdev.h"
#include "usbmsc.h"
#include "usbapi.h"
#include "usbapiex.h"
#include "scsi.h"
#include "Fsapi.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbMscInit(void);
void usbMscTask(void*);

s32 usbMscSend(u8, u8*, u32, u32*);
s32 usbMscRecv(u8, u8*, u32, u32*);

s32 usbMscCheckCbw(u32);
s32 usbMscSendCsw(u32, u8);
s32 usbMscSetSenseData(u8, u32, u8, u8, u8);

s32 usbMscCmdTestUnitReady(SCSI_CDB_TEST_UNIT_READY*);
s32 usbMscCmdRequestSense(SCSI_CDB_REQUEST_SENSE*);
s32 usbMscCmdInquiry(SCSI_CDB_INQUIRY*);
s32 usbMscCmdModeSelect6(SCSI_CDB_MODE_SELECT6*);
s32 usbMscCmdModeSense6(SCSI_CDB_MODE_SENSE6*);
s32 usbMscCmdStartStopUnit(SCSI_CDB_START_STOP_UNIT*);
s32 usbMscCmdPreventAllowMediumRemoval(SCSI_CDB_PREVENT_ALLOW_MEDIUM_REMOVAL*);
s32 usbMscCmdReadFormatCapacities(SCSI_CDB_READ_FORMAT_CAPACITIES*);
s32 usbMscCmdReadCapacity(SCSI_CDB_READ_CAPACITY*);
s32 usbMscCmdRead10(SCSI_CDB_READ10*);
s32 usbMscCmdWrite10(SCSI_CDB_WRITE10*);
s32 usbMscCmdVerify10(SCSI_CDB_VERIFY10*);
s32 usbMscCmdModeSense10(SCSI_CDB_MODE_SENSE10*);
s32 usbMscCmdUnknown(void);

s32 usbMscInitDev(void);
s32 usbMscInitStdInquiryData(void);
s32 usbMscInitModePage(void);
s32 usbMscInitModeParamHeader(void);
s32 usbMscInitModeParamBlockDesc(void);
s32 usbMscInitControlModePage(SCSI_CONTROL_MODE_PAGE*);
s32 usbMscInitDisconnReconnPage(SCSI_DISCONN_RECONN_PAGE*);
s32 usbMscInitPeripheralDevicePage(SCSI_PERIPHERAL_DEVICE_PAGE*);
s32 usbMscInitCachePage(SCSI_CACHE_PAGE*);
s32 usbMscInitFormatDevicePage(SCSI_FORMAT_DEVICE_PAGE*);
s32 usbMscInitMediumTypeSupportedPage(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE*);
s32 usbMscInitNotchPage(SCSI_NOTCH_PAGE*);
s32 usbMscInitReadWriteErrorRecoveryPage(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE*);
s32 usbMscInitVerifyErrorRecoveryPage(SCSI_VERIFY_ERROR_RECOVERY_PAGE*);

s32 usbPerfSdWrite(void);
s32 usbPerfUsbSend(void);

s32 usbMscUnInit(void);
s32 usbPerfInit(void);
s32 usbPerfUnInit(void);



/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
//#define usbMscDebugPrint 			DEBUG_USB
#define MAX_SECTION		16		/* maximum sections for usb MSC receiving */
#define PIPELINE_WRITE		0		/* indicates write parallel with USB recv and SD write */
#define PIPELINE_READ		0		/* indicates read parallel with USB send and SD read */
#define USB_DEBUG_PRT		0		/* indicator to print debug message */

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK usbMscTaskStack[USB_MSC_TASK_STACK_SIZE]; /* Stack of task usbMscTask() */
OS_STK usbPerfTaskStack[USB_PERF_TASK_STACK_SIZE];	/* Stack of task usbPerTask() */

OS_EVENT* usbMscSem; /* Interrupt/API events signal usbMscTask() via this semaphore */
OS_EVENT* usbSemPerfEvt;	/* Semaphore for SD performance improvement */
OS_EVENT* usbSemPerfLastBlock;		/* Semaphore to synchronize the last block writing cycle */
OS_EVENT* usbSemPerfCritSec;	/* Semaphore Synchronize critical section accessing */
OS_EVENT* usbSemPrevSend;	/* Semaphore Synchronize previous bulk-in transfer */


__align(4) USB_MSC_CBW usbMscCbw; /* command block wrapper */
__align(4) USB_MSC_CSW usbMscCsw; /* command status wrapper */

__align(4) u8 usbMscSendBuf[USB_MSC_BUF_COUNT][USB_MSC_BUF_SIZE]; /* send buffer */
__align(4) u8 usbMscRecvBuf[USB_MSC_BUF_COUNT][USB_MSC_BUF_SIZE]; /* receive buffer */

__align(4) SCSI_SENSE_DATA  scsiSenseData; /* sense data */
__align(4) SCSI_STANDARD_INQUIRY_DATA scsiStdInquiryData; /* standard inquiry data */
SCSI_MODE_PARAM_HEADER6  scsiModeParamHeader6;  /* mode parameter header (6) */
SCSI_MODE_PARAM_HEADER10 scsiModeParamHeader10; /* mode parameter header (10) */
SCSI_MODE_PARAM_BLOCK_DESC scsiModeParamBlockDesc[SCSI_MODE_PARAM_BLOCK_DESC_COUNT]; /* mode parameter block descriptor */
SCSI_MODE_PAGE_FORMAT scsiModePage[SCSI_MODE_PAGE_COUNT]; /* mode page */

extern u32 usbMscCurLun;
extern USB_MSC_LUN_INFO usbMscLunInfo[];
extern u8 usbMscLunStat[];
extern u8  gInsertNAND;

u8 Host_Cmd_Format=0;
u8 usb_msc_mode=0;


/* use for performance improvement */
u32	usbPerfReadWriteBlcokStart;		/* store the start block to read/write */
u32	unTotalCycle;			/* the total cycle to write */
u8	DataBufIdx[MAX_SECTION];		/* array to indicate data buf status to sychronize SD writing and usbMsc receiving */
u8	usbPerfReadWriteBlockCount[MAX_SECTION];			/* array to store block counts to read/write within each cycle */
u8	writeRetryCount = 0;
u8	usbScsiWrite10 = 0;
u8	usbScsiRead10 = 0;
u32	sendSize = 0;
u32	sendSizePerBuf = 0;
u8	ucbPerfLastBlock = 0;

extern OS_EVENT* message_MboxEvt;


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize USB Performance Task.

Arguments:

	None.

Return Value:

	None.

*/
s32 usbPerfInit(void)
{

    usbSemPerfEvt = OSSemCreate(0);
    usbSemPerfLastBlock = OSSemCreate(0);
    usbSemPerfCritSec = OSSemCreate(1);
    usbSemPrevSend = OSSemCreate(1);

    /* Create the task */
    OSTaskCreate(USB_PERF_TASK, USB_PERF_TASK_PARAMETER, USB_PERF_TASK_STACK, USB_PERF_TASK_PRIORITY);

    return 1;
}

/*

Routine Description:

	UnInitialize USB Performance Task.

Arguments:

	None.

Return Value:

	None.

*/
s32 usbPerfUnInit(void)
{
	u8	err;

	usbSemPerfEvt = OSSemDel(usbSemPerfEvt, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemPerfEvt Failed = %d\n", err);
		return 0;
	}

	usbSemPerfLastBlock = OSSemDel(usbSemPerfLastBlock, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemPerfLastBlock Failed = %d\n", err);
		return 0;
	}

	usbSemPerfCritSec = OSSemDel(usbSemPerfCritSec, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemPerfCritSec Failed = %d\n", err);
		return 0;
	}

	usbSemPrevSend = OSSemDel(usbSemPrevSend, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemPrevSend Failed = %d\n", err);
		return 0;
	}

    /* Del the task */
//    OSTaskCreate(USB_PERF_TASK, USB_PERF_TASK_PARAMETER, USB_PERF_TASK_STACK, USB_PERF_TASK_PRIORITY);

	err = OSTaskDel(USB_PERF_TASK_PRIORITY);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del USB_PERF_TASK Failed = %d\n", err);
		return 0;
	}

    return 1;
}


/*

Routine Description:

	The USB Performance task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
void usbPerfTask(void* pData)
{
    u8 err;
    s32 ret;


    while (1)
    {
        OSSemPend(usbSemPerfEvt, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemEvt is %d.\n", err);
            continue;
        }

        if (usbScsiWrite10)
        {
            if (usbPerfSdWrite() == 0)
                DEBUG_USB("Error: SD Write Fail.\n");
        }
        else if (usbScsiRead10)
        {
            if (usbPerfUsbSend() == 0)
                DEBUG_USB("Error: USB Send Fail.\n");
        }

    }

}



/*

Routine Description:

	USB Performance Wirte data to SD card.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbPerfSdWrite(void)
{

    u8*	ptrWriteBufAddr;
    u8	i;
    u8	ucProcBlockIdx = 0;
    u8	err;


    /* enter critical section */
    OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);
    /* read out the block idx to find out which section block to write */
    for (ucProcBlockIdx=0; ucProcBlockIdx < MAX_SECTION; ucProcBlockIdx++)
    {
        if (DataBufIdx[ucProcBlockIdx] == 1)
            break;
    }
    ptrWriteBufAddr = (u8*) PKBuf0 + ucProcBlockIdx * USB_MSC_BUF_SIZE;

    /* clear the buffer status */
    DataBufIdx[ucProcBlockIdx] = 0;

    /* exit critical section */
    OSSemPost(usbSemPerfCritSec);

    for (i = 0; i < writeRetryCount; i++)
    {
        if (usbMscFsLunWrite(usbMscCurLun, ptrWriteBufAddr, usbPerfReadWriteBlcokStart, usbPerfReadWriteBlockCount[ucProcBlockIdx]))
        {
            usbPerfReadWriteBlcokStart += usbPerfReadWriteBlockCount[ucProcBlockIdx];
            break;
        }
    }

    /* check if retry count is reached */
    if (i >= writeRetryCount)
    {
        usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, usbPerfReadWriteBlcokStart, SCSI_ASC_WRITE_FAULT, SCSI_ASCQ_WRITE_FAULT, 0);
        DEBUG_USB("Error: Write exceeds retry count.\n");
        return 0;
    }

    /* check if the last cycle */
    if (ucProcBlockIdx == (unTotalCycle - 1))
        OSSemPost(usbSemPerfLastBlock);

    return 1;

}


/*

Routine Description:

	USB Performance Send data to host.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbPerfUsbSend(void)
{

    u8*	ptrReadBufAddr;
    u8	i;
    u8	ucProcBlockIdx = 0;
    u8	err;
    u8	readBlockCount;

    /* enter critical section */
    OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);
    /* read out the block idx to find out which section block to write */
    for (ucProcBlockIdx=0; ucProcBlockIdx < MAX_SECTION; ucProcBlockIdx++)
    {
        if (DataBufIdx[ucProcBlockIdx] == 1)
            break;
    }
    ptrReadBufAddr = (u8*) PKBuf0 + ucProcBlockIdx * USB_MSC_BUF_SIZE;
    readBlockCount = usbPerfReadWriteBlockCount[ucProcBlockIdx];
    /* clear the buffer status */
    DataBufIdx[ucProcBlockIdx] = 0;
    /* exit critical section */
    OSSemPost(usbSemPerfCritSec);

    /* send data */
    if (usbMscSend(USB_EP_BULKIN, ptrReadBufAddr, readBlockCount*USB_MSC_SECTOR_SIZE, &sendSizePerBuf) != 1)
    {
        DEBUG_USB("Error: USB mass storage class device send data error in usbPerfUsbSend.\n");
        return 1;
    }
    sendSize += readBlockCount*USB_MSC_SECTOR_SIZE;

    /* check if the last cycle */
    if (ucProcBlockIdx == (unTotalCycle - 1))
    {
        /* enter critical section */
        OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);
        ucbPerfLastBlock = 1;
        /* exit critical section */
        OSSemPost(usbSemPerfCritSec);
    }

    return 1;

}


/*

Routine Description:

	Initialize USB MSC.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInit(void)
{
    /* Create the semaphore */
    usbMscSem = OSSemCreate(0);

	if ((PIPELINE_READ == 1) ||(PIPELINE_WRITE == 1))
	    usbPerfInit();

    /* Create the task */
    //DEBUG_USB("Trace: USB Mass Storage Class task creating\n");
    OSTaskCreate(USB_MSC_TASK, USB_MSC_TASK_PARAMETER, USB_MSC_TASK_STACK, USB_MSC_TASK_PRIORITY);

    return 1;
}


/*

Routine Description:

	Un-Initialize USB MSC.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscUnInit(void)
{
	u8	err;

	err = OSTaskDel(USB_MSC_TASK_PRIORITY);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del USB_MSC_TASK Failed = %d\n", err);
		return 0;
	}

	usbMscSem = OSSemDel(usbMscSem, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemEvt Failed = %d\n", err);
		return 0;
	}

	if ((PIPELINE_READ == 1) ||(PIPELINE_WRITE == 1))
    	if (usbPerfUnInit() == 0)
    		return 0;

	/* del MSC FS LUNs */
	usbMscFsUnInitLuns();

	/* release dma resource */
	usbReleaseDmaSource();

    return 1;
}


/*

Routine Description:

	The USB MSC task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
void usbMscTask(void* pData)
{
    u32 sizeRet;
    s32 ret;
    extern u8 siuOpMode;
    extern u8 system_busy_flag;

    usbMscInitDev();

    while (1)
    {
        /* receive CBW */
        if (usbMscRecv(USB_EP_BULKOUT, (u8*)&usbMscCbw, sizeof(USB_MSC_CBW), &sizeRet) != 1)
        {
            DEBUG_USB("Error: Receive CBW error.\n");
            continue;
        }

        /* check CBW */
        if ((usbMscCheckCbw(sizeRet)) == 0)
        {
            DEBUG_USB("Error: Format of CBW error.\n");
            usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
            continue;
        }

        UsbIntEna |= USB_INT_ENA_BULKOUT;

        /* save current LUN */
        usbMscCurLun = usbMscCbw.bCBWLUN;

        /* process command */
        switch (usbMscCbw.CBWCB[0])
        {
        case SCSI_CMD_TEST_UNIT_READY:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_TEST_UNIT_READY.\n");
#endif
            iduOSDEnable(1);
            ret = usbMscCmdTestUnitReady((SCSI_CDB_TEST_UNIT_READY*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_REQUEST_SENSE:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_REQUEST_SENSE.\n");
#endif
            ret = usbMscCmdRequestSense((SCSI_CDB_REQUEST_SENSE*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_INQUIRY:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_INQUIRY.\n");
#endif

#if ((HW_BOARD_OPTION != AURUM_DVRBOX)&&(HW_BOARD_OPTION!=ELEGANT_KFCDVR)&&(HW_BOARD_OPTION!=GOS_DVRBOX) && \
    (HW_BOARD_OPTION != ROULE_DOORPHONE)&&(HW_BOARD_OPTION != ROULE_SD8F) &&(HW_BOARD_OPTION != ROULE_SD7N)&&(HW_BOARD_OPTION != LEIYON_DOORPHONE)&& \
    (HW_BOARD_OPTION!=MUSTEK_DVRBOX)&&(HW_BOARD_OPTION!=SUNIN_CARDVR)&&(HW_BOARD_OPTION!=SUNIN1_CARDVR))

            siuOpMode       = SIUMODE_START;
            sysCameraMode   = SYS_CAMERA_MODE_PLAYBACK;
            iduPlaybackMode(640,480,640);
				
		#if((HW_BOARD_OPTION==SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)||(HW_BOARD_OPTION==ULTMOST_SDV)||(HW_BOARD_OPTION==SHUOYING_SDV)||(HW_BOARD_OPTION==SUNWAY_SDV)||(HW_BOARD_OPTION==WENSHING_SDV))
            DisableTVOUTINT();
		#endif

            if (sysTVOutOnFlag)
            {
       			IduVideo_ClearPKBuf(2);

		#if((HW_BOARD_OPTION==SALIX_SDV)||(HW_BOARD_OPTION==ULTMOST_SDV)||(HW_BOARD_OPTION==SHUOYING_SDV)||(HW_BOARD_OPTION==SUNWAY_SDV)||(HW_BOARD_OPTION==WENSHING_SDV))
                uiSetUSBInDisableTV();
		#endif
                ///idu_Stop_Get_Data();
                IduVidBuf0Addr = (u32)PKBuf2;   /* cause of idu buffer size is not enough for TV-out */
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            }
            else
            {
                idu_Stop_Get_Data();
				IduVideo_ClearBuf();
                IduVidBuf0Addr = (u32)iduvideobuff;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            }
            OSMboxPost(message_MboxEvt, "USB");    // Protect wait key circumstance while plug in USB
            osdDrawFillUSBMSC();
            system_busy_flag=1;
            usb_msc_mode=1;

#endif
            ret = usbMscCmdInquiry((SCSI_CDB_INQUIRY*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_MODE_SELECT6:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_MODE_SELECT6.\n");
#endif
            ret = usbMscCmdModeSelect6((SCSI_CDB_MODE_SELECT6*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_MODE_SENSE6:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_MODE_SENSE6.\n");
#endif
            ret = usbMscCmdModeSense6((SCSI_CDB_MODE_SENSE6*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_START_STOP_UNIT:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_START_STOP_UNIT.\n");
#endif
            ret = usbMscCmdStartStopUnit((SCSI_CDB_START_STOP_UNIT*) usbMscCbw.CBWCB);

			/* disable usb by pull-down resistor */
			usbDevEnaCtrl(USB_R_PULL_LOW);

            break;

        case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL.\n");
#endif
            ret = usbMscCmdPreventAllowMediumRemoval((SCSI_CDB_PREVENT_ALLOW_MEDIUM_REMOVAL*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_READ_FORMAT_CAPACITIES:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_READ_FORMAT_CAPACITIES.\n");
#endif
            ret = usbMscCmdReadFormatCapacities((SCSI_CDB_READ_FORMAT_CAPACITIES*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_READ_CAPACITY:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_READ_CAPACITY.\n");
#endif
            ret = usbMscCmdReadCapacity((SCSI_CDB_READ_CAPACITY*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_READ10:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_READ10.\n");
#endif


	#if ((CHIP_OPTION == CHIP_PA9001A) || (CHIP_OPTION == CHIP_PA9001C) || (CHIP_OPTION == CHIP_PA9001D))
            iduOSDDisable_All();
            ret = usbMscCmdRead10((SCSI_CDB_READ10*) usbMscCbw.CBWCB);
            iduOSDEnable(1);
	#elif ((CHIP_OPTION == CHIP_PA9002A)||(CHIP_OPTION == CHIP_PA9002B)||(CHIP_OPTION == CHIP_PA9002C)||(CHIP_OPTION == CHIP_PA9002D))
		{
			u32	unSdramSet;

            #if(HW_BOARD_OPTION == SUNWAY_SDV)
                SYS_CTL0 &= ~(0x00000020); //Disable IDU for LCM; Lucian: 新威是使用CPU 屏. 關掉IDU, "Mass Storage" 仍可顯示. 
            #else
                iduOSDDisable_All();
            #endif

			unSdramSet = SdramTimeCtrl;
			
			SdramTimeCtrl &= (~0x00c00000);	
			SdramTimeCtrl |= 0x00400000;		/* change channel priority: 4>1>2>3, modify for USB txf err when IDU OSD is enabled */

            ret = usbMscCmdRead10((SCSI_CDB_READ10*) usbMscCbw.CBWCB);

			SdramTimeCtrl = unSdramSet;

            #if(HW_BOARD_OPTION == SUNWAY_SDV)

            #else
                iduOSDEnable(1);
            #endif

		}
	#endif

            break;

        case SCSI_CMD_WRITE10:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_WRITE10.\n");
#endif
            #if(HW_BOARD_OPTION == SUNWAY_SDV)
                //新威是使用CPU 屏. 關掉IDU, "Mass Storage" 仍可顯示.
            #else
                iduOSDDisable_All();
            #endif
            
            ret = usbMscCmdWrite10((SCSI_CDB_WRITE10*) usbMscCbw.CBWCB);

            #if(HW_BOARD_OPTION == SUNWAY_SDV)
            #else
                iduOSDEnable(1);
            #endif
            
            break;

        case SCSI_CMD_VERIFY10:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_VERIFY10.\n");
#endif
            ret = usbMscCmdVerify10((SCSI_CDB_VERIFY10*) usbMscCbw.CBWCB);
            break;

        case SCSI_CMD_MODE_SENSE10:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: SCSI_CMD_MODE_SENSE10.\n");
#endif
            ret = usbMscCmdModeSense10((SCSI_CDB_MODE_SENSE10*) usbMscCbw.CBWCB);
            break;

        default:
#if USB_DEBUG_PRT
            DEBUG_USB("Trace: Unknown SCSI command - 0x%02x.\n", usbMscCbw.CBWCB[0]);
#endif
            usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
            ret = 0;
            break;
        }

        /* command processing error */
        if (ret == 0)
        {
            DEBUG_USB("Error: USB mass storage command processing error.\n");
            usbMscCmdUnknown();
            continue;
        }
    }
}

/*

Routine Description:

	USB MSC send data.

Arguments:

	epNum - Endpoint number.
	pData - Data to send.
	dataSize - Data size.
	pDataSizeSent - Data size sent.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscSend(u8 epNum, u8* pData, u32 dataSize, u32* pDataSizeSent)
{
    USB_IO_REQ req;
    u32 size;
    u8 stat;
    u8 err;

    memset((void*)&req, 0, sizeof(USB_IO_REQ));
    req.epNum    = epNum;
    req.pData    = pData;
    req.reqSize  = dataSize;
    req.pRetSize = &size;
    req.pRetStat = &stat;

    if (usbApiSendWait(&req) == 0)
    {
        DEBUG_USB("Error: [usbMscSend]USB API send error.\n");
        return 0;
    }

    if (usbScsiRead10 == 0)
    {
        if ((stat & USB_EP_REQ_STAT_OK) != USB_EP_REQ_STAT_OK)
        {
            DEBUG_USB("Error: USB API send return status not OK.\n");
            return 0;
        }
    }

    if (pDataSizeSent != NULL)
        *pDataSizeSent = size;

    return 1;
}

/*

Routine Description:

	USB MSC receive data.

Arguments:

	epNum - Endpoint number.
	pData - Data to receive.
	dataSize - Data size.
	pDataSizeRecv - Data size received.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscRecv(u8 epNum, u8* pData, u32 dataSize, u32* pDataSizeRecv)
{
    USB_IO_REQ req;
    u32 size;
    u8 stat;

    memset((void*)&req, 0, sizeof(USB_IO_REQ));
    req.epNum    = epNum;
    req.pData    = pData;
    req.reqSize  = dataSize;
    req.pRetSize = &size;
    req.pRetStat = &stat;

    if (usbApiRecvWait(&req) == 0)
    {
        DEBUG_USB("Error: USB API receive error.\n");
        return 0;
    }

    if (stat != USB_EP_REQ_STAT_OK)
    {
        DEBUG_USB("Error: USB API receive return status not OK - 0x%08x.\n", stat);
        return 0;
    }

    if (pDataSizeRecv != NULL)
        *pDataSizeRecv = size;

    return 1;
}

/*

Routine Description:

	USB MSC check CBW.

Arguments:

	size - Size received.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCheckCbw(u32 size)
{

    /* check CBW size */
    if (size != sizeof(USB_MSC_CBW))
        return 0;

    /* check signature of CBW */
    if (usbMscCbw.dCBWSignature != USB_MSC_CBW_SIGNATURE)
        return 0;

    /* check LUN */
    if (usbMscCbw.bCBWLUN >= USB_MSC_MAX_LUN)
        return 0;

    return 1;
}

/*

Routine Description:

	USB MSC send CSW.

Arguments:

	residueSize - Residue size.
	status - Status.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscSendCsw(u32 dataResidue, u8 status)
{
    usbMscCsw.dCSWSignature = USB_MSC_CSW_SIGNATURE;
    usbMscCsw.dCSWTag = usbMscCbw.dCBWTag;
    usbMscCsw.dCSWDataResidue = dataResidue;
    usbMscCsw.bCSWStatus = status;
//	DEBUG_USB("CSW\n");
    usbMscSend(USB_EP_BULKIN, (u8*)&usbMscCsw, sizeof(USB_MSC_CSW), NULL);

    return 1;
}

/*

Routine Description:

	USB MSC set sense data.

Arguments:

	senseKey - Sense key.
	info - Information
	asc - Additional sense code.
	ascq - Additional sense code qualifier.
	senseKeySpec0 - Sense key specific byte 0.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscSetSenseData(u8 senseKey, u32 info, u8 asc, u8 ascq, u8 senseKeySpec0)
{
    /* clear sense data */
    memset((void*)&scsiSenseData, 0, sizeof(SCSI_SENSE_DATA));
    /* set fields of sense data */
    //scsiSenseData.A0B.valid = 0;
    scsiSenseData.A0B.errorCode = SCSI_SENSE_CURRENT_ERROR;
    //scsiSenseData.segmentNumber = 0;
    scsiSenseData.A2B.senseKey = senseKey;
    //scsiSenseData.A2B.incorrectLengthIndicator = 0;
    //scsiSenseData.A2B.endOfMedium = 0;
    //scsiSenseData.A2B.filemark = 0;
    scsiSenseData.information = bSwap32(info);
    scsiSenseData.additionalSenseLength = SCSI_SENSE_AD_LEN;
    //scsiSenseData.commandSpecificInformation = 0;
    scsiSenseData.additionalSenseCode = asc;
    scsiSenseData.additionalSenseCodeQualifier = ascq;
    //scsiSenseData.fieldReplaceableUnitCode = 0;
    scsiSenseData.senseKeySpecific[0] = senseKeySpec0;
    //scsiSenseData.senseKeySpecific[1] = 0;
    //scsiSenseData.senseKeySpecific[2] = 0;

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - test unit ready.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdTestUnitReady(SCSI_CDB_TEST_UNIT_READY* pCdb)
{
    u8 status;

    if (usbMscFsLunGetStat(usbMscCurLun))
    {	/* get lun state is successful */
        status = usbMscLunStat[usbMscCurLun];
        if (status & USB_MSC_LUN_MOUNT)
        {	/* logical unit is mounted */
            if (status & USB_MSC_LUN_START)
            {	/* logical unit is started */

				if (status & USB_MSC_LUN_MEDIA_CHANGED)
				{
					usbMscLunStat[usbMscCurLun] &= ~USB_MSC_LUN_MEDIA_CHANGED;
					usbMscFsLunSetStat(usbMscCurLun);
					
		    		usbMscSetSenseData(SCSI_SK_UNIT_ATTENTION, 0, SCSI_ASC_NOT_READY_TO_READY, SCSI_ASCQ_NOT_READY_TO_READY, 0);
			        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);
				}
				else
				{			
	                usbMscSetSenseData(SCSI_SK_UNIT_ATTENTION, 0, SCSI_ASC_NOT_READY_TO_READY, SCSI_ASCQ_NOT_READY_TO_READY, 0);
                	usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);
				}
            }
            else
            {	/* logical unit is stopped */
                usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_NOT_READY_INIT_REQ, SCSI_ASCQ_NOT_READY_INIT_REQ, 0);
                usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);
            }
        }
        else
        {	/* logical unit is not mounted */
            usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
            usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);
        }
    }
    else
    {	/* get lun state failed */
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);
    }

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - request sense.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdRequestSense(SCSI_CDB_REQUEST_SENSE* pCdb)
{
    u32 len;
    u8 status;



#if 0

    /* check cdb */
    if (pCdb->allocationLength == 0x00)
    {
        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
        return 0;
    }

    /* check lun */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

#else
    /* check lun */
    if (usbMscFsLunGetStat(usbMscCurLun) != 0)
    {    /* check cdb */
	    if (pCdb->allocationLength == 0x00)
	    {
	        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
	        return 0;
	    }
    }
#endif
    /* send sense data */
    len = (pCdb->allocationLength < sizeof(SCSI_SENSE_DATA)) ? pCdb->allocationLength : sizeof(SCSI_SENSE_DATA);
    if (usbMscSend(USB_EP_BULKIN, (u8*)&scsiSenseData, len, NULL) != 1)
        return 1;

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - inquiry.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdInquiry(SCSI_CDB_INQUIRY* pCdb)
{
    u32 len;

    /* check cdb */
    if ((pCdb->A1B.logicalUnitNumber >= USB_MSC_MAX_LUN) ||
            (pCdb->A1B.enableVitalProductData != 0) ||
            (pCdb->pageCode != 0))
    {
        DEBUG_USB("Error: Invalid parameter of Inquiry.\n");
		DEBUG_USB("pCdb->A1B.logicalUnitNumber = %d\n", pCdb->A1B.logicalUnitNumber);
		DEBUG_USB("pCdb->A1B.enableVitalProductData = %d\n", pCdb->A1B.enableVitalProductData);
		DEBUG_USB("pCdb->pageCode = %d\n", pCdb->pageCode);
        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
        return 0;
    }

    /* send standard inquiry data */
    len = (pCdb->allocationLength < sizeof(SCSI_STANDARD_INQUIRY_DATA)) ? pCdb->allocationLength : sizeof(SCSI_STANDARD_INQUIRY_DATA);
    if (usbMscSend(USB_EP_BULKIN, (u8*)&scsiStdInquiryData, len, NULL) != 1)
    {
        DEBUG_USB("Error: Send data of Inquiry error.\n");
        return 1;
    }

	/* set usb plugin status to be mass storage */
	usbSetUsbPluginStat(USB_SITUATION_MASS_STORAGE);

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - mode select(6).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdModeSelect6(SCSI_CDB_MODE_SELECT6* pCdb)
{
    SCSI_MODE_PARAM_HEADER6 header = { 0 };
    SCSI_MODE_PARAM_BLOCK_DESC blockDesc = { 0 };
    SCSI_MODE_PAGE_FORMAT page = { 0 };
    u8 remainLen;
    u32 i;

    /* check cdb */
    if (pCdb->A1B.logicalUnitNumber >= USB_MSC_MAX_LUN)
    {
        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
        return 0;
    }

    /* receive mode parameter header (6) */
    if (usbMscRecv(USB_EP_BULKOUT, (u8*)&header, sizeof(SCSI_MODE_PARAM_HEADER6), NULL) != 1)
        return 1;

    /* recieve mode parameter block descriptor */
    remainLen = header.blockDescLength;
    while (remainLen > 0)
    {
        if (usbMscRecv(USB_EP_BULKOUT, (u8*)&blockDesc, sizeof(SCSI_MODE_PARAM_BLOCK_DESC), NULL) != 1)
            return 1;

        remainLen -= sizeof(SCSI_MODE_PARAM_BLOCK_DESC);
    }

    /* recieve mode page */
    remainLen = pCdb->parameterListLength - (sizeof(SCSI_MODE_PARAM_HEADER6) + header.blockDescLength);
    while (remainLen > 0 )
    {
        if (usbMscRecv(USB_EP_BULKOUT, (u8*)&page, 2, NULL) != 1)
            return 1;

        for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
        {	/* search and set mode page if found */
            if (page.A0B.pageCode == scsiModePage[i].A0B.pageCode)
            {
                scsiModePage[i].pageLength = page.pageLength;

                if (usbMscRecv(USB_EP_BULKOUT, (u8*)&scsiModePage[i].modeParam, page.pageLength, NULL) != 1)
                    return 1;

                remainLen -= (page.pageLength + 2);
                break;
            }
        }

        /* no mode page matched */
        if (i >= SCSI_MODE_PAGE_COUNT)
        {
            usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
            return 0;
        }
    }

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - mode sense(6).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdModeSense6(SCSI_CDB_MODE_SENSE6* pCdb)
{
    __align(4) u8 buf[256] = { 0 };
    u8 len = 0;
    u8 pageCode;
    u32 i;

    /* check cdb */
    if (pCdb->A1B.logicalUnitNumber >= USB_MSC_MAX_LUN)
    {
        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
        return 0;
    }

    /* check lun */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }


    /* set write protet */
#if((HW_BOARD_OPTION==SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)||(HW_BOARD_OPTION==SHUOYING_SDV)||(HW_BOARD_OPTION==SUNWAY_SDV)||(HW_BOARD_OPTION==WENSHING_SDV))
    if (gInsertNAND)
        scsiModeParamHeader6.A2B.writeProtect = 1;
    else
        scsiModeParamHeader6.A2B.writeProtect = 0;
#else

	#if (USB_MSC_FUNC_OPTION == USB_MSC_READ_ONLY)
        scsiModeParamHeader6.A2B.writeProtect = 1;	/* set write protect */
	#elif (USB_MSC_FUNC_OPTION == USB_MSC_READ_WRITE)
    if (usbMscLunStat[usbMscCurLun] & USB_MSC_LUN_WRITE_PROTECT)
        scsiModeParamHeader6.A2B.writeProtect = 1;
    else
        scsiModeParamHeader6.A2B.writeProtect = 0;
	#endif

#endif

    /* copy mode parameter header (6) */
    memcpy(&buf[len], &scsiModeParamHeader6, sizeof(SCSI_MODE_PARAM_HEADER6));
    len  += sizeof(SCSI_MODE_PARAM_HEADER6);

    /* copy mode parameter block descriptors */
    if (pCdb->A1B.disableBlockDescriptors == 0)
    {
        for (i = 0; i < SCSI_MODE_PARAM_BLOCK_DESC_COUNT ; i++)
        {
            memcpy(&buf[len], &scsiModeParamBlockDesc[i], sizeof(SCSI_MODE_PARAM_BLOCK_DESC));
            len  += sizeof(SCSI_MODE_PARAM_BLOCK_DESC);
        }
    }

    pageCode = pCdb->A2B.pageCode;
    for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
    {
        if ((pageCode == scsiModePage[i].A0B.pageCode) ||
                (pageCode == SCSI_ALL_PAGE_CODE))
        {
            memcpy(&buf[len], &scsiModePage[i], scsiModePage[i].pageLength + 2);
            len += (scsiModePage[i].pageLength + 2);
        }
    }

    /* send sense data */
    len = (pCdb->allocationLength < len) ? pCdb->allocationLength : len;

    if (usbMscSend(USB_EP_BULKIN, (u8*)&buf[0], len, NULL) != 1)
        return 1;

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - start stop unit.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdStartStopUnit(SCSI_CDB_START_STOP_UNIT* pCdb)
{
    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* load/unload and start/stop */
    if (pCdb->A4B.loadEject)
    {
        if (pCdb->A4B.start)
        {	/* load lun */
            usbMscFsLunLoad(usbMscCurLun);
        }
        else
        {	/* unload lun */
            usbMscFsLunUnload(usbMscCurLun);
        }
    }
    else
    {
        if (pCdb->A4B.start)
        {	/* start lun */
            usbMscFsLunStart(usbMscCurLun);
        }
        else
        {	/* stop lun */
            usbMscFsLunStop(usbMscCurLun);
        }
    }

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - prevent allow medium removal.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdPreventAllowMediumRemoval(SCSI_CDB_PREVENT_ALLOW_MEDIUM_REMOVAL* pCdb)
{
    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    if ((usbMscLunStat[usbMscCurLun] & USB_MSC_LUN_MOUNT) == 0)
    {	/* logical unit is not mounted */
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);
        return 1;
    }

    /*---- Media control ----*/
    if (pCdb->A4B.prevent == 1)
    {	/* prevent medium removal */
        usbMscLunStat[usbMscCurLun] &= ~USB_MSC_LUN_REMOVABLE;
        usbMscFsLunSetStat(usbMscCurLun);
    }
    else
    {	/* allow medium removal */
        usbMscLunStat[usbMscCurLun] |= USB_MSC_LUN_REMOVABLE;
        usbMscFsLunSetStat(usbMscCurLun);
    }

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - read format capacities.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdReadFormatCapacities(SCSI_CDB_READ_FORMAT_CAPACITIES* pCdb)
{
    __align(4) u8 buf[256] = { 0 };
    SCSI_CAPACITY_LIST_HEADER capListHeader = { 0 }; /* capacity list header */
    SCSI_CAPACITY_DESC capDesc = { 0 }; /* capacity descriptor */
    u32 len = 0;

    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {	/* lun is not mounted */
        /* return (capacity list header + maximum capacity descriptor) */
        /* set capacity list header */
        memset((void*)&capListHeader, 0, sizeof(SCSI_CAPACITY_LIST_HEADER));
        capListHeader.capabilityListLength = sizeof(SCSI_CAPACITY_LIST_HEADER);
        memcpy(&buf[len], &capListHeader, sizeof(SCSI_CAPACITY_LIST_HEADER));
        len += sizeof(SCSI_CAPACITY_LIST_HEADER);

        /* set maximum capacity descriptor */
        memset((void*)&capDesc, 0, sizeof(SCSI_CAPACITY_DESC));
        capDesc.numberOfBlock = bSwap32(SCSI_DEF_NUMBER_OF_BLOCK);
#if 1		
        capDesc.A4B.descriptorCode = SCSI_DESC_CODE_UNFORMATTED;
#else
        capDesc.A4B.descriptorCode = SCSI_DESC_CODE_NO_CARTRIDGE;
#endif
        capDesc.blockLength[0] = (u8)((SCSI_DEF_BLOCK_LENGTH & 0x00ff0000) >> 16);
        capDesc.blockLength[1] = (u8)((SCSI_DEF_BLOCK_LENGTH & 0x0000ff00) >> 8);
        capDesc.blockLength[2] = (u8) (SCSI_DEF_BLOCK_LENGTH & 0x000000ff);
        memcpy(&buf[len], &capDesc, sizeof(SCSI_CAPACITY_DESC));
        len += sizeof(SCSI_CAPACITY_DESC);

    }
    else        // NAND MSC go here
    {	/* lun is mounted  */
        /* return (capacity list header + current capacity descriptor + formattable capacity descriptor) */
        /* get lun information */
        if (usbMscFsLunGetInfo(usbMscCurLun) == 0)
        {
            usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
            return 0;
        }
        /* check total sector number and sector size */
	        if ((usbMscLunInfo[usbMscCurLun].sectorCount == 0) || (usbMscLunInfo[usbMscCurLun].sectorSize == 0))
        {
            usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
            return 0;
        }

        /* set capacity list header */
        memset((void*)&capListHeader, 0, sizeof(SCSI_CAPACITY_LIST_HEADER));
        capListHeader.capabilityListLength = sizeof(SCSI_CAPACITY_LIST_HEADER);
        memcpy(&buf[len], &capListHeader, sizeof(SCSI_CAPACITY_LIST_HEADER));
        len += sizeof(SCSI_CAPACITY_LIST_HEADER);

        /* set current capacity descriptor */
        memset((void*)&capDesc, 0, sizeof(SCSI_CAPACITY_DESC));
        capDesc.numberOfBlock = bSwap32(usbMscLunInfo[usbMscCurLun].sectorCount);
        capDesc.A4B.descriptorCode = SCSI_DESC_CODE_FORMATTED;
        capDesc.blockLength[0] = (u8)((usbMscLunInfo[usbMscCurLun].sectorSize & 0x00ff0000) >> 16);
        capDesc.blockLength[1] = (u8)((usbMscLunInfo[usbMscCurLun].sectorSize & 0x0000ff00) >> 8);
        capDesc.blockLength[2] = (u8) (usbMscLunInfo[usbMscCurLun].sectorSize & 0x000000ff);
        memcpy(&buf[len], &capDesc, sizeof(SCSI_CAPACITY_DESC));
        len += sizeof(SCSI_CAPACITY_DESC);

        /* set formattable capacity descriptor */
        capDesc.A4B.descriptorCode = SCSI_DESC_CODE_NONE;
        memcpy(&buf[len], &capDesc, sizeof(SCSI_CAPACITY_DESC));
        len += sizeof(SCSI_CAPACITY_DESC);
    }

    /* send format capacities */
    if (usbMscSend(USB_EP_BULKIN, (u8*)&buf[0], len, NULL) != 1)
        return 1;

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - read capacity.

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdReadCapacity(SCSI_CDB_READ_CAPACITY* pCdb)
{
    __align(4) SCSI_READ_CAPACITY_DATA capData;
	

    /* check if lun is mounted */
    if (usbMscFsLunGetInfo(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }


    /* set capacity data */
    capData.returnLogicalBlockAddress = bSwap32(usbMscLunInfo[usbMscCurLun].sectorCount - 1);
    capData.blockLength = bSwap32(usbMscLunInfo[usbMscCurLun].sectorSize);

    /* send format capacities */
    if (usbMscSend(USB_EP_BULKIN, (u8*)&capData, sizeof(SCSI_READ_CAPACITY_DATA), NULL) != 1)
        return 0;

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}


/*

Routine Description:

	USB MSC SCSI command - read(10).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdRead10(SCSI_CDB_READ10* pCdb)
{
    SCSI_READ_WRITE_ERROR_RECOVERY_PAGE* pPage;
    u8 readRetryCount = 0;
    u32 blockStart;
    u16 blockCount;
    u32 blockSize;
    u32 blockPerBuf;
    u32 i;
    u32 readBlockPerBuf;
    u8* pucReadBuf;
    u8	blockIdx;
    u8	err;

    sendSize = 0;
    sendSizePerBuf = 0;


    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
    {
        /* find read write error recovery page for readRetryCount */
        if (scsiModePage[i].A0B.pageCode == SCSI_READ_WRITE_ERROR_RECOVERY_PAGE_CODE)
        {
            pPage = (SCSI_READ_WRITE_ERROR_RECOVERY_PAGE*) &scsiModePage[i];
            readRetryCount = pPage->readRetryCount;
            break;
        }
    }

    /* get block information */
    blockStart = bSwap32(pCdb->logicalBlockAddress);
    blockCount = bSwap16(pCdb->transferLength);
    blockSize = usbMscLunInfo[usbMscCurLun].sectorSize;
    /* check block count */
    if (blockCount == 0)
    {
        /* send csw */
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);
    }

    /* check block size */
    if (blockSize == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* initialize send information */
    blockPerBuf = USB_MSC_BUF_SIZE / blockSize;
    pucReadBuf = PKBuf0;

#if PIPELINE_READ
    if (blockCount > blockPerBuf)	/* more than one cycle to read */
    {
        usbScsiRead10 = 1;
        usbPerfReadWriteBlcokStart = blockStart;	/* init block read/write start addr */

        /* because don't have ceiling function, use this method to get total cycles */
        unTotalCycle= ( (u32)blockCount / blockPerBuf);
        if (blockCount > (blockPerBuf * unTotalCycle))
            unTotalCycle ++;

        /* initialize data buf index */
        memset(DataBufIdx, 0, MAX_SECTION);
        memset(usbPerfReadWriteBlockCount, 0, MAX_SECTION);

        blockIdx = 0;
        /* read data from lun and send it to host */
        while (blockCount)
        {
            /* calculate read block per buffer */
            readBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* read data */
            for (i = 0; i < readRetryCount; i++)
            {
//    			if (usbMscFsLunRead(usbMscCurLun, &usbMscSendBuf[0][0], blockStart, readBlockPerBuf))
                if (usbMscFsLunRead(usbMscCurLun, pucReadBuf, blockStart, readBlockPerBuf))
                {
                    blockCount -= readBlockPerBuf;
                    blockStart += readBlockPerBuf;
                    break;
                }
            }

            /* check if retry count is reached */
            if (i >= readRetryCount)
            {
                usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, blockStart, SCSI_ASC_READ_ERROR, SCSI_ASCQ_READ_ERROR, 0);
                DEBUG_USB("Error: Read exceeds retry count.\n");
                return 0;
            }

            /* enter critical section */
            OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);
            pucReadBuf += (readBlockPerBuf * blockSize);
            usbPerfReadWriteBlockCount[blockIdx] = readBlockPerBuf;
            DataBufIdx[blockIdx] = 1;
            blockIdx ++;
            /* exit critical section */
            OSSemPost(usbSemPerfCritSec);

            /* post to synchronize SD performance write */
            OSSemPost(usbSemPerfEvt);

        }
        /* wait for the last send cycle completed */
        OSSemPend(usbSemPerfLastBlock, USB_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemPerfLastBlock is %d.\n", err);
            return 0;
        }
        ucbPerfLastBlock = 0;

    }
    else			/* only one cycle to read */
#endif
    {
        usbScsiRead10 = 0;

        /* read data from lun and send it to host */
        while (blockCount)
        {

            /* calculate read block per buffer */
            readBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* read data */
            for (i = 0; i < readRetryCount; i++)
            {
                //			if (usbMscFsLunRead(usbMscCurLun, &usbMscSendBuf[0][0], blockStart, readBlockPerBuf))
                if (usbMscFsLunRead(usbMscCurLun, pucReadBuf, blockStart, readBlockPerBuf))
                {
                    blockCount -= readBlockPerBuf;
                    blockStart += readBlockPerBuf;
                    break;
                }
            }

            /* check if retry count is reached */
            if (i >= readRetryCount)
            {
                usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, blockStart, SCSI_ASC_READ_ERROR, SCSI_ASCQ_READ_ERROR, 0);
                DEBUG_USB("Error: Read exceeds retry count.\n");
                return 0;
            }

            /* send data */
            //if (usbMscSend(USB_EP_BULKIN, (u8*)&usbMscSendBuf[0][0], readBlockPerBuf * blockSize, &sendSizePerBuf) != 1)
            if (usbMscSend(USB_EP_BULKIN, pucReadBuf, readBlockPerBuf * blockSize, &sendSizePerBuf) != 1)
            {
                DEBUG_USB("Error: USB mass storage class device send data error.\n");
                return 1;
            }

            sendSize += sendSizePerBuf;

        }
    }

    usbScsiRead10 = 0;

    /* send csw */
    if (usbMscSendCsw(usbMscCbw.dCBWDataTransferLength - sendSize, USB_MSC_CSW_COMMAND_PASS) != 1)
    {
        DEBUG_USB("Error: USB mass storage class device send status error.\n");
        return 1;
    }

    return 1;
}


/*

Routine Description:

	USB MSC SCSI command - write(10).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdWrite10(SCSI_CDB_WRITE10* pCdb)
{
    SCSI_READ_WRITE_ERROR_RECOVERY_PAGE* pPage;
//	u8 writeRetryCount = 0;
    u32 blockStart;
    u16 blockCount;
    u32 blockSize;
    u32 blockPerBuf;
    u32 i;
    u32 writeBlockPerBuf;
    u32 recvSize = 0;
    u32 recvSizePerBuf = 0;
    u8	err;
    u8	blockIdx;
    u8*	pucWriteBuf;


    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* check if write protect */
    if (usbMscLunStat[usbMscCurLun] & USB_MSC_LUN_WRITE_PROTECT)
    {
        usbMscSetSenseData(SCSI_SK_DATA_PROTECT, 0, SCSI_ASC_WRITE_PROTECTED, SCSI_ASCQ_WRITE_PROTECTED, 0);
        return 0;
    }

    for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
    {	/* find read write error recovery page for writeRetryCount */
        if (scsiModePage[i].A0B.pageCode == SCSI_READ_WRITE_ERROR_RECOVERY_PAGE_CODE)
        {
            pPage = (SCSI_READ_WRITE_ERROR_RECOVERY_PAGE*) &scsiModePage[i];
            writeRetryCount = pPage->writeRetryCount;
            break;
        }
    }

    /* get block information */
    blockStart = bSwap32(pCdb->logicalBlockAddress);
    blockCount = bSwap16(pCdb->transferLength);
    blockSize = usbMscLunInfo[usbMscCurLun].sectorSize;

    /* check block count */
    if (blockCount == 0)
    {
        /* send csw */
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);
    }

    /* check block size */
    if (blockSize == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* clear buffer - it seems to be not necessary */
    //memset((void*) usbMscRecvBuf, 0, USB_MSC_BUF_COUNT * USB_MSC_BUF_SIZE);

    /* initialize receive information */
    blockPerBuf = USB_MSC_BUF_SIZE / blockSize;
    pucWriteBuf = PKBuf0;

#if PIPELINE_WRITE
    if (blockCount > blockPerBuf)			/* more than one cycle to write */
    {
        /* receive data from host and write it to lun */
        usbScsiWrite10 = 1;		/* enter Scsi write10 */

        usbPerfReadWriteBlcokStart = blockStart;	/* init block read/write start addr */

        /* because don't have ceiling function, use this method to get total cycles*/
        unTotalCycle= ( (u32)blockCount / blockPerBuf);
        if (blockCount > (blockPerBuf * unTotalCycle))
            unTotalCycle ++;

        /* initialize data buf index */
        memset(DataBufIdx, 0, MAX_SECTION);
        memset(usbPerfReadWriteBlockCount, 0, MAX_SECTION);

        blockIdx = 0;

        /* receive data from host and write it to lun */
        while (blockCount)
        {
            /* calculate write block per buffer */
            writeBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* receive data */
            if (usbMscRecv(USB_EP_BULKOUT, pucWriteBuf , writeBlockPerBuf * blockSize, &recvSizePerBuf) != 1)
            {
                DEBUG_USB("Error: USB mass storage class device receive data error.\n");
                return 1;
            }
            else
            {     // Should modify the FAT NAND translation layer to fit Host assign MBR
                if (blockStart==0 && gInsertNAND==1)
                {
                    if ( *(u8*)pucWriteBuf == 0xEB || *(u8*)pucWriteBuf == 0xE9 )
                        Host_Cmd_Format=3;      // final state in host format process
                    else
                        Host_Cmd_Format=1;
                }
                // Host delete device data will execute following progress
                // 1. Read MBR and correspond FDB data
                // 2. Set 0x2E to FDB head
                // 3. Clear FAT region
            }

            recvSize += recvSizePerBuf;
            blockCount -= writeBlockPerBuf;

            /* enter critical section */
            OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);

            pucWriteBuf += (writeBlockPerBuf * blockSize);
            usbPerfReadWriteBlockCount[blockIdx] = writeBlockPerBuf;
            DataBufIdx[blockIdx] = 1;
            blockIdx ++;

            /* exit critical section */
            OSSemPost(usbSemPerfCritSec);

            /* post to synchronize SD performance write */
            OSSemPost(usbSemPerfEvt);

        }

        /* wait for the last SD write cycle completed */
        OSSemPend(usbSemPerfLastBlock, USB_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemPerfLastBlock is %d.\n", err);
            return 0;
        }
        usbScsiWrite10 = 0;		/* quit Scsi Write10 */

    }
    else		/* only one cycle to write */
#endif
    {
        /* receive data from host and write it to lun */
        while (blockCount)
        {
            /* calculate write block per buffer */
            writeBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* receive data */
            if (usbMscRecv(USB_EP_BULKOUT, pucWriteBuf, writeBlockPerBuf * blockSize, &recvSizePerBuf) != 1)
            {
                DEBUG_USB("Error: USB mass storage class device receive data error.\n");
                return 1;
            }
            else
            {     // Should modify the FAT NAND translation layer to fit Host assign MBR
                if (blockStart==0 && gInsertNAND==1)
                {
                    if ( *(u8*)pucWriteBuf == 0xEB || *(u8*)pucWriteBuf == 0xE9 )
                        Host_Cmd_Format=3;      // final state in host format process
                    else
                        Host_Cmd_Format=1;
                }
                // Host delete device data will execute following progress
                // 1. Read MBR and correspond FDB data
                // 2. Set 0x2E to FDB head
                // 3. Clear FAT region
            }

            recvSize += recvSizePerBuf;

            /* write data */
            for (i = 0; i < writeRetryCount; i++)
            {
                if (usbMscFsLunWrite(usbMscCurLun, pucWriteBuf, blockStart, writeBlockPerBuf))
                {
                    blockCount -= writeBlockPerBuf;
                    blockStart += writeBlockPerBuf;
                    break;
                }
            }

            /* check if retry count is reached */
            if (i >= writeRetryCount)
            {
                usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, blockStart, SCSI_ASC_WRITE_FAULT, SCSI_ASCQ_WRITE_FAULT, 0);
                DEBUG_USB("Error: Write exceeds retry count.\n");
                return 0;
            }
        }
    }

    /* send csw */
    usbMscSendCsw(usbMscCbw.dCBWDataTransferLength - recvSize, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}


/*

Routine Description:

	USB MSC SCSI command - verify(10).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdVerify10(SCSI_CDB_VERIFY10* pCdb)
{
    SCSI_READ_WRITE_ERROR_RECOVERY_PAGE* pPage;
    u8 readRetryCount = 0;
    u32 blockStart;
    u16 blockCount;
    u32 blockSize;
    u32 blockPerBuf;
    u32 i;
    u32 verifyBlockPerBuf;
    u32 verifySize = 0;
    u32 verifySizePerBuf = 0;

    /* Avoid warning */
    readRetryCount = readRetryCount;
    blockStart = blockStart;

    /* check if lun is mounted */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
    {	/* find read write error recovery page for readRetryCount */
        if (scsiModePage[i].A0B.pageCode == SCSI_READ_WRITE_ERROR_RECOVERY_PAGE_CODE)
        {
            pPage = (SCSI_READ_WRITE_ERROR_RECOVERY_PAGE*) &scsiModePage[i];
            readRetryCount = pPage->readRetryCount;
            break;
        }
    }

    /* get block information */
    blockStart = bSwap32(pCdb->logicalBlockAddress);
    blockCount = bSwap16(pCdb->transferLength);
    blockSize = usbMscLunInfo[usbMscCurLun].sectorSize;

    /* check block count */
    if (blockCount == 0)
    {	/* send csw */
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);
    }

    /* check block size */
    if (blockSize == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* clear buffer - it seems to be not necessary */
    //memset((void*) usbMscRecvBuf, 0, USB_MSC_BUF_COUNT * USB_MSC_BUF_SIZE);
    //memset((void*) usbMscSendBuf, 0, USB_MSC_BUF_COUNT * USB_MSC_BUF_SIZE);

    /* initialize receive information */
    blockPerBuf = USB_MSC_BUF_SIZE / blockSize;

    /* verify */
    if (pCdb->A1B.byteCheck)
    {
        /* compare data received from host with data read from lun */
        while (blockCount)
        {
            /* calculate verify block per buffer */
            verifyBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* receive data */
            if (usbMscRecv(USB_EP_BULKOUT, (u8*)&usbMscRecvBuf[0][0], verifyBlockPerBuf * blockSize, &verifySizePerBuf) != 1)
                return 1;

            verifySize += verifySizePerBuf;

#if 0 /*CY 1023*/
            /* read data */
            for (i = 0; i < readRetryCount; i++)
            {
                if (usbMscFsLunRead(usbMscCurLun, &usbMscSendBuf[0][0], blockStart, verifyBlockPerBuf))
                {
                    blockCount -= verifyBlockPerBuf;
                    blockStart += verifyBlockPerBuf;
                    break;
                }
            }

            /* check if retry count is reached */
            if (i >= readRetryCount)
            {
                usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, blockStart, SCSI_ASC_READ_ERROR, SCSI_ASCQ_READ_ERROR, 0);
                return 0;
            }

            /* compare data from host with data from lun */
            if (memcmp((void*)usbMscRecvBuf, (void*)usbMscSendBuf, verifySizePerBuf) != 0)
                return 0;
#if 0
            for (i = 0; i < verifySizePerBuf; i++)
            {
                if (usbMscRecvBuf[i] != usbMscRecvBuf[i])
                {
                    usbMscSetSenseData(SCSI_SK_MISCOMPARE, blockStart + i, SCSI_ASC_MISCOMPARE, SCSI_ASCQ_MISCOMPARE, 0);
                    return 0;
                }
            }
#endif
#endif
        }

        /* send csw */
        usbMscSendCsw(usbMscCbw.dCBWDataTransferLength - verifySize, USB_MSC_CSW_COMMAND_PASS);
    }
    else
    {	/* data read from lun */
#if 0 /*CY 1023*/
        while (blockCount)
        {
            /* calculate verify block per buffer */
            verifyBlockPerBuf = (blockCount < blockPerBuf) ? blockCount : blockPerBuf;

            /* read data */
            for (i = 0; i < readRetryCount; i++)
            {
                if (usbMscFsLunRead(usbMscCurLun, &usbMscSendBuf[0][0], blockStart, verifyBlockPerBuf))
                {
                    blockCount -= verifyBlockPerBuf;
                    blockStart += verifyBlockPerBuf;
                    break;
                }
            }

            /* check if retry count is reached */
            if (i >= readRetryCount)
            {
                usbMscSetSenseData(SCSI_SK_MEDIUM_ERR, blockStart, SCSI_ASC_READ_ERROR, SCSI_ASCQ_READ_ERROR, 0);
                return 0;
            }
        }
#endif
        /* send csw */
        usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);
    }

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - mode sense(10).

Arguments:

	pCdb - Command descriptor block.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdModeSense10(SCSI_CDB_MODE_SENSE10* pCdb)
{
    __align(4) u8 buf[300];
    u16 len = 0;
    u8 pageCode;
    u32 i;

    /* check cdb */
    if (pCdb->A1B.logicalUnitNumber >= USB_MSC_MAX_LUN)
    {
        usbMscSetSenseData(SCSI_SK_ILLEGAL_REQ, 0, SCSI_ASC_INVALID_CDB, SCSI_ASCQ_INVALID_CDB, 0);
        return 0;
    }

    /* check lun */
    if (usbMscFsLunGetStat(usbMscCurLun) == 0)
    {
        usbMscSetSenseData(SCSI_SK_NOT_READY, 0, SCSI_ASC_MEDIUM_NOT_PRESENT, SCSI_ASCQ_MEDIUM_NOT_PRESENT, 0);
        return 0;
    }

    /* set write protet */
	#if (USB_MSC_FUNC_OPTION == USB_MSC_READ_ONLY)
        scsiModeParamHeader6.A2B.writeProtect = 1;	/* set write protect */
	#elif (USB_MSC_FUNC_OPTION == USB_MSC_READ_WRITE)
    if (usbMscLunStat[usbMscCurLun] & USB_MSC_LUN_WRITE_PROTECT)
        scsiModeParamHeader10.A3B.writeProtect = 1;
    else
        scsiModeParamHeader10.A3B.writeProtect = 0;
	#endif

    /* copy mode parameter header (10) */
    memcpy(&buf[len], &scsiModeParamHeader10, sizeof(SCSI_MODE_PARAM_HEADER10));
    len  += sizeof(SCSI_MODE_PARAM_HEADER10);
#if 0
    /* copy mode parameter block descriptors */
    if (pCdb->A1B.disableBlockDescriptors == 0)
    {
        for (i = 0; i < SCSI_MODE_PARAM_BLOCK_DESC_COUNT ; i++)
        {
            memcpy(&buf[len], &scsiModeParamBlockDesc[i], sizeof(SCSI_MODE_PARAM_BLOCK_DESC));
            len  += sizeof(SCSI_MODE_PARAM_BLOCK_DESC);
        }
    }

    pageCode = pCdb->A2B.pageCode;
    for (i = 0; i < SCSI_MODE_PAGE_COUNT; i++)
    {
        if ((pageCode == scsiModePage[i].A0B.pageCode) ||
                (pageCode == SCSI_ALL_PAGE_CODE))
        {
            memcpy(&buf[len], &scsiModePage[i], scsiModePage[i].pageLength + 2);
            len += (scsiModePage[i].pageLength + 2);
        }
    }
#endif
    /* send sense data */
    len = (pCdb->allocationLength < len) ? pCdb->allocationLength : len;

    if (usbMscSend(USB_EP_BULKIN, (u8*)&buf[0], len, NULL) != 1)
        return 1;

    /* send csw */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_PASS);

    return 1;
}

/*

Routine Description:

	USB MSC SCSI command - unknown.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscCmdUnknown(void)
{
    if (usbMscCbw.dCBWDataTransferLength)
    {	/* stall the related endpoint */
        if (usbMscCbw.bmCBWFlags & USB_MSC_CBW_DIR_IN)
        {	/* stall bulk in */
            usbEpStall(USB_EP_BULKIN);
        }
        else
        {	/* stall bulk out */
            usbEpStall(USB_EP_BULKOUT);
            usbEpStall(USB_EP_BULKIN);
        }
    }

    /* host expects status phase when no data phase or after clear stall */
    usbMscSendCsw(0, USB_MSC_CSW_COMMAND_FAIL);

    return 1;
}

/*

Routine Description:

	Initialize device.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitDev(void)
{
    usbMscFsInitLuns();

    usbMscInitStdInquiryData();
    usbMscInitModePage();

    usbMscInitCb();

    return 1;
}

/*

Routine Description:

	Initialize standard inquiry data.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitStdInquiryData(void)
{
    memset((void*)&scsiStdInquiryData, 0, sizeof(SCSI_STANDARD_INQUIRY_DATA));
    scsiStdInquiryData.A0B.peripheralDeviceType = SCSI_DIRECT_ACCESS_DEVICE;
    scsiStdInquiryData.A0B.peripheralQualifier = SCSI_PHYS_SUPP_CONN;
    scsiStdInquiryData.A1B.removableMedium = 1;
    scsiStdInquiryData.A2B.ansiApprovedVersion = SCSC_ANSI_APPROVED_VERSION;
    scsiStdInquiryData.A2B.ecmaVersion = SCSI_ECMA_VERSION;
    scsiStdInquiryData.A2B.isoVersion = SCSI_ISO_VERSION;
    scsiStdInquiryData.A3B.responseDataFormat = SCSI_ANSI_2;
    //scsiStdInquiryData.A3B.terminateIoProcess = 0;
    //scsiStdInquiryData.A3B.asynchronousEventNotificationCapability = 0;
    scsiStdInquiryData.additionalLength = sizeof(SCSI_STANDARD_INQUIRY_DATA) - 4;
    //scsiStdInquiryData.A7B.softReset = 0;
    //scsiStdInquiryData.A7B.commandQueue = 0;
    //scsiStdInquiryData.A7B.linkedCommand = 0;
    //scsiStdInquiryData.A7B.synchronousTransfer = 0;
    //scsiStdInquiryData.A7B.wideBus16 = 0;
    //scsiStdInquiryData.A7B.wideBus32 = 0;
    //scsiStdInquiryData.A7B.relativeAddress = 0;
    memcpy((void*)scsiStdInquiryData.vendorIdentification, SCSI_VENDOR_ID, sizeof(SCSI_VENDOR_ID));
    memcpy((void*)scsiStdInquiryData.productIdentification, SCSI_PRODUCT_ID, sizeof(SCSI_PRODUCT_ID));
    memcpy((void*)scsiStdInquiryData.productRevisionLevel, SCSI_REVISION_LEVEL, sizeof(SCSI_REVISION_LEVEL));

    return 1;
}

/*

Routine Description:

	Initialize mode pages.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitModePage(void)
{
    u32 i = 0;

    usbMscInitModeParamHeader();
    usbMscInitModeParamBlockDesc();

    usbMscInitControlModePage((SCSI_CONTROL_MODE_PAGE*) &scsiModePage[i++]);
    usbMscInitDisconnReconnPage((SCSI_DISCONN_RECONN_PAGE*) &scsiModePage[i++]);
    usbMscInitPeripheralDevicePage((SCSI_PERIPHERAL_DEVICE_PAGE*) &scsiModePage[i++]);
    usbMscInitCachePage((SCSI_CACHE_PAGE*) &scsiModePage[i++]);
    usbMscInitFormatDevicePage((SCSI_FORMAT_DEVICE_PAGE*) &scsiModePage[i++]);
    usbMscInitMediumTypeSupportedPage((SCSI_MEDIUM_TYPE_SUPPORTED_PAGE*) &scsiModePage[i++]);
    usbMscInitNotchPage((SCSI_NOTCH_PAGE*) &scsiModePage[i++]);
    usbMscInitReadWriteErrorRecoveryPage((SCSI_READ_WRITE_ERROR_RECOVERY_PAGE*) &scsiModePage[i++]);
    usbMscInitVerifyErrorRecoveryPage((SCSI_VERIFY_ERROR_RECOVERY_PAGE*) &scsiModePage[i++]);

    return 1;
}

/*

Routine Description:

	Initialize mode parameter header.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitModeParamHeader(void)
{
    memset((void*)&scsiModeParamHeader6, 0, sizeof(SCSI_MODE_PARAM_HEADER6));
    scsiModeParamHeader6.modeDataLength =
        (sizeof(SCSI_MODE_PARAM_HEADER6) - 1) +
        sizeof(SCSI_MODE_PARAM_BLOCK_DESC) * SCSI_MODE_PARAM_BLOCK_DESC_COUNT +
        sizeof(SCSI_CONTROL_MODE_PAGE) +
        sizeof(SCSI_DISCONN_RECONN_PAGE) +
        sizeof(SCSI_PERIPHERAL_DEVICE_PAGE) +
        sizeof(SCSI_CACHE_PAGE) +
        sizeof(SCSI_FORMAT_DEVICE_PAGE) +
        sizeof(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE) +
        sizeof(SCSI_NOTCH_PAGE) +
        sizeof(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE) +
        sizeof(SCSI_VERIFY_ERROR_RECOVERY_PAGE);
    scsiModeParamHeader6.mediumType = SCSI_MEDIUM_TYPE_DEFAULT;
    //scsiModeParamHeader6.deviceSpecificParam = 0;
    scsiModeParamHeader6.blockDescLength = sizeof(SCSI_MODE_PARAM_BLOCK_DESC) * SCSI_MODE_PARAM_BLOCK_DESC_COUNT;

    memset((void*)&scsiModeParamHeader10, 0, sizeof(SCSI_MODE_PARAM_HEADER10));
    scsiModeParamHeader10.modeDataLength =
        bSwap16(
            (sizeof(SCSI_MODE_PARAM_HEADER10) - 2) +
            sizeof(SCSI_MODE_PARAM_BLOCK_DESC) * SCSI_MODE_PARAM_BLOCK_DESC_COUNT +
            sizeof(SCSI_CONTROL_MODE_PAGE) +
            sizeof(SCSI_DISCONN_RECONN_PAGE) +
            sizeof(SCSI_PERIPHERAL_DEVICE_PAGE) +
            sizeof(SCSI_CACHE_PAGE) +
            sizeof(SCSI_FORMAT_DEVICE_PAGE) +
            sizeof(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE) +
            sizeof(SCSI_NOTCH_PAGE) +
            sizeof(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE) +
            sizeof(SCSI_VERIFY_ERROR_RECOVERY_PAGE)
        );
    scsiModeParamHeader10.mediumType = SCSI_MEDIUM_TYPE_DEFAULT;
    //scsiModeParamHeader10.deviceSpecificParam = 0;
    scsiModeParamHeader10.blockDescLength =
        bSwap16(
            sizeof(SCSI_MODE_PARAM_BLOCK_DESC) * SCSI_MODE_PARAM_BLOCK_DESC_COUNT
        );

    return 1;

}

/*

Routine Description:

	Initialize mode parameter block descriptor.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitModeParamBlockDesc(void)
{
    u32 i;

    for (i = 0; i < SCSI_MODE_PARAM_BLOCK_DESC_COUNT; i++)
    {
        memset((void*)&scsiModeParamBlockDesc[i], 0, sizeof(SCSI_MODE_PARAM_BLOCK_DESC));
        //scsiModeParamBlockDesc[i].densityCode = 0;
        //scsiModeParamBlockDesc[i].numberOfBlocks[0] = 0;
        //scsiModeParamBlockDesc[i].numberOfBlocks[1] = 0;
        //scsiModeParamBlockDesc[i].numberOfBlocks[2] = 0;
        //scsiModeParamBlockDesc[i].blockLength[0] = 0;
        //scsiModeParamBlockDesc[i].blockLength[1] = 0;
        //scsiModeParamBlockDesc[i].blockLength[2] = 0;
    }

    return 1;
}

/*

Routine Description:

	Initialize control mode page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitControlModePage(SCSI_CONTROL_MODE_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_CONTROL_MODE_PAGE));
    pPage->A0B.pageCode = SCSI_CONTROL_MODE_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_CONTROL_MODE_PAGE) - 2;
    //pPage->A2B.reportLogExceptionCondition = 0;
    //pPage->A3B.disableQueue = 0;
    //pPage->A3B.queueErrorManagement = 0;
    //pPage->A3B.queueAlgorithmModifier = 0;
    //pPage->A4B.errorAenPermission = 0;
    //pPage->A4B.unitAttentionAenPermission = 0;
    //pPage->A4B.readyAenPermission = 0;
    //pPage->A4B.enableExtendedContingentAlliance = 0;
    //pPage->readyAenHoldoffPeriod = bSwap(0);

    return 1;
}

/*

Routine Description:

	Initialize disconnect reconnect page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitDisconnReconnPage(SCSI_DISCONN_RECONN_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_DISCONN_RECONN_PAGE));
    pPage->A0B.pageCode = SCSI_DISCONN_RECONN_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_DISCONN_RECONN_PAGE) - 2;
    pPage->bufferFullRatio = 0x80;
    pPage->bufferEmptyRatio = 0x80;
    //pPage->busInactLimit = bSwap16(0);
    //pPage->disconnTimeLimit = bSwap16(0);
    //pPage->connTimeLimit = bSwap16(0);
    //pPage->maxBurstSize = bSwap16(0);
    pPage->A12B.dataTransferDisconnectControl = SCSI_DISCONN_CTRL_NONE;

    return 1;
}

/*

Routine Description:

	Initialize peripheral device page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitPeripheralDevicePage(SCSI_PERIPHERAL_DEVICE_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_PERIPHERAL_DEVICE_PAGE));
    pPage->A0B.pageCode = SCSI_PERIPHERAL_DEVICE_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_PERIPHERAL_DEVICE_PAGE) - 2;
    pPage->interfaceIdentifier = bSwap16(SCSI_IF_ID_SCSI);

    return 1;
}

/*

Routine Description:

	Initialize cache page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitCachePage(SCSI_CACHE_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_CACHE_PAGE));
    pPage->A0B.pageCode = SCSI_CACHE_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_CACHE_PAGE) - 2;
    //pPage->A2B.readCacheDisable = 0;
    //pPage->A2B.multiplyFactor = 0;
    //pPage->A2B.writeCacheEnable = 0;
    //pPage->A3B.writeRetentionPriority = 0;
    //pPage->A3B.demandReadRetentionPriority = 0;
    //pPage->disablePrefetchTransferLength = bSwap16(0);
    //pPage->minPrefetch = bSwap16(0);
    //pPage->maxPrefetch = bSwap16(0);
    //pPage->maxPrefetchCeiling = bSwap16(0);

    return 1;
}

/*

Routine Description:

	Initialize format device page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitFormatDevicePage(SCSI_FORMAT_DEVICE_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_FORMAT_DEVICE_PAGE));
    pPage->A0B.pageCode = SCSI_FORMAT_DEVICE_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_FORMAT_DEVICE_PAGE) - 2;
    //pPage->trackPerZone = bSwap16(0);
    //pPage->altSectorPerZone = bSwap16(0);
    //pPage->altTrackPerZone = bSwap16(0);
    //pPage->altTrackPerLogicalUnit = bSwap16(0);
    //pPage->sectorPerTrack = bSwap16(0);
    //pPage->bytePerPhysicalSector = bSwap16(0);
    //pPage->interleave = bSwap16(0);
    //pPage->trackSkewFactor = bSwap16(0);
    //pPage->cylinderSkewFactor = bSwap16(0);
    //pPage->A20B.surface = 0;
    pPage->A20B.removable = 1;
    //pPage->A20B.hardSector = 0;
    //pPage->A20B.softSector = 0;

    return 1;
}

/*

Routine Description:

	Initialize medium type supported page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitMediumTypeSupportedPage(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE));
    pPage->A0B.pageCode = SCSI_MEDIUM_TYPE_SUPPORTED_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_MEDIUM_TYPE_SUPPORTED_PAGE) - 2;
    pPage->mediumTypeOneSupported = SCSI_MEDIUM_TYPE_DEFAULT;
    pPage->mediumTypeTwoSupported = SCSI_MEDIUM_TYPE_DEFAULT;
    pPage->mediumTypeThreeSupported = SCSI_MEDIUM_TYPE_DEFAULT;
    pPage->mediumTypeFourSupported = SCSI_MEDIUM_TYPE_DEFAULT;

    return 1;
}

/*

Routine Description:

	Initialize notch page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitNotchPage(SCSI_NOTCH_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_NOTCH_PAGE));
    pPage->A0B.pageCode = SCSI_NOTCH_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_NOTCH_PAGE) - 2;
    //pPage->A2B.logicalPhysicalNotch = 0;
    //pPage->A2B.notchDrive = 0;
    //pPage->maxNumberOfNotch = bSwap16(0);
    //pPage->activeNotch = bSwap16(0);
    //pPage->startBoundary = bSwap16(0);
    //pPage->endBoundary = bSwap16(0);
    //pPage->pageNotch[0] = 0;
    //pPage->pageNotch[1] = 0;
    //pPage->pageNotch[2] = 0;
    //pPage->pageNotch[3] = 0;
    //pPage->pageNotch[4] = 0;
    //pPage->pageNotch[5] = 0;
    //pPage->pageNotch[6] = 0;
    //pPage->pageNotch[7] = 0;

    return 1;
}

/*

Routine Description:

	Initialize read write error recovery page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitReadWriteErrorRecoveryPage(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE));
    pPage->A0B.pageCode = SCSI_READ_WRITE_ERROR_RECOVERY_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_READ_WRITE_ERROR_RECOVERY_PAGE) - 2;
    pPage->A2B.disableCorrection = 1;
    //pPage->A2B.disableTransferOnError = 0;
    //pPage->A2B.postError = 0;
    //pPage->A2B.enableEarlyRecovery = 0;
    //pPage->A2B.readContinuous = 0;
    //pPage->A2B.transferBlock = 0;
    //pPage->A2B.autoReadReallocEnable = 0;
    //pPage->A2B.autoWriteReallocEnable = 0;
    pPage->readRetryCount = 1;
    //pPage->correctionSpan = 0;
    //pPage->headOffsetCount = 0;
    //pPage->dataStrobeOffsetCount = 0;
    pPage->writeRetryCount = 1;
    pPage->recoveryTimeLimit = bSwap16(100);

    return 1;
}

/*

Routine Description:

	Initialize verify error recovery page.

Arguments:

	pPage - The page.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitVerifyErrorRecoveryPage(SCSI_VERIFY_ERROR_RECOVERY_PAGE* pPage)
{
    memset((void*)pPage, 0, sizeof(SCSI_VERIFY_ERROR_RECOVERY_PAGE));
    pPage->A0B.pageCode = SCSI_VERIFY_ERROR_RECOVERY_PAGE_CODE;
    //pPage->A0B.parameterSavable = 0;
    pPage->pageLength = sizeof(SCSI_VERIFY_ERROR_RECOVERY_PAGE) - 2;
    pPage->A2B.disableCorrection = 1;
    //pPage->A2B.disableTransferOnError = 0;
    //pPage->A2B.postError = 0;
    //pPage->A2B.enableEarlyRecovery = 0;
    pPage->verifyRetryCount = 1;
    //pPage->verifyCorrectionSpan = 0;
    pPage->verifyRecoveryTimeLimit= bSwap16(100);

    return 1;
}
