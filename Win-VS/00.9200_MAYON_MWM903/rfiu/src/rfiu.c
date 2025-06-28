/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    rfiu.c

Abstract:

    The routines of FR Interface Unit.
    
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2010/10/21  Lucian Yuan  Create  
*/

#include "general.h"
#include "board.h"
#include "task.h"
#include "sysapi.h"
#include "gpioapi.h"
#include "timerapi.h"
#include "rtcapi.h"

#include "uiKey.h"

#include "rfiuapi.h"
#include "rfiureg.h"
#include "rfiu.h"
#include "uiapi.h"
#include "mpeg4api.h"
#include "i2capi.h"
#include "uartapi.h"
#include "GlobalVariable.h"
#include "siuapi.h"
#include "MotionDetect_API.h"
#include "ciuapi.h"
#include "adcapi.h"

#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif


#if TX_FW_UPDATE_SUPPORT
#include "fsapi.h"
#include "dcfapi.h"

#if LWIP2_SUPPORT
#include "encrptyapi.h"
#else
#include <../LwIP/netif/ppp/md5.h>
#endif
#endif

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */
 #if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    #define TX_LINKBROKEN_TIMEOUT    18000
 #else
    #define TX_LINKBROKEN_TIMEOUT    14000
 #endif

#if(RFIC_SEL==RFIC_A7196_6M)
#define RFI_RX_PACKET_NUM_INIT         192   //Lucian: 預設RX time-out (packet unit)
#elif(RFIC_SEL==RFIC_NONE_5M)
#define RFI_RX_PACKET_NUM_INIT         256   //Lucian: 預設RX time-out (packet unit)
#else
#define RFI_RX_PACKET_NUM_INIT         128   //Lucian: 預設RX time-out (packet unit)
#endif

#if (RFIU_USEPKTMAP_MAX > 8 )
  #if(RFIC_SEL==RFIC_A7196_6M)
    #if RFIU_DATA_6M_ACK_3M_SUPPORT
        #define RFI_ACK_SYNC_PKTNUM     4
    #elif RFIU_DATA_6M_ACK_4M_SUPPORT
        #define RFI_ACK_SYNC_PKTNUM     6
    #else
        #define RFI_ACK_SYNC_PKTNUM     8
    #endif
    #define RFI_ACK_PAIR_PKTNUM     8  
    #define RFI_ACK_WAKE_PKTNUM     128
  #elif(RFIC_SEL==RFIC_NONE_5M)
    #define RFI_ACK_SYNC_PKTNUM     6
    #define RFI_ACK_PAIR_PKTNUM     6  
    #define RFI_ACK_WAKE_PKTNUM     128    
  #elif(RFIC_SEL==RFIC_A7130_4M)  
    #if RFIU_DATA_4M_ACK_2M_SUPPORT
      #define RFI_ACK_SYNC_PKTNUM     4
    #else
      #define RFI_ACK_SYNC_PKTNUM     6
    #endif
    #define RFI_ACK_PAIR_PKTNUM     6  
    #define RFI_ACK_WAKE_PKTNUM     128
  #else
    #define RFI_ACK_SYNC_PKTNUM     6
    #define RFI_ACK_PAIR_PKTNUM     6  
    #define RFI_ACK_WAKE_PKTNUM     128
  #endif
#else
  #define RFI_ACK_SYNC_PKTNUM     4
  #define RFI_ACK_PAIR_PKTNUM     4   
  #define RFI_ACK_WAKE_PKTNUM     64  
#endif

 #if RFI_SELF_TEST_TXRX_PROTOCOL
     #define RFI_RX_WAIT_TIME   64
     #define RFI_TX_WAIT_TIME   64

 #elif(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 ||RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 ||\
       RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_8TX_1RX_PROTOCOL)
    #if(RFIC_SEL == RFIC_A7196_6M)
     #define RFI_RX_WAIT_TIME   48  
     #if RFIU_DATA_6M_ACK_3M_SUPPORT
     #define RFI_TX_WAIT_TIME   24
     #elif RFIU_DATA_6M_ACK_4M_SUPPORT
     #define RFI_TX_WAIT_TIME   32
     #else
     #define RFI_TX_WAIT_TIME   48
     #endif
    #elif(RFIC_SEL==RFIC_NONE_5M)
     #define RFI_RX_WAIT_TIME   32  //32
     #define RFI_TX_WAIT_TIME   32
    #elif(RFIC_SEL == RFIC_A7130_4M)
     #define RFI_RX_WAIT_TIME   32  //32
     #if RFIU_DATA_4M_ACK_2M_SUPPORT
     #define RFI_TX_WAIT_TIME   16  
     #else
     #define RFI_TX_WAIT_TIME   32  //32
     #endif
    #else
     #define RFI_RX_WAIT_TIME   32  //32
     #define RFI_TX_WAIT_TIME   32
    #endif
 #else
     #define RFI_RX_WAIT_TIME   128
     #define RFI_TX_WAIT_TIME   128
 #endif


#define RFI_PAIR_CUSTOMER_ID   0x1234  
#define RFI_PAIR_SYNCWORD      0xabcd1234


#define RXCMD_NONE_CHECK       0x0000
#define RXCMD_PROTOCOL_CHECK   0x0001
#define RXCMD_AUDIOMAP_CHECK   0x0002
//------------------//
#define RXACK_NORMAL           0x0000
#define RXACK_KEEP             0x0001
#define RXACK_FWUPD_START      0xffff0002
#define RXACK_FWUPD_DATA       0xffff0003
#define RXACK_FWUPD_DONE       0xffff0004


#define FCC_DIRECT_TX_CHSEL_LOW      0
#define FCC_DIRECT_TX_CHSEL_MIDDLE   1
#define FCC_DIRECT_TX_CHSEL_HIGH     2

#define CHECK_PKTMAP_ON        1
#define CHECK_PKTMAP_OFF       0

#define CHECK_PKT_BURSTNUM_ON  1
#define CHECK_PKT_BURSTNUM_OFF 0

#define RFDATA_6M_TO_3M_THR  6  //Unit: 100KBps
#define RFDATA_3M_TO_6M_THR  12

#define MAX_CH_SCAN  8


#define RFIU_TESTCNTMAX_WD   (1024*128/4)   //4

#if(RFIC_SEL == RFIC_A7196_6M)
   #define RX_ACK_RSSI_THR   180
#elif(RFIC_SEL==RFIC_NONE_5M)
   #define RX_ACK_RSSI_THR   180
#else
   #define RX_ACK_RSSI_THR   180
#endif

#if RFIU_RX_AUDIO_RETURN
   #define RX_AUDIORET_RETRYNUM      10
#endif

#define RF1_ACKTIMESHIFT      8000   //800 ms=8000*100 us

#define RF_FIXCH_OPTIM        1

#define RFKEEP_3M_TIME   10  // unit in sec. 連線後, 每 N 秒去試 6M的狀態.

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
u32 rfiuTxSyncTotolTime;
u32 rfiuTxLifeTotolTime;
 
u32 rfiuRSSI_CH_Avg[MAX_RFIU_UNIT][RFI_DAT_CH_MAX];
 
u8 rfiu_resetflag[MAX_RFIU_UNIT];
int rfiuRSSI_CALDiff[MAX_RFIU_UNIT];

OS_STK rfiuTaskStack_Unit0[RFIU_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuTaskStack_Unit1[RFIU_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuTaskStack_Unit2[RFIU_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuTaskStack_Unit3[RFIU_TASK_STACK_SIZE_UNIT3]; 
#if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL) 
OS_STK rfiuTaskStack_Unit4[RFIU_TASK_STACK_SIZE_UNIT4]; 
OS_STK rfiuTaskStack_Unit5[RFIU_TASK_STACK_SIZE_UNIT5]; 
OS_STK rfiuTaskStack_Unit6[RFIU_TASK_STACK_SIZE_UNIT6]; 
OS_STK rfiuTaskStack_Unit7[RFIU_TASK_STACK_SIZE_UNIT7]; 
#endif


OS_FLAG_GRP  *gRfiuFlagGrp;
OS_FLAG_GRP  *gRfiu_nTx1RSwFlagGrp;  //for n-Tx vs 1R 
OS_FLAG_GRP  *gRfiuStateFlagGrp;
OS_FLAG_GRP  *gRf868EventFlagGrp;

OS_EVENT    *gRfiuSWReadySemEvt;

OS_EVENT     *gRfiuReqSem[MAX_RFIU_UNIT];
OS_EVENT     *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];

#if RFIU_SHARE_CTRLBUS_SUPPORT
OS_EVENT     *gRfiuCtrlBusReqSem;
#endif

static DEF_REGRFIU_CFG gRfiuParm_Tx[MAX_RFIU_UNIT];
static DEF_REGRFIU_CFG gRfiuParm_Rx[MAX_RFIU_UNIT];
DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
int gRfiuFCC247ChUsed[MAX_RFIU_UNIT][2];

unsigned int gRfiuTimer[MAX_RFIU_UNIT];
static unsigned int gRfiuTimer_offset[MAX_RFIU_UNIT];
unsigned int gRfiuTxSwCnt[MAX_RFIU_UNIT]={0};  // used in n-Tx vs 1 RX

unsigned int gRfiu_Op_Sta[MAX_RFIU_UNIT];
unsigned int gRfiu_RX_Sta[MAX_RFIU_UNIT];
unsigned int gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
unsigned int gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];

unsigned int gRfiu_TX_Sta[MAX_RFIU_UNIT];
unsigned int gRfiu_WrapEnc_Sta[MAX_RFIU_UNIT];
unsigned int gRfiu_MpegEnc_Sta[MAX_RFIU_UNIT];
unsigned int gRfiuSyncRevFlag[MAX_RFIU_UNIT] ;
int gRfiuTxFwUpdPercent[MAX_RFIU_UNIT];

#if (RF_PAIR_EN)
unsigned int Temp_TX_MAC_address[MAX_RFIU_UNIT];
unsigned int Temp_TX_CostomerCode[MAX_RFIU_UNIT];

unsigned int Temp_RX_MAC_Address[MAX_RFIU_UNIT];  
unsigned int Temp_RX_CostomerCode[MAX_RFIU_UNIT];

unsigned int MACadrsSetflag[MAX_RFIU_UNIT] ;
#endif

#if RFIU_FCC_FHSS15CH_SUPPORT
  //int gRfiuACK_CH_Table[RFI_DAT_CH_MAX]={10,19,28,37,46,55,64,73,82,91,100,109,118,127,136,136};  
  //int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={10,19,28,37,46,55,64,73,82,91,100,109,118,127,136,136};
  int gRfiuACK_CH_Table[RFI_DAT_CH_MAX]={136,136,127,118,109,100,91,82,73,64,55,46,37,28,19,10};  
  int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={136,136,127,118,109,100,91,82,73,64,55,46,37,28,19,10};
#else
  int gRfiuACK_CH_Table[RFI_DAT_CH_MAX]={16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136};  
  int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136};
#endif

const int gRfiuVitbiOnTable[2]={ RFI_VITBI_DISA,
                                 RFI_VITBI_EN
                               };
const int gRfiuVitbiCodeTable[4]={ RFI_VITBI_CR4_5,
                                   RFI_VITBI_CR3_4,
                                   RFI_VITBI_CR2_3,
                                   RFI_VITBI_CR1_2
                                 };
const int gRfiuRSCodeTable[4]={ RFI_RS_T2,
                                RFI_RS_T4,
                                RFI_RS_T8,
                                RFI_RS_T12
                              };

#if RFIU_TEST
 int gRfiuSyncWordTable[MAX_RFIU_UNIT]={ 
                                         0x5ac37136,
						                 0x5ac37136, 	
                                         0x5ac37136,
                                         0x5ac37136,
                                     #if(MAX_RFIU_UNIT==8)
                                         0x5ac37136,
                                         0x5ac37136,
                                         0x5ac37136,
                                         0x5ac37136,
                                     #endif
                                       };

#else
 int gRfiuSyncWordTable[MAX_RFIU_UNIT]={ 
                                         0x87654321,
						                 0x12345678, 	
                                         0xabcdef01,
                                         0xfedcba98,
                                     #if(MAX_RFIU_UNIT==8)
                                         0x13579bdf,
                                         0xfdb97531,
                                         0x2468ace0,
                                         0x0eca8642,
                                     #endif
                                       };
#endif

  int gRfiuCustomerCode[MAX_RFIU_UNIT]={0};

#if RFIU_TEST
u32 gRfiuTestPktMapTab[32]={
        0x03030303,0x12345678,0x23456710,0x42424242,
        0xfa53791f,0x00000010,0xf000000a,0x11111111,
        0x01150014,0x87654321,0x88888888,0xf0f0f0f0,
        0x1B3D5F81,0x00000000,0x44444444,0xabcdef01,
        0x00F000EF,0xfff00000,0x55555555,0x00000001,
        0xD1E2E0EF,0x0000ffff,0x66666666,0x01010101,
        0xCDF01223,0xf0f0f0f0,0x77777777,0x10101010,
        0x99999999,0x51358471,0x88888888,0xffffffff 
};

u32 gRfiuTestPktMapTab2[32]={
        0x00000100,0x80000000,0x30000000,0x10010001,
        0x00000001,0x00003000,0x01010101,0x00000003,
        0x01000000,0x00010000,0x00110011,0x11111111,
        0x00010001,0x80808080,0x40404040,0x20202020,
        0x00400400,0x00100010,0x50505050,0x02020202,
        0x00000000,0xf0000000,0x0000f000,0x00030000,
        0x00001000,0x40000000,0x00800000,0x10101010,
        0x00000000,0x11100000,0x00000010,0xff000001 
};
#endif


char RandHoppTab[9][RFI_DAT_CH_MAX]={ 
                                      { 0,15, 9, 4, 1,14, 5,11, 2, 8,13, 7, 3,10,12,  6},
                                      { 9,10,11,12,13,14,15,99,13,14,15, 9,10,11,12, 99},  
                                      { 0,10,11,12,13,14,15,99,13,14,15, 0,10,11,12, 99}, 
                                      { 0, 1,11,12,13,14,15,99,13,14,15, 0, 1,11,12, 99}, 
                                      { 0, 1, 2,12,13,14,15,99,13,14,15, 0, 1, 2,12, 99}, 
                                      { 0, 1, 2, 3,13,14,15,99,13,14,15, 0, 1, 2, 3, 99}, 
                                      { 0, 1, 2, 3, 4,14,15,99, 4,14,15, 0, 1, 2, 3, 99}, 
                                      { 0, 1, 2, 3, 4, 5,15,99, 4, 5,15, 0, 1, 2, 3, 99}, 
                                      { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13, 14, 15}
                                    };
//char RandHoppTab[RFI_DAT_CH_MAX]={ 9,10,11,12,13,14,15,12,13,14,15,9,10,11,12, 10};

unsigned int FCC_Unit_Sel;

int rfiuDoorBellTrig;
int rfiuBatCamPIRTrig;
int rfiuBatCamDcDetect;
int rfiuBatCamBattLev;
int rfiuPair2Detect;

u8 AmicReg_Data,AmicReg_Addr,AmicReg_RWen1,AmicReg_RWen2;

u8 rfiuVoxEna[MAX_RFIU_UNIT]={0};
u8 rfiuVoxThresh[MAX_RFIU_UNIT];

#if INSERT_NOSIGNAL_FRAME
static u8 Rx_status[MULTI_CHANNEL_MAX]={0};
#endif

s8 ispTxFWFileName[MAX_RFIU_UNIT][32]={
                                      #if(MAX_RFIU_UNIT == 4)
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin"
                                      #elif(MAX_RFIU_UNIT == 8)
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin",
                                         "\\pa9txfw.bin"
                                      #endif
                                      };

int RF3M_RunCnt[MAX_RFIU_UNIT]={0};

#if MESUARE_BTCWAKEUP_TIME
u32 rfiuBTCWakeTime[MAX_RFIU_UNIT];
#endif

#if(HW_BOARD_OPTION == MR9200_RX_RDI_UDR777 && (PROJ_OPT == 10 || PROJ_OPT == 11))
char* TxMdSensTableExistCL894 = "CL894CS-FP-V0.04-171219 ";
char* TxMdSensTableExistCA814 = "CA814G-HP-V0.07-180104 ";
#endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if RFIU_RX_WAKEUP_TX_SCHEME 
extern u32 rfiuBatCam_LiveMaxTime[MAX_RFIU_UNIT];
extern u32 rfiuBatCamVideo_TotalTime[MAX_RFIU_UNIT];
#endif

#if TX_PIRREC_VMDCHK
extern int rfiuBatCamDcDetect;
extern u8 MotionDetect_en;
extern u32 rfiuPIRRec_VMDTrig;
#endif

extern u32 guiSysTimerCnt;

extern u32 dcfStorageType; /*CY 1023*/
 
extern u8 ciu_2_OpMode;

extern u32  mpeg4MultiStreamEnable;
extern u32  mpeg4MultiStreamStart;
extern int MDTxNewVMDSupport;
extern s32 MD_Diff;
extern u8  MotionDetect_en;

extern u32 guiRFTimerID;
extern u32 guiSysTimerId;
extern u32 guiIRTimerId;

extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern INT32U  guiIISCh0RecDMAId, guiIISCh1RecDMAId, guiIISCh2RecDMAId, guiIISCh3RecDMAId;
extern INT32U  guiIISCh0PlayDMAId, guiIISCh1PlayDMAId, guiIISCh2PlayDMAId, guiIISCh3PlayDMAId;
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];

extern u32 rfiuVideoBufFill_idx[MAX_RFIU_UNIT];
extern u32 rfiuVideoBufPlay_idx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufPlay_idx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufFill_idx[MAX_RFIU_UNIT];
extern u8  rfiuAudioZeroBuf[RFI_AUIDIO_SILENCE_SIZE];
extern u32 rfiuAudioRetWrite_idx;
extern u32 rfiuAudioRetRec_idx;
extern u32 rfiuAudioRetRead_idx;

extern u8  sysBackOsdString[20];
extern u32 rfiuAudioRetFromApp;   
extern u8  AE_Flicker_50_60_sel;
extern u8 *rfiuAudioRetDMANextBuf[RFI_AUDIO_RET_BUF_NUM];

#if HW_MD_SUPPORT
extern u8 MD_blk_Mask_VGA[MC_CH_MAX][MD_BLOCK_NUM_MAX];
extern u8 MD_blk_Mask_HD[MC_CH_MAX][MD_BLOCK_NUM_MAX];
#endif
extern s32 rfiuciu1ZoomInx2(int OnOff,int Xpos,int Ypos);
extern s32 spiWriteRF(s32 dummy);
extern void rfiuAudioRet_RecDMA_ISR(int dummy);

extern void siu_SetNT99340_720P_FrameRate(u8 FrameRate);
extern void siu_SetPO3100K_720P_FrameRate(u8 FrameRate);
extern void siu_SetNT99230_1080p_FrameRate(u32 FrameRate);

#if TX_FW_UPDATE_SUPPORT    
extern int rfiuFwUpdLoadTxFW_SD(int RFUnit);
#endif

#if MOTOR_EN
extern u8  MotorStatusH;   // 0: 停止, 1: 正轉, 2: 負轉
extern u8  MotorStatusV;   // 0: 停止, 1: 正轉, 2: 負轉
#endif

#if INSERT_NOSIGNAL_FRAME
extern u8 Record_flag[MULTI_CHANNEL_MAX];
#endif

#if UI_LIGHT_SUPPORT
extern u8 uiManualLight_DuringUserSetting;
extern int uiManualLight_StartGrid;
#endif
extern u8 SIUMODE;
#if ENABLE_DOOR_BELL
extern u8  uiDoorCount;
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 
 int rfiu_SetTXInfo(u32 v1,u32 v2,u32 v3,u32 v4);
 int rfiuRXJdgDataSwich_3M(int RFUnit,int BitRate,int DAT_CH_sel);
 int rfiuDataRate6M_To_4M(int RFIunit);
 int rfiuDataRate4M_To_6M(int RFIunit);

 int rfiuNoWifiCHsel(int RFUnit,u32 RSSI_CH_Avg[]);

int CheckFCC247ChDiff( int CHNext,int RFUnit);

int rfiuTXCheckACKCome(DEF_REGRFIU_CFG *pRfiuPara);
int rfiuTXCheckAudioRetDataCome(DEF_REGRFIU_CFG *pRfiuPara,int RFUnit );

int marsRfiu_Test_Performance(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode);
int marsRfiu_Test_TxFunc(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode);
int marsRfiu_Test_RxFunc(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode);

int marsRfiu_Test_PktMap(unsigned int TestRun);
int marsRfiu_Test_PktBurst(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode);
int marsRfiu_Measure_RX1RX2_Sensitivity(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode);
s32 marsRfiu_FCC_DirectTXRX(s32 dummy);

int marsRfiu_Test_TxRxCommu(  unsigned int TestRun, 
                                           unsigned int SyncWord,
                                           unsigned int CustomerID);

void rfiu_Tx_Task_UnitX(void* pData);
void rfiu_Rx_Task_UnitX(void* pData);

int rfiuCalBufRemainCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
int rfiuCalVideoDisplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
int rfiuCalAudioplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);

int rfiu_nTnR_FindNextTx(int RFUnit);

#if TX_FW_UPDATE_SUPPORT
int rfiuFwUpdCheckStartCome(DEF_REGRFIU_CFG *pRfiuPara);
void rfiuTXSetVersionInfo(unsigned char *pp);
#endif


#if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
void InitMV400();
void SPI_W0_Board_1(unsigned int val);
void SPI_W0_Board_2(unsigned int val);
void MV400_CH_sel(int BoardSel,BYTE CH);

#elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
void InitA7130();
void A7130_TxMode_Start(int BoardSel);
void A7130_RxMode_Start(int BoardSel);
void A7130_TxMode_Stop(int BoardSel);
void A7130_RxMode_Stop(int BoardSel);

void A7130_CH_sel(int BoardSel,BYTE CH);

void A7130_ID_Update(int BoardSel ,unsigned int NewMACID );
u8 RSSI_measurement_A7130(int BoardSel);

void A7130_ChgTo_2M_B1(void);
void A7130_ChgTo_4M_B1(void);
void A7130_ChgTo_2M_B2(void);
void A7130_ChgTo_4M_B2(void);


#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
void InitA7196();
void A7196_TxMode_Start(int BoardSel);
void A7196_RxMode_Start(int BoardSel);
void A7196_TxMode_Stop(int BoardSel);
void A7196_RxMode_Stop(int BoardSel);

void A7196_CH_sel(int BoardSel,BYTE CH);

void A7196_ID_Update(int BoardSel ,unsigned int NewMACID );
u8 RSSI_measurement_A7196(int BoardSel);

void A7196_ChgTo_4M_B1(void);
void A7196_ChgTo_6M_B1(void);
void A7196_ChgTo_3M_B1(void);

void A7196_ChgTo_4M_B2(void);
void A7196_ChgTo_6M_B2(void);
void A7196_ChgTo_3M_B2(void);
#elif(RFIC_SEL==RFIC_NONE_5M)
void InitRFNONE();
void RFNONE_TxMode_Start(int BoardSel);
void RFNONE_RxMode_Start(int BoardSel);
void RFNONE_TxMode_Stop(int BoardSel);
void RFNONE_RxMode_Stop(int BoardSel);

void RFNONE_CH_sel(int BoardSel,BYTE CH);

void RFNONE_ID_Update(int BoardSel ,unsigned int NewMACID );
u8 RSSI_measurement_RFNONE(int BoardSel);

void RFNONE_ChgTo_10M_B1(void);
void RFNONE_ChgTo_5M_B1(void);

void RFNONE_ChgTo_10M_B2(void);
void RFNONE_ChgTo_5M_B2(void);

#endif

//----------------------------------------------//
s32 RfiuInit(void)
{
    unsigned char err;
    int i,j;
#if RFIU_TEST    
    unsigned int *pp;
#endif
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    u8 Level;
#endif    
    //-------------------//
  #if 0
    timerCountRead(guiRFTimerID, &ID);
    timerCountRead(guiSysTimerId, &temp);
    ID ^= temp;
    timerCountRead(guiIRTimerId, &temp);
    ID ^= temp;
    ID ^= uiRFID[0];
  #endif

    sysRFRxInMainCHsel = 0;
    sysRFRxInPIPCHsel=0;
    sysRFRXPIP_en=0;
    
    sysRF_PTZ_CHsel=0;
    sysRF_AudioChSw_DualMode=0;
    rfiuRX_CamPerRF=1;
	
    rfiuRxMainVideoPlayStart=0;
    rfiuRxMainAudioPlayStart=0;
	rfiuRxSub1VideoPlayStart=0;
	
    rfiuMainVideoTime=0; 
    rfiuMainVideoTime_frac=0;
	
	rfiuSub1VideoTime=0; 
	rfiuSub1VideoTime_frac=0;
	
    rfiuMainAudioTime=0;
    rfiuMainAudioTime_frac=0;
    rfiuDoorBellTrig=0;
    rfiuPair2Detect=0;
    rfiuBatCamPIRTrig=0;
    rfiuBatCamDcDetect=1;
    rfiuBatCamBattLev=RF_BATCAM_TXBATSTAT_NOTREDY;
    rfiuAudioRetFromApp=0;
    rfiuAudioRetStatus=RF_AUDIO_RET_OFF;
#if RFIU_RX_WAKEUP_TX_SCHEME
    //rfiuBatCam_PIRRecDurationTime=RF_BATCAM_REC_INTERVAL;  //PIR 錄影時間
#endif
    AmicReg_Data=0;
    AmicReg_Addr=0;
    AmicReg_RWen1=0;
    AmicReg_RWen2=0;
    
    gRfiuSWReadySemEvt=OSSemCreate(1);
    
#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

   #if ((SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
        (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
        sysVideoInCHsel=5;
   #else
        #if CIU2_REPLACE_CIU1
        sysVideoInCHsel=2;
        #else
        sysVideoInCHsel=1;
        #endif
   #endif
   
#endif

  #if PWIFI_SUPPORT
     #if(PWIFI_TEST_ACK_DATA > 1)
         rfiuRX_CamOnOff_Sta=0x01;
         rfiuRX_CamOnOff_Num=1;
      #endif   
  
    DEBUG_RFIU_P2("rfiuRX_CamOnOff_Sta=0x%x\n",rfiuRX_CamOnOff_Sta);
  #endif  
    rfiu_InitCamOnOff(rfiuRX_CamOnOff_Sta);

    SYS_CTL0        |= SYS_CTL0_RF1013_CKEN; 

#if(RFI_TEST_TX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFIU_TEST )
    SYS_CTL0_EXT        |= SYS_CTL0_EXT1_RF1013_CKEN; 
  #if ((CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
     GpioActFlashSelect |=CHIP_IO_RF13_PORT2_EN; 
  #endif
#endif


#if(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2)
    rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
#elif( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) )
    sysCameraMode = SYS_CAMERA_MODE_RF_RX_DUALSCR;
    rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
#elif( (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test)||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W))
    sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
    rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
#else //8120Rx,8200RX
    #if UI_RX_PWRON_QUAD_ENA
        if(rfiuRX_CamOnOff_Num<2)
        {
           sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;
           rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
        }
        else
        {
           sysCameraMode = SYS_CAMERA_MODE_RF_RX_QUADSCR;
           rfiuRX_OpMode = RFIU_RX_OPMODE_QUAD;  
           sysRFRxInMainCHsel=0x0ff;
        }
    #else
        sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;
        rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
    #endif
#endif

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
    (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )

#else
   if(rfiuRX_OpMode == RFIU_RX_OPMODE_QUAD)
   {
          #if RFRX_HALF_MODE_SUPPORT
            if(rfiuRX_CamOnOff_Num <= 2)
            {
               if(sysTVOutOnFlag)
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
               else
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
            }
            else
          #endif
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
    }
#endif    
    rfiuRX_P2pVideoQuality=RF_P2PVdoQalt_MEDIUM;
    
    for(j=0;j<DISPLAY_BUF_NUM;j++)
         rfiuMainVideoPresentTime[j]=0;

    //Lucian: 假設audio format為unsigned 8-bit PCM,若否需修正
    for(i=0;i<RFI_AUIDIO_SILENCE_SIZE;i++)
        rfiuAudioZeroBuf[i]=0x80;
    
    SYSReset(SYS_RSTCTL_RF1013_RST);
    gRfiuFlagGrp = OSFlagCreate(0x00000000, &err);
    gRfiu_nTx1RSwFlagGrp=OSFlagCreate(0x00000000, &err);
    gRfiuStateFlagGrp = OSFlagCreate(0x00000000, &err);
    gRf868EventFlagGrp=OSFlagCreate(0x00000000, &err);
#if RFIU_SHARE_CTRLBUS_SUPPORT
    gRfiuCtrlBusReqSem=OSSemCreate(1);
#endif

#if FPGA_BOARD_A1018_SERIES
    RFIU_DEBUG_PORT1= 0x14131211;
    RFIU_DEBUG_PORT2= 0x03020117;
    RFIU_DEBUG_PORT3= 0x00000704;
#endif

#if RFIU_DEBUGPORT_EN

    SYS_PIN_MUX_SEL = DEBUG_RFI13_EN |
                      LOWER20BITs |
                      PACKAGE_216PIN ;


   GpioActFlashSelect &= ~( GPIO_GPIU_FrDISP_EN );
   GpioActFlashSelect &= ~( GPIO_DV2FrDISP_EN );
   GpioActFlashSelect &= ~( GPIO_IISFrDISP_EN);
   
   GpioActFlashSelect |= GPIO_DEBUGE_EN;                  


    RFIU_DEBUG_PORT1= 0x04030201;
    RFIU_DEBUG_PORT2= 0x14131211;
    RFIU_DEBUG_PORT3= 0x00001606;
    
#endif    

//-----------A1016x series------------//
#if(HW_BOARD_OPTION == A1016_REALCHIP_A)   // RFU pin mux: use SPI2 for RF1 amd RF2
    GpioActFlashSelect |= GPIO_RF12_FrSP2_EN;
//-----------------------A1018X------------------//
#elif( (HW_BOARD_OPTION  == A1018_EVB_128M) || (HW_BOARD_OPTION  == A1018B_EVB_128M) || (HW_BOARD_OPTION  == A1019A_EVB_128M_TX) || (HW_BOARD_OPTION == A1019A_SKB_128M_RX) || (HW_BOARD_OPTION == A1018B_SKB_128M_RX) || (HW_BOARD_OPTION == A1018B_SKB_128M_BT_RX)|| (HW_BOARD_OPTION == MR9600_RX_DB_ETH))
    GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from UART-B
#elif( (HW_BOARD_OPTION  == MR9120_TX_DB) ||  (HW_BOARD_OPTION  == MR9120_TX_OPCOM) )
    GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from UART-B
#elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port
#elif(HW_BOARD_OPTION  == MR9300_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port   
#elif(HW_BOARD_OPTION  == MR9200_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port    
#elif(HW_BOARD_OPTION  == MR9600_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port       
#elif(HW_BOARD_OPTION  == MR9600_RX_OPCOM_CVI)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port        
#endif


    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       memset(&gRfiuParm_Tx[i],0,sizeof(DEF_REGRFIU_CFG));
       memset(&gRfiuParm_Rx[i],0,sizeof(DEF_REGRFIU_CFG));
       memset(&gRfiuUnitCntl[i],0,sizeof(DEF_RFIU_UNIT_CNTL));

       for(j=0;j<RFI_DAT_CH_MAX;j++)
         rfiuRSSI_CH_Avg[i][j]=0;

       gRfiu_Op_Sta[i]      = RFIU_OP_INIT;
       gRfiu_WrapDec_Sta[i] = RFI_WRAPDEC_TASK_NONE;
       gRfiu_MpegDec_Sta[i] = RFI_MPEGDEC_TASK_NONE;
       gRfiu_RX_Sta[i]      = RFI_RX_TASK_NONE;
       gRfiuUnitCntl[i].RFpara.PIR_en=1;
       gRfiuUnitCntl[i].RFpara.RxForceTxFirstReboot=1;  
       gRfiuUnitCntl[i].RFpara.TxBatteryLev=RF_BATCAM_TXBATSTAT_NOSHOW;
    #if PWIFI_SUPPORT
       gRfiuUnitCntl[i].RFpara.Rx_StaionJoinSta=PWIFI_RX_STAJOIN_NONE;
    #endif
       
       gRfiu_WrapEnc_Sta[i] = RFI_WRAPENC_TASK_NONE;
       gRfiu_MpegEnc_Sta[i] = RFI_MPEGENC_TASK_NONE;
       gRfiu_TX_Sta[i]      = RFI_TX_TASK_NONE;

       rfiuVoxThresh[i]=100;

       gRfiuFCC247ChUsed[i][0]=-1;
       gRfiuFCC247ChUsed[i][1]=-1;

   #if RFIU_RX_WAKEUP_TX_SCHEME    
       gRfiuUnitCntl[i].RFpara.BateryCam_support=1;
   #endif
       rfiuRX_OpModeCmdRetry[i]=0;

       rfiu_resetflag[i]=0;
       rfiuRXWrapSyncErrCnt[i]=0;

       rfiuRxVideoBufMngWriteIdx[i]=0;
       rfiuRxIIsSounBufMngWriteIdx[i]=0;

	   rfiuVideoBufFill_idx[i]=0;
	   rfiuVideoBufPlay_idx[i]=0;

	   rfiuAudioBufPlay_idx[i]=0;
	   rfiuAudioBufFill_idx[i]=0;

	   rfiuVideoTimeBase[i]=0;
       gRfiuUnitCntl[i].RFpara.RF_ID=uiRFID[i];
       
    #if(FPGA_BOARD_A1018_SERIES)
    #else
	   #if( (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) )

	   #else
        gRfiuSyncWordTable[i]=(int)uiRFID[i];
        gRfiuCustomerCode[i]=(int)uiRFCODE[i];
	   #endif
    #endif
              
       gRfiuReqSem[i] = OSSemCreate(1);
       gRfiuAVCmpSemEvt[i]= OSSemCreate(0);
    }
#if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
    //config Muchip board
    InitMV400();  
#elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
    InitA7130();
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    InitA7196();
#elif(RFIC_SEL==RFIC_NONE_5M)
    InitRFNONE();
#endif

#if RFIU_TEST
    pp=(unsigned int *)rfiuOperBuf[0];
    for(i=0;i<64*128*(RFI_BUF_SIZE_GRPUNIT+1)/4;i++)
    {
        *pp= *pp + i*1343;
        pp ++;
    }
#endif

#if 1  //Lucian: check 是否等於 default value(setup.ini)
    if(gRfiuUnitCntl[0].RFpara.RF_ID == 0x12345678 )
        gRfiuUnitCntl[0].RFpara.RF_ID=0xffffffff;

    if(gRfiuUnitCntl[1].RFpara.RF_ID == 0x87654321 )
        gRfiuUnitCntl[1].RFpara.RF_ID=0xffffffff;

    if(gRfiuUnitCntl[2].RFpara.RF_ID == 0x2a5c7536 )
        gRfiuUnitCntl[2].RFpara.RF_ID=0xffffffff;

    if(gRfiuUnitCntl[3].RFpara.RF_ID == 0xa2c57536 )
        gRfiuUnitCntl[3].RFpara.RF_ID=0xffffffff;  

    rfiuRX_CamPair_Sta=0;
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
        if( gRfiuUnitCntl[i].RFpara.RF_ID != 0xffffffff )
        {
            rfiuRX_CamPair_Sta = rfiuRX_CamPair_Sta | (0x01 << i);
        }
    }
#endif

    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuTimer_offset[i]=0;
    }

#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
     (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    gRfiuUnitCntl[0].RFpara.PIR_en=iconflag[UI_MENU_SETIDX_PIR];
    gRfiuUnitCntl[0].RFpara.MD_en =iconflag[UI_MENU_SETIDX_TX_MOTION];
    gRfiuUnitCntl[0].RFpara.MD_Level_Day =iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY];
    gRfiuUnitCntl[0].RFpara.MD_Level_Night =iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT];
    //gRfiuUnitCntl[0].RFpara.RF_ID=ID;
#endif

#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    rfiuBatCamDcDetect = sysDCPowerDetect();  //Lucian:判斷是否有 DC 電源插入.
    rfiuPair2Detect = sysRFPair2Detect();
    rfiuBatCamPIRTrig = sysPIRTrigDetect();
    //rfiuBatCamBattLev=;
    rfiuDoorBellTrig = sysRFDoorBell2Detect();

    if(rfiuBatCamDcDetect)
        rfiuBatCamPIRTrig=0;
    
    DEBUG_RFIU_P2("PIRTrig=%d,%d,%d, %d\n",rfiuBatCamPIRTrig,rfiuBatCamDcDetect,rfiuPair2Detect, rfiuDoorBellTrig);
    gpioSetInt(GPIO_GROUP_DC_DETECT, GPIO_BIT_DC_DETECT, 1);
    if(rfiuBatCamDcDetect)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        sysSetEvt(SYS_EVT_TXPIRTRIGONOFF,1);
    #endif
        gRfiuUnitCntl[0].RFpara.PIR_en=1;
    #if(BTC_PCB_VERSION  == 5)
        sysTxPIRTrigOnOff(1);
    #elif(BTC_PCB_VERSION  == 3)
        sysTxPIRTrigOnOff(1);
    #endif
    }
    else
    {
        //Lucian: 之後需讀去PIR_PWR_STA
        gpioGetLevel(GPIO_GROUP_PIRON_DET, GPIO_BIT_PIRON_DET, &Level);
        gRfiuUnitCntl[0].RFpara.PIR_en=Level;
    #if(BTC_PCB_VERSION  == 5)
        if(gRfiuUnitCntl[0].RFpara.PIR_en)
           sysTxPIRTrigOnOff(1);
    #elif(BTC_PCB_VERSION  == 3)
        if(gRfiuUnitCntl[0].RFpara.PIR_en)
           sysTxPIRTrigOnOff(1);
    #endif
    }
#endif


#if (RFIU_SUPPORT)
   for(i=0;i<MAX_RFIU_UNIT;i++)
   {
   #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
        (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
      gRfiuUnitCntl[i].RFpara.TX_TimeStampOn=iconflag[UI_MENU_SETIDX_CH1_TSP_ON+i]; //most RX no record this, no dare to modify all projects
      gRfiuUnitCntl[i].RFpara.TX_TimeStampType=iconflag[UI_MENU_SETIDX_CH1_TSP_TYPE+i];
      gRfiuUnitCntl[i].RFpara.TX_SubStreamBRSel=iconflag[UI_MENU_SETIDX_CH1_STREAM_QUALITY+i];
   #elif( SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
        #if(USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC) //show timestamp when RX send TSP on, avoid show on wrong position
          gRfiuUnitCntl[i].RFpara.TX_TimeStampOn=0; 
        #else
          gRfiuUnitCntl[i].RFpara.TX_TimeStampOn=iconflag[UI_MENU_SETIDX_CH1_TSP_ON+i];
        #endif
          gRfiuUnitCntl[i].RFpara.TX_TimeStampType=iconflag[UI_MENU_SETIDX_CH1_TSP_TYPE+i];
          gRfiuUnitCntl[i].RFpara.TX_SubStreamBRSel=iconflag[UI_MENU_SETIDX_CH1_STREAM_QUALITY+i];

   #endif
   }
#endif

#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
   #if( (Sensor_OPTION == Sensor_PO2210K_YUV601) || (Sensor_OPTION == Sensor_XC7021_SC2133) || (Sensor_OPTION == Sensor_NT99230_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_ZN220_YUV601))
      gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_FHD;
   #elif( (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_MI1320_YUV601))
      gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
   #elif( (Sensor_OPTION == Sensor_OV2643_YUV601) )
      gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_UXGA;
   #else
      gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;
   #endif
#endif   


#if RFIU_TEST    
    #if RFI_SELF_TEST_TXRX_PROTOCOL
        //--TX--//
        OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);

        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1); 
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);        
    #endif
#endif

   return 1;
}

s32 RfiuReset(int RFIunit)
{
   int count;
   int i;
   u32 temp;
   static int ErrCnt[MAX_RFIU_UNIT]=0;
   //=========//
   

   *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_RESET;
   count=0;
   for(i=0;i<50;i++); //delay 
   while( (*((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit))) & RFI_RESET )
   {
      DEBUG_RFIU_P2("RFI-%d reset Fail\n",RFIunit);
      count ++;
      if(count > 10)
        break;
   }

#if 1
   if(count > 10)
   {
         temp=*((volatile unsigned *)(REG_RFIU_SyncWord_H+0x1000*RFIunit));
         if((temp & 0x0ffff) != 0x1234)
         {
             if(ErrCnt[RFIunit]>4)
             {
                DEBUG_RFIU_P2("RFIU-%d local reset is Error! Reboot! 0x%x\n",RFIunit,temp);
				sysForceWDTtoReboot();
             }
             ErrCnt[RFIunit] ++;
             DEBUG_RFIU_P2("RFIU-%d local reset is Fail! 0x%x\n",RFIunit,temp);
         }
         else
         {
             ErrCnt[RFIunit]=0;
         }
   }
#endif
   

   return 1;
}

void rfiu_InitCamOnOff(u32 Status)
{
  int i;
  u32 Bits;
  unsigned int  cpu_sr = 0;
  //------------------------//

  OS_ENTER_CRITICAL();
#if RFI_TEST_2x_RX_PROTOCOL_B1
  gRfiuTxSwCnt[0]=0x0ff;
  Bits=Status;
  for(i=0;i<2;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[0] = i;
        sysRFRxInMainCHsel=i;
        break;
     }
     Bits= Bits>>1;
  }  
#elif RFI_TEST_4TX_2RX_PROTOCOL
  gRfiuTxSwCnt[0]=0x0ff;
  Bits=Status;
  for(i=0;i<2;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[0] = i;
        break;
     }
     Bits= Bits>>2;
  }  
  //===//
  gRfiuTxSwCnt[1]=0x0ff;
  Bits=Status>>1;
  for(i=0;i<2;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[1] = i;
        break;
     }
     Bits= Bits>>2;
  }  

  //Find Main channel
  Bits=Status;
  for(i=0;i<4;i++)
  {
     if(Bits & 0x01)
     {
        sysRFRxInMainCHsel=i;
        break;
     }
     Bits= Bits>>1;
  }     
#elif RFI_TEST_8TX_2RX_PROTOCOL
  gRfiuTxSwCnt[0]=0x0ff;
  Bits=Status;
  for(i=0;i<4;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[0] = i;
        break;
     }
     Bits= Bits>>2;
  }  
  //===//
  gRfiuTxSwCnt[1]=0x0ff;
  Bits=Status>>1;
  for(i=0;i<4;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[1] = i;
        break;
     }
     Bits= Bits>>2;
  }  

  //Find Main channel
  Bits=Status;
  for(i=0;i<8;i++)
  {
     if(Bits & 0x01)
     {
        sysRFRxInMainCHsel=i;
        break;
     }
     Bits= Bits>>1;
  }      

  
#elif RFI_TEST_4x_RX_PROTOCOL_B1
  gRfiuTxSwCnt[0]=0x0ff;
  Bits=Status;
  for(i=0;i<4;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[0] = i;
        sysRFRxInMainCHsel=i;
        break;
     }
     Bits= Bits>>1;
  }    

#elif RFI_TEST_8TX_1RX_PROTOCOL
  gRfiuTxSwCnt[0]=0x0ff;
  Bits=Status;
  for(i=0;i<8;i++)
  {
     if(Bits & 0x01)
     {
        gRfiuTxSwCnt[0] = i;
        sysRFRxInMainCHsel=i;
        break;
     }
     Bits= Bits>>1;
  }    
#endif
  OS_EXIT_CRITICAL();

}

int rfiuDataRate6M_To_4M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7196_6M)
   int Reg;
   int Div;

   Div= (RFI_CLK_FREQ/4000000) & 0x0ff;

   Reg= *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit));
   Reg &= (~0x0ff);
   Reg |= (Div-1) & 0x0ff;
   *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit))=Reg;

   *((volatile unsigned *)(REG_RFIU_DclkConfig+0x1000*RFIunit))=( (RFI_RX_PREAMBLE_NUM<<28) | ( (Div-1)<<24) | ((Div*2)<<16) | (Div<<8) | (Div<<0));

   if(RFIunit==0)
      A7196_ChgTo_4M_B1();
   else if(RFIunit==1)
      A7196_ChgTo_4M_B2();

#endif

   return 1;
}

int rfiuDataRate4M_To_6M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7196_6M)

   if(RFIunit==0)
      A7196_ChgTo_6M_B1();
   else if(RFIunit==1)
      A7196_ChgTo_6M_B2();

#endif

   return 1;
}

int rfiuDataRate6M_To_3M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7196_6M)
   int Reg;
   int Div;

   Div= (RFI_CLK_FREQ/3000000) & 0x0ff;

   Reg= *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit));
   Reg &= (~0x0ff);
   Reg |= (Div-1) & 0x0ff;
   *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit))=Reg;

   *((volatile unsigned *)(REG_RFIU_DclkConfig+0x1000*RFIunit))=( (RFI_RX_PREAMBLE_NUM<<28) | ( 0x0f<<24) | ((Div*2)<<16) | (Div<<8) | (Div<<0));

   if(RFIunit==0)
      A7196_ChgTo_3M_B1();
   else if(RFIunit==1)
      A7196_ChgTo_3M_B2();
#endif

   return 1;
}

int rfiuDataRate3M_To_6M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7196_6M)

   if(RFIunit==0)
      A7196_ChgTo_6M_B1();
   else if(RFIunit==1)
      A7196_ChgTo_6M_B2();
#endif

   return 1;
}

int rfiuDataRate4M_To_2M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7130_4M)
   int Reg;
   int Div;

   Div= (RFI_CLK_FREQ/3000000) & 0x0ff; //Lucian: 此時RF Clk=32M.  32/16=2M

   Reg= *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit));
   Reg &= (~0x0ff);
   Reg |= (Div-1) & 0x0ff;
   *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit))=Reg;

   *((volatile unsigned *)(REG_RFIU_DclkConfig+0x1000*RFIunit))=( (RFI_RX_PREAMBLE_NUM<<28) | ( 0x0f<<24) | ((Div*2)<<16) | (Div<<8) | (Div<<0));


   if(RFIunit==0)
      A7130_ChgTo_2M_B1();
   else if(RFIunit==1)
      A7130_ChgTo_2M_B2();
#endif

   return 1;
}

int rfiuDataRate2M_To_4M(int RFIunit)
{
#if(RFIC_SEL==RFIC_A7130_4M)
   if(RFIunit==0)
      A7130_ChgTo_4M_B1();
   else if(RFIunit==1)
      A7130_ChgTo_4M_B2();
#endif

   return 1;
}

unsigned int rfiuDataPktConfig_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara,int RFCh)
{
     unsigned int IntrMask;
     unsigned char err;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
     unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
     //-------------------------------//


     OSSemPend(gRfiuReqSem[RFIunit], OS_IPC_WAIT_FOREVER, &err);

     IntrMask = 0x03<<(RFIunit*2);
     OSFlagPost(gRfiuFlagGrp, IntrMask , OS_FLAG_CLR, &err);
     RFIU_INT_EN |= IntrMask;
    
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit))       = RFI_ENA |
                                                                     RFI_MODE_TX |
                                                                     RFI_TYPE_DATA |
                                                                     RFI_DATA_DECI_INTE |
                                                                     RFI_BYTEINTX_EN |
                                                                     RFI_TRAIL_DISA |
                                                                     pRfiuPara->SyncWordLenSel |
                                                                     pRfiuPara->Vitbi_en |
                                                                     pRfiuPara->CovCodeRateSel |
                                                                     pRfiuPara->RsCodeSizeSel |
                                                                    (pRfiuPara->DummyDataNum << RFI_DUMMYDATA_NUM_SHFT) |
                                                                    (pRfiuPara->PreambleNum<< RFI_PREAMBLE_NUM_SHFT) |
                                                                    (pRfiuPara->DummyPreambleNum<< RFI_DUMMYPREAMBLE_NUM_SHFT);
    if(pRfiuPara->Customer_ID_ext_en)
        *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_CUSTOMER_ID_EXT_EN;

    if(pRfiuPara->SuperBurstMode_en)
        *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_SUPER_BURST_EN;
        
                                                                   
    *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit))       =((pRfiuPara->UserData_H & RFI_USER_DATA_H_MASK) <<RFI_USER_DATA_H_SHFT) | RFI_SEQU_PKT_ENA | ( (RFI_CLK_DIV_SEL-1)<<RFI_TXCLK_DIV_SHFT); // 32/16=2M(暫定)
    *((volatile unsigned *)(REG_RFIU_SyncWord_L+0x1000*RFIunit))   =((pRfiuPara->UserData_L & RFI_USER_DATA_L_MASK) << RFI_USRDATA_L_SHFT) | ( (pRfiuPara->PktSyncWord & 0x0ffff)<<RFI_SYNCWORD_NUM_SHFT); 
 
    *((volatile unsigned *)(REG_RFIU_SyncWord_H+0x1000*RFIunit))=pRfiuPara->PktSyncWord >> 16;
    *((volatile unsigned *)(REG_RFIU_RxClockAdjust+0x1000*RFIunit))=pRfiuPara->RxClkAdjust;
    *((volatile unsigned *)(REG_RFIU_DclkConfig+0x1000*RFIunit))   =pRfiuPara->DclkConfig;
    *((volatile unsigned *)(REG_RFIU_RxTimingFineTune+0x1000*RFIunit))=pRfiuPara->RxTimingFineTune;

    if( (pRfiuPara->TxRxPktNum <1) || (pRfiuPara->TxRxPktNum >256))
        DEBUG_RFIU("Warning! TX packet number is illegle!");
    *((volatile unsigned *)(REG_RFIU_BurstPktCNTL+0x1000*RFIunit)) =(((pRfiuPara->TxRxPktNum-1) & 0x0ff) <<RFU_PKT_BURST_NUM_SHFT) |
                                                                     ((pRfiuPara->Customer_ID & RFU_CUSTOMER_ID_MASK)<<RFU_CUSTOMER_ID_SHFT); //zero start
        
    *((volatile unsigned *)(REG_RFIU_PktGrp_0_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[0];
    *((volatile unsigned *)(REG_RFIU_PktGrp_1_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[1];
    *((volatile unsigned *)(REG_RFIU_PktGrp_2_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[2];
    *((volatile unsigned *)(REG_RFIU_PktGrp_3_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[3];
    
    *((volatile unsigned *)(REG_RFIU_PktGrp_4_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[4];
    *((volatile unsigned *)(REG_RFIU_PktGrp_5_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[5];
    *((volatile unsigned *)(REG_RFIU_PktGrp_6_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[6];
    *((volatile unsigned *)(REG_RFIU_PktGrp_7_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[7];

    *((volatile unsigned *)(REG_RFIU_PktGrp_8_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[8];
    *((volatile unsigned *)(REG_RFIU_PktGrp_9_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[9];
    *((volatile unsigned *)(REG_RFIU_PktGrp_10_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[10];
    *((volatile unsigned *)(REG_RFIU_PktGrp_11_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[11];

    *((volatile unsigned *)(REG_RFIU_PktGrp_12_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[12];
    *((volatile unsigned *)(REG_RFIU_PktGrp_13_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[13];
    *((volatile unsigned *)(REG_RFIU_PktGrp_14_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[14];
    *((volatile unsigned *)(REG_RFIU_PktGrp_15_Addr+0x1000*RFIunit))=pRfiuPara->Pkt_Grp_offset[15];

    *((volatile unsigned *)(REG_RFIU_TxRxOpBaseAddr+0x1000*RFIunit))=(unsigned int)pRfiuPara->TxRxOpBaseAddr;
    

    *((volatile unsigned *)(REG_RFIU_PacketMap_0+0x1000*RFIunit))  =pRfiuPara->PktMap[0];
    *((volatile unsigned *)(REG_RFIU_PacketMap_1+0x1000*RFIunit))  =pRfiuPara->PktMap[1];
    *((volatile unsigned *)(REG_RFIU_PacketMap_2+0x1000*RFIunit))  =pRfiuPara->PktMap[2];
    *((volatile unsigned *)(REG_RFIU_PacketMap_3+0x1000*RFIunit))  =pRfiuPara->PktMap[3];
    *((volatile unsigned *)(REG_RFIU_PacketMap_4+0x1000*RFIunit))  =pRfiuPara->PktMap[4];
    *((volatile unsigned *)(REG_RFIU_PacketMap_5+0x1000*RFIunit))  =pRfiuPara->PktMap[5];
    *((volatile unsigned *)(REG_RFIU_PacketMap_6+0x1000*RFIunit))  =pRfiuPara->PktMap[6];
    *((volatile unsigned *)(REG_RFIU_PacketMap_7+0x1000*RFIunit))  =pRfiuPara->PktMap[7];

    *((volatile unsigned *)(REG_RFIU_PacketMap_8+0x1000*RFIunit))  =pRfiuPara->PktMap[8];
    *((volatile unsigned *)(REG_RFIU_PacketMap_9+0x1000*RFIunit))  =pRfiuPara->PktMap[9];
    *((volatile unsigned *)(REG_RFIU_PacketMap_10+0x1000*RFIunit))  =pRfiuPara->PktMap[10];
    *((volatile unsigned *)(REG_RFIU_PacketMap_11+0x1000*RFIunit))  =pRfiuPara->PktMap[11];
    *((volatile unsigned *)(REG_RFIU_PacketMap_12+0x1000*RFIunit))  =pRfiuPara->PktMap[12];
    *((volatile unsigned *)(REG_RFIU_PacketMap_13+0x1000*RFIunit))  =pRfiuPara->PktMap[13];
    *((volatile unsigned *)(REG_RFIU_PacketMap_14+0x1000*RFIunit))  =pRfiuPara->PktMap[14];
    *((volatile unsigned *)(REG_RFIU_PacketMap_15+0x1000*RFIunit))  =pRfiuPara->PktMap[15];

    *((volatile unsigned *)(REG_RFIU_PacketMap_16+0x1000*RFIunit))  =pRfiuPara->PktMap[16];
    *((volatile unsigned *)(REG_RFIU_PacketMap_17+0x1000*RFIunit))  =pRfiuPara->PktMap[17];
    *((volatile unsigned *)(REG_RFIU_PacketMap_18+0x1000*RFIunit))  =pRfiuPara->PktMap[18];
    *((volatile unsigned *)(REG_RFIU_PacketMap_19+0x1000*RFIunit))  =pRfiuPara->PktMap[19];
    *((volatile unsigned *)(REG_RFIU_PacketMap_20+0x1000*RFIunit))  =pRfiuPara->PktMap[20];
    *((volatile unsigned *)(REG_RFIU_PacketMap_21+0x1000*RFIunit))  =pRfiuPara->PktMap[21];
    *((volatile unsigned *)(REG_RFIU_PacketMap_22+0x1000*RFIunit))  =pRfiuPara->PktMap[22];
    *((volatile unsigned *)(REG_RFIU_PacketMap_23+0x1000*RFIunit))  =pRfiuPara->PktMap[23];

    *((volatile unsigned *)(REG_RFIU_PacketMap_24+0x1000*RFIunit))  =pRfiuPara->PktMap[24];
    *((volatile unsigned *)(REG_RFIU_PacketMap_25+0x1000*RFIunit))  =pRfiuPara->PktMap[25];
    *((volatile unsigned *)(REG_RFIU_PacketMap_26+0x1000*RFIunit))  =pRfiuPara->PktMap[26];
    *((volatile unsigned *)(REG_RFIU_PacketMap_27+0x1000*RFIunit))  =pRfiuPara->PktMap[27];
    *((volatile unsigned *)(REG_RFIU_PacketMap_28+0x1000*RFIunit))  =pRfiuPara->PktMap[28];
    *((volatile unsigned *)(REG_RFIU_PacketMap_29+0x1000*RFIunit))  =pRfiuPara->PktMap[29];
    *((volatile unsigned *)(REG_RFIU_PacketMap_30+0x1000*RFIunit))  =pRfiuPara->PktMap[30];
    *((volatile unsigned *)(REG_RFIU_PacketMap_31+0x1000*RFIunit))  =pRfiuPara->PktMap[31];

    *((volatile unsigned *)(REG_RFIU_BITSTUFF_DECT+0x1000*RFIunit)) = 0x0a;

#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
#endif
     OS_ENTER_CRITICAL();
  #if( RFIU_DATA_6M_ACK_4M_SUPPORT && (RFIC_SEL == RFIC_A7196_6M) )
    #if RFIU_TEST
       rfiuDataRate4M_To_6M(RFIunit);
    #elif(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
       if(RFIunit == 0)
          rfiuDataRate4M_To_6M(RFIunit);
       else if(RFIunit == 1)
          rfiuDataRate6M_To_4M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
            (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
       rfiuDataRate4M_To_6M(RFIunit);
    #else
       rfiuDataRate6M_To_4M(RFIunit);
    #endif
  #elif (RFIU_DATA_6M_ACK_3M_SUPPORT && (RFIC_SEL==RFIC_A7196_6M))
    #if RFIU_TEST
       rfiuDataRate3M_To_6M(RFIunit);
    #elif(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
       if(RFIunit == 0)
          rfiuDataRate3M_To_6M(RFIunit);
       else if(RFIunit == 1)
          rfiuDataRate6M_To_3M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
            (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
       if(gRfiuUnitCntl[RFCh].TXSendDataUse3M)
       {
          rfiuDataRate6M_To_3M(RFIunit);
          //DEBUG_RFIU_P2("TXSendDataUse3M\n");
       }   
       else 
          rfiuDataRate3M_To_6M(RFIunit);
    #else //RX
       rfiuDataRate6M_To_3M(RFIunit);
    #endif
  #elif (RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M))
    #if RFIU_TEST
        rfiuDataRate2M_To_4M(RFIunit);
    #elif(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        if(RFIunit == 0)
          rfiuDataRate2M_To_4M(RFIunit);
        else if(RFIunit == 1)
          rfiuDataRate4M_To_2M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||\
            (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        if(gRfiuUnitCntl[RFCh].TXSendDataUse3M)
        {
            rfiuDataRate4M_To_2M(RFIunit);
        }
        else
        {
            rfiuDataRate2M_To_4M(RFIunit);
        }
    #else //RX
       rfiuDataRate4M_To_2M(RFIunit);
    #endif
  #endif
     
  #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
    A7130_TxMode_Start(RFIunit+1);
  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    A7196_TxMode_Start(RFIunit+1);
  #elif(RFIC_SEL==RFIC_NONE_5M)
    RFNONE_TxMode_Start(RFIunit+1);
  #endif
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_TRIG;
     OS_EXIT_CRITICAL();
#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPost(gRfiuCtrlBusReqSem);
#endif

    return 1;
}

unsigned int rfiuDataPktConfig_Rx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara,int RFCh)
{
     unsigned int IntrMask;
     unsigned char err;
#if (RFI_FPGA_NOISE_GEN && RFI_FPGA_PERFORMANCE_MEASURE)  
     static int err_testrun=0;
     int err_interval;
     int err_num;
     int err_start;
     int err_burst;
     int temp;
#endif   
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
     //------------------------------//


     OSSemPend(gRfiuReqSem[RFIunit], OS_IPC_WAIT_FOREVER, &err);

     IntrMask = 0x03<<(RFIunit*2);
     OSFlagPost(gRfiuFlagGrp, IntrMask , OS_FLAG_CLR, &err);
     RFIU_INT_EN |= IntrMask;
 
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit))       = RFI_ENA |
                                                                     RFI_MODE_RX |
                                                                     RFI_TYPE_DATA |
                                                                     RFI_DATA_DECI_INTE |
                                                                     RFI_BYTEINTX_EN |
                                                                     RFI_TRAIL_DISA |
                                                                     pRfiuPara->SyncWordLenSel |
                                                                     pRfiuPara->Vitbi_en |
                                                                     pRfiuPara->CovCodeRateSel |
                                                                     pRfiuPara->RsCodeSizeSel |
                                                                    (pRfiuPara->DummyDataNum << RFI_DUMMYDATA_NUM_SHFT) |
                                                                    (pRfiuPara->PreambleNum<< RFI_PREAMBLE_NUM_SHFT) |
                                                                    (pRfiuPara->DummyPreambleNum<< RFI_DUMMYPREAMBLE_NUM_SHFT);
    if(pRfiuPara->Customer_ID_ext_en)
        *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_CUSTOMER_ID_EXT_EN;

    if(pRfiuPara->SuperBurstMode_en)
        *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_SUPER_BURST_EN;
    
    *((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit))       =( RFI_NEWPACKETSYNC_ENA | (pRfiuPara->UserData_H & RFI_USER_DATA_H_MASK) <<RFI_USER_DATA_H_SHFT) | RFI_SEQU_PKT_ENA | ( (RFI_CLK_DIV_SEL-1)<<RFI_TXCLK_DIV_SHFT); // 32/16=2M(暫定)
 
    *((volatile unsigned *)(REG_RFIU_SyncWord_L+0x1000*RFIunit))   =((pRfiuPara->UserData_L & RFI_USER_DATA_L_MASK) << RFI_USRDATA_L_SHFT) | ((pRfiuPara->PktSyncWord & 0x0ffff)<<RFI_SYNCWORD_NUM_SHFT); 

    *((volatile unsigned *)(REG_RFIU_BurstPktCNTL+0x1000*RFIunit)) =(((pRfiuPara->TxRxPktNum-1) & 0x0ff) <<RFU_PKT_BURST_NUM_SHFT) |
                                                                     ((pRfiuPara->Customer_ID & RFU_CUSTOMER_ID_MASK)<<RFU_CUSTOMER_ID_SHFT); //zero start

    *((volatile unsigned *)(REG_RFIU_SyncWord_H+0x1000*RFIunit))= pRfiuPara->PktSyncWord >> 16;;
    *((volatile unsigned *)(REG_RFIU_RxClockAdjust+0x1000*RFIunit))=pRfiuPara->RxClkAdjust;
    *((volatile unsigned *)(REG_RFIU_DclkConfig+0x1000*RFIunit))   =pRfiuPara->DclkConfig;
  #if( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    *((volatile unsigned *)(REG_RFIU_RxTimingFineTune+0x1000*RFIunit))=pRfiuPara->RxTimingFineTune & (~0x07); //Glitch filter oFF @A1025. Timming issue.
  #else
    *((volatile unsigned *)(REG_RFIU_RxTimingFineTune+0x1000*RFIunit))=pRfiuPara->RxTimingFineTune;
  #endif
        
   

    *((volatile unsigned *)(REG_RFIU_TxRxOpBaseAddr+0x1000*RFIunit))=(unsigned int)pRfiuPara->TxRxOpBaseAddr;

    *((volatile unsigned *)(REG_RFIU_DRAMADDRLIMIT+0x1000*RFIunit))= DRAM_MEMORY_END>>16;
#if RFIU_SHARE_CTRLBUS_SUPPORT
    OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
#endif
    OS_ENTER_CRITICAL();
  #if( RFIU_DATA_6M_ACK_4M_SUPPORT && (RFIC_SEL == RFIC_A7196_6M) )
    #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
       if(RFIunit == 0)
          rfiuDataRate6M_To_4M(RFIunit);
       else if(RFIunit == 1)
          rfiuDataRate4M_To_6M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
            (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
       rfiuDataRate6M_To_4M(RFIunit);
    #else //RX
       rfiuDataRate4M_To_6M(RFIunit);
    #endif
  #elif( RFIU_DATA_6M_ACK_3M_SUPPORT && (RFIC_SEL==RFIC_A7196_6M) )
    #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
       if(RFIunit == 0)
          rfiuDataRate6M_To_3M(RFIunit);
       else if(RFIunit == 1)
          rfiuDataRate3M_To_6M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
            (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
       rfiuDataRate6M_To_3M(RFIunit);//TX
    #else //RX
       if(gRfiuUnitCntl[RFCh].RXRecvDataUse3M)
           rfiuDataRate6M_To_3M(RFIunit);
       else
           rfiuDataRate3M_To_6M(RFIunit); 
    #endif
  #elif(RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M))
    #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
       if(RFIunit == 0)
          rfiuDataRate4M_To_2M(RFIunit);
       else if(RFIunit == 1)
          rfiuDataRate2M_To_4M(RFIunit);
    #elif ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
            (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
            (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
            (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
       rfiuDataRate4M_To_2M(RFIunit);//TX
    #else //RX
       if(gRfiuUnitCntl[RFCh].RXRecvDataUse3M)
          rfiuDataRate4M_To_2M(RFIunit); 
       else
          rfiuDataRate2M_To_4M(RFIunit); 
    #endif
  #endif
   
  #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
    A7130_RxMode_Start(RFIunit+1);
  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    A7196_RxMode_Start(RFIunit+1);
  #elif(RFIC_SEL==RFIC_NONE_5M)
    RFNONE_RxMode_Start(RFIunit+1);
  #endif
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_TRIG;  
    OS_EXIT_CRITICAL();
#if RFIU_SHARE_CTRLBUS_SUPPORT
    OSSemPost(gRfiuCtrlBusReqSem);
#endif

    return 1;
}

int rfiuWaitForInt_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara)
{
     unsigned int FlagMask;
     unsigned char err;
     unsigned char bitstuffcnt;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
     unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
     //----------------------//
     
     FlagMask = 0x03<<(RFIunit*2);
     OSFlagPend(gRfiuFlagGrp,FlagMask, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_TIMEOUT, &err);
     OSSemPost(gRfiuReqSem[RFIunit]);
     if (err != OS_NO_ERR)
     {
    	DEBUG_RFIU_P2("--->Wait RFI-TX Unit-%d Timeout!\n",RFIunit);
   #if RFIU_SHARE_CTRLBUS_SUPPORT
         OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
   #endif      
      OS_ENTER_CRITICAL();
      #if ( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
         A7130_TxMode_Stop(RFIunit+1);
      #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
         A7196_TxMode_Stop(RFIunit+1);
      #elif(RFIC_SEL==RFIC_NONE_5M)
         RFNONE_TxMode_Stop(RFIunit+1);
      #endif
      OS_EXIT_CRITICAL();
   #if RFIU_SHARE_CTRLBUS_SUPPORT
         OSSemPost(gRfiuCtrlBusReqSem);
   #endif      
        
        RFIU_INT_EN &= ~FlagMask;
        #if DEBUG_TX_TIMEOUT
        #endif
        return 0;    // error
     }
   
#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
#endif 
     OS_ENTER_CRITICAL();
  #if ( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     A7130_TxMode_Stop(RFIunit+1);
  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     A7196_TxMode_Stop(RFIunit+1);
  #elif(RFIC_SEL==RFIC_NONE_5M)
     RFNONE_TxMode_Stop(RFIunit+1);
  #endif
     OS_EXIT_CRITICAL();
#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPost(gRfiuCtrlBusReqSem);
#endif      
  
     RFIU_INT_EN &= ~FlagMask;
     pRfiuPara->BitStuffCnt=(*((volatile unsigned *)(REG_RFIU_BITSTUFF_DECT+0x1000*RFIunit)))>>8;
     return 1;
}

u8 rfiuWaitForInt_Rx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara,int WifiCHsel,u8 *pRSSI2)
{
     unsigned int FlagMask;
     unsigned char err;
     unsigned int pktmap,count,temp;
     int i,j;
     u8 Rssi;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
     //-------//
     
     
     FlagMask = 0x03<<(RFIunit*2);
     OSFlagPend(gRfiuFlagGrp,FlagMask, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_TIMEOUT, &err);
     OSSemPost(gRfiuReqSem[RFIunit]);
 
     
     if (err != OS_NO_ERR)
     {
    	DEBUG_RFIU_P2("--->Wait RFI-RX Unit-%d Timeout!\n",RFIunit);
        RFIU_INT_EN &= ~FlagMask;
        pRfiuPara->TxRxPktNum=0;
            
     #if DEBUG_RX_TIMEOUT   
     #endif
        return 0;    // error
     }
     
     RFIU_INT_EN &= ~FlagMask;
     pRfiuPara->PktMap[0] = *((volatile unsigned *)(REG_RFIU_PacketMap_0+0x1000*RFIunit));
     pRfiuPara->PktMap[1] = *((volatile unsigned *)(REG_RFIU_PacketMap_1+0x1000*RFIunit));
     pRfiuPara->PktMap[2] = *((volatile unsigned *)(REG_RFIU_PacketMap_2+0x1000*RFIunit));
     pRfiuPara->PktMap[3] = *((volatile unsigned *)(REG_RFIU_PacketMap_3+0x1000*RFIunit));
     pRfiuPara->PktMap[4] = *((volatile unsigned *)(REG_RFIU_PacketMap_4+0x1000*RFIunit));
     pRfiuPara->PktMap[5] = *((volatile unsigned *)(REG_RFIU_PacketMap_5+0x1000*RFIunit));
     pRfiuPara->PktMap[6] = *((volatile unsigned *)(REG_RFIU_PacketMap_6+0x1000*RFIunit));
     pRfiuPara->PktMap[7] = *((volatile unsigned *)(REG_RFIU_PacketMap_7+0x1000*RFIunit));

     pRfiuPara->PktMap[8] = *((volatile unsigned *)(REG_RFIU_PacketMap_8+0x1000*RFIunit));
     pRfiuPara->PktMap[9] = *((volatile unsigned *)(REG_RFIU_PacketMap_9+0x1000*RFIunit));
     pRfiuPara->PktMap[10] = *((volatile unsigned *)(REG_RFIU_PacketMap_10+0x1000*RFIunit));
     pRfiuPara->PktMap[11] = *((volatile unsigned *)(REG_RFIU_PacketMap_11+0x1000*RFIunit));
     pRfiuPara->PktMap[12] = *((volatile unsigned *)(REG_RFIU_PacketMap_12+0x1000*RFIunit));
     pRfiuPara->PktMap[13] = *((volatile unsigned *)(REG_RFIU_PacketMap_13+0x1000*RFIunit));
     pRfiuPara->PktMap[14] = *((volatile unsigned *)(REG_RFIU_PacketMap_14+0x1000*RFIunit));
     pRfiuPara->PktMap[15] = *((volatile unsigned *)(REG_RFIU_PacketMap_15+0x1000*RFIunit));

     pRfiuPara->PktMap[16] = *((volatile unsigned *)(REG_RFIU_PacketMap_16+0x1000*RFIunit));
     pRfiuPara->PktMap[17] = *((volatile unsigned *)(REG_RFIU_PacketMap_17+0x1000*RFIunit));
     pRfiuPara->PktMap[18] = *((volatile unsigned *)(REG_RFIU_PacketMap_18+0x1000*RFIunit));
     pRfiuPara->PktMap[19] = *((volatile unsigned *)(REG_RFIU_PacketMap_19+0x1000*RFIunit));
     pRfiuPara->PktMap[20] = *((volatile unsigned *)(REG_RFIU_PacketMap_20+0x1000*RFIunit));
     pRfiuPara->PktMap[21] = *((volatile unsigned *)(REG_RFIU_PacketMap_21+0x1000*RFIunit));
     pRfiuPara->PktMap[22] = *((volatile unsigned *)(REG_RFIU_PacketMap_22+0x1000*RFIunit));
     pRfiuPara->PktMap[23] = *((volatile unsigned *)(REG_RFIU_PacketMap_23+0x1000*RFIunit));

     pRfiuPara->PktMap[24] = *((volatile unsigned *)(REG_RFIU_PacketMap_24+0x1000*RFIunit));
     pRfiuPara->PktMap[25] = *((volatile unsigned *)(REG_RFIU_PacketMap_25+0x1000*RFIunit));
     pRfiuPara->PktMap[26] = *((volatile unsigned *)(REG_RFIU_PacketMap_26+0x1000*RFIunit));
     pRfiuPara->PktMap[27] = *((volatile unsigned *)(REG_RFIU_PacketMap_27+0x1000*RFIunit));
     pRfiuPara->PktMap[28] = *((volatile unsigned *)(REG_RFIU_PacketMap_28+0x1000*RFIunit));
     pRfiuPara->PktMap[29] = *((volatile unsigned *)(REG_RFIU_PacketMap_29+0x1000*RFIunit));
     pRfiuPara->PktMap[30] = *((volatile unsigned *)(REG_RFIU_PacketMap_30+0x1000*RFIunit));
     pRfiuPara->PktMap[31] = *((volatile unsigned *)(REG_RFIU_PacketMap_31+0x1000*RFIunit));

     pRfiuPara->Pkt_Grp_offset[0] = (*((volatile unsigned *)(REG_RFIU_PktGrp_0_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[1] = (*((volatile unsigned *)(REG_RFIU_PktGrp_1_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[2] = (*((volatile unsigned *)(REG_RFIU_PktGrp_2_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[3] = (*((volatile unsigned *)(REG_RFIU_PktGrp_3_Addr+0x1000*RFIunit))) & 0x00ffffff;

     pRfiuPara->Pkt_Grp_offset[4] = (*((volatile unsigned *)(REG_RFIU_PktGrp_4_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[5] = (*((volatile unsigned *)(REG_RFIU_PktGrp_5_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[6] = (*((volatile unsigned *)(REG_RFIU_PktGrp_6_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[7] = (*((volatile unsigned *)(REG_RFIU_PktGrp_7_Addr+0x1000*RFIunit))) & 0x00ffffff;

     pRfiuPara->Pkt_Grp_offset[8] = (*((volatile unsigned *)(REG_RFIU_PktGrp_8_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[9] = (*((volatile unsigned *)(REG_RFIU_PktGrp_9_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[10] = (*((volatile unsigned *)(REG_RFIU_PktGrp_10_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[11] = (*((volatile unsigned *)(REG_RFIU_PktGrp_11_Addr+0x1000*RFIunit))) & 0x00ffffff;

     pRfiuPara->Pkt_Grp_offset[12] = (*((volatile unsigned *)(REG_RFIU_PktGrp_12_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[13] = (*((volatile unsigned *)(REG_RFIU_PktGrp_13_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[14] = (*((volatile unsigned *)(REG_RFIU_PktGrp_14_Addr+0x1000*RFIunit))) & 0x00ffffff;
     pRfiuPara->Pkt_Grp_offset[15] = (*((volatile unsigned *)(REG_RFIU_PktGrp_15_Addr+0x1000*RFIunit))) & 0x00ffffff;


#if(CHIP_OPTION != CHIP_A1013A)
     temp=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFIunit));
     count = (temp >>8) & 0x1ff;

     pRfiuPara->CID_ErrCnt= temp & 0x0ff;
#else
     count =0;
     for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
     {
         pktmap=pRfiuPara->PktMap[i];
         if(pktmap !=0)
         {
             for(j=0;j<32;j++)
             {
                  if(pktmap & 0x01)
                      count ++;
                  pktmap >>=1;
             }
         }
     }

     #if 0
     temp=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFIunit));
     temp = (temp >>8) & 0x1ff;
     if(temp != count)
        DEBUG_RFIU_P2("RX PacketNum error: %d,%d\n",temp,count);
     #endif

 #endif    

     pRfiuPara->UserData_L       = *((volatile unsigned *)(REG_RFIU_SyncWord_L+0x1000*RFIunit)) & RFI_USER_DATA_L_MASK;
     pRfiuPara->UserData_H        = (*((volatile unsigned *)(REG_RFIU_CNTL_1+0x1000*RFIunit)) >> RFI_USER_DATA_H_SHFT) & RFI_USER_DATA_H_MASK;

     pRfiuPara->TxRxPktNum = count;

     for(i=0;i<3000;i++); //delay for RSSI  
#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
#endif    

     OS_ENTER_CRITICAL();
     #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     Rssi=RSSI_measurement_A7130(RFIunit+1);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     Rssi=RSSI_measurement_A7196(RFIunit+1);
     #elif(RFIC_SEL==RFIC_NONE_5M)
     Rssi=RSSI_measurement_RFNONE(RFIunit+1);
     #endif
     
 #if  1 //RF_SCAN_WIFI_CH
     #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
       A7130_CH_sel( RFIunit+1,gRfiuDAT_CH_Table[WifiCHsel]);
       for(i=0;i<3000;i++); //delay for RSSI  
       *pRSSI2=RSSI_measurement_A7130(RFIunit+1);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
       A7196_CH_sel( RFIunit+1,gRfiuDAT_CH_Table[WifiCHsel]);
       for(i=0;i<3000;i++); //delay for RSSI  
       *pRSSI2=RSSI_measurement_A7196(RFIunit+1);
     #elif(RFIC_SEL==RFIC_NONE_5M)
       RFNONE_CH_sel( RFIunit+1,gRfiuDAT_CH_Table[WifiCHsel]);
       for(i=0;i<3000;i++); //delay for RSSI  
       *pRSSI2=RSSI_measurement_RFNONE(RFIunit+1);
     #endif
 #endif
     
   #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     A7130_RxMode_Stop(RFIunit+1);
   #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     A7196_RxMode_Stop(RFIunit+1);
   #elif(RFIC_SEL==RFIC_NONE_5M)
     RFNONE_RxMode_Stop(RFIunit+1);
   #endif
    OS_EXIT_CRITICAL();   
   
#if RFIU_SHARE_CTRLBUS_SUPPORT
     OSSemPost(gRfiuCtrlBusReqSem);
#endif      
   
     return Rssi;
}
















































int rfiuCalBufRemainCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize)
{
    unsigned int count;
    
    if(WritePtr >= ReadPtr)
    {
        count= WritePtr - ReadPtr;
    }
    else
    {
        count= WritePtr+BufSize- ReadPtr;
    }

    if(count >= BufSize-2)
      return -1;
    else
      return count;
}

int rfiuCalVideoDisplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize)
{
    unsigned int count;
    
    if(WritePtr >= ReadPtr)
    {
        count= WritePtr - ReadPtr;
    }
    else
    {
        count= WritePtr+BufSize- ReadPtr;
    }

    if(count >= BufSize-1)
      return -1;
    else
      return count;
}

int rfiuCalAudioplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize)
{
    unsigned int count;
    
    if(WritePtr >= ReadPtr)
    {
        count= WritePtr - ReadPtr;
    }
    else
    {
        count= WritePtr+BufSize- ReadPtr;
    }

    if(count >= BufSize-4)
      return -1;
    else
      return count;
}





int rfiuEncUsrData(DEF_RFIU_USRDATA *pCtrlPara)
{
   int UsrData;
   int FEC;

   FEC= (0& 0x01) |
        ((0 & 0x03)<<1) |
        ((pCtrlPara->RS_sel & 0x03) << 0);

   UsrData = ( (FEC & RFIU_USRDATA_FEC_MASK) << RFIU_USRDATA_FEC_SHFT ) |
             ((pCtrlPara->SeqenceNum & RFIU_USRDATA_SEQNUM_MASK)<<RFIU_USRDATA_SEQNUM_SHFT) |
             ((pCtrlPara->DAT_CH & RFIU_USRDATA_DATACH_MASK)<<RFIU_USRDATA_DATACH_SHFT) |
             ((pCtrlPara->GrpDivs& RFIU_USRDATA_TXGRPDIV_MASK)<<RFIU_USRDATA_TXGRPDIV_SHFT) |
             ((pCtrlPara->GrpShift& RFIU_USRDATA_TXGRPSHF_MASK)<<RFIU_USRDATA_TXGRPSHF_SHFT);
   


    return UsrData;

}

int rfiuDecUsrData(int UsrData,DEF_RFIU_USRDATA *pCtrlPara)
{
    int FEC;

    FEC= (UsrData >> RFIU_USRDATA_FEC_SHFT) & RFIU_USRDATA_FEC_MASK;
    pCtrlPara->Vitbi_en   =  0;
    pCtrlPara->Vitbi_sel  =  0;
    pCtrlPara->RS_sel     = (FEC>>0) & 0x03;
    
    pCtrlPara->SeqenceNum = (UsrData>>RFIU_USRDATA_SEQNUM_SHFT) & RFIU_USRDATA_SEQNUM_MASK;
    pCtrlPara->DAT_CH     = (UsrData>>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
    pCtrlPara->GrpDivs    = (UsrData>>RFIU_USRDATA_TXGRPDIV_SHFT) & RFIU_USRDATA_TXGRPDIV_MASK;
    pCtrlPara->GrpShift   = (UsrData>>RFIU_USRDATA_TXGRPSHF_SHFT) & RFIU_USRDATA_TXGRPSHF_MASK;


    return 1;

}
void rfiu_PAIR_Linit (int RFUnit)
{
    unsigned int  cpu_sr = 0;
	   
    #if (RFI_SELF_TEST_TXRX_PROTOCOL)
        gRfiuUnitCntl[0].OpMode=RFIU_PAIRLint_MODE;
        gRfiuUnitCntl[0].OpState=RFIU_TX_STATE_INIT;        
        gRfiuUnitCntl[1].OpMode=RFIU_PAIRLint_MODE;
        gRfiuUnitCntl[1].OpState=RFIU_RX_STATE_LISTEN;
    #else
       OS_ENTER_CRITICAL();
       gRfiuUnitCntl[RFUnit].OpMode=RFIU_PAIRLint_MODE;
       OS_EXIT_CRITICAL();	  
    #endif
}

void rfiu_PAIR_Stop(int RFUnit)
{
    unsigned int  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    gRfiuUnitCntl[RFUnit].OpMode=RFIU_PAIR_STOP_MODE;
    OS_EXIT_CRITICAL();	  
}


int rfiu_SetRXOpMode_All(u32 mode,u32 level)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;

   //============//
   sprintf(cmd,"MODE %d %d",mode,level);
   for(i=0;i<MAX_RFIU_UNIT;i++)
   {
       if( rfiuRX_CamOnOff_Sta & (0x01<<i) )
       {
          cnt=0;
          RfBusy=1;
          while(RfBusy != 0)
          {
              RfBusy=rfiu_RXCMD_Enc(cmd,i);
              if (cnt >4)
              {
                  DEBUG_UI("rfiu_SetRXOpMode_All Timeout:%d !!!\n",i);
                  break;
              }
              cnt++;
              OSTimeDly(1);
          }          
          if(RfBusy)
            rfiuRX_OpModeCmdRetry[i]=1;
          else
            rfiuRX_OpModeCmdRetry[i]=0;
       }
       else
          rfiuRX_OpModeCmdRetry[i]=0;

   }

   return 1;
}

s32 rfiu_SetRXOpMode_1(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit))== 0 )
   {
      rfiuRX_OpModeCmdRetry[RFUnit]=0;
      return 1;
   }
   
   sprintf(cmd,"MODE %d %d",rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetRXOpMode_1 Timeout:%d !!!\n",RFUnit);
            rfiuRX_OpModeCmdRetry[RFUnit]=1;
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }
   rfiuRX_OpModeCmdRetry[RFUnit]=0;
   return 1;
}


s32 rfiu_SetTXTurbo_On(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"TURBO 1");
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetRXTurbo_OnOff Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }


   return 1;
}

s32 rfiu_SetTXPIRCfg(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int val;
   int cnt,RfBusy;
   //============//
   val=uiCheckPIRSchedule(RFUnit);
   sprintf(cmd,"PIRCFG %d",val);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXPIRCfg Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

s32 rfiu_SetTXReboot(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"BOOT");
   cnt=0;
   RfBusy=1;
   DEBUG_RFIU_P2("--rfiu_SetTXReboot:%d--\n",RFUnit);
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_RFIU_P2("rfiu_SetTXReboot Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }
   return 1;
}

s32 rfiu_SetTXTurbo_Off(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"TURBO 0");
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetRXTurbo_OnOff Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }


   return 1;
}



s32 rfiu_SendTxMdSense(s32 RFUnit)
{
   unsigned char cmd[32];
   int i;
   int cnt,RfBusy;
   //============//
   sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                  MD_SensitivityConfTab[0][0],
                  MD_SensitivityConfTab[0][1],
                  MD_SensitivityConfTab[1][0],
                  MD_SensitivityConfTab[1][1],
                  MD_SensitivityConfTab[2][0],
                  MD_SensitivityConfTab[2][1]
           );
    
   cnt=0;
   RfBusy=1;
   while (RfBusy != 0)
   {
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SendTxMdSense Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(cmd, RFUnit);
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

int rfiu_SetTXPWM(int RFUnit,u8 val)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"SETPWM %d",val);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXPWM Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }


   return 1;
}

int rfiu_SetTXMotorCtrl(int RFUnit,u8 val)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"SETMOTOR %d",val);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXMotorCtrl Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }


   return 1;
}

int rfiu_SetTXMelodyNum(int RFUnit,u8 val)
{
   unsigned char cmd[32];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"SETMELODY %d",val);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXMotorCtrl Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }


   return 1;
}
#if RFIU_RX_WAKEUP_TX_SCHEME
s32 rfiu_SetTXsleepTime(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   
   //============//   
   sprintf(cmd,"RESETSLPTIM  %d",0);
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXVoxCfg Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

s32 rfiu_SetTXDoorBellOff(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   
   //============//   
   sprintf(cmd,"BELLOFF %d",0);
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt > 4)
        {
            DEBUG_UI("rfiu_SetTXDoorBellOff Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

#endif

s32 rfiu_SetTXVoxCfg(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   
   //============//   
   sprintf(cmd,"VOXCFG %d %d",rfiuVoxEna[RFUnit],rfiuVoxThresh[RFUnit]);
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXVoxCfg Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6)
{
   unsigned char cmd[50];
   int i;
   int cnt,RfBusy;
   
   //============//
   if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0)
     return 1;

   
   sprintf(cmd,"SCHED %d %d %d %d %d %d",par1,par2,par3,par4,par5,par6);
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXSchedule Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }
   
   return 1;
}

#elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)

int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6, u8 par7, u8 par8)
{
   unsigned char cmd[50];
   int i;
   int cnt,RfBusy;
   
   //============//
   if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0)
     return 1;

   
   sprintf(cmd,"SCHED %d %d %d %d %d %d %d %d",par1,par2,par3,par4,par5,par6,par7,par8);
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXSchedule Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }
   
   return 1;
}
#endif

int rfiu_SetTXVoxTrig(u8 vol)
{
   unsigned char cmd[16];
   int i;
   int RfBusy;
   //============//   
   sprintf(cmd,"VOXTRIG %d",vol);
   RfBusy=rfiu_TXCMD_Enc(cmd,0);
        
   return 1;
}

int rfiu_SetRXLightTrig(u8 val)
{
   unsigned char cmd[16];
   int i;
   int RfBusy;
   //============//   
   sprintf(cmd,"LIGHTSTAT %d",val);
   RfBusy=rfiu_TXCMD_Enc(cmd,0);
        
   return 1;
}

int rfiu_SetTXInfo(u32 v1,u32 v2,u32 v3,u32 v4)
{
   unsigned char cmd[32];
   int i;
   int RfBusy;
   //============//   
   sprintf(cmd,"TXINFO %d %d %d %d",v1,v2,v3,v4);
   RfBusy=rfiu_TXCMD_Enc(cmd,0);
        
   return 1;
}


s32 rfiu_UpdateTXOthersPara(s32 RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   
   //============//  
   if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0)
     return 1;
   
   sprintf(cmd,"OTHERSPARA");
   cnt=0;
   RfBusy=1;
   
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_UpdateTXOthersPara Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}


int rfiu_TXCMD_Enc (u8 *cmd,int RFUnit)
{
    int data;
    u32 v1,v2,v3,v4;
    //Lucian: 只傳送重覆性的資料(Ex: event trigger),失敗不重送. 

    if(!strncmp((char*)cmd,"VOXTRIG ", strlen("VOXTRIG ")))
    {
       sscanf((char*)cmd, "VOXTRIG %d", &data);
       gRfiuUnitCntl[RFUnit].TXCmd_Type |=RFTXCMD_SEND_DATA;
       gRfiuUnitCntl[RFUnit].TX_CMDPara[0]=RFTXINFO_SET_VOXTRIG; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=data; //volum level
       gRfiuUnitCntl[RFUnit].TXCmd_en=1;
    }
    else if(!strncmp((char*)cmd,"LIGHTSTAT ", strlen("LIGHTSTAT ")))
    {
       sscanf((char*)cmd, "LIGHTSTAT %d", &data);
       gRfiuUnitCntl[RFUnit].TXCmd_Type |=RFTXCMD_SEND_DATA;
       gRfiuUnitCntl[RFUnit].TX_CMDPara[0]=RFTXINFO_SET_LIGHT_STA; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=data; //light status
       gRfiuUnitCntl[RFUnit].TXCmd_en=1;
    }
    else if(!strncmp((char*)cmd,"TXINFO ", strlen("TXINFO ")))
    {
       sscanf((char*)cmd, "TXINFO %d %d %d %d", &v1, &v2, &v3, &v4);
       gRfiuUnitCntl[RFUnit].TXCmd_Type |=RFTXCMD_SEND_DATA;
       gRfiuUnitCntl[RFUnit].TX_CMDPara[0]=RFTXINFO_SET_TXINFO; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=v1; //battery level
       gRfiuUnitCntl[RFUnit].TX_CMDPara[2]=v2; //wifi status
       gRfiuUnitCntl[RFUnit].TX_CMDPara[3]=v3; //
       gRfiuUnitCntl[RFUnit].TX_CMDPara[4]=v4; //
       gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev=v1;
       gRfiuUnitCntl[RFUnit].TXCmd_en=1;
    }

    return 0;
    
}

int rfiu_TXCMD_Dec (int RFUnit)
{
    int par1;
    char CmdType;
    //Lucian: 只傳送重覆性的資料(Ex: event trigger),失敗不重送. 

    CmdType=gRfiuUnitCntl[RFUnit].TX_CMDPara[0];
    
    if(CmdType==RFTXINFO_SET_VOXTRIG)
    {       
       sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_VOXTRIG, RFUnit);
    }
    else if(CmdType==RFTXINFO_SET_LIGHT_STA)
    {       
       sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_LIGHTSTA, RFUnit);
    }
    else if(CmdType==RFTXINFO_SET_TXINFO)
    {  
       if(gRfiuUnitCntl[RFUnit].TX_CMDPara[1] != 0xffffffff)
       {
          gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];
          //DEBUG_RFIU_P2("\nBATLV%d=%d\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev);
       }   

       if(gRfiuUnitCntl[RFUnit].TX_CMDPara[2] != 0xffffffff)
       {
          gRfiuUnitCntl[RFUnit].RFpara.TXWifiStat=gRfiuUnitCntl[RFUnit].TX_CMDPara[2] & 0x0ff;
          gRfiuUnitCntl[RFUnit].RFpara.TXWifiCHNum= (gRfiuUnitCntl[RFUnit].TX_CMDPara[2]>>8) & 0x0ff;
          DEBUG_RFIU_P2("\nWIFI%d=%d,%d\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.TXWifiStat,gRfiuUnitCntl[RFUnit].RFpara.TXWifiCHNum);
       }
    }
    else
    {
        DEBUG_RFIU_P2("Unknow Info..\n");
    }

    return 0;
    
}


int rfiu_CheckTxTaskMode(int RFUnit)
{
#if PWIFI_SUPPORT
    if(gRfiuUnitCntl[RFUnit].OpState & PWIFI_TX_STA_LINK_OK)
    {
        return RFIU_TX_MODE;
    }
    else
    {
        return RFIU_SYNC_MODE;
    }
#else
    return gRfiuUnitCntl[RFUnit].OpMode;
#endif    
}

//Lucian: Fix Tx bug
s32 rfiu_ResendTxMdConfig(s32 RFUnit)
{
   unsigned char cmd[32];
   int i;
   int cnt,RfBusy;
   //============//
   sprintf((char*)cmd,"MDCFG %d %d %d", gRfiuUnitCntl[RFUnit].RFpara.MD_en, gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day, gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night);
   cnt=0;
   RfBusy=1;
   while (RfBusy != 0)
   {
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetRXMdConfig Timeout !!!\r\n");
            gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day=0xff;
            gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night=0xff;
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(cmd, RFUnit);
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}

int rfiuCamSleepCmd(int RFUnit)
{
   int RfBusy;
   int cnt;
   char cmd[16];
   
   DEBUG_RFIU_P2("-----Set Camera-%d Sleep command----\n",RFUnit); 
 #if RFIU_RX_WAKEUP_TX_SCHEME
   if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support == 0)
   {
       gRfiuUnitCntl[RFUnit].SleepTxEn = 0;
       DEBUG_RFIU_P2("--Not Batery Cam, Cannot Sleep--\n");
       return 1;
   }
 #endif
   if( rfiuRX_CamOnOff_Sta & (0x01<<RFUnit) )
   {
       sprintf(cmd,"SLEEP");
       cnt=0;
       RfBusy=1;
       gRfiuUnitCntl[RFUnit].DoSleepCmd=1;
       while(RfBusy != 0)
       {
          RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
          if (cnt >4)
          {
          #if RFIU_RX_WAKEUP_TX_SCHEME
              if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
              {
                  gRfiuUnitCntl[RFUnit].SleepTxEn = 1; //if setting command is fail, 用斷線補救.
              }
          #endif
              gRfiuUnitCntl[RFUnit].DoSleepCmd=0;
              DEBUG_UI("rfiuCamSleepCmd Timeout:%d!!!\n",RFUnit);
              return 0;
          }
          cnt++;
          OSTimeDly(1);
       }  
   #if RFIU_RX_WAKEUP_TX_SCHEME
       if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support == 1)
          gRfiuUnitCntl[RFUnit].SleepTxEn = 1;  
       gRfiuUnitCntl[RFUnit].DoSleepCmd=0;
   #endif
   }

   return 1;
   
}
int rfiu_RXCMD_Enc (u8 *cmd,int RFUnit)
{
    unsigned char i;
    RTC_DATE_TIME   dateTime;
    u32  year, month, day, hour, min, sec;
	u32  reso_W,reso_H;
    u32  Rx_OpMode;
    u32  par1,par2,par3,par4;
    u32  par5,par6,par7,par8,par9;
    u8   err;
    u32  status,CamCnt;
    
	unsigned int  cpu_sr = 0;

    u8  RfLevel[5] = {  RF_P2PVdoQalt_HIGH,
                        RF_P2PVdoQalt_MEDIUM,
                        RF_P2PVdoQalt_LOW,
                        RF_P2PVdoQalt_MEDIUM,
                        RF_P2PVdoQalt_MEDIUM};

    //----------------//

     if(gRfiuUnitCntl[RFUnit].RXCmd_en)
        return 1;

     if(gRfiuUnitCntl[RFUnit].RXCmd_Busy )
        return 2;

  #if RFIU_AUTO_UNPAIR    
     if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0)
        return 0;
  #endif

    if(!strncmp((char*)cmd,"TIME ", strlen("TIME ")))
    {
        sscanf((char*)cmd, "TIME %d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        dateTime.year=year;
        dateTime.month=month;
        dateTime.day=day;
        dateTime.hour=hour;
        dateTime.min=min;
        dateTime.sec=sec;

        //DEBUG_RFIU_P2("Set Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_TIME; // 1;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= dateTime.year; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= dateTime.month;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= dateTime.day;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= dateTime.hour;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= dateTime.min;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= dateTime.sec;
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
	else if(!strncmp((char*)cmd,"RESO ", strlen("RESO ")))
    {
        sscanf((char*)cmd, "RESO %d %d", &reso_W,&reso_H);
        
        DEBUG_RFIU_P2("Set Resolution %d %d\n",reso_W,reso_H);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_RESOLUTION; // 1;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= reso_W/16;  //MB unit
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= reso_H/16;  //MB unit
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
        //OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_CHGRESO<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
        //gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_CHGRESO;

	}
    else if(!strncmp((char*)cmd,"MODE ", strlen("MODE ")))
    {
        sscanf((char*)cmd, "MODE %d %d", &Rx_OpMode,&par2);
        
        //DEBUG_RFIU_P2("Set TX-%d RXOpMode=%d,Level=%d\n",RFUnit,Rx_OpMode,par2);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_OPMODE;
        if(RFUnit == sysRFRxInMainCHsel)
           Rx_OpMode |=RFIU_RX_OPMODE_MAINCH;

   #if AUDIO_PCM_8BIT_SIGN
        Rx_OpMode |=RFIU_RX_OPMODE_PCM_SIGN;
   #endif
   
   #if THUMBNAIL_PREVIEW_ENA
        Rx_OpMode |=RFIU_RX_OPMODE_QUAD;
   #endif
   
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= Rx_OpMode;  //MB unit
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=RfLevel[par2-1];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= rfiuRX_CamOnOff_Num;
   #if (RFI_TEST_4TX_2RX_PROTOCOL )  
        if( (RFUnit & 0x01)==0 )
        {
            if( (rfiuRX_CamOnOff_Sta & 0x5)== 0x5 )
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=2;
            else
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
        }
        else if( (RFUnit & 0x01)==1 )
        {
            if( ( rfiuRX_CamOnOff_Sta & 0xa)== 0xa )
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=2;
            else
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
        }
        else
          gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
        
   #elif (RFI_TEST_8TX_2RX_PROTOCOL )  
        if( (RFUnit & 0x01)==0 )
            status=rfiuRX_CamOnOff_Sta & 0x55;
        else
            status=rfiuRX_CamOnOff_Sta & 0xaa;
        CamCnt=0;
        for(i=0;i<MAX_RFIU_UNIT;i++)
        {
            if(status & 0x01)
               CamCnt ++;
            status >>=1;
        }
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=CamCnt;
   #elif RFI_TEST_2x_RX_PROTOCOL_B1   
        status=rfiuRX_CamOnOff_Sta;
        CamCnt=0;
        for(i=0;i<2;i++)
        {
            if(status & 0x01)
               CamCnt ++;
            status >>=1;
        }
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=CamCnt;
   #elif (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        status=rfiuRX_CamOnOff_Sta;
        CamCnt=0;
        for(i=0;i<MAX_RFIU_UNIT;i++)
        {
            if(status & 0x01)
               CamCnt ++;
            status >>=1;
        }
     #if RFIU_RX_SHOW_ONLY
        if(Rx_OpMode & RFIU_RX_OPMODE_QUAD)
          gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=CamCnt;
        else
          gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;  
     #else
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=CamCnt;
     #endif
   #else
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
   #endif
   
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();

    }
    else if(!strncmp((char*)cmd,"BRIT ", strlen("BRIT ")))
    {
        sscanf((char*)cmd, "BRIT %d", &par1);
        
        //DEBUG_RFIU_P2("Set TxBritLv %d\n",par1);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_BRIGHTNESS;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1;  //MB unit
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();

    }
    else if(!strncmp((char*)cmd,"FLIC ", strlen("FLIC ")))
    {
        sscanf((char*)cmd, "FLIC %d", &par1);
        
        //DEBUG_RFIU_P2("Set TxFlick %d\n",par1);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_FLICKER;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1;  //MB unit
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();

    }
    else if(!strncmp((char*)cmd,"ZOOM ", strlen("ZOOM ")))
    {
        sscanf((char*)cmd, "ZOOM %d %d %d", &par1,&par2,&par3);
        
        //DEBUG_RFIU_P2("Set Zoom:Ena=%d,Xpos=%d, Ypos=%d\n",par1,par2,par3);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_ZOOM;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 & 0x01; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2>>3; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3>>3; 
                
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"MDCFG ", strlen("MDCFG ")))
    {
        sscanf((char*)cmd, "MDCFG %d %d %d", &par1,&par2,&par3); //Mask Area: par4,par5,par6
        
        //DEBUG_RFIU_P2("Set MDCFG: Ena=%d, Level_Day=%d, Level_Night=%d\n",par1,par2,par3);
            
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_MDCFG;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= (par1 & 0x03) | ((par2 & 0x07)<<2) | ((par3 & 0x07)<<5);  //bit0,1: ON/OFF, bit4~2: Level_Day, bit7~5: Level_Night
   #if 1 //Night mode sensitivity//
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= MD_SensitivityConfTab_Night[0][0];  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= MD_SensitivityConfTab_Night[0][1]; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= MD_SensitivityConfTab_Night[1][0]; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= MD_SensitivityConfTab_Night[1][1]; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= MD_SensitivityConfTab_Night[2][0]; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= MD_SensitivityConfTab_Night[2][1]; 
   #endif

        gRfiuUnitCntl[RFUnit].RFpara.MD_en         =par1 & 0x03;
        gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day  =par2;
        gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night=par3;
        gRfiuUnitCntl[RFUnit].RFpara.MD_Trig       =0;
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"PIRCFG ", strlen("PIRCFG ")))
    {
        sscanf((char*)cmd, "PIRCFG %d", &par1);
        
        //DEBUG_RFIU_P2("Set PIRCFG: Ena=%d\n",par1);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_PIRCFG;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 ;  //bit0: ON/OFF
   #if 1 //PIR sensitivity, INDOOR and OUTDOOR
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= PIR_SensitivityConfTab_indoor[0];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= PIR_SensitivityConfTab_indoor[1];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= PIR_SensitivityConfTab_indoor[2];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= PIR_SensitivityConfTab_outdoor[0];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= PIR_SensitivityConfTab_outdoor[1]; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= PIR_SensitivityConfTab_outdoor[2];
   #endif        

        gRfiuUnitCntl[RFUnit].RFpara.PIR_en=par1;
    #if RFIU_RX_WAKEUP_TX_SCHEME
        uiPIRScheduleOnOff[RFUnit]=par1;
    #endif    
        gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"TXOSD ", strlen("TXOSD ")))
    {
        sscanf((char*)cmd, "TXOSD %d",&par1);
        
        //DEBUG_RFIU_P2("Set TXOSD: Ena=%d\n",par1);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_TXOSD;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1;  //bit0: ON/OFF
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"PTZ485 ", strlen("PTZ485 ")))
    {
        //sscanf((char*)cmd, "PTZ485 %c %c %c %c %c %c %c",&par1,&par2,&par3,&par4,&par5,&par6,&par7);
        par1=cmd[7] & 0x0ff;
        par2=cmd[8] & 0x0ff;
        par3=cmd[9] & 0x0ff;
        par4=cmd[10]& 0x0ff;
        par5=cmd[11]& 0x0ff;
        par6=cmd[12]& 0x0ff;
        par7=cmd[13]& 0x0ff;
        par8=cmd[14]& 0x0ff;
        
        DEBUG_RFIU_P2("PTZ: %d %d %d %d %d %d %d\n",par1,par2,par3,par4,par5,par6,par7);        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_PTZ485;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 & 0x0ff; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2 & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3 & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par4 & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par5 & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par6 & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par7 & 0x0ff;
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SETGPO ", strlen("SETGPO ")))
    {
        sscanf((char*)cmd, "SETGPO %d", &par1);
        
        //DEBUG_RFIU_P2("Set SETGPO:%d\n",par1);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_GPO;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 ;  //bit0: ON/OFF

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"OTHERSPARA", strlen("OTHERSPARA")))
    {                
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_OTHERSPARAS;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]  = ((gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampType & 0xf)<<4) | (gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn & 0xf);        //Time-stamp ON/OFF: bit 0,others reserved.
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]  = gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamBRSel & 0xf;

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"MASKAREA_VGA ", strlen("MASKAREA_VGA ")))
    {
        sscanf((char*)cmd, "MASKAREA_VGA %d %d %d %d %d %d %d %d %d", 
               &par1,&par2,&par3,&par4,&par5,&par6,&par7,&par8,&par9);
        DEBUG_RFIU_P2("Set MASKAREA_VGA:%x %x %x %x %x %x %x %x %x\n",par1,par2,par3,par4,par5,par6,par7,par8,par9);        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_MASKAREA_VGA;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par4; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par5; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par6; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par7; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]= par8; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]= par9; 

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"MASKAREA_HD ", strlen("MASKAREA_HD ")))
    {
        sscanf((char*)cmd, "MASKAREA_HD %d %d %d %d %d %d %d %d %d", 
               &par1,&par2,&par3,&par4,&par5,&par6,&par7,&par8,&par9);
        //DEBUG_RFIU_P2("Set MASKAREA_HD:%x %x %x %x %x %x %x %x %x\n",par1,par2,par3,par4,par5,par6,par7,par8,par9);        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_MASKAREA_HD;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par4; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par5; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par6; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par7; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]= par8; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]= par9; 

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SLEEP", strlen("SLEEP")))
    {
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_SLEEP;

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"TURBO", strlen("TURBO")))
    {
        sscanf((char*)cmd, "TURBO %d",&par2);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_PWRTURBO;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2;

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"BELLOFF", strlen("BELLOFF")))
    {
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_DOORBELLOFF;

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"MDSENS ", strlen("MDSENS ")))
    {
        sscanf((char*)cmd, "MDSENS %d %d %d %d %d %d", 
               &par1,&par2,&par3,&par4,&par5,&par6);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_MDSENSTAB;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par3; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par4; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par5; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par6; 

        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SETPWM ", strlen("SETPWM ")))
    {
        sscanf((char*)cmd, "SETPWM %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_PWM;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SETMOTOR ", strlen("SETMOTOR ")))
    {
        sscanf((char*)cmd, "SETMOTOR %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_MOTORCTL;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SETMELODY ", strlen("SETMELODY ")))
    {
        sscanf((char*)cmd, "SETMELODY %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_MELODYNUM;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"VOXCFG ", strlen("VOXCFG ")))
    {
        sscanf((char*)cmd, "VOXCFG %d %d", &par1,&par2);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_VOXCFG;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2; 
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SUBSTR ", strlen("SUBSTR ")))
    {
        sscanf((char*)cmd, "SUBSTR %d ", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SUBSTR_EN;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SATU ", strlen("SATU ")))
    {
        sscanf((char*)cmd, "SATU %d %d %d %d", &par1,&par2,&par3,&par4);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SAT_CON_SHP;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par3;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par4; 
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"VOLUM ", strlen("VOLUM ")))
    {
        sscanf((char*)cmd, "VOLUM %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_VOLUME;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;         
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"LIGHT ", strlen("LIGHT ")))
    {
        sscanf((char*)cmd, "LIGHT %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_LIGHTONOFF;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;         
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SCHED ", strlen("SCHED ")))
    {
        sscanf((char*)cmd, "SCHED %d %d %d %d %d %d %d %d", &par1,&par2,&par3,&par4,&par5,&par6,&par7,&par8);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SCHED;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par3;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par4; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par5;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par6; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]= par7; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]= par8; 
        
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SNAP ", strlen("SNAP ")))
    {
        sscanf((char*)cmd, "SNAP %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SNAPSHOT;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"BOOT", strlen("BOOT")))
    {
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_REBOOT;
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"FBAPPSTAT ", strlen("FBAPPSTAT ")))
    {
        sscanf((char*)cmd, "FBAPPSTAT %d %d", &par1,&par2);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_FBAPPSTA;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2;      
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"RESETSLPTIM ", strlen("RESETSLPTIM ")))
    {
        sscanf((char*)cmd, "RESETSLPTIM %d",&par1);
        DEBUG_GREEN("RESETSLPTIM Enc\n");
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SLEEPRECNT;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    
    
    return 0;
}


#if RFIU_TEST
void rfiu_Start(void)
{

}
#else
void rfiu_Start(void)
{
    u16 i;
    #if RF_TX_OPTIMIZE
    s32             TimeOffset;
    #endif


    #if RFI_TEST_RX_PROTOCOL_B1
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 
        
    #elif RFI_TEST_RX_PROTOCOL_B2
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1); 
	
    #elif RFI_TEST_RXRX_PROTOCOL_B1B2    
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 

        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);

      #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) )
        //-----TV encoder init: must have 656 clock-----//
    	subTV_Preview(640,480);
        GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN );

        
        #if(TV_ENCODER == BIT1201G)
            i2cInit_BIT1201G();            
        #endif	  
        
        tvBT656CONF |= 0x00040000;
        SYS_ANA_TEST2 = (SYS_ANA_TEST2 & (~0x018)) | 0x008; //TV1 to DAC1; TV2 to DAC2 .
	    IsIduOsdEnable=0;
      #else
      #endif
    #elif RFI_TEST_2x_RX_PROTOCOL_B1   
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);
        OSTimeDly(1);

        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);

      #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) )
        //-----TV encoder init: must have 656 clock-----//
    	subTV_Preview(640,480);
        GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN );

        
        #if(TV_ENCODER == BIT1201G)
            i2cInit_BIT1201G();            
        #endif	  
        
        tvBT656CONF |= 0x00040000;
        SYS_ANA_TEST2 = (SYS_ANA_TEST2 & (~0x018)) | 0x008; //TV1 to DAC1; TV2 to DAC2 .
	    IsIduOsdEnable=0;
      #else
      #endif


    #elif RFI_TEST_4x_RX_PROTOCOL_B1    
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_3, RFIU_TASK_STACK_UNIT3, RFIU_TASK_PRIORITY_UNIT3);
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_2, RFIU_TASK_STACK_UNIT2, RFIU_TASK_PRIORITY_UNIT2);  
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);    
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 

    #elif RFI_TEST_4TX_2RX_PROTOCOL
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_3, RFIU_TASK_STACK_UNIT3, RFIU_TASK_PRIORITY_UNIT3);
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_2, RFIU_TASK_STACK_UNIT2, RFIU_TASK_PRIORITY_UNIT2);  
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);    
        OSTimeDly(1);
        
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 
        
    #elif (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
        //--RX7--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_7, RFIU_TASK_STACK_UNIT7, RFIU_TASK_PRIORITY_UNIT7);
        OSTimeDly(1);
        
        //--RX6--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_6, RFIU_TASK_STACK_UNIT6, RFIU_TASK_PRIORITY_UNIT6);  
        OSTimeDly(1);
        
        //--RX5--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_5, RFIU_TASK_STACK_UNIT5, RFIU_TASK_PRIORITY_UNIT5);    
        OSTimeDly(1);
        
        //--RX4--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_4, RFIU_TASK_STACK_UNIT4, RFIU_TASK_PRIORITY_UNIT4); 
        OSTimeDly(1);

        //--RX3--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_3, RFIU_TASK_STACK_UNIT3, RFIU_TASK_PRIORITY_UNIT3);
        OSTimeDly(1);
        
        //--RX2--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_2, RFIU_TASK_STACK_UNIT2, RFIU_TASK_PRIORITY_UNIT2);  
        OSTimeDly(1);
        
        //--RX1--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);    
        OSTimeDly(1);
        
        //--RX0--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 

        
    #elif (RFI_TEST_TX_PROTOCOL_B1 && RF_TX_OPTIMIZE)

    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        //move this setting just after siu-end in sysPreviewInit
    #else
        //--TX--//
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
        sysReady2CaptureVideo   = 1;
    #endif

        if(gRfiu_TX_Sta[0]==RFI_TX_TASK_NONE)
        {
           //DEBUG_ASF("====RFIU1_TASK Create====\n");
           OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
           gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;
        }
      #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
	    timerCountRead(2, (u32*) &TimeOffset);
    	IISTimeOffset   = TimeOffset >> 8;
	  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
		timerCountRead(1, (u32*) &TimeOffset);
	    IISTimeOffset   = TimeOffset / 100;
	  #endif


      #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

      #else
        iisResumeTask();
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4ResumeTask();
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskResume();
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgResumeTask();
        #endif
      #endif    

    #elif (RFI_TEST_TX_PROTOCOL_B2 && RF_TX_OPTIMIZE)
        //--TX--//
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
        sysReady2CaptureVideo   = 1;
        
        if(gRfiu_TX_Sta[1]==RFI_TX_TASK_NONE)
        {
           //DEBUG_ASF("====RFIU2_TASK Create====\n");
           OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);
           gRfiu_TX_Sta[1]=RFI_TX_TASK_RUNNING;
        }

        #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
    	    timerCountRead(2, (u32*) &TimeOffset);
        	IISTimeOffset   = TimeOffset >> 8;
    	#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
    		timerCountRead(1, (u32*) &TimeOffset);
    	    IISTimeOffset   = TimeOffset / 100;
    	#endif

      #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

      #else
        iisResumeTask();
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4ResumeTask();
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskResume();
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgResumeTask();
        #endif
      #endif
    #endif    

}
#endif


void rfiu_End(void)
{
    int i,j;
    u8 err;

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    #if RFI_TEST_TX_PROTOCOL_B1
      #if RFIU_RX_AUDIO_RETURN    
          if(guiIISCh0PlayDMAId != 0xFF)
          {
              marsDMAClose(guiIISCh0PlayDMAId);
              guiIISCh0PlayDMAId = 0xFF;
          }
      #endif    
        gRfiuUnitCntl[0].TX_Task_Stop=1;
        gRfiuUnitCntl[0].TX_Wrap_Stop=1;
        OSTimeDly(4);
        //--TX1--//
        if(gRfiu_WrapEnc_Sta[0] == RFI_WRAPENC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
        }
        gRfiuUnitCntl[0].TX_Wrap_Stop=0;

        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT0); 
        gRfiuUnitCntl[0].TX_Task_Stop=0;

    #elif RFI_TEST_TX_PROTOCOL_B2
      #if RFIU_RX_AUDIO_RETURN    
          if(guiIISCh0PlayDMAId != 0xFF)
          {
              marsDMAClose(guiIISCh0PlayDMAId);
              guiIISCh0PlayDMAId = 0xFF;
          }
      #endif  
      
        gRfiuUnitCntl[1].TX_Task_Stop=1;
        gRfiuUnitCntl[1].TX_Wrap_Stop=1;
        OSTimeDly(4);
        //--TX2--//
        if(gRfiu_WrapEnc_Sta[1] == RFI_WRAPENC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
        }
        gRfiuUnitCntl[1].TX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT1); 
        gRfiuUnitCntl[1].TX_Task_Stop=0;
        
    #elif RFI_TEST_RX_PROTOCOL_B1
        gRfiuUnitCntl[0].RX_Task_Stop=1;
        gRfiuUnitCntl[0].RX_Wrap_Stop=1;
        gRfiuUnitCntl[0].RX_MpegDec_Stop=1;
        OSTimeDly(40);
        //--RX1--//
        if(gRfiu_MpegDec_Sta[0] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
        }
        gRfiuUnitCntl[0].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[0] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
        }
        gRfiuUnitCntl[0].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0); 
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT0); 
        gRfiuUnitCntl[0].RX_Task_Stop=0;
        
    #elif RFI_TEST_RX_PROTOCOL_B2
        gRfiuUnitCntl[1].RX_Task_Stop=1;
        gRfiuUnitCntl[1].RX_Wrap_Stop=1;
        gRfiuUnitCntl[1].RX_MpegDec_Stop=1;
        OSTimeDly(40);
        //--RX2--//
        if(gRfiu_MpegDec_Sta[1] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
        }
        gRfiuUnitCntl[1].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[1] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
        }
        gRfiuUnitCntl[1].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT1); 
        gRfiuUnitCntl[1].RX_Task_Stop=0;
        
    #elif (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1)   
        for(i=0;i<2;i++)
        {
           gRfiuUnitCntl[i].RX_Task_Stop=1;
           gRfiuUnitCntl[i].RX_Wrap_Stop=1;
           gRfiuUnitCntl[i].RX_MpegDec_Stop=1;
        }
        OSTimeDly(40);

        //--RX1--//
        if(gRfiu_MpegDec_Sta[0] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
        }
        gRfiuUnitCntl[0].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[0] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
        }
        gRfiuUnitCntl[0].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT0); 
        gRfiuUnitCntl[0].RX_Task_Stop=0;

        //--RX2--//
        if(gRfiu_MpegDec_Sta[1] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
        }
        gRfiuUnitCntl[1].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[1] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
        }
        gRfiuUnitCntl[1].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT1); 
        gRfiuUnitCntl[1].RX_Task_Stop=0;
            
    #elif (RFI_TEST_4x_RX_PROTOCOL_B1  || RFI_TEST_4TX_2RX_PROTOCOL)   
        for(i=0;i<MAX_RFIU_UNIT;i++)
        {
           gRfiuUnitCntl[i].RX_Task_Stop=1;
           gRfiuUnitCntl[i].RX_Wrap_Stop=1;
           gRfiuUnitCntl[i].RX_MpegDec_Stop=1;
        }
        OSTimeDly(40);  //delay 2 sec,強迫tx 斷線

        //--RX1--//
        if(gRfiu_MpegDec_Sta[0] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT0\n");
        }
        gRfiuUnitCntl[0].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[0] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT0\n");
        }
        gRfiuUnitCntl[0].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT0); 
        gRfiuUnitCntl[0].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT0\n");

        //--RX2--//
        if(gRfiu_MpegDec_Sta[1] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT1\n");
        }
        gRfiuUnitCntl[1].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[1] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT1\n");
        }
        gRfiuUnitCntl[1].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT1); 
        gRfiuUnitCntl[1].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT1\n");

        //--RX3--//
        if(gRfiu_MpegDec_Sta[2] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT2\n");
        }
        gRfiuUnitCntl[2].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[2] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT2\n");
        }
        gRfiuUnitCntl[2].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT2);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT2); 
        gRfiuUnitCntl[2].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT2\n");
        
        //--RX4--//
        if(gRfiu_MpegDec_Sta[3] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT3\n");
        }
        gRfiuUnitCntl[3].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[3] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT3\n");
        }
        gRfiuUnitCntl[3].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT3);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT3); 
        gRfiuUnitCntl[3].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT3\n");
        
    #elif (RFI_TEST_8TX_1RX_PROTOCOL  || RFI_TEST_8TX_2RX_PROTOCOL)   
        for(i=0;i<MAX_RFIU_UNIT;i++)
        {
           gRfiuUnitCntl[i].RX_Task_Stop=1;
           gRfiuUnitCntl[i].RX_Wrap_Stop=1;
           gRfiuUnitCntl[i].RX_MpegDec_Stop=1;
        }
        OSTimeDly(40);  //delay 2 sec,強迫tx 斷線

        //--RX0--//
        if(gRfiu_MpegDec_Sta[0] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT0\n");
        }
        gRfiuUnitCntl[0].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[0] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT0\n");
        }
        gRfiuUnitCntl[0].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT0); 
        gRfiuUnitCntl[0].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT0\n");

        //--RX1--//
        if(gRfiu_MpegDec_Sta[1] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT1\n");
        }
        gRfiuUnitCntl[1].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[1] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT1\n");
        }
        gRfiuUnitCntl[1].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT1); 
        gRfiuUnitCntl[1].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT1\n");

        //--RX2--//
        if(gRfiu_MpegDec_Sta[2] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT2\n");
        }
        gRfiuUnitCntl[2].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[2] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT2\n");
        }
        gRfiuUnitCntl[2].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT2);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT2); 
        gRfiuUnitCntl[2].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT2\n");
        
        //--RX3--//
        if(gRfiu_MpegDec_Sta[3] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT3\n");
        }
        gRfiuUnitCntl[3].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[3] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT3\n");
        }
        gRfiuUnitCntl[3].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT3);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT3); 
        gRfiuUnitCntl[3].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT3\n");
    
        //--RX4--//
        if(gRfiu_MpegDec_Sta[4] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT4);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT4);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT4\n");
        }
        gRfiuUnitCntl[4].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[4] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT4);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT4);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT4\n");
        }
        gRfiuUnitCntl[4].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT4);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT4); 
        gRfiuUnitCntl[4].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT4\n");
        //--RX5--//
        if(gRfiu_MpegDec_Sta[5] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT5);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT5);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT5\n");
        }
        gRfiuUnitCntl[5].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[5] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT5);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT5);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT5\n");
        }
        gRfiuUnitCntl[5].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT5);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT5); 
        gRfiuUnitCntl[5].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT5\n");
        //--RX6--//
        if(gRfiu_MpegDec_Sta[6] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT6);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT6);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT6\n");
        }
        gRfiuUnitCntl[6].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[6] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT6);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT6);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT6\n");
        }
        gRfiuUnitCntl[6].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT6);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT6); 
        gRfiuUnitCntl[6].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT3\n");
        //--RX7--//
        if(gRfiu_MpegDec_Sta[7] == RFI_MPEGDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT7);
           OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT7);
           DEBUG_RFIU_P2("Del RFIU_DEC_TASK_PRIORITY_UNIT7\n");
        }
        gRfiuUnitCntl[7].RX_MpegDec_Stop=0;
        
        if(gRfiu_WrapDec_Sta[7] == RFI_WRAPDEC_TASK_RUNNING)
        {
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT7);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT7);
           DEBUG_RFIU_P2("Del RFIU_WRAP_TASK_PRIORITY_UNIT7\n");
        }
        gRfiuUnitCntl[7].RX_Wrap_Stop=0;
        
        OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT7);
        OSTaskDel(RFIU_TASK_PRIORITY_UNIT7); 
        gRfiuUnitCntl[7].RX_Task_Stop=0;
        
        DEBUG_RFIU_P2("Del RFIU_TASK_PRIORITY_UNIT7\n");
    #endif

    //------------------------------------------------------//
    gRfiuFlagGrp=OSFlagDel(gRfiuFlagGrp, OS_DEL_ALWAYS, &err);
    gRfiu_nTx1RSwFlagGrp=OSFlagDel(gRfiu_nTx1RSwFlagGrp, OS_DEL_ALWAYS, &err);
    gRfiuStateFlagGrp=OSFlagDel(gRfiuStateFlagGrp, OS_DEL_ALWAYS, &err);

    gRfiuFlagGrp = OSFlagCreate(0x00000000, &err);
    gRfiu_nTx1RSwFlagGrp=OSFlagCreate(0x00000000, &err);
    gRfiuStateFlagGrp = OSFlagCreate(0x00000000, &err);

    sysRFRxInMainCHsel = 0;
	
    rfiuRxMainVideoPlayStart=0;
    rfiuRxMainAudioPlayStart=0;
	rfiuRxSub1VideoPlayStart=0;
	
    rfiuMainVideoTime=0; 
    rfiuMainVideoTime_frac=0;
	
	rfiuSub1VideoTime=0; 
	rfiuSub1VideoTime_frac=0;
	
    rfiuMainAudioTime=0;
    rfiuMainAudioTime_frac=0;

    //rfiuRX_OpMode=RFIU_RX_OPMODE_NORMAL;
    
    for(j=0;j<DISPLAY_BUF_NUM;j++)
         rfiuMainVideoPresentTime[j]=0;

    //Lucian: 假設audio format為unsigned 8-bit PCM,若否需修正
    for(i=0;i<RFI_AUIDIO_SILENCE_SIZE;i++)
        rfiuAudioZeroBuf[i]=0x80;
    
    SYSReset(SYS_RSTCTL_RF1013_RST);
      
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       memset(&gRfiuParm_Tx[i],0,sizeof(DEF_REGRFIU_CFG));
       memset(&gRfiuParm_Rx[i],0,sizeof(DEF_REGRFIU_CFG));
       memset(&gRfiuUnitCntl[i],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));


       for(j=0;j<RFI_DAT_CH_MAX;j++)
         rfiuRSSI_CH_Avg[i][j]=0;

       gRfiu_Op_Sta[i]      = RFIU_OP_INIT;
       gRfiu_WrapDec_Sta[i] = RFI_WRAPDEC_TASK_NONE;
       gRfiu_MpegDec_Sta[i] = RFI_MPEGDEC_TASK_NONE;
       gRfiu_RX_Sta[i]      = RFI_RX_TASK_NONE;
       
       gRfiu_WrapEnc_Sta[i] = RFI_WRAPENC_TASK_NONE;
       gRfiu_MpegEnc_Sta[i] = RFI_MPEGENC_TASK_NONE;
       gRfiu_TX_Sta[i]      = RFI_TX_TASK_NONE;

       rfiuRxVideoBufMngWriteIdx[i]=0;
       rfiuRxIIsSounBufMngWriteIdx[i]=0;

	   rfiuVideoBufFill_idx[i]=0;
	   rfiuVideoBufPlay_idx[i]=0;

	   rfiuAudioBufPlay_idx[i]=0;
	   rfiuAudioBufFill_idx[i]=0;

	   rfiuVideoTimeBase[i]=0;
       gRfiuTimer_offset[i]=0;

       gRfiuFCC247ChUsed[i][0]=-1;
       gRfiuFCC247ChUsed[i][1]=-1;


       OSSemSet(gRfiuReqSem[i], 1, &err);
       OSSemSet(gRfiuAVCmpSemEvt[i], 0, &err);
    }

    OSSemPost(gRfiuSWReadySemEvt);
}

unsigned int rfiu_GetBitRate(int RFUnit)
{
   return gRfiuUnitCntl[RFUnit].BitRate;
}

unsigned int rfiu_TxGetCamNum(int RFUnit)
{
   return rfiuCmdRxStatus[RFUnit].CamNum;
}

s32 rfiuSetGPO_TX(s32 setting)
{
#if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)|| (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||(SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) 
    DEBUG_RFIU_P2("rfiuSetGPO_TX:%d\n",setting);

#if((HW_BOARD_OPTION  == MR9100_TX_MAYON_MWL612) || (HW_BOARD_OPTION  == MR9160_TX_MAYON_MWL613))
#if UI_LIGHT_SUPPORT
    switch(setting)
    {
        case 0:
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            uiInManualLight = 0;
            uiInScheduleLight = 2;
            break;
           
        case 1:
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
            uiInScheduleLight = 2;
            if (uiInManualLight == 0)
            {
                uiInManualLight = UI_TRIGGER_TIME;
                if (uiCurrentLightTime != 0)
                    uiCurrentLightTime = 0;
            }
            break;

        case 2:/*SAVE LIGHT ON */
            uiInManualLight = 0;
            uiCurrentLightTime = 0;
            iconflag[UI_MENU_SETIDX_LIGHT_ONOFF] = 0;
            sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
            break;
            
        case 3:/*SAVE LIGHT OFF */
            iconflag[UI_MENU_SETIDX_LIGHT_ONOFF] = 1;
            sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
            break;
            
       case 9:/*Trigger on*/
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
            if (uiCurrentLightTime == 0)
            {
                uiCurrentLightTime = UI_TRIGGER_TIME;
            }
            break;     

    }
#endif

#if UI_CAMERA_ALARM_SUPPORT
    switch(setting)
    {
        case 4:
            gpioSetLevel(GPIO_GROUP_ALARM, GPIO_BIT_ALARM, 0);
            uiInManualAlarm = 0;
            if (uiInScheduleAlarm != 1)
                uiInScheduleAlarm = 2;
            break;
           
        case 5:
            gpioSetLevel(GPIO_GROUP_ALARM, GPIO_BIT_ALARM, 1);
            if (uiInScheduleAlarm != 1)
                uiInScheduleAlarm = 2;
            if (uiInManualAlarm == 0)
            {
                uiInManualAlarm = UI_TRIGGER_TIME;
                if (uiCurrentAlarmTime != 0)
                    uiCurrentAlarmTime = 0;
            }
            break;
            
        case 6:/*SAVE ALREM ON */
            //uiInManualAlarm = 0;
            //uiCurrentAlarmTime = 0;        
            iconflag[UI_MENU_SETIDX_CA_ONOFF] = 0;
            sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
            break;
            
        case 7:/*SAVE ALREM OFF */
            iconflag[UI_MENU_SETIDX_CA_ONOFF] = 1;
            sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
            break;
            
       case 8:/*Trigger on*/
            gpioSetLevel(GPIO_GROUP_ALARM, GPIO_BIT_ALARM, 1);
            if (uiCurrentAlarmTime == 0)
            {
                uiCurrentAlarmTime = UI_TRIGGER_TIME;
            }
            break;     
    }
#endif

#else 

#if (UI_LIGHT_SUPPORT == 1)
    switch(setting)
    {
        case 0:
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            uiInManualLight = 0;
            uiSetTriggerDimmer = 0;
            uiInScheduleLight = 2;
            break;
           
        case 1:
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
            uiInScheduleLight = 2;
            if (uiInManualLight == 0)
            {
                uiInManualLight = 43201;
                if (uiCurrentLightTime != 0)
                    uiCurrentLightTime = 0;
                uiSetTriggerDimmer = UI_TRIGGER_DIMMER_TIME;
                uiMenuAction(UI_MENU_SETIDX_LIGHT_DIMMER);

            }
            break;

        case 2:
            if (uiInManualLight == 1)
            {
                gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
            }
            break;

       case 3:
            if (uiInManualLight == 0)
            {
                gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
            }
            break;
            
        
    }
#endif

#if ENABLE_DOOR_BELL
    switch(setting)
    {
        case 4:
            gpioSetLevel(GPIO_GROUP_LOCKER, GPIO_BIT_LOCKER, 1);
            uiDoorCount = 30;
            break;
    }
#endif

#endif

#endif /*end of #if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||(SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) */
}

s32 rfiuSetPWM_TX(s32 setting)
{
    DEBUG_RFIU_P2("rfiuSetPWM_TX:%d\n",setting);
#if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) 
#if (UI_LIGHT_SUPPORT == 1)
    if (setting & 0x80)  /*dimmer*/
    {
        uiMenuSet_Light_Dimmer((setting & 0x60)>>5);
    }
    else if (setting & 0x10)    /*duration*/
    {
        uiMenuSet_Light_Duration((setting & 0x0E)>>1);
    }
    else    /*on / off*/
    {
        if (setting & 0x01)
        {
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, GPIO_LEVEL_HI);
            uiInScheduleLight = 1;
        }
        else
        {
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, GPIO_LEVEL_LO);
            uiInScheduleLight = 0;
        }
    }

#endif
#endif /*#if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM)|| (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||(SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) */
}

s32 rfiuSetMotorCtrl_TX(s32 setting) // [3:0]=H, [7:4]=V
{
   DEBUG_RFIU_P2("rfiuSetMotorCtrl_TX:0x%02x\n",setting);
#if MOTOR_EN
   MotorStatusH = setting & 0x0f;
   MotorStatusV = setting >> 4;
#endif
}

s32 rfiuSetMelodyNum_TX(s32 setting)
{
   DEBUG_RFIU_P2("rfiuSetMelodyNum_TX:%d\n",setting);
}

s32 rfiuSetVoxTrig_RX(s32 RFUnit)
{
   u8 vol;
   
   vol=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];
   DEBUG_RFIU_P2("-->TX-%d Vox Trig:%d\r\n",RFUnit,vol);

}

s32 rfiuSetLightStat_RX(s32 RFUnit)
{
   u8 sta;
   
#if ((SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) ||(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) ||\
(SW_APPLICATION_OPTION == MR8202_AN_KLF08W)) 
   sta=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];

#if ((HW_BOARD_OPTION  == MR9200_RX_MAYON_MWM903) || (UI_VERSION == UI_VERSION_MAYON))
    uiFlowSetRfLightStatus(RFUnit, sta, 0);
#else
#if (UI_LIGHT_SUPPORT == 1)
    uiFlowSetRfLightStatus(RFUnit, sta, 0);
#endif
#endif
#endif
    //DEBUG_RFIU_P2("-->TX-%d Light Status:%d\r\n",RFUnit,sta);
}

s32 rfiuForceResync(s32 RFUnit)
{
    u8 err;

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_RFIU_P2("=============rfiuForceResync Start:%d=============\n",RFUnit);
    OSTimeDly(4); // 0.2 sec
    gRfiuUnitCntl[RFUnit].RX_Task_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
    OSTimeDly(36); // 1.8 sec
    gRfiuUnitCntl[RFUnit].RX_Task_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;

    DEBUG_RFIU_P2("=============rfiuForceResync End:%d=============\n",RFUnit);
    OSSemPost(gRfiuSWReadySemEvt);
    
    return 0;
}

void rfiuCamOnOffCmd(u8 *cmd)
{
   int Setting;
   int i;
   int num;
   
   sscanf((char*)cmd,"%d",&Setting);
   DEBUG_RFIU_P2("Camer ON OFF Setting=0x%x\n",Setting); 
#if PWIFI_SUPPORT
   pWifi_End();
#else
   rfiu_End();
#endif   
   rfiuRX_CamOnOff_Sta=Setting;

   num=0;
   for(i=0;i<MAX_RFIU_UNIT;i++)
   {
      if(Setting & 0x01)
        num ++;

      Setting >>= 1;
   }
   rfiuRX_CamOnOff_Num=num;
   rfiu_InitCamOnOff(rfiuRX_CamOnOff_Sta);
   
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) ||\
    (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

#else
   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
   {
       sysRFRxInMainCHsel=0x0ff;
        #if RFRX_HALF_MODE_SUPPORT   
            if(rfiuRX_CamOnOff_Num <= 2)
            {
               if(sysTVOutOnFlag)
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
               else
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
            }
            else
        #endif
              iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
   }
   else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
   {
   }
   else if(sysCameraMode == SYS_CAMERA_MODE_PLAYBACK)
   {
   }
   else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
   {
   }
   #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
   else if(sysCameraMode == SYS_CAMERA_MODE_UI) //Lsk: Leave maskarea
   {
   }
   #endif
   else
     iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
#endif   

#if PWIFI_SUPPORT
   pWifi_Start();
#else
   rfiu_Start();
#endif   

}




int rfiuCamFeedBackAPPStaCmd(int RFUnit,int par1,int par2)
{
   int RfBusy;
   u8 uartCmd[20];

   if (Main_Init_Ready == 0)
      return 0;

   if (gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_OK)
      return 0;

   sprintf((char*)uartCmd,"FBAPPSTAT %d %d",par1,par2);

   if( rfiuRX_CamOnOff_Sta & (0x01<<RFUnit) )
   {
        RfBusy = rfiu_RXCMD_Enc(uartCmd, RFUnit);
   }

   return RfBusy;
}

    int rfiuRWAmicReg1(u8 *cmd)
    {
        u8 addr,data;
        
        //================//
        if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %x %x", &addr,&data);
            DEBUG_RFIU_P2("AMICREG1 W 0x%x 0x%x\n",addr,data);
            AmicReg_Addr=addr;
            AmicReg_Data=data;
            AmicReg_RWen1=1;            
        }
        else if(!strncmp((char*)cmd,"R ", strlen("R ")))
        {
            sscanf((char*)cmd, "R %x", &addr);
            DEBUG_RFIU_P2("AMICREG1 R 0x%x\n",addr);
            AmicReg_Addr=addr;
            AmicReg_RWen1=2;
        } 

        return 1;
    }

    int rfiuRWAmicReg2(u8 *cmd)
    {
        u8 addr,data;
        
        //================//
        if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %x %x", &addr,&data);
            DEBUG_RFIU_P2("AMICREG2 W 0x%x 0x%x\n",addr,data);
            AmicReg_Addr=addr;
            AmicReg_Data=data;
            AmicReg_RWen2=1;
            
        }
        else if(!strncmp((char*)cmd,"R ", strlen("R ")))
        {
            sscanf((char*)cmd, "R %x", &addr);
            DEBUG_RFIU_P2("AMICREG2 R 0x%x\n",addr);
            AmicReg_Addr=addr;
            AmicReg_RWen2=2;
        }    

        return 1;
    }

    int rfiuCheckTX_IDSame(int RFUnit)
    {
         int i;
         
         for(i=0;i<MAX_RFIU_UNIT;i++)
         {
             if(i != RFUnit)
             {
                 if(gRfiuUnitCntl[i].RFpara.RF_ID == gRfiuUnitCntl[RFUnit].RFpara.RF_ID)
                 {
                     gRfiuUnitCntl[i].RFpara.RF_ID=0xffffffff;
                     return i;
                 }
             }
         }

         return -1;
    }

    int rfiuClearRFSyncWord(int RFUnit)
    {
        gRfiuSyncWordTable[RFUnit]=0xffffffff;
        uiRFID[RFUnit]=(unsigned int)gRfiuSyncWordTable[RFUnit];
        rfiuRX_CamPair_Sta = rfiuRX_CamPair_Sta & (~(0x01<<RFUnit));
        rfiuRX_CamOnOff_Sta = rfiuRX_CamOnOff_Sta & (~(0x01<<RFUnit));
        //spiWriteRF(0);
    }

    int rfiuCheckRFUnpair(int RFUnit)
    {
        if(gRfiuSyncWordTable[RFUnit]==0xffffffff)
            return 1;
        else
            return 0;
    }


    int rfiuRFModule_RW(u8 *cmd)
    {
        u8 addr,data;
        if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %x %x", &addr,&data);
        #if( (RFIC_SEL==RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )    
            A7196_WriteReg_B1(addr,data);
        #elif(RFIC_SEL==RFIC_NONE_5M)
        
        #else
            A7130_WriteReg_B1(addr,data);
        #endif
            DEBUG_RFIU_P2("Write RF Reg: addr=0x%x,data=0x%x\n",addr,data);
        }
        else if((!strncmp((char*)cmd,"R ", strlen("R "))))
        {
            sscanf((char*)cmd, "R %x", &addr);
        #if( (RFIC_SEL==RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            data = A7196_ReadReg_B1(addr);  
        #elif(RFIC_SEL==RFIC_NONE_5M)
        
        #else
            data = A7130_ReadReg_B1(addr);        
        #endif
            DEBUG_RFIU_P2("Read RF Reg: addr=0x%x,data=0x%x\n",addr,data);
        }

    }

int rfiuGetACKType(unsigned char RFUnit,
                         unsigned char *ACKBufAddr,
                         unsigned int  *pRXTimeCheck)
{
    unsigned int *pp;
    int i,j,count,bitcount;
    int RX_ACKType;
    unsigned int temp;

    
    //=====//
    count=0;  
    pp  = (unsigned int *)(ACKBufAddr);
    while( (count <32) && (*pp != 0xa55aaa55))
    {
        //---Read Group value---//       
        for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
        {
            pp ++;
            count ++;
        }
        //---Read Map value---//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           pp ++;
           count ++;
           //------//
           pp ++;
           count ++;
           
        }
        // new add
        //--Read password--//
   #if RF_CMD_EN   
      #if 1
        RX_ACKType=*pp;
        pp ++;
        count ++;

        *pRXTimeCheck=*pp;
        pp ++;
        count ++;

      #else
        for(i=0;i<RFIU_PASSWORD_MAX;i++)
        {
           pp ++;
           count ++;
        }  
      #endif  
        
        //--RX Command--//
        pp ++;
        count ++;

        //--RX Status--//
        temp=(*pp) ^ 0x5a5aa5a5;;
        rfiuCmdRxStatus[RFUnit].CamNum= temp & 0x07; 
        gRfiuUnitCntl[RFUnit].TXSendDataUse3M= (temp>>3) & 0x01;
        pp ++;
        count ++;

        //--Customer ID--//
        pp ++;
        count ++;
   #endif      
        //---RX timer---//
        pp ++;
        count ++;

        //---RX receive packet count---//
        if(RX_ACKType == RXACK_FWUPD_START)
           *pRXTimeCheck=*pp ^ 0x5a5aa5a5;
        pp ++;
        count ++;
        
    }

    return RX_ACKType;
}

int rfiuTXCheckACKCome(DEF_REGRFIU_CFG *pRfiuPara)
{
    int i;
    unsigned int CmdID;
    //-------------//
    
    for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
    {
        CmdID=(pRfiuPara->Pkt_Grp_offset[i] >> 7);
        if((pRfiuPara->PktMap[2*i] & RFI_ACK_ADDR_CHEKBIT) && ( CmdID == RFI_CMD_ADDR_OFFSET))
        {
           return 1;
        }
    }

    return 0;
}

int rfiuTxWaitACKState( unsigned char RFUnit,
                              unsigned int Vitbi_en, 
                              unsigned int RS_mode, 
                              unsigned int Vitbi_mode,
                              unsigned int SyncWord,
                              unsigned int CustomerID,
                              unsigned int TimeOut
                            )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    
    
    
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

    //------Rx------//
    
    pRfiuPara_Rx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Rx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Rx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Rx->PktSyncWord          =SyncWord;
#if TX_PIRREC_VMDCHK
    if( (rfiuBatCamDcDetect==0) && (rfiuBatCamPIRTrig==1) && MotionDetect_en && (rfiuPIRRec_VMDTrig==0) )
       pRfiuPara_Rx->Customer_ID          = (CustomerID ^ 0x0ffff) & RFU_CUSTOMER_ID_MASK;
    else
       pRfiuPara_Rx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
#else    
    pRfiuPara_Rx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
#endif    
    pRfiuPara_Rx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Rx->PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
    pRfiuPara_Rx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
    
    pRfiuPara_Rx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Rx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Rx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Rx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    pRfiuPara_Rx->TxRxPktNum           =TimeOut;            

    pRfiuPara_Rx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Rx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Rx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Rx(0,pRfiuPara_Rx,RFUnit);  //Rx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Rx(RFUnit & 0x01,pRfiuPara_Rx,RFUnit);  //Rx
#else 
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Rx(RFUnit,pRfiuPara_Rx,RFUnit);  //Rx
#endif

    return 1;
}




//-------------------------FCC test --------------------//
s32 marsRfiu_FCC_DirectTXRX(s32 dummy)
{
    int testrun,i;
    unsigned char *pp;
    unsigned int err;
    GPIO_CFG c;
    unsigned int TX_Freq;
    unsigned int Vitbi_en,RS_mode,Vitbi_mode;
    unsigned int Old_FCCUnit;
    int DAT_CH_sel;
    u8 RSSI,RSSI2;

    const u8 PN9_Tab[64]={ 
        0xFF, 0x83, 0xDF, 0x17, 0x32, 0x09, 0x4E, 0xD1,
        0xE7, 0xCD, 0x8A, 0x91, 0xC6, 0xD5, 0xC4, 0xC4,
        0x40, 0x21, 0x18, 0x4E, 0x55, 0x86, 0xF4, 0xDC,
        0x8A, 0x15, 0xA7, 0xEC, 0x92, 0xDF, 0x93, 0x53,
        0x30, 0x18, 0xCA, 0x34, 0xBF, 0xA2, 0xC7, 0x59,
        0x67, 0x8F, 0xBA, 0x0D, 0x6D, 0xD8, 0x2D, 0x7D,
        0x54, 0x0A, 0x57, 0x97, 0x70, 0x39, 0xD2, 0x7A,
        0xEA, 0x24, 0x33, 0x85, 0xED, 0x9A, 0x1D, 0xE0
    };
    //--------//
    FCC_Unit_Sel=0;
    Old_FCCUnit=0;

    Vitbi_en=RFI_VITBI_DISA;
    RS_mode=RFI_RS_T4;
    Vitbi_mode=RFI_VITBI_CR1_2;
   
    //---Setup test environment---//
    
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].TX_Task_Stop=1;
       gRfiuUnitCntl[i].TX_Wrap_Stop=1;
       gRfiuUnitCntl[i].TX_MpegEnc_Stop=1;

       gRfiuUnitCntl[i].RX_Task_Stop=1;
       gRfiuUnitCntl[i].RX_Wrap_Stop=1;
       gRfiuUnitCntl[i].RX_MpegDec_Stop=1;

       gRfiuUnitCntl[i].FCC_TestMode= FCC_DIRECT_TX_MODE;
       gRfiuUnitCntl[i].FCC_TX_RealData_ON=0;
       gRfiuUnitCntl[i].FCC_TX_NoData_Zero=0;
    #if RFIU_FCC_FHSS15CH_SUPPORT
       gRfiuUnitCntl[i].FCC_TX_Freq=2405*2;
       gRfiuUnitCntl[i].FCC_RX_Freq=2405*2;
    #else
       gRfiuUnitCntl[i].FCC_TX_Freq=2408*2;
       gRfiuUnitCntl[i].FCC_RX_Freq=2408*2;
    #endif   
    }
    OSTimeDly(3);
    
    pp=(unsigned char *)rfiuOperBuf[0];

    //---Set Default Power---//
 #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL==RFIC_A7130_2M) || (RFIC_SEL==RFIC_A7130_3M) )

 #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    A7196_WriteReg_B1(0x2d, 0x7f); // 19.0
  #if(RFI_FCC_DIRECT_TRX ==2)
    A7196_WriteReg_B2(0x2d, 0x7f); // 19.0
  #endif
 #elif(RFIC_SEL==RFIC_NONE_5M)
 
 #endif

    DEBUG_RFIU_P2("----------------------------marsRfiu_FCC_DirectTXRX---------------------------------\n");
    for(i=0;i<RFIU_TESTCNTMAX_WD*4;i++)
    {
        *pp= PN9_Tab[i % 64];
         pp ++;
    }

    //-----Test run------//
    testrun=0;
    while(1)
    {
        #if 1
            if(AmicReg_RWen1)
            {
               if(AmicReg_RWen1 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }

               AmicReg_RWen1=0;
            }    
            
            if(AmicReg_RWen2)
            {
               if(AmicReg_RWen2 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif
    
        if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_TX_MODE)
        {
            
            if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON)
            {
                //====Select CH====//            
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );            
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );  
            #elif(RFIC_SEL==RFIC_NONE_5M)
            
            #endif
            
                RfiuReset(FCC_Unit_Sel);
                DEBUG_RFIU_P2("TX-%d:%d,%d,Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq/2);
                
                //---config RFU parameter---//
    			//------TX1------//
                gRfiuParm_Tx[FCC_Unit_Sel].Vitbi_en             =Vitbi_en;
                gRfiuParm_Tx[FCC_Unit_Sel].RsCodeSizeSel        =RS_mode;
                gRfiuParm_Tx[FCC_Unit_Sel].CovCodeRateSel       =Vitbi_mode;
                
                gRfiuParm_Tx[FCC_Unit_Sel].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
                gRfiuParm_Tx[FCC_Unit_Sel].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
                gRfiuParm_Tx[FCC_Unit_Sel].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
                gRfiuParm_Tx[FCC_Unit_Sel].PktSyncWord          =gRfiuSyncWordTable[0];
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxClkConfig          =RFI_TXCLKCONFIG;
                gRfiuParm_Tx[FCC_Unit_Sel].RxClkAdjust          =RFI_RXCLKADJUST;
                gRfiuParm_Tx[FCC_Unit_Sel].DclkConfig           =RFI_DCLKCONF;
                gRfiuParm_Tx[FCC_Unit_Sel].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

                for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
                   gRfiuParm_Tx[FCC_Unit_Sel].PktMap[i]         =0xffffffff;
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxRxPktNum           =256;  
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxRxOpBaseAddr       =rfiuOperBuf[0];

                for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
                   gRfiuParm_Tx[FCC_Unit_Sel].Pkt_Grp_offset[i] =64*RFIU_PKT_SIZE*i;
                
                gRfiuParm_Tx[FCC_Unit_Sel].Customer_ID_ext_en   =0;
                gRfiuParm_Tx[FCC_Unit_Sel].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
              
                gRfiuParm_Tx[FCC_Unit_Sel].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
                
                gRfiuParm_Tx[FCC_Unit_Sel].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
                gRfiuParm_Tx[FCC_Unit_Sel].UserData_H           =testrun & RFI_USER_DATA_H_MASK;

                gRfiuParm_Tx[FCC_Unit_Sel].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
                
                //===Switch Real data or No data====//                
                //GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );  // RFU pin mux: use IIC for RF2
                c.ena = GPIO_DISA;
            	c.dir = GPIO_DIR_OUT;
            	c.level = GPIO_LEVEL_HI;
            	c.inPullUp = GPIO_IN_PULLUP_DISA;

                if(FCC_Unit_Sel==0)
                   gpioConfig(2,13,&c);
                else if(FCC_Unit_Sel==1)
                {
                   if(GpioActFlashSelect & CHIP_IO_RFI2_EN)
                      gpioConfig(1,17,&c);
                   else
                      gpioConfig(2,20,&c);
                }
                //----//
                rfiuDataPktConfig_Tx(FCC_Unit_Sel,&(gRfiuParm_Tx[FCC_Unit_Sel]),FCC_Unit_Sel );  //Tx
           #if(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_FCC_247)
                OSTimeDly(1);
                if(FCC_Unit_Sel==0)
                {
                    gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA Off
                    gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA Off
                }
                else
                {
                    gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA Off
                    gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA Off
                }
           #endif                
                rfiuWaitForInt_Tx(FCC_Unit_Sel,&gRfiuParm_Tx[FCC_Unit_Sel]);  //Tx  
                OSTimeDly(1);
            }
            else //No data
            {
                c.ena = GPIO_ENA;
            	c.dir = GPIO_DIR_OUT;
                if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero)
                   c.level = GPIO_LEVEL_LO;
                else
            	   c.level = GPIO_LEVEL_HI;
            	c.inPullUp = GPIO_IN_PULLUP_DISA;

                if(FCC_Unit_Sel==0)
                   gpioConfig(2,13,&c);
                else if(FCC_Unit_Sel==1)
                {
                   if(GpioActFlashSelect & CHIP_IO_RFI2_EN)
                      gpioConfig(1,17,&c);
                   else
                      gpioConfig(2,20,&c);
                }
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )   
                if(FCC_Unit_Sel ==0)
                   A7130_WriteReg_B1(0x15, 0x87); 
                else
                   A7130_WriteReg_B2(0x15, 0x87); 
                
                A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) ); 
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                if(FCC_Unit_Sel ==0)
                   A7196_WriteReg_B1(0x15, 0x07); 
                else
                   A7196_WriteReg_B2(0x15, 0x07);  
              
                A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) ); 
            #elif(RFIC_SEL==RFIC_NONE_5M)
            
            #endif
                TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                Old_FCCUnit=FCC_Unit_Sel;  
                
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )                            
                A7130_TxMode_Start(FCC_Unit_Sel+1);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_TxMode_Start(FCC_Unit_Sel+1);
            #elif(RFIC_SEL==RFIC_NONE_5M)
                RFNONE_TxMode_Start(FCC_Unit_Sel+1);
            #endif
                while( (gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON == 0) && (gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_TX_MODE) )
                {
                    OSTimeDly(4);

                #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    if( FCC_Unit_Sel != Old_FCCUnit )
                    {
                        if(FCC_Unit_Sel ==0)
                           A7130_WriteReg_B1(0x15, 0x87); 
                        else
                           A7130_WriteReg_B2(0x15, 0x87);      
                        
                        A7130_TxMode_Stop(Old_FCCUnit+1);
                        A7130_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );            
                        A7130_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                        Old_FCCUnit=FCC_Unit_Sel;                        
                    }
                
                    if( TX_Freq !=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq )
                    {
                        A7130_TxMode_Stop(FCC_Unit_Sel+1);
                        A7130_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );            
                        A7130_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                    }
                #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    if( FCC_Unit_Sel != Old_FCCUnit )
                    {
                        if(FCC_Unit_Sel ==0)
                           A7196_WriteReg_B1(0x15, 0x07); 
                        else
                           A7196_WriteReg_B2(0x15, 0x07);    
                    
                        A7196_TxMode_Stop(Old_FCCUnit+1);
                        A7196_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );            
                        A7196_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                        Old_FCCUnit=FCC_Unit_Sel;                        
                    }
                
                    if( TX_Freq !=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq )
                    {
                        A7196_TxMode_Stop(FCC_Unit_Sel+1);
                        A7196_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400*2) );            
                        A7196_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                    }
                #elif(RFIC_SEL==RFIC_NONE_5M)
                
                #endif

                    if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero)
                       c.level = GPIO_LEVEL_LO;
                    else
                	   c.level = GPIO_LEVEL_HI;

                    if(FCC_Unit_Sel==0)
                       gpioConfig(2,13,&c);
                    else if(FCC_Unit_Sel==1)
                    {
                       if(GpioActFlashSelect & CHIP_IO_RFI2_EN)
                          gpioConfig(1,17,&c);
                       else
                          gpioConfig(2,20,&c);
                    }                    
                    DEBUG_RFIU_P2("TX-%d:%d,%d,Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq/2);

                }
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                if(FCC_Unit_Sel ==0)
                   A7130_WriteReg_B1(0x15, 0xAF); 
                else
                   A7130_WriteReg_B2(0x15, 0xAF);  
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                if(FCC_Unit_Sel ==0)
                   A7196_WriteReg_B1(0x15, 0x2f); 
                else
                   A7196_WriteReg_B2(0x15, 0x2f);   
            #elif(RFIC_SEL==RFIC_NONE_5M)
            
            #endif     
            
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_TxMode_Stop(FCC_Unit_Sel+1);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_TxMode_Stop(FCC_Unit_Sel+1);
            #elif(RFIC_SEL==RFIC_NONE_5M)
                RFNONE_TxMode_Stop(FCC_Unit_Sel+1);
            #endif
            }
                    
        }
        else if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_RX_MODE)
        {
        #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) );            
        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) ); 
        #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) ); 
        #endif
            RfiuReset(FCC_Unit_Sel);

            //---config RFU parameter---//
			//------Rx1------//
            gRfiuParm_Rx[FCC_Unit_Sel].TxRxOpBaseAddr       =rfiuOperBuf[0];
            gRfiuParm_Rx[FCC_Unit_Sel].Vitbi_en             =Vitbi_en;
            gRfiuParm_Rx[FCC_Unit_Sel].CovCodeRateSel       =Vitbi_mode;
            gRfiuParm_Rx[FCC_Unit_Sel].RsCodeSizeSel        =RS_mode;
            
            gRfiuParm_Rx[FCC_Unit_Sel].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Rx[FCC_Unit_Sel].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Rx[FCC_Unit_Sel].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Rx[FCC_Unit_Sel].Customer_ID_ext_en   =0;
            gRfiuParm_Rx[FCC_Unit_Sel].SuperBurstMode_en    =0;
            gRfiuParm_Rx[FCC_Unit_Sel].Customer_ID          =0x1234;
            
            gRfiuParm_Rx[FCC_Unit_Sel].UserData_L           =0x0;
            gRfiuParm_Rx[FCC_Unit_Sel].UserData_H           =0x0;
            gRfiuParm_Rx[FCC_Unit_Sel].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[FCC_Unit_Sel].PktSyncWord          =0x17df83ff;
            
            gRfiuParm_Rx[FCC_Unit_Sel].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[FCC_Unit_Sel].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[FCC_Unit_Sel].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[FCC_Unit_Sel].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[FCC_Unit_Sel].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            rfiuDataPktConfig_Rx(FCC_Unit_Sel,&(gRfiuParm_Rx[FCC_Unit_Sel]),FCC_Unit_Sel );  //Rx            
            RSSI=rfiuWaitForInt_Rx(FCC_Unit_Sel,&gRfiuParm_Rx[FCC_Unit_Sel],0,&RSSI2);  //Rx
            DEBUG_RFIU_P2("RX-%d:Freq=%d,RSSI=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq/2,RSSI);
        }
        else if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_TXSCAN_MODE)
        {
                //====Select CH====//            
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_CH_sel( FCC_Unit_Sel+1,gRfiuDAT_CH_Table[DAT_CH_sel] );            
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_CH_sel( FCC_Unit_Sel+1,gRfiuDAT_CH_Table[DAT_CH_sel] );   
            #elif(RFIC_SEL==RFIC_NONE_5M)
                RFNONE_CH_sel( FCC_Unit_Sel+1,gRfiuDAT_CH_Table[DAT_CH_sel] );   
            #endif
            
                RfiuReset(FCC_Unit_Sel);
                DEBUG_RFIU_P2("TX-%d:%d,%d,Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero,gRfiuDAT_CH_Table[DAT_CH_sel]/2+2400);
                
                //---config RFU parameter---//
    			//------TX1------//
                gRfiuParm_Tx[FCC_Unit_Sel].Vitbi_en             =Vitbi_en;
                gRfiuParm_Tx[FCC_Unit_Sel].RsCodeSizeSel        =RS_mode;
                gRfiuParm_Tx[FCC_Unit_Sel].CovCodeRateSel       =Vitbi_mode;
                
                gRfiuParm_Tx[FCC_Unit_Sel].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
                gRfiuParm_Tx[FCC_Unit_Sel].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
                gRfiuParm_Tx[FCC_Unit_Sel].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
                gRfiuParm_Tx[FCC_Unit_Sel].PktSyncWord          =gRfiuSyncWordTable[0];
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxClkConfig          =RFI_TXCLKCONFIG;
                gRfiuParm_Tx[FCC_Unit_Sel].RxClkAdjust          =RFI_RXCLKADJUST;
                gRfiuParm_Tx[FCC_Unit_Sel].DclkConfig           =RFI_DCLKCONF;
                gRfiuParm_Tx[FCC_Unit_Sel].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

                for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
                   gRfiuParm_Tx[FCC_Unit_Sel].PktMap[i]         =0xffffffff;
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxRxPktNum           =256;  
                
                gRfiuParm_Tx[FCC_Unit_Sel].TxRxOpBaseAddr       =rfiuOperBuf[0];

                for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
                   gRfiuParm_Tx[FCC_Unit_Sel].Pkt_Grp_offset[i] =64*RFIU_PKT_SIZE*i;
                
                gRfiuParm_Tx[FCC_Unit_Sel].Customer_ID_ext_en   =0;
                gRfiuParm_Tx[FCC_Unit_Sel].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
              
                gRfiuParm_Tx[FCC_Unit_Sel].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
                
                gRfiuParm_Tx[FCC_Unit_Sel].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
                gRfiuParm_Tx[FCC_Unit_Sel].UserData_H           =testrun & RFI_USER_DATA_H_MASK;

                gRfiuParm_Tx[FCC_Unit_Sel].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
                
                //===Switch Real data or No data====//                
                //GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );  // RFU pin mux: use IIC for RF2
                c.ena = GPIO_DISA;
            	c.dir = GPIO_DIR_OUT;
            	c.level = GPIO_LEVEL_HI;
            	c.inPullUp = GPIO_IN_PULLUP_DISA;

                if(FCC_Unit_Sel==0)
                   gpioConfig(2,13,&c);
                else if(FCC_Unit_Sel==1)
                {
                   if(GpioActFlashSelect & CHIP_IO_RFI2_EN)
                      gpioConfig(1,17,&c);
                   else
                      gpioConfig(2,20,&c);
                }

                //----//
                rfiuDataPktConfig_Tx(FCC_Unit_Sel,&(gRfiuParm_Tx[FCC_Unit_Sel]),FCC_Unit_Sel );  //Tx
           #if(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_FCC_247)
                OSTimeDly(1);
                if(FCC_Unit_Sel==0)
                {
                    gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA Off
                    gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA Off
                }
                else
                {
                    gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA Off
                    gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA Off
                }
           #endif                
                rfiuWaitForInt_Tx(FCC_Unit_Sel,&gRfiuParm_Tx[FCC_Unit_Sel]);  //Tx  
                OSTimeDly(1);

                DAT_CH_sel= (DAT_CH_sel + 1) % 16;
            }
        
        testrun ++;
    }

    return 1;

}


    int rfiuFCCTX0Cmd(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;
        //================//
        sscanf((char*)cmd,"%d %d %d %d",&RealData_ON,&NoData_Zero,&Freq,PowerReg);
        DEBUG_RFIU_P2("FCC TX0 Cmd: RealData=%d, NoData_Zero=%d,Freq=%d,PwReg=%d\n",RealData_ON,NoData_Zero,Freq/2,PowerReg);

        gRfiuUnitCntl[RFI_UNIT_0].FCC_TestMode=FCC_DIRECT_TX_MODE;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_RealData_ON=RealData_ON;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_NoData_Zero=NoData_Zero;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=Freq;
      #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
        A7130_WriteReg_B1(0x2d, PowerReg);
      #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_WriteReg_B1(0x2d, PowerReg);
      #elif(RFIC_SEL == RFIC_NONE_5M)
      
      #endif
    }

    int rfiuFCCTX1Cmd(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;
        //================//
        sscanf((char*)cmd,"%d %d %d %d",&RealData_ON,&NoData_Zero,&Freq,PowerReg);
        DEBUG_RFIU_P2("FCC TX1 Cmd: RealData=%d, NoData_Zero=%d,Freq=%d,PwReg=%d\n",RealData_ON,NoData_Zero,Freq/2,PowerReg);

        gRfiuUnitCntl[RFI_UNIT_1].FCC_TestMode=FCC_DIRECT_TX_MODE;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_RealData_ON=RealData_ON;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_NoData_Zero=NoData_Zero;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=Freq;
      #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
        A7130_WriteReg_B2(0x2d, PowerReg);
      #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_WriteReg_B2(0x2d, PowerReg);
      #elif(RFIC_SEL == RFIC_NONE_5M)
      
      #endif

    }

    int rfiuFCCTX0Cmd2(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;

        int sel;
        //================//
        if(!strncmp((char*)cmd,"P ", strlen("P ")))
        {
            sscanf((char*)cmd, "P %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                case 0: 
                    A7130_WriteReg_B1(0x2d, 0x34); // -2.54
                    break;

                case 1: 
                    A7130_WriteReg_B1(0x2d, 0x35); // +0.2
                    break;

                case 2: 
                    A7130_WriteReg_B1(0x2d, 0x36); // +2.63
                    break;
                    
                case 3:
                    A7130_WriteReg_B1(0x2d, 0x37); // +4.5
                    break;
            #elif((RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                case 0: 
                    A7196_WriteReg_B1(0x2d, 0x23);  // 16.2
                    break;

                case 1: 
                    A7196_WriteReg_B1(0x2d, 0x27);  // 17.4
                    break;

                case 2: 
                    A7196_WriteReg_B1(0x2d, 0x6b);  // 18.5
                    break;
                    
                case 3: 
                    A7196_WriteReg_B1(0x2d, 0x7f);  // 19.0
                    break;
            #elif(RFIC_SEL == RFIC_NONE_5M)
            
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                case 0: 
                    A7130_WriteReg_B1(0x2d, 0x00);
                    break;

                case 1: 
                    A7130_WriteReg_B1(0x2d, 0x02);
                    break;

                case 2: 
                    A7130_WriteReg_B1(0x2d, 0x1b);
                    break;
                    
                case 3: 
                    A7130_WriteReg_B1(0x2d, 0x37);
                    break;
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )  
                case 0: 
                    A7196_WriteReg_B1(0x2d, 0x23);
                    break;

                case 1: 
                    A7196_WriteReg_B1(0x2d, 0x27);
                    break;

                case 2: 
                    A7196_WriteReg_B1(0x2d, 0x6b);
                    break;
                    
                case 3: 
                    A7196_WriteReg_B1(0x2d, 0x7f);
                    break;
            #elif(RFIC_SEL == RFIC_NONE_5M)
            
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"F ", strlen("F ")))
        {
            sscanf((char*)cmd, "F %d", &sel);
            switch(sel)
            {
                case 0: 
                  #if RFIU_FCC_FHSS15CH_SUPPORT
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2405*2;
                  #else
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2408*2;
                  #endif 
                    break;

                case 1: 
                  #if RFIU_FCC_FHSS15CH_SUPPORT
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2441*2;
                  #else
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2440*2;
                  #endif
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2468*2;
                    break;
                    
               
            }
        }
        else if(!strncmp((char*)cmd,"D ", strlen("D ")))
        {
            sscanf((char*)cmd, "D %d", &sel);
            switch(sel)
            {
                case 0: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_RealData_ON=0;
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_NoData_Zero=1;
                    break;

                case 1: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_RealData_ON=0;
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_NoData_Zero=0;
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_RealData_ON=1;
                    break;
                    
               
            }
        }
        else
        {
            sscanf((char*)cmd, "%d", &Freq);
            gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=Freq;
        }
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TestMode=FCC_DIRECT_TX_MODE; 
    }


    int rfiuFCCTX1Cmd2(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;

        int sel;
        //================//
        if(!strncmp((char*)cmd,"P ", strlen("P ")))
        {
            sscanf((char*)cmd, "P %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
                case 0: 
                    A7130_WriteReg_B2(0x2d, 0x34);  // -2.54 dBm
                    break;

                case 1: 
                    A7130_WriteReg_B2(0x2d, 0x35);  // +0.2 dBm
                    break;

                case 2: 
                    A7130_WriteReg_B2(0x2d, 0x36);  // +2.63 dBm
                    break;
                    
                case 3: 
                    A7130_WriteReg_B2(0x2d, 0x37);  // + 4.5 dBm
                    break;
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                case 0: 
                    A7196_WriteReg_B2(0x2d, 0x23);  // 16.2 dBm
                    break;

                case 1: 
                    A7196_WriteReg_B2(0x2d, 0x27);  // 17.4 dBm
                    break;

                case 2: 
                    A7196_WriteReg_B2(0x2d, 0x6b);  // 18.5 dBm
                    break;
                    
                case 3: 
                    A7196_WriteReg_B2(0x2d, 0x7f); // 19.0  dBm
                    break;
            #elif(RFIC_SEL == RFIC_NONE_5M)
            
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
                case 0: 
                    A7130_WriteReg_B2(0x2d, 0x00);
                    break;

                case 1: 
                    A7130_WriteReg_B2(0x2d, 0x02);
                    break;

                case 2: 
                    A7130_WriteReg_B2(0x2d, 0x1b);
                    break;
                    
                case 3: 
                    A7130_WriteReg_B2(0x2d, 0x37);
                    break;
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                case 0: 
                    A7196_WriteReg_B2(0x2d, 0x23);
                    break;

                case 1: 
                    A7196_WriteReg_B2(0x2d, 0x27);
                    break;

                case 2: 
                    A7196_WriteReg_B2(0x2d, 0x6b);
                    break;
                    
                case 3: 
                    A7196_WriteReg_B2(0x2d, 0x7f);
                    break;
            #elif(RFIC_SEL == RFIC_NONE_5M)
            
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"F ", strlen("F ")))
        {
            sscanf((char*)cmd, "F %d", &sel);
            switch(sel)
            {
                case 0: 
                  #if RFIU_FCC_FHSS15CH_SUPPORT
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2405*2;
                  #else
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2408*2;
                  #endif
                    break;

                case 1: 
                  #if RFIU_FCC_FHSS15CH_SUPPORT
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2441*2;
                  #else
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2440*2;
                  #endif
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2468*2;
                    break;
                    
               
            }
        }
        else if(!strncmp((char*)cmd,"D ", strlen("D ")))
        {
            sscanf((char*)cmd, "D %d", &sel);
            switch(sel)
            {
                case 0: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_RealData_ON=0;
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_NoData_Zero=1;
                    break;

                case 1: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_RealData_ON=0;
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_NoData_Zero=0;
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_RealData_ON=1;
                    break;
                    
               
            }
        }
        else
        {
            sscanf((char*)cmd, "%d", &Freq);
            gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=Freq;
        }
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TestMode=FCC_DIRECT_TX_MODE;        
    }
    
    int rfiuFCCRX0Cmd(u8 *cmd)
    {
        int Freq;
        //================//
        sscanf((char*)cmd,"%d",&Freq);
        DEBUG_RFIU_P2("FCC RX Cmd:Freq=%d(x2)\n",Freq);

        gRfiuUnitCntl[RFI_UNIT_0].FCC_TestMode=FCC_DIRECT_RX_MODE;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_RX_Freq=Freq;
    }

    int rfiuFCCRX1Cmd(u8 *cmd)
    {
        int Freq;
        //================//
        sscanf((char*)cmd,"%d",&Freq);
        DEBUG_RFIU_P2("FCC RX Cmd:Freq=%d(x2)\n",Freq);

        gRfiuUnitCntl[RFI_UNIT_1].FCC_TestMode=FCC_DIRECT_RX_MODE;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_RX_Freq=Freq;
    }

    int rfiuFCCUnitSel(u8 *cmd)
    {
        int RFUnit;
        //================//
        sscanf((char*)cmd,"%d",&RFUnit);
        DEBUG_RFIU_P2("FCC Unit Sel:Unit=%d\n",RFUnit);

        FCC_Unit_Sel=RFUnit;

    }



#if TX_FW_UPDATE_SUPPORT
unsigned int rfiuCalTxFwCheckSum(int RFUnit)
{
    unsigned int *pp;
    int Words;
    int i;
    unsigned int Sum;

    Sum=0;
    Words=(RFI_GRP_INPKTUNIT * RFI_BUF_SIZE_GRPUNIT * RFI_PAYLOAD_SIZE)/4;
    pp=(unsigned int *)rfiuOperBuf[RFUnit];
    for(i=0;i<Words;i++)
    {
       Sum += *pp;
       pp++;
    }

    return Sum;
}




int rfiuReplyACK_FWUPD(    int RFUnit,
                                        unsigned int Vitbi_en, 
                                        unsigned int RS_mode, 
                                        unsigned int Vitbi_mode,
                                        unsigned int SyncWord,
                                        unsigned int CustomerID,
                                        unsigned int UserData,
                                        unsigned int RX_TimeCheck,
                                        unsigned int CH_chg_flag,
                                        unsigned int RX_CHG_CHNext,
                                        unsigned int AckType,
                                        unsigned int CheckSum
                                    )
{

    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    int i,j,count;
    int nextBufReadPtr;
    int Grpcnt;
    //----------------//   
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

    if(AckType==RXACK_FWUPD_DATA)
    {
        while( (gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK].PktCount==0) && (gRfiuUnitCntl[RFUnit].BufReadPtr < RFI_BUF_SIZE_GRPUNIT) )
        {
           gRfiuUnitCntl[RFUnit].BufReadPtr= (gRfiuUnitCntl[RFUnit].BufReadPtr + 1) ;
        }
        DEBUG_RFIU_P2("BufReadPtr=%d,0x%x,0x%x\n",gRfiuUnitCntl[RFUnit].BufReadPtr,
                       gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK].PktMap0,
                       gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK].PktMap1);
        if(gRfiuUnitCntl[RFUnit].BufReadPtr >= RFI_BUF_SIZE_GRPUNIT)
            return 1;

        nextBufReadPtr=gRfiuUnitCntl[RFUnit].BufReadPtr;
        
        for(Grpcnt=0;Grpcnt<RFIU_USEPKTMAP_MAX/2;Grpcnt++)
        {   
            pRfiuPara_Rx->Pkt_Grp_offset[Grpcnt]    =0; 
            pRfiuPara_Rx->PktMap[Grpcnt*2]          =0;
            pRfiuPara_Rx->PktMap[Grpcnt*2+1]        =0;
        
            while( (gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount==0) && (nextBufReadPtr<RFI_BUF_SIZE_GRPUNIT) )
            {
               nextBufReadPtr = (nextBufReadPtr+1) & RFI_BUF_SIZE_MASK;
            }
            
            if(nextBufReadPtr<RFI_BUF_SIZE_GRPUNIT)
            {
               pRfiuPara_Rx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*nextBufReadPtr; 
               pRfiuPara_Rx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr].PktMap0;
               pRfiuPara_Rx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr].PktMap1;
               pRfiuPara_Rx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr].PktCount;

               nextBufReadPtr = (nextBufReadPtr+1);
            }

        } 

    }
    
    rfiuPutFwUpdPkt2ACK(RFUnit, pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,pRfiuPara_Rx,RX_TimeCheck,CH_chg_flag,RX_CHG_CHNext,AckType,CheckSum);                             
    //---config RFU parameter---//
    //-Tx-//
    
    pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Tx->PktSyncWord          =SyncWord;
    pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    
    pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
    pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
    pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    
    pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

    for(i=0;i<RFI_ACK_SYNC_PKTNUM ;i++)
    {
        pRfiuPara_Tx->PktMap[2*i]      =RFI_ACK_ADDR_CHEKBIT;
        pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
        pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
    }
    pRfiuPara_Tx->TxRxPktNum           = RFI_ACK_SYNC_PKTNUM;  
          
    pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
    pRfiuPara_Tx->UserData_H           =(UserData>>16) & RFI_USER_DATA_H_MASK;

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);      
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);
#else
    RfiuReset(RFUnit);      
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif

    return 0;
}


int rfiuPutFwUpdPkt2ACK(int RFUnit, unsigned char *ACKBufAddr,
                        DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,
                        unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext,
                        unsigned int AckType,unsigned int CheckSum)
{
    unsigned int *pp;
    int i,count;
    unsigned int RFTimer;
    //=====//
    
    timerCountRead(guiRFTimerID, &RFTimer);
    count=0;    
    pp  = (unsigned int *)(ACKBufAddr);
    //

    if(AckType ==RXACK_FWUPD_DATA)
    {
        for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
        {
            *pp = (((pRfiuPara->Pkt_Grp_offset[i*2]>>13) & 0x7ff)<<0) |
                  (((pRfiuPara->Pkt_Grp_offset[i*2+1]>>13) & 0x7ff)<<11) |
                  (((gRfiuUnitCntl[RFUnit].RX_CMD_Data[i]) & 0xff)<<24);
    		
            *pp ^= 0x5a5aa5a5;
            pp ++;
            count ++;
        }
        
        //
        for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
        {
           *pp=pRfiuPara->PktMap[i] ^ 0x5a5aa5a5; //Xor 0101b
           pp ++;
           count ++;
        }
    }
    else
    {
        for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
        {
            *pp = (((0x0>>13) & 0x7ff)<<0) |
                  (((0x0>>13) & 0x7ff)<<11) |
                  (((gRfiuUnitCntl[RFUnit].RX_CMD_Data[i]) & 0xff)<<24);
    		
            *pp ^= 0x5a5aa5a5;
            pp ++;
            count ++;
        }
        
        //
        for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
        {
           *pp=0x0 ^ 0x5a5aa5a5; //Xor 0101b
           pp ++;
           count ++;
        }
    }
    //--Put password--//    
    *pp = AckType;
    pp ++;
    count ++;

    *pp=RFTimer;
    pp ++;
    count ++;

    //--RX Cmd Data[5~7]
    *pp= (gRfiuUnitCntl[RFUnit].RX_CMD_Data[5] | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]<<8) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]<<16) | ( (gRfiuUnitCntl[RFUnit].BufWritePtr & 0x0ff)<<24) ) ^ 0x5a5aa5a5; 
    pp ++;
    count ++;

    //--Send Status: CH number bit3:0, RX Cmd[8~10] --//
    *pp= ( ( (RFUnit & 0x07) | ( ( gRfiuUnitCntl[RFUnit].ProtocolSel & 0x03 )<<4) | ((gRfiuUnitCntl[RFUnit].VMDSel & 0x01)<<6) | ( (CH_chg_flag & 0x01)<<7) ) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]<<8) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[9]<<16) | ( (RX_CHG_CHNext & 0xff)<<24) )^ 0x5a5aa5a5; 
    pp ++;
    count ++;

    //--Reserved or Customer Code: 1 word---//
    *pp= RF_CUSTOMERID_SET ^ 0x5a5aa5a5; 
    pp ++;
    count ++;
    
    //--Put time checker--//
    *pp = RX_TimeCheck;
    pp ++;
    count ++;
    
    //--Put Receive packet count--//
    if(AckType == RXACK_FWUPD_START)
       *pp = CheckSum;  
    else
       *pp=pRfiuPara->TxRxPktNum & 0x0ffff;
    
    *pp ^= 0x5a5aa5a5;
    pp ++;
    count ++;
    //
    for(i=count;i<32;i++)
    {
       *pp = 0xa55aaa55;//magic number for End.
       pp ++;
       count ++;
    }

    return 1;

}

int rfiuFwUpdListenDataState(unsigned char RFUnit,
                                      unsigned int Vitbi_en, 
                                      unsigned int RS_mode, 
                                      unsigned int Vitbi_mode,
                                      unsigned int SyncWord,
                                      unsigned int CustomerID,
                                      unsigned int TimeOut
                                     )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    
    //-------//
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
        
    //---config RFU parameter---//   
    //--Rx--//    
    pRfiuPara_Rx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Rx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Rx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Rx->PktSyncWord          =SyncWord;
    pRfiuPara_Rx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    
    pRfiuPara_Rx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Rx->PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
    pRfiuPara_Rx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
    
    pRfiuPara_Rx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Rx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Rx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Rx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    pRfiuPara_Rx->TxRxPktNum           =TimeOut;            

    pRfiuPara_Rx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Rx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Rx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
    
    //----//
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Rx(0,pRfiuPara_Rx,RFUnit);  //Rx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Rx(RFUnit & 0x01,pRfiuPara_Rx,RFUnit);  //Rx 
#else    
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Rx(RFUnit,pRfiuPara_Rx,RFUnit);  //Rx
#endif
    return 1;
  
}







#endif

//------------------------------------Rfiu_Project-----------------------------------//
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||\
    (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

void rfiuCheckTrigger(int RFUnit, int *TX_UsrData_next, int TXCmd_Valid)
{
#if (UI_LIGHT_SUPPORT)
        u8 dayLevel;
#endif
        u8 level;

 #if RF_CMD_EN
    if((gRfiuUnitCntl[RFUnit].TXCmd_en == 1) && (TXCmd_Valid==1) )
       *TX_UsrData_next |= RFIU_USRDATA_CMD_CHEK ; //enable command 
    else
       *TX_UsrData_next &= (~RFIU_USRDATA_CMD_CHEK);
    
    #if ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) ||\
        (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T) ||(HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612))
        gRfiuUnitCntl[RFUnit].RFpara.PIR_en=1;
    #endif
    if( gRfiuUnitCntl[RFUnit].RFpara.MD_en  || gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
    {

        if(gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
        {
        #if ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) || \
            (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612) ||\
            (HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM) || (HW_BOARD_OPTION  == MR9160_TX_MAYON_MWL613))
            #if (PIR_TRIGER_ACT_HIGH == 1)
                if (gpioGetLevel(GPIO_GROUP_PIR, GPIO_BIT_PIR, &level))
                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=level;  //Active High
            #else
                if (gpioGetLevel(GPIO_GROUP_PIR, GPIO_BIT_PIR, &level))
                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=!level;  //Active Low
            #endif
        #else
            level=0;
            gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=level;
        #endif
        }
        else
            gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;

        if(gRfiuUnitCntl[RFUnit].RFpara.MD_en)
        {
        #if ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
            if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 0)
                level = SIU_DAY_MODE;
            if(level==SIU_NIGHT_MODE)
            {
                if (gRfiuUnitCntl[RFUnit].RFpara.MD_en & 0x02)
                    gRfiuUnitCntl[RFUnit].RFpara.MD_Trig = MD_Diff;
                else
                    gRfiuUnitCntl[RFUnit].RFpara.MD_Trig = 0;
            }
            else
            {
                if (gRfiuUnitCntl[RFUnit].RFpara.MD_en & 0x01)
                    gRfiuUnitCntl[RFUnit].RFpara.MD_Trig = MD_Diff;
                else
                    gRfiuUnitCntl[RFUnit].RFpara.MD_Trig = 0;
            }
        #else
           #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=MD_Diff;
           #else
            gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=MD_Diff;
           #endif
        #endif
        }
        else
            gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=0;
        
    #if (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811 && PROJ_OPT == 12))
        if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig && gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
        {
        #if MD_DEBUG_ENA
            DEBUG_RFIU_P2("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
        #endif
            *TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
        }
        else
            *TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
        
    #elif (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612)   
        if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig && gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
        {
        #if (UI_LIGHT_SUPPORT == 1)
            if (iconflag[UI_MENU_SETIDX_LIGHT_ONOFF] == 0)
            {
                if ((uiInScheduleLight != 1) && (SIUMODE == SIU_NIGHT_MODE))
                {
                    //DEBUG_GREEN("PIR Trigger Start Light\n");
                    gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
                    uiCurrentLightTime = UI_TRIGGER_TIME;
                    rfiu_SetRXLightTrig(1);//trigger on
                }
            }
        #endif
        
        #if UI_CAMERA_ALARM_SUPPORT    
            if (iconflag[UI_MENU_SETIDX_CA_ONOFF] == 0)
            {
                if (uiInScheduleAlarm == 1)
                {
                    //DEBUG_GREEN("PIR Trigger Start Alarm \n");
                    gpioSetLevel(GPIO_GROUP_ALARM, GPIO_BIT_ALARM, 1);
                    uiCurrentAlarmTime = UI_TRIGGER_TIME;
                    rfiu_SetRXLightTrig(4);//trigger on
                }
            }
        #endif
        
        if (showmotion)
            DEBUG_GREEN("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);

            *TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
        }
        else
            *TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
        
    #elif (HW_BOARD_OPTION  == MR9160_TX_MAYON_MWL613)
        {
            static u8 checkPIR = 0;
            #if (PIR_PYD1588_TEST == 1)
            static u8 PIR_Trig_Pre_Status = 0;
            static u32 PIR_Trig_counter = 0;
            if (gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig != PIR_Trig_Pre_Status){
                if(gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig == 1)
                    PIR_Trig_counter++;
                    printf("[%d], PIR Trigger:%d\n", PIR_Trig_counter, gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig);
                    PIR_Trig_Pre_Status = gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig;
            }
            #endif
            if (((rfiuBatCamPIRTrig == 1) && (checkPIR == 0)) || (gRfiuUnitCntl[RFUnit].RFpara.MD_Trig && gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig))
            {
                if (rfiuBatCamPIRTrig == 1)//PIR開機要警報
                    checkPIR =1;
                
            #if (UI_LIGHT_SUPPORT == 1)
                if (iconflag[UI_MENU_SETIDX_LIGHT_ONOFF] == 0)
                {
                    if ((uiInScheduleLight != 1) && (SIUMODE == SIU_NIGHT_MODE))
                    {
                        //DEBUG_GREEN("PIR Trigger Start Light\n");
                        gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
                        //if (uiCurrentLightTime == 0)
                        uiCurrentLightTime = UI_TRIGGER_TIME;
                        rfiu_SetRXLightTrig(1);//trigger on
                    }
                }
            #endif
            
            #if UI_CAMERA_ALARM_SUPPORT    
                if (iconflag[UI_MENU_SETIDX_CA_ONOFF] == 0)
                {
                    if (uiInScheduleAlarm == 1)
                    {
                        //DEBUG_GREEN("PIR Trigger Start Alarm \n");
                        gpioSetLevel(GPIO_GROUP_ALARM, GPIO_BIT_ALARM, 1);
                        if (uiCurrentAlarmTime == 0)
                            uiCurrentAlarmTime = UI_TRIGGER_TIME;
                        rfiu_SetRXLightTrig(4);//trigger on
                    }
                }
            #endif

            if (showmotion)
                DEBUG_GREEN("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);

                *TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
            }
            else
                *TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
        }
    
    #else
        if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig || gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
        {
            #if (UI_LIGHT_SUPPORT == 1)
                if ((uiInScheduleLight == 1) || (uiInManualLight != 0))
                {
                    DEBUG_GPIO("PIR Trigger Cnange Dimmer\n");
                    uiMenuAction(UI_MENU_SETIDX_LIGHT_DIMMER);
                    uiSetTriggerDimmer = UI_TRIGGER_DIMMER_TIME;
                }
                else
                {
                    gpioGetLevel(GPIO_GROUP_WHITELED, GPIO_BIT_WHITELED, &dayLevel);

                    if (dayLevel == 1)
                    {
                        if (uiSetLightDuration != 0)
                        {
                            uiCurrentLightTime = uiSetLightDuration;
                            uiMenuAction(UI_MENU_SETIDX_LIGHT_DIMMER);
                            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 1);
                            rfiu_SetRXLightTrig(1);
                        }
                    }
                }
            #endif
        #if MD_DEBUG_ENA
            DEBUG_RFIU_P2("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
        #endif

            *TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
        }
        else
            *TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
     #endif
    }
 #endif      

}
#endif


//------------------------------------TX code-----------------------------------//
#if( RFIU_TEST || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||\
    (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

 #if TX_FW_UPDATE_SUPPORT
    int rfiuFwUpdCheckDoneCome(DEF_REGRFIU_CFG *pRfiuPara)
    {
        int i;
        unsigned int CmdID;
        //-------------//
        
        for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
        {
            CmdID=(pRfiuPara->Pkt_Grp_offset[i] >> 7);
            if((pRfiuPara->PktMap[2*i] & RFI_FWUPD_DONE_ADDR_CHEKBIT) && ( CmdID == RFI_CMD_ADDR_OFFSET))
            {
               return 1;
            }
        }

        return 0;
    }

    void rfiuTXSetVersionInfo(unsigned char *pp)
    {
        u8 Year;
        u8 Month;
        u8 Day;
        u16 TXtime;
        u16 Version;
        Year  = (((uiVersionTime[2] - 0x30) * 10) + (uiVersionTime[3] - 0x30));
        Month = (((uiVersionTime[4] - 0x30) * 10) + (uiVersionTime[5] - 0x30));
        Day   = (((uiVersionTime[6] - 0x30) * 10) + (uiVersionTime[7] - 0x30));        
        TXtime =  ((Year << 9) | ((Month << 5) & 0x1E0) | Day);  
#ifndef UI_PROJ_OPT
#ifndef PROJ_OPT
        Version = ((HW_BOARD_OPTION << 4) | 0);
#else
        Version = ((HW_BOARD_OPTION << 4) | PROJ_OPT);
#endif
#else
        Version = ((HW_BOARD_OPTION << 4) | UI_PROJ_OPT);
#endif
        pp[30] = TXtime >> 8;
        pp[29] = TXtime & 0xffff;
        pp[28] = Version >> 8;        
        pp[27] = Version & 0xffff;
    }

    int rfiuFwUpdSend_Start_State(   int RFUnit,
                                            unsigned int Vitbi_en, 
                                            unsigned int RS_mode, 
                                            unsigned int Vitbi_mode,
                                            unsigned int SyncWord,
                                            unsigned int CustomerID,
                                            unsigned int UserData
                                        )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        int i;
        //------//
        
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
                       
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

        for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
        {
            pRfiuPara_Tx->PktMap[2*i]      =RFI_FWUPD_START_ADDR_CHEKBIT;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_SYNC_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        RfiuReset(RFUnit & 0x01);
        rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);  //Tx
#else
        RfiuReset(RFUnit);
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);  //Tx
#endif

        return 1;
    }
 
    int rfiuFwUpdPktMapUpdate(int RFUnit)
    {
        int i,j,index;    
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        DEF_RFIU_GROUP_MAP ACKMapGrpIdx[RFIU_USEPKTMAP_MAX/2];
        DEF_RFIU_GROUP_MAP TmpMapGrpIdx[RFIU_USEPKTMAP_MAX/2];
        unsigned int pktmap; 
        int bitcount;
        //=====//
        
        pRfiuPara_Rx = &(gRfiuParm_Rx[RFUnit]);
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           TmpMapGrpIdx[i].Group= (pRfiuPara_Rx->Pkt_Grp_offset[i] >> 13) & 0x7ff;
           TmpMapGrpIdx[i].Map0 =pRfiuPara_Rx->PktMap[i*2];
           TmpMapGrpIdx[i].Map1 =pRfiuPara_Rx->PktMap[i*2+1];
        }

        //-------//
        index=0;
        memset(ACKMapGrpIdx,0,sizeof(DEF_RFIU_GROUP_MAP)*RFIU_USEPKTMAP_MAX/2);
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           if( (TmpMapGrpIdx[i].Map0 !=0) || (TmpMapGrpIdx[i].Map1 != 0) )
           {
               ACKMapGrpIdx[index].Group=TmpMapGrpIdx[i].Group;
               ACKMapGrpIdx[index].Map0=TmpMapGrpIdx[i].Map0;
               ACKMapGrpIdx[index].Map1=TmpMapGrpIdx[i].Map1;
               index ++;
           }

        }
        //-------//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           if( 
                 (i<(RFIU_USEPKTMAP_MAX/2-1)) && (ACKMapGrpIdx[i].Group==ACKMapGrpIdx[i+1].Group)
              )
           {
               ACKMapGrpIdx[i+1].Map0 |= ACKMapGrpIdx[i].Map0;
               ACKMapGrpIdx[i+1].Map1 |= ACKMapGrpIdx[i].Map1;
               continue;
           }
           
           if( (ACKMapGrpIdx[i].Map0 == 0) && (ACKMapGrpIdx[i].Map1 == 0) )
             continue;
           
           gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap0 ^= (ACKMapGrpIdx[i].Map0);
           gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap1 ^= (ACKMapGrpIdx[i].Map1);

           bitcount =0;           
           pktmap=gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap0;
           if(pktmap != 0)
           {
               for(j=0;j<32;j++)
               {
                  if(pktmap & 0x01)
                      bitcount ++;
                  pktmap >>=1;
               }
           }
           
           pktmap=gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap1;
           if(pktmap != 0)
           {
               for(j=0;j<32;j++)
               {
                  if(pktmap & 0x01)
                      bitcount ++;
                  pktmap >>=1;

               }
           }

           gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktCount=(unsigned char)bitcount;
        }
        

    }

    int rfiuFwUpdSaveTxFW2Flash(int RFUnit,int codeSize)
    {
        u8* codeAddr;
        int stutus=1;
        //-----------------//
        codeAddr=(unsigned char *)rfiuOperBuf[RFUnit];
        stutus=spiallCodeUpdate(codeAddr, codeSize);
        
        return stutus;
    }

  #endif

int rfiuGetACK2PacketMap_UpdateMap(unsigned char RFUnit,unsigned char *ACKBufAddr,
                                                 int *pRX_RecvTotalPktCnt,int *pRX_RecvDataPktCnt,
                                                 int *pRxBufWritePtr)
{
    unsigned int *pp;
    int i,j,count,bitcount,index;
    DEF_RFIU_GROUP_MAP ACKMapGrpIdx[RFIU_USEPKTMAP_MAX/2];
    DEF_RFIU_GROUP_MAP TmpMapGrpIdx[RFIU_USEPKTMAP_MAX/2];
    unsigned int pktmap,sum;
    int RX_TimeCheck;
    int DataPktCnt;
    unsigned int temp;
    unsigned int Rx_CustomerCode;
    unsigned int Tx_CustomerCode;
    
    
    //=====//
    count=0;  
    DataPktCnt=0;
    Tx_CustomerCode=RF_CUSTOMERID_SET;
    
    pp  = (unsigned int *)(ACKBufAddr);
    while( (count <32) && (*pp != 0xa55aaa55))
    {
        //---Read Group value---//       
        for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
        {
            *pp ^= 0x5a5aa5a5;
	 #if RF_CMD_EN
            TmpMapGrpIdx[i*2].Group= ((*pp >>0) & 0x07ff);
            TmpMapGrpIdx[i*2+1].Group= ((*pp >>11) & 0x07ff); 
            gRfiuUnitCntl[RFUnit].RX_CMD_Data[i] = ((*pp >>24) & 0xff); //RX command      
	 #else
            TmpMapGrpIdx[i*2].Group= ((*pp >>0) & 0x0ffff)>>5;
            TmpMapGrpIdx[i*2+1].Group= ((*pp >>16) & 0x0ffff)>>5;
	 #endif		                  
			pp ++;
            count ++;
        }
        
        //---Read Map value---//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           TmpMapGrpIdx[i].Map0 =((*pp) ^ 0x5a5aa5a5);
           pp ++;
           count ++;
           //------//
           TmpMapGrpIdx[i].Map1 = ((*pp) ^ 0x5a5aa5a5);
           pp ++;
           count ++;

        }
        //--Read password--//
	#if RF_CMD_EN        
        for(i=0;i<RFIU_PASSWORD_MAX;i++)
        {
           gRfiuUnitCntl[RFUnit].RX_PASSWORD[i]=((*pp) ^ 0x5a5aa5a5); 
           pp ++;
           count ++;
        }    
        //--RX Cmd Data[5~7]---//  
        temp=((*pp) ^ 0x5a5aa5a5);
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= temp & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= (temp>>8) & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]= (temp>>16) & 0x0ff;
        *pRxBufWritePtr=(temp>>24) & 0x0ff;
        
        pp ++;
        count ++;

        //--Status--//
        temp=(*pp) ^ 0x5a5aa5a5;;
        rfiuCmdRxStatus[RFUnit].CamNum= temp & 0x07; 
        gRfiuUnitCntl[RFUnit].TXSendDataUse3M= (temp>>3) & 0x01;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]= (temp>>8) & 0x0ff;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[9]= (temp>>16) & 0x0ff;
        //gRfiuUnitCntl[RFUnit].RX_CMD_Data[10]= (temp>>24) & 0x0ff;
        pp ++;
        count ++;

        //--Customer ID---//
        Rx_CustomerCode=*pp ^ 0x5a5aa5a5;
        pp ++;
        count ++;   
	#endif	
        //---RX timer---//
        RX_TimeCheck= *pp;
        pp ++;
        count ++;

        //---RX receive packet count---//
        temp=(*pp) ^ 0x5a5aa5a5;
        *pRX_RecvTotalPktCnt= temp & 0x0ffff;

        pp ++;
        count ++;
    }

    //-------//
    index=0;
    memset(ACKMapGrpIdx,0,sizeof(DEF_RFIU_GROUP_MAP)*RFIU_USEPKTMAP_MAX/2);
    for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
    {
       if( (TmpMapGrpIdx[i].Map0 !=0) || (TmpMapGrpIdx[i].Map1 != 0) )
       {
           ACKMapGrpIdx[index].Group=TmpMapGrpIdx[i].Group;
           ACKMapGrpIdx[index].Map0=TmpMapGrpIdx[i].Map0;
           ACKMapGrpIdx[index].Map1=TmpMapGrpIdx[i].Map1;
           index ++;
       }

    }
    //-------//
    for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
    {
    #if 1
       if( (i<(RFIU_USEPKTMAP_MAX/2-1)) && 
           (ACKMapGrpIdx[i].Group==ACKMapGrpIdx[i+1].Group)
          )
       {
           ACKMapGrpIdx[i+1].Map0 |= ACKMapGrpIdx[i].Map0;
           ACKMapGrpIdx[i+1].Map1 |= ACKMapGrpIdx[i].Map1;
           continue;
       }
    #endif
    #if 1
       if( (ACKMapGrpIdx[i].Map0 == 0) && (ACKMapGrpIdx[i].Map1 == 0) )
         continue;
    #endif   
       gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap0 ^= (ACKMapGrpIdx[i].Map0);
       gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap1 ^= (ACKMapGrpIdx[i].Map1);

       bitcount =0;           
       pktmap=gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap0;
       if(pktmap != 0)
       {
           for(j=0;j<32;j++)
           {
              if(pktmap & 0x01)
                  bitcount ++;
              pktmap >>=1;
           }
       }
       
       pktmap=gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktMap1;
       if(pktmap != 0)
       {
           for(j=0;j<32;j++)
           {
              if(pktmap & 0x01)
                  bitcount ++;
              pktmap >>=1;

           }
       }

       gRfiuUnitCntl[RFUnit].TxPktMap[ACKMapGrpIdx[i].Group].PktCount=(unsigned char)bitcount;
       DataPktCnt +=bitcount;
       //DEBUG_RFIU_P2("[%d,%d] ",i,bitcount);

       
    }

    *pRX_RecvDataPktCnt=DataPktCnt;
    
    if( (Rx_CustomerCode==RFI_CUSTOMER_ID_ALLPASS) || (Rx_CustomerCode==Tx_CustomerCode)  || (Tx_CustomerCode==RFI_CUSTOMER_ID_ALLPASS) )
    {
       return RX_TimeCheck;
    }   
    else
    {
       DEBUG_RFIU_P2("Customer ID mismatch!\n");
       return 0xffffffff;
    }

}

int rfiuGetACK2TimeCheck(unsigned char RFUnit,unsigned char *ACKBufAddr,
                         int *pRX_RecvTotalPktCnt,int *pRX_RecvDataPktCnt)
{
    unsigned int *pp;
    int i,j,count,bitcount;
    int RX_TimeCheck;
    unsigned int temp;
    unsigned int Rx_CustomerCode;
    unsigned int Tx_CustomerCode;

    
    //=====//
    count=0;  
    Tx_CustomerCode=RF_CUSTOMERID_SET;
    pp  = (unsigned int *)(ACKBufAddr);
    while( (count <32) && (*pp != 0xa55aaa55))
    {
        //---Read Group value---//       
        for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
        {
            pp ++;
            count ++;
        }
        //---Read Map value---//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           pp ++;
           count ++;
           //------//
           pp ++;
           count ++;
           
        }
        // new add
        //--Read password--//
   #if RF_CMD_EN        
        for(i=0;i<RFIU_PASSWORD_MAX;i++)
        {
           pp ++;
           count ++;
        }  
        
        //--RX Command--//
        pp ++;
        count ++;

        //--RX Status--//
        temp=(*pp) ^ 0x5a5aa5a5;;
        rfiuCmdRxStatus[RFUnit].CamNum= temp & 0x07; 
        gRfiuUnitCntl[RFUnit].TXSendDataUse3M= (temp>>3) & 0x01;
        pp ++;
        count ++;

        //--Customer ID--//
        Rx_CustomerCode=*pp ^ 0x5a5aa5a5;
        pp ++;
        count ++;
   #endif      
        //---RX timer---//
        RX_TimeCheck=*pp;
        pp ++;
        count ++;

        //---RX receive packet count---//
        *pRX_RecvTotalPktCnt= 0;
        pp ++;
        count ++;
        
    }

    *pRX_RecvDataPktCnt=0;

    if( (Rx_CustomerCode==RFI_CUSTOMER_ID_ALLPASS) || (Rx_CustomerCode==Tx_CustomerCode) || (Tx_CustomerCode==RFI_CUSTOMER_ID_ALLPASS) )
    {
       return RX_TimeCheck;
    }   
    else
    {
       DEBUG_RFIU_P2("Customer ID mismatch!\n");
       return 0xffffffff;
    }

}

int rfiuPutInfo2TXCMD(int RFUnit,unsigned char *ACKBufAddr1,unsigned char *ACKBufAddr2)
{
    unsigned int *pp , *qq;
    int i,count;
    unsigned int  cpu_sr = 0;
    //=====//
    
    count=0;
    pp  = (unsigned int *)(ACKBufAddr1);

    *pp = (unsigned int )gRfiuUnitCntl[RFUnit].TXCmd_Type;
    pp ++;
    count ++;

    //5 words
    if(gRfiuUnitCntl[RFUnit].TXCmd_Type & RFTXCMD_SEND_DATA)
    {
        for(i=0;i<RFIU_TXCMDDATA_MAX;i++)
        {
           *pp = (unsigned int )gRfiuUnitCntl[RFUnit].TX_CMDPara[i];
           pp ++;
           count ++;
        }
    }

    // 12 words//
    if(gRfiuUnitCntl[RFUnit].TXCmd_Type & RFTXCMD_SEND_AUDIORETMAP)
    {
        for(i=0;i<RFI_AUDIO_RET_GRPNUM;i++)
        {
           *pp = (unsigned int )gRfiuUnitCntl[RFUnit].AudioRetPktMap[i].Group;
           pp ++;
           count ++;

           *pp = (unsigned int )gRfiuUnitCntl[RFUnit].AudioRetPktMap[i].Map0;
           pp ++;
           count ++;

           *pp = (unsigned int )gRfiuUnitCntl[RFUnit].AudioRetPktMap[i].Map1;
           pp ++;
           count ++;
        }
    }

    
    for(i=count;i<32;i++)
    {
       *pp = 0xa55aaa55;//magic number for End.
       pp ++;
       count ++;
    }
    
    //====Copy CMD1 to CMD2====//
    pp  = (unsigned int *)(ACKBufAddr1);
    qq  = (unsigned int *)(ACKBufAddr2);
    for(i=0;i<32;i++)
    {
       *qq = *pp ;
       pp ++;
	   qq++;
    }	
    //========================//
    
    gRfiuUnitCntl[RFUnit].TXCmd_Type=RFTXCMD_SEND_NONE;
    
}



int rfiuTxUpdatePktMap(
                                 unsigned char RFUnit,
                                 int DatPktRecvFlag,
                                 DEF_RFIU_USRDATA *pCtrlPara,
                                 DEF_RFIU_USRDATA *pCtrlPara_next,
                                 int *pRX_RecvPktCnt,
                                 int *pRX_recvDataPktCnt,
                                 int prev_DAT_CH_sel,
                                 DEF_RFIU_DAT_CH_STATISTICS TX_CH_Stat[],
                                 DEF_RFIU_FEC_TYPE_STATISTICS TX_FEC_Stat[]
                             )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    unsigned int nextBufReadPtr,CurrBufWritePtr,tempptr,tmpPktcnt;
    int Grpcnt,i,j,temp;
    int Repeat_flag,WinCnt,RemainGrp,UsedGrp;
    int RX_TimeCheck;
    unsigned int PktMap0,PktMap1;
    unsigned char *GoodDataCH;
    unsigned int DivF;
    int SortTab[RFI_DAT_CH_MAX];
    int GrpWROffset;
    unsigned int  cpu_sr = 0; 
    unsigned int PktMapMask[9][2]={
                                     {0x00000000,0x00000000},
                                     {0x000000ff,0x00000000},
                                     {0x0000ffff,0x00000000},
                                     {0x00ffffff,0x00000000},
                                     {0xffffffff,0x00000000},
                                     {0xffffffff,0x000000ff},
                                     {0xffffffff,0x0000ffff},
                                     {0xffffffff,0x00ffffff},   
                                     {0xffffffff,0xffffffff},
                                  };
    unsigned int Map0Mask,Map1Mask;
    int FEC_Level;
    int FEC_Weight[RFI_FEC_TYPE_MAX]={145,141,134,128};  // x 128, 補償coding efficicy
    int RxBufWritePtr;
    int PtrDiff;
    //-----------------------------------------------//
    GoodDataCH = gRfiuUnitCntl[RFUnit].GoodDataCHTab;
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);

    if(gRfiuUnitCntl[RFUnit].OpState==RFIU_TX_STATE_INIT)
    {
    #if RFIU_TEST
        #if(RFI_TEST_TXRX_COMMU && RFI_TEST_WRAP_OnCOMMU)
            //do nothing!
        #elif(RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL)
            //do nothing!
        #else
        pRfiuPara_Tx->Pkt_Grp_offset[0]    =0;
        pRfiuPara_Tx->PktMap[0]            =0xffffffff;
        pRfiuPara_Tx->PktMap[1]            =0xffffffff;
        gRfiuUnitCntl[RFUnit].TxPktMap[0].RetryCount        =1;
        
        pRfiuPara_Tx->Pkt_Grp_offset[1]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*1;
        pRfiuPara_Tx->PktMap[2]            =0xffffffff;
        pRfiuPara_Tx->PktMap[3]            =0xffffffff;
        gRfiuUnitCntl[RFUnit].TxPktMap[1].RetryCount        =1;
        
        pRfiuPara_Tx->Pkt_Grp_offset[2]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*2;
        pRfiuPara_Tx->PktMap[4]            =0xffffffff;
        pRfiuPara_Tx->PktMap[5]            =0xffffffff;
        gRfiuUnitCntl[RFUnit].TxPktMap[2].RetryCount        =1;
        
        pRfiuPara_Tx->Pkt_Grp_offset[3]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*3;
        pRfiuPara_Tx->PktMap[6]            =0xffffffff;
        pRfiuPara_Tx->PktMap[7]            =0xffffffff;
        gRfiuUnitCntl[RFUnit].TxPktMap[3].RetryCount        =1;
        #endif
     #endif
     
        pRfiuPara_Tx->TxRxPktNum           =RFI_MAX_BURST_NUM;  
        pRfiuPara_Tx->DummyPktNum          =0;  
        *pRX_RecvPktCnt=0;


        return -1;
    }
    else
    {  
        if(DatPktRecvFlag)
            RX_TimeCheck=rfiuGetACK2PacketMap_UpdateMap(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,pRX_RecvPktCnt,pRX_recvDataPktCnt,&RxBufWritePtr);
        else
            RX_TimeCheck=rfiuGetACK2TimeCheck(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,
                                              pRX_RecvPktCnt,pRX_recvDataPktCnt);
        
        //------Statistic correct rate of TX DATA channel------//
        TX_CH_Stat[prev_DAT_CH_sel].RecvPktNum += (*pRX_RecvPktCnt);
        TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum += (*pRX_RecvPktCnt);

        TX_FEC_Stat[gRfiuUnitCntl[RFUnit].PrevTxFEC_Level].RecvPktNum +=(*pRX_RecvPktCnt);
        
        if(*pRX_RecvPktCnt == 0)
        {
           TX_CH_Stat[prev_DAT_CH_sel].BrokenNum +=1;
           TX_CH_Stat[RFI_DAT_CH_MAX].BrokenNum +=1;
        }
    #if DEBUG_TX_WARNING_MSG
        if( (*pRX_RecvPktCnt < 10) && (*pRX_RecvPktCnt > 2 ) ) 
        {
        }
    #endif

        //---Analysis Data channel and Find out Good data channel---//
        if(TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum == 0)
        {
           gRfiuUnitCntl[RFUnit].DataCHCnt=0;
		   gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;           
        }
        else if( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum & (RFIU_ANALYSIS_CH_INTV-1) ) == 0)
        {
           //Normalize channel data
           for(i=0;i<RFI_DAT_CH_MAX;i++)
           {
               if(TX_CH_Stat[i].SentPktNum > 256)
               {
                  DivF=(TX_CH_Stat[i].SentPktNum+8)>>4;
                  TX_CH_Stat[i].SentPktNum=256;
                  TX_CH_Stat[i].RecvPktNum= (TX_CH_Stat[i].RecvPktNum<<4)/DivF;
               }
               
               SortTab[i]=(int)(TX_CH_Stat[i].RecvPktNum);
               DEBUG_RFIU_P("%d ",SortTab[i]);
           }
           DEBUG_RFIU_P("\n");
           
           //Find out good channel
           gRfiuUnitCntl[RFUnit].GoodDataCHNum=0;
           for(j=0;j<RFI_DAT_CH_MAX;j++)
           {
              temp=-1;
              for(i=0;i<RFI_DAT_CH_MAX;i++)
              {
                 if(SortTab[i] > temp)
                 {
                    GoodDataCH[j]=i;
                    temp=SortTab[i];
                 }
              }
			  
			  if( SortTab[GoodDataCH[j]] > RFI_GOOD_CH_JDG_LV )
				 gRfiuUnitCntl[RFUnit].GoodDataCHNum += 1;
              SortTab[GoodDataCH[j]]=-2;
           }
           //
           gRfiuUnitCntl[RFUnit].DataCHCnt=0;
		   if(gRfiuUnitCntl[RFUnit].GoodDataCHNum < 4)
		   	 gRfiuUnitCntl[RFUnit].GoodDataCHNum=4;
		   DEBUG_RFIU_P("-->GoodDataCHNum=%d \n",gRfiuUnitCntl[RFUnit].GoodDataCHNum);
        }
        
        //--Suggest next data channel--//
        if( ( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum/RFIU_ANALYSIS_CH_INTV) & (64-1) ) == 0)
        {
           gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH = gRfiuUnitCntl[RFUnit].DataCHCnt & (RFI_DAT_CH_MAX-1); 
        }
        else
        {
           gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH = GoodDataCH[gRfiuUnitCntl[RFUnit].DataCHCnt % gRfiuUnitCntl[RFUnit].GoodDataCHNum ]; 
        }	
        gRfiuUnitCntl[RFUnit].DataCHCnt ++;


        //------Cal Group shift-----//
        if(DatPktRecvFlag)
        {
            pCtrlPara_next->GrpShift=0; 
        #if 1
            if(RxBufWritePtr != gRfiuUnitCntl[RFUnit].BufReadPtr)
            {
                DEBUG_RFIU_P2("TX/RX-%d Buf r/w ptr mismatch:%d,%d\n",RFUnit,gRfiuUnitCntl[RFUnit].BufReadPtr,RxBufWritePtr);
                if(gRfiuUnitCntl[RFUnit].BufReadPtr >= RxBufWritePtr)
                {
                   PtrDiff= gRfiuUnitCntl[RFUnit].BufReadPtr-RxBufWritePtr;
                }
                else
                {
                   PtrDiff= gRfiuUnitCntl[RFUnit].BufReadPtr+RFI_BUF_SIZE_GRPUNIT-RxBufWritePtr;
                }
                if(PtrDiff>63)
                   PtrDiff -= RFI_BUF_SIZE_GRPUNIT;

                pCtrlPara_next->GrpShift=PtrDiff;
            }
        #endif
        }
        OS_ENTER_CRITICAL();
        CurrBufWritePtr=gRfiuUnitCntl[RFUnit].BufWritePtr;
        //GrpWROffset = (CurrBufWritePtr + RFI_BUF_SIZE_GRPUNIT - gRfiuUnitCntl[RFUnit].BufReadPtr) & RFI_BUF_SIZE_MASK; 
        GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,gRfiuUnitCntl[RFUnit].BufReadPtr,RFI_BUF_SIZE_GRPUNIT);

        OS_EXIT_CRITICAL();
        while( (gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK].PktCount==0) && (GrpWROffset>0) )
        {
           pCtrlPara_next->GrpShift ++;
           OS_ENTER_CRITICAL();
           gRfiuUnitCntl[RFUnit].BufReadPtr= (gRfiuUnitCntl[RFUnit].BufReadPtr + 1) & RFI_BUF_SIZE_MASK ;
           //GrpWROffset = (CurrBufWritePtr + RFI_BUF_SIZE_GRPUNIT - gRfiuUnitCntl[RFUnit].BufReadPtr) & RFI_BUF_SIZE_MASK; 
           GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,gRfiuUnitCntl[RFUnit].BufReadPtr,RFI_BUF_SIZE_GRPUNIT);
           OS_EXIT_CRITICAL();
        }

        
        if(pCtrlPara_next->GrpShift>63)
        {
           DEBUG_RFIU_P2("\n=======>Warning Group shift is too much: %d\n",pCtrlPara_next->GrpShift);  
           pCtrlPara_next->GrpShift=63;
        }

        if(pCtrlPara_next->GrpShift < 0)
        {
           pCtrlPara_next->GrpShift=0;
        }
        
        pRfiuPara_Tx->TxRxPktNum   =0;
        pRfiuPara_Tx->DummyPktNum  =0;  

        OS_ENTER_CRITICAL();
        nextBufReadPtr = gRfiuUnitCntl[RFUnit].BufReadPtr;
        OS_EXIT_CRITICAL();
        
        //-----Cal Group Divisin status---//
        pCtrlPara_next->GrpDivs=0;
        //GrpWROffset = (CurrBufWritePtr + RFI_BUF_SIZE_GRPUNIT - nextBufReadPtr) & RFI_BUF_SIZE_MASK;
        GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
        if(GrpWROffset>0)
        {
            PktMap0=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
            PktMap1=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;

            for(i=0;i<8;i++)
            {
                if(i<4)
                {
                    if((PktMap0 & 0x0ff)==0)
                        pCtrlPara_next->GrpDivs ++;
                    else
                        break;
                    PktMap0 >>=8;
                }
                else
                {
                    if((PktMap1 & 0x0ff)==0)
                        pCtrlPara_next->GrpDivs ++;
                    else
                        break;
                    PktMap1 >>=8;
                }

            }
        }
    #if RFI_LOW_DELAY_ON    
        else if(GrpWROffset == 0)
        {
            PktMap0=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
            PktMap1=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;

            for(i=0;i<gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv;i++)
            {
                if(i<4)
                {
                    if((PktMap0 & 0x0ff)==0)
                        pCtrlPara_next->GrpDivs ++;
                    else
                        break;
                    PktMap0 >>=8;
                }
                else
                {
                    if((PktMap1 & 0x0ff)==0)
                        pCtrlPara_next->GrpDivs ++;
                    else
                        break;
                    PktMap1 >>=8;
                }

            }
        }
    #endif     
        //---Cal Seqence number---//
        if(DatPktRecvFlag)
           pCtrlPara_next->SeqenceNum ++; 

        //-------//
        Grpcnt=0;

		
        //====== TX -> RX  command have 4  command PKTs at 2-group
  #if(RF_CMD_EN)
        if((gRfiuUnitCntl[RFUnit].TXCmd_en) ==1)
        {
           pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE;        
           pRfiuPara_Tx->PktMap[Grpcnt*2]      =(RFI_TXCMD1_ADDR_CHEKBIT | RFI_TXCMD2_ADDR_CHEKBIT);
           pRfiuPara_Tx->PktMap[Grpcnt*2+1]    =0x00000000; 
           pRfiuPara_Tx->TxRxPktNum           +=2;
           Grpcnt ++;	
		   
           pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE;        
           pRfiuPara_Tx->PktMap[Grpcnt*2]      =(RFI_TXCMD1_ADDR_CHEKBIT | RFI_TXCMD2_ADDR_CHEKBIT);
           pRfiuPara_Tx->PktMap[Grpcnt*2+1]    =0x00000000; 
           pRfiuPara_Tx->TxRxPktNum           +=2;
           Grpcnt ++; 

		   //put cmand information
           rfiuPutInfo2TXCMD(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_TXCMD1_ADDR_OFFSET*128,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_TXCMD2_ADDR_OFFSET*128); 
		   
        }
  #endif	
        //=====decide Coding Level=====//
        if(TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum == 0)
        {          
           gRfiuUnitCntl[RFUnit].PrevTxFEC_Level=0;
           gRfiuUnitCntl[RFUnit].TxFEC_Level=0;
           FEC_Level=0;
        }
        else if( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum & (RFIU_ANALYSIS_FEC_INTV-1) ) == 0)
        {
           //Normalize channel data
           DEBUG_RFIU_P("\n");
           for(i=0;i<RFI_FEC_TYPE_MAX;i++)
           {
               if(TX_FEC_Stat[i].SentPktNum > 256)
               {
                  DivF=(TX_FEC_Stat[i].SentPktNum+8)>>4;
                  TX_FEC_Stat[i].SentPktNum=256;
                  TX_FEC_Stat[i].RecvPktNum= (TX_FEC_Stat[i].RecvPktNum<<4)/DivF;
               }
               
               SortTab[i]=(int)((TX_FEC_Stat[i].RecvPktNum * FEC_Weight[i]) >>7);
               DEBUG_RFIU_P("%d ",SortTab[i]);
           }
           
           //Find the best
           temp=-1;
           for(i=0;i<RFI_DAT_CH_MAX;i++)
           {
             if(SortTab[i] > temp)
             {
                gRfiuUnitCntl[RFUnit].TxFEC_Level=i;
                temp=SortTab[i];
             }
           }
		  		  
        }


        if( ( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum/RFIU_ANALYSIS_FEC_INTV) & (32-1) ) == 0)
        {
            FEC_Level= TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum % RFI_FEC_TYPE_MAX;
        }
        else
        {
            if( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum & 0x7) == 0 )
               FEC_Level= (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum >>3) % RFI_FEC_TYPE_MAX;
            else
            {
               FEC_Level= gRfiuUnitCntl[RFUnit].TxFEC_Level;
               //For exception//
               if(gRfiuUnitCntl[RFUnit].TX_PktCrtRate <10)
               {
                   FEC_Level=3;
               }
            }
        }	
  
		//===Do Repeat first burst===//
		gRfiuUnitCntl[RFUnit].TxBufFullness=0;
        if(gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount < 3)
        {
           Repeat_flag=1;
        }                
        else if(gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount <6)
        {  
           Repeat_flag=2;
        }        
        else if( gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount <9 )
        {
           Repeat_flag=2;
           FEC_Level=2;
        }
        else 
        {
           Repeat_flag=3;
           FEC_Level=3;
        }
  
        switch(FEC_Level)
        {
            case 0:
                pCtrlPara_next->Vitbi_en=0;
                pCtrlPara_next->Vitbi_sel=0;
                pCtrlPara_next->RS_sel=0;
                break;

            case 1:
                pCtrlPara_next->Vitbi_en=0;
                pCtrlPara_next->Vitbi_sel=0;
                pCtrlPara_next->RS_sel=1;
                break;

            case 2:
                pCtrlPara_next->Vitbi_en=0;
                pCtrlPara_next->Vitbi_sel=0;
                pCtrlPara_next->RS_sel=2;
                break;

            case 3:
            default:
                pCtrlPara_next->Vitbi_en=0;
                pCtrlPara_next->Vitbi_sel=0;
                pCtrlPara_next->RS_sel=3;
                break;

        }
        gRfiuUnitCntl[RFUnit].PrevTxFEC_Level=FEC_Level;

        
        GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);           
        if(GrpWROffset > 0)
        {
           for(i=0;i<Repeat_flag;i++)
           {
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               Grpcnt ++;
           }
        }
    #if RFI_LOW_DELAY_ON    
        else if(GrpWROffset == 0)
        {
           Map0Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][0];
           Map1Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][1];

           for(i=0;i<Repeat_flag;i++)
           {
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0 & Map0Mask;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1 & Map1Mask;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               Grpcnt ++;
           }
           
        }
    #endif   
        
        if(GrpWROffset>0)
            gRfiuUnitCntl[RFUnit].TxBufFullness += gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;

        if(pRfiuPara_Tx->TxRxPktNum < RFI_MAX_BURST_NUM)
           gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount +=1;
        nextBufReadPtr = (nextBufReadPtr + 1) & RFI_BUF_SIZE_MASK;           

        //===Do Repeat 2nd burst===//
        GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
        while((gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount==0) && (GrpWROffset>0))
        {
           nextBufReadPtr = (nextBufReadPtr+1) & RFI_BUF_SIZE_MASK;
           GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
        }
        if(gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount < 6)
        {
           Repeat_flag=1;
        }                                       
        else 
        {
           Repeat_flag=2;
        }
        if(GrpWROffset > 0)
        {
           for(i=0;i<Repeat_flag;i++)
           {
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               Grpcnt ++;
           }
        }
    #if RFI_LOW_DELAY_ON    
        else if(GrpWROffset == 0)
        {
           Map0Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][0];
           Map1Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][1];

           for(i=0;i<Repeat_flag;i++)
           {
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0 & Map0Mask;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1 & Map1Mask;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               Grpcnt ++;
           }
           
        }
    #endif   

        if(GrpWROffset>0)
            gRfiuUnitCntl[RFUnit].TxBufFullness += gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;

        if(pRfiuPara_Tx->TxRxPktNum < RFI_MAX_BURST_NUM)
           gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount +=1;
        nextBufReadPtr = (nextBufReadPtr + 1) & RFI_BUF_SIZE_MASK;           
        
        //-------------------------//
        Repeat_flag=0;
        tempptr=nextBufReadPtr;
        tmpPktcnt=0;
        WinCnt=1;
        RemainGrp=0;
		UsedGrp=0;
        
        for(i=Grpcnt;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
            GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,tempptr,RFI_BUF_SIZE_GRPUNIT);
            while((gRfiuUnitCntl[RFUnit].TxPktMap[tempptr & RFI_BUF_SIZE_MASK].PktCount==0) && (GrpWROffset>0))
            {
              tempptr = (tempptr + 1) & RFI_BUF_SIZE_MASK;
              WinCnt ++;
              GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,tempptr,RFI_BUF_SIZE_GRPUNIT);
            }

            GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,tempptr,RFI_BUF_SIZE_GRPUNIT);
            if(GrpWROffset>0)
            {
               tmpPktcnt +=gRfiuUnitCntl[RFUnit].TxPktMap[tempptr & RFI_BUF_SIZE_MASK].PktCount;
               tempptr = (tempptr + 1) & RFI_BUF_SIZE_MASK;
               WinCnt ++;
			   UsedGrp ++;
            }
         #if RFI_LOW_DELAY_ON  
            else if(GrpWROffset==0)
            {
               tmpPktcnt +=gRfiuUnitCntl[RFUnit].TxPktMap[tempptr & RFI_BUF_SIZE_MASK].PktCount;
               tempptr = (tempptr + 1) & RFI_BUF_SIZE_MASK;
               WinCnt ++;
			   UsedGrp ++;
            }  
         #endif
            else
               RemainGrp ++;
            
            if(tmpPktcnt+pRfiuPara_Tx->TxRxPktNum > RFI_MAX_BURST_NUM)
                break;
        }

        if(WinCnt>16)
        {
            Repeat_flag=1;             
            DEBUG_RFIU_P2("====>Repeat Group:WinCnt=%d!!\n",WinCnt);
        }


        if( ( (UsedGrp*4) <= (RemainGrp+UsedGrp) ) && (tmpPktcnt*4+pRfiuPara_Tx->TxRxPktNum <= RFI_MAX_BURST_NUM) )
        {
            Repeat_flag=3;
        }
        else if( ( (UsedGrp*3) <= (RemainGrp+UsedGrp) ) && (tmpPktcnt*3+pRfiuPara_Tx->TxRxPktNum <= RFI_MAX_BURST_NUM) )
        {
            Repeat_flag=2;
        }
        else if( ( (UsedGrp*2) <= (RemainGrp+UsedGrp) ) && (tmpPktcnt*2+pRfiuPara_Tx->TxRxPktNum <= RFI_MAX_BURST_NUM) )
        {
            Repeat_flag=1;
        }
        //------------------//
        for(;Grpcnt<RFIU_USEPKTMAP_MAX/2;Grpcnt++)
        {   
            GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
            while((gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount==0) && (GrpWROffset>0))
            {
               nextBufReadPtr = (nextBufReadPtr+1) & RFI_BUF_SIZE_MASK;
               GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
            }
            
            GrpWROffset = rfiuCalBufRemainCount(CurrBufWritePtr,nextBufReadPtr,RFI_BUF_SIZE_GRPUNIT);
            if(GrpWROffset>0)
            {
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;

               gRfiuUnitCntl[RFUnit].TxBufFullness += gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               //if(Repeat_flag)
               for(i=0;i<Repeat_flag;i++)
               {
                  Grpcnt++;
                  if(Grpcnt<RFIU_USEPKTMAP_MAX/2)
                  {
                     pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
                     pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0;
                     pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1;
                     pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
                  }
               }

               if(pRfiuPara_Tx->TxRxPktNum < RFI_MAX_BURST_NUM)
                  gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount +=1;
               nextBufReadPtr = (nextBufReadPtr+1) & RFI_BUF_SIZE_MASK;
            }
       #if RFI_LOW_DELAY_ON     
            else if(GrpWROffset == 0)
            {
               Map0Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][0];
               Map1Mask=PktMapMask[gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].WriteDiv][1];
               
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0 & Map0Mask;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1 & Map1Mask;
               pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
               
               //if(Repeat_flag)
               for(i=0;i<Repeat_flag;i++)
               {
                  Grpcnt++;
                  if(Grpcnt<RFIU_USEPKTMAP_MAX/2)
                  {
                     pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_GRP_INPKTUNIT*RFIU_PKT_SIZE*(nextBufReadPtr & RFI_BUF_SIZE_MASK); 
                     pRfiuPara_Tx->PktMap[Grpcnt*2]          =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap0 & Map0Mask;
                     pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktMap1 & Map1Mask;
                     pRfiuPara_Tx->TxRxPktNum               +=gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].PktCount;
                  }
               }

               if(pRfiuPara_Tx->TxRxPktNum < RFI_MAX_BURST_NUM)
                  gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount +=1;
               nextBufReadPtr = (nextBufReadPtr+1) & RFI_BUF_SIZE_MASK;
            }
       #endif     
            else
            {
            #if 1
               pRfiuPara_Tx->Pkt_Grp_offset[Grpcnt]    =RFI_DUMMY_ADDR_OFFSET*RFIU_PKT_SIZE;
               pRfiuPara_Tx->PktMap[Grpcnt*2]          =0xffffffff;
               pRfiuPara_Tx->PktMap[Grpcnt*2+1]        =0xffffffff;
               //pRfiuPara_Tx->TxRxPktNum          +=64;
               pRfiuPara_Tx->DummyPktNum         +=64;
            #endif
            }

            //if(pRfiuPara_Tx->TxRxPktNum > RFI_MAX_BURST_NUM)
                //break;
        } 

        if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
        {
            if(pRfiuPara_Tx->TxRxPktNum > RFI_MAX_BURST_NUM)
                pRfiuPara_Tx->TxRxPktNum = RFI_MAX_BURST_NUM;
            else if(pRfiuPara_Tx->TxRxPktNum < 96)
                pRfiuPara_Tx->TxRxPktNum=96;
        }
        else
        {
            if(pRfiuPara_Tx->TxRxPktNum > RFI_MAX_BURST_NUM)
                pRfiuPara_Tx->TxRxPktNum = RFI_MAX_BURST_NUM;
            else if(pRfiuPara_Tx->TxRxPktNum < 32)
                pRfiuPara_Tx->TxRxPktNum=32;
        }
		//DEBUG_RFIU_P2("%d ",pRfiuPara_Tx->TxRxPktNum);
            

    #if DEBUG_TX_WARNING_MSG
        if( (*pRX_RecvPktCnt < 10) && (*pRX_RecvPktCnt > 2) ) 
        {
        }
    #endif

        gRfiuUnitCntl[RFUnit].TxBufFullness = gRfiuUnitCntl[RFUnit].TxBufFullness >>3; //Lucian: 轉換成KByte. 1 packet=128 Bytes.
        TX_FEC_Stat[FEC_Level].SentPktNum += pRfiuPara_Tx->TxRxPktNum;
    
        return RX_TimeCheck;
    }

}

int rfiuTxSendDataState( unsigned char RFUnit,
                              unsigned int Vitbi_en, 
                              unsigned int RS_mode, 
                              unsigned int Vitbi_mode,
                              unsigned int SyncWord,
                              unsigned int CustomerID,
                              unsigned int UserData
                             )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    unsigned int nextBufReadPtr;
    unsigned int *pp,*qq;
    int i;
    
    //-------//
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
    
    //---config RFU parameter---//
    //-Tx-//
    pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Tx->PktSyncWord          =SyncWord;
    pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    
    pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
    pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
    pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    
    pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;


    //---Set User Data---//
    pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;     
    pRfiuPara_Tx->UserData_H           =(UserData>>16) & RFI_USER_DATA_H_MASK;
      
    //----//
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);  //Tx
#else
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);  //Tx
#endif

    return 1;
}



   #if RFIU_TEST
   void rfiu_Tx_Task_UnitX(void* pData)
   {

   }

   #else
    int rfiuGetACK2ProtocolType(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i,j,count,bitcount;
        int ProtocolType;
        int VMDsel;
        unsigned int temp;   
        //=====//
        
        count=0;  
        pp  = (unsigned int *)(ACKBufAddr);
        while( (count <32) && (*pp != 0xa55aaa55))
        {
            //---Read Group value---//       
            for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
            {
                pp ++;
                count ++;
            }
            //---Read Map value---//
            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
            {
               pp ++;
               count ++;
               //------//
               pp ++;
               count ++;
               
            }
            // new add
            //--Read password--//
       #if RF_CMD_EN        
            for(i=0;i<RFIU_PASSWORD_MAX;i++)
            {
               pp ++;
               count ++;
            }  
            
            //--RX Command--//
            pp ++;
            count ++;

            //--RX Status--//
            temp=(*pp) ^ 0x5a5aa5a5;;
            ProtocolType= (temp>>4) & 0x03;
            VMDsel=(temp>>6) & 0x01;
            pp ++;
            count ++;

            //--Customer ID--//
            pp ++;
            count ++;
       #endif      
            //---RX timer---//
            pp ++;
            count ++;

            //---RX receive packet count---//
            pp ++;
            count ++;
            
        }

        gRfiuUnitCntl[RFUnit].VMDSel=VMDsel;
    
    #if HW_MD_SUPPORT
        MDTxNewVMDSupport=VMDsel;
    #endif    
        return ProtocolType;
    }

    int rfiuGetACK2ChChange(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i,j,count,bitcount;
        int ChChange;
        unsigned int temp;   
        //=====//
        
        count=0;  
        pp  = (unsigned int *)(ACKBufAddr);
        while( (count <32) && (*pp != 0xa55aaa55))
        {
            //---Read Group value---//       
            for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
            {
                pp ++;
                count ++;
            }
            //---Read Map value---//
            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
            {
               pp ++;
               count ++;
               //------//
               pp ++;
               count ++;
               
            }
            // new add
            //--Read password--//
       #if RF_CMD_EN        
            for(i=0;i<RFIU_PASSWORD_MAX;i++)
            {
               pp ++;
               count ++;
            }  
            
            //--RX Command--//
            pp ++;
            count ++;

            //--RX Status--//
            temp=(*pp) ^ 0x5a5aa5a5;;
            ChChange= (temp>>7) & 0x01;
            gRfiuUnitCntl[RFUnit].TXChgCHNext=temp>>24;
            pp ++;
            count ++;

            //--Customer ID--//
            pp ++;
            count ++;
       #endif      
            //---RX timer---//
            pp ++;
            count ++;

            //---RX receive packet count---//
            pp ++;
            count ++;
            
        }
        
        return ChChange;
    }

    int rfiuGetACK2MACAddress(unsigned char RFUnit,unsigned char *ACKBufAddr,int *pRX_RecvPktCnt)
    {
        unsigned int *pp;
        int i,j,count,bitcount;
        unsigned int RX_MACtemp;
        
        //=====//
        count=0;  
        pp  = (unsigned int *)(ACKBufAddr);
        while( (count <32) && (*pp != 0xa55aaa55))
        {
            //---Read Group value---//       
            for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
            {
                pp ++;
                count ++;
            }
            //---Read Map value---//
            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
            {
               pp ++;
               count ++;
               //------//
               pp ++;
               count ++;
               
            }
            // new add
            //--Read password--//
       #if RF_CMD_EN        
            for(i=0;i<RFIU_PASSWORD_MAX;i++)
            {
               pp ++;
               count ++;
            }
            //--Cmd 5~7: 1 Word--//
            pp ++;
            count ++;
            //--Status: 1 Word--//
            pp ++;
            count ++;
            //--Customer ID or Customer Code: 1 word--//
            Temp_TX_CostomerCode[RFUnit]=*pp;
            pp ++;
            count ++;
       #endif          
            //---RX timer---//
            pp ++;
            count ++;

            //---RX receive packet count or MAC address---//
            *pRX_RecvPktCnt= 0;
            *pp ^= 0x5a5aa5a5;
            RX_MACtemp = (*pp);        
            pp ++;
            count ++;
            
        }
        
        return RX_MACtemp;

    }

    int rfiuGetACK2TimeCheck_Pair(unsigned char RFUnit,unsigned char *ACKBufAddr,
                                             int *pRX_RecvTotalPktCnt,int *pRX_RecvDataPktCnt)
    {
        unsigned int *pp;
        int i,j,count,bitcount;
        int RX_TimeCheck;
        unsigned int temp;
        unsigned int Rx_CustomerCode;

        
        //=====//
        count=0;  
        pp  = (unsigned int *)(ACKBufAddr);
        while( (count <32) && (*pp != 0xa55aaa55))
        {
            //---Read Group value---//       
            for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
            {
                pp ++;
                count ++;
            }
            //---Read Map value---//
            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
            {
               pp ++;
               count ++;
               //------//
               pp ++;
               count ++;
               
            }
            // new add
            //--Read password--//
       #if RF_CMD_EN        
            for(i=0;i<RFIU_PASSWORD_MAX;i++)
            {
               pp ++;
               count ++;
            }  
            
            //--RX Command--//
            pp ++;
            count ++;

            //--RX Status--//
            temp=(*pp) ^ 0x5a5aa5a5;;
            rfiuCmdRxStatus[RFUnit].CamNum= temp & 0x07; 
            gRfiuUnitCntl[RFUnit].TXSendDataUse3M= (temp>>3) & 0x01;
            pp ++;
            count ++;

            //--Customer ID--//
            Rx_CustomerCode=*pp ^ 0x5a5aa5a5;
            pp ++;
            count ++;
       #endif      
            //---RX timer---//
            RX_TimeCheck=*pp;
            pp ++;
            count ++;

            //---RX receive packet count---//
            *pRX_RecvTotalPktCnt= 0;
            pp ++;
            count ++;
            
        }

        *pRX_RecvDataPktCnt=0;

        return RX_TimeCheck;
    }





    int rfiuTXCheckAudioRetDataCome(DEF_REGRFIU_CFG *pRfiuPara,int RFUnit)
    {
        int i;
        unsigned int CmdID;
        int isAudioPkt;
        int AudioRetRepFlag;
        //-------------//
        isAudioPkt=0;

#if RFIU_RX_AUDIO_RETURN
        AudioRetRepFlag=(((pRfiuPara->UserData_L) | (pRfiuPara->UserData_H<<16)) >> RFIU_USRDATA_RXSEQNUM_SHFT) & RFIU_USRDATA_RXSEQNUM_MASK;
        for(i=RFI_ACK_SYNC_PKTNUM;i<(RFI_ACK_SYNC_PKTNUM+RFI_AUDIO_RET_GRPNUM);i++)
        {
            CmdID=(pRfiuPara->Pkt_Grp_offset[i] >> 7);
            if( ( (pRfiuPara->PktMap[2*i] != 0) || (pRfiuPara->PktMap[2*i+1] != 0) ) && ((CmdID >= RFI_AUDIORETURN1_ADDR_OFFSET) && (CmdID <= RFI_AUDIORETURN8_ADDR_OFFSET)) )
            {
               if(AudioRetRepFlag)
               {
                  if( gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Group == (CmdID>>6) )
                  {
                     gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map0 |= pRfiuPara->PktMap[2*i];
                     gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map1 |= pRfiuPara->PktMap[2*i+1];
                  }
                  else
                     DEBUG_RFIU_P2("Warning! Not repeat Audio return packet!!\n");
               }
               else
               {
                     gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Group = CmdID>>6;            
                     gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map0  = pRfiuPara->PktMap[2*i];
                     gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map1  = pRfiuPara->PktMap[2*i+1];
               }
               isAudioPkt |=1;    
            }
            else
            {
               gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Group=0;
               gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map0=0;
               gRfiuUnitCntl[RFUnit].AudioRetPktMap[i-RFI_ACK_SYNC_PKTNUM].Map1=0;
            }
        }
#endif
        return isAudioPkt;
    }



    int rfiuTxSentSYNCState(  int RFUnit,
                                    unsigned int Vitbi_en, 
                                    unsigned int RS_mode, 
                                    unsigned int Vitbi_mode,
                                    unsigned int SyncWord,
                                    unsigned int CustomerID,
                                    unsigned int UserData
                                  )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        int i;
        //------//
        
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

        //Lucian: Put sync information to SYNC Packet. Not implement now!
        rfiuPutInfo2SYNC(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_SYNC_ADDR_OFFSET*128);
                       
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

        for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
        {
            pRfiuPara_Tx->PktMap[2*i]      =RFI_SYNC_ADDR_CHEKBIT;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_SYNC_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif

        return 1;
    }
    int rfiuPutInfo2SYNC(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        unsigned char *qq; 
        int i,count;
        int width,height;
        unsigned int  cpu_sr = 0;
        unsigned int TX_Status;
        unsigned int TX_Status2;
    #if (UI_LIGHT_SUPPORT == 1)
        u8  level;
    #endif
        //=====//
        count=0;
        pp  = (unsigned int *)(ACKBufAddr);
        mpeg4GetVideoResolution(&width,&height);

        //--write TX read pointer--//
        OS_ENTER_CRITICAL();
        *pp = gRfiuUnitCntl[RFUnit].BufReadPtr;
        OS_EXIT_CRITICAL();
        pp ++;   
        count ++;

        //--write picture width and height--//
        *pp = width | (height<<16);
        pp ++;   
        count ++;

        //--TX status--//
        TX_Status=0;
#if  (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
        TX_Status |=RFIU_TX_STA__FIELD_ENC;
#endif

#if TX_FW_UPDATE_SUPPORT
        gRfiuUnitCntl[RFUnit].FWUpdate_support=1;
        TX_Status |=RFIU_TX_STAT_FWUPD_SUPPORT;
#endif


        if(gRfiuUnitCntl[RFUnit].RFpara.MD_en)
            TX_Status |=RFIU_TX_STA__MD_ON;

        if(gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
            TX_Status |=RFIU_TX_STA__PIR_ON;

        if(AE_Flicker_50_60_sel==SENSOR_AE_FLICKER_50HZ)        
            TX_Status |=RFIU_TX_STA_FLICKER_50HZ;

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType==TX_SENSORTYPE_HD)
            TX_Status |=RFIU_TX_STA__HD_SUPPORT;
        else if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType==TX_SENSORTYPE_FHD)
            TX_Status |= (RFIU_TX_STA__HD_SUPPORT | RFIU_TX_STA__FULLHD_SUPPORT);
        else if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType==TX_SENSORTYPE_UXGA)
            TX_Status |= (RFIU_TX_STA__HD_SUPPORT);

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn)
            TX_Status |= RFIU_TX_STA_TIMESTAMP_ON;

#if (UI_LIGHT_SUPPORT == 1)
        gpioGetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, &level);
        if (level)
           TX_Status |= RFIU_TX_STAT_BELL_ON;
#else
        if(rfiuDoorBellTrig)
           TX_Status |= RFIU_TX_STAT_BELL_ON;
#endif


        if(gRfiuUnitCntl[RFUnit].ProtocolSel ==RFI_PROTOCOL_ORIG) 
            TX_Status = (TX_Status & (~RFIU_TX_STAT_PROTCOL_SEL)) | (RFI_PROTOCOL_ORIG<<10);
        else if(gRfiuUnitCntl[RFUnit].ProtocolSel ==RFI_PROTOCOL_CE_181)
            TX_Status = (TX_Status & (~RFIU_TX_STAT_PROTCOL_SEL)) | (RFI_PROTOCOL_CE_181<<10);
        else if(gRfiuUnitCntl[RFUnit].ProtocolSel ==RFI_PROTOCOL_FCC_247)
            TX_Status = (TX_Status & (~RFIU_TX_STAT_PROTCOL_SEL)) | (RFI_PROTOCOL_FCC_247<<10);
        else if(gRfiuUnitCntl[RFUnit].ProtocolSel ==RFI_PROTOCOL_ISOWIFI)
            TX_Status = (TX_Status & (~RFIU_TX_STAT_PROTCOL_SEL)) | (RFI_PROTOCOL_ISOWIFI<<10);

            
#if USE_MPEG_QUANTIZATION
        TX_Status |=RFIU_TX_STA_MPEG_Q;
#endif

#if MULTI_STREAM_SUPPORT
        TX_Status |=RFIU_TX_STA_MULTISTREAM_ON;
#endif


        //feedback rfiuRX_OpMode to RX
        TX_Status |= ( (rfiuRX_OpMode & 0x0f)<<12);
        //TX_Status |= ( (rfiuRX_CamOnOff_Num & 0x07)<<16);
 #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        if(rfiuBatCamDcDetect == 0)
          TX_Status |= RFIU_TX_STA_BATCAM_SUPPORT;

        if(rfiuBatCamPIRTrig)
            TX_Status |= RFIU_TX_STA_BATCAM_PIRMODE;
 #endif
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_TX_BRIGHT] & 0x0f)<<20);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] & 0x0f)<<24);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT] & 0x0f)<<28);
           
        *pp=TX_Status;
        pp ++;   
        count ++;
        
        //---Code Version---//
        qq = (unsigned char*)pp;
        memset(qq, 0, 32);
        sprintf ((char*)qq,"%s ", uiVersion);
        #if TX_FW_UPDATE_SUPPORT
        rfiuTXSetVersionInfo(qq);
        #endif
    #if (UI_LIGHT_SUPPORT == 1)
        qq[31] |= UI_VERSION_BIT_LIGHT;
    #endif
    #if (UI_CAMERA_ALARM_SUPPORT == 1)
        qq[31] |= UI_VERSION_BIT_CA;
    #endif
        pp    += (32/4);
        count += (32/4);

        //----TX ID--//
        *pp = gRfiuUnitCntl[RFUnit].RFpara.RF_ID;
        pp ++;
        count ++;

        //-----AV sync time-----//
      #if RF_AV_SYNCTIME_EN
        *pp = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime;
        pp ++;
        count ++;

        *pp = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime;
        pp ++;
        count ++;    

        //DEBUG_RFIU_P2("AV total Time=%d,%d\n",gRfiuUnitCntl[RFUnit].VideoTotalTime,gRfiuUnitCntl[RFUnit].AudioTotalTime);
      #endif
        //-------------------//
        TX_Status2=0;
        TX_Status2 |= ((rfiuBatCamBattLev << 0) & RFIU_TX_STA2_BATTLV);

        if(gRfiuUnitCntl[RFUnit].RFpara.TXPirFaulseTrig)
            TX_Status2 |= RFIU_TX_STA2_FAULSETRIG;

        if( gRfiuUnitCntl[RFUnit].RFpara.TxAlermtOn )
        {
            TX_Status2 |= RFIU_TX_STA2_ALERMON;
        }
      #if (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)
        TX_Status2 |= RFIU_TX_STA2_DOORBELL_SUPPORT;
      #endif
        *pp=TX_Status2;
        pp ++;   
        count ++;
        //-----Packet End----//
        for(i=count;i<32;i++)
        {
           *pp = 0xa55aaa55;//magic number for End.
           pp ++;
           count ++;
        }
    }



    int rfiuTxSentPairPkt  (  int RFUnit,
                                    unsigned int Vitbi_en, 
                                    unsigned int RS_mode, 
                                    unsigned int Vitbi_mode,
                                    unsigned int SyncWord,
                                    unsigned int CustomerID,
                                    unsigned int UserData
                                  )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        int i;
        //------//
        
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

        //Yugo: Put pair information to PAIR Packet. 
        rfiuPutTxInfo2PAIR(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_PAIR_ADDR_OFFSET*128);
                       
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

        for(i=0;i<RFI_ACK_PAIR_PKTNUM;i++)
        {
            pRfiuPara_Tx->PktMap[2*i]      =RFI_PAIR_ADDR_CHEKBIT;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_PAIR_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command
        
        RfiuReset(RFUnit);      

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif

        return 1;
    }

    int rfiuPutTxInfo2PAIR(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i;
        unsigned int  cpu_sr = 0;
        //=====//
        pp  = (unsigned int *)(ACKBufAddr);
        *pp =(unsigned int)Temp_TX_MAC_address[RFUnit];
        pp ++;

        for(i=1;i<32;i++)
        {
           *pp = 0xa55aaa55;//magic number for End.
           pp ++;
        }
    }

    int rfiuPutInfo2DUMMY(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i;
        unsigned int  cpu_sr = 0;
        //=====//
        pp  = (unsigned int *)(ACKBufAddr);
        for(i=0;i<32;i++)
        {
           *pp = 0xa55aaa55;//magic number for End.
           pp ++;
        }
    }

    int rfiuTxSentPairACKPkt  (  int RFUnit,
                                    unsigned int Vitbi_en, 
                                    unsigned int RS_mode, 
                                    unsigned int Vitbi_mode,
                                    unsigned int SyncWord,
                                    unsigned int CustomerID,
                                    unsigned int UserData
                                  )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        int i;
        //------//
        
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

        //Yugo: Put pair information to PAIR Packet. 
        rfiuPutInfo2DUMMY(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_DUMMY_ADDR_OFFSET*128);
                       
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

        for(i=0;i<RFI_ACK_PAIR_PKTNUM;i++)
        {
            pRfiuPara_Tx->PktMap[2*i]      =0x00000001;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_DUMMY_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_PAIR_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command
        
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif    
        return 1;
    }


    void rfiu_RXCMD_Dec (char CmdType,int RFUnit,int *pTX_CHG_CHflag)
    {
        RTC_DATE_TIME   dateTime;
    	u32  reso_W,reso_H;
        u32 Brightness;
        u8 par1,par2,par3,par4;
        u8 par5,par6,par7,par8,par9;
        u8 mdset;
        int i,j,k,count;
        char CmdExtType;
        int SaveData;
        u32 RX_OpMode;
        int writeflag;
        static int ZoomOnOff_prev=0;
        static int ZoomXpos_prev=0;
        static int ZoomYpos_prev=0;
        u8 TempTime[7][6]={0}; 

        
        if(CmdType==RFRXCMD_SET_TIME)
        {
           dateTime.year=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           dateTime.month=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
           dateTime.day=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
           dateTime.hour=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
           dateTime.min=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4];
           dateTime.sec=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5];

           DEBUG_RFIU_P2("Set Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
           RTC_Set_Time(&dateTime);
        }
    	else if(CmdType==RFRXCMD_SET_RESOLUTION)
        {
           reso_W=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]*16;
    	   reso_H=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]*16;

    	   DEBUG_RFIU_P2("Set Reso %d %d\r\n", reso_W,reso_H); 
      
           if( (reso_W==640) && (reso_H==480))
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_640x480)
              {
                 //uiMenuVideoSizeSetting = UI_MENU_VIDEO_SIZE_640x480;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
              }
           #endif
           }
           else if( (reso_W==640) && (reso_H==352))
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_640x352)
              {
                 //uiMenuVideoSizeSetting = UI_MENU_VIDEO_SIZE_640x352;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x352);
              }
           #endif
           }
           else if( (reso_W==704) && (reso_H==480))
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_704x480)
              {
                 //uiMenuVideoSizeSetting = UI_MENU_VIDEO_SIZE_704x480;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
              }
           #endif
           }
           else if( (reso_W==1280) && (reso_H==720) )
           {
           #if RFIU_SUPPORT
              if( (gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_HD) || (gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_FHD) || (gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_UXGA) )
              {
                 if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_1280X720)
                 {
                    //uiMenuVideoSizeSetting =UI_MENU_VIDEO_SIZE_1280X720;
                    sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                 }
              }
              else
              {
                 DEBUG_RFIU_P2("==>Warning!! HD resolution is not support!(%d)\n",gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType); 
              }
           #endif
           }
           else if( (reso_W==1920) && ((reso_H==1072) || (reso_H==1088)) )
           {
           #if RFIU_SUPPORT
              if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_FHD)
              {
                  if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_1920x1072)
                  {
                     //uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1920x1072;
                     sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1920x1072);
                  }
              }
              else
              {
                 DEBUG_RFIU_P2("==>Warning!! FHD resolution is not support!(%d)\n",gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType); 
              }
           #endif
           }
           else if( (reso_W==2688) && (reso_H==1520) )
           {
           #if RFIU_SUPPORT
              if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_4M)
              {
                  if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_2688x1520)
                  {
                     //uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1920x1072;
                     sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_2688x1520);
                  }
              }
              else
              {
                 DEBUG_RFIU_P2("==>Warning!! 4M resolution is not support!(%d)\n",gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType);  
              }
           #endif
           }
           else if( (reso_W==1600) && (reso_H==896) )
           {
           #if RFIU_SUPPORT
              if( (gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_UXGA) || (gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_FHD) )
              {
                  if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_1600x896)
                  {
                     //uiMenuVideoSizeSetting =UI_MENU_VIDEO_SIZE_1600x896;
                     sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1600x896);
                  }
              }
              else
              {
                 DEBUG_RFIU_P2("==>Warning!! FHD resolution is not support!\n"); 
              }
           #endif
           }
           else if( (reso_W==320) && (reso_H==240) )
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_320x240)
              {
                 //uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_320x240;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_320x240);
              }
           #endif
           }
           else if( (reso_W==704) && (reso_H==480) )
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_704x480)
              {
                 //uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
              }
           #endif
           }
           else if( (reso_W==704) && (reso_H==576) )
           {
           #if RFIU_SUPPORT
              if(uiMenuVideoSizeSetting !=UI_MENU_VIDEO_SIZE_704x576)
              {
                 //uiMenuVideoSizeSetting =UI_MENU_VIDEO_SIZE_704x576;
                 sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x576);
              }
           #endif
           }
           else
           {
                DEBUG_RFIU_P2("--->Error! Invalid resolution. W=%d,H=%d\n",reso_W,reso_H);
           }
    	}
        else if(CmdType==RFRXCMD_SET_OPMODE)
        {
        #if(SW_APPLICATION_OPTION == MR8110_RFCAM_TX1)  
               RX_OpMode=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
               rfiuRX_P2pVideoQuality=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //Lucian: Only valid in P2p mode
               rfiuRX_CamOnOff_Num=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
               rfiuRX_CamPerRF=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];

               if(RX_OpMode != rfiuRX_OpMode)
               {
                   rfiuRX_OpMode=RX_OpMode;
                   if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
                   {
                       sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x352);
                   }
                   else 
                   {
                       sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                   }
               }
        #else
               RX_OpMode=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
               rfiuRX_P2pVideoQuality=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //Lucian: Only valid in P2p mode
               rfiuRX_CamOnOff_Num=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
               rfiuRX_CamPerRF=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];

            //-------------------------大小碼流------------------------//
            #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM))
               mpeg4MultiStreamStart=1;
            #elif(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
               mpeg4MultiStreamStart=1;
            #elif(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
               mpeg4MultiStreamStart=1;
            #else
              #if (HW_BOARD_OPTION == A1019A_SKB_128M_TX)
               mpeg4MultiStreamStart=1;
              #else
               if( RX_OpMode & (RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_P2P) )
                  mpeg4MultiStreamStart=1;
               else
                  mpeg4MultiStreamStart=0;
              #endif
            #endif

               if(RX_OpMode != rfiuRX_OpMode)
               {
                   rfiuRX_OpMode=RX_OpMode;
                                      
                //----------------------Frame rate control---------------------------//   
                #if (Sensor_OPTION == Sensor_NT99340_YUV601 )
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )
                   {
                      #if ( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_BT_USB))
                      siu_SetNT99340_720P_FrameRate(30);
                      #else
                      siu_SetNT99340_720P_FrameRate(27);
                      #endif
                   }
                   else
                   {
                      siu_SetNT99340_720P_FrameRate(27);
                   }
                #elif (Sensor_OPTION == Sensor_PO3100K_YUV601 )  
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )
                   {
                      #if ( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_BT_USB))
                      siu_SetPO3100K_720P_FrameRate(30);
                      #else
                      siu_SetPO3100K_720P_FrameRate(27);
                      #endif
                   }
                   else
                   {
                      siu_SetPO3100K_720P_FrameRate(27);
                   }
                #elif(Sensor_OPTION == Sensor_NT99230_YUV601 )
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_PIP) && (rfiuRX_OpMode & RFIU_RX_OPMODE_MAINCH) )
                   {
                       siu_SetNT99230_1080p_FrameRate(8);
                   }
                   else if((rfiuRX_OpMode & RFIU_RX_OPMODE_P2P))
                   {
                       siu_SetNT99230_1080p_FrameRate(10);
                   }
                   else
                   {
                       siu_SetNT99230_1080p_FrameRate(12);
                   }
                #endif
                }
        #endif
        
        #if (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
               if( rfiuRX_OpMode & RFIU_RX_OPMODE_PCM_SIGN)
               {
                    if(rfiu_TX8BitPCMFmt != PCM_8BITFMT_SIGN)
                    {
                        adc8BitPCMFmtSel(PCM_8BITFMT_SIGN);
                        iconflag[UI_MENU_SETIDX_TX_8BITPCMFMT] = PCM_8BITFMT_SIGN;
                        sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING, 0);
                    }
               }
               else
               {
                    if(rfiu_TX8BitPCMFmt != PCM_8BITFMT_UNSIGN)
                    {
                        adc8BitPCMFmtSel(PCM_8BITFMT_UNSIGN);
                        iconflag[UI_MENU_SETIDX_TX_8BITPCMFMT] = PCM_8BITFMT_UNSIGN;
                        sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING, 0);
                    }
               }
         #endif
        
    	   DEBUG_RFIU_P2("Set RxOpMode=0x%x,Level=%d,RxCamNum=%d,CamPerRF=%d\r\n", rfiuRX_OpMode,rfiuRX_P2pVideoQuality,rfiuRX_CamOnOff_Num,rfiuRX_CamPerRF);      
           
    	}
        else if(CmdType==RFRXCMD_SET_BRIGHTNESS)
        {
           Brightness=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           mdset=MotionDetect_en;
           if(mdset)
           {
           #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(0);
    	   #endif
           }
           uiMenuSet_TX_BRIGHTNESS(Brightness);
           if(mdset)
           {
           #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(1);
    	   #endif
           }

    	   DEBUG_RFIU_P2("Set Brightness %d\r\n", Brightness);      
           
    	}
        else if(CmdType==RFRXCMD_SET_FLICKER)
        {
           AE_Flicker_50_60_sel=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           mdset=MotionDetect_en;
           if(mdset)
           {
           #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(0);
    	   #endif
           }
           #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
               (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)|| (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
               (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
               (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
               (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
           uiMenuSet_TX_FLICER(AE_Flicker_50_60_sel);
           #endif
           siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
           if(mdset)
           {
           #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(1);
    	   #endif
           }

    	   DEBUG_RFIU_P2("Set Flicker %d\r\n", AE_Flicker_50_60_sel);      
    	}
        else if(CmdType==RFRXCMD_SET_ZOOM)
        {
           par1= gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           par2= gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
           par3= gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];

           if( (ZoomOnOff_prev == par1) && (ZoomXpos_prev == par2) && (ZoomYpos_prev == par3) )
           {

           }
           else
           {
               mdset=MotionDetect_en;
               if(mdset)
               {
               #if HW_MD_SUPPORT
                   mduMotionDetect_ONOFF(0);
        	   #endif
               }
           #if(MULTI_CHANNEL_SEL & 0x02)    
               rfiuciu1ZoomInx2(par1,par2<<3,par3<<3);
           #elif(MULTI_CHANNEL_SEL & 0x04)
               rfiuciu2ZoomInx2(par1,par2<<3,par3<<3);
           #endif
               if(mdset)
               {
               #if HW_MD_SUPPORT
                   mduMotionDetect_ONOFF(1);
        	   #endif
               }
           }

           ZoomOnOff_prev=par1;
           ZoomXpos_prev=par2;
           ZoomYpos_prev=par3;      
    	   DEBUG_RFIU_P2("Set Zoom Config: (%d,%d,%d)\r\n",par1,par2,par3);      
    	}
        else if(CmdType==RFRXCMD_SET_MDCFG)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           if (par1 != 0) //check data is not all zero
           {
               gRfiuUnitCntl[RFUnit].RFpara.MD_en         =par1 & 0x03;
               gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day  =(par1>>2) & 0x07;
               gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night=(par1>>5) & 0x07;
               gRfiuUnitCntl[RFUnit].RFpara.MD_Trig       =0;

            #if 1 //Night mode sensitivity//
               MD_SensitivityConfTab_Night[0][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];  
               MD_SensitivityConfTab_Night[0][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]; 
               MD_SensitivityConfTab_Night[1][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]; 
               MD_SensitivityConfTab_Night[1][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]; 
               MD_SensitivityConfTab_Night[2][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]; 
               MD_SensitivityConfTab_Night[2][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]; 
            #endif

               uiMenuSet_TX_MOTION(gRfiuUnitCntl[RFUnit].RFpara.MD_en,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night);
           }
    	   DEBUG_RFIU_P2("Set MD Config: 0x%x\r\n", par1);
    	}
        else if(CmdType==RFRXCMD_SET_PIRCFG)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];

           SaveData = 1;
         #if(PassiveIR_SensControl == PassiveIR_PYD1588)
           // update PIR sensitivity parameter from RX
           for (i = 0; i < 6; i++)
           {
               if (gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+1] == 0){
                   SaveData = 0; 
                   break;
               }
           }
           if (SaveData == 1)
           {
               for (i = 0; i < 6; i++)
               {
                   if (iconflag[UI_MENU_SETIDX_INDOOR_PIR_SEN_HIGH+i] != gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+1])
                   {
                        // PIR sensitivity changed, saved it
                        iconflag[UI_MENU_SETIDX_INDOOR_PIR_SEN_HIGH]  = gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                        iconflag[UI_MENU_SETIDX_INDOOR_PIR_SEN_MID]   = gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                        iconflag[UI_MENU_SETIDX_INDOOR_PIR_SEN_LOW]   = gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
                        iconflag[UI_MENU_SETIDX_OUTDOOR_PIR_SEN_HIGH] = gRfiuUnitCntl[RFUnit].RX_CMD_Data[4];
                        iconflag[UI_MENU_SETIDX_OUTDOOR_PIR_SEN_MID]  = gRfiuUnitCntl[RFUnit].RX_CMD_Data[5];
                        iconflag[UI_MENU_SETIDX_OUTDOOR_PIR_SEN_LOW]  = gRfiuUnitCntl[RFUnit].RX_CMD_Data[6];
                        Save_UI_Setting();
                        PIR_Init(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
                        break;
                   }
               }
           }
         #endif
           if (SaveData == 1)
           {
               if (gRfiuUnitCntl[RFUnit].RFpara.PIR_en != (par1 & 0x01))
               {
                   //do SYS_EVT_TXPIRTRIGONOFF when PIR enable status changed
                   gRfiuUnitCntl[RFUnit].RFpara.PIR_en=par1 & 0x01;
                   gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;

                 #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                   sysSetEvt(SYS_EVT_TXPIRTRIGONOFF,gRfiuUnitCntl[RFUnit].RFpara.PIR_en);
                 #endif
               }
           }
           DEBUG_RFIU_P2("Check:%d, Set PIR Config: 0x%x, Indoor:(%d,%d,%d), Outdoor:(%d,%d,%d)\r\n", SaveData, par1, gRfiuUnitCntl[RFUnit].RX_CMD_Data[1], gRfiuUnitCntl[RFUnit].RX_CMD_Data[2], gRfiuUnitCntl[RFUnit].RX_CMD_Data[3], gRfiuUnitCntl[RFUnit].RX_CMD_Data[4], gRfiuUnitCntl[RFUnit].RX_CMD_Data[5], gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]);
        }
        else if(CmdType==RFRXCMD_SET_TXOSD)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];

    	   DEBUG_RFIU_P2("Set TX OSD: %d\r\n", par1);      
    	}
        else if(CmdType==RFRXCMD_SET_PTZ485)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
           par3=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
           par4=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
           par5=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4];
           par6=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5];
           par7=gRfiuUnitCntl[RFUnit].RX_CMD_Data[6];
           
        #if 0
           DEBUG_RFIU_P2("Set PTZ:%d\r\n", par1);
        #else
           DEBUG_RFIU_P2("Set PTZ %c%c%c%c%c%c%c", (char)par1,(char)par2,(char)par3,(char)par4,(char)par5,(char)par6,(char)par7);  
           sendchar(PTS485_UART_ID, &par1);
           sendchar(PTS485_UART_ID, &par2);
           sendchar(PTS485_UART_ID, &par3);
           sendchar(PTS485_UART_ID, &par4);
           sendchar(PTS485_UART_ID, &par5);
           sendchar(PTS485_UART_ID, &par6);
           sendchar(PTS485_UART_ID, &par7);
        #endif
    	}
        else if(CmdType==RFRXCMD_SET_GPO)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETGPO, (s32)par1);

    	   DEBUG_RFIU_P2("Set TX GPO: %d\r\n", par1);      
    	}
        else if(CmdType==RFRXCMD_SET_OTHERSPARAS)
        {
        #if (RFIU_SUPPORT)
           SaveData=0;
           //---Time stamp---//
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0] & 0xf;
           if(gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn != par1)
           {
              gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=par1;
              iconflag[UI_MENU_SETIDX_CH1_TSP_ON+RFUnit]=par1;
              SaveData=1;
              uiMenuSet_TX_TimeStampOn(par1,0);
           }

           par2=(gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]>>4) & 0xf;
           if(gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampType != par2)
           {
              gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampType=par2;
              iconflag[UI_MENU_SETIDX_CH1_TSP_TYPE+RFUnit]=par2;
              SaveData=1;
           }
           
           //--Substream config--//
           par3=(gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]) & 0xf;
           if(gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamBRSel != par3)
           {
              gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamBRSel=par3;
              iconflag[UI_MENU_SETIDX_CH1_STREAM_QUALITY+RFUnit]=par3;
              SaveData=1;
           }

    	   DEBUG_RFIU_P2("--Set TX-%d others paras:%d,%d,%d--\n",RFUnit,par1,par2,par3); 
           if(SaveData)
           {
              sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING, 0);
              DEBUG_RFIU_P2("--Save UI Setting--\n"); 
           }
        #endif
    	}
        else if(CmdType==RFRXCMD_SET_MASKAREA_VGA)
        {
    	}
        else if(CmdType==RFRXCMD_SET_MASKAREA_HD)
        {
        #if HW_MD_SUPPORT  
           #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
                (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
                (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) 
               count=20*2*2; //Lucian: 第1,2條block,全部mask.
               for(i=0;i<count;i++)
               {
                  MD_blk_Mask_HD[RFUnit+5][i]=1;
               }   
           #else
               count=20; //Lucian: 第一條block,全部mask.
               for(i=0;i<count;i++)
               {
    		      #if CIU2_REPLACE_CIU1
                  MD_blk_Mask_HD[RFUnit+2][i]=1;
    			  #else
                  MD_blk_Mask_HD[RFUnit+1][i]=1;
                  #endif   
               }
           #endif
        #endif   
        
        #if HW_MD_SUPPORT
           for(j=0;j<9;j++)
           {
              par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[j];
              for(i=0;i<8;i++)
              {
              #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
                   (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)|| (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
                   (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)) 
                 for(k=0;k<4;k++)
                 {
                       MD_blk_Mask_HD[RFUnit+5][count]=par1 & 0x01;
                       MD_blk_Mask_HD[RFUnit+5][count+40]=par1 & 0x01;
                       MD_blk_Mask_HD[RFUnit+5][count+80]=par1 & 0x01;
                       MD_blk_Mask_HD[RFUnit+5][count+120]=par1 & 0x01;
                       count ++;
                 }
                 if( (count % 40)==0 )
                    count +=40*3;
              #else
                 for(k=0;k<2;k++)
                 {
					   #if CIU2_REPLACE_CIU1
                          MD_blk_Mask_HD[RFUnit+2][count]=par1 & 0x01;
                          MD_blk_Mask_HD[RFUnit+2][count+20]=par1 & 0x01;
					   #else
                          MD_blk_Mask_HD[RFUnit+1][count]=par1 & 0x01;
                          MD_blk_Mask_HD[RFUnit+1][count+20]=par1 & 0x01;
                       #endif
                       count ++;
                 }
                 if( (count % 20)==0 )
                    count +=20;
              #endif
                 par1 >>=1;
              }
           }       
        #endif

           
        #if MD_DEBUG_ENA   
    	   DEBUG_RFIU_P2("\nSet TX MaskArea HD:%x %x %x %x %x %x %x %x %x\n",
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[7],
                        gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]
             );    
          #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
               (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
               (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           for ( i = 0; i < MD_BLOCK_NUM_MAX; i++)
           {
                if (i%40 == 0)
                {
                    DEBUG_RFIU_P2("\n");
                    DEBUG_RFIU_P2("Line %02d : ", i/40);
                }
                DEBUG_RFIU_P2("%d ",MD_blk_Mask_HD[RFUnit+5][i]);
           }
          #else
           for ( i = 0; i < MD_BLOCK_NUM_MAX; i++)
           {
                if (i%20 == 0)
                {
                    DEBUG_RFIU_P2("\n");
                    DEBUG_RFIU_P2("Line %02d : ", i/20);
                }
		     
		        #if CIU2_REPLACE_CIU1
                DEBUG_RFIU_P2("%d ",MD_blk_Mask_HD[RFUnit+2][i]);
				#else
                DEBUG_RFIU_P2("%d ",MD_blk_Mask_HD[RFUnit+1][i]);
				#endif
           }
           #endif
        #endif
    	}
        else if(CmdType==RFRXCMD_SET_SLEEP)
        {
        #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
           DEBUG_RFIU_P2("Set TX Sleep\n"); 
           sysSaveAeExpoVal2UISetting();
           sysSaveLastBitRate();
           #if TX_PIR_INTERVAL_OFF
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
           #else
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
           #endif
        #else
           DEBUG_RFIU_P2("Sleep mode is not Support!\n"); 
        #endif
        }
        else if(CmdType==RFRXCMD_SET_EXTEND)
        {
             CmdExtType=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];

             if(CmdExtType==RFRXCMD_SETEXT_PWRTURBO)
             {
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("Set RF Turbo: %d\r\n",par2);  
                  if(par2)
                  {
                     gRfiuUnitCntl[RFUnit].RFpara.TX_TurboOn=1;
                  }
                  else
                  {
                     gRfiuUnitCntl[RFUnit].RFpara.TX_TurboOn=0;
                  }
                     
             }
             else if(CmdExtType==RFRXCMD_SETEXT_DOORBELLOFF)
             {
                  rfiuDoorBellTrig = 0;
             #if  ENABLE_DOOR_BELL   
                  iisStopDoorBell();
             #endif     
                  DEBUG_RFIU_P2("Set Door-bell off\r\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_MDSENSTAB)
             {
                  MD_SensitivityConfTab[0][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  MD_SensitivityConfTab[0][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  MD_SensitivityConfTab[1][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
                  MD_SensitivityConfTab[1][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4];
                  MD_SensitivityConfTab[2][0]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5];
                  MD_SensitivityConfTab[2][1]=gRfiuUnitCntl[RFUnit].RX_CMD_Data[6];
             #if HW_MD_SUPPORT
                  if (SIUMODE == SIU_DAY_MODE)
                    mduMotionDetect_Sensitivity_Config(gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
             #endif
                  DEBUG_RFIU_P2("Set MD Sens:H(%d,%d),M(%d,%d),L(%d,%d):%d\n",
                                MD_SensitivityConfTab[0][0],
                                MD_SensitivityConfTab[0][1],
                                MD_SensitivityConfTab[1][0],
                                MD_SensitivityConfTab[1][1],
                                MD_SensitivityConfTab[2][0],
                                MD_SensitivityConfTab[2][1],
                                gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day
                               );  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_CHGCH)
             {
                   *pTX_CHG_CHflag=1;
                   gRfiuUnitCntl[RFUnit].TXChgCHNext=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   //DEBUG_RFIU_P2("Change CH\n");
             }
             else if(CmdExtType==RFRXCMD_SETEXT_PWM)
             {
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETPWM, par2);
                  //DEBUG_RFIU_P2("Set PWM\r\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_MOTORCTL)
             {
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETMOTORCTRL, par2);
                  //DEBUG_RFIU_P2("Set MOTOR control\r\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_MELODYNUM)
             {
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETMELODYNUM, par2);
                  //DEBUG_RFIU_P2("Set Melody Num\r\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_VOXCFG)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  rfiuVoxEna[RFUnit]=par1;
                  rfiuVoxThresh[RFUnit]=par2;
                           
                  DEBUG_RFIU_P2("Set Vox Config:ONOFF=%d,THR=%d\n",par1,par2);  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_SUBSTR_EN)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  mpeg4MultiStreamStart=par1;
                           
                  DEBUG_RFIU_P2("Set SubStream ONOFF=%d\n",par1);  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_SAT_CON_SHP)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  par3=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
                  par4=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]; //for sensor flip_mirror
                  mdset=MotionDetect_en;
                  if(mdset)
                  {
                  #if HW_MD_SUPPORT
                       mduMotionDetect_ONOFF(0);
            	  #endif
                  }
                  writeflag=uiMenuSet_TX_SAT_CON_SHP(par1,par2,par3,par4);
                  if(writeflag)
                     sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING, 0);
                  
                  if(mdset)
                  {
                  #if HW_MD_SUPPORT
                       mduMotionDetect_ONOFF(1);
            	  #endif
                  }
                           
                  DEBUG_RFIU_P2("Set SAT CON SHP %d %d %d \n",par1,par2,par3);  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_VOLUME)
             {
                   par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   //control volume//
                   //sysProjectSetAudioVolume(par1);
            	   DEBUG_RFIU_P2("Set volume %d\r\n", par1);      
           
    	     }
             else if(CmdExtType==RFRXCMD_SETEXT_LIGHTONOFF)
             {
                   par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   //control Light On or OFF //
                   //sysProjectSetLightOnOff(par1);
            	   DEBUG_RFIU_P2("Set Light %d\r\n", par1);      
           
    	     }
             else if(CmdExtType==RFRXCMD_SETEXT_SCHED)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //For 30mins week 0~6
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]; //part1
                  par3=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]; //part2
                  par4=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]; //part3
                  par5=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]; //part4
                  par6=gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]; //part5
                  par7=gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]; //part6
                  par8=gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]; //bit(0) = LightTimer on/off, bit(1) = Maunall Setting

                  DEBUG_UI("[Light] rfiu_RXCMD Get Data: %d %d %d %d %d %d %d %d in RF\n", par1, par2, par3, par4, par5, par6, par7, par8);
                  
                  //--Compare UI setting--//
                  SaveData = 0;
                #if((HW_BOARD_OPTION  == MR9100_TX_MAYON_MWL612) || (HW_BOARD_OPTION  == MR9160_TX_MAYON_MWL613)) 
                  #if (UI_LIGHT_SUPPORT == 1)                    
                  if (~par8 & (1<<5)) //light
                      memcpy(&TempTime,&uiLightInterval,sizeof(uiLightInterval));
                  #endif  
  
                  #if UI_CAMERA_ALARM_SUPPORT
                  if (par8 & (1<<5)) //alarm
                      memcpy(&TempTime,&uiAlarmInterval,sizeof(uiAlarmInterval));
                  #endif
                  
                  for (i = 0; i < 6; i++)
                  {
                      if (gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+2] != TempTime[par1][i])
                      {
                          TempTime[par1][i] = gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+2];
                          SaveData = 1;
                      }
                  }

                  if (par8 & 0x10) //check whether manual light mode
                  {
                      if (par8 & (1<<5))
                          uiManualAlarm_DuringUserSetting = 1;
                      else
                          uiManualLight_DuringUserSetting = 1;
                      //DEBUG_RED("[Light] rfiu_RXCMD par8 & 0x10 true , uiInManualLight = %d \n", uiInManualLight);
                  }
                  
                  //----------------------//
                  if(SaveData)
                  {
                     if (par8 & (1<<5)) //alarm
                         memcpy(&uiAlarmInterval,&TempTime,sizeof(TempTime));
                     else
                         memcpy(&uiLightInterval,&TempTime,sizeof(TempTime));
                    sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
                  }

                #else  //other
                  #if (UI_LIGHT_SUPPORT == 1)

                      #if(UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)

                      for (i = 0; i < 4; i++)
                      {
                          if (gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+1] != uiLightTimer[i])
                          {
                              uiLightTimer[i] = gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+1];
                              SaveData = 1;
                          }
                      }
                      if (par5 != iconflag[UI_MENU_SETIDX_LIGHT_TIMER])
                      {
                          iconflag[UI_MENU_SETIDX_LIGHT_TIMER] = par5;
                          SaveData = 1;
                      }
                      if (par6 == 1)
                          SaveData = 1;
                      
                      #elif(UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
                      for (i = 0; i < 6; i++)
                      {
                          if (gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+2] != uiLightInterval[par1][i])
                          {
                              uiLightInterval[par1][i] = gRfiuUnitCntl[RFUnit].RX_CMD_Data[i+2];
                              SaveData = 1;
                          }
                      }

                      if (par8 & 0x10) //check whether manual light mode
                      {
                          DEBUG_UI("[Light] rfiu_RXCMD par8 & 0x10 true , uiInManualLight = %d \n", uiInManualLight);
                          uiManualLight_DuringUserSetting = 1;
                      }
                      #endif
                  #endif
                  //----------------------//
                  if(SaveData)
                  {
                     #if (UI_LIGHT_SUPPORT == 1)
                     uiMenuAction(UI_MENU_SETIDX_LIGHT_TIMER);
                     #endif
                     sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
                  }
                #endif
                           
                  DEBUG_RFIU_P2("-TXSCHED-\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_SNAPSHOT)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("--TX Snap shot:%d--\n",par1);
                  sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SNAPSHOT,par1);
             }
             else if(CmdExtType==RFRXCMD_SETEXT_REBOOT)
             {
                  DEBUG_RFIU_P2("--TX Reboot--\n");
                  sysForceWDTtoReboot();
             }
             else if(CmdExtType==RFRXCMD_SETEXT_FBAPPSTA)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  DEBUG_RFIU_P2("APPSTA:%d,%d\r\n",par1,par2); 
                  //add code by Ted
                     
             }           
             else if(CmdExtType == RFRXCMD_SETEXT_SLEEPRECNT)
             {
                //DEBUG_GREEN("Get RFCMD Type RFRXCMD_SET_SLEEPRECNT \n");
                rfiuTxLifeTotolTime = 0;
             }        
             else
             {
                   DEBUG_RFIU_P2("Unknow Ext Command:%d\r\n",CmdExtType); 
             }
                
        
        }
        
        else
        {
           DEBUG_RFIU_P2("Unknow Command\n"); 
        }
        

    }
    void rfiu_Tx_Task_UnitX(void* pData)
    {
    	u8 err;
        int TX_UsrData,TX_UsrACK,TX_UsrData_next;
        int i,temp;
#if DEBUG_WRPTR_WSHFT    
        int RxWrtPtr,TxRedPtr,TxSft,RxSft;
#endif
        int ACK_CH_sel=0;
        int DAT_CH_sel,prev_DAT_CH_sel;
        int TxAckRetryCnt=0;
        int RFUnit;
        unsigned int TX_timer,TX_TimeCheck;
        unsigned int RX_TimeCheck,OffsetDiff,TRX_TimeOffset;
        int DatPktRecvFlag;
        int AckPacketTime,RX_RecvPktCnt;
        int RX_recvDataPktCnt;   //目前作用不明.
        int TX_TransTotalPkt=0;  //計算 TX 所傳送的有效packet count;

        u8 RSSI,RSSI_Wifi;
        int CE181_CH_Mask[RFI_DAT_CH_MAX];
        u8 NextDAT_CH;
        int MinCh,MinVal;
        u32 Tmp;
#if (UI_LIGHT_SUPPORT)
        u8 dayLevel;
#endif

        DEF_RFIU_DAT_CH_STATISTICS TX_CH_Stat[RFI_DAT_CH_MAX+1]; //Lucian: used in TX-end.
        DEF_RFIU_FEC_TYPE_STATISTICS TX_FEC_Stat[RFI_FEC_TYPE_MAX];
        
        unsigned int t1,t2,dt,BitRate,BitRate2; //cal bitrate
        unsigned int prev_BitRate2=0;
        unsigned int t3,t4;            //cal CE181 RF CH occurppied time
        unsigned int t5,t6;            //cal Link-broken time
        unsigned int t7,t8;            //cal TX Sync time.
        unsigned int t9,t10;           //cal TX Life time
        unsigned int BR_t1,BR_t2;      // cal birate2 per sec with 2 sec windown

        u32 TxSyncCount=0;
        
        int GrpWROffset,FillIdx;
        unsigned int WtPtr,RdPtr,Prev_WtPtr;
        unsigned int TXWrapErrMonitor;

#if RF_PAIR_EN 
        unsigned int MACtemp;  //yugo add
        int TX_SYC_ErrorCnt;
        unsigned int TX_PAIR_ACKflag;
        unsigned int Old_TXMAC,Old_TXCODE;
#endif
        
        unsigned int  cpu_sr = 0; 
        unsigned int RFTimer,TimeCheck;
        int TxBufFullness;

    	unsigned int t_seed;
        int TXCmd_Valid;
        u8 level;

        int TX_CHG_CHflag;
        int TX_CHG_CHCnt;

     #if FORCE_LINKBROKEN_LOWBR
        int EnterLowBitRateCnt=0;
     #endif

#if TX_FW_UPDATE_SUPPORT
        int RecvFwUpdStart;
        int RecvFwUpdDone;
        u32 TxFWSum,TxFWSum2;
        u32 ACKType;
#endif

#if TX_PIRREC_VMDCHK
    unsigned int VMDChk_T5=0;
    unsigned int VMDChk_T6=0;
#endif    

        //---------------------------//
        TXWrapErrMonitor=0;
        Prev_WtPtr=0xffffffff;
        prev_DAT_CH_sel=0;
        DAT_CH_sel = 0;
        TXCmd_Valid=0;
        TX_CHG_CHflag=0;
        TX_CHG_CHCnt=0;
        RFUnit= (int)pData;
     
        memset(&gRfiuParm_Tx[RFUnit],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuParm_Rx[RFUnit],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));

        gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;
        gRfiuUnitCntl[RFUnit].TX_Pair_Done=0;
        gRfiuUnitCntl[RFUnit].BufReadPtr=0;
        gRfiuUnitCntl[RFUnit].BufWritePtr=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel=1;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.DAT_CH=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.SeqenceNum=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpShift=0;
        gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpDivs=0;
        gRfiuUnitCntl[RFUnit].RFpara.MD_en=MotionDetect_en;
        gRfiuUnitCntl[RFUnit].RunCount=0;
#if(RF_CMD_EN)
        gRfiuUnitCntl[RFUnit].TXCmd_Type=RFTXCMD_SEND_NONE; 
        gRfiuUnitCntl[RFUnit].TXCmd_en=0;    
#endif

        for(i=0;i<RFI_DAT_CH_MAX;i++)
        {
           CE181_CH_Mask[i]=0;
        }

        for(i=0;i<RFI_DAT_CH_MAX;i++)
          gRfiuUnitCntl[RFUnit].GoodDataCHTab[i]=i;

        for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
        {
           memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
        }

        for(i=0;i<RFI_FEC_TYPE_MAX;i++)
        {
           memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
        }
        
        TX_UsrData= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
        TX_UsrACK=TX_UsrData;
        TX_UsrData_next=TX_UsrData;

        memcpy(&gRfiuUnitCntl[RFUnit].TX_CtrlPara_next,&gRfiuUnitCntl[RFUnit].TX_CtrlPara,sizeof(DEF_RFIU_USRDATA));    

#if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(RFUnit+1,0);
#elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(RFUnit+1,100);
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[0]);
#elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel(RFUnit+1,100);
#endif
            
        //---Setup test environment---//
     #if(RF_PAIR_EN)
        TX_PAIR_ACKflag=0;
        TX_SYC_ErrorCnt=0;
     #endif
     
        gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;
        rfiuTxSyncTotolTime=0;
        rfiuTxLifeTotolTime=0;

        #if RFI_SELF_TEST_TXRX_PROTOCOL
        if(gRfiuUnitCntl[RFUnit].OpMode == RFIU_SYNC_MODE)
            OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0, RFIU_TASK_PRIORITY_HIGH);
        #endif

        gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
        gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr = rfiuOperBuf[RFUnit];
        gRfiuParm_Rx[RFUnit].TxRxOpBaseAddr = rfiuOperBuf[RFUnit];

     #if(RFIU_TEST)
        #if(RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL)

        #else
        gRfiuUnitCntl[RFUnit].BufWritePtr=64;
        for(i=0;i<gRfiuUnitCntl[RFUnit].BufWritePtr;i++)
        {
           gRfiuUnitCntl[RFUnit].TxPktMap[i].PktCount=64;
           gRfiuUnitCntl[RFUnit].TxPktMap[i].RetryCount=0;
           gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap0 =0xffffffff;
           gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap1 =0xffffffff;
        }
        #endif
     #endif
     

        OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
        gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;

        //gRfiuUnitCntl[RFUnit].TX_Task_Stop=0;
        timerCountRead(guiRFTimerID, &t1);
        timerCountRead(guiRFTimerID, &t3);
        timerCountRead(guiRFTimerID, &t5);
        BR_t1=t9=t7=t5;
#if TX_PIRREC_VMDCHK
        VMDChk_T5=guiSysTimerCnt;
#endif
        DEBUG_RFIU_P2("=Tx_Task_%d=\n",RFUnit);
    	while(1)
    	{
            timerCountRead(guiRFTimerID, &t10);
            if(t9 >= t10)
              dt=t9-t10;
            else
              dt=(t9+TimerGetTimerCounter(TIMER_7))-t10;
            rfiuTxLifeTotolTime += dt;
            t9=t10;
        #if TX_PIRREC_VMDCHK
            VMDChk_T6=guiSysTimerCnt;
        #endif
           
        #if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            if(rfiuBatCamDcDetect == 0)
            {
                if(rfiuTxLifeTotolTime > RF_BATCAM_TXRUN_MAXTIME*10000 )
                {
                   DEBUG_RFIU_P2("==TX Time OverRun,Go Sleep:%d==\n",rfiuTxLifeTotolTime);
                   sysSaveAeExpoVal2UISetting();
                   sysSaveLastBitRate();
                #if TX_PIR_INTERVAL_OFF
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
                #else
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
                #endif
                   rfiuTxLifeTotolTime=0;
                   timerCountRead(guiRFTimerID, &t9);
                }
            }
        #endif
        
    	    gRfiuUnitCntl[RFUnit].RunCount ++;
    	    if(gRfiuUnitCntl[RFUnit].TX_Task_Stop)
    	    {
    	        DEBUG_RFIU_P2("$");
                OSTimeDly(1);
                t8=t7;
                BR_t2=BR_t1;
                continue;
            }
            //------//
        #if TX_PIRREC_VMDCHK
            if( (rfiuBatCamDcDetect==0) && (rfiuBatCamPIRTrig==1) && MotionDetect_en )
            {
                if(rfiuPIRRec_VMDTrig==0)
                {
                   DEBUG_RFIU_P2("^");
                   if(VMDChk_T6 >= VMDChk_T5)
                      temp=VMDChk_T6-VMDChk_T5;
                   else
                      temp=VMDChk_T6 + (0xffffffff-VMDChk_T5);
                   temp=temp*25; // 25ms unit 
                   if(temp > 2000)
                   {
                   #if PIR_FALSETRIG_TEST
                     gRfiuUnitCntl[RFUnit].RFpara.TXPirFaulseTrig=1;
                     rfiuPIRRec_VMDTrig=1;
                     DEBUG_RFIU_P2("\n==PIR False Trig! ==\n");
                   #else
                     gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
                     gRfiuUnitCntl[RFUnit].TX_Task_Stop=1;
                     gRfiuUnitCntl[RFUnit].TX_MpegEnc_Stop=1;
                     sysSaveAeExpoVal2UISetting();
                     DEBUG_RFIU_P2("\n==PIR False Trig! Go sleep==\n");
                     sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
                   #endif
                   }
                   //OSTimeDly(1);  //Lucian: 不知道為什這種做法不行,改用customer ID 反相.
                   //continue;
                }
            }
        #endif 

            
        #if 1
            if(AmicReg_RWen1)
            {
               if(AmicReg_RWen1 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }

               AmicReg_RWen1=0;
            }    
            
            if(AmicReg_RWen2)
            {
               if(AmicReg_RWen2 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif
            
            
    	    switch(gRfiuUnitCntl[RFUnit].OpMode)
    	    {
    		case RFIU_TX_MODE:
        		    if( (gRfiuUnitCntl[RFUnit].OpState == RFIU_TX_STATE_READY) || (gRfiuUnitCntl[RFUnit].OpState == RFIU_TX_STATE_INIT) )
        		    {
                        if((TX_UsrData_next & RFIU_USRDATA_SEQNUM_CHEK ) == (TX_UsrACK & RFIU_USRDATA_SEQNUM_CHEK))
                        {
                            DatPktRecvFlag=1;
                        #if DEBUG_WRPTR_WSHFT
                            RxWrtPtr=(TX_UsrACK >> 11) & 0x1f;
                            TxRedPtr=gRfiuUnitCntl[RFUnit].BufReadPtr & 0x1f;

                            RxSft=(TX_UsrACK >> 16) & 0xf;
                            TxSft=(((TX_UsrData_next) >> 14)& 0xf);
                            
                            
                            if(RxWrtPtr != TxRedPtr)
                                DEBUG_RFIU_P2("===>Tx/Rx WrPtr mismatch: %d,%d.\n",TxRedPtr,RxWrtPtr);

                            if(TxSft != RxSft)
                                DEBUG_RFIU_P2("===>Tx/Rx Shfit mismatch: %d,%d.\n",TxSft,RxSft);
                        #endif
                        }
                        else
                        {
                            DatPktRecvFlag=0;

                        }
                        
                        rfiuDecUsrData(TX_UsrACK,&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
                        TX_UsrData=TX_UsrACK;                

                        //==Select next DATA channel and FEC code==//
                        if(gRfiuUnitCntl[RFUnit].TXCmd_en)
                            TXCmd_Valid=1;
                        else
                            TXCmd_Valid=0;
                            
                        RX_TimeCheck=rfiuTxUpdatePktMap(RFUnit,
                                                        DatPktRecvFlag,
                                                        &gRfiuUnitCntl[RFUnit].TX_CtrlPara,
                                                        &gRfiuUnitCntl[RFUnit].TX_CtrlPara_next,
                                                        &RX_RecvPktCnt,
                                                        &RX_recvDataPktCnt,
                                                        prev_DAT_CH_sel,
                                                        TX_CH_Stat,
                                                        TX_FEC_Stat
                                                       );

                        TX_TransTotalPkt +=RX_RecvPktCnt;

                        //--Cal TX's bitrate per sec with 2sec window--//
                        timerCountRead(guiRFTimerID, &BR_t2);
                        if(BR_t1 >= BR_t2)
                          dt=BR_t1-BR_t2;
                        else
                          dt=(BR_t1+TimerGetTimerCounter(TIMER_7))-BR_t2;
                        if(dt==0)
                            dt=1;
                        if(dt > 10000)
                        {
                            BitRate2=TX_TransTotalPkt*128*8*10/dt;
                            if(prev_BitRate2 != 0)
                                BitRate2= (prev_BitRate2 + BitRate2)/2;
                                
                            BR_t1=BR_t2;
                            TX_TransTotalPkt=0;
                        #if TX_USENEW_BRCAL
                            if(prev_BitRate2 !=0)
                            {
                               rfiuTXBitRate[0]=BitRate2;
                               //DEBUG_RFIU_P2("BR-%d:%d\n",RFUnit,BitRate2);
                            }  
                        #endif
                            prev_BitRate2=BitRate2;
                        }
                        //-------//


                        if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
                        {
                            //NextDAT_CH=gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH;
                            NextDAT_CH=ACK_CH_sel;
                            if(CE181_CH_Mask[NextDAT_CH] != 0 )
                            {
                               //NextDAT_CH= (NextDAT_CH-2) % RFI_DAT_CH_MAX;  //Lucian: Mod 不可用在負數
                               NextDAT_CH= (NextDAT_CH-2) & (RFI_DAT_CH_MAX-1);
                               
                               MinVal=CE181_CH_Mask[NextDAT_CH];
                               MinCh=NextDAT_CH;
                               
                               for(i=0;i < 5 ;i++)
                               {
                                   if(CE181_CH_Mask[NextDAT_CH]==0)
                                      break;
                                   NextDAT_CH=(NextDAT_CH+1) % RFI_DAT_CH_MAX;
                                   
                                   if(MinVal > CE181_CH_Mask[NextDAT_CH])
                                   {
                                      MinVal=CE181_CH_Mask[NextDAT_CH];
                                      MinCh=NextDAT_CH;
                                   }
                               }
                               //DEBUG_RFIU_P2("%d ",i);
                               if(CE181_CH_Mask[NextDAT_CH]!=0)
                               {
                                  NextDAT_CH=MinCh;
                               }
                               
                            }
                            
                            //if(NextDAT_CH >15)  DEBUG_RFIU_P2("\nNextDAT_CH=%d\n",NextDAT_CH);
                            
                            gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH=NextDAT_CH;
                            //DEBUG_RFIU_P2("%d ",NextDAT_CH);
                        }
                        //---TX Config---//
                        TX_UsrData_next= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].TX_CtrlPara_next);
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                        MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif(RFIC_SEL==RFIC_NONE_5M)
                        RFNONE_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #endif
                     
                        rfiuCheckTrigger(RFUnit, &TX_UsrData_next, TXCmd_Valid);
                        rfiuTxSendDataState( RFUnit,
                                             gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en], 
                                             gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel], 
                                             gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel],
                                             gRfiuSyncWordTable[RFUnit],
                                             gRfiuCustomerCode[RFUnit],
                                             TX_UsrData_next
                                           );
                     #if(RFIU_TEST )//Lucian: 做循環測試
                        #if(RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL)

                        #else
                        GrpWROffset = rfiuCalBufRemainCount(gRfiuUnitCntl[RFUnit].BufWritePtr,gRfiuUnitCntl[RFUnit].BufReadPtr,RFI_BUF_SIZE_GRPUNIT);
                        if( (GrpWROffset >= 0) && (GrpWROffset < 32) )
                        {
                            for(i=0;i<4;i++)
                            {
                               gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufWritePtr & RFI_BUF_SIZE_MASK].PktCount=64;
                               gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufWritePtr & RFI_BUF_SIZE_MASK].RetryCount=0;
                               gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufWritePtr & RFI_BUF_SIZE_MASK].PktMap0 =0xffffffff;
                               gRfiuUnitCntl[RFUnit].TxPktMap[gRfiuUnitCntl[RFUnit].BufWritePtr & RFI_BUF_SIZE_MASK].PktMap1 =0xffffffff;

                               gRfiuUnitCntl[RFUnit].BufWritePtr=(gRfiuUnitCntl[RFUnit].BufWritePtr + 1) & RFI_BUF_SIZE_MASK;
                            }
                        }
                        #endif
                     #endif                 

                 #if RF_CMD_EN 
                        if((gRfiuUnitCntl[RFUnit].TXCmd_en == 1) && (TXCmd_Valid==1)) 
                          gRfiuUnitCntl[RFUnit].TXCmd_en=0; //test
    			 #endif
                        if(DatPktRecvFlag && (RX_TimeCheck != 0xffffffff) )
                        {
                        #if 0
                            TRX_TimeOffset = ((RX_TimeCheck & 0x0ffff)+ 0x10000) - (TX_TimeCheck & 0xffff);

                            if( (gRfiuTimer_offset[RFUnit] & 0x0ffff) > (TRX_TimeOffset & 0x0ffff) )
                                OffsetDiff= (gRfiuTimer_offset[RFUnit] & 0x0ffff)  - (TRX_TimeOffset & 0x0ffff);
                            else
                                OffsetDiff=(TRX_TimeOffset & 0x0ffff)-(gRfiuTimer_offset[RFUnit] & 0x0ffff);

                            if(OffsetDiff > 32768)
                                OffsetDiff= 0x0ffff- OffsetDiff;
                        #else
                            if(RX_TimeCheck >= TX_TimeCheck)
                               TRX_TimeOffset = RX_TimeCheck- TX_TimeCheck;
                            else
                               TRX_TimeOffset = RX_TimeCheck + TIMER7_COUNT+1 - TX_TimeCheck;  

                            if( gRfiuTimer_offset[RFUnit] >= TRX_TimeOffset )
                                OffsetDiff= gRfiuTimer_offset[RFUnit] - TRX_TimeOffset;
                            else
                                OffsetDiff= TRX_TimeOffset - gRfiuTimer_offset[RFUnit];
                        #endif

                         #if 0
                            gRfiuTimer_offset[RFUnit]= TRX_TimeOffset;
                            DEBUG_RFIU_P2("\nTx Timer:(%d,%d),(%d,%d)\n",TRX_TimeOffset,OffsetDiff,RX_TimeCheck,TX_TimeCheck);
                         #else
                            if( (OffsetDiff > 2) && (OffsetDiff < TIMER7_COUNT+1-10) )
                            {
                               gRfiuTimer_offset[RFUnit]= TRX_TimeOffset;
                            #if RFI_SELF_TEST_TXRX_PROTOCOL   
                                  DEBUG_RFIU_P2("\nTx Timer update: (%d,%d),(%d,%d)\n",TRX_TimeOffset,OffsetDiff,RX_TimeCheck,TX_TimeCheck);
                            #else
                               if(OffsetDiff>10)
                               {
                                  DEBUG_RFIU_P2("\nTx Timer update: (%d,%d),(%d,%d)\n",TRX_TimeOffset,OffsetDiff,RX_TimeCheck,TX_TimeCheck);
                               }
                            #endif
                            }
                         #endif
                        }
                        //----Receive RX command----//
                      #if RF_CMD_EN
    				    if(DatPktRecvFlag)
    				    {
    	                    if(TX_UsrACK & RFIU_USRDATA_CMD_CHEK) 
    	                    {
    	                        gRfiuUnitCntl[RFUnit].RXCmd_Type=(TX_UsrACK>>RFIU_USRDATA_RXCMDTYP_SHFT) & RFIU_USRDATA_RXCMDTYP_MASK;
    	                  		//DEBUG_RFIU_P2("\n RX CMD_type=0x%X \n",gRfiuUnitCntl[RFUnit].RXCmd_Type );
    	                        rfiu_RXCMD_Dec(gRfiuUnitCntl[RFUnit].RXCmd_Type,RFUnit,&TX_CHG_CHflag);
    	                    } 
    				    }
    				  #endif
                        if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                        {
                           if(TX_CHG_CHflag==0)
                           {
                              TX_CHG_CHflag=rfiuGetACK2ChChange(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128);
                              if(TX_CHG_CHflag)
                              {
                                 if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                                    TimeCheck= (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1));  // 409.6 ms to change ACK channel.
                                 else
                					TimeCheck= (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]);
                                 //DEBUG_RFIU_P2("-->TRIG:%d,%d\n",gRfiuUnitCntl[RFUnit].TXChgCHNext, (TimeCheck>>10) & 0x0f );
                              }
                           }
                        }
                        //---Calculate Tx Buffer fullness--/
                        WtPtr=gRfiuUnitCntl[RFUnit].BufWritePtr;
                        RdPtr=gRfiuUnitCntl[RFUnit].BufReadPtr;
                        GrpWROffset = rfiuCalBufRemainCount(WtPtr,RdPtr,RFI_BUF_SIZE_GRPUNIT);
                        if( (GrpWROffset>=0) && (DatPktRecvFlag==1) )
                        {
                             TxBufFullness= GrpWROffset*8 + gRfiuUnitCntl[RFUnit].TxPktMap[WtPtr & RFI_BUF_SIZE_MASK].WriteDiv -gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpDivs;
                             rfiuTxBufFullness[0]=TxBufFullness;
                        }     
                         
                        //---Calculate bitrate---//
                        if(TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum == 0)
                        {
                           timerCountRead(guiRFTimerID, &t1);
                           BitRate=0;
                        }
                        else if( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum & (RFIU_CAL_BITRATE_INTV-1) ) == 0)
                        {
                           timerCountRead(guiRFTimerID, &t2);
                           if(t1 >= t2)
                              dt=t1-t2;
                           else
                              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

                           if(dt==0)
                              dt=1;
                           
                           BitRate = TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum*128*8*10/dt;
                           sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, BitRate/100);
                           gRfiuUnitCntl[RFUnit].BitRate=BitRate/100; //Unit: 100 Kbps
                        #if FORCE_LINKBROKEN_LOWBR
                           if(BitRate<200)
                              EnterLowBitRateCnt ++;
                           else
                              EnterLowBitRateCnt=0;
                        #endif
                           if(TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum>10)
                           {
                           #if TX_USENEW_BRCAL

                           #else
                             rfiuTXBitRate[0]=BitRate;
                           #endif
                             gRfiuUnitCntl[RFUnit].TX_PktCrtRate=TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum*100/TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum;
                             DEBUG_RFIU_P2("\n====>BR=%d/%d kbps,%d/%d,BufFull=%d,%d KB,dt=%d,DLLV=%d,3M->%d\n",BitRate,BitRate2,TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum,
                                           TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum,TxBufFullness,gRfiuUnitCntl[RFUnit].TxBufFullness,dt/10,(SdramDFI_PHYB >> 16),
                                           gRfiuUnitCntl[RFUnit].TXSendDataUse3M );

                             if( (TxBufFullness==0) && (Prev_WtPtr==WtPtr) && (BitRate>200) )
                             {
                                 TXWrapErrMonitor ++;
                             }
                             else
                             {
                                 TXWrapErrMonitor=0;
                             }

                             if(TXWrapErrMonitor>4)
                             {
                                 DEBUG_RFIU_P2("\n======== RFWrap-%d is Fatal Error! Reboot!======\n",RFUnit);
								 sysForceWDTtoReboot();
                             }
                         #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                             if(rfiuBatCamDcDetect==0)
                                rfiu_SetTXInfo(rfiuBatCamBattLev,0xffffffff,0xffffffff,0xffffffff);
                             else
                                rfiu_SetTXInfo(RF_BATCAM_TXBATSTAT_CHARGE,0xffffffff,0xffffffff,0xffffffff);
                         #elif (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)  
                             rfiu_SetTXInfo(RF_BATCAM_TXBATSTAT_CHARGE,
                                            ( (sys9211TXWifiStat & 0x0ff)  | ((rfiu_TX_WifiCHNum & 0x0ff)<<8) ),
                                            0xffffffff,
                                            0xffffffff);
                         #endif
                             Prev_WtPtr=WtPtr;
                           }

                           t1=t2;
                           TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum=0;
                           TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum=0;
                           TX_CH_Stat[RFI_DAT_CH_MAX].BrokenNum=0;
                        }
                        //------Statistic correct rate of TX DATA channel------//                  
                        TX_CH_Stat[DAT_CH_sel].SentPktNum += gRfiuParm_Tx[RFUnit].TxRxPktNum;
                        TX_CH_Stat[DAT_CH_sel].BurstNum +=1;
                        TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum += gRfiuParm_Tx[RFUnit].TxRxPktNum;
                        TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum +=1;

                        prev_DAT_CH_sel= DAT_CH_sel;
                        //---------//
                     #if RFI_SELF_TEST_TXRX_PROTOCOL
                        OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0, RFIU_TASK_PRIORITY_HIGH);
                        #if RFI_TEST_WRAP_OnPROTOCOL
                            temp= gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en*100 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel*10 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel;
                            DEBUG_RFIU_P("===>DCH=%d,BR=%d kbps,FEC=%d,SFT=%d,RP=%d,pR/cS=%d/%d\n",DAT_CH_sel,BitRate,temp,gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift,
                                         gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK,RX_RecvPktCnt,gRfiuParm_Tx[RFUnit].TxRxPktNum);
                            #if DEBUG_WRPTR_WSHFT
                            if(DatPktRecvFlag)
                            {
                               DEBUG_RFIU_P("===>Tx_WrtPtr=%d,Rx_WrtPtr=%d,TxSFT=%d,RxSFT=%d\n",TxRedPtr,RxWrtPtr,TxSft,RxSft);
                            }
                            #endif
                        #else
                            temp= gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en*100 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel*10 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel;
                            DEBUG_RFIU_P2("===>DCH=%d,BR=%d kbps,FEC=%d,SFT=%d,RP=%d,pR/cS=%d/%d\n",DAT_CH_sel,BitRate,temp,gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift,
                                         gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK,RX_RecvPktCnt,gRfiuParm_Tx[RFUnit].TxRxPktNum);
                        #endif
                     #else
                        #if 0
                        temp= gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en*100 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel*10 + gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel;
                        DEBUG_RFIU_P2("===>BR=%d kbps,FEC=%d,SFT=%d,TxBuf=%d,pR/cS=%d/%d\n",BitRate,temp,gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift,
                                     TxBufFullness,RX_RecvPktCnt,gRfiuParm_Tx[RFUnit].TxRxPktNum);
                        #endif

                        #if DEBUG_WRPTR_WSHFT
                        if(DatPktRecvFlag)
                           DEBUG_RFIU_P("===>Tx_WrtPtr=%d,Rx_WrtPtr=%d,TxSFT=%d,RxSFT=%d\n",TxRedPtr,RxWrtPtr,TxSft,RxSft);
                        #endif
                     #endif

                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
                     
                     #if DEBUG_TX_TIMEOUT
                     #endif    
                        
                        TX_TimeCheck=gRfiuTimer[RFUnit];
        		    }
                    
                        
                    //-----Transfer to Wait ACK state-----//
                    //Chanel selection 
                    if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                    {
                        if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                            RFTimer= (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1));  // 409.6 ms to change ACK channel.
                        else
        					RFTimer= (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]);
                 #if 0
                        if(TX_CHG_CHflag)
                        {
                            if(TX_CHG_CHCnt != ((RFTimer>>13) & 0x3) )
                            {
                               ACK_CH_sel = gRfiuUnitCntl[RFUnit].TXChgCHNext;
                               DEBUG_RFIU_P2("Chg CH:%d\n",ACK_CH_sel);
                               TX_CHG_CHflag=0;
                            }
                        }
                        else
                        {
                            TX_CHG_CHCnt= (RFTimer>>13) & 0x3;
                        }
    			 #else
                        if(TX_CHG_CHflag)
                        {
                            if(TX_CHG_CHCnt != ((RFTimer>>14) & 0x3) )
                            {
                               ACK_CH_sel = gRfiuUnitCntl[RFUnit].TXChgCHNext;
                               DEBUG_RFIU_P2("Chg CH:%d\n",ACK_CH_sel);
                               TX_CHG_CHflag=0;
                            }
                        }
                        else
                        {
                            TX_CHG_CHCnt= (RFTimer>>14) & 0x3;
                        }
                 #endif
                  
                    }
                    else
                    {
        			    if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                            ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1))>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                        else
        					ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit])>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;
                    }
                  
                #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                #elif(RFIC_SEL==RFIC_NONE_5M)
                    RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                #endif

                
                    rfiuTxWaitACKState( RFUnit,
                                            RFI_VITBI_DISA, 
                                            RFI_RS_T12, 
                                            RFI_VITBI_CR4_5,
                                            gRfiuSyncWordTable[RFUnit],
                                            gRfiuCustomerCode[RFUnit],
                                            RFI_TX_WAIT_TIME
                                          );    
                 #if RFI_SELF_TEST_TXRX_PROTOCOL
                    DEBUG_RFIU_P("====>Wait ACK. AckCH=%d\n",ACK_CH_sel);
                    if( (gRfiuUnitCntl[RFUnit].OpState == RFIU_TX_STATE_READY) || (gRfiuUnitCntl[RFUnit].OpState == RFIU_TX_STATE_INIT) )
                        OSTaskChangePrio(RFIU_TASK_PRIORITY_HIGH,RFIU_TASK_PRIORITY_UNIT0);
                 #endif

                    RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                    //if(RSSI > 200)
                        //RSSI=0;

                    if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
                    {
                        //DEBUG_RFIU_P2("(%d,%d)",RSSI,ACK_CH_sel);
                        if(RSSI > RFIU_RSSI_THR)
                        {
                             CE181_CH_Mask[ACK_CH_sel]=40000; // 5 sec
                        }

                        timerCountRead(guiRFTimerID, &t6);
                        if(t5 >= t6)
                          dt=t5-t6;
                        else
                          dt=(t5+TimerGetTimerCounter(TIMER_7))-t6;

                        t5=t6;
                        for(i=0;i<RFI_DAT_CH_MAX;i++)
                        {
                            CE181_CH_Mask[i] -=  dt;
                            if(CE181_CH_Mask[i]<0)
                                CE181_CH_Mask[i]=0;
                        }
                    }
                 
                 #if DEBUG_RX_TIMEOUT
                 #endif   

                    //----//
                    //if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
                    if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 0 )
                    {
                       gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_WAITACK;          
                       DEBUG_RFIU_P("======>No ACK  received. AckCH=%d\n",ACK_CH_sel);
                       TX_UsrACK =TX_UsrACK;
                       TxAckRetryCnt ++;
                    }
                #if TX_FW_UPDATE_SUPPORT
                    else if( RXACK_FWUPD_START == rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp ) )
                    {
                       TxAckRetryCnt=0;
                       OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                       gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;
                       if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                       {                   
                       #if RFIU_RX_AUDIO_RETURN    
                          if(guiIISCh0PlayDMAId != 0xFF)
                          {
                              marsDMAClose(guiIISCh0PlayDMAId);
                              guiIISCh0PlayDMAId = 0xFF;
                          }
                       #endif    

                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
                          OSTimeDly(3);
                          switch(RFUnit)
                          {
                             case 0:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                break;

                             case 1:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                break;

                             case 2:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                break; 

                             case 3:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                break;     
                          }  
                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=0;
                          gRfiu_WrapEnc_Sta[RFUnit]=RFI_WRAPENC_TASK_NONE;
                       }

                        memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));
                        
                        gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                        gRfiuUnitCntl[RFUnit].BufWritePtr=0;
    					gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;

    					for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
    				    {
    				       memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
    				    }

                        for(i=0;i<RFI_FEC_TYPE_MAX;i++)
                        {
                           memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
                        }
                                                
                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_INIT;
                       gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_WAITACK;          
                       DEBUG_RFIU_P2("\n==Recv RXACK_FWUPD_START==\n");
                    }
                #endif    
                    else
                    {
                       if(DatPktRecvFlag)
                           timerCountRead(guiRFTimerID, &t3);
                       gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_READY;
                       DEBUG_RFIU_P("======>ACK Packet recv :%d/%d,AckCH=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum,RFI_ACK_SYNC_PKTNUM,ACK_CH_sel);
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       TxAckRetryCnt=0;
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                    }
                 #if RFIU_RX_AUDIO_RETURN
                    if(rfiuTXCheckAudioRetDataCome(&gRfiuParm_Rx[RFUnit],RFUnit) == 1)
                    {
                        temp=((gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16));
                        rfiuAudioRetWrite_idx= (temp & (RFIU_USRDATA_RXGRPDIV_CHEK | RFIU_USRDATA_RXGRPWPR_CHEK) )>>RFIU_USRDATA_RXGRPDIV_SHFT;
                        gRfiuUnitCntl[RFUnit].TXCmd_en=1;
                        gRfiuUnitCntl[RFUnit].TXCmd_Type |=RFTXCMD_SEND_AUDIORETMAP;
                    }
                    else
                    {


                    }
                 #endif
                 

                    timerCountRead(guiRFTimerID, &t4);
                    if(t3 >= t4)
                      dt=t3-t4;
                    else
                      dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;
                   
                    
                    //if(TxAckRetryCnt>=RFI_TX_TRY_ACKCNT_MAX)
                 #if FORCE_LINKBROKEN_LOWBR
                    if( (dt>TX_LINKBROKEN_TIMEOUT) || (EnterLowBitRateCnt>3) )
                 #else
                    if(dt > TX_LINKBROKEN_TIMEOUT)
                 #endif
                    {
                       DEBUG_RFIU_P2("dt=%d\n",dt);
                       TxAckRetryCnt=0;
                       OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                       gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;
                    #if FORCE_LINKBROKEN_LOWBR
                       if(EnterLowBitRateCnt>3)
                       {
                          DEBUG_RFIU_P2("---->Low bitrate,Force to Link broken\n");
                          OSTimeDly(20*2);
                       }
                       EnterLowBitRateCnt=0;
                    #endif
                    #if( (RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL) || RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 )
                       if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                       {                   
                       #if RFIU_RX_AUDIO_RETURN    
                          if(guiIISCh0PlayDMAId != 0xFF)
                          {
                              marsDMAClose(guiIISCh0PlayDMAId);
                              guiIISCh0PlayDMAId = 0xFF;
                          }
                       #endif    

                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
                          OSTimeDly(3);
                          switch(RFUnit)
                          {
                             case 0:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                break;

                             case 1:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                break;

                             case 2:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                break; 

                             case 3:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                break;     
                          }  
                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=0;
                          gRfiu_WrapEnc_Sta[RFUnit]=RFI_WRAPENC_TASK_NONE;
                       }

                        memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));
                        
                        gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                        gRfiuUnitCntl[RFUnit].BufWritePtr=0;
    					gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;

    					for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
    				    {
    				       memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
    				    }

                        for(i=0;i<RFI_FEC_TYPE_MAX;i++)
                        {
                           memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
                        }
                        
                        #if RFI_SELF_TEST_TXRX_PROTOCOL
                        if(gRfiuUnitCntl[RFUnit].OpMode == RFIU_SYNC_MODE)
                            OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0, RFIU_TASK_PRIORITY_HIGH);
                        #endif
                        
                        gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;
                        gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                        rfiuTxSyncTotolTime=0;
                        timerCountRead(guiRFTimerID, &t7);

                      #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                        if(rfiuBatCamPIRTrig)
                        {
                           sysSaveAeExpoVal2UISetting();
                           sysSaveLastBitRate();
                        #if TX_PIR_INTERVAL_OFF
                           sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
                        #else
                           sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
                        #endif
                           OSTimeDly(40);
                        }   
                      #endif
                        
                        DEBUG_RFIU_P2("------>Link Error! Go to Sync\n");
                    #else
                       #if RFI_SELF_TEST_TXRX_PROTOCOL
                        if(gRfiuUnitCntl[RFUnit].OpMode == RFIU_SYNC_MODE)
                            OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0, RFIU_TASK_PRIORITY_HIGH);
                       #endif
                        
                        gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;
                        gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                        rfiuTxSyncTotolTime=0;
                        timerCountRead(guiRFTimerID, &t7);
                        DEBUG_RFIU_P2("------>Link Error! Go to Sync\n");
                    #endif
                    #if FORCE_LINKBROKEN_LOWBR
                       if(EnterLowBitRateCnt>3)
                       {
                          DEBUG_RFIU_P2("---->Low bitrate,Force to Link broken\n");
                          OSTimeDly(20*2);
                       }
                       EnterLowBitRateCnt=0;
                    #endif
                    }
                    break;

                case RFIU_SYNC_MODE:
                    timerCountRead(guiRFTimerID, &BR_t2);
                    prev_BitRate2=0;
                    TX_TransTotalPkt=0;
                    BR_t1=BR_t2;
                    
                    timerCountRead(guiRFTimerID, &t8);
                    if(t7 >= t8)
                      dt=t7-t8;
                    else
                      dt=(t7+TimerGetTimerCounter(TIMER_7))-t8;
                    rfiuTxSyncTotolTime += dt;
                    t7=t8;

                    TxSyncCount ++;
                    
                #if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                    if(rfiuBatCamDcDetect == 0)
                    {
                        if(rfiuBatCamPIRTrig)
                        {
                            if(rfiuTxSyncTotolTime > RF_BATCAM_TXSYNC_PIR_MAXTIME*10000 )
                            {
                               DEBUG_RFIU_P2("==Sync Time OverRun,Go Sleep:%d==\n",rfiuTxSyncTotolTime);
                               sysSaveAeExpoVal2UISetting();
                               sysSaveLastBitRate();
                            #if TX_PIR_INTERVAL_OFF
                               sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
                            #else
                               sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
                            #endif
                               rfiuTxSyncTotolTime=0;
                               timerCountRead(guiRFTimerID, &t7);
                            }                        
                        }
                        else
                        {
                            if(rfiuTxSyncTotolTime > RF_BATCAM_TXSYNC_WAK_MAXTIME*10000 )
                            {
                               DEBUG_RFIU_P2("==Sync Time OverRun,Go Sleep:%d==\n",rfiuTxSyncTotolTime);
                               sysSaveAeExpoVal2UISetting();
                               sysSaveLastBitRate();
                            #if TX_PIR_INTERVAL_OFF
                               sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
                            #else
                               sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
                            #endif
                               rfiuTxSyncTotolTime=0;
                               timerCountRead(guiRFTimerID, &t7);
                            }
                        }
                    }
                #endif
                
                #if((HW_BOARD_OPTION == A1019A_TX_MA8806) || (HW_BOARD_OPTION == MR9211_TX_MA8806) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) )
                        if(rfiuTxSyncTotolTime > 60*10000 )
                        {
                           DEBUG_RFIU_P2("==Sync Time OverRun,Go Reset RF module:%d==\n",rfiuTxSyncTotolTime);
                           //sysForceWDTtoReboot();
                         #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                            InitMV400();  
                         #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                            InitA7130();
                         #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                            InitA7196();
                         #elif(RFIC_SEL==RFIC_NONE_5M)
                            InitRFNONE();
                         #endif
                           
                           rfiuTxSyncTotolTime=0;
                           timerCountRead(guiRFTimerID, &t7);
                        }
                #endif
                    
                    if(TX_PAIR_ACKflag >0 )
                    {
                        rfiuTxSyncTotolTime=0; //配對時,讓 SyncTime Over run保護失效
                        if(gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247)
                        {

                        }
                        else
                        {
                            if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                                ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1))>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;  // 409.6 ms to change ACK channel.
                            else
            					ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit])>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;
                        }
                    }
                    else
                    {
                    #if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                        //if(rfiuBatCamDcDetect==0)
                        {
                            if(TxSyncCount & 0x01)
                               ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW;
                            else
                               ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH;
                        }    
                    #endif
                    }
                #if FORCE_LINKBROKEN_LOWBR
                    EnterLowBitRateCnt=0;
                #endif
                    //==Chanel selection==//     
                    TXWrapErrMonitor=0;
                #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                #elif(RFIC_SEL==RFIC_NONE_5M)
                    RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                #endif
                    //==Wait ACK==//
                    rfiuTxWaitACKState( RFUnit,
                                        RFI_VITBI_DISA, 
                                        RFI_RS_T12, 
                                        RFI_VITBI_CR4_5,
                                        gRfiuSyncWordTable[RFUnit],
                                        gRfiuCustomerCode[RFUnit],
                                        RFI_TX_WAIT_TIME
                                      );    
                #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                    DEBUG_RFIU_P2("S%x",ACK_CH_sel);
                #elif(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
                
                #else
                    //DEBUG_RFIU_P2("S");
                    DEBUG_RFIU_P2("S%x",ACK_CH_sel);
                #endif

                    rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                 
                 #if DEBUG_RX_TIMEOUT
                 #endif     

                    //==Check ACK==//
                    //if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
                    if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 0 )
                    {
                       DEBUG_RFIU_P("======>No ACK Packet received. AckCH=%d\n",ACK_CH_sel);
                       TX_UsrACK =TX_UsrACK;

                       if( (gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247) && (TX_PAIR_ACKflag >0 ) ) 
                       {

                       }
                       else
                          ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;

                    #if(RF_PAIR_EN)   //yugo
                       if(TX_PAIR_ACKflag >0 )   //enter pair ack 
                       {  
                           TX_SYC_ErrorCnt++;
                           if(TX_SYC_ErrorCnt >=10)
                           {
                               gRfiuSyncWordTable[RFUnit] = RFI_PAIR_SYNCWORD;
                               gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                               TX_PAIR_ACKflag=0;
                               TX_SYC_ErrorCnt =0;
                               gRfiuUnitCntl[RFUnit].OpMode=RFIU_PAIR_MODE;
                           #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                               A7130_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD);
                           #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                               A7196_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD);
                           #elif(RFIC_SEL==RFIC_NONE_5M)
                               RFNONE_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD);
                           #endif
                               DEBUG_RFIU_P2("-->TX Pair Retry\n");
                           }  
                       }
                    #endif  
                       
                    }
                #if TX_FW_UPDATE_SUPPORT
                    else if( RXACK_NORMAL != rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp ) )
                    {
                       TX_UsrACK =TX_UsrACK;
                       ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                    }
                #endif                      
                    else
                    {   
                     #if(RF_PAIR_EN)   //yugo
                         TX_SYC_ErrorCnt=0;  // 避免RX先行跳回PAIR
                     #endif 
                     
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                       MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                       A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                       A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif(RFIC_SEL==RFIC_NONE_5M)
                       RFNONE_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #endif

                       rfiuDecUsrData(TX_UsrACK,&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.SeqenceNum ++;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpShift=0;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpDivs=0;

                       gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift=0;
                       
                       TX_UsrData_next= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
                       gRfiuUnitCntl[RFUnit].ProtocolSel=rfiuGetACK2ProtocolType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128);
                       //DEBUG_RFIU_P2("--->ProtocolSel=%d\n",gRfiuUnitCntl[RFUnit].ProtocolSel);
                       rfiuTxSentSYNCState(  RFUnit,
                                             gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en], 
                                             gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel], 
                                             gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel],
                                             gRfiuSyncWordTable[RFUnit],
                                             gRfiuCustomerCode[RFUnit],
                                             TX_UsrData_next
                                          );
                    #if RFI_SELF_TEST_TXRX_PROTOCOL
                       OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0,RFIU_TASK_PRIORITY_HIGH);
                    #endif

                    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                       DEBUG_RFIU_P2("%x",DAT_CH_sel);
                    #else
                       DEBUG_RFIU_P2("%x",DAT_CH_sel);
                    #endif
                    
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);
                    
                        TX_TimeCheck=gRfiuTimer[RFUnit];
                    #if 1
                        if(TX_PAIR_ACKflag >0 )
                        {
                            if(gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247)
                            {

                            }
                            else
                            {
                                if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                                    ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1))>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;  // 409.6 ms to change ACK channel.
                                else
                					ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit])>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;
                            }
                        }
                    #endif    
                        //==Wait ACK==//
                    #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                        MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif(RFIC_SEL==RFIC_NONE_5M)
                        RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #endif
                        rfiuTxWaitACKState( RFUnit,
                                            RFI_VITBI_DISA, 
                                            RFI_RS_T12, 
                                            RFI_VITBI_CR4_5,
                                            gRfiuSyncWordTable[RFUnit],
                                            gRfiuCustomerCode[RFUnit],
                                            RFI_TX_WAIT_TIME
                                          );    

                        rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                     
                     #if DEBUG_RX_TIMEOUT
                     #endif    

                        //if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
                        if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 0 )
                        {
                           DEBUG_RFIU_P("======>No ACK Packet received. AckCH=%d\n",ACK_CH_sel);
                           TX_UsrACK =TX_UsrACK;
                           ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                           //DEBUG_RFIU_P2("\n");
                        }
                        else
                        { 
                        #if TX_FW_UPDATE_SUPPORT
                            rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                        #endif                      
                        
                           TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                           if(gRfiuUnitCntl[RFUnit].OpState != RFIU_TX_STATE_INIT)
                              gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_READY;
                           //DEBUG_RFIU_P2("2");

                           if((TX_UsrData_next & RFIU_USRDATA_SEQNUM_CHEK ) == (TX_UsrACK & RFIU_USRDATA_SEQNUM_CHEK))
                           {
                               TX_UsrData_next=TX_UsrACK;  //Lucian: accept new data channel.
                               RX_TimeCheck=rfiuGetACK2TimeCheck(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,
                                                                 &RX_RecvPktCnt,&RX_recvDataPktCnt);
                               //DEBUG_RFIU_P2("3");
                               if(RX_TimeCheck != 0xffffffff)
                               {
                               #if 0
                                   gRfiuTimer_offset[RFUnit] = ((RX_TimeCheck& 0x0ffff)+ 0x10000) - (TX_TimeCheck & 0xffff);
                               #else
                                   if(RX_TimeCheck >= TX_TimeCheck)
                                       gRfiuTimer_offset[RFUnit] = RX_TimeCheck- TX_TimeCheck;
                                   else
                                       gRfiuTimer_offset[RFUnit] = RX_TimeCheck + TIMER7_COUNT+1 - TX_TimeCheck;  
                               #endif

                                   DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                                   TxAckRetryCnt=0;
    							                                                   
                               #if( (RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL) || RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 )    
                                   if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_NONE)
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
                                   }
                               #else
                                   gRfiuUnitCntl[RFUnit].BufWritePtr=64;
                                   gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                                   for(i=0;i<gRfiuUnitCntl[RFUnit].BufWritePtr;i++)
                                   {
                                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktCount=64;
                                       gRfiuUnitCntl[RFUnit].TxPktMap[i].RetryCount=0;
                                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap0 =0xffffffff;
                                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap1 =0xffffffff;
                                   }
                               #endif

    						   #if(RF_PAIR_EN)   // 確定進入 SYC mode
    						       if(TX_PAIR_ACKflag>0)
    						       {
    						         gRfiuUnitCntl[RFUnit].TX_Pair_Done=1;
    						         uiRFID[RFUnit]=(unsigned int)gRfiuSyncWordTable[RFUnit];
                                     uiRFCODE[RFUnit]=(unsigned int)gRfiuCustomerCode[RFUnit];
                                     sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_RF_SETTING, 0);
                                     OSTimeDly(1);
    					             DEBUG_RFIU_P2("-->RF-ID Saving:0x%x,0x%x\n",gRfiuSyncWordTable[RFUnit],gRfiuCustomerCode[RFUnit]);
                                     OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_SET, &err);
                                     OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_PAIR_OK<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                                     gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_PAIR_OK;
    						       }
    							   OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_OK<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                                   gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_OK;
                                   
                                   TX_PAIR_ACKflag=0;
                                   TX_SYC_ErrorCnt=0;
                               #endif  
    						   
                               #if RFI_SELF_TEST_TXRX_PROTOCOL
                                   DEBUG_RFIU_P2("\n======>SYNC:ACK Packet received:%d/4,AckCH=%d,DataCH=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum,ACK_CH_sel,DAT_CH_sel);
                               #endif
                                   timerCountRead(guiRFTimerID, &t3);
                                   t5=t3;
                                   t6=t3;
                                   gRfiuUnitCntl[RFUnit].OpMode=RFIU_TX_MODE; 
                               #if( (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) ||\
                                  (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) ) 
                                   if(gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247)  
                                   {
                                       if(ACK_CH_sel != DAT_CH_sel)
                                          DEBUG_RFIU_P2("(%d,%d)",ACK_CH_sel,DAT_CH_sel);
                                       ACK_CH_sel=DAT_CH_sel;
                                   }
                               #endif
                               }
                           }
                           //DEBUG_RFIU_P2("\n");
                        }
                      #if RFI_SELF_TEST_TXRX_PROTOCOL
                        OSTaskChangePrio(RFIU_TASK_PRIORITY_HIGH,RFIU_TASK_PRIORITY_UNIT0);
                      #endif
                    }
                    
                    break;


                case RFIU_PAIR_MODE:
                     TX_TransTotalPkt=0;
                     TXWrapErrMonitor=0;
                     rfiuTxSyncTotolTime=0;
                    //==Chanel selection==//           
                 #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                 #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                 #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                 #elif(RFIC_SEL==RFIC_NONE_5M)
                    RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                 #endif
                    //==Wait ACK==//
                    rfiuTxWaitACKState( RFUnit,
                                        RFI_VITBI_DISA, 
                                        RFI_RS_T12, 
                                        RFI_VITBI_CR4_5,
                                        RFI_PAIR_SYNCWORD,
                                        RFI_PAIR_CUSTOMER_ID,
                                        RFI_TX_WAIT_TIME*2
                                      );    

                 #if RFI_SELF_TEST_TXRX_PROTOCOL
                    DEBUG_RFIU_P2("->PAIR: AckCH=%d\n",ACK_CH_sel);
                 #else
                    DEBUG_RFIU_P2("P");
                 #endif
                 
                    rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx

                    //==Check ACK==//
                    //if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
                    if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 0 )
                    {
                       //DEBUG_RFIU_P2("\n");
                       TX_UsrACK =TX_UsrACK;
                       ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                    }
                    else
                    {  
                    #if TX_FW_UPDATE_SUPPORT
                       rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                    #endif                      
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                       MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                       A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                       A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif(RFIC_SEL==RFIC_NONE_5M)
                       RFNONE_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #endif

                       rfiuDecUsrData(TX_UsrACK,&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.SeqenceNum ++;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpShift=0;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpDivs=0;

                       gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift=0;
                       
                       TX_UsrData_next= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
    				   timerCountRead(guiRFTimerID, &t_seed);	
                       // randow MAC address
                       Temp_TX_MAC_address[RFUnit] = t_seed & 0x0ffff;

                       gRfiuUnitCntl[RFUnit].ProtocolSel=rfiuGetACK2ProtocolType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128);
                       //DEBUG_RFIU_P2("--->ProtocolSel=%d\n",gRfiuUnitCntl[RFUnit].ProtocolSel);
                       rfiuTxSentPairPkt(    
                                             RFUnit,
                                             gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en], 
                                             gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel], 
                                             gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel],
                                             RFI_PAIR_SYNCWORD,
                                             RFI_PAIR_CUSTOMER_ID,
                                             TX_UsrData_next
                                          );
                    #if RFI_SELF_TEST_TXRX_PROTOCOL
                       OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0,RFIU_TASK_PRIORITY_HIGH);
                    #endif
                       //DEBUG_RFIU_P2(" Ran MAC=0x%x, AckCH=%d \n\n",Temp_TX_MAC_address[RFUnit],ACK_CH_sel);
                       rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);                   
                        TX_TimeCheck=gRfiuTimer[RFUnit];
                    
                        //==Wait ACK==//
                    #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                        MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif(RFIC_SEL==RFIC_NONE_5M)
                        RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #endif
                        rfiuTxWaitACKState( RFUnit,
                                            RFI_VITBI_DISA, 
                                            RFI_RS_T12, 
                                            RFI_VITBI_CR4_5,
                                            RFI_PAIR_SYNCWORD,
                                            RFI_PAIR_CUSTOMER_ID,
                                            RFI_TX_WAIT_TIME*2
                                            //200
                                          );    
                        rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                        //if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
                        if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 0 )
                        {
                           //DEBUG_RFIU_P2("TX PAIR No_ACK . AckCH=%d\n",ACK_CH_sel);
                           TX_UsrACK =TX_UsrACK;
                           ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                           //DEBUG_RFIU_P2("\n");
                         
                        }
                        else
                        { 
                        #if TX_FW_UPDATE_SUPPORT
                           rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                        #endif                      
                           TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);

                           if((TX_UsrData_next & RFIU_USRDATA_SEQNUM_CHEK ) == (TX_UsrACK & RFIU_USRDATA_SEQNUM_CHEK))
                           {
                               TX_UsrData_next=TX_UsrACK;  
                               RX_TimeCheck=rfiuGetACK2TimeCheck_Pair(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,
                                                                 &RX_RecvPktCnt,&RX_recvDataPktCnt);
                               if(RX_TimeCheck != 0xffffffff)
                               {
                                   MACtemp = rfiuGetACK2MACAddress(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&RX_RecvPktCnt);
                                   DEBUG_RFIU_P("\nt3:%x,%x\n",MACtemp & 0xffff,(Temp_TX_MAC_address[RFUnit] & 0xffff ) );
                                   //yugo compare MAC address                               
                                   if ( (MACtemp & 0xffff) == (Temp_TX_MAC_address[RFUnit] & 0xffff ) )
                                   {
                                       gRfiuSyncWordTable[RFUnit] = MACtemp ;
                                       gRfiuCustomerCode[RFUnit]=Temp_TX_CostomerCode[RFUnit];
                                   #if 0    
                                       gRfiuTimer_offset[RFUnit] = ((RX_TimeCheck& 0x0ffff)+ 0x10000) - (TX_TimeCheck & 0xffff);
                                   #else
                                       if(RX_TimeCheck >= TX_TimeCheck)
                                          gRfiuTimer_offset[RFUnit] = RX_TimeCheck- TX_TimeCheck;
                                       else
                                          gRfiuTimer_offset[RFUnit] = RX_TimeCheck + TIMER7_COUNT +1- TX_TimeCheck;  
                                   #endif

                                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                                       TxAckRetryCnt=0;
                                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;   //test
                                       rfiuTxSyncTotolTime=0;
                                       timerCountRead(guiRFTimerID, &t7);
                                   #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
                                       A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                                   #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                                       A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                                   #elif(RFIC_SEL==RFIC_NONE_5M)
                                       RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                                   #endif
                                       TX_PAIR_ACKflag=1;
                                       TX_SYC_ErrorCnt=0;
                                   #if RFI_SELF_TEST_TXRX_PROTOCOL
                                       DEBUG_RFIU_P2("\n======>PAIR:ACK Packet received:%d/4,AckCH=%d,DataCH=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum,ACK_CH_sel,DAT_CH_sel);
                                   #endif
                                   }
                               }
                           }
                        }
                      #if RFI_SELF_TEST_TXRX_PROTOCOL
                        OSTaskChangePrio(RFIU_TASK_PRIORITY_HIGH,RFIU_TASK_PRIORITY_UNIT0);
                      #endif
                    }
                    
                    break;  

                case RFIU_PAIRLint_MODE:
                        TX_TransTotalPkt=0;
                        TxAckRetryCnt=0;
                        rfiuTxSyncTotolTime=0;
                        OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                        gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;

                     #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                        rfiuBatCamPIRTrig=0;
                     #endif                                        
                        gRfiuUnitCntl[RFUnit].TX_Pair_Done=0;
                        Old_TXMAC = gRfiuSyncWordTable[RFUnit];
                        Old_TXCODE= gRfiuCustomerCode[RFUnit];
                        gRfiuSyncWordTable[RFUnit]=RFI_PAIR_SYNCWORD;
                        gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                        TX_PAIR_ACKflag=0;
                        TX_SYC_ErrorCnt=0;
                     #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )   
                        A7130_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD );
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD );
                     #elif(RFIC_SEL==RFIC_NONE_5M)
                        RFNONE_ID_Update(RFUnit+1 ,RFI_PAIR_SYNCWORD );
                     #endif

                        if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                        {
                       #if RFIU_RX_AUDIO_RETURN    
                          if(guiIISCh0PlayDMAId != 0xFF)
                          {
                              marsDMAClose(guiIISCh0PlayDMAId);
                              guiIISCh0PlayDMAId = 0xFF;
                          }
                       #endif    
                        
                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
                          OSTimeDly(2);
                          switch(RFUnit)
                          {
                             case 0:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                break;

                             case 1:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                break;

                             case 2:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                break; 

                             case 3:
                                OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                break;     
                          }   
                          gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=0;
                        }

                        gRfiu_WrapEnc_Sta[RFUnit]=RFI_WRAPENC_TASK_NONE;

                        memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));
                        
                        gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                        gRfiuUnitCntl[RFUnit].BufWritePtr=0;
    					gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;

    					for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
    				    {
    				       memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
    				    }

                        for(i=0;i<RFI_FEC_TYPE_MAX;i++)
                        {
                           memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
                        }

                        gRfiuUnitCntl[RFUnit].OpMode=RFIU_PAIR_MODE;
                        gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                        timerCountRead(guiRFTimerID, &t3);

                    break; 

                case RFIU_PAIR_STOP_MODE:
                        TX_TransTotalPkt=0;
                        if(gRfiuUnitCntl[RFUnit].TX_Pair_Done)
                        {
                            gRfiuUnitCntl[RFUnit].OpMode=RFIU_TX_MODE;
                            TX_PAIR_ACKflag=0;
                            TX_SYC_ErrorCnt=0;
                        }
                        else
                        {
                            TxAckRetryCnt=0;
                            gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;
                    
                            gRfiuUnitCntl[RFUnit].TX_Pair_Done=0;
                            gRfiuSyncWordTable[RFUnit]=Old_TXMAC;
                            gRfiuCustomerCode[RFUnit]=Old_TXCODE;
                            TX_PAIR_ACKflag=0;
                            TX_SYC_ErrorCnt=0;
                            TxAckRetryCnt=0;
                        #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
                            A7130_ID_Update(RFUnit+1 ,Old_TXMAC);
                        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                            A7196_ID_Update(RFUnit+1 ,Old_TXMAC);
                        #elif(RFIC_SEL==RFIC_NONE_5M)
                            RFNONE_ID_Update(RFUnit+1 ,Old_TXMAC);
                        #endif
                            timerCountRead(guiRFTimerID, &t3);
                        
                            if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                            {
                            #if RFIU_RX_AUDIO_RETURN    
                                  if(guiIISCh0PlayDMAId != 0xFF)
                                  {
                                     marsDMAClose(guiIISCh0PlayDMAId);
                                     guiIISCh0PlayDMAId = 0xFF;
                                  }
                            #endif    
                                  gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
                                  OSTimeDly(2);
                                  switch(RFUnit)
                                  {
                                     case 0:
                                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                                        break;

                                     case 1:
                                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                                        break;

                                     case 2:
                                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                                        break; 

                                     case 3:
                                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                                        break;     
                                  }  
                                  gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=0;
                            }
                            
                            gRfiu_WrapEnc_Sta[RFUnit]=RFI_WRAPENC_TASK_NONE;

                            memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));
                        
                            gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                            gRfiuUnitCntl[RFUnit].BufWritePtr=0;
    					    gRfiuUnitCntl[RFUnit].GoodDataCHNum=RFI_DAT_CH_MAX;

    					    for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
    				        {
    				           memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
    				        }

                            for(i=0;i<RFI_FEC_TYPE_MAX;i++)
                            {
                               memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
                            }
                            
                            gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;
                            gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                            rfiuTxSyncTotolTime=0;
                            timerCountRead(guiRFTimerID, &t7);
                            
                       }
                    break;
              #if TX_FW_UPDATE_SUPPORT
                //------------------------------------------------------------------------//
                case RFIU_FWUPD_INIT:
                    //--TX--//
                    gRfiuUnitCntl[0].TX_MpegEnc_Stop=1;
                    gRfiuUnitCntl[0].TX_IIS_Stop=1;
                    rfiuTxLifeTotolTime=0;
                    OSTimeDly(6); 

                    for(i=0;i<100;i++)
                    {
                       if(gRfiuUnitCntl[0].TX_MpegEnc_StopRdy == 1)
                         break;
                       OSTimeDly(1);
                    }
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
                    //-------//
                    
                    for(i=0;i<RFI_BUF_SIZE_GRPUNIT;i++)
                    {
                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktCount=64;
                       gRfiuUnitCntl[RFUnit].TxPktMap[i].RetryCount=0;
                       gRfiuUnitCntl[RFUnit].TxPktMap[i].WriteDiv=8;
                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap0 =0xffffffff;
                       gRfiuUnitCntl[RFUnit].TxPktMap[i].PktMap1 =0xffffffff;
                    }
                    
                    timerCountRead(guiRFTimerID, &t3);
                    t4=t3;
                    dt=0;
                    RecvFwUpdStart=0;
                    while(1)
                    {
                        if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                        {
                        }
                        else
                        {
            			    if( gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit] >= TIMER7_COUNT+1)
                                ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit]-(TIMER7_COUNT+1))>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                            else
            					ACK_CH_sel= ( (gRfiuTimer[RFUnit] +gRfiuTimer_offset[RFUnit])>>RFI_ACK_CH_PIRIOD ) % RFI_ACK_CH_MAX;
                      
                        #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                            MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                        #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                            A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                            A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                        #elif(RFIC_SEL==RFIC_NONE_5M)
                            RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                        #endif
                        }
                        rfiuTxWaitACKState( RFUnit,
                                                RFI_VITBI_DISA, 
                                                RFI_RS_T12, 
                                                RFI_VITBI_CR4_5,
                                                gRfiuSyncWordTable[RFUnit],
                                                gRfiuCustomerCode[RFUnit],
                                                RFI_TX_WAIT_TIME
                                              );    

                        RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx

                        if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 1 )
                        {
                            ACKType = rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                            //DEBUG_RFIU_P2("-->Recv:0x%x\n",ACKType);
                            if( RXACK_FWUPD_START == ACKType)
                            {
                                RecvFwUpdStart=1;
                                TxFWSum=Tmp;
                                DEBUG_RFIU_P2("-->Recv START\n");
                                rfiuFwUpdSend_Start_State(  
                                                              RFUnit,
                                                              RFI_VITBI_DISA, 
                                                              RFI_RS_T12, 
                                                              RFI_VITBI_CR4_5,
                                                              gRfiuSyncWordTable[RFUnit],
                                                              gRfiuCustomerCode[RFUnit],
                                                              0
                                                          );
                                rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);
                            }
                        }    
                        //------------------------------------//
                        timerCountRead(guiRFTimerID, &t4);
                        if(t3 >= t4)
                          dt=t3-t4;
                        else
                          dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                        if(dt >30000)
                        {
                            if(RecvFwUpdStart)
                            {
                               gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_START;
                               DEBUG_RFIU_P2("------>FW update Start:0x%x\n",TxFWSum);
                            }
                            else
                            {
                               gRfiuUnitCntl[RFUnit].OpMode=RFIU_IDLE_MODE;
                               sysForceWdt2Reboot=1;
                               DEBUG_RFIU_P2("------>FW update Init time-out! Reboot!\n");
                            }
                            gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                            break;
                        }
                    }
                    ACK_CH_sel=RFI_TXFWUPD_CH;
                    #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                        MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
                    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #elif(RFIC_SEL==RFIC_NONE_5M)
                        RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                    #endif

                    
                    break;
                //------------------------------------------------------------------------//
                case RFIU_FWUPD_START:

                #if 0
                    gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_DONE;
                #else
                    timerCountRead(guiRFTimerID, &t3);
                    t4=t3;
                    dt=0;
                    RecvFwUpdDone=0;
                    gRfiuUnitCntl[RFUnit].BufReadPtr=0;
                    gRfiuUnitCntl[RFUnit].BufWritePtr=RFI_BUF_SIZE_GRPUNIT;

                    while(1)
                    {
                        RecvFwUpdDone=rfiuReplyACK_FWUPD(  
                                                             RFUnit,
                                                             RFI_VITBI_DISA, 
                                                             RFI_RS_T12, 
                                                             RFI_VITBI_CR4_5,
                                                             gRfiuSyncWordTable[RFUnit],
                                                             gRfiuCustomerCode[RFUnit],
                                                             0,
                                                             RX_TimeCheck,
                                                             0,
                                                             0,
                                                             RXACK_FWUPD_DATA,
                                                             TxFWSum
                                                         );
                        if(RecvFwUpdDone)
                        {
                           gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_DONE;
                           DEBUG_RFIU_P2("------>FW update Data Transfer OK:0x%x\n",TxFWSum);
                           break;
                        }
                        
                     #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #else
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #endif

                        rfiuFwUpdListenDataState( RFUnit,
                                                  RFI_VITBI_DISA, 
                                                  RFI_RS_T12, 
                                                  RFI_VITBI_CR4_5,
                                                  gRfiuSyncWordTable[RFUnit],
                                                  gRfiuCustomerCode[RFUnit],
                                                  RFI_RX_WAIT_TIME
                                                 );

                    #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //R
                    #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                    #else
                        RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                    #endif
                        if(gRfiuParm_Rx[RFUnit].TxRxPktNum != 0)
                        {
                           //DEBUG_RFIU_P2("TxRxPktNum=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum);
                           rfiuFwUpdPktMapUpdate(RFUnit);
                        }
                        //------------------------------------//
                        timerCountRead(guiRFTimerID, &t4);
                        if(t3 >= t4)
                          dt=t3-t4;
                        else
                          dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                        if( (dt >600000) || ((dt >50000) && (gRfiuUnitCntl[RFUnit].BufReadPtr<2)) )
                        {
                            gRfiuUnitCntl[RFUnit].OpMode=RFIU_IDLE_MODE;
                            sysForceWdt2Reboot=1;
                            gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                            DEBUG_RFIU_P2("------>FW update Start time-out! Reboot!\n");
                            break;
                        }
                    }
                #endif
                    break;
                //------------------------------------------------------------------------//
                case RFIU_FWUPD_DONE:
                    timerCountRead(guiRFTimerID, &t3);
                    t4=t3;
                    dt=0;
                    RecvFwUpdDone=0;
                    TxFWSum2=0;
                    if(TxFWSum == TxFWSum2)
                        TxFWSum2 +=1;
                    
                    while(1)
                    {
                        rfiuReplyACK_FWUPD(  
                                             RFUnit,
                                             RFI_VITBI_DISA, 
                                             RFI_RS_T12, 
                                             RFI_VITBI_CR4_5,
                                             gRfiuSyncWordTable[RFUnit],
                                             gRfiuCustomerCode[RFUnit],
                                             0,
                                             RX_TimeCheck,
                                             0,
                                             0,
                                             RXACK_FWUPD_DONE,
                                             TxFWSum
                                          );
                     #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL) 
                        rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #else
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
                     #endif

                        rfiuFwUpdListenDataState( RFUnit,
                                                  RFI_VITBI_DISA, 
                                                  RFI_RS_T12, 
                                                  RFI_VITBI_CR4_5,
                                                  gRfiuSyncWordTable[RFUnit],
                                                  gRfiuCustomerCode[RFUnit],
                                                  RFI_RX_WAIT_TIME
                                                 );

                    #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //R
                    #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                    #else
                        RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
                    #endif

                        if( (gRfiuParm_Rx[RFUnit].TxRxPktNum) != 0)
                        {
                            if( rfiuFwUpdCheckDoneCome(&gRfiuParm_Rx[RFUnit]) == 1 )
                            {
                                DEBUG_RFIU_P2("-->PKT_FWUPD_DONE:%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum);
                                RecvFwUpdDone=1;
                            }
                        }
                        //------------------------------------//
                        timerCountRead(guiRFTimerID, &t4);
                        if(t3 >= t4)
                          dt=t3-t4;
                        else
                          dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                        if( (dt >30000) || RecvFwUpdDone)
                        {
                            if(RecvFwUpdDone)
                            {
                               gRfiuUnitCntl[RFUnit].OpMode=RFIU_IDLE_MODE;
                               TxFWSum2=rfiuCalTxFwCheckSum(RFUnit);
                               DEBUG_RFIU_P2("------>FW update Complete:0x%x\n",TxFWSum2);
                            }
                            else
                            {
                               gRfiuUnitCntl[RFUnit].OpMode=RFIU_IDLE_MODE;
                               DEBUG_RFIU_P2("------>FW update Done time-out! Reboot!\n");
                            }
                            gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                            break;
                        }
                    
                    } 
                    //-----Write to Flash----//
                    if(TxFWSum == TxFWSum2)
                    {
                       DEBUG_RFIU_P2("\n===CHECK SUM MATCH,Write to FLASH===\n");
                       if( 0== rfiuFwUpdSaveTxFW2Flash(RFUnit,1024*1024))
                       {
                           DEBUG_RFIU_P2("-->Writting to flash is Fail\n");
                       }
                       else
                           DEBUG_RFIU_P2("-->Writting to flash is Done\n");


                    }
                    else
                    {
                       DEBUG_RFIU_P2("\n===CHECK SUM MISMATCH,FW Update Fail!!===\n");
                    }
                    //-----------------------//
                    sysForceWdt2Reboot=1;
                    OSTimeDly(2);
                    break;
                //------------------------------------------------------------------------//
              #endif
     
                case RFIU_IDLE_MODE:
                default:
                    OSTimeDly(2);
                    break;
                }                          

    	}
    }

    void rfiu_Rx_Task_UnitX(void* pData)
    {

    }
    #endif

#endif


//------------------------------------RX code-----------------------------------//
#if(((SW_APPLICATION_OPTION != MR8120_RFCAM_TX1) && (SW_APPLICATION_OPTION != MR9120_RFCAM_TX5) &&\
    (SW_APPLICATION_OPTION != MR9120_RFCAM_TX5_SUBSTREAM) && (SW_APPLICATION_OPTION != MR9120_RFCAM_TX5_MUTISTREAM) &&\
    (SW_APPLICATION_OPTION != MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) && (SW_APPLICATION_OPTION != MR9211_DUALMODE_TX5) &&\
    (SW_APPLICATION_OPTION != MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) && (SW_APPLICATION_OPTION != MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) &&\
    (SW_APPLICATION_OPTION != MR8110_RFCAM_TX1) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX2) && (SW_APPLICATION_OPTION != MR8110_RFCAM_TX1) &&\
    (SW_APPLICATION_OPTION != MR9120_RFCAM_TX1_SUBSTREAM)  && (SW_APPLICATION_OPTION != MR9120_RFCAM_TX1_MUTISTREAM)))

void rfiu_RXCMD_Dec (char CmdType,int RFUnit,int *pTX_CHG_CHflag)
{}

u32 RXGetTXBorad(unsigned char RFUnit)
{
    return ((gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion[28] << 4) | ((gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion[27] >> 4) & 0xf));
}

u32 RXGetTXPROJ(unsigned char RFUnit)
{
    return gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion[27] & 0xf;
}

u32 RXGetTXFWTime(unsigned char RFUnit)
{
    u32 temp = (gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion[30] << 8) | gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion[29];
    u8 Year = (temp & 0xFE00) >> 9;
    u8 Month = (temp & 0x1E0) >> 5;
    u8 Day = (temp & 0x1f);

    return (((((20 * 100) + Year) * 100) + Month) * 100) + Day;
}

int rfiu_RSSI_Calibration(int RFUnit)
{
    int testrun,i;
    unsigned char *pp;
    unsigned int err;
    GPIO_CFG c;
    unsigned int Vitbi_en,RS_mode,Vitbi_mode;
    unsigned int Old_FCCUnit;
    int DAT_CH_sel;
    u8 RSSI,RSSI2;
    int sum,avg,diff;
    //--------//
    
    FCC_Unit_Sel=RFUnit;
    Old_FCCUnit=0;
    sum=0;
    avg=0;
    Vitbi_en=RFI_VITBI_DISA;
    RS_mode=RFI_RS_T4;
    Vitbi_mode=RFI_VITBI_CR1_2;
   
    //---Setup test environment---//
    rfiu_End();
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].FCC_TestMode= FCC_DIRECT_RX_MODE;
       gRfiuUnitCntl[i].FCC_TX_RealData_ON=0;
       gRfiuUnitCntl[i].FCC_TX_NoData_Zero=0;
       gRfiuUnitCntl[i].FCC_TX_Freq=2408*2;
       gRfiuUnitCntl[i].FCC_RX_Freq=2408*2;
    }
    OSTimeDly(3);
    pp=(unsigned char *)rfiuOperBuf[0];

    DEBUG_RFIU_P2("----------------------------rfiu_RSSI_Calibration---------------------------------\n");
    //-----Test run------//
    testrun=0;
    for(i=0;i<100;i++)
    {
    #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) );            
    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) );  
    #elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400*2) );  
    #endif
        RfiuReset(FCC_Unit_Sel);

        //---config RFU parameter---//
		//------Rx1------//
        gRfiuParm_Rx[FCC_Unit_Sel].TxRxOpBaseAddr       =rfiuOperBuf[0];
        gRfiuParm_Rx[FCC_Unit_Sel].Vitbi_en             =Vitbi_en;
        gRfiuParm_Rx[FCC_Unit_Sel].CovCodeRateSel       =Vitbi_mode;
        gRfiuParm_Rx[FCC_Unit_Sel].RsCodeSizeSel        =RS_mode;
        
        gRfiuParm_Rx[FCC_Unit_Sel].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        gRfiuParm_Rx[FCC_Unit_Sel].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
        gRfiuParm_Rx[FCC_Unit_Sel].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
        
        gRfiuParm_Rx[FCC_Unit_Sel].Customer_ID_ext_en   =0;
        gRfiuParm_Rx[FCC_Unit_Sel].SuperBurstMode_en    =0;
        gRfiuParm_Rx[FCC_Unit_Sel].Customer_ID          =0x1234;
        
        gRfiuParm_Rx[FCC_Unit_Sel].UserData_L           =0x0;
        gRfiuParm_Rx[FCC_Unit_Sel].UserData_H           =0x0;
        gRfiuParm_Rx[FCC_Unit_Sel].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        gRfiuParm_Rx[FCC_Unit_Sel].PktSyncWord          =0x17df83ff;
        
        gRfiuParm_Rx[FCC_Unit_Sel].TxClkConfig          =RFI_TXCLKCONFIG;
        gRfiuParm_Rx[FCC_Unit_Sel].RxClkAdjust          =RFI_RXCLKADJUST;
        gRfiuParm_Rx[FCC_Unit_Sel].DclkConfig           =RFI_DCLKCONF;
        gRfiuParm_Rx[FCC_Unit_Sel].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

        gRfiuParm_Rx[FCC_Unit_Sel].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

        rfiuDataPktConfig_Rx(FCC_Unit_Sel,&(gRfiuParm_Rx[FCC_Unit_Sel]),FCC_Unit_Sel );  //Rx            
        RSSI=rfiuWaitForInt_Rx(FCC_Unit_Sel,&gRfiuParm_Rx[FCC_Unit_Sel],0,&RSSI2);  //Rx
        sum +=RSSI;
        DEBUG_RFIU_P2("RX-%d:Freq=%d,RSSI=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq/2,RSSI);
        testrun ++;
    }
    avg=sum/testrun;
    DEBUG_RFIU_P2("-->Avg of RSSI=%d\n",avg);

    diff=RFIU_RSSI_THR-avg;

    if(diff > 20)
        diff=20;
    if(diff < -20)
        diff= -20;

    if( RFUnit & 0x01)
       rfiuRSSI_CALDiff[1]=rfiuRSSI_CALDiff[3]=diff;
    else
       rfiuRSSI_CALDiff[0]=rfiuRSSI_CALDiff[2]=diff;

    return diff;

}

 #if TX_FW_UPDATE_SUPPORT
        
    int rfiuFwUpdCheckStartCome(DEF_REGRFIU_CFG *pRfiuPara)
    {
        int i;
        unsigned int CmdID;
        //-------------//
        
        for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
        {
            CmdID=(pRfiuPara->Pkt_Grp_offset[i] >> 7);
            if((pRfiuPara->PktMap[2*i] & RFI_FWUPD_START_ADDR_CHEKBIT) && ( CmdID == RFI_CMD_ADDR_OFFSET))
            {
               return 1;
            }
        }

        return 0;
    }
 
    int rfiuFwUpdSend_Data_State(  unsigned char RFUnit,
                                          unsigned int Vitbi_en, 
                                          unsigned int RS_mode, 
                                          unsigned int Vitbi_mode,
                                          unsigned int SyncWord,
                                          unsigned int CustomerID,
                                          unsigned int UserData
                                        )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        unsigned int nextBufReadPtr;
        unsigned int *pp,*qq;
        int i;
        
        //-------//
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
        
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;


        //---Set User Data---//
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;     
        pRfiuPara_Tx->UserData_H           =(UserData>>16) & RFI_USER_DATA_H_MASK;
          
        //----//
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        RfiuReset(RFUnit & 0x01);
        rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);  //Tx
#else
        RfiuReset(RFUnit);
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);  //Tx
#endif

        return 1;
    }
 
    int rfiuFwUpdSend_Done_State(  int RFUnit,
                                            unsigned int Vitbi_en, 
                                            unsigned int RS_mode, 
                                            unsigned int Vitbi_mode,
                                            unsigned int SyncWord,
                                            unsigned int CustomerID,
                                            unsigned int UserData
                                        )
    {
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        DEF_REGRFIU_CFG *pRfiuPara_Rx;
        int i;
        //------//
        
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
                       
        //---config RFU parameter---//
        //-Tx-//
        pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
        pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
        pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
        pRfiuPara_Tx->PktSyncWord          =SyncWord;
        pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
        
        pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
        pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
        pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                    
        pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
        pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
        pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
        pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
        
        pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
        pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
        pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

        for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
        {
            pRfiuPara_Tx->PktMap[2*i]      =RFI_FWUPD_DONE_ADDR_CHEKBIT;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_SYNC_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
        RfiuReset(RFUnit & 0x01);
        rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);  //Tx
#else
        RfiuReset(RFUnit);
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);  //Tx
#endif

        return 1;
    }

    int rfiuGetACK2FwUpdDataMap(
                                              unsigned char RFUnit,
                                              unsigned char *ACKBufAddr
                                           )
    {
        unsigned int *pp;
        int i,j,count,bitcount;
        unsigned int pktmap;
        DEF_REGRFIU_CFG *pRfiuPara_Tx;
        int UpdatePtr;    
        //=====//
        count=0;  
        pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
        pRfiuPara_Tx->TxRxPktNum   =0;
        pRfiuPara_Tx->DummyPktNum  =0;  

        pp  = (unsigned int *)(ACKBufAddr);
        UpdatePtr= (*pp ^0x5a5aa5a5) & 0x07ff;
        
        while( (count <32) && (*pp != 0xa55aaa55))
        {
            //---Read Group value---//       
            for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
            {
                *pp ^= 0x5a5aa5a5;
    	 #if RF_CMD_EN
                pRfiuPara_Tx->Pkt_Grp_offset[i*2]  = ((*pp >>0) & 0x07ff)<<13;
                pRfiuPara_Tx->Pkt_Grp_offset[i*2+1]= ((*pp >>11) & 0x07ff)<<13; 
    	 #else
                pRfiuPara_Tx->Pkt_Grp_offset[i*2]  = (((*pp >>0) & 0x0ffff)>>5)<<13;
                pRfiuPara_Tx->Pkt_Grp_offset[i*2+1]= (((*pp >>16) & 0x0ffff)>>5)<<13;
    	 #endif		                  
    			pp ++;
                count ++;
            }
            
            //---Read Map value---//
            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
            {
               pRfiuPara_Tx->PktMap[i*2]=((*pp) ^ 0x5a5aa5a5);
               pp ++;
               count ++;
               //------//
               pRfiuPara_Tx->PktMap[i*2+1] = ((*pp) ^ 0x5a5aa5a5);
               pp ++;
               count ++;

            }
            //--Read password--//
    	#if RF_CMD_EN        
            for(i=0;i<RFIU_PASSWORD_MAX;i++)
            {
               pp ++;
               count ++;
            }    
            //--RX Cmd Data[5~7]---//  
            pp ++;
            count ++;

            //--Status--//
            pp ++;
            count ++;

            //--Customer ID---//
            pp ++;
            count ++;   
    	#endif	
            //---RX timer---//
            pp ++;
            count ++;

            //---RX receive packet count---//
            pp ++;
            count ++;
        }
        //-------//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {

           bitcount =0;           
           pktmap=pRfiuPara_Tx->PktMap[i*2];
           if(pktmap != 0)
           {
               for(j=0;j<32;j++)
               {
                  if(pktmap & 0x01)
                      bitcount ++;
                  pktmap >>=1;
               }
           }
           
           pktmap=pRfiuPara_Tx->PktMap[i*2+1];
           if(pktmap != 0)
           {
               for(j=0;j<32;j++)
               {
                  if(pktmap & 0x01)
                      bitcount ++;
                  pktmap >>=1;

               }
           }
           pRfiuPara_Tx->TxRxPktNum +=bitcount;    
        }

        if(pRfiuPara_Tx->TxRxPktNum < 32)
            pRfiuPara_Tx->TxRxPktNum=32;   

        if(pRfiuPara_Tx->TxRxPktNum > RFI_MAX_BURST_NUM)
            pRfiuPara_Tx->TxRxPktNum=RFI_MAX_BURST_NUM;   

        return UpdatePtr;
    }


	u8 *rfiuGetTxUpgradeBuf(void)
 	{
 		return rfiuTXFwUpdBuf;
 	}

 	char *rfiuGetTxVersionName(int RFUnit)
 	{
 		if(MAX_RFIU_UNIT <= RFUnit)
 			return NULL;

 		if((rfiuRX_CamOnOff_Sta & (0x01 << RFUnit)) == 0)
 			return NULL;
 			
 		return gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion;
 	}

 	int rfiuGetTxUpgradePercentage(int RFUnit)
 	{
 		if(MAX_RFIU_UNIT <= RFUnit)
 			return -1;

 		if((rfiuRX_CamOnOff_Sta & (0x01 << RFUnit)) == 0)
 			return -1;
            
 		return gRfiuTxFwUpdPercent[RFUnit];
 	}
 	 
    int rfiuFwUpdLoadTxFW_SD(int RFUnit)
    {
        u8* codeAddr;
        FS_FILE* pFile;
        u32 codeSize;
        u8  tmp, digest[16];
    	MD5_CTX ctx;
        int i;
        char buf2[50], buf3[2], MD5_buf[33];
        
        //-----------------//
        //codeAddr=(unsigned char *)rfiuOperBuf[RFUnit];;
        codeAddr=(unsigned char *)rfiuTXFwUpdBuf;
        
        if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
        {
            if ((pFile = dcfBackupOpen((signed char*)ispTxFWFileName[RFUnit], "rb")) == NULL)
            {
                DEBUG_ISP("Error: dcf open error!\n");
                return 0;
            }

            dcfBackupRead(pFile, codeAddr, pFile->size, &codeSize);
            DEBUG_ISP("FwUpdLoadTxFW_SD: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);
            /* close file */
            dcfBackupClose(pFile);

        }
        else
        {
            if ((pFile = dcfOpen((signed char*)ispTxFWFileName[RFUnit], "rb")) == NULL)
            {
                DEBUG_ISP("Error: dcf open error!\n");
                return 0;
            }

            if( dcfRead(pFile, codeAddr, pFile->size, &codeSize)==0)
            {
                return 0;
            }
            DEBUG_ISP("FwUpdLoadTxFW_SD: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);
            /* close file */
            dcfClose(pFile, &tmp);
        }

        if (codeSize == 0)
        {
            DEBUG_ISP("ISP Err: Code Size is 0 Byte!\n");
            DEBUG_ISP("Quit Code Update!\n");
            return 0;
        }

        MD5Init(&ctx);
    	MD5Update(&ctx, (unsigned char*)codeAddr+0x100,pFile->size-0x100);
        MD5Final(digest,&ctx);
        for (i = 0; i < 16; i++)
        {
            sprintf(buf3,"%02x",digest[i]);
            buf2[2*i]=buf3[0];
            buf2[2*i+1]=buf3[1];
        }
        buf2[32]='\0';

        for(i = 0; i < 32; i++)
        {
            MD5_buf[i]=(codeAddr+0xa0)[i];
        }
        MD5_buf[32]='\0';
        DEBUG_ISP("MD5:\n"\
                  "PC     Compute :%s\n"\
                  "Device Compute :%s\n",MD5_buf, buf2);

        if(strncmp(MD5_buf,buf2,32))
        {
            DEBUG_ISP("TX SD MD5 check error!!\n");
            return 0;
        }              

        return 1;

    }

    int rfiuFwUpdLoadTxFW_NET(int RFUnit)
    {
        //u8* codeAddr;

        //codeAddr=(unsigned char *)rfiuTXFwUpdBuf;

        //memcpy(codeAddr,Souce,1024*1024);
       	//DEBUG_RFIU_P2("rfiuFwUpdLoadTxFW_NET is not support new!!\n");
        return 1;
    }

    int rfiuFwUpdLoadTxFW_USB(int RFUnit)
    {
        u8* codeAddr;
        int codesize;
        u8  digest[16];
    	MD5_CTX ctx;
        int i;
        char buf2[50], buf3[2], MD5_buf[33];
        
        //-------------------------------------//
        codeAddr=(unsigned char *)rfiuTXFwUpdBuf;
        codesize=1024*1024;

        MD5Init(&ctx);
    	MD5Update(&ctx, (unsigned char*)codeAddr+0x100,codesize-0x100);
        MD5Final(digest,&ctx);
        for (i = 0; i < 16; i++)
        {
            sprintf(buf3,"%02x",digest[i]);
            buf2[2*i]=buf3[0];
            buf2[2*i+1]=buf3[1];
        }
        buf2[32]='\0';

        for(i = 0; i < 32; i++)
        {
            MD5_buf[i]=(codeAddr+0xa0)[i];
        }
        MD5_buf[32]='\0';
        DEBUG_ISP("MD5:\n"\
                  "PC     Compute :%s\n"\
                  "Device Compute :%s\n",MD5_buf, buf2);

        if(strncmp(MD5_buf,buf2,32))
        {
            DEBUG_ISP("TX SD MD5 check error!!\n");
            return 0;
        }
        
        return 1;
    }


    int rfiuTxFwUpdateFromSD(int RFUnit)
    {
        int count;
        char *TxFwVer;
        int Len;
        
        if( 0==rfiuFwUpdLoadTxFW_SD(RFUnit) )
        {
            gRfiuTxFwUpdPercent[RFUnit]=-1;
            DEBUG_RFIU_P2("------>FW-update From SD is Fail! Go to RX mode\n");
            return 0;
        }
        TxFwVer=rfiuTXFwUpdBuf+0x1f00;
        Len = strlen(gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
        //DEBUG_RFIU_P2("==>TXFW OLD VERSION:%s,%d\n",gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,Len); //TX 版號會多一個空格
        //DEBUG_RFIU_P2("==>TXFW NEW VERSION:%s,%d\n",TxFwVer,strncmp(TxFwVer,gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,Len-1));
        
        if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0 )
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>Cam-%d is OFF! Go to RX mode\n",RFUnit);
            return 0;
        }
        else if( 0 == strncmp(TxFwVer,gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,Len-1) )
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>TX FW Version is the same! Go to RX mode\n",RFUnit);
            return 0;
        }
    #if RFIU_RX_WAKEUP_TX_SCHEME
        else if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && ( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) ) )
        {
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=1;
            count=0;
            do
            {
                OSTimeDly(20);
                count ++;
                DEBUG_RFIU_P2("-->WakeUp TX-%d:%d\n",RFUnit,count);
            }while( (gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_OK) && (count<RF_BATCAM_RXWAKEUPTX_MAXTIME) );
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=0;
        }

        gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
        rfiuBatCam_LiveMaxTime[RFUnit]=0;
        rfiuBatCamVideo_TotalTime[RFUnit]=0;
    #endif
        
        gRfiuTxFwUpdPercent[RFUnit]=0;
        if(gRfiuUnitCntl[RFUnit].FWUpdate_support && (gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_OK) )
        {
           gRfiuUnitCntl[RFUnit].FWUpdate_Source=RF_TXFWUPDATE_FROM_SD;
           gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_INIT;

           return 1;
        }
        else
        {
           DEBUG_RFIU_P2("RF-Link is FAIL!! \r\n");
           gRfiuTxFwUpdPercent[RFUnit]=-1;
           return 0;
        }
    }

    int rfiuTxFwUpdateFromNet(int RFUnit)
    {
        int count;
        char *TxFwVer;
        int Len;
        
        if( 0==rfiuFwUpdLoadTxFW_NET(RFUnit) )
        {
            gRfiuTxFwUpdPercent[RFUnit]=-1;
            DEBUG_RFIU_P2("------>FW-update From Net is Fail! Go to RX mode\n");
            return 0;
        }
        TxFwVer=rfiuTXFwUpdBuf+0x1f00;
        Len = strlen(gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
        
        if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0 )
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>Cam-%d is OFF! Go to RX mode\n",RFUnit);
            return 0;
        }
        else if(0==strncmp(TxFwVer,gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,Len-1))
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>TX FW Version is the same! Go to RX mode\n",RFUnit);
            return 0;
        }
    #if RFIU_RX_WAKEUP_TX_SCHEME
        else if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && ( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) ) )
        {
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=1;
            count=0;
            do
            {
                OSTimeDly(20);
                count ++;
                DEBUG_RFIU_P2("-->WakeUp TX-%d:%d\n",RFUnit,count);
            }while( (gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_OK) && (count<RF_BATCAM_RXWAKEUPTX_MAXTIME) );
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=0;
        }

        gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
        rfiuBatCam_LiveMaxTime[RFUnit]=0;
        rfiuBatCamVideo_TotalTime[RFUnit]=0;
    #endif


        gRfiuTxFwUpdPercent[RFUnit]=0;
        if(gRfiuUnitCntl[RFUnit].FWUpdate_support && (gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_OK) )
        {
           gRfiuUnitCntl[RFUnit].FWUpdate_Source=RF_TXFWUPDATE_FROM_NET;
           gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_INIT;

           return 1;
        }
        else
        {
           DEBUG_RFIU_P2("RF-Link is FAIL!! \r\n");
           gRfiuTxFwUpdPercent[RFUnit]=-1;
           return 0;
        }
    }

    int rfiuTxFwUpdateFromUSB(int RFUnit)
    {
        int count;
        char *TxFwVer;
        int Len;
        
        if( 0==rfiuFwUpdLoadTxFW_USB(RFUnit) )
        {
            gRfiuTxFwUpdPercent[RFUnit]=-1;
            DEBUG_RFIU_P2("------>FW-update From USB is Fail! Go to RX mode\n");
            return 0;
        }

        TxFwVer=rfiuTXFwUpdBuf+0x1f00;
        Len = strlen(gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
        
        if( (rfiuRX_CamOnOff_Sta & (0x01<<RFUnit)) == 0 )
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>Cam-%d is OFF! Go to RX mode\n",RFUnit);
            return 0;
        }
        else if(0==strncmp(TxFwVer,gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,Len-1))
        {
            gRfiuTxFwUpdPercent[RFUnit]=100;
            DEBUG_RFIU_P2("------>TX FW Version is the same! Go to RX mode\n",RFUnit);
            return 0;
        }
    #if RFIU_RX_WAKEUP_TX_SCHEME
        else if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && ( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) ) )
        {
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=1;
            count=0;
            do
            {
                OSTimeDly(20);
                count ++;
                DEBUG_RFIU_P2("-->WakeUp TX-%d:%d\n",RFUnit,count);
            }while( (gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_OK) && (count<RF_BATCAM_RXWAKEUPTX_MAXTIME) );
            gRfiuUnitCntl[RFUnit].WakeUpTxEn=0;
        }

        gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
        rfiuBatCam_LiveMaxTime[RFUnit]=0;
        rfiuBatCamVideo_TotalTime[RFUnit]=0;
    #endif

        gRfiuTxFwUpdPercent[RFUnit]=0;
        if(gRfiuUnitCntl[RFUnit].FWUpdate_support && (gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_OK) )
        {
           gRfiuUnitCntl[RFUnit].FWUpdate_Source=RF_TXFWUPDATE_FROM_USB;
           gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_INIT;

           return 1;
        }
        else
        {
           DEBUG_RFIU_P2("RF-Link is FAIL!! \r\n");
           gRfiuTxFwUpdPercent[RFUnit]=-1;
           return 0;
        }
    }  


    int rfiuTxFWUPD_AllTX(int SourceSel)
    {
         int i;
         int RFUnit;
         int count;
            
         for(RFUnit=0;RFUnit < MAX_RFIU_UNIT; RFUnit ++)
         {
            switch(SourceSel)
            {
               case RF_TXFWUPDATE_FROM_SD:
                  rfiuTxFwUpdateFromSD(RFUnit); 
                  break;

               case RF_TXFWUPDATE_FROM_NET:
                    rfiuTxFwUpdateFromNet(RFUnit);             
                 break;
                 
               case RF_TXFWUPDATE_FROM_USB:
                    rfiuTxFwUpdateFromUSB(RFUnit);             
                 break;

                  
            }  
            count=0;
            while( (gRfiuTxFwUpdPercent[RFUnit] >=0) && (gRfiuTxFwUpdPercent[RFUnit]<100) )
            {
               OSTimeDly(20);
               count ++;
               if(count > 100)
               {
                  DEBUG_RFIU_P2("ERROR!! TX-%d FW UPDATE TIMEOUT!\n",RFUnit);
                  gRfiuTxFwUpdPercent[RFUnit]=-1;
                  break;
               }  
            }

            if(gRfiuTxFwUpdPercent[RFUnit]<0)
                DEBUG_RFIU_P2("TX-%d FW UPDATE is FAIL!\n",RFUnit);
            else
                DEBUG_RFIU_P2("TX-%d FW UPDATE is SUCCESS!\n",RFUnit);
 
         }   
         
    }
    
    
 #endif


int rfiuRXJdgDataSwich_3M(int RFUnit,int BitRate,int DAT_CH_sel)
{
    int i,Div;
    int CamSta,CamNum;
    static int Last6M_BRavg[MAX_RFIU_UNIT]={0x0fff};
    static int Last3M_BRavg[MAX_RFIU_UNIT]={0};
    //-------------------//
    if(gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_BROKEN)
        return 0;

    Div=1;
#if RFI_TEST_2x_RX_PROTOCOL_B1
    if( rfiuRX_CamOnOff_Num == 2 )
        Div=2;
    else
        Div=1;

#elif RFI_TEST_4TX_2RX_PROTOCOL
    if(RFUnit & 0x01)
    {
       if( (rfiuRX_CamOnOff_Sta & 0xa)== 0xa )
          Div=2;
       else
          Div=1;
    }
    else
    {
       if( (rfiuRX_CamOnOff_Sta & 0x5)== 0x5 )
          Div=2;
       else
          Div=1;
    }
#elif RFI_TEST_8TX_2RX_PROTOCOL
    if(RFUnit & 0x01)
       CamSta=rfiuRX_CamOnOff_Sta & 0xaa;
    else
       CamSta=rfiuRX_CamOnOff_Sta & 0x55;
    
    CamNum=0;
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       if(CamSta & 0x01)
          CamNum ++;
       CamSta >>=1;
    }
    
    if( CamSta >=3 )
        Div=3;
    else if( CamSta ==2 )
        Div=2;
    else
        Div=1;

#elif RFI_TEST_8TX_1RX_PROTOCOL
    if( rfiuRX_CamOnOff_Num >=6 )
        Div=6;
    else if( rfiuRX_CamOnOff_Num >=3 )
        Div=3;
    else if( rfiuRX_CamOnOff_Num ==2 )
        Div=2;
    else
        Div=1;
    
    
#elif RFI_TEST_4x_RX_PROTOCOL_B1
    if( rfiuRX_CamOnOff_Num ==4 )
        Div=3;
    else if( rfiuRX_CamOnOff_Num ==3 )
        Div=3;
    else if( rfiuRX_CamOnOff_Num ==2 )
        Div=2;
    else
        Div=1;

#endif

    if(gRfiuUnitCntl[RFUnit].RXRecvDataUse3M == 0) 
    {   //===6M mode===//
        if(BitRate <= RFDATA_6M_TO_3M_THR/Div) 
        {
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=1;
        }
        else
        {
            if(BitRate > Last3M_BRavg[RFUnit] )
               gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
            else
               gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=1;
        }

        if(Last6M_BRavg[RFUnit] == 0xfff)
            Last6M_BRavg[RFUnit]=BitRate;
        else
            Last6M_BRavg[RFUnit]=(Last6M_BRavg[RFUnit]+BitRate)/2;
        
        RF3M_RunCnt[RFUnit]=0;
    }
    else
    {   //===3M mode===//
        if(BitRate >= RFDATA_3M_TO_6M_THR/Div) 
        {
            if(BitRate > Last6M_BRavg[RFUnit] )
            {
               gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=1;
            }
            else
            {
               gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
            }   
        }
        else
        {
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=1;
        }
        
        if(Last3M_BRavg[RFUnit]==0)
            Last3M_BRavg[RFUnit]=BitRate;
        else
            Last3M_BRavg[RFUnit] = (Last3M_BRavg[RFUnit] + BitRate)/2;
        
        RF3M_RunCnt[RFUnit] ++;
        //Force to 6M every 10 sec
        if(RF3M_RunCnt[RFUnit]>RFKEEP_3M_TIME)
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
    }

    if(gRfiuUnitCntl[RFUnit].RXRecvDataUse3M)
    {
       //DEBUG_RFIU_P2("-->%d,%d,%d,%d\n",RFUnit,BitRate,Last6M_BRavg[RFUnit],DAT_CH_sel);
    }
    else
    {
       //DEBUG_RFIU_P2("==>%d,%d,%d,%d\n",RFUnit,BitRate,Last3M_BRavg[RFUnit],DAT_CH_sel);
    }

    
    return 1;
}

int rfiu_nTnR_FindNextTx(int RFUnit)
{
   int i;
   int NextTx;
   u32 Bits;

   
#if RFI_TEST_2x_RX_PROTOCOL_B1
   NextTx=((RFUnit+1) & 0x01);
   for(i=0;i<2;i++)
   {
      Bits=1 << NextTx;
      if(rfiuRX_CamOnOff_Sta & Bits)
      {
         return NextTx;
      }

      NextTx=((NextTx+1) & 0x01);
   }
#elif RFI_TEST_4TX_2RX_PROTOCOL
   NextTx=((RFUnit+2) & 0x03);
   for(i=0;i<2;i++)
   {
      Bits=1 << NextTx;
      if(rfiuRX_CamOnOff_Sta & Bits)
      {
         return NextTx;
      }

      NextTx=((NextTx+2) & 0x03);
   }
   
#elif RFI_TEST_8TX_2RX_PROTOCOL
   NextTx=((RFUnit+2) & 0x07);
   for(i=0;i<4;i++)
   {
      Bits=1 << NextTx;
      if(rfiuRX_CamOnOff_Sta & Bits)
      {
         return NextTx;
      }

      NextTx=((NextTx+2) & 0x07);
   }

#elif RFI_TEST_4x_RX_PROTOCOL_B1
   NextTx=((RFUnit+1) & 0x03);
   for(i=0;i<4;i++)
   {
      Bits=1 << NextTx;
      if(rfiuRX_CamOnOff_Sta & Bits)
      {
         return NextTx;
      }

      NextTx=((NextTx+1) & 0x03);
   }

#elif RFI_TEST_8TX_1RX_PROTOCOL
   NextTx=((RFUnit+1) & 0x07);
   for(i=0;i<8;i++)
   {
      Bits=1 << NextTx;
      if(rfiuRX_CamOnOff_Sta & Bits)
      {
         return NextTx;
      }

      NextTx=((NextTx+1) & 0x07);
   }
  
#endif

    return -1;

}

int rfiuPutPacketMap2ACK(int RFUnit, unsigned char *ACKBufAddr,
                                  DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,
                                  unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext)
{
    unsigned int *pp;
    int i,count;
    unsigned int RFTimer;
    //=====//
    timerCountRead(guiRFTimerID, &RFTimer);
    
    count=0;    
    pp  = (unsigned int *)(ACKBufAddr);
    //
    for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
    {
    #if RF_CMD_EN
        *pp = (((pRfiuPara->Pkt_Grp_offset[i*2]>>13) & 0x7ff)<<0) |
              (((pRfiuPara->Pkt_Grp_offset[i*2+1]>>13) & 0x7ff)<<11) |
              (((gRfiuUnitCntl[RFUnit].RX_CMD_Data[i]) & 0xff)<<24);
    #else
        *pp = (((pRfiuPara->Pkt_Grp_offset[i*2]>>8) & 0xffff)<<0) |
              (((pRfiuPara->Pkt_Grp_offset[i*2+1]>>8) & 0xffff)<<16);
    #endif
		
        *pp ^= 0x5a5aa5a5;
        pp ++;
        count ++;
    }
    
    //
    for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
    {
       *pp=pRfiuPara->PktMap[i] ^ 0x5a5aa5a5; //Xor 0101b
       pp ++;
       count ++;
    }

  #if RF_CMD_EN	
    //--Put password--//
    #if 0
    for(i=0;i<RFIU_PASSWORD_MAX;i++)
    {
       *pp=gRfiuUnitCntl[RFUnit].RX_PASSWORD[i] ^ 0x5a5aa5a5; 
       pp ++;
       count ++;
    }    
    #endif
    
    *pp = RXACK_NORMAL;
    pp ++;
    count ++;

    *pp=RFTimer;
    pp ++;
    count ++;

    //--RX Cmd Data[5~7]
    *pp= (gRfiuUnitCntl[RFUnit].RX_CMD_Data[5] | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]<<8) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[7]<<16) | ( (gRfiuUnitCntl[RFUnit].BufWritePtr & 0x0ff)<<24) ) ^ 0x5a5aa5a5; 
    pp ++;
    count ++;

    //--Send Status: CH number bit3:0, RX Cmd[8~10] --//
    *pp= ( ( (RFUnit & 0x07) | ((gRfiuUnitCntl[RFUnit].RXRecvDataUse3M & 0x01)<<3)  | ( ( gRfiuUnitCntl[RFUnit].ProtocolSel & 0x03 )<<4) | ((gRfiuUnitCntl[RFUnit].VMDSel & 0x01)<<6) | ( (CH_chg_flag & 0x01)<<7) ) | 
           (gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]<<8) | 
           (gRfiuUnitCntl[RFUnit].RX_CMD_Data[9]<<16) | 
           ( (RX_CHG_CHNext & 0xff)<<24) )^ 0x5a5aa5a5; 
    pp ++;
    count ++;

    //--Reserved or Customer Code: 1 word---//
    if (MACadrsSetflag[RFUnit] >0) 
       *pp = Temp_RX_CostomerCode[RFUnit] ;
    else
       *pp= RF_CUSTOMERID_SET ^ 0x5a5aa5a5; 
    pp ++;
    count ++;
  #endif  
    //--Put time checker--//
    *pp = RX_TimeCheck;
    pp ++;
    count ++;
    
    //--Put Receive packet count--//
   #if(RF_PAIR_EN) 
    if (MACadrsSetflag[RFUnit] >0) 
      *pp = Temp_RX_MAC_Address[RFUnit] ;
    else
      *pp = pRfiuPara->TxRxPktNum & 0x0ffff;          
   #else
    *pp = pRfiuPara->TxRxPktNum & 0x0ffff;     
   #endif
    *pp ^= 0x5a5aa5a5;
    pp ++;
    count ++;
    //
    for(i=count;i<32;i++)
    {
       *pp = 0xa55aaa55;//magic number for End.
       pp ++;
       count ++;
    }

    return 1;

}

//Lucian: Only Update Time-Check
int rfiuPutTimeCheck2ACK(int RFUnit,unsigned char *ACKBufAddr,DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext)
{
    unsigned int *pp;
    unsigned int temp;
    int i,count;
    unsigned int RFTimer;
    //=====//
    timerCountRead(guiRFTimerID, &RFTimer);
    count=0;    
    pp  = (unsigned int *)(ACKBufAddr);
    //
    for(i=0;i<RFIU_USEPKTMAP_MAX/4;i++)
    {
        pp ++;
        count ++;
    }
    //
    for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
    {
       pp ++;
       count ++;
    }
 #if RF_CMD_EN	
    //--Put password--//
   #if 0 
    for(i=0;i<RFIU_PASSWORD_MAX;i++)
    {
       pp ++;
       count ++;
    }
   #endif  
    *pp = RXACK_NORMAL; 
    pp ++;
    count ++;
   
    *pp= RFTimer;
    pp ++;
    count ++;

    //--RX Cmd Data[5~7]
    pp ++;
    count ++;

    //--Send Status: CH number bit2:0, --//
    temp= *pp ^ 0x5a5aa5a5; 
    //temp= (temp & (~0x30)) | ( ( gRfiuUnitCntl[RFUnit].ProtocolSel & 0x03 )<<4);
    temp= (temp & (~0x80)) | ((CH_chg_flag & 0x01)<<7);
    temp= (temp & (~0xff000000)) | ((RX_CHG_CHNext & 0xff) << 24);
    temp= (temp & (~0x08)) | ((gRfiuUnitCntl[RFUnit].RXRecvDataUse3M & 0x01)<<3);
    *pp = temp ^ 0x5a5aa5a5;
    pp ++;
    count ++;

    //--Customer ID---//
    pp ++;
    count ++;
 #endif      
    //--Put time checker--//
    *pp = RX_TimeCheck;
    pp ++;
    count ++;
    
    //--Put Receive packet count--//
    pp ++;
    count ++;
    //
    for(i=count;i<32;i++)
    {
       pp ++;
       count ++;
    }

    return 1;

}

int rfiuProcessCmdPkt(unsigned char RFUnit)
{
    DEF_REGRFIU_CFG *pRfiuPara;
    int i,j;
    unsigned int CmdID;
    unsigned int *pp;
    unsigned int  cpu_sr = 0;

    unsigned int t_seed;
    u8 err;
    int Ret_Group,Ret_map0,Ret_map1;
    int count;
    unsigned int pktmap0,pktmap1,GrpDivs;
    u32 AudioGrpWritePtr,AudioGrpReadPtr;
    int GrpWROffset;
    int tmp;
    //===//

    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==1)
        return 0;

    pRfiuPara= &gRfiuParm_Rx[RFUnit];

    for(i=0;i<RFI_ACK_SYNC_PKTNUM;i++)
    {
       CmdID=(pRfiuPara->Pkt_Grp_offset[i]>>7);
       if((pRfiuPara->PktMap[2*i] != 0) && ( CmdID == RFI_CMD_ADDR_OFFSET))
       {
           //SYNC Packet//
           if(pRfiuPara->PktMap[2*i] & RFI_SYNC_ADDR_CHEKBIT)
           {
               pp=(unsigned int *)(pRfiuPara->TxRxOpBaseAddr+RFI_SYNC_ADDR_OFFSET*128);
               //--write RX write pointer--//
               OS_ENTER_CRITICAL();
               gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
               gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
               gRfiuUnitCntl[RFUnit].BufWritePtr= *pp;
               gRfiuUnitCntl[RFUnit].BufReadPtr= *pp;
               OS_EXIT_CRITICAL();
               pp ++; 
               //--read picture width and height--//
               gRfiuUnitCntl[RFUnit].TX_PicWidth = *pp & 0x0ffff;
               gRfiuUnitCntl[RFUnit].TX_PicHeight= (*pp >>16);
               pp ++;
               //----Read TX statsu----//              
               gRfiuUnitCntl[RFUnit].TX_Status= *pp;
               pp++;
               
        #if TX_FW_UPDATE_SUPPORT
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STAT_FWUPD_SUPPORT)
                  gRfiuUnitCntl[RFUnit].FWUpdate_support=1;
               else
                  gRfiuUnitCntl[RFUnit].FWUpdate_support=0;
        #endif
        
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__MD_ON)
                  gRfiuUnitCntl[RFUnit].RFpara.MD_en=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.MD_en=0;

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__PIR_ON)
                  gRfiuUnitCntl[RFUnit].RFpara.PIR_en=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.PIR_en=0;

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_FLICKER_50HZ)
                  gRfiuUnitCntl[RFUnit].RFpara.TxSensorAE_FlickSetting=SENSOR_AE_FLICKER_50HZ;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.TxSensorAE_FlickSetting=SENSOR_AE_FLICKER_60HZ; 

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FULLHD_SUPPORT)
               {
                   if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__HD_SUPPORT)
                      gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_FHD;
                   else
                      gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_UXGA;
               }
               else
               {
                   if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__HD_SUPPORT)
                      gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
                   else
                      gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;
               }

          #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )//time stamp can be on/off in menu 
               gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=iconflag[UI_MENU_SETIDX_CAMERA_TIMESTAMP1 + RFUnit];
          #else //useless
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_TIMESTAMP_ON)
                   gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=1;
               else
                   gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=0;
          #endif

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STAT_BELL_ON)
                   gRfiuUnitCntl[RFUnit].RFpara.RX_DoorTrig=1;
               else
                   gRfiuUnitCntl[RFUnit].RFpara.RX_DoorTrig=0;

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_MULTISTREAM_ON)
                   gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport=1;
               else
                   gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport=0;

               gRfiuUnitCntl[RFUnit].ProtocolSel= (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STAT_PROTCOL_SEL) >> 10;
               gRfiuUnitCntl[RFUnit].VMDSel= 1;
          #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_BATCAM_SUPPORT)
                  gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support=0;
               
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_BATCAM_PIRMODE)
                  gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
          #endif     
          
               gRfiuUnitCntl[RFUnit].RFpara.TX_SensorBrightness =(gRfiuUnitCntl[RFUnit].TX_Status >>20) & 0x0f;
               gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day =(gRfiuUnitCntl[RFUnit].TX_Status >>24) & 0x0f;
               gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night =(gRfiuUnitCntl[RFUnit].TX_Status >>28) & 0x0f;
               //-----Read TX Code Version-----//
               memcpy(gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion,(char *)pp,32);
               pp += (32/4);

               //-----Read TX ID-----//
               gRfiuUnitCntl[RFUnit].RFpara.RF_ID= *pp;
               pp ++;

           #if RF_AV_SYNCTIME_EN     
               //-----AV sync time-----//
               gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = *pp;
               pp ++;

               gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = *pp;
               pp ++;
           #endif
               //----Read TX status2----// 
               gRfiuUnitCntl[RFUnit].TX_Status2 = *pp;
               if(gRfiuUnitCntl[RFUnit].TX_Status2 != 0xa55aaa55)
               {
                   tmp=(gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STA2_BATTLV)>>0;
                   if(tmp != RF_BATCAM_TXBATSTAT_NOTREDY)
                   {
                      gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev= tmp;
                      //DEBUG_RFIU_P2("BatLev_%d=%d\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev);
                   }
                   
                   gRfiuUnitCntl[RFUnit].RFpara.TXPirFaulseTrig = (gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STA2_FAULSETRIG)>>4;
                   gRfiuUnitCntl[RFUnit].RFpara.TxAlermtOn = (gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STA2_ALERMON) >> 5;
                   
                 #if RFIU_RX_WAKEUP_TX_SCHEME
                   if(gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STA2_DOORBELL_SUPPORT)
                      gRfiuUnitCntl[RFUnit].RFpara.DoorBell_support = 1;
                   else
                      gRfiuUnitCntl[RFUnit].RFpara.DoorBell_support = 0;
                 #endif
               }
               pp ++;
               //-----------------------//
               gRfiuSyncRevFlag[RFUnit]=1;
               return RXCMD_PROTOCOL_CHECK;
           }
        #if(RF_PAIR_EN)
           else if(pRfiuPara->PktMap[2*i] & RFI_PAIR_ADDR_CHEKBIT)
           {
                pp=(unsigned int *)(pRfiuPara->TxRxOpBaseAddr+RFI_PAIR_ADDR_OFFSET*128);
				
				timerCountRead(guiRFTimerID, &t_seed);	
                // randon MAC address
                Temp_RX_MAC_Address[RFUnit] = (*pp  & 0xffff);  
                Temp_RX_MAC_Address[RFUnit] |= ((t_seed & 0x0fff)<<16);  

                timerCountRead(0, &t_seed); //Lucian: use sys-timer's count for seed
                Temp_RX_CostomerCode[RFUnit]= ((t_seed<<2) | (RFUnit & 0x03)) & 0x1fff; //Lucian: 未來可加入客戶ID:6 bit
                //DEBUG_RFIU_P(" RX NEW MAC address = 0x%x,CID=0x%x\n",Temp_RX_MAC_Address[RFUnit],Temp_RX_CostomerCode[RFUnit]);   
                DEBUG_RFIU_P2("->Pair\n");
                MACadrsSetflag[RFUnit] =1;

                return RXCMD_PROTOCOL_CHECK;
           }
        #endif 

	    #if RF_CMD_EN	
            else if(pRfiuPara->PktMap[2*i] & (RFI_TXCMD1_ADDR_CHEKBIT | RFI_TXCMD2_ADDR_CHEKBIT) )
            {
                if(pRfiuPara->PktMap[2*i] & RFI_TXCMD1_ADDR_CHEKBIT)
                   pp=(unsigned int *)(pRfiuPara->TxRxOpBaseAddr+RFI_TXCMD1_ADDR_OFFSET*128);
                else
                   pp=(unsigned int *)(pRfiuPara->TxRxOpBaseAddr+RFI_TXCMD2_ADDR_OFFSET*128);
                
                //clear cmmand map . total use 2 groups
				pRfiuPara->PktMap[0] = 0;
                pRfiuPara->PktMap[1] = 0;
				pRfiuPara->PktMap[2] = 0;
                pRfiuPara->PktMap[3] = 0;		

                gRfiuUnitCntl[RFUnit].TXCmd_Type= *pp;
                pp ++;

                if(gRfiuUnitCntl[RFUnit].TXCmd_Type & RFTXCMD_SEND_DATA)
                {
				   //DEBUG_RFIU_P2("==>TX CMD Data REV:%d \n",*pp);
                   for(i=0;i<RFIU_TXCMDDATA_MAX;i++)
                   {
                      gRfiuUnitCntl[RFUnit].TX_CMDPara[i]= *pp;
                      pp ++;
                   }

                   rfiu_TXCMD_Dec(RFUnit);
                }
            #if RFIU_RX_AUDIO_RETURN
                if(gRfiuUnitCntl[RFUnit].TXCmd_Type & RFTXCMD_SEND_AUDIORETMAP)
                {
                   for(i=0;i<RFI_AUDIO_RET_GRPNUM;i++)
                   {
                       Ret_Group = *pp;
                       pp ++;

                       Ret_map0 = *pp;
                       pp ++;

                       Ret_map1 = *pp;
                       pp ++;

                       //DEBUG_RFIU_P2("(%d %d %d)",Ret_Group,Ret_map0,Ret_map1);


                       if( (Ret_Group >= RFI_AUDIORETURN1_ADDR_OFFSET/64) && (Ret_Group <= RFI_AUDIORETURN8_ADDR_OFFSET/64) )
                       {
                          //--Updata Map--//
                          count =0;  
                          
                          gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap0 ^= Ret_map0;
                          pktmap0=gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap0;
                          if(pktmap0 != 0)
                          {
                              for(j=0;j<32;j++)
                              {
                                  if(pktmap0 & 0x01)
                                      count ++;
                                  pktmap0 >>=1;
                              }
                          }
                          
                          gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap1 ^= Ret_map1;
                          pktmap1=gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap1;
                          if(pktmap1 != 0)
                          {
                              for(j=0;j<32;j++)
                              {
                                  if(pktmap1 & 0x01)
                                      count ++;
                                  pktmap1 >>=1;
                              }
                          }

                          gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktCount=count;

                          //--Cal GroupShft--//
                          GrpDivs=0;
                          pktmap0=gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap0;
                          pktmap1=gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].PktMap1;

                          for(i=0;i<8;i++)
                          {
                                if(i<4)
                                {
                                    if((pktmap0 & 0x0ff)==0)
                                        GrpDivs ++;
                                    else
                                        break;
                                    pktmap0 >>=8;
                                }
                                else
                                {
                                    if((pktmap1 & 0x0ff)==0)
                                        GrpDivs ++;
                                    else
                                        break;
                                    pktmap1 >>=8;
                                }
                          }
                          gRfiuUnitCntl[RFUnit].TxPktMap[Ret_Group].ReadDiv=GrpDivs;
                          //=====//
                       }
                   }
                   
                   //--Cal audio return read pointer--//
                   AudioGrpWritePtr=(rfiuAudioRetRec_idx>>3) & 0x07;
                   AudioGrpReadPtr=(rfiuAudioRetRead_idx>>3) & 0x07;
                   
                   GrpWROffset = rfiuCalBufRemainCount(AudioGrpWritePtr,AudioGrpReadPtr,RFI_AUDIO_RET_BUF_NUM>>3);
                   while( ((gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].PktCount==0) || 
                           (gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].RetryCount > RX_AUDIORET_RETRYNUM)) && 
                           (GrpWROffset>0) )
                   {
                      AudioGrpReadPtr= (AudioGrpReadPtr + 1) & 0x07 ;
                      GrpWROffset = rfiuCalBufRemainCount(AudioGrpWritePtr,AudioGrpReadPtr,RFI_AUDIO_RET_BUF_NUM>>3);
                   }
                   rfiuAudioRetRead_idx=(AudioGrpReadPtr<<3)+ gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x07)].ReadDiv;
                   //DEBUG_RFIU_P2("%d ",GrpWROffset);
                }
            #endif
                return (RXCMD_AUDIOMAP_CHECK | RXCMD_NONE_CHECK);
            }
	    #endif	
        
       }
    }

    return RXCMD_NONE_CHECK;
}

int rfiuRxListenDataState(  unsigned char RFUnit,
                                  unsigned int Vitbi_en, 
                                  unsigned int RS_mode, 
                                  unsigned int Vitbi_mode,
                                  unsigned int SyncWord,
                                  unsigned int CustomerID,
                                  unsigned int TimeOut
                              )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    
    //-------//
#if( RFIU_DATA_6M_ACK_3M_SUPPORT && (RFIC_SEL==RFIC_A7196_6M) )
    if(gRfiuUnitCntl[RFUnit].RXRecvDataUse3M)
    {
        TimeOut=TimeOut/2;
    }
#elif(RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M))
    if(gRfiuUnitCntl[RFUnit].RXRecvDataUse3M)
    {
        TimeOut=TimeOut/2;
    }
#endif

    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
    //---config RFU parameter---//   
    //--Rx--//    
    pRfiuPara_Rx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Rx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Rx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Rx->PktSyncWord          =SyncWord;
    pRfiuPara_Rx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    
    pRfiuPara_Rx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Rx->PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
    pRfiuPara_Rx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
    
    pRfiuPara_Rx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Rx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Rx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Rx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    pRfiuPara_Rx->TxRxPktNum           =TimeOut;            

    pRfiuPara_Rx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Rx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Rx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
    
    //----//
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Rx(0,pRfiuPara_Rx,RFUnit);  //Rx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Rx(RFUnit & 0x01,pRfiuPara_Rx,RFUnit);  //Rx 
#else    
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Rx(RFUnit,pRfiuPara_Rx,RFUnit);  //Rx
#endif

    return 1;
  
}

int rfiuRxReplyACKState(  int RFUnit,
                                unsigned int Vitbi_en, 
                                unsigned int RS_mode, 
                                unsigned int Vitbi_mode,
                                unsigned int SyncWord,
                                unsigned int CustomerID,
                                unsigned int UserData,
                                unsigned int RX_TimeCheck,
                                unsigned int CH_chg_flag,
                                unsigned int RX_CHG_CHNext,
                                unsigned int Retry
                              )
{

    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    int i,j,count;
    u32 AudioGrpWritePtr,AudioGrpReadPtr;
    u32 AudioGrpWrDiv;
    int GrpWROffset;
    unsigned int Map0Mask,Map1Mask;
    unsigned int PktMap0,PktMap1;
    unsigned int PktMapMask[9][2]={
                                     {0x00000000,0x00000000},
                                     {0x000000ff,0x00000000},
                                     {0x0000ffff,0x00000000},
                                     {0x00ffffff,0x00000000},
                                     {0xffffffff,0x00000000},
                                     {0xffffffff,0x000000ff},
                                     {0xffffffff,0x0000ffff},
                                     {0xffffffff,0x00ffffff},   
                                     {0xffffffff,0xffffffff},
                                  };
    
    //----------------//   
    
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);

    if(Retry)
    {
       //don't change ACK contain. change time-check only
       rfiuPutTimeCheck2ACK(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,pRfiuPara_Rx,RX_TimeCheck,CH_chg_flag,RX_CHG_CHNext);

    }
    else
       rfiuPutPacketMap2ACK(RFUnit, pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,pRfiuPara_Rx,RX_TimeCheck,CH_chg_flag,RX_CHG_CHNext);
                   
          
    //---config RFU parameter---//
    //-Tx-//
    
    pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
 #if RFIU_RX_WAKEUP_TX_SCHEME
    if(gRfiuUnitCntl[RFUnit].SleepTxEn && gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode) //Lucian: force to Link-broken.
    {
       pRfiuPara_Tx->PktSyncWord          =SyncWord ^ 0x0000ffff;
       pRfiuPara_Tx->Customer_ID          =(CustomerID ^ 0x0000ffff)& RFU_CUSTOMER_ID_MASK;
    }
    else
    {
       pRfiuPara_Tx->PktSyncWord          =SyncWord;
       pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    }   
 #else
    pRfiuPara_Tx->PktSyncWord          =SyncWord;
    pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
 #endif   
    pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
    pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
    pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    
    pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

    for(i=0;i<RFI_ACK_SYNC_PKTNUM ;i++)
    {
        pRfiuPara_Tx->PktMap[2*i]      =RFI_ACK_ADDR_CHEKBIT;
        pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
        pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
    }
    pRfiuPara_Tx->TxRxPktNum           = RFI_ACK_SYNC_PKTNUM;  

    //for Audio return
#if RFIU_RX_AUDIO_RETURN
    if(gRfiuUnitCntl[RFUnit].RXCmd_AudioRetEn)
    {
       AudioGrpWritePtr=(rfiuAudioRetRec_idx>>3) & 0x07;
       AudioGrpWrDiv=rfiuAudioRetRec_idx & 0x07;
       AudioGrpReadPtr=(rfiuAudioRetRead_idx>>3) & 0x07;

       GrpWROffset = rfiuCalBufRemainCount(AudioGrpWritePtr,AudioGrpReadPtr,RFI_AUDIO_RET_BUF_NUM>>3);
       for(i=RFI_ACK_SYNC_PKTNUM;i<(RFI_ACK_SYNC_PKTNUM + RFI_AUDIO_RET_GRPNUM);i++)
       {     
           while( (gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].PktCount==0) && (GrpWROffset>0) )
           {
              AudioGrpReadPtr= (AudioGrpReadPtr + 1) & 0x07 ;
              GrpWROffset = rfiuCalBufRemainCount(AudioGrpWritePtr,AudioGrpReadPtr,RFI_AUDIO_RET_BUF_NUM>>3);
           }

           GrpWROffset = rfiuCalBufRemainCount(AudioGrpWritePtr,AudioGrpReadPtr,RFI_AUDIO_RET_BUF_NUM>>3);
           if(GrpWROffset>0)
           {
              pRfiuPara_Tx->PktMap[2*i]      =gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].PktMap0;
              pRfiuPara_Tx->PktMap[2*i+1]    =gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].PktMap1;        
              pRfiuPara_Tx->Pkt_Grp_offset[i]=(RFI_AUDIORETURN1_ADDR_OFFSET + (AudioGrpReadPtr & 0x7)*RFI_GRP_INPKTUNIT  )* RFIU_PKT_SIZE;
              pRfiuPara_Tx->TxRxPktNum += gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].PktCount;

              gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].RetryCount +=1;
              AudioGrpReadPtr = (AudioGrpReadPtr+1) & 0x07;
           }
           else if(GrpWROffset == 0)
           {
               Map0Mask=PktMapMask[AudioGrpWrDiv][0];
               Map1Mask=PktMapMask[AudioGrpWrDiv][1];
               
               pRfiuPara_Tx->PktMap[2*i]=PktMap0  =gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x07)].PktMap0 & Map0Mask;
               pRfiuPara_Tx->PktMap[2*i+1]=PktMap1=gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x07)].PktMap1 & Map1Mask;
               pRfiuPara_Tx->Pkt_Grp_offset[i]    =(RFI_AUDIORETURN1_ADDR_OFFSET + (AudioGrpReadPtr & 0x7)*RFI_GRP_INPKTUNIT  )* RFIU_PKT_SIZE;

               count=0;
               for(j=0;j<32;j++)
               {
                  if(PktMap0 & 0x01)
                      count ++;
                  PktMap0 >>=1;
                  if(PktMap0==0)
                    break;
               }
               for(j=0;j<32;j++)
               {
                  if(PktMap1 & 0x01)
                      count ++;
                  PktMap1 >>=1;
                  if(PktMap1==0)
                    break;
               }
               
               pRfiuPara_Tx->TxRxPktNum +=count;
               //gRfiuUnitCntl[RFUnit].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (AudioGrpReadPtr & 0x7)].RetryCount +=1;
               AudioGrpReadPtr = (AudioGrpReadPtr+1) & 0x07;
           }
       }
       if(pRfiuPara_Tx->TxRxPktNum > (32+RFI_ACK_SYNC_PKTNUM) )
          pRfiuPara_Tx->TxRxPktNum=32+RFI_ACK_SYNC_PKTNUM;
       //DEBUG_RFIU_P2("%d ",pRfiuPara_Tx->TxRxPktNum-RFI_ACK_SYNC_PKTNUM);
    }       
#endif
          
    pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
    pRfiuPara_Tx->UserData_H           =(UserData>>16) & RFI_USER_DATA_H_MASK;

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);      
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);
#else
    RfiuReset(RFUnit);      
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif

    return 1;
}

int rfiuRxSendWakeState(  
                                  int RFUnit,
                                  unsigned int Vitbi_en, 
                                  unsigned int RS_mode, 
                                  unsigned int Vitbi_mode,
                                  unsigned int SyncWord,
                                  unsigned int CustomerID,
                                  unsigned int UserData
                               )
{
    DEF_REGRFIU_CFG *pRfiuPara_Tx;
    DEF_REGRFIU_CFG *pRfiuPara_Rx;
    int i,j,count;
    
    //----------------//   
    pRfiuPara_Tx=&(gRfiuParm_Tx[RFUnit]);
    pRfiuPara_Rx=&(gRfiuParm_Rx[RFUnit]);
    //---config RFU parameter---//
    //-Tx-//
    pRfiuPara_Tx->Vitbi_en             =Vitbi_en;
    pRfiuPara_Tx->RsCodeSizeSel        =RS_mode;
    pRfiuPara_Tx->CovCodeRateSel       =Vitbi_mode;
    pRfiuPara_Tx->PktSyncWord          =SyncWord;
    pRfiuPara_Tx->Customer_ID          =CustomerID & RFU_CUSTOMER_ID_MASK;
    
    pRfiuPara_Tx->DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
    pRfiuPara_Tx->PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
    pRfiuPara_Tx->DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
                
    pRfiuPara_Tx->TxClkConfig          =RFI_TXCLKCONFIG;
    pRfiuPara_Tx->RxClkAdjust          =RFI_RXCLKADJUST;
    pRfiuPara_Tx->DclkConfig           =RFI_DCLKCONF;
    pRfiuPara_Tx->RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
    
    pRfiuPara_Tx->SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
    pRfiuPara_Tx->Customer_ID_ext_en   =RFI_CUSTOMER_ID_EXT_SET;
    pRfiuPara_Tx->SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;

    for(i=0;i<8 ;i++)
    {
        pRfiuPara_Tx->PktMap[2*i]      =RFI_NACK_ADDR_CHEKBIT;
        pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
        pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
    }
    pRfiuPara_Tx->TxRxPktNum           = RFI_ACK_WAKE_PKTNUM;  

    pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
    pRfiuPara_Tx->UserData_H           =(UserData>>16) & RFI_USER_DATA_H_MASK;

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx,RFUnit);  //Tx
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
    RfiuReset(RFUnit & 0x01);      
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx,RFUnit);
#else
    RfiuReset(RFUnit);      
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx,RFUnit);
#endif

    return 1;
}

#if (RFIU_TEST == 0)
void rfiu_Tx_Task_UnitX(void* pData)
{

}
#endif
void rfiu_Rx_Task_UnitX(void* pData)
{
    int RX_UsrData,RX_UsrACK;
    int RFUnit;
    int ACK_CH_sel;
    int DAT_CH_sel;
    int RxAckRetryCnt=0;
    unsigned int RX_timer,RX_TimeCheck;
    int LastRecvPacketNum=0;
    int Debugflag=0;
    int RX_RecvPktLostCnt;
    int RX_LinkBrokenCnt;
	int PairCnt;
    int RX_SYNC_ACKflag =0;
    int RX_CHG_CHflag;
    int TX_CHG_CHflag;
    int RX_CHG_CHCnt;
    int RX_CHG_CHNext;
    int RX_LinkRetry;
    int RX_SyncRetry;
    int i,j,temp;
    unsigned int CheckTime;
    
    int WifiScanCnt;
    int WifiScanRdy;
#if(RF_PAIR_EN)  
    int RX_SYC_ErrorCnt = 0 ; 
    int RX_PAIR_ACKflag =0;
    unsigned int Old_RXMAC,Old_RXCode;
#endif     
    unsigned char Prev_SeqenceNum;
    unsigned int  cpu_sr = 0;  
    int isCmd;
    unsigned char err;
    unsigned int RFTimer;    
	unsigned int t1,t2,dt,BitRate,PktCnt;
    unsigned int t3,t4;
    unsigned int t5,t6,t7;
    unsigned int t8,t9;
    int NextTx;
    
    u8 RSSI,RSSI_avg;
    u8 RSSI_Wifi;
    int RSSI_Sum,RSSI_cnt;
    u32 *RSSI_CH_Avg;//[RFI_DAT_CH_MAX];
    u32 RSSI_CH_Sum[RFI_DAT_CH_MAX];
    u32 RSSI_CH_Cnt[RFI_DAT_CH_MAX];
    int RF_CH_MaskTime[RFI_DAT_CH_MAX];
    int NextDAT_CH;
    int MinVal,MinCh;
    int RXAckCnt;   
    int MaxCh,MaxVal;

#if RF_FIXCH_OPTIM
    u32 RSSI_CheckCnt=0;    
    u32 OldFixCH;
#endif    

#if TX_FW_UPDATE_SUPPORT
    u32 TxFWSum;
    int RecvFwUpdStart;
    int RecvFwUpdDone;
    int ACKType;
    u32 Tmp;
    int UpdatePtr;
#endif
#if RFIU_RX_WAKEUP_TX_SCHEME
    unsigned int RX_Task_RunCnt=0;
    int ClearScreenInQuad;
    int MeetWakeUp=0;
    int WakupCh;
#endif

#if CDVR_SYSTEM_LOG_SUPPORT
	extern int gTXtriggerEvent[4];
#endif
    u32 ACK_MissCnt=0;
    u32 BR_SlideWin[4];
    u32 BR_Cnt=0;
#if RFIU_AUTO_UNPAIR    
    int DelCam;
#endif
    //---------------------------//
    RFUnit= (int)pData;
    RSSI_CH_Avg=rfiuRSSI_CH_Avg[RFUnit];
    
    memset(&gRfiuParm_Tx[RFUnit],0,sizeof(DEF_REGRFIU_CFG));
    memset(&gRfiuParm_Rx[RFUnit],0,sizeof(DEF_REGRFIU_CFG));
    memset(&gRfiuUnitCntl[RFUnit],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));

    gRfiuUnitCntl[RFUnit].BufReadPtr=0;
    gRfiuUnitCntl[RFUnit].BufWritePtr=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel=1;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.DAT_CH=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.SeqenceNum=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.GrpShift=0;
    gRfiuUnitCntl[RFUnit].RX_CtrlPara.GrpDivs=0;
    gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
#if(RF_CMD_EN) 
    gRfiuUnitCntl[RFUnit].RXCmd_Type=0;
    gRfiuUnitCntl[RFUnit].RXCmd_en=0;   
    gRfiuUnitCntl[RFUnit].RXCmd_Busy=0;    
#endif 

#if(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_ORIG)
    gRfiuUnitCntl[RFUnit].ProtocolSel=RFI_PROTOCOL_ORIG;
#elif(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_CE_181)
    gRfiuUnitCntl[RFUnit].ProtocolSel=RFI_PROTOCOL_CE_181;
#elif(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_FCC_247)
    gRfiuUnitCntl[RFUnit].ProtocolSel=RFI_PROTOCOL_FCC_247;
#elif(RFIU_PROTOCOL_SEL == RFI_PROTOCOL_ISOWIFI)
    gRfiuUnitCntl[RFUnit].ProtocolSel=RFI_PROTOCOL_ISOWIFI;
#endif

#if(TX_VMD_ALGSEL  == TX_VMD_ORIG )
    gRfiuUnitCntl[RFUnit].VMDSel=TX_VMD_ORIG;
#else
    gRfiuUnitCntl[RFUnit].VMDSel=TX_VMD_NEW;
#endif

    RF3M_RunCnt[RFUnit]=0;
    RX_UsrData= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].RX_CtrlPara);
    RX_UsrACK=RX_UsrData;
#if RFIU_SHARE_CTRLBUS_SUPPORT
    OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
#endif
     OS_ENTER_CRITICAL();
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(0+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(0+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(0+1,100);
     #elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel(0+1,100);
     #endif
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel( (RFUnit & 0x01)+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel( (RFUnit & 0x01)+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel( (RFUnit & 0x01)+1,100);
     #elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel( (RFUnit & 0x01)+1,100);
     #endif
#else
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(RFUnit+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(RFUnit+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(RFUnit+1,100);
     #elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel(RFUnit+1,100);
     #endif
#endif
    OS_EXIT_CRITICAL();
#if RFIU_SHARE_CTRLBUS_SUPPORT
    OSSemPost(gRfiuCtrlBusReqSem);
#endif
    if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
    {
        if(RFUnit & 0x01)
        {
           ACK_CH_sel=15;
        #if RF_FIXCH_OPTIM   
           OldFixCH=15;
        #endif
        }
        else
        {
           ACK_CH_sel=0;
        #if RF_FIXCH_OPTIM   
           OldFixCH=0;
        #endif
        }
    }
    else
       ACK_CH_sel=0;
    
    gRfiuFCC247ChUsed[RFUnit][0]=-1;
    gRfiuFCC247ChUsed[RFUnit][1]=-1;

    DAT_CH_sel=0;
    RSSI_Sum=0;
    RSSI_cnt=0;

    WifiScanCnt=0;
    WifiScanRdy=0;
    
    RX_RecvPktLostCnt=0;
    RX_LinkBrokenCnt=0;
    Prev_SeqenceNum=0xff;
    gRfiuSyncRevFlag[RFUnit]=0;
    RX_SYNC_ACKflag=0;
    RX_CHG_CHflag=0;
    TX_CHG_CHflag=0;
    RX_CHG_CHCnt=0;
    RX_CHG_CHNext=0;
    RX_LinkRetry=0;
    RX_SyncRetry=0;
    
    if(RFUnit & 0x01)
       RXAckCnt=15;
    else
       RXAckCnt=0;
   
 #if(RF_PAIR_EN)  
    Old_RXMAC = gRfiuSyncWordTable[RFUnit];
    Old_RXCode= gRfiuCustomerCode[RFUnit];
    MACadrsSetflag[RFUnit]=0;
    RX_SYC_ErrorCnt=0;
    RX_PAIR_ACKflag =0;
 #endif   
    //---Setup test environment---//
    gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
    gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;

 #if RFIU_RX_WAKEUP_TX_SCHEME
    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_INIT;
 #else
    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_LISTEN;
 #endif
    gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr = rfiuOperBuf[RFUnit];
    gRfiuParm_Rx[RFUnit].TxRxOpBaseAddr = rfiuOperBuf[RFUnit];

    for(i=0;i<RFI_DAT_CH_MAX;i++)
        RF_CH_MaskTime[i]=0;


    BitRate=0;
    PktCnt=0;
	PairCnt=0;
    for(i=0;i<RFI_DAT_CH_MAX;i++)
    {
       RSSI_CH_Sum[i]=0;
       RSSI_CH_Cnt[i]=0;
       RSSI_CH_Avg[i]=0;
    }
    
    timerCountRead(guiRFTimerID, &t1);
    timerCountRead(guiRFTimerID, &t3);
    timerCountRead(guiRFTimerID, &t5);
    t9=t8=t7=t5;
    gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_LINK_BROKEN;
#if RFIU_RX_WAKEUP_TX_SCHEME    
    rfiuBatCam_LiveMaxTime[RFUnit]=0;
#endif
    //gRfiuUnitCntl[RFUnit].RX_Task_Stop=0;

    while(1)
	{
	    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop)
	    {
	        t9=t8;
            t7=t6;
	        t5=t6;
	        t3=t4;
            t1=t2;
            PktCnt=0;
            DEBUG_RFIU_P2("$");
            OSTimeDly(1);
            continue;
        }
    #if 1
        if(AmicReg_RWen1)
        {
        #if RFIU_SHARE_CTRLBUS_SUPPORT
           OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
        #endif
           OS_ENTER_CRITICAL();
           if(AmicReg_RWen1 == 1)
           {
           #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
              A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
           #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
              A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
           #elif(RFIC_SEL==RFIC_NONE_5M)
           
           #endif
           }
           else
           {
           #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
              AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
           #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
              AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
           #elif(RFIC_SEL==RFIC_NONE_5M)
           
           #endif
              DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
           }

           AmicReg_RWen1=0;
           OS_EXIT_CRITICAL();
        #if RFIU_SHARE_CTRLBUS_SUPPORT
           OSSemPost(gRfiuCtrlBusReqSem);
        #endif
        }    
        
        if(AmicReg_RWen2)
        {
        #if RFIU_SHARE_CTRLBUS_SUPPORT
           OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
        #endif
           OS_ENTER_CRITICAL();
           if(AmicReg_RWen2 == 1)
           {
           #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
              A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
           #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
              A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
           #elif(RFIC_SEL==RFIC_NONE_5M)
           
           #endif
           
           }
           else
           {
           #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
              AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
           #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
              AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
           #elif(RFIC_SEL==RFIC_NONE_5M)
           
           #endif
              DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
           }
           AmicReg_RWen2=0;
           OS_EXIT_CRITICAL();
        #if RFIU_SHARE_CTRLBUS_SUPPORT
           OSSemPost(gRfiuCtrlBusReqSem);
        #endif
        }   
    #endif
        
	    switch(gRfiuUnitCntl[RFUnit].OpMode)
	    {
		case RFIU_RX_MODE:
            rfiuDecUsrData(RX_UsrACK,&gRfiuUnitCntl[RFUnit].RX_CtrlPara);            
         #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
         #endif
            OS_ENTER_CRITICAL();
        #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)    
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
        #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
        #else
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
        #endif
            OS_EXIT_CRITICAL();
        #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPost(gRfiuCtrlBusReqSem);
        #endif
            if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
            {
                if(gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                {   //listen 45 ms
                    rfiuRxListenDataState( RFUnit,
                                           gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en], 
                                           gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel], 
                                           gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel],
                                           gRfiuSyncWordTable[RFUnit],
                                           gRfiuCustomerCode[RFUnit],
                                           RFI_RX_WAIT_TIME*2+RFI_ACK_SYNC_PKTNUM
                                         );
                }
                else
                {
                    rfiuRxListenDataState( RFUnit,
                                           gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en], 
                                           gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel], 
                                           gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel],
                                           gRfiuSyncWordTable[RFUnit],
                                           gRfiuCustomerCode[RFUnit],
                                           RFI_RX_WAIT_TIME*2+RFI_ACK_SYNC_PKTNUM
                                         );  
                }
            }
            else
            {
                    if(gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK)
                    {
                        rfiuRxListenDataState( RFUnit,
                                               gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en], 
                                               gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel], 
                                               gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel],
                                               gRfiuSyncWordTable[RFUnit],
                                               gRfiuCustomerCode[RFUnit],
                                               RFI_RX_WAIT_TIME/2
                                             );  
                    }
                    else
                    {
                        rfiuRxListenDataState( RFUnit,
                                               gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en], 
                                               gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel], 
                                               gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel],
                                               gRfiuSyncWordTable[RFUnit],
                                               gRfiuCustomerCode[RFUnit],
                                               RFI_RX_WAIT_TIME
                                             );  
                    }    
            }
       #if RFI_SELF_TEST_TXRX_PROTOCOL
           #if RFI_TEST_WRAP_OnPROTOCOL
            if(gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_RETRY_ACK)
            {
               DEBUG_RFIU_P2("-->ReAck\n");
               Debugflag=0;
            }
            else
            {
               Debugflag=1;
            }

           #else
            //DEBUG_RFIU_P2("\n<===RX: AckCH=%d,DatCH=%d,Last Recv=%d\n",ACK_CH_sel,DAT_CH_sel,LastRecvPacketNum);
           #endif
       #else
            if(gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_RETRY_ACK)
            {
            #if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2)
               DEBUG_RFIU_P("%d",ACK_CH_sel);
            #else
               DEBUG_RFIU_P("%d",RFUnit);
            #endif
               Debugflag=0;
            }
            else
            {
               if(Debugflag==0)
               {
               #if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2)
                  DEBUG_RFIU_P(" ");
               #endif
               }
               else
               {
                  //DEBUG_RFIU_P2("<=ACK%d\n",RFUnit);
               }
               Debugflag=1;
            }
       #endif


		    //=====Cal Bit Rate=====//
            timerCountRead(guiRFTimerID, &t2);
            if(t1 >= t2)
              dt=t1-t2;
            else
              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

         #if( ( (RFIU_DATA_6M_ACK_3M_SUPPORT || RFIU_DATA_6M_ACK_4M_SUPPORT) && (RFIC_SEL==RFIC_A7196_6M) ) ||\
               ( RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M) ) )
            if(dt > 10000)
         #else
            if(dt > 30000)
         #endif
            {
              BitRate = PktCnt*128*8/10/dt; //Unit: 100Kbps
              //Lucian: 做 4 sec平均, 讓天線符號不至過於震盪.
         #if( ( (RFIU_DATA_6M_ACK_3M_SUPPORT || RFIU_DATA_6M_ACK_4M_SUPPORT) && (RFIC_SEL==RFIC_A7196_6M) ) ||\
               ( RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M) ) )
              BR_SlideWin[BR_Cnt & 0x03]=BitRate;
              if(BR_Cnt<4)
              {
                 gRfiuUnitCntl[RFUnit].BitRate=BitRate;
              }   
              else
              {
                 gRfiuUnitCntl[RFUnit].BitRate=(BR_SlideWin[0]+BR_SlideWin[1]+BR_SlideWin[2]+BR_SlideWin[3])/4;
              }
              BR_Cnt ++;
         #else      
              gRfiuUnitCntl[RFUnit].BitRate=BitRate;
         #endif
         
         #if( ( (RFIU_DATA_6M_ACK_3M_SUPPORT || RFIU_DATA_6M_ACK_4M_SUPPORT) && (RFIC_SEL==RFIC_A7196_6M) ) ||\
               ( RFIU_DATA_4M_ACK_2M_SUPPORT && (RFIC_SEL==RFIC_A7130_4M) ) )
              rfiuRXJdgDataSwich_3M(RFUnit,BitRate,DAT_CH_sel);
         #endif 

         #if ( (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR8202_AN_KLF08W) )
              DEBUG_RFIU_P2("RF-%d BitRate=%d,%d,%d\n",RFUnit,gRfiuUnitCntl[RFUnit].BitRate,ACK_MissCnt,gRfiuUnitCntl[RFUnit].RXRecvDataUse3M);
         #endif
              ACK_MissCnt=0;

              if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysRFRxInMainCHsel==RFUnit) )
                 sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, gRfiuUnitCntl[RFUnit].BitRate);       
              else if((sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) && (sysRFRxInMainCHsel==RFUnit) )
                 sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, gRfiuUnitCntl[RFUnit].BitRate);
            #if RFIU_RX_WAKEUP_TX_SCHEME
              if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
              {
                  rfiuBatCam_LiveMaxTime[RFUnit] += dt;
              }
              else
                  rfiuBatCam_LiveMaxTime[RFUnit]=0;
            #endif
              t1=t2;
              PktCnt=0;
            }
    		
            //==================//
        #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
            RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //R
        #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //Rx
          #if 1
            for(i= ( (RFUnit+1)& 0x01);i<4;i+=2)
            {
                if(gRfiuFCC247ChUsed[i][0] != -1)
                {
                    if( ( (gRfiuFCC247ChUsed[i][0]+1) == WifiScanCnt) || ( (gRfiuFCC247ChUsed[i][0]+0) == WifiScanCnt) || ( (gRfiuFCC247ChUsed[i][0]-1) == WifiScanCnt) )
                    {
                         RSSI_Wifi=RFIU_RSSI_THR;
                    }
                }
            }
           #endif

        #else
            RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //Rx
        #endif


        #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
            if(RSSI >= RX_ACK_RSSI_THR)
            {
               RSSI_Sum += RSSI;
               RSSI_cnt ++;
            }
        #else
            RSSI_Sum += RSSI;
            RSSI_cnt ++;
        #endif
            if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
            {
            #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1))
               if(RSSI_Wifi >= RX_ACK_RSSI_THR)
               {
                  RSSI_CH_Sum[WifiScanCnt] += RSSI_Wifi;
                  RSSI_CH_Cnt[WifiScanCnt] ++;
               }
            #else
               RSSI_CH_Sum[WifiScanCnt] += RSSI_Wifi;
               RSSI_CH_Cnt[WifiScanCnt] ++;
            #endif
            }
            else
            {
               RSSI_CH_Sum[DAT_CH_sel] += RSSI;
               RSSI_CH_Cnt[DAT_CH_sel] ++;
            }
            WifiScanCnt = (WifiScanCnt+1) & 0x0f;
            
			PktCnt +=gRfiuParm_Rx[RFUnit].TxRxPktNum;

            //----------沒收到 packet------//
            if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("<---No Data Packet received!!AckCH=%d,DatCH=%d\n",ACK_CH_sel,DAT_CH_sel);
               RX_UsrData=RX_UsrACK;
               ACK_MissCnt ++;

           #if RFIU_RX_WAKEUP_TX_SCHEME
               if(RX_SYNC_ACKflag && (gRfiuSyncRevFlag[RFUnit] == 0) )
               {
                   if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                      RX_LinkRetry=1;
                   else
                      RX_LinkRetry=0;
               }    
           #endif
           
			   if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
			   {
               }
               else
               {
                #if RFI_SELF_TEST_TXRX_PROTOCOL
                   if(RX_RecvPktLostCnt >= 5)
                   {
                      DAT_CH_sel = (DAT_CH_sel+1) & 0x0f; 
                      RX_RecvPktLostCnt=0;
                   }
                #else
                   if(RX_RecvPktLostCnt >= (RFI_MAX_BURST_NUM/RFI_RX_WAIT_TIME-1))
                   {
                      DAT_CH_sel = (DAT_CH_sel+1) & 0x0f; 
                      RX_RecvPktLostCnt=0;
                   }
                #endif 
               }
               
               timerCountRead(guiRFTimerID, &t4);
               if(t3 >= t4)
                  dt=t3-t4;
               else
                  dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;
               
               if(dt > 20000)
               {
                   if(gRfiu_Op_Sta[RFUnit]==RFIU_OP_INIT)
                   {
                      DEBUG_RFIU_P("->RX%d is Paring!\n",RFUnit);
                      t3=t4;
                      t5=t6;
                      t7=t6;
                      t9=t8;
                   }
                   else
                   {
                       if(RX_PAIR_ACKflag == 0 )   //enter pair ack 
                       {
                        #if RFIU_RX_WAKEUP_TX_SCHEME 

                        #else
                           OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                        #endif
                           if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                           {
                           #if RFIU_RX_WAKEUP_TX_SCHEME 
                               if(gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_BROKEN)
                                  ClearScreenInQuad=1;
                               else
                                  ClearScreenInQuad=0;
                           #else
                               if(gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_BROKEN)
                               {
                               #if RFRX_HALF_MODE_SUPPORT 
                                  if(rfiuRX_CamOnOff_Num <= 2)
                                  {
                                     temp=rfiuRX_CamOnOff_Sta;
                                     for(i=0;i<MAX_RFIU_UNIT;i++)
                                     {
                                         if(temp & 0x01)
                                            break;
                                         temp = temp>>1;
                                     }
                                     if( i == RFUnit)
                                        sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 4);
                                     else
                                        sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 5); 
                                  }
                                  else
                                     sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                               #else
                                  sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                               #endif
                               }
                           #endif    
                           }
                       }
                     #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                       Record_flag[RFUnit] = 1;
                     #endif
                       gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_BROKEN;
                       gRfiuUnitCntl[RFUnit].RXCmd_en=0;
                       gRfiuUnitCntl[RFUnit].RXCmd_Busy=0; 
                       RX_LinkBrokenCnt=0;
                       t3=t4;
                       t5=t6;
                       t7=t6;
                       //t9=t8;

                       if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                       {  //---定頻---
                          //DEBUG_RFIU_P2("WifiScanRdy=%d\n",WifiScanRdy);
                       #if RFIU_RX_WAKEUP_TX_SCHEME
                          if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                          {
                             if(RFUnit & 0x01)
                                ACK_CH_sel =RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 
                             else
                                ACK_CH_sel =RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                             DAT_CH_sel =ACK_CH_sel;
                             //DAT_CH_sel =rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                          }
                          else
                          {
                             if(RFUnit & 0x01)
                                ACK_CH_sel =RFI_BATCAM_SLEEP_SYNC_CH_HIGH;
                             else
                                ACK_CH_sel =RFI_BATCAM_SLEEP_SYNC_CH_LOW;
                             DAT_CH_sel =ACK_CH_sel;
                          }
                       #else   
                          if(WifiScanRdy)
                          {
                              ACK_CH_sel= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                          }
                          DAT_CH_sel = ACK_CH_sel;
                       #endif
                          gRfiuFCC247ChUsed[RFUnit][0]=-1;
                          gRfiuFCC247ChUsed[RFUnit][1]=-1;
                          RSSI_Sum=0;
                          RSSI_cnt=0;
                       }                       
                       gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=1;  //Lucian: 斷線時,強迫用3M
                       RF3M_RunCnt[RFUnit]=RFKEEP_3M_TIME-2;
                       
                   #if RFIU_RX_WAKEUP_TX_SCHEME
                       if(gRfiuUnitCntl[RFUnit].SleepTxEn == 1) //if receive Sleep command
                       {
                           if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                           {
                               rfiuRX_CamSleep_Sta |= (0x01 << RFUnit);
                               gRfiuUnitCntl[RFUnit].SleepTxEn=0;
                               ClearScreenInQuad=0;
                               DEBUG_RFIU_P2("---TX-%d Enter Sleep Mode---\n",RFUnit);
                           }
                           else
                           {
                               gRfiuUnitCntl[RFUnit].SleepTxEn=0;
                               DEBUG_RFIU_P2("---TX-%d Not Support Sleep Mode---\n",RFUnit);
                           }
                       }
                       else
                       {
                           if( gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
                           {
                               rfiuRX_CamSleep_Sta |= (0x01 << RFUnit);
                               ClearScreenInQuad=0;
                           }
                       }
                       if(RX_PAIR_ACKflag == 0 )
                           OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                       
                       if(ClearScreenInQuad)
                       {
                       #if RFRX_HALF_MODE_SUPPORT 
                          if(rfiuRX_CamOnOff_Num <= 2)
                          {
                             temp=rfiuRX_CamOnOff_Sta;
                             for(i=0;i<MAX_RFIU_UNIT;i++)
                             {
                                 if(temp & 0x01)
                                    break;
                                 temp = temp>>1;
                             }
                             if( i == RFUnit)
                                sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 4);
                             else
                                sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 5); 
                          }
                          else
                             sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #else
                          sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #endif
                       }

                       rfiuBatCam_LiveMaxTime[RFUnit]=0;
                       if(rfiuRX_CamSleep_Sta & (0x01 << RFUnit))
                          DEBUG_RFIU_P2("->RX%d Enter Sleep Polling!0x%x,%d,%d,%d\n",RFUnit,gRfiuSyncWordTable[RFUnit],DAT_CH_sel,gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support,gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev);
                       else
                          DEBUG_RFIU_P2("->RX%d Link Broken!0x%x,%d,%d,\n",RFUnit,gRfiuSyncWordTable[RFUnit],DAT_CH_sel,gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support,gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev);
                   #else
                       DEBUG_RFIU_P2("->RX%d Link Broken!0x%x,%d\n",RFUnit,gRfiuSyncWordTable[RFUnit],DAT_CH_sel);
                   #endif
                   }
               }
            
            #if(RF_PAIR_EN)   
               if(RX_PAIR_ACKflag >0 )   //enter pair ack 
               {  
                   RX_SYC_ErrorCnt++;
                   if(RX_SYC_ErrorCnt >=16)
                   {
                     PairCnt ++;
    				 if(PairCnt >100)
    				 	PairCnt=0;
                     gRfiuSyncWordTable[RFUnit] = RFI_PAIR_SYNCWORD;
                     gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                     RX_PAIR_ACKflag=0;
                     RX_SYC_ErrorCnt =0;
                  #if RFIU_SHARE_CTRLBUS_SUPPORT
                     OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
                  #endif
                     OS_ENTER_CRITICAL();
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )      
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif(RFIC_SEL==RFIC_NONE_5M)
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #endif
                     OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                     OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                     DEBUG_RFIU_P2("-->RX Pair Retry\n");
    				 //sprintf(sysBackOsdString,"Paring%d..#%d  ",RFUnit+1,PairCnt);
    			     //sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                   }  
               }
            #endif 
            
               RX_RecvPktLostCnt ++;
               RX_LinkBrokenCnt ++;
               RX_TimeCheck=0xffffffff;
               isCmd=RXCMD_NONE_CHECK;
            }
            else //----------有收到 packet------//
            {  
               RX_RecvPktLostCnt=0;
               RX_LinkBrokenCnt=0;
               timerCountRead(guiRFTimerID, &t3);
               gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_LISTEN;
               DEBUG_RFIU_P("<---%d Packet received. AckCH=%d,DatCH=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum,ACK_CH_sel,DAT_CH_sel);
               RX_UsrData= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
            #if RFIU_FORCEUSE_ECC_RS12 
               #if(RFIC_SEL == RFIC_A7196_6M)
                  RX_UsrData |= (0x03 << RFIU_USRDATA_FEC_SHFT); //Lucian: force to use RS12 code, @2018/10/18
               #endif
            #endif   
               rfiuDecUsrData(RX_UsrData,&gRfiuUnitCntl[RFUnit].RX_CtrlPara_next);
               RxAckRetryCnt=0;


               if(gRfiuUnitCntl[RFUnit].RXCmd_Busy && gRfiuUnitCntl[RFUnit].RXCmd_en)
               {
                  OS_ENTER_CRITICAL();
                  gRfiuUnitCntl[RFUnit].RXCmd_en=0;
                  gRfiuUnitCntl[RFUnit].RXCmd_Busy=0;
                  OS_EXIT_CRITICAL();
               }
              //---------Check Event Trigger-------//   
              #if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
               if (1)   /*RDI 系列不做判斷*/
              #else
               if(gRfiuUnitCntl[RFUnit].RFpara.MD_en || gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
              #endif
               {
                   gRfiuUnitCntl[RFUnit].RFpara.MD_Trig = (RX_UsrData >> RFIU_USRDATA_MDDIF_SHFT) & RFIU_USRDATA_MDDIF_MASK;
                   GMotionTrigger[RFUnit+MULTI_CHANNEL_LOCAL_MAX] |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig;
               #if CDVR_SYSTEM_LOG_SUPPORT
				   gTXtriggerEvent[RFUnit] |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig;
			   #endif
               #if MULTI_CHANNEL_VIDEO_REC
                   VideoClipOption[RFUnit + MULTI_CHANNEL_LOCAL_MAX].MD_Diff   |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig; // for motion detect prerecord
               #endif
                   gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig;
               }
               else
                   gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=0;
              //-----------------------------------//
               if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
               {
                   RX_TimeCheck=gRfiuTimer[RFUnit];
               }
               else
               {
                   DAT_CH_sel = gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.DAT_CH;
                   if(RFUnit & 0x01)
                      RX_TimeCheck=gRfiuTimer[RFUnit]+RF1_ACKTIMESHIFT;
                   else
                      RX_TimeCheck=gRfiuTimer[RFUnit];
               }
               
               if(RX_UsrData & RFIU_USRDATA_CMD_CHEK)
               {
                   isCmd=rfiuProcessCmdPkt(RFUnit);
                   if(isCmd & RXCMD_PROTOCOL_CHECK)
                     Prev_SeqenceNum=0xff;
               }
               else
                  isCmd=RXCMD_NONE_CHECK;
             
               if(gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_BROKEN)
               {                  
                  OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                  gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                 #if INSERT_NOSIGNAL_FRAME
                  if(Rx_status[RFUnit] == 0)
                  {
                      Record_flag[RFUnit] = 0;
                      Rx_status[RFUnit]=1;
                  }
                 #endif
               }

			 #if(RF_PAIR_EN) 
               if(RX_PAIR_ACKflag >0 )   //enter pair ack 
               {
                  if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
                  {
					 uiRFID[RFUnit]=(unsigned int)gRfiuSyncWordTable[RFUnit];
                     uiRFCODE[RFUnit]=(unsigned int)gRfiuCustomerCode[RFUnit];
                  #if RFIU_SHARE_CTRLBUS_SUPPORT
                     OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
                  #endif
                     OS_ENTER_CRITICAL(); 
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )      
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif(RFIC_SEL==RFIC_NONE_5M)
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                     RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                     RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #endif
                     OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                     OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                     gRfiuUnitCntl[RFUnit].RX_Pair_Done=1;
                #if RFIU_AUTO_UNPAIR
                    rfiuRX_CamPair_Sta = rfiuRX_CamPair_Sta | (0x01 << RFUnit);
                    DelCam = rfiuCheckTX_IDSame(RFUnit);
                    if(DelCam != -1)
                    {
                        rfiuClearRFSyncWord(DelCam);
                        iconflag[UI_MENU_SETIDX_CH1_ON+DelCam] = 0;
                        sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING, 0);
                        DEBUG_RFIU_P2("===> DelCam = %d,0x%x,0x%x,%d,%d,%d,%d\n",DelCam,rfiuRX_CamPair_Sta,rfiuRX_CamOnOff_Sta,
                                        iconflag[UI_MENU_SETIDX_CH1_ON+0],
                                        iconflag[UI_MENU_SETIDX_CH1_ON+1],
                                        iconflag[UI_MENU_SETIDX_CH1_ON+2],
                                        iconflag[UI_MENU_SETIDX_CH1_ON+3]
                                      );

                    }
                #endif
                     spiWriteRF(0);
					 DEBUG_RFIU_P2("-->RF-ID Saving:0x%x,0x%x\n",gRfiuSyncWordTable[RFUnit],gRfiuCustomerCode[RFUnit]);
                     OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_SET, &err);
                     OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_PAIR_OK<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                     gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_PAIR_OK;

                     RX_PAIR_ACKflag=0;
                     RX_SYC_ErrorCnt=0; 
                  }
               }
			 #endif

               //-------------------- Receive Sync Packet and do something ---------------//
               if(RX_SYNC_ACKflag && (gRfiuSyncRevFlag[RFUnit] == 0) )
               {
               #if RFIU_RX_WAKEUP_TX_SCHEME
                   if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                   {
                       ACK_CH_sel=DAT_CH_sel;
                   }
               #endif
                   RX_SyncRetry=0;
               #if RFIU_SUPPORT  
                 #if RFIU_FORCE_TXSTARTREBOOT
                   if(gRfiuUnitCntl[RFUnit].RFpara.RxForceTxFirstReboot==1)
                   {
                      DEBUG_RFIU_P2("--SYS_BACKRF_RFI_TX_SETREBOOT--\n");
                      sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETREBOOT, RFUnit);
                   }   
                 #endif
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_SETOPMODE, RFUnit);
                 #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)  || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )

                 #else
                   //sysback_RF_SetEvt(SYS_BACKRF_RFI_SETTIME2TX, RFUnit); //Update TX time
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_UPDATETXOTHERSPARA, RFUnit); //Update TX other para
                 #endif
                 
                 #if(HW_BOARD_OPTION == MR9200_RX_RDI_UDR777 && (PROJ_OPT == 10 || PROJ_OPT == 11))
                 //sync VMD table apply day sensitivity level at night is improper. this two TX can't update, so fix bug in RX, 20180117
                 //someday, add day/night VMD table and sync by ui_task
                   if (!strcmp((const char*)gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion, TxMdSensTableExistCL894))
                       DEBUG_RFIU("%s don't send table, to avoid reapply day level at night\n",gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
                   else if (!strcmp((const char*)gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion, TxMdSensTableExistCA814))
                       DEBUG_RFIU("%s don't send table, to avoid reapply day level at night\n",gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
                   else                        
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_SENDTXMDSENS, RFUnit); //Lucian: Send TX MD sensitivity definition 
                 #elif(SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)   
                 
                 #else
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_SENDTXMDSENS, RFUnit); //Lucian: Send TX MD sensitivity definition
                 #endif

                 #if RFIU_VOX_SUPPORT
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_VOXCFG, RFUnit); 
                 #endif 
                 #if 0
                   if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                   {
                      if( gRfiuUnitCntl[RFUnit].RFpara.PIR_en != uiCheckPIRSchedule(RFUnit) )
                         sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SETPIRCFG, RFUnit); 
                   }   
                 #endif
                  
                   OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                   gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                   {
                   #if RFIU_RX_WAKEUP_TX_SCHEME
                       if( gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support == 0 )
                       {
                       #if RFRX_HALF_MODE_SUPPORT 
                           if(rfiuRX_CamOnOff_Num <= 2)
                           {
                               temp=rfiuRX_CamOnOff_Sta;
                               for(i=0;i<MAX_RFIU_UNIT;i++)
                               {
                                   if(temp & 0x01)
                                       break;
                                   temp = temp>>1;
                               }
                               if( i == RFUnit)
                                   sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 4);
                               else
                                   sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 5); 
                           }
                           else
                              sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #else
                           sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #endif
                       }
                   #else
                       #if RFRX_HALF_MODE_SUPPORT 
                          if(rfiuRX_CamOnOff_Num <= 2)
                          {
                             temp=rfiuRX_CamOnOff_Sta;
                             for(i=0;i<MAX_RFIU_UNIT;i++)
                             {
                                 if(temp & 0x01)
                                    break;
                                 temp = temp>>1;
                             }
                             if( i == RFUnit)
                                sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 4);
                             else
                                sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 5); 
                          }
                          else
                             sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #else
                          sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                       #endif
                   #endif
                   }
                   
                   #if RFIU_FORCE_TXSTARTREBOOT
                   if(gRfiuUnitCntl[RFUnit].RFpara.RxForceTxFirstReboot==1)
                   {
                      gRfiuUnitCntl[RFUnit].RFpara.RxForceTxFirstReboot=0;
                      sysSetEvt(SYS_EVT_FORCERESYNC_RF, RFUnit);
                   }  
                   else
                      sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_CH_RESTART, RFUnit);
                      
                   #else
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_CH_RESTART, RFUnit);
                   #endif
               #endif
    		 
    		   #if(RF_PAIR_EN) 
    		       //sprintf(sysBackOsdString,"                  ");
    			   //sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
    	       #endif	
                   RX_SYNC_ACKflag=0; 
                   RSSI_Sum=0;
                   RSSI_cnt=0;
               #if RF_AV_SYNCTIME_EN
                   DEBUG_RFIU_P2("-->Recv SYNC:%d:%d,%d,0x%x,%d,0x%x,%d,%d,0x%x\n",
                                 RFUnit,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight,
                                 gRfiuUnitCntl[RFUnit].TX_Status,gRfiuUnitCntl[RFUnit].ProtocolSel,gRfiuUnitCntl[RFUnit].RFpara.RF_ID,
                                 gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime,gRfiuUnitCntl[RFUnit].TX_Status2); 
               #else
                   DEBUG_RFIU_P2("-->Recv SYNC:%d:%d,%d,0x%x,%d,0x%x,0x%x\n",
                                 RFUnit,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight,
                                 gRfiuUnitCntl[RFUnit].TX_Status,gRfiuUnitCntl[RFUnit].ProtocolSel,gRfiuUnitCntl[RFUnit].RFpara.RF_ID,gRfiuUnitCntl[RFUnit].TX_Status2); 
               #endif
               }

             #if 0
               if(gRfiuParm_Rx[RFUnit].CID_ErrCnt != 0)
               {
                  DEBUG_RFIU_P2("-->CID error: %d\n",gRfiuParm_Rx[RFUnit].CID_ErrCnt);
               }
             #endif
             
               if(Prev_SeqenceNum != gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.SeqenceNum)
               {
                   if( ((Prev_SeqenceNum+1) & 0x03) == gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.SeqenceNum)
                   {
                       OS_ENTER_CRITICAL();
                       if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
                       {
                          gRfiuUnitCntl[RFUnit].BufWritePtr  = (gRfiuUnitCntl[RFUnit].BufWritePtr + gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpShift) & RFI_BUF_SIZE_MASK ;
                          gRfiuUnitCntl[RFUnit].WritePtr_Divs= gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpDivs;
                       }
                       OS_EXIT_CRITICAL();
                   }
                   else
                   {
                       if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
                          DEBUG_RFIU_P2("->SeqNum Err:%d\n",RFUnit);
                   }
               }
               else
               {
               #if DEBUG_WRPTR_WSHFT
               #endif
                   DEBUG_RFIU_P2("\n---->Received Repeat Data Packet:%d!!\n",Prev_SeqenceNum);
               #if DEBUG_WRPTR_WSHFT
               #endif
               }
               Prev_SeqenceNum=gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.SeqenceNum;
               
            }
            //----------------------------------------------------------//
            LastRecvPacketNum=gRfiuParm_Rx[RFUnit].TxRxPktNum;
            
          #if RFI_TEST_2x_RX_PROTOCOL_B1
            if( (isCmd & RXCMD_PROTOCOL_CHECK)== 0)
            {
            #if RFIU_RX_SHOW_ONLY
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   if(gRfiuTxSwCnt[0] == (RFUnit & 0x01) )
                   {
                      NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                      if(NextTx < 0)
                          DEBUG_RFIU_P2("Error! Find invalid TX\n");
                      else
                          OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                          //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x01) ) , OS_FLAG_SET, &err);
                   }
               }
               else
               {
                  NextTx=sysRFRxInMainCHsel;
                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #else
               if(gRfiuTxSwCnt[0] == (RFUnit & 0x01) )
               {
                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                  if(NextTx < 0)
                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                  else
                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                      //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x01) ) , OS_FLAG_SET, &err);
               }
            #endif
               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
               if (err != OS_NO_ERR)
               {
                	DEBUG_RFIU_P2("-->Wait RFIU_%d Timeout.\n",RFUnit);
               }
            #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
               {
                    if( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) )
                    {
                        if(gRfiuUnitCntl[RFUnit].WakeUpTxEn ==0)
                        {
                            while( (RX_Task_RunCnt & 0x03) != 0)
                            {
                               RX_Task_RunCnt ++;
                               OS_ENTER_CRITICAL();
                               gRfiuTxSwCnt[0] = RFUnit & 0x01;
                               OS_EXIT_CRITICAL();
                               if(gRfiuTxSwCnt[0] == (RFUnit & 0x01) )
                               {
                                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                                  if(NextTx < 0)
                                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                  else
                                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                                      //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x01) ) , OS_FLAG_SET, &err);
                               }
                               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
                            }
                        }    
                    }
               }
            #endif                    
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[0] = RFUnit & 0x01;
               OS_EXIT_CRITICAL();
            }
            
          #elif RFI_TEST_4TX_2RX_PROTOCOL
            if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
            {
            #if RFIU_RX_SHOW_ONLY
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x01) )
                   {
                      NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                      if(NextTx < 0)
                          DEBUG_RFIU_P2("Error! Find invalid TX\n");
                      else
                          OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                      //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
                   }
               }
               else
               {
                  NextTx=sysRFRxInMainCHsel;
                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #else
               if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x01) )
               {
                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                  if(NextTx < 0)
                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                  else
                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
               }
            #endif
               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);

               if (err != OS_NO_ERR)
               {
                	DEBUG_RFIU_P2("-->Wait RFIU_%d Timeout.\n",RFUnit);
               }
            #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
               {
                    if( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) )
                    {
                       if(gRfiuUnitCntl[RFUnit].WakeUpTxEn ==0)
                       {
                            while( (RX_Task_RunCnt & 0x03) != 0)
                            {
                               RX_Task_RunCnt ++;
                               OS_ENTER_CRITICAL();
                               gRfiuTxSwCnt[RFUnit & 0x01] = ((RFUnit>>1) & 0x01);
                               OS_EXIT_CRITICAL();
                               if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x01) )
                               {
                                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                                  if(NextTx < 0)
                                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                  else
                                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
                               }
                               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
                            }
                        }    
                    }

               }
            #endif     
               
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[RFUnit & 0x01] = ((RFUnit>>1) & 0x01);
               OS_EXIT_CRITICAL();
            }

          #elif RFI_TEST_8TX_2RX_PROTOCOL
            if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
            {
            #if RFIU_RX_SHOW_ONLY
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x03) )
                   {
                      NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                      if(NextTx < 0)
                          DEBUG_RFIU_P2("Error! Find invalid TX\n");
                      else
                          OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                      //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
                   }
               }
               else
               {
                  NextTx=sysRFRxInMainCHsel;
                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #else
               if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x03) )
               {
                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                  if(NextTx < 0)
                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                  else
                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
               }
            #endif
               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);

               if (err != OS_NO_ERR)
               {
                	DEBUG_RFIU_P2("-->Wait RFIU_%d Timeout.\n",RFUnit);
               }
            #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
               {
                    if( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) )
                    {
                        if(gRfiuUnitCntl[RFUnit].WakeUpTxEn ==0)
                        {
                            while( (RX_Task_RunCnt & 0x03) != 0)
                            {
                               RX_Task_RunCnt ++;
                               OS_ENTER_CRITICAL();
                               gRfiuTxSwCnt[RFUnit & 0x01] = ((RFUnit>>1) & 0x03);
                               OS_EXIT_CRITICAL();
                               if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x03) )
                               {
                                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                                  if(NextTx < 0)
                                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                  else
                                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+2) & 0x03) ) , OS_FLAG_SET, &err);
                               }
                               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
                            }
                        }   
                    }

               }
            #endif     
               
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[RFUnit & 0x01] = ((RFUnit>>1) & 0x03);
               OS_EXIT_CRITICAL();
            }

          
          #elif RFI_TEST_4x_RX_PROTOCOL_B1
            if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
            {
            #if RFIU_RX_SHOW_ONLY
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   if( gRfiuTxSwCnt[0] == (RFUnit & 0x03) )
                   {
                      NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                      if(NextTx < 0)
                          DEBUG_RFIU_P2("Error! Find invalid TX\n");
                      else
                          OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                      //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x03) ) , OS_FLAG_SET, &err);
                   }
               }
               else
               {
                  NextTx=sysRFRxInMainCHsel;
                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #else
               if( gRfiuTxSwCnt[0] == (RFUnit & 0x03) )
               {
                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                  if(NextTx < 0)
                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                  else
                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x03) ) , OS_FLAG_SET, &err);
               }
            #endif
               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);

               if (err != OS_NO_ERR)
               {
                	DEBUG_RFIU_P2("-->Wait RFIU_%d Timeout.\n",RFUnit);
               }
            #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
               {
                    if( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) )
                    {
                        if(gRfiuUnitCntl[RFUnit].WakeUpTxEn ==0)
                        {
                            while( (RX_Task_RunCnt & 0x03) != 0)
                            {
                               RX_Task_RunCnt ++;
                               OS_ENTER_CRITICAL();
                               gRfiuTxSwCnt[0] = (RFUnit & 0x03);
                               OS_EXIT_CRITICAL();
                               if( gRfiuTxSwCnt[0] == (RFUnit & 0x03) )
                               {
                                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                                  if(NextTx < 0)
                                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                  else
                                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                                  //OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01<<((RFUnit+1) & 0x03) ) , OS_FLAG_SET, &err);
                               }
                               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
                            }
                        }
                    }
               }
            #endif                    
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[0] = (RFUnit & 0x03);
               OS_EXIT_CRITICAL();
            }

          #elif RFI_TEST_8TX_1RX_PROTOCOL
            if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
            {
            #if RFIU_RX_SHOW_ONLY
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   if( gRfiuTxSwCnt[0] == (RFUnit & 0x07) )
                   {
                      NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                      if(NextTx < 0)
                          DEBUG_RFIU_P2("Error! Find invalid TX\n");
                      else
                          OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                   }
               }
               else
               {
                  NextTx=sysRFRxInMainCHsel;
                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #else
               if( gRfiuTxSwCnt[0] == (RFUnit & 0x07) )
               {
                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                  if(NextTx < 0)
                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                  else
                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               }
            #endif
               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);

               if (err != OS_NO_ERR)
               {
                	DEBUG_RFIU_P2("-->Wait RFIU_%d Timeout.\n",RFUnit);
               }
            #if RFIU_RX_WAKEUP_TX_SCHEME
               if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
               {
                    if( rfiuRX_CamSleep_Sta & (0x01 << RFUnit) )
                    {
                       if(gRfiuUnitCntl[RFUnit].WakeUpTxEn ==0)
                       {
                            while( (RX_Task_RunCnt & 0x03) != 0)
                            {
                               RX_Task_RunCnt ++;
                               OS_ENTER_CRITICAL();
                               gRfiuTxSwCnt[0] = (RFUnit & 0x07);
                               OS_EXIT_CRITICAL();
                               if( gRfiuTxSwCnt[0] == (RFUnit & 0x07) )
                               {
                                  NextTx=rfiu_nTnR_FindNextTx(RFUnit);
                                  if(NextTx < 0)
                                      DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                  else
                                      OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                               }
                               OSFlagPend(gRfiu_nTx1RSwFlagGrp,(0x01<<RFUnit), OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_nTx1RxSW_TIMEOUT, &err);
                            }
                       }
                    }
               }
            #endif                    
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[0] = (RFUnit & 0x07);
               OS_EXIT_CRITICAL();
            }

            
          #endif

          //-------Send Wakeup------//
          #if RFIU_RX_WAKEUP_TX_SCHEME
            RX_Task_RunCnt++;
          
            if(gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_LISTEN)
            {   
                 if( (gRfiuUnitCntl[RFUnit].WakeUpTxEn == 1) && (gRfiuUnitCntl[RFUnit].SleepTxEn == 0) )
                 {
                 #if MESUARE_BTCWAKEUP_TIME
                    DEBUG_RFIU_P2("\n->WakeTime:%d,%dms\n",RFUnit,(guiSysTimerCnt-rfiuBTCWakeTime[RFUnit])*25);
                 #endif     
                    MeetWakeUp=1;
                 }   
                 gRfiuUnitCntl[RFUnit].WakeUpTxEn=0;
       
                 if(gRfiuUnitCntl[RFUnit].SleepTxEn == 0) //if receive Sleep command
                 {
                    rfiuRX_CamSleep_Sta &=  (~(0x01 << RFUnit));
                 }
            }
            else
            {
                if(gRfiuUnitCntl[RFUnit].WakeUpTxEn == 1)
                {
                    if(gRfiuCustomerCode[RFUnit] & 0x01)
                       WakupCh=RFI_BATCAM_WAKEUP_CH_HIGH;
                    else
                       WakupCh=RFI_BATCAM_WAKEUP_CH_LOW;
                  #if RFIU_SHARE_CTRLBUS_SUPPORT
                    OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
                  #endif
                    OS_ENTER_CRITICAL();
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    A7130_CH_sel(0+1,gRfiuDAT_CH_Table[WakupCh]);
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    A7130_CH_sel((RFUnit & 0x01)+1,gRfiuDAT_CH_Table[WakupCh]);
                #else
                    A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[WakupCh]);
                #endif
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    A7196_CH_sel(0+1,gRfiuDAT_CH_Table[WakupCh]);
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    A7196_CH_sel((RFUnit & 0x01)+1,gRfiuDAT_CH_Table[WakupCh]);
                #else
                    A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[WakupCh]);
                #endif
            #elif(RFIC_SEL==RFIC_NONE_5M)
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    RFNONE_CH_sel(0+1,gRfiuDAT_CH_Table[WakupCh]);
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    RFNONE_CH_sel((RFUnit & 0x01)+1,gRfiuDAT_CH_Table[WakupCh]);
                #else
                    RFNONE_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[WakupCh]);
                #endif
            #endif
                    OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                    OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                    //gpioSetLevel(1,9,1);
                    rfiuRxSendWakeState(  
                                          RFUnit,
                                          RFI_VITBI_DISA, 
                                          RFI_RS_T2, 
                                          RFI_VITBI_CR4_5,
                                       #if 1   
                                          gRfiuSyncWordTable[RFUnit] ^ 0xffffffff,
                                       #else
                                          0xc95a3663,
                                       #endif
                                          gRfiuCustomerCode[RFUnit] ^ 0xffffffff,
                                          0
                                        );
                    //DEBUG_RFIU_P2("0x%x ",gRfiuSyncWordTable[0] ^ 0xffffffff);
                    DEBUG_RFIU_P2("!");
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
                #else
                    rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
                #endif
                    //gpioSetLevel(1,9,0);
                }
            }
         #endif

            //----------------------------------Reply ACK-------------------------------------//
            //Select ACK channel
            timerCountRead(guiRFTimerID, &RFTimer);
            if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
            {
                // Average RSSI and judge iff change channel
                timerCountRead(guiRFTimerID, &t6);
                if(t5 >= t6)
                  dt=t5-t6;
                else
                  dt=(t5+TimerGetTimerCounter(TIMER_7))-t6;
                if(dt > 10000)
                {
                    if( (((RFTimer>>10) & 0xf) <= 13) && (((RFTimer>>10) & 0xf) >= 10) && (RX_CHG_CHflag==0) )
                    {
                       if(RSSI_cnt==0)
                         RSSI_cnt=1;
                       RSSI_avg=RSSI_Sum /RSSI_cnt;
                       //DEBUG_RFIU_P2("%d:%d:%d \n",RFUnit,RSSI_avg,ACK_CH_sel);
                    #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
                       DEBUG_RFIU_P2("RSSI_cnt=%d,%d,%d\n",RSSI_cnt,RSSI_avg,ACK_CH_sel);
                       if( (RSSI_cnt > 10000/500/6) || ((ACK_CH_sel % 3) !=0) )
                    #else
                       if(RSSI_avg > RFIU_RSSI_THR)
                    #endif
                       {
                          if( (gRfiuUnitCntl[RFUnit].RXCmd_en==0) && (RX_SYNC_ACKflag==0) && (RX_PAIR_ACKflag==0) && (gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK) && WifiScanRdy )
                          {
                              RX_CHG_CHflag=1;
                              TX_CHG_CHflag=1;
                              RX_CHG_CHCnt=(RFTimer>>14) & 0x3;
                              RX_CHG_CHNext= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                       #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
                              while(RX_CHG_CHNext == ACK_CH_sel)
                              {
                                  timerCountRead(guiRFTimerID, &temp);
                                  RX_CHG_CHNext= ((temp & 0xff) % 6) * 3;
                                  if(RX_CHG_CHNext>15)
                                    RX_CHG_CHNext=15;
                                  
                                  DEBUG_RFIU_P2("===>RX_CHG_CHNext=%d\n",RX_CHG_CHNext);
                              }
                       #endif  
                              if(RX_CHG_CHNext == ACK_CH_sel)
                              {
                                   gRfiuFCC247ChUsed[RFUnit][0]=ACK_CH_sel;
                                   gRfiuFCC247ChUsed[RFUnit][1]=-1;

                                   //DEBUG_RFIU_P2("NoChg-%d,%d,%d\n",RFUnit,ACK_CH_sel,RSSI_avg );
                                   RX_CHG_CHflag=0;
                                   TX_CHG_CHflag=0;
                              }
                              else
                              {
                                 gRfiuFCC247ChUsed[RFUnit][1]=RX_CHG_CHNext;
                                 gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
                                 gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_CHGCH;
                                 gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=(unsigned char)RX_CHG_CHNext;
                                 gRfiuUnitCntl[RFUnit].RXCmd_en = 1;
                              }

                              //DEBUG_RFIU_P2("-->Trig-%d:%d,%d\n",RFUnit,RX_CHG_CHNext, (RFTimer>>10) & 0x0f );
                       #if RF_FIXCH_OPTIM
                              OldFixCH=RX_CHG_CHNext;
                              RSSI_CheckCnt=0;
                       #endif
                          }
                       }
                       #if RF_FIXCH_OPTIM
                       else
                       {
                          if( (gRfiuUnitCntl[RFUnit].RXCmd_en==0) && (RX_SYNC_ACKflag==0) && (RX_PAIR_ACKflag==0) && (gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK) )
                          {
                          #if 0
                              if(RSSI_CheckCnt ==30)
                              {
                                  RX_CHG_CHflag=1;
                                  TX_CHG_CHflag=1;
                                  RX_CHG_CHCnt=(RFTimer>>14) & 0x3;
                                  RX_CHG_CHNext= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                                  gRfiuFCC247ChUsed[RFUnit][1]=RX_CHG_CHNext;
                                  gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
                                  gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_CHGCH;
                                  gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=(unsigned char)RX_CHG_CHNext;
                                  gRfiuUnitCntl[RFUnit].RXCmd_en = 1;
                                  DEBUG_RFIU_P2("-->Scan:%d:%d,%d\n",RFUnit,RX_CHG_CHNext, (RFTimer>>10) & 0x0f );
                              }

                              RSSI_CheckCnt ++;  
                              if(RSSI_CheckCnt>40)
                                RSSI_CheckCnt=0;
                          #endif
                          }
                          //DEBUG_RFIU_P2("RSSI_CheckCnt=%d\n",RSSI_CheckCnt);
                       }
                       #endif    
                       
                       t5=t6;
                       RSSI_Sum=0;
                       RSSI_cnt=0;
                    }
                }

                if(RX_CHG_CHflag)
                {
                    if( RX_CHG_CHCnt != ((RFTimer>>14) & 0x3) )
                    {
                       ACK_CH_sel = RX_CHG_CHNext;
                       gRfiuFCC247ChUsed[RFUnit][0]=ACK_CH_sel;
                       gRfiuFCC247ChUsed[RFUnit][1]=-1;

                       DEBUG_RFIU_P2("Chg-%d,%d,%d\n",RFUnit,ACK_CH_sel,RSSI_avg );
                       RX_CHG_CHflag=0;
                       TX_CHG_CHflag=0;
                       RSSI_Sum=0;
                       RSSI_cnt=0;
                       t5=t6;

                       //if(RFUnit==0)
                          //DEBUG_RFIU_P2("-%x",((RFTimer>>10) & 0x3f));
                    }
                    else
                    {
                       if(TX_CHG_CHflag)
                       {
                           if(((RFTimer>>10) & 0xf) < 2)
                               TX_CHG_CHflag=0;
                       }
                       //if(RFUnit==0)
                          //DEBUG_RFIU_P2("-%x",((RFTimer>>10) & 0x3f));
                    }

                }
                
            #if RFIU_RX_WAKEUP_TX_SCHEME
                if( (rfiuRX_CamSleep_Sta &  (0x01 << RFUnit) ) && (RX_PAIR_ACKflag==0) )
                {
                   if(RFUnit & 0x01)
                      ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH;  
                   else
                      ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW;  
                   DAT_CH_sel=ACK_CH_sel;
                   //DAT_CH_sel= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                }
                else if( (gRfiuSyncRevFlag[RFUnit] == 1) && (RX_PAIR_ACKflag==0) )
                {
                   if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                   {
                       if(RFUnit & 0x01)
                          ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH;  
                       else
                          ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                       DAT_CH_sel=ACK_CH_sel;
                   }
                   else
                      DAT_CH_sel=ACK_CH_sel;
                   MeetWakeUp=0;
                }
                else if(RX_LinkRetry)
                {
                     RX_LinkRetry=0;
                     if(RFUnit & 0x01)
                        ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH;  
                     else
                        ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW;  
                     DAT_CH_sel=ACK_CH_sel;
                     //DAT_CH_sel= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                     //DEBUG_RFIU_P2("\n=LINKRETRY=\n");
                }
                else
                {
                   DAT_CH_sel=ACK_CH_sel;
                }
            #else
                DAT_CH_sel=ACK_CH_sel;
            #endif

            #if 1 //RF_SCAN_WIFI_CH
                timerCountRead(guiRFTimerID, &t8);
                if(t9 >= t8)
                  dt=t9-t8;
                else
                  dt=(t9+TimerGetTimerCounter(TIMER_7))-t8;
              #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
                if(dt > 40000)// 4 sec
              #else
                if(dt > 80000)// 8 sec
              #endif
                {   
                    //DEBUG_RFIU_P2("\n");
                    for(i=0;i<RFI_DAT_CH_MAX;i++)
                    {
                       if(RSSI_CH_Cnt[i]==0)
                          RSSI_CH_Cnt[i]=1;
                    #if( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
                       RSSI_CH_Avg[i]=RSSI_CH_Sum[i]; 
                    #else
                       RSSI_CH_Avg[i]=RSSI_CH_Sum[i]/RSSI_CH_Cnt[i]; 
                    #endif
                       RSSI_CH_Sum[i]=0;
                       RSSI_CH_Cnt[i]=0;

                       //DEBUG_RFIU_P2("%d ",RSSI_CH_Avg[i]);
                    }
                    //DEBUG_RFIU_P2("\n");
                    t9=t8;
                    WifiScanRdy=1;
                }
            #endif
                
            }       
            else
            {
                if(RFUnit & 0x01)
                   RFTimer +=RF1_ACKTIMESHIFT;

            #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
                ACK_CH_sel= (RFTimer>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
		    #else
                if(RFUnit & 0x01)
                   ACK_CH_sel= ( (gRfiuTimer[RFUnit]+RF1_ACKTIMESHIFT) >>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                else
                   ACK_CH_sel= (gRfiuTimer[RFUnit]>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
            #endif

            #if RFIU_RX_WAKEUP_TX_SCHEME
                if(RX_PAIR_ACKflag==0)
                {
                    if(rfiuRX_CamSleep_Sta & (0x01 << RFUnit)) 
                    {
                       if(ACK_CH_sel < RFI_ACK_CH_MAX/2)
                          ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                       else
                          ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 

                       //if(RFUnit==0)
                          //DEBUG_RFIU_P2("A%x",ACK_CH_sel);
                    }
                    else if( (gRfiuSyncRevFlag[RFUnit] == 1) )
                    {
                       if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                       {
                          if(ACK_CH_sel < RFI_ACK_CH_MAX/2)
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                          else
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 
                          //if(RFUnit==0)
                             //DEBUG_RFIU_P2("A%x",ACK_CH_sel);
                       }
                       MeetWakeUp=0;
                    }
                    else if(gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_BROKEN)
                    {
                       if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                       {
                          if(ACK_CH_sel < RFI_ACK_CH_MAX/2)
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                          else
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 
                          //if(RFUnit==0)
                             //DEBUG_RFIU_P2("A%x",ACK_CH_sel);
                       }
                    }
                    else if(RX_LinkRetry)
                    {
                       RX_LinkRetry=0;
                       if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                       {
                          if(ACK_CH_sel < RFI_ACK_CH_MAX/2)
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                          else
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 
                          //if(RFUnit==0)
                             //DEBUG_RFIU_P2("A%x",ACK_CH_sel);
                       }
                    }
                    else if(RX_SyncRetry)
                    {
                       //DEBUG_RFIU_P2("-SYNCRETRY-\n");
                       if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
                       {
                          if(ACK_CH_sel < RFI_ACK_CH_MAX/2)
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_LOW; 
                          else
                             ACK_CH_sel=RFI_BATCAM_SLEEP_SYNC_CH_HIGH; 
                          //if(RFUnit==0)
                             //DEBUG_RFIU_P2("A%x",ACK_CH_sel);
                       }
                    }
                }
            #endif

            
            #if 1
                if( (gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_ISOWIFI) )
                {
                    timerCountRead(guiRFTimerID, &t6);
                    if(t5 >= t6)
                      dt=t5-t6;
                    else
                      dt=(t5+TimerGetTimerCounter(TIMER_7))-t6;
                    if(dt > 50000)// 5 sec
                    {   
                        for(i=0;i<RFI_DAT_CH_MAX;i++)
                        {
                           if(RSSI_CH_Cnt[i]==0)
                              RSSI_CH_Cnt[i]=1;
                           RSSI_CH_Avg[i]=RSSI_CH_Sum[i]/RSSI_CH_Cnt[i];

                        #if(SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2)   
                           if(gRfiuUnitCntl[RFUnit].RFpara.TXWifiStat == MR9211_ENTER_WIFI)
                           {
                               if( (i >= (gRfiuUnitCntl[RFUnit].RFpara.TXWifiCHNum-2)) && (i <= (gRfiuUnitCntl[RFUnit].RFpara.TXWifiCHNum+2)) )
                               {
                                   RF_CH_MaskTime[i]=160000;
                               }
                           }
                           else
                           {
                              if(RSSI_CH_Avg[i]>RFIU_RSSI_THR)
                                 RF_CH_MaskTime[i]=160000;  // 16 sec
                           }
                        #else
                           if(RSSI_CH_Avg[i]>RFIU_RSSI_THR)
                              RF_CH_MaskTime[i]=160000;  // 16 sec
                        #endif   

                           RSSI_CH_Sum[i]=0;
                           RSSI_CH_Cnt[i]=0;
                        }
                        
                        t5=t6;
                    }

                    if(t7 >= t6)
                      dt=t7-t6;
                    else
                      dt=(t7+TimerGetTimerCounter(TIMER_7))-t6;
                    for(i=0;i<RFI_DAT_CH_MAX;i++)
                    {
                       RF_CH_MaskTime[i] -= dt;
                       if(RF_CH_MaskTime[i]<0)
                          RF_CH_MaskTime[i]=0;
                    }
                    t7=t6;
                    
                  #if 0 //(HW_BOARD_OPTION != MR9200_RX_RDI_M906)  
                       DAT_CH_sel=RandHoppTab[8][ACK_CH_sel];
                  #else
                    if( RF_CH_MaskTime[ (RandHoppTab[8][ACK_CH_sel]) ] == 0)
                       DAT_CH_sel=RandHoppTab[8][ACK_CH_sel];
                    else
                    {
                       NextDAT_CH= (RandHoppTab[8][ACK_CH_sel]-3) & (RFI_DAT_CH_MAX-1);
                       MinVal=RF_CH_MaskTime[NextDAT_CH];
                       MinCh=NextDAT_CH;
                       
                       for(i=0;i<(RFI_DAT_CH_MAX/2-1);i++)
                       {
                           if(RF_CH_MaskTime[NextDAT_CH]==0)
                              break;
                           NextDAT_CH=(NextDAT_CH+1) % RFI_DAT_CH_MAX;
                           
                           if(MinVal > RF_CH_MaskTime[NextDAT_CH])
                           {
                              MinVal=RF_CH_MaskTime[NextDAT_CH];
                              MinCh=NextDAT_CH;
                           }
                       }

                       if(RF_CH_MaskTime[NextDAT_CH]!=0)
                       {
                          NextDAT_CH=MinCh;
                       }
                       DAT_CH_sel=NextDAT_CH;
                    }
                  #endif     

                  
                    if(RFUnit & 0x01)
                    {
                       RXAckCnt --;
                       if(RXAckCnt<0)
                         RXAckCnt=RFI_DAT_CH_MAX-1;
                    }
                    else
                    {
                       RXAckCnt ++;
                       if(RXAckCnt > (RFI_DAT_CH_MAX-1) )
                         RXAckCnt=0;
                    }
                }

            #endif
                
            }
          #if 0
            if(RFUnit & 0x01)
               DEBUG_RFIU_P2("[%d]",DAT_CH_sel);
            else
               DEBUG_RFIU_P2("(%d)",DAT_CH_sel);
          #endif

          //if(gRfiuUnitCntl[RFUnit].WakeUpTxEn)
          //   DEBUG_RFIU_P2("%d",ACK_CH_sel);
          #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
          #endif
            OS_ENTER_CRITICAL();
      #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] ); 
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] ); 
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] ); 
          #endif
      #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
            RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
          #endif
      #else
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #endif
      #endif
            OS_EXIT_CRITICAL();
        #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPost(gRfiuCtrlBusReqSem);
        #endif

        #if 1
            if( DAT_CH_sel >= RFI_DAT_CH_MAX )
            {
                DAT_CH_sel=RFI_DAT_CH_MAX-1;
                //DEBUG_RFIU_P2("\nWarning!!!! DAT_CH_sel=%d\n",DAT_CH_sel);
            }
        #endif        
            RX_UsrData = (RX_UsrData & (~(RFIU_USRDATA_DATACH_MASK<<RFIU_USRDATA_DATACH_SHFT))) | ((DAT_CH_sel & RFIU_USRDATA_DATACH_MASK)<<RFIU_USRDATA_DATACH_SHFT);
          #if DEBUG_WRPTR_WSHFT
            RX_UsrData = (RX_UsrData & (~(0x1ff<<11))) | ((gRfiuUnitCntl[RFUnit].BufWritePtr & 0x1f)<<11) | ((gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpShift & 0xf)<<16);
          #endif
         
            if((gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_LISTEN) || (gRfiuUnitCntl[RFUnit].OpState==RFIU_RX_STATE_INIT) )
            {   
                 //Reply New ACK//
            #if( RF_CMD_EN && (DEBUG_WRPTR_WSHFT==0) )
                 //=== RX command from RX to TX (bit11~bit20)
                 if(gRfiuUnitCntl[RFUnit].RXCmd_en)
                 {
    			    RX_UsrData = (RX_UsrData & (~(RFIU_USRDATA_CMD_CHEK | RFIU_USRDATA_RXCMDTYP_CHEK))) | ((gRfiuUnitCntl[RFUnit].RXCmd_en & RFIU_USRDATA_CMD_MASK) << RFIU_USRDATA_CMD_SHFT) | ((gRfiuUnitCntl[RFUnit].RXCmd_Type & RFIU_USRDATA_RXCMDTYP_MASK)<<RFIU_USRDATA_RXCMDTYP_SHFT);
        			OS_ENTER_CRITICAL();
                    gRfiuUnitCntl[RFUnit].RXCmd_Busy = 1;
                	OS_EXIT_CRITICAL();	       
                 }
                 else
                 {
                    RX_UsrData = RX_UsrData & (~(RFIU_USRDATA_CMD_CHEK | RFIU_USRDATA_RXCMDTYP_CHEK));
                 }
 		    #endif	 

            #if RFIU_RX_AUDIO_RETURN
                 if(gRfiuUnitCntl[RFUnit].RXCmd_AudioRetEn)
                 {
    			    RX_UsrData = (RX_UsrData & (~(RFIU_USRDATA_RXGRPDIV_CHEK | RFIU_USRDATA_RXGRPWPR_CHEK | RFIU_USRDATA_RXSEQNUM_CHEK))) | 
                                 (((rfiuAudioRetRead_idx & 0x07) & RFIU_USRDATA_RXGRPDIV_MASK) << RFIU_USRDATA_RXGRPDIV_SHFT) | 
                                 (((rfiuAudioRetRead_idx>>3) & RFIU_USRDATA_RXGRPWPR_MASK)<<RFIU_USRDATA_RXGRPWPR_SHFT);

                    //DEBUG_RFIU_P2("%d ",rfiuAudioRetRead_idx);
                 }
                 else
                 {
                    RX_UsrData = RX_UsrData & (~(RFIU_USRDATA_RXGRPDIV_CHEK | RFIU_USRDATA_RXGRPWPR_CHEK | RFIU_USRDATA_RXSEQNUM_CHEK));
                 }

            #endif

                RX_UsrACK=RX_UsrData;
                rfiuRxReplyACKState( RFUnit,
                                     RFI_VITBI_DISA, 
                                     RFI_RS_T12, 
                                     RFI_VITBI_CR4_5,
                                     gRfiuSyncWordTable[RFUnit],
                                     gRfiuCustomerCode[RFUnit],
                                     RX_UsrACK,
                                     RX_TimeCheck,
                                     TX_CHG_CHflag,
                                     RX_CHG_CHNext,
                                     0
                                    );
            #if RFI_SELF_TEST_TXRX_PROTOCOL    
                DEBUG_RFIU_P("<------Reply ACK_CH=%d \n",ACK_CH_sel);
            #endif
                gRfiuUnitCntl[RFUnit].OpState = RFIU_RX_STATE_REPLY_ACK;           
            }
            else
            {   
                RX_UsrACK=RX_UsrData;
            #if RFIU_RX_AUDIO_RETURN
                if(gRfiuUnitCntl[RFUnit].RXCmd_AudioRetEn)
                {
    			    RX_UsrData = (RX_UsrData & (~(RFIU_USRDATA_RXGRPDIV_CHEK | RFIU_USRDATA_RXGRPWPR_CHEK | RFIU_USRDATA_RXSEQNUM_CHEK))) | 
                                 (((rfiuAudioRetRead_idx & 0x07) & RFIU_USRDATA_RXGRPDIV_MASK) << RFIU_USRDATA_RXGRPDIV_SHFT) | 
                                 (((rfiuAudioRetRead_idx>>3) & RFIU_USRDATA_RXGRPWPR_MASK)<<RFIU_USRDATA_RXGRPWPR_SHFT) |
                                 RFIU_USRDATA_RXSEQNUM_CHEK;
                }
                else
                {
                    RX_UsrData = RX_UsrData & (~(RFIU_USRDATA_RXGRPDIV_CHEK | RFIU_USRDATA_RXGRPWPR_CHEK | RFIU_USRDATA_RXSEQNUM_CHEK));
                }
            #endif
                
                rfiuRxReplyACKState( RFUnit,
                                     RFI_VITBI_DISA, 
                                     RFI_RS_T12, 
                                     RFI_VITBI_CR4_5,
                                     gRfiuSyncWordTable[RFUnit],
                                     gRfiuCustomerCode[RFUnit],
                                     RX_UsrACK,
                                     RX_TimeCheck,
                                     TX_CHG_CHflag,
                                     RX_CHG_CHNext,
                                     1
                                    );
            #if RFI_SELF_TEST_TXRX_PROTOCOL    
                DEBUG_RFIU_P("<------Retry ACK_CH=%d \n",ACK_CH_sel);
            #endif
                gRfiuUnitCntl[RFUnit].OpState = RFIU_RX_STATE_RETRY_ACK; 
                RxAckRetryCnt ++; 
            }
         
         #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
            rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
         #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
            rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
         #else
            rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
         #endif
         
         #if(RF_PAIR_EN)  
            if(MACadrsSetflag[RFUnit] >0)
            {
                  gRfiuSyncWordTable[RFUnit] = Temp_RX_MAC_Address[RFUnit];
                  gRfiuCustomerCode[RFUnit] = Temp_RX_CostomerCode[RFUnit];
               #if RFIU_SHARE_CTRLBUS_SUPPORT
                  OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
               #endif
                  OS_ENTER_CRITICAL();
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
               #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                  A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
               #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                  A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
               #else       
                  A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] ); 
               #endif
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
               #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                  A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
               #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                  A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
               #else       
                  A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] ); 
               #endif
            #elif(RFIC_SEL==RFIC_NONE_5M)
               #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                  RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
               #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                  RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
               #else       
                  RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] ); 
               #endif
            #endif
                  OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                  OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                  MACadrsSetflag[RFUnit] =0;
                  RX_PAIR_ACKflag=1;
            } 
         #endif


            //===Check Sync===//
            if(gRfiuSyncRevFlag[RFUnit] == 1)
            {
               RX_SYNC_ACKflag=1;
               RX_SyncRetry=1;
               gRfiuSyncRevFlag[RFUnit] =0;
            }
       
            #if DEBUG_TX_TIMEOUT
            #endif 
            break;  

      #if(RF_PAIR_EN) 
        case RFIU_PAIRLint_MODE:
                gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
                gRfiu_Op_Sta[RFUnit] = RFIU_OP_INIT;
			    //sprintf(sysBackOsdString,"Paring CH-%d",(RFUnit+1));
				//sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;
                Old_RXMAC = gRfiuSyncWordTable[RFUnit];
                Old_RXCode= gRfiuCustomerCode[RFUnit];
                gRfiuSyncWordTable[RFUnit]=RFI_PAIR_SYNCWORD;
                gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                MACadrsSetflag[RFUnit]=0;
                gRfiuUnitCntl[RFUnit].RFpara.RxForceTxFirstReboot=0; //Lucian: 配對時必須將此功能關閉, 避免TX 寫ID時掉碼.
                gRfiuUnitCntl[RFUnit].RFpara.TxBatteryLev=RF_BATCAM_TXBATSTAT_NOSHOW;
                RX_SYC_ErrorCnt=0;
                RX_PAIR_ACKflag=0; 
           #if  RFIU_RX_WAKEUP_TX_SCHEME
                gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
           #endif    
              #if RFIU_SHARE_CTRLBUS_SUPPORT
                OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
              #endif
                OS_ENTER_CRITICAL();
       #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL) 
                A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else     
                A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );  
           #endif
       #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else     
                A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );  
           #endif
       #elif(RFIC_SEL==RFIC_NONE_5M)
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else     
                RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );  
           #endif
       #endif
                OS_EXIT_CRITICAL();
            #if RFIU_SHARE_CTRLBUS_SUPPORT
                OSSemPost(gRfiuCtrlBusReqSem);
            #endif
				PairCnt=0;
           break; 

       case RFIU_PAIR_STOP_MODE:
                gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;        
                gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_LINK_BROKEN;
                if(gRfiuUnitCntl[RFUnit].RX_Pair_Done)
                {
                    RX_PAIR_ACKflag=0;
                    RX_SYC_ErrorCnt=0;
                    gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;
                    gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                }
                else
                {
                    //sprintf(sysBackOsdString,"                  ");
    			    //sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                    RX_PAIR_ACKflag=0;
                    RX_SYC_ErrorCnt=0;
                    gRfiuSyncWordTable[RFUnit]=Old_RXMAC;
                    gRfiuCustomerCode[RFUnit]=Old_RXCode;
                    gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;
                  #if RFIU_SHARE_CTRLBUS_SUPPORT
                    OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
                  #endif
                    OS_ENTER_CRITICAL();
          #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )          
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                #else
                    A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                #endif
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                #else
                    A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                #endif
          #elif(RFIC_SEL==RFIC_NONE_5M)
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                    RFNONE_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                    RFNONE_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                #else
                    RFNONE_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                #endif
          #endif
                    OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                    OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                    gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                }
           break;
           
      #endif  

      #if TX_FW_UPDATE_SUPPORT
        //------------------------------------------------------------------------//
        case RFIU_FWUPD_INIT:
            gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
            gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
            gRfiuTxFwUpdPercent[RFUnit]=0;
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
        #if  RFIU_RX_WAKEUP_TX_SCHEME    
            gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
        #endif
            OSTimeDly(8);
            if(gRfiu_MpegDec_Sta[RFUnit] == RFI_MPEGDEC_TASK_RUNNING)
            {
                switch(RFUnit)
                {
                    case 0:
                        OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
                        OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
                        break;

                    case 1:
                        OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
                        OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
                        break;

                    case 2:
                        OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
                        OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
                        break;

                    case 3:
                        OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
                        OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
                        break;
                }
                gRfiu_MpegDec_Sta[RFUnit]= RFI_MPEGDEC_TASK_NONE;
                //DEBUG_SYS("\n-->1\n");
            }

            if(gRfiu_WrapDec_Sta[RFUnit] == RFI_WRAPDEC_TASK_RUNNING)
            {
                switch(RFUnit)
                {
                    case 0:
                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
                        break;

                    case 1:
                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
                        break;

                    case 2:
                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT2);
                        break;

                    case 3:
                        OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                        OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT3);
                        break;
                }
                gRfiu_WrapDec_Sta[RFUnit] = RFI_WRAPDEC_TASK_NONE;
            } 
            //---Check TX support FW Update----//
            if(gRfiuUnitCntl[RFUnit].FWUpdate_support == 0)
            {
                gRfiuUnitCntl[RFUnit].OpMode =RFIU_RX_MODE;
                gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_RETRY_ACK;
                OSTimeDly(60);
                DEBUG_RFIU_P2("------>The TX don't support FW Update! Go to RX mode\n");
                break;
            }
            //---Load TX FW(1MB)---//
            memcpy(rfiuOperBuf[RFUnit],rfiuTXFwUpdBuf,1024*1024); // 1MB code
            //---Calculate checksum---//
            TxFWSum=rfiuCalTxFwCheckSum(RFUnit);

            timerCountRead(guiRFTimerID, &t4);
            t3=t4;
            dt=0;
            RecvFwUpdStart=0;
            while(1)
            {
                timerCountRead(guiRFTimerID, &RFTimer);
                if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                {

                }
                else
                {
                    if(RFUnit & 0x01)
                        RFTimer +=RF1_ACKTIMESHIFT;

                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
                    ACK_CH_sel= (RFTimer>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
    		    #else
                    if(RFUnit & 0x01)
                       ACK_CH_sel= ( (gRfiuTimer[RFUnit]+RF1_ACKTIMESHIFT) >>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                    else
                       ACK_CH_sel= (gRfiuTimer[RFUnit]>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                #endif
                  #if RFIU_SHARE_CTRLBUS_SUPPORT
                    OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
                  #endif
                    OS_ENTER_CRITICAL();
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                  #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    A7196_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif(RFIC_SEL==RFIC_NONE_5M)
                    RFNONE_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #endif
                #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                  #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    A7196_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif(RFIC_SEL==RFIC_NONE_5M)  
                    RFNONE_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #endif
                #else
                  #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #elif(RFIC_SEL==RFIC_NONE_5M)  
                    RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
                  #endif
                #endif
                   OS_EXIT_CRITICAL();
                #if RFIU_SHARE_CTRLBUS_SUPPORT
                   OSSemPost(gRfiuCtrlBusReqSem);
                #endif
                
                }

                rfiuReplyACK_FWUPD(  
                                     RFUnit,
                                     RFI_VITBI_DISA, 
                                     RFI_RS_T12, 
                                     RFI_VITBI_CR4_5,
                                     gRfiuSyncWordTable[RFUnit],
                                     gRfiuCustomerCode[RFUnit],
                                     0,
                                     RX_TimeCheck,
                                     0,
                                     0,
                                     RXACK_FWUPD_START,
                                     TxFWSum
                                  );
             #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
             #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
             #else
                rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
             #endif

                rfiuFwUpdListenDataState( RFUnit,
                                          RFI_VITBI_DISA, 
                                          RFI_RS_T12, 
                                          RFI_VITBI_CR4_5,
                                          gRfiuSyncWordTable[RFUnit],
                                          gRfiuCustomerCode[RFUnit],
                                          RFI_RX_WAIT_TIME
                                         );

            #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //R
            #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #else
                RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #endif

                if( (gRfiuParm_Rx[RFUnit].TxRxPktNum) != 0)
                {
                    if( rfiuFwUpdCheckStartCome(&gRfiuParm_Rx[RFUnit]) == 1 )
                    {
                        DEBUG_RFIU_P2("-->PKT_FWUPD_START:%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum);
                        RecvFwUpdStart=1;
                    }
                }
            
                //-------------------------------------//
                timerCountRead(guiRFTimerID, &t4);
                if(t3 >= t4)
                  dt=t3-t4;
                else
                  dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                if( (dt >30000)  || RecvFwUpdStart)
                {
                    if(RecvFwUpdStart)
                    {
                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_START;
                       DEBUG_RFIU_P2("------>FW update Start:0x%x\n",TxFWSum);
                    }
                    else
                    {
                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                       gRfiuTxFwUpdPercent[RFUnit]=-2;
                       DEBUG_RFIU_P2("------>FW update Init time-out! Go to RX mode\n");
                    }
                    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_RETRY_ACK;
                    break;
                }
            }
            
            ACK_CH_sel=RFI_TXFWUPD_CH;
          #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPend(gRfiuCtrlBusReqSem, OS_IPC_WAIT_FOREVER, &err);
          #endif
            OS_ENTER_CRITICAL();
        #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
          #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(0+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #endif
        #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
          #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #endif
        #else
          #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(RFUnit+1,gRfiuACK_CH_Table[ACK_CH_sel]);//
          #endif
        #endif
            OS_EXIT_CRITICAL();
        #if RFIU_SHARE_CTRLBUS_SUPPORT
            OSSemPost(gRfiuCtrlBusReqSem);
        #endif
        
            break;
        //------------------------------------------------------------------------//
        case RFIU_FWUPD_START:
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
            timerCountRead(guiRFTimerID, &t4);
            t3=t4;
            dt=0;
            RecvFwUpdDone=0;
            gRfiuTxFwUpdPercent[RFUnit]=0;
            while(1)
            {
                rfiuTxWaitACKState( RFUnit,
                                    RFI_VITBI_DISA, 
                                    RFI_RS_T12, 
                                    RFI_VITBI_CR4_5,
                                    gRfiuSyncWordTable[RFUnit],
                                    gRfiuCustomerCode[RFUnit],
                                    RFI_TX_WAIT_TIME
                                  );    

            #if  RFIU_RX_WAKEUP_TX_SCHEME    
                gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
            #endif


            #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #else
                RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #endif
                if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 1 )
                {
                    ACKType = rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                    //DEBUG_RFIU_P2("-->Recv:0x%x\n",ACKType);
                    if( RXACK_FWUPD_DONE == ACKType)
                    {
                        RecvFwUpdDone=1;
                        rfiuFwUpdSend_Done_State(  
                                                  RFUnit,
                                                  RFI_VITBI_DISA, 
                                                  RFI_RS_T12, 
                                                  RFI_VITBI_CR4_5,
                                                  gRfiuSyncWordTable[RFUnit],
                                                  gRfiuCustomerCode[RFUnit],
                                                  0
                                                );
                    #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);
                    #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);
                    #else
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);
                    #endif 
                        gRfiuTxFwUpdPercent[RFUnit]=100;
                        DEBUG_RFIU_P2("-->Recv DONE_ACK\n");
                    }
                    else if(RXACK_FWUPD_DATA == ACKType)
                    {  //Data transfer//
                    #if 1
                        UpdatePtr=rfiuGetACK2FwUpdDataMap(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128);
                        rfiuFwUpdSend_Data_State( 
                                                  RFUnit,
                                                  RFI_VITBI_DISA, 
                                                  RFI_RS_T12, 
                                                  RFI_VITBI_CR4_5,
                                                  gRfiuSyncWordTable[RFUnit],
                                                  gRfiuCustomerCode[RFUnit],
                                                  0
                                                );
                    #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);
                    #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);
                    #else
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);
                    #endif
                        gRfiuTxFwUpdPercent[RFUnit]=UpdatePtr*100/128;
                        DEBUG_RFIU_P2("-->Recv DATA_ACK:%d\n",UpdatePtr);
                    #endif
                    }
                }        
                //-------------------------------------//
                timerCountRead(guiRFTimerID, &t4);
                if(t3 >= t4)
                  dt=t3-t4;
                else
                  dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                if( (dt >600000)  || RecvFwUpdDone)
                {
                    if(RecvFwUpdDone)
                    {
                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_FWUPD_DONE;
                       DEBUG_RFIU_P2("------>FW update Done:0x%x\n",TxFWSum);
                    }
                    else
                    {
                       gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                       gRfiuTxFwUpdPercent[RFUnit]= -1;
                       DEBUG_RFIU_P2("------>FW update START time-out! Go to RX mode\n");
                    }
                    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_RETRY_ACK;
                    break;
                }

            }
            //-----------//
            break;
        //------------------------------------------------------------------------//
        case RFIU_FWUPD_DONE:
            gRfiuUnitCntl[RFUnit].RXRecvDataUse3M=0;
        #if  RFIU_RX_WAKEUP_TX_SCHEME    
            gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
        #endif
            timerCountRead(guiRFTimerID, &t4);
            t3=t4;
            dt=0;
            while(1)
            {
                rfiuTxWaitACKState( RFUnit,
                                    RFI_VITBI_DISA, 
                                    RFI_RS_T12, 
                                    RFI_VITBI_CR4_5,
                                    gRfiuSyncWordTable[RFUnit],
                                    gRfiuCustomerCode[RFUnit],
                                    RFI_TX_WAIT_TIME
                                  );    
            #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01 ,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #else
                RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],0,&RSSI_Wifi);  //Rx
            #endif
                if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFUnit]) == 1 )
                {
                    ACKType = rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp );
                    //DEBUG_RFIU_P2("-->Recv:0x%x\n",ACKType);
                    if( RXACK_FWUPD_DONE == ACKType)
                    {
                        rfiuFwUpdSend_Done_State(  
                                                      RFUnit,
                                                      RFI_VITBI_DISA, 
                                                      RFI_RS_T12, 
                                                      RFI_VITBI_CR4_5,
                                                      gRfiuSyncWordTable[RFUnit],
                                                      gRfiuCustomerCode[RFUnit],
                                                      0
                                                  );
                    #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
                        rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);
                    #elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL)
                        rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);
                    #else
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);
                    #endif
                        DEBUG_RFIU_P2("-->Recv DONE_ACK\n");
                    }
                }        
                //-------------------------------------//
                timerCountRead(guiRFTimerID, &t4);
                if(t3 >= t4)
                  dt=t3-t4;
                else
                  dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                if(dt >30000)
                {
                    gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_RETRY_ACK;
                    DEBUG_RFIU_P2("------>FW update Complete! Go to RX mode\n");
                    break;
                }
            }


            gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
            break;
        //------------------------------------------------------------------------//
      #endif

      
        case RFIU_IDLE_MODE:
        default:
            OSTimeDly(2);
            break;
           
	    }

	}
    
}

int CheckFCC247ChDiff( int CHNext,int RFUnit)
{
    int i;
    
    if(gRfiuFCC247ChUsed[RFUnit][0]== CHNext)
        return 0;

#if 0
    for(i=((RFUnit+1)& 0x01);i<MAX_RFIU_UNIT;i += 2)
    {
       if((rfiuRX_CamOnOff_Sta >> i) & 0x01)
       {
          if(gRfiuFCC247ChUsed[i][0] ==  CHNext)
             return 0;

          if(gRfiuFCC247ChUsed[i][1] ==  CHNext)
             return 0;
       }
    }
#endif

    return 1;

}


#if 0 //((HW_BOARD_OPTION==MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION==MR9200_RX_TRANWO_D8710R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H)||(HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
int rfiuNoWifiCHsel(int RFUnit,u32 RSSI_CH_Avg[])
{
    const u8 ScanTab[RFI_DAT_CH_MAX][5]=
    {
      { 1,  3,  0,  0,  2},   // 0
      { 0,  1,  1,  2,  3},   // 1
      { 0,  1,  2,  3,  4},   // 2
      { 1,  2,  3,  4,  5},   // 3
      { 2,  3,  4,  5,  6},   // 4
      { 3,  4,  5,  6,  7},   // 5
      { 4,  5,  6,  7,  8},   // 6
      { 5,  6,  7,  8,  9},   // 7
      { 6,  7,  8,  9, 10},   // 8
      { 7,  8,  9, 10, 11},   // 9
      { 8,  9, 10, 11, 12},   // 10
      { 9, 10, 11, 12, 13},   // 11
      {10, 11, 12, 13, 14},   // 12
      {11, 12, 13, 14, 15},   // 13
      {12, 13, 14, 14, 15},   // 14
      {13, 14, 15, 12, 15}    // 15
    };

    const u8 CHTab[2][RFI_DAT_CH_MAX]=
    {   // 0   1    2    3   4   5  6  7   8  9   10   11  12   13  14  15
        { 12, 12, 12, 12, 12,12, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0},
        { 15, 15, 15, 15, 15,15,15,15,15,15,  3,  3,  3,  3,  3,  3}
    };
   int i,j;
   int sum,min,index;
   u32 RSSI_CH_Sum[RFI_DAT_CH_MAX];
   u32 RSSI_temp[RFI_DAT_CH_MAX];
   int max;
   //------------------------------------------//
   sum=0;
   min=0xfffff;
   max=0;
   index=0;
   
   for(j=0;j<RFI_DAT_CH_MAX;j++)
   {
       RSSI_CH_Sum[j]=0;  
       for(i=0;i<MAX_RFIU_UNIT;i++)
          RSSI_CH_Sum[j] += rfiuRSSI_CH_Avg[i][j];

   }
   //-------------Find max RSSI area-------------//
   for(i=0;i<RFI_DAT_CH_MAX;i++)
       RSSI_temp[i]=RSSI_CH_Sum[i];

   max=0;
   //DEBUG_RFIU_P2("\n");
   for(i=0;i<RFI_DAT_CH_MAX;i++)
   {
      sum=0;
      for(j=0;j<5;j++)
      {
         sum += RSSI_temp[ ScanTab[i][j] ];
      }

      //DEBUG_RFIU_P2("%d ",sum);
      if(sum > max)
      {
         max=sum;
         index=i;
      }
   }   
   
   for(i=0;i<5;i++)
       RSSI_CH_Sum[ ScanTab[index][i] ]=9999;
   //--------------------------------------------//
#if 0   
   for(i=0;i<MAX_CH_SCAN;i++)
   {
      sum=0;
      for(j=0;j<5;j++)
      {
         sum += RSSI_CH_Sum[ ScanTab[CHTab[RFUnit & 0x01][i] ][j] ];
      }
      
      if(sum < min)
      {
         min=sum;
         index=i;
         //DEBUG_RFIU_P2("[%d,%d]\n",min,index);
      }
   }
#endif   
#if 1  
   if(RFUnit == 0)
   {
       DEBUG_RFIU_P2("\n%d,%d:",RFUnit,rfiuRX_CamOnOff_Num);

       //for(i=0;i<16;i++)
       //  DEBUG_RFIU_P2("%3d ",RSSI_temp[i]);     
       //DEBUG_RFIU_P2("\n    ");

       for(i=0;i<16;i++)
         DEBUG_RFIU_P2("%3d ",RSSI_CH_Sum[i]);     
       DEBUG_RFIU_P2("\n");
   }
#endif

   return CHTab[RFUnit & 0x01][index];

}
#elif( (SW_APPLICATION_OPTION==MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION==MR9100_RF_AHD_AVSED_RX1)  || (SW_APPLICATION_OPTION==MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1)||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) )
int rfiuNoWifiCHsel(int RFUnit,u32 RSSI_CH_Avg[])
{

    const u8 ScanTab[RFI_DAT_CH_MAX][5]=
    {
      { 1,  0,  0},   // 0
      { 1,  1,  2},   // 1
      { 1,  2,  3},   // 2
      { 2,  3,  4},   // 3
      { 3,  4,  5},   // 4
      { 4,  5,  6},   // 5
      { 5,  6,  7},   // 6
      { 6,  7,  8},   // 7
      { 7,  8,  9},   // 8
      { 8,  9, 10},   // 9
      { 9, 10, 11},   // 10
      {10, 11, 12},   // 11
      {11, 12, 13},   // 12
      {12, 13, 14},   // 13
      {13, 14, 15},   // 14
      {14, 15, 15}    // 15
    };
   int i,j;
   int sum,min,index;
   //------------------------------------------//
   sum=0;
   min=0x7fffffff;
   index=0;
   

#if 1
   for(i=0;i<RFI_DAT_CH_MAX;i+=3)
#else
   for(i=0;i<RFI_DAT_CH_MAX;i+=2)
#endif
   {
      sum=0;
      for(j=0;j<3;j++)
      {
         sum += RSSI_CH_Avg[ ScanTab[i][j] ];
      }
      
      if(sum < min)
      {
         min=sum;
         index=i;
      }
   }

#if 0   
   DEBUG_RFIU_P2("--->rfiuNoWifiCHsel:%d\n",index);
   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%4d ",RSSI_CH_Avg[i]);  
   DEBUG_RFIU_P2("\n");
#endif

   return index;

}

#else
int rfiuNoWifiCHsel(int RFUnit,u32 RSSI_CH_Avg[])
{
    const u8 ScanTab[RFI_DAT_CH_MAX][5]=
    {
      { 1,  1,  0,  0,  2},   // 0
      { 0,  1,  1,  2,  3},   // 1
      { 0,  1,  2,  3,  4},   // 2
      { 1,  2,  3,  4,  5},   // 3
      { 2,  3,  4,  5,  6},   // 4
      { 3,  4,  5,  6,  7},   // 5
      { 4,  5,  6,  7,  8},   // 6
      { 5,  6,  7,  8,  9},   // 7
      { 6,  7,  8,  9, 10},   // 8
      { 7,  8,  9, 10, 11},   // 9
      { 8,  9, 10, 11, 12},   // 10
      { 9, 10, 11, 12, 13},   // 11
      {10, 11, 12, 13, 14},   // 12
      {11, 12, 13, 14, 15},   // 13
      {12, 13, 14, 14, 15},   // 14
      {13, 14, 15, 14, 15}    // 15
    };

   #if 0 //RFIU_RX_WAKEUP_TX_SCHEME  //Lucian: CH0,CH1,CH14,CH15 保留給wakeup 用
    const u8 CHTab[RFI_DAT_CH_MAX][2][MAX_CH_SCAN]=
    {
      //CH-0
      {
        { 4, 5, 6, 7, 8, 8, 8, 8},
        { 11,12,13,13,13,13,13,13}
      },
      //CH-1
      {
        {  5, 5, 6, 7, 8, 8, 8, 8},
        { 11,12,13,13,13,13,13,13}
      },
      //CH-2
      {
        { 6, 6, 6, 7, 8, 9, 9, 9},
        { 12,12,13,13,13,13,13,13}
      },
      //CH-3
      {
        { 7, 7, 7, 7, 8, 9, 9, 9},
        { 12,12,13,13,13,13,13,13}
      },
      //CH-4
      {
        { 8, 8, 8, 8, 8, 9, 9, 10},
        { 13,13,13,13,13,13,13,13}
      },
      //CH-5
      {
        { 9, 9, 9, 9, 9, 10,10,10},
        { 13,13,13,13,13,13,13,13}
      },
      //CH-6
      {
        { 2, 2, 2, 2, 2, 10,10,10},
        { 13,13,13,13,13,13,13,13}
      },
      //CH-7
      {
        { 2, 2, 2, 2, 2, 3,  3, 3},
        { 11,12,13,13,13,13,13,13}
      },
      //CH-8
      {
        { 2, 2, 2, 2, 2, 3,  4, 4},
        { 12,12,13,13,13,13,13,13}
      },
      //CH-9
      {
        { 2, 2, 2, 2, 2, 3,  4, 4},
        { 13,13,13,13,13,13,13,13}
      },
      //CH-10
      {
        { 2, 2, 2, 2, 2, 3, 3, 3},
        { 6, 6, 6, 6, 6, 6, 6, 6}
      },
      //CH-11
      {
        { 2, 2, 2, 2, 2, 3, 3, 3},
        { 6, 6, 6, 7, 7, 7, 7, 7}
      },
      //CH-12
      {
        { 2, 2, 2, 2, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 8, 8, 8}
      },
      //CH-13
      {
        { 2, 2, 2, 2, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 9, 9, 9}
      },
      //CH-14
      {
        { 2, 2, 2, 2, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 9, 9,10}
      },
      //CH-15
      {
        { 2, 2, 2, 2, 2, 3, 3, 4},
        { 7, 7, 7, 7, 8, 9,10,11}
      },      
    };
   #else
    const u8 CHTab[RFI_DAT_CH_MAX][2][MAX_CH_SCAN]=
    {
      //CH-0
      {
        { 4, 5, 6, 7, 8, 8, 8, 8},
        { 11,12,13,14,15,15,15,15}
      },
      //CH-1
      {
        {  5, 5, 6, 7, 8, 8, 8, 8},
        { 11,12,13,14,15,15,15,15}
      },
      //CH-2
      {
        { 6, 6, 6, 7, 8, 9, 9, 9},
        { 12,12,13,14,15,15,15,15}
      },
      //CH-3
      {
        { 7, 7, 7, 7, 8, 9, 9, 9},
        { 12,12,13,14,15,15,15,15}
      },
      //CH-4
      {
        { 0, 0, 0, 0, 8, 9, 9, 10},
        { 13,13,13,14,15,15,15,15}
      },
      //CH-5
      {
        { 0, 0, 1, 1, 9, 10,10,10},
        { 13,13,13,14,15,15,15,15}
      },
      //CH-6
      {
        { 0, 0, 1, 1, 2, 10,10,10},
        { 13,13,13,14,15,15,15,15}
      },
      //CH-7
      {
        { 0, 0, 1, 1, 2, 3,  3, 3},
        { 11,12,13,14,15,15,15,15}
      },
      //CH-8
      {
        { 0, 0, 1, 1, 2, 3,  4, 4},
        { 12,12,13,13,14,14,15,15}
      },
      //CH-9
      {
        { 0, 0, 1, 1, 2, 3,  4, 4},
        { 13,13,13,14,14,15,15,15}
      },
      //CH-10
      {
        { 0, 0, 1, 1, 2, 3, 3, 3},
        { 6, 6, 6,14,15,15,15,15}
      },
      //CH-11
      {
        { 0, 0, 1, 1, 2, 3, 3, 3},
        { 6, 6, 6, 7,15,15,15,15}
      },
      //CH-12
      {
        { 0, 0, 1, 1, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 8, 8, 8}
      },
      //CH-13
      {
        { 0, 0, 1, 1, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 9, 9, 9}
      },
      //CH-14
      {
        { 0, 0, 1, 1, 2, 3, 3, 3},
        { 6, 6, 6, 7, 8, 9, 9,10}
      },
      //CH-15
      {
        { 0, 0, 1, 1, 2, 3, 3, 4},
        { 7, 7, 7, 7, 8, 9,10,11}
      },      
    };
   #endif
   int i,j,k;
   int sum,min,index;
   u32 RSSI_CH_Sum[RFI_DAT_CH_MAX];
   u32 RSSI_temp[RFI_DAT_CH_MAX];
   int max;
   int flag;
   int Wifi_Central_CH;
   //------------------------------------------//
   sum=0;
   min=0xfffff;
   max=0;
   index=0;
   
   for(j=0;j<RFI_DAT_CH_MAX;j++)
   {
       RSSI_CH_Sum[j]=0;  
       for(i=0;i<MAX_RFIU_UNIT;i++)
          RSSI_CH_Sum[j] += rfiuRSSI_CH_Avg[i][j];

   }
   //-------------Find max RSSI area-------------//
#if 1   
   for(i=0;i<RFI_DAT_CH_MAX;i++)
       RSSI_temp[i]=RSSI_CH_Sum[i];

   max=0;
   //DEBUG_RFIU_P2("\n");
   for(i=0;i<RFI_DAT_CH_MAX;i++)
   {
      sum=0;
      for(j=0;j<5;j++)
      {
         sum += RSSI_temp[ ScanTab[i][j] ];
      }

      //DEBUG_RFIU_P2("%d ",sum);
      if(sum > max)
      {
         max=sum;
         index=i;
      }
   }   
   Wifi_Central_CH=index;
   for(i=0;i<5;i++)
       RSSI_CH_Sum[ ScanTab[index][i] ]=99999;
#endif
   //--------------------------------------------//
#if 1   
   min=0xfffff;
   for(i=0;i<MAX_CH_SCAN;i++)
   {
      sum = RSSI_CH_Sum[ CHTab[Wifi_Central_CH][RFUnit & 0x01][i]  ];
      
      if(sum < min)
      {
         min=sum;
         index=i;
         //DEBUG_RFIU_P2("[%d,%d]\n",min,index);
      }      
   }
#endif   
#if 0   
   DEBUG_RFIU_P2("\n%d,%d,%d:",RFUnit,rfiuRX_CamOnOff_Num,Wifi_Central_CH);

   #if 0
   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_temp[i]);     
   DEBUG_RFIU_P2("\n    ");
   #endif
   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_CH_Sum[i]);     
   DEBUG_RFIU_P2("\n");
   
#endif
   
   return CHTab[Wifi_Central_CH][RFUnit & 0x01][index];

}
#endif




#endif

//-----------------------------Inner test--------------------------//
    

#if RFIU_TEST
    int rfiuCheckTestResult(int TxUnit,int RxUnit,int CheckPktMap, int CheckPktBurstNum)
    {
        int i,j,k;
        char err;
        unsigned int temp,map1,map2;
        unsigned char *TxAddr,*RxAddr;

       //-----------Compare PacketMap,Pkt_Grp_offset,MacID-----------//
       err=0;
       if(CheckPktMap)
       {
            DEBUG_RFIU("---->Check Packet Map:");
            for(i=0;i<4;i++)
            {
                   if(gRfiuParm_Tx[TxUnit].PktMap[i] != gRfiuParm_Rx[RxUnit].PktMap[i])
                   {
                      err=1;
                      DEBUG_RFIU_P("Map FAIL:0x%x,0x%x\n",gRfiuParm_Tx[TxUnit].PktMap[i],gRfiuParm_Rx[RxUnit].PktMap[i]);
                      break;
                   }
            }
            DEBUG_RFIU("PASS\n");
        }   
        //---//
        DEBUG_RFIU("---->Check Pkt_Grp_offset:");
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
           temp=gRfiuParm_Rx[RxUnit].PktMap[i*2]+gRfiuParm_Rx[RxUnit].PktMap[i*2+1];
           if(temp !=0)
           {
               if(gRfiuParm_Tx[TxUnit].Pkt_Grp_offset[i] != gRfiuParm_Rx[RxUnit].Pkt_Grp_offset[i])
               {
                  err=1;
                  DEBUG_RFIU_P("Pkt_Grp_offset FAIL:0x%x,0x%x\n",gRfiuParm_Tx[TxUnit].Pkt_Grp_offset[i],gRfiuParm_Rx[RxUnit].Pkt_Grp_offset[i]);
               }
           }
        }
        DEBUG_RFIU("PASS\n");
        //---//
        DEBUG_RFIU("---->Check Packet User data L:");
        if(gRfiuParm_Tx[TxUnit].UserData_L != gRfiuParm_Rx[RxUnit].UserData_L)
        {
            err=1;
            DEBUG_RFIU_P("User_data_L FAIL\n");
        }
        DEBUG_RFIU("PASS\n");
        //---//
        DEBUG_RFIU("---->Check User Data 1:");
        if(gRfiuParm_Tx[TxUnit].UserData_H != gRfiuParm_Rx[RxUnit].UserData_H)
        {
            err=1;
            DEBUG_RFIU_P("User_data_H FAIL\n");
        }
        DEBUG_RFIU("PASS\n");
        //---//
        if(CheckPktBurstNum)
        {
            DEBUG_RFIU_P2("---->Check TxRxPktNum: %d/%d,\n",gRfiuParm_Rx[RxUnit].TxRxPktNum,gRfiuParm_Tx[TxUnit].TxRxPktNum);
            if(gRfiuParm_Tx[TxUnit].TxRxPktNum!= gRfiuParm_Rx[RxUnit].TxRxPktNum)
            {
                err=1;
                DEBUG_RFIU_P("TxRxPktNum FAIL\n");
            }
            DEBUG_RFIU("PASS\n");
        }
        else
        {
           DEBUG_RFIU_P2("---->Check TxRxPktNum: %d/%d\n",gRfiuParm_Rx[RxUnit].TxRxPktNum,gRfiuParm_Tx[TxUnit].TxRxPktNum);
        }
        //----------Compare Data---------//
        for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
        {
            temp=gRfiuParm_Rx[RxUnit].PktMap[i*2]+gRfiuParm_Rx[RxUnit].PktMap[i*2+1];
            if(temp !=0)
            {
               DEBUG_RFIU("---->Check PacketData Group-%d,map1:",i);
               map1=gRfiuParm_Rx[RxUnit].PktMap[i*2];
               if(map1 != 0)
               {
                   for(j=0;j<32;j++)
                   {
                      RxAddr=gRfiuParm_Rx[RxUnit].TxRxOpBaseAddr + gRfiuParm_Rx[RxUnit].Pkt_Grp_offset[i] + j*128;
                      TxAddr=gRfiuParm_Tx[TxUnit].TxRxOpBaseAddr + gRfiuParm_Tx[TxUnit].Pkt_Grp_offset[i] + j*128;
                      if((map1>>j) & 0x01)
                      {
                          for(k=0;k<128;k++)
                          {
                              if(RxAddr[k] != TxAddr[k] )
                              {
                                  err=1;
                                  DEBUG_RFIU_P("Data FAIL_1:%d,%d\n",j,k);
                                  return 0;
                              }
                          }

                      }
                      #if 0
                      else
                      {
                          for(k=0;k<128;k++)
                          {
                              if(RxAddr[k] != 0x0 )
                              {
                                  err=1;
                                  DEBUG_RFIU_P("Data FAIL_2:%d,%d\n",j,k);
                                  return 0;
                              }
                          }
                      }
                      #endif
                   }
               }
               DEBUG_RFIU("PASS\n");
               //---//
               DEBUG_RFIU("---->Check PacketData Group-%d,map2:",i);
               map2=gRfiuParm_Rx[RxUnit].PktMap[i*2+1];
               if(map2 !=0 )
               {
                   for(j=0;j<32;j++)
                   {                  
                      RxAddr=gRfiuParm_Rx[RxUnit].TxRxOpBaseAddr + gRfiuParm_Rx[RxUnit].Pkt_Grp_offset[i] + (j+32)*128;
                      TxAddr=gRfiuParm_Tx[TxUnit].TxRxOpBaseAddr + gRfiuParm_Tx[TxUnit].Pkt_Grp_offset[i] + (j+32)*128;
                      
                      if((map2>>j) & 0x01)
                      {
                          for(k=0;k<128;k++)
                          {
                              if(RxAddr[k] != TxAddr[k] )
                              {
                                  err=1;
                                  DEBUG_RFIU_P("Data FAIL_3:%d,%d\n",j,k);
                                  return 0;
                              }
                          }
                      }
                      #if 0
                      else
                      {
                          for(k=0;k<128;k++)
                          {
                              if(RxAddr[k] != 0x0 )
                              {
                                  err=1;
                                  DEBUG_RFIU_P("Data FAIL_4:%d,%d\n",j,k);
                                  return 0;
                              }
                          }
                      }
                      #endif
                   }
               }
               DEBUG_RFIU("PASS\n");
            }
        }

        if(err)
            return 0;


        return 1;
    }

    



    int marsRfiu_Test()
    {
        int status;
        int i;

        AmicReg_Data=0;
        AmicReg_Addr=0;
        AmicReg_RWen1=0;
        AmicReg_RWen2=0;

    //-----//
    #if DEBUG_MAP_PKTNUM
       GpioActFlashSelect |= 0x20000;
    #endif

    #if RFI_MEASURE_RX1RX2_SENSITIVITY
        DEBUG_RFIU_P("------>marsRfiu_measure_2Rx sensitivity-----\n");
        status = marsRfiu_Measure_RX1RX2_Sensitivity(0xfffffff, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
    #endif

    #if RFI_FCC_DIRECT_TRX
        status = marsRfiu_FCC_DirectTXRX(0);
    #endif

    #if (RFI_TEST_PKTBURST || RFI_TEST_PKTMAP || RFI_TEST_PERFORMANCE || RFI_TEST_TXRX_COMMU)
        while(1)
        {
            //----------------------------//		 
            #if RFI_TEST_PKTBURST
                DEBUG_RFIU_P("------>marsRfiu_Test_PktBurst-----\n");
                status = marsRfiu_Test_PktBurst(128, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
                if(status==0)  return 0;            
            #endif
            //---------------------------//
            #if RFI_TEST_PKTMAP
                DEBUG_RFIU_P("------>marsRfiu_Test_PktMap-----\n");
                status=marsRfiu_Test_PktMap(256);
                if(status==0) return 0;
            #endif
            //---------------------------//
            #if RFI_TEST_PERFORMANCE
                DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_DISA,RFI_RS_T2,RFI_VITBI_CR1_2\n");
                status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
                if(status==0) return 0;
                
                DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_DISA,RFI_RS_T4,RFI_VITBI_CR1_2\n");
                status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_DISA, RFI_RS_T4, RFI_VITBI_CR1_2);
                if(status==0) return 0;

                DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_DISA,RFI_RS_T8,RFI_VITBI_CR1_2\n");
                status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_DISA, RFI_RS_T8, RFI_VITBI_CR1_2);
                if(status==0) return 0;

                DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_DISA,RFI_RS_T12,RFI_VITBI_CR1_2\n");
                status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_DISA, RFI_RS_T12, RFI_VITBI_CR1_2);
                if(status==0) return 0;
            #endif

              #if(RFI_TEST_TXRX_FUN==1)
                status = marsRfiu_Test_TxFunc(10000000, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
              #elif(RFI_TEST_TXRX_FUN==2)
                status = marsRfiu_Test_RxFunc(10000000, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
              #endif


            #if RFI_TEST_TXRX_COMMU
                for(i=0;i<RFI_TEST_TXRX_NUM;i++)
                {
                    DEBUG_RFIU_P2("\n---------------------RFIU Tx-RX Communication Test------------------------\n");
                #if RFI_TEST_WRAP_OnCOMMU
                    status = marsRfiu_Test_TxRxCommu(0x7fffffff,
                                                     gRfiuSyncWordTable[0],
                                                     0x1234);
                #else
                    status = marsRfiu_Test_TxRxCommu(300,
                                                     gRfiuSyncWordTable[0],
                                                     0x1234);
                #endif
                    if(status==0) return 0;
                }
            #endif
        }
     #endif
     
    #if RFI_SELF_TEST_TXRX_PROTOCOL
        OSTaskResume(RFIU_TASK_PRIORITY_UNIT1);
        OSTaskResume(RFIU_TASK_PRIORITY_UNIT0);
        #if RFI_TEST_WRAP_OnPROTOCOL
        OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
        #endif    
    #endif
    
        return 1;
    }

    int marsRfiu_Test_PktMap(unsigned int TestRun)
    {
        int testrun,i,j;
        unsigned int *pp,*qq;
        unsigned int err;
        unsigned int pktmap,count,temp;
        unsigned int VITBI_ONOFF[2]={RFI_VITBI_EN,RFI_VITBI_DISA};
        unsigned int VITBI_TYPE[4]={RFI_VITBI_CR1_2,RFI_VITBI_CR2_3,RFI_VITBI_CR3_4,RFI_VITBI_CR4_5};
        unsigned int RSCODE_TYPE[4]={RFI_RS_T2,RFI_RS_T4,RFI_RS_T8,RFI_RS_T12};
        u8 RSSI2;

        DEBUG_RFIU("----------------------------marsRfiu_Test Packet map: ---------------------------------\n");
        for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
        {
           gRfiuParm_Tx[RFI_UNIT_0].PktMap[i] = gRfiuTestPktMapTab[i];
           gRfiuParm_Tx[RFI_UNIT_1].PktMap[i] =  gRfiuTestPktMapTab2[i];
        }
        for(testrun=0;testrun<TestRun;testrun ++)
        {
            //==================(Unit-0 -->Unit-1)=================//
            DEBUG_RFIU_P("==================(Unit-0 -->Unit-1)=================\n");
            //Config Unit0:Tx, Unit1:Rx
            
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);

            //---Setup test environment---//
            pp=(unsigned int *)rfiuOperBuf[0];
            qq=(unsigned int *)rfiuOperBuf[1];
            for(i=0;i<RFIU_TESTCNTMAX_WD;i++)
            {
                *pp= *pp + i*100;
                *qq=0;
                pp ++;
                qq ++;
            }
            //---config RFU parameter---//
            //-Tx-//    
            gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en             =VITBI_ONOFF[(testrun>>4) & 0x01];

            if(gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en == RFI_VITBI_DISA )
            {
                gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =RFI_VITBI_CR1_2;   //hw limitation
                gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x03];
            }
            else
            {
                gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =VITBI_TYPE[(testrun>>2) & 0x03];
                if(gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel==RFI_VITBI_CR3_4)
                   gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x02];  //hw limitation
                else
                   gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x03];
            }
            gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;


            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_0].PktMap[i] +=  0x00010001;
            
            count =0;
            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
            {
                 pktmap=gRfiuParm_Tx[RFI_UNIT_0].PktMap[i];
                 for(j=0;j<32;j++)
                 {
                      if(pktmap & 0x01)
                          count ++;
                      pktmap >>=1;
                 }
            }
            if(count > RFI_RX_PACKET_NUM_INIT)
                count=RFI_RX_PACKET_NUM_INIT;
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =count;  //zero start
            if(count ==0)
                continue;
            DEBUG_RFIU_P2("---Packetnum=%d,Testrun=%d---\n",count,testrun);

            gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_0].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
                  
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en    =0;
            gRfiuParm_Tx[RFI_UNIT_0].SuperBurstMode_en     =0;
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID           =testrun & RFU_CUSTOMER_ID_MASK;
            
            gRfiuParm_Tx[RFI_UNIT_0].UserData_L            =0x5a5a & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_0].UserData_H            =testrun & RFI_USER_DATA_H_MASK;

            gRfiuParm_Tx[RFI_UNIT_0].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            //------Rx------//
            
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_0].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =0;
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID;
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;            
            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]),RFI_UNIT_1 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]),RFI_UNIT_0 );  //Tx
    
            rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
            #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_ON,CHECK_PKT_BURSTNUM_ON);
            #else
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
            #endif
               if(err==0)
                 return 0;
            }
            
            //==================(Unit-1 -->Unit-0)=================//

            //Config Unit0:Rx, Unit1:Tx
            
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);

            //---Setup test environment---//
            pp=(unsigned int *)rfiuOperBuf[0];
            qq=(unsigned int *)rfiuOperBuf[1];
            for(i=0;i<RFIU_TESTCNTMAX_WD;i++)
            {
                *pp= *pp + i*1000;
                *qq=0;
                pp ++;
                qq ++;
            }
            //---config RFU parameter---//
            //-Tx-//
       
            gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en             =VITBI_ONOFF[(testrun>>4) & 0x01];

            if(gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en == RFI_VITBI_DISA )
            {
                gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel       =RFI_VITBI_CR1_2;   //hw limitation
                gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x03];
            }
            else
            {
                gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel       =VITBI_TYPE[(testrun>>2) & 0x03];
                if(gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel==RFI_VITBI_CR3_4)
                   gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x02];  //hw limitation
                else
                   gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel        =RSCODE_TYPE[testrun & 0x03];
            }
            gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_1].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

        #if 0    
            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_1].PktMap[i] +=  0x00010001;

            count =0;
            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
            {
                 pktmap=gRfiuParm_Tx[RFI_UNIT_1].PktMap[i];
                 for(j=0;j<32;j++)
                 {
                      if(pktmap & 0x01)
                          count ++;
                      pktmap >>=1;
                 }
            }
            if(count > 256)
                count=256;
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           =256;  //zero start
        #else
            for(i=0;i<RFI_ACK_SYNC_PKTNUM ;i++)
            {
                gRfiuParm_Tx[RFI_UNIT_1].PktMap[2*i]      =RFI_ACK_ADDR_CHEKBIT;
                gRfiuParm_Tx[RFI_UNIT_1].PktMap[2*i+1]    =0x00000000;        
                gRfiuParm_Tx[RFI_UNIT_1].Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
            }
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           = RFI_ACK_SYNC_PKTNUM;  
            count = RFI_ACK_SYNC_PKTNUM;
        #endif    
        
            DEBUG_RFIU_P("=====(Unit-1 -->Unit-0:Sent %d Packet)====\n",count);
            DEBUG_RFIU_P("---Packetnum=%d,Testrun=%d---\n",count,testrun);

            
            gRfiuParm_Tx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_1].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
                             
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en    =0;
            gRfiuParm_Tx[RFI_UNIT_1].SuperBurstMode_en     =0;
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID           =testrun & RFU_CUSTOMER_ID_MASK;
            
            gRfiuParm_Tx[RFI_UNIT_1].UserData_L            =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_1].UserData_H            =testrun & RFI_USER_DATA_H_MASK;
            gRfiuParm_Tx[RFI_UNIT_1].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            
            gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_0].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_0].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_0].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_0].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_0].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_1].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_0].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_0].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID;
            
            gRfiuParm_Rx[RFI_UNIT_0].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_0].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            
            gRfiuParm_Rx[RFI_UNIT_0].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]),RFI_UNIT_0 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]),RFI_UNIT_1 );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_1,&gRfiuParm_Tx[RFI_UNIT_1]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_0,&gRfiuParm_Rx[RFI_UNIT_0],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
             #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_ON,CHECK_PKT_BURSTNUM_ON);
             #else
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
             #endif
               if(err==0)
                 return 0;
            }
        
        }
        return 1;
    }

    int marsRfiu_Test_PktBurst(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode)
    {
        int testrun,i;
        unsigned int *pp,*qq;
        unsigned int err;
        u8 RSSI2;
        
        DEBUG_RFIU("----------------------------marsRfiu_Test Burst mode:1~256---------------------------------\n");
        for(testrun=1;testrun<=TestRun;testrun ++)
        {
            //==================(Unit-0 -->Unit-1)=================//
            DEBUG_RFIU_P("==================(Unit-0 -->Unit-1)=================\n");
            
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
            //---Setup test environment---//
            pp=(unsigned int *)rfiuOperBuf[0];
            qq=(unsigned int *)rfiuOperBuf[1];
            for(i=0;i<RFIU_TESTCNTMAX_WD;i++)
            {
                *pp= *pp + i*1204;
                *qq=0;
                pp ++;
                qq ++;
            }
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_0].PktMap[i] = gRfiuTestPktMapTab[i];
            
       
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =testrun;  //zero start
            
            gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_0].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en   =0;
            gRfiuParm_Tx[RFI_UNIT_0].SuperBurstMode_en=0;
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =testrun & RFU_CUSTOMER_ID_MASK;
            
            gRfiuParm_Tx[RFI_UNIT_0].UserData_L           =0x5a5a & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_0].UserData_H           =testrun & RFI_USER_DATA_H_MASK;
            
            gRfiuParm_Tx[RFI_UNIT_0].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_0].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID;
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]),RFI_UNIT_1 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]),RFI_UNIT_0 );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
             #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_ON);
             #else
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
             #endif
               if(err==0)
                 return 0;
            }

            //==================(Unit-1 -->Unit-0)=================//
     #if 1       
            DEBUG_RFIU_P("==================(Unit-1 -->Unit-0)=================\n");
            //Config Unit0:Rx, Unit1:Tx
            
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
            //---Setup test environment---//
            pp=(unsigned int *)rfiuOperBuf[0];
            qq=(unsigned int *)rfiuOperBuf[1];
            for(i=0;i<RFIU_TESTCNTMAX_WD;i++)
            {
                *pp= *pp + i*111;
                *qq=0;
                pp ++;
                qq ++;
            }
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_1].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
            
        #if 1    
            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_1].PktMap[i]            =0xffffffff;
            
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           =testrun ;  //zero start
        #else
            for(i=0;i<RFI_ACK_SYNC_PKTNUM ;i++)
            {
                gRfiuParm_Tx[RFI_UNIT_1].PktMap[2*i]      =RFI_ACK_ADDR_CHEKBIT;
                gRfiuParm_Tx[RFI_UNIT_1].PktMap[2*i+1]    =0x00000000;        
                gRfiuParm_Tx[RFI_UNIT_1].Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
            }
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           = RFI_ACK_SYNC_PKTNUM;  
        #endif    
        
            gRfiuParm_Tx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_1].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
                  
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en    =0;
            gRfiuParm_Tx[RFI_UNIT_1].SuperBurstMode_en     =0;
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID           =testrun & RFU_CUSTOMER_ID_MASK;
            
            gRfiuParm_Tx[RFI_UNIT_1].UserData_L            =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_1].UserData_H            =testrun & RFI_USER_DATA_H_MASK;
            gRfiuParm_Tx[RFI_UNIT_1].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_0].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_0].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_0].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_0].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_0].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_1].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_0].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_0].SuperBurstMode_en    =0;
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID;
            
            gRfiuParm_Rx[RFI_UNIT_0].UserData_L            =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].UserData_H            =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_0].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]),RFI_UNIT_0 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]),RFI_UNIT_1);  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_1,&gRfiuParm_Tx[RFI_UNIT_1]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_0,&gRfiuParm_Rx[RFI_UNIT_0],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
             #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_ON);
             #else
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
             #endif
               if(err==0)
                 return 0;
            }
        #endif
            
        }

        return 1;

    }

    int marsRfiu_Test_Performance(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode)
    {
        int testrun,i;
        unsigned char *pp,*qq;
        unsigned int Tx0Count,Rx1Count;
        unsigned int SyncErrCnt,RsErrCnt;
        unsigned int err;
    #if RFI_FPGA_PERFORMANCE_MEASURE
        unsigned int Preamble_err,Trailer_err,SyncWord_err,RS_err,CRC_err,err1,err2,ID_err;
    #endif
        u8 RSSI2;
        //-----------------//
        Tx0Count=0;
        Rx1Count=0;
        SyncErrCnt=0;
        RsErrCnt=0;
        testrun=0;

        for(testrun=0;testrun<TestRun;testrun ++)
        {
            //---Setup test environment---//
            pp=(unsigned char *)rfiuOperBuf[0];
            qq=(unsigned char *)rfiuOperBuf[1];

            for(i=0;i<RFIU_TESTCNTMAX_WD*4;i++)
            {
            #if 1
                *pp= *pp + i*1343;
            #else
                *pp = i % 128;
            #endif
                *qq=0;
                pp ++;
                qq ++;
            }
            //==================(Unit-0 -->Unit-1)=================//
        #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(1,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
            A7130_CH_sel(2,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(1,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
            A7196_CH_sel(2,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
        #elif(RFIC_SEL==RFIC_NONE_5M)
            RFNONE_CH_sel(1,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
            RFNONE_CH_sel(2,gRfiuDAT_CH_Table[testrun % RFI_DAT_CH_MAX]);
        #endif

        #if 1
            if(AmicReg_RWen1)
            {
               if(AmicReg_RWen1 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }

               AmicReg_RWen1=0;
            }    
            
            if(AmicReg_RWen2)
            {
               if(AmicReg_RWen2 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif

#if 0      
            DEBUG_RFIU("==================(Unit-0 -->Unit-1)=================\n");
        
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
            
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
            gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_0].PktMap[i]            =0xffffffff;
            
      
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;  
            Tx0Count += gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum;           
            
            gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_0].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en   =0;
            gRfiuParm_Tx[RFI_UNIT_0].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =0x4321 & RFU_CUSTOMER_ID_MASK;
         #else   
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
         #endif
            
            gRfiuParm_Tx[RFI_UNIT_0].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_0].UserData_H        =testrun & RFI_USER_DATA_H_MASK;

            gRfiuParm_Tx[RFI_UNIT_0].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_0].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =0x1234 & RFU_CUSTOMER_ID_MASK;
         #else
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID;
         #endif
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]),RFI_UNIT_1 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]),RFI_UNIT_0 );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
            #if DEBUG_CID_ERR
            #endif

             #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_ON,CHECK_PKT_BURSTNUM_ON);
             #else
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
             #endif
               if(err==0)
                 return 0;
            }

            Rx1Count += gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum;
        #if RFI_FPGA_PERFORMANCE_MEASURE
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_1));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_1));
            CRC_err=(err1>>16) & 0x0ff;
            RS_err=(err1>>24) & 0x0ff;
            SyncWord_err=(err1>>8) & 0x0ff;
            Trailer_err=(err1) & 0x0ff;
            ID_err=err2 & 0xff;
            
            Preamble_err=(int)(gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum - gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum - CRC_err - RS_err);
            SyncErrCnt +=Preamble_err;
            RsErrCnt += (CRC_err + RS_err);
            
            DEBUG_RFIU_P("--->Err Cond. SyncErr=%d,RS=%d,CRC=%d,ID=%d\n",Preamble_err,RS_err,CRC_err,ID_err);
        #endif
#else		
            DEBUG_RFIU("==================(Unit-1 -->Unit-0)=================\n");
        
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
            //---Setup test environment---//
            pp=(unsigned char *)rfiuOperBuf[0];
            qq=(unsigned char *)rfiuOperBuf[1];

        
            for(i=0;i<RFIU_TESTCNTMAX_WD*4;i++)
            {
            #if 1
                *pp= *pp + i*1343;
            #else
                *pp = i % 128;
            #endif
                *qq=0;
                pp ++;
                qq ++;
            }
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_1].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
            gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_1].PktMap[i]            =0xffffffff;
            
      
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           =256;  
            Tx0Count += gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum;           
            
            gRfiuParm_Tx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_1].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en   =0;
            gRfiuParm_Tx[RFI_UNIT_1].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID          =0x4321 & RFU_CUSTOMER_ID_MASK;
         #else   
            gRfiuParm_Tx[RFI_UNIT_1].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
         #endif
            
            gRfiuParm_Tx[RFI_UNIT_1].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_1].UserData_H        =testrun & RFI_USER_DATA_H_MASK;

            gRfiuParm_Tx[RFI_UNIT_1].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_0].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_1].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_0].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_1].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_0].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_1].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_0].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_1].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_0].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_1].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_0].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_1].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_0].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID          =0x1234 & RFU_CUSTOMER_ID_MASK;
         #else
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_1].Customer_ID;
         #endif
            
            gRfiuParm_Rx[RFI_UNIT_0].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_0].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_0].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_1].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
            gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]),RFI_UNIT_0 );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]),RFI_UNIT_1 );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_1,&gRfiuParm_Tx[RFI_UNIT_1]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_0,&gRfiuParm_Rx[RFI_UNIT_0],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Packet received!!\n");
               //return 0;
            }
            else
            {
            #if DEBUG_CID_ERR
            #endif

             #if DEBUG_MAP_PKTNUM
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_ON,CHECK_PKT_BURSTNUM_ON);
             #else
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
             #endif
               if(err==0)
                 return 0;
            }

            Rx1Count += gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum;
        #if RFI_FPGA_PERFORMANCE_MEASURE
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_0));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_0));
            CRC_err=(err1>>16) & 0x0ff;
            RS_err=(err1>>24) & 0x0ff;
            SyncWord_err=(err1>>8) & 0x0ff;
            Trailer_err=(err1) & 0x0ff;
            ID_err=err2 & 0xff;
            
            Preamble_err=(int)(gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum - gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum - CRC_err - RS_err);
            SyncErrCnt +=Preamble_err;
            RsErrCnt += (CRC_err + RS_err);
            
            DEBUG_RFIU_P("--->Err Cond. SyncErr=%d,RS=%d,CRC=%d,ID=%d\n",Preamble_err,RS_err,CRC_err,ID_err);
        #endif        
#endif	
        }

        
        DEBUG_RFIU_P("Correction Rate--->%d/%d=%d(x10000)        \n",Rx1Count,Tx0Count,Rx1Count*10000/Tx0Count);
     #if RFI_FPGA_PERFORMANCE_MEASURE
        if(SyncErrCnt+RsErrCnt == 0)
          DEBUG_RFIU_P("Err condition--->%d:%d=%d(x10000)        \n",SyncErrCnt,RsErrCnt,(SyncErrCnt+RsErrCnt));
        else
          DEBUG_RFIU_P("Err condition--->%d:%d=%d(x10000)        \n",SyncErrCnt,RsErrCnt,SyncErrCnt*10000/(SyncErrCnt+RsErrCnt));
     #endif
        return 1;


    }


    int marsRfiu_Test_TxFunc(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode)
    {
        int testrun,i;
        unsigned char *pp,*qq;
        unsigned int Tx0Count,Rx1Count;
        unsigned int SyncErrCnt,RsErrCnt;
        unsigned int err;
        u8 RSSI2;
        //-----------------//
        Tx0Count=0;
        Rx1Count=0;
        SyncErrCnt=0;
        RsErrCnt=0;

        //---Setup test environment---//
        pp=(unsigned char *)rfiuOperBuf[0];
        qq=(unsigned char *)rfiuOperBuf[1];

    
        for(i=0;i<RFIU_TESTCNTMAX_WD*4;i++)
        {
        #if 1
            *pp= *pp + i*1343;
        #else
            *pp = i % 128;
        #endif
            *qq=0;
            pp ++;
            qq ++;
        }

        DEBUG_RFIU_P2("-----marsRfiu_Test_TxFunc-------\n");
        for(testrun=0;testrun<TestRun;testrun ++)
        {
        #if 1
            if(AmicReg_RWen1)
            {
               if(AmicReg_RWen1 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }

               AmicReg_RWen1=0;
            }    
            
            if(AmicReg_RWen2)
            {
               if(AmicReg_RWen2 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif

            DEBUG_RFIU_P2("TX ");        
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
            gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_0].PktMap[i]            =0xffffffff;
            
      
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =128;  
            Tx0Count += gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum;           
            
            gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_0].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en   =0;
            gRfiuParm_Tx[RFI_UNIT_0].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =0x4321 & RFU_CUSTOMER_ID_MASK;
         #else   
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
         #endif
            
            gRfiuParm_Tx[RFI_UNIT_0].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_0].UserData_H        =testrun & RFI_USER_DATA_H_MASK;

            gRfiuParm_Tx[RFI_UNIT_0].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_0].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =0x1234 & RFU_CUSTOMER_ID_MASK;
         #else
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID;
         #endif
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]),RFI_UNIT_0 );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
        }

        
        return 1;


    }

    int marsRfiu_Test_RxFunc(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode)
    {
        int testrun,i;
        unsigned char *pp,*qq;
        unsigned int Tx0Count,Rx1Count;
        unsigned int SyncErrCnt,RsErrCnt;
        unsigned int err;
    #if RFI_FPGA_PERFORMANCE_MEASURE
        unsigned int Preamble_err,Trailer_err,SyncWord_err,RS_err,CRC_err,err1,err2,ID_err;
    #endif
        u8 RSSI2;
        //-----------------//
        Tx0Count=0;
        Rx1Count=0;
        SyncErrCnt=0;
        RsErrCnt=0;

        DEBUG_RFIU_P2("-----marsRfiu_Test_RxFunc-------\n");

        for(testrun=0;testrun<TestRun;testrun ++)
        {
        #if 1
            if(AmicReg_RWen1)
            {
               if(AmicReg_RWen1 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B1(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }

               AmicReg_RWen1=0;
            }    
            
            if(AmicReg_RWen2)
            {
               if(AmicReg_RWen2 == 1)
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  A7130_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  A7196_WriteReg_B2(AmicReg_Addr, AmicReg_Data);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #elif(RFIC_SEL==RFIC_NONE_5M)
               
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif

            //DEBUG_RFIU_P2("RX ");        
        
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            
            //---config RFU parameter---//
            //-Tx-//
            gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en             =Vitbi_en;
            gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel        =RS_mode;
            gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel       =Vitbi_mode;
            
            gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Tx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;     //unit:2
            gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord          =gRfiuSyncWordTable[0];
            
            gRfiuParm_Tx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Tx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Tx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Tx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            for(i=0;i<RFIU_USEPKTMAP_MAX;i++)
               gRfiuParm_Tx[RFI_UNIT_0].PktMap[i]            =0xffffffff;
            
      
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =128;  
            Tx0Count += gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum;           
            
            gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];

            for(i=0;i<RFIU_USEPKTMAP_MAX/2;i++)
               gRfiuParm_Tx[RFI_UNIT_0].Pkt_Grp_offset[i]    =64*RFIU_PKT_SIZE*i;
            
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en   =0;
            gRfiuParm_Tx[RFI_UNIT_0].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =0x4321 & RFU_CUSTOMER_ID_MASK;
         #else   
            gRfiuParm_Tx[RFI_UNIT_0].Customer_ID          =(testrun+12345) & RFU_CUSTOMER_ID_MASK;
         #endif
            
            gRfiuParm_Tx[RFI_UNIT_0].UserData_L           =testrun & RFI_USER_DATA_L_MASK;
            gRfiuParm_Tx[RFI_UNIT_0].UserData_H        =testrun & RFI_USER_DATA_H_MASK;

            gRfiuParm_Tx[RFI_UNIT_0].SyncWordLenSel        =RFI_SYNCWORDLEN_SEL;
            
            //------Rx------//
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =gRfiuParm_Tx[RFI_UNIT_0].Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =gRfiuParm_Tx[RFI_UNIT_0].CovCodeRateSel;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =gRfiuParm_Tx[RFI_UNIT_0].RsCodeSizeSel;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =gRfiuParm_Tx[RFI_UNIT_0].DummyDataNum;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =gRfiuParm_Tx[RFI_UNIT_0].PreambleNum;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =gRfiuParm_Tx[RFI_UNIT_0].DummyPreambleNum;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID_ext_en;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =RFI_SUPERBURST_MODESEL;
         #if DEBUG_CID_ERR  
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =0x1234 & RFU_CUSTOMER_ID_MASK;
         #else
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =gRfiuParm_Tx[RFI_UNIT_0].Customer_ID;
         #endif
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =gRfiuParm_Tx[RFI_UNIT_0].PktSyncWord;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;
            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]),RFI_UNIT_1 );  //Rx
            //rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]),RFI_UNIT_0 );  //Tx

            //rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
            rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx

            
        #if RFI_FPGA_PERFORMANCE_MEASURE
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_1));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_1));
            CRC_err=(err1>>16) & 0x0ff;
            RS_err=(err1>>24) & 0x0ff;
            SyncWord_err=(err1>>8) & 0x0ff;
            Trailer_err=(err1) & 0x0ff;
            ID_err=err2 & 0xff;
            
            Preamble_err=(int)(gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum - gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum - CRC_err - RS_err);
            SyncErrCnt +=Preamble_err;
            RsErrCnt += (CRC_err + RS_err);
          #if 0  
            DEBUG_RFIU_P("--->%d/%d: SyncErr=%d,RS=%d,CRC=%d,ID=%d\n",
                         gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum,gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum,
                         Preamble_err,RS_err,CRC_err,ID_err);
          #else
            DEBUG_RFIU_P("->%d/%d\n",
                         gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum,gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum);
          #endif
        #endif

        }

        
        return 1;


    }  



    int marsRfiu_Test_TxRxCommu(unsigned int TestRunCount, 
                                             unsigned int SyncWord,
                                             unsigned int CustomerID
                                           )
    {}

    int marsRfiu_Measure_RX1RX2_Sensitivity(unsigned int TestRun, unsigned int Vitbi_en, unsigned int RS_mode, unsigned int Vitbi_mode)
    {
        int testrun,i;
        unsigned int *pp,*qq;
        unsigned int err;
        u8 RSSI2;
        
        DEBUG_RFIU("----------------------------marsRfiu_Measure_RX1RX2_Sensitivity---------------------------------\n");

    #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(1,0);
        MV400_CH_sel(2,0);
    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(1,100);
        A7130_CH_sel(2,100);

        A7130_ID_Update(1 ,0x17df83ff );
        A7130_ID_Update(2 ,0x17df83ff );
    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(1,100);
        A7196_CH_sel(2,100);

        A7196_ID_Update(1 ,0x17df83ff );
        A7196_ID_Update(2 ,0x17df83ff );
    #elif(RFIC_SEL==RFIC_NONE_5M)
        RFNONE_CH_sel(1,100);
        RFNONE_CH_sel(2,100);

        RFNONE_ID_Update(1 ,0x17df83ff );
        RFNONE_ID_Update(2 ,0x17df83ff );
    #endif		
	
        for(testrun=1;testrun<=TestRun;testrun ++)
        {            
            RfiuReset(RFI_UNIT_0);
            RfiuReset(RFI_UNIT_1);
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
            
            //---config RFU parameter---//
			//------Rx1------//
            gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr       =rfiuOperBuf[0];
            gRfiuParm_Rx[RFI_UNIT_0].Vitbi_en             =Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_0].CovCodeRateSel       =Vitbi_mode;
            gRfiuParm_Rx[RFI_UNIT_0].RsCodeSizeSel        =RS_mode;
            
            gRfiuParm_Rx[RFI_UNIT_0].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Rx[RFI_UNIT_0].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_0].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID_ext_en   =0;
            gRfiuParm_Rx[RFI_UNIT_0].SuperBurstMode_en    =0;
            gRfiuParm_Rx[RFI_UNIT_0].Customer_ID          =0x1234;
            
            gRfiuParm_Rx[RFI_UNIT_0].UserData_L            =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].UserData_H            =0x0;
            gRfiuParm_Rx[RFI_UNIT_0].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_0].PktSyncWord          =0x17df83ff;
            
            gRfiuParm_Rx[RFI_UNIT_0].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_0].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_0].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_0].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //------Rx2------//
            gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr       =rfiuOperBuf[1];
            gRfiuParm_Rx[RFI_UNIT_1].Vitbi_en             =Vitbi_en;
            gRfiuParm_Rx[RFI_UNIT_1].CovCodeRateSel       =Vitbi_mode;
            gRfiuParm_Rx[RFI_UNIT_1].RsCodeSizeSel        =RS_mode;
            
            gRfiuParm_Rx[RFI_UNIT_1].DummyDataNum         =RFI_TX_DUMMY_DATA_NUM;
            gRfiuParm_Rx[RFI_UNIT_1].PreambleNum          =RFI_TX_PREAMBLE_NUM;  //unit:2
            gRfiuParm_Rx[RFI_UNIT_1].DummyPreambleNum     =RFI_DUMMY_PREAMBLE_NUM;  //unit:2
            
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID_ext_en   =0;
            gRfiuParm_Rx[RFI_UNIT_1].SuperBurstMode_en    =0;
            gRfiuParm_Rx[RFI_UNIT_1].Customer_ID          =0x1234;
            
            gRfiuParm_Rx[RFI_UNIT_1].UserData_L           =0x0;
            gRfiuParm_Rx[RFI_UNIT_1].UserData_H           =0x0;

            gRfiuParm_Rx[RFI_UNIT_1].SyncWordLenSel       =RFI_SYNCWORDLEN_SEL;
            gRfiuParm_Rx[RFI_UNIT_1].PktSyncWord          =0x17df83ff;
            
            gRfiuParm_Rx[RFI_UNIT_1].TxClkConfig          =RFI_TXCLKCONFIG;
            gRfiuParm_Rx[RFI_UNIT_1].RxClkAdjust          =RFI_RXCLKADJUST;
            gRfiuParm_Rx[RFI_UNIT_1].DclkConfig           =RFI_DCLKCONF;
            gRfiuParm_Rx[RFI_UNIT_1].RxTimingFineTune     =RFI_RXTIMINGFINETUNE;

            gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum           =RFI_RX_PACKET_NUM_INIT;

            //----//
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]),RFI_UNIT_0 );  //Rx
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]),RFI_UNIT_1 );  //Rx
            while(1)
            {
               OSTimeDly(1);
			}

            rfiuWaitForInt_Rx(RFI_UNIT_0,&gRfiuParm_Rx[RFI_UNIT_0],0,&RSSI2);  //Rx
            rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
            //----//
            if(gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->RFI_UNIT_1 No Packet received!!\n");
            }

			if(gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->RFI_UNIT_0 No Packet received!!\n");
            }
        }

        return 1;

    }



#endif


