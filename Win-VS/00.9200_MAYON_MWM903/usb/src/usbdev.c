/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbdev.c

Abstract:

   	USB device specific function.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbreg.h"
#include "usbdev.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbSetDevAddr(u16);

s32 usbEpCancel(u8);
s32 usbEpReset(u8);
s32 usbEpClearFifo(u8);
s32 usbEpStall(u8);
s32 usbEpClearStall(u8);

s32 usbRemoteWakeup(void);

u32 usbGetIntStat(void);
s32 usbGetDevReq(u32*);

s32 usbGetEpReadData(u8, u8*, u32*);
s32 usbEpReadData(u8, u8*, u32);
s32 usbEpWriteData(u8, u8*, u32);

s32 usbSetReadData(u32, u8*, u32);
s32 usbSetWriteData(u32, u8*, u32);
s32 usbSetReadDataDma(u32, u8*, u32);
s32 usbSetWriteDataDma(u32, u8*, u32);
s32 usbCheckDmaReadReady(void);
s32 usbCheckDmaWriteReady(void);
s32 usbReleaseDmaSource(void);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbDevDebugPrint 				printf

//#define USB_TIMEOUT		100

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

u32 usbEpMaxPktSize[USB_EP_MAX];
u32 usbEpMaxDmaSize[USB_EP_MAX];
u32 usbPrevBulkOutDataLen;

extern USB_DEV_SETTING usbDevSetting;

extern u8	usbScsiRead10;
extern u8 	usbScsiWrite10;
u32 guiUSBReadDMAId=0xFF, guiUSBWriteDMAId=0xFF;


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	USB device initialization.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDeviceInit(void)
{
    usbEpMaxPktSize[USB_EP_CTRLIN]  = 0x00000008;
    usbEpMaxPktSize[USB_EP_CTRLOUT] = 0x00000008;
    usbEpMaxPktSize[USB_EP_BULKIN]  = 0x00000040;
    usbEpMaxPktSize[USB_EP_BULKOUT] = 0x00000040;
    usbEpMaxPktSize[USB_EP_INTRIN]  = 0x00000008;
    usbEpMaxPktSize[USB_EP_ISOIN]   = 0x00000000;

    usbEpMaxDmaSize[USB_EP_CTRLIN]  = 0x00000008;
    usbEpMaxDmaSize[USB_EP_CTRLOUT] = 0x00000008;
    usbEpMaxDmaSize[USB_EP_BULKIN]  = 0x00001000;
    usbEpMaxDmaSize[USB_EP_BULKOUT] = 0x00001000;
    usbEpMaxDmaSize[USB_EP_INTRIN]  = 0x00000008;
    usbEpMaxDmaSize[USB_EP_ISOIN]   = 0x000003c0;

    UsbIntEna = USB_INT_ENA_CTRLIN |
                USB_INT_ENA_CTRLOUT |
                USB_INT_ENA_SETUP |
                USB_INT_ENA_SETUP_OVERW |
                USB_INT_ENA_BULKIN |
                USB_INT_ENA_BULKOUT	|
                USB_INT_ENA_RESET |
                //USB_INT_ENA_SUSPEND |
                //USB_INT_ENA_RESUME |
                //USB_INT_ENA_SOF |
                //USB_INT_ENA_CTRLIN_NAK |
                //USB_INT_ENA_CTRLOUT_NAK |
                //USB_INT_ENA_BULKIN_NAK |
                //USB_INT_ENA_BULKOUT_NAK |
                USB_INT_ENA_CTRLIN_STALL |
                USB_INT_ENA_CTRLOUT_STALL |
//                USB_INT_ENA_INTRIN |
                //USB_INT_ENA_INTRIN_NAK |
//                USB_INT_ENA_ISOIN |
//                USB_INT_ENA_ISOIN_ZEROL |
                USB_INT_ENA_BULKIN_DMA_CMPL |
                USB_INT_ENA_BULKOUT_DMA_CMPL |
                USB_INT_ENA_ISOIN_DMA_CMPL;

    UsbCtrl = USB_C_BULKINOUT_INTRIN_ENA |
//              USB_C_ISOIN_ENA |
              USB_C_CONNECT;


    return 1;
}

/*

Routine Description:

	Set device address.

Arguments:

	addr - Device address.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetDevAddr(u16 addr)
{
    UsbDevAddr = addr;

    return 1;
}

/*

Routine Description:

	Endpoint cancel.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpCancel(u8 epNum)
{
    /* TBD: cancel endpoint transfer */
    switch (epNum)
    {
    case USB_EP_CTRLIN:
        break;

    case USB_EP_CTRLOUT:
        break;

    case USB_EP_BULKIN:
        break;

    case USB_EP_BULKOUT:
        break;

    case USB_EP_INTRIN:
        break;

    case USB_EP_ISOIN:
        break;

    default:
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint reset.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpReset(u8 epNum)
{
    /* TBD: reset endpoint */
    switch (epNum)
    {
    case USB_EP_CTRLIN:
        /* Reset endpoint 0 */
        UsbEp0CtrlStat = 0x00000000;
        break;

    case USB_EP_CTRLOUT:
        break;

    case USB_EP_BULKIN:
        UsbEp1CtrlStat = 0x00000000;
        break;

    case USB_EP_BULKOUT:
        UsbEp2CtrlStat = 0x00000000;
        break;

    case USB_EP_INTRIN:
        break;

    case USB_EP_ISOIN:
        break;

    default:
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint clear fifo.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpClearFifo(u8 epNum)
{
    /* TBD: reset endpoint */
    switch (epNum)
    {
    case USB_EP_CTRLIN:
        break;

    case USB_EP_CTRLOUT:
        break;

    case USB_EP_BULKIN:
        UsbEp1CtrlStat = 0x00000020;
        break;

    case USB_EP_BULKOUT:
        UsbEp2CtrlStat = 0x00000040;
        break;

    case USB_EP_INTRIN:
        break;

    case USB_EP_ISOIN:
        break;

    default:
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint stall.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpStall(u8 epNum)
{
    //DEBUG_USB("Trace: Stall endpoint %d.\n", epNum);

    switch (epNum)
    {
    case USB_EP_CTRLIN:
        UsbEp0CtrlStat |= USB_CS_CTRLIN_STALL;
        usbDevSetting.epStat[USB_EP_CTRLIN] |= USB_DEV_STAT_HALT;
        break;

    case USB_EP_CTRLOUT:
        UsbEp0CtrlStat |= USB_CS_CTRLOUT_STALL;
        usbDevSetting.epStat[USB_EP_CTRLOUT] |= USB_DEV_STAT_HALT;
        break;

    case USB_EP_BULKIN:
        UsbEp1CtrlStat |= USB_CS_BULKIN_STALL | USB_CS_BULKIN_DATA_TOGGLE_CLR;	/* clear data toggle bit */
        usbDevSetting.epStat[USB_EP_BULKIN] |= USB_DEV_STAT_HALT;
        break;

    case USB_EP_BULKOUT:
        UsbEp2CtrlStat |= USB_CS_BULKOUT_STALL | USB_CS_BULKOUT_DATA_TOGGLE_CLR;	/* clear data toggle bit */;
        usbDevSetting.epStat[USB_EP_BULKOUT] |= USB_DEV_STAT_HALT;
        break;

    case USB_EP_INTRIN:
        UsbEp3CtrlStat |= USB_CS_INTRIN_STALL;
        usbDevSetting.epStat[USB_EP_INTRIN] |= USB_DEV_STAT_HALT;
        break;

        /*
        case USB_EP_ISOIN:
        	break;
        */

    default:
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint clear stall.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpClearStall(u8 epNum)
{
    switch (epNum)
    {
    case USB_EP_CTRLIN:
        UsbEp0CtrlStat &= ~USB_CS_CTRLIN_STALL;
        usbDevSetting.epStat[USB_EP_CTRLIN] &= ~USB_DEV_STAT_HALT;
        break;

    case USB_EP_CTRLOUT:
        UsbEp0CtrlStat &= ~USB_CS_CTRLOUT_STALL;
        usbDevSetting.epStat[USB_EP_CTRLOUT] &= ~USB_DEV_STAT_HALT;
        break;

    case USB_EP_BULKIN:
        UsbEp1CtrlStat &= ~USB_CS_BULKIN_STALL;
        usbDevSetting.epStat[USB_EP_BULKIN] &= ~USB_DEV_STAT_HALT;
        break;

    case USB_EP_BULKOUT:
        UsbEp2CtrlStat &= ~USB_CS_BULKOUT_STALL;
        usbDevSetting.epStat[USB_EP_BULKOUT] &= ~USB_DEV_STAT_HALT;
        break;

    case USB_EP_INTRIN:
        UsbEp3CtrlStat &= ~USB_CS_INTRIN_STALL;
        usbDevSetting.epStat[USB_EP_INTRIN] &= ~USB_DEV_STAT_HALT;
        break;

        /*
        case USB_EP_ISOIN:
        	break;
        */

    default:
        /* Error endpoint number */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Remote wakeup.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbRemoteWakeup(void)
{
    UsbCtrl |= USB_C_RESUME;

    return 1;
}

/*

Routine Description:

	Get interrupt status.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
u32 usbGetIntStat(void)
{
    return (UsbIntStat);
}

/*

Routine Description:

	Get device request.

Arguments:

	pReq - Device request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbGetDevReq(u32* pReq)
{
    *pReq++ = UsbEp0RxData;
    *pReq   = UsbEp0RxData;

    return 1;
}

/*

Routine Description:

	Get endpoint data read.

Arguments:

	epNum - Endpoint number.
	pData - Location to put data read.
	pSize - Size of data read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbGetEpReadData(u8 epNum, u8* pData, u32* pSize)
{
    switch (epNum)
    {
    case USB_EP_CTRLOUT:
        *pSize = (UsbEp0DataLen & USB_CS_CTRLOUT_DATA_LEN_MASK) >> USB_CS_CTRLOUT_DATA_LEN_SHFT; /* get read size */
        usbSetReadData((u32)&(UsbEp0RxData), pData, *pSize); /* get read data */
        break;

    case USB_EP_BULKOUT:

        if (UsbEp2DataLen == 0x1F)
            UsbEp2CtrlStat |= USB_CS_BULKOUT_DMA_ENA;
        else
            UsbEp2CtrlStat |= USB_CS_BULKOUT_DMA_ENA | USB_CS_BULKOUT_ACK; /* trigger bulk out dma */

        usbCheckDmaReadReady(); /* check read dma ready */
        *pSize = usbPrevBulkOutDataLen;
        break;

    default:
        /* Error endpoint number */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint read data.

Arguments:

	epNum - Endpoint number.
	pData - Data pointer to read from.
	size - Data size to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpReadData(u8 epNum, u8* pData, u32 size)
{
    switch (epNum)
    {
    case USB_EP_CTRLOUT:
        //UsbEp0DataLen &= ~USB_CS_CTRLOUT_DATA_LEN_MASK;
        //UsbEp0DataLen |= size << USB_CS_CTRLOUT_DATA_LEN_SHFT;
        UsbEp0CtrlStat |= USB_CS_CTRLOUT_ACK; /* ack control out packet */
        break;

    case USB_EP_BULKOUT:
        usbPrevBulkOutDataLen = size;
        UsbEp2TotalDataLen = size;
        usbSetReadDataDma((u32)&(UsbEp2RxData), pData, size); /* set read dma */
        UsbEp2CtrlStat |= USB_CS_BULKOUT_ACK; /* ack bulk out packet */
        break;

    default:
        /* Error endpoint number */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Endpoint write data.

Arguments:

	epNum - Endpoint number.
	pData - Data pointer to write to.
	size - Data size to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpWriteData(u8 epNum, u8* pData, u32 size)
{
    switch (epNum)
    {
    case USB_EP_CTRLIN:
        //UsbEp0DataLen &= ~USB_CS_CTRLIN_DATA_LEN_MASK;
        UsbEp0DataLen = size << USB_CS_CTRLIN_DATA_LEN_SHFT; /* set write size */
        usbSetWriteData((u32)&(UsbEp0TxData), pData, size); /* set write data */
        UsbEp0CtrlStat |= USB_CS_CTRLIN_DATA_VALID; /* ack control in */
        break;

    case USB_EP_BULKIN:
        UsbEp1DataLen = size; /* set write size */
        usbSetWriteDataDma((u32)&(UsbEp1TxData), pData, size); /* set write dma */
        UsbEp1CtrlStat |= USB_CS_BULKIN_DMA_ENA | USB_CS_BULKIN_DATA_VALID; /* trigger bulk in dma */
        usbCheckDmaWriteReady(); /* check write dma ready */
        break;

    case USB_EP_INTRIN:
        UsbEp3DataLen = size; /* set write size */
        usbSetWriteData((u32)&(UsbEp3TxData), pData, size); /*set write data */
        UsbEp3CtrlStat |= USB_CS_INTRIN_DATA_VALID; /* ack interrupt in */
        break;

    case USB_EP_ISOIN:
        UsbEp4DataLen = size; /* set write size */
        usbSetWriteDataDma((u32)&(UsbEp4TxData), pData, size); /* set write dma */
        UsbEp4CtrlStat |= USB_CS_ISOIN_DMA_ENA; /* trigger isochronous in dma*/
        usbCheckDmaWriteReady(); /* check write dma ready */
        break;

    default:
        /* Error endpoint number */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Set read data.

Arguments:

	port - The port to reqd from.
	buf - The buffer to read to.
	siz - The size to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetReadData(u32 port, u8* buf, u32 siz)
{
    u32 i;
    u32* src = (u32*) port;
    u32* dst = (u32*) buf;

    for (i = 0; i < siz; i += 4)
    {
        *dst++ = *src;
    }

    return 1;
}

/*

Routine Description:

	Set data write.

Arguments:

	port - The port to write to.
	buf - The buffer to write from.
	siz - The size to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetWriteData(u32 port, u8* buf, u32 siz)
{
    u32 i;
    u32* src = (u32*) buf;
    u32* dst = (u32*) port;

    for (i = 0; i < siz; i += 4)
    {
        *dst = *src++;
    }

    return 1;
}

/*

Routine Description:

	Set read data dma.

Arguments:

	port - The port to reqd from.
	buf - The buffer to read to.
	siz - The size to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetReadDataDma(u32 port, u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)port;
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = (siz + 15) / 16;
    dmaCfg.burst = 1;
    //if (dmaConfig(DMA_REQ_USB_READ, &dmaCfg) == 0)
    //    return 0;
    guiUSBReadDMAId = marsDMAReq(DMA_REQ_USB_READ, &dmaCfg);

    return 1;
}

/*

Routine Description:

	Set data write dma.

Arguments:

	port - The port to write to.
	buf - The buffer to write from.
	siz - The size to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetWriteDataDma(u32 port, u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set write data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)port;
    dmaCfg.cnt = (siz + 15) / 16;
    dmaCfg.burst = 1;

    //if (dmaConfig(DMA_REQ_USB_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiUSBWriteDMAId = marsDMAReq(DMA_REQ_USB_WRITE, &dmaCfg);
    
    return 1;
}

/*

Routine Description:

	Check dma ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbCheckDmaReadReady(void)
{
    u8 err;
    
    err = marsDMACheckReady(guiUSBReadDMAId);
    guiUSBReadDMAId = 0x55;
    return err;
    /*
    OSSemPend(dmaSemChFin[DMA_CH_USB], USB_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: dmaSemChFin[DMA_CH_USB] is %d.\n", err);
#if 0
        DEBUG_USB("UsbEp1DataLen = 0x%8x\n", UsbEp1DataLen);
        DEBUG_USB("UsbEp1CtrlStat = 0x%8x\n", UsbEp1CtrlStat);
        DEBUG_USB("UsbEp1TxData = 0x%8x\n", UsbEp1TxData);
        DEBUG_USB("DmaCh2CycCnt = 0x%8x\n", DmaCh2CycCnt);

        DEBUG_USB("UsbEp2DataLen = 0x%8x\n", UsbEp2DataLen);
        DEBUG_USB("UsbEp2CtrlStat = 0x%8x\n", UsbEp2CtrlStat);
        DEBUG_USB("UsbEp2RxData = 0x%8x\n", UsbEp2RxData);
        DEBUG_USB("DmaCh2CycCnt = 0x%8x\n", DmaCh2CycCnt);

        DEBUG_USB("DmaCh1Cmd = 0x%8x\n", DmaCh1Cmd);
        DEBUG_USB("DmaCh2Cmd = 0x%8x\n", DmaCh2Cmd);
#endif
        return 0;
    }

    switch (dmaChResp[DMA_CH_USB])
    {
    case DMA_CH_FINISH:
        return 1;

    case DMA_CH_ERROR:
        return -1;

    default:
        return -2;
    }*/
}

s32 usbCheckDmaWriteReady(void)
{
    u8 err;
    
    err = marsDMACheckReady(guiUSBWriteDMAId);
    guiUSBWriteDMAId = 0x55;
    return err;
}    

/*

Routine Description:

	Device suspend.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDevSuspend(void)
{
    /* TBD: suspend */
    return 1;
}

/*

Routine Description:

	Device resume.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDevResume(void)
{
    /* TBD: resume */
    return 1;
}

/*

Routine Description:

	Restore setting after device reset.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDevReset(void)
{
    /*CY 1023*/
    /* For mass storage only */
    UsbEp2TotalDataLen = 0x0000001f;
    UsbEp2CtrlStat |= USB_CS_BULKOUT_ACK;

    return 1;
}

/*

Routine Description:

	Release Dma Source.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbReleaseDmaSource(void)
{
	/* release semaphone resource */
	marsDMACloseReleaseSource(guiUSBReadDMAId);


	return 1;
}

