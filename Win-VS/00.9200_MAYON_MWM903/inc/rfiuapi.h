
#if PWIFI_SUPPORT
   #include "pwifiapi.h"
#endif

#ifndef __RFIU_API_H__
#define __RFIU_API_H__
   #define RFI_PAYLOAD_SIZE                   128                      // 1 payload = 128 bytes
   #define RFI_GRP_INPKTUNIT                  64
   #define RFI_BUF_SIZE_GRPUNIT               128                      //buf size = N group-unit ,group unit: 2^N
   #define RFI_TOTAL_BUF_SIZE                 (RFI_GRP_INPKTUNIT*RFI_BUF_SIZE_GRPUNIT*RFI_PAYLOAD_SIZE)

   #define RFI_AUIDIO_SILENCE_SIZE             1024

   #define RFI_VIDEO_SYNC_SHIFT                33

   //------//
   #define FCC_DIRECT_TX_MODE                  0
   #define FCC_DIRECT_RX_MODE                  1
   #define FCC_DIRECT_TXSCAN_MODE              2
   //----------------------------------RFIU_TEST must be 1 or 2 ----------------------------// 
#if RFIU_TEST   

   #define DEBUG_MAP_PKTNUM               0
    //===========工程模式==========//
   #define RFI_MEASURE_RX1RX2_SENSITIVITY 0

   #define RFI_FCC_DIRECT_TRX             1  // 0: Turn off FCC test,  1: test only 1 RF, 2: test two RF

   //==Self-Test Packet burst from 1 ~ 256 ==//
   #define RFI_TEST_PKTBURST              0

   //==Self-Test Packet map randomly==//
   #define RFI_TEST_PKTMAP                0

   //==Self-Test RFIU performance on AIR==//
   #define RFI_TEST_PERFORMANCE           0
   #define RFI_TEST_PERFO_NUM             600   
   #define RFI_FPGA_PERFORMANCE_MEASURE   1
   #define RFI_FPGA_NOISE_GEN             0

   //==Self-Test RFIU Transmit or Receive on AIR==// Lucian: 用於不同PCB
   #define RFI_TEST_TXRX_FUN              0    //0:OFF,1:TX,2:RX

   //==Self-Test Tx/Rx Communication scheme==//
   #define RFI_TEST_TXRX_COMMU            0
   #define RFI_TEST_TXRX_NUM              5000
   #define RFI_TEST_WRAP_OnCOMMU          0    //測試 Wrapper,需搭配設定RFIU_TEST=2

   //==Self-Test Tx/Rx protocol in the same PCB==//
   #define RFI_SELF_TEST_TXRX_PROTOCOL    0    //Lucian: 目前有bug,暫時不能用.
   #define RFI_TEST_WRAP_OnPROTOCOL       0
#else
  //==Self-Test Packet burst from 1 ~ 256 ==//
   #define RFI_TEST_PKTBURST              0

   //==Self-Test Packet map randomly==//
   #define RFI_TEST_PKTMAP                0

   //==Self-Test RFIU performance on AIR==//
   #define RFI_TEST_PERFORMANCE           0
   #define RFI_TEST_PERFO_NUM             200   
   #define RFI_FPGA_PERFORMANCE_MEASURE   1
   #define RFI_FPGA_NOISE_GEN             0

   //==Self-Test RFIU Transmit or Receive on AIR==// Lucian: 用於不同PCB
   #define RFI_TEST_TXRX_FUN              0    //0:OFF,1:TX,2:RX

   //==Self-Test Tx/Rx Communication scheme==//
   #define RFI_TEST_TXRX_COMMU            0
   #define RFI_TEST_TXRX_NUM              5000
   #define RFI_TEST_WRAP_OnCOMMU          0    //測試 Wrapper,需搭配設定RFIU_TEST=2

   //==Self-Test Tx/Rx protocol in the same PCB==//
   #define RFI_SELF_TEST_TXRX_PROTOCOL    0
   #define RFI_TEST_WRAP_OnPROTOCOL       0
#endif
   
   #define  RFIU_DEBUGPORT_EN  0    //Lucian: enable debug port

   
   //---- RF Paring ----//
   #define  RF_PAIR_EN                   1

   //----RX command ----//
   #define  RF_CMD_EN                    1

   //-----AV sync--------//
   #define RF_AV_SYNCTIME_EN             1
   
   #define RFRXCMD_SET_TIME              0
   #define RFRXCMD_SET_RESOLUTION        1 
   #define RFRXCMD_SET_BRIGHTNESS        2
   #define RFRXCMD_SET_PIRCFG            3
   #define RFRXCMD_SET_OPMODE            4
   #define RFRXCMD_SET_MDCFG             5
   #define RFRXCMD_SET_TXOSD             6   
   #define RFRXCMD_SET_ZOOM              7
   #define RFRXCMD_SET_PTZ485            8
   #define RFRXCMD_SET_FLICKER           9
   #define RFRXCMD_SET_GPO               10
   #define RFRXCMD_SET_OTHERSPARAS       11   
   #define RFRXCMD_SET_MASKAREA_VGA      12
   #define RFRXCMD_SET_MASKAREA_HD       13
   #define RFRXCMD_SET_SLEEP             14
   #define RFRXCMD_SET_EXTEND            15

   #define RFRXCMD_SETEXT_PWRTURBO       0
   #define RFRXCMD_SETEXT_DOORBELLOFF    1
   #define RFRXCMD_SETEXT_MDSENSTAB      2
   #define RFRXCMD_SETEXT_CHGCH          3  //used in FCC247 protocol
   #define RFRXCMD_SETEXT_PWM            4
   #define RFRXCMD_SETEXT_MOTORCTL       5
   #define RFRXCMD_SETEXT_MELODYNUM      6
   #define RFRXCMD_SETEXT_VOXCFG         7
   #define RFRXCMD_SETEXT_SUBSTR_EN      8
   #define RFRXCMD_SETEXT_SAT_CON_SHP    9  //sent saturation,contrast,sharpness
   #define RFRXCMD_SETEXT_VOLUME         10
   #define RFRXCMD_SETEXT_LIGHTONOFF     11
   #define RFRXCMD_SETEXT_SCHED          12
   #define RFRXCMD_SETEXT_SNAPSHOT       13 // TX snap-shot
   #define RFRXCMD_SETEXT_REBOOT         14 // TX reboot.
   #define RFRXCMD_SETEXT_FBAPPSTA       15 // feed back APP status
   #define RFRXCMD_SETEXT_SLEEPRECNT     16 // Reset TX Sleep Time

   //----TX Information----//
   #define RFTXINFO_SET_VOXTRIG          0     // not used
   #define RFTXINFO_SET_LIGHT_STA        1
   #define RFTXINFO_SET_BODYINFO         2     // not used
   #define RFTXINFO_SET_TXINFO           3     //  byte1: Battery level, byte2: wifi status.
   

   //------TX Command Mask--------//
   #define RFTXCMD_SEND_NONE            0x0000
   #define RFTXCMD_SEND_DATA            0x0001
   #define RFTXCMD_SEND_AUDIORETMAP     0x0002
   #define RFTXCMD_SEND_TVTYPECHG       0x0004
   
   //---------TX Status via Sync--------//
   #define RFIU_TX_STA__FIELD_ENC       0x00000001
   #define RFIU_TX_STA__PAL_MODE        0x00000002  //no used now
   #define RFIU_TX_STAT_FWUPD_SUPPORT   0x00000002
   #define RFIU_TX_STA__MD_ON           0x00000004
   #define RFIU_TX_STA__PIR_ON          0x00000008

   #define RFIU_TX_STA__HD_SUPPORT      0x00000010
   #define RFIU_TX_STA_TIMESTAMP_ON     0x00000020  
   #define RFIU_TX_STA_MPEG_Q           0x00000040  //no used now
   #define RFIU_TX_STA_MULTISTREAM_ON   0x00000040
   #define RFIU_TX_STA_FLICKER_50HZ     0x00000080

   #define RFIU_TX_STA__FULLHD_SUPPORT  0x00000100
   #define RFIU_TX_STAT_BELL_ON         0x00000200
   #define RFIU_TX_STAT_PROTCOL_SEL     0x00000c00

   //bit 12~15 reserved for rfiuRX_OpMode
   #define RFIU_TX_STA_OPMODE           0x0000f000  //bit12~15 for TX Op mode
  
   #define RFIU_TX_STA_CAMNUM           0x00030000  //bit 16~17  for Camera num of conecting to RX. 目前看起來對RX沒有作用,可以做其他用途.
   #define RFIU_TX_STA_BATCAM_PIRMODE   0x00040000  //but 18: hint to RX that TX enter into PIR recording mode.
   #define RFIU_TX_STA_BATCAM_SUPPORT   0x00080000  //bit 19: Support battery cammera function: Sleep,Wakeup
  
   #define RFIU_TX_STA_BRIGHNESS        0x00f00000  //bit 23~20 for Camera brighness value
   #define RFIU_TX_STA_MD_SENS          0xff000000  //bit 31~24 for Camera MD sensitivity

   //----------TX Status2 via Sync-------------//
   #define RFIU_TX_STA2_BATTLV          0x0000000f  //bit 0~3 for battery level
   #define RFIU_TX_STA2_FAULSETRIG      0x00000010  //bit 4 for PIR false trigger.
   #define RFIU_TX_STA2_ALERMON         0x00000020  //bit 5 for TX alerm on
   #define RFIU_TX_STA2_DOORBELL_SUPPORT 0x00000040  //bit 6 for TX BATTERY's DOORBELL_SUPPORT

   
   //------------------------------------------//
   #define RFIU_TX_MODE        0
   #define RFIU_RX_MODE        1
   #define RFIU_IDLE_MODE      2
   #define RFIU_SYNC_MODE      3
   #define RFIU_PAIR_MODE      4
   #define RFIU_PAIRLint_MODE  5
   #define RFIU_TX_CMD_MODE    6
   #define RFIU_PAIR_STOP_MODE 7
   #define RFIU_TX_FWUPGRADE   8

   #define RFIU_FWUPD_INIT     9
   #define RFIU_FWUPD_START    10
   #define RFIU_FWUPD_DONE     11

   //-------------RX Operation Mode--------------//
   #define RFIU_RX_OPMODE_NORMAL    0x00000000
   #define RFIU_RX_OPMODE_QUAD      0x00000001
   #define RFIU_RX_OPMODE_P2P       0x00000002
   #define RFIU_RX_OPMODE_P2P_MULTI 0x00000004
   #define RFIU_RX_OPMODE_PIP       0x00000008

   #define RFIU_RX_OPMODE_MAINCH    0x00000010
   #define RfIU_RX_OPMODE_FORCE_I   0x00000020
   #define RFIU_RX_OPMODE_PCM_SIGN  0x00000040

   //-------------RF Status------//
   #define RFIU_OP_INIT             0x00
   
   #define RFIU_RX_STA_PAIR_OK      0x01
   #define RFIU_TX_STA_PAIR_OK      0x01
   
   #define RFIU_RX_STA_LINK_BROKEN  0x02
   #define RFIU_TX_STA_LINK_BROKEN  0x02
   
   #define RFIU_RX_STA_LINK_OK      0x04
   #define RFIU_TX_STA_LINK_OK      0x04

   #define RFIU_RX_STA_ERROR        0x08
   //---//
   //#define RFIU_RX_STA_CHGRESO      0x10
   //#define RFIU_RX_STA_FRAMESYNC    0x20

   //----------------------------//
   #define RFI_UNIT_0                0
   #define RFI_UNIT_1                1
   #define RFI_UNIT_2                2
   #define RFI_UNIT_3                3

  #if (RF_CMD_EN) 
   #define RFIU_USEPKTMAP_MAX        20
  #else
   #define RFIU_USEPKTMAP_MAX        24  
  #endif
  
   #define RFIU_PASSWORD_MAX         2  
   #define RFIU_TXCMDDATA_MAX        5
   #define RFI_AUDIO_RET_GRPNUM      4


   #define RFI_DAT_CH_MAX            16  //must be 2^n, max to 32
 #if 0
   #define RFI_ACK_CH_MAX            8   //max to 16
   #define RFI_ACK_CH_PIRIOD         10   // 2^RFI_ACK_CH_PIRIOD*100 (us)
 #else
   #define RFI_ACK_CH_MAX            16   //max to 16
   #define RFI_ACK_CH_PIRIOD         10   // 2^RFI_ACK_CH_PIRIOD*100 (us)
 #endif
   
   #define RF868_DEVICE_MAX          4

   #define RFI_FEC_TYPE_MAX          4

   #define RFRX_HALF_MODE_SHIFT      180    //Panel use
   #define RFRX_HALF_MODE_SHIFT_TV   180    //TV use

   //------------For 868-------//
     //New protocol
     #define  RF868_STA_LOWBETERY_V1     0x8000
     #define  RF868_STA_POWOFF_V1        0x4000
     #define  RF868_STA_FROMIR_V1        0x2000
     #define  RF868_STA_TRIG_V1          0x1000

     #define  RF868_STA_TYPEMASK_V1      0x0f
     #define  RF868_STA_TYPESHF_V1       8

     #define  RF868_DEVICE_IR_V1         8
     #define  RF868_DEVICE_DOOR_V1       0
     #define  RF868_DEVICE_SMOKE_V1      2
     #define  RF868_DEVICE_WATER_V1      1
     #define  RF868_DEVICE_INPIR_V1      3
     #define  RF868_DEVICE_GAS_V1        4
     #define  RF868_DEVICE_VIR_V1        5
     #define  RF868_DEVICE_OUTPIR_V1     6
     #define  RF868_DEVICE_ALARM_V1      7

     #define  RF868_OPMODE_SOS_V1        9
     #define  RF868_OPMODE_ALARM_V1      10
     #define  RF868_OPMODE_SILENCE_V1    12
     #define  RF868_OPMODE_HOME_V1       11
     #define  RF868_OPMODE_UNLOCK_V1      8


     //----Old protocol----//
     #define  RF868_STA_LOWBETERY_V0     0x8000
     #define  RF868_STA_POWOFF_V0        0x4000
     #define  RF868_STA_FROMIR_V0        0x1000
     #define  RF868_STA_TRIG_V0          0x0100

     #define  RF868_STA_TYPEMASK_V0      0x07
     #define  RF868_STA_TYPESHF_V0       9

     #define  RF868_DEVICE_IR_V0         255
     #define  RF868_DEVICE_DOOR_V0       4
     #define  RF868_DEVICE_SMOKE_V0      6
     #define  RF868_DEVICE_WATER_V0      5
     #define  RF868_DEVICE_INPIR_V0      7
     #define  RF868_DEVICE_GAS_V0        255
     #define  RF868_DEVICE_VIR_V0        255
     #define  RF868_DEVICE_OUTPIR_V0     255
     #define  RF868_DEVICE_ALARM_V0      255

     #define  RF868_OPMODE_SOS_V0        14
     #define  RF868_OPMODE_ALARM_V0      11
     #define  RF868_OPMODE_SILENCE_V0    9
     #define  RF868_OPMODE_HOME_V0       13
     #define  RF868_OPMODE_UNLOCK_V0     7

     //---------TX Sensor-------------// 
     #define TX_SENSORTYPE_VGA           0
     #define TX_SENSORTYPE_HD            1
     #define TX_SENSORTYPE_FHD           2
     #define TX_SENSORTYPE_UXGA          3
     #define TX_SENSORTYPE_4M            4

     //-----For audio return-----//
     #define RF_AUDIO_RET_OFF            0
     #define RF_AUDIO_RET_RX_USE         1
     #define RF_AUDIO_RET_APP_USE        2

     //TX substream BR index
     #define RF_SUBSTREAM_BR_H           0
     #define RF_SUBSTREAM_BR_M           1
     #define RF_SUBSTREAM_BR_L           2

   //======================//
    #define  RF_P2PVdoQalt_VGA_10_FPS    1
    #define  RF_P2PVdoQalt_VGA_15_FPS    2
    #define  RF_P2PVdoQalt_VGA_30_FPS    3 
    
    #define  RF_P2PVdoQalt_QHD_7_FPS     4
    #define  RF_P2PVdoQalt_QHD_10_FPS    5
    #define  RF_P2PVdoQalt_QHD_15_FPS    6
    #define  RF_P2PVdoQalt_QHD_30_FPS    7

    
    #define  RF_P2PVdoQalt_HD_5_FPS      8
    #define  RF_P2PVdoQalt_HD_7_FPS      9 
    #define  RF_P2PVdoQalt_HD_10_FPS     10 
    #define  RF_P2PVdoQalt_HD_15_FPS     11   
    #define  RF_P2PVdoQalt_HD_30_FPS     12   

    #define  RF_P2PVdoQalt_FHD_5_FPS     13
    #define  RF_P2PVdoQalt_FHD_10_FPS    14 
    //=======================//
    #define  RF_P2PVdoQalt_HIGH          0
    #define  RF_P2PVdoQalt_MEDIUM        1
    #define  RF_P2PVdoQalt_LOW           2

    #define  RFWIFI_P2P_QUALITY_HIGH     0
    #define  RFWIFI_P2P_QUALITY_MEDI     1
    #define  RFWIFI_P2P_QUALITY_LOW      2

    //------------------------//
    #define  RF_TXFWUPDATE_FROM_SD       0
    #define  RF_TXFWUPDATE_FROM_NET      1
    #define  RF_TXFWUPDATE_FROM_USB      2
    //-------------------------------------//
    #define RF_BATCAM_REC_INTERVAL       20    // @PIR mode, 錄影時間.
    #define RF_BATCAM_LIVE_MAXTIME      120    // Live view下,最大觀看時間, 萬一USER忘了關camera 下的保護機制.
    #define RF_BATCAM_RXWAKEUPTX_MAXTIME 15

    #define RF_BATCAM_TXSYNC_PIR_MAXTIME 15    // TX 無法連線時的斷電時間.
    #define RF_BATCAM_TXSYNC_WAK_MAXTIME 30
    
    #define RF_BATCAM_TXRUN_MAXTIME     150    // TX 上電後最長使用時間.
    //-------------------------------------//
    #define RF_BATCAM_TXBATSTAT_NOSHOW  15
    #define RF_BATCAM_TXBATSTAT_CHARGE  14
    #define RF_BATCAM_TXBATSTAT_FULL    13
    #define RF_BATCAM_TXBATSTAT_NOTREDY 12

    //-------------------------------------//
    #define PIRREC_TIMEEXTENDSEC_ARLO      7

    //-------------------------------------//
    typedef struct _RFIU_USRDATA
    {
       unsigned char Vitbi_en;     // 1 bit
       unsigned char RS_sel;       // 2 bit
       unsigned char Vitbi_sel;    // 2 bit
       unsigned char SeqenceNum;   // 2 bit
       unsigned char DAT_CH;       // 5 bit
       unsigned char GrpDivs;      // 3 bit
       unsigned char AudioSeqNum;  // 1 bit
       unsigned char reserved;
       
       int GrpShift;              // 6 bit
       
    }DEF_RFIU_USRDATA;

    typedef struct _RFIU_PACKETMAP_CNTL
    {
       unsigned char PktCount;
       unsigned char RetryCount;
       unsigned char WriteDiv;
       unsigned char ReadDiv;
       unsigned int PktMap0;
       unsigned int PktMap1;
    }DEF_RFIU_PACKETMAP_CNTL;

    typedef struct _RFIU_GROUP_MAP 
    {
       unsigned int Group;
       unsigned int Map0;
       unsigned int Map1;
    }DEF_RFIU_GROUP_MAP;

    typedef struct _RF868_CNTL_PARA 
    {
       unsigned int OpMode;
       unsigned int DeviceType[RF868_DEVICE_MAX];
    }DEF_RF868_CNTL_PARA;

    typedef struct _RFIU_UNIT_PARA
    {
       //==Motion Detectin==//
       int MD_en;
       int MD_Trig;
       int MD_Level_Day;
       int MD_Level_Night;

       //==PIR Detectin==//
       int PIR_en;
       int PIR_Trig;
       int PIR_RecDurationTime;
       int PIR_TrigEvent; // PIR or VMD
       int PIR_RecTimeExtendOn;

       //---TX Sensor Setting---//
       int TxSensorAE_FlickSetting;
       int TX_SensorType;
       int TX_SensorBrightness;
       int TX_SensorContrast;
       int TX_SensorSaturation;
       int TX_SensorSharpness;
       int TX_SensorFlipMirr;
       //---------------//
       int TX_TurboOn;
       int RX_TurboOn;
       int RX_DoorTrig;

       //---Others Para---//
       int TX_TimeStampOn;
       int TX_TimeStampType;

       int TX_SubStreamBRSel;
       int TX_SubStreamSupport;
    #if  RFIU_RX_WAKEUP_TX_SCHEME
       int BateryCam_support;
       int BatCam_PIRMode;
       int DoorBell_support;
    #endif

       //----Vox---//
       int VoxEna;
       int VoxThresh;
       
       //====TX Code Version====//
       char TxCodeVersion[32];

       //---ID----//
       u32 RF_ID;

       //---AV sync Time---//
       u32 VideoTotalTime;
       u32 AudioTotalTime;
    
       u32 RxForceTxFirstReboot;
       u32 TxBatteryLev;
       u32 TXPirFaulseTrig;
       u32 TxAlermtOn;
       
       //------WIFI Status----//
       u32 TXWifiStat;
       int TXWifiCHNum;
       int Rx_StaionJoinSta;

    }DEF_RFIU_UNIT_PARA;

    typedef struct _RFIU_UNIT_CNTL
    {
       unsigned int OpMode;
       unsigned int OpState;
       unsigned int BufReadPtr;
       unsigned int BufWritePtr;
       unsigned int WritePtr_Divs;
       unsigned int RunCount;
   #if (RFIU_RX_WAKEUP_TX_SCHEME || RFIU_TEST)
       unsigned int WakeUpTxEn;
       unsigned int SleepTxEn;
   #endif
       unsigned int DoSleepCmd;
   #if PWIFI_SUPPORT
       unsigned int PktMapGrp[PWIFI_PKTMAP_SIZE/32];
       unsigned int TxBurstNum;
       unsigned int TxDataBurstNum;
       unsigned int RxRecvDataStat;
       unsigned int prevTxDataBurstNum;
       unsigned int CmdCheck;
   #endif    
       unsigned int DataCHCnt;
       unsigned char GoodDataCHTab[RFI_DAT_CH_MAX];
	   unsigned int GoodDataCHNum;

       DEF_RFIU_USRDATA TX_CtrlPara;
       DEF_RFIU_USRDATA TX_CtrlPara_next;
       DEF_RFIU_USRDATA RX_CtrlPara;
       DEF_RFIU_USRDATA RX_CtrlPara_next;

       DEF_RFIU_PACKETMAP_CNTL TxPktMap[RFI_BUF_SIZE_GRPUNIT+1+1+8];  //16MB*1024/8=2048
       unsigned int TxBufFullness;
       unsigned int TX_PktCrtRate;

       int TxFEC_Level;
       int PrevTxFEC_Level;
   

       //--For Control--//
       int TX_Task_Stop;
       int TX_Wrap_Stop;
       int TX_MpegEnc_Stop;
       int TX_MpegEnc_StopRdy;
   #if RF_TX_OPTIMIZE  
       int TX_IIS_Stop;
   #endif

       int RX_Task_Stop;
       int RX_Wrap_Stop;
       int RX_MpegDec_Stop;

       //--Sync message--//
       int TX_PicWidth;
       int TX_PicHeight;
       unsigned int TX_Status;
       unsigned int TX_Status2;
       //---Paring---//
       int RX_Pair_Done;
       int TX_Pair_Done;
       
       //------------//
       unsigned int BitRate;  //Unit: 100K bps
       unsigned int FrameRate; //Unit: fps
    #if PWIFI_SUPPORT       
       unsigned int BitRateX100K;  //Unit: 100K bps
       unsigned int RxRecvDataCnt; //Unit: bytes
    #endif   
       //----Command----//
    #if PWIFI_SUPPORT
       unsigned char RX_CMD_Data[PWIFI_RXCMD_BYTESIZE];
       unsigned int  RX_PASSWORD[PWIFI_PASSWORD_MAX];  
    #else
       unsigned char RX_CMD_Data[12];
       unsigned int  RX_PASSWORD[RFIU_PASSWORD_MAX]; 
    #endif   
       unsigned char RXCmd_Type;     
       unsigned char RXCmd_en; 
       unsigned char RXCmd_Busy;   
       unsigned char RXCmd_AudioRetEn;
       unsigned int  RXRecvDataUse3M;
       //---//
       unsigned char TXCmd_en; 
       unsigned char TXCmd_Type;
     #if PWIFI_SUPPORT  
       unsigned int  TX_CMDPara[PWIFI_TXCMD_WORDSIZE];     
       unsigned int  PwifiAudioRetPktMap[PWIFI_AUDRET_PKTMAP_SIZE/32]; //
       unsigned char AudioRetPkt_RetryCnt[PWIFI_AUDRET_PKTMAP_SIZE]; 
     #else
       unsigned int  TX_CMDPara[RFIU_TXCMDDATA_MAX];
     #endif
       DEF_RFIU_GROUP_MAP AudioRetPktMap[RFI_AUDIO_RET_GRPNUM]; //
       unsigned int  TXSendDataUse3M;

       //----Protocol selection----//
       unsigned int ProtocolSel;
       unsigned int TXChgCHNext;

       unsigned int VMDSel;
       unsigned int FWUpdate_support;
       unsigned int FWUpdate_Source;

       //===FCC===//
       unsigned int FCC_TestMode;
       unsigned int FCC_TX_RealData_ON;
       unsigned int FCC_TX_NoData_Zero;
       unsigned int FCC_TX_Freq;
       unsigned int FCC_RX_Freq;
       //==Others==//

       DEF_RFIU_UNIT_PARA RFpara;  //Lucian: 必須放在最後面.
    }DEF_RFIU_UNIT_CNTL;

    typedef struct _RFIU_RXCMD_STATUS
    {
       int CamNum;
    }DEF_RFIU_RXCMD_STATUS;

#if RX_SNAPSHOT_SUPPORT
    typedef struct _DATA_BUF_MNG
    {
    	u32	size;
    	u8* buffer;	
    } DATA_BUF_MNG;
#endif
   //=========Extern Variable =========//
   extern OS_FLAG_GRP  *gRfiuStateFlagGrp;
   extern OS_FLAG_GRP  *gRf868EventFlagGrp;

   extern unsigned int gRfiu_Op_Sta[MAX_RFIU_UNIT];
   extern DEF_RFIU_RXCMD_STATUS rfiuCmdRxStatus[16];
   extern u32 rfiuAudioRetStatus;
   extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];

   extern u8 rfiuVoxEna[MAX_RFIU_UNIT];
   extern u8 rfiuVoxThresh[MAX_RFIU_UNIT];

   extern unsigned int gRfiu_RX_Sta[MAX_RFIU_UNIT];
   extern unsigned int         gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
   extern unsigned int         gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];
   extern OS_EVENT            *gRfiuSWReadySemEvt;

   extern int rfiuRSSI_CALDiff[MAX_RFIU_UNIT];
   extern s8 ispTxFWFileName[MAX_RFIU_UNIT][32];

#if RFIU_RX_WAKEUP_TX_SCHEME
   extern u32 rfiuBatCam_PIRRecDurationTime;
#endif

#if MESUARE_BTCWAKEUP_TIME
   extern u32 rfiuBTCWakeTime[MAX_RFIU_UNIT];
#endif
   extern int rfiuBatCamBattLev;

   extern u32 rfiuTxSyncTotolTime;
   extern u32 rfiuTxLifeTotolTime;

   //============================================================//
#if TX_FW_UPDATE_SUPPORT   
   u32 RXGetTXBorad(unsigned char RFUnit);
   u32 RXGetTXPROJ(unsigned char RFUnit);
   u32 RXGetTXFWTime(unsigned char RFUnit);
   int rfiuTxFwUpdateFromSD(int RFUnit);
   int rfiuTxFwUpdateFromNet(int RFUnit);
   int rfiuTxFwUpdateFromUSB(int RFUnit);
   int rfiuTxFWUPD_AllTX(int SourceSel);
   extern int rfiuFwUpdLoadTxFW_SD(int RFUnit);
   extern u8 *rfiuGetTxUpgradeBuf(void);
   extern char *rfiuGetTxVersionName(int RFUnit);
   extern int rfiuGetTxUpgradePercentage(int RFUnit);
#endif
   extern int rfiu_RSSI_Calibration(int RFUnit);

   extern int rfiuCheckTX_IDSame(int RFUnit);
   extern int rfiuClearRFSyncWord(int RFUnit);
   extern int rfiuCheckRFUnpair(int RFUnit);
 
   extern s32 RfiuInit(void);
   extern s32 RfiuReset(int RFIunit);
   extern void rfiu_Start(void);
   extern void rfiu_End(void);
   extern void rfiu_PAIR_Linit(int RFUnit);  
   extern void rfiu_PAIR_Stop(int RFUnit);
   extern unsigned int rfiu_GetBitRate(int RFUnit);
   extern unsigned int rfiu_TxGetCamNum(int RFUnit);


   extern int rfiu_RXCMD_Enc(u8 *cmd,int RFUnit); 
   extern int rfiu_TXCMD_Enc(u8 *cmd,int RFUnit);
   extern int rfiu_TXCMD_Dec (int RFUnit);


   extern void rfiu_RXCMD_Dec (char CmdType,int RFUnit,int *pTX_CHG_CHflag);
   extern int rfiuCalVideoDisplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
   extern int rfiu_CheckTxTaskMode(int RFUnit);
   extern void rfiu_Tx_Task_UnitX(void* pData);
   extern int rfiu_SetRXOpMode_All(u32 mode,u32 level);
   extern s32 rfiu_SetRXOpMode_1(s32 RFUnit);
   extern s32 rfiu_SetTXTurbo_On(s32 RFUnit);
   extern s32 rfiu_SetTXTurbo_Off(s32 RFUnit);
   extern s32 rfiu_SetTXReboot(s32 RFUnit);
   extern s32 rfiu_SetTXPIRCfg(s32 RFUnit);

   extern s32 rfiu_ResendTxMdConfig(s32 RFUnit);
   extern s32 rfiu_SendTxMdSense(s32 RFUnit);

   extern int rfiu_SetTXPWM(int RFUnit,u8 val);
   extern int rfiu_SetTXMotorCtrl(int RFUnit,u8 val);
   extern int rfiu_SetTXMelodyNum(int RFUnit,u8 val);
   extern s32 rfiu_SetTXVoxCfg(s32 RFUnit);

   extern int rfiu_SetTXVoxTrig(u8 vol);
   extern int rfiu_SetRXLightTrig(u8 val);


   extern  int rfiuFCCTX0Cmd(u8 *cmd);
   extern  int rfiuFCCTX1Cmd(u8 *cmd);

   extern  int rfiuFCCTX0Cmd2(u8 *cmd);
   extern  int rfiuFCCTX1Cmd2(u8 *cmd);

   extern s32 A7130_WOR_enable_B1(s32 PIR_IntvOn);
   extern void A7130_WOR_enable_B2(void);

   extern s32 A7196_WOR_enable_B1(s32 PIR_IntvOn);
   extern void A7196_WOR_enable_B2(void);

   extern s32 RFNONE_WOR_enable_B1(s32 dummy);
   extern void RFNONE_WOR_enable_B2(void);


   extern  int rfiuFCCRX0Cmd(u8 *cmd);
   extern  int rfiuFCCRX1Cmd(u8 *cmd);

   extern  int rfiuFCCUnitSel(u8 *cmd);
   extern  int rfiuRFModule_RW(u8 *cmd);

   extern void rfiu_InitCamOnOff(u32 Status);
   extern void rfiuCamOnOffCmd(u8 *cmd);
   extern int rfiuCamSleepCmd(int RFUnit);

   extern void A7130_WriteReg_B1(BYTE addr, BYTE dataByte);
   
   extern void A7196_WriteReg_B1(BYTE addr, BYTE dataByte);
   extern void A7196_WriteReg_B2(BYTE addr, BYTE dataByte);

   extern u8 A7196_ReadReg_B1(u8 addr);
   extern u8 A7196_ReadReg_B2(u8 addr);

   extern int rfiu_AudioRetONOFF_IIS(int OnOff);
   extern int rfiu_AudioRetONOFF_APP(int OnOff,int RFUnit);
   
   extern s32 rfiuSetGPO_TX(s32 setting);
   extern s32 rfiuSetPWM_TX(s32 setting);
   extern s32 rfiuSetMotorCtrl_TX(s32 setting);
   extern s32 rfiuSetMelodyNum_TX(s32 setting);
   extern s32 rfiuSetVoxTrig_RX(s32 RFUnit);
   extern s32 rfiuSetLightStat_RX(s32 RFUnit);


   extern void rfiuIntHandler(void);
   extern u8 rfiu_RfSwAudio_DualMode(void);
   extern s32 rfiuForceResync(s32 RFUnit);

   extern int rfiuRWAmicReg1(u8 *cmd);
   extern int rfiuRWAmicReg2(u8 *cmd);

   extern int rfiuCamFeedBackAPPStaCmd(int RFUnit,int par1,int par2);

   extern s32 rfiu_UpdateTXOthersPara(s32 RFUnit);
   #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
   extern int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6);
   #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
   extern int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6, u8 par7, u8 par8);
   #endif

   extern s32 rfiu_SetTXDoorBellOff(s32 RFUnit);
#if HOME_RF_SUPPORT


    #define HOMERF_NAME_MAX            16
//    #define HOMERF_SENSOR_MAX         128
    #define HOMERF_SENSOR_MAX         20
    #define HOMERF_MULTI_SENSOR_MAX     5
    #define HOMERF_ROOM_MAX            10
    #define HOMERF_SCENE_MAX           10
    
    typedef struct _SENSOR_DOOR
    {
        bool isOpen;  /* True: Open, False: close */
    }SENSOR_DOOR;

    typedef struct _SENSOR_IAQ	//Indoor Air Quality
    {
        u16  value;
        u16  high;			//TBD
        u16  low;			//TBD
    }SENSOR_IAQ;

    typedef struct _SENSOR_ADE
    {
        bool isPowerOn;
        bool isSupportGal;
        u32  wattage;
        u32  voltage;
        u32  current; 
    }SENSOR_ADE;
	
    typedef struct _SENSOR_SWITCH
    {
        u8 isPowerOn;
        bool isSupportGal;
        u32  wattage;
        u32  voltage;
        u32  current; 
    }SENSOR_SWITCH;

    typedef struct _SENSOR_FDS
    {
		u8 Accel_X_H;
		u8 Accel_X_L;
		u8 Accel_Y_H;
		u8 Accel_Y_L;
		u8 Accel_Z_H;
		u8 Accel_Z_L;
		u8 GYRO_X_H;
		u8 GYRO_X_L;
		u8 GYRO_Y_H;
		u8 GYRO_Y_L;
		u8 GYRO_Z_H;
		u8 GYRO_Z_L;
		u8 RSSI;
    }SENSOR_FDS;

    typedef struct _SENSOR_TEMP_HYG
    {
        u8  Thermometer_H;
        u8  Thermometer_L;
        u8  Hygrometer_H;
        u8 	Hygrometer_L;
        u8	Temp_alarm_H;
        u8	Temp_alarm_L;
        u8	Humi_alarm_H;
        u8	Humi_alarm_L;
    }SENSOR_TEMP_HYG;

    typedef struct _SENSOR_TEMP
    {
        u16  value;       /* 0-7: decemal, 8-15: integer */
        u16  high;
        u16  low;
        u8 alarmSwitch;   /* 0: all off; bit 0:高溫警報; bit 1:低溫警報 */
    }SENSOR_TEMP;

    typedef struct _SENSOR_HUMIDTY
    {
        u16 value; 
    }SENSOR_HUMIDTY;

    typedef struct _SENSOR_SIREN
    {
        bool isRinging;
    }SENSOR_SIREN;

    typedef struct _SENSOR_PLUG
    {
        bool isPowerOn;
        bool isSupportGal;
        u32  wattage;
        u32  voltage;
        u32  current; 
    }SENSOR_PLUG;

    typedef struct _SENSOR_MULTI_TYPE
    {
        u32 SID[HOMERF_MULTI_SENSOR_MAX];
    }SENSOR_MULTI_TYPE;


    typedef struct _HOMERF_SENSOR_DATA
    {

        u8  name[HOMERF_NAME_MAX];
        u8  battery;
        u8  isHealthSensor;
        u8  pushOnOff;
        u8  sirenOnOff;
        u8  status;
        u8  maxSubSensor;  /* 0,1:    none Multi sensor ,
                              others: multi sensor count;                                  
                           */
        u16 alarmTimer; //For Tranwo 10 mins count, add by Paul@20190401
        u8  majorVer;
        u8  minorVer;
        s16 lifeCount;
        u16 type;
        u32 sID;
        u64 sUID;
        u8  lowbatterycnt;
        
        union
        {
             SENSOR_DOOR door;
             SENSOR_TEMP Temp;
             SENSOR_PLUG plug;
             SENSOR_HUMIDTY  humidty;
             SENSOR_SIREN   siren;
             SENSOR_MULTI_TYPE  MSdata;
			 SENSOR_IAQ IAQ;
			 SENSOR_ADE ADE;
			 SENSOR_FDS FDS;
			 SENSOR_SWITCH SWITCH;
			 SENSOR_TEMP_HYG Temp_HYG;
        }data;
        u32	byteSameOldID;  //Sean: 20170612 Add.
        u32 reserve[37];
        //u32 reserve[46];

         
    }HOMERF_SENSOR_DATA; 



    typedef struct _HOMERF_ROOM_DATA
    {
        u32 roomID;
        u8  roomNmae[HOMERF_NAME_MAX];
        u8  scount;
        u32 sID[HOMERF_SENSOR_MAX];
    }HOMERF_ROOM_DATA;

    typedef struct _HOMERF_SCENE_SENSOR
    {        
        u8  isAlarm;
        u32 sID;
        
    }HOMERF_SCENE_SENSOR;


    typedef struct _HOMERF_SCENE_DATA
    {
        u8  sceneName[HOMERF_NAME_MAX];
        u8  isAlarm[HOMERF_SENSOR_MAX];
        u32 sID[HOMERF_SENSOR_MAX];    
        u32 sceneID;
        u32 totalCnt;

    }HOMERF_SCENE_DATA;



    typedef struct _HOMERF_SENSOR_LIST
    {
        HOMERF_SENSOR_DATA sSensor[HOMERF_SENSOR_MAX];
        
    }HOMERF_SENSOR_LIST;


    typedef struct _HOMERF_ROOM_LIST
    {
        HOMERF_ROOM_DATA sRoom[HOMERF_ROOM_MAX];
        
    }HOMERF_ROOM_LIST;


    typedef struct _HOMERF_SCENE_LIST
    {
        HOMERF_SCENE_DATA sScene[HOMERF_SCENE_MAX];
        
    }HOMERF_SCENE_LIST;





    enum
    {
        HOMERF_SENSOR_OFF=0,
        HOMERF_SENSOR_ON,
    };

    enum
    {
        HOMERF_SEND_CMD_FAIL=0,
        HOMERF_SEND_CMD_OK
    };

    
    enum
    {
        HOMERF_OP_NOMAL=0,
        HOMERF_OP_PAIR,
        HOMERF_OP_APP_PAIR,
        
    };
    
    /*status flag */
    #define HOMERF_FLAG_SEND_CMD_SUC    0x00000001
    #define HOMERF_FLAG_SEND_CMD_FAIL   0x00000002
    #define HOMERF_FLAG_PAIR_SUC        0x00000004
    #define HOMERF_FLAG_PAIR_FAIL       0x00000008
    #define HOMERF_FLAG_NEW_FW          0x00000010
    #define HOMERF_FLAG_OLD_FW          0x00000020
    #define HOMERF_FLAG_UPDATE_FAIL     0x00000040
    #define HOMERF_FLAG_UPDATE_SUC      0x00000080
    
    enum
    {
        HOMERF_SEND_LOCK=0,
        HOMERF_SEND_UNLOCK,
        HOMERF_SEND_PAIR,
        HOMERF_SEND_PLUG_ON,
        HOMERF_SEND_PLUG_OFF,
 /* 5*/ HOMERF_SEND_SIREN_ON,
        HOMERF_SEND_SIREN_OFF,
        HOMERF_SEND_GET_HOST,      /* get Host Address*/
        HOMERF_SEND_SEARCH_ALL,    /* search all sensor node */
        HOMERF_SEND_DELETE,        /* delete one sensornode */
 /*10*/ HOMERF_SEND_DELETE_ALL,    /*MARS1.0: delete all 結束後 會自動產生新的MAC ID */
                                       /*必須重新下HOMERF_SEND_SET_RFID 設定回原本的 否則下次開機會配不上*/
        HOMERF_SEND_RF_VERSON,     /* Get RF module vesion num*/
        HOMERF_SEND_APP_PAIR,
        HOMERF_SEND_QUERY_STATUS,  /* Query one sensor status*/
        HOMERF_SEND_QUERY_ALL,     /* Query all pair sensor*/
 /*15*/ HOMERF_SEND_SET_RFID,
        HOMERF_SEND_UPDATE_FW_AVAILABLE,
        HOMERF_SEND_SIREN_BEEP,
        HOMERF_SEND_SWITCH_ON,
        HOMERF_SEND_SWITCH_OFF,        
    };

    
    enum 
    {
    /* don't adjust sequence, it might cause UI image file error */
        HOMERF_DEVICE_DOOR=0,
        HOMERF_DEVICE_PIR,
        HOMERF_DEVICE_SIREN,
        HOMERF_DEVICE_IR,
        HOMERF_DEVICE_PLUG,
        HOMERF_DEVICE_TEMP_HUM,   /* temperature & humidity*/
        HOMERF_DEVICE_ROUTER,   
        HOMERF_DEVICE_VIBRATE,
        HOMERF_DEVICE_LEAK,
        HOMERF_DEVICE_SMOKE,
        HOMERF_DEVICE_GAS,
        HOMERF_DEVICE_TEMP,
        HOMERF_DEVICE_HUM,
        HOMERF_DEVICE_PANIC,
        HOMERF_DEVICE_IAQ,
        HOMERF_DEVICE_ADE,
        HOMERF_DEVICE_FDS,
        HOMERF_DEVICE_SWITCH,
        HOMERF_DEVICE_TEMP_HYG,
    };


    enum
    {
        HOMERF_SIREN_OFF=0,
        HOMERF_SIREN_ON,
        HOMERF_SIREN_BEEP,
    };


    enum
    {
        HOMERF_EVENT_NONE=0,
        HOMERF_EVENT_TRIGGER,
        HOMERF_EVENT_HEART,
        HOMERF_EVENT_PANIC,
        HOMERF_EVENT_ALARM,
        HOMERF_EVENT_DISARM,
    };

    enum
    {
        HOMERF_SYS_SCENE_ON=0,
        HOMERF_SYS_SCENE_OFF,
        HOMERF_SYS_SCENE_HOME,
    };

    #define HOMERF_SENSOR_STATUS_NORMAL  0x0
    #define HOMERF_SENSOR_STATUS_ALARM   0x00000001
    #define HOMERF_SENSOR_STATUS_HEART   0x00000002
    #define HOMERF_SENSOR_STATUS_FAIL    0x00000004
    #define HOMERF_SENSOR_STATUS_SOS     0x00000008
    #define HOMERF_SENSOR_STATUS_LOCK    0x00000010
    #define HOMERF_SENSOR_STATUS_UNLOCK  0x00000020
    #define HOMERF_SENSOR_STATUS_TEMP    0x00000040
    #define HOMERF_SENSOR_STATUS_LOWBAT  0x00000080
    #define HOMERF_SENSOR_STATUS_RESEND  0x00000100
    
    extern OS_FLAG_GRP  *gHomeRFEventFlagGrp;
    extern OS_FLAG_GRP  *gHomeRFStatusFlagGrp;
    extern u8 gHomeRFOpMode;
    extern u8 gHomeRFSensorCnt;
    extern u8 homeRFCmdParse(u8 * cmd);
    extern u8 homeRFGetUartCmd(u8 *pcString);
    extern u8 homeRFSendToSensor(u8 cmd_type, u8 sensor);
    extern u8 homeRFDeleteSensor(u8 idx);
    extern void homeRFDeleteRoom(u8 idx);
    extern void homeRFDeleteScene(u8 idx);
    extern u32 gHomeRFSensorID;
    extern void homeRFRunPerSec(void);
    extern u8 homeRFGetNewFWAvailable(u8 *srcAddr, u32 fileSize);
    
    extern HOMERF_SENSOR_LIST  *gHomeRFSensorList;
    extern HOMERF_ROOM_LIST    *gHomeRFRoomList;
    extern HOMERF_SCENE_LIST   *gHomeRFSceneList;
    extern u8  gHomeRFCheckAlert[16];
    extern u32 gHomeRFVersion;


    #define HOMERF_CMD_TEST           0


#endif

#if DOOR_BELL_SUPPORT
    extern OS_FLAG_GRP *DoorBellFlagGrp;
    #define DOORBELL_PAIR_FLAG    0x00000001
#endif


 #if RFIU_TEST
   extern     int marsRfiu_Test();
 #endif
 
 extern int gRfiuTxFwUpdPercent[MAX_RFIU_UNIT];

#endif

