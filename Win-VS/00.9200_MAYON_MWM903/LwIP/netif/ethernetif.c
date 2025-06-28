/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *  
 * Author: 
 *   
 */  
/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "general.h"
#include "../gpi/inc/net.h"
#include "arch/cc.h"
#include "netif/Dm9000.h"
#include "netif/etharp.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/lwipsys.h"
#include "ucos_ii.h"

#include "task.h"

#include "../gpi/inc/gpi.h"
#include "netif/Ftmac110.h"
#include "UIapi.h"
//#include "EMAC.h"
#if(NIC_SUPPORT == 1) 
//#define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
//#define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
//#define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
//#define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */

#define SPEED_10				10
#define SPEED_100				100

/* Duplex, half or full. */
#define DUPLEX_HALF				0x00
#define DUPLEX_FULL				0x01
 
#define MTU  1500

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

u8_t MAC_ADDR[6] = {0x6c,0xf0,0x49,0x11,0x22,0x68};
//u8_t MAC_ADDR[6] = {0x08,0x08,0x08,0x08,0x08,0x08};
u8_t MAC_DST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
u8_t TerminatedHeader[6] = {0x00,0x0D, 0xF0, 0xD0, 0x0F, 0xD0};//

__align(4)  u8_t  RxBufs[1600];			/* 接收緩衝區 */
__align(4)  u8_t  TxBufs[1600];			/* 發送緩衝區static volatile    */
 struct pbuf  _gpi_Rx_Pbuf;
  
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};
enum DM9KS_PHY_mode 
    {	DM9KS_10MHD   = 0,
        DM9KS_100MHD  = 1,
        DM9KS_10MFD   = 4,
        DM9KS_100MFD  = 5,
        DM9KS_AUTO    = 8,
        };
//static OS_EVENT*  hBlockOutput; //互斥信號量。   用於發送緩衝區的使用權。
OS_EVENT* hEthernetInput;
OS_EVENT* hEthernetOutput;
//extern OS_EVENT *MACSemRX_VALID;      /* Semaphore to  RX interrupt. */
extern OS_EVENT *MACSemTX_REQ;      /* Semaphore to tr interrupt. */
u8 renewIP=1;

extern u8_t my_hwaddr[6];
extern u8 LwipISStatic;
static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

//* 接收線程堆棧
#define	T_ETHERNETIF_INPUT_STKSIZE		2048  //接收中斷給ethernetif_input任務發送信號，而不直接調用ethernetif_input。
OS_STK  T_ETHERNETIF_INPUT_STK[T_ETHERNETIF_INPUT_STKSIZE];

/* Forward declarations. */
static void ethernetif_input(void *pReserved); 

static err_t ethernetif_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);
//extern void ftmac110_xmit(u32_t, u32_t );
//extern void EMACInit(void);  
extern u16 phyread16(u16_t);
extern void phywrite16(u16_t , u16_t );
extern u8_t rx_buff[MAX_PKT_SIZE];
extern u32_t rx_len;

extern ftmac110_txdes *txdes_top;
extern ftmac110_txdes *txdes_cur;
extern ftmac110_txdes *txdes_end;
extern UINT32 txdes_num;

extern ftmac110_rxdes *rxdes_top;
extern ftmac110_rxdes *rxdes_cur;
extern ftmac110_rxdes *rxdes_end;
extern UINT32 rxdes_num;

void
low_level_init(struct netif *pstNetif)
{
 // struct ethernetif *ethernetif = pstNeti->state;
  
  /* set MAC hardware address length */
  pstNetif->hwaddr_len = 6;
  //pstNetif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
  /* set MAC hardware address */
printf("low_level_init\n");   
	pstNetif->hwaddr[0] = MAC_ADDR[0];
	pstNetif->hwaddr[1] = MAC_ADDR[1]; 
	pstNetif->hwaddr[2] = MAC_ADDR[2]; 
	pstNetif->hwaddr[3] = MAC_ADDR[3];
	pstNetif->hwaddr[4] = MAC_ADDR[4];
	pstNetif->hwaddr[5] = MAC_ADDR[5];
	
  /* maximum transfer unit */
  pstNetif->mtu = MTU;
  /* broadcast capability */
  pstNetif->flags = NETIF_FLAG_BROADCAST;
  hEthernetInput = OSSemCreate(0);
  hEthernetOutput= OSSemCreate(1);
//  printf("\n ethernetif_input Task start\n");
	OSTaskCreate(ethernetif_input, pstNetif, &T_ETHERNETIF_INPUT_STK[T_ETHERNETIF_INPUT_STKSIZE-1], T_ETHERNETIF_INPUT_PRIO);
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */ 

static err_t
low_level_output(struct netif *pstNetif, struct pbuf *pstPbuf) //為簡化設計。每次發送最多佔用一個發送緩衝區。
{	

  	u32_t intstat ;
	u8_t TxProduceIndex;//,TxConsumeIndex;
	u8_t	__u8Err;
  	u8_t *__from_addr,*__to_addr;
  	u16_t i=0;
  
  	struct pbuf *__pstSendPbuf ;
	u16_t __totallen=0; 
	err_t __errReturn = ERR_OK;
  	u32_t total_len = 0;
 	OSSemPend(hEthernetOutput, OS_IPC_WAIT_FOREVER, &__u8Err); 
	__pstSendPbuf = pstPbuf;
	//* 阻塞對EMAC的訪問，以避免不同的任務同時訪問EMAC造成訪問衝突的問題，最長等待時間是2秒
	//  OSSemPend(hBlockOutput, 400, &__u8Err);
	if(__u8Err==OS_NO_ERR  )
	{
		for(; __pstSendPbuf!=NULL; __pstSendPbuf=__pstSendPbuf->next)
		{
			__from_addr=(u8_t*)__pstSendPbuf->payload;
          //  printf( "__from_addr:  %p \n",__from_addr);
//printf( "____len:  %d \n",__pstSendPbuf->len);
   		 	memcpy( TxBufs + total_len,__from_addr, __pstSendPbuf->len);
          	total_len +=  __pstSendPbuf->len;
			//printf( "len:  %d \n",total_len);	
   		}			                               
     	if(total_len<60) 
		{
			//printf("len<60.\n");		
			total_len=60;
			// printf( "total_len:  %d \n",total_len);
       	}
		// printf( "total_len:  %d \n",total_len);
		#if 0 //buffer chkeck data

		for (i=0 ;i<total_len;i++)
	   	{
   		    if((i%16)==0)DEBUG_GPIU("T_[%.3x] =",i);
		
            DEBUG_GPIU(" %.2x",TxBufs[i]);
		
			if((i%16)==15) 	DEBUG_GPIU("\n");
        
   		}
    	DEBUG_GPIU("\n");
		#endif
		ftmac110_xmit((u32)TxBufs, total_len);
        total_len = 0;
		TxProduceIndex++;		/* transmit now */
		OSSemPost(hEthernetOutput);		
	}
	else 
	{
		printf("no send.\n");
		OSSemPost(hEthernetOutput);
	}	
	OSSemPost(hEthernetOutput);
	
    return __errReturn;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
//extern u16_t GetInputPacketLen(void);
//extern void EMACReadPacket(struct pbuf *__Pstbuf ,u16_t Rlen);
static struct pbuf *low_level_input(struct netif *netif)
{

	    u32_t RxConsumeIndex;
	    u32_t *rx_status_addr;
	    u16_t __u16Len;
	    u8_t  * REC_BUFF_PTR;		
	    u8_t  *data_start_addr;
	    u32_t *__pp;
	    struct pbuf  *__pstPbuf,   *__pstCurPbuf ;
	    u32_t  i;
	    u16_t	__u16LenToRead ,__u16RemainLenToRead;						//* 還剩多少字節沒有讀取到pbuf中
		u16_t RxStatus, temp_len, more_len;
        u8_t  regs,status,*rdptr,rxbyte,__u8Err;		    
	    __pstPbuf = NULL;
		
		OSSemPend(hEthernetOutput, OS_IPC_WAIT_FOREVER, &__u8Err);       
    	if(__u8Err==OS_NO_ERR  )
		{
			rdptr=__pstPbuf->payload;
       		__u16Len = rx_len;

        //if((__u16Len<60)||(__u16Len>MTU)) 
        if((__u16Len<60)||(__u16Len>MAX_PKT_SIZE)) 
	    {
		    OSSemPost(hEthernetOutput);
	    	DEBUG_GPIU(" RX Len:%d(%x)\n", __u16Len, RxStatus);
	   		DEBUG_GPIU("__u16Len>MTU \n");
	    	//return __pstPbuf;
	    	pbuf_free(__pstPbuf);
	    	return NULL;
	    }
		//* 從pbuf pool中獲取一個pbuf鏈
		__pstPbuf = pbuf_alloc(PBUF_RAW, __u16Len, PBUF_POOL);
		if(__pstPbuf != NULL)
		{
			//* 複製數據
			__pstCurPbuf=__pstPbuf;
			__u16RemainLenToRead=__u16Len;

			while(__u16RemainLenToRead >0)	/*When the size of received packet large than PBUF_POOL_BUFSIZE */
			{
				REC_BUFF_PTR=(u8_t *)__pstCurPbuf->payload;   
				if(__u16RemainLenToRead>__pstCurPbuf->len)	/*將資料分放到兩個pbuf,但是封包若大於PBUF_POOL_BUFSIZE*2,則會出現錯誤.*/
	    		{	    	
    		
	      	    	__u16LenToRead=__pstCurPbuf->len;
		      	    __u16RemainLenToRead-=__pstCurPbuf->len;
    	            memcpy(REC_BUFF_PTR,RxBufs+2,__u16LenToRead);	
					
					__pstCurPbuf=__pstPbuf->next;
					REC_BUFF_PTR=(u8_t *)__pstCurPbuf->payload; 
					__u16LenToRead=__u16RemainLenToRead;
					__u16RemainLenToRead=0;			
					memcpy(REC_BUFF_PTR,RxBufs+2+PBUF_POOL_BUFSIZE,__u16LenToRead);
		        }
		        else 
		    	{	
		    	
		    	    __u16LenToRead=__u16RemainLenToRead;
		    	    __u16RemainLenToRead=0;
	                 memcpy(REC_BUFF_PTR,RxBufs+2,__u16LenToRead);
	
		        }
				if(__u16RemainLenToRead >0)  
				{
			
					 __pstCurPbuf=__pstCurPbuf->next;
					 if(__pstCurPbuf==NULL)  
					 {
					 	__u16RemainLenToRead =0;
					 }	
				}			
			} 
			OSSemPost(hEthernetOutput);
		}
		else
		{
			DEBUG_GPIU("NO SPACE\n");
			OSSemPost(hEthernetOutput);
		}
		OSSemPost(hEthernetOutput);
    }
	else
	{
		printf("=========abort RX============\n");
		return NULL;
    }
	OSSemPost(hEthernetOutput);
//    OSSemPost(MACSemRX_VALID);
	return __pstPbuf;

}

/*
 * ethernetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t
ethernetif_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
  
 /* resolve hardware address, then send (or queue) packet */
  return etharp_output(netif, ipaddr, p);
 
}
 
/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
static void
ethernetif_input(void *pReserved)
{
	struct ethernetif		*__pstEthernetif;
	struct pbuf				*__pstPbuf;
	struct eth_hdr			*__pstEthhdr;
	struct netif			*__pstNetif;
	INT8U					__u8RtnVal;
	
	__pstNetif = (struct netif*)pReserved;			



	while(TRUE)
	{	
		OSSemPend(hEthernetInput, OS_IPC_WAIT_FOREVER, &__u8RtnVal);/*檢查是否有data comming*/
		rpkt_finish_handler();
		while(rx_len>0)
		{	
			//* 從EMAC循環讀取數據		
			__pstPbuf = low_level_input(__pstNetif); //=eth_rx();
#if 0			 
{
UINT8  *copy_buf = NULL;	
u32 i;
u8_t  TmpBufs[100];	
memset(TmpBufs, 0, sizeof(TmpBufs));
copy_buf = TmpBufs;
memcpy((void *)(copy_buf), (void *)__pstPbuf->payload, 50);
		    for (i=0 ;i<48;i++)
		   //	 for (i=0 ;i<16;i++)
	   	    {
	   	        if((i%16)==0)
					printf("R_[%.3x] =",i);
			
                printf(" %.2x",copy_buf[i+2]);
			
			    if((i%16)==15) 	printf("\n");

	   	    }
        	printf("\n");
 }		
#endif
			if(__pstPbuf == NULL)
			{ 		
			printf("pstPbuf = NULL\n");
				break; 
			}		
	
			__pstEthernetif = (struct ethernetif*)__pstNetif->state;
			__pstEthhdr = __pstPbuf->payload;
	
			switch(htons(__pstEthhdr->type))
			{
				case ETHTYPE_IP:					
					//* 更新ARP表				
					etharp_ip_input(__pstNetif, __pstPbuf);
					//* 跳過以太網頭部字段 				
					pbuf_header(__pstPbuf, -sizeof(struct eth_hdr) );				 					
					
					//* 傳遞到網絡層
					__pstNetif->input(__pstPbuf, __pstNetif);
					
				break;
		
				case ETHTYPE_ARP:					
					//* 將__pstPbuf傳遞到ARP模塊											
					etharp_arp_input(__pstNetif, __pstEthernetif->ethaddr, __pstPbuf);
				break;

				default:
					
					pbuf_free(__pstPbuf);					
					__pstPbuf = NULL;
				break;
			}		
		rpkt_finish_handler();	
//printf("ETH L=%d\n",rx_len);		
		}
	}
}

static void 
arp_timer(void *arg) 
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/*
 * ethernetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */    
err_t  
ethernetif_init(struct netif *netif) 
{
  INT8U	__u8Err;
  static struct ethernetif ethernetif;
  
  netif->state = &ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = ethernetif_output;
  netif->linkoutput = low_level_output;
  
 // hBlockOutput = OSMutexCreate(MUTEX_EMAC_SEND_PIP, &__u8Err); //建立互斥信號量,為發送時多任務搶佔緩衝區使用。
  
  //hBlockOutput = OSSemCreate(1);
  
  ethernetif.ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);

  etharp_init();

  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);

  return ERR_OK;
}


/******************************************************************************
** Function name:		EMAC_TxEnable/EMAC_TxDisable
**
** Descriptions:		EMAC TX API modules
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
__inline void EMAC_TxEnable( void )
{

}

__inline void EMAC_TxDisable( void )
{

}

/******************************************************************************
** Function name:		EMAC_RxEnable/EMAC_RxDisable
**
** Descriptions:		EMAC RX API modules
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
__inline void EMAC_RxEnable( void )
{
   
}

__inline void EMAC_RxDisable( void )
{

}


void Write_PHY (u16_t phyadd,int PhyReg, int Value)
{

}
/*****************************************************************************
** Function name:		ReadPHY
**
** Descriptions:		Read data from the PHY port
**
** parameters:			PHY register
** Returned value:		PHY data
** 
*****************************************************************************/

unsigned short Read_PHY ( u16_t phyadd ,unsigned char PhyReg) 
{

}
/*****************************************************************************
** Function name:		EMACTxDesciptorInit
**
** Descriptions:		initialize EMAC TX descriptor table
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void EMACTxDescriptorInit( void )
{

}

/*****************************************************************************
** Function name:		EMACRxDesciptorInit
**
** Descriptions:		initialize EMAC RX descriptor table
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void EMACRxDescriptorInit( void ) 
{

}
 extern u8 EnableStreaming;

void Ethernet_Exception(void)  //網口接收中斷服務程序。
{
u8_t status,masks,rxbyte;

	{
	    OSSemPost(hEthernetInput);
		return;
	}
	
	if((status & ISR_PTS)&&(masks & IMR_PTM))
	{
	  //  gpioSetLevel(1, 6, 1);
	 //   OSSemPost(DMiSemTran);
		DM9000_iow(DM9000_ISR,ISR_PTS);
     //  	 DEBUG_GPIU(" S_\n");
    //     DM9000_iow(DM9000_IMR, IMR_PAR);
	}
}
static void transmission_isr()
{
printf("->T:\n");
//OSSemPost(DMiSemTran);
}

void reception_isr()
{

	/* plus 2, because of zero copy*/
	
	if(0 == ftmac110_addrcmp(MAC_ADDR, RxBufs + 2)){
		// OSSemPost(MACSemRX_VALID);
         printf("R2");
	}
	
}

#if 0
/* Set PHY operationg mode*/
 void set_PHY_mode(enum DM9KS_PHY_mode op_mode)
{
  u16 phy_reg0 = 0x1200;		/* Auto-negotiation & Restart Auto-negotiation */	
  u16 phy_reg4 = 0x01e1;		/* Default flow control disable*/
  u16 i,sta,lnk;

  		dm9000_reset();
    if ( !(op_mode & DM9KS_AUTO) ) // op_mode didn't auto sense */
    { 		switch(op_mode) 
    {			case DM9KS_10MHD:
                   phy_reg4 = 0x21;
                   phy_reg0 = 0x1000;
                   break;
                case DM9KS_10MFD:
                    phy_reg4 = 0x41;
                    phy_reg0 = 0x1100;
                    break;			
                case DM9KS_100MHD: 
                    phy_reg4 = 0x81;
                    phy_reg0 = 0x3000;
                    break;			
                case DM9KS_100MFD:
                    phy_reg4 = 0x101;
                    phy_reg0 = 0x3100;
                    break;
               case DM9KS_AUTOMF:
                   phy_write( 0, 0x2100);
                   phy_write( 0x14,0xb0);  //14H=B0 (disable auto-MDIX
                   printf("----------- REG0 0x%x\n",phy_read( 0 ) );
                   printf("----------- REG20 0x%x\n", phy_read( 0x14 ));
               while (1);
          
                    break;
                 default:
                        break;
    } // end of switch
   	} // e

if (op_mode == DM9KS_AUTOMF) return;


    phy_write( 4, phy_reg4);
    phy_write( 0, phy_reg0|0x1200);
       //Wait internal PHY link ready   
         
        DM9000_iow(0x38,0x6B ); 	
        DM9000_iow(DM9000_NCR, 0); 
        DM9000_iow(DM9000_TCR, 0x1E);      
        DM9000_iow(DM9000_BPTR, 0x3f);/* Less 3kb, 600us */  
        DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));  
        DM9000_iow(DM9000_FCR, 0x0);   
        DM9000_iow(DM9000_SMCR, 0); /* Special Mode */   
        DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END); /* clear TX status */  
        DM9000_iow(DM9000_ISR, 0x3f);/* Clear interrupt status */  
		DM9000_iow(DM9000_CSCR, CSCR_UDPCSE|CSCR_TCPCSE|CSCR_IPCSE);/* Enable TCP/IP/UDP Check Sum */ 

        for (i = 0, sta = 0x10; i < 6; i++, sta++)
                DM9000_iow(sta, my_hwaddr[i]);
        for (i = 0, sta = 0x16; i < 8; i++, sta++)
                DM9000_iow(sta, 0xff);

      
        DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);  /* RX enable */
 //       DM9000_iow(DM9000_IMR, IMR_PAR);        
      DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_LNKCHNG);        /* Enable RX interrupt mask */      
    i = 150;   
    while(--i)   
    {   
        udelay(0x1000);   
        printf(" Wait READY");
        sta=DM9000_ior(DM9000_NSR);
        
        if(sta && 0x40)
            {
              printf("Link ok\n ");
               break;  
            }
        else
            printf("Link fail\n ");
    } 
  }
void EMACInit()  
{   
int i, oft;
u8_t IO_mode;
u16_t  lnk;
		DEBUG_GPIU("EMACInit\n"); 
        if(dm9000_probe()== FALSE)
            {
             while(dm9000_probe()== FALSE)
                {
                   DEBUG_GPIU(" DM9000 Init Fail\n");
		  		  //  return ;
 	  		      }
 	  		}
		/* set the internal PHY power-on */
        DM9000_iow(DM9000_GPR, 0x00);   
		dm9000_reset();
 
 		IO_mode = DM9000_ior(DM9000_ISR)&0x80;

		if(IO_mode)
			DEBUG_GPIU("8 bit mode\n"); 
		else
			DEBUG_GPIU("16 bit mode\n");
		/* Set PHY */
        #if 0 //force mode
        set_PHY_mode(DM9KS_100MHD);
        #else            
		phy_write(4, 0x01e1);
		phy_write(0, 0x1200); /* N-way */
		#endif
        
	    DM9000_iow(0x38,0x60 ); 	
        DM9000_iow(DM9000_NCR, 0); 
        DM9000_iow(DM9000_TCR, 0x1E);      
        DM9000_iow(DM9000_BPTR, 0x3f);/* Less 3kb, 600us */  
        DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));  
        DM9000_iow(DM9000_FCR, 0x0);   
        DM9000_iow(DM9000_SMCR, 0); /* Special Mode */   
        DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END); /* clear TX status */  
        DM9000_iow(DM9000_ISR, 0x3f);/* Clear interrupt status */  
		DM9000_iow(DM9000_CSCR, CSCR_UDPCSE|CSCR_TCPCSE|CSCR_IPCSE);/* Enable TCP/IP/UDP Check Sum */ 

        for (i = 0, oft = 0x10; i < 6; i++, oft++)
                DM9000_iow(oft, my_hwaddr[i]);
        for (i = 0, oft = 0x16; i < 8; i++, oft++)
                DM9000_iow(oft, 0xff);

      
        DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);  /* RX enable */
 //       DM9000_iow(DM9000_IMR, IMR_PAR);        
      DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM);        /* Enable RX interrupt mask */
 //    DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_PTM);        /* Enable TX/RX interrupt mask */
#if 1
        /* see what we've got */
	
        lnk = phy_read(17) >> 12;
        printf("operating at ");
        switch (lnk) {
        case 1:
                printf("10M half duplex \n");
                break;
        case 2:
                printf("10M full duplex \n");
                break;
        case 4:
                printf("100M half duplex \n");
                break;
        case 8:
                printf("100M full duplex\n ");
                break;
        default:
                printf("unknown: %x \n ", lnk);
                break;
        }
		/*
        i= DM9000_ior(DM9000_NSR);
        if(i && 0x40)
        {
            printf("Link ok\n ");
			net_link_status=0;
        }	
        else
        {
            printf("Link fail\n ");
			net_link_status=1;
        }
        */
        EMAC_link_status();
        if(i && 0x80)
            printf("10Mbps \n ");
        else
            printf("100Mbps\n ");

         i= DM9000_ior(DM9000_NCR);
        if(i && 0x08)
            printf("FUll duplex\n ");
        else
            printf("half duplex\n ");
        //printf("mode\n");
#endif
//    	hEthernetInput = OSSemCreate(0);  
	    DMiSemTran= OSSemCreate(0);
       //DEBUG_GPIU("operating at 100M full duplex mode\n");
      
}
#endif
#endif