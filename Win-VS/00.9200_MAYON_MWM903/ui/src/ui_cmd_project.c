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
#include "GlobalVariable.h"
#include "MotionDetect_API.h"
#include "spiapi.h"
#include "adcapi.h"
#include "timerapi.h"
#include "p2pserver_api.h"
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
#include "gfuapi.h"
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
u8 uiLightTest = 0;
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio1IntCfg[GPIO_PIN_COUNT];
#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
extern u32 spiNetwrokStartAddr;
extern bool gfileLog;
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
extern void Timer_IR_TX(void);
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
    u8 i, win_num;
    u16 icon_w,icon_h;
    u32 data, val, cur_buf;

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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strcmp((char*)cmd,"L1W1"))
        {
            DEBUG_UI("Enable OSD L1W1\r\n");
            win_num = IDU_OSD_L1_WINDOW_1;
        }
        else if (!strcmp((char*)cmd,"L1W2"))
        {
            DEBUG_UI("Enable OSD L1W2\r\n");
            win_num = IDU_OSD_L1_WINDOW_2;
        }
#endif
        else if (!strcmp((char*)cmd,"L2"))
        {
            DEBUG_UI("Enable OSD L2\r\n");
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &val);
            win_num = (u8)val;
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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strcmp((char*)cmd,"L1W1"))
        {
            DEBUG_UI("Disable OSD L1W1\r\n");
            win_num = IDU_OSD_L1_WINDOW_1;
        }
        else if (!strcmp((char*)cmd,"L1W2"))
        {
            DEBUG_UI("Disable OSD L1W2\r\n");
            win_num = IDU_OSD_L1_WINDOW_2;
        }
#endif
        else if (!strcmp((char*)cmd,"L2"))
        {
            DEBUG_UI("Disable OSD L2\r\n");
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &val);
            win_num = (u8)val;
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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strncmp((char*)cmd,"L1W1 ", strlen("L1W1 ")))
        {
            sscanf((char*)cmd, "L1W1 <%d,%d>,<%d,%d>", &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set L1W1 Form (%d,%d) to (%d,%d)\r\n",x_s, y_s, x_e, y_e);
            win_num = IDU_OSD_L1_WINDOW_1;
        }
        else if (!strncmp((char*)cmd,"L1W2 ", strlen("L1W2 ")))
        {
            sscanf((char*)cmd, "L1W2 <%d,%d>,<%d,%d>", &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set L1W2 Form (%d,%d) to (%d,%d)\r\n",x_s, y_s, x_e, y_e);
            win_num = IDU_OSD_L1_WINDOW_2;
        }
#endif
        else if (!strncmp((char*)cmd, "L2 ", strlen("L2 ")))
        {
            sscanf((char*)cmd, "L2 <%d,%d>,<%d,%d>", &x_s, &y_s, &x_e, &y_e);
            DEBUG_UI("Set L2 Form (%d,%d) to (%d,%d)\r\n",x_s, y_s, x_e, y_e);
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d <%d,%d>,<%d,%d>", &val, &x_s, &y_s, &x_e, &y_e);
            win_num = (u8)val;
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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strcmp((char*)cmd,"L1W1"))
        {
            win_num = IDU_OSD_L1_WINDOW_1;
        }
        else if (!strcmp((char*)cmd,"L1W2"))
        {
            win_num = IDU_OSD_L1_WINDOW_2;
        }
#endif
        else if (!strcmp((char*)cmd,"L2"))
        {
            win_num = IDU_OSD_L2_WINDOW_0;
        }
        else
        {
            sscanf((char*)cmd, "W%d", &val);
            win_num = (u8)val;
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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strncmp((char*)cmd, "L1W1 ", strlen("L1W1 ")))
        {
            cur_buf = IDU_OSD_L1_WINDOW_1;
            cmd+=strlen("L1W1 ");
        }
        else if (!strncmp((char*)cmd, "L1W2 ", strlen("L1W2 ")))
        {
            cur_buf = IDU_OSD_L1_WINDOW_2;
            cmd+=strlen("L1W2 ");
        }
#endif
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
        sscanf((char*)cmd, "<%d*%d> <%d,%d> <%x>", &x_e, &y_e, &x_s, &y_s, &data);
        icon_w = (u16)x_e;
        icon_h = (u16)y_e;
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
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        else if (!strcmp((char*)cmd,"L1W1"))
        {
            DEBUG_UI("Clear OSD L1W1\r\n");
            cur_buf = IDU_OSD_L1_WINDOW_1;
        }
        else if (!strcmp((char*)cmd,"L1W2"))
        {
            DEBUG_UI("Clear OSD L1W2\r\n");
            cur_buf = IDU_OSD_L1_WINDOW_2;
        }
#endif
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
        uiClearOSDBuf((u8)cur_buf);
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
#if ( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
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
#endif
    else if(!strcmp((char*)cmd,"EN TIME"))
        showTime = 1;
    else if(!strcmp((char*)cmd,"DIS TIME"))
        showTime = 0;
}

#if ( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
void uiIRTest(u8 *cmd)
{
    u32 CUSTOM_CODE_ID;
    u32 RecCustom;
    u32 div;

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
        sscanf((char*)cmd, "%d",&div);
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
#endif

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
    #if ((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B))
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
            DEBUG_UI("Level Triger High Error Group %d %d\n", group, i);
        }
    }
    else if(!strcmp((char*)cmd,"LVTRG L"))
    {
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
            DEBUG_UI("Level Triger Low Error Group %d %d\n", group, i);
        }
    }
    else if(!strcmp((char*)cmd,"LVTRG OFF"))
    {
    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
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
            DEBUG_UI("Level Triger OFF Error Group %d %d\n", group, i);
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
               if(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit] == UI_MENU_SETTING_RESOLUTION_HD )
                  return 1;
                
               iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit]=UI_MENU_SETTING_RESOLUTION_HD;
            }
            else if( (reso_W == 640) && (reso_H == 352))
            {
               if(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit] == UI_MENU_SETTING_RESOLUTION_QHD)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit]=UI_MENU_SETTING_RESOLUTION_QHD;
            }
            else if( (reso_W == 1920) && (reso_H == 1072))
            {
               if(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit] == UI_MENU_SETTING_RESOLUTION_1920x1088)
                 return 1;
               
               iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+RFUnit]=UI_MENU_SETTING_RESOLUTION_1920x1088;
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
        return 1;
	}
  #endif
#endif

void uiDESTest(u8 *cmd)
{
    u32  suraddr;
    u32  outaddr;
    u8   iv[8];
    u8   key1[8],key2[8],key3[8];
    u32   i,j;
    u32   temp;


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


void ui_contrast_test(u8 *cmd)
{
    u32 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        contrast_write((u8)data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        contrast_read((u8*)&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_brightness_test(u8 *cmd)
{
    u32 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        brightness_write((u8)data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        brightness_read((u8*)&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_saturation_test(u8 *cmd)
{
    u32 data =0;

    if(!strncmp((char*)cmd,"W ", strlen("W ")))
    {
        cmd+=strlen("W ");
        sscanf((char *)cmd,"%x",&data);
        saturation_write((u8)data);
        DEBUG_UI("data:%x  \n",data);
    }else if(!strncmp((char*)cmd,"R ", strlen("R ")))
    {
        cmd+=strlen("R ");
    //    sscanf((char *)cmd,"%x",&data);
        saturation_read((u8*)&data);
        DEBUG_UI("data:%x  \n",data);
    }

}
void ui_change_test(u8 *cmd)
{
    u32 data = 0;
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
    CH_Channel_write(name,(u8)data);
    DEBUG_UI("data:%x  \n",data);

}

void uiTestAdc(u8 value)
{
    u32 adc_reg=0;
    s32 data=0;

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

    sscanf((char*)cmd, "%d",&Value);
    DEBUG_UI("Brightness Value : %d \r\n",Value);
    //uiMenuSet_TX_BRIGHTNESS(Value);
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

void uiPanelGammaSetting_Y(u8 *cmd)
{
    u32 level;
    u32 data;
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
    u32 level;
    u32 data;
    u32 gammaData;
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
    u32 mac1,mac2,mac3,mac4,mac5,mac6;
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

    if(!strncmp((const char *)uiMACAddr,(const char *)readdata,strlen((const char *)uiMACAddr)) )
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

    sscanf((char*)cmd, "%s", uiP2PID);
    spiWriteNet();

    spiReadData(SPIConfigBuf, spiNetwrokStartAddr, SPI_UI_NETWORK_INFO_SIZE);
    memcpy(&readdata,SPIConfigBuf,sizeof(readdata));
    if(!strncmp((const char *)uiP2PID,(const char *)readdata,strlen((const char *)uiP2PID)) )
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
u32 H264_Special_CMD=0x121;

u8 uiCmdPareCmd(u8* cmd)
{
    int value;

    DEBUG_UI("uiParseUartCmd Get Cmd %s\n", cmd);
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
		    		printf("\n");
	    		}	
			}
		}
    }
    else if(!strcmp((char*)cmd,"KM"))
    {
        u8 i,j;
        
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if (VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA)
                j=1;
            else
                j=0;
                
            DEBUG_YELLOW("Cam %d uiCurRecStatus %d motion %d record %d\r\n", i, uiCurRecStatus[i],j,MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX));
        }
       
        MyHandler.WhichKey   = UI_KEY_NONE;
    }	
    else if(!strncmp((char*)cmd, "KE_", strlen("KE_")))
    {
        cmd+=strlen("KE_");
        sscanf((char*)cmd, "%08x", &H264_Special_CMD);
        printf("H264_Special_CMD = 0x%08x\n",H264_Special_CMD);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd, "KEY", strlen("KEY")))
    {
        u8 i;
        {
            sscanf((char*)cmd, "KEY %d %d", &i, &value);
            #if(SUPPORT_TOUCH)
            TouchExtKey = value;
            #endif
            MyHandler.WhichKey   =  i;
        }
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

#if RFIU_SUPPORT
    else if (!strcmp((char*)cmd,"KRFZOOMON"))
    {
        DEBUG_UI("\n-->Do RF-%d Channel Zoom On\n",sysRFRxInMainCHsel);
        uiSetZoomMode(0,240,134,1);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFZOOMOFF"))
    {
        DEBUG_UI("\n-->Do RF-%d Channel Zoom Off\n",sysRFRxInMainCHsel);
        uiSetZoomMode(0,0,0,0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
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
        MyHandler.WhichKey   = UI_KEY_RF_QUAD;
    }
    else if (!strcmp((char*)cmd,"KRFMASK"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_ENTER_MASKAREA);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
	else if (!strcmp((char*)cmd,"KRFDUAL"))
    {
        uiSetRfDisplayMode(UI_MENU_RF_DUAL);
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
	else if (!strcmp((char*)cmd,"KRFFULL"))
    {
        if (MyHandler.MenuMode == QUAD_MODE)
        {
            MyHandler.WhichKey = UI_KEY_RF_QUAD;
        }
        else
        {
            uiSetRfDisplayMode(UI_MENU_RF_FULL);
    	    MyHandler.WhichKey = UI_KEY_NONE;
        }
    }
    else if (!strncmp((char*)cmd,"KRFCAMON ", strlen("KRFCAMON ")))
    {
        rfiuCamOnOffCmd(cmd+strlen("KRFCAMON "));
	    MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strcmp((char*)cmd,"KRFFCC_MODE"))
    {
    #if RFIU_TEST
        sysback_RF_SetEvt(SYS_BACKRF_FCC_DIRECT_TXRX, 0);
        MyHandler.WhichKey   = UI_KEY_NONE;
    #endif
    }
    else if (!strncmp((char*)cmd,"KRFWOR_MODE ",strlen("KRFWOR_MODE ")))
    {
        sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);        
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
#if RFIU_TEST
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
        rfiuFCCRX1Cmd(cmd+strlen("KRFFCC_RX1 "));
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
#endif
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
        uiRFPairTest(cmd+strlen("KRFPAR1"),0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }

    else if(!strncmp((char*)cmd,"KRFPAR2", strlen("KRFPAR2")))
    {
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
        uiRXCMDTest(cmd+strlen("KRFCMD1 "),0);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFCMD2 ", strlen("KRFCMD2 ")))
    {
        uiRXCMDTest(cmd+strlen("KRFCMD2 "),1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
    else if (!strncmp((char*)cmd,"KRFINFO ", strlen("KRFINFO ")))
    {
        uiTXCMDTest(cmd+strlen("KRFINFO "),0);
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

 #if 1//RFIU_TX_WAKEUP_SCHEME
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
  #if TX_FW_UPDATE_SUPPORT
    else if(!strncmp((char*)cmd,"KRFFWUPD1_SD", strlen("KRFFWUPD1_SD")))
    {
        DEBUG_UI("RX FW Update test from SD\r\n");
        rfiuTxFwUpdateFromSD(0);
        
        MyHandler.WhichKey   = UI_KEY_NONE;
    }
    else if(!strncmp((char*)cmd,"KRFFWUPD1_NET", strlen("KRFFWUPD1_NET")))
    {
        DEBUG_UI("RX FW Update test from NET\r\n");
        // SD updating: \\pa9txfw.bin
        rfiuTxFwUpdateFromNet(0);
            
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
#if ( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
    else if(!strncmp((char*)cmd,"KIR ", strlen("KIR ")))
    {
        uiIRTest(cmd+strlen("KIR "));
        MyHandler.WhichKey = UI_KEY_NONE;
    }
#endif
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
    else if(!strncmp((char*)cmd,"KIIC1B ", strlen("KIIC1B ")))
    {
        u32 slaveAddr    = 0;
        u32 registAddr   = 0;
        u32 data         = 0;
        cmd    += strlen("KIIC1B ");
        if(!strncmp((char*)cmd,"R ", strlen("R ")))
        {
            cmd    += strlen("R ");
            sscanf((char *)cmd, "%x %x", &slaveAddr, &registAddr);
            i2cRead_Byte((u8)slaveAddr, (u8)registAddr, (u8*)&data);

            DEBUG_UI("SLAVE:%x, regist:%x, data:%x\n", slaveAddr, registAddr, data);
        }
        else if(!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            cmd    += strlen("W ");
            sscanf((char *)cmd,"%x %x %x", &slaveAddr, &registAddr, &data);

            i2cWrite_Byte((u8)slaveAddr, (u8)registAddr, (u8)data);
            DEBUG_UI("SLAVE:%x, regist:%x, data:%x\n",slaveAddr,registAddr,data);
        }
        MyHandler.WhichKey = UI_KEY_NONE;
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
    else if(!strncmp((char*)cmd,"KI2CLCDOFF", strlen("KI2CLCDOFF")))
    {
        gpioSetLevel(1,17,1);
        MyHandler.WhichKey = UI_KEY_NONE;
    }
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
#if (AUDIO_DEVICE== AUDIO_IIS_WM8940)
    else if(!strncmp((char*)cmd,"K8940 ", strlen("K8940 ")))
    {
        u8 addr;
        u16 data;

        cmd+=strlen("K8940 ");
        sscanf((char*)cmd, "%02x %02x", &addr, &data);
        //printf("addr=0x%02x, data=0x%02x\n", addr, data);
        i2cWrite_WM8940(addr, data);
        MyHandler.WhichKey = UI_KEY_NONE;
	}
#endif
    else if (!strncmp((char*)cmd,"KLIGHT", strlen("KLIGHT")))
    {
        uiLightTest ^= 1;
		printf("@@uiLightTest = %d \n",uiLightTest);
        MyHandler.WhichKey = UI_KEY_NONE;        
    } 
    else
        MyHandler.WhichKey = UI_KEY_NONE;
    return 1;
}

