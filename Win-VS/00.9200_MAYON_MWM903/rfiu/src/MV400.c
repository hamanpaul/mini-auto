
#include "general.h"
#include "gpioapi.h"
#include "rfiuapi.h"
#include "rfiu.h"

#if( (RFIC_SEL==RFIC_MV400_4M) || (RFIC_SEL==RFIC_MV400_2M) )
void SPI_W0_Board_1(unsigned int val);
void SPI_W0_Board_2(unsigned int val);


//new  PA 2008/6/27
unsigned int MU2302_reg[19]={
                                0x34204,
                                0x003f5,
                                0x4A06A, 
                                0x41010,
                                0x3c2d1,
                                0x2ad82,
                                0x300a3,  
                                0x28066,
                                0x11407,
                                0x091e8,
                                0x61a09,
                                0x2a07b,
                                0x0205d,
                                0x011ce,
                                0x21a39,
                                0x4904f,
                                0x4804f,
                                0x35204,
                                0x4000C
                            };

unsigned int MV400_CHsel[9]={
                                0x4000C,
                                0x4288C,
                                0x450FC,
                                0x4796C,
                                0x4A1EC,
                                0x4CA6C,
                                0x4F2DC,
                                0x51B4C,
                                0x543BC
                            };

void InitMV400()
{
    int i;

 #if RFI_SELF_TEST_TXRX_PROTOCOL
    //config Muchip board
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);


    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);
    
#elif RFI_TEST_TX_PROTOCOL_B1
    //config Muchip board
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);

#elif RFI_TEST_TX_PROTOCOL_B2
    //config Muchip board
    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);
    
#elif RFI_TEST_RX_PROTOCOL_B1
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);

#elif RFI_TEST_RX_PROTOCOL_B2
    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);

#elif RFI_TEST_RXRX_PROTOCOL_B1B2
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);

    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);

#elif RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);

    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);

#elif RFI_TEST_2x_RX_PROTOCOL_B1
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);    

#elif RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_8TX_1RX_PROTOCOL
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39); 
    
#else
    //config Muchip board
    for (i=0;i<19;i++) 
        SPI_W0_Board_1(MU2302_reg[i]);
    SPI_W0_Board_1(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
    SPI_W0_Board_1(0x691e8);
    SPI_W0_Board_1(0x31244);
    SPI_W0_Board_1(0x61a39);
    SPI_W0_Board_1(0x21a39);


    for (i=0;i<19;i++) 
        SPI_W0_Board_2(MU2302_reg[i]);
    SPI_W0_Board_2(0x31a39);
    gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
    SPI_W0_Board_2(0x691e8);
    SPI_W0_Board_2(0x31244);
    SPI_W0_Board_2(0x61a39);
    SPI_W0_Board_2(0x21a39);
#endif
}

void SPI_W0_Board_1(unsigned int val)
{
   unsigned char i;
    
   //API.ClrBit(INDEX_GPIO1_DATA,RFI_nSS);
   gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 0);
   
   for (i=0;i<19;i++)
   {
       if (val&0x40000) // sda=1
       {
          //API.SetBit(INDEX_GPIO1_DATA,RFI_DI);
          gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA, 1);
          
          //API.SetBit(INDEX_GPIO1_DATA,RFI_SCK);
          gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK, 1);
       }
       else //sda=0
       {
          //API.ClrBit(INDEX_GPIO1_DATA,RFI_DI);
          gpioSetLevel(GPIO_GROUP_RFI1CONF_SDATA, GPIO_BIT_RFI1_SDATA, 0);
             
          //API.SetBit(INDEX_GPIO1_DATA,RFI_SCK);
          gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK, 1);
       }
       val<<=1;
       //API.ClrBit(INDEX_GPIO1_DATA,RFI_SCK);
       gpioSetLevel(GPIO_GROUP_RFI1CONF_SCLK, GPIO_BIT_RFI1_SCLK, 0);
   }
   //API.SetBit(INDEX_GPIO1_DATA,RFI_nSS);
   gpioSetLevel(GPIO_GROUP_RFI1CONF_nSS, GPIO_BIT_RFI1_nSS, 1);
}

void SPI_W0_Board_2(unsigned int val)
{
   unsigned char i;
    
   //API.ClrBit(INDEX_GPIO1_DATA,RFI_nSS);
   gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 0);
   
   for (i=0;i<19;i++)
   {
       if (val&0x40000) // sda=1
       {
          //API.SetBit(INDEX_GPIO1_DATA,RFI_DI);
          gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA, 1);
          
          //API.SetBit(INDEX_GPIO1_DATA,RFI_SCK);
          gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK, 1);
       }
       else //sda=0
       {
          //API.ClrBit(INDEX_GPIO1_DATA,RFI_DI);
          gpioSetLevel(GPIO_GROUP_RFI2CONF_SDATA, GPIO_BIT_RFI2_SDATA, 0);
             
          //API.SetBit(INDEX_GPIO1_DATA,RFI_SCK);
          gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK, 1);
       }
       val<<=1;
       //API.ClrBit(INDEX_GPIO1_DATA,RFI_SCK);
       gpioSetLevel(GPIO_GROUP_RFI2CONF_SCLK, GPIO_BIT_RFI2_SCLK, 0);
   }
   //API.SetBit(INDEX_GPIO1_DATA,RFI_nSS);
   gpioSetLevel(GPIO_GROUP_RFI2CONF_nSS, GPIO_BIT_RFI2_nSS, 1);
}

void MV400_CH_sel(int BoardSel,BYTE CH)
{
    if(BoardSel==1)
    {
        SPI_W0_Board_1(MV400_CHsel[CH]);
    }
    else if(BoardSel==2)
    {
        SPI_W0_Board_2(MV400_CHsel[CH]);
    }
    else if(BoardSel==3)
    {
        SPI_W0_Board_2(MV400_CHsel[CH]);
    }
}
#endif

