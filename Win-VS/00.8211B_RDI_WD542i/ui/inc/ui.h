/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ui.h

Abstract:

   	The declarations of user interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/
#include "fsapi.h"	//civic 070829
#include "rtcapi.h" //albert 20090520
#include "uiapi.h"

#ifndef __UI_H__
#define __UI_H__

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define GUI_TOUCH_DELAY         5
#define UI_MENU_MAX_COLUMN      4
#if ((HW_BOARD_OPTION == MR8200_RX_RDI_M930) ||\
     (HW_BOARD_OPTION == MR8200_RX_RDI_M706) ||\
     (HW_BOARD_OPTION == MR8200_RX_RDI_M742) ||\
     (HW_BOARD_OPTION == MR8200_RX_RDI_M731) ||\
     (HW_BOARD_OPTION == MR8120_RX_RDI_M733) ||\
     (HW_BOARD_OPTION == MR8200_RX_RDI_M731_HA) ||\
     (HW_BOARD_OPTION == MR8200_RX_RDI_M742_HA) ||\
     (HW_BOARD_OPTION == MR8100_GCT_LCD) ||\
     (HW_BOARD_OPTION == MR8100_RX_RDI_SEM) ||\
     (HW_BOARD_OPTION == MR8100_RX_RDI_M512) ||\
     (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) ||\
     (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
#define UI_MENU_SIZE_X          800
#elif(HW_BOARD_OPTION == MR8120_RX_RDI)
#define UI_MENU_SIZE_X          704
#else
#define UI_MENU_SIZE_X          640
#endif
/*
 *********************************************************************************************************
 * Data Type
 *********************************************************************************************************
 */

/* Menu input */
/* Each node list contains the nodes from the same parent */
typedef struct _UI_MENU_NODE_LIST
{
	u8			depth;	/* tree depth of the node list */
	u8			count;	/* count of nodes in the node list */ 
	u8			leaf;	/* 0: non-leaf, 1: general leaf, 2: increment/decrement leaf */
} UI_MENU_NODE_LIST;	

/* Menu link */
typedef struct {
    u8* FileName;
    s32 bufIndex;
} UI_NODE_PHOTO;

/* Menu link */
typedef struct {
    u8* IconAddr;
    u16 Icon_w;
    u16 Icon_h;
} UI_NODE_ICON;


typedef struct {
    UI_NODE_PHOTO*  FileInfo;
    u16 FileSize_x;
    u16 FileSize_y;
    u16 Location_x;
    u16 Location_y;

} UI_NODE_FILE;

typedef struct {
    UI_NODE_PHOTO*  FileInfo;
    u16 Location_x;
    u16 Location_y;
} UI_NODE_SUB_FILE;

typedef struct _Ui_Mult_Icon{
    u8* IconAddr;
    u16 Icon_w ;
    u16 Icon_h ;
}UI_MULT_ICON;

typedef struct _Ui_OsdIcon_Tab{
    OSD_ICONIDX        IconIdx;
    UI_MULT_ICON       *PicIcon;
}UI_OSDICON_TAB;

typedef struct {
    OSD_ICONIDX IconIndex;
    u16 Location_x;
    u16 Location_y;
    u16 Icon_w ;
    u16 Icon_h ;
    u8 Selected;
} UI_NODE_OSD;

typedef enum Mult_Size
{
	UI_MULT_ICON_B = 0,
	UI_MULT_ICON_S, 
	UI_MULT_ICON_END,                                                                                                                                                       

} MULT_SIZE;

#if (SUPPORT_TOUCH == 1)
typedef struct _UITOUCH_MENU_NODE_EVENT
{
    u16  uiLeftUp_X;
    u16  uiLeftUp_Y;	
    u16  uiIconWidth;
    u16  uiIconHeight;
    u32  uiKeyEvent;
    s8   uiNodeNum;
} UITOUCH_MENU_NODE_EVENT;

typedef struct {
    UITOUCH_MENU_NODE_EVENT *pTouchNodeEvent;
    u32 uiCheckEventNum;
} UITOUCH_NODE_EVENT_TBL;
#endif

typedef struct {
    u8* Node_Name;
    u32 FileNum;
    UI_NODE_FILE*   FileData;    
    UI_NODE_OSD *  IconData;
    s8  Action_no;
#if (SUPPORT_TOUCH == 1)
    UITOUCH_NODE_EVENT_TBL* TouchData;
#endif
} UI_NODE_DATA;

/* Each node is a menu item */
typedef struct _UI_MENU_NODE_ITEM
{
	u8			index;	/* index of peer node list */
	u8			setidx;	/* index of menu select/setting array */
	u8			leaf;	/* 0: non-leaf, 1: general leaf, 2: increment/decrement leaf */
	u8			depth;
	u8			count;    // How many option that this node have
	u8*			name;	/* menu name of the node */
    UI_NODE_DATA*   NodeData;
} UI_MENU_NODE_ITEM;

typedef struct _UI_MENU_NODE
{
	struct _UI_MENU_NODE*	parent;	
	struct _UI_MENU_NODE*	child;
	struct _UI_MENU_NODE*	left;
	struct _UI_MENU_NODE*	right;
	UI_MENU_NODE_ITEM	item;
} UI_MENU_NODE;


typedef struct _UI_HANDLER{
  u8          	MenuMode;         /* Indicate which UI mode we are in */
  u8          	WhichKey;          // Which key was clicked
} UI_HANDLER;


#if Auto_Video_Test
typedef struct {
    u8      VideoTest_Mode;      /*0:stop, 1:capture, 2:playbackInit, 3:playback, 4:delete*/
    u32     VideoTest_FileTime;  /*second*/
    u32     VideoTest_FileNum;  
    u32     VideoTest_CurrFileElapse;
    u32     VideoTest_CurrFileNum;
} AUTO_VIDEO_TEST;

extern AUTO_VIDEO_TEST Video_Auto;
#endif

typedef struct _Ui_Mult_Lan_Str{
    u8* StrIndex;
    u8  Str_len;
}UI_MULT_LAN_STR;

typedef struct _Ui_Msg_Tab{
    u8                  MsgIdx;
    UI_MULT_LAN_STR     *MsgStr;
}UI_MSG_TAB;


/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */
extern UI_MENU_NODE* uiCurrNode;
extern UI_NODE_DATA uiMenuNodeListItem[];
extern UI_HANDLER MyHandler;
extern u8 UI_CFG_RES[4];
extern u8 UI_TMP_RES[4];
extern u8  uiP2PRestoreCfgTime;
extern u8 UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
extern u8 Start_UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
extern u32 prev_P2pVideoQuality;
/*
 *********************************************************************************************************
 * Extern Function Prototype
 *********************************************************************************************************
 */
extern s32 uiKeyParse(u8 key);
extern u8 uiOsdGetStrLibByLanguage(MSG_SRTIDX str_inx, u8 **lib, u8 *font_w, u8 *font_h, UI_MULT_LAN_STR **str_info, u8 language);
extern u8 uiOsdGetStrLib(MSG_SRTIDX str_inx, u8 **lib, u8 *font_w, u8 *font_h, UI_MULT_LAN_STR **str_info);
extern void uiMenuTreeInit(void);
extern void uiFlowRFStatus(u32 waitFlag);
extern void uiMenuSetBootSetting(void);
extern s32 uiMenuSetImageQuality(s8 setting);
extern s32 uiMenuSetVideoSize(s8 setting);
extern void uiGraphDrawMenu(void);
extern void uiMenuOSDFrame(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx, u32 data);
extern void uiEnterMenu(u8 index);
extern u8 uiActSet_RfDisplayMode(u8 setting);
extern u8 uiSetRfDisplayMode(u8 setting);
extern void uiOsdDrawNoSignal(u8 act);
extern u8 uiGraphDrawJpgGraph(s32 fb_index, u8* pResult, u16* pWidth, u16* pHeight);
extern void uiGraphDrawJPGImage(UI_NODE_PHOTO image, u16 x_pos, u16 y_pos);
extern u8 uiOsdGetIconInfo(OSD_ICONIDX icon_inx , UI_MULT_ICON **icon_info);

#if (RFIU_SUPPORT)
extern u8 uiSetRfChangeChannel(u8 setting);
extern u8 uiSetRfChangeAudio_QuadMode(u8 setting);
extern u8 uiSetRfChgPTZ_CH(u8 setting);
extern u8 uiSetRfSwAudio_DualMode(u8 setting);

#endif
extern s32 uiSetFileInitFinish(void);
extern s32 uiSetFileDoCommand(u8* pucAddr, u32* bufferlen);
extern void uiOsdIconInit(void);
#if(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
extern void uiOsdDrawUpgradeFinish(void);
#endif
    
#if (HW_BOARD_OPTION==MR6730_AFN)	
#define FLAG_APP_INT_NUM			8
//
#define FLAG_APP_INT_0   0x00000001
#define FLAG_APP_INT_1   0x00000002
#define FLAG_APP_INT_2   0x00000004
#define FLAG_APP_INT_3   0x00000008
#define FLAG_APP_INT_4   0x00000010
#define FLAG_APP_INT_5   0x00000020
#define FLAG_APP_INT_6   0x00000040
#define FLAG_APP_INT_7   0x00000080


/*
const INT32U gFlagAppInt[FLAG_APP_INT_NUM] = {
    FLAG_APP_INT_0,
    FLAG_APP_INT_1,
    FLAG_APP_INT_2,
    FLAG_APP_INT_3,
    FLAG_APP_INT_4,
    FLAG_APP_INT_5,
    FLAG_APP_INT_6,
    FLAG_APP_INT_7,    
};
*/


extern void ui_App_TaskInit(void);
extern OS_FLAG_GRP  *gAppFlagGrp;


#endif//#if (HW_BOARD_OPTION==MR6730_AFN)


#endif  /*end of #ifndef __UI_H__*/

