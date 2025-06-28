#include "general.h"
#include "gpioapi.h"
#include "board.h"


#define USE_A7196_VER   1

#if ( (RFIC_SEL==RFIC_A7196_6M) || (RFIC_SEL==RFIC_A7196_4M) )
#include "rfiuapi.h"
#include "A7196reg.h"
#include "AMIC7196.h"
#include "rfiu.h"
#include "../../gpio/inc/gpioreg.h"

const BYTE KeyData_Tab[16]={0x00,0x00,0x00,0x00,0x00,0x0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //keyData code
const BYTE FCB_Tab[20]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //FCB code
const BYTE PageTab[12]={0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0,0xB0};//page select

    const BYTE A7196Config[]=
    {
    		0x00,		//MODE_REG 				
    		0x60,		//MODECTRL_REG		  
    		0x00,		//CALIBRATION_REG	
    		0x3F,		//FIFO1_REG 			  
    		0x00,		//FIFO2_REG 			  
    		0x00,		//FIFO_REG 				
    		0x00,		//IDCODE_REG 			
    		0x00,		//RCOSC1_REG 			
    		0x00,		//RCOSC2_REG 			
    		0x0C,		//RCOSC3_REG 	
      #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    		0x00,		//CKO_REG , Disable				  
    		0x00,		//GIO1_REG:RX, Disable. 				
    		0x24,		//GIO2_REG:Tx 	
      #else
    		0x3A,		//CKO_REG 				  
    		0x21,		//GIO1_REG:RX 				
    		0x24,		//GIO2_REG:Tx 	
      #endif		
    		0x2F,		//DATARATE_REG 		
    		0x5A,		//PLL1_REG 				
    		0x16,		//PLL2_REG				  
    		0x64,		//PLL3_REG 				
    		0x00,		//PLL4_REG				  
    		0x02,		//PLL5_REG 				
    		0x3C,		//CHGROUP1_REG		  
    		0x78,		//CHGROUP2_REG	
    	#if(RFIC_SEL==RFIC_A7196_6M)
    		0x2F,		//TX1_REG  				//0x2F Gaussian OFF
    		0x40,		//TX2_REG  	
        #elif(RFIC_SEL==RFIC_A7196_4M) 
    		0x2e,//0x2F,		//TX1_REG  				//0x2F Gaussian OFF
    		0x56,//0x40,		//TX2_REG  	
    	#endif	   
    		0x18,		//DELAY1_REG			  
    		0x40,		//DELAY2_REG			  
    		0x70,		//RX_REG					  
    		0xFC,		//RXGAIN1_REG			
    		0xCA,		//RXGAIN2_REG		
    	#if(RFIC_SEL==RFIC_A7196_6M)
    		0xFC,		//RXGAIN3_REG		
        #elif(RFIC_SEL==RFIC_A7196_4M)
    		0xBC,//0xFC,		//RXGAIN3_REG		
        #endif		   
    		0xCA,		//RXGAIN4_REG			
    		0x00,		//RSSI_REG				  
    		0xF1,		//ADC_REG  				
    		0x0c,		//CODE1_REG 			  
    		0x0e,//0x0e,		//CODE2_REG,preamble error tolerance=0 			  
    		0x2A,		//CODE3_REG 			  
    	    0x05,		//IFCAL1_REG  		  
    		0x51,		//IFCAL2_REG  		  
    		0xCF,		//VCOCCAL_REG  		
    		0xD0,		//VCOCAL1_REG  		
    		0x80,		//VCOCAL2_REG  	
    	 #if(RFIC_SEL==RFIC_A7196_6M)
    		0xB0,		//VCODEVCAL1_REG   
         #elif(RFIC_SEL==RFIC_A7196_4M)
    		0x70,//0xB0,		//VCODEVCAL1_REG   
         #endif		   
    		0x36,		//VCODEVCAL2_REG   
    		0x00,		//DASP_REG 
    	 #if(RFIC_SEL==RFIC_A7196_6M)
    		0xFF,		//VCOMODDELAY_REG	
         #elif(RFIC_SEL==RFIC_A7196_4M)
    		0xDB,//0xFF,		//VCOMODDELAY_REG	
         #endif
    		0x70,		//BATTERY_REG  	
    	 #if AMIC7196_USE_EXT_PA
    		0x40,		//TXTEST_REG  		
         #else
    		0x7F,		//TXTEST_REG  		
         #endif
    		0x37,//0x37,		//RXDEM1_REG: DC by SID1(preamble)  		  
    		0x70,//0x70,		//RXDEM2_REG  		  
    		0xF3,		//CPC1_REG				  
    		0x33,		//CPC2_REG				  
    		0x4D,		//CRYSTALTEST_REG	
    		0x19,		//PLLTEST_REG   	  
    		0x0A,		//VCOTEST_REG 		  
    		0x00,		//RFANALOG_REG 		
    		0x00,		//KEYDATA_REG 	
        #if(RFIC_SEL==RFIC_A7196_6M)
    		0xB7,		//CHSELECT_REG	
        #elif(RFIC_SEL==RFIC_A7196_4M)
    		0x77, //0xB7,		//CHSELECT_REG	
        #endif		   
    		0x00,		//ROMP_REG 				
    		0x00,		//DATARATECLOCK		
    		0x00,		//FCR_REG 				  
    		0x00,		//ARD_REG 				  
    		0x00,		//AFEP_REG 				
    		0x00,		//FCB_REG 				  
    		0x00,		//KEYC_REG 				
    		0x00 		//USID_REG 				
    };

const BYTE A7196_Addr2A_Config[]=
{
        0x39, //page0,
        0x09, //page1,
        0xF0, //Page2,
        0x80, //page3,
        0x80, //page4,
        0x08, //page5,
        0x82, //page6,
    #if(RFIC_SEL==RFIC_A7196_6M)	    
        0xC0, //page7,
    #elif(RFIC_SEL==RFIC_A7196_4M)
        0x40, //page7,
    #endif
        0x00, //page8,
        0x3C, //page9,
        0xE8, //pageA,
        0x00, //pageB,
};

const BYTE A7196_Addr38_Config[]=
{
        0x04, //page0,
        0xE0, //page1,
        0x30, //page2,
        0xB4, //page3,
        0x20, //page4,
   #if(RFIC_SEL==RFIC_A7196_6M)		
		0x20,   //page5,
   #elif(RFIC_SEL==RFIC_A7196_4M)		
        0x30,   //page5,
   #endif 

   #if AMIC7196_USE_EXT_PA
        0x70, //page6,
   #else
        0x00, //page6,
   #endif
        0x30, //page7,
        0x01, //page8,
};

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern int gRfiuSyncWordTable[MAX_RFIU_UNIT];
extern int gRfiuCustomerCode[MAX_RFIU_UNIT];

extern int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX];


void StrobeCmd_B1(u8 cmd);
void StrobeCmd_B2(u8 cmd);
void A7196_KeyData_B1(void);
void A7196_KeyData_B2(void);
void A7196_FCB_B1(void);
void A7196_FCB_B2(void);
void ByteSendFast_B1(u8 src);
void ByteSendFast_B2(u8 src);

/*********************************************************************************************************
* External Functions
*********************************************************************************************************/
#if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
extern int sysStopCiuSensor();
extern int sysSaveAeExpoVal2UISetting();
extern void WDT_off(void);
extern int sysTxPIRTrigOnOff(int isOn);
#endif



//----------------------------//
void _nop_()
{
   int i=0;

   i++;
}

/************************************************************************
**  Reset_RF
************************************************************************/
void A7196_Reset_B1(void)
{
    A7196_WriteReg_B1(MODE_REG, 0x00); //reset RF chip
}

void A7196_Reset_B2(void)
{
    A7196_WriteReg_B2(MODE_REG, 0x00); //reset RF chip
}
/************************************************************************
**  WriteID
************************************************************************/
void A7196_WriteID_B1(void)
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(addr);

#if(USE_A7196_VER == 0)
    ID = gRfiuSyncWordTable[0];
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}
#elif(USE_A7196_VER == 1)
    ID = 0xaaaaaaaa;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}

#endif
    
    //Lucian: repeat ID. 目前syncword 僅用到32 bit.
    ID = gRfiuSyncWordTable[0];    
    //ID=RFI_SYNCID_PREFIX;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);

	addr = IDCODE_REG | 0x40; //send address 0x06, bit cmd=0, r/w=1
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(addr);
	d1=ByteRead_B1();
	d2=ByteRead_B1();
	d3=ByteRead_B1();
	d4=ByteRead_B1();
	d5=ByteRead_B1();
	d6=ByteRead_B1();
	d7=ByteRead_B1();
	d8=ByteRead_B1();
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
  #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

  #else
    DEBUG_RFIU_P2("\n AMIC A7196 ID B1 :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", d1,d2,d3,d4,d5,d6,d7,d8);
  #endif
}

void A7196_WriteID_B2(void)
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(addr);

#if(USE_A7196_VER == 0)
    ID = gRfiuSyncWordTable[1];
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}
#elif(USE_A7196_VER == 1)
    ID = 0xaaaaaaaa;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}
#endif

    //Lucian: repeat ID. 目前syncword 僅用到32 bit.
    ID = gRfiuSyncWordTable[1];
    //ID=RFI_SYNCID_PREFIX;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}

	
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);

	addr = IDCODE_REG | 0x40; //send address 0x06, bit cmd=0, r/w=1
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(addr);
	d1=ByteRead_B2();
	d2=ByteRead_B2();
	d3=ByteRead_B2();
	d4=ByteRead_B2();
	d5=ByteRead_B2();
	d6=ByteRead_B2();
	d7=ByteRead_B2();
	d8=ByteRead_B2();
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
    DEBUG_RFIU_P2("\n AMIC A7196 ID B2 :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", d1,d2,d3,d4,d5,d6,d7,d8);

}

void A7196_WriteID_B1_EXT(unsigned int MAC_ID)  //unsigned int RX_TimeCheck
{
	BYTE i;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(addr);
#if(USE_A7196_VER == 1)
    ID = 0xaaaaaaaa;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}
#endif

    ID = MAC_ID;  
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);

}


void A7196_WriteID_B2_EXT(unsigned int MAC_ID)
{
	BYTE i;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(addr);

#if(USE_A7196_VER == 1)
    ID = 0xaaaaaaaa;
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}
#endif
    ID = MAC_ID; 
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);

}

/*********************************************************************
** A7196_ReadReg_Page
*********************************************************************/
u8 A7196_ReadReg_B1_Page(u8 addr, u8 page)
{
    u8 tmp;

    A7196_WriteReg_B1(RFANALOG_REG, (A7196Config[RFANALOG_REG]&0x0F) | PageTab[page]);//page select
    tmp = A7196_ReadReg_B1(addr);
    return tmp;
}

u8 A7196_ReadReg_B2_Page(u8 addr, u8 page)
{
    u8 tmp;

    A7196_WriteReg_B2(RFANALOG_REG, (A7196Config[RFANALOG_REG]&0x0F) | PageTab[page]);//page select
    tmp = A7196_ReadReg_B2(addr);
    return tmp;
}
/*********************************************************************
** A7196_WriteReg_Page
*********************************************************************/
void A7196_WriteReg_B1_Page(u8 addr, u8 wbyte, u8 page)
{
    A7196_WriteReg_B1(RFANALOG_REG, (A7196Config[RFANALOG_REG]&0x0F) | PageTab[page]);//page select
    A7196_WriteReg_B1(addr, wbyte);
}

void A7196_WriteReg_B2_Page(u8 addr, u8 wbyte, u8 page)
{
    A7196_WriteReg_B2(RFANALOG_REG, (A7196Config[RFANALOG_REG]&0x0F) | PageTab[page]);//page select
    A7196_WriteReg_B2(addr, wbyte);
}

/************************************************************************
**  A7196_WriteReg
************************************************************************/
void A7196_WriteReg_B1(u8 addr, u8 dataByte)
{
    BYTE i;

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);

    addr |= 0x00; //bit cmd=0,r/w=0
    for(i = 0; i < 8; i++)
    {
        if(addr & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);

        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        addr = addr << 1;
    }
    _nop_();

    //send data byte
    for(i = 0; i < 8; i++)
    {
        if(dataByte & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);

        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();

        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        dataByte = dataByte<< 1;
    }

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void A7196_WriteReg_B2(u8 addr, u8 dataByte)
{
    BYTE i;

    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);

    addr |= 0x00; //bit cmd=0,r/w=0
    for(i = 0; i < 8; i++)
    {
        if(addr & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,0);

        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
        addr = addr << 1;
    }
    _nop_();

    //send data byte
    for(i = 0; i < 8; i++)
    {
        if(dataByte & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,0);

        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();

        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
        dataByte = dataByte<< 1;
    }

    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
}

/************************************************************************
**  A7196_ReadReg
************************************************************************/
u8 A7196_ReadReg_B1(u8 addr)
{
    BYTE i;
    BYTE tmp,level;
    GPIO_CFG c;


    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
    addr |= 0x40; //bit cmd=0,r/w=1
    for(i = 0; i < 8; i++)
    {

        if(addr & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);
		_nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        addr = addr << 1;
    }

    _nop_();
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,0);	  //change TXD as input pin
    _nop_();
    //read data
    for(i = 0; i < 8; i++)
    {
        gpioGetLevel( GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,&level);
        if(level)
            tmp = (tmp << 1) | 0x01;
        else
            tmp = tmp << 1;

        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
    }

    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_OUT;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as output pin
    return tmp;
}


u8 A7196_ReadReg_B2(u8 addr)
{
    BYTE i;
    BYTE tmp,level;
    GPIO_CFG c;


    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
    addr |= 0x40; //bit cmd=0,r/w=1
    for(i = 0; i < 8; i++)
    {

        if(addr & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,0);
		_nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
        addr = addr << 1;
    }

    _nop_();
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,0);	  //change TXD as input pin
    _nop_();
    //read data
    for(i = 0; i < 8; i++)
    {
        gpioGetLevel( GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,&level);
        if(level)
            tmp = (tmp << 1) | 0x01;
        else
            tmp = tmp << 1;

        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
    }

    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_OUT;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as output pin
    return tmp;
}
/************************************************************************
**  ByteSend
************************************************************************/
void ByteSend_B1(u8 src)
{
    BYTE i;

    for(i = 0; i < 8; i++)
    {
        if(src & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);

		_nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        src = src << 1;
    }
}

void ByteSend_B2(u8 src)
{
    BYTE i;

    for(i = 0; i < 8; i++)
    {
        if(src & 0x80)
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);
        else
            gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,0);

		_nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
        src = src << 1;
    }
}


void ByteSendFast_B1(u8 src)
{
    BYTE i,j;
    u32 bitDataSet = 0x00000001 << GPIO_BIT_RFI1_SDATA; 
    u32 bitClkSet  = 0x00000001 << GPIO_BIT_RFI1_SCLK; 
    volatile unsigned int *GrpDataReg;
    volatile unsigned int *GrpClkReg;

    //---//
    switch(GPIO_GROUP_RFI1CONF_SDATA)
    {
       case 0:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0004));
           break;

       case 1:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0038));
           break;

       case 2:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0048));
           break;

       case 3:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0058));
           break;

       default:
           return;
    }

    switch(GPIO_GROUP_RFI1CONF_SCLK)
    {
       case 0:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0004));
           break;

       case 1:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0038));
           break;

       case 2:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0048));
           break;

       case 3:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0058));
           break;

       default:
           return;
    }


    for(i = 0; i < 8; i++)
    {
        if(src & 0x80)
            //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
            *GrpDataReg |= bitDataSet;
        else
            //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);
            *GrpDataReg &= ~bitDataSet;


        //gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        *GrpClkReg |= bitClkSet;
        _nop_();
        //gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        *GrpClkReg &= ~bitClkSet;

        src = src << 1;
    }
}

void ByteSendFast_B2(u8 src)
{
    BYTE i,j;
    u32 bitDataSet = 0x00000001 << GPIO_BIT_RFI2_SDATA; 
    u32 bitClkSet  = 0x00000001 << GPIO_BIT_RFI2_SCLK; 
    volatile unsigned int *GrpDataReg;
    volatile unsigned int *GrpClkReg;

    //---//
    switch(GPIO_GROUP_RFI2CONF_SDATA)
    {
       case 0:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0004));
           break;

       case 1:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0038));
           break;

       case 2:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0048));
           break;

       case 3:
           GrpDataReg=((volatile unsigned *)(GpioCtrlBase + 0x0058));           
           break;

       default:
           return;
    }

    switch(GPIO_GROUP_RFI2CONF_SCLK)
    {
       case 0:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0004));
           break;

       case 1:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0038));
           break;

       case 2:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0048));
           break;

       case 3:
           GrpClkReg=((volatile unsigned *)(GpioCtrlBase + 0x0058));
           break;

       default:
           return;
    }


    for(i = 0; i < 8; i++)
    {
        if(src & 0x80)
            //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
            *GrpDataReg |= bitDataSet;
        else
            //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,0);
            *GrpDataReg &= ~bitDataSet;


        //gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        *GrpClkReg |= bitClkSet;
        _nop_();
        //gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
        *GrpClkReg &= ~bitClkSet;

        src = src << 1;
    }
}



/************************************************************************
**  ByteRead
************************************************************************/
u8 ByteRead_B1(void)
{
	BYTE i,tmp,level;
    GPIO_CFG c;
    
	c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;

	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);

    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,0);	  //change TXD as input pin
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
    for(i = 0; i < 8; i++)
    {
        //DEBUG_RFIU_P2("1");    
        gpioGetLevel( GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,&level);
        if(level)
            tmp = (tmp << 1) | 0x01;
        else
            tmp = tmp << 1;

        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
    }
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_OUT;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as input pin
    return tmp;
}

u8 ByteRead_B2(void)
{
	BYTE i,tmp,level;
    GPIO_CFG c;
    
	c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);

    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,0);	  //change TXD as input pin
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
    for(i = 0; i < 8; i++)
    {
        gpioGetLevel( GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,&level);
        if(level)
            tmp = (tmp << 1) | 0x01;
        else
            tmp = tmp << 1;

        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,1);
        _nop_();
        gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
    }

    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_OUT;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as input pin
    return tmp;
}
/*********************************************************************
** SetCH
*********************************************************************/
void SetCH_B1(u8 ch)
{
	A7196_WriteReg_B1(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}

void SetCH_B2(u8 ch)
{
	A7196_WriteReg_B2(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}
/*********************************************************************
** initRF
*********************************************************************/
void initRF_B1(void)
{
    u16 i;
    //init io pin
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
	gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);
 #if 1 //AMIC7196_USE_EXT_PA
    gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
    gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA off
 #endif     

    A7196_Reset_B1(); //reset A7105 RF chip
    A7196_WriteID_B1(); //write ID code
    A7196_Config_B1(); //config A7105 chip
    A7196_Cal_B1(); //calibration IF,VCO,VCOC
    
    StrobeCmd_B1(CMD_SLEEP);
    _nop_();
    StrobeCmd_B1(CMD_STBY);
    _nop_();
    for(i=0;i<1000;i++);
}

void initRF_B2(void)
{
    u16 i;
    //init io pin
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);	
	gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);		 
    gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);
 #if 1 //AMIC7196_USE_EXT_PA
    gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
    gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off
 #endif     

    A7196_Reset_B2(); //reset A7105 RF chip
    A7196_WriteID_B2(); //write ID code
    A7196_Config_B2(); //config A7105 chip
    A7196_Cal_B2(); //calibration IF,VCO,VCOC
    
    StrobeCmd_B2(CMD_SLEEP);
    _nop_();
    StrobeCmd_B2(CMD_STBY);
    _nop_();
    for(i=0;i<1000;i++);
}



/*********************************************************************
** Strobe Command
*********************************************************************/
void StrobeCmd_B1(u8 cmd)
{
		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);	
		ByteSend_B1(cmd);
		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void StrobeCmd_B2(u8 cmd)
{
		gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);	
		ByteSend_B2(cmd);
		gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
}



/*********************************************************************
** Err_State
*********************************************************************/
void Err_State(void)
{
    //ERR display
    //Error Proc...
    //...
    while(1);
}

/*********************************************************************
** CHGroupCal
*********************************************************************/
void CHGroupCal_B1(u8 ch)
{
    u8 tmp;
    u8 vb,vbcf,vcb,vccf,adag;
    u8 deva,adev;

	A7196_WriteReg_B1(PLL1_REG, ch);
	A7196_WriteReg_B1(CALIBRATION_REG, 0x1C);
	do{
			tmp = A7196_ReadReg_B1(CALIBRATION_REG)&0x1C;
	}while (tmp);

    //for check
    tmp = A7196_ReadReg_B1(VCOCCAL_REG);
    vcb = tmp & 0x0F;
    vccf = (tmp>>4) & 0x01;
    
    tmp = A7196_ReadReg_B1(VCOCAL1_REG);
	vb = tmp & 0x07;
	vbcf = (tmp >>3) & 0x01;

	tmp = A7196_ReadReg_B1(VCOCAL2_REG);
	adag = tmp;

	tmp = A7196_ReadReg_B1(VCODEVCAL1_REG);
	deva = tmp;

	tmp = A7196_ReadReg_B1(VCODEVCAL2_REG);
	adev = tmp;

	if(vbcf||vccf)
	{
	    DEBUG_RFIU_P2("--->CHGroupCal_B1 Error!:0x%x,0x%x\n",vbcf,vccf);
   		Err_State();//error   
	}
}

void CHGroupCal_B2(u8 ch)
{
    u8 tmp;
    u8 vb,vbcf,vcb,vccf,adag;
    u8 deva,adev;

	A7196_WriteReg_B2(PLL1_REG, ch);
	A7196_WriteReg_B2(CALIBRATION_REG, 0x1C);
	do{
			tmp = A7196_ReadReg_B2(CALIBRATION_REG)&0x1C;
	}while (tmp);

    //for check
    tmp = A7196_ReadReg_B2(VCOCCAL_REG);
    vcb = tmp & 0x0F;
    vccf = (tmp>>4) & 0x01;
    
    tmp = A7196_ReadReg_B2(VCOCAL1_REG);
	vb = tmp & 0x07;
	vbcf = (tmp >>3) & 0x01;

	tmp = A7196_ReadReg_B2(VCOCAL2_REG);
	adag = tmp;

	tmp = A7196_ReadReg_B2(VCODEVCAL1_REG);
	deva = tmp;

	tmp = A7196_ReadReg_B2(VCODEVCAL2_REG);
	adev = tmp;

	if(vbcf||vccf)
	{
	    DEBUG_RFIU_P2("--->CHGroupCal_B2 Error!:0x%x,0x%x\n",vbcf,vccf);
   		Err_State();//error   
	}
}
/*********************************************************************
** calibration
*********************************************************************/
void A7196_Cal_B1(void)
{
		u8 tmp,tmp1;
		u8 rhc,rlc,fb,fbcf,fcd,rcrs,rcts;
		u16 rct;

        //DEBUG_RFIU_P2("--cal IF start--\n");
 		StrobeCmd_B1(CMD_PLL); //calibration @PLL state

		//IF,RSSI,RC procedure
		A7196_WriteReg_B1(CALIBRATION_REG, 0x23);
		do{
				tmp = A7196_ReadReg_B1(CALIBRATION_REG)&0x23;
		}while(tmp);	

		//calibration VBC,VDC procedure
		CHGroupCal_B1(30); //calibrate channel group Bank I
		CHGroupCal_B1(90); //calibrate channel group Bank II
		CHGroupCal_B1(150); //calibrate channel group Bank III
		StrobeCmd_B1(CMD_STBY); //return to STBY state

		tmp = A7196_ReadReg_B1(IFCAL1_REG);
		fb = tmp & 0x0F;
		fbcf = (tmp>>4) & 0x01;
	
		fcd = A7196_ReadReg_B1(IFCAL2_REG)&0x1F;	
		
		rhc = A7196_ReadReg_B1(RXGAIN2_REG);
		rlc = A7196_ReadReg_B1(RXGAIN3_REG);
		
		tmp = A7196_ReadReg_B1_Page(0x2A, 8);
		rcrs = tmp & 0x0F;
		tmp1 = (tmp &0xF0) >> 4;
		
		tmp = A7196_ReadReg_B1_Page(0x2A, 9);
		rcts = (tmp & 0x03)<<4 | tmp1;
		tmp1 = (tmp & 0xF0)>>4;
		
		tmp = A7196_ReadReg_B1_Page(0x2A, 10);
		rct = tmp | ((tmp1)*256);		
		
		if(fbcf)
		{
	       DEBUG_RFIU_P2("--->A7196_Cal_B1 Error:0x%x!\n",fbcf);
		   Err_State();
		}
        //DEBUG_RFIU_P2("--cal IF end--\n");
}

void A7196_Cal_B2(void)
{
		u8 tmp,tmp1;
		u8 rhc,rlc,fb,fbcf,fcd,rcrs,rcts;
		u16 rct;

        //DEBUG_RFIU_P2("--cal IF start--\n");

 		StrobeCmd_B2(CMD_PLL); //calibration @PLL state

		//IF,RSSI,RC procedure
		A7196_WriteReg_B2(CALIBRATION_REG, 0x23);
		do{
				tmp = A7196_ReadReg_B2(CALIBRATION_REG)&0x23;
		}while(tmp);	

		//calibration VBC,VDC procedure
		CHGroupCal_B2(30); //calibrate channel group Bank I
		CHGroupCal_B2(90); //calibrate channel group Bank II
		CHGroupCal_B2(150); //calibrate channel group Bank III
		StrobeCmd_B2(CMD_STBY); //return to STBY state

		tmp = A7196_ReadReg_B2(IFCAL1_REG);
		fb = tmp & 0x0F;
		fbcf = (tmp>>4) & 0x01;
	
		fcd = A7196_ReadReg_B2(IFCAL2_REG)&0x1F;	
		
		rhc = A7196_ReadReg_B2(RXGAIN2_REG);
		rlc = A7196_ReadReg_B2(RXGAIN3_REG);
		
		tmp = A7196_ReadReg_B2_Page(0x2A, 8);
		rcrs = tmp & 0x0F;
		tmp1 = (tmp &0xF0) >> 4;
		
		tmp = A7196_ReadReg_B2_Page(0x2A, 9);
		rcts = (tmp & 0x03)<<4 | tmp1;
		tmp1 = (tmp & 0xF0)>>4;
		
		tmp = A7196_ReadReg_B2_Page(0x2A, 10);
		rct = tmp | ((tmp1)*256);		
		
		if(fbcf)
		{
		   DEBUG_RFIU_P2("--->A7196_Cal_B2 Error:0x%x!\n",fbcf);
		   Err_State();
		}
        //DEBUG_RFIU_P2("--cal IF end--\n");
}

/*********************************************************************
** A7196_Config
*********************************************************************/
void A7196_Config_B1(void)
{
	u8 i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for(i=0x01; i<=0x02; i++)
	   A7196_WriteReg_B1(i, A7196Config[i]);
	   
	gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(0x03);
	ByteSend_B1(0x3F);
	ByteSend_B1(0x00);
	gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
	    
	A7196_WriteReg_B1(0x04, A7196Config[0x04]);
	
	for(i=0x07; i<=0x29; i++)
		A7196_WriteReg_B1(i, A7196Config[i]);

	for(i=0; i<=11; i++)//0x2A DAS
		A7196_WriteReg_B1_Page(0x2A, A7196_Addr2A_Config[i], i);
	
	A7196_WriteReg_B1_Page(0x2A, A7196_Addr2A_Config[9], 9);
	A7196_WriteReg_B1(0x2A, A7196_Addr2A_Config[9]);

	A7196_WriteReg_B1_Page(0x2A, A7196_Addr2A_Config[10], 10);
	A7196_WriteReg_B1(0x2A, A7196_Addr2A_Config[10]);
	
	for (i=0x2B; i<=0x35; i++)
		A7196_WriteReg_B1(i, A7196Config[i]);

	A7196_KeyData_B1();

	A7196_WriteReg_B1(0x37, A7196Config[0x37]);
#if(USE_A7196_VER == 0)
	for (i=0; i<=4; i++)//0x38 ROM
	   A7196_WriteReg_B1_Page(0x38, A7196_Addr38_Config[i], i);
#elif(USE_A7196_VER == 1)
    for (i=0; i<=8; i++)//0x38 ROM
	   A7196_WriteReg_B1_Page(0x38, A7196_Addr38_Config[i], i);
#endif    

	for(i=0x39; i<=0x3C; i++)
			A7196_WriteReg_B1(i, A7196Config[i]);

	A7196_FCB_B1();

	A7196_WriteReg_B1(0x3E, A7196Config[0x3E]);
	A7196_WriteReg_B1(0x3F, A7196Config[0x3F]);
}   

void A7196_Config_B2(void)
{
	u8 i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for(i=0x01; i<=0x02; i++)
	   A7196_WriteReg_B2(i, A7196Config[i]);
	   
	gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(0x03);
	ByteSend_B2(0x3F);
	ByteSend_B2(0x00);
	gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
	    
	A7196_WriteReg_B2(0x04, A7196Config[0x04]);
	
	for(i=0x07; i<=0x29; i++)
		A7196_WriteReg_B2(i, A7196Config[i]);

	for(i=0; i<=11; i++)//0x2A DAS
		A7196_WriteReg_B2_Page(0x2A, A7196_Addr2A_Config[i], i);
	
	A7196_WriteReg_B2_Page(0x2A, A7196_Addr2A_Config[9], 9);
	A7196_WriteReg_B2(0x2A, A7196_Addr2A_Config[9]);

	A7196_WriteReg_B2_Page(0x2A, A7196_Addr2A_Config[10], 10);
	A7196_WriteReg_B2(0x2A, A7196_Addr2A_Config[10]);
	
	for (i=0x2B; i<=0x35; i++)
		A7196_WriteReg_B2(i, A7196Config[i]);

	A7196_KeyData_B2();

	A7196_WriteReg_B2(0x37, A7196Config[0x37]);
#if(USE_A7196_VER == 0)
	for (i=0; i<=4; i++)//0x38 ROM
		A7196_WriteReg_B2_Page(0x38, A7196_Addr38_Config[i], i);
#elif(USE_A7196_VER == 1)
    for (i=0; i<=8; i++)//0x38 ROM
		A7196_WriteReg_B2_Page(0x38, A7196_Addr38_Config[i], i);
#endif    

	for(i=0x39; i<=0x3C; i++)
		A7196_WriteReg_B2(i, A7196Config[i]);

	A7196_FCB_B2();

	A7196_WriteReg_B2(0x3E, A7196Config[0x3E]);
	A7196_WriteReg_B2(0x3F, A7196Config[0x3F]);
}

#if 1
void A7196_ChgTo_4M_B1(void)
{
    const char RegTab[20]={
    	0x15, 0x2e,
    	0x16, 0x56,
    	0x1c, 0xbc,
    	0x28, 0x70,
    	0x2b, 0xdb,
    	0x37, 0x77,
        0x35, 0x70,
        0x2a, 0x40,
        0x35, 0x50,
        0x38, 0x30
    };
    int i;
    
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B1(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
	
}



void A7196_ChgTo_4M_B2(void)
{
    const char RegTab[20]={
    	0x15, 0x2e,
    	0x16, 0x56,
    	0x1c, 0xbc,
    	0x28, 0x70,
    	0x2b, 0xdb,
    	0x37, 0x77,
        0x35, 0x70,
        0x2a, 0x40,
        0x35, 0x50,
        0x38, 0x30
    };
    int i;
    
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B2(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
	
}

void A7196_ChgTo_6M_B1(void)
{
    const char RegTab[20]={
    	0x15, 0x2f,
    	0x16, 0x40,
    	0x1c, 0xfc,
    	0x28, 0xb0,
    	0x2b, 0xff,
    	0x37, 0xb7,
        0x35, 0x70,
        0x2a, 0xc0,
        0x35, 0x50,
        0x38, 0x20
    };
    int i;
    //----------//
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B1(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void A7196_ChgTo_6M_B2(void)
{
    const char RegTab[20]={
    	0x15, 0x2f,
    	0x16, 0x40,
    	0x1c, 0xfc,
    	0x28, 0xb0,
    	0x2b, 0xff,
    	0x37, 0xb7,
        0x35, 0x70,
        0x2a, 0xc0,
        0x35, 0x50,
        0x38, 0x20
    };
    int i;
    //----------//
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B2(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
	
}


void A7196_ChgTo_3M_B1(void)
{
    const char RegTab[20]={
    	0x15, 0x2e,
    	0x16, 0x40,
    	0x1c, 0xac,
    	0x28, 0x50,
    	0x2b, 0xe4,
    	0x37, 0x57,
        0x35, 0x70,
        0x2a, 0x40,
        0x35, 0x50,
        0x38, 0x20
    };
    int i;
    //----------//
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B1(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void A7196_ChgTo_3M_B2(void)
{
    const char RegTab[20]={
    	0x15, 0x2e,
    	0x16, 0x40,
    	0x1c, 0xac,
    	0x28, 0x50,
    	0x2b, 0xe4,
    	0x37, 0x57,
        0x35, 0x70,
        0x2a, 0x40,
        0x35, 0x50,
        0x38, 0x20
    };
    int i;
    //----------//
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);

    for(i=0;i<20;i++)
       ByteSendFast_B2(RegTab[i]);

    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
}
#else
void A7196_ChgTo_4M_B1(void)
{

  /*  
    0x2E,               //TX1_REG                              
    0x56,               //TX2_REG                              
    0xBC,              //RXGAIN3_REG                    
    0x70,               //VCODEVCAL1_REG   
    0xDB,              //VCOMODDELAY_REG                 
    0x77,               //CHSELECT_REG            
    */
	A7196_WriteReg_B1(0x15, 0x2e);
	A7196_WriteReg_B1(0x16, 0x56);
	A7196_WriteReg_B1(0x1c, 0xbc);
	A7196_WriteReg_B1(0x28, 0x70);
	A7196_WriteReg_B1(0x2b, 0xdb);
	A7196_WriteReg_B1(0x37, 0x77);
    	
    // 0x40,   //page7,    
    A7196_WriteReg_B1_Page(0x2A, 0x40, 0x07);
    
    //0x30,   //page5
    A7196_WriteReg_B1_Page(0x38, 0x30, 0x05);
	
}



void A7196_ChgTo_4M_B2(void)
{

  /*  
    0x2E,               //TX1_REG                              
    0x56,               //TX2_REG                              
    0xBC,              //RXGAIN3_REG                    
    0x70,               //VCODEVCAL1_REG   
    0xDB,              //VCOMODDELAY_REG                 
    0x77,               //CHSELECT_REG            
    */
	A7196_WriteReg_B2(0x15, 0x2e);
	A7196_WriteReg_B2(0x16, 0x56);
	A7196_WriteReg_B2(0x1c, 0xbc);
	A7196_WriteReg_B2(0x28, 0x70);
	A7196_WriteReg_B2(0x2b, 0xdb);
	A7196_WriteReg_B2(0x37, 0x77);
    	
    // 0x40,   //page7,    
    A7196_WriteReg_B2_Page(0x2A, 0x40, 0x07);
    
    //0x30,   //page5
    A7196_WriteReg_B2_Page(0x38, 0x30, 0x05);
	
}

void A7196_ChgTo_6M_B1(void)
{

  /*  
    0x2f,               //TX1_REG                              
    0x40,               //TX2_REG                              
    0xfc,               //RXGAIN3_REG                    
    0xb0,               //VCODEVCAL1_REG   
    0xff,               //VCOMODDELAY_REG                 
    0xb7,               //CHSELECT_REG            
    */
	A7196_WriteReg_B1(0x15, 0x2f);
	A7196_WriteReg_B1(0x16, 0x40);
	A7196_WriteReg_B1(0x1c, 0xfc);
	A7196_WriteReg_B1(0x28, 0xb0);
	A7196_WriteReg_B1(0x2b, 0xff);
	A7196_WriteReg_B1(0x37, 0xb7);
    
    // 0xc0,   //page7,    
    A7196_WriteReg_B1_Page(0x2A, 0xc0, 0x07);
    
    //0x20,   //page5
    A7196_WriteReg_B1_Page(0x38, 0x20, 0x05);	
}

void A7196_ChgTo_6M_B2(void)
{

  /*  
    0x2f,               //TX1_REG                              
    0x40,               //TX2_REG                              
    0xfc,               //RXGAIN3_REG                    
    0xb0,               //VCODEVCAL1_REG   
    0xff,               //VCOMODDELAY_REG                 
    0xb7,               //CHSELECT_REG            
    */
	A7196_WriteReg_B2(0x15, 0x2f);
	A7196_WriteReg_B2(0x16, 0x40);
	A7196_WriteReg_B2(0x1c, 0xfc);
	A7196_WriteReg_B2(0x28, 0xb0);
	A7196_WriteReg_B2(0x2b, 0xff);
	A7196_WriteReg_B2(0x37, 0xb7);
    
    // 0xc0,   //page7,    
    A7196_WriteReg_B2_Page(0x2A, 0xc0, 0x07);
    
    //0x20,   //page5
    A7196_WriteReg_B2_Page(0x38, 0x20, 0x05);	
}


void A7196_ChgTo_3M_B1(void)
{

  /*  
    0x2e,               //TX1_REG                              
    0x40,               //TX2_REG                              
    0xac,               //RXGAIN3_REG                    
    0x50,               //VCODEVCAL1_REG   
    0xe4,               //VCOMODDELAY_REG                 
    0x57,               //CHSELECT_REG            
    */
	A7196_WriteReg_B1(0x15, 0x2e);
	A7196_WriteReg_B1(0x16, 0x40);
	A7196_WriteReg_B1(0x1c, 0xac);
	A7196_WriteReg_B1(0x28, 0x50);
	A7196_WriteReg_B1(0x2b, 0xe4);
	A7196_WriteReg_B1(0x37, 0x57);
    
    // 0x40,   //page7,    
    A7196_WriteReg_B1_Page(0x2A, 0x40, 0x07);
    
    //0x20,   //page5
    A7196_WriteReg_B1_Page(0x38, 0x20, 0x05);	
}

void A7196_ChgTo_3M_B2(void)
{

  /*  
    0x2e,               //TX1_REG                              
    0x40,               //TX2_REG                              
    0xac,               //RXGAIN3_REG                    
    0x50,               //VCODEVCAL1_REG   
    0xe4,               //VCOMODDELAY_REG                 
    0x57,               //CHSELECT_REG            
    */
	A7196_WriteReg_B2(0x15, 0x2e);
	A7196_WriteReg_B2(0x16, 0x40);
	A7196_WriteReg_B2(0x1c, 0xac);
	A7196_WriteReg_B2(0x28, 0x50);
	A7196_WriteReg_B2(0x2b, 0xe4);
	A7196_WriteReg_B2(0x37, 0x57);
    
    // 0x40,   //page7,    
    A7196_WriteReg_B2_Page(0x2A, 0x40, 0x07);
    
    //0x20,   //page5
    A7196_WriteReg_B2_Page(0x38, 0x20, 0x05);	
}

#endif
/*********************************************************************
** Write A7196_KeyData                                                   
*********************************************************************/ 
void A7196_KeyData_B1(void)
{
    u8 i;
    u8 addr;

    addr = KEYDATA_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
    ByteSend_B1(addr);
    for (i=0; i < 16; i++)
        ByteSend_B1(KeyData_Tab[i]);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void A7196_KeyData_B2(void)
{
    u8 i;
    u8 addr;

    addr = KEYDATA_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
    ByteSend_B2(addr);
    for (i=0; i < 16; i++)
        ByteSend_B2(KeyData_Tab[i]);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
}


/*********************************************************************
** Write A7196_FCB                                                   
*********************************************************************/ 
void A7196_FCB_B1(void)
{
    u8 i;
    u8 addr;

    addr = FCB_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
    ByteSend_B1(addr);
    for (i=0; i < 20; i++)
			ByteSend_B1(FCB_Tab[i]);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
}

void A7196_FCB_B2(void)
{
    u8 i;
    u8 addr;

    addr = FCB_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
    ByteSend_B2(addr);
    for (i=0; i < 20; i++)
			ByteSend_B2(FCB_Tab[i]);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
}
/*********************************************************************
** RSSI_measurement_enable
*********************************************************************/
u8 RSSI_measurement_A7196(int BoardSel)
{
    u8 tmp;

    if(BoardSel == 1)
    {
        //A7196_WriteReg_B1(MODECTRL_REG, A7196Config[MODECTRL_REG] | 0x40);	//ARSSI=1
        //A7196_WriteReg_B1(ADC_REG, A7196Config[ADC_REG]|0x01);	// CDM Enable        
        tmp= A7196_ReadReg_B1(RSSI_REG);		//read RSSI value(wanted signal RSSI)
    }
    else if(BoardSel == 2)
    {
         //A7196_WriteReg_B2(MODECTRL_REG, A7196Config[MODECTRL_REG] | 0x40);	//ARSSI=1
        //A7196_WriteReg_B2(ADC_REG, A7196Config[ADC_REG]|0x01);	// CDM Enable        
        tmp= A7196_ReadReg_B2(RSSI_REG);		//read RSSI value(wanted signal RSSI
    }

    return tmp;
}


void InitA7196()
{
       u8 level;
       int i;
       
#if ( (RFIC_SEL==RFIC_A7196_6M) )     
    #if(SYS_CPU_CLK_FREQ  == 160000000)
       #if REDUCE_HCLK_TO_128M
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00700000;  // 384/8=48
       #elif REDUCE_HCLK_TO_144M
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00800000;  // 432/9=48
       #else
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00900000;  // 480/10=48
       #endif
    #elif(SYS_CPU_CLK_FREQ  == 162000000)
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00800000;  // 486/9=54   
    #elif(SYS_CPU_CLK_FREQ  == 180000000)
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00900000;  // 540/10=54
    #endif
#elif(RFIC_SEL==RFIC_A7196_4M)   
    #if(SYS_CPU_CLK_FREQ  == 160000000)
      #if REDUCE_HCLK_TO_128M
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00b00000;  // 384/12=32       
      #elif REDUCE_HCLK_TO_144M
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00b00000;  // 432/12=36       
      #else
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00e00000;  // 480/15=32       
      #endif
    #elif(SYS_CPU_CLK_FREQ  == 162000000)
       DEBUG_RFIU_P2("=== Cannot Support RF 4Mbps ===\n");
       sysForceWDTtoReboot();
    #elif(SYS_CPU_CLK_FREQ  == 180000000)
       SYS_CLK4 = (SYS_CLK4 & (~0x00f00000)) | 0x00e00000;  // 540/15=36
    #endif
#endif

#if RFI_SELF_TEST_TXRX_PROTOCOL
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz

       initRF_B2(); //init RF
       StrobeCmd_B2(CMD_STBY);
	   SetCH_B2(100); //freq 2450MHz

#elif RFI_TEST_TX_PROTOCOL_B1
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz

#elif RFI_TEST_TX_PROTOCOL_B2
       initRF_B2(); //init RF
       StrobeCmd_B2(CMD_STBY);
	   SetCH_B2(100); //freq 2450MHz

#elif RFI_TEST_RX_PROTOCOL_B1
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz
	   
#elif RFI_TEST_RX_PROTOCOL_B2
      initRF_B2(); //init RF
      StrobeCmd_B2(CMD_STBY);
	  SetCH_B2(100); //freq 2450MHz

#elif (RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_8TX_2RX_PROTOCOL)
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz

      initRF_B2(); //init RF
      StrobeCmd_B2(CMD_STBY);
	  SetCH_B2(100); //freq 2450MHz 

#elif (RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL)
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz      
#else
   #if (RFI_FCC_DIRECT_TRX == 1)
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz
   #elif(RFI_FCC_DIRECT_TRX == 2)
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz

       initRF_B2(); //init RF
       StrobeCmd_B2(CMD_STBY);
	   SetCH_B2(100); //freq 2450MHz
   #else
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz

       initRF_B2(); //init RF
       StrobeCmd_B2(CMD_STBY);
	   SetCH_B2(100); //freq 2450MHz
   #endif

#endif

}
void A7196_TxMode_Start(int BoardSel)
{

#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    GPIO_CFG c;
#endif

    if(BoardSel==1)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        c.ena = GPIO_DISA;   // TXD ON
    	c.dir = GPIO_DIR_IN;
    	c.level = GPIO_LEVEL_LO;
    	c.inPullUp = GPIO_IN_PULLUP_ENA;
        gpioConfig(2,13,&c);

        c.ena = GPIO_ENA;  // RXD OFF
    	c.dir = GPIO_DIR_IN;
    	c.level = GPIO_LEVEL_LO;
    	c.inPullUp = GPIO_IN_PULLUP_ENA;
        gpioConfig(2,14,&c);

        //A7196_WriteReg_B1(GIO2_REG,0x24);      //GPO2 as TX
	    //StrobeCmd_B1(CMD_TX); //entry tx & transmit	

		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);	
        ByteSendFast_B1(GIO2_REG);
        ByteSendFast_B1(0x24);
		ByteSendFast_B1(CMD_TX);
		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
    #else 
	   StrobeCmd_B1(CMD_TX); //entry tx & transmit	
    #endif
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA On     
       gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA off
 #endif     
    }
    else if(BoardSel==2)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

    #endif

	   StrobeCmd_B2(CMD_TX); //entry tx & transmit	
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA On     
       gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off
 #endif     
    }
    else if(BoardSel==3)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

    #endif

	   StrobeCmd_B2(CMD_TX); //entry tx & transmit	
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA On     
       gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off
 #endif     
    }
}

void A7196_TxMode_Stop(int BoardSel)
{

    if(BoardSel==1)
    {
	   StrobeCmd_B1(CMD_STBY); //entry tx & transmit	
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA off     
       gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA off
 #endif     
    }
    else if(BoardSel==2)
    {
	   StrobeCmd_B2(CMD_STBY); //entry tx & transmit
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA off     
       gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA off
 #endif     
    }
}
void A7196_RxMode_Start(int BoardSel)
{
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    GPIO_CFG c;
#endif

    if(BoardSel==1)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
        c.ena = GPIO_ENA;     //TXD OFF      
    	c.dir = GPIO_DIR_IN;
    	c.level = GPIO_LEVEL_LO;
    	c.inPullUp = GPIO_IN_PULLUP_ENA;
        gpioConfig(2,13,&c);

        c.ena = GPIO_DISA;    //RXD ON
    	c.dir = GPIO_DIR_IN;
    	c.level = GPIO_LEVEL_LO;
    	c.inPullUp = GPIO_IN_PULLUP_ENA;
        gpioConfig(2,14,&c);

        //A7196_WriteReg_B1(GIO2_REG,0x21);      //GPO2 as RX
        //StrobeCmd_B1(CMD_RX); //entry tx & transmit
		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);	
        ByteSendFast_B1(GIO2_REG);
        ByteSendFast_B1(0x21);
		ByteSendFast_B1(CMD_RX);
		gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
    #else
        StrobeCmd_B1(CMD_RX); //entry tx & transmit
    #endif
    
    #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off    
       gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA ON
    #endif     
    }
    else if(BoardSel==2)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    
    #endif    
        StrobeCmd_B2(CMD_RX); //entry tx & transmit
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off    
       gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA ON
 #endif     
    }
    else if(BoardSel==3)
    {
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    
    #endif
        StrobeCmd_B2(CMD_RX); //entry tx & transmit
 #if 1 //AMIC7196_USE_EXT_PA
       gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off    
       gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA ON
 #endif     
    }
}

void A7196_RxMode_Stop(int BoardSel)
{
    if(BoardSel==1)
    {
        StrobeCmd_B1(CMD_STBY); //entry tx & transmit
     #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA off
     #else
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA off
     #endif         
    }
    else if(BoardSel==2)
    {
        StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA off
     #else
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA off
     #endif         
    }
    else if(BoardSel==3)
    {
        StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA off
     #else
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA off
     #endif         
    }
}

void A7196_CH_sel(int BoardSel,BYTE CH)
{
    if(BoardSel==1)
    {
        SetCH_B1(CH);
    }
    else if(BoardSel==2)
    {
        SetCH_B2(CH);
    }
    else if(BoardSel==3)
    {
        SetCH_B2(CH);
    }
}

void A7196_ID_Update(int BoardSel ,unsigned int NewMACID )
{

    if(BoardSel==1)
    {
	   A7196_WriteID_B1_EXT(NewMACID);
    }
    else if(BoardSel==2)
    {
	   A7196_WriteID_B2_EXT(NewMACID);
    }
    else if(BoardSel==3)
    {
	   A7196_WriteID_B2_EXT(NewMACID);
    }
}

/*********************************************************************
** WOR_enable
*********************************************************************/
s32 A7196_WOR_enable_B1(s32 PIR_IntvOn)
{
    u8 tmp;
    int i;
    GPIO_CFG c;


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
#if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    sysStopCiuSensor();
#endif

    OSTimeDly(3); // delay 150ms wait for TX_Task stop.
    //-----
    if(gRfiuCustomerCode[0] & 0x01)
       A7196_CH_sel(0+1,gRfiuDAT_CH_Table[RFI_BATCAM_WAKEUP_CH_HIGH]);
    else
       A7196_CH_sel(0+1,gRfiuDAT_CH_Table[RFI_BATCAM_WAKEUP_CH_LOW]);
    //---Config Wake RF-ID---//
    DEBUG_RFIU_P2("\nSync ID=0x%x\n",gRfiuSyncWordTable[0]);
    A7196_ID_Update(0+1 ,gRfiuSyncWordTable[0] ^ 0xffffffff);

    
    
    gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA off
    A7196_WriteReg_B1(CODE2_REG,0x00); //ID torence=0
    
    //Real WOR Active Period = (WOR_AC[5:0]+1) x 244us 600us(X'TAL and Regulator settling time)
    //Note : Be aware that X'TAL settling time requirement includes initial tolerance, 
    //       temperature drift, aging and crystal loading.
    //Real WOR Sleep Period = (WOR_SL[9:0]+1) x 7.8ms
    A7196_WriteReg_B1(CKO_REG,0x3A);       //CKO as FSYNC
    A7196_WriteReg_B1(GIO1_REG,0x03);      //GIO1 as WTR,signal inverse
    A7196_WriteReg_B1(GIO2_REG,0x00);      //GPO2 disable

#if 1
    //--Config Sleep time--//    
    //A7196_WriteReg_B1(RCOSC1_REG, 0xC0); //WOR_SL 7:0 WOR_SL 7:0 ,Sleep time=1500 ms
    A7196_WriteReg_B1(RCOSC1_REG, 0x31); //WOR_SL 7:0 WOR_SL 7:0 ,Sleep time=400 ms
    
    //--Config Wakeup time--//
    // 0x3f = (63+1)*244=15.6ms ( actul 13ms )
    //A7196_WriteReg_B1(RCOSC2_REG, 0x3F);//WOR_SL bit 7:6 WOR_SL 9:8 bit 5:0 WOR_AC 5:0,wake up time=15 ms
    A7196_WriteReg_B1(RCOSC2_REG, 0x0F);//WOR_SL bit 7:6 WOR_SL 9:8 bit 5:0 WOR_AC 5:0,wake up time=4 ms
#else
    //--Config Sleep time--//    
    A7196_WriteReg_B1(RCOSC1_REG, 0xff); //WOR_SL 7:0 WOR_SL 7:0 ,Sleep time=8000 ms
    
    //--Config Wakeup time--//
    // 0x3f = (63+1)*244=15.6ms ( actul 13ms )
    A7196_WriteReg_B1(RCOSC2_REG, 0xc0);//WOR_SL bit 7:6 WOR_SL 9:8 bit 5:0 WOR_AC 5:0,wake up time=0.25 ms
#endif

    StrobeCmd_B1(CMD_STBY);   
    A7196_WriteReg_B1(MODECTRL_REG, 0x68);
	DEBUG_RFIU_P2("\n=====A7196_WOR_enable_B1:End=====\n");

    //---------------//
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    #if(BTC_PCB_VERSION  == 5)
    if(gRfiuUnitCntl[0].RFpara.PIR_en)
    {
       if(PIR_IntvOn)
          sysTxPIRTrigOnOff(15);
       else
          sysTxPIRTrigOnOff(1);
    }
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_LO;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
    gpioConfig(GPIO_GROUP_MCU_WRITECTL,GPIO_BIT_MCU_WRITECTL,&c);    
	gpioConfig(GPIO_GROUP_PIRTRIG_ON,GPIO_BIT_PIRTRIG_ON,&c);
	gpioConfig(GPIO_GROUP_PIRTRIG_OFF,GPIO_BIT_PIRTRIG_OFF,&c);

    #else
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_LO;
	c.inPullUp = GPIO_IN_PULLUP_ENA;
	gpioConfig(GPIO_GROUP_PIRTRIG_ON,GPIO_BIT_PIRTRIG_ON,&c);
	gpioConfig(GPIO_GROUP_PIRTRIG_OFF,GPIO_BIT_PIRTRIG_OFF,&c);
    #endif
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
#endif    
    //----------------//
    //---Power Down---//
    DEBUG_RFIU_P2("\n=====System power down=====\n");
    sysPowerOffDirect();
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    //WDT_off();
#endif
    //----------------//
}

void A7196_WOR_enable_B2(void)
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
    OSTimeDly(3); // delay 150ms wait for TX_Task stop.
    //-----
    if(gRfiuCustomerCode[0] & 0x01)
       A7196_CH_sel(1+1,gRfiuDAT_CH_Table[RFI_BATCAM_WAKEUP_CH_HIGH]);
    else
       A7196_CH_sel(1+1,gRfiuDAT_CH_Table[RFI_BATCAM_WAKEUP_CH_LOW]);
    //---Config Wake RF-ID---//
    DEBUG_RFIU_P2("Wake-Up ID=0x%x\n",gRfiuSyncWordTable[0] ^ 0xffffffff);
    A7196_ID_Update(1+1 ,gRfiuSyncWordTable[0] ^ 0xffffffff);
  
    gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off
    A7196_WriteReg_B2(CODE2_REG,0x00); //ID torence=0
    
    //Real WOR Active Period = (WOR_AC[5:0]+1) x 244us 600us(X'TAL and Regulator settling time)
    //Note : Be aware that X'TAL settling time requirement includes initial tolerance, 
    //       temperature drift, aging and crystal loading.
    //Real WOR Sleep Period = (WOR_SL[9:0]+1) x 7.8ms
    A7196_WriteReg_B2(CKO_REG,0x3E);       //CKO as FSYNC,inverse FSYNC
    A7196_WriteReg_B2(GIO1_REG,0x03);      //GIO1 as WTR,signal inverse
    A7196_WriteReg_B2(GIO2_REG,0x00);      //GPO2 disable
        
    A7196_WriteReg_B2(RCOSC1_REG, 0xC0); //WOR_SL 7:0 WOR_SL 7:0 ,Sleep time=1500 ms
    // 0x3f = (63+1)*244=15.6ms ( actul 13ms )
    A7196_WriteReg_B2(RCOSC2_REG, 0x3F);//WOR_SL bit 7:6 WOR_SL 9:8 bit 5:0 WOR_AC 5:0,wake up time=15 ms
        
    StrobeCmd_B2(CMD_STBY);   
    A7196_WriteReg_B2(MODECTRL_REG, 0x68);
	DEBUG_RFIU_P2("\n=====A7196_WOR_enable_B2:End=====\n");

    //---Power Down---//
    DEBUG_RFIU_P2("\n=====System power down=====\n");
    sysPowerOffDirect();
#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    //WDT_off();
#endif
    //----------------//
}



#else
void A7196_ID_Update(int BoardSel ,unsigned int NewMACID )
{

}

void A7196_CH_sel(int BoardSel,BYTE CH)
{

}

#endif

