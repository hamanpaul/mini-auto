/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sysapi.h

Abstract:

   	The application interface of mode change.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SYS_API_H__
#define __SYS_API_H__

#include "general.h"

#if (HOME_RF_SUPPORT)
#include "../LwIP/include/tutk_P2P/MR8200def_homeautomation.h"
#endif
#if CDVR_SYSTEM_LOG_SUPPORT
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif

#define SYSTEM_LOG_DATA_SIZE   64

#define TVPARA_CWELL           0
#define TV_PARAMETER_SEL       TVPARA_CWELL
//----------------Storage Bus selection--------------//
/* GpioActFlashSelect */


#define GPIO_ACT_FLASH_MASK	(~0x0000)
#define GPIO_ACT_FLASH_SD	0x0000
#define GPIO_ACT_FLASH_SPI	0x0000

#define GPIO_ACT_FLASH_SMC	0x0000    //reserved
#define GPIO_ACT_FLASH_CF	0x0000    //reserved
#define GPIO_DISP_FrDV1_EN	0x0004    //replace GPIU
#define GPIO_GPIU2_FrXX_EN	0x0008

#define GPIO_DEBUGE_EN		0x0010
#define GPIO_RF12_FrSP2_EN	0x0020
#define GPIO_RF2_FrIIC_EN	0x0040
#define GPIO_DV2_FrGPI_EN2	0x0080

#define GPIO_GPIU_FrDISP_EN	0x0100
#define GPIO_EXROM_EN		0x0200    //replace GPIU
#define GPIO_PWM_EN			0x0400


#define GPIO_SPI2_FrDISP	0x1000
#define GPIO_DV2FrDISP_EN	0x2000
#define GPIO_IISFrDISP_EN	0x4000


#define GPIO_SPI3_FrUARTDV1	0x100000
//chip io configureation register

#define CHIP_IO_DV2_EN3		0x00000001
#define CHIP_IO_DISP2_EN	0x00000002
#define CHIP_IO_DISP_EN		0x00000004
#define CHIP_IO_GPI2_EN		0x00000008
#define CHIP_IO_DEBUG_EN	0x00000010
#define CHIP_IO_RFI_EN_1	0x00000020
#define CHIP_IO_RF3_EN		0x00000040
#define CHIP_IO_DV2_EN2		0x00000080
#define CHIP_IO_GPI1_EN		0x00000100
#define CHIP_IO_DIS_ROM_EN	0x00000200
#define CHIP_IO_PWM0_EN		0x00000400
#define CHIP_IO_UARTB_EN	0x00000800
#define CHIP_IO_SPI2_EN		0x00001000
#define CHIP_IO_DV2_EN		0x00002000
#define CHIP_IO_IIS_EN		0x00004000
#define CHIP_IO_RFI_ALT		0x00008000
#define CHIP_IO_DV2_EN4		0x00010000
#define CHIP_IO_FB_MODE1	0x00020000
#define CHIP_IO_FB_MODE2	0x00040000
#define CHIP_IO_CCIR2_EN	0x00080000

#if( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
#define CHIP_IO_RF13_PORT2_EN 0x00080000
#endif

#define CHIP_IO_SPI3_EN		0x00100000
#define CHIP_IO_SERDISP_EN	0x00200000
#if( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
#define CHIP_IO_RMII_EN		0x00010000
#else
#define CHIP_IO_RMII_EN		0x00400000
#endif
#define CHIP_IO_SEN_EN		0x00800000
//#define CHIP_IO_RMII2_EN	0x01000000
#define CHIP_IO_RMII_EN4	0x01000000
#define CHIP_IO_RMII_EN2	0x02000000
#define CHIP_IO_SPI3_EN2	0x04000000
#define CHIP_IO_RMII_EN3	0x08000000
#define CHIP_IO_RFI2_EN		0x10000000
#define CHIP_IO_RFI2_EN2	0x20000000
#define CHIP_IO_I2C_EN		0x40000000
#define CHIP_IO_SMPTE2_EN	0x80000000

//chip io configureation register 2

#define CHIP_IO2_MII_EN		0x00000001
#define CHIP_IO2_DISP_EN2	0x00000002
#define CHIP_IO2_RFI_ALT3	0x00000004
#define CHIP_IO2_ALT_NTX20E	0x00000008
#define CHIP_IO2_ALT_NTX10E	0x00000010
#define CHIP_IO2_RFI_ALT2	0x00000020
#define CHIP_IO2_RFI_EN2	0x00000040
#define CHIP_IO2_RFI1012_EN	0x00000080
#define CHIP_IO2_CCIR4CH_EN	0x00000100
#define CHIP_IO2_ALT_NTX30E	0x00000200
#define CHIP_IO2_RFI3_EN2	0x00000400
#define CHIP_IO2_IIS2_EN	0x00000800
#define CHIP_IO2_SD2_EN		0x00002000
#if( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
#define CHIP_IO2_PWM05_EN	0x00020000
#endif
#define CHIP_IO2_FSND890	0x80000000

//-----------------Device Reset/Enable-------------//
/*
 * SYS_CTL0
 */
#define SYS_CTL0_SDRAM_CKEN         0x00000001
#define SYS_CTL0_SRAM_CKEN          0x00000002
#define SYS_CTL0_HIU_CKEN           0x00000000
#define SYS_CTL0_RF1012_CKEN        0x00000004
#define SYS_CTL0_SIU_CKEN           0x00000008
#define SYS_CTL0_IPU_CKEN           0x00000010
#define SYS_CTL0_IDU_CKEN           0x00000020
#define SYS_CTL0_ISU_CKEN           0x00000040
#define SYS_CTL0_JPEG_CKEN          0x00000080
#if (VIDEO_CODEC_OPTION == H264_CODEC)
#define SYS_CTL0_H264_CKEN          0x00000100
#define SYS_CTL0_MPEG4_CKEN         0x00000100
#elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
#define SYS_CTL0_H264_CKEN          0x00000100
#define SYS_CTL0_MPEG4_CKEN         0x00000100
#endif
#define SYS_CTL0_STM_CKEN           0x00000200
#define SYS_CTL0_SPI_CKEN	        0x00000200
#define SYS_CTL0_GPIO0_CKEN         0x00000400
#define SYS_CTL0_GPIO1_CKEN         0x00000800
#define SYS_CTL0_GPIO2_CKEN         0x00001000
#define SYS_CTL0_UART_CKEN          0x00002000
#define SYS_CTL0_I2C_CKEN           0x00004000
#define SYS_CTL0_IIS_CKEN           0x00008000
#define SYS_CTL0_USB_CKEN           0x00010000
#define SYS_CTL0_SD_CKEN            0x00020000
#define SYS_CTL0_CF_CKEN            0x00020000
#define SYS_CTL0_NAND_CKEN          0x00000000
#define SYS_CTL0_MD_CKEN            0x00040000
#define SYS_CTL0_TIMER0_CKEN        0x00080000
#define SYS_CTL0_TIMER1_CKEN        0x00080000
#define SYS_CTL0_TIMER2_CKEN        0x00080000
#define SYS_CTL0_TIMER3_CKEN        0x00080000
#define SYS_CTL0_TIMER4_CKEN        0x00080000
#define SYS_CTL0_TIMER5_CKEN        0x00080000
#define SYS_CTL0_TIMER6_CKEN        0x00080000
#define SYS_CTL0_TIMER7_CKEN        0x00080000
#define SYS_CTL0_TIMER8_CKEN        0x00080000
#define SYS_CTL0_TIMER9_CKEN        0x00080000
#define SYS_CTL0_TIMER10_CKEN        0x00080000
#define SYS_CTL0_TIMER11_CKEN        0x00080000
#define SYS_CTL0_TIMER12_CKEN        0x00080000



#define SYS_CTL0_RF1013_CKEN        0x00100000
#define SYS_CTL0_SCUP_CKEN          0x00200000
#define SYS_CTL0_CIU_CKEN           0x00400000
#define SYS_CTL0_IR_CKEN            0x00800000

#define SYS_CTL0_RTC_CKEN           0x01000000
#define SYS_CTL0_WDT_CKEN           0x02000000
#define SYS_CTL0_GPIO3_CKEN         0x04000000
#define SYS_CTL0_CIU2_CKEN          0x08000000  //A1018A

#define SYS_CTL0_TVE_CKEN           0x00000000
#define SYS_CTL0_GPIU_CKEN          0x10000000
#define SYS_CTL0_MAC_CKEN           0x10000000
#define SYS_CTL0_MCP_CKEN           0x20000000

#define SYS_CTL0_SER_MCKEN          0x40000000
#define SYS_CTL0_ADC_CKEN           0x80000000

#define SYS_CTL0_EXT_CIU3_CKEN      0x00000001
#define SYS_CTL0_EXT_CIU4_CKEN      0x00000002
#define SYS_CTL0_EXT_DES_CKEN       0x00000004
#if (CHIP_OPTION == CHIP_A1018A)
#define SYS_CTL0_EXT_MAC2_CKEN      0x00000008
#endif
#define SYS_CTL0_EXT_IDU2_CKEN      0x00000010
#define SYS_CTL0_EXT_IDU3_CKEN      0x00000020
#define SYS_CTL0_EXT_IDU4_CKEN      0x00000040
#define SYS_CTL0_EXT_MCP2_CKEN      0x00000080
#define SYS_CTL0_EXT1_RF1013_CKEN   0x00000100
#define SYS_CTL0_EXT2_RF1013_CKEN   0x00000200
#define SYS_CTL0_EXT_DIU_CKEN       0x00000400
#define SYS_CTL0_EXT_GFX_CKEN       0x00000800
#define SYS_CTL0_EXT_USB2_CKEN      0x00001000
//------SYS RESET----//
#define SYS_RSTCTL_SDRAM_RST        0x00000001
#define SYS_RSTCTL_SSRAM_RST        0x00000002
#define SYS_RSTCTL_HIU_RST          0x00000000
#define SYS_RSTCTL_RF1012_RST       0x00000004
#define SYS_RSTCTL_SIU_RST          0x00000008
#define SYS_RSTCTL_IPU_RST          0x00000010
#define SYS_RSTCTL_IDU_RST          0x00000020
#define SYS_RSTCTL_ISU_RST          0x00000040
#define SYS_RSTCTL_JPEG_RST         0x00000080
#define SYS_RSTCTL_MPEG4_RST        0x00000100
#define SYS_RSTCTL_STM_RST          0x00000200
#define SYS_RSTCTL_GPIO0_RST        0x00000400
#define SYS_RSTCTL_GPIO1_RST        0x00000800
#define SYS_RSTCTL_GPIO2_RST        0x00001000
#define SYS_RSTCTL_UART_RST         0x00002000
#define SYS_RSTCTL_I2C_RST          0x00004000
#define SYS_RSTCTL_IIS_RST          0x00008000
#define SYS_RSTCTL_USB_RST          0x00010000
#define SYS_RSTCTL_SD_RST           0x00020000
#define SYS_RSTCTL_CF_RST           0x00020000
#define SYS_RSTCTL_NAND_RST         0x00040000
#define SYS_RSTCTL_MD_RST           0x00040000
#define SYS_RSTCTL_TIMER0_RST       0x00080000
#define SYS_RSTCTL_TIMER1_RST       0x00080000
#define SYS_RSTCTL_TIMER2_RST       0x00080000
#define SYS_RSTCTL_TIMER3_RST       0x00080000
#define SYS_RSTCTL_TIMER4_RST       0x00080000
#define SYS_RSTCTL_RF1013_RST       0x00100000
#define SYS_RSTCTL_SCUP_RST         0x00200000
#define SYS_RSTCTL_CIU_RST          0x00400000
#define SYS_RSTCTL_IR_RST           0x00800000
#define SYS_RSTCTL_RTC_RST          0x01000000
#define SYS_RSTCTL_WDT_RST          0x02000000
#define SYS_RSTCTL_GPIO3_RST        0x04000000
//#define SYS_RSTCTL_TVE_RST          0x08000000
#define SYS_RSTCTL_CIU2_RST         0x08000000
#define SYS_RSTCTL_GPI_RST          0x10000000
#define SYS_RSTCTL_MCP_RST          0x20000000

#define SYS_RSTCTL_BIT30_RST        0x40000000
#define SYS_RSTCTL_ADCRX_RST        0x80000000

#define SYS_CTL0_EXT_CIU3_RST      0x00000001
#define SYS_CTL0_EXT_CIU4_RST      0x00000002
#define SYS_CTL0_EXT_DES_RST       0x00000004
#if (CHIP_OPTION == CHIP_A1018A)
#define SYS_CTL0_EXT_MAC2_RST      0x00000008
#endif
#define SYS_CTL0_EXT_IDU2_RST      0x00000010
#define SYS_CTL0_EXT_IDU3_RST      0x00000020
#define SYS_CTL0_EXT_IDU4_RST      0x00000040
#define SYS_CTL0_EXT_MCP2_RST      0x00000080
#define SYS_CTL0_EXT1_RF1013_RST	0x00000100
#define SYS_CTL0_EXT2_RF1013_RST    0x00000200
#define SYS_CTL0_EXT_USB_RST          0x00010000
//-----------AHB ARBIT Piority Config----------//
#define SYS_ARBHIPIR_DEFAULT         0x00000001
#define SYS_ARBHIPIR_DMALOW          0x00000002
#define SYS_ARBHIPIR_CPU_D           0x00000004
#define SYS_ARBHIPIR_CPU_I           0x00000008
#define SYS_ARBHIPIR_IPU             0x00000010
#define SYS_ARBHIPIR_SIU             0x00000020
#define SYS_ARBHIPIR_JPGVLC          0x00000040
#define SYS_ARBHIPIR_JPGDCT          0x00000080
#define SYS_ARBHIPIR_RESV8           0x00000100
#define SYS_ARBHIPIR_MD              0x00000200
#define SYS_ARBHIPIR_RESV10          0x00000400
#define SYS_ARBHIPIR_IDUROT          0x00000800
#define SYS_ARBHIPIR_MCPY            0x00001000
#define SYS_ARBHIPIR_RESV13          0x00002000
#define SYS_ARBHIPIR_RESV14          0x00004000
#define SYS_ARBHIPIR_RESV15          0x00008000

#define SYS_ARBHIPIR_ADPCM           0x00010000
#define SYS_ARBHIPIR_IDUVID          0x00020000
#define SYS_ARBHIPIR_DMAHIGH         0x00040000
#define SYS_ARBHIPIR_RESV19          0x00080000
#define SYS_ARBHIPIR_IDUOSD          0x00100000
#define SYS_ARBHIPIR_SUBTV           0x00200000
#define SYS_ARBHIPIR_RESV22          0x00400000
#define SYS_ARBHIPIR_RESV23          0x00800000
#define SYS_ARBHIPIR_ISU             0x01000000
#define SYS_ARBHIPIR_RESV25          0x02000000
#define SYS_ARBHIPIR_RESV26          0x04000000
#define SYS_ARBHIPIR_CIU1            0x08000000
#define SYS_ARBHIPIR_RESV28          0x10000000
#define SYS_ARBHIPIR_CIU2            0x20000000
#define SYS_ARBHIPIR_RF1013          0x40000000
#define SYS_ARBHIPIR_RESV31          0x80000000


/* Structure definition */

/* sys event queue to signal sysTask() */
#define SYS_EVT_MAX 8		/* max. sys event queued. Lucian: optimize 4-->8 */
typedef struct _SYS_EVT
{
    u8 cause[SYS_EVT_MAX];   		        /* cause of sys event */
    u32 param[SYS_EVT_MAX];		            /* parameter of sys event */
    u8 idxSet;                             /* index of set event */
    u8 idxGet;                             /* index of get event */
} SYS_EVT;


#define SYSBACK_RFEVT_MAX   64
typedef struct _SYSBACK_RF_EVT
{
    u8 cause[SYSBACK_RFEVT_MAX];   		/* cause of sys event */
    u32 param[SYSBACK_RFEVT_MAX];		    /* parameter of sys event */
    u8 idxSet;                             /* index of set event */
    u8 idxGet;                             /* index of get event */
} SYSBACK_RF_EVT;

#define SYSBACKLOW_EVT_MAX  8                   /* max. sysback low event queued*/
typedef struct _SYSBACKLOW_EVT
{
    u8 cause[SYSBACKLOW_EVT_MAX];   		/* cause of sys event */
    u32 param1[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32 param2[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32 param3[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */
    u32 param4[SYSBACKLOW_EVT_MAX];		    /* parameter of sys event */

    u8 idxSet;                             /* index of set event */
    u8 idxGet;                             /* index of get event */
} SYSBACKLOW_EVT;

#if (NIC_SUPPORT == 1)
#define SYSBACK_NET_EVT_MAX   10
typedef struct _SYSBACK_NET_EVT
{
    u8 cause[SYSBACK_NET_EVT_MAX];   		/* cause of sys event */
    u32 param1[SYSBACK_NET_EVT_MAX];		/* parameter 1 of sys event */
    u32 param2[SYSBACK_NET_EVT_MAX];		    /* parameter 2 of sys event */
    u8 idxSet;                             /* index of set event */
    u8 idxGet;                             /* index of get event */
} SYSBACK_NET_EVT;
#endif

typedef struct _SYS_THUMBNAIL
{
    s8 tname[20];
    u8 type;
    u8 bufinx;
} SYS_THUMBNAIL;

typedef struct _SYS_CONFIG_REC
{
    u8 Overwrite;
    u32 Seccion;
    u32 Duration;
    u32 EventExtendTime;

} SYS_CONFIG_REC;

typedef struct _SYS_CONFIG_CAMERA
{
    u8 RecMode;
    u8 CamerOnOff;
    u8 Resoultion;
    u8 Brightness;
} SYS_CONFIG_CAMERA;

typedef struct _SYS_CONFIG_SCHEDULE
{
    u8 SchEnable;
} SYS_CONFIG_SCHEDULE;

typedef struct _SYS_CONFIG_EVENT
{
    u8 MotionEnable;
    u8 MotionDayLevel;
    u8 MotionNeightLevel;
    u8 PIREnable;
    u32 EventRECTime;
} SYS_CONFIG_EVENT;

typedef struct _SYS_CONFIG_TIME
{
    u8 TimeZoneSign;
    u8 TimeZoneHour;
    u8 TimeZoneMin;
    u8 DSTEnable;
} SYS_CONFIG_TIME;

typedef struct _SYS_CONFIG_ALARM
{
    u8 AlarmEnable;
    u8 AlarmVal;
    u8 AlarmRange;
} SYS_CONFIG_ALARM;

typedef struct _SYS_CONFIG_NETWORK
{
    u8 DHCPEnable;
    u8 IPAddr[4];
    u8 NetMask[4];
    u8 Gateway[4];
} SYS_CONFIG_NETWORK;

typedef struct _SYS_CONFIG_SYSTEM
{
    u8 TVOut;
    u8 Language;
    u8 Flicker;
    u8 Volume;
} SYS_CONFIG_SYSTEM;


typedef struct _SYS_CONFIG_SETTING
{
    SYS_CONFIG_REC 		RecSetting;
    SYS_CONFIG_CAMERA 	CamSetting[MULTI_CHANNEL_MAX];
    SYS_CONFIG_SCHEDULE SchSetting[MULTI_CHANNEL_MAX];
    SYS_CONFIG_EVENT 	EventSetting[MULTI_CHANNEL_MAX];
    SYS_CONFIG_TIME		TimeSetting;
    SYS_CONFIG_ALARM 	AlarmSetting;
    SYS_CONFIG_NETWORK 	NetSetting;
    SYS_CONFIG_SYSTEM 	SysSetting;
} SYS_CONFIG_SETTING;


/* Constant */

/* Event */

enum
{
    SYS_EVT_PREVIEW_INIT = 0,
    SYS_EVT_PREVIEW_RESET,
    SYS_EVT_SNAPSHOT,
    SYS_EVT_PLAYBACK_ZOOM,
    SYS_EVT_PLAYBACK_PAN,
    SYS_EVT_PLAYBACK_MOVE_FORWARD,
    SYS_EVT_PLAYBACK_MOVE_BACKWARD,
    SYS_EVT_PLAYBACK_DELETE,
    SYS_EVT_PLAYBACK_DELETE_ALL,    
    SYS_EVT_PLAYBACK_FORMAT,
    SYS_EVT_BACKUP_FORMAT,
    SYS_EVT_PLAYBACK_ISP,
    SYS_EVT_VIDEOCAPTURE,
    SYS_EVT_PWRON, 
    SYS_EVT_PWROFF,
    SYS_EVT_MACRO,
    SYS_EVT_LCDROT,
    SYS_EVT_MOUNT,
    SYS_EVT_SDCD_IN,
    SYS_EVT_SDCD_OFF,
#if USB_HOST_MASS_SUPPORT
    SYS_EVT_USBCD_IN,
    SYS_EVT_USBCD_OFF,
#endif
    SYS_EVT_WhiteLight,
    SYS_EVT_FlashLight,
    SYS_EVT_VOICERECORD,
    SYS_EVT_ReadFile,
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    SYS_EVT_ContinuousReadFile,
    SYS_EVT_CopyFile,
#endif
    SYS_EVT_PREVIEWZOOMINOUT,
    SYS_EVT_TVPLAYBACK_DELETE,
    SYS_EVT_VIDEOZOOMINOUT,
    SYS_EVT_PLAYBACK_DELETE_DIR,
    SYS_EVT_USB_REMOVED,
    SYS_EVT_SET_UI_KEY,
    SYS_EVT_UPGRADE_FW,
    SYS_EVT_DEV_INSERT_UPGRADE,
    SYS_EVT_DOWNLOAD_BY_NET,
    SYS_EVT_UPGRADE_BY_NET,
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
    SYS_EVT_APP2NET_UPGRADE_EVT,
    SYS_EVT_APP2DEV_UPGRADE_EVT,
#endif    
#if (MULTI_CHANNEL_VIDEO_REC)
    SYS_EVT_VIDEOCAPTURE_STOP,
#endif
    SYS_EVT_SET_P2P_PLAYBACK,
    SYS_EVT_PLAYBACK_CALENDAR,
    SYS_EVT_GETDISKFREE,
#if RX_SNAPSHOT_SUPPORT
    SYS_EVT_RX_DATASAVE,
#endif
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	SYS_EVT_SCANFILE,
#endif
    SYS_EVT_FORCERESYNC_RF,
    SYS_EVT_UNDEF
};
enum
{
    SYS_BACK_EVT_W_SD = 0,
#if PLAYBEEP_TEST
    SYS_BACK_PLAY_BEEP,
#endif
    SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP,
    SYS_BACK_SENSOR_FLIP,
    SYS_BACK_GET_DISK_FREE,
    SYS_BACK_VIDEOZOOMINOUT,
    SYS_BACK_TVIN_CHANELCHANGE_PREVIEW,
    SYS_BACK_TVIN_CHANELCHANGE_CAPTUREVIDEO,
    SYS_BACK_PLAYBACK_FORMAT,
    SYS_BACK_BACKUP_FORMAT,
    SYS_BACK_DRAWTIMEONVIDEOCLIP,
#if( (CHIP_OPTION >= CHIP_A1013A) && RFIU_SUPPORT)
    SYS_BACK_RFI_RX_CH_RESTART,
    SYS_BACK_RFI_TX_CH_DEL,
    SYS_BACK_RFI_TX_CH_CREATE,
    SYS_BACK_RFI_TX_CHANGE_RESO,
#endif
#if (MOTIONDETEC_ENA==1)
    SYS_BACK_DRAW_MOTION_AREA_ONTV,
    SYS_BACK_DRAW_MOTION_AREA_ONPANEL,
#endif
#if HW_MD_SUPPORT
    SYS_BACK_DRAW_MOTION_AREA_ONTV,
#endif
    SYS_BACK_DRAW_BATTERY,
    SYS_BACK_CHECK_UI_CONTROL,
#if (NET_STATUS_POLLING && NIC_SUPPORT)
    SYS_BACK_DRAW_NET_ICON,
#endif
    SYS_BACK_CHECK_TVIN_FORMAT,
    SYS_BACK_CHECK_VIDEOIN_SOURCE,
    SYS_BACL_SET_SENSOR_COLOR,
    SYS_BACK_DRAW_BIT_RATE,
    SYS_BACK_DRAW_FRAME_RATE,
    SYS_BACK_DRAW_OSD_STRING,
    SYS_BACK_DRAW_SD_ICON,
#if(HOME_RF_SUPPORT)
    SYS_BACK_CHECK_HOMERF,
#endif
    SYS_BACK_SCHEDULEMODE,
#if RX_SNAPSHOT_SUPPORT
    SYS_BACK_RX_DATASAVE,
#endif
    SYSBACK_EVT_UNDEF,

};

enum
{
    SYSBACKLOW_EVT_DELETEFATLINK = 0,
    SYSBACKLOW_EVT_GETDISKFREE,
#if((FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR) && ( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) )
    SYSBACKLOW_EVT_OVERWRITEDEL,
#endif
	SYSBACKLOW_EVT_MOUNT_DEV,
	SYSBACKLOW_EVT_UNMOUNT_DEV,
    SYSBACKLOW_EVT_UI_KEY_SDCD,
#if USB_HOST_MASS_SUPPORT
    SYSBACKLOW_EVT_UI_KEY_USB,
#endif
    SYSBACKLOW_EVT_SYN_RF,
    SYSBACKLOW_EVT_UNDEF
};

enum
{
    SYS_BACKRF_RFI_RX_CH_RESTART=0,
    SYS_BACKRF_RFI_TX_CH_DEL,
    SYS_BACKRF_RFI_TX_CH_CREATE,
    SYS_BACKRF_RFI_TX_CHANGE_RESO,
    SYS_BACKRF_RFI_TX_SNAPSHOT,
    SYS_BACKRF_RFI_SAVE_RF_SETTING,
    SYS_BACKRF_FCC_DIRECT_TXRX,
    SYS_BACKRF_RFI_RX_SETOPMODE,
    SYS_BACKRF_RFI_TX_SETGPO,
    SYS_BACKRF_RFI_FORCERESYNC,
    SYS_BACKRF_RFI_CLEAR_QUADBUF,
    SYS_BACKRF_RFI_ENTER_WOR_B1,
    SYS_BACKRF_RFI_RESENDTXMDCFG,
    SYS_BACKRF_RFI_SENDTXMDSENS,
    SYS_BACKRF_RFI_TURBO_ON,
    SYS_BACKRF_RFI_TURBO_OFF,
    SYS_BACKRF_RFI_CAP_PHOTO,
    SYS_BACKRF_RFI_TX_SETPWM,
    SYS_BACKRF_RFI_TX_SETMOTORCTRL,
    SYS_BACKRF_RFI_TX_SETMELODYNUM,
    SYS_BACKRF_RFI_RX_VOXTRIG,
    SYS_BACKRF_RFI_RX_VOXCFG,
    SYS_BACKRF_RFI_UPDATETXOTHERSPARA,
    SYS_BACKRF_RFI_SAVE_UI_SETTING,
    SYS_BACKRF_RFI_SETTIME2TX,
    SYS_BACKRF_RFI_RX_LIGHTSTA,
    SYS_BACKRF_RFI_TX_SETREBOOT,
    SYS_BACKRF_RFI_TX_SETPIRCFG,
#if RFIU_RX_WAKEUP_TX_SCHEME
    SYS_BACKRF_RFI_TX_SETDOORBELLOFF,
#endif
    SYSBACK_RF_EVT_UNDEF
};

#if (NIC_SUPPORT == 1)
enum
{
    SYS_BACKRF_NTE_SEND_EVENT=0,
    SYS_BACKRF_NTE_NTP_UPDATE,
    SYS_BACKRF_NTE_FW_UPDATE,
#if CDVR_SYSTEM_LOG_SUPPORT
    SYS_BACKRF_WRITE_LOG,
#endif
#if(DEVSTATUS_ACTIVE_UPDATE)
    SYS_BACK_DEVSTATUS_UPDATE,
#endif
    SYSBACK_NTE_EVT_UNDEF
};

#if(DEVSTATUS_ACTIVE_UPDATE)
typedef enum _DEV_ACT_UPDATE//DEV_STATUS key ID
{
    SYS_BACK_DEVSTATUS_CHECK,
    SYS_BACK_DEVSTATUS_START,
    SYS_BACK_DEVSTATUS_STOP,
    SYS_BACK_DEVSTATUS_CLEAR,
    SYS_BACK_DEVSTATUS_PAUSE_RESUME
} DEV_ACT_UPDATE;
#endif
#endif

typedef enum dev_status
{
    DEV_SD_FULL = 0,
    DEV_SD_NOT_FULL,
    DEV_USB_PLUG_IN,
    DEV_NO_SD_CARD,
} DEV_STATUS;

enum
{
    SYS_PLAYBACK_MODE = 1,
    SYS_FILE_BACKUP_MODE
};

enum 
{
    SYS_CAM_NORMAL = 0,    
    SYS_CAM_BATTERY,       
};

/* Pan direction */
#define SYS_PAN_UP		0x00
#define SYS_PAN_DOWN	0x01
#define SYS_PAN_LEFT	0x02
#define SYS_PAN_RIGHT	0x03

/* Playback mode */
#define SYS_PLAYBACK_FORWARD_X1 0x00
#define SYS_PLAYBACK_FORWARD_X4 0x01
#define SYS_PLAYBACK_FORWARD_X8 0x02

#define CAP_PREVIEW_SCAL_UP_X1    0
#define CAP_PREVIEW_SCAL_UP_X2    1
#define CAP_PREVIEW_SCAL_UP_X3    2
#define CAP_PREVIEW_SCAL_DOWN_D2  3
#define CAP_PREVIEW_SCAL_DOWN_D4  4

#define FLAGSYS_RDYSTAT_MASK        0xFFFFFFFF

#define FLAGSYS_RDYSTAT_REC_START   0x00000001
#define FLAGSYS_RDYSTAT_REC_STOP    0x00000002
#define FLAGSYS_RDYSTAT_PLAY_START  0x00000004
#define FLAGSYS_RDYSTAT_PLAY_STOP   0x00000008

#define FLAGSYS_RDYSTAT_FORMAT      0x00000010
#define FLAGSYS_RDYSTAT_SET_REC     0x00000020
#define FLAGSYS_RDYSTAT_SET_PLAY    0x00000040
#define FLAGSYS_RDYSTAT_CARD1_ERR	0x00000080
#define FLAGSYS_RDYSTAT_CARD1_RDY	0x00000100
#define FLAGSYS_RDYSTAT_CARD1_STAT	0x00000200
#define FLAGSYS_RDYSTAT_PLAY_FINISH 0x00000400
#define FLAGSYS_RDYSTAT_DRAW_STOP	0x00000800
#define FLAGSYS_RDYSTAT_UPDATE_SEC	0x00001000
#define FLAGSYS_RDYSTAT_CH_SWITCH	0x00002000
#define FLAGSYS_RDYSTAT_DELETE      0x00004000
#define FLAGSYS_RDYSTAT_PWM_BEEP    0x00008000
#define FLAGSYS_RDYSTAT_PYBK_SEARCH 0x00010000
#define FLAGSYS_RDYSTAT_COPY_FILE   0x00020000

#define FLAGSYS_RDYSTAT_CARD2_ERR	0x00040000
#define FLAGSYS_RDYSTAT_CARD2_RDY	0x00080000
#define FLAGSYS_RDYSTAT_CARD2_STAT	0x00100000

#define FLAGSYS_RDYSTAT_TVinFormat  0x00000200

#if MULTI_CHANNEL_VIDEO_REC
#define FLAGSYS_SUB_RDYSTAT_REC_ALL 0x0000000f
#define FLAGSYS_SUB_RDYSTAT_REC_CH0 0x00000001
#define FLAGSYS_SUB_RDYSTAT_REC_CH1 0x00000002
#define FLAGSYS_SUB_RDYSTAT_REC_CH2 0x00000004
#define FLAGSYS_SUB_RDYSTAT_REC_CH3 0x00000008
#endif

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_ALL 0x0000000f
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 0x00000001
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH1 0x00000002
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH2 0x00000004
#define FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH3 0x00000008
#endif

/*when systask is deltetd, some flag must be set to default*/
#define FLAGSYS_SYS_INIT_RESET    (FLAGSYS_RDYSTAT_REC_START|FLAGSYS_RDYSTAT_REC_STOP|FLAGSYS_RDYSTAT_PLAY_START|\
                                   FLAGSYS_RDYSTAT_PLAY_STOP|FLAGSYS_RDYSTAT_FORMAT|FLAGSYS_RDYSTAT_SET_REC|\
                                   FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_CARD1_ERR|FLAGSYS_RDYSTAT_CARD1_RDY|\
                                   FLAGSYS_RDYSTAT_PLAY_FINISH|FLAGSYS_RDYSTAT_CARD2_ERR|FLAGSYS_RDYSTAT_CARD2_RDY)

// 
#define SYS_S_STORAGE_ORDER_BIT	4

// Storage order Index 
#define SYS_I_STORAGE_MAIN		(1 << SYS_S_STORAGE_ORDER_BIT * 0)
#define SYS_I_STORAGE_BACKUP	(1 << SYS_S_STORAGE_ORDER_BIT * 1)
//#define SYS_I_STORAGE_BBACKUP	(1 << SYS_S_STORAGE_ORDER_BIT * 2)
//#define SYS_I_STORAGE_BBBBACKUP	(1 << SYS_S_STORAGE_ORDER_BIT * 3)

// Storage selection
#define SYS_V_STORAGE_NONE		0
#if SD_TASK_INSTALL_FLOW_SUPPORT
#define SYS_V_STORAGE_USBMASS	1
#define SYS_V_STORAGE_SDC		2
#define SYS_V_STORAGE_END		3
#else
#define SYS_V_STORAGE_SDC		1
#define SYS_V_STORAGE_USBMASS	2
#endif
#define SYS_V_STORAGE_NREADY	0
#define SYS_V_STORAGE_READY		1

#define SYS_V_STORAGE_NKEEP		0

#define SYS_V_STORAGE_OFF		0
#define SYS_V_STORAGE_ON		1

#define SYS_BTC_WAKEUP_NO           0x00
#define SYS_BTC_WAKEUP_MANUAL       0x01
#define SYS_BTC_WAKEUP_PAIR         0x02
#define SYS_BTC_WAKEUP_APP          0x03  //Paul add for UI, 180507

#if 1 //(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
#define MR9211_QUIT_WIFI      0
#define MR9211_LEAVING_WIFI   1
#define MR9211_ENTER_WIFI     2
#endif

/*-----------Extern Variable -----------*/

#if ICOMMWIFI_SUPPORT
extern u32 sys9211TXWifiStat;
extern u32 sys9211TXWifiUserNum;
#endif


extern u32 gSystemStorageReady;		// bit 0: Main storage, bit 1: backup storage; instead of gInsertCard and gInsertBackup.
extern u32 gSystemStorageSel;
extern u32 gSystemCodeUpgrade;		// 0: Not in Upgrade processdure, 1: Upgrade Now

extern u32 gUISentKeyRetry;

extern u8 siuFlashReady;

extern u8 sysForceWdt2Reboot;
extern u32 sysLifeTime;
extern u32 sysLifeTime_prev;
extern u8  sysDeadLockCheck_ena;

extern OS_EVENT* sysSemEvt;
extern SYS_EVT sysEvt;
extern OS_EVENT* speciall_MboxEvt;
extern OS_EVENT* general_MboxEvt;


extern  s32 sysPreviewZoomFactor;
extern  u8  sysCaptureImageStart;
extern  u8  sysCaptureVideoStart;
extern  u8  sysCaptureVideoStop;
extern  u32 sysCaptureVideoMode;    // ASF_CAPTURE_NORMAL, ASF_CAPTURE_OVERWRITE, ASF_CAPTURE_EVENT
extern  u8  sysPlaybackVideoStart;
extern  u8  sysPlaybackVideoStop;
extern  u8  sysPlaybackVideoPause;
#if (AVSYNC == VIDEO_FOLLOW_AUDIO)
extern  u8  sysPlaybackForward;
extern  u8  sysPlaybackBackward;
#elif (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern  s8  sysPlaybackForward;
extern  s8  sysPlaybackBackward;
#endif
extern  u8  sysPlaybackThumbnail;
extern  u32 u32PacketStart;
extern  u32 sysUSBPlugInFlag;
extern  u32	sysTVinFormat;
extern  u8 sysCaptureImageStart;
extern  BOOLEAN sysTVInFormatLocked;
extern  BOOLEAN sysTVInFormatLocked1;
extern u8 sysReady2CaptureVideo;
extern u32 sysVideoInSel;
extern u8 system_busy_flag;
extern u8 Main_Init_Ready;
extern u8 got_disk_info;
extern u8 Iframe_flag;
extern SYS_THUMBNAIL *sysThumnailPtr;
extern u32 playback_location;
extern u8  pwroff;
extern u8 SelfTimer;
extern u32 sysPlaybackCamList;
extern u8 sysPlaybackYear, sysPlaybackMonth, sysPlaybackDay;
extern u32 sysPlaybackType;
extern u8 userClickFormat;
extern bool bRescan;

#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
extern u32 Video_totaltime[MAX_RFIU_UNIT];
extern u32 Audio_totaltime[MAX_RFIU_UNIT];
extern s32 Lose_video_time[MAX_RFIU_UNIT];
extern s32 Lose_audio_time[MAX_RFIU_UNIT];
extern  u8 ASF_Restart[MAX_RFIU_UNIT];
extern  u8 Motion_Error_ststus[MULTI_CHANNEL_MAX];
#endif


#if MULTI_CHANNEL_VIDEO_REC
extern OS_STK sysSubTaskStack0[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK sysSubTaskStack1[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK sysSubTaskStack2[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_STK sysSubTaskStack3[]; /* Stack of task MultiChannelMPEG4EncoderTask() */
extern OS_FLAG_GRP *gSysSubReadyFlagGrp;
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
extern OS_FLAG_GRP *gRfRxVideoPackerSubReadyFlagGrp;
#endif
#endif
extern BOOLEAN MemoryFullFlag;
extern BOOLEAN SysOverwriteFlag;
extern u8   SysCamOnOffFlag;
extern u8   SysShowPlay;
    
/*-----------------Extern Function prototype -------------*/
#if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
extern s32 sysTX9211_EnterWifi(s32 P2PQuailty);
extern s32 sysTX9211_LeaveWifi(s32 dummy);
#endif

extern int sysGetStorageSel(int StorageIndex);
extern int sysSetStorageSel(int StorageIndex, int Status);
extern int sysKeepSetStorageSel(int StorageIndex, int Status, int KeepStorageIndex);
extern int sysGetStorageStatus(int StorageIndex);
extern int sysSetStorageStatus(int StorageIndex, int Status);
extern int sysGetStorageInserted(int StorageIndex);

extern int sysSetUIKeyRetry(int StorageIndex);

#if SD_TASK_INSTALL_FLOW_SUPPORT
extern s32 sysBackLow_Device_Mount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3);
extern s32 sysBackLow_Device_UnMount(s32 NextDevSel, s32 dummy1, s32 dummy2, s32 dummy3);
#endif

extern int sysGetFWUpgradeStatus(void);
extern void sysSetFWUpgradeStatus(int Status);
extern s32 sysDevInsertedUpgradeEvt(s32 dummy);

extern void sysTurnOnTVDAC(int YOn,int CbOn,int CrOn);

extern void sysSentUiKeyTilOK(int UiKey);
extern s32 sysSetEvt(s8, s32);
extern s32 sysbackSetEvt(s8, s32);	//civic 070828
extern s32 sysback_RF_SetEvt(s8 cause, s32 param);
#if (NIC_SUPPORT == 1)
extern s32 sysback_Net_SetEvt(s8 cause, u32 param1, u32 param2);
#endif
extern s32 sysbackLowSetEvt(s8 cause, s32 param1,s32 param2,s32 param3,s32 param4);

extern s32 sysDeadLockMonitor(void);
extern s32 sysDeadLockMonitor_ON(void);
extern s32 sysDeadLockMonitor_OFF(void);
extern s32 sysDeadLockMonitor_Reset(void);
extern u8 sysCheckSDCD(void);
#if USB_HOST_MASS_SUPPORT
extern u8 sysCheckUSBCD(void);
#endif
extern s32 sysPreviewInit(s32);


extern s32 sysGetEvt(s8*, s32*);
extern s32 sysbackGetEvt(s8*, s32*);		//civic 070828
extern s32 sysbackLowGetEvt(s8* pCause, s32* pParam1,s32* pParam2,s32* pParam3,s32* pParam4);
extern s32 sysback_RF_GetEvt(s8* pCause, s32* pParam);
#if (NIC_SUPPORT == 1)
extern s32 sysback_Net_GetEvt(s8* pCause, u32* pParam, u32* pParam2);
extern s32 sys_back_Network_Task_init(void);
#endif
extern s32 sysInit(void);
extern void InitsysEvt(void);
extern void InitsysbackEvt(void);
extern void InitsysbackLowEvt(void);
extern s32 sysTest(s32 dummy);
extern s32 sysCheckNextEvtIsPrevOrNext(void);
extern s32 sysCheckNextEvtIsEmpty(void);
extern s32 sysbackCheckNextEvtIsEmpty(void);

extern void sysJPEG_enable(void);
extern void sysJPEG_disable(void);
extern void sysMPEG_enable(void);
extern void sysMPEG_disable(void);
extern void sysISU_enable(void);
extern void sysISU_disable(void);
extern void sysSIU_enable(void);
extern void sysSIU_disable(void);
extern void sysIDU_enable(void);
extern void sysIDU_disable(void);
extern void sysTVOUT_enable(void);
extern void sysTVOUT_disable(void);
extern void sysUSB_enable(void);
extern void sysUSB_disable(void);
extern void sysIIS_enable(void);
extern void sysIIS_disable(void);
extern void sysWDT_disable(void);
extern void sysWDT_enable(void);
extern void sysNAND_Enable(void);
extern void sysNAND_Disable(void);
extern void sysSD_Enable(void);
extern void sysSD_Disable(void);
extern void sysSPI_Disable(void);
extern void sysSPI_Enable(void);
extern s32 sysPreviewReset(s32);
extern s32 sysCiu_1_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_2_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_3_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_4_PreviewReset(s32 zoomFactor);
extern s32 sysCiu_5_PreviewReset(s32 zoomFactor);

extern s32 sysPreviewStop(void);
extern s32 sysPreviewZoomInOut(s32);
extern s32 sysVideoZoomInOut(s32);
extern s32 sysSnapshot_OnPreview(s32);

extern s32 sysTVInChannelChange_Preview(s32 zoomFactor);
extern s32 sysTVInChannelChange_CaptureVideo(s32 zoomFactor);

extern s32 sysReadWaveRing(u8 ucRingWaveNum);
extern s32 sysReadWaveRingwithBuf(u8* pucWaveFileBuf, u8 ucRingWaveNum);
extern s32 sysPlayRingWave(u8);
extern s32 sysWriteRingWave2SDwithBuf(u8*	pucWaveFileBuf, u8 ucRingWaveNum);

extern void SYSClkEnable(u32 uiClkEnable);
extern void SYSClkDisable(u32 uiClkDisable);
extern void SYSReset(u32 uiReset);
extern void SYSReset_EXT(u32 uiReset);

extern void idu_switch(void);
extern s32 gpioSetLevel(u8, u8, u8);
extern s32 gpioGetLevel(u8, u8, u8*);
extern void iduWaitCmdBusy(void);
extern void osdDrawVideoOn(u8 on);
extern u8  sysProjectDeviceStatus(DEV_STATUS status);
extern u32 getTVinFormat(void);
extern s32 sysGetDiskFree(s32 dummy);
extern s32 sysBackLowGetDiskFree(s32 dummy, s32 dummy2, s32 dummy3, s32 dummy4);
extern s32 sysSDCD_OFF(s32);
extern s32 sysSDCD_IN(s32);

#if USB_HOST_MASS_SUPPORT
extern s32 sysUSBCD_OFF(s32);
extern s32 sysUSBCD_IN(s32);
#endif

extern s32 sysForceWDTtoReboot(void);
extern s32 sysPowerOn(s32);
extern s32 sysPowerOff(s32);
extern void sysPowerOffDirect(void);


extern s32 sys_background_init(void);
extern s32 sys_backLowTask_init(void);
extern s32 sys_back_RF_Task_init(void);  //處理RF task 所發出的Event

extern u8 sysSet_Overwrite(u8 pData);
extern void sysGet_Overwrite(u8* pData);
extern u8 sysSet_Seccion(u32 pData);
extern void sysGet_Seccion(u32* pData);
extern u8 sysSet_Duration(u32 pData);
extern void sysGet_Duration(u32* pData);
extern u8 sysSet_EventExtendTime(u32 pData);
extern void sysGet_EventExtendTime(u32* pData);
extern u8 sysSet_RecMode(u8 nCam, u8 pData);
extern void sysGet_RecMode(u8 nCam, u8* pData);
extern u8 sysSet_CamerOnOff(u8 nCam, u8 pData);
extern void sysGet_CamerOnOff(u8 nCam, u8* pData);
extern u8 sysSet_FormatSD(void);
extern void sysGet_Resoultion(u8 nCam, u8* pData);
extern u8 sysSet_Resoultion(u8 nCam, u8 pData);
extern void sysGet_Brightness(u8 nCam, u8* pData);
extern u8 sysSet_Brightness(u8 nCam, u8 pData);
extern void sysGet_SchEnable(u8 nCam, u8* pData);
extern u8 sysSet_SchEnable(u8 nCam, u8 pData);
extern void sysGet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8* pData);
extern u8 sysSet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8 pData);
extern void sysGet_MotionEnable(u8 nCam, u8* pEnable, u8* pDay, u8* pNight);
extern u8 sysSet_MotionEnable(u8 nCam, u8 pEnable, u8 pDay, u8 pNight);
extern void sysGet_PIREnable(u8 nCam, u8* pData);
extern u8 sysSet_PIREnable(u8 nCam, u8 pData);
extern void sysGet_TimeZone(u8* pSign, u8* pHour, u8* pMin);
extern u8 sysSet_TimeZone(u8 pSign, u8 pHour, u8 pMin);
extern void sysGet_DSTEnable(u8* pData);
extern u8 sysSet_DSTEnable(u8 pData);
extern void sysGet_AlarmEnable(u8* pData);
extern u8 sysSet_AlarmEnable(u8 pData);
extern void sysGet_AlarmVal(u8* pData);
extern u8 sysSet_AlarmVal(u8 pData);
extern void sysGet_NetworkData(SYS_CONFIG_NETWORK *sNetData);
extern u8 sysSet_NetworkData(SYS_CONFIG_NETWORK *sNetData);
extern void sysGet_TVOut(u8* pData);
extern u8 sysSet_TVOut(u8 pData);
extern void sysGet_Language(u8* pData);
extern u8 sysSet_Language(u8 pData);
extern void sysGet_Flicker(u8* pData);
extern u8 sysSet_Flicker(u8 pData);
extern void sysGet_Volume(u8* pData);
extern u8 sysSet_Volume(u8 pData);
extern void sysGet_EventTime(u8 nCam, u32 *pData);
extern u8 sysSet_EventTime(u8 nCam, u32 pData);
extern u8 sysSet_Pair(u8 nCam);
extern u8 sysSet_FormatSD(void);
extern void sysGet_Time(RTC_DATE_TIME *pTime);
extern u8 sysSet_Time(RTC_DATE_TIME *pTime);
extern u8 sysSet_UpgradeFW(void);
extern u8 sysSet_Reset(void);
extern u8 sysGet_CurrTemp(float *temp);
extern u8 sysSetNightMode(unsigned char mode);
extern u8 sysGetNightMode(unsigned char *mode);
extern u8 sysGetLight(unsigned char *CurrentValueR,unsigned char *CurrentValueG,unsigned char *CurrentValueB,unsigned char *CurrentValueL,unsigned char *status);
extern u8 sysSetLight(unsigned char CurrentValueR,unsigned char CurrentValueG,unsigned char CurrentValueB,unsigned char CurrentValueL,unsigned char status);
extern u8 sysSetRecordMode(unsigned int mode);
extern u8 sysGetRecordMode(unsigned int *mode);
extern u8 sysSetMountSD(unsigned char mode);
extern u8 sysGetMountSD(unsigned char *mode);
extern u8 sysGetFileRecycle(unsigned char  *status);
extern u8 sysSetFileRecycle(unsigned char  status);
extern u8 sysGetFWver(char *version);
extern u8 sysGetMDSensitivity(unsigned int channel,unsigned int *sensitivity);
extern u8 sysSetMDSensitivity(unsigned int channel,unsigned int sensitivity);
extern u8 sysSetFrequency(unsigned int channel,unsigned char mode);
extern u8 sysGetFrequency(unsigned int channel,unsigned char *mode);
extern u8 sysSet_ResetToDef(void);
extern u8 sysGet_TempHighMargin(unsigned int channel,float *temp);
extern u8 sysSet_TempHighMargin(unsigned int channel,float temp);
extern u8 sysGet_TempLowMargin(unsigned int channel,float *temp);
extern u8 sysSet_TempLowMargin(unsigned int channel,float temp);
extern u8 sysGet_NoiseAlert(unsigned int channel,unsigned char *status);
extern u8 sysSet_NoiseAlert(unsigned int channel,unsigned char status);
extern u8 sysGet_TempAlert(unsigned int channel,unsigned char *status);
extern u8 sysSet_TempAlert(unsigned int channel,unsigned char status);
extern void sysSet_DefaultValue(void);
extern s32 uiSysMenuAction(s8 setidx);
extern s32 sysSetSeekTime(u32 time);
extern s32 sysEnableThumb(u8 flag);
extern s32 sysBack_ScheduleMode(s32 dummy);
extern void sysCaptureMuteRec(int ch, int on);

extern void sysSDCRst(void);

extern s32 sysCaptureImage_One_OnPreview420_CIU1(s32 ZoomFactor);
extern s32 sysCaptureImage_One_OnPreview420_CIU2(s32 ZoomFactor);
extern s32 sysCaptureImage_One_OnPreview420_CIU5(s32 ZoomFactor);


#if (MOTIONDETEC_ENA==1)
extern void DrawMotionArea_OnTV(unsigned int Diff);
extern void DrawMotionArea_OnPanel(unsigned int Diff);
#endif

#if HW_MD_SUPPORT
extern void DrawMotionArea_OnTV(unsigned int Diff);
#endif




extern OS_FLAG_GRP *gSysReadyFlagGrp;
extern OS_FLAG_GRP *gSDCExtFlagGrp;

#if RFIU_RX_WAKEUP_TX_SCHEME 
extern void sysSetBTCWakeTime(u16 second);
extern u16 sysGetBTCWakeTime(void);
extern void sysSetSyncBTCTime(u16 hour);
extern u16 sysGetSyncBTCTime(void);
extern void sysResetBTCCheckLev(void);
extern u8 sysGetBTCTimer(u8 cam);
extern void sysSetBTCTimer(u8 cam,u16 second);
extern bool sysBatteryCam_isSleeping(u8 channel_id);
extern void sysStartBatteryCam(u8 camID);
extern void sysBatteryCam_wake(u8 channel_id,u16 seconds);
extern void sysSetBTCWakeStatus(u8 channel_id,u8 status, bool set);
extern u8 sysGetBTCWakeStatus(u8 channel_id);
#endif

#if HOME_RF_SUPPORT
extern u8 sysAppGetSensor(u32 sID, SMsgAVIoctrlGetSensorResp * response);
extern u32 sysAppAddRoom(SMsgAVIoctrlSetAddRoomReq * request);
extern u8 sysAppDeleteRoom(SMsgAVIoctrlSetDelRoomReq * request);
extern u8 sysAppDeleteSensor(u32 sID);
extern u8 sysAppEditRoom(SMsgAVIoctrlSetEditRoomReq *request);
extern u8 sysAppEditSensor(u32 sID, u8 *name);
extern void  sysAppGetRoomList(SMsgAVIoctrlGetRoomLstResp * response, u8 order);
extern void  sysAppGetSensorList(SMsgAVIoctrlGetSensorLstResp * response,u8 order);
extern void sysAppGetSceneList(SMsgAVIoctrlGetSceneLstResp * response, u8 order);
extern void sysAppAddSensor(SMsgAVIoctrlSetAddSensorResp * response);
extern u8 sysAppExecuteScene(u32 sceneID);
extern u8 gAppPairFlag;
extern s32 sysBack_Check_HOMERF(s32 level);
extern void sysAppGetSensorName(u32 camidx);
#if CDVR_iHome_LOG_SUPPORT
extern u8 sysAppGetSensorLog(u32 sID, SMsgAVIoctrlGetSensorLogResp * response, u32 order);
extern u8 sysGetSensorLog(u8 Idx, HOMERF_SensorLogList * response, unsigned int order);
extern u8 sysGetSensorLogDayList(HOMERF_SensorLogDayList * response, u8 order);
extern void sysSetOutputMode(u8 ucMode);
extern void sysPlaybackSetOutputMode(u8 ucMode);
#endif

enum
{
    APP_PAIR_NONE=0,
    APP_PAIR_SUCCESS,
    APP_PAIR_FAIL,
};

enum
{
    APP_IDTYPE_ROOM=0,
    APP_IDTYPE_SCENE,
};

#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
enum
{
    HOMERF_EVENT_LOG=0,
    SYSTEM_EVENT_LOG,
};

enum
{
    SYSTEM_MOTION_NONE=0,
    SYSTEM_MOTION_ON,
    SYSTEM_MOTION_OFF,
    SYSTEM_REBOOT,
    SYSTEM_SHUTDOWN,
    SYSTEM_RESET,
};

enum
{
    SYSTEM_LOG_NONE=0,
    SYSTEM_LOG_MOTION,
    SYSTEM_LOG_CONTINUE,
    SYSTEM_LOG_BOTH,
};



#endif
#if CDVR_SYSTEM_LOG_SUPPORT
extern u8 sysUIGetSensorLog(u8 day, SYSTEM_EventLogList * response, unsigned int order);

#endif //#if CDVR_SYSTEM_LOG_SUPPORT



extern void sysSetOutputMode(u8 ucMode);
extern void sysPlaybackSetOutputMode(u8 ucMode);
extern void sysTVswitchResolutionbyImagesize(void);
extern void sysTVswitchResolutionbyAssign(u8 videoType);


#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#define USE_DBGPRINT_PTR						0

//--------------------------------------------------
#if(USE_DBGPRINT_PTR)

#define DBGPRINT_FLAG_NONE						0x00000000
#define DBGPRINT_FLAG_EN_FTMAC110				0x00000001
//...
typedef int (*dbg_printf) ( const char * format, ... );

//enable/disable the debug message output of ftmac110_isr()
extern void ftmac110isr_dbgprint_enabled(u8 Enabled);
#endif //#if(USE_DBGPRINT_PTR)
#endif

#if ENABLE_DOOR_BELL
typedef enum 
{
    SYS_DOOR_NONE = 0,
    SYS_DOOR_RING,
    SYS_DOOR_TALK,

}SYS_DOOR_STATE;
extern SYS_DOOR_STATE sysGetDoorBellState(void);
extern void sysSetDoorBellState(SYS_DOOR_STATE state);
#endif
#endif




#if QRCODE_SUPPORT
////////////////////////////////////////
// ----------------------
// QR Code Generate Code.
// 20170927 Sean Add.
// ----------------------
////////////////////////////////////////

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/*---- Enum and struct types----*/

/*
 * The error correction level used in a QR Code symbol.
 */
enum qrcodegen_Ecc {
	qrcodegen_Ecc_LOW = 0,
	qrcodegen_Ecc_MEDIUM,
	qrcodegen_Ecc_QUARTILE,
	qrcodegen_Ecc_HIGH,
};


/*
 * The mask pattern used in a QR Code symbol.
 */
enum qrcodegen_Mask {
	// A special value to tell the QR Code encoder to
	// automatically select an appropriate mask pattern
	qrcodegen_Mask_AUTO = -1,
	// The eight actual mask patterns
	qrcodegen_Mask_0 = 0,
	qrcodegen_Mask_1,
	qrcodegen_Mask_2,
	qrcodegen_Mask_3,
	qrcodegen_Mask_4,
	qrcodegen_Mask_5,
	qrcodegen_Mask_6,
	qrcodegen_Mask_7,
};


/*
 * The mode field of a segment.
 */
enum qrcodegen_Mode {
	qrcodegen_Mode_NUMERIC,
	qrcodegen_Mode_ALPHANUMERIC,
	qrcodegen_Mode_BYTE,
	qrcodegen_Mode_KANJI,
	qrcodegen_Mode_ECI,
};


/*
 * A segment of user/application data that a QR Code symbol can convey.
 * Each segment has a mode, a character count, and character/general data that is
 * already encoded as a sequence of bits. The maximum allowed bit length is 32767,
 * because even the largest QR Code (version 40) has only 31329 modules.
 */
struct qrcodegen_Segment {
	// The mode indicator for this segment.
	enum qrcodegen_Mode mode;

	// The length of this segment's unencoded data. Always in the range [0, 32767].
	// For numeric, alphanumeric, and kanji modes, this measures in Unicode code points.
	// For byte mode, this measures in bytes (raw binary data, text in UTF-8, or other encodings).
	// For ECI mode, this is always zero.
	int numChars;

	// The data bits of this segment, packed in bitwise big endian.
	// Can be null if the bit length is zero.
	uint8_t *data;

	// The number of valid data bits used in the buffer. Requires
	// 0 <= bitLength <= 32767, and bitLength <= (capacity of data array) * 8.
	int bitLength;
};



/*---- Macro constants and functions ----*/

// The minimum and maximum defined QR Code version numbers for Model 2.
#define qrcodegen_VERSION_MIN  1
#define qrcodegen_VERSION_MAX  40

// Calculates the number of bytes needed to store any QR Code up to and including the given version number,
// as a compile-time constant. For example, 'uint8_t buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(25)];'
// can store any single QR Code from version 1 to 25, inclusive.
// Requires qrcodegen_VERSION_MIN <= n <= qrcodegen_VERSION_MAX.
#define qrcodegen_BUFFER_LEN_FOR_VERSION(n)  ((((n) * 4 + 17) * ((n) * 4 + 17) + 7) / 8 + 1)

// The worst-case number of bytes needed to store one QR Code, up to and including
// version 40. This value equals 3918, which is just under 4 kilobytes.
// Use this more convenient value to avoid calculating tighter memory bounds for buffers.
#define qrcodegen_BUFFER_LEN_MAX  qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX)



/*---- Functions to generate QR Codes ----*/

/*
 * Encodes the given text string to a QR Code symbol, returning true if encoding succeeded.
 * If the data is too long to fit in any version in the given range
 * at the given ECC level, then false is returned.
 * - The input text must be encoded in UTF-8 and contain no NULs.
 * - The variables ecl and mask must correspond to enum constant values.
 * - Requires 1 <= minVersion <= maxVersion <= 40.
 * - The arrays tempBuffer and qrcode must each have a length
 *   of at least qrcodegen_BUFFER_LEN_FOR_VERSION(maxVersion).
 * - After the function returns, tempBuffer contains no useful data.
 * - If successful, the resulting QR Code may use numeric,
 *   alphanumeric, or byte mode to encode the text.
 * - In the most optimistic case, a QR Code at version 40 with low ECC
 *   can hold any UTF-8 string up to 2953 bytes, or any alphanumeric string
 *   up to 4296 characters, or any digit string up to 7089 characters.
 *   These numbers represent the hard upper limit of the QR Code standard.
 * - Please consult the QR Code specification for information on
 *   data capacities per version, ECC level, and text encoding mode.
 */
bool qrcodegen_encodeText(const char *text, uint8_t tempBuffer[], uint8_t qrcode[],
	enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl);


/*
 * Encodes the given binary data to a QR Code symbol, returning true if encoding succeeded.
 * If the data is too long to fit in any version in the given range
 * at the given ECC level, then false is returned.
 * - The input array range dataAndTemp[0 : dataLen] should normally be
 *   valid UTF-8 text, but is not required by the QR Code standard.
 * - The variables ecl and mask must correspond to enum constant values.
 * - Requires 1 <= minVersion <= maxVersion <= 40.
 * - The arrays dataAndTemp and qrcode must each have a length
 *   of at least qrcodegen_BUFFER_LEN_FOR_VERSION(maxVersion).
 * - After the function returns, the contents of dataAndTemp may have changed,
 *   and does not represent useful data anymore.
 * - If successful, the resulting QR Code will use byte mode to encode the data.
 * - In the most optimistic case, a QR Code at version 40 with low ECC can hold any byte
 *   sequence up to length 2953. This is the hard upper limit of the QR Code standard.
 * - Please consult the QR Code specification for information on
 *   data capacities per version, ECC level, and text encoding mode.
 */
bool qrcodegen_encodeBinary(uint8_t dataAndTemp[], size_t dataLen, uint8_t qrcode[],
	enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl);


/*
 * Tests whether the given string can be encoded as a segment in alphanumeric mode.
 */
bool qrcodegen_isAlphanumeric(const char *text);


/*
 * Tests whether the given string can be encoded as a segment in numeric mode.
 */
bool qrcodegen_isNumeric(const char *text);


/*
 * Returns the number of bytes (uint8_t) needed for the data buffer of a segment
 * containing the given number of characters using the given mode. Notes:
 * - Returns SIZE_MAX on failure, i.e. numChars > INT16_MAX or
 *   the number of needed bits exceeds INT16_MAX (i.e. 32767).
 * - Otherwise, all valid results are in the range [0, ceil(INT16_MAX / 8)], i.e. at most 4096.
 * - It is okay for the user to allocate more bytes for the buffer than needed.
 * - For byte mode, numChars measures the number of bytes, not Unicode code points.
 * - For ECI mode, numChars must be 0, and the worst-case number of bytes is returned.
 *   An actual ECI segment can have shorter data. For non-ECI modes, the result is exact.
 */
size_t qrcodegen_calcSegmentBufferSize(enum qrcodegen_Mode mode, size_t numChars);


/*
 * Returns a segment representing the given binary data encoded in byte mode.
 */
struct qrcodegen_Segment qrcodegen_makeBytes(const uint8_t data[], size_t len, uint8_t buf[]);


/*
 * Returns a segment representing the given string of decimal digits encoded in numeric mode.
 */
struct qrcodegen_Segment qrcodegen_makeNumeric(const char *digits, uint8_t buf[]);


/*
 * Returns a segment representing the given text string encoded in alphanumeric mode.
 * The characters allowed are: 0 to 9, A to Z (uppercase only), space,
 * dollar, percent, asterisk, plus, hyphen, period, slash, colon.
 */
struct qrcodegen_Segment qrcodegen_makeAlphanumeric(const char *text, uint8_t buf[]);


/*
 * Returns a segment representing an Extended Channel Interpretation
 * (ECI) designator with the given assignment value.
 */
struct qrcodegen_Segment qrcodegen_makeEci(long assignVal, uint8_t buf[]);


/*
 * Renders a QR Code symbol representing the given data segments at the given error correction
 * level or higher. The smallest possible QR Code version is automatically chosen for the output.
 * Returns true if QR Code creation succeeded, or false if the data is too long to fit in any version.
 * This function allows the user to create a custom sequence of segments that switches
 * between modes (such as alphanumeric and binary) to encode text more efficiently.
 * This function is considered to be lower level than simply encoding text or binary data.
 * To save memory, the segments' data buffers can alias/overlap tempBuffer, and will
 * result in them being clobbered, but the QR Code output will still be correct.
 * But the qrcode array must not overlap tempBuffer or any segment's data buffer.
 */
bool qrcodegen_encodeSegments(const struct qrcodegen_Segment segs[], size_t len,
	enum qrcodegen_Ecc ecl, uint8_t tempBuffer[], uint8_t qrcode[]);


/*
 * Renders a QR Code symbol representing the given data segments with the given encoding parameters.
 * Returns true if QR Code creation succeeded, or false if the data is too long to fit in the range of versions.
 * The smallest possible QR Code version within the given range is automatically chosen for the output.
 * This function allows the user to create a custom sequence of segments that switches
 * between modes (such as alphanumeric and binary) to encode text more efficiently.
 * This function is considered to be lower level than simply encoding text or binary data.
 * To save memory, the segments' data buffers can alias/overlap tempBuffer, and will
 * result in them being clobbered, but the QR Code output will still be correct.
 * But the qrcode array must not overlap tempBuffer or any segment's data buffer.
 */
bool qrcodegen_encodeSegmentsAdvanced(const struct qrcodegen_Segment segs[], size_t len, enum qrcodegen_Ecc ecl,
	int minVersion, int maxVersion, int mask, bool boostEcl, uint8_t tempBuffer[], uint8_t qrcode[]);


/*---- Functions to extract raw data from QR Codes ----*/

/*
 * Returns the side length of the given QR Code, assuming that encoding succeeded.
 * The result is in the range [21, 177]. Note that the length of the array buffer
 * is related to the side length - every 'uint8_t qrcode[]' must have length at least
 * qrcodegen_BUFFER_LEN_FOR_VERSION(version), which equals ceil(size^2 / 8 + 1).
 */
int qrcodegen_getSize(const uint8_t qrcode[]);


/*
 * Returns the color of the module (pixel) at the given coordinates, which is either
 * false for white or true for black. The top left corner has the coordinates (x=0, y=0).
 * If the given coordinates are out of bounds, then false (white) is returned.
 */
bool qrcodegen_getModule(const uint8_t qrcode[], int x, int y);
#endif
