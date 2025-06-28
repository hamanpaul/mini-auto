

#include "general.h"
#include "board.h"
#include "sysapi.h"
#include "gpi.h"
#include "gpireg.h"
#include "gpi_dm9000b.h"
#include "gpioapi.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "Net.h"
OS_EVENT *DMiSemRv;      /* Semaphore to  RX interrupt. */
OS_EVENT *DMiSemTran;      /* Semaphore to tr interrupt. */

#if defined(AUTOMDIX)
#define DMFE_TIMER_MDIX	jiffies+(HZ*1)	/* timer wakeup time : 1 second */
#endif



enum DM9000_PHY_mode 
{ 
	DM9000_10MHD 	= 0, 
	DM9000_100MHD 	=1, 
	DM9000_10MFD 	= 4, 
	DM9000_100MFD 	= 5, 
	DM9000_AUTO 	=8, 
	DM9000_1M_HPNA 	= 0x10
};

u16 media_mode = DM9000_100MFD;
extern u8  my_hwaddr[];

void udelay(u16 count)
{
  while (--count > 0);
}

u32 DM9000_ior(u8 reg)
{
u32	DATAaddr,CMD;
    CMD = reg;
	GPIToRead(0x01, CMD, 0x01, &DATAaddr);
   // DEBUG_GPIU("Read %x =%x\n",reg,DATAaddr);
	return DATAaddr; 
}

void
DM9000_iow(int reg, u8 value)
{ 
	u32 DATAbuf;
		DATAbuf = value;
		GPIToWrite(0x01, reg, 0x01, &DATAbuf);
}
void dm9000_reset(void)
{
        DM9000_iow(DM9000_NCR, NCR_RST);
        udelay(1000);
}

int
dm9000_probe(void)
{
        u32 id_val;
        id_val = DM9000_ior(DM9000_VIDL)&0x000000ff;
        id_val |= (DM9000_ior(DM9000_VIDH)&0x000000ff) << 8;
        id_val |= (DM9000_ior(DM9000_PIDL)&0x00ff) << 16;
        id_val |= (DM9000_ior(DM9000_PIDH)&0x00ff) << 24;
        DEBUG_GPIU(" VID: %x \n", id_val);
        if (id_val == DM9000_ID) 
		{
		    DEBUG_GPIU(" VID PASS \n");
         	return TRUE;
        } 
		else 
		{    DEBUG_GPIU("FALSE VID\n");
            return FALSE;
        }

}

 u16
phy_read(int reg )
{
        DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);

        DM9000_iow(DM9000_EPCR, 0x0C);  
        udelay(1000);          
        DM9000_iow(DM9000_EPCR, 0x00);  
        
       return ((DM9000_ior(DM9000_EPDRH) << 8)| DM9000_ior(DM9000_EPDRL));
    }

 void
phy_write(int reg, u16 value)
{
        DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);
        DM9000_iow(DM9000_EPDRL, (value & 0xff));
        DM9000_iow(DM9000_EPDRH, ((value >> 8) & 0xff));
        DM9000_iow(DM9000_EPCR, 0xa);  
        udelay(1000);          
        DM9000_iow(DM9000_EPCR, 0x0);  
        
}
void change_m_addr(void)
{
int i, oft;
        for (i = 0, oft = 0x16; i < 8; i++, oft++)
                DM9000_iow(oft, 0xFF);
}
void dump_dm9000_phy_regs(void)
{
 
u16 value; 

        value = phy_read(0);  
        printf("PHY_00= %x\n", value);   
        value = phy_read(1);  
        printf("PHY_01= %x\n", value); 
        value = phy_read(2);  
        printf("PHY_02= %x\n", value);   
        value = phy_read(3);  
        printf("PHY_03= %x\n", value);
        value = phy_read(4);  
        printf("PHY_04= %x\n", value);   
        value = phy_read(5);  
        printf("PHY_05= %x\n", value); 
        value = phy_read(6);  
        printf("PHY_06= %x\n", value);   
        value = phy_read(16);  
        printf("PHY_16= %x\n", value);
        value = phy_read(17);  
        printf("PHY_17= %x\n", value);   
        value = phy_read(18);  
        printf("PHY_18= %x\n", value); 
        value = phy_read(19);  
        printf("PHY_19= %x\n", value);   
        value = phy_read(20);  
        printf("PHY_20= %x\n", value);
        value = phy_read(27);  
        printf("PHY_27= %x\n", value);
        value = phy_read(29);  
        printf("PHY_29= %x\n", value);
        
}
void dump_dm9000_control_regs(void)
{
 
u32 value; 

        value = DM9000_ior(0);
        printf("dm9000_00= %x\n", value);   
        value = DM9000_ior(1);  
        printf("dm9000_01= %x\n", value); 
        value = DM9000_ior(2);  
        printf("dm9000_02= %x\n", value);   
        value = DM9000_ior(3);  
        printf("dm9000_03= %x\n", value);
        value = DM9000_ior(0x04);  
        printf("dm9000_04= %x\n", value);   
        value = DM9000_ior(0x05);  
        printf("dm9000_05= %x\n", value); 
        value = DM9000_ior(0x06);  
        printf("dm9000_06= %x\n", value);   
        value = DM9000_ior(0x07);  
        printf("dm9000_07= %x\n", value);
        value = DM9000_ior(0x08);  
        printf("dm9000_08= %x\n", value);   
        value = DM9000_ior(0x09);  
        printf("dm9000_09= %x\n", value); 
        value = DM9000_ior(0x0A);  
        printf("dm9000_0x0A= %x\n", value);   
        value = DM9000_ior(0x0B);  
        printf("dm9000_0x0B= %x\n", value);
        value = DM9000_ior(0x0C);  
        printf("dm9000_0x0C= %x\n", value);
        value = DM9000_ior(0x0D);  
        printf("dm9000_0x0D= %x\n", value);
        value = DM9000_ior(0x0E);  
        printf("dm9000_0x0E= %x\n", value);   
        value = DM9000_ior(0x0F);  
        printf("dm9000_0x0F= %x\n", value);
        value = DM9000_ior(0x10);  
        printf("dm9000_0x10= %x\n", value);
        value = DM9000_ior(0x11);  
        printf("dm9000_0x11= %x\n", value);
        value = DM9000_ior(0x12);
        printf("dm9000_0x12= %x\n", value);
        value = DM9000_ior(0x13);  
        printf("dm9000_0x13= %x\n", value);
        value = DM9000_ior(0x14);  
        printf("dm9000_0x14= %x\n", value);
        value = DM9000_ior(0x15);  
        printf("dm9000_0x15= %x\n", value);
        value = DM9000_ior(0x16);
        printf("dm9000_0x16= %x\n", value);
        value = DM9000_ior(0x17);  
        printf("dm9000_0x17= %x\n", value);
        value = DM9000_ior(0x18);  
        printf("dm9000_0x18= %x\n", value);
        value = DM9000_ior(0x19);  
        printf("dm9000_0x19= %x\n", value);
        value = DM9000_ior(0x1A);
        printf("dm9000_0x1A= %x\n", value);
        value = DM9000_ior(0x1B);  
        printf("dm9000_0x1B= %x\n", value);
        value = DM9000_ior(0x1C);  
        printf("dm9000_0x1C= %x\n", value);
        value = DM9000_ior(0x1D);  
        printf("dm9000_0x1D= %x\n", value);
        value = DM9000_ior(0x1E);
        printf("dm9000_0x1E= %x\n", value);
        value = DM9000_ior(0x1F);  
        printf("dm9000_0x1F= %x\n", value);  

        value = DM9000_ior(0x22);
        printf("dm9000_0x22= %x\n", value);
        value = DM9000_ior(0x23);  
        printf("dm9000_0x23= %x\n", value);
        value = DM9000_ior(0x24);  
        printf("dm9000_0x24= %x\n", value);
        value = DM9000_ior(0x25);  
        printf("dm9000_0x25= %x\n", value);
        value = DM9000_ior(0x26);
        printf("dm9000_0x26= %x\n", value);
        value = DM9000_ior(0x27);  
        printf("dm9000_0x27= %x\n", value);
        value = DM9000_ior(0x28);  
        printf("dm9000_0x28= %x\n", value);
        value = DM9000_ior(0x29);  
        printf("dm9000_0x29= %x\n", value);
        value = DM9000_ior(0x2A);
        printf("dm9000_0x2A= %x\n", value);
        value = DM9000_ior(0x2B);  
        printf("dm9000_0x2B= %x\n", value);
        value = DM9000_ior(0x2C);  
        printf("dm9000_0x2C= %x\n", value);
        value = DM9000_ior(0x2D);  
        printf("dm9000_0x2D= %x\n", value);
        value = DM9000_ior(0x2E);
        printf("dm9000_0x2E= %x\n", value);
        value = DM9000_ior(0x2F);  
        printf("dm9000_0x2F= %x\n", value);  
        
        value = DM9000_ior(0x30);  
        printf("dm9000_0x30= %x\n", value);
        value = DM9000_ior(0x31);  
        printf("dm9000_0x31= %x\n", value);
        value = DM9000_ior(0x32);
        printf("dm9000_0x32= %x\n", value);
        value = DM9000_ior(0x33);  
        printf("dm9000_0x33= %x\n", value);
        value = DM9000_ior(0x34);  
        printf("dm9000_0x34= %x\n", value);
        value = DM9000_ior(0x35);  
        printf("dm9000_0x35= %x\n", value);
        value = DM9000_ior(0x36);
        printf("dm9000_0x36= %x\n", value);
        value = DM9000_ior(0x37);  
        printf("dm9000_0x37= %x\n", value);
        value = DM9000_ior(0x38);  
        printf("dm9000_0x38= %x\n", value);
        value = DM9000_ior(0x39);  
        printf("dm9000_0x39= %x\n", value);     
        value = DM9000_ior(0x50);  
        printf("dm9000_0x30= %x\n", value);
        
}
void UartWriteDM9000Reg(char *UartCmdString)
{
    int val;
    u32 u32addr, u32val;

    val = sscanf(UartCmdString, "%x %x", &u32addr, &u32val);
    if(val==2)
    {
        DM9000_iow(u32addr, u32val);
        DEBUG_UART("Write dm9000: 0x%x = 0x%x\n", u32addr, u32val);
        
        printf("dm9000: 0x%x = 0x%x\n", u32addr, DM9000_ior(u32addr));
    }

}
 void UartReadM9000Reg(char *UartCmdString)
{
   int val;
   u8 u8addr;

    val = sscanf(UartCmdString, "%x", &u8addr);
    //DEBUG_UART("read Reg:(%d)\n",val);
    if(val)
        printf("Read dm9000: 0x%x = 0x%x\n", u8addr, DM9000_ior(u8addr));
}
void
eth_halt_true(void)
{

        phy_write(0, 0x8000);   /* PHY RESET */
        DM9000_iow(DM9000_GPR, 0x01);   /* Power-Down PHY */
        DM9000_iow(DM9000_IMR, 0x80);   /* Disable all interrupt */
        DM9000_iow(DM9000_RCR, 0x00);   /* Disable RX */
}
void Send2DM900bBuffer(u8 *packet, u16 length)
{
    u8 *data_ptr,temp_more;
    u16 temp_len;
    u32  temp,*Ptemp;
    u32  i=0;

    temp_len = length;
	
    #if (GPI_BUF_TX_MODE == USE_CPU_MODE)
	  data_ptr=packet;
         temp_more = temp_len % 4;
	  GPIToWrite(1, DM9000_MWCMD, temp_more, data_ptr);
         data_ptr += temp_more;
         temp_len = temp_len-temp_more;
    for (i=0 ;i<(temp_len/4);i++)
    {
        GPIToWrite(0, 0, 4, data_ptr);
        data_ptr+=4;
    }
    #elif(GPI_BUF_TX_MODE == USE_DMA_MODE)  
	temp=(u32)packet;
	//printf(" temp=%x\n",temp);
	temp_more = temp & 3;
	//printf(" temp_more=%x/n",temp_more);
	GPIToWrite(1, DM9000_MWCMD, temp_more, packet);
	if(length<4) return;
	temp_len-=temp_more;
	packet+=temp_more;	
       Ptemp = (u32*)packet;
	// printf(" packet=%x\n",packet);  
       GPIToWrite(0, 0, temp_len, Ptemp);
    #endif
}
void start_send2net(u16 length)
{
 u8 ucErr,status,i ;

       status=DM9000_ior(DM9000_ISR);
	 if(status & ISR_PTS)
		{
			DM9000_iow(DM9000_ISR,ISR_PTS);
        //	DEBUG_GPIU("Send_int_clr\n");
		}
      //    DEBUG_GPIU("Send_Pend\n");
	DM9000_iow( DM9000_TXPLL, length);
	DM9000_iow( DM9000_TXPLH, length >> 8);

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
	i=20;
	while (1)/* wait for tx complete */
        {
	   // if (DM9000_ior(DM9000_NSR)& (NSR_TX2END|NSR_TX1END))
          if ( !(DM9000_ior(DM9000_TCR) & TCR_TXREQ) )	
	    {
	     //   DEBUG_GPIU("\n=%d\n",i);
            break;
         }
       }
	if(i==0)printf("\nstart_send2net:******************TX Error****\n");
    #endif   
}
void DM9000A_Send_Buffer(u8 *packet, u16 length)
{
    u8 *data_ptr,temp_more,ucErr,status;
    u16 temp_len;
    u32  temp[500],*Ptemp;
    u32  i=0;
    // gpioSetLevel(0,0,0);
	//DEBUG_GPIU("entering %s\n", __func__);
    // gpioSetLevel(0,1,1);
    //  DEBUG_GPIU("length = %d \n",length);

    data_ptr = packet;
    //GPIToWrite(1, DM9000_MWCMD, 0, temp);
    //DEBUG_GPIU("data_ptr[%d] = 0x%8x\n",i,data_ptr[i]);

    if(length<60) length=60;
    temp_len = length;

    #if (GPI_BUF_TX_MODE == USE_CPU_MODE)
    temp_more = temp_len % 4;
	GPIToWrite(1, DM9000_MWCMD, temp_more, data_ptr);
    data_ptr += temp_more;
    temp_len = temp_len-temp_more;
    for (i=0 ;i<(temp_len/4);i++)
    {
        GPIToWrite(0, 0, 4, data_ptr);
        data_ptr+=4;
    }
    #elif(GPI_BUF_TX_MODE == USE_DMA_MODE)   
   // temp_more = temp_len % 4;
	GPIToWrite(1, DM9000_MWCMD, 0, temp);    
    Ptemp = (u32*)packet;
  //  temp_len = temp_len-temp_more;
    GPIToWrite(0, 0, temp_len, Ptemp);
  //  if(temp_more)
  //  {
  //      Ptemp = Ptemp + (temp_len/4);
  //      GPIToWrite(0, 0, temp_more, Ptemp);
 //   }
    #endif

#if 0 //for check data 
            data_ptr = packet;

        printf( "TX:@_index %p\n",data_ptr);
            for (i=0 ;i<length;i++)
       		{
       	    if((i%16)==0)DEBUG_GPIU("R_[%.3x] =",i);
            
            DEBUG_GPIU(" %.2x",data_ptr[i]);
    		
    		if((i%16)==15) 	DEBUG_GPIU("\n");
            
    	   		}
             DEBUG_GPIU("\n");
#endif            

// DEBUG_GPIU("==wait for tx complete ==\n");
//    	DEBUG_GPIU("length = %d \n",length);
	/* Set TX length to DM9000 */

        status=DM9000_ior(DM9000_ISR);
	    if(status & ISR_PTS)
		{
			DM9000_iow(DM9000_ISR,ISR_PTS);
        //	DEBUG_GPIU("Send_int_clr\n");
		}
      //    DEBUG_GPIU("Send_Pend\n");
	DM9000_iow( DM9000_TXPLL, length);
	DM9000_iow( DM9000_TXPLH, length >> 8);

	DM9000_iow( DM9000_TCR, TCR_TXREQ);	/* Cleared after TX complete */

    #if (GPI_TRG_TX_MODE == USE_INTERRUPT)
		if (DM9000_ior(DM9000_NSR)& (NSR_TX2END|NSR_TX1END))
    {
		 //       DEBUG_GPIU("1_ok\n");
    }
    else
    {
        // DM9000_iow(DM9000_IMR, IMR_PAR|IMR_PTM);        /* Enable TX interrupt mask */
        OSSemPend(DMiSemTran, 200, &ucErr);
    	if (ucErr != OS_NO_ERR)
        {
    	    DEBUG_GPIU("Error: DMiSemTran is %d.\n", ucErr);
     		//DEBUG_NET("SemEvtOSEventCnt = %d\n", DMiSemTran->OSEventCnt);
    	}
    }
    #elif (GPI_TRG_TX_MODE == USE_POLLING)    
	while (1)/* wait for tx complete */
    {
	    if (DM9000_ior(DM9000_NSR)& (NSR_TX2END|NSR_TX1END))
	    {
	        // DEBUG_GPIU("Send_ok\n");
            break;
         }
    }
    #endif    
    // while (DM9000_ior(DM9000_TCR) & TCR_TXREQ);	/* wait for end of transmission */
    //   DEBUG_GPIU("DM9000A_Send_Buffer_end\n");
    //DEBUG_GPIU("Send_ok\n");
}



