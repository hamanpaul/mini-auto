/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    uart.c

Abstract:

    The routines of UART.

    This implements a simple (polled) RS232 serial driver.

    For example, it could output single characters on Serial Port A at 38400 Baud,
    8 bit, no parity, 1 stop bit.

    Initialize the port with init_serial_A() before calling sendchar().

    To monitor the characters output, use a null-modem cable to
    connect Serial Port A to an RS232 terminal or PC running a
    terminal emulator, e.g. HyperTerminal.

    Define SCATTER_PERIP if the UART register locations are
    defined in a scatter file

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "uartreg.h"
#include "task.h"
#include "uiapi.h"
#include "intapi.h"
#include "../../board/inc/intreg.h"
#include "gpsapi.h"
#include "rtcapi.h"
#include "uiKey.h"
#include "uartapi.h"
#include "i2capi.h"
#if TUTK_SUPPORT
#include "p2pserver_api.h"
#endif
#if HOME_RF_SUPPORT
#include "rfiuapi.h"
#endif
#if SYSTEM_DEBUG_SD_LOG
extern u8  Start_Record;
u8 DEBUG_SD_BUF1[512];
u8 DEBUG_SD_BUF2[512];
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
extern u8 rfiuVocDustMode; /*0: Normal, 1: Voc Test Mode, 2: Dust Test Mode*/
#endif

/*****************************************************************************/
/* Constant                                                  */
/*****************************************************************************/

/*------ 設定 UART 1:Baud rate-----*/
#if UART_GPS_COMMAND
#define UART1_BAUD_RATE      9600
#else
#if(HW_BOARD_OPTION != MR6730_AFN)
#if(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)
#define UART1_BAUD_RATE      115200
#else
#define UART1_BAUD_RATE      19200
#endif
#else

	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_YD5150)
	#if(YD_STD_FUNC_VER)	
	#define UART1_BAUD_RATE      19200
	#else
	#define UART1_BAUD_RATE      4800	//<--- YD cancel it at 150330	
	#endif
	
	#else
	#define UART1_BAUD_RATE      19200

	#endif 
	
#endif//
#endif  //UART_GPS_COMMAND

/*------ 設定 UART 2:Baud rate-----*/
#if HOME_RF_SUPPORT  
    #if((HOME_RF_OPTION == HOME_SENSOR_MARS) || (HOME_RF_OPTION == HOME_SENSOR_BARVOTECH) || (HOME_RF_OPTION == HOME_SENSOR_SWANN) || (HOME_RF_OPTION == HOME_SENSOR_TRANWO))
        #define UART2_BAUD_RATE      115200
    #else
        #define UART2_BAUD_RATE      9600
    #endif
#elif (BLE_SUPPORT)
    #if(HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
        #define UART2_BAUD_RATE      115200
    #endif
#elif((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
      (HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
      (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
#define UART2_BAUD_RATE      9600

#else
#define UART2_BAUD_RATE      2400
#endif

/*------ 設定 UART 3:Baud rate-----*/
#define UART3_BAUD_RATE      9600



u32 sys_frequency=SYS_CPU_CLK_FREQ;
u32 UART_baudrate[3]={UART1_BAUD_RATE,UART2_BAUD_RATE,UART3_BAUD_RATE};

/* Divisor Latch High and Low - DLH and DLL */
#define UART1_DLH        (((sys_frequency / (16 * UART_baudrate[0])) >> 8) & 0xff)
#define UART1_DLL        ((sys_frequency / (16 * UART_baudrate[0])) & 0xff)

#define UART2_DLH        (((sys_frequency / (16 * UART_baudrate[1])) >> 8) & 0xff)
#define UART2_DLL        ((sys_frequency / (16 * UART_baudrate[1])) & 0xff)

#define UART3_DLH        (((sys_frequency / (16 * UART_baudrate[2])) >> 8) & 0xff)
#define UART3_DLL        ((sys_frequency / (16 * UART_baudrate[2])) & 0xff)

#if(HOME_RF_SUPPORT)
	#define UART_MAX_RCV_BUF_SIZ    2048
#else
	#define UART_MAX_RCV_BUF_SIZ    1024
#endif
#define UART_MAX_XMT_BUF_SIZ    512

#if UART_GPS_COMMAND
    #define UART_CMDBUF_SIZE   100
#else
    #define UART_CMDBUF_SIZE   40
#endif

#define TERMNULL        0
#define BACKSPACE       8
#define NEWLINE         10
#define ENTER           13

#define UART_USING_INT  1   //啟動 Interrupt event, 接收資料.


#define FLAUART_RX_READY    0x00000001
#define FLAUART_TX_READY    0x00000002
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#define FLAUART2_RX_READY    0x00000004
#define FLAUART2_TX_READY    0x00000008
#define FLAUART3_RX_READY    0x00000010
#define FLAUART3_TX_READY    0x00000020
#endif
enum
{
    UART_USE_DISABLE = 0,
    UART_USE_ENABLE,
};

/*****************************************************************************/
/* Variable                                                  */
/*****************************************************************************/

char uartRcvBuf[UART_MAX_NUM][UART_MAX_RCV_BUF_SIZ];
volatile unsigned int uartRcvHead[UART_MAX_NUM] = {0};
volatile unsigned int uartRcvTail[UART_MAX_NUM] = {0};
char uartXmtBuf[UART_MAX_NUM][UART_MAX_XMT_BUF_SIZ];
volatile unsigned int uartXmtHead[UART_MAX_NUM] = {0};
volatile unsigned int uartXmtTail[UART_MAX_NUM] = {0};

INT32U UartUseTable[UART_MAX_NUM] =
{
    UART_USE_ENABLE,
#if (UART_PTZ485_COMMAND_RX || UART_PTZ485_COMMAND_TX || HOME_RF_SUPPORT || BLE_SUPPORT  )
    UART_USE_ENABLE,
#elif ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
       (HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
       (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    UART_USE_ENABLE,
#else
    UART_USE_DISABLE,
#endif
    UART_USE_DISABLE
};

INT32U UartLsrReg[UART_MAX_NUM] =
{
    UartLsr,
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Lsr,
    Uart3Lsr,
#endif
};

INT32U UartDrReg[UART_MAX_NUM] =
{
    UartDr,
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Dr,
    Uart3Dr,
#endif
};

INT32U UartFcrReg[UART_MAX_NUM] =
{
    UartFcr,
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Fcr,
    Uart3Fcr,
#endif
};

INT32U UartLcrReg[UART_MAX_NUM] =
{
    UartLcr,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Lcr,
    Uart3Lcr,
#endif
};

INT32U UartDlhReg[UART_MAX_NUM] =
{
    UartDlh,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Dlh,
    Uart3Dlh,
#endif
};

INT32U UartDllReg[UART_MAX_NUM] =
{
    UartDll,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Dll,
    Uart3Dll,
#endif
};

INT32U UartIerReg[UART_MAX_NUM] =
{
    UartIer,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    Uart2Ier,
    Uart3Ier,
#endif
};

INT32U UartFlagRxReady[UART_MAX_NUM] =
{
    FLAUART_RX_READY,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    FLAUART2_RX_READY,
    FLAUART3_RX_READY,
#endif
};

INT32U UartFlagTxReady[UART_MAX_NUM] =
{
    FLAUART_TX_READY,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    FLAUART2_TX_READY,
    FLAUART3_TX_READY,
#endif
};

static u8  uart_state[UART_MAX_NUM]=0;

#if UART_COMMAND
    OS_STK  uartCmdTaskStack[UARTCMD_TASK_STACK_SIZE];
    char      UartCommand[UART_CMDBUF_SIZE];
    OS_EVENT* uart_MboxEvt;

 #if UART_GPS_COMMAND
    GPS_DATA    gGPS_data, gGPS_data1, gGPS_data2;
    OS_EVENT* GPSUpdateEvt;

    int gGPSPKErrorCount = 0;
    int gGPSSatellitesUsed=0;
    int gGPSFixValid=0;
 #endif

    #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A))
    OS_STK      uart2CmdTaskStack[UARTCMD_TASK_STACK_SIZE];
    char        Uart2Command[UART_CMDBUF_SIZE];
    OS_EVENT*   uart2_MboxEvt;

    OS_STK      uart3CmdTaskStack[UARTCMD_TASK_STACK_SIZE];
    char        Uart3Command[UART_CMDBUF_SIZE];
    OS_EVENT*   uart3_MboxEvt;
    #endif

#endif


#if HOME_RF_SUPPORT   
    char UartHomeRFCmd[UART_CMDBUF_SIZE];
#endif

#if(HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS)
    char UartCmdBuf[UART_CMDBUF_SIZE];
#endif
int TaskID[UART_MAX_NUM] =
{
    UART_1_ID,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    UART_2_ID,
    UART_3_ID,
#endif
};

#if UART_COMMAND
OS_STK *TaskStack[UART_MAX_NUM] =
{
    UARTCMD_TASK_STACK,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    UART2CMD_TASK_STACK,
    UART3CMD_TASK_STACK,
#endif
};

INT8U TaskPriority[UART_MAX_NUM] =
{
    UARTCMD_TASK1_PRIORITY,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    UARTCMD_TASK2_PRIORITY,
    UARTCMD_TASK3_PRIORITY,
#endif
};
#endif

OS_FLAG_GRP  *gUARTFlagGrp;


/*****************************************************************************/
/* Function prototype                                                */
/*****************************************************************************/

signed long uartBufRcv(u8 Uart_ID);
signed long uartBufXmt(u8 Uart_ID);


/*****************************************************************************/
/* extern variable prototype                                                 */
/*****************************************************************************/
extern volatile u8 gAutoTest;

extern u8 Main_Init_Ready;

/*****************************************************************************/
/* extern function prototype                                                 */
/*****************************************************************************/

extern int dm9000_probe(void);

extern s32 uiMenuSet_CapResolution(s8 setting);

#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
extern s32 i2cWrite_ALC5621(u8 addr, u16 data);
extern s32 i2cRead_ALC5621(u8 addr, u16* pData);
#endif
extern s32 sysPlayRingWave(u8);
extern s32 sysReadWaveRing(u8 ucRingWaveNum);
extern s32 sysWriteWaveRing(u8* pucSrcWaveFilesAddr, u8 ucRingWaveNum);
extern s32 sysWriteRingWave2SD(u8 ucRingWaveNum);
extern s32 sysDumpWaveRing(void);
extern s32 i2cWrite_SENSOR(u8, u16);
extern s32 i2cRead_SENSOR(u8, u16*);
enum DM9KS_PHY_mode 
    {	DM9KS_10MHD   = 0,
        DM9KS_100MHD  = 1,
        DM9KS_10MFD   = 4,
        DM9KS_100MFD  = 5,
        DM9KS_AUTOMF  = 7, //dm9000c
        DM9KS_AUTO    = 8
        };

extern void set_PHY_mode(enum DM9KS_PHY_mode);
extern void dump_dm9000_phy_regs(void);
extern void dump_dm9000_control_regs(void);
extern void UartWriteDM9000Reg(char *UartCmdString);
extern void UartReadDM9000Reg(char *UartCmdString);
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
extern void sendrawdata(void);
#endif
extern void  RTC_Set_GMT_Time(RTC_DATE_TIME *cur_time);


/*****************************************************************************/
/* Function                                                  */
/*****************************************************************************/

/*

Routine Description:

    Initialize UART.

Arguments:

    None.

Return Value:

    None.

*/
void init_serial_A(void)
{
    u8 i;

    for (i = 0; i < UART_MAX_NUM; i++)
    {
        if(UartUseTable[i] == UART_USE_DISABLE)
        {
            *((volatile unsigned *)UartIerReg[i]) = 0;
            continue;
        }
        //marsIntIRQDisable(INT_IRQ_MASK_UART);
        /* First set the correct baud rate and word length */
        *((volatile unsigned *)UartFcrReg[i]) =     UART_FCR_FIFO_ENA |
                                                    UART_FCR_DMA_MODE_1 |
                                                    UART_FCR_RCVR_INT_TRIG_8;

        *((volatile unsigned *)UartLcrReg[i]) =     UART_LCR_CHAR_LEN_8 |
                                                    UART_LCR_NUM_STOP_1 |
                                                    UART_LCR_DIV_LAT_ACC;

        //設定baud rate
        if(i==0)
        {
            *((volatile unsigned *)UartDlhReg[i]) =   UART1_DLH;
            *((volatile unsigned *)UartDllReg[i]) =   UART1_DLL;
        }
        else if(i==1)
        {
            *((volatile unsigned *)UartDlhReg[i]) =   UART2_DLH;
            *((volatile unsigned *)UartDllReg[i]) =   UART2_DLL;
        }
        else if(i==2)
        {
            *((volatile unsigned *)UartDlhReg[i]) =   UART3_DLH;
            *((volatile unsigned *)UartDllReg[i]) =   UART3_DLL;
        }
        
        *((volatile unsigned *)UartLcrReg[i]) &=  ~UART_LCR_DIV_LAT_ACC;

        /* Enable the necessary interrupts */
        #if UART_USING_INT
            *((volatile unsigned *)UartIerReg[i]) = UART_IER_RBR_AVL_INT_ENA;
        #endif
        DEBUG_UART("Init UART-%d complete\n",i);

    }
    /* Now enable the serial port */
    /* No need to do so */
}

/*

Routine Description:

    Send a character to UART.

Arguments:

    ch - The character to send.

Return Value:

    None.

*/
void sendchar(u8 Uart_ID, unsigned char *ch)
{
    static int charCnt[3] = {0};
	static int uart_char=0;
	static u8 buf_bit = 0;

    if( (charCnt[Uart_ID] & 0xf)  == 0)    /*避免連續polling拖累系統，每送16個字再polling*/
    {
        while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_THR_EMP) == 0);     // wait until THR is empty
    }
    *((volatile unsigned *)UartDrReg[Uart_ID]) = *ch;                         // Transmit next character


#if SYSTEM_DEBUG_SD_LOG	
	if(buf_bit == 0)
	{
		if(uart_char>=sizeof(DEBUG_SD_BUF1) && Start_Record == 1)
		{
			uart_char= 0;
			buf_bit  = 1;
			DEBUG_SD_BUF2[uart_char++] = *ch;
			sysUartLogRecord(!buf_bit);
			memset(DEBUG_SD_BUF1,0,sizeof(DEBUG_SD_BUF1));
		}
		else
			DEBUG_SD_BUF1[uart_char++] = *ch;	
	}
	else
	{
		if(uart_char>=sizeof(DEBUG_SD_BUF2) && Start_Record == 1)
		{
			uart_char= 0;
			buf_bit  = 0;
			DEBUG_SD_BUF1[uart_char++] = *ch;
			sysUartLogRecord(!buf_bit);
			memset(DEBUG_SD_BUF2,0,sizeof(DEBUG_SD_BUF2));
		}
		else
			DEBUG_SD_BUF2[uart_char++] = *ch;	
	}

#endif

	charCnt[Uart_ID]++;

    if (*ch == '\n')  // Send '\r' after '\n'
    {
        *ch = '\r';
        if( (charCnt[Uart_ID] & 0xf)  == 0)    /*避免連續polling拖累系統，每送16個字再polling*/
        {
           while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_THR_EMP) == 0);  // wait until THR is empty
        }
        *((volatile unsigned *)UartDrReg[Uart_ID]) = *ch;                       // Transmit next character

		charCnt[Uart_ID]++;
    }
}


/*

Routine Description:

    Send a byte to UART.

Arguments:

    data

Return Value:

    None.

*/
void sendData(u8 Uart_ID, unsigned char *ch)
{
    static int charCnt[3] = {0};

    if( (charCnt[Uart_ID] & 0xf)  == 0)    /*避免連續polling拖累系統，每送16個字再polling*/
    {
        while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_THR_EMP) == 0);     // wait until THR is empty
    }
    *((volatile unsigned *)UartDrReg[Uart_ID]) = *ch;                         // Transmit next character
    charCnt[Uart_ID]++;
    
}


/*

Routine Description:

    Send burst data to UART.

Arguments:

    ucstr - The data to send.
    len   - data len, len must be less or equal 16 bytes,
              don't check error condition for performance

Return Value:

    None.

*/
void sendburst(u8 Uart_ID, unsigned char *ucstr, u8 len)
{
    u8 i;
    while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_THR_EMP) == 0)       // wait until THR is empty
    ;

    for( i=0 ; i<len ; i++)
    {
        *((volatile unsigned *)UartDrReg[Uart_ID]) = *ucstr;                        // Transmit next character
        ucstr++;
    }
}

/*

Routine Description:

    Receive a character from UART.

Arguments:

    None.

Return Value:

    The character received.

*/
char receive_char(u8 Uart_ID)
{
    volatile char ch = '0';

#if UART_USING_INT

        INT8U   err;

        if (uartRcvHead[Uart_ID] == uartRcvTail[Uart_ID])
        {
            OSFlagPost(gUARTFlagGrp, UartFlagRxReady[Uart_ID], OS_FLAG_CLR, &err);
            OSFlagPend(gUARTFlagGrp, UartFlagRxReady[Uart_ID], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        }

        ch = uartRcvBuf[Uart_ID][uartRcvHead[Uart_ID]++];

        if(uartRcvHead[Uart_ID] >= UART_MAX_RCV_BUF_SIZ)
            uartRcvHead[Uart_ID] = 0;

#else //polling: 此方法無效率, Data lost 機率高.
    volatile char status;
    while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_DATA_RDY) == 0)  // wait until data ready in RBR
    ;
    ch = *((volatile unsigned *)UartDrReg[Uart_ID]);        // receive character
    status = *((volatile unsigned *)UartLsrReg[Uart_ID]);   // read the status register

    /* Could check status receive status here */
#endif
    return ch;
}

/*

Routine Description:

    The IRQ handler of UART1.

Arguments:

    None.

Return Value:

    None.

*/
void uartIntHandler(void)
{
    char iir;
    //char lsr, msr;

    iir = *((volatile unsigned *)UartIir) & 0x0f;

    if (iir == UART_IIR_RCVR_STAT)
    {
        /* Overrun/parity/framing errors or break interrupt. */
        //lsr = *((volatile unsigned *)UartLsrReg[Uart_ID]);
    }

    if ((iir == UART_IIR_RBR_AVL) || (iir == UART_IIR_CHAR_TIME_OUT))
    {
        /* Receiver data available or read data FIFO trigger level reached.     */
        /*  Process the received data later in a UART task.         */

        /* No characters in or out the RECV FIFO during th last 4 charater times and    */
        /*  there is at least 1 charater in it during this time.            */
        /*  Process the received data later in a UART task.             */
        uartBufRcv(UART_1_ID);
    }

//  if (iir == UART_IIR_CHAR_TIME_OUT)
//  {
//      /* No characters in or out the RECV FIFO during th last 4 charater times and    */
//      /*  there is at least 1 charater in it during this time.            */
//      /*  Process the received data later in a UART task.             */
//      uartBufRcv(UART_1_ID);
//  }

    if (iir == UART_IIR_THR_EMPTY)
    {
        /* Transmitter holding register empty.                  */
        /*  Prepare the transmitting data in advance in a UART task.    */
        //uartBufXmt(UART_1_ID);
    }

    if (iir == UART_IIR_MODEM_STAT_CHANGE)
    {
        /* Clear to send or data set ready or ring indicator or data center detect */
        //msr = *((volatile unsigned *)UartMsr);
    }

    if (iir == UART_IIR_NO_INT_PEND)
    {
        /* No interrupt source */
    }
}

/*

Routine Description:

    The test routine of UART.

Arguments:

    None.

Return Value:

    None.

*/
void uartTest(void)
{

}

#if HOME_RF_SUPPORT
/*

Routine Description:

    Respond HomeAutomation Ack from UART2.(HOMERF_RX_CMDID_PERIOD_EVENT, HOMERF_RX_CMDID_TRIG_EVENET)

Arguments:

    None.

Return Value:

    None.

*/
void uartHA_ACK(void)
{
    int i=0, j=0;
    u8 checksum=0;
    u8 tri_cmdStr[5] = {0x69, 0x96, 0x01, 0x41, 0x42};
    u8 prd_cmdStr[5] = {0x69, 0x96, 0x01, 0x42, 0x43};
    
    if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]-13])%UART_MAX_RCV_BUF_SIZ == 0x69)
    if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]-12])%UART_MAX_RCV_BUF_SIZ == 0x96)
    if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]-11])%UART_MAX_RCV_BUF_SIZ == 0x09)
    if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 10])%UART_MAX_RCV_BUF_SIZ == 0x11 || (uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 10])%UART_MAX_RCV_BUF_SIZ == 0x12)
    {
        for(i=0; i<=(uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 11])%UART_MAX_RCV_BUF_SIZ; i++)
            checksum+=((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 11 + i])%UART_MAX_RCV_BUF_SIZ);
        //printf("checksum = %x\n",checksum);
        //printf("uartRcvBuf = %x\n",(uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]-1])%UART_MAX_RCV_BUF_SIZ);
        if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]-1])%UART_MAX_RCV_BUF_SIZ == checksum)
        {
            if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 10])%UART_MAX_RCV_BUF_SIZ == 0x11)        //HOMERF_RX_CMDID_TRIG_EVENET
                for(j=0; j<5; j++)
                    sendData(HOMERF_UART_ID,&tri_cmdStr[j]);
            else if((uartRcvBuf[HOMERF_UART_ID][uartRcvTail[HOMERF_UART_ID]- 10])%UART_MAX_RCV_BUF_SIZ == 0x12)//HOMERF_RX_CMDID_PERIOD_EVENT
                for(j=0; j<5; j++)
                    sendData(HOMERF_UART_ID,&prd_cmdStr[j]);  
        }                        
    }
     
}
#endif

/*

Routine Description:

    Receive a character from UART1 to buffer.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
signed long uartBufRcv(u8 Uart_ID)
{
    unsigned int temp,i;
    INT8U   err;
    char    ch;
	
    i=0;
    
    while ((*((volatile unsigned *)UartLsrReg[Uart_ID]) & UART_LSR_DATA_RDY) != 0)  //讀到清為零為止: FIFO 讀完,會自動清為零
    {

        temp=(uartRcvTail[Uart_ID]+1)%UART_MAX_RCV_BUF_SIZ;
        ch = (char)*((volatile unsigned *)UartDrReg[Uart_ID]);
		


		
    #if UART_GPS_COMMAND
        if (Main_Init_Ready==1)  //等 mainTask 進入 while(1), 才開始接收 DATA
    #else
        if (uart_state[Uart_ID]==1)       //等 uartTask 進入 while(1), 才開始接收資料.
    #endif
        {
            if (temp == uartRcvHead[Uart_ID])
            {
                DEBUG_UART("UartOverFlow\n");
                return 0;
            }

            uartRcvBuf[Uart_ID][uartRcvTail[Uart_ID]] = ch;

            uartRcvTail[Uart_ID] = temp;
            i++; 
	}    
#if HOME_RF_SUPPORT        
        if(Uart_ID == HOMERF_UART_ID)   //20151019_Sean
            uartHA_ACK();
#endif            
    }

    if (i)
        OSFlagPost(gUARTFlagGrp, UartFlagRxReady[Uart_ID], OS_FLAG_SET, &err);

    return 1;
}

/*

Routine Description:

    Transmit a character to UART1 from buffer.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
signed long uartBufXmt(u8 Uart_ID)
{
    int i;

    for( i=0 ; i<16 ; i++)
    {
        if(uartXmtHead[Uart_ID] == uartXmtTail[Uart_ID])
            break;

        *((volatile unsigned *)UartDrReg[Uart_ID]) = uartXmtBuf[Uart_ID][uartXmtTail[Uart_ID]];
        uartXmtTail[Uart_ID]=(uartXmtTail[Uart_ID]+1)%UART_MAX_XMT_BUF_SIZ;
    }

    return 1;
}

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
/*

Routine Description:

    The IRQ handler of UART2.

Arguments:

    None.

Return Value:

    None.

*/
void uart2IntHandler(void)
{
    char iir;
    //char lsr, msr;
    
    iir = *((volatile unsigned *)Uart2Iir) & 0x0f;

    if (iir == UART_IIR_RCVR_STAT)
    {
        /* Overrun/parity/framing errors or break interrupt. */
        //lsr = *((volatile unsigned *)UartLsrReg[Uart_ID]);
    }

    if ((iir == UART_IIR_RBR_AVL) || (iir == UART_IIR_CHAR_TIME_OUT))
    {
        /* Receiver data available or read data FIFO trigger level reached.     */
        /*  Process the received data later in a UART task.         */

        /* No characters in or out the RECV FIFO during th last 4 charater times and    */
        /*  there is at least 1 charater in it during this time.            */
        /*  Process the received data later in a UART task.             */
        uartBufRcv(UART_2_ID);
    }

//  if (iir == UART_IIR_CHAR_TIME_OUT)
//  {
//      /* No characters in or out the RECV FIFO during th last 4 charater times and    */
//      /*  there is at least 1 charater in it during this time.            */
//      /*  Process the received data later in a UART task.             */
//      uartBufRcv(UART_2_ID);
//  }

    if (iir == UART_IIR_THR_EMPTY)
    {
        /* Transmitter holding register empty.                  */
        /*  Prepare the transmitting data in advance in a UART task.    */
        //uartBufXmt(UART_2_ID);
    }

    if (iir == UART_IIR_MODEM_STAT_CHANGE)
    {
        /* Clear to send or data set ready or ring indicator or data center detect */
        //msr = *((volatile unsigned *)UartMsr);
    }

    if (iir == UART_IIR_NO_INT_PEND)
    {
        /* No interrupt source */
    }
}

void uart3IntHandler(void)
{
    char iir;
    //char lsr, msr;

    iir = *((volatile unsigned *)Uart3Iir) & 0x0f;

    if (iir == UART_IIR_RCVR_STAT)
    {
        /* Overrun/parity/framing errors or break interrupt. */
        //lsr = *((volatile unsigned *)UartLsrReg[Uart_ID]);
    }

    if ((iir == UART_IIR_RBR_AVL) || (iir == UART_IIR_CHAR_TIME_OUT))
    {
        /* Receiver data available or read data FIFO trigger level reached.     */
        /*  Process the received data later in a UART task.         */

        /* No characters in or out the RECV FIFO during th last 4 charater times and    */
        /*  there is at least 1 charater in it during this time.            */
        /*  Process the received data later in a UART task.             */
        uartBufRcv(UART_3_ID);
    }

//  if (iir == UART_IIR_CHAR_TIME_OUT)
//  {
//      /* No characters in or out the RECV FIFO during th last 4 charater times and    */
//      /*  there is at least 1 charater in it during this time.            */
//      /*  Process the received data later in a UART task.             */
//      uartBufRcv(UART_3_ID);
//  }

    if (iir == UART_IIR_THR_EMPTY)
    {
        /* Transmitter holding register empty.                  */
        /*  Prepare the transmitting data in advance in a UART task.    */
        //uartBufXmt(UART_3_ID);
    }

    if (iir == UART_IIR_MODEM_STAT_CHANGE)
    {
        /* Clear to send or data set ready or ring indicator or data center detect */
        //msr = *((volatile unsigned *)UartMsr);
    }

    if (iir == UART_IIR_NO_INT_PEND)
    {
        /* No interrupt source */
    }
}

#endif

#if UART_COMMAND

void uartCmdInit(void)
{
    INT8U   Result;
    INT8U   err;
    u8      i;

#if 1
    for (i = UART_1_ID; i < UART_MAX_NUM; i++)
    {
        if(UartUseTable[i] == UART_USE_DISABLE)
            continue;
        Result  = OSTaskCreate(UARTCMD_TASK, (void *)&TaskID[i], TaskStack[i], TaskPriority[i]);
        if(Result != OS_NO_ERR)
        {
            DEBUG_UART("uartCmdInit %d error!!!\n", i);
        }
    }
#else
    Result  = OSTaskCreate(UARTCMD_TASK, UARTCMD_TASK_PARAMETER, UARTCMD_TASK_STACK, UARTCMD_TASK1_PRIORITY);
    if(Result != OS_NO_ERR)
    {
        DEBUG_UART("uartCmdInit1 error!!!\n");
    }
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) ||(CHIP_OPTION == CHIP_A1026A))
    Result  = OSTaskCreate(UARTCMD_TASK, UART2CMD_TASK_PARAMETER, UART2CMD_TASK_STACK, UARTCMD_TASK2_PRIORITY);
    if(Result != OS_NO_ERR)
    {
        DEBUG_UART("uartCmdInit2 error!!!\n");
    }
    Result  = OSTaskCreate(UARTCMD_TASK, UART3CMD_TASK_PARAMETER, UART3CMD_TASK_STACK, UARTCMD_TASK3_PRIORITY);
    if(Result != OS_NO_ERR)
    {
        DEBUG_UART("uartCmdInit3 error!!!\n");
    }
#endif
#endif
    gUARTFlagGrp = OSFlagCreate(0x00000000, &err);
    uart_MboxEvt = OSMboxCreate(NULL);
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    uart2_MboxEvt = OSMboxCreate(NULL);
    uart3_MboxEvt = OSMboxCreate(NULL);
#endif

}

#if UART_GPS_COMMAND

void GPS_SendCheckSum(char *UartGPSCmdString)
{
int i;
u8 checksum=0;
char strGPS[5];

    for( i=1 ; i<strlen(UartGPSCmdString) ; i++ )
    {
        checksum ^= *(UartGPSCmdString+i);
    }
    sprintf(strGPS,"*%02X",checksum);
    printf("%s\r\n",strGPS);

}

void GPS_Init(void)
{
    char strGPS[50];

#if (GPS_PROCESS_GLL == 0)
   //disable GLL command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_GLL);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if (GPS_PROCESS_GSA == 0)
    //disable GSA command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_GSA);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if (GPS_PROCESS_GSV == 0)
    //disable GSV command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_GSV);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if (GPS_PROCESS_GGA == 0)
    //disable GGA command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_GGA);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if (GPS_PROCESS_RMC == 0)
    //disable RMC command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_RMC);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if (GPS_PROCESS_VTG == 0)
    //disable VTG command
    sprintf(strGPS,"$PSRF103,%02d,00,00,01",GPS_COMMAND_VTG);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if 0
    //enable GGA command
    sprintf(strGPS,"$PSRF103,%02d,00,01,01",GPS_COMMAND_GGA);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif

#if 0
    //enable RMC command
    sprintf(strGPS,"$PSRF103,%02d,00,01,01",GPS_COMMAND_RMC);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);

    //enable VTG command
    sprintf(strGPS,"$PSRF103,%02d,00,01,01",GPS_COMMAND_VTG);
    printf("%s",strGPS);
    GPS_SendCheckSum(strGPS);
#endif


}

u8 UARTDoGPSComand_Checksum(char *UartGPSCmdString)
{
    int i=0, len,checksum_in;
    u8 ch, checksum=0;

    len=strlen(UartGPSCmdString);
    while(i<len)
    {
        ch = *UartGPSCmdString;
        if( ch=='*' )
            break;

        checksum ^= ch;
        UartGPSCmdString++;
        i++;
    }

    UartGPSCmdString++;     //escape the '*'

    sscanf(UartGPSCmdString, "%x", &checksum_in);
    //DEBUG_UART("Checksum=%X, %X, %s\n",checksum, checksum_in, UartGPSCmdString);

    if(checksum == ((u8)checksum_in))
        return 1;
    else
        return 0;

}

s32 UARTDoGPSCommand_GetField(char *UartGPSCmdString, int *len, int *type)
{
char *ptr, ch;
s32 dwvalue=0;
int stringlen;
    *len= 0;
    ptr = UartGPSCmdString;
    stringlen = (int)strlen(UartGPSCmdString);
    while(*len<stringlen)
    {
        ch = *ptr;
//      DEBUG_UART("GetField %s: %c,%d,%d,%d \n", ptr, ch, dwvalue, *len, stringlen);

        if(ch==',')
        {
            *type = 1;
            return dwvalue;
        }
        else if(ch=='.')
        {
            *type = 2;
            return dwvalue;
        }
        else if(ch=='*')
        {
            *type = 3;
            return dwvalue;
        }

        dwvalue = dwvalue*10+(ch-'0');
        ptr++; (*len)++;


    }

    *type = 4;
    return dwvalue;
}

char UARTDoGPSCommand_GetChar(char *UartGPSCmdString, int *len)
{
char ch;

    ch = *UartGPSCmdString;

//  DEBUG_UART("GetChar %s: %c,%c \n", UartGPSCmdString, ch, *UartGPSCmdString);
    if((ch==',')||(ch=='*'))
    {
        *len = 1;
        return 0;
    }
    *len = 2;

    return ch;
}

//UARTDoGPSCommand_GetFloat : parse the float filed


s32 UARTDoGPSCommand_GetFloat(char *UartGPSCmdString, int *len, s32 *s32int, int *ifract)
{
char *ptr;
s32 dwvalue=0;
int filedlen=0,type;

    ptr = UartGPSCmdString;
    *len = 0;

    dwvalue = UARTDoGPSCommand_GetField(ptr, &filedlen, &type);
//  DEBUG_UART("integer %s: %d,%d,%d \n", ptr, dwvalue, filedlen, type);

    if( (filedlen == 0) || (type != 2))
    {
        return -1;
    }

    *s32int = dwvalue;

    ptr += (filedlen+1);
    (*len) = (filedlen+1);

    dwvalue = UARTDoGPSCommand_GetField(ptr, &filedlen, &type);

//  DEBUG_UART("fraction %s: %d,%d,%d \n", ptr, dwvalue, filedlen, type);
    if( (filedlen == 0) || (type != 1))
    {
        return -2;
    }
    *ifract = dwvalue;

    (*len) += (filedlen+1);

    return 1;
}



u8 UARTDoGPSCommand_GGA(char *UartGPSCmdString)
{

#if (GPS_PROCESS_GGA == 1)
    long dwvalue, remain;
    int wvalue1, valid=1,len, type;
    char *ptr,ch;
    s32 ret;


    ptr = UartGPSCmdString;

    //Get UTC time
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
    }
    //DEBUG_UART("Time %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Time : %d:%d:%d \n", gGPS_data.Hour, gGPS_data.Min, gGPS_data.Sec);
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get Latitude
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
    }
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get N_S
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    //DEBUG_UART("N_S: %c \n", gGPS_data.N_S);
    ptr += len;

    //Get Longitude
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
    }
    //DEBUG_UART("Longitude %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Longitude : %d.%d \n", gGPS_data.Lon_I, gGPS_data.Lon_F);
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get E_W
    ch = UARTDoGPSCommand_GetChar(ptr, &len);

    //DEBUG_UART("E_W: %c \n", gGPS_data.E_W);
    ptr += len;

    //Get Position Fix indicator
    dwvalue = UARTDoGPSCommand_GetField(ptr, &len, &type);

    if( len == 0)
    {
        valid = 0;
        gGPSFixValid=0;
    }
    else
    {
        if(dwvalue==0)
            gGPSFixValid = 0;
        else
            gGPSFixValid = 1;
    }
    //DEBUG_UART("gGPSFixValid= %d\n",gGPSFixValid);

    ptr += (len+1);

    //Get Satellites Used
    dwvalue = UARTDoGPSCommand_GetField(ptr, &len, &type);

    if( len == 0)
    {
        valid = 0;
        gGPSSatellitesUsed = 0;
    }
    else
    {
        gGPSSatellitesUsed = dwvalue;
    }

    ptr += (len+1);

    //DEBUG_UART("gGPSSatellitesUsed= %d\n",gGPSSatellitesUsed);



    return 1;
#else

    return 0;

#endif

}

u8 UARTDoGPSCommand_GLL(char *UartGPSCmdString)
{
#if (GPS_PROCESS_GLL == 1)
#else
#endif
    return 0;

}

u8 UARTDoGPSCommand_GSA(char *UartGPSCmdString)
{
#if (GPS_PROCESS_GSA == 1)
#else
#endif
    return 0;

}

u8 UARTDoGPSCommand_GSV(char *UartGPSCmdString)
{
#if (GPS_PROCESS_GSV == 1)
#else
#endif
    return 0;

}

u8 UARTDoGPSCommand_RMC(char *UartGPSCmdString)
{
#if (GPS_PROCESS_RMC == 1)
    long dwvalue, remain;
    int wvalue1, valid=1,len, type;
    char *ptr,ch;
    s32 ret;
    static u32 n_LocalTimeInSec=0;

    //RTC_DATE_TIME   gmtTime;

#if (GPS_PROCESS_VTG != 1)
    u8 err;
    long speedi,speedf;
#endif

    ptr = UartGPSCmdString;
    gGPS_data.Field = 0;
    gGPS_data.valid = 0;

    //Get UTC time
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
        gGPS_data.Hour = (dwvalue/10000) & 0xff;
        remain=dwvalue%10000;
        gGPS_data.Min= (remain/100) & 0xff;
        remain=remain%100;
        gGPS_data.Sec= remain & 0xff;
        gGPS_data.Field |= GPS_FIELD_TIME;
    }
    //DEBUG_UART("Time %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Time : %d:%d:%d \n", gGPS_data.Hour, gGPS_data.Min, gGPS_data.Sec);
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get Status
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    if (ch == 'A')
    {
        //DEBUG_UART("A");
        gGPS_data.valid = 1;
    }
    else if(ch == 'V')
    {
        //DEBUG_UART("V");
        gGPS_data.valid = 0;
    }
    else
    {
        //DEBUG_UART("Status Error %c\n", ch);
        gGPS_data.valid = 0;
    }

    //DEBUG_UART("Status : %d \n", gGPS_data.valid);
    ptr += len;

    //Get Latitude
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
        gGPS_data.Lat_I=dwvalue;
        gGPS_data.Lat_F=wvalue1;
        gGPS_data.Field |= GPS_FIELD_LATITUDE;
    }
    //DEBUG_UART("Latitude %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Latitude : %d.%d \n", gGPS_data.Lat_I, gGPS_data.Lat_F);
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get N_S
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    if (ch == 'N')
        gGPS_data.N_S= 'N';
    else if(ch == 'S')
        gGPS_data.N_S= 'S';
    else
    {
        //DEBUG_UART("N_S Error %c\n", ch);
        gGPS_data.N_S= '-';
    }

    //DEBUG_UART("N_S: %c \n", gGPS_data.N_S);
    ptr += len;

    //Get Longitude
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
        gGPS_data.Lon_I=dwvalue;
        gGPS_data.Lon_F=wvalue1;
        gGPS_data.Field |= GPS_FIELD_LONGITUDE;
    }
    //DEBUG_UART("Longitude %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Longitude : %d.%d \n", gGPS_data.Lon_I, gGPS_data.Lon_F);
    ptr += len;

    //DEBUG_UART("%s\n", ptr);
    //Get E_W
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    if (ch == 'E')
        gGPS_data.E_W= 'E';
    else if(ch == 'W')
        gGPS_data.E_W= 'W';
    else
    {
        //DEBUG_UART("E_W Error %c\n", ch);
        gGPS_data.E_W= '-';
    }

    //DEBUG_UART("E_W: %c \n", gGPS_data.E_W);
    ptr += len;

    //Get Speed
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {

#if (GPS_PROCESS_VTG != 1)
      speedi = ((185200*dwvalue)+(18520*(long)wvalue1));
      speedf =  (speedi%100000)/10000;
      speedi =  (speedi/100000);
      gGPS_data.Speed_I=speedi;
      gGPS_data.Speed_F=speedf;
      gGPS_data.Field |= GPS_FIELD_SPEED;
      //DEBUG_UART("Speed : %d.%d \n", gGPS_data.Speed_I, gGPS_data.Speed_F);
#endif
    }
    //DEBUG_UART("Speed %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    ptr += len;

    //Get Direction
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
#if (GPS_PROCESS_VTG == 1)
        valid = 0;
#endif
        len = 1;
    }
    if( ret == -2)
    {
#if (GPS_PROCESS_VTG == 1)
        valid = 0;
#endif
        len += 1;
    }
    else
    {
        gGPS_data.Dir_I=dwvalue;
        gGPS_data.Dir_F=wvalue1;
        gGPS_data.Field |= GPS_FIELD_DIRECTION;
    }
    //DEBUG_UART("Direction %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Direction : %d.%d \n", gGPS_data.Dir_I, gGPS_data.Dir_F);
    ptr += len;

    //Get Date
    dwvalue = UARTDoGPSCommand_GetField(ptr, &len, &type);

    if( len == 0)
    {
        valid = 0;
    }
    else
    {
        gGPS_data.Day= (dwvalue/10000) & 0xff;
        remain=dwvalue%10000;
        gGPS_data.Month= (remain/100) & 0xff;
        remain=remain%100;
        gGPS_data.Year= remain & 0xff;
        gGPS_data.Field |= GPS_FIELD_DATE;
    }
    //DEBUG_UART("Date %s: %d,%d \n", ptr, len, dwvalue);
    //DEBUG_UART("Date : %d:%d:%d \n", gGPS_data.Year, gGPS_data.Month, gGPS_data.Day);

    ptr += (len+1);


    if ( (gGPS_data.valid == 1) && (valid == 1) )
    {

        if ((g_LocalTimeInSec - n_LocalTimeInSec) >= 60)
        {
            //DEBUG_UART("time : %x:%x \n", g_LocalTimeInSec, n_LocalTimeInSec);
            //n_LocalTimeInSec = g_LocalTimeInSec;
            RTC_Set_GMT_Time( (RTC_DATE_TIME *)&gGPS_data);
            RTCTime_Gmt_To_Local((RTC_DATE_TIME *)&gGPS_data, &g_LocalTime);
            n_LocalTimeInSec = g_LocalTimeInSec;

        }

#if (GPS_PROCESS_VTG != 1)
           OSSemPend(GPSUpdateEvt, 5, &err);
           gGPS_data.Sec_pre=10;
           memcpy(&gGPS_data1, &gGPS_data, sizeof(GPS_DATA));
           OSSemPost(GPSUpdateEvt);
           gGPS_data.valid = 0;
           //DEBUG_UART("G");
#endif //(GPS_PROCESS_VTG != 1)

   }

    return 1;
#else
    return 0;
#endif

}

u8 UARTDoGPSCommand_VTG(char *UartGPSCmdString)
{
#if (GPS_PROCESS_VTG == 1)
    long dwvalue;
    int wvalue1, valid=1,len;
    char *ptr,ch;
    s32 ret;
    u8 err;

    //DEBUG_UART("1");
    ptr = UartGPSCmdString;

    //Get Direction
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    else
    {
        //gGPS_data.Dir_I=dwvalue;
        //gGPS_data.Dir_F=wvalue1;

    }
    //DEBUG_UART("Direction: %d.%d \n", dwvalue, wvalue1);
    //DEBUG_UART("Direction : %d.%d \n", gGPS_data.Dir_I, gGPS_data.Dir_F);
    ptr += len;

    if (valid==0)
    {
        //DEBUG_UART("2");
        return 0;
    }
    //Get Reference
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    //DEBUG_UART("Reference : %c \n", ch);
    ptr += len;

    //Get Direction
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        len = 1;
    }
    if( ret == -2)
    {
        len += 1;
    }
    //DEBUG_UART("Direction: %d.%d \n", dwvalue, wvalue1);
    ptr += len;

    //Get Reference
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    //DEBUG_UART("Reference: %c \n", ch);
    ptr += len;

    //Get Speed
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        valid = 0;
        len += 1;
    }
    //DEBUG_UART("Speed : %d.%d \n", dwvalue, wvalue1);
    ptr += len;

    //Get unit
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    //DEBUG_UART("unit : %c \n", ch);
    ptr += len;

    //Get Speed
    ret = UARTDoGPSCommand_GetFloat(ptr, &len, &dwvalue, &wvalue1);

    if( ret == -1)
    {
        //DEBUG_UART("3");
        valid = 0;
        len = 1;
    }
    if( ret == -2)
    {
        //DEBUG_UART("4");
        valid = 0;
        len += 1;
    }
    else
    {
#if (GPS_PROCESS_VTG == 1)
        gGPS_data.Speed_I=dwvalue;
        gGPS_data.Speed_F=wvalue1;
        gGPS_data.Field |= GPS_FIELD_SPEED;
#endif
    }
    //DEBUG_UART("Speed %s: %d,%d,%d \n", ptr, len, dwvalue, wvalue1);
    //DEBUG_UART("Speed : %d.%d \n", gGPS_data.Speed_I, gGPS_data.Speed_F);
    ptr += len;

    //Get unit
    ch = UARTDoGPSCommand_GetChar(ptr, &len);
    //DEBUG_UART("unit : %c \n", ch);
    ptr += len;


    if ( (gGPS_data.valid == 1) && (valid == 1) )
    {
        //DEBUG_UART("5");
        //DEBUG_UART("GPS Data Update! \n");
        OSSemPend(GPSUpdateEvt, 5, &err);
        gGPS_data.Sec_pre=10;
        memcpy(&gGPS_data1, &gGPS_data, sizeof(GPS_DATA));
        OSSemPost(GPSUpdateEvt);
        gGPS_data.valid = 0;
        DEBUG_UART("G");
    }
    else
#endif

    return 1;
}


void UARTDoGPSComand(char *UartGPSCmdString)
{

    //DEBUG_UART("%d,%s\n",strlen(UartGPSCmdString), UartGPSCmdString);

    if (UARTDoGPSComand_Checksum(UartGPSCmdString)==0)
    {
        DEBUG_UART("GPS CheckSum Error\n");
        gGPSPKErrorCount++;

        if(gGPSPKErrorCount >= MAXGPSPKERRORCOUNT)
        {
            uart_state[UART_1_ID]=0;
            uartRcvTail[UART_1_ID]=uartRcvHead[UART_1_ID]=0;
            gGPSPKErrorCount=0;
            gGPS_data.State = 0;
            uart_state[UART_1_ID]=1;
        }
        return;
    }
    else
        gGPSPKErrorCount=0;
    //gGPScount++;

    if(!strncmp(UartGPSCmdString,"GPGGA,",6))
        UARTDoGPSCommand_GGA(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"GPGLL,",6))
        UARTDoGPSCommand_GLL(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"GPGSA,",6))
        UARTDoGPSCommand_GSA(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"GPGSV,",6))
        UARTDoGPSCommand_GSV(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"GPRMC,",6))
        UARTDoGPSCommand_RMC(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"GPVTG,",6))
        UARTDoGPSCommand_VTG(UartGPSCmdString+6);
    else if(!strncmp(UartGPSCmdString,"PSRFTXT,",8))
        DEBUG_UART("Text:%s\n", UartGPSCmdString);
    else
        DEBUG_UART("Non-support command: %s\n", UartGPSCmdString);

}

#endif // UART_GPS_COMMAND

void UartReadReg(char *UartCmdString)
{

int val;
u32 u32addr;

    val = sscanf(UartCmdString, "%x", &u32addr);
    //DEBUG_UART("read Reg:(%d)\n",val);
    if(val)
        printf("read Adress: 0x%08X = 0x%08x\n", u32addr, *((volatile unsigned *)(u32addr)));


}

void UartWriteReg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32val;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32val);
    if(val==2)
    {
        DEBUG_UART("Write Address 0x%x = 0x%x\n", u32addr, u32val);
        *((volatile unsigned *)(u32addr)) = u32val;
    }

}

void UartReadI2CReg(char *UartCmdString)
{

    int val;
    u32 u32addr;
    u16 u16value;

    val = sscanf(UartCmdString, "%x", &u32addr);
    //DEBUG_UART("read Reg:(%d)\n",val);
    if(val)
    {
#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        i2cRead_ALC5621(u32addr, &u16value);
#endif
        printf("read ALC5621 Adress: 0x%04X = 0x%04x\n", u32addr, u16value);
    }

}

void UartWriteI2CReg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32val;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32val);
    if(val==2)
    {
        DEBUG_UART("Write ALC5621 Address 0x%x = 0x%x\n", u32addr, u32val);
#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        i2cWrite_ALC5621(u32addr, u32val);
#endif
    }

}


void UartDumpI2CReg(void)
{

    int i;
    u16 u16value;
	u8 	u8value;
#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    DEBUG_UART("\nDump ALC5621 Reg from 0x00-0x68\n");
    for( i=0 ; i<=0x68 ; i+=2)
    {
        if ((i%8)==0)
            DEBUG_UART("\n0x%02X : ",i);
        i2cRead_ALC5621(i, &u16value);
        DEBUG_UART("%04X ", u16value);
    }
#elif(TV_DECODER == WT8861)
	DEBUG_UART("\nDump WT8861 Reg from 0x00-0xFF\n");
    for( i=0 ; i<=0xFF ; i++)
    {
        if ((i%8)==0)
		DEBUG_UART("\n0x%02X : ",i);
		i2cRead_WT8861(i,&u8value,I2C_WT8861_RD_SLAV_ADDR);
        DEBUG_UART("%04X ", u8value);
    }
#elif(TV_DECODER == TW9900)
	DEBUG_UART("\nDump TW9900 Reg from 0x00-0xFF\n");
    for( i=0 ; i<=0xFF ; i++)
    {
        if ((i%8)==0)
		DEBUG_UART("\n0x%02X : ",i);
		i2cRead_TW9900(i,&u8value,I2C_TW9900_RD_SLAV_ADDR);
        DEBUG_UART("%04X ", u8value);
    }
#elif(TV_DECODER == TW9910)
	DEBUG_UART("\nDump TW9910 Reg from 0x00-0xFF\n");
    for( i=0 ; i<=0xFF ; i++)
    {
        if ((i%8)==0)
		DEBUG_UART("\n0x%02X : ",i);
		i2cRead_TW9910(i,&u8value,I2C_TW9910_RD_SLAV_ADDR);
        DEBUG_UART("%04X ", u8value);
    }
#endif
#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    DEBUG_UART("\nDump ALC5621 index from 0x00-0x68\n");
    for( i=0 ; i<=0x46 ; i++)
    {
        if ((i%8)==0)
            DEBUG_UART("\n0x%02X : ",i);


        i2cWrite_ALC5621(0x6A, i);
        i2cRead_ALC5621(0x6C, &u16value);

        DEBUG_UART("%04X ", u16value);
    }
#endif	
    DEBUG_UART("\n");

}

#if(TV_DECODER == TI5150)
void UartReadTVP5150Reg(char *UartCmdString)
{

    int val;
    u32 u32addr;
    u8 u8value;

    if(!strncmp((char*)UartCmdString,"DUMP", strlen("DUMP")))
    {
        DEBUG_UI("Read TVP5150_1 Adress: \n");
        for(u32addr=0;u32addr<256;u32addr++)
        {
            i2cRead_TVP5150(u32addr, &u8value,I2C_TVP5150_RD_SLAV_ADDR_1);
            DEBUG_UI("  0x%04x = 0x%04x \n",u32addr,u8value);
            
        }
        DEBUG_UI("/************************************\ \n");
        DEBUG_UI("Read TVP5150_2 Adress: \n");
        for(u32addr=0;u32addr<256;u32addr++)
        {
            i2cRead_TVP5150(u32addr, &u8value,I2C_TVP5150_RD_SLAV_ADDR_2);
            DEBUG_UI("  0x%04x = 0x%04x \n",u32addr,u8value);
            
        }
    }
    else
    {
        val = sscanf(UartCmdString, "%x", &u32addr);
        //DEBUG_UART("read Reg:(%d)\n",val);
        if(val)
        {
            i2cRead_TVP5150(u32addr, &u8value,TV_CHECK_FORMAT_RD_ADDR);
            printf("read TVP5150 Adress: 0x%04X = 0x%04x\n", u32addr, u8value);
        }
    }
    

}

void UartWriteTVP5150Reg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32val;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32val);
    if(val==2)
    {
        DEBUG_UART("Write TVP5150 Address 0x%x = 0x%x\n", u32addr, u32val);
        i2cWrite_TVP5150(u32addr, u32val,TV_CHECK_FORMAT_WR_ADDR);
    }

}
#endif

void UartReadSensorReg(char *UartCmdString)
{

    int val;
    u32 u32addr;
    u16 u16value;
    
    if(!strncmp((char*)UartCmdString,"DUMP", strlen("DUMP")))
    {
        DEBUG_UI("Read Sensor: \n");
        for(u32addr=0;u32addr<256;u32addr++)
        {
            i2cRead_SENSOR(u32addr, &u16value);
            DEBUG_UI("  0x%04x = 0x%04x \n",u32addr,u16value);
            
        }
    }
    else
    {
        val = sscanf(UartCmdString, "%x", &u32addr);
        //DEBUG_UART("read Reg:(%d)\n",val);
        if(val)
        {
            i2cRead_SENSOR(u32addr, &u16value);
            printf("read Sensor Adress: 0x%04X = 0x%04x\n", u32addr, u16value);
        }    
    }
    

}

void UartReadSensorALLReg(void)
{
    int val;
    u32 u32addr, u32size;
    int i;
    u16 u16value;
    u32addr=0;
    u32size=0x100;

    printf("Dump Sensor Adress 0x%x ,0xSize= %x\n", u32addr, u32size);

    for( i=0 ; i<u32size ; i++)
    {
        if ((i%8)==0)
            printf("\n0x%02X : ",i);
        i2cRead_SENSOR(i, &u16value);
        printf("%02X ", u16value);
    }

    printf("\n");



}

void UartWriteSensorReg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32val;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32val);
    if(val==2)
    {
        DEBUG_UART("Write Sensor Address 0x%x = 0x%x\n", u32addr, u32val);
        i2cWrite_SENSOR(u32addr, u32val);
    }

}

void UartDumpReg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32size;
    int i;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32size);
    if(val==2)
    {
        printf("Dump Adress 0x%x ,0xSize= %x\n", u32addr, u32size);

        if (u32size == 0) return;

        if (u32size%4)
            u32size = u32size/4 + 1;
        else
            u32size = u32size/4;

        for( i=0 ; i<u32size ; i++)
        {
            if ((i%4)==0)
                printf("\n0x%08X : ",u32addr);
            printf("%08X ", *((volatile unsigned *)(u32addr)));
            u32addr += 4;
        }

        printf("\n");
    }

}

void UartFillReg(char *UartCmdString)
{

    int val;
    u32 u32addr, u32size, u32pattern;
    int i;

    val = sscanf(UartCmdString, "%x %x %x", &u32addr, &u32size, &u32pattern);
    if(val==3)
    {
        DEBUG_UART("Fill Address 0x%x ,Size= 0x%x, Patten= 0x%x\n", u32addr, u32size, u32pattern);

        if (u32size == 0) return;

        if (u32size%4)
            u32size = u32size/4 + 1;
        else
            u32size = u32size/4;

        for( i=0 ; i<u32size ; i++)
        {
            *((volatile unsigned *)(u32addr))= u32pattern;
            u32addr += 4;
        }
    }

}

void UartCmdParse(char *UartCmdString)
{
    u8 err;

    if (*UartCmdString < 0x20)
        return;
         //-------------Function Key-------------//

    //printf("%X, %X, %s \n", uartRcvHead[UART_1_ID], uartRcvTail[UART_1_ID], UartCmdString);
    switch(*UartCmdString)
    {
       //--------GPS command--------//
#if UART_GPS_COMMAND
        case '$':
            UARTDoGPSComand(UartCmdString+1);
            break;
#endif
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
        case '@':
            printf("net-");
            if(!strncmp(UartCmdString,"@SEND", 5))
                {
                 printf("SEND\n");
                 sendrawdata();
                }

            else if(!strncmp(UartCmdString,"@10MHD",6))
                {
                printf("10MHD\n");
                set_PHY_mode(DM9KS_10MHD);
                }
            else if(!strncmp(UartCmdString,"@100MHD",7))
                {
                printf("100MHD\n");
                set_PHY_mode(DM9KS_100MHD);
                }
            else if(!strncmp(UartCmdString,"@10MFD",6))
                {
                printf("10MFD\n");
                set_PHY_mode(DM9KS_10MFD);
                }
            else if(!strncmp(UartCmdString,"@100MFD",7))
                {
                printf("100MFD\n");
                set_PHY_mode(DM9KS_100MFD);
                }
            else if(!strncmp(UartCmdString,"@AUTOMFD",8))
                {
                printf("AUTOMFD\n");
                set_PHY_mode(DM9KS_AUTOMF);
                }
            else if(!strncmp(UartCmdString,"@PHYREG",7))
                {
                printf("DM9000 PHY REGs\n");
                dump_dm9000_phy_regs();
                }
            else if(!strncmp(UartCmdString,"@9000REG",8))
                {
                printf("DM9000 control REGs\n");
                dump_dm9000_control_regs();
                }
            else if(!strncmp(UartCmdString,"@AUTO",5))
                {
                printf("AUTO\n");
                set_PHY_mode(DM9KS_AUTO);
                }
			else if(!strncmp(UartCmdString,"@VID",4))
				{
				printf("Read DM9000's VID.\n");
				dm9000_probe();
				}
            else if(!strncmp(UartCmdString,"@WD ",3))
                {
                printf("Write DM9000.\n");
                 UartWriteDM9000Reg(UartCmdString+3);
                }
            else if(!strncmp(UartCmdString,"@RD ",3))
                {
                 printf("Read DM9000.\n");
                 UartReadM9000Reg(UartCmdString+3);
                }
            #if TUTK_SUPPORT
            else if(!strncmp(UartCmdString,"@P2PINFO",8))
                {
                    int info;
                    Check_P2P_info(&info);
                    printf("p2p server:%d.\n",info);
                }
            else if(!strncmp(UartCmdString,"@NTP",4))
                {
                    NTP_Switch(NTP_SWITCH_ON);
                    ntpdate();
                    printf("NTP update.\n");
                }
            else if(!strncmp(UartCmdString,"@MSG1",5))
                {
                    sysback_Net_SetEvt(0, 0, 0);
                    printf("Send MSG1.\n");
                }
             else if(!strncmp(UartCmdString,"@MSG2",5))
                {
                    sysback_Net_SetEvt(0, 0, 1);
                    printf("Send MSG2.\n");
                }
            #endif
            break;
#endif
       //--------UART command set---------//

        //-----Dump data from Register/memory----//
        case 'D':
            if(!strncmp(UartCmdString,"DUMP ", 5))
                UartDumpReg(UartCmdString+5);
            else if(!strncmp(UartCmdString,"DUMPI", 5))
                UartDumpI2CReg();
            break;

        //----Fill data to register/memory-----//
        case 'F':
            if(!strncmp(UartCmdString,"FILL ", 5))
                UartFillReg(UartCmdString+5);
            break;

        case 'R':
        //-----Read register/SDRAM-----//
            if(!strncmp(UartCmdString,"R ", 2))
                UartReadReg(UartCmdString+2);
            else if(!strncmp(UartCmdString,"RS ",3))
                UartReadSensorReg(UartCmdString+3);
            else if(!strncmp(UartCmdString,"RSALL",5))
                UartReadSensorALLReg();
            else if(!strncmp(UartCmdString,"RI ",3))
                UartReadI2CReg(UartCmdString+3);
        #if(TV_DECODER ==TI5150)
            else if(!strncmp(UartCmdString,"RV ",3))
                UartReadTVP5150Reg(UartCmdString+3);
        #endif
            break;

     //------------Write Register/SDRAM command----------//
     //Ex: w 80000000 12345678
        case 'W':
            if(!strncmp(UartCmdString,"W ", 2))
                UartWriteReg(UartCmdString+2);
            else if(!strncmp(UartCmdString,"WS ",3))
                UartWriteSensorReg(UartCmdString+3);
            else if(!strncmp(UartCmdString,"WI ",3))
                UartWriteI2CReg(UartCmdString+3);

        #if(TV_DECODER == TI5150)
            else if(!strncmp(UartCmdString,"WV ",3))
                UartWriteTVP5150Reg(UartCmdString+3);
        #endif
            break;


        case NEWLINE:
        case ENTER:
        case TERMNULL:

            break;


        default:
            UIKey=UI_KEY_UART;
            DEBUG_UART("UART Send Command %x to UI\n", *UartCmdString);
            break;

    }

#if(UI_VERSION == UI_VERSION_THREE_TASK)
    OSFlagPost(gUiKeyFlagGrp, UIKey, OS_FLAG_SET, &err);
    OSMboxPost(uart_MboxEvt, UartCmdString);
#else
    if(UIKey == UI_KEY_UART)
    {
        OSMboxPost(uart_MboxEvt, UartCmdString);
        OSSemPost(uiSemEvt);
    }
#endif
}

INT32U marsUartGetString(char *pcString, INT32U *pcBufferLen)
{
    INT32U uiBuferLen;

    uiBuferLen = *pcBufferLen;
    // check parameters
    if (!pcString || !uiBuferLen)
    {
        return 0;    // error
    }

    *pcBufferLen = 0;
    while (*pcBufferLen < uiBuferLen)
    {

        *pcString = receive_char(DEBUG_UART_ID);

        if (*pcString == BACKSPACE)
        {
            if (*pcBufferLen)
            {
                (*pcBufferLen)--;
                pcString--;
            }
        }
        else
        {
            if ((*pcString == NEWLINE) || (*pcString == ENTER))
            {
                *pcString = TERMNULL;
                return 1;    // OK
            }
            else
            {
                if ( (*pcString>='a') && (*pcString<='z') )
                    (*pcString) -= 0x20;
                (*pcBufferLen)++;
                pcString++;
            }
        }
    }
    *pcString = TERMNULL;
    return 0;    // error
}

void uartCmdTask(void* pData)
{
#if ICOMMWIFI_SUPPORT
	extern u8	uart_cmd_change_flag;
#endif	
    INT32U  uiUartCmdBufSize;
    u8 err;
    int *UartId = (int*)pData;
    char ch;
    char uartCmd[64];
    int retry;
    int i;
    int Version;


    //===============//
#if UART_GPS_COMMAND
    if(*UartId == UART_1_ID)
    {
        DEBUG_UART("uartCmdTask GPS Start \n");
        GPSUpdateEvt = OSSemCreate(1);
        memset(&gGPS_data2, 0, sizeof(GPS_DATA));
        memset(&gGPS_data1, 0, sizeof(GPS_DATA));
        memset(&gGPS_data, 0, sizeof(GPS_DATA));
//      GPS_Init();
    }
#else
    DEBUG_UART("uartCmdTask commandStart %d\n", *UartId);
#endif

    if(*UartId == DEBUG_UART_ID)
    {
        uart_state[DEBUG_UART_ID]=1;

        while(1)
        {
            uiUartCmdBufSize = UART_CMDBUF_SIZE-1;
            
			#if ICOMMWIFI_SUPPORT
            if(!uart_cmd_change_flag)
            #endif
            {
	            marsUartGetString(UartCommand, &uiUartCmdBufSize);

	            if(UIKey == UI_KEY_READY)
	            {
	                UartCmdParse(UartCommand);
	            }
	        #if (UART_GPS_COMMAND==0)  //GPS command 可漏掉.
	            else
	            {
	                //DEBUG_UART("UART Wait 0, %d\n",UIKey);
	                OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_COMMAND_FINISH, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);  //避免跟UI task 衝突.
	                //DEBUG_UART("UART Wait 1\n");
	                UartCmdParse(UartCommand);
	            }
        #endif
        	}
            OSTimeDly(1);
        }
    }
#if UART_PTZ485_COMMAND_RX
    else if(*UartId == PTS485_UART_ID)
    {
        uart_state[PTS485_UART_ID]=1;
        while(1)
        {
        #if 1
          sprintf((char*)uartCmd,"PTZ485 ");
          for(i=0;i<7;i++)
          {
             ch=receive_char(PTS485_UART_ID);
             uartCmd[7+i]=ch;
             //DEBUG_UART("%c",ch);
             //DEBUG_UART("%d ",(int)ch);
             sendchar(PTS485_UART_ID, &ch);
          }
          //DEBUG_UART("END\n");
        #else
           ch=receive_char(PTS485_UART_ID);
           sprintf((char*)uartCmd,"PTZ485 %d",(int)ch);
           //DEBUG_UART("%c",ch);
           //DEBUG_UART("%d ",(int)ch);
           sendchar(PTS485_UART_ID, &ch);
        #endif

    #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
           retry=8;
           while(retry>0)
           {
              if(0 == rfiu_RXCMD_Enc (uartCmd,0))
                 break;
              retry -=1;
              OSTimeDly(1);
           }

           retry=8;
           while(retry>0)
           {
              if(0 == rfiu_RXCMD_Enc (uartCmd,1))
                 break;
              retry -=1;
              OSTimeDly(1);
           }
    #else
           retry=8;
           while(retry>0)
           {
              if(0 == rfiu_RXCMD_Enc (uartCmd,sysRF_PTZ_CHsel))
                 break;
              retry -=1;
              OSTimeDly(1);
           }
    #endif    
        }
    }
#elif HOME_RF_SUPPORT   
    //Receive data from 868
    else if(*UartId == HOMERF_UART_ID)
    {
        uart_state[HOMERF_UART_ID]=1;
        while(1)
        {
           //Version=marsUartGet868Cmd(Uart868Command);
           homeRFGetUartCmd(UartHomeRFCmd);
           homeRFCmdParse(UartHomeRFCmd);
        //   rfiu_Parse868Cmd(Uart868Command,Version);
                      
           //OSTimeDly(1);
        }
    }
#elif(HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS)
    else if(*UartId == UART_2_ID)
    {
        uart_state[UART_2_ID]=1;
        while(1)
        {

           uiGetUartCmd(UartCmdBuf);
           uiCmdParse(UartCmdBuf);

        }
    }
#elif BLE_SUPPORT
    else if(*UartId == BLE_UART_ID)
    {
        uart_state[BLE_UART_ID] = 1;
        while(1)
        {
           memset(uartCmd, 0, 64);
           BLEGetUartCmd(uartCmd);
        }
    }
#elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    else if(*UartId == RDI_SEP_MCU_UART_ID)
    {
        uart_state[RDI_SEP_MCU_UART_ID] = 1;
        while(1)
        {
           if (rfiuVocDustMode != 0)
           {
               uiGetSEPTESTUartCmd(uartCmd);
               uiParseSEPTESTCmd(uartCmd);
           }
           else
           {
               uiGetSEPUartCmd(uartCmd);
               uiParseSEPCmd(uartCmd);
           }
        }
    }
#endif  
    else if(*UartId < UART_MAX_NUM)
    {
        if(UartUseTable[*UartId] == UART_USE_ENABLE)
        {   
            while(1)
            {
               OSFlagPost(gUARTFlagGrp, UartFlagRxReady[*UartId], OS_FLAG_CLR, &err);
               OSFlagPend(gUARTFlagGrp, UartFlagRxReady[*UartId], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            }
        }
    }
}

#endif  // #if UART_COMMAND

