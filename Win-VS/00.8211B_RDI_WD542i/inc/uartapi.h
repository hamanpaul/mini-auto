/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	uartapi.h

Abstract:

   	The application interface of the UART controller.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __UART_API_H__
#define __UART_API_H__

#include "task.h"



enum
{
    UART_1_ID = 0,
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    UART_2_ID,
    UART_3_ID,
#endif
    UART_MAX_NUM
};

#define DEBUG_UART_ID               UART_1_ID
#define PTS485_UART_ID              UART_2_ID
#define HOMERF_UART_ID              UART_2_ID
#define RDI_MCU_UART_ID             UART_2_ID
#define RDI_SEP_MCU_UART_ID         UART_2_ID
#define BLE_UART_ID                 UART_2_ID
#define GPS_UART_ID                 UART_3_ID

//================================================================//
extern void sendchar(u8 Uart_ID, unsigned char *ch);
extern void sendData(u8 Uart_ID, unsigned char *ch);
extern char receive_char(u8 Uart_ID);
extern void uartIntHandler(void);
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
extern void uart2IntHandler(void);
extern void uart3IntHandler(void);
#endif
void uartTest(void);
extern u32 sys_frequency;

#if UART_COMMAND
void uartCmdInit(void);
void UartCmdParse(char *UartCmdString);
void uartCmdTask(void *pdata);

extern OS_STK  uartCmdTaskStack[UARTCMD_TASK_STACK_SIZE];
extern OS_EVENT* uart_MboxEvt;
#endif
extern void init_serial_A(void);

#endif
