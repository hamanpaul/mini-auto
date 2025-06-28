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
#include "movapi.h"
#include "timerapi.h"
#include "usbapi.h"
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
#if (TUTK_SUPPORT)
#include "p2pserver_api.h"
#endif

#include "project.h"


#include "ciuapi.h"
#include "rfiuapi.h"


//#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE ||(HW_BOARD_OPTION == MR6730_AFN)	
    #include "../../ui/inc/Menu.h"   // Menu_getSetItemLayout
    #include "../../ui/inc/MainFlow.h"  // UI_gotoStandbyMode
#endif

/*************************************************************************************************
* Constant
**************************************************************************************************/

/* define debug print */
#define sysDebugPrint


/*********************************************************************************************************
* Function prototype
*********************************************************************************************************/
#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
s32 sysTX8211_EnterWifi(s32 P2PQuailty);
s32 sysTX8211_LeaveWifi(s32 dummy);
#endif

void uiTask(void*);
s32 sysPreviewInit(s32);
s32 sysPreviewReset(s32);
s32 sysPreviewZoomInOut(s32 zoomFactor);
s32 sysVideoZoomInOut(s32 zoomFactor);

s32 sysTVInChannelChange_Preview(s32 zoomFactor);
s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor);

#if TX_SNAPSHOT_SUPPORT
s32 sysTxCaptureImage_CIU1(s32 Setting);
#endif

#if RX_SNAPSHOT_SUPPORT
s32 sysBack_RFI_RX_DataSave(s32 RFUnit);
#endif


s32 sysSnapshot(s32);
s32 sysSnapshot_OnPreview(s32 dummy);
s32 sysPlaybackInit(s32);
s32 sysPlaybackZoom(s32);
s32 sysPlaybackPan(s32);
s32 sysPlaybackMoveForward(s32);
s32 sysPlaybackMoveBackward(s32);
s32 sysPlaybackDelete(s32);
s32 sysPlaybackDeleteAll(s32);
s32 sysPlaybackDeleteDir(s32);
s32 sysTVPlaybackDelete(s32);
s32 sysPlaybackFormat(s32);
s32 sysPlaybackIsp(s32); /*CY 1023*/
s32 sysCaptureVideo_Init(void); /*BJ 0530 S*/

s32 sysSelfTimer(void); /*CY 0907*/
s32 sysCaptureImage(s32); /*BJ 0530 S*/
s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor, u8 BurstNum,u8 ScalingFactor); /*BJ 0530 S*/
s32 sysCaptureImage_OnRFRx(int dummy);

s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor);


s32 sysSetUiKey(s32);
s32 sysUpgradeFW(s32);
s32 sysCaptureImageSeries(s32); /*BJ 0530 S*/
s32 sysCaptureVideo(s32);  /*BJ 0530 S*/
s32 sysVideoCaptureRoot(s32);
s32 sysVideoCaptureStop(s32 VideoChannelID);
s32 sysVideoCaptureRestart(s32 VideoChannelID);
s32 sysPowerOff(s32);
s32 sysMacro(s32);
s32 sysLcdRot(s32);
s32 sysSensorFlip(s32);
s32 sysGetDiskFree(s32);
s32 sysSDCD_IN(s32);
s32 sysSDCD_OFF(s32);
s32 sysUSBCheck(s32);
s32 sysUIReafFile(s32);
s32 sysP2PReadFile(s32);
s32 sysPlaybackCalendar(s32 dummy);
s32 sysDrawWaitLoad(s32 dummy);
s32 sysWhiteLight(s32);
s32 sysFlashLight(s32);
s32 sysVOICE_REC(s32);
s32 sysReadFile(void);
s32 sysbackEXIFWrite(s32);
void sysPendMainPage(void);
void sysPostMainPage(void);
void sysSD_Disable(void);
void sysSD_Enable(void);
s32 sysBackLow_UI_KEY_SDCD(s32,s32,s32,s32);
void sysNAND_Disable(void);
void sysNAND_Enable(void);
void sysSPI_Disable(void);
void sysSPI_Enable(void);
s32 sysUsbRemoved(void);
void sysundefinedata(u8 index);
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
s32 sysBack_RFI_TX_SnapShot(s32 Setting);

#endif
s32 sysBack_Draw_Battery(s32 level);
s32 sysBack_Check_UI(s32 level);
s32 sysBack_Check_UI_500ms(s32 level);
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)||(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
    u32 sysBack_Check_TVinFormat(u32 dummy);
#endif
u32 sysBack_Check_VideoinSource(u32 dummy);
u32 sysBack_Set_Sensor_Color(u32 dummy);
s32 sysDrawTimeOnCapture(s32 dummy);
u32 sysBack_Draw_BitRate(u32 value);
u32 sysBack_Draw_FrameRate(int RFUnit);
u32 sysBack_Draw_OSDString(u32 value);
u32 sysBack_Draw_SD_Icon(u32 value);
#if(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
u32 sysBack_Turn_SPK_GPIO(u8 value);
#elif(HW_BOARD_OPTION  == MR8100_GCT_VM9710)
u32 sysBack_Turn_SPK_GPIO(u8 value);
#elif((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
u32 sysBack_Draw_Sound_Bar(u8 value);
#endif
s32 sysBackLow_Syn_RF(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3);


/*********************************************************************************************************
* External Functions
*********************************************************************************************************/
extern void uiClearFfQuadBuf(u8 index);
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

extern s32 asfRepairFile(signed char* file_name);
extern s32 isuCapturePreviewImg(s8 zoomFactor);
extern void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H);
extern void isu_FCINTE_ena(void);
extern void isu_FCINTE_disa(void);

extern void siu_FCINTE_ena(void);
extern void siu_FCINTE_disa(void);

extern s32 ispUpdateWaveFile(void);
extern int _FS__fat_DeleteFATLink_Back( int Idx, int Unit,int todo,int curclst);
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

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
extern void Init_IIS_WM8974_play(void);
extern void Init_IIS_WM8974_rec(void);
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
extern void Init_IIS_ALC5621_play(void);
extern void Init_IIS_ALC5621_rec(void);
extern void Init_IIS_ALC5621_bypass(void);
extern void Close_IIS_ALC5621();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
extern s32 ac97SetupALC203_play(void);
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
extern void init_DAC_play(u8 start);
#endif

extern void init_serial_A(void);
extern s32 timerConfig(u8 number, TIMER_CFG* pCfg);
extern s32 timerCountEnable(u8 number, u8 enable);
extern s32 timerCountPause(u8 number, u8 pause);
extern s32 iisStopPlay(void);
extern void siuAdjustAE(u16 cur_index);
extern s32 siuSensorInit(u8 siuSensorMode,u8 zoomFactor);
extern void siuSensorSet_60fps();
extern void IduVideo_ClearPKBuf(u8 bufinx);
extern s32 i2cRead_SENSOR(u8 addr, u16* pData);
extern s32 i2cWrite_SENSOR(u8 addr, u16 data);
extern s32 uiMenuSetAutoOff(s8 setting);
extern void osdDrawFillWait(void);
extern s32 iduSwitchPreview_TV(int src_W,int src_H);
extern int marsRfiu_FCC_DirectTXRX();

#if CLOUD_SUPPORT
extern void SendCloudMessage(u32 sID, u32 target);
#endif



/*********************************************************************************************************
* Extern Global Variable
*********************************************************************************************************/
#if RX_SNAPSHOT_SUPPORT
extern DATA_BUF_MNG rfiuRxDataBufMng[]; 
#endif


extern u8 uiMenuVideoSizeSetting;
extern u32 guiIISPlayDMAId;
extern u32 guiIISRecDMAId;
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
extern u8 gInsertCard;
extern u8 sysVolumnControl;
extern FS_DISKFREE_T global_diskInfo; //civic 070903
extern u32 RTCseconds;
extern u8 UI_update;  //civic 071002
extern char wav_on;     //civic 071011
extern unsigned char File_protect_on;
extern u8 system_busy_flag;
extern u8 displaybuf_idx;
extern u8 TvOutMode;
extern ISU_IOSize   MSFSz;
extern volatile s32 isu_idufrmcnt;  //Lucian: 取得isu writing ptr.
extern u8 siuOpMode; //Lucian: 用於CapturePreviewImg();
extern u8 Main_Init_Ready;

#if(FACTORY_TOOL == TOOL_ON)
extern u8 config_mode_enable[6];
#endif
u8 Iframe_flag = 0; // Decided get I-frame or play whole file
extern u8 batteryflag;
extern u8 VideoClipOnTV;
extern s32 ZoomFactorBackup;
extern u8 TVorEarphone;
extern u8 configsalix;
extern int fat_bpb_err;

extern s32 siuWBComp_RBRatio[4];
extern u8 siuAeEnable;
extern u8 siuAeReadyToCapImage;
extern u16 AECurSet;
extern u8 IISPplyback;
extern u8 gSdcerr;
extern u8 usb_msc_mode;

extern u16 OB_B;
extern u16 OB_Gb;
extern u16 OB_R;
extern u16 OB_Gr;
extern u8  osdYShift;
#if ( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION==A1018_FPGA_BOARD)|| \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
u8  videoPlayNext=1;
#else
extern u8  videoPlayNext;
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


extern u8 SD_detect_status;
extern OS_EVENT    *dcfReadySemEvt;
extern OS_EVENT    *gRfiuSWReadySemEvt;

extern u8 UI_SDLastLevel;


extern u8 PowerOn_HotPlug;

#if RF_TX_OPTIMIZE
extern OS_STK VideoTaskStack[]; 
#endif
#if INSERT_NOSIGNAL_FRAME
extern u8 Record_flag[MULTI_CHANNEL_MAX];
#endif
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
u8 Motion_Error_ststus[MULTI_CHANNEL_MAX] = {0,0,0,0}; 
#endif
extern u8 Fileplaying;
#if USB2WIFI_SUPPORT
extern u8 Snapshot_Error;
extern u8 Change_RSE;
extern u8 Reset_RES;
#endif

#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 || SW_APPLICATION_OPTION == MR8100_RFCAM_TX1)
extern int ciu1ZoomStart;
#endif

extern u8 OnlineUpdateStatus;

/*
 *********************************************************************************************************
 * Global Variable
 *********************************************************************************************************
 */
#if TX_SNAPSHOT_SUPPORT
 u32 sysRFTXSnapImgRdy=0;
#endif
#if USB2WIFI_SUPPORT
u8 P2P_Snapshot = 0;
u8 P2P_Snapshot_cnt = 0;
#endif

#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
u32 sys8211TXWifiStat=MR8211_QUIT_WIFI;
u32 sys8211TXWifiUserNum=1;
#endif

s32 gSystemStroageReady;
u8  sys_format = 0;  //  1 : success, 0 : error.
s32 sysFSType = 0;  // -1: Error, 0: None, 1: can use

 u8  sysBackOsdString[20];
 int sysStorageOnlineStat[1] = { 0 };  //Lucian: 檢查儲存裝置是否更新. 設零可強迫讀取BPB.

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

#if (Get_sametime == 1)  
RTC_DATE_TIME   same_localTime;
u8 sametime_Pair=0;
u16 sametime_UseCnt[2]={0,0};
RTC_DATE_TIME sametime_CurrTime[2];
#endif


OS_EVENT* sd_backSemEvt;
OS_EVENT* general_MboxEvt;
OS_EVENT* speciall_MboxEvt;
OS_FLAG_GRP  *gSysReadyFlagGrp;
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
u8  sysPlaybackVideoPause;
#if (AVSYNC == VIDEO_FOLLOW_AUDIO)
u8  sysPlaybackForward;
u8  sysPlaybackBackward;
#elif (AVSYNC == AUDIO_FOLLOW_VIDEO)
s8  sysPlaybackForward;
s8  sysPlaybackBackward;
#endif

u8 sysReady2CaptureVideo;
u8  sysPlaybackThumbnail;
u32 u32PacketStart;
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
u8  sysPlaybackCamList = 0x0F;
u8  sysDisplaySDCardFail = 0;
u8  syscheckSDStatus =0;
u8  sysEnMenu=0;
u8  sysEnZoom=0;
u8  sysEnSnapshot=0;

/*-------------------- event function -------------------*/
s32 (*sysEvtFunc[])(s32) =
    {
        sysPreviewInit,             /* 0x00 - preview initialization */
        sysPreviewReset,            /* 0x01 - preview zoom */
#if( (Sensor_OPTION == Sensor_OV7725_VGA) )
     #if(GET_BAYERDATA_SIU || GET_SIU_RAWDATA_PURE)
        sysSnapshot,                /* 0x02 - snapshot */
     #else
        sysSnapshot_OnPreview,      /* 0x02 - snapshot */
     #endif
#elif(Sensor_OPTION == Sensor_MI_5M)
        sysSnapshot,                /* 0x02 - snapshot */
#else
        sysSnapshot_OnPreview,      /* 0x02 - snapshot */
#endif
        sysPlaybackInit,            /* 0x03 - playback initialization */
        sysPlaybackZoom,            /* 0x04 - playback zoom */
        sysPlaybackPan,             /* 0x05 - playback pan */
        sysPlaybackMoveForward,     /* 0x06 - playback move forward */
        sysPlaybackMoveBackward,    /* 0x07 - playback move backward */
        sysPlaybackDelete,          /* 0x08 - playback delete */
        sysPlaybackDeleteAll,       /* 0x09 - playback delete all */
        sysPlaybackFormat,          /* 0x0a - playback format */
        sysPlaybackIsp,             /* 0x0b - playback ISP */ /*CY 1023*/
        sysVideoCaptureRoot,            /* 0x0c - capture video*/
        sysPowerOff,                /* 0x0d - Power Off */
        sysMacro,                   /* 0x0e - Set Macro */
        sysLcdRot,                  /* 0x0f - Set LCD Roation */
        sysSDCD_IN,                 /* 0x10 - Set SD Card Plug-in */
        sysUSBCheck,                /* 0x11 - Set USB */
        sysSDCD_OFF,                /* 0x12 - set SD Card Take-off*/
        sysWhiteLight,              /* 0x13 - Set Front White Light */
        sysFlashLight,              /* 0x14 - Set Flash Light */
        sysVOICE_REC,               /* 0x15 - VOICE Record*/
        sysUIReafFile,              /* 0x16 - Read File */
        sysPreviewZoomInOut,        /* 0x17 - preview zoom in/out*/
        sysTVPlaybackDelete,        /* 0x18 - playback delete */
        sysVideoZoomInOut,                           /* 0x19 - video zoom in/out*/
		sysPlaybackDeleteDir,		/* 0x1d - playback delete all files in current directory */
		sysUsbRemoved,				/* 0x1f - usb removed */
        sysSetUiKey,
        sysUpgradeFW,
#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)
        sysVideoCaptureStop,
        sysVideoCaptureRestart,        
#endif
        sysP2PReadFile,
        sysPlaybackCalendar,
        sysDrawWaitLoad,
   #if RX_SNAPSHOT_SUPPORT
        sysBack_RFI_RX_DataSave,
   #endif
        
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
        sysDrawTimeOnVideoClip,
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT)
        sysBack_RFI_RX_CH_Restart,
        sysBack_RFI_TX_CH_Del,
        sysBack_RFI_TX_CH_Create,
        sysBack_RFI_TX_Change_Reso,
        sysBack_RFI_TX_SnapShot,
      #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)  
        sysTX8211_EnterWifi,
        sysTX8211_LeaveWifi,
      #endif
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
        sysBack_Check_UI_500ms,
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)||(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
        sysBack_Check_TVinFormat,
#endif
        sysBack_Check_VideoinSource,
        sysBack_Set_Sensor_Color,
        sysBack_Draw_BitRate,
        sysBack_Draw_FrameRate,
        sysBack_Draw_OSDString,
        sysBack_Draw_SD_Icon,
#if(HOME_RF_SUPPORT)
        sysBack_Check_HOMERF,
#endif

#if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
        sysBack_Turn_SPK_GPIO,
#elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        sysBack_Turn_SPK_GPIO,
#elif ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
        sysBack_Draw_Sound_Bar,
#endif

   #if RX_SNAPSHOT_SUPPORT
        sysBack_RFI_RX_DataSave,
   #endif
#if (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
    #if BLE_SUPPORT
        sysBack_BLE_SyncTime,
    #endif        
#endif        
    };

s32 (*sysbackLowEvtFunc[])(s32,s32,s32,s32) =
    {
        _FS__fat_DeleteFATLink_Back,
        sysBackLow_UI_KEY_SDCD,
        sysBackLow_Syn_RF,
    #if RX_SNAPSHOT_SUPPORT
        uiRxSnapshot_All,
        uiRxSnapshot_One
    #endif
    };

s32 (*sysback_RF_EvtFunc[])(s32) =
    {
 #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT )
        sysBack_RFI_RX_CH_Restart,
        sysBack_RFI_TX_CH_Del,
        sysBack_RFI_TX_CH_Create,
        sysBack_RFI_TX_Change_Reso,
        sysBack_RFI_TX_SnapShot,
      #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)  
        sysTX8211_EnterWifi,
        sysTX8211_LeaveWifi,
      #endif        
        spiWriteRF,
        marsRfiu_FCC_DirectTXRX,
        rfiu_SetRXOpMode_1,
        rfiuSetGPO_TX,
        rfiuForceResync,
        uiClearFfQuadBuf,
        A7130_WOR_enable_B1,
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
        Save_UI_Setting,
        rfiuSetLightStat_RX
 #else
        sysTest
 #endif
    };

#if (NIC_SUPPORT == 1)
s32 (*sysback_Net_EvtFunc[])(u32, u32) =
{
   	P2PSendEvent,
	ntpdate,
	Upgrade_fw_net,
#if CLOUD_SUPPORT
	SendCloudMessage,
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
        for (i=0;i<5;i++);
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
	INT8U err;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    //-------------------------------------------------//
    //Global Variable Initialize//
    isuStatus_OnRunning = 0;
    sysCameraMode = SYS_CAMERA_MODE_UNKNOWN;

    sysForceWdt2Reboot=0;
    sysLifeTime=0;
    sysLifeTime_prev=0;
    sysDeadLockCheck_ena=1;
    sysPlayBeepFlag=0;
    sysTaskStopflag=0;

    dcfReadySemEvt    = OSSemCreate(1);

#if(SENSOR_FLICKER50_60_SEL == SENSOR_AE_FLICKER_60HZ)
    AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_60HZ;
#else
    AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_50HZ;
#endif

    /*------- SDRAM Performance enhancement -----------*/
    /*//SDRAM Arbit(ICM)grand window
      Ch1: IDU,ISU,
      CH2: Mpeg-0
      CH3: Mpeg-1
      CH4: AHB(CPU,APB)
    */
#if( (CHIP_OPTION == CHIP_A1016A) )
    SdramArbit = 0x01000000;
#elif((CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    SdramArbit = 0x00000000;
    SYS_DBGIF_SEL= 0xe00;  //Lucian: Turn on TV DAC(A,B,C)
#else
    SdramArbit = 0x01000000;  //Set SDRAM Arbit(ICM)grand window
#endif

    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD;
    sysProjectSysInit(1);

    //--------------SDRAM Arbit Piority ------------------------//
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4
    SdramTimeCtrl |= 0x80000000;    /* Peter 070130: For DDR Timming Contorl */

    //----------- Power Management----------//
#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )


    #if IDU_TV_DISABLE
    IduEna =0;
#if (CHIP_OPTION == CHIP_A1016A)
    SYS_ANA_TEST2=0;
#endif
    #endif

    sys_ctl0_status     = SYS_CTL0;

    sys_ctl0_status    &= ~SYS_CTL0_RF1012_CKEN &
                      #if USB2WIFI_SUPPORT
                      #else
                          ~SYS_CTL0_USB_CKEN &
                      #endif
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_CIU2_CKEN &
                      #if IDU_TV_DISABLE
                          ~SYS_CTL0_IDU_CKEN &
                      #endif
                      #if SD_CARD_DISABLE
                          ~SYS_CTL0_SD_CKEN &
                      #endif
                          ~SYS_CTL0_IR_CKEN &
                          ~SYS_CTL0_GPIU_CKEN;

    SYS_CTL0            = sys_ctl0_status;

 #elif( (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2)  )
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
                          ~SYS_CTL0_MD_CKEN &
                      #if SD_CARD_DISABLE
                          ~SYS_CTL0_SD_CKEN &
                      #endif    
                          ~SYS_CTL0_SER_MCKEN;

    SYS_CTL0            = sys_ctl0_status;
 #elif( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
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
                          ~SYS_CTL0_MD_CKEN &
                          ~SYS_CTL0_GPIU_CKEN &
                          ~SYS_CTL0_IR_CKEN &
                      #if SD_CARD_DISABLE
                          ~SYS_CTL0_SD_CKEN &
                      #endif    
                          ~SYS_CTL0_SER_MCKEN;

    SYS_CTL0            = sys_ctl0_status;   
 #else

 #endif

    /* zero initialize structure */
    /*Twokey 1124 E*/
    memset((void *)&sysEvt, 0, sizeof(SYS_EVT));

    /* Create the semaphore */
    sysSemEvt = OSSemCreate(0);

    gSysReadyFlagGrp = OSFlagCreate(0xffffffff, &err);

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
    #if ((DDR_SLEWRATE_CONTROL_FOR_CLOCK == 1)&&((CHIP_OPTION == CHIP_A1016A)))
    SYS_DDR_PADCTL2 |= 0x04;
    #endif
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    SYS_DDR_PADCTL2 |= 0x0F; //Slew Rate Toby 130730
    SYS_CTL0&=~SYS_CTL0_SER_MCKEN;
    #endif

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
         GpioActFlashSelect |=  GPIO_SPI2_FrDISP;
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

void sysTest(void)
{
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
            (*sysEvtFunc[cause])(param);
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
            //DEBUG_SYS("{");
            funerr=(*sysbackLowEvtFunc[cause])(param1,param2,param3,param4);
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
    s32 param1, param2;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    while (1)
    {
        OS_ENTER_CRITICAL();
        if (sysback_Net_GetEvt(&cause, &param1, &param2))
        {
            OS_EXIT_CRITICAL();
            (*sysback_Net_EvtFunc[cause])(param1, param2);
        }
        else
        {
            OS_EXIT_CRITICAL();
            OSSemPend(sysbackNetSemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SYS("Error: sysbackNetSemEvt is %d.\n", err);
                continue ;
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
    {   /* cause out of range */
        return 0;
    }

    if( ((sysEvt.idxSet+SYS_EVT_MAX-sysEvt.idxGet)%SYS_EVT_MAX) == (SYS_EVT_MAX-1))
    {   /* Overrun */
        DEBUG_SYS("SYS task Overrun %d!\n",cause);
        return 0;
    }

    /* set the cause */
    sysEvt.param[sysEvt.idxSet] = param;
    sysEvt.cause[sysEvt.idxSet++] = cause;

    if (sysEvt.idxSet == SYS_EVT_MAX)
    {   /* wrap around the index */
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
    {   /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysbackEvt.param[sysbackEvt.idxSet] = param;
    sysbackEvt.cause[sysbackEvt.idxSet++] = cause;

    if (sysbackEvt.idxSet == SYS_EVT_MAX)
    {   /* wrap around the index */
        sysbackEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (sysbackEvt.idxSet == sysbackEvt.idxGet)
    {
        /* event queue is full */
        DEBUG_SYS("SYSBACK QUE is Full:%d!\n",cause);
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
    {   /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysback_RF_Evt.param[sysback_RF_Evt.idxSet] = param;
    sysback_RF_Evt.cause[sysback_RF_Evt.idxSet++] = cause;

    if (sysback_RF_Evt.idxSet == SYSBACK_RFEVT_MAX)
    {   /* wrap around the index */
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
    {   /* cause out of range */
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
        {   /* wrap around the index */
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
    {   /* cause out of range */
        return 0;
    }

    /* set the cause */
    sysback_NET_Evt.param1[sysback_NET_Evt.idxSet] = param1;
    sysback_NET_Evt.param2[sysback_NET_Evt.idxSet] = param2;
    sysback_NET_Evt.cause[sysback_NET_Evt.idxSet++] = cause;

    if (sysback_NET_Evt.idxSet == SYSBACK_NET_EVT_MAX)
    {   /* wrap around the index */
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
    {   /* wrap around the index */
        sysEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYS_EVT_UNDEF)
    {   /* cause out of range */
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
    {   /* wrap around the index */
        sysbackEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_EVT_UNDEF)
    {   /* cause out of range */
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
    {   /* wrap around the index */
        sysback_RF_Evt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_RF_EVT_UNDEF)
    {   /* cause out of range */
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
    {   /* wrap around the index */
        sysback_NET_Evt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACK_NTE_EVT_UNDEF)
    {   /* cause out of range */
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
    {   /* wrap around the index */
        sysbackLowEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= SYSBACKLOW_EVT_UNDEF)
    {   /* cause out of range */
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

/*

Routine Description:

    Preview zoom.

Arguments:

    zoomfactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if CIU_TEST
s32 sysPreviewReset(s32 zoomFactor)
{

  #if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sysProjectPreviewReset(SYS_PREVIEW_RESET_PWRMAG);
  #endif

    //---DRAM Channel piority config---//

    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

   sysCheckZoomRun_flag=0;
   IsuOutAreaFlagOnTV = FULL_TV_OUT;
   siuOpMode = SIUMODE_PREVIEW;          //Lucian: 目前只做到 4CH 同一個 mode . 未來可各別設定.
   sysCameraMode = SYS_CAMERA_MODE_PREVIEW;

   iduPreview(640,480);

   ciu_1_Stop();


   if(sysTVinFormat == TV_IN_NTSC)
      ciuPreviewInit_CH1(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU1_OSD_EN,640*2);
   else
      ciuPreviewInit_CH1(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU1_OSD_EN,640*2);


   return 1;
}

#else
s32 sysPreviewReset(s32 zoomFactor)
{
    u16   scale;
    int i;

#if MULTI_CHANNEL_SUPPORT

#else
  #if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sysProjectPreviewReset(SYS_PREVIEW_RESET_PWRMAG);
  #endif

    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

    sysCheckZoomRun_flag=0;
    IsuOutAreaFlagOnTV = FULL_TV_OUT;
    sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    if (sysTVOutOnFlag)
    {
    #if SUB_TV_TEST
        subTV_Preview(640,480);
    #endif

        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
           iduSwitchPreview_TV(640,480);
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduSwitchPreview_TV(704,480);
           else
              iduSwitchPreview_TV(704,576);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduSwitchPreview_TV(352,240);
           else
              iduSwitchPreview_TV(352,240);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduSwitchPreview_TV(720,480);
           else
              iduSwitchPreview_TV(720,576);
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
           iduSwitchPreview_TV(1280,720);
        }
        else
           iduSwitchPreview_TV(640,480);
		//Disable Video output, Only OSD out.
        sysProjectPreviewReset(SYS_PREVIEW_RESET_TV_VIDEOOFF);
    }
    else
    {
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
           iduPreview(640,480);
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(704,480);
           else
              iduPreview(704,576);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(352,240);
           else
              iduPreview(352,288);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(720,480);
           else
              iduPreview(720,576);
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
           iduPreview(1280,720);
        }
        else
           iduPreview(640,480);
    }
 #endif

    //---- stop preview process---//
    isuStop();
    ipuStop();
    siuStop();

    setSensorWinSize(zoomFactor, SIUMODE_PREVIEW);
    scale = getPreviewZoomScale(zoomFactor);
    DEBUG_SYS("zoomfactor = %d->%d/100\n", zoomFactor, scale);
    sysPreviewZoomFactor = zoomFactor;

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

#if AUDIO_BYPASS
    Init_IIS_ALC5621_bypass();
#endif

    return 1;
}
#endif

#if ((SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (HW_BOARD_OPTION == MR6730_AFN) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST))
s32 sysPreviewInit(s32 dummy){}
s32 sysPreviewZoomInOut(s32 zoomFactor){}
s32 sysTVInChannelChange_Preview(s32 zoomFactor){}
s32 sysVideoZoomInOut(s32 zoomFactor){}
s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor){}
s32 sysSnapshot_OnPreview(s32 ScalingFactor){}
s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor){}
s32 sysVideoCaptureRoot(s32 dummy){}
s32 sysCaptureVideo_Init(void){}
s32 sysSensorFlip(s32 dummy){}
s32 sysCaptureVideo(s32 ZoomFactor){}
u32 sysBack_Set_Sensor_Color(u32 dummy){}
s32 sysCiu_1_PreviewReset(s32 zoomFactor){}
s32 sysCiu_2_PreviewReset(s32 zoomFactor){}
s32 sysCiu_3_PreviewReset(s32 zoomFactor){}
s32 sysCiu_4_PreviewReset(s32 zoomFactor){}
u32 getTVinFormat(){return 0;}
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)||(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
u32 sysBack_Check_TVinFormat(u32 dummy){}
#endif
#if( (CHIP_OPTION  >CHIP_PA9002D) && MULTI_CHANNEL_SUPPORT)
s32 sysCaptureImage_One_OnPreview_CIU1(s32 ZoomFactor){}
s32 sysCaptureImage_One_OnPreview_CIU2(s32 ZoomFactor){}
#endif
#if( (CHIP_OPTION >= CHIP_A1018A) && MULTI_CHANNEL_SUPPORT)
s32 sysCaptureImage_One_OnPreview420_CIU1(s32 ZoomFactor){}
s32 sysCaptureImage_One_OnPreview420_CIU2(s32 ZoomFactor){}
#endif

#else

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

    u32 timeoutcnt=0;
#if MULTI_CHANNEL_SUPPORT
  #if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sysProjectPreviewReset(SYS_PREVIEW_RESET_PWRMAG);
  #endif



    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4
  #if CIU_SPLITER  /* Amon (140612)*/
    #if(CHIP_OPTION == CHIP_A1018A)
        GpioActFlashSelect &= ~(CHIP_IO_DV2_EN3 | CHIP_IO_DISP2_EN | CHIP_IO_SMPTE2_EN);
        SYS_CHIP_IO_CFG2 = (SYS_CHIP_IO_CFG2 |CHIP_IO2_CCIR4CH_EN) & ~(CHIP_IO2_RFI3_EN2);
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288) )
        {
            if(sysTVinFormat == TV_IN_NTSC)
                iduSwitchPreview_TV(352, 240);
            else
                iduSwitchPreview_TV(352, 288);
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
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
        #if DUAL_MODE_DISP_SUPPORT
            iduSwitchPreview_TV(1280 * 2, 720);
        #else
            iduSwitchPreview_TV(1280, 720);
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
         #if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
            iduPreview(800, 480);
         #else
            iduPreview(640, 480);
         #endif
        #endif
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)  )
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288)  )
        {
            if(sysTVinFormat == TV_IN_NTSC)
                iduPreview(352, 240);
            else
                iduPreview(352, 288);
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

  #if (MULTI_CHANNEL_SEL & 0x02)
    sysCiu_1_PreviewReset(0);
    while(ciu_idufrmcnt_ch1 < 1)
    {
        OSTimeDly(1);
        timeoutcnt++;
        if ( timeoutcnt>10)
        {
            DEBUG_ASF("\nError!! sysCiu_1_PreviewReset CIU/SIU Time Out!! Reboot!\n ");
			sysForceWDTtoReboot();
            break;
        }
    }
  #endif

  #if(MULTI_CHANNEL_SEL & 0x04)
    GpioActFlashSelect = (GpioActFlashSelect | GPIO_DV2_FrGPI_EN2) & (~GPIO_GPIU_FrDISP_EN) ;
    sysCiu_2_PreviewReset(0);
  #endif

  #if(MULTI_CHANNEL_SEL & 0x08)
    sysCiu_3_PreviewReset(0);
  #endif

  #if(MULTI_CHANNEL_SEL & 0x10)
    sysCiu_4_PreviewReset(0);
  #endif

#else
    sysProjectPreviewInit(SYS_PREVIEW_INIT_RESET);
#endif


    siuResumeTask();
#if NEW_IDU_BRI
    sysProjectPreviewInit(SYS_PREVIEW_INIT_OSDDRAWICON);
#else
    sysProjectPreviewInit(SYS_PREVIEW_INIT_OSDDRAWICON);
#endif
#if ( (TV_DECODER == TW2866) && (CIU_SPLITER) ) /* Amon 暫時放這 (140612)*/
    //DEBUG_WARERR("CIU_1_SPILTER_CTL = %x\n",CIU_1_SPILTER_CTL); 
    if( (CIU_1_SPILTER_CTL & 0xffff) == 0x3210 )
        CIU_1_SPILTER_CTL = 0x32100000;
    else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x2103 )
        CIU_1_SPILTER_CTL = 0x03210000;
    else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x1032 )
        CIU_1_SPILTER_CTL = 0x10320000;
    else if( (CIU_1_SPILTER_CTL & 0xffff) == 0x0321 )
        CIU_1_SPILTER_CTL = 0x21030000;
#endif

    return 1;
}

s32 sysPreviewZoomInOut(s32 zoomFactor)
{
   u16 scale;

   sysCheckZoomRun_flag=1;  //Lucian: 目前Zoom IN/OUT 正在進行.

   scale = getPreviewZoomScale(zoomFactor);
   DEBUG_SYS("PreviewZoomFactor = %d->%d/100\n", zoomFactor, scale);
#if MULTI_CHANNEL_SUPPORT
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
   }
#else
   isuScUpZoom(zoomFactor);
#endif

   sysCheckZoomRun_flag=0;  //Lucian: 目前Zoom IN/OUT 結束.

   return 1;
}

s32 sysTVInChannelChange_Preview(s32 zoomFactor)
{
    u16 scale;
    u8 status;
    int cnt=0;

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
    #if MULTI_CHANNEL_SUPPORT
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
       #if(CIU2_OPTION != Sensor_CCIR656)
            ciu3ScUpZoom(zoomFactor);
       #endif
            break;

          case 4:
       #if(CIU2_OPTION != Sensor_CCIR656)
            ciu4ScUpZoom(zoomFactor);
       #endif
            break;
       }
   #else
       isuScUpZoom(zoomFactor);
   #endif

    sysCheckZoomRun_flag=0;  //Lucian: 目前Zoom IN/OUT 結束.
    return 1;
}

s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor)
{
    u16 scale;
    u8 status;
    int cnt=0;

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

    //if(siuOpMode != SIUMODE_PREVIEW)
    if(sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
    {
        DEBUG_SYS("Only Valid on Preview Mode!\n");
        return;
    }

    if (Write_protet() && gInsertCard==1)
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
}

//Lucian: No room function. Only one shot.
s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u16 data;
	u32 FB_curr;
	u8 *srcImg;
    u8 FID;
    int count;

    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
	GPIO_INT_CFG a;
#if DINAMICALLY_POWER_MANAGEMENT
    u32     sys_ctl0_status;
#endif

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_SIU Start---\n");
    RTC_Get_Time(&curDateTime);
    exifSetDateTime(&curDateTime);
	//---Stop intr---//
	while( (isu_idufrmcnt % 3) !=0);
#if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *
#endif

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

    if ((diskInfo->avail_clusters <= (used_cluster+2))||(global_totalfile_count > (DCF_FILE_PER_DIR-20))) // protect write miss
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
    #if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
    {
    #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
    #else
        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
    #endif
    }
    #endif

    DEBUG_SYS("--sysCaptureImage_One_OnPreview_SIU End---\n");

	return 1;

}
#if( (CHIP_OPTION  >CHIP_PA9002D) && MULTI_CHANNEL_SUPPORT)
s32 sysCaptureImage_One_OnPreview_CIU1(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u16 data;
	u32 FB_curr;
	u8 *srcImg;
    u8 FID;
    int count;
    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU1 Start---\n");
	//---Stop intr---//
	while( (ciu_idufrmcnt_ch1 & 0x03) !=0);
#if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *
#endif

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
        #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || ( HW_BOARD_OPTION == MR6730_AFN) // 避免CIU Bob mode切換時 size會有變化
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,640,240,640,240,0,0,CIU1_OSD_EN,640*2);
		#else
            ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,640,288,640,288,0,0,CIU1_OSD_EN,640*2);
		#endif
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
   {
       if(sysTVinFormat == TV_IN_NTSC)
          ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,240,704,240,0,0,CIU1_OSD_EN,704*2);
       else
          ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,288,704,288,0,0,CIU1_OSD_EN,704*2);
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
   {
       if(sysTVinFormat == TV_IN_NTSC)
          ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,240,352,120,0,0,CIU1_OSD_EN,352*2);
       else
          ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,704,288,352,144,0,0,CIU1_OSD_EN,352*2);
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
      ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
      ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU1_OSD_EN,704);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
      ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU1_OSD_EN,352);
   else
      ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
 #endif

 #if (TV_DECODER == WT8861)
	if(sysTVinFormat == TV_IN_PAL)  // 若field有交換的話,第一張影像要輸出到第零張的位址再做壓縮
    {
    	while(ciu_idufrmcnt_ch1 < 3);
    } else {
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
#if (HW_BOARD_OPTION == MR9670_HECHI)
    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 80);
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(352, 240); //暫時用
        	    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
        	                          //PNBuf_sub1[0],
        	                          srcImg,
        	                          JPEG_OPMODE_FRAME,352,240);
        	    JpegImagePixelCount=352*240;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(352, 288); //暫時用
        	    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
        	                          //PNBuf_sub1[0],
        	                          srcImg,
        	                          JPEG_OPMODE_FRAME,352,288);
        	    JpegImagePixelCount=352*288;//GetJpegImagePixelCount();
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
        } else {        // JPEG bitstream fail, 用較低的品直再壓一次
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

    if ((diskInfo->avail_clusters <= (used_cluster + 2))||(global_totalfile_count > (DCF_FILE_PER_DIR - 20))) // protect write miss
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
#if IS_COMMAX_DOORPHONE
	//DateTimeStamp_disable();    // in MainFlow.c
	//OSTimeDly(20);    // 顯示拍的照片一秒鐘
    //LCD_turnOffBackLight(); // avoid the 420->422->420 switching noise
    sysCameraMode       = SYS_CAMERA_MODE_PREVIEW;
    ciu_1_OpMode        = SIUMODE_PREVIEW;
    ciu_idufrmcnt_ch1   = 0;
    //iduPreview(640, 480);
    sysCiu_1_PreviewReset(0);
	//while(ciu_idufrmcnt_ch1 < 5);
    //LCD_turnOnBackLight(); // avoid the 420->422->420 switching noise
#else
    sysCameraMode   = SYS_CAMERA_MODE_PREVIEW;
    ciu_1_OpMode    = SIUMODE_PREVIEW;
    sysCiu_1_PreviewReset(0);
  #if DUAL_MODE_DISP_SUPPORT
    sysCiu_2_PreviewReset(0);
  #endif
#endif

    #if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
    {
    #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
    #else
        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
    #endif
    }
    #endif

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
    u16 data;
	u32 FB_curr;
	u8 *srcImg;
    u8 FID;
    int count;
    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU2 Start---\n");
	//---Stop intr---//
	while( (ciu_idufrmcnt_ch2 & 0x03) !=0);
#if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
        tvTVE_INTC   = TV_INTC_ALL_DISA;    //TV interrupt control *
#endif

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
        #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || ( HW_BOARD_OPTION == MR6730_AFN) // 避免CIU Bob mode切換時 size會有變化
        	ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,640,240,640,240,0,0,CIU2_OSD_EN,640*2);
        #else
        	ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,640,288,640,288,0,0,CIU2_OSD_EN,640*2);
	    #endif
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
   {
       if(sysTVinFormat == TV_IN_NTSC)
          ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,240,704,240,0,0,CIU2_OSD_EN,704*2);
       else
          ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,288,704,288,0,0,CIU2_OSD_EN,704*2);
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
   {
       if(sysTVinFormat == TV_IN_NTSC)
          ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,240,352,120,0,0,CIU2_OSD_EN,352*2);
       else
          ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,704,288,352,144,0,0,CIU2_OSD_EN,352*2);
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
      ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
      ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
      ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU2_OSD_EN,352);
   else
      ciuPreviewInit_CH2(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
#endif



 #if (TV_DECODER == WT8861)
	if(sysTVinFormat == TV_IN_PAL)  // 若field有交換的話,第一張影像要輸出到第零張的位址再做壓縮
    {
    	while(ciu_idufrmcnt_ch2 < 3);
    } else {
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
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            exifSetImageResolution(352, 240); //暫時用
    	    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
        	                          //PNBuf_sub1[0],
        	                          srcImg,
    	                          JPEG_OPMODE_FRAME,352,240);
    	    JpegImagePixelCount=352*240;//GetJpegImagePixelCount();
        }
        else
        {
            exifSetImageResolution(352, 288); //暫時用
    	    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
        	                          //PNBuf_sub1[0],
        	                          srcImg,
    	                          JPEG_OPMODE_FRAME,352,288);
    	    JpegImagePixelCount = 352 * 288;//GetJpegImagePixelCount();
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
        } else {        // JPEG bitstream fail, 用較低的品直再壓一次
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

    if ((diskInfo->avail_clusters <= (used_cluster + 2))||(global_totalfile_count > (DCF_FILE_PER_DIR - 20))) // protect write miss
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

    #if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
    {
    #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
    #else
        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
    #endif
    }
    #endif

    DEBUG_SYS("--sysCaptureImage_One_OnPreview_CIU2 End---\n");

	return 1;

}

#endif

#if( (CHIP_OPTION >= CHIP_A1018A) && MULTI_CHANNEL_SUPPORT)

s32 sysCaptureImage_One_OnPreview420_CIU1(s32 ZoomFactor)
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
    u16 data;
	u32 FB_curr;
	u8  *srcImgY, *srcImgUV;
    u8  FID;
    u32 count;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
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
        OS_EXIT_CRITICAL();

    	if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    	{
    	    exifSetImageResolution(1280, 720); //暫時用
    	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
    	                             srcImgY,
    	                             srcImgUV,
    	                             JPEG_OPMODE_FRAME, 1280, 720, 1280);
    	    JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(352, 240); //暫時用
        	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
        	                             srcImgY,
    	                                 srcImgUV,
        	                             JPEG_OPMODE_FRAME, 352, 240, 352);
        	    JpegImagePixelCount = 352* 240;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(352, 288); //暫時用
        	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
        	                             srcImgY,
    	                                 srcImgUV,
        	                             JPEG_OPMODE_FRAME, 352, 288, 352);
        	    JpegImagePixelCount = 352 * 288;    //GetJpegImagePixelCount();
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
        } else {        // JPEG bitstream fail, 用較低的品直再壓一次
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

    if ((diskInfo->avail_clusters <= (used_cluster + 2))||(global_totalfile_count > (DCF_FILE_PER_DIR - 20))) // protect write miss
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

    #if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
    {
    #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC  = TV_INTC_BOTFDSTART_ENA;
    #else
        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
    #endif
    }
    #endif

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
    u16 data;
	u32 FB_curr;
	u8  *srcImgY, *srcImgUV;
    u8  FID;
    u32 count;
#if DINAMICALLY_POWER_MANAGEMENT
    u32 sys_ctl0_status;
#endif
    u32 i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    //========================================//
    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU2 Start---\n");

	while(ciu_idufrmcnt_ch1 == 0);

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
        OS_EXIT_CRITICAL();

    	if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    	{
    	    exifSetImageResolution(1280, 720); //暫時用
    	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
    	                             srcImgY,
    	                             srcImgUV,
    	                             JPEG_OPMODE_FRAME, 1280, 720, 1280);
    	    JpegImagePixelCount = 1280 * 720;   //GetJpegImagePixelCount();
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
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288) )
        {
            if(sysTVinFormat == TV_IN_NTSC)
            {
                exifSetImageResolution(352, 240); //暫時用
        	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
        	                             srcImgY,
    	                                 srcImgUV,
        	                             JPEG_OPMODE_FRAME, 352, 240, 352);
        	    JpegImagePixelCount = 352 * 240;//GetJpegImagePixelCount();
            }
            else
            {
                exifSetImageResolution(352, 288); //暫時用
        	    jpegCapturePreviewImg420((u8*)exifPrimaryImage.bitStream,
        	                             srcImgY,
    	                                 srcImgUV,
        	                             JPEG_OPMODE_FRAME, 352, 288, 352);
        	    JpegImagePixelCount = 352 * 288;    //GetJpegImagePixelCount();
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
        } else {        // JPEG bitstream fail, 用較低的品直再壓一次
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

    if ((diskInfo->avail_clusters <= (used_cluster + 2))||(global_totalfile_count > (DCF_FILE_PER_DIR - 20))) // protect write miss
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

    #if TV_DISP_BY_IDU
    if(sysTVOutOnFlag) //TV-out
    {
    #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC  = TV_INTC_BOTFDSTART_ENA;
    #else
        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
    #endif
    }
    #endif

    DEBUG_SYS("--sysCaptureImage_One_OnPreview420_CIU2 End---\n");

	return 1;

}

#endif
/*

Routine Description:

    sysVideoCaptureRoot.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysVideoCaptureRoot(s32 dummy)
{
    INT8S err;

#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif
    CaptureVideoRun = 1;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_CLR, &err);		/* set to start record */
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_REC, OS_FLAG_SET, &err);		/* set to record  */
    /*CY 0613 S*/

#if SD_CARD_DISABLE
#else
    if (Write_protet() && gInsertCard==1)
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
#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
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
}

s32 sysCaptureVideo_Init(void) /*BJ 0530 S*/
{

#if DINAMICALLY_POWER_MANAGEMENT
    /* Peter */
    u32     sys_ctl0_status;

    //--------Power Control-----//
    // disable unused module for reduce power consumption
  #if ( (Sensor_OPTION  == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_OV2643_YUV601) || (Sensor_OPTION  == Sensor_CCIR601) || (Sensor_OPTION  == Sensor_CCIR656) || (Sensor_OPTION  == Sensor_OV7725_YUV601) || (Sensor_OPTION  == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_PC1089_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601))
    sys_ctl0_status     = SYS_CTL0;

   #if SD_CARD_DISABLE

   #else
        sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
   #endif

    sys_ctl0_status    |= SYS_CTL0_SIU_CKEN |
                          SYS_CTL0_ISU_CKEN |
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
		               #if !IS_COMMAX_DOORPHONE
                          SYS_CTL0_SER_MCKEN |
                       #endif
                       #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                          SYS_CTL0_MPEG4_CKEN |
                       #elif(VIDEO_CODEC_OPTION == H264_CODEC)
                          SYS_CTL0_H264_CKEN |
                       #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
                          SYS_CTL0_JPEG_CKEN |   //Lsk : Q2 where to disable
                       #endif
                       #if USB2WIFI_SUPPORT 
                          SYS_CTL0_USB_CKEN   |
                       #endif
                          SYS_CTL0_IIS_CKEN;
    sys_ctl0_status    &=
                          #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                          ~SYS_CTL0_JPEG_CKEN &
                          #endif
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_HIU_CKEN &
                       #if USB2WIFI_SUPPORT
                       #else
                          ~SYS_CTL0_USB_CKEN &
                       #endif
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
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||
                        (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
		               #if !IS_COMMAX_DOORPHONE
                          SYS_CTL0_SER_MCKEN |
                       #endif
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



    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD;


#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A)) //Lucian: walkaround 9003 bug
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

#elif(CHIP_OPTION == CHIP_PA9002D)
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4
#else
    SdramTimeCtrl |= 0xc0000000;    /* Peter 070130: For DDR Timming Contorl */
#endif

    return 1;

}

s32 sysSensorFlip(s32 dummy)
{
    u8 level;
    u32 bitset = 0x00000001;

    gpioGetLevel(0, 0, &level);
    if (level)
    {//High
        Gpio0InIntRiseEdge &= ~bitset;
        Gpio0InIntFallEdge |= bitset;
        i2cWrite_SENSOR(0x0c,0x10);
        i2cWrite_SENSOR(0x32,0x0);

        //DEBUG_SYS("high\n\r");
    }
    else
    {//Low
        Gpio0InIntFallEdge &= ~bitset;
        Gpio0InIntRiseEdge |= bitset;
        i2cWrite_SENSOR(0x0c,0xD0);
        i2cWrite_SENSOR(0x32,0x40);
        //DEBUG_SYS("low\n\r");
    }
    return 1;
}
/*

Routine Description:

    Capture video.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*BJ 0530 S*/
s32 sysCaptureVideo(s32 ZoomFactor) /*BJ 0530 S*/
{
    u32 free_size, temp;
    FS_DISKFREE_T* diskInfo;
    u32 bytes_per_cluster;
    int i;
    u8 mdset;


	sysCaptureVideo_Init(); /*BJ 0530 S*/
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    tvTVE_INTC=TV_INTC_ALL_DISA;  //Lucian: disable IDU/TV interrupt. 避免畫面閃動.
#endif


#if AUDIO_IN_TO_OUT
    iisPreviewI2OEnd();
#endif

#if AUDIO_BYPASS
    Close_IIS_ALC5621();
#endif


    sysCheckZoomRun_flag=0;
    //------------Check Free Space---------------//
//#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
    if ((sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA) && gInsertCard)
    {
        DEBUG_SYS("-->Overwrite Mode, Allocat Disk free size\n ");
        DEBUG_SYS("avail_clusters       = %d\n", global_diskInfo.avail_clusters);
        DEBUG_SYS("bytes_per_sector     = %d\n", global_diskInfo.bytes_per_sector);
        DEBUG_SYS("sectors_per_cluster  = %d\n", global_diskInfo.sectors_per_cluster);
        DEBUG_SYS("total_clusters       = %d\n", global_diskInfo.total_clusters);
        diskInfo=&global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
        DEBUG_SYS("Free Space=%d (KBytes) \n",free_size);
        //Check filesystem capacity
    #if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR))
        while((free_size < DCF_OVERWRITE_THR_KBYTE)||(global_totalfile_count > (DCF_FILE_PER_DIR-20)))  //Lucian: 錄影前先清除出檔案空間, 檔案個數.
    #else
        while((free_size < DCF_OVERWRITE_THR_KBYTE))
    #endif
        {   // Find the oldest file pointer and delete it
            if(dcfOverWriteDel()==0)
            {
                DEBUG_SYS("Over Write delete fail!!\n");
                return 0;
            }
            else
            {
                sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_OVERWRITE_DELETE_PASS, 0);
                //DEBUG_SYS("Over Write delete Pass!!\n");
            }
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
            DEBUG_SYS("Free Space=%d (KBytes) \n",free_size);
        }

    }
//#endif
//-----------做系統升頻 48MHz --> 64MHz ---------------//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_RISE_FREQUENCY, 0);
//----------------------------------------------------//

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
    OSTimeDly(5);//avoid getting noise when enable codec
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#endif


    mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100);
    DEBUG_SYS("Trace: Capture video...\n");

    siuAeEnable = 0;

    // stop preview process
 #if MULTI_CHANNEL_SUPPORT
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
    }
  #endif    // #if MULTI_CHANNEL_VIDEO_REC
 #else
    isuStop();
    ipuStop();
    siuStop();
 #endif




#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
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

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    IIS_WM8974_reset();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
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

#if 0   // check free space for test, Peter
    temp    = global_diskInfo.avail_clusters;
    dcfDriveInfo(&global_diskInfo);
    if(global_diskInfo.avail_clusters != temp)
        DEBUG_ASF("----->Check Availabe size:Error %d,%d\n",global_diskInfo.avail_clusters,temp);
    else
        DEBUG_ASF("----->Check Availabe size:PASS %d,%d\n",global_diskInfo.avail_clusters,temp);
#endif  // check free space for test

	//----Do power-off, if need-----//
    sysProjectCaptureVideo(SYS_CAPTURE_VIDEO_POWER_OFF, 0);

    return 1;
}
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656)||(Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
u32 sysBack_Check_TVinFormat(u32 dummy)
{
    void* msg;
    u8 err;
	u8	ucTvinStat;
	u8	data;
	static u8	ucCntEn = 0;
    u8 status,cnt=0;
    u32 TV_format;

    if(sysVideoInSel == VIDEO_IN_SENSOR)
        return 0;

#if (HW_BOARD_OPTION == MR6730_AFN)
#if (USE_TVIN_CHG_DETECT)
//check TV-decoder every second
#else

	//if(UI_isVideoRecording())
	if( (UI_isVideoRecording()) || (UI_isPreviewMode()&& MACRO_UI_SET_STATUS_CHK((UI_SET_STATUS_BITMSK_CH1_RECON|UI_SET_STATUS_BITMSK_CH2_RECON))) )
	{//skip detection when video recording
		return 0;
	}
	
#endif 
#endif 

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

	#if (HW_BOARD_OPTION == MR6730_AFN) 
	#if(MULTI_CH_DEGRADE_1CH)	
	//only one channel supportted
	#else
		cnt=0;
		
		i2cRead_TVP5150(0xc0,&status,I2C_TVP5150_RD_SLAV_ADDR_2);	//check TVP5150 is locked to the video signal, if no video signal input, max check 100
		while(((status&0x80) != 0x80) && (cnt < 10))
		{
			cnt++;
			i2cRead_TVP5150(0xc0,&status,I2C_TVP5150_RD_SLAV_ADDR_2);
		}

		if(cnt < 10)
		{    
			//DEBUG_UI("sysback TI5150_2 lock the video signal, get correctly TV in format\n");
			sysTVInFormatLocked1 = TRUE;
		}
		else
		{
			sysTVInFormatLocked1 = FALSE;
   			//DEBUG_UI("sysback TI5150_2 can't lock the video signal, use default TV in format-NTSC\n");
		}
	#endif
	#endif


	
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
		#if (HW_BOARD_OPTION == MR6730_AFN)
			//DEBUG_UI("TW9900_1 locked.\n");
		#else
			DEBUG_UI("sysback TW9900_1 lock the video signal, get correctly TV in format\n");
		#endif 
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
		#if (HW_BOARD_OPTION == MR6730_AFN)
		   //DEBUG_UI("TW9900_1 unlocked.\n");
		#else	
		   DEBUG_UI("sysback TW9900_1 can't lock the video signal, use default TV in format-NTSC\n");
		#endif 
    }

//#if IS_COMMAX_DOORPHONE
#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR6730_AFN) 
#if(MULTI_CH_DEGRADE_1CH)	
	//only one channel supportted
#else

    cnt=0;
    i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_2_ADDR);  //check TW9900 is locked to the video signal, if no video signal input, max check 100
    //DEBUG_I2C("status=0x%x \n\n",status);
    while(((status&0x80) != 0x00) && (cnt < 10))
    {
        cnt++;
        i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_2_ADDR);
    }
    if(cnt < 10)
    {
    //    DEBUG_UI("sysback TW9900_2 lock the video signal, get correctly TV in format\n");
		#if (HW_BOARD_OPTION == MR6730_AFN) 
		//suppress the message
		/*
			#if (USE_UI_APP_TASK)
			DMSG_OUT("TW9900_2 locked.\n");
			#else
			DEBUG_UI("TW9900_2 locked.\n");
			#endif //#if (USE_UI_APP_TASK)	
		*/
		#else
		////DEBUG_UI("sysback TW9900_2 lock the video signal, get correctly TV in format\n");
		#endif  
        sysTVInFormatLocked1 = TRUE;
    }
    else
    {
        sysTVInFormatLocked1 = FALSE;
   //     DEBUG_UI("sysback TW9900_2 can't lock the video signal, use default TV in format-NTSC\n");
		#if (HW_BOARD_OPTION == MR6730_AFN) 
		//suppress the message
		/*
			#if (USE_UI_APP_TASK)/
			DMSG_OUT("TW9900_2 unlocked.\n");
			#else
			DEBUG_UI("TW9900_2 unlocked.\n");
			#endif //#if (USE_UI_APP_TASK)		
		*/
		#else
		////DEBUG_UI("sysback TW9900_2 can't lock the video signal, use default TV in format-NTSC\n");
		#endif     
    }
#endif	
#endif
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
		#if (HW_BOARD_OPTION==MR6730_AFN)
			//switch to corresponding format only in preview mode. otherwise,switching after exit the preview mode 
			#if (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
				if(UI_isPreviewMode())
				{
					#if (USE_TVDEC_RECFG)	        
					UI_Do_TvDecoder_Init();
					#endif //#if (USE_TVDEC_RECFG)					
					iduSwitchNTSCPAL(SYS_TV_OUT_PAL);		
				}
				else
			#endif	
					TvOutMode = SYS_TV_OUT_PAL;
									
		#else
									
			#if (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
				iduSwitchNTSCPAL(SYS_TV_OUT_PAL);		
			#else
				TvOutMode = SYS_TV_OUT_PAL;
			#endif
										
		#endif
				
        #if (MULTI_CHANNEL_SEL & 0x02)
            ciu1_ChangeInputSize(640,576/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x04)
            ciu2_ChangeInputSize(640,576/2);
        #endif
        }
        else
        {
		#if (HW_BOARD_OPTION==MR6730_AFN)
			//switch to corresponding format only in preview mode. otherwise,switching after exit the preview mode 
			#if (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
				if(UI_isPreviewMode())
				{
					#if (USE_TVDEC_RECFG)	       
					UI_Do_TvDecoder_Init();
					#endif //#if (USE_TVDEC_RECFG)					
					iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);		
				}
				else
			#endif	
					TvOutMode = SYS_TV_OUT_NTSC;
								
		#else
								
			#if (SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
				iduSwitchNTSCPAL(SYS_TV_OUT_PAL);		
			#else
				TvOutMode = SYS_TV_OUT_PAL;
			#endif
									
		#endif	
			
        #if (MULTI_CHANNEL_SEL & 0x02)
            ciu1_ChangeInputSize(640,480/2);
        #endif

        #if (MULTI_CHANNEL_SEL & 0x04)
            ciu2_ChangeInputSize(640,480/2);
        #endif
        }
        
	#if (HW_BOARD_OPTION==MR6730_AFN)
		DEBUG_SYS("-->TvOutMode=%d\n",TvOutMode);
		#if (USE_TVIN_CHG_DETECT)
		_APP_ENTER_CS_;
		TvFmtModeChg=1;//TV format mode changed
		_APP_EXIT_CS_;
		#endif 
		if(UI_isPreviewMode())
		{//redraw OSD icons after TV-Out format changed under preview mode
			uiOSDPreviewInit();
		}
	#endif	

		
    #if (HW_BOARD_OPTION == MR9670_WOAN)
        sysPreviewInit(0);
    #endif

    #if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
        uiCaptureVideoStop();
        uiCaptureVideo();
    #endif

        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_TVinFormat, OS_FLAG_SET, &err);
    }
    return 1;

}
#endif

u32 sysBack_Set_Sensor_Color(u32 dummy)
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

s32 sysCiu_1_PreviewReset(s32 zoomFactor)
{

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
  if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
  {
        u8 mode = SIUMODE_PREVIEW;

        #if 0//IS_COMMAX_DOORPHONE
        if (Menu_getSetItemLayout(MENU_SavingMethod) == LAY_SavingMethod_StillImage)
            mode = SIUMODE_CAP_RREVIEW;
        #endif
        if(sysTVinFormat == TV_IN_NTSC)
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(mode,640,240,320,120,320,120,CIU1_OSD_EN,640*2);
            else
            #if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
                ciuPreviewInit_CH1(mode,640,240,640,240,0,0,CIU1_OSD_EN,800*2);
            #else
                ciuPreviewInit_CH1(mode,640,240,640,240,0,0,CIU1_OSD_EN,640*2);
            #endif

            rfiuVideoInFrameRate=30;
        }
        else
        {
            if(sysPIPMain == PIP_MAIN_CH2)
                ciuPreviewInit_CH1(mode,640,288,320,144,320,144,CIU1_OSD_EN,640*2);
            else
            #if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
                ciuPreviewInit_CH1(mode,640,240,640,240,0,0,CIU1_OSD_EN,800*2);
            #elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)  // 避免CIU Bob mode切換時 size會有變化
                ciuPreviewInit_CH1(mode,640,240,640,240,0,0,CIU1_OSD_EN,640*2);
            #else
				ciuPreviewInit_CH1(mode,640,288,640,288,0,0,CIU1_OSD_EN,640*2);
			#endif
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
  else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288) )
  {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,240,352,120,0,0,CIU1_OSD_EN,352*2);
            rfiuVideoInFrameRate=30;
        }
        else
        {
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,704,288,352,144,0,0,CIU1_OSD_EN,352*2);
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
#elif (HW_BOARD_OPTION == MR8211_ZINWELL)
    rfiuVideoInFrameRate=siuSensorInit(SIUMODE_PREVIEW,0);
    setSensorWinSize(0, SIUMODE_PREVIEW);
    ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,320);
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
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,640);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU1_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
        if(sysPIPMain == PIP_MAIN_CH2)
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU1_OSD_EN,704);
        else
            ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU1_OSD_EN,704);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
    {      
        ciuPreviewInit_CH1(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU1_OSD_EN,352);
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
#if AUDIO_BYPASS
    Init_IIS_ALC5621_bypass();
#endif

    return 1;
}


s32 sysCiu_2_PreviewReset(s32 zoomFactor)
{

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
            #if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
                ciuPreviewInit_CH1(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU2_OSD_EN,800*2);
            #elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN) // 避免CIU Bob mode切換時 size會有變化
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,240,640,240,0,0,CIU2_OSD_EN,640*2);
			#else
                ciuPreviewInit_CH2(SIUMODE_PREVIEW,640,288,640,288,0,0,CIU2_OSD_EN,640*2);
            #endif
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
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
    {
        if(sysTVinFormat == TV_IN_NTSC)
        {
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,240,352,120,0,0,CIU2_OSD_EN,352*2);
            rfiuVideoInFrameRate    = 30;
        }
        else
        {
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,704,288,352,144,0,0,CIU2_OSD_EN,352*2);
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
//#elif (HW_BOARD_OPTION == MR8211_ZINWELL)
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
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,640);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
    {
        if(sysPIPMain == PIP_MAIN_CH1)
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,320,240,0,0,CIU2_OSD_EN,704);
        else
            ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
    {
        ciuPreviewInit_CH2(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU2_OSD_EN,352);
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
    #if AUDIO_BYPASS
    Init_IIS_ALC5621_bypass();
    #endif
    return 1;
}

s32 sysCiu_3_PreviewReset(s32 zoomFactor)
{
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
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
   {
       if(sysTVinFormat == TV_IN_NTSC)
       {
          ciuPreviewInit_CH3(SIUMODE_PREVIEW,704,240,352,120,0,0,CIU3_OSD_EN,352*2);
          rfiuVideoInFrameRate=30;
       }
       else
       {
          ciuPreviewInit_CH3(SIUMODE_PREVIEW,704,288,352,144,0,0,CIU3_OSD_EN,352*2);
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
      ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU3_OSD_EN,640);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
      ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU3_OSD_EN,704);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
      ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU3_OSD_EN,352);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
      ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,576,0,0,CIU3_OSD_EN,704);
   else
      ciuPreviewInit_CH3(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU3_OSD_EN,704);
#endif
    #if AUDIO_IN_TO_OUT
    /* Due to Dac power off, so must iisPreviewI2OBegin again*/
    iisPreviewI2OEnd();
    iisPreviewI2OBegin();
    #endif
    #if AUDIO_BYPASS
    Init_IIS_ALC5621_bypass();
    #endif
   return 1;
}

s32 sysCiu_4_PreviewReset(s32 zoomFactor)
{
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
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
   {
       if(sysTVinFormat == TV_IN_NTSC)
       {
          ciuPreviewInit_CH4(SIUMODE_PREVIEW,704,240,352,120,0,0,CIU4_OSD_EN,352*2);
          rfiuVideoInFrameRate=30;
       }
       else
       {
          ciuPreviewInit_CH4(SIUMODE_PREVIEW,704,288,352,144,0,0,CIU4_OSD_EN,352*2);
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
      ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU4_OSD_EN,640);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
      ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU4_OSD_EN,704);
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
      ciuPreviewInit_CH4(SIUMODE_PREVIEW,isuSrcImg.w,isuSrcImg.h,352,240,0,0,CIU4_OSD_EN,352);
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
    #if AUDIO_BYPASS
    Init_IIS_ALC5621_bypass();
    #endif
   return 1;
}
#if(TV_DECODER == BIT1605) //use bit1605 tv decoder
u32 getTVinFormat()
{
    u8 data,level;
    u32 TVformat;
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

    #if(HW_BOARD_OPTION == MR8120_TX_JESMAY)

        if(sysTVInFormatLocked == FALSE)
        {
            if(AV_IN_FORMAT == TV_IN_NTSC)
            {
                i2cWrite_TVP5150(0x28, 0x02,TV_CHECK_FORMAT_WR_ADDR);
                TVformat = TV_IN_NTSC;
            }
            else
            {
                TVformat=TV_IN_PAL;
                i2cWrite_TVP5150(0x28, 0x04,TV_CHECK_FORMAT_WR_ADDR);
            }
        }

        i2cRead_TVP5150(0x8c, &data,TV_CHECK_FORMAT_RD_ADDR);
        data = (data & 0x0E) >> 1;
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
                break;
        }


    #endif
 #else

    /* read the video standard status from TVP5150 auto-detected */
    i2cRead_TVP5150(0x88, &data,TV_CHECK_FORMAT_RD_ADDR);
	FieldRate = (data & 0x20) >> 5;
    i2cRead_TVP5150(0x8c, &data,TV_CHECK_FORMAT_RD_ADDR);

    #if(HW_BOARD_OPTION == MR8120_TX_JESMAY )
    //Roy: JESMSY 的板子Read value 有時是第一組Addr 有時是第二組Addr
    if(data == 0xff)
    {
        i2cRead_TVP5150(0x8c, &data,I2C_TVP5150_RD_SLAV_ADDR_2);
    }
    #endif
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

#endif

/*

Routine Description:

    Preview Stop.

Arguments:


Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysPreviewStop(void)
{
    u16   scale;
    int i;
#if DINAMICALLY_POWER_MANAGEMENT
    /* Peter */
    u32     sys_ctl0_status;

    sysProjectPreviewStop(1);
#endif



    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4


    sysCheckZoomRun_flag=0;
    IsuOutAreaFlagOnTV = FULL_TV_OUT;
    // stop preview process
    isuStop();  //Lucian: 080616
    ipuStop();
    siuStop();
    //while (SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
    if (sysTVOutOnFlag)
    {
        sysProjectPreviewStop(2);
    }
    else
    {
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
           iduPreview(640,480);
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(704,480);
           else
              iduPreview(704,576);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(352,240);
           else
              iduPreview(352,288);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
        {
           if(sysTVinFormat == TV_IN_NTSC)
              iduPreview(720,480);
           else
              iduPreview(720,576);
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
           iduPreview(1280,720);
        }
        else
           iduPreview(640,480);
    }

    return 1;
}

/*

Routine Description:

    Snapshot.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSnapshot(s32 dummy)
{
    int i;
#if( (Sensor_OPTION == Sensor_OV7725_VGA) || (Sensor_OPTION == Sensor_MI_5M))
    if (Write_protet() && (gInsertCard==1) )
    {
        osdDrawProtect(2);
        return 0;
    }
   #if(Sensor_OPTION == Sensor_MI_5M)
    if ((siuFlashReady == 0) && (siuFlashLightMode == SIU_FLASH_LIGHT_AUTOMATIC || siuFlashLightMode == SIU_FLASH_LIGHT_ALWAYS_ON))
    {
        osdDrawFlashLight(1); //閃光燈充電中, 無法拍照.
        OSTimeDly(5);
        osdDrawFlashLight(0);
        return 0;
    }
   #endif
    siuFlashReady = 0;

   #if(Sensor_OPTION  == Sensor_OV7725_VGA)

   #elif(Sensor_OPTION  == Sensor_MI_5M)
      #if(FACTORY_TOOL == TOOL_ON)
        if (!config_mode_enable[UI_MENU_SETIDX_FOCUS-UI_MENU_SETIDX_LAST-1])
      #endif
        {
            while (siuAeReadyToCapImage == 0)
            {
                if ( (AECurSet <= AECurSet_Min) || (AECurSet >= AEPrevCurSet_Max))
                    break;
            }
        }
   #endif

    siuAeReadyToCapImage = 0;

    siuAeEnable = 0;

    sysSelfTimer(); /*CY 0907*/
    /*CY 0613 S*/

    if (sysTVOutOnFlag)
        iduTVOSDDisable(2);
    else
    {
        iduOSDDisable(2);
        uiMenuEnable = 0;

    }
    system_busy_flag=1;
    //for(i=0;i<600;i++) //Lucian: For test
       sysCaptureImage(sysPreviewZoomFactor);  /*BJ 0530 S*/
    sysProjectSnapshot(1);
#endif
    return 1;
}

#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)

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
#if 1
    return MultiChannelSysCaptureVideoStopOneCh(VideoChannelID);
#else
    if(VideoChannelID < MULTI_CHANNEL_LOCAL_MAX)
        return sysCaptureVideoSubTaskDestroy(VideoChannelID);
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    else if(VideoChannelID < MULTI_CHANNEL_MAX)
    {
        //RfRxVideoPackerDisableOneCh(VideoChannelID - MULTI_CHANNEL_LOCAL_MAX);
        RfRxVideoPackerSubTaskDestroy(VideoChannelID - MULTI_CHANNEL_LOCAL_MAX);
        return 1;
    }
#endif
    else
    {
        DEBUG_SYS("Error: sysVideoCaptureStop(%d) isn't a valid channel\n", VideoChannelID);
        return 0;
    }
#endif
}


/*

Routine Description:

    sysVideoCaptureRestart, for video record task self restart capture video.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysVideoCaptureRestart(s32 VideoChannelID)
{
    if(MultiChannelGetCaptureVideoStatus(VideoChannelID))
    {
        MultiChannelSysCaptureVideoStopOneCh(VideoChannelID);
        MultiChannelSysCaptureVideoOneCh(VideoChannelID);
    }
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

}

s32 sysDeadLockMonitor_ON(void)
{
#if FWUPGRADE_PROTECT
	if(OnlineUpdateStatus != FW_DOWNLOADING) //20170420 Sean: when downloading, CAN NOT TURN ON WATCHDOG!
#endif	
   		sysDeadLockCheck_ena=1;
}

s32 sysDeadLockMonitor_OFF(void)
{
   sysDeadLockCheck_ena=0;
}

s32 sysDeadLockMonitor_Reset(void)
{
   sysLifeTime_prev=sysLifeTime;
}


s32 sysForceWDTtoReboot(void)
{
	u8 err;
	spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
	OSSemPend(i2cWDTProtect, OS_IPC_WAIT_FOREVER, &err);
	sysForceWdt2Reboot=1;
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
    static u8 power_off = 1;
    u32     cnt = 2;
    u8  buf;

    /*avoid warning message*/
    if (cnt || buf)
    {}

    DEBUG_SYS("Power off !\n");
    if (uiCheckVideoRec()>0)
    {   //Video Clip
        pwroff = 1;
        return 0;
    }
    sysProjectPowerOff(1);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

#if 1
    if (power_off==1 && gInsertNAND==1 )
    {
        power_off=0;
        if (smcWriteBackFAT()==0)
        {
            DEBUG_SYS("Write FAT cache Fail \n");
        }
        power_off=2;
    }
    else if (power_off==1 && gInsertCard==1 )
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
#else
    power_off=2;
#endif

#elif ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    power_off=2;
#endif

    if (power_off==2)
    {
        sysProjectPowerOff(2);
        sysProjectPowerOff(3);
    }
    return 1;
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
{
    u32 bitset = (0x00000001<<22);
    u8 level;
    u32 timeout=0xffff;
    while (timeout!=0)
    {
        timeout--;
    }
    timeout=0xffff;
    gpioGetLevel(0, 22, &level);
    sysProjectMacro(1);
    if (sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
        return 0;

    if (level)
    {//High

        Gpio0IntEna &= ~bitset;
        Gpio0InIntRiseEdge &= ~bitset;
        Gpio0InIntFallEdge &= ~bitset;
        Gpio0InIntFallEdge |= bitset;
        Gpio0IntEna |= bitset;

        DEBUG_SYS("Switch to Normal Lens !!\n\r");
    }
    else
    {//Low

        Gpio0IntEna &= ~bitset;
        Gpio0InIntFallEdge &= ~bitset;
        Gpio0InIntRiseEdge &= ~bitset;
        Gpio0InIntRiseEdge |= bitset;
        Gpio0IntEna |= bitset;

        DEBUG_SYS("Switch to MACRO Lens!!\n\r");
    }

    return 1;
}

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
{
#if 1
    u32 bitset = (0x00000001<<23);
    u8 level;
    gpioGetLevel(0, 23, &level);
    if (level)
    {//High
        IduEna    &=  ~0x00000008;
        IduEna  |=  0x00000008;
        IduEna &= (~0x00000002);
        OSTimeDly(1);
        IduMpuCmdCfg = 0x00ff0028;
        iduWaitCmdBusy();
        IduMpuCmd = 0x00000802;
        iduWaitCmdBusy();
        IduMpuCmd = 0x00001C08;
        iduWaitCmdBusy();
        IduMpuCmd = 0x0000300A;
        iduWaitCmdBusy();
        IduMpuCmd = 0x0000142B;
        IduDispCfg &= (~0x00700000);
        IduDispCfg |= 0x00500000;
        IduEna |= 0x00000002;

        Gpio0InIntRiseEdge &= ~bitset;
        Gpio0InIntFallEdge |= bitset;
        //DEBUG_SYS("high\n\r");
    }
    else
    {//Low
        IduEna  &=  ~0x00000008;
        IduEna  |=  0x00000008;
        IduEna &= (~0x0000002);
        OSTimeDly(1);
        IduMpuCmdCfg = 0x00ff0028;
        iduWaitCmdBusy();
        IduMpuCmd = 0x00000802;
        iduWaitCmdBusy();
        IduMpuCmd = 0x00001C08;
        iduWaitCmdBusy();
        IduMpuCmd = 0x0000300A;
        iduWaitCmdBusy();
        IduMpuCmd = 0x00001428;
        IduDispCfg    &=  ~0x00700000;
        IduDispCfg    |=  0x00600000;
        IduEna |= 0x00000002;

        Gpio0InIntFallEdge &= ~bitset;
        Gpio0InIntRiseEdge |= bitset;
        //DEBUG_SYS("low\n\r");
    }
#else
    ADCRectest();
#endif
    return 1;
}

s32 sysGetDiskFree(s32 dummy)
{
    FS_DISKFREE_T* diskInfo;

    diskInfo=&global_diskInfo;
    system_busy_flag=1;

    dcfDriveInfo(diskInfo);

//    if (usb_msc_mode != 1)    /*When USB Plug-In, the system_busy_flag always 1*/
        system_busy_flag=0;
    got_disk_info=1;


    return 1;
}


/*

Routine Description:

    sysSDCD_IN.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSDCD_IN(s32 dummy)
{
    u8  ledon=0, i,sem_err,delay_flag;
    INT8S err;
    void* msg;
	FS_DISKFREE_T   *diskInfo;
    u32             free_size;
    u32             bytes_per_cluster;
	u8              temp;
    u8 level;
    int status;
    /*avoid warning message*/
    if (ledon || i || sem_err || delay_flag || msg)
    {}
	OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY|FLAGSYS_RDYSTAT_CARD_ERROR, OS_FLAG_CLR, &err);

	/* flag to announce to usb is doing SD init */
	OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);

	/* flag to pend to wait for usb check by SD functions */
	OSFlagPend(gSdUsbProcFlagGrp, FLAGSD_CHECK_SD, OS_FLAG_WAIT_SET_ANY, TIMEOUT_SDC, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SYS("Error SYS: gSdUsbProcFlagGrp is %d\n", err);
	}

    system_busy_flag=1;
    sysProjectSDCD_IN(1);
    osdDrawSDCD(1);
    //Close FIQ
#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FCINTE_disa();
    isu_FCINTE_disa();
#endif

    gSdcerr=1;
    gInsertNAND=0;
    File_protect_on=1;
    got_disk_info=0;

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    if (make_BitMap_ok==1 && UI_update==0xAC)
    {
        make_BitMap_ok=0;
    }
#elif ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    //Do nothing
#endif

	sdcUnInit();	/* del all related semaphore about SD control */

    level= sysCheckSDCD();
    if(level==SDC_CD_IN)
    {
        dcfFileTypeCount_Clean();
        sysFSType = 0;
        global_totalfile_count = 0;
        OSSemSet(dcfReadySemEvt, 1, &err);

        //------Lucian: SD 卡插入後燈號處理-------//
        DEBUG_SYS("do sysSDCD_IN.\n");
        sysProjectSDCD_IN(2);
        //---------File system initialize-----------//
        sysStorageOnlineStat[0] = 0;  // Add this will Let FAT to re-read the BPB section
        dcfUninit();
        err=dcfInit(STORAGE_MEMORY_SD_MMC);
        DEBUG_SYS("err=%d\n",err);
        if(err == -2)
        {
             DEBUG_SYS("Error! No SD Card\n");
             sysSDCD_OFF(1);
             return -2;
        }
        //-------- 檔案系統錯誤處理---------//
        if (err <= 0)
        {
            DEBUG_SYS("Error: FS Operation Error,0x%x, err=%d\n",fat_bpb_err, err);
            gSystemStroageReady = -1;
            if(sysProjectSDCD_IN(3) == 1)
                return 0;
            sysProjectSDCD_IN(4);

            if ((fat_bpb_err==BPB_SETTING_ERROR) || (fat_bpb_err==FAT_SETTING_ERROR)||(err == -1))
            {
                DEBUG_SYS("Error: FS Operation Error, 1\n");
                sysProjectSDCD_IN(5);
                OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_SET, &err);		/* XOR Format procedure */
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_ERROR, OS_FLAG_SET, &err);
                system_busy_flag=0;
            #if 0//((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
            #else
                if (Main_Init_Ready==0)
                    Main_Init_Ready=1;  // To release key lock
            #endif
             #if(CHIP_OPTION == CHIP_PA9002D)
                if (sysTVOutOnFlag)
                {
                    isu_FCINTE_ena();
                }
             #endif

                status = sdcMount();
                if ( status == -2)
                {
                    DEBUG_SYS("Error: No SD Card. #1\n");
                    //osdDrawSDIcon(UI_OSD_CLEAR);
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                    gInsertCard=0;
                    return -2;
                }
                else if(status == -1)
                {
                    DEBUG_SYS("Error: SD identify Err!\n");
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                    gInsertCard=0;
                    return -2;
                }

                if(1 == dcfCheckUnit() )
                {
                    DEBUG_SYS("Warning: File system is Correct! Retry again!\n");
                    uiOsdDrawSDCardFail(UI_OSD_DRAW);
                    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                    gInsertCard=0;
                    return -2;
                }
                else
                {
                	sysFSType = -1;
                    //gInsertCard=0;
                    //global_diskInfo.avail_clusters=0;
                    //global_diskInfo.bytes_per_sector=512;
                    //global_diskInfo.sectors_per_cluster=16;
                    //global_diskInfo.total_clusters=0;
                    DEBUG_SYS("Error! File system is illegle! Enter Format!\n");
                }
                
            #if (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
            	DEBUG_SYS("Warming! File system is't support Format! Exit!\n");
            	for(i = 15; i < 22; i++)
            		sysProjectSDCD_IN(i);
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                return -2;
            #endif

            #if 0//((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
                DEBUG_SYS("Error: SD identify Err!\n");
                uiOsdDrawSDCardFail(UI_OSD_DRAW);
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                gInsertCard=1;
                sysDisplaySDCardFail = 1;
                //return -2;
            #else
			
                sysDeadLockMonitor_OFF();
                /*release SD Init flag for USB Plug-in*/
                OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
                speciall_MboxEvt->OSEventPtr=(void *)0;
                uiSetGoToFormat();
                msg=OSMboxPend(speciall_MboxEvt, OS_IPC_WAIT_FOREVER, &sem_err);
                OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_CLR, &err);		/* XOR Format procedure */
                sysDeadLockMonitor_ON();
                if (strcmp((const char*)msg, "PASS"))
                {	/* msg = USBIN or FORMAT FAIL */

                    if (strcmp((const char*)msg, "FORMAT FAIL"))
                    {   /* msg = USBIN */
                        /* if usb is plug-in when formatting SD card, gpioIntHandler would terminate the formatting procedure.
                            Otherwise, usb would not work until formatting procedure is finished. */
                        sys_format = 0;
                        DEBUG_SYS("TRACE SYS: USB-In! Terminate Formatting SD Card!\r\n");
                        sysProjectSDCD_IN(7);
                     #if(CHIP_OPTION == CHIP_PA9002D)
	                    siu_FCINTE_ena();
	                    isu_FCINTE_ena();
                     #endif
	                    system_busy_flag=0;
                        sysProjectSDCD_IN(8);
						OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
	                    return 0;
					}
					else
					{
                        sys_format = 0;
					    sysProjectSDCD_IN(9);
                    #if(CHIP_OPTION == CHIP_PA9002D)
	                    siu_FCINTE_ena();
	                    isu_FCINTE_ena();
                    #endif
					    sysProjectSDCD_IN(10);


				        	system_busy_flag=0;
					    sysProjectSDCD_IN(11);
						OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
	                    return 0;
					}
                }
                else
                {
                    sys_format = 1;
                    /*Continue to implement the SD init*/
                    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_SET, &err);
				    sysProjectSDCD_IN(12);
                }
                #endif
            }
            else
            {   //Lucian: 不明原因, 請客戶自行檢查.
            #if(CHIP_OPTION == CHIP_PA9002D)
                siu_FCINTE_ena();
                isu_FCINTE_ena();
            #endif
                sysProjectSDCD_IN(13);
		        	system_busy_flag=0;

                sysProjectSDCD_IN(14);
				OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
                OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
	        #if IS_COMMAX_DOORPHONE
                OSTimeDly(100);
                gInsertCard=0;
                if (dummy != 1)
                    sysProjectSDCD_IN(21);
                return -2;
            #else
                return 0;
            #endif
            }
        }
        //----檔案系統初始化成功處理-----//
        else //(err == 1)
        {
            sys_format = 1;
            sysProjectSDCD_IN(15);
        }

        //----//
    //    usbInitLuns();  // Mount MSC fnction driver


	#if MAKE_SPI_BIN
		/*
		 * make bin from serial flash to SD card
		 * This is used for mass burning data to serial flash in mass production
		 */
//	 	if (adcCheckHiddenModeIndex() == 1)
	 	{
	 	    sysDeadLockMonitor_OFF();
			DEBUG_SYS("Making BIN...\n");
			uiMenuOSDFrame(OSD_SizeX , 16*OSD_STRING_W , 16 , (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2) , OSD_Blk2 , 0);
                uiOSDASCIIStringByColor("Making BIN...", (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);

             //osdDrawWarningMessage("Making BIN...",2, TRUE, FALSE);


			sysMakeSpiBin();

			DEBUG_SYS("Finished!\n");
			uiMenuOSDFrame(OSD_SizeX , 16*OSD_STRING_W , 16 , (OSD_SizeX-16*OSD_STRING_W)/2 , 112+(osdYShift/2) , OSD_Blk2 , 0);
            uiOSDASCIIStringByColor("OK! Please Re-boot!", (OSD_SizeX-20*OSD_STRING_W)/2 , 112+(osdYShift/2), OSD_Blk2 , 0xC0, 0x41);

//	        osdDrawWarningMessage("OK! Please Re-boot!",2, TRUE, FALSE);
            sysProjectSDCD_IN(16);
			while(1);
	 	}

	#endif

        //--------------------------In system programming --------------------------//
        sysProjectSDCD_IN(17);

        //-----------------Check Disk Free Size------------------//
        sysProjectSDCD_IN(18);

        //-------Lucian: Copy APP from Nand to SD for Special purpose------//
        sysProjectSDCD_IN(19);
        File_protect_on=0;
    }
    uiClearOSDBuf(2);

   //---Hint to User to wait---//
   sysProjectSDCD_IN(20);

  #if(FACTORY_TOOL == TOOL_ON)
    if (configsalix)
    {
        if (uiGetMenuMode() != CONFIG_MODE)
        {
            if ((config_mode_enable[UI_MENU_SETIDX_AWB_COLORCHECK-UI_MENU_SETIDX_LAST-1] == 1))
            {
                uiClearOSDBuf(2);
                osdDrawAWBColorCheckOSDFrame();
            }
            else
                uiClearOSDBuf(2);
        }
        else
        {
            rightflag = 1;
            uiSetConfig();
        }
    }
  #endif

    //----open FIQ interrupt---//
#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FCINTE_ena();
    isu_FCINTE_ena();
#endif

    /*-----------SDCD_IN complete and busy LED off,Flag and Semephore setup-------------*/

    	system_busy_flag=0;

	OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
    OSFlagPost(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_CLR, &err);
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    if (dummy != 1)
        sysProjectSDCD_IN(21);
    #else
    	sysProjectSDCD_IN(21);
    #endif


	DEBUG_SYS("TRACE SYS: SDC_USB_MEDIA_READY\n");
	sdcChangeMediaStat(SDC_USB_MEDIA_READY);
	// To remind system's storage is ready to record films
	if(sysCheckSDCD() == SDC_CD_IN)
		gSystemStroageReady = 1;
	SD_detect_status=1;
	//DEBUG_UI("--->sysSDCD_IN SD_detect_status =%d\n",SD_detect_status);
    return 1;
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

    sysSDCD_OFF.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysSDCD_OFF(s32 dummy)
{
    //civic 070917 S
    u8 err;
    u8 CHstring[6];

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_CLR, &err);
    //Close FIQ
    system_busy_flag=1;
    sysProjectSDCD_OFF(1);
    sysProjectSDCD_OFF(2);

#if(CHIP_OPTION == CHIP_PA9002D)
    siu_FCINTE_disa();
    isu_FCINTE_disa();
#endif
    gInsertCard = 0;
#if (NIC_SUPPORT == 1)
	Fileplaying = 0; //20160930 Sean
#endif
	sdcUnInit();	/* del all related semaphore about SD control */
    File_protect_on=1;
    gSdcerr=1;      // let err message just run one times

    // Check the DMA semaphore
    if (gInsertNAND!=1)
    {
        //JJ Added(Because SPI Flash without file system so far)
#if ((FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT)||(FLASH_OPTION == FLASH_SERIAL_WINBOND))
        DEBUG_SYS("Error: Unknown storage memory\n");
        got_disk_info=0;

            sysProjectSDCD_OFF(3);

    #if(CHIP_OPTION == CHIP_PA9002D)
        siu_FCINTE_ena();
        isu_FCINTE_ena();
    #endif


        	system_busy_flag=0;

        sysProjectSDCD_OFF(4);

        File_protect_on=0;
        DEBUG_SYS("do sysSDCD_OFF.1\n");
        dcfFileTypeCount_Clean();
        global_totalfile_count = 0;
        dcfUninit();


		SD_detect_status=1;
		//DEBUG_UI("--->sysSDCD_OFF SD_detect_status =%d\n",SD_detect_status);
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    	if (dummy != 1)
            UI_gotoStandbyMode();
	#endif
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
        return 0;
#else

        dcfFileTypeCount_Clean();
        global_totalfile_count = 0;

        sysStorageOnlineStat[0] = 0;  // Add this will Let FAT to re-read the BPB section
        dcfUninit();
        gInsertNAND=1;
        got_disk_info=0;

        if (dcfInit(STORAGE_MEMORY_SMC_NAND) <= 0)
        {
            DEBUG_SYS("Error: Unknown storage memory\n");
	            osdDrawWarningMessage("NAND Flash Error!",2, TRUE, FALSE);

		#if(CHIP_OPTION == CHIP_PA9002D)
            siu_FCINTE_ena();
            isu_FCINTE_ena();
        #endif

	        	system_busy_flag=0;

            sysProjectSDCD_OFF(5);
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
            return 0;
        }

 //       usbInitLuns();  // Mount MSC fnction driver

        //sysbackSetEvt(SYS_BACK_GET_DISK_FREE, 0);
        sysGetDiskFree(0);
#endif
    }
    File_protect_on=0;

	/* no draw osd if mass storage is working */

	    osdDrawSDCD(0);
	    osdDrawPreviewIcon();

  #if(CHIP_OPTION == CHIP_PA9002D)
    siu_FCINTE_ena();
    isu_FCINTE_ena();
  #endif


    DEBUG_SYS("do sysSDCD_OFF.\n");

    MemoryFullFlag = FALSE;

	SD_detect_status=1;
	//DEBUG_SYS("--->sysSDCD_OFF SD_detect_status =%d\n",SD_detect_status);
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    if (dummy != 1)
    UI_gotoStandbyMode();
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)	
	//if (dummy != 1){
		UI_gotoPreviewMode();
		DEBUG_SYS("======== sysSDCD_OFF 2: preview");
	//}
#endif

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
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

    sysUSBCheck.

Arguments:

    dummy - Dummy parameter.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysUSBCheck(s32 dummy)
{
    gpioCheckLevel_USB();
    return 1;
}

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
                       #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
		       #if!IS_COMMAX_DOORPHONE
                          SYS_CTL0_SER_MCKEN |
                       #endif
                          SYS_CTL0_IIS_CKEN;

    sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN;
    SYS_CTL0            = sys_ctl0_status;
#endif
    DEBUG_SYS("Sys task -- Voice Rec\n");

    // stop preview process
    isuStop();
    ipuStop();
    siuStop();

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
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


#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
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

/*

Routine Description:

    Capture image.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysCaptureImage(s32 ZoomFactor) /*BJ 0530 S*/
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
#if ADDAPP3TOJPEG
    u32 App3VGAImageSize;
#endif
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster,i;
    u32 bytes_per_cluster;
    u16 data;
    u32 iduWinReg;
    u32 AHB_ARBCtrl_tempReg;
    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */

    u32 Begin_Time, End_Time;

#if DINAMICALLY_POWER_MANAGEMENT
    u32     sys_ctl0_status;
#endif

    //===============================//
    isuStop();
    ipuStop();
    siuStop();
    /*avoid warning message*/
    if (iduWinReg || i)
    {}
    sysCheckZoomRun_flag=0;
    sysCaptureImageStart=1;
    DEBUG_SYS("Capture image start...\n");
    Begin_Time=OS_tickcounter;

    RTC_Get_Time(&curDateTime);
    exifSetDateTime(&curDateTime);


    if (sysTVOutOnFlag)
    {
        IduVideo_ClearPKBuf(2);
        IduVidBuf0Addr= (u32)PKBuf2;
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
        iduWinReg=IduWinCtrl & 0x0f;
        IduWinCtrl &= ~0x0000000f;
    }
    else
    {
        //Lucian: Disalbe OSD and VDO, avoid SIU data overflow.
        iduWinReg=IduWinCtrl & 0x0f;
#if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
        IduDefBgColor   = 0x00000000;
#else
        IduDefBgColor   = 0x00808000;
#endif

        IduWinCtrl &= ~0x0000000f;
    }

    // get raw image data
    siuCapturePrimary(ZoomFactor);


    if (wav_on==1) //Lucian: 此時Bayer data is captured...提示音發出
    {
      #if PLAYBEEP_TEST
        sysbackSetEvt(SYS_BACK_PLAY_BEEP, 0);
      #endif
    }

    AHB_ARBCtrl_tempReg=AHB_ARBCtrl;

    AHB_ARBCtrl = SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT;


//-----------------------------壓thumbnail----------------------------------------
#if (GET_SIU_RAWDATA_PURE || GET_BAYERDATA_SIU)
        End_Time=OS_tickcounter;
        DEBUG_SYS("Capture ready..Time=%d ms.\n",(End_Time-Begin_Time)*TIMER0_INTERVAL);
        Begin_Time=End_Time;

     #if(Sensor_OPTION ==Sensor_MI_5M)
        RawWriteFile(2564*1924, siuRawBuf);
     #elif(Sensor_OPTION ==Sensor_OV7725_VGA)
        RawWriteFile(644*484, siuRawBuf);
     #endif
#endif

#if ADDAPP2TOJPEG
    //==Get App2 information==//
    exifApp2Data->APP2Marker=0xe2ff; //swap hi/lo byte
    data=sizeof(DEF_APPENDIXINFO)-2;
    exifApp2Data->APP2Size= (data<<8)|(data>>8); //swap hi/lo byte

    sysProjectCaptureImage(1);

  #if(Sensor_OPTION == Sensor_MI_5M)
    //Sensor info
    i2cRead_SENSOR(0x9, &data);
    exifApp2Data->sensorinfo.EL=data;

    i2cRead_SENSOR(0x35, &data);
    exifApp2Data->sensorinfo.DG= (data>>8) & 0x7f;

    if (data & 0x40)
    {
        data &= 0x3f;
        data *=2;
    }
    else
    {
        data &= 0x3f;
    }
    exifApp2Data->sensorinfo.AG=data;

    //SIU info
    exifApp2Data->siu_status.SIU_CTL1 =SiuSensCtrl;
    exifApp2Data->siu_status.OB_COMP =SiuOb;
    exifApp2Data->siu_DGT_GAIN1=SiuBGbGain;
    exifApp2Data->siu_DGT_GAIN2=SiuRGrGain;

    exifApp2Data->LightFeq=AE_Flicker_50_60_sel;

    //AE weight table//
    memcpy(exifApp2Data->ae_report.AEwtab,siu_dftAeWeight1,25);
    //Get AWB image
    AWB_main(
        siuRawBuf,
        exifApp2Data->AwbImg,
        SENSOR_VALID_SIZE_X,
        SENSOR_VALID_SIZE_Y,
        16
    );
   #endif

#endif
    //== For Thumbnail==//
    End_Time=OS_tickcounter;
    DEBUG_SYS("Capture complete..Get thumbnail start..Time=%d ms.\n",(End_Time-Begin_Time)*TIMER0_INTERVAL);
    Begin_Time=End_Time;
//RE_JPEG:

#if DINAMICALLY_POWER_MANAGEMENT
    // disable unused module for reduce power consumption
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_IPU_CKEN |
                       #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          SYS_CTL0_ISU_CKEN ;

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                       #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                       #else
                          ~SYS_CTL0_MPEG4_CKEN &
                       #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_JPEG_CKEN ;

    SYS_CTL0            = sys_ctl0_status;
#endif



    // For thumbnail 640x480 -> 240x160
    isuCapture640x480_B2F(ZoomFactor); //Lucian: Raw(Bayer) -->PKbuf,640x480(YUV422)
    ipuCapturePrimary();



#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_ISU_CKEN |
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          0x00000000;

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                        #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                        #else
                          ~SYS_CTL0_MPEG4_CKEN &
                        #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_IPU_CKEN;

    SYS_CTL0            = sys_ctl0_status;
#endif
    isuScalar2Thumbnail(ZoomFactor); //Lucian: PKbuf --> PKbuf0(Pannel size),PKbuf1(Thubnail 160x120)


#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                       #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                       #else
                          ~SYS_CTL0_MPEG4_CKEN &
                       #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_ISU_CKEN &
                       #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          ~SYS_CTL0_SCUP_CKEN &
                       #endif
                          ~SYS_CTL0_IPU_CKEN;

    SYS_CTL0            = sys_ctl0_status;
#endif

    jpegCaptureThumbnail((u8*)exifThumbnailImage.bitStream, &thumbnailImageSize);

    End_Time=OS_tickcounter;
    DEBUG_SYS("Get Thumbnail end..Get Primary start..Time=%d ms.\n",(End_Time-Begin_Time)*TIMER0_INTERVAL);
    Begin_Time=End_Time;
    //----------------------APP3 data-------------
#if ADDAPP3TOJPEG
    if (sysInsertAPP3_ena)
    {
        jpegCaptureAPP3Image((u8*)exifAPP3VGAImage.bitStream,PKBuf0,JPEG_OPMODE_FRAME,JPGAPP3_WIDTH * 2);
        App3VGAImageSize=WaitJpegEncComplete();

        exifAPP3Prefix.APP3Marker=0xe3ff; //swap hi/lo byte
        data=sizeof(EXIF_PRIMARY)+App3VGAImageSize+2+4;
        exifAPP3Prefix.APP3Size= (data<<8)|(data>>8); //swap hi/lo byte
        exifAPP3Prefix.ID=0x12345678;
        Global_APP3VGAImageSize=App3VGAImageSize;
    }
#endif

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_ISU_CKEN |
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          0x00000000;
    SYS_CTL0            = sys_ctl0_status;
#endif
    if (!sysTVOutOnFlag)
    {
    #if((LCM_OPTION ==  LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
        isuScalar_D2D_SRGBout(
            PKBuf , PKBuf0 ,
            MSFSz.outw, MSFSz.outh,
            PANNEL_X, PANNEL_Y
        );
    #else
        isuScalar_D2D(
            PKBuf , PKBuf0 ,
            MSFSz.outw, MSFSz.outh,
            PANNEL_X, PANNEL_Y
        );
    #endif
    }
    sysProjectCaptureImage(2);


    //---------------------- 壓Primary Image-------------------------------------

#if (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_FRAME)   //Frame Mode

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_ISU_CKEN |
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          SYS_CTL0_IPU_CKEN;

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                        #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                        #else
                          ~SYS_CTL0_MPEG4_CKEN &
                        #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_JPEG_CKEN;


    SYS_CTL0            = sys_ctl0_status;
#endif
    isuCapturePrimary_B2F(ZoomFactor);   /*BJ 0530 S*/
    ipuCapturePrimary();

#if 0//GET_BAYERDATA_SIU
#if(Sensor_OPTION ==Sensor_MI_5M)
    RawWriteFile(2560*1920*2,PKBuf);
#elif(Sensor_OPTION ==Sensor_OV7725_VGA)
    RawWriteFile(640*480*2,PKBuf);
#endif
#endif

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                        #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                        #else
                          ~SYS_CTL0_MPEG4_CKEN &
                        #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_ISU_CKEN &
                       #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          ~SYS_CTL0_SCUP_CKEN &
                       #endif
                          ~SYS_CTL0_IPU_CKEN;

    SYS_CTL0            = sys_ctl0_status;
#endif

    jpegCapturePrimary((u8*)exifPrimaryImage.bitStream,JPEG_OPMODE_FRAME);
    primaryImageSize=WaitJpegEncComplete();
    JpegImagePixelCount = sysProjectCaptureImage(3);
    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);
#elif (JPEG_ENC_OPMODE_OPTION == JPEG_OPMODE_SLICE)  //Block Mode
    /*Lucian 070525 S*/
#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;
    sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN |
                          SYS_CTL0_ISU_CKEN |
                        #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                            (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                            (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          SYS_CTL0_IPU_CKEN;



    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                        #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                        #else
                          ~SYS_CTL0_MPEG4_CKEN &
                        #endif
                          ~SYS_CTL0_SER_MCKEN;

    SYS_CTL0            = sys_ctl0_status;
#endif

    jpegCapturePrimary((u8*)exifPrimaryImage.bitStream,JPEG_OPMODE_SLICE);
    isuCapturePrimary_B2B(ZoomFactor); /*BJ 0530 S*/
    ipuCapturePrimary();

    primaryImageSize=WaitJpegEncComplete();
    JpegImagePixelCount=GetJpegImagePixelCount();
    /* set related EXIF IFD ... */
    compressedBitsPerPixel = primaryImageSize * 8 * 10 / (JpegImagePixelCount); /* compressedBitsPerPixel = Compressed bits per pixel * 10 */
    exifSetCompressedBitsPerPixel(compressedBitsPerPixel);
    exifSetCopyRightVersion(VERNUM);
    /*Lucian 070525 E*/
#endif

    AHB_ARBCtrl=AHB_ARBCtrl_tempReg;

    End_Time=OS_tickcounter;
    DEBUG_SYS("Get primary complete..Write card start..Time=%d ms.\n",(End_Time-Begin_Time)*TIMER0_INTERVAL);
    Begin_Time=End_Time;

#if DINAMICALLY_POWER_MANAGEMENT
    sys_ctl0_status     = SYS_CTL0;

#if SD_CARD_DISABLE

#else
    sys_ctl0_status    |= SYS_CTL0_SD_CKEN;
#endif

    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                        #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &
                        #else
                          ~SYS_CTL0_MPEG4_CKEN &
                        #endif
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          ~SYS_CTL0_SCUP_CKEN &
                       #endif
                          ~SYS_CTL0_IPU_CKEN;

    SYS_CTL0            = sys_ctl0_status;
#endif

    //judge memory full or not civic 070926
    diskInfo    =   &global_diskInfo;
    bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    if (sysInsertAPP3_ena)
        used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO) + sizeof(DEF_APP3PREFIX) + sizeof(EXIF_PRIMARY) + Global_APP3VGAImageSize + bytes_per_cluster-1) / bytes_per_cluster;
    else
        used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO)+ bytes_per_cluster-1) / bytes_per_cluster;
    if (diskInfo->avail_clusters <= (used_cluster+2))
    {
        DEBUG_SYS("Memory full \n");
        sysPreviewReset(sysPreviewZoomFactor);/*BJ 0530 S*/
        #if (UI_PREVIEW_OSD == 1)
            uiOSDPreviewInit();
            osdDrawMemFull(UI_OSD_DRAW);
            OSTimeDly(20);
            osdDrawMemFull(UI_OSD_CLEAR);
        #endif
        sysCaptureImageStart=0;
        system_busy_flag=0;
        return 0;
    }

    //civic 070828 S for sys back task

    Global_primaryImageSize=primaryImageSize;
    Global_thumbnailImageSize=thumbnailImageSize;
    sysPreviewReset(sysPreviewZoomFactor);/*BJ 0530 S*/
    #if (UI_PREVIEW_OSD == 1)
        uiOSDPreviewInit();
    #endif
    sysbackSetEvt(SYS_BACK_EVT_W_SD, 0);


    //civic 070828 E
    sysCaptureImageStart=0;
    sysCaptureImageStart=0;
    End_Time=OS_tickcounter;
    DEBUG_SYS("Write card complete..Time=%d ms.\n",(End_Time-Begin_Time)*TIMER0_INTERVAL);
    //DEBUG_SYS("Trace: JPEG file captured - primary size = %d byte, thumbnail size = %d byte.\n", primaryImageSize, thumbnailImageSize);
    Begin_Time=End_Time;

    sysProjectCaptureImage(4);
    return 1;
}

#define VGA_RAW_OFFSET  3

s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor,u8 BurstNum,u8 ScalingFactor) /*BJ 0530 S*/
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster,i,j;
    u32 bytes_per_cluster;
    u16 data;
    u32 isu_intStat;
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
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          SYS_CTL0_SCUP_CKEN |
                       #endif
                          SYS_CTL0_GPIO1_CKEN  |
                          SYS_CTL0_TIMER4_CKEN |       //PWM enable
                       #if USB2WIFI_SUPPORT 
                          SYS_CTL0_USB_CKEN    |
                       #endif
                          SYS_CTL0_SER_MCKEN;          //sensor master clock output enable

    sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN  &    //JPEG disable
                       #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN &     //H264 disable
                       #else
                          ~SYS_CTL0_MPEG4_CKEN &    //MPEG disable
                       #endif
                          ~SYS_CTL0_HIU_CKEN &      //HIU disable.
                #if USE_BUILD_IN_RTC
                #else
                          ~SYS_CTL0_RTC_CKEN&
                #endif
                       #if USB2WIFI_SUPPORT
                       #else
                          ~SYS_CTL0_USB_CKEN &      //USB disable
                       #endif 
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

    for (i=0;i<BurstNum;i++)
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

    if (wav_on==1)
    {
      #if PLAYBEEP_TEST
        sysbackSetEvt(SYS_BACK_PLAY_BEEP, 0);
      #endif
    }

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

    for (i=0;i<9;i++)
        exifApp2Data->CCM[i]=d50_IPU_CCM[i];

    exifApp2Data->LightFeq=AE_Flicker_50_60_sel;
#endif

    siuOpMode = SIUMODE_CAPTURE;

    siuGetPreviewZoomWidthHeight(ZoomFactor,&srcW,&srcH);
    for (i=0;i<BurstNum;i++)
    {
#if DINAMICALLY_POWER_MANAGEMENT
        sys_ctl0_status     = SYS_CTL0;
        sys_ctl0_status    |= SYS_CTL0_ISU_CKEN |
                         #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                            (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                            (CHIP_OPTION == CHIP_A1026A))
                              SYS_CTL0_SCUP_CKEN |
                         #endif
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
                           #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                              ~SYS_CTL0_H264_CKEN &
                           #else
                              ~SYS_CTL0_MPEG4_CKEN &
                           #endif
                              ~SYS_CTL0_SER_MCKEN &
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
                           #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                              ~SYS_CTL0_H264_CKEN &
                           #else
                              ~SYS_CTL0_MPEG4_CKEN &
                           #endif
                              ~SYS_CTL0_SER_MCKEN &
                              ~SYS_CTL0_JPEG_CKEN &
                              ~SYS_CTL0_ISU_CKEN &
                           #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                            (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                            (CHIP_OPTION == CHIP_A1026A))
                              ~SYS_CTL0_SCUP_CKEN &
                           #endif
                              ~SYS_CTL0_IPU_CKEN;

        SYS_CTL0            = sys_ctl0_status;
#endif

        //judge memory full or not civic 070926
        diskInfo    =   &global_diskInfo;
        bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        used_cluster=(exifThumbnailImageHeaderSize+ thumbnailImageSize + exifPrimaryImageHeaderSize + primaryImageSize + sizeof(DEF_APPENDIXINFO)+ bytes_per_cluster-1) / bytes_per_cluster;

        if ((diskInfo->avail_clusters <= (used_cluster+2))||(global_totalfile_count > (DCF_FILE_PER_DIR-20))) // protect write miss
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


s32 sysCaptureImage_OnRFRx(int dummy) 
{
    u32 primaryImageSize;
    u32 thumbnailImageSize;
    u32 compressedBitsPerPixel;
    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster,i,j;
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



/*

Routine Description:

    Capture image series.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysCaptureImageSeries(s32 ZoomFactor) /*BJ 0530 S*/
{   //目前不支援連拍
    u8 i;

    DEBUG_SYS("Trace: Capture image series...\n");

    for (i = 0; i < 4; i++)
        sysCaptureImage(ZoomFactor); /*BJ 0530 S*/

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

u8 sysBackfromMenu(u32 iduBuffer)
{

    if (sysTVOutOnFlag)
    {
        if (global_totalfile_count==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)PKBuf0;
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            osdDrawFillEmpty();
            return 0;
        }
        IduVidBuf0Addr=(u32)iduBuffer;
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
        osdDrawPlaybackArea(sysTVOutOnFlag);
    }
    else
    {
        if (global_totalfile_count==0)
        {
            IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)iduvideobuff;
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
             osdDrawFillEmpty();
            return 0;
        }
        IduVidBuf0Addr=(u32)iduBuffer;
    #if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
    #endif
        osdDrawPlaybackArea(sysTVOutOnFlag);
    }

    osdDrawPlayIcon();

    if (sysPlaybackVideoPause==1)
        sysShowTimeOnOSD_VideoClip(0);
    return 1;
}

s32 sysPlaybackInit(s32 dummy)
{
    u8 err;
    system_busy_flag=1;
    asfIndexTableCount=0;
    playback_location=global_totalfile_count;
    sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
    iduPlaybackMode(640,480,640);

    if(!sysTVOutOnFlag) //Lsk 090810 : HW_BOARD have both pannel and tv-out
    {
    #if(LCM_OPTION == LCM_TG200Q04)
    idu_switch();
    #endif
    }
    siuSuspendTask();
    if (sysTVOutOnFlag)
    {
        //IduVideo_ClearBuf();

        if ((global_totalfile_count==0)||(gInsertCard==0))
        {
            //IduVideo_ClearBuf();
            IduVideo_ClearPKBuf(1);
            osdDrawFillEmpty();
            system_busy_flag=0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            return 0;
        }
        dcfPlaybackCurDir = dcfListDirEntTail;
        dcfPlaybackCurFile = dcfListFileEntTail;

#if 0
        if (dcfPlaybackDirBackward() == 0)
        {
            IduVideo_ClearPKBuf(1);
            osdDrawFillEmpty();
            playbackflag = 2;
            system_busy_flag=0;
            system_busy_flag=0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            return 0;
        }
#endif
        sysThumnailPtr = &sysThumnail[0];

        if (sysReadFile() == 0)
        {
            system_busy_flag=0;
            osdDrawPlayIcon();
            return 0;
        }

        osdDrawPlaybackArea(sysTVOutOnFlag);
        if (sysThumnailPtr->type == 0)
        {

        }
        else  if (sysThumnailPtr->type == 1)
        {

        }

    }
    else
    {	/* Panel out */
        IduVideo_ClearBuf();

        if ((global_totalfile_count==0)||(gInsertCard==0))
        {
            //IduVideo_ClearBuf();
            IduVidBuf0Addr = (u32)iduvideobuff;
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            osdDrawFillEmpty();
            system_busy_flag=0;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
            return 0;
        }
        dcfPlaybackCurDir = dcfListDirEntTail;
        dcfPlaybackCurFile = dcfListFileEntTail;

        sysThumnailPtr = &sysThumnail[0];
        osdDrawPlaybackArea(sysTVOutOnFlag);


        if (sysReadFile() == 0)
        {
            system_busy_flag=0;
            osdDrawPlayIcon();
            return 0;
        }
        if (sysThumnailPtr->type == 0)
        {

        }
        else  if (sysThumnailPtr->type == 1)
        {
        }
    }
    system_busy_flag=0;
    osdDrawPlayIcon();

#if Auto_Video_Test
    Video_Auto.VideoTest_FileNum = global_totalfile_count;
#endif
    return 1;
}


s32 sysPlaybackMoveForward(s32 dummy)
{
    u32 FileNum = 0;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = global_totalfile_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = global_totalfile_count;
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
}

s32 sysPlaybackMoveBackward(s32 dummy)
{
    u32 FileNum = 0;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = global_totalfile_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = global_totalfile_count;
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
    u32 FileNum = 0;
    INT8S err;

    //OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_DELETE, OS_FLAG_CLR, &err);

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
    if (dcfPlaybackDirMode == DCF_DOOR_DIR_MOVIE)
        FileNum = global_totalfile_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_PICTURE)
        FileNum = global_total_Pic_file_count;
    else if (dcfPlaybackDirMode == DCF_DOOR_DIR_ALBUM)
        FileNum = global_total_Alb_file_count;
#else
    FileNum = global_totalfile_count;
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

#if IS_COMMAX_DOORPHONE
#else
    if (playback_location == FileNum)
        playback_location=0;  // Moveforward will add one
    else
        playback_location--;   // Moveforward will add one
#endif
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

}



s32 sysPlaybackDeleteAll(s32 dummy)
{
    u8 i = 0;

    if (sysTVOutOnFlag)
    {
        if (global_totalfile_count==0)
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
        for (i=0;i<3;i++)
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
        if (global_totalfile_count==0)
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
    u8 i = 0;

    if (sysTVOutOnFlag)
    {
        if (global_totalfile_count==0)
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
        if (global_totalfile_count==0)
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
    INT8S err;


    DEBUG_SYS("Trace: Format drive...\n");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_CLR, &err);

    if (dcfPlaybackFormat() == 0)
    {
        OSMboxPost(general_MboxEvt, "FORMAT FAIL");
        return 0;
    }

    diskInfo=&global_diskInfo;
    dcfDriveInfo(diskInfo);
    got_disk_info=1;

    //-----卡滿偵測-----//
    if (MemoryFullFlag == TRUE)
    {
        MemoryFullFlag = FALSE;
		sysProjectDeviceStatus(DEV_SD_NOT_FULL);
    }

    //------------------//
    playback_location=0xFF;
    if (gInsertNAND==1)
        userClickFormat=1;
    pagecount=0;
    fileitem=0;

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
s32 sysUsbRemoved(void)
{

    DEBUG_SYS("TRACE SYS: USB Removed!!\n\r");

	/*===============Set USB Pull-High Resistor to Ground================*/
    sysUSBPlugInFlag=0;
 //   usbDevEnaCtrl(USB_R_PULL_LOW);/* disable usb pull-high resistor */
//    DEBUG_SYS("Disable Pull-high USB resistor, %X\n",Gpio1Level);


	/* release source occupied by USB */
//	usbUninst();


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
	u8 i, doPlaybackBackward;
	INT8U err;
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

s32 sysReadFile(void)
{
    char* pExtName = &dcfPlaybackCurFile->pDirEnt->d_name[9];
    char extName[3];
    u8 i, doPlaybackBackward;
    u32 uWidth, uHeight;
    u32 display_x,display_y;

#if IIS_DEBUG_ENA
    extern u32 under_count;
#endif

#if DINAMICALLY_POWER_MANAGEMENT  /* Peter */
    u32     sys_ctl0_status;
#endif
    INT8U err;

    AHB_ARBCtrl = SYS_ARBHIPIR_IDUOSD | SYS_ARBHIPIR_IDUVID;


#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A)) //Lucian: walkaround 9003 bug
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4

#elif(CHIP_OPTION == CHIP_PA9002D)
    SdramTimeCtrl &= (~0x00c00000);    //chanel piority: 1>2>3>4
#else
    SdramTimeCtrl |= 0xc0000000;    /* Peter 070130: For DDR Timming Contorl */
#endif


    /*BJ 0609 S*/
    if(!sysTVOutOnFlag)  //Lsk 090810 : HW_BOARD have both pannel and tv-out
    {
#if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04)) /*BJ 0721 S*/
    idu_switch();   /*CY 0907 TEST*/
#endif
    }

 #if MULTI_CHANNEL_SUPPORT
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
 #else
    isuStop();
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
#if(HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_SRAM_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          ~SYS_CTL0_SCUP_CKEN ;
                       #endif
                       #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN;
                       #else
                          //~SYS_CTL0_MPEG4_CKEN;
                       #endif
    SYS_CTL0            = sys_ctl0_status;
#else
    sys_ctl0_status    &= ~SYS_CTL0_SIU_CKEN &
                          ~SYS_CTL0_SER_MCKEN &
                          ~SYS_CTL0_IPU_CKEN &
                          ~SYS_CTL0_SRAM_CKEN &
                          ~SYS_CTL0_JPEG_CKEN &
                          ~SYS_CTL0_ISU_CKEN &
                       #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                        (CHIP_OPTION == CHIP_A1026A))
                          ~SYS_CTL0_SCUP_CKEN &
                       #endif
                       #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                          ~SYS_CTL0_H264_CKEN;
                       #else
                          ~SYS_CTL0_MPEG4_CKEN;
                       #endif
    SYS_CTL0            = sys_ctl0_status;
#endif                       
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

            uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk0, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk0, 0xC0, 0x00);

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
            for (i=0;i<3;i++)
                iduTVOSDEnable(i);
        }
        else
        {
            uiClearOSDBuf(1);
            //exifDecodeJPEGToYUV(PKBuf, Setup_Wave, 9000, &uWidth, &uHeight);
            iduSetVBuff(PKBuf, PKBuf, PKBuf);
        }

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
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
        IISPplyback=1;
        wavreadfile();
        iisStopPlay();
        iisSuspendTask();
        iisReset(IIS_SYSPLL_SEL_48M);
        IISPplyback=0;
        sysVoiceRecStart=0;

        timerInterruptEnable(1,0);

        //Civic 070829 E

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
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

                    iduOSDEnable(1);

                    uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                    osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

                    OSTimeDly(10);
                    iduOSDDisable(1);
            }
            else
                osdDrawPlayIcon();
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
            OSMboxPost(general_MboxEvt, "PASS");
            return file_err_flag;
        }


        if (sysTVOutOnFlag)
        {
            uiClearOSDBuf(1);

            *((volatile unsigned *)0xc0000000) = 0x00000022; //Lucian: 提升DMA piority
        }
        else
        {
            *((volatile unsigned *)0xc0000000) = 0x00000022; //Lucian: 提升DMA piority
            uiClearOSDBuf(1);
        }



        #if TV_DISP_BY_IDU
        if(sysTVOutOnFlag)
        {
        #if(TV_DISP_BY_TV_INTR)
            tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
        #else
            tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
        #endif
        }
        #endif
        sysVoiceRecStart=2;
        RTCseconds=0;
        timerInterruptEnable(1,1);


        #if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
        #elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
        #elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
        #elif(AUDIO_OPTION == AUDIO_ADC_DAC)
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
            iduOSDEnable(1);

            uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

            OSTimeDly(10);
            iduOSDDisable(1);
        }

        if (sysTVOutOnFlag)
        {
            *((volatile unsigned *)0xc0000000) = 0x00000020; //Lucian:恢復CPU piority
        }
        else
        {
            *((volatile unsigned *)0xc0000000) = 0x00000020; //Lucian:恢復CPU piority
        }

        #if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
        #elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
        #elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Close_IIS_ALC5621();
        #endif
        iduOSDEnable_All();

        sysVoiceRecStart=0;
        timerInterruptEnable(1,0);
        uiMenuSetAutoOff(1);//re-count timetick
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");

    }
#endif  // #if(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
    else if (strncmp(extName, "ASF", 3) == 0)  //Lsk 090410 : add forward,backward function
    {
#if DINAMICALLY_POWER_MANAGEMENT
    #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
    #else
        SYS_CTL0   |= SYS_CTL0_MPEG4_CKEN | SYS_CTL0_IIS_CKEN;
    #endif
#endif

#if IIS_DEBUG_ENA
        under_count=0;
#endif
    #if !(IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR8120_RX_HECHI))
        IduVideo_ClearPKBuf(0);
    #endif

#if ( (HW_BOARD_OPTION == MR6730_AFN) && (USE_UI_PLAYBACK_WDT) )
	if(setUI.PlybkWdt)
	{
		setUI.PlybkWdt=0;//reset
		DEBUG_SYS("PlybkWdt init.\n");
	}
#endif		
        // for Thumnail display
        sysThumnailPtr->type = 1;  // 1: ASF
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
            #if ASF_SPLIT_FILE
            file_err_flag =asfSplitFile(u32PacketStart);
            #else
            file_err_flag =asfReadFile(u32PacketStart);
            #endif
            sysPlaybackVideoStart   = 0;
            sysPlaybackVideoStop    = 1;
            sysPlaybackVideoPause   = 0;
            sysPlaybackForward      = 0;
            sysPlaybackThumbnail    = 0;

#if DINAMICALLY_POWER_MANAGEMENT
        #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
            SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
        #else
            SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
        #endif
#endif
			
			//DEBUG_SYS("file_err_flag=%d\n",file_err_flag);
            if (!file_err_flag)
            {
                iduOSDEnable(1);

                uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
                osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);
				#if 0//( (HW_BOARD_OPTION == MR6730_AFN) && (USE_PLAYBACK_AUTONEXT) )	
				if(setUI.SYS_PlayOrder==1)	
				{
					//osdDrawPlayIcon();
					
					DEBUG_SYS("==TRY NEXT==\n");
					#if (UI_APP_UTM)
					{
						u8 tmr_id=0;
						//DEBUG_UI("UTM:%d,%d\n",(3000/UTM_TIMEBASE_MS),g_LocalTimeInSec);
						tmr_id=UI_UserTimer_Add(UTM_CB_1,(1000/UTM_TIMEBASE_MS)+1,UTM_ONESHOT);
						if(tmr_id>UTM_NUM_MAX)
						{
							DEBUG_UI("UTM ADD ERR\n");//error
						}
						/*
						else
						{
							DEBUG_UI("UTM id=%d\n",tmr_id);//error
						}
						*/
					}
					#endif
					
				}
				#endif	
                OSTimeDly(10);
                iduOSDDisable(1);
            }
            else
                osdDrawPlayIcon();
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
            OSMboxPost(general_MboxEvt, "PASS");
            return file_err_flag;
        }
        if (sysTVOutOnFlag)
        {

            *((volatile unsigned *)0xc0000000) = 0x00000022; //Lucian: 提升DMA piority
        }
        else
        {
            *((volatile unsigned *)0xc0000000) = 0x00000022; //Lucian: 提升DMA piority
            uiClearOSDBuf(1);
        }


#if TV_DISP_BY_IDU
        if(sysTVOutOnFlag)
        {
        #if(TV_DISP_BY_TV_INTR)
            tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
        #else
            tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
        #endif
        }
#endif
        sysVoiceRecStart=2;
        RTCseconds=0;
        timerInterruptEnable(1,1);

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
        init_DAC_play(1);
#endif


		DEBUG_SYS("ASF file.\n");
        sysPlaybackVideoStart   = 1;
        sysPlaybackVideoStop    = 0;
        sysPlaybackVideoPause   = 0;
        sysPlaybackForward      = 0;
        sysPlaybackBackward     = -1;
        u32PacketStart          = 0;
        //doPlaybackBackward      = 0;
        asfIndexTableRead       = 0;
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_CLR, &err);
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_SET, &err);
        video_playback_speed = pre_playback_speed = 5; //Lsk 0324
		file_err_flag = asfReadFile(u32PacketStart);
        sysPlaybackVideoStart   = 0;
        sysPlaybackVideoStop    = 1;
        sysPlaybackVideoPause   = 0;
		//sysPlaybackForward      = 0;
       #if ( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION==A1018_FPGA_BOARD)|| \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
		while(videoPlayNext == 1)
		{
			OSTimeDly(1);//Lsk 090505
			if(sysPlaybackForward >=0 && dcfPlaybackCurFile != dcfListFileEntTail)
			{
				if(sysPlaybackForward ==0)
				{
					#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        			Init_IIS_WM8974_play();
					#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
			        Init_IIS_ALC5621_play();
					#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
			        ac97SetupALC203_play();
					#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
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
			else if(sysPlaybackBackward >=0 && dcfPlaybackCurFile != dcfListFileEntHead)
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
    #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
    #else
        SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
    #endif
#endif

        if (!file_err_flag)
        {
            iduOSDEnable(1);

            uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk1, 0x00 , alpha_3);
            osdDrawMessage(MSG_FILE_ERROR, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);
			#if ( (HW_BOARD_OPTION == MR6730_AFN) && (USE_PLAYBACK_AUTONEXT) )
			if(setUI.SYS_PlayOrder==1)	
			{
				//osdDrawPlayIcon();
				
				DEBUG_SYS("==TRY NEXT==\n");
				#if (UI_APP_UTM)
				{
					u8 tmr_id=0;
					//DEBUG_UI("UTM:%d,%d\n",(3000/UTM_TIMEBASE_MS),g_LocalTimeInSec);
					tmr_id=UI_UserTimer_Add(UTM_CB_1,(1000/UTM_TIMEBASE_MS)+1,UTM_ONESHOT);
					if(tmr_id>UTM_NUM_MAX)
					{
						DEBUG_UI("UTM ADD ERR\n");//error
					}
					/*
					else
					{
						DEBUG_UI("UTM id=%d\n",tmr_id);//error
					}
					*/
				}
				#endif
				
			}
			#endif	
            OSTimeDly(10);
            iduOSDDisable(1);
        }
        if (sysTVOutOnFlag)
        {
            *((volatile unsigned *)0xc0000000) = 0x00000020; //Lucian:恢復CPU piority
        }
        else
        {
            *((volatile unsigned *)0xc0000000) = 0x00000020; //Lucian:恢復CPU piority
        }

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
#endif
#if (HW_BOARD_OPTION == MR8200_RX_RDI_RX240)
        iduOSDEnable(0);
#else
        //iduOSDEnable_All();
#endif
#if TV_DISP_BY_IDU
        #if(TV_DISP_BY_TV_INTR)
        tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
        IduDispWinSel(0);
        #else
        tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
        timerPwmCountEnable(3, 0); //Lucian: disable timer-3.
        IduDispWinSel(0);
        #endif
#endif

        sysVoiceRecStart=0;
        timerInterruptEnable(1,0);
        uiMenuSetAutoOff(1);//re-count timetick
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_SET, &err);
        OSMboxPost(general_MboxEvt, "PASS");

    }
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
    else if (strncmp(extName, "MP4", 3) == 0)
    {
#if DINAMICALLY_POWER_MANAGEMENT
    #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CTL0   |= SYS_CTL0_H264_CKEN | SYS_CTL0_IIS_CKEN;
    #else
        SYS_CTL0   |= SYS_CTL0_MPEG4_CKEN | SYS_CTL0_IIS_CKEN;
    #endif
#endif
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
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
    #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
    #else
        SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
    #endif
#endif
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        IIS_WM8974_reset();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
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
    for (i=0;i<30;i++);
    SYS_RSTCTL |= (uiReset);
    for (i=0;i<30;i++);
    SYS_RSTCTL &= (~uiReset);
}

void SYSReset_EXT(u32 uiReset)
{
    INT32U  i;

    SYS_RSTCTL_EXT &= (~uiReset);
    for (i=0;i<30;i++);
    SYS_RSTCTL_EXT |= (uiReset);
    for (i=0;i<30;i++);
    SYS_RSTCTL_EXT &= (~uiReset);
}
void sysI2cRst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_I2C_RST);
    for (i=0;i<10;i++);
    SYS_RSTCTL |= SYS_RSTCTL_I2C_RST;
    for (i=0;i<10;i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_I2C_RST);

}
void sysJpegRst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_JPEG_RST);
    for (i=0;i<10;i++);
    SYS_RSTCTL |= SYS_RSTCTL_JPEG_RST;
    for (i=0;i<10;i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_JPEG_RST);

}
void sysSDCRst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_SD_RST);
    for (i=0;i<30;i++);
    SYS_RSTCTL |= SYS_RSTCTL_SD_RST;
    for (i=0;i<30;i++);
    SYS_RSTCTL &= (~SYS_RSTCTL_SD_RST);

}

void sysSIURst(void)
{
    s32 i;

    SYS_RSTCTL &= (~SYS_RSTCTL_SIU_RST);
    for (i=0;i<10;i++);
    SYS_RSTCTL |= SYS_RSTCTL_SIU_RST;
    for (i=0;i<10;i++);
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
#if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    SYS_CTL0   |= SYS_CTL0_H264_CKEN;
#else
    SYS_CTL0   |= SYS_CTL0_MPEG4_CKEN;
#endif
}

void sysMPEG_disable(void)
{
#if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    SYS_CTL0   &= ~SYS_CTL0_H264_CKEN;
#else
    SYS_CTL0   &= ~SYS_CTL0_MPEG4_CKEN;
#endif
}

void sysISU_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_ISU_CKEN |
               #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                (CHIP_OPTION == CHIP_A1026A))
                  SYS_CTL0_SCUP_CKEN |
               #endif
                  0x00000000;
}

void sysISU_disable(void)
{
    SYS_CTL0   &= ~SYS_CTL0_ISU_CKEN  &
                #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                    (CHIP_OPTION == CHIP_A1026A))
                  ~SYS_CTL0_SCUP_CKEN &
                #endif
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

void sysIIS_enable(void)
{
    SYS_CTL0   |= SYS_CTL0_IIS_CKEN;
#if(AUDIO_OPTION == AUDIO_ADC_DAC)

#else
    GpioActFlashSelect &= ~(GPIO_ACT_FLASH_SPI | GPIO_SPI2_FrDISP | GPIO_GPIU_FrDISP_EN);
    GpioActFlashSelect |= GPIO_IISFrDISP_EN;
#endif
}

void sysIIS_disable(void)
{
   //Lucian: 由於應用上 可能有其他Task 正在用. Always on.
}
//----------------------------------------------------//
void sysundefinedata(u8 index)
{
    u32 i;
    for (i=0;i<4800;i++)
        iduscalerbuff[index][i] = 0;
    //iduscalerbuff[index][i] = Love[i];
}

s32 sysUIReafFile(s32 dummy)
{
    if (sysReadFile() == 0)
        return 0;
    return 1;
}

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
    dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, dummy);
#endif
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PYBK_SEARCH, OS_FLAG_SET, &err);
    return 1;
}

s32 sysDrawWaitLoad(s32 dummy)
{
#if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    uiFlowWaitLoad(dummy);
#endif

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
    /* Roy: 因JESMAY 是A1016A 第一個案子，有SPI 與RF 共pin 問題 */
    #if((HW_BOARD_OPTION == MR8120_TX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_JESMAY))
    gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_CLK, GPIO_ENA);
    gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_MOSI, GPIO_ENA);
    #endif
}

void sysSPI_Enable(void)
{
    /* Roy: 因JESMAY 是A1016A 第一個案子，有SPI 與RF 共pin 問題 */
    #if((HW_BOARD_OPTION == MR8120_TX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_JESMAY))
    gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_CLK, GPIO_DISA);
    gpioSetEnable( GPIO_GROUP_SPI, GPIO_PIN_SPI_MOSI, GPIO_DISA);
    #endif
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
s32 sysBackLow_UI_KEY_SDCD(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3)
{

    u8 ui_doublechack_SD,sdcheck;
    u8 err;
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_STATUS, OS_FLAG_CLR, &err);
    do
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_CLR, &err);
        UI_SDLastLevel= sysCheckSDCD();
        #if !IS_COMMAX_DOORPHONE
            #if (HW_BOARD_OPTION != MR6730_AFN)
            uiFlowSdCardMode();
            #else
            if(UI_isMenuMode())
            {
            	extern void (*OSDDisplay[])(u8, u32, u32, u32, u32);

            	DispBuf_clear();

            	// fill black in OSD buffer
            	memset( ((uint8_t *)OSD_Blk2), 0xC1, (TVOSD_SizeX*TVOSD_SizeY) );
            	//memset( (void*)OsdBuf_Addr(), 0xC1, (TVOSD_SizeX*TVOSD_SizeY) );
            	(*OSDDisplay[sysTVOutOnFlag])(OSD_Blk2, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            	uiOsdEnable(OSD_Blk2);

            }		
            #endif
        #endif
            osdDrawMemFull(UI_OSD_CLEAR);
        #if IS_COMMAX_DOORPHONE
            UI_showSDCardCheckingMsgBox();
        #endif
        #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
            sdcheck= sysCheckSDCD();
            if(sdcheck==1)
            OSTimeDly(20);
        #endif
        uiCheckSDCD(0);
        ui_doublechack_SD = sysCheckSDCD();
        DEBUG_SYS(" ---->UI_SDLastLevel =%d ,ui_doublechack_SD =%d \n ",UI_SDLastLevel,ui_doublechack_SD);

    }while(ui_doublechack_SD!=UI_SDLastLevel);
    uiFlowCardReady(ui_doublechack_SD);
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_STATUS, OS_FLAG_SET, &err);
	#if (HW_BOARD_OPTION == MR6730_AFN) 	
		MACRO_UI_SET_SYSSTAT_BIT_CLR(UI_SET_SYSSTAT_BIT_SD_CHG);					
	#endif		
    return 1;
}

s32 sysBackLow_Syn_RF(s32 dummy,s32 dummy1,s32 dummy2,s32 dummy3)
{
    u8  i;

    //DEBUG_SYS("=====sysBackLow_Syn_RF=====\n");
#if ((UI_VERSION == UI_VERSION_ST_2)||(UI_VERSION == UI_VERSION_TRANWO)|| (UI_VERSION == UI_VERSION_COMMAX))
    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        uiSynRfConfig(i);
    }
#elif ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3) ||(UI_VERSION == UI_VERSION_RDI_4))
    for(i=0;i<MULTI_CHANNEL_MAX;i++)
        uiSynRfConfig(i, 1);
#endif
}

s32 sysShowTimeOnOSD_VideoClip(s32 dummy)
{
    osdDrawVideoTime();
    return 1;
}

s32 sysDrawTimeOnVideoClip(s32 Param)
{
    //printf("sysDrawTimeOnVideoClip()\n");
#if (HW_BOARD_OPTION != MR6730_AFN)    
    uiDrawTimeOnVideoClip(Param);
#else

	if(UI_isPreviewMode())
	{
		extern s8 OverwriteStringEnable;
	
		uiDrawTimeOnVideoClip(Param);	//draw timestamp on CIU_OSD only preview mode
	#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
	#if (IDUOSD_TIMESTAMP)
		if(OverwriteStringEnable)
		{
			if(gPreviewInitDone==1)
			{				
			UI_IduOsd_DrawTimeStamp(UI_OSD_DRAW);
			}
			else
			{
			UI_IduOsd_DrawTimeStamp(UI_OSD_CLEAR);
			}				
		}
	#endif
	#endif	
	}

#endif
    return  1;
}

#if((CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT)
s32 sysBack_RFI_RX_CH_Restart(s32 RFUnit)
{
    INT8U err;
    //----------------//
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==1)
    {
        DEBUG_SYS("======RX_CH_Restart Return:%d====\n",RFUnit);
        OSSemPost(gRfiuSWReadySemEvt);
        return 0;
    }
    DEBUG_SYS("======RX_CH_Restart Begin:%d====\n",RFUnit);
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
    if( (sysRFRxInMainCHsel==RFUnit) && (sysCameraMode != SYS_CAMERA_MODE_PLAYBACK) )
    {
       #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
          if(guiIISPlayDMAId != 0xFF)
          {
             marsDMAClose(guiIISPlayDMAId);
             guiIISPlayDMAId = 0xFF;
          }
       #endif

       #if RFIU_RX_AUDIO_RETURN
          if(guiIISRecDMAId != 0xFF)
          {
             marsDMAClose(guiIISRecDMAId);
             guiIISRecDMAId = 0xFF;
          }
       #endif
    }
    OSTimeDly(5);
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
    rfiuRxVideoBufMngWriteIdx[RFUnit]=0;
    rfiuRxIIsSounBufMngWriteIdx[RFUnit]=0;

    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
    #if( (SW_APPLICATION_OPTION != MR8100_BABYMONITOR) && (SW_APPLICATION_OPTION != MR8100_DUALMODE_VBM) )
       if(sysRFRxInMainCHsel==RFUnit)
          memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
    #endif
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
       //Lucian:應該只清sub-window, not implement now.
       //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*4);
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
    {

    }
    //===============//

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

    #elif (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
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

            if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
            {
                if(sysRFRxInMainCHsel == RFUnit)
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
            }
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            Record_flag[RFUnit] = 0;
          #endif
            gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_RUNNING;

    #endif
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;
    DEBUG_SYS("====RX_CH_Restart End:%d====\n\n",RFUnit);
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}

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

s32 sysBack_RFI_TX_Change_Reso(s32 Setting)
{
    int temp;
    int i;
    #if RF_TX_OPTIMIZE
    s32             TimeOffset;
    #endif

    #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    return 1;
    #endif
    temp=gRfiuUnitCntl[0].RFpara.MD_en;
    gRfiuUnitCntl[0].RFpara.MD_en=0;
  #if USB2WIFI_SUPPORT
    Change_RSE = 0;
  #endif
    
#if RF_TX_OPTIMIZE
    //--TX--//
    gRfiuUnitCntl[0].TX_Task_Stop=1;
    gRfiuUnitCntl[0].TX_Wrap_Stop=1;
    gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
    gRfiuUnitCntl[0].TX_IIS_Stop=1;

    OSTimeDly(6);

    for(i=0;i<100;i++)
    {
       if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
         break;
       OSTimeDly(1);
    }

  
    OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
    OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
    gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
    gRfiuUnitCntl[0].TX_Task_Stop=0;
    DEBUG_ASF("====RFIU1_TASK Delete====\n");
    OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
    OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
    gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
    gRfiuUnitCntl[0].TX_Wrap_Stop=0;
    DEBUG_ASF("====RFIU1_WRAP Delete====\n");

    /* suspend mpeg4 task */
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
	mpeg4SuspendTask();
    OSTaskDel(MPEG4_TASK_PRIORITY);
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
	VideoTaskSuspend();
    OSTaskDel(VIDEO_TASK_PRIORITY);
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
	mjpgSuspendTask();
    OSTaskDel(MJPG_TASK_PRIORITY);
    #endif
    iisStopRec();
    iisSuspendTask();
    
    OSTaskDel(IIS_TASK_PRIORITY);
  #if(HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I)
    if(sys8211TXWifiStat != MR8211_ENTER_WIFI)
    {
    #if RFIU_RX_AUDIO_RETURN
        if(guiIISPlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISPlayDMAId);
            guiIISPlayDMAId = 0xFF;
        }
    #endif
    }
  #else
      #if RFIU_RX_AUDIO_RETURN
        if(guiIISPlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISPlayDMAId);
            guiIISPlayDMAId = 0xFF;
        }
    #endif

  #endif
    gRfiuUnitCntl[0].TX_Task_Stop=0;
    gRfiuUnitCntl[0].TX_Wrap_Stop=0;
    gRfiuUnitCntl[0].TX_MpegEnc_Stop=0;
    gRfiuUnitCntl[0].TX_IIS_Stop=0;
    //----Change sensor resolution-----//
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
  
    }
    uiMenuSet_VideoSize(Setting);
    sysPreviewInit(0);
    #if(MULTI_CHANNEL_SEL & 0x02)
    ciu_1_OpMode = SIUMODE_MPEGAVI;
    //ciu_1_FrameTime = 0;
    #elif(MULTI_CHANNEL_SEL & 0x04)
    ciu_2_OpMode = SIUMODE_MPEGAVI;
    ciu_2_FrameTime = 0;
    #endif

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
    OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);
    OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY); 

    DEBUG_ASF("====RFIU1_TASK Create====\n");
    OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
    gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;
#else
    uiCaptureVideoStop();
    uiMenuSet_VideoSize(Setting);
    if( (Setting != UI_MENU_VIDEO_SIZE_320x240) && (Setting != UI_MENU_VIDEO_SIZE_352x240)  )  //Lucian: 本機不支援QVGA,CIF,不回存
        Save_UI_Setting();
    uiCaptureVideo();
#endif
    gRfiuUnitCntl[0].RFpara.MD_en=temp;

    return 1;
}

s32 sysBack_RFI_TX_SnapShot(s32 Setting)
{
    int temp;
    int i;
    s32             TimeOffset;
    //----------------------------------//  
  #if USB2WIFI_SUPPORT
    P2P_Snapshot = 1;
    P2P_Snapshot_cnt = 3;
  #endif    
 #if RF_TX_OPTIMIZE   
    temp=gRfiuUnitCntl[0].RFpara.MD_en;
    gRfiuUnitCntl[0].RFpara.MD_en=0;
    //--TX--//
    gRfiuUnitCntl[0].TX_Task_Stop=1;
    gRfiuUnitCntl[0].TX_Wrap_Stop=1;
    gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
    gRfiuUnitCntl[0].TX_IIS_Stop=1;

    OSTimeDly(6); 

    for(i=0;i<100;i++)
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
    iisStopRec();
    iisSuspendTask();
    OSTaskDel(IIS_TASK_PRIORITY);
    DEBUG_SYS("====IIS task Delete====\n");
    
    #if RFIU_RX_AUDIO_RETURN
        if(guiIISPlayDMAId != 0xFF)
        {
            marsDMAClose(guiIISPlayDMAId);
            guiIISPlayDMAId = 0xFF;
        }
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

        break;
    }
    //----Snap Shot main body-----//
#if TX_SNAPSHOT_SUPPORT
    if(sysRFTXSnapImgRdy ==0)
    {
        sysTxCaptureImage_CIU1(Setting);
    }
    else
    {
        DEBUG_SYS("DATA image is not transmitted yet\n");
    }
    sysRFTXSnapImgRdy=1;
    rfiuTXSnapCheck.SnapStatus=RF_TXPHOTOSTA_DONE_OK;
#endif
    //----------------------------//
    sysPreviewInit(0);
    
    #if(MULTI_CHANNEL_SEL & 0x02)
    ciu_1_OpMode = SIUMODE_MPEGAVI;
    ciu_1_FrameTime = 0;
    #elif(MULTI_CHANNEL_SEL & 0x04)
    ciu_2_OpMode = SIUMODE_MPEGAVI;
    ciu_2_FrameTime = 0;
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
    ciu1ZoomStart=1;
   
#endif
#if USB2WIFI_SUPPORT
	  P2P_Snapshot = 0;
#endif
    OSTimeDly(200);
    return 1;
}

#if TX_SNAPSHOT_SUPPORT
    s32 sysTxCaptureImage_CIU1(s32 Setting)
    {
        u8 *srcImgY,*srcImgUV;
        u32 sys_ctl0_status;
        int i,j;
        u32 JpegImagePixelCount;
        u32 primaryImageSize;
     #if ADDAPP3TOJPEG   
        u32 App3VGAImageSize;
        u16 data;
     #endif
        u32 compressedBitsPerPixel;
        int OldSetting;
        u8 *pp;
        u32 TotalSize;
        u32 app4_size;
        u32 data_size;
        
        u8 *yy,*cc;
        int Width,Height;
        //---Point to necessary memory--//
        DEBUG_SYS("==sysTxCaptureImage_CIU1 Start==\n");
        //YUV422 -->PKBuf0
        exifPrimaryBitstream=PNBuf_sub1[3];
        exifThumbnailBitstream=exifPrimaryBitstream+MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT/2;
     #if ADDAPP3TOJPEG
        exifAPP3VGABitstream=exifThumbnailBitstream+160*120/2;
     #endif   
        exifFileInit();
        //-------//
        sysCameraMode = SYS_CAMERA_MODE_PREVIEW;

        OldSetting=uiMenuVideoSizeSetting;
        uiMenuVideoSizeSetting=Setting;
//        rfiuVideoInFrameRate=siuSensorInit(SIUMODE_CAP_RREVIEW,0); //避免夜間拍照彩色
        setSensorWinSize(0, SIUMODE_CAP_RREVIEW);
        OSTimeDly(10); //wait AE ready.
     
        ciu_1_OpMode = SIUMODE_CAP_RREVIEW;
     
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
           ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,1280,720,0,0,CIU2_OSD_EN,1280);
           Width=1280;
           Height=720;
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
           ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
           Width=640;
           Height=480;
        }
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
        {
           ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
           Width=704;
           Height=480;
        }
        else
        {
           ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,640,480,0,0,CIU2_OSD_EN,640);
           Width=640;
           Height=480;
        }
        //--------------------------------//
        while(ciu_idufrmcnt_ch1<1);
        //ciu_1_Stop();
        CIU_1_CTL2 &= (~(0x00000040 | 0x00000020));
    	DEBUG_SYS("--->Snap %d x %d\n",Width,Height);
    
        srcImgY  = PNBuf_sub1[0];
        srcImgUV    = srcImgY + PNBUF_SIZE_Y;
        
        #if((CHIP_OPTION == CHIP_A1016A))//Lucian: walk around A1016A's bug.
        pp = srcImgY+ Width*2*(Width/2);
        memcpy(pp,pp+Width*2,Width*2);      
        #endif
        //-------------------------------------------------//        
        sys_ctl0_status     = SYS_CTL0;
        sys_ctl0_status    |= SYS_CTL0_JPEG_CKEN;
        SYS_CTL0            = sys_ctl0_status;
        jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 85);
        for(i = 0; i < 7; i++)
        {
    	    exifSetImageResolution(Width, Height); //暫時用
    	    jpegCapturePreviewImg((u8*)exifPrimaryImage.bitStream,
    	                             srcImgY,
    	                             JPEG_OPMODE_FRAME, Width, Height);
    	    JpegImagePixelCount = Width * Height;   //GetJpegImagePixelCount();
            primaryImageSize    = WaitJpegEncComplete();
            DEBUG_SYS("primaryImageSize=%d\n",primaryImageSize);

        #if 1
            if(  
                 (exifPrimaryImage.bitStream[primaryImageSize - 2] == 0xff) &&
                 (exifPrimaryImage.bitStream[primaryImageSize - 1] == 0xd9) &&
                 (primaryImageSize < ( (1280 * 720/3)- (68*1024)) )
              )  // JPEG bitstream good
            {
                break;
            } 
            else 
            {   // JPEG bitstream fail, 用較低的品直再壓一次
                DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                switch(i)
                {
                case 0:
                    DEBUG_SYS("jpegSetQuantizationQuality(80)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 80);
                    break;
                case 1:
                    DEBUG_SYS("jpegSetQuantizationQuality(70)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 70);
                    break;
                case 2:
                    DEBUG_SYS("jpegSetQuantizationQuality(60)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 60);
                    break;
                case 3:
                    DEBUG_SYS("jpegSetQuantizationQuality(50)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 50);
                    break;
                case 4:
                    DEBUG_SYS("jpegSetQuantizationQuality(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 40);
                    break;
                case 5:
                    DEBUG_SYS("jpegSetQuantizationQuality(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 30);

                case 6:
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
        CIU_1_CTL2 = 0;

        //------//
     #if ADDAPP3TOJPEG   
        uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;        
        ciuPreviewInit_CH1(SIUMODE_CAP_RREVIEW,isuSrcImg.w,isuSrcImg.h,704,480,0,0,CIU2_OSD_EN,704);
        Width=704;
        Height=480;
        while(ciu_idufrmcnt_ch1<1);
        //ciu_1_Stop();
        CIU_1_CTL2 &= (~(0x00000040 | 0x00000020));  //for speed up
        DEBUG_SYS("--->APP3 %d x %d\n",Width,Height);

        srcImgY  = PNBuf_sub1[0];
        srcImgUV    = srcImgY + PNBUF_SIZE_Y;
        
        #if((CHIP_OPTION == CHIP_A1016A))//Lucian: walk around A1016A's bug.
        pp = srcImgY+ Width*2*(Width/2);
        memcpy(pp,pp+Width*2,Width*2);      
        #endif


        jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 85);
        for(i = 0; i < 7; i++)
        {
           jpegCaptureAPP3Image((u8*)exifAPP3VGAImage.bitStream,srcImgY,JPEG_OPMODE_FRAME,JPGAPP3_WIDTH * 2);
           App3VGAImageSize=WaitJpegEncComplete();
           DEBUG_SYS("App3VGAImageSize=%d\n",App3VGAImageSize);

           if(  
                 (exifAPP3VGAImage.bitStream[App3VGAImageSize - 2] == 0xff) &&
                 (exifAPP3VGAImage.bitStream[App3VGAImageSize - 1] == 0xd9) &&
                 (App3VGAImageSize < 63*1024)
              )  // JPEG bitstream good
            {
                break;
            } 
            else 
            {   // JPEG bitstream fail, 用較低的品直再壓一次
                DEBUG_SYS("JPEG encode fail bitstream, using poorer quality try again...\n");
                switch(i)
                {
                case 0:
                    DEBUG_SYS("jpegSetQuantizationQuality1(80)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 80);
                    break;
                case 1:
                    DEBUG_SYS("jpegSetQuantizationQuality1(70)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 70);
                    break;
                case 2:
                    DEBUG_SYS("jpegSetQuantizationQuality1(60)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 60);
                    break;
                case 3:
                    DEBUG_SYS("jpegSetQuantizationQuality1(50)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 50);
                    break;
                case 4:
                    DEBUG_SYS("jpegSetQuantizationQuality1(40)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 40);
                    break;
                case 5:
                    DEBUG_SYS("jpegSetQuantizationQuality1(30)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 30);

                case 6:
                    DEBUG_SYS("jpegSetQuantizationQuality1(20)\n");
                    jpegSetQuantizationQuality(JPEG_IMAGE_APP3IMG, 20);    
                    break;
                    
                default:
                    DEBUG_SYS("JPEG encoder something wrong!!!\n");
                }
            }
        }
        exifAPP3Prefix.APP3Marker=0xe3ff; //swap hi/lo byte
        data=sizeof(EXIF_PRIMARY)+App3VGAImageSize+2+4;
        exifAPP3Prefix.APP3Size= (data<<8)|(data>>8); //swap hi/lo byte
        exifAPP3Prefix.ID=0x12345678;
        CIU_1_CTL2 = 0;
     #endif
        //---Disable JPEG clock---//
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
  #if ADDAPP3TOJPEG      
        //if (dcfWrite(pFile, (unsigned char*)&exifAPP3Prefix, sizeof(DEF_APP3PREFIX), &size) == 0)
        memcpy(pp, (unsigned char*)&exifAPP3Prefix, sizeof(DEF_APP3PREFIX)); 
        pp +=sizeof(DEF_APP3PREFIX);
        TotalSize += sizeof(DEF_APP3PREFIX);
        
        //if (dcfWrite(pFile, (unsigned char*)&exifAPP3VGAImage, sizeof(EXIF_PRIMARY), &size) == 0)
        memcpy(pp, (unsigned char*)&exifAPP3VGAImage, sizeof(EXIF_PRIMARY)); 
        pp +=sizeof(EXIF_PRIMARY);
        TotalSize += sizeof(EXIF_PRIMARY);
        
        //if (dcfWrite(pFile, (unsigned char*)exifAPP3VGAImage.bitStream, Global_APP3VGAImageSize, &size) == 0)
        memcpy(pp, (unsigned char*)exifAPP3VGAImage.bitStream, App3VGAImageSize); 
        pp +=App3VGAImageSize;
        TotalSize += App3VGAImageSize;
   #endif
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
        ciu_1_OpMode = SIUMODE_PREVIEW;
        sysCameraMode = SIUMODE_PREVIEW;
        uiMenuVideoSizeSetting=OldSetting;
        rfiuVideoInFrameRate=siuSensorInit(SIUMODE_CAP_RREVIEW,0);
        setSensorWinSize(0, SIUMODE_CAP_RREVIEW);

        DEBUG_SYS("==sysTxCaptureImage_CIU1 End:%d==\n",TotalSize);
    }

    int sysTXSnapshotCheck(void)        
    {
        RTC_DATE_TIME   dateTime;
        int SnapMin,CurrMin;

        RTC_Get_Time(&dateTime);
        CurrMin=dateTime.hour*60 + dateTime.min;
        SnapMin=rfiuTXSnapCheck.SnapTimeInMin;
        DEBUG_SYS("--sysTXSnapshotCheck:[%d:%d][%d,%d]--\n",dateTime.hour,dateTime.min,rfiuTXSnapCheck.SnapTimeInMin/60,rfiuTXSnapCheck.SnapTimeInMin % 60);
        if(rfiuTXSnapCheck.ReCheck)
        {
             if(rfiuTXSnapCheck.SnapStatus == RF_TXPHOTOSTA_DONE_OK)
             {
                rfiuTXSnapCheck.CheckCnt =0;
                rfiuTXSnapCheck.ReCheck=0;
                rfiuTXSnapCheck.SnapStatus=RF_TXPHOTOSTA_NONE;
             }
             else
             {
                rfiuTXSnapCheck.CheckCnt ++;
             }
             
             DEBUG_SYS("--Count %d--\n",rfiuTXSnapCheck.CheckCnt);
             if(rfiuTXSnapCheck.CheckCnt >= 10*2)
             {
                 DEBUG_SYS("====PUSH MESSAGE: NO Snapshot====\n");
                 rfiuTXSnapCheck.CheckCnt=0;
                 rfiuTXSnapCheck.ReCheck=0; 
               #if USB2WIFI_SUPPORT
                 Snapshot_Error = 1;
               #endif

             }
        }
        else
        {
             if(SnapMin == CurrMin)
             {
                DEBUG_SYS("----TX SnapTime's up----\n");
                rfiuTXSnapCheck.ReCheck=1;                
                rfiuTXSnapCheck.CheckCnt =0;
             }
        }

    }

#endif
#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
s32 sysTX8211_EnterWifi(s32 P2PQuailty)
{
    int temp;
    int i;
    s32 TimeOffset;
    int width,height;
    u32 Setting;
    
    //----------------------------------//  
    rfiu_TX_P2pVideoQuality=P2PQuailty;

	#if ICOMMWIFI_SUPPORT
	iComm_WIFI_Status();
    rfiu_TX_WifiCHNum= (2407+M7688_WifiCHNum*5-2408+2)/4;  //對應到2408~2468MHz
    #else
    rfiu_TX_WifiPower=M7688_WifiPower;
    rfiu_TX_WifiCHNum= (2407+M7688_WifiCHNum*5-2408+2)/4;  //對應到2408~2468MHz
	#endif
    if(rfiu_TX_WifiCHNum>15)
        rfiu_TX_WifiCHNum=15;
    
    if(rfiu_TX_WifiCHNum<0)
        rfiu_TX_WifiCHNum=0;
    
    //----------------------------------//
    switch(P2PQuailty)
    {
       case RFWIFI_P2P_QUALITY_HIGH:
           Setting=UI_MENU_VIDEO_SIZE_1280X720;
           break;
       
       case RFWIFI_P2P_QUALITY_MEDI:
           Setting=UI_MENU_VIDEO_SIZE_704x480;
           break;

       case RFWIFI_P2P_QUALITY_LOW:
           Setting=UI_MENU_VIDEO_SIZE_704x480;
           break;
    }
    //-----//
    DEBUG_SYS("==sysTX8211_EnterWifi:%d==\n",P2PQuailty);
    if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD)  )
    {
        DEBUG_SYS("==Quad:%d==\n",rfiu_TX_P2pVideoQuality);
        sys8211TXWifiStat=MR8211_ENTER_WIFI;
        rfiu_TX_P2pVideoQuality=P2PQuailty;
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
        {
          #if USB2WIFI_SUPPORT
           Reset_RES = 0;
          #endif
           return 1;
        }
        else
            Setting=UI_MENU_VIDEO_SIZE_352x240;
           
    }
#if 0    
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) && (RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0)) )
    {
        DEBUG_SYS("==Quad2:%d==\n",rfiu_TX_P2pVideoQuality);
        sys8211TXWifiStat=MR8211_ENTER_WIFI;
        rfiu_TX_P2pVideoQuality=P2PQuailty;
        return 1;

    } 
#endif
    else if( Setting == uiMenuVideoSizeSetting) //single view
    {
        DEBUG_SYS("==Same Reso:%d==\n",rfiu_TX_P2pVideoQuality);
        sys8211TXWifiStat=MR8211_ENTER_WIFI;
      #if USB2WIFI_SUPPORT
        Reset_RES = 0;
      #endif
        return 1;
    }
  #if USB2WIFI_SUPPORT
    Reset_RES = 1;
  #endif

    //-----//
     #if RF_TX_OPTIMIZE   
        temp=gRfiuUnitCntl[0].RFpara.MD_en;
        gRfiuUnitCntl[0].RFpara.MD_en=0;
        //--TX--//
        gRfiuUnitCntl[0].TX_Task_Stop=1;
        gRfiuUnitCntl[0].TX_Wrap_Stop=1;
        gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
        gRfiuUnitCntl[0].TX_IIS_Stop=1;

        OSTimeDly(6); 

        for(i=0;i<100;i++)
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
        iisStopRec();
        iisSuspendTask();
        OSTaskDel(IIS_TASK_PRIORITY);
        DEBUG_SYS("====IIS task Delete====\n");
        
        #if RFIU_RX_AUDIO_RETURN
            if(guiIISPlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISPlayDMAId);
                guiIISPlayDMAId = 0xFF;
            }
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

            break;
        }
        //---------main body----------//
        uiMenuVideoSizeSetting=Setting;
        sys8211TXWifiStat=MR8211_ENTER_WIFI;
    	switch (uiMenuVideoSizeSetting)
    	{	/*CY 0907*/
            case UI_MENU_VIDEO_SIZE_720x480:
                width = 720;
                height = 480;
    			break;

            case UI_MENU_VIDEO_SIZE_704x480:
                width = 704;
                height = 480;
    			break;

            case UI_MENU_VIDEO_SIZE_352x240:
                width = 352;
                height = 240;
    			break;    

            case UI_MENU_VIDEO_SIZE_720x576:
                width = 720;
                height = 576;
    			break;

            case UI_MENU_VIDEO_SIZE_704x576:
                width = 704;
                height = 576;
    			break;

            case UI_MENU_VIDEO_SIZE_320x240:
                width   = 320;
            	height  = 240;
                break;

            case UI_MENU_VIDEO_SIZE_1280X720:
                width   = 1280;
                height  = 720;
                break;

    		case UI_MENU_VIDEO_SIZE_640x480:
                width  = 640;
                height = 480;
                break;

            case UI_MENU_VIDEO_SIZE_1920x1088:
                width  = 1920;
                height = 1088;
                break;

    		default:
    	        width  = 640;
                height = 480;
    			break;
    	}
        asfSetVideoResolution(width, height);

        //----------------------------//
        sysPreviewInit(0);
        
        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_OpMode = SIUMODE_MPEGAVI;
        ciu_1_FrameTime = 0;
        #elif(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
        ciu_2_FrameTime = 0;
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
        ciu1ZoomStart=1;
   #endif


    return 1;
}

s32 sysTX8211_LeaveWifi(s32 dummy)
{
    int temp;
    int i;
    s32 TimeOffset;
    int width,height;
    u32 Setting;
    //----------------------------------//  
    DEBUG_SYS("==sysTX8211_LeaveWifi==\n");
    
    Setting=UI_MENU_VIDEO_SIZE_704x480;
    if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) )
    {
        DEBUG_SYS("==Quad==\n");
        sys8211TXWifiStat=MR8211_QUIT_WIFI;
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
            return 1;
        else
            Setting=UI_MENU_VIDEO_SIZE_352x240;
    }
 #if 0   
    else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)  )
    {
        DEBUG_SYS("==Quad2:%d==\n",rfiu_TX_P2pVideoQuality);
        sys8211TXWifiStat=MR8211_QUIT_WIFI;
        return 1;
    } 
 #endif
    else if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_704x480)
    {
        DEBUG_SYS("==Same as D1==\n");
        sys8211TXWifiStat=MR8211_QUIT_WIFI;
        return 1;
    }

    //----------------------------------//
    
     #if RF_TX_OPTIMIZE   
        temp=gRfiuUnitCntl[0].RFpara.MD_en;
        gRfiuUnitCntl[0].RFpara.MD_en=0;
        //--TX--//
        gRfiuUnitCntl[0].TX_Task_Stop=1;
        gRfiuUnitCntl[0].TX_Wrap_Stop=1;
        gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
        gRfiuUnitCntl[0].TX_IIS_Stop=1;

        OSTimeDly(6); 

        for(i=0;i<100;i++)
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
        iisStopRec();
        iisSuspendTask();
        OSTaskDel(IIS_TASK_PRIORITY);
        DEBUG_SYS("====IIS task Delete====\n");
        
        #if RFIU_RX_AUDIO_RETURN
            if(guiIISPlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISPlayDMAId);
                guiIISPlayDMAId = 0xFF;
            }
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

            break;
        }
        //--------main body-----------//
        uiMenuVideoSizeSetting=Setting;
        sys8211TXWifiStat=MR8211_LEAVING_WIFI;
    	switch (uiMenuVideoSizeSetting)
    	{	/*CY 0907*/
            case UI_MENU_VIDEO_SIZE_720x480:
                width = 720;
                height = 480;
    			break;

            case UI_MENU_VIDEO_SIZE_704x480:
                width = 704;
                height = 480;
    			break;

            case UI_MENU_VIDEO_SIZE_352x240:
                width = 352;
                height = 240;
    			break;    

            case UI_MENU_VIDEO_SIZE_720x576:
                width = 720;
                height = 576;
    			break;

            case UI_MENU_VIDEO_SIZE_704x576:
                width = 704;
                height = 576;
    			break;

            case UI_MENU_VIDEO_SIZE_320x240:
                width   = 320;
            	height  = 240;
                break;

            case UI_MENU_VIDEO_SIZE_1280X720:
                width   = 1280;
                height  = 720;
                break;

    		case UI_MENU_VIDEO_SIZE_640x480:
                width  = 640;
                height = 480;
                break;

            case UI_MENU_VIDEO_SIZE_1920x1088:
                width  = 1920;
                height = 1088;
                break;

    		default:
    	        width  = 640;
                height = 480;
    			break;
    	}
        asfSetVideoResolution(width, height);
        //----------------------------//
        sysPreviewInit(0);
        sys8211TXWifiStat=MR8211_QUIT_WIFI;
        
        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_OpMode = SIUMODE_MPEGAVI;
        ciu_1_FrameTime = 0;
        #elif(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
        ciu_2_FrameTime = 0;
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
        ciu1ZoomStart=1;
       
    #endif


    return 1;
}
#endif
   
   #if RX_SNAPSHOT_SUPPORT
   s32 sysBack_RFI_RX_DataSave(s32 RFUnit)
   {
       FS_FILE* pFile;
       u32 ret;    
	   u8  tmp;
 
       DEBUG_SYS("==sysBack_RFI_RX_DataSave:%d==\n",RFUnit);
       //----------------------------------------//
       dcfWriteNextPhotoFile(RFUnit,rfiuRxDataBufMng[RFUnit].buffer,rfiuRxDataBufMng[RFUnit].size);
       //----------------------------------------//
       rfiuRxDataBufMng[RFUnit].buffer=rfiuRxDataBuf[RFUnit];
       rfiuRxDataBufMng[RFUnit].size =0;
       uiRetrySnapshot[RFUnit] = UI_SET_RF_OK;
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

    u8* DataAddr = (u8*)(SdramBase + SDRAM_SIZE - SPI_MAX_CODE_SIZE);
	u8*	DataAddrToSD = DataAddr;
	u32	unSize;
	s32 i, j, x;
	s32	k;
	u32	unAddr;
	u32 WriteSize;
	u32	ReadMaxSize;
    u8  BYTE_SWAP[4];
	u8	tmp;
	
    FS_FILE* pFile;

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

}



s32 sysBack_Draw_Battery(s32 level)
{
#if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    uiOsdDrawBattery(level, UI_OSD_DRAW);
#else
    uiOsdDrawBattery(level);
#endif;
    return 1;
}

s32 sysBack_Check_UI(s32 level)
{
    if(syscheckSDStatus == 1)
    {
        if(uiSentKeyToUi(UI_KEY_SDCD) == 1)
            syscheckSDStatus = 0;    
    }
	uiFlowRunPerSec(); //Lsk 090429 : appointment video recording for ebell

}

s32 sysBack_Check_UI_500ms(s32 level)
{
    #if 0//((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
    uiFlowRunPerSec_500ms();
    #endif
}


#if(HOME_RF_SUPPORT)
s32 sysBack_Check_HOMERF(s32 level)
{
    homeRFRunPerSec();    
}

#endif



/******************************************
  Check video in source : TV or Sensor
*******************************************/
u32 sysBack_Check_VideoinSource(u32 dummy)
{

    return 1;
}

u32 sysBack_Draw_BitRate(u32 value)
{
    uiOsdDrawBitRate(value);
    return 1;
}

u32 sysBack_Draw_FrameRate(int RFUnit)
{    
    int value;

    value=gRfiuUnitCntl[RFUnit].FrameRate;
    uiOsdDrawFrameRate(RFUnit);
    return 1;
}

u32 sysBack_Draw_OSDString(u32 value)
{
    #if(HW_BOARD_OPTION != MR8120_RX_JESMAY )
        if (value == MSG_ASCII_STR)
        {
            uiOSDASCIIStringByColor(sysBackOsdString, 60 , 120, OSD_Blk2 , 0xc6, 0x00);
        }
        else
        {
            uiOSDMultiLanguageStrCenter(value, OSD_Blk2, 0xc6, 0x00);
        }
    #endif
    return 1;
}

u32 sysBack_Draw_SD_Icon(u32 value)
{
    osdDrawSDIcon(value);
    return 1;
}
#if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
u32 sysBack_Turn_SPK_GPIO(u8 value)
{
    u8 data=0;
    if(value==1)
    {
        i2cRead_WT6853(0x01, &data);
        data &= ~0xf;
        DEBUG_SYS("===> TURN SPK 0\n");
        gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 1);
        OSTimeDly(1);
        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0);
        data |= 0xf;
        i2cWrite_WT6853(0x01, data);
    }
    else
    {
        DEBUG_SYS("===> TURN SPK 1\n");
        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
        OSTimeDly(1);
        gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
    }
    return 1;
}
#elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
u32 sysBack_Turn_SPK_GPIO(u8 value)
{
    return 1;
}
#elif((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
u32 sysBack_Draw_Sound_Bar(u8 value)
{
    uiOsdDrawSoundBar(value);
    return 1;
}
#endif
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

s32 sysUpgradeFW(s32 snKey)
{
    s32 isp_return;
    u8  err;

    if(gInsertCard == 0)
    {
        DEBUG_UI("uiMenuSet_UpgradeFW No Sd Card\n");
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
        return 0;
    }
    sysSD_Enable();
    /*disable watch dog when update firmware*/
    sysDeadLockMonitor_OFF();
    isp_return=ispUpdateAllload();//usb boot
    if(isp_return ==0)
    {
        ispUpdatebootload();
        isp_return=ispUpdate(1);		 // Check whether spiFW.bin exists ot not. If exist then update
    }
    /*enable watch dog when update firmware finish*/
    sysDeadLockMonitor_ON();
    if (isp_return == 0)
        OSMboxPost(general_MboxEvt, "FAIL");
    else
        OSMboxPost(general_MboxEvt, "PASS");
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
    if (isp_return == 0)
		return 0;
	else
    	return 1;
}


