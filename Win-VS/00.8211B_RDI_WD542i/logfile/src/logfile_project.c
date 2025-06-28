/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	logfile_project.c

Abstract:

   	The routines of log file project function

Environment:

    	ARM RealView Developer Suite

Revision History:

	2011/02/23	Peter Hsu	Create

*/

#include "general.h"
#if(HW_BOARD_OPTION==ELEGANT_KFCDVR)
#include "board.h"
#include "task.h"
#include "sysapi.h"
#include "siuapi.h"
#include "uiapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "iduapi.h"
#include "jpegapi.h"
#include "mp4api.h"
#include "asfapi.h"
#include "movapi.h"
#include "timerapi.h"
#include "usbapi.h"
#include "aviapi.h" /* Peter 0704 */
#include "mpeg4api.h"
#include "iisapi.h"
#include "ispapi.h" /*CY 1023*/
#include "gpioapi.h"
#include "adcapi.h"
#include "uartapi.h"
#include "ClockSwitchApi.h"
#include "awbapi.h"
#include "smcapi.h"
#include "spiapi.h"
#include "gpsapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "usbapi.h"
#include "uikey.h"
#include "logfileapi.h"

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

u8  szVideoOverlay1[MAX_OVERLAYSTR];
u8  szVideoOverlay2[MAX_OVERLAYSTR];
u8  LogCounter;

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
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


#if CDVR_LOG
void AppendLogString(u8 *LogString)
{
    u8              err;
    s32             StrLen;

    OSSemPend(LogFileSemEvt, 10, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SYS("sysDrawTimeOnVideoClip() Error: LogFileSemEvt is %d.\n", err);
    }
    strcpy(szLogFile, LogString);
    StrLen      = strlen(LogString);
    strcpy(szLogFile + StrLen, "\r\n");
    szLogFile  += StrLen + 2;
    if(szLogFile >= (LogFileBufEnd - MAX_OVERLAYSTR * 2))
    {
        pLogFileMid                     = szLogFile;
        szLogFile                       = LogFileBuf;
    }
    OSSemPost(LogFileSemEvt);
}
#endif

s32 GenerateLogString(void)
{
#if ISU_OVERLAY_ENABLE
    RTC_DATE_TIME   curDateTime;
    //u8              szVideoOverlay1[MAX_OVERLAYSTR];
    //static  u8      szVideoOverlay2[MAX_OVERLAYSTR];
    u8              cF;
  #if (G_SENSOR == G_SENSOR_LIS302DL)
    s8              out_X, out_Y, out_Z, out_X1, out_Y1, out_Z1;
    u8              X, Y, Z, signX, signY, signZ;
  #elif (G_SENSOR == G_SENSOR_H30CD)
    s16             out_X, out_Y, out_Z, out_X1, out_Y1, out_Z1;
    s16             X, Y, Z;
    u8              signX, signY, signZ;
  #endif
    s16             Temp;
    s8              LeftLight, BrakeLight, RightLight;
    s8              PowerSwitch, BatteryDetect;
    u32             Speed;
    //const   s8      Floating[16]    = {0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9};
    const   u8      Floating[16][3] = {"00", "06", "13", "19", "25", "31", "38", "44", "50", "56", "63", "69", "75", "81", "88", "94"};
    //static  u8      LogCounter;
    static  u32     Counter     = 0;
    static  u8      Counter1    = 0;
  #if UART_GPS_COMMAND
    static  u8      szGPS[48];
    u8              err;
  #endif
    
    Counter++;
    RTC_Get_Time(&curDateTime);
    sprintf (timeForRecord1, "20%02d/%02d/%02d %02d:%02d:%02d", curDateTime.year, curDateTime.month, curDateTime.day, curDateTime.hour, curDateTime.min, curDateTime.sec);
    LogCounter++;
    if (strcmp(timeForRecord1, timeForRecord2))
    {
    #if CDVR_LOG
        while(Counter > 40 && LogCounter < 30)
        {
            AppendLogString(szVideoOverlay1);
            LogCounter++;
            DEBUG_SYS("$");
        }
        //DEBUG_SYS("#%d ", LogCounter);
        LogCounter  = 0;
    #endif
        strcpy(timeForRecord2, timeForRecord1);

  #if UART_GPS_COMMAND
        if (gGPS_data1.valid == 1)
        {
            //copy data to 
            //DEBUG_SYS("T");
            OSSemPend(GPSUpdateEvt, 5, &err);
            memcpy(&gGPS_data2, &gGPS_data1, sizeof(GPS_DATA));
            gGPS_data1.valid    = 0;
            OSSemPost(GPSUpdateEvt);
        }

        if  (gGPS_data2.valid)
        {
            //DEBUG_SYS("O");
            gGPS_data2.valid    = 0;
            sprintf(szGPS, "%c%02d %02d.%03d %c%03d %02d.%03d",
                    gGPS_data2.N_S, (gGPS_data2.Lat_I /100) & 0xff, (gGPS_data2.Lat_I %100) & 0xff, gGPS_data2.Lat_F/10,
                    gGPS_data2.E_W, (gGPS_data2.Lon_I/100) & 0xff, (gGPS_data2.Lon_I %100) & 0xff, gGPS_data2.Lon_F/10);
            DEBUG_SYS("%s\n", szGPS);
        }
        else if(strlen(szGPS) == 0)
        {
            sprintf(szGPS,"%c%02d %02d.%03d %c%03d %02d.%03d",
                    'N', 0, 0, 0,
                    'E', 0, 0, 0);
            DEBUG_SYS("%s\n", szGPS);
        }
  #endif
    }
    if(LogCounter >= 30)
        return  0;

  #if (G_SENSOR == G_SENSOR_LIS302DL)
    i2cGet_LIS302DL_XYZ(&out_X1, &out_Y1, &out_Z1);
    Temp    = (s16)out_X1 - GXOffset;
    out_X1  = (Temp > 127) ? 127 : (Temp < -128) ? -128 : Temp;
    Temp    = (s16)out_Y1 - GYOffset;
    out_Y1  = (Temp > 127) ? 127 : (Temp < -128) ? -128 : Temp;
    Temp    = (s16)out_Z1 - GZOffset;
    out_Z1  = (Temp > 127) ? 127 : (Temp < -128) ? -128 : Temp;
    switch (GDirection)
    {
        case 1:
            out_X   = -out_Y1;
            out_Y   = out_X1;
            out_Z   = out_Z1;
            break;
        case 0:
        case 2:
            out_X   = out_X1;
            out_Y   = out_Y1;
            out_Z   = out_Z1;
            break;
        case 3:
            out_X   = out_Y1;
            out_Y   = -out_X1;
            out_Z   = out_Z1;
            break;
        case 4:
            out_X   = -out_X1;
            out_Y   = -out_Y1;
            out_Z   = out_Z1;
            break;
    }
    /*
    X       = abs(out_X);
    Y       = abs(out_Y);
    Z       = abs(out_Z);
    signX   = (out_X >= 0) ? '+' : '-';
    signY   = (out_Y >= 0) ? '+' : '-';
    signZ   = (out_Z >= 0) ? '+' : '-';
    */
    if(out_X >= 0)
    {
        X       = out_X;
        signX   = '+';
    } else {
        X       = -out_X;
        signX   = '-';
    }
    if(out_Y >= 0)
    {
        Y       = out_Y;
        signY   = '+';
    } else {
        Y       = -out_Y;
        signY   = '-';
    }
    if(out_Z >= 0)
    {
        Z       = out_Z;
        signZ   = '+';
    } else {
        Z       = -out_Z;
        signZ   = '-';
    }
  #elif (G_SENSOR == G_SENSOR_H30CD)
    #if 1   // Get G sensor value by here
    i2cGet_H30CD_XYZ(&out_X1, &out_Y1, &out_Z1);
    #else   // Get G sensor value by Timer
    out_X1  = GValueX;
    out_Y1  = GValueY;
    out_Z1  = GValueZ;
    #endif
    //DEBUG_I2C("H30CD(X, Y, Z) = (%5d, %5d, %5d)\n", out_X1, out_Y1, out_Z1);
    //DEBUG_I2C("          \r");
    Temp    = (s16)out_X1 - GXOffset;
    out_X1  = (Temp > 2047) ? 2047 : (Temp < -2048) ? -2048 : Temp;
    Temp    = (s16)out_Y1 - GYOffset;
    out_Y1  = (Temp > 2047) ? 2047 : (Temp < -2048) ? -2048 : Temp;
    Temp    = (s16)out_Z1 - GZOffset;
    out_Z1  = (Temp > 2047) ? 2047 : (Temp < -2048) ? -2048 : Temp;
    switch (GDirection)
    {
        case 1:
            out_X   = -out_Y1;
            out_Y   = out_X1;
            out_Z   = out_Z1;
            break;
        case 0:
        case 2:
            out_X   = out_X1;
            out_Y   = out_Y1;
            out_Z   = out_Z1;
            break;
        case 3:
            out_X   = out_Y1;
            out_Y   = -out_X1;
            out_Z   = out_Z1;
            break;
        case 4:
            out_X   = -out_X1;
            out_Y   = -out_Y1;
            out_Z   = out_Z1;
            break;
    }
    if(out_X >= 0)
    {
        X       = out_X;
        signX   = '+';
    } else {
        X       = -out_X;
        signX   = '-';
    }
    if(out_Y >= 0)
    {
        Y       = out_Y;
        signY   = '+';
    } else {
        Y       = -out_Y;
        signY   = '-';
    }
    if(out_Z >= 0)
    {
        Z       = out_Z;
        signZ   = '+';
    } else {
        Z       = -out_Z;
        signZ   = '-';
    }
    {
        static u8   PrintString = 0;
        /*
        if((X < 10 && Y < 10 && Z < 10) || (PrintString > 0 && PrintString < 5)) {
            if(PrintString == 0)
            {
                DEBUG_SYS("H30CD(X, Y, Z) = (%5d, %5d, %5d)\n", out_X, out_Y, out_Z);
                DEBUG_SYS("Reset H30CD...\n", out_X, out_Y, out_Z);
                gpioSetDir(0, 14, 0);   // output
                gpioSetLevel(0, 14, 0);
                PrintString = 1;
            } else if(PrintString == 1) {
                gpioSetLevel(0, 14, 1);
                //gpioSetLevel(0, 14, 0);
                //gpioSetLevel(0, 14, 1);
                gpioSetDir(0, 14, 1);   // input
                //i2cInit_H30CD();
                i2cWrite_H30CD(0x0f, 0x00);     // Temporal CTRL_REG0
                PrintString = 2;
            } else if(PrintString == 2) {
                i2cWrite_H30CD(0x14, 0x00);     // Temporal CTRL_REG1
                PrintString = 3;
            } else if(PrintString == 3) {
                i2cWrite_H30CD(0x15, 0x00);     // Temporal CTRL_REG2
                PrintString = 4;
            } else if(PrintString == 4) {
                i2cWriteCommand_H30CD(2);       // R_Mode
                PrintString = 5;
            } else if(PrintString == 5) {
                PrintString = 0;
            }
        } else if(PrintString == 5) {
            DEBUG_SYS("Success!!!\n");
            PrintString = 0;
        }
        */
        if((X < 10 && Y < 10 && Z < 10)) {
            if(PrintString == 0)
            {
                DEBUG_SYS("H30CD(X, Y, Z) = (%5d, %5d, %5d)\n", out_X, out_Y, out_Z);
                DEBUG_SYS("Reset H30CD...\n", out_X, out_Y, out_Z);
                PrintString = 1;
            }
            gpioSetDir(0, 14, 0);   // output
            gpioSetLevel(0, 14, 0);
            gpioSetLevel(0, 14, 1);
            gpioSetLevel(0, 14, 0);
            gpioSetLevel(0, 14, 1);
            gpioSetDir(0, 14, 1);   // input
            //i2cInit_H30CD();
            i2cWrite_H30CD(0x0f, 0x00);     // Temporal CTRL_REG0
            i2cWrite_H30CD(0x14, 0x00);     // Temporal CTRL_REG1
            i2cWrite_H30CD(0x15, 0x00);     // Temporal CTRL_REG2
            i2cWriteCommand_H30CD(2);       // R_Mode
        } else if(PrintString == 1) {
            DEBUG_SYS("Success!!!\n");
            PrintString = 0;
        }
    }
  #endif    // #if (G_SENSOR == G_SENSOR_LIS302DL)
    gpioGetLevel(0, 7, &RightLight);        // 0: turn on, 1: turn off
    gpioGetLevel(0, 8, &LeftLight);         // 0: turn on, 1: turn off
    gpioGetLevel(0, 9, &BrakeLight);        // 0: turn on, 1: turn off
    gpioGetLevel(0, 3, &PowerSwitch);       // Power Switch, 0: On, 1: Off
    gpioGetLevel(0, 24, &BatteryDetect);    // Battery Detect, 0: normal, 1: no power
    //if(PowerSwitch || BatteryDetect || Test_Mode >= 2)        // if power switch off or battery power lose, close file.
    if(PowerSwitch || BatteryDetect)        // if power switch off or battery power lose, close file.
    {
        static  u8  PrintPowerSwitch    = 0;
        static  u8  PrintBatteryDetect  = 0;
        //static  u8  PrintTest_Mode      = 0;
        if(PowerSwitch && !PrintPowerSwitch)
        {
            DEBUG_SYS("Power switch off\n");
            PrintPowerSwitch    = 1;
        }
        if(BatteryDetect && !PrintBatteryDetect)
        {
            DEBUG_SYS("Battery power lose\n");
            PrintBatteryDetect  = 1;
        }
        /*
        if(Test_Mode >= 2 && !PrintTest_Mode)
        {
            DEBUG_SYS("Test mode finish!!!\n");
            PrintTest_Mode      = 1;
        }
        */
        if((asfVopCount > 10 && asfAudiChunkCount > 12))
        {
            sysCaptureVideoStop     = 1;
            sysCaptureVideoStart    = 0;
        }
    }

  #if UART_GPS_COMMAND
    Speed   = gGPS_data2.Speed_I;
  #else
    //Speed   = SpeedCounter * 2 * 3600 / (637 * Speed_Pulse); // 速度[km/h]=(1秒間??生????×3600[秒])÷(637×車速???)
    //Speed   = SpeedCounter * 2 * 3600 * 256 / (637 * Speed_Pulse); // 速度[km/h]=(1秒間??生????×3600[秒])÷(637×車速???)
    Speed   = SpeedCounter * 2 * 3600 * 256 / (SpeedRPM * Speed_Pulse); // 速度[km/h]=(1秒間??生????×3600[秒])÷(637×車速???)
  #endif
    //cF      = ((abs(out_X) > GLimit) || (abs(out_Y) > GLimit) || (abs(out_Z) > GLimit)) ? 'F' : ' ';
    cF      = ((X > GLimitX) || (Y > GLimitY) || (Z > GLimitZ)) ? 'F' : ' ';
  #if (G_SENSOR == G_SENSOR_LIS302DL)
    //sprintf (szVideoOverlay1, "%s   %3dKM/H    %c %c %c  (%4d,%4d,%4d)       ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', out_X, out_Y, out_Z);
    //sprintf (szVideoOverlay1, "%s   %3dKM/H    %c %c %c  (%+2.1fG,%+2.1fG,%+2.1fG)       ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', (float)out_X / 16, (float)out_Y / 16, (float)out_Z / 16);
    //sprintf (szVideoOverlay1, "%s   %3dKM/H    %c %c %c  (%+2d.%1dG,%+2d.%1dG,%+2d.%1dG)       ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', out_X / 16, Floating[((out_X >= 0) ? out_X : -(s16)out_X) & 0xf], out_Y / 16, Floating[((out_Y >= 0) ? out_Y : -(s16)out_Y) & 0xf], out_Z / 16, Floating[((out_Z >= 0) ? out_Z : -(s16)out_Z) & 0xf]);
    //sprintf (szVideoOverlay1, "%s   %3dKM/H   %c %c %c  (%+2d.%1dG,%+2d.%1dG,%+2d.%1dG) %c%c%c    ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', out_X / 16, Floating[((out_X >= 0) ? out_X : -(s16)out_X) & 0xf], out_Y / 16, Floating[((out_Y >= 0) ? out_Y : -(s16)out_Y) & 0xf], out_Z / 16, Floating[((out_Z >= 0) ? out_Z : -(s16)out_Z) & 0xf], (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF);
    #if UART_GPS_COMMAND
    sprintf (szVideoOverlay1, "%s   %3dkm/h   %c%c%c (%c%d.%sG,%c%d.%sG,%c%d.%sG) %c%c%c    %s", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', signX, X >> 4, Floating[X & 0xf], signY, Y >> 4, Floating[Y & 0xf], signZ, Z >> 4, Floating[Z & 0xf], (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF, szGPS);
    #else
    sprintf (szVideoOverlay1, "%s   %3dkm/h   %c%c%c (%c%d.%sG,%c%d.%sG,%c%d.%sG) %c%c%c    ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', signX, X >> 4, Floating[X & 0xf], signY, Y >> 4, Floating[Y & 0xf], signZ, Z >> 4, Floating[Z & 0xf], (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF);
    #endif
  #elif (G_SENSOR == G_SENSOR_H30CD)
    #if UART_GPS_COMMAND
    sprintf (szVideoOverlay1, "%s   %3dkm/h   %c%c%c (%c%d.%02dG,%c%d.%02dG,%c%d.%02dG) %c%c%c    %s", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', signX, X / 1000, (X / 10) % 100, signY, Y / 1000, (Y / 10) % 100, signZ, Z / 1000, (Z / 10) % 100, (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF, szGPS);
    #else
    sprintf (szVideoOverlay1, "%s   %3dkm/h   %c%c%c (%c%d.%02dG,%c%d.%02dG,%c%d.%02dG) %c%c%c    ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', signX, X / 1000, (X / 10) % 100, signY, Y / 1000, (Y / 10) % 100, signZ, Z / 1000, (Z / 10) % 100, (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF);
    //sprintf (szVideoOverlay1, "%s   %3dkm/h   %c%c%c (%6d,%6d,%6d) %c%c%c    ", timeForRecord1, Speed, LeftLight ? ' ' : '<', BrakeLight ? ' ' : '=', RightLight ? ' ' : '>', out_X, out_Y, out_Z, (Speed > SpeedLimit) ? 'S' : ' ', (Speed == 0) ? 'I' : ' ', cF);
    #endif
  #endif    // #if (G_SENSOR == G_SENSOR_LIS302DL)

  #if CDVR_LOG
    AppendLogString(szVideoOverlay1);
  #endif

    if((Counter1 & 3) == 0)   // Check RTC per 4 frame for reduce background task loading
        sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
    Counter1++;

#endif  // #if ISU_OVERLAY_ENABLE

    return  1;
}



#endif  // #if(HW_BOARD_OPTION==ELEGANT_KFCDVR)
