/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    main.c

Abstract:

    The routines of main function.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"

#include "task.h"
#include "main.h"
#include "stmemapi.h"
#include "intapi.h"
#include "debugapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "dpofapi.h"
#include "sysapi.h"

#include "mpeg4api.h"                   /* FIQ */
#include "H264api.h"                   /* FIQ */
#include "jpegapi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "iduapi.h"

#include "timerapi.h"                   /* IRQ */
//#include "dmaapi.h"
#include "gpioapi.h"
#include "smcapi.h"
#include "sdcapi.h"
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#include "cfapi.h"
#include "ciuapi.h"
#include "rfiuapi.h"
#include "mcpuapi.h"
#include "ima_adpcm_api.h"
#include "encrptyapi.h"
#endif
#include "usbapi.h"
#include "rtcapi.h"
#include "uartapi.h"
#include "i2capi.h"
#include "iisapi.h"
#include "adcapi.h"
#include "hiapi.h"

#include "uiapi.h"
#include "mp4api.h"
#include "asfapi.h"
#include "aviapi.h" /* Peter 0704 */
#include "board.h"
#include "../../ui/inc/ui.h"
#ifdef  EXT_CACHE_SUPPORT
#include "cachereg.h"
#endif
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#include "intapi.h"
#include <mars_int.h>
#include <mars_dma.h>
#include <mars_timer.h>

void main_intInit(void);

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
OS_STK mainTaskStack[MAIN_TASK_STACK_SIZE]; /* main task stack */
FS_DISKFREE_T global_diskInfo;
u8 got_disk_info;
u8  gInsertNAND;
u8 PowerOn_HotPlug;  //判斷是否為 Power-on, HotPlug status.
u8 Main_Init_Ready = 0;
u8 Main_LCD_Ready=0;
u8 configsalix = 0;


/*
 *********************************************************************************************************
 *                                               Extern VARIABLES
 *********************************************************************************************************
 */

extern u32  Image$$ROM_CODE$$Base;
extern u32  Image$$ROM_CODE$$Limit;
extern u32  Image$$ROM_CODE$$Length;

/*
*********************************************************************************************************
*                                               Extern Function
*********************************************************************************************************
*/
extern u8 *initMemoryPool(u8 *addr);  //Lucian 070419

/* variables for time measurement: */
/* end of variables for time measurement */

extern void GPIIntHandler(void);
#if(USB_HOST == 1)
extern void usbHostIntHandler(void);
#elif(USB_DEVICE == 1)
extern void usbDeviceIntHandler(void);
#endif
#if USB2WIFI_SUPPORT
extern void usbVCntHandler(void);
#endif

extern void spiIntHandler(void);

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

/*

Routine Description:

    The main task routine.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/

#if 1
typedef unsigned int datum; /* Set the data bus width to 32 bits. */
datum memTestDataBus(volatile datum * address)
{
    datum pattern;

    /* Perform a walking 1's test at the given address.*/
    for (pattern = 1; pattern != 0; pattern <<= 1)
    {
        /* Write the test pattern.*/
        *address = pattern;
        /* Read it back (immediately is okay for this test).*/
        if (*address != pattern)
        {
            return (pattern);
        }
    }
    return (0);
}
datum * memTestAddressBus(volatile datum * baseAddress, unsigned long nBytes)
{
    unsigned long addressMask = (nBytes - 1);
    unsigned long offset;
    unsigned long testOffset;
    datum pattern = (datum) 0xAAAAAAAA;
    datum antipattern = (datum) 0x55555555;
    /*
    * Write the default pattern at each of the power-of-two offsets.
    */
    for (offset = sizeof(datum); (offset & addressMask) != 0; offset <<= 1)
    {
        baseAddress[offset] = pattern;
    }
    /*
    * Check for address bits stuck high.
    */
    testOffset = 0;
    baseAddress[testOffset] = antipattern;
    for (offset = sizeof(datum); (offset & addressMask) != 0; offset <<= 1)
    {
        if (baseAddress[offset] != pattern)
        {
            return ((datum *) &baseAddress[offset]);
        }
    }
    baseAddress[testOffset] = pattern;
    /*
    * Check for address bits stuck low or shorted.
    */
    for (testOffset = sizeof(datum); (testOffset & addressMask) != 0; testOffset <<= 1)
    {
        baseAddress[testOffset] = antipattern;
        for (offset = sizeof(datum); (offset & addressMask) != 0; offset <<= 1)
        {
            if ((baseAddress[offset] != pattern) && (offset != testOffset))
            {
                return ((datum *) &baseAddress[testOffset]);
            }
        }
        baseAddress[testOffset] = pattern;
    }
    return (0);
} /* memTestAddressBus() */

datum * memTestDevice(volatile datum * baseAddress, unsigned long nBytes)
{
    unsigned long offset;
    unsigned long nWords = nBytes / sizeof(datum);
    datum pattern;
    datum antipattern;
    /*
    * Fill memory with a known pattern.
    */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        baseAddress[offset] = pattern;
    }
    /*
    * Check each location and invert it for
    the second pass.
    
    */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        if (baseAddress[offset] != pattern)
        {
            return ((datum *) &baseAddress[offset]);
        }
        antipattern = ~pattern;
        baseAddress[offset] = antipattern;
    }
    /*
    * Check each location for the inverted pattern and zero it.
    */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        antipattern = ~pattern;
        if (baseAddress[offset] != antipattern)
        {
            return ((datum *) &baseAddress[offset]);
        }
        baseAddress[offset] = 0;
    }
    return (0);
} /* memTestDevice() */

#endif
u8 SDRAM_Test(u32* test_base1)
{
    u32             i;
    u32             testsize;
    volatile u32*   test_base;

    test_base   = test_base1;
    DEBUG_MAIN("Start SDRAM Test......................... \r\n");
    DEBUG_MAIN("Testing 0x%x ~ 0x%x\r\n", (u32)test_base, (u32)DRAM_MEMORY_END);
    if( memTestDataBus(test_base))
    {
        DEBUG_MAIN("memTestDataBus Fail\n");
        return 0;
    }
 
    if( memTestAddressBus(test_base, 65536) )
    {
       DEBUG_MAIN("memTestAddressBus Fail\n");
       return 0;
    }
    
    DEBUG_MAIN("Scan SDRAM first\n");
    testsize=DRAM_MEMORY_END-(u32)test_base;
    for (i=0;i<(testsize/4);i++)    // 64K as step
    {
        if(((u32)test_base & 0xffff) == 0)
            DEBUG_MAIN(".");
        *test_base=(u32)0x5a5a5a5a;
        if(*test_base != (u32)0x5a5a5a5a)
        {
           DEBUG_MAIN("Scan SDRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x,0x5a5a5a5a \n",*test_base);
           return 0;
        }

        *test_base= (u32)0xa5a5a5a5;
        if(*test_base != (u32)0xa5a5a5a5)
        {
           DEBUG_MAIN("Scan SDRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x, 0xa5a5a5a5\n",*test_base);
           return 0;
        }

        *test_base=0x12345678;
        if(*test_base != 0x12345678)
        {
           DEBUG_MAIN("Scan SDRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x,0x12345678 \n",*test_base);
           return 0;
        }
        *test_base=0x87654321;
        if(*test_base != 0x87654321)
        {
           DEBUG_MAIN("Scan SDRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x,0x87654321 \n",*test_base);
           return 0;
        }
        *test_base = (u32)test_base;
        test_base++;
    }
    DEBUG_MAIN("\nScan SDRAM second\n");
    test_base   = test_base1;
    for (i=0;i<(testsize/4);i++)    // 64K as step
    {
        if(((u32)test_base & 0xffff) == 0)
            DEBUG_MAIN(".");
        if(*test_base != (u32)test_base)
        {
           DEBUG_MAIN("Scan SDRAM Fail\n");
           DEBUG_MAIN("test_data 0x%x, 0x%x\n",*test_base, (u32)test_base);
           return 0;
        }
        test_base++;
    }
    DEBUG_MAIN("SDRAM Test Complete......................... \r\n");
    return 1;
}



#if EMBEDED_SRAM_TEST

#define EMBEDED_SRAM_SIZE  4096
int Embeded_SRAM_test(void)
{
    u32             i;
    u32             testsize;
    volatile u32*   test_base;  

    SYS_CTL0 |= 0x02;

    testsize= EMBEDED_SRAM_SIZE;
    test_base=(u32 *)EmbedSramBase;

    for (i=0;i<(testsize/4);i++)    // 64K as step
    {
        
        *test_base=(u32)0x5a5a5a5a;
        if(*test_base != (u32)0x5a5a5a5a)
        {
           DEBUG_MAIN("Scan SRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x,0x5a5a5a5a \n",*test_base);
           return 0;
        }

        *test_base= (u32)0xa5a5a5a5;
        if(*test_base != (u32)0xa5a5a5a5)
        {
           DEBUG_MAIN("Scan SRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x, 0xa5a5a5a5\n",*test_base);
           return 0;
        }

        *test_base=(u32)0x12345678;
        if(*test_base != (u32)0x12345678)
        {
           DEBUG_MAIN("Scan SRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x,0x12345678 \n",*test_base);
           return 0;
        }

        *test_base= (u32)0x87654321;
        if(*test_base != (u32)0x87654321)
        {
           DEBUG_MAIN("Scan SRAM Fail\n");
           DEBUG_MAIN("test_address %x \n",(u32)test_base);
           DEBUG_MAIN("test_data 0x%x, 0x87654321\n",*test_base);
           return 0;
        }
        
        test_base ++;
    }


    SYS_CTL0 &= ~0x02;

    return 1;
}
#endif

#ifdef EXT_CACHE_SUPPORT
/*

Routine Description:

    Enable External I-Cache.

Arguments:

    burst8  - 0: Burst 16.
              1: Burst 8.

Return Value:

    Void.

*/
void ExtCacheEnable(int burst8)
{
    u32 StartPage;
    u32 EndPage;
    u32 Value;

    StartPage   = ((u32)&Image$$ROM_CODE$$Base) & CACHE_PAGE_MASK;
    EndPage     = ((u32)&Image$$ROM_CODE$$Limit) & CACHE_PAGE_MASK;
    //Value       = (StartPage << CACHE_START_PAGE_SHIFT) | (EndPage >> CACHE_END_PAGE_SHIFT) | BURST8_MODE;
    Value       = (StartPage << CACHE_START_PAGE_SHIFT) | (EndPage >> CACHE_END_PAGE_SHIFT) | burst8;
                     
    CACHE_CTL  |= CACHE_STOP;   // disable cache
    CACHE_CTL   = CACHE_STOP | Value;
    CACHE_CTL   = Value;
    
    DEBUG_MAIN("\n************Cache Enable*************************\n");
    DEBUG_MAIN("Image$$ROM_CODE$$Base   = 0x%08x\n", &Image$$ROM_CODE$$Base);
    DEBUG_MAIN("Image$$ROM_CODE$$Limit  = 0x%08x\n", &Image$$ROM_CODE$$Limit);
    DEBUG_MAIN("Image$$ROM_CODE$$Length = 0x%08x\n", &Image$$ROM_CODE$$Length);
    DEBUG_MAIN("CACHE_CTL = 0x%08x\n", CACHE_CTL);
}

void ExtCacheDisable(void)
{
    CACHE_CTL  |= CACHE_STOP;   // disable cache
}
#endif

/*

Routine Description:

    The main function.

Arguments:

    None.

Return Value:

    None.

*/
void main(void)
{
    int ver;
    u8 *bottom_MemPool;
    
    // WDT Disable from boot code enable
    WDTctrBase = (8 | WDT_PULSEWIDTH);

#ifdef MMU_SUPPORT
    Read_cache_info();
    MMU_mapping();              // Create MMU Table
#endif

#ifdef EXT_CACHE_SUPPORT
    ExtCacheEnable(1);
#endif
    stmemInit();       /* initialize shadow of interrupt vector */
    main_intInit();
                     
                     
    DEBUG_MAIN("\n=============================================================================\n");
    bottom_MemPool=initMemoryPool((u8 *)MEMORY_POOL_START_ADDR);  //Lucian 070419
    DEBUG_MAIN("bottom_MemPool =>0x%08x --> 0x%08x.\n", MEMORY_POOL_START_ADDR,bottom_MemPool);

    if (bottom_MemPool > (u8 *)DRAM_MEMORY_END)
    {
        DEBUG_WARERR("##########################################\r\n");
        DEBUG_WARERR("##    MEMORY POOL Overflow 0x%08x   ##\r\n", bottom_MemPool);
        DEBUG_WARERR("##########################################\r\n");
        while(1);

    }
    //DEBUG_MAIN("Trace: uC/OS-II - The Real-Time Kernel.\n");
    //DEBUG_MAIN("Trace: OS initializing.\n");
    /* Start timer */
    OSInit();
    
    // init DMA semaphore & flag                 
    marsDMAInit();
    mcpuInit();

    // init Timer semaphore & flag
    marsTimerInit();
    
    ver=OSVersion();
    ver = ver;  /* acoid warning */
    //DEBUG_MAIN("Trace: OS version: %d.\n", ver);

#if EMBEDED_SRAM_TEST
    if(1==Embeded_SRAM_test())
    {
       DEBUG_MAIN("Embeded_SRAM_test is PASS\n");
    }
    else
       DEBUG_MAIN("Embeded_SRAM_test is FAIL\n");
#endif

    /* Create the task */
    //DEBUG_MAIN("Trace: MAIN task creating.\n");
    OSTaskCreate(MAIN_TASK, MAIN_TASK_PARAMETER, MAIN_TASK_STACK, MAIN_TASK_PRIORITY_START);

    /* start the system */
    DEBUG_MAIN("Trace: OS starting.\n");
    OSStart();

    /* never reached */
}

void main_intInit(void)
{
    // initial IRQ FIQ
    marsIntInit();

	//----- IRQ -----//
    //marsIntIRQDefIsr(INT_IRQID_DMA, dmaIntHandler);  //Lucian: 放在marsDMAInit() 內做.
    //marsIntIRQDefIsr(INT_IRQID_TIMER, timerIntHandler);  //Lucian: 放在marsTimerInit()內做.
    marsIntIRQDefIsr(INT_IRQID_IIS, iisIntHandler);
    marsIntIRQDefIsr(INT_IRQID_ADC, adcIntHandler);

    marsIntIRQDefIsr(INT_IRQID_UART_1, uartIntHandler);
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    marsIntIRQDefIsr(INT_IRQID_UART_2, uart2IntHandler);
    marsIntIRQDefIsr(INT_IRQID_UART_3, uart3IntHandler);
#endif    
    marsIntIRQDefIsr(INT_IRQID_I2C, i2cIntHandler);

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
	marsIntIRQDefIsr(INT_IRQID_CF, cfIntHandler);
    //marsIntIRQDefIsr(INT_IRQID_UART_3, uartIntHandler_3);
    //marsIntIRQDefIsr(INT_IRQID_UART_2, uartIntHandler_2);
    marsIntIRQDefIsr(INT_IRQID_RFI,  rfiuIntHandler);
    marsIntIRQDefIsr(INT_IRQID_GPIU, GPIIntHandler);
    marsIntIRQDefIsr(INT_IRQID_GPIO_1, gpio_1_IntHandler);
    marsIntIRQDefIsr(INT_IRQID_MCP,  mcpuIntHandler);
    marsIntIRQDefIsr(INT_IRQID_MD,  mdIntHandler);
    
#endif

#if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    marsIntIRQDefIsr(INT_IRQID_MCP2,  mcpu2IntHandler);
#endif
    marsIntIRQDefIsr(INT_IRQID_SDC, sdcIntHandler);

    marsIntIRQDefIsr(INT_IRQID_GPIO_0, gpioIntHandler);
#if ( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION  == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    marsIntIRQDefIsr(INT_IRQID_IR,  IRIntHandler);
#endif
#if (USB_HOST == 1)
    marsIntIRQDefIsr(INT_IRQID_USB, usbHostIntHandler);
#elif (USB_DEVICE == 1)
    marsIntIRQDefIsr(INT_IRQID_USB, usbDeviceIntHandler);
#else
#if USB2WIFI_SUPPORT
    marsIntIRQDefIsr(INT_IRQID_USB, usbVCntHandler);
#endif
  //  marsIntIRQDefIsr(INT_IRQID_USB, usbIntHandler);
#endif
    marsIntIRQDefIsr(INT_IRQID_RTC, rtcIntHandler);

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
    //--SMC--//    
    marsIntIRQDefIsr(INT_IRQID_SMC, smcIntHandler);
#endif
     

#if ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON) || (FLASH_OPTION == FLASH_SERIAL_SST))
    //--SPI Flash--//    
    marsIntIRQDefIsr(INT_IRQID_SPI, spiIntHandler);
#endif

    //------FIQ------//
    marsIntFIQDefIsr(INT_FIQID_ISU,  isuIntHandler);
    marsIntFIQDefIsr(INT_FIQID_IDU,   iduIntHandler);
    marsIntFIQDefIsr(INT_FIQID_SIU,   siuIntHandler);


    #if(VIDEO_CODEC_OPTION==MPEG4_CODEC)
    #if MULTI_CHANNEL_VIDEO_REC
    marsIntFIQDefIsr(INT_FIQID_MPEG4, MultiChannelMPEG4IntHandler);
    #else
    marsIntFIQDefIsr(INT_FIQID_MPEG4, mpeg4IntHandler);
    #endif
    #elif(VIDEO_CODEC_OPTION==H264_CODEC)
    marsIntFIQDefIsr(INT_FIQID_H264, H264IntHandler);
    #endif

    marsIntFIQDefIsr(INT_FIQID_JPEG, jpegIntHandler);
    marsIntFIQDefIsr(INT_FIQID_IPU, ipuIntHandler);
    //marsIntFIQDefIsr(INT_FIQID_HIU, hiuIntHandler);


#if( (CHIP_OPTION  == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION  == CHIP_A1016A)|| (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    marsIntFIQDefIsr(INT_FIQID_SUBTV, subTVIntHandler);
    marsIntFIQDefIsr(INT_FIQID_CIU1, ciuIntHandler_CH1);
    marsIntFIQDefIsr(INT_FIQID_CIU2, ciuIntHandler_CH2);
    marsIntFIQDefIsr(INT_FIQID_CIU3, ciuIntHandler_CH3);
    marsIntFIQDefIsr(INT_FIQID_CIU4, ciuIntHandler_CH4);
    #if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA)
    marsIntFIQDefIsr(INT_FIQID_DIU, diuIntHandler);
    #endif
#endif

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    marsIntFIQDefIsr(INT_FIQID_DES,  encryptIntHandler);
#endif

    marsIntFIQEnable( INT_FIQMASK_SIU
                     |INT_FIQMASK_IDU
                     |INT_FIQMASK_ISU
                     |INT_FIQMASK_JPEG
               #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                (CHIP_OPTION == CHIP_A1026A))
                     |INT_FIQMASK_SUBTV 
                     |INT_FIQMASK_CIU1 
                     |INT_FIQMASK_CIU2 
                     |INT_FIQMASK_CIU3 
               #endif
               #if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA)
                     |INT_FIQMASK_DIU 
               #endif
               #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                     |INT_FIQMASK_DES
               #endif
               #if(VIDEO_CODEC_OPTION==MJPEG_CODEC)
                                      );
               #elif(VIDEO_CODEC_OPTION==MPEG4_CODEC)
                     |INT_FIQMASK_MPEG4);
               #elif(VIDEO_CODEC_OPTION==H264_CODEC)
                     |INT_FIQMASK_H264);
               #endif
    marsIntIRQEnable(
			             INT_IRQMASK_TIMER |
	                     INT_IRQMASK_DMA |
	                     INT_IRQMASK_I2C | 
	                     //INT_IRQMASK_GPIO_0 |
	                     INT_IRQMASK_IIS |
	               #if( (CHIP_OPTION  == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION  == CHIP_A1016A) || \
                     (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                     (CHIP_OPTION == CHIP_A1026A))
	                	 //INT_IRQMASK_GPIU|
	                     INT_IRQMASK_CF  |
	                     //INT_IRQMASK_GPIO_1 |
	                     INT_IRQMASK_RFI |
	                     INT_IRQMASK_MD |
	                     INT_IRQMASK_UART_2 |
	                     INT_IRQMASK_UART_3 |
	                     INT_IRQMASK_MCP    |	                     
	                     INT_IRQID_GPIU |
	                     INT_IRQMASK_IR |
	               #else
	                     INT_IRQMASK_SMC |
	               #endif

                   #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                         INT_IRQMASK_MCP2   |
                   #endif
	                     INT_IRQMASK_SDC |
	                     INT_IRQMASK_RTC |
	                     INT_IRQMASK_SPI |
	                     INT_IRQMASK_UART |
	                     INT_IRQMASK_USB |
	                     0x0
	                );
                        
}

#if MCPU_TEST

void Sw_BlockCpy( u8 *DstAddr, u8 *SrcAddr, 
                        int Width, int Height,
                        int Stride
                      )
{
   int ByteCnt;
   int i,j;
   u8 *pp;
   //---------------//
   ByteCnt =Width * Height;

   if( (ByteCnt<16) || ((int)SrcAddr & 0x03) || ((int)DstAddr & 0x03) || (Width & 0x03) )
   {
      DEBUG_MAIN_ERR("MCPU's Paramater is illegle!\n");
      return;
   }

   pp=DstAddr;
   for(j=0;j<Height;j++)
   {
       for(i=0;i<Width;i++)
       {
          pp[i] = *SrcAddr;
          SrcAddr ++;
       }
       pp += Stride;
   }

}


#define MCPU_TEST_ADDRESS 0x81c00000
#define MCPU_TESTSIZE  2048
int marsMcpu_Test()
{
    u8 *pp,*qq,*rr;
    unsigned int *wptr;
    int i,j,k;
    int ZeroCount,val,temp;
    int src_offset,dst_offset,testbyte;

    int Width,Height,Stride,Start_X,Start_Y;

    //------------Test FAT SCAN ZERO----------//
 #if( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    DEBUG_MAIN("----->Test FAT-SCAN-ZERO:");
    for(k=2;k<1000;k+=5)
    {
        ZeroCount=0;
        wptr=(unsigned int *)MCPU_TEST_ADDRESS;
        for(i=0;i<(0x10000/4);i++)
        {
            val= (i+k*101+134) % k;
            if(val == 0)
                ZeroCount ++;
            *wptr= val;
            wptr ++;
        }

        temp=mcpu_FATZeroScan((u8 *)MCPU_TEST_ADDRESS,0x10000);
        if(temp != ZeroCount)
        {
           DEBUG_MCPU("FAIL(k=%d,%d,%d)\n",k,temp,ZeroCount);
           return 0;
        }

        DEBUG_MCPU("OK(k=%d,%d,%d)\n",k,temp,ZeroCount);      
    } 
 #endif
    //--------------Test Block Copy------------//
 #if( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    pp=(u8 *)MCPU_TEST_ADDRESS;
    for(i=0;i<MCPU_TESTSIZE;i++)
    {
        *pp=i & 0x0ff;
        pp ++;
    }
    qq=(u8 *)(MCPU_TEST_ADDRESS+0x10000);
    rr=(u8 *)(MCPU_TEST_ADDRESS+0x20000);
    for(i=0;i<0x10000;i++)
    {
       *qq=0;
       *rr=0;
       qq ++;
       rr ++;
    }
    
    Width=16;
    Height=12;
    Stride=256;
    Start_X=4;
    Start_Y=1;    
    //----//
    for(k=0;k<128;k++)
    {
        pp=(u8 *)MCPU_TEST_ADDRESS;
        qq=(u8 *)MCPU_TEST_ADDRESS+0x10000;
        rr=(u8 *)MCPU_TEST_ADDRESS+0x20000;
        
        Width = 16 + ((k*4) & 127);
        Height = 12 + (k & 63);
        Stride =256 + ((k*4) & 127);
        Start_X  =4 + ((k*4) & 63);
        //Start_Y +=1;

        DEBUG_MAIN("----->Test Block Copy(%d,%d,%d):",Width,Height,Stride);

        mcpu_BlockCpy( qq+Start_X+Start_Y*Stride, pp, 
                       Width, Height,
                       Stride);

        Sw_BlockCpy( rr+Start_X+Start_Y*Stride, pp, 
                     Width, Height,
                     Stride);

        for(i=0;i<0x10000;i++)
        {
           if(*qq != *rr)
           {
              DEBUG_MCPU("FAIL\n");
              return 0;
           }
           qq ++;
           rr ++;
        }
        DEBUG_MCPU("OK\n");
    }
  #endif  
    //------------Test Linear Copy -------------//
    DEBUG_MAIN("----->Test Linear Copy:\n");
    pp=(u8 *)MCPU_TEST_ADDRESS;
    for(i=0;i<MCPU_TESTSIZE;i++)
    {
        *pp=i & 0x0ff;
        pp ++;
    }

    for(testbyte=4;testbyte<(MCPU_TESTSIZE-1024);testbyte++)
    {
        DEBUG_MCPU("=====>Check Size=%d---->",testbyte);
        for(dst_offset=0;dst_offset<16;dst_offset++)
        {
            for(src_offset=0;src_offset<16;src_offset++)
            {
                pp=(u8 *)MCPU_TEST_ADDRESS;
                qq=(u8 *)MCPU_TEST_ADDRESS+0x10000;
                rr=(u8 *)MCPU_TEST_ADDRESS+0x20000;
                //setup environment
                for(i=0;i<((testbyte+128) & 0xfffffffc);i++)
                {
                    qq[i]= (i ^ 0xffffffff) & 0x0ff;

                    rr[i]= (i ^ 0xffffffff) & 0x0ff;
                }

                pp +=src_offset;
                qq +=dst_offset;
                rr +=dst_offset;

                mcpu_ByteMemcpy(qq,pp, testbyte);
                memcpy(rr,pp,testbyte);

                //Compare result
                for(i=0;i<(testbyte+32);i++)
                {
                    if(rr[i] != qq[i])
                    {
                       DEBUG_MCPU("FAIL\n");
                       return 0;
                    }
                }
             }
        }
        DEBUG_MCPU("PASS\n");
    }
    return 1;
    
}

int marsMcpu2_Test()
{
    u8 *pp,*qq,*rr;
    unsigned int *wptr;
    int i,j,k;
    int ZeroCount,val,temp;
    int src_offset,dst_offset,testbyte;

    int Width,Height,Stride,Start_X,Start_Y;

    //------------Test FAT SCAN ZERO----------//
 #if( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    DEBUG_MAIN("----->Test FAT-SCAN-ZERO:");
    for(k=2;k<1000;k+=5)
    {
        ZeroCount=0;
        wptr=(unsigned int *)MCPU_TEST_ADDRESS;
        for(i=0;i<(0x10000/4);i++)
        {
            val= (i+k*101+134) % k;
            if(val == 0)
                ZeroCount ++;
            *wptr= val;
            wptr ++;
        }

        temp=mcpu2_FATZeroScan((u8 *)MCPU_TEST_ADDRESS,0x10000);
        if(temp != ZeroCount)
        {
           DEBUG_MCPU("FAIL(k=%d,%d,%d)\n",k,temp,ZeroCount);
           return 0;
        }

        DEBUG_MCPU("OK(k=%d,%d,%d)\n",k,temp,ZeroCount);      
    } 
 #endif
    //--------------Test Block Copy------------//
 #if( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    pp=(u8 *)MCPU_TEST_ADDRESS;
    for(i=0;i<MCPU_TESTSIZE;i++)
    {
        *pp=i & 0x0ff;
        pp ++;
    }
    qq=(u8 *)(MCPU_TEST_ADDRESS+0x10000);
    rr=(u8 *)(MCPU_TEST_ADDRESS+0x20000);
    for(i=0;i<0x10000;i++)
    {
       *qq=0;
       *rr=0;
       qq ++;
       rr ++;
    }
    
    Width=16;
    Height=12;
    Stride=256;
    Start_X=4;
    Start_Y=1;    
    //----//
    for(k=0;k<128;k++)
    {
        pp=(u8 *)MCPU_TEST_ADDRESS;
        qq=(u8 *)MCPU_TEST_ADDRESS+0x10000;
        rr=(u8 *)MCPU_TEST_ADDRESS+0x20000;
        
        Width = 16 + ((k*4) & 127);
        Height = 12 + (k & 63);
        Stride =256 + ((k*4) & 127);
        Start_X  =4 + ((k*4) & 63);
        //Start_Y +=1;

        DEBUG_MAIN("----->Test Block Copy(%d,%d,%d):",Width,Height,Stride);

        mcpu2_BlockCpy( qq+Start_X+Start_Y*Stride, pp, 
                       Width, Height,
                       Stride);

        Sw_BlockCpy( rr+Start_X+Start_Y*Stride, pp, 
                     Width, Height,
                     Stride);

        for(i=0;i<0x10000;i++)
        {
           if(*qq != *rr)
           {
              DEBUG_MCPU("FAIL\n");
              return 0;
           }
           qq ++;
           rr ++;
        }
        DEBUG_MCPU("OK\n");
    }
  #endif  
    //------------Test Linear Copy -------------//
    DEBUG_MAIN("----->Test Linear Copy:\n");
    pp=(u8 *)MCPU_TEST_ADDRESS;
    for(i=0;i<MCPU_TESTSIZE;i++)
    {
        *pp=i & 0x0ff;
        pp ++;
    }

    for(testbyte=4;testbyte<(MCPU_TESTSIZE-1024);testbyte++)
    {
        DEBUG_MCPU("=====>Check Size=%d---->",testbyte);
        for(dst_offset=0;dst_offset<16;dst_offset++)
        {
            for(src_offset=0;src_offset<16;src_offset++)
            {
                pp=(u8 *)MCPU_TEST_ADDRESS;
                qq=(u8 *)MCPU_TEST_ADDRESS+0x10000;
                rr=(u8 *)MCPU_TEST_ADDRESS+0x20000;
                //setup environment
                for(i=0;i<((testbyte+128) & 0xfffffffc);i++)
                {
                    qq[i]= (i ^ 0xffffffff) & 0x0ff;

                    rr[i]= (i ^ 0xffffffff) & 0x0ff;
                }

                pp +=src_offset;
                qq +=dst_offset;
                rr +=dst_offset;

                mcpu2_ByteMemcpy(qq,pp, testbyte);
                memcpy(rr,pp,testbyte);

                //Compare result
                for(i=0;i<(testbyte+32);i++)
                {
                    if(rr[i] != qq[i])
                    {
                       DEBUG_MCPU("FAIL\n");
                       return 0;
                    }
                }
             }
        }
        DEBUG_MCPU("PASS\n");
    }
    return 1;
    
}


#endif


