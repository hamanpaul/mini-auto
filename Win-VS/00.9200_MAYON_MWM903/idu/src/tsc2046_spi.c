#include "general.h"
#if(TOUCH_PANEL == TOUCH_PANEL_DRIVER_TSC2046)
#include "board.h"
#include "gpioapi.h" 
#include "timerapi.h"
#include "../timer/inc/timer.h"

// SPI channel for touch controller
//#define TS_CHANNEL 1

#define SPI_DBG(x...) printf(x)
//#define SPI_DBG(x...)

//#define SPI_DELAY(x...)


#define ADS_START           (1 << 7)
#define ADS_A2A1A0_d_y      (1 << 4)    // differential
#define ADS_A2A1A0_d_z1     (3 << 4)    // differential
#define ADS_A2A1A0_d_z2     (4 << 4)    // differential
#define ADS_A2A1A0_d_x      (5 << 4)    // differential
#define ADS_A2A1A0_temp0    (0 << 4)    // non-differential
#define ADS_A2A1A0_vbatt    (2 << 4)    // non-differential
#define ADS_A2A1A0_vaux     (6 << 4)    // non-differential
#define ADS_A2A1A0_temp1    (7 << 4)    // non-differential
#define ADS_8_BIT           (1 << 3)
#define ADS_12_BIT          (0 << 3)
#define ADS_SER             (1 << 2)    // non-differential
#define ADS_DFR             (0 << 2)    // differential
#define ADS_PD10_PDOWN      (0 << 0)    // lowpower mode + penirq
#define ADS_PD10_ADC_ON     (1 << 0)    // ADC on
#define ADS_PD10_REF_ON     (2 << 0)    // vREF on + penirq
#define ADS_PD10_ALL_ON     (3 << 0)    // ADC + vREF on
 
#define REFOFF_IRQ_EN       (0 << 0)
#define REFOFF_IRQ_DIS      (1 << 0)
 
#define MAX_12BIT           ((1<<12)-1)
 
//* single-ended samples need to first power up reference voltage;
// * we leave both ADC and VREF powered 
//#define READ_12BIT_SER(x) (ADS_START|ADS_A2A1A0_ ## x |ADS_12_BIT|ADS_SER)
 
// leave ADC powered up (disables penirq) between differential samples
#define READ_12BIT_DFR(x) (ADS_START|ADS_A2A1A0_d_ ## x |ADS_12_BIT|ADS_DFR)
 
#define READ_X  (READ_12BIT_DFR(x)  | ADS_PD10_ADC_ON)
#define READ_Y  (READ_12BIT_DFR(y)  | ADS_PD10_ADC_ON)
#define READ_Z1 (READ_12BIT_DFR(z1) | ADS_PD10_ADC_ON)
#define READ_Z2 (READ_12BIT_DFR(z2) | ADS_PD10_ADC_ON)
#define PWRDOWN (READ_12BIT_DFR(y)  | ADS_PD10_PDOWN)
#define REF_ON  (READ_12BIT_DFR(x) | ADS_PD10_ALL_ON) 
#define REF_OFF (READ_12BIT_DFR(y) | ADS_PD10_PDOWN)
 

//static const u8 read_y  = READ_12BIT_DFR(y)  | ADS_PD10_ADC_ON;
//static const u8 read_z1 = READ_12BIT_DFR(z1) | ADS_PD10_ADC_ON;
//static const u8 read_z2 = READ_12BIT_DFR(z2) | ADS_PD10_ADC_ON;
//static const u8 read_x  = READ_12BIT_DFR(x)  | ADS_PD10_PDOWN;
 
static const u8 ref_on  =   READ_12BIT_DFR(y) | ADS_PD10_PDOWN;//ADS_PD10_PDOWN;//ADS_PD10_ADC_ON;//;//ADS_PD10_REF_ON;
static const u8 ref_off =   READ_12BIT_DFR(x) | ADS_PD10_PDOWN;//ADS_PD10_PDOWN;//ADS_PD10_ADC_ON;//;//ADS_PD10_REF_ON;
static const u8 read_z1 =   READ_12BIT_DFR(z1) | ADS_PD10_PDOWN;//ADS_PD10_PDOWN;//ADS_PD10_ADC_ON;//;//ADS_PD10_REF_ON;
 
//static const u8 ref_on  =   READ_12BIT_DFR(y) | REFOFF_IRQ_DIS;
//static const u8 ref_off =   READ_12BIT_DFR(x) | REFOFF_IRQ_EN;
//static const u8 read_z1 =   READ_12BIT_DFR(z1) | REFOFF_IRQ_DIS;
//static const u8 read_z2 =   READ_12BIT_DFR(z2) | REFOFF_IRQ_DIS;
 
#define DEBOUNCE_TOL 10
#define DEBOUNCE_REP 5
#define DEBOUNCE_MAX 10

// struct to return from ADS7846_GET_DATA
struct ads7846_sample   
{
    int pendown;    // reserved. do not use!
    unsigned int x; // raw (uncalibrated) x coord
    unsigned int y; // raw (uncalibrated) y coord
    unsigned int z; // 0 if no pressing, else otherwise ( = pressure) 
};
 
//=====================================================================================
// returns 0 if ~equals, else otherwise
static int compare_samples(const struct ads7846_sample *s1,
                           const struct ads7846_sample *s2,
                           unsigned int delta)
{
    unsigned int metric;
    
    metric = abs(s1->x - s2->x) + 
             abs(s1->y - s2->y) +
             abs(s1->z - s2->z);
 
    if(metric < delta)
        return 0;
    else
        return -1;
}

static void clear_sample(struct ads7846_sample *target)
{
    memset(target, 0, sizeof(struct ads7846_sample));
}
 
static void assign_sample(struct ads7846_sample *target, const struct ads7846_sample *source)
{
    target->x = source->x;
    target->y = source->y;
    target->z = source->z;
}

static unsigned int parse_reply(unsigned char reply[3])
{
    unsigned int data = 0;
    unsigned int result = 0;
 
    //SPI_DBG("parse_reply %x, %x, %x\n", reply[0], reply[1], reply[2]);
 
    data = reply[1];
    //data &= ~(1 << 8);    error
    data &= ~(1 << 7);
    data <<= 8;
    data |= reply[2];
    data >>= 3;
 
    result = data;
 
    return result;
}

//=====================================================================================
//PENIRQ  GPIO_0 bit_4

#define TSC2046_SPI_DELAY             20//5//10//10//100
#define TSC2046_SET_SPI_DCLK(Value)   gpioSetLevel(1, 16, Value)
#define TSC2046_GET_SPI_DCLK(Value)   gpioGetLevel(1, 16, Value)
#define TSC2046_SET_SPI_CS(Value)     gpioSetLevel(1, 17, Value)
#define TSC2046_GET_SPI_CS(Value)     gpioGetLevel(1, 17, Value)
#define TSC2046_SET_SPI_DOUT(Value)   gpioSetLevel(1, 19, Value)
#define TSC2046_GET_SPI_DI(Value)     gpioGetLevel(1, 18, Value)
/*
#define TSC2046_SET_SPI_DCLK(Value)   gpioSetLevel(1, 16, Value)
#define TSC2046_GET_SPI_DCLK(Value)   gpioGetLevel(1, 16, Value)
#define TSC2046_SET_SPI_CS(Value)     gpioSetLevel(1, 17, Value)
#define TSC2046_GET_SPI_CS(Value)     gpioGetLevel(1, 17, Value)
#define TSC2046_SET_SPI_DOUT(Value)   gpioSetLevel(1, 19, Value)
#define TSC2046_GET_SPI_DI(Value)     gpioGetLevel(1, 18, Value)
*/
//#define TSC2046_SET_SPI_DOUT(Value)   gpioSetLevel(1, 18, Value)
//#define TSC2046_GET_SPI_DI(Value)     gpioGetLevel(1, 19, Value)

void spi_delay(unsigned int TargetCnt)
{
    INT32U  t0=0, t1=0, CurrCnt=0;
    
    // timer1 => 10us unit   
    marsTimerCountRead(1, (INT32U*)&t0);
    
    while(10000)
    {
        marsTimerCountRead(1, (INT32U*)&t1);
        if(t1 != t0)
        {
            CurrCnt += (t0>t1)? (t0-t1):((TIMER1_COUNT-t1)+t0);
            if(CurrCnt > TargetCnt)
                return;
            t0 = t1;
        }
    }
    //SPI_DBG(" %x, \r\n", timerCnt);
}

static unsigned int spi_exchange_n(
    unsigned char *input,
    unsigned char *output)
{
    unsigned char RxBit, TxBit;    
    unsigned int i, j;
 
    // clear CS
    TSC2046_SET_SPI_CS(0);
    // delay
    spi_delay(TSC2046_SPI_DELAY);
    
    for(i=0; i<3; i++)
    {
      TxBit = 0x80;
      //SPI_DBG("IN=%x, ", input[i]);
      for(j=0; j<8; j++)
      {
      	// write data bit
      	if(input[i] & TxBit)
      	{
      	    //SPI_DBG("H ");
      	    TSC2046_SET_SPI_DOUT(1);
      	}
      	else
      	{
      	    //SPI_DBG("L ");
      	    TSC2046_SET_SPI_DOUT(0);
    	}
    	TxBit >>= 1;
    	
    	// set clk
        TSC2046_SET_SPI_DCLK(1);
        
        // read data bit
        TSC2046_GET_SPI_DI(&RxBit);
        output[i] = ((output[i] << 1) | RxBit);
        
        // delay
        spi_delay(TSC2046_SPI_DELAY);
        
        // clear clk
        TSC2046_SET_SPI_DCLK(0);
        
        // delay
        spi_delay(TSC2046_SPI_DELAY);
      }  
    }
    
    // set CS
    TSC2046_SET_SPI_CS(1);
    
    spi_delay(TSC2046_SPI_DELAY);
    return 0;
}


//=====================================================================================

static void take_sample(struct ads7846_sample *sample)
{
 
    unsigned char request[3][3] = {  {ref_on,  0, 0},
                                     {read_z1, 0, 0},
                                     {ref_off, 0, 0}  };
    unsigned char reply[3][3];

    spi_exchange_n( (unsigned char *)&request[0][0], (unsigned char *)&reply[0][0] );
    spi_exchange_n( (unsigned char *)&request[1][0], (unsigned char *)&reply[1][0] );
    spi_exchange_n( (unsigned char *)&request[2][0], (unsigned char *)&reply[2][0] );
    
    sample->y = parse_reply(reply[0]);
    sample->z = parse_reply(reply[1]);
    sample->x = parse_reply(reply[2]);
}

static void take_sample_with_debouncing(struct ads7846_sample *out_sample)
{
    struct ads7846_sample last_sample; // last remembered
    struct ads7846_sample curr_sample;
    unsigned int i = 3;
 
    clear_sample(&last_sample);
 
    while(i-- >= 0)
    {
        take_sample(&curr_sample);
        if(compare_samples(&last_sample, &curr_sample, DEBOUNCE_TOL) == 0)
        {
            // stable result
            assign_sample(out_sample, &curr_sample);
            break;
        }
        else
        {
            // need more samples
            assign_sample(&last_sample, &curr_sample);
        }
    }
}

 
void T2046_GetSample(unsigned int *pTouchX, unsigned int *pTouchY, unsigned int *pTouchZ)
{
    struct ads7846_sample  ObjSample;
    
    take_sample(&ObjSample);
    *pTouchX = ObjSample.x;
    *pTouchY = ObjSample.y;
    *pTouchZ = ObjSample.x;
    
}
#endif /*end of #if(TOUCH_PANEL == TOUCH_PANEL_DRIVER_TSC2046)*/

