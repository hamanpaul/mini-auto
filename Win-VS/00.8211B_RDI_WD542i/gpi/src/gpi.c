/*

Copyright (c) 2010  Himax Technologies, Inc.

Module Name:

    gpi.c

Abstract:

    The routines of GPI.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/08/02  Raymond Creates


*/

#include "general.h"
#include "board.h"
#include "intapi.h"
#include "task.h"
#include "sysapi.h"
#include "gpi.h"
#include "gpireg.h"
#include "gpi_dm9000b.h"
#include <../inc/mars_controller/mars_dma.h>


#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
OS_EVENT *gpiSemDMAComplete;        /* semaphore to synchronize DMA complete  */
OS_EVENT *gpiSemRX_VALID;      /* Semaphore to synchronize RX_VALID interrupt. */
OS_EVENT *gpiSemTX_REQ;      /* Semaphore to synchronize RX_VALID interrupt. */
OS_FLAG_GRP  *gpiNetStatusFlagGrp;    /* Flag for network Status. */
OS_STK nicTaskStack[NIC_TASK_STACK_SIZE]; /* Stack of task ciuTask() */
u8 gpiXferComplete;   /* Interrupt status for GPI transfer finished */
u8 gpiRX_VALID; /* Interrupt status for data available input */
u8 gpiTX_REQ;/* Interrupt status for output fifo request data */
u8  outbuf2[1600];
u8  inbuf2[1600];
u32 guiGPIReadDMAId=0xFF, guiGPIWriteDMAId=0xFF;
extern void dm9000_reset(void);
extern void DM9000_iow(int,u8);
extern u32 DM9000_ior(int);
extern int dm9000_probe(void);
extern void netSystemInitialization(void);
extern void dhcpc_init(u8*,u8);
extern void dhcp();
u8 SetLwIP(u8 mode);
extern u8 *my_hwaddr;
/*********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
*/
/*

Routine Description:
    Suspend NIC task.
Arguments:
    None.
Return Value:
    0 - Failure.
    1 - Success.
*/
s32 NicSuspendTask(void)
{
    /* Suspend the task */

    OSTaskSuspend(NIC_TASK_PRIORITY);
    
    return 1;
}

#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
s32 NicCloseAllTask(void)
{
    OSTaskDel(T_LWIP_THREAD_START_PRIO);
    OSTaskDel(T_LWIPENTRY_PRIOR);
    OSTaskDel(T_ETHERNETIF_INPUT_PRIO);
    OSTaskDel(NIC_TASK_PRIORITY);
#if (TUTK_SUPPORT==1)
 //   OSTaskDel(IOTC_LOGIN_TASK_PRIORITY);
 //   OSTaskDel(IOTC_ROUTINE_TASK_PRIORITY);
 //   OSTaskDel(SPEAKER_TASK_PRIORITY);
 //   OSTaskDel(P2P_PLAYFILE_TASK_PRIORITY);
 //   OSTaskDel(LISTEN_TASK_PRIORITY);
 //   OSTaskDel(LOGIN_TASK_PRIORITY);
#endif
    return 1;
}
s32 NicSuspendAllTask(void)
{
   // OSTaskSuspend(T_LWIP_THREAD_START_PRIO);
  //  OSTaskSuspend(T_LWIPENTRY_PRIOR);
    OSTaskSuspend(T_ETHERNETIF_INPUT_PRIO);
  //  OSTaskSuspend(NIC_TASK_PRIORITY);
#if (TUTK_SUPPORT==1)
//    OSTaskSuspend(IOTC_LOGIN_TASK_PRIORITY);
//    OSTaskSuspend(IOTC_ROUTINE_TASK_PRIORITY);
//    OSTaskSuspend(SPEAKER_TASK_PRIORITY);
//    OSTaskSuspend(P2P_PLAYFILE_TASK_PRIORITY);
//    OSTaskSuspend(LISTEN_TASK_PRIORITY);
//    OSTaskSuspend(LOGIN_TASK_PRIORITY);
#endif
    return 1;
}

s32 NicResumeAllTask(void)
{
//    OSTaskResume(T_LWIP_THREAD_START_PRIO);
//    OSTaskResume(T_LWIPENTRY_PRIOR);
      OSTaskResume(T_ETHERNETIF_INPUT_PRIO);
//    OSTaskResume(NIC_TASK_PRIORITY);
#if (TUTK_SUPPORT==1)
//    OSTaskResume(IOTC_LOGIN_TASK_PRIORITY);
//    OSTaskResume(IOTC_ROUTINE_TASK_PRIORITY);
//    OSTaskResume(SPEAKER_TASK_PRIORITY);
//    OSTaskResume(P2P_PLAYFILE_TASK_PRIORITY);
//    OSTaskResume(LISTEN_TASK_PRIORITY);
//    OSTaskResume(LOGIN_TASK_PRIORITY);
#endif
    return 1;
}
#endif



/*

Routine Description:

    Resume NIC Playback task.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 NicResumeTask(void)
{
    /* Resume the task */
    OSTaskResume(NIC_TASK_PRIORITY);
    
    return 1;
}
extern void marsIntIRQDisable(INT32U intno);
/*
Routine Description:

    Initialize System, GPI.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
s32 gpiInit(void)
{
    INT8U err;
    extern void T_LwIPEntry(void*);
//    marsIntIRQDisable(1);
    /* Create the semaphore */
    gpiSemDMAComplete = OSSemCreate(0);    
    gpiSemRX_VALID = OSSemCreate(0);  
    gpiSemTX_REQ = OSSemCreate(0);
    gpiNetStatusFlagGrp = OSFlagCreate(0x00000000, &err);
    //*((volatile unsigned *)(Uart3Ier))=0;  //Uart3Fcr =0;
    
    //*((volatile unsigned *)(Uart2Ier))=0;  //Uart2Fcr =0; ??? 
    // SYS_CTL0 |= SYS_CTL0_GPI_CK_EN;  
    SYSClkEnable(SYS_CTL0_GPIU_CKEN);
    #if((CHIP_OPTION==CHIP_A1016A))
    GpioActFlashSelect &= ~(GPIO_GPIU_FrDISP_EN|GPIO_GPIU2_FrXX_EN|GPIO_DV2_FrGPI_EN2);   
    #endif
	
    #if(HW_BOARD_OPTION == MR8211_ZINWELL)   //DV2 PIN MUX From DISP
    GpioActFlashSelect |= GPIO_DV2FrDISP_EN;
    #endif
   // Gpio1Ena = 0;
   // Gpio1Dir =0xffffffff;
   #if 0
   #if(HW_BOARD_OPTION  == MR8200_RX_DEMO_BOARD)
    gpioSetLevel(2, 4, 1);//Dm9000B rst pin pull hi
    #elif(HW_BOARD_OPTION  == MR8200_RX_RDI)
    gpioSetLevel(2, 0, 1);//Dm9000B rst pin pull hi
    #elif(HW_BOARD_OPTION  == MR8200_RX_ZINWELL)
    gpioSetLevel(0, 10, 1);//Dm9000B rst pin pull hi
    #endif
   #endif 
    gpioSetLevel(GPIO_GROUP_Dm9000B_RST, GPIO_BIT_Dm9000B_RST, 1);//Dm9000B rst pin pull hi
    #if (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA)
    i2cWrite_WT6853(0x01, 0x8);
    #endif
    SYSReset(SYS_RSTCTL_GPI_RST); 
    GPIReset();

    GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_R|GPI_NEG_DUTY_TIM_R;

    GPIIntCtrl = GPI_RX_MSK|GPI_FINISH_MSK|GPI_TX_MSK;//disable interrupt.
    //marsIntIRQEnable(INT_IRQMASK_GPIU);

    #if 1
    OSTaskCreate(T_LwIPEntry, (void*)NULL, NIC_TASK_STACK, T_LWIPENTRY_PRIOR);

    #else
    if (eth_init()==0) return ;
    netSystemInitialization();
    dhcpc_init(my_hwaddr,6);
    dhcp();
//    change_m_addr();
    /* Create the task */
    OSTaskCreate(NIC_TASK, NIC_TASK_PARAMETER, NIC_TASK_STACK, NIC_TASK_PRIORITY); 
//    NicSuspendTask();
    #endif
    return 1;
}
   #endif 
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
void NicOff()
{
   NicSuspendAllTask();
  // NicCloseAllTask();
}
   
void NicON()
{
   EMACInit();
   NicResumeAllTask();
 //  gpiInit();
 //  SetLwIP();
 //   NicCloseAllTask();
}
#endif   
/*
Routine Description:

    The IRQ handler of GPI.

Arguments:

    None.

Return Value:

    None.

*/
void GPIIntHandler(void)
{
    u32 intStat = GPIIntCtrl;

    /* Clear the interrupt flags */
    gpiXferComplete = gpiRX_VALID = gpiTX_REQ = 0;

    if (intStat & GPI_FINISH_INT)
    {
        gpiXferComplete = 1;
        //  DEBUG_GPIU("gpiXferComplete\n");
        /* Signal completion */
        OSSemPost(gpiSemDMAComplete);
        GPIIntCtrl=1;
    }

    if (intStat & GPI_RX_VALID_INT)
    {
        //  DEBUG_GPIU("gpiSemRX_VALID\n");
        gpiRX_VALID = 1;
        /* Signal completion */
        OSSemPost(gpiSemRX_VALID);
        GPIIntCtrl = GPI_RX_MSK;
    }

    if (intStat & GPI_TX_REQ_INT)
    {
        // DEBUG_GPIU("gpiTX_REQ\n");
        gpiTX_REQ = 1;
        /* Signal completion */
        OSSemPost(gpiSemTX_REQ);
        GPIIntCtrl = GPI_TX_MSK;
    }



    /* Signal completion */
}
#if (NIC_SUPPORT == 1)
void GPIReset(void)
{
//    u32 delay=150;
        
    GPICtrlReg = GPICtrlReg | GPI_Reset;
    gpioSetLevel(GPIO_GROUP_Dm9000B_RST, GPIO_BIT_Dm9000B_RST, 0);//Dm9000B rst pin pull Low
    #if (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA)
    i2cWrite_WT6853(0x01, 0x0);
    #endif
//    while (delay != 0)
//        delay--;
    OSTimeDly(1);
    gpioSetLevel(GPIO_GROUP_Dm9000B_RST, GPIO_BIT_Dm9000B_RST, 1);//Dm9000B rst pin pull hi
    #if (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA)
    i2cWrite_WT6853(0x01, 0x8);
    #endif
    GPICtrlReg = GPICtrlReg & (~GPI_Reset);
}

void GPIReset_RDI(void)
{
	
    OSTaskSuspend(T_ETHERNETIF_INPUT_PRIO);
	
    GPICtrlReg = GPICtrlReg | GPI_Reset;
    gpioSetLevel(GPIO_GROUP_Dm9000B_RST, GPIO_BIT_Dm9000B_RST, 0);//Dm9000B rst pin pull Low
    OSTimeDly(1);
    gpioSetLevel(GPIO_GROUP_Dm9000B_RST, GPIO_BIT_Dm9000B_RST, 1);//Dm9000B rst pin pull hi
    GPICtrlReg = GPICtrlReg & (~GPI_Reset);
    
	OSTaskResume(T_ETHERNETIF_INPUT_PRIO);
}

#endif
/*
Routine Description:

    Set read data dma.

Arguments:

    buf - The buffer to read to.
    siz - The size to read.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 GPISetReadDataDma(u32 *buf, u32 siz)
{
    DMA_CFG dmaCfg;
    
    /* Set read data dma */
    dmaCfg.src = (u32)&(GPIInput);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;   

    guiGPIReadDMAId = marsDMAReq(DMA_REQ_GPI_READ, &dmaCfg);
    return 1;
}
/*

Routine Description:

    Set data write dma.

Arguments:

    buf - The buffer to write from.
    siz - The size to write.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 GPISetWriteDataDma(u32 *buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* Set write data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(GPIOutput);
    dmaCfg.cnt = (siz+15) / 16;
    dmaCfg.burst = 1;   /*CY 0907*/
    guiGPIWriteDMAId= marsDMAReq(DMA_REQ_GPI_WRITE, &dmaCfg);
    return 1;
}
/*

Routine Description:

    Check if dma is completed.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 gpiCheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiGPIReadDMAId);
    guiGPIReadDMAId = 0x55;
    return (err);
}

s32 gpiCheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiGPIWriteDMAId);
    guiGPIWriteDMAId = 0x55;
    return (err);
}  
u32 ctol(u8 * buf)
{
    u32 value;
    
    value=0xff & buf[3];
    value=value<<8;
    value=value | buf[2];
    value=value<<8;
    value=value | buf[1];
    value=value<<8;
    value=value | buf[0];

    return value;
    
}
void ltoc(u8 * buf,u32 value)        
{

    buf[0]= 0xff & value;
    buf[1]= (value>>8) &0xff ;
    buf[2]= (value>>16) &0xff  ;
    buf[3]= (value>>24) &0xff ;
    
}
/*

Routine Description:

    Read Data 

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
void GPIToRead(u8 CMDS, u32 CMD, u32 GPI_LENGTH, u32 *dataBuf )
{
    u32 i,temp_dat;
    u8 ucErr,temp_len;
    u32 cnt;
    if(CMDS==1)
    {

        GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_R|GPI_NEG_DUTY_TIM_R|GPI_CHIP_CMD_IOR;
        temp_dat = (GPI_LENGTH << 16)|1;
        GPILength = temp_dat;
        //DEBUG_GPIU("Read CMD %x GPI_LENGTH %x\n",CMD,temp_dat);
    }
    else 
    {
        if(GPI_LENGTH <5)
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_R|GPI_NEG_DUTY_TIM_R|GPI_CHIP_CMD_IOR;
            GPILength = (GPI_LENGTH << 16);
        }
        else// just Read DMA data
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_R|GPI_NEG_DUTY_TIM_R|GPI_DMA_MODE|GPI_CHIP_CMD_IOR;
            GPILength = (GPI_LENGTH << 16);
        }
    }
    GPICtrlReg = GPICtrlReg | GPI_START_TRIG; 
    if(CMDS==1)
    {
     //   DEBUG_GPIU("send cmd");
#if 1 //polling     
       	cnt=20;
	while (cnt--)/* wait for tx complete */
        {
            if (GPIIntCtrl& GPI_TX_REQ_INT)
            break;
        }
        if(cnt == 0)
            printf("gpi polling 1 error");
#else        
        OSSemPend(gpiSemTX_REQ, 100, &ucErr);
#endif
        GPIOutput = CMD;    
    }

#if 1 //polling     
       	cnt=20;
   while (cnt--)/* wait for tx complete */
    {
        if (GPIIntCtrl& GPI_RX_VALID_INT)
        break;
    }
        if(cnt == 0)
            printf("gpi polling 2 error");
#else   
    OSSemPend(gpiSemRX_VALID, 100, &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_GPIU("Error: GpiSemTX_REQ is %d.\n", ucErr);
    }
#endif   

    if(GPI_LENGTH <5)
    {
        *dataBuf = GPIInput;
        // DEBUG_GPIU("Read CMD %x =%x\n",CMD,*dataBuf);
    }
    else
    {
        //DEBUG_GPIU("DMA Read Operation\n ");
        GPISetReadDataDma(dataBuf,GPI_LENGTH);

        /* check dma complete */
        if (gpiCheckDmaReadComplete() != 1)
            DEBUG_GPIU("DMA operation fails\n");
    }
   
}

void GPIReadData(u32 GPI_LENGTH, u32 *dataBuf )
{
    u32 i;
    u8 ucErr,temp_len;

    GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_R|GPI_NEG_DUTY_TIM_R|GPI_DMA_MODE
                |GPI_CHIP_CMD_IOR;

    GPILength = (GPI_LENGTH << 16);
    GPICtrlReg = GPICtrlReg | GPI_START_TRIG;
    OSSemPend(gpiSemRX_VALID, 100, &ucErr);
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_GPIU("Error: gpiSemTX_REQ is %d.\n", ucErr);
    }

    //DEBUG_GPIU("DMA Read Operation\n ");
    GPISetReadDataDma(dataBuf,GPI_LENGTH);

    /* check dma complete */
    if (gpiCheckDmaReadComplete() != 1)
        DEBUG_GPIU("GPIReadData:DMA operation fails\n");


}

/*

Routine Description:

    Write Data 

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
void GPIToWrite(u8 CMDS, u32 CMD, u32 GPI_LENGTH, u32 *dataBuf)
{
    u32 i,temp_dat;
    u8 ucErr,temp_len;
    u32 cnt;
    if(CMDS==1)
    {
        GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_W|GPI_NEG_DUTY_TIM_W
        |GPI_CHIP_CMD_IOW_2|GPI_CHIP_CMD_IOW_3|GPI_CHIP_CMD_IOW_4;
        i=0;
        temp_dat = dataBuf[i]<<8|CMD;
        GPILength = GPI_LENGTH+1;
        // DEBUG_GPIU("Write CMD %x \n",temp_dat);
        // DEBUG_GPIU("GPI_LENGTH %x \n",GPI_LENGTH);
    }
    else if(CMDS==0)
    {
        if(GPI_LENGTH <5)
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_W|GPI_NEG_DUTY_TIM_W
            |GPI_CHIP_CMD_IOW_1|GPI_CHIP_CMD_IOW_2|GPI_CHIP_CMD_IOW_3|GPI_CHIP_CMD_IOW_4;   
            temp_dat = *dataBuf;
            GPILength = GPI_LENGTH;
            //  DEBUG_GPIU("Data length = %X\n",GPI_LENGTH);
            //   DEBUG_GPIU("Write data %x \n",temp_dat);
        }
        else// just write DMA data
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_W|GPI_NEG_DUTY_TIM_W|GPI_DMA_MODE
            |GPI_CHIP_CMD_IOW_1|GPI_CHIP_CMD_IOW_2|GPI_CHIP_CMD_IOW_3|GPI_CHIP_CMD_IOW_4|GPI_BURST4_MODE;
            temp_dat = *dataBuf;
            GPILength = GPI_LENGTH;
        }
    }
    else if(CMDS==2)
    {
        if(GPI_LENGTH <4)
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_W|GPI_NEG_DUTY_TIM_W|GPI_OUT_BE
            |GPI_CHIP_CMD_IOW_1|GPI_CHIP_CMD_IOW_2|GPI_CHIP_CMD_IOW_3|GPI_CHIP_CMD_IOW_4;   
            temp_dat = *dataBuf;
            GPILength = GPI_LENGTH;
        }
        else// just write DMA data
        {
            GPICtrlReg = GPI_CHIP_SEL_1|GPI_POS_DUTY_TIM_W|GPI_NEG_DUTY_TIM_W|GPI_DMA_MODE|GPI_OUT_BE
            |GPI_CHIP_CMD_IOW_1|GPI_CHIP_CMD_IOW_2|GPI_CHIP_CMD_IOW_3|GPI_CHIP_CMD_IOW_4|GPI_BURST4_MODE;
            temp_dat = *dataBuf;
            GPILength = GPI_LENGTH;
        }
    }
    GPICtrlReg = GPICtrlReg | GPI_START_TRIG;

    if(GPI_LENGTH <5)
    {
    #if 1 //polling     

       	cnt=20;
	while (cnt--)/* wait for tx complete */
        {
            if (GPIIntCtrl& GPI_TX_REQ_INT)
            break;
        }
        if(cnt == 0)
            printf("gpi polling 3 error");
        GPIOutput = temp_dat;
    #else           
        {
            OSSemPend(gpiSemTX_REQ, 100, &ucErr);
            GPIOutput = temp_dat;
        }
    #endif              
    }
    else
    {
        //      temp_len = GPI_LENGTH % 4;
        // if(temp_len)   DEBUG_GPIU("DMA length NOT x4\n");
        //      DEBUG_GPIU("DMA Write length = %d\n",GPI_LENGTH);

    #if 1 //polling       
       	cnt=20;
	while (cnt--)/* wait for tx complete */
        {
            if (GPIIntCtrl& GPI_TX_REQ_INT)
            break;
        }
        if(cnt == 0)
            printf("gpi polling 4 error");
    #else     
        OSSemPend(gpiSemTX_REQ, 100, &ucErr);
    #endif
        GPISetWriteDataDma(dataBuf,GPI_LENGTH);
        if (gpiCheckDmaWriteComplete() != GPI_OK)
        {
            DEBUG_GPIU( "GPIToWrite:DMA operation fails\n");  
        }
    }
#if 0 
    if (ucErr != OS_NO_ERR)
    {
        DEBUG_GPIU("Error: gpiSemTX_REQ is %d.\n", ucErr);
    }
#endif

}

int GPIReadWriteReg(void)
{
    u32 i;
    u8 dat,F_ok;
    DEBUG_GPIU("GPI Read Write Reg.\n");
    F_ok = 1;
    dat = 0xFF;
    for( i=0x10; i<0x1d; i++)
    {
        DM9000_iow(i,dat);

        if(dat != DM9000_ior(i))
        {
            DEBUG_GPIU("Error addr %x = %x.\n",i,DM9000_ior(i) );
            F_ok = 0;
        }
    }
    dat = 0xAA;
    for( i=0x10; i<0x1d; i++)
    {
        DM9000_iow(i,dat);

        if(dat != DM9000_ior(i))
        {
            DEBUG_GPIU("Error addr %x = %x.\n",i,DM9000_ior(i) );
            F_ok = 0;
        }
    }
    dat = 0x55;
    for( i=0x10; i<0x1d; i++)
    {
        DM9000_iow(i,dat);
        if(dat != DM9000_ior(i))
        {
            DEBUG_GPIU("Error addr %x = %x.\n",i,DM9000_ior(i) );
            F_ok = 0;
        }
    }

    //DEBUG_GPIU("GPI Read Write END.\n");
    return F_ok;
}

extern u8   broadcast_hwaddr[];

u16 GPIDMAreadtest()
{
    u32 k,RxLen,status;
    u32  temp[400];
    u32  temp_r[400];
    u8 *rdptr;
    DEBUG_GPIU("==GPIDMAreadtest==\n");

    DEBUG_GPIU("DM9000_TRPAH = 0x%8x\n",DM9000_ior(DM9000_TRPAH));
    DEBUG_GPIU("DM9000_TRPAL = 0x%8x\n",DM9000_ior(DM9000_TRPAL));

    DEBUG_GPIU("DM9000_RWPAH = 0x%8x\n",DM9000_ior(DM9000_RWPAH));
    DEBUG_GPIU("DM9000_RWPAL = 0x%8x\n",DM9000_ior(DM9000_RWPAL));
    //        eth_send(inbuf2, broadcast_hwaddr, 0x0608, 28);
    RxLen =60;
    for (k=0 ;k<RxLen;k++)
    {
        inbuf2[k]=k;
    }
    rdptr=inbuf2;
    for (k=0 ;k<(RxLen/4);k++)
    {
        temp[k]=ctol(rdptr);
        rdptr+=4;
        DEBUG_GPIU("temp[%d] = 0x%8x\n",k,temp[k]);
    }
    DM9000_iow(DM9000_MWRH,0);
    DM9000_iow(DM9000_MWRL,0);
    GPIToWrite(1, DM9000_MWCMD, 0, temp);
    DEBUG_GPIU("==GPIToWrite==\n");
    GPIToWrite(0, 0, RxLen, temp);

    DEBUG_GPIU("DM9000_TRPAH = 0x%8x\n",DM9000_ior(DM9000_TRPAH));
    DEBUG_GPIU("DM9000_TRPAL = 0x%8x\n",DM9000_ior(DM9000_TRPAL));

    DEBUG_GPIU("DM9000_RWPAH = 0x%8x\n",DM9000_ior(DM9000_RWPAH));
    DEBUG_GPIU("DM9000_RWPAL = 0x%8x\n",DM9000_ior(DM9000_RWPAL));

    rdptr=outbuf2;

    DEBUG_GPIU("DM9000_MRRH = 0x%8x\n",DM9000_ior(DM9000_MRRH));
    DEBUG_GPIU("DM9000_MRRL = 0x%8x\n",DM9000_ior(DM9000_MRRL));

    DM9000_ior(DM9000_MRRH);
    DM9000_ior(DM9000_MRRL);        //must add this two read,weiyan
    //        DM9000_ior(DM9000_MRCMDX);      /* Dummy read */
    GPIToRead(1,DM9000_MRCMDX,2, &status);
    DEBUG_GPIU("status = 0x%8x\n",status);
    DM9000_iow(DM9000_MRRH,0);
    DM9000_iow(DM9000_MRRL,0);       
    GPIToWrite(1, DM9000_MRCMD, 0,temp_r);

    DEBUG_GPIU("==GPIReadData==\n");
    GPIToRead(0, 0, RxLen,temp_r);
    // GPIReadData(RxLen,temp_r);
    for (k=0 ;k<(RxLen/4);k++)
    {
        ltoc(rdptr,temp_r[k]);
        rdptr+=4;
        DEBUG_GPIU("temp_r[%d] = 0x%8x\n",k,temp_r[k]);
    }
    status = TRUE;
    for (k=0; k<RxLen; k++)
    if (inbuf2[k] != outbuf2[k])
    {
        DEBUG_GPIU("inbuf2[%d] = 0x%8x\n", k, inbuf2[k]);
        DEBUG_GPIU("outbuf2[%d] = 0x%8x\n", k, outbuf2[k]);
        DEBUG_GPIU(" result don't match!\n");
        status = FALSE;
        //break;
    }
    return status;      
}

//--------------
// Send a Packet
//--------------
void send_packet_test()
{
    u16 pkt_len= 60;

    __align(4) u8 data[64]= {0x11,0x22,0x33,0x44,0x55,0x66,0x00,0x50,0x7f,0xa7,0x26,0x30,0x08,0x06,0x00,0x01  
                            ,0x08,0x00,0x06,0x04,0x00,0x01,0x00,0x50,0x7f,0xa7,0x26,0x30,0xc0,0xa8,0x01,0x01   
                            ,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xa8,0x01,0x16,0x3f,0x06,0xb9,0x0a,0x3b,0x7c   
                            ,0xaa,0x45,0xc0,0xa8,0x01,0x26,0x00,0x16,0xc0,0x8e,0x79,0x8f,0x00,0x00,0x00,0x00};

    u32  temp[400],*ptemp;
    u32 k;  
    u8 *rdptr;

    DEBUG_GPIU("==send_packet_test==\n");
    DM9000_iow( DM9000_NCR, NCR_RST);
    DM9000_iow( DM9000_IMR, IMR_PAR);

    rdptr=data;
#if 0   
    for (k=0 ;k<(pkt_len/4);k++)
    {
        temp[k]=ctol(rdptr);
        rdptr+=4;
        DEBUG_GPIU("temp[%d] = 0x%8x\n",k,temp[k]);
    }
#else
    ptemp = (u32*)data;
#endif    
    //  writesw(db->io_data, data, (pkt_len+1) >> 1);
    GPIToWrite(1, DM9000_MWCMD, 0, temp);
    DEBUG_GPIU("==GPIToWrite==\n");
    GPIToWrite(0, 0, pkt_len,ptemp);

/* Set TX length to DM9000 */
    DM9000_iow( DM9000_TXPLL, pkt_len);
    DM9000_iow( DM9000_TXPLH, pkt_len >> 8);

/* Issue TX polling command */
    DM9000_iow( DM9000_TCR, TCR_TXREQ); /* Cleared after TX complete */
    //    while (DM9000_ior(DM9000_TCR) & TCR_TXREQ);   /* wait for end of transmission */
    DEBUG_GPIU("==wait for tx complete ==\n");
    while (1)/* wait for tx complete */
    {
        if (DM9000_ior(DM9000_NSR)& (NSR_TX2END|NSR_TX1END))    
        {
            DEBUG_GPIU("==send a packet 64 bytes==\n");
            break;
        }
    }
}

void GPITestDm9000(void)
{
    dm9000_reset();
    while (dm9000_probe()== FALSE);
    if(dm9000_probe()== FALSE)
    {
        DEBUG_GPIU("Search_1 DM9000 Fail\n");

        if(dm9000_probe()== FALSE)
        {
            DEBUG_GPIU("Search_2 DM9000 Fail\n");
          //  while (1);
            return;
        }
    }
#if 0
    if(GPIReadWriteReg()== FALSE)
    {
        DEBUG_GPIU("GPI Read Write Reg Fail\n");
        return;
    }
    if( GPIDMAreadtest()== FALSE)
    {
        DEBUG_GPIU("GPI DMA Read/Write  Fail\n");
        return;
    }
    DEBUG_GPIU("Test GPI UNiT Pass\n");
#endif
   // while(1)
   send_packet_test();
}
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
void sendrawdata()
{
    u16 pkt_len= 1514;

    __align(4) u8 data[1522]= {0x11,0x22,0x33,0x44,0x55,0x66,0x00,0x50,0x7f,0xa7,0x26,0x30,0x08,0x06,0x45,0x00  
                               ,0x08,0x00,0x06,0x04,0x00,0x01,0x00,0x50,0x7f,0xa7,0x26,0x30,0xc0,0xa8,0x01,0x01   
                               ,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xa8,0x01,0x16,0x3f,0x06,0xb9,0x0a,0x3b,0x7c   
                               ,0xaa,0x45,0xc0,0xa8,0x01,0x26,0x00,0x16,0xc0,0x8e,0x79,0x8f,0x00,0x00,0x00,0x00};

    u32  temp[400],*ptemp;
    u32 n,k;  
    u8 *rdptr;
    u8 ucErr,status;
     
    DEBUG_GPIU("==send_packet_test==\n");
    DM9000_iow( DM9000_NCR, NCR_RST);
    DM9000_iow( DM9000_IMR, IMR_PAR);

    rdptr=data;
#if 0   
    for (k=0 ;k<(pkt_len/4);k++)
    {
        temp[k]=ctol(rdptr);
        rdptr+=4;
        DEBUG_GPIU("temp[%d] = 0x%8x\n",k,temp[k]);
    }
#else
    ptemp = (u32*)data;
#endif 
        data[16] = pkt_len >> 8;
        data[17] = pkt_len & 0xff;
	for (n=0;n<200;n++)
	{
    ptemp = (u32*)data;
//[Do as it each trip.]
    for (k=34;k<60;k++)
     data[k] = (char) rand(); 
    do {
        data[k++] = n;
     } while (k < (1514)); // if > 1514, The sendto() will not send any data out thru 'ethX'
	 /* Send packet */


    //  writesw(db->io_data, data, (pkt_len+1) >> 1);
    GPIToWrite(1, DM9000_MWCMD, 0, temp);
    //DEBUG_GPIU("==GPIToWrite==\n");
    GPIToWrite(0, 0, pkt_len,ptemp);


/* Issue TX polling command */
   status=DM9000_ior(DM9000_ISR);
	 if(status & ISR_PTS)
		{
			DM9000_iow(DM9000_ISR,ISR_PTS);
        //	DEBUG_GPIU("Send_int_clr\n");
		}
      //    DEBUG_GPIU("Send_Pend\n");
	DM9000_iow( DM9000_TXPLL, pkt_len);
	DM9000_iow( DM9000_TXPLH, pkt_len >> 8);

	DM9000_iow( DM9000_TCR, TCR_TXREQ);	/* Cleared after TX complete */

    #if (GPI_TRG_TX_MODE == USE_INTERRUPT)

        // DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PTM);        /* Enable TX interrupt mask */
        OSSemPend(DMiSemTran, 200, &ucErr);
    	if (ucErr != OS_NO_ERR)
        {
    	    DEBUG_GPIU("Error: DMiSemTran is %d.\n", ucErr);
     		//DEBUG_NET("SemEvtOSEventCnt = %d\n", DMiSemTran->OSEventCnt);
    	}
    #elif (GPI_TRG_TX_MODE == USE_POLLING)    

	while (1)/* wait for tx complete */
        {
	   // if (DM9000_ior(DM9000_NSR)& (NSR_TX2END|NSR_TX1END))
          if ( !(DM9000_ior(DM9000_TCR) & TCR_TXREQ) )	
	    {
	     //   DEBUG_GPIU("\n=%d\n",i);
            break;
         }
       }
	    #endif   
    }
      printf("Done.\n");
}
#if 0
void phy_processing()
{
	UINT16 value;
	UINT32 maccr;
	/* read status of phy */
	value = phy_read(REG_STATUS);

	if ((value&FTPHY_REG_STATUS_LINK)==0){
		printf("??? Link status => FAIL\n");
		return (0);
	}

	value = phy_read(FTPHY_REG_CONTROL);

	maccr = ioread32(ftmac110_control->base + FTMAC110_OFFSET_MACCR);
	if((value & FTPHY_REG_CR_FULL) > 0){
		/* full duplex */
		maccr |= FTMAC110_MACCR_FULLDUP;
	}
	else{
		/* half duplex */
		maccr &= ~(FTMAC110_MACCR_FULLDUP);
	}

	if((value & FTPHY_REG_CR_SPEED) > 0){
		/* 100 */
		maccr |= FTMAC110_MACCR_MDC_SEL;
	}
	else{
		/* 10 */
		maccr &= ~(FTMAC110_MACCR_MDC_SEL);
	}
	iowrite32(maccr, ftmac110_control->base + FTMAC110_OFFSET_MACCR);

	return (1);
}

void change_duplex_speed(DUPLEX duplex, SPEED speed)
{
	/*
	 * The operation of change duplex mode and speed.
	 * 1. reset phy
	 * 2. disable auto-negotiation and set duplex and speed to phy
	 * 3. restart auto-N
	 * 4. call phy_processing to set mac
	 */
	UINT16 value;
	int i;
	/* 1. reset phy */
    value = phy_read(FTPHY_REG_CONTROL);
	value |= FTPHY_REG_CR_RESET;
	phy_write(value, FTPHY_REG_CONTROL);
	/* wait for reset complete */
	do{
		for(i = 0;i < 10000; i++);
		value = phy_read(FTPHY_REG_CONTROL);
	}while((value & FTPHY_REG_CR_RESET) > 0);

	/* 2.disable auto-negotiation and set duplex and speed to phy */
    value = phy_read(FTPHY_REG_AUTO_NEG);
	value = value & (~FTPHY_REG_04_MASK);

	switch(speed){
		case _100:
			if(duplex == FULL){
				value |= FTPHY_REG_04_100FULL;
			}
			else{
				value |= FTPHY_REG_04_100HALF;
			}
			break;
		case _10:
			if(duplex == FULL){
				value |= FTPHY_REG_04_10FULL;
			}
			else{
				value |= FTPHY_REG_04_10HALF;
			}
			break;
	}
	phy_write(value, FTPHY_REG_AUTO_NEG);
	/* 3. restart auto-N */
	value = phy_read(FTPHY_REG_CONTROL);
	value |= FTPHY_REG_CR_RESTART_AUTO;
	phy_write(value, FTPHY_REG_CONTROL);
	/* wait for auto-N complete */
	do{
		for(i = 0;i < 10000; i++);
		value = phy_read(FTPHY_REG_STATUS);
	}while((value & FTPHY_REG_STATUS_AUTO_OK) == 0);
	/* 4. call phy_processing to set mac */
	phy_processing();
}
#endif
#endif
#endif 
