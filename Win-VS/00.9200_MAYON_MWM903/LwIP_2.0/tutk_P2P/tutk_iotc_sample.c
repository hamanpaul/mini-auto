#include <stdio.h>
#include "general.h"
#include "task.h"
#include "gpiapi.h"
//#include "uartreg.h"
//#include "a1019.h"
//#include "DRAM_Buffer_layout.h"
#include "ucos_ii.h"
#include "IOTCAPIs.h"
#include "lwip\Ip_addr.h"
#include "lwip\Def.h"


#define TUTK_INIT_TASK_STK_SIZE     (1024*32)
#define TUTK_TRANSMISSION_TASK_STK  (1024*64)

OS_STK TutkTransmissionTaskStkSpace[TUTK_TRANSMISSION_TASK_STK/sizeof(OS_STK)];
OS_STK TutkInitTaskStkSpace[TUTK_INIT_TASK_STK_SIZE/sizeof(OS_STK)];
/************************************/
/*        TUTK IOTC TASK            */
/************************************/
OS_STK *TutkInitTaskStk = 0;
INT8U TutkInitTaskPri;

OS_STK *TutkTransmissionTaskStk = 0;
INT8U TutkTransmissionTaskPri;

void tutk_task_init(void)
{
//    rt_init_mem((void *)TUTK_LOCAL_BUF, TUTK_LOCAL_BUF_SIZE);
    memset(TutkInitTaskStkSpace, 0, sizeof(TutkInitTaskStkSpace));
    TutkInitTaskStk = &TutkInitTaskStkSpace[TUTK_INIT_TASK_STK_SIZE/sizeof(OS_STK)-1];
	TutkInitTaskPri = IOTC_INIT_TASK_PRIORITY;
    //printf("\033[31m...TutkInitTaskStk=0x%x TutkInitTaskPri=%d \033[0m\n", TutkInitTaskStk, TutkInitTaskPri);

    memset(TutkTransmissionTaskStkSpace, 0, sizeof(TutkTransmissionTaskStkSpace));
    TutkTransmissionTaskStk = &TutkTransmissionTaskStkSpace[TUTK_TRANSMISSION_TASK_STK/sizeof(OS_STK)-1];
	TutkTransmissionTaskPri = IOTC_TRANSMISSION1_TASK_PRIORITY;
    //printf("\033[31m...TutkTransmissionTaskStk=0x%x TutkTransmissionTaskPri=%d \033[0m\n", TutkTransmissionTaskStk, TutkTransmissionTaskPri);
}
/************************************/
/*        TUTK IOTC TASK END        */
/************************************/
