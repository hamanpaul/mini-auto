/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    idu.c

Abstract:

    The routines of Image Display Unit.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create
    2007/05/15      Lucian Yuan     modify

*/

#include "general.h"
#include "board.h"
#include "iduapi.h"
#include "idureg.h"
#include "gpioapi.h"
#include "../board/inc/intreg.h"
#include "Timerapi.h"
#include "sysapi.h"
#include "iisapi.h"
#include "rtcapi.h"
#include "uiapi.h"
#include "ciuapi.h"
#if(CHIP_OPTION >= CHIP_A1013A)
#include "rfiuapi.h"
#endif

#if (HW_BOARD_OPTION==MR6730_AFN) 
#include "..\..\ui\inc\MainFlow.h"

#endif


/*********************
*	Constant
*********************/
#if (LCM_OPTION == LCM_TG200Q04)
    #define PANEL_SELECT                     2  //0:TG200Q04  1:TG200Q23  2:TG200R00
#endif

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
    (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
   #define IDUBRI_HD_DISPLAY_SIZE  1024    //Lucian: 正常為1280,但FPGA 效能有限,下調為1024
#else
   #define IDUBRI_HD_DISPLAY_SIZE  1280
#endif

#if (LCM_OPTION == LCM_HX8817_RGB)
#define GROUP_LCD_SCLK	3
#define GROUP_LCD_SDA 	3
#define GROUP_LCD_CS  	3
#define PIN_LCD_SCLK	6
#define PIN_LCD_SDA 	7
#define PIN_LCD_CS  	8
#define LEVEL_HIGH		1
#define LEVEL_LOW		0
#endif

#if ((LCM_OPTION == LCM_TMT035DNAFWU24_320x240)||(LCM_OPTION == LCM_LQ035NC111))
  #define GROUP_LCD_SCLK	 3
  #define GROUP_LCD_SDA 	 3
  #define GROUP_LCD_CS  	 3
  #define GROUP_LCD_nRESET   3
  #define LEVEL_HIGH       1
  #define LEVEL_LOW        0
#endif

#if (LCM_OPTION == LCM_TD036THEA3_320x240)
#define GROUP_LCD_SCLK	3
#define GROUP_LCD_SDA 	3
#define GROUP_LCD_CS  	3
#define PIN_LCD_SDA 	7
#define PIN_LCD_SCLK	6
#define PIN_LCD_CS  	5
#define LEVEL_HIGH		1
#define LEVEL_LOW		0
#endif

#if ((LCM_OPTION == LCM_TD024THEB2)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
#define GROUP_LCD_SCLK	3
#define GROUP_LCD_SDA 	3
#define GROUP_LCD_CS  	3
#define PIN_LCD_SCLK	6
#define PIN_LCD_SDA 	7
#define PIN_LCD_CS  	8
#define LEVEL_HIGH		1
#define LEVEL_LOW		0
#endif


#if (LCM_OPTION == LCM_TJ015NC02AA)
#define GROUP_LCD_SCLK	3
#define GROUP_LCD_SDA 	3
#define GROUP_LCD_CS  	3
#define PIN_LCD_SDA 	7
#define PIN_LCD_SCLK	6
#define PIN_LCD_CS  	8
#define LEVEL_HIGH		1
#define LEVEL_LOW		0
#endif





/*********************
*	Variable
*********************/
extern u8 uiMenuVideoSizeSetting;
extern s32 isu_avifrmcnt;
extern u8 siuOpMode;
extern volatile s32 isu_idufrmcnt;   /*Peter 1109 S*/
extern u8 TvOutMode;

#if(HWPIP_SUPPORT)      /*Amon 20140428*/
#define  PIP_SizeX PANNEL_X
#define  PIP_SizeY PANNEL_Y
#if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA)
#define  TVPIP_SizeX 640
#define  TVPIP_SizeY 240
#else
#define  TVPIP_SizeX 640
#define  TVPIP_SizeY 240
#endif
u32 PIP_IN_X= 320;   //OSD size x
u32 PIP_IN_Y= 120;   //OSD size y
u32 PIP_S_X= 320;    //OSD start x
u32 PIP_S_Y= 120;    //OSD start y
u32 PIP_E_X= 640;    //OSD end x
u32 PIP_E_Y= 240;    //OSD end y

u16 PIPDispWidth[]= {
    PIP_SizeX,
    TVPIP_SizeX,
};
u16 PIPDispHeight[]= {
    PIP_SizeY,
    TVPIP_SizeY,
};

#endif



#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern  s64 Videodisplaytime[DISPLAY_BUF_NUM];
extern s64 VideoNextPresentTime;
extern u32 IsuIndex;
extern s64 Video_timebase ;       //Lsk 090330
extern u8 ResetPlayback;
extern u8 StartPlayBack;
s64 IDUInterruptTime;
s64 IDUVideoTimeOffset;
#define NTSC_IDUInterruptUnit 100100 //NTSC : 30/1.001 frames per second (very close to 29.97) frames per second,
#define PAL_IDUInterruptUnit  120000 //NTSC : 30/1.001 frames per second (very close to 29.97) frames per second,
#define DISPLAY_THRESHOLD   (200200/3)  //Lsk 090325 (2*1001000/30)
#endif
extern u8	*pucClockImageBuf;
extern u32 mpeg4Width, mpeg4Height;

#if (HW_BOARD_OPTION == SIYUAN_CVR)
extern RTC_DATE_TIME	sDateTime;
extern u8 ucAdjPos;
extern u8 ucSetLCDPara;		/* the parameter of LCD flash controlling */
extern u8 ucUISetting;
#endif

#if (HW_BOARD_OPTION == NEW_SIYUAN)
extern RTC_DATE_TIME	sDateTime;
extern u8 ucAdjPos;
extern u8 ucSetLCDPara;		/* the parameter of LCD flash controlling */
extern u8 ucUISetting;
extern u8 ucAdjPos_sub;
#endif

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
extern u32 ciu_idufrmcnt_ch1;
extern u32 ciu_idufrmcnt_ch2;
extern u32 ciu_idufrmcnt_ch3;
extern u32 ciu_idufrmcnt_ch4;

#endif
#if IS_COMMAX_DOORPHONE
extern u8 UI_gamma;
#endif

#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
   #if MENU_DONOT_SHARE_BUFFER
    extern u8 sysEnMenu;
    extern u8 sysEnZoom;
    extern u8 sysEnSnapshot;
   #endif
    extern u8 uiRFCHChg;
#endif


u8 gTVColorbarEn = 0;
IDU_OSD_BLK_INFO OsdBlkInfo[IDU_OSD_MAX_NUM];
/*********************
*	Function Prototype
*********************/
void iduRst(void);
s32 iduPreview(int src_W,int src_H);
s32 LCM_IDUInit(void);
void IDU_Init(u8, u8);
void iduWaitCmdBusy(void);
void iduDelay(u32);
void idu_switch(void);
void iduTypeConfig(void);
void iduOSDBRI(u32 ADDR, u32 SREIDE,u32 SIZE);
void iduOSDDisplay1(u8 blk_idx, u32 OSD_SX, u32 OSD_SY, u32 OSD_EX, u32 OSD_EY);
void iduTVOSDDisplay(u8 blk_idx, u32 TVOSD_SX, u32 TVOSD_SY, u32 TVOSD_EX, u32 TVOSD_EY);
void iduOSDClear(void);
u32  iduOSDGetYStartEnd(u8 buf_idx, u32 *OSD_SY, u32 *OSD_EY);
void iduTVOSDBRI(u32 ADDR, u32 SREIDE,u32 SIZE);
void iduOSDBRIClear(void);
void iduTVOSDClear(void);
void idu5073Config(void);
void idu5073Config_YUV(void);
void idu5073Gamma(void);
void LCDBright(u8 pattern);
void IduVideo_ClearBuf(void);
void IduVideo_ClearPartBuf(u32 start, u32 end);
void IduVideo_ClearPKBuf(u8 bufinx);
void IduVideo_ClearPartPKBuf(u32 start, u32 end, u8 bufinx);
u32 GetTVFrameBufAdr(u8 buf_idx);
void iduOSDDisable(u8 osd);
void iduOSDEnable(u8 osd);
void iduTVOSDDisable(u8 tvosd);
void iduTVOSDEnable(u8 tvosd);
extern void siuStop(void);
extern void ipuStop(void);
extern void isuStop(void);
extern void *memset_hw_Word(void *ori_dest, unsigned int dataVal, int ori_count);

void iduHx8817Write(u8, u8);
void iduSetLcdBrightness(s8);


u8  IsIduOsdEnable  = 1;    // 1: Enable, 0: Disable

#if (LCM_OPTION == LCM_TD036THEA3_320x240)
void iduTD036THEA3Write(u8 ucRegAddr, u8 ucData);
void iduTD036THEA3Read(u8 ucRegAddr, u8 *pucData);
#endif
#if ((LCM_OPTION == LCM_TD024THEB2)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
void iduTD024THEB2Write(u8 ucRegAddr, u8 ucData);
void iduTD024THEB2Read(u8 ucRegAddr, u8 *pucData);
#endif

#if (LCM_OPTION == LCM_TJ015NC02AA)
void iduTJ015NC02AAWrite(u8 ucRegAddr, u8 ucData);
void iduTJ015NC02AARead(u8 ucRegAddr, u8 *pucData);
#endif

#if (LCM_OPTION == LCM_TMT035DNAFWU24_320x240)
void iduTMT035Write(u8, u8);
#endif

#if ((HW_BOARD_OPTION == SIYUAN_CVR)||(HW_BOARD_OPTION == NEW_SIYUAN))
void iduDispLCDString( u8 *, u8, u8);
void iduDisplayDigitNum(u8 *, u8, u8);
void iduDisplayDefaultSymbol(void);
void iduControlSymbolVideo(u8);
void iduControlVideoSize(u8);
void iduControlFrameRate(u8);
void iduControlSymbolSD(u8);
void iduControlSymbolCol(u8);
void iduControlSymbolPower(u8);
void iduControlSymbolBattery(u8);
void iduInitHT1621LCD(void);
void iduCloseHT1621LCD(void);
void Parallel_RGB_888(void);
#endif
#if(HWPIP_SUPPORT)
int HWPIP_CH_Sel(u8 addr);
int HWPIP_Position(u32 S_X,u32 S_Y);
int HWPIP_OSDSize(u32 IN_X,u32 IN_Y);
int HWPIP_AlphaBlendingSet(u8 Blending);
int HWPIP_DownSampleSet(u8 Downsample);
void HWPIP_EdgeColorSet(u32 color);
void HWPIP_EdgeColorEn(u8 Edge_En);
int HWPIP_BurstBitwidthSet(u8 B8_OSD,u8 B8_VDO,u8 BW_64);
void HWPIP_BRI_En(u8 BRI_En);
void HWPIP_Open(u8 BRI_En,u8 Downsample,u8 Blending,u8 Edge_En,u8 B8_OSD,u8 B8_VDO,u8 W64_En);
void uiHWPIP_TEST1(u8 *cmd);
void uiHWPIP_TEST2(void);
void uiHWPIP_TEST3(u8 *cmd);

#endif

/*********************
*	Function Body
*********************/
u32 GetTVFrameBufAdr(u8 buf_idx)
{
  return((u32)((Jpeg_displaybuf[buf_idx])));
}

#if(LCM_OPTION == LCM_TM024HDH29)
void MPU_Update_Frame()
{
    u32 i ;

    DEBUG_IDU("MPU_Update_Frame\n");
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	IduMpuCmd = 0x0000002c;
	iduWaitCmdBusy();
    iduDelay(200);
	IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    for(i=0 ; i<76800 ; i++)
    {
        IduMpuCmd = 0x000000ff;
        iduWaitCmdBusy();
        iduDelay(20);
    }
    gpioSetLevel(3, 4, 1);
}

void MPU_Read_Register(u8 addr, u8 num)
{
    int i ;
    u32 tmp ;

    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8 ;
    IduMpuCmd = addr;
    iduWaitCmdBusy();
    iduDelay(3000);
    //IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8 | IDU_NWR_READ;
    IduMpuCmdCfg = 0x00008061 ;
    IduMpuCmdCfg = 0x00008063 ;
    iduWaitCmdBusy();
    iduDelay(3000);
    tmp = IduMpuRead ;
    //IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8 | IDU_NWR_READ;
    for(i=0 ; i<num ; i++)
    {
        IduMpuCmdCfg = 0x00008063 ;
        iduWaitCmdBusy();
        iduDelay(3000);
        //DEBUG_IDU("Read addr %x data %x \n", addr, IduMpuRead);
    }
    gpioSetLevel(3, 4, 1);
}
#endif

//------------------Set IDU clock------------------//
#if (LCM_OPTION == LCM_HX5073_RGB)
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif


    SYS_CLK1 = SYS_CLK1 | 0x00000400;
    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000804;
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0, 1);

    idu5073Config();
    idu5073Config();
    idu5073Gamma();

    return 1;
}
#elif (LCM_OPTION == LCM_TMT035DNAFWU24_320x240)
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif


    /* Reset LCD Module */
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_HIGH);
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_LOW);
    for (i=0; i<500; i++);
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_HIGH);
    for (i=0; i<2000; i++);	/* delay for LCD Module ready */

    IDU_Init(0, 1);

    /* set panel to support S-RGB */
    #if ((HW_BOARD_OPTION == SKYBEST_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==VEISE_CARDVR))
        iduTMT035Write(0x03, 0xC8);
    #elif(HW_BOARD_OPTION == WENSHING_SDV)
        iduTMT035Write(0x03, 0xC9);
    #endif

    /* adjust back-porch */
    iduTMT035Write(0x04, 0x54);
    /* Init Idu Controller */

    //Modify VCCOM AC
    iduTMT035Write(0x0E, 0x6D);
    iduTMT035Write(0x0F, 0xA4);
    iduTMT035Write(0x1E, 0x00);

    return 1;
}
#elif(LCM_OPTION == LCM_LQ035NC111)
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif


    /* Reset LCD Module */
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_HIGH);
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_LOW);
    for (i=0; i<500; i++);
    gpioSetLevel(GROUP_LCD_nRESET, PIN_LCD_nRESET, LEVEL_HIGH);
    for (i=0; i<2000; i++);	/* delay for LCD Module ready */

    IDU_Init(0, 1);
    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    IduMpuCmdCfg = 0x00140029;
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmdCfg = 0x00140028;
    IduMpuCmd = 0x0000B454;
    iduWaitCmdBusy();
    iduDelay(200);
    return 1;
}
#elif(LCM_OPTION == LCM_TJ015NC02AA)
s32 LCM_IDUInit(void)
{
    u32 temp;
    u8 i;

#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

#if(CHIP_OPTION  ==  CHIP_PA9001D)
    SYS_CLK1 = SYS_CLK1 | 0x00000000;//lisa_idu
    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
#else
    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000900; //Lucian: 改變IDU clock : 192/10= 19.2 MHz. framerate=62 fps.
    IduEna  =   IDU_DCLK_EN;
#endif

    gpioSetLevel(3, 4, 1);
    //----LCD reset----//
    gpioSetLevel(3, 5, 1);
    for(i=0;i<20;i++);
    gpioSetLevel(3, 5, 0);
    for(i=0;i<20;i++);
    gpioSetLevel(3, 5, 1);

    Gpio3Dir &= ~0x1C0;
    Gpio3Level |= 0x100;
    Gpio3Ena |= 0x1C0;

    gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    iduTJ015NC02AAWrite(0x02, 0x02);
    iduTJ015NC02AAWrite(0x07, 0x08);
    iduTJ015NC02AAWrite(0x0C, 0x0A);
    iduTJ015NC02AAWrite(0x04, 0x0F);
    IDU_Init(0 , 1);

}

#elif (LCM_OPTION == LCM_HX8224)
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif

    IDU_Init(0, 1);

    IduDispCfg &= ~0x0000000f;
    IduDispCfg |= IDU_INTF_SPI ;
    IduMpuCmdCfg = 0x001e0028;
    IduMpuCmd = 0x00006083;
    iduWaitCmdBusy();

    return 1;
}

#elif(LCM_OPTION == LCM_TD024THEB2)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; //Lucian: 改變IDU clock : 192/10= 19.2 MHz. framerate=62 fps.
      IduEna  =   IDU_DCLK_EN;
    /*** reset LCD - Gpio 1-1 ***/

    Gpio1Ena  |= 0x00000002;
    Gpio1Dir  &= ~0x00000002;
    Gpio1Level |= 0x00000002;
    	Gpio1Level |= 0x00000002;
    	Gpio1Level &= ~0x00000002;
    	Gpio1Level &= ~0x00000002;
    for(temp=0; temp<200; temp++);     //??
    Gpio1Level |= 0x00000002;
        Gpio1Level |= 0x00000002;

    /*** init GROUP_LCD_SCLK, GROUP_LCD_SDA, GROUP_LCD_CS : gpio 3-6,7,8***/
    Gpio3Dir   &= ~0x000001c0;
    Gpio3Level |= 0x00000100; //set LCD_CS high
    Gpio3Ena   |= 0x000001c0;

    IDU_Init(0, 1);
    gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    // set panel to support S-RGB
    iduTD024THEB2Write(0x02, 0x02); //input data format
    iduTD024THEB2Write(0x07, 0x08); //resolution selection
    iduTD024THEB2Write(0x0C, 0x0A); //offset of brightness

    iduTD024THEB2Write(0x04, 0x0f); //CP_CLK/PWM output


    return 1;
}
#elif(LCM_OPTION == LCM_TD024THEB2_SRGB)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; //Lucian: 改變IDU clock : 192/10= 19.2 MHz. framerate=62 fps.
      IduEna  =   IDU_DCLK_EN;
    /*** reset LCD - Gpio 1-1 ***/

    Gpio1Ena  |= 0x00000002;
    Gpio1Dir  &= ~0x00000002;
    Gpio1Level |= 0x00000002;
    	Gpio1Level |= 0x00000002;
    	Gpio1Level &= ~0x00000002;
    	Gpio1Level &= ~0x00000002;
    for(temp=0; temp<200; temp++);     //??
    Gpio1Level |= 0x00000002;
        Gpio1Level |= 0x00000002;

    /*** init GROUP_LCD_SCLK, GROUP_LCD_SDA, GROUP_LCD_CS : gpio 3-6,7,8***/
    Gpio3Dir   &= ~0x000001c0;
    Gpio3Level |= 0x00000100; //set LCD_CS high
    Gpio3Ena   |= 0x000001c0;

    IDU_Init(0, 1);
    gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    // set panel to support S-RGB
    iduTD024THEB2Write(0x02, 0x02); //input data format
    iduTD024THEB2Write(0x07, 0x08); //resolution selection
    iduTD024THEB2Write(0x0C, 0x0A); //offset of brightness

    iduTD024THEB2Write(0x04, 0x0f); //CP_CLK/PWM output


    return 1;
}


#elif (LCM_OPTION == LCM_HX5073_YUV)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CLK1 = SYS_CLK1 | 0x00000300;
    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000804;
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0, 1);
    idu5073Config_YUV();
    idu5073Config_YUV();
    idu5073Gamma();

    return 1;
}

#elif (LCM_OPTION == LCM_TPG105)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CLK1 = SYS_CLK1 | 0x00000300;
    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000804;
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0, 1);
    IduMpuCmdCfg = 0x00140028;
    IduMpuCmd = 0x00000802;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001C08;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000300A;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001003;

    return 1;
}

#elif (LCM_OPTION == LCM_TD020THEG1)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


#if(CHIP_OPTION  ==  CHIP_PA9001D)
      SYS_CLK1 = SYS_CLK1 | 0x00000000;//lisa_idu
      SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
#endif
    IduEna  =   IDU_DCLK_EN;
    IduBypassCtrl=0x00000003;//lisa_idu
    IduMpuCmdCfg = 0x00140028;
    IDU_Init(0 , 1);

    IduMpuCmd = 0x00000802;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001C08;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000300A;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000142B;
    iduWaitCmdBusy();

    IduMpuCmd = 0x00001818; //Horizontal start position
    iduWaitCmdBusy();

    IduMpuCmd = 0x00001C68; //Vertical start position
    iduWaitCmdBusy();

    IduMpuCmd = 0x0000100F; //Open PWM


    return 1;
}
#elif(LCM_OPTION == LCM_TD036THEA3_320x240)
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif


#if(CHIP_OPTION  ==  CHIP_PA9001D)
    SYS_CLK1 = SYS_CLK1 | 0x00000000;//lisa_idu
    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
    IduEna  =   IDU_DCLK_EN;
#else
    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; //Lucian: 改變IDU clock : 192/12= 16 MHz. framerate=52 fps.
    IduEna  =   IDU_DCLK_EN;
#endif



    gpioSetLevel(3, GPIO_LCD_STANDBY, 1);
    //----LCD reset----//
    gpioSetLevel(3, GPIO_LCD_RESET, 1);
    for(i=0;i<20;i++);
    gpioSetLevel(3, GPIO_LCD_RESET, 0);
    for(i=0;i<20;i++);
    gpioSetLevel(3, GPIO_LCD_RESET, 1);

    Gpio3Dir &= ~0xE0;
    Gpio3Level |= 0x20;
    Gpio3Ena |= 0xE0;

    IDU_Init(0 , 1);

    gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    // set panel to support S-RGB
    iduTD036THEA3Write(0x02, 0x02);

    iduTD036THEA3Write(0x03, 0x31);
    iduTD036THEA3Write(0x05, 0x17);

    iduTD036THEA3Write(0x06, 0x17);

    //iduTD036THEA3Write(0x07, 0x08);
    iduTD036THEA3Write(0x0C, 0x0A);

    iduTD036THEA3Write(0x04, 0x0F);


    return 1;
}
#elif (LCM_OPTION == LCM_A015AN04)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0 , 1);
    //Initialize LCM
    IduMpuCmdCfg = 0x00140028;
    IduMpuCmd = 0x00000001  ;
    iduWaitCmdBusy();
    IduMpuCmd =0x00002000  ;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00006000  ;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008006  ;

    return 1;
}
#elif(LCM_OPTION == LCM_A024CN02)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
    IduEna	=	IDU_DCLK_EN;
    IDU_Init(0 , 1);
    //Initialize LCM
    IduMpuCmdCfg = 0x00140028;
    IduMpuCmd = 0x0000C003;
    iduWaitCmdBusy();
    IduMpuCmd =0x00006010;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003028;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000507D;

    return 1;
}
#elif (LCM_OPTION == LCM_GPG48238QS4)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000004;
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0 , 1);
    //Initialize LCM
    IduMpuCmdCfg = 0x00140028;
    IduMpuCmd = 0x0000C003;
    iduWaitCmdBusy();
    IduMpuCmd =0x0000000F;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000F546;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000507D;
    iduWaitCmdBusy();

    return 1;
}
#elif( (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) )
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif



    IDU_Init(0 , 1);

    IduDispCfg &= ~0x0000000f;
    IduDispCfg |= IDU_INTF_SPI ;
    IduMpuCmdCfg = 0x001e0028;
  #if(LCM_OPTION == LCM_HX8224_601)
    IduMpuCmd = 0x00006083;
  #else //656
    IduMpuCmd = 0x00006002;
  #endif

    iduWaitCmdBusy();


    return 1;
}

#elif(LCM_OPTION == LCM_CCIR601_640x480P)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000400; //Lucian: 改變IDU clock : 48MHz ->64MHz.

    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0 , 1);
    IduMpuCmdCfg = 0x001e0028;

    IduMpuCmd = 0x00006083;  //Lucian: Select 24.53MHz, YUV 601 mode
    iduWaitCmdBusy();

    return 1;
}
#elif(LCM_OPTION == LCM_HX8817_RGB)
s32 LCM_IDUInit(void)
{
    u32 temp;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;

    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000600; //Lucian: 改變IDU clock : 192/7= 27.4 MHz. framerate=58 fps.

    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0 , 1);
    /*Set aging pattern, Enable PWM */
    iduHx8817Write(0x06, 0x4F);


    return 1;
}

#elif ( (LCM_OPTION == LCM_P_RGB_888_Innolux) )
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000200; //32MHz /3 = 10.66MHz
#else
  #if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A))
    #if(SYS_CPU_CLK_FREQ == 108000000)
      SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000c00; // 216MHz /13=16.6 MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #elif(SYS_CPU_CLK_FREQ == 96000000)
	  SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000900; // 288MHz /10=28.8MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #endif
  #elif((CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) )
    #if(SYS_CPU_CLK_FREQ == 160000000)
      SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000e00; // 480MHz /15=32 MHz
    #endif
  #endif
#endif
    IDU_Init(0 , 1);
    return 1;
}



#elif ( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000;
#endif
    IDU_Init(0 , 1);
    return 1;
}

#elif ((LCM_OPTION == LCM_P_RGB_888_HannStar)  || (LCM_OPTION == LCM_P_RGB_888_AT070TN90) || (LCM_OPTION == LCM_P_RGB_888_SY700BE104) || (LCM_OPTION == LCM_P_RGB_888_FC070227))
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000;
#else
    #if(SYS_CPU_CLK_FREQ == 108000000)
        #if (HW_BOARD_OPTION == MR9670_COMMAX)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; // 216MHz /8= 27 MHz
        #elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000700; // 216MHz /7= 30 MHz
        #elif (HW_BOARD_OPTION == MR9670_WOAN)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000900; // 216MHz /9= 30 MHz
        #else
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000e00; // 216MHz /15= 14.4 MHz
        #endif
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #elif(SYS_CPU_CLK_FREQ == 96000000)
	  SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; // 288MHz /12=24MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #endif
#endif
    IDU_Init(0 , 1);
    return 1;
}
#elif (LCM_OPTION == LCM_P_RGB_888_ILI6122)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000;
#else
    #if(SYS_CPU_CLK_FREQ == 108000000)
        #if (HW_BOARD_OPTION == MR9670_COMMAX)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; // 216MHz /9= 24 MHz
        #elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000700; // 216MHz /8= 27 MHz
        #elif (HW_BOARD_OPTION == MR9670_WOAN)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000900; // 216MHz /10= 21.6 MHz
        #else
           #if USE_IDUCLK_SLOW
              SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000a00; // 216MHz /11= 19.636 MHz
           #else
              SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; // 216MHz /9= 24 MHz
           #endif        
        #endif
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #elif(SYS_CPU_CLK_FREQ == 96000000)
	  SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; // 288MHz /12=24MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #endif
#endif
    IDU_Init(0 , 1);
    return 1;
}
#elif (LCM_OPTION == LCM_P_RGB_888_ILI6126C)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000;
#else
    #if(SYS_CPU_CLK_FREQ == 108000000)
        #if (HW_BOARD_OPTION == MR9670_COMMAX)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; // 216MHz /9= 24 MHz
        #elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000700; // 216MHz /8= 27 MHz
        #elif (HW_BOARD_OPTION == MR9670_WOAN)
        SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000900; // 216MHz /10= 21.6 MHz
        #else
           #if USE_IDUCLK_SLOW
              SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; // 216MHz /12= 18 MHz
           #else
              SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000800; // 216MHz /9= 24 MHz  //havent finetune
           #endif        
        #endif
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #elif(SYS_CPU_CLK_FREQ == 96000000)
	  SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; // 288MHz /12=24MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #endif
#endif
    IDU_Init(0 , 1);
    return 1;
}

#elif (LCM_OPTION == LCM_P_RGB_888_ZSX900B50BL)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif

    temp= SYS_CLK1;
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000;
#else
    #if(SYS_CPU_CLK_FREQ == 108000000)
      SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000c00; // 216MHz /13= 16.6154 MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #elif(SYS_CPU_CLK_FREQ == 96000000)
	  SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000b00; // 288MHz /12=24MHz
      SYS_CPU_PLLCTL &= (~0x00000080);  // LCD clock from sysclk.
    #endif
#endif
    IDU_Init(0 , 1);
    return 1;
}

#elif(LCM_OPTION == LCM_HX8257_RGB666_480x272)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000700; //Lucian: 改變IDU clock : 192/8= 24 MHz. framerate=50 fps.
    IduEna  =   IDU_DCLK_EN;

    gpioSetLevel(0, GPIO_LCD_CTRL, 1);
#if(HW_BOARD_OPTION==PROJECT_DW950_REAL)
    //gpioSetLevel(0, GPIO_LCD_BL, 1);    // delay backlight open
#else
    gpioSetLevel(0, GPIO_LCD_BL, 1);
#endif

    IDU_Init(0 , 1);
    return 1;
}

#elif(LCM_OPTION == LCM_HX8257_SRGB_480x272)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000f00; //Edison: 改變IDU clock : 216/16= 13.5 MHz. framerate=30 fps.
    IduEna  =   IDU_DCLK_EN;



    IDU_Init(0 , 1);
    return 1;
}

#elif(LCM_OPTION == LCM_HX8257_P_RGB_480x272)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000700; //Edison: 改變IDU clock : 216/8= 24 MHz. framerate=30 fps.
    IduEna  =   IDU_DCLK_EN;



    IDU_Init(0 , 1);
    return 1;
}

#elif(LCM_OPTION == LCM_sRGB_HD15_HDMI)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


		temp= SYS_CLK1;
		SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000400; //Edison: 改變IDU clock : 216/9= 24 MHz. framerate=50 fps.
    IduEna  =   IDU_DCLK_EN;

    IDU_Init(0 , 1);
    return 1;
}

#elif(LCM_OPTION == LCM_TM024HDH29)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif
    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00001000; //Lucian: 改變IDU clock : 192/20= 9.6 MHz.
    //IduEna  =   IDU_DCLK_EN;
    IduEna  = 0x00000000 ;
    IDU_Init(0 , 1);

    Gpio3Ena |= 0x00000010 ;
    Gpio3Dir &= ~0x00000010;
    gpioSetLevel(3, 4, 1);

    //Reset
    gpioSetLevel(0, 3, 1);
    iduDelay(600);
    gpioSetLevel(0, 3, 0);
    iduDelay(7200);
    gpioSetLevel(0, 3, 1);
    iduDelay(72000);

    // Initial
    // Command 0x36
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000036;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000008;
    //IduMpuCmd = 0x00000028;
    IduMpuCmd = 0x000000e8;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);

    // Command 0x01
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);

    // Command 0x11
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    //DEBUG_IDU("IduMpuCmdCfg = %x\n", IduMpuCmdCfg);
    gpioSetLevel(3, 4, 1);
    iduDelay(36000);

    // Command 0xcb
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000cb;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xcb, 1);

    // Command 0xc0
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000c0;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000026;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xc0, 2);

    // Command 0xc1
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000c1;
    iduWaitCmdBusy();
    iduDelay(3000);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(2000);
    gpioSetLevel(3, 4, 1);
    //Read
    MPU_Read_Register(0xc1, 1);

    // Command 0xc5
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000c5;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000002e;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000003c;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xc5, 2);

    /*
    // Command 0x36
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000036;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0x36, 1);
    */

    // Command 0xb1
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000b1;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000018;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xb1, 2);

    // Command 0xb6
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000b6;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000a;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000082;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xb6, 2);

    // Command 0xc7
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000c7;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000bb;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xc7, 1);

    // Command 0xf2
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000f2;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xf2, 1);

    // Command 0x26
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000026;
    iduWaitCmdBusy();
    iduDelay(300);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);
    gpioSetLevel(3, 4, 1);
    //Read
    MPU_Read_Register(0x26, 1);

    //Gamma
    // Command 0xe0
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000e0;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000000e;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000017;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000009;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000004a;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000086;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000002b;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000000d;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(200);
    //Read
    MPU_Read_Register(0xe0, 15);

    //Gamma
    // Command 0xe1
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000e1;
    iduWaitCmdBusy();
    iduDelay(200);
    //data
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000001a;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000020;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000000e;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000003a;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000008a;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000040;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x00000018;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000000f;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000003f;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000003f;
    iduWaitCmdBusy();
    iduDelay(200);
    IduMpuCmd = 0x0000003f;
    iduWaitCmdBusy();
    iduDelay(200);
    gpioSetLevel(3, 4, 1);
    //Read
    MPU_Read_Register(0xe1, 15);

    // Command 0x29
    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000029;
    iduWaitCmdBusy();
    gpioSetLevel(3, 4, 1);
    iduDelay(300);


}

#elif(LCM_OPTION == LCM_TG200Q04)
s32 LCM_IDUInit(void)
{
    u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
    //Default: LCD is ON,TV is OFF
    sysIDU_enable();
#endif


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00001400; //Lucian: 改變IDU clock : 192/20= 9.6 MHz.
    IduEna  =   IDU_DCLK_EN;
    IDU_Init(0 , 1);

#if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
	IduBypassCtrl |= 0x00000001 ;
    IduMpuCmdCfg |= 0x000A0000 ;
    SYS_CLK1 |= 0x00000400;
#endif

#if(PANEL_SELECT == 2)  //TG200R00 Panel
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8; // command data width is 8 bits
    gpioSetLevel(3, 5, 0);
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000001C;
    //IduMpuCmd = 0x0000001B;	//JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    //IduMpuCmd = 0x00000700;  //JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);


    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000010;
    IduMpuCmd = 0x00000012;//JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    //IduMpuCmd = 0x00000030;  //Normal
    IduMpuCmd = 0x00000028;  //Turn Right
    //IduMpuCmd = 0x00000018;  //Turn Left
    iduWaitCmdBusy();
    iduDelay(200);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000B;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    //IduMpuCmd = 0x00000000; //JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000C;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    //IduMpuCmd = 0x00000001;  //JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000F;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000F;
    //IduMpuCmd = 0x00000008;  //JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(400);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000020;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    //IduMpuCmd = 0x00000000;  //Normal, Turn Left
    IduMpuCmd = 0x000000AF; //Turn Right
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000021;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;  //Normal, Turn Right
    //IduMpuCmd = 0x000000DB;  //Turn Left
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000038;
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000012;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000021;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000013;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000066;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000014;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000005F;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000060;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000030;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000031;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000032;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000033;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000034;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000035;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000036;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000AF;  //Normal
    //IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000037;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000038;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;  //Normal
    //IduMpuCmd = 0x000000AF;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000039;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000050;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x00000004;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000051;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x0000000B;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000052;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x0000000C;
    IduMpuCmd = 0x0000000D;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000053;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x00000001;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x00000005;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000054;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x0000000C;
    IduMpuCmd = 0x0000000D;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000055;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x0000000B;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000056;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x00000004;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000057;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x00000005;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x00000001;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000058;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///IduMpuCmd = 0x0000000E;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000059;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmd = 0x0000000E;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms
    IduMpuCmd = 0x00000017;
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms

#elif(PANEL_SELECT == 1)  //TG200Q23 Panel
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8; // command data width is 8 bits
    gpioSetLevel(3, 5, 0);
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    iduDelay(300);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000DE;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000AA;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000055;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000015;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C2;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C0;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C3;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C4;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C5;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000002A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000027;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C7;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C9;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000C9;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000B1;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000C;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000B4;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000F3;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000F9;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000019;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000E0;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000020;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000063;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000071;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000080;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000094;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms


    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x000000E1;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000056;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000033;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000076;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000E9;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000001F;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000002A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    //IduMpuCmd = 0x000000AF;
    IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000002B;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    //IduMpuCmd = 0x000000DB;
    IduMpuCmd = 0x000000AF;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000003A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000005;
    IduMpuCmd = 0x000000E6;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000036;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000048;
    IduMpuCmd = 0x00000038;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000029;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;

#else  //TG200Q04

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8; // command data width is 8 bits
    gpioSetLevel(3, 5, 0);
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms


    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    ///IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000001C;
    //IduMpuCmd = 0x0000001B;	//JJ
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms




    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    //IduMpuCmd = 0x00000700;  //JJ
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms
    ///IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);


    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000003;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000010;
    IduMpuCmd = 0x00000012;//JJ
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms
    ///IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    //IduMpuCmd = 0x00000030;  //Normal
    IduMpuCmd = 0x00000028;  //Turn Right
    //IduMpuCmd = 0x00000018;  //Turn Left
    iduWaitCmdBusy();
    iduDelay(200);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000008;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms
    ///IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000002;
    iduWaitCmdBusy();
    iduDelay(200);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x0000000B;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    //IduMpuCmd = 0x00000000; //JJ
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x0000000C;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    //IduMpuCmd = 0x00000001;  //JJ
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x0000000F;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000F;
    //IduMpuCmd = 0x00000008;  //JJ
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(400);
    //iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000020;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    //IduMpuCmd = 0x00000000;  //Normal, Turn Left
    IduMpuCmd = 0x000000AF; //Turn Right
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000021;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;  //Normal, Turn Right
    //IduMpuCmd = 0x000000DB;  //Turn Left
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(400);// delay 10 ms
    //iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000038;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(400);// delay 10 ms
    //iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000012;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000021;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000013;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000066;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000014;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000005F;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000060;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000030;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000031;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000032;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000033;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000034;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000035;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000036;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000AF;  //Normal
    //IduMpuCmd = 0x000000DB;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000037;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000038;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x000000DB;  //Normal
    //IduMpuCmd = 0x000000AF;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000039;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000050;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000051;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000000B;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000052;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000C;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000053;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000054;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000A;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000000C;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000055;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000B;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000006;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000056;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000004;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000057;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000005;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000058;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x0000000E;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(200);// delay 10 ms

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000059;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x0000000E;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(400);// delay 10 ms
    //iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(400);// delay 10 ms
    IduMpuCmd = 0x00000017;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    iduDelay(400);// delay 10 ms
    ///iduDelay(0xffff);

    /*
    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    //iduDelay(100);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000001;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    ///iduDelay(200);// delay 10 ms
    iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    //iduDelay(100);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000011;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    ///iduDelay(200);// delay 10 ms
    iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    //iduDelay(100);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000000;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000013;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    ///iduDelay(200);// delay 10 ms
    iduDelay(0xffff);

    IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 0);
    IduMpuCmd = 0x00000007;
    iduWaitCmdBusy();
    //iduDelay(100);// delay 10 ms
    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    IduMpuCmd = 0x00000010;
    iduWaitCmdBusy();
    iduDelay(200);// delay 10 ms
    IduMpuCmd = 0x00000017;
    iduWaitCmdBusy();
    ///gpioSetLevel(3, 4, 1);
    ///iduDelay(200);// delay 10 ms
    iduDelay(0xffff);

    //IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    ///gpioSetLevel(3, 4, 1);
    //iduDelay(200);// delay 10 ms
    */
#endif

    return 1;
}
#elif(LCM_OPTION ==LCM_HX8312) //For LCM8312
s32 LCM_IDUInit(void)
{
     u32 temp,i;
#if DINAMICALLY_POWER_MANAGEMENT
     //Default: LCD is ON,TV is OFF
     sysIDU_enable();
#endif


    IduEna  =   IDU_DCLK_EN;
    IduMpuCmdCfg = IDU_CMD_W_16; // command data width is 16 bits
    //Initialize LCM
    IduMpuCmd = 0x00000110;
    IduMpuCmd = 0x000000a0;
    IduMpuCmd = 0x00000301;


    switch( rotation )
    {
        case 0:
            break;
        case 1:
            IduMpuCmd = 0x00000504;
            IduMpuCmd = 0x000001d0;
            break;
    }

    iduDelay(200);// delay 10 ms

    iduWaitCmdBusy();
    IduMpuCmd = 0x00000300;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00002b04;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00005901;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00006022;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00005900;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00002818;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001a05;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00002505;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001900;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001c73;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00002474;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001e01;
    iduWaitCmdBusy();
    IduMpuCmd = 0x000018c1;
    iduWaitCmdBusy();

    iduDelay(200); // delay 10 ms

    IduMpuCmd = 0x000018e1;
    iduWaitCmdBusy();
    IduMpuCmd = 0x000018f1;
    iduWaitCmdBusy();

    iduDelay(1200); // delay 60 ms

    IduMpuCmd = 0x000018f5;
    iduWaitCmdBusy();

    iduDelay(1200); // delay 60 ms

    IduMpuCmd = 0x00001b09;
    iduWaitCmdBusy();

    iduDelay(200); // delay 10 ms

    IduMpuCmd = 0x00001f11;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00002010;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001e81;
    iduWaitCmdBusy();

    iduDelay(200); // delay 10 ms

    IduMpuCmd = 0x00009d00;
    iduWaitCmdBusy();

    iduDelay(200); // delay 10 ms

    IduMpuCmd = 0x0000c000;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000c100;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00000e00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00000f00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001000;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001100;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001200;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001300;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001400;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001500;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001600;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001700;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003401;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003500;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004b00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004c00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004e00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004f00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00005000;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003c00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003d00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003e01;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003f3f;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004002;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004102;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004200;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004301;
    iduWaitCmdBusy();
    IduMpuCmd = 0x0000443f;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004500;
    iduWaitCmdBusy();
    IduMpuCmd = 0x000046ef;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004700;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004800;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003d00;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004901;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004a3f;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00001d08;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008600;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008730;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008802;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008905;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008d01;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008b30;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003301;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00003701;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00007600;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00008f10;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009067;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009107;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009265;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009307;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009401;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009576;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009656;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009700;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009800;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009900;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00009a00;
    iduWaitCmdBusy();

    //Display on
    IduMpuCmd = 0x00003b01;
    iduWaitCmdBusy();

    iduDelay(800);

    IduMpuCmd = 0x00000020;
    iduWaitCmdBusy();

    switch(version)
    {
        case 0: // old
            if (rotation)
                IduMpuCmd = 0x000001d0;
            else
            IduMpuCmd = 0x00000190;
            iduWaitCmdBusy();
            IduMpuCmd = 0x00000000;
            iduWaitCmdBusy();
            break;
        case 1: // new
            break;
    }

    return 1;
}

#endif



#if (LCM_OPTION == LCM_HX5073_RGB)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
    HDEND       = 0x140;    // width            0xc0120010 0-15
    HPD         = 0x186;    // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x153;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x15D;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x153;    // width + 16       0xc0120018 0-16

    VDEND       = 0xF0; // height           0xc012001c 0-15
    VPD         = 0x106;    //              0xc012001c 16-31
    VSYNC_S     = 0xF1; //              0xc0120020 0-15
    VSYNC_E     = 0xF2; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif (LCM_OPTION == LCM_HX5073_YUV)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x280;    // width            0xc0120010 0-15
    HPD         = 0x30C;    // width + 64       0xc0120010 16-31
    //HSYNC_S       = 0xAE; // width + 32       0xc0120014 0-15
    HSYNC_S     = 0x294;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x2A8;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x294;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xF0; // height           0xc012001c 0-15
    VPD         = 0x106;    //              0xc012001c 16-31
    //VSYNC_S       = 0xFA; //              0xc0120020 0-15
    VSYNC_S     = 0xF1; //              0xc0120020 0-15
    VSYNC_E     = 0xF2; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_TMT035DNAFWU24_320x240)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND           = 320;  // width
    HFETCH_S        = 332;   // width + 16
    HSYNC_S         = 400;// 404;     // width + 32
    HSYNC_E         = 401;// 406;     // width + 48
    HPD             = 428;         // width + 64

    //DVTIME
    VDEND           = 240; // height
    VSYNC_S         = 244; //
    VSYNC_E         = 246; //
    VPD             = 260;
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_LQ035NC111)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND           = 320;  // width
    HFETCH_S        = 336;  // width + 16
    HSYNC_S         = 340;  // 320 + 60/3 = 340
    HSYNC_E         = 342;  //
    HPD             = 408;  // 1224/3 = 408

    //DVTIME
    VDEND           = 240; // height
    VSYNC_S         = 244; // 240 + 4 = 204
    VSYNC_E         = 260; // 244+(18-2) = 260
    VPD             = 262;
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif ( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_TPG105) )
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0xA0; // width            0xc0120010 0-15
    HPD     = 0xCE; // width + 64       0xc0120010 16-31
    HSYNC_S     = 0xAE; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0xBE; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0xA8; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xf0; // height           0xc012001c 0-15
    VPD         = 0x107;    //              0xc012001c 16-31
    VSYNC_S     = 0xF8; //              0xc0120020 0-15
    VSYNC_E     = 0xFD; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif( (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) )
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x280;    // width            0xc0120010 0-15
    HPD         = 0x30C;    // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x281;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x30B;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x281;    // width + 16       0xc0120018 0-16

    VDEND       = 0xF0;     // height=240           0xc012001c 0-15
    VPD         = 0x106;    // period=262             0xc012001c 16-31
    VSYNC_S     = 0xF1; //              0xc0120020 0-15
    VSYNC_E     = 0x105; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif(LCM_OPTION == LCM_CCIR601_640x480P)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x280;        // width            0xc0120010 0-15
    HPD         = 0x30C;        // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x281;        // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x30B;        // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x281;        // width + 16       0xc0120018 0-16

    VDEND       = 0x1E0;        // height=480       0xc012001c 0-15
    VPD         = 0x220;        // period=544       0xc012001c 16-31
    VSYNC_S     = VDEND + 1;    //                  0xc0120020 0-15
    VSYNC_E     = VPD - 1;      //                  0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_HX8817_RGB)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x1e0;    // width=480            0xc0120010 0-15
    HPD         = 0x258;    // H period=600       0xc0120010 16-31
    HSYNC_S     = 0x212;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x220;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x1e1;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xf0;     // height=240           0xc012001c 0-15
    VPD         = 0x106;    // V period=262             0xc012001c 16-31
    VSYNC_S     = 0xf4;     //              0xc0120020 0-15
    VSYNC_E     = 0xfd;     //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif(LCM_OPTION == LCM_HX8257_RGB666_480x272)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 480;    // width=480            0xc0120010 0-15
    HPD         = 525;    // H period=600       0xc0120010 16-31
    HSYNC_S     = 485;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 486;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 482;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 272;     // height=240           0xc012001c 0-15
    VPD         = 288;    // V period=262             0xc012001c 16-31
    VSYNC_S     = 280;     //              0xc0120020 0-15
    VSYNC_E     = 281;     //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif(LCM_OPTION == LCM_HX8257_SRGB_480x272)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

	HDEND       = 480;    // width=480            0xc0120010 0-15
    HPD         = 525;    // H period=600       0xc0120010 16-31
    HSYNC_S     = 482;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 523;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 482;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 272;     // height=240           0xc012001c 0-15
    VPD         = 286;    // V period=262             0xc012001c 16-31
    VSYNC_S     = 274;     //              0xc0120020 0-15
    VSYNC_E     = 284;     //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif(LCM_OPTION == LCM_HX8257_P_RGB_480x272)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

	HDEND       = 480;    // width=480            0xc0120010 0-15
    HPD         = 576;    // H period=600       0xc0120010 16-31
    HSYNC_S     = 488;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 529;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 482;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 272;     // height=240           0xc012001c 0-15
    VPD         = 287;    // V period=262             0xc012001c 16-31
    VSYNC_S     = 275;     //              0xc0120020 0-15
    VSYNC_E     = 285;     //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif(LCM_OPTION == LCM_sRGB_HD15_HDMI)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

	HDEND       = 1280;    // width=480            0xc0120010 0-15
    HPD         = 1326;    // H period=600       0xc0120010 16-31
    HSYNC_S     = 1290;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 1306;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 1282;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 720;     // height=240           0xc012001c 0-15
    VPD         = 724;    // V period=262             0xc012001c 16-31
    VSYNC_S     = 721;     //              0xc0120020 0-15
    VSYNC_E     = 722;     //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif(LCM_OPTION == LCM_TJ015NC02AA)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0xA0;
    HPD         = 0xCB;
    HSYNC_S     = 0xAA;
    HSYNC_E     = 0xAB;
    HFETCH_S    = 0xAA;

    //DVTIME
    VDEND       = 0xf0;
    VPD         = 0x107;
    VSYNC_S     = 0xFA;
    VSYNC_E     = 0xFD;

    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif (LCM_OPTION == LCM_TD020THEG1)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0xD5; // width            0xc0120010 0-15
    HPD         = 0x111; // width + 64      0xc0120010 16-31
    HSYNC_S     = 0xEA; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0xEB; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0xEA; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xf0; // height           0xc012001c 0-15
    VPD         = 0x107;    //              0xc012001c 16-31
    VSYNC_S     = 0xFA; //              0xc0120020 0-15
    VSYNC_E     = 0xFD; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif(LCM_OPTION == LCM_TD036THEA3_320x240)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 320; // width            0xc0120010 0-15
    HPD         = 390; // width + 64      0xc0120010 16-31
    HSYNC_S     = 340; // width + 32       0xc0120014 0-15
    HSYNC_E     = 341; // width + 48       0xc0120014 16-31
    HFETCH_S    = 339; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 240; // height           0xc012001c 0-15
    VPD         = 262;    //              0xc012001c 16-31
    VSYNC_S     = 248; //              0xc0120020 0-15
    VSYNC_E     = 249; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_TD024THEB2)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 160;
    HPD         = 220;
    HSYNC_S     = 188;
    HSYNC_E     = 189;
    HFETCH_S    = 188;


    VDEND       = 240;//0xf0;
    VPD         = 263;//0x107;
    VSYNC_S     = 248;//0xFA;
    VSYNC_E     = 249;//0xFD;
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_TD024THEB2_SRGB)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 480;//160;
    HPD         = 659;
    HSYNC_S     = 480+83;//188;
    HSYNC_E     = 480+83+1;
    HFETCH_S    = 480+82;


    VDEND       = 240;//0xf0;
    VPD         = 263;//0x107;
    VSYNC_S     = 248;//0xFA;
    VSYNC_E     = 249;//0xFD;
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif (LCM_OPTION ==LCM_A015AN04)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x5D; // width            0xc0120010 0-15
    HPD     = 0x78; // width + 64       0xc0120010 16-31
    //HSYNC_S       = 0xAA; // width + 32       0xc0120014 0-15
    HSYNC_S     = 0x62; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x6E; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x62; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xDC; // height           0xc012001c 0-15
    VPD         = 0x106;    //              0xc012001c 16-31
    VSYNC_S     = 0xED; //              0xc0120020 0-15
    VSYNC_E     = 0xEE; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif((LCM_OPTION ==LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02))
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0xA0; // width            0xc0120010 0-15
    HPD     = 0xCE; // width + 64       0xc0120010 16-31
    //HSYNC_S       = 0xAA; // width + 32       0xc0120014 0-15
    HSYNC_S     = 0xAE; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0xBE; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0xA8; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xF0; // height           0xc012001c 0-15
    VPD         = 0x107;    //              0xc012001c 16-31
    VSYNC_S     = 0xFA; //              0xc0120020 0-15
    VSYNC_E     = 0xFD; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_HX8224_SRGB)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 0x1e0;    // width            0xc0120010 0-15
    HPD         = 0x26a;    // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x20a;    // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x23a;    // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x1f8;    // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0xf0; // height           0xc012001c 0-15
    VPD         = 0x107;    //              0xc012001c 16-31
    VSYNC_S     = 0xFA; //              0xc0120020 0-15
    VSYNC_E     = 0xFD; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_HX8312)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND           = 0x140;  // width
    HFETCH_S        = 0x150;   // width + 16
    HSYNC_S         = 0x160;     // width + 32
    HSYNC_E         = 0x170;     // width + 48
    HPD             = 0x180;         // width + 64

    //DVTIME
    VDEND           = 0xf0; // height
    VSYNC_S         = 0xf1; //
    VSYNC_E         = 0xf2; //
    VPD             = 0xf3;
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#elif (LCM_OPTION == LCM_P_RGB_888_ILI6122)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
    u16 HB_porch,HF_porch,HPlus_width;
    u16 VB_porch,VF_porch,VPlus_width;
/*Roy: because JESMAY change driver IC, it will occur a line in the right panel
       So, it should adjust sync value */

    HB_porch     = 46;
    HF_porch     = 210;
    HPlus_width  = 10;
    
    HDEND       = 800;
    HPD         = HDEND + HF_porch + HPlus_width + HB_porch ;
    HSYNC_S     = HDEND + HF_porch;
    HSYNC_E     = HDEND + HF_porch + HPlus_width;
    HFETCH_S    = 804;

    //DVTIME
    VB_porch     = 23;
    VF_porch     = 42;
    VPlus_width  = 10;

    VDEND       = 480;
    VPD         = VDEND + VF_porch + VPlus_width + VB_porch;
    VSYNC_S     = VDEND + VF_porch ;
    VSYNC_E     = VDEND + VF_porch +VPlus_width;

    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif (LCM_OPTION == LCM_P_RGB_888_ILI6126C)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
    u16 HB_porch,HF_porch,HPlus_width;
    u16 VB_porch,VF_porch,VPlus_width;

    HB_porch     = 72;
    HF_porch     = 180;
    HPlus_width  = 48;

    HDEND       = 800;
    HPD         = HDEND + HF_porch + HPlus_width + HB_porch ;
    HSYNC_S     = HDEND + HF_porch;
    HSYNC_E     = HDEND + HF_porch + HPlus_width;
    HFETCH_S    = 802;

    //DVTIME
    VB_porch     = 1;
    VF_porch     = 3;
    VPlus_width  = 1;

    VDEND       = 480;
    VPD         = VDEND + VF_porch + VPlus_width + VB_porch;
    VSYNC_S     = VDEND + VF_porch ;
    VSYNC_E     = VDEND + VF_porch +VPlus_width;

    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}
#elif((LCM_OPTION ==LCM_P_RGB_888_HannStar)  || (LCM_OPTION == LCM_P_RGB_888_AT070TN90) )
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
/*Roy: because JESMAY change driver IC, it will occur a line in the right panel
       So, it should adjust sync value */

#if (HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)
    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 914; // width + 64       0xc0120010 16-31
    HSYNC_S     = 826; // width + 32       0xc0120014 0-15
    HSYNC_E     = 893; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 480; // height           0xc012001c 0-15
    VPD         = 525;    //              0xc012001c 16-31
    VSYNC_S     = 493; //              0xc0120020 0-15
    VSYNC_E     = 496; //              0xc0120020 16-31

#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 914; // width + 64       0xc0120010 16-31
    HSYNC_S     = 868;//864; // width + 32       0xc0120014 0-15
    HSYNC_E     = 878;//874; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 480; // height           0xc012001c 0-15
    VPD         = 525;    //              0xc012001c 16-31
    VSYNC_S     = 500;//493; //              0xc0120020 0-15
    VSYNC_E     = 503;//496; //              0xc0120020 16-31
#else
	HDEND		= 800; // width 		   0xc0120010 0-15
	HPD 		= 914; // width + 64	   0xc0120010 16-31
	HSYNC_S 	= 864; // width + 32		 0xc0120014 0-15
	HSYNC_E 	= 874; // width + 48		 0xc0120014 16-31
	HFETCH_S	= 802; // width + 16	   0xc0120018 0-16

	//DVTIME
	VDEND		= 480; // height		   0xc012001c 0-15
	VPD 		= 525;	  //			  0xc012001c 16-31
	VSYNC_S 	= 493; // 			 0xc0120020 0-15
	VSYNC_E 	= 496; // 			 0xc0120020 16-31


#endif

#if 0
    HDEND       = 0x320; // width            0xc0120010 0-15
    HPD         = 0x370; // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x330; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x340; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x322; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0x1e0; // height           0xc012001c 0-15
    VPD         = 0x1f8;    //              0xc012001c 16-31
    VSYNC_S     = 0x1e8; //              0xc0120020 0-15
    VSYNC_E     = 0x1F0; //              0xc0120020 16-31
#endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}
#elif(LCM_OPTION ==LCM_P_RGB_888_FC070227)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

#if 1
    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 914;//914; // width + 64       0xc0120010 16-31
    HSYNC_S     = 868;//864; // width + 32       0xc0120014 0-15
    HSYNC_E     = 869;//874; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 480; // height           0xc012001c 0-15
    VPD         = 525;//525;    //              0xc012001c 16-31
    VSYNC_S     = 502;//493; //              0xc0120020 0-15
    VSYNC_E     = 503;//496; //              0xc0120020 16-31
#else
// test 20151202 KD070D10-50NB-H14
    HDEND       = 800;
    HSYNC_S     = HDEND + 40;
    HSYNC_E     = HSYNC_S  + 48;
    HPD         = HSYNC_E  + 40;
    HFETCH_S    = 802;
    
    //DVTIME
    VDEND       = 480;
    VPD         = 525;
    VSYNC_S     = 502;
    VSYNC_E     = 503;
#endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif(LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 1056;//914; // width + 64       0xc0120010 16-31
    HSYNC_S     = 1010;//864; // width + 32       0xc0120014 0-15
    HSYNC_E     = 1040;//874; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 480; // height           0xc012001c 0-15
    VPD         = 525;    //              0xc012001c 16-31
    VSYNC_S     = 493; //              0xc0120020 0-15
    VSYNC_E     = 496; //              0xc0120020 16-31
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif(LCM_OPTION ==LCM_P_RGB_888_SY700BE104)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;

#if 1
    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 914;//928;//914; // width + 64       0xc0120010 16-31
    HSYNC_S     = 826;//864; // width + 32       0xc0120014 0-15
    HSYNC_E     = 874;//874; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802;//802; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 480; // height           0xc012001c 0-15
    VPD         = 525; //              0xc012001c 16-31
    VSYNC_S     = 493; //              0xc0120020 0-15
    VSYNC_E     = 496; //              0xc0120020 16-31
#else
    HDEND       = 0x320; // width            0xc0120010 0-15
    HPD         = 0x370; // width + 64       0xc0120010 16-31
    HSYNC_S     = 0x330; // width + 32       0xc0120014 0-15
    HSYNC_E     = 0x340; // width + 48       0xc0120014 16-31
    HFETCH_S    = 0x322; // width + 16       0xc0120018 0-16

    //DVTIME
    VDEND       = 0x1e0; // height           0xc012001c 0-15
    VPD         = 0x1f8;    //              0xc012001c 16-31
    VSYNC_S     = 0x1e8; //              0xc0120020 0-15
    VSYNC_E     = 0x1F0; //              0xc0120020 16-31
#endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif((LCM_OPTION ==LCM_P_RGB_888_Innolux)  )
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;


#if 1
  HDEND       = 800;
  HPD         = 864;
  HSYNC_S     = 832;//816;
  HSYNC_E     = 834;//818;
  HFETCH_S    = 802;
  /*
    HDEND       = 800;
    HPD         = 1200;
    HSYNC_S     = 1138;
    HSYNC_E     = 1154;
    HFETCH_S    = 802;
    */
#else
    HDEND       = 800; // width            0xc0120010 0-15
    HPD         = 880; // width + 64       0xc0120010 16-31
    HSYNC_S     = 836; // width + 32       0xc0120014 0-15
    HSYNC_E     = 852; // width + 48       0xc0120014 16-31
    HFETCH_S    = 802; // width + 16       0xc0120018 0-16
#endif

    //DVTIME
#if 1
    VDEND       = 600; // height           0xc012001c 0-15
    VPD         = 624;    //              0xc012001c 16-31
    VSYNC_S     = 600; //              0xc0120020 0-15
    VSYNC_E     = 601; //              0xc0120020 16-31

#else
    VDEND       = 600; // height           0xc012001c 0-15
    VPD         = 624;    //              0xc012001c 16-31
    VSYNC_S     = 608; //              0xc0120020 0-15
    VSYNC_E     = 616; //              0xc0120020 16-31
#endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;


#if (LCM_OPTION == VGA_640X480_60HZ)
    HDEND       = 640;
    HPD         = 800;
    HSYNC_S     = 656;
    HSYNC_E     = 752;;
    HFETCH_S    = 642;

    VDEND       = 480;
    VPD         = 525;
    VSYNC_S     = 490;
    VSYNC_E     = 492;
#elif (LCM_OPTION == VGA_800X600_60HZ)
    HDEND       = 800;
    HPD         = 1056;
    HSYNC_S     = 840;
    HSYNC_E     = 968;;
    HFETCH_S    = 802;

    VDEND       = 600;
    VPD         = 628;
    VSYNC_S     = 601;
    VSYNC_E     = 605;

#elif (LCM_OPTION == VGA_1024X768_60HZ)
    HDEND       = 1024;
    HPD         = 1344;
    HSYNC_S     = 1048;
    HSYNC_E     = 1184;;
    HFETCH_S    = 1026;

    VDEND       = 768;
    VPD         = 806;
    VSYNC_S     = 771;
    VSYNC_E     = 777;

#elif (LCM_OPTION == VGA_1280X800_60HZ)
    HDEND       = 1280;
    HPD         = 1680;
    HSYNC_S     = 1280+64;
    HSYNC_E     = 1280+64+136;;
    HFETCH_S    = 1282;

    VDEND       = 800;
    VPD         = 828;
    VSYNC_S     = 801;
    VSYNC_E     = 804;
#endif

    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);
}

#elif(LCM_OPTION == LCM_TM024HDH29)
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
  #if 0  //Normal
    HDEND           = 176;
    HFETCH_S        = 184;
    HSYNC_S         = 192;
    HSYNC_E         = 208;
    HPD             = 224;

    //DVTIME
    VDEND           = 220;
    VSYNC_S         = 221;
    VSYNC_E         = 222;
    VPD             = 223;
  #else

    HDEND           = 320;
    HFETCH_S        = 328;
    HSYNC_S         = 336;
    HSYNC_E         = 352;
    HPD             = 368;

    //DVTIME
    VDEND           = 240;
    VSYNC_S         = 241;
    VSYNC_E         = 242;
    VPD             = 243;

    /*
    HDEND           = 240;
    HFETCH_S        = 248;
    HSYNC_S         = 256;
    HSYNC_E         = 272;
    HPD             = 288;

    //DVTIME
    VDEND           = 320;
    VSYNC_S         = 321;
    VSYNC_E         = 322;
    VPD             = 323;
    */
  #endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}

#elif((LCM_OPTION == LCM_TG200Q04))
void iduTypeConfig()
{
    u16 HPD,HDEND,HSYNC_E,HSYNC_S,HFETCH_S;
    u16 VPD,VDEND,VSYNC_E,VSYNC_S;
  #if 0  //Normal
    HDEND           = 176;
    HFETCH_S        = 184;
    HSYNC_S         = 192;
    HSYNC_E         = 208;
    HPD             = 224;

    //DVTIME
    VDEND           = 220;
    VSYNC_S         = 221;
    VSYNC_E         = 222;
    VPD             = 223;
  #else
    HDEND           = 220;
    HFETCH_S        = 228;
    HSYNC_S         = 236;
    HSYNC_E         = 252;
    HPD             = 268;

    //DVTIME
    VDEND           = 176;
    VSYNC_S         = 177;
    VSYNC_E         = 178;
    VPD             = 179;
  #endif
    //0x0010
    IduHorzTimeCfg0 = (HPD << IDU_HPD_SHFT) | (HDEND << IDU_HDEND_SHFT);

    //0x0014
    IduHorzTimeCfg1 = (HSYNC_E << IDU_HSYNC_E_SHFT) | (HSYNC_S << IDU_HSYNC_S_SHFT);

    //0x0018
    IduHorzTimeCfg2 = (HFETCH_S << IDU_HFECTH_S_SHFT);

    IduVertTimeCfg0 = (VPD << IDU_VPD_SHFT) | (VDEND << IDU_VDEND_SHFT);
    IduVertTimeCfg1 = (VSYNC_E << IDU_VSYNC_E_SHFT) | (VSYNC_S << IDU_VSYNC_S_SHFT);

}
#endif

/*

Routine Description:

    The FIQ handler of Image Display Unit.

Arguments:

    None.

Return Value:

    None.

*/
void iduIntHandler(void)
{
    u8 err;
    int i;
    int VideoDispOffset;
    u32 intStat = IduIntCtrl;

#if DEBUG_IDU_INTR_USE_LED6
    static u32 count=0;
    gpioSetLevel(1, 6, count & 0x01);
    count ++;
#endif
#if 0//( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
      if(intStat & 0x00000008)
      {
          DEBUG_IDU("OSD-1 underflow\n");
      }
      if(intStat & 0x00000010)
      {
          DEBUG_IDU("Video underflow\n");
      }
      if(intStat & 0x00000020)
      {
          DEBUG_IDU("OSD-0 underflow\n");
      }
      if(intStat & 0x00000040)
      {
          DEBUG_IDU("OSD-2 underflow\n");
      }
#endif

#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
	if( sysCameraMode == SYS_CAMERA_MODE_PLAYBACK)
	{
		if(!ResetPlayback && !sysPlaybackVideoPause && StartPlayBack)
		{
			if(sysTVOutOnFlag) //TV-out
			{
				if(sysPlaybackForward>=0)
				{
					IDUVideoTimeOffset =  (IDUInterruptTime / 3) - ((VideoNextPresentTime - Video_timebase) >> sysPlaybackForward) ;
					if(IDUVideoTimeOffset <= DISPLAY_THRESHOLD && IDUVideoTimeOffset >= -DISPLAY_THRESHOLD) //Play frame
					{
						if(MainVideodisplaybuf_idx + 1 <= IsuIndex)
						{
							MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
					else if( IDUVideoTimeOffset > DISPLAY_THRESHOLD)   //Drop frame
					{
						//Lsk 090515 : Modify jump to IDUVideoTimeOffset in threshold range
						if(MainVideodisplaybuf_idx + 2 <= IsuIndex)
						{
							MainVideodisplaybuf_idx += 2;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
						else if(MainVideodisplaybuf_idx + 1 <= IsuIndex)
						{
							MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
				}
				else  //Lsk 090422 : Because read I frame in large size SD card very slow , so don't drop frame.
				{
					IDUVideoTimeOffset =  (IDUInterruptTime / 3) - ((Video_timebase - VideoNextPresentTime) >> sysPlaybackBackward) ;

					if( IDUVideoTimeOffset >= -DISPLAY_THRESHOLD)   //Play frame
					{
						if(MainVideodisplaybuf_idx + 1 <= IsuIndex)
						{
							MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
				}

				if(TvOutMode == SYS_TV_OUT_PAL)
			 		IDUInterruptTime += PAL_IDUInterruptUnit;
				else if(TvOutMode == SYS_TV_OUT_NTSC)
					IDUInterruptTime += NTSC_IDUInterruptUnit;
                else if(TvOutMode == SYS_TV_OUT_HD720P)
					IDUInterruptTime += NTSC_IDUInterruptUnit/2;
                else if(TvOutMode == SYS_TV_OUT_FHD1080I)
					IDUInterruptTime += NTSC_IDUInterruptUnit;
                else if(TvOutMode == SYS_TV_OUT_FHD1080P)
					IDUInterruptTime += NTSC_IDUInterruptUnit/2;
			}
			else
			{
				if(sysPlaybackForward>=0)
				{
					IDUVideoTimeOffset =  (IDUInterruptTime / 3) - ((VideoNextPresentTime - Video_timebase) >> sysPlaybackForward) ;
					if(IDUVideoTimeOffset <= DISPLAY_THRESHOLD && IDUVideoTimeOffset >= -DISPLAY_THRESHOLD) //Play frame
					{
						if(MainVideodisplaybuf_idx + 1 < IsuIndex)
						{
						    MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
					else if( IDUVideoTimeOffset > DISPLAY_THRESHOLD)   //Drop frame
					{
						//Lsk 090515 : Modify jump to IDUVideoTimeOffset in threshold range
						if(MainVideodisplaybuf_idx + 2 < IsuIndex)
						{
						    MainVideodisplaybuf_idx += 2;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
						else if(MainVideodisplaybuf_idx + 1 < IsuIndex)
						{
						    MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
				}
				else  //Lsk 090422 : Because read I frame in large size SD card very slow , so don't drop frame.
				{
					IDUVideoTimeOffset =  (IDUInterruptTime / 3) - ((Video_timebase - VideoNextPresentTime) >> sysPlaybackBackward) ;

					if( IDUVideoTimeOffset >= -DISPLAY_THRESHOLD)   //Play frame
					{
						if(MainVideodisplaybuf_idx + 1 < IsuIndex)
						{
						    MainVideodisplaybuf_idx += 1;
							VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
						}
					}
				}
				IDUInterruptTime +=     60000;  //Lsk 090508 : 60000 for test
			}

		}

	}
#endif

	if(sysTVOutOnFlag) //TV-out
	{
    	  #if TV_DISP_BY_IDU
            #if(TV_DISP_BY_TV_INTR)
                if( sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
                {
                    //--------------------For Mpeg recording mode---------------------//
                    if (siuOpMode == SIUMODE_MPEGAVI)
                    {
                    #if MULTI_CHANNEL_SUPPORT
                        switch(sysVideoInCHsel)
                        {
                           case 0:
                               if(isu_idufrmcnt != 0)
                               {
                                   IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                   IduVidBuf0Addr=((u32)PNBuf_Y[ (isu_idufrmcnt-1) & 0x03]);
                                  #if NEW_IDU_BRI
                                   BRI_IADDR_Y = IduVidBuf0Addr;
                                   BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  #endif
                               }
                               break;

                           case 1:
                               if(ciu_idufrmcnt_ch1 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU1_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + ciu_1_pnbuf_size_y;
                                  #if DUAL_MODE_DISP_SUPPORT
                                    if(sysDualModeDisp)
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth * 2);
                                    else
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth);
                                  #endif
                                #endif

                               }
                               break;

                           case 2:
                               if(ciu_idufrmcnt_ch2 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU2_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-1) & 0x03];
                                #endif
                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + ciu_2_pnbuf_size_y;
                                  #if DUAL_MODE_DISP_SUPPORT
                                    if(sysDualModeDisp)
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth * 2);
                                    else
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth);
                                  #endif
                                #endif
                               }
                               break;

                           case 3:
                               if(ciu_idufrmcnt_ch3 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU3_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub3[(ciu_idufrmcnt_ch3-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub3[(ciu_idufrmcnt_ch3-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                #endif
                               }
                               break;

                           case 4:
                               if(ciu_idufrmcnt_ch4 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU4_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub4[(ciu_idufrmcnt_ch4-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub4[(ciu_idufrmcnt_ch4-1) & 0x03];
                                #endif
                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                #endif
                               }
                               break;

                        }
                    #else
                        if(isu_idufrmcnt != 0)
                        {
                        #if ISUCIU_PREVIEW_PNOUT
                            IduVidBuf0Addr=((u32)PNBuf_Y[ (isu_avifrmcnt-1) & 0x03]);

                           #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                           #endif

                        #else
                            IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);
                            #if NEW_IDU_BRI
                            switch( (isu_idufrmcnt-1) % 3)
                            {
                               case 0:
                                  BRI_IADDR_Y = IduVidBuf0Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 1:
                                  BRI_IADDR_Y = IduVidBuf1Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 2:
                                  BRI_IADDR_Y = IduVidBuf2Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                            }
                            #endif
                        #endif
                        }
                    #endif
                    }
                    //--------------------Preview mode--------------------//
                    else
                    {
                    #if CIU_TEST
                        if(ciu_idufrmcnt_ch1 != 0)
                        {
                        #if ISUCIU_PREVIEW_PNOUT
                            IduWinCtrl = (IduWinCtrl & ~0x00003000);
                            IduVidBuf0Addr=((u32)PNBuf_Y[ (ciu_idufrmcnt_ch1-1) & 0x03]);
                           #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                           #endif
                        #else
                            IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (ciu_idufrmcnt_ch1-1) % 3) << 12);
                            #if NEW_IDU_BRI
                            switch( (ciu_idufrmcnt_ch1-1) % 3)
                            {
                               case 0:
                                  BRI_IADDR_Y = IduVidBuf0Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 1:
                                  BRI_IADDR_Y = IduVidBuf1Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 2:
                                  BRI_IADDR_Y = IduVidBuf2Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                            }
                            #endif
                        #endif
                        }
                    #elif MULTI_CHANNEL_SUPPORT
                        switch(sysVideoInCHsel)
                        {
                           case 0:
                               if(isu_idufrmcnt != 0)
                               {
                                   IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                   IduVidBuf0Addr=((u32)PNBuf_Y[ (isu_idufrmcnt-1) & 0x03]);
                                  #if NEW_IDU_BRI
                                   BRI_IADDR_Y = IduVidBuf0Addr;
                                   BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  #endif
                               }
                               break;

                           case 1:
                               if(ciu_idufrmcnt_ch1 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU1_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + ciu_1_pnbuf_size_y;
                                  #if DUAL_MODE_DISP_SUPPORT
                                    if(sysDualModeDisp)
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth * 2);
                                    else
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth);
                                  #endif
                                #endif

                               }
                               break;

                           case 2:
                               if(ciu_idufrmcnt_ch2 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU2_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + ciu_2_pnbuf_size_y;
                                  #if DUAL_MODE_DISP_SUPPORT
                                    if(sysDualModeDisp)
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth * 2);
                                    else
                                        BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth);
                                  #endif
                                #endif
                               }
                               break;

                            case 3:
                               if(ciu_idufrmcnt_ch3 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU3_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub3[(ciu_idufrmcnt_ch3-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub3[(ciu_idufrmcnt_ch3-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                #endif
                               }
                               break;

                            case 4:
                               if(ciu_idufrmcnt_ch4 != 0)
                               {
                                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                                #if HW_DEINTERLACE_CIU4_ENA
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub4[(ciu_idufrmcnt_ch4-2) & 0x03];
                                #else
                                    IduVidBuf0Addr = (unsigned int)PNBuf_sub4[(ciu_idufrmcnt_ch4-1) & 0x03];
                                #endif

                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                #endif
                               }
                               break;

                        }

                        #if(HWPIP_SUPPORT) //HWPIP support Amon 140514

                        switch(sysOsdInCHsel)
                        {
                            case PIP_MAIN_CH1:
                                if(ciu_idufrmcnt_ch1 != 0)
                                {
                                    OSD_ADDR_Y = (unsigned int)PNBuf_sub1[(ciu_idufrmcnt_ch1-1) & 0x03];
                                    OSD_ADDR_C = OSD_ADDR_Y + PNBUF_SIZE_Y;
                                }
                                break;
                            case PIP_MAIN_CH2:
                                if(ciu_idufrmcnt_ch2 != 0)
                                {
                                    OSD_ADDR_Y = (unsigned int)PNBuf_sub2[(ciu_idufrmcnt_ch2-1) & 0x03];
                                    OSD_ADDR_C = OSD_ADDR_Y + PNBUF_SIZE_Y;
                                }
                              break;
                        }
                        #endif
                    #else
                        if(isu_idufrmcnt != 0)
                        {
                        #if ISUCIU_PREVIEW_PNOUT
                           IduWinCtrl = (IduWinCtrl & ~0x00003000);
                           IduVidBuf0Addr=((u32)PNBuf_Y[ (isu_idufrmcnt-1) & 0x03]);
                           #if NEW_IDU_BRI
                              BRI_IADDR_Y = IduVidBuf0Addr;
                              BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                           #endif
                        #else
                           IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);
                            #if NEW_IDU_BRI
                            switch( (isu_idufrmcnt-1) % 3)
                            {
                               case 0:
                                  BRI_IADDR_Y = IduVidBuf0Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 1:
                                  BRI_IADDR_Y = IduVidBuf1Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 2:
                                  BRI_IADDR_Y = IduVidBuf2Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                            }
                            #endif
                        #endif
                        }
                    #endif
                    }
                }
              #if RFIU_SUPPORT
				else if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR  )
                {
                #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_RX_AVSYNC )
				       if(rfiuRxMainVideoPlayStart)
				       {
				       #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
				          if(TvOutMode == SYS_TV_OUT_PAL)
				            rfiuMainVideoTime +=40;
				          else if(TvOutMode == SYS_TV_OUT_NTSC)//NTSC
				          {
				            rfiuMainVideoTime += 33;
				            if(rfiuMainVideoTime_frac>=30)
				            {
				               rfiuMainVideoTime_frac -=30;
				               rfiuMainVideoTime +=1;
				            }
				            rfiuMainVideoTime_frac += 11;
				          }
                          else if(TvOutMode == SYS_TV_OUT_HD720P)
				          {
                            rfiuMainVideoTime += 16;
            	            if(rfiuMainVideoTime_frac>=10000)
            	            {
            	               rfiuMainVideoTime_frac -=10000;
            	               rfiuMainVideoTime +=1;
            	            }
            	            rfiuMainVideoTime_frac += 6667;
                          }
                          else if(TvOutMode == SYS_TV_OUT_FHD1080I)
				          {
                            rfiuMainVideoTime += 33;
            	            if(rfiuMainVideoTime_frac>=10000)
            	            {
            	               rfiuMainVideoTime_frac -=10000;
            	               rfiuMainVideoTime +=1;
            	            }
            	            rfiuMainVideoTime_frac += 3333;
                          }
					   #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
						  rfiuMainVideoTime=rfiuMainAudioTime;
				       #endif

				          VideoDispOffset= (rfiuVideoBufFill_idx[sysRFRxInMainCHsel] >= rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) ?  (rfiuVideoBufFill_idx[sysRFRxInMainCHsel]-rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) : (rfiuVideoBufFill_idx[sysRFRxInMainCHsel]+DISPLAY_BUF_NUM-rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) ;
					      if(VideoDispOffset > 2)
					      {
			                 if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM])
							 {
							 	 rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] +=1;
                             #if 1
                                 if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[ (rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM]+RFI_VIDEO_SYNC_SHIFT)
                                 {
                                    rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] +=1;
                                 #if RF_LETENCY_DEBUG_ENA
                                    DEBUG_IDU("DF ");
                                 #endif
                                 }
                             #endif

							 }
                             else
                             {
                             #if RF_LETENCY_DEBUG_ENA
                                    DEBUG_IDU("1 ");
                             #endif
                             }
						  }
				          else if(VideoDispOffset > 1)
				          {
				             if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[ (rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM])
				             {
				                  rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] ++;
				             }
                             else
                             {
                             #if RF_LETENCY_DEBUG_ENA
                                    DEBUG_IDU("2 ");
                             #endif
                             }
				          }
				          else
				          {
				             rfiuMainVideoTime = rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM];
				             rfiuMainVideoTime_frac=0;
                         #if RF_LETENCY_DEBUG_ENA
                             DEBUG_IDU("EP ");
                         #endif
				          }
				       }
                #endif

                    if( (gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_BROKEN) || (gRfiu_Op_Sta[sysRFRxInMainCHsel]==RFIU_OP_INIT) )
                    {
                        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                        IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];

                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    }
                    else
                    {
                        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                        if(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] == 0)
                            IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];
                        else
                            IduVidBuf0Addr= (u32)MainVideodisplaybuf[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) % DISPLAY_BUF_NUM];
                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    }
			    }
				else if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA ) )
                {
                #if( RFIU_RX_AVSYNC )
			       #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
                      if(TvOutMode == SYS_TV_OUT_PAL)
			            rfiuMainVideoTime +=40;
			          else if(TvOutMode == SYS_TV_OUT_NTSC)//NTSC
			          {
			            rfiuMainVideoTime += 33;
			            if(rfiuMainVideoTime_frac>=30)
			            {
			               rfiuMainVideoTime_frac -=30;
			               rfiuMainVideoTime +=1;
			            }
			            rfiuMainVideoTime_frac += 11;
			          }
                      else if(TvOutMode == SYS_TV_OUT_HD720P)
			          {
                        rfiuMainVideoTime += 16;
        	            if(rfiuMainVideoTime_frac>=10000)
        	            {
        	               rfiuMainVideoTime_frac -=10000;
        	               rfiuMainVideoTime +=1;
        	            }
        	            rfiuMainVideoTime_frac += 6667;
                      }
                      else if(TvOutMode == SYS_TV_OUT_FHD1080I)
			          {
                        rfiuMainVideoTime += 33;
        	            if(rfiuMainVideoTime_frac>=10000)
        	            {
        	               rfiuMainVideoTime_frac -=10000;
        	               rfiuMainVideoTime +=1;
        	            }
        	            rfiuMainVideoTime_frac += 3333;
                      }
				   #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
					  rfiuMainVideoTime=rfiuMainAudioTime;
			       #endif
                #endif

                    IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                    IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];

                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
			    }
                else if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR )
                {
                #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_RX_AVSYNC )
				       if(rfiuRxMainVideoPlayStart)
				       {
				       #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
				          if(TvOutMode == SYS_TV_OUT_PAL)
    			            rfiuMainVideoTime +=40;
    			          else if(TvOutMode == SYS_TV_OUT_NTSC)//NTSC
    			          {
    			            rfiuMainVideoTime += 33;
    			            if(rfiuMainVideoTime_frac>=30)
    			            {
    			               rfiuMainVideoTime_frac -=30;
    			               rfiuMainVideoTime +=1;
    			            }
    			            rfiuMainVideoTime_frac += 11;
    			          }
                          else if(TvOutMode == SYS_TV_OUT_HD720P)
    			          {
                            rfiuMainVideoTime += 16;
            	            if(rfiuMainVideoTime_frac>=10000)
            	            {
            	               rfiuMainVideoTime_frac -=10000;
            	               rfiuMainVideoTime +=1;
            	            }
            	            rfiuMainVideoTime_frac += 6667;
                          }
                          else if(TvOutMode == SYS_TV_OUT_FHD1080I)
    			          {
                            rfiuMainVideoTime += 33;
            	            if(rfiuMainVideoTime_frac>=10000)
            	            {
            	               rfiuMainVideoTime_frac -=10000;
            	               rfiuMainVideoTime +=1;
            	            }
            	            rfiuMainVideoTime_frac += 3333;
                          }
					   #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
						  rfiuMainVideoTime=rfiuMainAudioTime;
				       #endif

				          VideoDispOffset= (rfiuVideoBufFill_idx[0] >= rfiuVideoBufPlay_idx[0]) ?  (rfiuVideoBufFill_idx[0]-rfiuVideoBufPlay_idx[0]) : (rfiuVideoBufFill_idx[0]+DISPLAY_BUF_NUM-rfiuVideoBufPlay_idx[0]) ;
					      if(VideoDispOffset > 2)
					      {
                             if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM])
            				 {
            				 	 rfiuVideoBufPlay_idx[0] +=1;
                                 if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM] + RFI_VIDEO_SYNC_SHIFT)
                                 {
                                     rfiuVideoBufPlay_idx[0] +=1;
                                     #if RF_LETENCY_DEBUG_ENA
                                     DEBUG_IDU(" DF1\n");
                                     #endif
                                 }
            				 }
						  }
				          else if(VideoDispOffset > 1)
				          {
				             if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM])
				                  rfiuVideoBufPlay_idx[0] ++;
				          }
				          else
				          {
				             rfiuMainVideoTime = rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[0] % DISPLAY_BUF_NUM];
				             rfiuMainVideoTime_frac=0;
				          }
				       }
                #endif

                    if( (gRfiu_Op_Sta[0] == RFIU_RX_STA_LINK_BROKEN) || (gRfiu_Op_Sta[0]==RFIU_OP_INIT) )
                    {
                        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                        IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];

                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    }
                    else
                    {
                        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                        if(rfiuVideoBufPlay_idx[0] == 0)
                            IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];
                        else
                            IduVidBuf0Addr= (u32)MainVideodisplaybuf[(rfiuVideoBufPlay_idx[0]) % DISPLAY_BUF_NUM];
                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    }
			    }
              #endif
              #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
                else if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
                {
                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                    IduVidBuf0Addr=(u32)PNBuf_Quad;
                  #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                  #endif
                }
              #endif
                else //playback mode
                {
                    IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
                    if(MainVideodisplaybuf_idx == 0)
                        IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];
                    else
                        IduVidBuf0Addr= (u32)MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM];
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                    #endif
                }
            #else

            #endif
          #endif
	}
	else  //For Pannel-out
	{
	#if RFIU_SUPPORT
	   if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
	   {
	   #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_RX_AVSYNC )
	      if(rfiuRxMainVideoPlayStart)
	      {
		  #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		     #if(LCM_OPTION  == LCM_P_RGB_888_Innolux) // 1000/(32000000/3/864/624)=50.5440
                #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                rfiuMainVideoTime += 50;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 5440;
                #else
	            rfiuMainVideoTime += 32;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 4480;
                #endif
		     #elif((LCM_OPTION  == LCM_P_RGB_888_AT070TN90) || (LCM_OPTION == LCM_P_RGB_888_SY700BE104))
	            rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;
             #elif((LCM_OPTION  == LCM_P_RGB_888_HannStar) || (LCM_OPTION == LCM_P_RGB_888_FC070227))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;

             #elif((LCM_OPTION == LCM_P_RGB_888_ILI6122))
                #if USE_IDUCLK_SLOW
                rfiuMainVideoTime += 30;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 1293;
                #else
				rfiuMainVideoTime += 24;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 6513;
                #endif
            #elif((LCM_OPTION == LCM_P_RGB_888_ILI6126C))
              #if USE_IDUCLK_SLOW
               rfiuMainVideoTime += 29;
               if(rfiuMainVideoTime_frac>=10000)
               {
                  rfiuMainVideoTime_frac -=10000;
                  rfiuMainVideoTime +=1;
               }
               rfiuMainVideoTime_frac += 6388;
             #else
               rfiuMainVideoTime += 22;
               if(rfiuMainVideoTime_frac>=10000)
               {
                  rfiuMainVideoTime_frac -=10000;
                  rfiuMainVideoTime +=1;
               }
               rfiuMainVideoTime_frac += 2291;
             #endif

             #elif(LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)
                rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;

             #elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
				rfiuMainVideoTime += 16;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
                #if (LCM_OPTION == VGA_640X480_60HZ)
	            rfiuMainVideoTime_frac += 8000;
                #elif(LCM_OPTION == VGA_800X600_60HZ)
                rfiuMainVideoTime_frac += 5792;
                #elif(LCM_OPTION == VGA_1024X768_60HZ)
                rfiuMainVideoTime_frac += 6656;
                #elif(LCM_OPTION == VGA_1280X800_60HZ)
                rfiuMainVideoTime_frac += 7595;
                #endif

		     #elif((LCM_OPTION  == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;
		     #else
               rfiuMainVideoTime += 33;
			 #endif
	      #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
		      rfiuMainVideoTime=rfiuMainAudioTime;
          #endif

			  VideoDispOffset= (rfiuVideoBufFill_idx[sysRFRxInMainCHsel] >= rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) ?  (rfiuVideoBufFill_idx[sysRFRxInMainCHsel]-rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) : (rfiuVideoBufFill_idx[sysRFRxInMainCHsel]+DISPLAY_BUF_NUM-rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]) ;
		      if(VideoDispOffset > 2)
		      {
                 if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM])
				 {
				 	 rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] +=1;
                 #if 1
                     if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM] +RFI_VIDEO_SYNC_SHIFT)
                     {
                         rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] +=1;
                     #if RF_LETENCY_DEBUG_ENA
                         DEBUG_IDU("DF ");
                     #endif
                     }
                 #endif
				 }
                 else
                 {
                 #if RF_LETENCY_DEBUG_ENA
                     DEBUG_IDU("1 ");
                 #endif
                 }
			  }
	          else if(VideoDispOffset > 1)
	          {
	             if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]+1) % DISPLAY_BUF_NUM])
	             {
	                  rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] ++;
	             }
                 else
                 {
                 #if RF_LETENCY_DEBUG_ENA
                     DEBUG_IDU("2 ");
                 #endif
                 }
	          }
	          else
	          {
	             rfiuMainVideoTime = rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM];
	             rfiuMainVideoTime_frac=0;
             #if RF_LETENCY_DEBUG_ENA
                 DEBUG_IDU("EP ");
             #endif

	          }

	      }
          #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
              #if MENU_DONOT_SHARE_BUFFER
                  if(sysEnMenu == TRUE)
                     iduPlaybackFrame(uiMenuBuf2);
                  else if(sysEnSnapshot == TRUE)
                     iduPlaybackFrame(uiMenuBuf3);
                  else
                  {
                     if(sysEnZoom)
                  	    iduPlaybackFrame(MainVideodisplaybuf[0]+1280*2*120+320*2);
                     else
                     {
                        if(uiRFCHChg && ( (rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM) == 0 ) )
                        {
                            iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM]);
                            rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]=0;
                        }
                        else
                  	       iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM]);
                     }
                  }
              #else
                  iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM]);
              #endif
          #else    
              iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM]);
          #endif
	   #else
		  iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[sysRFRxInMainCHsel] % DISPLAY_BUF_NUM]);
	   #endif
	   }
       else if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR )
	   {
	   #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_RX_AVSYNC )
	      if(rfiuRxMainVideoPlayStart)
	      {
		  #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		     #if(LCM_OPTION  == LCM_P_RGB_888_Innolux) // 1000/(32000000/3/864/624)=50.5440
                #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                rfiuMainVideoTime += 50;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 5440;
                #else
	            rfiuMainVideoTime += 32;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 4480;
                #endif
		     #elif((LCM_OPTION  == LCM_P_RGB_888_AT070TN90) || (LCM_OPTION == LCM_P_RGB_888_SY700BE104))
	            rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;
             #elif((LCM_OPTION  == LCM_P_RGB_888_HannStar) || (LCM_OPTION == LCM_P_RGB_888_FC070227))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;

             #elif((LCM_OPTION == LCM_P_RGB_888_ILI6122))
                #if USE_IDUCLK_SLOW
                rfiuMainVideoTime += 30;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 1293;
                #else
				rfiuMainVideoTime += 24;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 6513;
                #endif
             #elif((LCM_OPTION == LCM_P_RGB_888_ILI6126C))
                #if USE_IDUCLK_SLOW
                rfiuMainVideoTime += 29;
                if(rfiuMainVideoTime_frac>=10000)
                {
                   rfiuMainVideoTime_frac -=10000;
                   rfiuMainVideoTime +=1;
                }
                rfiuMainVideoTime_frac += 9388;

                #else
                rfiuMainVideoTime += 22;
                if(rfiuMainVideoTime_frac>=10000)
                {
                   rfiuMainVideoTime_frac -=10000;
                   rfiuMainVideoTime +=1;
                }
                rfiuMainVideoTime_frac += 2291;
                #endif

             #elif(LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)
                rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;

             #elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
				rfiuMainVideoTime += 16;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
                #if (LCM_OPTION == VGA_640X480_60HZ)
	            rfiuMainVideoTime_frac += 8000;
                #elif(LCM_OPTION == VGA_800X600_60HZ)
                rfiuMainVideoTime_frac += 5792;
                #elif(LCM_OPTION == VGA_1024X768_60HZ)
                rfiuMainVideoTime_frac += 6656;
                #elif(LCM_OPTION == VGA_1280X800_60HZ)
                rfiuMainVideoTime_frac += 7595;
                #endif

		     #elif((LCM_OPTION  == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;
		     #else
               rfiuMainVideoTime += 33;
			 #endif
	      #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
		      rfiuMainVideoTime=rfiuMainAudioTime;
          #endif

			  VideoDispOffset= (rfiuVideoBufFill_idx[0] >= rfiuVideoBufPlay_idx[0]) ?  (rfiuVideoBufFill_idx[0]-rfiuVideoBufPlay_idx[0]) : (rfiuVideoBufFill_idx[0]+DISPLAY_BUF_NUM-rfiuVideoBufPlay_idx[0]) ;
		      if(VideoDispOffset > 2)
		      {
                 if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM])
				 {
				 	 rfiuVideoBufPlay_idx[0] +=1;
                     if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM] + RFI_VIDEO_SYNC_SHIFT)
                     {
                         rfiuVideoBufPlay_idx[0] +=1;
                         #if RF_LETENCY_DEBUG_ENA
                         DEBUG_IDU(" DF1\n");
                         #endif
                     }
				 }
			  }
	          else if(VideoDispOffset > 1)
	          {
	             if(rfiuMainVideoTime >= rfiuMainVideoPresentTime[(rfiuVideoBufPlay_idx[0]+1) % DISPLAY_BUF_NUM])
	                  rfiuVideoBufPlay_idx[0] ++;
	          }
	          else
	          {
	             rfiuMainVideoTime = rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[0] % DISPLAY_BUF_NUM];
	             rfiuMainVideoTime_frac=0;
	          }

	      }
		  iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[0] % DISPLAY_BUF_NUM]);
	   #else
		  iduPlaybackFrame(MainVideodisplaybuf[rfiuVideoBufPlay_idx[0] % DISPLAY_BUF_NUM]);
	   #endif
	   }
	   else if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) || ( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA ) )
	   {
	      #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		     #if(LCM_OPTION  == LCM_P_RGB_888_Innolux)
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                rfiuMainVideoTime += 50;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 5440;
               #else
                rfiuMainVideoTime += 32;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 4480;
               #endif
	       #elif((LCM_OPTION  == LCM_P_RGB_888_AT070TN90)|| (LCM_OPTION == LCM_P_RGB_888_SY700BE104))
	            rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;
             #elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
	            rfiuMainVideoTime += 16;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
                #if (LCM_OPTION == VGA_640X480_60HZ)
	            rfiuMainVideoTime_frac += 8000;
                #elif(LCM_OPTION == VGA_800X600_60HZ)
                rfiuMainVideoTime_frac += 5792;
                #elif(LCM_OPTION == VGA_1024X768_60HZ)
                rfiuMainVideoTime_frac += 6656;
                #elif(LCM_OPTION == VGA_1280X800_60HZ)
                rfiuMainVideoTime_frac += 7595;
                #endif

             #elif((LCM_OPTION  == LCM_P_RGB_888_HannStar)|| (LCM_OPTION == LCM_P_RGB_888_FC070227))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3229;
                
             #elif((LCM_OPTION == LCM_P_RGB_888_ILI6122))				
                #if USE_IDUCLK_SLOW
                rfiuMainVideoTime += 30;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 1293;
                #else
				rfiuMainVideoTime += 24;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 6513;
                #endif
             #elif(LCM_OPTION == LCM_P_RGB_888_ILI6126C)
                #if USE_IDUCLK_SLOW
                 rfiuMainVideoTime += 29;
                 if(rfiuMainVideoTime_frac>=10000)
                 {
                    rfiuMainVideoTime_frac -=10000;
                    rfiuMainVideoTime +=1;
                 }
                 rfiuMainVideoTime_frac += 6388;
                #else
                 rfiuMainVideoTime += 22;
                 if(rfiuMainVideoTime_frac>=10000)
                 {
                    rfiuMainVideoTime_frac -=10000;
                    rfiuMainVideoTime +=1;
                 }
                 rfiuMainVideoTime_frac += 2291;
                #endif
             #elif(LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)
                rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;
		     #elif((LCM_OPTION  == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
				rfiuMainVideoTime += 33;
	            if(rfiuMainVideoTime_frac>=10000)
	            {
	               rfiuMainVideoTime_frac -=10000;
	               rfiuMainVideoTime +=1;
	            }
	            rfiuMainVideoTime_frac += 3667;
		     #else
               rfiuMainVideoTime += 33;
			 #endif
          #endif
          #if( SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM )
                  if(sysEnSnapshot == TRUE)
                  {
                     if(sysEnMenu == TRUE)
                        iduPlaybackFrame(uiMenuBuf2);
                     else
                        iduPlaybackFrame(uiMenuBuf3);
                  } 
                  else
                  {
                     iduPlaybackFrame(MainVideodisplaybuf[0]);
                  }
          #else    
              iduPlaybackFrame(MainVideodisplaybuf[0]);
          #endif

          
	   }
	   else
     #endif
	   {
	   #if( (SW_APPLICATION_OPTION  !=  MR8100_BABYMONITOR) && (SW_APPLICATION_OPTION != MR8100_DUALMODE_VBM) )
		  iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
       #else
            #if MENU_DONOT_SHARE_BUFFER
                iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
            #endif
       #endif
	   }
	}
}

void subTVIntHandler(void)
{

    u8 err;
    int VideoDispOffset;

    u32 intStat = tv2TVE_INTC;

  #if TV_DISP_BY_IDU
        if( sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            //--------------------For Mpeg recording mode---------------------//
            if (siuOpMode == SIUMODE_MPEGAVI)
            {
                if(isu_idufrmcnt != 0)
                {
                    tv2FRAME_CTL = (tv2FRAME_CTL & ~0x00003000);
                    tv2TVFB_IADDR0=((u32)PNBuf_Y[ (isu_idufrmcnt-1) & 0x03]);
                }
            }
            //--------------------Preview mode--------------------//
            else
            {
            #if MULTI_CHANNEL_SUPPORT
                switch(sysVideoInCHsel)
                {
                   case 0:
                       if(isu_idufrmcnt != 0)
                       {
                           tv2FRAME_CTL = (tv2FRAME_CTL & ~0x00003000);
                           tv2TVFB_IADDR0=((u32)PNBuf_Y[ (isu_idufrmcnt-1) & 0x03]);
                       }
                       break;

                   case 1:
                       if(ciu_idufrmcnt_ch1 != 0)
                       {
                            tv2FRAME_CTL = (tv2FRAME_CTL & ~0x00003000);
                            switch( (ciu_idufrmcnt_ch1-1) & 0x03)
                            {
                              case 0:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub1[0];
                                 break;

                              case 1:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub1[1];
                                 break;

                              case 2:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub1[2];
                                 break;

                              case 3:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub1[3];
                                 break;
                            }
                       }
                       break;

                   case 2:
                       if(ciu_idufrmcnt_ch2 != 0)
                       {
                            tv2FRAME_CTL = (tv2FRAME_CTL & ~0x00003000);
                            switch( (ciu_idufrmcnt_ch2-1) & 0x03)
                            {
                              case 0:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub2[0];
                                 break;

                              case 1:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub2[1];
                                 break;

                              case 2:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub2[2];
                                 break;

                              case 3:
                                 tv2TVFB_IADDR0     = (unsigned int)PNBuf_sub2[3];
                                 break;
                            }
                       }
                       break;

                }
            #else
                if(isu_idufrmcnt != 0)
                {
                   tv2FRAME_CTL = (tv2FRAME_CTL & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);
                }
            #endif
            }
        }
    #if RFIU_SUPPORT
        else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
	    {
	       #if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_RX_AVSYNC )
		       if(rfiuRxSub1VideoPlayStart)
		       {
		       #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		          if(TvOutMode == SYS_TV_OUT_PAL)
		            rfiuSub1VideoTime += 40;
		          else if(TvOutMode == SYS_TV_OUT_NTSC)//NTSC
		          {
		            rfiuSub1VideoTime += 33;
		            if(rfiuSub1VideoTime_frac>=30)
		            {
		               rfiuSub1VideoTime_frac -=30;
		               rfiuSub1VideoTime +=1;
		            }
		            rfiuSub1VideoTime_frac += 11;
		          }
                  else if(TvOutMode == SYS_TV_OUT_HD720P)
		          {
		            rfiuSub1VideoTime += 16;
		            if(rfiuSub1VideoTime_frac>=10000)
		            {
		               rfiuSub1VideoTime_frac -=10000;
		               rfiuSub1VideoTime +=1;
		            }
		            rfiuSub1VideoTime_frac += 6667;
		          }
                  else if(TvOutMode == SYS_TV_OUT_FHD1080I)
		          {
		            rfiuSub1VideoTime += 33;
		            if(rfiuSub1VideoTime_frac>=10000)
		            {
		               rfiuSub1VideoTime_frac -=10000;
		               rfiuSub1VideoTime +=1;
		            }
		            rfiuSub1VideoTime_frac += 3333;
		          }
			   #elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
				  rfiuSub1VideoTime=rfiuMainAudioTime;
		       #endif

		          VideoDispOffset= (rfiuVideoBufFill_idx[1] >= rfiuVideoBufPlay_idx[1]) ?  (rfiuVideoBufFill_idx[1]-rfiuVideoBufPlay_idx[1]) : (rfiuVideoBufFill_idx[1]+DISPLAY_BUF_NUM-rfiuVideoBufPlay_idx[1]) ;
			      if(VideoDispOffset > 2)
			      {
	                 if(rfiuSub1VideoTime >= rfiuSub1VideoPresentTime[(rfiuVideoBufPlay_idx[1]+1) % DISPLAY_BUF_NUM])
					 {
					 	 rfiuVideoBufPlay_idx[1] +=1;
                     #if 1
                         if(rfiuSub1VideoTime >= rfiuSub1VideoPresentTime[(rfiuVideoBufPlay_idx[1]+1) % DISPLAY_BUF_NUM] +RFI_VIDEO_SYNC_SHIFT)
                         {
                            rfiuVideoBufPlay_idx[1] +=1;
                            #if RF_LETENCY_DEBUG_ENA
                            DEBUG_IDU(" DF2\n");
                            #endif
                         }
                     #endif
					 }
				  }
		          else if(VideoDispOffset > 1)
		          {
		             if(rfiuSub1VideoTime >= rfiuSub1VideoPresentTime[(rfiuVideoBufPlay_idx[1]+1) % DISPLAY_BUF_NUM])
		                  rfiuVideoBufPlay_idx[1] ++;
		          }
		          else
		          {
		             rfiuSub1VideoTime = rfiuSub1VideoPresentTime[rfiuVideoBufPlay_idx[1] % DISPLAY_BUF_NUM];
		             rfiuSub1VideoTime_frac=0;
		          }
		       }
	        #endif

            if( (gRfiu_Op_Sta[1] == RFIU_RX_STA_LINK_BROKEN) || (gRfiu_Op_Sta[1]==RFIU_OP_INIT) )
            {
                tv2FRAME_CTL = ( tv2FRAME_CTL & (~0x00003000) );
                tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[0];
            }
            else
            {
                tv2FRAME_CTL = ( tv2FRAME_CTL & (~0x00003000) );

                if(rfiuVideoBufPlay_idx[1] == 0)
                    tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[0];
                else
                    tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[(rfiuVideoBufPlay_idx[1]) % DISPLAY_BUF_NUM];
            }

        }
    #endif
        //----------playback mode-----------//
        else
        {
            tv2FRAME_CTL = ( tv2FRAME_CTL & (~0x00003000) );
            tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[0];
        }
  #endif

}

#if (LCM_OPTION == LCM_TMT035DNAFWU24_320x240)
#if 0	/* Unverified */
/*

Routine Description:

    The Read to TMT035 reg by SPI I/F.

Arguments:

	ucRegAddr - Reg addr to write to.
	ucData - Data to write to.

Return Value:

    None.

*/
void iduTMT035Read(u8 ucRegAddr, u8 *pucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;
	u8 ucGetData = 0;

	/*========Init for SPI I/F========*/
	Gpio3Dir &= ~0x700000;
	Gpio3Level |= 0x700000;
	Gpio3Ena |= 0x700000;

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

	for (i=0; i<100; i++);

	/* compose data stream */
	unTotalBitContToWrite = ((ucRegAddr<<10)| (*pucData));

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
	for (i=15; i>=0; i--)
	{
		if (i<=8)
		{
			Gpio3Dir |= 0x100000;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i>=8)
		{
			if (i==8)	/* floating SDA */
				for (j=0; j<14; j++);
			else
			{
				if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
				else
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
			}
		}
		else
		{
			gpioGetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, &ucGetData);
			*pucData = (*pucData |ucGetData);
			*pucData <<= 1;
		}
		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<14; j++);


	}

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
}
#endif
/*

Routine Description:

    The Write to TMT035 reg by SPI I/F.

Arguments:

	ucRegAddr - Reg addr to write to.
	ucData - Data to write to.

Return Value:

    None.

*/
void iduTMT035Write(u8 ucRegAddr, u8 ucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;

        #if ((HW_BOARD_OPTION == SKYBEST_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==VEISE_CARDVR))
	/*========Init for SPI I/F========*/
	Gpio3Dir &= ~0x700000;
	Gpio3Level |= 0x700000;
	Gpio3Ena |= 0x700000;
        #elif(HW_BOARD_OPTION == WENSHING_SDV)
	Gpio3Dir &= ~0x1C0;
	Gpio3Level |= 0x1C0;
	Gpio3Ena |= 0x1C0;
        #endif

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

	/* compose data stream */
	unTotalBitContToWrite = ((((ucRegAddr<<2)|0x3)<<8)| ucData);

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
	for (i=15; i>=0; i--)
	{
		if (i==8)
		{
		    #if ((HW_BOARD_OPTION == SKYBEST_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==VEISE_CARDVR))
			Gpio3Dir |= 0x100000;	/* set the 8th bit be input */
		    #elif(HW_BOARD_OPTION == WENSHING_SDV)
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		    #endif
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i==8)	/* floating SDA */
			for (j=0; j<14; j++);
		else
		{
			if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
			else
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
		}

		for (j=0; j<14; j++);

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<28; j++);

		if (i==8)
		{
		    #if ((HW_BOARD_OPTION == SKYBEST_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==VEISE_CARDVR))
			Gpio3Dir &= ~0x100000;	/* set the 8th bit be output */
		    #elif(HW_BOARD_OPTION == WENSHING_SDV)
		        Gpio3Dir &= ~0x80;	/* set the 8th bit be output */
		    #endif
		}

	}

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

}
#endif
#if (LCM_OPTION == LCM_TJ015NC02AA)
void iduTJ015NC02AARead(u8 ucRegAddr, u8 *pucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;
	u8 ucGetData = 0;

	/*========Init for SPI I/F========*/
	//Gpio3Dir &= ~0x700000;
	//Gpio3Level |= 0x700000;
	//Gpio3Ena |= 0x700000;

	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	for (i=0; i<100; i++);

	/* compose data stream */
	unTotalBitContToWrite = ((((ucRegAddr<<2)|0x03)<<8)| (*pucData));

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
	for (i=15; i>=0; i--)
	{
		if (i<=8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i>=8)
		{
			if (i==8)	/* floating SDA */
				for (j=0; j<14; j++);
			else
			{
				if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
				else
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
			}
		}
		else
		{
			gpioGetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, &ucGetData);
			*pucData = (*pucData |ucGetData);
			*pucData <<= 1;
		}
		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<14; j++);


	}

	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
}

void iduTJ015NC02AAWrite(u8 ucRegAddr, u8 ucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;

	/*========Init for SPI I/F========*/

	/* compose data stream */
	unTotalBitContToWrite = ((((u32)ucRegAddr)<<10) | ((u32)ucData));
    //DEBUG_IDU("unTotalBitContToWrite = %#x\n", unTotalBitContToWrite);
	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	for (i=15; i>=0; i--)
	{
		if (i==8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i==8)	/* floating SDA */
			for (j=0; j<14; j++);
		else
		{
			if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
			else
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
		}

		for (j=0; j<14; j++);

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<28; j++);

		if (i==8)
		{
			Gpio3Dir &= ~0x80;	/* set the 8th bit be output */
		}

	}

	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

}
#endif
#if (LCM_OPTION == LCM_HX8817_RGB)
/*

Routine Description:

    The write to HX8817 reg by SPI I/F.

Arguments:

	ucRegAddr - Reg addr to write to.
	ucData - Data to write to.

Return Value:

    None.

*/
void iduHx8817Write(u8 ucRegAddr, u8 ucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;

	/*========Init for SPI I/F========*/
	Gpio3Ena |= 0x1C0;
	Gpio3Dir &= ~0x1C0;
	Gpio3Level |= 0x0C0;

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

	unTotalBitContToWrite = ((ucRegAddr & 0x1F) << 8)| ucData;
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	for (i=13; i>=0; i--)
	{
		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
		if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
			gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
		else
			gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<14; j++);

	}

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	for (j=0; j<14; j++);
	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	for (j=0; j<14; j++);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

}
#endif

#if (LCM_OPTION == LCM_TD036THEA3_320x240)

void iduTD036THEA3Read(u8 ucRegAddr, u8 *pucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;
	u8 ucGetData = 0;

	/*========Init for SPI I/F========*/
	//Gpio3Dir &= ~0x700000;
	//Gpio3Level |= 0x700000;
	//Gpio3Ena |= 0x700000;

	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	for (i=0; i<100; i++);

	/* compose data stream */
	unTotalBitContToWrite = ((((ucRegAddr<<2)|0x03)<<8)| (*pucData));

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
	for (i=15; i>=0; i--)
	{
		if (i<=8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i>=8)
		{
			if (i==8)	/* floating SDA */
				for (j=0; j<14; j++);
			else
			{
				if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
				else
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
			}
		}
		else
		{
			gpioGetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, &ucGetData);
			*pucData = (*pucData |ucGetData);
			*pucData <<= 1;
		}
		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<14; j++);


	}

	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
}

void iduTD036THEA3Write(u8 ucRegAddr, u8 ucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;

	/*========Init for SPI I/F========*/

	/* compose data stream */
	unTotalBitContToWrite = ((((u32)ucRegAddr)<<10) | ((u32)ucData));
    //DEBUG_IDU("unTotalBitContToWrite = %#x\n", unTotalBitContToWrite);
	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	for (i=15; i>=0; i--)
	{
		if (i==8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i==8)	/* floating SDA */
			for (j=0; j<14; j++);
		else
		{
			if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
			else
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
		}

		for (j=0; j<14; j++);

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<28; j++);

		if (i==8)
		{
			Gpio3Dir &= ~0x80;	/* set the 8th bit be output */
		}

	}

	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

}
#endif

#if ((LCM_OPTION == LCM_TD024THEB2)||(LCM_OPTION == LCM_TD024THEB2_SRGB))

void iduTD024THEB2Read(u8 ucRegAddr, u8 *pucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;
	u8 ucGetData = 0;

	/*========Init for SPI I/F========*/
	//Gpio3Dir &= ~0x700000;
	//Gpio3Level |= 0x700000;
	//Gpio3Ena |= 0x700000;

	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

    gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	for (i=0; i<100; i++);

	/* compose data stream */
	unTotalBitContToWrite = ((((ucRegAddr<<2)|0x03)<<8)| (*pucData));

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
	for (i=15; i>=0; i--)
	{
		if (i<=8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i>=8)
		{
			if (i==8)	/* floating SDA */
				for (j=0; j<14; j++);
			else
			{
				if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
				else
					gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
			}
		}
		else
		{
			gpioGetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, &ucGetData);
			*pucData = (*pucData |ucGetData);
			*pucData <<= 1;
		}
		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<14; j++);


	}

	//gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
}

void iduTD024THEB2Write(u8 ucRegAddr, u8 ucData)
{
	u32	j;
	s8	i;
	u32	unBitLevel = 1;
	u32	unTotalBitContToWrite;
    /* 3-wires serial port interface */
	/*========Init for SPI I/F========*/

	/* compose data stream */
	unTotalBitContToWrite = ((((u32)ucRegAddr)<<10) | ((u32)ucData));
    //DEBUG_IDU("unTotalBitContToWrite = %#x\n", unTotalBitContToWrite);
	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);

	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_LOW);
    //gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	//gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
	//gpio 3-8 ??
	for (i=15; i>=0; i--)
	{
		if (i==8)
		{
			Gpio3Dir |= 0x80;	/* set the 8th bit be input */
		}

		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);

		if (i==8)	/* floating SDA */
			for (j=0; j<14; j++);
		else
		{
			if ((((unBitLevel << i) & unTotalBitContToWrite) >> i) == 1)
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);
			else
				gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
		}

		for (j=0; j<14; j++);
    		gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
		for (j=0; j<28; j++);

		if (i==8)
		{
			Gpio3Dir &= ~0x80;	/* set the 8th bit be output */
		}

	}

	///gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_HIGH);
	///gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_HIGH);

	gpioSetLevel(GROUP_LCD_SCLK, PIN_LCD_SCLK, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_SDA, PIN_LCD_SDA, LEVEL_LOW);
	gpioSetLevel(GROUP_LCD_CS, PIN_LCD_CS, LEVEL_HIGH);

}
#endif


/*

Routine Description:

    The test routine of Image Display Unit.

Arguments:

    None.

Return Value:

    None.

*/


#if (LCM_OPTION == LCM_HX5073_RGB)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;


        SYS_CLK1 = SYS_CLK1 | 0x00000400;
        SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000804;
        IduEna  =   IduEna | IDU_DCLK_EN;
        IduWinCtrl =    IDU_CTL_VDO_ENA;
        IduDispCfg =IDU_RGBTYPE_3|
                        IDU_DB_W_8|
                        IDU_INTF_RGB_CF;
        IduDispCfg  =   IduDispCfg | 0x01000000;

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        iduTypeConfig();


}


#elif (LCM_OPTION == LCM_HX5073_YUV)

void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
        SYS_CLK1 = SYS_CLK1 | 0x00000300;
        SYS_CPU_PLLCTL = SYS_CPU_PLLCTL | 0x00000804;
        IduEna  =   IduEna | IDU_DCLK_EN;
        IduWinCtrl =    IDU_CTL_VDO_ENA;
        IduDispCfg =IDU_RGBTYPE_3|
                    IDU_DB_W_8|IDU_INTF_CCIR601;

        IduDispCfg  =   IduDispCfg | 0x01000000;

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        iduTypeConfig();

}

#elif (LCM_OPTION ==LCM_A015AN04)

void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if (IDU_CLK_FREQ == 24000000)
    Clk_Div = 3;
#elif (IDU_CLK_FREQ == 32000000)
    Clk_Div = 7;
#elif (IDU_CLK_FREQ == 48000000)
    Clk_Div = 7;
#elif (IDU_CLK_FREQ == 54000000)
    Clk_Div = 7;
#elif (IDU_CLK_FREQ == 96000000)
    Clk_Div = 7;
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;
    IduDispCfg =    ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3|
                IDU_DB_W_8|
                IDU_CFT_1|
                IDU_DEN_POL_1|
                IDU_INTF_RGB_CF;

    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
}
#elif((LCM_OPTION == LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02))

void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

    #if (IDU_CLK_FREQ == 24000000)
        Clk_Div = 2;
    #elif (IDU_CLK_FREQ == 32000000)
        Clk_Div = 5;
    #elif (IDU_CLK_FREQ == 48000000)
        Clk_Div = 5;
	#elif (IDU_CLK_FREQ == 54000000)
        Clk_Div = 5;
    #elif (IDU_CLK_FREQ == 96000000)
        Clk_Div = 8;
    #endif

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;
    // =0x07900231
    IduDispCfg =    ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3|
                IDU_DB_W_8|
                IDU_INTF_RGB_CF;

    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);

    iduTypeConfig();
   //----------------Set IDU gamma Value---------------//
   /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//
}


//
// version ==>  No Used
// rotation ==> No Used


#elif (LCM_OPTION == LCM_HX8224)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 2;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 3; //  3
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 5;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 5;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 11;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                     IDU_RGBTYPE_3|
                     IDU_DB_W_8|
                     IDU_INTF_RGB_CF;
        IduEna  =   IDU_DCLK_EN;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}
#elif (LCM_OPTION == LCM_HX8224_SRGB)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;

        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 2;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 3;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 5;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 5;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 11;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =    ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3|
                IDU_DB_W_8|
                IDU_INTF_RGB_CFII;

        IduEna  =   IDU_DCLK_EN;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}
#elif ( (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) )
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 1;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 3;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 3;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 5;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                    IDU_RGBTYPE_3|
                    IDU_DB_W_8|
                    IDU_YUV_SWAP_1 |
                    //IDU_HS_POL_HIGH |
                    //IDU_VS_POL_HIGH |
                    IDU_INTF_CCIR601;
        IduEna =IDU_DCLK_EN;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();




}
#elif (LCM_OPTION == LCM_CCIR601_640x480P)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 0;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 3;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                    IDU_RGBTYPE_3|
                    IDU_DB_W_8|
                    IDU_YUV_SWAP_1 |
                    //IDU_HS_POL_HIGH |
                    //IDU_VS_POL_HIGH |
                    IDU_INTF_CCIR601;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}

#elif (LCM_OPTION == LCM_TPG105)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 2;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 3;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 5;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 5;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 5;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg  =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                       IDU_RGBTYPE_3|
                       IDU_DB_W_8|
                       IDU_CFT_5|
                       IDU_INTF_RGB_CF;
        IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;//lisa_IDU
        IduWinCtrl |=IDU_CTL_VDO_ENA;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}
#elif (LCM_OPTION == LCM_TD020THEG1)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 0;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 0;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                    IDU_RGBTYPE_3|
                    IDU_DB_W_8|
                    IDU_CFT_5|
                    0x03000000|
                    IDU_INTF_RGB_CF;
        IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;//lisa_IDU
        IduWinCtrl |=IDU_CTL_VDO_ENA;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}
#elif(LCM_OPTION == LCM_TD036THEA3_320x240)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 0;
		#elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 0;
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 0;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                     IDU_RGBTYPE_3|
                     IDU_DB_W_8|
                     IDU_INTF_RGB;

        IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;//lisa_IDU
        IduWinCtrl  |=IDU_CTL_VDO_ENA;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();

        //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//

}
#elif (LCM_OPTION == LCM_TD024THEB2)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 1;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 1;   //192/9/2=10.66MHz  (Spec 10.36MHz)
        #elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 1;   //192/9/2=10.66MHz  (Spec 10.36MHz)
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 0;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =    ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                        IDU_RGBTYPE_3|
                        IDU_DB_W_8|
                        IDU_CFT_5|
                        //IDU_D_POL_RE|
                        IDU_DEN_POL_1|
                        //0x03000000|
                        IDU_INTF_RGB_CF;
        IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;
        IduWinCtrl |=IDU_CTL_VDO_ENA;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}
#elif (LCM_OPTION == LCM_TD024THEB2_SRGB)
void IDU_Init(u8 version , u8 rotation)
{
        u8 Clk_Div;
        #if (IDU_CLK_FREQ == 24000000)
                    Clk_Div = 0;
        #elif(IDU_CLK_FREQ == 32000000)
                    Clk_Div = 1;
        #elif (IDU_CLK_FREQ == 48000000)
                    Clk_Div = 1;   //192/9/2=10.66MHz  (Spec 10.36MHz)
        #elif (IDU_CLK_FREQ == 54000000)
                    Clk_Div = 1;   //192/9/2=10.66MHz  (Spec 10.36MHz)
        #elif (IDU_CLK_FREQ == 96000000)
                    Clk_Div = 0;
        #endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

        IduWinCtrl |=IDU_CTL_VDO_ENA;
        IduDispCfg =    ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                        IDU_RGBTYPE_3|
                        IDU_DB_W_8|
                        IDU_CFT_5|
                        //IDU_D_POL_RE|
                        IDU_DEN_POL_1|
                        //0x03000000|
                        IDU_INTF_RGB_CFII;
        IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;
        IduWinCtrl |=IDU_CTL_VDO_ENA;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
        iduTypeConfig();
}

#elif (LCM_OPTION == LCM_HX8817_RGB)


void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;


#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif


    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3   |
                IDU_DB_W_8      |
                IDU_DEN_POL_1   |
                IDU_INTF_RGB;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);

    iduTypeConfig();



}
#elif (LCM_OPTION == LCM_HX8257_RGB666_480x272)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_1   |
                IDU_DB_W_18     |
                IDU_DEN_POL_1   |
                IDU_D_POL_RE    |  //rising edge latch data
                IDU_INTF_RGB;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
    //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//

}

#elif (LCM_OPTION == LCM_HX8257_SRGB_480x272)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

  #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1)  )  //for 128pin QFP
	GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN);
  #endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3   |
                IDU_DB_W_8      |
                IDU_DEN_POL_1   |
                IDU_D_POL_RE    |  //rising edge latch data
                IDU_INTF_RGB;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
    //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//

}

#elif (LCM_OPTION == LCM_HX8257_P_RGB_480x272)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

  #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) )  //for 128pin QFP
	GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN);
  #endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3   |
                IDU_DB_W_24     |
                IDU_DEN_POL_1   |
                IDU_D_POL_RE    |  //rising edge latch data
                IDU_INTF_RGB;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
    //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//

}

#elif (LCM_OPTION == LCM_sRGB_HD15_HDMI)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;    //Dclk=pclk/3=24/3=8MHz, 52.9 fps.
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

 #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1)  )////for 128pin QFP
	GpioActFlashSelect |= (GPIO_DISP_FrDV1_EN);
 #endif
    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                IDU_RGBTYPE_3   |
                IDU_DB_W_8      |
                IDU_DEN_POL_1   |
                IDU_D_POL_RE    |  //rising edge latch data
                IDU_INTF_RGB;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
    //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//

}

#elif ((LCM_OPTION == LCM_TMT035DNAFWU24_320x240)||(LCM_OPTION == LCM_LQ035NC111))

void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
	u8 data;


#if (IDU_CLK_FREQ == 24000000)
            Clk_Div = 0;
#elif(IDU_CLK_FREQ == 32000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 48000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 54000000)
            Clk_Div = 0;
#elif (IDU_CLK_FREQ == 96000000)
            Clk_Div = 3;
#endif

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg =((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
				IDU_INTF_RGB	|
                IDU_RGBTYPE_3   |
                IDU_DB_W_8      |
                IDU_DEN_POL_1;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();
     //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//
}
#elif(LCM_OPTION == LCM_TJ015NC02AA)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
	u8 data;
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    Clk_Div = 2;
    IduWinCtrl |=IDU_CTL_VDO_ENA;

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                         IDU_RGBTYPE_3|
                         IDU_DB_W_8|
                         IDU_CFT_5|
                         IDU_INTF_RGB_CF;
    IduWinCtrl  &= ~ IDU_CTL_OSD2_ENA;//lisa_IDU
    IduWinCtrl  |=IDU_CTL_VDO_ENA;

    IduEna    &=    (~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();

}

#elif((LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))
void IDU_Init(u8 version , u8 rotation)
{
	u8 Clk_Div;

	#if (IDU_CLK_FREQ == 24000000)
		Clk_Div = 0;
    #elif(IDU_CLK_FREQ == 32000000)
        Clk_Div = 0;
	#elif (IDU_CLK_FREQ == 48000000)
		Clk_Div = 0;
	#elif (IDU_CLK_FREQ == 54000000)
		Clk_Div = 0;
    #elif (IDU_CLK_FREQ == 96000000)
		Clk_Div = 3;
    #else
		Clk_Div = 23;
	#endif

	IduWinCtrl |= IDU_CTL_VDO_ENA; // Enable Video Window Enable

	IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
		     IDU_INTF_80|
		     IDU_DB_W_8|
		     IDU_RGBTYPE_3|
		     //IDU_RGBTYPE_2|
		     IDU_DEN_POL_1;

	IduEna    &=    ( ~IDU_ROT_DEN);
	IduEna    &=    (~IDU_DAC_PWN_ON);
	IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();

}

#elif((LCM_OPTION ==LCM_P_RGB_888_HannStar) || (LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227)||(LCM_OPTION == LCM_P_RGB_888_ILI6122)||(LCM_OPTION == LCM_P_RGB_888_ILI6126C))
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

#if ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=2;
#else
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#endif

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
		             IDU_D_POL_RE |
                     IDU_DEN_POL_1 |
                     IDU_RGBTYPE_3 |
                     IDU_DB_W_24|
                     IDU_INTF_RGB;

    iduTypeConfig();
    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    //IduEna  =  IDU_ENA | IDU_DATA_EN | IDU_DCLK_EN;
#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN)
    IduL1L2FifoThresh = 0x00055303;
#if((TV_DECODER == TW9900)||(TV_DECODER == TW9910))
    IduGammaY0 = 0x2A040000;
    IduGammaY1 = 0xFFE4804B;
	IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
#else if(TV_DECODER == WT8861)
    IduGammaY0 = 0x804B2A14;
    IduGammaY1 = 0xffEBD6B7;
    IduGammaX1 = 0x1C181410;
    IduGammaX0 = 0x0C080401;
#endif
#endif

#if ((HW_BOARD_OPTION == MR8200_RX_COMMAX) || (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX) )

    IduGammaY0 = 0x2A040000;
    IduGammaY1 = 0xFFE4804B;
    IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
#elif (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) 
    IduGammaY0 = 0x3a251404;
    IduGammaY1 = 0xFFE4804B;
    IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
#elif (HW_BOARD_OPTION == MR8200_RX_RDI_M920)
    IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
    IduGammaY0 = 0x3A190800;
    IduGammaY1 = 0xFFAC7C5B;
#elif ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592) ||\
    (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) ||\
    (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
    IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
    IduGammaY0 = 0x5A280800;
    IduGammaY1 = 0xFFDCAA82;
#elif (HW_BOARD_OPTION == MR9670_HECHI)
    IduGammaY0 = 0x2A040000;
    IduGammaY1 = 0xFFE4804B;
	IduGammaX0 = 0x04020101;
    IduGammaX1 = 0x18100c08;
#endif
}

#elif(LCM_OPTION ==LCM_P_RGB_888_Innolux)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#else
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#endif

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                 IDU_D_POL_RE | //yugo modify
                 IDU_DEN_POL_1 |
                 IDU_RGBTYPE_3 |
                 IDU_DB_W_24 |
                 IDU_INTF_RGB;

    iduTypeConfig();
    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    //IduEna  =  IDU_ENA | IDU_DATA_EN | IDU_DCLK_EN;
}
#elif(LCM_OPTION ==LCM_P_RGB_888_AT070TN90)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=2;
#else
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#endif

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                 IDU_D_POL_RE | //yugo modify
                 IDU_DEN_POL_1 |
                 IDU_RGBTYPE_2 |
                 IDU_DB_W_24 |
                 IDU_INTF_RGB;

    iduTypeConfig();
    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    //IduEna  =  IDU_ENA | IDU_DATA_EN | IDU_DCLK_EN;
}
#elif(LCM_OPTION == LCM_P_RGB_888_SY700BE104)
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=2;
#else
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#endif

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                 IDU_D_POL_FE |
                 IDU_DEN_POL_1 |
                 IDU_RGBTYPE_2 |
                 IDU_DB_W_24 |
                 IDU_INTF_RGB;

    iduTypeConfig();
    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    //IduEna  =  IDU_ENA | IDU_DATA_EN | IDU_DCLK_EN;
}
#elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
    int i;

    SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#if(LCM_OPTION == VGA_640X480_60HZ)
    SYS_DDR_PLLCTL=0x6404c001;                            //24M* 100/4=600
    SYS_CLK1= (SYS_CLK1 & (~0x0000ff00)) | 0x00001700;    //600/24=25MHz
#elif(LCM_OPTION == VGA_800X600_60HZ)
    SYS_DDR_PLLCTL=0x4103c001;                            //24M* 65/3=520
    SYS_CLK1= (SYS_CLK1 & (~0x0000ff00)) | 0x00000c00;    //520/13=40
#elif(LCM_OPTION == VGA_1024X768_60HZ)
    SYS_DDR_PLLCTL=0x4103c001;                            //24M* 65/3=520
    SYS_CLK1= (SYS_CLK1 & (~0x0000ff00)) | 0x00000700;    //520/8=65
#elif(LCM_OPTION == VGA_1280X800_60HZ)
    SYS_DDR_PLLCTL=0x5304c001;                            //24M* 83/4=498
    SYS_CLK1= (SYS_CLK1 & (~0x0000ff00)) | 0x00000500;    //498/6=83
#endif
    for(i=0 ; i<200 ; i++);
    SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL

    SYS_ANA_TEST2 = (SYS_ANA_TEST2 & (~0x018)) | 0x010;  //switch to VGA mode of DAC

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#else
    IduWinCtrl = IDU_CTL_VDO_ENA | IDU_CTL_HISH_SPEED_EN;
    Clk_Div=0;
#endif

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                 IDU_D_POL_RE | //yugo modify
                 IDU_DEN_POL_1 |
                 IDU_RGBTYPE_3 |
                 IDU_DB_W_24 |
                 IDU_INTF_RGB;

    iduTypeConfig();
    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    IduDefBgColor   = 0x00808000;

    //IduEna  =  IDU_ENA | IDU_DATA_EN | IDU_DCLK_EN;
}

#elif(LCM_OPTION ==LCM_HX8312) //For LCM8312
//
// version ==>  0 :old  1 : new
// rotation ==> 0 :no   1 : rotation 90
void IDU_Init(u8 version , u8 rotation)
{
    u8 Clk_Div;
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from Sysclk
#endif

    #if (IDU_CLK_FREQ == 24000000)
        Clk_Div = 1;
    #elif(IDU_CLK_FREQ == 32000000)
        Clk_Div = 2;
    #elif (IDU_CLK_FREQ == 48000000)
        Clk_Div = 3;
	#elif (IDU_CLK_FREQ == 54000000)
        Clk_Div = 3;
    #elif (IDU_CLK_FREQ == 96000000)
        Clk_Div = 3;
    #else
        Clk_Div = 3;
    #endif

    IduWinCtrl |= IDU_CTL_VDO_ENA; // Enable Video Window Enable

    IduDispCfg = ((Clk_Div<<IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
             IDU_DEN_POL_1|
             IDU_INTF_80; // Div = 7 (real div = 7+1)


    IduEna    &=    ( ~IDU_ROT_DEN);
    IduEna    &=    (~IDU_DAC_PWN_ON);
    IduEna    &=    (~IDU_TV_MODE_ENA);
    iduTypeConfig();

    //----------------Set IDU gamma Value---------------//
     /*-------- Lucian: IDU YCbCr2RGB color matrix Reassign: -------------
      s2.5 format
      Standard:
       [ 1        0   1.402
         1   -0.344  -0.714
         1    1.722       0
       ]

      New:

      1 -0.19   1.87
      1 -0.67   -1.09
      1  2.29   -0.09

      more saturation:
      1 -0.262   2.047
      1 -0.795  -1.233
      1  2.485  -0.126

      more blue
      0.98  -0.257  2.0
      1 -0.795  -1.233
      1  2.485  -0.126

    */

    //IduYCbCr2R=0x003c8620;
    //IduYCbCr2G=0x00a39520;
    //IduYCbCr2B=0x00834920;

    IduYCbCr2R=0x0040881f;
    IduYCbCr2G=0x00a79920;
    IduYCbCr2B=0x00845020;

    //-------IDU Gamma Reassign----------//
    /*
        X= 0, 8, 16, 32, 64, 96, 128, 192, 255
        Y= 0, 0,  8, 25, 58, 91, 124, 190, 255
    */
    IduGammaY0 = 0x3a190800;
    IduGammaY1 = 0xffbe7c5b;
    IduGammaX1 = 0x18100c08;
    IduGammaX0 = 0x04020101;
    //-----------------------------------//


}
#endif



void iduDelay(u32 counter)
{
    u32 i;
    for(i=0 ; i<counter ; i++)
    {
        i= i;
    }
}

void iduWaitCmdBusy(void)
{
    while(((IduMpuCmdCfg & IDU_CMD_BUSY_MASK)) != 0)
    {
        iduDelay(10);
    }
    return;

}

#if IS_COMMAX_DOORPHONE

s32 iduPreview422(int src_W,int src_H)
{
    u32* temp_00;
    u32 temp;
    u32 i,j;
	static u8 just_run_one=1;


      IDU_Init(0 , 1);
      IduVidWinStart = 0x00000000;
      IduVidWinEnd = ((PANNEL_X-1)<<IDU_VDO_WSX_SHFT )|((PANNEL_Y-1)<<IDU_VDO_WSY_SHFT);

   #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB)) //RGB data
      IduVidBufStride = PANNEL_X >> 2;
   #else
     IduVidBufStride = PANNEL_X>>1;
   #endif


    temp        =   IduWinCtrl;
    temp       |=   IDU_CTL_VDO_ENA|
             #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IDU_CTL_SRC_FMT_SRGB |
             #endif
                    IDU_CTL_TRIPLE_ENA|     // ? It relate to ISU triple-setting ?
                    IDU_CTL_IDU_WAIT_DISA;  //Enable, IDU will pending while input FIFO empty.

    temp       &= (~IDU_CTL_FBAUTO_ENA);    //Lucian: Preview mode is changed to use Manual mode.
    temp       &= (~0x00003000);          //Lucian: FB_sel=0
    temp       |= IDU_CTL_FB_SEL_2;           //Lucian: FB_sel=2
    IduWinCtrl  = temp;

    //if(sysPIPMain == PIP_MAIN_NONE)
        IduIntCtrl =IDU_OURINT_DISA|
                    IDU_VURINT_DISA|
                    IDU_FTCINT_DISA;
    //else
    //    IduIntCtrl =IDU_OURINT_DISA|
    //                IDU_VURINT_DISA|
    //                IDU_FTCINT_ENA;

    #if(LCM_OPTION == LCM_TG200Q04)
	  IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    #else
      IduMpuCmdCfg = IDU_CMD_W_16;
    #endif
      IduCrcCtrl = 0x00000000;

      #if NEW_IDU_BRI
         IduFifoThresh = 0xf00a8304;
      #else
         IduFifoThresh = 0xc0835304;
      #endif

      IduVidBuf0Addr= (u32)PNBuf_sub1[0];
      IduVidBuf1Addr= (u32)PKBuf1;
      IduVidBuf2Addr= (u32)PKBuf2;

          #if NEW_IDU_BRI
             BRI_OUT_SIZE =(PANNEL_Y<<16) | PANNEL_X;

             BRI_IADDR_Y = (unsigned int )PNBuf_sub1[0];
             BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;

            #if NEW_IDU_BRI_PANEL_INTLX
             if(src_H > 640) //HD
             {
             #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
             #else
                BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
             #endif
                BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
             }
             else
             {
             #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
                BRI_IN_SIZE =( (src_H/2/2)<<16 ) | src_W;
             #else
                BRI_IN_SIZE =( (src_H/2)<<16 ) | src_W;
             #endif
                BRI_STRIDE = src_W*2;
             }
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
            #else
             if(src_H > 640) //HD
             {
             #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                BRI_IN_SIZE =( (src_H/2)<<16 ) | 1024;
             #else
                BRI_IN_SIZE =( (src_H/2)<<16 ) | src_W;
             #endif
                BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
             }
             else
             {
             #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
               BRI_IN_SIZE =((src_H/4)<<16) | src_W;
             #else
                BRI_IN_SIZE =(src_H<<16) | src_W;
             #endif
                BRI_STRIDE = src_W*2;
             }
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
            #endif
          #endif

          #if (LCM_OPTION == LCM_TM024HDH29)
          #else
            IduEna = IDU_ENA | IDU_DATA_EN;
          #endif

  return 1;
}

#endif  // #if IS_COMMAX_DOORPHONE


/*

Routine Description:

    Preview.

Arguments:

    zoomFactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iduPreview(int src_W,int src_H)
{
    u32* temp_00;
    u32 temp;
    u32 i,j;
	static u8 just_run_one=1;


  if(sysTVOutOnFlag) //TV-out
  {
  #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
          TvOutMode=SYS_TV_OUT_HD720P;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
          TvOutMode=SYS_TV_OUT_FHD1080I;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
  #else
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
  #endif
         #if ((HW_BOARD_OPTION==ROULE_DOORPHONE)||(HW_BOARD_OPTION == ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N))
          if(uiGetMenuMode() == STANDBY_MODE)
          {
             IduEna &= ~IDU_ENA;
             IduVidBuf0Addr= (u32)pucClockImageBuf;
             IduVidBuf1Addr= (u32)pucClockImageBuf;
             IduVidBuf2Addr= (u32)pucClockImageBuf;
          }
          else
          {
             IduVidBuf0Addr= (u32)PKBuf0;
             IduVidBuf1Addr= (u32)PKBuf1;
             IduVidBuf2Addr= (u32)PKBuf2;
          }
        #else
          IduVidBuf0Addr= (u32)PKBuf0;
          IduVidBuf1Addr= (u32)PKBuf1;
          IduVidBuf2Addr= (u32)PKBuf2;
    	#endif

          temp         = tvFRAME_CTL;
          temp         = TV_FRMCTL_VDO_EN    |
                        //TV_FRMCTL_OSD0_EN |
                        //TV_FRMCTL_OSD1_EN |
                        //TV_FRMCTL_OSD2_EN |
                        //TV_FRMCTL_FB_AUTO |
                        TV_FRMCTL_TRIPLE;
                        //TV_FRMCTL_FB_SEL0;
          temp        &= (~0x00003000);
          temp        |= TV_FRMCTL_FB_SEL2;
          tvFRAME_CTL  = temp;

      #if TV_DISP_BY_IDU
          #if(TV_DISP_BY_TV_INTR)
            tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
          #else
            tvTVE_INTC   =TV_INTC_FRMEND__ENA;    //TV interrupt control *
          #endif
      #else
          tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
      #endif

      #if(CHIP_OPTION == CHIP_A1013A)
          tvFIFO_TH = 0x00002103;
      #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
          tvFIFO_TH = 0x40008401;
      #elif( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
          tvFIFO_TH = 0x40008501;
      #else
          tvFIFO_TH = 0x00002104;
      #endif

#if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
     tvTVFB_STRIDE= 1280/2;
#elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
   #if SIMU1080I_VIA720P
     tvTVFB_STRIDE= 1280/2;
   #else
     tvTVFB_STRIDE= 1920/2;
   #endif
#else
     #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
         tvTVFB_STRIDE= 704/2;
     #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
         tvTVFB_STRIDE= 640/2;
     #else
         tvTVFB_STRIDE= 640/2;
     #endif
#endif

          IduVideo_ClearBuf();
  #if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A)) )
     #if NEW_IDU_BRI
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
              if(src_H > 640)
              {
              #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                 BRI_IN_SIZE =( (src_H/2/2)<<16 ) | 1024;
              #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                 BRI_IN_SIZE =((src_H/2)<<16) | src_W;
              #else
                 BRI_IN_SIZE =( (src_H/2/2)<<16 ) | src_W;
              #endif
              }
              else
                 BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                  BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }
           else
           {
              if(src_H > 640)
              {
              #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                 BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
              #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                 BRI_IN_SIZE =((src_H/2)<<16) | src_W;
              #else
                 BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
              #endif
              }
              else
                 BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
             BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
           #else
             BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2); //跳行抓,省頻寬
           #endif
           }
           else
             BRI_STRIDE = src_W;

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =(src_H<<16) | 1024;
           #else
              BRI_IN_SIZE =(src_H<<16) | src_W;
           #endif
           }
           else
              BRI_IN_SIZE =(src_H<<16) | src_W;

           BRI_OUT_SIZE =(720<<16) | 1280;

           if(src_H > 640)
           {
              BRI_STRIDE = BRI_BFA_DIV1 | src_W;
           }
           else
           {
              BRI_STRIDE = src_W;
           }

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =( (src_H/2)<<16) | 1024;
           #else
              BRI_IN_SIZE =( (src_H/2)<<16) | src_W;
           #endif
           }
           else
              BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif

           if(src_H > 640)
           {
              BRI_STRIDE = BRI_BFA_DIV1 | src_W;
           }
           else
           {
              BRI_STRIDE = src_W;
           }

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
         #endif

     #else
         //Lucian: For A1013A only
     #endif
  #else    //for YUV422 output
       #if NEW_IDU_BRI
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
                #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
                #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
                #endif
                }
                else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

               #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                  #if TV_D1_OUT_FULL_HALF
                      BRI_OUT_SIZE =(240<<16) | 704;
                  #else
                  BRI_OUT_SIZE =(288<<16) | 704;
                  #endif
               #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                  BRI_OUT_SIZE =(288<<16) | 640;
               #else
                  BRI_OUT_SIZE =(240<<16) | 640;
               #endif
           }
           else
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           #else
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
           #endif
           }
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)

           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =(src_H<<16) | 1024;
           #else
              BRI_IN_SIZE =(src_H<<16) | src_W;
           #endif
           }
           else
              BRI_IN_SIZE =(src_H<<16) | src_W;

           BRI_OUT_SIZE =(720<<16) | 1280;


           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)

           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =((src_H/2)<<16) | 1024;
           #else
              BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           #endif
           }
           else
              BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif


           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
         #endif

       #endif
       tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
  #endif
  }
  else //Pannel-out
  {
      IDU_Init(0 , 1);
      IduVidWinStart = 0x00000000;
      IduVidWinEnd = ((PANNEL_X-1)<<IDU_VDO_WSX_SHFT )|((PANNEL_Y-1)<<IDU_VDO_WSY_SHFT);

   #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB)) //RGB data
      IduVidBufStride = PANNEL_X >> 2;
   #else
      #if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1013A) || \
        (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A)) )  //YUV420
         IduVidBufStride = PANNEL_X>>1; //Lucian:  the same as yuv422
      #else //YUV422
         IduVidBufStride = PANNEL_X>>1;
      #endif
   #endif


    temp        =   IduWinCtrl;
    temp       |=   IDU_CTL_VDO_ENA|
             #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IDU_CTL_SRC_FMT_SRGB |
             #endif
                    IDU_CTL_TRIPLE_ENA|     // ? It relate to ISU triple-setting ?
                    IDU_CTL_IDU_WAIT_DISA;  //Enable, IDU will pending while input FIFO empty.

    temp       &= (~IDU_CTL_FBAUTO_ENA);    //Lucian: Preview mode is changed to use Manual mode.
    temp       &= (~0x00003000);          //Lucian: FB_sel=0
    temp       |= IDU_CTL_FB_SEL_2;           //Lucian: FB_sel=2
    IduWinCtrl  = temp;

    //if(sysPIPMain == PIP_MAIN_NONE)
        IduIntCtrl =IDU_OURINT_DISA|
                    IDU_VURINT_DISA|
                    IDU_FTCINT_DISA;
    //else
    //    IduIntCtrl =IDU_OURINT_DISA|
    //                IDU_VURINT_DISA|
    //                IDU_FTCINT_ENA;

    #if(LCM_OPTION == LCM_TG200Q04)
	  IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    #else
      IduMpuCmdCfg = IDU_CMD_W_16;
    #endif
      IduCrcCtrl = 0x00000000;

    #if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
        (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A)) )
      #if NEW_IDU_BRI
         IduFifoThresh = 0xf00a8304;
      #else
         IduFifoThresh = 0xc0815304;
      #endif
    #else
      #if NEW_IDU_BRI
         IduFifoThresh = 0xf00a8304;
      #else
         IduFifoThresh = 0xc0835304;
      #endif
    #endif

    #if ((HW_BOARD_OPTION==ROULE_DOORPHONE)||(HW_BOARD_OPTION == ROULE_SD8F)|| (HW_BOARD_OPTION == ROULE_SD7N))
      {   /* set the init screen is black */
        if(uiGetMenuMode() == STANDBY_MODE)
        {
            IduEna &= ~IDU_ENA;
            IduVidBuf0Addr= (u32)pucClockImageBuf;
            IduVidBuf1Addr= (u32)pucClockImageBuf;
            IduVidBuf2Addr= (u32)pucClockImageBuf;
        }
        else
        {
            IduVidBuf0Addr= (u32)PKBuf0;
            IduVidBuf1Addr= (u32)PKBuf1;
            IduVidBuf2Addr= (u32)PKBuf2;
        }
      }
    #else
      IduVidBuf0Addr= (u32)PKBuf0;
      IduVidBuf1Addr= (u32)PKBuf1;
      IduVidBuf2Addr= (u32)PKBuf2;
	#endif

  #if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A)) )
    #if NEW_IDU_BRI
         BRI_OUT_SIZE =(PANNEL_Y<<16) | (PANNEL_X);
         BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
         BRI_IADDR_C = (unsigned int)PNBuf_C[0];
       #if NEW_IDU_BRI_PANEL_INTLX
         if(src_H > 640)
         {
         #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
           BRI_IN_SIZE =( (src_H/2/2) << 16) | 1024;
         #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
           BRI_IN_SIZE =( (src_H/2) << 16) | src_W;
         #else
           BRI_IN_SIZE =( (src_H/2/2) << 16) | src_W;
         #endif
           BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
         }
         else
         {
         #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
           BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
         #else
           BRI_IN_SIZE =((src_H/2)<<16) | src_W;
         #endif
           BRI_STRIDE = src_W;
         }
         BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
       #else
         if(src_H > 640)
         {
         #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
           BRI_IN_SIZE =((src_H/2)<<16) | 1024;
         #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
           BRI_IN_SIZE =( (src_H) << 16) | src_W;
         #else
           BRI_IN_SIZE =((src_H/2)<<16) | src_W;
         #endif
           BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
         }
         else
         {
          #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
           BRI_IN_SIZE =((src_H/4)<<16) | src_W;
          #else
           BRI_IN_SIZE =(src_H<<16) | src_W;
          #endif
           BRI_STRIDE = src_W;
         }
         BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
       #endif

         IduEna = IDU_ENA | IDU_DATA_EN;
    #else
      IduUV_AddrOffset = PNBUF_SIZE_Y;
      IduEna  = (IDU_DATA_EN | IDU_ENA | IDU_BRI_DECI_1x2 |IDU_BRI_DECI_FIELD_TOP | IDU_YUV420BRI_EN);
    #endif
  #else
      #if NEW_IDU_BRI
         BRI_OUT_SIZE =(PANNEL_Y<<16) | PANNEL_X;

         BRI_IADDR_Y = (unsigned int )PKBuf0;
         BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;

        #if NEW_IDU_BRI_PANEL_INTLX
         if(src_H > 640) //HD
         {
         #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
            BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
         #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
            BRI_IN_SIZE =((src_H/2)<<16) | src_W;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
         #else
            BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
         #endif
         }
         else
         {
         #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
            BRI_IN_SIZE =( (src_H/2/2)<<16 ) | src_W;
         #else
            BRI_IN_SIZE =( (src_H/2)<<16 ) | src_W;
         #endif
            BRI_STRIDE = src_W*2;
         }
         BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
        #else
         if(src_H > 640) //HD
         {
         #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
            BRI_IN_SIZE =( (src_H/2)<<16 ) | 1024;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
         #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
            BRI_IN_SIZE =((src_H)<<16) | src_W;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
         #else
            BRI_IN_SIZE =( (src_H/2)<<16 ) | src_W;
            BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
         #endif

         }
         else
         {
         #if( ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) && (LCM_OPTION  == VGA_640X480_60HZ))
            BRI_IN_SIZE =((src_H/4)<<16) | src_W;
         #else
            BRI_IN_SIZE =(src_H<<16) | src_W;
         #endif
            BRI_STRIDE = src_W*2;
         }
         BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
        #endif
      #endif

      #if (LCM_OPTION == LCM_TM024HDH29)
      #else
        IduEna = IDU_ENA | IDU_DATA_EN;
      #endif
  #endif
  }

  return 1;
}


s32 subTV_Preview(int src_W,int src_H)
{
#if ( (CHIP_OPTION  == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))

    u32* temp_00;
    u32 temp;
    u32 i,j;
	static u8 just_run_one=1;
    //------------------------------//
#if ((CHIP_OPTION == CHIP_A1016B) )

    if(sysTVOutOnFlag) //TV-out
    {
    subTV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);

    tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[0];
    tv2TVFB_IADDR1= (u32)Sub1Videodisplaybuf[1];
    tv2TVFB_IADDR2= (u32)Sub1Videodisplaybuf[2];

    temp         = tv2FRAME_CTL;
    temp         = TV_FRMCTL_VDO_EN    |
                   TV_FRMCTL_TRIPLE;
    temp        &= (~0x00003000);
    temp        |= TV_FRMCTL_FB_SEL0;
    tv2FRAME_CTL  = temp;

    #if TV_DISP_BY_IDU
        #if(TV_DISP_BY_TV_INTR)
            tv2TVE_INTC   =TV_INTC_BOTFDSTART_ENA;
        #else
            tv2TVE_INTC   =TV_INTC_FRMEND__ENA;    //TV interrupt control *
        #endif
    #else
        tv2TVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
    #endif

    tv2FIFO_TH = 0x008a6403;

    #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
         tv2TVFB_STRIDE= 704/2;
    #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
         tv2TVFB_STRIDE= 640/2;
    #else
         tv2TVFB_STRIDE= 640/2;
    #endif

#if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) ) )
    #if NEW_IDU_BRI
        #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
            if(TvOutMode == SYS_TV_OUT_PAL)
            {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                    BRI2_IN_SIZE =( (src_H/2/2)<<16 ) | 1024;
                #else
                    BRI2_IN_SIZE =( (src_H/2/2)<<16 ) | src_W;
                #endif
                }
                else
                    BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

                #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                    BRI2_OUT_SIZE =(288<<16) | 704;
                #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                    BRI2_OUT_SIZE =(288<<16) | 640;
                #else
                    BRI2_OUT_SIZE =(240<<16) | 640;
                #endif
            }
            else
            {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) ||\
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | 1024;
                #else
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | src_W;
                #endif
                }
                else
                    BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

                #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                    BRI2_OUT_SIZE =(240<<16) | 704;
                #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                    BRI2_OUT_SIZE =(240<<16) | 640;
                #else
                    BRI2_OUT_SIZE =(240<<16) | 640;
                #endif
            }

            if(src_H > 640)
                BRI2_STRIDE = BRI_BFA_DIV1 | (src_W*2); //跳行抓,省頻寬
            else
                BRI2_STRIDE = src_W;

            BRI2_IADDR_Y = (unsigned int)PNBuf_Y[0];
            BRI2_IADDR_C = (unsigned int)PNBuf_C[0];
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
            tv2TVE_EN |=TVE_EN;           //Trun on TV-Encoder

        #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
            if(src_H > 640)
            {
            #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                BRI2_IN_SIZE =(src_H<<16) | 1024;
            #else
                BRI2_IN_SIZE =(src_H<<16) | src_W;
            #endif
            }
            else
                BRI2_IN_SIZE =(src_H<<16) | src_W;

            BRI2_OUT_SIZE =(720<<16) | 1280;

            if(src_H > 640)
            {
                BRI2_STRIDE = BRI_BFA_DIV1 | src_W;
            }
            else
            {
                BRI2_STRIDE = src_W;
            }

            BRI2_IADDR_Y = (unsigned int)PNBuf_Y[0];
            BRI2_IADDR_C = (unsigned int)PNBuf_C[0];
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
            tv2TVE_EN |=TVE_EN;           //Trun on TV-Encoder

        #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
            if(src_H > 640)
            {
            #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                BRI2_IN_SIZE =((src_H/2)<<16) | 1024;
            #else
                BRI2_IN_SIZE =((src_H/2)<<16) | src_W;
            #endif
            }
            else
                BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

            #if SIMU1080I_VIA720P
            BRI_OUT_SIZE =(360<<16) | 1280;
            #else
            BRI2_OUT_SIZE =(540<<16) | 1920;
            #endif

            if(src_H > 640)
            {
                BRI2_STRIDE = BRI_BFA_DIV1 | src_W;
            }
            else
            {
                BRI2_STRIDE = src_W;
            }

            BRI2_IADDR_Y = (unsigned int)PNBuf_Y[0];
            BRI2_IADDR_C = (unsigned int)PNBuf_C[0];
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
            tv2TVE_EN |=TVE_EN;           //Trun on TV-Encoder
        #endif

    #else
        //Lucian: For A1013A only
    #endif
#else   //for YUV422 output
    #if NEW_IDU_BRI
        #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
            if(TvOutMode == SYS_TV_OUT_PAL)
            {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | 1024;
                #else
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | src_W;
                #endif
                }
                else
                    BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

                #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                    BRI2_OUT_SIZE =(288<<16) | 704;
                #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                    BRI2_OUT_SIZE =(288<<16) | 640;
                #else
                    BRI2_OUT_SIZE =(240<<16) | 640;
                #endif
            }
            else
            {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | 1024;
                #else
                    BRI2_IN_SIZE =((src_H/2/2)<<16) | src_W;
                #endif
                }
                else
                    BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

            #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                BRI2_OUT_SIZE =(240<<16) | 704;
            #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                BRI2_OUT_SIZE =(240<<16) | 640;
            #else
                BRI2_OUT_SIZE =(240<<16) | 640;
            #endif
            }

            if(src_H > 640)
                BRI2_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
            else
                BRI2_STRIDE = src_W*2;

            BRI2_IADDR_Y = (unsigned int )PKBuf0;
            BRI2_IADDR_C = BRI2_IADDR_Y + PNBUF_SIZE_Y;
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;

        #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)

            if(src_H > 640)
            {
            #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                BRI2_IN_SIZE =(src_H<<16) | 1024;
            #else
                BRI2_IN_SIZE =(src_H<<16) | src_W;
            #endif
            }
            else
                BRI2_IN_SIZE =(src_H<<16) | src_W;

            BRI2_OUT_SIZE =(720<<16) | 1280;


            if(src_H > 640)
                BRI2_STRIDE = BRI_BFA_DIV1 | (src_W*2);
            else
                BRI2_STRIDE = src_W*2;

            BRI2_IADDR_Y = (unsigned int )PKBuf0;
            BRI2_IADDR_C = BRI2_IADDR_Y + PNBUF_SIZE_Y;
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)

            if(src_H > 640)
            {
            #if( (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
                BRI2_IN_SIZE =((src_H/2)<<16) | 1024;
            #else
                BRI2_IN_SIZE =((src_H/2)<<16) | src_W;
            #endif
            }
            else
                BRI2_IN_SIZE =((src_H/2)<<16) | src_W;

            #if SIMU1080I_VIA720P
            BRI_OUT_SIZE =(360<<16) | 1280;
            #else
            BRI2_OUT_SIZE =(540<<16) | 1920;
            #endif

            if(src_H > 640)
                BRI2_STRIDE = BRI_BFA_DIV1 | (src_W*2);
            else
                BRI2_STRIDE = src_W*2;

            BRI2_IADDR_Y = (unsigned int )PKBuf0;
            BRI2_IADDR_C = BRI2_IADDR_Y + PNBUF_SIZE_Y;
            BRI2_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
         #endif

    #endif
        tv2TVE_EN |=TVE_EN;           //Trun on TV-Encoder
#endif
    }
    else
    {

    }
  return 1;
#else
    subTV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);

    tv2TVFB_IADDR0= (u32)Sub1Videodisplaybuf[0];
    tv2TVFB_IADDR1= (u32)Sub1Videodisplaybuf[1];
    tv2TVFB_IADDR2= (u32)Sub1Videodisplaybuf[2];

    temp         = tv2FRAME_CTL;
    temp         = TV_FRMCTL_VDO_EN    |
                 TV_FRMCTL_TRIPLE;
    temp        &= (~0x00003000);
    temp        |= TV_FRMCTL_FB_SEL0;
    tv2FRAME_CTL  = temp;

    #if TV_DISP_BY_IDU
        #if(TV_DISP_BY_TV_INTR)
            tv2TVE_INTC   =TV_INTC_BOTFDSTART_ENA;
        #else
            tv2TVE_INTC   =TV_INTC_FRMEND__ENA;    //TV interrupt control *
        #endif
    #else
        tv2TVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
    #endif

    tv2FIFO_TH = 0x008a6403;

    tv2TVE_EN |= TVE_EN;           //Trun on TV-Encoder

    return 1;
#endif
#endif
}
/*

Routine Description:

    Capture primary.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iduCapturePrimary(void)
{
     if(sysTVOutOnFlag) //TV-out
     {
     #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
          TvOutMode=SYS_TV_OUT_HD720P;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
     #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
          TvOutMode=SYS_TV_OUT_FHD1080I;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
     #else
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
     #endif

            tvFRAME_CTL = TV_FRMCTL_VDO_EN    |
                          //TV_FRMCTL_OSD0_EN |
                          //TV_FRMCTL_OSD1_EN |
                          //TV_FRMCTL_OSD2_EN |
                          //TV_FRMCTL_FB_AUTO |
                          TV_FRMCTL_TRIPLE    |
                          TV_FRMCTL_FB_SEL0;

            IduVidBuf0Addr= (u32)PKBuf0;
            IduVidBuf1Addr= (u32)PKBuf1;
            IduVidBuf2Addr= (u32)PKBuf2;
      #if(CHIP_OPTION == CHIP_A1013A)
          tvFIFO_TH = 0x00002103;
      #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
          tvFIFO_TH = 0x40008401;
      #elif( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
          tvFIFO_TH = 0x40008501;
      #else
          tvFIFO_TH = 0x00002104;
      #endif
            tvTVE_INTC   =0x00000000;    //TV interrupt control *

            tvTVE_EN |=TVE_EN;    //Trun on TV-Encoder

     }
     else //Pannel-out
     {
            IDU_Init(0 , 1);

     #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
            IduWinCtrl =IDU_CTL_VDO_ENA|
                                IDU_CTL_OSD0_DISA|
                                IDU_CTL_OSD1_DISA|
                                IDU_CTL_OSD2_DISA|
                                IDU_CTL_FBAUTO_DISA|
                                IDU_CTL_TRIPLE_ENA| // ? It relate to ISU triple-setting ?
                                IDU_CTL_IDU_WAIT_DISA |
                                IDU_CTL_SRC_FMT_SRGB |  // serial RGB mode
                                IDU_CTL_FB_SEL_0;
	#elif( (LCM_OPTION  == LCM_P_RGB_888_Innolux) || (LCM_OPTION  == LCM_P_RGB_888_HannStar) || (LCM_OPTION==LCM_P_RGB_888_AT070TN90) || (LCM_OPTION == LCM_P_RGB_888_SY700BE104) ||\
            (LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227)||(LCM_OPTION == LCM_P_RGB_888_ILI6122)||(LCM_OPTION == LCM_P_RGB_888_ILI6126C))
	        IduWinCtrl =        IDU_CTL_VDO_ENA|
                                IDU_CTL_OSD0_DISA|
                                IDU_CTL_OSD1_DISA|
                                IDU_CTL_OSD2_DISA|
                                IDU_CTL_FBAUTO_DISA|
                                IDU_CTL_TRIPLE_ENA| // ? It relate to ISU triple-setting ?
                                IDU_CTL_IDU_WAIT_DISA |
                                IDU_CTL_HISH_SPEED_EN |
                                IDU_CTL_FB_SEL_0;
    #elif( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
	        IduWinCtrl =        IDU_CTL_VDO_ENA|
                                IDU_CTL_OSD0_DISA|
                                IDU_CTL_OSD1_DISA|
                                IDU_CTL_OSD2_DISA|
                                IDU_CTL_FBAUTO_DISA|
                                IDU_CTL_TRIPLE_ENA| // ? It relate to ISU triple-setting ?
                                IDU_CTL_IDU_WAIT_DISA |
                                IDU_CTL_HISH_SPEED_EN |
                                IDU_CTL_FB_SEL_0;

    #else
            IduWinCtrl =IDU_CTL_VDO_ENA|
                                IDU_CTL_OSD0_DISA|
                                IDU_CTL_OSD1_DISA|
                                IDU_CTL_OSD2_DISA|
                                IDU_CTL_FBAUTO_DISA|
                                IDU_CTL_TRIPLE_ENA| // ? It relate to ISU triple-setting ?
                                IDU_CTL_IDU_WAIT_DISA |
                                IDU_CTL_FB_SEL_0;
    #endif
                IduIntCtrl  = 0;


            // 0x0024
            IduMpuCmdCfg = IDU_CMD_W_16;
#if( (HW_BOARD_OPTION==PROJECT_MR8980_6720)||(HW_BOARD_OPTION==PROJECT_OPCOM_REAL)||(HW_BOARD_OPTION==PROJECT_DW950_REAL) )

#else
            // 0x0030
            IduOsdCtrl = 0x00000000;
#endif

            // 0x0034, 0x0038
            IduVidWinStart = 0x00000000;
            IduVidWinEnd = ((PANNEL_X-1)<<IDU_VDO_WSX_SHFT )|((PANNEL_Y-1)<<IDU_VDO_WSY_SHFT);

            // 0x00a0, 0x00a4, 0x00a8
            IduVidBuf0Addr= (u32)PKBuf0;

            //0x00ac
#if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
            IduVidBufStride = PANNEL_X/4;
#else
            IduVidBufStride = PANNEL_X/2;
#endif

            // 0x00c0
            IduCrcCtrl = 0x00000000;

            IduFifoThresh = 0xf00a8304;

            //Trigger and start

    #if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04))
            IduEna = IDU_SINGLE | IDU_ENA | IDU_DATA_EN;
    #else
            IduEna = IDU_ENA | IDU_DATA_EN;
    #endif
        }
    return 1;
}


/*BJ 0530 S*/
s32 iduCaptureVideo(int src_W,int src_H)
{
     int i;
     //======//
     DEBUG_IDU("iduCaptureVideo: W=%d,H=%d\n",src_W,src_H);
     if(sysTVOutOnFlag) //TV-out
     {
        #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
          TvOutMode=SYS_TV_OUT_HD720P;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
        #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
          TvOutMode=SYS_TV_OUT_FHD1080I;
          TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
        #else
     	  TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
        #endif

        tvFRAME_CTL = TV_FRMCTL_VDO_EN    |
                      //TV_FRMCTL_OSD0_EN |
                      //TV_FRMCTL_OSD1_EN |
                      //TV_FRMCTL_OSD2_EN |
                      TV_FRMCTL_FBAUTO_DISA|
                      TV_FRMCTL_TRIPLE    |
                   #if ISUCIU_PREVIEW_PNOUT  //Lucian: @Video clip mode, 固定在FB_SEL0, 改變IduVidBuf0Addr 即可.
                      TV_FRMCTL_FB_SEL0;
                   #else
                      TV_FRMCTL_FB_SEL0;
                   #endif

        #if TV_DISP_BY_IDU
          #if(TV_DISP_BY_TV_INTR)
             tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
          #else
             tvTVE_INTC   =TV_INTC_FRMEND__ENA;    //TV interrupt control *
          #endif
        #else
          tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
        #endif

        #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
            (CHIP_OPTION == CHIP_A1018B) )
          tvFIFO_TH = 0x40008401;
        #elif( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
          tvFIFO_TH = 0x40008501;
        #else
          tvFIFO_TH = 0x00002104;
        #endif
#if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
     tvTVFB_STRIDE= 1280/2;
#elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
     #if SIMU1080I_VIA720P
     tvTVFB_STRIDE= 1280/2;
     #else
     tvTVFB_STRIDE= 1920/2;
     #endif
#else
     #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
         tvTVFB_STRIDE= 704/2;
     #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
         tvTVFB_STRIDE= 640/2;
     #else
         tvTVFB_STRIDE= 640/2;
     #endif
#endif
        IduVidBuf0Addr= (u32)PKBuf0;
        IduVidBuf1Addr= (u32)PKBuf1;
        IduVidBuf2Addr= (u32)PKBuf2;

  #if ( IDUTV_DISPLAY_PN )
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }
           else
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
           #else
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);  //Lucian: 跳行抓
           #endif
           }
           else
              BRI_STRIDE = src_W;

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P )
           BRI_IN_SIZE =(src_H<<16) | src_W;
           BRI_OUT_SIZE =(720<<16) | 1280;

           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | src_W;
           else
              BRI_STRIDE = src_W;

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I )
           BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif

           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | src_W;
           else
              BRI_STRIDE = src_W;

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
         #endif
  #else   //YUV422
       #if NEW_IDU_BRI
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }
           else
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           #else
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);  //Lucian: 跳行抓
           #endif
           }
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int)PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
           BRI_IN_SIZE =(src_H<<16) | src_W;
           BRI_OUT_SIZE =(720<<16) | 1280;

           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int)PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
           BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif
           if(src_H > 640)
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           else
              BRI_STRIDE = src_W*2;

           BRI_IADDR_Y = (unsigned int)PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
         #endif

       #endif
       tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
  #endif

     }
     else //Pannel out
     {
        IDU_Init(0 , 1);

        //0x0004
        IduWinCtrl =IDU_CTL_VDO_ENA|
                  #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IDU_CTL_SRC_FMT_SRGB |
                  #endif
                    IDU_CTL_OSD0_DISA|
                    IDU_CTL_OSD1_DISA|
                    IDU_CTL_OSD2_DISA|
                    IDU_CTL_FBAUTO_DISA|
                    IDU_CTL_TRIPLE_ENA| // ? It relate to ISU triple-setting ?
                    IDU_CTL_IDU_WAIT_DISA |
                  #if( (LCM_OPTION == LCM_P_RGB_888_Innolux) || (LCM_OPTION == LCM_P_RGB_888_HannStar) || (LCM_OPTION == LCM_P_RGB_888_AT070TN90) || \
                    (LCM_OPTION == LCM_P_RGB_888_SY700BE104) || (LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227)||\
                    (LCM_OPTION == LCM_P_RGB_888_ILI6122)||(LCM_OPTION == LCM_P_RGB_888_ILI6126C))
                    IDU_CTL_HISH_SPEED_EN |
				  #endif
                  #if ( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
                    IDU_CTL_HISH_SPEED_EN |
				  #endif
                  #if ISUCIU_PREVIEW_PNOUT  //Lucian: @Video clip mode, 固定在FB_SEL0, 改變IduVidBuf0Addr 即可.
                    IDU_CTL_FB_SEL_0;
                  #else
                    IDU_CTL_FB_SEL_2;
                  #endif

        IduIntCtrl  = 0;


        // 0x0024
        IduMpuCmdCfg = IDU_CMD_W_16;

        // 0x0030
        IduOsdCtrl = 0x00000000;


        // 0x0034, 0x0038
        IduVidWinStart = 0x00000000;

        IduVidWinEnd = ((PANNEL_X-1)<<IDU_VDO_WSX_SHFT )|((PANNEL_Y-1)<<IDU_VDO_WSY_SHFT);

		IduVidBuf0Addr= (u32)PKBuf0;
		IduVidBuf1Addr= (u32)PKBuf1;
		IduVidBuf2Addr= (u32)PKBuf2;

        //0x00ac
    #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
        IduVidBufStride = PANNEL_X/4;
    #else
        IduVidBufStride = PANNEL_X/2;
    #endif

        // 0x00c0
        IduCrcCtrl = 0x00000000;
    #if NEW_IDU_BRI
         IduFifoThresh = 0xf00a8304;
    #else
        IduFifoThresh = 0xc0867504; //Lucian: Avoid Zoom noise, @20080706
    #endif

#if ( IDUTV_DISPLAY_PN )
       BRI_OUT_SIZE =(PANNEL_Y<<16) | PANNEL_X;

       BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
       BRI_IADDR_C = (unsigned int)PNBuf_C[0];

     #if NEW_IDU_BRI_PANEL_INTLX
       if(src_H > 640)
       {
       #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
         BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
         BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2); //Lucian: 跳行抓
       #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
         BRI_IN_SIZE =((src_H/2)<<16) | src_W;
         BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
       #else
         BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
         BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2); //Lucian: 跳行抓
       #endif

       }
       else
       {
         BRI_IN_SIZE =((src_H/2)<<16) | src_W;
         BRI_STRIDE = src_W;
       }
       BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
     #else
       if(src_H > 640)
       {
       #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
          BRI_IN_SIZE =((src_H/2)<<16) | 1024;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2); //Lucian: 跳行抓
       #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
          BRI_IN_SIZE =((src_H)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
       #else
          BRI_IN_SIZE =((src_H/2)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2); //Lucian: 跳行抓
       #endif

       }
       else
       {
          BRI_IN_SIZE =(src_H<<16) | src_W;
          BRI_STRIDE = src_W;
       }
       BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
     #endif

       IduEna = IDU_ENA | IDU_DATA_EN;
#else
   #if NEW_IDU_BRI
       BRI_OUT_SIZE =(PANNEL_Y<<16) | PANNEL_X;

       BRI_IADDR_Y = (unsigned int )PKBuf0;
       BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
      #if NEW_IDU_BRI_PANEL_INTLX
       if(src_H > 640)
       {
       #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
          BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
       #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
          BRI_IN_SIZE =((src_H/2)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
       #else
          BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
       #endif

       }
       else
       {
          BRI_IN_SIZE =((src_H/2)<<16) | src_W;
          BRI_STRIDE = src_W*2;
       }
       BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
      #else
       if(src_H > 640)
       {
       #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
        (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
          BRI_IN_SIZE =((src_H/2)<<16) | 1024;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
       #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
          BRI_IN_SIZE =((src_H)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
       #else
          BRI_IN_SIZE =((src_H/2)<<16) | src_W;
          BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
       #endif

       }
       else
       {
          BRI_IN_SIZE =(src_H<<16) | src_W;
          BRI_STRIDE = src_W*2;
       }
       BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
      #endif
    #endif
     #if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04))
        IduEna = IDU_ENA | IDU_SINGLE | IDU_DATA_EN;  //on LCM,we support SINGLE
     #elif (LCM_OPTION == LCM_TM024HDH29)

     #else
		IduEna = IDU_ENA | IDU_DATA_EN;
     #endif
#endif
    }
    return 1;
}
/*BJ 0530 E*/

/* BJ 0505 S */



/* BJ 0505 E */
void iduOSDDisable_All(void)
{
    u8  i;
    for(i = 0; i < IDU_OSD_MAX_NUM; i++)
        iduOSDDisable(i);
}
void iduOSDEnable_All(void)
{
    u8  i;
    for(i = 0; i < IDU_OSD_MAX_NUM; i++)
        iduOSDEnable(i);
}
void iduOSDDisable(u8 osd)
{
    switch(osd)
    {
        case IDU_OSD_L0_WINDOW_0:
            IduWinCtrl  &= ~(IDU_CTL_OSD0_ENA);
            break;

        case IDU_OSD_L0_WINDOW_1:
            IduWinCtrl  &= ~(IDU_CTL_OSD1_ENA);
            break;

        case IDU_OSD_L0_WINDOW_2:
            IduWinCtrl  &= ~(IDU_CTL_OSD2_ENA);
            break;

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        case IDU_OSD_L1_WINDOW_0:
            IduOsdL1Ctrl  &= ~(IDU_L1_OSD0_ENA);
            break;

        case IDU_OSD_L2_WINDOW_0:
            IduOsdL2Ctrl  &= ~(IDU_L2_OSD0_ENA);
            break;
#endif
        default:
            break;
    }

}

void iduOSDEnable(u8 osd)
{
    if(IsIduOsdEnable == 0)
        return;

    if (sysTVOutOnFlag == 0)
    {
        if ((OSD_SizeX >= 800) && (osd < IDU_OSD_L1_WINDOW_0))
        {
            DEBUG_IDU("OSD Layer %d can not enable\n",osd);
            return;
        }
    }
    switch(osd)
    {
        case IDU_OSD_L0_WINDOW_0:
            IduWinCtrl |= IDU_CTL_OSD0_ENA;
            break;

        case IDU_OSD_L0_WINDOW_1:
            IduWinCtrl |= IDU_CTL_OSD1_ENA;
            break;

        case IDU_OSD_L0_WINDOW_2:
            IduWinCtrl |= IDU_CTL_OSD2_ENA;
            break;

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        case IDU_OSD_L1_WINDOW_0:
            IduOsdL1Ctrl |= IDU_L1_OSD0_ENA;
            break;

        case IDU_OSD_L2_WINDOW_0:
            IduOsdL2Ctrl |= IDU_L2_OSD0_ENA;
            break;
#endif
        default:
            break;
    }
}
void iduTVOSDDisable_All(void)
{
    u8  i;
    for(i = 0; i < IDU_OSD_MAX_NUM; i++)
        iduTVOSDDisable(i);
}

void iduTVOSDDisable(u8 tvosd)
{
    switch(tvosd)
    {
        case IDU_OSD_L0_WINDOW_0:
            tvFRAME_CTL &= ~(TV_FRMCTL_OSD0_EN);
            break;

        case IDU_OSD_L0_WINDOW_1:
            tvFRAME_CTL &= ~(TV_FRMCTL_OSD1_EN);
            break;

        case IDU_OSD_L0_WINDOW_2:
            tvFRAME_CTL &= ~(TV_FRMCTL_OSD2_EN);
            break;

        default:
            break;
    }
}
void iduTVOSDEnable(u8 tvosd)
{
    if(IsIduOsdEnable == 0)
        return;

    if (sysTVOutOnFlag == 0)
    {
        if ((OSD_SizeX >= 800) && (tvosd < IDU_OSD_L1_WINDOW_0))
        {
            DEBUG_IDU("TVOSD Layer %d can not enable\n",tvosd);
            return;
        }
    }
    switch(tvosd)
    {
        case IDU_OSD_L0_WINDOW_0:
            tvFRAME_CTL |= TV_FRMCTL_OSD0_EN;
            break;

        case IDU_OSD_L0_WINDOW_1:
            tvFRAME_CTL |= TV_FRMCTL_OSD1_EN;
            break;

        case IDU_OSD_L0_WINDOW_2:
            tvFRAME_CTL |= TV_FRMCTL_OSD2_EN;
            break;

        default:
            break;
    }
}


/* BJ 0505 E */

void iduOSDClear(void)
{
    IduOsdWin0Start = 0;
    IduOsdWin0End = 0;
    IduOsdWin1Start = 0;
    IduOsdWin1End = 0;
    IduOsdWin2Start = 0;
    IduOsdWin2End = 0;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IduOsdL1Win0Start = 0;
    IduOsdL1Win0End = 0;
    IduOsdL2Win0Start = 0;
    IduOsdL2Win0End = 0;
#endif
    IduOsdBufStride = 0;
}


void iduOSDColorRGB(void)
{
    IduOsdPal0  = 0x00FFFFFF;
    IduOsdPal1  = 0x00000000;
    IduOsdPal2  = 0x000000FF;
    IduOsdPal3  = 0x0000C369;
    IduOsdPal4  = 0x00FF0000;
    IduOsdPal5  = 0x00212121;
    IduOsdPal6  = 0x0000E1FF;
    IduOsdPal7  = 0x00F0F0F0;
    IduOsdPal8  = 0x00007CFC;
    IduOsdPal9  = 0x00F28CBF;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IduOsdPal10 = 0x00484849;
    IduOsdPal11 = 0x00605f5f;
    IduOsdPal12 = 0x0071716f;
    IduOsdPal13 = 0x008a8a8c;
    IduOsdPal14 = 0x00979797;
    IduOsdPal15 = 0x00BFBFBF;
    IduOsdPal16 = 0x00201832;
    IduOsdPal17 = 0x000f0819;
    IduOsdPal18 = 0x00cfc2df;
    IduOsdPal19 = 0x00e49df1;
    IduOsdPal20 = 0x00F8E8F8;
    IduOsdPal21 = 0x00b93f93;
    IduOsdPal22 = 0x00f0d5e6;
    IduOsdPal23 = 0x008f1e5d;
    IduOsdPal24 = 0x00f8e8e8;
    IduOsdPal25 = 0x00333030;
    IduOsdPal26 = 0x008e1609;
    IduOsdPal27 = 0x0035110a;
    IduOsdPal28 = 0x00a5a1a0;
    IduOsdPal29 = 0x00bcb8b6;
    IduOsdPal30 = 0x008a8887;
    IduOsdPal31 = 0x00766e69;
    IduOsdPal32 = 0x00ab4a0b;
    IduOsdPal33 = 0x00524f4d;
    IduOsdPal34 = 0x00d4aa7f;
    IduOsdPal35 = 0x00eecba7;
    IduOsdPal36 = 0x001a1816;
    IduOsdPal37 = 0x00dad8d6;
    IduOsdPal38 = 0x00c78f52;
    IduOsdPal39 = 0x006b4920;
    IduOsdPal40 = 0x00c17516;
    IduOsdPal41 = 0x00392c12;
    IduOsdPal42 = 0x00f5e8cd;
    IduOsdPal43 = 0x00eec40c;
    IduOsdPal44 = 0x00f8f8e4;
    IduOsdPal45 = 0x00e7f8e7;
    IduOsdPal46 = 0x00178c21;
    IduOsdPal47 = 0x00bbd9bf;
    IduOsdPal48 = 0x002cb949;
    IduOsdPal49 = 0x00d3e8d8;
    IduOsdPal50 = 0x005dca85;
    IduOsdPal51 = 0x00081912;
    IduOsdPal52 = 0x00e3f8f8;
    IduOsdPal53 = 0x0097f0f7;
    IduOsdPal54 = 0x003ac4ec;
    IduOsdPal55 = 0x001a343b;
    IduOsdPal56 = 0x0077d0ee;
    IduOsdPal57 = 0x00236a85;
    IduOsdPal58 = 0x002599e1;
    IduOsdPal59 = 0x00c9d6ef;
    IduOsdPal60 = 0x00899ed0;
    IduOsdPal61 = 0x00e1e8f8;
    IduOsdPal62 = 0x005467cf;
    IduOsdPal63 = 0x00233ce3;
#else
    IduOsdPal10 = 0x00969696;
    IduOsdPal11 = 0x00E2CABF;
    IduOsdPal12 = 0x00A1500A;
#endif
}

void iduOSDColorYUV(void)
{
    tvOSD_PAL0  = 0x008181FD;
    tvOSD_PAL1  = 0x00808000;
    tvOSD_PAL2  = 0x00FF554C;
    tvOSD_PAL3  = 0x00622C91;
    tvOSD_PAL4  = 0x006CFF1C;
    tvOSD_PAL5  = 0x0080801F;
    tvOSD_PAL6  = 0x00A209CF;
    tvOSD_PAL7  = 0x008080EF;
    tvOSD_PAL8  = 0x00CB2B93;
    tvOSD_PAL9  = 0x0092ACA6;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    tvOSD_PAL10 =  0x00807F4E;
    tvOSD_PAL11 =  0x007F8061;
    tvOSD_PAL12 =  0x007F806F;
    tvOSD_PAL13 =  0x00807F87;
    tvOSD_PAL14 =  0x00808091;
    tvOSD_PAL15 =  0x008080B4;
    tvOSD_PAL16 =  0x008A7F2C;
    tvOSD_PAL17 =  0x0086801B;
    tvOSD_PAL18 =  0x008B81BF;
    tvOSD_PAL19 =  0x009F92B3;
    tvOSD_PAL20 =  0x008584DC;
    tvOSD_PAL21 =  0x009CA967;
    tvOSD_PAL22 =  0x008589CD;
    tvOSD_PAL23 =  0x0093A845;
    tvOSD_PAL24 =  0x007E87D8;
    tvOSD_PAL25 =  0x007F8139;
    tvOSD_PAL26 =  0x0071B62B;
    tvOSD_PAL27 =  0x007A9020;
    tvOSD_PAL28 =  0x007F819A;
    tvOSD_PAL29 =  0x007E82AD;
    tvOSD_PAL30 =  0x007F8184;
    tvOSD_PAL31 =  0x007D846D;
    tvOSD_PAL32 =  0x005DB348;
    tvOSD_PAL33 =  0x007E8153;
    tvOSD_PAL34 =  0x006A989B;
    tvOSD_PAL35 =  0x006D94B8;
    tvOSD_PAL36 =  0x007E8124;
    tvOSD_PAL37 =  0x007E81C9;
    tvOSD_PAL38 =  0x0061A180;
    tvOSD_PAL39 =  0x006B9447;
    tvOSD_PAL40 =  0x00738930;
    tvOSD_PAL41 =  0x007389D1;
    tvOSD_PAL42 =  0x002BAD8E;
    tvOSD_PAL43 =  0x007782DF;
    tvOSD_PAL44 =  0x00797BDE;
    tvOSD_PAL45 =  0x00595C61;
    tvOSD_PAL46 =  0x007676C0;
    tvOSD_PAL47 =  0x00585284;
    tvOSD_PAL48 =  0x007A79D1;
    tvOSD_PAL49 =  0x00695AA1;
    tvOSD_PAL50 =  0x007E7922;
    tvOSD_PAL51 =  0x008080EB;
    tvOSD_PAL52 =  0x008176E2;
    tvOSD_PAL53 =  0x008957D7;
    tvOSD_PAL54 =  0x009B3DB5;
    tvOSD_PAL55 =  0x0084733B;
    tvOSD_PAL56 =  0x009354C1;
    tvOSD_PAL57 =  0x00905C6B;
    tvOSD_PAL58 =  0x00A7429A;
    tvOSD_PAL59 =  0x008B76CC;
    tvOSD_PAL60 =  0x00976FA2;
    tvOSD_PAL61 =  0x00877ADA;
    tvOSD_PAL62 =  0x00AF6881;
    tvOSD_PAL63 =  0x00CB5C6C;
#else
    tvOSD_PAL10 = 0x00808194;
    tvOSD_PAL11 = 0x00798FC8;
    tvOSD_PAL12 = 0x0056B643;
    tvOSD_PAL13 = 0x004E8AED;
    tvOSD_PAL14 = 0x008A4EED;
#endif
}

#if (IS_COMMAX_DOORPHONE) || (IS_HECHI_DOORPHONE) || (HW_BOARD_OPTION == MR6730_AFN)
    // defined in MainFlow.c
#else
void iduOSDBRI(u32 ADDR, u32 SREIDE,u32 SIZE)
{
    #if (CHIP_OPTION == CHIP_A1018B)
        if ((SREIDE/64)!=0)
        {
            DEBUG_IDU("iduOSDBRI Only support 64 pixel alignments \n");
            return;
        }
        OSD_BRI_ADDR = ADDR;
        OSD_BRI_SREIDE = SREIDE;
        OSD_BRI_WIDTH = SREIDE;
        OSD_BRI_SIZE = SIZE;

        DEBUG_IDU("iduOSDBRI_EN \n");
        OSD_BRI_CTRL = 0x1 ;// enable
    #endif
    
    #if IDU_OSD_TEST
        if ((SREIDE/64)!=0)
        {
            DEBUG_IDU("iduOSDBRI Only support 64 pixel alignments \n");
            return;
        }
        DEBUG_IDU("iduOSDBRI_EN \n");
        OSD_BRI_CTRL = 0x1 ;// enable
        OSD_BRI_ADDR = ADDR;
        OSD_BRI_SREIDE = SREIDE;
        OSD_BRI_WIDTH = SREIDE;
        OSD_BRI_SIZE = SIZE;
    #endif
}

void iduOSDDisplay1(u8 blk_idx, u32 OSD_SX, u32 OSD_SY, u32 OSD_EX, u32 OSD_EY)
{

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))

#else  //Lucian: 不應在  OSD function 內設定Video register. 暫時保留.
    u32 unDisplayWidth = PANNEL_X;

    u32 unDisplayWindowXStart = 0;
    u32 unDisplayWindowYStart = 0;
    u32 unDisplayWindowXEnd = PANNEL_X - 1;
    u32 unDisplayWindowYEnd = PANNEL_Y - 1;

    IduVidWinStart  = (unDisplayWindowYStart << 16) | unDisplayWindowXStart;
    IduVidWinEnd    = ((unDisplayWindowYEnd) << 16) | (unDisplayWindowXEnd);
  #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
    IduVidBufStride = unDisplayWidth / 4;
  #else
    IduVidBufStride = unDisplayWidth / 2;
  #endif
    IduEna      =   IDU_NORMAL|
                    IDU_ENA |
                    IDU_DATA_EN |
                    IDU_DCLK_EN;
#endif

    #if ((LCM_OPTION == LCM_HX5073_YUV)||(LCM_OPTION == LCM_HX8224_601) ||(LCM_OPTION == LCM_CCIR601_640x480P) ||(LCM_OPTION == LCM_HX8224_656))
        iduOSDColorYUV();
    #else
        iduOSDColorRGB();
    #endif

    #if ((CHIP_OPTION == CHIP_A1016A))
        if ((OSD_SizeX >= 800) && (blk_idx < IDU_OSD_L1_WINDOW_0))
        {
            DEBUG_IDU("OSD Layer 0 do not have enough bandwith\n");
            return;
        }
    #endif

    OsdBlkInfo[blk_idx].BlkSX = OSD_SX;
    OsdBlkInfo[blk_idx].BlkSY = OSD_SY;
    OsdBlkInfo[blk_idx].BlkEX = OSD_EX;
    OsdBlkInfo[blk_idx].BlkEY = OSD_EY;
    #if (CHIP_OPTION == CHIP_A1018B)
        if (blk_idx == IDU_OSD_L0_WINDOW_0)
            OsdBlkInfo[blk_idx].BlkAddr = OSD_buf1; // for mouse
        else
    OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + OSD_SizeX*OSD_SY+OSD_SX;
    #else
        OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + OSD_SizeX*OSD_SY+OSD_SX;
    #endif
    if(blk_idx == IDU_OSD_L0_WINDOW_0)
    {
        IduOsdWin0Start  = ( OSD_SX | (OSD_SY<<16) );
        IduOsdWin0End    = ( (OSD_EX-1) | ((OSD_EY-1)<<16) );
        IduOsdBuf0Addr   = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        IduOsdBufStride  = (OSD_SizeX/4);

    }
    else if(blk_idx == IDU_OSD_L0_WINDOW_1)
    {
        IduOsdWin1Start  = ( OSD_SX | (OSD_SY<<16) );
        IduOsdWin1End    = ( (OSD_EX-1) | ((OSD_EY-1)<<16) );
        IduOsdBuf1Addr   = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        IduOsdBufStride  |=  (OSD_SizeX/4)<<8;

    }
    else if(blk_idx == IDU_OSD_L0_WINDOW_2)
    {
        IduOsdWin2Start  = ( OSD_SX | (OSD_SY<<16) );
        IduOsdWin2End    = ( (OSD_EX-1) | ((OSD_EY-1)<<16) );
        IduOsdBuf2Addr   = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        IduOsdBufStride  |=  (OSD_SizeX/4)<<16;
    }
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    else if(blk_idx == IDU_OSD_L1_WINDOW_0)
    {
        IduOsdL1Win0Start   = (OSD_SX | (OSD_SY<<16));
        IduOsdL1Win0End     = ( (OSD_EX-1) | ((OSD_EY-1)<<16) );
        IduOsdL1Buf0Addr    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        IduOsdL1BufStride   = (OSD_SizeX/4);
    #if 0
        if (OSD_SizeX >= 800)
            IduOsdL1Ctrl    = (IDU_L1_4BIT_INDEX_OSD|
                               IDU_L1_FRAME_ADDR|
                               IDU_L1_MK_DIS_VIDEO|
                               IDU_L1_UMK_DIS_OSD|
                               IDU_L1_COLOR_KEY);
        else
    #endif
        iduOSDBRI(IduOsdL1Buf0Addr,OSD_SizeX,((OSD_EX-OSD_SX)/4)*(OSD_EY-OSD_SY));// Amon TEST (140624)
        #if (UI_VERSION == UI_VERSION_RDI_3)
            IduOsdL1Ctrl    = (IDU_L1_ALPHA_ON_ENA|IDU_L1_FRAME_ADDR | IDU_L1_ALPHA_MODE_3);
        #else
            IduOsdL1Ctrl    = (IDU_L1_ALPHA_ON_ENA|IDU_L1_FRAME_ADDR);
        #endif
    }
    else if(blk_idx == IDU_OSD_L2_WINDOW_0)
    {
        IduOsdL2Win0Start   = (OSD_SX | (OSD_SY<<16));
        IduOsdL2Win0End     = ( (OSD_EX-1) | ((OSD_EY-1)<<16) );
        IduOsdL2Buf0Addr    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        IduOsdL2BufStride   = (OSD_SizeX/4);
        IduOsdL2Ctrl        = (IDU_L2_ALPHA_ON_ENA|IDU_L2_FRAME_ADDR);
    }
#endif
    IduOsdCtrl  = IDU_ALPHA_ON_ENA|
                  IDU_OSD_RGB;


#if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
      if( (sysCameraMode==SYS_CAMERA_MODE_PLAYBACK) || (sysCameraMode==SYS_CAMERA_MODE_RF_RX_FULLSCR) || ( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA ) || (sysCameraMode==SYS_CAMERA_MODE_RF_RX_QUADSCR) )
         idu_switch();
#endif

}
#endif  // IS_COMMAX_DOORPHONE

void iduTVOSDBRI(u32 ADDR, u32 SREIDE,u32 SIZE)
{
    #if (CHIP_OPTION == CHIP_A1018B)
        if ((SREIDE/64)!=0)
        {
            DEBUG_IDU("iduTVOSDBRI Only support 64 pixel alignments \n");
            return;
        }
        OSD_BRI_ADDR = ADDR;
        OSD_BRI_SREIDE = SREIDE*2;
        OSD_BRI_WIDTH = SREIDE;
        OSD_BRI_SIZE = SIZE/2;

        DEBUG_IDU("iduTVOSDBRI_EN \n");
        OSD_BRI_CTRL = 0x3 ;// enable interlace mode
    #endif

    #if IDU_OSD_TEST
        if ((SREIDE/64)!=0)
        {
            DEBUG_IDU("iduTVOSDBRI Only support 64 pixel alignments \n");
            return;
        }
        DEBUG_IDU("iduTVOSDBRI_EN \n");
        OSD_BRI_CTRL = 0x3 ;// enable
        OSD_BRI_ADDR = ADDR;
        OSD_BRI_SREIDE = SREIDE*2;
        OSD_BRI_WIDTH = SREIDE;
        OSD_BRI_SIZE = SIZE/2;
    #endif

}
void iduOSDBRIClear(void)
{
#if (CHIP_OPTION == CHIP_A1018B) // 2014/07/21
    OSD_BRI_CTRL    = 0;
    OSD_BRI_ADDR    = 0;
    OSD_BRI_SREIDE  = 0;
    OSD_BRI_WIDTH   = 0;
    OSD_BRI_SIZE    = 0;
#endif
}
void iduTVOSDClear(void)
{
    tvOSD0_WSP = 0;
    tvOSD0_WEP    = 0;
    tvOSD1_WSP = 0;
    tvOSD1_WEP    = 0;
    tvOSD2_WSP = 0;
    tvOSD2_WEP    = 0;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IduOsdL1Win0Start = 0;
    IduOsdL1Win0End = 0;
    IduOsdL2Win0Start = 0;
    IduOsdL2Win0End = 0;
#endif
    tvTOFB_STRIDE = 0;
#if (CHIP_OPTION == CHIP_A1018B) // 2014/07/18
    tvOSDL1W0_WSP = 0;
    tvOSDL1W0_WEP = 0;
    tvOSDL1W1_WSP = 0;
    tvOSDL1W1_WEP = 0;
    tvOSDL1W2_WSP = 0;
    tvOSDL1W2_WEP = 0;
    
    tvTOFBL1_STRIDE = 0;
#endif
}

void iduTVOSDDisplay(u8 blk_idx, u32 TVOSD_SX, u32 TVOSD_SY, u32 TVOSD_EX, u32 TVOSD_EY)
{

    u8 X_offset;
    u8 Y_offset;

    iduOSDColorYUV();

    #if 0
        if (TVOSD_SX <48 )
            TVOSD_SX = 48;

        if (TVOSD_EX > 600)
            TVOSD_EX = 600;
    #endif
    OsdBlkInfo[blk_idx].BlkSX = TVOSD_SX;
    OsdBlkInfo[blk_idx].BlkSY = TVOSD_SY;
    OsdBlkInfo[blk_idx].BlkEX = TVOSD_EX;
    OsdBlkInfo[blk_idx].BlkEY = TVOSD_EY;
    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) )
    if (blk_idx >= IDU_OSD_L1_WINDOW_0)
        OsdBlkInfo[blk_idx].BlkAddr = OSD_buf1 + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
    else
        OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
    #elif(CHIP_OPTION == CHIP_A1018B)
        if (blk_idx == IDU_OSD_L1_WINDOW_0)
            OsdBlkInfo[blk_idx].BlkAddr = OSD_buf1; // for mouse
        else
            OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
#else
    OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
#endif
    #if OSD_SIZE_X2_DISABLE
    #else
    TVOSD_SX*=2;
    TVOSD_EX*=2;
    TVOSD_SY*=2;
    TVOSD_EY*=2;
    #endif

    if(TvOutMode==SYS_TV_OUT_PAL)
    {
        X_offset = 0xAE;
        Y_offset = 0x02;
    }
    else if(TvOutMode==SYS_TV_OUT_NTSC)
    {
        X_offset = 0xA2;
        Y_offset = 0x02;
    }
    else if(TvOutMode==SYS_TV_OUT_HD720P)
    {
        X_offset = 0xA2;
        Y_offset = 0x02;
    }
    else if(TvOutMode==SYS_TV_OUT_FHD1080I)
    {
        X_offset = 0xA2;
        Y_offset = 0x02;
    }

    if (blk_idx == IDU_OSD_L0_WINDOW_0)
    {
        tvOSD0_WSP      = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD0_WEP      = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR0    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   = (TVOSD_SizeX/4);
        //iduTVOSDBRI(tvTOFB_IDDR0,TVOSD_SizeX,0x9600);// Amon TEST (140619)
        iduTVOSDBRI(tvTOFB_IDDR0,TVOSD_SizeX,((TVOSD_EX-TVOSD_SX)/4)*(TVOSD_EY-TVOSD_SY)/2);// Amon TEST (140721)
    }
    else if (blk_idx == IDU_OSD_L0_WINDOW_1)
    {
        tvOSD1_WSP      = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD1_WEP      = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR1    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   |=  (TVOSD_SizeX/4)<<8;
    }
    else if (blk_idx == IDU_OSD_L0_WINDOW_2)
    {
        tvOSD2_WSP      = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD2_WEP      = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR2    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   |=  (TVOSD_SizeX/4)<<16;
    }

}

void iduTVOSDDisplay_D1(u8 blk_idx, u32 TVOSD_SX, u32 TVOSD_SY, u32 TVOSD_EX, u32 TVOSD_EY)
{

    u8 X_offset;
    u8 Y_offset;

    iduOSDColorYUV();

    #if 0
        if (TVOSD_SX <48 )
            TVOSD_SX = 48;

        if (TVOSD_EX > 600)
            TVOSD_EX = 600;
    #endif

    OsdBlkInfo[blk_idx].BlkSX = TVOSD_SX;
    OsdBlkInfo[blk_idx].BlkSY = TVOSD_SY;
    OsdBlkInfo[blk_idx].BlkEX = TVOSD_EX;
    OsdBlkInfo[blk_idx].BlkEY = TVOSD_EY;
#if IDU_OSD_TEST
    DEBUG_IDU("******************iduTVOSDDisplay_D1\n");
#else
#if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) )
    if (blk_idx >= IDU_OSD_L1_WINDOW_0)
        OsdBlkInfo[blk_idx].BlkAddr = OSD_buf1 + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
    else
        OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
#else
    OsdBlkInfo[blk_idx].BlkAddr = OSD_buf + TVOSD_SizeX*TVOSD_SY + TVOSD_SX;
#endif
#endif
    #if OSD_SIZE_X2_DISABLE
    #else
    TVOSD_SX*=2;
    TVOSD_EX*=2;
    TVOSD_SY*=2;
    TVOSD_EY*=2;
    #endif

    if(TvOutMode==SYS_TV_OUT_PAL)
    {
        X_offset = 0x88;
        Y_offset = 0x02;
        TVOSD_EX = 698; /* it may cause bandwidth problem in quad mode, if window size=704*/
    }
    else if(TvOutMode==SYS_TV_OUT_NTSC)
    {
        X_offset = 0x8a;
        Y_offset = 0x02;
    }
    else if(TvOutMode==SYS_TV_OUT_HD720P)
    {
        X_offset = 0x8a;
        Y_offset = 0x02;
    }
    else if(TvOutMode==SYS_TV_OUT_FHD1080I)
    {
        X_offset = 0x8a;
        Y_offset = 0x02;
    }

    if (blk_idx == IDU_OSD_L0_WINDOW_0)
    {
        tvOSD0_WSP  = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD0_WEP  = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR0    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   = (TV_MAXOSD_SizeX/4);

    }
    else if(blk_idx == IDU_OSD_L0_WINDOW_1)
    {
        tvOSD1_WSP  = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD1_WEP  = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR1    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   |=  (TV_MAXOSD_SizeX/4)<<8;
    }
    else if (blk_idx == IDU_OSD_L0_WINDOW_2)
    {
        tvOSD2_WSP  = ((X_offset+TVOSD_SX)|((Y_offset+TVOSD_SY)<<16));
        tvOSD2_WEP  = ((X_offset+TVOSD_EX-1)|((Y_offset+TVOSD_EY-1)<<16));
        tvTOFB_IDDR2    = (u32)OsdBlkInfo[blk_idx].BlkAddr;
        tvTOFB_STRIDE   |=  (TV_MAXOSD_SizeX/4)<<16;
    }

}

/*
    0: 4-bit alpha index mode (2-bit alpha + 2-bit index data);
    1: 4-bit color index mode (4-bit index data)
    2: 8-bit alpha index mode (2-bit alpha + 2-bit dummy + 4-bit index data)
*/
u8 iduGetOSDType(u8 buf_idx)
{
    u8 type;

    switch(buf_idx)
    {
        case IDU_OSD_L0_WINDOW_0:
        case IDU_OSD_L0_WINDOW_1:
        case IDU_OSD_L0_WINDOW_2:
            type = IduOsdCtrl & 0x00000003;
            break;

        case IDU_OSD_L1_WINDOW_0:
            type = IduOsdL1Ctrl & 0x00000003;
            break;

        case IDU_OSD_L2_WINDOW_0:
            type = IduOsdL1Ctrl & 0x00000003;
            break;
    }
    return type;
}

u32 iduOSDGetYStartEnd(u8 buf_idx, u32 *OSD_SY, u32 *OSD_EY)
{
    u8  OSDx2;
    switch(buf_idx)
    {
        case OSD_Blk0:
             if (!(IduWinCtrl & IDU_CTL_OSD0_ENA))
                return 0;
            *OSD_SY = IduOsdWin0Start;
            *OSD_EY = IduOsdWin0End;
            break;

        case OSD_Blk1:
            if (!(IduWinCtrl & IDU_CTL_OSD1_ENA))
                return 0;
            *OSD_SY = IduOsdWin1Start;
            *OSD_EY = IduOsdWin1End;
            break;

        case OSD_Blk2:
            if (!(IduWinCtrl & IDU_CTL_OSD2_ENA))
                return 0;
            *OSD_SY = IduOsdWin2Start;
            *OSD_EY = IduOsdWin2End;
            break;

        case OSD_L1Blk0:
            if (!(IduOsdL1Ctrl & IDU_L1_OSD0_ENA))
                return 0;
            *OSD_SY = IduOsdL1Win0Start;
            *OSD_EY = IduOsdL1Win0End;
            break;

        case OSD_L2Blk0:
            if (!(IduOsdL2Ctrl & IDU_L2_OSD0_ENA))
                return 0;
            *OSD_SY = IduOsdL2Win0Start;
            *OSD_EY = IduOsdL2Win0End;
            break;

        default:
            return 0;
    }

    *OSD_SY >>= 16;
    *OSD_EY >>= 16;

#if OSD_SIZE_X2_DISABLE
    OSDx2 = 0;
#else
    OSDx2 = 1;
#endif

    if(sysTVOutOnFlag)
    {
        if(TvOutMode==SYS_TV_OUT_PAL)
        {
            *OSD_SY = (*OSD_SY - 0x02 - 0x30)>>OSDx2;
            *OSD_EY = (*OSD_EY - 0x02 - 0x30)>>OSDx2;
        }
        else if(TvOutMode==SYS_TV_OUT_NTSC)
        {
            *OSD_SY = (*OSD_SY - 0x02)>>OSDx2;
            *OSD_EY = (*OSD_EY - 0x02)>>OSDx2;
        }
        else if(TvOutMode==SYS_TV_OUT_HD720P)
        {
            *OSD_SY = (*OSD_SY - 0x02)>>OSDx2;
            *OSD_EY = (*OSD_EY - 0x02)>>OSDx2;
        }
        else if(TvOutMode==SYS_TV_OUT_FHD1080I)
        {
            *OSD_SY = (*OSD_SY - 0x02)>>OSDx2;
            *OSD_EY = (*OSD_EY - 0x02)>>OSDx2;
        }
    }
    return 1;

}

void iduPlaybackToMenu(void)
{

    if (sysTVOutOnFlag)
    {
        if(TvOutMode == SYS_TV_OUT_PAL) //Lsk 090629 : when Play D1 video than return menu, reset TV-out setting
        {
	    #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
            tvTVFB_STRIDE= 704/2;
            tvACTSTAEND = TV_ACTSTAEND_PAL_D1; //PAL
        #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
            tvTVFB_STRIDE= 640/2;
            tvACTSTAEND = TV_ACTSTAEND_PAL; //PAL
        #else
            tvTVFB_STRIDE= 640/2;
	        tvACTSTAEND = TV_ACTSTAEND_PAL; //PAL
        #endif
        }
        else if(TvOutMode == SYS_TV_OUT_NTSC)
        {
        #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
            tvTVFB_STRIDE= 704/2;
            tvACTSTAEND = TV_ACTSTAEND_NTSC_D1;
        #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
            tvTVFB_STRIDE= 640/2;
            tvACTSTAEND = TV_ACTSTAEND_NTSC;
        #else
            tvTVFB_STRIDE= 640/2;
			tvACTSTAEND = TV_ACTSTAEND_NTSC;
        #endif
        }
        else if(TvOutMode == SYS_TV_OUT_HD720P)
        {

        }
        else if(TvOutMode == SYS_TV_OUT_FHD1080I)
        {

        }
    }
}

void iduSetVidBufAddr(u8 bufidx, u8 *bufaddr)
{
    switch (bufidx)
    {
        case 0:
            IduVidBuf0Addr = (u32)bufaddr;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            break;

        case 1:
            IduVidBuf1Addr = (u32)bufaddr;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf1Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            break;

        case 2:
            IduVidBuf2Addr = (u32)bufaddr;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf2Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            break;

    }
}

void iduRst(void)
{
    u32 i;

    //Disable IDU
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    IduEna  &= (~(IDU_ENA | IDU_DATA_EN | IDU_YUV420BRI_EN));
#else
    IduEna  &= (~(IDU_ENA | IDU_DATA_EN));
#endif
    //IDU Reset
 #if 1
    IduEna  &= (~IDU_RESET);
    for(i=0; i<50;i++){;}
    IduEna  |= IDU_RESET;
    for(i=0; i<50;i++){;}
    IduEna  &= (~IDU_RESET);
 #endif
}


/*BJ 0523 S*/
void iduPlaybackFrame(u8 * PBuf)
{
    IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
    IduVidBuf0Addr= (u32)PBuf;
 #if NEW_IDU_BRI
    BRI_IADDR_Y = IduVidBuf0Addr;
    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
 #endif


 #if ((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_COASIA) || (LCM_OPTION == LCM_TG200Q04) || (LCM_OPTION == LCM_TM024HDH29))
    if(!sysTVOutOnFlag)//Lsk 090810 : HW_BOARD have both pannel and tv-out
    {
            idu_switch();
    }
 #endif
}
/*BJ 0523 E*/
/*
    Lucian: 僅用於Mobile Pannel.
    idu_switch: Refresh image data on LCM
*/
void idu_switch(void)
{
    u32 temp, i, j, k, l;
    if(sysTVOutOnFlag)//Lsk 090810 : HW_BOARD have both pannel and tv-out
        return;

#if (LCM_OPTION == LCM_HX8312)
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004200;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004300;
    iduWaitCmdBusy();
    IduMpuCmd = 0x00004400;
    IduEna |= (IDU_ENA | IDU_DATA_EN);

#elif (LCM_OPTION == LCM_COASIA)
    IduMpuCmdCfg    = 0x00000020;
    IduMpuCmd       = 0x00000021;
    IduMpuCmdCfg    = 0x00000021;
    IduMpuCmd       = 0x00000000;
    IduMpuCmdCfg    = 0x00000020;
    IduMpuCmd       = 0x00000022;
    IduEna |= (IDU_ENA | IDU_DATA_EN);

#elif(LCM_OPTION == LCM_TG200Q04)
    #if(PANEL_SELECT == 2)  //TG200Q00 Panel
		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		gpioSetLevel(3, 5, 0);
		IduMpuCmd = 0x00000020;
		iduWaitCmdBusy();
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		//IduMpuCmd = 0x00000000;//Normal, Turn Left
		IduMpuCmd = 0x000000AF; //Turn Right
		iduWaitCmdBusy();

		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000021;
		iduWaitCmdBusy();
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		IduMpuCmd = 0x00000000;  //Normal, Turn Right
		//IduMpuCmd = 0x000000DB;  //Turn Left
		iduWaitCmdBusy();

		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000022;
		iduWaitCmdBusy();
	    	//iduDelay(100);// delay 10 ms
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    #elif(PANEL_SELECT == 1)  //TG200Q23 Panel
		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		gpioSetLevel(3, 5, 0);
		IduMpuCmd = 0x0000002A;
		iduWaitCmdBusy();
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		//IduMpuCmd = 0x000000AF;
		IduMpuCmd = 0x000000DB;
		iduWaitCmdBusy();

		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x0000002B;
		iduWaitCmdBusy();
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		//IduMpuCmd = 0x000000DB;
		IduMpuCmd = 0x000000AF;
		iduWaitCmdBusy();


		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x0000002C;
		iduWaitCmdBusy();
	    	//iduDelay(100);// delay 10 ms
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;

    #else  //TG200Q04 Panel
		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		gpioSetLevel(3, 5, 0);
		IduMpuCmd = 0x00000020;
		iduWaitCmdBusy();
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		//IduMpuCmd = 0x00000000;//Normal, Turn Left
		IduMpuCmd = 0x000000AF; //Turn Right
		iduWaitCmdBusy();
		//gpioSetLevel(3, 4, 1);
		//iduDelay(200);// delay 10 ms

		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		///gpioSetLevel(3, 4, 0);
		IduMpuCmd = 0x00000021;
		iduWaitCmdBusy();
		//iduDelay(100);// delay 10 ms
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		IduMpuCmd = 0x00000000;
		iduWaitCmdBusy();
		//iduDelay(100);// delay 10 ms
		IduMpuCmd = 0x00000000;  //Normal, Turn Right
		//IduMpuCmd = 0x000000DB;  //Turn Left
		iduWaitCmdBusy();
		///gpioSetLevel(3, 4, 1);
		//iduDelay(200);// delay 10 ms


		IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
		//gpioSetLevel(3, 4, 0);
		//IduMpuCmd = 0x00000000;
		//iduWaitCmdBusy();
		IduMpuCmd = 0x00000022;
		iduWaitCmdBusy();
		///gpioSetLevel(3, 4, 1);
	    	//iduDelay(100);// delay 10 ms
		IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
    #endif
	IduEna |= (IDU_ENA | IDU_DATA_EN);
#endif

#if (LCM_OPTION == LCM_TM024HDH29)
    gpioSetLevel(3, 4, 1);
    IduWinCtrl |= 0x00000281 ;
    IduDispCfg = 0x00880232 ;
    IduVidBufStride = 0x000000a0;

    gpioSetLevel(3, 4, 0);
    IduMpuCmdCfg = 0x00008060 ;
	IduMpuCmd = 0x0000002c;
	iduWaitCmdBusy();

    IduEna = 0x00000012;
#endif

}



/*
Routine Description:

    Idu Video Display Control.

Arguments:

	ucCtrl - Control index. 0->Disable, 1->Enable.

Return Value:

	None.

*/
void iduVdoCtrl(u8 ucCtrl)
{

	if (ucCtrl == GLB_ENA)
    {
        IduWinCtrl |= IDU_CTL_VDO_ENA;
    }
	else
    {
        IduWinCtrl &= ~IDU_CTL_VDO_ENA;
        IduDefBgColor = 0x00808000;
    }
}



s32 iduSwitchPreview_TV(int src_W,int src_H)
{
    u32 temp,i;

    #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
      TvOutMode=SYS_TV_OUT_HD720P;
      TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
      TvOutMode=SYS_TV_OUT_FHD1080I;
      TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
    #else
 	  TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);
    #endif

#if( (Sensor_OPTION != Sensor_CCIR601) && (Sensor_OPTION  != Sensor_CCIR656) ) /*sensor input 640*480*/
    if(sysTVOutOnFlag && (TvOutMode==SYS_TV_OUT_PAL) )
    {   // PAL mode, 上下清為黑邊.
        IduVideo_ClearPartBuf(0,61440);   		//top part
       IduVideo_ClearPartBuf(675840,737280);   //bottom part
    }
#endif

#if ((HW_BOARD_OPTION==ROULE_DOORPHONE)||(HW_BOARD_OPTION == ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N))
    if(uiGetMenuMode() == STANDBY_MODE)
    {
        IduEna &= ~IDU_ENA;
        IduVidBuf0Addr= (u32)pucClockImageBuf;
        IduVidBuf1Addr= (u32)pucClockImageBuf;
        IduVidBuf2Addr= (u32)pucClockImageBuf;

    }
    else
    {
        IduVidBuf0Addr= (u32)PKBuf0;
        IduVidBuf1Addr= (u32)PKBuf1;
        IduVidBuf2Addr= (u32)PKBuf2;
    }
#else
    IduVidBuf0Addr= (u32)PKBuf0;
    IduVidBuf1Addr= (u32)PKBuf1;
    IduVidBuf2Addr= (u32)PKBuf2;
#endif

    temp         = tvFRAME_CTL;
#if( HW_BOARD_OPTION == MR6730_AFN )
    temp         = TV_FRMCTL_TRIPLE;
	
#else
    temp         = TV_FRMCTL_VDO_EN    |
                   TV_FRMCTL_TRIPLE;
#endif
    temp        &= (~0x00003000);
    temp        |= TV_FRMCTL_FB_SEL2;
    tvFRAME_CTL = temp;


	#if( HW_BOARD_OPTION == MR6730_AFN )
	if(Main_Init_Ready)
	{ 	  
	//we need the OSD0 to be colorbar as a black screen

		if(UI_Get_OpMode() == MODE_PREVIEW)
		{
			if(gTVColorbarEn)
			{
				tvFRAME_CTL |= TV_FRMCTL_OSD0_EN;//OSD0_EN
				DBG((">>)) OSD0_EN ((<<\n")); 		  
			} 	  
		}   

	}
	#endif


  #if TV_DISP_BY_IDU
      #if(TV_DISP_BY_TV_INTR)
         tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
      #else
         tvTVE_INTC   =TV_INTC_FRMEND__ENA;    //TV interrupt control *
      #endif
  #else
      tvTVE_INTC   =TV_INTC_ALL_DISA;    //TV interrupt control *
  #endif


  #if(CHIP_OPTION == CHIP_A1013A)
      tvFIFO_TH = 0x00002104;
  #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) )
      tvFIFO_TH = 0x40008401;
  #elif( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
      tvFIFO_TH = 0x40008501;
  #else
      tvFIFO_TH = 0x00002104;
  #endif


  #if ( IDUTV_DISPLAY_PN && ((CHIP_OPTION == CHIP_A1013A) || \
    (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) ||(CHIP_OPTION == CHIP_A1020A)) ||\
    (CHIP_OPTION == CHIP_A1026A))
     #if NEW_IDU_BRI
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;  //FPGA performance issue
                #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
                #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
                #endif
                }
                else
                {
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
                }

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }
           else
           {
                if(src_H > 640)
                {
                #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;  //FPGA performance issue
                #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
                #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W; // 1280
                #endif
                }
                else
                {
                    BRI_IN_SIZE =((src_H/2)<<16) | src_W;
                }

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
           #else
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           #endif
           }
           else
           {
              BRI_STRIDE = src_W;
           }

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P )

           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
               BRI_IN_SIZE =(src_H<<16) | 1024; // 1280
           #else
               BRI_IN_SIZE =(src_H<<16) | src_W; // 1280
           #endif
           }
           else
           {
               BRI_IN_SIZE =(src_H<<16) | src_W;
           }

           BRI_OUT_SIZE =(720<<16) | 1280;

           if(src_H > 640)
           {
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
           }
           else
           {
              BRI_STRIDE = src_W;
           }

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

         #elif( TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I )
           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
               BRI_IN_SIZE =((src_H/2)<<16) | 1024; // 1280
           #else
               BRI_IN_SIZE =((src_H/2)<<16) | src_W; // 1280
           #endif
           }
           else
           {
               BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           }

           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif

           if(src_H > 640)
           {
              BRI_STRIDE = BRI_BFA_DIV1 | (src_W);
           }
           else
           {
              BRI_STRIDE = src_W;
           }

           BRI_IADDR_Y = (unsigned int)PNBuf_Y[0];
           BRI_IADDR_C = (unsigned int)PNBuf_C[0];
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
           tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
         #endif
     #else
         //For A1013A only
     #endif
  #else  //YUV422
       #if NEW_IDU_BRI
         #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_QVGA) )
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
               {
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;

               }

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }
           else
           {
               if(src_H > 640)
               {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                  BRI_IN_SIZE =((src_H/2/2)<<16) | 1024;
               #elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               #else
                  BRI_IN_SIZE =((src_H/2/2)<<16) | src_W;
               #endif
               }
               else
               {
                  BRI_IN_SIZE =((src_H/2)<<16) | src_W;
               }

           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif
           }

           if(src_H > 640)
           {
           #if((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
               BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           #else
               BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2*2);
           #endif
           }
           else
           {
               BRI_STRIDE = src_W*2;
           }

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)

           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =(src_H<<16) | 1024;
           #else
              BRI_IN_SIZE =(src_H<<16) | src_W;
           #endif
           }
           else
           {
              BRI_IN_SIZE =(src_H<<16) | src_W;
           }
           BRI_OUT_SIZE =(720<<16) | 1280;

           if(src_H > 640)
           {
               BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           }
           else
           {
               BRI_STRIDE = src_W*2;
           }

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;

         #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)

           if(src_H > 640)
           {
           #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
              BRI_IN_SIZE =((src_H/2)<<16) | 1024;
           #else
              BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           #endif
           }
           else
           {
              BRI_IN_SIZE =((src_H/2)<<16) | src_W;
           }

           #if SIMU1080I_VIA720P
           BRI_OUT_SIZE =(360<<16) | 1280;
           #else
           BRI_OUT_SIZE =(540<<16) | 1920;
           #endif

           if(src_H > 640)
           {
               BRI_STRIDE = BRI_BFA_DIV1 | (src_W*2);
           }
           else
           {
               BRI_STRIDE = src_W*2;
           }

           BRI_IADDR_Y = (unsigned int )PKBuf0;
           BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
           BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
         #endif
       #endif

       tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
  #endif


	return 1;
}

void iduSwitchNTSCPAL(int Mode)
{
    if( (Mode != TvOutMode) && sysTVOutOnFlag)
    {
       TvOutMode=Mode;
       tvTVE_EN &=  (~TVE_EN);
			#if (HW_BOARD_OPTION==MR6730_AFN)
				OSTimeDly(10);//prevent from red screen when TV-Out
			#else	   
				OSTimeDly(2);    //防止JESMAY and SkySuccess TV out出現藍色、紅色顛倒情況
			#endif 
       TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);

       if(Mode == SYS_TV_OUT_PAL)
       {
       #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
          #if TV_D1_OUT_FULL_HALF
            BRI_OUT_SIZE =(240<<16) | 704;
          #else
          BRI_OUT_SIZE =(288<<16) | 704;
          #endif
       #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
          BRI_OUT_SIZE =(288<<16) | 640;
       #else
          BRI_OUT_SIZE =(240<<16) | 640;
       #endif
       }
       else if(Mode == SYS_TV_OUT_NTSC)
       {
       #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
          BRI_OUT_SIZE =(240<<16) | 704;
       #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
          BRI_OUT_SIZE =(240<<16) | 640;
       #else
          BRI_OUT_SIZE =(240<<16) | 640;
       #endif
       }

       tvTVE_EN |=TVE_EN;

    }
}

void subTVSwitchNTSCPAL(int Mode)
{
    if( (Mode != TvOutMode) && sysTVOutOnFlag)
    {
       TvOutMode=Mode;
       tv2TVE_EN &=  (~TVE_EN);
       OSTimeDly(2);    //防止JESMAY and SkySuccess TV out出現藍色、紅色顛倒情況
       subTV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PREVIEW);


#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) )
       if(Mode == SYS_TV_OUT_PAL)
       {
       #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
          BRI_OUT_SIZE =(288<<16) | 704;
       #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
          BRI_OUT_SIZE =(288<<16) | 640;
       #else
          BRI_OUT_SIZE =(240<<16) | 640;
       #endif
       }
       else if(Mode == SYS_TV_OUT_NTSC)
       {
       #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
          BRI_OUT_SIZE =(240<<16) | 704;
       #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
          BRI_OUT_SIZE =(240<<16) | 640;
       #else
          BRI_OUT_SIZE =(240<<16) | 640;
       #endif
       }
#endif

       tv2TVE_EN |=TVE_EN;

    }
}

/* Lucian:
   On MR6720,A1013A, (SrcWidth,SrcHieght) 是沒有作用的.
   On A1013B, A1016A,因IDU BRI 內含SC Engine,需有(SrcWidth,SrcHieght)等參數.

*/
void iduPlaybackMode(int SrcWidth,int SrcHeight,int SrcStride)
{
     u32 i;
     #if FPGA_TEST_DTV_720P_HDMI
	 sysTVOutOnFlag = 0; // Panel-out
	 #endif

     if(sysTVOutOnFlag) //TV-out
     {
     #if MULTI_CHANNEL_SUPPORT
        #if(MULTI_CHANNEL_SEL & 0x01)
        isuStop();
        ipuStop();
        siuStop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_Stop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_Stop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x08)
        ciu_3_Stop();
        #endif
     #else
        isuStop();
        ipuStop();
        siuStop();
     #endif

    #if 0
        //IduIntCtrl = 0x00000000;    //IDU interrupt control *
        IduWinCtrl &= (~0x0f);
        OSTimeDly(2);
        IduEna = 0;
        BRI_CTRL_REG |= BRI_CTRL_RST;
        i=0;
        BRI_CTRL_REG &= ~BRI_CTRL_RST;
    #endif

    #if SUB_TV_TEST
        subTV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PLAYBACK);
    #endif
        TV_init(TvOutMode,TVOUT_OSDx2_VDOx1,SYS_RUN_PLAYBACK);

        tvTVE_INTC   =0x00000000;    //TV interrupt control *
    #if(TV_DISP_BY_TV_INTR && TV_DISP_BY_IDU)

    #else
        timerPwmCountEnable(3, 0);   //Lucian: disable timer-3.
    #endif

       #if(CHIP_OPTION == CHIP_A1013A)
          tvFIFO_TH = 0x00002103;
       #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
          tvFIFO_TH = 0x40008401;
       #elif( (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
          tvFIFO_TH = 0x40008501;
       #else
          tvFIFO_TH = 0x00002104;
       #endif

        sysTVOutOnFlag = 1;

        IduVidBuf0Addr= (u32)PKBuf0;
        IduVidBuf1Addr= (u32)PKBuf1;
        IduVidBuf2Addr= (u32)PKBuf2;

     #if ( IDUTV_DISPLAY_PN )
        #if NEW_IDU_BRI
         if(SrcHeight>=720)
         {
               if(TvOutMode == SYS_TV_OUT_PAL)
               {
                   #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                      #if TV_D1_OUT_FULL_HALF
                        BRI_OUT_SIZE =(240<<16) | 704;
                      #else
                      BRI_OUT_SIZE =(288<<16) | 704;
                      #endif
                   #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                      BRI_OUT_SIZE =(288<<16) | 640;
                   #else
                      BRI_OUT_SIZE =(240<<16) | 640;
                   #endif

                #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
                      BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride;
                   #else
                      BRI_STRIDE = SrcStride*2;
                   #endif
                #else
                      BRI_IN_SIZE =((SrcHeight/2/2)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride*2;
                   #else
                      BRI_STRIDE = SrcStride*2*2;
                   #endif
                #endif
               }
               else if(TvOutMode == SYS_TV_OUT_NTSC)
               {
                   #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
                      BRI_OUT_SIZE =(240<<16) | 704;
                   #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
                      BRI_OUT_SIZE =(240<<16) | 640;
                   #else
                      BRI_OUT_SIZE =(240<<16) | 640;
                   #endif

               #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
                      BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride;
                   #else
                      BRI_STRIDE = SrcStride*2;
                   #endif
               #else
                      BRI_IN_SIZE =((SrcHeight/2/2)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride*2;
                   #else
                      BRI_STRIDE = SrcStride*2*2;
                   #endif
               #endif
               }
               else if(TvOutMode == SYS_TV_OUT_HD720P)
               {
                      BRI_OUT_SIZE =(720<<16) | 1280;

                      BRI_IN_SIZE =((SrcHeight)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride;
                   #else
                      BRI_STRIDE = SrcStride*2;
                   #endif
               }
               else if(TvOutMode == SYS_TV_OUT_FHD1080I)
               {
                      BRI_OUT_SIZE =(540<<16) | 1920;

                      BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
                   #if (VIDEO_CODEC_OPTION == H264_CODEC)
                      BRI_STRIDE = SrcStride;
                   #else
                      BRI_STRIDE = SrcStride*2;
                   #endif
               }

         }
         else
         {
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
           #if (VIDEO_CODEC_OPTION == H264_CODEC)
              BRI_STRIDE = SrcStride;
           #else
              BRI_STRIDE = SrcStride*2;
           #endif
           }
           else if(TvOutMode == SYS_TV_OUT_NTSC)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
           #if (VIDEO_CODEC_OPTION == H264_CODEC)
              BRI_STRIDE = SrcStride;
           #else
              BRI_STRIDE = SrcStride*2;
           #endif
           }
           else if(TvOutMode == SYS_TV_OUT_HD720P)
           {
              BRI_OUT_SIZE =(720<<16) | 1280;

              BRI_IN_SIZE =((SrcHeight)<<16) | SrcWidth;
           #if (VIDEO_CODEC_OPTION == H264_CODEC)
              BRI_STRIDE = SrcStride;
           #else
              BRI_STRIDE = SrcStride*2;
           #endif
           }
           else if(TvOutMode == SYS_TV_OUT_FHD1080I)
           {
              BRI_OUT_SIZE =(540<<16) | 1920;

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
           #if (VIDEO_CODEC_OPTION == H264_CODEC)
              BRI_STRIDE = SrcStride;
           #else
              BRI_STRIDE = SrcStride*2;
           #endif
           }

         }
         BRI_IADDR_Y = (unsigned int)PKBuf0;
         BRI_IADDR_C = (unsigned int)BRI_IADDR_Y + PNBUF_SIZE_Y;

         if(TvOutMode == SYS_TV_OUT_HD720P)
         {
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
             #else
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
             #endif
         }
         else
         {
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
             #else
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
             #endif
         }

         tvFRAME_CTL |= TV_FRMCTL_VDO_EN    |
					  TV_FRMCTL_FB_SEL0;

		#if (HW_BOARD_OPTION==MR6730_AFN)	

		 	#if(USE_PWR_ONOFF_KEY && PWKEY_PWRCTRL)
		 	if(MACRO_UI_SET_PWSTAT_CODE==UI_SET_PWSTAT_INIT)
		 	{
		 		//NOP
		 	}
			else
			#endif
			{
				tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder
			}

		#else
		
         tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

		#endif
		
        #else
            tvFRAME_CTL =0;
            tvTVE_EN &=  (~IDU_YUV420BRI_EN);  //Lucian: Playback mode always run on YUV422 mode.
            tvTVE_EN |=TVE_EN;    //Trun on TV-Encoder
            tvFRAME_CTL = TV_FRMCTL_VDO_EN    |
    					  TV_FRMCTL_FB_SEL0;
        #endif
     #else
        #if NEW_IDU_BRI
         if(SrcHeight>=720)
         {
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

           #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           #else
              BRI_IN_SIZE =((SrcHeight/2/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2*2;
           #endif
           }
           else if(TvOutMode == SYS_TV_OUT_NTSC)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

           #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           #else
              BRI_IN_SIZE =((SrcHeight/2/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2*2;
           #endif
           }
           else if(TvOutMode == SYS_TV_OUT_HD720P)
           {
              BRI_OUT_SIZE =(720<<16) | 1280;

              BRI_IN_SIZE =((SrcHeight)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }
           else if(TvOutMode == SYS_TV_OUT_FHD1080I)
           {
              BRI_OUT_SIZE =(540<<16) | 1920;

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }

         }
         else
         {
           if(TvOutMode == SYS_TV_OUT_PAL)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              #if TV_D1_OUT_FULL_HALF
                BRI_OUT_SIZE =(240<<16) | 704;
              #else
              BRI_OUT_SIZE =(288<<16) | 704;
              #endif
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(288<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }
           else if(TvOutMode == SYS_TV_OUT_NTSC)
           {
           #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              BRI_OUT_SIZE =(240<<16) | 704;
           #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              BRI_OUT_SIZE =(240<<16) | 640;
           #else
              BRI_OUT_SIZE =(240<<16) | 640;
           #endif

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }
           else if(TvOutMode == SYS_TV_OUT_HD720P)
           {
              BRI_OUT_SIZE =(720<<16) | 1280;

              BRI_IN_SIZE =((SrcHeight)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }
           else if(TvOutMode == SYS_TV_OUT_FHD1080I)
           {
              BRI_OUT_SIZE =(540<<16) | 1920;

              BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
              BRI_STRIDE = SrcStride*2;
           }

         }
        #endif
         BRI_IADDR_Y = (unsigned int )PKBuf0;
         BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;

         if(TvOutMode == SYS_TV_OUT_HD720P)
         {
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
             #else
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
             #endif
         }
         else
         {
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
             #else
             BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
             #endif
         }
         tvFRAME_CTL |= TV_FRMCTL_VDO_EN    |
					  TV_FRMCTL_FB_SEL0;
         tvTVE_EN |=TVE_EN;    //Trun on TV-Encoder
     #endif

     }
     else //Pannel-out
     {
     #if MULTI_CHANNEL_SUPPORT
        #if(MULTI_CHANNEL_SEL & 0x01)
        isuStop();
        ipuStop();
        siuStop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x02)
        ciu_1_Stop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x04)
        ciu_2_Stop();
        #endif

        #if(MULTI_CHANNEL_SEL & 0x08)
        ciu_3_Stop();
        #endif
     #else
        isuStop();
        ipuStop();
        siuStop();
     #endif



        IduWinCtrl =IDU_CTL_VDO_ENA|
                    IDU_CTL_OSD0_DISA|
                    IDU_CTL_OSD1_DISA|
                    IDU_CTL_OSD2_DISA|
                    IDU_CTL_FBAUTO_DISA|
                    IDU_CTL_IDU_WAIT_DISA |
                #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IDU_CTL_SRC_FMT_SRGB |
                #endif
                #if ( (LCM_OPTION==LCM_P_RGB_888_Innolux) || (LCM_OPTION == LCM_P_RGB_888_HannStar)  || (LCM_OPTION == LCM_P_RGB_888_AT070TN90)||\
                    (LCM_OPTION == LCM_P_RGB_888_SY700BE104) || (LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227)||\
                    (LCM_OPTION == LCM_P_RGB_888_ILI6122) ||(LCM_OPTION == LCM_P_RGB_888_ILI6126C))
				    IDU_CTL_HISH_SPEED_EN |
				#endif
                #if ( (LCM_OPTION == VGA_640X480_60HZ) || (LCM_OPTION == VGA_800X600_60HZ) || (LCM_OPTION == VGA_1024X768_60HZ) || (LCM_OPTION == VGA_1280X800_60HZ) )
				    IDU_CTL_HISH_SPEED_EN |
				#endif
                    IDU_CTL_FB_SEL_0;

        IduIntCtrl  = 0;

        IduVidWinStart = 0x00000000;
        IduVidWinEnd = ((PANNEL_X-1)<<IDU_VDO_WSX_SHFT )|((PANNEL_Y-1)<<IDU_VDO_WSY_SHFT);

    #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
        IduVidBufStride = PANNEL_X/4;
    #else
        IduVidBufStride = PANNEL_X/2;
    #endif
        IduCrcCtrl = 0x00000000;


    #if NEW_IDU_BRI
        IduFifoThresh = 0xf00a8304;
    #else
        IduFifoThresh = 0xc0ca7304;
    #endif

     #if(TV_DISP_BY_TV_INTR  && TV_DISP_BY_IDU)

     #else
        #if(HW_BOARD_OPTION == SUNWAY_SDV)
           timerPwmCountEnable(4, 0);   //Lucian: disable timer-4.
        #else
            timerPwmCountEnable(3, 0); //Lucian: disable timer-3.
        #endif
     #endif
            IduWinCtrl = (IduWinCtrl & ~0x00003000);
     #if NEW_IDU_BRI
         BRI_OUT_SIZE =(PANNEL_Y<<16) | PANNEL_X;

         BRI_IADDR_Y = (unsigned int )PKBuf0;
         BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;

        #if NEW_IDU_BRI_PANEL_INTLX
         if(SrcHeight>=720)
         {
         #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
             BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
             #else
               BRI_STRIDE = SrcStride*2;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
             #endif
         #else
             BRI_IN_SIZE =((SrcHeight/2/2)<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride*2;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
             #else
               BRI_STRIDE = SrcStride*2*2;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
             #endif
         #endif
         }
         else
         {
             BRI_IN_SIZE =((SrcHeight/2)<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT420;
             #else
               BRI_STRIDE = SrcStride*2;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_I_SCAN | BRI_CTRL_DAT422;
             #endif
         }
        #else
         if(SrcHeight>=720)
         {
         #if( (CHIP_OPTION ==CHIP_A1018A) || (CHIP_OPTION ==CHIP_A1018B) )
               BRI_IN_SIZE =(SrcHeight<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
             #else
               BRI_STRIDE = SrcStride*2;
    		   #if (LCM_OPTION == LCM_sRGB_HD15_HDMI)
        	     BRI_CTRL_REG = BRI_CTRL_SC_DISA | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #else
                 BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #endif
             #endif
         #else
               BRI_IN_SIZE =( (SrcHeight/2)<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride*2;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
             #else

               BRI_STRIDE = SrcStride*2*2;
    		   #if (LCM_OPTION == LCM_sRGB_HD15_HDMI)
        	     BRI_CTRL_REG = BRI_CTRL_SC_DISA | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #else
                 BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #endif
             #endif
         #endif
         }
         else
         {
               BRI_IN_SIZE =(SrcHeight<<16) | SrcWidth;
             #if (VIDEO_CODEC_OPTION == H264_CODEC)
               BRI_STRIDE = SrcStride;
               BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT420;
             #else
               BRI_STRIDE = SrcStride*2;
    		   #if (LCM_OPTION == LCM_sRGB_HD15_HDMI)
        	     BRI_CTRL_REG = BRI_CTRL_SC_DISA | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #else
                 BRI_CTRL_REG = BRI_CTRL_SC_EN | BRI_CTRL_P_SCAN | BRI_CTRL_DAT422;
    		   #endif
             #endif
         }
        #endif
     #endif

     #if((LCM_OPTION == LCM_HX8312) || (LCM_OPTION == LCM_TG200Q04))
        IduEna = IDU_ENA | IDU_SINGLE | IDU_DATA_EN;  //on LCM,we support SINGLE mode
     #else
        IduEna = IDU_ENA | IDU_DATA_EN;  //on LCD, Dont use SINGLE mode
     #endif
     }



#if AUDIO_IN_TO_OUT
    iisPreviewI2OEnd();
#endif

#if AUDIO_BYPASS
    Close_IIS_ALC5621();
#endif

}

void idu_Stop_Get_Data(void)
{
    IduDefBgColor   = 0x00808000;
    IduWinCtrl &=~ IDU_CTL_VDO_ENA;
}
/* Lucian 070515 E*/
#if(LCM_OPTION == LCM_HX5073_RGB)
void idu5073Config(void)
{
    Gpio3Ena=0x000001C0;      iduDelay(200);
    Gpio3Dir=0x00;            iduDelay(200);
    Gpio3Level=0x00000100;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000080;    iduDelay(200);
    Gpio3Level=0x000000C0;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000080;    iduDelay(200);
    Gpio3Level=0x000000C0;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000040;    iduDelay(200);
    Gpio3Level=0x00000000;    iduDelay(200);
    Gpio3Level=0x00000100;    iduDelay(200);

}

void idu5073Config_YUV(void)
{
    Gpio3Ena=0x000001C0;      iduDelay(200);
    Gpio3Dir=0x00;            iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000080;iduDelay(200);
Gpio3Level=0x000000C0;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000080;iduDelay(200);
Gpio3Level=0x000000C0;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000100;iduDelay(200);
Gpio3Level=0x00000100;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000080;iduDelay(200);
Gpio3Level=0x000000C0;iduDelay(200);
Gpio3Level=0x00000080;iduDelay(200);
Gpio3Level=0x000000C0;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000040;iduDelay(200);
Gpio3Level=0x00000000;iduDelay(200);
Gpio3Level=0x00000100;iduDelay(200);

}





void idu5073Gamma(void)
{
//  Gpio3Ena=0x000001C0;      iduDelay(200);
//  Gpio3Dir=0x00;            iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000080;iduDelay(200);
    Gpio3Level=0x000000C0;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000040;iduDelay(200);
    Gpio3Level=0x00000000;iduDelay(200);
    Gpio3Level=0x00000100;iduDelay(200);

    Gpio3Level=0x00000100; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000100; iduDelay(200);
    Gpio3Level=0x00000100; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000100; iduDelay(200);
    Gpio3Level=0x00000100; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000080; iduDelay(200);
    Gpio3Level=0x000000C0; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000040; iduDelay(200);
    Gpio3Level=0x00000000; iduDelay(200);
    Gpio3Level=0x00000100; iduDelay(200);

}

#endif


void iduSetVBuff(u8* buf1 , u8 * buf2 , u8 * buf3)
{
    IduVidBuf0Addr = (u32)buf1;
    IduVidBuf1Addr = (u32)buf2;
    IduVidBuf2Addr = (u32)buf3;
}


void iduSetVideoBuf0Addr(u8* buf)
{
    IduVidBuf0Addr = (u32)buf;

#if NEW_IDU_BRI
    BRI_IADDR_Y = IduVidBuf0Addr;
    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
#endif
}


/*
  Lucian: 目前僅考慮到 preview/video mode, TV-out 解析度為640x480,
          playback時,TV-out 可為D1/VGA.

*/

void TV_reset()
{
     if(TvOutMode == SYS_TV_OUT_PAL) //Lsk 090629 : when Play D1,QVGA video than return menu, reset TV-out setting
     {
     #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
        tvTVFB_STRIDE= 704/2;
        tvACTSTAEND = TV_ACTSTAEND_PAL_D1;
     #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
        tvTVFB_STRIDE= 640/2;
        tvACTSTAEND = TV_ACTSTAEND_PAL;
     #else
        tvTVFB_STRIDE= 640/2;
        tvACTSTAEND = TV_ACTSTAEND_PAL;
     #endif
     }
     else if(TvOutMode == SYS_TV_OUT_NTSC)
     {
     #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
        tvTVFB_STRIDE= 704/2;
        tvACTSTAEND = TV_ACTSTAEND_NTSC_D1;
     #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
        tvTVFB_STRIDE= 640/2;
        tvACTSTAEND = TV_ACTSTAEND_NTSC;
     #else
        tvTVFB_STRIDE= 640/2;
        tvACTSTAEND = TV_ACTSTAEND_NTSC;
     #endif
     }
     tvTVE_EN &= ~(TVE_VX2);
     #if OSD_SIZE_X2_DISABLE
     tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL);
     #else
     tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
     #endif


}
void TV_init(u8 TVOutMode,u32 DivFac,u8 RunMode)
{
    int i;
    u32 temp;
	static u8	ucFirst = 1;

#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from sysclk
      SYS_DDR_PLLCTL=0x5a04c001;  // 24* 90/4=540 MHz
    #if(CHIP_OPTION == CHIP_A1018A)
      SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00040000; // 540/5=108, 108/4=27
    #elif(CHIP_OPTION == CHIP_A1018B)
      SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00090000; // 540/10=54, 54/2=27
    #endif
      for(i=0 ; i<200 ; i++);
      SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL

      SYS_ANA_TEST2 = (SYS_ANA_TEST2 & (~0x018)) | 0x000;  //switch to TV mode of DAC
#endif

    //--------Run First only------//
	if (ucFirst == 1)
	{
	    //TV encoder reset
	    tvTVE_EN=0x00000000;
	    tvTVE_EN |=TV_RST;
	    for(i=0;i<0xfff;i++);
	    tvTVE_EN &= (~TV_RST);

    #if NEW_IDU_BRI

    #else
	    //Reset TV-Encoder
	    SYS_RSTCTL &= ~(0x08000000);
	    SYS_RSTCTL |= 0x08000000;
	    for(i=0;i<10;i++);
	    SYS_RSTCTL &= ~(0x08000000);
    #endif

		ucFirst = 0;
	}
    //---------------------------//


    if(TVOutMode == SYS_TV_OUT_PAL)//PAL
    {
          temp=tvTVE_EN;
     #if OSD_SIZE_X2_DISABLE
          temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
     #else
          temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
     #endif

          if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
          {
            temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
          }
	      else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
	      {
     #if OSD_SIZE_X2_DISABLE
            temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
     #else
    	    temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
     #endif
	      }
          else    // =1, osd x2
          {
     #if OSD_SIZE_X2_DISABLE
            temp |= (TVDAC_POWON | TVMODE_SEL);
     #else
            temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
     #endif
          }

		  tvTVE_EN=temp;
		  //DEBUG_IDU("Trace: PAL - tvTVE_EN=%#x\n",tvTVE_EN);


     #if(TV_DigiOut_SEL == TV_DigiOut_601)
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW | TV_CLPF_ON;
          #else
          tvTV_CONF    = 0x08500440 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW | TV_CLPF_ON;
          #endif
         #else
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW | TV_CLPF_ON;
         #endif
     #elif(TV_DigiOut_SEL == TV_DigiOut_YUV)
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tvTV_CONF    = 0x08500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
         #endif
     #else //656
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tvTV_CONF    = 0x08500440 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tvTV_CONF    = 0x28500440 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
         #endif
     #endif

     #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
          tvTV_CONF = (tvTV_CONF & (~0xf0000000) ) | 0x80000000;
     #endif
          tvTV_CTRL    =0x03FF0AA7;



      #if TVOUT_CRYSTAL_24MHZ
              tvCORING     =0x00000000;    //Coring threshold
              tvHTOTAL     =0x00000600;    //Htotal=1728/2=864 pixel/line
              tvCSYNCCFG   =0x008000d4;    //Hsync config: height,incremental value
              tvBURSTCFG   =0x00260065;    //Color burst config: height,incremental value
              tvBLANKCFG   =0x001400fc;    //Blank config: Blank level value,setup value
              tvVACTSTAEND = ((288-1)<<16) | 0;
              tvBURSTSTAEND=0x00bc0086;    //Color burst start/end position
              tvACTSTAEND = TV_ACTSTAEND_PAL;
              tvHSYNC_WIDTH = 0x70 | ((tvHTOTAL-(tvACTSTAEND>>16)+(tvACTSTAEND & 0x0ffff))<<16);
              tvYUVGAIN_REG=TV_YUV_GAIN_PAL;    //RGB Gain
              tvSUB_CAR_FR =0x2F4a25cb;    //subcarrier freq.

          #if(TV_D1_OUT_FULL)
              tvVACTSTAEND = ((288-1)<<16) | 0;
          #else
              tvVACTSTAEND = ((267-1)<<16) | 27;
          #endif
      #else
              tvCORING     =0x00000000;    //Coring threshold
              tvHTOTAL     =0x000006C0;    //Htotal=1728/2=864 pixel/line
              tvCSYNCCFG   =0x008000fc;    //Hsync config: height,incremental value
              tvBURSTCFG   =0x00260065;    //Color burst config: height,incremental value
              tvBLANKCFG   =0x000000fc;    //Blank config: Blank level value,setup value
            #if(TV_D1_OUT_FULL )
              #if TV_D1_OUT_FULL_HALF
                tvVACTSTAEND = ((267-1)<<16) | 27;
              #else
              tvVACTSTAEND = ((288-1)<<16) | 0;
              #endif
            #else
              tvVACTSTAEND = ((267-1)<<16) | 27;
            #endif

              tvBURSTSTAEND=0x00d40097;    //Color burst start/end position
          #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
              tvACTSTAEND = TV_ACTSTAEND_PAL_D1;
          #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
              tvACTSTAEND = TV_ACTSTAEND_PAL;
          #else
              tvACTSTAEND = TV_ACTSTAEND_PAL;
          #endif
              tvHSYNC_WIDTH = 0x7e | ((tvHTOTAL-(tvACTSTAEND>>16)+(tvACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
              tvYUVGAIN_REG=TV_YUV_GAIN_PAL;    //RGB Gain
              tvSUB_CAR_FR =0x2a090eb0;    //subcarrier freq.
      #endif

      #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
          tvTVFB_STRIDE= 704/2;
      #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
          tvTVFB_STRIDE= 640/2;
      #else
          tvTVFB_STRIDE= 640/2;
      #endif

          tvDACVAL     =0x01500020;    //for DAC test,y,uv out
          tvDACVAL1    =0x00000005;    //for DAC test,cvbs out

        #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
          tvBT656CONF  = TV_656_FIX_ENA  | TV_656_SUB_OUT;
        #else
          tvBT656CONF  = TV_656_FIX_ENA  | TV_656_MAIN_OUT;
        #endif
        
        #if(TV_BT656_54M_ENA)
          tvBT656CONF  |= TV_656_MUX_ENA;
          SYS_DDR_PLLCTL |= 0x01; /* use internal PLL clk */
        #else
          tvBT656CONF  |= TV_656_MUX_DIS;
        #endif
        
          tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
          tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
          tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
          tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black
    }
    else if(TVOutMode == SYS_TV_OUT_NTSC)
    {
        temp=tvTVE_EN;
    #if OSD_SIZE_X2_DISABLE
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
    #else
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
    #endif
        if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
        {
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
        }
        else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
        {
    #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
    #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
    #endif

        }
        else    // =1, osd x2
        {
    #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL );
    #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
    #endif
        }

        tvTVE_EN=temp;
        //DEBUG_IDU("Trace: NTSC - tvTVE_EN=%#x\n",tvTVE_EN);

     #if(TV_DigiOut_SEL == TV_DigiOut_601)
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tvTV_CONF    = 0x08500442 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_601 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
         #endif
     #elif(TV_DigiOut_SEL == TV_DigiOut_YUV)
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tvTV_CONF    = 0x08500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
         #endif
     #else //656
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tvTV_CONF    = 0x08500442 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tvTV_CONF    = 0x28500442 | TV_DV_SEL_656_MODE1 | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
         #endif
     #endif

     #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A) )
          tvTV_CONF = (tvTV_CONF & (~0xf0000000) ) | 0x80000000;
     #endif

        tvTV_CTRL    =0x03FF0AA5;

     #if TVOUT_CRYSTAL_24MHZ
            tvCORING     =0x00000000;    //Coring threshold
            tvHTOTAL     =0x000005f5;    //Htotal=1716/2=858 pixel/line
            tvCSYNCCFG   =0x004000ce;    //Hsync config: height,incremental value
            tvBURSTCFG   =0x00240060;    //Color burst config: height,incremental value
            tvBLANKCFG   =0x002800ce;    //Blank config: Blank level value,setup value
            tvBURSTSTAEND=0x00bc0080;    //Color burst start/end position
            tvVACTSTAEND = ((240-1)<<16) | 0;
            tvACTSTAEND = TV_ACTSTAEND_NTSC;
            tvHSYNC_WIDTH = 0x70 | ((tvHTOTAL-(tvACTSTAEND>>16)+(tvACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
            tvYUVGAIN_REG=TV_YUV_GAIN;    //RGB Gain
            tvSUB_CAR_FR =0x262e78cc;    //subcarrier freq.
        #if(TV_D1_OUT_FULL)
            tvACTSTAEND = TV_ACTSTAEND_NTSC;
        #else
            tvACTSTAEND = TV_ACTSTAEND_NTSC;
        #endif
     #else
            tvCORING     =0x00000000;    //Coring threshold            tvHTOTAL     =0x000006b4;    //Htotal=1716/2=858 pixel/line
            tvHTOTAL     =0x000006b4;    //Htotal=1716/2=858 pixel/line
            tvCSYNCCFG   =0x008000d4;    //Hsync config: height,incremental value
            tvBURSTCFG   =0x00280072;    //Color burst config: height,incremental value
            tvBLANKCFG   =0x001C00e0;    //Blank config: Blank level value,setup value
            tvBURSTSTAEND=0x00d40090;    //Color burst start/end position
            tvVACTSTAEND = ((240-1)<<16) | 0;
        #if(TV_D1_OUT_FULL )
            tvACTSTAEND = TV_ACTSTAEND_NTSC_D1;
        #else
            tvACTSTAEND = TV_ACTSTAEND_NTSC;
        #endif
            tvHSYNC_WIDTH = 0x7e | ((tvHTOTAL-(tvACTSTAEND>>16)+(tvACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
            tvYUVGAIN_REG=TV_YUV_GAIN;    //RGB Gain
            tvSUB_CAR_FR =0x21f07c20;    //subcarrier freq.
     #endif

     #if(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==0) )
        tvTVFB_STRIDE= 704/2;
     #elif(TV_D1_OUT_FULL && (TVOUT_CRYSTAL_24MHZ==1) )
        tvTVFB_STRIDE= 640/2;
     #else
        tvTVFB_STRIDE= 640/2;
     #endif

        tvDACVAL     =0x01500020;    //for DAC test,y,uv out
        tvDACVAL1    =0x00000005;    //for DAC test,cvbs out

    #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
        tvBT656CONF  = TV_656_FIX_ENA  | TV_656_SUB_OUT;
    #else
        tvBT656CONF  = TV_656_FIX_ENA  | TV_656_MAIN_OUT;
    #endif
    
    #if(TV_BT656_54M_ENA)
        tvBT656CONF  |= TV_656_MUX_ENA;
        SYS_DDR_PLLCTL |= 0x01; /* use internal PLL clk */
    #else
        tvBT656CONF  |= TV_656_MUX_DIS;
    #endif      

        tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
        tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
        tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
        tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black
    }

 #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    else if(TVOutMode == SYS_TV_OUT_HD720P)
    {
        tvFRAME_CTL=0x0;
        for(i=0;i<0xff;i++);
        tvTVE_EN=0x00000000;
	    tvTVE_EN |=TV_RST;

	    tvTVE_EN &= (~TV_RST);
    #if((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
        //----Set TV PLL clock=74.25 MHz.----//
        SYS_TV_PLLCTL = 0x00026237 ;
        for(i=0 ; i<200 ; i++);
        SYS_TV_PLLCTL = 0x00036237 ;
        SYS_TV_PLLCTL = 0x0001e237 ;
    #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from sysclk
        SYS_DDR_PLLCTL=0x6304c001;  // 24*99/4=594
        #if(CHIP_OPTION == CHIP_A1018A)
        SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00010000;  //594/2=297, 297/4=74.25
        #elif(CHIP_OPTION == CHIP_A1018B)
        SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00030000;  //594/4=148.5, 148.5/2=74.25
        #endif
        for(i=0 ; i<200 ; i++);
        SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL
    #else

    #endif

        temp=tvTVE_EN;
  #if OSD_SIZE_X2_DISABLE
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
        {
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
        }
        else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        }
        else    // =1, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
  #endif

        }

        tvTVE_EN=temp;
        //-------//
        //GpioActFlashSelect |= GPIO_DISP_D8_15_EXT ;
        tvBT656CONF  = TV_656_FIX_ENA | TV_656_MUX_DIS | TV_656_MAIN_OUT;

    #if IDUTV_DISPLAY_PN
      #if NEW_IDU_BRI
        tvTV_CONF    = 0x2d6020f1;
      #else
        tvTV_CONF    = 0x0d6020f1;
      #endif
    #else
        tvTV_CONF    = 0x2d6020f1;
    #endif

    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
        tvTV_CONF = (tvTV_CONF & (~0xf0000000) ) | 0x80000000;
    #endif

        tvTV_CTRL = 0x03ff22a2;
        tvCORING = 0x00000000;
        tvHTOTAL = 0x00000672;
        tvCSYNCCFG = 0x0c2800ec;
        tvBURSTCFG = 0x00280072;
        tvBLANKCFG = 0x000000ec;
    	tvHSYNC_WIDTH = 0x01720028;

        tvVACTSTAEND = 0x02d00000;
        tvBURSTSTAEND = 0x00d40090;
    	tvACTSTAEND = 0x06010101;

        tvTVFB_STRIDE = 0x00000280;
        tvYUVGAIN_REG = TV_YUV_GAIN;
        tvSUB_CAR_FR = 0x21f07c20;
        tvDACVAL = 0x01500020;
        tvDACVAL1 = 0x00000005;

        tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
        tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
        tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
        tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black
    }
    else if(TVOutMode == SYS_TV_OUT_FHD1080I)
    {
        tvFRAME_CTL=0x0;
        for(i=0;i<0xff;i++);
        tvTVE_EN=0x00000000;
	    tvTVE_EN |=TV_RST;

	    tvTVE_EN &= (~TV_RST);
    #if((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
        //----Set TV PLL clock=74.25 MHz.----//
        SYS_TV_PLLCTL = 0x00026237 ;
        for(i=0 ; i<200 ; i++);
        SYS_TV_PLLCTL = 0x00036237 ;
        SYS_TV_PLLCTL = 0x0001e237 ;
    #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
        SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from sysclk
        SYS_DDR_PLLCTL=0x6304c001;
        #if(CHIP_OPTION == CHIP_A1018A)
        SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00010000;  //594/2=297, 297/4=74.25
        #elif(CHIP_OPTION == CHIP_A1018B)
        SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00030000;  //594/4=148.5, 148.5/2=74.25
        #endif

        for(i=0 ; i<200 ; i++);
        SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL
    #else

    #endif

        temp=tvTVE_EN;
  #if OSD_SIZE_X2_DISABLE
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
        {
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
        }
        else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        }
        else    // =1, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
  #endif

        }

        tvTVE_EN=temp;
        //-------//
        //GpioActFlashSelect |= GPIO_DISP_D8_15_EXT ;
        tvBT656CONF  = TV_656_FIX_ENA | TV_656_MUX_DIS | TV_656_MAIN_OUT;

    #if IDUTV_DISPLAY_PN
      #if NEW_IDU_BRI
        tvTV_CONF    = 0x2e6020f1;
      #else
        tvTV_CONF    = 0x0e6020f1;
      #endif
    #else
        tvTV_CONF    = 0x2e6020f1;
    #endif

    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
        tvTV_CONF = (tvTV_CONF & (~0xf0000000) ) | 0x80000000;
    #endif

        tvTV_CTRL = 0x03ff22a2;
        tvCORING = 0x00000000;
        tvHTOTAL = 0x00000898;
        tvCSYNCCFG = 0x0c2800ec;
        tvBURSTCFG = 0x00280072;
        tvBLANKCFG = 0x000000ec;
    	tvHSYNC_WIDTH = 0x01180028;

    #if SIMU1080I_VIA720P
        tvVACTSTAEND = 0x01680000;
    	tvACTSTAEND = 0x06010101;
    #else
        tvVACTSTAEND = 0x021C0000;
        tvACTSTAEND = 0x084100c1;
    #endif
        tvBURSTSTAEND = 0x00d40090;

        tvTVFB_STRIDE = 0x000003c0;
        tvYUVGAIN_REG = TV_YUV_GAIN;
        tvSUB_CAR_FR = 0x21f07c20;
        tvDACVAL = 0x01500020;
        tvDACVAL1 = 0x00000005;

        tvWHITEYEL   =0x01ee00fe;    //Color bar config: white,yellow
        tvCYANGRN    =0x03ce02de;    //Color bar config: cyan,green
        tvMAGRED     =0x05ae04be;    //Color bar config: magenta, red
        tvBLUEBLACK  =0x078e069e;    //Color bar config: blue,black
    }
  #endif

    if(gTVColorbarEn==1)
    {
        tvYUVGAIN_REG=0;
    }
}


void subTV_init(u8 TVOutMode,u32 DivFac,u8 RunMode)
{
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    int i;
    u32 temp;
	static u8	ucFirst = 1;

	if (ucFirst == 1)
	{
	    //TV encoder reset
	    tv2TVE_EN  =0x00000000;
	    tv2TVE_EN |=TV_RST;
	    for(i=0;i<0xfff;i++);
	    tv2TVE_EN &= (~TV_RST);
		ucFirst = 0;
	}

    if(TVOutMode == SYS_TV_OUT_PAL)//PAL
    {
          temp=tv2TVE_EN;
    #if OSD_SIZE_X2_DISABLE
          temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 );
    #else
          temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
    #endif
          if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
          {
            temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
          }
	      else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
	      {
    #if OSD_SIZE_X2_DISABLE
    	    temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 );
    #else
    	    temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
    #endif

	      }
          else    // =1, osd x2
          {
    #if OSD_SIZE_X2_DISABLE
            temp |= (TVDAC_POWON | TVMODE_SEL);
    #else
            temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
    #endif
          }

		  tv2TVE_EN=temp;

     #if(TV_DigiOut_SEL == TV_DigiOut_601)
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
            tv2TV_CONF    = 0x08500440 | TV_DV_SEL_601;
          #else
            tv2TV_CONF    = 0x08500440 | TV_DV_SEL_601;
          #endif
         #else
          tv2TV_CONF    = 0x28500440 | TV_DV_SEL_601;
         #endif
     #elif(TV_DigiOut_SEL == TV_DigiOut_YUV)
             #if IDUTV_DISPLAY_PN
              #if NEW_IDU_BRI
              tv2TV_CONF    = 0x08500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
              #else
              tv2TV_CONF    = 0x08500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
              #endif
             #else
              tv2TV_CONF    = 0x28500440 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
             #endif
     #else //656
         #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
            tv2TV_CONF    = 0x08500440 | TV_DV_SEL_656_MODE1;
          #else
            tv2TV_CONF    = 0x08500440 | TV_DV_SEL_656_MODE1;
          #endif
         #else
          tv2TV_CONF    = 0x28500440 | TV_DV_SEL_656_MODE1;
         #endif
     #endif

          tv2TV_CTRL    =0x03FF0AA7;

      #if TVOUT_CRYSTAL_24MHZ
              tv2CORING     =0x00000000;    //Coring threshold
              tv2HTOTAL     =0x00000600;    //Htotal=1728/2=864 pixel/line
              tv2CSYNCCFG   =0x008000d4;    //Hsync config: height,incremental value
              tv2BURSTCFG   =0x00260065;    //Color burst config: height,incremental value
              tv2BLANKCFG   =0x001400fc;    //Blank config: Blank level value,setup value
           #if(TV_D1_OUT_FULL)
              tv2VACTSTAEND = ((288-1)<<16) | 0;
           #else
              tv2VACTSTAEND = ((267-1)<<16) | 27;
           #endif
              tv2BURSTSTAEND=0x00bc0086;    //Color burst start/end position
              tv2ACTSTAEND = TV_ACTSTAEND_PAL;
              tv2HSYNC_WIDTH = 0x70 | ((tvHTOTAL-(tv2ACTSTAEND>>16)+(tv2ACTSTAEND & 0x0ffff))<<16);
              tv2YUVGAIN_REG=0x00808080;    //RGB Gain
              tv2SUB_CAR_FR =0x2F4a25cb;    //subcarrier freq.
      #else
              tv2CORING     =0x00000000;    //Coring threshold
              tv2HTOTAL     =0x000006C0;    //Htotal=1728/2=864 pixel/line
              tv2CSYNCCFG   =0x008000d4;    //Hsync config: height,incremental value
              tv2BURSTCFG   =0x00260065;    //Color burst config: height,incremental value
              tv2BLANKCFG   =0x001400fc;    //Blank config: Blank level value,setup value
              #if(TV_D1_OUT_FULL)
                tv2VACTSTAEND = ((288-1)<<16) | 0;
              #else
                tv2VACTSTAEND = ((267-1)<<16) | 27;
              #endif
              tv2BURSTSTAEND=0x00d40097;    //Color burst start/end position
           #if(TV_D1_OUT_FULL)
              tv2ACTSTAEND = TV_ACTSTAEND_PAL_D1;
           #else
              tv2ACTSTAEND = TV_ACTSTAEND_PAL;
           #endif
              tv2HSYNC_WIDTH = 0x7e | ((tvHTOTAL-(tv2ACTSTAEND>>16)+(tv2ACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
              tv2YUVGAIN_REG=0x00808080;    //RGB Gain
              tv2SUB_CAR_FR =0x2A0925CB;    //subcarrier freq.
      #endif

      #if(TV_D1_OUT_FULL)
          tv2TVFB_STRIDE=704/2;    // Video Buffer stride = 704
      #else
          tv2TVFB_STRIDE=640/2;    // Video Buffer stride = 640
      #endif
          tv2DACVAL     =0x01500020;    //for DAC test,y,uv out
          tv2DACVAL1    =0x00000005;    //for DAC test,cvbs out

          tv2WHITEYEL   =0x019e00fe;    //Color bar config: white,yellow
          tv2CYANGRN    =0x02de023e;    //Color bar config: cyan,green
          tv2MAGRED     =0x041e037e;    //Color bar config: magenta, red
          tv2BLUEBLACK  =0x055e04be;    //Color bar config: blue,black
          tv2TOFB_STRIDE = 0;
    }
    else if(TVOutMode == SYS_TV_OUT_NTSC)
    {

        temp=tv2TVE_EN;
  #if OSD_SIZE_X2_DISABLE
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
        temp &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        if(DivFac==TVOUT_OSDx1_VDOx2) //video x2
        {
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
        }
        else if(DivFac==TVOUT_OSDx2_VDOx2) //video x2, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
  #endif
        }
        else    // =1, osd x2
        {
  #if OSD_SIZE_X2_DISABLE
           temp |= (TVDAC_POWON | TVMODE_SEL);
  #else
           temp |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
  #endif
        }

        tv2TVE_EN=temp;

     #if(TV_DigiOut_SEL == TV_DigiOut_601)
       #if IDUTV_DISPLAY_PN
        #if NEW_IDU_BRI
          tv2TV_CONF    = 0x08500442 | TV_DV_SEL_601;
        #else
          tv2TV_CONF    = 0x08500442 | TV_DV_SEL_601;
        #endif
       #else
        tv2TV_CONF    = 0x28500442 | TV_DV_SEL_601;
       #endif
     #elif(TV_DigiOut_SEL == TV_DigiOut_YUV)
       #if IDUTV_DISPLAY_PN
          #if NEW_IDU_BRI
          tv2TV_CONF    = 0x08500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #else
          tv2TV_CONF    = 0x08500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
          #endif
         #else
          tv2TV_CONF    = 0x28500442 | TV_DV_SEL_YUV | TV_D_POL_FAL | TV_HS_POL_LOW | TV_VS_POL_LOW;
       #endif
     #else //656
       #if IDUTV_DISPLAY_PN
        #if NEW_IDU_BRI
        tv2TV_CONF    = 0x08500442 | TV_DV_SEL_656_MODE1;
        #else
        tv2TV_CONF    = 0x08500442 | TV_DV_SEL_656_MODE1;
        #endif
       #else
        tv2TV_CONF    = 0x28500442 | TV_DV_SEL_656_MODE1;
       #endif
     #endif

        tv2TV_CTRL    =0x03FF0AA5;

     #if TVOUT_CRYSTAL_24MHZ
            tv2CORING     =0x00000000;    //Coring threshold
            tv2HTOTAL     =0x000005f5;    //Htotal=1716/2=858 pixel/line
            tv2CSYNCCFG   =0x004000ce;    //Hsync config: height,incremental value
            tv2BURSTCFG   =0x00240060;    //Color burst config: height,incremental value
            tv2BLANKCFG   =0x002800ce;    //Blank config: Blank level value,setup value
            tv2BURSTSTAEND=0x00bc0080;    //Color burst start/end position
            tv2VACTSTAEND = ((240-1)<<16) | 0;
            tv2ACTSTAEND = TV_ACTSTAEND_NTSC;
            tv2HSYNC_WIDTH = 0x70 | ((tvHTOTAL-(tv2ACTSTAEND>>16)+(tv2ACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
            tv2YUVGAIN_REG=0x00808080;    //RGB Gain
            tv2SUB_CAR_FR =0x262e78cc;    //subcarrier freq.
     #else
            tv2CORING     =0x00000000;    //Coring threshold            tvHTOTAL     =0x000006b4;    //Htotal=1716/2=858 pixel/line
            tv2HTOTAL     =0x000006b4;    //Htotal=1716/2=858 pixel/line
            tv2CSYNCCFG   =0x008000d4;    //Hsync config: height,incremental value
            tv2BURSTCFG   =0x00280072;    //Color burst config: height,incremental value
            tv2BLANKCFG   =0x001C00e0;    //Blank config: Blank level value,setup value
            tv2BURSTSTAEND=0x00d40090;    //Color burst start/end position
            tv2VACTSTAEND = ((240-1)<<16) | 0;
        #if(TV_D1_OUT_FULL)
            tv2ACTSTAEND = TV_ACTSTAEND_NTSC_D1;
        #else
            tv2ACTSTAEND = TV_ACTSTAEND_NTSC;
        #endif
            tv2HSYNC_WIDTH = 0x7e | ((tvHTOTAL-(tv2ACTSTAEND>>16)+(tv2ACTSTAEND & 0x0ffff))<<16);    //Hsync width config. HSWIDTH_YUV = HTAL - HACT_E + HACT_S
            tv2YUVGAIN_REG=0x00808080;    //RGB Gain
            tv2SUB_CAR_FR =0x21f07c20;    //subcarrier freq.
     #endif

     #if(TV_D1_OUT_FULL)
        tv2TVFB_STRIDE=704/2;    // Video Buffer stride = 704
     #else
        tv2TVFB_STRIDE=640/2;    // Video Buffer stride = 640
     #endif
        tv2DACVAL     =0x01500020;    //for DAC test,y,uv out
        tv2DACVAL1    =0x00000005;    //for DAC test,cvbs out

        tv2WHITEYEL   =0x019e00fe;    //Color bar config: white,yellow
        tv2CYANGRN    =0x02de023e;    //Color bar config: cyan,green
        tv2MAGRED     =0x041e037e;    //Color bar config: magenta, red
        tv2BLUEBLACK  =0x055e04be;    //Color bar config: blue,black
        tv2TOFB_STRIDE = 0;
    }
#endif
}
void IduVideo_ClearBuf(void)
{
    u32 i;
    u32 *addr;


    if(sysTVOutOnFlag)
    {
    #if 1
        memset_hw_Word(PKBuf0, 0x80800000, VIDEODISPBUF_SIZE);
        memset_hw_Word(PKBuf1, 0x80800000, VIDEODISPBUF_SIZE);
        memset_hw_Word(PKBuf2, 0x80800000, VIDEODISPBUF_SIZE);
    #else
          addr= (u32 *)PKBuf0;
          for(i=0 ; i<VIDEODISPBUF_SIZE ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
          addr= (u32 *)PKBuf1;
          for(i=0 ; i<VIDEODISPBUF_SIZE ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
          addr= (u32 *)PKBuf2;
          for(i=0 ; i<VIDEODISPBUF_SIZE ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
     #endif
    }
    else
    {
    #if 1
        memset_hw_Word(iduvideobuff, 0x80800000, (PANNEL_X * PANNEL_Y * 2));
    #else
        addr = (u32 *)iduvideobuff;
        for(i = 0; i < (PANNEL_X * PANNEL_Y * 2); i += 4)
        {

            *addr = 0x80800000;
            addr++;
        }
    #endif
    }
}

void IduVideo_ClearPartBuf(u32 start, u32 end)
{
    u32 i;
    u32 *addr;

    if(start > end)
        return;

    if(sysTVOutOnFlag)
    {
    #if 1
        addr= (u32 *)(PKBuf0 + start);
        memset_hw_Word(addr, 0x80800000, (int)(end-start));

        addr= (u32 *)(PKBuf1 + start);
        memset_hw_Word(addr, 0x80800000, (int)(end-start));

        addr= (u32 *)(PKBuf2 + start);
        memset_hw_Word(addr, 0x80800000, (int)(end-start));
    #else
          addr= (u32 *)(PKBuf0 + start);
          for(i=start ; i<end ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
          addr= (u32 *)(PKBuf1 + start);
          for(i=start ; i<end ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
          addr= (u32 *)(PKBuf2 + start);
          for(i=start ; i<end ; i+=4)
          {
            *addr = 0x80800000;
            addr++;
          }
    #endif
    }
}

void IduVideo_ClearPKBuf(u8 bufinx)
{
#ifdef TVOUT_UI_ENA
    u32 i;
    u32 *addr;


    if(bufinx == 0)
        addr= (u32 *)PKBuf0;
    else if(bufinx == 1)
        addr= (u32 *)PKBuf1;
    else if(bufinx == 2)
        addr= (u32 *)PKBuf2;
  #if 0
    for(i=0 ; i<VIDEODISPBUF_SIZE ; i+=4)
    {
        *addr = 0x80800000;
        addr++;
    }
  #else
    memset_hw_Word(addr, 0x80800000, VIDEODISPBUF_SIZE);
  #endif

#endif
}

void IduVideo_ClearPartPKBuf(u32 start, u32 end, u8 bufinx)
{
#ifdef TVOUT_UI_ENA
    u32 i;
    u32 *addr;

    if(bufinx == 0)
        addr= (u32 *)(PKBuf0 + start);
    else if(bufinx == 1)
        addr= (u32 *)(PKBuf1 + start);
    else if(bufinx == 2)
        addr= (u32 *)(PKBuf2 + start);

    #if 1
        memset_hw_Word(addr, 0x80800000, (int)(end-start));
    #else
        for(i=start ; i<end ; i+=4)
        {
            *addr = 0x80800000;
            addr++;
        }
    #endif
#endif
}

void IduDispWinSel(u8 index)
{
   u32 temp;

   temp=tvFRAME_CTL;
   temp &= (~0x00003000);

   switch(index)
   {
      case 0:
        temp |= TV_FRMCTL_FB_SEL0;
        break;

      case 1:
        temp |= TV_FRMCTL_FB_SEL1;
        break;

      case 2:
        temp |= TV_FRMCTL_FB_SEL2;
        break;
   }
   tvFRAME_CTL = temp;

}

void IduVideoEnable(u8 enable)
{
#if ((HW_BOARD_OPTION == MR8200_RX_RDI_RX240) || (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014) ||\
    (HW_BOARD_OPTION == MR8200_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8120_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV) ||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3) || (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4))
    if(sysTVOutOnFlag)
        return;
#endif
    if (enable)
        IduWinCtrl |= IDU_CTL_VDO_ENA;
    else
        IduWinCtrl &= ~IDU_CTL_VDO_ENA;
}

void IDU_TVLayer_Stride(u16 pannel_w , u8 *scaler_data , u16 scaler_w , u16 scaler_h , u16 x_pos , u16 y_pos, u16 scaler_stride_w,u8 *target_addr)
{
    u32 *addr;
    u32 *data;
    u32 i,j;
    u32 sx=0,ex=0;

    data = (u32 *)scaler_data;
    addr= (u32 *)target_addr;


    //word align
    pannel_w >>= 1;
    scaler_w >>= 1;
    scaler_stride_w >>= 1;
    x_pos >>= 1;
    sx = x_pos;
    ex = scaler_w+x_pos;

    for(j=y_pos;j<(y_pos+scaler_h);j++)
       for(i=sx;i<ex;i++)
           addr[(j*pannel_w)+i] = data[((j-y_pos)*scaler_stride_w)+(i-sx)];
}

void IDU_TVLayer(u16 pannel_w , u8 *scaler_data , u16 scaler_w , u16 scaler_h , u16 x_pos , u16 y_pos)
{
    u32 *addr;
    u32 *data;
    u32 i,j;
    u32 sx=0,ex=0,tmp_sx,tmp_icon_x;;

    data = (u32 *)scaler_data;
   #if ((HW_BOARD_OPTION==ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N))
    addr= (u32 *)pucClockImageBuf;
   #else
    addr= (u32 *)PKBuf0;
   #endif


    //word align
    pannel_w >>= 1;
    scaler_w >>= 1;
    x_pos >>= 1;
    sx = x_pos;
    ex = scaler_w+x_pos;
    tmp_sx = pannel_w * y_pos;
    tmp_icon_x =  0 ;
    for (j = y_pos; j< (y_pos+ scaler_h); j++)
    {
        for (i = sx; i < ex; i++)
        {
                addr[tmp_sx+i] = data[tmp_icon_x +(i-sx)] ;
        }
        tmp_sx += pannel_w;
        tmp_icon_x += scaler_w;
    }
}

void IDU_TVMoveData(u16 pannel_w , u8 *dst_data , u8 *src_data , u16 scaler_w , u16 scaler_h , u8 type)
{
  u32 *addr;
  u32 *data;
  u32 i,j;


  //data = (u32 *)src_data;
  if(TvOutMode==SYS_TV_OUT_PAL)
  {
      data = (u32 *)(src_data+0xF000);
      addr= (u32 *)(dst_data+0xF000); //640*48*2 = 0xF000
  }
  else if(TvOutMode==SYS_TV_OUT_NTSC)
  {
      data = (u32 *)src_data;
      addr= (u32 *)dst_data;
  }
  else if(TvOutMode==SYS_TV_OUT_HD720P)
  {
      data = (u32 *)src_data;
      addr= (u32 *)dst_data;
  }
  else if(TvOutMode==SYS_TV_OUT_FHD1080I)
  {
      data = (u32 *)src_data;
      addr= (u32 *)dst_data;
  }

  //word align
  pannel_w >>= 1;
  scaler_w >>= 1;
  if(type == 0)
  {
    for(j=0;j<scaler_h;j++)
      for(i=0;i<scaler_w;i++)
        addr[(j*pannel_w)+i] = data[(j*pannel_w)+i];
  }
  else
  {
    for(j=0;j<scaler_h;j++)
      for(i=0;i<scaler_w;i++)
        addr[(j*pannel_w)+i] = 0x80800000;
  }
}

void LCDBright(u8 pattern)
{

#if(LCM_OPTION == LCM_A015AN04)

#elif (LCM_OPTION == LCM_TPG105)
    u8 i;


  IduEna  &=  ~0x00000008;
  IduEna    |=  0x00000008;
  for(i=0;i<100;i++);
  IduEna &= (~0x00000002);
  for(i=0;i<100;i++);
  OSTimeDly(1);
      IduMpuCmdCfg = 0x00ff0028;
      iduWaitCmdBusy();
      IduMpuCmd = 0x00000802;
      iduWaitCmdBusy();
          IduMpuCmd = 0x00001C08;
      iduWaitCmdBusy();
          IduMpuCmd = 0x0000300A;
          iduWaitCmdBusy();
    switch(pattern)
    {
        case 0:
             //IduMpuCmdCfg = 0x00140028;
             //iduWaitCmdBusy();
         IduMpuCmd = 0x00003000;
         break;
        case 1:
             //IduMpuCmdCfg = 0x00140028;
             //iduWaitCmdBusy();
         //IduMpuCmd = 0x00003008;
         IduMpuCmd = 0x00003010;
         break;
        case 2:
             //IduMpuCmdCfg = 0x00140028;
             //iduWaitCmdBusy();
         //IduMpuCmd = 0x00003010;
         IduMpuCmd = 0x00003018;
         break;
        case 3:
             //IduMpuCmdCfg = 0x00140028;
             //iduWaitCmdBusy();
         //IduMpuCmd = 0x00003018;
         IduMpuCmd = 0x00003028;
      case 4:
             //IduMpuCmdCfg = 0x00140028;
             //iduWaitCmdBusy();
         //IduMpuCmd = 0x00003020;
         IduMpuCmd = 0x00003030;
         break;
    }
    IduEna |= 0x00000002;
    //DEBUG_IDU("pattern = %d\n\r",pattern);
#endif
}

#if(HW_BOARD_OPTION == SUNWAY_SDV)
void iduPanelOnOff(u8 on)
{
    #if(PANEL_SELECT == 1)//Q23
    if(on)
    {
        IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    IduMpuCmd = 0x00000029;
	    iduWaitCmdBusy();
	    iduDelay(200);// delay 10 ms
	    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	}
	else
	{
        IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    IduMpuCmd = 0x00000028;
	    iduWaitCmdBusy();
	    iduDelay(200);// delay 10 ms
	    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	}
    #else//Q04/R00
    if(on)
    {
        IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    IduMpuCmd = 0x00000007;
	    iduWaitCmdBusy();
	    iduDelay(200);// delay 10 ms
	    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    IduMpuCmd = 0x00000010;
	    iduWaitCmdBusy();
	    iduDelay(400);// delay 10 ms
	    IduMpuCmd = 0x00000017;
	    iduWaitCmdBusy();
    }
    else
    {
        IduMpuCmdCfg = IDU_CMD_RS_1 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    IduMpuCmd = 0x00000007;
	    iduWaitCmdBusy();
	    iduDelay(200);// delay 10 ms
	    IduMpuCmdCfg = IDU_CMD_RS_0 | IDU_CMD_SPI_0 | IDU_CMD_W_8;
	    //IduMpuCmd = 0x00000000;
            IduMpuCmd = 0x00000010;
	    iduWaitCmdBusy();
	    iduDelay(400);// delay 10 ms
	    //IduMpuCmd = 0x00000000;
            IduMpuCmd = 0x00000014;
	    iduWaitCmdBusy();
    }
    #endif
}
#endif

/*

Routine Description:

    LCD Brightness Adjustment. This is only for Roule's project now.

Arguments:

    ucSetLevel - Brightness level to set.

Return Value:

	None.

*/
#if (HW_BOARD_OPTION == ROULE_SD7N)
void iduSetTVOutBrightness(s8 ucSetLevel)
{
	u32	unParaOri;

	unParaOri = tvYUVGAIN_REG;
	unParaOri &= 0x00FFFF00;
       ucSetLevel =ucSetLevel*3;

	unParaOri |= 250-ucSetLevel;

    tvYUVGAIN_REG = unParaOri;

}

void iduSetTVOutSaturation(s8 ucSetLevel)
{
	u32	unParaOri;
	u32	temp_32;

	unParaOri = tvYUVGAIN_REG;
	unParaOri &= 0x000000FF;

	 temp_32 = ucSetLevel*3;

	 temp_32 = 250-temp_32;

	 unParaOri|=temp_32<<8|temp_32<<16;

    tvYUVGAIN_REG = unParaOri;
}
#else
void iduSetTVOutBrightness(s8 ucSetLevel)
{
	u32	unParaOri;

	unParaOri = tvYUVGAIN_REG;
	unParaOri &= 0x00FFFF00;

	switch(ucSetLevel)
	{
		case 0:
			unParaOri |= 0xA9;	/* 0xA9 is the system default value for tv-out */
			break;

		case 1:
			unParaOri |= 0x91;
			break;

		case 2:
			unParaOri |= 0x79;
			break;

		case 3:
			unParaOri |= 0x71;
			break;

		case 4:
			unParaOri |= 0x69;
			break;

		case 5:
			unParaOri |= 0x59;
			break;

		case 6:
			unParaOri |= 0x41;
			break;

		case 7:
			unParaOri |= 0x39;
			break;

		case 8:
			unParaOri |= 0x21;
			break;

		case 9:
			unParaOri |= 0x19;
			break;
	}
    tvYUVGAIN_REG = unParaOri;

}

void iduSetTVOutSaturation(s8 ucSetLevel)
{
	u32	unParaOri;

	unParaOri = tvYUVGAIN_REG;
	unParaOri &= 0x000000FF;

	switch(ucSetLevel)
	{
		case 0:
			unParaOri |= 0x00A9A900;	/* 0xA9 is the system default value for tv-out */
			break;

		case 1:
			unParaOri |= 0x00919100;
			break;

		case 2:
			unParaOri |= 0x00797900;
			break;

		case 3:
			unParaOri |= 0x00717100;
			break;

		case 4:
			unParaOri |= 0x00696900;
			break;

		case 5:
			unParaOri |= 0x00595900;	/* 0xA9 is the system default value for tv-out */
			break;

		case 6:
			unParaOri |= 0x00414100;
			break;

		case 7:
			unParaOri |= 0x00393900;
			break;

		case 8:
			unParaOri |= 0x00212100;
			break;

		case 9:
			unParaOri |= 0x00191900;
			break;
	}
    tvYUVGAIN_REG = unParaOri;

}
#endif
#if( (HW_BOARD_OPTION==ROULE_DOORPHONE)||(HW_BOARD_OPTION == ROULE_SD8F)||(HW_BOARD_OPTION == ROULE_SD7N)||(HW_BOARD_OPTION == LEIYON_DOORPHONE)||\
     (HW_BOARD_OPTION==PROJECT_MR8980_6720)||(HW_BOARD_OPTION==PROJECT_OPCOM_REAL)||(HW_BOARD_OPTION==PROJECT_DW950_REAL) )

/*

Routine Description:

    LCD Brightness Adjustment. This is only for Roule's project now.

Arguments:

    ucSetLevel - Brightness level to set.

Return Value:

	None.

*/
void iduSetLcdBrightness(s8 ucSetLevel)
{
	u32	unParaOri;

	unParaOri = IduYCbCr2R;
	unParaOri &= 0x00FFFF00;
    IduYCbCr2R = (unIduParaLcdBrightness[ucSetLevel][0] | unParaOri);

	unParaOri = IduYCbCr2G;
	unParaOri &= 0x00FFFF00;
    IduYCbCr2G = (unIduParaLcdBrightness[ucSetLevel][1] | unParaOri);

	unParaOri = IduYCbCr2B;
	unParaOri &= 0x00FFFF00;
    IduYCbCr2B = (unIduParaLcdBrightness[ucSetLevel][2] | unParaOri);

}

/*

Routine Description:

    LCD Saturation Adjustment. This is only for Roule's project now.

Arguments:

    ucSetLevel - Saturation level to set.

Return Value:

	None.

*/
void iduSetLcdSat(s8 ucSetLevel)
{

	u32 unParaOri;

	unParaOri = IduYCbCr2R;
	unParaOri &= 0x000000FF;
	IduYCbCr2R = (unIduParaLcdSat[ucSetLevel][0] | unParaOri);

	unParaOri = IduYCbCr2G;
	unParaOri &= 0x000000FF;
	IduYCbCr2G = (unIduParaLcdSat[ucSetLevel][1] | unParaOri);

	unParaOri = IduYCbCr2B;
	unParaOri &= 0x000000FF;
	IduYCbCr2B = (unIduParaLcdSat[ucSetLevel][2] | unParaOri);
}
/*

Routine Description:

    Set Video Buf Addr.


Return Value:

	None.

*/
void iduSetVideoBufAddr(u8* pucBuf1, u8* pucBuf2, u8* pucBuf3)
{

	IduVidBuf0Addr= (u32)pucBuf1;
	IduVidBuf1Addr= (u32)pucBuf2;
	IduVidBuf2Addr= (u32)pucBuf3;

}

#endif

#if ((HW_BOARD_OPTION == VER100_CARDVR)||(HW_BOARD_OPTION==SIYUAN_CVR)||(HW_BOARD_OPTION==NEW_SIYUAN))
#if (HW_BOARD_OPTION == VER100_CARDVR)
u8 iduDigitTab[16]=
{
    //0~9, A~F
	0xAF,0xA0,0x6D,0xE9,
	0xE2,0xCB,0xCF,0xA1,
	0xEF,0xEB,0xE7,0xCE,
	0x0F,0xEC,0x4F,0x47
};

u8 iduSymbol[14] =
{
    //SEG1~14,D3~D0
    0xA0,0xF0,0xB0,0xF0,0x90,
    0xB0,0xF0,0xB0,0xF0,0xD0,
    0xF0,0xA0,0xF0,0xB0
};

u8 iduDigitAddr[6] = {7,5,2,0,11,13};

#elif(HW_BOARD_OPTION == SIYUAN_CVR)

u8 iduDigitTab[16]=
{
    //0~9, A~F
	0xFA,0x0A,0xD6,0x9E,
	0x2E,0xBC,0xFC,0x1A,
	0xFE,0xBE,0x7E,0xEC,
	0xF0,0xCE,0xF4,0x74
};
/*
u8 iduSymbol[30] =
{
    //SEG1~34,D3~D0
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xE0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0
};
*/
u8 iduSymbol[30] =
{
    //SEG1~34,D3~D0
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xC0,0x20,
    0xE0,0xF0,0xA0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0,
    0xF0,0xF0,0xF0,0xF0,0xF0
};

u8 iduDigiNumSymbol[30] =
{
    //SEG1~34,D3~D0
    0xF0,0xE0,0xF0,0xE0,0xF0,
    0xE0,0xF0,0xE0,0xF0,0xE0,
    0xF0,0xE0,0xF0,0xE0,0xF0,
    0xE0,0xF0,0xE0,0xE0,0xF0,
    0xE0,0xF0,0xE0,0xF0,0xE0,
    0xF0,0xE0,0xF0,0xE0,0xF0
};

//Digital 1~15
u8 iduDigitAddr[15] = {0,2,4,6,21,23,25,27,29,19,8,10,12,14,16};
u8 iduSymbolAddr[15] = {1, 3, 5, 7, 22, 24, 26, 28, 18, 20, 9, 11, 13, 15, 17};	/* symbols addr besides digit number */

#elif(HW_BOARD_OPTION==NEW_SIYUAN)

u8 iduDigitTab[10]=
{
    //0~9
	0xAF,0x06,0xCB,0x4F,
	0x66,0x6D,0xED,0x07,
	0xEF,0x6F
};

u8 iduSymbol[12] =
{
    //SEG1~12,D3~D0
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00
};

u8 iduDigitAddr[4] = {0,2,4,6};
u8 iduSymbolAddr[9] = {0, 2, 4, 6, 7, 8, 9, 10, 11};
static u8 FlashClass = 0 ;
static u8 SetTimeFlashClass = 0 ;
static u8 SetSizeFlashClass = 0 ;
static u8 SetFrameRateFlashClass = 0 ;
static u8 SetDeleteFileFlashClass = 0 ;
static u8 SetYearPositionFlash = 0 ;

#endif



void iduSendBit_1621(u8 dt,u8 cnt) //first sent MSB.
{
	u8 i;
    for(i =0; i <cnt; i ++)
	{
	    if((dt&0x80)==0)
	        gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_LOW);
	    else
 	        gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);

	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_LOW); //latch
	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	    dt<<=1;
	}
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
}

void iduRecvDataBit_1621(u8 *pucData,u8 cnt) //first sent LSB.
{
	u8 i;
	u8 ucDataTemp;

	*pucData = 0;

	gpioSetDir(GROUP_LCDMODULE, PIN_LCD_SDA, 1);

	for(i =0; i <cnt; i ++)
	{
	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_LOW);
	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_HIGH);

		gpioGetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, &ucDataTemp);

		*pucData |= (ucDataTemp << i);

	}
	gpioSetDir(GROUP_LCDMODULE, PIN_LCD_SDA, 0);

	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_HIGH);

}

void iduSendDataBit_1621(u8 dt,u8 cnt) //first sent LSB.
{
	u8 i;
	for(i =0; i <cnt; i ++)
	{
        if((dt&0x10)==0)
            gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_LOW);
	    else
	        gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);

	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_LOW); //latch
	    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	    dt>>=1;
	}
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
}

void iduSendHT1621Cmd(u8 command)
{
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_LOW);
	iduSendBit_1621(0x80,3); //Command Mode "100"
	iduSendBit_1621(command,9); //9bit data, 8bit real data, 1bit don't care
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
}

void iduWrite_1621(u8 addr,u8 dt)
{
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_LOW);
	iduSendBit_1621(0xa0,3); //Write Mode "101"
	iduSendBit_1621(addr<<2,6); //6bit addr
	iduSendDataBit_1621(dt,4); 	// 4bit data
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
}

void iduRead_1621(u8 addr,u8 *pucData)
{
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_LOW);
	iduSendBit_1621(0xC0,3); //Read Mode "110"
	iduSendBit_1621(addr<<2,6); //6bit addr
	iduRecvDataBit_1621(pucData,4); 	// 4bit data
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
}

void iduWriteAll_1621(u8 addr,u8 *p,u8 cnt)
{
	u8 i;

	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_LOW);
	iduSendBit_1621(0xa0,3); //Write Mode "101"
	iduSendBit_1621(addr<<2,6); //6bit addr
	for(i =0; i <cnt; i++,p++) //sequential write
	{
	    iduSendDataBit_1621(*p,4);
	}
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
	gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
}

void iduLCDDispAll(void)
{
	u8 i;
	u8 a[LCDSEGMAX];

	for(i=0;i<LCDSEGMAX;i++)
	{
		a[i]=0xf0;
	}
	iduWriteAll_1621(0,a,LCDSEGMAX);
}

void iduClearLCD(void)
{
	u8 i;
	u8 a[LCDSEGMAX];

	for(i=0;i<LCDSEGMAX;i++)
	{
		a[i]=0x00;
	}
	iduWriteAll_1621(0,a,LCDSEGMAX);
}

#if (HW_BOARD_OPTION == VER100_CARDVR)
void iduControlSymbol(u8 sd,u8 circle, u8 rec, u8 usb)
{
    if(sd)
        iduSymbol[9] |= 0x40;
    else
        iduSymbol[9] &= ~0x40;

    if(circle)
        iduSymbol[9] |= 0x80;
    else
        iduSymbol[9] &= ~0x80;

    if(rec)
        iduSymbol[0] |= 0x10;
    else
        iduSymbol[0] &= ~0x10;

    if(usb)
        iduSymbol[9] |= 0x20;
    else
        iduSymbol[9] &= ~0x20;

    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolSD(u8 sd)
{
    if(sd)
        iduSymbol[9] |= 0x40;
    else
        iduSymbol[9] &= ~0x40;

    //iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolCircle(u8 circle)
{
    if(circle)
        iduSymbol[9] |= 0x80;
    else
        iduSymbol[9] &= ~0x80;

    //iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolREC(u8 rec)
{
    if(rec)
        iduSymbol[0] |= 0x10;
    else
        iduSymbol[0] &= ~0x10;

    //iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolUSB(u8 usb)
{
    if(usb)
        iduSymbol[9] |= 0x20;
    else
        iduSymbol[9] &= ~0x20;

    //iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolDataTime(u8 DT)
{
    if(DT == 0)
    {   //Display Time
        iduSymbol[11] &= ~0x10;
        iduSymbol[13] |= 0x10;
        iduSymbol[4] &= ~0xC0;
        iduSymbol[4] |= 0x80;
    }
    else
    {   //Display Date
        iduSymbol[11] |= 0x10;
        iduSymbol[13] &= ~0x10;
        iduSymbol[4] &= ~0xC0;
        iduSymbol[4] |= 0x40;
    }
}

void iduDisplaySymbol(u8 sd,u8 circle, u8 rec, u8 usb)
{
    u8 i;

    for(i=0;i<13;i++)
        iduSymbol[i] = 0x00;

    iduClearLCD();
    iduControlSymbol(sd,circle,rec,usb);
    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduDispLCDString(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    u8 i;
    u8 tempstr[6];
    u8 pos;
    u8 temp;
    //len = strlen((char *)string);
    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        tempstr[i] = iduDigitTab[(*string - 48)];
        string++;

        if(i<4)
        {
            //Digit1
            pos = iduDigitAddr[i];
            temp = tempstr[i] & 0xF0;
            iduSymbol[pos] &= 0x10;
            iduSymbol[pos] |= temp;
            //Digit2
            temp = (tempstr[i] & 0x0F) << 4;
            iduSymbol[pos+1] = temp;
        }
        else
        {
            //Digit1
            pos = iduDigitAddr[i];
            temp = tempstr[i] & 0xF0;
            iduSymbol[pos] &= 0x10;
            iduSymbol[pos] |= temp;
            //Digit2
            temp = (tempstr[i] & 0x0F) << 4;
            iduSymbol[pos-1] = temp;
        }
    }

}

void iduDispSDString(u8 on)
{
    u8 iduSDSymbol[14] =
    {
        //SEG1~14,D3~D0
        0xE0,0xC0,0xC0,0xB0,0x40,
        0x40,0x00,0x40,0x00,0x00,
        0x00,0x40,0x00,0x40
    };

    if(on)
        iduWriteAll_1621(0,iduSDSymbol,LCDSEGMAX);
    else
        iduClearLCD();
}

void iduClearDigitNum(u8 ucSymbolStart, u8 ucSymbolStop)
{
    u8 i;
    u8 tempstr[6];
    u8 pos;
    u8 temp;
    //len = strlen((char *)string);
    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        //tempstr[i] = iduDigitTab[(*string - 48)];
        //string++;

        if(i<4)
        {
            //Digit1
            pos = iduDigitAddr[i];
            ///temp = tempstr[i] & 0xF0;
            iduSymbol[pos] &= ~0xE0;
            iduWrite_1621(pos, iduSymbol[pos]);
            //iduSymbol[pos] |= temp;
            //Digit2
            ///temp = (tempstr[i] & 0x0F) << 4;
            iduSymbol[pos+1] = 0;
            iduWrite_1621((pos+1), 0);
        }
        else
        {
            //Digit1
            pos = iduDigitAddr[i];
            ///temp = tempstr[i] & 0xF0;
            iduSymbol[pos] &= ~0xE0;
            iduWrite_1621(pos, iduSymbol[pos]);
            ///iduSymbol[pos] |= temp;
            //Digit2
            ///temp = (tempstr[i] & 0x0F) << 4;
            iduSymbol[pos-1] = 0;
            iduWrite_1621((pos-1), 0);
        }
    }
}

void iduDisplayDigitNum(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    iduDispLCDString(string, ucSymbolStart, ucSymbolStop);
    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduDisplayDefaultSymbol(void)
{
    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

#elif(HW_BOARD_OPTION == SIYUAN_CVR)
/*

Routine Description:

    Display string on LCD.

Arguments:

    string - string buffer to display.
    ucSymbolStart - the first position of symbol to display
    ucSymbolStop - the last position of symbol to display

Return Value:

	None.

*/

void iduDispLCDString( u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    u8 i;
    u8 tempstr[16];
    u8 pos;
    u8 temp;

	if (ucSymbolStart==0)
	{
		DEBUG_IDU("Error: Display symbol position error\n");
		return;
	}

    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        tempstr[i] = iduDigitTab[(*string - 48)];
        string++;

        //Digital Left Part
        pos = iduDigitAddr[i];
        temp = tempstr[i] & 0xF0;
        iduSymbol[pos] = temp;

        //Digital Right Part
        temp = (tempstr[i] & 0x0F) << 4;

		if (pos == 29)
		{
	        iduSymbol[18] &= 0x10;
	        iduSymbol[18] |= temp;
		}
		else
		{
	        iduSymbol[pos+1] &= 0x10;
	        iduSymbol[pos+1] |= temp;
		}
    }
}

void iduDisplayDigitNum(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    iduDispLCDString(string, ucSymbolStart, ucSymbolStop);
    iduWriteAll_1621(0, iduSymbol, LCDSEGMAX);
}

void iduClearDigitNum(u8 ucSymbolStart, u8 ucSymbolStop)
{
	u8 ucData, ucTemp;
	u8 i, pos;

	if (ucSymbolStart == 0)
	{
		DEBUG_IDU("Error! ucSymbolStart is 0\n");
		return;
	}

    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        //Digital Left Part
        pos = iduSymbolAddr[i];
		iduRead_1621(pos, &ucTemp);

		ucData = ((ucTemp & 0x01)<<4);

		/* clear the left part */
		if (pos == 18)
			iduWrite_1621(29, 0);
		else
			iduWrite_1621(pos-1, 0);

		/* clear the right part */
		iduWrite_1621(pos, ucData);
    }
}

void iduDisplayDefaultSymbol(void)
{
    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}
void iduControlSymbolVideo(u8 video)
{
    if(video)
        iduSymbol[13] |= 0x10;
    else
        iduSymbol[13] &= ~0x10;
}

void iduControlVideoSize(u8 Setting)
{
	u8	Str[3];
	u32	unVideoSize;

	switch(Setting)
	{
		case 1:		/* 640 */
			unVideoSize = 640;
			break;

		case 2:
			unVideoSize = 320;
			break;

	}

	sprintf(Str, "%3d", unVideoSize);

	iduDisplayDigitNum(Str, 13, 15);

}

void iduControlFrameRate(u8 Setting)
{
	u8	Str[2];
	u8	ucFrameRate;

	if (Setting == 0)
		ucFrameRate = 30;
	else
		ucFrameRate = 15;

	sprintf(Str, "%2d", ucFrameRate);
	iduDisplayDigitNum(Str, 11, 12);

}

void iduControlSymbolSD(u8 sd)
{
    if(sd)
        iduSymbol[9] |= 0x10;
    else
        iduSymbol[9] &= ~0x10;
}

void iduControlSymbolCol(u8 ucEna)
{
    if(ucEna)
        iduSymbol[3] |= 0x10;
    else
        iduSymbol[3] &= ~0x10;
}

void iduControlSymbolPower(u8 power)
{
    if(power)
        iduSymbol[17] |= 0x10;
    else
        iduSymbol[17] &= ~0x10;
}

void iduControlSymbolBattery(u8 level)
{
    if (level == 0)  //Hidden Symbol
    {
        iduSymbol[15] &= ~0x10;
        iduSymbol[1] &= ~0x10;
        iduSymbol[5] &= ~0x10;
        iduSymbol[7] &= ~0x10;
    }
    else if (level == 1) // 2/3
    {
        iduSymbol[15] |= 0x10;
        iduSymbol[1] |= 0x10;
        iduSymbol[5] |= 0x10;
        iduSymbol[7] &= ~0x10;
    }
    else if (level == 2) // 1/3
    {
        iduSymbol[15] |= 0x10;
		iduSymbol[1] |= 0x10;
        iduSymbol[5] &= ~0x10;
        iduSymbol[7] &= ~0x10;
    }
    else if (level == 3) //empty
    {
        iduSymbol[15] |= 0x10;
        iduSymbol[1] &= ~0x10;
        iduSymbol[5] &= ~0x10;
        iduSymbol[7] &= ~0x10;
    }
	else if (level == 4)//full
    {
        iduSymbol[15] |= 0x10;
        iduSymbol[1] |= 0x10;
        iduSymbol[5] |= 0x10;
        iduSymbol[7] |= 0x10;
    }
    //iduDisplayDefaultSymbol();
}

/*

Routine Description:

	update time on LCD.

Arguments:

	ucCallBy - indicator of called by UI or timer.

Return Value:

	None.

*/
void iduSetDateTime(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;
	u8	str[4];

	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(ucAdjPos)
	{
		case 1:	/* Year */
			if (ucFlashStat == 1)
			{
				iduClearDigitNum(5, 6);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.year);
				iduDisplayDigitNum(str, 5, 6);
			}
			break;

		case 2:	/* Month */
			if (ucFlashStat == 1)
			{
				iduClearDigitNum(7, 8);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.month);
				iduDisplayDigitNum(str, 7, 8);
			}
			break;

		case 3:	/* Day */
			if (ucFlashStat == 1)
			{
				iduClearDigitNum(9, 10);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.day);
				iduDisplayDigitNum(str, 9, 10);
			}
			break;
		case 4:	/* Hour */
			if (ucFlashStat == 1)
			{
				iduClearDigitNum(1, 2);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.hour);
				iduDisplayDigitNum(str, 1, 2);
			}
			break;

		case 5:	/* Min */
			if (ucFlashStat == 1)
			{
				iduClearDigitNum(3, 4);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.min);
				iduDisplayDigitNum(str, 3, 4);
			}
			break;


		default:
			return ;

	}

	ucFlashStat = 1 - ucFlashStat;

}

/*

Routine Description:

	Set video size on LCD.

Arguments:

	ucCallBy - indicator of called by UI or timer.

Return Value:

	None.

*/
void iduSetVideoSize(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;
	u8	str[4];
	u32	unTemp;

	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(ucAdjPos)
	{
		case 1:	/* 30 */
			unTemp = 640;
			break;

		case 2:	/* 15 */
			unTemp = 320;
			break;

		default:
			return ;

	}

	if (ucFlashStat == 1)
	{
		iduClearDigitNum(13, 15);
	}
	else
	{
		sprintf(str, "%03d", unTemp);
		iduDisplayDigitNum(str, 13, 15);
	}


	ucFlashStat = 1 - ucFlashStat;
}
/*

Routine Description:

	Set Frame rate and TV-out format on LCD.

Arguments:

	ucCallBy - indicator of called by UI or timer.

Return Value:

	None.

*/
void iduSetFrameRate(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;
	u8	str[4];
	u8	ucTemp;

	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(ucAdjPos)
	{
		case 1:	/* 30 */
			ucTemp = 30;
			break;

		case 2:	/* 15 */
			ucTemp = 15;
			break;

		case 3:	/* 50 */
			ucTemp = 50;
			break;

		case 4:	/* 60 */
			ucTemp = 60;
			break;

		default:
			return ;

	}

	if (ucFlashStat == 1)
	{
		iduClearDigitNum(11, 12);
	}
	else
	{
		sprintf(str, "%02d", ucTemp);
		iduDisplayDigitNum(str, 11, 12);
	}


	ucFlashStat = 1 - ucFlashStat;

}
/*

Routine Description:

	update LCD with specific part display.

Arguments:

	None.

Return Value:

	None.

*/
void iduUpdateLCD(void)
{

	switch(ucSetLCDPara)
	{
		case LCD_FLASH_SET_DATE_TIME:	/* in Set date and time mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetDateTime(0);
			}
			break;

		case LCD_FLASH_SET_VIDEO_SIZE:	/* in Set video size mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetVideoSize(0);
			}

			break;

		case LCD_FLASH_SET_FRAME_RATE:	/* in set frame rate and output format mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetFrameRate(0);
			}

			break;

	}

}

/*

Routine Description:

	update current time on LCD.

Arguments:

	None.

Return Value:

	None.

*/
void iduDispTime(void)
{
	RTC_DATE_TIME curDateTime;
	u8	szTime[11];
	static u8	ucCol = 0;
	if (ucSetLCDPara == 0)//LCD_FLASH_SET_DATE_TIME)
    {
        RTC_Get_Time(&curDateTime);
        sprintf(szTime, "%02d%02d%02d%02d%02d", curDateTime.hour, curDateTime.min, curDateTime.year, curDateTime.month, curDateTime.day);
        iduDisplayDigitNum(szTime, 1, 10);
		ucCol = 1 - ucCol;
		iduControlSymbolCol(ucCol);
    }
}

void iduShutdownClearAll(void)
{
    u16 i ;
    for(i=0 ; i<30 ; i++)
        iduSymbol[i] = 0x00 ;
    iduDisplayDefaultSymbol();
}
#elif(HW_BOARD_OPTION == NEW_SIYUAN)

void iduSetFlashClass(LCD_FLASH_CLASS class)
{
    FlashClass = class ;
}

void iduSetTimeFlashClass(LCD_FLASH_DATE class)
{
    SetTimeFlashClass = class ;
}

void iduSetSizeFlashClass(LCD_FLASH_RESOLUTION class)
{
    SetSizeFlashClass = class ;
}

void iduSetFameRateFlashClass(LCD_FLASH_FRAME_RATE class)
{
    SetFrameRateFlashClass = class ;
}

void iduSetDeleteFileFlashClass(LCD_FLASH_DELETE_FILE class)
{
    SetDeleteFileFlashClass = class ;
}

void iduSetYearPositionFlash(LCD_FLASH_YEAR_POSITION class)
{
    SetYearPositionFlash = class ;
}

void iduDispLCDString( u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    u8 i;
    u8 tempstr[16];
    u8 pos;
    u8 temp;

	if (ucSymbolStart==0)
	{
		DEBUG_IDU("Error: Display symbol position error\n");
		return;
	}

    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        tempstr[i] = iduDigitTab[(*string - 48)];
        string++;

        //Digital Left Part
        pos = iduDigitAddr[i];
        temp = tempstr[i] & 0xF0;
        iduSymbol[pos] &= 0x10;
        iduSymbol[pos] |= temp;

        //Digital Right Part
        temp = (tempstr[i] & 0x0F) << 4;
        iduSymbol[pos+1] = temp;
    }

}

void iduDisplayDigitNum(u8 *string, u8 ucSymbolStart, u8 ucSymbolStop)
{
    iduDispLCDString(string, ucSymbolStart, ucSymbolStop);
    iduWriteAll_1621(0, iduSymbol, LCDSEGMAX);
}

void iduClearDigitNum(u8 ucSymbolStart, u8 ucSymbolStop)
{
	u8 ucData, ucTemp;
	u8 i, pos;

	if (ucSymbolStart == 0)
	{
		DEBUG_IDU("Error! ucSymbolStart is 0\n");
		return;
	}

    for(i=(ucSymbolStart-1);i<ucSymbolStop;i++)
    {
        //Digital Left Part
        pos = iduDigitAddr[i];
		iduRead_1621(pos, &ucTemp);

        ucData = ((ucTemp & 0x01)<<4);
		// clear the left part
		iduWrite_1621(pos, ucData);

		// clear the right part
		iduWrite_1621(pos+1, 0);
    }

}

void iduDisplayDefaultSymbol(void)
{
    //DEBUG_IDU("IDU D2\n");
    iduWriteAll_1621(0,iduSymbol,LCDSEGMAX);
}

void iduControlSymbolVideo(u8 video)
{
    if(video)
        iduSymbol[13] |= 0x10;
    else
        iduSymbol[13] &= ~0x10;
}

void iduControlVideoSize(u8 Setting)
{
    //DEBUG_IDU("# iduControlVideoSize\n");
	switch(Setting)
	{
		case LCD_RESOLUTION_BIG:		/* 640 */
            iduSymbol[11] &= 0x10;
			iduSymbol[11] |= 0x80;
			break;

		case LCD_RESOLUTION_MEDIUM:     /* 352 */
            iduSymbol[11] &= 0x10;
			iduSymbol[11] |= 0x40;
			break;

        case LCD_RESOLUTION_SMALL:     /* 320 */
            iduSymbol[11] &= 0x10;
			iduSymbol[11] |= 0x20;
			break;

        case LCD_RESOLUTION_CLEAR:     /* All Clear */
            iduSymbol[11] &= 0x10;
			break;

	}
}

void iduControlFrameRate(u8 Setting)
{
	u8	Str[2];
	u8	ucFrameRate;

    //DEBUG_IDU("# iduControlFrameRate\n");
	switch(Setting)
	{
		case LCD_FRAME_RATE_30:		/* 30 */
            iduSymbol[8] &= 0xC0;
			iduSymbol[8] |= 0x10;
			break;

		case LCD_FRAME_RATE_15:     /* 15 */
            iduSymbol[8] &= 0xC0;
			iduSymbol[8] |= 0x20;
			break;

        case LCD_FRAME_RATE_CLEAR:     /* All Clear */
            iduSymbol[8] &= 0xC0;
			break;
	}

	//iduWriteAll_1621(0, iduSymbol, LCDSEGMAX);

}

void iduControlSymbolSD(u8 sd)
{
    //DEBUG_IDU("# iduControlSymbolSD %d\n", sd);
    if(sd == LCD_SD_ENABLE)
        iduSymbol[8] |= 0x80;
    else
        iduSymbol[8] &= ~0x80;
    //iduDisplayDefaultSymbol();
}

void iduControlSymbolCol(u8 ucEna)
{
    if(ucEna)
        iduSymbol[4] |= 0x10;
    else
        iduSymbol[4] &= ~0x10;
}

void iduControlSymbolBattery(u8 low)
{
    //DEBUG_IDU("# iduControlSymbolBattery\n");

    if(low)
        iduSymbol[8] |= 0x40;
    else
        iduSymbol[8] &= ~0x40;
    //iduDisplayDefaultSymbol();
}

void iduControlSymbolDeleteFile(u8 Setting)
{
    //DEBUG_IDU("# iduControlSymbolDeleteFile\n");

    switch(Setting)
	{
		case LCD_DELETE_LAST_FILE:		/* Last File */
            iduSymbol[9] &= 0xC0;
			iduSymbol[9] |= 0x10;
			break;

		case LCD_DELETE_ALL_FILE:     /* All File */
            iduSymbol[9] &= 0xC0;
			iduSymbol[9] |= 0x20;
			break;

        case LCD_DELETE_FILE_CLEAR:     /* All Clear */
            iduSymbol[9] &= 0xC0;
			break;
	}
    //iduWriteAll_1621(0, iduSymbol, LCDSEGMAX);
}

void iduControlVoice(u8 ucEna)
{
    if(ucEna == LCD_VOICE_ENABLE)
        iduSymbol[11] |= 0x10;
    else
        iduSymbol[11] &= 0xE0;
    //iduDisplayDefaultSymbol();
}

void iduControlCalendar(u8 ucEna)
{
    if(ucEna == LCD_CALENDAR_ENABLE)
        iduSymbol[9] |= 0x40;
    else
        iduSymbol[9] &= 0xB0;
    //iduDisplayDefaultSymbol();
}

void iduControlY(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_Y_ENABLE)
        iduSymbol[0] |= 0x10;
    else
        iduSymbol[0] &= 0xE0;
    //iduDisplayDefaultSymbol();
}

void iduControlM1(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_M1_ENABLE)
        iduSymbol[2] |= 0x10;
    else
        iduSymbol[2] &= 0xE0;
    //iduDisplayDefaultSymbol();
}

void iduControlM2(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_M2_ENABLE)
        iduSymbol[10] |= 0x40;
    else
        iduSymbol[10] &= ~0x40;
    //iduDisplayDefaultSymbol();
}

void iduControlH(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_H_ENABLE)
        iduSymbol[6] |= 0x10;
    else
        iduSymbol[6] &= ~0x10;
    //iduDisplayDefaultSymbol();
}

void iduControlD(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_D_ENABLE)
        iduSymbol[10] |= 0x10;
    else
        iduSymbol[10] &= ~0x10;
    //iduDisplayDefaultSymbol();
}

void iduControlS(u8 ucEna)
{
    if(ucEna == LCD_SYMBOL_S_ENABLE)
        iduSymbol[10] |= 0x20;
    else
        iduSymbol[10] &= ~0x20;
    //iduDisplayDefaultSymbol();
}

void iduSetDateTime(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;
	u8	str[4];

    //DEBUG_IDU("# iduSetDateTime\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;

	switch(SetTimeFlashClass)
	{
		case LCD_SET_YEAR:	/* Year */
			if (ucFlashStat == 1)
			{
			    if(SetYearPositionFlash == LCD_TEN_YEARS)
				    iduClearDigitNum(3, 3);
                else if(SetYearPositionFlash == LCD_SINGLE_YEAR)
				    iduClearDigitNum(4, 4);
			}
			else
			{
			    //First two digit is 20
                sprintf(str, "%02d%02d", 20, sDateTime.year);
				iduDisplayDigitNum(str, 1, 4);
			}
			break;

		case LCD_SET_MONTH:	/* Month */
            //DEBUG_IDU("# Month\n");
			if (ucFlashStat == 1)
			{
			    /*
			    if(ucAdjPos_sub == 1)
				    iduClearDigitNum(1, 1);
                else if(ucAdjPos_sub == 2)
				    iduClearDigitNum(2, 2);
				*/
				iduClearDigitNum(1, 2);
			}
			else
			{
				//sprintf(str, "%02d", sDateTime.month);
                sprintf(str, "%02d%02d", sDateTime.month, sDateTime.day);
				iduDisplayDigitNum(str, 1, 4);
			}
			break;

		case LCD_SET_DATE:	/* Day */
			if (ucFlashStat == 1)
			{
			    /*
			    if(ucAdjPos_sub == 1)
				    iduClearDigitNum(3, 3);
                else if(ucAdjPos_sub == 2)
				    iduClearDigitNum(4, 4);
				*/
				iduClearDigitNum(3, 4);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.day);
				iduDisplayDigitNum(str, 3, 4);
			}
			break;
		case LCD_SET_HOUR:	/* Hour */
			if (ucFlashStat == 1)
			{
			    /*
			    if(ucAdjPos_sub == 1)
				    iduClearDigitNum(1, 1);
                else if(ucAdjPos_sub == 2)
				    iduClearDigitNum(2, 2);
				*/
				iduClearDigitNum(1, 2);
			}
			else
			{
				//sprintf(str, "%02d", sDateTime.hour);
				sprintf(str, "%02d%02d", sDateTime.hour, sDateTime.min);
				iduDisplayDigitNum(str, 1, 4);
			}
			break;

		case LCD_SET_MINUTE:	/* Min */
			if (ucFlashStat == 1)
			{
			    /*
			    if(ucAdjPos_sub == 1)
				    iduClearDigitNum(3, 3);
                else if(ucAdjPos_sub == 2)
				    iduClearDigitNum(4, 4);
				*/
				iduClearDigitNum(3, 4);
			}
			else
			{
				sprintf(str, "%02d", sDateTime.min);
				iduDisplayDigitNum(str, 3, 4);
			}
			break;


		default:
			return ;

	}

	ucFlashStat = 1 - ucFlashStat;

}

void iduSetVideoSize(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;
	u8	str[4];
	u32	unTemp;

    //DEBUG_IDU("# iduSetVideoSize\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(SetSizeFlashClass)
	{
		case LCD_RESOLUTION_BIG:	/* 640 */
            if(ucFlashStat == 1)
			    iduSymbol[11] &= 0x10;
            else
			    iduSymbol[11] |= 0x80;
			break;

		case LCD_RESOLUTION_MEDIUM:	/* 352 */
			if(ucFlashStat == 1)
			    iduSymbol[11] &= 0x10;
            else
			    iduSymbol[11] |= 0x40;
			break;

        case LCD_RESOLUTION_SMALL:	/* 320 */
			if(ucFlashStat == 1)
			    iduSymbol[11] &= 0x10;
            else
			    iduSymbol[11] |= 0x20;
			break;

		default:
			return ;

	}
	ucFlashStat = 1 - ucFlashStat;
    iduDisplayDefaultSymbol();
}

void iduSetFrameRate(u8 ucCallBy)
{
	static u8	ucFlashStat = 0;

    //DEBUG_IDU("# iduSetFrameRate\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(SetFrameRateFlashClass)
	{
		case LCD_FRAME_RATE_30:	/* 30 */
			if(ucFlashStat == 1)
			    iduSymbol[8] &= 0xC0;
            else
			    iduSymbol[8] |= 0x10;
			break;

		case LCD_FRAME_RATE_15:	/* 15 */
			if(ucFlashStat == 1)
			    iduSymbol[8] &= 0xC0;
            else
			    iduSymbol[8] |= 0x20;
			break;

		default:
			return ;

	}

	ucFlashStat = 1 - ucFlashStat;
    iduDisplayDefaultSymbol();
}

void iduSetDeleteFile(u8 ucCallBy)
{
    static u8	ucFlashStat = 0;

    //DEBUG_IDU("# iduSetDeleteFile\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;


	switch(SetDeleteFileFlashClass)
	{
		case LCD_DELETE_LAST_FILE:	/* Last File */
			if(ucFlashStat == 1)
			    iduSymbol[9] &= 0xC0;
            else
			    iduSymbol[9] |= 0x10;
			break;

		case LCD_DELETE_ALL_FILE:	/* All File */
			if(ucFlashStat == 1)
			    iduSymbol[9] &= 0xC0;
            else
			    iduSymbol[9] |= 0x20;
			break;

		default:
			return ;

	}

	ucFlashStat = 1 - ucFlashStat;
    iduDisplayDefaultSymbol();
}

void iduSetVoice(u8 ucCallBy)
{
    static u8	ucFlashStat = 0;

    //DEBUG_IDU("# iduSetVoice\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;

    if(ucFlashStat == 1)
	    iduSymbol[11] &= 0xE0;
    else
	    iduSymbol[11] |= 0x10;

    ucFlashStat = 1 - ucFlashStat;
    iduDisplayDefaultSymbol();
}

void iduSetCalendar(u8 ucCallBy)
{
    static u8	ucFlashStat = 0;

    //DEBUG_IDU("# iduSetCalendar\n");
	if (ucCallBy == 1)	/* called by UI, not clear LCD */
		ucFlashStat = 0;

    if(ucFlashStat == 1)
	    iduSymbol[9] &= 0xB0;
    else
	    iduSymbol[9] |= 0x40;

    ucFlashStat = 1 - ucFlashStat;
    iduDisplayDefaultSymbol();
}

void iduUpdateLCD(void)
{
    //DEBUG_IDU("# iduUpdateLCD\n");

	switch(FlashClass)
	{
		case LCD_FLASH_SET_DATE_TIME:	/* in Set date and time mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetDateTime(0);
			}
			break;

		case LCD_FLASH_SET_VIDEO_SIZE:	/* in Set video size mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetVideoSize(0);
			}

			break;

		case LCD_FLASH_SET_FRAME_RATE:	/* in set frame rate and output format mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetFrameRate(0);
			}

			break;

        case LCD_FLASH_SET_DELET_FILE:	/* in set delete file mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetDeleteFile(0);
			}

			break;

        case LCD_FLASH_SET_VOICE:	/* in set delete file mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetVoice(0);
			}

			break;

        case LCD_FLASH_SET_CALENDAR:	/* in set delete file mode */
			/* always set enable when control other symbols */
			iduControlSymbolCol(1);

			if (ucUISetting == 0)
			{	/* ui is not setting */
				/* set the specific part flash */
				iduSetCalendar(0);
			}

			break;

        case LCD_FLASH_DEFAULT:         // No flashing
            break;

	}

}

void iduDispTime(void)
{
	RTC_DATE_TIME curDateTime;
	u8	szTime[5];
	static u8	ucCol = 0;

	if (FlashClass == LCD_FLASH_DEFAULT)//LCD_FLASH_SET_DATE_TIME)
    {
        //DEBUG_IDU("# RTC_Get_Time\n");
        RTC_Get_Time(&curDateTime);
        sprintf(szTime, "%02d%02d", curDateTime.hour, curDateTime.min);
        iduDisplayDigitNum(szTime, 1, 4);
		ucCol = 1 - ucCol;
		iduControlSymbolCol(ucCol);
    }
}

void iduDispRecTime(void)
{
    REC_TIME rec_time;
    u8	szTime[5];
	static u8	ucCol = 0;

    //DEBUG_IDU("# Get_REC Time\n");
    Get_Rec_Time(&rec_time);
    sprintf(szTime, "%02d%02d", rec_time.min, rec_time.sec);
    iduDisplayDigitNum(szTime, 1, 4);
    ucCol = 1 - ucCol;
    iduControlSymbolCol(ucCol);
}

#endif



void iduInitHT1621LCD(void)
{
#if ((HW_BOARD_OPTION == VER100_CARDVR)||(HW_BOARD_OPTION==NEW_SIYUAN))
    /*** init GROUP_LCD_CS, GROUP_LCD_RD, GROUP_LCD_WR, GROUP_LCD_SDA : gpio 3-9,10,11,12 ***/
    Gpio3Dir   &= ~0x00001E00; //output
    Gpio3Level |= 0x000000200; //set LCD_CS high
    Gpio3Ena   |= 0x00001E00;
#elif(HW_BOARD_OPTION == SIYUAN_CVR)
    /*** init GROUP_LCD_CS, GROUP_LCD_RD, GROUP_LCD_WR, GROUP_LCD_SDA : gpio 1-24,23,22,21 ***/
    Gpio1Dir   &= ~0x01E00000; //output
    Gpio1Level |= 0x010000000; //set LCD_CS high
    Gpio1Ena   |= 0x01E00000;
#endif

    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_RD, LEVEL_HIGH);
    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_WR, LEVEL_HIGH);
    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_SDA, LEVEL_HIGH);
    gpioSetLevel(GROUP_LCDMODULE, PIN_LCD_CS, LEVEL_HIGH);

    iduSendHT1621Cmd(HT1621BIAS);
    iduSendHT1621Cmd(HT1621SYSEN);
    iduSendHT1621Cmd(HT1621LCDON);
    iduClearLCD();
    #if (HW_BOARD_OPTION == VER100_CARDVR)
    gpioSetLevel(0, 23, 1);  //Enable LCD Backlight
    #endif
    iduDisplayDefaultSymbol();
}

void iduCloseHT1621LCD(void)
{
	iduSendHT1621Cmd(HT1621BIAS);
	iduSendHT1621Cmd(HT1621SYSDIS);
	iduSendHT1621Cmd(HT1621LCDOFF);
}
#endif


#if (FPGA_TEST_TV == 1)
void PaintHColorBar(u8 * buf, u16 total_width, u16 total_height)
{
    u32 height, i, j, k ;
    u8  Y, Cb, Cr, R, G, B ;
    u32 line_offset;
    u8  *ptr;

    height = total_height / 8 ;
    line_offset = total_width << 1 ;
    ptr = buf ;

    for(i=0 ; i<8 ; i++)
    {
        switch(i)
        {
            case 0:
                R = 255 ;
                G = 0 ;
                B = 0 ;
                break;
            case 1:
                R = 0 ;
                G = 255 ;
                B = 0 ;
                break;
            case 2:
                R = 0 ;
                G = 0 ;
                B = 255 ;
                break;
            case 3:
                R = 255 ;
                G = 255 ;
                B = 0 ;
                break;
            case 4:
                R = 0 ;
                G = 255 ;
                B = 255 ;
                break;
            case 5:
                R = 255 ;
                G = 0 ;
                B = 255 ;
            case 6:
                R = 0 ;
                G = 0 ;
                B = 0 ;
            case 7:
                R = 255 ;
                G = 255 ;
                B = 255 ;
                break;
        }
        Y = ((306 * R) + (601 * G) + (117 * B)) >> 10 ;
        Cb = ((-172 * R) + (-340 * G) + (512 * B) + 131072) >> 10;
        Cr = ((512 * R) + (-429 * G) + (-83 * B) + 131072) >> 10;

        for(j=0; j<(total_width/2); j++)
        {
            *ptr++ = Cr;
            *ptr++ = Cb;
            *ptr++ = Y;
            *ptr++ = Y;
        }

        for (k = (height - 1); k > 0; k--)
	    {
		    memcpy(ptr, buf, (total_width << 1));
		    ptr += line_offset;
	    }
    }
}


void FPGA_TV_Composite_Test(void)
{
    u32 i ;
    u8 mode;
    //TV Encoder reset
    tvTVE_EN=0x00000000;
	tvTVE_EN |= TV_RST;
	for(i=0;i<0xfff;i++);
	tvTVE_EN &= (~TV_RST);
	SYS_RSTCTL &= ~(0x08000000);
	SYS_RSTCTL |= 0x08000000;
	for(i=0;i<10;i++);
	SYS_RSTCTL &= ~(0x08000000);

    mode = 0 ;  // 0 is NTSC, 1 is PAL
    if(mode == 0)
        PaintHColorBar(PKBuf0, 640, 480);
    else
        PaintHColorBar(PKBuf0, 704, 480);
    if(mode == 1)//PAL
    {
#if OSD_SIZE_X2_DISABLE
		tvTVE_EN &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2);
        tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL);
#else
		tvTVE_EN &= ~(TVDAC_POWON | TVMODE_SEL | TVE_VX2 | TVE_OX2);
        tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
#endif
		  DEBUG_IDU("Trace: PAL - tvTVE_EN=%#x\n",tvTVE_EN);
        #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
            (CHIP_OPTION == CHIP_A1018B) )
          tvTV_CONF    =0x8A502C42;
        #else
          tvTV_CONF    =0x2A502C42;
        #endif
          DEBUG_IDU("Trace: PAL - tvTV_CONF=%#x\n",tvTV_CONF);
          tvTV_CTRL    =0x03FF0AA7;
          tvBURSTCFG   =0x00260065;    //Color burst config: height,incremental value
          tvCORING     =0x00000000;    //Coring threshold
          tvHTOTAL     =0x000006C0;    //Htotal=1728/2=864 pixel/line
          tvCSYNCCFG   =0x008000E0;    //Hsync config: height,incremental value

          tvBLANKCFG   =0x001400fc;    //Blank config: Blank level value,setup value
          tvHSYNC_WIDTH=0x01b2007e;    //Hsync width config.

          tvBURSTSTAEND=0x00d40097;    //Color burst start/end position

          //704x576
          tvACTSTAEND = 0x06A30123;
          tvTVFB_STRIDE=704/2;
          DEBUG_IDU("Trace: PAL - tvACTSTAEND=%#x\n", tvACTSTAEND);
          DEBUG_IDU("Trace: PAL - tvTVFB_STRIDE=%#x\n",tvTVFB_STRIDE);


          tvYUVGAIN_REG = 0x00c08296;    //RGB Gain

          tvSUB_CAR_FR =0x2A0925CB;    //subcarrier freq.

          tvDACVAL     =0x01500020;    //for DAC test,y,uv out
          tvDACVAL1    =0x00000005;    //for DAC test,cvbs out

          tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
          tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
          tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
          tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black
          tvTOFB_STRIDE = 0;

    }
    else//NTSC
    {
#if OSD_SIZE_X2_DISABLE
        tvTVE_EN &= ~(TVDAC_POWON | TVMODE_SEL);
        tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL );
#else
        tvTVE_EN &= ~(TVDAC_POWON | TVMODE_SEL | TVE_OX2);
        tvTVE_EN |= (TVDAC_POWON | TVMODE_SEL | TVE_OX2);
#endif
        DEBUG_IDU("Trace: NTSC - tvTVE_EN=%#x\n",tvTVE_EN);

        #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
            (CHIP_OPTION == CHIP_A1018B) )
          tvTV_CONF    =0x8A502C42;
        #else
          tvTV_CONF    =0x2A502C42;
        #endif
        DEBUG_IDU("Trace: NTSC - tvTV_CONF=%#x\n",tvTV_CONF);


        tvTV_CTRL    =0x03FF0AA5;    //03ff0aa2
        tvCORING     =0x00000000;    //Coring threshold
        tvHTOTAL     =0x000006b4;    //Htotal=1716/2=858 pixel/line
        tvCSYNCCFG   =0x008000E0;    //Hsync config: height,incremental value

        tvBURSTCFG   =0x00280072;    //Color burst config: height,incremental value
        tvBLANKCFG   =0x001C00e0;    //Blank config: Blank level value,setup value
        tvHSYNC_WIDTH=0x01b2007e;    //Hsync width config.

        tvBURSTSTAEND=0x00d40090;    //Color burst start/end position

        //640x480
        tvACTSTAEND = 0x064A0148;
        tvTVFB_STRIDE=640/2;
        DEBUG_IDU("Trace: NTSC - tvACTSTAEND=%#x\n", tvACTSTAEND);
        DEBUG_IDU("Trace: NTSC - tvTVFB_STRIDE=%#x\n", tvTVFB_STRIDE);


        //tvYUVGAIN_REG=0x00b680a9;    //RGB Gain
        tvYUVGAIN_REG =  0x00c08296;
        tvSUB_CAR_FR =0x21f07c20;    //subcarrier freq.

        tvDACVAL     =0x01500020;    //for DAC test,y,uv out
        tvDACVAL1    =0x00000005;    //for DAC test,cvbs out
        tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
        tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
        tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
        tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black
        tvTOFB_STRIDE = 0;
    }

    //Frame control
    tvFRAME_CTL = 0x00000001;   // Video Display Enable
    IduVidBuf0Addr = (u32)PKBuf0 ;

    //TV-Encoder Start
    tvTVE_EN |=TVE_EN;           //Trun on TV-Encoder

}
#endif

#if FPGA_TEST_DTV_YUV656

void IDU_601_Init()
{
        u8 Clk_Div;

        Clk_Div = 0;
        IduWinCtrl |= IDU_CTL_VDO_ENA;
        IduDispCfg =((Clk_Div << IDU_CLKDIV_SHFT) & IDU_CLKDIV_MASK) |
                    IDU_RGBTYPE_3|
                    IDU_DB_W_8|
                    IDU_YUV_SWAP_1 |
                    //IDU_HS_POL_HIGH |
                    //IDU_VS_POL_HIGH |
                    //IDU_INTF_CCIR601;
                    IDU_INTF_SPI;

        IduEna    &=    ( ~IDU_ROT_DEN);
        IduEna    &=    (~IDU_DAC_PWN_ON);
        IduEna    &=    (~IDU_TV_MODE_ENA);
}


void FPGA_YUV601_Test(void)
{
    u32 temp, i, j, k;
    u32 bar = 10 ;
    u8 *ptr ;

    sysIDU_enable();

    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000; //24MHz

    //IduEna  =   IDU_DCLK_EN;

    //Frame buffer init
    ptr = PKBuf0 ;
    DEBUG_IDU("#@ ptr addr =%x \n", ptr);
    for(i=0 ; i<bar ; i++)
    {
        for(j=0 ; j<48 ; j++)
        {
            for(k=0 ; k<360 ; k++)
            {
            #if 1
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            #else
                *ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
            #endif

            }
        }
    }

    IDU_601_Init();

    // Initial Code
    DEBUG_IDU("# Write Initial Code to LCD\n");
    IduMpuCmdCfg = 0x000A0028;
    IduMpuCmd = 0x00006087;  //CCIR 656 mode
    iduWaitCmdBusy();
    DEBUG_IDU("# Write Initial Code to LCD done\n");

    // TV 656 digital output

    tvTVE_EN = 0x00000000;
    tvTVE_EN = 0x00000001;
    tvFRAME_CTL = 0x00000001;
    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
    tvTV_CONF = 0x8c5084f2;
    #else
    tvTV_CONF = 0x2c5084f2;
    #endif
    tvTVE_INTC = 0x00008600;
    tvTV_CTRL = 0x03ff22a2;
    tvCORING = 0x00000000;
    tvHTOTAL = 0x000006b4;
    tvCSYNCCFG = 0x0c2800e0;
    tvBURSTCFG = 0x00280072;
    tvBLANKCFG = 0x000000e0;
    tvHSYNC_WIDTH = 0x0116007e;
    tvOSD0_WSP = 0x00000000;
    tvOSD0_WEP = 0x00000000;
    tvOSD1_WSP = 0x00000000;
    tvOSD1_WEP = 0x00000000;
    tvOSD2_WSP = 0x00000000;
    tvOSD2_WEP = 0x00000000;
    tvVACTSTAEND = 0x00f00000;
    tvBURSTSTAEND = 0x00d40090;
    tvACTSTAEND = 0x069C00fe;
    tvOSD_PAL0 = 0x00f85848;
    tvOSD_PAL1 = 0x0008a8a7;
    tvOSD_PAL2 = 0x009308d4;
    tvOSD_PAL3 = 0x00808000;
    tvOSD_PAL4 = 0x008080ff;
    tvOSD_PAL5 = 0x008080ff;
    tvOSD_PAL6 = 0x008080ff;
    tvOSD_PAL7 = 0x008080ff;
    tvOSD_PAL8 = 0x008080ff;
    tvOSD_PAL9 = 0x008080ff;
    tvOSD_PAL10 = 0x008080ff;
    tvOSD_PAL11 = 0x008080ff;
    tvOSD_PAL12 = 0x008080ff;
    tvOSD_PAL13 = 0x008080ff;
    tvOSD_PAL14 = 0x008080ff;
    tvOSD_PAL15 = 0x008080ff;
    IduVidBuf0Addr = (u32)PKBuf0;
    IduVidBuf1Addr = 0x80400000;
    IduVidBuf2Addr = 0x80500000;
    tvTVFB_STRIDE = 0x00000168;
    tvTOFB_IDDR0 = 0x80600000;
    tvTOFB_IDDR1 = 0x80800000;
    tvTOFB_IDDR2 = 0x80900000;
    tvTOFB_STRIDE = 0x001e641e;
    tvFIFO_TH = 0x40008401;
    tvYUVGAIN_REG =  0x00c08296;
    tvSUB_CAR_FR = 0x21f07c20;
    tvDACVAL = 0x01500020;
    tvDACVAL1 = 0x00000005;
    tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
    tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
    tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
    tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black

    tvTVE_EN = 0x000000c2;

}

#endif

#if FPGA_TEST_DTV_720P_HDMI
void FPGA_720P_HDMI_Test(int SrcWidth,int SrcHeight)
{
    u32 temp, i, j, k;
    u32 bar = 10 ;
    u8 *ptr ;
    i=0;
    ptr = PKBuf0 ;
    DEBUG_IDU("#@ ptr addr =%x \n", ptr);
	DEBUG_IDU("#@ SrcWidth =%x \n", SrcWidth);
	DEBUG_IDU("#@ SrcHeight =%x \n", SrcHeight);

	for(j=0 ; j<(SrcHeight/2); j++)
    {
    	for(k=0 ; k<SrcWidth/2 ; k++)
		{
			if(k<10)
			{
				*ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x00 ;
                *ptr++ = 0x00 ;
			}
			else
			{
            if(k<SrcWidth/4)
            {
            	if(k<SrcWidth/8)
            	{
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            	}
				else
				{
				*ptr++ = (20 * 10) ;
                *ptr++ = (20 * 10) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
				}
            }
            else
            {
                *ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
            }
    		}
        }
	}
	for(j=(SrcHeight/2) ; j<SrcHeight; j++)
    {
    	for(k=0 ; k<SrcWidth/2 ; k++)
		{
            if(k<SrcWidth/4)
            {
            	*ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
    		}
            else
            {
                if(k<SrcWidth/8)
            	{
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            	}
				else
				{
				*ptr++ = (20 * 10) ;
                *ptr++ = (20 * 10) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
				}
    		}
        }
	}
    /*for(i=0 ; i<bar ; i++)
    {
        for(j=0 ; j<(SrcHeight/bar); j++)
        {
            for(k=0 ; k<SrcWidth/2 ; k++)
            {
            #if 1
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            #else
                *ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
            #endif

            }
        }
		DEBUG_IDU("i = %d \n", i);
    }*/
    DEBUG_IDU("=========== edison test ===========\n");
	iduPlaybackFrame(PKBuf0);
}
#endif

#if FPGA_TEST_DTV_720P_EP952_HDMI

void FPGA_720P_Test(void)
{
    u32 temp, i, j, k;
    u32 bar = 10 ;
    u8 *ptr ;


    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000; //48MHz

    ptr = PKBuf0 ;
    DEBUG_IDU("#@ ptr addr =%x \n", ptr);
    for(i=0 ; i<bar ; i++)
    {
        for(j=0 ; j<720/bar ; j++)
        {
            for(k=0 ; k<1280/2 ; k++)
            {
            #if 1
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            #else
                *ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
            #endif

            }
        }
    }



    SYS_TV_PLLCTL = 0x00026237 ;
    for(i=0 ; i<200 ; i++);
    SYS_TV_PLLCTL = 0x00036237 ;
    SYS_TV_PLLCTL = 0x0001e237 ;

    GpioActFlashSelect |= 0x00000004 ;
    tvBT656CONF = 0X00000000 ;
    tvTVE_EN = 0x00000000;
    tvTVE_EN = 0x00000001;
    tvFRAME_CTL = 0x00000001;

    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
    tvTV_CONF = 0x8d6030f1;  //Color bar enable
    #else
    tvTV_CONF = 0x2d6030f1;  //Color bar enable
    #endif

    tvTVE_INTC = 0x00008600;
    tvTV_CTRL = 0x03ff22a2;
    tvCORING = 0x00000000;
    tvHTOTAL = 0x00000672;
    tvCSYNCCFG = 0x0c2800ec;
    tvBURSTCFG = 0x00280072;
    tvBLANKCFG = 0x000000ec;
    //tvHSYNC_WIDTH = 0x01710028;
	tvHSYNC_WIDTH = 0x01720028;
    tvOSD0_WSP = 0x00000000;
    tvOSD0_WEP = 0x00000000;
    tvOSD1_WSP = 0x00000000;
    tvOSD1_WEP = 0x00000000;
    tvOSD2_WSP = 0x00000000;
    tvOSD2_WEP = 0x00000000;
    tvVACTSTAEND = 0x02d00000;
    tvBURSTSTAEND = 0x00d40090;
    //tvACTSTAEND = 0x06010100;
	tvACTSTAEND = 0x06010101;
    tvOSD_PAL0 = 0x00f85848;
    tvOSD_PAL1 = 0x0008a8a7;
    tvOSD_PAL2 = 0x009308d4;
    tvOSD_PAL3 = 0x00808000;
    tvOSD_PAL4 = 0x008080ff;
    tvOSD_PAL5 = 0x008080ff;
    tvOSD_PAL6 = 0x008080ff;
    tvOSD_PAL7 = 0x008080ff;
    tvOSD_PAL8 = 0x008080ff;
    tvOSD_PAL9 = 0x008080ff;
    tvOSD_PAL10 = 0x008080ff;
    tvOSD_PAL11 = 0x008080ff;
    tvOSD_PAL12 = 0x008080ff;
    tvOSD_PAL13 = 0x008080ff;
    tvOSD_PAL14 = 0x008080ff;
    tvOSD_PAL15 = 0x008080ff;
    IduVidBuf0Addr = (u32)PKBuf0;
    IduVidBuf1Addr = 0x80400000;
    IduVidBuf2Addr = 0x80500000;
    tvTVFB_STRIDE = 0x00000280;
    tvTOFB_IDDR0 = 0x80600000;
    tvTOFB_IDDR1 = 0x80800000;
    tvTOFB_IDDR2 = 0x80900000;
    tvTOFB_STRIDE = 0x001e641e;
    tvFIFO_TH = 0x40008401;
    tvYUVGAIN_REG = 0x00c08296;
    tvSUB_CAR_FR = 0x21f07c20;
    tvDACVAL = 0x01500020;
    tvDACVAL1 = 0x00000005;
    tvWHITEYEL   =0x01ae002e;    //Color bar config: white,yellow
    tvCYANGRN    =0x03180264;    //Color bar config: cyan,green
    tvMAGRED     =0x047703c6;    //Color bar config: magenta, red
    tvBLUEBLACK  =0x05da0526;    //Color bar config: blue,black

    tvTVE_EN = 0x000000c2;

}

#endif

#if FPGA_TEST_DTV_1080I_EP952_HDMI
void FPGA_1080I_Test(void)
{
    u32 temp, i, j, k;
    u32 bar = 10 ;
    u8 *ptr ;

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    temp= SYS_CLK1;
    SYS_CLK1= (temp & (~0x0000ff00)) | 0x00000000; //48MHz
#elif((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
    //----Set TV PLL clock=74.25 MHz.----//
    SYS_TV_PLLCTL = 0x00026237 ;
    for(i=0 ; i<200 ; i++);
    SYS_TV_PLLCTL = 0x00036237 ;
    SYS_TV_PLLCTL = 0x0001e237 ;
#elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    SYS_CPU_PLLCTL &=  (~0x00000080); //idu clock from sysclk
    SYS_DDR_PLLCTL=0x6304c001;
    #if(CHIP_OPTION == CHIP_A1018A)
    SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00010000;  //594/2=297, 297/4=74.25
    #elif(CHIP_OPTION == CHIP_A1018B)
    SYS_CLK4 = (SYS_CLK4 & (~0x000f0000)) | 0x00030000;  //594/4=148.5, 148.5/2=74.25
    #endif
    for(i=0 ; i<200 ; i++);
    SYS_CPU_PLLCTL |=  (0x00000080); //idu clock from DPLL
#else

#endif


    ptr = PKBuf0 ;
    DEBUG_IDU("#@ ptr addr =%x \n", ptr);
    for(i=0 ; i<bar ; i++)
    {
        for(j=0 ; j<1080/bar ; j++)
        {
            for(k=0 ; k<1920/2 ; k++)
            {
            #if 1
            	*ptr++ = (20 * i) ;
                *ptr++ = (20 * i) ;
                *ptr++ = 0x80 ;
                *ptr++ = 0x80 ;
            #else
                *ptr++ = 0x4c ;
                *ptr++ = 0x4c ;
                *ptr++ = 0xff ;
                *ptr++ = 0x54 ;
            #endif

            }
        }
    }



    SYS_TV_PLLCTL = 0x00026237 ;
    for(i=0 ; i<200 ; i++);
    SYS_TV_PLLCTL = 0x00036237 ;
    SYS_TV_PLLCTL = 0x0001e237 ;

    GpioActFlashSelect |= 0x00000004 ;
    tvBT656CONF = 0X00000000 ;
    tvTVE_EN = 0x00000000;
    tvTVE_EN = 0x00000001;
    tvFRAME_CTL = 0x00000001;

    #if ( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) )
    tvTV_CONF = 0x8e6030f1;  //Color bar enable
    #else
    tvTV_CONF = 0x2e6030f1;  //Color bar enable
    #endif
    tvTVE_INTC = 0x00008600;
    tvTV_CTRL = 0x03ff22a2;
    tvCORING = 0x00000000;
    tvHTOTAL = 0x00000898;
    tvCSYNCCFG = 0x0c2800ec;
    tvBURSTCFG = 0x00280072;
    tvBLANKCFG = 0x000000ec;

	tvHSYNC_WIDTH = 0x01180028;

    tvOSD0_WSP = 0x00000000;
    tvOSD0_WEP = 0x00000000;
    tvOSD1_WSP = 0x00000000;
    tvOSD1_WEP = 0x00000000;
    tvOSD2_WSP = 0x00000000;
    tvOSD2_WEP = 0x00000000;

    tvVACTSTAEND = 0x021C0000;

    tvBURSTSTAEND = 0x00d40090;

	  tvACTSTAEND = 0x084100c1;

    tvOSD_PAL0 = 0x00f85848;
    tvOSD_PAL1 = 0x0008a8a7;
    tvOSD_PAL2 = 0x009308d4;
    tvOSD_PAL3 = 0x00808000;
    tvOSD_PAL4 = 0x008080ff;
    tvOSD_PAL5 = 0x008080ff;
    tvOSD_PAL6 = 0x008080ff;
    tvOSD_PAL7 = 0x008080ff;
    tvOSD_PAL8 = 0x008080ff;
    tvOSD_PAL9 = 0x008080ff;
    tvOSD_PAL10 = 0x008080ff;
    tvOSD_PAL11 = 0x008080ff;
    tvOSD_PAL12 = 0x008080ff;
    tvOSD_PAL13 = 0x008080ff;
    tvOSD_PAL14 = 0x008080ff;
    tvOSD_PAL15 = 0x008080ff;
    IduVidBuf0Addr = (u32)PKBuf0;
    IduVidBuf1Addr = 0x80400000;
    IduVidBuf2Addr = 0x80500000;

    tvTVFB_STRIDE = 0x000003c0;

    tvTOFB_IDDR0 = 0x80600000;
    tvTOFB_IDDR1 = 0x80800000;
    tvTOFB_IDDR2 = 0x80900000;
    tvTOFB_STRIDE = 0x001e641e;
    tvFIFO_TH = 0x40008401;
    tvYUVGAIN_REG = 0x00c08296;
    tvSUB_CAR_FR = 0x21f07c20;
    tvDACVAL = 0x01500020;
    tvDACVAL1 = 0x00000005;

    tvWHITEYEL   =0x01ee00fe;    //Color bar config: white,yellow
    tvCYANGRN    =0x03ce02de;    //Color bar config: cyan,green
    tvMAGRED     =0x05ae04be;    //Color bar config: magenta, red
    tvBLUEBLACK  =0x078e069e;    //Color bar config: blue,black

    tvTVE_EN = 0x000000c2;

}

#endif

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
static u32 IduWinCtrl_status,IduOsdL1Ctrl_status;
void Signal_off(void)
{
	IduWinCtrl_status=IduWinCtrl;
	IduOsdL1Ctrl_status=IduOsdL1Ctrl;
	IduWinCtrl=IduOsdL1Ctrl=0x0;
}
void Signal_on(void)
{
	IduWinCtrl=IduWinCtrl_status;
	IduOsdL1Ctrl=IduOsdL1Ctrl_status;
}
#endif
void PANEL_ON(void)
{
#if (HW_BOARD_OPTION == MR8120_RX_HECHI)
	gpioSetLevel(1 ,23 ,GPIO_ENA);		//POWER_ON
	gpioSetLevel(GPIO_GROUP_RESET ,GPIO_BIT_RESET ,GPIO_ENA);		//RESET_ON
	gpioSetEnable(0 ,14 ,GPIO_DISA);		//VLCD_OFF
	gpioSetLevel(GPIO_GROUP_LCD_EN ,GPIO_BIT_LCD_EN ,GPIO_ENA);		//VLCD_ON
	gpioSetEnable(2, 0, 1);             //data_out
	OSTimeDly(5);
	gpioSetLevel(GPIO_GROUP_BACK_LIGHT ,GPIO_BIT_BACK_LIGHT ,GPIO_ENA);	//BL_ON

	gpioSetLevel(1 ,24 ,0);				//LED_ON
	DEBUG_IDU("---->PANEL_ON\n\n");
#endif
}
void PANEL_OFF(void)
{
#if (HW_BOARD_OPTION == MR8120_RX_HECHI)
	gpioSetLevel(GPIO_GROUP_BACK_LIGHT , GPIO_BIT_BACK_LIGHT , GPIO_DISA); //BL_OFF
	gpioSetEnable(2, 0, 0);             //data_shut
	gpioSetEnable(0 ,14 ,GPIO_ENA);		//VLCD_OFF
	gpioSetLevel(GPIO_GROUP_LCD_EN ,GPIO_BIT_LCD_EN ,GPIO_DISA);		//VLCD_OFF
	gpioSetLevel(GPIO_GROUP_RESET ,GPIO_BIT_RESET ,GPIO_DISA);		//RESET_OFF
    gpioSetLevel(1 ,23 ,GPIO_DISA);		//POWER_OFF

    gpioSetLevel(1 ,25 ,1);				//MUSIC_STOP_ON
	gpioSetLevel(1 ,24 ,1);				//LED_OFF
	gpioSetLevel(GPIO_GROUP_SPK_EN ,GPIO_BIT_SPK_EN ,0);				//SPK_OFF
	DEBUG_IDU("---->PANEL_OFF\n\n");
#endif
}

void iduChangeDispSrcWidthHeight(int Width, int Height)
{
    BRI_IN_SIZE = (Height << 16) | Width;
}

#if(HWPIP_SUPPORT)

int HWPIP_CH_Sel(u8 addr)
    {
    switch(addr)
    {
        case 1: // ciu1
            sysOsdInCHsel = PIP_MAIN_CH1;
            break;
        case 2: // ciu2
            sysOsdInCHsel = PIP_MAIN_CH2;
            break;
        default:
            sysOsdInCHsel = PIP_MAIN_NONE;
            return 0;
}
    return 1;

}

int HWPIP_Position(u32 S_X,u32 S_Y)
{
    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        PIP_S_X = S_X;
        PIP_S_Y = S_Y;
        PIP_E_X = PIP_S_X + PIP_IN_X;
        PIP_E_Y = PIP_S_Y + PIP_IN_Y;
        if((PIP_E_X > PIPDispWidth[sysTVOutOnFlag]))
        {
            DEBUG_IDU("Warning!,PIP_S_X is  %d out of range %d\r\n",PIP_E_X,PIPDispWidth[sysTVOutOnFlag]);
            return 0;
        }
        if((PIP_E_Y > PIPDispHeight[sysTVOutOnFlag]))
        {
            DEBUG_IDU("Warning!,PIP_E_Y is  %d out of range %d\r\n",PIP_E_Y,PIPDispHeight[sysTVOutOnFlag]);
            return;
        }     
        OSD_END_X    = PIP_E_X | (PIP_E_Y<<16) ;
        OSD_START_X  = PIP_S_X | (PIP_S_Y<<16) ;
    }
    else
    {
        DEBUG_IDU("Warning!,PIP disable\r\n");
        return 0;
    }
    return 1;
}

int HWPIP_OSDSize(u32 IN_X,u32 IN_Y)
{
    u8 downdample;
    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        if(IN_X < 96)
        {
            DEBUG_IDU("Warning!!,PIP_IN_X must be greater than to 96\r\n");
            return 0;
        }
        PIP_IN_X = IN_X;
        PIP_IN_Y = IN_Y;
        PIP_E_X = PIP_S_X + PIP_IN_X;
        PIP_E_Y = PIP_S_Y + PIP_IN_Y;
        if((PIP_E_X > PIPDispWidth[sysTVOutOnFlag]))
        {
            DEBUG_IDU("Warning!,PIP_S_X is  %d out of range %d\r\n",PIP_E_X,PIPDispWidth[sysTVOutOnFlag]);
            return 0;
        }
        if((PIP_E_Y > PIPDispHeight[sysTVOutOnFlag]))
        {
            DEBUG_IDU("Warning!,PIP_E_Y is  %d out of range %d\r\n",PIP_E_Y,PIPDispHeight[sysTVOutOnFlag]);
            return 0;
        }
        downdample = (BRI_CTRL_REG & 0x300)>>8;
        OSD_END_X = PIP_E_X | (PIP_E_Y<<16) ;
        switch(downdample)
        {
            case 0:
                OSD_IN_WIDTH = (PIP_IN_X | PIP_IN_Y<<16);
                break;
            case 1:
                OSD_IN_WIDTH = (PIP_IN_X | PIP_IN_Y<<16)<<1;
                break;
            case 2:
                OSD_IN_WIDTH = (PIP_IN_X | PIP_IN_Y<<16)<<2;
                break;
            default:
                DEBUG_IDU("Warning,HWPIP_OSDSize\r\n");
                return 0;
        }
    }
    else
    {
        DEBUG_IDU("Warning!,PIP disable\r\n");
        return 0;
}
    return 1;
}
int HWPIP_AlphaBlendingSet(u8 Blending)
{
    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        BRI_CTRL_REG &= ~(0xC00); // reset 10-11
        switch(Blending)
        {
            case 0:
                BRI_CTRL_REG |= OSD_BLENDING_100;
                break;
            case 1:
                BRI_CTRL_REG |= OSD_BLENDING_075;
                break;
            case 2:
                BRI_CTRL_REG |= OSD_BLENDING_050;
                break;
            case 3:
                BRI_CTRL_REG |= OSD_BLENDING_025;
                break;
            default:
                DEBUG_IDU("Warning,HWPIP_AlphaBlending \r\n");
                return 0;
        }
    }
    else
    {
        DEBUG_IDU("Warning!,PIP disable\r\n");
        return 0;
}

    return 1;
}

int HWPIP_DownSampleSet(u8 Downsample)
{
    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        BRI_CTRL_REG &= ~(0x300); // reset 8-9
        switch(Downsample)
        {
            case 0:
                BRI_CTRL_REG |= OSD_DOWNSAMPLE_1;
                OSD_IN_WIDTH = OSD_END_X - OSD_START_X;
                break;
            case 1:
                BRI_CTRL_REG |= OSD_DOWNSAMPLE_2;
                OSD_IN_WIDTH = (OSD_END_X - OSD_START_X)<<1;
                break;
            case 2:
                BRI_CTRL_REG |= OSD_DOWNSAMPLE_4;
                OSD_IN_WIDTH = (OSD_END_X - OSD_START_X)<<2;
                break;
            default:
                DEBUG_IDU("Warning,HWPIP_DownSample\r\n");
                return 0;
        }
    }
    else
    {
        DEBUG_IDU("Warning!,PIP disable\r\n");
        return 0;
}
    return 1;
}
void HWPIP_EdgeColorSet(u32 color) //KHPE
{
    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        OSD_EDGE_COLOR = color;
    }
    else
        DEBUG_IDU("Warning!,PIP disable\r\n");
}
void HWPIP_EdgeColorEn(u8 Edge_En)
{

    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {

        BRI_CTRL_REG &= ~(0x1000); // reset 12
        switch(Edge_En)
        {
            case 0:
            DEBUG_IDU("Close OSD_Edge_Color  \r\n ");
                BRI_CTRL_REG |= EDGE_COLOR_DISA;
                break;
            case 1:
                DEBUG_IDU("Open OSD_Edge_Color  \r\n ");
                BRI_CTRL_REG |= EDGE_COLOR_EN;
                break;
            default:
                DEBUG_IDU("Warning!,HWPIP_EdgeColor_Set\r\n");
                break;
        }
    }
    else
        DEBUG_IDU("Warning!,PIP disable\r\n");

}
int HWPIP_BurstBitwidthSet(u8 B8_OSD,u8 B8_VDO,u8 BW_64)
{

    if((BRI_CTRL_REG & BRI_OSD_EN) == BRI_OSD_EN)
    {
        BRI_CTRL_REG &= ~(0x70000); // reset 16-18
        if(B8_OSD == 1)
            BRI_CTRL_REG |= BURST_8_OSD_8;
        else
            BRI_CTRL_REG |= BURST_8_OSD_16;

        if(B8_VDO == 1)
            BRI_CTRL_REG |= BURST_8_VIDEO_8;
        else
            BRI_CTRL_REG |= BURST_8_VIDEO_16;

        if(BW_64 == 1)
            BRI_CTRL_REG |= BITWIDTH64_EN;
        else
            BRI_CTRL_REG |= BITWIDTH64_DISA;
    }
    else
    {
        DEBUG_IDU("Warning!,PIP disable\r\n");
        return 0;
    }
    return 1;
}

void HWPIP_BRI_En(u8 BRI_En)
{
    BRI_CTRL_REG &= ~(0x40); // reset 6
    if(BRI_En == 1)
        BRI_CTRL_REG |= BRI_OSD_EN;
    else if(BRI_En == 0)
        BRI_CTRL_REG |= BRI_OSD_DISA;

}
void HWPIP_Open(u8 BRI_En,        // 0:Dis  1:En
                        u8 Downsample,  // 0:1/1  1:1/2  2:1/4
                        u8 Blending,    // 0:100  1:75   2:50   3:25
                        u8 Edge_En,     // 0:Dis  1:En
                        u8 B8_OSD,      // 0:16   1:8
                        u8 B8_VDO,      // 0:16   1:8
                        u8 W64_En)      // 0:Dis  1:En
{
    HWPIP_BRI_En(BRI_En);
    HWPIP_DownSampleSet(Downsample);
    HWPIP_AlphaBlendingSet(Blending);
    HWPIP_EdgeColorEn(Edge_En);
    HWPIP_BurstBitwidthSet(B8_OSD,B8_VDO,W64_En);
}
void uiHWPIP_TEST1(u8 *cmd)
{
    u32 OSD_E_X,OSD_E_Y,OSD_S_X,OSD_S_Y;
    u32 OSD_IN_X,OSD_IN_Y;
    DEBUG_UI("uiHWPIP_TEST1 \n");

    sscanf((char*)cmd, "%d %d %d %d", &OSD_S_X, &OSD_S_Y, &OSD_IN_X, &OSD_IN_Y);
    DEBUG_UI("Set OSD_START_X (%d,%d) OSD_IN_WIDTH (%d,%d)\r\n",OSD_S_X, OSD_S_Y, OSD_IN_X, OSD_IN_Y);

    OSD_E_X = OSD_IN_X + OSD_S_X;
    OSD_E_Y = OSD_IN_Y + OSD_S_Y;
    OSD_ADDR_Y   = BRI_IADDR_Y ;
    OSD_ADDR_C   = BRI_IADDR_C ;
    OSD_STRIDE   = 0x00000280 ;
    OSD_IN_WIDTH = OSD_IN_X | (OSD_IN_Y<<16);
    OSD_END_X    = OSD_E_X | (OSD_E_Y<<16) ;
    OSD_START_X  = OSD_S_X | (OSD_S_Y<<16) ;
    BRI_CTRL_REG = 0x00041072 ;
    BRI_IN_SIZE  = 0x00f00280 ;
    BRI_OUT_SIZE = 0x00f002C0 ;
    /*OSD_ADDR_Y   = 0x80DA1600 ;
    OSD_ADDR_C   = 0x80E06A00 ;
    OSD_STRIDE   = 0x00000280 ;
    OSD_IN_WIDTH = 0x005000A0 ;
    OSD_END_X    = 0x00B00100 ;
    OSD_START_X  = 0x00600060 ;
    BRI_CTRL_REG = 0x00041072 ;
    BRI_IN_SIZE  = 0x00f00280 ;
    BRI_OUT_SIZE = 0x00f002C0 ;*/
}
void uiHWPIP_TEST2(void)
{
    u32 i=0,j=0;
    u32 OSD_IN_X,OSD_IN_Y;
    u32 OSD_E_X,OSD_E_Y,OSD_S_X,OSD_S_Y;
    //DEBUG_UI("BRI_CTRL_REG & 0x1000 = %x \n",BRI_CTRL_REG & 0x1000);
    if((BRI_CTRL_REG & 0x1000) == 0x1000)
    {
        DEBUG_UI("uiHWPIP_TEST2 \n");
        OSD_IN_X = OSD_IN_WIDTH & 0x0000FFFF;
        OSD_IN_Y = (OSD_IN_WIDTH & 0xFFFF0000)>>16;
        for(j=0;j<480;j++)
        {
            if((j+OSD_IN_Y)>PIPDispHeight[sysTVOutOnFlag])
                break;
            for(i=0;i<PIPDispWidth[sysTVOutOnFlag];i+=4)
            {
            if((i+OSD_IN_X)>PIPDispWidth[sysTVOutOnFlag])
                break;
            OSD_E_X = i+OSD_IN_X;
            OSD_E_Y = j+OSD_IN_Y;
            OSD_END_X    = OSD_E_X | (OSD_E_Y<<16) ;
            OSD_START_X  = i | (j<<16) ;
            DEBUG_UI(" pos (%d,%d)\n",i,j);
            OSTimeDly(1);
            }
        }
        DEBUG_UI("End uiHWPIP_TEST2 \n");
    }
    else
        DEBUG_UI("Error!!,please open PIP! \n");
    
}
void uiHWPIP_TEST3(u8 *cmd)
{
    u32 i=0,j=0;
    u32 OSD_IN_X,OSD_IN_Y;
    u32 OSD_E_X,OSD_E_Y,OSD_S_X,OSD_S_Y;
    sscanf((char*)cmd, "%d", &j);
    if((BRI_CTRL_REG & 0x1000) == 0x1000)
    {
        DEBUG_UI("uiHWPIP_TEST3 \n");
        OSD_IN_X = OSD_IN_WIDTH & 0x0000FFFF;
        OSD_IN_Y = (OSD_IN_WIDTH & 0xFFFF0000)>>16;
        for(i=0;i<704;i+=4)
        {
            if((i+OSD_IN_X)>PIPDispWidth[sysTVOutOnFlag] || (j+OSD_IN_Y)>PIPDispHeight[sysTVOutOnFlag])
                break;
            OSD_E_X = i+OSD_IN_X;
            OSD_E_Y = j+OSD_IN_Y;
            OSD_END_X    = OSD_E_X | (OSD_E_Y<<16) ;
            OSD_START_X  = i | (j<<16) ;
            DEBUG_UI(" pos (%d,%d)\n",i,j);
            OSTimeDly(10);
        }
        DEBUG_UI("End uiHWPIP_TEST3 \n");
    }
    else
        DEBUG_UI("Error!!,please open PIP! \n");
    
}

#endif

void OSD_Black_Color_Bar(u8 bEnable)
{
    if(sysTVOutOnFlag)   /* A1016 color bar can not full screen cover black */
    {                    /* It can set YUV gain to 0, than the sreen cover full black */
        if(bEnable==1)
        {
            gTVColorbarEn=1;
            tvYUVGAIN_REG=0;
        }
        else
        {
            gTVColorbarEn=0;
            if(sysTVinFormat == TV_IN_NTSC )
            {
                tvYUVGAIN_REG=TV_YUV_GAIN;
            }
            else
            {
                tvYUVGAIN_REG=TV_YUV_GAIN_PAL;    
            }
        }
    }
}
