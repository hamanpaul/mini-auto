/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_cmd_project.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "uiapi.h"
#include "ui.h"
#include "gpioapi.h"
#include "sysapi.h"
#include "ui_project.h"
#include "rfiuapi.h"
#include "i2capi.h"
#include "board.h"
#include "timerapi.h"
#include "GlobalVariable.h"
#include "MotionDetect_API.h"
#include "spiapi.h"
#include "uartapi.h"
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
#include "gfuapi.h"
#endif
#if iHome_LOG_TEST
#include "dcfapi.h"
#endif

#if (HW_BOARD_OPTION == MR6730_AFN) 
#include "MainFlow.h"
#endif

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
typedef struct _TIMER_PWM_CFG {
    INT32U  preScale;
    INT32U  pulseWidth;
    INT32U  clockDivisor;
    INT32U  pwmEnable;
    INT32U  pwmMode;
    INT32U  runMode;
} TIMER_PWM_CFG;
#if IDU_OSD_TEST
u16 TVout_Y[]={TVOUT_Y_NTSC,TVOUT_Y_PAL};
#endif
u8  k1[64],k2[64];
u8  pmac[13];
u8  uiDESK1Count=0;
u8  uiDESK2Count=0;
#if IS_COMMAX_DOORPHONE
u8  UI_gamma=0;
#endif
u32 uiPlayPWMFreg = 200;
#if USB2WIFI_SUPPORT
u8 Snapshot_Error = 0;
#endif
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
 extern u8 uiEnZoom;
#endif       
extern GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio1IntCfg[GPIO_PIN_COUNT];
#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) ) // A1020DIFF1016
extern GPIO_INT_CFG gpio2IntCfg[GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio3IntCfg[GPIO_PIN_COUNT];
#endif
extern GPIO_CFG gpioCfg[GPIO_GROUP_COUNT][GPIO_PIN_COUNT];
#if RFIU_SUPPORT
  extern unsigned int gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
  extern unsigned int gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];
  extern unsigned int gRfiu_WrapEnc_Sta[MAX_RFIU_UNIT];
  extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];

#endif
extern u32 guiIISPlayDMAId;
extern OS_EVENT    *mpeg4ReadySemEvt;
u8 PhotoFramenum=2;
#if (HW_BOARD_OPTION == MR8200_RX_COMMAX)
extern UI_SCHEDULE_SET ScheduledTime[UI_MENU_SETTING_SCHEDULE_MAX];
#endif
extern u32 spiNetwrokStartAddr;
#if MOTOR_EN
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
//NULL
#else
extern u8  MotorStatusH;
extern u8  MotorStatusV;
#endif
#endif

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
extern u8 rfiuVocDustMode;
#endif

#if AFN_DEBUG
extern u8   AudioTrigger[MULTI_CHANNEL_LOCAL_MAX];
#endif
#if SUPPORT_TOUCH
extern int uiTouchLevel;
#endif

#if HOME_RF_SUPPORT
extern u8 DefaultSensorCnt;
#endif
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern s32 gpioIntConfig(u8 group, u8 pin, GPIO_INT_CFG* pCfg);
extern INT32U marsTimerCountEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerCountWrite(INT32U uiTimerId, INT32U count);
extern INT32U marsTimerCountRead(INT32U uiTimerId, INT32U* pCount);
extern INT32U marsTimerPwmEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerPwmCountEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerPwmConfig(INT32U uiTimerId, TIMER_PWM_CFG* pCfg);
extern void TestEncrypt(void);
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
#if 1
#if IDU_OSD_TEST
void OSD_TEST1(void)
{
    u16 i=0,j=0 ;
    u32 data;
    u16 icon_w,icon_h;

    icon_w = OSDDispWidth[sysTVOutOnFlag];
    icon_h = OSDDispHeight[sysTVOutOnFlag];
    uiMenuOSDReset();
    if(sysTVOutOnFlag)
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, 0, 0, TVOSD_SizeX, TVOSD_SizeY);
        uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], icon_w, icon_h, i, j, IDU_OSD_L0_WINDOW_0, 0xc0c0c0c0);
    }
    else // pannel
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, 0, 0, OSD_SizeX, OSD_SizeY);
        uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], icon_w, icon_h, i, j, IDU_OSD_L1_WINDOW_0, 0xc0c0c0c0);

    }
}

void OSD_TEST2(void)
{
    u16 i,j;
    u32 data;
    u16 icon_w=40,icon_h=40;
    
    uiMenuOSDReset();
    if(sysTVOutOnFlag)
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, 0, 0, TVOSD_SizeX, TVOSD_SizeY);
        uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 320, 240, 160, 120, IDU_OSD_L0_WINDOW_0, 0xc0c0c0c0);

        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, 140, 100, 180, 140);
        uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], icon_w, icon_w, 0, 0, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
    }
    else // pannel
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, 0, 0, OSD_SizeX, OSD_SizeY);
        uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 320, 240, 160, 120, IDU_OSD_L1_WINDOW_0, 0xc0c0c0c0);

        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, 140, 100, 180, 140);
        uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], icon_w, icon_w, 0, 0, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
    }
}

void OSD_TEST3(void)
{
    u16 i,j;
    u16 icon_w=40,icon_h=40;
    
    uiMenuOSDReset();
    if(sysTVOutOnFlag)
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, 0, 0, TVOUT_X, TVout_Y[TvOutMode]);
        uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 320, 240, 160, 120, IDU_OSD_L0_WINDOW_0, 0xc0c0c0c0);

        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, 0, 0, 40, 40);
        uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 4, 0, 0, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 4, 0, 4, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 12, 4, 0, 8, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 16, 4, 0, 12, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 20, 4, 0, 16, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 24, 4, 0, 20, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 28, 4, 0, 24, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 12, 4, 0, 28, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 4, 0, 32, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 4, 0, 36, IDU_OSD_L1_WINDOW_0, 0xc1c1c1c1);
        
        DEBUG_UI("xy(%d , %d)\r\n",TVOUT_X ,TVout_Y[TvOutMode]);
        for(j=0; j<(TVout_Y[TvOutMode]-icon_h); j+=40)
        for(i=0 ;i<(TVOUT_X-icon_w); i+=24)
        {
            (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, i, j, 40+i, 40+j);
            DEBUG_UI("pos(%d , %d)\r\n",i ,j);
            OSTimeDly(1);
        }
    }
    else // pannel
    {
        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, 0, 0, OSD_SizeX, OSD_SizeY);
        uiOsdEnable(IDU_OSD_L1_WINDOW_0);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 320, 240, 160, 120, IDU_OSD_L1_WINDOW_0, 0xc0c0c0c0);

        (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, 0, 0, 40, 40);
        uiOsdEnable(IDU_OSD_L0_WINDOW_0);
        
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 4, 0, 0, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 4, 0, 4, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 12, 4, 0, 8, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 16, 4, 0, 12, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 20, 4, 0, 16, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 24, 4, 0, 20, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 28, 4, 0, 24, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 12, 4, 0, 28, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 4, 0, 32, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 4, 0, 36, IDU_OSD_L0_WINDOW_0, 0xc1c1c1c1);
            
        for(j=0; j<(OSD_SizeY-icon_h); j+=40)
        for(i=0 ;i<(OSD_SizeX-icon_w); i+=24)
        {
            (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, i, j, 40+i, 40+j);
            DEBUG_UI("pos(%d , %d)\r\n",i ,j);
            OSTimeDly(2);
        }

    }
}

void OSD_Up(void)
{
    u16 OSD_SX,OSD_SY;
    u16 OSD_EX,OSD_EY;

    if(sysTVOutOnFlag)
    {
    OSD_SX = tvOSDL1W0_WSP  -0xA2;
    OSD_SY = (tvOSDL1W0_WSP>>16)-0x2;
    OSD_EX = (tvOSDL1W0_WEP +1)-0xA2;
    OSD_EY = ((tvOSDL1W0_WEP>>16)+1)-0x2;
    DEBUG_UI("tvOSDL1W0_WSP=%x\n",tvOSDL1W0_WSP);
    DEBUG_UI("tvOSDL1W0_WEP=%x\n",tvOSDL1W0_WEP);

    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, OSD_SX, OSD_SY-8,OSD_EX, OSD_EY-8);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX ,OSD_SY-8,OSD_EX, OSD_EY-8);
    }
    else
    {
    OSD_SX = IduOsdWin0Start  ;
    OSD_SY = IduOsdWin0Start >>16;
    OSD_EX = IduOsdWin0End +1;
    OSD_EY = (IduOsdWin0End>>16)+1;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, OSD_SX, OSD_SY-8,OSD_EX, OSD_EY-8);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX ,OSD_SY-8,OSD_EX, OSD_EY-8);
    }

}
void OSD_Down(void)
{
    u16 OSD_SX,OSD_SY;
    u16 OSD_EX,OSD_EY;
    
    if(sysTVOutOnFlag)
    {
    OSD_SX = tvOSDL1W0_WSP  -0xA2;
    OSD_SY = (tvOSDL1W0_WSP>>16)-0x2;
    OSD_EX = (tvOSDL1W0_WEP +1)-0xA2;
    OSD_EY = ((tvOSDL1W0_WEP>>16)+1)-0x2;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, OSD_SX, OSD_SY+8,OSD_EX, OSD_EY+8);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX ,OSD_SY+8,OSD_EX, OSD_EY+8);
    }
    else
    {
    OSD_SX = IduOsdWin0Start  ;
    OSD_SY = IduOsdWin0Start >>16;
    OSD_EX = IduOsdWin0End +1;
    OSD_EY = (IduOsdWin0End>>16)+1;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, OSD_SX, OSD_SY+8,OSD_EX, OSD_EY+8);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX ,OSD_SY+8,OSD_EX, OSD_EY+8);
    }

}
void OSD_Right(void)
{
    u16 OSD_SX,OSD_SY;
    u16 OSD_EX,OSD_EY;

    if(sysTVOutOnFlag)
    {
    OSD_SX = tvOSDL1W0_WSP  -0xA2;
    OSD_SY = (tvOSDL1W0_WSP>>16)-0x2;
    OSD_EX = (tvOSDL1W0_WEP +1)-0xA2;
    OSD_EY = ((tvOSDL1W0_WEP>>16)+1)-0x2;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, OSD_SX+4, OSD_SY,OSD_EX+4, OSD_EY);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX+4,OSD_SY,OSD_EX+4, OSD_EY);
    }
    else
    {
    OSD_SX = IduOsdWin0Start  ;
    OSD_SY = IduOsdWin0Start >>16;
    OSD_EX = IduOsdWin0End +1;
    OSD_EY = (IduOsdWin0End>>16)+1;
     (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, OSD_SX+4, OSD_SY,OSD_EX+4, OSD_EY);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX+4,OSD_SY,OSD_EX+4, OSD_EY);
    }
}
void OSD_Left(void)
{
    u16 OSD_SX,OSD_SY;
    u16 OSD_EX,OSD_EY;

    if(sysTVOutOnFlag)
    {
    OSD_SX = tvOSDL1W0_WSP  -0xA2;
    OSD_SY = (tvOSDL1W0_WSP>>16)-0x2;
    OSD_EX = (tvOSDL1W0_WEP +1)-0xA2;
    OSD_EY = ((tvOSDL1W0_WEP>>16)+1)-0x2;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L1_WINDOW_0, OSD_SX-4, OSD_SY,OSD_EX-4, OSD_EY);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX-4,OSD_SY,OSD_EX-4, OSD_EY);
    }
    else
    {
    OSD_SX = IduOsdWin0Start  ;
    OSD_SY = IduOsdWin0Start >>16;
    OSD_EX = IduOsdWin0End +1;
    OSD_EY = (IduOsdWin0End>>16)+1;
    (*OSDDisplay[sysTVOutOnFlag])(IDU_OSD_L0_WINDOW_0, OSD_SX-4, OSD_SY,OSD_EX-4, OSD_EY);
    DEBUG_UI("start(%d ,%d) end(%d ,%d)\r\n",OSD_SX-4,OSD_SY,OSD_EX-4, OSD_EY);
    }
}
#endif
void uiOsdTest(u8 *cmd)
{
    u32 x_s, x_e, y_s, y_e;
    u8 i, cur_buf, win_num;
    u16 icon_w,icon_h;
    u32 data;

    if (!strcmp((char*)cmd,"RESET"))
    {
        DEBUG_UI("RESET OSD!!!\r\n");
        uiMenuOSDReset();
    }
    else if (!strncmp((char*)cmd, "EN ", strlen("EN ")))
    {
        cmd+=strlen("EN ");
        if (!strcmp((char*)cmd,"L1"))
        {
            DEBUG_UI("Enable OSD L1\r\n");
            win_num = IDU_OSD_L1_WINDOW_0;
        }
        else if (!strcmp((char*)cmd,"L2"))
        {
            DEBUG_UI("Enable OSD L2\r\n");
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &win_num);
            DEBUG_UI("Enable OSD Window %d\r\n",win_num);
        }
        uiOsdEnable(win_num);
    }
    else if (!strncmp((char*)cmd, "DIS ", strlen("DIS ")))
    {
        cmd+=strlen("DIS ");
        if (!strcmp((char*)cmd,"L1"))
        {
            DEBUG_UI("Disable OSD L1\r\n");
            win_num = IDU_OSD_L1_WINDOW_0;
        }
        else if (!strcmp((char*)cmd,"L2"))
        {
            DEBUG_UI("Disable OSD L2\r\n");
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &win_num);
            DEBUG_UI("Disable OSD window %d\r\n",win_num);
        }
        uiOsdDisable(win_num);
    }
    else if (!strncmp((char*)cmd, "SIZE ", strlen("SIZE ")))
    {
        cmd+=strlen("SIZE ");
        if (!strncmp((char*)cmd, "L1 ", strlen("L1 ")))
        {
            sscanf((char*)cmd, "L1 <%d,%d>,<%d,%d>", &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set L1 Form (%d,%d) to (%d,%d)\r\n",x_s, y_s, x_e, y_e);
            win_num = IDU_OSD_L1_WINDOW_0;
        }
        else if (!strncmp((char*)cmd, "L2 ", strlen("L2 ")))
        {
            sscanf((char*)cmd, "L2 <%d,%d>,<%d,%d>", &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set L2 Form (%d,%d) to (%d,%d)\r\n",x_s, y_s, x_e, y_e);
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d <%d,%d>,<%d,%d>", &win_num, &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set Window %d Form (%d,%d) to (%d,%d)\r\n",win_num, x_s, y_s, x_e, y_e);
        }
        (*OSDDisplay[sysTVOutOnFlag])(win_num, x_s, y_s, x_e, y_e);
    }
    else if (!strncmp((char*)cmd, "64COLOR ", strlen("64COLOR ")))
    {
        cmd+=strlen("64COLOR ");
        if (!strcmp((char*)cmd,"L1"))
        {
            win_num = IDU_OSD_L1_WINDOW_0;
        }
        else if (!strcmp((char*)cmd,"L2"))
        {
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &win_num);
        }
        uiClearOSDBuf(win_num);
        DEBUG_UI("Draw Window %d Icon\r\n",win_num);
        (*OSDDisplay[sysTVOutOnFlag])(win_num, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);

        for (i = 0; i < 64; i++)
        {
            data = ((i+192)<<24)+((i+192)<<16)+((i+192)<<8)+(i+192);
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (OSDDispWidth[sysTVOutOnFlag]/8), 30, i%8*(OSDDispWidth[sysTVOutOnFlag]/8), i/8*30, win_num, data);
        }
        uiOsdEnable(win_num);
    }
    else if (!strncmp((char*)cmd, "B ", strlen("B ")))
    {
        cmd+=strlen("B ");
        if (!strncmp((char*)cmd, "L1 ", strlen("L1 ")))
        {
            cur_buf = IDU_OSD_L1_WINDOW_0;
            cmd+=strlen("L1 ");
        }
        else if (!strncmp((char*)cmd, "L2 ", strlen("L2 ")))
        {
            cur_buf = IDU_OSD_L2_WINDOW_0;
            cmd+=strlen("L2 ");
        }
        else if (!strncmp((char*)cmd, "W0 ", strlen("W0 ")))
        {
            cur_buf = IDU_OSD_L0_WINDOW_0;
            cmd+=strlen("W0 ");
        }
        else if (!strncmp((char*)cmd, "W1 ", strlen("W1 ")))
        {
            cur_buf = IDU_OSD_L0_WINDOW_1;
            cmd+=strlen("W1 ");
        }
        else if (!strncmp((char*)cmd, "W2 ", strlen("W2 ")))
        {
            cur_buf = IDU_OSD_L0_WINDOW_2;
            cmd+=strlen("W2 ");
        }
        uiOsdEnable(cur_buf);
        sscanf((char*)cmd, "<%d*%d> <%d,%d> <%x>", &icon_w, &icon_h, &x_s, &y_s, &data);
        DEBUG_UI("Draw Window %d Icon %dx%d, in (%d,%d) data %x\r\n",cur_buf,icon_w, icon_h,  x_s, y_s, data);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], icon_w, icon_h, x_s, y_s, cur_buf, data);
    }
    else if (!strncmp((char*)cmd, "CL ", strlen("CL ")))
    {
        cmd+=strlen("CL ");
        if (!strcmp((char*)cmd,"L1"))
        {
            DEBUG_UI("Clear OSD L1\r\n");
            cur_buf = IDU_OSD_L1_WINDOW_0;
        }
        else if (!strcmp((char*)cmd,"L2"))
        {
            DEBUG_UI("Clear OSD L2\r\n");
            cur_buf = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &cur_buf);
            DEBUG_UI("Clear OSD Window %d\r\n",cur_buf);
        }
        uiClearOSDBuf(cur_buf);
        DEBUG_UI("Clear Window %d Buffer\r\n",cur_buf);

    }
#if IDU_OSD_TEST
    else if (!strncmp((char*)cmd, "TEST1", strlen("TEST1")))
    {
        DEBUG_UI("OSD TEST1\r\n");
        OSD_TEST1();
    }
    else if (!strncmp((char*)cmd, "TEST2", strlen("TEST2")))
    {
        DEBUG_UI("OSD TEST2\r\n");
        OSD_TEST2();
    }
    else if (!strncmp((char*)cmd, "TEST3", strlen("TEST3")))
    {
        DEBUG_UI("OSD TEST3\r\n");
        OSD_TEST3();
    }
    else if (!strcmp((char*)cmd,"U"))
    {
        DEBUG_UI("OSD_Up \r\n");
        OSD_Up();
    }
    else if (!strcmp((char*)cmd,"D"))
    {
        DEBUG_UI("OSD_Down\r\n");
        OSD_Down();
    }
    else if (!strcmp((char*)cmd,"R"))
    {
        DEBUG_UI("OSD_Right\r\n");
        OSD_Right();
    }
    else if (!strcmp((char*)cmd,"L"))
    {
        DEBUG_UI("OSD_Left\r\n");
        OSD_Left();
    }
#endif
}
void uiTimeTest(u8 *cmd)
{
    RTC_DATE_TIME   dateTime;
    RTC_COUNT       count;
    u32  year, month, day, hour, min, sec;

    DEBUG_UI("uiTimeTest %s\n", cmd);
    if(!strncmp((char*)cmd,"SET ", strlen("SET ")))
    {
        sscanf((char*)cmd, "SET %d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        dateTime.year=year;
        dateTime.month=month;
        dateTime.day=day;
        dateTime.hour=hour;
        dateTime.min=min;
        dateTime.sec=sec;
        DEBUG_UI("Set Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
        RTC_Set_Time(&dateTime);
    }
    else if(!strncmp((char*)cmd,"TMP COUNT ", strlen("TMP COUNT ")))
    {
        cmd+=strlen("TMP COUNT ");
        sscanf((char*)cmd, "%d %d:%d:%d", &day, &hour, &min, &sec);
        count.day=day;
        count.hour=hour;
        count.min=min;
        count.sec=sec;
        DEBUG_UI("Set Tmp Count %d %d:%d:%d\r\n", count.day, count.hour, count.min, count.sec);
        rtcSetTmpCount(&count);
    }
    else if(!strncmp((char*)cmd,"TMP TIME ", strlen("TMP TIME ")))
    {
        cmd+=strlen("TMP TIME ");
        sscanf((char*)cmd, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        dateTime.year=year;
        dateTime.month=month;
        dateTime.day=day;
        dateTime.hour=hour;
        dateTime.min=min;
        dateTime.sec=sec;
        DEBUG_UI("Set Tmp Time %d/%d/%d %d:%d:%d\r\n", dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
        rtcGetTmpCount(&count);
        DEBUG_UI("Get Tmp Count %d %d:%d:%d\r\n", count.day, count.hour, count.min, count.sec);
        RTC_Set_Time_With_TmpCnt(&dateTime, &count);
    }
    else if(!strcmp((char*)cmd,"EN TIME"))
        showTime = 1;
    else if(!strcmp((char*)cmd,"DIS TIME"))
        showTime = 0;
}

void uiIRTest(u8 *cmd)
{
    u32 CUSTOM_CODE_ID;
    u32 RecCustom;
    u8 div;

    if (!strcmp((char*)cmd,"RESET ON"))
    {
        DEBUG_UI("RESET ON IR!!!\r\n");
        IRCtrReset(TRUE);
    }
    else if (!strcmp((char*)cmd,"RESET OFF"))
    {
        DEBUG_UI("RESET OFF IR!!!\r\n");
        IRCtrReset(FALSE);
    }
    else if (!strncmp((char*)cmd,"DIV ", strlen("DIV ")))
    {
        cmd+=strlen("DIV ");
        sscanf(cmd, "%d",&div);
        DEBUG_UI("Clock Divisor : %d \r\n",div);
        IRSetDiv(div);
    }
    else if (!strncmp((char*)cmd, "ENABLE", strlen("ENABLE")))
    {
        DEBUG_UI("Enable IR \r\n");
        IRCtrEnable(TRUE);
    }
    else if (!strncmp((char*)cmd, "DISABLE", strlen("DISABLE")))
    {
        DEBUG_UI("Disable IR \r\n");
        IRCtrEnable(FALSE);
    }
    else if(!strncmp((char*)cmd, "READCODE", strlen("READCODE")))
        {
        cmd+=strlen("READCODE");
        IRGetRecCustomCode(&RecCustom);
        DEBUG_UI("CUSTOM CODE ID: %x \r\n",RecCustom);

        }
    else if (!strncmp((char*)cmd, "CODE ", strlen("CODE ")))
    {
        cmd+=strlen("CODE ");
        sscanf((char*)cmd, "<%x>",&CUSTOM_CODE_ID);
        DEBUG_UI("CUSTOM CODE ID: %x \r\n",CUSTOM_CODE_ID);
        IRSetCustomCode(CUSTOM_CODE_ID);
    }
    else if (!strncmp((char*)cmd, "INT ", strlen("INT ")))
    {
        cmd+=strlen("INT ");
        if (!strcmp((char*)cmd,"DISABLE"))
        {
            DEBUG_UI("DISABLE INT IR!!! \r\n");
            IREnableInt(FALSE);
        }
        else if (!strcmp((char*)cmd,"ENABLE"))
        {
            DEBUG_UI("ENABLE INT IR!!! \r\n");
            IREnableInt(TRUE);
        }
    }
}

void uiTestGPIO(u8* cmd)
{
    u32 i, group;
    u8  level;

    DEBUG_UI("uiTestLed %s\n", cmd);

    sscanf((char*)cmd, "%d %d ", &group,&i);
    cmd+=strlen("1 04 ");
    if(!strcmp((char*)cmd,"LEVEL_H"))
    {
        gpioSetLevel(group, i, 1);
        DEBUG_UI("Set GPIO %d_%d High\n", group, i);
    }
    else if(!strcmp((char*)cmd,"LEVEL_L"))
    {
        gpioSetLevel(group, i, 0);
        DEBUG_UI("Set GPIO %d_%d Low\n", group, i);
    }
    else if(!strcmp((char*)cmd,"LEVEL"))
    {
        gpioGetLevel(group, i, &level);
        DEBUG_UI("Get GPIO %d_%d %d\n", group, i, level);
    }
    else if(!strcmp((char*)cmd,"FALL EN"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intFallEdgeEna = 1;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intFallEdgeEna = 1;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if ((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B)|| (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intFallEdgeEna = 1;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intFallEdgeEna = 1;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d FALL Enable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"FALL DIS"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intFallEdgeEna = 0;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intFallEdgeEna = 0;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intFallEdgeEna = 0;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intFallEdgeEna = 0;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d FALL Disable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"RISE EN"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intRiseEdgeEna = 1;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intRiseEdgeEna = 1;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intRiseEdgeEna = 1;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intRiseEdgeEna = 1;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d RISE Enable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"RISE DIS"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intRiseEdgeEna = 0;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intRiseEdgeEna = 0;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intRiseEdgeEna = 0;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intRiseEdgeEna = 0;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d RISE Disable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"INT EN"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intEna = 1;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intEna = 1;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intEna = 1;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intEna = 1;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d INT Enable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"INT DIS"))
    {
        if(group == 0)
        {
            gpio0IntCfg[i].intEna = 0;
            gpioIntConfig(group, i, &gpio0IntCfg[i]);
        }
        else if(group == 1)
        {
            gpio1IntCfg[i].intEna = 0;
            gpioIntConfig(group, i, &gpio1IntCfg[i]);
        }
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        else if(group == 2)
        {
            gpio2IntCfg[i].intEna = 0;
            gpioIntConfig(group, i, &gpio2IntCfg[i]);
        }
        else if(group == 3)
        {
            gpio3IntCfg[i].intEna = 0;
            gpioIntConfig(group, i, &gpio3IntCfg[i]);
        }
    #endif
        DEBUG_UI("Set GPIO %d_%d INT Disable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"LVTRG H"))
    {
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        if((group == 1) || (group == 2) || (group == 3))
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_HIGH);
            DEBUG_UI("Set GPIO %d_%d Level Triger High\n", group, i);
        }        
    #else
        if(group == 1)
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_HIGH);
            DEBUG_UI("Set GPIO %d_%d Level Triger High\n", group, i);
        }
    #endif
        else
        {
            DEBUG_UI("Level Triger High Error Group %d\n", group, i);
        }
    }
    else if(!strcmp((char*)cmd,"LVTRG L"))
    {
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        if((group == 1) || (group == 2) || (group == 3))
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_LOW);
            DEBUG_UI("Set GPIO %d_%d Level Triger Low\n", group, i);
        }        
    #else
        if(group == 1)
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_LOW);
            DEBUG_UI("Set GPIO %d_%d Level Triger Low\n", group, i);
        }
    #endif
        else
        {
            DEBUG_UI("Level Triger Low Error Group %d\n", group, i);
        }
    }
    else if(!strcmp((char*)cmd,"LVTRG OFF"))
    {
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        if((group == 1) || (group == 2) || (group == 3))
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_NONE);
            DEBUG_UI("Set GPIO %d_%d Level Triger OFF\n", group, i);
        }        
    #else
        if(group == 1)
        {
            gpioLvTrgIntCfg(group, i, GPIO_LVTRG_NONE);
            DEBUG_UI("Set GPIO %d_%d Level Triger OFF\n", group, i);
        }
    #endif
        else
        {
            DEBUG_UI("Level Triger OFF Error Group %d\n", group, i);
        }
    }
    else if(!strcmp((char*)cmd,"EN"))
    {
        gpioCfg[group][i].ena = GPIO_ENA;
        gpioConfig(group, i, &gpioCfg[group][i]);
        DEBUG_UI("Set GPIO %d_%d Enable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"DIS"))
    {
        gpioCfg[group][i].ena = GPIO_DISA;
        gpioConfig(group, i, &gpioCfg[group][i]);
        DEBUG_UI("Set GPIO %d_%d Disable\n", group, i);
    }
    else if(!strcmp((char*)cmd,"DIR_IN"))
    {
        gpioCfg[group][i].dir= GPIO_DIR_IN;
        gpioConfig(group, i, &gpioCfg[group][i]);
        DEBUG_UI("Set GPIO %d_%d Dir In\n", group, i);
    }
    else if(!strcmp((char*)cmd,"DIR_OUT"))
    {
        gpioCfg[group][i].dir= GPIO_DIR_OUT;
        gpioConfig(group, i, &gpioCfg[group][i]);
        DEBUG_UI("Set GPIO %d_%d Dir Out\n", group, i);
    }
}

void uiTimerTest(u8 *cmd)
{
    u32 num;
    DEBUG_UI("uiTimer Test %s\n", cmd);
    if(!strncmp((char*)cmd,"EN ", strlen("EN ")))
    {
        cmd+=strlen("EN ");
        sscanf((char*)cmd, "%d", &num);
        DEBUG_UI("Enable timer%d\n", num);
        marsTimerCountEnable(num, 1);
    }
    else if(!strncmp((char*)cmd,"DIS ", strlen("DIS ")))
    {
        cmd+=strlen("DIS ");
        sscanf((char*)cmd, "%d", &num);
        DEBUG_UI("Disable timer%d\n", num);
        marsTimerCountEnable(num, 0);
    }
}

void uiPWMTest(u8 *cmd)
{
    u32 id;
    u32 num;
    TIMER_PWM_CFG   cfg;

    DEBUG_UI("uiPWMTest %s\n", cmd);
    sscanf((char*)cmd, "%d ", &id);
    cmd+=strlen("1 ");

    if(!strncmp((char*)cmd,"CNT ", strlen("CNT ")))
    {
        cmd+=strlen("CNT ");
        sscanf((char*)cmd, "%d", &num);
        DEBUG_UI("ui PWM %d Set Counter %d\n", id, num);
        marsTimerCountWrite(id, num);
    }
    else if(!strcmp((char*)cmd,"RCNT"))
    {
        marsTimerCountRead(id, (INT32U*)&num);
        DEBUG_UI("ui PWM %d Get Counter %d\n", id, num);
    }
    else if(!strcmp((char*)cmd,"EN"))
    {
        DEBUG_UI("ui Enable PWM %d\n", id);
        marsTimerPwmEnable(id, 1);
    }
    else if(!strcmp((char*)cmd,"DIS"))
    {
        marsTimerPwmEnable(id, 0);
        DEBUG_UI("ui Disable PWM %d\n", id);
    }
    else if(!strcmp((char*)cmd,"ENCNT"))
    {
        DEBUG_UI("ui Enable PWM %d Count\n", id);
        marsTimerPwmCountEnable(id, 1);
    }
    else if(!strcmp((char*)cmd,"DISCNT"))
    {
        DEBUG_UI("ui Disable PWM %d Count\n", id);
        marsTimerPwmCountEnable(id, 0);
    }
    else if(!strncmp((char*)cmd,"CFG ", strlen("CFG ")))
    {
        cmd+=strlen("CFG ");
        sscanf((char*)cmd, "S %d W %d D%d PM%d RM%d", &(cfg.preScale), &(cfg.pulseWidth), &(cfg.clockDivisor), &(cfg.pwmMode), &(cfg.runMode));
        marsTimerPwmConfig(id, &cfg);
    }
    else if(!strncmp((char*)cmd,"FREQ ", strlen("FREQ ")))
    {
        sscanf((char*)cmd, "FREQ %d", &num);
        uiPlayPWMFreg = num;
        Beep_function(3,uiPlayPWMFreg,60,60,200,0);
        DEBUG_UI("PWM FREQ = %d \n", num);
    }
    else if(!strncmp((char*)cmd,"VOL ", strlen("VOL "))) /* Be used to test in adjust alarm volume value */
    {
        sscanf((char*)cmd, "VOL %d", &num);
        gBeep_pause_width = num;
        DEBUG_UI("PWM gBeep_pause_width = %d \n", gBeep_pause_width);
        Beep_function(3,uiPlayPWMFreg,60,60,200,0);
    }

}
#endif

#if RFIU_SUPPORT
  #if RF_PAIR_EN
	void uiRFPairTest(u8 *cmd,int RFUnit)
	{
	    DEBUG_UI("uiRFPAIR-%d Test: %s\n",RFUnit,cmd);
	    //marsRfiu_Test();
	    rfiu_PAIR_Linit(RFUnit);
	}
  #endif

  #if RF_CMD_EN
	int uiRXCMDTest(u8 *cmd,int RFUnit)
	{
	    int busy;
        int reso_W,reso_H;
        int RetryCnt;

	    DEBUG_UI("uiRF_RX_CMD: %s\n", cmd);

        if(!strncmp((char*)cmd,"RESO ", strlen("RESO ")))
        {
            sscanf((char*)cmd, "RESO %d %d", &reso_W,&reso_H);

            if( (reso_W == 1280) && (reso_H == 720))
            {
               if(iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit] == UI_MENU_SETTING_RESOLUTION_HD )
                  return 1;
                
               iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit]=UI_MENU_SETTING_RESOLUTION_HD;
            }
            else if( (reso_W == 704) && (reso_H == 480))
            {
               if(iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit] == UI_MENU_SETTING_RESOLUTION_D1_480V)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit]=UI_MENU_SETTING_RESOLUTION_D1_480V;
            }
            else if( (reso_W == 640) && (reso_H == 480))
            {
               if(iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit] == UI_MENU_SETTING_RESOLUTION_VGA)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit]=UI_MENU_SETTING_RESOLUTION_VGA;
            }
            else if( (reso_W == 320) && (reso_H == 240))
            {
               if(iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit] == UI_MENU_SETTING_RESOLUTION_QVGA)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit]=UI_MENU_SETTING_RESOLUTION_QVGA;
            }
            else if( (reso_W == 352) && (reso_H == 240))
            {
               if(iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit] == UI_MENU_SETTING_RESOLUTION_352x240)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_CH1_RES+RFUnit]=UI_MENU_SETTING_RESOLUTION_352x240;
            }
        }

        RetryCnt=0;
        while(RetryCnt<3)
        {
	       busy=rfiu_RXCMD_Enc(cmd,RFUnit);
           if(busy==0)
             break;
           OSTimeDly(1);
           RetryCnt ++;
        }
        if(busy)
        {
           DEBUG_UI("RFRXCMD is busy:%d !\n",RFUnit);
           return 0;
        }
        else
            return 1;
	}

    int uiTXCMDTest(u8 *cmd,int RFUnit)
	{
	    int busy;

	    DEBUG_UI("uiRF_TX_CMD: %s\n", cmd);
	    busy=rfiu_TXCMD_Enc(cmd,RFUnit);
        if(busy)
           DEBUG_UI("RFTXCMD is busy!\n");
	}
  #endif
#endif

void uiDESTest(u8 *cmd)
{
    u32  suraddr;
    u32  outaddr;
    u8   iv[8];
    u8   key1[8],key2[8],key3[8];
    u8   i,j;
    u8   temp;


    if(!strncmp((char*)cmd,"SURADDR ", strlen("SURADDR ")))
    {
        cmd+=strlen("SURADDR ");
        sscanf((char *)cmd,"%lx",&suraddr);

        DEBUG_UI("suraddr: %x\r\n",suraddr);

    }
    else if(!strncmp((char*)cmd,"OUTADDR ", strlen("OUTADDR ")))
    {
        cmd+=strlen("OUTADDR ");
        sscanf((char *)cmd,"%lx",&outaddr);
        DEBUG_UI("outaddr: %x \r\n",outaddr);
    }
    else if(!strncmp((char*)cmd,"SET IV ", strlen("SET IV ")))
    {

        DEBUG_UI("iv: ");
        cmd+=strlen("SET IV ");
        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            iv[i]= temp;
            cmd+=3;
            DEBUG_UI("%x ",iv[i]);
        }

        DEBUG_UI("\r\n");
    }
    else if(!strncmp((char*)cmd,"SET KEY1 ", strlen("SET KEY1 ")))
    {
        DEBUG_UI("key1: ");
        cmd+=strlen("SET KEY1 ");


        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            key1[i]=temp;
            cmd+=3;
            DEBUG_UI("%x ",key1[i]);
        }
        DEBUG_UI("\r\n");
    }
    else if(!strncmp((char*)cmd,"SET KEY2 ", strlen("SET KEY2 ")))
    {
        DEBUG_UI("key2: ");
        cmd+=strlen("SET KEY2 ");


        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            key2[i]=temp;
            cmd+=3;
            DEBUG_UI("%x ",key2[i]);
        }
        DEBUG_UI("\r\n");
    }
    else if(!strncmp((char*)cmd,"SET KEY3 ", strlen("SET KEY3 ")))
    {
        DEBUG_UI("key3: ");
        cmd+=strlen("SET KEY3 ");

        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            key3[i]=temp;
            cmd+=3;
            DEBUG_UI("%x ",key3[i]);
        }
        DEBUG_UI("\r\n");
    }
    else if(!strncmp((char*)cmd,"SET K1 ", strlen("SET K1 ")))
    {
        cmd+=strlen("SET K1 ");
        DEBUG_UI("k1: ");
        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            k1[uiDESK1Count*8+i]=temp;
            cmd+=3;
        }
        if(uiDESK1Count<8)
        {
            uiDESK1Count++;
        }
        for(i=0;i<uiDESK1Count;i++)
        {
            for(j=0;j<8;j++)
                DEBUG_UI("%x ",k1[i*8+j]);
            DEBUG_UI("  %d\r\n",i);
        }
        if(uiDESK1Count==8)
            uiDESK1Count=0;

    }
    else if(!strncmp((char*)cmd,"SET K2 ", strlen("SET K2 ")))
    {
        cmd+=strlen("SET K2 ");
        DEBUG_UI("k2: ");
        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            k2[uiDESK2Count*8+i]=temp;
            cmd+=3;
        }
        if(uiDESK2Count<8)
        {
            uiDESK2Count++;
        }
        for(i=0;i<uiDESK2Count;i++)
        {
            for(j=0;j<8;j++)
                DEBUG_UI("%x ",k2[i*8+j]);
            DEBUG_UI("  %d\r\n",i);
        }
        if(uiDESK2Count==8)
            uiDESK2Count=0;

    }
    else if(!strncmp((char*)cmd,"SET PMAC ", strlen("SET PMAC ")))
    {
        DEBUG_UI("pmac: ");
        cmd+=strlen("SET PMAC ");
        for(i=0;i<8;i++)
        {
            sscanf((char *)cmd,"%x ",&temp);
            pmac[i]=temp;
            cmd+=3;
        }
        for(i=0;i<13;i++)
            DEBUG_UI("%x ",pmac[i]);
        DEBUG_UI("\r\n");
    }
    else if(!strncmp((char*)cmd,"SET PDATA SIZE ", strlen("SET PDATA SIZE ")))
    {
        DEBUG_UI("pmac: ");
        cmd+=strlen("SET PDATA SIZE ");
        pmac[8]=23;
        pmac[9]=3;
        pmac[10]=1;
        sscanf((char *)cmd,"%x %x",&i,&j);
        pmac[11]=i;
        pmac[12]=j;
        for(i=0;i<13;i++)
            DEBUG_UI("%x ",pmac[i]);
        DEBUG_UI("\r\n");

    }
    else if(!strcmp((char*)cmd,"DUMP SHA"))
    {
        DEBUG_UI("DUMP SHA \r\n");
    }
    else if(!strcmp((char*)cmd,"DUMP AUTH"))
    {
        DEBUG_UI("DUMP AUTH \r\n");
    }
    else if(!strcmp((char*)cmd,"ENCODE" ))
    {
        DEBUG_UI("DES ENCODE \r\n");
    }
    else if(!strcmp((char*)cmd,"DECODE" ))
    {
        DEBUG_UI("DES DECODE \r\n");
    }

    //DEBUG_UI("key contain: %s\r\n",cmd);

}

void uiBIT1605_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x %x",&slaveAddr,&registAddr);
        if(slaveAddr==I2C_BIT1605_RD_SLAV_ADDR)
        {
           i2cRead_BIT1605(registAddr,&data);
        }
        else if(slaveAddr==I2C_BIT1605_RD_SLAV_2_ADDR)
        {
            i2cRead_BIT1605_DV2(registAddr,&data);
        }

        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x %x",&slaveAddr,&registAddr,&data);

        if(slaveAddr==I2C_BIT1605_WR_SLAV_ADDR)
            i2cWrite_BIT1605(registAddr,data);
        else if(slaveAddr==I2C_BIT1605_WR_SLAV_2_ADDR)
            i2cWrite_BIT1605_DV2(registAddr,data);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
}
void uiWT8861_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x %x",&slaveAddr,&registAddr);
        i2cRead_WT8861(registAddr,&data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x %x",&slaveAddr,&registAddr,&data);
        i2cWrite_WT8861(registAddr,data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
}
#if (TV_DECODER == TW2866)
void uiTW2866_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        i2cRead_TW2866_Manual(registAddr,&data);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",I2C_TW2866_RD_SLAV_ADDR,registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_TW2866(registAddr,data);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",I2C_TW2866_WR_SLAV_ADDR,registAddr,data);
    }
}
#endif
void ui_contrast_test(u8 *cmd)
{
    u8 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        contrast_write(data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        contrast_read(&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_brightness_test(u8 *cmd)
{
    u8 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        brightness_write(data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        brightness_read(&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_saturation_test(u8 *cmd)
{
    u8 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        saturation_write(data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        saturation_read(&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_change_test(u8 *cmd)
{
    u8 data = 0;
    u8 name;
    if(!strncmp((char*)cmd,"CCTV ", strlen("CCTV ")))
    {
        cmd+=strlen("CCTV ");
        sscanf((char *)cmd,"%x",&data);
        name=2;

    }else if(!strncmp((char*)cmd,"CAM ", strlen("CAM ")))
    {
        cmd+=strlen("CAM ");
        sscanf((char *)cmd,"%x",&data);
        name=1;
    }
    CH_Channel_write(name,data);
    DEBUG_UI("data:%x  \n",data);

}
void ui_I2CLOCK_test(u8 *cmd)
{
    u8 data = 0;
    u8 status,cnt=0;
    u32 name;
    if(!strncmp((char*)cmd,"CH ", strlen("CH ")))
    {
        cmd+=strlen("CH ");
        sscanf((char *)cmd,"%x",&data);
    }
#if (TV_DECODER == WT8861)
 	if(data==1)
        name=I2C_WT8861_RD_SLAV_ADDR;
    else if(data==2)
        name=I2C_WT8861_RD_SLAV_2_ADDR;
	i2cRead_WT8861(0x3A,&status,name);  //check WT8861 is locked to the video signal, if no video signal input, max check 100
    while(((status&0x0F) != 0x0E) && (cnt < 10))
    {
        cnt++;
        i2cRead_WT8861(0x3A,&status,name);
    }
    if(cnt < 10)
    {
        DEBUG_UI("sysback WT8861 lock the video signal, get correctly TV in format\n");
        sysTVInFormatLocked = TRUE;
    }
    else
    {
        sysTVInFormatLocked = FALSE;
        DEBUG_UI("sysback WT8861 can't lock the video signal, use default TV in format-NTSC\n");
    }
#elif (TV_DECODER == TW9900)
    if(data==1)
        name=I2C_TW9900_RD_SLAV_ADDR;
//#if IS_COMMAX_DOORPHONE
#if IS_COMMAX_DOORPHONE	|| ((HW_BOARD_OPTION == MR6730_AFN) && (!MULTI_CH_DEGRADE_1CH) ) 
    else if(data==2)
        name=I2C_TW9900_RD_SLAV_2_ADDR;
#endif
    i2cRead_TW9900(0x01,&status,name);
 //   DEBUG_I2C("status=%d \n\n",status);
    while(((status&0x80) != 0x00) && (cnt < 10))
    {
        cnt++;
        i2cRead_TW9900(0x01,&status,name);
    }
    if(cnt < 10)
    {
        DEBUG_UI("sysback TW9900 lock the video signal, get correctly TV in format\n");
        if(data==1)
        sysTVInFormatLocked = TRUE;
        else if(data==2)
        sysTVInFormatLocked1 = TRUE;
    }
    else
    {
        DEBUG_UI("sysback TW9900 can't lock the video signal, use default TV in format-NTSC\n");
        if(data==1)
        sysTVInFormatLocked = FALSE;
        else if(data==2)
        sysTVInFormatLocked1 = FALSE;
    }
#endif

}
void uiTW9900_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x %x",&slaveAddr,&registAddr);
        i2cRead_TW9900(registAddr,&data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x %x",&slaveAddr,&registAddr,&data);
        i2cWrite_TW9900(registAddr,data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
}
void uiTW9910_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x %x",&slaveAddr,&registAddr);
        i2cRead_TW9910(registAddr,&data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x %x",&slaveAddr,&registAddr,&data);
        i2cWrite_TW9910(registAddr,data,slaveAddr);
        DEBUG_UI("SLAVE:%x, regist:%x, data:%x  \n",slaveAddr,registAddr,data);
    }
}
void uiBIT1201G_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        i2cRead_BIT1201G(registAddr, &data, I2C_BIT1201G_RD_SLAV_ADDR);

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_BIT1201G(registAddr, data, I2C_BIT1201G_WR_SLAV_ADDR);
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    }

}

#if (IO_EXPAND == IO_EXPAND_WT6853)
void uiWT6853_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        i2cRead_WT6853(registAddr, &data);

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_WT6853(registAddr, data);
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    }

}
#endif

#if (TOUCH_KEY == TOUCH_KEY_MA86P03)
void uiMA86P03_TestI2c(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        i2cRead_MA86P03(registAddr , &data );

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_MA86P03(registAddr , data );
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    }

}


#endif


#if (Sensor_OPTION == Sensor_HM1375_YUV601 )
void uiHM1375_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u16 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        #if(GPIO_I2C_ENA)
        i2cRead_HM1375(registAddr, &data);
        #else
        i2cRead_HM1375_Manual(registAddr, &data);
        #endif


        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_HM1375(registAddr, data);
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    }

}

#endif

#if (Sensor_OPTION == Sensor_NT99141_YUV601 )
void uiNT99141_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u16 registAddr=0;
    u8 data =0;

    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        i2cRead_NT99141_Manual(registAddr, &data);

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_NT99141(registAddr, data);
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);
    }

}

#endif

void uiCS8556_TestI2C(u8 *cmd)
{
    u8 slaveAddr=0;
    u8 registAddr=0;
    u8 data =0;
    u16 i=0;
    if(GPIO_I2C_ENA == 0)
        return ;
    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);

        i2cRead_CS8556(registAddr, &data, I2C_CS8556_RD_SLAV_ADDR);

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        i2cWrite_CS8556(registAddr, data, I2C_CS8556_WR_SLAV_ADDR);
        DEBUG_UI("wirte regist:%x, data:%x  \n",registAddr,data);

        i2cRead_CS8556(registAddr, &data, I2C_CS8556_RD_SLAV_ADDR);

        DEBUG_UI("read regist:%x, data:%x  \n",registAddr,data);
    }
    else if(!strncmp((char*)cmd,"DUMP", strlen("DUMP")))
    {
        cmd+=strlen("DUMP");
        DEBUG_UI("\nCS8556: \n");
        for(i=0;i<256;i++)
        {
            if(i%4 == 0)
            {
                DEBUG_UI("\n %x: ",i);
            }
            i2cRead_CS8556(i, &data, I2C_CS8556_RD_SLAV_ADDR);

            DEBUG_UI("%x ",data);


        }
        DEBUG_UI("\n");


    }

}
#if Melody_SNC7232_ENA
void uiKSNC7232_TestI2C(u8 *cmd)
{
    u8 registAddr=0;
    u16 data =0;
    u16 i=0;
    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x %x",&registAddr,&data);
        Melody_SNC7232_W_Byte(registAddr, data);
        DEBUG_UI("wirte cmd:%x, data:%x  \n",registAddr,data);

    }
    else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char *)cmd,"%x",&registAddr);
        Melody_SNC7232_R_Byte(registAddr, &data);
        DEBUG_UI("read cmd:%x, data:%x  \n",registAddr,data);

    }
    else if(!strncmp((char*)cmd,"INIT ", strlen("INIT ")))
    {
        Melody_SNC7232_init(registAddr, data);
        DEBUG_UI("uiKSNC7232_TestI2C Init \n");

    }
    else if(!strncmp((char*)cmd,"PLAY ", strlen("PLAY ")))
    {
        cmd+=strlen("PLAY ");
        sscanf((char *)cmd,"%d ",&data);
        Melody_SNC7232_Play(data);
        DEBUG_UI("uiKSNC7232_TestI2C play %d \n",data);

    }
    else if(!strncmp((char*)cmd,"STOP", strlen("STOP")))
    {
        Melody_SNC7232_Stop();
        DEBUG_UI("uiKSNC7232_TestI2C Stop \n");
    }
    else if(!strncmp((char*)cmd,"PAUSE", strlen("PAUSE")))
    {
        Melody_SNC7232_PAUSE();
        DEBUG_UI("uiKSNC7232_TestI2C PAUSE \n");
    }
    else if(!strncmp((char*)cmd,"START", strlen("START")))
    {
        Melody_SNC7232_Start();
        DEBUG_UI("uiKSNC7232_TestI2C START  \n");
    }
    else if(!strncmp((char*)cmd,"STARTALL", strlen("STARTALL")))
    {
        Melody_SNC7232_Start_ALL();
        DEBUG_UI("uiKSNC7232_TestI2C STARTALL  \n");
    }
    else if(!strncmp((char*)cmd,"DELAY ", strlen("DELAY ")))
    {
        cmd+=strlen("DELAY ");
        sscanf((char *)cmd,"%d ",&data);
        Melody_SNC7232_SETDELAY(data);
        DEBUG_UI("uiKSNC7232_TestI2C DELAY %d \n",data);
    }

}
#endif
#if(TV_ENCODER == CH7025)
void uiCS7025_TestI2C(u8 *cmd)
{
    u8 data;
    u8 reg;
    u8 i;
    if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char*)cmd, "%x", &reg);
        i2cRead_CH7025(reg, &data );
        DEBUG_UI("Reg:%x Data:%x \n",reg,data);

    }
    else if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char*)cmd,"%x %x",&reg,&data);
        i2cWrite_CH7025(reg,data);
    }
    else if(!strcmp((char*)cmd,"DUMP"))
    {
        for(i=0;i<255;i++)
        {
            if(i%4==0)
            {
                DEBUG_UI("\n 0x%02x: ",i);
            }
            i2cRead_CH7025(i, &data );
            DEBUG_UI("%02x ",data);
        }
    }
}
#endif

void uiTestAdc(u8 value)
{
    u8 adc_reg=0;
    s8 data=0;

    switch(value)
    {
        case 0:
            adc_reg=0xd00d0008;
            data=*((volatile unsigned *)(adc_reg));
            data=(data & 0x00ff0000) >>16;
            printf("ADC0: %d\n", data);
            break;
        case 1:
            adc_reg=0xd00d0008;
            data=*((volatile unsigned *)(adc_reg));
            data=(data & 0x00000fff);
            printf("ADC1: %d\n", data);
            break;
        case 2:
            adc_reg=0xd00d000C;
            data=*((volatile unsigned *)(adc_reg));
            data=(data & 0x00ff0000) >>16;
            printf("ADC2: %d\n", data);
            break;
        case 3:
            adc_reg=0xd00d000C;
            data=*((volatile unsigned *)(adc_reg));
            data=(data & 0x00000fff);
            printf("ADC3: %d\n", data);
            break;
    }
}


void uiBrightnessTest(u8* cmd)
{
    u32 Value;

    sscanf(cmd, "%d",&Value);
    DEBUG_UI("Brightness Value : %d \r\n",Value);
    uiMenuSet_TX_BRIGHTNESS(Value);
}

void uiIPNetworkTest(u8* cmd)
{
   UI_NET_INFO ipInfo;
    static UI_NET_INFO SetIp;
    u32  ip[4], i;

#if (NIC_SUPPORT == 1)
    DEBUG_UI("uiTimeTest %s\n", cmd);
    if (!strcmp((char*)cmd,"INFO"))
    {
        GetNetworkInfo(&ipInfo);
        DEBUG_UI("******************************\n");
        DEBUG_UI("IP Address       :%u.%u.%u.%u \n",ipInfo.IPaddr[0], ipInfo.IPaddr[1], ipInfo.IPaddr[2], ipInfo.IPaddr[3]);
        DEBUG_UI("Subnet Mask      :%u.%u.%u.%u \n",ipInfo.Netmask[0], ipInfo.Netmask[1], ipInfo.Netmask[2], ipInfo.Netmask[3]);
        DEBUG_UI("Default Getway   :%u.%u.%u.%u \n",ipInfo.Gateway[0], ipInfo.Gateway[1], ipInfo.Gateway[2], ipInfo.Gateway[3]);
        DEBUG_UI("******************************\n");
    }
    else if (!strncmp((char*)cmd,"SET IP ", strlen("SET IP ")))
    {
        cmd+=strlen("SET IP ");
        sscanf((char*)cmd, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_UI("IP Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                SetIp.IPaddr[i] = ip[i];
        }
        DEBUG_UI("Set IP Address  :%u.%u.%u.%u \n",SetIp.IPaddr[0], SetIp.IPaddr[1], SetIp.IPaddr[2], SetIp.IPaddr[3]);
    }
    else if (!strncmp((char*)cmd,"SET MASK ", strlen("SET MASK ")))
    {
        cmd+=strlen("SET MASK ");
        sscanf((char*)cmd, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_UI("Mask Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                SetIp.Netmask[i] = ip[i];
        }
        DEBUG_UI("Set Mask Address  :%u.%u.%u.%u \n",SetIp.Netmask[0], SetIp.Netmask[1], SetIp.Netmask[2], SetIp.Netmask[3]);
    }
    else if (!strncmp((char*)cmd,"SET GETWAY ", strlen("SET GETWAY ")))
    {
        cmd+=strlen("SET GETWAY ");
        sscanf((char*)cmd, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_UI("Getway Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                SetIp.Gateway[i] = ip[i];
        }
        DEBUG_UI("Set Getway Address  :%u.%u.%u.%u \n",SetIp.Gateway[0], SetIp.Gateway[1], SetIp.Gateway[2], SetIp.Gateway[3]);
    }
    else if (!strcmp((char*)cmd,"APPLY"))
    {
        memcpy(&UINetInfo, &SetIp, sizeof(UI_NET_INFO));
        DEBUG_UI("Set Enable DHCP %d\n", UINetInfo.IsStaticIP);
        Save_UI_Setting();
        DEBUG_UI("******************************\n");
        DEBUG_UI("Please Reboot ...............\n");
        DEBUG_UI("******************************\n");
        while (1);
    }
    else if (!strcmp((char*)cmd,"EN DHCP"))
    {
        SetIp.IsStaticIP = 0;
        DEBUG_UI("Set Enable DHCP\n");
    }
    else if (!strcmp((char*)cmd,"DIS DHCP"))
    {
        SetIp.IsStaticIP = 1;
        DEBUG_UI("Set Disable DHCP\n");
    }
#endif
}

#if (HW_BOARD_OPTION == MR8200_RX_COMMAX)
void uiCmdSetScheduleTime(u8 *cmd)
{
    u32 Sid, i, val;
    if (!strncmp((char*)cmd, "GET ", strlen("GET ")))
    {
        cmd+=strlen("GET ");
        sscanf((char*)cmd, "%d ", &Sid);
        DEBUG_UI("Schedule %d : 20%d/%d/%d (%d) form %d:%d to %d:%d\n",
            Sid, ScheduledTime[Sid].year, ScheduledTime[Sid].month, ScheduledTime[Sid].day,
            ScheduledTime[Sid].week,ScheduledTime[Sid].Shour, ScheduledTime[Sid].Smin,
            ScheduledTime[Sid].Ehour, ScheduledTime[Sid].Emin);
        DEBUG_UI("Camera ");
        for (i = 0; i < 4; i++)
        {
            val = (1 << i);
            if (ScheduledTime[i].channel & val == 1)
                DEBUG_UI("%d ",val);
        }
        DEBUG_UI("\nStatus %d\n",ScheduledTime[i].State);
    }
    else if (!strncmp((char*)cmd, "SET ", strlen("SET ")))
    {
        cmd+=strlen("SET ");
        sscanf((char*)cmd, "%d ", &Sid);
        cmd+=strlen("1 ");
        if (!strncmp((char*)cmd, "YEAR ", strlen("YEAR ")))
        {
            sscanf((char*)cmd, "YEAR %d ", &val);
            DEBUG_UI("Get Schedule %d Year %d\n", Sid, val);
            ScheduledTime[Sid].year = val;
        }
        else if (!strncmp((char*)cmd, "MONTH ", strlen("MONTH ")))
        {
            sscanf((char*)cmd, "MONTH %d ", &val);
            DEBUG_UI("Get Schedule %d MONTH %d\n", Sid, val);
            ScheduledTime[Sid].month= val;
        }
        else if (!strncmp((char*)cmd, "DAY ", strlen("DAY ")))
        {
            sscanf((char*)cmd, "DAY %d ", &val);
            DEBUG_UI("Get Schedule %d DAY %d\n", Sid, val);
            ScheduledTime[Sid].day= val;
        }
        else if (!strncmp((char*)cmd, "WEEK ", strlen("WEEK ")))
        {
            sscanf((char*)cmd, "WEEK %d ", &val);
            DEBUG_UI("Get Schedule %d WEEK %d\n", Sid, val);
            ScheduledTime[Sid].week= val;
        }
        else if (!strncmp((char*)cmd, "SHOUR ", strlen("SHOUR ")))
        {
            sscanf((char*)cmd, "SHOUR %d ", &val);
            DEBUG_UI("Get Schedule %d SHOUR %d\n", Sid, val);
            ScheduledTime[Sid].Shour= val;
        }
        else if (!strncmp((char*)cmd, "SMIN ", strlen("SMIN ")))
        {
            sscanf((char*)cmd, "SMIN %d ", &val);
            DEBUG_UI("Get Schedule %d SMIN %d\n", Sid, val);
            ScheduledTime[Sid].Smin= val;
        }
        else if (!strncmp((char*)cmd, "EHOUR ", strlen("EHOUR ")))
        {
            sscanf((char*)cmd, "EHOUR %d ", &val);
            DEBUG_UI("Get Schedule %d EHOUR %d\n", Sid, val);
            ScheduledTime[Sid].Ehour= val;
        }
        else if (!strncmp((char*)cmd, "EMIN ", strlen("EMIN ")))
        {
            sscanf((char*)cmd, "EMIN %d ", &val);
            DEBUG_UI("Get Schedule %d EMIN %d\n", Sid, val);
            ScheduledTime[Sid].Emin= val;
        }
        else if (!strncmp((char*)cmd, "STATUS ", strlen("STATUS ")))
        {
            sscanf((char*)cmd, "STATUS %d ", &val);
            DEBUG_UI("Get Schedule %d STATUS %d\n", Sid, val);
            ScheduledTime[Sid].State = val;
        }
    }

}
#endif
void uiPanelGammaSetting_Y(u8 *cmd)
{
    u8 level;
    u8 data;
    u32 gammaData;
    if (!strncmp((char*)cmd, "R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char*)cmd, "%d", &level);

        if(level<5)
        {
            level-=1;
            gammaData=(IduGammaY0>>(level*8))&0xff;
            DEBUG_UI("GAMMADATAy %d=%x %x\n",level,gammaData,IduGammaY0);
            level+=1;
        }
        else
        {
            level-=5;
            gammaData=(IduGammaY1>>(level*8))&0xff;
            DEBUG_UI("GAMMADATAy %d=%x %x\n",level,gammaData,IduGammaY1);
            level+=5;
        }
     //  DEBUG_UI("GAMMADATA %d=%x \n",level,gammaData);
    }
    else if (!strncmp((char*)cmd, "W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char*)cmd, "%d %x", &level, &data);
        if(level<5)
        {
            level-=1;

            gammaData=(IduGammaY0>>(level*8))&0x00;
            gammaData|=data;
            gammaData=gammaData<<(level*8);
			switch(level)
			{
				case 0:
					IduGammaY0&=0xffffff00;
				break;
				case 1:
					IduGammaY0&=0xffff00ff;
				break;
				case 2:
					IduGammaY0&=0xff00ffff;
				break;
				case 3:
					IduGammaY0&=0x00ffffff;
				break;
			}
            IduGammaY0|=gammaData;
            level+=1;

       //     DEBUG_UI("GAMMADATAy %d=%x %x\n",level,gammaData,IduGammaY0);
        }
        else
        {
            level-=5;

            gammaData=(IduGammaY1>>(level*8))&0x00;
            gammaData|=data;
            gammaData=gammaData<<(level*8);
			switch(level)
			{
				case 0:
					IduGammaY1&=0xffffff00;
				break;
				case 1:
					IduGammaY1&=0xffff00ff;
				break;
				case 2:
					IduGammaY1&=0xff00ffff;
				break;
				case 3:
					IduGammaY1&=0x00ffffff;
				break;
			}
            IduGammaY1|=gammaData;
       //     DEBUG_UI("GAMMADATAy %d=%x %x\n",level,gammaData,IduGammaY1);
            level+=5;
        }
    }
}

void uiPanelGammaSetting_X(u8 *cmd)
{
    u8 level;
    u16 data;
    u32 gammaData;
	u32 gamma_zero;
    if (!strncmp((char*)cmd, "R ", strlen("R ")))
    {
        cmd+=strlen("R ");
        sscanf((char*)cmd, "%d", &level);

        if(level<4)
        {
        //    level-=1;
            gammaData=(IduGammaX0>>(level*8))&0xff;
            DEBUG_UI("GAMMADATAx %d=%x %x\n",level,gammaData,IduGammaX0);
       //     level+=1;
        }
        else
        {
            level-=4;
            gammaData=(IduGammaX1>>(level*8))&0xff;
            DEBUG_UI("GAMMADATAx %d=%x %x\n",level,gammaData,IduGammaX1);
            level+=4;
        }
     //  DEBUG_UI("GAMMADATA %d=%x \n",level,gammaData);
    }
    else if (!strncmp((char*)cmd, "W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char*)cmd, "%d %x", &level, &data);
        if(level<4)
        {
         //   level-=1;
            gammaData=(IduGammaX0>>((level)*8))&0x00;
            gammaData|=data;
            gammaData=gammaData<<((level)*8);
			switch(level)
			{
				case 1:
					IduGammaX0&=0xffff00ff;
				break;
				case 2:
					IduGammaX0&=0xff00ffff;
				break;
				case 3:
					IduGammaX0&=0x00ffffff;
				break;
			}
            IduGammaX0|=gammaData;
         //   level+=1;
        //    DEBUG_UI("GAMMADATAx %d=%x %x\n",level,gammaData,IduGammaX0);
        }
        else
        {
            level-=4;

            gammaData=(IduGammaX1>>(level*8))&0x00;
            gammaData|=data;
            gammaData=gammaData<<(level*8);
			switch(level)
			{
				case 0:
					IduGammaX1&=0xffffff00;
				break;
				case 1:
					IduGammaX1&=0xffff00ff;
				break;
				case 2:
					IduGammaX1&=0xff00ffff;
				break;
				case 3:
					IduGammaX1&=0x00ffffff;
				break;
			}
            IduGammaX1|=gammaData;
         //   DEBUG_UI("GAMMADATAx %d=%x %x\n",level,gammaData,IduGammaX1);
            level+=4;
        }
    }
}

void uiSetMAC_To_Flash(u8 *cmd)
{
    u8 mac1,mac2,mac3,mac4,mac5,mac6;
    u8 readdata[6]={0};

    sscanf((char*)cmd, "%02X-%02X-%02X-%02X-%02X-%02X", &mac1, &mac2, &mac3, &mac4, &mac5, &mac6);

    uiMACAddr[0]=mac1;
    uiMACAddr[1]=mac2;
    uiMACAddr[2]=mac3;
    uiMACAddr[3]=mac4;
    uiMACAddr[4]=mac5;
    uiMACAddr[5]=mac6;
    spiWriteNet();

    spiReadData(SPIConfigBuf, spiNetwrokStartAddr, SPI_UI_NETWORK_INFO_SIZE);
    memcpy(&readdata,SPIConfigBuf+UI_SETUP_GUID_SIZE,sizeof(readdata));

    if(!strncmp(uiMACAddr,readdata,strlen(uiMACAddr)) )
    {
        printf("RMAC %02X-%02X-%02X-%02X-%02X-%02X\n",readdata[0],readdata[1],readdata[2],readdata[3],readdata[4],readdata[5]);
    }
    else
    {
        printf("RMAC FAIL\n");
    }
#if(UI_VERSION == UI_VERSION_TRANWO)   
    memset(uiRFID,0,sizeof(uiRFID));
    spiWriteRF();
#endif

}

void uiSetUID_To_Flash(u8 *cmd)
{
    u8 readdata[21]={0};

    sscanf((char*)cmd, "%s", &uiP2PID);
    spiWriteNet();

   	spiReadData(SPIConfigBuf, spiNetwrokStartAddr, SPI_UI_NETWORK_INFO_SIZE);
    memcpy(&readdata,SPIConfigBuf,sizeof(readdata));
    if(!strncmp(uiP2PID,readdata,strlen(uiP2PID)) )
    {
        printf("RUID %s\n",readdata);
    }
    else
    {
        printf("RUID FAIL\n");
    }
#if(UI_VERSION == UI_VERSION_TRANWO)      
    memset(uiRFID,0,sizeof(uiRFID));
    spiWriteRF();
#endif

}

#if(HWPIP_SUPPORT)

void HWPIP_OpenTest(u8 *cmd) // KHPO
{
    u8 ch;

    sscanf((char*)cmd, "%d",&ch);
    if(HWPIP_CH_Sel(ch) == 1) 
        DEBUG_IDU("Set CIU%d to PIP addr \r\n",ch);
    else
    {
        DEBUG_IDU("Set OSD addr error!! \r\n");
        return;
    }
    HWPIP_Open(1,0,0,1,1,1,1);
    HWPIP_Position(320,120);
    HWPIP_OSDSize(320,120);
    OSD_STRIDE   = 0x00000280;    
    HWPIP_EdgeColorSet(0x8080FFFF);
    
}

void HWPIP_Close() // KHPC
{
    DEBUG_IDU("HWPIP_Close \r\n");
    OSD_ADDR_Y   = 0x0;
    OSD_ADDR_C   = 0x0;
    OSD_STRIDE   = 0x0;
    OSD_IN_WIDTH = 0x0;
    OSD_END_X    = 0x0;
    OSD_START_X  = 0x0;
    OSD_EDGE_COLOR = 0x0;
    BRI_CTRL_REG &= 0x3F;
}

void HWPIP_PositionTest(u8 *cmd) //KHPP
{
    u32 S_X,S_Y;
    u8 sel;

    sscanf((char*)cmd, "%d %d", &S_X, &S_Y);
    DEBUG_IDU("Set OSD_START_X (%d,%d)\r\n",S_X, S_Y);
    HWPIP_Position(S_X,S_Y);
}

void HWPIP_SizeTest(u8 *cmd) //KHPS
{
    u32 IN_X,IN_Y;
    u8 sel;

    sscanf((char*)cmd, "%d %d", &IN_X, &IN_Y);
    DEBUG_IDU("Set OSD_IN_WIDTH (%d,%d)\r\n",IN_X, IN_Y);
    HWPIP_OSDSize(IN_X,IN_Y);
}

void HWPIP_AlphaBlendingTest(u8 *cmd) //KHPA
{
    u8 Blending;

    sscanf((char*)cmd, "%d", &Blending);
    HWPIP_AlphaBlendingSet(Blending);
}

void HWPIP_DownSampleTest(u8 *cmd) //KHPD
{
    u8 Downsample;

    sscanf((char*)cmd, "%d", &Downsample);
    if(HWPIP_DownSampleSet(Downsample))
        DEBUG_IDU("Seting down sample is %d",Downsample);
   
}

void HWPIP_EdgeColorSetTest(u8 *cmd) //KHPE
{
    u32 color;

    sscanf((char*)cmd, "%x", &color);
    DEBUG_IDU("Seting [OSD_Edge_Color] = %8X \r\n ",color);
    HWPIP_EdgeColorSet(color);
}

void HWPIP_EdgeColorEnTest(u8 *cmd) //KHPES
{
    u8 Edge_En;

    sscanf((char*)cmd, "%d", &Edge_En);
    HWPIP_EdgeColorEn(Edge_En);
}

void HWPIP_BurstBitwidthSetTest(u8 *cmd) //KHPB
{
    u8 B8_OSD,B8_VDO,BW_64;

    sscanf((char*)cmd, "%d %d %d", &B8_OSD,&B8_VDO,&BW_64);
    DEBUG_IDU("Seting [Burst_8_OSD] = %d \r\n ",B8_OSD);
    DEBUG_IDU("Seting [Burst_8_Video] = %d \r\n ",B8_VDO);
    DEBUG_IDU("Seting [Bitwidth64_off] = %d \r\n ",BW_64);
    HWPIP_BurstBitwidthSet(B8_OSD,B8_VDO,BW_64);
}
#endif






#if(HW_BOARD_OPTION == MR6730_AFN)
#include "..\..\ui\inc\Key.h"
extern Key KEYIR_MSG;


#include "Menu.h"
#include "MenuID.h"
extern Menu g_menus[];
//extern uint32_t RecFileLen[];
extern u8 mpeg4VideoRecQulity;
extern u8 Current_REC_Mode;
extern u32 asfSectionTime;
extern u32 sysCaptureVideoMode;
extern s8	OverwriteStringEnable;
//
extern u8  MotionDetect_en;
extern u32	MD_period_Preview;
extern u32	MD_period_Video;

extern u32* pMD_Thr_PixelDiff ;      
extern u32* pMD_Thr_BlockMargin;

extern u8 uiMenuVideoSizeSetting;




#if (USE_UART_CMD_CTRL) 








u8 gDmsg_Enabled=0;
u32 gDmsg_Password=DMSG_PASSWORD;


void ui_DMSG_Cmd(u8 *cmd) 
{//debug message subset script of  'K' command
	
		
		u8 u8val;
		u16 u16val;
		u32 u32val;
		




		
		if(strncmp((char*)cmd, "EN", strlen("EN"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "EN ", strlen("EN "))==0)
			{
				cmd+=strlen("EN ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u32val);
				#if 1
				DEBUG_DMSG("Dmsg password: %.*s \r\n",strlen(cmd),"**********");
				#else
				DEBUG_DMSG("Dmsg password: %*d \r\n",u32val);
				#endif
				if(u32val==gDmsg_Password)
				{
					gDmsg_Enabled=1;//enabled
					DEBUG_DMSG("Enable DMSG functions.\r\n");
				}
				else
				{				
					gDmsg_Enabled=0;//disbled
					if(u32val)
						DEBUG_DMSG("(%d):incorrect password.\r\n",u32val);
					else
						DEBUG_DMSG("Disable DMSG functions.\r\n");
				}
				//DEBUG_DMSG("%s DMSG \r\n",(gDmsg_Enabled)?"ENABLE":"DISABLE");

				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("DMSG Enabled:%d (%s)\r\n",gDmsg_Enabled,(gDmsg_Enabled)?"ENABLE":"DISABLE");
			}
	
		}else
		if(!gDmsg_Enabled)
		{
			DEBUG_DMSG("[= Input password to enable DMSG functions! =]\r\n");
			return;
		}else
		









		#if (USE_UI_APP_TASK)		
		if(strncmp((char*)cmd, "PRINT", strlen("PRINT"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "PRINT ", strlen("PRINT "))==0)
			{
				cmd+=strlen("PRINT ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("Runtime message level: %d \r\n",u8val);
				
			#if 1
				//only support 2 level 0:silence/1:verbose presently
				sysRuntimeUartMsg=(u8val)?1:0;
				DEBUG_DMSG("%s DMSG PRINT\r\n",(sysRuntimeUartMsg)?"ENABLE":"DISABLE");
			#else
				sysRuntimeUartMsg=(u8val&0x0F);
				DEBUG_DMSG("DMSG Level=%d\r\n",sysRuntimeUartMsg);
			#endif
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("DMSG PRINT Enabled:%d (%s mode)\r\n",sysRuntimeUartMsg,(sysRuntimeUartMsg)?"verbose":"silence");
			}
	
		}else
		#endif//#if (USE_UI_APP_TASK)





	
		if (!strncmp((char*)cmd, "FWVER", strlen("FWVER")))
		{
			DEBUG_DMSG("\n");
			DEBUG_DMSG("MODEL TYPE:%d , FW VER:v%d.%d.%d",HW_DERIV_MODEL,gFW_VER[0],gFW_VER[1],gFW_VER[2]);
		#if(!FW_RELEASED)
			//under development only
			//show extended version
			DEBUG_DMSG(".%d ",gFW_VER[3]);
		#endif //#if(!FW_RELEASED)
			DEBUG_DMSG("\n");
		
		}else
	

	
		if (!strncmp((char*)cmd, "SWRST ", strlen("SWRST ")))
		{
			cmd+=strlen("SWRST ");

			sscanf(cmd, "%d", &u16val);
			DEBUG_DMSG("SW-Reset: %d \r\n", u16val);
			
			_APP_ENTER_CS_; 	
			gApp_Var.UN.MB.SWRST_RestSec=u16val;
			_APP_EXIT_CS_;
			if(gApp_Var.UN.MB.SWRST_RestSec)
			{
			DEBUG_DMSG("SW Reset after %d sec.\n\n",gApp_Var.UN.MB.SWRST_RestSec);
			}
			else
			{
			DEBUG_DMSG("SW Reset canceled.(%d)\n\n",gApp_Var.UN.MB.SWRST_RestSec);	
			}
		}else



		
		//DIAG sub command set ----------------------------------------------------------------------
#if 1	//DMSG_DIAG_CMD

		if (!strncmp((char*)cmd, "DIAG ", strlen("DIAG ")))
		{
			u8 DevAddr=0;
			u8 RegAddr=0;		
			u8 data=0;
			volatile unsigned long *pReg=0;
			u32 Reg=0;
			u32 dwdata=0;	
			cmd    += strlen("DIAG ");
		#if 0
			if(!strncmp((char*)cmd,"I2CR ", strlen("I2CR ")))
			{
				cmd += strlen("I2CR ");
				sscanf((char *)cmd, "%x %x", &DevAddr, &RegAddr);
				i2cRead_Byte(DevAddr, RegAddr, &data);
				DEBUG_DMSG("Device:%x, regist:%x, data:%x\n",DevAddr,RegAddr,data);
			}else 
			if(!strncmp((char*)cmd,"I2CW ", strlen("I2CW ")))
			{
				cmd   += strlen("I2CW ");
				sscanf((char *)cmd,"%x %x %x", &DevAddr, &RegAddr, &data);
		
				i2cWrite_Byte(DevAddr, RegAddr, data);
				DEBUG_DMSG("Device:%x, regist:%x, data:%x\n",DevAddr,RegAddr,data);
			}else
		#endif
		#if 1
			if(!strncmp((char*)cmd,"REGR ", strlen("REGR ")))
			{
				cmd += strlen("REGR ");
				sscanf((char *)cmd, "%x", &Reg);
				pReg=(volatile unsigned long *)Reg;
				dwdata=*pReg;
				DEBUG_DMSG("read MCU REG[0x%08X]:0x%08X\n",Reg,dwdata);
			}else 
			if(!strncmp((char*)cmd,"REGW ", strlen("REGW ")))
			{
				cmd   += strlen("REGW ");
				sscanf((char *)cmd,"%x %x", &Reg, &dwdata);
				pReg=(volatile unsigned long *)Reg;
				*pReg=dwdata;
				DEBUG_DMSG("write MCU REG[0x%08X]=0x%08X\n",Reg,dwdata);
			}else
		#endif 
	
		#if 1
		if(strncmp((char*)cmd, "SYSPW", strlen("SYSPW"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "SYSPW ", strlen("SYSPW "))==0)
			{
				cmd+=strlen("SYSPW ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("SYSPW>>%d \r\n",u8val);
				
				if(u8val>2)	u8val=1;
				setUI.SYS_PowerOff=u8val;
				
				#if 1
				if(setUI.SYS_PowerOff==0)
				{
					INF(("power-on"));
					OSTimeDlyHMSM(0,0,0,50);//delay for message print-out	

					Power_On();//raise power enabled pin
					
					#if 1
					//turn on LED indicator
						#if(LED_INDIC_SPEC_TYPE==0)	
						setUI.LEDOnOff = 1;  //LED on		  
						#else
						setUI.LEDOnOff = 0;  //LED off	
						#endif
						LED_OnOff(setUI.LEDOnOff);	
					#endif
					//OSTimeDlyHMSM(0,0,5,0);//debug

					#if 0//(PWKEY_PWRCTRL)	//can't undo
					DEBUG_UI("POWER ON. \r\n");		
					//OSTimeDlyHMSM(0,0,5,0);//debug
					//...
					#endif
					
					//turn-on Display(TV-OUT) later
					//...
					
				}
				#endif	
				
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{//read current recording status
				DEBUG_DMSG("SYSPW(%d)\r\n",setUI.SYS_PowerOff);
			}
	
		}else	
		#endif

	#if 1
		if(strncmp((char*)cmd, "IOTEST", strlen("IOTEST"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "IOTEST ", strlen("IOTEST "))==0)
			{
				cmd+=strlen("IOTEST ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("IOTEST>>%d \r\n",u8val);
				
					if(u8val)
					{
						//if(!MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_DIAG_IOTEST))
						{
							MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_DIAG_IOTEST); 
						}
					}
					else
					{
						//if(MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_DIAG_IOTEST))
						{
							MACRO_UI_SET_SYSSTAT_BIT_CLR(UI_SET_SYSSTAT_BIT_DIAG_IOTEST); 
						}
					}
					
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{//read current recording status
				DEBUG_DMSG("IOTEST enabled=%d\r\n",(MACRO_UI_DIAG_IOTEST_CHK()?1:0));				
			}	
		}else		
	#endif
	
	#if (defined(IO_GRP_UIDISP)&&defined(IO_PIN_UIDISP)&&defined(IO_PORT_UIDISP))//(USE_UI_DISP_OUT)
		if(strncmp((char*)cmd, "UIO", strlen("UIO"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "UIO ", strlen("UIO "))==0)
			{
				cmd+=strlen("UIO ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("UIO>>%d \r\n",u8val);
				
					if(u8val)//logical,(1:ON / 0:OFF)
					{
						//UiDisp_OnOff(UI_DISP_ON);
						UiDisp_OnOff(1);
					}
					else
					{
						//UiDisp_OnOff(UI_DISP_OFF);
						UiDisp_OnOff(0);
					}

					
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{//read current recording status
				//DEBUG_DMSG("UIO(%d)\r\n",(IsUiDispOn()?1:0));
				DEBUG_DMSG("UIO(%d.%d active %s)=%d\r\n",UI_DISP_PIN_GRP,UI_DISP_PIN,(UI_DISP_LOW_ACTIVATE)?"low":"high",(IsUiDispOn()?1:0));
			}
	
		}else	
	#endif

	#if(USE_AUDIO_PATH_SW)
	if(strncmp((char*)cmd, "AUSW", strlen("AUSW"))==0)
	{
		u8 show_sv=1;
		if (strncmp((char*)cmd, "AUSW ", strlen("AUSW "))==0)
		{
			cmd+=strlen("AUSW ");
			if(*cmd)
			{
			sscanf(cmd, "%d",&u8val);
			DEBUG_DMSG("AUSW>>%d \r\n",u8val);
				
				if(u8val)//logical,(1:ON / 0:OFF)
				{
					//AuPathSw_OnOff(AUPATH_BYPASS_ON);
					AuPathSw_OnOff(1);
				}
				else
				{
					//AuPathSw_OnOff(AUPATH_BYPASS_OFF);
					AuPathSw_OnOff(0);
				}

				
			show_sv=0;
			}	
		}

		if(show_sv)
		{//read current recording status
			DEBUG_DMSG("AUSW(%d.%d active %s)=%d\r\n",AUPATH_SW_PIN_GRP,AUPATH_SW_PIN,(AUPATH_SW_LOW_ACTIVATE)?"low":"high",(IsAuPathSw()?1:0));
			//DEBUG_DMSG("AUSW(%d.%d active %s)=%d\r\n",AUPATH_SW_PIN_GRP,AUPATH_SW_PIN,(AUPATH_SW_LOW_ACTIVATE)?"low":"high",(IsAuPathSw()==AUPATH_BYPASS_ON)?1:0);		
		}

	}else	
	#endif//#if(USE_AUDIO_PATH_SW)

	
	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)
		
	if(strncmp((char*)cmd, "USB_DET", strlen("USB_DET"))==0)
	{
		u8 show_sv=1;
	#if 0
		if (strncmp((char*)cmd, "USB_DET ", strlen("USB_DET "))==0)
		{
			cmd+=strlen("USB_DET ");
			if(*cmd)
			{
			sscanf(cmd, "%d",&u8val);
			DEBUG_DMSG("USB_DET>>%d \r\n",u8val);
				
				if(u8val)//logical,(1:ON / 0:OFF)
				{
					//DO_Ctrl_OnOff(1);
				}
				else
				{
					//DO_Ctrl_OnOff(0);
				}

				
			show_sv=0;
			}	
		}
	#endif
		if(show_sv)
		{//read current recording status
			DEBUG_DMSG("USB_DET(%d.%d active %s)=%d\r\n",USB_DET_PIN_GRP,USB_DET_PIN,(USB_DET_LOW_ACTIVATE)?"low":"high",(IsUsbDetIn()?1:0));		
		}

	}else	
	#endif

	
	#if 0
		//User GPIO test
		if(!strncmp((char*)cmd,"UIOR ", strlen("UIOR ")))
		{		
			u8 data=0;
			u8 Val=0;
	
			cmd += strlen("UIOR ");
			sscanf((char *)cmd, "%d", &Val);
			switch(Val)
			{
			case 1://UIDP:GPIO0.7(p194)
				{
					data=(IsUiDispOn()?1:0);
				}
				break;
			}
			DEBUG_DMSG("read USER IO[0x%02X]:0x%02X\n",Val,data);
		}else 
		if(!strncmp((char*)cmd,"UIOW ", strlen("UIOW ")))
		{
			u8 data=0;
			u8 Val=0;
	
			cmd   += strlen("UIOW ");
			sscanf((char *)cmd,"%d %x", &Val, &data);
			switch(Val)
			{
			case 1://UIDP:GPIO0.7(p194)
				{
					if(data)
					{
						UiDisp_OnOff(UI_DISP_ON);
					}
					else
					{
						UiDisp_OnOff(UI_DISP_OFF);
					}
				}
				break;
			}
	
			DEBUG_DMSG("write USER IO[0x%02X]=0x%02X\n",Val,data);
		}
	#endif	
	




	

		

	#if(SIMU_RTM_STOP)	
		if(strncmp((char*)cmd, "SIMU_RTMSTOP", strlen("SIMU_RTMSTOP"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "SIMU_RTMSTOP ", strlen("SIMU_RTMSTOP "))==0)
			{
				cmd+=strlen("SIMU_RTMSTOP ");
				if(*cmd)
				{
				sscanf((char *)cmd,"%x", &data);
	
				gStopRTM_Simu=data;
				DEBUG_DMSG("SIMU_RTMSTOP(0x%02X)\n",data);
				
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("SIMU_RTMSTOP:0x%02X\r\n",gStopRTM_Simu);
			}
		}else
	#endif //#if(SIMU_RTM_STOP)
	#if (SIMU_RTM_FAIL)	
		if(strncmp((char*)cmd, "SIMU_RTMFAIL", strlen("SIMU_RTMFAIL"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "SIMU_RTMFAIL ", strlen("SIMU_RTMFAIL "))==0)
			{
				cmd+=strlen("SIMU_RTMFAIL ");
				if(*cmd)
				{
				sscanf((char *)cmd,"%x", &data);
	
				if(data&0x01)
				{
					//simulate the Ch1 abnormal terminated
					MultiChannelSysCaptureVideoStopOneCh(VIDEO_CH1);
				}
				if(data&0x02)
				{
					//simulate the Ch2 abnormal terminated
					MultiChannelSysCaptureVideoStopOneCh(VIDEO_CH2);			
				}
				DEBUG_DMSG("SIMU_RTMFAIL(0x%02X)\n",data);
	
				
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("SIMU_RTMFAIL:0x%02X\r\n",setUI.RecStatus);
			}
		}else
	#endif

	
	#if 1
		if(strncmp((char*)cmd, "PWRDET", strlen("PWRDET"))==0)	
		{
			u8 show_sv=1;
			u8 OnOff=0;
			if (strncmp((char*)cmd, "PWRDET ", strlen("PWRDET "))==0)
			{
				cmd+=strlen("PWRDET ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				OnOff=(IsPwrDet())?1:0; 	
				DEBUG_DMSG("can't set input pin PWRDET=%d \r\n",OnOff);
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				/*
				u8 OnOff=0;
				u8 SigLvl=0;
				OnOff=(IsPwrDet())?1:0; 			
			#if (PWR_ENB_LOW_ACTIVATE)
				SigLvl=(OnOff)?0:1;
			#else
				SigLvl=(OnOff)?1:0;
			#endif
				DEBUG_DMSG("PWRDET=%d (%s)\r\n",OnOff,(SigLvl)?"H":"L");
				*/
				OnOff=(IsPwrDet())?1:0; 			
				DEBUG_DMSG("PWRDET=%d (%s)\r\n",OnOff,(OnOff==PWR_DET_ON)?"H":"L"); 			
			}
		}else
		if(strncmp((char*)cmd, "PWREN", strlen("PWREN"))==0)	
		{
			u8 show_sv=1;
			u8 OnOff=0;
			if (strncmp((char*)cmd, "PWREN ", strlen("PWREN "))==0)
			{
				cmd+=strlen("PWREN ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				
				OnOff=(u8val)?1:0; 
				DEBUG_DMSG("set PWREN=%d (%s)\r\n",OnOff,(OnOff==PWR_ENB_ON)?"H":"L");
				
	
				PwrEn_OnOff((u8val)?1:0);// 1: On , 0: Off
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				OnOff=(IsPwrEn())?1:0;			
				DEBUG_DMSG("PWREN=%d (%s)\r\n",OnOff,(OnOff==PWR_ENB_ON)?"H":"L");			
			}
		}else
	#endif
	
	
	#if(USE_PWR_ONOFF_KEY)	
		if(strncmp((char*)cmd, "PWKEY", strlen("PWKEY"))==0)	
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "PWKEY ", strlen("PWKEY "))==0)
			{
				cmd+=strlen("PWKEY ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("can't set input pin PWKEY=%d \r\n",u8val);
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("PWKEY=%d (%s)\r\n",(IsPwKey())?PW_KEY_ON:PW_KEY_OFF,(IsPwKey())?"ON":"OFF");
			}
	
		}else
	
		
		if(strncmp((char*)cmd, "PWKEEP", strlen("PWKEEP"))==0)	
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "PWKEEP ", strlen("PWKEEP "))==0)
			{
				cmd+=strlen("PWKEEP ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				//DEBUG_DMSG("set PWKEEP =%d \r\n",(u8val)?PW_KEEP_ON:PW_KEEP_OFF);
				DEBUG_DMSG("set PWKEEP=%d (%s)\r\n",(u8val)?PW_KEEP_ON:PW_KEEP_OFF,(u8val)?"ON":"OFF");
				
	
				PwKeep_OnOff((u8val)?1:0);// 1: On , 0: Off
	
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
			//DEBUG_DMSG("PWKEEP=%d \r\n",(IsPwKeep())?PW_KEEP_ON:PW_KEEP_OFF);
				DEBUG_DMSG("PWKEEP=%d (%s)\r\n",(IsPwKeep())?PW_KEEP_ON:PW_KEEP_OFF,(IsPwKeep())?"ON":"OFF");
			}
	
		}else	
	#endif//#if(USE_PWR_ONOFF_KEY)
	
	
	
		//...
	
		
			MyHandler.WhichKey = UI_KEY_NONE;
		
		}else	
		
#endif ////DMSG_DIAG_CMD
	//DIAG sub command set ----------------------------------------------------------------------




	




		
	

	

	
	
	
	
	
	//extension commands ------------------------------------------------------------------------
#if 1	//DMSG_EXT_CMD

	#if (EXT_IR_CODE)//debug only

		if(strncmp((char*)cmd, "IR_CODE", strlen("IR_CODE"))==0)	
		{
			u8 show_sv=1;
			#if 1
			//debug only
			if (strncmp((char*)cmd, "IR_CODE ", strlen("IR_CODE "))==0)
			{
				cmd+=strlen("IR_CODE ");
				if(*cmd)
				{
					u8 TblIdx;
					u8 BytVal;
					sscanf(cmd, "%d %X",&TblIdx,&BytVal);

					if(TblIdx<IR_CODE_DAT_MAX)
					{
						gIrCodeDat._total[TblIdx]=BytVal;
						DEBUG_DMSG("set gIrCodeDat[%d] = 0x%02X \r\n",TblIdx,BytVal);
					}
					else
					{
						if(TblIdx==0xFF)
						{
						//init it again							
							hwIrInit_Ex(NULL);
							DEBUG_DMSG("hwIrInit(0x%4X)\n",IrCodeData.CustomCode);
						}
						else
							DEBUG_DMSG("index overrange.\n");
					}
	
				show_sv=0;
				}	
			}
			#endif
			if(show_sv)
			{
				u8 idx=0;
				DEBUG_DMSG("Ir-CustomCode=0x%04X\n",IrCodeData.CustomCode);
				for(idx=0;idx<IrKeyId_Num;idx++)
				{
					DEBUG_DMSG("Ir-KeyCode[%d]:0x%02X\n",idx,IrCodeData.KeyCode[idx]);
				}			
			}
	
		}else
	#endif//#if (EXT_IR_CODE)




	#if (CIU_OSD_METHOD_2)
		if(strncmp((char*)cmd, "COSDPOSAUTO", strlen("COSDPOSAUTO"))==0)	
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "COSDPOSAUTO ", strlen("COSDPOSAUTO "))==0)
			{
				cmd+=strlen("COSDPOSAUTO ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("set COSD2bPosAutoChange = %d \r\n",u8val);
				
	
				gCOSD2bPosAutoChange=(u8val)?1:0;
				DEBUG_DMSG("%s COSD2bPosAutoChange \r\n",(gCOSD2bPosAutoChange)?"ENABLE":"DISABLE");
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("COSD2bPosAutoChange Enabled=%d (%s)\r\n",gCOSD2bPosAutoChange,(gCOSD2bPosAutoChange)?"ENABLE":"DISABLE");
			}
	
		}else
	
		
		if(strncmp((char*)cmd, "COSDPOS", strlen("COSDPOS"))==0)
		{
		u8 show_sv=1;
			if (strncmp((char*)cmd, "COSDPOS ", strlen("COSDPOS "))==0)
			{
				cmd+=strlen("COSDPOS ");
				if(*cmd)
				{	
				int x1,y1;	
				int x2,y2;
	
					sscanf((char *)cmd, "%d %d %d %d", &x1, &y1, &x2, &y2);
	
					COSD2b_X1=x1;
					COSD2b_Y1=y1;
					COSD2b_X2=x2;
					COSD2b_Y2=y2;
					DEBUG_DMSG("set COSDPOS:[%d,%d,%d,%d]\n",COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("COSDPOS:%d,%d,%d,%d \r\n", COSD2b_X1,COSD2b_Y1,COSD2b_X2,COSD2b_Y2);
			}				
		}else 
	#endif
	
	#if (CIU_BOB_MODE)
		if(strncmp((char*)cmd, "MD_BOB_SHOW", strlen("MD_BOB_SHOW"))==0)
		{
			u8 show_sv=1;
			
			if (strncmp((char*)cmd, "MD_BOB_SHOW ", strlen("MD_BOB_SHOW "))==0)
			{		
				cmd+=strlen("MD_BOB_SHOW ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
	
				g_ShowBobOnOSD=(u8val)?1:0; 		
				DEBUG_DMSG("%s ShowBobOnOSD \r\n",(g_ShowBobOnOSD)?"ENABLE":"DISABLE");
	
	
				show_sv=0;
				}					
			}
			////
			if(show_sv)
			{			
				DEBUG_DMSG("ShowBobOnOSD=%d \r\n", g_ShowBobOnOSD);
			}
		}else
	#endif
		
		
		if(strncmp((char*)cmd, "MDCTRL", strlen("MDCTRL"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "MDCTRL ", strlen("MDCTRL "))==0)
			{
				cmd+=strlen("MDCTRL ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("set MD OnOff=%d \r\n",u8val);
				
				if(u8val)
				{
					mduMotionDetect_ONOFF(1); 
				}
				else
				{
					mduMotionDetect_ONOFF(0); 
				}
				
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("MotionDetect_en=%d \r\n", MotionDetect_en);
			}
	
		}else
	
		if(strncmp((char*)cmd, "MDLV_D", strlen("MDLV_D"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "MDLV_D ", strlen("MDLV_D "))==0)
			{
				cmd+=strlen("MDLV_D ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("set MD_level_DeIntlac=%d \r\n",u8val);
				
				MD_level_DeIntlac=u8val;
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("MD_level_DeIntlac=%d (MD_Diff=%d,video_double_field_flag=[%d,%d])\r\n", MD_level_DeIntlac, MD_Diff, VideoClipOption[VIDEO_CH1].video_double_field_flag,VideoClipOption[VIDEO_CH2].video_double_field_flag);
			}
	
		}else
		if(strncmp((char*)cmd, "MDLV_M", strlen("MDLV_M"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "MDLV_M ", strlen("MDLV_M "))==0)
			{
				cmd+=strlen("MDLV_M ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("set MD_level_MdTrig=%d \r\n",u8val);
				
				MD_level_MdTrig=u8val;
		
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{
				DEBUG_DMSG("MD_level_MdTrig=%d (MD_Diff=%d,MD_trigger=[%d,%d])\r\n", MD_level_MdTrig, MD_Diff, VideoClipOption[VIDEO_CH1].MD_Diff,VideoClipOption[VIDEO_CH2].MD_Diff);
			}
		
		}else
	
		//Motion Detection Period
		if(strncmp((char*)cmd, "MD_P_SEL", strlen("MD_P_SEL"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "MD_P_SEL ", strlen("MD_P_SEL "))==0)
			{
				cmd+=strlen("MD_P_SEL ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				
				switch(u8val)
				{
				//case 0 ... 2://0:high, 1:mid, 2:low			
				case 0:
				case 1:
				case 2:
					{
						//legal
						DEBUG_DMSG("set MD_Period_Sel=%d \r\n",u8val);
					}
					break;							
				}
				MD_Period_Sel=u8val;
	
	
				//---------------------
				switch(MD_Period_Sel)
				{
				case 0:
					{//Hight speed
						MD_period_Preview=4;	//must be 2^n
						MD_period_Video=8;		//must be 2^n	
					}
					break;
				case 1:
					{//medium speed
						MD_period_Preview=4;	//must be 2^n
						MD_period_Video=16; 	//must be 2^n
					}
					break;
				case 2:
					{//Low speed
						MD_period_Preview=8;	//must be 2^n
						MD_period_Video=32; 	//must be 2^n
					}
					break;
				}
				//---------------------
	
		
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{			
				DEBUG_DMSG("MD_Period_Sel=%d \r\n", MD_Period_Sel);
			}
		
		}else
	
		//Motion DeInterlace always on if MotionDetect_en is enabled
		if(strncmp((char*)cmd, "MD_DEINT_A", strlen("MD_DEINT_A"))==0)
		{
			if(MotionDetect_en)
			{
				u8 show_sv=1;
				
				if (strncmp((char*)cmd, "MD_DEINT_A ", strlen("MD_DEINT_A "))==0)
				{		
					cmd+=strlen("MD_DEINT_A ");
					if(*cmd)
					{
					sscanf(cmd, "%d",&u8val);
	
					MD_DeInt_AlwaysOn=(u8val)?1:0;
			
					show_sv=0;
					}					
				}
				////
				if(show_sv)
				{			
					DEBUG_DMSG("MD_DeInt_AlwaysOn=%d \r\n", MD_DeInt_AlwaysOn);
				}
				
			}
			else
			{
				DEBUG_DMSG("MotionDetect_en is not enabled.\r\n");
			}
	
		
		}else
	
		//Motion Sensitivity Selection
		if(strncmp((char*)cmd, "MD_S_SEL", strlen("MD_S_SEL"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "MD_S_SEL ", strlen("MD_S_SEL "))==0)
			{
				cmd+=strlen("MD_S_SEL ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				
				switch(u8val)
				{
				//case 0 ... 2://0:high,1:mid,2:low
				case 0:
				case 1:
				case 2:
					{
						//legal
						DEBUG_DMSG("set MD_SenLevel=%d \r\n",u8val);
					}
					break;
				default:
					{
						//illegal
						u8val=0;//default
						DEBUG_DMSG("set MD_SenLevel=%d \r\n",u8val);
					}
					break;				
				}
				MD_SenLevel=u8val;
	
	
				//---------------------
				#if HW_MD_SUPPORT
					mduMotionDetect_Sensitivity_Config(MD_SenLevel);
					DEBUG_DMSG("mduMotionDetect_Sensitivity_Config(%d),{%d,%d}\n",MD_SenLevel,*pMD_Thr_PixelDiff,*pMD_Thr_BlockMargin);
				#endif
				//---------------------
	
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{			
				DEBUG_DMSG("MD_SenLevel=%d \r\n", MD_SenLevel);
			}
		
		}else				
	
		#if 1
			if(strncmp((char*)cmd, "VDORES", strlen("VDORES"))==0)
			{
				u8 show_sv=1;
				if (strncmp((char*)cmd, "VDORES ", strlen("VDORES "))==0)
				{
					cmd+=strlen("VDORES ");
					if(*cmd)
					{
					//sscanf(cmd, "%d",&u8val);
					//DEBUG_DMSG("VDORES>>%d \r\n",u8val);
					sscanf(cmd, "%x",&u8val);
					DEBUG_DMSG("VDORES>>0x%02X \r\n",u8val);
				
					if((u8val&0xF0))//with options			
					{
						UI_SwitchVideoResolution(u8val);	
					}
					else
					{
						if(u8val>UI_MENU_SETTING_RESOLUTION_QVGA)
						{
							DEBUG_DMSG("unsupported resolution ! \n");
						}
						else
						{
							UI_SwitchVideoResolution(u8val);//(u8val&0xF0)						
						}
					}
						
					show_sv=0;
					}	
				}
		
				if(show_sv)
				{//read current recording status
					char TxtBuff[8];
					u8 VdoResSel=10;
					switch(uiMenuVideoSizeSetting)
					{
					case UI_MENU_VIDEO_SIZE_1280X720://UI_MENU_SETTING_RESOLUTION_HD
						{
							VdoResSel=UI_MENU_SETTING_RESOLUTION_HD;
							sprintf(TxtBuff,"%s","HD");
						}
						break;
					case UI_MENU_VIDEO_SIZE_640x480://UI_MENU_SETTING_RESOLUTION_VGA
						{
							VdoResSel=UI_MENU_SETTING_RESOLUTION_VGA;
							sprintf(TxtBuff,"%s","VGA");
						}
						break;

                    case UI_MENU_VIDEO_SIZE_704x480://UI_MENU_SETTING_RESOLUTION_D1_480V
						{
							VdoResSel=UI_MENU_SETTING_RESOLUTION_D1_480V;
							sprintf(TxtBuff,"%s","D1");
						}
						break;   

                    case UI_MENU_VIDEO_SIZE_352x240://UI_MENU_SETTING_RESOLUTION_CIF
						{
							VdoResSel=UI_MENU_SETTING_RESOLUTION_352x240;
							sprintf(TxtBuff,"%s","CIF");
						}
						break;        

					case UI_MENU_VIDEO_SIZE_320x240://UI_MENU_SETTING_RESOLUTION_QVGA
						{
							VdoResSel=UI_MENU_SETTING_RESOLUTION_QVGA;					
							sprintf(TxtBuff,"%s","QVGA");
						}
						break;
					default:
						{//unsupported resolution
							sprintf(TxtBuff,"%s","NA");
						}
						break;
					}//switch()					

					//if(VdoResSel<=UI_MENU_SETTING_RESOLUTION_QVGA)
					{
						DEBUG_DMSG("VDORES=%d(%s)\r\n",VdoResSel,TxtBuff);
					}
					
					
				}
		
			}else	
		#endif
		
		if(strncmp((char*)cmd, "RECCTRL", strlen("RECCTRL"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "RECCTRL ", strlen("RECCTRL "))==0)
			{
				cmd+=strlen("RECCTRL ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("RECCTRL>>%d \r\n",(u8val)?1:0);
	
					if(UI_FastCtrl_Rec((u8val)?1:0)==0)
					{
						DEBUG_DMSG("success\n");
					}
					else
					{
						DEBUG_DMSG("fail\n");
					}
	
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{//read current recording status
				//DEBUG_DMSG("RECSTATUS(%d)\r\n", UI_isVideoRecording());
				DEBUG_DMSG("RECSTATUS(%d)\r\n", UI_IsRec());
			}
	
		}else
	
#if 1
	//App Mode Switching
	
		if(strncmp((char*)cmd, "APPMODE", strlen("APPMODE"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "APPMODE ", strlen("APPMODE "))==0)
			{
				cmd+=strlen("APPMODE ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("APPMODE>>%d \r\n",u8val);
	
				if(u8val<APP_OPMODE_NUM)
				{
					if(UI_APP_OP_MODE_SET(u8val)==0)
					{
						DEBUG_DMSG("success\n");
					}
					else
					{
						DEBUG_DMSG("fail\n");
					}			
				}
				else
				{
					DEBUG_DMSG("unknow AppMode.\n");
				}
	
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{//read current app mode
				char TxtBuff[32];
				
				UI_APP_OP_MODE_GET((char*)TxtBuff);
				if(strlen(TxtBuff)==0)
				{
					sprintf(TxtBuff,"-NA-");
				}
				DEBUG_DMSG("APPMODE=%s(%d)\r\n",TxtBuff,UI_Get_OpMode());
			}
	
		}else
#endif	
	
#if 1	

		//
		//SD Re-Mount
		//	
		if(strncmp((char*)cmd, "SDMNT", strlen("SDMNT"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "SDMNT ", strlen("SDMNT "))==0)
			{
				cmd+=strlen("SDMNT ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("SDMNT>>%d \r\n",u8val);
		
				//---------------------
				
				if(u8val)
				{
					DEBUG_DMSG("SD Mount...\n");
					//Do_SD_Mount(0);//silent changed
					UI_Do_SD_Mount(0x01);//with UI changed
				}
				else
				{
					DEBUG_DMSG("SD UnMount...\n");	
					//Do_SD_UnMount(0);//silent changed
					UI_Do_SD_UnMount(0x01);//with UI changed
				}

				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current status
				if(gInsertCard)
				{			
					int status=sdcDevStatus(0);
					DEBUG_DMSG("SDMNT %s(%d)\r\n",(status==0)?"OK":"NO",status);
				}
				else
				{
					DEBUG_DMSG("SD card is absent!\n");
				}				
			}
		
		}else

		//
		//SD chkdsk
		//		
		if(strncmp((char*)cmd, "SDCHK", strlen("SDCHK"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "SDCHK ", strlen("SDCHK "))==0)
			{
				cmd+=strlen("SDCHK ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("SDCHK>>%d \r\n",u8val);
		
				//---------------------
				if(u8val)
				{//with options
					//...
					//TBD
				}
				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current status
				if(gInsertCard)
				{			
					s32 status=0;
					status=dcfCheckUnit();
					DEBUG_DMSG("SDCHK %s(dcfCheckUnit=%d)\r\n",(status==1)?"OK":"ERR",status);
				}
				else
				{
					DEBUG_DMSG("SD card is absent!\n");
				}				
			}
		
		}else



		//
		//SD In/Off
		//		
		if(strncmp((char*)cmd, "SDIN", strlen("SDIN"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "SDIN ", strlen("SDIN "))==0)
			{
				cmd+=strlen("SDIN ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("SDIN>>%d \r\n",u8val);
		
				//---------------------
				{
				s32 status=0;
					if(u8val==0)
					{					
						status=sysSDCD_OFF(1);
						DEBUG_DMSG("SD Card Off=%d\n",status);
					}
					else
					{
						status=sysSDCD_IN(1);
						DEBUG_DMSG("SD Card In=%d\n",status);
					}
				}
				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current status
	
				//DEBUG_DMSG("SDIN:%s,(formatted=%d)\r\n",(gInsertCard==0)?"OFF":"IN",sys_format);			
				DEBUG_DMSG("SDIN=%d,(formatted=%d)\r\n",gInsertCard,sys_format);			
			}
		
		}else






				  

		//
		//TV-Out Format
		//		
		if(strncmp((char*)cmd, "TVOFMT", strlen("TVOFMT"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "TVOFMT ", strlen("TVOFMT "))==0)
			{
				cmd+=strlen("TVOFMT ");
				if(*cmd)
				{
				sscanf(cmd, "%x",&u8val);
				DEBUG_DMSG("TVOFMT>>0x%02X \r\n",u8val);
		
				//---------------------
				{
					u8 TvFmt=0;
					TvFmt=(u8val&0x0F);	
					if((u8val&0xF0))//with options
					{
						UI_SwitchTvOutFormat(u8val);
					}
					else
					{
						if(TvFmt==SYS_TV_OUT_NTSC||TvFmt==SYS_TV_OUT_PAL)
						{
							UI_SwitchTvOutFormat((0x40|(TvFmt+1)));
						}
						else
						{
							DEBUG_DMSG("Unknown TV Out mode\n");
						}
					}
					
				}
				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current status
				char TxtBuff[32]="";
								
				switch(TvOutMode)
				{
				case SYS_TV_OUT_NTSC:
					sprintf(TxtBuff,"NTSC");
					break;				
				case SYS_TV_OUT_PAL:
					sprintf(TxtBuff,"PAL");
					break;
				default:
					sprintf(TxtBuff,"NA");
					break;					
				}
				DEBUG_DMSG("TVOFMT %s(%d)\r\n",TxtBuff,TvOutMode);

			}
		
		}else
		//
		//TV-In Format
		//
		if(strncmp((char*)cmd, "TVIFMT", strlen("TVIFMT"))==0)
		{
			u8 show_sv=1;
			
			
			if (strncmp((char*)cmd, "TVIFMT ", strlen("TVIFMT "))==0)
			{
				cmd+=strlen("TVIFMT ");
				if(*cmd)
				{
				sscanf(cmd, "%x",&u8val);
				DEBUG_DMSG("TVIFMT>>0x%02X \r\n",u8val);
		
				//---------------------
				{
					u8 TvFmt=0;
					TvFmt=(u8val&0x0F);
					if((u8val&0xF0))//with options
					{
						UI_SwitchTvInFormat(u8val);
					}
					else
					{
						if(TvFmt==TV_IN_NTSC||TvFmt==TV_IN_PAL)
						{
							UI_SwitchTvInFormat((0x40|(TvFmt+1)));
						}
						else
						{
							DEBUG_DMSG("Unknown TV In format\n");
						}			
					}

				}
				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current status
				char TxtBuff[32]="";
								
				switch(TvOutMode)
				{
				case TV_IN_NTSC:
					sprintf(TxtBuff,"NTSC");
					break;				
				case TV_IN_PAL:
					sprintf(TxtBuff,"PAL");
					break;
				default:
					sprintf(TxtBuff,"NA");
					break;					
				}
				DEBUG_DMSG("TVIFMT %s(%d)\r\n",TxtBuff,sysTVinFormat);

			}
		
		}else



		
	#if (USE_USER_CNT)
		if(strncmp((char*)cmd, "USERCNT", strlen("USERCNT"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "USERCNT ", strlen("USERCNT "))==0)
			{
				cmd+=strlen("USERCNT ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u32val);
				DEBUG_DMSG("USERCNT: %d \r\n",u32val);
				//---------------------
				//...

				UI_Set_UserCnt(u32val);

				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("USERCNT=%d\r\n",setUI.UserCnt);
			}
	
		}else
	#endif

	

	#if (USE_REC_TERM_MON)
	//RTM Enabled
		if(strncmp((char*)cmd, "RTM_ENB", strlen("RTM_ENB"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "RTM_ENB ", strlen("RTM_ENB "))==0)
			{
				cmd+=strlen("RTM_ENB ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("RTM_ENB: %d \r\n",u8val);
				//---------------------
				//...
				
				_APP_ENTER_CS_;//OS_ENTER_CRITICAL();	
				gRTM_En=(u8val)?1:0;				
				_APP_EXIT_CS_;//OS_EXIT_CRITICAL(); 
				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("RTM_ENB=%d\r\n",gRTM_En);
			}
	
		}else
	#endif
	


	#if (USE_UI_TASK_WDT)
	//UI_TASK_WDT  Enabled
		if(strncmp((char*)cmd, "UIWDT_ENB", strlen("UIWDT_ENB"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "UIWDT_ENB ", strlen("UIWDT_ENB "))==0)
			{
				cmd+=strlen("UIWDT_ENB ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("UIWDT_ENB: %d \r\n",u8val);
				//---------------------
				//...
				
				_APP_ENTER_CS_;//OS_ENTER_CRITICAL();	
				if(u8val==0)
				{
				//MACRO_UI_SET_SYSSTAT_BIT_CLR(UI_SET_SYSSTAT_BIT_UITASKWDT_EN);
					UI_TaskWdt_Enabled_Set(UITSKWDT_DISABLE);
				}else
				{
				//MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_UITASKWDT_EN);
					UI_TaskWdt_Enabled_Set(UITSKWDT_ENABLE);
				}			
				_APP_EXIT_CS_;//OS_EXIT_CRITICAL(); 

				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				//DEBUG_DMSG("UIWDT_ENB=%d (%d)\r\n",MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_UITASKWDT_EN)?1:0,setUI.UiTaskWdt);
				DEBUG_DMSG("UIWDT_ENB=%d (%d)\r\n",(UI_TaskWdt_Enabled_Chk()==UITSKWDT_ENABLE)?UITSKWDT_ENABLE:UITSKWDT_DISABLE,setUI.UiTaskWdt);
			}
	
		}else
	#endif


	#if (CIU_1_OVL_IDXCOLOR_TEST)//(_TEST ONLY_)
	//CIU_1_OVL_IDXCOLOR  
		if(strncmp((char*)cmd, "CIU1OV_Y", strlen("CIU1OV_Y"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "CIU1OV_Y ", strlen("CIU1OV_Y "))==0)
			{
				cmd+=strlen("CIU1OV_Y ");
				if(*cmd)
				{
				sscanf(cmd, "%X",&u32val);
				DEBUG_DMSG("CIU1OV_Y: 0x%08X \r\n",u32val);
				//---------------------
				//...
				
				_APP_ENTER_CS_;//OS_ENTER_CRITICAL();	
				CIU_1_OVL_IDXCOLOR_Y = u32val;
				_APP_EXIT_CS_;//OS_EXIT_CRITICAL(); 

				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("CIU1OV_Y=0x%08X \r\n",CIU_1_OVL_IDXCOLOR_Y);
			}
	
		}else
		if(strncmp((char*)cmd, "CIU1OV_CB", strlen("CIU1OV_CB"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "CIU1OV_CB ", strlen("CIU1OV_CB "))==0)
			{
				cmd+=strlen("CIU1OV_CB ");
				if(*cmd)
				{
				sscanf(cmd, "%X",&u32val);
				DEBUG_DMSG("CIU1OV_CB: 0x%08X \r\n",u32val);
				//---------------------
				//...
				
				_APP_ENTER_CS_;//OS_ENTER_CRITICAL();	
				CIU_1_OVL_IDXCOLOR_CB = u32val;
				_APP_EXIT_CS_;//OS_EXIT_CRITICAL(); 

				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("CIU1OV_CB=0x%08X \r\n",CIU_1_OVL_IDXCOLOR_CB);
			}
	
		}else
		if(strncmp((char*)cmd, "CIU1OV_CR", strlen("CIU1OV_CR"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "CIU1OV_CR ", strlen("CIU1OV_CR "))==0)
			{
				cmd+=strlen("CIU1OV_CR ");
				if(*cmd)
				{
				sscanf(cmd, "%X",&u32val);
				DEBUG_DMSG("CIU1OV_CR: 0x%08X \r\n",u32val);
				//---------------------
				//...
				
				_APP_ENTER_CS_;//OS_ENTER_CRITICAL();	
				CIU_1_OVL_IDXCOLOR_CR = u32val;
				_APP_EXIT_CS_;//OS_EXIT_CRITICAL(); 

				//---------------------
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("CIU1OV_CR=0x%08X \r\n",CIU_1_OVL_IDXCOLOR_CR);
			}
	
		}else		
	#endif




	#if 1	
	//
	//Delete last recording file
	//
	if(strncmp((char*)cmd, "DEL_LAST", strlen("DEL_LAST"))==0)
	{
		if(gInsertCard && sys_format)
		{
			if((!UI_IsRec()) || (!UI_isVideoPlaying()))
			{
			/*
				if(dcfListDirEntTail!=NULL) dcfPlaybackCurDir = dcfListDirEntTail;
				if(dcfListFileEntTail!=NULL)	dcfPlaybackCurFile = dcfListFileEntTail;
				if(dcfPlaybackCurDir && dcfPlaybackCurFile)
			*/
				{
					DEBUG_DMSG("DEL_LAST...\r\n");
					UI_DeleteRecLastFile();
				}
			}
			else
			{
				DEBUG_DMSG("No operation be accepted when video is playing or recording.\n", UI_IsRec());
			}//if(UI_IsRec())

		}//if(gInsertCard && sys_format)
		else
		{
	
			if(!gInsertCard)
			{
				DEBUG_DMSG("no SD card.\n");
			}
			else
			{
			if(!sys_format)
				DEBUG_DMSG("unknown format\n");						
			}

		}			
	}else
	#endif




	




	#if 1//(DCF_FILE_TEST)//(_TEST ONLY_)

		if(strncmp((char*)cmd, "SD_DIR_OP", strlen("SD_DIR_OP"))==0)
		{

		
		if(gInsertCard && sys_format)
		{
			if((!UI_IsRec()) || (!UI_isVideoPlaying()))
			{
			
			

			u8 show_sv=1;
			if (strncmp((char*)cmd, "SD_DIR_OP ", strlen("SD_DIR_OP "))==0)
			{
				cmd+=strlen("SD_DIR_OP ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("SD_DIR_OP: %d \r\n",u8val);
				//---------------------
				//...
				//
				{
				//
				//OP code:
				// 0: Root
				// 1: Next
				// 2: Prev
				// 3: Enter
				// 4: Leave
				// 5: List
				// 6: Delete
				// others: pwd(print working directory)
				//				
				s32 ret = 0;//>0:success
				
				extern u8 PlayListDspType; //UI_DSP_PLAY_LIST_FILE;
				extern s32 (*uiPlaybackListNext[])(void);
				extern s32 (*uiPlaybackListPrev[])(void);


				if(dcfPlaybackCurDir==NULL) 	dcfPlaybackCurDir = dcfListDirEntTail;
				if(dcfPlaybackCurFile==NULL)	dcfPlaybackCurFile = dcfListFileEntTail;
				


				#if (USE_UI_TASK_WDT)	
					UI_TaskWdt_Enabled_Set(UITSKWDT_DISABLE);
				#endif

				switch(u8val)
				{
				case 0:	//Root
					{
						ret = dcfChDir("\\");// change root directory 
						PlayListDspType=UI_DSP_PLAY_LIST_DIR;
						DEBUG_DMSG("Root(PlayListDspType=%d)\n",PlayListDspType);					
					}
					break;
				case 1:	//Next
					{
						(*uiPlaybackListNext[PlayListDspType])();
						if(PlayListDspType==UI_DSP_PLAY_LIST_DIR)
						{
							if(dcfPlaybackCurDir->pDirEnt)
							{
								DEBUG_DMSG(">CurDir:[%.*s]\n",8,dcfPlaybackCurDir->pDirEnt->d_name);					
							}					
						}
						else if(PlayListDspType==UI_DSP_PLAY_LIST_FILE)
						{
							if(dcfPlaybackCurFile->pDirEnt)
							{
								DEBUG_DMSG(">CurFile:[%.*s]\n",12,dcfPlaybackCurFile->pDirEnt->d_name);						
							}					
						}	
						ret = 1;
					}
					break;
				case 2:	//Prev
					{
						(*uiPlaybackListPrev[PlayListDspType])();
						if(PlayListDspType==UI_DSP_PLAY_LIST_DIR)
						{
							if(dcfPlaybackCurDir->pDirEnt)
							{
								DEBUG_DMSG(">CurDir:[%.*s]\n",8,dcfPlaybackCurDir->pDirEnt->d_name);				
							}					
						}
						else if(PlayListDspType==UI_DSP_PLAY_LIST_FILE)
						{
							if(dcfPlaybackCurFile->pDirEnt)
							{
								DEBUG_DMSG(">CurFile:[%.*s]\n",12,dcfPlaybackCurFile->pDirEnt->d_name);						
							}					
						}	
						ret = 1;
					}
					break;
				case 3:	//Enter
					{
						if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
            			{
							if(global_totaldir_count == 0 || gInsertCard == 0)
							{
								ret=-1;
								break;
							}
							DEBUG_DMSG("Enter folder %s\r\n",dcfPlaybackCurDir->pDirEnt->d_name);
							PlayListDspType = UI_DSP_PLAY_LIST_FILE;


							if(dcfScanFileOnPlaybackDir() == 0)
							{
								DEBUG_DMSG("Folder %s inital fail \r\n",dcfPlaybackCurDir->pDirEnt->d_name);
								ret=-2;
								break;
							}
							else
							{
								DEBUG_DMSG("%d files in folder\r\n",global_totalfile_count);
								ret=1;

								#if 1
								//
								//change directory 
								//
								if(dcfPlaybackCurDir->pDirEnt)
								{
								#if 1
									//DEBUG_DMSG("Directory Name: %s\n",dcfRecCurDir->pDirEnt->d_name);

						            /* change current directory */
									ret = dcfChDir((s8*)dcfPlaybackCurDir->pDirEnt->d_name);
						            if (ret == 0)
						            {   /* change current directory error */
						                DEBUG_DMSG("Error: Change current directory failed.\n");
										break;
						            }
				
										
								#else
									s8 DirName[32];

									sprintf((const char*)DirName,"%.*s",8,dcfPlaybackCurDir->pDirEnt->d_name);						
										
									//if (strcmp((const char*)dcfCurDir, "\\") != 0)
						            {   /* current directory is not root directory */
						                strcat((char*)dcfCurDir, "\\");
						            }
						            strcat((char*)dcfCurDir, (const char*)DirName);
									ret = dcfChDir(dcfCurDir);
								#endif

								}
								#endif
							}

						 
						}
						else
						{
						#if 1
							//if(dcfPlaybackCurDir->pDirEnt->d_name && dcfPlaybackCurFile->pDirEnt->d_name)
							if(dcfPlaybackCurDir && dcfPlaybackCurFile)
							{
								UI_DirectPlayLastFile(0,0);
							}

						#else
							DEBUG_DMSG("unsupport operation\n");
						#endif
						}
						
					}
					break;
				case 4:	//Leave
					{
						#if 1
						
						if(PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
						{
							//stop playback and force to switch to preview mode					
							if(!UI_isPreviewMode())
							{
								if(UI_isPlaybackMode())
								{
									uiPlaybackStop(GLB_ENA);	
									OSTimeDlyHMSM(0, 0, 0, 100);// delay 100ms		
								}								
								UI_gotoPreviewMode();
							}
							
							PlayListDspType = UI_DSP_PLAY_LIST_FILE;
						}
						else
						{

							ret = dcfChDir("..");// change root directory 
							PlayListDspType = UI_DSP_PLAY_LIST_DIR;
							DEBUG_DMSG("backward to upper level\n");	

						}
						
						
						#else

						if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
            			{
							if(global_totaldir_count == 0 || gInsertCard == 0)
							{
								ret=-1;
								break;
							}
							DEBUG_DMSG("Leave folder %s\r\n",dcfPlaybackCurDir->pDirEnt->d_name);
							PlayListDspType = UI_DSP_PLAY_LIST_DIR;

							dcfPlaybackCurDir = dcfListDirEntTail;
							if(dcfScanFileOnPlaybackDir() == 0)
							{
								DEBUG_UI("Folder %s inital fail \r\n",dcfPlaybackCurDir->pDirEnt->d_name);
								ret=-2;
								break;
							}
							else
							{
								DEBUG_UI("%d folder\r\n",global_totaldir_count);
								ret=1;
								
							}

						 
						}
						else
						{
							DEBUG_DMSG("unsupport operation\n");
						}

						#endif
					}
					break;
				case 5:// List
					{

						if(PlayListDspType==UI_DSP_PLAY_LIST_DIR)
						{
							ret = dcfDirInit();
							DEBUG_DMSG("total %d folders\n",global_totaldir_count);										
						}
						else if(PlayListDspType==UI_DSP_PLAY_LIST_FILE)
						{
						//ret = dcfFileInit(dcfListDirEntTail, CURRENTFILEENTRY);
						//#define CURRENTFILEENTRY     0x00
							ret = dcfFileInit(dcfPlaybackCurDir, 0x00);//CURRENTFILEENTRY


							if(ret)
							{
							DCF_LIST_FILEENT* ins;
							extern u32 dcfFileCount;
							extern DCF_LIST_FILEENT dcfListFileEnt[];
							extern struct FS_DIRENT dcfFileEnt[];
							int i;
								for (i = 0; i < dcfFileCount; i++)
								{
									ins = &dcfListFileEnt[i];
									ins->prev = &dcfListFileEnt[i-1];
									ins->next = &dcfListFileEnt[i+1];
								
									ins->used_flag = DCF_FILE_USE_CLOSE;
									ins->pDirEnt = &dcfFileEnt[i];
								#if 0
									////if(Main_Init_Ready)
									////	DEBUG_DCF("dcfListFileEnt[%d].d_name=%s \n",i,dcfFileEnt[i].d_name); 
									
									//DEBUG_DCF("[%d]:%s\n",i,dcfFileEnt[i].d_name); 
									
									//if(i%50==0)	WDT_Reset_Count();
									if(i%31==0)
									{
									if(sysDeadLockCheck_ena)	sysDeadLockMonitor_Reset();
									}
								#else
								DEBUG_DMSG("[%d]:%s\n",i,dcfFileEnt[i].d_name); 		
								#endif
									ins ++;
								}

							}
							DEBUG_DMSG("total %d files\n",global_totalfile_count);					
						}	
						//ret = 1;

					}
					break;
				case 6:	//Delete
					{
						ret=-1;
						if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
						{
							DEBUG_DMSG("Delete Folder\r\n");
							ret=dcfPlaybackDelDir();
							/*
							if(dcfPlaybackDelDir() == 0)
							{
								DEBUG_DMSG("Delete Folder Fail\r\n");
								ret=-2;
							}	
							else
								ret=0;
							*/
						}
						else if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
						{
							DEBUG_DMSG("Delete File\r\n");
							ret=dcfPlaybackDel();
							/*
							if(dcfPlaybackDel() == 0)
							{
								DEBUG_DMSG("Delete File Fail\r\n");							
								ret=-2;
							}
							else
								ret=0;
							*/
						}

					}
					break;		
				default:	//PWD
					{
						#if 1
						ret = dcfChDir(".");
						DEBUG_DMSG("pwd: %s\n",dcfCurPath);

							#if 0						
							//s8* pDirName;
							{
								dcfCreateTargetPath(pFileName);
								strcpy((char*)CurrTargetPath, (const char*)dcfTargetPath);
							}
							/*
							pFile = FS_FOpen((const char*)CurrTargetPath, (const char*)pMode); /* open file */
							if (pFile == NULL)
							{	/* file open failed */
								DEBUG_DCF("Error: Open %s failed.\n", CurrTargetPath);
						
								
							}
							*/
							#endif
							




						#else
						s8	DirName[32];
						sprintf ((char*)DirName, "%s\\", dcfCurDir);
						ret = dcfChDir(DirName);
						if (ret == 0)
						{	/* change directory \DCIM error */
						}
						#endif
					}
					break;					
				}

				#if (USE_UI_TASK_WDT)	
					UI_TaskWdt_Enabled_Set(UITSKWDT_ENABLE);
				#endif 




				DEBUG_DMSG("SD_DIR_OP ret=%d \n",ret);
				}
				//---------------------
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{
				DEBUG_DMSG("SD_DIR_OP,dcfCurDir:%s ",dcfCurDir);
				if(PlayListDspType==UI_DSP_PLAY_LIST_DIR)
				{
					DEBUG_DMSG("CurDir:[%.*s]",8,dcfPlaybackCurDir->pDirEnt->d_name); 									
				}
				else if(PlayListDspType==UI_DSP_PLAY_LIST_FILE)
				{
					DEBUG_DMSG("CurFile:[%.*s]",12,dcfPlaybackCurFile->pDirEnt->d_name); 									
				}
				DEBUG_DMSG("\n");						
			}








			}
			else
			{
				DEBUG_DMSG("No operation be accepted when video is playing or recording.\n", UI_IsRec());
			}//if(UI_IsRec())



		}//if(gInsertCard && sys_format)
		else
		{
	
			if(!gInsertCard)
			{
				DEBUG_DMSG("no SD card.\n");
			}
			else
			{
			if(!sys_format)
				DEBUG_DMSG("unknown format\n");						
			}

		}	
		
		}else






	
	#endif

	




#endif
		
#if (USE_UISYS_PARM)
	#if(UISYS_PAR_BMAP)
		if(strncmp((char*)cmd, "PARAUTOINC", strlen("PARAUTOINC"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "PARAUTOINC ", strlen("PARAUTOINC "))==0)
			{
				cmd+=strlen("PARAUTOINC ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				
				if(u8val<=1)
				{//write
					DEBUG_DMSG("set UISYS_PAR_BMAP[] index auto increase=%d\r\n",(u8val)?1:0);
					_APP_ENTER_CS_;
					gUISYS_PAR_BMAP_IdxAutoInc=(u8val)?1:0;
					_APP_EXIT_CS_;					
				}
				else
				{
					DEBUG_DMSG("invalid setting\n");
				}
	
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read
				DEBUG_DMSG("UISYS_PAR_BMAP[] index auto increase:%s\r\n", (gUISYS_PAR_BMAP_IdxAutoInc)?"enable":"disable");
			}
		
		}else
	
		if(strncmp((char*)cmd, "PARIDX", strlen("PARIDX"))==0)
		{
			u8 show_sv=1;
			if (strncmp((char*)cmd, "PARIDX ", strlen("PARIDX "))==0)
			{
				cmd+=strlen("PARIDX ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				DEBUG_DMSG("set UISYS_PARM ByteIndex=%d \r\n",u8val);
	
				_APP_ENTER_CS_;
				gUISYS_PAR_BMAP_Idx=u8val;
				_APP_EXIT_CS_;
	
				show_sv=0;
				}	
			}
	
			if(show_sv)
			{
				DEBUG_DMSG("UISYS_PAR_BMAP ByteIndex=%d\r\n", gUISYS_PAR_BMAP_Idx);
			}
	
		}else
		
		if(strncmp((char*)cmd, "PAREDIT", strlen("PAREDIT"))==0)
		{
			u8 show_sv=1;
			
			assert ((UISYS_PAR_BID_NUM <= sizeof(gUISYS_PARM)));
		
			if (strncmp((char*)cmd, "PAREDIT ", strlen("PAREDIT "))==0)
			{
				cmd+=strlen("PAREDIT ");
				if(*cmd)
				{
				sscanf(cmd, "%d",&u8val);
				
				//if(gUISYS_PAR_BMAP_Idx<sizeof(gUISYS_PARM))
				if(gUISYS_PAR_BMAP_Idx<UISYS_PAR_BID_NUM)
				{//write
					DEBUG_DMSG("PAREDIT UISYS_PARM[%d]=%02X\r\n",gUISYS_PAR_BMAP_Idx,u8val);
					_APP_ENTER_CS_;
					gpUISYS_PAR_BMAP[gUISYS_PAR_BMAP_Idx]=u8val;
					//
					if(gUISYS_PAR_BMAP_IdxAutoInc)
					{
						gUISYS_PAR_BMAP_Idx++;
					}
					_APP_EXIT_CS_;
				}
				else
				{
					DEBUG_DMSG("invalid index\n");
				}
	
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read
				//if(gUISYS_PAR_BMAP_Idx<sizeof(gUISYS_PARM))
				if(gUISYS_PAR_BMAP_Idx<UISYS_PAR_BID_NUM)
				{
					DEBUG_DMSG("UISYS_PAR_BMAP[%d]=%02X\r\n", gUISYS_PAR_BMAP_Idx,gpUISYS_PAR_BMAP[gUISYS_PAR_BMAP_Idx]);
					_APP_ENTER_CS_;
					if(gUISYS_PAR_BMAP_IdxAutoInc)
					{
						gUISYS_PAR_BMAP_Idx++;
					}
					_APP_EXIT_CS_;			
				}
				else
				{
					DEBUG_DMSG("invalid index\n");
				}			
			}
		
		}else
	#endif //#if(UISYS_PAR_BMAP)
		
		if(strncmp((char*)cmd, "PARSET", strlen("PARSET"))==0)
		{
			//int res=-1;
			//res=UISYS_PARM_Set(NULL);
			//DEBUG_DMSG("PARSET(%d) \r\n",res);
			DEBUG_DMSG("PARSET(%d) \r\n",UISYS_PARM_Set(NULL));
		}else
	
		if(strncmp((char*)cmd, "PARPRESET", strlen("PARPRESET"))==0)
		{
			u8 show_sv=1;
	
			if (strncmp((char*)cmd, "PARPRESET ", strlen("PARPRESET "))==0)
			{
	
				cmd+=strlen("PARPRESET ");
				if(*cmd)
				{
				u8 cmdcode=0;
				sscanf(cmd, "%d",&u8val);
				cmdcode=u8val;
				//DEBUG_DMSG("PARTEST>>%d \r\n",cmdcode);
	
					//switch(cmdcode)
					//{
					//	case 0:
					
						//preset
						DEBUG_DMSG("PARTEST PRESET>>(%d)\n",UI_PresetUiSysParam());
				
					//	break;
					//...
					//}
	
				show_sv=0;
				}	
			}
		
			if(show_sv)
			{//read current recording status
				DEBUG_DMSG("PARTEST SHOW>>\n");				
				UI_ShowUiSysParam();
			}
		
		}else

		
#endif //#if (USE_UISYS_PARM)
			
#endif//DMSG_EXT_CMD
	//extension commands ------------------------------------------------------------------------	
	
	
	
	#if 1	//SHOW_SYS_INFO
		if (strncmp((char*)cmd, "INFO", strlen("INFO"))==0)
		{
			u8 options=0xFF;//SHOW ALL
	
			if (strncmp((char*)cmd, "INFO ", strlen("INFO "))==0)
			{
				cmd+=strlen("INFO ");
				if(*cmd)
				{
				//sscanf(cmd, "%d",&u8val);
				sscanf(cmd, "%02X",&u8val);
				if(u8val)
					options=u8val;
				}//if(*cmd)
			}
			
			if(options)
			{
				char TxtBuff[32];
				
				
	
				
				DEBUG_DMSG("\n");
				DEBUG_DMSG("--------------------------------------------------------------------\n");
				DEBUG_DMSG("Info:\n");
				
				if(options&0x01)
				{//system info
				
					u8 op_mode=UI_Get_OpMode();
					//MODE_PREVIEW:7
					//MODE_TIMESET:8
					//... (Menu Screen)
					//MODE_PLAYBACK:13
	
	
								
					DEBUG_DMSG("----------------------------------------\n");
					DEBUG_DMSG("--- 		SYSTEM STATUS		  ---\n");
					DEBUG_DMSG("----------------------------------------\n");
	
					DEBUG_DMSG("LocalTime:\t%04d/%02d/%02d-%02d:%02d:%02d\n\n",(g_LocalTime.year+2000),g_LocalTime.month,g_LocalTime.day,g_LocalTime.hour,g_LocalTime.min,g_LocalTime.sec);
	
					DEBUG_DMSG("SYS_PowerOff:\t%d\n",setUI.SYS_PowerOff);
					DEBUG_DMSG("PwrCtrl_Phase:\t%d\n",setUI.PwrCtrl_Phase);
				#if(TV_DECODER != TVDEC_NONE) 
				#if(!MULTI_CH_DEGRADE_1CH)	
					DEBUG_DMSG("TV_IN_LOCK:\tCH1:%s , CH2:%s\n",(sysTVInFormatLocked)?"Locked":"unlock",(sysTVInFormatLocked1)?"Locked":"unlock");
				#else
					DEBUG_DMSG("TV_IN_LOCK:\tCH1:%s \n",(sysTVInFormatLocked)?"Locked":"unlock");
				#endif
				#endif
					DEBUG_DMSG("TV_IN:\t\t%s \n",(sysTVinFormat  == TV_IN_NTSC)?"NTSC":"PAL");
					DEBUG_DMSG("TV_OUT:\t\t%s \n",(TvOutMode==SYS_TV_OUT_NTSC)?"NTSC":"PAL");
					/*
					if(sysTVOutOnFlag)
						DEBUG_DMSG("OSD Size:\t(X:%d Y:%d)\n",TVOSD_SizeX,TVOSD_SizeY);
						else
						DEBUG_DMSG("OSD Size:\t(X:%d Y:%d)\n",OSD_Width,OSD_Height);			
					   */		
					////VIDEO_RESOLUTION_SEL
					if(sysTVOutOnFlag)//for TV-OUT
					{				
						DEBUG_DMSG("VIDEO_RESOLUTION:\t%d x %d\n",VideoClipOption[VIDEO_CH1].asfVopWidth,VideoClipOption[VIDEO_CH1].asfVopHeight);
					}
					
					DEBUG_DMSG("OP_MODE:\t0x%02X\n",op_mode);//_opMode
					DEBUG_DMSG("IO_STATUS:\t0x%08X\n",g_DIOStatus.data[0]);
					DEBUG_DMSG("SD card:\t\t%s ",(gInsertCard)?"inserted":"absent");
					if(gInsertCard)
					{			
						DEBUG_DMSG(", %s ",(setUI.SD_NotFormat==0)?"formatted":"not format");
						DEBUG_DMSG(", Capacity:%dMB ",Mem_Capacity());
				#if 0// 1
						DEBUG_DMSG(", Available:%4.1f%% ",((float)Mem_Available_percent()/10));
				#else
						{
							int  tmp_1=0;
							int  tmp_2=0;
							
							tmp_1=Mem_Available_percent();
							tmp_2=(tmp_1%10);
							tmp_1/=10;
							DEBUG_DMSG(", Available:%d.%d%% ",tmp_1,tmp_2);
						}
				#endif 
						DEBUG_DMSG(", %s\n",(UI_isMemoryFull())?"full":"workable");
					}
					else	DEBUG_DMSG("\n");	
					
					if(op_mode==0x07)//MODE_PREVIEW
					{	
						//DEBUG_DMSG("Preview CH:	%d\n",setUI.PreviewCH);
						//switch(setUI.RecMode)
						switch(setUI.PreviewCH)
						{
						case 0:
							sprintf(TxtBuff,"CH1");
							break;
						case 1:
							sprintf(TxtBuff,"CH2");
							break;
						case 2:
							sprintf(TxtBuff,"CH1+CH2");
							break;			
						}				
						DEBUG_DMSG("Preview CH:\t%s\n",TxtBuff);
						
						DEBUG_DMSG("Recording:\t%s,(REC:0x%02X,EN:0x%02X)\n",(UI_isVideoRecording())?"Yes":"No",(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_ISREC|UI_SET_STATUS_BITMSK_CH2_ISREC)),(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_RECON|UI_SET_STATUS_BITMSK_CH2_RECON))>>4);
						
					}
					//if(op_mode==0x07)
					{
						switch(setUI.RecMode)
						{
						case RECMODE_AUTO:
							sprintf(TxtBuff,"Auto");
							break;
						case RECMODE_MANUAL:
							sprintf(TxtBuff,"Manual");
							break;
						case RECMODE_MOTION:
							sprintf(TxtBuff,"Motion");
							break;			
						}
						DEBUG_DMSG("Rec Mode:\t%s\n",TxtBuff);
					}
					#if(USE_PLAYBACK_AUTONEXT)
					//without Alarm-Out
					#else
					if(setUI.MD_AlarmTime)
					{
					DEBUG_DMSG("AlarmState:\t%d\n",setUI.MD_Alarming);
					DEBUG_DMSG("AlarmTime:\t%d(s)\n",setUI.MD_AlarmTime);
					}
					#endif
				#if 1
					//DEBUG_DMSG("----------------------------------------\n");
					DEBUG_DMSG("------------------------------\n");
					DEBUG_DMSG("setUI.RecStatus:\t0x%08X\n",setUI.RecStatus);
					DEBUG_DMSG("setUI.RecErr:\t0x%08X\n",setUI.RecErr);
				#if 1
					DEBUG_DMSG("setUI.RecTicks[0]:\t0x%08X\n",setUI.RecTicks[0]);
					DEBUG_DMSG("setUI.RecTicks[1]:\t0x%08X\n",setUI.RecTicks[1]);
				#endif 
					DEBUG_DMSG("setUI.SysStatus:\t0x%08X\n",setUI.SysStatus);
					DEBUG_DMSG("------------------------------\n");
					//DEBUG_DMSG("----------------------------------------\n");
				#endif 
					////DEBUG_DMSG("Working Folder:\t%s\n",dcfPlaybackCurFile->pDirEnt->d_name);	
								
				}
				//DEBUG_DMSG("----------------------------------------\n");
				if(options&0x02)
				{//dir/file info
					DEBUG_DMSG("----------------------------------------\n");
					DEBUG_DMSG("--- 		DIR/FILE PATH		  ---\n");
					DEBUG_DMSG("----------------------------------------\n");
					
					//
					//Rec_File_Path
					//
					if(gInsertCard && sys_format)
					{
					//DEBUG_DMSG("dcfCurDrive:%s, dcfCurDir:%s, dcfTargetPath:%s\n",dcfCurDrive,dcfCurDir,dcfTargetPath);
					DEBUG_DMSG("dcfCurDrive:%s , dcfCurDir:%s\n",dcfCurDrive,dcfCurDir);
					DEBUG_DMSG("FileEntTail:\n");
					if(UI_isPlaybackMode())
					{
						if(dcfPlaybackCurDir==NULL) 	dcfPlaybackCurDir = dcfListDirEntTail;
						if(dcfPlaybackCurFile==NULL)	dcfPlaybackCurFile = dcfListFileEntTail;
	
						if(strlen(dcfPlaybackCurDir->pDirEnt->d_name))
						{
							DEBUG_DMSG("dir_name:%s\n",dcfPlaybackCurDir->pDirEnt->d_name);
						}
						else
						{
							DEBUG_DMSG("dir_name:--- \n");
						}			
						if(strlen(dcfPlaybackCurFile->pDirEnt->d_name))
						{
							DEBUG_DMSG("file_name:%s \n",dcfPlaybackCurFile->pDirEnt->d_name);
						}
						else
						{
							DEBUG_DMSG("file_name:--- \n");
						}
	
					}
					else
					{
	
	
						if(dcfListDirEntTail||dcfListFileEntTail)
						{
							if(dcfListDirEntTail)
							{
								if(strlen(dcfListDirEntTail->pDirEnt->d_name))
								{
									DEBUG_DMSG("dir_name:%s\n",dcfListDirEntTail->pDirEnt->d_name);
								}
								else
								{
									DEBUG_DMSG("dir_name:--- \n");
								}										
							}
						
							if(dcfListFileEntTail)
							{
								if(strlen(dcfListFileEntTail->pDirEnt->d_name))
								{
									DEBUG_DMSG("file_name:%s \n",dcfListFileEntTail->pDirEnt->d_name);
								}
								else
								{
									DEBUG_DMSG("file_name:--- \n");
								}					
							}
						}
						else
						{
							DEBUG_DMSG("--- (dir:%d,file:%d) \n",global_totaldir_count,global_totalfile_count);
						}
					}
	
					}//if(gInsertCard)
					else
					{
						DEBUG_DMSG("no SD card\n");
					}
				}	
				//DEBUG_DMSG("----------------------------------------\n");
				if(options&0x04)
				{//menu settings info
					DEBUG_DMSG("----------------------------------------\n");
					DEBUG_DMSG("--- 		MENU SETTINGS		  ---\n");
					DEBUG_DMSG("----------------------------------------\n");
					//
					//REC_FPS_FILESAVE
					//
					switch(g_menus[MENU_CH1FPS].setItem)
					{
					case MPEG4_VIDEO_FRAMERATE_5:
						sprintf(TxtBuff,"5fps");
						break;
					case MPEG4_VIDEO_FRAMERATE_15:
						sprintf(TxtBuff,"15fps");
						break;
					case MPEG4_VIDEO_FRAMERATE_30:
						sprintf(TxtBuff,"30fps");
						break;
					default:
						sprintf(TxtBuff,"NA");
						break;					
					}				
					DEBUG_DMSG("CH1_FPS:\t%s (%d)\n",TxtBuff,VideoClipOption[1].VideoRecFrameRate);
	
					DEBUG_DMSG("CH1_FileSave:\t%s (%d)\n",(g_menus[MENU_CH1SAVE].setItem==0)?"yes":"no",(setUI.CH1SaveEn));
	
				#if(!MULTI_CH_DEGRADE_1CH)	
					switch(g_menus[MENU_CH2FPS].setItem)
					{
					case MPEG4_VIDEO_FRAMERATE_5:
						sprintf(TxtBuff,"5fps");
						break;
					case MPEG4_VIDEO_FRAMERATE_15:
						sprintf(TxtBuff,"15fps");
						break;
					case MPEG4_VIDEO_FRAMERATE_30:
						sprintf(TxtBuff,"30fps");
						break;
					default:
						sprintf(TxtBuff,"NA");
						break;					
					}				
					DEBUG_DMSG("CH2_FPS:\t%s (%d)\n",TxtBuff,VideoClipOption[2].VideoRecFrameRate);
	
					DEBUG_DMSG("CH2_FileSave:\t%s (%d)\n",(g_menus[MENU_CH2SAVE].setItem==0)?"yes":"no",(setUI.CH2SaveEn));
				#endif
	
					//
					//RECQUALITY
					//
					switch(g_menus[MENU_RECQUALITY].setItem)
					{
					case MPEG4_VIDEO_QUALITY_HIGH:
						sprintf(TxtBuff,"high");
						break;
					case MPEG4_VIDEO_QUALITY_MEDIUM:
						sprintf(TxtBuff,"medium");
						break;
					case MPEG4_VIDEO_QUALITY_LOW:
						sprintf(TxtBuff,"low");
						break;	
					default:
						sprintf(TxtBuff,"NA");
						break;					
					}				
					DEBUG_DMSG("RECQUALITY:\t%s (%d)\n",TxtBuff,mpeg4VideoRecQulity);
	
					//
					//RECOVERWR
					//
					DEBUG_DMSG("OVERWRITE:\t%s (%d)\n",(g_menus[MENU_RECOVERWR].setItem==0)?"yes":"no",(sysCaptureVideoMode&ASF_CAPTURE_OVERWRITE_ENA)?1:0);
	
					//
					//RECFILELEN
					//
					switch(RecFileLen[g_menus[MENU_RECFILELEN].setItem])
					{
					case UI_MENU_SETTING_SECTION_1MIN:
						sprintf(TxtBuff,"1min");
						break;
					case UI_MENU_SETTING_SECTION_5MIN:
						sprintf(TxtBuff,"5min");
						break;
					case UI_MENU_SETTING_SECTION_15MIN:
						sprintf(TxtBuff,"15min");
						break;	
					case UI_MENU_SETTING_SECTION_30MIN:
						sprintf(TxtBuff,"30min");
						break;
					//#if(RECFILELEN_60MIN==1)	
					case UI_MENU_SETTING_SECTION_60MIN:
						sprintf(TxtBuff,"60min");
						break;
					//#endif		
					//#if(RECFILELEN_120MIN==1)
					case UI_MENU_SETTING_SECTION_120MIN:
						sprintf(TxtBuff,"120min");
						break;
					//#endif											
					default:
						sprintf(TxtBuff,"NA");
						break;				
					}				
					DEBUG_DMSG("RECFILELEN:\t%s (%d)\n",TxtBuff,asfSectionTime);
	
					//
					//RECMODE
					//
					switch(g_menus[MENU_RECMODE].setItem)
					{
					case RECMODE_MANUAL:
						sprintf(TxtBuff,"Manual");
						break;
					case RECMODE_MOTION:
						sprintf(TxtBuff,"Motion");
						break;
					case RECMODE_AUTO:
						sprintf(TxtBuff,"Auto");
						break;			
					default:
						sprintf(TxtBuff,"NA");
						break;				
					}				
					DEBUG_DMSG("RECMODE:\t%s (%d)\n",TxtBuff,Current_REC_Mode);
	
	
					//
					//LANGUAGE
					//
					switch(g_menus[MENU_SYSLAN].setItem)
					{
					case L_English:
						sprintf(TxtBuff,"ENG");
						break;
					case L_Chinese:
						sprintf(TxtBuff,"CHT");
						break;
					case L_Schinese:
						sprintf(TxtBuff,"CHS");
						break;				
					default:
						sprintf(TxtBuff,"NA");
						break;				
					}				
					DEBUG_DMSG("LANG:\t\t%s (%d)\n",TxtBuff,g_langID);
	
					
				#if(USE_PLAYBACK_AUTONEXT) 
				//without Alarm-Out

					//
					//PLAYORDER
					//
					switch(g_menus[MENU_SYSPLAY].setItem)
					{
					case 0:
						sprintf(TxtBuff,"single");
						break;
					case 1:
						sprintf(TxtBuff,"order");
						break;
					/*
					case 2:
						sprintf(TxtBuff,"reserved");
						break;	
					*/
					default:
						sprintf(TxtBuff,"NA");
						break;				
					}				
					DEBUG_DMSG("PLAYORDER:\t\t%s (%d)\n",TxtBuff,setUI.SYS_PlayOrder);
			
				#else
					//
					//ALARM
					//
					switch(g_menus[MENU_SYSALARM].setItem)
					{
					case 0:
						sprintf(TxtBuff,"OFF");
						break;
					case 1:
						sprintf(TxtBuff,"10s");
						break;
					case 2:
						sprintf(TxtBuff,"20s");
						break;	
					case 3:
						sprintf(TxtBuff,"30s");
						break;				
					default:
						sprintf(TxtBuff,"NA");
						break;				
					}				
					DEBUG_DMSG("ALARM:\t\t%s (%d)\n",TxtBuff,setUI.SYS_AlarmTime);
				#endif
				
					//
					//WITHTIMESTAMP
					//
	
					DEBUG_DMSG("TIMESTAMP:\t%s (%d)\n",(g_menus[MENU_SYSWITHTIME].setItem==0)?"enable":"disable",OverwriteStringEnable);				
					
				}
				//DEBUG_DMSG("----------------------------------------\n");
				if(options&0x08)
				{//menu page info
					DEBUG_DMSG("----------------------------------------\n");
					DEBUG_DMSG("--- 		MENU PAGE INFO		  ---\n");
					DEBUG_DMSG("----------------------------------------\n");	
	
					//
					//MENU PAGE INFO
					//
					if(UI_isMenuMode())
					{
	
		
						switch(gMenuInfo.MenuId)
						{
						case MENU_TimeSetting:
							sprintf(TxtBuff,"TimeSetting");
							break;
						case MENU_CH1SET:
							sprintf(TxtBuff,"CH1SET");
							break;
					#if(!MULTI_CH_DEGRADE_1CH) 
						case MENU_CH2SET:
							sprintf(TxtBuff,"CH2SET");
							break;
					#endif
						case MENU_RECSET:
							sprintf(TxtBuff,"RECSET");
							break;			
						case MENU_CARDSET:
							sprintf(TxtBuff,"CARDSET");
							break;				
						case MENU_SYSSET:
							sprintf(TxtBuff,"SYSSET");
							break;			
								
						default:
							sprintf(TxtBuff,"NA");
							break;				
						}				
						DEBUG_DMSG("MENU PAGE ID:\t%s (%d)\n",TxtBuff,gMenuInfo.MenuId);//g_menus[MStack_now()]->intr->id(MenuID)
						//DEBUG_DMSG("SUB ID:\t%d\n",gMenuInfo.SubId);//subid;	
						//DEBUG_DMSG("SUB ID NAME:\t%s\n",gMenuInfo.SubId_Name);//subid name;	
						DEBUG_DMSG("SUB ID:\t\t%s (%d)\n",gMenuInfo.SubId_Name,gMenuInfo.SubId);//name,subid;
						DEBUG_DMSG("NOW ITEM:\t%d\n",gMenuInfo.NowItem);//m->nowItem; 
						DEBUG_DMSG("SET ITEM:\t%d\n",gMenuInfo.SetItem);//g_menus[subid].setItem; 
						
					}		
					else
					{
						DEBUG_DMSG("It's not under Menu mode\n");
					}
					
				}
				//DEBUG_DMSG("----------------------------------------\n");
				//...
	
	
	
				
				DEBUG_DMSG("--------------------------------------------------------------------\n\r");
			}
	
		}else	
	#endif 	//SHOW_SYS_INFO	
		{
			//;//NOP
			DEBUG_DMSG("unsupported command.\r\n");
		}	
		
	
}
#endif











#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)

#if (YD_UART_PROTOCOL)	

typedef enum _tUART_CMD_IDX
{
	UART_CMD_BEGIN = 0,
	//0
	UART_CMD_KU=UART_CMD_BEGIN,
	UART_CMD_KD,
	UART_CMD_KL,
	UART_CMD_KR,
	UART_CMD_KE,
	UART_CMD_KDEL,
	UART_CMD_KPLAY,	
	UART_CMD_KMENU,	
	UART_CMD_KESC,	
	UART_CMD_KSTOP,	
	UART_CMD_KCH,	
	UART_CMD_KREC,
	UART_CMD_QUERY,		
	//11
	UART_CMD_END,
	UART_CMD_NUM=UART_CMD_END,
}UART_CMD_IDX;






extern char* UART_CMD_TXT[UART_CMD_NUM];
typedef int (*tUART_CMD_CB)(void* pDat,u32 Len);//(u32* pDat,u16 Len);


typedef struct _sUART_CMD_CB
{
	tUART_CMD_CB pURCMD_KU;
	tUART_CMD_CB pURCMD_KD;
	tUART_CMD_CB pURCMD_KL;
	tUART_CMD_CB pURCMD_KR;
	tUART_CMD_CB pURCMD_KE;
	tUART_CMD_CB pURCMD_KDEL;
	tUART_CMD_CB pURCMD_KPLAY;
	tUART_CMD_CB pURCMD_KMENU;
	tUART_CMD_CB pURCMD_KESC;	
	tUART_CMD_CB pURCMD_KSTOP;
	tUART_CMD_CB pURCMD_KCH;
	tUART_CMD_CB pURCMD_KREC;
	tUART_CMD_CB pURCMD_QUERY;	
}sUART_CMD_CB;

extern int UART_CMD_CB_Init(void);




extern int URCMD_CB_KU(void* pDat,u32 Len);
extern int URCMD_CB_KD(void* pDat,u32 Len);
extern int URCMD_CB_KL(void* pDat,u32 Len);
extern int URCMD_CB_KR(void* pDat,u32 Len);
extern int URCMD_CB_KE(void* pDat,u32 Len);
extern int URCMD_CB_KDEL(void* pDat,u32 Len);
extern int URCMD_CB_KPLAY(void* pDat,u32 Len);
extern int URCMD_CB_KMENU(void* pDat,u32 Len);
extern int URCMD_CB_KESC(void* pDat,u32 Len);
extern int URCMD_CB_KSTOP(void* pDat,u32 Len);
extern int URCMD_CB_KCH(void* pDat,u32 Len);
extern int URCMD_CB_KREC(void* pDat,u32 Len);
extern int URCMD_CB_QUERY(void* pDat,u32 Len);



tUART_CMD_CB UART_CMD_CB[UART_CMD_NUM]=
{
	URCMD_CB_KU,
	URCMD_CB_KD,
	URCMD_CB_KL,
	URCMD_CB_KR,
	URCMD_CB_KE,
	URCMD_CB_KDEL,
	URCMD_CB_KPLAY,
	URCMD_CB_KMENU,
	URCMD_CB_KESC,
	URCMD_CB_KSTOP,
	URCMD_CB_KCH,
	URCMD_CB_KREC,
	URCMD_CB_QUERY,
};


static char* UART_CMD_TXT[UART_CMD_NUM]=
{

	"KU",
	"KD",
	"KL",
	"KR",
	"KE",
	"KDEL",
	"KPLAY",
	"KMENU",
	"KESC",
	"KSTOP",
	"KCH ",	
	"KREC",	
	"QUERY",
};










int URCMD_CB_KU(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRUP;
		_APP_EXIT_CS_; 		
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KU]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms		


	res=0;
	}
	return res;
}



int URCMD_CB_KD(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRDOWN;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KD]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	


	res=0;
	}
	return res;
}



int URCMD_CB_KR(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRRIGHT;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KR]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	


	res=0;
	}
	return res;
}



int URCMD_CB_KL(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRLEFT;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KL]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms

		
	res=0;
	}
	return res;
}



int URCMD_CB_KE(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IROK;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KE]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	

		
	res=0;
	}
	return res;
}



int URCMD_CB_KDEL(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRDEL;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KDEL]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	

		
	res=0;
	}
	return res;
}



int URCMD_CB_KPLAY(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRPLAY;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KPLAY]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	

		
	res=0;
	}
	return res;
}



int URCMD_CB_KMENU(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRMENU;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KMENU]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms

		
	res=0;
	}
	return res;
}



int URCMD_CB_KESC(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_; 
		KEYIR_MSG = KEY_IRESC;
		_APP_EXIT_CS_; 
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KESC]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms

		
	res=0;
	}
	return res;
}



int URCMD_CB_KSTOP(void* pDat,u32 Len)
{
	int res=-1;
	
	if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;


		_APP_ENTER_CS_;
		KEYIR_MSG = KEY_IRSTOP;
		_APP_EXIT_CS_;
		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KSTOP]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	

		
	res=0;
	}
	return res;
}



int URCMD_CB_KCH(void* pDat,u32 Len)
{
	int res=-1;

	if(pDat)
	{
		u8* cmd=(u8*)pDat;
		u8 u8val;
		u16 u16val;

		cmd+=strlen(UART_CMD_TXT[UART_CMD_KCH]);

		sscanf(cmd, "%d", &u16val);
		printf("{%s%d}\n",UART_CMD_TXT[UART_CMD_KCH],u16val);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms						
		
		if(!UI_isPreviewMode())
		{
			if(UI_isPlaybackMode())
			{
				uiPlaybackStop(GLB_ENA);
				//OSTimeDlyHMSM(0, 0, 0, 200);// delay 200ms		
				OSTimeDlyHMSM(0, 0, 0, 100);// delay 100ms		
			}
			UI_gotoPreviewMode();
			OSTimeDlyHMSM(0, 0, 0, 100);// delay 100ms		
		}
					
		if(u16val<MS_End) 
		{
			_APP_ENTER_CS_; 
			switch(u16val)
			{

			#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)
			case 0:
				//OFF
				setUI.PreviewCH=MS_Dummy;
				break;
			#endif
			case 1:
				//CH1
				setUI.PreviewCH=MS_Door1;
				break;
			#if(!MULTI_CH_DEGRADE_1CH)		
			case 2:
				//CH2
				setUI.PreviewCH=MS_Door2;
				break;
			case 3:
				//CH1+2
				setUI.PreviewCH=MS_Total;
				break;	
			#endif//#if(DERIVATIVE_MODEL!=MODEL_TYPE_PUSH)		
			}
			_APP_EXIT_CS_;	
			DEBUG_UI("Switch to CH%d \r\n", u16val);//debug only
			UI_Switch_PreviewCH(setUI.PreviewCH);			
			
		}
		else
		{//It will return current channel selection if input an invalid channel
		#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)
			printf("{%s(%d)}\n",UART_CMD_TXT[UART_CMD_KCH], (setUI.PreviewCH==MS_Dummy)?0:(setUI.PreviewCH+1));
		#else
			printf("{%s(%d)}\n",UART_CMD_TXT[UART_CMD_KCH], (setUI.PreviewCH+1));
		#endif
		}

		
	res=0;
	}
	return res;
}



int URCMD_CB_KREC(void* pDat,u32 Len)
{
	int res=-1;

	//if(pDat)
	{
		//u8* cmd=(u8*)pDat;
		//u8 u8val;
		//u16 u16val;



		printf("{%s}\n",UART_CMD_TXT[UART_CMD_KREC]);//ACK
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms				
		if(!UI_isPreviewMode())
		{
			if(UI_isPlaybackMode())
			{
				uiPlaybackStop(GLB_ENA);
				//OSTimeDlyHMSM(0, 0, 0, 200);// delay 200ms		
				OSTimeDlyHMSM(0, 0, 0, 100);// delay 100ms		
			}
			UI_gotoPreviewMode();
			OSTimeDlyHMSM(0, 0, 0, 100);// delay 100ms		
		}	
		
		if(UI_isPreviewMode())
		{
			if(setUI.RecMode==RECMODE_MANUAL)
			{
				if(!UI_isVideoRecording())
				{
					_APP_ENTER_CS_; 
					KEYIR_MSG = KEY_IROK;
					_APP_EXIT_CS_; 
				}
			
			}
			else
			{
				if(!UI_isVideoRecording())
				{
					_APP_ENTER_CS_; 
					KEYIR_MSG = KEY_IROK;
					_APP_EXIT_CS_; 
				}

			}

		}


		
	res=0;
	}
	return res;
}





int URCMD_CB_QUERY(void* pDat,u32 Len)
{
	int res=-1;

	//if(pDat)
	{
		char strTmp[128];
		char strMode[32];
		char strCH[16];
							

		switch(setUI.PreviewCH)
		{
			case MS_Door1:
			{
				sprintf(strCH,"1");
			}
			break;
		#if(!MULTI_CH_DEGRADE_1CH)	
			case MS_Door2:
			{
				sprintf(strCH,"2");
			}
			break;
			case MS_Total:
			{
				sprintf(strCH,"1+2");// 3
			}
			break;
			#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)
			case MS_Dummy:
			{
				sprintf(strCH,"0FF");// 0
			}
			break;			
			#endif
		#endif
		}
		

		UI_APP_OP_MODE_GET(strMode);

		sprintf(strTmp,"{%s[MODE:%s,CH:%s]}",UART_CMD_TXT[UART_CMD_QUERY],strMode,strCH);
		_APP_ENTER_CS_; 
		printf("%s\n",strTmp);//ACK		
		_APP_EXIT_CS_; 
		OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms

		
	res=0;
	}
	return res;
}

#endif //#if (YD_UART_PROTOCOL)


#endif //#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)

#endif //#if(HW_BOARD_OPTION == MR6730_AFN)


u32 H264_Special_CMD=0x121;

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
void uiGetSEPTESTUartCmd(u8 *pcString)
{
    u8 idx=0, len=0, checksum=0;
    u8 i;

    while(1)
    {
        idx=len=checksum=0;

        pcString[idx++] = receive_char(UART_2_ID);
        if(pcString[0] != 0x69)
            continue;
        pcString[idx++] = receive_char(UART_2_ID);
        if(pcString[1] != 0x55)
            continue;

        /*data*/
        for(i = 0; i < 2; i++)
        {
            pcString[idx] = receive_char(UART_2_ID);
            checksum += pcString[idx];
            idx++;
        }
        checksum += (0x69+0x55);

        /*checksum */
        pcString[idx] = receive_char(UART_2_ID);

        if ((checksum & 0xFF) != pcString[idx])
            continue;
        else
            break;
    }
#if 0
    DEBUG_GREEN("GET VOC_DUST TEST MSG: [0x%02x,0x%02x,0x%02x,0x%02x,0x%02x]\n", pcString[0], pcString[1], pcString[2], pcString[3], pcString[4]);
#endif
}
void uiParseSEPTESTCmd(u8 *cmd)
{
    if (rfiuVocDustMode == 1)
        DEBUG_UI("Send VOC Val\r\n");
    else
        DEBUG_UI("Send Dust Val\r\n");

    #if 0
    DEBUG_GREEN("PUT VOC_DUST TEST MSG: [0x%02x,0x%02x]\n", cmd[2], cmd[3]);
    #endif
    rfiuPutTXTemHumPM(cmd[2], cmd[3], 0, 0);
}
void uiGetSEPUartCmd(u8* cmd)
{
    cmd[0] = receive_char(UART_2_ID);
    #if 0
    DEBUG_UI("uiGetSEPUartCmd = %02x\r\n",cmd[0]);
    #endif
}
    #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) && (UI_PROJ_OPT == 0))
    void uiParseSEPCmd(u8* cmd)
    {
        static u8 humiVal = 0; 
        static u8 tempVal = 0;
        static u8 dustVal = 0;

        if (cmd[0] == 0x0D)
        {
            /*No Insert Sensor*/
            //DEBUG_UI("No Insert Sensor \r\n");
            rfiuPutTXTemHumPM(0, 0, 0, 0);
            return;
        }
        if ((cmd[0] >= 0x80) && (cmd[0] <= 0xDF)) /* 0% - 95% */
        {
            /*Humidity*/
            if (humiVal != (cmd[0] - 0x80))
                humiVal = (cmd[0] - 0x80);        
        }
        else if ((cmd[0] >= 0x0E) && (cmd[0] <= 0x7A)) /* 14F - 122F */
        {
            /*Temperature*/
            if (tempVal != (cmd[0] - 0x0))
                tempVal = (cmd[0] - 0x0); 
        }
        else if ((cmd[0] >= 0x01) && (cmd[0] <= 0x03))
        {
            /*Dust */
            switch(cmd[0])
            {
                case 0x01: /*Good*/
                    dustVal = cmd[0];
                    break;
                case 0x02: /*Normal*/
                    dustVal = cmd[0];
                    break;
                case 0x03: /*Bad*/
                    dustVal = cmd[0];
                    break;
            }
            if (dustVal != cmd[0])
                dustVal = cmd[0];
        }
        else
            DEBUG_UI("uiParseSEPCmd ERROR \r\n");
        //DEBUG_UI("uiParseSEPCmd: Temp(0x%x), Hum(0x%x), dustVal(0x%x)\n",tempVal,humiVal,dustVal);
        if (humiVal && tempVal && dustVal)
            rfiuPutTXTemHumPM(tempVal, humiVal, dustVal, 0);
    }
    #else
    void uiParseSEPCmd(u8* cmd)
    {
        static u8 tempVal = 0;

        if ((cmd[0] >= 0x0E) && (cmd[0] <= 0x7A)) /* 14F - 122F */
        {
            /*Temperature*/
            if (tempVal != (cmd[0] - 0x0))
                tempVal = (cmd[0] - 0x0);
            rfiuPutTXTemHumPM(tempVal, 0, 0, 0);
            //DEBUG_UI("uiParseSEPCmd: Temp(0x%x)\n",tempVal);
        }
        else
            DEBUG_UI("uiParseSEPCmd ERROR \r\n");
    }
    #endif  /*end of if((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) && (UI_PROJ_OPT == 0))*/
#endif

#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
void uiCmdPareRDI_MCU(u8* cmd)
{
    u8  SendCmd;
    if(!strcmp((char*)cmd,"UP"))
    {
        
        DEBUG_UI("uiCmdPareRDI_MCU Set UP\n");
        SendCmd = 0x01;
        sendchar(RDI_MCU_UART_ID, &SendCmd);
    }
    else if(!strcmp((char*)cmd,"DOWN"))
    {
        DEBUG_UI("uiCmdPareRDI_MCU Set DOWN\n");
        SendCmd = 0x02;
        sendchar(RDI_MCU_UART_ID, &SendCmd);
    }
    else if(!strcmp((char*)cmd,"LEFT"))
    {
        DEBUG_UI("uiCmdPareRDI_MCU Set DOWN\n");
        SendCmd = 0x04;
        sendchar(RDI_MCU_UART_ID, &SendCmd);
    }
    else if(!strcmp((char*)cmd,"RIGHT"))
    {
        DEBUG_UI("uiCmdPareRDI_MCU Set DOWN\n");
        SendCmd = 0x08;
        sendchar(RDI_MCU_UART_ID, &SendCmd);
    }
}

#endif
u8 uiCmdPareCmd(u8* cmd)
{
    int value;
	u8 err;
    static u32 motor_cnt = 0;
    DEBUG_UI("uiParseUartCmd Get Cmd %s\n", cmd);
	
#if(HW_BOARD_OPTION == MR6730_AFN)
			
	#if AFN_DEBUG	
	if(strcmp((char*)cmd, "KAT") == 0){

		DEBUG_UI("AudioTrigger(%d,%d)\n", AudioTrigger[0], AudioTrigger[1]);
	}
	#endif
	

		
#if (USE_UART_CMD_CTRL) 


	#if 1 	
	//debug message subset script of  'K' command
	if(!strncmp((char*)cmd,"KDMSG ", strlen("KDMSG ")))
	{
		ui_DMSG_Cmd(cmd+strlen("KDMSG "));
		MyHandler.WhichKey = UI_KEY_NONE;
	}else	
	#endif  
		
	
		
	
		
	#if ( (DERIVATIVE_MODEL==MODEL_TYPE_SWG)&&(FW_RESTRICTED) )
		//
		//not support virtual key
		//
	#else
	
		#if (DERIVATIVE_MODEL==MODEL_TYPE_YD)
		
			#if (YD_UART_PROTOCOL)	
			//-----------------------
			// YD_UART_PROTOCOL
			//-----------------------
			{
				//u8 u8val;
				//u16 u16val;
	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_QUERY]) == 0){
					UART_CMD_CB[UART_CMD_QUERY](NULL,NULL);	
				}else	
				if (!strncmp((char*)cmd, UART_CMD_TXT[UART_CMD_KCH], strlen(UART_CMD_TXT[UART_CMD_KCH])))
				{
					UART_CMD_CB[UART_CMD_KCH](cmd,NULL);	
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KREC]) == 0){		
					UART_CMD_CB[UART_CMD_KREC](NULL,NULL);					
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KU]) == 0){ 	
					UART_CMD_CB[UART_CMD_KU](cmd,NULL);										
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KD]) == 0){ 	
					UART_CMD_CB[UART_CMD_KD](cmd,NULL);						
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KL]) == 0){ 
					UART_CMD_CB[UART_CMD_KL](cmd,NULL);							
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KR]) == 0){ 
					UART_CMD_CB[UART_CMD_KR](cmd,NULL);						
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KE]) == 0){ 	
					UART_CMD_CB[UART_CMD_KE](cmd,NULL);						
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KDEL]) == 0){	
					UART_CMD_CB[UART_CMD_KDEL](cmd,NULL);						
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KPLAY]) == 0){		
					UART_CMD_CB[UART_CMD_KPLAY](cmd,NULL);						
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KMENU]) == 0){	
					UART_CMD_CB[UART_CMD_KMENU](cmd,NULL);							
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KESC]) == 0){	
					UART_CMD_CB[UART_CMD_KESC](cmd,NULL);							
				}else	
				if(strcmp((char*)cmd, UART_CMD_TXT[UART_CMD_KSTOP]) == 0){
					UART_CMD_CB[UART_CMD_KSTOP](cmd,NULL);								
				}
	
	
			}
			#else
	
			//-----------------------
			// STD_UART_PROTOCOL
			//-----------------------
	
			if(strcmp((char*)cmd, "QUERY") == 0){
				#if(RESP_WITH_ACK) 
				printf("{QUERY}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms
				#endif
				uiOsdPrintStates();
			}else		
			if(strcmp((char*)cmd, "KU") == 0){
				#if(RESP_WITH_ACK)
				printf("{KU}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	
				#endif
				KEYIR_MSG = KEY_IRUP;
			}else			
			if(strcmp((char*)cmd, "KD") == 0){
				#if(RESP_WITH_ACK)
				printf("{KD}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	
				#endif
				KEYIR_MSG = KEY_IRDOWN;
			}else			
			if(strcmp((char*)cmd, "KL") == 0){
				#if(RESP_WITH_ACK)
				printf("{KL}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	
				#endif
				KEYIR_MSG = KEY_IRLEFT;
			}else			
			if(strcmp((char*)cmd, "KR") == 0){
				#if(RESP_WITH_ACK)
				printf("{KR}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms			
				#endif
				KEYIR_MSG = KEY_IRRIGHT;
			}else			
			if(strcmp((char*)cmd, "KE") == 0){
				#if(RESP_WITH_ACK)
				printf("{KE}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms			
				#endif
				KEYIR_MSG = KEY_IROK;
			}else			
			if(strcmp((char*)cmd, "KDEL") == 0){
				#if(RESP_WITH_ACK)
				printf("{KDEL}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	
				#endif
				KEYIR_MSG = KEY_IRDEL;
			}else			
			if(strcmp((char*)cmd, "KPLAY") == 0){
				#if(RESP_WITH_ACK)
				printf("{KPLAY}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms			
				#endif
				KEYIR_MSG = KEY_IRPLAY;
			}else			
			if(strcmp((char*)cmd, "KMENU") == 0){
				#if(RESP_WITH_ACK)
				printf("{KMENU}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms			
				#endif
				KEYIR_MSG = KEY_IRMENU;
			}else			
			if(strcmp((char*)cmd, "KESC") == 0){
				#if(RESP_WITH_ACK)
				printf("{KESC}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms	
				#endif
				KEYIR_MSG = KEY_IRESC;
			}else			
			if(strcmp((char*)cmd, "KSTOP") == 0){
				#if(RESP_WITH_ACK)
				printf("{KSTOP}\n");//ACK
				OSTimeDlyHMSM(0, 0, 0, 10);// delay 10ms		
				#endif
				KEYIR_MSG = KEY_IRSTOP;
			}	
			#endif
	
	
		
		#else


		
			//---------- for AFN -----------
			if(strcmp((char*)cmd, "KQURRY") == 0){
			 //DEBUG_UI("========kqurry=======\n");
			 //printf("KQURRY");
				uiOsdPrintStates();
			}else
			#if 1
			if(strcmp((char*)cmd, "KPWR") == 0){
				KEYIR_MSG = KEY_IRPWR;
			}else	
			#endif	
			if(strcmp((char*)cmd, "KU") == 0){
				KEYIR_MSG = KEY_IRUP;
			}else
		
			if(strcmp((char*)cmd, "KD") == 0){
				KEYIR_MSG = KEY_IRDOWN;
			}else
		
			if(strcmp((char*)cmd, "KL") == 0){
				KEYIR_MSG = KEY_IRLEFT;
			}else
		
			if(strcmp((char*)cmd, "KR") == 0){
				KEYIR_MSG = KEY_IRRIGHT;
			}else
		
			if(strcmp((char*)cmd, "KE") == 0){
				KEYIR_MSG = KEY_IROK;
			}else
		
			if(strcmp((char*)cmd, "KDEL") == 0){
				KEYIR_MSG = KEY_IRDEL;
			}else
		
			if(strcmp((char*)cmd, "KPLAY") == 0){
				KEYIR_MSG = KEY_IRPLAY;
			}else
		
			if(strcmp((char*)cmd, "KMENU") == 0){
				KEYIR_MSG = KEY_IRMENU;
			}else
		
			if(strcmp((char*)cmd, "KESC") == 0){
				KEYIR_MSG = KEY_IRESC;
			}else
		
			if(strcmp((char*)cmd, "KSTOP") == 0){
				KEYIR_MSG = KEY_IRSTOP;
			}
		#endif
	#endif//#if ( (DERIVATIVE_MODEL==MODEL_TYPE_SWG)&&(FW_RESTRICTED) )
	
	
	//...
	
	
	if(KEYIR_MSG != 0){
		DEBUG_UI("uart_cmd(KEYIR_MSG=%d)\n", KEYIR_MSG);//debug
		MyHandler.WhichKey = UI_KEY_NONE;
	}	
	
		
	
		
#endif//#if (USE_UART_CMD_CTRL)
		
	
	
	
	
	
#else
	//orig-code

    if(!strcmp((char*)cmd,"KU"))
        MyHandler.WhichKey   = UI_KEY_UP;
    else if(!strcmp((char*)cmd,"KD"))
        MyHandler.WhichKey   = UI_KEY_DOWN;
    else if(!strcmp((char*)cmd,"KE"))
        MyHandler.WhichKey   = UI_KEY_ENTER;
	else if(!strncmp((char*)cmd,"KRSF ", strlen("KRSF ")))
    {
    	{
    		u8 *tmpBuf = SPIConfigBuf;
    		u32 j, sAddr;
    		s32 cLen;
        	sscanf((const char *)cmd, "KRSF %x %d", &sAddr, &cLen);
        	if(cLen%0x100)
        		cLen = ((cLen/0x100)+1)*0x100;
			for(;cLen > 0; cLen -= 0x100, sAddr += 0x100)
			{
				spiReadData(tmpBuf, sAddr, 0x100);
	    		{
	    			for(j = 0; j < 0x100; j++)
		    		{
		    			if(j && !(j%16))
		    				printf("\n");
		    			printf("%02x ", tmpBuf[j]);
		    		}
	    		}	
			}
		}
    }
    else if(!strncmp((char*)cmd, "KE_", strlen("KE_")))
    {
        cmd+=strlen("KE_");
        sscanf((char*)cmd, "%08x", &H264_Special_CMD);
        printf("H264_Special_CMD = 0x%08x\n",H264_Special_CMD);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if(!strcmp((char*)cmd,"KMENU"))
        MyHandler.WhichKey   = UI_KEY_MENU;
    else if(!strcmp((char*)cmd,"KMODE"))
        MyHandler.WhichKey   = UI_KEY_MODE;
    else if(!strcmp((char*)cmd,"KTV"))
    {
        MyHandler.WhichKey  = UI_KEY_TVOUT_DET; /*TV OUT*/
        DEBUG_UI("UI_KEY_TVOUT_DET \r\n");
    }
    else if(!strcmp((char*)cmd,"KNTSC"))
    {
        MyHandler.WhichKey  = UI_KEY_NONE; /*TV OUT*/
        iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
        DEBUG_UI("UI_KEY_NTSC_OUT \r\n");
    }
#if PLAYBEEP_TEST
	else if(!strncmp((char*)cmd,"KBEEP", strlen("KBEEP")))
    {
    	cmd+=strlen("KBEEP");
        sscanf((char*)cmd, "%d", &value);
        MyHandler.WhichKey  = UI_KEY_NONE; /*TV OUT*/
        sysbackSetEvt(SYS_BACK_PLAY_BEEP, value);
        DEBUG_UI("KBEEP \r\n");
    }
#endif
    else if(!strcmp((char*)cmd,"KPAL"))
    {
        MyHandler.WhichKey  = UI_KEY_NONE; /*TV OUT*/
        iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
        DEBUG_UI("UI_KEY_PAL_OUT \r\n");
    }
    else if (!strcmp((char*)cmd,"KL"))
        MyHandler.WhichKey   = UI_KEY_LEFT;
    else if (!strcmp((char*)cmd,"KR"))
        MyHandler.WhichKey   = UI_KEY_RIGHT;
    else if (!strcmp((char*)cmd,"KDEL"))
        MyHandler.WhichKey   = UI_KEY_DELETE;
    else if (!strcmp((char*)cmd,"KPLAY"))
        MyHandler.WhichKey   = UI_KEY_PLAY;
    else if (!strcmp((char*)cmd,"KSTOP"))
        MyHandler.WhichKey   = UI_KEY_STOP;
    else if (!strcmp((char*)cmd,"KREC"))
        MyHandler.WhichKey   = UI_KEY_REC;
    else if (!strcmp((char*)cmd,"KIRS"))
    {
        Timer_IR_TX();  //´£«ý York IR 0.5ms timer
        DEBUG_UI("timer KIR start\r\n");
    }
    else if(!strcmp((char*)cmd,"KOAUDIO"))
	{    
		rfAudioEnable = 1;
		MyHandler.WhichKey   = UI_KEY_NONE;
	}
    else if(!strcmp((char*)cmd,"KCAUDIO"))
	{   
		rfAudioEnable = 0;
		MyHandler.WhichKey   = UI_KEY_NONE;
	}

#endif //#if
#if USB2WIFI_SUPPORT
    else if (!strncmp((char*)cmd,"KSANP", strlen("KSANP")))
    {
        Snapshot_Error = 1;
        DEBUG_UI("Snapshot_Error\r\n");
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
#endif
#if ((HW_BOARD_OPTION == MR8200_RX_DB2)||(HW_BOARD_OPTION == MR8200_RX_DB3))
    else if (!strncmp((char*)cmd,"KMASK ", strlen("KMASK ")))
    {
        sscanf((const char *)cmd, "KMASK %d", &value);
        uiOsdDrawMaskAreaSetting(0, value);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }

#endif
#if((SUPPORT_TOUCH) && (UI_VERSION == UI_VERSION_TRANWO))
    else if (!strncmp((char*)cmd,"KTGOTO ", strlen("KTGOTO ")))
    {
        MyHandler.WhichKey   = UI_KEY_GOTO;  
        sscanf((const char *)cmd, "KTGOTO %d", &value);
        TouchExtKey=value;
    }
    else if (!strncmp((char*)cmd,"KTE ", strlen("KTE ")))
    {
        MyHandler.WhichKey   = UI_KEY_ENTER;  
        sscanf((const char *)cmd, "KTE %d", &value);
        TouchExtKey=value;
    }
    else if (!strncmp((char*)cmd,"KTDEL ", strlen("KTDEL ")))
    {
        MyHandler.WhichKey   = UI_KEY_DELETE;  
        sscanf((const char *)cmd, "KTDEL %d", &value);
        TouchExtKey=value;
    }
    else if (!strncmp((char*)cmd,"KTREC ", strlen("KTREC ")))
    {
        MyHandler.WhichKey   = UI_KEY_REC;  
        sscanf((const char *)cmd, "KTREC %d", &value);
        TouchExtKey=value;
    }
    else if (!strncmp((char*)cmd,"KTMAIN ", strlen("KTMAIN ")))
    {
        MyHandler.WhichKey   = UI_KEY_MAIN;  
        sscanf((const char *)cmd, "KTMAIN %d", &value);
        TouchExtKey=value;
    }
    else if (!strncmp((char*)cmd,"KTSCH ", strlen("KTSCH ")))
    {
        u16 x, y;
        MyHandler.WhichKey   = UI_KEY_ENTER;  
        sscanf((const char *)cmd, "KTSCH %d %d", &x, &y);
    }
    else if (!strncmp((char*)cmd,"KCH1 ", strlen("KCH1 ")))
    {
        MyHandler.WhichKey   = UI_KEY_CH1;     
    }
    else if (!strncmp((char*)cmd,"KCH2 ", strlen("KCH2 ")))
    {
        MyHandler.WhichKey   = UI_KEY_CH2;     
    }
    else if (!strncmp((char*)cmd,"KCH3 ", strlen("KCH3 ")))
    {
        MyHandler.WhichKey   = UI_KEY_CH3;     
    }
    else if (!strncmp((char*)cmd,"KCH4 ", strlen("KCH4 ")))
    {
        MyHandler.WhichKey   = UI_KEY_CH4;     
    }    
    
#endif
#if MULTI_CHANNEL_SUPPORT
    else if (!strcmp((char*)cmd,"KCHU"))
    {
        do
        {
            sysVideoInCHsel = (sysVideoInCHsel+1) % MULTI_CHANNEL_MAX;
        }while( (MULTI_CHANNEL_SEL & (1<<sysVideoInCHsel))==0 );
        DEBUG_UI("Video Channel increase: %d\r\n",sysVideoInCHsel);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KCHD"))
    {
        do
        {
           if(sysVideoInCHsel ==0)
              sysVideoInCHsel= MULTI_CHANNEL_MAX;
           sysVideoInCHsel = (sysVideoInCHsel-1) % MULTI_CHANNEL_MAX;
        }while((MULTI_CHANNEL_SEL & (1<<sysVideoInCHsel))==0);
        DEBUG_UI("Video Channel decrease: %d\r\n",sysVideoInCHsel);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KCIUQUAD"))
    {
         if(sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
            sysCameraMode = SYS_CAMERA_MODE_CIU_QUADSCR;
    }
    else if (!strcmp((char*)cmd,"KCIUFULL"))
    {
         if(sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
            sysCameraMode = SYS_CAMERA_MODE_PREVIEW;
    }
 #if (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU)
    else if (!strcmp((char*)cmd,"KGFUTEST"))
    {
         sysCameraMode = SYS_CAMERA_MODE_GFU_TESTSCR;
         gfuTest_Rect_Cmd(0,0,640,480,0x00008080);
    }
 #endif
    else if (!strcmp((char*)cmd,"KPIP0"))
    {
        sysPIPMain  = PIP_MAIN_NONE;
        uiMenuEnterPreview(0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KPIP1"))
    {
        sysPIPMain      = PIP_MAIN_CH1;
        sysVideoInCHsel = 2;
        uiMenuEnterPreview(0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KPIP2"))
    {
        sysPIPMain      = PIP_MAIN_CH2;
        sysVideoInCHsel = 1;
        uiMenuEnterPreview(0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KSNAP"))
    {
        sysSnapshot_OnPreview(0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
  #if DUAL_MODE_DISP_SUPPORT
    else if (!strcmp((char*)cmd,"KDUAL"))
    {
        sysVideoInCHsel     = 1;
        sysDualModeDisp     = 1;
        DEBUG_UI("Dual mode display enable\n");
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KFULL"))
    {
        sysDualModeDisp     = 0;
        DEBUG_UI("Dual mode display disable\n");
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
  #endif
  #if MULTI_CHANNEL_VIDEO_REC
    else if (!strncmp((char*)cmd,"KREC ", strlen("KREC ")))
    {
        int VideoChannelID;
        sscanf((const char *)cmd, "KREC %d", &VideoChannelID);
        DEBUG_UI("Parsing command: KREC %d\n", VideoChannelID);
        MultiChannelSysCaptureVideoOneCh(VideoChannelID);
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KSTOP ", strlen("KSTOP ")))
    {
        int VideoChannelID;
        sscanf((const char *)cmd, "KSTOP %d", &VideoChannelID);
        DEBUG_UI("Parsing command: KSTOP %d\n", VideoChannelID);
        MultiChannelSysCaptureVideoStopOneCh(VideoChannelID);
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KEVENT_ALARM ", strlen("KEVENT_ALARM "))) // Alarm event detected
    {
        int VideoChannelID;
        sscanf((const char *)cmd, "KEVENT_ALARM %d", &VideoChannelID);
        DEBUG_UI("Parsing command: KEVENT_ALARM %d\n", VideoChannelID);
        VideoClipOption[VideoChannelID].AlarmDetect = 1;
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
   #if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
    else if (!strncmp((char*)cmd,"KEVENT_MD ", strlen("KEVENT_MD ")))   // Motion event detected
    {
        int VideoChannelID;
        sscanf((const char *)cmd, "KEVENT_MD %d", &VideoChannelID);
        DEBUG_UI("Parsing command: KEVENT_MD %d\n", VideoChannelID);
        VideoClipOption[VideoChannelID].MD_Diff = 1;
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
   #endif   // #if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
   #if(G_SENSOR_DETECT)
    else if (!strncmp((char*)cmd,"KEVENT_GSENSOR ", strlen("GSensorEvent ")))   // Gsensor event detected
    {
        int VideoChannelID;
        sscanf((const char *)cmd, "GSensorEvent %d", &VideoChannelID);
        DEBUG_UI("Parsing command: GSensorEvent %d\n", VideoChannelID);
        VideoClipOption[VideoChannelID].GSensorEvent = 1;
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
   #endif   // #if(G_SENSOR_DETECT)
  #endif    // #if MULTI_CHANNEL_VIDEO_REC
#endif      // #if MULTI_CHANNEL_SUPPORT
    else if (!strcmp((char*)cmd,"KDLLSTA"))
    {
        DEBUG_UI("CHIP DLL Status=%d\n",(SdramDFI_PHYB>>16) );
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#if(HWPIP_SUPPORT)
    else if(!strncmp((char*)cmd,"KHPO ", strlen("KHPO ")))
    {
        HWPIP_OpenTest(cmd+strlen("KHPO "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KHPC"))
    {
        HWPIP_Close();
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPP ", strlen("KHPP ")))
    {
        HWPIP_PositionTest(cmd+strlen("KHPP "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPS ", strlen("KHPS ")))
    {
        HWPIP_SizeTest(cmd+strlen("KHPS "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPA ", strlen("KHPA ")))
    {
        HWPIP_AlphaBlendingTest(cmd+strlen("KHPA "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPD ", strlen("KHPD ")))
    {
        HWPIP_DownSampleTest(cmd+strlen("KHPD "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPE ", strlen("KHPE ")))
    {
        HWPIP_EdgeColorSetTest(cmd+strlen("KHPE "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPES ", strlen("KHPES ")))
    {
        HWPIP_EdgeColorEnTest(cmd+strlen("KHPES "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHPB ", strlen("KHPB ")))
    {
        HWPIP_BurstBitwidthSetTest(cmd+strlen("KHPB "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KHP TEST2"))
    //else if(!strncmp((char*)cmd,"KHP TEST1", strlen("KHP TEST1")))
    {
        uiHWPIP_TEST2();
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KHP TEST3 ", strlen("KHP TEST3 ")))
    {
        uiHWPIP_TEST3(cmd+strlen("KHP TEST3 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }


#endif

#if HW_MD_SUPPORT
    else if(!strncmp((char*)cmd,"KMDTEST ", strlen("KMDTEST ")))
    {
        mdu_TestSensitivity(cmd+strlen("KMDTEST "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }

#endif

#if (VMDSW && HW_MD_SUPPORT)
    else if(!strncmp((char*)cmd,"KMDSWTEST ", strlen("KMDSWTEST ")))
    {
        VMDSW_TestSensitivity(cmd+strlen("KMDSWTEST "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif

#if iHome_LOG_TEST
    else if(!strncmp((char*)cmd,"KLOG ", strlen("KLOG ")))
    {
        dcfLogTest(cmd+strlen("KLOG "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif

#if RX_SNAPSHOT_SUPPORT
    else if(!strncmp((char*)cmd,"KPHOTOSCAN ", strlen("KPHOTOSCAN ")))
    {
        sscanf((char*)cmd, "KPHOTOSCAN %d", &value);    
        dcfPhotoFileInit(value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif


#if RFIU_SUPPORT
    else if (!strcmp((char*)cmd,"KRFU"))
    {
        DEBUG_UI("\n-->Do RF Channel Change_%d\n",sysRFRxInMainCHsel);
        uiSetRfChangeChannel(UI_MENU_CHANNEL_ADD);
        DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFQUP"))
    {
        uiSetRfChangeAudio_QuadMode(UI_MENU_CHANNEL_ADD);
        DEBUG_UI("RF Audio Channel increase: %d\r\n",sysRFRxInMainCHsel);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFSYNC1"))
    {
        rfiuForceResync(0);
        DEBUG_UI("RF Resync: %d\r\n",0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFSYNC2"))
    {
        rfiuForceResync(1);
        DEBUG_UI("RF Resync: %d\r\n",1);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFADOSW")) //Only vailid in dual mode
    {
        uiSetRfSwAudio_DualMode(UI_MENU_CHANNEL_ADD);
        //DEBUG_UI("RF Audio Channel increase: %d\r\n",(sysRFRxInMainCHsel+1) & 0x01 );
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFPTZU"))
    {
        uiSetRfChgPTZ_CH(UI_MENU_CHANNEL_ADD);
        DEBUG_UI("RF PTZ Channel increase: %d\r\n",sysRF_PTZ_CHsel);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFQUAD"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_QUAD);
        MyHandler.WhichKey   = UI_KEY_NONE;    }
    else if (!strcmp((char*)cmd,"KRFMASK"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_ENTER_MASKAREA);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
 #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
    else if (!strcmp((char*)cmd,"KRFZOOMON"))
    {
        sysEnZoom=1;
    #if RFRX_FULLSCR_HD_SINGLE
        iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth/2,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH*2);
    #else
        iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
    #endif
        IduIntCtrl |= IDU_FTCINT_ENA;

        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFZOOMOFF"))
    {
        sysEnZoom=0;
    #if RFRX_FULLSCR_HD_SINGLE
        iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth/2,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH);
    #else
        iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    #endif
        IduIntCtrl |= IDU_FTCINT_ENA;
        MyHandler.WhichKey   = UI_KEY_NONE;
    }

 #endif
 #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    else if (!strcmp((char*)cmd,"MOTOR_U"))
    {
        motor_cnt++;
        MotorSet(1);
        DEBUG_UI("MOTOR_U cnt = %d\n",motor_cnt);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"MOTOR_D"))
    {
        motor_cnt++;
        MotorSet(2);
        DEBUG_UI("MOTOR_D cnt = %d\n",motor_cnt);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"MOTOR_L"))
    {
        motor_cnt++;
        MotorSet(3);
        DEBUG_UI("MOTOR_L cnt = %d\n",motor_cnt);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"MOTOR_R"))
    {
        motor_cnt++;
        MotorSet(4);
        DEBUG_UI("MOTOR_L cnt = %R\n",motor_cnt);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
 #endif
	else if (!strcmp((char*)cmd,"KRFDUAL"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_DUAL);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
	else if (!strcmp((char*)cmd,"KRFFULL"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_FULL);
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFCAMON ", strlen("KRFCAMON ")))
    {
        rfiuCamOnOffCmd(cmd+strlen("KRFCAMON "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFWOR_MODE ",strlen("KRFWOR_MODE ")))
    {
        rfiuCamSleepCmd(cmd+strlen("KRFWOR_MODE "));
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFFCC_MODE"))
    {
        sysback_RF_SetEvt(SYS_BACKRF_FCC_DIRECT_TXRX, 0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCC_UNIT ", strlen("KRFFCC_UNIT ")))
    {
        rfiuFCCUnitSel(cmd+strlen("KRFFCC_UNIT "));
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCC_TX0 ", strlen("KRFFCC_TX0 ")))
    {
        rfiuFCCTX0Cmd2(cmd+strlen("KRFFCC_TX0 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCC_TX1 ", strlen("KRFFCC_TX1 ")))
    {
        rfiuFCCTX1Cmd2(cmd+strlen("KRFFCC_TX1 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCC_RX0 ", strlen("KRFFCC_RX0 ")))
    {
        rfiuFCCRX0Cmd(cmd+strlen("KRFFCC_RX0 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCC_RX1 ", strlen("KRFFCC_RX1 ")))
    {
        rfiuFCCRX0Cmd(cmd+strlen("KRFFCC_RX1 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCCTX0 ", strlen("KRFFCCTX0 ")))
    {
        rfiuFCCTX0Cmd(cmd+strlen("KRFFCCTX0 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCCTX1 ", strlen("KRFFCCTX1 ")))
    {
        rfiuFCCTX1Cmd(cmd+strlen("KRFFCCTX1 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCCRX0 ", strlen("KRFFCCRX0 ")))
    {
        rfiuFCCRX0Cmd(cmd+strlen("KRFFCCRX0 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFCCRX1 ", strlen("KRFFCCRX1 ")))
    {
        rfiuFCCRX1Cmd(cmd+strlen("KRFFCCRX1 "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFMDSEN", strlen("KRFMDSEN")))
    {
        int cam=0, pixel_diff=0, block=0;
        sscanf((char*)cmd, "KRFMDSEN %d %d %d", &cam, &pixel_diff,&block);

        DEBUG_UI("Cam%d motion pixel diff:%d block margin:%d \n",cam,pixel_diff, block);
        MD_SensitivityConfTab[0][0]=pixel_diff;
        MD_SensitivityConfTab[0][1]=block;
        MD_SensitivityConfTab[1][0]=pixel_diff;
        MD_SensitivityConfTab[1][1]=block;
        MD_SensitivityConfTab[2][0]=pixel_diff;
        MD_SensitivityConfTab[2][1]=block;
        rfiu_SendTxMdSense(cam);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
 #if RF_PAIR_EN
    else if(!strncmp((char*)cmd,"KRFPAR1", strlen("KRFPAR1")))
    {
    #if( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        sysRFRxInMainCHsel=0;
    #endif
        uiRFPairTest(cmd+strlen("KRFPAR1"),0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPAR2", strlen("KRFPAR2")))
    {
    #if( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        sysRFRxInMainCHsel=1;
    #endif
        uiRFPairTest(cmd+strlen("KRFPAR2"),1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPAR3", strlen("KRFPAR3")))
    {
        uiRFPairTest(cmd+strlen("KRFPAR3"),2);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPAR4", strlen("KRFPAR4")))
    {
        uiRFPairTest(cmd+strlen("KRFPAR4"),3);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPST1", strlen("KRFPST1")))
    {
        rfiu_PAIR_Stop(0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPST2", strlen("KRFPST2")))
    {
        rfiu_PAIR_Stop(1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPST3", strlen("KRFPST3")))
    {
        rfiu_PAIR_Stop(2);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPST4", strlen("KRFPST4")))
    {
        rfiu_PAIR_Stop(3);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
 #endif
 #if RF_CMD_EN
    else if(!strncmp((char*)cmd,"KRFCMD1 ", strlen("KRFCMD1 ")))
    {
        err=uiRXCMDTest(cmd+strlen("KRFCMD1 "),0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFCMD2 ", strlen("KRFCMD2 ")))
    {
        err=uiRXCMDTest(cmd+strlen("KRFCMD2 "),1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFINFO ", strlen("KRFINFO ")))
    {
        err=uiTXCMDTest(cmd+strlen("KRFINFO "),0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFAUDRET_EN_IIS", strlen("KRFAUDRET_EN_IIS")))
    {
        rfiu_AudioRetONOFF_IIS(1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFAUDRET_DIS_IIS", strlen("KRFAUDRET_DIS_IIS")))
    {
        rfiu_AudioRetONOFF_IIS(0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFAUDRET_EN_APP", strlen("KRFAUDRET_EN_APP")))
    {

        rfiu_AudioRetONOFF_APP(1,0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFAUDRET_DIS_APP", strlen("KRFAUDRET_DIS_APP")))
    {

        rfiu_AudioRetONOFF_APP(0,0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
 #endif

 #if RFIU_TX_WAKEUP_SCHEME
    else if (!strcmp((char*)cmd,"KRFWAKEUP1"))
    {
        DEBUG_UI("==Wake up TX1==\n");
        gRfiuUnitCntl[0].WakeUpTxEn=1;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFWAKEUP2"))
    {
        DEBUG_UI("==Wake up TX2==\n");
        gRfiuUnitCntl[1].WakeUpTxEn=1;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
 #endif
    else if (!strcmp((char*)cmd,"KRFPHOTO"))
    {
        DEBUG_UI("\n-->Capture RF Photo\n");
        sysback_RF_SetEvt(SYS_BACKRF_RFI_CAP_PHOTO,0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }

 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    else if (!strcmp((char*)cmd,"KRFENTWIFI"))
    {
        DEBUG_UI("\n-->TX Enter Wifi mode\n");
//        sysTX8211_EnterWifi(RFWIFI_P2P_QUALITY_HIGH);
            //sysbackSetEvt(SYS_BACK_RFI_TX_ENT_WIFI,RFWIFI_P2P_QUALITY_HIGH);
            sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_ENT_WIFI,RFWIFI_P2P_QUALITY_HIGH);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFQUITWIFI"))
    {
        DEBUG_UI("\n-->TX Quit Wifi mode\n");
//        sysTX8211_LeaveWifi(0);
            //sysbackSetEvt(SYS_BACK_RFI_TX_LEV_WIFI,RFWIFI_P2P_QUALITY_HIGH);
            sysback_RF_SetEvt(SYS_BACKRF_RFI_TX_LEV_WIFI,RFWIFI_P2P_QUALITY_HIGH);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }


 #endif   

#endif


#if 1
    else if(!strcmp((char*)cmd,"KHELP"))
    {
        DEBUG_UI("*******************************************\r\n");
        DEBUG_UI("OSD Command :\n");
        DEBUG_UI("KOSD RESET    :RESET OSD\r\n");
        DEBUG_UI("KOSD EN W0    :Enable OSD L0 Window 0\r\n");
        DEBUG_UI("KOSD EN W1    :Enable OSD L0 Window 1\r\n");
        DEBUG_UI("KOSD EN W2    :Enable OSD L0 Window 2\r\n");
        DEBUG_UI("KOSD EN L1    :Enable OSD L1\r\n");
        DEBUG_UI("KOSD EN L2    :Enable OSD L2\r\n");
        DEBUG_UI("KOSD DIS W0   :Disable OSD L0 Window 0\r\n");
        DEBUG_UI("KOSD DIS W1   :Disable OSD L0 Window 1\r\n");
        DEBUG_UI("KOSD DIS W2   :Disable OSD L0 Window 2\r\n");
        DEBUG_UI("KOSD DIS L1   :Disable OSD L1\r\n");
        DEBUG_UI("KOSD DIS L2   :Disable OSD L2\r\n");
        DEBUG_UI("KOSD SIZE W0 <num1,num2>,<num3,num4>  :num1->Start X, num2->Start Y, num3->End X, num4->End Y\r\n");
        DEBUG_UI("KOSD SIZE W1 <num1,num2>,<num3,num4>  :num1->Start X, num2->Start Y, num3->End X, num4->End Y\r\n");
        DEBUG_UI("KOSD SIZE W2 <num1,num2>,<num3,num4>  :num1->Start X, num2->Start Y, num3->End X, num4->End Y\r\n");
        DEBUG_UI("KOSD SIZE L1 <num1,num2>,<num3,num4>  :num1->Start X, num2->Start Y, num3->End X, num4->End Y\r\n");
        DEBUG_UI("KOSD SIZE L2 <num1,num2>,<num3,num4>  :num1->Start X, num2->Start Y, num3->End X, num4->End Y\r\n");
        DEBUG_UI("KOSD 64COLOR W0|W1|W2|L1|L2           :Draw 8x8 block OSD\r\n");
        DEBUG_UI("KOSD B W0|W1|W2|L1|L2 <W*H> <X,Y> <DATA> : Draw OSD block \r\n");
        DEBUG_UI("KIR ENABLE\r\n");
        DEBUG_UI("KIR DISABLE\r\n");
        DEBUG_UI("KIR RESET ON\r\n");
        DEBUG_UI("KIR RESET OFF\r\n");
        DEBUG_UI("KIR CODE <ABCD>\r\n");
        DEBUG_UI("KIR INT DISABLE");
        DEBUG_UI("KIR INT ENABLE\r\n");
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KOSD ", strlen("KOSD ")))
    {
        uiOsdTest(cmd+strlen("KOSD "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KIR ", strlen("KIR ")))
    {
        uiIRTest(cmd+strlen("KIR "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KTIME ", strlen("KTIME ")))
    {
        uiTimeTest(cmd+strlen("KTIME "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KWD", strlen("KWD")))
    {
		sysForceWDTtoReboot();
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KGPIO ", strlen("KGPIO ")))
    {
        uiTestGPIO(cmd+strlen("KGPIO "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KTIMER ", strlen("KTIMER ")))
    {
        uiTimerTest(cmd+strlen("KTIMER "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KPWM ", strlen("KPWM ")))
    {
        uiPWMTest(cmd+strlen("KPWM "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KPWM"))
    {
        Beep_function(3,200,60,200,200,0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    #if((HW_BOARD_OPTION == A1018_FPGA_BOARD) || (HW_BOARD_OPTION == A1018B_FPGA_BOARD))
    else if(!strncmp((char*)cmd,"KDES ", strlen("KDES ")))
    {
        uiDESTest(cmd+strlen("KDES "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    #endif

#endif
    else if(!strncmp((char*)cmd,"KBIT ", strlen("KBIT ")))
    {
        sscanf((char*)cmd, "KBIT %d", &value);
        uiOsdDrawBitRate(value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KFRAME ", strlen("KFRAME ")))
    {
        sscanf((char*)cmd, "KFRAME %d", &value);
        uiOsdDrawFrameRate(value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KIIC ", strlen("KIIC ")))
    {
        uiBIT1605_TestI2C(cmd+strlen("KIIC "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KIIC1B ", strlen("KIIC1B ")))
    {
        u8 slaveAddr    = 0;
        u8 registAddr   = 0;
        u8 data         = 0;
        cmd    += strlen("KIIC1B ");
        if(!strncmp((char*)cmd,"R ", strlen("R ")))
        {
            cmd    += strlen("R ");
            sscanf((char *)cmd, "%x %x", &slaveAddr, &registAddr);
            i2cRead_Byte(slaveAddr, registAddr, &data);

            DEBUG_UI("SLAVE:%x, regist:%x, data:%x\n", slaveAddr, registAddr, data);
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            cmd    += strlen("W ");
            sscanf((char *)cmd,"%x %x %x", &slaveAddr, &registAddr, &data);

            i2cWrite_Byte(slaveAddr, registAddr, data);
            DEBUG_UI("SLAVE:%x, regist:%x, data:%x\n",slaveAddr,registAddr,data);
        }
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KIIC1201 ", strlen("KIIC1201 ")))
    {
        uiBIT1201G_TestI2C(cmd+strlen("KIIC1201 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2C ", strlen("KI2C ")))
    {
        #if(TV_DECODER == WT8861)
        uiWT8861_TestI2C(cmd+strlen("KI2C "));
        #elif(TV_DECODER == TW9900)
        uiTW9900_TestI2C(cmd+strlen("KI2C "));
		#elif(TV_DECODER == TW9910)
        uiTW9910_TestI2C(cmd+strlen("KI2C "));
		#elif(TV_DECODER == TW2866)
        uiTW2866_TestI2C(cmd+strlen("KI2C "));
        #endif
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KWT6853 ", strlen("KWT6853 ")))
    {
        #if (IO_EXPAND == IO_EXPAND_WT6853)
        uiWT6853_TestI2C(cmd+strlen("KWT6853 "));
        MyHandler.WhichKey = UI_KEY_NONE;
        #endif
    }
    else if (!strncmp((char*)cmd,"KMA86P03 ", strlen("KMA86P03 ")))
    {
        #if (TOUCH_KEY == TOUCH_KEY_MA86P03)
        uiMA86P03_TestI2c(cmd+strlen("KMA86P03 "));
        MyHandler.WhichKey = UI_KEY_NONE;
        #endif
    }

    else if(!strncmp((char*)cmd,"KI2CCON ", strlen("KI2CCON ")))
    {
        ui_contrast_test(cmd+strlen("KI2CCON "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2CBRI ", strlen("KI2CBRI ")))
    {
        ui_brightness_test(cmd+strlen("KI2CBRI "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2CSAT ", strlen("KI2CSAT ")))
    {
        ui_saturation_test(cmd+strlen("KI2CSAT "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2CCH ", strlen("KI2CCH ")))
    {
        ui_change_test(cmd+strlen("KI2CCH "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2CLOCK ", strlen("KI2CLOCK ")))
    {
        ui_I2CLOCK_test(cmd+strlen("KI2CLOCK "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KI2CLCDOFF", strlen("KI2CLCDOFF")))
    {
        gpioSetLevel(1,17,1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#if(TV_ENCODER == CH7025)
    else if (!strncmp((char*)cmd,"KI2C7025 ", strlen("KI2C7025 ")))
    {
        uiCS7025_TestI2C(cmd+strlen("KI2C7025 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
    else if(!strncmp((char*)cmd,"KI2CLCDON", strlen("KI2CLCDON")))
    {
        gpioSetLevel(1,17,0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KPF", strlen("KPF")))
    {
        PhotoFramenum=0;

     //   uiphotoframe(cmd+strlen("KPF"));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KPS", strlen("KPS")))
    {
        PhotoFramenum=2;
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KGAMMAY ", strlen("KGAMMAY ")))
    {
        uiPanelGammaSetting_Y(cmd+strlen("KGAMMAY "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
	else if(!strncmp((char*)cmd,"KGAMMAX ", strlen("KGAMMAX ")))
    {
        uiPanelGammaSetting_X(cmd+strlen("KGAMMAX "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
#if IS_COMMAX_DOORPHONE
	else if (!strcmp((char*)cmd,"KGAMON"))
    {
        //IduGammaX0 |= 0x00000001;
        IduGammaY0 = 0x804B2A14;
        IduGammaY1 = 0xffEBD6B7;
        IduGammaX1 = 0x1C181410;
        IduGammaX0 = 0x0C080401;
		UI_gamma = 1;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KGAMOFF"))
    {
        //IduGammaX0 &= ~0x00000001;
        IduGammaY0 = 0;
        IduGammaY1 = 0;
        IduGammaX1 = 0;
        IduGammaX0 = 0;
		UI_gamma = 0;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
#if (Sensor_OPTION == Sensor_HM1375_YUV601 )
    else if(!strncmp((char*)cmd,"KIIC1375 ", strlen("KIIC1375 ")))
    {
        uiHM1375_TestI2C(cmd+strlen("KIIC1375 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
#if (Sensor_OPTION == Sensor_NT99141_YUV601 )
    else if(!strncmp((char*)cmd,"KIIC99141 ", strlen("KIIC99141 ")))
    {
        uiNT99141_TestI2C(cmd+strlen("KIIC99141 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
#if(GPIO_I2C_ENA == 1)
    else if(!strncmp((char*)cmd,"KIIC8556 ", strlen("KIIC8556 ")))
    {
        uiCS8556_TestI2C(cmd+strlen("KIIC8556 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
#if(Melody_SNC7232_ENA)
    else if(!strncmp((char*)cmd,"KSNC7232 ", strlen("KSNC7232 ")))
    {
        uiKSNC7232_TestI2C(cmd+strlen("KSNC7232 "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
    else if (!strcmp((char*)cmd,"KATETVON"))
    {
        tvTV_CONF |= 0x00001000;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KATETVOFF"))
    {
        tvTV_CONF &=(~0x00001000);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KATECIU1ON"))
    {
        CIU_1_CTL2 |= 0x00000100;
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KATECIU1OFF"))
    {
        CIU_1_CTL2 &= (~0x00000100);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KBRI ", strlen("KBRI ")))
    {
        uiBrightnessTest(cmd+strlen("KBRI "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KMICG ", strlen("KMICG ")))
    {
        sscanf((char*)cmd, "KMICG %d", &value);
        adcSetADC_MICIN_PGA_Gain(value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KADC ", strlen("KADC ")))
    {
        sscanf((char*)cmd, "KADC %d", &value);
        uiTestAdc(value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KIP ", strlen("KIP ")))
    {
        uiIPNetworkTest(cmd+strlen("KIP "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KFW"))
    {
        uiMenuAction(UI_MENU_SETIDX_UPGRADE_FW);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KMAC ", strlen("KMAC ")))
    {
        uiSetMAC_To_Flash(cmd+strlen("KMAC "));
    }
    else if (!strncmp((char*)cmd,"KUID ", strlen("KUID ")))
    {
        uiSetUID_To_Flash(cmd+strlen("KUID "));
    }
    else if (!strcmp((char*)cmd,"KPRVOSD"))
    {
        uiOsdDrawAllPreviewIcon();
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#if (HW_BOARD_OPTION == MR8200_RX_COMMAX)
    else if (!strncmp((char*)cmd,"KSCH ", strlen("KSCH ")))
    {
        uiCmdSetScheduleTime(cmd+strlen("KSCH "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
    else if (!strcmp((char*)cmd,"K720P"))
    {
    #if RFIU_SUPPORT
        sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_1280X720);
        MyHandler.WhichKey = UI_KEY_NONE;
    #else
        uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_1280X720);
        sysCiu_1_PreviewReset(0);
        MyHandler.WhichKey = UI_KEY_NONE;
    #endif
    }
    else if (!strcmp((char*)cmd,"KVGA"))
    {
    #if RFIU_SUPPORT
        sysbackSetEvt(SYS_BACK_RFI_TX_CHANGE_RESO, UI_MENU_VIDEO_SIZE_640x480);
        MyHandler.WhichKey = UI_KEY_NONE;
    #else
        uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_640x480);
        sysCiu_1_PreviewReset(0);
        MyHandler.WhichKey = UI_KEY_NONE;
    #endif
    }
    else if (!strcmp((char*)cmd,"KENC"))
    {
        TestEncrypt();
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#if MOTOR_EN
    #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    //NULL
    #else
    else if(!strncmp((char*)cmd,"KMOTORH ", strlen("KMOTORH ")))
    {
        sscanf((char*)cmd, "KMOTORH %d", &value);
        MotorStatusH        = value;
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KMOTORV ", strlen("KMOTORV ")))
    {
        sscanf((char*)cmd, "KMOTORV %d", &value);
        MotorStatusV        = value;
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
    #endif
#endif
#if SUPPORT_TOUCH
    else if(!strncmp((char*)cmd,"KTOUCH ", strlen("KTOUCH ")))
    {
        sscanf((char*)cmd, "KTOUCH %d", &value);
        DEBUG_UI("*** Set touch level form %d to %d\n",uiTouchLevel,value);
        uiTouchLevel        = value;
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
#endif
#if ICOMMWIFI_SUPPORT
    else if (!strncmp((char*)cmd,"KAP", strlen("KAP")))
    {
        //homeRFSendToSensor(HOMERF_SEND_GET_HOST,0);
        DEBUG_RED("SET AP REBOOT MODE\n");
        iconflag[UI_MENU_SETIDX_NIGHT_LIGHT] = UI_MENU_SETIDX_LIGHT_ON;
        iconflag[UI_MENU_SETIDX_AP_SCONFIG_DETECT] = UI_BOOT_IN_AP_MODE;
        Save_UI_Setting();
        OSTimeDly(5);
        DEBUG_RED("AP REBOOT\n");
        sysForceWDTtoReboot();
        MyHandler.WhichKey = UI_KEY_NONE;
        
    }
#endif
#if(HOME_RF_SUPPORT)
    else if (!strncmp((char*)cmd,"KHA ", strlen("KHA ")))
    {
        //homeRFSendToSensor(HOMERF_SEND_GET_HOST,0);
        u8 cmdID, sensor;
        sscanf((char*)cmd, "KHA %d %d", &cmdID, &sensor);
        DEBUG_UI("*** cmdID %d , sensor %d \n",cmdID, sensor);
        homeRFSendToSensor(cmdID,sensor);
        MyHandler.WhichKey = UI_KEY_NONE;
        
    }
    else if (!strncmp((char*)cmd,"KHACLR", strlen("KHACLR")))
    {
        
        memset(gHomeRFSensorList->sSensor, 0, sizeof(gHomeRFSensorList->sSensor));
        memset(gHomeRFRoomList->sRoom, 0, sizeof(gHomeRFRoomList->sRoom));
        memset(gHomeRFSceneList->sScene, 0, sizeof(gHomeRFSceneList->sScene));
        gHomeRFSensorCnt=0;
        DefaultSensorCnt=0;
        homeRFSendToSensor(HOMERF_SEND_DELETE_ALL, 0);
        homeRFSendToSensor(HOMERF_SEND_SET_RFID, 0);
		spiWriteHomeRF(SPI_HOMERF_CONFIG);
        spiWriteHomeRF(SPI_HOMERF_SENSOR);
        spiWriteHomeRF(SPI_HOMERF_ROOM);
        spiWriteHomeRF(SPI_HOMERF_SCENE);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KHADEL ", strlen("KHADEL ")))
    {
        u8 sensor;
        
        sscanf((char*)cmd, "KHADEL %d", &sensor);        
        homeRFDeleteSensor(sensor);
        MyHandler.WhichKey = UI_KEY_NONE;
        
    } 
    else if (!strncmp((char*)cmd,"KHALIST ", strlen("KHALIST ")))
    {
    	int i, cnt;
			
		sscanf((char*)cmd, "KHALIST %d", &cnt);

		if(cnt > 128)
			cnt = 128; //Sensor MAX count is 128.

		for(i=0; i<cnt; i++)
		{
			printf("%03d ID:%02x, type:%d, minor:%d\n",\
			i, gHomeRFSensorList->sSensor[i].sID,gHomeRFSensorList->sSensor[i].type, \
			gHomeRFSensorList->sSensor[i].minorVer);
		}
        MyHandler.WhichKey = UI_KEY_NONE;
    } 

#endif
    else if(!strncmp((char*)cmd,"KVOL ", strlen("KVOL ")))
    {
        sscanf((char*)cmd, "KVOL %d", &value);
        adcSetDAC_OutputGain(value);
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
    else if(!strncmp((char*)cmd,"KLTIMER ", strlen("KLTIMER ")))
    {
        int value1, value2, value3, value4;
        sscanf((char*)cmd, "KLTIMER %d:%d~%d:%d %d", &value1, &value2, &value3, &value4, &value);
        uiLightTimer[0] = value1;
        uiLightTimer[1] = value2;
        uiLightTimer[2] = value3;
        uiLightTimer[3] = value4;
        iconflag[UI_MENU_SETIDX_LIGHT_TIMER] = value;
        uiMenuAction(UI_MENU_SETIDX_LIGHT_TIMER);
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
#endif
#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
    else if(!strncmp((char*)cmd,"KRMCU ", strlen("KRMCU ")))
    {
        uiCmdPareRDI_MCU(cmd+strlen("KRMCU "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
#if (HW_BOARD_OPTION == MR8100_GCT_LCD)
	else if(!strncmp((char*)cmd,"KK ", strlen("KK ")))
		{
			int cam;
	        sscanf((char*)cmd, "KK %d", &cam);
			if (MyHandler.MenuMode == VIDEO_MODE)
	        	MyHandler.WhichKey = UI_KEY_RF_QUAD;
			else
			{
				switch(cam)
				{
					case 0 :
						MyHandler.WhichKey = UI_KEY_RIGHT;
						break;
					case 1:
						MyHandler.WhichKey = UI_KEY_CH1;
						break;
					case 2:
						MyHandler.WhichKey = UI_KEY_CH2;
						break;
					case 3:
						MyHandler.WhichKey = UI_KEY_CH3;
						break;
					case 4:
						MyHandler.WhichKey = UI_KEY_CH4;
						break;
				}
						
			}
		}	
#endif  
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    else if(!strncmp((char*)cmd,"KMUSIC ", strlen("KMUSIC ")))
    {
        sscanf((char*)cmd, "KMUSIC %d", &value);
        gpioMusicCtr(value);
        MyHandler.WhichKey  = UI_KEY_NONE;
    }
#endif
#if ICOMMWIFI_SUPPORT
    else if(!strncmp((char*)cmd,"KWIFICHG", strlen("KWIFICHG")))
    {
		extern u8	uart_cmd_change_flag;
		
		uart_cmd_change_flag = 1;
		printf("\x1B[96m uart_cmd_change_flag = %d \x1B[0m\n",uart_cmd_change_flag);
    }
    else if(!strncmp((char*)cmd,"KSEAN", strlen("KSEAN")))
    {
		printf("\x1B[96mResetALL\x1B[0m\n");
        ResetALL();
    }
#endif
#if ((HW_BOARD_OPTION == MR8100_GCT_LCD)&&(UI_PROJ_OPT == 1))
    else if (!strncmp((char*)cmd,"TEMP ", strlen("TEMP ")))
    {
        u32 cam;
        sscanf((char*)cmd, "TEMP %d %d", &cam, &value);
        uiSensorTest = 1;   
        uiTestTemp[cam] = value;
        DEBUG_UI("Set Cam%d: Tempture:%d \n", cam, value);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
    else
        MyHandler.WhichKey = UI_KEY_NONE;

}

