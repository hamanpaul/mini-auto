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
 #include "sysopt.h"
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

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

//aher
#include "../gpi/inc/gpi.h"
#include "board.h"
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3)|| (UI_FW_UPGRADE_ICON_ENABLE == 1))
	#include "UIapi.h"
#endif
//#include "EMAC.h"

//#define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
//#define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
//#define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
//#define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */

#define	RX_over_Protect			0		//Sean:20170810 add.

#define SPEED_10				10
#define SPEED_100				100

/* Duplex, half or full. */
#define DUPLEX_HALF				0x00
#define DUPLEX_FULL				0x01
 
#define MTU  1500
//#define FLOW_CTRL

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

__align(4)  u8_t  gpi_RxBufs[1600];			// 接收緩衝區
__align(4)  u8_t  gpi_TxBufs[1600];			// 發送緩衝區static volatile   
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
        DM9KS_AUTOMF  = 7, //dm9000c
        DM9KS_AUTO    = 8
        };
//static OS_EVENT*  hBlockOutput; //互斥信號量。   用於發送緩衝區的使用權。
OS_EVENT* hEthernetInput;
OS_EVENT* hdm9000b;
u8 renewIP=0;
extern OS_EVENT *DMiSemTran;      /* Semaphore to tr interrupt. */
extern OS_EVENT* Wait_Link_Status_Evt;    /* 等待網路線連線狀態 */
extern u8_t my_hwaddr[6];
extern LwipISStatic;
#if NIC_SUPPORT
extern u8 Reminder_FW_Upgrade;
#endif
//static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};
int net_link_status;


//* 接收線程堆棧
#define	T_ETHERNETIF_INPUT_STKSIZE		2048  //接收中斷給ethernetif_input任務發送信號，而不直接調用ethernetif_input。
OS_STK  T_ETHERNETIF_INPUT_STK[T_ETHERNETIF_INPUT_STKSIZE];

/* Forward declarations. */
static void ethernetif_input(void *pReserved); 

static err_t ethernetif_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);

extern void EMACInit(void);  
extern u16 phy_read(u16_t);
extern phy_write(int reg, u16_t value);
static void EMAC_link_status();
void
low_level_init(struct netif *pstNetif)
{
 // struct ethernetif *ethernetif = pstNeti->state;
  
  /* set MAC hardware address length */
  pstNetif->hwaddr_len = 6;
  //pstNetif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
  /* set MAC hardware address */
	pstNetif->hwaddr[0] = my_hwaddr[0];
	pstNetif->hwaddr[1] = my_hwaddr[1]; 
	pstNetif->hwaddr[2] = my_hwaddr[2]; 
	pstNetif->hwaddr[3] = my_hwaddr[3];
	pstNetif->hwaddr[4] = my_hwaddr[4];
	pstNetif->hwaddr[5] = my_hwaddr[5];
	
  /* maximum transfer unit */
  pstNetif->mtu = MTU;
  /* broadcast capability */
  pstNetif->flags = NETIF_FLAG_BROADCAST;
  hEthernetInput = OSSemCreate(0);
  hdm9000b= OSSemCreate(1);
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
//#if OS_CRITICAL_METHOD == 3   // 發送時關中斷。                 
//    OS_CPU_SR  		cpu_sr = 0;
//#endif	
//  u32_t intstat ;
  u8_t TxProduceIndex;//,TxConsumeIndex;
  u8_t	__u8Err;
  u8_t *__from_addr;//,*__to_addr;
  u16_t i=0;
  
  struct pbuf *__pstSendPbuf ;
//	u16_t __totallen=0; 
	err_t __errReturn = ERR_OK;
  u32_t total_len = 0;

    OSSemPend(hdm9000b, 400, &__u8Err); 
	__pstSendPbuf = pstPbuf;

	//* 阻塞對EMAC的訪問，以避免不同的任務同時訪問EMAC造成訪問衝突的問題，最長等待時間是2秒
//  OSSemPend(hBlockOutput, 400, &__u8Err);
 if(__u8Err==OS_NO_ERR  )
	{
//	    OS_ENTER_CRITICAL();
		for(; __pstSendPbuf!=NULL; __pstSendPbuf=__pstSendPbuf->next)
		{
			__from_addr=(u8_t*)__pstSendPbuf->payload;
          //  printf( "__from_addr:  %p \n",__from_addr);
           //printf( "____len:  %d \n",__pstSendPbuf->len);
          		// memcpy_hw( gpi_TxBufs+total_len,__from_addr, __pstSendPbuf->len);
		
			Send2DM900bBuffer(	 __from_addr, __pstSendPbuf->len);
          		total_len +=  __pstSendPbuf->len;
			//printf( "len:  %d \n",total_len);	
   		}			                               
            if(total_len<60) 
		{
			//printf("len<60.\n");		
			//Send2DM900bBuffer(	 __from_addr,60- total_len);
			//total_len=60;
			// printf( "total_len:  %d \n",total_len);
            	}
		//DM9000A_Send_Buffer(gpi_TxBufs,offset);
		// printf( "total_len:  %d \n",total_len);
		start_send2net(total_len);
              total_len = 0;
	  TxProduceIndex++;		/* transmit now */
//		OS_EXIT_CRITICAL();
	}
	else 
		{
		printf("no send.\n");
		}

    OSSemPost(hdm9000b);
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
//	    u32_t RxConsumeIndex;
//	    u32_t *rx_status_addr;
	    u16_t __u16Len;
	    u8_t  * REC_BUFF_PTR;		
//	    u8_t  *data_start_addr;
//	    u32_t *__pp;
	    struct pbuf  *__pstPbuf,   *__pstCurPbuf ;
	    u32_t  i;
	    u16_t	__u16LenToRead ,__u16RemainLenToRead;						//* 還剩多少字節沒有讀取到pbuf中
		u16_t RxStatus, temp_len, more_len;
        u8_t  status,rxbyte,__u8Err;		
	    u8 group,pin,gpio_level;
//		u8 xxx;
		
		#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
		    OS_CPU_SR  cpu_sr;	
		    cpu_sr = 0;                                  /* Prevent compiler warning                           */
		#endif    
	    __pstPbuf = NULL;

        OSSemPend(hdm9000b,400, &__u8Err);
    if(__u8Err==OS_NO_ERR  )
	{
        // DEBUG_GPIU("\n==low_level_input==\n");
        status=DM9000_ior(DM9000_ISR);
        //   masks =DM9000_ior(DM9000_IMR);
        //if((status & ISR_PRS)||(status==0x82))
        //if((status & ISR_PRS)||(status&ISR_PTS))
		
		//printf("DM9000_ISR 0x%x\n",status); 
        if((status & ISR_ROOS))
		{
		  printf("RX cnt over\n");
		}
        if((status & ISR_ROS))
		{
			printf("RX over\n");
			sysDeadLockMonitor_OFF(); /*Turn off watch dog.*/	
			#if	RX_over_Protect
			pbuf_init();
			GPIReset();
			#endif
			EMACInit();
			sysDeadLockMonitor_Reset();
			sysDeadLockMonitor_ON(); /*Turn on watch dog.*/

			printf("Reset EMAC\n");		  
			OSSemPost(hdm9000b);
			return NULL;	  
		}
		if((status & ISR_PRS))
		{
		      DM9000_iow(DM9000_ISR,ISR_PRS);
	           // DEBUG_GPIU("m \n"); 
		}
		
		else if((status==0x82))
		{
			//printf("PTS=0x%x\n",status);
		}
		
		else if(status & ISR_LNKCHNG)
		{
//aher 20151007 , CS中有使用到OStimdly, 先mark掉,確認是否有問題.
			//OS_ENTER_CRITICAL();
		   DM9000_iow(DM9000_ISR,ISR_LNKCHNG);
		   DEBUG_GPIU("LNKCHNG \n");  
		   OSTimeDly(1);
	       status=DM9000_ior(DM9000_NSR);
	       if(status & NSR_LINKST)
	       {
	         DEBUG_GPIU("Link OK \n"); 
		   	 net_link_status=0;
	       } 
	        else
	       {
	         DEBUG_GPIU("Link Fail \n"); 
			 net_link_status=1;	
			 renewIP=1;
			 #if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||\
                        (UI_VERSION == UI_VERSION_RDI_3) || (UI_FW_UPGRADE_ICON_ENABLE == 1))	
                    #if NIC_SUPPORT
	             Reminder_FW_Upgrade=0;           
	                #endif
				 uiOsdDrawRemindDownload(UI_OSD_CLEAR);
			 #endif
			 if(LwipISStatic!=1)
			 {	
	   			ClearNetworkInfo();
			 }	
	       }
		   uiOsdDrawNetworkLink(net_link_status);
	       //DM9000_iow(DM9000_ISR,ISR_LNKCHNG);
//		   OS_EXIT_CRITICAL();
		   OSSemPost(hdm9000b);
		   OSSemPost(Wait_Link_Status_Evt);
		   return NULL;
		}
	#if 0
	else
	{
		if(NIC_INT_ROULE==INT_USE_GROUP_1)
		{
			group=1;
			pin=19;
		}
		else
		{
			group=0;
			pin=0;
		}
			
		gpioGetLevel(group,pin,gpio_level);
		if(gpio_level)
		{
			printf("GPIO -> ERROR!!\n");	
			gpioSetDir(group,pin,0);
			gpioSetLevel(group,pin,0);
			gpioSetDir(group,pin,1);
		}	
		else
		{
			printf("GPIO -> No ERROR!!\n");
		}
		return NULL;
	}
	#else
	else
	{
		
		//printf("Unknow interrupt=%x from DM9000.\n",status);
		//DM9000_iow(DM9000_ISR,status&0x3f);
		//xxx=DM9000_ior(0x39);
		//printf("PIN Controller=%x\n",xxx);
		//xxx=DM9000_ior(0xfe);
		//printf("Int Status=%x\n",xxx);
		//xxx=DM9000_ior(0xff);
		//printf("Int Mask=%x\n",xxx);

		if(NIC_INT_ROULE==INT_USE_GROUP_1)
		{
			group=1;
			pin=19;
		}
		else
		{
			group=0;
			pin=0;
		}
			
		gpioGetLevel(group,pin,gpio_level);
		//printf("Gpio0Dir=%x,Level=%x\n",Gpio0Dir,gpio_level);
		OS_ENTER_CRITICAL();
		gpioSetDir(group,pin,0);
		gpioSetLevel(group,pin,0);
		gpioSetDir(group,pin,1);
	    OS_EXIT_CRITICAL();
		OSSemPost(hdm9000b);

		return NULL;
	}
	#endif
#if 0		
        DM9000_ior(DM9000_MRRH);
        DM9000_ior(DM9000_MRRL);        //must add this two read,weiyan
        //DM9000_ior(DM9000_MRCMDX);      
		GPIToRead(1,DM9000_MRCMDX,2, &i);
        rxbyte= i>>8;
       // DEBUG_GPIU(" pK status1(%x)\n", i);
//        GPIToRead(1,DM9000_MRCMDX,2, &i);
//        rxbyte= i>>8;
 //       DEBUG_GPIU(" pK status2(%x)\n", i);
#else
		rxbyte = DM9000_ior(DM9000_MRCMDX);
		rxbyte = DM9000_ior(DM9000_ISR);
		rxbyte = DM9000_ior(DM9000_MRCMDX) & 0xff;
#endif
      //  DEBUG_GPIU("<DM9000> RX rxbyte:%x\n", rxbyte);
        if (rxbyte == 0) 
        	{
        	  OSSemPost(hdm9000b);
		  return NULL;
        	}	
        if (rxbyte > 1) {
                DM9000_iow(DM9000_RCR, 0x00);   /* Stop Device */
                DM9000_iow(DM9000_ISR, 0x80);   /* Stop INT request */
                EMACInit();
                DEBUG_GPIU(" RESTART \n");
                OSSemPost(hdm9000b);
                return NULL;
        }
        
        /* A packet ready now  & Get status/length */
		GPIToRead(1,DM9000_MRCMD,4, &i);	
		RxStatus = i;
	
        __u16Len = i>>16;
       // __u16Len -=4;
       // DEBUG_GPIU(" RX Len:%d(%x)\n", __u16Len, RxStatus);


/////////////////////////////////////////////////////////////////////////////        
	   // __u16Len++;
		//* 從pbuf pool中獲取一個pbuf鏈
		__pstPbuf = pbuf_alloc(PBUF_RAW, __u16Len, PBUF_POOL);
		if(__pstPbuf != NULL)
		{
			//* 複製數據
		__pstCurPbuf=__pstPbuf;
		__u16RemainLenToRead=__u16Len;

		while(__u16RemainLenToRead >0)
		{
		   // printf( "RX:@pbuf %p\n", (void *)__pstCurPbuf); 
	        REC_BUFF_PTR=(u8_t *)__pstCurPbuf->payload;   
          //  printf( "RX:@payload %p\n",REC_BUFF_PTR);
            
			if(__u16RemainLenToRead>__pstCurPbuf->len)
	    	{
	      	    __u16LenToRead=__pstCurPbuf->len;
	      	    __u16RemainLenToRead-=__pstCurPbuf->len;
                //printf( "RX:@1_payload %p\n",REC_BUFF_PTR);
                 GPIToRead(0, 0,__u16LenToRead, REC_BUFF_PTR);
	        }
	        else 
	    	{
	    	    __u16LenToRead=__u16RemainLenToRead;
	    	    __u16RemainLenToRead=0;
		more_len = __u16LenToRead % 4;
        temp_len = __u16LenToRead - more_len;
                 //printf( "RX:@2_payload %p\n",REC_BUFF_PTR);
                 #if (GPI_BUF_RX_MODE == USE_DMA_MODE)				 
        GPIToRead(0, 0,temp_len, REC_BUFF_PTR);
                #elif(GPI_BUF_RX_MODE == USE_CPU_MODE)
        			temp_p=REC_BUFF_PTR;
    			for(i=0;i<(temp_len/4);i++)
    			{
    			        GPIToRead(0, 0,4, temp_p);
    				temp_p +=4;
                
    			}
    			//printf("r_\n");
    		  #endif 
        if(more_len) 
            {
                REC_BUFF_PTR = REC_BUFF_PTR + temp_len;
            	GPIToRead(0, 0, more_len,REC_BUFF_PTR);
            }
	        }
            //    GPIToRead(0, 0, 4,&i); //CRC
			if(__u16RemainLenToRead >0)  
				{
					 __pstCurPbuf=__pstCurPbuf->next;
					 if(__pstCurPbuf==NULL)  __u16RemainLenToRead =0;
                      //printf( "RX:@next_pbuf %p\n", (void *)__pstCurPbuf); 
				}

		}

 /////////////////////////////////////////////////////       


#if 0 //buffer chkeck data
        __pstCurPbuf=__pstPbuf;
        REC_BUFF_PTR=(u8_t *)__pstCurPbuf->payload;
        printf( "RX:@_index %p\n",REC_BUFF_PTR);
		for (i=0 ;i<__u16Len;i++)
	   	{
	   	    if((i%16)==0)DEBUG_GPIU("R_[%.3x] =",i);
			
            DEBUG_GPIU(" %.2x",REC_BUFF_PTR[i]);
			
			if((i%16)==15) 	DEBUG_GPIU("\n");
            
            if(i% PBUF_POOL_BUFSIZE == 0 && i >0) 
                {
                    __pstCurPbuf=__pstCurPbuf->next;
                    REC_BUFF_PTR=(u8_t *)__pstCurPbuf->payload; 
                    DEBUG_GPIU( "RX:@next_index %p\n",REC_BUFF_PTR);
	   	}
	   	}
        	DEBUG_GPIU("\n");
#endif		
        	//if((__u16Len<60)||(__u16Len>MTU)) 
			if((__u16Len<60)||(__u16Len>DM9000_PKT_MAX)) 
	    	{
	           OSSemPost(hdm9000b);
               DEBUG_GPIU(" RX Len:%d(%x)\n", __u16Len, RxStatus);
	   		   DEBUG_GPIU("__u16Len>MTU \n");
               pbuf_free(__pstPbuf);
	    	   return NULL;
	    	}

         if ((RxStatus & 0xbf00) || (__pstPbuf->len < 0x40)|| (__u16Len > DM9000_PKT_MAX))
		 	{
                if (RxStatus & 0x100) {
                        DEBUG_GPIU("rx fifo error\n");
                }
                if (RxStatus & 0x200) {
                        DEBUG_GPIU("rx crc error\n");
                }
                if (RxStatus & 0x8000) {
                        DEBUG_GPIU("rx length error\n");
                }
                if (__u16Len > DM9000_PKT_MAX) {
                        DEBUG_GPIU("rx length too big\n");
                        EMACInit();
                }
                       	pbuf_free(__pstPbuf);
		        __pstPbuf = NULL;  
                
			}
		 else
		 	{
                //DEBUG_GPIU(" RX Len:%d:%d\n", __pstPbuf->len,__u16Len);
                //printf("r_end\n");
                      OSSemPost(hdm9000b);
                return __pstPbuf;
        	}

		}
		else
		{
			  DEBUG_GPIU("NO SPACE\n");
		}
    	}
	else
		{
                   printf("=========abort RX============\n");
                   return NULL;
                }

    		OSSemPost(hdm9000b);
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
    u8 err;
	//printf("\n _input\n");
	__pstNetif = (struct netif*)pReserved;			

	while(TRUE)
	{
#if 0
		OSSemPend(hEthernetInput, 0, &__u8RtnVal);
			__pstPbuf = low_level_input(__pstNetif); //=eth_rx();
			if(__pstPbuf == NULL) continue;
     do
	   {
		__pstEthernetif = (struct ethernetif*)__pstNetif->state;
		__pstEthhdr = __pstPbuf->payload;
		
		switch(htons(__pstEthhdr->type))
		{
			case ETHTYPE_IP:
				//* 更新ARP表
				printf("\n _IP\n");
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
          __pstPbuf = low_level_input(__pstNetif); //=eth_rx();
         }while(__pstPbuf != NULL);
        DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_PTM); 
    #else
		OSSemPend(hEthernetInput, 400, &__u8RtnVal);
      if (__u8RtnVal != OS_NO_ERR)
        {
        	#if RX_over_Protect
        	printf("\x1B[91m ERR:%d \x1B[0m\n",__u8RtnVal);
        	if(__u8RtnVal == OS_ERR_EVENT_TYPE)
        	{
				DEBUG_GPIU("Waring!! hEthernetInput Time out!\n");
				hEthernetInput = OSSemDel(hEthernetInput, OS_DEL_ALWAYS, &err);
				hEthernetInput = OSSemCreate(0);
				pbuf_init();
				GPIReset();
				EMACInit();
        	}
       		continue;
       		#endif
        }

	while(TRUE)
		{
		//* 從EMAC循環讀取數據
			__pstPbuf = low_level_input(__pstNetif); //=eth_rx();
			if(__pstPbuf == NULL)
               { 
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
        #endif
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
  //INT8U	__u8Err;
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
u8_t status,masks;
  //  printf("\n=>\n");

//    status=DM9000_ior(DM9000_ISR);
 //   masks =DM9000_ior(DM9000_IMR);
//    if((status & ISR_PRS)&&(masks & IMR_PRM))
	{
	    OSSemPost(hEthernetInput);
		return;
	//  	DM9000_iow(DM9000_ISR,ISR_PRS);
   //   DEBUG_GPIU(" R_\n");
    //  DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PTM); 
	}
	
	if((status & ISR_PTS)&&(masks & IMR_PTM))
	{
	  //  gpioSetLevel(1, 6, 1);
	    OSSemPost(DMiSemTran);
		DM9000_iow(DM9000_ISR,ISR_PTS);
     //  	 DEBUG_GPIU(" S_\n");
    //     DM9000_iow(DM9000_IMR, IMR_PAR);
	}
}

/* Set PHY operationg mode*/
 void set_PHY_mode(enum DM9KS_PHY_mode op_mode)
{
  u16 phy_reg0 = 0x1200;		/* Auto-negotiation & Restart Auto-negotiation */	
  u16 phy_reg4 = 0x01e1;		/* Default flow control disable*/
  u16 i,sta;

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
        for (i = 0, sta = 0x16; i < 7; i++, sta++)
                DM9000_iow(sta, 0x00);
                 DM9000_iow(0x1D, 0x80);
      
        DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);  /* RX enable */
 //       DM9000_iow(DM9000_IMR, IMR_PAR);        
      DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_LNKCHNG);        /* Enable RX interrupt mask */      
    i = 150;   
    while(--i)   
    {   
        udelay(0x1000);   
		//Check Link status
		//EMAC_link_status();
		/*
        printf(" Wait READY");
        sta=DM9000_ior(DM9000_NSR);
        
        if(sta && 0x40)
            {
              printf("Link ok\n ");
               break;  
            }
        else
            printf("Link fail\n ");
        */    
    } 
  }

extern void NicCloseAllTask(void);
extern void GPIReset();
void EMACInit()  
{   
	int i, oft, retry_cnt=0;
	u8_t IO_mode;
	u16_t  lnk;
	INT8U  err;

	DEBUG_GPIU("EMACInit\n"); 
    i=0;
    if(dm9000_probe()== FALSE)
        {
         while(dm9000_probe()== FALSE)
            {
               DEBUG_GPIU(" DM9000 Init Fail\n");
//#if (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD)||(HW_BOARD_OPTION == MR8200_RX_GCT_LCD)                 
#if 1
               i++;
                if(i==50)
                    {
                     dm9000_reset();    
                     #if (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
                     GPIReset();
					 #elif(HW_BOARD_OPTION == MR8200_RX_RDI_M721 && PROJ_OPT == 5)
					  GPIReset_RDI();
                     #endif
                     DEBUG_GPIU("NicReset\n");
                    i=0;
                    retry_cnt++;
                    if(retry_cnt > 4)
                   	{
                   		DEBUG_GPIU("NicReset Fail.\n");
						break;
                   	}
                    }
#endif                  
	  		      }
	  		}
	/* set the internal PHY power-on */
    DM9000_iow(DM9000_GPR, 0x00);   
	dm9000_reset();
	#if (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
    GPIReset();
    #endif

		IO_mode = DM9000_ior(DM9000_ISR)&0x80;

	if(IO_mode)
		DEBUG_GPIU("8 bit mode\n"); 
	else
		DEBUG_GPIU("16 bit mode\n");
	/* Set PHY */
    #if 0 //Flow Control
	phy_write(4, 0x05e1);
	phy_write(0, 0x1200); /* N-way */
    DM9000_iow(DM9000_FCR, 0x0A); 
    DEBUG_GPIU("FLOW cotrol\n"); 
    #else            
	phy_write(4, 0x01e1);
	phy_write(0, 0x1200); /* N-way */
    DM9000_iow(DM9000_FCR, 0x0);  
	#endif
    
    DM9000_iow(0x38,0x6B ); 
	//DM9000_iow(0x39,0x01 ); // set to LOW active.
    DM9000_iow(DM9000_NCR, 0); 
    DM9000_iow(DM9000_TCR, 0x1E);      
    DM9000_iow(DM9000_BPTR, 0x3f);/* Less 3kb, 600us */  
    DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));  

    DM9000_iow(DM9000_SMCR, 0); /* Special Mode */   
    DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END); /* clear TX status */  
    DM9000_iow(DM9000_ISR, 0x3f);/* Clear interrupt status */  
	DM9000_iow(DM9000_CSCR, CSCR_UDPCSE|CSCR_TCPCSE|CSCR_IPCSE);/* Enable TCP/IP/UDP Check Sum */ 

    for (i = 0, oft = 0x10; i < 6; i++, oft++)
            DM9000_iow(oft, my_hwaddr[i]);
    for (i = 0, oft = 0x16; i < 7; i++, oft++)
            DM9000_iow(oft, 0x00);
            DM9000_iow(0x1D,0x80);
  
    DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);  /* RX enable */
//       DM9000_iow(DM9000_IMR, IMR_PAR);        
   DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_LNKCHNG);        /* Enable RX interrupt mask */
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
	DMiSemTran = OSSemDel(DMiSemTran, OS_DEL_ALWAYS, &err);
    DMiSemTran= OSSemCreate(0);
   //DEBUG_GPIU("operating at 100M full duplex mode\n");
  
}

void EMACInit_reboot()  
{   
int i, oft, retry_cnt=0;
u8_t IO_mode;
u16_t  lnk;
		INT8U  err;

		DEBUG_GPIU("EMACInit_reboot\n"); 
        i=0;
        if(dm9000_probe()== FALSE)
            {
             while(dm9000_probe()== FALSE)
                {
                   DEBUG_GPIU(" DM9000 Init Fail\n");
//#if (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD)||(HW_BOARD_OPTION == MR8200_RX_GCT_LCD)                 
#if 1
                   i++;
                    if(i==50)
                    {
                         dm9000_reset();                           
						#if (HW_BOARD_OPTION == MR8200_RX_RDI_M721 && PROJ_OPT == 5)
						  GPIReset_RDI();
						#endif					   
                         DEBUG_GPIU("NicReset\n");
                        i=0;
                        retry_cnt++;
                        if(retry_cnt > 4)
                       	{
                       		DEBUG_GPIU("NicReset Fail.\n");
							break;
                       	}
                    }
#endif                  
 	  		      }
 	  		}
		/* set the internal PHY power-on */
        DM9000_iow(DM9000_GPR, 0x00);   
		//dm9000_reset();
 
 		IO_mode = DM9000_ior(DM9000_ISR)&0x80;

		if(IO_mode)
			DEBUG_GPIU("8 bit mode\n"); 
		else
			DEBUG_GPIU("16 bit mode\n");
		/* Set PHY */
        #if 0 //Flow Control
		phy_write(4, 0x05e1);
		phy_write(0, 0x1200); /* N-way */
        DM9000_iow(DM9000_FCR, 0x0A); 
        DEBUG_GPIU("FLOW cotrol\n"); 
        #else            
		phy_write(4, 0x01e1);
		phy_write(0, 0x1200); /* N-way */
        DM9000_iow(DM9000_FCR, 0x0);  
		#endif
        
	    DM9000_iow(0x38,0x6B ); 
		//DM9000_iow(0x39,0x01 ); // set to LOW active.
        DM9000_iow(DM9000_NCR, 0); 
        DM9000_iow(DM9000_TCR, 0x1E);      
        DM9000_iow(DM9000_BPTR, 0x3f);/* Less 3kb, 600us */  
        DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));  
 
        DM9000_iow(DM9000_SMCR, 0); /* Special Mode */   
        DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END); /* clear TX status */  
        DM9000_iow(DM9000_ISR, 0x3f);/* Clear interrupt status */  
		DM9000_iow(DM9000_CSCR, CSCR_UDPCSE|CSCR_TCPCSE|CSCR_IPCSE);/* Enable TCP/IP/UDP Check Sum */ 

        for (i = 0, oft = 0x10; i < 6; i++, oft++)
                DM9000_iow(oft, my_hwaddr[i]);
        for (i = 0, oft = 0x16; i < 7; i++, oft++)
                DM9000_iow(oft, 0x00);
                DM9000_iow(0x1D,0x80);

      
        DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);  /* RX enable */
 //       DM9000_iow(DM9000_IMR, IMR_PAR);        
       DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PRM|IMR_LNKCHNG);        /* Enable RX interrupt mask */
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
		DMiSemTran = OSSemDel(DMiSemTran, OS_DEL_ALWAYS, &err);
	    DMiSemTran= OSSemCreate(0);
       //DEBUG_GPIU("operating at 100M full duplex mode\n");
      
}


//Check link status. aher 2013/01/17
static void EMAC_link_status()
{
	if(phy_read(1)&0x04)
	{
		printf("Link status is connecting\n ");
		net_link_status=0;
	}	
	else
	{
     	printf("Link status is disconnect. \n");
		net_link_status=1;
	}		
     
}
#else
int net_link_status;
#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
