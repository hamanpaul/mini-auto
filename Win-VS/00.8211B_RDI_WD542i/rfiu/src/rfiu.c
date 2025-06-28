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

#if RF_TX_OPTIMIZE
#include "ciuapi.h"
#endif


/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */
#if(RFIC_SEL  ==  RFIC_A7196_6M)
#define RFI_RX_PACKET_NUM_INIT         192   //Lucian: 預設RX time-out (packet unit)
#else
#define RFI_RX_PACKET_NUM_INIT         128   //Lucian: 預設RX time-out (packet unit)
#endif

#if (RFIU_USEPKTMAP_MAX > 8 )
  #define RFI_ACK_SYNC_PKTNUM     6
  #define RFI_ACK_PAIR_PKTNUM     6  
  #define RFI_ACK_WAKE_PKTNUM     64
#else
  #define RFI_ACK_SYNC_PKTNUM     4
  #define RFI_ACK_PAIR_PKTNUM     4   
  #define RFI_ACK_WAKE_PKTNUM     64  
#endif

 #if RFI_SELF_TEST_TXRX_PROTOCOL
     #define RFI_RX_WAIT_TIME   64
     #define RFI_TX_WAIT_TIME   64

 #elif(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 ||RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2)
    #if(RFIC_SEL  ==  RFIC_A7196_6M)
     #define RFI_RX_WAIT_TIME   48  //32
     #define RFI_TX_WAIT_TIME   48
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
#define RXCMD_KEEP_CHECK       0x0004
//--------------//
#define RXACK_NORMAL           0x0000
#define RXACK_KEEP             0x0001

#define RFIU_TESTCNTMAX_WD   (1024*128/4)   //4


#if RFIU_RX_AUDIO_RETURN
  #if( (SW_APPLICATION_OPTION  ==  MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
   #define RX_AUDIORET_RETRYNUM      6
  #else
   #define RX_AUDIORET_RETRYNUM      6
  #endif
#endif

#define RF1_ACKTIMESHIFT      8000   //800 ms=8000*100 us

#define RF_FIXCH_OPTIM        1

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
#if TX_SNAPSHOT_SUPPORT 
DEF_RFIU_TXSNAPSHOTCHK rfiuTXSnapCheck;
#endif

u32 rfiuRSSI_CH_Avg[MAX_RFIU_UNIT][RFI_DAT_CH_MAX];

OS_STK rfiuTaskStack_Unit0[RFIU_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuTaskStack_Unit1[RFIU_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuTaskStack_Unit2[RFIU_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuTaskStack_Unit3[RFIU_TASK_STACK_SIZE_UNIT3]; 

OS_FLAG_GRP  *gRfiuFlagGrp;
OS_FLAG_GRP  *gRfiu_nTx1RSwFlagGrp;  //for n-Tx vs 1R 
OS_FLAG_GRP  *gRfiuStateFlagGrp;
OS_FLAG_GRP  *gRf868EventFlagGrp;

OS_EVENT    *gRfiuSWReadySemEvt;

OS_EVENT     *gRfiuReqSem[MAX_RFIU_UNIT];
OS_EVENT     *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];


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


#if (RF_PAIR_EN)
unsigned int Temp_TX_MAC_address[MAX_RFIU_UNIT];
unsigned int Temp_TX_CostomerCode[MAX_RFIU_UNIT];

unsigned int Temp_RX_MAC_Address[MAX_RFIU_UNIT];  
unsigned int Temp_RX_CostomerCode[MAX_RFIU_UNIT];

unsigned int MACadrsSetflag[MAX_RFIU_UNIT] ;
#endif

#if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
  #if(RFI_ACK_CH_MAX == 16)
      int gRfiuACK_CH_Table[4][16]={
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136}
                                   };
  #else
      int gRfiuACK_CH_Table[4][16]={
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                   };
  #endif
    int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136};
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
  #if(RFI_ACK_CH_MAX == 16)
      int gRfiuACK_CH_Table[4][16]={
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136}
                                   };
  #else
      int gRfiuACK_CH_Table[4][16]={
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                     { 10, 30, 50, 70, 90,110,130,140, 20, 40, 60, 80, 90,100,120, 10},
                                   };
  #endif
    int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136};
#elif((RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
    int gRfiuACK_CH_Table[4][16]={
                                    {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},
                                    {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},
                                    {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},
                                    {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},
                                 };
    int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX]={0,1,2,3,
                                           4,5,6,7,
                                           8,0,1,2,
                                           3,4,5,6};
#else
    int gRfiuACK_CH_Table[4][16]={
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136},
                                     {16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136}
                                 };

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
const int gRfiuRSCodeLens[4]={4,8,16,24};
const int gRfiuVitbiLensX12[4]={15,16,18,24};



#if(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 ||RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_RXRX_PROTOCOL_B1B2)
	 int gRfiuSyncWordTable[MAX_RF_DEVICE]={ 
	                                         0x87654321,
							                 0x12345678, 	
	                                         0xabcdef01,
	                                         0xfedcba98
	                                       };
#else
     int gRfiuSyncWordTable[MAX_RF_DEVICE]={ 
                                             0xbac57536,
                                             0xbac57536,
                                             0xbac57536,
                                             0xbac57536
                                           };
#endif

  int gRfiuCustomerCode[MAX_RF_DEVICE];

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
                                      { 0, 1, 2, 3, 4, 5, 6,99, 4, 5, 6, 0, 1, 2, 3, 99}
                                    };
//char RandHoppTab[RFI_DAT_CH_MAX]={ 9,10,11,12,13,14,15,12,13,14,15,9,10,11,12, 10};

unsigned int FCC_Unit_Sel;
int rfiuDoorBellTrig;

u8 AmicReg_Data,AmicReg_Addr,AmicReg_RWen1,AmicReg_RWen2;

#if (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
u8 rfiuVoxEna[MAX_RF_DEVICE]={1,1,1,1};
u8 rfiuVoxThresh[MAX_RF_DEVICE]={60,60,60,60}; /*20151211 modify*/
#elif(HW_BOARD_OPTION == MR8211_TX_RDI_SEP)
u8 rfiuVoxEna[MAX_RF_DEVICE]={1,1,1,1};
    #if ((UI_PROJ_OPT == 1) || (UI_PROJ_OPT == 2))
    u8 rfiuVoxThresh[MAX_RF_DEVICE]={98,98,98,98};
    #else
    u8 rfiuVoxThresh[MAX_RF_DEVICE]={50,50,50,50};
    #endif
#elif (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)
u8 rfiuVoxEna[MAX_RF_DEVICE]={1,1,1,1};
u8 rfiuVoxThresh[MAX_RF_DEVICE]={98,98,98,98};
#else
u8 rfiuVoxEna[MAX_RF_DEVICE]={0,0,0,0};
u8 rfiuVoxThresh[MAX_RF_DEVICE]={100,100,100,100};
#endif
u8 rfiuVoxTrigFlag = 0;
u32 rfiuVoxTrigLev=0;

#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
u8 Melody_Last_play=0;
#endif

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
u8 rfiuVocDustMode = 0; /*0: Normal, 1: Voc Test Mode, 2: Dust Test Mode*/
#endif

#if( (HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I) )
u8  nMotorTime=0;
#endif

#if (HW_BOARD_OPTION == MR8200_RX_RDI_M706 && PROJ_OPT == 8)
char* TxMdSensTableForOldCL692 = "CL692-FN-V0.15-150803 ";
#endif

#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692 && UI_PROJ_OPT == 0)
u8 prevPIRTrigWLED = 0;
#endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
extern u8 sysEnSnapshot;
extern u8  uiEnterScanMode;
#endif
 
extern int ciu1ZoomStart;
extern int ciu1ZoomOnOff;
extern int ciu1ZoomXpos;
extern int ciu1ZoomYpos;

extern int MDTxNewVMDSupport;
extern s32 MD_Diff;
extern u8  MotionDetect_en;

extern u32 guiRFTimerID;
extern u32 guiSysTimerId;
extern u32 guiIRTimerId;

extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern u32 guiIISPlayDMAId;
extern u32 guiIISRecDMAId;
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
extern s32 rfiuciu1ZoomIn(int OnOff,int Xpos,int Ypos);
extern s32 spiWriteRF(void);
extern void rfiuAudioRet_RecDMA_ISR(int dummy);


#if MOTOR_EN
  #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        //NULL
  #else
    extern u8  MotorStatusH;   // 0: 停止, 1: 正轉, 2: 負轉
    extern u8  MotorStatusV;   // 0: 停止, 1: 正轉, 2: 負轉
  #endif
    extern u8  nMotorTime;
    extern u8  uiIsVM9710;
#endif


#if (Melody_SNC7232_ENA)
extern u8 Melody_play_num;
#endif
#if INSERT_NOSIGNAL_FRAME
static u8 Rx_status[MULTI_CHANNEL_MAX]={0,0,0,0};
extern u8 Record_flag[MULTI_CHANNEL_MAX];
#endif
#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)) 
extern u8 uiNightVision;
#endif
#if ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
extern s8    uiVoxChannel;
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 
int rfiu_MaskWifiUsedCH(int WifiCH,int CH_Mask[]);
 
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
int marsRfiu_FCC_DirectTXRX(void);

int marsRfiu_Test_TxRxCommu(  unsigned int TestRun, 
                                           unsigned int SyncWord,
                                           unsigned int CustomerID);

void rfiu_Tx_Task_UnitX(void* pData);
void rfiu_Rx_Task_UnitX(void* pData);

int rfiuCalBufRemainCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
int rfiuCalVideoDisplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
int rfiuCalAudioplayBufCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);

int rfiu_nTnR_FindNextTx(int RFUnit);

#if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
int rfiu_DualMode_FindNextTx_Single(int RFUnit);
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


#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
void InitA7196();
void A7196_TxMode_Start(int BoardSel);
void A7196_RxMode_Start(int BoardSel);
void A7196_TxMode_Stop(int BoardSel);
void A7196_RxMode_Stop(int BoardSel);

void A7196_CH_sel(int BoardSel,BYTE CH);

void A7196_ID_Update(int BoardSel ,unsigned int NewMACID );
u8 RSSI_measurement_A7196(int BoardSel);

#endif

//----------------------------------------------//
s32 RfiuInit(void)
{
    unsigned char err;
    int i,j;
    unsigned int *pp;
    u8 level;
    u32 ID,temp;
    //-------------------//
    timerCountRead(guiRFTimerID, &ID);
    timerCountRead(guiSysTimerId, &temp);
    ID ^= temp;
    timerCountRead(guiIRTimerId, &temp);
    ID ^= temp;
    ID ^= uiRFID[0];
    //-------------------//
    sysRFRxInMainCHsel = 0;
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
#if TX_SNAPSHOT_SUPPORT 
    rfiuTXSnapCheck.ReCheck=0;
    rfiuTXSnapCheck.SnapStatus=RF_TXPHOTOSTA_NONE;
    rfiuTXSnapCheck.SnapTimeInMin=60*30;
    rfiuTXSnapCheck.CheckCnt=0;
#endif

    
#if(HW_BOARD_OPTION  == MR8120_TX_HECHI)
    gpioGetLevel(1, 9, &level);
    if(level==0)
    {
       rfiuDoorBellTrig=0;
       DEBUG_RFIU_P2("--Wake up Trig--\n");
    }
    else
    {
       rfiuDoorBellTrig=1;
       DEBUG_RFIU_P2("-- Doorbell Trig--\n");
    }

#else
    rfiuDoorBellTrig=0;
#endif
    rfiuAudioRetFromApp=0;
    rfiuAudioRetStatus=RF_AUDIO_RET_OFF;
    gRfiuSWReadySemEvt=OSSemCreate(1);
    
#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
    sysVideoInCHsel=1;
#endif

    rfiu_InitCamOnOff(rfiuRX_CamOnOff_Sta);
#if(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2)
    rfiuRX_OpMode = RFIU_RX_OPMODE_NORMAL;
#elif( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
    sysCameraMode = SYS_CAMERA_MODE_RF_RX_DUALSCR;
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

   if(rfiuRX_OpMode == RFIU_RX_OPMODE_QUAD)
   {
      #if UI_GRAPH_QVGA_ENABLE
        #if RFRX_HALF_MODE_SUPPORT
           if(rfiuRX_CamOnOff_Num <= 2)
           {
              if(sysTVOutOnFlag)
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
              else
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
           }
           else
        #endif
              iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
      #else
        if( 
              (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) && 
              ( (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS) ) 
          )
        {
        #if RFRX_HALF_MODE_SUPPORT
           if(rfiuRX_CamOnOff_Num <= 2)
           {
              if(sysTVOutOnFlag)
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
              else
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
           }
           else
        #endif
              iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
        }
        else
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
    }
    
    rfiuRX_P2pVideoQuality=RF_P2PVdoQalt_QVGA_15_FPS;
    rfiu_TX_P2pVideoQuality=RFWIFI_P2P_QUALITY_MEDI;
    rfiu_TX_WifiPower=0;
    rfiu_TX_WifiCHNum=0;
    M7688_WifiPower=0;
    M7688_WifiCHNum=0;

    
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

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
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
#elif(HW_BOARD_OPTION == MR8600_RX_DEMO_BOARD)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );
#elif(HW_BOARD_OPTION == MR8600_RX_DB2)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );
#elif((HW_BOARD_OPTION == MR8600_RX_RDI) || (HW_BOARD_OPTION == MR8600_RX_RDI2))
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_JIT)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_JESMAY)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_TRANWO)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_MAYON)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8600_RX_GCT)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN);
#elif(HW_BOARD_OPTION == MR8200_RX_ZINWELL)
    GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );  // RFU pin mux: use IIC for RF2
#elif(HW_BOARD_OPTION == MR8200_RX_JIT)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2  
#elif(HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2  
#elif(HW_BOARD_OPTION == MR8120_RX_JIT_M703SW4)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2     
#elif(HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M930)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M742)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M706)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M731)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8120_RX_RDI_M733)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M742_HA)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M900)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M902)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M712)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_RDI_M713)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8600_RX_RDI_M904D)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_RX240)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M721)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2   
#elif(HW_BOARD_OPTION == MR8120_RX_RDI_M724)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2   
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M701)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2 
#elif(HW_BOARD_OPTION == MR8120_RX_RDI_M703)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2 
#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M920)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2 
#elif(HW_BOARD_OPTION == MR8200_RX_DB2)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_DB3)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_COMMAX)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_BOX)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_Alford_BOX)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_GCT_LCD)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2    
#elif(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_TELEFIELDS)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION == MR8120_RX_GCT_SC7700)
    GpioActFlashSelect |= (GPIO_RF12_FrSP2_EN );  // RFU pin mux: use SPI2 for RF1 amd RF2
#elif(HW_BOARD_OPTION  == MR8120_TX_SOCKET)    
    #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )

	#else
      GpioActFlashSelect |= (GPIO_RF2_FrIIC_EN );
    #endif 
//-----------------------A1018X------------------//
#elif(HW_BOARD_OPTION  == A1018_EVB_128M)
    GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from UART-B
#elif(HW_BOARD_OPTION  == MR9120_TX_DB)
    GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from UART-B
#elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port
#elif(HW_BOARD_OPTION  == MR9300_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port   
#elif(HW_BOARD_OPTION  == MR9200_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port    
#elif(HW_BOARD_OPTION  == MR9600_RX_DB)
    //GpioActFlashSelect |= CHIP_IO_RFI2_EN;  //from dedicate port       
#elif(HW_BOARD_OPTION  == MR9600_RX_OPCOM)
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
       
       gRfiu_WrapEnc_Sta[i] = RFI_WRAPENC_TASK_NONE;
       gRfiu_MpegEnc_Sta[i] = RFI_MPEGENC_TASK_NONE;
       gRfiu_TX_Sta[i]      = RFI_TX_TASK_NONE;

       gRfiuFCC247ChUsed[i][0]=-1;
       gRfiuFCC247ChUsed[i][1]=-1;

       rfiuRXWrapSyncErrCnt[i]=0;

       rfiuRxVideoBufMngWriteIdx[i]=0;
       rfiuRxIIsSounBufMngWriteIdx[i]=0;

	   rfiuVideoBufFill_idx[i]=0;
	   rfiuVideoBufPlay_idx[i]=0;

	   rfiuAudioBufPlay_idx[i]=0;
	   rfiuAudioBufFill_idx[i]=0;

	   rfiuVideoTimeBase[i]=0;
       gRfiuUnitCntl[i].RFpara.RF_ID=0xffffffff;
       
    #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
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
#endif

#if RFIU_TEST
    pp=(unsigned int *)rfiuOperBuf[0];
    for(i=0;i<64*128*(RFI_BUF_SIZE_GRPUNIT+1)/4;i++)
    {
        *pp= *pp + i*1343;
        pp ++;
    }
#endif
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuTimer_offset[i]=0;
    }

#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
    gRfiuUnitCntl[0].RFpara.PIR_en=iconflag[UI_MENU_SETIDX_PIR];
    gRfiuUnitCntl[0].RFpara.MD_en =iconflag[UI_MENU_SETIDX_TX_MOTION];
    gRfiuUnitCntl[0].RFpara.MD_Level_Day =iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY];
    gRfiuUnitCntl[0].RFpara.MD_Level_Night =iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT];
    gRfiuUnitCntl[0].RFpara.RF_ID=ID;

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
    
}

s32 RfiuReset(int RFIunit)
{
   int i;
   u32 temp;
   static int ErrCnt[MAX_RFIU_UNIT]=0;;
   //=========//
   
#if FIX_A1018A_BUG_RF
   if(RFIunit==RFI_UNIT_1)
      RFIunit=RFI_UNIT_2;
#endif

   #if 1//(RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_4TX_2RX_PROTOCOL || RFIU_TEST)
     *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_RESET;
     for(i=0;i<50;i++); //delay 
     #if 1
	     temp=*((volatile unsigned *)(REG_RFIU_SyncWord_H+0x1000*RFIunit));
		 if((temp & 0x0ffff) != 0x1234)
		 {
		     if(ErrCnt[RFIunit]>4)
		     {
		        DEBUG_RFIU_P2("A1016A RFIU-%d local reset is Error! Reboot! 0x%x\n",RFIunit,temp);
		        sysForceWDTtoReboot(); 
		     }
             ErrCnt[RFIunit] ++;
		     DEBUG_RFIU_P2("A1016A RFIU-%d local reset is Fail! 0x%x\n",RFIunit,temp);
		 }
         else
         {
             ErrCnt[RFIunit]=0;
         }
	 #endif
   #else
     //A1016 ASIC bug , so must hardware reset
       SYS_RSTCTL |= SYS_RSTCTL_RF1013_RST;
     for(i=0;i<10;i++); //delay
       SYS_RSTCTL &= ~(SYS_RSTCTL_RF1013_RST);  
     for(i=0;i<10;i++); // delay      
   #endif

}

void rfiu_InitCamOnOff(u32 Status)
{
  int i;
  u32 Bits;
  unsigned int  cpu_sr = 0;
  //------------------------------//

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
#endif
  OS_EXIT_CRITICAL();
}








unsigned int rfiuDataPktConfig_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara)
{
     unsigned int IntrMask;
     unsigned char err;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
     unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
     //-------------------------------//
#if FIX_A1018A_BUG_RF
   if(RFIunit==RFI_UNIT_1)
      RFIunit=RFI_UNIT_2;
#endif


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
#if RFIU_SUPPORT_A1018_25SERS
    *((volatile unsigned *)(REG_RFIU_FMTSEL+0x1000*RFIunit)) = 0x01;
#endif

     OS_ENTER_CRITICAL();
#if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
    A7130_TxMode_Start(RFIunit+1);
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    A7196_TxMode_Start(RFIunit+1);
#endif
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_TRIG;
     OS_EXIT_CRITICAL();
 

    return 1;
}

unsigned int rfiuDataPktConfig_Rx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara)
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

#if FIX_A1018A_BUG_RF
   if(RFIunit==RFI_UNIT_1)
      RFIunit=RFI_UNIT_2;
#endif

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
    *((volatile unsigned *)(REG_RFIU_RxTimingFineTune+0x1000*RFIunit))=pRfiuPara->RxTimingFineTune;

        
   

    *((volatile unsigned *)(REG_RFIU_TxRxOpBaseAddr+0x1000*RFIunit))=(unsigned int)pRfiuPara->TxRxOpBaseAddr;

    *((volatile unsigned *)(REG_RFIU_DRAMADDRLIMIT+0x1000*RFIunit))= DRAM_MEMORY_END>>16;

#if RFIU_SUPPORT_A1018_25SERS
    *((volatile unsigned *)(REG_RFIU_FMTSEL+0x1000*RFIunit)) = 0x01;
#endif


    OS_ENTER_CRITICAL();
#if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
    A7130_RxMode_Start(RFIunit+1);
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    A7196_RxMode_Start(RFIunit+1);
#endif
    *((volatile unsigned *)(REG_RFIU_CNTL_0+0x1000*RFIunit)) |= RFI_TRIG;  
    OS_EXIT_CRITICAL();
 

    return 1;
}

int rfiuWaitForInt_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara)
{
     unsigned int FlagMask;
     unsigned char err;
     unsigned char bitstuffcnt;
     static int RF_Tx_err=0;
     //----------------------//
#if FIX_A1018A_BUG_RF
   if(RFIunit==RFI_UNIT_1)
      RFIunit=RFI_UNIT_2;
#endif
     
     FlagMask = 0x03<<(RFIunit*2);
     OSFlagPend(gRfiuFlagGrp,FlagMask, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_TIMEOUT, &err);
     OSSemPost(gRfiuReqSem[RFIunit]);
     
#if ( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     A7130_TxMode_Stop(RFIunit+1);
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     A7196_TxMode_Stop(RFIunit+1);
#endif
     if (err != OS_NO_ERR)
     {
    	DEBUG_RFIU_P2("--->Wait RFI-TX Unit-%d Timeout!\n",RFIunit);
        RFIU_INT_EN &= ~FlagMask;
        #if DEBUG_TX_TIMEOUT
        #endif
        RF_Tx_err ++;
        if(RF_Tx_err>15)
        {
            DEBUG_RFIU_P2("Force to reboot!\n");
            sysForceWDTtoReboot();
        }
        
        return 0;    // error
     }
     else
     {
         RF_Tx_err=0;
     }
     
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
     static int RF_Rx_err=0;
     //-------//
     
#if FIX_A1018A_BUG_RF
     if(RFIunit==RFI_UNIT_1)
        RFIunit=RFI_UNIT_2;
#endif
     
     FlagMask = 0x03<<(RFIunit*2);
     OSFlagPend(gRfiuFlagGrp,FlagMask, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, RFIU_TIMEOUT, &err);
     OSSemPost(gRfiuReqSem[RFIunit]);

     
     if (err != OS_NO_ERR)
     {
    	DEBUG_RFIU_P2("--->Wait RFI-RX Unit-%d Timeout!\n",RFIunit);
        RFIU_INT_EN &= ~FlagMask;
        pRfiuPara->TxRxPktNum=0;
        RF_Rx_err ++;
        if(RF_Rx_err>15)
        {
            DEBUG_RFIU_P2("Force to reboot!\n");
            sysForceWDTtoReboot();
        }
            
     #if DEBUG_RX_TIMEOUT   
     #endif
        return 0;    // error
     }
     else
     {
        RF_Rx_err=0;
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
     
     #if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     Rssi=RSSI_measurement_A7130(RFIunit+1);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     Rssi=RSSI_measurement_A7196(RFIunit+1);
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
     #endif
 #endif
   
#if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
     A7130_RxMode_Stop(RFIunit+1);
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
     A7196_RxMode_Stop(RFIunit+1);
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
          #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
              if (cnt >10)
          #else
              if (cnt >4)
          #endif
              {
                  DEBUG_UI("rfiu_SetRXOpMode_All Timeout:%d !!!\n",i);
                  break;
              }
              cnt++;
              OSTimeDly(1);
          }          
       }
   }

   return 1;
}

int rfiu_SetRXOpMode_1(int RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"MODE %d %d",rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetRXOpMode_1 Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }

   return 1;
}


int rfiu_SetTXTurbo_On(int RFUnit)
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

int rfiu_SetTXTurbo_Off(int RFUnit)
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

//Lucian: Fix Tx bug
int rfiu_ResendTxMdConfig(int RFUnit)
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

int rfiu_SendTxMdSense(int RFUnit)
{
   unsigned char cmd[32];
   int i;
   int cnt,RfBusy;
   //============//

#if HW_MD_SUPPORT
 #if (HW_BOARD_OPTION == MR8200_RX_RDI_M706 && PROJ_OPT == 8)
    if(gRfiuUnitCntl[RFUnit].VMDSel)
    {
        if (!strcmp((const char*)gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion, TxMdSensTableForOldCL692))
        {
            DEBUG_RFIU("%s Use Non-sensitive Table to disable VMD\n",gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion);
            sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                               MD_SensitivityConfTab_CL692_150803[0][0],
                               MD_SensitivityConfTab_CL692_150803[0][1],
                               MD_SensitivityConfTab_CL692_150803[1][0],
                               MD_SensitivityConfTab_CL692_150803[1][1],
                               MD_SensitivityConfTab_CL692_150803[2][0],
                               MD_SensitivityConfTab_CL692_150803[2][1]
                    );

        }
        else
        {
            sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                                MD_SensitivityConfTab_New[0][0],
                                MD_SensitivityConfTab_New[0][1],
                                MD_SensitivityConfTab_New[1][0],
                                MD_SensitivityConfTab_New[1][1],
                                MD_SensitivityConfTab_New[2][0],
                                MD_SensitivityConfTab_New[2][1]
                    );
        }
    }
    else
    {
        sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                           MD_SensitivityConfTab[0][0],
                           MD_SensitivityConfTab[0][1],
                           MD_SensitivityConfTab[1][0],
                           MD_SensitivityConfTab[1][1],
                           MD_SensitivityConfTab[2][0],
                           MD_SensitivityConfTab[2][1]
                );

    }
 #else

   if(gRfiuUnitCntl[RFUnit].VMDSel)
       sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                      MD_SensitivityConfTab_New[0][0],
                      MD_SensitivityConfTab_New[0][1],
                      MD_SensitivityConfTab_New[1][0],
                      MD_SensitivityConfTab_New[1][1],
                      MD_SensitivityConfTab_New[2][0],
                      MD_SensitivityConfTab_New[2][1]
               );
   else
       sprintf((char*)cmd,"MDSENS %d %d %d %d %d %d", 
                      MD_SensitivityConfTab[0][0],
                      MD_SensitivityConfTab[0][1],
                      MD_SensitivityConfTab[1][0],
                      MD_SensitivityConfTab[1][1],
                      MD_SensitivityConfTab[2][0],
                      MD_SensitivityConfTab[2][1]
               );
 #endif
#endif    
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


int rfiu_SetTXVoxCfg(int RFUnit)
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   
   //============//   
    if (gRfiu_Op_Sta[RFUnit] != RFIU_RX_STA_LINK_OK)
        return 0;

   
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

int rfiu_SetTXSchedule(int RFUnit,u8 par1,u8 par2,u8 par3,u8 par4,u8 par5,u8 par6)
{
   unsigned char cmd[50];
   int i;
   int cnt,RfBusy;
   
   //============//
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
   
   return RfBusy;
}

int rfiu_SetRXVoxTrig(u8 vol)
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


int rfiu_TXCMD_Enc (u8 *cmd,int RFUnit)
{
    int data;
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
 #if (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    else if(!strncmp((char*)cmd,"BODYINFO", strlen("BODYINFO")))
    {
       gRfiuUnitCntl[RFUnit].TXCmd_Type  |=RFTXCMD_SEND_DATA;
       gRfiuUnitCntl[RFUnit].TX_CMDPara[0]=RFTXINFO_SET_BODYINFO; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=gRfiuUnitCntl[RFUnit].RFpara.Temperature; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[2]=gRfiuUnitCntl[RFUnit].RFpara.Humidity; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[3]=gRfiuUnitCntl[RFUnit].RFpara.PM25; 
       if(sys8211TXWifiStat == MR8211_ENTER_WIFI)
           gRfiuUnitCntl[RFUnit].TX_CMDPara[4]= ((rfiu_TX_WifiCHNum & 0xf)<<4) | 0x01; 
       else
           gRfiuUnitCntl[RFUnit].TX_CMDPara[4]= 0; 

       gRfiuUnitCntl[RFUnit].TXCmd_en=1;
    }
 #elif (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1)
    else if(!strncmp((char*)cmd,"BODYINFO", strlen("BODYINFO")))
    {
       gRfiuUnitCntl[RFUnit].TXCmd_Type  |=RFTXCMD_SEND_DATA;
       gRfiuUnitCntl[RFUnit].TX_CMDPara[0]=RFTXINFO_SET_BODYINFO; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=gRfiuUnitCntl[RFUnit].RFpara.Temperature; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[2]=gRfiuUnitCntl[RFUnit].RFpara.Humidity; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[3]=gRfiuUnitCntl[RFUnit].RFpara.PM25; 
       gRfiuUnitCntl[RFUnit].TX_CMDPara[4]= 0; 

       gRfiuUnitCntl[RFUnit].TXCmd_en=1;
    }
#endif
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
    else if(CmdType==RFTXINFO_SET_BODYINFO)
    {       
       gRfiuUnitCntl[RFUnit].RFpara.Temperature=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];
       gRfiuUnitCntl[RFUnit].RFpara.Humidity   =gRfiuUnitCntl[RFUnit].TX_CMDPara[2];
       gRfiuUnitCntl[RFUnit].RFpara.PM25       =gRfiuUnitCntl[RFUnit].TX_CMDPara[3];
       gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn =gRfiuUnitCntl[RFUnit].TX_CMDPara[4] & 0x01;
       gRfiuUnitCntl[RFUnit].RFpara.WifiLinkCH =(gRfiuUnitCntl[RFUnit].TX_CMDPara[4]>>4) & 0x0f;
       DEBUG_RFIU_P2("\n--Body info:%d,%d,%d-\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn,gRfiuUnitCntl[RFUnit].RFpara.WifiLinkCH);
    }
    else
    {
        DEBUG_RFIU_P2("Unknow Info..\n");
    }

    return 0;
    
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
    u32 status,CamCnt;
    
	unsigned int  cpu_sr = 0;
#if UI_GRAPH_QVGA_ENABLE
    u8  RfLevel[5] = {  RF_P2PVdoQalt_VGA_15_FPS,
                        RF_P2PVdoQalt_VGA_10_FPS,
                        RF_P2PVdoQalt_QVGA_10_FPS,
                        RF_P2PVdoQalt_VGA_15_FPS,
                        RF_P2PVdoQalt_VGA_10_FPS};
#else
    u8  RfLevel[5] = {  RF_P2PVdoQalt_HD_5_FPS,
                        RF_P2PVdoQalt_VGA_10_FPS,
                        RF_P2PVdoQalt_QVGA_15_FPS,
                        RF_P2PVdoQalt_VGA_30_FPS,
                        RF_P2PVdoQalt_QVGA_7_FPS};
#endif

    //----------------//

     if(gRfiuUnitCntl[RFUnit].RXCmd_en)
        return 1;

     if(gRfiuUnitCntl[RFUnit].RXCmd_Busy )
        return 2;

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
        OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_CHGRESO<<(RFUnit*8), OS_FLAG_SET, &err);
        gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_CHGRESO;

	}
    else if(!strncmp((char*)cmd,"MODE ", strlen("MODE ")))
    {
        sscanf((char*)cmd, "MODE %d %d", &Rx_OpMode,&par2);
        
        DEBUG_RFIU_P2("Set TX-%d RXOpMode=%d,Level=%d,RXCamNum=%d\n",RFUnit,Rx_OpMode,par2,rfiuRX_CamOnOff_Num);
        
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_OPMODE;

        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= Rx_OpMode;  //MB unit
        //gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=RfLevel[par2-1];
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= rfiuRX_CamOnOff_Num;

   #if (RFI_TEST_4TX_2RX_PROTOCOL)  
        if( (RFUnit & 0x01) == 0 )
        {
            if( (rfiuRX_CamOnOff_Sta & 0x5)== 0x5 )
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=2;
            else
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
        }
        else if( (RFUnit & 0x01) == 1 )
        {
            if( ( rfiuRX_CamOnOff_Sta & 0xa)== 0xa )
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=2;
            else
                gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
        }
        else
          gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]=1;
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
         
   #elif RFI_TEST_4x_RX_PROTOCOL_B1
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
        
    #if( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 & 0x03; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2>>2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3>>2; 
    #else
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 & 0x01; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par2>>3; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par3>>3; 
    #endif            
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
   #if 0 //Mask Area//

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

        gRfiuUnitCntl[RFUnit].RFpara.PIR_en=par1;
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
        
        //DEBUG_RFIU_P2("PTZ: %d %d %d %d %d %d %d\n",par1,par2,par3,par4,par5,par6,par7);        
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
    else if(!strncmp((char*)cmd,"STAMP ", strlen("STAMP ")))
    {
        sscanf((char*)cmd, "STAMP %d", &par1);
                
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_TIMESTAMP;
        
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= par1 ;  //bit0: ON/OFF

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
        DEBUG_RFIU_P2("Set MASKAREA_HD:%x %x %x %x %x %x %x %x %x\n",par1,par2,par3,par4,par5,par6,par7,par8,par9);        
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
    else if(!strncmp((char*)cmd,"SCHED ", strlen("SCHED ")))
    {
        sscanf((char*)cmd, "SCHED %d %d %d %d %d %d", &par1,&par2,&par3,&par4,&par5,&par6);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SCHED;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[3]= par3;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[4]= par4; 
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[5]= par5;  
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[6]= par6; 
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"SATU ", strlen("SATU ")))
    {
        sscanf((char*)cmd, "SATU %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SATURATION;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;         
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
    else if(!strncmp((char*)cmd,"SNAP ", strlen("SNAP ")))
    {
        sscanf((char*)cmd, "SNAP %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_SNAPSHOT;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
        DEBUG_RFIU_P2("==Set SNAP:%d==\n",RFUnit);
    }
    else if(!strncmp((char*)cmd,"MUSIC ", strlen("MUSIC ")))
    {
        sscanf((char*)cmd, "MUSIC %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_MUSICCTL;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;         
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"PHOTOTIME ", strlen("PHOTOTIME ")))
    {
        sscanf((char*)cmd, "PHOTOTIME %d %d", &par1,&par2);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type    = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_PHOTOTIME;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;  //Hour
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[2]= par2;  //min       
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"VOCTEST ", strlen("VOCTEST ")))
    {
        sscanf((char*)cmd, "VOCTEST %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_VOCTEST;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
        gRfiuUnitCntl[RFUnit].RXCmd_en = 1;	   
        OS_EXIT_CRITICAL();
    }
    else if(!strncmp((char*)cmd,"PUSHAPPMSG ", strlen("PUSHAPPMSG ")))
    {
        sscanf((char*)cmd, "PUSHAPPMSG %d", &par1);
        OS_ENTER_CRITICAL();	
        gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_PUSHAPPMSG;
        gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]= par1;      
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
    
    
    return 0;
}


int rfiu_CheckTxTaskMode(int RFUnit)
{
     return gRfiuUnitCntl[RFUnit].OpMode;
}

void rfiu_Start(void)
{
    u16 i;
#if RF_TX_OPTIMIZE
    s32             TimeOffset;
#endif

       DEBUG_RFIU_P2("--rfiu_Start--\n");

       
    #if RFI_TEST_RX_PROTOCOL_B1
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 
        
    #elif RFI_TEST_RX_PROTOCOL_B2
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1); 
	
    #elif RFI_TEST_RXRX_PROTOCOL_B1B2    
        rfiuRX_CamOnOff_Sta=0x03;
        rfiuRX_CamOnOff_Num=2;
      #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
        //-----TV encoder init: must have 656 clock-----//
    	subTV_Preview(640,480);
        GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN );

        #if(TV_ENCODER == BIT1201G)
            i2cInit_BIT1201G();            
        #endif	  
        
        tvBT656CONF |= 0x00040000;
	    SYS_ANA_TEST2 &= (~0x08); //swith to main TV DAC .
	    IsIduOsdEnable=0;
      #else
      #endif
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0); 
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);

      
    #elif RFI_TEST_2x_RX_PROTOCOL_B1    
        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);
        OSTimeDly(1);

        //--RX--//
        OSTaskCreate(RFIU_RX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);

        #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
        //-----TV encoder init: must have 656 clock-----//
    	subTV_Preview(640,480);
        GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN );

        
        #if(TV_ENCODER == BIT1201G)
            i2cInit_BIT1201G();            
        #endif	  
        
        tvBT656CONF |= 0x00040000;
	    SYS_ANA_TEST2 &= (~0x08); //swith to main TV DAC .
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

   #elif (RFI_TEST_TX_PROTOCOL_B1 && RF_TX_OPTIMIZE)
        //--TX--//
        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_OpMode = SIUMODE_MPEGAVI;
        ciu_1_FrameTime = 0;

        #elif(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
        ciu_2_FrameTime = 0;

        #endif
        sysReady2CaptureVideo   = 1;
        
        if(gRfiu_TX_Sta[0]==RFI_TX_TASK_NONE)
        {
           DEBUG_ASF("====RFIU1_TASK Create====\n");
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
      
        iisResumeTask();
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4ResumeTask();
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskResume();
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgResumeTask();
        #endif

    #elif (RFI_TEST_TX_PROTOCOL_B2 && RF_TX_OPTIMIZE)
        //--TX--//
        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_OpMode = SIUMODE_MPEGAVI;
        ciu_1_FrameTime = 0;

        #elif(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
        ciu_2_FrameTime = 0;

        #endif
        sysReady2CaptureVideo   = 1;
        
        if(gRfiu_TX_Sta[1]==RFI_TX_TASK_NONE)
        {
           DEBUG_ASF("====RFIU2_TASK Create====\n");
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

        iisResumeTask();
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            mpeg4ResumeTask();
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
            VideoTaskResume();
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
            mjpgResumeTask();
        #endif
    
    #endif

}

void rfiu_End(void)
{
    int i,j;
    u8 err;

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    #if RFI_TEST_TX_PROTOCOL_B1
      #if RFIU_RX_AUDIO_RETURN    
          if(guiIISPlayDMAId != 0xFF)
          {
              marsDMAClose(guiIISPlayDMAId);
              guiIISPlayDMAId = 0xFF;
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
          if(guiIISPlayDMAId != 0xFF)
          {
              marsDMAClose(guiIISPlayDMAId);
              guiIISPlayDMAId = 0xFF;
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
        for(i=0;i<4;i++)
        {
           gRfiuUnitCntl[i].RX_Task_Stop=1;
           gRfiuUnitCntl[i].RX_Wrap_Stop=1;
           gRfiuUnitCntl[i].RX_MpegDec_Stop=1;
        }
        #if( (SW_APPLICATION_OPTION  ==  MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        OSTimeDly(40);  //delay 2 sec
        #else
        OSTimeDly(40);  //delay 2 sec,強迫tx 斷線
        #endif
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
       gRfiuFCC247ChUsed[i][0]  = -1;
       gRfiuFCC247ChUsed[i][1]  = -1;
       
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

int rfiuSetGPO_TX(u8 setting)
{
#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
     (HW_BOARD_OPTION  == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
    u8  SendCmd;
    u8  MirrorFlip;
#endif

#if(HW_BOARD_OPTION ==  MR8120_TX_DB2)
    switch(setting)
    {
        case 0:
           gpioSetLevel(1, 9, 0);
           break;
           
        case 1:
           gpioSetLevel(1, 9, 1);
           break;
    }
#elif(HW_BOARD_OPTION == MR8120_TX_HECHI)
    switch(setting) // 1:ON 0:OFF
    {
       case 0: //OFF
           gpioSetLevel(1, 5, 1);
           break;

       case 1: //ON
           gpioSetLevel(1, 5, 0);
           OSTimeDly(20);
           gpioSetLevel(1, 5, 1);
           break;
    }
#elif (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    DEBUG_RFIU_P2("rfiuSetGPO_TX:%d\n",setting);
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
#elif (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
    switch(setting)
    {
        case 0:
           gpioSetLevel(GPIO_GROUP_P_LED, GPIO_BIT_P_LED, 0);
           break;
           
        case 1:
           gpioSetLevel(GPIO_GROUP_P_LED, GPIO_BIT_P_LED, 1);
           break;
    }
#elif ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
    gpioGetLevel(GPIO_GROUP_MIRROR_FLIP,GPIO_PIN_MIRROR_FLIP,&MirrorFlip); /*0: Normal, 1: Flip*/
    DEBUG_RFIU_P2("rfiuSetGPO_TX:%d, MirrorFlip = %d\n", setting, MirrorFlip);
    switch(setting)
    {
        case SYS_UNTOUCHDE_PAN_ARROW_UP:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = (MirrorFlip ? 0x01 : 0x02); /*Delect MirrorFlip*/
            sendchar(RDI_MCU_UART_ID, &SendCmd);
            break;
            
        case SYS_UNTOUCHDE_PAN_ARROW_DOWN:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = (MirrorFlip ? 0x02 : 0x01); /*Delect MirrorFlip*/
            sendchar(RDI_MCU_UART_ID, &SendCmd);
            break;

        case SYS_UNTOUCHDE_PAN_ARROW_LEFT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = (MirrorFlip ? 0x04 : 0x08); /*Delect MirrorFlip*/
            sendchar(RDI_MCU_UART_ID, &SendCmd);
            break;

        case SYS_UNTOUCHDE_PAN_ARROW_RIGHT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = (MirrorFlip ? 0x08 : 0x04); /*Delect MirrorFlip*/
            sendchar(RDI_MCU_UART_ID, &SendCmd);
            break;
    }
#elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
    uiSentKeyToUi(UI_KEY_SEP_LIGHT_OFF+setting);
    DEBUG_RFIU_P2("rfiuSetGPO_TX:%d, SendCmd(0x%02x)\n",setting,SendCmd);
#endif

}


int rfiuSetPWM_TX(u8 setting)
{
    DEBUG_RFIU_P2("rfiuSetPWM_TX:%d\n",setting);
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
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
            uiMenuAction(UI_MENU_SETIDX_LIGHT_DIMMER);
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, GPIO_LEVEL_HI);
            uiInScheduleLight = 1;
            uiSetTriggerDimmer = UI_TRIGGER_DIMMER_TIME;
        }
        else
        {
            gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, GPIO_LEVEL_LO);
            uiInScheduleLight = 0;
            uiSetTriggerDimmer = 0;
        }
    }

#endif
}
#if ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
int rfiu_SetTXVocDustTestMode(int RFUnit,u8 val) /*0: Voc Test, 1: Dust Test*/
{
   unsigned char cmd[16];
   int i;
   int cnt,RfBusy;
   //============//
   
   sprintf(cmd,"VOCTEST %d",val);
   cnt=0;
   RfBusy=1;
   while(RfBusy != 0)
   {
        RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
        if (cnt >4)
        {
            DEBUG_UI("rfiu_SetTXVocDustTestMode Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
   }
   return 1;
}
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
int rfiuSetSensorLightCtrl_TX(u8 setting)
{
    u8  SendCmd;
    switch(setting)
    {
        case 0:  /*Switch Off*/
            SendCmd = 0x20;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case 1: /*Switch On*/
            SendCmd = 0x21;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;
            
        default:
            DEBUG_RFIU_P2("rfiuSetSensorLightCtrl_TX ERROR\r\n");
            break;
    }
    DEBUG_RFIU_P2("rfiuSetSensorLightCtrl_TX: Switch(%d),SendCmd(0x%02x)\n",setting , SendCmd);

}

int rfiuSetMotorCtrl_TX(u8 setting)
{
    u8  SendCmd;
    switch(setting)
    {
        case UI_SET_MOTOR_ARROW_UP:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x01;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;
            
        case UI_SET_MOTOR_ARROW_DOWN:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x02;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_LEFT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x04;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_RIGHT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x08;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_CONTINUE_UP:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x06;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;
            
        case UI_SET_MOTOR_ARROW_CONTINUE_DOWN:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x07;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_CONTINUE_LEFT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x09;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_CONTINUE_RIGHT:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x0C;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;

        case UI_SET_MOTOR_ARROW_STOP:
            adcSetADC_MICIN_PGA_Gain(31);
            timerDisableMic = 20;
            SendCmd = 0x0E;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            break;
    }
    nMotorTime=2;

    DEBUG_RFIU_P2("rfiuSetMotorCtrl_TX: ARROW(%d),SendCmd(0x%02x)\n",setting , SendCmd);

}
int rfiuSetVOCTest_TX(void)
{
    u8  SendCmd = 0x18;

    sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
    rfiuVocDustMode = 1;
    DEBUG_RFIU_P2("rfiuSetVOCTest_TX: SendCmd(0x%02x), rfiuVocDustMode(%d)\n", SendCmd, rfiuVocDustMode);
}
int rfiuSetDustTest_TX(void)
{
    u8  SendCmd = 0x19;

    sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
    rfiuVocDustMode = 2;
    DEBUG_RFIU_P2("rfiuSetDustTest_TX: SendCmd(0x%02x), rfiuVocDustMode(%d)\n", SendCmd, rfiuVocDustMode);
}
#else
int rfiuSetVOCTest_TX(void)
{}
int rfiuSetDustTest_TX(void)
{}
int rfiuSetMotorCtrl_TX(u8 setting) // [3:0]=H, [7:4]=V
{
   DEBUG_RFIU_P2("rfiuSetMotorCtrl_TX:0x%02x\n",setting);
#if MOTOR_EN
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    MotorSet(setting & 0x0f);
    if(uiIsVM9710 == 1)
    {
        if(setting & 0x10)
           nMotorTime = 5;
        else
           nMotorTime = 3;
    } 
    else
        nMotorTime = 1;
#else
   //   if(nMotorTime>0)
   //        return 0;
      MotorStatusH = setting & 0x0f;
      MotorStatusV = setting >> 4;
      nMotorTime = 5;
#endif
#endif

}
#endif
int rfiuSetMelodyNum_TX(u8 setting)
{
    u8 data=0;

    DEBUG_RFIU_P2("rfiuSetMelodyNum_TX:%d\n",setting);
#if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    i2cRead_WT6853(0x01, &data);
    data &= ~0xf;
    switch (setting)
    {
        case 0:
            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 1);
//            gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
//            OSTimeDly(1);
//            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
            data |= 0xf;
            i2cWrite_WT6853(0x01, data);
            break;
        
        case 1:
            data |= 8;
            i2cWrite_WT6853(0x01, data);
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
            break;
        
        case 2:
            data |= 4;
            i2cWrite_WT6853(0x01, data);
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
            
            break;
        
        case 3:
            data |= 12;
            i2cWrite_WT6853(0x01, data);
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
            break;
            
        default:
            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 1);
//            gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
//            OSTimeDly(1);
//            gpioSetLevel(GPIO_GROUP_POP_EN, GPIO_BIT_POP_EN, 0);
            data |= 0xf;
            i2cWrite_WT6853(0x01, data);
            break;
    }
#elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    switch (setting)
    {
        case 5:
            Melody_play_num=1;
            if(Melody_Last_play!=5)
                if (uiSentKeyToUi(UI_KEY_Melody_PlayAll) )
                    Melody_Last_play=5;
            break;
        case 6:
            Melody_play_num=1;
            if(Melody_Last_play!=6)
                if (uiSentKeyToUi(UI_KEY_Melody_Play) )
                    Melody_Last_play=6;
            break;
        case 7:
            Melody_play_num=2;
            if(Melody_Last_play!=7)
                if (uiSentKeyToUi(UI_KEY_Melody_Play) )
                    Melody_Last_play=7;
            break;
        case 8:
            Melody_play_num=3;
            if(Melody_Last_play!=8)
                if (uiSentKeyToUi(UI_KEY_Melody_Play) )
                    Melody_Last_play=8;
            break;
        case 9:
            Melody_play_num=4;
            if(Melody_Last_play!=9)
                if (uiSentKeyToUi(UI_KEY_Melody_Play))
                    Melody_Last_play=9;
            break;
        case 10:
            if(Melody_Last_play!=10)
                if (uiSentKeyToUi(UI_KEY_Melody_Pause) )
                    Melody_Last_play=10;
            break;
        case 11:
            if(Melody_Last_play!=11)
                if (uiSentKeyToUi(UI_KEY_Melody_Start) )
                    Melody_Last_play=11;
            break;
        case 12:
            if(Melody_Last_play!=12)
                if (uiSentKeyToUi(UI_KEY_Melody_StartAll) )
                    Melody_Last_play=12;
            break;
        default:
            Melody_Last_play=0;
            uiSentKeyToUi(UI_KEY_Melody_Stop);
            break;
    }
#endif
}


int rfiuSetVoxTrig_RX(int RFUnit)
{
   u8 vol;
   u8 key = 0xff;
   
   vol=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];
   //DEBUG_RFIU_P2("-->TX-%d Vox Trig:%d\r\n",RFUnit,vol);
#if (HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
    switch (RFUnit)
    {
        case 0:
            key = UI_KEY_CAM1;
            break;
        
        case 1:
            key = UI_KEY_CAM2;
            break;
        
        case 2:
            key = UI_KEY_CAM3;
            break;
        
        case 3:
            key = UI_KEY_CAM4;
            break;
            
        default:
            key = 0xff;
            break;
    }
   if(key!=0xff)
       uiSentKeyToUi(key);
#elif (HW_BOARD_OPTION == MR8100_GCT_LCD)
    if (rfiuVoxEna[RFUnit] == 1)
        uiSentKeyToUi(UI_KEY_MONITOR);
#elif ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    if (rfiuVoxEna[RFUnit] == 1)
    {
        uiVoxChannel = RFUnit;
        uiSentKeyToUi(UI_KEY_MONITOR);
    }
  #if(BLE_SUPPORT)
    if(BLEGetStatus() == BLE_CONNECTED)
        BLE_VoxTriggerSetting(RFUnit, 1);
  #endif
//printf("\x1B[96m Vox trigger, here should inform wrist band (Cam%d)!!!!!!! \x1B[0m\n", RFUnit);
#endif
}

int rfiuSetLightStat_RX(int RFUnit)
{
   u8 sta;
   
   sta=gRfiuUnitCntl[RFUnit].TX_CMDPara[1];
#if UI_LIGHT_SUPPORT
    uiFlowSetRfLightStatus(RFUnit, sta, 0);
#endif

#if (HW_BOARD_OPTION == MR8100_GCT_LCD)
    uiOsdSetNightStatus(RFUnit, sta & 0x01 );
    //uiOsdSetLightStatus(RFUnit, (sta>>1) & 0x01 );
#endif
#if (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
    if (sta < 2)
        uiOsdSetNightStatus(RFUnit, sta & 0x01 );
    else
        uiFlowSetRfLightStatus(RFUnit, (sta-2), 0);
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
    if (sta >= 2)
        uiFlowSetRfLightStatus(RFUnit, (sta-2), 0);
#endif
    //DEBUG_RFIU_P2("-->TX-%d Light Status:%d\r\n",RFUnit,sta);
}


int rfiuForceResync(int RFUnit)
{
    u8 err;

    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_RFIU_P2("=============rfiuForceResync Start=============\n");
    gRfiuUnitCntl[RFUnit].RX_Task_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
    OSTimeDly(40);
    gRfiuUnitCntl[RFUnit].RX_Task_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=0;
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;
    DEBUG_RFIU_P2("=============rfiuForceResync End=============\n");
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
  
   rfiu_End();
   rfiuRX_CamOnOff_Sta=Setting;

   num=0;
   for(i=0;i<4;i++)
   {
      if(Setting & 0x01)
        num ++;

      Setting >>= 1;
   }
   rfiuRX_CamOnOff_Num=num;
   
   rfiu_InitCamOnOff(rfiuRX_CamOnOff_Sta);
   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
   {
       sysRFRxInMainCHsel=0x0ff;
   #if UI_GRAPH_QVGA_ENABLE
        #if RFRX_HALF_MODE_SUPPORT
           if(rfiuRX_CamOnOff_Num <= 2)
           {
              if(sysTVOutOnFlag)
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
              else
                 iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
           }
           else
        #endif
              iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);          
    #else
          if( 
                (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) && 
                ( (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS) ) 
            )
          {
            #if RFRX_HALF_MODE_SUPPORT
               if(rfiuRX_CamOnOff_Num <= 2)
               {
                  if(sysTVOutOnFlag)
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                  else
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
               }
               else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
          }
          else
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
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
    {
    }
    else
    {
        #if ((HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
            iduPlaybackMode(800, 480, 800);
        #else
            iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
        #endif
    }
    rfiu_Start();

}


int rfiuCamSleepCmd(u8 *cmd)
{
   int RFUnit;
   int RfBusy;
   int cnt;
   
   sscanf((char*)cmd,"%d",&RFUnit);
   DEBUG_RFIU_P2("-----Set Camera-%d Sleep command----\n",RFUnit); 

   if( rfiuRX_CamOnOff_Sta & (0x01<<RFUnit) )
   {
       sprintf(cmd,"SLEEP");
       cnt=0;
       RfBusy=1;
       while(RfBusy != 0)
       {
          RfBusy=rfiu_RXCMD_Enc(cmd,RFUnit);
          if (cnt >4)
          {
              DEBUG_UI("rfiuCamSleepCmd Timeout:%d!!!\n",RFUnit);
              return 0;
          }
          cnt++;
          OSTimeDly(1);
       }  
   }

   return 1;
   
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
#if (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) //Lucian1113
    uiRFSTAT[RFUnit]=0;
#endif    
    spiWriteRF();
}

int rfiuCheckRFUnpair(int RFUnit)
{
    if(gRfiuSyncWordTable[RFUnit]==0xffffffff)
        return 1;
    else
        return 0;
}

int rfiuPutTXTemHumPM(u32 Tem,u32 Hum,u32 PM25,int RFUnit)
{
    gRfiuUnitCntl[RFUnit].RFpara.Temperature=Tem;
    gRfiuUnitCntl[RFUnit].RFpara.Humidity=Hum;
    gRfiuUnitCntl[RFUnit].RFpara.PM25=PM25;
    return 0;
}

int rfiuPutTXTem(u32 Tem,int RFUnit)
{
    gRfiuUnitCntl[RFUnit].RFpara.Temperature=Tem;
    return 0;
}


int rfiuPutTXHum(u32 Hum,int RFUnit)
{
    gRfiuUnitCntl[RFUnit].RFpara.Humidity=Hum;
    return 0;
}

int rfiuPutTXPM25(u32 PM25,int RFUnit)
{
    gRfiuUnitCntl[RFUnit].RFpara.PM25=PM25;
    return 0;
}


int rfiuGetRXWifiStatus(int RFUnit)
{
    
    return gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn;
}

int rfiuGetRXTemHumPM(u32 *pTem,u32 *pHum,u32 *pPM25,int RFUnit)
{
    *pTem=gRfiuUnitCntl[RFUnit].RFpara.Temperature;
    *pHum=gRfiuUnitCntl[RFUnit].RFpara.Humidity;
    *pPM25=gRfiuUnitCntl[RFUnit].RFpara.PM25;
}

int rfiuGetRXTem(u32 *pTem,int RFUnit)
{
    *pTem=gRfiuUnitCntl[RFUnit].RFpara.Temperature;
}

int rfiuGetRXHum(u32 *pHum,int RFUnit)
{
    *pHum=gRfiuUnitCntl[RFUnit].RFpara.Humidity;
}

int rfiuGetRXPM25(u32 *pPM25,int RFUnit)
{
    *pPM25=gRfiuUnitCntl[RFUnit].RFpara.PM25;
}



#if(  RFIU_TEST || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )

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
        rfiuCmdRxStatus[RFUnit].EnterScanMode= (temp >>3) & 0x01;
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
        rfiuCmdRxStatus[RFUnit].EnterScanMode= (temp >>3) & 0x01;
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
        rfiuCmdRxStatus[RFUnit].EnterScanMode= (temp >>3) & 0x01;
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

    return RX_ACKType;
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

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    RfiuReset(0);
    rfiuDataPktConfig_Rx(0,pRfiuPara_Rx);  //Rx
#elif RFI_TEST_4TX_2RX_PROTOCOL
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Rx(RFUnit & 0x01,pRfiuPara_Rx);  //Rx
#else 
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Rx(RFUnit,pRfiuPara_Rx);  //Rx
#endif

    return 1;
}

int rfiuTxUpdatePktMap(
                                 unsigned char RFUnit,
                                 int DatPktRecvFlag,
                                 DEF_RFIU_USRDATA *pCtrlPara,
                                 DEF_RFIU_USRDATA *pCtrlPara_next,
                                 int *pRX_RecvPktCnt,
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
    int RX_recvDataPktCnt;
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
            RX_TimeCheck=rfiuGetACK2PacketMap_UpdateMap(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,pRX_RecvPktCnt,&RX_recvDataPktCnt,&RxBufWritePtr);
        else
            RX_TimeCheck=rfiuGetACK2TimeCheck(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,
                                              pRX_RecvPktCnt,&RX_recvDataPktCnt);
        
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
        #if 1
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
                      DivF=(TX_CH_Stat[i].SentPktNum+4)>>3;
                      TX_CH_Stat[i].SentPktNum=256;
                      TX_CH_Stat[i].RecvPktNum= (TX_CH_Stat[i].RecvPktNum<<5)/DivF;
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
            #if 0
               for(i=0;i<gRfiuUnitCntl[RFUnit].GoodDataCHNum;i++)
               {
                  DEBUG_RFIU_P2("%d ",GoodDataCH[i]);
               }
               DEBUG_RFIU_P2("\n");
            #endif
            }
            
            //--Suggest next data channel--//
            if( ( (TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum/RFIU_ANALYSIS_CH_INTV) & (32-1) ) == 0)
            {
               gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH = gRfiuUnitCntl[RFUnit].DataCHCnt & (RFI_DAT_CH_MAX-1); 
            }
            else
            {
               if( ((gRfiuUnitCntl[RFUnit].DataCHCnt/16) & (16-1)) == 0 )
               {
                  gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH = GoodDataCH[gRfiuUnitCntl[RFUnit].DataCHCnt & (RFI_DAT_CH_MAX-1) ];
               }
               else
                  gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH = GoodDataCH[gRfiuUnitCntl[RFUnit].DataCHCnt % gRfiuUnitCntl[RFUnit].GoodDataCHNum ]; 
            }	
            gRfiuUnitCntl[RFUnit].DataCHCnt ++;
        #else
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
        #endif

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
           FEC_Level=2;
           Repeat_flag=2;
        }
        else 
        {
           FEC_Level=3;
           Repeat_flag=3;
        }

        //DEBUG_RFIU_P2("%d ",FEC_Level);
  
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

        if(gRfiuUnitCntl[RFUnit].TxPktMap[nextBufReadPtr & RFI_BUF_SIZE_MASK].RetryCount < 4)
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
            DEBUG_RFIU_P("====>Repeat Group:WinCnt=%d!!\n",WinCnt);
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
        //DEBUG_RFIU_P2("%d ",Repeat_flag);

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
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#elif RFI_TEST_4TX_2RX_PROTOCOL
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx);  //Tx
#else
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);  //Tx
#endif

    return 1;
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

   #if(RFIU_TEST==0)
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
            rfiuCmdRxStatus[RFUnit].EnterScanMode= (temp >>3) & 0x01;
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

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
#endif

        return 1;
    }
    

    int rfiuTxSentKEEPState(  int RFUnit,
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
        rfiuPutInfo2KEEP(RFUnit,pRfiuPara_Tx->TxRxOpBaseAddr+RFI_KEEP_ADDR_OFFSET*128);
                       
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
            pRfiuPara_Tx->PktMap[2*i]      =RFI_KEEP_ADDR_CHEKBIT;
            pRfiuPara_Tx->PktMap[2*i+1]    =0x00000000;        
            pRfiuPara_Tx->Pkt_Grp_offset[i]=RFI_CMD_ADDR_OFFSET * RFIU_PKT_SIZE; 
        }
        pRfiuPara_Tx->TxRxPktNum           =RFI_ACK_SYNC_PKTNUM;  
           
        pRfiuPara_Tx->UserData_L           =UserData & RFI_USER_DATA_L_MASK;
        pRfiuPara_Tx->UserData_H           =( (UserData | RFIU_USRDATA_CMD_CHEK)>>16) & RFI_USER_DATA_H_MASK;  //it's Command

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
#endif

        return 1;
    }

    int rfiuPutInfo2SYNC(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i,count;
        int width,height;
        unsigned int  cpu_sr = 0;
        unsigned int TX_Status;
        unsigned int TX_Status2;
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
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
        if(sysTVinFormat==TV_IN_PAL)
            TX_Status |=RFIU_TX_STA__PAL_MODE;

        if(gRfiuUnitCntl[RFUnit].RFpara.MD_en)
            TX_Status |=RFIU_TX_STA__MD_ON;

        if(gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
            TX_Status |=RFIU_TX_STA__PIR_ON;

        if(AE_Flicker_50_60_sel==SENSOR_AE_FLICKER_50HZ)        
            TX_Status |=RFIU_TX_STA_FLICKER_50HZ;

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType==TX_SENSORTYPE_HD)
            TX_Status |=RFIU_TX_STA__HD_SUPPORT;

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn)
            TX_Status |= RFIU_TX_STA_TIMESTAMP_ON;

#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
        gpioGetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, &level);
        if (level)
           TX_Status |= RFIU_TX_STAT_BELL_ON;
#elif ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
        if (uiNightVision)
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

        if(gRfiuUnitCntl[RFUnit].VMDSel==TX_VMD_ORIG) 
            TX_Status = (TX_Status & (~RFIU_TX_STA_NEWMD_SUPPORT)) | (TX_VMD_ORIG<<9);
        else
            TX_Status = (TX_Status & (~RFIU_TX_STA_NEWMD_SUPPORT)) | (TX_VMD_NEW<<9);

        
#if USE_MPEG_QUANTIZATION
        TX_Status |=RFIU_TX_STA_MPEG_Q;
#endif
        //feedback rfiuRX_OpMode to RX
        TX_Status |= ( (rfiuRX_OpMode & 0x0f)<<12);
        TX_Status |= ( (rfiuRX_CamOnOff_Num & 0x0f)<<16);
      #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_TX_BRIGHT] & 0x0f)<<20);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] & 0x0f)<<24);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT] & 0x0f)<<28);
      #endif
           
        *pp=TX_Status;
        pp ++;   
        count ++;
        
        //---Code Version---//
        sprintf ((char*)pp,"%s ", uiVersion);
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

        //---TX_status2---//
        TX_Status2=0;
      #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
        if(sys8211TXWifiStat == MR8211_ENTER_WIFI)
        {
           TX_Status2 |= RFIU_TX_STAT_WIFI_ON;
        }
        if (iconflag[UI_MENU_SETIDX_NIGHT_LIGHT])
           TX_Status2 |= RFIU_TX_STAT_LIGHT_ON;
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

    int rfiuPutInfo2KEEP(int RFUnit,unsigned char *ACKBufAddr)
    {
        unsigned int *pp;
        int i,count;
        unsigned int TX_Status;
        unsigned int TX_Status2;
        
        //=====//
        count=0;
        pp  = (unsigned int *)(ACKBufAddr);

        //------- TX status -------//
        TX_Status=0;
#if  (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
        TX_Status |=RFIU_TX_STA__FIELD_ENC;
#endif
        if(sysTVinFormat==TV_IN_PAL)
            TX_Status |=RFIU_TX_STA__PAL_MODE;

        if(gRfiuUnitCntl[RFUnit].RFpara.MD_en)
            TX_Status |=RFIU_TX_STA__MD_ON;

        if(gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
            TX_Status |=RFIU_TX_STA__PIR_ON;

        if(AE_Flicker_50_60_sel==SENSOR_AE_FLICKER_50HZ)        
            TX_Status |=RFIU_TX_STA_FLICKER_50HZ;

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType==TX_SENSORTYPE_HD)
            TX_Status |=RFIU_TX_STA__HD_SUPPORT;

        if(gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn)
            TX_Status |= RFIU_TX_STA_TIMESTAMP_ON;

#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
        if (uiNightVision)
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

        if(gRfiuUnitCntl[RFUnit].VMDSel==TX_VMD_ORIG) 
            TX_Status = (TX_Status & (~RFIU_TX_STA_NEWMD_SUPPORT)) | (TX_VMD_ORIG<<9);
        else
            TX_Status = (TX_Status & (~RFIU_TX_STA_NEWMD_SUPPORT)) | (TX_VMD_NEW<<9);

        
#if USE_MPEG_QUANTIZATION
        TX_Status |=RFIU_TX_STA_MPEG_Q;
#endif
        //feedback rfiuRX_OpMode to RX
        TX_Status |= ( (rfiuRX_OpMode & 0x0f)<<12);
        TX_Status |= ( (rfiuRX_CamOnOff_Num & 0x0f)<<16);
      #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_TX_BRIGHT] & 0x0f)<<20);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] & 0x0f)<<24);
        TX_Status |= ( (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT] & 0x0f)<<28);
      #endif
           
        *pp=TX_Status;
        pp ++;   
        count ++;
        
        //---TX_status2---//
        TX_Status2=0;
      #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
        if(sys8211TXWifiStat == MR8211_ENTER_WIFI)
        {
           TX_Status2 |= RFIU_TX_STAT_WIFI_ON;
        }
        if (iconflag[UI_MENU_SETIDX_NIGHT_LIGHT])
           TX_Status2 |= RFIU_TX_STAT_LIGHT_ON;
      #endif
        *pp=TX_Status2;
        pp ++;   
        count ++;
        
        //----------溫度---------//
        *pp=gRfiuUnitCntl[RFUnit].RFpara.Temperature;
        pp ++;
        count ++;

        //----------溼度---------//
        *pp=gRfiuUnitCntl[RFUnit].RFpara.Humidity;
        pp ++;
        count ++;

        //-------PM2.5------//
        *pp=gRfiuUnitCntl[RFUnit].RFpara.PM25;
        pp ++;
        count ++;

        //-- Vox Trig flag--//
        if ( (rfiuVoxTrigFlag == 1) )
        {
            //DEBUG_RFIU_P2("-->%d,%d,%d\n",rfiuVoxTrigFlag,rfiuVoxEna[RFUnit],RFUnit);
            *pp=1;
        #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)    
           #if TX_PUSH_APPMSG
             //if(rfiuCmdRxStatus[RFUnit].EnterScanMode)
             {
                sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT,0, 0);
                //DEBUG_RFIU_P2("Push Vox Meg\n");
             }   
           #endif
        #endif
            rfiuVoxTrigFlag=0;
        }
        else
            *pp=0;
        pp ++;
        count ++;

        //-- Vox Trig Level--//
        *pp=rfiuVoxTrigLev;
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
        //rfiuDataPktConfig_Tx(RFUnit);

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
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
        
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
        RfiuReset(0);
        rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#else    
        RfiuReset(RFUnit);      
        rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
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
        int temp;

        static int ZoomOnOff_prev=0;
        static int ZoomXpos_prev=0;
        static int ZoomYpos_prev=0;

        
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
                #if ICOMMWIFI_SUPPORT
				uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
				sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
				#else
                uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;
                sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
                #endif
                }
           #endif

           }
           else if( (reso_W==1280) && (reso_H==720) )
           {
           #if RFIU_SUPPORT
              if(gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType == TX_SENSORTYPE_HD)
              {
                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_1280X720)
                {
                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1280X720;
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                }
              }
              else
              {
                 DEBUG_RFIU_P2("==>Warning!! HD resolution is not support!\n"); 
              }
           #endif
           }
           else if( (reso_W==320) && (reso_H==240) )
           {
           #if RFIU_SUPPORT
                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_320x240)
                {
                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_320x240;
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_320x240);
                }
           #endif
           }
           else if( (reso_W==352) && (reso_H==240) )
           {
           #if RFIU_SUPPORT
                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_352x240)
                {
                #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                   rfiuRX_OpMode |= RFIU_RX_OPMODE_QUAD;
                #endif
                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_352x240;
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
                }
           #endif
           }
           else if( (reso_W==704) && (reso_H==480) )
           {
           #if RFIU_SUPPORT
                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_704x480)
                {
                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                }
           #endif
           }
           else if( (reso_W==704) && (reso_H==576) )
           {
           #if RFIU_SUPPORT
                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_704x576)
                {
                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x576;
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
        #if( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1)  ) 
           RX_OpMode=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           rfiuRX_P2pVideoQuality=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //Lucian: Only valid in P2p mode
           rfiuRX_CamOnOff_Num=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
           rfiuRX_CamPerRF=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];

           if( (RX_OpMode+rfiuRX_P2pVideoQuality+rfiuRX_CamOnOff_Num+rfiuRX_CamPerRF) == 0 )
           {
               RX_OpMode=rfiuRX_OpMode;
    	       DEBUG_RFIU_P2("Invalid OPMode command!\n");      
           }

           if(RX_OpMode != rfiuRX_OpMode)
           {
               rfiuRX_OpMode=RX_OpMode;
           #if USE_704x480_RESO    
               if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
               {
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
               }
               else 
               {
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
               }
           #else
               if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
               {
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
               }
               else 
               {
                   sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
               }
           #endif
           }
        #elif(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
           RX_OpMode=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           rfiuRX_P2pVideoQuality=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //Lucian: Only valid in P2p mode
           rfiuRX_CamOnOff_Num=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
           rfiuRX_CamPerRF=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];

           if( (RX_OpMode+rfiuRX_P2pVideoQuality+rfiuRX_CamOnOff_Num+rfiuRX_CamPerRF) == 0 )
           {
               RX_OpMode=rfiuRX_OpMode;
    	       DEBUG_RFIU_P2("Invalid OPMode command!\n");      
           }

           if(RX_OpMode != rfiuRX_OpMode)
           {
               rfiuRX_OpMode=RX_OpMode;
               if(sys8211TXWifiStat == MR8211_QUIT_WIFI)
               {
               #if USE_704x480_RESO    
                   if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
                   {
                       DEBUG_RFIU_P2("=NoWifi/Quad=\n");
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
                   }
                   else 
                   {
                       DEBUG_RFIU_P2("=NoWifi/Full=\n");
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                   }
               #else
                   if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
                   {
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
                   }
                   else 
                   {
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                   }
               #endif
             }
             else //on Wifi
             {
               #if USE_704x480_RESO    
                   if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
                   {
                       DEBUG_RFIU_P2("=Wifi/Quad=\n");
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_352x240);
                   }
                   else 
                   {
                       DEBUG_RFIU_P2("=Wifi/Full:%d=\n",rfiu_TX_P2pVideoQuality);
                       if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
                          //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                          sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                       else
                          //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                          sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                   }
               #else
                   if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD )
                   {
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
                   }
                   else 
                   {
                       //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                   }
               #endif
             }
           }
        #else
           rfiuRX_OpMode=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           rfiuRX_P2pVideoQuality=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]; //Lucian: Only valid in P2p mode
           rfiuRX_CamOnOff_Num=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
           rfiuRX_CamPerRF=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
        #endif
    	   DEBUG_RFIU_P2("Set RxOpMode=%d,Level=%d,RxCamNum=%d,CamPerRF=%d\r\n", rfiuRX_OpMode,rfiuRX_P2pVideoQuality,rfiuRX_CamOnOff_Num,rfiuRX_CamPerRF);      
           
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
           #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
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
           #if( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
               //rfiuciu1ZoomIn(par1,par2<<2,par3<<2);
               ciu1ZoomOnOff=par1;
               ciu1ZoomXpos=par2<<2;
               ciu1ZoomYpos=par3<<2;
               ciu1ZoomStart=1;
           #else
               rfiuciu1ZoomIn(par1,par2<<3,par3<<3);
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
        #if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692 && UI_PROJ_OPT == 0)
            if (par1 & 0x03)        
                gRfiuUnitCntl[RFUnit].RFpara.MD_en         = 3;
            else
                gRfiuUnitCntl[RFUnit].RFpara.MD_en         =(par1 & 0x03);
        #else

           gRfiuUnitCntl[RFUnit].RFpara.MD_en         =par1 & 0x03;
        #endif
           gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day  =(par1>>2) & 0x07;
           gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night=(par1>>5) & 0x07;
           gRfiuUnitCntl[RFUnit].RFpara.MD_Trig       =0;
           uiMenuSet_TX_MOTION(gRfiuUnitCntl[RFUnit].RFpara.MD_en,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night);
        #if 0 //Mask Area: 

        #endif       
    	   DEBUG_RFIU_P2("Set MD Config: 0x%x\r\n", par1);      
    	}
        else if(CmdType==RFRXCMD_SET_PIRCFG)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           gRfiuUnitCntl[RFUnit].RFpara.PIR_en=par1 & 0x01;
           gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;

         #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
           uiMenuSet_TX_PIR(gRfiuUnitCntl[RFUnit].RFpara.PIR_en);
         #endif
    	   DEBUG_RFIU_P2("Set PIR Config: 0x%x\r\n", par1);      
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
    	   //DEBUG_RFIU_P2("%c%c%c%c%c%c%c", (char)par1,(char)par2,(char)par3,(char)par4,(char)par5,(char)par6,(char)par7);  
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
        else if(CmdType==RFRXCMD_SET_TIMESTAMP)
        {
           par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[0];
           gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=par1;

    	   DEBUG_RFIU_P2("Set TX Time Stamp: %d\r\n", par1);      
    	}
        else if(CmdType==RFRXCMD_SET_MASKAREA_VGA)
        {
           count=20; //Lucian: 第一條block,全部mask.
        #if HW_MD_SUPPORT   
           for(i=0;i<count;i++)
              MD_blk_Mask_VGA[RFUnit+1][i]=1;
        #endif   
           
           for(j=0;j<9;j++)
           {
              par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[j];
              for(i=0;i<8;i++)
              {
                 for(k=0;k<2;k++)
                 {
                 #if HW_MD_SUPPORT
                    MD_blk_Mask_VGA[RFUnit+1][count]=par1 & 0x01;
                    MD_blk_Mask_VGA[RFUnit+1][count+20]=par1 & 0x01;
                 #endif
                    count ++;
                 }
                 if( (count % 20)==0 )
                    count +=20;
                 
                 par1 >>=1;
              }
           }       
        #if MD_DEBUG_ENA   
           DEBUG_RFIU_P2("\nSet TX MaskArea VGA:%x %x %x %x %x %x %x %x %x\n",
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
           for ( i = 0; i < MD_BLOCK_NUM_MAX; i++)
           {
                if (i%20 == 0)
                {
                    DEBUG_RFIU_P2("\n");
                    DEBUG_RFIU_P2("Line %02d : ", i/20);
                }
                DEBUG_RFIU_P2("%d ",MD_blk_Mask_VGA[RFUnit+1][i]);
           }
        #endif
    	}
        else if(CmdType==RFRXCMD_SET_MASKAREA_HD)
        {
           count=20; //Lucian: 第一條block,全部mask.
        #if HW_MD_SUPPORT   
           for(i=0;i<count;i++)
              MD_blk_Mask_HD[RFUnit+1][i]=1;
        #endif   
           for(j=0;j<9;j++)
           {
              par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[j];
              for(i=0;i<8;i++)
              {
                 for(k=0;k<2;k++)
                 {
                 #if HW_MD_SUPPORT
                    MD_blk_Mask_HD[RFUnit+1][count]=par1 & 0x01;
                    MD_blk_Mask_HD[RFUnit+1][count+20]=par1 & 0x01;
                 #endif
                    count ++;
                 }
                 if( (count % 20)==0 )
                    count +=20;
                 
                 par1 >>=1;
              }
           }       
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
           for ( i = 0; i < MD_BLOCK_NUM_MAX; i++)
           {
                if (i%20 == 0)
                {
                    DEBUG_RFIU_P2("\n");
                    DEBUG_RFIU_P2("Line %02d : ", i/20);
                }
                DEBUG_RFIU_P2("%d ",MD_blk_Mask_HD[RFUnit+1][i]);
           }
        #endif
    	}
        else if(CmdType==RFRXCMD_SET_SLEEP)
        {
           DEBUG_RFIU_P2("Set TX Sleep\n"); 
           sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, RFUnit);
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
                  rfiuDoorBellTrig=0;
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
                  mduMotionDetect_Sensitivity_Config(gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
             #endif
                  DEBUG_RFIU_P2("Set MD Sens:H(%d,%d),M(%d,%d),L(%d,%d)\n",
                                MD_SensitivityConfTab[0][0],
                                MD_SensitivityConfTab[0][1],
                                MD_SensitivityConfTab[1][0],
                                MD_SensitivityConfTab[1][1],
                                MD_SensitivityConfTab[2][0],
                                MD_SensitivityConfTab[2][1]
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
              #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) 
                  rfiuVoxEna[RFUnit]=1;
                  if(par2 < 0xf0)
                     rfiuVoxThresh[RFUnit]=par2;
              #else
                  rfiuVoxEna[RFUnit]=par1;
                  rfiuVoxThresh[RFUnit]=par2;
              #endif             
                  DEBUG_RFIU_P2("Set Vox Config:ONOFF=%d,THR=%d\n",par1,par2);  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_SCHED)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  par3=gRfiuUnitCntl[RFUnit].RX_CMD_Data[3];
                  par4=gRfiuUnitCntl[RFUnit].RX_CMD_Data[4];
                  par5=gRfiuUnitCntl[RFUnit].RX_CMD_Data[5];
                  par6=gRfiuUnitCntl[RFUnit].RX_CMD_Data[6];
                  //--Compare UI setting--//
                  SaveData = 0;
                  #if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
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
                      if (par6 == 0)
                          SaveData = 1;
                  #endif
                  //----------------------//
                  if(SaveData)
                  {
                     #if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
                     uiMenuAction(UI_MENU_SETIDX_LIGHT_TIMER);
                     #endif
                     sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
                  }
                           
                  DEBUG_RFIU_P2("--Set SCHED Config--\n");  
             }
             else if(CmdExtType==RFRXCMD_SETEXT_SATURATION)
             {
                   par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   mdset=MotionDetect_en;
                   if(mdset)
                   {
                   #if HW_MD_SUPPORT
                       mduMotionDetect_ONOFF(0);
            	   #endif
                   }
                   sysProjectSetSensorChrome(par1);
                   if(mdset)
                   {
                   #if HW_MD_SUPPORT
                       mduMotionDetect_ONOFF(1);
            	   #endif
                   }

            	   DEBUG_RFIU_P2("Set Saturation %d\r\n", par1);      
           
    	     }
             else if(CmdExtType==RFRXCMD_SETEXT_VOLUME)
             {
                   par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   //control volume//
                   sysProjectSetAudioVolume(par1);
            	   DEBUG_RFIU_P2("Set volume %d\r\n", par1);      
           
    	     }
             else if(CmdExtType==RFRXCMD_SETEXT_LIGHTONOFF)
             {
                   par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                   //control Light On or OFF //
            	   DEBUG_RFIU_P2("Set Light %d\r\n", par1);      
                   sysProjectSetLightOnOff(par1);           
    	     }
             else if(CmdExtType==RFRXCMD_SETEXT_SNAPSHOT)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("--TX Snap shot:%d--\n",par1);
                  //sysbackSetEvt(SYS_BACK_RFI_TX_SNAPSHOT,par1);
                  sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_SNAPSHOT,par1);
             }
             else if(CmdExtType==RFRXCMD_SETEXT_MUSICCTL)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("--TX Set PlayMuicIC:%d--\n", par1);
                  #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I))
                  gpioMusicCtr(par1);
                  #endif
             }
             else if(CmdExtType==RFRXCMD_SETEXT_PHOTOTIME)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];

                  DEBUG_RFIU_P2("--TX Set Photo time:[%d:%2d]--\n",par1,par2);
                  #if TX_SNAPSHOT_SUPPORT
                  RTC_Get_Time(&dateTime);
                  temp=(par1*60+par2);
                  if(rfiuTXSnapCheck.SnapTimeInMin != temp )
                  {
                      rfiuTXSnapCheck.SnapTimeInMin=temp;
                      if( ((temp - (dateTime.hour*60+dateTime.min)) > 5) ||  ((temp - (dateTime.hour*60+dateTime.min)) < 0) )
                      {
                          rfiuTXSnapCheck.SnapStatus=RF_TXPHOTOSTA_NONE;
                      }
                      else
                      {
                          rfiuTXSnapCheck.SnapStatus=RF_TXPHOTOSTA_DONE_OK;
                      }
                      
                  }
                  #endif
             }
             else if(CmdExtType==RFRXCMD_SETEXT_VOCTEST)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("Set VOC Test: %d\r\n",par1);  
                  if(par1 == 0)
                  {
                     rfiuSetVOCTest_TX();
                  }
                  else
                  {
                     rfiuSetDustTest_TX();
                  }
                     
             }
             else if(CmdExtType==RFRXCMD_SETEXT_PUSHAPPMSG)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  DEBUG_RFIU_P2("Push APP Msg: %d\r\n",par1);  
                  if(par1 == 0)
                  {
                      // Push Message-0
                  #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)       
                    #if TX_PUSH_APPMSG
                      sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT,0, 0);
                    #endif
                  #endif
                  }
                  else
                  {
                  }
                     
             }    
             else if(CmdExtType==RFRXCMD_SETEXT_FBAPPSTA)
             {
                  par1=gRfiuUnitCntl[RFUnit].RX_CMD_Data[1];
                  par2=gRfiuUnitCntl[RFUnit].RX_CMD_Data[2];
                  DEBUG_RFIU_P2("APPSTA:%d,%d\r\n",par1,par2); 
                  //add code by Ted
                     
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
        int RX_recvDataPktCnt;
        
        u8 RSSI,RSSI_Wifi;
        unsigned int t5,t6;
        int CE181_CH_Mask[RFI_DAT_CH_MAX];
        u8 NextDAT_CH;
        int MinCh,MinVal;
        u32 Tmp;
#if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
        u8 dayLevel;
#endif

        DEF_RFIU_DAT_CH_STATISTICS TX_CH_Stat[RFI_DAT_CH_MAX+1]; //Lucian: used in TX-end.
        DEF_RFIU_FEC_TYPE_STATISTICS TX_FEC_Stat[RFI_FEC_TYPE_MAX];
        
        unsigned int t1,t2,dt,BitRate;
        unsigned int t3,t4;
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

     #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
        int TX_EnterKeepMode=0;
        int TX_KEEP_ErrorCnt=0;
     #endif
        int t7,t8;
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
        A7196_CH_sel(RFUnit+1,100);
#endif
            
        //---Setup test environment---//
     #if(RF_PAIR_EN)
        TX_PAIR_ACKflag=0;
        TX_SYC_ErrorCnt=0;
     #endif
     
        gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;

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
     

        OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*8), OS_FLAG_SET, &err);
        gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;

        //gRfiuUnitCntl[RFUnit].TX_Task_Stop=0;
        timerCountRead(guiRFTimerID, &t1);
        timerCountRead(guiRFTimerID, &t3);
        timerCountRead(guiRFTimerID, &t5);
        timerCountRead(guiRFTimerID, &t7);

        DEBUG_RFIU_P2("\n============rfiu_Tx_Task_UnitX(%d)==========\n",RFUnit);
    	while(1)
    	{
    	    gRfiuUnitCntl[RFUnit].RunCount ++;
    	    if(gRfiuUnitCntl[RFUnit].TX_Task_Stop)
    	    {
    	        DEBUG_RFIU_P2("$");
                timerCountRead(guiRFTimerID, &t7);
                OSTimeDly(1);
                continue;
            }
            
    	    switch(gRfiuUnitCntl[RFUnit].OpMode)
    	    {
    		case RFIU_TX_MODE:
                    timerCountRead(guiRFTimerID, &t7);
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
                                                        prev_DAT_CH_sel,
                                                        TX_CH_Stat,
                                                        TX_FEC_Stat
                                                       );

                        if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
                        {
                            //NextDAT_CH=gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.DAT_CH;
                            NextDAT_CH=ACK_CH_sel;


                            if(CE181_CH_Mask[NextDAT_CH] != 0 )
                            {
                               timerCountRead(guiRFTimerID, &Tmp);
                               NextDAT_CH= Tmp & (RFI_DAT_CH_MAX-1);
                               MinVal=CE181_CH_Mask[NextDAT_CH];
                               MinCh=NextDAT_CH;
                               
                               for(i=0;i<RFI_DAT_CH_MAX;i++)
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

                               if(CE181_CH_Mask[NextDAT_CH]!=0)
                               {
                                  NextDAT_CH=MinCh;
                               }
                            }
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
                     #endif

    				 #if RF_CMD_EN
                        if((gRfiuUnitCntl[RFUnit].TXCmd_en == 1) && (TXCmd_Valid==1) )
    				       TX_UsrData_next |= RFIU_USRDATA_CMD_CHEK ; //enable command 
    				    else
                           TX_UsrData_next &= (~RFIU_USRDATA_CMD_CHEK);

                        #if( (HW_BOARD_OPTION  == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) || \
                            (HW_BOARD_OPTION == MR8120_TX_RDI_CA530) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||\
                            (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
                          gRfiuUnitCntl[RFUnit].RFpara.PIR_en=1;
                        #elif ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                          gRfiuUnitCntl[RFUnit].RFpara.PIR_en=0;
                        #endif
                        //if( gRfiuUnitCntl[RFUnit].RFpara.MD_en  || 1)
                        if( gRfiuUnitCntl[RFUnit].RFpara.MD_en  || gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
                        {
                            //if(1)
                            if(gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
                            {
                            #if((HW_BOARD_OPTION  == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) ||\
                                (HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||\
                                (HW_BOARD_OPTION == MR8120_TX_TRANWO2)||(HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) ||\
                                (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||(HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
                                (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                                #if (PIR_TRIGER_ACT_HIGH == 1)
                                    gpioGetLevel(GPIO_GROUP_PIR, GPIO_BIT_PIR, &level);
                                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=level;  //Active High
                                #else
                                    gpioGetLevel(GPIO_GROUP_PIR, GPIO_BIT_PIR, &level);
                                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=!level;  //Active Low
                                #endif
                            #elif((HW_BOARD_OPTION  == MR8120_TX_HECHI) )
                                gpioGetLevel(GPIO_GROUP_PIR, GPIO_BIT_PIR, &level);
                                if( (level==0) || rfiuDoorBellTrig )
                                   gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=1;  //Active Low
                                else
                                   gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;
                                #if 0
                                if(gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
                                {
                                    DEBUG_RFIU_P2("PIR ");
                                }
                                #endif
                            #elif(HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
                                if (rfiuVoxTrigFlag == 1)
                                {
                                    DEBUG_RFIU_P2("Vox_PIR Triger\n");
                                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig = 1;
                                    rfiuVoxTrigFlag = 0;
                                }
                                else
                                    gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig = 0;
                            #else
                                level=0;
                                gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=level;
                            #endif
                            }
                            else
                                gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig=0;

                            if(gRfiuUnitCntl[RFUnit].RFpara.MD_en)
                            {
                             #if((HW_BOARD_OPTION == MR8120_TX_RDI) || ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 0))||\
                                ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 1)) || ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 2)) ||\
                                ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 3)) || ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 6)) ||\
                                (HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 7))||\
                                (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))

                                if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
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
                                gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=MD_Diff;
                             #endif
                            }
                            else
                                gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=0;
                        #if ((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
                            if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig && gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
                            {
                            #if MD_DEBUG_ENA
                                DEBUG_RFIU_P2("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
                            #endif
                                TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
                            }
                            else
                                TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
                        #elif (((HW_BOARD_OPTION  == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 4)) || ((HW_BOARD_OPTION  == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 5)) ||\
                            (HW_BOARD_OPTION  == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION == MR8120S_TX_GCT_VM00))||\
                            (HW_BOARD_OPTION == MR8120_TX_GCT_VM00)
                            /*VMD Only*/
                            if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig)
                            {
                            #if MD_DEBUG_ENA
                                DEBUG_RFIU_P2("TrigSrc=(%d,%d),Lv=%d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day);
                            #endif
                                TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
                            }
                            else
                                TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
                        #elif (HW_BOARD_OPTION == MR8120_TX_RDI_CL692 && UI_PROJ_OPT == 0) //RDI said VMD && PIR start from CLM892732
                            if((gRfiuUnitCntl[RFUnit].RFpara.MD_Trig && gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig) || ( prevPIRTrigWLED && gRfiuUnitCntl[RFUnit].RFpara.MD_Trig))
                            {
                                if ((uiInScheduleLight == 1) || (uiInManualLight != 0))
                                {
                                    //DEBUG_GPIO("PIR Trigger Cnange Dimmer\n");
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
                                
                              #if MD_DEBUG_ENA
                                DEBUG_RFIU_P2("TrigSrc=(%d,%d),Lv=%d %d\n",gRfiuUnitCntl[RFUnit].RFpara.MD_Trig,gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Day ,gRfiuUnitCntl[RFUnit].RFpara.MD_Level_Night);
                              #endif
                                TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
                            }
                            else
                                TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);

                        #else
                            if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig || gRfiuUnitCntl[RFUnit].RFpara.PIR_Trig)
                            {
                                #if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
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
                                TX_UsrData_next |= RFIU_USRDATA_MDDIF_CHEK;
                            }
                            else
                                TX_UsrData_next &= (~RFIU_USRDATA_MDDIF_CHEK);
                        #endif
        		        }
    				 #endif      
                        rfiuTxSendDataState( RFUnit,
                                             gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en], 
                                             gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel], 
                                             gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel],
                                             gRfiuSyncWordTable[RFUnit],
                                             gRfiuCustomerCode[RFUnit],
                                             TX_UsrData_next
                                           );
                        //DEBUG_RFIU_P2("%d ",DAT_CH_sel);
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
                                 //DEBUG_RFIU_P2("-->TRIG:%d,%d\n",gRfiuUnitCntl[RFUnit].TXChgCHNext, (TimeCheck>>10) & 0x07 );
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
                           //sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, BitRate/100);
                           gRfiuUnitCntl[RFUnit].BitRate=BitRate/100; //Unit: 100 Kbps
                        #if FORCE_LINKBROKEN_LOWBR
                           if(BitRate<200)
                              EnterLowBitRateCnt ++;
                           else
                              EnterLowBitRateCnt=0;
                        #endif
                        
                           if(TX_CH_Stat[RFI_DAT_CH_MAX].BurstNum>10)
                           {
                             rfiuTXBitRate[0]=BitRate;
                             gRfiuUnitCntl[RFUnit].TX_PktCrtRate=TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum*100/TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum;
                             DEBUG_RFIU_P2("\n====>BR=%d kbps,%d/%d,TxBufFull=%d,%d KB,dt=%d,DLLDelay=%d\n",BitRate,TX_CH_Stat[RFI_DAT_CH_MAX].RecvPktNum,
                                           TX_CH_Stat[RFI_DAT_CH_MAX].SentPktNum,TxBufFullness,gRfiuUnitCntl[RFUnit].TxBufFullness,dt/10,(SdramDFI_PHYB >> 16) );

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
                           #if(SW_APPLICATION_OPTION==MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION==MR8100_RFCAM_TX1)
                             rfiu_TXCMD_Enc("BODYINFO",0);
                               #if TX_PUSH_APPMSG
                                    if(rfiuVoxTrigFlag)
                                    {
                                       sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT,0, 0);
                                       DEBUG_RFIU_P2("Push Vox Meg\n");
                                       rfiuVoxTrigFlag=0;
                                    }   
                               #endif
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
                    #if 1        
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
                            if(TX_CHG_CHCnt != ((RFTimer>>14) & 0x1) )
                            {
                               ACK_CH_sel = gRfiuUnitCntl[RFUnit].TXChgCHNext;
                               DEBUG_RFIU_P2("Chg CH:%d\n",ACK_CH_sel);
                               TX_CHG_CHflag=0;
                            }
                        }
                        else
                        {
                            TX_CHG_CHCnt= (RFTimer>>14) & 0x1;
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
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
                #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
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
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)  //Lucian: 考慮近距離Wifi 天線 @ dual mode VBM 
                    if(RSSI > 240) // 170
                        RSSI=0;
                 #else
                    if(RSSI > 220) // 170
                        RSSI=0;
                 #endif

                    if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_CE_181)
                    {
                        if(RSSI > RFIU_RSSI_THR)
                        {
                             //DEBUG_RFIU_P2("(%d,%d)",RSSI,ACK_CH_sel);
                             CE181_CH_Mask[ACK_CH_sel]=50000; // 5 sec
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
                        
                    #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                        if(sys8211TXWifiStat == MR8211_ENTER_WIFI)
                        {
                            rfiu_MaskWifiUsedCH(rfiu_TX_WifiCHNum,CE181_CH_Mask);
                        }
                    #endif
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
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                    else if( RXACK_KEEP == rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp ) )
                    {
                       gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_WAITACK;          
                       DEBUG_RFIU_P2("\n KEEP \n");
                       TX_UsrACK =TX_UsrACK;
                       TxAckRetryCnt ++;                    
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
                       rfiuDoorBellTrig=0;
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
                    if( (dt>14000) || (EnterLowBitRateCnt>3) )
                 #else
                    if(dt > 14000)
                 #endif
                    {
                       DEBUG_RFIU_P2("dt=%d\n",dt);
                       TxAckRetryCnt=0;
                       OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*8), OS_FLAG_SET, &err);
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
                       DEBUG_RFIU_P2("------>Link Error! Go to Sync:%d,%d\n", gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime);
                    #else
                       #if RFI_SELF_TEST_TXRX_PROTOCOL
                        if(gRfiuUnitCntl[RFUnit].OpMode == RFIU_SYNC_MODE)
                            OSTaskChangePrio(RFIU_TASK_PRIORITY_UNIT0, RFIU_TASK_PRIORITY_HIGH);
                       #endif
                        
                        gRfiuUnitCntl[RFUnit].OpMode=RFIU_SYNC_MODE;
                        gRfiuUnitCntl[RFUnit].OpState=RFIU_TX_STATE_INIT;
                        DEBUG_RFIU_P2("------>Link Error! Go to Sync:%d,%d\n",gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime);
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
                #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                    if(TX_EnterKeepMode)
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

                #if 1   //Cal Sync time
                    timerCountRead(guiRFTimerID, &t8);
                    if(t7 >= t8)
                      dt=t7-t8;
                    else
                      dt=(t7+TimerGetTimerCounter(TIMER_7))-t8;

                    if(dt==0)
                      dt=1;   

                    if(dt > 100000) //  10 sec
                    {
                    #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) 
                       DEBUG_RFIU_P2("Sync over 10 sec\n");
                       if(sys8211TXWifiStat == MR8211_ENTER_WIFI) 
                       {
                            rfiuRX_OpMode &= (~RFIU_RX_OPMODE_QUAD);
                            switch(rfiu_TX_P2pVideoQuality)
                            {
                            case 0:
                                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_1280X720)
                                {
                                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1280X720;
                                   //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                                   sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
                                }
                                break;
                            case 1:
                            case 2:
                                if(uiMenuVideoSizeSetting != UI_MENU_VIDEO_SIZE_704x480)
                                {
                                   uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
                                   //sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                                   sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_704x480);
                                }
                                break;
                            }
                       }
                       #if TX_PUSH_APPMSG
                            if(rfiuVoxTrigFlag)
                            {
                               sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT,0, 0);
                               DEBUG_RFIU_P2("Push Vox Meg\n");
                               rfiuVoxTrigFlag=0;
                            }   
                       #endif
                    #endif
                       timerCountRead(guiRFTimerID, &t7);
                    }
                #endif
                       
                #if FORCE_LINKBROKEN_LOWBR
                    EnterLowBitRateCnt=0;
                #endif
                    //==Chanel selection==//     
                    TXWrapErrMonitor=0;
                #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M))
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                #endif
                    //==Wait ACK==//
                #if( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
                    rfiuTxWaitACKState( RFUnit,
                                        RFI_VITBI_DISA, 
                                        RFI_RS_T12, 
                                        RFI_VITBI_CR4_5,
                                        gRfiuSyncWordTable[RFUnit],
                                        gRfiuCustomerCode[RFUnit],
                                        RFI_TX_WAIT_TIME
                                      ); 
                #else
                    rfiuTxWaitACKState( RFUnit,
                                        RFI_VITBI_DISA, 
                                        RFI_RS_T12, 
                                        RFI_VITBI_CR4_5,
                                        gRfiuSyncWordTable[RFUnit],
                                        gRfiuCustomerCode[RFUnit],
                                        RFI_TX_WAIT_TIME*2
                                      ); 
                #endif
                    DEBUG_RFIU_P2("S");

                
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
                       {
                       #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                          if(TX_EnterKeepMode && (gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247) )
                          {
                          }
                          else
                             ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;                       
                       #else
                          ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                       #endif
                       }

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
                           #endif
                               DEBUG_RFIU_P2("-->TX Pair Retry\n");
                           }  
                       }
                    #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                       if(TX_EnterKeepMode)
                       {
                           TX_KEEP_ErrorCnt ++;
                           if(TX_KEEP_ErrorCnt > 120)
                           {
                               TX_EnterKeepMode=0;
                               TX_KEEP_ErrorCnt=0;
                               DEBUG_RFIU_P2("-->KEEP mode Leave!\n");
                           }
                       }
                    #endif
                       
                    }
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                    else if( (RXACK_KEEP == rfiuGetACKType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128,&Tmp )) && (TX_PAIR_ACKflag==0) )
                    {
                       DEBUG_RFIU_P2("4");
                       //------------------//
                       TX_TimeCheck=gRfiuTimer[RFUnit];

                       if(Tmp >= TX_TimeCheck)
                           gRfiuTimer_offset[RFUnit] = Tmp- TX_TimeCheck;
                       else
                           gRfiuTimer_offset[RFUnit] = Tmp + TIMER7_COUNT+1 - TX_TimeCheck;  
                       TX_EnterKeepMode=1;
                       TX_KEEP_ErrorCnt=0;

                       //------------------//
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                       gRfiuUnitCntl[RFUnit].ProtocolSel=rfiuGetACK2ProtocolType(RFUnit,gRfiuParm_Tx[RFUnit].TxRxOpBaseAddr+RFI_ACK_ADDR_OFFSET*128);
                       if(gRfiuUnitCntl[RFUnit].ProtocolSel==RFI_PROTOCOL_FCC_247)
                       {
                          ACK_CH_sel=DAT_CH_sel;
                       }
                       
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                       MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                       A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                       A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #endif

                       rfiuDecUsrData(TX_UsrACK,&gRfiuUnitCntl[RFUnit].TX_CtrlPara);
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.SeqenceNum ++;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpShift=0;
                       gRfiuUnitCntl[RFUnit].TX_CtrlPara.GrpDivs=0;

                       gRfiuUnitCntl[RFUnit].TX_CtrlPara_next.GrpShift=0;
                       
                       TX_UsrData_next= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].TX_CtrlPara);

                       rfiuTxSentKEEPState(  RFUnit,
                                             gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_en], 
                                             gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.RS_sel], 
                                             gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].TX_CtrlPara.Vitbi_sel],
                                             gRfiuSyncWordTable[RFUnit],
                                             gRfiuCustomerCode[RFUnit],
                                             TX_UsrData_next
                                          );
                        rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);

                    }
                 #endif
                    else
                    {   
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1 )
                       TX_EnterKeepMode=0;
                       TX_KEEP_ErrorCnt=0;
                 #endif   
                       TX_SYC_ErrorCnt=0;  // 避免RX先行跳回PAIR
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                       MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                       A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                       A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
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
                       DEBUG_RFIU_P2("1");
                    
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
                        MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
                    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
                    #endif

                    #if( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
                        rfiuTxWaitACKState( RFUnit,
                                            RFI_VITBI_DISA, 
                                            RFI_RS_T12, 
                                            RFI_VITBI_CR4_5,
                                            gRfiuSyncWordTable[RFUnit],
                                            gRfiuCustomerCode[RFUnit],
                                            RFI_TX_WAIT_TIME
                                          );    
                    #else
                        rfiuTxWaitACKState( RFUnit,
                                            RFI_VITBI_DISA, 
                                            RFI_RS_T12, 
                                            RFI_VITBI_CR4_5,
                                            gRfiuSyncWordTable[RFUnit],
                                            gRfiuCustomerCode[RFUnit],
                                            RFI_TX_WAIT_TIME*2
                                          );    
                    #endif
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
                               #if ( (RFI_SELF_TEST_TXRX_PROTOCOL && RFI_TEST_WRAP_OnPROTOCOL) || RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 )  
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
                                   
    					             DEBUG_RFIU_P2("-->RF-ID Saving\n");
                                     OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_SET, &err);
                                     OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_PAIR_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                                     gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_PAIR_OK;
    						       }
    							   OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
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

                               }
                           }
                           //DEBUG_RFIU_P2("\n");
                        }
                      #if RFI_SELF_TEST_TXRX_PROTOCOL
                        OSTaskChangePrio(RFIU_TASK_PRIORITY_HIGH,RFIU_TASK_PRIORITY_UNIT0);
                      #endif
                    }
                    
                    break;

              #if (RF_PAIR_EN)

                case RFIU_PAIR_MODE:
                     TXWrapErrMonitor=0;
                     timerCountRead(guiRFTimerID, &t7);
                    //==Chanel selection==//           
                 #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                    MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                 #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                    A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                 #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                    A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
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
                       TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                       DAT_CH_sel = (TX_UsrACK >>RFIU_USRDATA_DATACH_SHFT) & RFIU_USRDATA_DATACH_MASK;
                     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                       MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                       A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
                     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                       A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
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
                        MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
                    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                        A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
                    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                        A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
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
                           TX_UsrACK= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
                           //DEBUG_RFIU_P2("2:%x,%x",TX_UsrData_next & RFIU_USRDATA_SEQNUM_CHEK,TX_UsrACK & RFIU_USRDATA_SEQNUM_CHEK);

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
                                   #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
                                       A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                                   #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                                       A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
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
                        TxAckRetryCnt=0;
                        timerCountRead(guiRFTimerID, &t7);
                        OSFlagPost(gRfiuStateFlagGrp, RFIU_TX_STA_LINK_BROKEN<<(RFUnit*8), OS_FLAG_SET, &err);
                        gRfiu_Op_Sta[RFUnit]=RFIU_TX_STA_LINK_BROKEN;
                                        
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
                     #endif

                        if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                        {
                       #if RFIU_RX_AUDIO_RETURN    
                          if(guiIISPlayDMAId != 0xFF)
                          {
                              marsDMAClose(guiIISPlayDMAId);
                              guiIISPlayDMAId = 0xFF;
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
                        timerCountRead(guiRFTimerID, &t7);
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
                        #endif
                            timerCountRead(guiRFTimerID, &t3);
                        
                            if(gRfiu_WrapEnc_Sta[RFUnit] == RFI_WRAPENC_TASK_RUNNING)
                            {
                            #if RFIU_RX_AUDIO_RETURN    
                                  if(guiIISPlayDMAId != 0xFF)
                                  {
                                     marsDMAClose(guiIISPlayDMAId);
                                     guiIISPlayDMAId = 0xFF;
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
                            
                       }
                    break;
                #endif
     
                case RFIU_IDLE_MODE:
                default:
                    timerCountRead(guiRFTimerID, &t7);
                    OSTimeDly(2);
                    break;
                }                          

    	}
    }

    void rfiu_Rx_Task_UnitX(void* pData)
    {

    }

    int rfiu_MaskWifiUsedCH(int WifiCH,int CH_Mask[])
    {
    #if ICOMMWIFI_SUPPORT
        const u8 ScanTab[RFI_DAT_CH_MAX][9]=
        {
          { 0,  0,  0,  0,  1,  2,  3,  4, 5},   // 0
          { 0,  0,  0,  1,  2,  3,  4,  5, 6},   // 1
          { 0,  0,  1,  2,  3,  4,  5,  6, 7},   // 2
          { 0,  1,  2,  3,  4,  5,  6,  7, 8},   // 3
          { 0,  1,  2,  3,  4,  5,  6,  7, 8},   // 4
          { 1,  2,  3,  4,  5,  6,  7,  8, 9},   // 5
          { 2,  3,  4,  5,  6,  7,  8,  9,10},   // 6
          { 3,  4,  5,  6,  7,  8,  9, 10,11},   // 7
          { 4,  5,  6,  7,  8,  9, 10, 11,12},   // 8
          { 5,  6,  7,  8,  9, 10, 11, 12,13},   // 9
          { 6,  7,  8,  9, 10, 11, 12, 13,14},   // 10
          { 7,  8,  9, 10, 11, 12, 13, 14,15},   // 11
          { 7,  8,  9, 10, 11, 12, 13, 14,15},   // 12
          { 8,  9, 10, 11, 12, 13, 14, 15,15},   // 13
          { 9, 10, 11, 12, 13, 14, 15, 15,15},   // 14
          {10, 11, 12, 13, 14, 15, 15, 15,15}    // 15
        };   
        int i;

        for(i=0;i<9;i++)
        {
             CH_Mask[ ScanTab[WifiCH][i] ]=60000; // 6 sec
        }

    #else
        const u8 ScanTab[RFI_DAT_CH_MAX][7]=
        {
          { 0,  0,  0,  0,  1,  2,  3},   // 0
          { 0,  0,  0,  1,  2,  3,  4},   // 1
          { 0,  0,  1,  2,  3,  4,  5},   // 2
          { 0,  1,  2,  3,  4,  5,  6},   // 3
          { 1,  2,  3,  4,  5,  6,  7},   // 4
          { 2,  3,  4,  5,  6,  7,  8},   // 5
          { 3,  4,  5,  6,  7,  8,  9},   // 6
          { 4,  5,  6,  7,  8,  9, 10},   // 7
          { 5,  6,  7,  8,  9, 10, 11},   // 8
          { 6,  7,  8,  9, 10, 11, 12},   // 9
          { 7,  8,  9, 10, 11, 12, 13},   // 10
          { 8,  9, 10, 11, 12, 13, 14},   // 11
          { 9, 10, 11, 12, 13, 14, 15},   // 12
          {10, 11, 12, 13, 14, 15, 15},   // 13
          {11, 12, 13, 14, 15, 15, 15},   // 14
          {12, 13, 14, 15, 15, 15, 15}    // 15
        };   
        int i;

        for(i=0;i<7;i++)
        {
             CH_Mask[ ScanTab[WifiCH][i] ]=60000; // 6 sec
        }
     #endif   
     }       
   #endif

#endif


#if(RFIU_TEST || ( (SW_APPLICATION_OPTION != MR8120_RFCAM_TX1) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX1_6M) && (SW_APPLICATION_OPTION != MR8120_RFCAM_TX2) && (SW_APPLICATION_OPTION != MR8100_RFCAM_TX1)) && (SW_APPLICATION_OPTION != MR8211_RFCAM_TX1) )
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
#endif

    return -1;

}


#if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
int rfiu_DualMode_FindNextTx_Single(int RFUnit)
{
   static int Run=0;
   int i;
   int NextTx;
   u32 Bits;
   //-------------//
   NextTx=( Run & 0x03 );
   for(i=0;i<4;i++)
   {
      Bits=1 << NextTx;
      Run ++;
      if( (rfiuRX_CamOnOff_Sta & Bits) && (NextTx != sysRFRxInMainCHsel) )
      {
         return NextTx;
      }
      NextTx=(Run & 0x03);
   }

   return sysRFRxInMainCHsel;
   
}

#endif
int rfiuPutPacketMap2ACK(int RFUnit, unsigned char *ACKBufAddr,
                                  DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,
                                  unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext)
{
    unsigned int *pp;
    int i,count;
    unsigned int RFTimer;

    //=====//
    timerCountRead(guiRFTimerID, &RFTimer);
    if(RFUnit & 0x01)
        RFTimer += RF1_ACKTIMESHIFT;

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
  #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysRFRxInMainCHsel != RFUnit) )
       *pp = RXACK_KEEP;
    else
       *pp = RXACK_NORMAL;
  #else
   *pp = RXACK_NORMAL;
  #endif
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
  #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    *pp= ( ( (RFUnit & 0x07) | ((uiEnterScanMode & 0x01)<<3) | ( ( gRfiuUnitCntl[RFUnit].ProtocolSel & 0x03 )<<4) | 
           ((gRfiuUnitCntl[RFUnit].VMDSel & 0x01)<<6) | ( (CH_chg_flag & 0x01)<<7) ) | 
           (gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]<<8) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[9]<<16) | 
           ( (RX_CHG_CHNext & 0xff)<<24) 
          )^ 0x5a5aa5a5;
  #else
    *pp= ( ( (RFUnit & 0x07) | ( ( gRfiuUnitCntl[RFUnit].ProtocolSel & 0x03 )<<4) | ((gRfiuUnitCntl[RFUnit].VMDSel & 0x01)<<6) | ( (CH_chg_flag & 0x01)<<7) ) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[8]<<8) | (gRfiuUnitCntl[RFUnit].RX_CMD_Data[9]<<16) | ( (RX_CHG_CHNext & 0xff)<<24) )^ 0x5a5aa5a5;
  #endif
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

    if(RFUnit & 0x01)
        RFTimer += RF1_ACKTIMESHIFT;
    
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
 #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
      if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysRFRxInMainCHsel != RFUnit) )
       *pp = RXACK_KEEP;
      else
       *pp = RXACK_NORMAL;
 #else
   *pp = RXACK_NORMAL; 
 #endif
   pp ++;
   count ++;
   
   *pp= RFTimer;
   pp ++;
   count ++;
  
    //--RX Cmd Data[5~7]
    pp ++;
    count ++;

    //--Send Status: CH number bit3:0, --//
    temp= *pp ^ 0x5a5aa5a5; 
    temp= (temp & (~0x80)) | ((CH_chg_flag & 0x01)<<7);
    temp= (temp & (~0xff000000)) | ((RX_CHG_CHNext & 0xff) << 24);
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
           #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )  
               gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
               gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
           #else
               gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
               gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
           #endif
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
        #if( ((SW_APPLICATION_OPTION==MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION==MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )&&\
             (HW_BOARD_OPTION != MR8600_RX_MAYON)) 
               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__PAL_MODE)
                  iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
               else
                  iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
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

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__HD_SUPPORT)
                  gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.TX_SensorType=TX_SENSORTYPE_VGA;

               if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_TIMESTAMP_ON)
                   gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=1;
               else
                   gRfiuUnitCntl[RFUnit].RFpara.TX_TimeStampOn=0;

                if(gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STAT_BELL_ON)
                   gRfiuUnitCntl[RFUnit].RFpara.RX_DoorTrig=1;
               else
                   gRfiuUnitCntl[RFUnit].RFpara.RX_DoorTrig=0;

               gRfiuUnitCntl[RFUnit].ProtocolSel= (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STAT_PROTCOL_SEL) >> 10;
               gRfiuUnitCntl[RFUnit].VMDSel= (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_NEWMD_SUPPORT) >> 9;

               
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
           #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
               if(gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STAT_WIFI_ON)
                  gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn=0;
               if(gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STAT_LIGHT_ON)
                  gRfiuUnitCntl[RFUnit].RFpara.NightLight=1;
               else
                  gRfiuUnitCntl[RFUnit].RFpara.NightLight=0;
           #endif
               //------------------------------// 
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
                Temp_RX_MAC_Address[RFUnit] |= ((t_seed & 0xffff)<<16);  

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
				   //DEBUG_RFIU_P2("==>TX-%d Info Data REV:%d \n",RFUnit,*pp);
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
            else if(pRfiuPara->PktMap[2*i] & RFI_KEEP_ADDR_CHEKBIT)
            {
                pp=(unsigned int *)(pRfiuPara->TxRxOpBaseAddr+RFI_KEEP_ADDR_OFFSET*128);
                gRfiuUnitCntl[RFUnit].TX_Status= *pp;
                pp ++;

                gRfiuUnitCntl[RFUnit].TX_Status2= *pp;
                pp ++;

                gRfiuUnitCntl[RFUnit].RFpara.Temperature= *pp;
                pp ++;

                gRfiuUnitCntl[RFUnit].RFpara.Humidity= *pp;
                pp ++;
                
                gRfiuUnitCntl[RFUnit].RFpara.PM25= *pp;
                pp ++;

                gRfiuUnitCntl[RFUnit].RFpara.VoxTrigFlag= *pp;
                pp ++;

                gRfiuUnitCntl[RFUnit].RFpara.VoxTrigLev= *pp;
                pp ++;

                if(gRfiuUnitCntl[RFUnit].RFpara.VoxTrigFlag)
                {
                    gRfiuUnitCntl[RFUnit].TX_CMDPara[1]=gRfiuUnitCntl[RFUnit].RFpara.VoxTrigLev;
                    //DEBUG_RFIU_P2("==VOX==\n");
                    sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_VOXTRIG, RFUnit);
                    gRfiuUnitCntl[RFUnit].RFpara.VoxTrigFlag=0;
                }

                if(gRfiuUnitCntl[RFUnit].TX_Status2 & RFIU_TX_STAT_WIFI_ON)
                  gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn=1;
                else
                  gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn=0;

                gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_KEEP;

                //DEBUG_RFIU_P2(" KEEP-%d \n",RFUnit);
                return RXCMD_KEEP_CHECK;
            }
            
	    #endif	
        
       }
    }

    return RXCMD_NONE_CHECK;
}

int rfiuRxListenDataState( unsigned char RFUnit,
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
    int i;
    
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
    
    //gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_LISTEN;

    //----//
#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    RfiuReset(0);
    rfiuDataPktConfig_Rx(0,pRfiuPara_Rx);  //Rx
#elif RFI_TEST_4TX_2RX_PROTOCOL
    RfiuReset(RFUnit & 0x01);
    rfiuDataPktConfig_Rx(RFUnit & 0x01,pRfiuPara_Rx);  //Rx 
#else    
    RfiuReset(RFUnit);
    rfiuDataPktConfig_Rx(RFUnit,pRfiuPara_Rx);  //Rx
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

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#elif RFI_TEST_4TX_2RX_PROTOCOL
    RfiuReset(RFUnit & 0x01);      
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx);
#else
    RfiuReset(RFUnit);      
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
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

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
    RfiuReset(0);
    rfiuDataPktConfig_Tx(0,pRfiuPara_Tx);  //Tx
#elif RFI_TEST_4TX_2RX_PROTOCOL
    RfiuReset(RFUnit & 0x01);      
    rfiuDataPktConfig_Tx(RFUnit & 0x01,pRfiuPara_Tx);
#else
    RfiuReset(RFUnit);      
    rfiuDataPktConfig_Tx(RFUnit,pRfiuPara_Tx);
#endif

    return 1;
}



void rfiu_Tx_Task_UnitX(void* pData)
{

}

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
    u32 CheckWifiRun;
    int RF_CH_MaskTime[RFI_DAT_CH_MAX];
    int NextDAT_CH;
    int MinVal,MinCh;
    int RXAckCnt;   
    int MaxCh,MaxVal;
#if RF_FIXCH_OPTIM
    u32 RSSI_CheckCnt=0;    
    u32 OldFixCH;
#endif    
    int TX_ModeError=0;
#if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
    u32 KeepRun=0;
#endif
    u32 RxTimeoutMax;
    u32 RXLinkOKCnt=0;
    unsigned int t10,t11;
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
    
    RX_UsrData= rfiuEncUsrData(&gRfiuUnitCntl[RFUnit].RX_CtrlPara);
    RX_UsrACK=RX_UsrData;

#if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(0+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(0+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(0+1,100);
     #endif
#elif RFI_TEST_4TX_2RX_PROTOCOL
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel( (RFUnit & 0x01)+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel( (RFUnit & 0x01)+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel( (RFUnit & 0x01)+1,100);
     #endif
#else
     #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(RFUnit+1,0);
     #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(RFUnit+1,100);
     #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(RFUnit+1,100);
     #endif
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
    CheckWifiRun=0;
    
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

    gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_LISTEN;
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
    t11=t9=t8=t7=t5;
    gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_LINK_BROKEN;
    //gRfiuUnitCntl[RFUnit].RX_Task_Stop=0;

    while(1)
	{
	    if(gRfiuUnitCntl[RFUnit].RX_Task_Stop)
	    {
	        timerCountRead(guiRFTimerID, &t1);
            t11=t10=t9=t8=t7=t6=t5=t4=t3=t2=t1;	        
            RXLinkOKCnt=0;
            PktCnt=0;
            DEBUG_RFIU_P2("$");
            OSTimeDly(1);
            continue;
        }
        
	    switch(gRfiuUnitCntl[RFUnit].OpMode)
	    {
		case RFIU_RX_MODE:
            rfiuDecUsrData(RX_UsrACK,&gRfiuUnitCntl[RFUnit].RX_CtrlPara);
            
        #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)    
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(0+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
        #elif RFI_TEST_4TX_2RX_PROTOCOL
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel( (RFUnit & 0x01)+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
        #else
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[DAT_CH_sel]);
            #endif
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
                rfiuRxListenDataState(     RFUnit,
                                           gRfiuVitbiOnTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_en], 
                                           gRfiuRSCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.RS_sel], 
                                           gRfiuVitbiCodeTable[gRfiuUnitCntl[RFUnit].RX_CtrlPara.Vitbi_sel],
                                           gRfiuSyncWordTable[RFUnit],
                                           gRfiuCustomerCode[RFUnit],
                                           RFI_RX_WAIT_TIME
                                     );  
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

            if(dt > 20000)
            {
              BitRate = PktCnt*128*8/10/dt; //Unit: 100Kbps
              gRfiuUnitCntl[RFUnit].BitRate=BitRate;
              if(BitRate>32)
                BitRate=32;
              DEBUG_RFIU_P("RF-%d BitRate=%d\n",RFUnit,BitRate);

              if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysRFRxInMainCHsel==RFUnit) )
                 sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, BitRate);       
              else if((sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) && (sysRFRxInMainCHsel==RFUnit) )
                 sysbackSetEvt(SYS_BACK_DRAW_BIT_RATE, BitRate);
              
              t1=t2;
              PktCnt=0;
            }

            //==================//
        #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
            RSSI=rfiuWaitForInt_Rx(0,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //R
        #elif(RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_RXRX_PROTOCOL_B1B2)
            RSSI=rfiuWaitForInt_Rx(RFUnit & 0x01,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //Rx
           #if 1
            for(i= ( (RFUnit+1)& 0x01);i<4;i+=2)
            {
                if(gRfiuFCC247ChUsed[i][0] != -1)
                {
                    if( ( (gRfiuFCC247ChUsed[i][0]+1) == WifiScanCnt) || ( (gRfiuFCC247ChUsed[i][0]+0) == WifiScanCnt) || ( (gRfiuFCC247ChUsed[i][0]-1) == WifiScanCnt) )
                    {
                    #if(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)
                         RSSI_Wifi=110;
                    #else
                         RSSI_Wifi=RFIU_RSSI_THR;
                    #endif
                    }
                }
            }
           #endif
        #else
            RSSI=rfiuWaitForInt_Rx(RFUnit,&gRfiuParm_Rx[RFUnit],WifiScanCnt,&RSSI_Wifi);  //Rx
        #endif

            RSSI_Sum += RSSI;
            RSSI_cnt ++;
               
            if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
            {
               RSSI_CH_Sum[WifiScanCnt] += RSSI_Wifi;
               RSSI_CH_Cnt[WifiScanCnt] ++;
            }
            else
            {
               RSSI_CH_Sum[DAT_CH_sel] += RSSI;
               RSSI_CH_Cnt[DAT_CH_sel] ++;
            }
            WifiScanCnt = (WifiScanCnt+1) & 0x0f;
               
			PktCnt +=gRfiuParm_Rx[RFUnit].TxRxPktNum;

            //------------------------No packet received----------------------//
            if(gRfiuParm_Rx[RFUnit].TxRxPktNum==0)   
            {
               DEBUG_RFIU_P("<---No Data Packet received!!AckCH=%d,DatCH=%d\n",ACK_CH_sel,DAT_CH_sel);
               RX_UsrData=RX_UsrACK;
			   
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

               //-----------判斷是否time-out---------//

             #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                  RxTimeoutMax=40000;
               }
               else
               {
                  if(RFUnit==sysRFRxInMainCHsel)
                     RxTimeoutMax=40000;
                  else
                    RxTimeoutMax=100000;
               }

               if(sysEnSnapshot)
               {
                   t3=t4;
                   dt=0;
               }
             #else
               RxTimeoutMax=20000;
             #endif
             
               if(dt > RxTimeoutMax)
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
                       #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                           if(sysEnSnapshot == 0)
                              OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(RFUnit*8), OS_FLAG_SET, &err);
                       #else
                           OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(RFUnit*8), OS_FLAG_SET, &err);
                       #endif
                           if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
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
                                #if (((HW_BOARD_OPTION != MR8200_RX_TRANWO_D8593)&&(UI_PROJ_OPT != 10)) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS))
                                 if( i == RFUnit)
                                    sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 4);
                                 else
                                    sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, 5); 
                                #endif
                              }
                              else
                              {
                                #if (((HW_BOARD_OPTION != MR8200_RX_TRANWO_D8593)&&(UI_PROJ_OPT != 10)) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS))
                                 sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                                #endif
                              }
                           #else
                              sysback_RF_SetEvt(SYS_BACKRF_RFI_CLEAR_QUADBUF, RFUnit);
                           #endif
                           }
                       }
                     #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
                       Record_flag[RFUnit] = 1;
                     #endif

                     #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                       if(sysEnSnapshot == 0)
                          gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_BROKEN;
                     #else       
                       gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_BROKEN;
                     #endif
                       gRfiuUnitCntl[RFUnit].RXCmd_en=0;
                       gRfiuUnitCntl[RFUnit].RXCmd_Busy=0; 
                       gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn=0;
                       RX_LinkBrokenCnt=0;
                       RXLinkOKCnt=0;
                       timerCountRead(guiRFTimerID, &t11);
                       t3=t4;
                       t5=t6;
                       t7=t6;
                       t9=t8;

                       if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
                       {
                       #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)

                       #else
                          if(RFUnit & 0x01)
                             ACK_CH_sel = (ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                          else
                          {
                             ACK_CH_sel = (ACK_CH_sel-1);
                             if(ACK_CH_sel < 0)
                                ACK_CH_sel=RFI_ACK_CH_MAX-1;
                          }
                       #endif 
                          DAT_CH_sel = ACK_CH_sel;
                          gRfiuFCC247ChUsed[RFUnit][0]=-1;
                          gRfiuFCC247ChUsed[RFUnit][1]=-1;
                          RSSI_Sum=0;
                          RSSI_cnt=0;
                       }
                #if DETECT_RX_VOLUME
                       RxVolume[RFUnit] = 0;
                #endif
                       DEBUG_RFIU_P2("->RX%d Link Broken!0x%x\n",RFUnit,gRfiuSyncWordTable[RFUnit]);
                   }
               }

               //-------判斷pair time-out--------//
            #if(RF_PAIR_EN)   
               if(RX_PAIR_ACKflag >0 )   //enter pair ack 
               {  
                   RX_SYC_ErrorCnt++;
                   RXLinkOKCnt=0;
                   timerCountRead(guiRFTimerID, &t11);
                   if(RX_SYC_ErrorCnt >=16)
                   {
                     PairCnt ++;
    				 if(PairCnt >100)
    				 	PairCnt=0;
                     gRfiuSyncWordTable[RFUnit] = RFI_PAIR_SYNCWORD;
                     gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                     RX_PAIR_ACKflag=0;
                     RX_SYC_ErrorCnt =0;
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )      
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                     A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif RFI_TEST_4TX_2RX_PROTOCOL
                     A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                     A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif RFI_TEST_4TX_2RX_PROTOCOL
                     A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #endif
                     DEBUG_RFIU_P2("-->RX Pair Retry\n");
    				 sprintf(sysBackOsdString,"Paring%d..#%d  ",RFUnit+1,PairCnt);
    			     sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                   }  
               }
            #endif 
            
               RX_RecvPktLostCnt ++;
               RX_LinkBrokenCnt ++;
               RX_TimeCheck=0xffffffff;
               isCmd=RXCMD_NONE_CHECK;
            }
            else //-------------------Some packet received------------------//
            {  
               RX_RecvPktLostCnt=0;
               RX_LinkBrokenCnt=0;
            #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
               if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
               {
                   timerCountRead(guiRFTimerID, &t3);
               }
               else
               {
                   //if(RFUnit==sysRFRxInMainCHsel)
                        timerCountRead(guiRFTimerID, &t3);
               }
            #else
               timerCountRead(guiRFTimerID, &t3);
            #endif
               gRfiuUnitCntl[RFUnit].OpState=RFIU_RX_STATE_LISTEN;
               DEBUG_RFIU_P("<---%d Packet received. AckCH=%d,DatCH=%d\n",gRfiuParm_Rx[RFUnit].TxRxPktNum,ACK_CH_sel,DAT_CH_sel);
               RX_UsrData= (gRfiuParm_Rx[RFUnit].UserData_L) | (gRfiuParm_Rx[RFUnit].UserData_H<<16);
               rfiuDecUsrData(RX_UsrData,&gRfiuUnitCntl[RFUnit].RX_CtrlPara_next);
               RxAckRetryCnt=0;               
 
             #if RF_CMD_EN
               if(gRfiuUnitCntl[RFUnit].RXCmd_Busy && gRfiuUnitCntl[RFUnit].RXCmd_en)
               {
                  OS_ENTER_CRITICAL();
                  gRfiuUnitCntl[RFUnit].RXCmd_en=0;
                  gRfiuUnitCntl[RFUnit].RXCmd_Busy=0;
                  OS_EXIT_CRITICAL();
               }
           #if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3) || (UI_VERSION == UI_VERSION_RDI_4))
               if (1)   /*RDI 系列不做判斷*/
           #else
               if(gRfiuUnitCntl[RFUnit].RFpara.MD_en || gRfiuUnitCntl[RFUnit].RFpara.PIR_en)
           #endif
               {
                   gRfiuUnitCntl[RFUnit].RFpara.MD_Trig= (RX_UsrData >> RFIU_USRDATA_MDDIF_SHFT) & RFIU_USRDATA_MDDIF_MASK;
                   GMotionTrigger[RFUnit+MULTI_CHANNEL_LOCAL_MAX] |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig;
               #if MULTI_CHANNEL_VIDEO_REC
                   VideoClipOption[RFUnit + MULTI_CHANNEL_LOCAL_MAX].MD_Diff   |= gRfiuUnitCntl[RFUnit].RFpara.MD_Trig; // for motion detect prerecord
               #endif
               #if 0    
                   if(gRfiuUnitCntl[RFUnit].RFpara.MD_Trig)
                      DEBUG_RFIU_P2("TRIG ");
               #endif
               }
               else
                   gRfiuUnitCntl[RFUnit].RFpara.MD_Trig=0;
             #endif

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

               #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                  if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                  {
                      if(sysEnSnapshot ==0)
                      {
                         gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                         OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                      }

                  }
                  else
                  {
                      if(RFUnit==sysRFRxInMainCHsel)
                      {
                         gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                         OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                      }
                      else
                      {
                         gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_KEEP;
                      }
                  }
               #else    
                 #if (HW_BOARD_OPTION != MR8600_RX_SKYSUCCESS)
                  OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                 #endif
                  gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
               #endif
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
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )      
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                     A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif RFI_TEST_4TX_2RX_PROTOCOL
                     A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                     A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                  #elif RFI_TEST_4TX_2RX_PROTOCOL
                     A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                  #else  
                     A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                  #endif
               #endif
               
                     gRfiuUnitCntl[RFUnit].RX_Pair_Done=1;
                  #if(HW_BOARD_OPTION  != A1016_FPGA_BOARD)
                     spiWriteRF();
                  #endif
					 DEBUG_RFIU_P2("-->RF-ID Saving\n");
                     OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_SET, &err);
                     OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_PAIR_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                     gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_PAIR_OK;

                     RX_PAIR_ACKflag=0;
                     RX_SYC_ErrorCnt=0; 
                     RXLinkOKCnt=0;
                     timerCountRead(guiRFTimerID, &t11);
                  }
               }
			 #endif
               //-----------------Check Link Ok Count--------------//
               if(gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK)
               {
                   timerCountRead(guiRFTimerID, &t10);
                   if(t11 >= t10)
                      dt=t11-t10;
                   else
                      dt=(t11+TimerGetTimerCounter(TIMER_7))-t10;
                
                   RXLinkOKCnt ++;

                 #if 0
                   if(RXLinkOKCnt > 50*5)
                 #else
                   if(dt > 50000) //5 sec
                 #endif
                   {
                       if(gRfiu_WrapDec_Sta[RFUnit] != RFI_WRAPDEC_TASK_RUNNING)
                       {
                       #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                           DEBUG_RFIU_P2("=Warning! RFWrap not restart! ,Force Resync:%d,%d=\n",RFUnit,RXLinkOKCnt);
                           sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
                       #endif
                       }
                       else
                       {
                           //DEBUG_RFIU_P2("=WrapCheck:%d=\n",RFUnit);
                       #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)    
                           if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) && (sysEnSnapshot==0) )
                           {
                               if( (gRfiuUnitCntl[RFUnit].TX_PicWidth != 352) || (gRfiuUnitCntl[RFUnit].TX_PicHeight != 240) ) 
                               {
                                   DEBUG_RFIU_P2("-Quad,Force chg reso-\n");
                                   rfiu_RXCMD_Enc("RESO 352 240",RFUnit);
                                   //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * 4); //clear display buffer.
                               }
                           }
                           else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
                           {

                           }
                       #endif
                       }
                       RXLinkOKCnt=0;
                       t11=t10;
                   }
               }
               else
               {
                  RXLinkOKCnt=0;
                  timerCountRead(guiRFTimerID, &t11);
               }

               //-------------------- Receive Sync Packet and do something ---------------//
               if(RX_SYNC_ACKflag && (gRfiuSyncRevFlag[RFUnit] == 0) )
               {
               #if( (SW_APPLICATION_OPTION  ==  MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                   //Baby monitor don't use VMD
                   if(sysEnSnapshot ==0)
                   {
                       //DEBUG_RFIU_P2("==VOX EVT==\n");
                 #if RFIU_VOX_SUPPORT
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_VOXCFG, RFUnit); //Lucian: Fix TX bug
                 #endif
                       //Compare RXOpMode
                       if( (((gRfiuUnitCntl[RFUnit].TX_Status >>12) & 0x0f) != rfiuRX_OpMode) )
                       {
                           sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_SETOPMODE, RFUnit);
                           TX_ModeError=1;
                           DEBUG_RFIU_P2("\n-->RXOPDiff:%d,%d\n",rfiuRX_OpMode,((gRfiuUnitCntl[RFUnit].TX_Status >>12) & 0x0f));
                       }
                   }
               #else
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_SENDTXMDSENS, RFUnit); //Lucian: Send TX MD sensitivity definition
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RESENDTXMDCFG, RFUnit); //Lucian: Fix TX bug
                 #if RFIU_VOX_SUPPORT
                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_VOXCFG, RFUnit); //Lucian: Fix TX bug
                 #endif
                   //Compare RXOpMode
                   if( (((gRfiuUnitCntl[RFUnit].TX_Status >>12) & 0x0f) != rfiuRX_OpMode) ||  (((gRfiuUnitCntl[RFUnit].TX_Status >>16) & 0x0f) != rfiuRX_CamOnOff_Num))
                   {
                       sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_SETOPMODE, RFUnit);
                       DEBUG_RFIU_P2("\n-->RXOPDiff:%d,%d\n",rfiuRX_OpMode,((gRfiuUnitCntl[RFUnit].TX_Status >>12) & 0x0f));
                   }
               #endif
                                  
           #if RFIU_SUPPORT  

               #if( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                   if(sysEnSnapshot ==0)
                      OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                   gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                   if(TX_ModeError)
                   {
                       if(uiEnPair2Preview==0)
                         sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_CH_RESTART, RFUnit);
                       TX_ModeError=0;
                   }
                   else
                   {
                       if(uiEnPair2Preview==0)
                          sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_CH_RESTART, RFUnit);
                       TX_ModeError=0;
                   }
                   
                  #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                   if(gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn)
                   {
                       DEBUG_RFIU_P2("--RF-%d WIFI Visit--\n",RFUnit);
                   }
                   else
                   {
                       DEBUG_RFIU_P2("--RF-%d WIFI OFF--\n",RFUnit);
                   }
                  #endif
               #else
                   OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_OK<<(RFUnit*8), OS_FLAG_SET, &err);
                   gRfiu_Op_Sta[RFUnit]=RFIU_RX_STA_LINK_OK;
                   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
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

                   sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_CH_RESTART, RFUnit);
               #endif
           #endif
    		 
    		   #if(RF_PAIR_EN) 
    		       sprintf(sysBackOsdString,"                  ");
    			   sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
    	       #endif	
                   RX_SYNC_ACKflag=0; 
                   RSSI_Sum=0;
                   RSSI_cnt=0;
                   RXLinkOKCnt=0;
                   timerCountRead(guiRFTimerID, &t11);

               #if RF_AV_SYNCTIME_EN
                   DEBUG_RFIU_P2("-->Recv SYNC:%d:%d,%d,0x%x,%d,0x%x,%d,%d\n",
                                 RFUnit,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight,
                                 gRfiuUnitCntl[RFUnit].TX_Status,gRfiuUnitCntl[RFUnit].ProtocolSel,gRfiuUnitCntl[RFUnit].RFpara.RF_ID,
                                 gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime); 
               #else
                   DEBUG_RFIU_P2("-->Recv SYNC:%d:%d,%d,0x%x,%d,0x%x\n",
                                 RFUnit,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight,
                                 gRfiuUnitCntl[RFUnit].TX_Status,gRfiuUnitCntl[RFUnit].ProtocolSel,gRfiuUnitCntl[RFUnit].RFpara.RF_ID); 
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
                       gRfiuUnitCntl[RFUnit].BufWritePtr  = (gRfiuUnitCntl[RFUnit].BufWritePtr + gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpShift) & RFI_BUF_SIZE_MASK ;
                       gRfiuUnitCntl[RFUnit].WritePtr_Divs= gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpDivs;
                       OS_EXIT_CRITICAL();
                   }
                   else
                   {
                       if( (isCmd & RXCMD_PROTOCOL_CHECK) == 0)
                          DEBUG_RFIU_P2("->SeqNum Err:%d\n",RFUnit);
                       else
                       {
                       #if 0
                           if(gRfiuSyncRevFlag[RFUnit] == 1)
                           {
                              DEBUG_RFIU_P2("=Sync command error,Force Resync=\n");
                              sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
                           }
                       #endif
                       }
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
               #if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
                  if(RFUnit==sysRFRxInMainCHsel)
                  {
                       if( (KeepRun & 0x03)==0 )
                       {
                           if(gRfiuTxSwCnt[0] == (RFUnit & 0x01) )
                           {
                              NextTx=rfiu_DualMode_FindNextTx_Single(RFUnit);
                              if(NextTx < 0)
                                  DEBUG_RFIU_P2("Error! Find invalid TX\n");
                              else
                                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                           }    
                       }
                       else
                       {
                            NextTx=sysRFRxInMainCHsel;
                            if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                               OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                       }
                       KeepRun ++;
                   }
                  else
                  {
                      NextTx=sysRFRxInMainCHsel;
                      if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                         OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                  }
               #else
                  NextTx=sysRFRxInMainCHsel;
                  if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                     OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               #endif
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
               #if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
                  if(RFUnit==sysRFRxInMainCHsel)
                  {
                        if( (KeepRun & 0x03)==0 )
                        {
                           if( gRfiuTxSwCnt[RFUnit & 0x01] == ((RFUnit>>1) & 0x01) )
                           {
                              NextTx=rfiu_DualMode_FindNextTx_Single(RFUnit);
                              if(NextTx < 0)
                                  DEBUG_RFIU_P2("Error! Find invalid TX\n");
                              else
                                  OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx) , OS_FLAG_SET, &err);
                           }
                        }
                        else
                        {
                           NextTx=sysRFRxInMainCHsel;
                           if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                              OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                        }
                        KeepRun ++;
                  }
                  else
                  {
                      NextTx=sysRFRxInMainCHsel;
                      if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                         OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                  }
               #else
                  NextTx=sysRFRxInMainCHsel;
                  if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                     OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               #endif
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
               
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[RFUnit & 0x01] = ((RFUnit>>1) & 0x01);
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
                   }
               }
               else
               {
               #if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
                  if(RFUnit==sysRFRxInMainCHsel)
                  {
                     if(rfiuRX_CamOnOff_Num <= 2)
                     {
                         if( (KeepRun % 6)==0 )
                         {
                             if( gRfiuTxSwCnt[0] == (RFUnit & 0x03) )
                             {
                                NextTx=rfiu_DualMode_FindNextTx_Single(RFUnit);
                                if(NextTx < 0)
                                   DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                else
                                   OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                             }
                         }
                         else
                         {
                              NextTx=sysRFRxInMainCHsel;
                              if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                                 OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                         }
                     }
                     else
                     {
                         if( (KeepRun & 0x3)==0 )
                         {
                             if( gRfiuTxSwCnt[0] == (RFUnit & 0x03) )
                             {
                                NextTx=rfiu_DualMode_FindNextTx_Single(RFUnit);
                                if(NextTx < 0)
                                   DEBUG_RFIU_P2("Error! Find invalid TX\n");
                                else
                                   OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                             }
                         }
                         else
                         {
                              NextTx=sysRFRxInMainCHsel;
                              if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                                 OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                         }
                     }
                     KeepRun ++;
                  }
                  else
                  {
                      NextTx=sysRFRxInMainCHsel;
                      if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                         OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
                  }
               #else
                  NextTx=sysRFRxInMainCHsel;
                  if(rfiuRX_CamOnOff_Sta & (0x01 << NextTx ) )
                     OSFlagPost(gRfiu_nTx1RSwFlagGrp, (0x01 << NextTx ) , OS_FLAG_SET, &err);
               #endif
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
               
               OS_ENTER_CRITICAL();
               gRfiuTxSwCnt[0] = (RFUnit & 0x03);
               OS_EXIT_CRITICAL();
            }
          #endif

          //-------Send Wakeup------//
          #if RFIU_TX_WAKEUP_SCHEME
            if(gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_LISTEN)
            {   
                 gRfiuUnitCntl[RFUnit].WakeUpTxEn=0;
            }
            else
            {
                if(gRfiuUnitCntl[RFUnit].WakeUpTxEn == 1)
                {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                    A7130_CH_sel(0+1,gRfiuDAT_CH_Table[0]);
                #elif RFI_TEST_4TX_2RX_PROTOCOL
                    A7130_CH_sel((RFUnit & 0x01)+1,gRfiuDAT_CH_Table[0]);
                #else
                    A7130_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[0]);
                #endif
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                    A7196_CH_sel(0+1,gRfiuDAT_CH_Table[0]);
                #elif RFI_TEST_4TX_2RX_PROTOCOL
                    A7196_CH_sel((RFUnit & 0x01)+1,gRfiuDAT_CH_Table[0]);
                #else
                    A7196_CH_sel(RFUnit+1,gRfiuDAT_CH_Table[0]);
                #endif
            #endif
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
                    DEBUG_RFIU_P2("*");
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                    rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
                #elif RFI_TEST_4TX_2RX_PROTOCOL
                    rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
                #else
                    rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
                #endif
                }
            }
         #endif

            //----------------------------------Reply ACK-------------------------------------//
            //Select ACK channel
            timerCountRead(guiRFTimerID, &RFTimer);

  #if 1
            if(gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_FCC_247)
            {
                // Average RSSI and judge iff change channel
                timerCountRead(guiRFTimerID, &t6);
                if(t5 >= t6)
                  dt=t5-t6;
                else
                  dt=(t5+TimerGetTimerCounter(TIMER_7))-t6;
                if(dt > 10000/2)
                {
                    
                    if( (((RFTimer>>10) & 0x7) <= 6) && (((RFTimer>>10) & 0x7) >= 4) && (RX_CHG_CHflag==0) )
                    {
                       if(RSSI_cnt==0)
                         RSSI_cnt=1;
                       RSSI_avg=RSSI_Sum /RSSI_cnt;
                       //DEBUG_RFIU_P2("%d:%d:%d \n",RFUnit,RSSI_avg,ACK_CH_sel);
                       if(RSSI_avg > RFIU_RSSI_THR)
                       {
                          if( (gRfiuUnitCntl[RFUnit].RXCmd_en==0) && (RX_SYNC_ACKflag==0) && (RX_PAIR_ACKflag==0) && (gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK) && WifiScanRdy )
                          {
                              RX_CHG_CHflag=1;
                              TX_CHG_CHflag=1;
                              RX_CHG_CHCnt=(RFTimer>>13) & 0x3;
                              
                              RX_CHG_CHNext= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);                              

                              gRfiuFCC247ChUsed[RFUnit][1]=RX_CHG_CHNext;
                          #if 1    
                              gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
                              gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_CHGCH;
                              gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=(unsigned char)RX_CHG_CHNext;
                              gRfiuUnitCntl[RFUnit].RXCmd_en = 1;
                          #endif

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
                       #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                          if( (gRfiuUnitCntl[RFUnit].RXCmd_en==0) && (RX_SYNC_ACKflag==0) && (RX_PAIR_ACKflag==0) && (gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK) )
                          {
                              if(RSSI_CheckCnt >10)
                              {
                                  if(gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn)
                                  {
                                      RX_CHG_CHflag=1;
                                      TX_CHG_CHflag=1;
                                      RX_CHG_CHCnt=(RFTimer>>13) & 0x3;
                                      RX_CHG_CHNext= rfiuNoWifiCHsel(RFUnit,RSSI_CH_Avg);
                                      gRfiuFCC247ChUsed[RFUnit][1]=RX_CHG_CHNext;
                                      gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
                                      gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_CHGCH;
                                      gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=(unsigned char)RX_CHG_CHNext;
                                      gRfiuUnitCntl[RFUnit].RXCmd_en = 1;
                                      DEBUG_RFIU_P2("-->Scan:%d:%d,%d\n",RFUnit,RX_CHG_CHNext, (RFTimer>>10) & 0x0f );
                                  }
                                  RSSI_CheckCnt=0;
                              }       
                              RSSI_CheckCnt ++; 
                          }
                       #endif
                       }
                       #endif

                       
                       t5=t6;
                       RSSI_Sum=0;
                       RSSI_cnt=0;
                    }
                }

                if(RX_CHG_CHflag)
                {
                    if( RX_CHG_CHCnt != ((RFTimer>>13) & 0x3) )
                    {
                       ACK_CH_sel = RX_CHG_CHNext;
                       gRfiuFCC247ChUsed[RFUnit][0]=ACK_CH_sel;
                       gRfiuFCC247ChUsed[RFUnit][1]=-1;

                       DEBUG_RFIU_P2("Chg-%d,%d\n",RFUnit,ACK_CH_sel );
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
                           if(((RFTimer>>10) & 0x7) < 2)
                               TX_CHG_CHflag=0;
                       }
                       //if(RFUnit==0)
                          //DEBUG_RFIU_P2("-%x",((RFTimer>>10) & 0x3f));
                    }

                }
                DAT_CH_sel=ACK_CH_sel;

            #if 1 //RF_SCAN_WIFI_CH
                timerCountRead(guiRFTimerID, &t8);
                if(t9 >= t8)
                  dt=t9-t8;
                else
                  dt=(t9+TimerGetTimerCounter(TIMER_7))-t8;
                if(dt > 80000)// 8 sec
                {   
                    //DEBUG_RFIU_P2("\n");
                    for(i=0;i<RFI_DAT_CH_MAX;i++)
                    {
                       if(RSSI_CH_Cnt[i]==0)
                          RSSI_CH_Cnt[i]=1;
                       RSSI_CH_Avg[i]=RSSI_CH_Sum[i]/RSSI_CH_Cnt[i]; 
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
  #else
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
                       if(RSSI_avg > RFIU_RSSI_THR)
                       {
                          if( (gRfiuUnitCntl[RFUnit].RXCmd_en==0) && (RX_SYNC_ACKflag==0) && (RX_PAIR_ACKflag==0) && (gRfiu_Op_Sta[RFUnit]==RFIU_RX_STA_LINK_OK) )
                          {
                              RX_CHG_CHflag=1;
                              TX_CHG_CHflag=1;
                              RX_CHG_CHCnt=(RFTimer>>14) & 0x3;
                              
                          #if (RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_4TX_2RX_PROTOCOL)
                              if(RFUnit & 0x01)
                                RX_CHG_CHNext = (RFTimer & 0x0f) | 0x01;
                              else
                                RX_CHG_CHNext = (RFTimer & 0x0f) & 0x0e;
                              
                              while(0==CheckFCC247ChDiff(RX_CHG_CHNext,RFUnit))
                              {
                                 RX_CHG_CHNext=(RX_CHG_CHNext+2)  % RFI_ACK_CH_MAX;
                              }   
                          #else
                              RX_CHG_CHNext= (RFTimer & 0x0f) % RFI_ACK_CH_MAX;
                              while(0==CheckFCC247ChDiff(RX_CHG_CHNext,RFUnit))
                              {
                                 RX_CHG_CHNext=(RX_CHG_CHNext+1)  % RFI_ACK_CH_MAX;
                              }
                          #endif
                              

                              gRfiuFCC247ChUsed[RFUnit][1]=RX_CHG_CHNext;
                          #if 1    
                              gRfiuUnitCntl[RFUnit].RXCmd_Type = RFRXCMD_SET_EXTEND;
                              gRfiuUnitCntl[RFUnit].RX_CMD_Data[0]= RFRXCMD_SETEXT_CHGCH;
                              gRfiuUnitCntl[RFUnit].RX_CMD_Data[1]=(unsigned char)RX_CHG_CHNext;
                              gRfiuUnitCntl[RFUnit].RXCmd_en = 1;
                          #endif

                              //DEBUG_RFIU_P2("-->Trig-%d:%d,%d\n",RFUnit,RX_CHG_CHNext, (RFTimer>>10) & 0x0f );
                          }
                       }
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

                       DEBUG_RFIU_P2("Chg-%d,%d\n",RFUnit,ACK_CH_sel );
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
                DAT_CH_sel=ACK_CH_sel;
                
            }  
  #endif
            else
            {
                if(RFUnit & 0x01)
                     RFTimer +=RF1_ACKTIMESHIFT;

            #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
                ACK_CH_sel= (RFTimer>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
    		#else
                if(RFUnit & 0x01)
                   ACK_CH_sel= ( (gRfiuTimer[RFUnit]+RF1_ACKTIMESHIFT) >>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
                else
                   ACK_CH_sel= (gRfiuTimer[RFUnit]>>RFI_ACK_CH_PIRIOD) % RFI_ACK_CH_MAX;  // 102.4 ms to change ACK channel.
            #endif

            #if 1
                if( (gRfiuUnitCntl[RFUnit].ProtocolSel == RFI_PROTOCOL_ISOWIFI) )
                {
                    timerCountRead(guiRFTimerID, &t6);
                    if(t5 >= t6)
                      dt=t5-t6;
                    else
                      dt=(t5+TimerGetTimerCounter(TIMER_7))-t6;
                    if(dt > 50000)// 2 sec
                    {   
                        for(i=0;i<RFI_DAT_CH_MAX;i++)
                        {
                           if(RSSI_CH_Cnt[i]==0)
                              RSSI_CH_Cnt[i]=1;
                           RSSI_CH_Avg[i]=RSSI_CH_Sum[i]/RSSI_CH_Cnt[i];
                           

                           if(RSSI_CH_Avg[i]>RFIU_RSSI_THR)
                              RF_CH_MaskTime[i]=160000;  
                           
                           //if( (RFUnit & 0x01) == 0)
                              //DEBUG_RFIU_P2("%3d ",RSSI_CH_Avg[i]);

                           RSSI_CH_Sum[i]=0;
                           RSSI_CH_Cnt[i]=0;
                        }

                        //if( (RFUnit & 0x01) == 0)
                           //DEBUG_RFIU_P2("\n");
                        
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
                #if 1
                    if( RF_CH_MaskTime[ (RandHoppTab[0][ACK_CH_sel]) ] == 0)
                       DAT_CH_sel=RandHoppTab[0][ACK_CH_sel];
                #else
                    if( RF_CH_MaskTime[ (RandHoppTab[0][RXAckCnt]) ] == 0)
                       DAT_CH_sel=RandHoppTab[0][RXAckCnt];
                #endif    
                    else
                    {
                       NextDAT_CH= t6 & (RFI_DAT_CH_MAX-1);
                       MinVal=RF_CH_MaskTime[NextDAT_CH];
                       MinCh=NextDAT_CH;
                       
                       for(i=0;i<RFI_DAT_CH_MAX;i++)
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
          
            
          
      #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel(0+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(0+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
            A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] ); 
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(0+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
            A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] ); 
          #endif
      #elif RFI_TEST_4TX_2RX_PROTOCOL
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
            A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel((RFUnit & 0x01)+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
            A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
          #endif
      #else
          #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )   
            MV400_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);
          #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(RFUnit+1,gRfiuACK_CH_Table[RFUnit][ACK_CH_sel]);//
          #endif
      #endif

            RX_UsrData = (RX_UsrData & (~(RFIU_USRDATA_DATACH_MASK<<RFIU_USRDATA_DATACH_SHFT))) | ((DAT_CH_sel & RFIU_USRDATA_DATACH_MASK)<<RFIU_USRDATA_DATACH_SHFT);
          #if DEBUG_WRPTR_WSHFT
            RX_UsrData = (RX_UsrData & (~(0x1ff<<11))) | ((gRfiuUnitCntl[RFUnit].BufWritePtr & 0x1f)<<11) | ((gRfiuUnitCntl[RFUnit].RX_CtrlPara_next.GrpShift & 0xf)<<16);
          #endif
         
            if(gRfiuUnitCntl[RFUnit].OpState == RFIU_RX_STATE_LISTEN)
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
         #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
            rfiuWaitForInt_Tx(0,&gRfiuParm_Tx[RFUnit]);  //Tx
         #elif RFI_TEST_4TX_2RX_PROTOCOL
            rfiuWaitForInt_Tx(RFUnit & 0x01,&gRfiuParm_Tx[RFUnit]);  //Tx
         #else
            rfiuWaitForInt_Tx(RFUnit,&gRfiuParm_Tx[RFUnit]);  //Tx
         #endif
         
         #if(RF_PAIR_EN)  
            if(MACadrsSetflag[RFUnit] >0)
            {
              gRfiuSyncWordTable[RFUnit] = Temp_RX_MAC_Address[RFUnit];
              gRfiuCustomerCode[RFUnit] = Temp_RX_CostomerCode[RFUnit];
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
              A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif RFI_TEST_4TX_2RX_PROTOCOL
              A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else       
              A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] ); 
           #endif
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
               #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                  A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
               #elif RFI_TEST_4TX_2RX_PROTOCOL
                  A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
               #else       
                  A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] ); 
               #endif
            #endif
              MACadrsSetflag[RFUnit] =0;
              RX_PAIR_ACKflag=1;
              RXLinkOKCnt=0;
              timerCountRead(guiRFTimerID, &t11);
            } 
         #endif


            //===Check Sync===//
            if(gRfiuSyncRevFlag[RFUnit] == 1)
            {
               RX_SYNC_ACKflag=1;
               gRfiuSyncRevFlag[RFUnit] =0;
               RXLinkOKCnt=0;
               timerCountRead(guiRFTimerID, &t11);
            }
       
            #if DEBUG_TX_TIMEOUT
            #endif 
            break;  

      #if(RF_PAIR_EN) 
        case RFIU_PAIRLint_MODE:
                gRfiu_Op_Sta[RFUnit] = RFIU_OP_INIT;
			    sprintf(sysBackOsdString,"Paring CH-%d",(RFUnit+1));
				sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;
                Old_RXMAC = gRfiuSyncWordTable[RFUnit];
                Old_RXCode= gRfiuCustomerCode[RFUnit];
                gRfiuSyncWordTable[RFUnit]=RFI_PAIR_SYNCWORD;
                gRfiuCustomerCode[RFUnit]=RFI_PAIR_CUSTOMER_ID;
                MACadrsSetflag[RFUnit]=0;
                RX_SYC_ErrorCnt=0;
                RX_PAIR_ACKflag=0; 
       #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif RFI_TEST_4TX_2RX_PROTOCOL
                A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else     
                A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );  
           #endif
       #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
           #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
           #elif RFI_TEST_4TX_2RX_PROTOCOL
                A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
           #else     
                A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );  
           #endif
       #endif
				PairCnt=0;
           break; 

       case RFIU_PAIR_STOP_MODE:
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
                    sprintf(sysBackOsdString,"                  ");
    			    sysbackSetEvt(SYS_BACK_DRAW_OSD_STRING, MSG_ASCII_STR);
                    RX_PAIR_ACKflag=0;
                    RX_SYC_ErrorCnt=0;
                    gRfiuSyncWordTable[RFUnit]=Old_RXMAC;
                    gRfiuCustomerCode[RFUnit]=Old_RXCode;
                    gRfiuUnitCntl[RFUnit].RX_Pair_Done=0;
          #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )          
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                    A7130_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                #elif RFI_TEST_4TX_2RX_PROTOCOL
                    A7130_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                #else
                    A7130_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                #endif
          #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                #if (RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1)
                    A7196_ID_Update(0+1 ,gRfiuSyncWordTable[RFUnit] );
                #elif RFI_TEST_4TX_2RX_PROTOCOL
                    A7196_ID_Update((RFUnit & 0x01)+1 ,gRfiuSyncWordTable[RFUnit] );
                #else
                    A7196_ID_Update(RFUnit+1 ,gRfiuSyncWordTable[RFUnit] );
                #endif
          #endif
                    gRfiuUnitCntl[RFUnit].OpMode=RFIU_RX_MODE;
                }
           break;
           
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

#if 1//(RFIC_SEL==RFIC_A7196_6M)
   #define MAX_CH_SCAN  4
#else
   #define MAX_CH_SCAN  8
#endif

#if(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)
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
    {   // 0   1   2  3   4   5  6  7  8  9  10  11  12  13  14  15
        { 13, 13, 13, 13, 13,13, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0},
        { 15, 15, 15, 15, 15,15,15,15,15, 2,  2,  2,  2,  2,  2,  2}
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
       RSSI_CH_Sum[ ScanTab[index][i] ]=999;
   //--------------------------------------------//

   return CHTab[RFUnit & 0x01][index];

}
#elif(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)  //只有單支天線
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

const u8 CHTab[RFI_DAT_CH_MAX]=
        // 0   1   2  3   4   5  6  7  8  9  10  11  12  13  14  15
        { 15, 15, 15, 15, 15,15,15,15,15, 0,  0,  0,  0,  0,  0,  0};

   int i,j,k;
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

   if(gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn)
   {
       for(i=0;i<5;i++)
           RSSI_CH_Sum[ ScanTab[gRfiuUnitCntl[RFUnit].RFpara.WifiLinkCH][i] ]=9999;
   }
   else
   {
       for(i=0;i<5;i++)
           RSSI_CH_Sum[ ScanTab[index][i] ]=9999;
   }
   //--------------------------------------------//
   for(i=0;i<RFI_DAT_CH_MAX;i++)
   {
      sum=0;
      for(j=0;j<5;j++)
      {
         sum += RSSI_CH_Sum[ ScanTab[i][j] ];
      }
      
      if(sum < min)
      {
         min=sum;
         index=i;
         //DEBUG_RFIU_P2("[%d,%d]\n",min,index);
      }
   }
#if 0   
   DEBUG_RFIU_P2("\n%d,%d:",RFUnit,rfiuRX_CamOnOff_Num);

   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_temp[i]);     
   DEBUG_RFIU_P2("\n    ");

   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_CH_Sum[i]);     
   DEBUG_RFIU_P2("\n");
#endif

#if 1  //Jason 建議
   if(gRfiuUnitCntl[RFUnit].RFpara.WifiLinkOn)
   {
       index=CHTab[gRfiuUnitCntl[RFUnit].RFpara.WifiLinkCH];
   }
#endif

   return index;

}
#else //8200,8600 series, two attena
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

   #if 1 //(RFIC_SEL==RFIC_A7196_6M)
    const u8 CHTab[2][MAX_CH_SCAN]=
    {
        { 0, 4, 8, 12},
        { 2, 6,10, 15}
    };
   #else
    const u8 CHTab[2][MAX_CH_SCAN]=
    {
        { 0, 2, 4, 6, 8,10,12,14},
        { 1, 3, 5, 7, 9,11,13,15}
    };
   #endif
   int i,j,k;
   int sum,min,index;
   u32 RSSI_CH_Sum[RFI_DAT_CH_MAX];
   u32 RSSI_temp[RFI_DAT_CH_MAX];
   int max;
   int flag;
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
   for(i=0;i<MAX_CH_SCAN;i++)
   {
      sum=0;
      for(j=0;j<5;j++)
      {
         sum += RSSI_CH_Sum[ ScanTab[CHTab[RFUnit & 0x01][i] ][j] ];
      }
      
      if(sum < min)
      {
        flag=0;
        for(k=( (RFUnit+1)& 0x01);k<4;k+=2)
        {
            if(gRfiuFCC247ChUsed[k][0] != -1)
            {
                if( ( (gRfiuFCC247ChUsed[k][0]+1) == CHTab[RFUnit & 0x01][i]) || ( (gRfiuFCC247ChUsed[k][0]-1) == CHTab[RFUnit & 0x01][i]) )
                {
                   flag=1;
                }
            }
        }
         if(flag==0)
         {
           min=sum;
           index=i;
         }
         //DEBUG_RFIU_P2("[%d,%d]\n",min,index);
      }
   }
#if 0   
   DEBUG_RFIU_P2("\n%d,%d:",RFUnit,rfiuRX_CamOnOff_Num);

   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_temp[i]);     
   DEBUG_RFIU_P2("\n    ");

   for(i=0;i<16;i++)
     DEBUG_RFIU_P2("%3d ",RSSI_CH_Sum[i]);     
   DEBUG_RFIU_P2("\n");
   
#endif

   return CHTab[RFUnit & 0x01][index];

}
#endif


#endif



//-----------------------------Inner test--------------------------//

int marsRfiu_FCC_DirectTXRX()
{
    int testrun,i;
    unsigned char *pp;
    unsigned int err;
    GPIO_CFG c;
    unsigned int TX_Freq;
    unsigned int Vitbi_en,RS_mode,Vitbi_mode;
    unsigned int Old_FCCUnit;
    u8 RSSI2;
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
       gRfiuUnitCntl[i].FCC_TX_Freq=2408;
       gRfiuUnitCntl[i].FCC_RX_Freq=2408;
    }
    OSTimeDly(3);
    
    pp=(unsigned char *)rfiuOperBuf[0];

#if( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
    //---Set Default Power---//
    A7196_WriteReg_B1(0x2d, 0x6b); // 16.0
    A7196_WriteReg_B2(0x2d, 0x6b); // 16.0
#endif

    DEBUG_RFIU_P2("----------------------------marsRfiu_FCC_DirectTXRX---------------------------------\n");
    for(i=0;i<RFIU_TESTCNTMAX_WD*4;i++)
    {
        *pp= *pp + (i*1343 & 0x0ff);
        // *pp= 0x5a;
         pp ++;
    }

    //-----Test run------//
    testrun=0;
    while(1)
    {
        if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_TX_MODE)
        {
            
            if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON)
            {
                //====Select CH====//            
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );            
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );   
            #endif
            
                RfiuReset(FCC_Unit_Sel);
                DEBUG_RFIU_P2("TX-%d:%d,%d,Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq);
                
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
                   if(GpioActFlashSelect & GPIO_RF2_FrIIC_EN)
                       gpioConfig(1,1,&c);
                   else
                       gpioConfig(0,7,&c);
                }
                //----//
                rfiuDataPktConfig_Tx(FCC_Unit_Sel,&(gRfiuParm_Tx[FCC_Unit_Sel]) );  //Tx
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
                   if(GpioActFlashSelect & GPIO_RF2_FrIIC_EN)
                      gpioConfig(1,1,&c);
                   else
                      gpioConfig(0,7,&c);
                }
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )   
                if(FCC_Unit_Sel ==0)
                   A7130_WriteReg_B1(0x15, 0x87); 
                else
                   A7130_WriteReg_B2(0x15, 0x87); 
                
                A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 ); 
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                if(FCC_Unit_Sel ==0)
                   A7196_WriteReg_B1(0x15, 0x07); 
                else
                   A7196_WriteReg_B2(0x15, 0x07);  
              
                A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 ); 
            #endif
                TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                Old_FCCUnit=FCC_Unit_Sel;  
                
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )                            
                A7130_TxMode_Start(FCC_Unit_Sel+1);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_TxMode_Start(FCC_Unit_Sel+1);
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
                        A7130_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );            
                        A7130_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                        Old_FCCUnit=FCC_Unit_Sel;                        
                    }
                
                    if( TX_Freq !=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq )
                    {
                        A7130_TxMode_Stop(FCC_Unit_Sel+1);
                        A7130_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );            
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
                        A7196_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );            
                        A7196_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                        Old_FCCUnit=FCC_Unit_Sel;                        
                    }
                
                    if( TX_Freq !=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq )
                    {
                        A7196_TxMode_Stop(FCC_Unit_Sel+1);
                        A7196_CH_sel(FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq-2400)*2 );            
                        A7196_TxMode_Start(FCC_Unit_Sel+1);
                        TX_Freq=gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq;
                    }
                #endif

                    if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero)
                       c.level = GPIO_LEVEL_LO;
                    else
                	   c.level = GPIO_LEVEL_HI;

                    if(FCC_Unit_Sel==0)
                       gpioConfig(2,13,&c);
                    else if(FCC_Unit_Sel==1)
                    {
                       if(GpioActFlashSelect & GPIO_RF2_FrIIC_EN)
                          gpioConfig(1,1,&c);
                       else
                          gpioConfig(0,7,&c);
                    }
                    DEBUG_RFIU_P2("TX-%d:%d,%d,Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_RealData_ON,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_NoData_Zero,gRfiuUnitCntl[FCC_Unit_Sel].FCC_TX_Freq);

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
            #endif     
            
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_TxMode_Stop(FCC_Unit_Sel+1);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_TxMode_Stop(FCC_Unit_Sel+1);
            #endif
            }
                    
        }
        else if(gRfiuUnitCntl[FCC_Unit_Sel].FCC_TestMode== FCC_DIRECT_RX_MODE)
        {
        #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400)*2 );            
        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel( FCC_Unit_Sel+1,(gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq-2400)*2 );     
        #endif
            RfiuReset(FCC_Unit_Sel);
            DEBUG_RFIU_P2("RX-%d:Freq=%d\n",FCC_Unit_Sel,gRfiuUnitCntl[FCC_Unit_Sel].FCC_RX_Freq);

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

            rfiuDataPktConfig_Rx(FCC_Unit_Sel,&(gRfiuParm_Rx[FCC_Unit_Sel]) );  //Rx            
            rfiuWaitForInt_Rx(FCC_Unit_Sel,&gRfiuParm_Rx[FCC_Unit_Sel],0,&RSSI2);  //Rx
        }

        testrun ++;
    }

    return 1;

}

    #define FCC_DIRECT_TX_CHSEL_LOW      0
    #define FCC_DIRECT_TX_CHSEL_MIDDLE   1
    #define FCC_DIRECT_TX_CHSEL_HIGH     2

    int rfiuFCCTX0Cmd(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;
        //================//
    #if(RFIC_SEL != RFIC_NONE)
        sscanf((char*)cmd,"%d %d %d %d",&RealData_ON,&NoData_Zero,&Freq,&PowerReg);
        DEBUG_RFIU_P2("FCC TX0 Cmd: RealData=%d, NoData_Zero=%d,Freq=%d,PwReg=%d\n",RealData_ON,NoData_Zero,Freq,PowerReg);

        gRfiuUnitCntl[RFI_UNIT_0].FCC_TestMode=FCC_DIRECT_TX_MODE;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_RealData_ON=RealData_ON;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_NoData_Zero=NoData_Zero;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=Freq;
      #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
        A7130_WriteReg_B1(0x2d, PowerReg);
      #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_WriteReg_B1(0x2d, PowerReg);
      #endif
	#endif
    }

    int rfiuFCCTX1Cmd(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;
        //================//
    #if(RFIC_SEL != RFIC_NONE)
        sscanf((char*)cmd,"%d %d %d %d",&RealData_ON,&NoData_Zero,&Freq,&PowerReg);
        DEBUG_RFIU_P2("FCC TX1 Cmd: RealData=%d, NoData_Zero=%d,Freq=%d,PwReg=%d\n",RealData_ON,NoData_Zero,Freq,PowerReg);

        gRfiuUnitCntl[RFI_UNIT_1].FCC_TestMode=FCC_DIRECT_TX_MODE;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_RealData_ON=RealData_ON;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_NoData_Zero=NoData_Zero;
        gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=Freq;
      #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
        A7130_WriteReg_B2(0x2d, PowerReg);
      #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_WriteReg_B2(0x2d, PowerReg);
      #endif
	#endif

    }

    int rfiuFCCTX0Cmd2(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;

        int sel;
        u8 reg;
        //================//
    #if(RFIC_SEL != RFIC_NONE)
        if(!strncmp((char*)cmd,"P ", strlen("P ")))
        {
            sscanf((char*)cmd, "P %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                case 0: 
                    A7130_WriteReg_B1(0x2d, 0x34); // -2.54 dBm
                    break;

                case 1: 
                    A7130_WriteReg_B1(0x2d, 0x35);  //+0.2 dBm
                    break;

                case 2: 
                    A7130_WriteReg_B1(0x2d, 0x36); // +2.63
                    break;
                    
                case 3: 
                    A7130_WriteReg_B1(0x2d, 0x37); // +4.5 dBm
                    break;
            #elif((RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                case 0: 
                    A7196_WriteReg_B1(0x2d, 0x67);  // 15.3
                    break;

                case 1: 
                    A7196_WriteReg_B1(0x2d, 0x6b); // 16.0
                    break;

                case 2: 
                    A7196_WriteReg_B1(0x2d, 0x6f); // 16.5
                    break;
                    
                case 3: 
                    A7196_WriteReg_B1(0x2d, 0x7f); // 17.5
                    break;
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %x", &reg);
        #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )    
            A7130_WriteReg_B1(0x2d, reg);
        #elif((RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_WriteReg_B1(0x2d, reg);
        #endif
        }
        else if(!strncmp((char*)cmd,"F ", strlen("F ")))
        {
            sscanf((char*)cmd, "F %d", &sel);
            switch(sel)
            {
                case 0: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2408;
                    break;

                case 1: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2440;
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_0].FCC_TX_Freq=2468;
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
    #endif
    }


    int rfiuFCCTX1Cmd2(u8 *cmd)
    {
        int RealData_ON;
        int NoData_Zero;
        int Freq;
        int PowerReg;

        int sel;
        u8 reg;
        //================//
    #if(RFIC_SEL != RFIC_NONE)
        if(!strncmp((char*)cmd,"P ", strlen("P ")))
        {
            sscanf((char*)cmd, "P %d", &sel);
            switch(sel)
            {
            #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )  
                case 0: 
                    A7130_WriteReg_B2(0x2d, 0x34); // -2.54
                    break;

                case 1: 
                    A7130_WriteReg_B2(0x2d, 0x35); //+ 0.2
                    break;

                case 2: 
                    A7130_WriteReg_B2(0x2d, 0x36);  //+2.63
                    break;
                    
                case 3: 
                    A7130_WriteReg_B2(0x2d, 0x37);  //+4.5
                    break;
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                case 0: 
                    A7196_WriteReg_B2(0x2d, 0x34);
                    break;

                case 1: 
                    A7196_WriteReg_B2(0x2d, 0x35);
                    break;

                case 2: 
                    A7196_WriteReg_B2(0x2d, 0x36);
                    break;
                    
                case 3: 
                    A7196_WriteReg_B2(0x2d, 0x37);
                    break;
            #endif
            }
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            sscanf((char*)cmd, "W %x", &reg);
        #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )        
            A7130_WriteReg_B2(0x2d, reg);
        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_WriteReg_B2(0x2d, reg);
        #endif
        }
        else if(!strncmp((char*)cmd,"F ", strlen("F ")))
        {
            sscanf((char*)cmd, "F %d", &sel);
            switch(sel)
            {
                case 0: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2408;
                    break;

                case 1: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2440;
                    break;

                case 2: 
                    gRfiuUnitCntl[RFI_UNIT_1].FCC_TX_Freq=2468;
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
    #endif
    }
    
    int rfiuFCCRX0Cmd(u8 *cmd)
    {
        int Freq;
        //================//
        sscanf((char*)cmd,"%d",&Freq);
        DEBUG_RFIU_P2("FCC RX Cmd:Freq=%d\n",Freq);

        gRfiuUnitCntl[RFI_UNIT_0].FCC_TestMode=FCC_DIRECT_RX_MODE;
        gRfiuUnitCntl[RFI_UNIT_0].FCC_RX_Freq=Freq;
    }

    int rfiuFCCRX1Cmd(u8 *cmd)
    {
        int Freq;
        //================//
        sscanf((char*)cmd,"%d",&Freq);
        DEBUG_RFIU_P2("FCC RX Cmd:Freq=%d\n",Freq);

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
#if RFIU_TEST
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
    



#define CHECK_PKTMAP_ON        1
#define CHECK_PKTMAP_OFF       0

#define CHECK_PKT_BURSTNUM_ON  1
#define CHECK_PKT_BURSTNUM_OFF 0

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
                      DEBUG_RFIU_P("Map FAIL\n");
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
                  DEBUG_RFIU_P("Pkt_Grp_offset FAIL\n");
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

        SYS_CTL0 = SYS_CTL0 & (~0x40000000); //disable Mclk

    //-----//
    #if RFI_MEASURE_RX1RX2_SENSITIVITY
        DEBUG_RFIU_P("------>marsRfiu_measure_2Rx sensitivity-----\n");
        status = marsRfiu_Measure_RX1RX2_Sensitivity(0xfffffff, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
    #endif

    #if RFI_FCC_DIRECT_TRX
        status = marsRfiu_FCC_DirectTXRX();
    #endif

    //----------------------------//		 
    #if RFI_TEST_PKTBURST
        DEBUG_RFIU_P("------>marsRfiu_Test_PktBurst-----\n");
        status = marsRfiu_Test_PktBurst(256, RFI_VITBI_DISA, RFI_RS_T2, RFI_VITBI_CR1_2);
        if(status==0)  return 0;            
    #endif
    //---------------------------//
    #if RFI_TEST_PKTMAP
        DEBUG_RFIU_P("------>marsRfiu_Test_PktMap-----\n");
        status=marsRfiu_Test_PktMap(2000);
        if(status==0) return 0;
    #endif
    //---------------------------//
    #if RFI_TEST_PERFORMANCE
      #if 1
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
        //----//
      #if 0
        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T2,RFI_VITBI_CR4_5\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T2, RFI_VITBI_CR4_5);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T4,RFI_VITBI_CR4_5\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T4, RFI_VITBI_CR4_5);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performancet:RFI_VITBI_EN,RFI_RS_T8,RFI_VITBI_CR4_5\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T8, RFI_VITBI_CR4_5);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T12,RFI_VITBI_CR4_5\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T12, RFI_VITBI_CR4_5);
        if(status==0) return 0;
        
        //----//
        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T2,RFI_VITBI_CR3_4\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T2, RFI_VITBI_CR3_4);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T8,RFI_VITBI_CR3_4\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T8, RFI_VITBI_CR3_4);
        if(status==0) return 0;

        //----//
        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T2,RFI_VITBI_CR2_3\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T2, RFI_VITBI_CR2_3);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T4,RFI_VITBI_CR2_3\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T4, RFI_VITBI_CR2_3);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T8,RFI_VITBI_CR2_3\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T8, RFI_VITBI_CR2_3);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T12,RFI_VITBI_CR2_3\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T12, RFI_VITBI_CR2_3);
        if(status==0) return 0;
      #endif


        //----//
      #if 0
        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T2,RFI_VITBI_CR1_2\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T2, RFI_VITBI_CR1_2);
        if(status==0) return 0;
        
        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T4,RFI_VITBI_CR1_2\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T4, RFI_VITBI_CR1_2);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T8,RFI_VITBI_CR1_2\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T8, RFI_VITBI_CR1_2);
        if(status==0) return 0;

        DEBUG_RFIU_P("-->marsRfiu_Test_Performance:RFI_VITBI_EN,RFI_RS_T12,RFI_VITBI_CR1_2\n");
        status = marsRfiu_Test_Performance(RFI_TEST_PERFO_NUM, RFI_VITBI_EN, RFI_RS_T12, RFI_VITBI_CR1_2);
        if(status==0) return 0;
      #endif
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
            if(count > 128)
                count=128;
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =count;  //zero start
            if(count ==0)
                continue;
            DEBUG_RFIU("---Packetnum=%d,Testrun=%d---\n",count,testrun);

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
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]) );  //Tx
    
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
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
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
            DEBUG_RFIU("---Packetnum=%d,Testrun=%d---\n",count,testrun);

            
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
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]) );  //Tx

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
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
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
            
       
            gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum           =testrun % 129;  //zero start
            
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
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]) );  //Tx

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
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
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
            
            gRfiuParm_Tx[RFI_UNIT_1].TxRxPktNum           =testrun % 129;  //zero start
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
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]));  //Tx

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
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
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
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
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
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
               #endif
                  DEBUG_RFIU_P2("AMIC Reg-0x%x=0x%x\n",AmicReg_Addr,AmicReg_Data);
               }
               AmicReg_RWen2=0;
            }   
        #endif

#if 1      
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
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]) );  //Tx

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
               err=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
               if(err==0)
                 return 0;
            }

            Rx1Count += gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum;
        #if RFI_FPGA_PERFORMANCE_MEASURE
          #if FIX_A1018A_BUG_RF
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_2));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_2));
          #else
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_1));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_1));
          #endif
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
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_1,&(gRfiuParm_Tx[RFI_UNIT_1]) );  //Tx

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
               err=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
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
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
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
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
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
            //rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
            rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]) );  //Tx

            rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
            //rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
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
               #endif
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B1(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B1(AmicReg_Addr);
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
               #endif
               
               }
               else
               {
               #if( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                  AmicReg_Data=A7130_ReadReg_B2(AmicReg_Addr);
               #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                  AmicReg_Data=A7196_ReadReg_B2(AmicReg_Addr);
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
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
            //rfiuDataPktConfig_Tx(RFI_UNIT_0,&(gRfiuParm_Tx[RFI_UNIT_0]) );  //Tx

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
    {
        int testrun,i,j,state;
        unsigned int *pp,*qq,*rr;
        int TxAckRetryCnt;
        int ACK_CH_sel=0;
        int TX_UsrData,TX_UsrACK,TX_UsrData_next;
        int RX_UsrData,RX_UsrACK;
        int GrpCnt,TryCnt;
        int RX_RecvPktCnt;
        int DatPktRecvFlag;
        int DAT_CH_sel,prev_DAT_CH_sel;
        DEF_RFIU_DAT_CH_STATISTICS TX_CH_Stat[RFI_DAT_CH_MAX+1]; //Lucian: used in TX-end.
        DEF_RFIU_FEC_TYPE_STATISTICS TX_FEC_Stat[RFI_FEC_TYPE_MAX];
        unsigned int  cpu_sr = 0; 
       
    #if RFI_FPGA_PERFORMANCE_MEASURE
        unsigned int SyncErrCnt=0,RsErrCnt=0;
        unsigned int Preamble_err,Trailer_err,SyncWord_err,RS_err,CRC_err,err1,err2,ID_err;
    #endif
        u8 RSSI2;
        //--------------------------------//
        prev_DAT_CH_sel=0;
        DAT_CH_sel = 0;
        
        for(i=0;i<(RFI_DAT_CH_MAX+1);i++)
        {
           memset(&TX_CH_Stat[i],0,sizeof(DEF_RFIU_DAT_CH_STATISTICS));
        }

        for(i=0;i<RFI_FEC_TYPE_MAX;i++)
        {
           memset(&TX_FEC_Stat[i],0,sizeof(DEF_RFIU_FEC_TYPE_STATISTICS));
        }
        
        TX_UsrData= rfiuEncUsrData(&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara);
        TX_UsrACK=TX_UsrData;
        TX_UsrData_next=TX_UsrData;

        memcpy(&gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara,&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara,sizeof(DEF_RFIU_USRDATA));     
        memcpy(&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next,&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara,sizeof(DEF_RFIU_USRDATA));    
        RX_UsrData= rfiuEncUsrData(&gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara);
        RX_UsrACK=RX_UsrData;

    #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
        MV400_CH_sel(1,0);
        MV400_CH_sel(2,0);
    #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
        A7130_CH_sel(1,100);
        A7130_CH_sel(2,100);
    #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
        A7196_CH_sel(1,100);
        A7196_CH_sel(2,100);
    #endif
        
        //---Setup test environment---//
   
    #if RFI_TEST_WRAP_OnCOMMU 
        //==TX==//
        memset(&gRfiuParm_Tx[RFI_UNIT_0],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuParm_Rx[RFI_UNIT_0],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuUnitCntl[RFI_UNIT_0],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));

        gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_INIT;
        gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr = rfiuOperBuf[0];
        gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr = rfiuOperBuf[0];
       
        gRfiuUnitCntl[RFI_UNIT_0].BufReadPtr=0;
        gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_en=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_sel=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.RS_sel=1;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.DAT_CH=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.SeqenceNum=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.GrpShift=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.GrpDivs=0;
        OSTaskCreate(RFIU_TX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_0, RFIU_WRAP_TASK_STACK_UNIT0, RFIU_WRAP_TASK_PRIORITY_UNIT0);
        
        //==RX==//
        memset(&gRfiuParm_Tx[RFI_UNIT_1],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuParm_Rx[RFI_UNIT_1],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuUnitCntl[RFI_UNIT_1],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));

        gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_LISTEN;
        gRfiuParm_Tx[RFI_UNIT_1].TxRxOpBaseAddr = rfiuOperBuf[1];
        gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr = rfiuOperBuf[1];

        gRfiuUnitCntl[RFI_UNIT_1].BufReadPtr=0;
        gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.Vitbi_en=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.Vitbi_sel=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.RS_sel=1;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.DAT_CH=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.SeqenceNum=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.GrpShift=0;
        gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.GrpDivs=0;
        OSTaskCreate(RFIU_RX_WRAP_TASK_UNITX, RFIU_WRAP_TASK_PARAMETER_UNIT_1, RFIU_WRAP_TASK_STACK_UNIT1, RFIU_WRAP_TASK_PRIORITY_UNIT1);
    #else
        pp=(unsigned int *)rfiuOperBuf[0];
        qq=(unsigned int *)rfiuOperBuf[1];
        for(i=0;i<RFI_GRP_INPKTUNIT*RFI_PAYLOAD_SIZE*(RFI_BUF_SIZE_GRPUNIT)/4;i++)
        {
            *pp= *pp + i*1343;
            //*pp= 0x5a5aa5a5;
            *qq=0;
            pp ++;
            qq ++;
        }

        memset(&gRfiuParm_Tx[RFI_UNIT_0],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuParm_Rx[RFI_UNIT_0],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuUnitCntl[RFI_UNIT_0],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));

        memset(&gRfiuParm_Tx[RFI_UNIT_1],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuParm_Rx[RFI_UNIT_1],0,sizeof(DEF_REGRFIU_CFG));
        memset(&gRfiuUnitCntl[RFI_UNIT_1],0,sizeof(DEF_RFIU_UNIT_CNTL)-sizeof(DEF_RFIU_UNIT_PARA));
        
        gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_INIT;
        gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_LISTEN;

        gRfiuParm_Tx[RFI_UNIT_0].TxRxOpBaseAddr = rfiuOperBuf[0];
        gRfiuParm_Rx[RFI_UNIT_0].TxRxOpBaseAddr = rfiuOperBuf[0];

        gRfiuParm_Tx[RFI_UNIT_1].TxRxOpBaseAddr = rfiuOperBuf[1];
        gRfiuParm_Rx[RFI_UNIT_1].TxRxOpBaseAddr = rfiuOperBuf[1];
        
        gRfiuUnitCntl[RFI_UNIT_0].BufReadPtr=0;
        gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr=64;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_en=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_sel=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.RS_sel=1;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.DAT_CH=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.SeqenceNum=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.GrpShift=0;
        gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.GrpDivs=0;
        
        for(i=0;i<gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr;i++)
        {
           gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[i].PktCount=64;
           gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[i].RetryCount=0;
           gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[i].PktMap0 =0xffffffff;
           gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[i].PktMap1 =0xffffffff;
        }
    #endif    
        //--------------------------------------------------------------//
        GrpCnt=0;
        TryCnt=0;
        for(testrun=0;testrun<TestRunCount;testrun ++)
        {
        #if 1
            //==================(Unit-0 -->Unit-1)=================//
            //Config Unit0:Tx, Unit1:Rx
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            #endif

            if((TX_UsrData_next & 0x60 ) == (TX_UsrACK & 0x60))
            {
                DatPktRecvFlag=1;
            }
            else
            {
                DatPktRecvFlag=0;
            }
                    
            rfiuDecUsrData(TX_UsrACK,&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara);
            TX_UsrData=TX_UsrACK;                
       
            rfiuTxUpdatePktMap(RFI_UNIT_0,
                               DatPktRecvFlag,
                               &gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara,
                               &gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next,
                               &RX_RecvPktCnt,
                               prev_DAT_CH_sel,
                               TX_CH_Stat,
                               TX_FEC_Stat
                              );
            
        
            GrpCnt += gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next.GrpShift;
            TryCnt ++;

            //------Statistic correct rate of TX DATA channel------//
            DAT_CH_sel=gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next.DAT_CH;
                        
            TX_CH_Stat[DAT_CH_sel].SentPktNum += gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum;
            TX_CH_Stat[DAT_CH_sel].BurstNum +=1;
            
            prev_DAT_CH_sel= DAT_CH_sel;

            //-------Compare data-------//
        #if RFI_TEST_WRAP_OnCOMMU
            

        #else
            //for(i=0;i<gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next.GrpShift;i++)
            for(i=0;i<2;i++)
            {
               gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr & RFI_BUF_SIZE_MASK].PktCount=64;
               gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr & RFI_BUF_SIZE_MASK].RetryCount=0;
               gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr & RFI_BUF_SIZE_MASK].PktMap0 =0xffffffff;
               gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr & RFI_BUF_SIZE_MASK].PktMap1 =0xffffffff;

               rr=(unsigned int *)(rfiuOperBuf[RFI_UNIT_0] + ( ((gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr) & RFI_BUF_SIZE_MASK)<<13));
               for(j=0;j<RFI_GRP_INPKTUNIT*RFI_PAYLOAD_SIZE/4;j++)
               {
                  *rr += 0x01010101;
               }

               gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr=(gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr + 1) & RFI_BUF_SIZE_MASK;
            }            
        #endif
            //---RX---//
            rfiuDecUsrData(RX_UsrData,&gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara);
        #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(2,gRfiuDAT_CH_Table[gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.DAT_CH]);
        #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(2,gRfiuDAT_CH_Table[testrun & 0xf]);
        #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(2,gRfiuDAT_CH_Table[testrun & 0xf]);
        #endif
            rfiuRxListenDataState( RFI_UNIT_1,
                                   gRfiuVitbiOnTable[gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.Vitbi_en], 
                                   gRfiuRSCodeTable[gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.RS_sel], 
                                   gRfiuVitbiCodeTable[gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara.Vitbi_sel],
                                   SyncWord,
                                   CustomerID,
                                   128
                                  );            

            //---TX---//   
            TX_UsrData_next= rfiuEncUsrData(&gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next);
         #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
            MV400_CH_sel(1,gRfiuDAT_CH_Table[gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.DAT_CH]);
         #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
            A7130_CH_sel(1,gRfiuDAT_CH_Table[testrun & 0xf]);
         #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
            A7196_CH_sel(1,gRfiuDAT_CH_Table[testrun & 0xf]);
         #endif
            rfiuTxSendDataState( RFI_UNIT_0,
                                 gRfiuVitbiOnTable[gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_en], 
                                 gRfiuRSCodeTable[gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.RS_sel], 
                                 gRfiuVitbiCodeTable[gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara.Vitbi_sel],
                                 SyncWord,
                                 CustomerID,
                                 TX_UsrData_next
                               );
            DEBUG_RFIU_P("\n====Data Transmit(Unit-0 -->Unit-1),TxCH=%d,RxCH=%d======\n",gRfiuDAT_CH_Table[testrun & 0xf],gRfiuDAT_CH_Table[testrun & 0xf]);            
            state=rfiuWaitForInt_Rx(RFI_UNIT_1,&gRfiuParm_Rx[RFI_UNIT_1],0,&RSSI2);  //Rx
         #if(RFI_TEST_WRAP_OnCOMMU==0) 
            #if DEBUG_RX_TIMEOUT
            #else
             if(state==0)
                 return 0;
            #endif
         #endif    
            state=rfiuWaitForInt_Tx(RFI_UNIT_0,&gRfiuParm_Tx[RFI_UNIT_0]);  //Tx
         #if(RFI_TEST_WRAP_OnCOMMU==0) 
            #if DEBUG_TX_TIMEOUT
            #else
             if(state==0)
                 return 0;
            #endif
         #endif      

            //----//
            if(gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum==0)
            {
               DEBUG_RFIU_P("--->No Data Packet received!!\n");
               gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_WAITACK;
               gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_REPLY_ACK;
               RX_UsrData=RX_UsrData;
               rfiuDecUsrData(RX_UsrData,&gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next);
               gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift=0;
            }
            else
            {
            #if 1
               if(gRfiuParm_Rx[RFI_UNIT_1].CID_ErrCnt != 0)
               {
                  DEBUG_RFIU_P2("-->DATA CID error: %d\n",gRfiuParm_Rx[RFI_UNIT_1].CID_ErrCnt);
               }
            #endif

            #if RFI_TEST_WRAP_OnCOMMU

            #else
               state=rfiuCheckTestResult(RFI_UNIT_0,RFI_UNIT_1,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
               if(state==0)
                 return 0;
            #endif
               gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_WAITACK;
               gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_REPLY_ACK;
               RX_UsrData= (gRfiuParm_Rx[RFI_UNIT_1].UserData_L) | (gRfiuParm_Rx[RFI_UNIT_1].UserData_H<<16);
               rfiuDecUsrData(RX_UsrData,&gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next);
            }
            
        #if RFI_TEST_WRAP_OnCOMMU
            for(i=0;i<gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift;i++)
            {
               pp=(unsigned int *)(rfiuOperBuf[0] + ( ((gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr+i) & RFI_BUF_SIZE_MASK)<<13));
               qq=(unsigned int *)(rfiuOperBuf[1] + ( ((gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr+i) & RFI_BUF_SIZE_MASK)<<13));
               for(j=0;j<64;j++)
               {
                   if(*pp != *qq)
                   {
                     DEBUG_RFIU_P2("Warning! TX/RX data mismatch: 0x%x,0x%x,i=%d,j=%d\n",*pp,*qq,i,j);
                     DEBUG_RFIU_P2("TX_Wp=%d,TX_Rp=%d,RX_Wp=%d,GrpShift=%d\n",
                                   gRfiuUnitCntl[0].BufWritePtr,
                                   gRfiuUnitCntl[0].BufReadPtr,
                                   gRfiuUnitCntl[1].BufWritePtr,
                                   gRfiuUnitCntl[1].RX_CtrlPara_next.GrpShift);
                     DEBUG_RFIU_P2("PktCount=%d,Map0=0x%x,Map1=0x%x\n",
                                   gRfiuUnitCntl[0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktCount,
                                   gRfiuUnitCntl[0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktMap0,
                                   gRfiuUnitCntl[0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktMap1
                                  );
                   }
                   pp += 32;
                   qq += 32;
               }
            }
        #else
            
            for(i=0;i<gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift;i++)
            {
               pp=(unsigned int *)(rfiuOperBuf[0] + ( ((gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr+i) & RFI_BUF_SIZE_MASK)<<13));
               qq=(unsigned int *)(rfiuOperBuf[1] + ( ((gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr+i) & RFI_BUF_SIZE_MASK)<<13));

               for(j=0;j<RFI_GRP_INPKTUNIT*RFI_PAYLOAD_SIZE/4;j++)
               {
                   if(*pp != *qq)
                   {
                     DEBUG_RFIU_P2("Warning! TX/RX data mismatch: 0x%x,0x%x,i=%d\n",*pp,*qq,i);

                     DEBUG_RFIU_P2("===>TxSFT=%d,RxSFT=%d,TX_WP=%d,TX_RP=%d,RX_WP=%d\n",
                                     gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next.GrpShift,
                                     gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift,
                                     gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr,
                                     gRfiuUnitCntl[RFI_UNIT_0].BufReadPtr,
                                     gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr
                                  );
                     DEBUG_RFIU_P2("PktCount=%d,Map0=0x%x,Map1=0x%x\n",
                                   gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktCount,
                                   gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktMap0,
                                   gRfiuUnitCntl[RFI_UNIT_0].TxPktMap[(gRfiuUnitCntl[1].BufReadPtr+i) & RFI_BUF_SIZE_MASK].PktMap1
                                  );
                     return 0;
                   }

                   pp ++;
                   qq ++;
                }
            }
        #endif    
        
            OS_ENTER_CRITICAL();
            gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr  = (gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr + gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift) & RFI_BUF_SIZE_MASK ;
            gRfiuUnitCntl[RFI_UNIT_1].WritePtr_Divs= gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpDivs;
            OS_EXIT_CRITICAL();
        
            DEBUG_RFIU_P("===>TxSFT=%d,RxSFT=%d,TX_WP=%d,TX_RP=%d,RX_WP=%d\n",
                                 gRfiuUnitCntl[RFI_UNIT_0].TX_CtrlPara_next.GrpShift,
                                 gRfiuUnitCntl[RFI_UNIT_1].RX_CtrlPara_next.GrpShift,
                                 gRfiuUnitCntl[RFI_UNIT_0].BufWritePtr,
                                 gRfiuUnitCntl[RFI_UNIT_0].BufReadPtr,
                                 gRfiuUnitCntl[RFI_UNIT_1].BufWritePtr
                         );
        #if RFI_FPGA_PERFORMANCE_MEASURE
          #if FIX_A1018A_BUG_RF
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_2));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_2));
          #else
            err1=*((volatile unsigned *)(REG_RFIU_ErrReport_1+0x1000*RFI_UNIT_1));
            err2=*((volatile unsigned *)(REG_RFIU_ErrReport_2+0x1000*RFI_UNIT_1));
          #endif
            CRC_err=(err1>>16) & 0x0ff;
            RS_err=(err1>>24) & 0x0ff;
            SyncWord_err=(err1>>8) & 0x0ff;
            Trailer_err=(err1) & 0x0ff;
            ID_err=err2 & 0xff;
            
            Preamble_err=(int)(gRfiuParm_Tx[RFI_UNIT_0].TxRxPktNum - gRfiuParm_Rx[RFI_UNIT_1].TxRxPktNum - CRC_err - RS_err);
            SyncErrCnt +=Preamble_err;
            RsErrCnt += (CRC_err + RS_err);
            
            DEBUG_RFIU_P("--->Err Cond. SyncErr=%d,RS=%d,CRC=%d,ID=%d,StuffCnt=%d\n",Preamble_err,RS_err,CRC_err,ID_err,gRfiuParm_Tx[RFI_UNIT_0].BitStuffCnt);
        #endif
     #endif       
     #if 1      
            //==================(Unit-1 -->Unit-0)=================//
            DEBUG_RFIU("---Packetnum=%d---\n",testrun);
                   
            gRfiuUnitCntl[RFI_UNIT_0].OpState= RFIU_TX_STATE_WAITACK;
            
            //---config RFU parameter---//
            RX_UsrACK=RX_UsrData;
            TxAckRetryCnt=0;
            while(gRfiuUnitCntl[RFI_UNIT_0].OpState== RFIU_TX_STATE_WAITACK)
            {  
                DEBUG_RFIU_P("==========ACK command(Unit-1 -->Unit-0),CH=%d============\n",ACK_CH_sel);
                //Chanel selection
                if(TxAckRetryCnt>5)
                {
                   ACK_CH_sel =(ACK_CH_sel+1) % RFI_ACK_CH_MAX;
                   DEBUG_RFIU_P("============> ACK Channel switch to %d\n",ACK_CH_sel);
                   
                   TxAckRetryCnt = 0;
                }
            #if ( (RFIC_SEL == RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
                MV400_CH_sel(1,gRfiuACK_CH_Table[0][ACK_CH_sel]);
                MV400_CH_sel(2,gRfiuACK_CH_Table[0][ACK_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7130_4M) || (RFIC_SEL == RFIC_A7130_2M) || (RFIC_SEL == RFIC_A7130_3M) )
                A7130_CH_sel(1,gRfiuACK_CH_Table[0][ACK_CH_sel]);
                A7130_CH_sel(2,gRfiuACK_CH_Table[0][ACK_CH_sel]);
            #elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
                A7196_CH_sel(1,gRfiuACK_CH_Table[0][ACK_CH_sel]);
                A7196_CH_sel(2,gRfiuACK_CH_Table[0][ACK_CH_sel]);
            #endif
                rfiuTxWaitACKState( RFI_UNIT_0,
                                    RFI_VITBI_DISA, 
                                    RFI_RS_T12, 
                                    RFI_VITBI_CR4_5,
                                    SyncWord,
                                    CustomerID,
                                    256
                                  );               

                rfiuRxReplyACKState( RFI_UNIT_1,
                                     RFI_VITBI_DISA, 
                                     RFI_RS_T12, 
                                     RFI_VITBI_CR4_5,
                                      SyncWord,
                                      CustomerID,
                                      RX_UsrACK,
                                      0,
                                      0,
                                      0,
                                      0
                                    );
                state=rfiuWaitForInt_Rx(RFI_UNIT_0,&gRfiuParm_Rx[RFI_UNIT_0],0,&RSSI2);  //Rx
             #if(RFI_TEST_WRAP_OnCOMMU==0)  
                #if DEBUG_RX_TIMEOUT
                #else
                if(state==0)
                   return 0;
                #endif
             #endif      
                state=rfiuWaitForInt_Tx(RFI_UNIT_1,&gRfiuParm_Tx[RFI_UNIT_1]);  //Tx
             #if(RFI_TEST_WRAP_OnCOMMU==0) 
                #if DEBUG_TX_TIMEOUT
                #else
                if(state==0)
                  return 0;
                #endif
             #endif   
         
                //----//
                //if(gRfiuParm_Rx[RFI_UNIT_0].TxRxPktNum==0)
                if( rfiuTXCheckACKCome(&gRfiuParm_Rx[RFI_UNIT_0]) == 0 )
                {
                   DEBUG_RFIU_P("--->No ACK Packet received. #%d!! \n",TxAckRetryCnt);
                   gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_WAITACK;
                   gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_REPLY_ACK;
                   TX_UsrACK =TX_UsrACK;
                   TxAckRetryCnt ++;
                }
                else
                {
                #if 1
                   if(gRfiuParm_Rx[RFI_UNIT_0].CID_ErrCnt != 0)
                   {
                      DEBUG_RFIU_P2("-->ACK CID error: %d\n",gRfiuParm_Rx[RFI_UNIT_0].CID_ErrCnt);
                   }
                #endif

                #if RFI_TEST_WRAP_OnCOMMU

                #else
                   state=rfiuCheckTestResult(RFI_UNIT_1,RFI_UNIT_0,CHECK_PKTMAP_OFF,CHECK_PKT_BURSTNUM_OFF);
                   if(state==0)
                     return 0;
                #endif
                   gRfiuUnitCntl[RFI_UNIT_0].OpState=RFIU_TX_STATE_READY;
                   gRfiuUnitCntl[RFI_UNIT_1].OpState=RFIU_RX_STATE_LISTEN;
                   TX_UsrACK= (gRfiuParm_Rx[RFI_UNIT_0].UserData_L) | (gRfiuParm_Rx[RFI_UNIT_0].UserData_H<<16);
                }
                                
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
                
                DEBUG_RFIU_P("--->Err Cond. SyncErr=%d,RS=%d,CRC=%d,ID=%d,StuffCnt=%d\n",Preamble_err,RS_err,CRC_err,ID_err,gRfiuParm_Tx[RFI_UNIT_1].BitStuffCnt);
             #endif
            }

        #endif
            
        }

        return 1;
    }

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
            rfiuDataPktConfig_Rx(RFI_UNIT_0,&(gRfiuParm_Rx[RFI_UNIT_0]) );  //Rx
            rfiuDataPktConfig_Rx(RFI_UNIT_1,&(gRfiuParm_Rx[RFI_UNIT_1]) );  //Rx
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


