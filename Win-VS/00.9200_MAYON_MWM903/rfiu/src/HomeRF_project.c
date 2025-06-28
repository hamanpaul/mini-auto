/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    HomRF.c

Abstract:

    The routines of Home Sensor Interface Unit.
    
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2014/09/16    Roy Create  
*/




#include "general.h"
#include "rfiuapi.h"
#if HOME_RF_SUPPORT

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */


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

u8 homeRFGetBattery(u32 adcVal)
{
    u8 battery;

    if(adcVal < 0x0f29)      /* 3.0V*/
        battery=100;    
    else if(adcVal < 0x0f2a) /* 2.9V*/
        battery=90;    
    else if(adcVal < 0x0f2c) /* 2.8V*/
        battery=80;    
    else if(adcVal < 0x0f2e) /* 2.7V*/
        battery=70;    
    else if(adcVal < 0x0f31) /* 2.6V*/
        battery=60;    
    else if(adcVal < 0x0f33) /* 2.5V*/
        battery=50;    
    else if(adcVal < 0x0f36) /* 2.4V*/
        battery=40;    
    else if(adcVal < 0x0f39) /* 2.3V*/
        battery=30;    
    else if(adcVal < 0x0f3b) /* 2.2V*/
        battery=20;    
    else                    
        battery=10; 

    return battery;        
}

u8 homeRFGetBattery_TRANWO_PIR(u32 adcVal)
{
    u8 battery;

    if(adcVal > 0x0fff)      /* 3.0V*/
        battery=100;    
    else if(adcVal > 0x0f7e) /* 2.9V*/
        battery=90;    
    else if(adcVal > 0x0f00) /* 2.8V*/
        battery=80;    
    else if(adcVal > 0x0e90) /* 2.7V*/
        battery=70;    
    else if(adcVal > 0x0e20) /* 2.6V*/
        battery=60;    
    else if(adcVal > 0x0d97) /* 2.5V*/
        battery=50;    
    else if(adcVal > 0x0d08) /* 2.4V*/
        battery=40;    
    else if(adcVal > 0x0c7e) /* 2.3V*/
        battery=30;    
    else if(adcVal > 0x0bf8) /* 2.2V*/
        battery=20;    
    else                    
        battery=10; 

    return battery;        
}

u8 homeRFGetBattery_TRANWO_DOOR(u32 adcVal)
{
    u8 battery;

    if(adcVal > 0x0c50)      /* 3.0V*/
        battery=100;    
    else if(adcVal > 0x0bf8) /* 2.9V*/
        battery=90;    
    else if(adcVal > 0x0b98) /* 2.8V*/
        battery=80;    
    else if(adcVal > 0x0b3f) /* 2.7V*/
        battery=70;    
    else if(adcVal > 0x0ae8) /* 2.6V*/
        battery=60;    
    else if(adcVal > 0x0a88) /* 2.5V*/
        battery=50;    
    else if(adcVal > 0x0a20) /* 2.4V*/
        battery=40;    
    else if(adcVal > 0x09b8) /* 2.3V*/
        battery=30;    
    else if(adcVal > 0x0950) /* 2.2V*/
        battery=20;    
    else                    
        battery=10; 

    return battery;        
}




#endif  /* end #if (HOME_RF_SUPPORT )  */
