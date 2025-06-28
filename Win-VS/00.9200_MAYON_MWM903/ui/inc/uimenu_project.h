/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	osdlib.h

Abstract:

   	The declarations of user interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2008/05/21	CC Huang	Create	

*/
#include "uiapi.h"
#include "ui.h"

#ifndef __A1013_UIMENU_H__
#define __A1013_UIMENU_H__
#define	UI_MENU_NODE_LIST_ITEM_MAX		0x20
#define UI_MENU_SETIDX_COUNT            UI_MENU_SETIDX_LAST
#define UI_MENU_NODE_END                11
u8 uiMenuSelect[UI_MENU_SETIDX_COUNT];
u8 uiMenuSetting[UI_MENU_SETIDX_COUNT];

UI_MENU_NODE_LIST uiMenuNodeList[] =
{
/*  depth   count   leaf    */

    {   0,  4,  0, },  /*  "Preview", "Quad", "Menu", "Play File" */

    {   1,  0,  11,},  /*  "Preview" */
    {   1,  0,  11,},  /*  "Quad" */  
    {   1,  6,  1 ,},  /*  "Camera Setting", "Recording",  "Setting", "Storge Setting", "Playback", "System Info" */
    {   1,  0,  11,},  /*  "Play File" */  

    #if((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    {   2,  6,  1,  },  /* "Pairing", "Camera On/Off", "Resolution" , "Brightness", "Camera Alarm", "Anti-flicker" */  
    #else 
    {   2,  5,  1,  },  /* "Pairing", "Camera On/Off", "Resolution" , "Brightness", "Anti-flicker" */  
    #endif
    #if(UI_BAT_SUPPORT)
    {   2,  5,  1,  },  /* "REC Mode", "Sensitivy", "Section", "Scheduled", "Battery Cam Scheduled" */
    #else
    {   2,  4,  1,  },  /* "REC Mode", "Sensitivy", "Section", "Scheduled" */
    #endif
    {   2,  1,  1,  },  /* "Calendar" */
#if USB_HOST_MASS_SUPPORT
    {   2,  4,  1,  },  /* "Overwrite", "Format", "Storage Info", "HDD Remove"*/
#else
    {   2,  3,  1,  },  /* "Overwrite", "Format", "Storage Info"*/
#endif
    {   2,  6,  1,  },  /* "Date Time", "Language", "Default", "Update", "Network", "Monitor Alarm"*/
    {   2,  3,  1,  },  /* "Version", "Network" , "APP"*/

    {   3,  4,  11, },  /* "Pairing Camera1", "Pairing Camera2", "Pairing Camera3", "Pairing Camera4"*/
    {   3,  1,  11, },  /* "Camera All Resolution" */
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {   3,  4,   1, },  /* "TX Light On/Off","TX Light Sch","TX Alarm On/Off","TX Alarm Sch"*/
    #elif ((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    {   3,  2,   1, },  /* "TX Light On/Off","TX Light Sch"*/
    #endif
    {   3,  1,  11, },  /* "Camera All On/Off" */
    {   3,  1,  11, },  /* "Camera All Brightness" */
    {   3,  2,  11, },  /* "Anti-flicker 50 Set","Anti-flicker 60 Set"*/

    {   3,  1,  11, },  /* "REC Mode Setting" */  
    {   3,  1,  11, },  /* "Sensitivy Setting" */
    {   3,  3,  11, },  /* "Section Setting 15","Section Setting 30","Section Setting 60"  */
    {   3,  1,  1 , },  /* "Scheduled Bar"  */
    #if(UI_BAT_SUPPORT)
    {   3,  1,  1 , },  /* "Battery Cam Scheduled Bar"  */
    #endif
    
    {   3,  1,  1, },  /* "Playback List"*/

    {   3,  2,  11, },  /* "Overwrite Setting yes","Overwrite Setting no"*/
    {   3,  2,  11, },  /* "Fomat Setting yes","Overwrite Setting no"*/
    {   3,  1,  11, },  /* "Storage Info" */
#if USB_HOST_MASS_SUPPORT  
    {   3,  2,  11, },  /* "HDD Remove yes", "HDD Remove no" */
#endif

#if SET_NTPTIME_TO_RTC
    {   3,  1,  1 , },  /* "YYYY/MM/DD/HH/mm/ss" */
#else
    {   3,  1,  11, },  /* "YYYY/MM/DD/HH/mm/ss" */
#endif
    {   3,  3,  11, },  /* "Language Setting English","Language Setting Italy","Language Setting French" */
    {   3,  2,  11, },  /* "Defalut Setting yes","Defalut Setting no"*/
    {   3,  2,  1 , },  /* "Server", "SD"*/
    {   3,  2,  1 , },  /* "DHCP", "Static IP"*/
    {   3,  2,  11, },  /* "Monitor Alarm Setting yes","Monitor Alarm Setting no" */
    
    {   3,  1,  11, },  /* "Version Info" */
    {   3,  1,  11, },  /* "Netwrok Info" */
    {   3,  1,  11, },  /* "APP Info" */

    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {   4,  1,  11, },  /* "TX Light On/Off"*/
    {   4,  1,   1, },  /* "TX Light Sch Bar"*/
    {   4,  1,  11, },  /* "TX Alarm On/Off"*/
    {   4,  1,   1, },  /* "TX Alarm Sch Bar"*/
    #elif ((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    {   4,  1,  11, },  /* "TX Light On/Off"*/
    {   4,  1,   1, },  /* "TX Light Sch Bar"*/
    #endif
    
    {   4,  1,  11, },  /* "Scheduled Setting"  */
    
    #if(UI_BAT_SUPPORT)
    {   4,  1,  11, },  /* "Battery Cam Scheduled Setting"  */
    #endif
    
    {   4,  2,  11, },  /* "delete yes", "delete no"*/

#if SET_NTPTIME_TO_RTC
    {   4,  1,  11, },  /* "Time Zone"*/
#endif

    {   4,  2,  11, },  /* "Server Setting yes","Server Setting no"*/   
    {   4,  2,  11, },  /* "SD Setting yes","SD Setting no"*/
    
    {   4,  2,  11, },  /* "DHCP Setting yes","DHCP Setting no"*/  
    {   4,  1,   1, },  /* "Static Setting"*/

    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {   5,  1,  11, },  /* "TX Light Sch Setting"*/
    {   5,  1,  11, },  /* "TX Alarm Sch Setting"*/
    #elif ((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    {   5,  1,  11, },  /* "TX Light Sch Setting"*/
    #endif
    
    {   5,  1,  11, },  /* "KEYPAD"*/
        
};

#define UI_MENU_NODE_LIST_COUNT     (sizeof(uiMenuNodeList)/sizeof(UI_MENU_NODE_LIST))

#endif  

