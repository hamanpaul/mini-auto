/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ui.h

Abstract:

   	The declarations of user interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2009/03/08	JJ Huang	Create	

*/

#ifndef __OSD_DRAW_MR9620_H__
#define __OSD_DRAW_MR9620_H__

/* Constant definition */


/* Node list item */
enum
{
    /***************************level 0***************************/
    UI_MENU_NODE_PREVIEW_MODE = 0,
    UI_MENU_NODE_QUAD_MODE,
    UI_MENU_NODE_MENU,
    UI_MENU_NODE_PLAYBACK_MODE,

    
    /***************************level 1***************************/
    UI_MENU_NODE_CAMERA_SET,    
    UI_MENU_NODE_REC_SET,
    UI_MENU_NODE_PLAYBACK,
    UI_MENU_NODE_STORAGE_SET,
    UI_MENU_NODE_SYSTEM_SET,
    UI_MENU_NODE_SYSTEM_INFO,  


    /***************************level 2***************************/         
    /*Camera Setting*/
    UI_MENU_NODE_CAMSET_PARING,
    UI_MENU_NODE_CAMSET_CAMERA_ONOFF,
    UI_MENU_NODE_CAMSET_RESOLUTION,
    UI_MENU_NODE_CAMSET_BRIGHTNESS,
    #if((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    UI_MENU_NODE_CAMSET_ALARM,
    #endif 
    UI_MENU_NODE_CAMSET_ANTIFLICKER,
    
    /*Recording Setting*/
    UI_MENU_NODE_REC_RECMODE,
    UI_MENU_NODE_REC_SENSITIVE,
    UI_MENU_NODE_REC_SECTION,
    UI_MENU_NODE_REC_SCHEDULED,
    #if(UI_BAT_SUPPORT)
    UI_MENU_NODE_REC_BATCAM_SCHEDULED,
    #endif
    
    /*Playback*/
    UI_MENU_NODE_PLAYBACK_CALENDAR,//23

    /*Storage Setting*/
    UI_MENU_NODE_STORAGE_OVERWRITE,
    UI_MENU_NODE_STORAGE_FORMAT,
    UI_MENU_NODE_STORAGE_INFO,
#if USB_HOST_MASS_SUPPORT
    UI_MENU_NODE_STORAGE_HDD_REMOVE,
#endif

    /*System Setting*/
    UI_MENU_NODE_SYSTEM_TIME,
    UI_MENU_NODE_SYSTEM_LANGUAGE,
    UI_MENU_NODE_SYSTEM_DEFAULT,
    UI_MENU_NODE_SYSTEM_UPDATE,
    UI_MENU_NODE_SYSTEM_NETWORK,
    UI_MENU_NODE_SYSTEM_MONITOR_ALARM,

    /*System Info*/
    UI_MENU_NODE_SYSINFO_VERSION,
    UI_MENU_NODE_SYSINFO_NETWORK,
    UI_MENU_NODE_SYSINFO_APP,

    
    /***************************level 3***************************/
    /*Camera Setting */
    UI_MENU_NODE_SET_PAIRING1,
    UI_MENU_NODE_SET_PAIRING2,
    UI_MENU_NODE_SET_PAIRING3,
    UI_MENU_NODE_SET_PAIRING4,

    UI_MENU_NODE_SET_RESOLUCTION,

    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    UI_MENU_NODE_CAMERA_LIGHT_ONOFF,
    UI_MENU_NODE_CAMERA_LIGHT_SCH_BAR,
    UI_MENU_NODE_CAMERA_ALARM_ONOFF,
    UI_MENU_NODE_CAMERA_ALARM_SCH_BAR,
    #elif (UI_LIGHT_SUPPORT)
    UI_MENU_NODE_CAMERA_LIGHT_ONOFF,
    UI_MENU_NODE_CAMERA_LIGHT_SCH_BAR,
    #elif (UI_CAMERA_ALARM_SUPPORT)
    UI_MENU_NODE_CAMERA_ALARM_ONOFF,
    UI_MENU_NODE_CAMERA_ALARM_SCH_BAR,
    #endif
    
    UI_MENU_NODE_CAM_ONOFF,

    UI_MENU_NODE_SET_BRIGHTNESS,

    UI_MENU_NODE_SET_FLICKER_50,
    UI_MENU_NODE_SET_FLICKER_60,
    
   /*Recording Setting */
    UI_MENU_NODE_SET_RECMODE,

    UI_MENU_NODE_SET_SENSITIVITY,
    
    UI_MENU_NODE_SET_SECTION_15,
    UI_MENU_NODE_SET_SECTION_30,
    UI_MENU_NODE_SET_SECTION_60,

    UI_MENU_NODE_SCHEDULED_BAR,

    #if(UI_BAT_SUPPORT)
    UI_MENU_NODE_BATCAM_SCHEDULED_BAR,
    #endif
    
    /*Playback*/
    UI_MENU_NODE_SET_PLAYBACK,

    /*Storage Setting*/
    UI_MENU_NODE_SET_OVERWRITE_YES,
    UI_MENU_NODE_SET_OVERWRITE_NO,
    
    UI_MENU_NODE_SET_FORMAT_YES,
    UI_MENU_NODE_SET_FORMAT_NO,
    
    UI_MENU_NODE_SET_STORAGE_INFO,

#if USB_HOST_MASS_SUPPORT
    UI_MENU_NODE_SET_HDD_REMOVE_YES,
    UI_MENU_NODE_SET_HDD_REMOVE_NO,
#endif
    
    /*System Setting */
    UI_MENU_NODE_SET_TIME,
      
    UI_MENU_NODE_SET_LANGUAGE_ENGLISH,
    UI_MENU_NODE_SET_LANGUAGE_ITALY,
    UI_MENU_NODE_SET_LANGUAGE_FRENCE,
    
    UI_MENU_NODE_DEFAULT_YES,
    UI_MENU_NODE_DEFAULT_NO,
    
    UI_MENU_NODE_UPGRADE_NET,
    UI_MENU_NODE_UPGRADE_SD,

    UI_MENU_NODE_NETWORK_DHCP,
    UI_MENU_NODE_NETWORK_STATIC,
    
    UI_MENU_NODE_SET_MONITOR_ALARM_YES,
    UI_MENU_NODE_SET_MONITOR_ALARM_NO,

    /*System Info*/
    UI_MENU_NODE_SET_DEVICEINFO,
    UI_MENU_NODE_SET_NETWORKIFNO,
    UI_MENU_NODE_SET_APPIFNO,
    

    /***************************level 4***************************/
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    UI_MENU_NODE_SET_TX_LIGHT_ONOFF,
    UI_MENU_NODE_TX_LIGHT_SCH_BAR,
    UI_MENU_NODE_SET_ALARM_ONOFF,
    UI_MENU_NODE_TX_ALARM_SCH_BAR,
    #elif (UI_LIGHT_SUPPORT)
    UI_MENU_NODE_SET_TX_LIGHT_ONOFF,
    UI_MENU_NODE_TX_LIGHT_SCH_BAR,
    #elif (UI_CAMERA_ALARM_SUPPORT)
    UI_MENU_NODE_SET_ALARM_ONOFF,
    UI_MENU_NODE_TX_ALARM_SCH_BAR,
    #endif
    
    UI_MENU_NODE_SET_SCHEDULED,

    #if(UI_BAT_SUPPORT)
    UI_MENU_NODE_SET_BATCAM_SCHEDULED,
    #endif
    
    UI_MENU_NODE_DELETE_YES,
    UI_MENU_NODE_DELETE_NO,

#if SET_NTPTIME_TO_RTC
    UI_MENU_NODE_TIME_ZONE,
#endif

    UI_MENU_NODE_SET_SERVER_YES,
    UI_MENU_NODE_SET_SERVER_NO,
    UI_MENU_NODE_SET_SD_YES,
    UI_MENU_NODE_SET_SD_NO,

    UI_MENU_NODE_SET_DHCP_YES,    
    UI_MENU_NODE_SET_DHCP_NO,    
    UI_MENU_NODE_SET_STATIC,

    /***************************level 5***************************/
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    UI_MENU_NODE_SET_TX_LIGHT_SCH,
    UI_MENU_NODE_SET_TX_ALARM_SCH,
    #elif (UI_LIGHT_SUPPORT)
    UI_MENU_NODE_SET_TX_LIGHT_SCH,
    #elif (UI_CAMERA_ALARM_SUPPORT)
    UI_MENU_NODE_SET_TX_ALARM_SCH,
    #endif
    
    UI_MENU_NODE_KEYPAD,

    
    /*keep in the last*/
    UI_MENU_NODE_COUNT
} ;

extern void osdDrawQuadFrame(u8 channel,u8 act);
extern void osdDrawQuadFrameBG(void);

#endif

