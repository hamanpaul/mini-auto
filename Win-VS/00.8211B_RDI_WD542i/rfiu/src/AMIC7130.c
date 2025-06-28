#include "general.h"
#include "gpioapi.h"

#if ( (RFIC_SEL==RFIC_A7130_4M) || (RFIC_SEL==RFIC_A7130_2M) || (RFIC_SEL==RFIC_A7130_3M) )
#include "rfiuapi.h"
#include "A7130reg.h"
#include "AMIC7130.h"
#include "rfiu.h"
#include "../../gpio/inc/gpioreg.h"


#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710))
extern u8  uiIsVM9710;
#endif

const BYTE  PageTab[8]={0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70};//page select

#if((HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
    (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
    (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
  //Lucian: for FPGA only, CKO isn't as RX.
const BYTE  A7130Config[]=
{
	0x00, //RESET register,			only reset, not use on config
	0x60, //MODE register     direct mode
	0x00, //CALIBRATION register,	only read, not use on config
	0x3F, //FIFO1 register,
	0x00, //FIFO2 register,
	0x00, //FIFO register,			for fifo read/write
	0x00, //IDDATA register,		for idcode
	0x00, //RCOSC1 register,
	0x00, //RCOSC2 register,
	0x00, //RCOSC3 register,
	0x3a, //CKO register,
	//0x05, //GPIO1 register                                                               
    //0x1D, //GPIO2 register, 
    0x21, //GPIO1 register: RX                                                              
    0x24, //GPIO2 register: TX       
	0x9F, //DATARATECLOCK register,
	0x00, //PLL1 register,
	0x0E, //PLL2 register, 				RFbase 2400.001MHz
	0x96, //PLL3 register,
	0x00, //PLL4 register,
	0x04, //PLL5 register,
	0x3C, //ChannelGroup1 register,
	0x78, //ChannelGroup1 register,
	0xAF, //TX1 register, 				gaussian 0.5T
	0x40, //TX2 register,
	  0x12, //DELAY1 register: PDL=010b=60us, TDL=01b=40us
	0x40, //DELAY2 register: WSEL=100b=1 ms
	0x70, //RX register,
	0x6F, //RXGAIN1 register,
	0xDF, //RXGAIN2 register,				
	0x3D, //RXGAIN3 register,
	0xE7, //RXGAIN4 register,
	0x00, //RSSI register,
	0xF1, //ADC register,
	  0x07, //CODE1 register, ID:4 byte, Preamble: 4 byte.
	  0x07, //CODE2 register, preable detect 16 bit, ID tolerance 1 bit.
	0x2A, //CODE3 register,
	0x60, //IFCAL1 register,
#if( AMIC7130_PAON ==1)
	0x7F, //IFCAL2 register,    option PA
#else
	0xFF, //IFCAL2 register,    0x7F			
#endif
	0x80, //VCOCCAL register,
	0xD0, //VCOCAL1 register,
	0x00, //VCOCAL2 register,
	0x70, //VCO deviation 1 register,
	0x00, //VCO deviation 2 register,
	0x00, //DSA register,
	0xDC, //VCO Modulation delay register,	
	0xF0, //BATTERY register,
	0x37, //TXTEST register,
	0x27, //0x47,RXDEM1 register: by pre-amble.
	0xF3, //RXDEM2 register,
	0xF0, //CPC1 register,
	0x37, //CPC2 register,
	0x51, //CRYSTAL register,0x51
	0x15, //PLLTEST register,
	0x15, //VCOTEST register,
	0x00, //RF Analog register,
	0x00, //Key data register,
	0x77, //Channel select register,
	0x00, //ROM register,
	0x00, //DataRate register,
	0x00, //FCR register,
	0x00, //ARD register,
	0x00, //AFEP register,
	0x00, //FCB register,
	0x00, //KEYC register,
	0x00  //USID register,
};

#elif (AMIC7130_PAON==1)
const BYTE  A7130Config[]=
{
	0x00, //RESET register,			only reset, not use on config
	0x60, //MODE register     direct mode
	0x00, //CALIBRATION register,	only read, not use on config
	0x3F, //FIFO1 register,
	0x00, //FIFO2 register,
	0x00, //FIFO register,			for fifo read/write
	0x00, //IDDATA register,		for idcode
	0x00, //RCOSC1 register,
	0x00, //RCOSC2 register,
	0x00, //RCOSC3 register,
	//0x32, //CKO register,	 CKO as RX pin
#if RFIU_TX_WAKEUP_SCHEME
	0x62, //C2,CKO register,	 CKO as RX pin PA option
#else
	0x3A, //CKO register,	 CKO as RX pin PA option
#endif
    //0x05, //GPIO1 register                                                               
    //0x1D, //GPIO2 register, 
    //0x00, //GPIO1 register: RX  
    0x21, //GPIO1 register: RX     
    0x24, //GPIO2 register: TX       
	0x9F, //DATARATECLOCK register,
	0x00, //PLL1 register,
	0x0E, //PLL2 register, 				RFbase 2400.001MHz
	0x96, //PLL3 register,
	0x00, //PLL4 register,
	0x04, //PLL5 register,
	0x3C, //ChannelGroup1 register,
	0x78, //ChannelGroup1 register,
	0xAF, //TX1 register, 				gaussian 0.5T
	0x40, //TX2 register,
	  0x12, //DELAY1 register: PDL=010b=60us, TDL=01b=40us
	0x40, //DELAY2 register: WSEL=100b=1 ms
	0x70, //RX register,
	0x6F, //RXGAIN1 register,
	0xDF, //RXGAIN2 register,				
	0x3D, //RXGAIN3 register,
	0xE7, //RXGAIN4 register,
	0x00, //RSSI register,
	0xF1, //ADC register,
	  0x07, //CODE1 register, ID:4 byte, Preamble: 4 byte.
#if RFI_MEASURE_RX1RX2_SENSITIVITY
      0x00,
#else
	  0x07, //CODE2 register, preable detect 16 bit, ID tolerance 1 bit.
#endif
	0x2A, //CODE3 register,
	0x60, //IFCAL1 register,
//	0xFF, //IFCAL2 register,    0x7F	
	0x7F, //IFCAL2 register,    0x7F  PA		
	0x80, //VCOCCAL register,
	0xD0, //VCOCAL1 register,
	0x00, //VCOCAL2 register,
	0x70, //VCO deviation 1 register,
	0x00, //VCO deviation 2 register,
	0x00, //DSA register,
	0xDC, //VCO Modulation delay register,	
	0xF0, //BATTERY register,
#if( (HW_BOARD_OPTION  == MR8100_GCT_LCD) )
	0x35, //TXTEST register,  //0x37:TX power=4.5 dBm, 0x35: +0.5dBm
#elif( (HW_BOARD_OPTION  == MR8100_GCT_VM9710) )
	0x35, //TXTEST register,  //0x37:TX power=4.5 dBm,0x35: +0.5dBm
#else
	0x37, //TXTEST register,  //TX power=4.5 dBm
#endif	

#if RFI_MEASURE_RX1RX2_SENSITIVITY
      0x47,
#else
	  0x27, //0x47,RXDEM1 register: by pre-amble.
#endif
	0xF3, //RXDEM2 register,
	0xF0, //CPC1 register,
	0x37, //CPC2 register,
	0x51, //CRYSTAL register,0x51
	0x15, //PLLTEST register,
	0x15, //VCOTEST register,
	0x00, //RF Analog register,
	0x00, //Key data register,
	0x77, //Channel select register,
	0x00, //ROM register,
	0x00, //DataRate register,
	0x00, //FCR register,
	0x00, //ARD register,
	0x00, //AFEP register,
	0x00, //FCB register,
	0x00, //KEYC register,
	0x00  //USID register,
};
#elif(AMIC7130_PAON==0)
const BYTE  A7130Config[]=
{
	0x00, //RESET register,			only reset, not use on config
	0x60, //MODE register     direct mode
	0x00, //CALIBRATION register,	only read, not use on config
	0x3F, //FIFO1 register,
	0x00, //FIFO2 register,
	0x00, //FIFO register,			for fifo read/write
	0x00, //IDDATA register,		for idcode
	0x00, //RCOSC1 register,
	0x00, //RCOSC2 register,
	0x00, //RCOSC3 register,
	0x32, //CKO register,	 CKO as RX pin
	//0x3A, //CKO register,	 CKO as RX pin PA option
    //0x05, //GPIO1 register                                                               
    //0x1D, //GPIO2 register, 
    0x00, //GPIO1 register: RX  
    0x24, //GPIO2 register: TX       
	0x9F, //DATARATECLOCK register,
	0x00, //PLL1 register,
	0x0E, //PLL2 register, 				RFbase 2400.001MHz
	0x96, //PLL3 register,
	0x00, //PLL4 register,
	0x04, //PLL5 register,
	0x3C, //ChannelGroup1 register,
	0x78, //ChannelGroup1 register,
	0xAF, //TX1 register, 				gaussian 0.5T
	0x40, //TX2 register,
	  0x12, //DELAY1 register: PDL=010b=60us, TDL=01b=40us
	0x40, //DELAY2 register: WSEL=100b=1 ms
	0x70, //RX register,
	0x6F, //RXGAIN1 register,
	0xDF, //RXGAIN2 register,				
	0x3D, //RXGAIN3 register,
	0xE7, //RXGAIN4 register,
	0x00, //RSSI register,
	0xF1, //ADC register,
	  0x07, //CODE1 register, ID:4 byte, Preamble: 4 byte.
	  0x07, //CODE2 register, preable detect 16 bit, ID tolerance 1 bit.
	0x2A, //CODE3 register,
	0x60, //IFCAL1 register,
	0xFF, //IFCAL2 register,    0x7F	
//	0x7F, //IFCAL2 register,    0x7F  PA		
	0x80, //VCOCCAL register,
	0xD0, //VCOCAL1 register,
	0x00, //VCOCAL2 register,
	0x70, //VCO deviation 1 register,
	0x00, //VCO deviation 2 register,
	0x00, //DSA register,
	0xDC, //VCO Modulation delay register,	
	0xF0, //BATTERY register,
	0x37, //TXTEST register,
	0x27, //0x47,RXDEM1 register: by pre-amble.
	0xF3, //RXDEM2 register,
	0xF0, //CPC1 register,
	0x37, //CPC2 register,
	0x51, //CRYSTAL register,0x51
	0x15, //PLLTEST register,
	0x15, //VCOTEST register,
	0x00, //RF Analog register,
	0x00, //Key data register,
	0x77, //Channel select register,
	0x00, //ROM register,
	0x00, //DataRate register,
	0x00, //FCR register,
	0x00, //ARD register,
	0x00, //AFEP register,
	0x00, //FCB register,
	0x00, //KEYC register,
	0x00  //USID register,
};

#endif


#if AMIC7130_PAON
const BYTE  A7130_Addr2A_Config[]=
{
	0x74, //page0,   PA setting for 17dBm TX power module		
	0x41, //page1,
	0x00, //Page2,
	0x80, //page3,
	0x80, //page4,
	0x00, //page5,
	0x00, //page6,
	0x00, //page7,
};

#else
const BYTE  A7130_Addr2A_Config[]=
{
	0x34, //page0, 2E
	0x41, //page1,
	0x00, //Page2,
	0x80, //page3,
	0x80, //page4,
	0x00, //page5,
	0x00, //page6,
	0x00, //page7,
};
#endif


const BYTE  A7130_Addr38_Config[]=
{
	0x00, //page0,
	0x10, //page1,
	0x20, //page2,
	0x24, //page3,
	0x20, //page4,
};
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern int gRfiuSyncWordTable[MAX_RF_DEVICE];
extern int gRfiuDAT_CH_Table[RFI_DAT_CH_MAX];

//----------------------------//
void _nop_()
{
   int i=0;

   i++;
}

/************************************************************************
**  Reset_RF
************************************************************************/
void A7130_Reset_B1(void)
{
	A7130_WriteReg_B1(MODE_REG, 0x00); //reset RF chip
}

void A7130_Reset_B2(void)
{
	A7130_WriteReg_B2(MODE_REG, 0x00); //reset RF chip
}

/************************************************************************
**  WriteID
************************************************************************/
void A7130_WriteID_B1(void)
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(addr);

    ID = gRfiuSyncWordTable[0];
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}
    
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
    DEBUG_RFIU_P2("\n AMIC A7130 ID B1 :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", d1,d2,d3,d4,d5,d6,d7,d8);
}

void A7130_WriteID_B2(void)
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(addr);

    ID = gRfiuSyncWordTable[1];
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B2(temp);
        ID = ID >>8;
	}

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
    DEBUG_RFIU_P2("\n AMIC A7130 ID B2 :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", d1,d2,d3,d4,d5,d6,d7,d8);

}


void A7130_WriteID_B1_EXT(unsigned int MAC_ID)  //unsigned int RX_TimeCheck
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);
	ByteSend_B1(addr);

    ID = MAC_ID;  
    for (i=0; i < 4; i++)
	{
	    temp = ID & 0x0ff;
		ByteSend_B1(temp);
        ID = ID >>8;
	}
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);

}


void A7130_WriteID_B2_EXT(unsigned int MAC_ID)
{
	BYTE i;
	BYTE d1,d2,d3,d4,d5,d6,d7,d8;
	BYTE addr;
    int ID;
    BYTE temp;

	addr = IDCODE_REG; //send address 0x06, bit cmd=0, r/w=0
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);
	ByteSend_B2(addr);

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
**
*********************************************************************/
BYTE A7130_ReadReg_B1_Page(BYTE addr, BYTE page)
{
	BYTE tmp;

	A7130_WriteReg_B1(RFANALOG_REG, A7130Config[0x35] | PageTab[page]);//page select
	tmp = A7130_ReadReg_B1(addr);
	return tmp;
}

BYTE A7130_ReadReg_B2_Page(BYTE addr, BYTE page)
{
	BYTE tmp;

	A7130_WriteReg_B2(RFANALOG_REG, A7130Config[0x35] | PageTab[page]);//page select
	tmp = A7130_ReadReg_B2(addr);
	return tmp;
}

/*********************************************************************
** A7130_WriteReg_B1_Page
*********************************************************************/
void A7130_WriteReg_B1_Page(BYTE addr, BYTE wbyte, BYTE page)
{
	A7130_WriteReg_B1(RFANALOG_REG, A7130Config[0x35] | PageTab[page]);//page select
	A7130_WriteReg_B1(addr, wbyte);
}

void A7130_WriteReg_B2_Page(BYTE addr, BYTE wbyte, BYTE page)
{
	A7130_WriteReg_B2(RFANALOG_REG, A7130Config[0x35] | PageTab[page]);//page select
	A7130_WriteReg_B2(addr, wbyte);
}

/************************************************************************
**  A7130_WriteReg_B1
************************************************************************/
void A7130_WriteReg_B1(BYTE addr, BYTE dataByte)
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

void A7130_WriteReg_B2(BYTE addr, BYTE dataByte)
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
**  A7130_ReadReg_B1
************************************************************************/
BYTE A7130_ReadReg_B1(BYTE addr)
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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as output pin
    return tmp;
}


BYTE A7130_ReadReg_B2(BYTE addr)
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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as output pin
    return tmp;
}



/************************************************************************
**  ByteSend_B1
************************************************************************/
void ByteSend_B1(BYTE src)
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

void ByteSend_B2(BYTE src)
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
/************************************************************************
**  ByteRead_B1
************************************************************************/
BYTE ByteRead_B1(void)
{
	BYTE i,tmp,level;
    GPIO_CFG c;
    
	c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_DISA;

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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
	gpioConfig(GPIO_GROUP_RFI1CONF_SDATA,GPIO_BIT_RFI1_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as input pin
    return tmp;
}

BYTE ByteRead_B2(void)
{
	BYTE i,tmp,level;
    GPIO_CFG c;
    
	c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_HI;
	c.inPullUp = GPIO_IN_PULLUP_DISA;
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
	c.inPullUp = GPIO_IN_PULLUP_DISA;
	gpioConfig(GPIO_GROUP_RFI2CONF_SDATA,GPIO_BIT_RFI2_SDATA,&c);
    //gpioSetLevel(GPIO_GROUP_RFICONF_OE, SPI_RXD,1);	  //change TXD as input pin
    return tmp;
}
/************************************************************************
**  Send4Bit
************************************************************************/
void Send4Bit_B1(BYTE src)
{
    BYTE i;

    for(i = 0; i < 4; i++)
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

void Send4Bit_B2(BYTE src)
{
    BYTE i;

    for(i = 0; i < 4; i++)
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

/*********************************************************************
** SetCH
*********************************************************************/
void SetCH_B1(BYTE ch)
{
	A7130_WriteReg_B1(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}

void SetCH_B2(BYTE ch)
{
	A7130_WriteReg_B2(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}

/*********************************************************************
** initRF
*********************************************************************/
void initRF_B1(void)
{

	gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);
		
	gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK,0);
		 
    gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA,1);

 #if AMIC7130_PAON
    gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
    gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA off
 #endif     

    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_CKO,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_GIO1,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_GIO2,1);

    A7130_Reset_B1(); //reset A7105 RF chip
	A7130_WriteID_B1(); //write ID code
	A7130_Config_B1(); //config A7105 chip
	A7130_Cal_B1(); //calibration IF,VCO,VCOC

}

void initRF_B2(void)
{

	gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,1);
		
	gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK,0);
		 
    gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA,1);

 #if AMIC7130_PAON
    gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
    gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off

 #endif 

    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_CKO,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_GIO1,1);
    //gpioSetLevel(GPIO_GROUP_RFICONF, AMIC_GIO2,1);

    A7130_Reset_B2(); //reset A7105 RF chip
	A7130_WriteID_B2(); //write ID code
	A7130_Config_B2(); //config A7105 chip
	A7130_Cal_B2(); //calibration IF,VCO,VCOC

}
 
/*********************************************************************
** Strobe Command
*********************************************************************/
void StrobeCmd_B1(BYTE cmd)
{
	gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,0);	
	Send4Bit_B1(cmd);
	gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS,1);	
}

void StrobeCmd_B2(BYTE cmd)
{
	gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS,0);	
	Send4Bit_B2(cmd);
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
    DEBUG_RFIU_P2("--->A7130 Error!\n");
    while(1);
}

/*********************************************************************
** CHGroupCal
*********************************************************************/
void CHGroupCal_B1(BYTE ch)
{
    BYTE tmp;
    BYTE vb,vbcf;
    BYTE deva,adev;

	A7130_WriteReg_B1(PLL1_REG, ch);
	A7130_WriteReg_B1(CALIBRATION_REG, 0x0C);
	do
	{
		tmp = A7130_ReadReg_B1(CALIBRATION_REG);
		tmp &= 0x0C;
	}
	while (tmp);

    //for check
    tmp = A7130_ReadReg_B1(VCOCAL1_REG);
	vb = tmp & 0x07;
	vbcf = (tmp >>3) & 0x01;

	tmp = A7130_ReadReg_B1(VCODEVCAL1_REG);
	deva = tmp;
#if 0
    if (deva<45 || deva >68)
    {
        DEBUG_RFIU_P2("--->DEVA Error,%d!\n",deva);
		Err_State();//error
    }
    else
    {
       DEBUG_RFIU_P2("--->DEVA=%d\n",deva); 
    }
#endif
	tmp = A7130_ReadReg_B1(VCODEVCAL2_REG);
	adev = tmp;
#if 1
    if (adev<43 || adev >68)
    {
        DEBUG_RFIU_P2("--->ADEV Error,%d!\n",adev);
		Err_State();//error
    }
    else
    {
       DEBUG_RFIU_P2("--->ADEV=%d\n",adev); 
    }
#endif
	if (vbcf)
	   Err_State();//error
}

void CHGroupCal_B2(BYTE ch)
{
    BYTE tmp;
    BYTE vb,vbcf;
    BYTE deva,adev;

	A7130_WriteReg_B2(PLL1_REG, ch);
	A7130_WriteReg_B2(CALIBRATION_REG, 0x0C);
	do
	{
		tmp = A7130_ReadReg_B2(CALIBRATION_REG);
		tmp &= 0x0C;
	}
	while (tmp);

    //for check
    tmp = A7130_ReadReg_B2(VCOCAL1_REG);
	vb = tmp & 0x07;
	vbcf = (tmp >>3) & 0x01;

	tmp = A7130_ReadReg_B2(VCODEVCAL1_REG);
	deva = tmp;
#if 0
    if (deva<45 || deva >68)
    {
        DEBUG_RFIU_P2("--->DEVA Error,%d!\n",deva);
		Err_State();//error
    }
    else
    {
       DEBUG_RFIU_P2("--->DEVA=%d\n",deva); 
    }
#endif
	tmp = A7130_ReadReg_B2(VCODEVCAL2_REG);
	adev = tmp;
#if 1
    if (adev<43 || adev >68)
    {
        DEBUG_RFIU_P2("--->ADEV Error,%d!\n",adev);
		Err_State();//error
    }
    else
    {
       DEBUG_RFIU_P2("--->ADEV=%d\n",adev); 
    }
#endif


	if (vbcf)
	   Err_State();//error
}
/*********************************************************************
** calibration
*********************************************************************/
void A7130_Cal_B1(void)
{
	BYTE tmp;
	BYTE fb, fbcf, fcd, vcb, vccf, rhc, rlc;

    StrobeCmd_B1(CMD_PLL); //calibration @PLL state

	//calibration IF procedure
	DEBUG_RFIU_P2("--cal IF start--\n");
	A7130_WriteReg_B1(RXGAIN3_REG, 0x1D);
	do
	{
		A7130_WriteReg_B1(CALIBRATION_REG, 0x02);
		do
		{
			tmp = A7130_ReadReg_B1(CALIBRATION_REG);
			tmp &= 0x02;
		}
		while(tmp);

		tmp = A7130_ReadReg_B1(IFCAL1_REG);
		fb = tmp & 0x0F;
	} while(fb<4 || fb>9);
    DEBUG_RFIU_P2("--cal IF end:%d--\n",tmp);
    
	A7130_WriteReg_B1(RXGAIN3_REG, 0x3D);
	//calibration RSSI, VCC procedure
	A7130_WriteReg_B1(CALIBRATION_REG, 0x11);
	do
	{
		tmp = A7130_ReadReg_B1(CALIBRATION_REG);
		tmp &= 0x11;
	}
	while(tmp);

	//calibration VBC,VDC procedure
	CHGroupCal_B1(30); //calibrate channel group Bank I
	CHGroupCal_B1(90); //calibrate channel group Bank II
	CHGroupCal_B1(150); //calibrate channel group Bank III
	StrobeCmd_B1(CMD_STBY); //return to STBY state

	//for check
	tmp = A7130_ReadReg_B1(IFCAL1_REG);
	fb = tmp & 0x0F;
	fbcf = (tmp >>4) & 0x01;

	tmp = A7130_ReadReg_B1(IFCAL2_REG);
	fcd = tmp & 0x1F;
	
	tmp = A7130_ReadReg_B1(VCOCCAL_REG) & 0x1F;
	vcb = tmp & 0x0F;
	vccf = (tmp >>4) & 0x01;

	rhc = A7130_ReadReg_B1(RXGAIN2_REG);
	rlc = A7130_ReadReg_B1(RXGAIN3_REG);

	if (fbcf || vccf)
	   Err_State();
}

void A7130_Cal_B2(void)
{
	BYTE tmp;
	BYTE fb, fbcf, fcd, vcb, vccf, rhc, rlc;

    StrobeCmd_B2(CMD_PLL); //calibration @PLL state

	//calibration IF procedure
	DEBUG_RFIU_P2("--cal IF start--\n");
	A7130_WriteReg_B2(RXGAIN3_REG, 0x1D);
	do
	{
		A7130_WriteReg_B2(CALIBRATION_REG, 0x02);
		do
		{
			tmp = A7130_ReadReg_B2(CALIBRATION_REG);
			tmp &= 0x02;
		}
		while(tmp);

		tmp = A7130_ReadReg_B2(IFCAL1_REG);
		fb = tmp & 0x0F;
	} while(fb<4 || fb>9);
    DEBUG_RFIU_P2("--cal IF end:%d--\n",tmp);


	A7130_WriteReg_B2(RXGAIN3_REG, 0x3D);
	
	//calibration RSSI, VCC procedure
	A7130_WriteReg_B2(CALIBRATION_REG, 0x11);
	do
	{
		tmp = A7130_ReadReg_B2(CALIBRATION_REG);
		tmp &= 0x11;
	}
	while(tmp);

	//calibration VBC,VDC procedure
	CHGroupCal_B2(30); //calibrate channel group Bank I
	CHGroupCal_B2(90); //calibrate channel group Bank II
	CHGroupCal_B2(150); //calibrate channel group Bank III
	StrobeCmd_B2(CMD_STBY); //return to STBY state

	//for check
	tmp = A7130_ReadReg_B2(IFCAL1_REG);
	fb = tmp & 0x0F;
	fbcf = (tmp >>4) & 0x01;

	tmp = A7130_ReadReg_B2(IFCAL2_REG);
	fcd = tmp & 0x1F;
	
	tmp = A7130_ReadReg_B2(VCOCCAL_REG) & 0x1F;
	vcb = tmp & 0x0F;
	vccf = (tmp >>4) & 0x01;

	rhc = A7130_ReadReg_B2(RXGAIN2_REG);
	rlc = A7130_ReadReg_B2(RXGAIN3_REG);

	if (fbcf || vccf)
	   Err_State();
}

/*********************************************************************
** A7130_Config_M
*********************************************************************/
void A7130_Config_B1(void)
{
	BYTE i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for (i=0x01; i<=0x04; i++)
	    A7130_WriteReg_B1(i, A7130Config[i]);

	for (i=0x07; i<=0x29; i++)
		A7130_WriteReg_B1(i, A7130Config[i]);

	for (i=0; i<=7; i++)//0x2A DAS
		A7130_WriteReg_B1_Page(0x2A, A7130_Addr2A_Config[i], i);

	for (i=0x2B; i<=0x35; i++)
		A7130_WriteReg_B1(i, A7130Config[i]);

	A7130_WriteReg_B1(0x37, A7130Config[0x37]);

	for (i=0; i<=4; i++)//0x38 ROM
		A7130_WriteReg_B1_Page(0x38, A7130_Addr38_Config[i], i);

	for (i=0x39; i<=0x3C; i++)
		A7130_WriteReg_B1(i, A7130Config[i]);

	A7130_WriteReg_B1(0x3E, A7130Config[0x3E]);
}

void A7130_Config_B2(void)
{
	BYTE i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for (i=0x01; i<=0x04; i++)
	    A7130_WriteReg_B2(i, A7130Config[i]);

	for (i=0x07; i<=0x29; i++)
		A7130_WriteReg_B2(i, A7130Config[i]);

	for (i=0; i<=7; i++)//0x2A DAS
		A7130_WriteReg_B2_Page(0x2A, A7130_Addr2A_Config[i], i);

	for (i=0x2B; i<=0x35; i++)
		A7130_WriteReg_B2(i, A7130Config[i]);

	A7130_WriteReg_B2(0x37, A7130Config[0x37]);

	for (i=0; i<=4; i++)//0x38 ROM
		A7130_WriteReg_B2_Page(0x38, A7130_Addr38_Config[i], i);

	for (i=0x39; i<=0x3C; i++)
		A7130_WriteReg_B2(i, A7130Config[i]);

	A7130_WriteReg_B2(0x3E, A7130Config[0x3E]);
}

void InitA7130()
{
       u8 level;
       int i;

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


#elif RFI_TEST_RXRX_PROTOCOL_B1B2
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz

      initRF_B2(); //init RF
      StrobeCmd_B2(CMD_STBY);
	  SetCH_B2(100); //freq 2450MHz   
	  

#elif RFI_TEST_4TX_2RX_PROTOCOL
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz

      initRF_B2(); //init RF
      StrobeCmd_B2(CMD_STBY);
	  SetCH_B2(100); //freq 2450MHz 


#elif RFI_TEST_2x_RX_PROTOCOL_B1
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz
	  

#elif RFI_TEST_4x_RX_PROTOCOL_B1
      initRF_B1(); //init RF
      StrobeCmd_B1(CMD_STBY);
	  SetCH_B1(100); //freq 2450MHz      
#else
   #if (RFI_FCC_DIRECT_TRX ==1)
       initRF_B1(); //init RF
       StrobeCmd_B1(CMD_STBY);
	   SetCH_B1(100); //freq 2450MHz
   #elif(RFI_FCC_DIRECT_TRX ==2)
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
void A7130_TxMode_Start(int BoardSel)
{

    if(BoardSel==1)
    {
       //A7130_WriteReg_B1(GPIO1_REG, 0x09);  //TX debug: TMEO
	   StrobeCmd_B1(CMD_TX); //entry tx & transmit	
     #if AMIC7130_PAON
        #if( (HW_BOARD_OPTION  == MR8100_GCT_VM9710) )
        if(uiIsVM9710)
        {
           gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA Off
           gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA ON
        }
        else
        {
           gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA Off
           gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA ON
        }
        #else
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA ON
        #endif
     #endif	   	   
    }
    else if(BoardSel==2)
    {
       //A7130_WriteReg_B2(GPIO1_REG, 0x09);  //TX debug: TMEO
	   StrobeCmd_B2(CMD_TX); //entry tx & transmit	
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA ON
     #endif	 	   
    }
    else if(BoardSel==3)
    {
       //A7130_WriteReg_B2(GPIO1_REG, 0x09);  //TX debug: TMEO
	   StrobeCmd_B2(CMD_TX); //entry tx & transmit	
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,0);   //PA ON
     #endif	 	   
    }
}

void A7130_TxMode_Stop(int BoardSel)
{

    if(BoardSel==1)
    {
	   StrobeCmd_B1(CMD_STBY); //entry tx & transmit	
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off
     #endif  	   
    }
    else if(BoardSel==2)
    {
	   StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off
     #endif 	   
    }
    else if(BoardSel==3)
    {
	   StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA Off
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off
     #endif 	   
    }
}
void A7130_RxMode_Start(int BoardSel)
{
    if(BoardSel==1)
    {
        //A7130_WriteReg_B1(GPIO1_REG, 0x05);  //RX debug: Fsync
        StrobeCmd_B1(CMD_RX); //entry tx & transmit
      #if AMIC7130_PAON
        #if( (HW_BOARD_OPTION  == MR8100_GCT_VM9710) )
        if(uiIsVM9710)
        {
            gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
            gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA ON
        }
        else
        {
            gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,0);   //PA Off     
            gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA ON
        }
        #else
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,0);   //LNA ON
        #endif

     #endif         
    }
    else if(BoardSel==2)
    {
        //A7130_WriteReg_B2(GPIO1_REG, 0x05);  //RX debug: Fsync
        StrobeCmd_B2(CMD_RX); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA ON

     #endif         
    }
    else if(BoardSel==3)
    {
        //A7130_WriteReg_B2(GPIO1_REG, 0x05);  //RX debug: Fsync
        StrobeCmd_B2(CMD_RX); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,0);   //LNA ON

     #endif         
    }
}

void A7130_RxMode_Stop(int BoardSel)
{
    if(BoardSel==1)
    {
        StrobeCmd_B1(CMD_STBY); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI1CONF_TXSW, GPIO_BIT_RFI1_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI1CONF_RXSW, GPIO_BIT_RFI1_RXSW,1);   //LNA off

     #endif         
    }
    else if(BoardSel==2)
    {
        StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off

     #endif         
    }
    else if(BoardSel==3)
    {
        StrobeCmd_B2(CMD_STBY); //entry tx & transmit
     #if AMIC7130_PAON
        gpioSetLevel(GPIO_GROUP_RFI2CONF_TXSW, GPIO_BIT_RFI2_TXSW,1);   //PA Off     
        gpioSetLevel(GPIO_GROUP_RFI2CONF_RXSW, GPIO_BIT_RFI2_RXSW,1);   //LNA off

     #endif         
    }
}

void A7130_CH_sel(int BoardSel,BYTE CH)
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

void A7130_ID_Update(int BoardSel ,unsigned int NewMACID )
{

    if(BoardSel==1)
    {
	   A7130_WriteID_B1_EXT(NewMACID);
    }
    else if(BoardSel==2)
    {
	   A7130_WriteID_B2_EXT(NewMACID);
    }
    else if(BoardSel==3)
    {
	   A7130_WriteID_B2_EXT(NewMACID);
    }
}

/*********************************************************************
** WOR_enable
*********************************************************************/
void A7130_WOR_enable_B1(void)
{
    u8 tmp;
    int i;

    DEBUG_RFIU_P2("=====A7130_WOR_enable_B1:Start=====\n");
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].TX_Task_Stop=1;
       gRfiuUnitCntl[i].TX_Wrap_Stop=1;
       gRfiuUnitCntl[i].TX_MpegEnc_Stop=1;

       gRfiuUnitCntl[i].RX_Task_Stop=1;
       gRfiuUnitCntl[i].RX_Wrap_Stop=1;
       gRfiuUnitCntl[i].RX_MpegDec_Stop=1;    
    }
    OSTimeDly(10);
    //-----
    A7130_CH_sel(0+1,gRfiuDAT_CH_Table[0]);

    A7130_WriteReg_B1(MODECTRL_REG,0x62);
    A7130_WriteReg_B1(CKO_REG,0x3A);

    //A7130_WriteReg_B1(CKO_REG,0x0a);
    //A7130_WriteReg_B1(GPIO1_REG,0x01);

    A7130_WriteReg_B1(DATARATE_REG,0xBF);  //DATARATE register

    A7130_WriteReg_B1(PLL2_REG,0x9E); //PLL2 register,                                     
    A7130_WriteReg_B1(PLL3_REG,0x4B); //PLL3 register,                                     
    A7130_WriteReg_B1(PLL5_REG,0x02); //PLL5 register,                                     
    A7130_WriteReg_B1(TX1_REG,0x2E); //TX1 register,
    A7130_WriteReg_B1(DELAY1_REG,0x10); //DELAY1 register, 
    A7130_WriteReg_B1(DELAY2_REG,0x60); //DELAY2 register,
    A7130_WriteReg_B1(RXGAIN2_REG,0xC2); //RXGAIN2 register,

    A7130_WriteReg_B1(CODE2_REG,0x07);     //CODE 2 register
    A7130_WriteReg_B1(IFCAL2_REG,0x7F); //IFCAL2 register,
    A7130_WriteReg_B1(VCOMODDELAY_REG,0xDB); //VCO Modulation delay register,

    A7130_WriteReg_B1(RXDEM1_REG,0x27);    //Rx Dem. I register
    A7130_WriteReg_B1(CRYSTALTEST_REG,0x55); //CRYSTAL register,

    A7130_WriteReg_B1_Page(0x2A, 0x74, 0);
    A7130_WriteReg_B1_Page(0x2A, 0x0a, 6);    
    //---Config Wake RF-ID---//
    DEBUG_RFIU_P2("Wake-Up ID=0x%x\n",gRfiuSyncWordTable[0] ^ 0xffffffff);
    A7130_ID_Update(0+1 ,gRfiuSyncWordTable[0] ^ 0xffffffff);
  
    //----------------------//
    A7130_WriteReg_B1(CODE1_REG,A7130Config[CODE1_REG]&0xEF); //Disable CRC
	//Real WOR Active Period = (WOR_AC[5:0]+1) x 244us – 600us(X'TAL and Regulator settling time)
	//Note : Be aware that X’tal settling time requirement includes initial tolerance, 
	//       temperature drift, aging and crystal loading.
	//Real WOR Sleep Period = (WOR_SL[9:0]+1) x 7.8ms

	A7130_WriteReg_B1(RCOSC1_REG, 0x7f);	//setup WOR Sleep time and RX time: 1 sec
	A7130_WriteReg_B1(RCOSC2_REG, 0x1f);    //wake time: 6.5 ms
	StrobeCmd_B1(CMD_STBY);				//entry standby mode

	//WOR Calibration
	A7130_WriteReg_B1(RCOSC3_REG, 0x1C);
	do{
	    tmp = A7130_ReadReg_B1(RCOSC3_REG) & 0x08;
        DEBUG_RFIU_P2("@");
	}while(tmp);

    A7130_RxMode_Start(0+1);
	A7130_WriteReg_B1(MODECTRL_REG, A7130Config[MODECTRL_REG] | 0x08);	//WORE=1 to enable WOR function
	//while(GIO2==0);		//Stay in WOR mode until receiving ID code(sync ok)
	DEBUG_RFIU_P2("\n=====A7130_WOR_enable_B1:End=====\n");

    //---Power Down---//
    DEBUG_RFIU_P2("\n=====System power down=====\n");
#if(HW_BOARD_OPTION == MR8120_TX_HECHI)
    gpioSetLevel(GPIO_GROUP_POWERKEEP, GPIO_BIT_POWERKEEP, 0);
#endif
    //----------------//
}

void A7130_WOR_enable_B2(void)
{
    u8 tmp;
    int i;

    DEBUG_RFIU_P2("=====A7130_WOR_enable_B2:Start=====\n");
    for(i=0;i<MAX_RFIU_UNIT;i++)
    {
       gRfiuUnitCntl[i].TX_Task_Stop=1;
       gRfiuUnitCntl[i].TX_Wrap_Stop=1;
       gRfiuUnitCntl[i].TX_MpegEnc_Stop=1;

       gRfiuUnitCntl[i].RX_Task_Stop=1;
       gRfiuUnitCntl[i].RX_Wrap_Stop=1;
       gRfiuUnitCntl[i].RX_MpegDec_Stop=1;    
    }
    OSTimeDly(10);
    //-----
    A7130_CH_sel(1+1,gRfiuDAT_CH_Table[0]);

    A7130_WriteReg_B2(MODECTRL_REG,0x62);
    A7130_WriteReg_B2(CKO_REG,0x3A);

    //A7130_WriteReg_B2(CKO_REG,0x0a);
    //A7130_WriteReg_B2(GPIO1_REG,0x01);

    A7130_WriteReg_B2(DATARATE_REG,0xBF);  //DATARATE register

    A7130_WriteReg_B2(PLL2_REG,0x9E); //PLL2 register,                                     
    A7130_WriteReg_B2(PLL3_REG,0x4B); //PLL3 register,                                     
    A7130_WriteReg_B2(PLL5_REG,0x02); //PLL5 register,                                     
    A7130_WriteReg_B2(TX1_REG,0x2E); //TX1 register,
    A7130_WriteReg_B2(DELAY1_REG,0x10); //DELAY1 register, 
    A7130_WriteReg_B2(DELAY2_REG,0x60); //DELAY2 register,
    A7130_WriteReg_B2(RXGAIN2_REG,0xC2); //RXGAIN2 register,

    A7130_WriteReg_B2(CODE2_REG,0x07);     //CODE 2 register
    A7130_WriteReg_B2(IFCAL2_REG,0x7F); //IFCAL2 register,
    A7130_WriteReg_B2(VCOMODDELAY_REG,0xDB); //VCO Modulation delay register,

    A7130_WriteReg_B2(RXDEM1_REG,0x27);    //Rx Dem. I register
    A7130_WriteReg_B2(CRYSTALTEST_REG,0x55); //CRYSTAL register,

    A7130_WriteReg_B2_Page(0x2A, 0x74, 0);
    A7130_WriteReg_B2_Page(0x2A, 0x0a, 6);    
    //---Config Wake RF-ID---//
    DEBUG_RFIU_P2("Wake-Up ID=0x%x\n",gRfiuSyncWordTable[0] ^ 0xffffffff);
    A7130_ID_Update(1+1 ,gRfiuSyncWordTable[0] ^ 0xffffffff);
  
    //----------------------//
    A7130_WriteReg_B2(CODE1_REG,A7130Config[CODE1_REG]&0xEF); //Disable CRC
	//Real WOR Active Period = (WOR_AC[5:0]+1) x 244us – 600us(X'TAL and Regulator settling time)
	//Note : Be aware that X’tal settling time requirement includes initial tolerance, 
	//       temperature drift, aging and crystal loading.
	//Real WOR Sleep Period = (WOR_SL[9:0]+1) x 7.8ms

	A7130_WriteReg_B2(RCOSC1_REG, 0x7f);	//setup WOR Sleep time and RX time: 1 sec
	A7130_WriteReg_B2(RCOSC2_REG, 0x1f);    // wakeup time : 6.5ms
	StrobeCmd_B2(CMD_STBY);				//entry standby mode

	//WOR Calibration
	A7130_WriteReg_B2(RCOSC3_REG, 0x1C);
	do{
	    tmp = A7130_ReadReg_B2(RCOSC3_REG) & 0x08;
        DEBUG_RFIU_P2("@");
	}while(tmp);

    A7130_RxMode_Start(1+1);
	A7130_WriteReg_B2(MODECTRL_REG, A7130Config[MODECTRL_REG] | 0x08);	//WORE=1 to enable WOR function
	//while(GIO2==0);		//Stay in WOR mode until receiving ID code(sync ok)
	DEBUG_RFIU_P2("\n=====A7130_WOR_enable_B2:End=====\n");

    //---Power Down---//

    //----------------//
}

/*********************************************************************
** RSSI_measurement_enable
*********************************************************************/
u8 RSSI_measurement_A7130(int BoardSel)
{
    u8 tmp;

    if(BoardSel == 1)
    {
        //A7130_WriteReg_B1(MODECTRL_REG, A7130Config[MODECTRL_REG] | 0x40);	//ARSSI=1
        //A7130_WriteReg_B1(ADC_REG, A7130Config[ADC_REG]|0x01);	// CDM Enable        
        tmp= A7130_ReadReg_B1(RSSI_REG);		//read RSSI value(wanted signal RSSI)
    }
    else if(BoardSel == 2)
    {
        //A7130_WriteReg_B2(MODECTRL_REG, A7130Config[MODECTRL_REG] | 0x40);	//ARSSI=1
        //A7130_WriteReg_B2(ADC_REG, A7130Config[ADC_REG]|0x01);	// CDM Enable        
        tmp= A7130_ReadReg_B2(RSSI_REG);		//read RSSI value(wanted signal RSSI
    }

    return tmp;
}


#else
void A7130_ID_Update(int BoardSel ,unsigned int NewMACID )
{

}

void A7130_CH_sel(int BoardSel,BYTE CH)
{

}
#endif
