/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    adc.c

Abstract:

    The routines of ADC.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "adcreg.h"
#include "sysapi.h"
#include "smcapi.h"
#include "uiapi.h"
#include "siuapi.h"
#include "gpioapi.h"
#include "iisapi.h"
#include "asfapi.h"
#include "uiKey.h"
#include "adcapi.h"


#if (HW_BOARD_OPTION == MR9670_WOAN)
    #include "..\..\ui\inc\ui_woan_project.h"
#endif
#if (UI_VERSION== UI_VERSION_TRANWO)
    #include "..\..\ui\inc\uiact_project.h"
#endif

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


#if (HW_BOARD_OPTION == MR8120_RX_HECHI)
#define	STAT_NONE_PRESS_FORMAT		6
#define	STAT_SINGLE_PRESS_FORMAT	7
#define	STAT_CONT_PRESS_FORMAT		8
#endif
void adcSetDAC_OutputGain(u32 unAudioVol);

 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
u32 ADCnumber[30];
u16 ADCcnt=0;
#if(HW_BOARD_OPTION == MR8120_RX_HECHI)
u8 timer_cnt=0;
#endif
u32 gAudioVol=31;
u8  adcBatErrRange = 0x10;

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) )
    #if(ADC_IIS_CLK_FREQ == 24000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24};
    u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_4     ,ADC_CLK_DIV_B_2     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0,
                             ADC_CLK_DIV_B_4     ,ADC_CLK_DIV_B_2     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0,
                             ADC_CLK_DIV_B_4     ,ADC_CLK_DIV_B_2     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0,
                             ADC_CLK_DIV_B_4     ,ADC_CLK_DIV_B_2     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_0};
    u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_29};
    #elif(ADC_IIS_CLK_FREQ == 32000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_33    ,ADC_CLK_DIV_A_24     ,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_33    ,ADC_CLK_DIV_A_24     ,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_33    ,ADC_CLK_DIV_A_24     ,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_33    ,ADC_CLK_DIV_A_24    };
    u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_3     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,
                             ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_3     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,
                             ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_3     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,
                             ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_3     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_1     };
    u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_21,ADC_REC_CONV_RATE_19,
                             ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_21,ADC_REC_CONV_RATE_19,
                             ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_21,ADC_REC_CONV_RATE_19,
                             ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_21,ADC_REC_CONV_RATE_19};
    #elif(ADC_IIS_CLK_FREQ == 48000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                        //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                             ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24};
    u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                             ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                             ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                             ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1};
    u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                             ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29};
    #endif
#elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A) ||\
(CHIP_OPTION == CHIP_A1026A))
    #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    	#if(ADC_IIS_CLK_FREQ == 24000000)
        #elif(ADC_IIS_CLK_FREQ == 32000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 4CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79};
        #elif(ADC_IIS_CLK_FREQ == 48000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24};
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1};
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29};
        #else
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24};
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1};
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29};
        #endif
    #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
        #if(ADC_IIS_CLK_FREQ == 24000000)
        #elif(ADC_IIS_CLK_FREQ == 32000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 3CH ***
        *** ADC_Conv_RATE > 15
        *** ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) MAX 768k, so only support 8khz~16khz
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19};
        #elif(ADC_IIS_CLK_FREQ == 48000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 3CH ***
        *** ADC_Conv_RATE > 15
        *** ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) MAX 768k, so only support 8khz~16khz
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59};
        #elif(ADC_IIS_CLK_FREQ == 54000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 3CH ***
        *** ADC_Conv_RATE > 15
        *** ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) MAX 768k, so only support 8khz~16khz
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_227    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_227    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_227    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_227    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_69    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_69    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_69    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_69    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59};

        #endif
	#endif
#elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    	#if(ADC_IIS_CLK_FREQ == 24000000)

        #elif(ADC_IIS_CLK_FREQ == 32000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 4CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     ,
                                 ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_24    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_1     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_79};
        #elif(ADC_IIS_CLK_FREQ == 48000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24};
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1};
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29};
        #else
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 16CH ***
        *** ADC_Conv_RATE > 15
        ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24,
                                 ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_65     ,ADC_CLK_DIV_A_24};
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1,
                                 ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_5     ,ADC_CLK_DIV_B_1     ,ADC_CLK_DIV_B_0     ,ADC_CLK_DIV_B_1};
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29,
                                 ADC_REC_CONV_RATE_29,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_44,ADC_REC_CONV_RATE_66,ADC_REC_CONV_RATE_29};
        #endif
    #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
        #if(ADC_IIS_CLK_FREQ == 24000000)

        #elif(ADC_IIS_CLK_FREQ == 32000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 3CH ***
        *** ADC_Conv_RATE > 15
        *** ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) MAX 768k, so only support 8khz~16khz
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24     ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128    ,ADC_CLK_DIV_A_24   };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_39    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_24,ADC_REC_CONV_RATE_17,ADC_REC_CONV_RATE_19};
        #elif(ADC_IIS_CLK_FREQ == 48000000)
        /******************************************************************************************************************
        *** Sample rate = ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) / (ADC_Conv_RATE+1) / 3CH ***
        *** ADC_Conv_RATE > 15
        *** ADC_IIS_CLK_FREQ * (ADC_DIV_A/(ADC_DIV_A+1)) / (ADC_DIV_B+1) MAX 768k, so only support 8khz~16khz
        ******************************************************************************************************************/
                                //8k                 ,16k                 ,32k                 ,44.1K               ,48K
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_24    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_59    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_31,ADC_REC_CONV_RATE_15,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59};
        #endif
	#endif
#endif
    u32 ADC_RX_FMT[20] = {ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,
                          ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit};

    #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) )
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,
                               ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED};
    #elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,
                               ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED};
    #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,
                               ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS};
    #endif
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,
                               ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED};
    #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,
                               ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS};
    #endif

    #endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8   zoomflag;
extern u8   Lwipredhcp;
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
extern u16 Humidity;
extern u16 Light;
extern f32 TEMP;
extern s32 TempC;
#endif

#if ((HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||\
     (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) ||\
     (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)||\
     (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) ||\
     (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
extern u8 uiCurAlertStatus;
extern u8 uiFakeShutdown;
#endif
#if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
extern u8 uiFakeShutdown;
#endif
#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532)|| (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
     (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
extern u8 timerDisableMic;
#endif

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern void osdDrawShowZoom(u8 value);

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

 /*

Routine Description:

    Initialize the ADC.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 adcInit(u8 adcrecena)
{

#if( (CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B))
    #if(HW_BOARD_OPTION == A1013_FPGA_BOARD)
    if(adcrecena) //set adc rec freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000;

        //set up sample rate
        AdcCtrlReg = ADC_PWOFF;
        AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
        ADCRX_AVG_FIFO_INT = ADC_AVG_G1_ENA;
        //set up Group map, and rec Group/ch
        AdcRXRecChMap_G0 = (ADC_REC_CH_0 << 24) | (ADC_REC_CH_6  << 16) | (ADC_REC_CH_5 << 8)  | (ADC_REC_CH_4);
        AdcRXRecChMap_G1 = (ADC_REC_CH_1 << 24) | (ADC_REC_CH_9  << 16) | (ADC_REC_CH_8 << 8)  | (ADC_REC_CH_7);
        AdcRXRecChMap_G2 = (ADC_REC_CH_2 << 24) | (ADC_REC_CH_12 << 16) | (ADC_REC_CH_11 << 8) | (ADC_REC_CH_10);
        AdcRXRecChMap_G3 = (ADC_REC_CH_3 << 24) | (ADC_REC_CH_15 << 16) | (ADC_REC_CH_14 << 8) | (ADC_REC_CH_13);
        //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
        AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
        AdcCtrlReg |= ADC_PWON;
        // caution : start ADC recording sequence !!
        AdcCtrlReg |= ADC_REC_CONV_ENA;
        AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH3);
    }
    else  //set adc key freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000700;// ADC clock source, 192M/8=24MHz.
        AdcCtrlReg  =  ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_0DB | ADC_PGINL_0DB | ADC_REC_CH0 | 0x004a0000;
    }
    #elif(HW_BOARD_OPTION == A1013_REALCHIP_A)
    if(adcrecena) //set adc rec freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000;// ADC clock source, 48M/1=48MHz.

        //set up sample rate
        AdcCtrlReg = ADC_PWOFF;
        AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
        //ADCRX_AVG_FIFO_INT = ADC_AVG_G1_ENA;
        //set up Group map, and rec Group/ch
        AdcRXRecChMap_G0 = (ADC_REC_CH_4 << 24) | (ADC_REC_CH_7  << 16) | (ADC_REC_CH_6 << 8)  | (ADC_REC_CH_4);
        AdcRXRecChMap_G1 = (ADC_REC_CH_6 << 24) | (ADC_REC_CH_9  << 16) | (ADC_REC_CH_8 << 8)  | (ADC_REC_CH_1);
        AdcRXRecChMap_G2 = (ADC_REC_CH_2 << 24) | (ADC_REC_CH_12 << 16) | (ADC_REC_CH_11 << 8) | (ADC_REC_CH_10);
        AdcRXRecChMap_G3 = (ADC_REC_CH_3 << 24) | (ADC_REC_CH_15 << 16) | (ADC_REC_CH_14 << 8) | (ADC_REC_CH_13);
        //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
        AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
        AdcCtrlReg |= ADC_PWON;
        // caution : start ADC recording sequence !!
        AdcCtrlReg |= ADC_REC_CONV_ENA;
        AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH3);
    }
    else  //set adc key freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000;// ADC clock source, 48M/1=48MHz.
        AdcCtrlReg  =  ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_0DB | ADC_PGINL_0DB | ADC_REC_CH0 | 0x004a0000;
    }
    #endif
#elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A) ||\
(CHIP_OPTION == CHIP_A1026A))
 #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
	if(adcrecena) //set adc rec freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000;

        //set up sample rate
        AdcCtrlReg = ADC_PWOFF;
        AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
        //ADCRX_AVG_FIFO_INT = ADC_AVG_G0_ENA;
        //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
        AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
        AdcCtrlReg |= ADC_PWON;

        // caution : start ADC recording sequence !!
        AdcCtrlReg |= ADC_REC_CONV_ENA;
        AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH1);
        AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2); //ch1
    }
    else  //set adc key freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000700;// ADC clock source, 192M/8=24MHz.
        AdcCtrlReg  =  ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_0DB | ADC_PGINL_0DB | ADC_REC_CH0 | 0x004a0000;
    }
 #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
    /*** Scan seq : 0-2-3,1-2-3,0-2-3,1-2-3, ***/
    SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
    //SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000; //RealChip move to bootcode

    //set up sample rate
  #if IS_COMMAX_DOORPHONE // 無外部Amp的用這個, Mircro-phone in
    DacTxCtrl = ((ADC_PGA_GAIN & 0x1f) << 8) | DAC_ENBST_ON | DAC_ENMIC_ON | DAC_ENMICBIAS_ON | DAC_VREFSM_STRONG| DAC_ENVREF_ON;// | DAC_PWON;
  #else
    #if( (SW_APPLICATION_OPTION == MR9670_DOORPHONE) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)  )// 有外部Amp的用這個,Line-in
        DacTxCtrl = ((0x1f & 0x1f)<<8) | DAC_ENBST_OFF | DAC_ENMIC_OFF | DAC_ENMICBIAS_OFF | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_PWON;
    #elif( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        DacTxCtrl = ((ADC_PGA_GAIN & 0x1f) << 8) | DAC_ENMIC_ON | DAC_ENMICBIAS_ON | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_PWON | DAC_VREFSM_WEAK;
    #elif( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) ) 
        DacTxCtrl = ((ADC_PGA_GAIN & 0x1f) << 8) | DAC_ENBST_OFF | DAC_ENMIC_ON | DAC_ENMICBIAS_ON | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_VREFSM_WEAK | DAC_MUTE_ENA;
    #else   // 無外部Amp的用這個, Mircro-phone in
        DacTxCtrl = ((ADC_PGA_GAIN & 0x1f) << 8) | DAC_ENBST_ON | DAC_ENMIC_ON | DAC_ENMICBIAS_ON | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_PWON;
    #endif
  #endif

    AdcCtrlReg = ADC_PWOFF;
    #if(HW_BOARD_OPTION == MR8120_TX_RDI_AV)  //RDI AV sender 聲音 1V輸入 不到1V輸出 因此做數位放大
    AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06100000;
    #else
    AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
    #endif
    //ADCRX_AVG_FIFO_INT = ADC_AVG_G0_ENA;
    //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
    AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
    AdcCtrlReg |= ADC_PWON;

    // caution : start ADC recording sequence !!
    AdcCtrlReg |= ADC_REC_CONV_ENA;
   #if (HW_BOARD_OPTION == MR8211_ZINWELL)
    AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH3);    //MIC(Line)
    AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH3);    //MIC(Line)
   #elif (HW_BOARD_OPTION == MR9670_WOAN)
	AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH3);    //ADCIN 2
	AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH3);    //MIC(Line)
   #elif(HW_BOARD_OPTION == MR6730_AFN)
   //CH1--->G1 , CH2--->G0 

	#if 0
		//exchange channel
		AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH3);    //MIC(Line)	for CH2
		AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2);    //ADCIN 2	for CH1

	#else

		AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH2);    //ADCIN 2	for CH2
		AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH3);    //MIC(Line)	for CH1

	#endif

		DEBUG_ADC("== adcInit(%d) ==",adcrecena);//debug only 
		if(adcrecena)
		{
		//after init ...
		}
		else
		{//init	
			if(Main_Init_Ready==0)
			{
				AdcCtrlReg &= (~ADC_REC_G1);
				AdcCtrlReg &= (~ADC_REC_G0);
			}
		}


   #else
    AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH3);    //MIC(Line)
    AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2);    //ADCIN 2
   #endif
 #endif

    #if AUDIO_IN_OUT_SELFTEST
    adcSetDAC_OutputGain(0);
    #else
    adcSetDAC_OutputGain(gAudioVol);
    #endif
#elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    	if(adcrecena) //set adc rec freq
        {
            SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
            SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000;

            //set up sample rate
            AdcCtrlReg = ADC_PWOFF;
            AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
            //ADCRX_AVG_FIFO_INT = ADC_AVG_G0_ENA;
            //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
            AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
            AdcCtrlReg |= ADC_PWON;

            // caution : start ADC recording sequence !!
            AdcCtrlReg |= ADC_REC_CONV_ENA;
            AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH1);
            AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2); //ch1
        }
        else  //set adc key freq
        {
            SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
            SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000700;// ADC clock source, 192M/8=24MHz.
            AdcCtrlReg  =  ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_0DB | ADC_PGINL_0DB | ADC_REC_CH0 | 0x004a0000;
        }
        #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
        /*** Scan seq : 0-2-3,1-2-3,0-2-3,1-2-3, ***/
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        //SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000000; //RealChip move to bootcode

        //set up sample rate
        //DacTxCtrl = ((ADC_PGA_GAIN & 0x1f)<<8) | DAC_ENBST_ON | DAC_ENMIC_ON | DAC_ENMICBIAS_OFF | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_PWON;
        DacTxCtrl = ((ADC_PGA_GAIN & 0x1f) << 8) | DAC_ENBST_ON | DAC_ENMIC_ON | DAC_ENMICBIAS_ON | DAC_VREFSM_STRONG| DAC_ENVREF_ON | DAC_PWON | 0x00000020;
        AdcCtrlReg = ADC_PWOFF;
        AdcRecConvRate = ADC_Conv_RATE[Audio_formate] | 0x06000000;
        //ADCRX_AVG_FIFO_INT = ADC_AVG_G0_ENA;
        //AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate]| (ADC_REC_G1 | ADC_REC_G1_CH3);
        AdcCtrlReg =  ADC_RX_FMT[Audio_formate]|ADC_RX_FMT_SIGN[Audio_formate]|ADC_DIV_A[Audio_formate] | ADC_DIV_B[Audio_formate];
        AdcCtrlReg |= ADC_PWON;

        // caution : start ADC recording sequence !!
        AdcCtrlReg |= ADC_REC_CONV_ENA;
        AdcCtrlReg |= (ADC_REC_G0 | ADC_REC_G0_CH3);   //MIC(Line)
        //AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2); //ADCIN 2
    	#endif
    #if AUDIO_IN_OUT_SELFTEST
    adcSetDAC_OutputGain(0);
    #else
    adcSetDAC_OutputGain(gAudioVol);
    #endif
#else
    if(adcrecena) //set adc rec freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000300;// ADC clock source= 192M/4=48M
        AdcCtrlReg = ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_20DB | ADC_PGINL_6DB | ADC_REC_CH1 | 0x00950000; //(ADC sample rate = 48M/150/10/4=8000Hz)
    }
    else  //set adc key freq
    {
        SYS_CTL0 |= SYS_CTL0_ADC_CKEN;
        SYS_CLK2 = ( SYS_CLK2 & (~0x0000ff00) ) | 0x00000300;// ADC clock source, 192M/4=48MHz.
        AdcCtrlReg  =  ADC_PWON | ADC_RX_ENA | ADC_REF_TC_0PERCENT | ADC_PWD_PGA_ENA | ADC_PGINM_0DB | ADC_PGINL_0DB | ADC_REC_CH0 | 0x00950000;
    }
#endif

    return 1;
}

/*
Routine Description:

    Use ADC to measure bettery capacity in ADC channel 1

Arguments:

    None.

Return Value:
     u32 value= 255* v/RVDD
*/

u32 adcBatteryCheck(void)
{
    u32 adc_0 ,adc_1,adc_2;
    u8  cur_status=0, cur_status_dc;
    static u8 rnu_one=0,check_cnt = 0,check_flag = 0;
    static u16 shutdown_cnt = 0;
    RTC_DATE_TIME   localTime;
    static u32 adc_value;
    static u8 lastLV = UI_BATTERY_CLEAR, cnt=0, dc_detect=0xff, cnt1=0, charge_status = 0;
    static u8 count_osd=0;
    static u32 adc_level[8]={0,0,0,0,0,0,0,0};
    u8 level=0xff;
    u8 detect_cnt=0, Bat_Check_Ok_Flag = 0;
    static u32 tmpAdcVal[5];
    u8 i=0,j=0,bcharge=0, gpiolevel = 0;

    if(Main_Init_Ready  == 0 )
        return 0;

#if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) ||\
     (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS))
    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16; /*Battery Value*/
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);     /*Adapter Plug in-out*/
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16; /*Check Battery Charge Finish*/
    //printf("AdcRecData_G1G2 %08x, adc0:%03x adc1:%03x adc2:%03x\n", AdcRecData_G1G2, adc_0, adc_1, adc_2);
    
    cur_status=UI_BATTERY_NONE;
    //DEBUG_ADC(" ***** gpioLevel %d\n",gpioLevel);
    if((adc_0 > 0x7ff) && (adc_0 < 0xd00)) /* No Battery */
    {
        cur_status=UI_BATTERY_CLEAR;
        
        /* if battery broken , and now is in Fake Shutdown 
           It should only detect adapter plug in */
        if((((adc_1 > 0x000)&&(adc_1 <0x718)) || ((adc_1 > 0xd27)&&( adc_1 <0xfff))) && (uiFakeShutdown == 1))
        {
            cur_status=UI_BATTERY_CHARGE;
        }       
    }
    /* Battery +Adapter */
    /* adc_2: Low(No charge): 0x800, High(charge): 0x7ff */
    /* 0x2e0 < adc_1 < 0x400 (no docking) */
    /* 0x50  < adc_1 < 0x110 (with docking) */
    else if((((adc_1 > 0x000)&&(adc_1 <0x718)) || ((adc_1 > 0xd27)&&( adc_1 <0xfff))) && ((adc_2>0x600)&&(adc_2  < 0x800 )))
    {
        cur_status=UI_BATTERY_CHARGE;
    }
    else  /* Only Battery */
    {        
        if(cnt == 4)
        {
            adc_value =adc_value>>2;
            //DEBUG_ADC("ADC value %x \n",adc_value);
            if( adc_value >0x1f0)
            {
                cur_status=UI_BATTERY_LV4;
            }
            else if(adc_value >0x190)
            {
                cur_status=UI_BATTERY_LV3;
            }
            else if(adc_value >0x168)
            {
                cur_status=UI_BATTERY_LV2;
            }
            else if(adc_value >0xc0)
            {
                cur_status=UI_BATTERY_LV1;    
            }
            else
            {
                cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
            }    
            cnt=0;
            adc_value=0;
        } 
        else if(cnt <4)
        {
            cnt++;
            adc_value += adc_0;  
        }

    }

    
    //printf("%d curStatus %d lastLV:%d %x\n",cnt,cur_status,lastLV ,adc_value );
    if((batteryflag != cur_status) && (cur_status!= UI_BATTERY_NONE))
    {
        if (((lastLV >= UI_BATTERY_LV0)&&(lastLV < UI_BATTERY_LV5)) && (cur_status > lastLV)&& ((cur_status >=UI_BATTERY_LV0 )&&(cur_status < UI_BATTERY_LV5)))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }   
        
        lastLV=cur_status;
        if(cur_status == UI_BATTERY_CLEAR)   /* No Battery */
        {
            DEBUG_ADC("****** No Battery ******\n");
            batteryflag = UI_BATTERY_CLEAR;
            gpioTimerCtrLed(LED_RED_OFF);
            gpioTimerCtrLed(LED_GREEN_ON);
        }
        else if(cur_status == UI_BATTERY_CHARGE)  /* Battery +Adapter */
        {
            DEBUG_ADC("****** Charge ******\n");
            batteryflag =UI_BATTERY_CHARGE;
            gpioTimerCtrLed(LED_RED_ON);
            gpioTimerCtrLed(LED_GREEN_ON);
            uiCurAlertStatus &= ~UI_ALERT_STATUS_BATTERY_LOW;
            if(uiFakeShutdown ==1)
            {
                uiFakeShutdown=0;
                sysForceWDTtoReboot();
            }
            if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) ==UI_ALERT_TYPE_BATLOW)
            {
                uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW_NONE,UI_BEEP_CHANGE);
            }
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
        {
            //DEBUG_ADC("******* Fake Shutdown *******");
            if(uiFakeShutdown ==0)
            {
                DEBUG_ADC("******* Fake Shutdown *******");
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
				sysForceWDTtoReboot();
            }
            batteryflag=UI_BATTERY_LV0;
        }
        else                                     /* Only Battery */
        {
            DEBUG_ADC("****** Only Battery ******\n");
            batteryflag = cur_status;
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            if(cur_status != UI_BATTERY_LV1)
            {
                gpioTimerCtrLed(LED_RED_OFF);
                gpioTimerCtrLed(LED_GREEN_ON);
            }
            else
            {
                uiCurAlertStatus |= UI_ALERT_STATUS_BATTERY_LOW;
                if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) != UI_ALERT_TYPE_BATLOW)
                    uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_CHANGE);
                gpioTimerCtrLed(LED_RED_FLASH);
                gpioTimerCtrLed(LED_GREEN_OFF);
            }
        }
    }

    if(batteryflag == UI_BATTERY_LV1)
    {
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);    
    }
    else
    {
        count_osd++;
        if((count_osd %10) == 0)
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);     
            count_osd=0;
        }
    }
#elif((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS))
    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16; /*Battery Value*/
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);     /*Adapter Plug in-out*/
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16; /*Check Battery Charge Finish*/
    //printf("AdcRecData_G1G2 %08x, adc0:%03x adc1:%03x adc2:%03x\n", AdcRecData_G1G2, adc_0, adc_1, adc_2);
    
    cur_status=UI_BATTERY_NONE;
    //DEBUG_ADC(" ***** gpioLevel %d\n",gpioLevel);
    if((adc_0 > 0x7ff) && (adc_0 < 0xd00)) /* No Battery */
    {
        cur_status=UI_BATTERY_CLEAR;
    }
    /* Battery +Adapter */
    /* adc_2: Low(No charge): 0x800, High(charge): 0x7ff */
    /* 0x2e0 < adc_1 < 0x400 (no docking) */
    /* 0x50  < adc_1 < 0x110 (with docking) */
    else if((((adc_1 > 0x000)&&(adc_1 <0x718)) || ((adc_1 > 0xd27)&&( adc_1 <0xfff))) && ((adc_2>0x600)&&(adc_2  < 0x800 )))
    {
            if (adc_0 < 0xc0)
            {
                cur_status = UI_BATTERY_LV0;
                DEBUG_ADC("fake charge , battery low %d %d\n", __LINE__, uiFakeShutdown);
            }
            else
            {
                cur_status=UI_BATTERY_CHARGE;
                DEBUG_ADC("Charge %d %d\n", __LINE__, uiFakeShutdown);
                if (uiFakeShutdown == 1)
                {
                    sysWDT_enable();
                    sysForceWDTtoReboot();
                    uiFakeShutdown=0;
                }

            }
    }
    else  /* Only Battery */
    {        
        if(cnt == 4)
        {
            adc_value =adc_value>>2;
            //DEBUG_ADC("ADC value %x \n",adc_value);
            if( adc_value >0x1f0)
            {
                cur_status=UI_BATTERY_LV4;
            }
            else if(adc_value >0x190)
            {
                cur_status=UI_BATTERY_LV3;
            }
            else if(adc_value >0x168)
            {
                cur_status=UI_BATTERY_LV2;
            }
            else if(adc_value >0xc0)
            {
                cur_status=UI_BATTERY_LV1;    
            }
            else
            {
                cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
            }    
            cnt=0;
            adc_value=0;
        } 
        else if(cnt <4)
        {
            cnt++;
            adc_value += adc_0;  
        }

    }
    if((Main_Init_Ready == 0 )&&(uiFakeShutdown ==0))
    {

        DEBUG_ADC("BOOT LOW POWER==>adc0:%03x  %d \n", adc_0, uiFakeShutdown);
        if(adc_0 < 0x0c0)
        {
            if(uiFakeShutdown == 0)
            {
                sysDeadLockMonitor_OFF();
                sysWDT_disable();
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
            }
        }
    }
    
    //printf("%d curStatus %d lastLV:%d %x\n",cnt,cur_status,lastLV ,adc_value );
    if((batteryflag != cur_status) && (cur_status!= UI_BATTERY_NONE))
    {
        if (((lastLV >= UI_BATTERY_LV0)&&(lastLV < UI_BATTERY_LV5)) && (cur_status > lastLV)&& ((cur_status >=UI_BATTERY_LV0 )&&(cur_status < UI_BATTERY_LV5)))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }   
        
        lastLV=cur_status;
        if(cur_status == UI_BATTERY_CLEAR)   /* No Battery */
        {
            DEBUG_ADC("****** No Battery ******\n");
            batteryflag = UI_BATTERY_CLEAR;
            gpioTimerCtrLed(LED_RED_OFF);
            gpioTimerCtrLed(LED_GREEN_ON);
        }
        else if(cur_status == UI_BATTERY_CHARGE)  /* Battery +Adapter */
        {
            DEBUG_ADC("****** Charge ******\n");
            batteryflag =UI_BATTERY_CHARGE;
            gpioTimerCtrLed(LED_RED_ON);
            gpioTimerCtrLed(LED_GREEN_ON);
            uiCurAlertStatus &= ~UI_ALERT_STATUS_BATTERY_LOW;
            if(uiFakeShutdown ==1)
            {
                sysWDT_enable();
                sysForceWDTtoReboot();
                uiFakeShutdown=0;
            }
            if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) ==UI_ALERT_TYPE_BATLOW)
            {
                uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW_NONE,UI_BEEP_CHANGE);
            }
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
        {
            //DEBUG_ADC("******* Fake Shutdown *******");
            if(uiFakeShutdown ==0)
            {
                DEBUG_ADC("******* Fake Shutdown *******");
                sysDeadLockMonitor_OFF();
                sysWDT_disable();
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
            }
            batteryflag=UI_BATTERY_LV0;
        }
        else                                     /* Only Battery */
        {
            DEBUG_ADC("****** Only Battery ******\n");
            batteryflag = cur_status;
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            if(cur_status != UI_BATTERY_LV1)
            {
                gpioTimerCtrLed(LED_RED_OFF);
                gpioTimerCtrLed(LED_GREEN_ON);
            }
            else
            {
                uiCurAlertStatus |= UI_ALERT_STATUS_BATTERY_LOW;
                if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) != UI_ALERT_TYPE_BATLOW)
                    uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_CHANGE);
                gpioTimerCtrLed(LED_RED_FLASH);
                gpioTimerCtrLed(LED_GREEN_OFF);
            }
        }
    }

    if(batteryflag == UI_BATTERY_LV1)
    {
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);    
    }
    else
    {
        count_osd++;
        if((count_osd % 4) == 0)
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);     
            count_osd=0;
        }
    }
#elif ((HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))

    adc_0 = adcGetValue(0); //(AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = adcGetValue(1); //(AdcRecData_G1G2  &  0x0000fff);
    adc_2 = adcGetValue(2); //(AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf(" adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2);
    //====================================================
    cur_status = UI_BATTERY_NONE;
    
    if(adc_1 > 0xC00 )//tranwo Adaptor out 0xFxx, Adaptor in 0x7xx,0x8xx,0x9xx
    {
        if (batteryflag == UI_BATTERY_CHARGE || batteryflag == UI_BATTERY_UNCHARGE)
        {
            cnt = 0;
            adc_value = 0;
            cur_status = UI_BATTERY_CLEAR;
            batteryflag = cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        if(cnt == 4)
        {
            adc_value = adc_value >> 2;

            DEBUG_ADC("ADC value %x \n", adc_value);
            if( adc_value > 0x931)
            {
                cur_status = UI_BATTERY_LV4; //
            }
            else if(adc_value > 0x8B0)
            {
                cur_status = UI_BATTERY_LV3; //
            }
            else if(adc_value > 0x85E)
            {
                cur_status = UI_BATTERY_LV2; //
            }
            else if(adc_value >= 0x838) //empty
            {
                cur_status = UI_BATTERY_LV1;
            }
            else
            {
                cur_status = UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
            }
            cnt = 0;
            adc_value = 0;

            if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
            {
                batteryflag = cur_status;
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            }
        }
        else if(cnt < 4)
        {
            cnt++;
            adc_value += adc_0;
        }
        if((Main_Init_Ready == 0 )&&(uiFakeShutdown ==0))
        {

            printf("BOOT LOW POWER==>adc0:%03x  %d \n", adc_0, uiFakeShutdown);
            if(adc_0 < 0x838)
            {
                uiFakeShutdown = 2;
            }
        }

        if(uiFakeShutdown == 0)
        {
            if(batteryflag == UI_BATTERY_LV0)
            {
                DEBUG_ADC("******* Fake Shutdown *******\n");
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
            }
            else
            {
                if(batteryflag == UI_BATTERY_LV1)
                {
                    uiCurAlertStatus |= UI_ALERT_STATUS_BATTERY_LOW;
                    if (uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW, UI_BEEP_GET_TYPE) != UI_ALERT_TYPE_BATLOW)
                        uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW, UI_BEEP_CHANGE);
                    gpioTimerCtrLed(LED_GREEN_OFF);
                    gpioTimerCtrLed(LED_RED_FLASH);
                    count_osd++;
                    if((count_osd % 2) == 0)
                    {
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                        count_osd = 0;
                    }
                }
                else
                {
                    gpioTimerCtrLed(LED_GREEN_ON);
                    gpioTimerCtrLed(LED_RED_OFF);
                    //DEBUG_ADC("******* DRAW_BATTERY ,Adaptor out*******\n");
                }
            }
        }
        else if(uiFakeShutdown == 2)
        {
            sysDeadLockMonitor_OFF();
            sysWDT_disable();
            DEBUG_ADC("******* Shutdown *******\n");
            //sysFakeShutodown(); not function
            DEBUG_ADC("******* Shutdown End *******\n");
            uiFakeShutdown = 1;
            gpioTimerCtrLed(LED_GREEN_OFF);
            gpioTimerCtrLed(LED_RED_OFF);
        }
        else if(uiFakeShutdown == 1)
        {
            gpioTimerCtrLed(LED_GREEN_OFF);
            gpioTimerCtrLed(LED_RED_FLASH);

        }
        if (charge_status == 1)
        {
            charge_status = 0;
            gpioGetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, &gpiolevel);
            if (gpiolevel == 0)
                gpioSetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, 1);
        }
    }
    else //Adaptor in //charger oN
    {
        if (batteryflag < UI_BATTERY_LV5) //floating once when battery mode not stable
        {
            cnt = 0;
            adc_value = 0;
            cur_status = UI_BATTERY_CLEAR;
            batteryflag = cur_status;
            return;
        }

        if(cnt < 2)
        {
            cnt++;
            adc_value += adc_0;
            return;
        }
        else
        {
            adc_value = adc_value >> 1;
            DEBUG_ADC("Adapter ADC %03x ADC2 %03x \n", adc_value, adc_2);

            if(uiFakeShutdown == 1)
            {
                if(1) //reboot when adaptor in
                {
                    DEBUG_ADC("******* Reboot *******\n");
                    sysWDT_enable();
                    sysForceWDTtoReboot();
                    uiFakeShutdown =0;
                }
                gpioTimerCtrLed(LED_GREEN_OFF);
                gpioTimerCtrLed(LED_RED_FLASH);
            }
            else
            {
                if (charge_status == 0) //justboot, or not charging
                {
                    if(adc_value > 0x998) //full or not charging, show adapter when full or over 80% battery
                    {
                        cur_status=UI_BATTERY_UNCHARGE;
                        //DEBUG_ADC("******* DRAW_ADAPTER_IN******\n");
                    }
                    else if (adc_value <= 0x998) //charging till full
                    {

                        cur_status = UI_BATTERY_CHARGE;
                        charge_status = 1;
                        //DEBUG_ADC("******* DRAW_CHARGING******\n");

                    }
                }
                else if(charge_status == 1)
                {
                    if(adc_2 < 0x800) //charging 0xFxx, discharging 0x000
                    {
                        cur_status=UI_BATTERY_UNCHARGE;
                        charge_status = 0;
                        //DEBUG_ADC("Full\n");
                    }
                    else// if(adc_2 > 0x800)
                    {
                        cur_status = UI_BATTERY_CHARGE;
                        //DEBUG_ADC("Charging\n");
                    }
                }

            }
            cnt = 0;
            adc_value = 0;
        }

        if (batteryflag != cur_status)
        {
            if (cur_status == UI_BATTERY_UNCHARGE)
            {
                gpioGetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, &gpiolevel);
                if (gpiolevel == 0)
                    gpioSetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, 1);
                gpioTimerCtrLed(LED_GREEN_ON);
                gpioTimerCtrLed(LED_RED_OFF);
            }
            else if (cur_status == UI_BATTERY_CHARGE)
            {
                uiCurAlertStatus &= ~UI_ALERT_STATUS_BATTERY_LOW;
                if (uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW, UI_BEEP_GET_TYPE) == UI_ALERT_TYPE_BATLOW)
                {
                    uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW_NONE, UI_BEEP_CHANGE);
                }
                gpioGetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, &gpiolevel);
                if (gpiolevel == 1)
                    gpioSetLevel(GPIO_GROUP_DISCHARGE_EN, GPIO_BIT_DISCHARGE_EN, 0);
                gpioTimerCtrLed(LED_GREEN_ON);
                gpioTimerCtrLed(LED_RED_ON);
            }
            batteryflag = cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);    
        }
    }


#elif((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592))
    

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, adc0:%03x adc1:%03x adc2:%03x\n", AdcRecData_G1G2, adc_0, adc_1, adc_2);
    
    cur_status=UI_BATTERY_NONE;
    //DEBUG_ADC(" ***** gpioLevel %d\n",gpioLevel);
    if((adc_0 > 0x7ff) && (adc_0 < 0xd00)) /* No Battery */
    {
        cur_status=UI_BATTERY_CLEAR;
        
        /* if battery broken , and now is in Fake Shutdown 
           It should only detect adapter plug in */
    #if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593) && (UI_PROJ_OPT == 13))
        if((((adc_1 > 0x000)&&(adc_1 <0x718)) || ((adc_1 > 0xd27)&&( adc_1 <0xfff))) && (uiFakeShutdown == 1))
    #else
        if((((adc_1 > 0x200)&&(adc_1 <0x400)) || ((adc_1 > 0x50)&&( adc_1 <0x110))) && (uiFakeShutdown == 1))
    #endif
        {
            cur_status=UI_BATTERY_CHARGE;
        }       
    }
    /* Battery +Adapter */
    /* adc_2: Low(No charge): 0x800, High(charge): 0x7ff */
    /* 0x2e0 < adc_1 < 0x400 (no docking) */
    /* 0x50  < adc_1 < 0x110 (with docking) */
    #if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593) && (UI_PROJ_OPT == 13))
    else if((((adc_1 > 0x000)&&(adc_1 <0x718)) || ((adc_1 > 0xd27)&&( adc_1 <0xfff))) && ((adc_2>0x600)&&(adc_2  < 0x800 )))
    #else
    else if((((adc_1 > 0x200)&&(adc_1 <0x400)) || ((adc_1 > 0x50)&&( adc_1 <0x110))) && ((adc_2>0x600)&&(adc_2  < 0x800 )))
    #endif
    {
        cur_status=UI_BATTERY_CHARGE;
    }
    else  /* Only Battery */
    {        
        if(cnt == 4)
        {
            adc_value =adc_value>>2;
            //DEBUG_ADC("ADC value %x \n",adc_value);
            if( adc_value >0x1f0)
            {
                cur_status=UI_BATTERY_LV4;
            }
            else if(adc_value >0x190)
            {
                cur_status=UI_BATTERY_LV3;
            }
            else if(adc_value >0x168)
            {
                cur_status=UI_BATTERY_LV2;
            }
            else if(adc_value >0xc0)
            {
                cur_status=UI_BATTERY_LV1;    
            }
            else
            {
                cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
            }    
            cnt=0;
            adc_value=0;
        } 
        else if(cnt <4)
        {
            cnt++;
            adc_value += adc_0;  
        }

    }

    
    //printf("%d curStatus %d lastLV:%d %x\n",cnt,cur_status,lastLV ,adc_value );
    if((batteryflag != cur_status) && (cur_status!= UI_BATTERY_NONE))
    {
        if (((lastLV >= UI_BATTERY_LV0)&&(lastLV < UI_BATTERY_LV5)) && (cur_status > lastLV)&& ((cur_status >=UI_BATTERY_LV0 )&&(cur_status < UI_BATTERY_LV5)))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }   
        
        lastLV=cur_status;
        if(cur_status == UI_BATTERY_CLEAR)   /* No Battery */
        {
            DEBUG_ADC("****** No Battery ******\n");
            batteryflag = UI_BATTERY_CLEAR;


            gpioTimerCtrLed(LED_RED_OFF);
            gpioTimerCtrLed(LED_GREEN_ON);


        }
        else if(cur_status == UI_BATTERY_CHARGE)  /* Battery +Adapter */
        {
            DEBUG_ADC("****** Charge ******\n");
            batteryflag =UI_BATTERY_CHARGE;

            gpioTimerCtrLed(LED_RED_ON);
            gpioTimerCtrLed(LED_GREEN_ON);

            uiCurAlertStatus &= ~UI_ALERT_STATUS_BATTERY_LOW;
            if(uiFakeShutdown ==1)
            {
                uiFakeShutdown=0;
                sysForceWDTtoReboot();
            }
            if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) ==UI_ALERT_TYPE_BATLOW)
            {
                uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW_NONE,UI_BEEP_CHANGE);
            }
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
        {
            //DEBUG_ADC("******* Fake Shutdown *******");
            
            
            if(uiFakeShutdown ==0)
            {
                DEBUG_ADC("******* Fake Shutdown *******");
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
				sysForceWDTtoReboot();
            }
            
            batteryflag=UI_BATTERY_LV0;

        }
        else                                     /* Only Battery */
        {
            DEBUG_ADC("****** Only Battery ******\n");
            batteryflag = cur_status;
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            if(cur_status != UI_BATTERY_LV1)
            {
                gpioTimerCtrLed(LED_RED_OFF);
                gpioTimerCtrLed(LED_GREEN_ON);
            }
            else
            {
                uiCurAlertStatus |= UI_ALERT_STATUS_BATTERY_LOW;
                if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) != UI_ALERT_TYPE_BATLOW)
                    uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_CHANGE);
                gpioTimerCtrLed(LED_RED_FLASH);
                gpioTimerCtrLed(LED_GREEN_OFF);
            }
        }
    }

    if(batteryflag == UI_BATTERY_LV1)
    {
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);    
    }
    else
    {
        count_osd++;
        if((count_osd %10) == 0)
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);     
            count_osd=0;
        }
    }
    
#elif(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
        
    
        adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
        adc_1 = (AdcRecData_G1G2  &  0x0000fff);
        adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
        //printf("AdcRecData_G1G2 %08x, adc0:%03x adc1:%03x adc2:%03x\n", AdcRecData_G1G2, adc_0, adc_1, adc_2);
        
        cur_status=UI_BATTERY_NONE;
        //DEBUG_ADC(" ***** gpioLevel %d\n",gpioLevel);
        if((adc_0 > 0x7ff) && (adc_0 < 0xd00)) /* No Battery */
        {
            cur_status=UI_BATTERY_CLEAR;
            
            /* if battery broken , and now is in Fake Shutdown 
               It should only detect adapter plug in */
            if((((adc_1 > 0x200)&&(adc_1 <0x400)) || ((adc_1 > 0x50)&&( adc_1 <0x110))) && (uiFakeShutdown == 1))
            {
                cur_status=UI_BATTERY_CHARGE;
            }       
        }
        /* Battery +Adapter */
        /* adc_2: Low(No charge): 0x800, High(charge): 0x7ff */
        /* 0x2e0 < adc_1 < 0x400 (no docking) */
        /* 0x50  < adc_1 < 0x110 (with docking) */
        else if((((adc_1 > 0x200)&&(adc_1 <0x400)) || ((adc_1 > 0x50)&&( adc_1 <0x110))) && ((adc_2>0x600)&&(adc_2  < 0x800 )))
        {
            cur_status=UI_BATTERY_CHARGE;
        }
        else  /* Only Battery */
        {        
            if(cnt == 4)
            {
                adc_value =adc_value>>2;
                //DEBUG_ADC("ADC value %x \n",adc_value);
                if( adc_value >0x249)//76)
                {
                    cur_status=UI_BATTERY_LV4;
                }
                else if(adc_value >0x1cc)//207)
                {
                    cur_status=UI_BATTERY_LV3;
                }
                else if(adc_value >0x179)//b3)
                {
                    cur_status=UI_BATTERY_LV2;
                }
                else if(adc_value >0x122)//33)//12)
                {
                    cur_status=UI_BATTERY_LV1;    
                }
                else
                {
                    cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                }    
                cnt=0;
                adc_value=0;
            } 
            else if(cnt <4)
            {
                cnt++;
                adc_value += adc_0;  
            }
    
        }
    
        
        //printf("%d curStatus %d lastLV:%d %x\n",cnt,cur_status,lastLV ,adc_value );
        if((batteryflag != cur_status) && (cur_status!= UI_BATTERY_NONE))
        {
            //DEBUG_ADC("===> curStatus %d lastLV:%d %x\n",cur_status,lastLV ,adc_value );
            if (((lastLV >= UI_BATTERY_LV0)&&(lastLV < UI_BATTERY_LV5)) && (cur_status > lastLV)&& ((cur_status >=UI_BATTERY_LV0 )&&(cur_status < UI_BATTERY_LV5)))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                return;
            }   
            
            lastLV=cur_status;
            if(cur_status == UI_BATTERY_CLEAR)   /* No Battery */
            {
                DEBUG_ADC("****** No Battery ******\n");
                batteryflag = UI_BATTERY_CLEAR;
    
    
                gpioTimerCtrLed(LED_RED_OFF);
                gpioTimerCtrLed(LED_GREEN_ON);
    
    
            }
            else if(cur_status == UI_BATTERY_CHARGE)  /* Battery +Adapter */
            {
                DEBUG_ADC("****** Charge ******\n");
                batteryflag =UI_BATTERY_CHARGE;
    
                gpioTimerCtrLed(LED_RED_ON);
                gpioTimerCtrLed(LED_GREEN_ON);
    
                uiCurAlertStatus &= ~UI_ALERT_STATUS_BATTERY_LOW;
                if(uiFakeShutdown ==1)
                {
                    uiFakeShutdown=0;
                    sysForceWDTtoReboot();
                }
                if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) ==UI_ALERT_TYPE_BATLOW)
                {
                    uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW_NONE,UI_BEEP_CHANGE);
                }
               // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            }
            else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
            {
                //DEBUG_ADC("******* Fake Shutdown *******");
                
                
                if(uiFakeShutdown ==0)
                {
                    DEBUG_ADC("******* Fake Shutdown *******");
                    uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
                    sysForceWDTtoReboot();
                }
                
                batteryflag=UI_BATTERY_LV0;
    
            }
            else                                     /* Only Battery */
            {
                DEBUG_ADC("****** Only Battery ******\n");
                batteryflag = cur_status;
               // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                if(cur_status != UI_BATTERY_LV1)
                {
                    gpioTimerCtrLed(LED_RED_OFF);
                    gpioTimerCtrLed(LED_GREEN_ON);
                }
                else
                {
                    uiCurAlertStatus |= UI_ALERT_STATUS_BATTERY_LOW;
                    if(uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_GET_TYPE) != UI_ALERT_TYPE_BATLOW)
                        uiFlowCheckAlert(UI_ALERT_TYPE_BATLOW,UI_BEEP_CHANGE);
                    gpioTimerCtrLed(LED_RED_FLASH);
                    gpioTimerCtrLed(LED_GREEN_OFF);
                }
            }
        }
    
        if(batteryflag == UI_BATTERY_LV1)
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);    
        }
        else
        {
            count_osd++;
            if((count_osd %10) == 0)
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);     
                count_osd=0;
            }
        }
#elif(HW_BOARD_OPTION == MR8100_GCT_LCD)

        if (Main_Init_Ready == 0)
            return;
        cur_status=UI_BATTERY_NONE;
        adc_0 = adcGetValue(0);//(AdcRecData_G1G2  &  0xfff0000)>>16;
        adc_2 = adcGetValue(2);//(AdcRecData_G3G4  &  0xfff0000)>>16;
        gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
        //printf("AdcRecData_G3G4 %08x, %03x %03x, level=%d\n", AdcRecData_G3G4, adc_0, adc_2, level);
        //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, level);

        cur_status_dc=0;
#if 0
        if(adc_0 > 0x600)
        {
            if(adc_2 > 0x600)
            {
                if(batteryflag != UI_BATTERY_CLEAR)
                {
                    batteryflag=UI_BATTERY_CLEAR;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            }
            //else if((adc_0 > 0x600) && (0x500 >= adc_2))
            else if(level == 0)
            {
                if(batteryflag != UI_BATTERY_CHARGE)
                {
                    batteryflag=UI_BATTERY_CHARGE;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            }
            return;
        }

        if (cnt < 5)
        {
            tmpAdcVal[cnt] = adc_2;
            cnt++;
        }
        else
        {
            cnt = 0;
            if((tmpAdcVal[0]>(tmpAdcVal[2]-adcBatErrRange)) &&(tmpAdcVal[0]<(tmpAdcVal[2]+adcBatErrRange)))
            {
                if((tmpAdcVal[0]>(tmpAdcVal[3]-adcBatErrRange)) &&(tmpAdcVal[0]<(tmpAdcVal[3]+adcBatErrRange))) 
                {
                    if((tmpAdcVal[0]>(tmpAdcVal[4]-adcBatErrRange)) &&(tmpAdcVal[0]<(tmpAdcVal[4]+adcBatErrRange))) 
                    {
                        adc_value = tmpAdcVal[0];
                        Bat_Check_Ok_Flag=1;      
                    }
                    else
                        Bat_Check_Ok_Flag=0;         
                } 
                else
                    Bat_Check_Ok_Flag=0;     
            } 
            else
            {
                if((tmpAdcVal[1] > (tmpAdcVal[3] - adcBatErrRange)) &&(tmpAdcVal[1] < (tmpAdcVal[3] + adcBatErrRange))) 
                {
                    if((tmpAdcVal[1]>(tmpAdcVal[4] - adcBatErrRange)) &&(tmpAdcVal[1] < (tmpAdcVal[4] + adcBatErrRange))) 
                    {
                        adc_value = tmpAdcVal[1]; 
                        Bat_Check_Ok_Flag = 1;        
                    }
                    else
                        Bat_Check_Ok_Flag = 0;     
                } 
                else
                    Bat_Check_Ok_Flag = 0;                          
            }  
    
        }
        // draw battery level
        if(Bat_Check_Ok_Flag == 1)
        {
            DEBUG_ADC("===> ADC value %x \n",adc_value);
            if( adc_value > 0x530)// 0x440)
            {
                cur_status=UI_BATTERY_LV4;
            }
            else if(adc_value > 0x3b0)// 0x420)
            {
                cur_status=UI_BATTERY_LV3;
            }
            else if(adc_value > 0x360)// 0x3e0)
            {
                cur_status=UI_BATTERY_LV2;
            }
            else if(adc_value > 0x2f0)// 0x3c0)
            {
                cur_status=UI_BATTERY_LV1;    
            }
            else 
            {
                //batteryflag = UI_BATTERY_SHUTDOWN;
                cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                Beep_function(1,200,60,200,200,0);
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
            }    
        } 
        //printf("===> batteryflag = %d\n",batteryflag);
        if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
        {
            batteryflag=cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
#else
        //=======================================
        if(cnt == 8)
        {
            adc_value=0;
            for(i=2;i<6;i++)
            {
                adc_value=adc_value+adc_level[i];
                //DEBUG_ADC("===> adc_level[%d] = %x \n",i,adc_level[i]);
            }
            adc_value = adc_value>>2;
            //DEBUG_ADC("===> adc_value = %x \n",adc_value);
            cnt=0;
        } 
        else if(cnt < 8)
        {
            if(cnt == 0)
            {
                adc_level[0] = adc_2;
                for(i=1;i<8;i++)
                    adc_level[i] = 0;
                cnt++;
            }
            else
            {
                for(i=0;i<cnt;i++)
                {
                    if(adc_2 > adc_level[i])
                    {
                        //for(j=(cnt-1);j>i;j--)
                        for(j=cnt;j>i;j--)
                            adc_level[j] = adc_level[j-1];
                        adc_level[i] = adc_2;
                        i=cnt;
                    }
                }
                if(adc_level[cnt]==0)
                    adc_level[cnt] = adc_2;
                cnt++;
            }
        }
        //=======================================

        //if((adc_0 < 0x600) && (adc_2 < 0xd50)) // charge
        bcharge=1; //  no charge
        if(adc_0 < 0x600)// charge
        {
            if(level == 0)
                count_osd++;
            if(cnt1 > 4)
            {
                if(count_osd > 2)
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 0);
                    bcharge=0; // charge
                    if(batteryflag != UI_BATTERY_CHARGE)
                    {
                        batteryflag=UI_BATTERY_CHARGE;
                        if(uiFakeShutdown ==0)
                            gpioTimerCtrLed(LED_RED_ON);
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else if(adc_value > 0xe14)
                {
                    if(batteryflag != UI_BATTERY_CLEAR)
                    {
                        batteryflag=UI_BATTERY_CLEAR;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 1);
                    
                    if((batteryflag != UI_BATTERY_LV5) && (adc_value > 0))
                    {
                        batteryflag=UI_BATTERY_LV5;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, bcharge);
                count_osd=0;
                cnt1=0;
                shutdown_cnt = 0;
                rnu_one = 0;
            }
            cnt1++;
            return;
        }
        else // draw battery level
        {
			if ((level == 1) && (uiFakeShutdown == 1))
				gpioTimerCtrLed(LED_RED_ON);

            #if 1
            if(cnt > 0)
                return;
            //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, bcharge);
            if (rnu_one == 0)
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    cur_status=UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    cur_status=UI_BATTERY_LV3;
                }
                else if(adc_value > 0xbb0)// 3.71v
                {
                    cur_status=UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    cur_status=UI_BATTERY_LV1;    
                }
                else
                {
                    //batteryflag = UI_BATTERY_SHUTDOWN;
                    cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                    gpioTimerCtrLed(LED_RED_FLASH);
                }
                    rnu_one = 1;
            }
            else
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    if (check_flag == UI_BATTERY_LV4)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    if (check_flag == UI_BATTERY_LV3)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV3;
                }
                else if(adc_value > 0xbb0)// 3.71v
                {
                    if (check_flag == UI_BATTERY_LV2)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    if (check_flag == UI_BATTERY_LV1)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV1;
                }
                else
                {
                    if (check_flag == UI_BATTERY_LV0)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV0;
                    if (shutdown_cnt > 2000) // 1200sec / 0.9 = 1333 1800sec / 0.9 = 2000
                    {
                        uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
                    }
                    else
                        shutdown_cnt++;
                }
                if(check_cnt < 5 )
                {
                    cur_status = batteryflag;
                }
                else
                {
                    check_cnt = 0;
                    if( adc_value > 0xc50)// 3.95v
                    {
                        cur_status=UI_BATTERY_LV4;
                    }
                    else if(adc_value > 0xbe0)// 3.8V
                    {
                        cur_status=UI_BATTERY_LV3;
                    }
                    else if(adc_value > 0xbb0)// 3.71v
                    {
                        cur_status=UI_BATTERY_LV2;
                    }
                    else if(adc_value > 0xb60)// 3.62V
                    {
                        cur_status=UI_BATTERY_LV1;    
                    }
                    else
                    {
                        cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                        gpioTimerCtrLed(LED_RED_FLASH);
                    }
                }
            }
            #else
            if(cnt == 8)
            {
                for(i=2;i<6;i++)
                {
                    adc_value=adc_value+adc_level[i];
                    //DEBUG_ADC("===> adc_level[%d] = %x \n",i,adc_level[i]);
                }
                adc_value = adc_value>>2;
                //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_value, level);
                //DEBUG_ADC("===> ADC value %x \n",adc_value);
                if( adc_value > 0xc40)// 0x530)// 0x440) 3.9v
                {
                    cur_status=UI_BATTERY_LV4;
                }
                else if(adc_value > 0xba0)// 0x3b0)// 0x420) 3.7v
                {
                    cur_status=UI_BATTERY_LV3;
                }
                else if(adc_value > 0xb50)// 0x360)// 0x3e0) 3.6v
                {
                    cur_status=UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb10)// 0x2f0)// 0x3c0) 3.47
                {
                    cur_status=UI_BATTERY_LV1;    
                }
                else 
                {
                    //batteryflag = UI_BATTERY_SHUTDOWN;
                    cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                    if(iconflag[UI_MENU_SETIDX_VOLUME]>0)
                        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
                    Beep_function(1,200,60,200,200,0);
                    uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
                }    
                cnt=0;
                adc_value=0;
            } 
            else if(cnt < 8)
            {
                if(cnt == 0)
                {
                    adc_level[0] = adc_2;
                    for(i=1;i<8;i++)
                        adc_level[i] = 0;
                    cnt++;
                }
                else
                {
                    cnt++;
                    for(i=0;i<cnt;i++)
                    {
                        if(adc_2 > adc_level[i])
                        {
                            for(j=(cnt-1);j>i;j--)
                                adc_level[j] = adc_level[j-1];
                            adc_level[i] = adc_2;
                            return;
                        }
                    }
                }
            }
            #endif
        }
        //printf(" %4d %d %d %d %d\n",shutdown_cnt,cur_status,batteryflag,check_cnt,adc_value);
        //printf("===> batteryflag = %d\n",batteryflag);
        if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
        {
            batteryflag=cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
#endif
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
        if (Main_Init_Ready == 0)
            return 0;
        cur_status=UI_BATTERY_NONE;
        adc_0 = adcGetValue(0);//(AdcRecData_G1G2  &  0xfff0000)>>16;
        adc_2 = adcGetValue(2);//(AdcRecData_G3G4  &  0xfff0000)>>16;
        gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
        //printf("AdcRecData_G3G4 %08x, %03x %03x, level=%d\n", AdcRecData_G3G4, adc_0, adc_2, level);
        //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, level);
        //RTC_Get_Time(&localTime);
        //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

        cur_status_dc=0;

        //=======================================
        if(cnt == 8)
        {
            adc_value=0;
            for(i=2;i<6;i++)
            {
                adc_value=adc_value+adc_level[i];
                //DEBUG_ADC("===> adc_level[%d] = %x \n",i,adc_level[i]);
            }
            adc_value = adc_value>>2;
            //DEBUG_ADC("===> adc_value = %x \n",adc_value);
            cnt=0;
        } 
        else if(cnt < 8)
        {
            if(cnt == 0)
            {
                adc_level[0] = adc_2;
                for(i=1;i<8;i++)
                    adc_level[i] = 0;
                cnt++;
            }
            else
            {
                for(i=0;i<cnt;i++)
                {
                    if(adc_2 > adc_level[i])
                    {
                        //for(j=(cnt-1);j>i;j--)
                        for(j=cnt;j>i;j--)
                            adc_level[j] = adc_level[j-1];
                        adc_level[i] = adc_2;
                        i=cnt;
                    }
                }
                if(adc_level[cnt]==0)
                    adc_level[cnt] = adc_2;
                cnt++;
            }
        }
        //=======================================

        //if((adc_0 < 0x600) && (adc_2 < 0xd50)) // charge
        if(adc_0 < 0x600)// adapter plug in
        {
            //printf("level %d adc_value %d \n", level, adc_value);
            if(cnt1 > 4)
            {
            #if 1   /*目前沒辦法判斷沒電池與充飽電的狀況，先暫時改成沒電池也顯示充飽電*/
                if(level == 0)   /*charge*/
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 0);
                    if(batteryflag != UI_BATTERY_CHARGE)
                    {
                        batteryflag=UI_BATTERY_CHARGE;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else    /*充飽電*/
                {
                    batteryflag=UI_BATTERY_LV5;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            #else
                if(adc_value > 0xD00)  /*no battery*/
                {
                    if(batteryflag != UI_BATTERY_CLEAR)
                    {
                        batteryflag=UI_BATTERY_CLEAR;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else if(level == 0)   /*charge*/
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 0);
                    if(batteryflag != UI_BATTERY_CHARGE)
                    {
                        batteryflag=UI_BATTERY_CHARGE;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else    /*充飽電*/
                {
                    batteryflag=UI_BATTERY_LV5;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            #endif
                //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, level);
                cnt1=0;
                shutdown_cnt = 0;
                rnu_one = 0;
            }
            cnt1++;
            return 0;
        }
        else // draw battery level
        {
            if(cnt > 0)
                return;
            //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, level);
            if (rnu_one == 0)
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    cur_status=UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    cur_status=UI_BATTERY_LV3;
                }
                else if(adc_value > 0xba6)// 3.71v
                {
                    cur_status=UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    cur_status=UI_BATTERY_LV1;    
                }
                else if(adc_value > 0xb00)// 3.62V
                {
                    cur_status=UI_BATTERY_LV0;    
                }
                else
                {
                    //batteryflag = UI_BATTERY_SHUTDOWN;
                    cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                }
                    rnu_one = 1;
            }
            else
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    if (check_flag == UI_BATTERY_LV4)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    if (check_flag == UI_BATTERY_LV3)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV3;
                }
                else if(adc_value > 0xba6)// 3.71v
                {
                    if (check_flag == UI_BATTERY_LV2)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    if (check_flag == UI_BATTERY_LV1)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV1;
                }
                else if(adc_value > 0xb00)// 3.62V
                {
                    if (check_flag == UI_BATTERY_LV0)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV0;
                }
                else
                {
                    if (check_flag == UI_BATTERY_LV0)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV0;
                    if (shutdown_cnt > 1) // 1200sec / 0.9 = 1333 1800sec / 0.9 = 2000
                    {
                        uiSentKeyToUi(UI_KEY_PWR_OFF);
                    }
                    else
                        shutdown_cnt++;
                }
                if(check_cnt < 5 )
                {
                    cur_status = batteryflag;
                }
                else
                {
                    check_cnt = 0;
                    if( adc_value > 0xc50)// 3.95v
                    {
                        cur_status=UI_BATTERY_LV4;
                    }
                    else if(adc_value > 0xbe0)// 3.8V
                    {
                        cur_status=UI_BATTERY_LV3;
                    }
                    else if(adc_value > 0xba6)// 3.71v
                    {
                        cur_status=UI_BATTERY_LV2;
                    }
                    else if(adc_value > 0xb60)// 3.62V
                    {
                        cur_status=UI_BATTERY_LV1;    
                    }
                    else if(adc_value > 0xb00)// 3.62V
                    {
                        cur_status=UI_BATTERY_LV0;    
                    }
                    else
                    {
                        cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                    }
                }
            }
        }
        //printf(" %4d %d %d %d %d\n",shutdown_cnt,cur_status,batteryflag,check_cnt,adc_value);
        //printf("===> batteryflag = %d\n",batteryflag);
        if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
        {
            batteryflag=cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
        if (Main_Init_Ready == 0)
            return 0;
        cur_status=UI_BATTERY_NONE;
        adc_0 = adcGetValue(0);//(AdcRecData_G1G2  &  0xfff0000)>>16;
        adc_2 = adcGetValue(2);//(AdcRecData_G3G4  &  0xfff0000)>>16;
        gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
        //printf("AdcRecData_G3G4 %08x, %03x %03x, level=%d\n", AdcRecData_G3G4, adc_0, adc_2, level);
        //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, level);
        //RTC_Get_Time(&localTime);
        //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

        cur_status_dc=0;

        //=======================================
        if(cnt == 8)
        {
            adc_value=0;
            for(i=2;i<6;i++)
            {
                adc_value=adc_value+adc_level[i];
                //DEBUG_ADC("===> adc_level[%d] = %x \n",i,adc_level[i]);
            }
            adc_value = adc_value>>2;
            //DEBUG_ADC("===> adc_value = %x \n",adc_value);
            cnt=0;
        } 
        else if(cnt < 8)
        {
            if(cnt == 0)
            {
                adc_level[0] = adc_2;
                for(i=1;i<8;i++)
                    adc_level[i] = 0;
                cnt++;
            }
            else
            {
                for(i=0;i<cnt;i++)
                {
                    if(adc_2 > adc_level[i])
                    {
                        //for(j=(cnt-1);j>i;j--)
                        for(j=cnt;j>i;j--)
                            adc_level[j] = adc_level[j-1];
                        adc_level[i] = adc_2;
                        i=cnt;
                    }
                }
                if(adc_level[cnt]==0)
                    adc_level[cnt] = adc_2;
                cnt++;
            }
        }
        //=======================================

        //if((adc_0 < 0x600) && (adc_2 < 0xd50)) // charge
        if(adc_0 < 0x600)// adapter plug in
        {
            //printf("level %d adc_value %d \n", level, adc_value);
            if(cnt1 > 4)
            {
            #if 1   /*目前沒辦法判斷沒電池與充飽電的狀況，先暫時改成沒電池也顯示充飽電*/
                if(level == 0)   /*charge*/
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 0);
                    if(batteryflag != UI_BATTERY_CHARGE)
                    {
                        batteryflag=UI_BATTERY_CHARGE;
                        gpioTimerCtrLed(LED_ON);
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else    /*充飽電*/
                {
                    batteryflag=UI_BATTERY_LV5;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            #else
                if(adc_value > 0xD00)  /*no battery*/
                {
                    if(batteryflag != UI_BATTERY_CLEAR)
                    {
                        batteryflag=UI_BATTERY_CLEAR;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else if(level == 0)   /*charge*/
                {
                    //printf("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, 0);
                    if(batteryflag != UI_BATTERY_CHARGE)
                    {
                        batteryflag=UI_BATTERY_CHARGE;
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                    }
                }
                else    /*充飽電*/
                {
                    batteryflag=UI_BATTERY_LV5;
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            #endif
                //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, level);
                cnt1=0;
                shutdown_cnt = 0;
                rnu_one = 0;
            }
            cnt1++;
            return 0;
        }
        else // draw battery level
        {
            if(cnt > 0)
                return;
            //printf("===> %d adc_0 = %03x, adc_2 = %3x, level=%d\n", __LINE__, adc_0, adc_value, level);
            if (rnu_one == 0)
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    cur_status=UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    cur_status=UI_BATTERY_LV3;
                }
                else if(adc_value > 0xba6)// 3.71v
                {
                    cur_status=UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    cur_status=UI_BATTERY_LV1;    
                }
                else if(adc_value > 0xb00)// 3.62V
                {
                    cur_status=UI_BATTERY_LV0;    
                }
                else
                {
                    //batteryflag = UI_BATTERY_SHUTDOWN;
                    cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                }
                    rnu_one = 1;
            }
            else
            {
                if( adc_value > 0xc50)// 3.95v
                {
                    if (check_flag == UI_BATTERY_LV4)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV4;
                }
                else if(adc_value > 0xbe0)// 3.8V
                {
                    if (check_flag == UI_BATTERY_LV3)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV3;
                }
                else if(adc_value > 0xba6)// 3.71v
                {
                    if (check_flag == UI_BATTERY_LV2)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV2;
                }
                else if(adc_value > 0xb60)// 3.62V
                {
                    if (check_flag == UI_BATTERY_LV1)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV1;
                }
                else if(adc_value > 0xb00)// 3.62V
                {
                    if (check_flag == UI_BATTERY_LV0)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV0;
                }
                else
                {
                    if (check_flag == UI_BATTERY_LV0)
                        check_cnt++;
                    else
                        check_cnt = 0;
                    check_flag = UI_BATTERY_LV0;
                    if (shutdown_cnt > 1) // 1200sec / 0.9 = 1333 1800sec / 0.9 = 2000
                    {
                        uiSentKeyToUi(UI_KEY_PWR_OFF);
                    }
                    else
                        shutdown_cnt++;
                }
                if(check_cnt < 5 )
                {
                    cur_status = batteryflag;
                }
                else
                {
                    check_cnt = 0;
                    if( adc_value > 0xc50)// 3.95v
                    {
                        cur_status=UI_BATTERY_LV4;
                    }
                    else if(adc_value > 0xbe0)// 3.8V
                    {
                        cur_status=UI_BATTERY_LV3;
                    }
                    else if(adc_value > 0xba6)// 3.71v
                    {
                        cur_status=UI_BATTERY_LV2;
                    }
                    else if(adc_value > 0xb60)// 3.62V
                    {
                        cur_status=UI_BATTERY_LV1;    
                    }
                    else if(adc_value > 0xb00)// 3.62V
                    {
                        cur_status=UI_BATTERY_LV0;    
                    }
                    else
                    {
                        cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
                    }
                }
            }
                
        }
        //printf(" %4d %d %d %d %d\n",shutdown_cnt,cur_status,batteryflag,check_cnt,adc_value);
        //printf("===> batteryflag = %d\n",batteryflag);
        if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
        {
            batteryflag=cur_status;
            if (batteryflag == UI_BATTERY_LV0)
                gpioTimerCtrLed(LED_FLASH);
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
#elif(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)
    return;
    //RTC_Get_Time(&localTime);
    detect_cnt=10;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;

    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
    cur_status=UI_BATTERY_NONE;
    if( adc_0 < 0x900) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //printf("AdcRecData_G3G4 %08x, %03x %03x, level=%d\n", AdcRecData_G3G4, adc_0, adc_2, level);

    if((level == GPIO_LEVEL_LO) && (cur_status_dc == 1))
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if(cnt == 4)
        {
            adc_value =adc_value>>2;
            DEBUG_ADC("===> ADC value %x \n",adc_value);
            if( adc_value >0x158)
            {
                cur_status=UI_BATTERY_LV3;
            }
            else if(adc_value >0x0a2)
            {
                cur_status=UI_BATTERY_LV2;
            }
            else if(adc_value >0x07e)
            {
                cur_status=UI_BATTERY_LV1;
            }
            else if(adc_value >0x05c)
            {
                cur_status=UI_BATTERY_LV0;    
            }
            else 
            {
                cur_status=UI_BATTERY_LV0;    /* LV0 enter fake shutdown*/
            }    
            cnt=0;
            adc_value=0;
        } 
        else if(cnt <4)
        {
            cnt++;
            adc_value += adc_2;  
        }
    }

    if((batteryflag != cur_status) && (cur_status!= UI_BATTERY_NONE))
    {
        DEBUG_ADC("===> curStatus %d lastLV:%d %x\n",cur_status,lastLV ,adc_value );
        if (((lastLV >= UI_BATTERY_LV0)&&(lastLV < UI_BATTERY_LV3)) && (cur_status > lastLV)&& ((cur_status >=UI_BATTERY_LV0 )&&(cur_status < UI_BATTERY_LV3)))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }   
        
        lastLV=cur_status;
        if(cur_status == UI_BATTERY_CLEAR)   /* No Battery */
        {
            DEBUG_ADC("****** No Battery ******\n");
            batteryflag = UI_BATTERY_CLEAR;
        }
        else if(cur_status == UI_BATTERY_CHARGE)  /* Battery +Adapter */
        {
            DEBUG_ADC("****** Charge ******\n");
            batteryflag =UI_BATTERY_CHARGE;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        #if 0
        else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
        {
            //DEBUG_ADC("******* Fake Shutdown *******");
            
            
            if(uiFakeShutdown ==0)
            {
                DEBUG_ADC("******* Fake Shutdown *******");
                uiSentKeyToUi(UI_KEY_FAKE_SHUTODWN);
                sysForceWDTtoReboot();
            }
            
            batteryflag=UI_BATTERY_LV0;

        }
        #endif
        else                                     /* Only Battery */
        {
            DEBUG_ADC("****** Only Battery ******\n");
            batteryflag = cur_status;
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
    }
    #if 0
    count_osd++;
    if((count_osd %10) == 0)
    {
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);     
        count_osd=0;
    }
    #endif
#if 0 
    if  ((lastLV == UI_BATTERY_LV3) && (cur_status != UI_BATTERY_LV3))
    {
        cnt1=detect_cnt;
    }
    if(cur_status_dc != dc_detect)
    {
        cnt1 = detect_cnt;
        dc_detect = cur_status_dc;
    }

    DEBUG_ADC("GPIO:%d cur:%d, lastLV:%d \n",level,cur_status,lastLV);
    if ((lastLV != cur_status) && (cur_status!= UI_BATTERY_NONE))
    {
        if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV3))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            DEBUG_ADC("draw battery lastLV = %d cur_status = %d\n",lastLV,cur_status);
            return;
        }
        if(cnt1 == detect_cnt)
        {
            lastLV = cur_status;
            batteryflag = cur_status;
            cnt1 = 0;
//            if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV3))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                DEBUG_ADC("draw battery lastLV = %d cur_status = %d\n",lastLV,cur_status);
                return;
            }
            DEBUG_ADC("batteryflag = %d lastLV = %d cur_status = %d\n",batteryflag,lastLV,cur_status);
#if 0
            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                //uiSentKeyToUi(UI_KEY_PWR_OFF);
                //return;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                //gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    //gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    //gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    //gpioTimerCtrLed(LED_OFF);
                }

            }
            #endif
        }
        else
            cnt1++;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
    #endif

#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M900)||((HW_BOARD_OPTION == MR8200_RX_RDI_M902) && (PROJ_OPT == 1)) ||\
    	((HW_BOARD_OPTION == MR8200_RX_RDI_M701)&&(PROJ_OPT == 0)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M902) && (PROJ_OPT == 4)))
    
    //RTC_Get_Time(&localTime);
    detect_cnt=10;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);

  #if (HW_BOARD_OPTION == MR8200_RX_RDI_M900)
    if(level ==GPIO_LEVEL_HI)
  #else
    if((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1))
  #endif
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (adc_2 > 0x1a0)   // 149
            cur_status = UI_BATTERY_LV5;
        else if (adc_2 > 0x149)  //FF
            cur_status = UI_BATTERY_LV4;
        else if (adc_2 > 0xFF)  //CE
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xCE)   //A2
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0xAE)   //85
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 0x67)   //47
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
    }
    
    if  ((lastLV == UI_BATTERY_LV5) && (cur_status != UI_BATTERY_LV5))
    {
        cnt=detect_cnt;
    }

    if(cur_status_dc != dc_detect)
    {
        cnt = detect_cnt;
        dc_detect = cur_status_dc;
    }
    //DEBUG_ADC("GPIO:%d cur:%d, lastLV:%d \n",level,cur_status,lastLV);
    if (lastLV != cur_status)
    {
        if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV5))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }
        
        if(cnt == detect_cnt)
        {
            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                uiSentKeyToUi(UI_KEY_PWR_OFF);
                return;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }

            }
        }
        else
            cnt++;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#elif (((HW_BOARD_OPTION == MR8600_RX_RDI_M904D)&&(PROJ_OPT == 0))||(HW_BOARD_OPTION == MR8200_RX_RDI_M902))
        
        RTC_Get_Time(&localTime);
        detect_cnt=10;

        adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

        adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
        //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
        //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

        if( adc_0 >0x7ff) // adapter plug out
        {
            cur_status_dc = 0;
        }
        else  // adapter plug in
        {
            cur_status_dc = 1;
        }

        gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);

  #if (HW_BOARD_OPTION == MR8200_RX_RDI_M900)
        if(level ==GPIO_LEVEL_HI)
  #else
        if((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1))
  #endif
        {
            cur_status = UI_BATTERY_CHARGE;
        }
        else
        {
            if (adc_2 > 0x140) // 0x19A
                cur_status = UI_BATTERY_LV5;
            else if (adc_2 > 0x120) // 0x142
                cur_status = UI_BATTERY_LV4;
            else if (adc_2 > 0xf0) // 0x0EF
                cur_status = UI_BATTERY_LV3;
            else if (adc_2 > 0xc0)  // 0xDD
                cur_status = UI_BATTERY_LV2;
            else if (adc_2 > 0xa0)  // 0xD1
                cur_status = UI_BATTERY_LV1;
            else if (adc_2 > 0x74)  // 0x74
                cur_status = UI_BATTERY_LV0;
            else
                cur_status = UI_BATTERY_SHUTDOWN;
        }
    
    if  ((lastLV == UI_BATTERY_LV5) && (cur_status != UI_BATTERY_LV5))
    {
        cnt=detect_cnt;
    }

    if(cur_status_dc != dc_detect)
    {
            cnt = detect_cnt;
            dc_detect = cur_status_dc;
        }
        //DEBUG_ADC("GPIO:%d cur:%d, lastLV:%d \n",level,cur_status,lastLV);
        if (lastLV != cur_status)
        {
            if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV5))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                return;
            }
            
            if(cnt == detect_cnt)
            {
                lastLV = cur_status;
                batteryflag = cur_status;
                cnt = 0;

                if(batteryflag == UI_BATTERY_SHUTDOWN)
                {
                    uiSentKeyToUi(UI_KEY_PWR_OFF);
                    return;
                }
                else if(batteryflag == UI_BATTERY_LV0) //could be an issue about battery icon no flash
                {
                    gpioTimerCtrLed(LED_FLASH);
                }
                else if(batteryflag == UI_BATTERY_CHARGE)
                {
                    if(uiEnterScanMode ==1)
                    {
                        gpioTimerCtrLed(LED_ON);    
                    }
                    else
                    {
                        gpioTimerCtrLed(LED_OFF);
                    }
                }
                else
                {
                    if(uiEnterScanMode != 1)
                    {
                        gpioTimerCtrLed(LED_OFF);
                    }
    
                }
            }
            else
                cnt++;
        }
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M713)&&(PROJ_OPT == 1)) 

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
    printf("adc_0 %03x adc_2 %03x\n", adc_0, adc_2);

    cur_status = UI_BATTERY_NONE;

    if (adc_0 > 0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);

    if((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1))
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (batteryflag == UI_BATTERY_CHARGE)
        {
            cnt = 0;
            adc_value = 0;
            cur_status = UI_BATTERY_CLEAR;
            batteryflag = cur_status;
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        if(cnt < (2-1))
        {
            cnt++;
            adc_value += adc_2;
        }
        else
        {
            cnt++;
            adc_value += adc_2;
            adc_value = adc_value >> 1;
            DEBUG_ADC("ADC value %x\n", adc_value);

            if (adc_value > 0x17C)
                cur_status = UI_BATTERY_LV5;
            else if (adc_value > 0x152)
                cur_status = UI_BATTERY_LV4;
            else if (adc_value > 0x124)
                cur_status = UI_BATTERY_LV3;
            else if (adc_value > 0x107)
                cur_status = UI_BATTERY_LV2;
            else if (adc_value > 0xDA)
                cur_status = UI_BATTERY_LV1;
            else if (adc_value > 0x9A)
                cur_status = UI_BATTERY_LV0;
            else
                cur_status = UI_BATTERY_SHUTDOWN;
            
            cnt = 0;
            adc_value = 0;
        }
    }

    if (batteryflag != cur_status && cur_status != UI_BATTERY_NONE)
    {
       if ((cur_status > batteryflag) && (cur_status <= UI_BATTERY_LV5))
       {
           DEBUG_ADC("Battery level don't go up, invaild return\n");
           return;
       }

        batteryflag = cur_status;

        if(batteryflag == UI_BATTERY_SHUTDOWN)
        {
            uiSentKeyToUi(UI_KEY_PWR_OFF);
        }
        else if(batteryflag == UI_BATTERY_LV1)
        {
            gpioTimerCtrLed(LED_FLASH);
        }
        else if(batteryflag == UI_BATTERY_CHARGE)
        {
            if(uiEnterScanMode == 1)
            {
                gpioTimerCtrLed(LED_ON);    
            }
            else
            {
                gpioTimerCtrLed(LED_OFF);
            }
        }
        else
        {
            if(uiEnterScanMode != 1)
            {
                gpioTimerCtrLed(LED_OFF);
            }
        }
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M920)
#if(PROJ_OPT == 9)
return ;
#endif
    RTC_Get_Time(&localTime);
    detect_cnt=10;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }


    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);

    if( adc_2 > 0x2f0)     // no battery
    {
        cur_status = UI_BATTERY_LV5;
        lastLV=cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        return 0;
    }
    else if((level == GPIO_LEVEL_LO) && (cur_status_dc == 1))   // charging
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (adc_2 > 0x188)       // V10: 0x149
            cur_status = UI_BATTERY_LV5;
        else if (adc_2 > 0x149)  // V10:0x110, V09  FF
            cur_status = UI_BATTERY_LV4;
        else if (adc_2 > 0xFF)   // V09: CE
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xCE)   // V09: AE
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0xAE)   // V09: 67
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 0x67)   // V09: 53
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
    }

    if  ((lastLV == UI_BATTERY_LV5) && (cur_status != UI_BATTERY_LV5))
    {
        cnt=detect_cnt;
    }


    if(cur_status_dc != dc_detect)
    {
        cnt = detect_cnt;
        dc_detect = cur_status_dc;
    }
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV5))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }
        if(cnt == detect_cnt)
        {
            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                uiSentKeyToUi(UI_KEY_PWR_OFF);
                batteryflag= UI_BATTERY_LV0;
                return 0;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
        }
        else
            cnt++;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);


#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M742) || (HW_BOARD_OPTION == MR8200_RX_RDI_M731) ||\
    (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA) || (HW_BOARD_OPTION == MR8200_RX_RDI_M742_HA)||\
    (HW_BOARD_OPTION == MR8120_RX_RDI_M733))
#if((HW_BOARD_OPTION  == MR8200_RX_RDI_M742_HA)&&(PROJ_OPT == 2))
    return;
#endif
    RTC_Get_Time(&localTime);

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);


    if( adc_2 >0x2f0)   // no battery
    {
        cur_status = UI_BATTERY_LV3;
        lastLV=cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        return 0;
    }
    else if ((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1)) // charging
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (adc_2 > 0x11C)
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xC9)
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0x99)
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 0x4f)
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
    }
    if ((lastLV == UI_BATTERY_LV3) && (cur_status != UI_BATTERY_LV3))
    {
        cnt=8;
    }

    if(cur_status_dc != dc_detect)
    {
        cnt=8;
        dc_detect=cur_status_dc;
    }
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        //if ((cur_status == UI_BATTERY_SHUTDOWN) && (cnt < 7))
        if (cnt < 7)
            cnt = 7;
        if(cnt == 8)
        {

            if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV3))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                return;
            }

            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                uiSentKeyToUi(UI_KEY_PWR_OFF);
                batteryflag= UI_BATTERY_LV0;
                return 0;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
        }
        else
            cnt++;
    }
    else
    {
        cnt=0;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
    
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M706)
    RTC_Get_Time(&localTime);

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //DEBUG_GREEN("AdcRecData_G1G2 %08x, %03x %03d\n", AdcRecData_G1G2, adc_0, adc_2);
    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    //DEBUG_GREEN("===> adc_0 = %03x, adc_2 = %3x, level=%d\n", adc_0, adc_2, level);
    //DEBUG_GREEN("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_2 >0x2f0)   // no battery
    {
        cur_status = UI_BATTERY_LV3;
        lastLV=cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        return 0;
    }
    else if ((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1)) // charging
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
#if ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 6))
        if (adc_2 > 250)
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 190)
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 160)
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 100)
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;

#elif(((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 7))||\
      ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
      if (adc_2 > 0x115)
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xe0)
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0x8c)
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 0x50)
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
        
#else        
      if (adc_2 > 210)
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 150)
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 120)
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 85)
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
#endif        
    }
    if ((lastLV == UI_BATTERY_LV3) && (cur_status != UI_BATTERY_LV3))
    {
        cnt=8;
    }

    if(cur_status_dc != dc_detect)
    {
        cnt=8;
        dc_detect=cur_status_dc;
    }
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        //if ((cur_status == UI_BATTERY_SHUTDOWN) && (cnt < 7))
        if (cnt < 7)
            cnt = 7;
        if(cnt == 8)
        {

            if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV3))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                return;
            }

            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                uiSentKeyToUi(UI_KEY_PWR_OFF);
                batteryflag= UI_BATTERY_LV0;
                return 0;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
        }
        else
            cnt++;
    }
    else
    {
        cnt=0;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#elif(HW_BOARD_OPTION == MR8200_RX_RDI_M930)
    RTC_Get_Time(&localTime);
    detect_cnt=10;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }


    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);

    if( adc_2 > 0x2f0)     // no battery
    {
        cur_status = UI_BATTERY_LV5;
        lastLV=cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        return 0;
    }
    else if((level == GPIO_LEVEL_LO) && (cur_status_dc == 1))   // charging
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (adc_2 > 0x188)       // V10: 0x149
            cur_status = UI_BATTERY_LV5;
        else if (adc_2 > 0x149)  // V10:0x110, V09  FF
            cur_status = UI_BATTERY_LV4;
        else if (adc_2 > 0xFF)   // V09: CE
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xCE)   // V09: AE
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0xAE)   // V09: 67
            cur_status = UI_BATTERY_LV1;
        else if (adc_2 > 0x67)   // V09: 53
            cur_status = UI_BATTERY_LV0;
        else
            cur_status = UI_BATTERY_SHUTDOWN;
    }

    if  ((lastLV == UI_BATTERY_LV5) && (cur_status != UI_BATTERY_LV5))
    {
        cnt=detect_cnt;
    }


    if(cur_status_dc != dc_detect)
    {
        cnt = detect_cnt;
        dc_detect = cur_status_dc;
    }
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV5))
        {
            sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
            return;
        }
        if(cnt == detect_cnt)
        {
            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                uiSentKeyToUi(UI_KEY_PWR_OFF);
                batteryflag= UI_BATTERY_LV0;
                return 0;
            }
            else if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
        }
        else
            cnt++;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);


#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) || (HW_BOARD_OPTION == MR8120_RX_RDI_M724))

#if(((HW_BOARD_OPTION == MR8200_RX_RDI_M721)&&(PROJ_OPT == 3)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M721)&&(PROJ_OPT == 4)) ||\
    ((HW_BOARD_OPTION == MR8200_RX_RDI_M721)&&(PROJ_OPT == 5)))
    return;
#endif

    RTC_Get_Time(&localTime);

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //printf("AdcRecData_G1G2 %08x, %03x %03x\n", AdcRecData_G1G2, adc_0, adc_2);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if( adc_0 >0x7ff) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);


    if( adc_2 >0x2f0)   // no battery
    {
        cur_status = UI_BATTERY_LV5;
        lastLV=cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        return 0;
    }
    else if ((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1)) // charging
    {
        cur_status = UI_BATTERY_CHARGE;
    }
    else
    {
        if (adc_2 > 0x160)
            cur_status = UI_BATTERY_LV5;
        else if (adc_2 > 0x130)  // V8:0x130 V7:120
            cur_status = UI_BATTERY_LV4;
        else if (adc_2 > 0xf0)   // V8:f0 V7:e0
            cur_status = UI_BATTERY_LV3;
        else if (adc_2 > 0xc0)   // V8:c0 V6/V7:0xb0 V5:0xa0
            cur_status = UI_BATTERY_LV2;
        else if (adc_2 > 0xa0)   // V8:a0 V6/V7:0x90 V5:0x80
            cur_status = UI_BATTERY_LV1;
        else
            cur_status = UI_BATTERY_LV0;
    }
    if ((lastLV == UI_BATTERY_LV5) && (cur_status != UI_BATTERY_LV5))
    {
        cnt=8;
    }


    if(cur_status_dc != dc_detect)
    {
        cnt=8;
        dc_detect=cur_status_dc;
    }
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        if(cnt == 8)
        {

            if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV)&& (cur_status <= UI_BATTERY_LV5))
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                return;
            }

            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;

            if(batteryflag == UI_BATTERY_LV0)
            {
                gpioTimerCtrLed(LED_FLASH);
            }
            else if(batteryflag == UI_BATTERY_CHARGE)
            {
                if(uiEnterScanMode ==1)
                {
                    gpioTimerCtrLed(LED_ON);    
                }
                else
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
            else
            {
                if(uiEnterScanMode != 1)
                {
                    gpioTimerCtrLed(LED_OFF);
                }
            }
        }
        else
            cnt++;
    }
    else
    {
        cnt=0;
    }
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#endif

    return batteryflag;
}



/*

Routine Description:

    The IRQ handler of ADC.

Arguments:

    None.

Return Value:

    None.

*/
void adcIntHandler(void)
{
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) )
    // test Group1 AGC function
    INT32U i, CurDMAIntStat, uiDMAChkBit=0x03;
    INT32U AVG_VOLUM = (ADCRX_AVG_VOLUMN & 0x0000ff00)>>8;
    CurDMAIntStat = (ADCRX_AVG_FIFO_INT & 0x000000f0) >> 4 ;

    if(CurDMAIntStat & 0x00000002)//group 1
    {
        if(AVG_VOLUM > 240)
            ;
        else if(AVG_VOLUM > 200)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1125;
        }
        else if(AVG_VOLUM > 100)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1500;
        }
        else if(AVG_VOLUM >  50)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1875;
        }
    }

 #elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A) ||\
 (CHIP_OPTION == CHIP_A1026A))
    // test Group1 AGC function
    INT32U i, CurDMAIntStat, uiDMAChkBit=0x03;
    INT32U AVG_VOLUM = (ADCRX_AVG_VOLUMN & 0x000000ff);
    // printf("E = 0x%08x",ADCRX_AVG_FIFO_INT);
    CurDMAIntStat = (ADCRX_AVG_FIFO_INT & 0x000000f0) >> 4 ;


    if(CurDMAIntStat & 0x00000001)//group 0
    {
        if(AVG_VOLUM > 240)
        {
        }
        else if(AVG_VOLUM > 200)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1125;
        }
        else if(AVG_VOLUM > 100)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1500;
        }
        else if(AVG_VOLUM >  50)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1750;
        }
        else
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1875;
        }
    }

 #elif( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    // test Group1 AGC function
    INT32U i, CurDMAIntStat, uiDMAChkBit=0x03;
    INT32U AVG_VOLUM = (ADCRX_AVG_VOLUMN & 0x000000ff);
    // printf("E = 0x%08x",ADCRX_AVG_FIFO_INT);
    CurDMAIntStat = (ADCRX_AVG_FIFO_INT & 0x000000f0) >> 4 ;


    if(CurDMAIntStat & 0x00000001)//group 0
    {
        if(AVG_VOLUM > 240)
        {
        }
        else if(AVG_VOLUM > 200)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1125;
        }
        else if(AVG_VOLUM > 100)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1500;
        }
        else if(AVG_VOLUM >  50)
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1750;
        }
        else
        {
            AdcRecConvRate &= 0xffe3ffff;
            AdcRecConvRate |= ADC_BUS_SFT_FRACTION_1875;
        }
    }

 #endif
}

/* SW 0108 S */
s32 adcKeyPolling1(void)
{
    s16          adc_key1,adc_key2;
    u8           tempKey;
    static u8    ucDisableCnt = 0;
    static u8    ucDayNightmode = 0; // 0 is day mode
    static u8    unGetValue[3]={0};

#if (HW_BOARD_OPTION == MR8120_TX_ZINWELL)
    adc_key1 = AdcRecData_G1G2 >> 16;
    adc_key2 = AdcRecData_G1G2 & 0xffff;
/*    if (UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY)
    {
//        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
//            ucDisableCnt = 0;
        }
		return 0;
    }*/
    printf("IR Led = %08x\n",adc_key1);
    printf("Thermal sensor = %08x\n",adc_key2);
/*    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }
    ucDisableCnt++;*/
//    if (adc_key1 < 100)
    {
        if ((adc_key1 > 0x500) && (ucDayNightmode == 1))
        {
//            tempKey=UI_KEY_RF_QUAD;
            DEBUG_ADC("\nIR led = %08x ,night to day\n",adc_key1);
            ucDayNightmode=0; // day mode
            gpioSetLevel(1, 6, 0);
        }
        if ((adc_key1 < 0x400) && (ucDayNightmode == 0))
        {
//            tempKey=UI_KEY_TVOUT_DET;
            DEBUG_ADC("\nIR led = %08x ,day to night\n",adc_key1);
            ucDayNightmode=1; // night mode
            gpioSetLevel(1, 6, 1);
        }
    }

#endif
    return 0;
}

s32 adcKeyPolling(void)
{

    s8           adc_key1,adc_key2,adc_key3;
    u8           tempKey;
    u32          adc_1;
    static u8    ucDisableCnt = 0, lastKey = 0, waitCnt = 13, lastKey1 = 0,Cnt = 0;
    static u8    unGetValue[3]={0};


#if (HW_BOARD_OPTION == MR8200_RX_DEMO_BOARD || HW_BOARD_OPTION == MR8200_RX_DB2)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key1 < 100)
    {
        if (adc_key1 > 50)
        {
            tempKey=UI_KEY_RF_QUAD;
            DEBUG_ADC("Key UI_KEY_RF_QUAD\n");
        }
        else if (adc_key1 > 0)
        {
            tempKey=UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET: ADC\n");
        }
        else if (adc_key1 > -50)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key1 > -100)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
    }

    else if (adc_key2 < 100)
    {
        if (adc_key2 > 50)
        {
            tempKey=UI_KEY_RF_CH;
            DEBUG_ADC("Key UI_KEY_RF_CH\n");
        }
        else if (adc_key2 > 0)
        {
            tempKey=UI_KEY_PLAY;
            DEBUG_ADC("Key UI_KEY_PLAY\n");
        }
        else if (adc_key2 > -50)
        {
            tempKey=UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP\n");
        }
        else if (adc_key2 > -100)
        {
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else
        {
            tempKey=UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_MENU\n");
        }
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(MsgKey == UI_KEY_WAIT_KEY)
    {
        MsgKey = tempKey;
        OSMboxPost(message_MboxEvt, "adc_key");
        DEBUG_ADC("OSMboxPost MsgKey %d\n",MsgKey);
    }
    else if (UIKey == UI_KEY_READY)
    {
        UIKey = tempKey;
        OSSemPost(uiSemEvt);
        DEBUG_ADC("OSMboxPost UIKey %d\n",UIKey);
    }
    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);
#elif (HW_BOARD_OPTION == MR8200_RX_DB3)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    adc_key3 = (AdcRecData_G1G2 & 0xff00000) >>20;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2,adc_key3);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)&&(adc_key3 >= 50)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100)&&(adc_key3 >= 50))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key1 < 100)
    {
        if (adc_key1 > 30)
        {
            tempKey = UI_KEY_CH4;
            DEBUG_ADC("Key UI_KEY_CH4\n");
        }
        else if (adc_key1 > 0)
        {
            tempKey = UI_KEY_CH3;
            DEBUG_ADC("Key UI_KEY_CH3\n");
        }
        else if (adc_key1 > -30)
        {
            tempKey = UI_KEY_CH2;
            DEBUG_ADC("Key UI_KEY_CH2\n");
        }
        else if (adc_key1 > -60)
        {
            tempKey = UI_KEY_CH1;
            DEBUG_ADC("Key UI_KEY_CH1\n");
        }
        else if (adc_key1 > -100)
        {
            tempKey = UI_KEY_RF_QUAD;
            DEBUG_ADC("Key UI_KEY_RF_QUAD\n");
        }
        else
        {
            tempKey = UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
    }

    else if (adc_key2 < 100)
    {
        if (adc_key2 > 0)
        {
            tempKey = UI_KEY_MODE;
            DEBUG_ADC("Key UI_KEY_MODE\n");
        }
        else if (adc_key2 > -30)
        {
            tempKey = UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP\n");
        }
        else if (adc_key2 > -60)
        {
            tempKey = UI_KEY_PLAY;
            DEBUG_ADC("Key UI_KEY_PLAY\n");
        }
        else if (adc_key2 > -100)
        {
            tempKey = UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else
        {
            tempKey = UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_MENU\n");
        }
    }
    else if (adc_key3 < 50)
    {
        if (adc_key3 > 0)
        {
            tempKey = UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key3 > -30)
        {
            tempKey = UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key3 > -60)
        {
            tempKey = UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key3 > -100)
        {
            tempKey = UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else
        {
            tempKey = UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key3 < 50)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    uiSentKeyToUi(tempKey);

#elif ((HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720)||(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710 )||\
      (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)||(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011))
      
    adc_key3= (AdcRecData_G3G4 & 0xff00000) >>20;  //ADC2
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;       //ADC1
    adc_key1 = (AdcRecData_G1G2 & 0xff00000) >>20;  //ADC0
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2,adc_key3);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key2 >= 100)&&(adc_key3 >= 0)))
    {
        if((adc_key2 >= 100)&&(adc_key3 >= 0))
        {
            ucDisableCnt = 0;
        }
  return 0;
    }

    else if (adc_key3< 0)
    {
        if (adc_key3 > -40)
        {
            tempKey = UI_KEY_TALK;
            DEBUG_ADC("Key UI_KEY_TALK\n");
        }

        else if (adc_key3 > -100)
        {
            tempKey = UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
        else
        {
            tempKey = UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
    }
    else if (adc_key2< 100)
    {
        if (adc_key2 > 80)
        {
            tempKey = UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > 20)
        {
            tempKey = UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > -40)
        {
            tempKey = UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key2 > -100)
        {
            tempKey = UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else
        {
            tempKey = UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
    }



    #if 1
 /*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 100)
    {
     unGetValue[ucDisableCnt] = tempKey; /* get the current value */
    }
    else if(adc_key3 < 0)
    {
     unGetValue[ucDisableCnt] = tempKey; /* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    uiSentKeyToUi(tempKey);



#elif (HW_BOARD_OPTION == MR8120_RX_SUNIN_820HD)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    adc_key3 = (AdcRecData_G1G2 & 0xff00000) >>20;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2,adc_key3);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }

       if (adc_key1< 100)
    {
        if (adc_key1 > 0)
        {
            tempKey = UI_KEY_DELETE;
            DEBUG_ADC("Key UI_KEY_DELETE\n");
        }
        else if (adc_key1 > -20)
        {
            tempKey = UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP\n");
        }
        else if (adc_key1 > -60)
        {
            tempKey = UI_KEY_PLAY;
            DEBUG_ADC("Key UI_KEY_PLAY\n");
        }
        else if (adc_key1 > -90)
        {
            tempKey = UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else
        {
            tempKey = UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_MENU\n");
        }
    }

      else if  (adc_key2 < 100)
    {
        if (adc_key2 > 10)
        {
            tempKey = UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > -20)
        {
            tempKey = UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > -60)
        {
            tempKey = UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey = UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }

        else
        {
            tempKey = UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
    }


    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }

    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    uiSentKeyToUi(tempKey);


#elif (HW_BOARD_OPTION == MR9670_DEMO_BOARD)
return; // close adc
	adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key1 < -50)
    {
    printf("adc_key1 = %d\n",adc_key1);
        if (adc_key1 > -80)
        {
//            tempKey=UI_KEY_TALK;
            DEBUG_ADC("Key UI_KEY_TALK\n");
        }
        else if (adc_key1 > -100)
        {
//            tempKey=UI_KEY_RING;
            DEBUG_ADC("Key UI_KEY_RING\n");
        }
        else if (adc_key1 > -115)
        {
//            tempKey=UI_KEY_UNLOCK;	// ring key
            DEBUG_ADC("Key UI_KEY_UNLOCK\n");
        }
        else
        {
//            tempKey=UI_KEY_MANAGER;		// talk key
            DEBUG_ADC("Key UI_KEY_MANAGER\n");
        }
    }

    else if (adc_key2 < 0)
    {
    printf("adc_key1 = %d\n",adc_key2);
        if (adc_key2 > -15)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key2 > -32)
        {
            tempKey=UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP\n");
        }
        else if (adc_key2 > -50)
        {
//            tempKey=UI_KEY_ESC;
            DEBUG_ADC("Key UI_KEY_ESC\n");
        }
		else if (adc_key2 > -65)
        {
            tempKey=UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_MENU\n");
        }
		else if (adc_key2 > -81)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else if (adc_key2 > -98)
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
		else if (adc_key2 > -115)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(MsgKey == UI_KEY_WAIT_KEY)
    {
        MsgKey = tempKey;
        OSMboxPost(message_MboxEvt, "adc_key");
        DEBUG_ADC("OSMboxPost MsgKey %d\n",MsgKey);
    }
    else if (UIKey == UI_KEY_READY)
    {
        UIKey = tempKey;
        OSSemPost(uiSemEvt);
        DEBUG_ADC("OSMboxPost UIKey %d\n",UIKey);
    }
    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);
#elif (HW_BOARD_OPTION == MR9670_WOAN)
//return; // close adc
	adc_key1 = (AdcRecData_G1G2 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf(" [MR9670_WOAN] adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }

	//printf(" [MR9670_WOAN] adc_key1 %d, adc_key2 %d\n",adc_key1,adc_key2);
    if (adc_key1 < 0)
    {
    //printf("adc_key1 = %d\n",adc_key1);
#if( VERSION_FOR_MOTION_DETECT == 1 )
        if (adc_key1 > -38)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT  (4) \n");
        }
        else if (adc_key1 > -67)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT  (5) \n");
        }
        else if (adc_key1 > -93)
        {
            tempKey=UI_KEY_MODE;
            DEBUG_ADC("Key UI_KEY_MODE (UI_KEY_ESC)  (6)\n");
        }
        else
        {
            tempKey=UI_KEY_PLAY;
            DEBUG_ADC("Key UI_KEY_PLAY (7)\n");
        }
#else
        if (adc_key1 > -40)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT  (4) \n");
        }
        else if (adc_key1 > -60)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT  (5) \n");
        }
        else if (adc_key1 > -90)
        {
            tempKey=UI_KEY_MODE;
            DEBUG_ADC("Key UI_KEY_MODE (UI_KEY_ESC)  (6)\n");
        }
        else
        {
            tempKey=UI_KEY_PLAY;
            DEBUG_ADC("Key UI_KEY_PLAY (7)\n");
        }
#endif
    }

    else if (adc_key2 < 0)
    {
    //printf("adc_key2 = %d\n",adc_key2);
#if( VERSION_FOR_MOTION_DETECT == 1 )
        if (adc_key2 > -29)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER (UI_KEY_MENU)(1) \n");
        }
        else if (adc_key2 > -40)
        {
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC (UI_KEY_REC) (2) \n");
        }
        else if (adc_key2 > -54)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP  (2) \n");
        }
        else if (adc_key2 > -74)
        {
            tempKey=UI_KEY_PWR_OFF;
            DEBUG_ADC("Key UI_KEY_PWR_OFF (UI_KEY_POWER) \n");
        }
        else if (adc_key2 > -92)
        {
            tempKey=UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP(UI_KEY_SEGMENTATION)  (8) \n");
        }
        else
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN  (3) \n");

        }
#else
        if (adc_key2 > -40)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER (UI_KEY_MENU)(1) \n");
        }
        else if (adc_key2 > -60)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP  (2) \n");
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_STOP;
            DEBUG_ADC("Key UI_KEY_STOP(UI_KEY_SEGMENTATION)  (8) \n");
        }
        else
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN  (3) \n");

        }
#endif
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(MsgKey == UI_KEY_WAIT_KEY)
    {
        MsgKey = tempKey;
        OSMboxPost(message_MboxEvt, "adc_key");
        DEBUG_ADC("OSMboxPost MsgKey %d\n",MsgKey);
    }
    else if (UIKey == UI_KEY_READY)
    {
        UIKey = tempKey;
        OSSemPost(uiSemEvt);
        //DEBUG_ADC("OSMboxPost UIKey %d\n",UIKey);
    }
    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);

#elif (HW_BOARD_OPTION == MR8200_RX_RDI)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 0))
    {
        if(adc_key2 >= 0)
        {
            ucDisableCnt = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -60)
        {
            tempKey = UI_KEY_B_UP;
            DEBUG_ADC("Key UI_KEY_B_UP\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_B_RIGHT;
            DEBUG_ADC("Key UI_KEY_B_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey=UI_KEY_B_LEFT;
            DEBUG_ADC("Key UI_KEY_B_LEFT\n");
        }
        else
        {
            tempKey=UI_KEY_B_DOWN;
            DEBUG_ADC("Key UI_KEY_B_DOWN\n");
        }
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif


    uiSentKeyToUi(tempKey);
#elif  ((HW_BOARD_OPTION == MR8200_RX_RDI_M900)||(HW_BOARD_OPTION == MR8200_RX_RDI_M902))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 0)&&(adc_key2 >= 120)))
    {
        if ((adc_key1 >= 0)&&(adc_key2 >= 120))
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -30)
        {
            tempKey = UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
        else if (adc_key2 > -60)
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_B_DOWN))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_B_DOWN;
                DEBUG_ADC("Key UI_KEY_B_DOWN\n");
            }        
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_B_RIGHT;
            DEBUG_ADC("Key UI_KEY_B_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey=UI_KEY_B_LEFT;
            DEBUG_ADC("Key UI_KEY_B_LEFT\n");
        }
        else
        {
            tempKey=UI_KEY_B_UP;
            DEBUG_ADC("Key UI_KEY_B_UP\n");
        }
    }
    else
    {
        if (adc_key2 < 15 )
        {
            tempKey = UI_KEY_B_OK;
            DEBUG_ADC("Key UI_KEY_B_OK\n");
        }    
    }


    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 0)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 15)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif


    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    uiSentKeyToUi(tempKey);
    lastKey = tempKey;
    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);

#elif  ((HW_BOARD_OPTION == MR8200_RX_RDI_M712)||(HW_BOARD_OPTION == MR8120_RX_RDI_M713)||(HW_BOARD_OPTION == MR8600_RX_RDI_M904D))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 120))
    {
        if (adc_key2 >= 120)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -30)
        {
            tempKey = UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
        else if (adc_key2 > -60)
        {
            tempKey = UI_KEY_B_DOWN;
            DEBUG_ADC("Key UI_KEY_B_DOWN\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_B_RIGHT;
            DEBUG_ADC("Key UI_KEY_B_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey=UI_KEY_B_LEFT;
            DEBUG_ADC("Key UI_KEY_B_LEFT\n");
        }
        else
        {
            tempKey=UI_KEY_B_UP;
            DEBUG_ADC("Key UI_KEY_B_UP\n");
        }
    }
    else
    {
        if (adc_key2 < 15 )
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_B_OK))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_B_OK;
                DEBUG_ADC("Key UI_KEY_B_OK\n");
            }
        }

    }


    #if 1
	/*==================Button Press De-Bouncing===================*/
    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif


    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    uiSentKeyToUi(tempKey);
    lastKey = tempKey;
    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);

#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) || (HW_BOARD_OPTION == MR8200_RX_RDI_M701)||(HW_BOARD_OPTION==MR8120_RX_RDI_M703) ||\
    (HW_BOARD_OPTION == MR8120_RX_RDI_M724))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 120))
    {
        if (adc_key2 >= 120)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -30)
        {
            tempKey = UI_KEY_OK;
            DEBUG_ADC("Key UI_KEY_OK\n");
        }
        else if (adc_key2 > -60)
        {
            tempKey=UI_KEY_DOWN;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_DOWN))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_DOWN;
            }
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_RIGHT;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_RIGHT))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_RIGHT;
            }
            DEBUG_ADC("Key UI_KEY_B_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey=UI_KEY_LEFT;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_LEFT))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_LEFT;
            }
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else
        {
            tempKey = UI_KEY_UP;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_UP))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_UP;
            }
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
    }


    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 0)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    lastKey = tempKey;
    uiSentKeyToUi(tempKey);
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M706)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 120))
    {
        if (adc_key2 >= 120)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -90)
        {
            tempKey=UI_KEY_PLAYBACK;
            DEBUG_ADC("Key UI_KEY_PLAYBACK\n");
        }
        else if (adc_key2 > -110)
        {
            if (((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_VOL_DOWN)) &&  (uiGetMenuMode() == SETUP_MODE))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_VOL_DOWN;
                DEBUG_ADC("Key UI_KEY_VOL_DOWN\n");
            }
        }
        else
        {
            tempKey = UI_KEY_VOL_UP;
            DEBUG_ADC("Key UI_KEY_VOL_UP\n");
        }
    }
   

    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 0)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    
    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    
#if DOOR_BELL_SUPPORT    
    if ((tempKey == UI_KEY_VOL_UP) && (lastKey == tempKey))
    {
        if(Cnt < 5)
        {
            DEBUG_GREEN("Cnt %d \n ",Cnt);
            Cnt++;
            return 0;
        }
        else
        {
            tempKey = UI_MENU_DOOR_BELL;
            Cnt = 0;
        }
    }
    else
        Cnt = 0;
#endif    
    lastKey = tempKey;
    uiSentKeyToUi(tempKey);
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M930)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 100))
    {
        if (adc_key2 >= 100)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -110)
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_VOL_DOWN))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_VOL_DOWN;
                DEBUG_ADC("Key UI_KEY_VOL_DOWN\n");
            }
        }
        else
        {
            tempKey = UI_KEY_VOL_UP;
            DEBUG_ADC("Key UI_KEY_VOL_UP\n");
        }
    }

    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 15)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    lastKey = tempKey;
    uiSentKeyToUi(tempKey);

#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M742) || (HW_BOARD_OPTION == MR8200_RX_RDI_M731)||\
    (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA) || (HW_BOARD_OPTION == MR8120_RX_RDI_M733))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 100))
    {
        if (adc_key2 >= 100)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -110)
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_VOL_DOWN))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_VOL_DOWN;
                DEBUG_ADC("Key UI_KEY_VOL_DOWN\n");
            }
        }
        else
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_VOL_UP))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_VOL_UP;
                DEBUG_ADC("Key UI_KEY_VOL_UP\n");
            }
        }
    }

    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 15)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    uiSentKeyToUi(tempKey);
    lastKey = tempKey;
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M742_HA)

    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 100))
    {
        if (adc_key2 >= 100)
        {
            if (lastKey == UI_KEY_TALK)
            {
#if 1           
                tempKey = UI_KEY_TALK_OFF;
                if (uiSentKeyToUi(tempKey) == 1)
                    lastKey = tempKey;
#else
                tempKey = UI_KEY_TALK_OFF;
                if (lastKey1 == UI_KEY_TALK_OFF)
                {
                    if (ucDisableCnt == 3) 
                    {
                        ucDisableCnt = 0;
                        lastKey1 = 0;
                    }
                }
                else
                {
                    ucDisableCnt = 0;
                    lastKey1 = UI_KEY_TALK_OFF;
                }
#endif
            }
            else
            {
                ucDisableCnt = 0;
                waitCnt = 13;
                lastKey = 0;
                return 0;
            }
        }
        else
		    return 0;
    }

    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -110)
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_VOL_UP))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_VOL_UP;
                DEBUG_ADC("Key UI_KEY_VOL_UP\n");
            }
        }
        else
        {
            tempKey = UI_KEY_TALK;
            DEBUG_ADC("Key UI_KEY_TALK\n");
        }
    }

    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 15)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    if (uiSentKeyToUi(tempKey) == 1)
        lastKey = tempKey;

#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M920)

    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 120))
    {
        if (adc_key2 >= 120)
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
		return 0;
    }

    if (adc_key2 < 0)
    {
        if (adc_key2 > -30)
        {
            tempKey = UI_KEY_OK;
            DEBUG_ADC("Key UI_KEY_OK\n");
        }
        else if (adc_key2 > -60)
        {
            tempKey=UI_KEY_DOWN;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_DOWN))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_DOWN;
            }
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey=UI_KEY_RIGHT;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_RIGHT))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_RIGHT;
            }
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey=UI_KEY_LEFT;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_LEFT))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_LEFT;
            }
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else
        {
            tempKey = UI_KEY_UP;
            if ((lastKey == tempKey) || (lastKey == UI_KEY_CONT_UP))
            {
                waitCnt = 4;
                tempKey = UI_KEY_CONT_UP;
            }

            DEBUG_ADC("Key UI_KEY_UP\n");
        }
    }
    else
    {
        if (adc_key2 <15 )
        {
            tempKey=UI_KEY_VOL_DOWN;
        DEBUG_ADC("Key UI_KEY_VOL_DOWN\n");
        }

    }


    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key2 < 15)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    lastKey = tempKey;
    uiSentKeyToUi(tempKey);
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_RX240)

    adc_key1= (AdcRecData_G1G2 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    adc_key3 = (AdcRecData_G3G4 & 0xff00000) >>20;

    //printf("adc DacKeyCtrl %08x  %08x  %d  %d %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2,adc_key3);

    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key2 >= 0)&&(adc_key3 >= 0)))
    {
        if((adc_key2 >= 0)&&(adc_key3 >= 0))
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
        return 0;
    }


    if (adc_key2 < 0)
    {
        if (adc_key2 > -60)
        {
            tempKey = UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > -90)
        {
            tempKey = UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > -110)
        {
            tempKey = UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else
        {
            tempKey = UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
    }
    else if (adc_key3 < 0)
    {
        if  (adc_key3 > -60)
        {
            tempKey = UI_KEY_MOTION;
            DEBUG_ADC("Key UI_KEY_MOTION\n");
        }
        else if (adc_key3 > -90)
        {
            tempKey = UI_KEY_QUAD;
            DEBUG_ADC("Key UI_KEY_QUAD\n");
        }
        else if (adc_key3 > -110)
        {
            tempKey = UI_KEY_OK;
            DEBUG_ADC("Key UI_KEY_OK\n");
        }
        else
        {
            if ((lastKey == UI_KEY_L_DEL) ||(lastKey == UI_KEY_DELETE))
            {
                waitCnt = 100;
                tempKey = UI_KEY_L_DEL;
                DEBUG_ADC("Key UI_KEY_L_DEL\n");
            }
            else
            {
                tempKey = UI_KEY_DELETE;
                DEBUG_ADC("Key UI_KEY_DELETE\n");
            }
        }
    }



    #if 1
 /*==================Button Press De-Bouncing===================*/
    if(adc_key2 < 0)
    {
     unGetValue[ucDisableCnt] = tempKey; /* get the current value */
    }
    else if(adc_key3 < 0)
    {
     unGetValue[ucDisableCnt] = tempKey; /* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif
    if ((tempKey == UI_KEY_L_DEL) && (lastKey != tempKey))
    {
        lastKey = tempKey;
        return 0;
    }
    uiSentKeyToUi(tempKey);
    lastKey = tempKey;

#elif (HW_BOARD_OPTION == MR8600_RX_GCT)
    u8           level;
    static u8    test_mode_count=0;
    static u8    timer_count=0;
    static u8    RF_TestMode =0;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key2<0)
    {
        ucDisableCnt++;
        level=0;
    }
    else if(adc_key2 > 0)
    {
        /* 500 ms */
        if((ucDisableCnt <20) && (ucDisableCnt > 0 ) )
        {
            if(test_mode_count == 0)
            {
                timer_count=0;
            }
            test_mode_count++;
        }
        ucDisableCnt=0;
        tempKey=0xff;
        level =1;
    }

    timer_count++;

    if(RF_TestMode == 1)
    {
        RF_TEST_MODE_CH2(level);
        return ;
    }

    //  press continued 5 times within 2 sec
    if((timer_count <80) &&(test_mode_count == 5) )
    {
        test_mode_count =0;
        RF_TestMode=1;
        ucDisableCnt=0;
        sysback_RF_SetEvt(SYS_BACKRF_FCC_DIRECT_TXRX, 0);


    }
    else if (timer_count >80)
    {
        test_mode_count=0;
    }



    if(ucDisableCnt == 80)  // 2s
    {
        tempKey=UI_KEY_RF_PAIR1;
    }


    if ((tempKey != 0xff) && (tempKey != 0) )
    {
        uiSentKeyToUi(tempKey);
    }
#elif (HW_BOARD_OPTION == MR8600_RX_JIT)
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key2<0)
    {
        ucDisableCnt++;
    }
    else if(adc_key2 > 0)
    {
        ucDisableCnt=0;
        tempKey=0xff;
    }

    if(ucDisableCnt == 80)  // 2s
    {
        tempKey=UI_KEY_RF_PAIR1;
    }


    if ((tempKey != 0xff) && (tempKey != 0) )
    {
        uiSentKeyToUi(tempKey);
    }
#elif (HW_BOARD_OPTION == MR8600_RX_TRANWO)
    adc_key1 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key1<0)
    {
        ucDisableCnt++;
    }
    else if(adc_key1 > 0)
    {
        ucDisableCnt=0;
        tempKey=0xff;
    }

    if(ucDisableCnt == 80)  // 2s
    {
        tempKey=UI_KEY_RF_PAIR1;
    }


    if ((tempKey != 0xff) && (tempKey != 0) )
    {
        uiSentKeyToUi(tempKey);
    }

#elif (HW_BOARD_OPTION == MR8600_RX_JESMAY)
    adc_key2 = (AdcRecData_G1G2 & 0xff00000) >>20;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key2<0)
    {
        ucDisableCnt++;
    }
    else if(adc_key2 > 0)
    {
        if((ucDisableCnt <20) && (ucDisableCnt > 0 ) )
        {
            tempKey=UI_KEY_AUDIO_CH2;
        }
        ucDisableCnt=0;
    }

    if(ucDisableCnt == 80)  // 2s
    {
        tempKey=UI_KEY_RF_PAIR1;
    }


    if ((tempKey != 0xff) && (tempKey != 0) )
    {
        uiSentKeyToUi(tempKey);
    }

#elif (HW_BOARD_OPTION == MR8600_RX_MAYON)
    adc_key1 = (AdcRecData_G1G2  & 0xff0)>>4;
   // printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key1<0)
    {
        ucDisableCnt++;
    }
    else if(adc_key1 > 0)
    {
        if((ucDisableCnt <20) && (ucDisableCnt > 0 ) )
        {
            tempKey=UI_KEY_AUDIO_CH2;
        }
        ucDisableCnt=0;
    }

    if(ucDisableCnt == 80)  // 2s
    {
        tempKey=UI_KEY_RF_PAIR1;
    }

    if ((tempKey != 0xff) && (tempKey != 0) )
    {
        uiSentKeyToUi(tempKey);
    }
    
#elif (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX)
    //adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || (adc_key2 >= 100))
    {
        if(adc_key2 >= 100)
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key2 < 100)
    {
        if (adc_key2 > 0)
        {
            tempKey=UI_KEY_ADC_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key2 > -50)
        {
            tempKey=UI_KEY_ADC_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else if (adc_key2 > -100)
        {
            tempKey=UI_KEY_ADC_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else
        {
            tempKey=UI_KEY_ADC_MENU;
            DEBUG_ADC("Key UI_KEY_MENU\n");
        }
    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(MsgKey == UI_KEY_WAIT_KEY)
    {
        MsgKey = tempKey;
        OSMboxPost(message_MboxEvt, "adc_key");
        DEBUG_ADC("OSMboxPost MsgKey %d\n",MsgKey);
    }
    else if (UIKey == UI_KEY_READY)
    {
        UIKey = tempKey;
        OSSemPost(uiSemEvt);
        DEBUG_ADC("OSMboxPost UIKey %d\n",UIKey);
    }
#elif (HW_BOARD_OPTION == MR8120_TX_Philio)
    u8 level;
    u8 pulse_time=250;   //每輸出250次Hight Level 大約 花0.5ms
    u32 i;
    static u8 Mode=SIU_DAY_MODE;    
    adc_key1 = (AdcRecData_G1G2  &  0x0000fff);
    
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    /*==================Prevent From Cont. Detecting===================*/
    if(adc_key1 < 0)
    {
        level=SIU_NIGHT_MODE;
    }
    else 
    {
        level=SIU_DAY_MODE;
    }

    if(Mode != level)
    {
        if(level == SIU_NIGHT_MODE)
        {
            gpioSetLevel(GPIO_GROUP_IR_LED, GPIO_BIT_IR_LED, GPIO_LEVEL_LO);
            Mode=SIU_NIGHT_MODE;  
        } 
        else
        {
            gpioSetLevel(GPIO_GROUP_IR_LED, GPIO_BIT_IR_LED, GPIO_LEVEL_HI);            
            Mode=SIU_DAY_MODE;    
        }    
    }

    
#elif ((HW_BOARD_OPTION == MR8200_RX_JIT) || (HW_BOARD_OPTION == MR8120_RX_JIT_LCD) ||\
    (HW_BOARD_OPTION  == MR8120_RX_JIT_M703SW4) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key1 < 100)
    {
        if (adc_key1 > 60)
        {
            tempKey=UI_KEY_RF_QUAD;
            DEBUG_ADC("Key UI_KEY_RF_QUAD\n");
        }
        else if (adc_key1 > 5)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }

    }

    else if (adc_key2 < 100)
    {
#if (HW_BOARD_OPTION == MR8200_RX_JIT)    
        if (adc_key2 > 80)
        {
            tempKey = UI_KEY_LCD_BL;
            DEBUG_ADC("Key UI_KEY_LCD_BL\n");
        }
        else if (adc_key2 > 60)
#else
        if (adc_key2 > 60)
#endif
        {
            tempKey=UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
        else if (adc_key2 > 10)
        {
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else if (adc_key2 > -40)
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > -80)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > -100)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else if (adc_key2 < -120)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }


    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(tempKey !=0)
    {
        uiSentKeyToUi(tempKey);
    }

#elif((HW_BOARD_OPTION == MR8200_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8120_RX_JIT_BOX) ||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3) || (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key2< 100)
    {
        if (adc_key2> 60)
        {
#if (HW_BOARD_OPTION == MR8200_RX_JIT_BOX)        
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
#else
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
#endif
        }
        else if (adc_key2> 5)
        {
            tempKey=UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
#if (HW_BOARD_OPTION == MR8200_RX_JIT_BOX)        
        else if (adc_key2> -28)
        {
            tempKey=UI_KEY_RF_QUAD;
            DEBUG_ADC("Key UI_KEY_QUAD\n");
        }
#endif
    }

    else if (adc_key1< 100)
    {
        if (adc_key1 > 60)
        {
#if (HW_BOARD_OPTION == MR8200_RX_JIT_BOX)        
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
#else
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
#endif
        }
        else if (adc_key1 > -40)
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key1 > -75)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key1 > -100)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else if (adc_key1 > -120)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }


    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(tempKey !=0)
    {
        uiSentKeyToUi(tempKey);
    }

#elif((HW_BOARD_OPTION == MR8120_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3))
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 100)&&(adc_key2 >= 100)))
    {
        if((adc_key1 >= 100)&&(adc_key2 >= 100))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }
    if (adc_key2< 100)
    {
        if (adc_key2> 60)
        {
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else if (adc_key2> 5)
        {
            tempKey=UI_KEY_MENU;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }

    }

    else if (adc_key1< 100)
    {
        if (adc_key1 > 60)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
        else if (adc_key1 > -40)
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key1 > -75)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key1 > -100)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else if (adc_key1 > -120)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }


    }



    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 100)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if(ucDisableCnt == 2)
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    if(tempKey !=0)
    {
        uiSentKeyToUi(tempKey);
    }


#elif (HW_BOARD_OPTION == MR8211_ZINWELL)
/*
    u16 *pTempTable;

    if(++ucDisableCnt < 40)
        return;
    ucDisableCnt    = 0;

    //Light   = (AdcRecData_G1G2 & 0xffff0000) >> 16;
    Light   = (AdcRecData_G1G2 >> 16) ^ 0x0800; // signed->unsigned
    //TEMP    = (AdcRecData_G1G2 & 0x0000ffff);
    TEMP    = (AdcRecData_G1G2 & 0x0000ffff) ^ 0x0800; // signed->unsigned
    //Humi    = (AdcRecData_G3G4 & 0xffff0000) >> 16;

    pTempTable  = &TempTable[1];
    for(TempC = -10; TempC <= 60; TempC++)
    {
        if(TEMP < *pTempTable++)
            break;
    }
    if(TempC > 60)
        TempC   = 60;

    //DEBUG_ADC("\nL = 0x%04x, T = 0x%04x, H = 0x%04x, GPIO0 = 0x%08x\n", Light, TEMP, Humi, Gpio0Level);
    DEBUG_ADC("\nL = 0x%04x, T = 0x%04x(%d Do C), H = 0x%04x, GPIO0 = 0x%08x\n", Light, TEMP, TempC, Humidity, Gpio0Level);
*/
#elif ((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) ||\
       (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) )
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);     /*Adapter Plug in-out*/

    if((Main_Init_Ready == 1) && (adc_1 > 0x7FF))
    {
        tempKey=UI_KEY_PWR_OFF;
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt=0;
    }
    if((tempKey !=0) &&(ucDisableCnt > 2) )
    {
        uiSentKeyToUi(tempKey);
        ucDisableCnt=0;
    }
    
#elif((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592))
    adc_key1 = (AdcRecData_G1G2  &  0x0000fff);


    //printf("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

    if(adc_key1 == 0)
    {
        tempKey=UI_KEY_PWR_OFF;
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt=0;
    }


    if((tempKey !=0) &&(ucDisableCnt > 5) )
    {

        if (Main_Init_Ready == 0)
        {
            gpioSetLevel(GPIO_GROUP_POWER_OFF, GPIO_BIT_POWER_OFF,0) ;        
        }
        else
        {
            uiSentKeyToUi(tempKey);
            ucDisableCnt=0;    
        }

    }
#elif (HW_BOARD_OPTION == MR8120_RX_HECHI)
    static u8	ucPressStatFormat = STAT_NONE_PRESS_FORMAT;
    static u8	ucDelay = 15;	/* used for delaying the next press */
    static u8	ucStillPressFormat = 0;

	adc_key1 = (AdcRecData_G1G2 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G1G2  & 0xff0)>>4;
//	DEBUG_ADC("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);
//	DEBUG_ADC(" %08x  %d  %d\n",AdcRecData_G1G2,adc_key1,adc_key2);


    /*==================Prevent From Cont. Detecting===================*/
//	DEBUG_ADC("--->>>> ucDisableCnt =%d \n",ucDisableCnt);
    if (ucDisableCnt >= 5)
    {
        if(ucDisableCnt > 13)
            ucDisableCnt = 0;
        else
        {
            ucDisableCnt++;
        }
        return 0;
    }

    if (adc_key1 >= 50)
	{	/* button released */
		if (ucPressStatFormat == STAT_SINGLE_PRESS_FORMAT)
		{
            tempKey=UI_KEY_MONITOR;
            DEBUG_ADC("Key: UI_KEY_MONITOR\n");
    		timer_cnt = 120;
            uiSentKeyToUi(tempKey);
		}
		ucPressStatFormat = STAT_NONE_PRESS_FORMAT;
		ucDelay = 60;
        ucStillPressFormat = 0;
	}
	if (adc_key1 > -50 && adc_key1 < -25)
    {       /* Menu, Monitor Pressed */
        if (ucPressStatFormat == STAT_NONE_PRESS_FORMAT)
        {
            ucPressStatFormat = STAT_SINGLE_PRESS_FORMAT;
            return 0;
        }
        else if ((ucPressStatFormat == STAT_SINGLE_PRESS_FORMAT) ||(ucPressStatFormat == STAT_CONT_PRESS_FORMAT))
        {       /* Continue Press */
            if (ucStillPressFormat > ucDelay)
            {
                ucStillPressFormat = 0;		/* reset the counter */
                ucPressStatFormat = STAT_CONT_PRESS_FORMAT;   /* Set the stat to be cont. press */
                tempKey=UI_KEY_LONG_MONITOR;
                uiSentKeyToUi(tempKey);
                DEBUG_ADC("Key: UI_KEY_LONG_MONITOR\n");
				ucDelay = 15;
            }
            else
                ucStillPressFormat ++;
            DEBUG_ADC("%d, %d \n",ucDelay,ucStillPressFormat);
            return 0;
        }
    }

    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 50)&&(adc_key2 >= 50)))
    {
        if((adc_key1 >= 50)&&(adc_key2 >= 50))
        {
            ucDisableCnt = 0;
        }
		return 0;
    }


    if (adc_key1 <50)
    {
        DEBUG_ADC("adc_key1 = %d\n",adc_key1);
        if (adc_key1 < -100)
        {
            tempKey=UI_KEY_TALK;
            DEBUG_ADC("UI_KEY_TALK \n");
        }
        else if (adc_key1 > -100 && adc_key1 < -75)
        {
            tempKey=UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC \n");
        }
        else if (adc_key1 > -75 && adc_key1 < -50)
        {
            tempKey=UI_KEY_UNLOCK;
            DEBUG_ADC("Key UI_KEY_Unlock \n");
        }
        else if (adc_key1 > -50 && adc_key1 < -25)
        {
            tempKey=UI_KEY_MONITOR;
            DEBUG_TIMER("Key UI_KEY_MONITOR \n");
        }
		timer_cnt = 90;
    }

    else if (adc_key2 <50)
    {
    DEBUG_ADC("adc_key2 = %d\n",adc_key2);
       if (adc_key2 < -100)
        {
            tempKey=UI_KEY_MENU;

            DEBUG_ADC("Key UI_KEY_Return\n");
        }
        else if (adc_key2 > -100 && adc_key2 < -75)
        {
            tempKey=UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_OK\n");
        }
        else if (adc_key2 > -75 && adc_key2 < -50)
        {
            tempKey=UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
		else if (adc_key2 > -50 && adc_key2 < -25)
        {
            tempKey=UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
		else if (adc_key2 > -25 && adc_key2 < 25)
        {
            tempKey=UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else if (adc_key2 > 25 && adc_key2 < 50)
        {
            tempKey=UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
		timer_cnt=90;
    }
    #if 1
	/*==================Button Press De-Bouncing===================*/
    if(adc_key1 < 50)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    else if(adc_key2 < 50)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }
    if((ucDisableCnt == 2))
    {
        if((unGetValue[0] != unGetValue[1]) || (unGetValue[1] != unGetValue[2]))
        {
            ucDisableCnt = 0;
            return 0;
        }
        ucDisableCnt++;
    }
    else
    {
        ucDisableCnt++;
        return 0;
    }
    #endif

    uiSentKeyToUi(tempKey);

    //printf("adc DacKeyCtrl %08x  %08x  %x  %x\n",DacKeyCtrl,AdcRecData_G1G2,adc_key1,adc_key2);

#endif
}

u8 adcGetConvert2TemperatureC(u32 adcTemper)
{
    u8 temperC = 0;

#if (HW_BOARD_OPTION == MR8100_GCT_VM9710 && UI_PROJ_OPT == 1)
    if (adcTemper > 3052)
        temperC = 4;
    else if (adcTemper > 2707)
    {
        if (adcTemper > 2884)
            temperC = 5;
        else if (adcTemper > 2842)
            temperC = 6;
        else if (adcTemper > 2798)
            temperC = 7;
        else if (adcTemper > 2755)
            temperC = 8;
        else
            temperC = 9;
    }
    else if (adcTemper > 2489)
    {
        if (adcTemper > 2661)
            temperC = 10;
        else if (adcTemper > 2616)
            temperC = 11;
        else if (adcTemper > 2575)
            temperC = 12;
        else if (adcTemper > 2525)
            temperC = 13;
        else
            temperC = 14;
    }
    else if (adcTemper > 2265)
    {
        if (adcTemper > 2430)
            temperC = 15;
        else if (adcTemper > 2388)
            temperC = 16;
        else if (adcTemper > 2342)
            temperC = 17;
        else if (adcTemper > 2297)
            temperC = 18;
        else
            temperC = 19;
    }
    else if (adcTemper > 2018)
    {
        if (adcTemper > 2198)
            temperC = 20;
        else if (adcTemper > 2159)
            temperC = 21;
        else if (adcTemper > 2110)
            temperC = 22;
        else if (adcTemper > 2065)
            temperC = 23;
        else
            temperC = 24;
    }
    else if (adcTemper > 1799)
    {
        if (adcTemper > 1974)
            temperC = 25;
        else if (adcTemper > 1930)
            temperC = 26;
        else if (adcTemper > 1884)
            temperC = 27;
        else if (adcTemper > 1839)
            temperC = 28;
        else
            temperC = 29;
    }
    else if (adcTemper > 1586)
    {
        if (adcTemper > 1754)
            temperC = 30;
        else if (adcTemper > 1714)
            temperC = 31;
        else if (adcTemper > 1670)
            temperC = 32;
        else if (adcTemper > 1635)
            temperC = 33;
        else
            temperC = 34;
    }
    else if (adcTemper == 1586)
        temperC = 35;
    else
        temperC = 36;

#endif
    
    return temperC;
}

void adcTemperaturePolling(void)
{

#if (HW_BOARD_OPTION == MR8100_GCT_VM9710 && UI_PROJ_OPT == 1)
    static u32 adcavg = 0;
    static u8 cnt = 0;
    u8 temperC = 0;

    if (cnt == 8)
    {
        adcavg = adcavg >> 3;
        temperC = adcGetConvert2TemperatureC(adcavg);
        DEBUG_GREEN("Temperature %d adc %d\n", temperC, adcavg);
        adcavg = 0;
        cnt = 0;
        rfiuPutTXTem(temperC, 0);
    }
    else
    {
        adcavg += adcGetValue(2);
        cnt++;
    }

#endif

}

/*
Routine Description:

    Set DAC output gain.

Arguments:

    unAudioVol - Audio volume to set (0~31), 0: Max, 31: Mute.

Return Value:

    None.
*/
void adcSetDAC_OutputGain(u32 unAudioVol)
{
    // A1016
    if (unAudioVol > 31)
    {
        DEBUG_ADC("adcSetDAC_OutputGain(%d) Fail !!!!!!!\n",unAudioVol);
        return;
    }
#if(CHIP_OPTION == CHIP_A1018A) // walk around A1018A'bug
    gAudioVol=unAudioVol=16;
#else
    gAudioVol=unAudioVol;
#endif
    if (unAudioVol == 31)
    {
        DacTxCtrl |= DAC_MUTE_ENA;
    }
    else
    {
        DacTxCtrl  |= DAC_PWON;
        DacTxCtrl   = (DacTxCtrl & ~DAC_MUTE_ENA) | (unAudioVol << 18) | (unAudioVol << 23);
    }
}

/*
Routine Description:

    Use ADC to measure bettery capacity in ADC channel 1

Arguments:

    unAudioVol - Audio volume to set.

Return Value:

    None.
*/
void adcInitDAC_Play(u32 unAudioVol)
{
    #if ((HW_BOARD_OPTION == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
        (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if (unAudioVol == 31)
    {
        DacTxCtrl  &= ~DAC_PWON;
        DacTxCtrl  |= DAC_ENVREF_ON | DAC_MUTE_DISA;
    }
    else
        DacTxCtrl  |= DAC_ENVREF_ON | DAC_PWON | DAC_MUTE_DISA;
    
    #else
        DacTxCtrl  |= DAC_ENVREF_ON | DAC_PWON | DAC_MUTE_DISA;
    #endif
    
    adcSetDAC_OutputGain(unAudioVol);
}

/*
Routine Description:

    Set ADC MICIN gain of P.G.A.

Arguments:

    unAudioVol - Audio volume to set (0~31), 0: Max, 31: Mute.

Return Value:

    None.
*/

void adcSetADC_MICIN_PGA_Gain(u32 unAudioVol)
{
    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
         (HW_BOARD_OPTION  == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if ((unAudioVol > 31) || (timerDisableMic != 0))
    #else
    if ((unAudioVol > 31))
    #endif
    {
        DacTxCtrl   = (DacTxCtrl & ~DAC_PGAG_MUTE) | (0x1f << 8);
    }
    else
        DacTxCtrl   = (DacTxCtrl & ~DAC_PGAG_MUTE) | (unAudioVol << 8);
}

/*
Routine Description:

    Set DAC gain of L-ch

Arguments:

    unAudioVol - Audio volume to set (0~31), 0: Max, 31: Mute.

Return Value:

    None.
*/
void adcSetDAC_L_CH_Gain(u32 unAudioVol)
{
    if (unAudioVol > 31)
    {
        DacTxCtrl = DacTxCtrl | DAC_DALG_MUTE;
    }
    else
        DacTxCtrl   = (DacTxCtrl & ~DAC_DALG_MUTE) | (unAudioVol << 23);
}


/*
Routine Description:

    Set DAC gain of R-ch

Arguments:

    unAudioVol - Audio volume to set (0~31), 0: Max, 31: Mute.

Return Value:

    None.
*/
void adcSetDAC_R_CH_Gain(u32 unAudioVol)
{
    if (unAudioVol > 31)
    {
        DacTxCtrl = DacTxCtrl | DAC_DARG_MUTE;
    }
    else
        DacTxCtrl   = (DacTxCtrl & ~DAC_DARG_MUTE) | (unAudioVol << 18);
}

#if(NIC_SUPPORT)
extern void NicON(void);
extern void NicOff(void);
enum
{
    DOCK_OUT=0,
    DOCK_IN,
    DOCK_NONE,
};


#endif



u32 adcGetValue(u8 adc_num)
{

    u32 value =0;

    switch(adc_num)
    {
        case ADC_IN_0:
            value = (AdcRecData_G1G2  &  0xfff0000)>>16;
            break;

        case ADC_IN_1:
            value = (AdcRecData_G1G2  &  0x0000fff);
            break;

        case ADC_IN_2:
            value = (AdcRecData_G3G4  &  0xfff0000)>>16;
            break;

        case ADC_IN_3:
            value = (AdcRecData_G3G4  &  0x0000fff);
            break;

        default:
            DEBUG_ADC("Error ADC Number \n");
            break;
    }


    return ( (value+0x800)&0xfff );


}

u32 adcGetValueAverage(u8 adc_num, u16 average_num)
{

    u32 value =0;
    u32 i,j;
    u32 temp;
    u32 average=0;
    u8  drop_num=3;
    if(average_num < 11)
        drop_num = 1;
    switch(adc_num)
    {
        case ADC_IN_0:
            value = (AdcRecData_G1G2  &  0xfff0000)>>16;
            break;

        case ADC_IN_1:
            value = (AdcRecData_G1G2  &  0x0000fff);
            break;

        case ADC_IN_2:
            value = (AdcRecData_G3G4  &  0xfff0000)>>16;
            break;

        case ADC_IN_3:
            value = (AdcRecData_G3G4  &  0x0000fff);
            break;

        default:
            DEBUG_ADC("Error ADC Number \n");
            break;
    }
    ADCnumber[ADCcnt]=value;
    if(ADCcnt==(average_num-1))
    {
        for( i = 0; i < average_num; i++) 
        {
            for( j = i; j < average_num; j++) 
            {
                if( ADCnumber[j] < ADCnumber[i] ) 
                {
                    temp = ADCnumber[j];
                    ADCnumber[j] = ADCnumber[i];
                    ADCnumber[i] = temp;
                }
            }
        }
//        for( i = 0; i < average_num; i++ )
//        {
//            DEBUG_ADC("%d ", ADCnumber[i]);        
//        }
        for( i = drop_num; i < (average_num-drop_num); i++) 
        {
            average +=ADCnumber[i];
        }
        value = average/(average_num-drop_num*2);
    }
    if(ADCcnt==(average_num-1))
    {
        ADCcnt=0;
//        DEBUG_ADC("\ADC %d = %d \n",adc_num,(value+0x800)&0xfff);
        return ( (value+0x800)&0xfff );
    }
    else
    {
        ADCcnt ++;
        return 0;
    }


}

void adcSetDAC_Power(u8 Power_control)
{
    if(Power_control)
        DacTxCtrl  |= DAC_PWON;
    else
        DacTxCtrl  &= ~DAC_PWON;
}
