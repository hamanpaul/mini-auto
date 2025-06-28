/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbvc.c

Abstract:

   	USB Video Class routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbvc.h"
#include "usbreg.h"
#include "usbdev.h"
#include "usbapievt.h"
#include "usbintevt.h"
#include "osapi.h"
#include "sysapi.h"

//#include "usb_Device.h"
#include "rfiuapi.h"
#include "rtcapi.h"
#include "uiKey.h"
#include "uiapi.h"
#include "..\..\ui\inc\uiact_project.h"

#if LWIP2_SUPPORT
#include "encrptyapi.h"
#else
#include <../LwIP/netif/ppp/md5.h>
#endif
#include "ispapi.h"
#include <../rfiu/inc/rfiu.h>

//ahet test
#include "fsapi.h"
#include "dcfapi.h"

#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif

//#define USB_DONGLE_SUPPORT 1

#define mENABLE_USB_IRQ     {OS_ENTER_CRITICAL();   IntIrqMask&=~INT_IRQ_MASK_USB;  OS_EXIT_CRITICAL();}
#if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
#define MAX_AV_CH 8
#else
#define MAX_AV_CH 4
#endif

#define INT_IRQ_MASK_USB		0x00002000
#define INT_IRQ_INPUT_USB		0x00002000
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbVcInit(void);

void usbVcTask(void*);
void UpdateH264Header(int ch);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
//#define usbVcDebugPrint 			printf

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK usbVcTaskStack[USB_VC_TASK_STACK_SIZE]; /* Stack of task usbVcTask() */

OS_EVENT* usbVcSem; /*  Interrupt/API events signal usbVcTask() via this semaphore */

//Multi-CH related
s8  USBEnableStreaming[MAX_AV_CH];
u32 USBVideoBufReadIdx[MAX_AV_CH];
u32 USBVideoPresentTime[MAX_AV_CH];
u32 USBAudioBufReadIdx[MAX_AV_CH];
u32 USBAudioPresentTime[MAX_AV_CH];
VIDEO_BUF_MNG *USBVideoBuf[MAX_AV_CH];
IIS_BUF_MNG   *USBAudioBuf[MAX_AV_CH];

OS_EVENT* USBAudioCmpSemEvt[MAX_AV_CH];
OS_EVENT* USBVideoCmpSemEvt[MAX_AV_CH];

#if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
u8 Session_Ready[MAX_AV_CH]={0,0,0,0,0,0,0,0};
#else
u8 Session_Ready[MAX_AV_CH]={0,0,0,0};
int updateNALheader[MAX_AV_CH]={0,0,0,0};
#endif

int USB_timeout=0;
int USB_COM_DATA=0;
#define H264_header_size 24
static unsigned char H264_config[H264_header_size] =
{
    0x00, 0x00, 0x00, 0x01, 0x67,
    0x42, 0x00, 0x1E, 0xDA, 0x01,
    0x40, 0x16, 0xE4, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x68,
    0xCE, 0x38, 0x80,
};

//aher test
int uuu=0;
unsigned int cks=0;
unsigned long USB_total_size=0;
int gUSB_UPGRADE=0;
unsigned int reboot_cnt=0;
u8 f_com = 0;
u16 UsbHostEnableCam;           // as mac_id
u32 Audio_Total=0;

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM];
extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern IIS_BUF_MNG rfiuRxIIsSounBufMng[MAX_RFIU_UNIT][IIS_BUF_NUM];
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];
extern u8 *rfiuAudioRetDMANextBuf[RFI_AUDIO_RET_BUF_NUM];
extern u32 rfiuAudioRetRec_idx;


extern u8 usb_str_desc0[];
extern u8 usb_str_desc1[];
extern u8 usb_str_desc2[];
extern u8 usb_str_desc3[];

extern u8 iconflag[UI_MENU_SETIDX_LAST];
extern u8  GMotionTrigger[MULTI_CHANNEL_MAX];
extern u8  uiVersionTime[9]; /*Firmware version for MARS internal using.*/

#if USB_DONGLE_SUPPORT
extern u8* usbfwupgrade_buf;
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
void Init_USB_Session();
/*

Routine Description:

	Initialize USB  Class.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
#if ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3))
u32 SPI_Video_time[8]={0,0,0,0,0,0,0,0};
u32 SPI_Audio_time[8]={0,0,0,0,0,0,0,0};
u8 PIR_trigger_status=0;
u8 PIR_trigger_status_cnt[8]={0,0,0,0,0,0,0,0};
CAMERA_STATUS CamStatus[8];

s32 usbVcInit(void)
{
  u8 Result;
    /* Create the semaphore */
    usbVcSem = OSSemCreate(0);
	Init_USB_Session();
    /* Create the task */
    DEBUG_USB("Trace: USB Vendor Class task creating.\n");
    Result = OSTaskCreate(USB_VC_TASK, USB_VC_TASK_PARAMETER, USB_VC_TASK_STACK, USB_VC_TASK_PRIORITY);

    if(Result != OS_NO_ERR)
    {
        DEBUG_UART("usbVcInit %d error!!!\n", Result);
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
typedef struct _USB_Bulk_cmd
{
	u32 rw_WORD_count;//bit31=1 read;bit31=0 write;	bit 30 addr inc enable, bit0-3 (BYTE count/4)-1
	u32 par[15];
}USB_Bulk_cmd;

//FP call_function;
OS_FLAG_GRP *OSUSB_task_flag;
/*
bit 0: usb int
bit 1: USBD_MDMA_CMPLT
bit 2: USBD_MF0_OUT_INT | USBD_MF0_SPK_INT
bit 3: USBD_MF1_OUT_INT | USBD_MF1_SPK_INT
bit 4: mpeg4 frame in
bit 5: audio in
*/
u8  PcamPreviewStart;
u16 UsbAltSetting,UsbTest_mode;
u16 UsbOut;
u16 PcamCurRegPos;
u8  PcamCmd[64];
//u8  remapRFID2Stream[8] = {0,2,4,6,  1,3,5,7};
u8  remapRFID2Stream[8] = {0,1,2,3,4,5,6,7};
u8 Rx_MultiStream;
u8  UsbTxStatus;
//u8  UsbImageMode;
u8  UsbHostPirTime;
void usbVC_init()
{
	u32 i,val;
    u8* pos;

 	//init clock
    DEBUG_USB("\n usbVC_init \n");
	SYS_CTL0 |=(1<<16)|(1<<19);

	SYS_RSTCTL|=(1<<16);
	for (i=0;i<10;i++);
	SYS_RSTCTL&=(~(1<<16));

    SYS_ANA_TEST1|= 1 | (1<<7);

    PcamPreviewStart = 0;
    UsbAltSetting = 0;
    UsbHostEnableCam=0x00;
    memset( PcamCmd, 0, 64 );

    //UsbImageMode = 0;
    usb_device_buf;
    DEBUG_USB("\n**** usb_device_buf=>%x \n",usb_device_buf);
#if 1
    pos = usb_device_buf;
    val = 0;
    for(i=0; i<256; i++)
    {
        *pos = val;
        pos++;
        val++;
    }
#endif

    OSUSB_task_flag = OSFlagCreate(0,0);

	//reset
	DevicePHYTestMode=1;

	DeviceMaskInterrupt=0x0;
	DeviceMaskInterruptG0=~(USBD_MCX_SETUP_INT);
	DeviceMaskInterruptG1=~(USBD_MF0_OUT_INT | USBD_MF0_SPK_INT | USBD_MF1_OUT_INT | USBD_MF1_SPK_INT ); //ML
    DeviceMaskInterruptG2=~(USBD_MUSBRST_INT | USBD_MDMA_CMPLT);

    // EP1: ISO   IN
    // EP3: BULK  IN    (cmd resp)
    // EP4: BULK OUT    (cmd)
	DeviceInEP1MaxPktSize=0;        // iso  IN
    DeviceOutEP1MaxPktSize=0;       // iso  IN
	DeviceInEP2MaxPktSize=64;       // x
	DeviceInEP3MaxPktSize=64;       // bulk IN  reg
	DeviceOutEP4MaxPktSize=512;     // bulk OUT reg
    DeviceInEP5MaxPktSize=0;        // x
    DeviceInEP6MaxPktSize=0;        // x
    DeviceOutEP7MaxPktSize=512;     // bulk OUT

    // EP1: FIFO 2~3
    // EP3: FIFO 1
    // EP4: FIFO 0
    DeviceEP1to4Map=0x00110022;
    DeviceEP5to8Map=0x11000000;

    // FIFO map
    // FIFO 0: BULK 3
    // FIFO 1: BULK 4
    // FIFO 2: ISO
    // FIFO 3: ISO
    DeviceFIFOMap=0x27111304;
    DeviceFIFOConfig=((USBD_PIPE_TYP_BULK<< 0)|(0<< 2)|(0<< 4)|(1<< 5))|
					 ((USBD_PIPE_TYP_BULK<< 8)|(0<<10)|(0<<12)|(1<<13))|
					 ((USBD_PIPE_TYP_ISO <<16)|(1<<18)|(1<<20)|(1<<21))|
					 ((USBD_PIPE_TYP_BULK<<24)|(1<<26)|(1<<28)|(1<<29));


	DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO3ByteCnt=(1<<12);     // reset fifo
  #if 0	/*1:Force USB1.1 , 0:USB 2.0*/
	DeviceMainCtl=(1<<5)|(1<<9)(1<<2);     // bit9: force full speed1.1
  #else
	DeviceMainCtl=(1<<5)|(0<<9)|(1<<2);     // usb High speed
  #endif
	DeviceAddress=0;
	//soft plug
	DevicePHYTestMode=0;
    IntIrqMask&=(~INT_IRQ_MASK_USB);       // enable USB interrupt

    GLOBALInterruptMask = 0x6;              // disable OTG, Host control interrupt

}

void Usb_Host_Reset_Cmd()
{
    DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO3ByteCnt=(1<<12);     // reset fifo
    DeviceAddress=0;
    DeviceInterruptSourceG2=1;

    DEBUG_USB("\n H2D reset.");
}

u32 FIFO0_H2D_64Byte()//64
{
    u32 len;
    u32 flag;
    u32 cnt;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    if(DeviceFIFO0ByteCnt == 0)
        return ;

    //DEBUG_USB("DeviceFIFO0ByteCnt=%d\n",DeviceFIFO0ByteCnt);
    len=DeviceFIFO0ByteCnt;
    DeviceDMATargetFIFONum=(1<<0);
    DeviceDMACtlParam1=((u32)len<<8)|(0<<1);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    //DEBUG_USB( "\nH2D len: 0x%08X\n", len);
    mENABLE_USB_IRQ;
    DeviceDMACtlParam1=((u32)len<<8)|(0<<1)|(1<<0);     // start dma

    cnt=0;
    while(1)
    {
        flag = OSFlagPend(OSUSB_task_flag, 0xff, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME,OS_TICKS_PER_SEC*50, 0 );
        mENABLE_USB_IRQ;

        if(flag==0)
        {
            DEBUG_USB("timeout 4!");
            return 0;
        }
        if(flag&0x02)
        {
            /*
            while( DeviceFIFO0ByteCnt != 0 )
            {
                OSTimeDly(4>>2);
                cnt++;
                if(cnt > 200*4 )     // wait 200 ms
                {
                    DeviceDMACtlParam1 = (1<<3);    // dma abort
                    DeviceFIFO0ByteCnt=(1<<12);
                    DEBUG_USB("timeout 16-3!");
                    return;
                }
            }*/
            //DEBUG_USB("flag %d\n",flag);
            break;
        }
    }
    return len;
    //DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
}

void FIFO1_D2H_64Byte()
{
    u32 len;
    u32 flag;
    u32 cnt;

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    len=64;

    DeviceDMATargetFIFONum=(1<<1); //fifo1
    DeviceDMACtlParam1=(len<<8)|(1<<1);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    mENABLE_USB_IRQ;
    DeviceDMACtlParam1=(len<<8)|(1<<1)|(1<<0);
    cnt=0;
    DEBUG_USB("\n#D2H 16 Bulk S\n");
    while(1)
    {
        flag = OSFlagPend(OSUSB_task_flag, 0x03, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, 10, 0 );
        mENABLE_USB_IRQ;

        if(flag==0)
        {
            DeviceDMACtlParam1 = (1<<3)|(1<<4);    // dma abort
            DeviceFIFO1ByteCnt=(1<<12);
            DEBUG_USB("timeout 16!");
            break;
        }
        if(flag&0x02)
        {
            /*while( DeviceFIFO0ByteCnt != 0 )
            {
                OSTimeDly(OS_TICKS_PER_MS>>2);
                cnt++;

                if(cnt > 30*4 )     // wait 30 ms
                {
                    DeviceDMACtlParam1 = (1<<3)|(1<<4);    // dma abort
                    DeviceFIFO0ByteCnt=(1<<12);
                    DeviceFIFO1ByteCnt=(1<<12);
                    DEBUG_USB("timeout 16-2!");
                    break;
                }
            }*/
            break;
        }
    }
    DEBUG_USB("#D2H 16 end\n");
}

void Init_USB_Session(void)
{
    int i;

    for(i=0; i<MAX_AV_CH; i++)
    {
        USBVideoCmpSemEvt[i]   = OSSemCreate(0);
        USBAudioCmpSemEvt[i]   = OSSemCreate(0);
    }
}

u8 Usb_GET_Streaming_Status(int ch)
{
    return USBEnableStreaming[ch];
}

/*Initial buffer for RF video streaming.*/
void Start_USB_Session(int ch)
{

	/**************************************
    **** Streaming Audio/Video Payload ****
    **************************************/
    unsigned int    cpu_sr = 0;
    u8              level;
    int             i;

    ch = ch % MAX_AV_CH;

    if(USBEnableStreaming[ch] == 0)
    {
        DEBUG_USB("CH %d, start streaming Audio/Video Payload\n",ch);
        if( (rfiuRX_CamOnOff_Sta >> ch) & 0x01)
        {
            OS_ENTER_CRITICAL();
            USBVideoBufReadIdx[ch]  = (rfiuRxVideoBufMngWriteIdx[ch]) % VIDEO_BUF_NUM;
            USBAudioBufReadIdx[ch]  = (rfiuRxIIsSounBufMngWriteIdx[ch]) % IIS_BUF_NUM;
            USBVideoPresentTime[ch] = 100;
            USBAudioPresentTime[ch] = 100;
            USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
            USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
            SPI_Video_time[ch] = 0;
            SPI_Audio_time[ch] = 0;
            USBEnableStreaming[ch]  = 1;
            Session_Ready[ch] = 1;
            OS_EXIT_CRITICAL();
            if(gRfiu_Op_Sta[ch] == RFIU_RX_STA_LINK_BROKEN)
            {
                DEBUG_USB("===== %d-CH is Out of Range =====\n",ch);
				DEBUG_USB("Out of Range\n");
            }
         }
         else
         {
                DEBUG_USB("-->RF CH-%d is OFF, Client-%d\n",ch,USBEnableStreaming[ch]);
                OS_ENTER_CRITICAL();
                USBVideoBufReadIdx[ch]  = rfiuRxVideoBufMngWriteIdx[ch];
                USBAudioBufReadIdx[ch]  = rfiuRxIIsSounBufMngWriteIdx[ch];
                USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
                USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
                USBEnableStreaming[ch]  = 1;
                OS_EXIT_CRITICAL();
          }
    }
    else
    {
        if( (rfiuRX_CamOnOff_Sta >> ch) & 0x01)
        {

        }
        else
        {
            DEBUG_USB("-->RF CH-%d is OFF, Client-%d\n",ch,USBEnableStreaming[ch]);
            OS_ENTER_CRITICAL();
            USBVideoBufReadIdx[ch]  = rfiuRxVideoBufMngWriteIdx[ch];
            USBAudioBufReadIdx[ch]  = rfiuRxIIsSounBufMngWriteIdx[ch];
            USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
            USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
            USBEnableStreaming[ch]  = 1;
            OS_EXIT_CRITICAL();
        }
    }
}

void Stop_USB_Session(int ch)
{
    u8 err;

    ch = ch % MAX_AV_CH;

    USBEnableStreaming[ch]  = 0;
    if(USBEnableStreaming[ch] <= 0)
    {
        DEBUG_USB("CH %d, stop streaming Audio/Video Payload\n",ch);
        OSSemSet(USBVideoCmpSemEvt[ch], 0, &err);
        OSSemSet(USBAudioCmpSemEvt[ch], 0, &err);
		USBEnableStreaming[ch]=0;
        Session_Ready[ch] = 0;
    }
}

void Mars_Pcam_Cmd()
{
    u8 ch;
    int i;
    u8* codeAddr = usbfwupgrade_buf;
 	RTC_DATE_TIME set_time;
	MD5_CTX ctx;
	char buf2[50];
	char buf3[2];
 	char MD5_buf[33];
    unsigned char digest[16];

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    DEBUG_USB( "Cmd: 0x%02X  0x%02X  0x%02X  0x%02X  0x%02X\n ", PcamCmd[0], PcamCmd[1], PcamCmd[2],PcamCmd[3],PcamCmd[4]);
    switch( PcamCmd[0] )
    {
        case PCAM_CMD_START_PREVIEW:
            ch=PcamCmd[1];
            for(i=0;i<MAX_AV_CH;i++)
            {
                if( ch & (1<<i) )
                {
                    gRfiuUnitCntl[i].WakeUpTxEn=1;
                    UsbHostEnableCam |= (1<<(i+1));
                    Start_USB_Session(i);
                    break;
                }
            }
            if(UsbHostEnableCam)
                PcamPreviewStart = 1;
            DEBUG_USB("PCAM_CMD_START_PREVIEW\n");
            break;
        case PCAM_CMD_STOP_PREVIEW:
            ch=PcamCmd[1];
            for(i=0;i<MAX_AV_CH;i++)
            {
                if( ch & (1<<i) )
                {
                    OS_ENTER_CRITICAL();
                    UsbHostEnableCam &= ~(1<<(i+1));
                    OS_EXIT_CRITICAL();

                    Stop_USB_Session(i);
                    if(gRfiuUnitCntl[i].WakeUpTxEn == 1)
                        gRfiuUnitCntl[i].WakeUpTxEn = 0;
                    else
                        rfiuCamSleepCmd(i);
                }
            }
            if(UsbHostEnableCam == 0)
                PcamPreviewStart = 0;

            DEBUG_USB("PCAM_CMD_STOP_PREVIEW %d\n",PcamPreviewStart);
            break;
        case PCAM_CMD_SET_RESOLUTION:
        {
            switch( PcamCmd[1] )
            {
                case 1:     //HD
                    for(ch=0;ch<MAX_AV_CH;ch++)
                    {
                        if(iconflag[UI_MENU_SETIDX_CH1_RES+ch] != UI_MENU_SETTING_RESOLUTION_HD)   //HD
                        {
                            UiExtKey = ch;
                            iconflag[UI_MENU_SETIDX_CH1_RES+UiExtKey] = UI_MENU_SETTING_RESOLUTION_HD;
                        }
                    }
                    uiSentKeyToUi(UI_KEY_RF_RESOLUTION);
                    break;
                case 2:     //FHD
                    for(ch=0;ch<MAX_AV_CH;ch++)
                    {
                        if(iconflag[UI_MENU_SETIDX_CH1_RES+ch] != UI_MENU_SETTING_RESOLUTION_1920x1072)   //FHD
                        {
                            UiExtKey = ch;
                            iconflag[UI_MENU_SETIDX_CH1_RES+UiExtKey] = UI_MENU_SETTING_RESOLUTION_1920x1072;
                        }
                    }
                    uiSentKeyToUi(UI_KEY_RF_RESOLUTION);
                    break;
                case 3:     //4M
                    for(ch=0;ch<MAX_AV_CH;ch++)
                    {
                        if(iconflag[UI_MENU_SETIDX_CH1_RES+ch] != UI_MENU_SETTING_RESOLUTION_4M)   //FHD
                        {
                            UiExtKey = ch;
                            iconflag[UI_MENU_SETIDX_CH1_RES+UiExtKey] = UI_MENU_SETTING_RESOLUTION_4M;
                        }
                    }
                    uiSentKeyToUi(UI_KEY_RF_RESOLUTION);
                    break;
                default:
                    break;

            }
            DEBUG_USB("PCAM_CMD_SET_RESOLUTION %d\n", PcamCmd[1]);
            break;
        }
        case PCAM_CMD_SET_PIR_TIME:
        {
            UsbHostPirTime = PcamCmd[1];

            if( UsbHostPirTime >= RF_BATCAM_LIVE_MAXTIME )  //MAX
                UsbHostPirTime = RF_BATCAM_LIVE_MAXTIME;
            if( UsbHostPirTime <= 10 )  //MIN
                UsbHostPirTime = 10;

            rfiuBatCam_PIRRecDurationTime = UsbHostPirTime;

            DEBUG_USB("PCAM_CMD_SET_PIR_TIME %d\n", PcamCmd[1]);
            break;
        }
    	case PCAM_CMD_ENABLE_SMALL_STREAM:
        {
            Rx_MultiStream=1;
            DEBUG_USB("PCAM_CMD_ENABLE_SMALL_STREAM\n");

            break;
        }
        case PCAM_CMD_DISABLE_SMALL_STREAM:
        {
            Rx_MultiStream=0;
            DEBUG_USB("PCAM_CMD_DISABLE_SMALL_STREAM\n");

            break;
        }
        case PCAM_CMD_SET_MOTION:
        {
            for(i=0;i<MAX_AV_CH;i++)
            {
                if(((u32)PcamCmd[i+1]&0x03) != iconflag[UI_MENU_SETIDX_CH1_MOTION_SENSITIVITY+i])
                {
                    UiExtKey = i;
                    iconflag[UI_MENU_SETIDX_CH1_MOTION_SENSITIVITY+UiExtKey] = ((u32)PcamCmd[i+1]&0x03);
                    uiSentKeyToUi(UI_KEY_RF_MOTION_1);
                }
            }
            DEBUG_USB("PCAM_CMD_SET_MOTION %d %d %d %d\n",PcamCmd[1],PcamCmd[2],PcamCmd[3],PcamCmd[4]);
            break;
        }
        case PCAM_CMD_PAIR:
        {
            ch=0xff;
            for(i=0;i<MAX_AV_CH;i++)
            {
                if( PcamCmd[1] & (1<<i) )
                {
                    ch=i;
                    break;
                }
            }
            if( ch < MAX_AV_CH )
            {
                UiExtKey=ch;
                uiSentKeyToUi(UI_KEY_RF_PAIR_1);
                DEBUG_USB("Pair Parameter:%x Mac:%d\n", PcamCmd[1], UI_KEY_RF_PAIR_1+UiExtKey);
            }
            else
                DEBUG_USB("Pair Parameter error:%x\n", PcamCmd[1]);

            break;
        }
        case PCAM_CMD_SET_CAM_ENABLE:
        {
            ch=PcamCmd[1];
            for(i=0;i<MAX_AV_CH;i++)
            {
                if( ch & (1<<i) )
                {
                    if(iconflag[UI_MENU_SETIDX_CH1_ON+i] != UI_MENU_SETTING_CAMERA_ON)
                    {
                        UiExtKey = i;
                        iconflag[UI_MENU_SETIDX_CH1_ON+UiExtKey] = UI_MENU_SETTING_CAMERA_ON;
                        uiSentKeyToUi (UI_KEY_RF_CAM_ON_1);
                    }
                }
                else
                {
                    if(iconflag[UI_MENU_SETIDX_CH1_ON+i] != UI_MENU_SETTING_CAMERA_OFF)
                    {
                        UiExtKey = i;
                        iconflag[UI_MENU_SETIDX_CH1_ON+UiExtKey] = UI_MENU_SETTING_CAMERA_OFF;
                        uiSentKeyToUi (UI_KEY_RF_CAM_ON_1);
                    }
                }
            }
            DEBUG_USB("PCAM_CMD_SET_CAM_ENABLE:%d\n", PcamCmd[1]);
            break;
        }
        case PCAM_CMD_SET_TIME:
        {
    		DEBUG_USB("RECV time para\n");
            OS_ENTER_CRITICAL();
    		set_time.year   =PcamCmd[1];
    		set_time.month  =PcamCmd[2];
    		set_time.day    =PcamCmd[3];
    		set_time.hour   =PcamCmd[4];
    		set_time.min    =PcamCmd[5];
    		set_time.sec    =PcamCmd[6];
            OS_EXIT_CRITICAL();
    		DEBUG_USB("y=%d m=%d d=%d, h=%d m=%d s=%d\n",set_time.year,set_time.month,set_time.day,set_time.hour,set_time.min,set_time.sec);

        	RTC_Set_GMT_Time(&set_time);
            UiExtKey=0;
            uiSentKeyToUi (UI_KEY_RF_TIME_1);

    		DEBUG_USB("PCAM_CMD_SET_TIME\n");
            break;
        }
        case PCAM_CMD_SET_AUDIO:
        {
            u32 H2D_Pos;
            u32 tmp, get_byte;
            u32 total;
            u32 i=0;

            ch=PcamCmd[1];
            total = PcamCmd[2] | (PcamCmd[3]<<8) | (PcamCmd[4]<<16);
            //DEBUG_USB("PCAM_CMD_SET_AUDIO %d %d\n" ,ch,total);
            //total = 16*1024;
            //memset(usb_TwowayAudio_buf, 0, total);
            rfiu_AudioRetONOFF_APP(1,ch-1);
            H2D_Pos=0;
            do
            {
                tmp = DeviceInterruptSourceG1&(USBD_MF0_SPK_INT|USBD_MF0_OUT_INT);
                if(tmp)
                {
                    get_byte=FIFO0_H2D_64Byte();
    			    //memcpy(usb_TwowayAudio_buf+i,usb_device_buf,get_byte);
    			    memcpy(rfiuAudioRetDMANextBuf[0]+Audio_Total,usb_device_buf,get_byte);
//                    if(H2D_Pos == 0)
//                        DEBUG_YELLOW("A[%d][%d] %x %x %x %x\n",rfiuAudioRetRec_idx,Audio_Total,*(rfiuAudioRetDMANextBuf[0]+Audio_Total),*(rfiuAudioRetDMANextBuf[0]+Audio_Total+1),*(rfiuAudioRetDMANextBuf[0]+Audio_Total+2),*(rfiuAudioRetDMANextBuf[0]+Audio_Total+3));
                    Audio_Total += get_byte;
                    i+=get_byte;
                    H2D_Pos+=get_byte;
					if (Audio_Total<0)
						DEBUG_USB("Receiving audio fail. totalsize=%d\n",Audio_Total);
                    if(Audio_Total%1024)
                    {
						if(Audio_Total>=(1024*(rfiuAudioRetRec_idx+1)))
						{
							rfiuAudioRetRec_idx=(rfiuAudioRetRec_idx+1)%RFI_AUDIO_RET_BUF_NUM;
                            if(rfiuAudioRetRec_idx == 0)
                            {
                               Audio_Total=0;
                            }
						}
						if((rfiuAudioRetRec_idx & 0x07)==0)
        				{
        				#if PWIFI_SUPPORT

                        #else
					        gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktCount=64;
				    		gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].RetryCount=0;
				        	gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].WriteDiv=8;
					        gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].ReadDiv =0;
					        gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktMap0 =0xffffffff;
					        gRfiuUnitCntl[0].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktMap1 =0xffffffff;
                        #endif    
		    	    	}
                    }
                }
                if( H2D_Pos >= total )
                {
                    break;
                }
            }while(i <= total);        // 300ms

//            if( H2D_Pos != total )
//                DEBUG_USB(" H2D_Pos:%d\n", H2D_Pos );
//            else
//                DEBUG_USB("--DONE.\n");

            break;
        }
        case PCAM_CMD_DISABLE_2WAY_AUDIO:
        {
            ch=PcamCmd[1];
            rfiu_AudioRetONOFF_APP(0,ch-1);

            DEBUG_USB("PCAM_CMD_DISABLE_2WAY_AUDIO %d\n",PcamCmd[1]);
            break;
        }
        case PCAM_CMD_DL_FW_BIN:
        {
            u32 H2D_Pos;
            u32 tmp, get_byte;
            u32 total;
            u32 i=0;
            total = PcamCmd[1] | (PcamCmd[2]<<8) | (PcamCmd[3]<<16);

            DEBUG_USB("Total:%d\n", total );

            PcamCmd[0] = PCAM_CMD_NONE;
            memset(codeAddr, 0, 1024*1024*1);
            H2D_Pos=0;
            do
            {
                tmp = DeviceInterruptSourceG1&(USBD_MF0_SPK_INT|USBD_MF0_OUT_INT);
                if(tmp)
                {
                    get_byte=FIFO0_H2D_64Byte();
    			    memcpy(codeAddr+i,usb_device_buf,get_byte);
                    i += get_byte;
                    H2D_Pos+=get_byte;
                }
                if( H2D_Pos >= total )
                {
                    break;
                }
            }while(i <= 1024*1024);        // 300ms

            if( H2D_Pos != total )
                DEBUG_USB( " H2D_Pos:%d\n", H2D_Pos );
            else
                DEBUG_USB("--DONE.\n");

            break;
        }
    	case PCAM_CMD_FW_UPDATE:
    		/*Recv setting data from host.*/
    		DEBUG_USB("\nFW upgrading.\n");
    		MD5Init(&ctx);
    		MD5Update(&ctx, (unsigned char*)codeAddr+0x100,1024*1024-0x100);
    		MD5Final(digest,&ctx);
    		for (i = 0; i < 16; i++)
    		{
    			sprintf(buf3,"%02x",digest[i]);
    			buf2[2*i]=buf3[0];
    			buf2[2*i+1]=buf3[1];
    		}
    		buf2[32]='\0';
    		DEBUG_USB("FW MD5_1:%s\n",buf2);

    		for(i = 0; i < 32; i++)
            {
                MD5_buf[i]=(codeAddr+0xa0)[i];
            }
    		DEBUG_USB("FW MD5_2:%s\n",MD5_buf);
    		if(!strncmp(MD5_buf,buf2,32))
    		{
    		if(ispUpdateAllload_Net(1024*1024)==0)
    			DEBUG_USB("Firmware upgrade fail.\n");
    		else
    			DEBUG_USB("Firmware upgrade success.\n");
    		}
    		else
    			DEBUG_USB("MD5SUM check error!\n");
            UsbTxStatus = 0x20;

          break;
    case PCAM_CMD_REBOOT:
    {
        if( PcamCmd[1] == 0xF8 )
        {
            DEBUG_USB("\n================");
            DEBUG_USB("\n==SYSTEM RESET==");
            DEBUG_USB("\n================");
			sysForceWDTtoReboot();
    		OSTimeDly(100);
        }
        else

        break;
    }
        default:
            break;
    }
    PcamCmd[0] = PCAM_CMD_NONE;
}

void Pcam_Reg_Cmd_Handle()
{
    u8  i;
    u8 *ucom;
    RTC_DATE_TIME Get_time;
    FIFO0_H2D_64Byte();

    ucom = usb_device_buf;
    if( (ucom[0] == 0x25) && (ucom[1] == 0x6f) )    // read command
    {
        switch(ucom[2])
        {
            case PCAM_CMD_GET_RESOLUTION:
                if(iconflag[UI_MENU_SETIDX_CH1_RES] == UI_MENU_SETTING_RESOLUTION_HD)
                    *(usb_device_buf+0) = 0x01; //HD
                else if(iconflag[UI_MENU_SETIDX_CH1_RES] == UI_MENU_SETTING_RESOLUTION_1920x1072)
                    *(usb_device_buf+0) = 0x02; //FHD

                DEBUG_USB("PCAM_CMD_GET_RESOLUTION %d\n",iconflag[UI_MENU_SETIDX_CH1_RES]);
                break;
            case PCAM_CMD_GET_PIR_TIME:
                *(usb_device_buf+0) = 20;   //min

                DEBUG_USB("PCAM_CMD_GET_PIR_TIME %d\n",iconflag[UI_MENU_SETIDX_CH1_RES]);
                break;
            case PCAM_CMD_GET_MOTION:
            {
                u32 HostMD;

                HostMD = 0;
                for(i=0;i<MAX_AV_CH;i++)
                    HostMD |= (iconflag[UI_MENU_SETIDX_CH1_MOTION_SENSITIVITY+i])<<(remapRFID2Stream[i]*4);

                for(i=0;i<MAX_AV_CH;i++)
                    *(usb_device_buf+i) = (HostMD>>i*4)&0x03;

                DEBUG_USB("PCAM_CMD_GET_MOTION:%x\n", HostMD);
                break;
            }
        	case PCAM_CMD_GET_VERSION:
            {
        		memcpy(usb_device_buf,uiVersion,32);
        		DEBUG_USB("PCAM_CMD_GET_VERSION\n");
        	    break;
        	}
        	case PCAM_CMD_GET_TXVERSION:
            {
        		memcpy(usb_device_buf,gRfiuUnitCntl[ucom[3]].RFpara.TxCodeVersion,32);
        		DEBUG_USB("PCAM_CMD_GET_TX%d VERSION \n",ucom[3]);
        	    break;
        	}
        	case PCAM_CMD_GET_TIME:
            {
        		RTC_Get_Time(&Get_time);
        		*(usb_device_buf+0)=Get_time.year;
        		*(usb_device_buf+1)=Get_time.month;
        		*(usb_device_buf+2)=Get_time.day;
        		*(usb_device_buf+3)=Get_time.hour;
        		*(usb_device_buf+4)=Get_time.min;
        		*(usb_device_buf+5)=Get_time.sec;

        		DEBUG_USB("PCAM_CMD_GET_TIME\n");
        	    break;
        	}
            case PCAM_CMD_GET_CAM_ENABLE:
            {
                u8 rtn;
                rtn=0;
                for( i=0;i<MAX_AV_CH;i++)
                {
                    if( iconflag[ UI_MENU_SETIDX_CH1_ON+i] )
                        rtn |= (1<<remapRFID2Stream[i]);
                }
                usb_device_buf[0] = rtn;

                DEBUG_USB("PCAM_CMD_GET_CAM_ENABLE:%02x \n", rtn );
                break;
            }
            case PCAM_CMD_FW_UPDATE_STATUS:
            {
                usb_device_buf[0] = UsbTxStatus;

                DEBUG_USB("PCAM_CMD_FW_UPDATE_STATUS:%02x \n", usb_device_buf[0]);
                break;
            }
        }

        FIFO1_D2H_64Byte();
        PcamCmd[0] = PCAM_CMD_NONE;
    }
    else    // write command
    {
        DEBUG_USB("%x %x\n",ucom[0],ucom[1]);
        if( (ucom[0]==0x1f) && (ucom[1]==0xf9) )
        {
            int i;
            for(i=0;i<16;i++)
                PcamCmd[i]=ucom[2+i];
        }

    }

    if(PcamCmd[0] != PCAM_CMD_NONE)
    {
        Mars_Pcam_Cmd();
    }

}

void USB_EPO_output_data(BYTE* data,u16 len)//note len must<=64 BYTES
{
    OS_FLAGS flag;
    u16 rem, xlen,pos;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    rem = len;
    pos=0;

    while( rem )
    {
        if(rem >= 64 )
            xlen = 64;
        else
            xlen = rem;

        DeviceCXConfigFIFOEmpS|=(1<<3); //ML
        DeviceDMATargetFIFONum=(1<<4);	//control bus
        DeviceDMACtlParam1=((u32)xlen<<8)|(1<<1);
        DeviceDMACtlParam2=(u32)data+pos;
        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        mENABLE_USB_IRQ;

        DeviceDMACtlParam1=((u32)xlen<<8)|(1<<1)|(1<<0);

        while(1)
        {
            mENABLE_USB_IRQ;
            flag = OSFlagPend(OSUSB_task_flag, 0xff, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_TICKS_PER_SEC, 0 );

            if(flag==0)
            {
                DEBUG_USB("timeout EP0!");
                break;
            }
            if(flag&0x02)
            {
                //OSTimeDly(4*1);
                break;
            }
        }
        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        pos +=xlen;
        rem -= xlen;
    }
    DeviceDMATargetFIFONum=0;
}

void USB_test_packet()
{
/* for high speed test mode; see USB 2.0 spec 7.1.20 */
__align(4) u8 usb_test_packet[] = {
                                         /* implicit SYNC then DATA0 to start */
                                        /* JKJKJKJK x9 */
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         /* JJKKJJKK x8 */
                                        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                        /* JJJJKKKK x8 */
                                        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                        /* JJJJJJJKKKKKKK x8 */
                                        0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                        /* JJJJJJJK x8 */
                                        0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd,
                                         /* JKKKKKKK x10, JK */
                                         //0xfc, 0x7e, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0x7e, 0x00, 0x00,0x00,0x00
                                         0xfc, 0x7e, 0xbf, 0xdf, 0xfb, 0xfd, 0xfb, 0xfd, 0x7e, 0x00, 0x00, 0x00, 0x00
                                         /* implicit CRC16 then EOP to end */
                                        };
	DevicePHYTestMode =0x00000010;
   // HCPortSC = HCPortSC | 0x00040000;
    USB_EPO_output_data((BYTE*)&usb_test_packet,55);

    DEBUG_USB("Enter set tst_pk done routine...\n");
    DeviceCXConfigFIFOEmpS = DeviceCXConfigFIFOEmpS | 0x02;
}

//u16 IosPktSizeTbl[9] = {0, 128, 256, 384, 512, 680, 800, 900, 1007 };
u16 IosPktSizeTbl[9] = {0, 256, 512, 1000, 512, 680, 800, 900, 1007 };
void Usb_Ctrl_pipe_Handle()
{
    #define USB_CONTROL_PIPE_SET_COMMAND_DONE	(DeviceCXConfigFIFOEmpS|=1)

	USB_DEV_REQ USB_Setup_req;
	u16 u16tmp;

    DeviceDMATargetFIFONum=(1<<4);
     ((u32*)&USB_Setup_req)[0]=DeviceDMACtlParam3;
     ((u32*)&USB_Setup_req)[1]=DeviceDMACtlParam3;
    // DEBUG_USB("\n%x,%x",((u32*)&USB_Setup_req)[0],((u32*)&USB_Setup_req)[1]);
    DeviceDMATargetFIFONum=0;
    switch (USB_Setup_req.bRequest)
    {
        case USB_RC_SET_ADDRESS:
            //DEBUG_USB("\nA");
            DEBUG_USB("\nSet address to %d",USB_Setup_req.wValue);
            if(reboot_cnt>30)
            {
                DEBUG_USB("USB dongle initial fail, Rebooting...");
                sysForceWDTtoReboot();
            }
            else
                reboot_cnt++;

            DeviceAddress=(DeviceAddress&0x80)|(USB_Setup_req.wValue&0x7f);
            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
            break;
        case USB_RC_GET_DESCRIPTOR:
            //DEBUG_USB("\nD");
            switch ((USB_Setup_req.wValue>>8)&USB_DEV_REQ_REC_MASK)
            {
                case USB_DESC_TYPE_DEVICE:
                    /* get device descriptor */
                    u16tmp=sizeof(USB_DEV_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_DEV_DESC);
                    //DEBUG_USB("\n%x",u16tmp);
                    USB_EPO_output_data((BYTE*)&usb_isp_dev_desc,u16tmp);
                    break;

                case USB_DESC_TYPE_CONFIGURATION:
                    DEBUG_USB("\nGet Config");
                    //u16tmp=sizeof(USB_CFG_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_CFG_DESC);
                    //USB_EPO_output_data((BYTE*)&usb_msc_configuration_desc.cfg,u16tmp);
                    //DEBUG_USB("\n USB_Setup_req.wLength: 0x%08x", USB_Setup_req.wLength);
                    u16tmp=sizeof(USB_ISP_CONFIGURATION_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_ISP_CONFIGURATION_DESC);
                    USB_EPO_output_data((BYTE*)&usb_isp_configuration_desc,u16tmp);
                    break;

                case USB_DESC_TYPE_STRING:
                    DEBUG_USB("\nGet string");
                    reboot_cnt=0;
                    switch ((USB_Setup_req.wValue)&0xff)
                    {
                        case 0:
                            u16tmp=USB_STR0_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR0_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc0,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 1:
                            u16tmp=USB_STR1_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR1_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc1,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 2:
                            u16tmp=USB_STR2_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR2_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc2,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 3:
                            u16tmp=USB_STR3_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR3_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc3,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;

                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case USB_RC_SET_CONFIGURATION:
            DeviceAddress|=(1<<7);
            //Initial buffer for RF video streaming. aher

            break;
        case USB_RC_SET_INTERFACE:
            UsbAltSetting = USB_Setup_req.wValue;
            DEBUG_USB("USB_Setup_req.wValue %d\n",USB_Setup_req.wValue);
            if( UsbAltSetting > 8 )
            {
                DEBUG_USB("%d\n",UsbAltSetting);
                UsbAltSetting =0;
            }

            DeviceOutEP1MaxPktSize = IosPktSizeTbl[UsbAltSetting];
            DeviceInEP1MaxPktSize  = IosPktSizeTbl[UsbAltSetting];

            DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
            //DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
            DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
            DeviceFIFO3ByteCnt=(1<<12);     // reset fifo

            DEBUG_USB("\n SET_INTERFACE %02x  Size: %i", UsbAltSetting, IosPktSizeTbl[UsbAltSetting]);
            if(reboot_cnt>30)
            {
            	DEBUG_USB("USB dongle initial fail2, Rebooting...");
            	sysForceWDTtoReboot();
            }
            else
            	reboot_cnt++;

            break;
         case USB_RC_SET_FEATURE:
          //   UsbTest_mode = USB_Setup_req.wValue.
            switch (USB_Setup_req.wValue)
            {
                case TEST_SE0_NAK:
                    DevicePHYTestMode =0x00000008;
                    break;
                case TEST_J:
                    DevicePHYTestMode =0x00000002;
                    break;
                case TEST_K:
                    DevicePHYTestMode =0x00000004;
                    break;
                case TEST_PACKET:
                    USB_test_packet();
                    break;
                default:
                    break;
            }
            DEBUG_USB("\n SET_FEATURE %x ", USB_Setup_req.wValue);

            break;

        case USB_RC_GET_INTERFACE:
            DEBUG_USB("\n USB_RC_GET_INTERFACE");
            USB_EPO_output_data((BYTE*)&UsbAltSetting, 1);

            break;
        default:
            break;
    }
    USB_CONTROL_PIPE_SET_COMMAND_DONE;
}

// USB frame header 24 bytes
// u8  sign[6]          [0:5]   signature string 0xFFFF00FF9664
// u8  frameType        [6]     1: I-frame  0: P-frame
// u8  cam              [7]     1~8: 720P stream, 9~16: FHD stream, 17~24: 640x352 stream
// u8  imageMode        [8]     0x01: 1280x720, 0x10: 640x352, 0x02: FHD
// u8  imageLen[3]      [9:11]  image length( big endian )
// u8  interval[2]      [12:13] xxxms( big endian )
// u8  status           [14]    bit0: PIR,  bit1: motion detected, bit5..7: RF signal 0: no signal, 4: strong
// u8  battery          [15]    bit7: DC status 1: DC plug-in,  bit6: Charge status 1: charging  bit2..0: battery levle 0~4
// u8  timestamp[4]     [16..19]
// u8  PirRemain        [20]    PIR remain time, units: sec
// u8  rev[3]           [21..23]

// frameType: 1: I-frame, 0: P-frame
// format   : 1: Audio, 2: Video
u8 D2H_Image(u8 format, u32 imgAddr, u32 imgLen, u8 StreamID, u8 frameType, u8 cam )
{
    static u32 UsbIsoTimeoutErr;
    u32 pos,rem,len,jj,flag,cnt,x;
//	int packet_len=8192;
    u8  UsbHeader[24];
    u8  orgHeader[24];
    u8  pirTrigger;
    u8  TransResult;
    u8  UsbImageMode;
    u8  PirRemain;
    u32 *header;
    u32 temp_u32;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif
    //DEBUG_USB("D2H_image %d\n",UsbAltSetting);


    TransResult=0;
    pirTrigger=0;

    if( UsbAltSetting == 0 )
    {
        //DEBUG_USB("UsbAltSetting %d\n",UsbAltSetting);
        return TransResult;
    }

    DeviceFIFO2ByteCnt=(1<<12);
    DeviceFIFO3ByteCnt=(1<<12);
    if(gRfiuUnitCntl[cam].RFpara.BatCam_PIRMode)
    {
        pirTrigger=1;
    }
    if( GMotionTrigger[cam] )
    {
        pirTrigger|=0x02;
        GMotionTrigger[cam]=0;
    }

    rem = imgLen;
    pos = imgAddr;
    // 720P stream ID = 1 ~  8
    // FHD  stream ID = 9 ~ 16
    // VGA  stream ID = 17 ~24
    // 4M   stream ID = 25 ~32
    memcpy_hw(usb_AV_buf+24,imgAddr,imgLen);
    (void *)imgAddr = usb_AV_buf;

    if(format == 1)
    {
        if( USBAudioBuf[cam][USBAudioBufReadIdx[cam]].size == 2048 )    // PCM
        {
            UsbImageMode=0x01;
            StreamID+=8;                            // FHD mapping to 9~16

        }
        else  // AAC
        {
            UsbImageMode=0x02;
            //StreamID+=8;
        }
        // [0..3]
        header = (u32*)&UsbHeader[0];
        *header = 0xff00ffff;
        header++;
        // [4..7]
        *header = 0x00008096 | (StreamID<<24) | (frameType<<16);    // Audio: 0x80

        header++;
        len = imgLen-24;
        // [8..11]
        *header = UsbImageMode | (((u8)len)<<24) | ((u8)(len>>8)<<16) | ((u8)(len>>16)<<8);

        if( (len&0x00ffffff) > (2*1024) )
        {
            DEBUG_USB("len %d\n",len);
            return TransResult;
        }

        header++;
        // [12..15]
        if(gRfiuUnitCntl[cam].RFpara.TxBatteryLev == RF_BATCAM_TXBATSTAT_NOSHOW)
            CamStatus[cam].battery |= (1<<7);
        else if(gRfiuUnitCntl[cam].RFpara.TxBatteryLev == RF_BATCAM_TXBATSTAT_CHARGE)
            CamStatus[cam].battery |= (1<<6);
        else
            CamStatus[cam].battery = gRfiuUnitCntl[cam].RFpara.TxBatteryLev;
        *header = 112 | (pirTrigger<<16) | (CamStatus[cam].battery<<24);   // 100ms interval
        header++;
        // [16..19]
        SPI_Audio_time[cam] += USBAudioBuf[cam][USBAudioBufReadIdx[cam]].time;
        *header = ((u8)SPI_Audio_time[cam]<<24) | ((u8)(SPI_Audio_time[cam]>>8)<<16) | ((u8)(SPI_Audio_time[cam]>>16)<<8) | ((u8)(SPI_Audio_time[cam]>>24));

        //*header = USBVideoBuf[cam][USBVideoBufReadIdx[cam]].time;
        header++;

        // [20..23]
        *header = PirRemain;
    }
    else
    {
        if( gRfiuUnitCntl[cam].TX_PicWidth == 1280 )        // HD
        {
            UsbImageMode=0x01;                      // 720P mapping to 1~8
        }
        else if( gRfiuUnitCntl[cam].TX_PicWidth == 1920 )   // FHD
        {
            UsbImageMode=0x02;
            StreamID+=8;                            // FHD mapping to 9~16
        }
        else if( gRfiuUnitCntl[cam].TX_PicWidth == 2688 )   // 4M
        {
            UsbImageMode=0x03;
            StreamID+=24;                            // 4M mapping to 25~32
            //DEBUG_USB("imgLen %d\n",imgLen);
        }
        else if( gRfiuUnitCntl[cam].TX_PicWidth == 640 )   // QHD
        {
            UsbImageMode=0x10;                      // 640x352
            StreamID-=8;
            StreamID+=16;                           // VGA stream mapping to 17~24
        }
        else
        {
            DEBUG_USB("gRfiuUnitCntl[%d].TX_PicWidth %d %d\n",cam,gRfiuUnitCntl[cam].TX_PicWidth,gRfiuUnitCntl[cam].TX_PicHeight);
            return TransResult; // unknow image frame???
        }
        // [0..3]
        header = (u32*)&UsbHeader[0];
        *header = 0xff00ffff;
        header++;
        // [4..7]
        *header = 0x00006496 | (StreamID<<24) | (frameType<<16);    // Video: 0x64

        header++;
        len = imgLen-24;
        // [8..11]
        *header = UsbImageMode | (((u8)len)<<24) | ((u8)(len>>8)<<16) | ((u8)(len>>16)<<8);

        if( (len&0x00ffffff) > (300*1024) )
        {
            DEBUG_USB("len %d\n",len);
            return TransResult;
        }

        header++;
        // [12..15]
        if(gRfiuUnitCntl[cam].RFpara.TxBatteryLev == RF_BATCAM_TXBATSTAT_NOSHOW)
            CamStatus[cam].battery |= (1<<7);
        else if(gRfiuUnitCntl[cam].RFpara.TxBatteryLev == RF_BATCAM_TXBATSTAT_CHARGE)
            CamStatus[cam].battery |= (1<<6);
        else
            CamStatus[cam].battery = gRfiuUnitCntl[cam].RFpara.TxBatteryLev;
        *header = 112 | (pirTrigger<<16) | (CamStatus[cam].battery<<24);   // 100ms interval
        header++;
        // [16..19]
        SPI_Video_time[cam] += USBVideoBuf[cam][USBVideoBufReadIdx[cam]].time;
        *header = ((u8)SPI_Video_time[cam]<<24) | ((u8)(SPI_Video_time[cam]>>8)<<16) | ((u8)(SPI_Video_time[cam]>>16)<<8) | ((u8)(SPI_Video_time[cam]>>24));
        header++;

        // [20..23]
        *header = PirRemain;
    }

    memcpy( (void *)imgAddr, UsbHeader, 24 );

    pos = imgAddr;
    rem = imgLen;
    jj=0;
//    if(format == 1)
//        return TransResult;
//    DEBUG_USB("Type %d, Time %d, size %ld\n",frameType,SPI_video_time,USBVideoBuf[cam][USBVideoBufReadIdx[cam]].size);
//    DEBUG_USB("D2H %02x %02x %02x %02x \n",*(USBVideoBuf[cam][USBVideoBufReadIdx[cam]].buffer+3),*(USBVideoBuf[cam][USBVideoBufReadIdx[cam]].buffer+4)
//                                        ,*(USBVideoBuf[cam][USBVideoBufReadIdx[cam]].buffer+5),*(USBVideoBuf[cam][USBVideoBufReadIdx[cam]].buffer+6));

    while(rem)
    {
        if ( rem >= IosPktSizeTbl[UsbAltSetting] )
            len = IosPktSizeTbl[UsbAltSetting];
        else
            len = rem;

        DeviceFIFO2ByteCnt=(1<<12);
        DeviceDMATargetFIFONum=(1<<2);
        DeviceDMACtlParam1=(len<<8)|(1<<1);
        DeviceDMACtlParam2=pos;
        //DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        DeviceDMACtlParam1=(len<<8)|(1<<1)|(1<<0);
        mENABLE_USB_IRQ;
        cnt = 0;
        while(1)
        {
            flag = OSFlagPend(OSUSB_task_flag, 0x03, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, 33, 0 );
            mENABLE_USB_IRQ;
            if(flag==0)
            {
                UsbIsoTimeoutErr++;
                DeviceDMACtlParam1 = (1<<3);    // TIMEOUT dma abort
                DeviceFIFO2ByteCnt=(1<<12);
                DeviceFIFO3ByteCnt=(1<<12);
                //PcamPreviewStart=0;
                DEBUG_USB("\nUSB Iso timeout!");

                TransResult=1;
                if( UsbIsoTimeoutErr >= 5 )
                {
                    /*u32 tick;
                    UsbIsoTimeoutErr=0;

                    DevicePHYTestMode=1;
                    tick=SYSTEM_TICK;

                    while( Time_differ(tick) <= (200*1000) );

                    usb_init();*/
                    sysForceWDTtoReboot();

                    //UsbPirEnableCam=0;
                    //UsbPirEnableCam=0;
                    return TransResult;
                }
                else
                    break;
            }
            if(flag&0x02)
            {
                //OSTimeDly(1);
                //OSTimeDly20us(OS_TICKS_PER_MS>>2);
              //  for(x=0;x<10000;x++);
                cnt = 0;
                temp_u32 =DeviceInterruptSourceG1;
            #if 1
                while( !(temp_u32 & USBD_MF2_IN_INT) )
                //while( DeviceFIFO2ByteCnt == len )
                {
                    temp_u32 =DeviceInterruptSourceG1;

                    //DEBUG_USB( "F2Cnt: 0x%08x\n ", DeviceFIFO2ByteCnt );
                    cnt++;
                    //if(cnt > 500*4 )     // wait 500 ms
                    if(cnt > 8000000 )     // wait 500 ms
                    {
                        DeviceDMACtlParam1=(1<<3);      // dma abort
                        DeviceFIFO2ByteCnt=(1<<12);
                        DeviceFIFO3ByteCnt=(1<<12);
                        //DEBUG_USB( "\n Stop Pcam-2." );
                        DEBUG_USB( "\n USB timeout-2. CAM:%d", cam );

                        TransResult=2;
                        UsbIsoTimeoutErr++;
                        if( UsbIsoTimeoutErr >= 5 )
                        {
                            /*u32 tick;
                            UsbIsoTimeoutErr=0;

                            DevicePHYTestMode=1;
                            tick=SYSTEM_TICK;

                            while( Time_differ(tick) <= (200*1000) );

                            usb_init();

                            return;*/

                            sysForceWDTtoReboot();
                        }
                        break;
                    }

                }
            #endif
                break;
            }
        }
        pos += len;
        rem -= len;
    }
    if( rem == 0 )
        UsbIsoTimeoutErr = 0;

    return TransResult;
}

void usbVcTask(void* pData)
{
    u8 id,i,UsbRetryCnt;
    u8 frameType,DlyFlag;
    u32 SmallstreamSize;
	u16 video_value[MAX_AV_CH]={0,0,0,0,0,0,0,0};
	u16 audio_value[MAX_AV_CH]={0,0,0,0,0,0,0,0};
    u16 ch,err;
	u32 video_buf_offset=0,Size,tmp,len,cnt1=0,sub_stream_size;
	int video_frame_cnt;
    char raytest[]="raymond test file use usb write 123456789";

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    OS_FLAGS USB_event;

    usbVC_init();

    while (1)
    {
        mENABLE_USB_IRQ;
        USB_event=OSFlagPend (OSUSB_task_flag, 0xffffffff, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, 1, 0);
        //mENABLE_USB_IRQ;

//        DEBUG_USB("\n.");
        //if( USB_event == 0x00 )
        //    continue;

        if (DeviceInterruptSourceG2&1)
        {
            Usb_Host_Reset_Cmd();
        }
        tmp=DeviceInterruptSourceG0&(USBD_MCX_SETUP_INT);
        if (tmp)
        {
            if (tmp&USBD_MCX_SETUP_INT)
            {
                Usb_Ctrl_pipe_Handle();
            }
        }
        tmp=DeviceInterruptSourceG1&(USBD_MF0_SPK_INT|USBD_MF0_OUT_INT);
        if (tmp)
        {
            Pcam_Reg_Cmd_Handle();
        }
        DlyFlag=1;
        if(PcamPreviewStart || ((USB_event & 0x0020) || (USB_event & 0x0010)))    //AUDIO
        {
            //DEBUG_USB("PcamPreviewStart %d\n",PcamPreviewStart);
            if( UsbAltSetting == 0 )
            {
                DEBUG_USB("UsbAltSetting %d\n",UsbAltSetting);
                continue;
            }
            for(ch=0;ch<MAX_AV_CH;ch++)
            {
                OS_ENTER_CRITICAL();
                //if( ((UsbPirEnableCam & (1<<i))==0) && ((UsbHostEnableCam & (1<<i))==0) )
                if((UsbHostEnableCam & (1<<(ch+1)))==0)
                {
                    OS_EXIT_CRITICAL();
                    continue;
                }
//                if( (Preview_en_flag_val & (1<<i))==0 )
//                {
//                    OS_EXIT_CRITICAL();
//                    continue;
//                }
//                if( UsbUpdate == 0x5A )
//                {
//                    OS_EXIT_CRITICAL();
//                    continue;
//                }
                OS_EXIT_CRITICAL();

//                if(USB_event & 0x0020)
//                    DEBUG_RED("A[%d] %d\n",ch,USBAudioCmpSemEvt[ch]->OSEventCnt);
//                else
//                    DEBUG_RED("V[%d] %d\n",ch,USBVideoCmpSemEvt[ch]->OSEventCnt);

                if(1)//(USB_event & 0x0020)  //audio
                {
                    if((video_value[ch] == 0) || (USBAudioPresentTime[ch] <= USBVideoPresentTime[ch]))
                    {
                        audio_value[ch] = OSSemAccept(USBAudioCmpSemEvt[ch]);
                        if (audio_value[ch] > 0)
                        {
                            if(audio_value[ch] > 10)
                            {
//                                DEBUG_YELLOW("ch[%d]=%d\n", ch, USBVideoCmpSemEvt[ch]->OSEventCnt);
                                DEBUG_RED("A[%d] %d %d %d %d\n",ch,audio_value[ch],rfiuRxIIsSounBufMngWriteIdx[ch],USBAudioBufReadIdx[ch],USBAudioPresentTime[ch]);
                            }
                            if( USBAudioBuf[ch][USBAudioBufReadIdx[ch]].flag)
                                frameType=1;
                            else
                                frameType=0;
                            id = remapRFID2Stream[ch];       // large stream
                            id+=1;

                            UsbRetryCnt=3;
                            do
                            {
                                if( D2H_Image(1,(u32)USBAudioBuf[ch][USBAudioBufReadIdx[ch]].buffer, USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size+24, id,frameType, ch ) == 0)
                                    break;

                            }while(UsbRetryCnt--);

                            USBAudioPresentTime[ch] += (USBAudioBuf[ch][USBAudioBufReadIdx[ch]].time); //if use chunk time
                            USBAudioBufReadIdx[ch] = (USBAudioBufReadIdx[ch] + 1) % IIS_BUF_NUM;
                            //DEBUG_RED("A[%d] %d %d\n",ch,USBAudioBuf[ch][USBAudioBufReadIdx[ch]].time,USBAudioPresentTime[ch]);

                        }
                    }
                }
                if(1)//(USB_event & 0x0010)  //video
                {
                    if((audio_value[ch] == 0) || (USBAudioPresentTime[ch] >= USBVideoPresentTime[ch]))
                    {
                        video_value[ch] = OSSemAccept(USBVideoCmpSemEvt[ch]);
                        if (video_value[ch] > 0)
                        {
                            if(video_value[ch] > 10)
                            {
//                                DEBUG_YELLOW("ch[%d]=%d\n", ch, USBVideoCmpSemEvt[ch]->OSEventCnt);
                                DEBUG_YELLOW("V[%d] %d %d %d %d\n",ch,video_value[ch],rfiuRxVideoBufMngWriteIdx[ch],USBVideoBufReadIdx[ch],USBVideoPresentTime[ch]);
                            }
                            if( USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
                                frameType=1;
                            else
                                frameType=0;
                            id = remapRFID2Stream[ch];       // large stream
                            id+=1;

                            UsbRetryCnt=3;
                            do
                            {
                                if( D2H_Image(2,(u32)USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer, USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset+24, id,frameType, ch ) == 0)
                                    break;

                            }while(UsbRetryCnt--);

                            if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size != USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset)
                                Rx_MultiStream = 1;
                            else
                                Rx_MultiStream = 0;

                            if(0)//(Rx_MultiStream)
                            {
                                if( USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
                                    frameType=1;
                                else
                                    frameType=0;

                                SmallstreamSize=USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size-USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset;
                                id = remapRFID2Stream[ch];       // small stream
                                id+=1+8;
                                //DEBUG_USB("Rx_MultiStream %d %d \n",USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset,SmallstreamSize);
                                UsbRetryCnt=3;
                                do
                                {
                                    if( D2H_Image(2, (u32)USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset, SmallstreamSize+24, id,frameType, ch ) == 0)
                                        break;

                                }while(UsbRetryCnt--);
                            }
                            USBVideoPresentTime[ch] += (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time); //if use chunk time
                            USBVideoBufReadIdx[ch] = (USBVideoBufReadIdx[ch] + 1) % VIDEO_BUF_NUM;
                            //DEBUG_YELLOW("V[%d] %d %d\n",ch,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time,USBVideoPresentTime[ch]);
                        }
                    }
                }
                if ((video_value[ch] > 0)||(audio_value[ch] > 0))
                {
                    //DEBUG_YELLOW("video_value[%d] %d\n",ch,video_value[ch]);
                    DlyFlag=0;
                }
            }
    	}

        if(DlyFlag == 1)
        {
            //DEBUG_BLUE("V[ch] %d A[ch] %d\n",video_value[ch],audio_value[ch]);
            //OSTimeDly(1);
        }
        tmp=DeviceInterruptSourceG2&(USBD_MDMA_CMPLT);
        if( tmp )
        {
            DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
        }
    }
}

void USB_RF_LINK_Broken(u8 ch)
{
    //u8 ch;
    DEBUG_USB("%d USB_RF_LINK_Broken Start\n",ch);

    UsbHostEnableCam &= ~(1<<(ch+1));

    Stop_USB_Session(ch);
    if(UsbHostEnableCam == 0)
        PcamPreviewStart = 0;
}

void USB_RF_LINK_Success(u8 ch)
{
    DEBUG_USB("%d USB_RF_LINK_Success Start\n",ch);

    UsbHostEnableCam |= (1<<(ch+1));
    Start_USB_Session(ch);

    if(UsbHostEnableCam)
        PcamPreviewStart = 1;
}

void usbVCntHandler(void)
{
    u8 err;
    OS_FLAGS flag;
    IntIrqInput&=(~INT_IRQ_INPUT_USB);

    IntIrqMask|=INT_IRQ_MASK_USB;       // disable USB interrupt

    flag=0x01;
    if(DeviceInterruptSourceG2&USBD_MDMA_CMPLT)
    {
        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
        flag |= 0x02;           // USB DMA complete
    }

    if(DeviceInterruptSourceG1& (USBD_MF0_OUT_INT | USBD_MF0_SPK_INT) )
        flag |= 0x04;
    if(DeviceInterruptSourceG1& (USBD_MF1_OUT_INT | USBD_MF1_SPK_INT) )
        flag |= 0x08;

    OSFlagPost(OSUSB_task_flag, flag , OS_FLAG_SET, &err);
}

#else
s32 usbVcInit(void)
{
  u8 Result;
    /* Create the semaphore */
    usbVcSem = OSSemCreate(0);
	Init_USB_Session();
    /* Create the task */
    DEBUG_USB("Trace: USB Vendor Class task creating.\n");
    Result = OSTaskCreate(USB_VC_TASK, USB_VC_TASK_PARAMETER, USB_VC_TASK_STACK, USB_VC_TASK_PRIORITY);

    if(Result != OS_NO_ERR)
    {
        DEBUG_USB("usbVcInit %d error!!!\n", Result);
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
typedef struct _USB_Bulk_cmd
{
	u32 rw_WORD_count;//bit31=1 read;bit31=0 write;	bit 30 addr inc enable, bit0-3 (BYTE count/4)-1
	u32 par[15];
}USB_Bulk_cmd;

//FP call_function;
OS_FLAG_GRP *OSUSB_task_flag;
/*
bit 0: usb int
bit 1: USBD_MDMA_CMPLT
bit 2: USBD_MF0_OUT_INT | USBD_MF0_SPK_INT
bit 3: USBD_MF1_OUT_INT | USBD_MF1_SPK_INT
bit 4: mpeg4 frame in
bit 5: audio in
*/

u16 UsbOut;
u16 PcamCurRegPos;
u8  PcamCmd[5];
u8  PcamPreviewStart;
u16 UsbAltSetting,UsbTest_mode;
u8  UsbImageMode;

void usbVC_init()
{
	u32 i;
 	//init clock

    DEBUG_USB("\n usbVC_init \n");
	SYS_CTL0 |=(1<<16)|(1<<19);
	SYS_RSTCTL|=(1<<16);
	for (i=0;i<10;i++);
	SYS_RSTCTL&=(~(1<<16));
    SYS_ANA_TEST1|= 1 | (1<<7);

    //SYS_ANA_TEST1|= 1 | (1<<7) | (1<<9);

	//TM5_CTLREG=((239ul<<3)&0xff00)|(239ul&0x3f)|0xC0;//10us
	//TM5_PAUSE_INTEN=0;

    PcamPreviewStart = 0;
    UsbAltSetting = 0;
    PcamCmd[0]=0;
    PcamCmd[1]=0;
    PcamCmd[2]=0;
    PcamCmd[3]=0;
    PcamCmd[4]=0;

    UsbImageMode = 0;
    usb_device_buf;
    DEBUG_USB("\n**** usb_device_buf=>%x \n",usb_device_buf);
#if 1
    {
        u32 i, val;
        u8* pos;

        pos = usb_device_buf;
        val = 0;
        for(i=0; i<256; i++)
        {
            *pos = val;
            pos++;
            val++;
        }
    }

#endif

    OSUSB_task_flag = OSFlagCreate(0,0);

	//reset

	DevicePHYTestMode=1;

	DeviceMaskInterrupt=0x0;
	DeviceMaskInterruptG0=~(USBD_MCX_SETUP_INT);
	DeviceMaskInterruptG1=~(USBD_MF0_OUT_INT | USBD_MF0_SPK_INT);
    DeviceMaskInterruptG2=~(USBD_MUSBRST_INT | USBD_MDMA_CMPLT);

	//set pipe FIFO 0:control,1:Bulk out(cmd), 2:Bulk In(resp) 3.Bulk out DMA 4.Bulk in DMA
	DeviceInEP1MaxPktSize=0;     // iso  out
    DeviceOutEP1MaxPktSize=0;    // iso  IN

	DeviceInEP2MaxPktSize=16;       // x
	DeviceOutEP2MaxPktSize=16;

	DeviceOutEP3MaxPktSize=16;       // bulk out reg
	DeviceInEP3MaxPktSize=512;       // bulk IN  reg

	DeviceOutEP4MaxPktSize=512;      // bulk OUT reg

    DeviceInEP5MaxPktSize=32;        // x

    DeviceInEP6MaxPktSize=0;        // x

    DeviceOutEP7MaxPktSize=64;      // bulk OUT

    //DeviceEP1to4Map=0x00112233;

	DeviceEP1to4Map=0x00113333;

    DeviceEP5to8Map=0x33333322;

    DeviceFIFOMap=0xF7151304;

    DeviceFIFOConfig=((USBD_PIPE_TYP_BULK<<0)|(0<<2)|(0<<4)|(1<<5))|
					 ((USBD_PIPE_TYP_BULK<<8)|(0<<10)|(0<<12)|(1<<13))|
					 ((USBD_PIPE_TYP_BULK<<16)|(0<<18)|(0<<20)|(1<<21))|
					 ((USBD_PIPE_TYP_ISO<<24)|(1<<26)|(1<<28)|(0<<29));

	DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
	DeviceFIFO3ByteCnt=(1<<12);     // reset fifo
	#if 0	/*1:Force USB1.1 , 0:USB 2.0*/
	DeviceMainCtl=(1<<5)|(1<<9)(1<<2);     // bit9: force full speed1.1
	#else
	DeviceMainCtl=(1<<5)|(1<<2);     // usb High speed
        #endif
	DeviceAddress=0;
	//soft plug
	DevicePHYTestMode=0;
    IntIrqMask&=(~INT_IRQ_MASK_USB);       // enable USB interrupt

    GLOBALInterruptMask = 0x6;              // disable OTG, Host control interrupt
}

void USB_EPO_output_data(BYTE* data,u16 len)//note len must<=64 BYTES
{
    //OS_FLAGS tmpFlag;
    OS_FLAGS flag;
    u8 err;
    u16 rem, xlen,pos;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    rem = len;
    pos=0;

    while( rem )
    {
        if(rem >= 64 )
            xlen = 64;
        else
            xlen = rem;

        DeviceDMATargetFIFONum=(1<<4);	//control bus
        DeviceDMACtlParam1=((u32)xlen<<8)|(1<<1);
        DeviceDMACtlParam2=(u32)data+pos;
        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        mENABLE_USB_IRQ;

        DeviceDMACtlParam1=((u32)xlen<<8)|(1<<1)|(1<<0);

        while(1)
        {
            mENABLE_USB_IRQ;
            flag = OSFlagPend(OSUSB_task_flag, 0xff, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_TICKS_PER_SEC*50, 0 );

            if(flag==0)
            {
                DEBUG_USB("timeout EP0!");
                break;
            }
            if(flag&0x02)
            {
                //DEBUG_USB("%01x", dbg++);
                OSTimeDly(4*1);
                break;
            }
        }

        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        pos +=xlen;
        rem -= xlen;

    }

    DeviceDMATargetFIFONum=0;
}



void USB_test_packet()
{
/* for high speed test mode; see USB 2.0 spec 7.1.20 */
__align(4) u8 usb_test_packet[] = {
                                         /* implicit SYNC then DATA0 to start */
                                        /* JKJKJKJK x9 */
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         /* JJKKJJKK x8 */
                                        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                        /* JJJJKKKK x8 */
                                        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                        /* JJJJJJJKKKKKKK x8 */
                                        0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                        /* JJJJJJJK x8 */
                                        0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd,
                                         /* JKKKKKKK x10, JK */
                                         //0xfc, 0x7e, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0x7e, 0x00, 0x00,0x00,0x00
                                         0xfc, 0x7e, 0xbf, 0xdf, 0xfb, 0xfd, 0xfb, 0xfd, 0x7e, 0x00, 0x00, 0x00, 0x00
                                         /* implicit CRC16 then EOP to end */
                                        };
	DevicePHYTestMode =0x00000010;
   // HCPortSC = HCPortSC | 0x00040000;
    USB_EPO_output_data((BYTE*)&usb_test_packet,55);

    DEBUG_USB("Enter set tst_pk done routine...\n");
    DeviceCXConfigFIFOEmpS = DeviceCXConfigFIFOEmpS | 0x02;
}

u16 IosPktSizeTbl[9] = {0, 128, 256, 384, 512, 680, 800, 900, 1007 };

void Usb_Ctrl_pipe_Handle()
{
#define USB_CONTROL_PIPE_SET_COMMAND_DONE	(DeviceCXConfigFIFOEmpS|=1)

	USB_DEV_REQ USB_Setup_req;
	u16 u16tmp;
    u32 u32tmp;
    //DEBUG_USB("\nS");
    DeviceDMATargetFIFONum=(1<<4);
     ((u32*)&USB_Setup_req)[0]=DeviceDMACtlParam3;
     ((u32*)&USB_Setup_req)[1]=DeviceDMACtlParam3;
    // DEBUG_USB("\n%x,%x",((u32*)&USB_Setup_req)[0],((u32*)&USB_Setup_req)[1]);
    DeviceDMATargetFIFONum=0;

    switch (USB_Setup_req.bRequest)
    {
        case USB_RC_SET_ADDRESS:
            //DEBUG_USB("\nA");
            	DEBUG_USB("\nSet address to %d",USB_Setup_req.wValue);
			if(reboot_cnt>30)
			{
				DEBUG_USB("USB dongle initial fail, Rebooting...");
				sysForceWDTtoReboot();

			}
			else
				reboot_cnt++;

            DeviceAddress=(DeviceAddress&0x80)|(USB_Setup_req.wValue&0x7f);
            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
            break;
        case USB_RC_GET_DESCRIPTOR:
            //DEBUG_USB("\nD");
            switch ((USB_Setup_req.wValue>>8)&USB_DEV_REQ_REC_MASK)
            {
                case USB_DESC_TYPE_DEVICE:

                    /* get device descriptor */
                    u16tmp=sizeof(USB_DEV_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_DEV_DESC);
                    //DEBUG_USB("\n%x",u16tmp);
                    USB_EPO_output_data((BYTE*)&usb_isp_dev_desc,u16tmp);
                    break;

                case USB_DESC_TYPE_CONFIGURATION:
                    DEBUG_USB("\nGet Config");
                    //u16tmp=sizeof(USB_CFG_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_CFG_DESC);
                    //USB_EPO_output_data((BYTE*)&usb_msc_configuration_desc.cfg,u16tmp);
                    //DEBUG_USB("\n USB_Setup_req.wLength: 0x%08x", USB_Setup_req.wLength);
                    u16tmp=sizeof(USB_ISP_CONFIGURATION_DESC)>USB_Setup_req.wLength?USB_Setup_req.wLength:sizeof(USB_ISP_CONFIGURATION_DESC);
                    USB_EPO_output_data((BYTE*)&usb_isp_configuration_desc,u16tmp);
                    break;

                case USB_DESC_TYPE_STRING:
                     DEBUG_USB("\nGet string");
					 reboot_cnt=0;
                    switch ((USB_Setup_req.wValue)&0xff)
                    {
                        case 0:
                            u16tmp=USB_STR0_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR0_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc0,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 1:
                            u16tmp=USB_STR1_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR1_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc1,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 2:
                            u16tmp=USB_STR2_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR2_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc2,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;
                        case 3:
                            u16tmp=USB_STR3_bLength>USB_Setup_req.wLength?USB_Setup_req.wLength:USB_STR3_bLength;
                            USB_EPO_output_data((BYTE*)&usb_str_desc3,u16tmp);
                            //USB_CONTROL_PIPE_SET_COMMAND_DONE;
                            break;

                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case USB_RC_SET_CONFIGURATION:
            DeviceAddress|=(1<<7);
            //Initial buffer for RF video streaming. aher

            break;
        case USB_RC_SET_INTERFACE:
            UsbAltSetting = USB_Setup_req.wValue;

            if( UsbAltSetting > 8 )
                UsbAltSetting =0;

            DeviceOutEP1MaxPktSize = IosPktSizeTbl[UsbAltSetting];
            DeviceInEP1MaxPktSize  = IosPktSizeTbl[UsbAltSetting];


            DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
            DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
            DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
            DeviceFIFO3ByteCnt=(1<<12);     // reset fifo

            DEBUG_USB("\n SET_INTERFACE %02x  Size: %i", UsbAltSetting, IosPktSizeTbl[UsbAltSetting]);
			if(reboot_cnt>30)
			{
				DEBUG_USB("USB dongle initial fail2, Rebooting...");
				sysForceWDTtoReboot();

			}
			else
				reboot_cnt++;
            break;
         case USB_RC_SET_FEATURE:
          //   UsbTest_mode = USB_Setup_req.wValue.

	         switch (USB_Setup_req.wValue)
                {
	            case TEST_SE0_NAK:
		                DevicePHYTestMode =0x00000008;
		                break;
	            case TEST_J:
		                DevicePHYTestMode =0x00000002;
                        break;
	            case TEST_K:
		                DevicePHYTestMode =0x00000004;
		                break;
	            case TEST_PACKET:
                        USB_test_packet();
		                break;
	            default:
                        break;
                	}

            DEBUG_USB("\n SET_FEATURE %x ", USB_Setup_req.wValue);

            break;

        case USB_RC_GET_INTERFACE:
            DEBUG_USB("\n USB_RC_GET_INTERFACE");
            USB_EPO_output_data((BYTE*)&UsbAltSetting, 1);
            break;
        default:
            break;
    }
    USB_CONTROL_PIPE_SET_COMMAND_DONE;
}

void Usb_Host_Reset_Cmd()
{
    DeviceFIFO0ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO1ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO2ByteCnt=(1<<12);     // reset fifo
    DeviceFIFO3ByteCnt=(1<<12);     // reset fifo
    DeviceAddress=0;
    DeviceInterruptSourceG2=1;

    DEBUG_USB( "\n H2D reset." );
}

void FIFO2_Bulk_In_Transfer(u32 len)
{
    u32 flag;
    u32 cnt;

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    DeviceDMATargetFIFONum = 0x00000004; //fifo2
    DeviceDMACtlParam1=(len<<8)|(1<<1);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    mENABLE_USB_IRQ;
    DeviceDMACtlParam1 |=1;

    while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000);

    DEBUG_USB("# f2 Bulk in \n");
    cnt=0;
    while((DeviceInterruptSourceG1 & USBD_MF2_IN_INT) == 0x00000000)
    {
        cnt++;
        if(cnt>8000)
    	{
    		DEBUG_USB("USBD_MF2_IN timeout\n");
    		break;
    	}
    };
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
}

void FIFO0_H2D_4Byte()
{
    u32 len;
    u32 cnt;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    //DEBUG_USB("DeviceFIFO0ByteCnt=%d\n",DeviceFIFO0ByteCnt);
    len=DeviceFIFO0ByteCnt;
    DeviceDMATargetFIFONum=(1<<0);
    DeviceDMACtlParam1=((u32)len<<8)|(0<<1);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

  //  DEBUG_USB( "\nH2D len: 0x%08X\n", len);
    if(len > 5)
		USB_COM_DATA=1;
    else
		USB_COM_DATA=0;

    mENABLE_USB_IRQ;
    DeviceDMACtlParam1 |= 1;    // start dma

    //while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000);
     	cnt=0;
    while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000)
    {
    	cnt++;
        if(cnt>2000000)
    	{
    		DEBUG_USB("USB: recv timeout\n");
            break;
    	}
    }

    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
    OSTimeDly(1);
}

void FIFO1_D2H_16Byte( )
{
    u32 len;
    u32 cnt;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    len=16;
    DeviceDMATargetFIFONum=(1<<1); //fifo1
    DeviceDMACtlParam1=(len<<8)|(1<<1);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    mENABLE_USB_IRQ;
    DeviceDMACtlParam1 |=1;
    while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000);
    DEBUG_USB("# 16 Bulk F\n");

    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    cnt=0;
    while((DeviceInterruptSourceG1 & USBD_MF1_IN_INT) == 0x00000000)
    {
        cnt++;
        if(cnt>80000)
    	{
    		DEBUG_USB("USB:16_MF1_IN timeout\n");
    		break;
    	}
    };
}

void FIFO0_H2D_512Byte()
{
    u32 len;
    u32 cnt;

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    len=DeviceFIFO0ByteCnt;
    DeviceDMATargetFIFONum=(1<<0);
    DeviceDMACtlParam1=((u32)len<<8);
    DeviceDMACtlParam2=(u32)usb_device_buf;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
    DeviceDMACtlParam1 |= 1;    // start dma

    cnt=0;
    while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000)
    {
        cnt++;
        if(cnt>2000000)
        {
        	DEBUG_USB("USBD_MDMA_ timeout\n");
        	gUSB_UPGRADE=1;
        	break;
        }
    };

    cnt=0;
    while((DeviceInterruptSourceG1 & USBD_MF0_OUT_INT) == 0x00000000)
    {
        cnt++;
		if(cnt>2000000)
		{
			DEBUG_USB("USBD_MF0_ timeout\n");
			gUSB_UPGRADE=1;
			break;
		}
    };
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
    DeviceDMATargetFIFONum = 0x00000000 ;
}

void D2H_XFER(u32 len ,u32 addr)
{
    u32 cnt;
    s32 retry;

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    DeviceFIFO1ByteCnt=(1<<12);
    DeviceDMATargetFIFONum=(1<<1); //fifo1
    DeviceDMACtlParam1=(len<<8)|(1<<1);
    DeviceDMACtlParam2=(u32)addr;
    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

    mENABLE_USB_IRQ;
    DeviceDMACtlParam1 |=1;

 	cnt=0;
    retry=3;	/*Usb dongle retry number when occurs timeout or other errors.*/
	USB_timeout=0;
    while((DeviceInterruptSourceG2 & USBD_MDMA_CMPLT) == 0x00000000)
    {
		cnt++;
		if(retry <=0)
		{
      		DEBUG_USB("USB:Transfer Timeout:%d.\n",retry);
			DeviceDMACtlParam1 = (1<<3);    // dma abort
    	    DeviceFIFO1ByteCnt=(1<<12);
			USB_timeout=1;
			DEBUG_USB("USB dongle rebooting...");
			sysForceWDTtoReboot();
    	    break;
		}
        if(DeviceInterruptSourceG2 & USBD_MDMA_ERROR)
        {
          	DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

            DeviceDMACtlParam1 = (1<<3);    // dma abort
            while(DeviceDMACtlParam1 & 0X00000008);
            DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
        	DEBUG_USB("USB:DMA fail.\n");
            DeviceFIFO1ByteCnt=(1<<12);
            DeviceDMATargetFIFONum=(1<<1); //fifo1
            DeviceDMACtlParam1=(len<<8)|(1<<1);
            DeviceDMACtlParam2=(u32)addr;
            DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

            DeviceDMACtlParam1 |=1;
            cnt=0;
            retry--;
        }

        if(cnt > 8000000 )   /*If counter decrease to 0, means usb transfer timeout.*/
        {
            DeviceDMACtlParam1 = (1<<3);    // dma abort
            while(DeviceDMACtlParam1 & 0X00000008);
            DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
        	DEBUG_USB("USB:Retry start.\n");
            DeviceFIFO1ByteCnt=(1<<12);
            DeviceDMATargetFIFONum=(1<<1); //fifo1
            DeviceDMACtlParam1=(len<<8)|(1<<1);
            DeviceDMACtlParam2=(u32)addr;
            DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

            DeviceDMACtlParam1 |=1;
            cnt=0;
            retry--;
        }

    }

    cnt=0;
    while((DeviceInterruptSourceG1 & USBD_MF1_IN_INT) == 0x00000000)
    {
        cnt++;
        if(cnt>8000000)
        {
        	DEBUG_USB("USBX_MF1_IN timeout\n");
        	break;
        }
    };

    DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

}


void UpdateH264Header(int ch)
{
    if((gRfiuUnitCntl[ch].TX_PicWidth == 640)&&(gRfiuUnitCntl[ch].TX_PicHeight == 352))
    {
        H264_config[0x07] = 0x1E;
        H264_config[0x08] = 0xDA;
        H264_config[0x09] = 0x02;
        H264_config[0x0A] = 0x80;
        H264_config[0x0B] = 0xB6;
        H264_config[0x0C] = 0x40;
    }
    else if((gRfiuUnitCntl[ch].TX_PicWidth == 1280)&&(gRfiuUnitCntl[ch].TX_PicHeight == 720))
    {
        if(updateNALheader[ch] == 2)
        {
            H264_config[0x07] = 0x1E;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x02;
            H264_config[0x0A] = 0x80;
            H264_config[0x0B] = 0xB6;
            H264_config[0x0C] = 0x40;
        }
        else
        {
            H264_config[0x07] = 0x1E;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x01;
            H264_config[0x0A] = 0x40;
            H264_config[0x0B] = 0x16;
            H264_config[0x0C] = 0xE4;
        }
    }
    else if((gRfiuUnitCntl[ch].TX_PicWidth == 1920)&&(gRfiuUnitCntl[ch].TX_PicHeight == 1072))
    {
        if(updateNALheader[ch] == 2)
        {
            H264_config[0x07] = 0x1E;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x02;
            H264_config[0x0A] = 0x80;
            H264_config[0x0B] = 0xB6;
            H264_config[0x0C] = 0x40;
        }
        else
        {
            H264_config[0x07] = 0x28;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x01;
            H264_config[0x0A] = 0xE0;
            H264_config[0x0B] = 0x08;
            H264_config[0x0C] = 0x79;
        }

    }
    else if((gRfiuUnitCntl[ch].TX_PicWidth == 1920)&&(gRfiuUnitCntl[ch].TX_PicHeight == 1088))
    {
        if(updateNALheader[ch] == 2)
        {
            H264_config[0x07] = 0x1E;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x02;
            H264_config[0x0A] = 0x80;
            H264_config[0x0B] = 0xB6;
            H264_config[0x0C] = 0x40;
        }
        else
        {
            H264_config[0x07] = 0x28;
            H264_config[0x08] = 0xDA;
            H264_config[0x09] = 0x01;
            H264_config[0x0A] = 0xE0;
            H264_config[0x0B] = 0x08;
            H264_config[0x0C] = 0x9F;
            H264_config[0x0D] = 0x95;
        }
    }
}
void Init_USB_Session(void)
{
    int i;

    for(i=0; i<MAX_AV_CH; i++)
    {
        USBVideoCmpSemEvt[i]   = OSSemCreate(0);
        USBAudioCmpSemEvt[i]   = OSSemCreate(0);
    }
}

u8 Usb_GET_Streaming_Status(int ch)
{
    return USBEnableStreaming[ch];
}

/*Initial buffer for RF video streaming.*/
void Start_USB_Session(int ch)
{

	/**************************************
    **** Streaming Audio/Video Payload ****
    **************************************/
    unsigned int    cpu_sr = 0;
    INT8U           err;
    u8              level;
    int             i;

    ch = ch % MAX_AV_CH;

    if(USBEnableStreaming[ch] == 0)
    {
        DEBUG_USB("CH %d, start streaming Audio/Video Payload\n",ch);
        if( (rfiuRX_CamOnOff_Sta >> ch) & 0x01)
        {
            OS_ENTER_CRITICAL();
            USBVideoBufReadIdx[ch]  = (rfiuRxVideoBufMngWriteIdx[ch]) % VIDEO_BUF_NUM;
            USBAudioBufReadIdx[ch]  = rfiuRxIIsSounBufMngWriteIdx[ch];
			USBVideoPresentTime[ch] = 100;
            USBAudioPresentTime[ch] = 100;
            USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
            USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
            USBEnableStreaming[ch]  = 1;
            Session_Ready[ch] = 1;
            OS_EXIT_CRITICAL();
            if(gRfiu_Op_Sta[ch]==RFIU_RX_STA_LINK_BROKEN)
            {
                DEBUG_USB("===== %d-CH is Out of Range =====\n",ch);
				DEBUG_USB("Out of Range\n");
            }
         }
         else
         {
                DEBUG_USB("-->RF CH-%d is OFF, Client-%d\n",ch,USBEnableStreaming[ch]);
                OS_ENTER_CRITICAL();
                USBVideoBufReadIdx[ch]  = rfiuRxVideoBufMngWriteIdx[ch];
                USBAudioBufReadIdx[ch]  = rfiuRxIIsSounBufMngWriteIdx[ch];
                USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
                USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
                USBEnableStreaming[ch]  = 1;
                OS_EXIT_CRITICAL();
          }
    }
    else
    {
        if( (rfiuRX_CamOnOff_Sta >> ch) & 0x01)
        {

        }
        else
        {
            DEBUG_USB("-->RF CH-%d is OFF, Client-%d\n",ch,USBEnableStreaming[ch]);
            OS_ENTER_CRITICAL();
            USBVideoBufReadIdx[ch]  = rfiuRxVideoBufMngWriteIdx[ch];
            USBAudioBufReadIdx[ch]  = rfiuRxIIsSounBufMngWriteIdx[ch];
            USBVideoBuf[ch] = rfiuRxVideoBufMng[ch];
            USBAudioBuf[ch] = rfiuRxIIsSounBufMng[ch];
            USBEnableStreaming[ch]  = 1;
            OS_EXIT_CRITICAL();
        }
    }
}

void Stop_USB_Session(int ch)
{
    u8 err;

    ch = ch % MAX_AV_CH;
    USBEnableStreaming[ch]  = 0;
    if(USBEnableStreaming[ch] <= 0)
    {
        DEBUG_USB("CH %d, stop streaming Audio/Video Payload\n",ch);
        OSSemSet(USBVideoCmpSemEvt[ch], 0, &err);
        OSSemSet(USBAudioCmpSemEvt[ch], 0, &err);
		USBEnableStreaming[ch]=0;
        Session_Ready[ch] = 0;
    }
}

void Mars_Pcam_Cmd()
{
    int xxx=0;
 	RTC_DATE_TIME set_time;
	char command_data[100];
	int ch;
	unsigned char digest[16];
	MD5_CTX ctx;
	int bytesRecv;
	int i;
    u8 num1;
	char buf2[50];
	char buf3[2];
    u8* codeAddr = usbfwupgrade_buf;
 	char MD5_buf[33];

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    extern void change_image_mode(u8 req_mode);

    DEBUG_USB( "\n Cmd: 0x%02X  0x%02X  0x%02X  0x%02X  0x%02X", PcamCmd[0], PcamCmd[1], PcamCmd[2],PcamCmd[3],PcamCmd[4]);

    switch( PcamCmd[0] )
    {
        case PCAM_CMD_START_PREVIEW:
            PcamPreviewStart=1;
            USB_timeout=0;
            USB_total_size = 0;
            //USBEnableStreaming[0]  = 1;
            Start_USB_Session(0);					/*CH number			*/
            Start_USB_Session(1);
            Start_USB_Session(2);					/*CH number			*/
            Start_USB_Session(3);
            DEBUG_USB( "\nPreviewStart\n");
            //OSFlagPost(OSMpeg4_encode_task_flag, 0x30000, OS_FLAG_SET, 0);  // reset mpeg4 encode

            //    OS_ENTER_CRITICAL();
            //    UsbOut = (Mpeg4_stream_enc_fifo.out-1)&VIDEO_FIFO_MASK;
            //    OS_EXIT_CRITICAL();
            break;

        case PCAM_CMD_STOP_PREVIEW:
            PcamPreviewStart=0;
            Stop_USB_Session(0);					/*CH number.		*/
            Stop_USB_Session(1);
            Stop_USB_Session(2);					/*CH number.		*/
            Stop_USB_Session(3);
            updateNALheader[0]=0;
            updateNALheader[1]=0;
            updateNALheader[2]=0;
            updateNALheader[3]=0;
            DEBUG_USB( "\nPreview Stop\n");
            break;

        case PCAM_CMD_GET_STATUS:
    		break;

        case PCAM_CMD_GET_RESOLUTION:
    		break;

        case PCAM_CMD_GET_TX_PIR:
    		break;

        case PCAM_CMD_GET_PAIR_STATUS:
            break;

        case PCAM_CMD_SET_RESOLUTION:
            switch( PcamCmd[1] )
            {
                case 0x12:  // QVGA
                  //  change_image_mode(2);
                    UsbImageMode = 0x02;
                    break;

                default:
                case 0x10:  // VGA
              //      change_image_mode(0);
                    UsbImageMode = 0x00;
                    break;
            }
        case PCAM_CMD_SET_TIME:
    		DEBUG_USB("\nPCAM_CMD_SET_TIME\n");
    		xxx=0;
    		FIFO0_H2D_4Byte();
    		DEBUG_USB("\nRECV time para\n");

    		set_time.year=*(usb_device_buf);
    		set_time.month=*(usb_device_buf+1);
    		set_time.day=*(usb_device_buf+2);
    		set_time.hour=*(usb_device_buf+3);
    		set_time.min=*(usb_device_buf+4);
    		set_time.sec=*(usb_device_buf+5);
    		DEBUG_USB("y=%d m=%d d=%d, h=%d m=%d s=%d\n",set_time.year,set_time.month,set_time.day,set_time.hour,set_time.min,set_time.sec);

    		RTC_Set_GMT_Time(&set_time);
    		for(ch=0;ch<4;ch++)
    			uiSetRfTimeRxToTx(ch);
    		break;

        case PCAM_CMD_QUAD_MODE:
    		break;

        case PCAM_CMD_SIGNAL_MODE:
    		break;

        case PCAM_CMD_PAIR:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV PAIR para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nPAIR CH=%d\n",ch);
    		switch(ch)
    		{
    			case 1:
        			DEBUG_USB("Pairing CH1...\n");
        			uiSentKeyToUi(UI_KEY_RF_PAIR_1);
        			break;
    			case 2:
        			DEBUG_USB("Pairing CH2...\n");
        			uiSentKeyToUi(UI_KEY_RF_PAIR_2);
        			break;
    			case 3:
        			DEBUG_USB("Pairing CH3...\n");
        			uiSentKeyToUi(UI_KEY_RF_PAIR_3);
        			break;
    			case 4:
        			DEBUG_USB("Pairing CH4...\n");
        			uiSentKeyToUi(UI_KEY_RF_PAIR_4);
        			break;
    		}
    	    break;

    	case PCAM_CMD_STOP_PAIR:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV STOP PAIR para\n");
    		FIFO0_H2D_4Byte();
    		DEBUG_USB("\nStop pairing CH=%d...\n",*(usb_device_buf));
    		rfiu_PAIR_Stop(*(usb_device_buf)-1);
    	    break;

    	case PCAM_CMD_SET_TXFLIPMIRROR:
    		/*Recv setting data from host.*/
            FIFO0_H2D_4Byte();
    		DEBUG_USB("PCAM_CMD_SET_TXFLIPMIRROR %d\n",*usb_device_buf);
            ch = *usb_device_buf;
            switch(ch)
            {
                case 1:
    				DEBUG_USB(" Set CH1 MIRROR %d...\n",*(usb_device_buf+1));
                    if(*(usb_device_buf+1) == 0)
                    {
                        iconflag[UI_MENU_SETIDX_CH1_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_OFF;
                    }
                    else if(*(usb_device_buf+1) == 1)
                    {
                        iconflag[UI_MENU_SETIDX_CH1_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_RIGHT;
                    }
                    else if(*(usb_device_buf+1) == 2)
                    {
                        iconflag[UI_MENU_SETIDX_CH1_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_DOWN;
                    }
                    else if(*(usb_device_buf+1) == 3)
                    {
                        iconflag[UI_MENU_SETIDX_CH1_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_ALL;
                    }
                break;
                case 2:
    				DEBUG_USB(" Set CH2 MIRROR %d...\n",*(usb_device_buf+1));
                    if(*(usb_device_buf+1) == 0)
                    {
                        iconflag[UI_MENU_SETIDX_CH2_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_OFF;
                    }
                    else if(*(usb_device_buf+1) == 1)
                    {
                        iconflag[UI_MENU_SETIDX_CH2_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_RIGHT;
                    }
                    else if(*(usb_device_buf+1) == 2)
                    {
                        iconflag[UI_MENU_SETIDX_CH2_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_DOWN;
                    }
                    else if(*(usb_device_buf+1) == 3)
                    {
                        iconflag[UI_MENU_SETIDX_CH2_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_ALL;
                    }
                break;
                case 3:
    				DEBUG_USB(" Set CH3 MIRROR %d...\n",*(usb_device_buf+1));
                    if(*(usb_device_buf+1) == 0)
                    {
                        iconflag[UI_MENU_SETIDX_CH3_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_OFF;
                    }
                    else if(*(usb_device_buf+1) == 1)
                    {
                        iconflag[UI_MENU_SETIDX_CH3_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_RIGHT;
                    }
                    else if(*(usb_device_buf+1) == 2)
                    {
                        iconflag[UI_MENU_SETIDX_CH3_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_DOWN;
                    }
                    else if(*(usb_device_buf+1) == 3)
                    {
                        iconflag[UI_MENU_SETIDX_CH3_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_ALL;
                    }
                break;
                case 4:
    				DEBUG_USB(" Set CH4 MIRROR %d...\n",*(usb_device_buf+1));
                    if(*(usb_device_buf+1) == 0)
                    {
                        iconflag[UI_MENU_SETIDX_CH4_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_OFF;
                    }
                    else if(*(usb_device_buf+1) == 1)
                    {
                        iconflag[UI_MENU_SETIDX_CH4_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_RIGHT;
                    }
                    else if(*(usb_device_buf+1) == 2)
                    {
                        iconflag[UI_MENU_SETIDX_CH4_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_DOWN;
                    }
                    else if(*(usb_device_buf+1) == 3)
                    {
                        iconflag[UI_MENU_SETIDX_CH4_MIRROR] = UI_MENU_SETTING_VIDEO_MIRROR_ALL;
                    }
                break;

            }
            uiSentKeyToUi(UI_KEY_RF_FLICKER);
    	    break;

    	case PCAM_CMD_SET_TVSYSTEM:
    		/*Recv setting data from host.*/
    		DEBUG_USB("\nRECV TVSYSTEM para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nCH=%d, TV system=%d\n",ch,*(usb_device_buf+1));
    		if (*(usb_device_buf+1)==1)
    			iconflag[UI_MENU_SETIDX_FLICKER] = UI_MENU_SENSOR_FLICKER_60HZ;
    		else
    			iconflag[UI_MENU_SETIDX_FLICKER] = UI_MENU_SENSOR_FLICKER_50HZ;
    		uiSentKeyToUi(UI_KEY_RF_FLICKER);
    		break;

    	case PCAM_CMD_SET_OSDMODE:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV OSD mode para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nCH=%d, Switch=%d, Type=%d\n",*usb_device_buf,*(usb_device_buf+1),*(usb_device_buf+2));
    		switch(ch)
    		{
    			case 1:
    				DEBUG_USB(" Set CH1 OSD mode...\n");
    				/*Enable or disable OSD*/
    				if(*(usb_device_buf+1)==0)
    					iconflag[ UI_MENU_SETIDX_CH1_TSP_ON] = UI_MENU_TIME_STAMP_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH1_TSP_ON] = UI_MENU_TIME_STAMP_ON;
        				//uiSentKeyToUi (UI_KEY_RF_STAMP_ON_1);
    				/*Select the OSD display mode.*/
    				iconflag [ UI_MENU_SETIDX_CH1_TSP_TYPE] = *(usb_device_buf+2);
        			uiSentKeyToUi (UI_KEY_RF_STAMP_TYPE_1);
    			    break;
    			case 2:
    				DEBUG_USB(" Set CH2 OSD mode...\n");
    				/*Enable or disable OSD*/
    				if(*(usb_device_buf+1)==0)
    					iconflag[ UI_MENU_SETIDX_CH2_TSP_ON] = UI_MENU_TIME_STAMP_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH2_TSP_ON] = UI_MENU_TIME_STAMP_ON;
        				//uiSentKeyToUi (UI_KEY_RF_STAMP_ON_2);
    				/*Select the OSD display mode.*/
    				iconflag [ UI_MENU_SETIDX_CH2_TSP_TYPE] = *(usb_device_buf+2);
        			uiSentKeyToUi (UI_KEY_RF_STAMP_TYPE_2);
    			    break;
    			case 3:
    				DEBUG_USB(" Set CH3 OSD mode...\n");
    				/*Enable or disable OSD*/
    				if(*(usb_device_buf+1)==0)
    					iconflag[ UI_MENU_SETIDX_CH3_TSP_ON] = UI_MENU_TIME_STAMP_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH3_TSP_ON] = UI_MENU_TIME_STAMP_ON;
        				//uiSentKeyToUi (UI_KEY_RF_STAMP_ON_3);
    				/*Select the OSD display mode.*/
    				iconflag [ UI_MENU_SETIDX_CH3_TSP_TYPE] = *(usb_device_buf+2);
        		    uiSentKeyToUi (UI_KEY_RF_STAMP_TYPE_3);
    			    break;
    			case 4:
    				DEBUG_USB(" Set CH4 OSD mode...\n");
    				/*Enable or disable OSD*/
    				if(*(usb_device_buf+1)==0)
    					iconflag[ UI_MENU_SETIDX_CH4_TSP_ON] = UI_MENU_TIME_STAMP_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH4_TSP_ON] = UI_MENU_TIME_STAMP_ON;
        				//uiSentKeyToUi (UI_KEY_RF_STAMP_ON_4);
    				/*Select the OSD display mode.*/
    				iconflag [ UI_MENU_SETIDX_CH4_TSP_TYPE] = *(usb_device_buf+2);
                    uiSentKeyToUi (UI_KEY_RF_STAMP_TYPE_4);
    			    break;
    		}
    		break;

    	case PCAM_CMD_SET_SMALLSTREAM_Q:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV set small stream quality para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nCH=%d, Level=%d\n",*usb_device_buf,*(usb_device_buf+1));
    		switch(ch)
    		{
    			case 1:
    				DEBUG_USB(" Set CH1 quality...\n");
    				/*Set the video quality for small stream.*/
    				iconflag [ UI_MENU_SETIDX_CH1_STREAM_QUALITY] =*(usb_device_buf+1);
                    uiSentKeyToUi (UI_KEY_RF_STREAM_Q_1);
    			break;
    			case 2:
    				DEBUG_USB(" Set CH2 quality...\n");
    				/*Set the video quality for small stream.*/
    				iconflag [ UI_MENU_SETIDX_CH2_STREAM_QUALITY] =*(usb_device_buf+1);
                    uiSentKeyToUi (UI_KEY_RF_STREAM_Q_2);
    			break;
    			case 3:
    				DEBUG_USB(" Set CH3 quality...\n");
    				/*Set the video quality for small stream.*/
    				iconflag [ UI_MENU_SETIDX_CH3_STREAM_QUALITY] =*(usb_device_buf+1);
                    uiSentKeyToUi (UI_KEY_RF_STREAM_Q_3);
    			break;
    			case 4:
    				DEBUG_USB(" Set CH4 quality...\n");
    				/*Set the video quality for small stream.*/
    				iconflag [ UI_MENU_SETIDX_CH4_STREAM_QUALITY] =*(usb_device_buf+1);
                    uiSentKeyToUi (UI_KEY_RF_STREAM_Q_4);
    			break;
    		}
    		break;

        case PCAM_CMD_SET_BRIGHTNESS:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV Camera config para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
            DEBUG_USB("\nCH=%d, bright=%d, version =%d\n",ch,*(usb_device_buf+1));
    		switch(ch)
    		{
    			case 1:
        			DEBUG_USB("Set brightness for CH1...\n");
        			iconflag[UI_MENU_SETIDX_CH1_BRIGHT] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_BRIGHTNESS_1);
        			break;
    			case 2:
        			DEBUG_USB("Set brightness for CH2...\n");
        			iconflag[UI_MENU_SETIDX_CH2_BRIGHT] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_BRIGHTNESS_2);
        			break;
    			case 3:
        			DEBUG_USB("Set brightness for CH3...\n");
        			iconflag[UI_MENU_SETIDX_CH3_BRIGHT] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_BRIGHTNESS_3);
        			break;
    			case 4:
        			DEBUG_USB("Set brightness for CH4...\n");
        			iconflag[UI_MENU_SETIDX_CH4_BRIGHT] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_BRIGHTNESS_4);
        			break;
    		}
    	    break;

    	case PCAM_CMD_SET_CAMSWITCH:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV Camera switch para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nCH=%d, switch=%d\n",ch,*(usb_device_buf+1));
    		switch(ch)
    		{
    			case 1:
        			DEBUG_USB("Set switch for CH1...\n");
        			if(*(usb_device_buf+1)==0)
    					iconflag[UI_MENU_SETIDX_CH1_ON ] = UI_MENU_SETTING_CAMERA_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH1_ON] = UI_MENU_SETTING_CAMERA_ON;
        				uiSentKeyToUi (UI_KEY_RF_CAM_ON_1);
    			    break;
    			case 2:
        			DEBUG_USB("Set switch for CH2...\n");
        			if(*(usb_device_buf+1)==0)
    					iconflag[UI_MENU_SETIDX_CH2_ON ] = UI_MENU_SETTING_CAMERA_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH2_ON] = UI_MENU_SETTING_CAMERA_ON;
        				uiSentKeyToUi (UI_KEY_RF_CAM_ON_2);
    			    break;
    			case 3:
        			DEBUG_USB("Set switch for CH3...\n");
        			if(*(usb_device_buf+1)==0)
    					iconflag[UI_MENU_SETIDX_CH3_ON ] = UI_MENU_SETTING_CAMERA_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH3_ON] = UI_MENU_SETTING_CAMERA_ON;
        				uiSentKeyToUi (UI_KEY_RF_CAM_ON_3);
    			    break;
    			case 4:
        			DEBUG_USB("Set switch for CH4...\n");
        			if(*(usb_device_buf+1)==0)
    					iconflag[UI_MENU_SETIDX_CH4_ON ] = UI_MENU_SETTING_CAMERA_OFF;
    				else
    					iconflag[ UI_MENU_SETIDX_CH4_ON] = UI_MENU_SETTING_CAMERA_ON;
        				uiSentKeyToUi (UI_KEY_RF_CAM_ON_4);
    			    break;
    		}
    	    break;

    	case PCAM_CMD_SET_MOTION:
    		/*Recv setting data from host.*/
    		DEBUG_USB("RECV motion config para\n");
    		FIFO0_H2D_4Byte();
    		ch=*usb_device_buf;
    		DEBUG_USB("\nCH=%d, Sensitivity=%d\n",ch,*(usb_device_buf+1));
    		switch(ch)
    		{
    			case 1:
        			DEBUG_USB("Set motion detect for CH1...\n");
        			iconflag[UI_MENU_SETIDX_CH1_MOTION_SENSITIVITY] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_MOTION_1);
        			break;
    			case 2:
        			DEBUG_USB("Set motion detect for CH2...\n");
        			iconflag[UI_MENU_SETIDX_CH2_MOTION_SENSITIVITY] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_MOTION_2);
        			break;
    			case 3:
        			DEBUG_USB("Set motion detect for CH3...\n");
        			iconflag[UI_MENU_SETIDX_CH3_MOTION_SENSITIVITY] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_MOTION_3);
        			break;
    			case 4:
        			DEBUG_USB("Set motion detect for CH4...\n");
        			iconflag[UI_MENU_SETIDX_CH4_MOTION_SENSITIVITY] = *(usb_device_buf+1);
        			uiSentKeyToUi(UI_KEY_RF_MOTION_4);
        			break;
    		}
    	    break;

        case PCAM_CMD_CAMERA_SEL:
    	    break;

        case PCAM_CMD_CLR_TX_PIR:
    	    break;

    	case PCAM_CMD_GET_OSDMODE:
    		DEBUG_USB("\nSend OSD para\n");
    		*(usb_device_buf+0)=1;
    		*(usb_device_buf+1)=iconflag[ UI_MENU_SETIDX_CH1_TSP_ON];
    		*(usb_device_buf+2)=iconflag[ UI_MENU_SETIDX_CH1_TSP_TYPE];
    		*(usb_device_buf+3)=0;
    		*(usb_device_buf+4)=2;
    		*(usb_device_buf+5)=iconflag[ UI_MENU_SETIDX_CH2_TSP_ON];
    		*(usb_device_buf+6)=iconflag[ UI_MENU_SETIDX_CH2_TSP_TYPE];
    		*(usb_device_buf+7)=0;
    		*(usb_device_buf+8)=3;
    		*(usb_device_buf+9)=iconflag[ UI_MENU_SETIDX_CH3_TSP_ON];
    		*(usb_device_buf+10)=iconflag[ UI_MENU_SETIDX_CH3_TSP_TYPE];
    		*(usb_device_buf+11)=0;
    		*(usb_device_buf+12)=4;
    		*(usb_device_buf+13)=iconflag[ UI_MENU_SETIDX_CH4_TSP_ON];
    		*(usb_device_buf+14)=iconflag[ UI_MENU_SETIDX_CH4_TSP_TYPE];
    		*(usb_device_buf+15)=0;

    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_SMALLSTREAM_Q:
    		DEBUG_USB("\nSend SMALLSTREAM_Q para\n");
    		*(usb_device_buf+0)=1;
    		*(usb_device_buf+1)=iconflag[ UI_MENU_SETIDX_CH1_STREAM_QUALITY];
    		*(usb_device_buf+2)=0;
    		*(usb_device_buf+3)=0;
    		*(usb_device_buf+4)=2;
    		*(usb_device_buf+5)=iconflag[ UI_MENU_SETIDX_CH2_STREAM_QUALITY];
    		*(usb_device_buf+6)=0;
    		*(usb_device_buf+7)=0;
    		*(usb_device_buf+8)=3;
    		*(usb_device_buf+9)=iconflag[ UI_MENU_SETIDX_CH3_STREAM_QUALITY];
    		*(usb_device_buf+10)=0;
    		*(usb_device_buf+11)=0;
    		*(usb_device_buf+12)=4;
    		*(usb_device_buf+13)=iconflag[ UI_MENU_SETIDX_CH4_STREAM_QUALITY];
    		*(usb_device_buf+14)=0;
    		*(usb_device_buf+15)=0;
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_TVSYSTEM:
    		DEBUG_USB("\nSend TV SYSTEM para\n");
    		*(usb_device_buf+0)=0xFF;
    		*(usb_device_buf+1)=iconflag[ UI_MENU_SETIDX_FLICKER];
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_MOTION:
    		DEBUG_USB("\nSend motion detect para\n");
    		*(usb_device_buf+0)=1;
    		*(usb_device_buf+1)=iconflag[ UI_MENU_SETIDX_CH1_MOTION_SENSITIVITY];
    		*(usb_device_buf+2)=0;
    		*(usb_device_buf+3)=0;
    		*(usb_device_buf+4)=2;
    		*(usb_device_buf+5)=iconflag[ UI_MENU_SETIDX_CH2_MOTION_SENSITIVITY];
    		*(usb_device_buf+6)=0;
    		*(usb_device_buf+7)=0;
    		*(usb_device_buf+8)=3;
    		*(usb_device_buf+9)=iconflag[ UI_MENU_SETIDX_CH3_MOTION_SENSITIVITY];
    		*(usb_device_buf+10)=0;
    		*(usb_device_buf+11)=0;
    		*(usb_device_buf+12)=4;
    		*(usb_device_buf+13)=iconflag[ UI_MENU_SETIDX_CH4_MOTION_SENSITIVITY];
    		*(usb_device_buf+14)=0;
    		*(usb_device_buf+15)=0;
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_TIME:
    		DEBUG_USB("\nSend SYSTEM TIME para\n");
    		RTC_Get_Time(&set_time);
    		*(usb_device_buf+0)=set_time.year;
    		*(usb_device_buf+1)=set_time.month;
    		*(usb_device_buf+2)=set_time.day;
    		*(usb_device_buf+3)=set_time.hour;
    		*(usb_device_buf+4)=set_time.min;
    		*(usb_device_buf+5)=set_time.sec;
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_BRIGHTNESS:
    		DEBUG_USB("\nSend camera configuration \n");
            for(i=0;i<4;i++)
            {
                num1 = strlen(gRfiuUnitCntl[i].RFpara.TxCodeVersion) - 1;
                if((gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-6] > 0x49) || (gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-5] > 0x54))
                {
                    *(usb_device_buf+2+i*8) = 2;    //New FW
                    DEBUG_USB("ch %d %d %d\n",i,gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-6],gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-5]);
                }
                else if((gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-4] > 0x48) || (gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-3] > 0x53))
                {
                    *(usb_device_buf+2+i*8) = 2;    //New FW
                    DEBUG_USB("ch %d %d %d\n",i,gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-4],gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-3]);
                }
                else if((gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-2] > 0x50) || (gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-1] > 0x55))
                {
                    *(usb_device_buf+2+i*8) = 2;    //New FW
                    DEBUG_USB("ch %d %d %d\n",i,gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-2],gRfiuUnitCntl[i].RFpara.TxCodeVersion[num1-1]);
                }
                else
                {
                    *(usb_device_buf+2+i*8) = 1;    //Old FW
                    DEBUG_USB("ch %d %s\n",i,gRfiuUnitCntl[i].RFpara.TxCodeVersion);
                }
            }
    		*(usb_device_buf+0)=1;
    		*(usb_device_buf+1)=iconflag[UI_MENU_SETIDX_CH1_BRIGHT];

    		*(usb_device_buf+8)=2;
    		*(usb_device_buf+9)=iconflag[UI_MENU_SETIDX_CH2_BRIGHT];

            *(usb_device_buf+16)=3;
    		*(usb_device_buf+17)=iconflag[UI_MENU_SETIDX_CH3_BRIGHT];

            *(usb_device_buf+24)=4;
    		*(usb_device_buf+25)=iconflag[UI_MENU_SETIDX_CH4_BRIGHT];

            FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_CAMSWITCH:
    		DEBUG_USB("\nSend camera switch para\n");
    		*(usb_device_buf+0)=1;
    		*(usb_device_buf+1)=iconflag[ UI_MENU_SETIDX_CH1_ON];
    		*(usb_device_buf+2)=0;
    		*(usb_device_buf+3)=0;
    		*(usb_device_buf+4)=2;
    		*(usb_device_buf+5)=iconflag[ UI_MENU_SETIDX_CH2_ON];
    		*(usb_device_buf+6)=0;
    		*(usb_device_buf+7)=0;
    		*(usb_device_buf+8)=3;
    		*(usb_device_buf+9)=iconflag[ UI_MENU_SETIDX_CH3_ON];
    		*(usb_device_buf+10)=0;
    		*(usb_device_buf+11)=0;
    		*(usb_device_buf+12)=4;
    		*(usb_device_buf+13)=iconflag[ UI_MENU_SETIDX_CH4_ON];
    		*(usb_device_buf+14)=0;
    		*(usb_device_buf+15)=0;
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_VERSION:
    		DEBUG_USB("\nSend USB FW VERSION.\n");
    		memcpy(usb_device_buf,uiVersion,32);
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_VERSION_TIME:
    		DEBUG_USB("\nSend USB FW VERSION TIME.\n");
    		memcpy(usb_device_buf,uiVersionTime,9);
    		FIFO2_Bulk_In_Transfer(32);
    	    break;

    	case PCAM_CMD_GET_VIDEO_READY:
    		DEBUG_USB("\nCheck Video status...........%d\n",USB_total_size);
    		if(USB_total_size>200)
    			*(usb_device_buf+0)=1; // video is ready.
    		else
    			*(usb_device_buf+0)=0;//video not ready.
    		FIFO2_Bulk_In_Transfer(32);
    	    break;
    	case PCAM_CMD_FW_UPGRADE:
    		/*Recv setting data from host.*/
    		DEBUG_USB("\nFW upgrading.\n");
    		sysDeadLockMonitor_OFF(); /*Turn off watch dog.*/
            memset(codeAddr, 0, 1024*1024*1);
        #if 1 /*Enter Menu screen.*/
          #if (RFIU_SUPPORT)
        	uiSetRfDisplayMode(4);// 4="UI_MENU_RF_ENTER_SETUP" Enter Menu mode to alloc 8MB memory for f/w upgrade.
          #endif
        #endif
            DEBUG_USB("\nFW start.\n");
           // OS_ENTER_CRITICAL();
        	for(xxx=0;xxx<1024*1024;)
    		{
    			FIFO0_H2D_512Byte();
    			memcpy(codeAddr+xxx,usb_device_buf,512);
    			xxx+=512;
    			if(gUSB_UPGRADE==1)
    			{
    				DEBUG_USB("Download firmware  fail.\n");
					sysForceWDTtoReboot();
    				OSTimeDly(100);
    			}
    		}
    		DEBUG_USB("Recv file complete.\n");
    		if (xxx!=(1024*1024))
    		{
    			DEBUG_USB("Firmware length error = %d.\n",xxx);
				sysForceWDTtoReboot();
                OSTimeDly(100);
    		}
        #if 1
    		MD5Init(&ctx);
    		MD5Update(&ctx, (unsigned char*)codeAddr+0x100,1024*1024-0x100);
    		MD5Final(digest,&ctx);
    		for (i = 0; i < 16; i++)
    		{
    			sprintf(buf3,"%02x",digest[i]);
    			buf2[2*i]=buf3[0];
    			buf2[2*i+1]=buf3[1];
    		}
    		buf2[32]='\0';
    		DEBUG_USB("FW MD5_1:%s\n",buf2);

    		for(i = 0; i < 32; i++)
            {
                MD5_buf[i]=(codeAddr+0xa0)[i];
            }
    		DEBUG_USB("FW MD5_2:%s\n",MD5_buf);
    	#endif
    	#if 1
    		if(!strncmp(MD5_buf,buf2,32))
    		{
    		if(ispUpdateAllload_Net(1024*1024)==0)
    			DEBUG_USB("Firmware upgrade fail.\n");
    		else
    			DEBUG_USB("Firmware upgrade success.\n");
    		}
    		else
    			DEBUG_USB("MD5SUM check error!\n");
			sysForceWDTtoReboot();
    		OSTimeDly(100);
    	#endif
    	    break;

    	case PCAM_CMD_USB_RESET:
    		DEBUG_USB("USB dongle reboot...\n");
			sysForceWDTtoReboot();
    		OSTimeDly(100);
    	    break;

        default:
            break;
    }
    PcamCmd[0] = PCAM_CMD_NONE;
}

void Pcam_Reg_Cmd_Handle()
{
    u32 reg;
    u16 wReg;
    u8 *pos,ucom[4];
    //DEBUG_USB("B");

    FIFO0_H2D_4Byte();

    reg = *(u32*)(usb_device_buf);
    pos = usb_device_buf;

    //DEBUG_USB(" %x",*pos);
    ucom[0]=*pos;
    pos++;
    // DEBUG_USB(" %x",*pos);
    ucom[1]=*pos;;
    pos++;
    //DEBUG_USB(" %x",*pos);
    ucom[2]=*pos;
    pos++;
    // DEBUG_USB(" %x",*pos);
    ucom[3]=*pos;
    //reg=reg+((u32)(*pos << 24));

    //  DEBUG_USB( "\n CMD reg: 0x%08X", reg);

    if( ((u8)reg == 0x25) && ( (u8)(reg>>8) == 0x6f ) ) // Reg read
    {
        DEBUG_USB("\n read");

        if( ((u8)(reg>>16) == 0x1f) && ( ucom[3] == 0xf8 ) )
        {
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[0];
             DEBUG_USB("\n 3");
        }
        else if( ((u8)(reg>>16) == 0x1f) && ( ucom[3] == 0xf9 ) )
        {
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[1];

        }
        else if( ((u8)(reg>>16) == 0x1f) && ( ucom[3] == 0xfa ) )
        {
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[2];
             DEBUG_USB("\n 4");
        }
        else if( ((u8)(reg>>16) == 0x1f) && ( ucom[3] == 0xfb ) )
        {
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[3];
            f_com = 1;
            DEBUG_USB("\n 5 f_end");
        }
        else if( ((u8)(reg>>16) == 0x1f) && ( ucom[3] == 0xfc ) )
        {
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[4];
        }
        else
            *((volatile unsigned *)(usb_device_buf)) = PcamCmd[3];

        FIFO1_D2H_16Byte();
        DEBUG_USB("u");
    }
    else    // Reg write
    {
      DEBUG_USB("\n write");
        if( ((u8)(reg>>0) == 0x1f) && ( (u8)(reg>>8) == 0xf8 ) )
        {
            PcamCmd[0] = (u8)(reg>>16);
            PcamCmd[1] = ucom[3];
            DEBUG_USB("\n 2");
            if((PcamCmd[0]!=PCAM_CMD_START_PREVIEW)&& (PcamCmd[0]!=PCAM_CMD_CAMERA_SEL))
                {
                DEBUG_USB("\n ***** 2");
                 f_com = 1;
                }
        }
        if( ((u8)(reg>>0) == 0x1f) && ( (u8)(reg>>8) == 0xf9 ) )
        {
            PcamCmd[1] = (u8)(reg>>16);
            f_com = 0;
             DEBUG_USB("\n f_strat 1");
        }
        if( ((u8)(reg>>0) == 0x1f) && ( (u8)(reg>>8) == 0xfa ) )
        {
            PcamCmd[2] = (u8)(reg>>16);
        }
        if( ((u8)(reg>>0) == 0x1f) && ( (u8)(reg>>8) == 0xfb ) )
        {
            PcamCmd[3] = (u8)(reg>>16);

        }
        if( ((u8)(reg>>0) == 0x1f) && ( (u8)(reg>>8) == 0xfc ) )
        {
            PcamCmd[4] = (u8)(reg>>16);
        }
    }
    if(PcamCmd[0] != PCAM_CMD_NONE)
    {
        Mars_Pcam_Cmd();
    }
   // DEBUG_USB( "\n Pcam_Reg_Cmd: 0x%08X", *((volatile unsigned *)(usb_device_buf)));
}

void D2H_Audio( u32 audAddr, u32 audLen )
{
    u32 flag;
    u32 rem, pos;
    u32 len;
    u32 cnt;
    u32 *header;
   #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    if( UsbAltSetting == 0 )
        return;

    DeviceFIFO2ByteCnt=(1<<12);
    DeviceFIFO3ByteCnt=(1<<12);

    rem = audLen;
    pos = audAddr;

    header = (u32*)audAddr;
    *header = 0xff00ffff;       // signature 1
    header++;
    *header = 0x00048096;       // signature 2


    header++;
    len = *header;
    *header = 4;    // time stamp 4 byte


    //DEBUG_USB( "\n audLen=0x%05x", audLen );

    DEBUG_USB( "a" );
    while(rem)
    {
        if ( rem >= IosPktSizeTbl[UsbAltSetting] )
            len = IosPktSizeTbl[UsbAltSetting];
        else
            len = rem;

        DeviceDMATargetFIFONum=(1<<1);//fifo1
        DeviceDMACtlParam1=(len<<8)|(1<<1);
        DeviceDMACtlParam2=pos;
        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);

        mENABLE_USB_IRQ;
        DeviceDMACtlParam1=(len<<8)|(1<<1)|(1<<0);

        cnt = 0;
        while(1)
        {
            flag = OSFlagPend(OSUSB_task_flag, 0x02, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_TICKS_PER_SEC*200, 0 );
            mENABLE_USB_IRQ;

            if(flag==0)
            {
                DeviceDMACtlParam1 = (1<<3);    // TIMEOUT dma abort
                DeviceFIFO2ByteCnt=(1<<12);
                DeviceFIFO3ByteCnt=(1<<12);
                //DEBUG_USB("timeout Iso!");
                //DEBUG_USB( "c" );
                return;
            }
            if(flag&0x02)
            {
                while( DeviceFIFO2ByteCnt != 0 )
                {
                    //DEBUG_USB( "\n F2Cnt: 0x%08x", DeviceFIFO2ByteCnt );
                    OSTimeDly(1);
                    cnt++;

                    if(cnt > 200*4 )     // wait 200 ms
                    {
                        DeviceDMACtlParam1=(1<<3);    // dma abort
                        DeviceFIFO2ByteCnt=(1<<12);
                        DeviceFIFO3ByteCnt=(1<<12);
                        //PcamPreviewStart=0;
                        //DEBUG_USB( "\n Stop Pcam." );
                        return;
                    }
                }
                //DEBUG_USB( "\n USB IN  Done." );
                break;
            }

        }

        DeviceInterruptSourceG2|=(USBD_MDMA_CMPLT);
        pos += len;
        rem -= len;
    }
}

void D2H_Image( u32 imgAddr, u32 imgLen )
{
    u32 pos,len;
	int packet_len=8192;
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    len = imgLen;
    pos = imgAddr;

	while(len>0)
	{
        if((PcamPreviewStart)&&(USB_timeout==0))
        {
        	if(len > packet_len)
        	{
        		D2H_XFER(packet_len,imgAddr);
        		imgAddr+=packet_len;
        		if(USB_timeout)
        			len=0;
        		else
        			len-=packet_len;
        	}
        	else
        	{
        		D2H_XFER(len,imgAddr);
        		len=0;
        	}
        }
        else
        {
        	len=0;
        	DEBUG_USB("STOP transfer to USB\n.");
        }
	    //OSTimeDly(1);
	}
}
#define header_size 26
u8 USB_AV_header[header_size] = { 0x00,         // [0] video = 1, audio = 2
                                  0x00,                   // [1] CH : 1, 2, 3, 4;
                                  0x00, 0x00, 0x00,       // [2:4] PresentTime
                                  0x00, 0x00, 0x00,       // [5:7] Size
                                  0x00, 0x00, 0x00,       // [8:10] offset
                                  0x00,                   // [11] FrameType
                                  0x00, 0x00,             // [12:13]Time
                                  /***big video***/
                                  0x00, 0x00,             // [14:15] Width
                                  0x00, 0x00,             // [16:17] Height
                                  /***small video***/
                                  0x00, 0x00,             // [18:19] Width
                                  0x00, 0x00,             // [20:21] Height
                                  0x00,                   // [22] FrameRate
                                  /***audio***/
                                  0x00, 0x00,             // [23:24]
                                  0x00,                   // [25] motion
                                  //0x00                    // [25] frame num
                                  };
//unsigned int GRTCTime;
s32 T1 = 0;
s32 T_prev = 0;
void UpdateUSBMIXHeader(int ch,u8 format)
{
    u32 size;
    static unsigned int frame_num[MAX_AV_CH] = 0;
    static s32 smallstream_time[MAX_AV_CH] = 0;

	unsigned int cur_time;
	u32 diff_time;

    cur_time=OSTimeGet();


    USB_AV_header[0] = format;
    USB_AV_header[1] = ch;
    if(format == 1) //audio
    {
        USB_AV_header[2] = USBAudioPresentTime[ch];
        USB_AV_header[3] = USBAudioPresentTime[ch] >> 8;
        USB_AV_header[4] = USBAudioPresentTime[ch] >> 16;
        USB_AV_header[5] = USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size;
        USB_AV_header[6] = USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size >> 8;
        USB_AV_header[7] = USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size >> 16;
        USB_AV_header[11] = 'A';
        USB_AV_header[12] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time;
        USB_AV_header[13] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time >> 8;
        USB_AV_header[23] = gRfiuUnitCntl[ch].BitRate;
        USB_AV_header[24] = gRfiuUnitCntl[ch].BitRate >> 8;
        //DEBUG_USB(" %08d %08d",(u32)USBAudioPresentTime[ch],USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size);
    }
    else    //video
    {
        if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
            USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size += 0x30;

        if((USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size) != (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset))
        {
            smallstream_time[ch] += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time;
            USB_AV_header[2] = smallstream_time[ch];
            USB_AV_header[3] = smallstream_time[ch] >> 8;
            USB_AV_header[4] = smallstream_time[ch] >> 16;
            //DEBUG_USB("#1 %d\n",smallstream_time[ch]);
            smallstream_time[ch] = 0;
        }
        else
        {
            smallstream_time[ch] += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time;
        }

        USB_AV_header[5] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size;
        USB_AV_header[6] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size >> 8;
        USB_AV_header[7] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size >> 16;

        if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
        {
            USB_AV_header[8] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset+ H264_header_size);
            USB_AV_header[9] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset+ H264_header_size) >> 8;
            USB_AV_header[10] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset+ H264_header_size) >> 16;
        }
        else
        {
            USB_AV_header[8] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset);
            USB_AV_header[9] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset) >> 8;
            USB_AV_header[10] = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset) >> 16;
        }

        if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
            USB_AV_header[11] = 'I';
        else
            USB_AV_header[11] = 'P';

        USB_AV_header[12] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time;
        USB_AV_header[13] = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time >> 8;
        USB_AV_header[14] = gRfiuUnitCntl[ch].TX_PicWidth;
        USB_AV_header[15] = gRfiuUnitCntl[ch].TX_PicWidth >> 8;

        USB_AV_header[16] = gRfiuUnitCntl[ch].TX_PicHeight;
        USB_AV_header[17] = gRfiuUnitCntl[ch].TX_PicHeight >> 8;

	    USB_AV_header[18] = 640;
        USB_AV_header[19] = 640 >> 8;

        USB_AV_header[20] = 352;
        USB_AV_header[21] = 352 >> 8;

        USB_AV_header[22] = gRfiuUnitCntl[ch].FrameRate;

        USB_AV_header[23] = gRfiuUnitCntl[ch].BitRate;
        USB_AV_header[24] = gRfiuUnitCntl[ch].BitRate >> 8;
        //USB_AV_header[21] = frame_num[ch];
        if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
            USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size -= 0x30;

        if(GMotionTrigger[ch + MULTI_CHANNEL_LOCAL_MAX] == 1)
        {
            USB_AV_header[25] = 1;
            GMotionTrigger[ch + MULTI_CHANNEL_LOCAL_MAX] = 0;
        }
        else
            USB_AV_header[25] = 0;

    }

}

void usbVcTask(void* pData)
{
    u8 delay_flag;
    u8  *buf_addr;
	u16 video_value[MAX_AV_CH]={0,0,0,0};
	u16 video_value_max[MAX_AV_CH]={0,0,0,0};
	u16 audio_value[MAX_AV_CH]={0,0,0,0};
	u16 audio_value_max[MAX_AV_CH]={0,0,0,0};
    u16 ch,err;
	u32 video_buf_offset=0,Size,tmp,len,cnt1=0,sub_stream_size,buf_size;
	int video_frame_cnt,Audio_cnt;
    int count=0;
    static u8 frame_num[MAX_AV_CH] = {0x0,0x0,0x0,0x0};
    char raytest[]="raymond test file use usb write 123456789";

    //FS_FILE* pFile;
    //FS_FILE* p1File;

    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif

    OS_FLAGS USB_event;

    //GRTCTime=OSTimeGet();
    //DEBUG_USB( "\n sizeof(USB_ISP_CONFIGURATION_DESC): 0x%08x", sizeof(USB_ISP_CONFIGURATION_DESC) );
    usbVC_init();
#if 0
    if ((pFile = dcfOpen("AV.txt", "w")) == NULL)
    {
    	DEBUG_USB("open file error!\n");
    }
//#else
    if ((p1File = dcfOpen("RF.txt", "w")) == NULL)
    {
    	DEBUG_USB("open file error!\n");
    }
#endif

    while (1)
    {
        mENABLE_USB_IRQ;
        USB_event=OSFlagPend (OSUSB_task_flag, 0xffffffff, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_TICKS_PER_SEC*200, 0);
        //mENABLE_USB_IRQ;

        DEBUG_USB("\n.");
        //if( USB_event == 0x00 )
        //    continue;

        if (DeviceInterruptSourceG2&1)
        {
            Usb_Host_Reset_Cmd();
        }
        tmp=DeviceInterruptSourceG0&(USBD_MCX_SETUP_INT);
        if (tmp)
        {
            if (tmp&USBD_MCX_SETUP_INT)
            {
                Usb_Ctrl_pipe_Handle();
            }
        }
        tmp=DeviceInterruptSourceG1&(USBD_MF0_SPK_INT|USBD_MF0_OUT_INT);
        if (tmp)
        {
            Pcam_Reg_Cmd_Handle();
        }

      /*if(USB_event & 0x10)
        {
            DEBUG_USB("Img");
        }*/

	  	if(PcamPreviewStart==0)
	  	{
	  		int xxx=0;
	  		for(xxx=0;xxx<MAX_AV_CH;xxx++)
	  		{
				video_value[xxx]=0;
				video_value_max[xxx]=0;
				audio_value[xxx]=0;
				audio_value_max[xxx]=0;
				//updateNALheader[xxx]=0;
	  		}
			video_buf_offset=0;
			cnt1=0;
	  	}
      	if(PcamPreviewStart && f_com)
	    {
            //time1 = OSTimeGet();    // �t�ήɶ�
            T1 = OSTimeGet();
            if((T_prev - T1) > 20)
            {
                USB_total_size = 0;
                video_buf_offset = 0;
                T_prev = OSTimeGet();
            }
        	while((PcamPreviewStart == 1)&&(USB_timeout == 0))
        	{
        		tmp=DeviceInterruptSourceG1&(USBD_MF0_SPK_INT|USBD_MF0_OUT_INT);
                if (tmp)
                {
                    Pcam_Reg_Cmd_Handle();
                }
        		for(ch=0; ch<4; ch++)
        		{
            	// ------Streaming audio payload------//
                    //if((video_value[ch] == 0) || (USBAudioPresentTime[ch] <= USBVideoPresentTime[ch]))
                    if(0)
                    {
        				if(rfiuRxIIsSounBufMngWriteIdx[ch] < USBAudioBufReadIdx[ch])
        					Audio_cnt = (rfiuRxIIsSounBufMngWriteIdx[ch] + IIS_BUF_NUM) - USBAudioBufReadIdx[ch];
        				else
        					Audio_cnt = rfiuRxIIsSounBufMngWriteIdx[ch]- USBAudioBufReadIdx[ch];

                        audio_value[ch] = Audio_cnt;

                        //DEBUG_USB("a %d\n",Audio_cnt);
                        while((Audio_cnt > 0)&&(USB_timeout == 0))
                        {
                            /* Add mix header ()*/
                            if(1)//(updateNALheader[ch] == 1)
                            {
        					    UpdateUSBMIXHeader(ch,1);
                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += header_size;

                                memcpy_hw(buf_addr ,USB_AV_header,header_size);
                                video_buf_offset += header_size;

                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size;
                                #if 0
                                if (dcfWrite(p1File,USBVideoBuf[0][USBVideoBufReadIdx[0]].buffer, USBVideoBuf[0][USBVideoBufReadIdx[0]].size, &Size) == 0)
                                {
                                        DEBUG_USB(" Write test error!!!\n");
                                        dcfClose(p1File);
                                }
                                #endif
                                if((buf_addr + USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size) > usb_AV_buf_end)
                                {
                                    memcpy_hw(buf_addr ,USBAudioBuf[ch][USBAudioBufReadIdx[ch]].buffer,(usb_AV_buf_end - buf_addr));

                                    buf_size = USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size - (usb_AV_buf_end - buf_addr);
                                    memcpy_hw(usb_AV_buf ,USBAudioBuf[ch][USBAudioBufReadIdx[ch]].buffer + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    memcpy_hw(buf_addr ,USBAudioBuf[ch][USBAudioBufReadIdx[ch]].buffer,USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size);
                                    video_buf_offset += USBAudioBuf[ch][USBAudioBufReadIdx[ch]].size;
                                }
                            }
                            if(Audio_cnt > 0)
                            {
                            	USBAudioBufReadIdx[ch] = (USBAudioBufReadIdx[ch] + 1) % IIS_BUF_NUM;
                            	Audio_cnt--;
                            	OSSemAccept(USBAudioCmpSemEvt[ch]);
                            	USBAudioPresentTime[ch] += (USBAudioBuf[ch][USBAudioBufReadIdx[ch]].time);    //if use chunk time
                            }
        	            }
        	        }
                //------ Streaming video payload------//
        	    	//if((audio_value[ch] == 0) || (USBAudioPresentTime[ch] >= USBVideoPresentTime[ch]))
        	    	if(1)
        			{
        			    //if(gRfiu_Op_Sta[ch] == RFIU_RX_STA_LINK_BROKEN) // out of range
        			    //    usb_outofrange();

        			    /* Rf write index copy to usb read index Start  */
                        if(Session_Ready[ch] == 1)
                        {
            				if(rfiuRxVideoBufMngWriteIdx[ch] < USBVideoBufReadIdx[ch])
            					video_frame_cnt = ((rfiuRxVideoBufMngWriteIdx[ch] + VIDEO_BUF_NUM) - USBVideoBufReadIdx[ch]);
            				else
                                video_frame_cnt = rfiuRxVideoBufMngWriteIdx[ch] - USBVideoBufReadIdx[ch];
                        }
                        else
                        {
                            video_frame_cnt = 0;
                        }

                        video_value[ch] = video_frame_cnt;
                        #if 1
                        if (video_frame_cnt >= 30)
                        {
                            DEBUG_USB("v %d %d %d %d\n",ch,video_frame_cnt,rfiuRxVideoBufMngWriteIdx[ch],USBVideoBufReadIdx[ch]);
                            USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                            video_frame_cnt = 0;

                        }
                            //DEBUG_USB("v %d %d %d %d\n",ch,video_frame_cnt,USBVideoBufReadIdx[ch],rfiuRxVideoBufMngWriteIdx[ch]);
                        #endif
        			    /* Rf write index move to usb read index End  */
                        while((video_frame_cnt>0) && (USB_timeout==0))
                        {
                            T_prev = OSTimeGet();
                            delay_flag = 0;
                            if(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].flag)
                            {
                                /* Add mix header ()*/
                                //updateNALheader[ch] = 0;
                                frame_num[ch] = 0;
                                *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) = 0x84;

                                UpdateUSBMIXHeader(ch,2);
                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += header_size;
                                if((buf_addr + header_size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USB_AV_header,(usb_AV_buf_end - buf_addr));

                                    buf_size = header_size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,USB_AV_header + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    memcpy_hw(buf_addr ,USB_AV_header,header_size);
                                    video_buf_offset += header_size;
                                }

    							UpdateH264Header(ch);
                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += H264_header_size;
                                if((buf_addr + H264_header_size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,H264_config,(usb_AV_buf_end - buf_addr));

                                    buf_size = H264_header_size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,H264_config + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    /* Add H264 header ()*/
                                    memcpy_hw(buf_addr,H264_config,H264_header_size);
        							//updateNALheader[ch] = 2;
                                    video_buf_offset += H264_header_size;
                                }

                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset;
                                if((buf_addr + USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer,(usb_AV_buf_end - buf_addr));

                                    buf_size = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    if((USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset < 0) || (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset);
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset);
                                    video_buf_offset += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset;
                                }

                                updateNALheader[ch] = 2;

    							UpdateH264Header(ch);
                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += H264_header_size;
                                if((buf_addr + H264_header_size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,H264_config,(usb_AV_buf_end - buf_addr));

                                    buf_size = H264_header_size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,H264_config + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    /* Add H264 header ()*/
                                    memcpy_hw(buf_addr,H264_config,H264_header_size);
        							//updateNALheader[ch] = 2;
                                    video_buf_offset += H264_header_size;
                                }

                                buf_addr = usb_AV_buf + video_buf_offset;
                                sub_stream_size = (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size - USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset);
                                USB_total_size += sub_stream_size;
                                if((buf_addr + sub_stream_size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer + USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset,(usb_AV_buf_end - buf_addr));

                                    buf_size = sub_stream_size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer + USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset+ (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    if((sub_stream_size < 0) || (sub_stream_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",sub_stream_size);
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer + USBVideoBuf[ch][USBVideoBufReadIdx[ch]].offset,sub_stream_size);
                                    video_buf_offset += sub_stream_size ;
                                }
                                updateNALheader[ch] = 0;
    					    }
                            else
                            {
                                if(frame_num[ch] == 0xf)
                                    frame_num[ch] = 0x0;
                                else
                                    frame_num[ch]++;

                                if(frame_num[ch] < 0x8)
                                {
                                    if(*(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+5) != 0x9A)
                                        *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+5) = 0x9A;

                                    if((*(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) & 0xE0) != frame_num[ch])
                                    {
                                        *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) &= 0x1f;
                                        *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) |= (frame_num[ch]<<5);
                                    }
                                }
                                else
                                {
                                    *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+5) = 0x9B;
                                    if((*(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) & 0xE0) != frame_num[ch])
                                    {
                                        *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) &= 0x1f;
                                        *(USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer+6) |= (frame_num[ch]<<5);
                                    }
                                }

                                /* Add mix header ()*/
        					    UpdateUSBMIXHeader(ch,2);
                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += header_size;

                                if((buf_addr + header_size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USB_AV_header,(usb_AV_buf_end - buf_addr));

                                    buf_size = header_size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,USB_AV_header + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    memcpy_hw(buf_addr ,USB_AV_header,header_size);
                                    video_buf_offset += header_size;
                                }

                                buf_addr = usb_AV_buf + video_buf_offset;
                                USB_total_size += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size;
                                #if 0
                                    if (dcfWrite(p1File,USBVideoBuf[0][USBVideoBufReadIdx[0]].buffer, USBVideoBuf[0][USBVideoBufReadIdx[0]].size, &Size) == 0)
                                    {
                                        DEBUG_USB(" Write test error!!!\n");
                                        dcfClose(p1File);
                                    }
                                #endif
                                if((buf_addr + USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size) > usb_AV_buf_end)
                                {
                                    if(((usb_AV_buf_end - buf_addr) < 0) || ((usb_AV_buf_end - buf_addr) > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",(usb_AV_buf_end - buf_addr));
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer,(usb_AV_buf_end - buf_addr));

                                    buf_size = USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size - (usb_AV_buf_end - buf_addr);
                                    if((buf_size < 0) || (buf_size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",buf_size);
                                        break;
                                    }
                                    memcpy_hw(usb_AV_buf ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer + (usb_AV_buf_end - buf_addr) ,buf_size);
                                    video_buf_offset = buf_size;
                                }
                                else
                                {
                                    if((USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size < 0) || (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size > 614400)) //300k
                                    {
                                        USBVideoBufReadIdx[ch] = rfiuRxVideoBufMngWriteIdx[ch] ;
                                        video_frame_cnt = 0;
                                        DEBUG_USB("warnning size %d\n",USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size);
                                        break;
                                    }
                                    memcpy_hw(buf_addr ,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].buffer,USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size);
                                    video_buf_offset += USBVideoBuf[ch][USBVideoBufReadIdx[ch]].size;
                                }
    							//updateNALheader[ch] = 0;
                            }
        					if(video_frame_cnt > 0)
        					{
        						USBVideoBufReadIdx[ch] = (USBVideoBufReadIdx[ch] + 1) % VIDEO_BUF_NUM;
        						video_frame_cnt--;
        						OSSemAccept(USBVideoCmpSemEvt[ch]);
            		    	    USBVideoPresentTime[ch] += (USBVideoBuf[ch][USBVideoBufReadIdx[ch]].time); //if use chunk time
        					}
                        }
                    }
                    /* Sent usb_AV_buf to USB */
	       			if(USB_total_size > 0)
	           	 	{
                        T1 = OSTimeGet();
                        //DEBUG_USB("#1 %d %d %d\n",T1,T_prev,USB_total_size);
//                        if((T1 - T_prev) > 20)  // fix stream error(when one camera is using)
//                        {
//
//                            USB_total_size = 0;
//                            video_buf_offset = 0;
//                            cnt1 = 0;
//                            if(delay_flag == 0) // �קK�s��video stream �ߨ�α��b�쥻��video stream����e�X, �ҥH�[�Wdelay ����U�@��stream �e�X.
//                                OSTimeDly(160); // 8s
//                            delay_flag++;
//                        }
                    	buf_addr = usb_AV_buf + 8192 * cnt1;
                    	while((USB_total_size > 8192) && (USB_timeout == 0))
                        {
							if( buf_addr >= usb_AV_buf_end )
                       		{
	                       		cnt1 = 0;
    	                    	buf_addr = usb_AV_buf + 8192 * cnt1;
                	        }
							D2H_Image((u32)buf_addr ,8192);
                            //DEBUG_USB("D2H %d %d %d %d \n",*buf_addr,*(buf_addr+1),*(buf_addr+2),*(buf_addr+3));
                        #if 0
                            if (1)//( cnt1 % 2 == 0)
                            {
                             	if (dcfWrite(pFile,buf_addr, 2048, &Size) == 0)
                                {
                                    DEBUG_USB(" Write test error!!!\n");
                                    dcfClose(pFile);
                                }
                            }
                        #endif
    	                    buf_addr += 8192;
        	                USB_total_size -= 8192;
            	            cnt1 ++;
                            T_prev = OSTimeGet();
                    	}
            	    }
        	    }
                OSTimeDly(1);
            }
            //dcfClose(pFile);
            //dcfClose(p1File);
	    }
	    OSTimeDly(1);
	}
}

void USB_RF_LINK_Broken(void)
{
    DEBUG_USB("USB_RF_LINK_Broken Start\n");
    PcamPreviewStart = 0;
    Stop_USB_Session(0);
    Stop_USB_Session(1);
    Stop_USB_Session(2);
    Stop_USB_Session(3);
    updateNALheader[0] = 0;
    updateNALheader[1] = 0;
    updateNALheader[2] = 0;
    updateNALheader[3] = 0;
    DEBUG_USB("USB_RF_LINK_Broken End\n");
}

void USB_RF_LINK_Success(void)
{
#if 0
    DEBUG_USB("USB_RF_LINK_Success Start\n");

    PcamPreviewStart = 1;
    USB_timeout = 0;
    USB_total_size = 0;
    //USBEnableStreaming[0]  = 1;
    Start_USB_Session(0);
    Start_USB_Session(1);
    Start_USB_Session(2);
    Start_USB_Session(3);

    DEBUG_USB("USB_RF_LINK_Success End\n");
#endif
}

void usbVCntHandler(void)
{
    u8 err;
    OS_FLAGS flag;

    IntIrqMask|=INT_IRQ_MASK_USB;
    // DEBUG_USB("@");
    flag=0x01;

    if(DeviceInterruptSourceG2&USBD_MDMA_CMPLT)
    {
        flag |= 0x02;           // USB DMA complete
        // DEBUG_USB("\nCMP\n");
    }
    if(DeviceInterruptSourceG1& (USBD_MF0_OUT_INT | USBD_MF0_SPK_INT))
        flag |= 0x04;
    if(DeviceInterruptSourceG1& (USBD_MF1_IN_INT | USBD_MF1_SPK_INT))
        flag |= 0x08;
    if(DeviceInterruptSourceG1& (USBD_MF2_IN_INT | USBD_MF2_SPK_INT))
        flag |= 0x10;

    OSFlagPost(OSUSB_task_flag, flag , OS_FLAG_SET, &err);
}
#endif
#endif
