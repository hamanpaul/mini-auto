
/*

Copyright (c) 2008  MARS Technologies, Inc.

Module Name:

    rfiu.h

Abstract:

    The structures and constants of RF Interface.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/10/21  Lucian Yuan   Create              

*/

#ifndef __RFIU_H__
#define __RFIU_H__

//---------- RFIU Test-------------//
/*  
        測試Tx --> Rx
            Rx <-- Tx

*/
//---For Debug---//
#define DEBUG_RFIU_INTR_USE_LED7_9     0
#define DEBUG_TX_WARNING_MSG           0
#define DEBUG_RX_TIMEOUT               0
#define DEBUG_TX_TIMEOUT               0
#define DEBUG_WRPTR_WSHFT              0
#define DEBUG_CID_ERR                  0

#define DEBUG_TRX_CMD                  0

//---For Performance---//
#define RFI_LOW_DELAY_ON               1

#if(RFIC_SEL  ==  RFIC_A7196_6M)
#define RFI_MAX_BURST_NUM              192
#else
#define RFI_MAX_BURST_NUM              128
#endif

#define RFI_GOOD_CH_JDG_LV             204 // X/256

#if(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2 || RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 )
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
#elif RFI_TEST_TXRX_COMMU
    #if RFI_TEST_WRAP_OnCOMMU
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #else
    #define DEBUG_RFIU_P(fmt...)    printf(fmt)
    #endif
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
#elif RFI_SELF_TEST_TXRX_PROTOCOL
    #if RFI_TEST_WRAP_OnPROTOCOL
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #else
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #endif
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
    
#elif RFI_TEST_RXRX_PROTOCOL_B1B2    
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)

#elif RFI_TEST_2x_RX_PROTOCOL_B1    
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)

#elif RFI_TEST_4x_RX_PROTOCOL_B1    
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
    
#elif RFI_TEST_4TX_2RX_PROTOCOL    
    #define DEBUG_RFIU_P(fmt...)    //printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
#else
    #define DEBUG_RFIU_P(fmt...)    printf(fmt)
    #define DEBUG_RFIU_P2(fmt...)   printf(fmt)
#endif
 //---Parameter---//
#if (RFIC_SEL == RFIC_MV400_4M)
  #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    90
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    150
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (180)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (180)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif

  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      4

  #define RFI_DUMMY_PREAMBLE_NUM   2 
  
#elif(RFIC_SEL==RFIC_MV400_2M)
  #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_16
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    90
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    150
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    90*2
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    90*2	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif

  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      4

  #define RFI_DUMMY_PREAMBLE_NUM   2 
#elif(RFIC_SEL == RFIC_A7130_2M)
  #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_16
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    50 //  14
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    75
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (100*2)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (100*2)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif
  
  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      4

  #define RFI_DUMMY_PREAMBLE_NUM   2 

#elif(RFIC_SEL == RFIC_A7130_3M)
  #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_16
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    50 //  14
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    75
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (100*2)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (100*2)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif
  
  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      3

  #define RFI_DUMMY_PREAMBLE_NUM   1 
 
#elif(RFIC_SEL == RFIC_A7130_4M)
  #if(RFI_CLK_FREQ == 36000000)
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_9

  #elif(RFI_CLK_FREQ == 32000000)	
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
	
  #else
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
  #endif


  
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    50 //  14
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    75
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (150)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (150)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif
  
  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      3
  
  #define RFI_DUMMY_PREAMBLE_NUM   1 
  
#elif( (RFIC_SEL == RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
  #if(RFI_CLK_FREQ == 36000000)
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_9

  #elif(RFI_CLK_FREQ == 32000000)	
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
	
  #else
    #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
  #endif


  
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    50 //  14
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    75
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (150)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (150)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif
  
  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      3
  
  #define RFI_DUMMY_PREAMBLE_NUM   1 
  
#else
  #define RFI_CLK_DIV_SEL  RFI_TXCLK_DIV_8
  //--Config Dummy data timming---//
  #if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
    #define RFI_TX_DUMMY_DATA_NUM    50 //  14
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
    #define RFI_TX_DUMMY_DATA_NUM    75
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
    #define RFI_TX_DUMMY_DATA_NUM    (100)
  #elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
    #define RFI_TX_DUMMY_DATA_NUM    (100)	
  #else
    #define RFI_TX_DUMMY_DATA_NUM    90
  #endif
  
  //----Config Preamble/dummy-preamble number---//
  #define RFI_TX_PREAMBLE_NUM      8
  #define RFI_RX_PREAMBLE_NUM      3
  
  #define RFI_DUMMY_PREAMBLE_NUM   1 
#endif

#define RFI_SYNCID_PREFIX          0x5aa5a55a

//---Config RX timming--//
#define RFI_TXCLKCONFIG       ( ((RFI_CLK_DIV_SEL-1)<<8) | (RFI_CLK_DIV_SEL*2))

#if(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_16)
#define RFI_RXCLKADJUST       ((0<<16) | (0x2<<3) | (0x0<<0))
#define RFI_DCLKCONF          ( (RFI_RX_PREAMBLE_NUM<<28) | (0xf<<24) | ((RFI_CLK_DIV_SEL*2)<<16) | (RFI_CLK_DIV_SEL<<8) | (RFI_CLK_DIV_SEL<<0))
#define RFI_RXTIMINGFINETUNE  ((0x4<<3) | (0x3<<0))
#elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_12)
#define RFI_RXCLKADJUST       ((0<<16) | (0x2<<3) | (0x0<<0))
#define RFI_DCLKCONF          ( (RFI_RX_PREAMBLE_NUM<<28) | (0xb<<24) | ((RFI_CLK_DIV_SEL*2)<<16) | (RFI_CLK_DIV_SEL<<8) | (RFI_CLK_DIV_SEL<<0))
#define RFI_RXTIMINGFINETUNE  ((0x2<<3) | (0x2<<0))
#elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_8)
#define RFI_RXCLKADJUST       ((0<<16) | (0x2<<3) | (0x0<<0))
#define RFI_DCLKCONF          ( (RFI_RX_PREAMBLE_NUM<<28) | (0x7<<24) | ((RFI_CLK_DIV_SEL*2)<<16) | (RFI_CLK_DIV_SEL<<8) | (RFI_CLK_DIV_SEL<<0))
#define RFI_RXTIMINGFINETUNE  ((0x2<<3) | (0x2<<0))
#elif(RFI_CLK_DIV_SEL == RFI_TXCLK_DIV_9)
#define RFI_RXCLKADJUST       ((0<<16) | (0x2<<3) | (0x0<<0))
#define RFI_DCLKCONF          ( (RFI_RX_PREAMBLE_NUM<<28) | (0x7<<24) | ((RFI_CLK_DIV_SEL*2)<<16) | (RFI_CLK_DIV_SEL<<8) | (RFI_CLK_DIV_SEL<<0))
#define RFI_RXTIMINGFINETUNE  ((0x2<<3) | (0x2<<0))
#endif

//----//
#define RFI_SYNCWORDLEN_SEL     RFI_SYNCWD_LEN_32
#define RFI_CUSTOMER_ID_EXT_SET 1  //Lucian: 暫時不能打開, 需改寫ACK,SYNC command 的寫法.
#define RFI_SUPERBURST_MODESEL  1



//------------------------//


#define RFI_BUF_SIZE_MASK                  (RFI_BUF_SIZE_GRPUNIT-1)

//Lucian: define command packet: 1 group
#define RFI_CMD_ADDR_OFFSET                (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 0) )

#define RFI_KEEP_ADDR_OFFSET               (RFI_CMD_ADDR_OFFSET + 25)
#define RFI_KEEP_ADDR_CHEKBIT               (0x01 << 25)


#define RFI_TXCMD1_ADDR_OFFSET             (RFI_CMD_ADDR_OFFSET + 26)
#define RFI_TXCMD1_ADDR_CHEKBIT            (0x01 << 26)

#define RFI_TXCMD2_ADDR_OFFSET             (RFI_CMD_ADDR_OFFSET + 27)
#define RFI_TXCMD2_ADDR_CHEKBIT            (0x01 << 27)


#define RFI_PAIR_ADDR_OFFSET               (RFI_CMD_ADDR_OFFSET + 28)
#define RFI_PAIR_ADDR_CHEKBIT              (0x01 << 28)

#define RFI_SYNC_ADDR_OFFSET               (RFI_CMD_ADDR_OFFSET + 29)
#define RFI_SYNC_ADDR_CHEKBIT              (0x01 << 29)

#define RFI_NACK_ADDR_OFFSET               (RFI_CMD_ADDR_OFFSET + 30)
#define RFI_NACK_ADDR_CHEKBIT              (0x01 << 30)

#define RFI_ACK_ADDR_OFFSET                (RFI_CMD_ADDR_OFFSET + 31)  //packet unit
#define RFI_ACK_ADDR_CHEKBIT               (0x01 << 31)

//Lucian: For dummy data: 1 group//
#define RFI_DUMMY_ADDR_OFFSET              (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 1) )

//Audio data return: 8 group//
#define RFI_AUDIORETURN1_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 2) )
#define RFI_AUDIORETURN2_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 3) )
#define RFI_AUDIORETURN3_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 4) )
#define RFI_AUDIORETURN4_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 5) )
#define RFI_AUDIORETURN5_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 6) )
#define RFI_AUDIORETURN6_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 7) )
#define RFI_AUDIORETURN7_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 8) )
#define RFI_AUDIORETURN8_ADDR_OFFSET        (RFI_GRP_INPKTUNIT * (RFI_BUF_SIZE_GRPUNIT + 9) )



#if RFIU_TEST
#define RFIU_TIMEOUT  6   // 200ms: Viterbi=1/2, T=12.
#else
#define RFIU_TIMEOUT  4   // 200ms: Viterbi=1/2, T=12.
#endif

#if RFI_TEST_2x_RX_PROTOCOL_B1
#define RFIU_nTx1RxSW_TIMEOUT  0   //wait forever

#elif RFI_TEST_4TX_2RX_PROTOCOL
#define RFIU_nTx1RxSW_TIMEOUT  0   //wait forever

#elif RFI_TEST_4x_RX_PROTOCOL_B1
#define RFIU_nTx1RxSW_TIMEOUT  0   //wait forever

#endif
//------------//
#define RFIU_SAMPLE_CLK  32

#define RFIU_PKT_SIZE 128

//-------//
#define RFI_TX_TRY_ACKCNT_MAX     128
#define RFI_RX_TRY_ACKCNT_MAX     (RFI_TX_TRY_ACKCNT_MAX*3/4)

//-------//
#define RFIU_TX_STATE_SYNC        0
#define RFIU_TX_STATE_INIT        1
#define RFIU_TX_STATE_READY       2
#define RFIU_TX_STATE_WAITACK     3

#define RFIU_RX_STATE_SYNC        8
#define RFIU_RX_STATE_LISTEN      9
#define RFIU_RX_STATE_REPLY_ACK   10
#define RFIU_RX_STATE_RETRY_ACK   11

//---For Usr Data----//
#define RFIU_USRDATA_DATACH_MASK     0x01f
#define RFIU_USRDATA_DATACH_SHFT     0
#define RFIU_USRDATA_DATACH_CHEK     0x000001f

#define RFIU_USRDATA_SEQNUM_MASK     0x03
#define RFIU_USRDATA_SEQNUM_SHFT     5
#define RFIU_USRDATA_SEQNUM_CHEK     0x000060

#define RFIU_USRDATA_FEC_MASK        0x03
#define RFIU_USRDATA_FEC_SHFT        7
#define RFIU_USRDATA_FEC_CHEK        0x000180

#define RFIU_USRDATA_MDDIF_MASK      0x01
#define RFIU_USRDATA_MDDIF_SHFT      10
#define RFIU_USRDATA_MDDIF_CHEK      0x000400

#define RFIU_USRDATA_TXGRPDIV_MASK   0x07
#define RFIU_USRDATA_TXGRPDIV_SHFT   11
#define RFIU_USRDATA_TXGRPDIV_CHEK   0x003800

#define RFIU_USRDATA_TXGRPSHF_MASK   0x3f
#define RFIU_USRDATA_TXGRPSHF_SHFT   14
#define RFIU_USRDATA_TXGRPSHF_CHEK   0x0fc000

#define RFIU_USRDATA_RXCMDTYP_MASK   0x0f
#define RFIU_USRDATA_RXCMDTYP_SHFT   10
#define RFIU_USRDATA_RXCMDTYP_CHEK   0x003c00

#define RFIU_USRDATA_RXSEQNUM_MASK   0x01
#define RFIU_USRDATA_RXSEQNUM_SHFT   9
#define RFIU_USRDATA_RXSEQNUM_CHEK   0x000200

#define RFIU_USRDATA_RXGRPDIV_MASK   0x07
#define RFIU_USRDATA_RXGRPDIV_SHFT   14
#define RFIU_USRDATA_RXGRPDIV_CHEK   0x01c000

#define RFIU_USRDATA_RXGRPWPR_MASK   0x07
#define RFIU_USRDATA_RXGRPWPR_SHFT   17
#define RFIU_USRDATA_RXGRPWPR_CHEK   0x0e0000

#define RFIU_USRDATA_CMD_MASK        0x01
#define RFIU_USRDATA_CMD_SHFT        20
#define RFIU_USRDATA_CMD_CHEK        0x100000

//-------------------//
#define RFIU_CAL_BITRATE_INTV        64  //must be 2^n
#define RFIU_ANALYSIS_CH_INTV        64  //must be 2^n

#define RFIU_ANALYSIS_FEC_INTV       64  //must be 2^n
//-----//

#define RFI_PIN_STATE                0x0500 // P0.7 =>TXOUT2 , P0.8 =>RXIN2 P0.9 =>TXOUT1 , P0.10 =>RXIN1
#define RFI_PIN                      0x00000780
#define RFI_PIN_STATE_RF1            0x00000038   //P0.3 P0.4 P0.5
#define RFI_PIN_STATE_RF2            0x00000006   //P2.1 P2.2
#define RFI_PIN_STATE_RF21           0x00020000   //P1.17


//-------//
typedef struct _REGRFIU_CFG {
    unsigned char *TxRxOpBaseAddr;
    unsigned int   Pkt_Grp_offset[16];
    
    unsigned int   Vitbi_en;
    unsigned int   CovCodeRateSel;
    unsigned int   RsCodeSizeSel;
    unsigned int   Customer_ID_ext_en;
    unsigned int   SuperBurstMode_en;

    unsigned int   DummyDataNum;
    unsigned int   PreambleNum;
    unsigned int   DummyPreambleNum;

    unsigned int   UserData_L;
    unsigned int   UserData_H;
    unsigned int   Customer_ID;
    unsigned int   PktSyncWord;

    unsigned int   CID_ErrCnt;

    unsigned int   TxClkConfig;
    unsigned int   RxClkAdjust;
    unsigned int   DclkConfig;
    unsigned int   RxTimingFineTune;

    unsigned int   SyncWordLenSel;

    unsigned int   PktMap[32];
   
    unsigned int   TxRxPktNum;
    unsigned int   DummyPktNum;
    unsigned int   BitStuffCnt;
      
    unsigned char  CMD_Data[5];     //commad data
    unsigned int   PASSWORD[2];  //password 64 bits
    
      
}DEF_REGRFIU_CFG;








typedef struct _RFIU_DAT_CH_STATISTICS
{
   unsigned int SentPktNum;
   unsigned int RecvPktNum;

   unsigned int BurstNum;
   unsigned int BrokenNum;

} DEF_RFIU_DAT_CH_STATISTICS;

typedef struct _RFIU_FEC_TYPE_STATISTICS
{
   unsigned int SentPktNum;
   unsigned int RecvPktNum;

} DEF_RFIU_FEC_TYPE_STATISTICS;

/*
 *********************************************************************************************************
 *Extern Function prototype
 *********************************************************************************************************
 */
extern unsigned int rfiuDataPktConfig_Rx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara);
extern unsigned int rfiuDataPktConfig_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara);
extern int rfiuWaitForInt_Tx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara);
extern u8 rfiuWaitForInt_Rx(int RFIunit,DEF_REGRFIU_CFG *pRfiuPara,int WifiCHsel,u8 *pRSSI2);
extern int rfiuPutPacketMap2ACK( int RFUnit, unsigned char *ACKBufAddr,DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext);
extern int rfiuPutTimeCheck2ACK(int RFUnit,unsigned char *ACKBufAddr,DEF_REGRFIU_CFG *pRfiuPara,unsigned int RX_TimeCheck,unsigned int CH_chg_flag,unsigned int RX_CHG_CHNext);
extern int rfiuPutInfo2SYNC(int RFUnit, unsigned char *ACKBufAddr);
extern int rfiuPutInfo2KEEP(int RFUnit,unsigned char *ACKBufAddr);

extern int rfiuProcessCmdPkt(unsigned char RFUnit);


extern int rfiuGetACK2PacketMap_UpdateMap(unsigned char RFUnit,unsigned char *ACKBufAddr,int *pRX_RecvTotalPktCnt,int *pRX_RecvDataPktCnt,int *pRxBufWritePtr);
extern int rfiuGetACK2TimeCheck(unsigned char RFUnit,unsigned char *ACKBufAddr,int *pRX_RecvPktCnt,int *pRX_RecvDataPktCnt);

extern int rfiuGetACKType(unsigned char RFUnit,
                           unsigned char *ACKBufAddr,
                           unsigned int  *pRXTimeCheck);

extern int rfiuGetACK2ProtocolType(int RFUnit,unsigned char *ACKBufAddr);
extern int rfiuGetACK2ChChange(int RFUnit,unsigned char *ACKBufAddr);

extern int rfiuGetACK2TimeCheck_Pair(unsigned char RFUnit,unsigned char *ACKBufAddr,
                                int *pRX_RecvPktCnt,int *pRX_RecvDataPktCnt);

extern int rfiuTxSendDataState(  unsigned char RFUnit,
                                      unsigned int Vitbi_en, 
                                      unsigned int RS_mode, 
                                      unsigned int Vitbi_mode,
                                      unsigned int SyncWord,
                                      unsigned int CustomerID,
                                      unsigned int UserData
                                     );
extern int rfiuRxListenDataState( unsigned char RFUnit,
                                        unsigned int Vitbi_en, 
                                        unsigned int RS_mode, 
                                        unsigned int Vitbi_mode,
                                        unsigned int SyncWord,
                                        unsigned int CustomerID,
                                        unsigned int TimeOut
                                      );

extern int rfiuTxWaitACKState( unsigned char RFUnit,
                                      unsigned int Vitbi_en, 
                                      unsigned int RS_mode, 
                                      unsigned int Vitbi_mode,
                                      unsigned int SyncWord,
                                      unsigned int CustomerID,
                                      unsigned int TimeOut
                                    );

extern int rfiuRxReplyACKState(  int RFUnit,
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
                                      );
extern int rfiuTxSentSYNCState(  int RFUnit,
                                        unsigned int Vitbi_en, 
                                        unsigned int RS_mode, 
                                        unsigned int Vitbi_mode,
                                        unsigned int SyncWord,
                                        unsigned int CustomerID,
                                        unsigned int UserData
                                     );

 extern int rfiuTxSentKEEPState( int RFUnit,
                                        unsigned int Vitbi_en, 
                                        unsigned int RS_mode, 
                                        unsigned int Vitbi_mode,
                                        unsigned int SyncWord,
                                        unsigned int CustomerID,
                                        unsigned int UserData
                                      );


extern int rfiuTxUpdatePktMap( unsigned char RFUnit,
                                      int DatPktRecvFlag,
                                      DEF_RFIU_USRDATA *pCtrlPara,
                                      DEF_RFIU_USRDATA *pCtrlPara_next,
                                      int *pRX_RecvPktCnt,
                                      int prev_DAT_CH_sel,
                                      DEF_RFIU_DAT_CH_STATISTICS TX_CH_Stat[],
                                      DEF_RFIU_FEC_TYPE_STATISTICS TX_FEC_Stat[]
                                    );
extern int rfiuEncUsrData(DEF_RFIU_USRDATA *pCtrlPara);
extern int rfiuDecUsrData(int UsrData,DEF_RFIU_USRDATA *pCtrlPara);

extern int rfiuRxSendWakeState(  
                                          int RFUnit,
                                          unsigned int Vitbi_en, 
                                          unsigned int RS_mode, 
                                          unsigned int Vitbi_mode,
                                          unsigned int SyncWord,
                                          unsigned int CustomerID,
                                          unsigned int UserData
                                      );


#if RFIU_TEST
extern int rfiuCheckTestResult(int TxUnit,int RxUnit,int CheckPktMap, int CheckPktBurstNum);
#endif

#endif
