#include "general.h"
#include "gpioapi.h"
#include "board.h"

#if(RFIC_SEL==RFIC_NONE_5M)
#include "rfiuapi.h"
#include "rfiu.h"
#include "../../gpio/inc/gpioreg.h"

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern int gRfiuSyncWordTable[MAX_RFIU_UNIT];
extern int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX];

//----------------------------//
void RFNONE_ChgTo_10M_B1(void)
{
}

void RFNONE_ChgTo_10M_B2(void)
{
	
}

void RFNONE_ChgTo_5M_B1(void)
{
}

void RFNONE_ChgTo_5M_B2(void)
{
}

/*********************************************************************
** RSSI_measurement_enable
*********************************************************************/
u8 RSSI_measurement_RFNONE(int BoardSel)
{
    u8 tmp=0;

    if(BoardSel == 1)
    {
    }
    else if(BoardSel == 2)
    {
    }

    return tmp;
}


void InitRFNONE()
{
       u8 level;
       int i;
       
#if ( (RFIC_SEL==RFIC_NONE_5M) )     
    #if(SYS_CPU_CLK_FREQ  == 160000000)
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00200000;  // 480/3=160
    #elif(SYS_CPU_CLK_FREQ  == 32000000)
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00000000;  // 32/1=32
    #else
      #if PWIFI_SUPPORT

      #else
        DEBUG_RFIU_P2("==RF CLOCK INVALID!!==\n");
        while(1);
      #endif 
    #endif
#endif

#if RFI_SELF_TEST_TXRX_PROTOCOL

#elif RFI_TEST_TX_PROTOCOL_B1
#elif RFI_TEST_TX_PROTOCOL_B2
#elif RFI_TEST_RX_PROTOCOL_B1

#elif RFI_TEST_RX_PROTOCOL_B2
#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_8TX_2RX_PROTOCOL)
#elif (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
#else
   #if (RFI_FCC_DIRECT_TRX == 1)
   #elif(RFI_FCC_DIRECT_TRX == 2)
   #else
   #endif
#endif

}
void RFNONE_TxMode_Start(int BoardSel)
{
    int i;
    
    if(BoardSel==1)
    {
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //RX START
    }
    else if(BoardSel==2)
    {
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //RX START
    }
    //for(i=0;i<100000;i++); //Delay 5 ms
}

void RFNONE_TxMode_Stop(int BoardSel)
{
    int i;
    
    //for(i=0;i<100000;i++); //Delay 5 ms
    if(BoardSel==1)
    {
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //RX START
    }
    else if(BoardSel==2)
    {
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //RX START
    }
}
void RFNONE_RxMode_Start(int BoardSel)
{
    if(BoardSel==1)
    {
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //RX START
    }
    else if(BoardSel==2)
    {
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //RX START
    }
}

void RFNONE_RxMode_Stop(int BoardSel)
{
    if(BoardSel==1)
    {
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //RX START
    }
    else if(BoardSel==2)
    {
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //TX START
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //RX START
    }
}

void RFNONE_CH_sel(int BoardSel,BYTE CH)
{
    if(BoardSel==1)
    {
    }
    else if(BoardSel==2)
    {
    }
    else if(BoardSel==3)
    {
    }
}

void RFNONE_ID_Update(int BoardSel ,unsigned int NewMACID )
{
    if(BoardSel==1)
    {
    }
    else if(BoardSel==2)
    {
    }
    else if(BoardSel==3)
    {
    }
}

/*********************************************************************
** WOR_enable
*********************************************************************/
s32 RFNONE_WOR_enable_B1(s32 dummy)
{
    u8 tmp;
    int i;

    DEBUG_RFIU_P2("=====A7196_WOR_enable_B1:Start=====\n");
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].TX_Task_Stop=1;
       gRfiuUnitCntl[i].TX_Wrap_Stop=1;
       gRfiuUnitCntl[i].TX_MpegEnc_Stop=1;

       gRfiuUnitCntl[i].RX_Task_Stop=1;
       gRfiuUnitCntl[i].RX_Wrap_Stop=1;
       gRfiuUnitCntl[i].RX_MpegDec_Stop=1;    
    }
    OSTimeDly(10);
    //---Power Down---//
    DEBUG_RFIU_P2("\n=====System power down=====\n");
    sysPowerOffDirect();
    //----------------//
}

void RFNONE_WOR_enable_B2(void)
{
    u8 tmp;
    int i;

    DEBUG_RFIU_P2("=====A7196_WOR_enable_B2:Start=====\n");
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].TX_Task_Stop=1;
       gRfiuUnitCntl[i].TX_Wrap_Stop=1;
       gRfiuUnitCntl[i].TX_MpegEnc_Stop=1;

       gRfiuUnitCntl[i].RX_Task_Stop=1;
       gRfiuUnitCntl[i].RX_Wrap_Stop=1;
       gRfiuUnitCntl[i].RX_MpegDec_Stop=1;    
    }
    OSTimeDly(10);
    //---Power Down---//
    DEBUG_RFIU_P2("\n=====System power down=====\n");
    sysPowerOffDirect();
    //----------------//
}

#endif




