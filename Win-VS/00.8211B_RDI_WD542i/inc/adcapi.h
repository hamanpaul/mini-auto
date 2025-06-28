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

extern s32 adcInit(u8 adcrecena);
void adcIntHandler(void);
extern void adcTest(void);
extern u32 adcBatteryCheck(void);
extern s32 adcKeyPolling(void);
//extern s32 adcKeyPolling1(void);
extern void init_DAC_play(u8 start);
extern void adcBrtSatPolling(void);		/* polling Brightness and Saturation voltage level */
extern void adcSetDAC_OutputGain(u32);
extern void adcInitDAC_Play(u32);
extern void adcSetADC_MICIN_PGA_Gain(u32 unAudioVol);
extern u8 adcCheckHiddenModeIndex(void);
extern void adcInitDACTx(void);
extern u8 polling_recon;
extern u8 BootToMenu;
extern void adcGetBattLevel(u8*);
extern void adcSetDAC_Power(u8 Power_control);

extern void adcSetDAC_L_CH_Gain(u32 unAudioVol);
extern void adcSetDAC_R_CH_Gain(u32 unAudioVol);
extern u32  adcGetValue(u8 adc_num);
extern void adcEMICOnOff(u8 On);
extern void adcTemperaturePolling(void);

enum
{
    ADC_IN_0,
    ADC_IN_1,
    ADC_IN_2,
    ADC_IN_3,
};




#endif
