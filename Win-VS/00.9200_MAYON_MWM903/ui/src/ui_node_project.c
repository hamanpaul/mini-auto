#include <osapi.h>
#include <general.h>
#include "ui.h"
#include "Uiapi.h"
#include "ui_project.h"

/*define all file size for ui menu*/
#define UI_MENU_SIZE_TITLE_X        0
#define UI_MENU_SIZE_TITLE_Y        0
#define UI_MENU_SIZE_4ITEMS_X       320
#define UI_MENU_SIZE_4ITEMS_Y       95
#define UI_MENU_SIZE_8ITEMS_X       280
#define UI_MENU_SIZE_8ITEMS_Y       90
#define UI_MENU_SIZE_BG_X           1024
#define UI_MENU_SIZE_BG_Y           600

/*define all file location for ui menu*/
#define UI_MENU_LOC_BG_X            0
#define UI_MENU_LOC_BG_Y            0
#define UI_MENU_LOC_TITLE_X         1024
#define UI_MENU_LOC_TITLE_Y         92

#define UI_MENU_LOC_4ITEMS_X        160
#define UI_MENU_LOC_4ITEMS_NODE1_Y  97
#define UI_MENU_LOC_4ITEMS_NODE2_Y  193
#define UI_MENU_LOC_4ITEMS_NODE3_Y  288
#define UI_MENU_LOC_4ITEMS_NODE4_Y  382


#define UI_MENU_LOC_8ITEMS_NODE1_X  32
#define UI_MENU_LOC_8ITEMS_NODE2_X  332
#define UI_MENU_LOC_8ITEMS_NODE1_Y  100
#define UI_MENU_LOC_8ITEMS_NODE2_Y  195
#define UI_MENU_LOC_8ITEMS_NODE3_Y  285
#define UI_MENU_LOC_8ITEMS_NODE4_Y  380

#define UI_MENU_LOC_3I_NODE1_Y  177

#define UI_MENU_LOC_2I_NODE1_X  319
#define UI_MENU_LOC_2I_NODE2_X  446

#define UI_MENU_LOC_3I_NODE1_X  130
#define UI_MENU_LOC_3I_NODE2_X  257
#define UI_MENU_LOC_3I_NODE3_X  384

#define UI_MENU_LOC_4I_NODE1_X  67
#define UI_MENU_LOC_4I_NODE2_X  194
#define UI_MENU_LOC_4I_NODE3_X  321
#define UI_MENU_LOC_4I_NODE4_X  448

#define UI_MENU_LOC_5I_NODE1_X  3
#define UI_MENU_LOC_5I_NODE2_X  130
#define UI_MENU_LOC_5I_NODE3_X  257
#define UI_MENU_LOC_5I_NODE4_X  384
#define UI_MENU_LOC_5I_NODE5_X  511


#define UI_MENU_LOC_8I_NODE1_Y  109
#define UI_MENU_LOC_8I_NODE2_Y  239
#define UI_MENU_LOC_8I_NODE1_X  67
#define UI_MENU_LOC_8I_NODE2_X  194
#define UI_MENU_LOC_8I_NODE3_X  321
#define UI_MENU_LOC_8I_NODE4_X  448

#define UI_MENU_LOC_CHOICE_NODE_Y 223
#define UI_MENU_LOC_CHOICE_NODE_X 215

#define UI_MENU_LOC_RETURN_NODE_X 525
#define UI_MENU_LOC_RETURN_NODE_Y 20
#define UI_MENU_SIZE_RETURN_NODE_X 80
#define UI_MENU_SIZE_RETURN_NODE_Y 60

#define UI_MENU_LOC_OK_NODE_X 495
#define UI_MENU_LOC_OK_NODE_Y 400
#define UI_MENU_SIZE_OK_NODE_X 140
#define UI_MENU_SIZE_OK_NODE_Y 52

#define UI_MENU_SIZE_YESNO_X    68
#define UI_MENU_SIZE_YESNO_Y    68
#define UI_MENU_LOC_YESNO_X    311
#define UI_MENU_LOC_YESNO_I1_Y 140
#define UI_MENU_LOC_YESNO_I2_Y 279
#define UI_MENU_LOC_YESNO_I3_Y 417

#define DataNum(node)                   sizeof(node)/sizeof(UI_NODE_FILE)
#define OsdNum(node)                   sizeof(node)/sizeof(UI_NODE_OSD)


/* Graphic Menu */

/****LEVEL 1 ****/

UI_NODE_PHOTO Menu_BG[UI_MULT_LANU_END] =
{
    {"main_bg1.jpg",0},
    {"main_bg2.jpg",0},
    {"main_bg3.jpg",0}
};


/****LEVEL 2 ****/

UI_NODE_PHOTO Pair_BG[UI_MULT_LANU_END] =
{
    {"pair1.jpg",0},
    {"pair2.jpg",0},
    {"pair3.jpg",0}
};

UI_NODE_PHOTO REC_Mode_BG[UI_MULT_LANU_END] =
{
    {"rec1.jpg",0},
    {"rec2.jpg",0},
    {"rec3.jpg",0},
};

UI_NODE_PHOTO Time_BG[UI_MULT_LANU_END] =
{
    {"date1.jpg",0},
    {"date2.jpg",0},
    {"date3.jpg",0}
};

UI_NODE_PHOTO Overwrite_BG[UI_MULT_LANU_END] =
{
    {"ove1.jpg",0},
    {"ove2.jpg",0},
    {"ove3.jpg",0}
};


UI_NODE_PHOTO VersionInfo_BG[UI_MULT_LANU_END] =
{
    {"veri1.jpg",0},
    {"veri2.jpg",0},
    {"veri3.jpg",0}
};

UI_NODE_PHOTO Playback_BG[UI_MULT_LANU_END] =
{
    {"play1.jpg",0},
    {"play1.jpg",0},
    {"play1.jpg",0}
};


/****LEVEL 3 ****/

UI_NODE_PHOTO SET_Pair_BG[UI_MULT_LANU_END] =
{
    {"pair_bg1.jpg",0},
    {"pair_bg1.jpg",0},
    {"pair_bg1.jpg",0}
};



/****LEVEL 4****/

UI_NODE_PHOTO SET_CamOnOff_BG[UI_MULT_LANU_END] =
{
    {"camon_bg1.jpg",0},
    {"camon_bg1.jpg",0},
    {"camon_bg1.jpg",0}
};

UI_NODE_PHOTO SET_Resolution_BG[UI_MULT_LANU_END] =
{
    {"res_bg1.jpg",0},
    {"res_bg1.jpg",0},
    {"res_bg1.jpg",0}
};

UI_NODE_PHOTO SET_Brightness_BG[UI_MULT_LANU_END] =
{
    {"bri_bg1.jpg",0},
    {"bri_bg1.jpg",0},
    {"bri_bg1.jpg",0}
};

UI_NODE_PHOTO SET_CamAlarm_BG[UI_MULT_LANU_END] =
{
    {"cmalr_bg1.jpg",0},
    {"cmalr_bg2.jpg",0},
    {"cmalr_bg3.jpg",0}
};

UI_NODE_PHOTO SET_Flicker_50_BG[UI_MULT_LANU_END] =
{
    {"flk50_bg.jpg",0},
    {"flk50_bg.jpg",0},
    {"flk50_bg.jpg",0}
};

UI_NODE_PHOTO SET_Flicker_60_BG[UI_MULT_LANU_END] =
{
    {"flk60_bg.jpg",0},
    {"flk60_bg.jpg",0},
    {"flk60_bg.jpg",0}
};

UI_NODE_PHOTO Set_Rec_Mode_BG[UI_MULT_LANU_END] =
{
    {"recmo_bg1.jpg",0},
    {"recmo_bg2.jpg",0},
    {"recmo_bg3.jpg",0}
};
    
UI_NODE_PHOTO SET_Sensitive_BG[UI_MULT_LANU_END] =
{
    {"sens_bg1.jpg",0},
    {"sens_bg2.jpg",0},
    {"sens_bg3.jpg",0}
};

UI_NODE_PHOTO SET_Section_15_BG[UI_MULT_LANU_END] =
{
    {"sec15_bg.jpg",0},
    {"sec15_bg.jpg",0},
    {"sec15_bg.jpg",0}
};

UI_NODE_PHOTO SET_Section_30_BG[UI_MULT_LANU_END] =
{
    {"sec30_bg.jpg",0},
    {"sec30_bg.jpg",0},
    {"sec30_bg.jpg",0}
};

UI_NODE_PHOTO SET_Section_60_BG[UI_MULT_LANU_END] =
{
    {"sec60_bg.jpg",0},
    {"sec60_bg.jpg",0},
    {"sec60_bg.jpg",0}
};

UI_NODE_PHOTO Scheduled_Bar_BG[UI_MULT_LANU_END] =
{
    {"schbr_bg1.jpg",0},
    {"schbr_bg1.jpg",0},
    {"schbr_bg1.jpg",0}
};

UI_NODE_PHOTO SET_DateTime_BG[UI_MULT_LANU_END] =
{
    {"date_bg1.jpg",0},
    {"date_bg1.jpg",0},
    {"date_bg1.jpg",0}
};

#if SET_NTPTIME_TO_RTC
UI_NODE_PHOTO SET_TimeZone_BG[UI_MULT_LANU_END] =
{
    {"zone_bg1.jpg",0},
    {"zone_bg1.jpg",0},
    {"zone_bg1.jpg",0}
};
#endif

UI_NODE_PHOTO SET_Language_English_BG[UI_MULT_LANU_END] =
{
    {"Eng_bg.jpg",0},
    {"Eng_bg.jpg",0},
    {"Eng_bg.jpg",0}
};

UI_NODE_PHOTO SET_Language_Italy_BG[UI_MULT_LANU_END] =
{
    {"Ita_bg.jpg",0},
    {"Ita_bg.jpg",0},
    {"Ita_bg.jpg",0}
};

UI_NODE_PHOTO SET_Language_French_BG[UI_MULT_LANU_END] =
{
    {"Fren_bg.jpg",0},
    {"Fren_bg.jpg",0},
    {"Fren_bg.jpg",0}
};

UI_NODE_PHOTO SET_Default_Yes_BG[UI_MULT_LANU_END] =
{
    {"defY_bg.jpg",0},
    {"defY_bg.jpg",0},
    {"defY_bg.jpg",0}
};

UI_NODE_PHOTO SET_Default_No_BG[UI_MULT_LANU_END] =
{
    {"defN_bg.jpg",0},
    {"defN_bg.jpg",0},
    {"defN_bg.jpg",0}
};

UI_NODE_PHOTO Upgrade_BG[UI_MULT_LANU_END] =
{
    {"upgr_bg1.jpg",0},
    {"upgr_bg2.jpg",0},
    {"upgr_bg3.jpg",0}
};

UI_NODE_PHOTO Network_BG[UI_MULT_LANU_END] =
{
    {"nset_bg1.jpg",0},
    {"nset_bg2.jpg",0},
    {"nset_bg3.jpg",0}
};

UI_NODE_PHOTO SET_Monitor_Alarm_BG[UI_MULT_LANU_END] =
{
    {"alrm_bg1.jpg",0},
    {"alrm_bg2.jpg",0},
    {"alrm_bg3.jpg",0}
};

UI_NODE_PHOTO SET_Storage_Info_BG[UI_MULT_LANU_END] =
{
    {"stoi_bg1.jpg",0},
    {"stoi_bg2.jpg",0},
    {"stoi_bg3.jpg",0}
};

UI_NODE_PHOTO SET_VersionInfo_BG[UI_MULT_LANU_END] =
{
    {"veri_bg1.jpg",0},
    {"veri_bg1.jpg",0},
    {"veri_bg1.jpg",0}
};

UI_NODE_PHOTO SET_NetworkInfo_BG[UI_MULT_LANU_END] =
{
    {"ntvr_bg1.jpg",0},
    {"ntvr_bg2.jpg",0},
    {"ntvr_bg3.jpg",0}
};

UI_NODE_PHOTO SET_AppInfo_BG[UI_MULT_LANU_END] =
{
    {"apvr_bg1.jpg",0},
    {"apvr_bg1.jpg",0},
    {"apvr_bg1.jpg",0}
};

UI_NODE_PHOTO SET_Schedule_BG[UI_MULT_LANU_END] =
{
    {"ssch_bg1.jpg",0},
    {"ssch_bg1.jpg",0},
    {"ssch_bg1.jpg",0}
};

UI_NODE_PHOTO SET_Static_BG[UI_MULT_LANU_END] =
{
    {"stip_bg1.jpg",0},
    {"stip_bg2.jpg",0},
    {"stip_bg3.jpg",0}
};

UI_NODE_PHOTO SET_Keypad_BG[UI_MULT_LANU_END] =
{
    {"key_bg1.jpg",0},
    {"key_bg1.jpg",0},
    {"key_bg1.jpg",0}
};



/*Title*/

/****LEVEL 2 ****/

UI_NODE_PHOTO Playback_T[UI_MULT_LANU_END] =
{
    {"playb_t1.jpg",0},
    {"playb_t2.jpg",0},
    {"playb_t3.jpg",0}
};

UI_NODE_PHOTO Playback_Week_T[UI_MULT_LANU_END] =
{
    {"plybw_t1.jpg",0},
    {"plybw_t2.jpg",0},
    {"plybw_t3.jpg",0}
};


/****LEVEL 3 ****/

UI_NODE_PHOTO SET_Pair_T[UI_MULT_LANU_END] =
{
    {"pair_t1.jpg",0},
    {"pair_t2.jpg",0},
    {"pair_t3.jpg",0}
};

UI_NODE_PHOTO SET_CAMSet_T[UI_MULT_LANU_END] =
{
    {"cams_t1.jpg",0},
    {"cams_t2.jpg",0},
    {"cams_t3.jpg",0}
};

UI_NODE_PHOTO SET_Resolution_T[UI_MULT_LANU_END] =
{
    {"res_t1.jpg",0},
    {"res_t2.jpg",0},
    {"res_t3.jpg",0}
};

UI_NODE_PHOTO SET_Brightness_T[UI_MULT_LANU_END] =
{
    {"bri_t1.jpg",0},
    {"bri_t2.jpg",0},
    {"bri_t3.jpg",0}
};

UI_NODE_PHOTO SET_Flicker_T[UI_MULT_LANU_END] =
{
    {"flick_t1.jpg",0},
    {"flick_t2.jpg",0},
    {"flick_t3.jpg",0}
};

UI_NODE_PHOTO Set_Rec_Mode_T[UI_MULT_LANU_END] =
{
    {"recmo_t1.jpg",0},
    {"recmo_t2.jpg",0},
    {"recmo_t3.jpg",0}
};

UI_NODE_PHOTO SET_Sensitive_T[UI_MULT_LANU_END] =
{
    {"sens_t1.jpg",0},
    {"sens_t2.jpg",0},
    {"sens_t3.jpg",0}
};

UI_NODE_PHOTO SET_Section_T[UI_MULT_LANU_END] =
{
    {"sec_t1.jpg",0},
    {"sec_t2.jpg",0},
    {"sec_t3.jpg",0}
};

UI_NODE_PHOTO SET_DateTime_T[UI_MULT_LANU_END] =
{
    {"date_t1.jpg",0},
    {"date_t2.jpg",0},
    {"date_t3.jpg",0}
};

UI_NODE_PHOTO SET_Language_T[UI_MULT_LANU_END] =
{
    {"lang_t1.jpg",0},
    {"lang_t2.jpg",0},
    {"lang_t3.jpg",0}
};

UI_NODE_PHOTO SET_Monitor_Alarm_T[UI_MULT_LANU_END] =
{
    {"alrm_t1.jpg",0},
    {"alrm_t2.jpg",0},
    {"alrm_t3.jpg",0}
};

UI_NODE_PHOTO SET_Overwrite_T[UI_MULT_LANU_END] =
{
    {"ovew_t1.jpg",0},
    {"ovew_t2.jpg",0},
    {"ovew_t3.jpg",0}
};

UI_NODE_PHOTO SET_Format_T[UI_MULT_LANU_END] =
{
    {"fmat_t1.jpg",0},
    {"fmat_t2.jpg",0},
    {"fmat_t3.jpg",0}
};
    
UI_NODE_PHOTO SET_Storage_Info_T[UI_MULT_LANU_END] =
{
    {"stoi_t1.jpg",0},
    {"stoi_t2.jpg",0},
    {"stoi_t3.jpg",0}
};

UI_NODE_PHOTO SET_NetworkInfo_T[UI_MULT_LANU_END] =
{
    {"ntvr_t1.jpg",0},
    {"ntvr_t2.jpg",0},
    {"ntvr_t3.jpg",0}
};

UI_NODE_PHOTO SET_VersionInfo_T[UI_MULT_LANU_END] =
{
    {"veri_t1.jpg",0},
    {"veri_t2.jpg",0},
    {"veri_t3.jpg",0}
};



/****LEVEL 4****/

UI_NODE_PHOTO SET_Light_T[UI_MULT_LANU_END] =
{
    {"light_t1.jpg",0},
    {"light_t2.jpg",0},
    {"light_t3.jpg",0}
};

UI_NODE_PHOTO SET_TX_Alarm_T[UI_MULT_LANU_END] =
{
    {"talrm_t1.jpg",0},
    {"talrm_t2.jpg",0},
    {"talrm_t3.jpg",0}
};

UI_NODE_PHOTO SET_Static_T[UI_MULT_LANU_END] =
{
    {"stip_t1.jpg",0},
    {"stip_t2.jpg",0},
    {"stip_t3.jpg",0}
};

/*Menu*/

/****LEVEL 3 ****/


/*NODE*/

/****LEVEL 0****/

UI_NODE_FILE    Preview_NODE[] =
{
    {NULL, 0, 0, 0, 0},
};


/****LEVEL 1****/

UI_NODE_FILE    CAMERA_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    PLAYBACK_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SYSTEM_SETTING_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    REC_SETTING_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    Storage_SETTING_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SYSTEM_Info_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};


/****LEVEL 2****/

UI_NODE_FILE    Pairing_NODE[] =
{
    {Pair_BG,          UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    REC_Mode_NODE[] =
{
    {REC_Mode_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    Date_Time_NODE[] =
{
    {Time_BG,            UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    Overwirte_NODE[] =
{
    {Overwrite_BG,       UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    PLAYBACK_Mode_NODE[] =
{
    {Playback_BG,       UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Playback_T,        UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
    {Playback_Week_T,   787, 18, 132, 152},
};

UI_NODE_FILE    Version_Info_NODE[] =
{
    {VersionInfo_BG,   UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    DELETE_YES_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    DELETE_NO_NODE[] =
{
    {Menu_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

/****LEVEL 3****/

UI_NODE_FILE    SET_PAIR_CAM_NODE[] =
{
    {SET_Pair_BG,   UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Pair_T,    UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_CAM_ON_OFF_NODE[] =
{
    {SET_CamOnOff_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_CAMSet_T,       UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Resolution_NODE[] =
{
    {SET_Resolution_BG,          UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Resolution_T,           UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Brightness_NODE[] =
{
    {SET_Brightness_BG,          UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Brightness_T,           UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_CamAlarm_NODE[] = //7 node
{
    {SET_CamAlarm_BG,          UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Flicker_50_NODE[] =
{
    {SET_Flicker_50_BG,       UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Flicker_T,           UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Flicker_60_NODE[] =
{
    {SET_Flicker_60_BG,       UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Flicker_T,           UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_REC_MODE_NODE[] =
{
    {Set_Rec_Mode_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Set_Rec_Mode_T,  UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Sensitive_NODE[] =
{
    {SET_Sensitive_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Sensitive_T,  UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Section_15_NODE[] =
{
    {SET_Section_15_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Section_T,     UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Section_30_NODE[] =
{
    {SET_Section_30_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Section_T,     UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Section_60_NODE[] =
{
    {SET_Section_60_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Section_T,     UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    Scheduled_Bar_NODE[] =
{
    {Scheduled_Bar_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_DateTime_NODE[] =
{
    {SET_DateTime_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_DateTime_T,  UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

#if SET_NTPTIME_TO_RTC
UI_NODE_FILE    SET_TimeZone_NODE[] =
{
    {SET_TimeZone_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};
#endif

UI_NODE_FILE    SET_English_NODE[] =
{
    {SET_Language_English_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Language_T,          UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Italy_NODE[] =
{
    {SET_Language_Italy_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Language_T,        UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_French_NODE[] =
{
    {SET_Language_French_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Language_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Default_Yes_NODE[] =
{
    {SET_Default_Yes_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Default_No_NODE[] =
{
    {SET_Default_No_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    Upgrade_NODE[] =
{
    {Upgrade_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    Network_NODE[] =
{
    {Network_BG, UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Monitor_Alarm_Yes_NODE[] =
{
    {SET_Default_Yes_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Monitor_Alarm_T,       UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Monitor_Alarm_No_NODE[] =
{
    {SET_Default_No_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Monitor_Alarm_T,      UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Overwrite_Yes_NODE[] =
{
    {SET_Default_Yes_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Overwrite_T,       UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Overwrite_No_NODE[] =
{
    {SET_Default_No_BG,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Overwrite_T,      UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Format_Yes_NODE[] =
{
    {SET_Default_Yes_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Format_T,              UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Format_No_NODE[] =
{
    {SET_Default_No_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Format_T,             UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Storage_Info_NODE[] =
{
    {SET_Storage_Info_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Storage_Info_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    Playback_List_NODE[] =
{
    {NULL, 0, 0, 0, 0},
};

UI_NODE_FILE    SET_VersionInfo_NODE[] =
{
    {SET_VersionInfo_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_VersionInfo_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_NetworkInfo_NODE[] =
{
    {SET_NetworkInfo_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_NetworkInfo_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_AppInfo_NODE[] =
{
    {SET_AppInfo_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};



/****LEVEL 4****/

UI_NODE_FILE    SET_Light_NODE[] =
{
    {SET_CamOnOff_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Light_T,            UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_TX_Alarm_NODE[] =
{
    {SET_CamOnOff_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_TX_Alarm_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Schedule_NODE[] =
{
    {SET_Schedule_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Upgrade_Sever_Yes_NODE[] =
{
    {SET_Default_Yes_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Upgrade_Sever_No_NODE[] =
{
    {SET_Default_No_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Upgrade_SD_Yes_NODE[] =
{
    {SET_Default_Yes_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Upgrade_SD_No_NODE[] =
{
    {SET_Default_No_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_DHCP_Yes_NODE[] =
{
    {SET_Default_Yes_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_DHCP_No_NODE[] =
{
    {SET_Default_No_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UI_NODE_FILE    SET_Static_NODE[] =
{
    {SET_Static_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {SET_Static_T,         UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y,UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y},  
};

UI_NODE_FILE    SET_Keypad_NODE[] =
{
    {SET_Keypad_BG,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Preview_Node[] =
{
    {    0, 168,  36, 232, UI_KEY_MENU,    -1},
    {  490,   0,  50,  50, UI_KEY_LIGHT,   -1},
    {  540,   0,  60,  50, UI_KEY_ALARM,   -1},
    {   10, 420,  50,  60, UI_KEY_LCD_BL,  -1},
    {   60, 420,  50,  60, UI_KEY_AREC,    -1},
    {  120, 420,  50,  60, UI_KEY_REC,     -1},    
    {  500, 420,  50,  70, UI_KEY_VOL,     -1},
    {  510, 300,  50,  60, UI_KEY_UP,      -1},
    {  510, 400,  50,  60, UI_KEY_DOWN,    -1},
    {  560, 420,  50,  60, UI_KEY_TALK,    -1},
    {  260, 140, 200, 160, UI_KEY_PLAY,    -1},
    {  600, 420,  50,  60, UI_KEY_ENTER,   -1},//change mode
    {    0,   0, 640, 480, UI_KEY_MODE,    -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Quad_Node[] =
{
    {    0, 168,  36, 232, UI_KEY_MENU,  -1},
    {  100,   0,  30,  50, UI_KEY_LIGHT,  0},
    {  420,   0,  30,  50, UI_KEY_LIGHT,  1},
    {  100, 240,  30,  50, UI_KEY_LIGHT,  2},
    {  420, 240,  30,  50, UI_KEY_LIGHT,  3},
    
    {  130,   0,  80,  50, UI_KEY_ALARM,  0},
    {  470,   0,  80,  50, UI_KEY_ALARM,  1},
    {  130, 240,  80,  50, UI_KEY_ALARM,  2},
    {  470, 240,  80,  50, UI_KEY_ALARM,  3},

    {  120,  80,  100, 320, UI_KEY_PLAY,   0},
    {  440,  80,  100, 320, UI_KEY_PLAY,   1},
    //{  120, 300,  100,  80, UI_KEY_PLAY,   2},
    //{  440, 300,  100,  80, UI_KEY_PLAY,   3},
    //{  120, 170,  100,  80, UI_KEY_PLAY,   0},
    //{  440, 170,  100,  80, UI_KEY_PLAY,   1},
    
    {   10, 420,  50,  60, UI_KEY_LCD_BL, -1},
    //{   60, 420,  50,  60, UI_KEY_AREC,   -1},
    {   60, 420,  50,  60, UI_KEY_REC,   -1},
    //{  120, 420,  50,  60, UI_KEY_REC,    -1}, 
    {  600, 420,  50,  60, UI_KEY_ENTER,  -1},//change mode
    
    {    0,   0, 320, 240, UI_KEY_UP,     -1},
    {    0, 240, 320, 240, UI_KEY_DOWN,   -1},
    {  320,   0, 320, 240, UI_KEY_RIGHT,  -1},
    {  320, 240, 320, 240, UI_KEY_LEFT,   -1},        
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Menu_Node[] =
{
    {    600,   0,  40, 480, UI_KEY_MENU, -1},
    {      0,   0, 213, 250, UI_KEY_ENTER, 0},
    {    214,   0, 213, 250, UI_KEY_ENTER, 1},    
    {    427,   0, 173, 250, UI_KEY_ENTER, 2},  
    {      0, 250, 213, 230, UI_KEY_ENTER, 3},   
    {    214, 250, 213, 230, UI_KEY_ENTER, 4},  
    {    427, 250, 173, 230, UI_KEY_ENTER, 5},      
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Playback_Node[] =
{
    {  564,   0,  70,  60, UI_KEY_MENU,    -1},  
    {  240,   0,  40,  50, UI_KEY_LEFT,   -1},
    //{  220,   0,  50,  50, UI_KEY_ENTER,  -1},
    {  280,   0,  40,  50, UI_KEY_ENTER,  -1},
    {  320,   0,  40,  50, UI_KEY_STOP,   -1},
    {  360,   0,  40,  50, UI_KEY_RIGHT,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_SysSet_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {   32, 110, 260,  82, UI_KEY_ENTER,   0},
    {   32, 236, 260,  82, UI_KEY_ENTER,   1},
    {   32, 362, 260,  82, UI_KEY_ENTER,   2},
    {  384, 110, 260,  82, UI_KEY_ENTER,   3},
    {  384, 236, 260,  82, UI_KEY_ENTER,   4},
    {  384, 362, 260,  82, UI_KEY_ENTER,   5},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Pair_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  280,  90,  72,  80, UI_KEY_ENTER,   0},
    {  280, 190,  72,  80, UI_KEY_ENTER,   1},
    {  280, 280,  72,  80, UI_KEY_ENTER,   2},
    {  280, 350,  72,  80, UI_KEY_ENTER,   3},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Brightness_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1}, 
    
    {  218, 104, 46,  70, UI_KEY_UP,   0},
    {  264, 104, 46,  70, UI_KEY_UP,   1},
    {  314, 104, 46,  70, UI_KEY_UP,   2},
    {  354, 104, 46,  70, UI_KEY_UP,   3},
    {  396, 104, 46,  70, UI_KEY_UP,   4},
    {  440, 104, 46,  70, UI_KEY_UP,   5},
    
    {  218, 190, 46,  70, UI_KEY_DOWN,   0},
    {  264, 190, 46,  70, UI_KEY_DOWN,   1},
    {  314, 190, 46,  70, UI_KEY_DOWN,   2},
    {  354, 190, 46,  70, UI_KEY_DOWN,   3},
    {  396, 190, 46,  70, UI_KEY_DOWN,   4},
    {  440, 190, 46,  70, UI_KEY_DOWN,   5},
    
    {  218, 270, 46,  70, UI_KEY_LEFT,   0},
    {  264, 270, 46,  70, UI_KEY_LEFT,   1},
    {  314, 270, 46,  70, UI_KEY_LEFT,   2},
    {  354, 270, 46,  70, UI_KEY_LEFT,   3},
    {  396, 270, 46,  70, UI_KEY_LEFT,   4},
    {  440, 270, 46,  70, UI_KEY_LEFT,   5},
    
    {  218, 360, 46,  70, UI_KEY_RIGHT,   0},
    {  264, 360, 46,  70, UI_KEY_RIGHT,   1},
    {  314, 360, 46,  70, UI_KEY_RIGHT,   2},
    {  354, 360, 46,  70, UI_KEY_RIGHT,   3},
    {  396, 360, 46,  70, UI_KEY_RIGHT,   4},
    {  440, 360, 46,  70, UI_KEY_RIGHT,   5},

    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Resolution_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  280,  90,  60,  60, UI_KEY_UP,     0},
    {  410,  90,  60,  60, UI_KEY_UP,     1},
    {  280, 180,  60,  60, UI_KEY_DOWN,   0},
    {  410, 180,  60,  60, UI_KEY_DOWN,   1},
    {  280, 270,  60,  60, UI_KEY_LEFT,   0},
    {  410, 270,  60,  60, UI_KEY_LEFT,   1},
    {  280, 350,  60,  60, UI_KEY_RIGHT,  0},
    {  410, 350,  60,  60, UI_KEY_RIGHT,  1},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Camonoff_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  208, 100, 230,  70, UI_KEY_UP,     -1},
    {  208, 200, 230,  70, UI_KEY_DOWN,   -1},
    {  208, 280, 230,  70, UI_KEY_LEFT,   -1},
    {  208, 370, 230,  70, UI_KEY_RIGHT,  -1},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_TxAlarm_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  68,  260, 100,  80, UI_KEY_ENTER,   0},
    {  178, 260, 100,  80, UI_KEY_ENTER,   1},
    {  362, 260, 100,  80, UI_KEY_ENTER,   2},
    {  472, 260, 100,  80, UI_KEY_ENTER,   3},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_YesNo_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  200, 130,  80, 130, UI_KEY_RIGHT,   0},//flicker
    {  200, 250,  80, 130, UI_KEY_RIGHT,   1},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_SenseMode_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  280,  90,  60,  60, UI_KEY_UP,     0},
    {  410,  90,  60,  60, UI_KEY_UP,     1},
    {  520,  90,  60,  60, UI_KEY_UP,     2},
    {  280, 180,  60,  60, UI_KEY_DOWN,   0},
    {  410, 180,  60,  60, UI_KEY_DOWN,   1},
    {  520, 180,  60,  60, UI_KEY_DOWN,   2},
    {  280, 270,  60,  60, UI_KEY_LEFT,   0},
    {  410, 270,  60,  60, UI_KEY_LEFT,   1},
    {  520, 270,  60,  60, UI_KEY_LEFT,   2},
    {  280, 350,  60,  60, UI_KEY_RIGHT,  0},
    {  410, 350,  60,  60, UI_KEY_RIGHT,  1},
    {  520, 350,  60,  60, UI_KEY_RIGHT,  2},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_RecMode_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  280,  90,  60,  60, UI_KEY_UP,     0},
    //{  410,  90,  60,  60, UI_KEY_UP,     1},
    {  520,  90,  60,  60, UI_KEY_UP,     2},
    {  280, 180,  60,  60, UI_KEY_DOWN,   0},
    //{  410, 180,  60,  60, UI_KEY_DOWN,   1},
    {  520, 180,  60,  60, UI_KEY_DOWN,   2},
    {  280, 270,  60,  60, UI_KEY_LEFT,   0},
    //{  410, 270,  60,  60, UI_KEY_LEFT,   1},
    {  520, 270,  60,  60, UI_KEY_LEFT,   2},
    {  280, 350,  60,  60, UI_KEY_RIGHT,  0},
    //{  410, 350,  60,  60, UI_KEY_RIGHT,  1},
    {  520, 350,  60,  60, UI_KEY_RIGHT,  2},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Section_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  200, 120,  80,  90, UI_KEY_RIGHT,     0},
    {  200, 210,  80,  90, UI_KEY_RIGHT,     1},
    {  200, 310,  80,  90, UI_KEY_RIGHT,     2},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Schedule_Bar_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  184,   0,  56,  56, UI_KEY_LEFT,   -1},
    {  408,   0,  56,  56, UI_KEY_RIGHT,  -1},
#if(SUPPORT_TOUCH)
    {   88,  84, 532, 396, UI_KEY_ENTER,  -1},
#else
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
#endif    
};

UITOUCH_MENU_NODE_EVENT    gUITouch_ScheduleSet_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  336, 144,  56,  48, UI_KEY_UP,     -1},
    {  344, 260,  56,  48, UI_KEY_DOWN,   -1},
    {   20, 104,  60,  76, UI_KEY_ENTER,   0},/* Camera */
    {   20, 184,  60,  76, UI_KEY_ENTER,   1},
    {   20, 264,  60,  76, UI_KEY_ENTER,   2},
    {   20, 348,  60,  76, UI_KEY_ENTER,   3},
    {  182, 192,  60,  70, UI_KEY_ENTER,   4},/* Time */
    {  266, 192,  70,  70, UI_KEY_ENTER,   5},
    {  400, 192,  70,  70, UI_KEY_ENTER,   6},
    {  484, 192,  70,  70, UI_KEY_ENTER,   7},
    {  144, 310,  60,  60, UI_KEY_ENTER,   8},/* Day */
    {  210, 310,  60,  60, UI_KEY_ENTER,   9},
    {  272, 310,  60,  60, UI_KEY_ENTER,  10},
    {  340, 310,  60,  60, UI_KEY_ENTER,  11},
    {  404, 310,  60,  60, UI_KEY_ENTER,  12},
    {  468, 310,  60,  60, UI_KEY_ENTER,  13},
    {  532, 310,  60,  60, UI_KEY_ENTER,  14},
    {  425, 410,  80,  70, UI_KEY_ENTER,  16},/* delete */
    {  530, 410,  80,  70, UI_KEY_ENTER,  15},/* ok */
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Time_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  336, 148, 125,  72, UI_KEY_RIGHT,   0},//year
    {  265, 148,  64,  72, UI_KEY_RIGHT,   1},//month
    {  187, 148,  64,  72, UI_KEY_RIGHT,   2},//day
    {  208, 264,  64,  72, UI_KEY_RIGHT,   3},
    {  286, 264,  64,  72, UI_KEY_RIGHT,   4},
    {  368, 264,  64,  72, UI_KEY_RIGHT,   5},
    {  472, 148,  60,  76, UI_KEY_UP,     -1},
    {  472, 240,  60,  76, UI_KEY_DOWN,   -1},
    {  100, 260, 100,  76, UI_KEY_MODE,   -1},
#if SET_NTPTIME_TO_RTC   
    {  100, 170, 100,  76, UI_KEY_MAIN,   -1},
#endif
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Language_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  190, 130,  60,  60, UI_KEY_RIGHT,   0},
    {  190, 230,  60,  60, UI_KEY_RIGHT,   1},
    {  190, 340,  60,  60, UI_KEY_RIGHT,   2},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Upgrade_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  260, 140, 100,  70, UI_KEY_ENTER,   1},
    {  260, 250, 100,  70, UI_KEY_ENTER,   0},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Network_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {   30,  80, 300,  80, UI_KEY_RIGHT,   1},
    {  360,  80, 280,  80, UI_KEY_RIGHT,   0},
    {  110, 190, 115,  40, UI_KEY_ENTER,   0},
    {  110, 275, 115,  40, UI_KEY_ENTER,   1},
    {  110, 340, 115,  40, UI_KEY_ENTER,   2},
    {  285, 440,  90,  40, UI_KEY_ENTER,   3},
    {  215, 265, 245,  70, UI_KEY_ENTER,   4},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Keypad_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  132, 192, 132,  70, UI_KEY_ENTER,  1},
    {  264, 192, 132,  70, UI_KEY_ENTER,  2},
    {  396, 192, 132,  70, UI_KEY_ENTER,  3},
    {  132, 262, 132,  70, UI_KEY_ENTER,  4},
    {  264, 262, 132,  70, UI_KEY_ENTER,  5},
    {  396, 262, 132,  70, UI_KEY_ENTER,  6},
    {  132, 332, 132,  70, UI_KEY_ENTER,  7},
    {  264, 332, 132,  70, UI_KEY_ENTER,  8},
    {  396, 332, 132,  70, UI_KEY_ENTER,  9},
    {  132, 402, 132,  70, UI_KEY_ENTER, 10},//.
    {  264, 402, 132,  70, UI_KEY_ENTER,  0},    
    {  396, 402, 132,  70, UI_KEY_ENTER, 11},// <-
    {  544, 420, 100,  70, UI_KEY_ENTER, 12},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Calendar_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_PLAY,    -1},  
    {    320,  80,  40,  42, UI_KEY_LEFT, 0},  /*month -*/
    {    140,  80,  40,  42, UI_KEY_LEFT, 1},  /*year -*/
    {    480,  80,  40,  42, UI_KEY_RIGHT,0},  /*month +*/
    {    280,  80,  40,  42, UI_KEY_RIGHT,1},  /*year +*/
    {     20, 128,  54,  80, UI_KEY_DOWN, 0},  /*Camera*/
    {     20, 212,  54,  80, UI_KEY_DOWN, 1},  
    {     20, 296,  54,  80, UI_KEY_DOWN, 2},  
    {     20, 380,  54,  80, UI_KEY_DOWN, 3},
    {    590, 128,  54,  80, UI_KEY_UP,   0},  /*REC TYPE*/
    {    590, 212,  54,  80, UI_KEY_UP,   1},  
    {    590, 296,  54,  80, UI_KEY_UP,   2},  
    {    590, 380,  54,  80, UI_KEY_UP,   3},
    {     83, 150, 500, 330, UI_KEY_ENTER, -1}, /*Calendar Day*/ 
    
    //{      0, 152, 640, 328, UI_KEY_OK,  -1},  
};

UITOUCH_MENU_NODE_EVENT    gUITouch_PlayList_Node[] =
{
    {  580,   4,  60,  60, UI_KEY_MENU,    -1},  
    {  520,   0,  50,  50, UI_KEY_DELETE, -1},
    {  600,  80,  50,  50, UI_KEY_LEFT,   -1},
    {  600, 250,  50,  50, UI_KEY_RIGHT,  -1},
    {   90,  50, 450,  40, UI_KEY_ENTER,   0},
    {   90,  90, 450,  30, UI_KEY_ENTER,   1},
    {   90, 120, 450,  30, UI_KEY_ENTER,   2},
    {   90, 150, 450,  30, UI_KEY_ENTER,   3},
    {   90, 180, 450,  30, UI_KEY_ENTER,   4},
    {   90, 220, 450,  30, UI_KEY_ENTER,   5},
    {   90, 250, 450,  30, UI_KEY_ENTER,   6},
    {   90, 280, 450,  30, UI_KEY_ENTER,   7},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Info_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1}, 
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Static_IP_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1}, 
    {  260, 150, 220,  60, UI_KEY_ENTER,         0},  /*ip*/
    {  260, 220, 220,  60, UI_KEY_ENTER,         1},  /*mask*/
    {  260, 290, 220,  60, UI_KEY_ENTER,         2},  /*gateway*/
    {  500, 410, 120,  70, UI_KEY_ENTER,        -1},  /*ok*/
};

UITOUCH_MENU_NODE_EVENT    gUITouch_SubMenu_2_Node[] =
{
    {    580,   4,  60,   60, UI_KEY_MENU,    -1},  
    {    200,  190, 200,  60, UI_KEY_UP,       0},
    {    200,  250, 200,  60, UI_KEY_UP,       1},
    {    400,  360, 100, 100, UI_KEY_ENTER,   -1},
};

UITOUCH_MENU_NODE_EVENT    gUITouch_Time_Zone_Node[] =
{
    {   12,   4,  72,  60, UI_KEY_MENU,         -1},
    {  564,   4,  76,  60, UI_KEY_ENTER_PRV,    -1},  
    {  260, 200,  40,  72, UI_KEY_RIGHT,   0},//minus
    {  300, 200,  72,  72, UI_KEY_RIGHT,   1},//hour
    {  372, 200,  72,  72, UI_KEY_RIGHT,   2},//min
    {  472, 160,  60,  90, UI_KEY_UP,     -1},
    {  472, 250,  60,  90, UI_KEY_DOWN,   -1},
    {  500, 410, 120,  70, UI_KEY_ENTER,  -1},
};
    
UITOUCH_NODE_EVENT_TBL gUISetupTouchEventCheckTbl[] =
{
/*00*/   { gUITouch_Preview_Node,        sizeof(gUITouch_Preview_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*01*/   { gUITouch_Quad_Node,           sizeof(gUITouch_Quad_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*02*/   { gUITouch_Menu_Node,           sizeof(gUITouch_Menu_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*03*/   { gUITouch_Playback_Node,       sizeof(gUITouch_Playback_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*04*/   { gUITouch_SysSet_Node,         sizeof(gUITouch_SysSet_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*05*/   { gUITouch_Pair_Node,           sizeof(gUITouch_Pair_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*06*/   { gUITouch_Brightness_Node,     sizeof(gUITouch_Brightness_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*07*/   { gUITouch_Resolution_Node,     sizeof(gUITouch_Resolution_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*08*/   { gUITouch_Camonoff_Node,       sizeof(gUITouch_Camonoff_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*09*/   { gUITouch_TxAlarm_Node,        sizeof(gUITouch_TxAlarm_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*10*/   { gUITouch_YesNo_Node,          sizeof(gUITouch_YesNo_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*11*/   { gUITouch_RecMode_Node,        sizeof(gUITouch_RecMode_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*12*/   { gUITouch_Section_Node,        sizeof(gUITouch_Section_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*13*/   { gUITouch_Schedule_Bar_Node,   sizeof(gUITouch_Schedule_Bar_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*14*/   { gUITouch_ScheduleSet_Node,    sizeof(gUITouch_ScheduleSet_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*15*/   { gUITouch_Time_Node,           sizeof(gUITouch_Time_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*16*/   { gUITouch_Language_Node,       sizeof(gUITouch_Language_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*17*/   { gUITouch_Upgrade_Node,        sizeof(gUITouch_Upgrade_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*18*/   { gUITouch_Keypad_Node,         sizeof(gUITouch_Keypad_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
#if UI_CALENDAR_SUPPORT
/*19*/   { gUITouch_Calendar_Node,       sizeof(gUITouch_Calendar_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
#else
/*19*/   { gUITouch_PlayList_Node,       sizeof(gUITouch_PlayList_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
#endif
/*20*/   { gUITouch_PlayList_Node,       sizeof(gUITouch_PlayList_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*21*/   { gUITouch_Info_Node,           sizeof(gUITouch_Info_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*22*/   { gUITouch_Static_IP_Node,      sizeof(gUITouch_Static_IP_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*23*/   { gUITouch_SubMenu_2_Node,      sizeof(gUITouch_SubMenu_2_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*24*/   { gUITouch_Time_Zone_Node,      sizeof(gUITouch_Time_Zone_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
/*25*/   { gUITouch_SenseMode_Node,      sizeof(gUITouch_SenseMode_Node)/sizeof(UITOUCH_MENU_NODE_EVENT)},
};

UI_NODE_DATA uiMenuNodeListItem[] =
{
    /*level 0*/
    {"Preview",     DataNum(Preview_NODE),  Preview_NODE, NULL,   UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[0]},
    {"Quad",        DataNum(Preview_NODE),  Preview_NODE, NULL,   UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[1]},
    {"Menu",        DataNum(Preview_NODE),  Preview_NODE, NULL,   UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},
    {"Play File",   DataNum(Preview_NODE),  Preview_NODE, NULL,   UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[3]},


     /*level 1*/
    {"Camera Set",      DataNum(CAMERA_NODE),           CAMERA_NODE,            NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},
    {"Record Set",      DataNum(REC_SETTING_NODE),      REC_SETTING_NODE,       NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},
    {"PlayBack",        DataNum(PLAYBACK_NODE),         PLAYBACK_NODE,          NULL, UI_MENU_SETIDX_PLAYBACK, &gUISetupTouchEventCheckTbl[2]},
    {"Storage Set",     DataNum(Storage_SETTING_NODE),  Storage_SETTING_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},
    {"System Set",      DataNum(SYSTEM_SETTING_NODE),   SYSTEM_SETTING_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},
    {"System Info",     DataNum(SYSTEM_Info_NODE),      SYSTEM_Info_NODE,       NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[2]},

    /*level 2*/
    {"Pairing",         DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_PAIRING,           &gUISetupTouchEventCheckTbl[4]},
    {"Resolution",      DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_RESOLUTION_CH1,    &gUISetupTouchEventCheckTbl[4]},
    #if((UI_LIGHT_SUPPORT) || (UI_CAMERA_ALARM_SUPPORT))
    {"Camera Alarm",    DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,         &gUISetupTouchEventCheckTbl[4]},
    #endif
    {"Camera On/Off",   DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_CH1_ON,            &gUISetupTouchEventCheckTbl[4]},
    {"Brightness",      DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_BRIGHTNESS_CH1,    &gUISetupTouchEventCheckTbl[4]},
    {"Anti-flicker",    DataNum(Pairing_NODE),  Pairing_NODE,   NULL, UI_MENU_SETIDX_50HZ_60HZ,         &gUISetupTouchEventCheckTbl[4]},

    {"REC Mode",        DataNum(REC_Mode_NODE),      REC_Mode_NODE,     NULL, UI_MENU_SETIDX_REC_MODE_CH1,  &gUISetupTouchEventCheckTbl[4]},
    {"Sensitive",       DataNum(REC_Mode_NODE),      REC_Mode_NODE,     NULL, UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1, &gUISetupTouchEventCheckTbl[4]},
    {"Section",         DataNum(REC_Mode_NODE),      REC_Mode_NODE,     NULL, UI_MENU_SETIDX_MOTION_SECTION,       &gUISetupTouchEventCheckTbl[4]},
    {"Scheduled",       DataNum(REC_Mode_NODE),      REC_Mode_NODE,     NULL, UI_MENU_SETIDX_SCHEDULED,     &gUISetupTouchEventCheckTbl[4]},
    #if(UI_BAT_SUPPORT)
    {"BatCam Scheduled",       DataNum(REC_Mode_NODE),      REC_Mode_NODE,     NULL, UI_MENU_SETIDX_SCHEDULED,     &gUISetupTouchEventCheckTbl[4]},
    #endif
    
    {"Calendar",        DataNum(PLAYBACK_Mode_NODE),    PLAYBACK_Mode_NODE, NULL, UI_MENU_SETIDX_NO_ACTION,  &gUISetupTouchEventCheckTbl[19]},

    {"Storage Overwrite",   DataNum(Overwirte_NODE),    Overwirte_NODE,     NULL, UI_MENU_SETIDX_OVERWRITE, &gUISetupTouchEventCheckTbl[4]},
    {"Storage Format",      DataNum(Overwirte_NODE),    Overwirte_NODE,     NULL, UI_MENU_SETIDX_FORMAT,    &gUISetupTouchEventCheckTbl[4]},
    {"Storage Info",        DataNum(Overwirte_NODE),    Overwirte_NODE,     NULL, UI_MENU_SETIDX_CARDINFO,  &gUISetupTouchEventCheckTbl[4]},
#if USB_HOST_MASS_SUPPORT
    {"HDD Remove",          DataNum(Overwirte_NODE),    Overwirte_NODE,     NULL, UI_MENU_SETIDX_HDD_REMOVE,  &gUISetupTouchEventCheckTbl[4]},
#endif

    {"Date Time",       DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_DATE_TIME,     &gUISetupTouchEventCheckTbl[4]},
    {"Language",        DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_LANGUAGE,      &gUISetupTouchEventCheckTbl[4]},
    {"Default",         DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_DEFAULT,       &gUISetupTouchEventCheckTbl[4]},
    {"Update",          DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[4]},
    {"Network",         DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_NO_ACTION,       &gUISetupTouchEventCheckTbl[4]},
    {"Monitor Alarm",   DataNum(Date_Time_NODE),    Date_Time_NODE, NULL, UI_MENU_SETIDX_ALARM,         &gUISetupTouchEventCheckTbl[4]},

    {"Version Info",    DataNum(Version_Info_NODE), Version_Info_NODE,  NULL, UI_MENU_SETIDX_VERSION_INFO,     &gUISetupTouchEventCheckTbl[4]},
    {"Network Info",    DataNum(Version_Info_NODE), Version_Info_NODE,  NULL, UI_MENU_SETIDX_NETWORK_INFO, &gUISetupTouchEventCheckTbl[4]},
    {"APP Info",        DataNum(Version_Info_NODE), Version_Info_NODE,  NULL, UI_MENU_SETIDX_APP_INFO, &gUISetupTouchEventCheckTbl[4]},


    /*level 3*/
    {"Pair Cam1",                DataNum(SET_PAIR_CAM_NODE),     SET_PAIR_CAM_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[5]},
    {"Pair Cam2",                DataNum(SET_PAIR_CAM_NODE),     SET_PAIR_CAM_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[5]},
    {"Pair Cam3",                DataNum(SET_PAIR_CAM_NODE),     SET_PAIR_CAM_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[5]},
    {"Pair Cam4",                DataNum(SET_PAIR_CAM_NODE),     SET_PAIR_CAM_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[5]},
    {"Set Resolution",           DataNum(SET_Resolution_NODE),   SET_Resolution_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[7]},
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {"TX Light On/Off",          DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_CH1_LS_ONOFF, &gUISetupTouchEventCheckTbl[9]},
    {"TX Light Sch",             DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_SCHEDULED, &gUISetupTouchEventCheckTbl[9]},
    {"TX Alarm On/Off",          DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_CH1_CA_ONOFF, &gUISetupTouchEventCheckTbl[9]},
    {"TX Alarm Sch",             DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_SCHEDULED, &gUISetupTouchEventCheckTbl[9]},
    #elif (UI_LIGHT_SUPPORT)
    {"TX Light On/Off",          DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_CH1_LS_ONOFF, &gUISetupTouchEventCheckTbl[9]},
    {"TX Light Sch",             DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_SCHEDULED, &gUISetupTouchEventCheckTbl[9]},
    #elif (UI_CAMERA_ALARM_SUPPORT)
    {"TX Alarm On/Off",          DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_CH1_CA_ONOFF, &gUISetupTouchEventCheckTbl[9]},
    {"TX Alarm Sch",             DataNum(SET_CamAlarm_NODE),     SET_CamAlarm_NODE,      NULL, UI_MENU_SETIDX_SCHEDULED, &gUISetupTouchEventCheckTbl[9]},
    #endif
    {"Set Camera On/Off",        DataNum(SET_CAM_ON_OFF_NODE),   SET_CAM_ON_OFF_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[8]},
    {"Set Brightness",           DataNum(SET_Brightness_NODE),   SET_Brightness_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[6]},
    {"Set Flicker 50",           DataNum(SET_Flicker_50_NODE),   SET_Flicker_50_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Flicker 60",           DataNum(SET_Flicker_60_NODE),   SET_Flicker_60_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},

    {"Set Rec Mode",           DataNum(SET_REC_MODE_NODE),     SET_REC_MODE_NODE,        NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[11]},
    {"Set Sensitive",          DataNum(SET_Sensitive_NODE),    SET_Sensitive_NODE,       NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[25]},
    {"Set Section 15",         DataNum(SET_Section_15_NODE),   SET_Section_15_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[12]},
    {"Set Section 30",         DataNum(SET_Section_30_NODE),   SET_Section_30_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[12]},
    {"Set Section 60",         DataNum(SET_Section_60_NODE),   SET_Section_60_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,     &gUISetupTouchEventCheckTbl[12]},
    {"Scheduled Bar",          DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE,       NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    #if(UI_BAT_SUPPORT)
    {"BatCam Scheduled Bar",   DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE,       NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    #endif
    
    {"Playback List",       DataNum(Playback_List_NODE),    Playback_List_NODE,         NULL, UI_MENU_SETIDX_PLAYBACK, &gUISetupTouchEventCheckTbl[20]},

    {"Set Overwrite yes",   DataNum(SET_Overwrite_Yes_NODE),   SET_Overwrite_Yes_NODE, NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Overwrite no",    DataNum(SET_Overwrite_No_NODE),    SET_Overwrite_No_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Format yes",      DataNum(SET_Format_Yes_NODE),      SET_Format_Yes_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Format no",       DataNum(SET_Format_No_NODE),       SET_Format_No_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Storage Info ",       DataNum(SET_Storage_Info_NODE),    SET_Storage_Info_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[21]},
#if USB_HOST_MASS_SUPPORT
    {"Set HDD Remove yes",             DataNum(SET_Default_Yes_NODE),     SET_Default_Yes_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set HDD Remove no",              DataNum(SET_Default_No_NODE),      SET_Default_No_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
#endif

#if SET_NTPTIME_TO_RTC
    {"Set Date Time",               DataNum(SET_DateTime_NODE),     SET_DateTime_NODE,        NULL, UI_MENU_SETIDX_NTP, &gUISetupTouchEventCheckTbl[15]},
#else
    {"Set Date Time",               DataNum(SET_DateTime_NODE),     SET_DateTime_NODE,        NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[15]},
#endif
    {"Set Language English",        DataNum(SET_English_NODE),      SET_English_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[16]},
    {"Set Language Italy",          DataNum(SET_Italy_NODE),        SET_Italy_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[16]},
    {"Set Language French",         DataNum(SET_French_NODE),       SET_French_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[16]},
    {"Set Default yes",             DataNum(SET_Default_Yes_NODE),      SET_Default_Yes_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Default no",              DataNum(SET_Default_No_NODE),      SET_Default_No_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Upgrade Sever",       DataNum(Upgrade_NODE),          Upgrade_NODE,       NULL, UI_MENU_SETIDX_UPGRADE_FW_NET, &gUISetupTouchEventCheckTbl[17]},
    {"Upgrade SD",          DataNum(Upgrade_NODE),          Upgrade_NODE,       NULL, UI_MENU_SETIDX_UPGRADE_FW, &gUISetupTouchEventCheckTbl[17]},
    {"DHCP",                DataNum(Network_NODE),          Network_NODE,       NULL, UI_MENU_SETIDX_NETWORK_STATUS, &gUISetupTouchEventCheckTbl[17]},
    {"Static IP",           DataNum(Network_NODE),          Network_NODE,       NULL, UI_MENU_SETIDX_ST_IP_SET, &gUISetupTouchEventCheckTbl[17]},
    {"Set Monitor Alarm yes",   DataNum(SET_Monitor_Alarm_Yes_NODE),    SET_Monitor_Alarm_Yes_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,    &gUISetupTouchEventCheckTbl[10]},
    {"Set Monitor Alarm no",    DataNum(SET_Monitor_Alarm_No_NODE),    SET_Monitor_Alarm_No_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION,    &gUISetupTouchEventCheckTbl[10]},

    {"Set Device Info",     DataNum(SET_VersionInfo_NODE),  SET_VersionInfo_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[21]},
    {"Set Network Info",    DataNum(SET_NetworkInfo_NODE),  SET_NetworkInfo_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[21]},
    {"Set APP Info ",       DataNum(SET_AppInfo_NODE),      SET_AppInfo_NODE,       NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[21]},


    /*level 4 */
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {"Set TX Light On/Off",     DataNum(SET_Light_NODE),        SET_Light_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[8]},
    {"TX Light Sch Bar",        DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE, NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    {"Set TX Alarm On/Off",     DataNum(SET_TX_Alarm_NODE),     SET_TX_Alarm_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[8]},
    {"TX Alarm Sch Bar",        DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE, NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    #elif (UI_LIGHT_SUPPORT)
    {"Set TX Light On/Off",     DataNum(SET_Light_NODE),        SET_Light_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[8]},
    {"TX Light Sch Bar",        DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE, NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    #elif (UI_CAMERA_ALARM_SUPPORT)
    {"Set TX Alarm On/Off",     DataNum(SET_TX_Alarm_NODE),     SET_TX_Alarm_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[8]},
    {"TX Alarm Sch Bar",        DataNum(Scheduled_Bar_NODE),    Scheduled_Bar_NODE, NULL, UI_MENU_SETIDX_SCHEDULED_SET, &gUISetupTouchEventCheckTbl[13]},
    #endif
    
    {"Set Scheduled",           DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION,    &gUISetupTouchEventCheckTbl[14]},

    #if(UI_BAT_SUPPORT)
    {"Set BatCam Scheduled",    DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION,    &gUISetupTouchEventCheckTbl[14]},
    #endif
    
    {"Delete Yes",      DataNum(DELETE_YES_NODE),   DELETE_YES_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[23]},
    {"Delete No",       DataNum(DELETE_NO_NODE),    DELETE_NO_NODE,       NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[23]},

#if SET_NTPTIME_TO_RTC
    {"Set Time Zone",           DataNum(SET_TimeZone_NODE),     SET_TimeZone_NODE,        NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[24]},
#endif

    {"Set Upgrade Sever yes",   DataNum(SET_Upgrade_Sever_Yes_NODE),   SET_Upgrade_Sever_Yes_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Upgrade Sever no",    DataNum(SET_Upgrade_Sever_No_NODE),    SET_Upgrade_Sever_No_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Upgrade SD yes",      DataNum(SET_Upgrade_SD_Yes_NODE),      SET_Upgrade_SD_Yes_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Upgrade SD no",       DataNum(SET_Upgrade_SD_No_NODE),       SET_Upgrade_SD_No_NODE,        NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},

    {"Set DHCP yes",            DataNum(SET_DHCP_Yes_NODE),         SET_DHCP_Yes_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set DHCP no",             DataNum(SET_DHCP_No_NODE),          SET_DHCP_No_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[10]},
    {"Set Static",              DataNum(SET_Static_NODE),           SET_Static_NODE,          NULL, UI_MENU_SETIDX_NETWORK_KEYPAD, &gUISetupTouchEventCheckTbl[22]},


    /*level 5 */
    #if((UI_LIGHT_SUPPORT) && (UI_CAMERA_ALARM_SUPPORT))
    {"Set TX Light Sch",    DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[14]},
    {"Set TX Alarm Sch",    DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[14]},
    #elif (UI_LIGHT_SUPPORT)
    {"Set TX Light Sch",    DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[14]},
    #elif (UI_CAMERA_ALARM_SUPPORT)
    {"Set TX Alarm Sch",    DataNum(SET_Schedule_NODE),     SET_Schedule_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[14]},
    #endif
    
    {"Keypad",              DataNum(SET_Keypad_NODE),       SET_Keypad_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION, &gUISetupTouchEventCheckTbl[18]},


};


