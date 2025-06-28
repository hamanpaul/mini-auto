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
#define	UI_MENU_NODE_LIST_ITEM_MAX		0x12
#define UI_MENU_SETIDX_COUNT            UI_MENU_SETIDX_LAST
#define UI_MENU_NODE_END                11
u8 uiMenuSelect[UI_MENU_SETIDX_COUNT];
u8 uiMenuSetting[UI_MENU_SETIDX_COUNT];

#if (SUPPORT_TOUCH == 1)
UI_MENU_NODE_LIST uiMenuNodeList[] =
{
    /*  depth   count   leaf    */
    {   0,  3,  0,  },  /* "Preview", "Setup", "Playback"*/
    {   1,  2,  11, },  /* "Split 4" , "Full Screen*/
    {   1,  2,  11, },  /*  "Image", "REC Setting"*/
    {   1,  1,  11, },  /* "Playback Mode*/
};
#else
UI_MENU_NODE_LIST uiMenuNodeList[] =
{
    /*  depth   count   leaf    */
    {   0,  6,  0,  },  /* "Channel control", "Image Zoom", "Sound control", "Image adjust", "Menu", "Playback"*/

    {   1,  2,  11, },  /* "Split 4" , "Full Screen*/
    {   1,  4,  1, },  /* Image Zoom ~ CH1~4*/
    {   1,  1,  11, },  /* "Sound Set" */
    {   1,  4,  1, },  /* Image adjust CH1~4*/
    {   1,  6,  1, },  /*  "Resolution", "REC Setting", "REC Mode", "System Setting", "Audio", "Camera"*/
    {   1,  2,  1, },  /* "Search all , Search By" */

    {   2,  1,  11, },  /* Image Zoom CH1 Set */
    {   2,  1,  11, },  /* Image Zoom CH2 Set */    
    {   2,  1,  11, },  /* Image Zoom CH3 Set */
    {   2,  1,  11, },  /* Image Zoom CH4 Set */         

    {   2,  1,  11, },  /* Image adjust CH1 Set */
    {   2,  1,  11, },  /* Image adjust CH2 Set */    
    {   2,  1,  11, },  /* Image adjust CH3 Set */
    {   2,  1,  11, },  /* Image adjust CH4 Set */         
    {   2,  4,  1,  },  /* "Resolution 1", "Resolution 2", "Resolution 3 "Resolution 4" */
   
    {   2,  4,  1,  },  /* "OverWrite", "Section", "Motion Detection" ,  "Channel" */
    {   2,  2,  11, },  /* "Manual", "Motion Detec REC" */
    {   2,  5,  1,  },  /* "Time", "Format", "Card Info.", "Network", "Display device"*/
    {   2,  2,  1,  },  /* "Resolution", "Sampling Rate"*/
    {   2,  2,  2,  },  /* "Pair", "On/Off"*/

    {   2,  2,  11, },  /* "All Delete Yes , "All Delete No"*/
    {   2,  2,  11, },  /* "delete yes", "delete no"*/
    
    {   3,  3,  11, },  /* "Camera 1 HD", "Camera 1 VGA", "Camera 1 QVGA" */
    {   3,  3,  11, },  /* "Camera 2 HD", "Camera 2 VGA", "Camera 2 QVGA" */
    {   3,  3,  11, },  /* "Camera 3 HD", "Camera 3 VGA", "Camera 3 QVGA" */
    {   3,  3,  11, },  /* "Camera 4 HD", "Camera 4 VGA", "Camera 4 QVGA" */


    {   3,  2,  11, },  /* " Overwrite Yes", "Overwrite NO" */
    {   3,  6,  11, },  /* "20 sec", " 1min", "5min", "10min", "15min", "30min",  */
    {   3,  2,  2,  },  /* "Mask Area", "Sensitivity"*/
    {   3,  2,  11, },  /* "Channel 1", "Channel 2" */
    {   3,  1,  11, },  /* "YYYY/MM/DD/HH/mm" */
    {   3,  2,  11, },  /* "Format Yes", "Format NO" */
    {   3,  1,  11, },  /* " Card Size" */
    {   3,  1,  11, },  /* " Network" */
    {   3,  2,  11, },  /* " Display TV", "Display Panel" */

    {   3,  2,  11, },  /* " Resolution 8bit", " Resolution 16bit" */
    {   3,  5,  11, },  /* " Sampling 8k", "Sampling 16k", "Sampling 32k", "Sampling 44.1k", "Sampling 48k" */

    {   3,  4,  11, },  /* " Pair Cam1", " Pair Cam2", " Pair Cam3", " Pair Cam4"*/
    {   3,  4,  3,  },  /* " Cam1 On/Off", "Cam2 On/Off", "Cam3 On/Off", "Cam4 On/Off"*/

    {   4,  1,  11, },  /* "Area" */
    {   4,  3,  11, },  /* "H", "M", "L" */

    {   4,  2,  11, },  /* "Cam1 On", "Cam1 Off"*/
    {   4,  2,  11, },  /* "Cam2 On", "Cam2 Off"*/
    {   4,  2,  11, },  /* "Cam3 On", "Cam3 Off"*/
    {   4,  2,  11, },  /* "Cam4 On", "Cam4 Off"*/
    
};
#endif
#define UI_MENU_NODE_LIST_COUNT     (sizeof(uiMenuNodeList)/sizeof(UI_MENU_NODE_LIST))

#endif


