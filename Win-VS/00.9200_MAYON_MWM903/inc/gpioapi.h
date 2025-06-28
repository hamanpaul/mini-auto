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
extern int gpioCheckLevel_USB(void);
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

#if(HW_BOARD_OPTION == MR8202A_RX_MAYON)
typedef enum
{
    LED_NONE = 0,
    LED_RED_LIGHT,
    LED_RED_LIGHT_1S,
    LED_GREEN_LIGHT,
    LED_GREEN_LIGHT_300MS,
    LED_YELLOW_LIGHT_500MS,
    LED_NET_LIGHT,
    LED_NET_LIGHT_1S,
    LED_NET_LIGHT_500MS,
    LED_OFF,
} LED_STATUS;

typedef enum _SPK_CYCLE
{
    SPK_NONE = 0,
    SPK_STOP, 
    SPK_START,
    SPK_1S,
    SPK_5S,
} SPK_CYCLE;
extern void gpioTimerCtrLed(u32 CamNum, LED_STATUS state);
extern void gpioTimerCtrNetLed(LED_STATUS state);
extern void gpioSDPwrSwitch(u32 PWR);
extern void gpioTimerCtrSpeak(SPK_CYCLE state);
#elif(HW_BOARD_OPTION == A1025A_EVB_axviwe)
typedef enum
{
    LED_NONE = 0,
    LED_ON,
    LED_FLASH_500MS,
    LED_FLASH_1S,
    LED_OFF,
} LED_STATUS;

typedef enum _SPK_CYCLE
{
    SPK_NONE = 0,
    SPK_STOP, 
    SPK_START,
    SPK_1P,
    SPK_3P,
} SPK_CYCLE;

extern void gpioTimerCtrLed(u32 CamNum, LED_STATUS state);
extern void gpioTimerCtrNetLed(LED_STATUS state);
extern void gpioSDPwrSwitch(u32 PWR);
#elif(HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018)
typedef enum
{
    LED_NONE = 0,
    LED_RED_LIGHT,
    LED_RED_LIGHT_1S,
    LED_GREEN_LIGHT,
    LED_GREEN_LIGHT_300MS,
    LED_YELLOW_LIGHT_500MS,
    LED_NET_LIGHT,
    LED_NET_LIGHT_1S,
    LED_NET_LIGHT_500MS,
    LED_OFF,
} LED_STATUS;

extern void gpioTimerCtrNetLed(LED_STATUS state);
extern void gpioTimerCtrLed(u32 CamNum, LED_STATUS state);
#elif ((HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530)||(HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8730))
typedef enum led_status
{
    LED_NONE = 0,
    LED_ALERT_FLASH_1Hz,
    LED_ALERT_FLASH_4Hz,
    LED_ALERT_ON,
    LED_ALERT_OFF,
    LED_ON,
    LED_FLASH_1S,
    LED_FLASH_500MS,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
extern void gpioTimerCtrNetLed(LED_STATUS state);
#elif (UI_VERSION == UI_VERSION_TRANWO)
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
      (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
      (UI_VERSION == UI_VERSION_RDI_3) || (HW_BOARD_OPTION  == MR9200_RX_MAYON_MWM903))
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif ( (HW_BOARD_OPTION == MR9600_RX_RDI_AHD) || (HW_BOARD_OPTION == MR9600_RX_SKY_AHD) || (HW_BOARD_OPTION == MR9600_RX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9120_RX_DB_AHD) || (HW_BOARD_OPTION == MR9120_RX_MUXCOM_AHD) || (HW_BOARD_OPTION == MR9120_RX_DB_HDMI))
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH_500ms,
    LED_FLASH_1000ms,
    LED_ON,
    LED_OFF,
} LED_STATUS;
extern void gpioTimerCtrLed(LED_STATUS);
#elif ((HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T) ||(HW_BOARD_OPTION == MR9120_TX_TRANWO_D87T))
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
#elif (HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4)
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
#elif (HW_BOARD_OPTION == MR9100_AHDINREC_MUXCOM)
typedef enum led_status
{
    LED_NONE = 0,
    LED_FLASH_5Times,
    LED_FLASH_3Times,
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
#define GPIO_IN_PULLUP_ENA	0   //與字面上相反, GPIO inpu的 PULL-UP 關掉
#define GPIO_IN_PULLUP_DISA	1   //與字面上相反, GPIO inpu的 PULL-UP 打開

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
extern void gpio_IIC_W_RByte(GPIO_IIC_CFG *stGPIO_IIC, u32 Value);
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
#define IIC_DEV_HM8563      9
#define IIC_DEV_PT7C43390      10

#if GPIO_I2C_ENA
  extern GPIO_IIC_CFG GPIO_IIC_CFG_PT2257   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_TVP5150  ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_BIT1605  ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_OV7740   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_OV7725   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_H30CD    ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_BIT1201G ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_CS8556   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_HM1375;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_FM1288;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_HM8563;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_PT7C43390   ;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_PCF8574;
  extern GPIO_IIC_CFG GPIO_IIC_CFG_PCF8575;
#endif
extern s8 gpio_IIC_Write(u8, u32, u8, u8);
extern s8 gpio_IIC_Read(u8, u32, u8, u8 *);
extern s8 gpio_IIC_Read_Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData);
extern s8 gpio_IIC_Read_2Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData);

#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
extern u8 getHDMI_HotPlugStatus(void);
#endif


#if(G_SENSOR_DETECT)
extern u8  GSensorEvent;
#endif

extern void IRIntHandler(void);
extern void IRSetCustomCode(u32 customCode);
extern void IRCtrEnable(BOOLEAN enable);
extern void IRCtrReset(BOOLEAN reset);
extern void IREnableInt(BOOLEAN enable);
extern void IRSetDiv(u32 divisor);
extern void IRGetRecCustomCode(u32* RecCustom);
extern s32 gpioConfig(u8 group, u8 pin, GPIO_CFG* pCfg);
extern s32 gpioLvTrgIntCfg(u8 group, u8 pin, GPIO_LVTRG_SATAUS cfg);

#if (HW_BOARD_OPTION == A1025A_EVB_axviwe)
extern void gpioSpeakSwitch(SPK_CYCLE param);
#endif

#endif
