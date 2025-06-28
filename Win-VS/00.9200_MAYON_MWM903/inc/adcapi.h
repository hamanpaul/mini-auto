/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	adcapi.h

Abstract:

   	The application interface of ADC.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __ADC_API_H__
#define __ADC_API_H__

#include "../adc/inc/adcreg.h"



enum
{
    ADC_IN_0,
    ADC_IN_1,
    ADC_IN_2,
    ADC_IN_3,
};

#define PCM_8BITFMT_UNSIGN   0
#define PCM_8BITFMT_SIGN     1

//---------Extern Fuction-----------//
extern int adc8BitPCMFmtSel(int OutFmt);
extern void adcIntHandler(void);
extern s32 adcInit(u8 adcrecena);
extern void adcTest(void);
extern u32 adcBatteryCheck(void);
extern s32 adcKeyPolling(void);
extern void init_DAC_play(u8 start);
extern void adcBrtSatPolling(void);		/* polling Brightness and Saturation voltage level */
extern void adcSetDAC_OutputGain(u32);
extern void adcInitDAC_Play(u32);
extern void adcSetADC_MICIN_PGA_Gain(u32 unAudioVol);
extern u8 adcCheckHiddenModeIndex(void);
extern void adcInitDACTx(void);
extern void adcGetBattLevel(u8*);
extern void adcSetDAC_L_CH_Gain(u32 unAudioVol);
extern void adcSetDAC_R_CH_Gain(u32 unAudioVol);
extern u32  adcGetValue(u8 adc_num);
extern void adcSetDAC_Power(u8 u8OnOff);
extern void SPKERPlay(void);
extern void SPKERStop(void);

//--------Extern Variable-----------//
extern u8 polling_recon;
extern u8 BootToMenu;





#endif
