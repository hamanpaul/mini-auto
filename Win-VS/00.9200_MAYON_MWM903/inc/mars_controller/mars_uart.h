

#ifndef __MARS_UART_H__
#define __MARS_UART_H__

#include <osapi.h>

/* UartIer_1 */
#define UART_IER_MASK	                 0x0000000F
#define UART_IER_RX_DATA_INT_ENA         0x00000001
#define UART_IER_THR_EMPTY_INT_ENA       0x00000002
#define UART_IER_RX_LINE_STATUS_INT_ENA  0x00000004
#define UART_IER_MODEM_STATUS_INT_ENA    0x00000008


typedef struct _UART_CFG{
    INT32U  IntEnable;
    INT32U  BaudRate;
    //INT8U   UartId;
    //INT8U   IfHWCtrlmode;
    //INT8U   reserved[2];
}UART_CFG;


//=================================================================
extern void marsUartInit(void);
extern INT32U marsUartConfig(UART_CFG* pCfg);
extern void marsUartSend(INT8U *pdata);
extern INT8U marsUartRecv(void);
extern INT32U marsUartGetString(char *pcString, INT32U *pcBufferLen);
extern INT32U marsUartPutString(char *pString);
//=================================================================

#endif    // __MARS_UART_H__
