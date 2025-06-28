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


#if (UI_VERSION== UI_VERSION_TRANWO)
    #include "..\..\ui\inc\uiact_project.h"
#endif

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


void adcSetDAC_OutputGain(u32 unAudioVol);

 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

u32 ADCnumber[30];
u16 ADCcnt=0;

u32 gAudioVol=31;

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
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59};
        #endif
	#elif(ADC_SUBBOARD == ADC_SUBBOARD_Veri)
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
        u32 ADC_DIV_A[20]     = {ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    ,
                                 ADC_CLK_DIV_A_254    ,ADC_CLK_DIV_A_4    ,ADC_CLK_DIV_A_4     ,ADC_CLK_DIV_A_128   ,ADC_CLK_DIV_A_24    };
        u32 ADC_DIV_B[20]     = {ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     ,
                                 ADC_CLK_DIV_B_82    ,ADC_CLK_DIV_B_44    ,ADC_CLK_DIV_B_7     ,ADC_CLK_DIV_B_9     ,ADC_CLK_DIV_B_7     };
        u32 ADC_Conv_RATE[20] = {ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59,
                                 ADC_REC_CONV_RATE_26,ADC_REC_CONV_RATE_19,ADC_REC_CONV_RATE_74,ADC_REC_CONV_RATE_53,ADC_REC_CONV_RATE_59};
        #endif		
	#endif
    u32 ADC_RX_FMT[20] = {ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,
                          ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_8bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit,ADC_RX_16bit};

    #if(ADC_SUBBOARD == ADC_SUBBOARD_Fuji)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,
                               ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED};
    #elif(ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,
                               ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS};
	#elif(ADC_SUBBOARD == ADC_SUBBOARD_Veri)
    u32 ADC_RX_FMT_SIGN[20] = {ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,
                               ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_BYPASS,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED,ADC_RX_INVERTED};    
    #endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8   zoomflag;
extern u8   Lwipredhcp;
extern u32 IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) ||\
     (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) ||\
     (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
extern u8 uiCurAlertStatus;
extern u8 uiFakeShutdown;
extern u8 ADC_Init_Ready ;
#endif
#if ((HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613))
extern int rfiuBatCamBattLev;
extern int rfiuBatCamDcDetect;
#endif
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */

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

    if(IISMode == 1)
        Audio_formate = Audio_formate_Out;
    else
        Audio_formate = Audio_formate_In;
    
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
  #elif(ADC_SUBBOARD == ADC_SUBBOARD_Veri)
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
  //  AdcCtrlReg |= (ADC_REC_G1 | ADC_REC_G1_CH2); //ADCIN 2
	#endif
        
    #if AUDIO_IN_OUT_SELFTEST
    adcSetDAC_OutputGain(0);
    #else
    adcSetDAC_OutputGain(gAudioVol);
    #endif

    return 1;
}

int adc8BitPCMFmtSel(int OutFmt)
{
    //printf("adc8BitPCMFmtSel=%d\n",OutFmt);
    if(OutFmt == PCM_8BITFMT_UNSIGN)
    {
       AdcCtrlReg &= (~ADC_RX_INVERTED);
       rfiu_TX8BitPCMFmt=PCM_8BITFMT_UNSIGN;
    }   
    else
    {
       AdcCtrlReg |= ADC_RX_INVERTED;
       rfiu_TX8BitPCMFmt=PCM_8BITFMT_SIGN;
    }
        
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
    u32 adc_0 ,adc_1;//, adc_3;
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO) || (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) ||\
     (HW_BOARD_OPTION == MR9200_RX_RDI_M906) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
     (HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM) ||\
     (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H))
	u32 adc_2, adc_3;
#endif
    u8  cur_status=0;
#if((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777)||(HW_BOARD_OPTION == MR9200_RX_RDI_M906) || (HW_BOARD_OPTION == MR9200_RX_RDI_M1000))
    u8 cur_status_dc;
    u8 level=0xff;
#endif
#if(HW_BOARD_OPTION == MR9200_RX_RDI_M906)
    RTC_DATE_TIME   localTime;
    static u8 dc_detect=0xff;
#endif
    static u32 adc_value;
    static u8 lastLV = UI_BATTERY_CLEAR, cnt=0;//, dc_detect=0xff;
    static u8 count_osd=0;
    static u8 detect_cnt=10;
#if(HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903)
    u32 adc_2,adc_3;
    u8 level=0xff;
    u8 cur_status_dc;
#endif
    

#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
     (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H))

    if(ADC_Init_Ready  == 0 )
        return 0;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    adc_3 = (AdcRecData_G3G4  &  0x0000fff);
    //printf(" adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2, adc_3);
    //====================================================
    cur_status = UI_BATTERY_NONE;
    //if(((adc_2>0x110)&&(adc_2  < 0x1c0 ))||((adc_2>0x430)&&(adc_2  < 0x4c0 )))//Adaptor out
    //if(((adc_2>0x500)&&(adc_2  < 0x5C0 ))||((adc_2>0x7f0)&&(adc_2  < 0x900 )))//Adaptor out
    if(adc_2 > 0xc00)//tranwo //Adaptor out 0xfxx vs 0x7xx
    {
        if(cnt == 4)
        {
            adc_value = adc_value >> 2;

            DEBUG_ADC("ADC value %x \n", adc_value);
            if( adc_value > 0x483)
            {
                cur_status = UI_BATTERY_LV4; //3 
            }
            else if(adc_value > 0x461)
            {
                cur_status = UI_BATTERY_LV3; //2 3.539
            }
            else if(adc_value > 0x445)
            {
                cur_status = UI_BATTERY_LV2; //1 3.359
            }
            else if(adc_value >= 0x425) //empty
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
                //sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
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
            if(adc_0 < 0x3ff)
            {
                if(uiFakeShutdown == 0)
                    uiFakeShutdown = 1;
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
                    gpioTimerCtrLed(LED_GREEN_OFF);
                    gpioTimerCtrLed(LED_RED_FLASH);
                    count_osd++;
                    if((count_osd % 2) == 0)
                    {
                        //DEBUG_ADC("******* DRAW_BATTERY,Adaptor in******\n");
                        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                        count_osd = 0;
                    }
                }
                else
                {
                    gpioTimerCtrLed(LED_GREEN_ON);
                    gpioTimerCtrLed(LED_RED_OFF);
                    //DEBUG_ADC("******* DRAW_BATTERY ,Adaptor out*******\n");
                    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                }
            }
        }
        else if(uiFakeShutdown == 1)
        {
            sysDeadLockMonitor_OFF();
            sysWDT_disable();
            DEBUG_ADC("******* Shutdown *******\n");
            sysFakeShutodown();
            DEBUG_ADC("******* Shutdown End *******\n");
            uiFakeShutdown = 2;
            // detect_cnt=10;
            gpioTimerCtrLed(LED_GREEN_OFF);
            gpioTimerCtrLed(LED_RED_OFF);
        }
        else if(uiFakeShutdown == 2)
        {
            gpioTimerCtrLed(LED_GREEN_OFF);
            gpioTimerCtrLed(LED_RED_FLASH);

        }

    }
    else //Adaptor in //charger oN
    {
        if(uiFakeShutdown == 2)
        {
            //DEBUG_ADC("******* Reboot? *******\n");
            //detect_cnt=10;
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
            if(adc_3 < 0x100) //charger full 0xfxx vs 0x0xx
            {
                cur_status=UI_BATTERY_LV4;
                gpioTimerCtrLed(LED_GREEN_ON);
                gpioTimerCtrLed(LED_RED_OFF);
                batteryflag=cur_status;
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                //DEBUG_ADC("******* DRAW_BATTERY,BATTERY Full******\n");
            }
            else
            {
                cur_status = UI_BATTERY_CHARGE;
                gpioTimerCtrLed(LED_GREEN_ON);
                gpioTimerCtrLed(LED_RED_ON);
                batteryflag = cur_status;

                //DEBUG_ADC("******* DRAW_BATTERY,Adaptor in******\n");
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

            }
        }
    }

//=============================================================

#elif (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
    //if(ADC_Init_Ready  == 0 )
    //    return 0;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    adc_3 = (AdcRecData_G3G4  &  0x0000fff);
    //printf(" adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2, adc_3);
    //====================================================
    cur_status = UI_BATTERY_NONE;

    if(!sysDCPowerDetect())//tranwo //Adaptor out 0xfxx vs 0x7xx
    {
        if(cnt == 4)
        {
            adc_value = adc_value >> 2;

            //DEBUG_ADC("ADC value %x \n", adc_value);
            if( adc_value > 0x9a0)
            {
                cur_status = UI_BATTERY_LV4;
            }
            else if(adc_value > 0x970)
            {
                cur_status = UI_BATTERY_LV3;
            }
            else if(adc_value > 0x940)
            {
                cur_status = UI_BATTERY_LV2;
            }
            else if(adc_value >= 0x910)
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
                //sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                rfiuBatCamBattLev = batteryflag;
            }
        }
        else if(cnt < 4)
        {
            cnt++;
            adc_value += adc_2;
        }

    }
    else //Adaptor in //charger oN
    {
        cur_status = UI_BATTERY_CHARGE;
        batteryflag = cur_status;
        rfiuBatCamBattLev = batteryflag;
    }

 #elif (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM)

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    adc_3 = (AdcRecData_G3G4  &  0x0000fff);
    //printf(" adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2, adc_3);
    //====================================================
    cur_status = UI_BATTERY_NONE;

    if(!sysDCPowerDetect())//tranwo //Adaptor out 0xfxx vs 0x7xx
    {
        if(cnt == 4)
        {
            adc_value = adc_value >> 2;

            //DEBUG_ADC("ADC value %x \n", adc_value);
            if( adc_value > 0xa00)
            {
                cur_status = UI_BATTERY_LV4;
            }
            else if(adc_value > 0x98a)
            {
                cur_status = UI_BATTERY_LV3;
            }
            else if(adc_value > 0x943)
            {
                cur_status = UI_BATTERY_LV2;
            }
            else if(adc_value >= 0x91b)
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
                //sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                rfiuBatCamBattLev = batteryflag;
            }
        }
        else if(cnt < 4)
        {
            cnt++;
            adc_value += adc_2;
        }

    }
    else //Adaptor in //charger oN
    {
        cur_status = UI_BATTERY_CHARGE;
        batteryflag = cur_status;
        rfiuBatCamBattLev = batteryflag;
    }
#elif ((HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613))

    if(Main_Init_Ready  == 0 )
        return 0;

    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    adc_3 = (AdcRecData_G3G4  &  0x0000fff);
    //printf(" adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2, adc_3);
    //====================================================
    cur_status = UI_BATTERY_NONE;

    #if(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
        return 0;
    #endif

    if(rfiuBatCamDcDetect != sysDCPowerDetect())
    {
    #if 1
        sysSetEvt(SYS_EVT_TXPOWERSWITCH,0);
    #else
        sysSetEvt(SYS_EVT_FORCEREBOOT,0);
        DEBUG_ADC("==Cam Power Change,Reboot!==\n\r");
    #endif
    }

    if(!rfiuBatCamDcDetect)//tranwo //Adaptor out 0xfxx vs 0x7xx
    {
        if(cnt == 2)
        {
            adc_value = adc_value >> 1;

            //DEBUG_ADC("ADC value %x \n", adc_value);
            if(adc_value > 0x1de)
            {
                cur_status = UI_BATTERY_LV3;
            }
            else if(adc_value > 0x1c7)
            {
                cur_status = UI_BATTERY_LV2;
            }
            else if(adc_value > 0x1b2)
            {
                cur_status = UI_BATTERY_LV1; 
            }
            else
            {
                cur_status = UI_BATTERY_LV0; 
            }
            
            cnt = 0;
            adc_value = 0;
        
            if((cur_status != batteryflag) && (cur_status < batteryflag) && (cur_status!= UI_BATTERY_NONE))
            {
                batteryflag = cur_status;
                //sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                rfiuBatCamBattLev = batteryflag;
            }
        }
        else if(cnt < 2)
        {
            cnt++;
            adc_value += adc_0;
        }

    }
    else //Adaptor in //charger oN
    {
        cur_status = UI_BATTERY_CHARGE;
        batteryflag = cur_status;
        rfiuBatCamBattLev = batteryflag;
    }

#endif



#if(HW_BOARD_OPTION == MR9200_RX_TRANWO)
    
    if(Main_Init_Ready  == 0 )
        return 0;
	
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
        if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV) && (cur_status < UI_BATTERY_LV5))
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
            //DEBUG_ADC("******* Fake Shutdown *******\n");
            
            
            if(uiFakeShutdown ==0)
            {
                DEBUG_ADC("******* Fake Shutdown *******\n");
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
#elif (HW_BOARD_OPTION == MR9200_RX_RDI_UNIDEN)

    if(Main_Init_Ready  == 0 )
        return 0;
	
    //RTC_Get_Time(&localTime);
    adc_0 = (AdcRecData_G1G2  &  0xfff0000)>>16;
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    //adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //adc_3 = (AdcRecData_G3G4  &  0x0000fff);

    //printf("adc0:%03x adc1:%03x adc2:%03x adc3:%03x\n", adc_0, adc_1, adc_2, adc_3);
    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

#elif ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) || (HW_BOARD_OPTION == MR9200_RX_RDI_M1000))

    if(Main_Init_Ready  == 0 )
        return 0;
	
    #if (UI_USE_DEMO_UI == 1)
        return;
    #endif
	
    //RTC_Get_Time(&localTime);
    //adc_2  = (AdcRecData_G1G2  &  0xfff0000)>>16; //HW is altered, adc has huge difference
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_0 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //adc_3 = (AdcRecData_G3G4  &  0x0000fff);

    //DEBUG_LIGHTRED("adc0:%03x adc1:%03x \n", adc_0, adc_1);
    //DEBUG_LIGHTYELLOW("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if(adc_0 < 0x600) //  adapter plug out //0x0__~0x1__ no adapter; 0xa__~0xb__ with adapter
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //DEBUG_ADC("*** BAT LEVEL: %d  Curst %d ,,,Last %d....  ##%02d:%02d:%02d\n",level,cur_status,lastLV,localTime.hour, localTime.min, localTime.sec);

    if((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1)) // charging
    {
        cur_status = UI_BATTERY_CHARGE;
        adc_value = 0;
        if(count_osd < 5) //200ms a time, 5 for 1 sec
        {
            count_osd++;
            return 0;
        }
    }
    else if (cur_status_dc == 1)   // no battery but DC, display full battery icon, //adc0=0x500, 
    {
        cur_status = UI_BATTERY_LV3;
        lastLV = cur_status;
        batteryflag = cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        adc_value = 0;
        count_osd = 0;
        cnt = 0;
        return 0;
    }
    else
    {
        if (cnt == 4)
        {
            adc_value = adc_value>>2;
            if (adc_value > 0x8e0)
                cur_status = UI_BATTERY_LV3;
            else if (adc_value > 0x8a0)
                cur_status = UI_BATTERY_LV2;
            else if (adc_value > 0x870)
                cur_status = UI_BATTERY_LV1;
            else if (adc_value > 0x837)
                cur_status = UI_BATTERY_LV0;
            else
                cur_status = UI_BATTERY_SHUTDOWN;

            adc_value = 0;
        }
        else
        {
            adc_value += adc_1;
            cnt++;
            return 0;
        }
    }
    
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        if((cnt >= 4) || (count_osd >= 5))
        {
            if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV) && (cur_status <= UI_BATTERY_LV3)) //avoid battery level raise up by accident
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                cnt = 0;
                return 0;
            }

            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;
            count_osd = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                printf("Warning!!! Battery is dead. Send power off to ui\n");
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
    }
    else
    {
        cnt=0;
        count_osd = 0;
    }
    
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#elif (HW_BOARD_OPTION == MR9200_RX_RDI_M906)

    if(Main_Init_Ready  == 0 )
        return 0;
	
    #if (UI_USE_DEMO_UI == 1)
        return;
    #endif
	
    RTC_Get_Time(&localTime);
    //adc_2  = (AdcRecData_G1G2  &  0xfff0000)>>16; //HW is altered, adc has huge difference
    adc_1 = (AdcRecData_G1G2  &  0x0000fff);
    adc_0 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    //adc_3 = (AdcRecData_G3G4  &  0x0000fff);

    //DEBUG_LIGHTRED("adc0:%03x adc1:%03x \n", adc_0, adc_1);
    //DEBUG_LIGHTYELLOW("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    if(adc_0 < 0x600) //  adapter plug out //0x0__~0x1__ no adapter; 0xa__~0xb__ with adapter
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //DEBUG_ADC("*** BAT LEVEL: %d  Curst %d ,,,Last %d....  ##%02d:%02d:%02d\n",level,cur_status,lastLV,localTime.hour, localTime.min, localTime.sec);

    if((level ==GPIO_LEVEL_LO) && (cur_status_dc == 1)) // charging
    {
        cur_status = UI_BATTERY_CHARGE;
        adc_value = 0;
        if(count_osd < 5) //200ms a time, 5 for 1 sec
        {
            count_osd++;
            return 0;
        }
    }
    else if (cur_status_dc == 1)   // no battery but DC, display full battery icon, //adc0=0x500, 
    {
        cur_status = UI_BATTERY_LV3;
        lastLV = cur_status;
        batteryflag = cur_status;
        sysbackSetEvt(SYS_BACK_DRAW_BATTERY, cur_status);
        adc_value = 0;
        count_osd = 0;
        cnt = 0;
        return 0;
    }
    else
    {
        if (cnt == 4)
        {
            adc_value = adc_value>>2;
            if (adc_value > 0x98C)
                cur_status = UI_BATTERY_LV3;
            else if (adc_value > 0x948)
                cur_status = UI_BATTERY_LV2;
            else if (adc_value > 0x90A)
                cur_status = UI_BATTERY_LV1;
            else if (adc_value > 0x8E9)
                cur_status = UI_BATTERY_LV0;
            else
                cur_status = UI_BATTERY_SHUTDOWN;

            adc_value = 0;
        }
        else
        {
            adc_value += adc_1;
            cnt++;
            return 0;
        }
    }
    
    //DEBUG_ADC("*** BAT LEVEL: %d ,,,%d.... %x ##%02d:%02d:%02d\n",cur_status,lastLV,adc_2,localTime.hour, localTime.min, localTime.sec);
    if (lastLV != cur_status)
    {
        if((cnt >= 4) || (count_osd >= 5))
        {
            if ((lastLV < UI_BATTERY_LV3) && (cur_status > lastLV) && (cur_status <= UI_BATTERY_LV3)) //avoid battery level raise up by accident
            {
                sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
                cnt = 0;
                return 0;
            }

            lastLV = cur_status;
            batteryflag = cur_status;
            cnt = 0;
            count_osd = 0;

            if(batteryflag == UI_BATTERY_SHUTDOWN)
            {
                printf("Warning!!! Battery is dead. Send power off to ui\n");
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
    }
    else
    {
        cnt=0;
        count_osd = 0;
    }
    
    sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);

#endif




#if(HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903)
    return 0;
    //RTC_Get_Time(&localTime);
    detect_cnt=10;

    adc_2 = (AdcRecData_G3G4  &  0xfff0000)>>16;
    adc_3 = (AdcRecData_G3G4  &  0x0000fff);

    //printf("Date Time 20%02d/%02d/%02d %02d:%02d:%02d\n",localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
    cur_status=UI_BATTERY_NONE;
    if( adc_2 < 0x900) // adapter plug out
    {
        cur_status_dc = 0;
    }
    else  // adapter plug in
    {
        cur_status_dc = 1;
    }

    gpioGetLevel(GPIO_GROUP_BATTERY_CHARGE, GPIO_BIT_BATTERY_CHARGE, &level);
    //printf("AdcRecData_G3G4 %08x,adc_2 %03x,adc_3 %03x\n", AdcRecData_G3G4, adc_2, adc_3);
        //printf("===> AdcRecData_G1G2  %08x AdcRecData_G3G4 %08x %d %d\n",AdcRecData_G1G2,AdcRecData_G3G4,adc_0,adc_2);

    if((level == GPIO_LEVEL_LO) && (cur_status_dc == 1))
    {
        cur_status = UI_BATTERY_CHARGE;
        //DEBUG_ADC("===> UI_BATTERY_CHARGE\n");
    }
    else
    {
        if(cnt == 4)
        {
            adc_value =adc_value>>2;
            //DEBUG_ADC("===> ADC value %x \n",adc_value);
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
            return 0;
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
#endif

#if(HW_BOARD_OPTION == MR9200_RX_ROULE)
    return;

    if(Main_Init_Ready  == 0 )
        return 0;
	
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
        if ((lastLV < UI_BATTERY_LV5) && (cur_status > lastLV) && (cur_status < UI_BATTERY_LV5))
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
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
        }
        else if(cur_status == UI_BATTERY_LV0)   /* LV0 enter fake shutdown*/
        {
            //DEBUG_ADC("******* Fake Shutdown *******\n");
            batteryflag=UI_BATTERY_LV0;

        }
        else                                     /* Only Battery */
        {
            DEBUG_ADC("****** Only Battery ******\n");
            batteryflag = cur_status;
           // sysbackSetEvt(SYS_BACK_DRAW_BATTERY, batteryflag);
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
    // test Group1 AGC function
    INT32U CurDMAIntStat;
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

}

/* SW 0108 S */
s32 adcKeyPolling1(void)
{

    return 0;
}

s32 adcKeyPolling(void)
{
    u8           adc_key1,adc_key2;
    u32          adc_1;
    static u8    ucDisableCnt = 0, lastKey = 0, waitCnt = 13, lastKey1 = 0;
    static u8    unGetValue[3]={0};
    u8           tempKey = UI_KEY_READY;
    
#if (HW_BOARD_OPTION == MR9200_RX_JIT_M916HN4)
    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G3G4 & 0xff0) >>4;
    //DEBUG_RED("adc DacKeyCtrl %08x  %08x  %d  %d\n",AdcRecData_G3G4,AdcRecData_G1G2,adc_key1,adc_key2);

     /*==================Prevent From Cont. Detecting===================*/
    if (ucDisableCnt >= 3)
    {
        if(ucDisableCnt > waitCnt)
            ucDisableCnt = 0;
        else
            ucDisableCnt++;
        return 0;
    }
    
    if ((UIKey != UI_KEY_READY && MsgKey != UI_KEY_WAIT_KEY) || ((adc_key1 >= 220) && (adc_key2 >= 224)))
    {
        if ((adc_key1 >= 220) && (adc_key2 >= 224))
        {
            ucDisableCnt = 0;
            waitCnt = 13;
            lastKey = 0;
        }
        return 0;
    }

    if (adc_key1 > 220)
    {     
        if(adc_key2 > 215)
        {
            tempKey = UI_KEY_DOWN;
            DEBUG_ADC("Key UI_KEY_DOWN\n");
        }
        else if (adc_key2 > 185)
        {
            tempKey = UI_KEY_LCD_BL;
            DEBUG_ADC("Key UI_KEY_LCD_BL\n");
        }
        else if (adc_key2 > 150)
        {
            tempKey = UI_KEY_TVOUT_DET;
            DEBUG_ADC("Key UI_KEY_TVOUT_DET\n");
        }
        else if (adc_key2 > 120)
        {
            tempKey = UI_KEY_LEFT;
            DEBUG_ADC("Key UI_KEY_LEFT\n");
        }
        else if (adc_key2 > 80)
        {
            tempKey = UI_KEY_RIGHT;
            DEBUG_ADC("Key UI_KEY_RIGHT\n");
        }
        else if (adc_key2 > 60)
        {
            tempKey = UI_KEY_UP;
            DEBUG_ADC("Key UI_KEY_UP\n");
        }
        else if (adc_key2 > 20)
        {
            tempKey = UI_KEY_REC;
            DEBUG_ADC("Key UI_KEY_REC\n");
        }
        else if (adc_key2 > -1)
        {
            tempKey = UI_KEY_TALK;
            DEBUG_ADC("Key UI_KEY_TALK\n");
        }
    }
    else
    {
        if (adc_key1 > 180)
        {
            tempKey = UI_KEY_RF_QUAD;
            DEBUG_ADC("Key UI_KEY_RF_QUAD\n");
        }
        else if(adc_key1 > 120)
        {
            tempKey = UI_KEY_ENTER;
            DEBUG_ADC("Key UI_KEY_ENTER\n");
        }
    }
    
    #if 1
	/*==================Button Press De-Bouncing===================*/

    if(adc_key1 < 220)
    {
	    unGetValue[ucDisableCnt] = tempKey;	/* get the current value */
    }    
    else if(adc_key2 < 224)
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
    if (tempKey != UI_KEY_NONE)
        uiSentKeyToUi(tempKey);

#elif (HW_BOARD_OPTION == MR9200_RX_RDI_M906)

    adc_key1 = (AdcRecData_G3G4 & 0xff00000) >>20;
    adc_key2 = (AdcRecData_G3G4  & 0xff0)>>4;
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

    if((adc_key2 > 75) && (adc_key2 <85))
    {
        tempKey = UI_KEY_B_DOWN;
        DEBUG_ADC("Key UI_KEY_DOWN\n");
    }
    else if (adc_key2 < 10)
    {
        tempKey = UI_KEY_B_LEFT;
        DEBUG_ADC("Key UI_KEY_LEFT\n");
    }
    else if ((adc_key2 > 45) && (adc_key2 <55))
    {
        tempKey = UI_KEY_B_RIGHT;
        DEBUG_ADC("Key UI_KEY_RIGHT\n");
    }
    else if ((adc_key2 > 24) && (adc_key2 <34))
    {
        tempKey = UI_KEY_B_UP;
        DEBUG_ADC("Key UI_KEY_UP\n");
    }
    else
    {
        DEBUG_UI("ADC Value Error %d \n",adc_key2);
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
    
#endif    
    return 0;
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
#if ((Volume_Control == Volume_Control_PT2257) && (!AUDIO_IN_OUT_SELFTEST))
    VolumeControl_PT2257(unAudioVol);
#endif
}

/*
Routine Description:

    Set DAC Power ON/off

Arguments:

    u8 u8OnOff

Return Value:

    None.
*/
void adcSetDAC_Power(u8 u8OnOff)
{
    if(u8OnOff)
        DacTxCtrl |= DAC_PWON; //audio on
    else
        DacTxCtrl &= ~DAC_PWON; //audio off
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

    DacTxCtrl  |= DAC_ENVREF_ON | DAC_PWON | DAC_MUTE_DISA;
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
    if (unAudioVol > 31)
    {
        DacTxCtrl   = (DacTxCtrl & ~DAC_PGAG_MUTE) | (0x1f << 8);
        #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
            (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
             (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) || (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        DacTxCtrl &= ~DAC_PWON;
        #endif
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
        #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
            (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
             (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) || (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        DacTxCtrl &= ~DAC_PWON;
        #endif
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
        #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) ||\
            (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
             (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) ||\
             (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R) || (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
        DacTxCtrl &= ~DAC_PWON;
        #endif
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
    return value;
}

u32 adcGetValueAverage(u8 adc_num, u16 average_num)
{

    u32 value =0;
    u32 i,j;
    u32 temp;
    u32 average=0;
    u8  drop_num=3; //5 drop 1, 15 drop 3

    if (average_num <= 5)
        drop_num = 1;
    else if (average_num <= 10)
        drop_num = 2;
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
        return value;
    }
    else
    {
        ADCcnt ++;
        return 0;
    }


}

void SPKERPlay(void)
{
    //DEBUG_YELLOW("SPKERPlay \n");  
    gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
  #if((AUDIO_DEVICE == AUDIO_IIS_WM8940)&&(HW_BOARD_OPTION != MR9100_TX_RDI_CA811))
    WM8940_SpeakVol_FadeIn();
  #endif
}
void SPKERStop(void)
{
    //DEBUG_YELLOW("SPKERStop \n");
  #if((AUDIO_DEVICE == AUDIO_IIS_WM8940)&&(HW_BOARD_OPTION != MR9100_TX_RDI_CA811))
    WM8940_SpeakVol_FadeOut();
  #endif
    gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0);

}






