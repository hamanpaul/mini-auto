/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "msgevt.h"


#include "ssv_drv.h"
#include "ssv_drv_config.h"
#include "spi_def.h"
#include "spi.h"

//aher test for icomm
#include "gpioapi.h"
#include "board.h"

#define SPI_1 	 1
#define SPI_2 	 2

#define uint8_t u8
#define uint16_t u16
#define uint32_t u32
#define int32_t s32
#define UINT8 u8
#define UINT32 u32

//aher test for icomm
#define CS_EN 0x00000010


//#define SPI_INTERUPT_GPIO    GPIO0
//struct gpio_dev_t   gpio_dev;
void (*gpio_spi_isr)(void);


__align(4) u8 temp_wbuf[1600]={0};

static void _spi_host_cs_init(void);
static void _spi_host_irq_pin_init(void (*spi_isr)(void));
static void _spi_host_cs_high(void);
static void _spi_host_cs_low(void);
static void _spi_host_cs_init(void)
{
}

void spi_isr_thread(void *param){
}
//static uint32_t gpio_hisr_stack[4096];

static void _spi_host_irq_pin_init(void (*spi_isr)(void))
{
    //aher test for icomm

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    gpio_spi_isr = spi_isr;
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);

}

static void _spi_host_cs_high(void)
{

    	//aher test for icomm
//printf("CS_HIGH\n");	
	Spi2Ctrl &= ~CS_EN;	 
	Spi2Ctrl=0;
}

static void _spi_host_cs_low(void)
{
   //aher test for icomm
//printf("CS_LOW\n");
	Spi2Ctrl |= CS_EN;  
}

bool is_truly_isr()
{
    return TRUE;
}

bool spi_host_init(void (*spi_isr)(void))
{


    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    if(spi_isr == NULL)
    {
        SDRV_ERROR("%s(): spi_isr is a null pointer\r\n",__FUNCTION__);
        return FALSE;
    }
    
    _spi_host_cs_init();
    _spi_host_cs_high();
    _spi_host_irq_pin_init(spi_isr);

	return TRUE;
}

bool spi_ht_write(UINT8 *buf, UINT32 sizeToTransfer)
{
	printf("\x1B[96m spi_ht_write \x1B[0m\n");

}


bool spi_ht_read(UINT32 *rbuf, UINT32 sizeToTransfer)
{
	printf("\x1B[96m spi_ht_read \x1B[0m\n");

}

bool spi_host_readwrite(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u8 options, bool IsRead)
{
    bool ret=TRUE;
    int i;
	u32 unAddr;
	
//printf("SPI: Size=%d, %s\n",sizeToTransfer,(IsRead)?"Read":"Write");	
	
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE)
    {
        _spi_host_cs_low();

    }
    if(options & SPI_TRANSFER_OPTIONS_CPU_POLLING_MODE)
    {
    
        if(TRUE==IsRead)
        {
          ret = spi_master_in_cpu(0,buf,sizeToTransfer);
        }
        else
        {
			ret = spi_master_out_cpu((buf),(sizeToTransfer)); // CPU DMA mode
        } 
    }
    else
    {
        if(TRUE==IsRead)
        {
			ret = spi_master_in(0x00,buf, sizeToTransfer);
			//ret = spi_master_in_cpu(0,buf,sizeToTransfer);
        }
        else
        {
			ret = spi_master_out(buf,sizeToTransfer); // SPI DMA mode
			//ret = spi_master_out_cpu(buf,sizeToTransfer);
        }        
    }
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE)
    {
        _spi_host_cs_high();
     
    }
    return ret;

}

bool spi_host_write_only(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u8 options)
{
	printf("\x1B[96m spi_host_write_only \x1B[0m\n");

}

#if 0
bool spi_host_write(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, FALSE);
}

bool spi_host_read(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, TRUE);
}
#endif
void spi_host_irq_enable(bool enable)
{
#if (OS_CRITICAL_METHOD == 3)
		unsigned int  cpu_sr = 0;
#endif
		

#if 0
	u32 bitset = (0x00000001<<1);
	
	if(enable)
	{
        Gpio0IntEna |= bitset;
	}
	else
	{
		Gpio0IntEna &= ~bitset;
	}	
#elif 1
//20170504 Sean test for icomm
	u32 bitset = (0x00000001<<23);
	//printf("\x1B[96m IRQ START. \x1B[0m\n");
#if 0
if(enable == 1)
	printf("\x1B[96m+\x1B[0m");
else
	printf("\x1B[96m-\x1B[0m");
#endif	
	OS_ENTER_CRITICAL(); //Sean 20170626.
	if(enable)
	{
		Gpio1LevelInt |= bitset;
	}
	else
	{
		Gpio1LevelInt &= ~bitset;
	}
	OS_EXIT_CRITICAL();
#endif
}


