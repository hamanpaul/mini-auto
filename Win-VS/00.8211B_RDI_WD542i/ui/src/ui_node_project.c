#include <osapi.h>
#include <general.h>
#include "ui.h"
#include "Uiapi.h"
#include "ui_project.h"

/*define all file size for ui menu*/
#define UI_MENU_SIZE_TITLE_X        276
#define UI_MENU_SIZE_TITLE_Y        38
#define UI_MENU_SIZE_NODE_X         136
#define UI_MENU_SIZE_NODE_Y         24
#define UI_MENU_SIZE_CARD_INFO_X    120
#define UI_MENU_SIZE_CARD_INFO_Y    66

#define UI_MENU_SIZE_BG_X           320
#define UI_MENU_SIZE_BG_Y           240
#define UI_MENU_SIZE_BG1_X          640
#define UI_MENU_SIZE_BG1_Y          480

/*define all file location for ui menu*/
#define UI_MENU_LOC_BG_X            0
#define UI_MENU_LOC_BG_Y            0
#define UI_MENU_LOC_TITLE_X         30
#define UI_MENU_LOC_TITLE_Y         8
#define UI_MENU_LOC_SUB_PARENT_X    28
#define UI_MENU_LOC_CARD_INFO_X     150
#define UI_MENU_LOC_CARD_INFO_Y     200

#define UI_MENU_LOC_PARENT          2
#define UI_MENU_LOC_PARENT_X        28
#define UI_MENU_LOC_PARENT_Y        80
#define UI_MENU_LOC_TITLE1_X        184
#define UI_MENU_LOC_TITLE1_Y        48
#define UI_MENU_LOC_SUB_TITLE_X     200
#define UI_MENU_LOC_SUB_TITLE_Y     420

#define UI_MENU_LOC_NODE1_X         160
#define UI_MENU_LOC_NODE2_X         308
#define UI_MENU_LOC_NODE3_X         452
#define UI_MENU_LOC_NODE4_X         UI_MENU_LOC_NODE1_X
#define UI_MENU_LOC_NODE5_X         UI_MENU_LOC_NODE2_X
#define UI_MENU_LOC_NODE6_X         UI_MENU_LOC_NODE3_X
#define UI_MENU_LOC_NODE11_Y         120
#define UI_MENU_LOC_NODE12_Y         UI_MENU_LOC_NODE11_Y
#define UI_MENU_LOC_NODE13_Y         UI_MENU_LOC_NODE11_Y
#define UI_MENU_LOC_NODE14_Y         264
#define UI_MENU_LOC_NODE15_Y         UI_MENU_LOC_NODE14_Y
#define UI_MENU_LOC_NODE16_Y         UI_MENU_LOC_NODE14_Y

#define UI_MENU_OSD_LOC_NODE1_X         4
#define UI_MENU_OSD_LOC_NODE2_X         56
#define UI_MENU_OSD_LOC_NODE3_X         108
#define UI_MENU_OSD_LOC_NODE4_X         160
#define UI_MENU_OSD_LOC_NODE5_X         212
#define UI_MENU_OSD_LOC_NODE6_X         264
#define UI_MENU_OSD_LOC_NODE1_Y         180
#define UI_MENU_OSD_LOC_NODE2_Y         UI_MENU_OSD_LOC_NODE1_Y
#define UI_MENU_OSD_LOC_NODE3_Y         UI_MENU_OSD_LOC_NODE1_Y
#define UI_MENU_OSD_LOC_NODE4_Y         UI_MENU_OSD_LOC_NODE1_Y
#define UI_MENU_OSD_LOC_NODE5_Y         UI_MENU_OSD_LOC_NODE1_Y
#define UI_MENU_OSD_LOC_NODE6_Y         UI_MENU_OSD_LOC_NODE1_Y
#define UI_MENU_OSD_LOC_SUB_NODE1_Y     (UI_MENU_OSD_LOC_NODE1_Y - 52)
#define UI_MENU_OSD_LOC_SUB_NODE2_Y     UI_MENU_OSD_LOC_SUB_NODE1_Y
#define UI_MENU_OSD_LOC_SUB_NODE3_Y     UI_MENU_OSD_LOC_SUB_NODE1_Y
#define UI_MENU_OSD_LOC_SUB_NODE4_Y     UI_MENU_OSD_LOC_SUB_NODE1_Y

#define DataNum(node)                   sizeof(node)/sizeof(UI_NODE_FILE)
#define OsdNum(node)                   sizeof(node)/sizeof(UI_NODE_OSD)

/* Graphic Menu */
UI_NODE_PHOTO Background[UI_MULT_LANU_END] =
{
    {"bg.jpg",0},
    {"bg.jpg",0},
    {"bg.jpg",0}
};

UI_NODE_PHOTO FromTo[UI_MULT_LANU_END] =
{
    {"ft.jpg",0},
    {"ft.jpg",0},
    {"ft.jpg",0}
};

UI_NODE_PHOTO CardInfo[UI_MULT_LANU_END] =
{
    {"ci_1.jpg",0},
    {"ci_1.jpg",0},
    {"ci_1.jpg",0}
};

UI_NODE_PHOTO Network[UI_MULT_LANU_END] =
{
    {"net_1.jpg",0},
    {"net_1.jpg",0},
    {"net_1.jpg",0}
};

/*Title*/
UI_NODE_PHOTO Image_T[UI_MULT_LANU_END] =
{
    {"img_t1.jpg",0},
    {"img_t1.jpg",0},
    {"img_t1.jpg",0}
};

UI_NODE_PHOTO RECSet_T[UI_MULT_LANU_END] =
{
    {"rec_t1.jpg",0},
    {"rec_t1.jpg",0},
    {"rec_t1.jpg",0}
};

UI_NODE_PHOTO RECMode_T[UI_MULT_LANU_END] =
{
    {"rem_t1.jpg",0},
    {"rem_t1.jpg",0},
    {"rem_t1.jpg",0}
};

UI_NODE_PHOTO System_T[UI_MULT_LANU_END] =
{
    {"sys_t1.jpg",0},
    {"sys_t1.jpg",0},
    {"sys_t1.jpg",0}
};

UI_NODE_PHOTO Audio_T[UI_MULT_LANU_END] =
{
    {"aud_t1.jpg",0},
    {"aud_t1.jpg",0},
    {"aud_t1.jpg",0}
};

UI_NODE_PHOTO Cam_T[UI_MULT_LANU_END] =
{
    {"cam_t1.jpg",0},
    {"cam_t1.jpg",0},
    {"cam_t1.jpg",0}
};

UI_NODE_PHOTO Pair_T[UI_MULT_LANU_END] =
{
    {"pair_t1.jpg",0},
    {"pair_t1.jpg",0},
    {"pair_t1.jpg",0}
};

UI_NODE_PHOTO CamOnOff_T[UI_MULT_LANU_END] =
{
    {"caof_t1.jpg",0},
    {"caof_t1.jpg",0},
    {"caof_t1.jpg",0}
};

UI_NODE_PHOTO Cam1_T[UI_MULT_LANU_END] =
{
    {"Cam1_t1.jpg",0},
    {"Cam1_t1.jpg",0},
    {"Cam1_t1.jpg",0}
};

UI_NODE_PHOTO Cam2_T[UI_MULT_LANU_END] =
{
    {"Cam2_t1.jpg",0},
    {"Cam2_t1.jpg",0},
    {"Cam2_t1.jpg",0}
};

UI_NODE_PHOTO Cam3_T[UI_MULT_LANU_END] =
{
    {"Cam3_t1.jpg",0},
    {"Cam3_t1.jpg",0},
    {"Cam3_t1.jpg",0}
};

UI_NODE_PHOTO Cam4_T[UI_MULT_LANU_END] =
{
    {"Cam4_t1.jpg",0},
    {"Cam4_t1.jpg",0},
    {"Cam4_t1.jpg",0}
};

UI_NODE_PHOTO Quality_T[UI_MULT_LANU_END] =
{
    {"qua_t1.jpg",0},
    {"qua_t1.jpg",0},
    {"qua_t1.jpg",0}
};

UI_NODE_PHOTO FrameRate_T[UI_MULT_LANU_END] =
{
    {"far_t1.jpg",0},
    {"far_t1.jpg",0},
    {"far_t1.jpg",0}
};


UI_NODE_PHOTO Resolution_T[UI_MULT_LANU_END] =
{
    {"res_t1.jpg",0},
    {"res_t1.jpg",0},
    {"res_t1.jpg",0}
};


UI_NODE_PHOTO Resolution_CH1_T[UI_MULT_LANU_END] =
{
    {"rec_ch1.jpg",0},
    {"rec_ch1.jpg",0},
    {"rec_ch1.jpg",0}
};

UI_NODE_PHOTO Resolution_CH2_T[UI_MULT_LANU_END] =
{
    {"rec_ch2.jpg",0},
    {"rec_ch2.jpg",0},
    {"rec_ch2.jpg",0}
};


UI_NODE_PHOTO Overwrite_T[UI_MULT_LANU_END] =
{
    {"ovr_t1.jpg",0},
    {"ovr_t1.jpg",0},
    {"ovr_t1.jpg",0}
};

UI_NODE_PHOTO Section_T[UI_MULT_LANU_END] =
{
    {"sec_t1.jpg",0},
    {"sec_t1.jpg",0},
    {"sec_t1.jpg",0}
};

UI_NODE_PHOTO Motion_T[UI_MULT_LANU_END] =
{
    {"mot_t1.jpg",0},
    {"mot_t1.jpg",0},
    {"mot_t1.jpg",0}
};

UI_NODE_PHOTO Channel_T[UI_MULT_LANU_END] =
{
    {"cha_t1.jpg",0},
    {"cha_t1.jpg",0},
    {"cha_t1.jpg",0}
};

UI_NODE_PHOTO Manual_T[UI_MULT_LANU_END] =
{
    {"man_t1.jpg",0},
    {"man_t1.jpg",0},
    {"man_t1.jpg",0}
};

UI_NODE_PHOTO Delete_T[UI_MULT_LANU_END] =
{
    {"del_t1.jpg",0},
    {"del_t1.jpg",0},
    {"del_t1.jpg",0}
};

UI_NODE_PHOTO Time_T[UI_MULT_LANU_END] =
{
    {"time_t1.jpg",0},
    {"time_t1.jpg",0},
    {"time_t1.jpg",0}
};

UI_NODE_PHOTO Format_T[UI_MULT_LANU_END] =
{
    {"for_t1.jpg",0},
    {"for_t1.jpg",0},
    {"for_t1.jpg",0}
};

UI_NODE_PHOTO CardInfo_T[UI_MULT_LANU_END] =
{
    {"car_t1.jpg",0},
    {"car_t1.jpg",0},
    {"car_t1.jpg",0}
};

UI_NODE_PHOTO Network_T[UI_MULT_LANU_END] =
{
    {"net_t1.jpg",0},
    {"net_t1.jpg",0},
    {"net_t1.jpg",0}
};

UI_NODE_PHOTO Display_T[UI_MULT_LANU_END] =
{
    {"dis_t1.jpg",0},
    {"dis_t1.jpg",0},
    {"dis_t1.jpg",0}
};

UI_NODE_PHOTO AudRes_T[UI_MULT_LANU_END] =
{
    {"aur_t1.jpg",0},
    {"aur_t1.jpg",0},
    {"aur_t1.jpg",0}
};

UI_NODE_PHOTO Sampling_T[UI_MULT_LANU_END] =
{
    {"sam_t1.jpg",0},
    {"sam_t1.jpg",0},
    {"sam_t1.jpg",0}
};

UI_NODE_PHOTO H_T[UI_MULT_LANU_END] =
{
    {"h_t1.jpg",0},
    {"h_t1.jpg",0},
    {"h_t1.jpg",0}
};

UI_NODE_PHOTO M_T[UI_MULT_LANU_END] =
{
    {"m_t1.jpg",0},
    {"m_t1.jpg",0},
    {"m_t1.jpg",0}
};

UI_NODE_PHOTO L_T[UI_MULT_LANU_END] =
{
    {"l_t1.jpg",0},
    {"l_t1.jpg",0},
    {"l_t1.jpg",0}
};

UI_NODE_PHOTO FrameRate30_T[UI_MULT_LANU_END] =
{
    {"fp30_t1.jpg",0},
    {"fp30_t1.jpg",0},
    {"fp30_t1.jpg",0}
};

UI_NODE_PHOTO FrameRate15_T[UI_MULT_LANU_END] =
{
    {"fp15_t1.jpg",0},
    {"fp15_t1.jpg",0},
    {"fp15_t1.jpg",0}
};

UI_NODE_PHOTO FrameRate5_T[UI_MULT_LANU_END] =
{
    {"fp5_t1.jpg",0},
    {"fp5_t1.jpg",0},
    {"fp5_t1.jpg",0}
};

UI_NODE_PHOTO FrameRate10_T[UI_MULT_LANU_END] =
{
    {"fp10_t1.jpg",0},
    {"fp10_t1.jpg",0},
    {"fp10_t1.jpg",0}
};

UI_NODE_PHOTO HD_T[UI_MULT_LANU_END] =
{
    {"hd_t1.jpg",0},
    {"hd_t1.jpg",0},
    {"hd_t1.jpg",0}
};

UI_NODE_PHOTO D1_T[UI_MULT_LANU_END] =
{
    {"d1_t1.jpg",0},
    {"d1_t1.jpg",0},
    {"d1_t1.jpg",0}
};

UI_NODE_PHOTO VGA_T[UI_MULT_LANU_END] =
{
    {"vga_t1.jpg",0},
    {"vga_t1.jpg",0},
    {"vga_t1.jpg",0}
};

UI_NODE_PHOTO QVGA_T[UI_MULT_LANU_END] =
{
    {"qvg_t1.jpg",0},
    {"qvg_t1.jpg",0},
    {"qvg_t1.jpg",0}
};

UI_NODE_PHOTO Yes_T[UI_MULT_LANU_END] =
{
    {"yes_t1.jpg",0},
    {"yes_t1.jpg",0},
    {"yes_t1.jpg",0}
};

UI_NODE_PHOTO No_T[UI_MULT_LANU_END] =
{
    {"no_t1.jpg",0},
    {"no_t1.jpg",0},
    {"no_t1.jpg",0}
};

UI_NODE_PHOTO On_T[UI_MULT_LANU_END] =
{
    {"on_t1.jpg",0},
    {"on_t1.jpg",0},
    {"on_t1.jpg",0}
};

UI_NODE_PHOTO Off_T[UI_MULT_LANU_END] =
{
    {"off_t1.jpg",0},
    {"off_t1.jpg",0},
    {"off_t1.jpg",0}
};

UI_NODE_PHOTO sec20_T[UI_MULT_LANU_END] =
{
    {"s20_t1.jpg",0},
    {"s20_t1.jpg",0},
    {"s20_t1.jpg",0}
};

UI_NODE_PHOTO min1_T[UI_MULT_LANU_END] =
{
    {"m1_t1.jpg",0},
    {"m1_t1.jpg",0},
    {"m1_t1.jpg",0}
};

UI_NODE_PHOTO min5_T[UI_MULT_LANU_END] =
{
    {"m5_t1.jpg",0},
    {"m5_t1.jpg",0},
    {"m5_t1.jpg",0}
};

UI_NODE_PHOTO min10_T[UI_MULT_LANU_END] =
{
    {"m10_t1.jpg",0},
    {"m10_t1.jpg",0},
    {"m10_t1.jpg",0}
};

UI_NODE_PHOTO min15_T[UI_MULT_LANU_END] =
{
    {"m15_t1.jpg",0},
    {"m15_t1.jpg",0},
    {"m15_t1.jpg",0}
};

UI_NODE_PHOTO min30_T[UI_MULT_LANU_END] =
{
    {"m30_t1.jpg",0},
    {"m30_t1.jpg",0},
    {"m30_t1.jpg",0}
};

UI_NODE_PHOTO hr1_T[UI_MULT_LANU_END] =
{
    {"hr1_t1.jpg",0},
    {"hr1_t1.jpg",0},
    {"hr1_t1.jpg",0}
};

UI_NODE_PHOTO MaskArea_T[UI_MULT_LANU_END] =
{
    {"mas_t1.jpg",0},
    {"mas_t1.jpg",0},
    {"mas_t1.jpg",0}
};

UI_NODE_PHOTO Sensitivity_T[UI_MULT_LANU_END] =
{
    {"sen_t1.jpg",0},
    {"sen_t1.jpg",0},
    {"sen_t1.jpg",0}
};

UI_NODE_PHOTO Channel1_T[UI_MULT_LANU_END] =
{
    {"cha1_t1.jpg",0},
    {"cha1_t1.jpg",0},
    {"cha1_t1.jpg",0}
};

UI_NODE_PHOTO Channel2_T[UI_MULT_LANU_END] =
{
    {"cha2_t1.jpg",0},
    {"cha2_t1.jpg",0},
    {"cha2_t1.jpg",0}
};

UI_NODE_PHOTO TV_T[UI_MULT_LANU_END] =
{
    {"tv_t1.jpg",0},
    {"tv_t1.jpg",0},
    {"tv_t1.jpg",0}
};

UI_NODE_PHOTO Panel_T[UI_MULT_LANU_END] =
{
    {"pan_t1.jpg",0},
    {"pan_t1.jpg",0},
    {"pan_t1.jpg",0}
};

UI_NODE_PHOTO Bit8_T[UI_MULT_LANU_END] =
{
    {"bi8_t1.jpg",0},
    {"bi8_t1.jpg",0},
    {"bi8_t1.jpg",0}
};

UI_NODE_PHOTO Bit16_T[UI_MULT_LANU_END] =
{
    {"bi16_t1.jpg",0},
    {"bi16_t1.jpg",0},
    {"bi16_t1.jpg",0}
};

UI_NODE_PHOTO K8_T[UI_MULT_LANU_END] =
{
    {"k8_t1.jpg",0},
    {"k8_t1.jpg",0},
    {"k8_t1.jpg",0}
};

UI_NODE_PHOTO K16_T[UI_MULT_LANU_END] =
{
    {"k16_t1.jpg",0},
    {"k16_t1.jpg",0},
    {"k16_t1.jpg",0}
};

UI_NODE_PHOTO K32_T[UI_MULT_LANU_END] =
{
    {"k32_t1.jpg",0},
    {"k32_t1.jpg",0},
    {"k32_t1.jpg",0}
};

UI_NODE_PHOTO K44_T[UI_MULT_LANU_END] =
{
    {"k44_t1.jpg",0},
    {"k44_t1.jpg",0},
    {"k44_t1.jpg",0}
};

UI_NODE_PHOTO K48_T[UI_MULT_LANU_END] =
{
    {"k48_t1.jpg",0},
    {"k48_t1.jpg",0},
    {"k48_t1.jpg",0}
};

/*Menu*/
UI_NODE_PHOTO Image_1_M[UI_MULT_LANU_END] =
{
    {"ima1_m1.jpg",0},
    {"ima1_m1.jpg",0},
    {"ima1_m1.jpg",0}
};

UI_NODE_PHOTO Image_2_M[UI_MULT_LANU_END] =
{
    {"ima2_m1.jpg",0},
    {"ima2_m1.jpg",0},
    {"ima2_m1.jpg",0}
};

UI_NODE_PHOTO RECSet_1_M[UI_MULT_LANU_END] =
{
    {"rec1_m1.jpg",0},
    {"rec1_m1.jpg",0},
    {"rec1_m1.jpg",0}
};

UI_NODE_PHOTO RECSet_2_M[UI_MULT_LANU_END] =
{
    {"rec2_m1.jpg",0},
    {"rec2_m1.jpg",0},
    {"rec2_m1.jpg",0}
};

UI_NODE_PHOTO RECMode_1_M[UI_MULT_LANU_END] =
{
    {"rem1_m1.jpg",0},
    {"rem1_m1.jpg",0},
    {"rem1_m1.jpg",0}
};

UI_NODE_PHOTO RECMode_2_M[UI_MULT_LANU_END] =
{
    {"rem2_m1.jpg",0},
    {"rem2_m1.jpg",0},
    {"rem2_m1.jpg",0}
};

UI_NODE_PHOTO System_1_M[UI_MULT_LANU_END] =
{
    {"sys1_m1.jpg",0},
    {"sys1_m1.jpg",0},
    {"sys1_m1.jpg",0}
};

UI_NODE_PHOTO System_2_M[UI_MULT_LANU_END] =
{
    {"sys2_m1.jpg",0},
    {"sys2_m1.jpg",0},
    {"sys2_m1.jpg",0}
};

UI_NODE_PHOTO Audio_1_M[UI_MULT_LANU_END] =
{
    {"aud1_m1.jpg",0},
    {"aud1_m1.jpg",0},
    {"aud1_m1.jpg",0}
};

UI_NODE_PHOTO Audio_2_M[UI_MULT_LANU_END] =
{
    {"aud2_m1.jpg",0},
    {"aud2_m1.jpg",0},
    {"aud2_m1.jpg",0}
};

UI_NODE_PHOTO CamM_1_M[UI_MULT_LANU_END] =
{
    {"cam1_m1.jpg",0},
    {"cam1_m1.jpg",0},
    {"cam1_m1.jpg",0}
};

UI_NODE_PHOTO CamM_2_M[UI_MULT_LANU_END] =
{
    {"cam2_m1.jpg",0},
    {"cam2_m1.jpg",0},
    {"cam2_m1.jpg",0}
};

UI_NODE_PHOTO Pair_1_M[UI_MULT_LANU_END] =
{
    {"pair1_m1.jpg",0},
    {"pair1_m1.jpg",0},
    {"pair1_m1.jpg",0}
};

UI_NODE_PHOTO Pair_2_M[UI_MULT_LANU_END] =
{
    {"pair2_m1.jpg",0},
    {"pair2_m1.jpg",0},
    {"pair2_m1.jpg",0}
};

UI_NODE_PHOTO CamOnOff_1_M[UI_MULT_LANU_END] =
{
    {"pair1_m1.jpg",0},
    {"pair1_m1.jpg",0},
    {"pair1_m1.jpg",0}
};

UI_NODE_PHOTO CamOnOff_2_M[UI_MULT_LANU_END] =
{
    {"pair2_m1.jpg",0},
    {"pair2_m1.jpg",0},
    {"pair2_m1.jpg",0}
};

UI_NODE_PHOTO Quality_1_M[UI_MULT_LANU_END] =
{
    {"qua1_m1.jpg",0},
    {"qua1_m1.jpg",0},
    {"qua1_m1.jpg",0}
};

UI_NODE_PHOTO Quality_2_M[UI_MULT_LANU_END] =
{
    {"qua2_m1.jpg",0},
    {"qua2_m1.jpg",0},
    {"qua2_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate_1_M[UI_MULT_LANU_END] =
{
    {"far1_m1.jpg",0},
    {"far1_m1.jpg",0},
    {"far1_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate_2_M[UI_MULT_LANU_END] =
{
    {"far2_m1.jpg",0},
    {"far2_m1.jpg",0},
    {"far2_m1.jpg",0}
};

UI_NODE_PHOTO Resolution_1_M[UI_MULT_LANU_END] =
{
    {"res1_m1.jpg",0},
    {"res1_m1.jpg",0},
    {"res1_m1.jpg",0}
};

UI_NODE_PHOTO Resolution_2_M[UI_MULT_LANU_END] =
{
    {"res2_m1.jpg",0},
    {"res2_m1.jpg",0},
    {"res2_m1.jpg",0}
};

UI_NODE_PHOTO Overwrite_1_M[UI_MULT_LANU_END] =
{
    {"ove1_m1.jpg",0},
    {"ove1_m1.jpg",0},
    {"ove1_m1.jpg",0}
};

UI_NODE_PHOTO Overwrite_2_M[UI_MULT_LANU_END] =
{
    {"ove2_m1.jpg",0},
    {"ove2_m1.jpg",0},
    {"ove2_m1.jpg",0}
};

UI_NODE_PHOTO Section_1_M[UI_MULT_LANU_END] =
{
    {"sec1_m1.jpg",0},
    {"sec1_m1.jpg",0},
    {"sec1_m1.jpg",0}
};

UI_NODE_PHOTO Section_2_M[UI_MULT_LANU_END] =
{
    {"sec2_m1.jpg",0},
    {"sec2_m1.jpg",0},
    {"sec2_m1.jpg",0}
};

UI_NODE_PHOTO Motion_1_M[UI_MULT_LANU_END] =
{
    {"mot1_m1.jpg",0},
    {"mot1_m1.jpg",0},
    {"mot1_m1.jpg",0}
};

UI_NODE_PHOTO Motion_2_M[UI_MULT_LANU_END] =
{
    {"mot2_m1.jpg",0},
    {"mot2_m1.jpg",0},
    {"mot2_m1.jpg",0}
};

UI_NODE_PHOTO Channel_1_M[UI_MULT_LANU_END] =
{
    {"cha1_m1.jpg",0},
    {"cha1_m1.jpg",0},
    {"cha1_m1.jpg",0}
};

UI_NODE_PHOTO Channel_2_M[UI_MULT_LANU_END] =
{
    {"cha2_m1.jpg",0},
    {"cha2_m1.jpg",0},
    {"cha2_m1.jpg",0}
};

UI_NODE_PHOTO Time_1_M[UI_MULT_LANU_END] =
{
    {"tim1_m1.jpg",0},
    {"tim1_m1.jpg",0},
    {"tim1_m1.jpg",0}
};

UI_NODE_PHOTO Time_2_M[UI_MULT_LANU_END] =
{
    {"tim2_m1.jpg",0},
    {"tim2_m1.jpg",0},
    {"tim2_m1.jpg",0}
};

UI_NODE_PHOTO Format_1_M[UI_MULT_LANU_END] =
{
    {"for1_m1.jpg",0},
    {"for1_m1.jpg",0},
    {"for1_m1.jpg",0}
};

UI_NODE_PHOTO Format_2_M[UI_MULT_LANU_END] =
{
    {"for2_m1.jpg",0},
    {"for2_m1.jpg",0},
    {"for2_m1.jpg",0}
};

UI_NODE_PHOTO CareInfo_1_M[UI_MULT_LANU_END] =
{
    {"car1_m1.jpg",0},
    {"car1_m1.jpg",0},
    {"car1_m1.jpg",0}
};

UI_NODE_PHOTO CareInfo_2_M[UI_MULT_LANU_END] =
{
    {"car2_m1.jpg",0},
    {"car2_m1.jpg",0},
    {"car2_m1.jpg",0}
};

UI_NODE_PHOTO Network_1_M[UI_MULT_LANU_END] =
{
    {"net1_m1.jpg",0},
    {"net1_m1.jpg",0},
    {"net1_m1.jpg",0}
};

UI_NODE_PHOTO Network_2_M[UI_MULT_LANU_END] =
{
    {"net2_m1.jpg",0},
    {"net2_m1.jpg",0},
    {"net2_m1.jpg",0}
};

UI_NODE_PHOTO Display_1_M[UI_MULT_LANU_END] =
{
    {"dis1_m1.jpg",0},
    {"dis1_m1.jpg",0},
    {"dis1_m1.jpg",0}
};

UI_NODE_PHOTO Display_2_M[UI_MULT_LANU_END] =
{
    {"dis2_m1.jpg",0},
    {"dis2_m1.jpg",0},
    {"dis2_m1.jpg",0}
};

UI_NODE_PHOTO Cam_1_M[UI_MULT_LANU_END] =
{
    {"cam1_m1.jpg",0},
    {"cam1_m1.jpg",0},
    {"cam1_m1.jpg",0}
};

UI_NODE_PHOTO Cam_2_M[UI_MULT_LANU_END] =
{
    {"cam2_m1.jpg",0},
    {"cam2_m1.jpg",0},
    {"cam2_m1.jpg",0}
};

UI_NODE_PHOTO AudRes_1_M[UI_MULT_LANU_END] =
{
    {"aur1_m1.jpg",0},
    {"aur1_m1.jpg",0},
    {"aur1_m1.jpg",0}
};

UI_NODE_PHOTO AudRes_2_M[UI_MULT_LANU_END] =
{
    {"aur2_m1.jpg",0},
    {"aur2_m1.jpg",0},
    {"aur2_m1.jpg",0}
};

UI_NODE_PHOTO Sampling_1_M[UI_MULT_LANU_END] =
{
    {"sam1_m1.jpg",0},
    {"sam1_m1.jpg",0},
    {"sam1_m1.jpg",0}
};

UI_NODE_PHOTO Sampling_2_M[UI_MULT_LANU_END] =
{
    {"sam2_m1.jpg",0},
    {"sam2_m1.jpg",0},
    {"sam2_m1.jpg",0}
};

UI_NODE_PHOTO MaskArea_1_M[UI_MULT_LANU_END] =
{
    {"mas1_m1.jpg",0},
    {"mas1_m1.jpg",0},
    {"mas1_m1.jpg",0}
};

UI_NODE_PHOTO MaskArea_2_M[UI_MULT_LANU_END] =
{
    {"mas2_m1.jpg",0},
    {"mas2_m1.jpg",0},
    {"mas2_m1.jpg",0}
};

UI_NODE_PHOTO Sensitivity_1_M[UI_MULT_LANU_END] =
{
    {"sen1_m1.jpg",0},
    {"sen1_m1.jpg",0},
    {"sen1_m1.jpg",0}
};

UI_NODE_PHOTO Sensitivity_2_M[UI_MULT_LANU_END] =
{
    {"sen2_m1.jpg",0},
    {"sen2_m1.jpg",0},
    {"sen2_m1.jpg",0}
};

UI_NODE_PHOTO Delete_1_M[UI_MULT_LANU_END] =
{
    {"del1_m1.jpg",0},
    {"del1_m1.jpg",0},
    {"del1_m1.jpg",0}
};

/*sub menu*/
UI_NODE_PHOTO Manual_1_S[UI_MULT_LANU_END] =
{
    {"man1_m1.jpg",0},
    {"man1_m1.jpg",0},
    {"man1_m1.jpg",0}
};

UI_NODE_PHOTO Manual_2_S[UI_MULT_LANU_END] =
{
    {"man2_m1.jpg",0},
    {"man2_m1.jpg",0},
    {"man2_m1.jpg",0}
};

UI_NODE_PHOTO Motion_1_S[UI_MULT_LANU_END] =
{
    {"mti1_m1.jpg",0},
    {"mti1_m1.jpg",0},
    {"mti1_m1.jpg",0}
};

UI_NODE_PHOTO Motion_2_S[UI_MULT_LANU_END] =
{
    {"mti2_m1.jpg",0},
    {"mti2_m1.jpg",0},
    {"mti2_m1.jpg",0}
};

UI_NODE_PHOTO H_1_S[UI_MULT_LANU_END] =
{
    {"h1_m1.jpg",0},
    {"h1_m1.jpg",0},
    {"h1_m1.jpg",0}
};

UI_NODE_PHOTO H_2_S[UI_MULT_LANU_END] =
{
    {"h2_m1.jpg",0},
    {"h2_m1.jpg",0},
    {"h2_m1.jpg",0}
};

UI_NODE_PHOTO M_1_S[UI_MULT_LANU_END] =
{
    {"m1_m1.jpg",0},
    {"m1_m1.jpg",0},
    {"m1_m1.jpg",0}
};

UI_NODE_PHOTO M_2_S[UI_MULT_LANU_END] =
{
    {"m2_m1.jpg",0},
    {"m2_m1.jpg",0},
    {"m2_m1.jpg",0}
};

UI_NODE_PHOTO L_1_S[UI_MULT_LANU_END] =
{
    {"l1_m1.jpg",0},
    {"l1_m1.jpg",0},
    {"l1_m1.jpg",0}
};

UI_NODE_PHOTO L_2_S[UI_MULT_LANU_END] =
{
    {"l2_m1.jpg",0},
    {"l2_m1.jpg",0},
    {"l2_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate30_1_S[UI_MULT_LANU_END] =
{
    {"f30p1_m1.jpg",0},
    {"f30p1_m1.jpg",0},
    {"f30p1_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate30_2_S[UI_MULT_LANU_END] =
{
    {"f30p2_m1.jpg",0},
    {"f30p2_m1.jpg",0},
    {"f30p2_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate15_1_S[UI_MULT_LANU_END] =
{
    {"f15p1_m1.jpg",0},
    {"f15p1_m1.jpg",0},
    {"f15p1_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate15_2_S[UI_MULT_LANU_END] =
{
    {"f15p2_m1.jpg",0},
    {"f15p2_m1.jpg",0},
    {"f15p2_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate5_1_S[UI_MULT_LANU_END] =
{
    {"f5p1_m1.jpg",0},
    {"f5p1_m1.jpg",0},
    {"f5p1_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate5_2_S[UI_MULT_LANU_END] =
{
    {"f5p2_m1.jpg",0},
    {"f5p2_m1.jpg",0},
    {"f5p2_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate10_1_S[UI_MULT_LANU_END] =
{
    {"f10p1_m1.jpg",0},
    {"f10p1_m1.jpg",0},
    {"f10p1_m1.jpg",0}
};

UI_NODE_PHOTO FrameRate10_2_S[UI_MULT_LANU_END] =
{
    {"f10p2_m1.jpg",0},
    {"f10p2_m1.jpg",0},
    {"f10p2_m1.jpg",0}
};

UI_NODE_PHOTO HD_1_S[UI_MULT_LANU_END] =
{
    {"rhd1_m1.jpg",0},
    {"rhd1_m1.jpg",0},
    {"rhd1_m1.jpg",0}
};

UI_NODE_PHOTO HD_2_S[UI_MULT_LANU_END] =
{
    {"rhd2_m1.jpg",0},
    {"rhd2_m1.jpg",0},
    {"rhd2_m1.jpg",0}
};

UI_NODE_PHOTO D1_1_S[UI_MULT_LANU_END] =
{
    {"d1_1_m1.jpg",0},
    {"d1_1_m1.jpg",0},
    {"d1_1_m1.jpg",0}
};

UI_NODE_PHOTO D1_2_S[UI_MULT_LANU_END] =
{
    {"d1_2_m1.jpg",0},
    {"d1_2_m1.jpg",0},
    {"d1_2_m1.jpg",0}
};

UI_NODE_PHOTO VGA_1_S[UI_MULT_LANU_END] =
{
    {"vga1_m1.jpg",0},
    {"vga1_m1.jpg",0},
    {"vga1_m1.jpg",0}
};

UI_NODE_PHOTO VGA_2_S[UI_MULT_LANU_END] =
{
    {"vga2_m1.jpg",0},
    {"vga2_m1.jpg",0},
    {"vga2_m1.jpg",0}
};

UI_NODE_PHOTO QVGA_1_S[UI_MULT_LANU_END] =
{
    {"qvg1_m1.jpg",0},
    {"qvg1_m1.jpg",0},
    {"qvg1_m1.jpg",0}
};

UI_NODE_PHOTO QVGA_2_S[UI_MULT_LANU_END] =
{
    {"qvg2_m1.jpg",0},
    {"qvg2_m1.jpg",0},
    {"qvg2_m1.jpg",0}
};

UI_NODE_PHOTO Yes_1_S[UI_MULT_LANU_END] =
{
    {"yes1_m1.jpg",0},
    {"yes1_m1.jpg",0},
    {"yes1_m1.jpg",0}
};

UI_NODE_PHOTO Yes_2_S[UI_MULT_LANU_END] =
{
    {"yes2_m1.jpg",0},
    {"yes2_m1.jpg",0},
    {"yes2_m1.jpg",0}
};

UI_NODE_PHOTO No_1_S[UI_MULT_LANU_END] =
{
    {"no1_m1.jpg",0},
    {"no1_m1.jpg",0},
    {"no1_m1.jpg",0}
};

UI_NODE_PHOTO No_2_S[UI_MULT_LANU_END] =
{
    {"no2_m1.jpg",0},
    {"no2_m1.jpg",0},
    {"no2_m1.jpg",0}
};

UI_NODE_PHOTO sec20_1_S[UI_MULT_LANU_END] =
{
    {"s20n1_m1.jpg",0},
    {"s20n1_m1.jpg",0},
    {"s20n1_m1.jpg",0}
};


UI_NODE_PHOTO sec20_2_S[UI_MULT_LANU_END] =
{
    {"s20n2_m1.jpg",0},
    {"s20n2_m1.jpg",0},
    {"s20n2_m1.jpg",0}
};

UI_NODE_PHOTO min1_1_S[UI_MULT_LANU_END] =
{
    {"m1n1_m1.jpg",0},
    {"m1n1_m1.jpg",0},
    {"m1n1_m1.jpg",0}
};

UI_NODE_PHOTO min1_2_S[UI_MULT_LANU_END] =
{
    {"m1n2_m1.jpg",0},
    {"m1n2_m1.jpg",0},
    {"m1n2_m1.jpg",0}
};

UI_NODE_PHOTO min5_1_S[UI_MULT_LANU_END] =
{
    {"m5n1_m1.jpg",0},
    {"m5n1_m1.jpg",0},
    {"m5n1_m1.jpg",0}
};

UI_NODE_PHOTO min5_2_S[UI_MULT_LANU_END] =
{
    {"m5n2_m1.jpg",0},
    {"m5n2_m1.jpg",0},
    {"m5n2_m1.jpg",0}
};

UI_NODE_PHOTO min10_1_S[UI_MULT_LANU_END] =
{
    {"m10n1_m1.jpg",0},
    {"m10n1_m1.jpg",0},
    {"m10n1_m1.jpg",0}
};

UI_NODE_PHOTO min10_2_S[UI_MULT_LANU_END] =
{
    {"m10n2_m1.jpg",0},
    {"m10n2_m1.jpg",0},
    {"m10n2_m1.jpg",0}
};

UI_NODE_PHOTO min15_1_S[UI_MULT_LANU_END] =
{
    {"m15n1_m1.jpg",0},
    {"m15n1_m1.jpg",0},
    {"m15n1_m1.jpg",0}
};

UI_NODE_PHOTO min15_2_S[UI_MULT_LANU_END] =
{
    {"m15n2_m1.jpg",0},
    {"m15n2_m1.jpg",0},
    {"m15n2_m1.jpg",0}
};

UI_NODE_PHOTO min30_1_S[UI_MULT_LANU_END] =
{
    {"m30n1_m1.jpg",0},
    {"m30n1_m1.jpg",0},
    {"m30n1_m1.jpg",0}
};

UI_NODE_PHOTO min30_2_S[UI_MULT_LANU_END] =
{
    {"m30n2_m1.jpg",0},
    {"m30n2_m1.jpg",0},
    {"m30n2_m1.jpg",0}
};

UI_NODE_PHOTO hr1_1_S[UI_MULT_LANU_END] =
{
    {"h1r1_m1.jpg",0},
    {"h1r1_m1.jpg",0},
    {"h1r1_m1.jpg",0}
};

UI_NODE_PHOTO hr1_2_S[UI_MULT_LANU_END] =
{
    {"h1r2_m1.jpg",0},
    {"h1r2_m1.jpg",0},
    {"h1r2_m1.jpg",0}
};

UI_NODE_PHOTO Channel1_1_S[UI_MULT_LANU_END] =
{
    {"cha11_m1.jpg",0},
    {"cha11_m1.jpg",0},
    {"cha11_m1.jpg",0}
};

UI_NODE_PHOTO Channel1_2_S[UI_MULT_LANU_END] =
{
    {"cha12_m1.jpg",0},
    {"cha12_m1.jpg",0},
    {"cha12_m1.jpg",0}
};

UI_NODE_PHOTO Channel2_1_S[UI_MULT_LANU_END] =
{
    {"cha21_m1.jpg",0},
    {"cha21_m1.jpg",0},
    {"cha21_m1.jpg",0}
};

UI_NODE_PHOTO Channel2_2_S[UI_MULT_LANU_END] =
{
    {"cha22_m1.jpg",0},
    {"cha22_m1.jpg",0},
    {"cha22_m1.jpg",0}
};

UI_NODE_PHOTO TV_1_S[UI_MULT_LANU_END] =
{
    {"tv1_m1.jpg",0},
    {"tv1_m1.jpg",0},
    {"tv1_m1.jpg",0}
};

UI_NODE_PHOTO TV_2_S[UI_MULT_LANU_END] =
{
    {"tv2_m1.jpg",0},
    {"tv2_m1.jpg",0},
    {"tv2_m1.jpg",0}
};

UI_NODE_PHOTO Panel_1_S[UI_MULT_LANU_END] =
{
    {"pan1_m1.jpg",0},
    {"pan1_m1.jpg",0},
    {"pan1_m1.jpg",0}
};

UI_NODE_PHOTO Panel_2_S[UI_MULT_LANU_END] =
{
    {"pan2_m1.jpg",0},
    {"pan2_m1.jpg",0},
    {"pan2_m1.jpg",0}
};

UI_NODE_PHOTO Bit8_1_S[UI_MULT_LANU_END] =
{
    {"bi81_m1.jpg",0},
    {"bi81_m1.jpg",0},
    {"bi81_m1.jpg",0}
};

UI_NODE_PHOTO Bit8_2_S[UI_MULT_LANU_END] =
{
    {"bi82_m1.jpg",0},
    {"bi82_m1.jpg",0},
    {"bi82_m1.jpg",0}
};

UI_NODE_PHOTO Bit16_1_S[UI_MULT_LANU_END] =
{
    {"b161_m1.jpg",0},
    {"b161_m1.jpg",0},
    {"b161_m1.jpg",0}
};

UI_NODE_PHOTO Bit16_2_S[UI_MULT_LANU_END] =
{
    {"b162_m1.jpg",0},
    {"b162_m1.jpg",0},
    {"b162_m1.jpg",0}
};

UI_NODE_PHOTO K8_1_S[UI_MULT_LANU_END] =
{
    {"k81_m1.jpg",0},
    {"k81_m1.jpg",0},
    {"k81_m1.jpg",0}
};

UI_NODE_PHOTO K8_2_S[UI_MULT_LANU_END] =
{
    {"k82_m1.jpg",0},
    {"k82_m1.jpg",0},
    {"k82_m1.jpg",0}
};

UI_NODE_PHOTO K16_1_S[UI_MULT_LANU_END] =
{
    {"k161_m1.jpg",0},
    {"k161_m1.jpg",0},
    {"k161_m1.jpg",0}
};

UI_NODE_PHOTO K16_2_S[UI_MULT_LANU_END] =
{
    {"k162_m1.jpg",0},
    {"k162_m1.jpg",0},
    {"k162_m1.jpg",0}
};

UI_NODE_PHOTO K32_1_S[UI_MULT_LANU_END] =
{
    {"k321_m1.jpg",0},
    {"k321_m1.jpg",0},
    {"k321_m1.jpg",0}
};

UI_NODE_PHOTO K32_2_S[UI_MULT_LANU_END] =
{
    {"k322_m1.jpg",0},
    {"k322_m1.jpg",0},
    {"k322_m1.jpg",0}
};

UI_NODE_PHOTO K44_1_S[UI_MULT_LANU_END] =
{
    {"k441_m1.jpg",0},
    {"k441_m1.jpg",0},
    {"k441_m1.jpg",0}
};

UI_NODE_PHOTO K44_2_S[UI_MULT_LANU_END] =
{
    {"k442_m1.jpg",0},
    {"k442_m1.jpg",0},
    {"k442_m1.jpg",0}
};

UI_NODE_PHOTO K48_1_S[UI_MULT_LANU_END] =
{
    {"k481_m1.jpg",0},
    {"k481_m1.jpg",0},
    {"k481_m1.jpg",0}
};

UI_NODE_PHOTO K48_2_S[UI_MULT_LANU_END] =
{
    {"k482_m1.jpg",0},
    {"k482_m1.jpg",0},
    {"k482_m1.jpg",0}
};

UI_NODE_OSD    OSD_CHANNEL_NODE[] =
{
    {OSD_ICON_CHANNEL_1M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,1},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
};

UI_NODE_OSD    OSD_ZOOM_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,1},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},

};
UI_NODE_OSD    OSD_SOUND_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_1M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,1},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},

};

UI_NODE_OSD    OSD_IMG_ADUJUST_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},

};

UI_NODE_OSD    OSD_MENU_NODE[] =
{
    {OSD_ICON_CHANNEL_2M  , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M     , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M    , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M  , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_1M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},

};

UI_NODE_OSD    OSD_PLAYBACK_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_1M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},

};

UI_NODE_OSD    OSD_SPLIT_NODE[] =
{
    {OSD_ICON_CHANNEL_1M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_SPLIT_1M      , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1}, 
    {OSD_ICON_FULL_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},      
};

UI_NODE_OSD    OSD_FULL_NODE[] =
{
    {OSD_ICON_CHANNEL_1M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_SPLIT_2M      , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1}, 
    {OSD_ICON_FULL_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},      

};

UI_NODE_OSD    OSD_ZOOMCH1_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_1M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},

};

UI_NODE_OSD    OSD_ZOOMCH2_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_1M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,1},
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},

};
UI_NODE_OSD    OSD_ZOOMCH3_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_1M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,1},
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},

};
UI_NODE_OSD    OSD_ZOOMCH4_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_1M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,1},
};


UI_NODE_OSD    OSD_SOUNDSET_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_1M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,1},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    /*未完成*/
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},    
};

UI_NODE_OSD    OSD_IMGCH1_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_1M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};

UI_NODE_OSD    OSD_IMGCH2_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_1M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,1},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};
UI_NODE_OSD    OSD_IMGCH3_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_1M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,1},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};
UI_NODE_OSD    OSD_IMGCH4_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_1M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,1},        
};

UI_NODE_OSD    OSD_MENUIN_NODE[] =
{
    {0, 0, 0 ,0},

};
UI_NODE_OSD    OSD_PLAYBACKIN_NODE[] =
{
    {0, 0, 0 ,0},
};

UI_NODE_OSD    OSD_ZOOM_IN_CH1[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,1},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_1M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},
    {OSD_ICON_CH2_2M, UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_2M, UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_2M, UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},
    {OSD_ICON_ZOOM_IN_1M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},

};


UI_NODE_OSD    OSD_ZOOM_IN_CH2[] =
{
    {OSD_ICON_CHANNEL_2M  , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M     , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,1},
    {OSD_ICON_SOUND_2M    , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M  , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_1M, UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,1},
    {OSD_ICON_CH3_2M, UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_2M, UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},
    {OSD_ICON_ZOOM_IN_1M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},

};


UI_NODE_OSD    OSD_ZOOM_IN_CH3[] =
{
    {OSD_ICON_CHANNEL_2M  , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M     , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,1},
    {OSD_ICON_SOUND_2M    , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M  , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_2M, UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_1M, UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,1},
    {OSD_ICON_CH4_2M, UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},
    {OSD_ICON_ZOOM_IN_1M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},
};


UI_NODE_OSD    OSD_ZOOM_IN_CH4[] =
{
    {OSD_ICON_CHANNEL_2M  , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_1M     , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,1},
    {OSD_ICON_SOUND_2M    , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_2M  , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,0},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},
    {OSD_ICON_CH2_2M, UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},
    {OSD_ICON_CH3_2M, UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},
    {OSD_ICON_CH4_1M, UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,1},
    {OSD_ICON_ZOOM_IN_1M, UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},

};

/*未完成*/
UI_NODE_OSD    OSD_CH1SET_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_1M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,1},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};

UI_NODE_OSD    OSD_CH2SET_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_1M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,1},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};
UI_NODE_OSD    OSD_CH3SET_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_1M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,1},     
    {OSD_ICON_CH4_2M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,0},        

};
UI_NODE_OSD    OSD_CH4SET_NODE[] =
{
    {OSD_ICON_CHANNEL_2M    , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_NODE1_Y ,0},
    {OSD_ICON_ZOOM_2M       , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_NODE2_Y ,0},
    {OSD_ICON_SOUND_2M      , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_NODE3_Y ,0},
    {OSD_ICON_IMG_SET_1M    , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_NODE4_Y ,1},
    {OSD_ICON_MENU_2M       , UI_MENU_OSD_LOC_NODE5_X, UI_MENU_OSD_LOC_NODE5_Y ,0},
    {OSD_ICON_PLAYBACK_2M   , UI_MENU_OSD_LOC_NODE6_X, UI_MENU_OSD_LOC_NODE6_Y ,0},
    {OSD_ICON_CH1_2M        , UI_MENU_OSD_LOC_NODE1_X, UI_MENU_OSD_LOC_SUB_NODE1_Y ,0},    
    {OSD_ICON_CH2_2M        , UI_MENU_OSD_LOC_NODE2_X, UI_MENU_OSD_LOC_SUB_NODE2_Y ,0},    
    {OSD_ICON_CH3_2M        , UI_MENU_OSD_LOC_NODE3_X, UI_MENU_OSD_LOC_SUB_NODE3_Y ,0},     
    {OSD_ICON_CH4_1M        , UI_MENU_OSD_LOC_NODE4_X, UI_MENU_OSD_LOC_SUB_NODE4_Y ,1},        
};



UI_NODE_FILE    RESOLUTION_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Resolution_T,  UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_1_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};


UI_NODE_FILE    REC_SET_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {RECSet_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_2_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};

UI_NODE_FILE    REC_MODE_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {RECMode_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_2_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};

UI_NODE_FILE    SYSTEM_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {System_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_2_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};


UI_NODE_FILE    AUDIO_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Audio_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_2_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_1_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};

UI_NODE_FILE    CAMERA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Resolution_2_M,UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {RECSet_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {RECMode_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {System_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Audio_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {CamM_1_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
};

UI_NODE_FILE    DELETE_YES_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Delete_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Delete_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Yes_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    DELETE_NO_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Delete_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Delete_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {No_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    RES_CAM1_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Resolution_T,  UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    RES_CAM2_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Resolution_T,  UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    RES_CAM3_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Resolution_T,  UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    RES_CAM4_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Resolution_T,  UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    OVERWRITE_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Overwrite_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_1_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Section_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Motion_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Channel_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
};

UI_NODE_FILE    SECTION_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Section_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Motion_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Channel_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
};

UI_NODE_FILE    MOTION_SET_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Motion_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Section_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Motion_1_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Channel_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
};

UI_NODE_FILE    CHANNEL_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Channel_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_2_M,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Section_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Motion_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Channel_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
};

UI_NODE_FILE    MANUAL_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {RECMode_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {RECMode_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_SUB_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Manual_1_S,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Motion_2_S,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Manual_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    MOTION_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {RECMode_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {RECMode_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Manual_2_S,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Motion_1_S,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Motion_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    TIME_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Time_T,            UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_1_M,          UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Format_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {CareInfo_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Network_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Display_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
};

UI_NODE_FILE    FORMAT_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Format_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_2_M,          UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Format_1_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {CareInfo_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Network_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Display_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
};

UI_NODE_FILE    CARDINFO_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CardInfo_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_2_M,          UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Format_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {CareInfo_1_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Network_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Display_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
};

UI_NODE_FILE    NETWORK_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Network_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_2_M,          UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Format_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {CareInfo_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Network_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Display_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
};

UI_NODE_FILE    DISPLAY_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Display_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_2_M,          UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Format_2_M,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {CareInfo_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Network_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Display_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
};

UI_NODE_FILE    AUDIO_RES_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {AudRes_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {AudRes_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Sampling_2_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};

UI_NODE_FILE    SAMPLING_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {AudRes_2_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};

UI_NODE_FILE    PAIR_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Pair_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Pair_1_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {CamOnOff_2_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};

UI_NODE_FILE    CAM_ONOFF_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CamOnOff_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Pair_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {CamOnOff_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};


UI_NODE_FILE    C1_HD_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {HD_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C1_VGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {VGA_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C1_QVGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {QVGA_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C2_HD_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {HD_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C2_VGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {VGA_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C2_QVGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {QVGA_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C3_HD_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {HD_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C3_VGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {VGA_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C3_QVGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {QVGA_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C4_HD_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {HD_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C4_VGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {VGA_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    C4_QVGA_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {HD_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {VGA_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {QVGA_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {QVGA_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};


UI_NODE_FILE    OVERWRITE_YES_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Overwrite_T,   UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_1_M, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Yes_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    OVERWRITE_NO_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Overwrite_T,   UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Overwrite_1_M, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {No_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_20SEC_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {sec20_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_1MIN_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {min1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_5MIN_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_1_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {min5_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_10MIN_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_1_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {min10_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_15MIN_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_1_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {min15_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_30MIN_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {sec20_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min5_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {min30_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {min30_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SECTION_1HR_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Section_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Section_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {min1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {min5_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {min10_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {min15_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {min30_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {hr1_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE6_X, UI_MENU_LOC_NODE16_Y},
    {hr1_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    MOTION_MASK_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {MaskArea_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {MaskArea_1_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Sensitivity_2_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};

UI_NODE_FILE    MOTION_SENSITIVITY_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sensitivity_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {MaskArea_2_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Sensitivity_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
};

UI_NODE_FILE    CHANNEL_1_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Channel_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Channel_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Channel1_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Channel1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Channel1_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CHANNEL_2_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Channel_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Channel_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Channel1_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Channel1_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Channel2_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    DATETIME_NODE[] =
{
    {Background, UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Time_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Time_1_M,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
};

UI_NODE_FILE    FORMAT_YES_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Format_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE_X, UI_MENU_LOC_TITLE_Y},
    {Format_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Yes_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    FORMAT_NO_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Format_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Format_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {No_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CARD_INFO_NODE[] =
{
    {Background, UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CardInfo_T, UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {CardInfo,   UI_MENU_SIZE_CARD_INFO_X, UI_MENU_SIZE_CARD_INFO_Y, UI_MENU_LOC_CARD_INFO_X, UI_MENU_LOC_CARD_INFO_Y},
};

UI_NODE_FILE    NET_WORK_NODE[] =
{
    {Background, UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
//    {Network_T, UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
//    {Network,    UI_MENU_SIZE_CARD_INFO_X, UI_MENU_SIZE_CARD_INFO_Y, UI_MENU_LOC_CARD_INFO_X, UI_MENU_LOC_CARD_INFO_Y},
};

UI_NODE_FILE    DISPLAY_TV_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Display_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Display_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {TV_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Panel_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {TV_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    DISPLAY_PANEL_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Display_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Display_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {TV_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Panel_1_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Panel_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    AUDRES_8BIT_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {AudRes_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {AudRes_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Bit8_1_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Bit16_2_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Bit8_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    AUDRES_16BIT_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {AudRes_T,      UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {AudRes_1_M,    UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Bit8_2_S,      UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Bit16_1_S,     UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Bit16_T,       UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SAMPLING_8K_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {K8_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {K16_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {K32_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {K44_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {K48_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {K8_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SAMPLING_16K_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {K8_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {K16_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {K32_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {K44_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {K48_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {K16_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SAMPLING_32K_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {K8_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {K16_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {K32_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {K44_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {K48_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {K32_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SAMPLING_44K_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {K8_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {K16_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {K32_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {K44_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {K48_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {K44_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SAMPLING_48K_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sampling_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sampling_1_M,  UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {K8_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {K16_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {K32_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {K44_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {K48_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE5_X, UI_MENU_LOC_NODE15_Y},
    {K48_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    PAIR_CAM1_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Pair_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    PAIR_CAM2_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Pair_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    PAIR_CAM3_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Pair_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    PAIR_CAM4_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Pair_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    ON_CAM1_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CamOnOff_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    ON_CAM2_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CamOnOff_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    ON_CAM3_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CamOnOff_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    ON_CAM4_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG1_X, UI_MENU_SIZE_BG1_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {CamOnOff_T,    UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Cam_2_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE4_X, UI_MENU_LOC_NODE14_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SENSITIVITY_H_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sensitivity_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sensitivity_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {H_1_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {M_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {L_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {H_T,               UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SENSITIVITY_M_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sensitivity_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sensitivity_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {H_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {M_1_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {L_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {M_T,               UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    SENSITIVITY_L_NODE[] =
{
    {Background,        UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Sensitivity_T,     UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Sensitivity_1_M,   UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {H_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {M_2_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {L_1_S,             UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE3_X, UI_MENU_LOC_NODE13_Y},
    {L_T,               UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM1_ON_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {On_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM1_OFF_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam1_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Off_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM2_ON_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {On_T,          UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM2_OFF_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam2_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Off_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM3_ON_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {On_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM3_OFF_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam3_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Off_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM4_ON_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_1_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_2_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {On_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};

UI_NODE_FILE    CAM4_OFF_NODE[] =
{
    {Background,    UI_MENU_SIZE_BG_X, UI_MENU_SIZE_BG_Y, UI_MENU_LOC_BG_X, UI_MENU_LOC_BG_Y},
    {Cam4_T,        UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_TITLE1_X, UI_MENU_LOC_TITLE1_Y},
    {Cam_1_M,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_PARENT_X, UI_MENU_LOC_PARENT_Y},
    {Yes_2_S,       UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y},
    {No_1_S,        UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y},
    {Off_T,         UI_MENU_SIZE_TITLE_X, UI_MENU_SIZE_TITLE_Y, UI_MENU_LOC_SUB_TITLE_X, UI_MENU_LOC_SUB_TITLE_Y},
};


#if (SUPPORT_TOUCH == 1)
UITOUCH_MENU_NODE_EVENT    gUITouchNodeSetup1[] =
{
    {  UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 0},
};

UITOUCH_MENU_NODE_EVENT    gUITouchNodeSetup2[] =
{
    {  UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 0},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 1},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 2},
    {  20, 20, 180, 20, UI_KEY_MODE, -1},

};

UITOUCH_MENU_NODE_EVENT    gUITouchNodeSetup3[] =
{
    {  UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 0},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 1},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 2},
};

UITOUCH_MENU_NODE_EVENT    gUITouchNodeSetup4[] =
{
    {  UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 0},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 1},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 2},
};

UITOUCH_MENU_NODE_EVENT    gUITouchNodeSetup5[] =
{
    {  UI_MENU_LOC_NODE1_X, UI_MENU_LOC_NODE11_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 0},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 1},
    {  UI_MENU_LOC_NODE2_X, UI_MENU_LOC_NODE12_Y, UI_MENU_SIZE_NODE_X, UI_MENU_SIZE_NODE_Y, UI_KEY_ENTER, 2},

};

UITOUCH_NODE_EVENT_TBL gUISetupTouchEventCheckTbl[] = 
{
    { gUITouchNodeSetup1,  sizeof(gUITouchNodeSetup1)/sizeof(UITOUCH_MENU_NODE_EVENT)},
    { gUITouchNodeSetup2,  sizeof(gUITouchNodeSetup2)/sizeof(UITOUCH_MENU_NODE_EVENT)},
    { gUITouchNodeSetup3,  sizeof(gUITouchNodeSetup3)/sizeof(UITOUCH_MENU_NODE_EVENT)},    
};
#endif

#if (SUPPORT_TOUCH == 1)
UI_NODE_DATA uiMenuNodeListItem[] =
{

    /*level 0*/
    {"Channel Control", OsdNum(OSD_CHANNEL_NODE),       NULL, OSD_CHANNEL_NODE,     UI_MENU_SETIDX_FULL_SCREEN, NULL},
    {"Setup",           OsdNum(OSD_ZOOM_NODE),          NULL, OSD_ZOOM_NODE,        UI_MENU_SETIDX_NO_ACTION, NULL},
    {"Playback",        OsdNum(OSD_SOUND_NODE),         NULL, OSD_SOUND_NODE,       UI_MENU_SETIDX_VOLUME, NULL},
    /*level 1*/

    {"Split 4",         OsdNum(OSD_SPLIT_NODE), NULL, OSD_SPLIT_NODE,   UI_MENU_SETIDX_NO_ACTION, NULL},
    {"Full Screen",     OsdNum(node)(OSD_FULL_NODE),  NULL, OSD_FULL_NODE,    UI_MENU_SETIDX_NO_ACTION, NULL},

    {"Resolution",      DataNum(RESOLUTION_NODE), RESOLUTION_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"REC. Setting",    DataNum(REC_SET_NODE),  REC_SET_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION, NULL},

    {"Playback Mode",   DataNum(OSD_SOUND_NODE),         NULL, OSD_SOUND_NODE,       UI_MENU_SETIDX_VOLUME, NULL},
};
#else
UI_NODE_DATA uiMenuNodeListItem[] =
{

    /*level 0*/
    {"Channel Control", sizeof(OSD_CHANNEL_NODE)/sizeof(UI_NODE_OSD),       NULL, OSD_CHANNEL_NODE,     UI_MENU_SETIDX_FULL_SCREEN},
    {"Image Zoom",      sizeof(OSD_ZOOM_NODE)/sizeof(UI_NODE_OSD),          NULL, OSD_ZOOM_NODE,        UI_MENU_SETIDX_NO_ACTION},
    {"Sound control",   sizeof(OSD_SOUND_NODE)/sizeof(UI_NODE_OSD),         NULL, OSD_SOUND_NODE,       UI_MENU_SETIDX_VOLUME},
    {"Image adjust",    sizeof(OSD_IMG_ADUJUST_NODE)/sizeof(UI_NODE_OSD),   NULL, OSD_IMG_ADUJUST_NODE, UI_MENU_SETIDX_NO_ACTION},
    {"Menu",            sizeof(OSD_MENU_NODE)/sizeof(UI_NODE_OSD),          NULL, OSD_MENU_NODE,        UI_MENU_SETIDX_NO_ACTION},
    {"Playback",        sizeof(OSD_PLAYBACK_NODE)/sizeof(UI_NODE_OSD),      NULL, OSD_PLAYBACK_NODE,    UI_MENU_SETIDX_PLAYBACK},

    /*level 1*/

    {"Split 4",         sizeof(OSD_SPLIT_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_SPLIT_NODE,   UI_MENU_SETIDX_NO_ACTION},
    {"Full Screen",     sizeof(OSD_FULL_NODE)/sizeof(UI_NODE_OSD),  NULL, OSD_FULL_NODE,    UI_MENU_SETIDX_NO_ACTION},

    {"Zoom CH1",        sizeof(OSD_ZOOMCH1_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOMCH1_NODE, UI_MENU_SETIDX_CH1ZOOM}, /*Zoom CH1*/
    {"Zoom CH2",        sizeof(OSD_ZOOMCH2_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOMCH2_NODE, UI_MENU_SETIDX_CH2ZOOM}, /*Zoom CH2*/
    {"Zoom CH3",        sizeof(OSD_ZOOMCH3_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOMCH3_NODE, UI_MENU_SETIDX_CH3ZOOM}, /*Zoom CH3*/
    {"Zoom CH4",        sizeof(OSD_ZOOMCH4_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOMCH4_NODE, UI_MENU_SETIDX_CH4ZOOM}, /*Zoom CH4*/

    {"SOUND SET",       sizeof(OSD_SOUNDSET_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_SOUNDSET_NODE, UI_MENU_SETIDX_NO_ACTION},       

    {"Image adjust CH1", sizeof(OSD_IMGCH1_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_IMGCH1_NODE, UI_MENU_SETIDX_CH1IMG},  /* Image adjust CH1*/
    {"Image adjust CH2", sizeof(OSD_IMGCH2_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_IMGCH2_NODE, UI_MENU_SETIDX_CH2IMG},  /* Image adjust CH2*/
    {"Image adjust CH3", sizeof(OSD_IMGCH3_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_IMGCH3_NODE, UI_MENU_SETIDX_CH3IMG},  /* Image adjust CH3*/
    {"Image adjust CH4", sizeof(OSD_IMGCH4_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_IMGCH4_NODE, UI_MENU_SETIDX_CH4IMG},  /* Image adjust CH4*/

   // {"Playback",        sizeof(PLAYBACK_NODE)/sizeof(UI_NODE_FILE), PLAYBACK_NODE,  NULL, UI_MENU_SETIDX_PLAYBACK},
    {"Resolution",      sizeof(RESOLUTION_NODE)/sizeof(UI_NODE_FILE), RESOLUTION_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"REC. Setting",    sizeof(REC_SET_NODE)/sizeof(UI_NODE_FILE),  REC_SET_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION},
    {"REC. Mode",       sizeof(REC_MODE_NODE)/sizeof(UI_NODE_FILE), REC_MODE_NODE,  NULL, UI_MENU_SETIDX_REC_MODE},
    {"System Setting",  sizeof(SYSTEM_NODE)/sizeof(UI_NODE_FILE),   SYSTEM_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Audio",           sizeof(AUDIO_NODE)/sizeof(UI_NODE_FILE),    AUDIO_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Camer",           sizeof(CAMERA_NODE)/sizeof(UI_NODE_FILE),   CAMERA_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Search All",  0, NULL, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Search By",   0, NULL, NULL, UI_MENU_SETIDX_NO_ACTION},

    /*level 2*/
    {"CH1 INOUT", sizeof(OSD_ZOOM_IN_CH1)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOM_IN_CH1, UI_MENU_SETIDX_NO_ACTION},
    {"CH2 INOUT", sizeof(OSD_ZOOM_IN_CH2)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOM_IN_CH2, UI_MENU_SETIDX_NO_ACTION},
    {"CH3 INOUT", sizeof(OSD_ZOOM_IN_CH3)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOM_IN_CH3, UI_MENU_SETIDX_NO_ACTION},
    {"CH4 INOUT", sizeof(OSD_ZOOM_IN_CH4)/sizeof(UI_NODE_OSD), NULL, OSD_ZOOM_IN_CH4, UI_MENU_SETIDX_NO_ACTION},

    {"CH1", sizeof(OSD_CH1SET_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_CH1SET_NODE, UI_MENU_SETIDX_NO_ACTION},
    {"CH2", sizeof(OSD_CH2SET_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_CH2SET_NODE, UI_MENU_SETIDX_NO_ACTION},
    {"CH3", sizeof(OSD_CH3SET_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_CH3SET_NODE, UI_MENU_SETIDX_NO_ACTION},
    {"CH4", sizeof(OSD_CH4SET_NODE)/sizeof(UI_NODE_OSD), NULL, OSD_CH4SET_NODE, UI_MENU_SETIDX_NO_ACTION},

    {"Res Camera 1", sizeof(RES_CAM1_NODE)/sizeof(UI_NODE_FILE), RES_CAM1_NODE, NULL, UI_MENU_SETIDX_VIDEO_SIZE},
    {"Res Camera 2", sizeof(RES_CAM2_NODE)/sizeof(UI_NODE_FILE), RES_CAM2_NODE, NULL, UI_MENU_SETIDX_VIDEO_SIZE_CH2},
    {"Res Camera 3", sizeof(RES_CAM3_NODE)/sizeof(UI_NODE_FILE), RES_CAM3_NODE, NULL, UI_MENU_SETIDX_VIDEO_SIZE_CH3},
    {"Res Camera 4", sizeof(RES_CAM4_NODE)/sizeof(UI_NODE_FILE), RES_CAM4_NODE, NULL, UI_MENU_SETIDX_VIDEO_SIZE_CH4},

    {"Overwrite",       sizeof(OVERWRITE_NODE)/sizeof(UI_NODE_FILE),  OVERWRITE_NODE,   NULL, UI_MENU_SETIDX_OVERWRITE},
    {"Section",         sizeof(SECTION_NODE)/sizeof(UI_NODE_FILE),    SECTION_NODE,     NULL, UI_MENU_SETIDX_SECTION},
    {"Motion Detection",sizeof(MOTION_SET_NODE)/sizeof(UI_NODE_FILE), MOTION_SET_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Channel",         sizeof(CHANNEL_NODE)/sizeof(UI_NODE_FILE),    CHANNEL_NODE,     NULL, UI_MENU_SETIDX_CHANNEL},

    {"Manual", sizeof(MANUAL_NODE)/sizeof(UI_NODE_FILE), MANUAL_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Motion", sizeof(MOTION_NODE)/sizeof(UI_NODE_FILE), MOTION_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Time",            sizeof(TIME_NODE)/sizeof(UI_NODE_FILE),     TIME_NODE,     NULL, UI_MENU_SETIDX_DATE_TIME},
    {"Format",          sizeof(FORMAT_NODE)/sizeof(UI_NODE_FILE),   FORMAT_NODE,   NULL, UI_MENU_SETIDX_FORMAT},
    {"Card Info",       sizeof(CARDINFO_NODE)/sizeof(UI_NODE_FILE), CARDINFO_NODE, NULL, UI_MENU_SETIDX_CARDINFO},
    {"Network",         sizeof(NETWORK_NODE)/sizeof(UI_NODE_FILE),  NETWORK_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Display Device",  sizeof(DISPLAY_NODE)/sizeof(UI_NODE_FILE),  DISPLAY_NODE,  NULL, UI_MENU_SETIDX_DISPLAY},

    {"Resolution",      sizeof(AUDIO_RES_NODE)/sizeof(UI_NODE_FILE), AUDIO_RES_NODE, NULL, UI_MENU_SETIDX_AUDRES},
    {"Sampling Rate",   sizeof(SAMPLING_NODE)/sizeof(UI_NODE_FILE),  SAMPLING_NODE,  NULL, UI_MENU_SETIDX_SAMPLING},
    
    {"Pair",            sizeof(PAIR_NODE)/sizeof(UI_NODE_FILE),         PAIR_NODE,  NULL, UI_MENU_SETIDX_PAIRING},
    {"Camer OnOff",     sizeof(CAM_ONOFF_NODE)/sizeof(UI_NODE_FILE),    CAM_ONOFF_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"All Delete Yes",  sizeof(DELETE_YES_NODE)/sizeof(UI_NODE_FILE),   DELETE_YES_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"All Delete No",   sizeof(DELETE_NO_NODE)/sizeof(UI_NODE_FILE),    DELETE_NO_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Delete Yes",  sizeof(DELETE_YES_NODE)/sizeof(UI_NODE_FILE),   DELETE_YES_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Delete No",   sizeof(DELETE_NO_NODE)/sizeof(UI_NODE_FILE),    DELETE_NO_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    /*level 3*/
    {"C1 HD",      sizeof(C1_HD_NODE)/sizeof(UI_NODE_FILE),   C1_HD_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C1 VGA",     sizeof(C1_VGA_NODE)/sizeof(UI_NODE_FILE),  C1_VGA_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C1 QVGA",    sizeof(C1_QVGA_NODE)/sizeof(UI_NODE_FILE), C1_QVGA_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"C2 HD",      sizeof(C2_HD_NODE)/sizeof(UI_NODE_FILE),   C2_HD_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C2 VGA",     sizeof(C2_VGA_NODE)/sizeof(UI_NODE_FILE),  C2_VGA_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C2 QVGA",    sizeof(C2_QVGA_NODE)/sizeof(UI_NODE_FILE), C2_QVGA_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"C3 HD",      sizeof(C3_HD_NODE)/sizeof(UI_NODE_FILE),   C3_HD_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C3 VGA",     sizeof(C3_VGA_NODE)/sizeof(UI_NODE_FILE),  C3_VGA_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C3 QVGA",    sizeof(C3_QVGA_NODE)/sizeof(UI_NODE_FILE), C3_QVGA_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"C4 HD",      sizeof(C4_HD_NODE)/sizeof(UI_NODE_FILE),   C4_HD_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C4 VGA",     sizeof(C4_VGA_NODE)/sizeof(UI_NODE_FILE),  C4_VGA_NODE,   NULL, UI_MENU_SETIDX_NO_ACTION},
    {"C4 QVGA",    sizeof(C4_QVGA_NODE)/sizeof(UI_NODE_FILE), C4_QVGA_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},


    {"Overwrite Yes",   sizeof(OVERWRITE_YES_NODE)/sizeof(UI_NODE_FILE), OVERWRITE_YES_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Overwrite No",    sizeof(OVERWRITE_NO_NODE)/sizeof(UI_NODE_FILE),  OVERWRITE_NO_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Section 20sec",   sizeof(SECTION_20SEC_NODE)/sizeof(UI_NODE_FILE), SECTION_20SEC_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Section 1min",    sizeof(SECTION_1MIN_NODE)/sizeof(UI_NODE_FILE),  SECTION_1MIN_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Section 5min",    sizeof(SECTION_5MIN_NODE)/sizeof(UI_NODE_FILE),  SECTION_5MIN_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Section 10hr",    sizeof(SECTION_10MIN_NODE)/sizeof(UI_NODE_FILE), SECTION_10MIN_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Section 15min",   sizeof(SECTION_15MIN_NODE)/sizeof(UI_NODE_FILE), SECTION_15MIN_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Section 30min",   sizeof(SECTION_30MIN_NODE)/sizeof(UI_NODE_FILE), SECTION_30MIN_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    

    {"Mask Area",   sizeof(MOTION_MASK_NODE)/sizeof(UI_NODE_FILE),        MOTION_MASK_NODE,         NULL, UI_MENU_SETIDX_MOTION_MASK},
    {"Seneitivity", sizeof(MOTION_SENSITIVITY_NODE)/sizeof(UI_NODE_FILE), MOTION_SENSITIVITY_NODE,  NULL, UI_MENU_SETIDX_MOTION_SENSITIVITY},

    {"Channel 1", sizeof(CHANNEL_1_NODE)/sizeof(UI_NODE_FILE), CHANNEL_1_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Channel 2", sizeof(CHANNEL_2_NODE)/sizeof(UI_NODE_FILE), CHANNEL_2_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Time Setting",        sizeof(DATETIME_NODE)/sizeof(UI_NODE_FILE),     DATETIME_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Format Yes",          sizeof(FORMAT_YES_NODE)/sizeof(UI_NODE_FILE),   FORMAT_YES_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Format No",           sizeof(FORMAT_NO_NODE)/sizeof(UI_NODE_FILE),    FORMAT_NO_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Card Size",           sizeof(CARD_INFO_NODE)/sizeof(UI_NODE_FILE),    CARD_INFO_NODE,     NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Set Network",         sizeof(NET_WORK_NODE)/sizeof(UI_NODE_FILE),     NET_WORK_NODE,      NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Display Device TV",   sizeof(DISPLAY_TV_NODE)/sizeof(UI_NODE_FILE),   DISPLAY_TV_NODE,    NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Display Device Panel",sizeof(DISPLAY_PANEL_NODE)/sizeof(UI_NODE_FILE),DISPLAY_PANEL_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Resolution 8bit",     sizeof(AUDRES_8BIT_NODE)/sizeof(UI_NODE_FILE),  AUDRES_8BIT_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Resolution 16bit",    sizeof(AUDRES_16BIT_NODE)/sizeof(UI_NODE_FILE), AUDRES_16BIT_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Sampling Rate 8k",    sizeof(SAMPLING_8K_NODE)/sizeof(UI_NODE_FILE),  SAMPLING_8K_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Sampling Rate 16k",   sizeof(SAMPLING_16K_NODE)/sizeof(UI_NODE_FILE), SAMPLING_16K_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Sampling Rate 32k",   sizeof(SAMPLING_32K_NODE)/sizeof(UI_NODE_FILE), SAMPLING_32K_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Sampling Rate 44.1k", sizeof(SAMPLING_44K_NODE)/sizeof(UI_NODE_FILE), SAMPLING_44K_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Sampling Rate 48k",   sizeof(SAMPLING_48K_NODE)/sizeof(UI_NODE_FILE), SAMPLING_48K_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Pair Cam1", sizeof(PAIR_CAM1_NODE)/sizeof(UI_NODE_FILE), PAIR_CAM1_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Pair Cam2", sizeof(PAIR_CAM2_NODE)/sizeof(UI_NODE_FILE), PAIR_CAM2_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Pair Cam3", sizeof(PAIR_CAM3_NODE)/sizeof(UI_NODE_FILE), PAIR_CAM3_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Pair Cam4", sizeof(PAIR_CAM4_NODE)/sizeof(UI_NODE_FILE), PAIR_CAM4_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"On Cam1", sizeof(ON_CAM1_NODE)/sizeof(UI_NODE_FILE), ON_CAM1_NODE, NULL, UI_MENU_SETIDX_CH1_ON},
    {"On Cam2", sizeof(ON_CAM2_NODE)/sizeof(UI_NODE_FILE), ON_CAM2_NODE, NULL, UI_MENU_SETIDX_CH2_ON},
    {"On Cam3", sizeof(ON_CAM3_NODE)/sizeof(UI_NODE_FILE), ON_CAM3_NODE, NULL, UI_MENU_SETIDX_CH3_ON},
    {"On Cam4", sizeof(ON_CAM4_NODE)/sizeof(UI_NODE_FILE), ON_CAM4_NODE, NULL, UI_MENU_SETIDX_CH4_ON},


   /*level 4*/
    {"Motion Mask Setting", 0, NULL,NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Seneitivity H", sizeof(SENSITIVITY_H_NODE)/sizeof(UI_NODE_FILE), SENSITIVITY_H_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Seneitivity M", sizeof(SENSITIVITY_M_NODE)/sizeof(UI_NODE_FILE), SENSITIVITY_M_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Seneitivity L", sizeof(SENSITIVITY_L_NODE)/sizeof(UI_NODE_FILE), SENSITIVITY_L_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},

    {"Cam1 Off",    sizeof(CAM1_OFF_NODE)/sizeof(UI_NODE_FILE), CAM1_OFF_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam1 On",     sizeof(CAM1_ON_NODE)/sizeof(UI_NODE_FILE),  CAM1_ON_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam2 Off",    sizeof(CAM2_OFF_NODE)/sizeof(UI_NODE_FILE), CAM2_OFF_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam2 On",     sizeof(CAM2_ON_NODE)/sizeof(UI_NODE_FILE),  CAM2_ON_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam3 Off",    sizeof(CAM3_OFF_NODE)/sizeof(UI_NODE_FILE), CAM3_OFF_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam3 On",     sizeof(CAM3_ON_NODE)/sizeof(UI_NODE_FILE),  CAM3_ON_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam4 Off",    sizeof(CAM4_OFF_NODE)/sizeof(UI_NODE_FILE), CAM4_OFF_NODE, NULL, UI_MENU_SETIDX_NO_ACTION},
    {"Cam4 On",     sizeof(CAM4_ON_NODE)/sizeof(UI_NODE_FILE),  CAM4_ON_NODE,  NULL, UI_MENU_SETIDX_NO_ACTION},
};
#endif

