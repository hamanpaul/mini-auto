

#ifndef __MARS_ADC_H__
#define __MARS_ADC_H__

#include <osapi.h>

#if 0

#define ADC_ID_0    0
#define ADC_ID_1    1
#define ADC_ID_2    2
#define ADC_ID_3    3
#define ADC_ID_NUM  4

#ifndef ADC_PWON
#define ADC_PWON             0x00000001
#define ADC_PWOFF            0x00000000
#define ADC_RX_ENA           0x00000004
#define ADC_RX_DISA          0x00000000
#define ADC_REF_TC_0PERCENT  0x00000080
#define ADC_CLK_DIV_A_53     0x00003500
#define ADC_CLK_DIV_B_22     0x00160000
#define ADC_PWD_PGA_ENA      0x00000000
#define ADC_PWD_PGA_DISA     0x01000000

#define ADC_PGINM_20DB       0x04000000
#define ADC_PGINM_10DB       0x02000000
#define ADC_PGINM_0DB        0x00000000

#define ADC_PGINL_21DB       0x38000000
#define ADC_PGINL_18DB       0x30000000
#define ADC_PGINL_15DB       0x28000000
#define ADC_PGINL_12DB       0x20000000
#define ADC_PGINL_9DB        0x18000000
#define ADC_PGINL_6DB        0x10000000
#define ADC_PGINL_3DB        0x08000000
#define ADC_PGINL_0DB        0x00000000

#define ADC_REC_CH0          0x00000000
#define ADC_REC_CH1          0x40000000  
#define ADC_REC_CH2          0x80000000                      
#define ADC_REC_CH3          0xC0000000



#define DAC_PWON               0x00000000
#define DAC_PWOFF              0x00000001
#define DAC_MUTE_ENA           0x00000002
#define DAC_MUTE_DISA          0x00000000
#define DAC_MODE_BINARY        0x00000004
#define DAC_MODE_COMPLEMENT    0x00000000
#define DAC_PGO_0DB            0x00000000
#define DAC_PGO_2_5DB          0x00000010
#define DAC_PGO_5DB            0x00000020
#define DAC_PGO_7_5DB          0x00000030
#define DAC_PGO_10DB           0x00000040
#define DAC_PGO_20DB           0x00000080

#endif

#endif

//=================================================================
extern void marsADCInit(INT32U uiSysClkADCDiv, INT32U uiAdcCtrlReg);
extern void marsADCSetRXCtrl(INT32U uiADCRXCtrl);
extern void marsDACSetTXCtrl(INT32U uiDACTXCtrl);
extern INT32U marsADCGetRXData(INT32U uiADCCh);

//=================================================================

#endif    // __MARS_ADC_H__
