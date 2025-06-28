/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sys.c

Abstract:

    The routines of system control.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"

#include "task.h"
#include "sys.h"
#include "sysapi.h"
#include "siuapi.h"
#include "uiapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "iduapi.h"
#include "jpegapi.h"
#include "mp4api.h"
#include "asfapi.h"
#include "hdmiapi.h"
#include "movapi.h"
#include "timerapi.h"
#include "aviapi.h" /* Peter 0704 */
#include "mpeg4api.h"
#include "iisapi.h"
#include "ispapi.h" /*CY 1023*/
#include "gpioapi.h"
#include "adcapi.h"
#include "uartapi.h"
#include "ClockSwitchApi.h"
#include "awbapi.h"
#include "../ui/inc/ui.h"
#include "smcapi.h"
#include "spiapi.h"
#include "gpsapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "usbapi.h"
#include "uikey.h"
#include "logfileapi.h"
#include "i2capi.h"
#include "SDK_recordapi.h"
#include "../inc/mars_controller/mars_dma.h"
#if (TUTK_SUPPORT)
#include "p2pserver_api.h"
#endif


#include "ciuapi.h"
#include "rfiuapi.h"

#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif
#include "gpiapi.h"

/*************************************************************************************************
* Constant
**************************************************************************************************/

/* define debug print */
#define sysDebugPrint
#define VGA_RAW_OFFSET  3
#define SNAPSHOT_DEBUG  1

#define THRESHOLD_SECONDS  10 // 10seconds
#define SYNC_BTC_LEVEL_COUNTDOWN  35 // 35seconds


/*********************************************************************************************************
* Function prototype
*********************************************************************************************************/
void uiTask(void*);


s32 sysBackLow_Device_Mount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3);
s32 sysBackLow_Device_UnMount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3);
s32 sysBackLow_UI_KEY_SDCD(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3);
s32 sysBackLow_UI_KEY_USB(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3);


s32 sysPreviewInit(s32);
s32 sysPreviewReset(s32);
s32 sysPreviewZoomInOut(s32 zoomFactor);
s32 sysVideoZoomInOut(s32 zoomFactor);

s32 sysTVInChannelChange_Preview(s32 zoomFactor);
s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor);

#if TX_SNAPSHOT_SUPPORT
s32 sysTxCaptureImage_CIU5(s32 Setting);
#endif

#if RX_SNAPSHOT_SUPPORT
s32 sysBack_RFI_RX_DataSave(s32 RFUnit);
#endif

s32 sysSnapshot_OnPreview(s32 dummy);
s32 sysPlaybackZoom(s32);
s32 sysPlaybackPan(s32);
s32 sysPlaybackMoveForward(s32);
s32 sysPlaybackMoveBackward(s32);
s32 sysPlaybackDelete(s32);
s32 sysPlaybackDeleteAll(s32);
s32 sysPlaybackDeleteDir(s32);
s32 sysTVPlaybackDelete(s32);
s32 sysPlaybackFormat(s32);
s32 sysBackupFormat(s32 dummy);

s32 sysPlaybackIsp(s32); /*CY 1023*/
s32 sysCaptureVideo_Init(void); /*BJ 0530 S*/

s32 sysSelfTimer(void); /*CY 0907*/
s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor, u8 BurstNum,u8 ScalingFactor); /*BJ 0530 S*/
s32 sysCaptureImage_OnRFRx(s32 dummy);

s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor);


s32 sysSetUiKey(s32);
s32 sysUpgradeFW(s32);
s32 sysCaptureVideo(s32);  /*BJ 0530 S*/
s32 sysVideoCaptureRoot(s32);
s32 sysVideoCaptureStop(s32 VideoChannelID);
s32 sysPowerOff(s32);
s32 sysMacro(s32);
s32 sysLcdRot(s32);
s32 sysSensorFlip(s32);
s32 sys_Device_Mount(s32 NextDevSel);
s32 sysSDCD_IN(s32);
s32 sysSDCD_OFF(s32);
#if USB_HOST_MASS_SUPPORT
s32 sysUSBCD_IN(s32);
s32 sysUSBCD_OFF(s32);
#endif
s32 sysUIReadFile(s32);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
s32 sysUIContinuousReadFile(s32);
s32 sysUICopyFile(s32);
#endif

s32 sysP2PReadFile(s32);
s32 sysPlaybackCalendar(s32 dummy);
s32 sysWhiteLight(s32);
s32 sysFlashLight(s32);
s32 sysVOICE_REC(s32);
s32 sysSetSeekTime(u32 time);
s32 sysEnableThumb(u8 flag);
s32 sysSeekFile(int time);
u32 sysGetVideoTime(void);
s32 sysContinuousReadFile(void);
s32 sysReadFile(void);
s32 sysbackEXIFWrite(s32);
void sysPendMainPage(void);
void sysPostMainPage(void);
void sysSD_Disable(void);
void sysSD_Enable(void);
void sysNAND_Disable(void);
void sysNAND_Enable(void);
void sysSPI_Disable(void);
void sysSPI_Enable(void);
s32 sysUsbRemoved(s32 dummy);
#if MAKE_SPI_BIN
s8 sysMakeSpiBin(void);
#endif
s32 sysShowTimeOnOSD_VideoClip(s32 dummy);
s32 sysDrawTimeOnVideoClip(s32 dummy);
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT )
s32 sysBack_RFI_RX_CH_Restart(s32 RFUnit);
s32 sysBack_RFI_TX_CH_Del(s32 RFUnit);
s32 sysBack_RFI_TX_CH_Create(s32 RFUnit);
s32 sysBack_RFI_TX_Change_Reso(s32 Setting);
s32 sysTxCaptureImage(s32 Setting);
s32 sysBack_RFI_TX_SnapShot(s32 Setting);

#endif
s32 sysBack_Draw_Battery(s32 level);
s32 sysBack_Check_UI(s32 level);
#if (NET_STATUS_POLLING && NIC_SUPPORT)
s32 sysBack_Draw_NET_Icon(s32 level);
#endif
s32 sysBack_Check_TVinFormat(s32 dummy);
s32 sysBack_Check_VideoinSource(s32 dummy);
s32 sysBack_Set_Sensor_Color(s32 dummy);
s32 sysDrawTimeOnCapture(s32 dummy);
s32 sysBack_Draw_BitRate(s32 value);
s32 sysBack_Draw_FrameRate(s32 value);
s32 sysBack_Draw_OSDString(s32 value);
s32 sysBack_Draw_SD_Icon(s32 value);
s32 sysBackLow_Syn_RF(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3);

void sysPowerOffDirect(void);

/*********************************************************************************************************
* External Functions
*********************************************************************************************************/
extern s32 uiClearFfQuadBuf(s32 index);
extern int marsDMATerminate(INT32U uiDMAId);
extern s32 isuScalar_D2D(
    u8 *Srcbuf , u8 *Dstbuf ,
    u16 sizeX_src, u16 sizeY_src,
    u16 sizeX_dst, u16 sizeY_dst
);
extern s32 isuScalar_D2D_SRGBout(
    u8 *Srcbuf , u8 *Dstbuf ,
    u16 sizeX_src, u16 sizeY_src,
    u16 sizeX_dst, u16 sizeY_dst
);

extern s32 isuCapturePreviewImg(s8 zoomFactor);
extern void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H);

extern void siu_FCINTE_ena(void);
extern void siu_FCINTE_disa(void);

extern s32 ispUpdateWaveFile(void);
extern void idu_switch(void);
extern s32 gpioSetLevel(u8, u8, u8);
extern s32 gpioGetLevel(u8, u8, u8*);
extern void iduWaitCmdBusy(void);
extern void rtcIntEnable(void);
extern void rtcIntDisable(void);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern char smcWriteBackFAT(void);
extern char smcWriteBackUISetting(void);
extern void smcStart(void);
#endif
#if PLAYBEEP_TEST
extern s32 playSoundBeep(s32 select);
#endif
extern char smcWriteBackAWBSetting(void);
extern char Write_protet(void);
extern u32 GetTVFrameBufAdr(u8 buf_idx);
extern s32 exifDecodeJPEGToYUV(u8 *cYUVRawData, u8 *JpegBitStream, u32 uFilesize, u32 *uWidth, u32 *uHeight);
extern void DefectPixel_WriteToNAND_SDV2(void);
extern void siuMergeDefectPixelTab(void);

#if (AUDIO_DEVICE== AUDIO_IIS_WM8974)
extern void Init_IIS_WM8974_play(void);
extern void Init_IIS_WM8974_rec(void);
#elif (AUDIO_DEVICE== AUDIO_IIS_WM8940)
extern void Init_IIS_WM8940_play(void);
extern void Init_IIS_WM8940_rec(void);
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
extern void Init_IIS_ALC5621_play(void);
extern void Init_IIS_ALC5621_rec(void);
extern void Init_IIS_ALC5621_bypass(void);
extern void Close_IIS_ALC5621();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
extern s32 ac97SetupALC203_play(void);
#elif(AUDIO_DEVICE == AUDIO_NULL)
extern void init_DAC_play(u8 start);
#endif

extern void init_serial_A(void);
extern s32 timerConfig(u8 number, TIMER_CFG* pCfg);
extern s32 timerCountEnable(u8 number, u8 enable);
extern s32 timerCountPause(u8 number, u8 pause);
extern s32 iisStopPlay(void);
extern void siuAdjustAE(u16 cur_index);
extern s32 siuSensorInit(u8 siuSensorMode,u8 zoomFactor);
extern void siuSensorSet_60fps(void);
extern void IduVideo_ClearPKBuf(u8 bufinx);
extern s32 i2cRead_SENSOR(u16 addr, u16* pData);
extern s32 i2cWrite_SENSOR(u16 addr, u16 data);
extern void osdDrawFillWait(void);
extern s32 iduSwitchPreview_TV(int src_W,int src_H);
extern s32 marsRfiu_FCC_DirectTXRX(s32 dummy);
extern void sysBack_WriteLog(u8 SysEventID, u8 channel);
#if(DEVSTATUS_ACTIVE_UPDATE)
extern s32 sysBack_DevStatus(u32 isLiveview, u32 isRecvAck);
#endif

/*********************************************************************************************************
* Extern Global Variable
*********************************************************************************************************/
#if RX_SNAPSHOT_SUPPORT
extern DATA_BUF_MNG rfiuRxDataBufMng[];
#endif

#if NIC_SUPPORT
extern u8 Fileplaying;
extern u8 qetIP;
#endif

extern u8 uiMenuVideoSizeSetting;
extern INT32U guiIISCh0RecDMAId, guiIISCh1RecDMAId, guiIISCh2RecDMAId, guiIISCh3RecDMAId;
extern INT32U guiIISCh0PlayDMAId, guiIISCh1PlayDMAId, guiIISCh2PlayDMAId, guiIISCh3PlayDMAId,guiIISCh4PlayDMAId;
extern u32 guiSDReadDMAId;
extern u32 guiSDWriteDMAId;
extern u8  MotionDetect_en;


extern GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT];
extern s16 AWBgain_Preview[3];
extern const s32 d50_IPU_CCM[10];
extern u8 AE_Flicker_50_60_sel;
extern u8 siuVideoZoomSect;     //Lucian 080617
extern u8 got_disk_info;        //civic 070917
extern u8 gInsertNAND;      //civic 070917
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern u8 make_BitMap_ok;
#endif

extern u8 rightflag;
extern u8 pwroff;
extern u8 uiMenuEnable;
extern u8 playbackflag;
extern u8 gUSBDevOn;
extern u8 sysVolumnControl;
extern FS_DISKFREE_T global_diskInfo; //civic 070903
extern u32 RTCseconds;
extern u8 UI_update;  //civic 071002
extern char wav_on;     //civic 071011
extern u8 system_busy_flag;
extern u8 displaybuf_idx;
extern u8 TvOutMode;
extern ISU_IOSize   MSFSz;
extern volatile s32 isu_idufrmcnt;  //Lucian: 取得isu writing ptr.
extern u8 siuOpMode; //Lucian: 用於CapturePreviewImg();
extern u8 Main_Init_Ready;

u8 Iframe_flag = 0; // Decided get I-frame or play whole file
extern u8 batteryflag;
extern u8 VideoClipOnTV;
extern s32 ZoomFactorBackup;
extern u8 TVorEarphone;

extern s32 siuWBComp_RBRatio[4];
extern u8 siuAeEnable;
extern u8 siuAeReadyToCapImage;
extern u16 AECurSet;
extern u8 IISPplyback;
extern u8 usb_msc_mode;

extern u16 OB_B;
extern u16 OB_Gb;
extern u16 OB_R;
extern u16 OB_Gr;
extern u8  osdYShift;
extern u8  StartPlayBack;
extern u8  ResetPlayback;

#if FPGA_BOARD_A1018_SERIES
u8  videoPlayNext=1;
#else
u8  videoPlayNext;
#endif
extern u32 mpeg4Width;
#if MAKE_SPI_BIN
extern u32	spiTotalSize;
#endif

#if OSD_LANGUAGE_ENA
extern u8 CurrLanguage;
#endif
extern OS_FLAG_GRP  *gSdUsbProcFlagGrp;

extern u8 video_playback_speed; //for asf player level control
extern u8 pre_playback_speed;   //for asf player level control

#if(CHIP_OPTION >= CHIP_A1013A)
extern u32 ciu_idufrmcnt_ch1;
extern u32 ciu_idufrmcnt_ch2;
extern u32 ciu_idufrmcnt_ch3;
extern u32 ciu_idufrmcnt_ch4;


extern u8 ciu_1_OpMode;
extern u8 ciu_2_OpMode;
extern u8 ciu_3_OpMode;
extern u8 ciu_4_OpMode;

#endif

#if RFIU_SUPPORT
extern unsigned int gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
extern unsigned int gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];
extern unsigned int gRfiu_WrapEnc_Sta[MAX_RFIU_UNIT];

extern unsigned int gRfiu_TX_Sta[MAX_RFIU_UNIT];
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];

#endif


extern OS_EVENT    *dcfReadySemEvt;
extern OS_EVENT    *gRfiuSWReadySemEvt;

extern u8 UI_SDLastLevel;
extern u8 UI_USBLastStat;

#if INSERT_NOSIGNAL_FRAME
extern u8 Record_flag[MULTI_CHANNEL_MAX];
#endif

#if RF_TX_OPTIMIZE
extern OS_STK VideoTaskStack[];
#endif

extern u32 guiSysTimerCnt;
/*
 *********************************************************************************************************
 * Global Variable
 *********************************************************************************************************
 */
#if ICOMMWIFI_SUPPORT
u32 sys9211TXWifiStat=MR9211_QUIT_WIFI;
u32 sys9211TXWifiUserNum=1;
#endif

#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
u32 Video_totaltime[MAX_RFIU_UNIT]= {0};
u32 Audio_totaltime[MAX_RFIU_UNIT]= {0};
s32 Lose_video_time[MAX_RFIU_UNIT]= {0};
s32 Lose_audio_time[MAX_RFIU_UNIT]= {0};
u8 ASF_Restart[MAX_RFIU_UNIT] = {0};
u8 Motion_Error_ststus[MULTI_CHANNEL_MAX] = {0};
#endif
#if TX_SNAPSHOT_SUPPORT
u32 sysRFTXSnapImgRdy=0;
#endif

OS_EVENT *SysMountEvt;
//
u32 gSystemStorageReady;	// bit 0: Main storage, bit 1: backup storage; instead of gInsertCard and gInsertBackup.
u32 gSystemStorageSel;
u32 gSystemCodeUpgrade;		// 0: Not in Upgrade processdure, 1: Upgrade Now

u32 gUISentKeyRetry = 0;
//

u8  sysBackOsdString[20];
int sysStorageOnlineStat[8] = {0,0,0,0,0,0,0,0};  //Lucian: 檢查儲存裝置是否更新. 設零可強迫讀取BPB.


u8  sysForceWdt2Reboot=0;      // 1: Reboot by WDT, 0: otherwise
u32 sysLifeTime=0;
u32 sysLifeTime_prev=0;
u8  sysDeadLockCheck_ena=1;

u8 sysDeleteFATLinkOnRunning=0;
u8 DefectPixelCalib_switch=0;
u8 sysCheckZoomRun_flag=0;
u8 siuFlashReady = 0;
u32 playpage = 0;
u8 fileitem = 0;
u32 pagecount = 0;
u8 playbackfiletype = 0;
/* current setting */
s32 sysPreviewZoomFactor = 0; /* preview zoom factor = sysPreviewZoomFactor / 10 = [0 ~ 4] */
u8 break_PlayMainPage =0;
u32 playback_location;
u8 pwroff = 0;
u8 SelfTimer;

OS_STK sysTaskStack[SYS_TASK_STACK_SIZE]; /* Stack of task sysTask() */
OS_EVENT* sysSemEvt;
SYS_EVT sysEvt;
int sysTaskStopflag=0;

//Civic 070822 S
SYS_EVT sysbackEvt;
OS_STK sysbackTaskStack[SYSBACK_TASK_STACK_SIZE]; /* Stack of task sysbackTask() */
OS_EVENT* sysbackSemEvt;

SYSBACKLOW_EVT sysbackLowEvt;
OS_STK sysbackLowTaskStack[SYSBACK_LOW_TASK_STACK_SIZE]; /* Stack of task sysbackLowTask() */
OS_EVENT* sysbackLowSemEvt;
OS_EVENT* sysbackLowCountEvt;

SYSBACK_RF_EVT sysback_RF_Evt;
OS_STK sysback_RF_TaskStack[SYSBACK_RF_TASK_STACK_SIZE]; /* Stack of task sysbackRFTask() */
OS_EVENT* sysback_RF_SemEvt;

#if (NIC_SUPPORT == 1)
SYSBACK_NET_EVT sysback_NET_Evt;
OS_STK sysbackNetTaskStack[SYSBACK_NET_TASK_STACK_SIZE]; /* Stack of task sysTask() */
OS_EVENT* sysbackNetSemEvt;
#endif

#if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
OS_EVENT* sysDisplaySemReq;
#define sysDisplayTimeOut 20
#endif

OS_EVENT* sd_backSemEvt;
OS_EVENT* general_MboxEvt;
OS_EVENT* speciall_MboxEvt;
OS_FLAG_GRP  *gSysReadyFlagGrp;
OS_FLAG_GRP  *gSDCExtFlagGrp;
u32 Global_thumbnailImageSize;
u32 Global_primaryImageSize;
u32 Global_APP3VGAImageSize;
u8 system_busy_flag=0;

//Civic 070822 E
SYS_THUMBNAIL sysThumnail[6];
SYS_THUMBNAIL *sysThumnailPtr=&sysThumnail[0];
/*CY 0613 S*/
/* state of capture video */  //civicpend
u8 sysCaptureImageStart;
u8 sysCaptureVideoStart;
u8 sysCaptureVideoStop;
u32 sysCaptureVideoMode;    // ASF_CAPTURE_NORMAL, ASF_CAPTURE_OVERWRITE, ASF_CAPTURE_EVENT
s32 ZoomFactorBackup;
u8 DefectPixel_Msg = 0;
u8  sysPlaybackVideoStart;
u8  sysPlaybackVideoStop;
u8 sysPlaybackVideoStopDone;
u8  sysPlaybackVideoPause;
#if (AVSYNC == VIDEO_FOLLOW_AUDIO)
u8  sysPlaybackForward;
u8  sysPlaybackBackward;
#elif (AVSYNC == AUDIO_FOLLOW_VIDEO)
s8  sysPlaybackForward;
s8  sysPlaybackBackward;
#endif

u8 sysPlayBackNextFile;
u8 sysReady2CaptureVideo;

u8  sysPlaybackThumbnail;
u32 u32PacketStart = 0;
u32 sysUSBPlugInFlag;

u8 userClicksnapshot=0;
u8 userClickvideoRec=0;
u8 userClickvoiceRec=0;
u8 userClickFormat=0;

u8 CaptureVideoRun = 0; /*1: running , 0 : stop*/
BOOLEAN MemoryFullFlag = 0;  /*TRUE: full, FALSE: not full*/
BOOLEAN SysOverwriteFlag = 0;  /*TRUE: overwrite yes, FALSE: not overwrite*/
u32	sysTVinFormat = TV_IN_NTSC;
#if( (Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601))
u32 sysVideoInSel = VIDEO_IN_TV;
#else
u32 sysVideoInSel = VIDEO_IN_SENSOR;
#endif
u8  IsuOutAreaFlagOnTV = FULL_TV_OUT; // 1: VGA 0: QVGA
u8 file_err_flag = 1;
BOOLEAN sysTVInFormatLocked = FALSE;
BOOLEAN sysTVInFormatLocked1 = FALSE;
u8  sysPlaybackYear, sysPlaybackMonth, sysPlaybackDay;
u32  sysPlaybackCamList = 0x0F;
int sysPlaybackHeight=0;

u32 sysPlaybackType=0xff00;
bool bRescan=true;

u8 SYSCheckSchedule = 48;   // 24/0.5 = 48
u8 SYSCheckScheduleGetTime = 10;   /*Per 10 Sec Get RTC time*/
u8 SYSSetScheduleTime = 0;

u8  SysCamOnOffFlag = 0x00;
#if RFIU_RX_WAKEUP_TX_SCHEME 
u8  gSysBatCamTimer[MULTI_CHANNEL_MAX] = {0,0,0,0};
u8  gSysSetRfBTCSleep[MULTI_CHANNEL_MAX]={SYS_SET_RF_NO_NEED,SYS_SET_RF_NO_NEED,SYS_SET_RF_NO_NEED,SYS_SET_RF_NO_NEED};
#if(SW_APPLICATION_OPTION == MR8202_AN_KLF08W)
u16 gSysWakeSecond = 150;
#else
u16 gSysWakeSecond = 30;
#endif
u16 gSysSyncBTCTime = 8;
u8  gSyscheckBTCbatLev = FALSE;
u8  gSysBTCWakeStatus[MULTI_CHANNEL_MAX] = {SYS_BTC_WAKEUP_NO,SYS_BTC_WAKEUP_NO,SYS_BTC_WAKEUP_NO,SYS_BTC_WAKEUP_NO};
u8  SysShowPlay = 0x00;
#endif

#if ENABLE_DOOR_BELL
SYS_DOOR_STATE gSysDoorBellState = SYS_DOOR_NONE;
#endif

/*-------------------- event function -------------------*/
s32 (*sysEvtFunc[])(s32) =
{
    sysPreviewInit,             /* 0x00 - preview initialization */
    sysPreviewReset,            /* 0x01 - preview zoom */
    sysSnapshot_OnPreview,      /* 0x02 - snapshot */
    sysPlaybackZoom,            /* 0x04 - playback zoom */
    sysPlaybackPan,             /* 0x05 - playback pan */
    sysPlaybackMoveForward,     /* 0x06 - playback move forward */
    sysPlaybackMoveBackward,    /* 0x07 - playback move backward */
    sysPlaybackDelete,          /* 0x08 - playback delete */
    sysPlaybackDeleteAll,       /* 0x09 - playback delete all */
    sysPlaybackFormat,          /* 0x0a - playback format */
    sysBackupFormat,
    sysPlaybackIsp,             /* 0x0b - playback ISP */ /*CY 1023*/
    sysVideoCaptureRoot,            /* 0x0c - capture video*/
    sysPowerOn,
    sysPowerOff,                /* 0x0d - Power Off */
    sysMacro,                   /* 0x0e - Set Macro */
    sysLcdRot,                  /* 0x0f - Set LCD Roation */
    sys_Device_Mount,
    sysSDCD_IN,                 /* 0x10 - Set SD Card Plug-in */
    sysSDCD_OFF,                /* 0x12 - set SD Card Take-off*/
#if USB_HOST_MASS_SUPPORT
    sysUSBCD_IN,
    sysUSBCD_OFF,
#endif
    sysWhiteLight,              /* 0x13 - Set Front White Light */
    sysFlashLight,              /* 0x14 - Set Flash Light */
    sysVOICE_REC,               /* 0x15 - VOICE Record*/
    sysUIReadFile,              /* 0x16 - Read File */
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
    sysUIContinuousReadFile,      /* 0x17 - Read File */
    sysUICopyFile,
#endif
    sysPreviewZoomInOut,        /* 0x18 - preview zoom in/out*/
    sysTVPlaybackDelete,        /* 0x19 - playback delete */
    sysVideoZoomInOut,          /* 0x1A - video zoom in/out*/
    sysPlaybackDeleteDir,		/* 0x1B - playback delete all files in current directory */
    sysUsbRemoved,				/* 0x1C - usb removed */
    sysSetUiKey,
    sysUpgradeFW,
    sysDevInsertedUpgradeEvt,
    ispFirmwareNetPrepare,
    ispFirmwareNetUpdateFlow,
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
    ispNetworkUpgradeProcedure,
    ispDriveUpgradeProcedure,
#endif    
#if (MULTI_CHANNEL_VIDEO_REC)
    sysVideoCaptureStop,
#endif
    sysP2PReadFile,
    sysPlaybackCalendar,
    sysGetDiskFree,
#if RX_SNAPSHOT_SUPPORT
    sysBack_RFI_RX_DataSave,
#endif
#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
	dcfScanDiskAll_bdT,
#endif
    rfiuForceResync,

};

s32 (*sysbackEvtFunc[])(s32) =
{
    sysbackEXIFWrite,             /* 0x00 - Backgrounf DCF write function */
#if PLAYBEEP_TEST
    playSoundBeep,                      /* 0x02 */
#endif
    sysShowTimeOnOSD_VideoClip,   /* 0x03 */
    sysSensorFlip,                /* 0x04 */
    sysGetDiskFree,
    sysVideoZoomInOut,
    sysTVInChannelChange_Preview,
    sysTVInChannelChange_CaptureVideo,
    sysPlaybackFormat,
    sysBackupFormat,
    sysDrawTimeOnVideoClip,
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT)
    sysBack_RFI_RX_CH_Restart,
    sysBack_RFI_TX_CH_Del,
    sysBack_RFI_TX_CH_Create,
    sysBack_RFI_TX_Change_Reso,
#endif

#if MOTIONDETEC_ENA
    DrawMotionArea_OnTV,
    DrawMotionArea_OnPanel,
#endif
#if HW_MD_SUPPORT
    DrawMotionArea_OnTV,
#endif
    sysBack_Draw_Battery,
    sysBack_Check_UI,
#if (NET_STATUS_POLLING && NIC_SUPPORT)
    sysBack_Draw_NET_Icon,
#endif
    sysBack_Check_TVinFormat,
    sysBack_Check_VideoinSource,
    sysBack_Set_Sensor_Color,
    sysBack_Draw_BitRate,
    sysBack_Draw_FrameRate,
    sysBack_Draw_OSDString,
    sysBack_Draw_SD_Icon,
#if(HOME_RF_SUPPORT)
    sysBack_Check_HOMERF,
#endif
    sysBack_ScheduleMode,
#if RX_SNAPSHOT_SUPPORT
    sysBack_RFI_RX_DataSave,
#endif
};

s32 (*sysbackLowEvtFunc[])(s32,s32,s32,s32) =
{
    FSFATFreeFATLink_bg,
    sysBackLowGetDiskFree,
#if((FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR) && ((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)))
    dcfOverWriteDel_bgT,
#endif
	sysBackLow_Device_Mount,
    sysBackLow_Device_UnMount,
    sysBackLow_UI_KEY_SDCD,
#if USB_HOST_MASS_SUPPORT
    sysBackLow_UI_KEY_USB,
#endif
    sysBackLow_Syn_RF,
};

s32 (*sysback_RF_EvtFunc[])(s32) =
{
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT )
    sysBack_RFI_RX_CH_Restart,
    sysBack_RFI_TX_CH_Del,
    sysBack_RFI_TX_CH_Create,
    sysBack_RFI_TX_Change_Reso,
    sysBack_RFI_TX_SnapShot,
    spiWriteRF,
    marsRfiu_FCC_DirectTXRX,
    rfiu_SetRXOpMode_1,
    rfiuSetGPO_TX,
    rfiuForceResync,
    uiClearFfQuadBuf,
   #if( (RFIC_SEL==RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) ) 
    A7196_WOR_enable_B1,
   #elif(RFIC_SEL==RFIC_NONE_5M)
    RFNONE_WOR_enable_B1,
   #else
    A7130_WOR_enable_B1,
   #endif
    rfiu_ResendTxMdConfig,
    rfiu_SendTxMdSense,
    rfiu_SetTXTurbo_On,
    rfiu_SetTXTurbo_Off,
    sysCaptureImage_OnRFRx,
    rfiuSetPWM_TX,
    rfiuSetMotorCtrl_TX,
    rfiuSetMelodyNum_TX,
    rfiuSetVoxTrig_RX,
    rfiu_SetTXVoxCfg,
    rfiu_UpdateTXOthersPara,
    Save_UI_Setting_Task,
    uiSetRfTimeRxToTx,
    rfiuSetLightStat_RX,
    rfiu_SetTXReboot,
    rfiu_SetTXPIRCfg,
 #if RFIU_RX_WAKEUP_TX_SCHEME
    rfiu_SetTXDoorBellOff,
  #endif        
#else
    sysTest
#endif
};

#if (NIC_SUPPORT == 1)
s32 (*sysback_Net_EvtFunc[])(u32, u32) =
{
    P2PSendEvent,
    #if !ICOMMWIFI_SUPPORT
    ntpdate,
    #endif
    Upgrade_fw_net,
#if CDVR_SYSTEM_LOG_SUPPORT
    sysBack_WriteLog,
#endif    
#if(DEVSTATUS_ACTIVE_UPDATE)
    sysBack_DevStatus,
#endif
};
#endif
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
/*BJ 060929 S*/
#if ( (LCM_OPTION == LCM_HX8224) ||(LCM_OPTION == LCM_TMT035DNAFWU24_320x240)|| (LCM_OPTION == LCM_HX8817_RGB)|| (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| (LCM_OPTION == LCM_HX8224_SRGB)|| (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || (LCM_OPTION == LCM_HX5073_RGB) || (LCM_OPTION == LCM_HX5073_YUV) ||  (LCM_OPTION == LCM_TPG105) || (LCM_OPTION ==LCM_A015AN04) || (LCM_OPTION == LCM_TD020THEG1)||(LCM_OPTION == LCM_TD036THEA3_320x240) ||(LCM_OPTION == LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02)||(LCM_OPTION == LCM_CCIR601_640x480P)||(LCM_OPTION==LCM_TJ015NC02AA)||(LCM_OPTION == LCM_LQ035NC111))
void SyncIduIsu(void)
{
    u8 i;
    u32 FrameCnt;

    SiuSensCtrl &= 0xFFFFDFFF;
    OSTimeDly(1);
    if ( ((IduWinCtrl&0x0000C000)>>14) ==  ((IsuSCA_MODE&0x000C0000)>>18))
    {
        IsuSCA_EN = 0;
        FrameCnt = (((IduWinCtrl&0x0000C000)>>14)+1)%3;
        IsuSCA_MODE = (IsuSCA_MODE & 0xfffff3ff)|(FrameCnt<<10);

        IsuSCA_EN = 1;
        for (i=0; i<5; i++);
        IsuSCA_EN = 2;
    }

    SiuSensCtrl |= (0x00002000);
}
#endif
/*BJ 060929 E*/
/*

Routine Description:

    Initialize the system control.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysInit(void)
{
    u8 err;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    //-------------------------------------------------//
    //Global Variable Initialize//
    isuStatus_OnRunning = 0;
    sysCameraMode = SYS_CAMERA_MODE_UNKNOWN;

    global_diskInfo.total_clusters=12345678;
    global_diskInfo.avail_clusters=12345678;
    global_diskInfo.bytes_per_sector=512;
    global_diskInfo.sectors_per_cluster=64;

#if AUTO_STORAGE_CHANGE
    sysSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_SDC);
    sysSetStorageSel(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_USBMASS);
#else
#if(FPGA_BOARD_A1018_SERIES)
    sysSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_SDC);
#elif((SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_USB_HOST) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    sysSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_USBMASS);
    sysSetStorageSel(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_SDC);
#elif( (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) )
    sysSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_SDC);
#else
    sysSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_SDC);
    sysSetStorageSel(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_USBMASS);
#endif
#endif
#if CIU2_REPLACE_CIU1
    GpioActFlashSelect |= CHIP_IO_DV2_EN4;
#endif
#if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||\
     (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) )
    GpioActFlashSelect |= CHIP_IO_SEN_EN;
#if(Sensor_OPTION == Sensor_MI_5M) //for raw sensor
    SYS_CHIP_IO_CFG2 &= (~CHIP_IO2_FSND890);
#else
    SYS_CHIP_IO_CFG2 |= CHIP_IO2_FSND890;
#endif
#endif
#if( (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1))
    GpioActFlashSelect |= CHIP_IO_SMPTE2_EN;
#endif
#if (HW_BOARD_OPTION == MR9600_RX_OPCOM_CVI)
    SYS_CHIP_IO_CFG2 |= CHIP_IO2_IIS2_EN;
#endif
#if(HW_BOARD_OPTION == MR9120_TX_RDI_CA840)
    GpioActFlashSelect |=  CHIP_IO_PWM0_EN;
#endif

#if(HW_BOARD_OPTION == MR9100_TX_RDI_CA811)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
    GpioActFlashSelect |=  CHIP_IO_PWM0_EN;
#endif

#if(HW_BOARD_OPTION == A1019A_EVB_128M_TX)  //Only for 2RF+2SDIO WiF+YUV or RAW Sensor model test
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_SD2_EN;
    GpioActFlashSelect |=  CHIP_IO_RFI2_EN;
#endif

#if(HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif

#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
     (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
     (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
     (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
    //senclk to WM8940 27M for 44.1k, then WM8940 I2S to HDMI
    GpioActFlashSelect |=  CHIP_IO_SEN_EN;
    SYS_CTL0 |= SYS_CTL0_SER_MCKEN;
#endif

#if(HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif
#if(HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif

#if(HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif

#if(HW_BOARD_OPTION == MR9100_TX_SKY_W_AHD)
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif

#if ( (HW_BOARD_OPTION == MR9100_TX_DB_AHDIN) || (HW_BOARD_OPTION == MR9100_TX_MUXCOM_AHDIN))
    SYS_CHIP_IO_CFG2 |=  CHIP_IO2_IIS2_EN;
#endif

#if (HW_BOARD_OPTION == MR9600_RX_DB_ETH)
    GpioActFlashSelect |= CHIP_IO_RMII_EN4 | CHIP_IO_UARTB_EN;
#endif

#if (HW_BOARD_OPTION == MR9300_RX_RDI)
    GpioActFlashSelect |= CHIP_IO_SPI3_EN | CHIP_IO_UARTB_EN; //
#endif

#if ((HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530)||(HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8730))
     SYS_CHIP_IO_CFG2 |=  CHIP_IO2_PWM05_EN;
#endif
    sysForceWdt2Reboot=0;
    sysLifeTime=0;
    sysLifeTime_prev=0;
    sysDeadLockCheck_ena=1;
    sysPlayBeepFlag=0;
    sysTaskStopflag=0;

    dcfReadySemEvt = OSSemCreate(1);
    dcfWriteSemEvt = OSSemCreate(1);

#if(SENSOR_FLICKER50_60_SEL == SENSOR_AE_FLICKER_60HZ)
    AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_60HZ;
#else
    AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_50HZ;
#endif

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#if(USE_DBGPRINT_PTR)
    ftmac110isr_dbgprint_enabled(1);
#endif //#if(USE_DBGPRINT_PTR)
#endif
    /*------- SDRAM Performance enhancement -----------*/
    /*//SDRAM Arbit(ICM)grand window
      Ch1: IDU,ISU,
      CH2: Mpeg-0
      CH3: Mpeg-1
      CH4: AHB(CPU,APB)
    */

    SdramArbit = 0x00000000;

    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD | SYS_ARBHIPIR_SIU | SYS_ARBHIPIR_ISU;
    sysProjectSysInit(1);

    //--------------SDRAM Arbit Piority ------------------------//
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 5>1>2>3>4
    SdramTimeCtrl |= 0x80000000;    /* Peter 070130: For DDR Timming Contorl */

    //----------- Power Management----------//
#if( (CHIP_OPTION  == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1016B)  )
#if ( (HW_BOARD_OPTION == MR9600_RX_RDI_AHD) || (HW_BOARD_OPTION == MR9600_RX_SKY_AHD) || (HW_BOARD_OPTION == MR9120_RX_DB_AHD) || (HW_BOARD_OPTION == MR9120_RX_MUXCOM_AHD) || (HW_BOARD_OPTION == MR9120_RX_DB_HDMI))
    SYS_DBGIF_SEL= 0x0e00;  //Lucian: Turn off TV DAC(A,B,C), Audio ADC on
#else
    SYS_DBGIF_SEL= 0x7e00;  //Lucian: Turn on TV DAC(A,B,C), Audio ADC on
#endif
#else
    SYS_DBGIF_SEL= 0x0e00;  //Lucian: Turn off TV DAC(A,B,C), Audio ADC on
#endif

    SYS_CTL0_EXT =0; //Lucian: extend moudle power off.

#if GFU_SUPPORT
    SYS_CTL0_EXT |= SYS_CTL0_EXT_GFX_CKEN;
#endif

#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
      (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))


#if IDU_TV_DISABLE
    IduEna =0;
    SYS_ANA_TEST2=0;
#endif

    sys_ctl0_status     = SYS_CTL0;

    sys_ctl0_status    &= ~SYS_CTL0_USB_CKEN &
                          ~SYS_CTL0_RF1012_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
#if CIU2_REPLACE_CIU1
                          ~SYS_CTL0_CIU_CKEN &
#else
                          ~SYS_CTL0_CIU2_CKEN &
#endif
#if IDU_TV_DISABLE
                          ~SYS_CTL0_IDU_CKEN &
#endif
#if SD_CARD_DISABLE
                          ~SYS_CTL0_SD_CKEN &
#endif
#if !HW_IR_SUPPORT
                          ~SYS_CTL0_IR_CKEN &
#endif
                          ~SYS_CTL0_GPIU_CKEN;

    SYS_CTL0            = sys_ctl0_status;

#elif( (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) )
    sys_ctl0_status     = SYS_CTL0;

    sys_ctl0_status    &= ~SYS_CTL0_USB_CKEN &
                          ~SYS_CTL0_SIU_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                          ~SYS_CTL0_SCUP_CKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_RF1012_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_CIU_CKEN &
                          ~SYS_CTL0_CIU2_CKEN &
                          ~SYS_CTL0_SER_MCKEN;

    SYS_CTL0            = sys_ctl0_status;

#elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
    sys_ctl0_status     = SYS_CTL0;

    sys_ctl0_status    &= ~SYS_CTL0_USB_CKEN &
                          ~SYS_CTL0_SIU_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                          ~SYS_CTL0_SCUP_CKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_RF1012_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_CIU_CKEN &
                          ~SYS_CTL0_CIU2_CKEN;
#if HW_IR_SUPPORT
     sys_ctl0_status|=SYS_CTL0_IR_CKEN;                   
#endif
    SYS_CTL0            = sys_ctl0_status;

    
#elif (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1)
    sys_ctl0_status     = SYS_CTL0;

    sys_ctl0_status    &= ~SYS_CTL0_USB_CKEN ;

    SYS_CTL0            = sys_ctl0_status;
#else

#endif

    /* zero initialize structure */
    /*Twokey 1124 E*/
    memset((void *)&sysEvt, 0, sizeof(SYS_EVT));

    /* Create the semaphore */
    sysSemEvt = OSSemCreate(0);
    SysMountEvt = OSSemCreate(0);

    gSysReadyFlagGrp = OSFlagCreate(0xffffffff, &err);
    gSDCExtFlagGrp = OSFlagCreate(0x0, &err);

#if MULTI_CHANNEL_VIDEO_REC
    gSysSubReadyFlagGrp             = OSFlagCreate(0xffffffff, &err);
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    gRfRxVideoPackerSubReadyFlagGrp = OSFlagCreate(0xffffffff, &err);
#endif
#endif

    /* Flag init for USB and Sd */
    gSdUsbProcFlagGrp = OSFlagCreate(0xFFFFFFFF, &err);

    /* Create the task */
    //DEBUG_SYS("Trace: SYS` task creating\n");
    OSTaskCreate(SYS_TASK, SYS_TASK_PARAMETER, SYS_TASK_STACK, SYS_TASK_PRIORITY);

    /* SLOW Output DDR Slew Rate */
#if ((DDR_SLEWRATE_CONTROL_FOR_CLOCK == 1)&&(CHIP_OPTION == CHIP_A1016A))
    SYS_DDR_PADCTL2 |= 0x04;
#endif

    #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
    sysDisplaySemReq = OSSemCreate(1);
    #endif
    
#if( (HW_BOARD_OPTION == MR9211_TX_MA8806) || (HW_BOARD_OPTION == MR9300_216M_EVB) )  // ICOMM_SUPPORT
	GpioActFlashSelect |=	GPIO_SPI2_FrDISP; //for ICOMM 6030P
#endif

    return 1;
}

/*Civic 070822 S*/
/*

Routine Description:

    Initialize the background system I/O.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sys_background_init(void)
{

    memset((void *)&sysbackEvt, 0, sizeof(SYS_EVT));

    /* Create the semaphore */
    sysbackSemEvt = OSSemCreate(0);
    general_MboxEvt=  OSMboxCreate(NULL);
    speciall_MboxEvt= OSMboxCreate(NULL);
    sd_backSemEvt= OSSemCreate(1);
    /* Create the task */
    DEBUG_MAIN("Trace: SYSBACK task creating\n");
    OSTaskCreate(SYS_BACK_TASK, SYSBACK_TASK_PARAMETER, SYSBACK_TASK_STACK, SYSBACK_TASK_PRIORITY);

    return 1;
}


s32 sys_backLowTask_init(void)
{

    memset((void *)&sysbackLowEvt, 0, sizeof(SYSBACKLOW_EVT));

    /* Create the semaphore */
    sysbackLowSemEvt = OSSemCreate(0);

    /* Create the task */
    DEBUG_MAIN("Trace: SYSBACKLOW task creating\n");
    OSTaskCreate(SYS_BACK_LOW_TASK, SYSBACK_LOW_TASK_PARAMETER, SYSBACK_LOW_TASK_STACK, SYSBACK_LOW_TASK_PRIORITY);
    sysbackLowCountEvt = OSSemCreate(SYSBACKLOW_EVT_MAX-1);

    return 1;
}


s32 sys_back_RF_Task_init(void)  //處理RF task 所發出的Event
{

    memset((void *)&sysback_RF_Evt, 0, sizeof(SYSBACK_RF_EVT));

    /* Create the semaphore */
    sysback_RF_SemEvt = OSSemCreate(0);

    /* Create the task */
    DEBUG_MAIN("Trace: SYSBACK_RF task creating\n");
    OSTaskCreate(SYS_BACK_RF_TASK, SYSBACK_RF_TASK_PARAMETER, SYSBACK_RF_TASK_STACK, SYSBACK_RF_TASK_PRIORITY);

    return 1;
}

#if (NIC_SUPPORT == 1)
s32 sys_back_Network_Task_init(void)
{

    memset((void *)&sysback_NET_Evt, 0, sizeof(SYSBACK_NET_EVT));

    /* Create the semaphore */
    sysbackNetSemEvt = OSSemCreate(0);

    /* Create the task */
    DEBUG_SYS("Trace: SYSBACK_RF task creating\n");
    OSTaskCreate(SYS_BACK_NET_TASK, SYSBACK_NET_TASK_PARAMETER, SYSBACK_NET_TASK_STACK, SYSBACK_NET_TASK_PRIORITY);

    return 1;
}
#endif

/*

Routine Description:

    The test routine of system control.

Arguments:

    None.

Return Value:

    None.

*/

s32 sysTest(s32 dummy)
{
    return 1;
}

/*

Routine Description:

    The SYS task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void sysTask(void* pData)
{
    u8 err;
    s8 cause;
    s32 param;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    // direct goto preview mode

    /*lisa 060929 S*/
#if (AF_ZOOM_OPTION == MI_5M_AF_ZOOM)
    OSTimeDly(5);
    sensor_HWrst();
#endif
    /*lisa 060929 E*/

    while (1)
    {
        if(sysTaskStopflag)
        {
            OSTimeDly(1);
            continue;
        }

        OS_ENTER_CRITICAL();
        if (sysGetEvt(&cause, &param))
        {
            OS_EXIT_CRITICAL();
            //DEBUG_YELLOW("sys w cause: %d\n", cause);
            (*sysEvtFunc[cause])(param);
            //DEBUG_GREEN("sys p cause: %d\n", cause);
        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysSemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysSemEvt is %d.\n", err);
                continue ;
            }
        }
    }
}

int sysGetStorageSel(int StorageIndex)
{
	if(StorageIndex == SYS_V_STORAGE_NONE)
	{
		DEBUG_SYS("[W] Sys params error.\n");
		return SYS_V_STORAGE_NONE;
	}
	
    return (gSystemStorageSel & (StorageIndex * 0xF)) / StorageIndex;
}

int sysSetStorageSel(int StorageIndex, int Status)
{
    gSystemStorageSel &= ~(StorageIndex * 0xF);
    gSystemStorageSel |= (StorageIndex * Status);
    return gSystemStorageSel;
}

int sysKeepSetStorageSel(int StorageIndex, int DevSel, int KeepStorageIndex)
{
    int tmpSelVal, tmpStatusVal;
    if(KeepStorageIndex)
    {
        tmpStatusVal = sysGetStorageStatus(StorageIndex);
        tmpSelVal = sysGetStorageSel(StorageIndex);
        if(DevSel == tmpSelVal)
            return tmpSelVal;

        sysSetStorageStatus(StorageIndex, SYS_V_STORAGE_NREADY);	// At first, default value is Not Ready.
        sysSetStorageSel(StorageIndex, DevSel);
        sysSetStorageStatus(KeepStorageIndex, tmpStatusVal);
        return sysSetStorageSel(KeepStorageIndex, tmpSelVal);
    }
    else
    {
        sysSetStorageStatus(StorageIndex, SYS_V_STORAGE_NREADY);
        return sysSetStorageSel(StorageIndex, DevSel);
    }
}

int sysGetStorageIndex(int StorageType)
{
    int i;

    for(i = 0; i < FS_DEV_ONLINE_MAX; i++)
    {
        if(StorageType == sysGetStorageSel(1 << (SYS_S_STORAGE_ORDER_BIT * i)))
            return 1 << (SYS_S_STORAGE_ORDER_BIT * i);
    }
    return SYS_V_STORAGE_NONE;
}

// File system install status
int sysGetStorageStatus(int StorageIndex)
{
    return (gSystemStorageReady & StorageIndex)? SYS_V_STORAGE_READY: SYS_V_STORAGE_NREADY;
}

int sysSetStorageStatus(int StorageIndex, int Status)
{
	int i;

	for(i = 0; i < FS_DEV_ONLINE_MAX;i++)
	{
		if(StorageIndex == (1 << (SYS_S_STORAGE_ORDER_BIT * i)))
			break;
	}

	if(i >= FS_DEV_ONLINE_MAX)
	{
		DEBUG_SYS("[W] Sys set status error: %#x\n", StorageIndex);
		return -1;
	}

    //DEBUG_MAGENTA("StorageIndex: %d, Status: %d\n", StorageIndex, Status);
    if(Status)
        gSystemStorageReady |= StorageIndex;
    else
        gSystemStorageReady &= ~StorageIndex;
    //DEBUG_MAGENTA("gSystemStorageReady: %#x\n", gSystemStorageReady);
    return gSystemStorageReady & StorageIndex;
}

// plug in/out detection for Hardwave
int sysGetStorageInserted(int StorageIndex)
{
    switch(sysGetStorageSel(StorageIndex))
    {
        case SYS_V_STORAGE_SDC:
            if(sysCheckSDCD() == SDC_CD_OFF)
                return SYS_V_STORAGE_OFF;
            else
                return SYS_V_STORAGE_ON;
        case SYS_V_STORAGE_USBMASS:
#if USB_HOST_MASS_SUPPORT
            if((sysCheckUSBCD() == USB_CD_OFF) || usb_hdd_removed)
                return SYS_V_STORAGE_OFF;
            else
                return SYS_V_STORAGE_ON;
#endif
        case SYS_V_STORAGE_NONE:
        default:
            return SYS_V_STORAGE_OFF;
    }
}

int sysGetStoragePriority(u32 DevSel)
{
	int i, Lvl, Priority;

	for(i = SYS_V_STORAGE_NONE + 1, Priority = 0; i < DevSel; i++)
	{
		Lvl = sysGetStorageInserted(sysGetStorageIndex(i));
		switch(i)
		{
			case SYS_V_STORAGE_USBMASS:
#if USB_HOST_MASS_SUPPORT
				if(!usb_hdd_removed && (Lvl == SYS_V_STORAGE_ON))
					Priority++;
#endif
				break;
			case SYS_V_STORAGE_SDC:
				if(Lvl == SYS_V_STORAGE_ON)
					Priority++;
				break;
			default: 
				DEBUG_SYS("[E] Sys Err idx: %d\n", i);
				break;
		}
	}

	return (1 << (SYS_S_STORAGE_ORDER_BIT * Priority));
}

int sysSetUIKeyRetry(int StorageType)
{
	int i;
#if SD_TASK_INSTALL_FLOW_SUPPORT
	for(i = 0; i < 4; i++)
	{
		if((gUISentKeyRetry & (0xf << (SYS_S_STORAGE_ORDER_BIT * i))) == 0)
		{
			gUISentKeyRetry |= (StorageType << (SYS_S_STORAGE_ORDER_BIT * i));
			break;
		}
	}
	
	return 1;
#else
	// (1).
	// →→→→→→→→→→→→→new? USB  SDC →
	// |----|----|----|----|----|----|----|----|
	// (2).
	// →→→→→→→→→→→→→→→→new? USB →
	// |----|----|----|----|----|----|----|----|
	for(i = 0; i < FS_DEV_ONLINE_MAX; i++)
    {
    	if((gUISentKeyRetry & (0xF << (SYS_S_STORAGE_ORDER_BIT * i))) ==  
    		(StorageType & (0xF << (SYS_S_STORAGE_ORDER_BIT * i))))
    		return 1;
		if(gUISentKeyRetry & (0xF << (SYS_S_STORAGE_ORDER_BIT * i)))
			continue;
		gUISentKeyRetry |= (StorageType << (SYS_S_STORAGE_ORDER_BIT * i));
		return 1;
	}
	return -1;
#endif
}

int sysReleaseUIKeyRetry(void)
{
#if SD_TASK_INSTALL_FLOW_SUPPORT
	gUISentKeyRetry >>= SYS_S_STORAGE_ORDER_BIT;
#else
	if((gUISentKeyRetry & 0xF) == SYS_V_STORAGE_NONE)
		return -1;

	gUISentKeyRetry &= ~(0xF << (SYS_S_STORAGE_ORDER_BIT * 0));
	gUISentKeyRetry >>= SYS_S_STORAGE_ORDER_BIT;
#endif
	return 1;
}

u32 sysGetOneUIKeyRetry(void)
{
    return gUISentKeyRetry & 0xF;
}

void sysLockMountSeq(void)
{
	u8 err;
	OSSemPend(SysMountEvt, OS_IPC_WAIT_FOREVER, &err);
	if(err != OS_NO_ERR)
		DEBUG_SYS("[E] SysMountEvt: %d\n", err);
}

void sysUnlockMountSeq(void)
{
	OSSemPost(SysMountEvt);
}

s32 sysSentMountSeq(int DevSel)
{
	u32 DevPriority, ret;

	DevPriority = sysGetStoragePriority(DevSel);
	//DEBUG_CYAN("[I] Sys Start mount(%#x) seq.\n", DevPriority);
	// Ask ui layer to stop the video recording only in Main storage.
	if((DevPriority == SYS_I_STORAGE_MAIN) && (Main_Init_Ready == 1))
	{
		ret = 1;
		// HDD stop recording already. Skip it.
#if USB_HOST_MASS_SUPPORT		
		if((DevSel == SYS_V_STORAGE_USBMASS) && (usb_hdd_removed == 1))
			ret = 0;
#endif
		if(ret)
		{
#if SD_TASK_INSTALL_FLOW_SUPPORT
			if(!uiSentKeyToUi(UI_KEY_DEVCD))
				sysSetUIKeyRetry(UI_KEY_DEVCD);
			//if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
				sysLockMountSeq();	// Unlock is called by UI_KEY
			//DEBUG_GREEN("# SysMountEvt released.\n");
#endif
		}	
	}
	sysSetEvt(SYS_EVT_MOUNT, DevSel);
	return 1;
}

s32 sysSentUnMountSeq(int DevSel)
{
	return 1;
}

s32 sys_Device_Mount(s32 NextDevSel)
{
	//DEBUG_DARKGARY("[I] sys START seq.\n");
	
	// Wait for recording procedure finish.
	
#if MULTI_CHANNEL_VIDEO_REC
	while(MultiChannelCheckRecordChannel() != 0)
	{
		OSTimeDly(1);
	}
#endif
	// After recording finish, start device mounting procedure.
	sysbackLowSetEvt(SYSBACKLOW_EVT_MOUNT_DEV, NextDevSel, 0, 0, 0);
	//DEBUG_DARKGARY("[I] sys LEAVE seq.\n");
	return 1;
}

s32 sysBackLow_Device_Mount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3)
{
	u32 InsteadDevSel, InsteadDevPrio, InsteadDevStatus;
	u32 NextDevIndex, tmpIdx;
	int ret, i;

	DEBUG_MAGENTA("[I] sysBackLow START seq.\n");

	InsteadDevPrio = sysGetStoragePriority(NextDevSel);
	
	InsteadDevStatus = sysGetStorageStatus(InsteadDevPrio);
	InsteadDevSel = sysGetStorageSel(InsteadDevPrio);

	// Unmount the device then that device will be replaced.
	if((InsteadDevStatus == SYS_V_STORAGE_READY) && (InsteadDevSel == NextDevSel))
	{
		sysSetStorageStatus(InsteadDevPrio, SYS_V_STORAGE_NREADY);
		switch(InsteadDevSel)
		{
			case SYS_V_STORAGE_USBMASS:
#if USB_HOST_MASS_SUPPORT			
					sysUSBCD_OFF(0);
#endif
				break;
			case SYS_V_STORAGE_SDC:
					sysSDCD_OFF(0);
				break;
			default: break;
		}
	}

	// Mount the NextDevSel device
	NextDevIndex = sysGetStorageIndex(NextDevSel);
	if(sysGetStorageInserted(NextDevIndex) == SYS_V_STORAGE_ON)
	{
		sysKeepSetStorageSel(InsteadDevPrio, NextDevSel, NextDevIndex);
		switch(NextDevSel)
		{
			case SYS_V_STORAGE_USBMASS:
#if USB_HOST_MASS_SUPPORT
				ret = sysUSBCD_IN(0);
#endif				
				break;
			case SYS_V_STORAGE_SDC:
				ret = sysSDCD_IN(0);
				break;
			default: break;
		}

		if(ret < 0)
		{
			//sysKeepSetStorageSel(NextDevIndex, NextDevSel, InsteadDevPrio);
			DEBUG_SYS("[W] File sytem install failed.\n");
			//return -1;
		}

		// The dev had been switched need to install again.	
		DEBUG_SYS("[I] InsteadDevSel: %d, %d\n", sysGetStorageSel(NextDevIndex), sysGetStorageSel(InsteadDevPrio));
		if((InsteadDevPrio != NextDevIndex) &&
			(sysGetStorageInserted(InsteadDevPrio) == SYS_V_STORAGE_ON))	
		{
			switch(InsteadDevSel)
			{
				case SYS_V_STORAGE_USBMASS:
#if USB_HOST_MASS_SUPPORT
					ret = sysUSBCD_IN(0);
#endif					
					break;
				case SYS_V_STORAGE_SDC:
					ret = sysSDCD_IN(0);
					break;
				default: break;
			}
		}
	}
	else
	{
		// System needs another one to fit the hole of storage if the dev has removed.
		for(i = 0; i < FS_DEV_ONLINE_MAX; i++)
	    {
#if 0	// Bad drive will not be promoted	    
	    	tmpIdx = 1 << (SYS_S_STORAGE_ORDER_BIT * i);
	        if(sysGetStorageStatus(tmpIdx) == SYS_V_STORAGE_READY)
	        {
	        	InsteadDevSel = sysGetStorageSel(tmpIdx);
	        	if(InsteadDevPrio == sysGetStoragePriority(InsteadDevSel))
	        		break;
	        }
#else
			tmpIdx = 1 << (SYS_S_STORAGE_ORDER_BIT * i);
			if(tmpIdx <= InsteadDevPrio)
				continue;
			if(sysGetStorageInserted(tmpIdx) == SYS_V_STORAGE_ON)
	        {
	        	InsteadDevSel = sysGetStorageSel(tmpIdx);
	        	if(InsteadDevPrio == sysGetStoragePriority(InsteadDevSel))
	        		break;
	        }
#endif
	    }

	    if(i < FS_DEV_ONLINE_MAX)
	    {
	    	switch(InsteadDevSel)
			{
				case SYS_V_STORAGE_USBMASS:
					sysKeepSetStorageSel(InsteadDevPrio, InsteadDevSel, tmpIdx);
#if USB_HOST_MASS_SUPPORT
					sysUSBCD_IN(0);
#endif
					break;
				case SYS_V_STORAGE_SDC:
					sysKeepSetStorageSel(InsteadDevPrio, InsteadDevSel, tmpIdx);
					sysSDCD_IN(0);
					break;
				default: break;
			}
	    }
	}
	DEBUG_MAGENTA("[I] sysBackLow LEAVE seq.\n");
#if !defined(NEW_UI_ARCHITECTURE)
	if(InsteadDevPrio == SYS_I_STORAGE_MAIN)
    	uiFlowSdCardMode();
#endif
	return 1;
}

s32 sysBackLow_Device_UnMount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3)
{

}

s32 sysBackLow_UI_KEY_SDCD(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3)
{
    u8 ui_doublechack_SD;
    u8 err;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_STAT, OS_FLAG_CLR, &err);
    do
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_CLR, &err);
        UI_SDLastLevel= sysCheckSDCD();
        //osdDrawMemFull(UI_OSD_CLEAR);

        uiCheckSDCD(0);

#if !defined(NEW_UI_ARCHITECTURE)
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
		sysSentUiKeyTilOK(UI_KEY_SDINIT);
#else
		uiFlowSdCardMode();
#endif
#endif
        ui_doublechack_SD = sysCheckSDCD();
        DEBUG_SYS(" ---->UI_SDLastLevel =%d ,ui_doublechack_SD =%d \n ",UI_SDLastLevel,ui_doublechack_SD);

    }
    while(ui_doublechack_SD!=UI_SDLastLevel);
    uiFlowCardReady(ui_doublechack_SD);
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_STAT, OS_FLAG_SET, &err);
    return 1;
}

#if USB_HOST_MASS_SUPPORT
s32 sysBackLow_UI_KEY_USB(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3)
{
    u8 ui_doublechack_USB;
    u8 err;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_STAT, OS_FLAG_CLR, &err);
    do
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_CLR, &err);
        UI_USBLastStat = sysCheckUSBCD();
        //osdDrawMemFull(UI_OSD_CLEAR);

        uiCheckUSBCD(0);
        
#if !defined(NEW_UI_ARCHITECTURE)
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
		sysSentUiKeyTilOK(UI_KEY_SDINIT);
#else
		uiFlowSdCardMode();
#endif

#endif
        ui_doublechack_USB = sysCheckUSBCD();
        DEBUG_SYS(" ---->UI_USBLastLevel =%d ,ui_doublechack_USB =%d \n ",UI_USBLastStat, ui_doublechack_USB);

    }
    while(ui_doublechack_USB != UI_USBLastStat);
    uiFlowCardReady(ui_doublechack_USB);
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_STAT, OS_FLAG_SET, &err);
    return 1;
}
#endif

/*

Routine Description:

    sysSDCD_IN.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSDCD_IN(s32 mode)
{
    int ret;
    int devIdx, storageIdx;
    u8 err;
    void* msg;
    //
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY | FLAGSYS_RDYSTAT_CARD1_ERR, OS_FLAG_CLR, &err);
    OSFlagPost(gSDCExtFlagGrp, 0xF, OS_FLAG_CLR, &err);
    // flag to announce to usb is doing SD init
    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);
    // flag to pend to wait for usb check by SD functions
    OSFlagPend(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_WAIT_SET_ANY, TIMEOUT_SDC, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SYS("Error SYS: gSdUsbProcFlagGrp is %d\n", err);
    }
    // old stuff, useless
    system_busy_flag = 1;
    gInsertNAND = 0;
    //
    devIdx = DCF_GetDeviceIndex("sdmmc");
    storageIdx = sysGetStorageIndex(SYS_V_STORAGE_SDC);

    sdcUnInit();	/* del all related semaphore about SD control */

    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_OFF)
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
        return -1;
    }

    DEBUG_SYS("do sysSDCD_IN: %#x.\n", storageIdx);
    switch(storageIdx)
    {
        case SYS_I_STORAGE_MAIN:
            got_disk_info = 0;
            dcfFileTypeCount_Clean();
            sysProjectSDCD_IN(2);	//------Lucian: SD 卡插入後燈號處理-------
            //---------File system initialize-----------
            sysStorageOnlineStat[devIdx] = 0;  // Add this will Let FAT to re-read the BPB section
            dcfUninit();
            OSSemSet(dcfReadySemEvt, 1, &err);
            ret = dcfInit(STORAGE_MEMORY_SD_MMC);
            DEBUG_SYS("ret = %#x\n", ret);
            // distinguish the result value whether is from low layer of SDC or not.
            if(sdcErrorResultFilter(ret) < 0)
            {
                system_busy_flag = 0;
                sysProjectSDCD_IN(13);
                sysProjectSDCD_IN(14);
                // print the SD Error OSD msg to remind the user this SD card had a big problem
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                OSFlagPost(gSDCExtFlagGrp, 0x1, OS_FLAG_SET, &err);
                return -1;
            }
            switch(ret)
            {
                // SD card init & File system init success
                case 1:
                    break;
                // SDC is not inserted.
                case -2:
                    system_busy_flag = 0;
                    DEBUG_SYS("Error: No SD Card.\n");
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                    return -1;

                default:
                    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_OFF)
                    {
                        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                        return -1;
                    }
#if SYS_NOT_SUPPORT_INSERT_FORMAT
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                    return -1;
#endif
                    DEBUG_SYS("Error: FS Operation error.\n");
                    if(Main_Init_Ready == 0)
                        Main_Init_Ready = 1;	// To release the lock of UI page. Touch, Key needs the unlock.
                    sysProjectSDCD_IN(5);
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_ERR, OS_FLAG_SET, &err);
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_SET, &err);		// XOR Format procedure
                    sysDeadLockMonitor_OFF();
                    // release SD Init flag for USB Plug-in?
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
                    speciall_MboxEvt->OSEventPtr = (void *)0;
                    uiSetGoToFormat();
                    msg = OSMboxPend(speciall_MboxEvt, OS_IPC_WAIT_FOREVER, &err);
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_CLR, &err);		// XOR Format procedure
                    sysDeadLockMonitor_ON();
                    if(strcmp((const char*)msg, "PASS"))
                    {
                        system_busy_flag = 0;
                        if(strcmp((const char*)msg, "FORMAT FAIL"))
                        {
                            // if usb is plug-in when formatting SD card, gpioIntHandler would terminate the formatting procedure.
                            //  Otherwise, usb would not work until formatting procedure is finished.
                            DEBUG_SYS("TRACE SYS: Terminate Formatting SD Card!\n");
                            sysProjectSDCD_IN(7);
                        }
                        else
                        {
                            sysProjectSDCD_IN(9);
                            sysProjectSDCD_IN(10);
                        }
                        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                        return -1;
                    }
                    else
                    {
                        // Continue to implement the SD init
                        sysProjectSDCD_IN(12);
                        OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);
                    }
                    break;
            }
#if MAKE_SPI_BIN
            {
                // make bin from serial flash to SD card
                // This is used for mass burning data to serial flash in mass production
                sysDeadLockMonitor_OFF();
                DEBUG_SYS("Making BIN...\n");
                uiMenuOSDFrame(OSD_SizeX , 16*OSD_STRING_W , 16 , (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2) , OSD_Blk2 , 0);
                uiOSDASCIIStringByColor("Making BIN...", (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                sysMakeSpiBin();
                DEBUG_SYS("Finished!\n");
                uiMenuOSDFrame(OSD_SizeX , 16*OSD_STRING_W , 16 , (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2) , OSD_Blk2 , 0);
                uiOSDASCIIStringByColor("OK! Please Re-boot!", (OSD_SizeX-20*OSD_STRING_W)/2 , 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);
                sysProjectSDCD_IN(16);
                while(1);
            }
#endif
            //--------------------------In system programming --------------------------
            sysProjectSDCD_IN(17);
            //-----------------Check Disk Free Size------------------
            ret = sysProjectSDCD_IN(18);
            if(sdcErrorResultFilter(ret) < 0)
            {
                // print the SD Error OSD msg to remind the user this SD card had a big problem
                DEBUG_SYS("Error: Calculate available fail.\n");
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                OSFlagPost(gSDCExtFlagGrp, 0x1, OS_FLAG_SET, &err);
                return -1;
            }
            //-------Lucian: Copy APP from Nand to SD for Special purpose------
            sysProjectSDCD_IN(19);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
			sysSetEvt(SYS_EVT_SCANFILE, 0);
#endif
            break;

        case SYS_I_STORAGE_BACKUP:
            dcfBackupUninit(STORAGE_MEMORY_SD_MMC);
            ret = dcfBackupInit(STORAGE_MEMORY_SD_MMC);
            DEBUG_SYS("ret = %d\n", ret);
            switch(ret)
            {
                case 1:
                    ret = dcfBackupDriveInfo(&Backup_diskInfo, 1);
                    break;
                case -2:	// SDC is not inserted.
                    DEBUG_SYS("Error: No SD Card.\n");
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                default:
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                    return -1;
                    //break;
            }

            if(sdcErrorResultFilter(ret) < 0)
            {
                // print the SD Error OSD msg to remind the user this SD card had a big problem
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
                OSFlagPost(gSDCExtFlagGrp, 0x1, OS_FLAG_SET, &err);
                return -1;
            }
            break;
        default:
            break;
    }

#if !defined(NEW_UI_ARCHITECTURE)
    uiClearOSDBuf(2);
#endif
    //---Hint to User to wait---//
    sysProjectSDCD_IN(20);

    //----open FIQ interrupt---//
    /*-----------SDCD_IN complete and busy LED off,Flag and Semephore setup-------------*/

    system_busy_flag = 0;

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
    sysProjectSDCD_IN(21);

    DEBUG_SYS("TRACE SYS: SDC MEDIA_READY\n");
    sdcChangeMediaStat(SDC_USB_MEDIA_READY);
    // To remind system's storage is ready to record films
    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_ON)
    {
        switch(storageIdx)
        {
            case SYS_I_STORAGE_MAIN:
                sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_READY);
                break;
            case SYS_I_STORAGE_BACKUP:
                sysSetStorageStatus(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_READY);
                break;
            default:
                break;
        }
    }
    return 1;
}


/*

Routine Description:

    sysSDCD_OFF.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSDCD_OFF(s32 dummy)
{
    u8 err;
    int devIdx, storageIdx;
    //
    devIdx = DCF_GetDeviceIndex("sdmmc");
    storageIdx = sysGetStorageIndex(SYS_V_STORAGE_SDC);
    //
    switch(storageIdx)
    {
        case SYS_I_STORAGE_MAIN:
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_CLR, &err);
            //Close FIQ?
            system_busy_flag = 1;
            got_disk_info = 0;
            sysStorageOnlineStat[devIdx] = 0;  // Add this will Let FAT to re-read the BPB section
#if NIC_SUPPORT
            Fileplaying = 0;	// clean playback playing video flag
#endif
            dcfUninit();
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)            
            sdcUnInit();	// del all related semaphore about SD control
#endif
			osdDrawPreviewIcon();
            MemoryFullFlag = FALSE;
            system_busy_flag = 0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
            break;

        case SYS_I_STORAGE_BACKUP:
            sysStorageOnlineStat[devIdx] = 0;  // Add this will Let FAT to re-read the BPB section
            sdcUnInit();	// del all related semaphore about SD control
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD1_RDY, OS_FLAG_SET, &err);
            break;

        default:
            break;
    }
    DEBUG_SYS("do sysSDCD_OFF: %#x\n", storageIdx);

    return 1;
}

#if USB_HOST_MASS_SUPPORT
s32 sysUSBCD_IN(s32 mode)
{
    int devIdx, storageIdx;
    int ret;
    u8 err;
    void* msg;

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY | FLAGSYS_RDYSTAT_CARD2_ERR, OS_FLAG_CLR, &err);
    // flag to announce to usb is doing SD init
    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);
    // flag to pend to wait for usb check by SD functions
    OSFlagPend(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_WAIT_SET_ANY, TIMEOUT_SDC, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SYS("Error SYS: gSdUsbProcFlagGrp is %d\n", err);
    }
    // old stuff, useless
    system_busy_flag = 1;
    gInsertNAND = 0;
    //
    devIdx = DCF_GetDeviceIndex("usbfs");
    storageIdx = sysGetStorageIndex(SYS_V_STORAGE_USBMASS);

    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_OFF)
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
        return -1;
    }

    DEBUG_SYS("do sysUSBCD_IN: %#x.\n", storageIdx);
    sysStorageOnlineStat[devIdx] = 0;  // Add this will Let FAT to re-read the BPB section
    switch(storageIdx)
    {
        case SYS_I_STORAGE_MAIN:
            got_disk_info = 0;
            dcfFileTypeCount_Clean();
            OSSemSet(dcfReadySemEvt, 1, &err);
            //---------File system initialize-----------//
            dcfUninit();
            ret = dcfInit(STORAGE_MEMORY_USB_HOST);
            DEBUG_SYS("ret = %#x\n", ret);
            // distinguish the result value whether is from low layer of SDC or not.

            //
            switch(ret)
            {
                // USB init & File system init success
                case 1:
                    break;

                default:
			if(usb_init_flag)
		       {
                        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
                        return -1;
                    }
                    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_OFF)
                    {
                        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
                        return -1;
                    }
                    DEBUG_SYS("Error: FS Operation error.\n");
                    if(Main_Init_Ready == 0)
                        Main_Init_Ready = 1;	// To release the lock of UI page. Touch, Key needs the unlock.
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_SET, &err);		// XOR Format procedure
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_ERR, OS_FLAG_SET, &err);
                    sysDeadLockMonitor_OFF();
                    // release SD Init flag for USB Plug-in?
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
                    speciall_MboxEvt->OSEventPtr = (void *)0;
                    uiSetGoToFormat();
                    msg = OSMboxPend(speciall_MboxEvt, OS_IPC_WAIT_FOREVER, &err);
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_CLR, &err);		// XOR Format procedure
                    sysDeadLockMonitor_ON();
                    if(strcmp((const char*)msg, "PASS"))
                    {
                        system_busy_flag = 0;
                        DEBUG_SYS("TRACE SYS: Terminate Formatting Harddrive!\n");
                        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
                        return -1;
                    }
                    else
                    {
                        // Continue to implement the Harddrive init
                        OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);
                    }
                    break;
            }
            //--------------------------In system programming --------------------------
            sysProjectSDCD_IN(17);
            //-----------------Check Disk Free Size------------------
#if 0
            sysProjectSDCD_IN(18);
#else
            //status = sysSetEvt(SYS_EVT_GETDISKFREE, 0);
            //sysbackLowSetEvt(SYSBACKLOW_EVT_GETDISKFREE, 0, 0, 0, 0);
            usb_dev_mass_scan_free_extend();
            global_diskInfo.total_clusters = 12345678;
            global_diskInfo.avail_clusters = 12345678;
            global_diskInfo.bytes_per_sector = 512;
            global_diskInfo.sectors_per_cluster = 64;
#endif
            //-------Lucian: Copy APP from Nand to SD for Special purpose------//
            sysProjectSDCD_IN(19);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
			sysSetEvt(SYS_EVT_SCANFILE, 0);
#endif
            break;

        case SYS_I_STORAGE_BACKUP:
            break;

        default:
            break;
    }

#if !defined(NEW_UI_ARCHITECTURE)
    uiClearOSDBuf(2);
#endif

    system_busy_flag = 0;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
    DEBUG_SYS("TRACE SYS: USB_MEDIA_READY. \n");
    // To remind system's storage is ready to record films
    if(sysGetStorageInserted(storageIdx) == SYS_V_STORAGE_ON)
    {
        switch(storageIdx)
        {
            case SYS_I_STORAGE_MAIN:
                sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_READY);
                break;
            case SYS_I_STORAGE_BACKUP:
                sysSetStorageStatus(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_READY);
                break;
            default:
                break;
        }
    }

    return 1;
}

s32 sysUSBCD_OFF(s32 dummy)
{
    int devIdx, storageIdx;
    u8 err;
    //
    devIdx = DCF_GetDeviceIndex("usbfs");
    storageIdx = sysGetStorageIndex(SYS_V_STORAGE_USBMASS);
    //
    switch(storageIdx)
    {
        case SYS_I_STORAGE_MAIN:
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_CLR, &err);
            system_busy_flag = 1;
            got_disk_info = 0;
            sysStorageOnlineStat[devIdx] = 0;
#if NIC_SUPPORT
            Fileplaying = 0;	// clean playback playing video flag
#endif
            MemoryFullFlag = FALSE;	// I don't know the assignment whether is necessary or not
            system_busy_flag = 0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);

			dcfUninit();
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
#if AUTO_STORAGE_CHANGE	// Switch the
            sysKeepSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_SDC, SYS_I_STORAGE_BACKUP);
            if(sysCheckSDCD() == SDC_CD_IN)
                sysSetEvt(SYS_EVT_SDCD_IN, 0);
#endif
#endif
            break;

        case SYS_I_STORAGE_BACKUP:
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD2_RDY, OS_FLAG_SET, &err);
            break;

        default:
            break;
    }
    DEBUG_SYS("do sysUSBCD_OFF: %#x\n", storageIdx);

#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
	gUSBDevOn = 0;
#endif 

	//if (ehci_hcd_init() != 0)
    //  return -1;
    return 0;
}

#endif

int sysGetFWUpgradeStatus(void)
{
	return gSystemCodeUpgrade;
}

void sysSetFWUpgradeStatus(int Status)
{
	if(Status)
		gSystemCodeUpgrade = 1;
	else
		gSystemCodeUpgrade = 0;
}

s32 sysDevInsertedUpgradeEvt(s32 dummy)
{
	u32 temp;
	int ret;

#if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
    temp = MotionDetect_en;	//暫時關掉motion detection.
    MotionDetect_en=0;
#endif
    if (sysCaptureVideoStart)
        sysCaptureVideoStop = 1;

	// Block UI Sent key
    uiFlowEnterMenuMode(SETUP_MODE);
    IduVideo_ClearPKBuf(0);
    // Show Upgrade OSD
    osdDrawISPStatus(3);
    
	ret = ispFirmwareUpdateFlow(1);
	
    osdDrawISPStatus(ret);
    if (ret != 0)	 // Only need to re-mount SD while return 1 -1 -2
    {
        OSTimeDly(20);
        uiClearOSDBuf(2);
    }

    while(1);

    //return 1;
}

s32 sysbackCheckNextEvtIsEmpty(void)
{
    /* check if event queue is empty */
    if (sysbackEvt.idxGet == sysbackEvt.idxSet)
    {
        /* event queue is empty */
        return TRUE;
    }
    return FALSE;
}

s32 sysbackEXIFWrite(s32 dummy)
{
    u8 err;

    /*aviod warning message*/
    if (dummy)
    {}
    OSSemPend(sd_backSemEvt, OS_IPC_WAIT_FOREVER, &err);
    //DEBUG_SYS("Enter Exif Write atomic region\n");

    if (exifWriteFile(Global_thumbnailImageSize, Global_primaryImageSize, 0)==0)
    {
        system_busy_flag=0;
        DEBUG_SYS("exifWriteFile error\n");
    }
    else
    {
        system_busy_flag=0;
    }
    sysProjectExifWrite(1);
    if ((sysCameraMode == SYS_CAMERA_MODE_PREVIEW)&&(uiMenuEnable == 0))
    {
        if (sysTVOutOnFlag)
            osdDrawPreviewIcon();
        else
            osdDrawPreviewIcon();
    }
    //DEBUG_SYS("Exit Exif Write atomic region\n");
    if (gInsertNAND==1)
        userClicksnapshot=1;        // Means user clicked snapshot --> need to write back cache
    OSSemPost(sd_backSemEvt);
    sysProjectExifWrite(2);
    return 1;
}

void sysbackTask(void* pData)
{
    u8 err;
    s8 cause;
    s32 param;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    while (1)
    {
        OS_ENTER_CRITICAL();
        if (sysbackGetEvt(&cause, &param))
        {
            OS_EXIT_CRITICAL();
#if SYSBACK_DEBUG_ENA
            gpioSetLevel(0, 12, 1);
#endif
            (*sysbackEvtFunc[cause])(param);
#if SYSBACK_DEBUG_ENA
            gpioSetLevel(0, 12, 0);
#endif
            //Further use --> shoude define a function table civic
        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysbackSemEvt, OS_IPC_WAIT_FOREVER, &err);
            //DEBUG_SYS("sysbackSemEvt Calling\n");
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysbackSemEvt is %d.\n", err);
                continue ;
            }
        }

    }
}


void sysback_Low_Task(void* pData)
{
    u8 err;
    s8 cause;
    s32 param1,param2,param3,param4;
    int funerr;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    while (1)
    {
        OS_ENTER_CRITICAL();
        if (sysbackLowGetEvt(&cause, &param1,&param2,&param3,&param4))
        {
            OS_EXIT_CRITICAL();
            //DEBUG_YELLOW("sysbl w cause: %d\n", cause);
            funerr=(*sysbackLowEvtFunc[cause])(param1,param2,param3,param4);
            //DEBUG_GREEN("sysbl p cause: %d\n", cause);
            if(funerr<0)
                DEBUG_SYS("Error: sysbackLowTask's Function is %d.\n", funerr);
            OSSemPost(sysbackLowCountEvt);
            //DEBUG_SYS("%d}\n",sysbackLowCountEvt->OSEventCnt);

        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysbackLowSemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysbackLowSemEvt is %d.\n", err);
                continue ;
            }
        }

    }

}

void sysback_RF_Task(void* pData)
{
    u8 err;
    s8 cause;
    s32 param;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    while (1)
    {
        OS_ENTER_CRITICAL();
        if (sysback_RF_GetEvt(&cause, &param))
        {
            OS_EXIT_CRITICAL();
            (*sysback_RF_EvtFunc[cause])(param);
        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysback_RF_SemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysbackRFSemEvt is %d.\n", err);
                continue ;
            }
        }

    }
}



#if (NIC_SUPPORT == 1)
void sysback_Net_Task(void* pData)
{
    u8 err;
    s8 cause;
    u32 param1, param2;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    while (1)
    {
        OS_ENTER_CRITICAL();
        if (sysback_Net_GetEvt(&cause, &param1, &param2))
        {
            OS_EXIT_CRITICAL();
        #if (PWIFI_SUPPORT || ICOMMWIFI_SUPPORT)

        #else
            if(cause == 0)//Push message
            {
                if(qetIP == 0)
                {
                    DEBUG_SYS("Error: network not ready\n");
                    continue;
                }                
            }
        #endif    
            (*sysback_Net_EvtFunc[cause])(param1, param2);
        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysbackNetSemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysbackNetSemEvt is %d.\n", err);
                continue;
            }
        }

    }
}
#endif

//// Civic 070822 E
/*

Routine Description:

    Set sys event.

Arguments:

    cause - Cause of the event to set.
    param - Parameter of the event.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSetEvt(s8 cause, s32 param)
{
    /* check if cause is valid */
    if (cause >= SYS_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    if( ((sysEvt.idxSet+SYS_EVT_MAX-sysEvt.idxGet)%SYS_EVT_MAX) == (SYS_EVT_MAX-1))
    {
        /* Overrun */
        DEBUG_SYS("SYS task Overrun %d!\n",cause);
        return 0;
    }

    /* set the cause */
    sysEvt.param[sysEvt.idxSet] = param;
    sysEvt.cause[sysEvt.idxSet++] = cause;

    if (sysEvt.idxSet == SYS_EVT_MAX)
    {
        /* wrap around the index */
        sysEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (sysEvt.idxSet == sysEvt.idxGet)
    {
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    OSSemPost(sysSemEvt);

    return 1;
}

void InitsysEvt(void)
{
    u8 err;
    //=================//

    sysTaskStopflag=1;
    OSTimeDly(4);
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
#if 1
    if((guiSDReadDMAId != 0x55))
    {
        marsDMATerminate(guiSDReadDMAId);
        marsDMAClose(guiSDReadDMAId);
        guiSDReadDMAId = 0x55;
    }

    if((guiSDWriteDMAId != 0x55))
    {
        marsDMATerminate(guiSDWriteDMAId);
        marsDMAClose(guiSDWriteDMAId);
        guiSDWriteDMAId = 0x55;
    }
#endif
    //del sys TASK
    OSTaskSuspend(SYS_TASK_PRIORITY);
    OSTaskDel(SYS_TASK_PRIORITY);
    sysTaskStopflag=0;
    OSSemPost(uiOSDSemEvt);
    memset((void *)&sysEvt, 0, sizeof(SYS_EVT));

    /* Create the task */
    //DEBUG_SYS("Trace: SYS` task creating\n");
    OSTaskCreate(SYS_TASK, SYS_TASK_PARAMETER, SYS_TASK_STACK, SYS_TASK_PRIORITY);
    /* Accept sysSemEvt*/
    while (sysSemEvt->OSEventCnt > 0)
    {
        OSSemAccept(sysSemEvt);
    }
    // Set event empty
    sysEvt.idxSet=sysEvt.idxGet=0;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_SYS_INIT_RESET, OS_FLAG_SET, &err);

}

void InitsysbackEvt(void)
{
    sysbackEvt.idxSet=sysbackEvt.idxGet=0;
    OSTaskSuspend(SYSBACK_TASK_PRIORITY);
    OSTaskDel(SYSBACK_TASK_PRIORITY);
    memset((void *)&sysbackEvt, 0, sizeof(SYS_EVT));
    /* Create the task */

    OSTaskCreate(SYS_BACK_TASK, SYSBACK_TASK_PARAMETER, SYSBACK_TASK_STACK, SYSBACK_TASK_PRIORITY);
    /* Accept sysSemEvt*/
    while (sysbackSemEvt->OSEventCnt > 0)
    {
        OSSemAccept(sysbackSemEvt);
    }
    // Set event empty
    sysbackEvt.idxSet=sysbackEvt.idxGet=0;


}

void InitsysbackLowEvt(void)
{
    u8      err;
    OSTaskDel(SYSBACK_LOW_TASK_PRIORITY);
    memset((void *)&sysbackLowEvt, 0, sizeof(SYSBACKLOW_EVT));
    /* Create the task */
    OSSemSet(sysbackLowCountEvt, SYSBACKLOW_EVT_MAX-1 , &err);
    OSTaskCreate(SYS_BACK_LOW_TASK, SYSBACK_LOW_TASK_PARAMETER, SYSBACK_LOW_TASK_STACK, SYSBACK_LOW_TASK_PRIORITY);
    /* Accept sysSemEvt*/
    while (sysbackLowSemEvt->OSEventCnt > 0)
    {
        OSSemAccept(sysbackLowSemEvt);
    }
    // Set event empty
    sysbackLowEvt.idxSet=sysbackLowEvt.idxGet=0;

}
//Civic 070822 S
/*

Routine Description:

    Set sys event.

Arguments:

    cause - Cause of the event to set.
    param - Parameter of the event.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysbackSetEvt(s8 cause, s32 param)
{
    /* check if cause is valid */
    if (cause >= SYSBACK_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysbackEvt.param[sysbackEvt.idxSet] = param;
    sysbackEvt.cause[sysbackEvt.idxSet++] = cause;

    if (sysbackEvt.idxSet == SYS_EVT_MAX)
    {
        /* wrap around the index */
        sysbackEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (sysbackEvt.idxSet == sysbackEvt.idxGet)
    {
        /* event queue is full */
        DEBUG_SYS("SYSBACK QUE is Full! %d \n",cause);
        return 0;
    }
    
    /* signal event semaphore */
    OSSemPost(sysbackSemEvt);

    return 1;
}

s32 sysback_RF_SetEvt(s8 cause, s32 param)
{
    /* check if cause is valid */
    if (cause >= SYSBACK_RF_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysback_RF_Evt.param[sysback_RF_Evt.idxSet] = param;
    sysback_RF_Evt.cause[sysback_RF_Evt.idxSet++] = cause;

    if (sysback_RF_Evt.idxSet == SYSBACK_RFEVT_MAX)
    {
        /* wrap around the index */
        sysback_RF_Evt.idxSet = 0;
    }

    /* check if event queue is full */
    if (sysback_RF_Evt.idxSet == sysback_RF_Evt.idxGet)
    {
        /* event queue is full */
        DEBUG_SYS("SYSBACK_RF QUE is Full!\n");
        return 0;
    }

    /* signal event semaphore */
    OSSemPost(sysback_RF_SemEvt);

    return 1;
}


s32 sysbackLowSetEvt(s8 cause, s32 param1,s32 param2,s32 param3,s32 param4)
{
    u8 err;
    u16 WaitTime;
    /* check if cause is valid */
    if (cause >= SYSBACKLOW_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }
    if (cause == SYSBACKLOW_EVT_SYN_RF)
        WaitTime = 1;
    else
        WaitTime = OS_IPC_WAIT_FOREVER;
    OSSemPend(sysbackLowCountEvt, WaitTime, &err);
    if (err == OS_NO_ERR)
    {
        /* set the cause */
        sysbackLowEvt.param1[sysbackLowEvt.idxSet] = param1;
        sysbackLowEvt.param2[sysbackLowEvt.idxSet] = param2;
        sysbackLowEvt.param3[sysbackLowEvt.idxSet] = param3;
        sysbackLowEvt.param4[sysbackLowEvt.idxSet] = param4;
        sysbackLowEvt.cause[sysbackLowEvt.idxSet++] = cause;

        if (sysbackLowEvt.idxSet == SYSBACKLOW_EVT_MAX)
        {
            /* wrap around the index */
            sysbackLowEvt.idxSet = 0;
        }

        /* check if event queue is full */
        if (sysbackLowEvt.idxSet == sysbackLowEvt.idxGet)
        {
            /* event queue is full */
            return 0;
        }

        /* signal event semaphore */
        OSSemPost(sysbackLowSemEvt);
    }
    return 1;
}

#if (NIC_SUPPORT == 1)
s32 sysback_Net_SetEvt(s8 cause, u32 param1, u32 param2)
{
    /* check if cause is valid */
    if (cause >= SYSBACK_NTE_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysback_NET_Evt.param1[sysback_NET_Evt.idxSet] = param1;
    sysback_NET_Evt.param2[sysback_NET_Evt.idxSet] = param2;
    sysback_NET_Evt.cause[sysback_NET_Evt.idxSet++] = cause;

    if (sysback_NET_Evt.idxSet == SYSBACK_NET_EVT_MAX)
    {
        /* wrap around the index */
        sysback_NET_Evt.idxSet = 0;
    }

    /* check if event queue is full */
    if (sysback_NET_Evt.idxSet == sysback_NET_Evt.idxGet)
    {
        /* event queue is full */
        DEBUG_SYS("SYSBACK_NET QUE is Full!\n");
        return 0;
    }

    /* signal event semaphore */
    OSSemPost(sysbackNetSemEvt);

    return 1;
}
#endif
//civic 070822 E

/*

Routine Description:

    Get sys event.

Arguments:

    pCause - Cause of the event got.
    pParam - Paramter of the event got.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysGetEvt(s8* pCause, s32* pParam)
{
    /* check if event queue is empty */
    if (sysEvt.idxGet == sysEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    //DEBUG_SYS("-----sysGetEvt:%d,%d------\n",sysEvt.idxGet,sysEvt.idxSet);

    /* get the cause */
    *pParam = sysEvt.param[sysEvt.idxGet];
    *pCause = sysEvt.cause[sysEvt.idxGet++];

    if (sysEvt.idxGet == SYS_EVT_MAX)
    {
        /* wrap around the index */
        sysEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYS_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }
    return 1;
}

s32 sysbackGetEvt(s8* pCause, s32* pParam)
{
    /* check if event queue is empty */
    if (sysbackEvt.idxGet == sysbackEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pParam = sysbackEvt.param[sysbackEvt.idxGet];
    *pCause = sysbackEvt.cause[sysbackEvt.idxGet++];

    if (sysbackEvt.idxGet == SYS_EVT_MAX)
    {
        /* wrap around the index */
        sysbackEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }
    return 1;
}

s32 sysback_RF_GetEvt(s8* pCause, s32* pParam)
{
    /* check if event queue is empty */
    if (sysback_RF_Evt.idxGet == sysback_RF_Evt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pParam = sysback_RF_Evt.param[sysback_RF_Evt.idxGet];
    *pCause = sysback_RF_Evt.cause[sysback_RF_Evt.idxGet++];

    if (sysback_RF_Evt.idxGet == SYSBACK_RFEVT_MAX)
    {
        /* wrap around the index */
        sysback_RF_Evt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_RF_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    return 1;
}


#if (NIC_SUPPORT == 1)
s32 sysback_Net_GetEvt(s8* pCause, u32* pParam1, u32* pParam2)
{
    /* check if event queue is empty */
    if (sysback_NET_Evt.idxGet == sysback_NET_Evt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pParam1 = sysback_NET_Evt.param1[sysback_NET_Evt.idxGet];
    *pParam2 = sysback_NET_Evt.param2[sysback_NET_Evt.idxGet];
    *pCause = sysback_NET_Evt.cause[sysback_NET_Evt.idxGet++];

    if (sysback_NET_Evt.idxGet == SYSBACK_NET_EVT_MAX)
    {
        /* wrap around the index */
        sysback_NET_Evt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_NTE_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }

    return 1;
}
#endif
s32 sysbackLowGetEvt(s8* pCause, s32* pParam1,s32* pParam2,s32* pParam3,s32* pParam4)
{
    /* check if event queue is empty */
    if (sysbackLowEvt.idxGet == sysbackLowEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pParam1 = sysbackLowEvt.param1[sysbackLowEvt.idxGet];
    *pParam2 = sysbackLowEvt.param2[sysbackLowEvt.idxGet];
    *pParam3 = sysbackLowEvt.param3[sysbackLowEvt.idxGet];
    *pParam4 = sysbackLowEvt.param4[sysbackLowEvt.idxGet];
    *pCause  = sysbackLowEvt.cause[sysbackLowEvt.idxGet++];

    if (sysbackLowEvt.idxGet == SYSBACKLOW_EVT_MAX)
    {
        /* wrap around the index */
        sysbackLowEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACKLOW_EVT_UNDEF)
    {
        /* cause out of range */
        return 0;
    }
    return 1;
}


s32 sysCheckNextEvtIsPrevOrNext(void)
{
    /* check if event queue is empty */
    if (sysEvt.idxGet == sysEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    if (
        (sysEvt.cause[sysEvt.idxGet]== SYS_EVT_PLAYBACK_MOVE_FORWARD) ||
        (sysEvt.cause[sysEvt.idxGet]== SYS_EVT_PLAYBACK_MOVE_BACKWARD) ||
        (sysEvt.cause[sysEvt.idxGet]== SYS_EVT_ReadFile)
    )
        return 1;

    return 0;
}

s32 sysCheckNextEvtIsEmpty(void)
{
    /* check if event queue is empty */
    if (sysEvt.idxGet == sysEvt.idxSet)
    {
        /* event queue is empty */
        return TRUE;
    }
    return FALSE;
}



#if (MULTI_CHANNEL_VIDEO_REC)

/*

Routine Description:

    sysVideoCaptureStop, for video record task self stop capture video.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysVideoCaptureStop(s32 VideoChannelID)
{
    return MultiChannelSysCaptureVideoStopOneCh(VideoChannelID);
}

#endif

s32 sysDeadLockMonitor(void)
{
    if(sysDeadLockCheck_ena)
    {
        if(sysLifeTime - sysLifeTime_prev > SYSTEM_DEADLOCKTIME_MAX)
        {
            DEBUG_SYS("---->Warning !!MainTask Dead Lock,Force to Reboot!\n");
            sysForceWDTtoReboot();
        }
    }
    else
        sysLifeTime_prev=sysLifeTime;

    if(sysLifeTime >SYSTEM_LIFETIME_MAX)
    {
        DEBUG_SYS("---->Warning !!Exceed Max life time, Force to Reboot!\n");
        sysForceWDTtoReboot();
    }
    return 1;
}

s32 sysDeadLockMonitor_ON(void)
{
    sysDeadLockCheck_ena=1;
    return 1;
}

s32 sysDeadLockMonitor_OFF(void)
{
    sysDeadLockCheck_ena=0;
    return 1;
}

s32 sysDeadLockMonitor_Reset(void)
{
    sysLifeTime_prev=sysLifeTime;
    return 1;
}

s32 sysForceWDTtoReboot(void)
{
#if CDVR_SYSTEM_LOG_SUPPORT
	sysWriteLog(SYSTEM_REBOOT, 0, 0);
	dcfLogFileSaveRefresh();
#endif
	// Lock spi pipe
	spiReleaseFlashStatus();
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
    
    
    sysForceWdt2Reboot=1;
    return 1;
}
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
     (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
void sysFakeShutodown()
{
    u8 i;
    u32 sys_ctl0;
//   uiCaptureVideoStop();
    iconflag[UI_MENU_SETIDX_UPGRADE_FW] = 1; /* Reset default to "No" */

    for ( i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
    {
        if ((i == SYSTIMER_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY))
            // (i == SYS_TASK_PRIORITY))

        {
            continue;
        }
        DEBUG_SYS("UI OSTaskDel %d!\n",i);
        OSTaskSuspend(i);
        OSTaskDel(i);
    }

    DEBUG_SYS("g_TASK_PRIORITY!!\n");
#if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)
    sys_ctl0 = SYS_CTL0;
    DEBUG_SYS("sys_ctl0=%x!!\n",sys_ctl0);
    sys_ctl0 &= ~SYS_CTL0_H264_CKEN  &
                ~SYS_CTL0_JPEG_CKEN  &
                ~SYS_CTL0_RF1012_CKEN &
                ~SYS_CTL0_SIU_CKEN &
                ~SYS_CTL0_IPU_CKEN &
                ~SYS_CTL0_ISU_CKEN &
                ~SYS_CTL0_IDU_CKEN &
                ~SYS_CTL0_SD_CKEN &
                ~SYS_CTL0_MD_CKEN &
                ~SYS_CTL0_SCUP_CKEN &
                ~SYS_CTL0_CIU_CKEN &
                ~SYS_CTL0_CIU2_CKEN&
                ~SYS_CTL0_IIS_CKEN&
                ~SYS_CTL0_USB_CKEN&
                //    ~SYS_CTL0_UART_CKEN&
                ~SYS_CTL0_SD_CKEN&
                ~SYS_CTL0_IR_CKEN&
                ~SYS_CTL0_WDT_CKEN&
                ~SYS_CTL0_GPIO3_CKEN&
                ~SYS_CTL0_WDT_CKEN&
                ~SYS_CTL0_GPIU_CKEN&
                ~SYS_CTL0_MCP_CKEN&
                ~SYS_CTL0_RF1013_CKEN;
    SYS_CTL0 = sys_ctl0;
// SYS_CTL0 = SYS_CTL0_SDRAM_CKEN|SYS_CTL0_SRAM_CKEN|SYS_CTL0_TIMER0_CKEN|SYS_CTL0_ADC_CKEN|SYS_CTL0_GPIO0_CKEN|SYS_CTL0_UART_CKEN|SYS_CTL0_RTC_CKEN;
#endif
    sys_ctl0 = SYS_CTL0;
//DEBUG_SYS("sys_ctl0=%x!!\n",sys_ctl0);
    DEBUG_SYS("g_End!\n");
    SYS_CTL0_EXT =0;


}
#endif

/*

Routine Description:

    sysPowerOn.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPowerOn(s32 dummy)
{
#if (HW_BOARD_OPTION == A1025A_EVB_axviwe)

#if EXTIO_LED_ENA				
    // Close light of LED.
    i2c_PCF8574_SwitchTx1LED(0);
    i2c_PCF8574_SwitchTx2LED(0);
    i2c_PCF8574_SwitchTx3LED(0);
    i2c_PCF8574_SwitchTx4LED(0);
#endif

#elif(HW_BOARD_OPTION == MR8202A_RX_MAYON)
    
#if EXTIO_LED_ENA				
    // Close light of LED.
    gpioTimerCtrLed(0, LED_OFF);
    gpioTimerCtrLed(1, LED_OFF);
    gpioTimerCtrLed(2, LED_OFF);
    gpioTimerCtrLed(3, LED_OFF);
    gpioTimerCtrNetLed(LED_OFF);
#endif

#endif
    return 1;
}


/*

Routine Description:

    sysPowerOff.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPowerOff(s32 dummy)
{
#if (HW_BOARD_OPTION == MR8202A_RX_MAYON)

#if EXTIO_LED_ENA
    u32 Cnt, i;
    u8 Level;
    DEBUG_SYS("Power off evt is going!\n");
    for(Cnt = 0, i = 0; i < 10; i++)
    {
        gpioGetLevel(0, 2, &Level);
        if(Level > 0)
            return 0;
        OSTimeDly(10);
    }

    i2c_PCF8575_SwitchAllLED(1);
    i2c_PCF8575_SetBlocker(1);

#if (A1025_GATE_WAY_SERIES)
    uiFlowSetDefault();
#else
    uiSetDefaultSetting();
#endif
    Save_UI_Setting();

    for(;;)
    {
        gpioGetLevel(0, 2, &Level);
        if(Level > 0)
            break;
        OSTimeDly(10);
    }
    sysForceWDTtoReboot();
    
#endif

#elif (HW_BOARD_OPTION != A1025A_EVB_axviwe)
    static u8 power_off = 1;
    u32     cnt = 2;
    u8  buf;

    /*avoid warning message*/
    if (cnt || buf)
    {}

    DEBUG_SYS("Power off !\n");
    if (uiCheckVideoRec()>0)
    {
        //Video Clip
        pwroff = 1;
        return 0;
    }
    sysProjectPowerOff(1);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    if (power_off==1 && gInsertNAND==1 )
    {
        power_off=0;
        if (smcWriteBackFAT()==0)
        {
            DEBUG_SYS("Write FAT cache Fail \n");
        }
        power_off=2;
    }
    else if (power_off==1 && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        power_off=0;
        smcStart();
        if (smcWriteBackFAT()==0)
        {
            DEBUG_SYS("Write FAT cache Fail\n");
        }
        power_off=2;
    }
    DEBUG_SYS("Write FAT cache Finish \n");

#elif ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    power_off=2;
#endif

    if (power_off==2)
    {
        sysProjectPowerOff(2);
        sysProjectPowerOff(3);
    }
    return 1;
#else

#if EXTIO_LED_ENA
    u32 Cnt, i;
    u8 Level;
    DEBUG_SYS("Power off evt is going!\n");
    for(Cnt = 0, i = 0; i < 10; i++)
    {
        gpioGetLevel(0, 2, &Level);
        if(Level > 0)
            return 0;
        OSTimeDly(10);
    }

    i2c_PCF8574_SwitchTx1LED(1);
    i2c_PCF8574_SwitchTx2LED(1);
    i2c_PCF8574_SwitchTx3LED(1);
    i2c_PCF8574_SwitchTx4LED(1);

#if (A1025_GATE_WAY_SERIES)
    uiFlowSetDefault();
#else
    uiSetDefaultSetting();
#endif
    Save_UI_Setting();

    for(;;)
    {
        gpioGetLevel(0, 2, &Level);
        if(Level > 0)
            break;
        OSTimeDly(10);
    }
    sysForceWDTtoReboot();
#endif

#endif    
}


/*

Routine Description:

    sysMacro.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysMacro(s32 dummy)
{}

/*

Routine Description:

    sysLcdRot.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysLcdRot(s32 dummy)
{}



s32 sysGetDiskFree(s32 dummy)
{
    FS_DISKFREE_T* diskInfo = &global_diskInfo;
    int ret;
    system_busy_flag = 1;

    ret = dcfDriveInfo(diskInfo, 1);

    //if (usb_msc_mode != 1)    /*When USB Plug-In, the system_busy_flag always 1*/
    system_busy_flag = 0;
    got_disk_info = 1;

    return ret;
}

s32 sysBackLowGetDiskFree(s32 dummy, s32 dummy2, s32 dummy3, s32 dummy4)
{
    return sysGetDiskFree(dummy);
}

/*

Routine Description:

    sysWhiteLight.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysWhiteLight(s32 dummy)
{
    sysProjectWhiteLight(1, dummy);
    return 1;
}

/*

Routine Description:

    sysFlashLight.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysFlashLight(s32 dummy)
{

    return 1;
}



/*

Routine Description:

    sysTVOUT_ON.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/



/*

Routine Description:

    sys voice rec.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysVOICE_REC(s32 dummy)
{

#if DINAMICALLY_POWER_MANAGEMENT

    u32     sys_ctl0_status;
    // disable unused module for reduce power consumption
    sys_ctl0_status     = SYS_CTL0;

#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif
    sys_ctl0_status    |= SYS_CTL0_SIU_CKEN |
                          SYS_CTL0_IPU_CKEN |
                          SYS_CTL0_SRAM_CKEN |
#if IDU_TV_DISABLE

#else
                          SYS_CTL0_IDU_CKEN |
#endif
                          SYS_CTL0_ISU_CKEN |
                          SYS_CTL0_SCUP_CKEN |
                          SYS_CTL0_SER_MCKEN |
                          SYS_CTL0_IIS_CKEN;

    sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif
    DEBUG_SYS("Sys task -- Voice Rec\n");

    // stop preview process
    isuStop();
    ipuStop();
    siuStop();

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
    Init_IIS_WM8940_rec();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#endif


#if 1
    adcInit(1);
#endif

    iisCaptureVideoInit();
    RTCseconds=0;

    timerInterruptEnable(1,1);

    wavRecVoice();


    timerInterruptEnable(1,0);


#if(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    Close_IIS_ALC5621();
#endif
    if (gInsertNAND==1)
        userClickvoiceRec=1;
    return 1;
}

/*

Routine Description:

    Self-timer.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSelfTimer(void)  /*CY 0907*/
{

    DEBUG_SYS("Trace: Wait self-timer %d sec...\n", SelfTimer);
    sysProjectSelfTimer(SYS_SELF_TIMER_DRAW_ICON);
    return 1;
}


s32 sysCaptureImage_OnRFRx(s32 dummy)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize = 0;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    int srcW,srcH;

    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */

    //========================================//
    if(sysCameraMode != SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        DEBUG_SYS("Waring!! not in the Single mode...\n");
        return 0;
    }

    RTC_Get_Time(&curDateTime);
    exifSetDateTime(&curDateTime);

    srcW=gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth;
    srcH=gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight;
    DEBUG_SYS("-->Capture RX image start:(%d,%d)\n",srcW,srcH);

    exifSetImageResolution(srcW, srcH); //暫時用
    SYS_CTL0 |=SYS_CTL0_JPEG_CKEN;
    jpegCapturePreviewImg(
        (u8*)exifPrimaryImage.bitStream,
        MainVideodisplaybuf[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) % DISPLAY_BUF_NUM],
        JPEG_OPMODE_FRAME,
        srcW,
        srcH
    );
    JpegImagePixelCount=srcW*srcH;//GetJpegImagePixelCount();

    primaryImageSize=WaitJpegEncComplete();
    SYS_CTL0 &= (~SYS_CTL0_JPEG_CKEN);
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);

    diskInfo    =   &global_diskInfo;
    bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO)+ bytes_per_cluster-1) / bytes_per_cluster;

    if(diskInfo->avail_clusters <= (used_cluster+2)) // protect write miss
    {
        DEBUG_SYS("Memory full \n");
        sysCaptureImageStart=0;
        return 0;
    }

    exifWriteFile(thumbnailImageSize, primaryImageSize, sysRFRxInMainCHsel);

    return 1;
}








s32 sys_USBHOST_STORAGE_IN(s32 dummy)
{


    return 1;
}






/*

Routine Description:

    Playback zoom.

Arguments:

    factor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPlaybackZoom(s32 factor)
{
    DEBUG_SYS("Trace: Playback zoom factor = %d.%d\n", factor / 10, factor % 10);

    return 1;
}

/*

Routine Description:

    Playback pan.

Arguments:

    dir - Direction.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPlaybackPan(s32 dir)
{
    DEBUG_SYS("Trace: Playback pan - pan direction = %d\n", dir);

    return 1;
}


s32 sysPlaybackMoveForward(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u32 FileNum = 0;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = dcfGetCurDirFileCount();
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = dcfGetCurDirFileCount();
#endif
    asfIndexTableCount=0;

    if (FileNum == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        return 0;
    }

    playback_location--;
    if (!playback_location)
        playback_location = FileNum;

    if (dcfPlaybackCurFile == NULL)
        return 0;

    if (dummy==0)  // 0: Just moveforward 1:After delete and don't need advanced
        dcfPlaybackFileNext();
    osdDrawPlaybackArea(sysTVOutOnFlag);
    osdDrawFileNum(playback_location);
    if (sysReadFile() == 0)
    {
        osdDrawPlayIcon();
        return 0;
    }
    osdDrawPlayIcon();
    return 1;
#endif
}

s32 sysPlaybackMoveBackward(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u32 FileNum = 0;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = dcfGetCurDirFileCount();
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = dcfGetCurDirFileCount();
#endif
    asfIndexTableCount = 0;

    if (FileNum == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        return 0;
    }

    playback_location++;
    if (playback_location > FileNum)
        playback_location = 1;

    // DEBUG_SYS("playback_location %d \n",playback_location);
    if (dcfPlaybackCurFile == NULL)
        return 0;
    dcfPlaybackFilePrev();
    osdDrawPlaybackArea(sysTVOutOnFlag);
    osdDrawFileNum(playback_location);
    if (sysReadFile() == 0)
    {
        osdDrawPlayIcon();
        return 0;
    }
    osdDrawPlayIcon();
    return 1;
#endif
}

/*

Routine Description:

    Playback delete current file.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 sysPlaybackDelete(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u32 FileNum = 0;
    u8 err;

    //OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_DELETE, OS_FLAG_CLR, &err);

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = dcfGetCurDirFileCount();
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = dcfGetCurDirFileCount();
#endif

    if (FileNum == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_DELETE, OS_FLAG_SET, &err);
        return 0;
    }

    if (playback_location == FileNum)
        playback_location=0;  // Moveforward will add one
    else
        playback_location--;   // Moveforward will add one
    system_busy_flag=1;

    IduVideo_ClearBuf();
    if (sysTVOutOnFlag)
        iduSetVidBufAddr(0, PKBuf2);
    else
        iduSetVidBufAddr(0, iduvideobuff);
    osdDrawFillWait();
    if (dcfPlaybackDel() == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        system_busy_flag=0;
        if (gInsertNAND==1)
            userClickFormat=1;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_DELETE, OS_FLAG_SET, &err);
        return 0;
    }
    if (sysTVOutOnFlag)
        iduSetVidBufAddr(0, PKBuf0);

    sysPlaybackMoveForward(1);
    system_busy_flag=0;
    if (gInsertNAND==1)
        userClickFormat=1;

    if (MemoryFullFlag == TRUE)
        MemoryFullFlag = FALSE;

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_DELETE, OS_FLAG_SET, &err);
    return 1;
#endif
}



s32 sysPlaybackDeleteAll(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u8 i = 0;

    if (sysTVOutOnFlag)
    {
        if (dcfGetCurDirFileCount()==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)PKBuf0;
#if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
            osdDrawFillEmpty();

            if (gInsertNAND==1)
            {
                OSMboxPost(general_MboxEvt, "PASS");
            }
            return 0;
        }

        DEBUG_SYS("Trace: Delete all files...\n");
        for (i=0; i<3; i++)
            uiClearOSDBuf(i);

        system_busy_flag=1;
        IduVideo_ClearBuf();
        IduVidBuf0Addr = (u32)PKBuf0;
#if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
        osdDrawFillWait();
        if (dcfPlaybackDelAll() == 0)
        {
            system_busy_flag=0;
            DEBUG_SYS("delete All Fail\r\n");
            if (gInsertNAND==1)
            {
                userClickFormat=1;
                OSMboxPost(general_MboxEvt, "FAIL");
            }
            return 0;
        }
        osdDrawFillEmpty();
        system_busy_flag=0;
        if (gInsertNAND==1)
        {
            userClickFormat=1;
            OSMboxPost(general_MboxEvt, "PASS");
        }
        DEBUG_SYS("delete All Pass\r\n");
    }
    else
    {
        if (dcfGetCurDirFileCount()==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)iduvideobuff;
#if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
            osdDrawFillEmpty();
            if (gInsertNAND==1)
            {
                OSMboxPost(general_MboxEvt, "PASS");
            }
            return 0;
        }

        DEBUG_SYS("Trace: Delete all files...\n");

        system_busy_flag=1;
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)iduvideobuff;
#if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
            osdDrawFillWait();
        }
        if (dcfPlaybackDelAll() == 0)
        {
            system_busy_flag=0;
            DEBUG_SYS("delete All Fail\r\n");
            if (gInsertNAND==1)
            {
                userClickFormat=1;
                OSMboxPost(general_MboxEvt, "FAIL");
            }
            return 0;
        }
        osdDrawFillEmpty();
        system_busy_flag=0;
        if (gInsertNAND==1)
        {
            userClickFormat=1;
            OSMboxPost(general_MboxEvt, "PASS");
        }
        else
            OSMboxPost(general_MboxEvt, "PASS");
        DEBUG_SYS("delete All Pass\r\n");

    }

    if (MemoryFullFlag == TRUE)
        MemoryFullFlag = FALSE;
    return 1;
#endif
}

/*

Routine Description:

    Playback delete the all  files in the current directory.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 sysPlaybackDeleteDir(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    if (sysTVOutOnFlag)
    {
        if (dcfGetCurDirFileCount()==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)PKBuf0;
#if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
            osdDrawFillEmpty();
            if (gInsertNAND==1)
            {
                OSMboxPost(general_MboxEvt, "PASS");
            }
            return 0;
        }

        DEBUG_SYS("Trace: Delete all files...\n");
        system_busy_flag=1;

        IduVideo_ClearBuf();
        IduVidBuf0Addr = (u32)PKBuf0;
#if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
        osdDrawFillWait();
        if (dcfPlaybackDelDir() == 0)
        {
            system_busy_flag=0;
            DEBUG_SYS("delete Dir Fail\r\n");
            if (gInsertNAND==1)
            {
                userClickFormat=1;
                OSMboxPost(general_MboxEvt, "FAIL");
            }
            return 0;
        }
        osdDrawFillEmpty();
        system_busy_flag=0;
        if (gInsertNAND==1)
        {
            userClickFormat=1;
            OSMboxPost(general_MboxEvt, "PASS");
        }
//		DEBUG_SYS("delete Dir Pass1\r\n");

    }
    else
    {
        if (dcfGetCurDirFileCount()==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)iduvideobuff;
#if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
            osdDrawFillEmpty();
            if (gInsertNAND==1)
            {
                OSMboxPost(general_MboxEvt, "PASS");
            }
            return 0;
        }

        DEBUG_SYS("Trace: Delete all files...\n");

        system_busy_flag=1;
        IduVideo_ClearBuf();
        IduVidBuf0Addr = (u32)iduvideobuff;
#if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
        osdDrawFillWait();
        DEBUG_SYS("Trace: begin 1 Delete all files...\n");

        if (dcfPlaybackDelDir() == 0)
        {
            system_busy_flag=0;
            DEBUG_SYS("delete Dir Fail\r\n");
            if (gInsertNAND==1)
            {
                userClickFormat=1;
                OSMboxPost(general_MboxEvt, "FAIL");
            }
            return 0;
        }
        osdDrawFillEmpty();
        system_busy_flag=0;
        if (gInsertNAND==1)
        {
            userClickFormat=1;
            OSMboxPost(general_MboxEvt, "PASS");
        }
//		DEBUG_SYS("delete Dir Pass2\r\n");

    }

    if (MemoryFullFlag == TRUE)
        MemoryFullFlag = FALSE;
    return 1;
#endif
}

/*

Routine Description:

  TV-OUT Playback delete current file.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 sysTVPlaybackDelete(s32 dummy)
{

    return 1;
}

/*

Routine Description:

    Playback format drive.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPlaybackFormat(s32 dummy)
{
    FS_DISKFREE_T *diskInfo;
    u8 err;

    DEBUG_SYS("Trace: Format drive...\n");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_CLR, &err);

    if (dcfPlaybackFormat() < 0)
    {
        OSMboxPost(general_MboxEvt, "FORMAT FAIL");
        return 0;
    }

    dcfCacheClean();
    diskInfo = &global_diskInfo;
    dcfDriveInfo(diskInfo, 0);
    got_disk_info = 1;

    //-----卡滿偵測-----//
    if (MemoryFullFlag == TRUE)
    {
        MemoryFullFlag = FALSE;
        sysProjectDeviceStatus(DEV_SD_NOT_FULL);
    }

    //------------------//
    playback_location = 0xFF;
    if (gInsertNAND == 1)
        userClickFormat = 1;
    pagecount = 0;
    fileitem = 0;

    OSMboxPost(general_MboxEvt, "PASS");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_SET, &err);
    return 1;
}


s32 sysBackupFormat(s32 dummy)
{
    FS_DISKFREE_T *diskInfo;
    u8 err;


    DEBUG_SYS("Trace: Format backup drive...\n");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_CLR, &err);

    if (dcfBackupFormat() == 0)
    {
        OSMboxPost(general_MboxEvt, "FORMAT FAIL");
        return 0;
    }

    dcfBackupCacheClean();
    diskInfo=&Backup_diskInfo;
    dcfBackupDriveInfo(diskInfo, 0);
    //-----卡滿偵測-----//

    //------------------//

    OSMboxPost(general_MboxEvt, "PASS");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_SET, &err);
    return 1;
}
/*

Routine Description:

    Playback in-system programming.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPlaybackIsp(s32 dummy)
{
    /*CY 1023*/
    DEBUG_SYS("Trace: In-system programming...\n");

    ispUpdate(ISP_UPDATE_CODE);
    uiMenuEnable = 0x51;
    //uiKeyReturn();
    return 1;
}

/*

Routine Description:

    Process fucntions after USB removed.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysUsbRemoved(s32 dummy)
{

    DEBUG_SYS("TRACE SYS: USB Removed!!\n\r");

    /*===============Set USB Pull-High Resistor to Ground================*/
    sysUSBPlugInFlag=0;
//   usbDevEnaCtrl(USB_R_PULL_LOW);/* disable usb pull-high resistor */
//    DEBUG_SYS("Disable Pull-high USB resistor, %X\n",Gpio1Level);


    /* release source occupied by USB */
    usbUninst();


    /* disable usb clock */
    sysUSB_disable();
    system_busy_flag = 0;   /* flag to make system be not busy */

    /* Return to Preview mode */
    sysSetEvt(SYS_EVT_PREVIEW_INIT, 0); //Lucian remove: 應於上層做.


    return 1;
}


/*

Routine Description:

    Read file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sys_P2P_Readfile(void)
{
    char* pExtName = &dcfPlaybackCurFile->pDirEnt->d_name[9];
    char extName[3];
    u8 i;
    u8 err;
    if (dcfPlaybackCurFile == NULL)
    {
        DEBUG_SYS("Trace: No current file.\n");

        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        return 0;
    }
    DEBUG_SYS("Trace: Current file %s is a ", dcfPlaybackCurFile->pDirEnt->d_name);
    /* change the file extension to upper-case */
    for (i = 0; i < 3; i++)
    {
        extName[i] = pExtName[i];
        if ((extName[i] >= 'a') && (extName[i] <= 'z'))
            extName[i] -= 0x20;
    }
    if (strncmp(extName, "ASF", 3) == 0)  //Lsk 090410 : add forward,backward function
    {
        file_err_flag =asfSplitFile(u32PacketStart);
    }
    return 1;
}

u32 seek_time;
s32 sysSetSeekTime(u32 time)
{
    seek_time = time;
    return 0;
}
s32 sysEnableThumb(u8 flag)
{
    Iframe_flag = flag;
    return 0;
}
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
/*
Description: to detect if the selected time exists file

return:
    TRUE  : Exist file
    FALSE : no file
*/

u8 sysIsTimeExistFile(u32 timeBegin, u32 timeEnd, u8 mode){
	u32   FileStartTime, NextFileStartTime, FileEndTime;
    u32   time1, time2, time3;
    u32    h1, m1, s1;
    u32    h2, m2, s2;
    u8     err;

    sysPlayBackNextFile = 0;
    dcfPlaybackCurFile = dcfGetPlaybackFileListHead();
    
    while(dcfPlaybackCurFile != dcfGetPlaybackFileListTail()){
        NextFileStartTime = AsfGetVideoTime(dcfPlaybackCurFile->next->pDirEnt->fsFileCreateTime_HMS);
//        printf("Begin:%d, NextFile:%d\n", timeBegin, NextFileStartTime);
        if(NextFileStartTime >=  timeBegin)
            break;
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
    }
    FileStartTime  = AsfGetVideoTime(dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS);
	FileEndTime    = FileStartTime + GetVideoDuration(dcfPlaybackCurFile->pDirEnt->d_name);

//    printf("(%d, %d), (%d ~ %d)/%d\n", FileStartTime, FileEndTime, timeBegin, (timeBegin + THRESHOLD_SECONDS), NextFileStartTime);
    
    if(mode == SYS_PLAYBACK_MODE) //Playback Flow
    {
        /*if the selected time is almost the current file's end, play the next file*/
        if(dcfPlaybackCurFile != dcfGetPlaybackFileListTail()){
            if((timeBegin < NextFileStartTime) && (((timeBegin + THRESHOLD_SECONDS) > NextFileStartTime)))
            {
                DEBUG_SYS("Play the Next File:%s\n", dcfPlaybackCurFile->next->pDirEnt->d_name);
                sysPlayBackNextFile = 1;
            }
            return TRUE;
        }
        /*the selected time is in the middle of this file, OK*/
        if((timeBegin >= FileStartTime && timeBegin <= FileEndTime)){
            return TRUE;
        }
    }
    else if(mode == SYS_FILE_BACKUP_MODE) //Backup File Flow
    {
        if((timeBegin >= FileStartTime && timeBegin <= FileEndTime) || (timeEnd >= FileStartTime && timeEnd <= FileEndTime)){
            return TRUE;
        }
        if((FileStartTime >= timeBegin && FileStartTime <= timeEnd) || (FileEndTime >= timeBegin && FileEndTime <= timeEnd)){
            return TRUE;
        }
    }

    /*File not exist*/
    if(mode == SYS_PLAYBACK_MODE)
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
    else if(mode == SYS_FILE_BACKUP_MODE)
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_COPY_FILE, OS_FLAG_SET, &err);    
    return FALSE;
}
s32 sysSeekFile(int time) //hhmmss
{
    u32   time1, time2;
    u8    h1, m1, s1;
    u8    h2, m2, s2;

    if(dcfGetPlaybackFileListHead() == dcfGetPlaybackFileListTail())//Lsk: only one file
    {
        u32PacketStart = 0;
        DEBUG_SYS("Only one file\n");
        return 0;
    }

    u32PacketStart = 60*60; //60min
    dcfPlaybackCurFile = dcfGetPlaybackFileListHead(); //Inverse file link

    DEBUG_SYS("Seek File pos start\n");
    while(1)
    {
        //printf("%s\n", (signed char*)dcfPlaybackCurFile->pDirEnt->d_name);
        //printf("%s\n", (signed char*)dcfPlaybackCurFile->next->pDirEnt->d_name);

        sscanf((char*)(signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "%d", &time1);
        sscanf((char*)(signed char*)dcfPlaybackCurFile->next->pDirEnt->d_name, "%d", &time2);
        //DEBUG_SYS("ASF name: <curr, search, curr_next> = <%06d, %06d, %06d>\n", time1, time, time2);

        if(time<time1)
        {
            u32PacketStart = 0;
            //DEBUG_SYS("u32PacketStart  = %d\n", u32PacketStart);
            break;
        }
        else if(((time>=time1) && (time<time2))||(dcfPlaybackCurFile == dcfGetPlaybackFileListTail()))
        {
            //DEBUG_SYS("find time = <%d, %d, %d>\n", time1, time, time2);
            h1=time1/(100*100);
            m1=(time1-h1*(100*100))/100;
            s1 = time1%100;

            h2=time/(100*100);
            m2=(time-h2*(100*100))/100;
            s2 = time%100;

            u32PacketStart = (h2*3600+m2*60+s2)-(h1*3600+m1*60+s1);
            //DEBUG_SYS("u32PacketStart  = %d\n", u32PacketStart);
            break;
        }
        else
        {
            if(dcfPlaybackCurFile == dcfGetPlaybackFileListTail())
                break;
            else
                dcfPlaybackCurFile = dcfPlaybackCurFile->next;
        }

    }

    if(sysPlayBackNextFile == 1){
        dcfPlaybackCurFile = dcfPlaybackCurFile->next;
        u32PacketStart = 0;
    }
    //DEBUG_SHOW_START();
    DEBUG_SYS("Seek File pos end, FileName = %s, u32PacketStart=%d\n", dcfPlaybackCurFile->pDirEnt->d_name, u32PacketStart);
    //DEBUG_SHOW_END();
    return 0;
}

u32 sysGetVideoTime(void)
{
    //File creat time add video VideoNextPresentTime
    u8    h1, m1, s1;
    u8    h2, m2, s2;
    u8    h3, m3, s3;

    u32   tmp;
    u8    add=0;

    h1 = (VideoNextPresentTime/1000000)/3600;
    m1 = (VideoNextPresentTime/1000000 - h1*3600) / 60;
    s1 = VideoNextPresentTime/1000000 - h1*3600 - m1*60;

    DEBUG_SYS("@@@ %s\n", (signed char*)dcfPlaybackCurFile->pDirEnt->d_name);
    sscanf(dcfPlaybackCurFile->pDirEnt->d_name, "%d", &tmp);
    DEBUG_SYS("@@@ %d\n", tmp);
    h2=tmp/10000;
    m2=(tmp-h2*1000)/100;
    s2 = tmp%100;

    if(s1+s2>60)
    {
        s3=s1+s2-60;
        add=1;
    }
    else
    {
        s3=s1+s2;
        add=0;
    }

    if(m1+m2+add>60)
    {
        m3=m1+m2+add-60;
        add=1;
    }
    else
    {
        m3=m1+m2+add-60;
        add=0;
    }

    if(h1+h2+add>60)
    {
        h3=h1+h2+add-60;
        add=1;
    }
    else
    {
        h3=h1+h2+add-60;
        add=0;
    }

    return (h3*1000+m3*100+s3);

}


s32 InitReadFile(void)
{
    int i;

    //current use
#if(USE_NEW_MEMORY_MAP)
    for(i=0; i<DISPLAY_BUF_NUM; i++)
    {
        if(((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i])||((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i]+DISPLAY_BUFFER_Y_OFFSET))
        {
            break;
        }
    }

#else
    for(i=0; i<DISPLAY_BUF_NUM; i++)
    {
        if((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i])
        {
            break;
        }
    }
#endif
    //clear next buffer to black screen and show it

#if(USE_NEW_MEMORY_MAP)
    Idu_ClearBuf_FHD_IN_ULTRA_FHD(i);
    iduPlaybackFrame(MainVideodisplaybuf[i]);
#else
    memDispBufArrage(DISPBUF_9300FHD);
    Idu_ClearBuf_9300FHD_Idx((i+1)%DISPLAY_BUF_NUM);
    iduPlaybackFrame(MainVideodisplaybuf[((i+1)%DISPLAY_BUF_NUM)]);
#endif

    return 1;
}
s32 ExitReadFile(void)
{
    /***************************************************************
    Lsk: reset Init mode=DISPBUF_NORMAL
    ***************************************************************/
    memDispBufArrage(DISPBUF_NORMAL);
    return 1;
}
s32 BeforeReadFile(int mode)
{
    printf("2.MainVideodisplaybuf_idx = %d, mode = %d\n\n\n", MainVideodisplaybuf_idx, mode);

    //OSTimeDly(15*20);
    if(mode == DISPBUF_9300FHD)
        Idu_ClearBuf_9300FHD_FromHD();
    //else if(mode == DISPBUF_NORMAL)
    //	Idu_ClearBuf_9300FHD_FromFHD();
    //OSTimeDly(15*20);
    return 1;
}

s32 sysContinuousReadFile(void)
{
    char* pExtName = &dcfPlaybackCurFile->pDirEnt->d_name[9];
    char extName[3];
    u8 i, doPlaybackBackward;
#if IIS_DEBUG_ENA
    extern u32 under_count;
#endif

#if DINAMICALLY_POWER_MANAGEMENT  /* Peter */
    u32     sys_ctl0_status;
#endif
    u32 local_height=0;
    u8 err;

    // Avoid warning msg
    i = 0;
    doPlaybackBackward = doPlaybackBackward;
    //return 1;

    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD | SYS_ARBHIPIR_SIU;

    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 5>1>2>3>4

    tvTVE_EN |= 0x00000040;//TVDAC_POWON;
    SYS_TV_PLLCTL = (SYS_TV_PLLCTL & 0xfff3ffff) |  0x000080000;  //Lucian:  因FPGA board共用TV DAC, this register 可做切換.
    /*BJ 0609 S*/


    if(!sysTVOutOnFlag)  //Lsk 090810 : HW_BOARD have both pannel and tv-out
    {
#if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04)) /*BJ 0721 S*/
        idu_switch();   /*CY 0907 TEST*/
#endif
    }

#if(MULTI_CHANNEL_SEL & 0x01)
    isuStop();
    ipuStop();
    siuStop();
#endif

#if(MULTI_CHANNEL_SEL & 0x02)
    ciu_1_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x04)
    ciu_2_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x08)
    ciu_3_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x10)
    ciu_4_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x20)
    ciu_5_Stop();
    ipuStop();
    siuStop();
#endif


#if DINAMICALLY_POWER_MANAGEMENT
    /* Peter */
    //u32     sys_ctl0_status;
    // disable unused module for reduce power consumption
    sys_ctl0_status     = SYS_CTL0;
#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif

#if IDU_TV_DISABLE
    sys_ctl0_status    |= SYS_CTL0_IIS_CKEN;        // For playback IIS;
#else
    sys_ctl0_status    |= SYS_CTL0_IDU_CKEN |
                          SYS_CTL0_IIS_CKEN;        // For playback IIS;
#endif

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                          //~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_SRAM_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                          ~SYS_CTL0_SCUP_CKEN &
                          ~SYS_CTL0_H264_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

    //return 1;
    /* check if null */
    if (dcfPlaybackCurFile == NULL)
    {
        DEBUG_SYS("Trace: No current file.\n");

        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        return 0;
    }

    DEBUG_SYS("Trace: Current file %s is a ", dcfPlaybackCurFile->pDirEnt->d_name);
    pExtName = &dcfPlaybackCurFile->pDirEnt->d_name[9];
    /* change the file extension to upper-case */
    for (i = 0; i < 3; i++)
    {

        extName[i] = pExtName[i];
        if ((extName[i] >= 'a') && (extName[i] <= 'z'))
            extName[i] -= 0x20;
    }

    if (strncmp(extName, "ASF", 3) == 0)  //Lsk 090410 : add forward,backward function
    {
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
#endif

#if IIS_DEBUG_ENA
        under_count=0;
#endif


        sysSeekFile(seek_time);

        // for Thumnail display
        sysThumnailPtr->type = 1;  // 1: ASF
        if ((Iframe_flag==1) || (Iframe_flag==2))
        {
            strcpy((char*)sysThumnailPtr->tname,dcfPlaybackCurFile->pDirEnt->d_name);

            VideoNextPresentTime	= u32PacketStart*1000000;
            if(Iframe_flag==1)
            sysPlaybackThumbnail    = 1;
            else
                sysPlaybackThumbnail    = 2;
            sysPlaybackVideoStart   = 1;
            sysPlaybackVideoStop    = 0;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackBackward     = -1;  //Lsk 090403
            video_playback_speed = pre_playback_speed = 5; //Lsk 0324
            //u32PacketStart          = 0;
            doPlaybackBackward      = 0;
            asfIndexTableRead       = 0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
#if ASF_SPLIT_FILE
            file_err_flag =asfSplitFile(u32PacketStart);
#else
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            sysPlaybackHeight = GetVideoHeight((s8 *)dcfPlaybackCurFile->pDirEnt->d_name);
            InitReadFile();
#else
			if( Iframe_flag == 1 )
            Idu_ClearBuf(DISPLAY_BUF_NUM);
#endif

            file_err_flag =asfReadFile(u32PacketStart);
#endif
            sysPlaybackVideoStart   = 0;
            sysPlaybackVideoStop    = 1;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackThumbnail    = 0;

#if DINAMICALLY_POWER_MANAGEMENT
            SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#endif
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
            OSMboxPost(general_MboxEvt, "PASS");

            ExitReadFile();
            return file_err_flag;
        }

        if (sysTVOutOnFlag)
        {

            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
        }
        else
        {
            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
        }

        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
                tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
        }

        //return 1;

        sysVoiceRecStart=2;
        RTCseconds=0;
        timerInterruptEnable(1,1);

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_DEVICE == AUDIO_NULL)
        init_DAC_play(1);
#endif
        DEBUG_SYS("ASF file.\n");
        VideoNextPresentTime	= u32PacketStart*1000000;
        sysPlaybackVideoStart   = 1;
        sysPlaybackVideoStop    = 0;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = 0;
        sysPlaybackBackward     = -1;
        //u32PacketStart          = 0;
        //doPlaybackBackward      = 0;
        asfIndexTableRead       = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        video_playback_speed = pre_playback_speed = 5; //Lsk 0324
        DEBUG_SHOW_START();
        DEBUG_SYS("File name : %s\n",dcfPlaybackCurFile->pDirEnt->d_name);
        DEBUG_SHOW_END();
        curr_playback_speed = 5;
        curr_slow_speed = 0;
        //return 1;
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
        //InitReadFile(); #if(USE_NEW_MEMORY_MAP)
        local_height = sysPlaybackHeight = GetVideoHeight((s8 *)dcfPlaybackCurFile->pDirEnt->d_name);
#endif
        file_err_flag = asfReadFile(u32PacketStart);
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        //sysPlaybackForward      = 0;

        //DEBUG_SYS("dcfListFileEntHead File name : %s\n",dcfGetPlaybackFileListHead()->pDirEnt->d_name);
        //DEBUG_SYS("dcfListFileEntTail File name : %s\n",dcfGetPlaybackFileListTail()->pDirEnt->d_name);


        while(videoPlayNext == 1)
        {
            printf("file 0x%08x, 0x%08x, 0x%08x\n", dcfPlaybackCurFile, dcfGetPlaybackFileListHead(), dcfGetPlaybackFileListTail());
            OSTimeDly(1);//Lsk 090505
            if(sysPlaybackForward >=0 && (dcfPlaybackCurFile!=dcfGetPlaybackFileListTail()))
            {
                if(sysPlaybackForward ==0)
                {
#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
                    Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
                    Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
                    Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
                    ac97SetupALC203_play();
#elif(AUDIO_DEVICE == AUDIO_NULL)
                    init_DAC_play(1);
#endif
                }

                playback_location++;
                //osdDrawFileNum(playback_location);
                //osdDrawPlayIndicator(sysThumnailPtr->type);
                dcfPlaybackCurFile = dcfPlaybackCurFile->next;
                VideoNextPresentTime	= u32PacketStart*1000000;
                DEBUG_SYS("1.File name : %s\n",dcfPlaybackCurFile->pDirEnt->d_name);
                sysPlaybackVideoStart   = 1;
                sysPlaybackVideoStop    = 0;
                sysPlaybackVideoPause   = 0;
                u32PacketStart          = 0;
                asfIndexTableRead       = 0;
                IsuIndex                = 0;
                VideoNextPresentTime    = 0;
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                DEBUG_SYS("2. Trace: Current file %s is a ", dcfPlaybackCurFile->pDirEnt->d_name);
                local_height = GetVideoHeight((s8 *)dcfPlaybackCurFile->pDirEnt->d_name);
                if(local_height != sysPlaybackHeight)
                {
#if(USE_NEW_MEMORY_MAP)
                    for(i=0; i<DISPLAY_BUF_NUM; i++)
                    {
                        if(((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i])||((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i]+DISPLAY_BUFFER_Y_OFFSET))
                        {
                            break;
                        }
                    }
                    Idu_ClearBuf_FHD_IN_ULTRA_FHD(i);
                    if(i!=0)
                        Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
                    sysPlaybackHeight = local_height;
#else
                    if(local_height > 720)
                        BeforeReadFile(DISPBUF_9300FHD);
                    else
                        BeforeReadFile(DISPBUF_NORMAL);
                    sysPlaybackHeight = local_height;
#endif
                }
#endif
                file_err_flag = asfReadFile(0);
                //sysPlaybackVideoStart   = 0;
                //sysPlaybackVideoStop    = 1;
                //sysPlaybackVideoPause   = 0;
            }
            else if(sysPlaybackBackward >=0 && (dcfPlaybackCurFile!=dcfGetPlaybackFileListHead()))
            {
                playback_location--;
                //osdDrawFileNum(playback_location);
                //osdDrawPlayIndicator(sysThumnailPtr->type);

                dcfPlaybackCurFile = dcfPlaybackCurFile->prev;
                IsuIndex                = 0;                
                VideoNextPresentTime	= GetVideoDuration(dcfPlaybackCurFile->pDirEnt->d_name)*1000000;

                DEBUG_SYS("2.File name : %s\n",dcfPlaybackCurFile->pDirEnt->d_name);
                sysPlaybackVideoStart   = 1;
                sysPlaybackVideoStop    = 0;
                sysPlaybackVideoPause   = 0;
                u32PacketStart          = 0;
                asfIndexTableRead       = 0;
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                DEBUG_SYS("2. Trace: Current file %s is a ", dcfPlaybackCurFile->pDirEnt->d_name);
                local_height = GetVideoHeight((s8 *)dcfPlaybackCurFile->pDirEnt->d_name);
                if(local_height != sysPlaybackHeight)
                {
#if(USE_NEW_MEMORY_MAP)
                    for(i=0; i<DISPLAY_BUF_NUM; i++)
                    {
                        if(((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i])||((u32)IduVidBuf0Addr == (u32)MainVideodisplaybuf[i]+DISPLAY_BUFFER_Y_OFFSET))
                        {
                            break;
                        }
                    }
                    Idu_ClearBuf_FHD_IN_ULTRA_FHD(i);
                    if(i!=0)
                        Idu_ClearBuf_FHD_IN_ULTRA_FHD(0);
                    sysPlaybackHeight = local_height;
#else
                    if(local_height > 720)
                        BeforeReadFile(DISPBUF_9300FHD);
                    else
                        BeforeReadFile(DISPBUF_NORMAL);
                    sysPlaybackHeight = local_height;
#endif
                }
#endif
                file_err_flag = asfReadFile(3600);
                //sysPlaybackVideoStart   = 0;
                //sysPlaybackVideoStop    = 1;
                //sysPlaybackVideoPause   = 0;
            }
            else
            {
                OSTimeDly(1);
                for(i=0;i<DISPLAY_BUF_NUM;i++){
                    Idu_ClearBuf_ULTRA_FHD(i); //black screen when playback finished
                }
                videoPlayNext = 0;
                break;
            }
        }
        //osdDrawPlayIndicator(100);
        videoPlayNext = 0;
        curr_playback_speed = 5;
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = 0;
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#endif
        if (sysTVOutOnFlag)
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }
        else
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
#endif
        iduOSDEnable_All();
        tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
        IduDispWinSel(0);

        sysVoiceRecStart=0;
        timerInterruptEnable(1,0);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");

    }
    else
    {
        DEBUG_SYS("unknown file.\n");
    }

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
    ExitReadFile();
    return file_err_flag;
}

u32 CopyBeginSec, CopyEndSec;
u32 CopyDone=0;

s32 sysSetCopyDuration(u32 BeginSec, u32 EndSec)
{
    CopyBeginSec = BeginSec;
    CopyEndSec   = EndSec;

    return true;
}

s32 sysCopyFile(void)
{
    u8 err;

    u8	hh, mm, ss;
    u32 tmp;

    hh = CopyBeginSec/3600;
    mm = (CopyBeginSec-hh*3600)/60;
    ss = CopyBeginSec%60;
    tmp = hh*10000 + mm*100 + ss;

    printf("seek_time = %d\n", CopyBeginSec);
    sysSeekFile(tmp);
    FileBackup(CopyBeginSec, CopyEndSec);
    sysPlaybackVideoStop = 1;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_COPY_FILE, OS_FLAG_SET, &err);
    CopyDone = 1;

    return 0;
}
#endif

s32 sysReadFile(void)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    char* pExtName = &dcfPlaybackCurFile->pDirEnt->d_name[9];
    char extName[3];
    u8 i, doPlaybackBackward;

#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
				(UI_VERSION == UI_VERSION_RDI_3))
#else
    UI_MULT_ICON *iconInfo;
    u16 DrawX = 138;
#endif
#if IIS_DEBUG_ENA
    extern u32 under_count;
#endif

#if DINAMICALLY_POWER_MANAGEMENT  /* Peter */
    u32     sys_ctl0_status;
#endif
    u8 err;
    //
    doPlaybackBackward = doPlaybackBackward;	// Avoid warning msg
    //
    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD | SYS_ARBHIPIR_SIU;

    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

    tvTVE_EN |= 0x00000040;//TVDAC_POWON;
    SYS_TV_PLLCTL = (SYS_TV_PLLCTL & 0xfff3ffff) |  0x000080000;  //Lucian:  因FPGA board共用TV DAC, this register 可做切換.

    /*BJ 0609 S*/
    if(!sysTVOutOnFlag)  //Lsk 090810 : HW_BOARD have both pannel and tv-out
    {
#if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04)) /*BJ 0721 S*/
        idu_switch();   /*CY 0907 TEST*/
#endif
    }

#if(MULTI_CHANNEL_SEL & 0x01)
    isuStop();
    ipuStop();
    siuStop();
#endif

#if(MULTI_CHANNEL_SEL & 0x02)
    ciu_1_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x04)
    ciu_2_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x08)
    ciu_3_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x10)
    ciu_4_Stop();
#endif

#if(MULTI_CHANNEL_SEL & 0x20)
    ciu_5_Stop();
    ipuStop();
    siuStop();
#endif


#if DINAMICALLY_POWER_MANAGEMENT
    /* Peter */
    //u32     sys_ctl0_status;
    // disable unused module for reduce power consumption
    sys_ctl0_status     = SYS_CTL0;
#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif

#if IDU_TV_DISABLE
    sys_ctl0_status    |= SYS_CTL0_IIS_CKEN;        // For playback IIS;
#else
    sys_ctl0_status    |= SYS_CTL0_IDU_CKEN |
                          SYS_CTL0_IIS_CKEN;        // For playback IIS;
#endif

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                          //~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_SRAM_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                          ~SYS_CTL0_SCUP_CKEN &
                          ~SYS_CTL0_H264_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

    /* check if null */
    if (dcfPlaybackCurFile == NULL)
    {
        DEBUG_SYS("Trace: No current file.\n");

        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        return 0;
    }

    DEBUG_SYS("Trace: Current file %s is a ", dcfPlaybackCurFile->pDirEnt->d_name);

    /* change the file extension to upper-case */
    for (i = 0; i < 3; i++)
    {
        extName[i] = pExtName[i];
        if ((extName[i] >= 'a') && (extName[i] <= 'z'))
            extName[i] -= 0x20;
    }

    /* check the file extension */
    if (strncmp(extName, "TIF", 3) == 0)
    {
        DEBUG_SYS("TIF file.\n");
    }
    else if (strncmp(extName, "JPG", 3) == 0)
    {
        DEBUG_SYS("JPG file.\n");
        strcpy((char*)sysThumnailPtr->tname,dcfPlaybackCurFile->pDirEnt->d_name);
        sysThumnailPtr->type = 0;  //0: JPG
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        file_err_flag = exifReadFile();
        if (!file_err_flag)
        {
#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
            (UI_VERSION == UI_VERSION_RDI_3))
#else
            uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,DrawX , 88+osdYShift/2 , OSD_Blk0, 0x00 , alpha_3);
            uiOsdGetIconInfo(OSD_ICON_WARNING_1, &iconInfo);
            DrawX += iconInfo->Icon_w;
            uiOSDIconColorByXY((OSD_ICONIDX)(OSD_ICON_WARNING_1+1) ,DrawX , 88+osdYShift/2 , OSD_Blk0, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk0, 0xC0, 0x00);
#endif
        }
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");
    }
    else if (strncmp(extName, "THM", 3) == 0)
    {
        DEBUG_SYS("THM file.\n");
    }
    else if (strncmp(extName, "WAV", 3) == 0)
    {

        if (uiGetMenuMode() != PLAYBACK_MODE)
        {
            strcpy((char*)sysThumnailPtr->tname,dcfPlaybackCurFile->pDirEnt->d_name);
            sysThumnailPtr->type = 2;  // 2: WAV
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            return 1;
        }
        if (sysTVOutOnFlag)
        {
            uiClearOSDBuf(1);
            //sysISU_enable();
            //exifDecodeJPEGToYUV(PKBuf, Setup_Wave, 9000, &uWidth, &uHeight);
            //isuScalar_D2D( PKBuf , PKBuf2 , 160, 240, display_x, display_y);
            iduSetVBuff(PKBuf2, PKBuf2, PKBuf2);
            for (i=0; i<3; i++)
                iduTVOSDEnable(i);
        }
        else
        {
            uiClearOSDBuf(1);
            //exifDecodeJPEGToYUV(PKBuf, Setup_Wave, 9000, &uWidth, &uHeight);
            iduSetVBuff(PKBuf, PKBuf, PKBuf);
        }

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_DEVICE == AUDIO_NULL)
        init_DAC_play(1);
#endif
        DEBUG_SYS("WAV file.\n");

        //Civic 070829 S
        RTCseconds=0;
        sysVoiceRecStart=3;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        timerInterruptEnable(1,1);

        SYS_CTL0   |= SYS_CTL0_IIS_CKEN;
        sysVoicePlayStop=0;
        IISPplyback = 1;
        IISMode = 1;
        wavreadfile();
        iisStopPlay();
        iisSuspendTask();
        iis5Reset(IIS_SYSPLL_SEL_48M);
        IISPplyback=0;
        sysVoiceRecStart=0;

        timerInterruptEnable(1,0);

        //Civic 070829 E

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#endif
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
    }
#if(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
    else if (strncmp(extName, "AVI", 3) == 0)
    {

#if DINAMICALLY_POWER_MANAGEMENT
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        SYS_CTL0   |= SYS_CTL0_MPEG4_CKEN | SYS_CTL0_IIS_CKEN;
#elif (VIDEO_CODEC_OPTION == H264_CODEC)
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
        SYS_CTL0   |= SYS_CTL0_JPEG_CKEN | SYS_CTL0_IIS_CKEN;
#endif
#endif
        // for Thumnail display
        sysThumnailPtr->type = 1;  // 1: ASF/Avi
        if (Iframe_flag==1)
        {


            strcpy((char*)sysThumnailPtr->tname,dcfPlaybackCurFile->pDirEnt->d_name);

            sysPlaybackThumbnail    = 1;
            sysPlaybackVideoStart   = 1;
            sysPlaybackVideoStop    = 0;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackBackward     = -1;  //Lsk 090403
            u32PacketStart          = 0;
            doPlaybackBackward      = 0;
            asfIndexTableRead       = 0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            file_err_flag =aviReadFile();
            sysPlaybackVideoStart   = 0;
            sysPlaybackVideoStop    = 1;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackThumbnail    = 0;

#if DINAMICALLY_POWER_MANAGEMENT
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
#elif (VIDEO_CODEC_OPTION == H264_CODEC)
            SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            SYS_CTL0   &= ~SYS_CTL0_JPEG_CKEN;
#endif
#endif

            if (!file_err_flag)
            {
#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
            (UI_VERSION == UI_VERSION_RDI_3))
#else
                iduOSDEnable(1);
                uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                uiOsdGetIconInfo(OSD_ICON_WARNING_1, &iconInfo);
                DrawX += iconInfo->Icon_w;
                uiOSDIconColorByXY(OSD_ICON_WARNING_1+1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

                OSTimeDly(10);
                iduOSDDisable(1);
#endif
            }
            else
                osdDrawPlayIcon();
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
            OSMboxPost(general_MboxEvt, "PASS");
            return file_err_flag;
        }

        if (sysTVOutOnFlag)
        {
            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
        }
        else
        {
            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
        }
        uiClearOSDBuf(1);



        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
                tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
        }
        sysVoiceRecStart=2;
        RTCseconds=0;
        timerInterruptEnable(1,1);


#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_DEVICE == AUDIO_NULL)
        init_DAC_play(1);
#endif

        DEBUG_SYS("AVI file.\n");
        sysPlaybackVideoStart   = 1;
        sysPlaybackVideoStop    = 0;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = SYS_PLAYBACK_FORWARD_X1;
        sysPlaybackBackward     = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        aviReadFile();  /* Peter: 0707 */
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = SYS_PLAYBACK_FORWARD_X1;
        sysPlaybackBackward     = 0;
#if DINAMICALLY_POWER_MANAGEMENT
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
#elif (VIDEO_CODEC_OPTION == H264_CODEC)
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
        SYS_CTL0   &= ~SYS_CTL0_JPEG_CKEN;
#endif
#endif

        if (!file_err_flag)
        {
#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
            (UI_VERSION == UI_VERSION_RDI_3))
#else
            iduOSDEnable(1);

            uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            uiOsdGetIconInfo(OSD_ICON_WARNING_1, &iconInfo);
            DrawX += iconInfo->Icon_w;
            uiOSDIconColorByXY(OSD_ICON_WARNING_1+1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

            OSTimeDly(10);
            iduOSDDisable(1);
#endif
        }

        if (sysTVOutOnFlag)
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }
        else
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Close_IIS_ALC5621();
#endif
        iduOSDEnable_All();

        sysVoiceRecStart=0;
        timerInterruptEnable(1,0);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");

    }
#endif  // #if(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
    else if (strncmp(extName, "ASF", 3) == 0)  //Lsk 090410 : add forward,backward function
    {
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
#endif

#if IIS_DEBUG_ENA
        under_count=0;
#endif
        if(Iframe_flag != 2)
        Idu_ClearBuf(DISPLAY_BUF_NUM);
        // for Thumnail display
        sysThumnailPtr->type = 1;  // 1: ASF
        if ((Iframe_flag==1) || (Iframe_flag==2))
        {
            strcpy((char*)sysThumnailPtr->tname,dcfPlaybackCurFile->pDirEnt->d_name);
            if(Iframe_flag==1)
            sysPlaybackThumbnail    = 1;
            else
                sysPlaybackThumbnail    = 2;
            
            sysPlaybackVideoStart   = 1;
            sysPlaybackVideoStop    = 0;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackBackward     = -1;  //Lsk 090403
            //u32PacketStart          = 0;
            doPlaybackBackward      = 0;
            asfIndexTableRead       = 0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            video_playback_speed = pre_playback_speed = 5; //Lsk 0324
#if ASF_SPLIT_FILE
            file_err_flag =asfSplitFile(u32PacketStart);
#else
            file_err_flag =asfReadFile(u32PacketStart);
#endif
            u32PacketStart          = 0;
            sysPlaybackVideoStart   = 0;
            sysPlaybackVideoStop    = 1;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackThumbnail    = 0;

#if DINAMICALLY_POWER_MANAGEMENT
            SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#endif
            if (!file_err_flag)
            {
#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
            (UI_VERSION == UI_VERSION_RDI_3))
#else
                iduOSDEnable(1);

                uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                uiOsdGetIconInfo(OSD_ICON_WARNING_1, &iconInfo);
                DrawX += iconInfo->Icon_w;
                uiOSDIconColorByXY((OSD_ICONIDX)(OSD_ICON_WARNING_1+1) ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

                OSTimeDly(10);
                iduOSDDisable(1);
#endif
            }
            else
            {
              #if UI_SYNCHRONOUS_DUAL_OUTPUT
                uiOsdDrawInit();
              #endif
              if(Iframe_flag != 2)
                osdDrawPlayIcon();
              #if UI_SYNCHRONOUS_DUAL_OUTPUT
                if(((Iframe_flag == 1) || (!sysPlaybackVideoPause && StartPlayBack)) && (BRI_SCCTRL_MODE & 0x4) )
                {
                    iduTVColorbar_onoff(0); 
                }
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
              #endif
            }
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
            OSMboxPost(general_MboxEvt, "PASS");
			StartPlayBack=0;
            return file_err_flag;
        }
        if (sysTVOutOnFlag)
        {

            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
        }
        else
        {
            AHB_ARBCtrl |= SYS_ARBHIPIR_DMALOW; //Lucian: 提升DMA piority
            uiClearOSDBuf(1);
        }


        if(sysTVOutOnFlag)
        {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
                tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
        }
        sysVoiceRecStart=2;
        RTCseconds=0;
        timerInterruptEnable(1,1);

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_DEVICE == AUDIO_NULL)
        init_DAC_play(1);
#endif


        DEBUG_SYS("ASF file.\n");
        sysPlaybackVideoStart   = 1;
        sysPlaybackVideoStop    = 0;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = 0;
        sysPlaybackBackward     = -1;        
        //u32PacketStart          = 0;
        //doPlaybackBackward      = 0;
        asfIndexTableRead       = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        video_playback_speed = pre_playback_speed = 5; //Lsk 0324
        file_err_flag = asfReadFile(u32PacketStart);
        u32PacketStart          = 0;
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        //sysPlaybackForward      = 0;
#if FPGA_BOARD_A1018_SERIES
        while(videoPlayNext == 1)
        {
            OSTimeDly(1);//Lsk 090505
            if(sysPlaybackForward >=0 && dcfPlaybackCurFile != dcfGetPlaybackFileListTail())
            {
                if(sysPlaybackForward ==0)
                {
   #if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
                    Init_IIS_WM8974_play();
   #elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
                    Init_IIS_WM8940_play();
   #elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
                    Init_IIS_ALC5621_play();
   #elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
                    ac97SetupALC203_play();
   #elif(AUDIO_DEVICE == AUDIO_NULL)
                    init_DAC_play(1);
   #endif
                }

                playback_location++;
                osdDrawFileNum(playback_location);
                osdDrawPlayIndicator(sysThumnailPtr->type);
                dcfPlaybackCurFile = dcfPlaybackCurFile->next;
                DEBUG_SYS("File name : %s\n",dcfPlaybackCurFile->pDirEnt->d_name);
                sysPlaybackVideoStart   = 1;
                sysPlaybackVideoStop    = 0;
                sysPlaybackVideoPause   = 0;
                u32PacketStart          = 0;
                asfIndexTableRead       = 0;
                file_err_flag = asfReadFile(u32PacketStart);
                //sysPlaybackVideoStart   = 0;
                //sysPlaybackVideoStop    = 1;
                //sysPlaybackVideoPause   = 0;
            }
            else if(sysPlaybackBackward >=0 && dcfPlaybackCurFile != dcfGetPlaybackFileListHead())
            {
                playback_location--;
                osdDrawFileNum(playback_location);
                osdDrawPlayIndicator(sysThumnailPtr->type);

                dcfPlaybackCurFile = dcfPlaybackCurFile->prev;
                DEBUG_SYS("File name : %s\n",dcfPlaybackCurFile->pDirEnt->d_name);
                sysPlaybackVideoStart   = 1;
                sysPlaybackVideoStop    = 0;
                sysPlaybackVideoPause   = 0;
                u32PacketStart          = 0;
                asfIndexTableRead       = 0;
                file_err_flag = asfReadFile(u32PacketStart);
                //sysPlaybackVideoStart   = 0;
                //sysPlaybackVideoStop    = 1;
                //sysPlaybackVideoPause   = 0;
            }
            else
            {
                videoPlayNext = 0;
                break;
            }
        }
        osdDrawPlayIndicator(100);
#endif
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = 0;
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#endif
        if (!file_err_flag)
        {
#if ((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
            (UI_VERSION == UI_VERSION_RDI_3))
#else
            iduOSDEnable(1);

            uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            uiOsdGetIconInfo(OSD_ICON_WARNING_1, &iconInfo);
            DrawX += iconInfo->Icon_w;
            uiOSDIconColorByXY((OSD_ICONIDX)(OSD_ICON_WARNING_1+1),DrawX , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

            OSTimeDly(10);
            iduOSDDisable(1);
#endif
        }
        if (sysTVOutOnFlag)
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }
        else
        {
            AHB_ARBCtrl &= (~SYS_ARBHIPIR_DMALOW); //Lucian:恢復CPU piority
        }

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
#endif  
        if(sysTVOutOnFlag)
        {
            uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        }
        else // pannel
        {
            uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        }
        //iduOSDEnable_All();
        tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
        IduDispWinSel(0);

        sysVoiceRecStart=0;
        timerInterruptEnable(1,0);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");
		StartPlayBack=0;

    }
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
    else if (strncmp(extName, "MP4", 3) == 0)
    {
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
#endif
#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        Init_IIS_WM8940_play();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#endif

        DEBUG_SYS("MP4 file.\n");
        sysPlaybackVideoStart   = 1;
        sysPlaybackVideoStop    = 0;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = SYS_PLAYBACK_FORWARD_X1;
        sysPlaybackBackward     = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        mp4ReadFile();  /* cytsai: 0315 */
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = SYS_PLAYBACK_FORWARD_X1;
        sysPlaybackBackward     = 0;
#if DINAMICALLY_POWER_MANAGEMENT
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#endif
#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#endif
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
    }
#endif
    else
    {
        DEBUG_SYS("unknown file.\n");
    }

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
    return file_err_flag;
#endif
}

//--------------------------------------------------//
void SYSClkEnable(u32 uiClkEnable)
{
    SYS_CTL0 |= uiClkEnable;
}

void SYSClkDisable(u32 uiClkDisable)
{
    SYS_CTL0 &= (~uiClkDisable);
}

void SYSReset(u32 uiReset)
{
    INT32U  i;

    SYS_RSTCTL &= (~uiReset);
    for (i=0; i<30; i++);
    SYS_RSTCTL |= (uiReset);
    for (i=0; i<30; i++);
    SYS_RSTCTL &= (~uiReset);
}

void SYSReset_EXT(u32 uiReset)
{
    INT32U  i;

    SYS_RSTCTL_EXT &= (~uiReset);
    for (i=0; i<30; i++);
    SYS_RSTCTL_EXT |= (uiReset);
    for (i=0; i<30; i++);
    SYS_RSTCTL_EXT &= (~uiReset);
}
void sysJpegRst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_JPEG_RST);
    for (i=0; i<10; i++);
    SYS_RSTCTL |= SYS_RSTCTL_JPEG_RST;
    for (i=0; i<10; i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_JPEG_RST);

}
void sysSDCRst(void)
{
    s32 i;

#if((CHIP_OPTION == CHIP_A1019A)||(CHIP_OPTION == CHIP_A1025A))
    //Lucian:暫時不做reset, 怕影響SD2
#elif(CHIP_OPTION == CHIP_A1021A)

#else
    SYS_RSTCTL &= (~SYS_RSTCTL_SD_RST);
    for (i=0; i<30; i++);
    SYS_RSTCTL |= SYS_RSTCTL_SD_RST;
    for (i=0; i<30; i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_SD_RST);
#endif
}

void sysSIURst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_SIU_RST);
    for (i=0; i<10; i++);
    SYS_RSTCTL |= SYS_RSTCTL_SIU_RST;
    for (i=0; i<10; i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_SIU_RST);

}

void sysSIU_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_SIU_CKEN;
}

void sysSIU_disable(void)
{
    siuStop();
    ipuStop();
    SYS_CTL0   &= ~SYS_CTL0_SIU_CKEN;
}

void sysJPEG_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_JPEG_CKEN;
}

void sysJPEG_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_JPEG_CKEN;
}

void sysMPEG_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_H264_CKEN;
}

void sysMPEG_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
}

void sysISU_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_ISU_CKEN |
                  SYS_CTL0_SCUP_CKEN |
                  0x00000000;
}

void sysISU_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_ISU_CKEN  &
                  ~SYS_CTL0_SCUP_CKEN &
                  0xffffffff;
}

void sysIDU_enable(void)
{
#if IDU_TV_DISABLE

#else
    SYS_CTL0    |= SYS_CTL0_IDU_CKEN;
#endif
}

void sysIDU_disable(void)
{
    SYS_CTL0    &= ~SYS_CTL0_IDU_CKEN;
}

void sysTVOUT_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_TVE_CKEN;
}

void sysTVOUT_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_TVE_CKEN;
}

void sysUSB_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_USB_CKEN;
}

void sysUSB_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_USB_CKEN;
}
void sysUSB2_enable(void)
{
    SYS_CTL0_EXT   |= SYS_CTL0_EXT_USB2_CKEN;
}

void sysUSB2_disable(void)
{
    SYS_CTL0_EXT   &= ~SYS_CTL0_EXT_USB2_CKEN;
}
void sysIIS_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_IIS_CKEN;
#if(AUDIO_OPTION == AUDIO_ADC_DAC)
#else
    GpioActFlashSelect &= ~(GPIO_ACT_FLASH_SPI | GPIO_GPIU_FrDISP_EN);
#endif
#if(HW_BOARD_OPTION  == MR9120_TX_OPCOM)
    GpioActFlashSelect |= GPIO_IISFrDISP_EN;
#endif
}

void sysTurnOnTVDAC(int YOn,int CbOn,int CrOn)
{
    u32 Temp;

    Temp= ((YOn&0x01)<<12) | ((CbOn&0x01)<<13) | ((CrOn&0x01)<<14);

#if( (CHIP_OPTION  == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1016B)  )
#if IDU_TV_DISABLE
    SYS_DBGIF_SEL |= 0x7000;  //Lucian: Turn off TV DAC(A,B,C)
#else
    SYS_DBGIF_SEL = (SYS_DBGIF_SEL | 0x07000) & (~Temp) ;  //Lucian: Turn on TV DAC(A,B,C)
#endif
#else
#if IDU_TV_DISABLE
    SYS_DBGIF_SEL &= (~0x07000);  //Lucian: Turn off TV DAC(A,B,C)
#else
    SYS_DBGIF_SEL = (SYS_DBGIF_SEL & (~0x07000) ) | Temp ;  //Lucian: Turn on TV DAC(A,B,C)
#endif
#endif

}

void sysIIS_disable(void)
{
    //Lucian: 由於應用上 可能有其他Task 正在用. Always on.
}
//----------------------------------------------------//
s32 sysUIReadFile(s32 dummy)
{
    if (sysReadFile() == 0)
        return 0;
    return 1;
}

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
s32 sysUIContinuousReadFile(s32 dummy)
{
    if (sysContinuousReadFile() == 0)
        return 0;
    return 1;
}
s32 sysUICopyFile(s32 dummy)
{
    if (sysCopyFile() == 0)
        return 0;
    return 1;
}

#endif

s32 sysP2PReadFile(s32 dummy)
{
    if (sys_P2P_Readfile() == 0)
        return 0;
    return 1;
}

s32 sysPlaybackCalendar(s32 dummy)
{
    u8  err;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList,sysPlaybackType,dummy);

#endif
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PYBK_SEARCH, OS_FLAG_SET, &err);
    return 1;
}

void sysNAND_Disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_NAND_CKEN;
#if(CHIP_OPTION == CHIP_PA9002D)
    GpioActFlashSelect = 0;
#else
    GpioActFlashSelect &= GPIO_ACT_FLASH_MASK;
#endif
}

void sysNAND_Enable(void)
{
    SYS_CTL0   |= SYS_CTL0_NAND_CKEN;
#if(CHIP_OPTION == CHIP_PA9002D)
    GpioActFlashSelect = GPIO_ACT_FLASH_SMC;
#else
    GpioActFlashSelect |= GPIO_ACT_FLASH_SMC;
#endif
}

void sysSPI_Disable(void)
{
    //gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_CLK, GPIO_ENA);
    //gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_MOSI, GPIO_ENA);
}

void sysSPI_Enable(void)
{
    //gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_CLK, GPIO_DISA);
    //gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_MOSI, GPIO_DISA);
}

void sysSD_Disable(void)
{
    //SYS_CTL0   &= ~SYS_CTL0_SD_CKEN;
}
void sysSD_Enable(void)
{
    //DEBUG_SYS("sysSD_Enable\n");
#if SD_CARD_DISABLE

#else
    SYS_CTL0   |= SYS_CTL0_SD_CKEN;
#endif
}
void sysWDT_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_WDT_CKEN;
}

void sysWDT_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_WDT_CKEN;

}

void sysSentUiKeyTilOK(int UiKey)
{
	for(;;)
    {
    	if(uiSentKeyToUi(UiKey))
    		break;
    	OSTimeDly(1);
    }
}

s32 sysBackLow_Syn_RF(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3)
{
    u8  i;

#if (RFIU_SUPPORT)
  #if PWIFI_SUPPORT

  #else
        //DEBUG_SYS("=====sysBackLow_Syn_RF=====\n");
        for(i=0; i<MULTI_CHANNEL_MAX; i++)
        {
        #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)

        #else
          #if RFIU_TEST

          #else
            uiSynRfConfig(i);
          #endif
        #endif
            if(rfiuRX_OpModeCmdRetry[i])
            {
                DEBUG_SYS("==Retry Opmode command==\n");
                rfiu_SetRXOpMode_1(i);
            }
        }
  #endif
#endif

    return 1;
}

#if (JPEG_DEBUG_ENA_9300||JPEG_DEBUG_ENA_9200)
extern bool Sequence_Pic_fromJpgRes(void);
extern bool Random_Pic_fromJpgRes(void);
extern bool FIX_Pic_fromJpgRes(void);
extern u32 jpeg_debug_mode;

s32 sysShowTimeOnOSD_VideoClip(s32 dummy)
{
    //osdDrawVideoTime();
    if(jpeg_debug_mode==1)
        Random_Pic_fromJpgRes();
    else if(jpeg_debug_mode==2)
        Sequence_Pic_fromJpgRes();
    else if(jpeg_debug_mode==3)
        FIX_Pic_fromJpgRes();
    return 1;
}
#else
s32 sysShowTimeOnOSD_VideoClip(s32 dummy)
{
    osdDrawVideoTime();
    return 1;
}
#endif



s32 sysDrawTimeOnVideoClip(s32 Param)
{
    //printf("sysDrawTimeOnVideoClip()\n");
    uiDrawTimeOnVideoClip(Param);
    return  1;
}

#if(RFIU_SUPPORT)

#if PWIFI_SUPPORT
s32 sysBack_RFI_RX_CH_Restart(s32 RFUnit)
{
    u8 err;
    int count;
    //----------------//
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    cpu_sr = cpu_sr;	// Avoid warning msg
#endif
    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==1)
        return 0;
    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_SYS("======RX_CH_Restart Begin:%d====\n",RFUnit);
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
    if( (sysRFRxInMainCHsel==RFUnit) && (sysCameraMode != SYS_CAMERA_MODE_PLAYBACK) )
    {
#if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
   #if IIS1_REPLACE_IIS5
        if(guiIISCh0PlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0PlayDMAId);
            guiIISCh0PlayDMAId = 0xFF;
        }
   #else
        if(guiIISCh4PlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh4PlayDMAId);
            guiIISCh4PlayDMAId = 0xFF;
        }
   #endif
#endif

#if RFIU_RX_AUDIO_RETURN
        if(guiIISCh0RecDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0RecDMAId);
            guiIISCh0RecDMAId = 0xFF;
        }
#endif
    }

    
    OSTimeDly(1);
    if(gRfiu_WrapDec_Sta[RFUnit] != RFI_WRAPDEC_TASK_RUNNING)
        gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=2;
    if(gRfiu_MpegDec_Sta[RFUnit] != RFI_MPEGDEC_TASK_RUNNING)
        gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=2;       
    
    count=0;
    while( (gRfiuUnitCntl[RFUnit].RX_Wrap_Stop !=2) || (gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop !=2)  )
    {
        OSTimeDly(1);
        DEBUG_SYS("@");
        count ++;
        if(count >10)
            break;
    }
    
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    OS_ENTER_CRITICAL();
    if(Motion_Error_ststus[RFUnit] == 1)
    {
        Record_flag[RFUnit] = 0;
        Motion_Error_ststus[RFUnit] = 0;
    }
    else
        Record_flag[RFUnit] = 1;
    OS_EXIT_CRITICAL();
#endif
    //DEBUG_SYS("\n-->1\n");
    rfiuRxVideoBufMngWriteIdx[RFUnit]=0;
    rfiuRxIIsSounBufMngWriteIdx[RFUnit]=0;

#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0xa55aaa55) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0xa55aaa55))
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = 0;
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = 0;
    }
    if(gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime > gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime;
    }
    else if (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime > gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime;
    }
    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0))
    {
        Video_totaltime[RFUnit] = 0;
        Audio_totaltime[RFUnit] = 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStart	= 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStop	= 1;
        Record_flag[RFUnit]   = 0;
        DEBUG_SYS("TX transfer error\n\n");
    }

    if(Lose_video_time[RFUnit] == 0)
        Lose_video_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] ;
    else
        Lose_video_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] );
    DEBUG_SYS("\n============(Vlose %ld,Tx %ld,RX %ld)==========\n",Lose_video_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,Video_totaltime[RFUnit]);
    Video_totaltime[RFUnit] = 0;

    if(Lose_audio_time[RFUnit] == 0)
        Lose_audio_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] ;
    else
        Lose_audio_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] );
    DEBUG_SYS("\n============(Alose %ld,Tx %ld,RX %ld)==========\n",Lose_audio_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime,Audio_totaltime[RFUnit]);
    Audio_totaltime[RFUnit] = 0;

    OS_ENTER_CRITICAL();
    if((Lose_video_time[RFUnit] <0)||(Lose_audio_time[RFUnit]<0))
    {
        pvcoRfiu[RFUnit]->sysCaptureVideoStart	= 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStop	= 1;
        Record_flag[RFUnit]   = 0;
        DEBUG_SYS("TX transfer error\n\n");
    }
    OS_EXIT_CRITICAL();

    if((pvcoRfiu[RFUnit]->sysCaptureVideoStart == 0) && (pvcoRfiu[RFUnit]->sysCaptureVideoStop == 1))
    {
        Lose_video_time[RFUnit] = 0;
        Lose_audio_time[RFUnit] = 0;
        DEBUG_SYS("Record stop\n\n");
        ASF_Restart[RFUnit] = 1;
    }
#endif

    //===============//
    //DEBUG_SYS("\n-->2\n");
#if( RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL )
    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
                
        }
        gRfiu_MpegDec_Sta[RFUnit]= RFI_MPEGDEC_TASK_SUSPEND;
        //DEBUG_SYS("\n-->1\n");
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_SUSPEND;
    }
    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_NONE;
        //DEBUG_SYS("\n-->2\n");
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }

    #if MULTI_CHANNEL_RF_RX_VIDEO_REC
      #if INSERT_NOSIGNAL_FRAME
            OS_ENTER_CRITICAL();
        #if(NOSIGNAL_MODE == 3)
            if(Record_flag[RFUnit] == 1)
                Record_flag[RFUnit] = 1;
        #else
            Record_flag[RFUnit] = 1;
        #endif
            OS_EXIT_CRITICAL();
      #endif
            RfRxVideoPackerSubTaskDestroy(RFUnit);
    #endif
    }

    //DEBUG_SYS("\n-->3\n");

    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if( (sysRFRxInMainCHsel == RFUnit) || ( (sysRFRxInPIPCHsel == RFUnit) && sysRFRXPIP_en ) )
        {
            switch(RFUnit)
            {
                case 0:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                    break;

                case 1:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                    break;

                case 2:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                    break;

                case 3:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                    break;
            #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                case 4:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                    break;

                case 5:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                    break;

                case 6:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                    break;

                case 7:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                    break;
            #endif
            }
            gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
        }

    }
    else if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)  )
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;

        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }
    else if( ( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA ) && (sysRFRxInMainCHsel==RFUnit) )
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }

    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }

    switch(RFUnit)
    {
        case 0:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
            break;

        case 1:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
            break;

        case 2:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_2, RFIU_WRAP_TASK_STACK_UNIT2, RFIU_WRAP_TASK_PRIORITY_UNIT2);
            break;

        case 3:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_3, RFIU_WRAP_TASK_STACK_UNIT3, RFIU_WRAP_TASK_PRIORITY_UNIT3);
            break;
    #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        case 4:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_4, RFIU_WRAP_TASK_STACK_UNIT4, RFIU_WRAP_TASK_PRIORITY_UNIT4);
            break;

        case 5:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_5, RFIU_WRAP_TASK_STACK_UNIT5, RFIU_WRAP_TASK_PRIORITY_UNIT5);
            break;

        case 6:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_6, RFIU_WRAP_TASK_STACK_UNIT6, RFIU_WRAP_TASK_PRIORITY_UNIT6);
            break;

        case 7:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_7, RFIU_WRAP_TASK_STACK_UNIT7, RFIU_WRAP_TASK_PRIORITY_UNIT7);
            break;
    #endif
    }
   #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    Record_flag[RFUnit] = 0;
   #endif
    gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_RUNNING;
 #endif
    //DEBUG_SYS("\n-->4\n");

    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;
    DEBUG_SYS("====RX_CH_Restart End:%d====\n\n",RFUnit);
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}
#else
s32 sysBack_RFI_RX_CH_Restart(s32 RFUnit)
{
    u8 err;
    int count;
    //----------------//
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    cpu_sr = cpu_sr;	// Avoid warning msg
#endif
    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==1)
        return 0;
    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_SYS("======RX_CH_Restart Begin:%d====\n",RFUnit);
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
    if( (sysRFRxInMainCHsel==RFUnit) && (sysCameraMode != SYS_CAMERA_MODE_PLAYBACK) )
    {
#if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
   #if IIS1_REPLACE_IIS5
        if(guiIISCh0PlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0PlayDMAId);
            guiIISCh0PlayDMAId = 0xFF;
        }
   #else
        if(guiIISCh4PlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh4PlayDMAId);
            guiIISCh4PlayDMAId = 0xFF;
        }
   #endif
#endif

#if RFIU_RX_AUDIO_RETURN
        if(guiIISCh0RecDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0RecDMAId);
            guiIISCh0RecDMAId = 0xFF;
        }
#endif
    }

    
    OSTimeDly(1);
    if(gRfiu_WrapDec_Sta[RFUnit] != RFI_WRAPDEC_TASK_RUNNING)
        gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=2;
    if(gRfiu_MpegDec_Sta[RFUnit] != RFI_MPEGDEC_TASK_RUNNING)
        gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=2;       
    
    count=0;
    while( (gRfiuUnitCntl[RFUnit].RX_Wrap_Stop !=2) || (gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop !=2)  )
    {
        OSTimeDly(1);
        DEBUG_SYS("@");
        count ++;
        if(count >10)
            break;
    }
    
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    OS_ENTER_CRITICAL();
    if(Motion_Error_ststus[RFUnit] == 1)
    {
        Record_flag[RFUnit] = 0;
        Motion_Error_ststus[RFUnit] = 0;
    }
    else
        Record_flag[RFUnit] = 1;
    OS_EXIT_CRITICAL();
#endif
    //DEBUG_SYS("\n-->1\n");
    rfiuRxVideoBufMngWriteIdx[RFUnit]=0;
    rfiuRxIIsSounBufMngWriteIdx[RFUnit]=0;

#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0xa55aaa55) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0xa55aaa55))
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = 0;
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = 0;
    }
    if(gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime > gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime;
    }
    else if (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime > gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime;
    }
    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0))
    {
        Video_totaltime[RFUnit] = 0;
        Audio_totaltime[RFUnit] = 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStart	= 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStop	= 1;
        Record_flag[RFUnit]   = 0;
        DEBUG_SYS("TX transfer error\n\n");
    }

    if(Lose_video_time[RFUnit] == 0)
        Lose_video_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] ;
    else
        Lose_video_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] );
    DEBUG_SYS("\n============(Vlose %ld,Tx %ld,RX %ld)==========\n",Lose_video_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,Video_totaltime[RFUnit]);
    Video_totaltime[RFUnit] = 0;

    if(Lose_audio_time[RFUnit] == 0)
        Lose_audio_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] ;
    else
        Lose_audio_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] );
    DEBUG_SYS("\n============(Alose %ld,Tx %ld,RX %ld)==========\n",Lose_audio_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime,Audio_totaltime[RFUnit]);
    Audio_totaltime[RFUnit] = 0;

    OS_ENTER_CRITICAL();
    if((Lose_video_time[RFUnit] <0)||(Lose_audio_time[RFUnit]<0))
    {
        pvcoRfiu[RFUnit]->sysCaptureVideoStart	= 0;
        pvcoRfiu[RFUnit]->sysCaptureVideoStop	= 1;
        Record_flag[RFUnit]   = 0;
        DEBUG_SYS("TX transfer error\n\n");
    }
    OS_EXIT_CRITICAL();

    if((pvcoRfiu[RFUnit]->sysCaptureVideoStart == 0) && (pvcoRfiu[RFUnit]->sysCaptureVideoStop == 1))
    {
        Lose_video_time[RFUnit] = 0;
        Lose_audio_time[RFUnit] = 0;
        DEBUG_SYS("Record stop\n\n");
        ASF_Restart[RFUnit] = 1;
    }
#endif

    //OSTimeDly(20);

#if 0
    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if(sysRFRxInMainCHsel==RFUnit)
        {
            Idu_ClearBuf(DISPLAY_BUF_NUM);
        }
        //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
        //Lucian:應該只清sub-window, not implement now.
        //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*4);
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
    {

    }
#endif
    //===============//
    //DEBUG_SYS("\n-->2\n");
#if(RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL)
    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_RUNNING)
    {
        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
        gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_SUSPEND;
    }
    if(gRfiu_WrapDec_Sta[1] == RFI_WRAPDEC_TASK_SUSPEND)
    {
        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
    }
    OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
    gRfiu_WrapDec_Sta[1] = RFI_WRAPDEC_TASK_RUNNING;

#elif (RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2)
    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        }
        gRfiu_MpegDec_Sta[RFUnit]= RFI_MPEGDEC_TASK_SUSPEND;
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        }
        gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_SUSPEND;
    }

    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_NONE;
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        }
 #if MULTI_CHANNEL_RF_RX_VIDEO_REC
    #if INSERT_NOSIGNAL_FRAME
        OS_ENTER_CRITICAL();
      #if(NOSIGNAL_MODE == 3)
        if(Record_flag[RFUnit] == 1)
            Record_flag[RFUnit] = 1;
      #else
        Record_flag[RFUnit] = 1;
      #endif
        OS_EXIT_CRITICAL();
    #endif
        RfRxVideoPackerSubTaskDestroy(RFUnit);
 #endif
    }
    switch(RFUnit)
    {
        case 0:
            OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
            break;

        case 1:
            OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
            break;

        case 2:
            OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_2, RFIU_WRAP_TASK_STACK_UNIT2, RFIU_WRAP_TASK_PRIORITY_UNIT2);
            break;

        case 3:
            OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_3, RFIU_WRAP_TASK_STACK_UNIT3, RFIU_WRAP_TASK_PRIORITY_UNIT3);
            break;
    }

    gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_RUNNING;
    gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;

#elif( RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL )
    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
                
        }
        gRfiu_MpegDec_Sta[RFUnit]= RFI_MPEGDEC_TASK_SUSPEND;
        //DEBUG_SYS("\n-->1\n");
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_RUNNING)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_SUSPEND;
    }
    if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_NONE;
        //DEBUG_SYS("\n-->2\n");
    }

    if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_SUSPEND)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }

    #if MULTI_CHANNEL_RF_RX_VIDEO_REC
      #if INSERT_NOSIGNAL_FRAME
            OS_ENTER_CRITICAL();
        #if(NOSIGNAL_MODE == 3)
            if(Record_flag[RFUnit] == 1)
                Record_flag[RFUnit] = 1;
        #else
            Record_flag[RFUnit] = 1;
        #endif
            OS_EXIT_CRITICAL();
      #endif
            RfRxVideoPackerSubTaskDestroy(RFUnit);
    #endif
    }

    //DEBUG_SYS("\n-->3\n");

    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if( (sysRFRxInMainCHsel == RFUnit) || ( (sysRFRxInPIPCHsel == RFUnit) && sysRFRXPIP_en ) )
        {
            switch(RFUnit)
            {
                case 0:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                    break;

                case 1:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                    break;

                case 2:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                    break;

                case 3:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                    break;
            #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                case 4:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                    break;

                case 5:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                    break;

                case 6:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                    break;

                case 7:
                    OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                    break;
            #endif
            }
            gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
        }

    }
    else if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)  )
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;

        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }
    else if( ( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA ) && (sysRFRxInMainCHsel==RFUnit) )
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }

    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
    {
        switch(RFUnit)
        {
            case 0:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
                break;

            case 1:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
                break;

            case 2:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
                break;

            case 3:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
                break;
        #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            case 4:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_4, RFIU_DEC_TASK_STACK_UNIT4, RFIU_DEC_TASK_PRIORITY_UNIT4);
                break;

            case 5:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_5, RFIU_DEC_TASK_STACK_UNIT5, RFIU_DEC_TASK_PRIORITY_UNIT5);
                break;

            case 6:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_6, RFIU_DEC_TASK_STACK_UNIT6, RFIU_DEC_TASK_PRIORITY_UNIT6);
                break;

            case 7:
                OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_7, RFIU_DEC_TASK_STACK_UNIT7, RFIU_DEC_TASK_PRIORITY_UNIT7);
                break;
        #endif
        }
        gRfiu_MpegDec_Sta[RFUnit] = RFI_MPEGDEC_TASK_RUNNING;
    }

    switch(RFUnit)
    {
        case 0:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
            break;

        case 1:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
            break;

        case 2:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_2, RFIU_WRAP_TASK_STACK_UNIT2, RFIU_WRAP_TASK_PRIORITY_UNIT2);
            break;

        case 3:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_3, RFIU_WRAP_TASK_STACK_UNIT3, RFIU_WRAP_TASK_PRIORITY_UNIT3);
            break;
    #if(RFI_TEST_8TX_1RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        case 4:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_4, RFIU_WRAP_TASK_STACK_UNIT4, RFIU_WRAP_TASK_PRIORITY_UNIT4);
            break;

        case 5:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_5, RFIU_WRAP_TASK_STACK_UNIT5, RFIU_WRAP_TASK_PRIORITY_UNIT5);
            break;

        case 6:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_6, RFIU_WRAP_TASK_STACK_UNIT6, RFIU_WRAP_TASK_PRIORITY_UNIT6);
            break;

        case 7:
            OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_7, RFIU_WRAP_TASK_STACK_UNIT7, RFIU_WRAP_TASK_PRIORITY_UNIT7);
            break;
    #endif
    }
   #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    Record_flag[RFUnit] = 0;
   #endif
    gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_RUNNING;
 #endif
    //DEBUG_SYS("\n-->4\n");

    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;
    DEBUG_SYS("====RX_CH_Restart End:%d====\n\n",RFUnit);
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}
#endif


s32 sysBack_RFI_TX_CH_Del(s32 RFUnit)
{
    switch(RFUnit)
    {
        case 0:
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            break;

        case 1:
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
            break;

        case 2:
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
            break;

        case 3:
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
            break;
    }
    return 1;
}

s32 sysBack_RFI_TX_CH_Create(s32 RFUnit)
{
    switch(RFUnit)
    {
        case 0:
            OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
            break;

        case 1:
            OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
            break;

        case 2:
            OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_2, RFIU_WRAP_TASK_STACK_UNIT2, RFIU_WRAP_TASK_PRIORITY_UNIT2);
            break;

        case 3:
            OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_3, RFIU_WRAP_TASK_STACK_UNIT3, RFIU_WRAP_TASK_PRIORITY_UNIT3);
            break;
    }
    gRfiu_WrapEnc_Sta[RFUnit] = RFI_WRAPENC_TASK_RUNNING;
    return 1;
}


#if TX_SNAPSHOT_SUPPORT
s32 sysTxCaptureImage_CIU5(s32 Setting)
{
    u8 *srcImgY,*srcImgUV;
    u32 sys_ctl0_status;
    int i;
    u32 JpegImagePixelCount;
    u32 primaryImageSize;
    u32 compressedBitsPerPixel;
    int OldSetting;
    u8 *pp;
    u32 TotalSize;
    u32 app4_size;
    u32 data_size;

    //---Point to necessary memory--//
    DEBUG_SYS("==sysTxCaptureImage_CIU5 Start==\n");
    //YUV422 -->PKBuf0
    exifPrimaryBitstream=PNBuf_sub5[2];
    exifThumbnailBitstream=exifPrimaryBitstream+MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT;
    exifFileInit();
    //-------//
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;

#if 1
    //Lucian: 8200 series 不須設定sensor, 只需靠CIU scaling 就可以.
    if(Setting != uiMenuVideoSizeSetting)
    {
        OldSetting=uiMenuVideoSizeSetting;
        uiMenuVideoSizeSetting=Setting;
        rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
        setSensorWinSize(0, SIUMODE_PREVIEW);
    }
    else
    {
        OldSetting=uiMenuVideoSizeSetting;
    }
#else
    OldSetting=uiMenuVideoSizeSetting;
#endif

#if 1
    ciu_5_OpMode = SIUMODE_PREVIEW;

    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,1280);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU5_OSD_EN,1280);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,1920);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,CIU5_OSD_EN,1920);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,896);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU5_OSD_EN,896);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU5_OSD_EN,640);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,352);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU5_OSD_EN,640);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,704);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU5_OSD_EN,704);
#endif
    }
    else
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU5_OSD_EN,640);
#endif
    }
    ipuPreview(0);
    siuPreview(0);

    //--------------------------------//
    while(ciu_idufrmcnt_ch5<2);
    ciu_5_Stop();
    ipuStop();
    siuStop();
#endif
    srcImgY  = PNBuf_sub5[1];
    srcImgUV    = srcImgY + PNBUF_SIZE_Y;

    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 80);
    for(i = 0; i < 6; i++)
    {
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            DEBUG_SYS("--->Snap 1280x720\n");
            exifSetImageResolution(1280, 720); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1280, 720, 1280);
            JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
            DEBUG_SYS("--->Snap 1920x1072\n");
            exifSetImageResolution(1920, 1072); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1920, 1072, 1920);
            JpegImagePixelCount = 1920 * 1072;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
            DEBUG_SYS("--->Snap 1600x896\n");
            exifSetImageResolution(1600, 896); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1600, 896, 1600);
            JpegImagePixelCount = 1600 * 896;   //GetJpegImagePixelCount();
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
            DEBUG_SYS("--->Snap 704x480\n");
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(704, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 480, 704);
                JpegImagePixelCount = 704 * 480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(704, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 576, 704);
                JpegImagePixelCount = 704 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
            DEBUG_SYS("--->Snap 720x480\n");
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(720, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 480, 720);
                JpegImagePixelCount = 720 * 480;    //GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(720, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 576, 720);
                JpegImagePixelCount = 720 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            DEBUG_SYS("--->Snap 640x352\n");
            exifSetImageResolution(640, 352); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 352, 640);
            JpegImagePixelCount = 640 * 352;    //GetJpegImagePixelCount();
        }
        else
        {
            DEBUG_SYS("--->Snap 640x480\n");
            exifSetImageResolution(640, 480); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 480, 640);
            JpegImagePixelCount = 640 * 480;    //GetJpegImagePixelCount();
        }
        primaryImageSize    = WaitJpegEncComplete();
        DEBUG_SYS("primaryImageSize=%d\n",primaryImageSize);

#if 1
        if(
            (exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
            (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9) &&
            (primaryImageSize < 1024*220)
        )  // JPEG bitstream good
        {
            break;
        }
        else
        {
            // JPEG bitstream fail, 用較低的品直再壓一次
            switch(i)
            {
                case 0:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(70)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 70);
                    break;
                case 1:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(60)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 60);
                    break;
                case 2:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(50)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);
                    break;
                case 3:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 4:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                    break;
                case 5:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                    break;

                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
            }
        }
#endif
    }
    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);

    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;

    //-----Packing JPEG bitstream------//
    //exifWriteFile(thumbnailImageSize, primaryImageSize, 1);
    pp=sysRFTXImgData;
    TotalSize=0;
    memcpy(pp, (unsigned char*)&exifThumbnailImage, 2);
    pp +=2;
    TotalSize += 2;
    // Write dummy to APP4 for align address
    app4_size=(TotalSize+ exifPrimaryImageHeaderSize);
    app4_size &= 0x0F ;
    if(app4_size!=0)
    {
        app4_size=( TotalSize+ exifPrimaryImageHeaderSize + sizeof(DEF_APP4PREFIX) ) & 0x0F ;
        if(app4_size==0)    // Add marker length
            app4_size=0x10;
        else
            app4_size=0x10-app4_size;
        exifAPP4Prefix.APP4Marker= EXIF_MARKER_APP4;
        data_size= app4_size+2;
        exifAPP4Prefix.APP4Size= (data_size<<8)|(data_size>>8);
        memcpy(pp, (unsigned char*)&exifAPP4Prefix, sizeof(DEF_APP4PREFIX));
        pp += sizeof(DEF_APP4PREFIX);
        TotalSize += sizeof(DEF_APP4PREFIX);
        memcpy(pp, (unsigned char*)PKBuf, app4_size);//Insert Dummy data
        pp += app4_size;
        TotalSize += app4_size;
    }

    /* write primary to file */
    memcpy(pp, (unsigned char*)&exifPrimaryImage, exifPrimaryImageHeaderSize);
    pp += exifPrimaryImageHeaderSize;
    TotalSize += exifPrimaryImageHeaderSize;

    memcpy(pp, (unsigned char*)exifPrimaryImage.bitStream, primaryImageSize);
    pp += primaryImageSize;
    TotalSize += primaryImageSize;

    sysRFTXDataSize=TotalSize;
    //---------------------------------//
    ciu_5_OpMode = SIUMODE_PREVIEW;
    sysCameraMode = SIUMODE_PREVIEW;
    uiMenuVideoSizeSetting=OldSetting;
    DEBUG_SYS("==sysTxCaptureImage_CIU5 End:%d==\n",TotalSize);
}
#endif

#if RX_SNAPSHOT_SUPPORT
s32 sysBack_RFI_RX_DataSave(s32 RFUnit)
{
    DEBUG_SYS("==sysBack_RFI_RX_DataSave==\n");
    //----------------------------------------//
#if 1
    dcfWriteNextPhotoFile(RFUnit,rfiuRxDataBufMng[RFUnit].buffer,rfiuRxDataBufMng[RFUnit].size);
#else
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_JPG, RFUnit)) == NULL)
        return 0;
    if (dcfWrite(pFile, rfiuRxDataBufMng[RFUnit].buffer, rfiuRxDataBufMng[RFUnit].size , &ret) == 0)
    {
        dcfCloseFileByIdx(pFile, RFUnit, &tmp);
        return 0;
    }
    dcfCloseFileByIdx(pFile, RFUnit, &tmp);
#endif
    //----------------------------------------//
    rfiuRxDataBufMng[RFUnit].buffer=rfiuRxDataBuf[RFUnit];
    rfiuRxDataBufMng[RFUnit].size =0;
    return 1;
}
#endif

#endif

#if MAKE_SPI_BIN
/*

Routine Description:

	Make Bin. Copy contents from spi to sd card.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s8 sysMakeSpiBin(void)
{

    u8* DataAddr = (u8*)((DRAM_MEMORY_END + 4) - SPI_MAX_CODE_SIZE);
    u8*	DataAddrToSD = DataAddr;
    u32	unSize;
    s32 i, j, x;
    s32	k;
    u32	unAddr;
    u32 WriteSize;
    u32	ReadMaxSize;
    u8  BYTE_SWAP[4];

    FS_FILE* pFile;
    u8  tmp;

    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
        return 0;

    if ((pFile = dcfOpen("\\spi.bin", "w")) == NULL)
    {
        DEBUG_ISP("Error: dcf open error!\n");
        return 0;
    }

    unAddr = 0;		/* read spi from addr = 0 */
//    03FFFFF
//    for (i=0x200000; i>0; i-=unSize)
    for (i=spiTotalSize; i>0; i-=unSize)
    {
        sysSD_Disable();
        sysSPI_Enable();


        unSize = 0;
        ReadMaxSize = i>SPI_MAX_CODE_SIZE ? SPI_MAX_CODE_SIZE: i;

        /* copy from flash */
        for (k=ReadMaxSize; k>0; k-=SPI_MAX_BUF_SIZE)
        {

            if (!spiRead(DataAddr, unAddr, SPI_MAX_BUF_SIZE))
            {
                DEBUG_SPI("Error: spiRead Error\n");
                return 0;
            }

            if ((i==0)&&(k==ReadMaxSize))
                DEBUG_SPI("data = %#x\n", *(u32*)DataAddr);

            unAddr += SPI_MAX_BUF_SIZE;
            DataAddr += SPI_MAX_BUF_SIZE;
            unSize += SPI_MAX_BUF_SIZE;
        }

        sysSPI_Disable();
        sysSD_Enable();

        for(x = 0; x < unSize; x+=4)
        {
            BYTE_SWAP[0] = DataAddrToSD[x];
            BYTE_SWAP[1] = DataAddrToSD[x+1];
            BYTE_SWAP[2] = DataAddrToSD[x+2];
            BYTE_SWAP[3] = DataAddrToSD[x+3];

            DataAddrToSD[x]   = BYTE_SWAP[3];
            DataAddrToSD[x+1] = BYTE_SWAP[2];
            DataAddrToSD[x+2] = BYTE_SWAP[1];
            DataAddrToSD[x+3] = BYTE_SWAP[0];
        }

        /* write to sd card */
        dcfWrite(pFile, DataAddrToSD, unSize, &WriteSize);

        /* reset the related values */
        DataAddr = DataAddrToSD;	/* reset to start addr */

    }
    dcfClose(pFile, &tmp);
    return 1;
}
#endif

s32 sysDrawTimeOnCapture(s32 dummy)
{
    return 1;
}

s32 sysBack_Draw_Battery(s32 level)
{
#if (UI_VERSION == UI_VERSION_TX)
#else
    uiOsdDrawBattery(level);
#endif
    return 1;
}

#if ENABLE_DOOR_BELL
SYS_DOOR_STATE sysGetDoorBellState(void)
{
    return gSysDoorBellState;
}

void sysSetDoorBellState(SYS_DOOR_STATE state)
{
    gSysDoorBellState = state;
}
#endif

#if RFIU_RX_WAKEUP_TX_SCHEME 
void sysSetBTCWakeTime(u16 second)
{
    gSysWakeSecond = second;
}

u16 sysGetBTCWakeTime(void)
{
    return gSysWakeSecond;
}

void sysSetSyncBTCTime(u16 hour)
{
    gSysSyncBTCTime = hour;
}

u16 sysGetSyncBTCTime(void)
{
    return gSysSyncBTCTime;
}

void sysResetBTCCheckLev(void)
{
    gSyscheckBTCbatLev = FALSE;
}

u8 sysGetBTCTimer(u8 cam)
{
    return gSysBatCamTimer[cam];
}

void sysSetBTCTimer(u8 cam,u16 second)
{
    gSysBatCamTimer[cam] = second;
}

void sysSetBTCWakeStatus(u8 channel_id,u8 status, bool set)
{
    //DEBUG_SYS("\n==sysSetBTCWakeStatus(%d,%d,%d)==\n",channel_id,status,set);
    if (set)
        gSysBTCWakeStatus[channel_id] |= status;
    else
        gSysBTCWakeStatus[channel_id] &= ~status;
}

u8 sysGetBTCWakeStatus(u8 channel_id)
{
    return gSysBTCWakeStatus[channel_id];
}

void sysBatteryCam_sleep(u8 channel_id)
{
    if (rfiuCamSleepCmd(channel_id) == 0) 
       gSysSetRfBTCSleep[channel_id] = SYS_SET_RF_BUSY;
    else
       gSysSetRfBTCSleep[channel_id] = SYS_SET_RF_OK;
}

 /** Queries if a battery camera is sleeping. */
bool sysBatteryCam_isSleeping(u8 channel_id)
{
    if (gRfiu_Op_Sta[channel_id] != RFIU_RX_STA_LINK_BROKEN)
        return FALSE;
    else    
        return TRUE;
}

/** Wakes up a battery camera. */
void sysBatteryCam_wake(u8 channel_id,u16 seconds)
{

    gRfiuUnitCntl[channel_id].WakeUpTxEn = 1;
#if MESUARE_BTCWAKEUP_TIME
    rfiuBTCWakeTime[channel_id] = guiSysTimerCnt;
#endif
    sysSetBTCTimer(channel_id, seconds);
    sysSetBTCWakeStatus(channel_id, SYS_BTC_WAKEUP_MANUAL, TRUE);
    //DEBUG_GREEN("Wake up battery camera: %d\n", channel_id);
}

/** Stops a waked battery camera. */
u8 sysBatteryCam_stop(u8 channel_id)
{
    u8 result = SYS_SET_RF_OK;
    
    if (gRfiuUnitCntl[channel_id].WakeUpTxEn == 1)
       gRfiuUnitCntl[channel_id].WakeUpTxEn = 0;
    else
    {
       sysBatteryCam_sleep(channel_id); 
       result = gSysSetRfBTCSleep[channel_id];
    }
    
    return result;
}

/** Stops battery cameras if time out. */
void sysBatteryCam_stopIfDone(void)
{
    int i;

    if (Main_Init_Ready == 0)
        return;
    
    for (i = 0; i < MULTI_CHANNEL_MAX; ++i) 
    {
        if (gSysBatCamTimer[i] > 0)
        {
            gSysBatCamTimer[i]--;
            //DEBUG_GREEN("Cam %d Walk up %d\n", i, gSysBatCamTimer[i]);

            if (gSysBatCamTimer[i] == 0)
            {
                if (sysBatteryCam_stop(i) == SYS_SET_RF_OK)
                {
                    sysSetBTCWakeStatus(i, SYS_BTC_WAKEUP_MANUAL, FALSE);
                    sysSetBTCWakeStatus(i, SYS_BTC_WAKEUP_APP, FALSE);		//Add by Paul for clear state when app triggered, 180508
                #if (UI_BAT_SUPPORT)    
                    uiOsdDrawCameraBatteryLevel(i, UI_OSD_DRAW);
                #endif
                }
                DEBUG_SYS("Timer is done for battery camera: %d\n", i);
            }
        }
        else
        {
            //DEBUG_GREEN("Cam %d Walkup %d WakeStatus %d\n", i, gSysBatCamTimer[i], gSysBTCWakeStatus[i]);
            if (gSysBTCWakeStatus[i] != SYS_BTC_WAKEUP_NO)
            {
                gSysBTCWakeStatus[i] = SYS_BTC_WAKEUP_NO;
            }
        }

    }
}

void sysStartBatteryCam(u8 camID)
{
    if (!(SysCamOnOffFlag & (1<<camID)))
        return;
  #if (UI_BAT_SUPPORT)  
    uiOsdDrawCameraBatteryLevel(camID, UI_OSD_CLEAR);
  #endif
    sysBatteryCam_wake(camID, sysGetBTCWakeTime());
}

void sysCheckBTCBatteryLevel(void)
{
    RTC_DATE_TIME   localTime;
    u8 i;
    u8 nightClk;
    static u8 count = 0;
    u8 lowBatCam = 0;
    
    nightClk = gSysSyncBTCTime + 12;

    RTC_Get_Time(&localTime);
    if ((((localTime.hour == gSysSyncBTCTime) && (localTime.min == 0)) || ((localTime.hour == nightClk) && (localTime.min == 0))) && (gSyscheckBTCbatLev == FALSE))
    {
        count = SYNC_BTC_LEVEL_COUNTDOWN;
        gSyscheckBTCbatLev = TRUE;
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if (!(SysCamOnOffFlag & (1<<i)))
                continue;
            
            if ((gRfiuUnitCntl[i].RFpara.BateryCam_support == 1) && (sysBatteryCam_isSleeping(i)))
            {
              #if (UI_BAT_SUPPORT)  
                uiOsdDrawCameraBatteryLevel(i, UI_OSD_CLEAR);
              #endif
                sysBatteryCam_wake(i, sysGetBTCWakeTime());
            }
        }
    }
    else if (!(((localTime.hour == gSysSyncBTCTime) && (localTime.min == 0)) || ((localTime.hour == nightClk) && (localTime.min == 0))))
    {
        gSyscheckBTCbatLev = FALSE;
    }
    else
    {
        if ((gSyscheckBTCbatLev) && (count > 0))
        {
            if (count == 1)
            {
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if (!(SysCamOnOffFlag & (1<<i)))
                        continue;
                    
                    if ((gRfiuUnitCntl[i].RFpara.BateryCam_support == 1) && (gRfiuUnitCntl[i].RFpara.TxBatteryLev < 1))
                    {
                        lowBatCam |= (1<<i);  
                    }
                }
                if (lowBatCam != 0)
                {
                    #if(NIC_SUPPORT)
                    sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, lowBatCam, EVENT_LOWBATTERY);
                    #endif
                }
            }
            count--;
        }

    }
    
}
#endif

s32 sysBack_Check_UI(s32 level)
{
#if SD_TASK_INSTALL_FLOW_SUPPORT
	if(sysGetOneUIKeyRetry())
	{
		if(uiSentKeyToUi(UI_KEY_DEVCD) == 1)
			sysReleaseUIKeyRetry();
	}
#else
	switch(sysGetOneUIKeyRetry())
	{
		case SYS_V_STORAGE_SDC:
			if(uiSentKeyToUi(UI_KEY_SDCD) == 1)
			{
				sysReleaseUIKeyRetry();
			}
			break;
  #if ( ((SW_APPLICATION_OPTION != MR9300_RFDVR_RX1RX2) && (SW_APPLICATION_OPTION != MR9300_NETBOX_RX1RX2)) && USB_HOST_MASS_SUPPORT)
		case SYS_V_STORAGE_USBMASS:
			if(uiSentKeyToUi(UI_KEY_USBCD) == 1)
			{
				sysReleaseUIKeyRetry();
			}
			break;
  #endif
		default: break;
	}
#endif
    uiFlowRunPerSec(); //Lsk 090429 : appointment video recording for ebell
    #if RFIU_RX_WAKEUP_TX_SCHEME 
    sysBatteryCam_stopIfDone();
    sysCheckBTCBatteryLevel();
    #endif
    
    return 1;
}

#if (NET_STATUS_POLLING && NIC_SUPPORT)
s32 sysBack_Draw_NET_Icon(s32 level)
{
#if ICOMMWIFI_SUPPORT

#else
    phyNETRunPerSec(); //Sean 20160629 : Check Network Status by polling.
#endif    
    return 1;
}
#endif

#if(HOME_RF_SUPPORT)
s32 sysBack_Check_HOMERF(s32 level)
{
    homeRFRunPerSec();
    return 1;
}
#endif





/******************************************
  Check video in source : TV or Sensor
*******************************************/
s32 sysBack_Check_VideoinSource(s32 dummy)
{

    return 1;
}



s32 sysBack_Draw_BitRate(s32 value)
{
  #if(SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1)

  #else
    uiOsdDrawBitRate(value);
  #endif
  
    return 1;
}

s32 sysBack_Draw_FrameRate(s32 value)
{
#if(SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1)

#else
    uiOsdDrawFrameRate(value);
#endif
    return 1;
}

s32 sysBack_Draw_OSDString(s32 value)
{
    if (value == MSG_ASCII_STR)
    {
        uiOSDASCIIStringByColor(sysBackOsdString, 60 , 120, OSD_Blk2 , 0xc6, 0x00);
    }
    else
    {
        uiOSDMultiLanguageStrCenter((MSG_SRTIDX) value, OSD_Blk2, 0xc6, 0x00);
    }
    return 1;
}

s32 sysBack_Draw_SD_Icon(s32 value)
{
    osdDrawSDIcon(value);
    return 1;
}
/*

Routine Description:

    Set a ui key of rec.

Arguments:



Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSetUiKey(s32 snKey)
{

    UIKey = snKey;

    OSSemPost(uiSemEvt);

    return 1;
}

u8 sysCheckSDCD(void)
{
    u8 SDCD;

#if SDC_SDCD_ENA
    SDCD = (SdcStat & 0x04)>>2;
#else
    SDCD = SDC_CD_IN;
#endif

    return SDCD;
}

#if USB_HOST_MASS_SUPPORT
u8 sysCheckUSBCD(void)
{
    if (HCPortSC & 0x01)
    {
#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
        gUSBDevOn= 1;
#endif        
        return USB_CD_IN;
    }
    DEBUG_UHOST("sysCheckUSBCD: no usb device \n");
    return USB_CD_OFF;
}
#else
u8 sysCheckUSBCD(void)
{
    return USB_CD_OFF;
}
#endif

s32 sysUpgradeFW(s32 snKey)
{
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
	return ispFirmwareUpdateFlow(1);
#else
	s32 isp_return;
    int tmpVal;
    u8  err;
    
    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
        tmpVal = SYS_I_STORAGE_MAIN;
    else
        tmpVal = SYS_I_STORAGE_BACKUP;
    if(sysGetStorageStatus(tmpVal) == SYS_V_STORAGE_NREADY)
    {
        DEBUG_SYS("uiMenuSet_UpgradeFW No Sd Card\n");
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
        return 0;
    }

    sysSD_Enable();
    /*disable watch dog when update firmware*/
    sysDeadLockMonitor_OFF();

    //--------------------------------//
    isp_return=ispUpdateAllload();//usb name boot
    if(isp_return ==0)
    {
        isp_return=ispUpdatebootload();
        if(isp_return ==0)
            isp_return=ispUpdate(1);		 // Check whether spiFW.bin exists ot not. If exist then update
    }
    //-------------------------------//
    /*enable watch dog when update firmware finish*/
    sysDeadLockMonitor_ON();
    if (isp_return == 0)
        OSMboxPost(general_MboxEvt, "FAIL");
    else
        OSMboxPost(general_MboxEvt, "PASS");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
    
    return 1;
#endif
}


void sysPowerOffDirect(void)
{
#if(HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
    gpioSetLevel(GPIO_GROUP_POWERKEEP, GPIO_BIT_POWERKEEP, 0);
#endif
}

#if ICOMMWIFI_SUPPORT
s32 sysTX9211_EnterWifi(s32 P2PQuailty)
{
    
    //----------------------------------//  
    rfiu_TX_P2pVideoQuality=P2PQuailty;

	#if ICOMMWIFI_SUPPORT
	iComm_WIFI_Status();
    rfiu_TX_WifiCHNum= (2407+Icomm_WifiCHNum*5-2408+2)/4;  //對應到2408~2468MHz
	#endif
    if(rfiu_TX_WifiCHNum>15)
        rfiu_TX_WifiCHNum=15;
    
    if(rfiu_TX_WifiCHNum<0)
        rfiu_TX_WifiCHNum=0;
    
    //----------------------------------//
    //-----//
    DEBUG_SYS("==sysTX9211_EnterWifi:%d==\n",P2PQuailty);
    sys9211TXWifiStat=MR9211_ENTER_WIFI;
    rfiu_TX_P2pVideoQuality=P2PQuailty;
    //-----//
    return 1;
}

s32 sysTX9211_LeaveWifi(s32 dummy)
{

    //----------------------------------//  
    
    DEBUG_SYS("==sysTX9211_LeaveWifi==\n");
    
    sys9211TXWifiStat=MR9211_QUIT_WIFI;

    //----------------------------------//
    
    return 1;
}
#endif



#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#if(USE_DBGPRINT_PTR)

static u32 dbgprint_flag = DBGPRINT_FLAG_NONE;

u32 _dbgprint_flag_get(void)
{
    return dbgprint_flag;
}

void _dbgprint_flag_set(u32 flag)
{
    dbgprint_flag = flag;
}


#endif //#if(USE_DBGPRINT_PTR)
#endif

//------------------------------------------//
#if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) ||\
     (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) ||\
     (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1)  || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
     (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) ||\
     (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8110_BABYMONITOR) ||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1)||\
     (SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))

#if(RFIU_SUPPORT)
s32 sysBack_RFI_TX_Change_Reso(s32 Setting)
{}

s32 sysBack_RFI_TX_SnapShot(s32 Setting)
{}
#endif


s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor,u8 BurstNum,u8 ScalingFactor) /*BJ 0530 S*/
{}

/*

Routine Description:

    Preview initialization.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPreviewInit(s32 dummy)
{}

/*

Routine Description:

    Preview zoom.

Arguments:

    zoomfactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPreviewReset(s32 zoomFactor)
{}


s32 sysCiu_1_PreviewReset(s32 zoomFactor)
{}


s32 sysCiu_2_PreviewReset(s32 zoomFactor)
{}

s32 sysCiu_3_PreviewReset(s32 zoomFactor)
{}

s32 sysCiu_4_PreviewReset(s32 zoomFactor)
{}

s32 sysCiu_5_PreviewReset(s32 zoomFactor)
{}

s32 sysPreviewZoomInOut(s32 zoomFactor)
{}


s32 sysTVInChannelChange_Preview(s32 zoomFactor)
{}

s32 sysVideoZoomInOut(s32 zoomFactor)
{}


s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor)
{}

s32 sysSnapshot_OnPreview(s32 ScalingFactor)
{}


s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor)
{}


s32 sysCaptureImage_One_OnPreview_CIU1(s32 ZoomFactor)
{}

s32 sysCaptureImage_One_OnPreview_CIU2(s32 ZoomFactor)
{}

s32 sysCaptureImage_One_OnPreview420_CIU1(s32 ZoomFactor)
{}

s32 sysCaptureImage_One_OnPreview420_CIU2(s32 ZoomFactor)
{}


s32 sysCaptureImage_One_OnPreview420_CIU5(s32 ZoomFactor)
{}

s32 sysVideoCaptureRoot(s32 dummy)
{}

s32 sysCaptureVideo_Init(void) /*BJ 0530 S*/
{}
s32 sysSensorFlip(s32 dummy)
{}
s32 sysCaptureVideo(s32 ZoomFactor) /*BJ 0530 S*/
{}

s32 sysBack_Check_TVinFormat(s32 dummy)
{

}

s32 sysBack_Set_Sensor_Color(s32 dummy)
{

}
u32 getTVinFormat()
{
    return 0;
}

s32 sysBack_ScheduleMode(s32 dummy)
{
    u8 tmpSch, i, mode;
    //u8 j,k;
    RTC_DATE_TIME   localTime;

    //static u8 CurWeek,lastSchedule = 0xFF;

    if ((sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)||((SysOverwriteFlag == FALSE) && (MemoryFullFlag == TRUE)))
    {
        //DEBUG_SYS("Disk full!!!\n");
        return 0;
    }
#if 0
    if((SYSSetScheduleTime == 0) && (Main_Init_Ready == 1))
    {
        printf("SYSSetScheduleTime\n\n\n\n");
        SYSSetScheduleTime = 1;
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            for(j = 0; j < 7; j++) // day
            {
                for(k = 0; k < 48; k++) //SYSCheckSchedule
                {
                    mode = Menu_RecordingScheduleTable_QueryType(j,k,i);
                    SDK_Record_SaveScheduleTime(mode,i,j,k);
                }
            }
        }
    }
#endif
    RTC_Get_Time(&localTime);
    tmpSch = localTime.hour*2+localTime.min/30;
    /*
        if (SYSCheckScheduleGetTime == 0)
        {
            SYSCheckScheduleGetTime = 10;
            RTC_Get_Time(&localTime);
            tmpSch = localTime.hour*2+localTime.min/30;
            if ((tmpSch < SYSCheckSchedule)||(SYSCheckSchedule == 48))
            {
                CurWeek = RTC_Get_Week(&localTime);
                if (SYSCheckSchedule == 48)
                    lastSchedule = 0xFF;
            }
            SYSCheckSchedule = tmpSch;
        }
        else
        {
            SYSCheckScheduleGetTime--;
            return;
        }

        if (lastSchedule == SYSCheckSchedule)  // same status return.
            return;
    */
    if(SYSSetScheduleTime == 0)
        return 0;
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        SDK_Record_ScheduleMode(mode, i, localTime.week, tmpSch);  // asf check schedule mode SDK.
    }

    return 1;
}

void sysSetOutputMode(u8 ucMode) //uiSetOutputMode
{
    if(ucMode == SYS_OUTMODE_TV)
    {
    #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
         (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
         (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
         (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
         (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        /* OSC 8M
        AdjustIIS5FreqData(SYS_OUTMODE_TV); //set audio clock and 32k data
        i2cWrite_WM8940(0x02, 0x11);
        i2cWrite_WM8940(0x06, 0x0d);
        i2cWrite_WM8940(0x2f, 0x07);
        i2cWrite_WM8940(0x32, 0x20);
        */
        // PLL 27M 
        AdjustIIS5FreqData(SYS_OUTMODE_TV); //set audio clock and 44.1k 16bit data
          #if (AUDIO_DEVICE == AUDIO_IIS_WM8940)
            WM8940_SpeakerMute();
            #if (UI_BOOT_FROM_PANEL == 0)
            WM8940_PLL_AuxPlay_ALL();
            #else
            WM8940_PLL_LineIn();
            #endif
          #endif
    #endif
        
        //--Back Light off---//
    #if (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P)
        //D96R plus panel always on
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
    #else
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, GPIO_LEVEL_LO);
    #endif
        OSTimeDly(2);
        sysTVOutOnFlag=GLB_ENA;
        IIS_SwitchPanel2HDMI();
        DEBUG_SYS("SET to TV-OUT!!\n\r");
        IduIntCtrl = 0x00000000;    //IDU interrupt control *
        IduWinCtrl &= (~0x0f);
        OSTimeDly(2);
        IduEna = 0;

        TvOutMode = 0;
        //---Lucian: 設定 CCIR656 Pannel,讓TV out與 656-Panel 同時輸出---//
   #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);

        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
      #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
      #else //656
        IduMpuCmd = 0x00006002;
      #endif
        iduWaitCmdBusy();
   #endif

        ResetisuPanel();

#if TVOUT_DYNAMIC

    #if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
    if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
    {
        if(isCap1920x1080I() == 1)
            TvOutMode =SYS_TV_OUT_FHD1080I60;
        else
            TvOutMode =SYS_TV_OUT_HD720P60;
        DEBUG_SYS("FHD isCap1920x1080I() = %d\n", isCap1920x1080I());
    }
    else
        TvOutMode =SYS_TV_OUT_HD720P60;
    #endif

#else
    #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60)
        TvOutMode=SYS_TV_OUT_HD720P60;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60_37M)
        TvOutMode=SYS_TV_OUT_HD720P60_37M;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P30)
        TvOutMode=SYS_TV_OUT_HD720P30;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P25)
        TvOutMode=SYS_TV_OUT_HD720P25;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I60)
        TvOutMode=SYS_TV_OUT_FHD1080I60;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080P30)
        TvOutMode=SYS_TV_OUT_FHD1080P30;
    #else
    #endif
#endif
        SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL
        Idu_ClearBuf(6);
        TV_init(TvOutMode,TVOUT_OSDx1_VDOx1,SYS_RUN_PLAYBACK);
        uiOutputRedirection();
    #if( (LCM_OPTION ==LCM_HX8224_601) || (LCM_OPTION ==LCM_HX8224_656))

    #else
        //IduDispCfg &= (~0x00400000);  //Lucian: Turn off digital TV out.
    #endif

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
    HDMITX_AudioEnable(TRUE); // switch audio format between HDMI and Panel, should disable HDMI audio, 否則panel mode leads HDMI audio overflow
#endif
    }
    else
    {
#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
    HDMITX_AudioEnable(FALSE);
    //HDMITX_DisableVideoOutput();
#endif
        DEBUG_SYS("SET to PANEL-OUT!!\n\r");
        sysTVOutOnFlag=GLB_DISA;
        tvTVE_INTC   =0x00000000;    //TV interrupt control *
        tvFRAME_CTL &= (~0x0f);
        IduOsdL1Ctrl  &= ~(0x00000010);
		IduOsdL2Ctrl  &= ~(0x00000010);
        OSTimeDly(2);
        IduEna = 0;
		iduRst();
        
        //---Lucian: 設定 CCIR656 Pannel---//
   #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
        IduMpuCmd = 0x00006000;
        iduWaitCmdBusy();
   #elif((LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
      #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
      #else //656
        IduMpuCmd = 0x00006002;
      #endif
        iduWaitCmdBusy();
   #endif

        IduYCbCr2R=0x002C0020;
        IduYCbCr2G=0x00968B20;
        IduYCbCr2B=0x00003820;

        //iduTVColorbar_onoff(1); 

        ResetisuPanel();

        IDU_Init(0 , 1);
        uiOutputRedirection();

    #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
         (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
         (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
        if(isHDMIpluged() != 1 )
        {
            gpioSetLevel(GPIO_GROUP_HDMI_RST, GPIO_BIT_HDMI_RST, GPIO_LEVEL_LO);
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_HDMI_RST, GPIO_BIT_HDMI_RST, GPIO_LEVEL_HI);
        }
    #endif

    #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
         (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
         (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
         (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
         (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        AdjustIIS5FreqData(SYS_OUTMODE_PANEL); //set audio clock and 16kHz
      #if (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        WM8940_AdjustSpeakerVolume(sysVolumnControl);
        WM8940_MicRec_AuxPlay_Switch2panel();
      #endif
    #endif
        IIS_SwitchPanel2HDMI();

        //----Turn on back light---//
    #if (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P)
        //D96R plus panel always on
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_HI);
    #else
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, GPIO_LEVEL_HI);
        OSTimeDly(4);
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_HI);
    #endif
    }
}

void sysPlaybackSetOutputMode(u8 ucMode) //uiSetOutputMode add playback function
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    if(ucMode == SYS_OUTMODE_TV)
    {

        //--Back Light off---//
        #if (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P)
        //D96R plus panel always on
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
        #else
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, GPIO_LEVEL_LO);
        #endif
        
        OSTimeDly(2);
        sysTVOutOnFlag=GLB_ENA;
        DEBUG_SYS("Playback SET to TV-OUT!!\n\r");
        IduIntCtrl = 0x00000000;    //IDU interrupt control *
        IduWinCtrl &= (~0x0f);
        OSTimeDly(2);
        IduEna = 0;

        TvOutMode = 0;
        //---Lucian: 設定 CCIR656 Pannel,讓TV out與 656-Panel 同時輸出---//
      #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);

        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
      #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
      #else //656
        IduMpuCmd = 0x00006002;
      #endif
        iduWaitCmdBusy();
      #endif

        ResetisuPanel();

      #if TVOUT_DYNAMIC
        if(asfVopWidth == 1920)
        {
            if(isCap1920x1080I() == 1)
                TvOutMode =SYS_TV_OUT_FHD1080I60;
            else
                TvOutMode =SYS_TV_OUT_HD720P60;
        }
        else if(asfVopWidth == 1280)
            TvOutMode =SYS_TV_OUT_HD720P60;
      #else
       #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60)
        TvOutMode=SYS_TV_OUT_HD720P60;
       #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60_37M)
        TvOutMode=SYS_TV_OUT_HD720P60_37M;
       #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P30)
        TvOutMode=SYS_TV_OUT_HD720P30;
       #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P25)
        TvOutMode=SYS_TV_OUT_HD720P25;
       #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I60)
        TvOutMode=SYS_TV_OUT_FHD1080I60;
       #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080P30)
        TvOutMode=SYS_TV_OUT_FHD1080P30;
       #else
       #endif
      #endif
        SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL
        Idu_ClearBuf(6);
        TV_init(TvOutMode,TVOUT_OSDx1_VDOx1,SYS_RUN_PLAYBACK);

        asfSwitchSource(SYS_OUTMODE_TV);
        
        uiOutputRedirection();
      #if( (LCM_OPTION ==LCM_HX8224_601) || (LCM_OPTION ==LCM_HX8224_656))

      #else
        //IduDispCfg &= (~0x00400000);  //Lucian: Turn off digital TV out.
      #endif
        OSTimeDly(1);
      #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
           (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
           (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
           (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
           (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        AdjustIIS5FreqData(SYS_OUTMODE_TV); //set audio clock and 44.1k 16bit data
       #if (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        WM8940_SpeakerMute();
        WM8940_PLL_LineIn();
       #endif
      #endif
        IIS_SwitchPanel2HDMI();
      #if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
        HDMITX_AudioEnable(TRUE); // switch audio format between HDMI and Panel, should disable HDMI audio, 否則panel mode leads HDMI audio overflow
      #endif
        if(asfVopWidth == 1280)
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],1276,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
        //else if (asfVopWidth == 1920)
        //    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],1916,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
    }
    else
    {
        DEBUG_SYS("Playback SET to PANEL-OUT!!\n\r");
        sysTVOutOnFlag=GLB_DISA;
        tvTVE_INTC   =0x00000000;    //TV interrupt control *
        tvFRAME_CTL &= (~0x0f);
        IduOsdL1Ctrl  &= ~(0x00000010);
		IduOsdL2Ctrl  &= ~(0x00000010);
        OSTimeDly(2);
        IduEna = 0;
		iduRst();
        
        //---Lucian: 設定 CCIR656 Pannel---//
      #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
        IduMpuCmd = 0x00006000;
        iduWaitCmdBusy();
      #elif((LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
       #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
       #else //656
        IduMpuCmd = 0x00006002;
       #endif
        iduWaitCmdBusy();
      #endif
        IduYCbCr2R=0x002C0020;
        IduYCbCr2G=0x00968B20;
        IduYCbCr2B=0x00003820;
        iduTVColorbar_onoff(1); 
        ResetisuPanel();

        OSTimeDly(1);
        IDU_Init(0 , 1);
        uiOutputRedirection();

      #if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
        HDMITX_AudioEnable(FALSE);
        //HDMITX_DisableVideoOutput();
      #endif        
      #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
           (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
           (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
        if(isHDMIpluged() != 1 )
        {
            gpioSetLevel(GPIO_GROUP_HDMI_RST, GPIO_BIT_HDMI_RST, GPIO_LEVEL_LO);
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_HDMI_RST, GPIO_BIT_HDMI_RST, GPIO_LEVEL_HI);
        }
      #endif
      #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
           (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
           (HW_BOARD_OPTION == MR9200_RX_RDI_M1000) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
           (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
           (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        AdjustIIS5FreqData(SYS_OUTMODE_PANEL); //set audio clock and 16kHz
        #if (AUDIO_DEVICE == AUDIO_IIS_WM8940)
        WM8940_AdjustSpeakerVolume(sysVolumnControl);
        WM8940_MicRec_AuxPlay_Switch2panel();
        #endif
      #endif
        IIS_SwitchPanel2HDMI();

        //----Turn on back light---//
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, GPIO_LEVEL_HI);
        OSTimeDly(2);
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_HI);
    }
}

/** sysTVswitchResolutionbyImagesize is better 
 ** state is unclear(enter menu) can use this function to set specific resolution **/
void sysTVswitchResolutionbyAssign(u8 videoType)
{
#if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
    if(sysTVOutOnFlag)
    {
        if (videoType == TV_720p60)
        {
            TvOutMode =SYS_TV_OUT_HD720P60;
            HDMITX_VideoChange(TV_720p60);
            iduPlaybackMode(1280, 720, 1280);
            #if(Video_Processer == Video_Processer_VX9988)
            i2cCmd_VX9988_SwitchResolution(TV_720p60);
            #endif
        }
        else if(videoType == TV_1080i60)
        {
            if(isCap1920x1080I() == 1)
            {
                TvOutMode =SYS_TV_OUT_FHD1080I60;
                HDMITX_VideoChange(TV_1080i60);
                iduPlaybackMode(1920, 1080, 1920);
                #if(Video_Processer == Video_Processer_VX9988)
                i2cCmd_VX9988_SwitchResolution(TV_1080i60);
                #endif
            }
            else
            {
                TvOutMode =SYS_TV_OUT_HD720P60;
                HDMITX_VideoChange(TV_720p60);
                iduPlaybackMode(1280, 720, 1280);
                #if(Video_Processer == Video_Processer_VX9988)
                i2cCmd_VX9988_SwitchResolution(TV_720p60);
                #endif
            }
        }
    }
#endif
}

void sysTVswitchResolutionbyImagesize(void)
{
#if (HDMI_TXIC_SEL ==  HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
    u8 err = 0;
    OSSemPend(sysDisplaySemReq, sysDisplayTimeOut, &err); //task racing when power on, live view connect with TX(FHD) vs SD card format(HD)
    if (err != OS_NO_ERR)
    {
        DEBUG_HDMI("Error: sysTVswitchResolutionbyImagesize sysDisplaySemReq is %d.\n", err);
    }
    if(sysTVOutOnFlag)
    {
        if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
        {
            if (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2)
            {
                if(isCap1920x1080I() == 1)
                {
                    TvOutMode =SYS_TV_OUT_FHD1080I60;
                    HDMITX_VideoChange(TV_1080i60);
                    iduPlaybackMode(1920, 1080, 1920);
                    #if(Video_Processer == Video_Processer_VX9988)
                    i2cCmd_VX9988_SwitchResolution(TV_1080i60);
                    #endif
                }
                else
                {
                    TvOutMode =SYS_TV_OUT_HD720P60;
                    HDMITX_VideoChange(TV_720p60);
                    iduPlaybackMode(1280, 720, 1280);
                    #if(Video_Processer == Video_Processer_VX9988)
                    i2cCmd_VX9988_SwitchResolution(TV_720p60);
                    #endif
                }
            }
            else
            {
                TvOutMode =SYS_TV_OUT_HD720P60;
                HDMITX_VideoChange(TV_720p60);
                iduPlaybackMode(1280, 720, 1280);
                #if(Video_Processer == Video_Processer_VX9988)
                i2cCmd_VX9988_SwitchResolution(TV_720p60);
                #endif
            }
        }
        else if (sysCameraMode==SYS_CAMERA_MODE_RF_RX_QUADSCR)
        {
            TvOutMode =SYS_TV_OUT_HD720P60;
            HDMITX_VideoChange(TV_720p60);
//            iduPlaybackMode(1280, 720, 1280);
            #if(Video_Processer == Video_Processer_VX9988)
            i2cCmd_VX9988_SwitchResolution(TV_720p60);
            #endif
        }
        else if (sysCameraMode == SYS_CAMERA_MODE_PLAYBACK)
        {
          #if PLAYBACK_FIRST_IFRAME
            if ((Iframe_flag == 1) || (!sysPlaybackVideoPause && StartPlayBack))
          #else
            if (StartPlayBack == 0)  //playback start is not (Iframe_flag == 1)
           #endif
            {
                if(asfVopWidth > RF_RX_2DISP_WIDTH*2) //1920
                {
                    if(isCap1920x1080I() == 1)
                    {
                        TvOutMode =SYS_TV_OUT_FHD1080I60;
                        HDMITX_VideoChange(TV_1080i60);
                        iduPlaybackMode(1920, 1080, 1920);
                        #if(Video_Processer == Video_Processer_VX9988)
                        i2cCmd_VX9988_SwitchResolution(TV_1080i60);
                        #endif
                    }
                    else
                    {
                        TvOutMode =SYS_TV_OUT_HD720P60;
                        HDMITX_VideoChange(TV_720p60);
                        iduPlaybackMode(960,720,1280);
                        #if(Video_Processer == Video_Processer_VX9988)
                        i2cCmd_VX9988_SwitchResolution(TV_720p60);
                        #endif
                    }
                }
                else //720
                {
                    TvOutMode =SYS_TV_OUT_HD720P60;
                    HDMITX_VideoChange(TV_720p60);
                    iduPlaybackMode(1280, 720, 1280);
                    #if(Video_Processer == Video_Processer_VX9988)
                    i2cCmd_VX9988_SwitchResolution(TV_720p60);
                    #endif
                }
            }
            else // menu
            {
                TvOutMode =SYS_TV_OUT_HD720P60;
                HDMITX_VideoChange(TV_720p60);
                iduPlaybackMode(1280, 720, 1280);
                #if(Video_Processer == Video_Processer_VX9988)
                i2cCmd_VX9988_SwitchResolution(TV_720p60);
                #endif
            }
        }
        else if(sysCameraMode == SYS_CAMERA_MODE_UI) //M1000 menu
        {
            TvOutMode =SYS_TV_OUT_HD720P60;
            HDMITX_VideoChange(TV_720p60);
            iduPlaybackMode(1280, 720, 1280);
            #if(Video_Processer == Video_Processer_VX9988)
            i2cCmd_VX9988_SwitchResolution(TV_720p60);
            #endif
        }
        //Idu_ClearBuf(1);
    }
    OSSemPost(sysDisplaySemReq);
#endif
}

void sysPlaybackSetTVSupport1080I(u8 setmode)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    
    if(sysCameraMode == SYS_CAMERA_MODE_PLAYBACK)
    {
        if(!ResetPlayback && !sysPlaybackVideoPause && StartPlayBack)
        {
            if(setmode)    //In
            {
                if((isCap1920x1080I() != 1) && (asfVopWidth == 1920))
                {
                    iduTVColorbar_onoff(1);
                    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                    
                    Idu_ClearBuf(6);
                    asfSwitchSource(SYS_OUTMODE_TV);
                    sysTVswitchResolutionbyImagesize();
                    iduPlaybackMode(960,720,1280);
                #if UI_SYNCHRONOUS_DUAL_OUTPUT    
                    uiOsdDrawInit();
                #endif
                    osdDrawPlayIcon();
                    if(sysPlaybackVideoPause==0)
                        osdDrawPlayIndicator(UI_PLAY_ICON_PAUSE);
                    else
                        osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);            

                    if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                       tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                    else
                       tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                    iduTVColorbar_onoff(0);
                    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);

                }
            }
            else    //Out
            {
                if((TvOutMode == SYS_TV_OUT_HD720P60) && (asfVopWidth == 1920))
                {
                    iduTVColorbar_onoff(1);
                    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                    
                    Idu_ClearBuf(6);
                    asfSwitchSource(SYS_OUTMODE_TV);
                    sysTVswitchResolutionbyImagesize();
                    iduPlaybackMode(1920,1080,1920);
                 #if UI_SYNCHRONOUS_DUAL_OUTPUT
                    uiOsdDrawInit();
                 #endif
                    osdDrawPlayIcon();
                    if(sysPlaybackVideoPause==0)
                        osdDrawPlayIndicator(UI_PLAY_ICON_PAUSE);
                    else
                        osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);            

                    if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                       tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                    else
                       tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                    iduTVColorbar_onoff(0);
                    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
                }
            }
        }
        
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if(setmode)    //In
        {
            if((isCap1920x1080I() != 1) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2))
            {
                iduTVColorbar_onoff(1);
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                Idu_ClearBuf(6);
                asfSwitchSource(SYS_OUTMODE_TV);
                sysTVswitchResolutionbyImagesize();
                iduPlaybackMode(960,720,1280);
                iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                   tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                else
                   tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                iduTVColorbar_onoff(0);
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
            }
        }
        else
        {
            if((TvOutMode == SYS_TV_OUT_HD720P60) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2))
            {
                iduTVColorbar_onoff(1);
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                Idu_ClearBuf(6);
                asfSwitchSource(SYS_OUTMODE_TV);
                sysTVswitchResolutionbyImagesize();
                iduPlaybackMode(1920,1080,1920);
                iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                   tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                else
                   tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                iduTVColorbar_onoff(0);
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
            }
        }
            
    }

}

#else //----------------------------------------------TX---------------------------------------------------------------//


#if(RFIU_SUPPORT)
  #if PWIFI_SUPPORT
        s32 sysBack_RFI_TX_Change_Reso(s32 Setting)
        {

            int temp;
        #if RF_TX_OPTIMIZE

            int i;
            s32             TimeOffset;
        #endif

        #if(FPGA_BOARD_A1018_SERIES)
            return 1;
        #endif

            temp=gRfiuUnitCntl[0].RFpara.MD_en;
            gRfiuUnitCntl[0].RFpara.MD_en=0;
      #if RF_TX_OPTIMIZE
            //--TX--//
            gRfiuUnitCntl[0].TX_Task_Stop=1;
            gRfiuUnitCntl[0].TX_Wrap_Stop=1;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
            gRfiuUnitCntl[0].TX_IIS_Stop=1;

            OSTimeDly(6);

            for(i=0; i<100; i++)
            {
                if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
                    break;
                OSTimeDly(1);
            }

            OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            DEBUG_SYS("====RFIU1_TASK Delete====\n");

            OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            DEBUG_SYS("====RFIU1_WRAP Delete====\n");

            /* suspend mpeg4 task */
         #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4SuspendTask();
            OSTaskDel(MPEG4_TASK_PRIORITY);
            DEBUG_SYS("====Mpeg4 task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskSuspend();
            OSTaskDel(VIDEO_TASK_PRIORITY);
            DEBUG_SYS("====VIDEO task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgSuspendTask();
            OSTaskDel(MJPG_TASK_PRIORITY);
         #endif

            //==IIS task==//
            iisStopRec_ch(0);
            iisSuspendTask();
            OSTaskDel(IIS_TASK_PRIORITY);
            DEBUG_SYS("====IIS task Delete====\n");

       #if RFIU_RX_AUDIO_RETURN
          #if IIS1_REPLACE_IIS5
            if(guiIISCh0PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh0PlayDMAId);
                guiIISCh0PlayDMAId = 0xFF;
            }
          #else
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
          #endif
       #endif
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=0;
            gRfiuUnitCntl[0].TX_IIS_Stop=0;
            //----Change sensor resolution-----//
            DEBUG_SYS("sysVideoInCHsel=%d\n",sysVideoInCHsel);
            switch(sysVideoInCHsel)
            {
                case 0:
                    isuStop();
                    ipuStop();
                    siuStop();
                    break;

                case 1:
                    ciu_1_Stop();
                    break;

                case 2:
                    ciu_2_Stop();
                    break;
                case 3:
                    ciu_3_Stop();
                    break;

                case 4:
                    ciu_4_Stop();
                    break;

                case 5:
                    ciu_5_Stop();
                    ipuStop();
                    siuStop();
                    break;
            }
            uiMenuSet_VideoSize(Setting);
            sysPreviewInit(0);
         #if(MULTI_CHANNEL_SEL & 0x02)
            ciu_1_OpMode = SIUMODE_MPEGAVI;
            ciu_1_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x04)
            ciu_2_OpMode = SIUMODE_MPEGAVI;
            ciu_2_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x20)
            ciu_5_OpMode = SIUMODE_MPEGAVI;
            ciu_5_FrameTime = 0;
         #endif
            OSTimeDly(20);
            if( (Setting != UI_MENU_VIDEO_SIZE_320x240) && (Setting != UI_MENU_VIDEO_SIZE_352x240) )  //Lucian: 本機不支援QVGA,不回存
                Save_UI_Setting();
            //--------------//
        #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
            timerCountRead(2, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset >> 8;
        #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            timerCountRead(1, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset / 100;
        #endif

            DEBUG_SYS("====IIS_TASK Create====\n");
            OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);

            DEBUG_SYS("====VIDEO_TASK Create====\n");
            OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY);

            DEBUG_SYS("====RFIU1_TASK Create====\n");
            OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;

      #else
            uiCaptureVideoStop();
            uiMenuSet_VideoSize(Setting);
            if( (Setting != UI_MENU_VIDEO_SIZE_320x240) && (Setting != UI_MENU_VIDEO_SIZE_352x240) ) //Lucian: 本機不支援QVGA,不回存
                Save_UI_Setting();
            uiCaptureVideo();
      #endif
            gRfiuUnitCntl[0].RFpara.MD_en=temp;

            return 1;
        }
  
        s32 sysBack_RFI_TX_SnapShot(s32 Setting)
        {
        #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) || (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            s32 TimeOffset;
        #endif
        #if RF_TX_OPTIMIZE
            int temp;
            int i;
        #endif
            //----------------------------------//
            TimeOffset = TimeOffset;	// Avoid warning msg
      #if RF_TX_OPTIMIZE
            temp=gRfiuUnitCntl[0].RFpara.MD_en;
            gRfiuUnitCntl[0].RFpara.MD_en=0;
            //--TX--//
            gRfiuUnitCntl[0].TX_Task_Stop=1;
            gRfiuUnitCntl[0].TX_Wrap_Stop=1;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
            gRfiuUnitCntl[0].TX_IIS_Stop=1;

            OSTimeDly(6);

            for(i=0; i<100; i++)
            {
                if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
                    break;
                OSTimeDly(1);
            }

            OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            DEBUG_SYS("====RFIU1_TASK Delete====\n");

            OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            DEBUG_SYS("====RFIU1_WRAP Delete====\n");

            /* suspend mpeg4 task */
         #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4SuspendTask();
            OSTaskDel(MPEG4_TASK_PRIORITY);
            DEBUG_SYS("====Mpeg4 task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskSuspend();
            OSTaskDel(VIDEO_TASK_PRIORITY);
            DEBUG_SYS("====VIDEO task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgSuspendTask();
            OSTaskDel(MJPG_TASK_PRIORITY);
         #endif

            //==IIS task==//
            iisStopRec_ch(0);
            iisSuspendTask();
            OSTaskDel(IIS_TASK_PRIORITY);
            DEBUG_SYS("====IIS task Delete====\n");

        #if RFIU_RX_AUDIO_RETURN
          #if IIS1_REPLACE_IIS5
            if(guiIISCh0PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh0PlayDMAId);
                guiIISCh0PlayDMAId = 0xFF;
            }
          #else
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
          #endif
        #endif
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=0;
            gRfiuUnitCntl[0].TX_IIS_Stop=0;
            //----Change sensor resolution-----//
            DEBUG_SYS("sysVideoInCHsel=%d\n",sysVideoInCHsel);
            switch(sysVideoInCHsel)
            {
                case 0:
                    isuStop();
                    ipuStop();
                    siuStop();
                    break;

                case 1:
                    ciu_1_Stop();
                    break;

                case 2:
                    ciu_2_Stop();
                    break;
                case 3:
                    ciu_3_Stop();
                    break;

                case 4:
                    ciu_4_Stop();
                    break;

                case 5:
                    ciu_5_Stop();
                    ipuStop();
                    siuStop();
                    break;
            }
            //----Snap Shot main body-----//
         #if TX_SNAPSHOT_SUPPORT
            if(sysRFTXSnapImgRdy ==0)
            {
                sysTxCaptureImage_CIU5(Setting);
            }
            else
            {
                DEBUG_SYS("DATA image is not transmitted yet\n");
            }
            sysRFTXSnapImgRdy=1;
         #endif
            //----------------------------//
            sysPreviewInit(0);

         #if(MULTI_CHANNEL_SEL & 0x02)
            ciu_1_OpMode = SIUMODE_MPEGAVI;
            ciu_1_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x04)
            ciu_2_OpMode = SIUMODE_MPEGAVI;
            ciu_2_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x20)
            ciu_5_OpMode = SIUMODE_MPEGAVI;
         #endif

            //--------------//

         #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
            timerCountRead(2, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset >> 8;
         #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            timerCountRead(1, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset / 100;
         #endif

            DEBUG_SYS("====IIS_TASK Create====\n");
            OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);

            DEBUG_SYS("====VIDEO_TASK Create====\n");
            OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY);

            DEBUG_SYS("====RFIU1_TASK Create====\n");
            OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;

            gRfiuUnitCntl[0].RFpara.MD_en=temp;

        #endif


            return 1;
        }  
  #else
        s32 sysBack_RFI_TX_Change_Reso(s32 Setting)
        {

            int temp;
        #if RF_TX_OPTIMIZE

            int i;
            s32             TimeOffset;
        #endif

        #if(FPGA_BOARD_A1018_SERIES)
            return 1;
        #endif

            temp=gRfiuUnitCntl[0].RFpara.MD_en;
            gRfiuUnitCntl[0].RFpara.MD_en=0;
      #if RF_TX_OPTIMIZE
            //--TX--//
            gRfiuUnitCntl[0].TX_Task_Stop=1;
            gRfiuUnitCntl[0].TX_Wrap_Stop=1;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
            gRfiuUnitCntl[0].TX_IIS_Stop=1;

            OSTimeDly(6);

            for(i=0; i<100; i++)
            {
                if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
                    break;
                OSTimeDly(1);
            }

            OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            DEBUG_SYS("====RFIU1_TASK Delete====\n");

            OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            DEBUG_SYS("====RFIU1_WRAP Delete====\n");

            /* suspend mpeg4 task */
         #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4SuspendTask();
            OSTaskDel(MPEG4_TASK_PRIORITY);
            DEBUG_SYS("====Mpeg4 task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskSuspend();
            OSTaskDel(VIDEO_TASK_PRIORITY);
            DEBUG_SYS("====VIDEO task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgSuspendTask();
            OSTaskDel(MJPG_TASK_PRIORITY);
         #endif

            //==IIS task==//
            iisStopRec_ch(0);
            iisSuspendTask();
            OSTaskDel(IIS_TASK_PRIORITY);
            DEBUG_SYS("====IIS task Delete====\n");

       #if RFIU_RX_AUDIO_RETURN
          #if IIS1_REPLACE_IIS5
            if(guiIISCh0PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh0PlayDMAId);
                guiIISCh0PlayDMAId = 0xFF;
            }
          #else
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
          #endif
       #endif
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=0;
            gRfiuUnitCntl[0].TX_IIS_Stop=0;
            //----Change sensor resolution-----//
            DEBUG_SYS("sysVideoInCHsel=%d\n",sysVideoInCHsel);
            switch(sysVideoInCHsel)
            {
                case 0:
                    isuStop();
                    ipuStop();
                    siuStop();
                    break;

                case 1:
                    ciu_1_Stop();
                    break;

                case 2:
                    ciu_2_Stop();
                    break;
                case 3:
                    ciu_3_Stop();
                    break;

                case 4:
                    ciu_4_Stop();
                    break;

                case 5:
                    ciu_5_Stop();
                    ipuStop();
                    siuStop();
                    break;
            }
            uiMenuSet_VideoSize(Setting);
            sysPreviewInit(0);
         #if(MULTI_CHANNEL_SEL & 0x02)
            ciu_1_OpMode = SIUMODE_MPEGAVI;
            ciu_1_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x04)
            ciu_2_OpMode = SIUMODE_MPEGAVI;
            ciu_2_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x20)
            ciu_5_OpMode = SIUMODE_MPEGAVI;
            ciu_5_FrameTime = 0;
         #endif
            OSTimeDly(20);
            if( (Setting != UI_MENU_VIDEO_SIZE_320x240) && (Setting != UI_MENU_VIDEO_SIZE_352x240) )  //Lucian: 本機不支援QVGA,不回存
                Save_UI_Setting();
            //--------------//
        #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
            timerCountRead(2, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset >> 8;
        #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            timerCountRead(1, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset / 100;
        #endif

            DEBUG_SYS("====IIS_TASK Create====\n");
            OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);

            DEBUG_SYS("====VIDEO_TASK Create====\n");
            OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY);

            DEBUG_SYS("====RFIU1_TASK Create====\n");
            OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;

      #else
            uiCaptureVideoStop();
            uiMenuSet_VideoSize(Setting);
            if( (Setting != UI_MENU_VIDEO_SIZE_320x240) && (Setting != UI_MENU_VIDEO_SIZE_352x240) ) //Lucian: 本機不支援QVGA,不回存
                Save_UI_Setting();
            uiCaptureVideo();
      #endif
            gRfiuUnitCntl[0].RFpara.MD_en=temp;

            return 1;
        }

        s32 sysBack_RFI_TX_SnapShot(s32 Setting)
        {
        #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) || (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            s32 TimeOffset;
        #endif
        #if RF_TX_OPTIMIZE
            int temp;
            int i;
        #endif
            //----------------------------------//
            TimeOffset = TimeOffset;	// Avoid warning msg
      #if RF_TX_OPTIMIZE
            temp=gRfiuUnitCntl[0].RFpara.MD_en;
            gRfiuUnitCntl[0].RFpara.MD_en=0;
            //--TX--//
            gRfiuUnitCntl[0].TX_Task_Stop=1;
            gRfiuUnitCntl[0].TX_Wrap_Stop=1;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
            gRfiuUnitCntl[0].TX_IIS_Stop=1;

            OSTimeDly(6);

            for(i=0; i<100; i++)
            {
                if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
                    break;
                OSTimeDly(1);
            }

            OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            DEBUG_SYS("====RFIU1_TASK Delete====\n");

            OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
            gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            DEBUG_SYS("====RFIU1_WRAP Delete====\n");

            /* suspend mpeg4 task */
         #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4SuspendTask();
            OSTaskDel(MPEG4_TASK_PRIORITY);
            DEBUG_SYS("====Mpeg4 task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskSuspend();
            OSTaskDel(VIDEO_TASK_PRIORITY);
            DEBUG_SYS("====VIDEO task Delete====\n");
         #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgSuspendTask();
            OSTaskDel(MJPG_TASK_PRIORITY);
         #endif

            //==IIS task==//
            iisStopRec_ch(0);
            iisSuspendTask();
            OSTaskDel(IIS_TASK_PRIORITY);
            DEBUG_SYS("====IIS task Delete====\n");

        #if RFIU_RX_AUDIO_RETURN
          #if IIS1_REPLACE_IIS5
            if(guiIISCh0PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh0PlayDMAId);
                guiIISCh0PlayDMAId = 0xFF;
            }
          #else
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
          #endif
        #endif
            gRfiuUnitCntl[0].TX_Task_Stop=0;
            gRfiuUnitCntl[0].TX_Wrap_Stop=0;
            gRfiuUnitCntl[0].TX_MpegEnc_Stop=0;
            gRfiuUnitCntl[0].TX_IIS_Stop=0;
            //----Change sensor resolution-----//
            DEBUG_SYS("sysVideoInCHsel=%d\n",sysVideoInCHsel);
            switch(sysVideoInCHsel)
            {
                case 0:
                    isuStop();
                    ipuStop();
                    siuStop();
                    break;

                case 1:
                    ciu_1_Stop();
                    break;

                case 2:
                    ciu_2_Stop();
                    break;
                case 3:
                    ciu_3_Stop();
                    break;

                case 4:
                    ciu_4_Stop();
                    break;

                case 5:
                    ciu_5_Stop();
                    ipuStop();
                    siuStop();
                    break;
            }
            //----Snap Shot main body-----//
         #if TX_SNAPSHOT_SUPPORT
            if(sysRFTXSnapImgRdy ==0)
            {
                sysTxCaptureImage_CIU5(Setting);
            }
            else
            {
                DEBUG_SYS("DATA image is not transmitted yet\n");
            }
            sysRFTXSnapImgRdy=1;
         #endif
            //----------------------------//
            sysPreviewInit(0);

         #if(MULTI_CHANNEL_SEL & 0x02)
            ciu_1_OpMode = SIUMODE_MPEGAVI;
            ciu_1_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x04)
            ciu_2_OpMode = SIUMODE_MPEGAVI;
            ciu_2_FrameTime = 0;
         #elif(MULTI_CHANNEL_SEL & 0x20)
            ciu_5_OpMode = SIUMODE_MPEGAVI;
         #endif

            //--------------//

         #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
            timerCountRead(2, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset >> 8;
         #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            timerCountRead(1, (u32*) &TimeOffset);
            IISTimeOffset   = TimeOffset / 100;
         #endif

            DEBUG_SYS("====IIS_TASK Create====\n");
            OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);

            DEBUG_SYS("====VIDEO_TASK Create====\n");
            OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY);

            DEBUG_SYS("====RFIU1_TASK Create====\n");
            OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
            gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;

            gRfiuUnitCntl[0].RFpara.MD_en=temp;

        #endif


            return 1;
        }
  #endif      
#endif

s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor,u8 BurstNum,u8 ScalingFactor) /*BJ 0530 S*/
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster,i,j;
    u32 bytes_per_cluster;
    u16   scale;
    u32 iduWinReg;
    u32* temp_00;
    u16 srcW,srcH;

    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */

#if DINAMICALLY_POWER_MANAGEMENT
    u32     sys_ctl0_status;
#endif
    //========================================//

    RTC_Get_Time(&curDateTime);
    exifSetDateTime(&curDateTime);

#if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sys_ctl0_status      = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_SIU_CKEN    |       //SIU enable
                          SYS_CTL0_IPU_CKEN    |       //IPU enable
                          SYS_CTL0_SRAM_CKEN   |       //IPU SRAM enable
                          SYS_CTL0_ISU_CKEN    |       //ISU enable
                          SYS_CTL0_SCUP_CKEN |
                          SYS_CTL0_GPIO1_CKEN  |
                          SYS_CTL0_TIMER4_CKEN |       //PWM enable
                          SYS_CTL0_SER_MCKEN;          //sensor master clock output enable

    sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN  &    //JPEG disable
                          ~SYS_CTL0_H264_CKEN &     //H264 disable
                          ~SYS_CTL0_HIU_CKEN &      //HIU disable.
#if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)
                          ~SYS_CTL0_RTC_CKEN&
#endif
                          //    ~SYS_CTL0_USB_CKEN &      //USB disable
                          0xffffffff;


    SYS_CTL0            = sys_ctl0_status;
#endif

    sysCaptureImageStart=1;
    DEBUG_SYS("Capture image start...\n");
    iduWinReg = sysProjectCaptureImage_Burst_OnPreview(1, 0, 0);

    scale = getPreviewZoomScale(ZoomFactor);
    DEBUG_SYS("zoomfactor = %d->%d/100\n", ZoomFactor, scale);
    sysPreviewZoomFactor = ZoomFactor;
    setSensorWinSize(ZoomFactor, SIUMODE_PREVIEW);  //Lucian: 永遠選擇640x480 capture.

    // stop preview process
    isuStop();
    ipuStop();
    siuStop();


    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    siuOpMode = SIUMODE_CAP_RREVIEW;

    for (i=0; i<BurstNum; i++)
    {
        sysProjectCaptureImage_Burst_OnPreview(2, 0, 0);
        isuCapturePreviewImg(ZoomFactor); //Lucian:data out to PKBUF
        ipuPreview(ZoomFactor);
        //while (SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
        siuPreviewInit(ZoomFactor,SIUMODE_CAP_RREVIEW);

        sysProjectCaptureImage_Burst_OnPreview(3, 0, 0);

        while (isu_idufrmcnt<2);

        isuStop();
        ipuStop();
        siuStop();

        switch( (isu_idufrmcnt-1) % 3)
        {
            case 0:
                CopyMemory(PKBuf+640*480*2*(i+VGA_RAW_OFFSET),PKBuf0,640*480*2);
                break;
            case 1:
                CopyMemory(PKBuf+640*480*2*(i+VGA_RAW_OFFSET),PKBuf1,640*480*2);
                break;
            case 2:
                CopyMemory(PKBuf+640*480*2*(i+VGA_RAW_OFFSET),PKBuf2,640*480*2);
                break;
        }
        sysProjectCaptureImage_Burst_OnPreview(4, BurstNum, i);
    }
#if PLAYBEEP_TEST
    if (wav_on==1)
    {
        sysbackSetEvt(SYS_BACK_PLAY_BEEP, 0);
    }
#endif
#if ADDAPP2TOJPEG
    exifApp2Data->APP2Marker=0xe2ff; //swap hi/lo byte
    data=sizeof(DEF_APPENDIXINFO)-2;
    exifApp2Data->APP2Size= (data<<8)|(data>>8); //swap hi/lo byte
    memcpy(exifApp2Data->ae_report.AEwtab,siu_dftAeWeight1,25);
    sysProjectCaptureImage_Burst_OnPreview(5, 0, 0);



    //SIU info
    exifApp2Data->siu_status.SIU_CTL1 =SiuSensCtrl;
    exifApp2Data->siu_status.OB_COMP =SiuOb;
    exifApp2Data->siu_DGT_GAIN1=SiuBGbGain;
    exifApp2Data->siu_DGT_GAIN2=SiuRGrGain;

    exifApp2Data->AWBgain_Preview[0]=AWBgain_Preview[0];
    exifApp2Data->AWBgain_Preview[1]=AWBgain_Preview[1];
    exifApp2Data->AWBgain_Preview[2]=AWBgain_Preview[2];

    exifApp2Data->AWBgain_Capture[0]=AWBgain_Preview[0];
    exifApp2Data->AWBgain_Capture[1]=AWBgain_Preview[1];
    exifApp2Data->AWBgain_Capture[2]=AWBgain_Preview[2];

    for (i=0; i<9; i++)
        exifApp2Data->CCM[i]=d50_IPU_CCM[i];

    exifApp2Data->LightFeq=AE_Flicker_50_60_sel;
#endif

    siuOpMode = SIUMODE_CAPTURE;

    siuGetPreviewZoomWidthHeight(ZoomFactor,&srcW,&srcH);
    for (i=0; i<BurstNum; i++)
    {
#if DINAMICALLY_POWER_MANAGEMENT
        sys_ctl0_status     = SYS_CTL0;
        sys_ctl0_status    |= SYS_CTL0_ISU_CKEN |
                              SYS_CTL0_SCUP_CKEN |
                              0x00000000;
        SYS_CTL0            = sys_ctl0_status;
#endif
        isuScalar_D2D(
            PKBuf + (i+VGA_RAW_OFFSET)*640*480*2 , //source address
            PKBuf ,                                //destination address
            srcW , srcH ,                            //source x,y
            640 , 480                              //destination x,y
        );

        // For thumbnail 640x480 -> 320x240
        isuScalar2Thumbnail(ZoomFactor); /*BJ 0530 S*/

#if DINAMICALLY_POWER_MANAGEMENT
        sys_ctl0_status     = SYS_CTL0;
        sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
        sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                              ~SYS_CTL0_H264_CKEN &
                              //~SYS_CTL0_SER_MCKEN &
                              ~SYS_CTL0_IPU_CKEN;

        SYS_CTL0            = sys_ctl0_status;
#endif
        jpegCaptureThumbnail((u8*)exifThumbnailImage.bitStream, &thumbnailImageSize);
        //====//
        switch (ScalingFactor)
        {
            case CAP_PREVIEW_SCAL_UP_X3: //Scaling to 2048x1536
                isuScalar_D2D(
                    PKBuf ,                                //src address
                    PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 , //dst address
                    640 , 480,                            //source x,y
                    2048 , 1536                              //destination x,y
                );
                exifSetImageResolution(2048, 1536); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 ,
                                      JPEG_OPMODE_FRAME,2048,1536);
                JpegImagePixelCount=2048*1536;//GetJpegImagePixelCount();
                break;
            case CAP_PREVIEW_SCAL_UP_X2: //Scaling to 1280x960
                isuScalar_D2D(
                    PKBuf ,                                //src address
                    PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 , //dst address
                    640 , 480,                            //source x,y
                    1280 , 960                              //destination x,y
                );
                exifSetImageResolution(1280, 960); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 ,
                                      JPEG_OPMODE_FRAME,1280,960);
                JpegImagePixelCount=1280*960;//GetJpegImagePixelCount();
                break;

            case CAP_PREVIEW_SCAL_DOWN_D2:
                isuScalar_D2D(
                    PKBuf ,                                       //src address
                    PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 , //dst address
                    640 , 480,                                    //source x,y
                    320 , 240                                     //destination x,y
                );
                exifSetImageResolution(320, 240); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 ,
                                      JPEG_OPMODE_FRAME,320,240);
                JpegImagePixelCount=320*240;//GetJpegImagePixelCount();
                break;

            case CAP_PREVIEW_SCAL_DOWN_D4:
                isuScalar_D2D(
                    PKBuf ,                                       //src address
                    PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 , //dst address
                    640 , 480,                                    //source x,y
                    160 , 120                                     //destination x,y
                );
                exifSetImageResolution(160, 120); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 ,
                                      JPEG_OPMODE_FRAME,160,120);
                JpegImagePixelCount=160*120;//GetJpegImagePixelCount();
                break;

            case CAP_PREVIEW_SCAL_UP_X1:
            default:
                isuScalar_D2D(
                    PKBuf ,                                //src address
                    PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 , //dst address
                    640 , 480,                            //source x,y
                    640 , 480                              //destination x,y
                );
                exifSetImageResolution(640, 480); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      PKBuf + (BurstNum+VGA_RAW_OFFSET)*640*480*2 ,
                                      JPEG_OPMODE_FRAME,640,480);
                JpegImagePixelCount=640*480;//GetJpegImagePixelCount();
                break;
        }
        primaryImageSize=WaitJpegEncComplete();
        /* set related EXIF IFD ... */
        compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
        exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
        exifSetCopyRightVersion(VERNUM);
#if DINAMICALLY_POWER_MANAGEMENT
        sys_ctl0_status     = SYS_CTL0;
#if SD_CARD_DISABLE

#else
        sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif

        sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                              ~SYS_CTL0_H264_CKEN &
                              //~SYS_CTL0_SER_MCKEN &
                              ~SYS_CTL0_JPEG_CKEN &
                              ~SYS_CTL0_ISU_CKEN &
                              ~SYS_CTL0_SCUP_CKEN &
                              ~SYS_CTL0_IPU_CKEN;

        SYS_CTL0            = sys_ctl0_status;
#endif

        //judge memory full or not civic 070926
        diskInfo    =   &global_diskInfo;
        bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO)+ bytes_per_cluster-1) / bytes_per_cluster;

        if ( (diskInfo->avail_clusters <= (used_cluster+2)) ) // protect write miss
        {
            //diskInfo->avail_clusters=0;     // let reset snashop times equal 0;
            if (sysTVOutOnFlag)
            {
                IduVideo_ClearPKBuf(2);
                IduVidBuf0Addr= (u32)PKBuf2;
#if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
                IduWinCtrl |= iduWinReg;
            }
            else
                IduWinCtrl |= iduWinReg; //Lucian:恢復IDU win control reg

            DEBUG_SYS("Memory full \n");
            sysPreviewReset(sysPreviewZoomFactor);
#if (UI_PREVIEW_OSD == 1)
            uiOSDPreviewInit();
            osdDrawMemFull(UI_OSD_DRAW);
            OSTimeDly(20);
            osdDrawMemFull(UI_OSD_CLEAR);
#endif
            sysCaptureImageStart=0;
            return 0;
        }

        exifWriteFile(thumbnailImageSize, primaryImageSize, 0);
    }


    if (sysTVOutOnFlag)
    {
        sysProjectCaptureImage_Burst_OnPreview(6, 0, 0);
        IduVidBuf0Addr= (u32)PKBuf0;
#if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
    }
    else
    {
        temp_00 = (u32 *)PKBuf0;
        for (j=0 ; j<3 ; j++)
        {
            temp_00=(u32 *)PKBuf0 + (j)*VIDEODISPBUF_SIZE/4;
            for (i=0 ; i<(PANNEL_X*PANNEL_Y*2) ; i+=4)
            {
                *temp_00 = 0x80800000;
                temp_00++;
            }
        }
        IduWinCtrl |= iduWinReg;
    }

    sysPreviewReset(sysPreviewZoomFactor);/*BJ 0530 S*/

#if (UI_PREVIEW_OSD == 1)
    uiOSDPreviewInit();
#endif
    if (gInsertNAND==1)
        userClicksnapshot=1;        // Means user clicked snapshot --> need to write back cache
    sysCaptureImageStart=0;

    sysProjectCaptureImage_Burst_OnPreview(7, 0, 0);

    return 1;
}

void sysSetOutputMode(u8 ucMode) //uiSetOutputMode
{}

/*

Routine Description:

    Preview initialization.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPreviewInit(s32 dummy)
{
#if ( (TV_DECODER == TW2866) && (CIU_SPLITER) )
    static u8 just_run_one=1;
#endif

#if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sysProjectPreviewReset(SYS_PREVIEW_RESET_PWRMAG);
#endif



    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

#if CIU_SPLITER  /* Amon (140612)*/
   #if(CHIP_OPTION == CHIP_A1018A)
        GpioActFlashSelect &= ~(CHIP_IO_DV2_EN3 | CHIP_IO_DISP2_EN | CHIP_IO_SMPTE2_EN);
      #if(HW_BOARD_OPTION  == MR9120_TX_OPCOM)
        GpioActFlashSelect |= GPIO_IISFrDISP_EN;
      #endif
        DEBUG_SYS("..11\n");
        SYS_CHIP_IO_CFG2 = (SYS_CHIP_IO_CFG2 |CHIP_IO2_CCIR4CH_EN) & ~(CHIP_IO2_RFI3_EN2);
   #elif(CHIP_OPTION == CHIP_A1018B)
        GpioActFlashSelect &= ~(CHIP_IO_DISP_EN | CHIP_IO_GPI2_EN | CHIP_IO_SPI3_EN | CHIP_IO_RMII_EN | CHIP_IO_SEN_EN | CHIP_IO_RMII_EN4);
        SYS_CHIP_IO_CFG2 &= ~CHIP_IO2_CCIR4CH_EN; // A1018B dummy PAD_SN_D
   #elif(CHIP_OPTION == CHIP_A1019A)
        GpioActFlashSelect &= ~(CHIP_IO_DISP_EN | CHIP_IO_GPI2_EN | CHIP_IO_SPI3_EN | CHIP_IO_RMII_EN | CHIP_IO_RMII_EN4);
        SYS_CHIP_IO_CFG2 &= ~CHIP_IO2_CCIR4CH_EN; // A1018B dummy PAD_SN_D
   #elif(CHIP_OPTION == CHIP_A1025A)
        GpioActFlashSelect &= ~(CHIP_IO_DISP_EN | CHIP_IO_GPI2_EN | CHIP_IO_SPI3_EN | CHIP_IO_RMII_EN4);
        SYS_CHIP_IO_CFG2 &= ~CHIP_IO2_CCIR4CH_EN; // A1018B dummy PAD_SN_D
   #elif (CHIP_OPTION == CHIP_A1021A)
       //not implement now.
   #endif
#endif
    sysCheckZoomRun_flag    = 0;
    IsuOutAreaFlagOnTV      = FULL_TV_OUT;
    sysCameraMode           = SYS_CAMERA_MODE_PREVIEW;
    if (sysTVOutOnFlag)
    {
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(640 * 2, 480);
#else
            iduSwitchPreview_TV(640, 480);
#endif
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
#if DUAL_MODE_DISP_SUPPORT
            if(sysTVinFormat == TV_IN_NTSC)
                iduSwitchPreview_TV(704 * 2, 480);
            else
                iduSwitchPreview_TV(704 * 2, 576);
#else
            if(sysTVinFormat == TV_IN_NTSC)
                iduSwitchPreview_TV(704, 480);
            else
                iduSwitchPreview_TV(704, 576);
#endif
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
#if DUAL_MODE_DISP_SUPPORT
            if(sysTVinFormat == TV_IN_NTSC)
                iduSwitchPreview_TV(720 * 2, 480);
            else
                iduSwitchPreview_TV(720 * 2, 576);
#else
            if(sysTVinFormat == TV_IN_NTSC)
                iduSwitchPreview_TV(720, 480);
            else
                iduSwitchPreview_TV(720, 576);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(640 * 2, 352);
#else
            iduSwitchPreview_TV(640, 352);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(1280 * 2, 720);
#else
            iduSwitchPreview_TV(1280, 720);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(1920 * 2, 1080);
#else
            iduSwitchPreview_TV(1920, 1080);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(1600 * 2, 896);
#else
            iduSwitchPreview_TV(1600, 896);
#endif
        }
        else
        {
#if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(640 * 2, 480);
#else
            iduSwitchPreview_TV(640, 480);
#endif
        }

        sysProjectPreviewReset(SYS_PREVIEW_RESET_TV_VIDEOOFF);
#if SUB_TV_TEST
        subTV_Preview(640,480);
#endif

    }
    else
    {
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(640 * 2, 480);
#else
            iduPreview(640, 480);
#endif
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
#if DUAL_MODE_DISP_SUPPORT
            if(sysTVinFormat == TV_IN_NTSC)
                iduPreview(704 * 2, 480);
            else
                iduPreview(704 * 2, 576);
#else
            if(sysTVinFormat == TV_IN_NTSC)
                iduPreview(704, 480);
            else
                iduPreview(704, 576);
#endif
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
#if DUAL_MODE_DISP_SUPPORT
            if(sysTVinFormat == TV_IN_NTSC)
                iduPreview(720 * 2, 480);
            else
                iduPreview(720 * 2, 576);
#else
            if(sysTVinFormat == TV_IN_NTSC)
                iduPreview(720, 480);
            else
                iduPreview(720, 576);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(1280 * 2, 720);
#else
            iduPreview(1280, 720);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(640 * 2, 352);
#else
            iduPreview(640, 352);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(1920 * 2, 1080);
#else
            iduPreview(1920, 1080);
#endif
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(1600 * 2, 896);
#else
            iduPreview(1600, 896);
#endif
        }
        else
        {
#if DUAL_MODE_DISP_SUPPORT
            iduPreview(640 * 2, 480);
#else
            iduPreview(640, 480);
#endif
        }

    }
    //-------------------------------------------//
#if(MULTI_CHANNEL_SEL & 0x01)
    sysProjectPreviewInit(SYS_PREVIEW_INIT_RESET);
#endif

#if (CIU_SPLITER | (MULTI_CHANNEL_SEL & 0x02))
    sysCiu_1_PreviewReset(0);
#endif

#if(CIU_SPLITER | (MULTI_CHANNEL_SEL & 0x04))
    GpioActFlashSelect = (GpioActFlashSelect | GPIO_DV2_FrGPI_EN2) & (~GPIO_GPIU_FrDISP_EN) ;
    sysCiu_2_PreviewReset(0);
#endif

#if(CIU_SPLITER | (MULTI_CHANNEL_SEL & 0x08))
    sysCiu_3_PreviewReset(0);
#endif

#if(CIU_SPLITER | (MULTI_CHANNEL_SEL & 0x10))
    sysCiu_4_PreviewReset(0);
#endif

#if((MULTI_CHANNEL_SEL & 0x20))
    sysCiu_5_PreviewReset(0);
#if(Sensor_OPTION == Sensor_MI_5M) // support raw sensor
    SYSClkEnable(SYS_CTL0_IPU_CKEN);
    SYSClkEnable(SYS_CTL0_SRAM_CKEN);
#endif
    ipuPreview(0);
    siuPreview(0);
#endif




    siuResumeTask();
#if NEW_IDU_BRI
    sysProjectPreviewInit(SYS_PREVIEW_INIT_OSDDRAWICON);
#else
    sysProjectPreviewInit(SYS_PREVIEW_INIT_OSDDRAWICON);
#endif
#if ( (TV_DECODER == TW2866) && (CIU_SPLITER) ) /* Amon 暫時放這 (140612)*/
    if (just_run_one)
    {
#if (CIU_SPLITER && CHIP_OPTION == CHIP_A1018A)
        CIU_1_SPILTER_CTL = 0x32100000;// default seting
        OSTimeDly(1);
        //DEBUG_I2C("CIU_1_SPILTER_CTL =%x\n",CIU_1_SPILTER_CTL);
#endif

        //DEBUG_WARERR("CIU_1_SPILTER_CTL = %x\n",CIU_1_SPILTER_CTL);
        if( (CIU_1_SPILTER_CTL & 0xffff) == 0x3210 )
            CIU_1_SPILTER_CTL = 0x32100000;
        else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x2103 )
            CIU_1_SPILTER_CTL = 0x03210000;
        else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x1032 )
            CIU_1_SPILTER_CTL = 0x10320000;
        else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x0321 )
            CIU_1_SPILTER_CTL = 0x21030000;
        just_run_one=0;
    }
#endif

    return 1;
}

/*

Routine Description:

    Preview zoom.

Arguments:

    zoomfactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPreviewReset(s32 zoomFactor)
{
    u16   scale;

    //---- stop preview process---//
    isuStop();
    ipuStop();
    siuStop();

    setSensorWinSize(zoomFactor, SIUMODE_PREVIEW);
    scale = getPreviewZoomScale(zoomFactor);
    DEBUG_SYS("zoomfactor = %d->%d/100\n", zoomFactor, scale);
    sysPreviewZoomFactor = zoomFactor;
    siuOpMode = SIUMODE_PREVIEW;

    isuPreview(zoomFactor);
    ipuPreview(zoomFactor);
    siuPreview(zoomFactor);



#if ( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FID_INT_ena();
#endif
#endif

#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/

    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif

#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif

    return 1;
}


s32 sysCiu_1_PreviewReset(s32 zoomFactor)
{

    siuOpMode = SIUMODE_PREVIEW;
#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        u8 mode = SIUMODE_PREVIEW;

        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(mode,640,240,320,120,320,120,CIU1_OSD_EN,640*2);
            else
                ciuPreviewInit_CH1(mode,640,240,640,240,0,0,CIU1_OSD_EN,640*2);

            rfiuVideoInFrameRate=30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(mode,640,288,320,144,320,144,CIU1_OSD_EN,640*2);
            else
                ciuPreviewInit_CH1(mode,640,288,640,288,0,0,CIU1_OSD_EN,640*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,240,320,120,320,120,CIU1_OSD_EN,704*2);
            else
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,240,704,240,0,0,CIU1_OSD_EN,704*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,288,320,144,320,144,CIU1_OSD_EN,704*2);
            else
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,288,704,288,0,0,CIU1_OSD_EN,704*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,720,240,320,120,320,120,CIU1_OSD_EN,720*2);
            else
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,720,240,720,240,0,0,CIU1_OSD_EN,720*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,720,288,320,144,320,144,CIU1_OSD_EN,720*2);
            else
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,720,288,720,288,0,0,CIU1_OSD_EN,720*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else
    {
        DEBUG_SYS("Sensor_CCIR656 not support 720p\n");
    }
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    //DEBUG_SYS("rfiuVideoInFrameRate=%d\n",rfiuVideoInFrameRate);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,360,0,0,CIU1_OSD_EN,1280);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU1_OSD_EN,1280);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,960,544,0,0,CIU1_OSD_EN,1920);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1920,1080,0,0,CIU1_OSD_EN,1920);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600/2,896/2,0,0,CIU1_OSD_EN,1600);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU1_OSD_EN,1600);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,640);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,176,0,0,CIU1_OSD_EN,640);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU1_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,704);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU1_OSD_EN,704);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,704);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU1_OSD_EN,704);
    }
    else
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,640);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
    }
#endif
#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif
#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif

    return 1;
}


s32 sysCiu_2_PreviewReset(s32 zoomFactor)
{

    siuOpMode = SIUMODE_PREVIEW;
#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,240,320,120,320,120,CIU2_OSD_EN,640*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU2_OSD_EN,640*2);
            rfiuVideoInFrameRate    = 30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,288,320,144,320,120,CIU2_OSD_EN,640*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU2_OSD_EN,640*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,240,320,120,320,120,CIU2_OSD_EN,704*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,240,704,240,0,0,CIU2_OSD_EN,704*2);
            rfiuVideoInFrameRate    = 30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,288,320,144,320,144,CIU2_OSD_EN,704*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,288,704,288,0,0,CIU2_OSD_EN,704*2);
            rfiuVideoInFrameRate=25;
        }

    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,720,240,320,120,320,120,CIU2_OSD_EN,720*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,720,240,720,240,0,0,CIU2_OSD_EN,720*2);
            rfiuVideoInFrameRate    = 30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH1)
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,720,288,320,144,320,144,CIU2_OSD_EN,720*2);
            else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,720,288,720,288,0,0,CIU2_OSD_EN,720*2);
            rfiuVideoInFrameRate=25;
        }

    }
#elif 0
    ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,320);
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,360,0,0,CIU2_OSD_EN,1280);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU2_OSD_EN,1280);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,960,544,0,0,CIU2_OSD_EN,1920);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,CIU2_OSD_EN,1920);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600/2,896/2,0,0,CIU2_OSD_EN,1600);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU2_OSD_EN,1600);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,640);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,176,0,0,CIU2_OSD_EN,640);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU2_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,704);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,704);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU2_OSD_EN,704);
    }
    else
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,640);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
    }

#endif
#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif
#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif
    return 1;
}

s32 sysCiu_3_PreviewReset(s32 zoomFactor)
{
    siuOpMode = SIUMODE_PREVIEW;
#if( (CIU3_OPTION==Sensor_CCIR656) || (CIU3_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU3_OSD_EN,640*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU3_OSD_EN,640*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,704,240,704,240,0,0,CIU3_OSD_EN,704*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,704,288,704,288,0,0,CIU3_OSD_EN,704*2);
            rfiuVideoInFrameRate=25;
        }

    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,720,240,720,240,0,0,CIU3_OSD_EN,720*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH3(SIUMODE_PREVIEW,720,288,720,288,0,0,CIU3_OSD_EN,720*2);
            rfiuVideoInFrameRate=25;
        }

    }
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU3_OSD_EN,1280);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1920,1080,0,0,CIU3_OSD_EN,1920);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU3_OSD_EN,1600);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU3_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU3_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU3_OSD_EN,704);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU3_OSD_EN,704);
    else
        ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU3_OSD_EN,640);
#endif
#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif
#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif
    return 1;
}

s32 sysCiu_4_PreviewReset(s32 zoomFactor)
{
    siuOpMode = SIUMODE_PREVIEW;
#if( (CIU3_OPTION==Sensor_CCIR656) || (CIU3_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU4_OSD_EN,640*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU4_OSD_EN,640*2);
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,704,240,704,240,0,0,CIU4_OSD_EN,704*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,704,288,704,288,0,0,CIU4_OSD_EN,704*2);
            rfiuVideoInFrameRate=25;
        }

    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,720,240,720,240,0,0,CIU4_OSD_EN,720*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH4(SIUMODE_PREVIEW,720,288,720,288,0,0,CIU4_OSD_EN,720*2);
            rfiuVideoInFrameRate=25;
        }

    }
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU4_OSD_EN,1280);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1920,1080,0,0,CIU4_OSD_EN,1920);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU4_OSD_EN,1600);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU4_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU4_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU4_OSD_EN,704);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU4_OSD_EN,704);
    else
        ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU4_OSD_EN,640);
#endif
#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif
#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif
    return 1;
}

s32 sysCiu_5_PreviewReset(s32 zoomFactor)
{
    u8 x;
    siuOpMode = SIUMODE_PREVIEW;
#if( (CIU5_OPTION==Sensor_CCIR656) || (CIU5_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,640,240,640,240,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU5_OSD_EN,640*2);
#endif
            rfiuVideoInFrameRate=30;
        }
        else
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,640,288,640,288,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU5_OSD_EN,640*2);
#endif
            rfiuVideoInFrameRate=25;
        }
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,704,240,704,240,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,704*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,704,240,704,240,0,0,CIU5_OSD_EN,704*2);
#endif
            rfiuVideoInFrameRate=30;
        }
        else
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,704,288,704,288,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,704*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,704,288,704,288,0,0,CIU5_OSD_EN,704*2);
#endif
            rfiuVideoInFrameRate=25;
        }

    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,720,240,720,240,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,720*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,720,240,720,240,0,0,CIU5_OSD_EN,720*2);
#endif
            rfiuVideoInFrameRate=30;
        }
        else
        {
#if TIMESTAMPCTR_ENABLE
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,720,288,720,288,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,720*2);
#else
            ciuPreviewInit_CH5(SIUMODE_PREVIEW,720,288,720,288,0,0,CIU5_OSD_EN,720*2);
#endif
            rfiuVideoInFrameRate=25;
        }

    }
#else
   #if ENABLE_H264_1080
    for(x=0; x<4; x++)
    {
        memset_hw_Word(PNBuf_sub5[x], 0x00000000, 1920*1088 ); //clear display buffer.
        memset_hw_Word(PNBuf_sub5[x]+1920*1088, 0x80808080, 1920*1088/2 ); //clear display buffer.
    }
   #endif
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,1280);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU5_OSD_EN,1280);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,1920);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,CIU5_OSD_EN,1920);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,1600);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU5_OSD_EN,1600);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU5_OSD_EN,640);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,360,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,360,0,0,CIU5_OSD_EN,640);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,704);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU5_OSD_EN,704);
#endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)\
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,704);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU5_OSD_EN,704);
#endif
    }
    else
    {
#if TIMESTAMPCTR_ENABLE
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,gRfiuUnitCntl[0].RFpara.TX_TimeStampOn,640);
#else
        ciuPreviewInit_CH5(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU5_OSD_EN,640);
#endif
    }
#endif
#if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
#endif
#if ((AUDIO_DEVICE == AUDIO_IIS_ALC5621) && (AUDIO_BYPASS == 1))
    Init_IIS_ALC5621_bypass();
#endif
    return 1;
}

s32 sysPreviewZoomInOut(s32 zoomFactor)
{
    u16 scale;

    sysCheckZoomRun_flag=1;  //Lucian: 目前Zoom IN/OUT 正在進行.

    scale = getPreviewZoomScale(zoomFactor);
    DEBUG_SYS("PreviewZoomFactor = %d->%d/100\n", zoomFactor, scale);
    switch(sysVideoInCHsel)
    {
        case 0:
#if( (Sensor_OPTION != Sensor_CCIR656))
            isuScUpZoom(zoomFactor);
#endif
            break;

        case 1:
#if( (CIU1_OPTION != Sensor_CCIR656) )
            ciu1ScUpZoom(zoomFactor);
#endif
            break;

        case 2:
#if( (CIU2_OPTION != Sensor_CCIR656) )
            ciu2ScUpZoom(zoomFactor);
#endif
            break;

        case 3:
#if( (CIU3_OPTION != Sensor_CCIR656) )
            ciu3ScUpZoom(zoomFactor);
#endif
            break;

        case 4:
#if( (CIU4_OPTION != Sensor_CCIR656) )
            ciu4ScUpZoom(zoomFactor);
#endif
            break;

        case 5:
#if( (CIU5_OPTION != Sensor_CCIR656) )
            ciu5ScUpZoom(zoomFactor);
#endif
            break;
    }

    sysCheckZoomRun_flag=0;  //Lucian: 目前Zoom IN/OUT 結束.

    return 1;
}


s32 sysTVInChannelChange_Preview(s32 zoomFactor)
{
#if(TV_DECODER == TI5150)
    u8 status;
    int cnt=0;
#endif
    //Lucian: modified to fix 畫面被分割的現象.
    siuStop();
    ipuStop();
    isuStop();

#if(TV_DECODER == BIT1605) //use bit1605 tv decoder

#elif(TV_DECODER == MI9V136)

#elif(TV_DECODER == TI5150) //use TI5150 tv decoder
    OSTimeDly(3);
    i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);  //check TVP5150 is locked to the video signal
    while(((status&0x80) != 0x80) && (cnt < 10))
    {
        cnt++;
        OSTimeDly(3);
        i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);
    }
#endif
    //initialize sensor
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW_ZOOM,zoomFactor);
    setSensorWinSize(zoomFactor, SIUMODE_PREVIEW);

    isuPreviewZoom(zoomFactor);
    ipuPreview(zoomFactor);
    siuPreviewZoom(zoomFactor);

    return 1;
}

s32 sysVideoZoomInOut(s32 zoomFactor)
{
    u16 scale;

    sysCheckZoomRun_flag=1;  //Lucian: 目前Zoom IN/OUT 正在進行.

    scale = getPreviewZoomScale(zoomFactor);
    DEBUG_SYS("PreviewZoomFactor = %d->%d/100\n", zoomFactor, scale);
    switch(sysVideoInCHsel)
    {
        case 0:
#if(Sensor_OPTION != Sensor_CCIR656)
            isuScUpZoom(zoomFactor);
#endif
            break;

        case 1:
#if(CIU1_OPTION != Sensor_CCIR656)
            ciu1ScUpZoom(zoomFactor);
#endif
            break;

        case 2:
#if(CIU2_OPTION != Sensor_CCIR656)
            ciu2ScUpZoom(zoomFactor);
#endif
            break;

        case 3:
#if(CIU3_OPTION != Sensor_CCIR656)
            ciu3ScUpZoom(zoomFactor);
#endif
            break;

        case 4:
#if(CIU4_OPTION != Sensor_CCIR656)
            ciu4ScUpZoom(zoomFactor);
#endif
            break;

        case 5:
#if(CIU5_OPTION != Sensor_CCIR656)
            ciu5ScUpZoom(zoomFactor);
#endif
            break;
    }

    sysCheckZoomRun_flag=0;  //Lucian: 目前Zoom IN/OUT 結束.
    return 1;
}


s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor)
{
#if(TV_DECODER == TI5150)
    u8 status;
    int cnt=0;
#endif

    //Lucian: modified to fix 畫面被分割的現象.
    siuStop();
    ipuStop();
    isuStop();

#if(TV_DECODER == BIT1605) //use bit1605 tv decoder

#elif(TV_DECODER == MI9V136)

#elif(TV_DECODER == TI5150) //use TI5150 tv decoder
    OSTimeDly(3);
    i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);  //check TVP5150 is locked to the video signal
    while(((status&0x80) != 0x80) && (cnt < 10))
    {
        cnt++;
        OSTimeDly(3);
        i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);
    }
#endif
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_MPEGAVI_ZOOM,zoomFactor);     // initialize sensor
    setSensorWinSize(zoomFactor, SIUMODE_MPEGAVI); // Set SIU window size

    isuVideoZoom(zoomFactor);
    ipuCaptureVideo();
    siuVideoZoom(zoomFactor);
    return 1;
}




s32 sysSnapshot_OnPreview(s32 ScalingFactor)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    //if(siuOpMode != SIUMODE_PREVIEW)
    if(sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
    {
        DEBUG_SYS("Only Valid on Preview Mode!\n");
        return 0;
    }

    if (Write_protet() && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        osdDrawProtect(2);
        return 0;
    }

    siuFlashReady = 0;


    siuAeReadyToCapImage = 0;
    siuAeEnable = 0;
    sysProjectSnapshotOnPreview(1, ScalingFactor);

    system_busy_flag=1;

    sysProjectSnapshotOnPreview(2, ScalingFactor);
    system_busy_flag=0;

    sysProjectSnapshotOnPreview(3, ScalingFactor);
    return 1;
#endif
}



s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8 FID;
    int count;

    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
    //GPIO_INT_CFG a;
#if DINAMICALLY_POWER_MANAGEMENT
    u32     sys_ctl0_status;
#endif

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_SIU Start---\n");
    RTC_Get_Time(&curDateTime);
    exifSetDateTime(&curDateTime);
    //---Stop intr---//
    while( (isu_idufrmcnt % 3) !=0);
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *

    isuStop();
    ipuStop();
    siuStop();

    IduWinCtrl = (IduWinCtrl & ~0x00003000) | (0x02 << 12);
    //---Capture start---//
    setSensorWinSize(ZoomFactor, SIUMODE_PREVIEW);  //Lucian: 永遠選擇640x480 capture.
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    siuOpMode = SIUMODE_CAP_RREVIEW;

    isuCapturePreviewImg(ZoomFactor); //Lucian:data out to PKBUF
    sysDrawTimeOnCapture(0);
    ipuPreview(ZoomFactor);
    siuPreviewInit(ZoomFactor,SIUMODE_CAP_RREVIEW);
    while(isu_idufrmcnt<2);
    isuStop();
    ipuStop();
    siuStop();
    siuOpMode = SIUMODE_CAPTURE;
    isuScalar_D2D(
        PKBuf0 , PKBuf1 ,
        640, 480,
        160, 120);
#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif
    jpegCaptureThumbnail((u8*)exifThumbnailImage.bitStream, &thumbnailImageSize);

    exifSetImageResolution(640, 480); //暫時用
    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                          PKBuf0,
                          JPEG_OPMODE_FRAME,640,480);

    JpegImagePixelCount=640*480;//GetJpegImagePixelCount();

    primaryImageSize=WaitJpegEncComplete();

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;
#endif


    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);



    //----judge memory full or not -----
    diskInfo    =   &global_diskInfo;
    bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO)+ bytes_per_cluster-1) / bytes_per_cluster;

    if ((diskInfo->avail_clusters <= (used_cluster+2)) ) // protect write miss
    {

        DEBUG_SYS("Memory full \n");
#if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        osdDrawMemFull(UI_OSD_CLEAR);
#endif
    }
    else
    {
        exifWriteFile(thumbnailImageSize, primaryImageSize, 0);
    }

    // Return to preview
    //---Start intr---//
    //sysPreviewReset(sysPreviewZoomFactor);/*BJ 0530 S*/
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    isuPreview(ZoomFactor);
    ipuPreview(ZoomFactor);
    if(sysVideoInSel == VIDEO_IN_TV) //TV-in
    {
        count = 0;
        FID=1;
        while(FID)
        {
#if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )
            //gpioGetLevel(0, 32, &FID);
#else //656
            FID=(SiuSyncStat>>31) & 0x01;
#endif
            count ++;
            if(count > 0x03ffff)
            {
                DEBUG_SYS("FID time out\n");
                break;
            }
        }
    }
    siuPreview(ZoomFactor);
    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

    DEBUG_SYS("--sysCaptureImage_One_OnPreview_SIU End---\n");

    return 1;

}


s32 sysCaptureImage_One_OnPreview_CIU1(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8 *srcImg;
    //RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU1 Start---\n");
    //---Stop intr---//
    while( (ciu_idufrmcnt_ch1 & 0x03) !=0);
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *

#if 1
    ciu_1_Stop();
#if DUAL_MODE_DISP_SUPPORT
    ciu_2_Stop();
#endif

    //---Capture start---//
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    ciu_1_OpMode = SIUMODE_CAP_RREVIEW;

    //sysCiu_1_PreviewReset(0);
#if ((CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601))
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,640,240,640,240,0,0,CIU1_OSD_EN,640*2);
        else
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,640,288,640,288,0,0,CIU1_OSD_EN,640*2);
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,240,704,240,0,0,CIU1_OSD_EN,704*2);
        else
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,288,704,288,0,0,CIU1_OSD_EN,704*2);
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,720,240,720,240,0,0,CIU1_OSD_EN,720*2);
        else
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,720,288,720,288,0,0,CIU1_OSD_EN,720*2);
    }
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU1_OSD_EN,1280);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1920,1080,0,0,CIU1_OSD_EN,1920);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU1_OSD_EN,1600);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU1_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU1_OSD_EN,704);
    else
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
#endif

#if (TV_DECODER == WT8861)
    if(sysTVinFormat == TV_IN_PAL)  // 若field有交換的話,第一張影像要輸出到第零張的位址再做壓縮
    {
        while(ciu_idufrmcnt_ch1 < 3);
    }
    else
    {
        while(ciu_idufrmcnt_ch1 < 2);
    }
#else
#if((CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601))  // A1016要丟兩張,不然CIU420轉422會有field的問題
    while(ciu_idufrmcnt_ch1 < 3);
#else
    while(ciu_idufrmcnt_ch1 < 2);
#endif
    ciu_1_Stop();
#endif

#if((CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601))    // A1016要丟兩張,不然CIU420轉422會有field的問題
    srcImg  = PNBuf_sub1[2];
#else
    srcImg  = PNBuf_sub1[0];
#endif

#else	// if IS_COMMAX_DOORPHONE
    //while((ciu_idufrmcnt_ch1 & 0x03) != 1);
#endif


#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif
    for(i = 0; i < 6; i++)
    {
#if (TV_DECODER == WT8861)
        while((ciu_idufrmcnt_ch1 & 0x03) != 1);
#endif
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            exifSetImageResolution(1280, 720); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1280,720);
            JpegImagePixelCount=1280*720;//GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
            exifSetImageResolution(1920, 1072); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1920,1072);
            JpegImagePixelCount=1920*1072;//GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
            exifSetImageResolution(1600, 896); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1600,896);
            JpegImagePixelCount=1600*896;//GetJpegImagePixelCount();
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(704, 480); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,704,480);
                JpegImagePixelCount=704*480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(704, 576); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,704,576);
                JpegImagePixelCount=704*576;//GetJpegImagePixelCount();
            }
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(720, 480); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,720,480);
                JpegImagePixelCount=720*480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(720, 576); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,720,576);
                JpegImagePixelCount=720*576;//GetJpegImagePixelCount();
            }
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            exifSetImageResolution(640, 352); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,640,352);
            JpegImagePixelCount=640*352;//GetJpegImagePixelCount();
        }
        else
        {
            exifSetImageResolution(640, 480); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,640,480);
            JpegImagePixelCount=640*480;//GetJpegImagePixelCount();
        }
        primaryImageSize    = WaitJpegEncComplete();

        if((exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9))  // JPEG bitstream good
        {
            break;
        }
        else            // JPEG bitstream fail, 用較低的品直再壓一次
        {
            switch(i)
            {
                case 0:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 1:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                    break;
                case 2:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                    break;
                case 3:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(10)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 10);
                    break;
                case 4:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(0)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 0);
                    break;
                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
            }
        }
    }

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;
#endif


    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);



    //----judge memory full or not -----
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster        = (exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + bytes_per_cluster - 1) / bytes_per_cluster;

    if ((diskInfo->avail_clusters <= (used_cluster + 2)) ) // protect write miss
    {

        DEBUG_SYS("Memory full\n");
        DEBUG_SYS("avail_clusters               = %d\n", diskInfo->avail_clusters);
        DEBUG_SYS("used_cluster                 = %d\n", used_cluster);
        DEBUG_SYS("exifPrimaryImageHeaderSize   = %d\n", exifPrimaryImageHeaderSize);
        DEBUG_SYS("primaryImageSize             = %d\n", primaryImageSize);
#if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        osdDrawMemFull(UI_OSD_CLEAR);
#endif
    }
    else
    {
        exifWriteFile(thumbnailImageSize, primaryImageSize, 1);
    }

    if(i != 0)  // 若Q Table有被變過就還原Q table的預設值
        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);

    //----- Return to preview--------//
    //---Start intr---//
    sysCameraMode   = SYS_CAMERA_MODE_PREVIEW;
    ciu_1_OpMode    = SIUMODE_PREVIEW;
    sysCiu_1_PreviewReset(0);
#if DUAL_MODE_DISP_SUPPORT
    sysCiu_2_PreviewReset(0);
#endif

    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU1 End---\n");

    return 1;

}

s32 sysCaptureImage_One_OnPreview_CIU2(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8 *srcImg;
    //RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU2 Start---\n");
    //---Stop intr---//
    while( (ciu_idufrmcnt_ch2 & 0x03) !=0);
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *

#if DUAL_MODE_DISP_SUPPORT
    ciu_1_Stop();
#endif
    ciu_2_Stop();

    //---Capture start---//
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    ciu_2_OpMode = SIUMODE_CAP_RREVIEW;

#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,640,240,640,240,0,0,CIU2_OSD_EN,640*2);
        else
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,640,288,640,288,0,0,CIU2_OSD_EN,640*2);
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,240,704,240,0,0,CIU2_OSD_EN,704*2);
        else
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,288,704,288,0,0,CIU2_OSD_EN,704*2);
    }
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
    {
        if(sysTVinFormat == TV_IN_NTSC)
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,720,240,720,240,0,0,CIU2_OSD_EN,720*2);
        else
            ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,720,288,720,288,0,0,CIU2_OSD_EN,720*2);
    }
#else
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU2_OSD_EN,1280);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,isuSrcImg.w,isuSrcImg.h,0,0,CIU2_OSD_EN,1920);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1600,896,0,0,CIU2_OSD_EN,896);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,352,0,0,CIU2_OSD_EN,640);
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
    else
        ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
#endif



#if (TV_DECODER == WT8861)
    if(sysTVinFormat == TV_IN_PAL)  // 若field有交換的話,第一張影像要輸出到第零張的位址再做壓縮
    {
        while(ciu_idufrmcnt_ch2 < 3);
    }
    else
    {
        while(ciu_idufrmcnt_ch2 < 2);
    }
#else
#if((CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601))  // A1016要丟兩張,不然CIU420轉422會有field的問題
    while(ciu_idufrmcnt_ch2 < 3);
#else
    while(ciu_idufrmcnt_ch2<2);
#endif
    ciu_2_Stop();
#endif

#if((CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601))    // A1016要丟兩張,不然CIU420轉422會有field的問題
    srcImg  = PNBuf_sub2[2];
#else
    srcImg  = PNBuf_sub2[0];
#endif
//	while(ciu_idufrmcnt_ch2<2);
//   ciu_2_Stop();




#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

    for(i = 0; i < 6; i++)
    {
#if (TV_DECODER == WT8861)
        while((ciu_idufrmcnt_ch2 & 0x03) != 1);
#endif
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            exifSetImageResolution(1280, 720); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1280,720);
            JpegImagePixelCount=1280*720;//GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
            exifSetImageResolution(1920, 1072); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1920,1072);
            JpegImagePixelCount=1920*1072;//GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
            exifSetImageResolution(1600, 896); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,1600,896);
            JpegImagePixelCount=1600*896;//GetJpegImagePixelCount();
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(704, 480); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,704,480);
                JpegImagePixelCount=704*480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(704, 576); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,704,576);
                JpegImagePixelCount = 704 * 576;//GetJpegImagePixelCount();
            }
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(720, 480); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,720,480);
                JpegImagePixelCount = 720 * 480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(720, 576); //暫時用
                jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                      //PNBuf_sub1[0],
                                      srcImg,
                                      JPEG_OPMODE_FRAME,720,576);
                JpegImagePixelCount = 720 * 576;//GetJpegImagePixelCount();
            }
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            exifSetImageResolution(640, 352); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,640,352);
            JpegImagePixelCount = 640 * 352;//GetJpegImagePixelCount();
        }
        else
        {
            exifSetImageResolution(640, 480); //暫時用
            jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
                                  //PNBuf_sub1[0],
                                  srcImg,
                                  JPEG_OPMODE_FRAME,640,480);
            JpegImagePixelCount = 640 * 480;//GetJpegImagePixelCount();
        }
        primaryImageSize=WaitJpegEncComplete();

        if((exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9))  // JPEG bitstream good
        {
            break;
        }
        else            // JPEG bitstream fail, 用較低的品直再壓一次
        {
            switch(i)
            {
                case 0:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 1:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                    break;
                case 2:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                    break;
                case 3:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(10)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 10);
                    break;
                case 4:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(0)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 0);
                    break;
                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
            }
        }
    }

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;
#endif


    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);



    //----judge memory full or not -----
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster        = (exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + bytes_per_cluster - 1) / bytes_per_cluster;

    if ( (diskInfo->avail_clusters <= (used_cluster + 2)) ) // protect write miss
    {

        DEBUG_SYS("Memory full\n");
        DEBUG_SYS("avail_clusters               = %d\n", diskInfo->avail_clusters);
        DEBUG_SYS("used_cluster                 = %d\n", used_cluster);
        DEBUG_SYS("exifPrimaryImageHeaderSize   = %d\n", exifPrimaryImageHeaderSize);
        DEBUG_SYS("primaryImageSize             = %d\n", primaryImageSize);
#if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        osdDrawMemFull(UI_OSD_CLEAR);
#endif
    }
    else
    {
        exifWriteFile(thumbnailImageSize, primaryImageSize, 2);
    }
    if(i != 0)  // 若Q Table有被變過就還原Q table的預設值
        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);
    // Return to preview
    //---Start intr---//
    sysCameraMode   = SYS_CAMERA_MODE_PREVIEW;
    ciu_2_OpMode    = SIUMODE_PREVIEW;

#if DUAL_MODE_DISP_SUPPORT
    sysCiu_1_PreviewReset(0);
#endif
    sysCiu_2_PreviewReset(0);

    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU2 End---\n");

    return 1;

}



s32 sysCaptureImage_One_OnPreview420_CIU1(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8  *srcImgY, *srcImgUV;
    u32 count;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;
#if (OS_CRITICAL_METHOD == 3)	/* Allocate storage for CPU status register           */
    unsigned int cpu_sr = 0;	/* Prevent compiler warning                           */
    //
    cpu_sr = cpu_sr;	// Avoid warning msg
#endif

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU1 Start---\n");

    while(ciu_idufrmcnt_ch1 == 0);

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

    for(i = 0; i < 6; i++)
    {
        OS_ENTER_CRITICAL();
        count       = ciu_idufrmcnt_ch1 - 1;
        srcImgY     = PNBuf_sub1[count & 0x03];
        srcImgUV    = srcImgY + ciu_1_pnbuf_size_y;
        OS_ENTER_CRITICAL();

        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            exifSetImageResolution(1280, 720); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1280, 720, 1280);
            JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
            exifSetImageResolution(1920, 1072); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1920, 1072, 1920);
            JpegImagePixelCount = 1920 * 1072;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
            exifSetImageResolution(1600, 896); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1600, 896, 1600);
            JpegImagePixelCount = 1600 * 896;   //GetJpegImagePixelCount();
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(704, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 480, 704);
                JpegImagePixelCount = 704 * 480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(704, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 576, 704);
                JpegImagePixelCount = 704 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(720, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 480, 720);
                JpegImagePixelCount = 720 * 480;    //GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(720, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 576, 720);
                JpegImagePixelCount = 720 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            exifSetImageResolution(640, 352); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 352, 640);
            JpegImagePixelCount = 640 * 352;    //GetJpegImagePixelCount();
        }
        else
        {
            exifSetImageResolution(640, 480); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 480, 640);
            JpegImagePixelCount = 640 * 480;    //GetJpegImagePixelCount();
        }
        primaryImageSize    = WaitJpegEncComplete();

        if((exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9))  // JPEG bitstream good
        {
            break;
        }
        else            // JPEG bitstream fail, 用較低的品直再壓一次
        {
            switch(i)
            {
                case 0:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 1:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                    break;
                case 2:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                    break;
                case 3:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(10)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 10);
                    break;
                case 4:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(0)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 0);
                    break;
                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
            }
        }
    }

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;
#endif


    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);



    //----judge memory full or not -----
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster        = (exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + bytes_per_cluster - 1) / bytes_per_cluster;

    if ((diskInfo->avail_clusters <= (used_cluster + 2))  ) // protect write miss
    {

        DEBUG_SYS("Memory full\n");
        DEBUG_SYS("avail_clusters               = %d\n", diskInfo->avail_clusters);
        DEBUG_SYS("used_cluster                 = %d\n", used_cluster);
        DEBUG_SYS("exifPrimaryImageHeaderSize   = %d\n", exifPrimaryImageHeaderSize);
        DEBUG_SYS("primaryImageSize             = %d\n", primaryImageSize);
#if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        osdDrawMemFull(UI_OSD_CLEAR);
#endif
    }
    else
    {
        exifWriteFile(thumbnailImageSize, primaryImageSize, 1);
    }

    if(i != 0)  // 若Q Table有被變過就還原Q table的預設值
        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);

    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU1 End---\n");

    return 1;

}

s32 sysCaptureImage_One_OnPreview420_CIU2(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8  *srcImgY, *srcImgUV;
    u32 count;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    //
    cpu_sr = cpu_sr; // Avoid warning msg
    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU2 Start---\n");

    while(ciu_idufrmcnt_ch2 == 0);

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

    for(i = 0; i < 6; i++)
    {
        OS_ENTER_CRITICAL();
        count       = ciu_idufrmcnt_ch2 - 1;
        srcImgY     = PNBuf_sub2[count & 0x03];
        srcImgUV    = srcImgY + ciu_2_pnbuf_size_y;
        OS_ENTER_CRITICAL();

        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            exifSetImageResolution(1280, 720); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1280, 720, 1280);
            JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
        {
            exifSetImageResolution(1920, 1072); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1920, 1072, 1920);
            JpegImagePixelCount = 1920 * 1072;   //GetJpegImagePixelCount();
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
        {
            exifSetImageResolution(1600, 896); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 1600, 896, 1600);
            JpegImagePixelCount = 1600 * 896;   //GetJpegImagePixelCount();
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(704, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 480, 704);
                JpegImagePixelCount = 704 * 480;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(704, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 704, 576, 704);
                JpegImagePixelCount = 704 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(720, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 480, 720);
                JpegImagePixelCount = 720 * 480;    //GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(720, 576); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 720, 576, 720);
                JpegImagePixelCount = 720 * 576;    //GetJpegImagePixelCount();
            }
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            exifSetImageResolution(640, 352); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 352, 640);
            JpegImagePixelCount = 640 * 352;    //GetJpegImagePixelCount();
        }
        else
        {
            exifSetImageResolution(640, 480); //暫時用
            jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                     srcImgY,
                                     srcImgUV,
                                     JPEG_OPMODE_FRAME, 640, 480, 640);
            JpegImagePixelCount = 640 * 480;    //GetJpegImagePixelCount();
        }
        primaryImageSize    = WaitJpegEncComplete();

        if((exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9))  // JPEG bitstream good
        {
            break;
        }
        else            // JPEG bitstream fail, 用較低的品直再壓一次
        {
            switch(i)
            {
                case 0:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 1:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                    break;
                case 2:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                    break;
                case 3:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(10)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 10);
                    break;
                case 4:
                    DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                    DEBUG_SYS("jpegSetQuantizationQuality(0)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 0);
                    break;
                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
            }
        }
    }

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
    SYS_CTL0            = sys_ctl0_status;
#endif


    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);



    //----judge memory full or not -----
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    used_cluster        = (exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + bytes_per_cluster - 1) / bytes_per_cluster;

    if ( (diskInfo->avail_clusters <= (used_cluster + 2))  ) // protect write miss
    {

        DEBUG_SYS("Memory full\n");
        DEBUG_SYS("avail_clusters               = %d\n", diskInfo->avail_clusters);
        DEBUG_SYS("used_cluster                 = %d\n", used_cluster);
        DEBUG_SYS("exifPrimaryImageHeaderSize   = %d\n", exifPrimaryImageHeaderSize);
        DEBUG_SYS("primaryImageSize             = %d\n", primaryImageSize);
#if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);
        osdDrawMemFull(UI_OSD_CLEAR);
#endif
    }
    else
    {
        exifWriteFile(thumbnailImageSize, primaryImageSize, 1);
    }

    if(i != 0)  // 若Q Table有被變過就還原Q table的預設值
        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);

    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU2 End---\n");

    return 1;

}


s32 sysCaptureImage_One_OnPreview420_CIU5(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u8  *srcImgY, *srcImgUV;
    u32 count;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

#if SNAPSHOT_DEBUG
    u32 j;
#endif
    //
    cpu_sr = cpu_sr;	// Avoid warning msg
    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU5 Start---\n");

    while(ciu_idufrmcnt_ch5 == 0);

#if SNAPSHOT_DEBUG
    CIU_5_CTL2 &= (~0x00000020);
#endif

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif

#if SNAPSHOT_DEBUG
    for(j=0; j<20; j++)
    {
#endif
        for(i = 0; i < 6; i++)
        {
            OS_ENTER_CRITICAL();
            count       = ciu_idufrmcnt_ch5 - 1;
            srcImgY     = PNBuf_sub5[count & 0x03];
            srcImgUV    = srcImgY + PNBUF_SIZE_Y;
            OS_ENTER_CRITICAL();

            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                exifSetImageResolution(1280, 720); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 1280, 720, 1280);
                JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
            {
                exifSetImageResolution(1920, 1072); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 1920, 1072, 1920);
                JpegImagePixelCount = 1920 * 1072;   //GetJpegImagePixelCount();
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
            {
                exifSetImageResolution(1600, 896); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 1600, 896, 1600);
                JpegImagePixelCount = 1600 * 896;   //GetJpegImagePixelCount();
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
            {
                if(sysTVinFormat == TV_IN_NTSC)
                {
                    exifSetImageResolution(704, 480); //暫時用
                    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                             srcImgY,
                                             srcImgUV,
                                             JPEG_OPMODE_FRAME, 704, 480, 704);
                    JpegImagePixelCount = 704 * 480;//GetJpegImagePixelCount();
                }
                else
                {
                    exifSetImageResolution(704, 576); //暫時用
                    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                             srcImgY,
                                             srcImgUV,
                                             JPEG_OPMODE_FRAME, 704, 576, 704);
                    JpegImagePixelCount = 704 * 576;    //GetJpegImagePixelCount();
                }
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
            {
                if(sysTVinFormat == TV_IN_NTSC)
                {
                    exifSetImageResolution(720, 480); //暫時用
                    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                             srcImgY,
                                             srcImgUV,
                                             JPEG_OPMODE_FRAME, 720, 480, 720);
                    JpegImagePixelCount = 720 * 480;    //GetJpegImagePixelCount();
                }
                else
                {
                    exifSetImageResolution(720, 576); //暫時用
                    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                             srcImgY,
                                             srcImgUV,
                                             JPEG_OPMODE_FRAME, 720, 576, 720);
                    JpegImagePixelCount = 720 * 576;    //GetJpegImagePixelCount();
                }
            }
            else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                exifSetImageResolution(640, 352); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 640, 352, 640);
                JpegImagePixelCount = 640 * 352;    //GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(640, 480); //暫時用
                jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
                                         srcImgY,
                                         srcImgUV,
                                         JPEG_OPMODE_FRAME, 640, 480, 640);
                JpegImagePixelCount = 640 * 480;    //GetJpegImagePixelCount();
            }
            primaryImageSize    = WaitJpegEncComplete();

            if((exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                    (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9))  // JPEG bitstream good
            {
                break;
            }
            else            // JPEG bitstream fail, 用較低的品直再壓一次
            {
                switch(i)
                {
                    case 0:
                        DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                        DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                        break;
                    case 1:
                        DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                        DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);
                        break;
                    case 2:
                        DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                        DEBUG_SYS("jpegSetQuantizationQuality(20)\n");
                        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 20);
                        break;
                    case 3:
                        DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                        DEBUG_SYS("jpegSetQuantizationQuality(10)\n");
                        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 10);
                        break;
                    case 4:
                        DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                        DEBUG_SYS("jpegSetQuantizationQuality(0)\n");
                        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 0);
                        break;
                    default:
                        DEBUG_SYS("JPEG encoder something wrong!!!\n");
                }
            }
        }

#if DINAMICALLY_POWER_MANAGEMENT
        sys_ctl0_status     = SYS_CTL0;
        sys_ctl0_status    &= (~SYS_CTL0_JPEG_CKEN);
        SYS_CTL0            = sys_ctl0_status;
#endif


        /* set related EXIF IFD ... */
        compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
        exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
        exifSetCopyRightVersion(VERNUM);



        //----judge memory full or not -----
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        used_cluster        = (exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + bytes_per_cluster - 1) / bytes_per_cluster;

        if ( (diskInfo->avail_clusters <= (used_cluster + 2)) ) // protect write miss
        {

            DEBUG_SYS("Memory full\n");
            DEBUG_SYS("avail_clusters               = %d\n", diskInfo->avail_clusters);
            DEBUG_SYS("used_cluster                 = %d\n", used_cluster);
            DEBUG_SYS("exifPrimaryImageHeaderSize   = %d\n", exifPrimaryImageHeaderSize);
            DEBUG_SYS("primaryImageSize             = %d\n", primaryImageSize);
#if (UI_PREVIEW_OSD == 1)
            uiOSDPreviewInit();
            osdDrawMemFull(UI_OSD_DRAW);
            OSTimeDly(20);
            osdDrawMemFull(UI_OSD_CLEAR);
#endif
        }
        else
        {
            exifWriteFile(thumbnailImageSize, primaryImageSize, 1);
        }

        if(i != 0)  // 若Q Table有被變過就還原Q table的預設值
            jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);

#if SNAPSHOT_DEBUG
    }
#endif

    if(sysTVOutOnFlag) //TV-out
    {
        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;
        else
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
    }

#if SNAPSHOT_DEBUG
    CIU_5_CTL2 |= 0x00000020;
#endif
    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU2 End---\n");

    return 1;

}


s32 sysVideoCaptureRoot(s32 dummy)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u8 err;

#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif
    CaptureVideoRun = 1;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_CLR, &err);		/* set to start record */
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_REC, OS_FLAG_SET, &err);		/* set to record  */
    /*CY 0613 S*/

#if SD_CARD_DISABLE
#else
    if (Write_protet() && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        osdDrawProtect(2);
        CaptureVideoRun = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_SET, &err);
        sysProjectVideoCaptureRoot(1);
        sysCaptureVideoStart = 0;
        return 0;
    }
#endif


#if ( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FID_INT_disa();
#endif
#endif

    sysCaptureVideoStart = 1 - sysCaptureVideoStart;
    if (sysCaptureVideoStart)
    {
        sysProjectVideoCaptureRoot(2);
        sysCaptureVideoStop = 0;
        ZoomFactorBackup = sysPreviewZoomFactor;

        sysCaptureVideo(sysPreviewZoomFactor);  /*BJ 0530 S*/
        //sysCaptureVideoStart=0;

        if (pwroff == 1)//power off
        {
            //gpioSetLevel(0, 1, 0);
            sysPowerOff(1);
        }
#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
      (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
        //Lucian: 不需回到preview mode, 會再啟動錄影.
#else
        sysProjectVideoCaptureRoot(3);
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
        time1=OSTimeGet();
        printf("System Stop Capture Time =%d (x50ms)\n",time1);
#endif
    }
    /*CY 0613 E*/

    sysProjectVideoCaptureRoot(4);

    if (gInsertNAND==1)
        userClickvideoRec=1;
    CaptureVideoRun = 0;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_SET, &err);
    return 1;
#endif
}

s32 sysCaptureVideo_Init(void) /*BJ 0530 S*/
{

#if DINAMICALLY_POWER_MANAGEMENT
    /* Peter */
    u32     sys_ctl0_status;

    //--------Power Control-----//
    // disable unused module for reduce power consumption
#if ((Sensor_OPTION  == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_OV2643_YUV601) || (Sensor_OPTION  == Sensor_CCIR601) || (Sensor_OPTION  == Sensor_CCIR656) || (Sensor_OPTION  == Sensor_OV7725_YUV601) || (Sensor_OPTION  == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_PC1089_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601) || (Sensor_OPTION == Sensor_PO2210K_YUV601) || (Sensor_OPTION == Sensor_ZN220_YUV601) || (Sensor_OPTION == Sensor_XC7021_SC2133) || (Sensor_OPTION == Sensor_NT99230_YUV601) || (Sensor_OPTION == Sensor_XC7021_GC2023))
    sys_ctl0_status     = SYS_CTL0;

#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif

    sys_ctl0_status    |= SYS_CTL0_SIU_CKEN |
                          SYS_CTL0_ISU_CKEN |
                          SYS_CTL0_SCUP_CKEN |
                          SYS_CTL0_SER_MCKEN |
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                          SYS_CTL0_MPEG4_CKEN |
#elif(VIDEO_CODEC_OPTION == H264_CODEC)
                          SYS_CTL0_H264_CKEN |
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
                          SYS_CTL0_JPEG_CKEN |   //Lsk : Q2 where to disable
#endif
                          SYS_CTL0_IIS_CKEN;
    sys_ctl0_status    &=
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        ~SYS_CTL0_JPEG_CKEN &
#endif
        ~SYS_CTL0_IPU_CKEN &
        ~SYS_CTL0_HIU_CKEN &
        // ~SYS_CTL0_USB_CKEN &
        ~SYS_CTL0_NAND_CKEN &
        ~SYS_CTL0_SRAM_CKEN;
    SYS_CTL0            = sys_ctl0_status;

#else
    sys_ctl0_status     = SYS_CTL0;

#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif
    sys_ctl0_status    |= SYS_CTL0_SIU_CKEN |
                          SYS_CTL0_IPU_CKEN |
                          SYS_CTL0_SRAM_CKEN |
                          SYS_CTL0_ISU_CKEN |
                          SYS_CTL0_SCUP_CKEN |
                          SYS_CTL0_SER_MCKEN |
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                          SYS_CTL0_MPEG4_CKEN |
#elif(VIDEO_CODEC_OPTION == H264_CODEC)
                          SYS_CTL0_H264_CKEN |
#else
                          SYS_CTL0_JPEG_CKEN |   //Lsk : Q2 where to disable
#endif
                          SYS_CTL0_IIS_CKEN;

    sys_ctl0_status    &=
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        ~SYS_CTL0_JPEG_CKEN;
#endif

    SYS_CTL0            = sys_ctl0_status;
#endif
#endif

    //----DRAM Chanel/AHB piority setting------//
    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD | SYS_ARBHIPIR_SIU | SYS_ARBHIPIR_ISU;

    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

    return 1;

}
s32 sysSensorFlip(s32 dummy)
{
    u8 level;
    u32 bitset = 0x00000001;

    gpioGetLevel(0, 0, &level);
    if (level)
    {
        //High
        Gpio0InIntRiseEdge &= ~bitset;
        Gpio0InIntFallEdge |= bitset;
        i2cWrite_SENSOR(0x0c,0x10);
        i2cWrite_SENSOR(0x32,0x0);

        //DEBUG_SYS("high\n\r");
    }
    else
    {
        //Low
        Gpio0InIntFallEdge &= ~bitset;
        Gpio0InIntRiseEdge |= bitset;
        i2cWrite_SENSOR(0x0c,0xD0);
        i2cWrite_SENSOR(0x32,0x40);
        //DEBUG_SYS("low\n\r");
    }
    return 1;
}

void sysTVswitchResolutionbyImagesize(void)
{}

s32 sysCaptureVideo(s32 ZoomFactor) /*BJ 0530 S*/
{
    u32 free_size;
    FS_DISKFREE_T* diskInfo;
    u32 bytes_per_cluster;
    int i;
    u8 mdset;


    sysCaptureVideo_Init(); /*BJ 0530 S*/
    tvTVE_INTC=TV_INTC_ALL_DISA;  //Lucian: disable IDU/TV interrupt. 避免畫面閃動.


#if AUDIO_IN_TO_OUT
    iisPreviewI2OEnd();
#endif

#if (AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    Close_IIS_ALC5621();
#endif


    sysCheckZoomRun_flag=0;
    //------------Check Free Space---------------//
    if ((sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA) && (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY))
    {
        free_size = dcfGetMainStorageFreeSize();
        //Check filesystem capacity
        switch(dcfOverWriteOP)
        {
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            case DCF_OVERWRITE_OP_OFF:
                break;
            case DCF_OVERWRITE_OP_01_DAYS:
            case DCF_OVERWRITE_OP_07_DAYS:
            case DCF_OVERWRITE_OP_30_DAYS:
            case DCF_OVERWRITE_OP_60_DAYS:
                /*while(free_size < DCF_OVERWRITE_THR_KBYTE)
                {
                    if(dcfOverWriteDel()==0)
                    {
                        DEBUG_DCF("Over Write delete fail!!\n");
                        return 0;
                    }
                    //due to only update global_diskInfo when clos file, so we must calculate when open file
                    free_size = dcfGetMainStorageFreeSize();
                    DEBUG_SYS("Free space = %d (KB)\n", free_size);
                }*/
                if(dcfOverWriteOP >= dcfGetTotalDirCount())
                    break;
                sysbackLowSetEvt(SYSBACKLOW_EVT_OVERWRITEDEL, dcfOverWriteOP, 0, 0, 0);
                break;
#endif
            default:
                while((free_size < DCF_OVERWRITE_THR_KBYTE))  //Lucian: 錄影前先清除出檔案空間, 檔案個數.
                {
                    // Find the oldest file pointer and delete it
                    if(dcfOverWriteDel() == 0)
                    {
                        DEBUG_SYS("Over Write delete fail!!\n");
                        return 0;
                    }
                    else
                    {
                        sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_OVERWRITE_DELETE_PASS, 0);
                        //DEBUG_SYS("Over Write delete Pass!!\n");
                    }
                    free_size = dcfGetMainStorageFreeSize();
                    DEBUG_SYS("Free space = %d (KB)\n", free_size);
                }
                break;
        }
    }
//-----------做系統升頻 48MHz --> 64MHz ---------------//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_RISE_FREQUENCY, 0);
//----------------------------------------------------//

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
    Init_IIS_WM8940_rec();
#elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
    OSTimeDly(5);//avoid getting noise when enable codec
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#endif


    mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100);
    DEBUG_SYS("Trace: Capture video...\n");

    siuAeEnable = 0;

    // stop preview process
#if MULTI_CHANNEL_VIDEO_REC
    for(i = 0; i < MULTI_CHANNEL_LOCAL_MAX; i++)
    {
        if(MULTI_CHANNEL_SEL & (1 << i))
        {
            switch(i)
            {
                case 0:
                    isuStop();
                    ipuStop();
                    siuStop();
                    break;

                case 1:
                    ciu_1_Stop();
                    break;

                case 2:
                    ciu_2_Stop();
                    break;

                case 3:
                    ciu_3_Stop();
                    break;

                case 4:
                    ciu_4_Stop();
                    break;

                case 5:
                    ciu_5_Stop();
                    ipuStop();
                    siuStop();
                    break;
            }
        }
    }
#else
    switch(sysVideoInCHsel)
    {
        case 0:
            isuStop();
            ipuStop();
            siuStop();
            break;

        case 1:
            ciu_1_Stop();
            break;

        case 2:
            ciu_2_Stop();
            break;
        case 3:
            ciu_3_Stop();
            break;

        case 4:
            ciu_4_Stop();
            break;

        case 5:
            ciu_5_Stop();
            ipuStop();
            siuStop();
            break;
    }
#endif    // #if MULTI_CHANNEL_VIDEO_REC




#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
      (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
    //do nothing.
#else
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_CLEAR_BUFFER, 0);
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_CHANGE_CHANNEL, 0);
    if( ((mpeg4Width == 320) || (mpeg4Width == 352)) && !sysTVOutOnFlag) //Lsk 090707: QVGA clear video buffer
    {
        IduVideo_ClearPKBuf(0);
        IduVideo_ClearPKBuf(1);
        IduVideo_ClearPKBuf(2);
    }
#endif

    // enable preview process for mpeg4
    iisCaptureVideoInit();    /* Peter 070104 */

    siuVideoZoomSect=0xf;

    // initialize sensor
    mdset=MotionDetect_en;
    if(mdset)
    {
#if HW_MD_SUPPORT
        mduMotionDetect_ONOFF(0);
#endif
    }
    //while (SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_MPEGAVI,ZoomFactor);
    setSensorWinSize(ZoomFactor, SIUMODE_MPEGAVI);
    siuOpMode = SIUMODE_MPEGAVI;
    siuAdjustAE(AECurSet); //Lucian: Preview->Video, AE index transfer, Set Sensor Again.

    if(mdset)
    {
#if HW_MD_SUPPORT
        mduMotionDetect_ONOFF(1);
#endif
    }
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_CHECK_SET_60FPS, 0);

#if ( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FID_INT_ena();
#endif
#endif

#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
    //---------MP4 file format------//
    mp4CaptureVideo(ZoomFactor);    /* capture and write mp4 file */
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
    //--------ASF file format------//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_WRITE_ASF_FILE, ZoomFactor);
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)   /*Peter 0704 S*/
    //-----------AVI file format---------//
    aviCaptureVideo(ZoomFactor, sysCaptureVideoMode);    /* capture and write avi file */
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
    //----------MOV file format----------//
    movCaptureVideo(ZoomFactor, sysCaptureVideoMode);    /* capture and write asf file */
#endif                          /*Peter 0704 E*/

//-----------啟動Timer-1-------------//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_SET_SIU_MODE, 0);

    timerInterruptEnable(1,0);
    OSMboxPost(general_MboxEvt, "PASS");		/* post mail box to announce the event of video capture is stopped and finished. */

    sysVoiceRecStart=0;

//-----系統降頻  64MHz -->48MHz ------//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_FALL_FREQUENCY, 0);
//-----------------------------------//

#if (AUDIO_DEVICE == AUDIO_IIS_WM8974)
    IIS_WM8974_reset();
#elif (AUDIO_DEVICE == AUDIO_IIS_WM8940)
    IIS_WM8940_reset();
#elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
    ac97SetupALC203_pwd();
#endif

    sysPreviewZoomFactor = ZoomFactorBackup;
    system_busy_flag=0;

    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_SET_LED, 0);

#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    //Lucian: Wait until "Delete FAT Link" complete.
    if(sysDeleteFATLinkOnRunning)
    {
        DEBUG_SYS("Wait Delete-FAT-Link Complete.\n");
        osdDrawFillWait();
        while(sysDeleteFATLinkOnRunning)
            OSTimeDly(10);
    }
#endif
    dcfCacheClean(); //Lucian: Write back to SD card.

    //----Do power-off, if need-----//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_POWER_OFF, 0);

    return 1;
}

s32 sysBack_Check_TVinFormat(s32 dummy)
{
    u8 err;
    u8 status,cnt=0;
    u32 TV_format;

    if(sysVideoInSel == VIDEO_IN_SENSOR)
        return 0;

    //
    status = status;
    cnt = cnt;	// Avoid warning msg
    //
#if(TV_DECODER == BIT1605) //use bit1605 tv decoder

    //DEBUG_UI("sysback check BIT1605 is locked to the video signal\n");
    i2cRead_BIT1605(0x7A,&status);  //check TVP5150 is locked to the video signal, if no video signal input, max check 100
    while(((status&0x04) != 0x04) && (cnt < 10))
    {
        cnt++;
        i2cRead_BIT1605(0x7A,&status);  //check Bitek1604 is locked to the video signal, if no video signal input, max check 100
    }
    if(cnt < 10)
    {
        //DEBUG_UI("sysback BIT1605 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        //DEBUG_UI("sysback BIT1605 can't lock the video signal, use default format\n");
    }
#elif(TV_DECODER == TI5150) //use TI5150 tv decoder
    //DEBUG_UI("sysback check TVP5150 is locked to the video signal\n");
    i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);  //check TVP5150 is locked to the video signal, if no video signal input, max check 100
    while(((status&0x80) != 0x80) && (cnt < 10))
    {
        cnt++;
        i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);
    }
    if(cnt < 10)
    {
        //DEBUG_UI("sysback TI5150 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        //DEBUG_UI("sysback TI5150 can't lock the video signal, use default TV in format-NTSC\n");
    }

#elif(TV_DECODER == WT8861) //use WT8861 tv decoder
    //DEBUG_UI("sysback check WT8861 is locked to the video signal\n");
    i2cRead_WT8861(0x3A,&status,I2C_WT8861_RD_SLAV_ADDR);  //check WT8861 is locked to the video signal, if no video signal input, max check 100
    while(((status&0x0F) != 0x0E) && (cnt < 10))
    {
        cnt++;
        i2cRead_WT8861(0x3A,&status,I2C_WT8861_RD_SLAV_ADDR);
    }
    if(cnt < 10)
    {
        i2cWrite_WT8861(0x3f,0x00,I2C_WT8861_WR_SLAV_ADDR);
        DEBUG_UI("sysback WT8861 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        DEBUG_UI("sysback WT8861 can't lock the video signal, use default TV in format-NTSC\n");
    }
#elif(TV_DECODER == TW9900) //use WT8861 tv decoder
    //DEBUG_UI("sysback check TW9900 is locked to the video signal\n");
    i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_ADDR);  //check TW9900 is locked to the video signal, if no video signal input, max check 100
    //DEBUG_I2C("status=%d \n\n",status);
    while(((status&0x80) != 0x00) && (cnt < 10))
    {
        cnt++;
        i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_ADDR);
    }
    if(cnt < 10)
    {
        DEBUG_UI("sysback TW9900_1 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        DEBUG_UI("sysback TW9900_1 can't lock the video signal, use default TV in format-NTSC\n");
    }

#elif(TV_DECODER == TW9910) //use WT8861 tv decoder
    //DEBUG_UI("sysback check TW9910 is locked to the video signal\n");
    i2cRead_TW9910(0x01,&status,I2C_TW9910_RD_SLAV_ADDR);  //check TW9900 is locked to the video signal, if no video signal input, max check 100
    //DEBUG_I2C("status=%d \n\n",status);
    while(((status&0x80) != 0x00) && (cnt < 10))
    {
        cnt++;
        i2cRead_TW9910(0x01,&status,I2C_TW9910_RD_SLAV_ADDR);
    }
    if(cnt < 10)
    {
        DEBUG_UI("sysback TW9910_1 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        DEBUG_UI("sysback TW9910_1 can't lock the video signal, use default TV in format-NTSC\n");
    }
#elif(TV_DECODER == MI9V136)

#endif  /*end of #if(TV_DECODER == BIT1605)*/

    TV_format=getTVinFormat();

    if( (sysTVinFormat != TV_format) && (sysTVInFormatLocked == TRUE))
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_TVinFormat, OS_FLAG_CLR, &err);

        sysTVinFormat = TV_format;
        DEBUG_SYS("-->TV-in Format is Changed!,sysTVinFormat=%d\n",sysTVinFormat);
        if(sysTVinFormat  == TV_IN_PAL)
        {
            TvOutMode = SYS_TV_OUT_PAL;
#if CIU_SUPPORT_EN            
        #if (MULTI_CHANNEL_SEL & 0x02)
            ciu1_ChangeInputSize(640,576/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x04)
            ciu2_ChangeInputSize(640,576/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x08)
            ciu3_ChangeInputSize(640,576/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x10)
            ciu4_ChangeInputSize(640,576/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x20)
            ciu5_ChangeInputSize(640,576/2);
        #endif
#endif
        }
        else
        {
            TvOutMode = SYS_TV_OUT_NTSC;
#if CIU_SUPPORT_EN            
            
         #if (MULTI_CHANNEL_SEL & 0x02)
            ciu1_ChangeInputSize(640,480/2);
         #endif

         #if (MULTI_CHANNEL_SEL & 0x04)
            ciu2_ChangeInputSize(640,480/2);
         #endif

         #if (MULTI_CHANNEL_SEL & 0x08)
            ciu3_ChangeInputSize(640,480/2);
         #endif

         #if (MULTI_CHANNEL_SEL & 0x10)
            ciu4_ChangeInputSize(640,480/2);
         #endif

         #if (MULTI_CHANNEL_SEL & 0x20)
            ciu5_ChangeInputSize(640,480/2);
         #endif
#endif
        }

#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
      (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
        uiCaptureVideoStop();
        uiCaptureVideo();
#endif

        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_TVinFormat, OS_FLAG_SET, &err);
    }
    return 1;

}

s32 sysBack_Set_Sensor_Color(s32 dummy)
{
    u8  level = 0;

    if(level == 0)  /*color*/
    {
        i2cWrite_SENSOR( 0xda, 0x02); //Saturation disable
        i2cWrite_SENSOR( 0xdd, 0x40); //U saturation
        i2cWrite_SENSOR( 0xde, 0x40); //V saturation
    }
    else    /*monochrome */
    {
        i2cWrite_SENSOR( 0xda, 0x02);  //Saturation enable
        i2cWrite_SENSOR( 0xdd, 0x00);  //U saturation
        i2cWrite_SENSOR( 0xde, 0x00);  //V saturation
    }
    return 1;
}

s32 sysBack_ScheduleMode(s32 dummy)
{

}

//-------//
#if(TV_DECODER == BIT1605) //use bit1605 tv decoder
u32 getTVinFormat()
{
    u8 data,level;
    u32 TVformat;
#if(TVIN_FORMAT_DETECT_METHOD == TV_IN_DETECT_BY_PCB)
    //TV-in format select: 由PCB 上電阻決定  HIGH(NTSC), LOW(PAL)
    gpioGetLevel(GPIO_DETECT_TVINFORMAT_GRP, GPIO_DETECT_TVINFORMAT_BIT, &data);
    if(data)
        TVformat=TV_IN_NTSC;
    else
        TVformat=TV_IN_PAL;
#else
    /* read the video standard status from BIT1650 auto-detected */
    i2cRead_BIT1605(0x79, &data);
    data = (data & 0x07) ;
    switch(data)
    {
        case 0:
        case 1:
        case 3:
            TVformat = TV_IN_PAL;
            break;
        case 4:
        case 5:
        case 6:
            TVformat = TV_IN_NTSC;
            break;
        default:
            TVformat = TV_IN_PAL;
            break;
    }

    if(TVformat == TV_IN_NTSC) //Lucian: BITEK TV decoder 的起始點微調.
    {
        sensor_validsize.imgStr.x = SENSOR_NTSC_START_X;
        sensor_validsize.imgStr.y = SENSOR_NTSC_START_Y;
        SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) | (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);

    }
    else if(TVformat == TV_IN_PAL)
    {
        sensor_validsize.imgStr.x = SENSOR_PAL_START_X;//163;
        sensor_validsize.imgStr.y = SENSOR_PAL_START_Y;
        SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) | (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
    }
#endif
    return TVformat;
}

#elif(TV_DECODER == TI5150) //use TI5150 tv decoder
u32 getTVinFormat()
{
    u8 data;
    u8 FieldRate;
    u32 TVformat;


#if(TVIN_FORMAT_DETECT_METHOD == TV_IN_DETECT_BY_PCB)
    //TV-in format select: 由PCB 上電阻決定  HIGH(NTSC), LOW(PAL)
    /*
#if 1
    gpioGetLevel(GPIO_DETECT_TVINFORMAT_GRP, GPIO_DETECT_TVINFORMAT_BIT, &data);
    if(data)
       TVformat=TV_IN_NTSC;
    else
       TVformat=TV_IN_PAL;
#else
    TVformat=TV_IN_NTSC;
#endif
    */

#else

    /* read the video standard status from TVP5150 auto-detected */
    i2cRead_TVP5150(0x88, &data,TV_CHECK_FORMAT_RD_ADDR);
    FieldRate = (data & 0x20) >> 5;
    i2cRead_TVP5150(0x8c, &data,TV_CHECK_FORMAT_RD_ADDR);

    data = (data & 0x0E) >> 1;

    //DEBUG_SIU("Video standard = %d\n",data);
#if (TVIN_FORMAT_DETECT_MODE  == TV_IN_FORMAT_DETECT_ONCE )
    if(sysTVInFormatLocked == FALSE)
        return TV_IN_NTSC;

    switch(data)
    {

        case 0: /*(M) NTSC ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x02,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_NTSC;
            break;

        case 1: /*(B, G, H, I, N) PAL ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x04,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_PAL;
            break;

        case 2: /*(M) PAL ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x06,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_PAL;
            break;

        case 3: /*PAL-N ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x08,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_PAL;
            break;

        case 4: /*NTSC 4.43 ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x0A,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_NTSC;
            break;

        case 5: /*SECAM ITU-R BT.601*/
            i2cWrite_TVP5150(0x28, 0x0C,TV_CHECK_FORMAT_WR_ADDR);
            TVformat = TV_IN_PAL;
            break;

        default:
            i2cWrite_TVP5150(0x28, 0x00,TV_CHECK_FORMAT_WR_ADDR);   /*auto*/
            TVformat=TV_IN_NTSC;
            break;
    }
#else
    switch(data)
    {

        case 0:
        case 4:
            if(FieldRate == TV_IN_60HZ)
                TVformat = TV_IN_NTSC;
            else
                DEBUG_SIU("TI5150 detect error. NTSC,50HZ...\n");
            break;

        case 1:
        case 2:
        case 3:
        case 5:
            if(FieldRate == TV_IN_50HZ)
                TVformat = TV_IN_PAL;
            else
                DEBUG_SIU("TI5150 detect error. PAL,60HZ...\n");
            break;

        default:
            TVformat=TV_IN_NTSC;
            break;
    }
#endif

#endif
    return TVformat;
}
#elif(TV_DECODER == WT8861)
u32 getTVinFormat()
{
    u8 data;
    u32 TVformat=0;
#if(TVIN_FORMAT_DETECT_METHOD == TV_IN_DETECT_BY_PCB)
    //TV-in format select: 由PCB 上電阻決定  HIGH(NTSC), LOW(PAL)
#if 1
    gpioGetLevel(GPIO_DETECT_TVINFORMAT_GRP, GPIO_DETECT_TVINFORMAT_BIT, &data);
    if(data)
        TVformat=TV_IN_NTSC;
    else
        TVformat=TV_IN_PAL;
#else
    TVformat=TV_IN_NTSC;
#endif
#else
    if(sysTVInFormatLocked == FALSE)
        return TV_IN_NTSC;
    /* read the video standard status from BIT1650 auto-detected */
    i2cRead_WT8861(0x3C, &data,I2C_WT8861_RD_SLAV_ADDR);
    data = (data & 0x07) ;
    switch(data)
    {
        case 0:
            TVformat = TV_IN_NTSC;
            DEBUG_SYS("TVformat4=%d\n\n",TVformat);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
            TVformat = TV_IN_PAL;
            DEBUG_SYS("TVformat0=%d\n\n",TVformat);
            break;
        default:
            TVformat = TV_IN_NTSC;
            DEBUG_SYS("TVformat=%d\n\n",TVformat);
            break;
    }
#endif
    //DEBUG_SYS("TVformat=%d\n\n",TVformat);
    return TVformat;
}
#elif(TV_DECODER == TW9900)
u32 getTVinFormat()
{
    u8 data;
    u32 TVformat=0;
#if(TVIN_FORMAT_DETECT_METHOD == TV_IN_DETECT_BY_PCB)
    //TV-in format select: 由PCB 上電阻決定  HIGH(NTSC), LOW(PAL)
#if 1
    gpioGetLevel(GPIO_DETECT_TVINFORMAT_GRP, GPIO_DETECT_TVINFORMAT_BIT, &data);
    if(data)
        TVformat=TV_IN_NTSC;
    else
        TVformat=TV_IN_PAL;
#else
    TVformat=TV_IN_NTSC;
#endif
#else
    if((sysTVInFormatLocked == FALSE) && (sysTVInFormatLocked1 == FALSE))
        return TV_IN_NTSC;
    /* read the video standard status from BIT1650 auto-detected */
    i2cRead_TW9900(0x1C, &data,I2C_TW9900_RD_SLAV_ADDR);
    //        DEBUG_SYS("Old data=0x%x\n\n",data);
    data = (data & 0x70) ;
    data = data>>4 ;
    switch(data)
    {
        case 0:
            TVformat = TV_IN_NTSC;
            //DEBUG_SYS("TVformat4=%d\n\n",TVformat);
            break;
        case 1:
            TVformat = TV_IN_PAL;
            //DEBUG_SYS("TVformat0=%d\n\n",TVformat);
            break;
        default:
            TVformat = TV_IN_NTSC;
            //DEBUG_SYS("TVformat=%d\n\n",TVformat);
            break;
    }
#endif

    //DEBUG_SYS("TVformat=%d\n\n",TVformat);
    return TVformat;
}
#elif(TV_DECODER == TW9910)
u32 getTVinFormat()
{
    u8 data;
    u32 TVformat=0;
#if(TVIN_FORMAT_DETECT_METHOD == TV_IN_DETECT_BY_PCB)
    //TV-in format select: 由PCB 上電阻決定  HIGH(NTSC), LOW(PAL)
#if 1
    gpioGetLevel(GPIO_DETECT_TVINFORMAT_GRP, GPIO_DETECT_TVINFORMAT_BIT, &data);
    if(data)
        TVformat=TV_IN_NTSC;
    else
        TVformat=TV_IN_PAL;
#else
    TVformat=TV_IN_NTSC;
#endif
#else
    if(sysTVInFormatLocked == FALSE)
        return TV_IN_NTSC;
    /* read the video standard status from BIT1650 auto-detected */
    i2cRead_TW9910(0x1C, &data,I2C_TW9910_RD_SLAV_ADDR);
    //        DEBUG_SYS("Old data=0x%x\n\n",data);
    data &= 0x70;
    data = data>>4 ;
    switch(data)
    {
        case 0:
            TVformat = TV_IN_NTSC;
            //DEBUG_SYS("TVformat4=%d\n\n",TVformat);
            break;
        case 1:
            TVformat = TV_IN_PAL;
            //DEBUG_SYS("TVformat0=%d\n\n",TVformat);
            break;
        default:
            TVformat = TV_IN_NTSC;
            //DEBUG_SYS("TVformat=%d\n\n",TVformat);
            break;
    }
#endif

    //DEBUG_SYS("TVformat=%d\n\n",TVformat);
    return TVformat;
}
#elif(TV_DECODER == MI9V136)
u32 getTVinFormat()
{
    u32 TVformat=0;

    return TVformat;
}
#else
u32 getTVinFormat()
{
    u32 TVformat=sysTVinFormat;

    return TVformat;
}
#endif
//----------//
#endif
