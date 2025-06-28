#ifndef __RFIU_API_H__
#define __RFIU_API_H__

#if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) || (HW_BOARD_OPTION == MR8100_GCT_LCD) \
    || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)|| (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
#define DETECT_RX_VOLUME           1
#else
#define DETECT_RX_VOLUME           0
#endif

    #define RFI_PAYLOAD_SIZE                   128                      // 1 payload = 128 bytes
    #define RFI_GRP_INPKTUNIT                  64
    #define RFI_BUF_SIZE_GRPUNIT               128                      //buf size = N group-unit ,group unit: 2^N
    #define RFI_TOTAL_BUF_SIZE                 (RFI_GRP_INPKTUNIT*RFI_BUF_SIZE_GRPUNIT*RFI_PAYLOAD_SIZE)
 
    #define RFI_AUIDIO_SILENCE_SIZE             1024
 
    #define RFI_VIDEO_SYNC_SHIFT                33
 
    //------//
    #define FCC_DIRECT_TX_MODE                  0
    #define FCC_DIRECT_RX_MODE                  1
   //----------------------------------RFIU_TEST must be 1 or 2 ----------------------------// 
#if RFIU_TEST   
    //===========工程模式==========//
    #define RFI_MEASURE_RX1RX2_SENSITIVITY 0
 
    #define RFI_FCC_DIRECT_TRX             1    // 1: for test 1st RF, 2: for test 1st and 2nd RF
 
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
    #define RFRXCMD_SET_TIMESTAMP         11
    #define RFRXCMD_SET_MASKAREA_VGA      12
    #define RFRXCMD_SET_MASKAREA_HD       13
    #define RFRXCMD_SET_SLEEP             14
    #define RFRXCMD_SET_EXTEND            15
    //Max to 15
 
    #define RFRXCMD_SETEXT_PWRTURBO       0
    #define RFRXCMD_SETEXT_DOORBELLOFF    1
    #define RFRXCMD_SETEXT_MDSENSTAB      2
    #define RFRXCMD_SETEXT_CHGCH          3  //used in FCC247 protocol
    #define RFRXCMD_SETEXT_PWM            4
    #define RFRXCMD_SETEXT_MOTORCTL       5
    #define RFRXCMD_SETEXT_MELODYNUM      6
    #define RFRXCMD_SETEXT_VOXCFG         7
    #define RFRXCMD_SETEXT_SCHED          8
    #define RFRXCMD_SETEXT_SATURATION     9
    #define RFRXCMD_SETEXT_VOLUME         10
    #define RFRXCMD_SETEXT_LIGHTONOFF     11
    #define RFRXCMD_SETEXT_SNAPSHOT       13 // TX snap-shot
    #define RFRXCMD_SETEXT_MUSICCTL       14
	#define RFRXCMD_SETEXT_PHOTOTIME      15
	#define RFRXCMD_SETEXT_VOCTEST        16
    #define RFRXCMD_SETEXT_PUSHAPPMSG     17
    #define RFRXCMD_SETEXT_FBAPPSTA       18

    //----TX Information----//
    #define RFTXINFO_SET_VOXTRIG          0
    #define RFTXINFO_SET_LIGHT_STA        1
    #define RFTXINFO_SET_BODYINFO         2
    
 
    //------TX Command Mask--------//
    #define RFTXCMD_SEND_NONE            0x0000
    #define RFTXCMD_SEND_DATA            0x0001
    #define RFTXCMD_SEND_AUDIORETMAP     0x0002
    #define RFTXCMD_SEND_TVTYPECHG       0x0004
   
    //---------TX Status via Sync--------//
    #define RFIU_TX_STA__FIELD_ENC       0x00000001
    #define RFIU_TX_STA__PAL_MODE        0x00000002
    #define RFIU_TX_STA__MD_ON           0x00000004
    #define RFIU_TX_STA__PIR_ON          0x00000008
  
    #define RFIU_TX_STA__HD_SUPPORT      0x00000010
    
    #define RFIU_TX_STA_TIMESTAMP_ON     0x00000020
    #define RFIU_TX_STAT_BELL_ON         0x00000020
    
    #define RFIU_TX_STA_MPEG_Q           0x00000040
    #define RFIU_TX_STA_FLICKER_50HZ     0x00000080
  
    #define RFIU_TX_STA__FULLHD_SUPPORT  0x00000100
    #define RFIU_TX_STA_NEWMD_SUPPORT    0x00000200
    #define RFIU_TX_STAT_PROTCOL_SEL     0x00000c00
  
    #define RFIU_TX_STA_OPMODE           0x0000f000  //bit12~15 for TX Op mode
    #define RFIU_TX_STA_CAMNUM           0x000f0000  //bit 16~19  for Camera num of conecting to RX
    #define RFIU_TX_STA_BRIGHNESS        0x00f00000  //bit 23~20 for Camera brighness value
    #define RFIU_TX_STA_MD_SENS          0xff000000  //bit 31~24 for Camera MD sensitivity

    //bit 12~15 reserved for rfiuRX_OpMode
 
    //------------TX Status2 via Sync-----------//
    #define RFIU_TX_STAT_WIFI_ON         0x00000002  //used for MR8211 TX,MR8100 RX
    #define RFIU_TX_STAT_LIGHT_ON        0x00000004
    
    //-----------------------//
    #define RFIU_TX_MODE        0
    #define RFIU_RX_MODE        1
    #define RFIU_IDLE_MODE      2
    #define RFIU_SYNC_MODE      3
    #define RFIU_PAIR_MODE      4
    #define RFIU_PAIRLint_MODE  5
    #define RFIU_TX_CMD_MODE    6
    #define RFIU_PAIR_STOP_MODE 7
 
    //-------------RX Operation Mode--------------//
    #define RFIU_RX_OPMODE_NORMAL    0x00000000
    #define RFIU_RX_OPMODE_QUAD      0x00000001
    #define RFIU_RX_OPMODE_P2P       0x00000002
    #define RFIU_RX_OPMODE_P2P_MULTI 0x00000004

    #define RFIU_RX_OPMODE_BABM_HD   0x00000010
 
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
    #define RFIU_RX_STA_CHGRESO      0x10
    #define RFIU_RX_STA_FRAMESYNC    0x20

    #define RFIU_RX_STA_PAIR_STOP    0x40

	#define RFIU_RX_STA_KEEP         0x80 
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
   
 
    #define RFI_FEC_TYPE_MAX          4
 
#if ((HW_BOARD_OPTION == MR8100_RX_RDI_M512) && (UI_PROJ_OPT == 0))
    #define RFRX_HALF_MODE_SHIFT      240    //Panel use
#else
    #define RFRX_HALF_MODE_SHIFT      120    //Panel use
#endif
    #define RFRX_HALF_MODE_SHIFT_TV   240    //TV use

  

    //---------TX Sensor-------------// 
    #define TX_SENSORTYPE_VGA           0
    #define TX_SENSORTYPE_HD            1

    //-----For audio return-----//
    #define RF_AUDIO_RET_OFF            0
    #define RF_AUDIO_RET_RX_USE         1
    #define RF_AUDIO_RET_APP_USE        2

    //======================//
    #define  RF_P2PVdoQalt_HD_5_FPS      1
    #define  RF_P2PVdoQalt_VGA_15_FPS    2
    #define  RF_P2PVdoQalt_QVGA_15_FPS   3
    #define  RF_P2PVdoQalt_QVGA_10_FPS   4
    #define  RF_P2PVdoQalt_QVGA_7_FPS    5

    #define  RF_P2PVdoQalt_VGA_10_FPS    6
    #define  RF_P2PVdoQalt_VGA_30_FPS    7 

    #define  RF_P2PVdoQalt_HD_7_FPS      8 
    #define  RF_P2PVdoQalt_HD_15_FPS     9   // not support now.
    //-----------------------------------//
    #define  RFWIFI_P2P_QUALITY_HIGH     0
    #define  RFWIFI_P2P_QUALITY_MEDI     1
    #define  RFWIFI_P2P_QUALITY_LOW      2
  
    //---------TX Snapshot status--------//
	#define RF_TXPHOTOSTA_DONE_OK		 0
	#define RF_TXPHOTOSTA_NONE			 1
    //-----------------------------------//
    typedef struct _RFIU_TXSNAPSHOTCHK
    {
        int SnapTimeInMin;
        int ReCheck;
        int CheckCnt;
        int SnapStatus;        
    }DEF_RFIU_TXSNAPSHOTCHK;

    
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
 
        //---TX Sensor Setting---//
        int TxSensorAE_FlickSetting;
        int TX_SensorType;
        int TX_SensorBrightness;
 
        //---Others---//
        int TX_TimeStampOn;
        int TX_TurboOn;
        int RX_TurboOn;
        int RX_DoorTrig;
 
        //----Vox---//
        int VoxEna;
        int VoxThresh;
        u32 VoxTrigFlag;
        u32 VoxTrigLev;
        
        //====TX Code Version====//
        char TxCodeVersion[32];

        //---ID----//
        u32 RF_ID;

        //---AV sync Time---//
    #if RF_AV_SYNCTIME_EN 
        u32 VideoTotalTime;
        u32 AudioTotalTime;
    #endif
        //-------------//
        u32 WifiLinkOn;
        u32 WifiLinkCH;
        u32 NightLight;
        int Temperature;
        int Humidity;
        int PM25;
    }DEF_RFIU_UNIT_PARA;

    typedef struct _RFIU_UNIT_CNTL
    {
        unsigned int OpMode;
        unsigned int OpState;
        unsigned int BufReadPtr;
        unsigned int BufWritePtr;
        unsigned int WritePtr_Divs;
        unsigned int RunCount;
#if RFIU_TX_WAKEUP_SCHEME
        unsigned int WakeUpTxEn;
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
        
        //----Command----//
        unsigned char RX_CMD_Data[12];
        unsigned int  RX_PASSWORD[RFIU_PASSWORD_MAX];  
        unsigned char RXCmd_Type;     
        unsigned char RXCmd_en; 
        unsigned char RXCmd_Busy;   
        unsigned char RXCmd_AudioRetEn;
        //---//
        unsigned char TXCmd_en; 
        unsigned char TXCmd_Type;
        int  TX_CMDPara[RFIU_TXCMDDATA_MAX];
 
        DEF_RFIU_GROUP_MAP AudioRetPktMap[RFI_AUDIO_RET_GRPNUM]; //
 
        //----Protocol selection----//
        unsigned int ProtocolSel;
        unsigned int TXChgCHNext;
 
        unsigned int VMDSel;
        //===FCC===//
        unsigned int FCC_TestMode;
        unsigned int FCC_TX_RealData_ON;
        unsigned int FCC_TX_NoData_Zero;
        unsigned int FCC_TX_Freq;
        unsigned int FCC_RX_Freq;
 
        DEF_RFIU_UNIT_PARA RFpara;  //Lucian: 必須放在最後面.
    }DEF_RFIU_UNIT_CNTL;

    typedef struct _RFIU_RXCMD_STATUS
    {
        int CamNum;
        int EnterScanMode;
    }DEF_RFIU_RXCMD_STATUS;

#if RX_SNAPSHOT_SUPPORT
    typedef struct _DATA_BUF_MNG
    {
    	u32	size;
    	u8* buffer;	
    } DATA_BUF_MNG;
#endif

    //=========Extern Variable =========//
#if TX_SNAPSHOT_SUPPORT 
	extern DEF_RFIU_TXSNAPSHOTCHK rfiuTXSnapCheck;
#endif
    
    extern OS_FLAG_GRP  *gRfiuStateFlagGrp;
    extern OS_FLAG_GRP  *gRf868EventFlagGrp;

    extern unsigned int gRfiu_Op_Sta[MAX_RFIU_UNIT];
    extern DEF_RFIU_RXCMD_STATUS rfiuCmdRxStatus[16];
    extern u32 rfiuAudioRetStatus;
    extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];

    extern u8 rfiuVoxEna[MAX_RF_DEVICE];
    extern u8 rfiuVoxThresh[MAX_RF_DEVICE];
    extern u8 rfAudioEnable;
#if DETECT_RX_VOLUME
    extern u32 RxVolume[MAX_RFIU_UNIT];
#endif

    //============================================================//
    extern  int rfiuCheckTX_IDSame(int RFUnit);
    extern int rfiuClearRFSyncWord(int RFUnit);
    extern int rfiuCheckRFUnpair(int RFUnit);

    extern int rfiuPutTXTemHumPM(u32 Tem,u32 Hum,u32 PM25,int RFUnit);
    extern int rfiuPutTXTem(u32 Tem,int RFUnit);
    extern int rfiuPutTXHum(u32 Hum,int RFUnit);
    extern int rfiuPutTXPM25(u32 PM25,int RFUnit);

    extern int rfiuGetRXTemHumPM(u32 *pTem,u32 *pHum,u32 *pPM25,int RFUnit);
    extern int rfiuGetRXTem(u32 *pTem,int RFUnit);
    extern int rfiuGetRXHum(u32 *pHum,int RFUnit);
    extern int rfiuGetRXPM25(u32 *pPM25,int RFUnit);

    extern int rfiuGetRXWifiStatus(int RFUnit);

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
    extern void rfiu_StopRXMpegDec(int RFUnit);
    extern void rfiu_Tx_Task_UnitX(void* pData);
    extern int rfiu_SetRXOpMode_All(u32 mode,u32 level);
    extern int rfiu_SetRXOpMode_1(int RFUnit);
    extern int rfiu_SetTXTurbo_On(int RFUnit);
    extern int rfiu_SetTXTurbo_Off(int RFUnit);
    extern int rfiu_ForceTXSetSameResolution(int RFUnit);

    extern int rfiu_ResendTxMdConfig(int RFUnit);
    extern int rfiu_SendTxMdSense(int RFUnit);

    extern int rfiu_SetTXPWM(int RFUnit,u8 val);
    extern int rfiu_SetTXMotorCtrl(int RFUnit,u8 val);
    extern int rfiu_SetTXMelodyNum(int RFUnit,u8 val);
    extern int rfiu_SetTXVoxCfg(int RFUnit);

    extern int rfiu_SetRXVoxTrig(u8 vol);

    extern  int rfiuFCCTX0Cmd(u8 *cmd);
    extern  int rfiuFCCTX1Cmd(u8 *cmd);

    extern  int rfiuFCCTX0Cmd2(u8 *cmd);
    extern  int rfiuFCCTX1Cmd2(u8 *cmd);

    extern void A7130_WOR_enable_B1(void);
    extern void A7130_WOR_enable_B2(void);

    extern  int rfiuFCCRX0Cmd(u8 *cmd);
    extern  int rfiuFCCRX1Cmd(u8 *cmd);

    extern  int rfiuFCCUnitSel(u8 *cmd);

    extern void rfiu_ChangeDispSrcWidthHeight(int Width,int Height);
    extern void rfiu_ChangeDispSrcStride(int Stride);
    extern void rfiu_InitCamOnOff(u32 Status);
    extern void rfiuCamOnOffCmd(u8 *cmd);
    extern int rfiuCamSleepCmd(u8 *cmd);

    extern void A7130_WriteReg_B1(BYTE addr, BYTE dataByte);

    extern void A7196_WriteReg_B1(BYTE addr, BYTE dataByte);
    extern void A7196_WriteReg_B2(BYTE addr, BYTE dataByte);

    extern u8 A7196_ReadReg_B1(u8 addr);
    extern u8 A7196_ReadReg_B2(u8 addr);

    extern int rfiu_AudioRetONOFF_IIS(int OnOff);
    extern int rfiu_AudioRetONOFF_APP(int OnOff,int RFUnit);
    extern unsigned int rfiu_GetTxFrameRate(int RFUnit);
    extern unsigned int rfiu_GetTxBitRate(int RFUnit);   
    extern char* rfiu_GetTxCodeVersion(int RFUnit);
    extern unsigned int rfiu_GetTxFlickerSet(int RFUnit);
    extern unsigned int rfiu_GetTxBrightness(int RFUnit);

    extern int rfiuSetGPO_TX(u8 setting);
    extern int rfiuSetPWM_TX(u8 setting);
    extern int rfiuSetMotorCtrl_TX(u8 setting);
    extern int rfiuSetMelodyNum_TX(u8 setting);
    extern int rfiuSetVoxTrig_RX(int RFUnit);
    extern int rfiuSetLightStat_RX(int RFUnit);

    

    extern void rfiuIntHandler(void);
    extern u8 rfiu_RfSwAudio_DualMode(void);
    extern int rfiuForceResync(int RFUnit);

    extern int rfiuRWAmicReg1(u8 *cmd);
    extern int rfiuRWAmicReg2(u8 *cmd);
    extern int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6);
    extern int rfiuCamFeedBackAPPStaCmd(int RFUnit,int par1,int par2);


    #if ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    extern int rfiu_SetTXVocDustTestMode(int RFUnit,u8 val);
    #endif
    
#if HOME_RF_SUPPORT  


    #define HOMERF_NAME_MAX            16
    #define HOMERF_SENSOR_MAX         128
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
        u8  alarmTimer;
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

#if RFIU_TEST
    extern     int marsRfiu_Test();
#endif

#if DOOR_BELL_SUPPORT
extern OS_FLAG_GRP *DoorBellFlagGrp;
#define DOORBELL_PAIR_FLAG    0x00000001
#endif


#if(BLE_SUPPORT)

#define BLE_MAC_ADDR_MAX        7 //MAC addr 6 byte + '\0'
#define BLE_LIST_MAX            8

extern u8 g_BLEsyncTimeError;
extern u8 g_BLEmoduleDetCount;

enum
{
    BLE_DEV_UNPAIRED,
    BLE_DEV_PAIRED,
};

typedef enum
{
    BLE_OFF,
    BLE_ON, //for UI used
//    BLE_STANDBY,
    BLE_ADVERTISING,
//    BLE_INITIATING,
    BLE_CONNECTED,
    BLE_NULL,
}BLE_WORKING_STATE;

typedef struct _BLE_CANDITATE_DEV
{
    u8 devID;
    u8  devName[BLE_MAC_ADDR_MAX];
    u8  status;
}BLE_CANDITATE_DEV;
    
typedef struct _BLE_SCAN_LIST
{
    BLE_CANDITATE_DEV sDev[BLE_LIST_MAX];
}BLE_SCAN_LIST;

enum
{
    BLE_VOX_ON = 254,
    BLE_VOX_OFF = 255,
};

//for ui
u8 BLE_Switch(BLE_WORKING_STATE ble_status);
u8 BLE_ScanList(BLE_SCAN_LIST *sScanList);
u8 BLE_ChooseToPair(BLE_SCAN_LIST *sScanList, u8 num);
u8 BLE_UnPair(u8 devID);
u8 BLE_VoxTriggerSetting(u8 num, u8 value);
u8 BLE_syncTime(void);
void BLE_VoxDurationCountdown(void);
void BLE_VoxDurationCountReset(u8 num, u8 u8OnOff);
u8 sysBack_BLE_SyncTime(u8 FuncType);

#endif

#endif

