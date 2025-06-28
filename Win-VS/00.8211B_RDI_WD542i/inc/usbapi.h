/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbapi.h

Abstract:

   	The application interface of the USB controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __USB_API_H__
#define __USB_API_H__

/* Constant */

/* status */
#define USB_MSC_LUN_MOUNT		0x01	/* mount */
#define USB_MSC_LUN_START		0x02	/* start */
#define USB_MSC_LUN_WRITE_PROTECT	0x04	/* write protect */
#define USB_MSC_LUN_REMOVABLE		0x08    /* removable */
#define USB_MSC_LUN_MEDIA_CHANGED		0x10    /* Media Changed */
#define USB_MSC_LUN_MEDIA_REMOVED		0x20	/* Media Removed */


#if((HW_BOARD_OPTION == MMR6720_EBELL)||(HW_BOARD_OPTION==AOYA_DVRBOX)|| (HW_BOARD_OPTION==YUXIN_DVRBOX)||\
    (HW_BOARD_OPTION==KONLIKA_DVRBOX)|| (HW_BOARD_OPTION==STONE_DVRBOX)|| (HW_BOARD_OPTION==JSM_DVRBOX)|| \
    (HW_BOARD_OPTION==SKYBEST_DVRBOX)|| (HW_BOARD_OPTION==CWELL_DVRBOX)|| (HW_BOARD_OPTION==JSW_DVRBOX)||\
    (HW_BOARD_OPTION==RDI_CARMREC)||(HW_BOARD_OPTION==MARS_DVRBOX)||(HW_BOARD_OPTION==EVERSPRING_DVRBOX)||\
    (HW_BOARD_OPTION == AURUM_DVRBOX)||(HW_BOARD_OPTION == ELEGANT_KFCDVR)||(HW_BOARD_OPTION==GOS_DVRBOX)||\
    (HW_BOARD_OPTION==VER100_CARDVR)||(HW_BOARD_OPTION==ULTMOST_SDV)||(HW_BOARD_OPTION==SHUOYING_SDV)||\
    (HW_BOARD_OPTION==SUNWAY_SDV)||(HW_BOARD_OPTION==WINNIN_IRDVR)||(HW_BOARD_OPTION==DTY_IRDVR)||\
    (HW_BOARD_OPTION==ALM_DVRBOX)||(HW_BOARD_OPTION==RDI_DOORPHONE_774)||(HW_BOARD_OPTION==SIYUAN_CVR)||\
    (HW_BOARD_OPTION==D010_CARDVR)||(HW_BOARD_OPTION==PROJECT_MR8980_6720)||(HW_BOARD_OPTION==NEW_SIYUAN)||\
    (HW_BOARD_OPTION==PROJECT_OPCOM_REAL)||(HW_BOARD_OPTION==PROJECT_DW950_REAL)||(HW_BOARD_OPTION==ACT611_DVRBOX)||\
    (HW_BOARD_OPTION==ES_LIGHTING)||(HW_BOARD_OPTION==WENSHING_SDV)||(HW_BOARD_OPTION==KD_DVRBOX)||\
    (HW_BOARD_OPTION==JSY_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==SUNIN_CARDVR)||(HW_BOARD_OPTION==SUNIN1_CARDVR)||\
    (HW_BOARD_OPTION==ITS_CARDVR)||(HW_BOARD_OPTION==DEMO_DVR)||(HW_BOARD_OPTION==AV9AV_DVR)||(HW_BOARD_OPTION==SEENEDGE_CARDVR))
#define USB_R_PULL_LOW		0x01	/* pull low resistor */
#define USB_R_PULL_HIGH		0x00	/* pull high resistor */
#else
#define USB_R_PULL_LOW		0x00	/* pull low resistor */
#define USB_R_PULL_HIGH		0x01	/* pull high resistor */
#endif
/* Structure */

/* USB MSC LUN information */
typedef struct _USB_MSC_LUN_INFO
{
	u32	sectorCount;
	u32	sectorSize;
} USB_MSC_LUN_INFO;

/* USB MSC file system function table */
typedef struct _USB_MSC_FS_FUNC_TABLE
{
	s32 (*checkMount)(void);
	s32 (*getInfo)(USB_MSC_LUN_INFO*);
	s32 (*getStat)(u8*);
	s32 (*setStat)(u8);
	s32 (*physRead)(u8*, u32, u32);
	s32 (*physWrite)(u8*, u32, u32);
} USB_MSC_FS_FUNC_TABLE;

/* Extern Function prototype */
extern s32 usbInit(void);
extern void usbIntHandler(void);
extern void usbTest(void);
extern void usbInitLuns(void);
extern void usbDevEnaCtrl(u8);


extern void usbCbAttach(u32);
extern void usbCbDetach(u32);
extern void usbCbBusReset(u32);
extern void usbCbBusSuspend(u32);
extern void usbCbGetStatus(u32);
extern void usbCbClearFeature(u32);
extern void usbCbSetFeature(u32);
extern void usbCbSetAddress(u32);
extern void usbCbGetDescriptor(u32);
extern void usbCbSetDescriptor(u32);
extern void usbCbGetConfiguration(u32);
extern void usbCbSetConfiguration(u32);
extern void usbCbGetInterface(u32);
extern void usbCbSetInterface(u32);
extern void usbCbSynchFrame(u32);
extern void usbCbClassReq(u32);
extern void usbCbVendorReq(u32);

extern s32 usbEpReset(u8);
extern s32 usbEpClearFifo(u8);
extern s32 usbEpStall(u8);
extern s32 usbEpClearStall(u8);
extern s32 usbGetDevReq(u32*);
extern s32 usbEpWriteData(u8, u8*, u32);
extern s32 usbEpReadData(u8, u8*, u32);
extern s32 usbGetEpReadData(u8, u8*, u32*);
extern s32 usbSetReadData(u32, u8*, u32);
extern s32 usbSetWriteData(u32, u8*, u32);
extern s32 usbSetReadDataDma(u32, u8*, u32);
extern s32 usbSetWriteDataDma(u32, u8*, u32);
extern s32 usbCheckDmaReady(void);

extern s32 usbClassGetDevDesc(u32*, u32*);
extern s32 usbClassGetCfgDesc(u8, u32*, u32*);
extern s32 usbClassGetStrDesc(u8, u32*, u32*);
extern s32 usbClassGetDevQualDesc(u32*, u32*);
extern s32 usbClassGetOtherSpeedCfgDesc(u8, u32*, u32*);

extern s32 usbDevSuspend(void);
extern s32 usbDevResume(void);

extern s32 usbApiSendRecvNotify(u8);





extern void usbIntEvtAttach(void);
extern void usbIntEvtDetach(void);
extern void usbIntEvtCtrlIn(void);
extern void usbIntEvtCtrlOut(void);
extern void usbIntEvtSetup(void);
extern void usbIntEvtSetupOverw(void);
extern void usbIntEvtBulkIn(void);
extern void usbIntEvtBulkOut(void);
extern void usbIntEvtReset(void);
extern void usbIntEvtSuspend(void);
extern void usbIntEvtResume(void);
extern void usbIntEvtSof(void);
extern void usbIntEvtCtrlInNak(void);
extern void usbIntEvtCtrlOutNak(void);
extern void usbIntEvtBulkInNak(void);
extern void usbIntEvtBulkOutNak(void);
extern void usbIntEvtCtrlInStall(void);
extern void usbIntEvtCtrlOutStall(void);
extern void usbIntEvtIntrIn(void);
extern void usbIntEvtIntrInNak(void);
extern void usbIntEvtIsoIn(void);
extern void usbIntEvtIsoInZeroL(void);
extern void usbIntEvtBulkInDmaCmpl(void);
extern void usbIntEvtBulkOutDmaCmpl(void);
extern void usbIntEvtIsoInDmaCmpl(void);

extern void usbApiEvtTerminate(void);
extern void usbApiEvtSend(void);
extern void usbApiEvtRecv(void);
extern void usbApiEvtEpCancel(void);
extern void usbApiEvtEpReset(void);
extern void usbApiEvtEpStall(void);
extern void usbApiEvtEpClearStall(void);
extern void usbApiEvtRemoteWakeup(void);
extern void usbApiEvtQueryStatus(void);

extern s32 usbDeviceInit(void);
extern s32 usbClassCfg(void);
extern s32 usbClassInit(void);

extern u32 usbGetIntStat(void);

extern u8 usbSetUsbPluginStat(u8);
extern u8 usbSetInitUsb(void);

extern s32 usbMscInit(void);
extern s32 usbMscUnInit(void);
extern s32 usbPerfInit(void);
extern s32 usbPerfUnInit(void);


extern s32 usbMscFsInitLuns(void);
extern s32 usbMscFsUnInitLuns(void);
extern s32 usbMscFsLunGetInfo(u32);
extern s32 usbMscFsLunGetStat(u32);
extern s32 usbMscFsLunSetStat(u32);
extern s32 usbMscFsLunRead(u32, u8*, u32, u32);
extern s32 usbMscFsLunWrite(u32, u8*, u32, u32);
extern s32 usbMscFsLunLoad(u32);
extern s32 usbMscFsLunUnload(u32);
extern s32 usbMscFsLunStart(u32);
extern s32 usbMscFsLunStop(u32);

extern s32 usbMscInitCb(void);

extern s32 usbEpStall(u8);

extern s32 usbReleaseDmaSource(void);


#if((HW_BOARD_OPTION==SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)||(HW_BOARD_OPTION==ULTMOST_SDV)||(HW_BOARD_OPTION==SHUOYING_SDV)||(HW_BOARD_OPTION==SUNWAY_SDV)||(HW_BOARD_OPTION==WENSHING_SDV))
extern void DisableTVOUTINT(void);
extern void uiSetUSBInDisableTV(void);
#endif


/* Extern Global Variable */
extern u8	gucUsbPlugInStat;	/* a flag to indicate the usb plug-in status. */


/* Define Constant */

/* define for USB plug-in status */
#define USB_PLUGIN_STAT_NONE			0	
#define USB_PLUGIN_STAT_ONLY_POWER		0x01
#define USB_PLUGIN_STAT_PC				0x02
#define USB_PLUGIN_STAT_MASS_STORAGE	0x04

/* define for USB plug-in situration */
#define USB_SITUATION_NONE				0
#define USB_SITUATION_BOOT				1
#define USB_SITUATION_PC				2
#define USB_SITUATION_MASS_STORAGE		3


#endif
