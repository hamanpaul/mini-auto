/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sdcapi.h

Abstract:

    The application interface of the SecurityDisk controller

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#ifndef __SDC_API_H__
#define __SDC_API_H__

#include "../sdc/inc/sdcerr.h"

#define VERIFYSD       0

#if(CHIP_OPTION == CHIP_PA9001D)
    #define SW_MUL_WRITE   1
#else
   #define SW_MUL_WRITE   0
#endif

#define REAL_CHIP      1 /* (VCC) 1: For Real-Chip  0: For FPGA board */
#define BURN_IN_TEST   0 /* (VCC) 1:FOR Burn-In Test  0: FOR Normal Test */
#define SD_SPEC_2DOT0  1 /* (VCC) 1:FOR Spec 2.0  0: FOR Spec 1.1 */

#define USB_MUL_RW     1 /* (VCC) 1:Multiple Read/Write 0: Single Read/Write */

#if(CHIP_OPTION == CHIP_PA9001D)
     #define SD_MUL_READ_BURST 0 /* (VCC) 1: Read all blocks of data in one transaction 0: Read in multiple transcation */
#else
     #define SD_MUL_READ_BURST 1 /* (VCC) 1: Read all blocks of data in one transaction 0: Read in multiple transcation */
#endif

#if(CHIP_OPTION == CHIP_PA9001A || CHIP_OPTION == CHIP_PA9001C || CHIP_OPTION == CHIP_PA9002A)
   #define USE_TX_INT     0 /* (VCC) 1:Use Tx_INT interrupt  0:Use polling. 
                            Tx_INT interrupt was added in PA9001D */
#else
#define USE_TX_INT     1 /* (VCC) 1:Use Tx_INT interrupt  0:Use polling.    // Set 0: Transcend 2GB will be okay -- VC_Chen
                        Tx_INT interrupt was added in PA9001D */
#endif

// #define USE_MUL_TX_INT 1 /* (VCC) 1:Use Multiple Tx_INT interrupt*/
/* Constant */
#define SDC_BLK_LEN 0x0200      /* SD block length */
#define MMC_BLK_LEN 0x0200      /* MMC block length */

/* Function prototype */
extern s32 sdcInit(void);
extern s32 sdcUnInit(void);
extern s32 sdcMount(void);
extern s32 sdcUnmount(void);
extern void sdcIntHandler(void);
extern void sdcTest(void);

extern int sdcDevStatus(u32); 
extern int sdcDevRead(u32, u32, void*); 
extern int sdcDevMulRead(u32, u32, u32, void*);
extern int sdcDevWrite(u32, u32, void*); 
extern int sdcDevMulWrite(u32, u32, u32, void *); 
extern int sdcDevIoCtl(u32, s32, s32, void*);
extern s8 sdcChangeMediaStat(u8);
extern char Write_protet(void);

extern s32 sdcSetStat(u8 stat);

extern u32 errHandle(u32 ucErrorCode, u8 *ucOutputMessage);
extern int sdcErrorResultFilter(int funcValue);

extern u32 sdcTryInvertSDClk;



//#define SD_MUL_READ_BURST 1 /* (VCC) 1: Read all blocks of data in one transaction 0: Read in multiple transcation */

/* define for USB process */
#define SDC_USB_MEDIA_REMOVED	0
#define SDC_USB_MEDIA_CHANGED	1
#define SDC_USB_MEDIA_READY		2

/* Flag for SD and USB process */
#define FLAGSD_SET_SD_STAT			0x01
#define FLAGSD_CHECK_SD				0x02
#define FLAGUSB_SET_PLUGIN_STAT		0x03

/* time out constant for OS */
#define TIMEOUT_SDC			80

#define SDC_CARD_RISE_DETEC_ENA                0x00000200
#define SDC_CARD_FALL_DETEC_ENA                0x00000400

#endif
