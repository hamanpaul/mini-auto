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
    UI_MENU_NODE_CHANNEL_CONTROL = 0,
    UI_MENU_NODE_ZOOM,
    UI_MENU_NODE_SOUND,
    UI_MENU_NODE_IMAGE_ADJUST,
    UI_MENU_NODE_SETUP,
    UI_MENU_NODE_PLAYBACKKEY,


    /***************************level 1***************************/
    UI_MENU_NODE_SPLIT4,
    UI_MENU_NODE_FULL_SCREEN,    

    UI_MENU_NODE_ZOOM_CH1,    
    UI_MENU_NODE_ZOOM_CH2,        
    UI_MENU_NODE_ZOOM_CH3,    
    UI_MENU_NODE_ZOOM_CH4,        

    UI_MENU_NODE_VIDEO_SOUND_SET,

    UI_MENU_NODE_VIDEO_ADJUST_CH1,    
    UI_MENU_NODE_VIDEO_ADJUST_CH2,        
    UI_MENU_NODE_VIDEO_ADJUST_CH3,    
    UI_MENU_NODE_VIDEO_ADJUST_CH4,    

    UI_MENU_NODE_RESOLUTION,
    UI_MENU_NODE_RECSetting,
    UI_MENU_NODE_RECMODE,
    UI_MENU_NODE_SYSTEM,
    UI_MENU_NODE_AUDIO,
    UI_MENU_NODE_CAMERA,

    UI_MENU_NODE_PLAYBACK_SEARCH_ALL,
    UI_MENU_NODE_PLAYBACK_SEARCH_BY,

    /***************************level 2***************************/    
    UI_MENU_NODE_ZOOM_CH1_ZOOM_IN,     
    UI_MENU_NODE_ZOOM_CH2_ZOOM_IN,     
    UI_MENU_NODE_ZOOM_CH3_ZOOM_IN,     
    UI_MENU_NODE_ZOOM_CH4_ZOOM_IN,     

    UI_MENU_NODE_CH1_IMG_SET,     
    UI_MENU_NODE_CH2_IMG_SET,     
    UI_MENU_NODE_CH3_IMG_SET,     
    UI_MENU_NODE_CH4_IMG_SET,         
    
    /*Resolution Setting*/
    UI_MENU_NODE_RES_CAM1,
    UI_MENU_NODE_RES_CAM2,
    UI_MENU_NODE_RES_CAM3,
    UI_MENU_NODE_RES_CAM4,

    /*REC. Setting*/
    UI_MENU_NODE_RECSET_OVERWRITE,
    UI_MENU_NODE_RECSET_SECTION,
    UI_MENU_NODE_RECSET_MOTION,
    UI_MENU_NODE_RECSET_CHANNEL,

    /*REC. Mode*/
    UI_MENU_NODE_RECMODE_MANUAL,
    UI_MENU_NODE_RECMODE_MOTION,

    /*System Setting*/
    UI_MENU_NODE_SYSTEM_TIME,
    UI_MENU_NODE_SYSTEM_FORMAT,
    UI_MENU_NODE_SYSTEM_CARDINFO,
    UI_MENU_NODE_SYSTEM_NETWORK,
    UI_MENU_NODE_SYSTEM_DISPLAY,

    /*Audio*/
    UI_MENU_NODE_AUDIO_RESOLUTION,
    UI_MENU_NODE_AUDIO_SAMPLING,
    
    /*Camera*/
    UI_MENU_NODE_CAMERA_PAIR,
    UI_MENU_NODE_CAMERA_ONOFF,

    /*Playback*/
    UI_MENU_NODE_ALL_DELETE_YES,
    UI_MENU_NODE_ALL_DELETE_NO,

    /*Playback*/
    UI_MENU_NODE_DELETE_YES,
    UI_MENU_NODE_DELETE_NO,

    /***************************level 3***************************/
    /*Resolution Setting -> CAM 1*/
    UI_MENU_NODE_RES_C1_HD,
    UI_MENU_NODE_RES_C1_VGA,
    UI_MENU_NODE_RES_C1_QVGA,

    /*Resolution Setting -> CAM 2*/
    UI_MENU_NODE_RES_C2_HD,
    UI_MENU_NODE_RES_C2_VGA,
    UI_MENU_NODE_RES_C2_QVGA,

    /*Resolution Setting -> CAM 3*/
    UI_MENU_NODE_RES_C3_HD,
    UI_MENU_NODE_RES_C3_VGA,
    UI_MENU_NODE_RES_C3_QVGA,

    /*Resolution Setting -> CAM 4*/
    UI_MENU_NODE_RES_C4_HD,
    UI_MENU_NODE_RES_C4_VGA,
    UI_MENU_NODE_RES_C4_QVGA,

    /*REC. Setting -> Overwrite*/
    UI_MENU_NODE_RECSET_OVERWRITE_YES,
    UI_MENU_NODE_RECSET_OVERWRITE_NO,

    /*REC. Setting -> Scetion*/
    UI_MENU_NODE_RECSET_SECTION_1MIN,
    UI_MENU_NODE_RECSET_SECTION_5MIN,
    UI_MENU_NODE_RECSET_SECTION_10MIN,
    UI_MENU_NODE_RECSET_SECTION_15MIN,
    UI_MENU_NODE_RECSET_SECTION_30MIN,
    UI_MENU_NODE_RECSET_SECTION_60MIN,

    /*REC. Setting -> Motion Detection*/
    UI_MENU_NODE_RECSET_MOTION_MASK,
    UI_MENU_NODE_RECSET_MOTION_SENSITIVITY,

    /*REC. Setting -> Channel*/
    UI_MENU_NODE_RECSET_CHANNEL_1,
    UI_MENU_NODE_RECSET_CHANNEL_2,

    /*System Setting -> Time*/
    UI_MENU_NODE_TIME_DATE,

    /*System Setting -> Format*/
    UI_MENU_NODE_FORMAT_YES,
    UI_MENU_NODE_FORMAT_NO,

    /*System Setting -> Card Info*/
    UI_MENU_NODE_CARDINFO_INFO,

    /*System Setting -> Network*/
    UI_MENU_NODE_NETWORK,

    /*System Setting -> Display Device*/
    UI_MENU_NODE_DISPLAY_TV,
    UI_MENU_NODE_DISPLAY_PANEL,
    
    /*Audio -> Resolution*/
    UI_MENU_NODE_RESOLUTION_8BIT,
    UI_MENU_NODE_RESOLUTION_16BIT,

    /*Audio -> Sampling Rate*/
    UI_MENU_NODE_SAMPLING_8K,
    UI_MENU_NODE_SAMPLING_16K,
    UI_MENU_NODE_SAMPLING_32K,
    UI_MENU_NODE_SAMPLING_44K,
    UI_MENU_NODE_SAMPLING_48K,

    /*Camera-->Pair*/
    UI_MENU_NODE_PAIRING_CAM1,
    UI_MENU_NODE_PAIRING_CAM2,
    UI_MENU_NODE_PAIRING_CAM3,
    UI_MENU_NODE_PAIRING_CAM4,

    /*Camera-->OnOff*/
    UI_MENU_NODE_ONOFF_CAM1,
    UI_MENU_NODE_ONOFF_CAM2,
    UI_MENU_NODE_ONOFF_CAM3,
    UI_MENU_NODE_ONOFF_CAM4,

    /***************************level 4***************************/
    /*REC. Setting -> Motion Detection -> Mask*/
    UI_MENU_NODE_RECSET_MOTION_MASK_SETTINGS,

    /*REC. Setting -> Motion Detection -> Sensitivity*/
    UI_MENU_NODE_RECSET_MOTION_SENSITIVITY_H,
    UI_MENU_NODE_RECSET_MOTION_SENSITIVITY_M,
    UI_MENU_NODE_RECSET_MOTION_SENSITIVITY_L,

    /*Camera-->OnOff-->cam 1*/
    UI_MENU_NODE_CAM1_ON,
    UI_MENU_NODE_CAM1_OFF,

    /*Camera-->OnOff-->cam 2*/
    UI_MENU_NODE_CAM2_ON,
    UI_MENU_NODE_CAM2_OFF,

    /*Camera-->OnOff-->cam3*/
    UI_MENU_NODE_CAM3_ON,
    UI_MENU_NODE_CAM3_OFF,

    /*Camera-->OnOff-->cam 4*/
    UI_MENU_NODE_CAM4_ON,
    UI_MENU_NODE_CAM4_OFF,

    /*keep in the last*/
    UI_MENU_NODE_COUNT
} ;

extern u8 OSD_DV_ICON[20*40];
extern u8 OSD_CAMERA_ICON[20*40];
//extern u8 OSD_BAT_ICON[3][16*24];
extern u8 OSD_M_ICON[20*25];
extern u8 OSD_SD_ICON[20*40];
extern u8 OSD_WARNING[16*24];
extern u8 OSD_VREC[16*24];
extern u8 OSD_PLAYBACK_ICON[20*25];
extern u8 OSD_STILL_ICON[20*40];
extern u8 OSD_VIDEO_ICON[20*40];
extern u8 OSD_PLAY_ICON[16*25];
extern u8 OSD_trancan_ICON[20*40];
extern u8 OSD_FF[12*16];
extern u8 OSD_REW[12*16];

#endif

