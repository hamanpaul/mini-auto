/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    gpio.c

Abstract:

    The routines of general purpose I/O.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

/*CY 0718*/

#include "general.h"
#include "board.h"
#include "gpioapi.h"    
#include "gpio.h"
#include "gpioreg.h"
#include "sysapi.h"
#include "Usbapi.h"
#include "asfapi.h"
#include "intapi.h"
#include "intapi.h"


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

#if GPIO_I2C_ENA
  /* Set init values of gpio for simulating IIC */
  GPIO_IIC_CFG GPIO_IIC_CFG_PT2257   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};    /* Audio volume control IC PT2257 */
  GPIO_IIC_CFG GPIO_IIC_CFG_TVP5150  = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_BIT1605  = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_OV7740   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_OV7725   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_H30CD    = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_BIT1201G = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};  
  GPIO_IIC_CFG GPIO_IIC_CFG_CS8556   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};    
  GPIO_IIC_CFG GPIO_IIC_CFG_HM1375   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};
  GPIO_IIC_CFG GPIO_IIC_CFG_FM1288   = {GPIO_GROUP_I2C_SCK, GPIO_GROUP_I2C_SDA, GPIO_BIT_I2C_SCK, GPIO_BIT_I2C_SDA};  
#endif

/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */ 

/*
 *********************************************************************************************************
 * Extern Functions
 *********************************************************************************************************
 */ 

extern u32 getTVinFormat(void); 
#ifdef MMU_SUPPORT
extern void Disable_Dcache(void);
#endif

extern void marsIntIRQEnable(INT32U intno);


void i2cInit_TVP5150_1(void);

void gpio_SensorInSwithc2TVIn(void);
void gpio_TVInSwithc2SensorIn(void);
s32 gpioLvTrgIntCfg(u8 group, u8 pin, GPIO_LVTRG_SATAUS cfg);


//=====================================================================//



/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
 
s32 gpioConfig(u8, u8, GPIO_CFG*);
s32 gpioIntConfig(u8, u8, GPIO_INT_CFG*);  
s32 gpioSetLevel(u8, u8, u8);
s32 gpioGetLevel(u8, u8, u8*);
 
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */ 
    

int gpioCheckLevel_USB(void)
{
   return 1;
}






//--------------------------------------------------------------------//
/*

Routine Description:

    Initialize the general purpose I/O.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 gpioInit(void)
{
    u32 i, j;
    GPIO_CFG c;
        
    for (i = 0; i < GPIO_GROUP_COUNT; i++)
    {
        for (j = 0; j < GPIO_PIN_COUNT; j++)        
        {
            gpioConfig(i, j, &gpioCfg[i][j]);   

            if (i == 0)
            {
                gpioIntConfig(i, j, &gpio0IntCfg[j]);   
            }
        #if( (CHIP_OPTION == CHIP_A1018A) )
            else if (i==1)
            {
                if (j < 16)
                    gpioIntConfig(i, j, &gpio1IntCfg[j]);
                gpioLvTrgIntCfg(i, j, GPIO1_LVTRG_CFG[j]);
            }            
        #else
            else if (i==1)
            {
                gpioIntConfig(i, j, &gpio1IntCfg[j]);
                gpioLvTrgIntCfg(i, j, (GPIO_LVTRG_SATAUS)GPIO1_LVTRG_CFG[j]);
            }            
            else if (i==2)
            {
                gpioIntConfig(i, j, &gpio2IntCfg[j]);
                gpioLvTrgIntCfg(i, j, (GPIO_LVTRG_SATAUS)GPIO2_LVTRG_CFG[j]);
            }
            else if (i==3)
            {
                gpioIntConfig(i, j, &gpio3IntCfg[j]);
                gpioLvTrgIntCfg(i, j, (GPIO_LVTRG_SATAUS)GPIO3_LVTRG_CFG[j]);
            }
        #endif
            
        }
    }
    
    marsIntIRQEnable((INT32U)(INT_IRQMASK_GPIO_0 | INT_IRQMASK_GPIO_1));

    //Lucian: use for check chip id.
    c.ena = GPIO_ENA;
	c.dir = GPIO_DIR_IN;
	c.level = GPIO_LEVEL_LO;
	c.inPullUp = GPIO_IN_PULLUP_DISA;
    gpioConfig(2,26,&c);
    gpioConfig(2,27,&c);
	gpioConfig(2,28,&c);
    gpioConfig(2,29,&c);
    gpioConfig(2,30,&c);
    gpioConfig(2,31,&c);

#if( (HW_BOARD_OPTION == A1018B_SKB_128M_RX)|| (HW_BOARD_OPTION == A1018B_SKB_128M_BT_RX)|| (HW_BOARD_OPTION == A1019A_SKB_128M_RX) || (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB)||\
     (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9120_TX_RDI_USB) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) ||\
     (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_BT_USB) || (HW_BOARD_OPTION  == MR9100_TX_RDI_USB) )
    gpioSetLevel(1, 7, 1);
#endif
    
    return 1;   
}


void DisableTVOUTINT(void)
{
    
}

/*

Routine Description:

    The configuration of general purpose I/O.

Arguments:

    group - Group number.
    pin - Pin number.
    pCfg - Configuration.

Return Value:

    0 - Failure.
    1 - Success.

    NOTES: A1018B,A1019A 的GPIO 3-27,3-28 與 1-0,1-1  的PULL-UP config 設定交換. 2017/12/04

*/
s32 gpioConfig(u8 group, u8 pin, GPIO_CFG* pCfg)
{
    u32 bitSet = 0x00000001 << pin; 

    if (pin >= GPIO_PIN_COUNT)
        return 0;

    switch (group)
    {
        case 0:
            if (pCfg->dir == GPIO_DIR_OUT)
            {
                Gpio0Dir &= ~bitSet;
                if (pCfg->level == GPIO_LEVEL_LO)
                {
                    Gpio0Level &= ~bitSet;
                }
                else    /* GPIO_LEVEL_LO */
                {
                    Gpio0Level |= bitSet;
                }   
            }
            else    /* GPIO_DIR_IN */
            {
                Gpio0Dir |= bitSet;
                if (pCfg->inPullUp == GPIO_IN_PULLUP_ENA)
                {
                    Gpio0InPullUp &= ~bitSet;
                }
                else    /* GPIO_IN_PULLUP_DISA */
                {
                    Gpio0InPullUp |= bitSet;
                }
            }
            if (pCfg->ena == GPIO_DISA)
            {
                Gpio0Ena &= ~bitSet;
            }
            else    /* GPIO_ENA */
            {
                Gpio0Ena |= bitSet;
            }
            break;
        
        case 1:
            if (pCfg->dir == GPIO_DIR_OUT)
            {
                Gpio1Dir &= ~bitSet;
                if (pCfg->level == GPIO_LEVEL_LO)
                {
                    Gpio1Level &= ~bitSet;
                }
                else    /* GPIO_LEVEL_LO */
                {
                    Gpio1Level |= bitSet;
                }   
            }
            else    /* GPIO_DIR_IN */
            {
                Gpio1Dir |= bitSet;
                if (pCfg->inPullUp == GPIO_IN_PULLUP_ENA)
                {
                    Gpio1InPullUp &= ~bitSet;
                }
                else    /* GPIO_IN_PULLUP_DISA */
                {
                    Gpio1InPullUp |= bitSet;
                }
            }           
            if (pCfg->ena == GPIO_DISA)
            {
                Gpio1Ena &= ~bitSet;
            }
            else    /* GPIO_ENA */
            {
                Gpio1Ena |= bitSet;
            }
            break;
        
        case 2:
            if (pCfg->dir == GPIO_DIR_OUT)
            {
                Gpio2Dir &= ~bitSet;
                if (pCfg->level == GPIO_LEVEL_LO)
                {
                    Gpio2Level &= ~bitSet;
                }
                else    /* GPIO_LEVEL_LO */
                {
                    Gpio2Level |= bitSet;
                }   
            }
            else    /* GPIO_DIR_IN */
            {
                Gpio2Dir |= bitSet;
                if (pCfg->inPullUp == GPIO_IN_PULLUP_ENA)
                {
                    Gpio2InPullUp &= ~bitSet;
                }
                else    /* GPIO_IN_PULLUP_DISA */
                {
                    Gpio2InPullUp |= bitSet;
                }
            }           
            if (pCfg->ena == GPIO_DISA)
            {
                Gpio2Ena &= ~bitSet;
            }
            else    /* GPIO_ENA */
            {
                Gpio2Ena |= bitSet;
            }
            break;
            
        case 3:
            if (pCfg->dir == GPIO_DIR_OUT)
            {
                Gpio3Dir &= ~bitSet;
                if (pCfg->level == GPIO_LEVEL_LO)
                {
                    Gpio3Level &= ~bitSet;
                }
                else    /* GPIO_LEVEL_LO */
                {
                    Gpio3Level |= bitSet;
                }   
            }
            else    /* GPIO_DIR_IN */
            {
                Gpio3Dir |= bitSet;
                if (pCfg->inPullUp == GPIO_IN_PULLUP_ENA)
                {
                    Gpio3InPullUp &= ~bitSet;
                }
                else    /* GPIO_IN_PULLUP_DISA */
                {
                    Gpio3InPullUp |= bitSet;
                }
            }
            if (pCfg->ena == GPIO_DISA)
            {
                Gpio3Ena &= ~bitSet;
            }
            else    /* GPIO_ENA */
            {
                Gpio3Ena |= bitSet;
            }
            break;
            
        default:
            return 0;
    }               
        
    return 1;       
}

/*

Routine Description:

    Config gpio direction.

Arguments:

    ucGroup - Group number.
    ucPin - Pin number.
    ucDir - Direction.

Return Value:

    None.

*/
void gpioSetDir(u8 ucGroup, u8 ucPin, u8 ucDir)
{
    u32 bit = 1;

    bit <<= ucPin;

    if (ucPin >= GPIO_PIN_COUNT)
        return;


    switch (ucGroup)
    {
        case 0:

            if (ucDir == 0) /* output */
                Gpio0Dir &= ~bit;
            else        /* input */
                Gpio0Dir |= bit;

            break;

        case 1:

            if (ucDir == 0) /* output */
                Gpio1Dir &= ~bit;
            else        /* input */
                Gpio1Dir |= bit;

            break;

        case 2:

            if (ucDir == 0) /* output */
                Gpio2Dir &= ~bit;
            else        /* input */
                Gpio2Dir |= bit;

            break;

        case 3:

            if (ucDir == 0) /* output */
                Gpio3Dir &= ~bit;
            else        /* input */
                Gpio3Dir |= bit;

            break;          

    }


}


/*

Routine Description:

    The interrupt configuration of general purpose I/O.

Arguments:

    group - Group number.
    pin - Pin number.
    pCfg - Interrupt configuration.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 gpioIntConfig(u8 group, u8 pin, GPIO_INT_CFG* pCfg)
{
    u32 bitSet = 0x00000001 << pin; 
    
    if (pin >= GPIO_PIN_COUNT)
        return 0;


    if(group > 4)
        return 0;

    if(group == 0)
    {
        if (pCfg->intFallEdgeEna == GPIO_IN_INT_FALL_DISA)
        {
            Gpio0InIntFallEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_FALL_ENA*/
        {
            Gpio0InIntFallEdge |= bitSet; 
        }

        if (pCfg->intRiseEdgeEna == GPIO_IN_INT_RISE_DISA)
        {
            Gpio0InIntRiseEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_RISE_ENA*/
        {
            Gpio0InIntRiseEdge |= bitSet; 
        }
        
        if (pCfg->intEna == GPIO_INT_DISA)
        {
            Gpio0IntEna &= ~bitSet; 
        }
        else    /* GPIO_INT_ENA*/
        {
            Gpio0IntEna |= bitSet; 
        }
    }
    else if(group == 1)
    {
        if (pCfg->intFallEdgeEna == GPIO_IN_INT_FALL_DISA)
        {
            Gpio1InIntFallEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_FALL_ENA*/
        {
            Gpio1InIntFallEdge |= bitSet; 
        }

        if (pCfg->intRiseEdgeEna == GPIO_IN_INT_RISE_DISA)
        {
            Gpio1InIntRiseEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_RISE_ENA*/
        {
            Gpio1InIntRiseEdge |= bitSet; 
        }
    }
    else if(group == 2)
    {
        if (pCfg->intFallEdgeEna == GPIO_IN_INT_FALL_DISA)
        {
            Gpio2InIntFallEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_FALL_ENA*/
        {
            Gpio2InIntFallEdge |= bitSet; 
        }

        if (pCfg->intRiseEdgeEna == GPIO_IN_INT_RISE_DISA)
        {
            Gpio2InIntRiseEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_RISE_ENA*/
        {
            Gpio2InIntRiseEdge |= bitSet; 
        }
    }
    else if(group == 3)
    {
        if (pCfg->intFallEdgeEna == GPIO_IN_INT_FALL_DISA)
        {
            Gpio3InIntFallEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_FALL_ENA*/
        {
            Gpio3InIntFallEdge |= bitSet; 
        }   

        if (pCfg->intRiseEdgeEna == GPIO_IN_INT_RISE_DISA)
        {
            Gpio3InIntRiseEdge &= ~bitSet; 
        }
        else    /* GPIO_IN_INT_RISE_ENA*/
        {
            Gpio3InIntRiseEdge |= bitSet;
        }       
    }
    return 1;       
}

s32 gpioLvTrgIntCfg(u8 group, u8 pin, GPIO_LVTRG_SATAUS cfg)
{
    u32 LowBitSet, HighBitSet; 
    static u32  BackupReg = 0;

    if (pin >= GPIO_PIN_COUNT)
        return 0;


    if(group == 1)
    {
        if (pin >= 32)
        {
            DEBUG_GPIO("Error: gpio1LvTrgIntCfg out of range %d_%d\r\n",group, pin);
            return 0;
        }
        if (pin < 16 ) // [15:0]
        {
            LowBitSet   = 0x00000001 << pin;
            HighBitSet  = 0x00000001 << (pin+16);

            switch(cfg)
            {
                case GPIO_LVTRG_NONE:
                    BackupReg &= ~LowBitSet;
                    BackupReg &= ~HighBitSet;
                    break;

                case GPIO_LVTRG_HIGH:
                    BackupReg &= ~LowBitSet;
                    BackupReg |= HighBitSet;
                    break;

                case GPIO_LVTRG_LOW:
                    BackupReg |= LowBitSet;
                    BackupReg &= ~HighBitSet;
                    break;

                default:
                    DEBUG_GPIO("gpio1LvTrgIntCfg error set %d_%d %d\r\n",group, pin, cfg);
                    return 0;
                    
            }
            /*write only*/
            Gpio1LevelInt = BackupReg;
        }
        else // [31:16]
        {
            LowBitSet   = 0x00000001 << (pin%16);
            HighBitSet  = 0x00000001 << ((pin%16)+16);
            switch(cfg)
            {
                case GPIO_LVTRG_NONE:
                    BackupReg &= ~LowBitSet;
                    BackupReg &= ~HighBitSet;
                    break;

                case GPIO_LVTRG_HIGH:
                    BackupReg &= ~LowBitSet;
                    BackupReg |= HighBitSet;
                    break;

                case GPIO_LVTRG_LOW:
                    BackupReg |= LowBitSet;
                    BackupReg &= ~HighBitSet;
                    break;

                default:
                    DEBUG_GPIO("gpio1LvTrgIntCfg error set %d_%d %d\r\n",group, pin, cfg);
                    return 0;
                    
            }
            /*write only*/
            Gpio1aLevelInt = BackupReg;
        }
        return 1;
    }
    else if(group == 2)
    {
        if (pin >= 32)
        {
            DEBUG_GPIO("Error: gpio2LvTrgIntCfg out of range %d_%d\r\n",group, pin);
            return 0;
        }
        
        LowBitSet   = 0x00000001 << pin;
        HighBitSet  = 0x00000001 << pin;

        switch(cfg)
        {
            case GPIO_LVTRG_NONE:
                Gpio2LevelInt_Lo &= ~LowBitSet;
                Gpio2LevelInt_Hi &= ~HighBitSet;
                break;

            case GPIO_LVTRG_HIGH:
                Gpio2LevelInt_Lo &= ~LowBitSet;
                Gpio2LevelInt_Hi |= HighBitSet;
                break;

            case GPIO_LVTRG_LOW:
                Gpio2LevelInt_Lo |= LowBitSet;
                Gpio2LevelInt_Hi &= ~HighBitSet;
                break;

            default:
                DEBUG_GPIO("gpio2LvTrgIntCfg error set %d_%d %d\r\n",group, pin, cfg);
                return 0;
                
        }
        return 1;
    }
    else if(group == 3)
    {
        if (pin >= 32)
        {
            DEBUG_GPIO("Error: gpio3LvTrgIntCfg out of range %d_%d\r\n",group, pin);
            return 0;
        }
        
        LowBitSet   = 0x00000001 << pin;
        HighBitSet  = 0x00000001 << pin;

        switch(cfg)
        {
            case GPIO_LVTRG_NONE:
                Gpio3LevelInt_Lo &= ~LowBitSet;
                Gpio3LevelInt_Hi &= ~HighBitSet;
                break;

            case GPIO_LVTRG_HIGH:
                Gpio3LevelInt_Lo &= ~LowBitSet;
                Gpio3LevelInt_Hi |= HighBitSet;
                break;

            case GPIO_LVTRG_LOW:
                Gpio3LevelInt_Lo |= LowBitSet;
                Gpio3LevelInt_Hi &= ~HighBitSet;
                break;

            default:
                DEBUG_GPIO("gpio3LvTrgIntCfg error set %d_%d %d\r\n",group, pin, cfg);
                return 0;
                
        }
        return 1;
    }
    DEBUG_GPIO("gpioLvTrgIntCfg error group %d\r\n",group);
    return 0;
}
/*

Routine Description:

    Set the level of general purpose I/O.

Arguments:

    group - Group number.
    number - Pin number.
    level - Level of pin.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 gpioSetLevel(u8 group, u8 pin, u8 level)
{
    u32 bitSet = 0x00000001 << pin; 

    if (pin >= GPIO_PIN_COUNT)
        return 0;

    switch (group)
    {
        case 0:
            if (level == GPIO_LEVEL_LO)
                Gpio0Level &= ~bitSet; 
            else    /* GPIO_LEVEL_HI */
                Gpio0Level |= bitSet; 
            break;
        
        case 1:
            if (level == GPIO_LEVEL_LO)
                Gpio1Level &= ~bitSet; 
            else    /* GPIO_LEVEL_HI */
                Gpio1Level |= bitSet; 
            break;
        
        case 2:
            if (level == GPIO_LEVEL_LO)
                Gpio2Level &= ~bitSet; 
            else    /* GPIO_LEVEL_HI */
                Gpio2Level |= bitSet; 
            break;
        
        case 3:
            if (level == GPIO_LEVEL_LO)
                Gpio3Level &= ~bitSet; 
            else    /* GPIO_LEVEL_HI */
                Gpio3Level |= bitSet; 
            break;
        
        default:
            return 0;
    }       

    return 1;       
}

/*

Routine Description:

    Get the level of general purpose I/O.

Arguments:

    group - Group number.
    number - Pin number.
    pLevel - Level of pin.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 gpioGetLevel(u8 group, u8 pin, u8* pLevel)
{
    u32 bitSet = 0x00000001 << pin; 
    
    if (pin >= GPIO_PIN_COUNT)
        return 0;

    switch (group)
    {
        case 0:
            if (Gpio0Level & bitSet)
            {
                *pLevel = GPIO_LEVEL_HI;
            }    
            else 
            {
                *pLevel = GPIO_LEVEL_LO;
            }
            if ((Gpio0Dir & bitSet) == GPIO_DIR_OUT)
                return 0;
            break;
        
        case 1:
            if (Gpio1Level & bitSet)
            {
                *pLevel = GPIO_LEVEL_HI;
            }    
            else 
            {
                *pLevel = GPIO_LEVEL_LO;
            }
            if ((Gpio1Dir & bitSet) == GPIO_DIR_OUT)
                return 0;
            break;
        
        case 2:
            if (Gpio2Level & bitSet)
            {
                *pLevel = GPIO_LEVEL_HI;
            }    
            else 
            {
                *pLevel = GPIO_LEVEL_LO;
            }
            if ((Gpio2Dir & bitSet) == GPIO_DIR_OUT)
                return 0;
            break;
        
        case 3:
            if (Gpio3Level & bitSet)
            {
                *pLevel = GPIO_LEVEL_HI;
            }    
            else 
            {
                *pLevel = GPIO_LEVEL_LO;
            }
            if ((Gpio3Dir & bitSet) == GPIO_DIR_OUT)
                return 0;
            break;
        
        default:
            return 0;
    }       

    return 1;       
}

/*

Routine Description:

    Set the enable of general purpose I/O.

Arguments:

    group - Group number.
    number - Pin number.
    isEnable - enable of pin.

Return Value:

    none

*/
void gpioSetEnable(u8 group, u8 pin, u8 isEnable)
{
    u32 bitSet = 0x00000001 << pin; 
    
    if (pin >= GPIO_PIN_COUNT)
        return;

    switch (group)
    {
        case 0:
            if (isEnable == GPIO_DISA)
                Gpio0Ena&= ~bitSet; 
            else    /* GPIO_ENA */
                Gpio0Ena|= bitSet;            
            break;
        
        case 1:
            if (isEnable == GPIO_DISA)
                Gpio1Ena&= ~bitSet; 
            else    /* GPIO_ENA */
                Gpio1Ena|= bitSet; 
            break;
        
        case 2:
            if (isEnable == GPIO_DISA)
                Gpio2Ena&= ~bitSet; 
            else    /* GPIO_ENA */
                Gpio2Ena|= bitSet; 
            break;
        
        case 3:
            if (isEnable == GPIO_DISA)
                Gpio3Ena&= ~bitSet; 
            else    /* GPIO_ENA */
                Gpio3Ena|= bitSet; 
            break;
        
    }    
  
}


void gpioSetInt(u8 group, u8 pin, u8 Enable)
{
    if (pin >= GPIO_PIN_COUNT)
        return;

    if(Enable)
        gpio0IntCfg[pin].intEna = GPIO_INT_ENA;
    else
        gpio0IntCfg[pin].intEna = GPIO_INT_DISA;
    gpioIntConfig(group, pin, &gpio0IntCfg[pin]);
}

/*

Routine Description:

    Enable/ Disable gpio interrupt.

Arguments:

    pin - Pin number of gpio.
    stat - "1" for enable, "0" for disable.

Return Value:

    None.

*/
void gpioSetIntr(u8 pin, u8 stat)
{
    u32 unBit = 1;


    if (pin >= GPIO_PIN_COUNT)
        return;

    unBit <<= pin;

    if (stat == 1)
    {
        /* Enable gpio intr */
        Gpio0IntEna |= unBit;
    }
    else
    {
        /* Disable gpio intr */
        Gpio0IntEna &= ~unBit;
    }

}

/*

Routine Description:

    Get Gpio Level.

Arguments:

    ucGrp - Group to be checked.
    ucPin - Pin to be checked in the group.

Return Value:

    ucLevel - The detected level.

*/
u8  gpioCheckLevel(u8 ucGrp, u8 ucPin)
{

    u8  ucLevel;

    gpioGetLevel(ucGrp, ucPin, &ucLevel);

    return ucLevel;

}
void gpioDebug(u32 value)
{
}

void gpioDebug1(u32 value)
{
}

void gpioDebug2(u32 value)
{
}

void gpioDebug12(u32 value)
{
}

void gpioDebug123(u32 value)
{
}


// for K310 debug
void gpioDebugFIQ(u32 value)
{
}

// for K310 debug
void gpioDebugIRQ(u32 value)
{

}

void gpioeBellAll(u32 value)
{
}

void gpioeBellIRQ(u32 value)
{
}


#if GPIO_I2C_ENA

void gpio_IIC_W_Byte(GPIO_IIC_CFG *stGPIO_IIC, u32 Value)
{
    s32 i;
    
    for(i = 7; i >= 0; i--)
    {
        gpio_IIC_SCK_W(stGPIO_IIC->GrpSCK, stGPIO_IIC->BitSCK, 0);
        gpio_IIC_SDA_W(stGPIO_IIC->GrpSDA, stGPIO_IIC->BitSDA, (Value >> i) & 1);
        gpio_IIC_SCK_W(stGPIO_IIC->GrpSCK, stGPIO_IIC->BitSCK, 1);
        gpio_IIC_SCK_W(stGPIO_IIC->GrpSCK, stGPIO_IIC->BitSCK, 1);  /* for delay to match duty cycle */     
        gpio_IIC_SCK_W(stGPIO_IIC->GrpSCK, stGPIO_IIC->BitSCK, 0);
    }
    gpio_IIC_SDA_W(stGPIO_IIC->GrpSDA, stGPIO_IIC->BitSDA, 0);
}

void gpio_IIC_R_Byte(GPIO_IIC_CFG * stGpio_Iic, u8 *pucValue)
{
    u8  i, data, data1;
    
    /* set SDA gpio input */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir |= (1<<stGpio_Iic->BitSDA);
            break;
    }
    
    data    = 0;
    data1   = 0;
    for(i = 8; i > 0; i--)
    {
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
        gpio_IIC_SDA_R(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, &data);
        data1   = (data1 << 1 | data);
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
    }
    *pucValue  = data1;

    /* set SDA gpio output */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
    }

}

s8 gpio_IIC_Ack_R(GPIO_IIC_CFG * stGpio_Iic)
{
    u8  data;
    u32 i;

    /* set SDA gpio input */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir |= (1<<stGpio_Iic->BitSDA);
            break;
    }

    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);

    /* Set and hold SCK to be HIGH for delay */
    for(i=0; i<3; i++)
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);


    for(i = 0, data = 1; (i < 1000000) && data; i++)
    {
        gpio_IIC_SDA_R(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, &data);
    }
    if(data)
    {
        DEBUG_GPIO("Error: gpio_IIC_Ack_R() time out!!!\n");
        return 0;
    }

    /* Set and hold SCK to be Low for delay */
    for(i=0; i<3; i++)
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);


    /* set SDA gpio output */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;

    }
	//DEBUG_SDC("gpio_IIC_Ack_R Gpio1Dir=0x%x\n",Gpio1Dir);

    return 1;

}

s8 gpio_IIC_nAck_R(GPIO_IIC_CFG * stGpio_Iic)
{
    u8  data;
    u32 i;

    /* set SDA gpio input */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir |= (1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir |= (1<<stGpio_Iic->BitSDA);
            break;
    }

    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);

    /* Set and hold SCK to be HIGH for delay */
    for(i=0; i<3; i++)
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);


    for(i = 0, data = 0; (i < 1000000) && data; i++)
    {
        gpio_IIC_SDA_R(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, &data);
    }
    if(data)
    {
        DEBUG_GPIO("Error: gpio_IIC_nAck_R() time out!!!\n");
        return 0;
    }

    /* Set and hold SCK to be Low for delay */
    for(i=0; i<3; i++)
        gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);


    /* set SDA gpio output */
    switch(stGpio_Iic->GrpSDA)
    {
        case 0:
            Gpio0Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 1:
            Gpio1Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 2:
            Gpio2Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;
        
        case 3:
            Gpio3Dir &= ~(1<<stGpio_Iic->BitSDA);
            break;

    }
	//DEBUG_SDC("gpio_IIC_nAck_R Gpio1Dir=0x%x\n",Gpio1Dir);

    return 1;

}

void gpio_IIC_Ack_W(GPIO_IIC_CFG * stGpio_Iic)
{
    gpio_IIC_SDA_W(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
    gpio_IIC_SDA_W(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, 1);
}

void gpio_IIC_nAck_W(GPIO_IIC_CFG * stGpio_Iic)
{
    gpio_IIC_SDA_W(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic->GrpSCK, stGpio_Iic->BitSCK, 0);
    gpio_IIC_SDA_W(stGpio_Iic->GrpSDA, stGpio_Iic->BitSDA, 0);
}

void gpio_IIC_Enable(GPIO_IIC_CFG ucDevCFG)
{

    u32 unBit = 0;

	unBit = 1 << ucDevCFG.BitSCK;
    switch(ucDevCFG.GrpSCK)
    {
        case 0:
            Gpio0Dir   &= ~unBit;
            Gpio0Level |= unBit;
            Gpio0Ena   |= unBit;
            break;
        case 1:
            Gpio1Dir   &= ~unBit;
            Gpio1Level |= unBit;
            Gpio1Ena   |= unBit;
            break;
        case 2:
            Gpio2Dir   &= ~unBit;
            Gpio2Level |= unBit;
            Gpio2Ena   |= unBit;
            break;
        case 3:
            Gpio3Dir   &= ~unBit;
            Gpio3Level |= unBit;
            Gpio3Ena   |= unBit;
            break;
        default:
            DEBUG_GPIO("GPIO I2C Group Config error \n");
            break;
        
    }

	unBit = 1 << ucDevCFG.BitSDA;
    switch(ucDevCFG.GrpSDA)
    {
        case 0:
            Gpio0Dir   &= ~unBit;
            Gpio0Level |= unBit;
            Gpio0Ena   |= unBit;
            break;
        case 1:
            Gpio1Dir   &= ~unBit;
            Gpio1Level |= unBit;
            Gpio1Ena   |= unBit;
            break;
        case 2:
            Gpio2Dir   &= ~unBit;
            Gpio2Level |= unBit;
            Gpio2Ena   |= unBit;
            break;
        case 3:
            Gpio3Dir   &= ~unBit;
            Gpio3Level |= unBit;
            Gpio3Ena   |= unBit;
            break;
        default:
            DEBUG_GPIO("GPIO I2C Pin Config error \n");
            break;
        
    }


}

/*

Routine Description:

    Use gpio to simulate IIC protocol to write data.

Arguments:

    ucDevIdx - Device index to write.
    unDevAddr - Device slave addr to write.
    ucDevRegAddr - Device register addr to write.
    ucData - Data to write to register.

Return Value:

    1 => Success.
    0 => Failure

*/
s8 gpio_IIC_Write(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 ucData)
{
    GPIO_IIC_CFG    stGpio_Iic;
    u32 i;

    switch(ucDevIdx)
    {
        case IIC_DEV_TVP5150:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_TVP5150, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_PT2257:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_PT2257, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_BIT1605:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1605, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_OV7740:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7740, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_OV7725:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7725, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_H30CD:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_H30CD, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_BIT1201G:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1201G, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_CS8556:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));
            break;

    }

    /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);


    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, unDevAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C Write data */
    gpio_IIC_W_Byte(&stGpio_Iic, ucData);
    /* Acknowledge */
    gpio_IIC_Ack_R(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
    {
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    }
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);        /* for delay */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic); 

    return 1;

RECOVER_GPIO:
    
    gpio_IIC_Disable(stGpio_Iic);
    return 0;

}

/*

Routine Description:

    Use gpio to simulate IIC protocol to read data.

Arguments:

    ucDevIdx - Device index to read.
    unDevAddr - Device slave addr to read.
    ucDevRegAddr - Device register addr to read.
    pucData - Data pointer to read from register.

Return Value:

    1 => Success.
    0 => Failure

*/
s8 gpio_IIC_Read(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData)
{
    GPIO_IIC_CFG    stGpio_Iic;
    u32 ucDevReadAddr;
    u32 i;

    switch(ucDevIdx)
    {
        case IIC_DEV_TVP5150:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_TVP5150, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_PT2257:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_PT2257, sizeof (GPIO_IIC_CFG));
            break;
 
        case IIC_DEV_BIT1605:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1605, sizeof (GPIO_IIC_CFG));
            break;            

        case IIC_DEV_OV7740:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7740, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_OV7725:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7725, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_H30CD:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_H30CD, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_BIT1201G:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1201G, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_CS8556:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));
            break;
    }

    
    
    /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, unDevAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    if(ucDevIdx != IIC_DEV_BIT1201G )
    {
        /* Stop */
        gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
        gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);  
    }
    
    
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    ucDevReadAddr = unDevAddr | 0x00000001;
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevReadAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData);
    /* Inverse Acknowledge */
    gpio_IIC_nAck_W(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    return 1;

RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    return 0;


}

/*

Routine Description:

    Use gpio to simulate IIC protocol to read one word data.

Arguments:

    ucDevIdx - Device index to read.
    unDevAddr - Device slave addr to read.
    ucDevRegAddr - Device register addr to read.
    pucData - Data pointer to read from register.

Return Value:

    1 => Success.
    0 => Failure

*/
s8 gpio_IIC_Read_Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData)
{
    GPIO_IIC_CFG    stGpio_Iic;
    u32 ucDevReadAddr;
    u32 i;

    switch(ucDevIdx)
    {
        case IIC_DEV_TVP5150:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_TVP5150, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_PT2257:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_PT2257, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_BIT1605:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1605, sizeof (GPIO_IIC_CFG));
            break;            

        case IIC_DEV_OV7740:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7740, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_OV7725:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7725, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_H30CD:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_H30CD, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_CS8556:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));
            break;
    }

    /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, unDevAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    ucDevReadAddr = unDevAddr | 0x00000001;
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevReadAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 3);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 2);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 1);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 0);
    /* Inverse Acknowledge */
    gpio_IIC_nAck_W(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    return 1;

RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    return 0;


}

/*

Routine Description:

    Use gpio to simulate IIC protocol to read two word data.

Arguments:

    ucDevIdx - Device index to read.
    unDevAddr - Device slave addr to read.
    ucDevRegAddr - Device register addr to read.
    pucData - Data pointer to read from register.

Return Value:

    1 => Success.
    0 => Failure

*/
s8 gpio_IIC_Read_2Word(u8 ucDevIdx, u32 unDevAddr, u8 ucDevRegAddr, u8 *pucData)
{
    GPIO_IIC_CFG    stGpio_Iic;
    u32 ucDevReadAddr;
    u32 i;

    switch(ucDevIdx)
    {
        case IIC_DEV_TVP5150:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_TVP5150, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_PT2257:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_PT2257, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_BIT1605:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_BIT1605, sizeof (GPIO_IIC_CFG));
            break;            

        case IIC_DEV_OV7740:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7740, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_OV7725:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_OV7725, sizeof (GPIO_IIC_CFG));
            break;

        case IIC_DEV_H30CD:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_H30CD, sizeof (GPIO_IIC_CFG));
            break;
        case IIC_DEV_CS8556:
            memcpy(&stGpio_Iic, &GPIO_IIC_CFG_CS8556, sizeof (GPIO_IIC_CFG));
            break;
    }

    /* Enable IIC and set gpio */
    gpio_IIC_Enable(stGpio_Iic);
    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, unDevAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* I2C Write register address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevRegAddr);
    /* Acknowledge */
    if (!gpio_IIC_Ack_R(&stGpio_Iic))
        goto RECOVER_GPIO;
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* Stop */
    /*
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    */

    /* Start */
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    ucDevReadAddr = unDevAddr | 0x00000001;
    /* I2C General address */
    gpio_IIC_W_Byte(&stGpio_Iic, ucDevReadAddr);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 3);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 2);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 1);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 0);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 7);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 6);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 5);
    /* Acknowledge */
    gpio_IIC_Ack_W(&stGpio_Iic);
    /* I2C read data */
    gpio_IIC_R_Byte(&stGpio_Iic, pucData + 4);
    /* Inverse Acknowledge */
    gpio_IIC_nAck_W(&stGpio_Iic);
    /* delay max 64 us */
    for(i = 0; i < 10; i++)
        gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);

    /* Stop */
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 0);
    gpio_IIC_SCK_W(stGpio_Iic.GrpSCK, stGpio_Iic.BitSCK, 1);
    gpio_IIC_SDA_W(stGpio_Iic.GrpSDA, stGpio_Iic.BitSDA, 1);
    gpio_IIC_Disable(stGpio_Iic);
    return 1;

RECOVER_GPIO:

    gpio_IIC_Disable(stGpio_Iic);
    return 0;


}

void gpio_IIC_Disable(GPIO_IIC_CFG ucDevCFG)
{
    u32 unBit = 0;;

    unBit = 1 << ucDevCFG.BitSCK;
    switch(ucDevCFG.GrpSCK)
    {

        case 0:
            Gpio0Ena   &= ~unBit;
            break;
        case 1:
            Gpio1Ena   &= ~unBit;
            break;
        case 2:
            Gpio2Ena   &= ~unBit;
            break;
        case 3:
            Gpio3Ena   &= ~unBit;
            break;
        default:
            DEBUG_GPIO("GPIO I2C Group Config error\n");
            break;
    }

	unBit = 1 << ucDevCFG.BitSDA;
    switch(ucDevCFG.GrpSDA)
    {

        case 0:
            Gpio0Ena   &= ~unBit;
            break;
        case 1:
            Gpio1Ena   &= ~unBit;
            break;
        case 2:
            Gpio2Ena   &= ~unBit;
            break;
        case 3:
            Gpio3Ena   &= ~unBit;
            break;
        default:
            DEBUG_GPIO("GPIO I2C PIN Config error\n");
            break;
    }


}


#endif



void gpio_SensorInSwithc2TVIn()
{
    

}

    void gpio_TVInSwithc2SensorIn()
    {
        sysVideoInSel = VIDEO_IN_SENSOR;

    }

s32 gpioExpandSetLevel(u8 registAddr, u8 pin, u8 level)
{
#if (IO_EXPAND == IO_EXPAND_WT6853)
    u8 data =0;
    u8 bitSet = 0x1 << pin;

    i2cRead_WT6853(registAddr, &data);
    DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    if (level == GPIO_LEVEL_HI)
        data &= ~bitSet; 
    else    /* GPIO_LEVEL_HI */
        data |= bitSet; 
    i2cWrite_WT6853(registAddr, data);
    DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    i2cRead_WT6853(registAddr, &data);
    DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
#endif

    return 1;       
}

s32 gpioExpandGetLevel(u8 registAddr, u8 pin, u8* plevel)
{
#if (IO_EXPAND == IO_EXPAND_WT6853)
    u8 data =0;
    u8 bitSet = 0x1 << pin; 

    i2cRead_WT6853(registAddr, &data);
    DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    if (data & bitSet)
    {
        *plevel = GPIO_LEVEL_LO;
    }    
    else 
    {
        *plevel = GPIO_LEVEL_HI;
    }
#endif

    return 1;
}

