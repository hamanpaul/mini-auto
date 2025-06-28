/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpioapi.h

Abstract:

   	The application interface of general purpose I/O.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __GPIO_API_H__
#define __GPIO_API_H__

/*CY 0718*/

extern s32 gpioInit(void);
extern void gpioIntHandler(void);
extern void gpio_1_IntHandler(void);
extern s32 gpioSetLevel(u8, u8, u8);
extern s32 gpioGetLevel(u8, u8, u8*);
extern s32 gpioExpandSetLevel(u8, u8, u8);
extern s32 gpioExpandGetLevel(u8, u8, u8*);
extern void gpioSetInt(u8, u8, u8);
extern void gpioCheckLevel_USB(void);
extern u8 gpio_Get_GSensor_SetVal(void);
extern u8	gpioCheckLevel(u8, u8);
extern void gpioSetIntr(u8, u8);
extern void gpioSetDir(u8, u8, u8);
extern void gpioSetEnable(u8 , u8 , u8 );

#define GPIO_GROUP_COUNT    4
#define GPIO_PIN_COUNT      32

/* Type definition */

typedef struct _GPIO_CFG {
	u8		ena;
	u8 		dir;
	u8 		level;
	u8		inPullUp;
} GPIO_CFG;

typedef struct _GPIO_INT_CFG {
	u8		intEna;
	u8		intFallEdgeEna;
	u8		intRiseEdgeEna;
} GPIO_INT_CFG;

typedef enum gpio_level_trigger
{
    GPIO_LVTRG_NONE = 0,
    GPIO_LVTRG_HIGH,
    GPIO_LVTRG_LOW
} GPIO_LVTRG_SATAUS;



#if ((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_TX_JESMAY))
typedef enum led_status
{
    LED_NONE = 0,
    LED_R_ON,
    LED_R_FLASH,
    LED_R_OFF,
} LED_STATUS;
#elif ((HW_BOARD_OPTION == MR8600_RX_RDI) || (HW_BOARD_OPTION == MR8120_RX_RDI ) || \
        (HW_BOARD_OPTION == MR8600_RX_TRANWO) ||(HW_BOARD_OPTION == MR8600_RX_RDI2) )
        
typedef enum led_status
{
    LED_NONE = 0,
    LED_R_FLASH,
    LED_L_FLASH,
    LED_RL_FLASH,
    LED_R_LONG_FLASH,
    LED_L_LONG_FLASH,
    LED_R_ON,
    LED_L_ON,
    LED_RL_ON,
    LED_L_OFF,
    LED_R_OFF,
    LED_OFF,
} LED_STATUS;
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_RX240)
typedef enum led_status
{
    LED_NONE = 0,
    LED_FULL_ON,
    LED_NEW_ON,
    LED_AREC_ON,
    LED_REC_ON,
    LED_FULL_OFF,
    LED_NEW_OFF,
    LED_AREC_OFF,
    LED_REC_OFF,
    LED_ON,
    LED_OFF,
} LED_STATUS;

#elif(HW_BOARD_OPTION == MR8120_TX_RDI_AV)
typedef enum led_status
{
    LED_NONE = 0,
    LED_OFF,
    LED_ON,
    LED_LONG_FLASH,
}LED_STATUS;
#elif(HW_BOARD_OPTION==MR6730_WINEYE)
typedef enum led_status
{
    LED_NONE = 0,
    LED_R_ON,
    LED_R_1SEC,   /*on 1sec off 1sec*/
    LED_R_05SEC,   /*on 0.5sec off 0.5sec*/
    LED_R_1SEC5T,   /*on 1sec off 1sec total 5 times*/
    LED_R_OFF,
    LED_B_ON,
    LED_B_1SEC,   /*on 1sec off 1sec*/
    LED_B_05SEC,   /*on 0.5sec off 0.5sec*/
    LED_B_OFF,
    LED_ALL_ON,     /*always on*/
    LED_ALL_3S1S,   /*on 3sec ,red on 1sec off 1sec and blue off*/
    LED_ALL_RE,     /*red: on 0.5sec off 0.5sec blue: off 0.5sec on 0.5sec*/
    LED_ALL_FLASH,  /*on 0.5sec off 0.5sec*/
    LED_ALL_OFF,    /*always off*/
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif((HW_BOARD_OPTION == MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8120_TX_TRANWO2) ||\
      (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH_1s,
    LED_FLASH_500ms,
    LED_FLASH_200ms,
    LED_FLASH_100ms,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif((HW_BOARD_OPTION  == MR8100_GCT_VM9710)||(HW_BOARD_OPTION == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH_1s,
    LED_FLASH_500ms,
    LED_FLASH_200ms,
    LED_FLASH_100ms,
    LED_FLASH_CH1,
    LED_FLASH_CH8,
    LED_FLASH_CH16,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);


#elif ((HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) ||(UI_VERSION == UI_VERSION_TRANWO) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) )
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH,
    LED_ON,
    LED_OFF,
    LED_RED_ON,    
    LED_RED_OFF,
    LED_RED_FLASH,
    LED_GREEN_ON,
    LED_GREEN_OFF,
    LED_GREEN_FLASH,
    LED_ORANGE_ON,
    LED_ORANGE_OFF,
    LED_ORANGE_FLASH,
    LED_ALERT_ON,
    LED_ALERT_OFF,
    LED_ALERT_FLASH,    
   
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) ||\
      (UI_VERSION == UI_VERSION_RDI_3) ||(UI_VERSION == UI_VERSION_RDI_4) ||(HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD) ||\
      (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720) || (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710)||\
      (HW_BOARD_OPTION == MR8120_TX_JESMAY_LCD)|| (HW_BOARD_OPTION == MR8120_TX_Philio)||\
      (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014) ||(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)||\
      (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif (HW_BOARD_OPTION == MR8120_RX_GCT_SC7700)
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);

#else
typedef enum led_status
{
    LED_NONE = 0,
    LED_R_FLASH,
    LED_L_FLASH,
    LED_RL_FLASH,
    LED_R_LONG_FLASH,
    LED_L_LONG_FLASH,
    LED_R_ON,
    LED_L_ON,
    LED_L_OFF,
    LED_R_OFF,
    LED_RL_ON,
    LED_OFF,

} LED_STATUS;
#endif

#if((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
typedef enum music_status{
    GPIO_PLAY_MUSIC1 = 0,
    GPIO_PLAY_MUSIC2,
    GPIO_PLAY_MUSIC3,
    GPIO_PLAY_MUSIC4,
    GPIO_PLAY_ALL,
    GPIO_MUSIC_STOP,
    GPIO_PLAY_PAUSE,
    GPIO_PLAY_RESTART,
    GPIO_PLAY_RESTART_ALL,
}MUSIC_STATUS;

extern u8 gpioMusicCtr(MUSIC_STATUS opvalue);
#elif (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
typedef enum music_status{
    GPIO_PLAY_MUSIC1 = 0,
    GPIO_PLAY_MUSIC2,
    GPIO_PLAY_MUSIC3,
    GPIO_PLAY_MUSIC4,
    GPIO_PLAY_ALL,
    GPIO_MUSIC_STOP,
    GPIO_PLAY_PAUSE,
    GPIO_PLAY_RESTART,
    GPIO_PLAY_RESTART_ALL,
}MUSIC_STATUS;

extern u8 gpioMusicCtr(MUSIC_STATUS opvalue);

#endif

#if((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    extern void gpioSetBLEPWR(u8 gpioLev);
    extern u8 gpioGetBLEPWR(void);
    extern u8 gpioGetBLENTP(void);
#endif

/* GpioEna */
#define GPIO_DISA		0
#define GPIO_ENA		1

/* GpioDir */
#define GPIO_DIR_OUT		0
#define GPIO_DIR_IN		1

/* GpioLevel */
#define GPIO_LEVEL_LO		0
#define GPIO_LEVEL_HI		1

/* GpioInPullUp */
#define GPIO_IN_PULLUP_ENA	0
#define GPIO_IN_PULLUP_DISA	1

/* GpioIntEna */
#define GPIO_INT_DISA		0
#define GPIO_INT_ENA		1

/* GpioIntStat */
#define GPIO_INT_NOT_OCCUR	0
#define GPIO_INT_OCCUR		1

/* GpioInIntFallEdge */
#define GPIO_IN_INT_FALL_DISA	0
#define GPIO_IN_INT_FALL_ENA	1

/* GpioInIntRiseEdge */
#define GPIO_IN_INT_RISE_DISA	0
#define GPIO_IN_INT_RISE_ENA	1
// for K310 debug
extern void gpioDebug(u32 value);
extern void gpioDebug1(u32 value);
extern void gpioDebug2(u32 value);
extern void gpioDebug12(u32 value);
extern void gpioDebug123(u32 value);
extern void gpioDebugFIQ(u32 value);
extern void gpioDebugIRQ(u32 value);
extern void gpioeBellAll(u32 value);
extern void gpioeBellIRQ(u32 value);
extern s32 gpioConfig(u8, u8, GPIO_CFG*);
extern s32 gpioLvTrgIntCfg(u8 group, u8 pin, GPIO_LVTRG_SATAUS cfg);




//#if(HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS )
#if(HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS\
	||HW_BOARD_OPTION == MR6730_AFN\
	)	
	extern void gpioKeyPolling(void);
#endif


#if GPIO_I2C_ENA

#define gpio_IIC_SDA_W  	gpioSetLevel
#define gpio_IIC_SDA_R		gpioGetLevel
#define gpio_IIC_SCK_W		gpioSetLevel
#define gpio_IIC_SCK_R		gpioGetLevel


typedef struct _GPIO_IIC_CFG
{
	u8	GrpSCK;
	u8	GrpSDA;
	u32	BitSCK;
	u32	BitSDA;
}GPIO_IIC_CFG;

extern void gpio_IIC_W_Byte(GPIO_IIC_CFG *,u32);
extern void gpio_IIC_R_Byte(GPIO_IIC_CFG *,u8 *);
extern s8   gpio_IIC_Ack_R(GPIO_IIC_CFG *);
extern void gpio_IIC_Ack_W(GPIO_IIC_CFG *);
extern s8   gpio_IIC_nAck_R(GPIO_IIC_CFG *);
extern void gpio_IIC_nAck_W(GPIO_IIC_CFG *);
extern void gpio_IIC_Enable(GPIO_IIC_CFG ucDevCFG);
extern void gpio_IIC_Disable(GPIO_IIC_CFG ucDevCFG);

#endif

extern GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT];
extern GPIO_CFG gpioCfg[GPIO_GROUP_COUNT][GPIO_PIN_COUNT];
extern  u8  AlarmDetect;    // 1: Alarm trigger, 0: otherwise

#define	IIC_DEV_TVP5150		1
#define	IIC_DEV_PT2257		2
#define	IIC_DEV_BIT1605		3
#define IIC_DEV_OV7740		4
#define IIC_DEV_OV7725      5
#define IIC_DEV_H30CD       6
#define IIC_DEV_BIT1201G    7
#define IIC_DEV_CS8556      8


#if GPIO_I2C_ENA
  extern GPIO_IIC_CFG GPIO_IIC_CFG_PT2257   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_TVP5150  ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_BIT1605  ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_OV7740   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_OV7725   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_H30CD    ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_BIT1201G ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_CS8556   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_HM1375   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_FM1288   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_SD2068   ;
#endif
extern s8 gpio_IIC_Write(u8, u32, u8, u8);
extern s8 gpio_IIC_Read(u8, u32, u8, u8 *);
extern s8 gpio_IIC_Read_Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData);
extern s8 gpio_IIC_Read_2Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData);






#if(G_SENSOR_DETECT)
extern u8  GSensorEvent;
#endif

#if(HW_BOARD_OPTION == MR8600_RX_GCT )

extern void RF_TEST_MODE_CH2(u8 level);
extern void RF_TEST_MODE_CH1(u8 level);
#endif

#if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
extern void IRIntHandler(void);
extern void IRSetCustomCode(u32 customCode);
extern void IRCtrEnable(BOOLEAN enable);
extern void IRCtrReset(BOOLEAN reset);
extern void IREnableInt(BOOLEAN enable);
extern void IRSetDiv(u32 divisor);
extern void IRGetRecCustomCode(u32* RecCustom);

#endif

#if((HW_BOARD_OPTION == MR8120_TX_TRANWO ) ||(HW_BOARD_OPTION == MR8120_TX_TRANWO2 )  ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) || (HW_BOARD_OPTION == MR8120_TX_JIT) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)|| (HW_BOARD_OPTION == MR8120_TX_MA8806)||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)||\
    (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || \
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) ||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || \
    (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
extern void gpioDetectIRLED(void);
extern void gpioIRLEDInit(void);
#endif

#endif
