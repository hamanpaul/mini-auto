/**************************************************************************
   Copyright  Faraday Technology Corp 2011.  All rights reserved.
  -------------------------------------------------------------------------
   Name: FTMAC110.c
   Description: FTMAC110 Non-OS firmware
   Author: Jason Lin
   Date: 2011/08/08
   Version:1.0
***************************************************************************/

#include "general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osapi.h"
#include "FTMAC110.h"
#include "task.h"
#include "sysapi.h"

#if (NIC_SUPPORT == 1)
OS_STK nicTaskStack[NIC_TASK_STACK_SIZE]; /* Stack of task ciuTask() */

//OS_EVENT *MACSemDMAComplete;        /* semaphore to synchronize DMA complete  */
//OS_EVENT *MACSemRX_VALID;      /* Semaphore to synchronize RX_VALID interrupt. */
//OS_EVENT *MACSemTX_REQ;      /* Semaphore to synchronize RX_VALID interrupt. */

__align(32) UINT8 rx_buff[RX_BUF_SIZE * RX_QUEUE_ENTRIES];
UINT32 rx_len;
__align(16) ftmac110_rxdes RX_DESC_ADDR[RX_QUEUE_ENTRIES];
__align(16) ftmac110_txdes TX_DESC_ADDR[TX_QUEUE_ENTRIES];

INT32 rxbuf_size;
UINT32 wakeup_frame_mask[4];
u8	gPHY_ready=2;	//20171002 Sean: 2 means init value, 1 means PHY ready value, 0 means PHY processing.
ftmac110_txdes *txdes_top;
ftmac110_txdes *txdes_cur;
ftmac110_txdes *txdes_end;
UINT32 txdes_num;

ftmac110_rxdes *rxdes_top;
ftmac110_rxdes *rxdes_cur;
ftmac110_rxdes *rxdes_end;
UINT32 rxdes_num;

int net_link_status = 1; //Sean: 20170921 Default Set icon OFF.

extern u8 renewIP;
extern OS_EVENT *hEthernetInput;
/* default source mac address */
extern UINT8 MAC_ADDR[6];
extern UINT8 RxBufs[1600];
//extern UINT8  my_hwaddr[6];
extern UINT8 uiMACAddr[6];

#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#if(USE_DBGPRINT_PTR)

static dbg_printf pDbgPrint_ftmac110 = 0;

#define DbgPrint_ftmac110(fmt...)	{if(pDbgPrint_ftmac110)	pDbgPrint_ftmac110(fmt);}

#ifdef DbgPrint
#undef DbgPrint
#endif

#define DbgPrint(fmt...)	DbgPrint_ftmac110(fmt)


/**	enable/disable the debug message output of ftmac110_isr()	*/
void ftmac110isr_dbgprint_enabled(u8 Enabled)
{
//Enabled:
//non-zero: 	enable the debug message output
//zero:		disable debug message output
	u32 masked_flag=_dbgprint_flag_get();

	if(Enabled)
		masked_flag|=DBGPRINT_FLAG_EN_FTMAC110;
	else
		masked_flag&=(~DBGPRINT_FLAG_EN_FTMAC110);

	_dbgprint_flag_set(masked_flag);

	masked_flag=_dbgprint_flag_get();
	//b0
	pDbgPrint_ftmac110=(masked_flag & DBGPRINT_FLAG_EN_FTMAC110)?printf:0;
	//...
}
#endif //#if(USE_DBGPRINT_PTR)
#endif

void (*rx_isr)(FTMAC110_Device *dev);
void iowrite32(UINT32 value, UINT32 addr)
{
	*((volatile UINT32 *)addr) = value;
}

UINT32 ioread32(UINT32 addr)
{
     return *((volatile UINT32 *)addr);
}

/*
 * a global variable for ftmac110,
 * initialized when user call malloc_ftmac110_device function
 */




unsigned char ftmac110_pause(void)
{

	return 0;
}

void ftmac110_delaysecs(UINT32 i)
{
	i=i*1000000;
	while(i--);

}

int ftmac110_addrcmp(const UINT8 *addr1, const UINT8 *addr2)
{
	int i;
	for(i = 0;i < 6;i++){
		if(addr1[i] != addr2[i]){
          printf("addrcmp err ");
			return -1;
		}
	}
	return 0;
}

UINT32 ftmac110_fpga_version()
{
//UINT32 i;

    //for(i = FTMAC110_BASE;i <= FTMAC110_OFFSET_XP;i+=4){
         //printf("%x=( %x)\n",i,ioread32(i));
         //}
	return ioread32( FTMAC110_OFFSET_FPGA_VER);
}

void ftmac110_reset_mac()
{
	iowrite32(FTMAC110_MACCR_SW_RST,  FTMAC110_OFFSET_MACCR);
}

void ftmac110_reset_phy()
{
	UINT16 value;
	/* read phy */
	/* fill out PHYCR */
    //printf("FTMAC110 Reset Phy \n");
	iowrite32(
				FTMAC110_PHYCR_PHYAD(FTPHY_ADD) |
				FTMAC110_PHYCR_REGAD(FTPHY_REG_CONTROL) |
				FTMAC110_PHYCR_MIIRD
				, FTMAC110_OFFSET_PHYCR);
	/* wait for read complete */
    //printf("\nEntry while loop\n");
	while(ioread32(FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIRD);
    //printf("read complete \n");
	value = (ioread32( FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIRDATA);

	value |= FTPHY_REG_CR_RESET;
	/* write to phy */
	/* write data to PHYWDATA (0x94h) */
	iowrite32(value, FTMAC110_OFFSET_PHYWDATA);
	/* wait */
	while(ioread32( FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIWR);

	/* fille out PHYCR */
	iowrite32(
				FTMAC110_PHYCR_PHYAD(FTPHY_ADD) |
				FTMAC110_PHYCR_REGAD(FTPHY_REG_CONTROL) |
				FTMAC110_PHYCR_MIIWR
				,  FTMAC110_OFFSET_PHYCR);

////////////////////////////////////

/*
iowrite32(
				FTMAC110_PHYCR_PHYAD(FTPHY_ADD) |
				FTMAC110_PHYCR_REGAD(FTPHY_REG_INTERRUPT) |
				FTMAC110_PHYCR_MIIWR
				,  FTMAC110_OFFSET_PHYCR);
*/

	/* wait */
	while(ioread32(FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIWR);
    //printf("ftmac110_reset_phy complete \n");

	/*ENABLE_FTMAC110_INT_PHYSTS_CHG*/
	//phywrite16(0x8000,FTPHY_REG_CONTROL_TEST2);

   //=========raymond add for FORCE in loop or 10M.
   // value = phyread16(FTPHY_REG_CONTROL);
	//value &= ~(FTPHY_REG_CR_SPEED|FTPHY_REG_CR_AUTO);
   // value |=FTPHY_REG_CR_LOOPBACK;
	//phywrite16(value, FTPHY_REG_CONTROL);
    //========================================
    //=========raymond add for FORCE in 10M.
    //value =FTPHY_REG_04_10FULL;
	//phywrite16(value,FTPHY_REG_AUTO_NEG );

    //========================================
}


void ftmac110_close()
{
	/* a sw reset */
	iowrite32(FTMAC110_MACCR_SW_RST, FTMAC110_OFFSET_MACCR);
}

void ftmac110_xmit(UINT32 phy_tx_addr, UINT32 size)
{

#if OS_CRITICAL_METHOD == 3   // 發送時關中斷。
    OS_CPU_SR  		cpu_sr = 0;
#endif

//UINT8 ucErr=0;
	/* in this function, tx is always completed by one descriptor */
	while(txdes_cur->txdes0 & FTMAC110_TXDES0_TXDMA_OWN){
		printf("??? => tx descriptor is full\n");
		OSTimeDly(1);
//		return;
	}
OS_ENTER_CRITICAL();
   // printf("??? =>1\n");
	/* initial txdes */
	txdes_cur->txdes2 = phy_tx_addr;
	/* reset txdes1 */
	txdes_cur->txdes1 &= FTMAC110_TXDES1_EDOTR;
	txdes_cur->txdes1 |=
			FTMAC110_TXDES1_TXBUF_SIZE(size) |
			FTMAC110_TXDES1_LTS |
			FTMAC110_TXDES1_FTS |
			FTMAC110_TXDES1_TXIC;
	txdes_cur->txdes0 = FTMAC110_TXDES0_TXDMA_OWN;
	txdes_cur = (ftmac110_txdes *)txdes_cur->txdes3;
OS_EXIT_CRITICAL();
#if 0
      printf("txdes_cur->txdes0=%x\n",txdes_cur->txdes0);
            printf("txdes_cur->txdes1=%x\n",txdes_cur->txdes1);
                  printf("txdes_cur->txdes2=%x\n",txdes_cur->txdes2);
                        printf("txdes_cur->txdes3=%x\n",txdes_cur->txdes3);
                   printf("FTMAC110_OFFSET_TXR_BADR=%x\n",ioread32(FTMAC110_OFFSET_TXR_BADR));

         printf("txdes_top->txdes0=%x\n",txdes_top->txdes0);
            printf("txdes_top->txdes1=%x\n",txdes_top->txdes1);
                  printf("txdes_top->txdes2=%x\n",txdes_top->txdes2);
                        printf("txdes_top->txdes3=%x\n",txdes_top->txdes3);
#endif
	if(0 == (ioread32( FTMAC110_OFFSET_APTC) & FTMAC110_APTC_TXPOLL_CNT(0xf)))
     {
         //  printf("kick off txdma\n");
     		//iowrite32(0x0000ffff,FTMAC110_OFFSET_TXPD);
            *((UINT32 *)FTMAC110_OFFSET_TXPD) = 0x0000ffff;
       }
/*
   while(txdes_cur->txdes0 & FTMAC110_TXDES0_TXDMA_OWN){
		printf(".");

	}
*/
/*
        OSSemPend(MACSemTX_REQ, OS_IPC_WAIT_FOREVER, &ucErr);

    if (ucErr != OS_NO_ERR)
    {
        printf("Error: MACSemTX_REQ is %d.\n", ucErr);
    }
*/
      //   printf("FTMAC110_OFFSET_ISR=%x\n",ioread32(FTMAC110_OFFSET_ISR));
      //  txdes_cur = (ftmac110_txdes *)txdes_cur->txdes3;
       // printf("ftmac110_xmit end\n");
}

void ftmac110_xmit_multi(const UINT32 number, FTMAC110_multides txdeses[])
{
	/* the first descriptor must be set at last */
	ftmac110_txdes *tmp = (ftmac110_txdes *)txdes_cur->txdes3;
	UINT32 i;

	if(number <= 0){
		printf("??? => number of description is less than zero\n");
		return;
	}
	else if(1 == number){
		ftmac110_xmit(txdeses[0].phy_tx_addr, txdeses[0].size);
	}
	else{
		if(txdes_cur->txdes0 & FTMAC110_TXDES0_TXDMA_OWN){
			printf("??? => tx descriptor is full\n");
			return;
		}

		for(i = 1; i < (number-1); i++){
			if(tmp->txdes0 & FTMAC110_TXDES0_TXDMA_OWN){
				printf("??? => tx descriptor is full\n");
				return;
			}

			tmp->txdes2 = txdeses[i].phy_tx_addr;
			/* reset txdes1 */
			tmp->txdes1 &= FTMAC110_TXDES1_EDOTR;
			tmp->txdes1 |= FTMAC110_TXDES1_TXBUF_SIZE(txdeses[i].size);
			tmp->txdes0 = FTMAC110_TXDES0_TXDMA_OWN;
			tmp = (ftmac110_txdes *)tmp->txdes3;
		}

		/* the last descriptor */
		if(tmp->txdes0 & FTMAC110_TXDES0_TXDMA_OWN){
			printf("??? => tx descriptor is full\n");
			return;
		}
		tmp->txdes2 = txdeses[i].phy_tx_addr;
		/* reset txdes1 */
		tmp->txdes1 &= FTMAC110_TXDES1_EDOTR;
		tmp->txdes1 |= FTMAC110_TXDES1_TXBUF_SIZE(txdeses[i].size) | FTMAC110_TXDES1_LTS;
		tmp->txdes0 = FTMAC110_TXDES0_TXDMA_OWN;

		/* the first descriptor */
		txdes_cur->txdes2 = txdeses[0].phy_tx_addr;
		/* reset txdes1 */
		txdes_cur->txdes1 &= FTMAC110_TXDES1_EDOTR;
		txdes_cur->txdes1 |= FTMAC110_TXDES1_TXBUF_SIZE(txdeses[i].size) |
											FTMAC110_TXDES1_FTS | FTMAC110_TXDES1_TXIC;
		txdes_cur->txdes0 = FTMAC110_TXDES0_TXDMA_OWN;
		txdes_cur = tmp;

		if(0 == (ioread32( FTMAC110_OFFSET_APTC) & FTMAC110_APTC_TXPOLL_CNT(0xf))){
			/* kick off txdma */
			iowrite32(0xFFFFFFFF,  FTMAC110_OFFSET_TXPD);
		}
	}
}

void ftmac110_half_dup_10()
{
	change_duplex_speed(HALF, _10);
}

void ftmac110_half_dup_100()
{
	change_duplex_speed(HALF, _100);
}

void ftmac110_full_dup_10()
{
	change_duplex_speed(FULL, _10);
}

void ftmac110_full_dup_100()
{
	change_duplex_speed(FULL, _100);
}

UINT8 ftmac110_set_multicast_address(UINT8 *mac_addr)
{
	/*
	 * 1. generate crc-32 and get the 6 bits (most significant)
	 * 2. set the registers
	 * 3. wrtie backup the bit to the hashtable0/hashtable1
	 */
	UINT8 crc_6bit = 0, crc_6bit_reserve = 0; /* crc value with 6 bits */
	UINT32 table_num = 0;
	UINT32 set_bit = 0, tmp32 = 0;

	crc_6bit = ftmac110_crc32_ret_6_bit(mac_addr);
	crc_6bit_reserve = crc_6bit;

	if(crc_6bit > 31){
		table_num = 1;
		crc_6bit -= 32;
	}
	else{
		table_num = 0;
	}

	set_bit = (1 << crc_6bit);
	if(0 == table_num){
		tmp32 = ioread32( FTMAC110_OFFSET_MAHT0);
		tmp32 |= set_bit;
		iowrite32(tmp32,  FTMAC110_OFFSET_MAHT0);
	}
	else{
		tmp32 = ioread32( FTMAC110_OFFSET_MAHT1);
		tmp32 |= set_bit;
		iowrite32(tmp32,  FTMAC110_OFFSET_MAHT1);
	}

	return crc_6bit_reserve;
}

UINT8 *ftmac110_get_mac_addr()
{
	return MAC_ADDR;
}

void ftmac110_rx_manual()
{
	/* kick off rxdma */
	iowrite32(0xffffffff, FTMAC110_OFFSET_RXPD);
}

void ftmac110_enter_power_down(UINT32 power_down_setting, UINT8 *wakeup_frame, UINT32 wakeup_frame_size)
{
	 /* 1.  set XDMA_EN (88h.0) = 0 */
	 iowrite32((~FTMAC110_MACCR_XDMA_EN) & ioread32(FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
	 /* 2.  poll the dma/fifo state register (C8h) to wait for tx fifo empty*/
	 while(0 == (ioread32( FTMAC110_OFFSET_DMAFIFOS) & FTMAC110_TXFIFO_EMPTY));
	 /* 3.  set XMT_EN (88h.5) = 0 to terminate the tramsmission */
	 iowrite32((~FTMAC110_MACCR_XMT_EN) & ioread32(FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
	 /* 4.  set RCV_EN (88h.8) = 0 */
	 iowrite32((~FTMAC110_MACCR_RCV_EN) & ioread32(FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
	 /* 5.  poll the dma/fifo state register (C8h) to wait for rx fifo empty */
	 while(0 == (ioread32(FTMAC110_OFFSET_DMAFIFOS) & FTMAC110_RXFIFO_EMPTY));
	 /* 6.  set RDMA_EN (88h.1) = 0 */
	 iowrite32((~FTMAC110_MACCR_RDMA_EN) & ioread32( FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
	 /* 7.  program the wake-up frame */
	 /* write mask */
	 iowrite32(wakeup_frame_mask[0],  FTMAC110_OFFSET_WFBM1);
	 iowrite32(wakeup_frame_mask[1],  FTMAC110_OFFSET_WFBM2);
	 iowrite32(wakeup_frame_mask[2],  FTMAC110_OFFSET_WFBM3);
	 iowrite32(wakeup_frame_mask[3],  FTMAC110_OFFSET_WFBM4);
	 /* calculate crc value */
	 iowrite32(ftmac110_wakeup_crc32(wakeup_frame, wakeup_frame_size, wakeup_frame_mask), FTMAC110_OFFSET_WFCRC);
	 /* 8.  write 32'hFFFF_FFFF to clear wake-on-lan status register (A4h) */
	 iowrite32(0xffffffff, FTMAC110_OFFSET_WOLSR);
	 /* 9.  program the requested wake-up events and
	  *     power state into wake-on-lan register (A0h)
	  *     to let FTMAC110 go into the power saving mode
	  */
	 iowrite32(power_down_setting, FTMAC110_OFFSET_WOLCR);
	 /* 10. set RCV_EN (88h.8) = 1 to enable the reception (RCV_ALL has to be enable, too) */
	 iowrite32(FTMAC110_MACCR_RCV_EN | FTMAC110_MACCR_RCV_ALL | ioread32( FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
}

void ftmac110_exit_power_down()
{
	/* 1. wait for the occurrence of wake-up events */
	while(0 == ioread32( FTMAC110_OFFSET_WOLSR));
	/* 2. set RCV_EN (88h.8) = 0 to terminate the reception */
	iowrite32((~FTMAC110_MACCR_RCV_EN) & ioread32( FTMAC110_OFFSET_MACCR),  FTMAC110_OFFSET_MACCR);
	/* 3. read the Wake-On-LAN status register (A4h) to check which wake-up event happened */

	/* 4. program the Wake-On-LAN register (A0h) to let FTMAC110 exit the power-saving mode */
	iowrite32((~0x7f) & ioread32( FTMAC110_OFFSET_WOLCR),  FTMAC110_OFFSET_WOLCR);
	/* 5. set SW_RST (88h.2) = 1 to reset FTMAC110 */
	iowrite32(FTMAC110_MACCR_SW_RST,  FTMAC110_OFFSET_MACCR);
	/* 6. check SW_RST (88h.2) = 0 to make sure that FTMAC110 has finished reset */
	while(ioread32( FTMAC110_OFFSET_MACCR) & FTMAC110_MACCR_SW_RST);
	/* 7. re-initialize FTMAC110 to transmit and receive packets */
}

void ftmac110_set_wakeup_mask(UINT32 mask_value, UINT32 mask_index)
{
	if((mask_index >= 1) && (mask_index <= 4)){
		wakeup_frame_mask[mask_index - 1] = mask_value;
	}
	else{
		printf("mask num must in [1,4]\n");
	}
}

void ftmac110_set_flow_control(UINT16 pause_time, UINT8 high_threshold, UINT8 low_threshold, UINT8 fc_thr_en, UINT8 fc_en)
{
	UINT32 set_value = 0x0;
	set_value = FTMAC110_FCR_FC_PAUSE_TIME(pause_time);
	set_value |= FTMAC110_FCR_FC_HIGH(high_threshold);
	set_value |= FTMAC110_FCR_FC_LOW(low_threshold);
	if(fc_thr_en){
		set_value |= FTMAC110_FCR_FC_THREN;
	}
	if(fc_en){
		set_value |= FTMAC110_FCR_FC_EN;
	}
	iowrite32(set_value,  FTMAC110_OFFSET_FCR);
}

void ftmac110_xmit_pause_frame()
{
	iowrite32(FTMAC110_FCR_TX_PAUSE | ioread32(FTMAC110_OFFSET_FCR),  FTMAC110_OFFSET_FCR);
}

UINT32 ftmac110_wait_pause_frame()
{
	if((FTMAC110_FCR_RX_PAUSE & ioread32( FTMAC110_OFFSET_FCR)) > 0){
		return 1;
	}
	else{
		return 0;
	}
}

void initial_interrupt()
{

}

void ftmac110_isr(void)
{
	UINT32 int_status;
	UINT8 ucErr=0;

	int_status = ioread32(FTMAC110_OFFSET_ISR);
     //printf("#int_status=%x\n",int_status);
	if(int_status & FTMAC110_INT_RPKT_FINISH){
//printf("#R\n");
		//rpkt_finish_handler();
		//if(rx_len != 0)
        	OSSemPost(hEthernetInput);

	}

	if(int_status & FTMAC110_INT_NORXBUF){
	#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (USE_DBGPRINT_PTR) )
		DbgPrint("#FTMAC110_INT_NORXBUF\n");
	#endif
	}

	if(int_status & FTMAC110_INT_XPKT_FINISH){
       printf("#3\n");
		xpkt_finish_handler();
        //OSSemPost(MACSemTX_REQ);
	}

	if(int_status & FTMAC110_INT_NOTXBUF){
       //printf("#FTMAC110_INT_NOTXBUF\n");

	}

	if(int_status & FTMAC110_INT_XPKT_OK){
		//xpkt_ok_handler();
		//printf("#T\n");
		//OSSemPost(MACSemTX_REQ);
		//printf("#T\n");
	}

	if(int_status & FTMAC110_INT_XPKT_LOST){
       //printf("#FTMAC110_INT_XPKT_LOST\n");

	}

	if(int_status & FTMAC110_INT_RPKT_SAV){
       //printf("#FTMAC110_INT_RPKT_SAV\n");

	}

	if(int_status & FTMAC110_INT_RPKT_LOST){
	#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (USE_DBGPRINT_PTR) )
		DbgPrint("#FTMAC110_INT_RPKT_LOST\n");
	#endif
	}

    if(int_status & FTMAC110_INT_AHB_ERR){
	#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) && (USE_DBGPRINT_PTR) )
		DbgPrint("##FTMAC110_INT_AHB_ERR\n");
	#else
       printf("#FTMAC110_INT_AHB_ERR\n");
	#endif
	}

    if(int_status & FTMAC110_INT_PHYSTS_CHG){
		//physts_chg_handler();
       //printf("#FTMAC110_INT_PHYSTS_CHG\n");
	}

}

void rpkt_finish_handler(void)
{
	UINT32 copy_size = 0;
	UINT32 remain_size = 0;
	UINT8  *copy_buf = NULL;

#if OS_CRITICAL_METHOD == 3   // 發送時關中斷。
    OS_CPU_SR  		cpu_sr = 0;
#endif

	/* initialize copy receive data buffer */
	memset(RxBufs, 0, sizeof(RxBufs));
	rx_len = 0;

	/*
	if(!(rxdes_cur->rxdes0 & FTMAC110_RXDES0_FRS))
	{
		printf("des0=%x\n",rxdes_cur->rxdes0);
	}
	*/
	if(rxdes_cur->rxdes0 & FTMAC110_RXDES0_FRS)
	{
		/* check rx packet */
		if(rxdes_cur->rxdes0 & FTMAC110_RXDES0_RX_ODD_NB){
			printf("??? => receive odd nibbles\n");
			return;
		}

		if(rxdes_cur->rxdes0 & FTMAC110_RXDES0_CRC_ERR){
			printf("??? => receive crc error\n");
			return;
		}

		if(rxdes_cur->rxdes0 & FTMAC110_RXDES0_RX_ERR){
			printf("??? => receive error\n");
			return;
		}

		rx_len = rxdes_cur->rxdes0 & FTMAC110_RXDES0_RFL;
//printf("R_rx_len=%d\n",rx_len);
		if(rx_len > MAX_PKT_SIZE){
			printf("??? => receive large packet which large than rx buffer size: %d\n", MAX_PKT_SIZE);
			return;
		}

		/* plus 2 for zero copy */
		remain_size = (rxdes_cur->rxdes0 & FTMAC110_RXDES0_RFL) + 2;
		copy_size = remain_size;
		copy_buf = RxBufs;
	}
	else{
		/* check the first descriptor */
		//printf("??? => not the first receive segment descriptor\n");
		return;
	}

	while(1){

		OS_ENTER_CRITICAL();
		/* wait for rx_dma completes the frame reception */
		while(rxdes_cur->rxdes0 & FTMAC110_RXDES0_RXDMA_OWN);
		if(remain_size > RX_BUF_SIZE){
			copy_size = RX_BUF_SIZE;
			remain_size -= RX_BUF_SIZE;
		}
		else{
			copy_size = remain_size;
		}

		memcpy((void *)(copy_buf), (void *)rxdes_cur->rxdes2, copy_size);
#if 0 //buffer chkeck data
        {
         u32 i;

            printf( "RX:@_index %p\n",copy_buf);
		    for (i=0 ;i<copy_size-2;i++)
		   //	 for (i=0 ;i<16;i++)
	   	    {
	   	        if((i%16)==0)DEBUG_GPIU("R_[%.3x] =",i);

                DEBUG_GPIU(" %.2x",copy_buf[i+2]);

			    if((i%16)==15) 	DEBUG_GPIU("\n");

	   	    }
        	printf("\n");
        }

#endif
		copy_buf = copy_buf + copy_size;

		if(rxdes_cur->rxdes0 & FTMAC110_RXDES0_LRS){
			/* the last descriptor -> finish copy */
			rxdes_cur->rxdes0 = FTMAC110_RXDES0_RXDMA_OWN;
			rxdes_cur = (ftmac110_rxdes *)rxdes_cur->rxdes3;
			OS_EXIT_CRITICAL();
			break;
		}
		/* release own of this rxdes */
		rxdes_cur->rxdes0 & FTMAC110_RXDES0_RXDMA_OWN;
		rxdes_cur = (ftmac110_rxdes *)rxdes_cur->rxdes3;
		OS_EXIT_CRITICAL();
	}
}

void norxbuf_handler(void)
{
}

void xpkt_finish_handler(void)
{
}

void xpkt_ok_handler(void)
{
	/* point to next */
	txdes_cur = (ftmac110_txdes *)txdes_cur->txdes3;
}

void xpkt_lost_handler(void)
{
}

void rpkt_sav_handler(void)
{
}

void rpkt_lost_handler(void)
{
}

void ahb_error_handler(void)
{
}

void physts_chg_handler(void)
{
	UINT16 value;
	if(phyread16(FTPHY_REG_CONTROL_TEST2)&0x01)
	{
		if(phyread16(FTPHY_REG_CONTROL_TEST3)&0x0400)
		{
			net_link_status=0;
			renewIP=1;
			printf("LINK UP.\n");
			//change_duplex_speed(FULL, _100);

		}
		else
		{
			net_link_status=1;
			ClearNetworkInfo();
			printf("LINK DOWN.\n");
		}
	uiOsdDrawNetworkLink(net_link_status);
	}
}

#if NET_STATUS_POLLING
void phyNETRunPerSec(void)
{
#if (A1025_GATE_WAY_SERIES)
 int p2p_info;
#endif
    if (gPHY_ready)
    {
		if(phyread16(FTPHY_REG_CONTROL_TEST2)&0x01)
		{
			if(phyread16(FTPHY_REG_CONTROL_TEST3)&0x0400)
			{
				if(net_link_status == 1)
				{
					if(gPHY_ready == 2)
						gPHY_ready=0;
					renewIP=1;
					printf("LINK UP.\n");
				}
				net_link_status=0;
			}
			else if((net_link_status == 0) && !(phyread16(FTPHY_REG_CONTROL_TEST3)&0x0400))
			{
				net_link_status=1;
				ClearNetworkInfo();
				printf("LINK DOWN.\n");
			}
			uiOsdDrawNetworkLink(net_link_status);
		}
#if (A1025_GATE_WAY_SERIES)
		else if(net_link_status==0)
		{
			Check_P2P_info(&p2p_info);
			if(p2p_info & 0x06)
				uiOsdDrawNetworkLink(2);
		}
#endif
	}
}
#endif

void alloc_txdes()
{
	int i;
	ftmac110_txdes *tmp;

	/* use static address (16-byte alignment) */
	txdes_top =  TX_DESC_ADDR;
	txdes_cur = txdes_top;
	txdes_end = txdes_top + (txdes_num - 1);
    printf("txdes_top=%x\n",txdes_top);
     printf("txdes_end=%x\n",txdes_end);
	/* initial txdes rings */
	memset((void *)TX_DESC_ADDR, 0, txdes_num * sizeof(ftmac110_txdes));
	/* set last txdes flag */
	txdes_end->txdes1 |= FTMAC110_TXDES1_EDOTR;

	/* initial txdes3 for next pointer */
	tmp = txdes_top;
	for(i = 0; i < txdes_num; i++){
		tmp->txdes3 = (UINT32)(tmp + 1);
       // printf("tmp->txdes3=%x\n",tmp->txdes3);
		tmp = (ftmac110_txdes *)tmp->txdes3;
       // printf("tmp=%x\n",tmp);
	}
	txdes_end->txdes3 = (UINT32)txdes_top;
    tmp = txdes_top;
/*
    	for(i = 0; i < txdes_num; i++){
        printf("tmp=%x\n",tmp);
        printf("tmp->txdes0=%x\n",tmp->txdes0);
        printf("tmp->txdes1=%x\n",tmp->txdes1);
        printf("tmp->txdes2=%x\n",tmp->txdes2);
        printf("tmp->txdes3=%x\n",tmp->txdes3);
        tmp++;

	}
*/
}

void alloc_rxdes()
{
	int i;
	ftmac110_rxdes *tmp;

	/* use static address (16-byte alignment) */
	rxdes_top =  RX_DESC_ADDR;
	rxdes_cur = rxdes_top;
	rxdes_end = rxdes_top + (rxdes_num - 1);

	/* initial rxdes rings */
	memset(RX_DESC_ADDR, 0, rxdes_num * sizeof(ftmac110_rxdes));
	/* set last rxdes flag */
	rxdes_end->rxdes1 |= FTMAC110_RXDES1_EDORR;

	/* initial txdes3 for next pointer */
	tmp = rxdes_top;
	for(i = 0; i < rxdes_num; i++){
		/* must set own to mac */
		tmp->rxdes0 |= FTMAC110_RXDES0_RXDMA_OWN;
		tmp->rxdes1 |= FTMAC110_RXDES1_RXBUF_SIZE(rxbuf_size);
		/* use static address (4-byte alignment) */
		tmp->rxdes2 =(u32)rx_buff + rxbuf_size*i;
		tmp->rxdes3 = (UINT32)(tmp + 1);
		tmp = (ftmac110_rxdes *)tmp->rxdes3;
	}
	rxdes_end->rxdes3 = (UINT32)rxdes_top;
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
#if (PHY_FORCE_NEGOTIATE_SUPPORT == 1)
    static u8 retryCnt = 0;
_RETRY__:
#endif

	/* 1. reset phy */
    value = phyread16(FTPHY_REG_CONTROL);
	value |= FTPHY_REG_CR_RESET;
	phywrite16(value, FTPHY_REG_CONTROL);
	/* wait for reset complete */
	do{
		for(i = 0;i < 80000; i++);
			value = phyread16(FTPHY_REG_CONTROL);
		OSTimeDly(1);
	}while((value & FTPHY_REG_CR_RESET) > 0);
    //=========raymond add for FORCE in 10M.
   // value = phyread16(FTPHY_REG_CONTROL);
	//value &= ~(FTPHY_REG_CR_SPEED|FTPHY_REG_CR_AUTO);
   // value |=FTPHY_REG_CR_LOOPBACK;
	//phywrite16(value, FTPHY_REG_CONTROL);
    //========================================
	/* 2.disable auto-negotiation and set duplex and speed to phy */
    value = phyread16(FTPHY_REG_AUTO_NEG);
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
	phywrite16(value, FTPHY_REG_AUTO_NEG);
	/* 3. restart auto-N */
	value = phyread16(FTPHY_REG_CONTROL);
	value |= FTPHY_REG_CR_RESTART_AUTO;
	phywrite16(value, FTPHY_REG_CONTROL);
	/* wait for auto-N complete */
	do{
		for(i = 0;i < 80000; i++);
			value = phyread16(FTPHY_REG_STATUS);
		OSTimeDly(1);
	}while((value & FTPHY_REG_STATUS_AUTO_OK) == 0);
	/* 4. call phy_processing to set mac */
	i = phy_processing();
#if (PHY_FORCE_NEGOTIATE_SUPPORT == 1)
    if( i != (duplex <<1 & speed))
    {
        if(retryCnt++ >=3)
        {
            printf("Ethernet negotiate fail, force reboot\n");
            sysForceWDTtoReboot();
        }
        goto _RETRY__;
    }
    else
        retryCnt = 0;
#endif
}

static UINT8 ftmac110_crc32_ret_6_bit(UINT8 *mac_addr)
{
	INT32 perByte;
    INT32 perBit;
    const UINT32 poly = 0x04C11DB7;
    UINT32 crc_value = 0xFFFFFFFF;
	UINT8 c;
	for ( perByte = 0; perByte < 6; perByte ++ ) {
		c = *(mac_addr++);
		for ( perBit = 0; perBit < 8; perBit++ ) {
			crc_value = (crc_value<<1)^
						((((crc_value>>31)^c)&0x01)?poly:0);
			c >>= 1;
		}
	}
	crc_value=crc_value>>26; //Return most significant 6 bits.
	return ((UINT8)crc_value);
}

static UINT32 ftmac110_wakeup_crc32(UINT8 *wakeup_frame, UINT32 size, UINT32 *mask)
{
	INT32 perByte;
	INT32 perBit;
	const UINT32 poly = 0x04C11DB7;
	UINT32 crc_value = 0xFFFFFFFF;
	UINT8 c;
	for ( perByte = 0; perByte < size; perByte ++ ){
		if(perByte >= 128){
			break;
		}

		if((1 << (perByte % 32)) & mask[perByte >> 5]){
			c = *(wakeup_frame++);
			for ( perBit = 0; perBit < 8; perBit++ ){
				crc_value = (crc_value<<1)^
							((((crc_value>>31)^c)&0x01)?poly:0);
				c >>= 1;
			}
		}
		else{
			wakeup_frame++;
		}
	}
	return (crc_value);
}

UINT32 phy_processing()
{
	UINT16 value;
	UINT32 maccr;
#if (PHY_FORCE_NEGOTIATE_SUPPORT == 1)
    UINT32 ret=0;
#endif
	/* read status of phy */

	value = phyread16(FTPHY_REG_STATUS);
    printf("FTPHY_REG_STATUS:0x%x \n",value);
	while ((value&FTPHY_REG_STATUS_LINK)==0){
		net_link_status=1;
		printf("~Link => FAIL\n");
        value = phyread16(FTPHY_REG_STATUS);
		OSTimeDly(1);
		//return (0);
	}
	net_link_status=0;
    printf(" Link status => Ok\n");

	value = phyread16(FTPHY_REG_CONTROL);
    printf("phy_FTPHY_REG_CONTROL:0x%x \n",value);

	maccr = ioread32(FTMAC110_OFFSET_MACCR);
    printf(" FTMAC110_OFFSET_MACCR= %x \n",maccr);
	if((value & FTPHY_REG_CR_FULL) > 0){
		/* full duplex */
		maccr |= FTMAC110_MACCR_FULLDUP;
        printf("=>full duplex ");
	}
	else{
		/* half duplex */
		maccr &= ~(FTMAC110_MACCR_FULLDUP);
        printf("=>half duplex ");
	}

	if((value & FTPHY_REG_CR_SPEED) > 0){
		/* 100 */
		maccr |= FTMAC110_MACCR_MDC_SEL;
        printf(" 100Mhz\n");
	}
	else{
		/* 10 */
		maccr &= ~(FTMAC110_MACCR_MDC_SEL);
        printf(" 10Mhz\n");
	}
	iowrite32(maccr, FTMAC110_OFFSET_MACCR);
    printf(" FTMAC110_OFFSET_MACCR= %x \n",maccr);
	gPHY_ready = 1; //20170915 Sean: phy ready.

#if (PHY_FORCE_NEGOTIATE_SUPPORT == 1)
    if((value & FTPHY_REG_CR_FULL) > 0)
        ret&= ~(1<<1);//FULL
    else
        ret|=1<<1;//HALF
    if((value & FTPHY_REG_CR_SPEED) > 0)
        ret&= ~(1<<0);//100
    else
        ret|=1<<0;//10
    return ret;
#else
	return (1);
#endif
}

UINT16 phyread16(UINT16 regad)
{
    UINT32 tmp;
    int		err_cnt=0;
	/* fill out PHYCR */
	iowrite32(
				FTMAC110_PHYCR_PHYAD(FTPHY_ADD) |
				FTMAC110_PHYCR_REGAD(regad) |
				FTMAC110_PHYCR_MIIRD
				,FTMAC110_OFFSET_PHYCR);
	/* wait for read complete */
	while(ioread32(FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIRD)
	{
		err_cnt++;
		if(err_cnt >= 20)
			break;
		OSTimeDly(1);
	}
    tmp=(ioread32( FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIRDATA);
  //  printf("Phy addr 0x%x=0x%x\n",regad,tmp);
	return tmp;
}

void phywrite16(UINT16 value, UINT16 regad)
{
    int		err_cnt=0;

	/* write data to PHYWDATA (0x94h) */
	iowrite32(value, FTMAC110_OFFSET_PHYWDATA);
	/* wait */
	while(ioread32(FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIWR)
	{
		err_cnt++;
		if(err_cnt >= 20)
			break;
		OSTimeDly(1);
	}
	/* fille out PHYCR */
	iowrite32(
				FTMAC110_PHYCR_PHYAD(FTPHY_ADD) |
				FTMAC110_PHYCR_REGAD(regad) |
				FTMAC110_PHYCR_MIIWR
				, FTMAC110_OFFSET_PHYCR);
	/* wait */
	while(ioread32(FTMAC110_OFFSET_PHYCR) & FTMAC110_PHYCR_MIIWR)
	{
		err_cnt++;
		if(err_cnt >= 20)
			break;
		OSTimeDly(1);
	}
}
void dump_phy_regs(void)
{

u16 value;

        value = phyread16(0);
        printf("PHY_00= %x\n", value);
        value = phyread16(1);
        printf("PHY_01= %x\n", value);
        value = phyread16(2);
        printf("PHY_02= %x\n", value);
        value = phyread16(3);
        printf("PHY_03= %x\n", value);
        value = phyread16(4);
        printf("PHY_04= %x\n", value);
        value = phyread16(5);
        printf("PHY_05= %x\n", value);
        value = phyread16(6);
        printf("PHY_06= %x\n", value);
        value = phyread16(16);
        printf("PHY_16= %x\n", value);
        value = phyread16(17);
        printf("PHY_17= %x\n", value);
        value = phyread16(18);
        printf("PHY_18= %x\n", value);
        value = phyread16(30);
        printf("PHY_30= %x\n", value);

}

#if 1
void ftmac110_init()
{
    UINT32 _maccr,mac_addr;
    UINT16 value,id1,id2;

    //printf("ftmac110_init start\n");
    // phyad = FTPHY_ADD;
   	/* reset mac */
	ftmac110_reset_mac();
    /* reset phy */
	ftmac110_reset_phy();

#if 1
    do{
        value = phyread16(FTPHY_REG_ID1);
        id1=value;
        //printf("ftmacPhy_id1 = %x\n",value); //
        value = phyread16(FTPHY_REG_ID2);
        id2=value;
        //printf("ftmacPhy_id2 = %x\n",value);
    }while((id1!=0x0243)&&(id2!=0x0c54));
    value = phyread16(FTPHY_REG_CONTROL);
    //printf("FTPHY_REG_CONTROL = %x\n",value);
    value = phyread16(FTPHY_REG_STATUS);
    //printf("FTPHY_REG_STATUS = %x\n",value);
    value = phyread16(FTPHY_REG_AUTO_NEG);
    //printf("FTPHY_REG_AUTO_NEG = %x\n",value);
#endif

//    dump_phy_regs();

    /* initial ftmac110 control */
    txdes_num             = TX_QUEUE_ENTRIES;
    rxdes_num             = RX_QUEUE_ENTRIES;
    wakeup_frame_mask[0]  = 0xffffffff;
    wakeup_frame_mask[1]  = 0xffffffff;
    wakeup_frame_mask[2]  = 0xffffffff;
    wakeup_frame_mask[3]  = 0xffffffff;
	/* initial PHY address */


    _maccr = FTMAC110_MACCR_DEFAULT;

	/* reset */
	iowrite32(FTMAC110_MACCR_SW_RST,  FTMAC110_OFFSET_MACCR);

	/* initial descriptor */
	alloc_txdes();
	rxbuf_size = RX_BUF_SIZE;
	alloc_rxdes();
	/* initial interrupt */
	//ftmac110_control->rx_isr  = reception_isr;
	//initial_interrupt();
	/* set tx rx ring base address */
	iowrite32((UINT32)TX_DESC_ADDR,  FTMAC110_OFFSET_TXR_BADR);
    printf("FTMAC110_OFFSET_TXR_BADR = %x\n",ioread32(FTMAC110_OFFSET_TXR_BADR));
	iowrite32((UINT32)RX_DESC_ADDR,  FTMAC110_OFFSET_RXR_BADR);
	/* set interrupt mask */
	ioread32(FTMAC110_OFFSET_IMR);
    #if 1
		iowrite32(0, FTMAC110_OFFSET_IMR);
		OSTimeDly(1);
		/*iowrite32(
				FTMAC110_INT_RPKT_FINISH | FTMAC110_INT_NORXBUF   |
				FTMAC110_INT_XPKT_OK     | FTMAC110_INT_XPKT_LOST |
				FTMAC110_INT_RPKT_LOST   | FTMAC110_INT_AHB_ERR   |
				FTMAC110_INT_PHYSTS_CHG
				,FTMAC110_OFFSET_IMR);
		*/
		iowrite32(
				0xffff
				,FTMAC110_OFFSET_IMR);
    #else
   		iowrite32(0, FTMAC110_OFFSET_IMR);
    #endif

	/* set mac address */
	MAC_ADDR[0]=uiMACAddr[0];
    MAC_ADDR[1]=uiMACAddr[1];
    MAC_ADDR[2]=uiMACAddr[2];
    MAC_ADDR[3]=uiMACAddr[3];
    MAC_ADDR[4]=uiMACAddr[4];
    MAC_ADDR[5]=uiMACAddr[5];
    printf("SetLwIP_my_hwaddr: %2x:%2x:%2x:%2x:%2x:%2x\n",MAC_ADDR[0],MAC_ADDR[1],MAC_ADDR[2],MAC_ADDR[3],MAC_ADDR[4],MAC_ADDR[5]);
    mac_addr=(MAC_ADDR[2]<<24)|(MAC_ADDR[3]<<16)|(MAC_ADDR[4]<<8)|MAC_ADDR[5];
	printf("mac_addr = %x\n",mac_addr);

	iowrite32(mac_addr, FTMAC110_OFFSET_MAC_LADR);
    mac_addr=(MAC_ADDR[0]<<8)|MAC_ADDR[1];
    printf("mac_addr = %x\n",mac_addr);
	iowrite32(mac_addr & 0xffff,  FTMAC110_OFFSET_MAC_MADR);
	/* set multicast table not finished */

	/* set interrupt timer control register */
	iowrite32(FTMAC110_ITC_DEFAULT,  FTMAC110_OFFSET_ITC);
	/* set auto polling timer control register */
	iowrite32(FTMAC110_APTC_DEFAULT,  FTMAC110_OFFSET_APTC);
	/* set burst length */
	iowrite32(FTMAC110_DBLAC_DEFAULT, FTMAC110_OFFSET_DBLAC);

	/* processing PHY, set full/half duplex, 10/100 Mbps, using auto-negotiation */
	//change_duplex_speed(FULL, _100);
	//phy_processing();

	_maccr |= ioread32( FTMAC110_OFFSET_MACCR);
	//* set MAC control register */
	iowrite32(_maccr,  FTMAC110_OFFSET_MACCR);
	//printf("ftmac110_init end\n");
	/*ENABLE_FTMAC110_INT_PHYSTS_CHG*/
	//phywrite16(0x8000,FTPHY_REG_CONTROL_TEST2);
}
#endif
/*

void FTMAC110_Green_Init(void)
{
	//1.
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG,0x81E1);
	//2.
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_CONTROL, 0x3300);
	//3.
	while(0 == (FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_EXP) & FTPHY_PAGE_RX));
	FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_LINK_PART_ABILITY);
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT, 0xA80A);
	while(0 == (FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_EXP) & FTPHY_PAGE_RX));
	FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_LINK_PART_ABILITY);
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT, 0xA80A);
	//4.
	while(0 == (FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_EXP) & FTPHY_PAGE_RX));
	FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT_ABILITY);
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT, 0x0002);
	while(0 == (FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_EXP) & FTPHY_PAGE_RX));
	FTMAC110_PHY_Read(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT_ABILITY);
	FTMAC110_PHY_Write(FTMAC110DEV.PHY_Address,FTPHY_REG_AUTO_NEG_NEXT, 0x0002);
	//5.
	FTMAC110_DelaySecs(100);


}*/

#if 0
__align(16) u8 data[MAX_PKT_SIZE]= {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x50,0x7f,0xa7,0x26,0x30,0x08,0x06,0x00,0x01
                               ,0x08,0x00,0x06,0x04,0x00,0x01,0x00,0x50,0x7f,0xa7,0x26,0x30,0xc0,0xa8,0x01,0x01
                               ,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xa8,0x01,0x16,0x3f,0x06,0xb9,0x0a,0x3b,0x7c
                               ,0xaa,0x45,0xc0,0xa8,0x01,0x26,0x00,0x16,0xc0,0x8e,0x79,0x8f,0x00,0x00,0x00,0x00};
#endif
#if 0
void FTMAC110_Transmission(void)
{
    UINT8 SRC[6] = {0x6c,0xf0,0x49,0x11,0x22,0x33};
    UINT8  DST[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
	//UINT32 timeout = 10000, timecnt = 0;
	UINT8* buff ;
    UINT32 buff32;
	UINT32 i;


   // buff32 =(u32*) data;
    buff = (u8*)data;
	/* initial transmission buff */
	memcpy(data, DST, 6);
	memcpy(data + 6, SRC, 6);
	for(i = 64;i < MAX_PKT_SIZE; i++){
	//	buff[i] = i & 0xFF;
	//}
    //for(i = 0;i < MAX_PKT_SIZE; i++){
		buff[i] = 0x1b;
	}
    printf("data=%x\n",data);
    printf("---Test 1. Normal frame transmission.-----------------------------------------------\n");
	//ftmac110_xmit(buff32, MAX_PKT_SIZE);
    ftmac110_xmit((u32) buff, 60);
    ftmac110_delaysecs(1);
	ftmac110_xmit( (u32) buff, 61);
    ftmac110_delaysecs(1);
	ftmac110_xmit((u32) buff, 62);
	ftmac110_xmit( (u32) buff, 63);

}
#endif

void T_LwIPEntry(void *pevent);


void FTMAC110_main(void)
{
	UINT32 version;

    /* Create the semaphore */
    //MACSemDMAComplete = OSSemCreate(0);
//    MACSemRX_VALID = OSSemCreate(1);
  //  MACSemTX_REQ = OSSemCreate(0);
#if (CHIP_OPTION == CHIP_A1018A)
 #if 1 //_2en
   SYS_CTL0_EXT |= SYS_CTL0_EXT_MAC2_CKEN;
   SYSReset_EXT(SYS_CTL0_EXT_MAC2_RST);
   GpioActFlashSelect |= CHIP_IO_RMII_EN4; //_2en
 #else //en2 en3
     SYS_CTL0 |= SYS_CTL0_MAC_CKEN;
     SYSReset(SYS_RSTCTL_GPI_RST);
     GpioActFlashSelect = CHIP_IO_RMII_EN2;
 #endif
#elif( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
   SYS_CTL0 |= SYS_CTL0_MAC_CKEN;
   SYSReset(SYS_RSTCTL_GPI_RST);
   GpioActFlashSelect |= CHIP_IO_RMII_EN;
#elif (HW_BOARD_OPTION== MR9600_RX_DB_ETH)
   SYS_CTL0 |= SYS_CTL0_MAC_CKEN;
   SYSReset(SYS_RSTCTL_GPI_RST);
   GpioActFlashSelect |= CHIP_IO_RMII_EN4;
#else
   SYS_CTL0 |= SYS_CTL0_MAC_CKEN;
   SYSReset(SYS_RSTCTL_GPI_RST);
   GpioActFlashSelect |= CHIP_IO_RMII_EN3;
#endif
    //printf("GpioActFlashSelect=%x \n",GpioActFlashSelect);

  // gpioSetLevel(0,1,0);
  //  for(i=0;i<0x00fff;i++);	/* delay for power stable */
  //  gpioSetLevel(0,1,1);
#if 1 //ETH PHY debug
	//printf("FTMAC110_main\n");

    //printf("SYS_CHIP_IO_CFG 0x%08x\n",GpioActFlashSelect);
    //printf("SYS_CHIP_IO_CFG2 0x%08x\n",SYS_CHIP_IO_CFG2);
    //printf("\n Gpio1Ena = 0x%08x\n",Gpio1Ena);
    //printf("\n Gpio2Ena = 0x%08x\n",Gpio2Ena);
    //printf("SYS_CTL0_EXT=0x%08x\n",SYS_CTL0_EXT);
	version = ftmac110_fpga_version();
    printf("FTMAC110_version() %x\n",version);
#endif
	ftmac110_init();
   // FTMAC110_Transmission();

   OSTaskCreate(T_LwIPEntry, (void*)NULL, NIC_TASK_STACK, T_LWIPENTRY_PRIOR);

  //  printf("FTMAC110_main end\n");
}
#endif
