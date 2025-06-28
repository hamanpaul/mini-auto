/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_graph_project.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "board.h"
#include "uiapi.h"
#include "ui.h"
#include "dcfapi.h"
#include "jpegapi.h"
#include "ui_project.h"
#include "spiapi.h"
#include "sysapi.h"
#include "gpioapi.h"
#include "rfiuapi.h"
#include "osd_draw_project.h"
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



UI_NODE_PHOTO TimeNum[40] =
{
    {"time00.jpg",0},
    {"time01.jpg",0},
    {"time02.jpg",0},
    {"time03.jpg",0},
    {"time04.jpg",0},
    {"time05.jpg",0},
    {"time06.jpg",0},
    {"time07.jpg",0},
    {"time08.jpg",0},
    {"time09.jpg",0},
    {"time10.jpg",0},
    {"time11.jpg",0},
    {"time12.jpg",0},
    {"time13.jpg",0},
    {"time14.jpg",0},
    {"time15.jpg",0},
    {"time16.jpg",0},
    {"time17.jpg",0},
    {"time18.jpg",0},
    {"time19.jpg",0},
    {"time20.jpg",0},
    {"time21.jpg",0},
    {"time22.jpg",0},
    {"time23.jpg",0},
    {"time24.jpg",0},
    {"time25.jpg",0},
    {"time26.jpg",0},
    {"time27.jpg",0},
    {"time28.jpg",0},
    {"time29.jpg",0},
    {"time30.jpg",0},
    {"time31.jpg",0},
    {"time32.jpg",0},
    {"time33.jpg",0},
    {"time34.jpg",0},
    {"time35.jpg",0},
    {"time36.jpg",0},
    {"time37.jpg",0},
    {"time38.jpg",0},
    {"time39.jpg",0},
};

UI_NODE_PHOTO PairNum[10]=
{
    {"time00.jpg",0},
    {"time01.jpg",0},
    {"time02.jpg",0},
    {"time03.jpg",0},
    {"time04.jpg",0},
    {"time05.jpg",0},
    {"time06.jpg",0},
    {"time07.jpg",0},
    {"time08.jpg",0},
    {"time09.jpg",0},
};

#if SET_NTPTIME_TO_RTC
UI_NODE_PHOTO NTP_Image[6]=
{
    {"syndt2_1.jpg",0},
    {"syndt1_1.jpg",0},        
    {"syndt2_2.jpg",0},
    {"syndt1_2.jpg",0},
    {"syndt2_3.jpg",0},
    {"syndt1_3.jpg",0},
};

UI_NODE_PHOTO TimeZoneBtn[3]=
{
    {"zone_1.jpg",0},
    {"zone_2.jpg",0},
    {"zone_3.jpg",0},
};

UI_NODE_PHOTO NumOp[] =
{
    {"pls00.jpg",0},
    {"min01.jpg",0},
    {"pls10.jpg",0},
    {"min11.jpg",0},
};
#endif

UI_NODE_PHOTO RECMode_Image[]=
{
    {"mnul1_m1.jpg",0},    /* 0 */
    {"mot1_m1.jpg",0},    /* 1 */
    {"sch1_m1.jpg",0},      /* 2 */
    {"mnul2_m1.jpg",0},    /* 3 */
    {"mot2_m1.jpg",0},    /* 4 */
    {"sch2_m1.jpg",0},      /* 5 */     
    
    {"mnul1_m2.jpg",1},    /* 0 */
    {"mot1_m2.jpg",1},    /* 1 */
    {"sch1_m2.jpg",1},      /* 2 */
    {"mnul2_m2.jpg",1},    /* 3 */
    {"mot2_m2.jpg",1},    /* 4 */
    {"sch2_m2.jpg",1},      /* 5 */     
    
    {"mnul1_m3.jpg",2},    /* 0 */
    {"mot1_m3.jpg",2},    /* 1 */
    {"sch1_m3.jpg",2},      /* 2 */
    {"mnul2_m3.jpg",2},    /* 3 */
    {"mot2_m3.jpg",2},    /* 4 */
    {"sch2_m3.jpg",2},      /* 5 */     
};

UI_NODE_PHOTO Sensitivity_Image[]=
{
    {"low1_m1.jpg",0},    /* 0 */
    {"mid1_m1.jpg",0},    /* 1 */
    {"high1_m1.jpg",0},   /* 2 */
    {"low2_m1.jpg",0},    /* 3 */
    {"mid2_m1.jpg",0},    /* 4 */
    {"high2_m1.jpg",0},   /* 5 */ 

    {"low1_m2.jpg",0},    /* 0 */
    {"mid1_m2.jpg",0},    /* 1 */
    {"high1_m2.jpg",0},   /* 2 */
    {"low2_m2.jpg",0},    /* 3 */
    {"mid2_m2.jpg",0},    /* 4 */
    {"high2_m2.jpg",0},   /* 5 */ 

    {"low1_m3.jpg",0},    /* 0 */
    {"mid1_m3.jpg",0},    /* 1 */
    {"high1_m3.jpg",0},   /* 2 */
    {"low2_m3.jpg",0},    /* 3 */
    {"mid2_m3.jpg",0},    /* 4 */
    {"high2_m3.jpg",0},   /* 5 */ 
};

UI_NODE_PHOTO OnOff_Image[]=
{
    {"off1_m1.jpg",0},    /* close */
    {"off2_m1.jpg",0},     
    {"off3_m1.jpg",0},     
    {"off4_m1.jpg",0},     
    {"on1_m1.jpg",0},     
    {"on2_m1.jpg",0},     
    {"on3_m1.jpg",0},     
    {"on4_m1.jpg",0},     
};

UI_NODE_PHOTO Resolution_BG[]=
{
    {"off1_m1.jpg",0},    /* close */
    {"on1_m1.jpg",0},     
    {"off1_m1.jpg",0},    /* close */
    {"on1_m1.jpg",0},     
};

UI_NODE_PHOTO Resolution_Image[]=
{
    {"fhd1_m1.jpg",0},     /* 0 */
    {"hd1_m1.jpg",0},      /* 1 */
    {"fhd2_m1.jpg",0},     /* 2 */
    {"hd2_m1.jpg",0},      /* 3 */

};

UI_NODE_PHOTO Select_Image = {"select.jpg",0};// green rectangle   

UI_NODE_PHOTO Select_Item[12] = {
    {"select00.jpg",0}, 
    {"select01.jpg",0}, 
    {"select02.jpg",0}, 
    {"select10.jpg",0}, 
    {"select11.jpg",0}, 
    {"select12.jpg",0}, 
    {"select20.jpg",0}, 
    {"select21.jpg",0}, 
    {"select22.jpg",0}, 
    {"select30.jpg",0}, 
    {"select31.jpg",0}, 
    {"select32.jpg",0}, 
};

UI_NODE_PHOTO YesNo_Image[] = { // red rectangle  
    {"redSel20.jpg",0}, //empty
    {"redSel31.jpg",0}, 
};

UI_NODE_PHOTO Brightness_Bar_Image[]=
{
    {"seleBar1.jpg",0},     /* 0 */
    {"seleBar2.jpg",0},     /* 1 */
    {"seleBar3.jpg",0},     /* 2 */
    {"seleBar4.jpg",0},     /* 3 */

};

UI_NODE_PHOTO Flicker_BG={"flick1.jpg",0};

UI_NODE_PHOTO Flicker_Image[2]=
{
    {"flick1.jpg",0},     /* no selet */
    {"flick2.jpg",0},   
};

/* Network */
UI_NODE_PHOTO Dynmic_OFF[]=
{
    {"dyip2_m1.jpg",0},
    {"dyip2_m2.jpg",0},
    {"dyip2_m3.jpg",0},
};
UI_NODE_PHOTO Dynmic_ON[]=
{
    {"dyip1_m1.jpg",0},
    {"dyip1_m2.jpg",0},
    {"dyip1_m3.jpg",0},
};
UI_NODE_PHOTO Static_BG[]= 
{
    {"stip_bg1.jpg",0},
    {"stip_bg2.jpg",0},
    {"stip_bg3.jpg",0},
};
UI_NODE_PHOTO Txt_Address_1_M[]=
{
    {"addr1_m1.jpg",0},
    {"addr1_m2.jpg",0},
    {"addr1_m3.jpg",0},
};
UI_NODE_PHOTO Txt_Address_2_M[]=
{
    {"addr2_m1.jpg",0},
    {"addr2_m2.jpg",0},
    {"addr2_m3.jpg",0},
};
UI_NODE_PHOTO Txt_Mask_1_M[]=          
{
    {"msip1_m1.jpg",0},
    {"msip1_m2.jpg",0},
    {"msip1_m3.jpg",0},

};
UI_NODE_PHOTO Txt_Mask_2_M[]=          
{
    {"msip2_m1.jpg",0},
    {"msip2_m2.jpg",0},
    {"msip2_m3.jpg",0},
 };   
UI_NODE_PHOTO Txt_Gateway_1_M[]=       
{
    {"gate1_m1.jpg",0},
    {"gate1_m2.jpg",0},
    {"gate1_m3.jpg",0},    
};    
UI_NODE_PHOTO Txt_Gateway_2_M[]=       
{
    {"gate2_m1.jpg",0},
    {"gate2_m2.jpg",0},    
    {"gate2_m3.jpg",0},
};
UI_NODE_PHOTO Txt_DNS1_1_M[]=         
{
    {"dns_1_m1.jpg",0},
    {"dns_1_m2.jpg",0},
    {"dns_1_m3.jpg",0},

};
UI_NODE_PHOTO Txt_DNS1_2_M[]=          
{
    {"dns_2_m1.jpg",0},
    {"dns_2_m2.jpg",0},
    {"dns_2_m3.jpg",0},

};
UI_NODE_PHOTO Txt_DNS2_1_M[]=         
{
    {"dn2_1_m1.jpg",0},
    {"dn2_1_m2.jpg",0},
    {"dn2_1_m3.jpg",0},
};
UI_NODE_PHOTO Txt_DNS2_2_M[]=          
{
    {"dn2_2_m1.jpg",0},
    {"dn2_2_m2.jpg",0},
    {"dn2_2_m3.jpg",0},

};
UI_NODE_PHOTO BTN_OK_1_M[]=            
{
    {"btn1_m1.jpg",0},
    {"btn1_m2.jpg",0},
    {"btn1_m3.jpg",0},  
};
UI_NODE_PHOTO BTN_OK_2_M[]=            
{
    {"btn2_m1.jpg",0},
    {"btn2_m2.jpg",0},
    {"btn2_m3.jpg",0},
};

UI_NODE_PHOTO busy_ON[]=              
{
    {"busy0001.jpg",0},
    {"busy0002.jpg",0},
    {"busy0003.jpg",0},
};

/* Scheduled Table*/
UI_NODE_PHOTO ScheduledTitle_Image []=
{
    {"mon_t1.jpg",0},     /* 0 */
    {"tue_t1.jpg",0},     /* 1 */
    {"wed_t1.jpg",0},     /* 2 */
    {"thr_t1.jpg",0},     /* 3 */
    {"fri_t1.jpg",0},     /* 4 */
    {"sat_t1.jpg",0},     /* 5 */ 
    {"sun_t1.jpg",0},     /* 6 */

    {"mon_t2.jpg",0},     /* 0 */
    {"tue_t2.jpg",0},     /* 1 */
    {"wed_t2.jpg",0},     /* 2 */
    {"thr_t2.jpg",0},     /* 3 */
    {"fri_t2.jpg",0},     /* 4 */
    {"sat_t2.jpg",0},     /* 5 */ 
    {"sun_t2.jpg",0},     /* 6 */

    {"mon_t3.jpg",0},     /* 0 */
    {"tue_t3.jpg",0},     /* 1 */
    {"wed_t3.jpg",0},     /* 2 */
    {"thr_t3.jpg",0},     /* 3 */
    {"fri_t3.jpg",0},     /* 4 */
    {"sat_t3.jpg",0},     /* 5 */ 
    {"sun_t3.jpg",0},     /* 6 */
};

UI_NODE_PHOTO Scheduled_Table_BG[]=
{
    {"sch_bg1.jpg",0},
    {"sch_bg2.jpg",0},
    {"sch_bg3.jpg",0},
};

UI_NODE_PHOTO Keypad_1_BG           ={"typebg_1.jpg",0};
UI_NODE_PHOTO Dot                   ={"dot01_m1.jpg",0};
UI_NODE_PHOTO Right_1_M             ={"rig1_m1.jpg",0};
UI_NODE_PHOTO Right_2_M             ={"rig2_m1.jpg",0};
UI_NODE_PHOTO Left_1_M              ={"left1_m1.jpg",0};
UI_NODE_PHOTO Left_2_M              ={"left2_m1.jpg",0};
UI_NODE_PHOTO grey_item             ={"grey.jpg",0};
UI_NODE_PHOTO yellow_item={"yellow1.jpg",0}; 
UI_NODE_PHOTO schbar={"schbar.jpg",0}; 

UI_NODE_PHOTO Modify_1_M[]=
{
    {"modb_s1.jpg",0},
    {"modb_s2.jpg",0},
    {"modb_s3.jpg",0},
};

UI_NODE_PHOTO Modify_2_M []=
{
    {"modg_s1.jpg",0},
    {"modg_s2.jpg",0},
    {"modg_s3.jpg",0},
};

/* Scheduled Setting */

UI_NODE_PHOTO Camera_Btn[]=
{
    {"cam1g_s1.jpg",0},     /* 0 */
    {"cam2g_s1.jpg",0},     /* 1 */
    {"cam3g_s1.jpg",0},     /* 2 */
    {"cam4g_s1.jpg",0},     /* 2 */  
    {"cam1b_s1.jpg",0},     /* 4 */
    {"cam2b_s1.jpg",0},     /* 5 */
    {"cam3b_s1.jpg",0},     /* 6 */
    {"cam4b_s1.jpg",0},     /* 7 */
};

UI_NODE_PHOTO Day_Btn[]=
{
   
    {"mong_s1.jpg",0},     /* 0 */
    {"tueg_s1.jpg",0},     /* 1 */
    {"wedg_s1.jpg",0},     /* 2 */
    {"thug_s1.jpg",0},     /* 3 */
    {"frig_s1.jpg",0},     /* 4 */
    {"satg_s1.jpg",0},     /* 5 */
    {"sung_s1.jpg",0},     /* 6 */
    {"monb_s1.jpg",0},     /* 7 */
    {"tueb_s1.jpg",0},     /* 8 */
    {"wedb_s1.jpg",0},     /* 9 */
    {"thub_s1.jpg",0},     /* 10 */
    {"frib_s1.jpg",0},     /* 11 */
    {"satb_s1.jpg",0},     /* 12 */
    {"sunb_s1.jpg",0},     /* 13 */
   
    {"mong_s2.jpg",0},     /* 0 */
    {"tueg_s2.jpg",0},     /* 1 */
    {"wedg_s2.jpg",0},     /* 2 */
    {"thug_s2.jpg",0},     /* 3 */
    {"frig_s2.jpg",0},     /* 4 */
    {"satg_s2.jpg",0},     /* 5 */
    {"sung_s2.jpg",0},     /* 6 */
    {"monb_s2.jpg",0},     /* 7 */
    {"tueb_s2.jpg",0},     /* 8 */
    {"wedb_s2.jpg",0},     /* 9 */
    {"thub_s2.jpg",0},     /* 10 */
    {"frib_s2.jpg",0},     /* 11 */
    {"satb_s2.jpg",0},     /* 12 */
    {"sunb_s2.jpg",0},     /* 13 */
   
    {"mong_s3.jpg",0},     /* 0 */
    {"tueg_s3.jpg",0},     /* 1 */
    {"wedg_s3.jpg",0},     /* 2 */
    {"thug_s3.jpg",0},     /* 3 */
    {"frig_s3.jpg",0},     /* 4 */
    {"satg_s3.jpg",0},     /* 5 */
    {"sung_s3.jpg",0},     /* 6 */
    {"monb_s3.jpg",0},     /* 7 */
    {"tueb_s3.jpg",0},     /* 8 */
    {"wedb_s3.jpg",0},     /* 9 */
    {"thub_s3.jpg",0},     /* 10 */
    {"frib_s3.jpg",0},     /* 11 */
    {"satb_s3.jpg",0},     /* 12 */
    {"sunb_s3.jpg",0},     /* 13 */
};

UI_NODE_PHOTO Cam_Image[]=
{
    {"cam1s_m1.jpg",0},     /* 0 */
    {"cam2s_m1.jpg",0},     /* 1 */
    {"cam3s_m1.jpg",0},     /* 2 */
    {"cam4s_m1.jpg",0},     /* 3 */
    {"cam1n_m1.jpg",0},     /* 4 */
    {"cam2n_m1.jpg",0},     /* 5 */ 
    {"cam3n_m1.jpg",0},     /* 6 */
    {"cam4n_m1.jpg",0},     /* 7 */

    {"cam1s_m2.jpg",0},     /* 0 */
    {"cam2s_m2.jpg",0},     /* 1 */
    {"cam3s_m2.jpg",0},     /* 2 */
    {"cam4s_m2.jpg",0},     /* 3 */
    {"cam1n_m2.jpg",0},     /* 4 */
    {"cam2n_m2.jpg",0},     /* 5 */ 
    {"cam3n_m2.jpg",0},     /* 6 */
    {"cam4n_m2.jpg",0},     /* 7 */

    {"cam1s_m3.jpg",0},     /* 0 */
    {"cam2s_m3.jpg",0},     /* 1 */
    {"cam3s_m3.jpg",0},     /* 2 */
    {"cam4s_m3.jpg",0},     /* 3 */
    {"cam1n_m3.jpg",0},     /* 4 */
    {"cam2n_m3.jpg",0},     /* 5 */ 
    {"cam3n_m3.jpg",0},     /* 6 */
    {"cam4n_m3.jpg",0},     /* 7 */
};

UI_NODE_PHOTO Set_1_M[] =
{
    {"set1_m1.jpg",0},
    {"set1_m2.jpg",0},
    {"set1_m3.jpg",0},
};
UI_NODE_PHOTO Set_2_M[] =
{
    {"set2_m1.jpg",0},
    {"set2_m2.jpg",0},
    {"set2_m3.jpg",0},
};
UI_NODE_PHOTO Delete_1_M[] =
{
    {"delb_s1.jpg",0},
    {"delb_s2.jpg",0},
    {"delb_s3.jpg",0},
};
UI_NODE_PHOTO Delete_2_M[] =
{
    {"delg_s1.jpg",0},
    {"delg_s2.jpg",0},
    {"delg_s3.jpg",0},
};
UI_NODE_PHOTO Edit_bar            ={"edit_bar.jpg",0};

/* Network Info. */
UI_NODE_PHOTO CamSetBt[] =
{
    {"hd1_m1.jpg",0},
    {"vga1_m1.jpg",0},
    {"hd2_m1.jpg",0},
    {"vga2_m1.jpg",0},
};

UI_NODE_PHOTO ASCII_PHOTO[]=
{
    /*gray*/
    {"num_00.jpg",0},   /*0*/ /* num 0*/
    {"num_01.jpg",0},
    {"num_02.jpg",0},
    {"num_03.jpg",0},
    {"num_04.jpg",0},
    {"num_05.jpg",0},   /*5*/
    {"num_06.jpg",0},
    {"num_07.jpg",0},
    {"num_08.jpg",0},
    {"num_09.jpg",0},           /* num 9*/
    {"char_ba.jpg",0},  /*10*/  /*   A  */
    {"char_bb.jpg",0},
    {"char_bc.jpg",0},
    {"char_bd.jpg",0},
    {"char_be.jpg",0},
    {"char_bf.jpg",0},  /*15*/
    {"char_bg.jpg",0},
    {"char_bh.jpg",0},
    {"char_bi.jpg",0},
    {"char_bj.jpg",0},
    {"char_bk.jpg",0},  /*20*/
    {"char_bl.jpg",0},
    {"char_bm.jpg",0},
    {"char_bn.jpg",0},
    {"char_bo.jpg",0},
    {"char_bp.jpg",0},  /*25*/
    {"char_bq.jpg",0},
    {"char_br.jpg",0},
    {"char_bs.jpg",0},
    {"char_bt.jpg",0},
    {"char_bu.jpg",0},  /*30*/
    {"char_bv.jpg",0},
    {"char_bw.jpg",0},
    {"char_bx.jpg",0},
    {"char_by.jpg",0},
    {"char_bz.jpg",0},  /*35*/ /*   Z  */
    {"asci_b2d.jpg",0},         /* ascii 0x2d - */
    {"asci_b2e.jpg",0},         /* ascii 0x2e . */
    {"char_b2f.jpg",0},         /* ascii 0x2f / */
    {"char_b20.jpg",0},         /* ascii 0x20   */
    {"char_b3a.jpg",0}, /*40*/  /* ascii 0x3A :  */
    {"char_b81.jpg",0},         /*法語*/
    /*green*/
    {"num_10.jpg",0},
    {"num_11.jpg",0},
    {"num_12.jpg",0},
    {"num_13.jpg",0},
    {"num_14.jpg",0},
    {"num_15.jpg",0},
    {"num_16.jpg",0},
    {"num_17.jpg",0},
    {"num_18.jpg",0},
    {"num_19.jpg",0},
    {"char_ga.jpg",0}, /*   A  */
    {"char_gb.jpg",0},
    {"char_gc.jpg",0},
    {"char_gd.jpg",0},
    {"char_ge.jpg",0},
    {"char_gf.jpg",0},
    {"char_gg.jpg",0},
    {"char_gh.jpg",0},
    {"char_gi.jpg",0},
    {"char_gj.jpg",0},
    {"char_gk.jpg",0},
    {"char_gl.jpg",0},
    {"char_gm.jpg",0},
    {"char_gn.jpg",0},
    {"char_go.jpg",0},
    {"char_gp.jpg",0},
    {"char_gq.jpg",0},
    {"char_gr.jpg",0},
    {"char_gs.jpg",0},
    {"char_gt.jpg",0},
    {"char_gu.jpg",0},
    {"char_gv.jpg",0},
    {"char_gw.jpg",0},
    {"char_gx.jpg",0},
    {"char_gy.jpg",0},
    {"char_gz.jpg",0},
    {"asci_g2d.jpg",0},         /* ascii 0x2d - */
    {"asci_g2e.jpg",0},         /* ascii 0x2e . */
    {"char_g2f.jpg",0},         /* ascii 0x2f / */
    {"char_g20.jpg",0},         /* ascii 0x20   */
    {"char_g3a.jpg",0},         /* ascii 0x3A :  */
    {"char_g81.jpg",0},         /*法語*/
};

UI_NODE_PHOTO PBListDayBG[2]       = {{"playdbg1.jpg",0},{"playdbg2.jpg",0}};
UI_NODE_PHOTO PBListDay_half_1  = {"playdhm1.jpg",0};
UI_NODE_PHOTO PBListDay_1       = {"playd_m1.jpg",0};
UI_NODE_PHOTO PBButton       = {"playbutt.jpg",0};

UI_NODE_PHOTO PlaybackList_Cam[8]=
{
    /*green*/
    {"pbklcm01.jpg",0},     /* cam1 */
    {"pbklcm02.jpg",0},     /* cam2 */
    {"pbklcm03.jpg",0},     /* cam3 */
    {"pbklcm04.jpg",0},     /* cam4 */

    /*red*/
    {"pbklcm11.jpg",0},     /* cam1 */
    {"pbklcm12.jpg",0},     /* cam2 */
    {"pbklcm13.jpg",0},     /* cam3 */
    {"pbklcm14.jpg",0},     /* cam4 */
};

UI_NODE_PHOTO PlaybackList_RecType[8]=
{
    /*green*/
    {"recmg_s1.jpg",0},     /* manual */
    {"recsg_s1.jpg",0},     /* sch */
    {"recdg_s1.jpg",0},     /* detect */
    {"recrg_s1.jpg",0},     /* door */

    /*red*/
    {"recmb_s1.jpg",0},     /* manual */
    {"recsb_s1.jpg",0},     /* sch */
    {"recdb_s1.jpg",0},     /* detect */
    {"recrb_s1.jpg",0},     /* door */
};

UI_NODE_PHOTO LoadingLevel[] =
{
    {"load_lv1.jpg",0},
    {"load_lv2.jpg",0},
    {"load_lv3.jpg",0},
    {"load_lv4.jpg",0},
    {"load_lv5.jpg",0},
    {"load_lv6.jpg",0},
};

s8 ipAddrSetting[12]         ={1,9,2,1,6,8,0,0,1,1,1,8};
s8 subMaskSetting[12]        ={2,5,5,2,5,5,2,5,5,0,0,0};
s8 defaultGatewaySetting[12] ={1,9,2,1,6,8,0,0,1,0,0,1};
s8 DNS1Setting[12]           ={0,0,0,0,0,0,0,0,0,0,0,0};
s8 DNS2Setting[12]           ={0,0,0,0,0,0,0,0,0,0,0,0};

s8 cam_info[4][5] = {UI_MENU_SETTING_CAMERA_ON,UI_MENU_REC_MODE_MOTION,UI_MENU_SETTING_RESOLUTION_HD,UI_MENU_SETTING_BRIGHTNESS_LV3,UI_MENU_SETTING_SENSITIVITY_MID,
					 UI_MENU_SETTING_CAMERA_ON,UI_MENU_REC_MODE_MOTION,UI_MENU_SETTING_RESOLUTION_HD,UI_MENU_SETTING_BRIGHTNESS_LV3,UI_MENU_SETTING_SENSITIVITY_MID,
					 UI_MENU_SETTING_CAMERA_ON,UI_MENU_REC_MODE_MOTION,UI_MENU_SETTING_RESOLUTION_HD,UI_MENU_SETTING_BRIGHTNESS_LV3,UI_MENU_SETTING_SENSITIVITY_MID,
					 UI_MENU_SETTING_CAMERA_ON,UI_MENU_REC_MODE_MOTION,UI_MENU_SETTING_RESOLUTION_HD,UI_MENU_SETTING_BRIGHTNESS_LV3,UI_MENU_SETTING_SENSITIVITY_MID}; 

s8  scheduleStartTimeSetting[2];
s8  scheduleEndTimeSetting[2];
u8  CameraSetting[4]={0};
u8  DaysSetting[7]={0};
s8  Sensor_cam[4]={0,1,2,3};
s8  uiCamSet[4]={0,0,0,0};
u8  gTempString[16];
u8  uiSetCfg = 0;
#if UI_LIGHT_SUPPORT
u8  uiSetLightTimer[MULTI_CHANNEL_MAX][4];    /*10:11~12:14*/
u8 LSTimer_APM_Setting[2]={0};
#endif

enum
{
    cam1 =0,
    cam2,
    cam3,    
    cam4,    
};

typedef enum
{
    file_m =0,
    file_s,
    file_d,    
    file_r,    
}FILE_TYPE;

u8 AsciiNum;
u8  TempTime[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern UI_NODE_PHOTO Background[UI_MULT_LANU_END];
extern u8 uiIsRFParing[RFID_MAX_WORD];   // 1:Pairing 0:Not Pairing 
#if (NIC_SUPPORT)
extern UI_NET_INFO UINetInfo; 
#endif
#if(UI_BAT_SUPPORT)
extern u8  uiBatteryInterval[MULTI_CHANNEL_MAX][7][6];
extern u8 _uiCheckBatterySch;
#endif
extern u8  UICheckSchedule;      /*Per day check week,reset sch*/
extern u8 gUiLeaveKaypad;
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern u8 uiGraphDrawJpgGraph(s32 fb_index, u8* pResult, u16* pWidth, u16* pHeight);
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
 
 /*act 0: gray
  act 1: green
  act 3: HomeRF
*/
void uiGraphDrawString(u8* str, u16 x_pos, u16 y_pos, u8 act)
{
    u8 i;
    u8  str_len;
    u8 img_idx;
    u8  err;
    u8 HomeRF_Flag = 0;
    if (act == 3)
    {
        HomeRF_Flag = 1;
        act = 0;
    }
    if (str == NULL)
    {
        DEBUG_UI("uiGraphDrawString string error \n");
        return;
    }
    str_len= strlen((char*)str);
    //DEBUG_UI(" string length %d string %s %d %d\n",str_len ,str,x_pos,y_pos);

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    for(i=0; i<str_len; i++)
    {
        img_idx=str[i];
        //DEBUG_UI("#### img idx = %d xpos: %d  ypos: %d\n", img_idx, x_pos, y_pos);

        if((img_idx > 0x2F) &&(img_idx < 0x3A )) /* num0 ~ num9 */
        {
            img_idx = img_idx-0x30+act*AsciiNum;
        }
        else if((img_idx > 0x40 ) && (img_idx < 0x5B))  /* A~Z */
        {
            img_idx=img_idx-0x41+10+act*AsciiNum;
        }
        else if((img_idx > 0x60 ) && (img_idx < 0x7B))  /* a~z */
        {
            img_idx=img_idx-0x61+10+act*AsciiNum;
        }
        else if((img_idx> 0x2c ) && (img_idx < 0x30))
        {
            img_idx =img_idx-0x2d+36+act*AsciiNum;
        }
        else if(img_idx == 0x20 )
        {
#if HOME_RF_SUPPORT
            if (HomeRF_Flag == 1)
                img_idx = 38;
            else
#endif
            img_idx = 39+act*AsciiNum;
        }
        else if(img_idx == 0x3a )
        {
            img_idx = 40+act*AsciiNum;
        }
        else if((img_idx == 0x5B) && (HomeRF_Flag == 1)) /* [ */
        {
            img_idx = 36;
        }
        else if((img_idx == 0x5D) && (HomeRF_Flag == 1)) /* ] */
        {
            img_idx = 37;
        }
        else
        {
            continue;
        }
#if HOME_RF_SUPPORT
        if (HomeRF_Flag == 1)
            uiGraphDrawJPGImage(HomeRF_ASCII_PHOTO[img_idx], x_pos+i*30, y_pos);    
        else
#endif
        uiGraphDrawJPGImage(ASCII_PHOTO[img_idx], x_pos+i*20, y_pos);
    }

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphGetMenuData(void)
{
    u32 i;
    UI_NODE_DATA *draw_data;
    static UI_NODE_DATA  last_data;
    u16 uiJpgWidth, uiJpgHeight;
    u8  err;
    u8	rteVal = 0, Cnt = 3;
    
#if ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||\
    (FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    draw_data = uiCurrNode->item.NodeData;
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    for (i = 0; i < draw_data->FileNum; i++)
    {
    #if 0
        if((MyHandler.WhichKey == UI_KEY_UP) || (MyHandler.WhichKey == UI_KEY_DOWN)||(MyHandler.WhichKey == UI_KEY_LEFT)||(MyHandler.WhichKey == UI_KEY_RIGHT))
        {
            if(draw_data->FileData[i].FileInfo[CurrLanguage].bufIndex == last_data.FileData[i].FileInfo[CurrLanguage].bufIndex)
                continue;
        }
    #endif
    
		do
		{
			rteVal = uiGraphDrawJpgGraph(draw_data->FileData[i].FileInfo[CurrLanguage].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight);
			if(rteVal != 1)
				DEBUG_UI("File %s Open Fail\r\n",draw_data->FileData[i].FileInfo[CurrLanguage].FileName);
			
			Cnt--;
			if (Cnt==0)
			{
				if(rteVal==0)
				{
					DEBUG_UI("File %s Open Fail\r\n",draw_data->FileData[i].FileInfo[CurrLanguage].FileName);
					return;
				}
				else
					break;
			}
		}while (rteVal != 1);

        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight,
                draw_data->FileData[i].Location_x,
                draw_data->FileData[i].Location_y,UI_MENU_SIZE_X,PKBuf0);
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    memcpy(&last_data, draw_data, sizeof(UI_NODE_DATA));
#endif
}

void uiGraphDrawTimeGraph(s8 setCursor, RTC_DATE_TIME* drawTime, u16 x_pos, u16 y_pos,u8 act)
{
    u8  numIndex[2], i, Num;
    u16 year_x = 550;
    u16 year_y = 192;
    u8  err;
    
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    //DEBUG_UI("Enter DrawDateTime setCursor %d %d\r\n",setCursor,act);    
    switch((setCursor%6))
    {
        case 0: /*year*/
            Num = drawTime->year;
            break;

        case 1: /*month*/
            Num = drawTime->month;
            break;

        case 2: /*day*/
            Num = drawTime->day;
            break;

        case 3: /*hour*/
            Num = drawTime->hour;
            break;

        case 4: /*min*/
            Num = drawTime->min;
            break;

        case 5: /*sec*/
            Num = drawTime->sec;
            break;


    }
    numIndex[0] = (Num/10);
    numIndex[1] = (Num%10);

    if (((setCursor%6)==0) && (act == 1))
    {
        uiGraphDrawJPGImage(TimeNum[2+act*10],  year_x,     year_y);//year
        uiGraphDrawJPGImage(TimeNum[0+act*10],  year_x+42,  year_y);
    }
    else 
    {
        uiGraphDrawJPGImage(TimeNum[2],  year_x,     year_y);//year
        uiGraphDrawJPGImage(TimeNum[0],  year_x+42,  year_y);
    }
    
    for (i = 0; i < 2; i++)
    {
        uiGraphDrawJPGImage(TimeNum[numIndex[i]+act*10],  x_pos+i*42,  y_pos);
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

}

void uiGraphDrawSelectDateTime(RTC_DATE_TIME* date, u8 index, u8 opt)
{
    u8 leap_year;

    switch (index)
    {
        case 0: /*year*/
            if (opt == 1)   /*+*/
            {
                if (date->year >= 63)
                    date->year = 0;
                else
                    date->year++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->year == 0)
                    date->year = 63;
                else
                    date->year--;
            }
            leap_year = date->year%4;
            if (date->month == 2 && leap_year != 0 && date->day == 29)
                date->day = 28;
            break;

        case 1: /*month*/
            if (opt == 1)   /*+*/
            {
                if (date->month >= 12)
                    date->month = 1;
                else
                    date->month++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->month <= 1)
                    date->month = 12;
                else
                    date->month--;
            }
            switch(date->month)
            {
                case 2:
                    leap_year = date->year%4;
                    if (leap_year == 0 && date->day > 29)
                        date->day = 29;
                    else if (leap_year != 0 && date->day > 28)
                        date->day = 28;
                    break;

                case 4:
                case 6:
                case 9:
                case 11:
                    if (date->day > 30)
                        date->day = 30;
                    break;
                default:
                    break;

            }
            break;

        case 2: /*day*/
            if (opt == 1)   /*+*/
                date->day++;
            else if (opt == 2)  /*-*/
                date->day--;

            switch(date->month)
            {
                case 2:
                    leap_year = date->year%4;
                    if (leap_year == 0)
                    {
                        if (date->day > 29)
                            date->day = 1;
                        else if (date->day == 0)
                            date->day = 29;
                    }
                    else
                    {
                        if (date->day > 28)
                            date->day = 1;
                        else if (date->day == 0)
                            date->day = 28;
                    }
                    break;

                case 4:
                case 6:
                case 9:
                case 11:
                    if (date->day > 30)
                        date->day = 1;
                    else if (date->day == 0)
                        date->day = 30;
                    break;

                default:
                    if (date->day >31)
                        date->day = 1;
                    else if (date->day == 0)
                        date->day = 31;
                    break;
            }
            break;

        case 3: /*hour*/
            if (opt == 1)   /*+*/
            {
                if (date->hour >= 23)
                    date->hour = 0;
                else
                    date->hour++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->hour == 0)
                    date->hour = 23;
                else
                    date->hour--;
            }
            break;

        case 4: /*min*/
            if (opt == 1)   /*+*/
            {
                if (date->min >= 59)
                    date->min = 0;
                else
                    date->min++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->min == 0)
                    date->min = 59;
                else
                    date->min--;
            }
            break;

        case 5: /*sec*/
            if (opt == 1)   /*+*/
            {
                if (date->sec >= 59)
                    date->sec = 0;
                else
                    date->sec++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->sec == 0)
                    date->sec = 59;
                else
                    date->sec--;
            }
            break;


        default:
            DEBUG_UI("uiGraphDrawSelectDateTime error index %d\r\n",index);
            return;
    }
}

#if SET_NTPTIME_TO_RTC
void uiGraphDrawSyncNTP(u8 value)
{
    u16 x_pos = 143;
    u16 y_pos = 333;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    uiGraphDrawJPGImage(NTP_Image[value+(CurrLanguage*2)], x_pos, y_pos);

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

}

void uiGraphDrawTimeZoneBtn(void)
{
    u16 x_pos = 143;
    u16 y_pos = 218;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    uiGraphDrawJPGImage(TimeZoneBtn[CurrLanguage], x_pos, y_pos);

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawTimeZoneGraph(s8 setCursor, RTC_TIME_ZONE* drawTime, u16 x_pos, u16 y_pos,u8 act)
{
    u8  numIndex[2], i, Num;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    DEBUG_UI("Enter DrawDateTime setCursor %d %d\r\n",setCursor,act);    
    switch((setCursor%3))
    {
        case 0: /*operator*/
            Num = drawTime->operator;
            break;

        case 1: /*hour*/
            Num = drawTime->hour;
            break;

        case 2: /*min*/
            Num = drawTime->min;
            break;
    }
    numIndex[0] = (Num/10);
    numIndex[1] = (Num%10);

    if ((setCursor%3)==0)
        uiGraphDrawJPGImage(NumOp[Num+act*2], x_pos, y_pos);
    else
    {
        for (i = 0; i < 2; i++)
        {
            uiGraphDrawJPGImage(TimeNum[numIndex[i]+act*10],  x_pos+i*42,  y_pos);
        }
    }

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

}

void uiGraphDrawSelectTimeZone(RTC_TIME_ZONE* date, u8 index, u8 opt)
{
    u8 leap_year;

    switch (index)
    {
        case 0: /*operator*/
            date->operator ^= 1;
            break;

        case 1: /*hour*/
            if (opt == 1)   /*+*/
            {
                if (date->hour >= 12)
                    date->hour = 0;
                else
                    date->hour++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->hour == 0)
                    date->hour = 12;
                else
                    date->hour--;
            }
            break;

        case 2: /*min*/
            if (opt == 1)   /*+*/
            {
                if (date->min == 30)
                    date->min = 0;
                else
                    date->min += 30;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->min == 0)
                    date->min = 30;
                else
                    date->min -= 30;
            }
            break;
            
        default:
            DEBUG_UI("uiGraphDrawSelectDateTime error index %d\r\n",index);
            return;
    }
}

void uiGraphDrawTimezone(u8 key)
{
    static s8 setCursor;
    static u8 startSelect=0;
    static u8 ntp=0;
    u8  i;
    u16 x_pos[3] = {392, 465, 581};
    u16 y_pos = 267;
    static RTC_TIME_ZONE temp;

    DEBUG_UI("Enter uiGraphDrawTimezone key %d %d\r\n",key,setCursor);
    
    switch(key)
    {
        case 0:
            setCursor = 0;
            startSelect = 0;
            memcpy(&temp, &SetZone, sizeof(RTC_TIME_ZONE));
            for(i=0;i<3;i++)
            {
                uiGraphDrawTimeZoneGraph(i,&temp,x_pos[i], y_pos,0);
            }
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            startSelect = 1;
            uiGraphDrawTimeZoneGraph(setCursor,&temp,x_pos[setCursor], y_pos,0); 
            //uiOsdDrawTimeFrame(setCursor,1);
            if (TouchExtKey != -1)
            {
                setCursor = TouchExtKey;
            }
            else
            {
                if(key== UI_KEY_RIGHT)
                {
                    setCursor++;     
                    if(setCursor > 2)
                        setCursor = 0;
                }
                else
                {
                    setCursor--;
                    if(setCursor < 0)
                        setCursor = 5;
                }
            }
            uiGraphDrawTimeZoneGraph(setCursor,&temp,x_pos[setCursor], y_pos,1); 
            //uiOsdDrawTimeFrame(setCursor,0);
            break;

        case UI_KEY_UP:
            if (startSelect < 1) break;
            uiGraphDrawSelectTimeZone(&temp, setCursor, 1);
            uiGraphDrawTimeZoneGraph(setCursor, &temp, x_pos[setCursor], y_pos, 1);
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;

        case UI_KEY_DOWN:
            if (startSelect < 1) break;
            uiGraphDrawSelectTimeZone(&temp, setCursor, 2);
            uiGraphDrawTimeZoneGraph(setCursor, &temp, x_pos[setCursor], y_pos, 1);
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;

        case UI_KEY_ENTER:
            memcpy(&SetZone, &temp, sizeof(RTC_TIME_ZONE));
            Save_UI_Setting();            
            uiMenuAction(uiCurrNode->parent->item.NodeData->Action_no);
            //MyHandler.MenuMode = SETUP_MODE;
            uiFrowGoToLastNode();
            uiGraphDrawMenu();
            break;

        case UI_KEY_MENU:   /*leave*/
            //uiOsdDrawSetting(0);
            //MyHandler.MenuMode = SETUP_MODE;
            uiFrowGoToLastNode();
            uiGraphDrawMenu();   
            break;

        default:
            DEBUG_UI("uiGraphDrawTimezone error key %d\r\n",key);
            break;
    }
}
#endif

void uiGraphDrawDateTime(u8 key)
{
    static s8 setCursor;
    static u8 startSelect=0;
    u8  i;
    u16 x_pos[6] ={634,434,318,347,465,581};
    u16 y_pos[2] ={192,336};
    #if SET_NTPTIME_TO_RTC
    static u8 ntp=0;
    #endif
    
    //DEBUG_UI("Enter DrawDateTime key %d \r\n",key);
    
    switch(key)
    {
        case 0:
            setCursor = 0;
            startSelect = 0;
            /*get current time*/
            RTC_Get_Time(&SetTime);
            for(i=0;i<6;i++)
            {
                if(i<3)
                    uiGraphDrawTimeGraph(i,&SetTime,x_pos[i], y_pos[0],0);
                else
                    uiGraphDrawTimeGraph(i,&SetTime,x_pos[i], y_pos[1],0);    
            }
            #if SET_NTPTIME_TO_RTC
            ntp=iconflag[UI_MENU_SETIDX_DATE_TIME];
            uiGraphDrawSyncNTP(ntp);
            uiGraphDrawTimeZoneBtn();
            #endif
            //uiOsdDrawSetting(1);
            //uiOsdDrawTimeFrame(0,0);
            
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            startSelect = 1;
            uiGraphDrawTimeGraph(setCursor,&SetTime,x_pos[setCursor], y_pos[setCursor/3],0); 
            //uiOsdDrawTimeFrame(setCursor,1);
            if (TouchExtKey != -1)
            {
                setCursor = TouchExtKey;
            }
            else
            {
                if(key== UI_KEY_RIGHT)
                {
                    setCursor++;     
                    if(setCursor >5)
                        setCursor=0;
                }
                else
                {
                    setCursor--;
                    if(setCursor <0)
                        setCursor=5;
                }
            }
            uiGraphDrawTimeGraph(setCursor,&SetTime,x_pos[setCursor], y_pos[setCursor/3],1); 
            //uiOsdDrawTimeFrame(setCursor,0);
            break;

        case UI_KEY_UP:
            if (startSelect < 1) break;
            uiGraphDrawSelectDateTime(&SetTime, setCursor, 1);
            uiGraphDrawTimeGraph(setCursor,&SetTime,x_pos[setCursor], y_pos[setCursor/3],1);
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;

        case UI_KEY_DOWN:
            if (startSelect < 1) break;
            uiGraphDrawSelectDateTime(&SetTime, setCursor, 2);
            uiGraphDrawTimeGraph(setCursor,&SetTime,x_pos[setCursor], y_pos[setCursor/3],1); 
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;
#if SET_NTPTIME_TO_RTC
        case UI_KEY_MODE:
            ntp ^= 1;
            uiGraphDrawSyncNTP(ntp);
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;

        case UI_KEY_MAIN:
            uiEnterMenu(UI_MENU_NODE_TIME_ZONE);
            uiGraphDrawMenu();
            break;
#endif
            
        case UI_KEY_ENTER:
            //uiOsdDrawSetting(0);
            uiCurrNode = uiCurrNode->parent;
            iconflag[UI_MENU_SETIDX_DATE_TIME] = ntp;
            uiMenuAction(uiCurrNode->item.NodeData->Action_no);
            MyHandler.MenuMode = SETUP_MODE;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;

        case UI_KEY_MENU:   /*leave*/
            ntp=0;
            uiOsdDrawSetting(0);
            MyHandler.MenuMode = SETUP_MODE;
            uiFrowGoToLastNode();
            uiGraphDrawMenu();   
            break;

        default:
            DEBUG_UI("osdDrawDateTime error key %d\r\n",key);
            break;
    }
}

void uiGraphDrawSelectGraph(u16 x_pos,u16 y_pos,u8 act)
{
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    DEBUG_UI(" Draw Select Graph\n");

    uiGraphDrawJPGImage(YesNo_Image[act],x_pos,y_pos);
    
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawLanguage(u8 key)
{
    static s8 value;	
    u16 y_pos[3]= {140,279,417};
    u16 x_pos = 311;
    u8 i;
    
    DEBUG_UI("uiGraphDrawLanguage Key %d\n",key);

    switch(key)
    {
        case 0:
            value = iconflag[UI_MENU_SETIDX_LANGUAGE];
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);
            break;
            
        case UI_KEY_UP:
            value = TouchExtKey;
            for (i=0;i<3;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;            
                                                              
        case UI_KEY_ENTER:
            iconflag[UI_MENU_SETIDX_LANGUAGE] = value;
            uiMenuSet_Language(iconflag[UI_MENU_SETIDX_LANGUAGE]);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawFomat error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawDefault(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[2]= {105,205};
    u16 x_pos = 210;

    enum
    {
        yes = 0,
        no,
    };
    
    DEBUG_UI("uiGraphDrawDefault Key %d\n",key);

    switch(key)
    {
        case 0:
            uiGraphDrawSelectGraph(x_pos,y_pos[no],1);//draw
            break;
            
        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if (key == UI_KEY_UP)
                value = yes;
            else
                value = no;
            for (i=0;i<2;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;            
                                                              
        case UI_KEY_ENTER:
            uiMenuSet_Default(value);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawDefault error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawMonitorAlarm(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[2]= {105,205};
    u16 x_pos;

    enum
    {
        yes = 0,
        no,
    };
    
    DEBUG_UI("uiGraphDrawMonitorAlarm Key %d\n",key);

    switch(key)
    {
        case 0:
            uiGraphDrawSelectGraph(x_pos,y_pos[no],1);//draw
            break;
            
        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if (key == UI_KEY_UP)
                value = yes;
            else
                value = no;
            for (i=0;i<2;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;            
                                                              
        case UI_KEY_ENTER:
            iconflag[UI_MENU_SETIDX_ALARM] = value;
            uiMenuSet_Alarm(iconflag[UI_MENU_SETIDX_ALARM]);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawMonitorAlarm error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawUpgrade(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[2]= {105,205};
    u16 x_pos = 210;

    enum
    {
        yes = 0,
        no,
    };
    
    DEBUG_UI("uiGraphDrawUpgrade Key %d\n",key);

    switch(key)
    {
        case 0:
            uiGraphDrawSelectGraph(x_pos,y_pos[no],1);//draw
            break;
            
        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if (key == UI_KEY_UP)
                value = yes;
            else
                value = no;
            for (i=0;i<2;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;            
                                                              
        case UI_KEY_ENTER:
            
            if (uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_UPGRADE_FW)
            {
                DEBUG_UI("Upgrade %d\r\n", value);
                osdDrawUpgradeFW();

            }
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawDefault error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawRECModeGraph(s8 setCursor)
{
    u8 i,value;
    u16 y_pos[4]= {138,242,346,455};
    u16 x_pos[3]={470,686,875};
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    value = cam_info[setCursor][1];
    
    DEBUG_UI(" Draw Rec Mode Graph %d\n",setCursor);

    for(i=0;i<3;i++)
    {
        if (i==1)
            continue;
        
        if (i==value)
            uiGraphDrawJPGImage(Select_Image ,x_pos[i],y_pos[setCursor]);   
        else
            uiGraphDrawJPGImage(Select_Item[setCursor*3+i],x_pos[i],y_pos[setCursor]);//draw empty
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawRECMode(u8 key)
{
    u8 i;
    
    //DEBUG_UI("uiGraphDrawRECMode Key %d\n",key);

    switch(key)
    {
        case 0:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                switch(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i])
                {
                    case UI_MENU_REC_MODE_MOTION:
                        cam_info[i][1] = 0;            
                        break;

                    case UI_MENU_REC_MODE_MANUAL:
                        cam_info[i][1] = 0;            
                        break;
                        
                    case UI_MENU_REC_MODE_SCHEDULE:
                        cam_info[i][1] = 2;            
                        break;

                    default:
                        cam_info[i][1] = 0;            
                        break;
                } 
                uiGraphDrawRECModeGraph(i);  
            }
            break;

        case UI_KEY_RIGHT:
            cam_info[cam4][1] = TouchExtKey;  
            uiGraphDrawRECModeGraph(cam4);  
            break;

        case UI_KEY_LEFT:
            cam_info[cam3][1] = TouchExtKey;  
            uiGraphDrawRECModeGraph(cam3);  
            break;

        case UI_KEY_UP:
            cam_info[cam1][1] = TouchExtKey;  
            uiGraphDrawRECModeGraph(cam1);  
            break;

        case UI_KEY_DOWN:
            cam_info[cam2][1] = TouchExtKey;  
            uiGraphDrawRECModeGraph(cam2);  
            break;
            
        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                switch(cam_info[i][1])
                {
                    case 0:
                        cam_info[i][1] = UI_MENU_REC_MODE_MOTION;            
                        break;

                    case 2:
                        cam_info[i][1] = UI_MENU_REC_MODE_SCHEDULE;            
                        break;

                    default:
                        cam_info[i][1] = UI_MENU_REC_MODE_MOTION;   
                        break;
                }
                
                if(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i] != cam_info[i][1])
                {
                    iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i]=cam_info[i][1]; 
                    uiMenuAction(UI_MENU_SETIDX_REC_MODE_CH1+i);
                }
            }
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();            
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();
            uiGraphDrawMenu();            
            break;
            
        default:
            DEBUG_UI("uiGraphDrawRECMode error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawStaticIPBackground(u8 itemCursor)
{
    u16 x_pos=95;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    uiGraphDrawJPGImage(Txt_Address_2_M[CurrLanguage],x_pos,202);
    uiGraphDrawJPGImage(Txt_Mask_2_M[CurrLanguage],x_pos,275);
    uiGraphDrawJPGImage(Txt_Gateway_2_M[CurrLanguage],x_pos, 340);
    uiGraphDrawJPGImage(BTN_OK_2_M[CurrLanguage],235,420);
    
    switch(itemCursor)
    {
        case 0:
            uiGraphDrawJPGImage(Txt_Address_1_M[CurrLanguage],x_pos,202);
            break;
        case 1:
            uiGraphDrawJPGImage(Txt_Mask_1_M[CurrLanguage],x_pos,275);
            break;
        case 2:
            uiGraphDrawJPGImage(Txt_Gateway_1_M[CurrLanguage],x_pos,340);
            break;
        case 3:
            uiGraphDrawJPGImage(BTN_OK_1_M[CurrLanguage],235,420);
            break;
            
        default:
            DEBUG_UI("Draw Static IP BG error \n");              
            break;
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    
}



#if(NIC_SUPPORT)
void ChangeStrtoIP(u8 select)
{
    u8  i;
    u32 ip[4];
    u8  *NewIP;
    u8 nLen;
    u8 sTempString[16];
    u8 nTemp;
    
/*    strcpy(gTempString,"1.22.8.4");
    sprintf(gTempString,"%s%d",gTempString,1);
    sprintf(gTempString,"%s%s",gTempString,".");
    DEBUG_UI("Set IP Address  :%s \n",gTempString);
    nLen=strlen(gTempString);
    gTempString[nLen-1]='\0';
    DEBUG_UI("Set IP Address  :%s \n",gTempString);
    return;*/
    DEBUG_GREEN("=============== gTempString = %s \n",gTempString);
    nLen = strlen(gTempString);
    if((nLen<7) || (nLen>15))
    {
        strcpy(gTempString,"");
        return;
    }

    sscanf((char*)gTempString, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
    sprintf((char*)sTempString, "%03u.%03u.%03u.%03u", ip[0], ip[1], ip[2], ip[3]);
    switch(select)
    {
        case 0:
            NewIP=ipAddrSetting;           
        break;
        case 1:
            NewIP=subMaskSetting;           
        break;
        case 2:
            NewIP=defaultGatewaySetting;            
        break;
    }
    
    for (i = 0; i < 4; i++)
    {
        if ((ip[i] > 255) || (ip[i] < 0))
        {
            DEBUG_UI("IP Address %d Error value %u!!!\n", i, ip[i]);
            strcpy(gTempString,"");
            return;
        }
    }
    for (i = 0; i < 4; i++)
    {
        NewIP[i*3+2] = ip[i] % 10;
        nTemp = ip[i] / 10;
        NewIP[i*3+1] = nTemp % 10;
        NewIP[i*3]=ip[i] / 100;
    }
    strcpy(gTempString,"");
    DEBUG_UI("Set IP Address  :%u.%u.%u.%u \n",ip[0], ip[1], ip[2], ip[3]);
    DEBUG_UI("Set IP Address  :%u.%u.%u.%u \n",ipAddrSetting[0], ipAddrSetting[1], ipAddrSetting[2], ipAddrSetting[3]);
}
#if 0
void uiGraphDrawNetwork(u8 key)
{
    static s8 setCursor=0 , itemCursor=0,setLevel=0;    /*setCursor = 0: DYNAMIC_IP, setCursor=1: STATIC_IP*/
    u8 i;
    
    sysSPI_Enable();
    sysJPEG_enable();

    DEBUG_UI("===> uiGraphDrawNetwork\n");
    if(setLevel == 1)
    {
#if 0
        retval=uiGraphDrawStaticIP(key,itemCursor);

        if(((key== UI_KEY_MENU) && (retval== 1)) ||(key == UI_KEY_ENTER))
        {
            setLevel=0;
        }
#endif
        uiGraphDrawJPGImage(Static_BG[CurrLanguage],0,0);
        uiGraphDrawStaticIPBackground(itemCursor);
        //uiGraphReadStaticIP();
        ChangeStrtoIP(itemCursor);
        uiGraphDrawStaticIP(0,itemCursor);
        
        setCursor=UI_MENU_STATIC_IP;
        //itemCursor=0;
        setLevel=0;
        return ;
    }

    switch(key)
    {
        case 0:
            uiGraphReadStaticIP();
            uiGraphDrawStaticIP(0,0);
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            if (TouchExtKey != -1)
            {
                if(TouchExtKey == setCursor)
                    return;
                setCursor = TouchExtKey;
            }
            else
            {
                if(key == UI_KEY_RIGHT)
                {
                    setCursor++;
                    if(setCursor > 1)
                        setCursor=0;
                }                           
                else
                {
                    setCursor--;
                    if(setCursor < 0)
                        setCursor=1;
                }
            }
                
            uiGraphDrawStaticIP(0,0);
                        
            break;
            
        case UI_KEY_DOWN:
        case UI_KEY_UP:
            if(setCursor == UI_MENU_STATIC_IP)
            {
                if(key == UI_KEY_UP)
                {
                    itemCursor--;
                    if(itemCursor < 0)
                        itemCursor=3;
                }
                else
                {
                    itemCursor++;
                    if(itemCursor > 3)
                        itemCursor=0;
                }
                uiGraphDrawStaticIPBackground(itemCursor);
            }
            
            break;
            
        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
            {
                if(setCursor==UI_MENU_STATIC_IP)
                {
                    if(TouchExtKey<4)
                        itemCursor = TouchExtKey;
                    else
                        return;
                }
                else
                {
                    if(TouchExtKey!=4)
                        return;
                }
            }
            
            if(itemCursor == 3)  // press OK Button
            {
                UINetInfo.IsStaticIP=UI_MENU_STATIC_IP;
                for(i=0 ;i<3; i++)
                {
                    uiGraphSaveStaticIP(i);
                }
                Save_UI_Setting();
                sysSPI_Enable();
                sysJPEG_enable();
                osdDrawSystemReboot();
                sysForceWDTtoReboot();
                
                uiCurrNode = uiCurrNode->parent;
                setLevel=0;
                uiGraphDrawMenu();    
            }
            else
            {
                setLevel=1;
//                    uiGraphDrawStaticIP(0,itemCursor);    
//                    uiGraphDrawStaticIPNum(itemCursor,0,1);
                //uiEnterMenu(UI_MENU_NODE_SET_NETWORK_KEYPAD);
                uiGraphDrawMenu();
            }
                
            iconflag[UI_MENU_SETIDX_NETWORK_STATUS]=UINetInfo.IsStaticIP;;
                
            
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();
            uiGraphDrawMenu();
            break;           
        
        default:
            DEBUG_UI("uiGraphDrawNetwork error key %d\r\n",key);
            break;
    }
    
    sysJPEG_disable();
    sysSPI_Disable();
    
}
#endif

void uiGraphDrawSelectStaticIP(u8 itemCursor, u8 setCursor,u8 opt)
{
    
    switch (itemCursor)
    {
        case 0:
            if(opt == 0) /* "-" */
            {
                ipAddrSetting[setCursor]--;

                if(ipAddrSetting [setCursor] <0)
                    ipAddrSetting [setCursor] = 9; 
            }
            else        /* "+" */
            {
                ipAddrSetting[setCursor]++;    
                if(ipAddrSetting [setCursor] >9)
                    ipAddrSetting [setCursor] = 0; 
            }       
            
            break;
            
        case 1:
            if(opt == 0) /* "-" */
            {
                subMaskSetting[setCursor]--;

                if(subMaskSetting [setCursor] <0)
                    subMaskSetting [setCursor] = 9; 
            }
            else        /* "+" */
            {
                subMaskSetting[setCursor]++;    
                if(subMaskSetting [setCursor] >9)
                    subMaskSetting [setCursor] = 0; 
            }
            break;
            
        case 2:
            if(opt == 0) /* "-" */
            {
                defaultGatewaySetting[setCursor]--;

                if(defaultGatewaySetting [setCursor] <0)
                    defaultGatewaySetting [setCursor] = 9; 
            }
            else        /* "+" */
            {
                defaultGatewaySetting[setCursor]++;    
                if(defaultGatewaySetting [setCursor] >9)
                    defaultGatewaySetting [setCursor] = 0; 
            }

            break;
            
            
        default:
            DEBUG_UI("uiGraphDrawSelectStaticIP Error \n"); 
            break;
    }
        
}

void uiGraphDrawStaticIPNum(u8 itemCursor, u8 setCursor ,u8 act)
{
    u8 ip_val;
    u8 num[1];
    u16 x_pos[12]={432,452,472,
                   504,524,544,
                   580,600,620,
                   652,672,692};
    u16 y_pos[3]={196,288,380};

    switch (itemCursor)
    {
        case 0:
            ip_val=ipAddrSetting[setCursor];
            break;
            
        case 1:
            ip_val=subMaskSetting[setCursor];
            break;
            
        case 2:
            ip_val=defaultGatewaySetting[setCursor];
            break;
    }
    sprintf((char *)num,"%d",ip_val);
    uiGraphDrawString(num, x_pos[setCursor],y_pos[itemCursor], act);

}

void uiGraphReadStaticIP(void)
{
    u8 i,j,num,value;
    u8 *ip_set;
    u8 *ip;
    
    for(i=0;i<3;i++)
    {
        switch(i)
        {
            case 0:
                ip_set=ipAddrSetting;
                ip = UINetInfo.IPaddr;
                break;
            case 1:
                ip_set=subMaskSetting;
                ip = UINetInfo.Netmask;
                break;
            case 2:
                ip_set=defaultGatewaySetting;
                ip = UINetInfo.Gateway;
                break;           
        }

        for(j=0;j<4;j++)
        {
            value=ip[j];
            num=0;
            
            num=value/100;
            ip_set[j*3]=num;
            value=value-num*100;

            num=value/10;
            ip_set[j*3+1]=num;
            value=value-num*10;

            num=value;
            ip_set[j*3+2]=num;
            
        }
                
    }
}

void  uiGraphSaveStaticIP(u8 select)
{
    u16 num;
    u8 j,idx=0;    
    u8 ip_s[12]={0};
    u8 ip_u[4]={0};

    switch(select)
    {
        case 0:
            memcpy((void *)ip_s,(void *)ipAddrSetting,12);
            break;
        case 1:
            memcpy((void *)ip_s,(void *)subMaskSetting,12);
            break;
        case 2:
            memcpy((void *)ip_s,(void *)defaultGatewaySetting,12);
            break;  
    }
    
    for(j=0;j<4;j++)
    {
        num=0;          
        num+=(ip_s[idx]*100);
        idx++;
        num+=(ip_s[idx]*10);
        idx++;
        num+=ip_s[idx];        
        idx++;
        ip_u[j]=num;        
            
    }

     
    switch(select)
    {
        case 0:
            memcpy((void *)UINetInfo.IPaddr,(void *)ip_u,4);
            break;
            
        case 1:
            memcpy((void *)UINetInfo.Netmask,(void *)ip_u,4);
            break;
            
        case 2:
            memcpy((void *)UINetInfo.Gateway,(void *)ip_u,4);
            break;            
    }            
}
    

u8 uiGraphDrawStaticIP(u8 key)
{
    static s8 setCursor = -1;    
    u8 i=0;
    u8 j=0;
    
    DEBUG_GREEN("uiGraphDrawStaticIP setCursor %d %d\n",setCursor,gUiLeaveKaypad);
    switch(key)
    {
        case 0:
            if ((setCursor < 0) || (gUiLeaveKaypad == 0))
            {
                setCursor = -1;
                GetNetworkInfo(&UINetInfo);
                uiGraphReadStaticIP();
                for(j=0;j<3;j++)
                {
                    for(i=0;i<12;i++) 
                    {
                        uiGraphDrawStaticIPNum(j,i,0);   
                    }
                }
            
            }
            else
            {
                ChangeStrtoIP(setCursor);
                for(j=0;j<3;j++)
                {
                    for(i=0;i<12;i++) 
                    {
                        if (j==setCursor)
                            uiGraphDrawStaticIPNum(j,i,1); 
                        else
                            uiGraphDrawStaticIPNum(j,i,0); 
                    }
                }
                gUiLeaveKaypad = 0;
            }
            break;
            
        case UI_KEY_ENTER:
            if (TouchExtKey < 0)
            {
                UINetInfo.IsStaticIP=UI_MENU_STATIC_IP;
                for(i=0 ;i<3; i++)
                {
                    uiGraphSaveStaticIP(i);
                }
                uiMenuAction(UI_MENU_SETIDX_ST_IP_SET);
                iconflag[UI_MENU_SETIDX_NETWORK_STATUS] = UI_MENU_STATIC_IP;
                Save_UI_Setting();
                sysSPI_Enable();
                sysJPEG_enable();
                osdDrawSystemReboot();
                sysForceWDTtoReboot();
                
                uiCurrNode = uiCurrNode->parent;
                uiGraphDrawMenu();    
            }
            else
            {
                setCursor = TouchExtKey;
                uiEnterMenu(UI_MENU_NODE_KEYPAD);
                uiGraphDrawMenu();
                gUiLeaveKaypad = 1;
            }

            break;    
            
        case UI_KEY_MENU:
            setCursor = -1;
            uiFrowGoToLastNode();
            uiGraphDrawMenu();
            gUiLeaveKaypad = 0;
            break;    
            
        default:
            DEBUG_UI("uiGraphDrawStaticIP error key %d\r\n",key);
            return 0;
    }

    return 1;
}

/* onOff=0 -> static IP
   onOff=1 -> dynamic IP
*/
u8 uiGraphDrawDynamicIP(u8 onOff)
{
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    switch(onOff)
    {
        case 0:
            uiGraphDrawJPGImage(Dynmic_OFF[CurrLanguage],0,0);
            break;
            
        case 1:
            uiGraphDrawJPGImage(Dynmic_ON[CurrLanguage],0,0);
            break;
          
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    return 1;
}

#endif
#if (REMOTE_FILE_PLAYBACK)
void uiGraphDrawPlaybackbusy(void)
{
    u8  err;
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

   uiGraphDrawJPGImage(busy_ON[CurrLanguage],220,240);
   DEBUG_UI("---> uiGraphDrawJPGImage \n");
    
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}
#endif
/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */
void uiGraphDrawLoadingGraph(u8 level)
{
    u8 err;
    u16 x_pos = 472;
    u16 y_pos = 260;

    
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    uiGraphDrawJPGImage(LoadingLevel[level],x_pos,y_pos);

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

#if UI_CALENDAR_SUPPORT
/*Act = 0, gray
  Act = 1, green
*/
void uiGraphDrawPlaybackListYM(u8 year, u8 mon, u8 Act)
{
    u16 y_pos = 98, YearX, MonthX;
    u8 tmpStr[5];

    YearX=316;
    MonthX = 651;
        
    /*Draw Year 20XX*/
    sprintf((char*)tmpStr, "20%02d", year);
    uiGraphDrawString(tmpStr, YearX, y_pos, Act);
    
    /*Month*/
    sprintf((char*)tmpStr, "%02d", mon);
    uiGraphDrawString(tmpStr, MonthX, y_pos, Act);

}

void uiGraphDrawPlaybackListCam(u8 CamId, u8 Act)
{
    u16 y_pos = 162, x_pos = 28;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    uiGraphDrawJPGImage(PlaybackList_Cam[CamId+Act*4], x_pos, y_pos+102*CamId);

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawPlaybackListRecType(FILE_TYPE type,u8 Act)
{
    u16 y_pos[4] = {165,264,367,467}, x_pos = 926;
    u8  err;

    #if (DCF_RECORD_TYPE_API == 0)
        return;
    #endif
    
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    uiGraphDrawJPGImage(PlaybackList_RecType[type+Act*4], x_pos, y_pos[type]);

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

u8 uiGraphDrawPlaybackListCalendar(u8 year, u8 mon)
{
    u8  StartWeek, weekCnt = 0, i, monday, week;
    RTC_DATE_TIME   CalTime;
    u16 week_x[7] = {172, 276, 392, 504, 628, 736, 856};
    u16 week_y[6] = {184, 258, 328, 404, 480, 550};
    u16 grid_x[7] = {130, 246, 360, 474, 588, 702, 818};
    u16 grid_y[6] = {186, 262, 336, 408, 484, 554};
    u16 day_x, day_y, dayBg_x, dayBg_y;
    u8  err;
    u8  tmpStr[3];
    UI_NODE_PHOTO   dayBgIdx;
    u8 days=0;
        
    CalTime.year = year;
    CalTime.month = mon;
    CalTime.day = 1;
    StartWeek = RTC_Get_Week(&CalTime);
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    monday=rtcGetDayNum(year, mon);
    week = StartWeek;
    //DEBUG_YELLOW("%d %s %s StartWeek %d\n",__LINE__, __FILE__,__FUNCTION__,StartWeek);
    if (week>5)
    {
        if (monday>29) 
        {
            days = 1;
        }
    }
    else if (week>4)
    {
        if (monday>30) 
        {
            days = 1;
        }
    }

    uiGraphDrawJPGImage(PBListDayBG[days], 130, 185);
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

    for ( i = 1; i <= monday; i++)
    {

        day_x = week_x[week];
        day_y = week_y[weekCnt];
        dayBg_x = grid_x[week];
        dayBg_y = grid_y[weekCnt];
        dayBgIdx.bufIndex = PBListDay_1.bufIndex;
        dayBgIdx.FileName = PBListDay_1.FileName;
        if (i < 10)
        {
            day_x = week_x[week]+8;            
        }

        sprintf((char*)tmpStr, "%d", i);
        uiGraphDrawString(tmpStr, day_x, day_y, 0);

        if (dcfPlaybackDayInfo[i-1].DirNum > 0)
        {
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();
            uiGraphDrawJPGImage(PBButton, dayBg_x, dayBg_y);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
        }

        week++;
        if (week == 7)
        {
            weekCnt++;
            week = 0;
        }
    }
    return StartWeek;
}

/*FileCnt = 0, only draw one item*/
void uiGraphDrawCalendarFileLoad(int reScan)
{
    u32 waitFlag = 0;
    u16 waitTime = 5;
    u8  err, cnt = 0;

    
	if(dcfDirRescanSwitch)
	{
		reScan = 1;
		dcfDirRescanSwitch = 0;
	}
#if ((NIC_SUPPORT == 1) || (UI_USE_DEMO_UI == 1))
    if (Fileplaying == 1)
    {
        dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, reScan);
        return;
    }
#endif
	//sysPlaybackType = DCF_DISTRIB_ALL_TYPE;//之後拿到外面設
    if (sysSetEvt(SYS_EVT_PLAYBACK_CALENDAR, reScan) == 1)
    {
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PYBK_SEARCH, OS_FLAG_CLR, &err);
        uiOsdDrawLoadIcon(0, 0);
        //uiOsdDrawLoadIcon(cnt, 1);
        while(waitFlag == 0)
        {
            waitFlag = OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PYBK_SEARCH, OS_FLAG_WAIT_SET_ANY, waitTime, &err);
            if (waitFlag == FLAGSYS_RDYSTAT_PYBK_SEARCH)
            {
                uiOsdDrawLoadIcon(0, 3);
                return;
            }
            else
            {
                //uiOsdDrawLoadIcon(cnt, 2);
                cnt++;
                if (cnt == 10)
                    cnt = 0;
                //uiOsdDrawLoadIcon(cnt, 1);
            }
        }
    }
}

/*Calendar*/ 
u8 uiGraphDrawPlaybackList(u8 key, s8 ExtKey)
{
    RTC_DATE_TIME CurTime;
    int reScan;
    u8  i;
    s8  j;
    u8  MonthDayNum;
    static u8 PlayYear, PlayMon, setCursor;
    static u8 isAllCamOn, startWeek;  /* 1: all camera on  0: at least one camera on */
    
    DEBUG_UI("uiGraphDrawCalendar key %d ExtKey %d\r\n",key, ExtKey);
    DEBUG_UI("Year %d Mon %d Day %d Cam %x\r\n",sysPlaybackYear, sysPlaybackMonth, sysPlaybackDay, sysPlaybackCamList);
    reScan = 1;
    switch (key)
    {
        case 0:
            dcfNewFile = 0;
            setCursor = 1;
            isAllCamOn =1;
            PlayListDspType = UI_DSP_PLAY_LIST_DIR;
            //uiGraphDrawPlaybackListYearActGraph(0, 0);
            //uiGraphDrawPlaybackListYearActGraph(1, 0);
            
            /*Get Current time*/
            RTC_Get_Time(&CurTime);
            PlayYear = CurTime.year;
            PlayMon = CurTime.month;
            uiGraphDrawPlaybackListYM(PlayYear, PlayMon, 0);
            
            sysPlaybackYear  = PlayYear;
            sysPlaybackMonth = PlayMon;
            sysPlaybackCamList = 0x0F;
            sysPlaybackType = DCF_DISTRIB_ALL_TYPE;
            
            /* Draw Camera */
            for ( i = 0; i < 4; i++)
            {
                uiGraphDrawPlaybackListCam(i, 1);    /* red */
                /* Draw Rec Type */
                uiGraphDrawPlaybackListRecType(i,1);    /* red */
            }
      
            uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            startWeek = uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
                        
            break;

        case UI_KEY_LEFT:  
            //uiGraphDrawPlaybackListYearActGraph(0, 1);
            if (ExtKey == 1)/*year -*/
            {
                if (PlayYear == 0)
                    PlayYear = 63;
                else
                    PlayYear --;
            }
            else /*month -*/
            {
                if (PlayMon == 1)
                    PlayMon = 12;
                else
                    PlayMon --;
            }

            uiGraphDrawPlaybackListYM(PlayYear, PlayMon, 0);
            sysPlaybackYear=PlayYear;
            sysPlaybackMonth=PlayMon;
            uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            startWeek = uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            //uiGraphDrawPlaybackListYearActGraph(0, 0);
            break;

        case UI_KEY_RIGHT:  
           // uiGraphDrawPlaybackListYearActGraph(1, 1);
           if (ExtKey == 1)/*year +*/
            {
                if (PlayYear == 63)
                    PlayYear = 0;
                else
                    PlayYear ++;
            }
            else /*month +*/
            {
                if (PlayMon == 12)
                    PlayMon = 1;
                else
                    PlayMon ++;
            }

            uiGraphDrawPlaybackListYM(PlayYear, PlayMon, 0);
            sysPlaybackYear=PlayYear;
            sysPlaybackMonth=PlayMon;
            uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            startWeek = uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            //uiGraphDrawPlaybackListYearActGraph(1, 0);
            break;

        case UI_KEY_DOWN:   /*Cam Sel*/
            sysPlaybackCamList ^= 0x01<<ExtKey;
            
            for ( i = 0; i < 4; i++)
            {
                if (sysPlaybackCamList & (0x01<<i))
                    uiGraphDrawPlaybackListCam(i, 1);
                else
                    uiGraphDrawPlaybackListCam(i, 0);
            }

            uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            startWeek = uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            break;

        case UI_KEY_UP:   /*REC TYPE Sel*/
            #if (DCF_RECORD_TYPE_API == 0)
                return;
            #endif

            {
                u8 value = 0;
                
                switch(ExtKey)
                {
                    case file_m:
                        sysPlaybackType ^= DCF_DISTRIB_MANU;
                        if (sysPlaybackType & DCF_DISTRIB_MANU)
                            value = 1;
                        break;
                        
                    case file_s:
                        sysPlaybackType ^= DCF_DISTRIB_SCHE;
                        if (sysPlaybackType & DCF_DISTRIB_SCHE)
                            value = 1;
                        break;
                        
                    case file_d:
                        sysPlaybackType ^= DCF_DISTRIB_MOTN;
                        if (sysPlaybackType & DCF_DISTRIB_MOTN)
                            value = 1;
                        break;
                        
                    case file_r:
                        sysPlaybackType ^= DCF_DISTRIB_RING;
                        if (sysPlaybackType & DCF_DISTRIB_RING)
                            value = 1;
                        break;
                }
            
                if (value) 
                    uiGraphDrawPlaybackListRecType(ExtKey,1);
                else
                    uiGraphDrawPlaybackListRecType(ExtKey,0);            
            }

            uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            startWeek = uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            break;

        case UI_KEY_ENTER:            
            setCursor = (UiGetTouchX-81)/72+(UiGetTouchY-148)/61*7;
            if (setCursor < startWeek)
                return 0;
            
            sysPlaybackDay = setCursor-startWeek+1;
            MonthDayNum = rtcGetDayNum(PlayYear, PlayMon);
            if (sysPlaybackDay > MonthDayNum)
                return 0;

            if(dcfPlaybackDayInfo[sysPlaybackDay-1].DirNum == 0)
                return 0;
            
            PlayYear = sysPlaybackYear;
            PlayMon = sysPlaybackMonth;
            DEBUG_UI("sysPlaybackDay %d, CamList %x setCursor %d\r\n",sysPlaybackDay, sysPlaybackCamList, setCursor);
            uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
            IduVideo_ClearPKBuf(0);
            uiOsdDrawPlaybackMenu(0);
            break;

        case UI_KEY_MODE:
        	reScan = 0;
            DEBUG_UI("From File List to Calendar\r\n");
            uiGraphGetMenuData();
            //uiGraphDrawPlaybackListYearActGraph(0, 0);
            //uiGraphDrawPlaybackListYearActGraph(1, 0);
            uiGraphDrawPlaybackListYM(PlayYear, PlayMon, 0);
            
            /* Draw Camera */
            if (sysPlaybackCamList == 0x0F)
            {
                for ( i = 0; i < 4; i++)
                {
                    uiGraphDrawPlaybackListCam(i, 1);
                }
            }
            else
                for ( i = 0; i < 4; i++)
                {
                    if (sysPlaybackCamList & (0x01<<i))
                        uiGraphDrawPlaybackListCam(i, 1);
                    else
                        uiGraphDrawPlaybackListCam(i, 0);
                }
                
            /* Draw FILE TYPE */
            if (sysPlaybackType == 0xf00)
            {
                for ( i = 0; i < 4; i++)
                {
                    uiGraphDrawPlaybackListRecType(i,1);
                }
            }
            else
                for ( i = 0; i < 4; i++)
                {
                    if (sysPlaybackType & (0x100<<i))
                        uiGraphDrawPlaybackListRecType(i,1);
                    else
                        uiGraphDrawPlaybackListRecType(i,0);
                }

            if (ExtKey == 1)
            {
                uiGraphDrawCalendarFileLoad(reScan);
                //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            }
            uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            break;

        case UI_KEY_DELETE:
            uiGraphGetMenuData();
            //uiGraphDrawPlaybackListYearActGraph(0, 0);
            //uiGraphDrawPlaybackListYearActGraph(1, 0);
            uiGraphDrawPlaybackListYM(sysPlaybackYear, sysPlaybackMonth, 0);
            setCursor = 0;
            
            /* Draw Camera */
            if (sysPlaybackCamList == 0x0F)
            {
                uiGraphDrawPlaybackListCam(0, 1);
                for ( i = 1; i < 5; i++)
                {
                    uiGraphDrawPlaybackListCam(i, 0);
                }
            }
            else
            {
                uiGraphDrawPlaybackListCam(0, 0);
                for ( i = 1; i < 5; i++)
                {
                    if((sysPlaybackCamList >> (i-1))& 0x01)
                    {
                        uiGraphDrawPlaybackListCam(i, 1);
                    }
                    else
                    {
                        uiGraphDrawPlaybackListCam(i, 0);
                    }
                }
            }

            /* Draw FILE TYPE */
            if (sysPlaybackType == 0x0F)
            {
                uiGraphDrawPlaybackListRecType(0, 1);
                for ( i = 1; i < 5; i++)
                {
                    uiGraphDrawPlaybackListRecType(i, 0);
                }
            }
            else
            {
                uiGraphDrawPlaybackListRecType(0, 0);
                for ( i = 1; i < 5; i++)
                {
                    if((sysPlaybackType >> (i-1))& 0x01)
                    {
                        uiGraphDrawPlaybackListRecType(i, 1);
                    }
                    else
                    {
                        uiGraphDrawPlaybackListRecType(i, 0);
                    }
                }
            }
                
            PlayYear = sysPlaybackYear;
            PlayMon = sysPlaybackMonth;
            //uiGraphDrawCalendarFileLoad(reScan);
            //dcfPlaybackCalendarInit(sysPlaybackYear, sysPlaybackMonth, sysPlaybackCamList, DCF_DISTRIB_ALL_TYPE, 1);
            uiGraphDrawPlaybackListCalendar(PlayYear, PlayMon);
            for (j = 30; j > 0; j--)
            {
                if(dcfPlaybackDayInfo[j].DirNum > 0)
                {
                    setCursor = j+6;
                    break;
                }
            }
            //uiOsdDrawGraphSet(1);
            break;
            
        case UI_KEY_MENU:
            #if (UI_SUPPORT_TREE == 1)
            #if 0
            for(i=0; i < MULTI_CHANNEL_MAX; i++)
            {
                if (MultiChannelGetCaptureVideoStatus(i) == UI_REC_STATUS_RECING)
                {
                    uiCurRecStatus[i]=UI_REC_STATUS_RECING;
                }
                else
                    uiCurRecStatus[i]=UI_REC_STATUS_NONE;
            }
            #endif
            DEBUG_UI("Playback list to Setup mode\r\n");
            uiCaptureVideoStop();
            MyHandler.MenuMode = SETUP_MODE;
            uiOsdDisableAll();
            uiEnterMenu(UI_MENU_NODE_PLAYBACK);
            uiGraphDrawMenu();
            uiReturnPreview = UI_MENU_TO_PRV;
            dcfPlaybackCurDir = dcfGetVideoDirListTail();
            dcfScanFileOnPlaybackDir();
            #else
            DEBUG_UI("Not Support Setup mode\r\n");
            #endif
            break;
            
        default:
            DEBUG_UI("uiGraphDrawPlaybackList error key %d\r\n",key);
    }
    return 1;
}
#endif

void uiGraphDrawPreviewListGraph(s8 setCursor)
{
    u8 i;
    u8 index;
    u16 y_fram[4]= {105,205,305,405};
    u16 x_pos=250;  
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
    for(i=0;i < MULTI_CHANNEL_MAX;i++)
    {
        if(i == setCursor)
        {
            index=Sensor_cam[i];
        }
        else
        {
            index=Sensor_cam[i]+4;    
        }

        uiGraphDrawJPGImage(Cam_Image[index+8*CurrLanguage],x_pos,y_fram[i]);
        uiGraphDrawJPGImage(Edit_bar,10,y_fram[i]);
    }   
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawPreviewList(u8 key)
{
    u8  err;
    static s8 setCursor;	
    u16 y_fram[4]= {105,205,305,405};
    u16 x_pos=250;
    
    DEBUG_UI("uiGraphDrawPreviewList Key %d\n",key);

    switch(key)
    {
        case 0:
            setCursor=0;
            #if 0
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                cam_info[i][2] = iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i];    
            }
            #endif
            uiGraphDrawPreviewListGraph(0);
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            if(key == UI_KEY_RIGHT)
            {
                if (Sensor_cam[setCursor] > 2)
                    Sensor_cam[setCursor] = 0;
                else
                    Sensor_cam[setCursor]++;
            }
            else
            {
                if (Sensor_cam[setCursor] < 1)
                    Sensor_cam[setCursor] = 3;
                else
                    Sensor_cam[setCursor]--;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();
            uiGraphDrawJPGImage(Cam_Image[Sensor_cam[setCursor]+8*CurrLanguage],x_pos,y_fram[setCursor]);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;
            
        case UI_KEY_UP:
        case UI_KEY_DOWN:

            if(key == UI_KEY_DOWN)
            {
                setCursor++;
                if (setCursor > 3)
                    setCursor = 0;
            }
            else
            {
                setCursor--;
                if (setCursor < 0)
                    setCursor = 3;
            }
            uiGraphDrawPreviewListGraph(setCursor);
            break;
        case UI_KEY_ENTER:
            #if 0
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                if(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i] != cam_info[i][2])
                {
                    iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i]=cam_info[i][2];
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i],i);
                }
            }
            #endif
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            //Save_UI_Setting();
            break;
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
        default:
            DEBUG_UI("uiGraphDrawPreviewList error key %d\r\n",key);
            break;
           
           
    }
}


void uiGraphDrawScheduledTime(u8 setCursor, u8 act)
{
    u16 time_xpos[4]   ={294, 434, 646, 779 };
    u8  num;
    u8  numIndex[2];
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    switch (setCursor)
    {
        case 0:
            num=scheduleStartTimeSetting[0]; 
            break;
        case 1:
            num=scheduleStartTimeSetting[1]; 
            break;
        case 2:
            num=scheduleEndTimeSetting[0]; 
            break;
        case 3:
            num=scheduleEndTimeSetting[1]; 
            break;
            
    }
    numIndex[0] = (num/10)+(act*10);
    numIndex[1] = (num%10)+(act*10);  
    uiGraphDrawJPGImage (TimeNum[numIndex[0]],time_xpos[setCursor],238); 
    uiGraphDrawJPGImage (TimeNum[numIndex[1]],time_xpos[setCursor]+ 42,238);
    
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}


void uiGraphDrawScheduledTimeSelect(u8 select,u8 mode)
{
    u8 i, change_one = 1;
    /* mode=1:inrease time, mode=0 decrease time */
     DEBUG_UI("===> uiGraphDrawScheduledTimeSelect %d\r\n",select);
    switch(select)
    {
        case 4:
            if(mode ==1)
                scheduleStartTimeSetting[0]++;
            else
                scheduleStartTimeSetting[0]--;
            
            if(scheduleStartTimeSetting[0] > 23)
                scheduleStartTimeSetting[0]=0;
            else if(scheduleStartTimeSetting[0]<0)
                scheduleStartTimeSetting[0]=23;
            break;

        case 5:
            if(mode ==1)
                scheduleStartTimeSetting[1]+=30;
            else
                scheduleStartTimeSetting[1]-=30;
            
            if(scheduleStartTimeSetting[1] == 60)
                scheduleStartTimeSetting[1]=0;
            else if(scheduleStartTimeSetting[1]<0)
                scheduleStartTimeSetting[1]=30;
            
            break;
        case 6:
            if(mode ==1)
                scheduleEndTimeSetting[0]++;
            else
                scheduleEndTimeSetting[0]--;
            
            if(scheduleEndTimeSetting[0] > 24)
                scheduleEndTimeSetting[0]=scheduleStartTimeSetting[0]+1;
            else if(scheduleEndTimeSetting[0]<0)
            {
                scheduleEndTimeSetting[0]=24;
            }
            break;
        case 7:
            if(mode ==1)
                scheduleEndTimeSetting[1]+=30;
            else
                scheduleEndTimeSetting[1]-=30;
            
            if(scheduleEndTimeSetting[1] == 60)
                scheduleEndTimeSetting[1]=0;
            else if(scheduleEndTimeSetting[1]<0)
                scheduleEndTimeSetting[1]=30;
            break;
            
    }

    /*check value*/
    if ((scheduleEndTimeSetting[0] == 24) && (scheduleEndTimeSetting[1] != 0))
    {
        change_one = 0;
        scheduleEndTimeSetting[1] = 0;
    }
    if (scheduleEndTimeSetting[0] < scheduleStartTimeSetting[0])
    {
        change_one = 0;
        scheduleEndTimeSetting[0] = 24;
        scheduleEndTimeSetting[1] = 0;
    }
    else if (scheduleEndTimeSetting[0] == scheduleStartTimeSetting[0])
    {
        if (scheduleEndTimeSetting[1] < scheduleStartTimeSetting[1])
        {
            change_one = 0;
            scheduleEndTimeSetting[0] += 1;
            scheduleEndTimeSetting[1] = 0;
        }
        else if (scheduleEndTimeSetting[1] == scheduleStartTimeSetting[1])
        {
            change_one = 0;
            if (scheduleEndTimeSetting[1] == 30)
            {
                scheduleEndTimeSetting[0] += 1;
                scheduleEndTimeSetting[1] = 0;
            }
            else
                scheduleEndTimeSetting[1] = 30;
        }
    }

    if (change_one == 1)
        uiGraphDrawScheduledTime(select-4,1);
    else
    {
        for (i = 0; i < 4; i++)
            uiGraphDrawScheduledTime(i,0);
    }
}


void uiGraphDrawScheduledSettingGraph(u8 select, u8 act)
{
    u16 camera_ypos[4] = {130,230,332,434};
    u16 day_xpos[7]    ={230, 333, 436, 539, 642, 745, 848};
    u8 idx;
    u8  err;


    DEBUG_UI("===> uiGraphDrawScheduledSettingGraph %d\r\n",select);
    if(select < 4)           /* Camera */
    {
        idx= select + act*4;
        OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        sysJPEG_enable();
        uiGraphDrawJPGImage(Camera_Btn[idx],42,camera_ypos[select]);
        sysJPEG_disable();
        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    }
    else if (select < 8)    /* Time */
    {
        idx = (select-4) ;
        uiGraphDrawScheduledTime(idx,act); 
    }
    else if (select < 15)    /* Day */
    {
        idx = (select -8) + act*7;
        OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        sysJPEG_enable();
        uiGraphDrawJPGImage(Day_Btn[idx+14*CurrLanguage],day_xpos[select-8],396);
        sysJPEG_disable();
        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    }
    #if 0
    else if (select < 16)    /* Set */
    {
        if(act)
        {
            uiGraphDrawJPGImage(Set_1_M[CurrLanguage],290,380);     
        }
        else
        {
            uiGraphDrawJPGImage(Set_2_M[CurrLanguage],290,380);  
        }
    }
    else if (select < 17)    /* Delete */
    {
        if(act)
        {
            uiGraphDrawJPGImage(Delete_1_M[CurrLanguage],435,380);     
        }
        else
        {
            uiGraphDrawJPGImage(Delete_2_M[CurrLanguage],435,380);  
        }    
    }
   #endif
}


u8 uiGraphDrawScheduledSetting(u8 key)
{
    static s8 setCursor=0 ;
    static u8 tempInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
    static u8 index=UI_MENU_SETIDX_CH1_LS_TIMER;//now setting index
    u8 i,j,k,bit,count=0,value=0,days=0;
    u8 isSetVal=0;  /* 0: delete value , 1: set value */ 
    u8 startTime,endTime;
    static u8 setWeek, setCam;
    
    switch (key)
    {
        case 0:
            setCursor=0;
            setWeek = 0;
            setCam = 0;
            for (i=0;i < MULTI_CHANNEL_MAX;i++)
                CameraSetting[i]=0;
            for (i=0;i<7;i++)
                DaysSetting[i]=0;
            
            scheduleStartTimeSetting[0]=0;
            scheduleStartTimeSetting[1]=0;                
            scheduleEndTimeSetting[0]=24;
            scheduleEndTimeSetting[1]=0;
            for(i=4;i<8;i++)
            {
                uiGraphDrawScheduledSettingGraph(i, 0);  
            }
            for(i=8;i<15;i++)
            {
                uiGraphDrawScheduledSettingGraph(i, 0);  
            }            
            //uiOsdDrawSetting(1);
            //uiOsdDrawScheduledFrame(setCursor, 0);
#if UI_LIGHT_SUPPORT
            if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Light Sch"))
            {
                memcpy(&tempInterval,&uiLightInterval,sizeof(uiLightInterval));
                index=UI_MENU_SETIDX_CH1_LS_TIMER;
            }
#endif
#if UI_CAMERA_ALARM_SUPPORT     
            if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Alarm Sch"))
            {
                memcpy(&tempInterval,&uiCamAlarmInterval,sizeof(uiCamAlarmInterval));
                index=UI_MENU_SETIDX_CH1_CA_TIMER;
            }
#endif
#if UI_BAT_SUPPORT
            if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set BatCam Scheduled"))
            {
                memcpy(&tempInterval,&uiBatteryInterval,sizeof(uiBatteryInterval));
            }
#endif 
            UICheckSchedule = 48;
            break;

        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if ( (setCursor > 3)&& (setCursor < 8) )  //draw scheduled time
            {
                if( key == UI_KEY_UP)
                {
                    uiGraphDrawScheduledTimeSelect(setCursor,1);      
                }
                else
                {
                    uiGraphDrawScheduledTimeSelect(setCursor,0);      
                }            

            }
            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            
            break;

        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
            {
                uiOsdDrawScheduledFrame(setCursor, 1);
                if ( (setCursor > 3)&& (setCursor < 8) )  //clear scheduled time
                {
                    uiGraphDrawScheduledSettingGraph(setCursor, 0);    
                }
                setCursor = TouchExtKey;
                //uiOsdDrawScheduledFrame(setCursor, 0);
                if ( (setCursor > 3)&& (setCursor < 8) )  //draw scheduled time
                {
                    uiGraphDrawScheduledSettingGraph(setCursor, 1);    
                }
            }
            if(setCursor <4)    /* Camera */
            {
                CameraSetting[setCursor]^=1;
                uiGraphDrawScheduledSettingGraph(setCursor, CameraSetting[setCursor]);
                setCam = 0;
                for (i=0; i<4;i++)
                {
                    if (CameraSetting[i] == 1)
                    {
                       setCam = 1; 
                    }
                }

            }
            else if( (setCursor > 7) && (setCursor <15) )  /* Day */
            {
                DaysSetting[setCursor-8]^=1;
                uiGraphDrawScheduledSettingGraph(setCursor, DaysSetting[setCursor-8]);
                setWeek= 0;
                for (i=0; i<7;i++)
                {
                    if (DaysSetting[i] == 1)
                    {
                       setWeek = 1; 
                    }
                }
            }
            else if(( setCursor == 15) || ( setCursor == 16) )  
            {
                if (setCam ==0) 
                {
                    osdDrawSchduleWarnMsg(0, UI_OSD_DRAW);
                    break;
                }

                if (setWeek == 0) 
                {
                    osdDrawSchduleWarnMsg(1, UI_OSD_DRAW);
                    break;
                }
               
                if(setCursor == 15)  /* Set */
                    isSetVal=1;
                else                 /* Delete */
                    isSetVal=0;
                
                if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set Scheduled"))
                {

                    for(i=0; i<7;i++)
                    {
                        if (DaysSetting[i] == 0)
                            continue;  

                        for(j=0; j<4;j++)
                        {
                            if (CameraSetting[j] == 0)
                                continue;    
                                  
                            if (scheduleStartTimeSetting[1] == 0)
                                startTime = scheduleStartTimeSetting[0]*2;
                            else
                                startTime = scheduleStartTimeSetting[0]*2+1;
                            
                            if (scheduleEndTimeSetting[1] == 0)
                                endTime = scheduleEndTimeSetting[0]*2;
                            else
                                endTime = scheduleEndTimeSetting[0]*2+1;

                            for (k = startTime; k < endTime; k++)
                            {
                                if (isSetVal == 0)   /*delete*/
                                    uiScheduleTime[i][j][k] = UI_MENU_SCHEDULE_OFF;
                                else
                                    uiScheduleTime[i][j][k] = UI_MENU_SCHEDULE_REC;
                        
                            }
                        }
                    } 
                    UICheckSchedule = 48;
                 //DEBUG_RED("%d %d %d %d\n",scheduleStartTimeSetting[0],scheduleStartTimeSetting[1],scheduleEndTimeSetting[0],scheduleEndTimeSetting[1]); 
                }
                else
                {
                    for(j=0; j<4;j++)//cam
                    {
                        
                        if (CameraSetting[j] == 0)
                            continue; 

                        for(i=0; i<7;i++)
                        {
                            if (DaysSetting[i] == 0)
                                continue;  
                            
                            days ^= (0x01 << i);

                            if (scheduleStartTimeSetting[1] == 0)
                                startTime = scheduleStartTimeSetting[0]*2;
                            else
                                startTime = scheduleStartTimeSetting[0]*2+1;
                            
                            if (scheduleEndTimeSetting[1] == 0)
                                endTime = scheduleEndTimeSetting[0]*2;
                            else
                                endTime = scheduleEndTimeSetting[0]*2+1;
                 //DEBUG_RED("%d %d %d %d\n",scheduleStartTimeSetting[0],scheduleStartTimeSetting[1],scheduleEndTimeSetting[0],scheduleEndTimeSetting[1]); 
                            count = 0;
                            value = tempInterval[j][i][count];
                            for (bit=0; bit < 48; bit++)
                            {
                                if ((bit >= startTime) && (bit < endTime))
                                {
                                    if (isSetVal == 0) //delete  
                                        value &= ~(0x01<<(bit%8));
                                    else
                                        value |= (0x01<<(bit%8));
                                }
                                    
                                if (((bit+1)%8)==0)
                                {
                                    tempInterval[j][i][count++] = value;
                                    value = tempInterval[j][i][count];//next bit
                                }
                            }
                            
                        }    
                        days = 0;
                    }
#if UI_LIGHT_SUPPORT     
                    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Light Sch"))
                        memcpy(&uiLightInterval,&tempInterval,sizeof(tempInterval));
#endif
#if UI_CAMERA_ALARM_SUPPORT     
                    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Alarm Sch"))
                        memcpy(&uiCamAlarmInterval,&tempInterval,sizeof(tempInterval));
#endif
#if UI_BAT_SUPPORT
                    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set BatCam Scheduled"))
                        memcpy(&uiBatteryInterval,&tempInterval,sizeof(tempInterval));
#endif
                    for (i=0; i < 4; i++)
                    {
                        #if(UI_BAT_SUPPORT)
                        if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set BatCam Scheduled"))
                        {
                            _uiCheckBatterySch = 48;                      
                        }
                        else
                        #endif    
                        {
                        #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
                            uiSetCfg = 1;
                            uiMenuAction(index+i);                        
                            uiSetCfg = 0;
                        #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
                            uiSetCfg = 0x11; //Enable light control(always on) and User setting
                            uiMenuAction(index+i);
                            uiSetCfg = 0x01; //Enable light control(always on)
                        #endif
                        }
                    }

                } 
                Save_UI_Setting();
                osdDrawSchduleWarnMsg(1, UI_OSD_CLEAR);
                
                if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set Scheduled"))
                    uiEnterMenu(UI_MENU_NODE_SCHEDULED_BAR);
                #if (UI_LIGHT_SUPPORT==1) 
                else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Light Sch"))
                    uiEnterMenu(UI_MENU_NODE_TX_LIGHT_SCH_BAR);
                #endif
                #if (UI_CAMERA_ALARM_SUPPORT==1)
                else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set TX Alarm Sch"))
                    uiEnterMenu(UI_MENU_NODE_TX_ALARM_SCH_BAR);
                #endif
                #if(UI_BAT_SUPPORT)
                else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Set BatCam Scheduled"))
                    uiEnterMenu(UI_MENU_NODE_BATCAM_SCHEDULED_BAR);
                #endif
                uiGraphDrawMenu();
            }

            if (uiConTouchPress == 0)
            {
                /*避免執行Key連續發送*/
                OSTimeDly(UI_TOUCH_ACT_DELAY);
            }
            break;

        case UI_KEY_MENU:
            osdDrawSchduleWarnMsg(1, UI_OSD_CLEAR);
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;

        default:
            break;
           
    }
    return 0;
}

void uiGraphDrawScheduledTable(u8 day)
{
    u8 x_bg = 144;
    u8 y_bg = 128;
    u8 i, j, index, bSet, bit, value=0;
    u8 distance=0;
    u16 DrawY[4] = {141,242,343,444};
    u16 DrawCamY[4] = {127,228,330,430};
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();
    
#if UI_LIGHT_SUPPORT
    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "TX Light Sch Bar"))
        memcpy(&TempTime,&uiLightInterval,sizeof(uiLightInterval));
#endif
#if UI_CAMERA_ALARM_SUPPORT  
    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "TX Alarm Sch Bar"))
        memcpy(&TempTime,&uiCamAlarmInterval,sizeof(uiCamAlarmInterval));
#endif
#if UI_BAT_SUPPORT
    if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "BatCam Scheduled Bar"))
        memcpy(&TempTime,&uiBatteryInterval,sizeof(uiBatteryInterval));
#endif

    uiGraphDrawJPGImage(schbar, x_bg, y_bg);
    
    for (i=0; i < MULTI_CHANNEL_MAX; i++)
    {
        bSet = 0;
        if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Scheduled Bar"))
        {
            for(j=0;j<48;j++)
            {           
                if(uiScheduleTime[day][i][j] > UI_MENU_SCHEDULE_OFF )
                {
                    bSet = 1;
                    switch (uiScheduleTime[day][i][j] )
                    {
                        case UI_MENU_SCHEDULE_REC:                        
                            index = j/2;
                            if (j%2==0)
                            {
                                distance = 170;                       
                            }
                            else
                            {
                                distance = 182;
                            }
                            uiGraphDrawJPGImage(yellow_item,distance+(index*33),DrawY[i]);
                            break;
                    }
                }
            }
        }
        else //Light Timer 
        {
            for(j=0;j<6;j++)
            {        
                if (TempTime[i][day][j] > 0)
                {
                    bSet = 1;
                    for(bit=0;bit<8;bit++)
                    {
                        value = j*8+bit;
                        
                        if ((TempTime[i][day][j]) & (0x1 << bit)) //0x1:UI_MENU_SCHEDULE_REC
                        {
                            index = value/2;
                            if ((value%2)==0)
                            {
                                distance = 170;                       
                            }
                            else
                            {
                                distance = 182;
                            }
                            uiGraphDrawJPGImage(yellow_item,distance+(index*33),DrawY[i]);
                        } 

                    }
                }
            }           
        }
        uiGraphDrawJPGImage(Camera_Btn[(4*bSet)+i], 42, DrawCamY[i]);     

    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

#if UI_LIGHT_SUPPORT
void uiGraphDrawLightScheduledTable(u8 day)
{
    u8 x_bg = 144;
    u8 y_bg = 128;
    u8 i, j, index, bSet, bit;
    u8 distance=0;
    u16 DrawY[4] = {141,242,343,444};
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    uiGraphDrawJPGImage(schbar, x_bg, y_bg);
    
    for (i=0; i < MULTI_CHANNEL_MAX; i++)
    {
        bSet = 0;
        for(j=0;j<6;j++)
        {           
            bSet = 1;
            for(bit=0;j<8;j++)
            {
                if( uiLightInterval[i][day][j] & (0x1 << bit))
                {
                    switch (uiLightInterval[i][day][j] )
                    {
                        case UI_MENU_SCHEDULE_REC:
                            if (j%2==1)
                                distance = 33;
                            else
                                distance = 13;
                            index = j/2;
                            uiGraphDrawJPGImage(yellow_item,170+(index*distance),DrawY[i]);
                            break;
                    }
                }
            }
        }
      uiGraphDrawJPGImage(Camera_Btn[(4*bSet)+i], 42, DrawY[i]);     
      
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    
}
#endif
void uiGraphDrawScheduled(u8 key)
{
    static s8 setCursor=0 ;
    //static s8 setLevel=0;  /* 0: in Scheduled Table,  1: in Scheduled Setting */
    //u8 ret_val=0;
    u8  err;
#if 0
    if (setLevel == 1) 
    {
        ret_val=uiGraphDrawScheduledSetting(key);
        if((ret_val ==1) || (key == UI_KEY_MENU))
        {
            setLevel =0;
            key=0;
            uiOsdDrawSetting(0);
        }
        else
        {
            return ;    
        }

            
    }
#endif    
    switch (key)
    {
        case 0:
            setCursor=0;
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();
            uiGraphDrawJPGImage(ScheduledTitle_Image[setCursor+7*CurrLanguage],420,24);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            uiGraphDrawScheduledTable(0);
            break;
            
        case UI_KEY_LEFT:
        case UI_KEY_RIGHT:
            if(key == UI_KEY_LEFT)
            {
                setCursor--;
                if(setCursor <0 )
                    setCursor=6;
            }
            else
            {
                setCursor++;
                if(setCursor >6 )
                    setCursor = 0;
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();
            uiGraphDrawJPGImage(ScheduledTitle_Image[setCursor+7*CurrLanguage],420,24);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            uiGraphDrawScheduledTable(setCursor);          
            break;

        case UI_KEY_ENTER:
            if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "Scheduled Bar"))
                uiEnterMenu(UI_MENU_NODE_SET_SCHEDULED);
            #if(UI_BAT_SUPPORT)
            else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "BatCam Scheduled Bar"))
                uiEnterMenu(UI_MENU_NODE_SET_BATCAM_SCHEDULED);
            #endif
            #if (UI_LIGHT_SUPPORT==1) 
            else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "TX Light Sch Bar"))
                uiEnterMenu(UI_MENU_NODE_SET_TX_LIGHT_SCH);
            #endif
            #if (UI_CAMERA_ALARM_SUPPORT==1) 
            else if (!strcmp((const char*)uiCurrNode->item.NodeData->Node_Name, "TX Alarm Sch Bar"))
                uiEnterMenu(UI_MENU_NODE_SET_TX_ALARM_SCH);
            #endif
            uiGraphDrawMenu();
            break;

        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;

        default:
            DEBUG_UI("uiGraphDrawScheduled error key %d\r\n",key);
            break;
            

       
    }
}

void uiGraphDrawSection(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[3]= {105,205,305};
    u16 x_pos;
    
    DEBUG_UI("uiGraphSection Key %d\n",key);
    
    switch(key)
    {
        case 0:
            value = iconflag[UI_MENU_SETIDX_SECTION];
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;
            
        case UI_KEY_UP:
            value = TouchExtKey;
            for (i=0;i<3;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;
            
        case UI_KEY_ENTER:
            iconflag[UI_MENU_SETIDX_SECTION] = value;
            uiMenuSet_Section(iconflag[UI_MENU_SETIDX_SECTION]);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawBrightness error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawOverwrite(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[2]= {105,205};
    u16 x_pos=210;

    enum
    {
        yes = 0,
        no,
    };
    
    DEBUG_UI("uiGraphDrawOverwrite Key %d\n",key);

    switch(key)
    {
        case 0:
            value = iconflag[UI_MENU_SETIDX_OVERWRITE];
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;
             
        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if (key == UI_KEY_UP)
                value = yes;
            else
                value = no;
            for (i=0;i<2;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;             
                                                              
        case UI_KEY_ENTER:
            iconflag[UI_MENU_SETIDX_OVERWRITE] = value;
            uiMenuSet_Overwrite(iconflag[UI_MENU_SETIDX_OVERWRITE]);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawOverwrite error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawFomat(u8 key)
{
    static s8 value;	
    u8 i;
    u16 y_pos[2]= {105,205};
    u16 x_pos = 210;
    
    enum
    {
        no = 0,
        yes,
    };
    
    DEBUG_UI("uiGraphDrawFomat Key %d\n",key);

    switch(key)
    {
        case 0:
            value = iconflag[UI_MENU_SETIDX_FORMAT];
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;
            
        case UI_KEY_UP:
        case UI_KEY_DOWN:
            if (key == UI_KEY_UP)
                value = yes;
            else
                value = no;
            for (i=0;i<2;i++)
                uiGraphDrawSelectGraph(x_pos,y_pos[i],0);//clear
            uiGraphDrawSelectGraph(x_pos,y_pos[value],1);//draw
            break;  
            
        case UI_KEY_ENTER:
            if (value == yes)
                value = UI_MENU_FORMAT_YES;
            else
                value = UI_MENU_FORMAT_NO;
            uiMenuSet_Format(value);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawFomat error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawCardInfoNum(u8 select,u32 num)
{
    u8  val, cnt = 0;
#if((HW_BOARD_OPTION == MR8200_RX_JIT)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8120_RX_JIT_LCD)||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_BOX))
    u16 y_pos[3]={142,212,282};
    u16 x_pos[4]={455,420,400, 380};
#else
    u16 y_pos[3]={140,230,318};
    u16 x_pos[4]={385,350,330, 310};
#endif
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    switch(select)
    {
        case 0:    /*total size */
            do
            {
                val = num %10;
                num/=10;                
                uiGraphDrawJPGImage(TimeNum[val],(x_pos[0] - cnt*20), y_pos[select]);
                cnt++;
            }while(num != 0);
            break;
            
        case 1:    /* usage size */
        case 2:    /* reamin size */
            do
            {
                val = num %10;
                num/=10;
                uiGraphDrawJPGImage(TimeNum[val], x_pos[cnt], y_pos[select]);
                cnt++;
            }while ((num != 0) || (cnt == 1));
            break;

        default:
            DEBUG_UI("Error select %d in uiGraphDrawCardInfoNum\n",select);
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

}


void  uiGraphDrawCardInfo(u8 key)
{
    switch (key)
    {
        case 0:
            uiOsdDrawCardInfo(UI_OSD_DRAW);
            break;

        case UI_KEY_MENU:
            uiOsdDrawCardInfo(UI_OSD_CLEAR);
            uiFrowGoToLastNode();           
            uiGraphDrawMenu();
            break;

        default:
            DEBUG_UI("uiGraphDrawCardInfo error key %d\r\n",key);
            break;
    }
}

void uiGraphDrawVersionInfo(u8 key)
{
    switch (key)
    {
        case 0:
            uiOsdDrawFWVersion(UI_OSD_DRAW);            
            break;
            
        case UI_KEY_MENU:
            uiOsdDrawFWVersion(UI_OSD_CLEAR);            
            uiFrowGoToLastNode();         
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawVersionInfo error key %d\r\n",key);
            break;                 
    }
}

#if NIC_SUPPORT
void uiGraphDrawNetworkInfoNum(u8 select)
{
    u16 x_pos[4]   = {368, 444, 520, 592};
    u16 y_pos[4]   = {210, 268, 326, 384};
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    u8  i;
    u8  tempStr[4] = {0};
    UI_NET_INFO     NetInfo;
    u8 *ip;
    
    GetNetworkInfo(&NetInfo);
    switch(select)
    {
        case 0:
            ip = NetInfo.IPaddr;
            break;

        case 1:
            ip = NetInfo.Netmask;
            break;

        case 2:
            ip = NetInfo.Gateway;
            break;

        case 3:
            uiOSDASCIIStringByColor(uiP2PID, 340 ,y_pos[3] , OSD_BLK[sysTVOutOnFlag] , 0xc1, 0x00);
            return;
    }

    for(i=0; i < MULTI_CHANNEL_MAX; i++)
    {
        sprintf((u8 *) tempStr ,"%3d", ip[i]);
        uiOSDASCIIStringByColor(tempStr, x_pos[i] ,y_pos[select] , OSD_BLK[sysTVOutOnFlag] , 0xc1, 0x00);            
    }

    return;
}


void uiGraphDrawNetworkInfo(u8 key)
{

    UI_NET_INFO     NetInfo;

    switch (key)
    {
        case 0:
            GetNetworkInfo(&NetInfo);
            uiDrawNetworkInfo(&NetInfo, UI_OSD_DRAW);
            break;
            
        case UI_KEY_MENU:
            uiDrawNetworkInfo(NULL, UI_OSD_CLEAR);
            uiFrowGoToLastNode();           
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawNetworkInfo error key %d\r\n",key);
            break;                 
    }
}
#endif

void uiGraphDrawAPPInfo(u8 key)
{
    switch (key)
    {
        case 0:
            uiOsdDrawFWVersion(UI_OSD_DRAW);            
            break;
            
        case UI_KEY_MENU:
            uiOsdDrawFWVersion(UI_OSD_CLEAR);            
            uiFrowGoToLastNode();         
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawVersionInfo error key %d\r\n",key);
            break;                 
    }
}


void uiGraphDrawCameraOnOffGraph(s8 setCursor)
{
    u8 value;
    u16 y_fram[4]= {135,240,345,458};
    u16 x_pos=360;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    value=cam_info[setCursor][0];
    
    DEBUG_UI(" Draw CamOnOff Graph %d %d\n",setCursor,value);
    
    uiGraphDrawJPGImage(OnOff_Image[setCursor+value*4],x_pos,y_fram[setCursor]);
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawCameraOnOff(u8 key)
{
    u8 i,set=0;
    
    DEBUG_UI("uiGraphDrawCameraOnOff Key %d\n",key);

    switch(key)
    {
        case 0:            
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                cam_info[i][0] = iconflag[UI_MENU_SETIDX_CH1_ON+i]; 
                uiGraphDrawCameraOnOffGraph(i);                
            }
            break;

        case UI_KEY_RIGHT:
            cam_info[cam4][0] ^= 1;
            uiGraphDrawCameraOnOffGraph(cam4);                
            break;

        case UI_KEY_LEFT:
            cam_info[cam3][0] ^= 1;
            uiGraphDrawCameraOnOffGraph(cam3);                
            break;
            
        case UI_KEY_UP:
            cam_info[cam1][0] ^= 1;
            uiGraphDrawCameraOnOffGraph(cam1);                
            break;
            
        case UI_KEY_DOWN:
            cam_info[cam2][0] ^= 1;
            uiGraphDrawCameraOnOffGraph(cam2);                
            break;

        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                if(iconflag[UI_MENU_SETIDX_CH1_ON+i] != cam_info[i][0])
                {
                    iconflag[UI_MENU_SETIDX_CH1_ON+i]=cam_info[i][0];    
                    set = 1;
                }
            }
            if (set == 1)
                uiMenuSet_TX_CameraOnOff(iconflag[UI_MENU_SETIDX_CH1_ON],0);
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();        
            uiGraphDrawMenu();
            break;
        default:
            DEBUG_UI("uiGraphDrawCameraOnOff error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawCameraAlarmOnOff(u8 key)
{
    u8 i,index;
    static u8 value[4]={0,0,0,0};
    u16 y_fram[4]= {135,240,345,458};
    u16 x_pos=360;
    u8  err;
    
    index = uiCurrNode->parent->item.NodeData->Action_no; 
    
    DEBUG_UI("uiGraphDraw CameraAlarm Key %d Action_no %d\n",key,index);
        
    switch(key)
    {
        case 0:
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();            
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                value[i] = iconflag[index+i];
                uiGraphDrawJPGImage(OnOff_Image[i+value[i]*4],x_pos,y_fram[i]);
            }
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;

        case UI_KEY_RIGHT:
            value[cam4] ^= 1;
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();            
            uiGraphDrawJPGImage(OnOff_Image[cam4+value[cam4]*4],x_pos,y_fram[cam4]);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;

        case UI_KEY_LEFT:
            value[cam3] ^= 1;
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();            
            uiGraphDrawJPGImage(OnOff_Image[cam3+value[cam3]*4],x_pos,y_fram[cam3]);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;
            
        case UI_KEY_UP:
            value[cam1] ^= 1;
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();            
            uiGraphDrawJPGImage(OnOff_Image[cam1+value[cam1]*4],x_pos,y_fram[cam1]);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;
            
        case UI_KEY_DOWN:
            value[cam2] ^= 1;
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            sysJPEG_enable();            
            uiGraphDrawJPGImage(OnOff_Image[cam2+value[cam2]*4],x_pos,y_fram[cam2]);
            sysJPEG_disable();
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
            break;

        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                iconflag[index+i]=value[i];
                if (index == UI_MENU_SETIDX_CH1_LS_ONOFF)
                    iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+i]=UI_LIGHT_MANUAL_OFF;
                else
                    iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+i]=UI_CAMERA_ALARM_MANUAL_OFF;
            }

            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();

            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                if (index == UI_MENU_SETIDX_CH1_LS_ONOFF)
                    uiMenuAction(UI_MENU_SETIDX_CH1_LS_ONOFF+i);
                else
                    uiMenuAction(UI_MENU_SETIDX_CH1_CA_ONOFF+i);
            }
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();        
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDraw CameraAlarm error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawResolutionGraph(s8 setCursor)
{
    u8 i,value;
    u16 y_pos[4]= {138,242,346,455};
    u16 x_pos[2]={470,686};
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    value = cam_info[setCursor][2];
    
    //DEBUG_GREEN(" Draw Resolution Graph cam %d value %d\n",setCursor,value);

    for(i=0;i<2;i++)
    {
        if (i==value)
            uiGraphDrawJPGImage(Select_Image ,x_pos[i],y_pos[setCursor]);   
        else
            uiGraphDrawJPGImage(Select_Item[setCursor*3+i],x_pos[i],y_pos[setCursor]);//draw empty
    }
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}

void uiGraphDrawResolution(u8 key)
{
    
    u8 i;
    
    DEBUG_UI("uiGraphDrawResolution Key %d\n",key);
    
    switch(key)
    {
        case 0:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                switch(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i])
                {
                    case UI_MENU_SETTING_RESOLUTION_HD:
                        cam_info[i][2] = 0;            
                        break;

                    case UI_MENU_SETTING_RESOLUTION_1920x1088:
                        cam_info[i][2] = 1;            
                        break;
                }
                uiGraphDrawResolutionGraph(i);  
            }
            break;

        case UI_KEY_RIGHT:
            cam_info[cam4][2] = TouchExtKey;  
            uiGraphDrawResolutionGraph(cam4);  
            break;

        case UI_KEY_LEFT:
            cam_info[cam3][2] = TouchExtKey;  
            uiGraphDrawResolutionGraph(cam3);  
            break;

        case UI_KEY_UP:
            
            cam_info[cam1][2] = TouchExtKey;  
            uiGraphDrawResolutionGraph(cam1);  
            break;

        case UI_KEY_DOWN:
            cam_info[cam2][2] = TouchExtKey;  
            uiGraphDrawResolutionGraph(cam2);  
            break;

        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                switch(cam_info[i][2])
                {
                    case 0:
                        cam_info[i][2] = UI_MENU_SETTING_RESOLUTION_HD;            
                        break;

                    case 1:
                        cam_info[i][2] = UI_MENU_SETTING_RESOLUTION_1920x1088;            
                        break;
                }
                
                if(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i] != cam_info[i][2])
                {
                    iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i]=cam_info[i][2];
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i],i);
                    #if UI_BAT_SUPPORT
                    if ((gRfiuUnitCntl[i].RFpara.BateryCam_support == 1) && (sysBatteryCam_isSleeping(i) == TRUE))
                    {
                        sysStartBatteryCam(i);
                    }
                    #endif
                }
            }
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();         
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawResolution error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawBrightnessGraph(s8 setCursor,u8 value)
{
    u8 i;
    u16 x_bar= 334;
    u16 y_bar[4]= {132,242,352,462};
    u16 x_fram[6]= {337,408,478,548,618,688};
    u16 y_fram[4]= {134,244,354,464};
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    DEBUG_UI(" Draw Brightness Graph %d\n",value);
    
    uiGraphDrawJPGImage(Brightness_Bar_Image[setCursor],x_bar,y_bar[setCursor]);
    for(i=0;i<value+1;i++)
        uiGraphDrawJPGImage(Select_Image,x_fram[i],y_fram[setCursor]);
    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);

}

void uiGraphDrawBrightness(u8 key)
{
    
    u8 i;

    //DEBUG_UI("uiGraphDrawBrightness Key %d\n",key);
    
    switch(key)
    {
        case 0:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                cam_info[i][3] = iconflag[UI_MENU_SETIDX_BRIGHTNESS_CH1+i]; 
                uiGraphDrawBrightnessGraph(i,cam_info[i][3]);      
            }
            break;
            
        case UI_KEY_UP:
            cam_info[cam1][3] = TouchExtKey;
            uiGraphDrawBrightnessGraph(cam1,cam_info[cam1][3]);
            break;            
                        
        case UI_KEY_DOWN:
            cam_info[cam2][3] = TouchExtKey;
            uiGraphDrawBrightnessGraph(cam2,cam_info[cam2][3]);
            break;            

        case UI_KEY_LEFT:
            cam_info[cam3][3] = TouchExtKey;
            uiGraphDrawBrightnessGraph(cam3,cam_info[cam3][3]);
            break;            

        case UI_KEY_RIGHT:
            cam_info[cam4][3] = TouchExtKey;
            uiGraphDrawBrightnessGraph(cam4,cam_info[cam4][3]);
            break;            
                                
        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                if(iconflag[UI_MENU_SETIDX_BRIGHTNESS_CH1+i] != cam_info[i][3])
                {
                    iconflag[UI_MENU_SETIDX_BRIGHTNESS_CH1+i]=cam_info[i][3];    
                    uiMenuSet_TX_VideoBrightness(iconflag[UI_MENU_SETIDX_BRIGHTNESS_CH1+i],i);
                }
            }
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;
            
        case UI_KEY_MENU:
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();
            break;
            
        default:
            DEBUG_UI("uiGraphDrawBrightness error key %d\r\n",key);
            break;
           
    }
}

void uiGraphDrawMotionSensitivityGraph(s8 setCursor)
{
    u16 y_pos[4]= {138,242,346,455};
    u16 x_pos[3]={470,686,875};
    u8  value,i;
    u8  err;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    sysJPEG_enable();

    value = cam_info[setCursor][4];
    
    DEBUG_UI(" Draw Sensitivity Graph\n");
    
    for(i=0;i<3;i++)
    {
        if (i==value)
            uiGraphDrawJPGImage(Select_Image ,x_pos[i],y_pos[setCursor]);   
        else
            uiGraphDrawJPGImage(Select_Item[setCursor*3+i],x_pos[i],y_pos[setCursor]);//draw empty
    }

    sysJPEG_disable();
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
    
}

void uiGraphDrawMotionSensitivity(u8 key)
{
    
    u8 i;
    
    DEBUG_UI("uiGraphDrawMotionSensitivity Key %d\n",key);
    
    switch(key)
    {
        case 0:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                switch(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+i])
                {
                    case UI_MENU_SETTING_SENSITIVITY_LOW:
                        cam_info[i][4] = 0;            
                        break;

                    case UI_MENU_SETTING_SENSITIVITY_MID:
                        cam_info[i][4] = 1;            
                        break;
                        
                    case UI_MENU_SETTING_SENSITIVITY_HIGHT:
                        cam_info[i][4] = 2;            
                        break;
                }
                uiGraphDrawMotionSensitivityGraph(i);  
            }
            break;
            
        case UI_KEY_RIGHT:
            cam_info[cam4][4] = TouchExtKey;  
            uiGraphDrawMotionSensitivityGraph(cam4);  
            break;

        case UI_KEY_LEFT:
            cam_info[cam3][4] = TouchExtKey;  
            uiGraphDrawMotionSensitivityGraph(cam3);  
            break;

        case UI_KEY_UP:
            cam_info[cam1][4] = TouchExtKey;  
            uiGraphDrawMotionSensitivityGraph(cam1);  
            break;

        case UI_KEY_DOWN:
            cam_info[cam2][4] = TouchExtKey;  
            uiGraphDrawMotionSensitivityGraph(cam2);  
            break;

        case UI_KEY_ENTER:
            for(i=0; i < MULTI_CHANNEL_MAX;i++)
            {
                switch(cam_info[i][4])
                {
                    case 0:
                        cam_info[i][4] = UI_MENU_SETTING_SENSITIVITY_LOW;            
                        break;

                    case 1:
                        cam_info[i][4] = UI_MENU_SETTING_SENSITIVITY_MID;            
                        break;
                        
                    case 2:
                        cam_info[i][4] = UI_MENU_SETTING_SENSITIVITY_HIGHT;            
                        break;
                }
            
                if(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+i] != cam_info[i][4])
                {
                    iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+i]=cam_info[i][4]; 
                    uiMenuSet_MotionSensitivity(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+i],i);
                }
            }
            Save_UI_Setting();
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            break;
            
        case UI_KEY_MENU:            
            uiFrowGoToLastNode();          
            uiGraphDrawMenu();            
            break;
            
        default:
            DEBUG_UI("uiGraphDrawMotionSensitivity error key %d\r\n",key);
            break;
           
           
    }
}

void uiGraphDrawKeypadGraph()
{
    u8 i,nLen;
    u8 y_pos=148;
    u8 dotindex=0;
    s16 nStrLen=260;
    
    nLen=strlen(gTempString);
    uiGraphGetMenuData();
    for(i=0;i<nLen;i++)
    {
        //nStrLen=154+(i*23);
        switch(gTempString[i])
        {
            case '0':
                uiGraphDrawString("0", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;

            case '1':
                uiGraphDrawString("1", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '2':
                uiGraphDrawString("2", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '3':
                uiGraphDrawString("3", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '4':
                uiGraphDrawString("4", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;

            case '5':
                uiGraphDrawString("5", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;

            case '6':
                uiGraphDrawString("6", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '7':
                uiGraphDrawString("7", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '8':
                uiGraphDrawString("8", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;
                
            case '9':
                uiGraphDrawString("9", nStrLen, y_pos, 0);
                nStrLen+=25;
                break;

            case '.':
                nStrLen+=90+(3-(i-dotindex))*18;
                dotindex = i;
                break;
        }
    }
    
}

void uiGraphDrawKeypad(u8 key)
{
    static s8 setCursor,dotindex=0;
    u8 i,nLen;
    static u8 KeypadX,KeypadY;
    s8 nNumTable[4][3]={{1,4,7},{2,5,8},{3,6,9},{11,0,10}};
    
    DEBUG_UI("uiGraphDrawKeypad key %d\r\n",key);

    switch(key)
    {
        case 0:
            KeypadX=0;
            KeypadY=4;
            setCursor=13;   // cancel
            strcpy(gTempString,"");
            break;
            
        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
                setCursor = TouchExtKey;

            nLen=strlen(gTempString);
            if ((nLen>0) && (setCursor < 10))
            {
                if ((nLen-dotindex)>2)
                {
                    if((nLen)<15)
                        sprintf(gTempString,"%s%s",gTempString,".");
                    nLen=strlen(gTempString);
                    dotindex = nLen;//tag final dot
                }            
            }

            if(setCursor == 12)//ok
            {
                setCursor=0;
                dotindex=0;
                //DEBUG_GREEN("Set IP Address  :%s \n",gTempString);
                uiEnterMenu(UI_MENU_NODE_SET_STATIC);
                uiGraphDrawMenu();
                return;
            }
            else if(setCursor == 10)//.
            {
            DEBUG_GREEN("Set IP Address  :%s dotindex %d\n",gTempString,dotindex);
                nLen=strlen(gTempString);
                if (nLen<1) break;
                
                if (gTempString[nLen-1]=='.') 
                    break;
                
                if(nLen<15)
                    sprintf(gTempString,"%s%s",gTempString,".");
                nLen=strlen(gTempString);
                dotindex = nLen;//tag latest dot
            }
            else if(setCursor == 11)
            {
                nLen=strlen(gTempString);
                if (nLen<1) break;

                if (gTempString[nLen-1]=='.') //clear final dot
                {
                    gTempString[nLen-1]='\0';
                    nLen=strlen(gTempString);
                }
                gTempString[nLen-1]='\0';
                dotindex = 0;
                for (i=0;i<nLen;i++)//tag final dot
                {
                    if (gTempString[i]=='.')  
                        dotindex = i+1;
                }
            }
            else
            {
                nLen=strlen(gTempString);
                if(nLen<15)
                    sprintf(gTempString,"%s%d",gTempString,setCursor);
            }
            uiGraphDrawKeypadGraph();
            //DEBUG_GREEN("Set IP Address  :%s \n",gTempString);
            break;

        case UI_KEY_MENU:
            setCursor=0;
            dotindex=0;
            strcpy(gTempString,"");
            uiEnterMenu(UI_MENU_NODE_SET_STATIC);
            uiGraphDrawMenu();
            break;
            
        case UI_KEY_ZERO:
            //uiGraphDrawSubKeypadGraph(setCursor,1);
            KeypadX=3;
            KeypadY=1;
            setCursor=nNumTable[KeypadX][KeypadY];
            //uiGraphDrawSubKeypadGraph(setCursor,0);
            nLen=strlen(gTempString);
            if(nLen<15)
                sprintf(gTempString,"%s%d",gTempString,setCursor);
            uiGraphDrawKeypadGraph();
            break;

        default:
            DEBUG_UI("uiGraphDrawKeypad error key %d\r\n",key);
            return;
    }
}

#if 0
void uiGraphDrawCamSetGraph(s8 setCursor)
{
    u8 i,j;
    u16 uiJpgWidth, uiJpgHeight;
    DEBUG_UI("uiGraphDrawCamSetGraph\r\n");

    sysSPI_Enable();
    sysJPEG_enable();

    for(i=0;i < MULTI_CHANNEL_MAX;i++)
    {
        if(i==setCursor)
        {
            uiGraphDrawJPGImage(CamSetBt[uiCamSet[i]], 380, 140+(75*i), PKBuf2, PKBuf0);
        }
        else
        {
            uiGraphDrawJPGImage(CamSetBt[uiCamSet[i]+2], 380, 140+(75*i), PKBuf2, PKBuf0);
        }
    }

    sysJPEG_disable();
    sysSPI_Disable();
}
#endif

void uiGraphDrawCamSet(u8 key)
{
#if 0
    static s8 setCursor = 0;
    u8 i;
    DEBUG_UI("uiGraphDrawCamSet\r\n");

    switch(key)
    {
        case 0:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
                uiCamSet[i] = iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i];
            uiGraphDrawCamSetGraph(setCursor);
            break;

        case UI_KEY_UP:
        case UI_KEY_LEFT:
        case UI_KEY_ADC_LEFT:
            if (setCursor == 0)
                setCursor = 3;
            else
                setCursor--;
            uiGraphDrawCamSetGraph(setCursor);
            break;
            
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_ADC_RIGHT:
            if (setCursor == 3)
                setCursor = 0;
            else
                setCursor++;
            uiGraphDrawCamSetGraph(setCursor);
            break;

        case UI_KEY_ADC_ENTER:
        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
            {
                setCursor = TouchExtKey;
            }
            uiCamSet[setCursor] = uiCamSet[setCursor]+1;
            
            if(uiCamSet[setCursor]>=3)
                uiCamSet[setCursor]=0;
            uiGraphDrawCamSetGraph(setCursor);
            break;

        case UI_KEY_ADC_MENU:
        case UI_KEY_MODE:
            for(i=0;i < MULTI_CHANNEL_MAX;i++)
            {
                if(iconflag[UI_MENU_SETIDX_CH1_RES+i] != uiCamSet[i])
                {
                    iconflag[UI_MENU_SETIDX_CH1_RES+i] = uiCamSet[i];
                    uiMenuSet_TX_VideoResolution(iconflag[UI_MENU_SETIDX_CH1_RES+i],i);
                }
            }
            uiCurrNode = uiCurrNode->parent;
            MyHandler.MenuMode = SETUP_MODE;
            uiGraphDrawMenu();
            Save_UI_Setting();
            break;

        default:
            DEBUG_UI("uiGraphDrawCamSet error key %d\r\n",key);
            return;
    }
#endif
}

void uiGraphDrawMenu(void)
{
    UI_MENU_NODE *paretn;
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

    if(uiCurrNode->parent)
        paretn = uiCurrNode->parent;
    else
        paretn = uiCurrNode;
    
    DEBUG_UI("current node %s action %d\r\n",uiCurrNode->item.NodeData->Node_Name, uiCurrNode->item.NodeData->Action_no);
    DEBUG_UI("paretn node %s action %d\r\n",paretn->item.NodeData->Node_Name, paretn->item.NodeData->Action_no);

    switch(paretn->item.NodeData->Action_no)
    {
        /*
        case UI_MENU_SETIDX_MOTION_MASK:
            DEBUG_UI("Enter Mask Area Setting\r\n");
            MyHandler.MenuMode = SET_MASK_AREA;
            uiOsdDrawMaskArea(0);
            break;
        */
        case UI_MENU_SETIDX_DATE_TIME:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawDateTime(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;

#if SET_NTPTIME_TO_RTC
        case UI_MENU_SETIDX_NTP:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawTimezone(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;
#endif

        case UI_MENU_SETIDX_REC_MODE_CH1:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawRECMode(0);  
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        case UI_MENU_SETIDX_RESOLUTION_CH1:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawResolution(0);    
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        case UI_MENU_SETIDX_BRIGHTNESS_CH1:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawBrightness(0); 
            iduSetVideoBuf0Addr(PKBuf0);
            break;  
            
        case UI_MENU_SETIDX_CH1_ON:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawCameraOnOff(0);  
            iduSetVideoBuf0Addr(PKBuf0);
            break;    
            
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawMotionSensitivity(0);  
            iduSetVideoBuf0Addr(PKBuf0);
            break;
        #if (NIC_SUPPORT)
        //case UI_MENU_SETIDX_NETWORK:
            //sysISU_enable();
            //uiGraphGetMenuData();
            //uiGraphDrawNetwork(0); 
            //iduSetVideoBuf0Addr(PKBuf0);
            //break;
            
        case UI_MENU_SETIDX_NETWORK_INFO:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawNetworkInfo(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        case UI_MENU_SETIDX_NETWORK_KEYPAD:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawKeypad(0); 
            iduSetVideoBuf0Addr(PKBuf0);
            break;

        case UI_MENU_SETIDX_ST_IP_SET:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawStaticIP(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;
        #endif    
    
        case UI_MENU_SETIDX_SCHEDULED:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawScheduled(0);  
            iduSetVideoBuf0Addr(PKBuf0);
            break; 

        case UI_MENU_SETIDX_SCHEDULED_SET:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawScheduledSetting(0);  
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        case UI_MENU_SETIDX_CARDINFO:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawCardInfo(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;

        case UI_MENU_SETIDX_VERSION_INFO:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawVersionInfo(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        case UI_MENU_SETIDX_CH1_LS_ONOFF:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawCameraAlarmOnOff(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;

        case UI_MENU_SETIDX_CH1_CA_ONOFF:
            sysISU_enable();
            uiGraphGetMenuData();            
            uiGraphDrawCameraAlarmOnOff(0);
            iduSetVideoBuf0Addr(PKBuf0);
            break;
            
        default:
            sysISU_enable();
            uiGraphGetMenuData();
            IduVidBuf0Addr = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            break;
    }

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("UI Show Menu Time 1 =%d (x50ms)\n",time1);
#endif
}


void uiGraphGetImageID(void)
{
    u8 i;
    u32 num;
    
    for(i=0;i<40;i++)
    {
        TimeNum[i].bufIndex = spiGet_UI_FB_Index(TimeNum[i].FileName);
    }

#if SET_NTPTIME_TO_RTC    
    for(i=0;i<6;i++)
    {
        NTP_Image[i].bufIndex = spiGet_UI_FB_Index(NTP_Image[i].FileName);
    }

    for(i=0;i<3;i++)
    {
        TimeZoneBtn[i].bufIndex = spiGet_UI_FB_Index(TimeZoneBtn[i].FileName);
    }
        
    num = sizeof(NumOp)/sizeof(UI_NODE_PHOTO);
    for(i=0;i < num;i++)
    {
        NumOp[i].bufIndex = spiGet_UI_FB_Index(NumOp[i].FileName);
    }
#endif
        
    num = sizeof(RECMode_Image)/sizeof(UI_NODE_PHOTO);
    for(i=0;i<num;i++)
    {
        RECMode_Image[i].bufIndex = spiGet_UI_FB_Index(RECMode_Image[i].FileName);
    }
        
    num = sizeof(Sensitivity_Image)/sizeof(UI_NODE_PHOTO);
    for(i=0;i<num;i++)
    {
        Sensitivity_Image[i].bufIndex = spiGet_UI_FB_Index(Sensitivity_Image[i].FileName);
    }
        
    num = sizeof(Resolution_Image)/sizeof(UI_NODE_PHOTO);
    for (i=0; i<num; i++)
    {
        Resolution_Image[i].bufIndex = spiGet_UI_FB_Index(Resolution_Image[i].FileName);
    }
        
    num = sizeof(OnOff_Image)/sizeof(UI_NODE_PHOTO);
    for(i=0;i<num;i++)
    {
        OnOff_Image[i].bufIndex = spiGet_UI_FB_Index(OnOff_Image[i].FileName);
    }

    Select_Image.bufIndex = spiGet_UI_FB_Index(Select_Image.FileName);
    
    for(i=0;i<12;i++)
    {
        Select_Item[i].bufIndex = spiGet_UI_FB_Index(Select_Item[i].FileName);
    }
    
    for(i=0;i<2;i++)
    {
        YesNo_Image[i].bufIndex = spiGet_UI_FB_Index(YesNo_Image[i].FileName);
    }
        
    num = sizeof(Brightness_Bar_Image)/sizeof(UI_NODE_PHOTO);
    for(i=0;i<num;i++)
    {
        Brightness_Bar_Image[i].bufIndex = spiGet_UI_FB_Index(Brightness_Bar_Image[i].FileName);
    }

    PBButton.bufIndex = spiGet_UI_FB_Index(PBButton.FileName);
    for(i=0;i<2;i++)
        PBListDayBG[i].bufIndex = spiGet_UI_FB_Index(PBListDayBG[i].FileName);
    
    for (i=0; i < 8; i++)
    {
        PlaybackList_Cam[i].bufIndex = spiGet_UI_FB_Index(PlaybackList_Cam[i].FileName);
        PlaybackList_RecType[i].bufIndex = spiGet_UI_FB_Index(PlaybackList_RecType[i].FileName);
    }
    
    
    /* Network */
    for (i=0; i < 3; i++)
    {
        Dynmic_OFF[i].bufIndex = spiGet_UI_FB_Index(Dynmic_OFF[i].FileName);
        Dynmic_ON[i].bufIndex = spiGet_UI_FB_Index(Dynmic_ON[i].FileName);
        Static_BG[i].bufIndex = spiGet_UI_FB_Index(Static_BG[i].FileName);
        Txt_Address_1_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Address_1_M[i].FileName);
        Txt_Address_2_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Address_2_M[i].FileName);
        Txt_Mask_1_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Mask_1_M[i].FileName);
        Txt_Mask_2_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Mask_2_M[i].FileName);
        Txt_Gateway_1_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Gateway_1_M[i].FileName);
        Txt_Gateway_2_M[i].bufIndex = spiGet_UI_FB_Index(Txt_Gateway_2_M[i].FileName);
        Txt_DNS1_1_M[i].bufIndex = spiGet_UI_FB_Index(Txt_DNS1_1_M[i].FileName);
        Txt_DNS1_2_M[i].bufIndex = spiGet_UI_FB_Index(Txt_DNS1_2_M[i].FileName);
        Txt_DNS2_1_M[i].bufIndex = spiGet_UI_FB_Index(Txt_DNS2_1_M[i].FileName);
        Txt_DNS2_2_M[i].bufIndex = spiGet_UI_FB_Index(Txt_DNS2_2_M[i].FileName);
        BTN_OK_1_M[i].bufIndex = spiGet_UI_FB_Index(BTN_OK_1_M[i].FileName);
        BTN_OK_2_M[i].bufIndex = spiGet_UI_FB_Index(BTN_OK_2_M[i].FileName);
        busy_ON[i].bufIndex = spiGet_UI_FB_Index(busy_ON[i].FileName);
    }

    /*Scheduled Table*/
    num = sizeof(ScheduledTitle_Image)/sizeof(UI_NODE_PHOTO);
    for( i=0;i<num;i++)
    {
        ScheduledTitle_Image[i].bufIndex = spiGet_UI_FB_Index(ScheduledTitle_Image[i].FileName);    
    }

    Right_1_M.bufIndex = spiGet_UI_FB_Index(Right_1_M.FileName);
    Right_2_M.bufIndex = spiGet_UI_FB_Index(Right_2_M.FileName);
    Left_1_M.bufIndex = spiGet_UI_FB_Index(Left_1_M.FileName);
    Left_2_M.bufIndex = spiGet_UI_FB_Index(Left_2_M.FileName);
    grey_item.bufIndex = spiGet_UI_FB_Index(grey_item.FileName);
    yellow_item.bufIndex = spiGet_UI_FB_Index(yellow_item.FileName);
    schbar.bufIndex = spiGet_UI_FB_Index(schbar.FileName);

    for (i=0; i < 3; i++)
    {
        Scheduled_Table_BG[i].bufIndex = spiGet_UI_FB_Index(Scheduled_Table_BG[i].FileName);
        Modify_1_M[i].bufIndex = spiGet_UI_FB_Index(Modify_1_M[i].FileName);
        Modify_2_M[i].bufIndex = spiGet_UI_FB_Index(Modify_2_M[i].FileName);
    }
    Keypad_1_BG.bufIndex = spiGet_UI_FB_Index(Keypad_1_BG.FileName);
    Dot.bufIndex = spiGet_UI_FB_Index(Dot.FileName);

    /*Scheduled Setting */
    for (i=0; i < 3; i++)
    {
        Set_1_M[i].bufIndex = spiGet_UI_FB_Index(Set_1_M[i].FileName);
        Set_2_M[i].bufIndex = spiGet_UI_FB_Index(Set_2_M[i].FileName);
        Delete_1_M[i].bufIndex = spiGet_UI_FB_Index(Delete_1_M[i].FileName);
        Delete_2_M[i].bufIndex = spiGet_UI_FB_Index(Delete_2_M[i].FileName);
    }
    for(i=0;i<24;i++)
    {
        Camera_Btn[i].bufIndex = spiGet_UI_FB_Index(Camera_Btn[i].FileName);    
        Cam_Image[i].bufIndex = spiGet_UI_FB_Index(Cam_Image[i].FileName);
    }
        
    num = sizeof(Day_Btn)/sizeof(UI_NODE_PHOTO);
    for(i=0;i<num;i++)
    {
        Day_Btn[i].bufIndex = spiGet_UI_FB_Index(Day_Btn[i].FileName);        
    }
    Edit_bar.bufIndex = spiGet_UI_FB_Index(Edit_bar.FileName);

    num = sizeof(ASCII_PHOTO)/sizeof(UI_NODE_PHOTO);
    for (i=0; i<num; i++)
    {
        ASCII_PHOTO[i].bufIndex = spiGet_UI_FB_Index(ASCII_PHOTO[i].FileName);
    }
    AsciiNum = num/2;
         
    num = sizeof(LoadingLevel)/sizeof(UI_NODE_PHOTO);
    for (i=0; i < num; i++)
    {
        LoadingLevel[i].bufIndex = spiGet_UI_FB_Index(LoadingLevel[i].FileName);
    }
        
}



/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */


