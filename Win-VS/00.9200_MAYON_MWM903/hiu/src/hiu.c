/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	hiu.c

Abstract:

   	The routines of host interface unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#include "sysopt.h"
#if HIU_COMMAND
#include "general.h"
#include "board.h"
#include "sysapi.h"	
#include "intapi.h"
#include "mars_controller/mars_int.h"    
#include "hiu.h"
#include "hiureg.h"

#include "task.h"
#include "hiapi.h"
#include "intapi.h"
#include "osapi.h"
#include "gpioapi.h"

#include "rtcapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "isuapi.h"
#include "iduapi.h"
#include "MemoryPool.h"
#include "uiapi.h"
#include "uiKey.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define MAX_HIU_CMDBUF_SIZE   	16
#define HIU_CMDBUF_SIZE   		8

#define HIU_STATUS				0x80
 
#define HIU_CMDLIST_WRITE		0x00
#define HIU_CMDLIST_READ		0x80

#define HIU_CMDLIST_SYSTEM		0x00
#define HIU_CMDLIST_PLAYBACK	0x10
#define HIU_CMDLIST_RECORD		0x20


//Command definition
#define HIU_CMD_NULL            HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x00
#define HIU_CMD_KEY             HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x01
#define HIU_CMD_TV				HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x02
#define HIU_CMD_READTV			HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x02
#define HIU_CMD_TIME			HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x03
#define HIU_CMD_READTIME		HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x03
#define HIU_CMD_ALARMPD			HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x04
#define HIU_CMD_READALARMPD		HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x04
#define HIU_CMD_LANG			HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x05
#define HIU_CMD_READLANG		HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x05
#define HIU_CMD_CHAN			HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x06
#define HIU_CMD_READCHAN		HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x06
#define HIU_CMD_OSD       		HIU_CMDLIST_WRITE+HIU_CMDLIST_SYSTEM+0x07
#define HIU_CMD_READSYSMODE		HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x0D
#define HIU_CMD_VERSION			HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x0E
#define HIU_CMD_CARDINFO		HIU_CMDLIST_READ+HIU_CMDLIST_SYSTEM+0x0F


#define HIU_CMD_VPLAY			HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x00
#define HIU_CMD_FILE			HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x01
#define HIU_CMD_DIR				HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x02
#define HIU_CMD_DEL				HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x03
#define HIU_CMD_FORMAT			HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x04
#define HIU_CMD_AVOL			HIU_CMDLIST_WRITE+HIU_CMDLIST_PLAYBACK+0x05
#define HIU_CMD_READAVOL		HIU_CMDLIST_READ+HIU_CMDLIST_PLAYBACK+0x05

#define HIU_CMD_VQUAL			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x00
#define HIU_CMD_READVQUAL		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x00
#define HIU_CMD_FRAMERATE		HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x01
#define HIU_CMD_READFRAMERATE	HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x01
#define HIU_CMD_STIME1			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x02
#define HIU_CMD_READSTIME1		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x02
#define HIU_CMD_STIME2			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x03
#define HIU_CMD_READSTIME2		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x03
#define HIU_CMD_OVERWRITE		HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x04
#define HIU_CMD_READOVERWRITE	HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x04
#define HIU_CMD_SECTION			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x05
#define HIU_CMD_READSECTION		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x05
#define HIU_CMD_MDSEN			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x06
#define HIU_CMD_READMDSEN		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x06
#define HIU_CMD_MDSPEED			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x07
#define HIU_CMD_READMDSPEED		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x07
#define HIU_CMD_MDNOISE			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x08
#define HIU_CMD_READMDNOISE		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x08
#define HIU_CMD_MDMASKXY		HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x09
#define HIU_CMD_READMDMASKXY	HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x09
#define HIU_CMD_RECMODE			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x0A
#define HIU_CMD_READRECMODE		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x0A
#define HIU_CMD_VREC			HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x0B
#define HIU_CMD_READVREC		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x0B
#define HIU_CMD_MDMASKSAVE		HIU_CMDLIST_WRITE+HIU_CMDLIST_RECORD+0x0C
#define HIU_CMD_READMASKNO		HIU_CMDLIST_READ+HIU_CMDLIST_RECORD+0x0C

//**********************************************************
//        define SUB_CMD
//**********************************************************

//HIU_CMD_KEY
#define HIU_SUBCMD_KEY_UP               0x00
#define HIU_SUBCMD_KEY_LEFT             0x01
#define HIU_SUBCMD_KEY_DOWN             0x02
#define HIU_SUBCMD_KEY_RIGHT            0x03
#define HIU_SUBCMD_KEY_ENTER            0x04
#define HIU_SUBCMD_KEY_DELETE           0x05
#define HIU_SUBCMD_KEY_PLAY             0x06
#define HIU_SUBCMD_KEY_MENU             0x07
#define HIU_SUBCMD_KEY_MODE             0x08
#define HIU_SUBCMD_KEY_STOP             0x09
#define HIU_SUBCMD_KEY_CHSEL            0x0A

//HIU_CMD_OSD
#define HIU_SUBCMD_OSD_DISABLE      	0x00
#define HIU_SUBCMD_OSD_ENABLE       	0x01

//HIU_CMD_TV			
//HIU_CMD_READTV			
#define HIU_SUBCMD_TV_NTSC				0x00
#define HIU_SUBCMD_TV_PAL				0x01

//HIU_CMD_TIME
//HIU_CMD_READTIME

//HIU_CMD_ALARMPD
//HIU_CMD_READALARMPD			
#define HIU_SUBCMD_ALARMPD_OFF			0x00
#define HIU_SUBCMD_ALARMPD_10S			0x01
#define HIU_SUBCMD_ALARMPD_20S			0x02
#define HIU_SUBCMD_ALARMPD_30S			0x03

//HIU_CMD_LANG
//HIU_CMD_READLANG
#define HIU_SUBCMD_LANG_ENG				0x00
#define HIU_SUBCMD_LANG_TCHI			0x01
#define HIU_SUBCMD_LANG_SCHI			0x02

//HIU_CMD_CHAN
//HIU_CMD_READCHAN
#define HIU_SUBCMD_CHAN_1				0x00
#define HIU_SUBCMD_CHAN_2				0x01

//HIU_CMD_CARDINFO
#define HIU_SUBCMD_CARD_TOTAL			0x00
#define HIU_SUBCMD_CARD_USAGE			0x01
#define HIU_SUBCMD_CARD_REMAIN			0x02

//HIU_CMD_VPLAY
#define HIU_SUBCMD_VPLAY_STOP			0x00
#define HIU_SUBCMD_VPLAY_PLAY			0x01
#define HIU_SUBCMD_VPLAY_PAUSE			0x02
#define HIU_SUBCMD_VPLAY_16XREW			0x03
#define HIU_SUBCMD_VPLAY_8XREW			0x04
#define HIU_SUBCMD_VPLAY_4XREW			0x05
#define HIU_SUBCMD_VPLAY_2XREW			0x06
#define HIU_SUBCMD_VPLAY_1XREW			0x07
#define HIU_SUBCMD_VPLAY_1XFF			0x08
#define HIU_SUBCMD_VPLAY_2XFF			0x09
#define HIU_SUBCMD_VPLAY_4XFF			0x0A
#define HIU_SUBCMD_VPLAY_8XFF			0x0B
#define HIU_SUBCMD_VPLAY_16XFF			0x0C

//HIU_CMD_FILE
#define HIU_SUBCMD_FILE_HEAD			0x00
#define HIU_SUBCMD_FILE_TAIL			0x01
#define HIU_SUBCMD_FILE_NEXT			0x02
#define HIU_SUBCMD_FILE_PRE				0x03
#define HIU_SUBCMD_FILE_NANE			0x04
#define HIU_SUBCMD_FILE_DATE			0x05
#define HIU_SUBCMD_FILE_SIZE			0x06

//HIU_CMD_DIR
#define HIU_SUBCMD_DIR_HEAD				0x00
#define HIU_SUBCMD_DIR_TAIL				0x01
#define HIU_SUBCMD_DIR_NEXT				0x02
#define HIU_SUBCMD_DIR_PRE				0x03
#define HIU_SUBCMD_DIR_NAME				0x04
#define HIU_SUBCMD_DIR_DATE				0x05
#define HIU_SUBCMD_DIR_FILENO			0x06

//HIU_CMD_DEL
#define HIU_SUBCMD_DEL_FILE				0x00
#define HIU_SUBCMD_DEL_DIR				0x01
#define HIU_SUBCMD_DEL_ALL				0x02

//HIU_CMD_FORMAT
#define HIU_SUBCMD_FORMAT_SD			0x00
#define HIU_SUBCMD_FORMAT_NAND			0x01
#define HIU_SUBCMD_FORMAT_UPDATE		0x02

//HIU_CMD_AVOL
#define HIU_SUBCMD_AVOL_MUTE			0x00
#define HIU_SUBCMD_AVOL_RESUME			0x01

#define HIU_SUBCMD_AVOL_MIN				0x00
#define HIU_SUBCMD_AVOL_MAX				0x01
#define HIU_SUBCMD_AVOL_INC				0x02
#define HIU_SUBCMD_AVOL_DEC				0x03

//HIU_CMD_READAVOL

//HIU_CMD_VQUAL
//HIU_CMD_READVQUAL
#define HIU_SUBCMD_VQUAL_HIGH			0x00
#define HIU_SUBCMD_VQUAL_MIDDLE			0x01
#define HIU_SUBCMD_VQUAL_LOW			0x02

//HIU_CMD_FRAMERATE
//HIU_CMD_READFRAMERATE
#define HIU_SUBCMD_FRAMERATE_30			0x00
#define HIU_SUBCMD_FRAMERATE_15			0x01
#define HIU_SUBCMD_FRAMERATE_05			0x02

//HIU_CMD_STIME1
//HIU_CMD_READSTIME1
//HIU_CMD_STIME2
//HIU_CMD_READSTIME2

//HIU_CMD_OVERWRITE
//HIU_CMD_READOVERWRITE
#define HIU_SUBCMD_OVERWRITE_OFF		0x00
#define HIU_SUBCMD_OVERWRITE_ON			0x01

//HIU_CMD_SECTION
//HIU_CMD_READSECTION
#define HIU_SUBCMD_SECTION_15M			0x00
#define HIU_SUBCMD_SECTION_30M			0x01
#define HIU_SUBCMD_SECTION_60M			0x02

//HIU_CMD_MDSEN
//HIU_CMD_READMDSEN
#define HIU_SUBCMD_MDSEN_HIGH			0x00
#define HIU_SUBCMD_MDSEN_MIDDLE			0x01
#define HIU_SUBCMD_MDSEN_LOW			0x02

//HIU_CMD_MDSPEED
//HIU_CMD_READMDSPEED
#define HIU_SUBCMD_MDSPEED_HIGH			0x00
#define HIU_SUBCMD_MDSPEED_MIDDLE		0x01
#define HIU_SUBCMD_MDSPEED_LOW			0x02

//HIU_CMD_MDNOISE
//HIU_CMD_READMDNOISE
#define HIU_SUBCMD_MDNOISE_HIGH			0x00
#define HIU_SUBCMD_MDNOISE_MIDDLE		0x01
#define HIU_SUBCMD_MDNOISE_LOW			0x02

//HIU_CMD_MDMASKXY
//HIU_CMD_READMDMASKXY

//HIU_CMD_RECMODE
//HIU_CMD_READRECMODE
#define HIU_SUBCMD_RECMODE_MANU			0x00
#define HIU_SUBCMD_RECMODE_SCHE			0x01
#define HIU_SUBCMD_RECMODE_MOTI			0x02
#define HIU_SUBCMD_RECMODE_ALAR			0x03
#define HIU_SUBCMD_RECMODE_MOTI_ALAR	0x04


//HIU_CMD_VREC
//HIU_CMD_READVREC
#define HIU_SUBCMD_VREC_START   		0x00
#define HIU_SUBCMD_VREC_STOP    		0x01
#define HIU_SUBCMD_VREC_PREDEERMINED	0x02


//HIU_CMD_MDMASKSAVE
//HIU_CMD_READMASKNO

//**********************************************************
//        Non support SUB_CMD
//**********************************************************

//HIU_CMD_DSTAMP
#define HIU_SUBCMD_DSTAMP_DISABLE   	0x00
#define HIU_SUBCMD_DSTAMP_ENABLE    	0x01

//HIU_CMD_VRES
#define HIU_SUBCMD_VRES_320X240			0x00
#define HIU_SUBCMD_VRES_640X480			0x01
#define HIU_SUBCMD_VRES_720X480			0x02
#define HIU_SUBCMD_VRES_800X600			0x03



/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

// SPI controller register
static PHIU_REG pRegHIU = (PHIU_REG)HiuCtrlBase;

OS_FLAG_GRP  *gHIUFlagGrp;

//#if HIU_COMMAND
OS_STK  hiuCmdTaskStack[HIUCMD_TASK_STACK_SIZE];
__align(4)char      HIUCommand[MAX_HIU_CMDBUF_SIZE];
__align(4)char      HIUCMDBUF[MAX_HIU_CMDBUF_SIZE];
__align(4)  u8 HIUDMABuf[1024];

//#endif

//*********************************************************************************************************
#if 0
__align(1024)  u8 HIU_DMA_DRAM[1024];
u8 HIUCC = 0;

OS_EVENT* hiuSemEvt;

u8 hiuDataMode = 0;
u32 DataError = 0;

u16 count = 0;
u16 hostCheckSum = 0;
s8 hiuDataCorrect_flag = -1;
u8 hiuDataReadLength = 0;

u8 hiuLogFile[1024 * 15 * 10] = {0};
u16 hiuLogDataCount = 0;
u32 hiuLogPosition = 0; 

u8 hiuTimeFile[256] = {0};
u16 DateFileFormat = 0;
u16 TimeFileFormat = 0;

u32 hiuOpStatus = 0;

u8 sysIsCardStatus_OK;

u8 HIU_CMD_SIZE= 4;

#endif
/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void hiu_init(void);


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */	
 
s32 Hiuc_Reset(void)
{
	u8 i;

	pRegHIU->HIU_CTL0 = HIU_RST; // reset HIU	 , auto clear

	for(i=0;i<10;i++)
		i=i;
	pRegHIU->HIU_CTL0 = 0;
	for(i=0;i<10;i++)
		i=i;
	pRegHIU->HIU_CTL0 = HIU_RST; // reset HIU	 , auto clear
	for(i=0;i<10;i++)
		i=i;
	pRegHIU->HIU_CTL0 = 0;
	for(i=0;i<10;i++)
		i=i;

	return 1;
}


void Hiuc_setConfig(HIU_CfgData *pHIUCfg)
{
        
    
	pRegHIU->INT_HIU =  pHIUCfg->IntEnable ;
	pRegHIU->HIU_CTL1 = pHIUCfg->CTL1;
	pRegHIU->HIU_CTL0 = pHIUCfg->CTL0|HIUC_IF_SEL|HIU_OUTPUT_EN;
	pRegHIU->DRAM_SADDR = (u32)HIUDMABuf;
	pRegHIU->CMD_HIU[0] = 0x000000FF;


}


u32 Hiuc_ReciveCMD(char * buf , u32 size)
{
u32 i, dwsize;
u32 *ptr = (u32 *) buf;

	if (size == 0)
		return 0;
	
	if (size>16)
		size =16;
		
	dwsize = (size+3)%4;

	for( i=0 ; i<dwsize ; i++)
		ptr[i] = pRegHIU->CMD_HIU[i];

	return size;


}


u32 Hiuc_SendStatus(u8 * buf , u32 size)
{
u32 i, dwsize;
u32 *ptr = (u32 *) buf;

	if (size == 0)
		return 0;
	
	if (size>16)
		size =16;
		
	dwsize = (size+3)%4;

	for( i=1 ; i<dwsize ; i++)
		pRegHIU->CMD_HIU[i] = ptr[i];

	pRegHIU->CMD_HIU[0] = ptr[0];

	return size;


}



/*

Routine Description:

	The IRQ handler of host interface unit.

Arguments:

	None.

Return Value:

	None.

*/
void hiuIntHandler(void)
{

    INT32U  uiHIUIntSts, uiHIUIntFlag=0;
    INT8U   err;


	uiHIUIntSts = pRegHIU->STATUS_HIU;

	if(uiHIUIntSts &HIU_INT_CMD_STS)
	{
		Hiuc_ReciveCMD(HIUCommand, HIU_CMDBUF_SIZE);
		uiHIUIntFlag |= FLAHIU_CMD_READY;
	}
	if(uiHIUIntSts &HIU_INT_DATA_STS)
		uiHIUIntFlag |= FLAHIU_DATA_READY;

    OSFlagPost(gHIUFlagGrp, uiHIUIntFlag, OS_FLAG_SET, &err);
}


//-----------------------------------------------------
//Driver
//------------------------------------------------------


void Hiu_CmdInit(void)
{
    INT8U   Result;
    Result  = OSTaskCreate(HIUCMD_TASK, HIUCMD_TASK_PARAMETER, HIUCMD_TASK_STACK, HIUCMD_TASK_PRIORITY);
    if(Result != OS_NO_ERR)
    {
        DEBUG_HIU("hiuCmdInit error!!!\n");
    }
}

u32  Hiu_open(void)
{
    INT8U   err;

	//Disable HIU clock
	SYS_CTL0 &= ~SYS_CTL0_HIU_CKEN ;

	//set pin mux. 
	//gpioInit() do it
	
	//Disable HIU clock
	SYS_CTL0 |= SYS_CTL0_HIU_CKEN ;

	Hiu_init();
	marsIntFIQEnable(INT_FIQMASK_HIU);

    gHIUFlagGrp = OSFlagCreate(0x00000000, &err);
    
    return 1;
    
}



void Hiu_init(void)
{
    HIU_CfgData tmpHIUfg;
    HIU_CfgData *pHIUCfg = (HIU_CfgData*)&tmpHIUfg;

    pHIUCfg->CTL0 = HIUC_DMA_EN|HIUC_FIFO_RECV|HIUC_WR_FALL|HIUC_ER_FALL|
                    HIUC_IF_I80|HIUC_SGL_RS|HIUC_DT_8b|HIUC_LATCH_SYSCLK;
    pHIUCfg->CTL1 = HIUC_P_SIZE_256|(HIUC_CMD_SIZE_8B<<HIUC_CMD_SIZE_SHIFT)|
		            (4<<HIUC_CLK_DIV_SHIFT)|HIUC_DMAMODE_DMA ;
    pHIUCfg->IntEnable = HIUC_INT_CMD_MASK|HIUC_INT_DATA_MASK ;

    Hiuc_setConfig(pHIUCfg);

}

void Hiu_CmdParse(char *CmdString)
{

	if (*CmdString & 0x80)
	{
		DEBUG_HIU("Command Error, 0x%02X \r\n",CmdString[0]);
		return;
	}

	if ((*CmdString & 0x30)==0x00)
	{
		switch(*CmdString)
		{

			case HIU_CMD_KEY:

				switch(CmdString[1])
				{
					case HIU_SUBCMD_KEY_UP: 	UIKey	= UI_KEY_UP;		break;
					case HIU_SUBCMD_KEY_LEFT: 	UIKey	= UI_KEY_LEFT;		break;
					case HIU_SUBCMD_KEY_DOWN: 	UIKey	= UI_KEY_DOWN;		break;
					case HIU_SUBCMD_KEY_RIGHT: 	UIKey	= UI_KEY_RIGHT; 	break;
					case HIU_SUBCMD_KEY_ENTER: 	UIKey	= UI_KEY_ENTER; 	break;
					case HIU_SUBCMD_KEY_DELETE: UIKey	= UI_KEY_DELETE;	break;
					case HIU_SUBCMD_KEY_PLAY: 	UIKey	= UI_KEY_PLAY;		break;
					case HIU_SUBCMD_KEY_MENU: 	UIKey	= UI_KEY_MENU;		break;
					case HIU_SUBCMD_KEY_MODE: 	UIKey	= UI_KEY_MODE;		break;
					case HIU_SUBCMD_KEY_STOP: 	UIKey	= UI_KEY_STOP;		break;
//					case HIU_SUBCMD_KEY_CHSEL:	UIKey	= UI_KEY_MODE;		break;

					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
						
				}

				break;

			case HIU_CMD_OSD:

				switch(CmdString[1])
				{
					case HIU_SUBCMD_OSD_DISABLE: UIKey	= UI_KEY_OSD_DISABLE;		break;
					case HIU_SUBCMD_OSD_ENABLE:	UIKey	= UI_KEY_OSD_ENABLE;		break;
						
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}

				break;
			
			case HIU_CMD_TV:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_TV_NTSC: 	UISubKey	= UI_SUBKEY_TV_NTSC;		break;
					case HIU_SUBCMD_TV_PAL:		UISubKey	= UI_SUBKEY_TV_PAL;		break;
						
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_TV;

				break;

			case HIU_CMD_READTV:
				
				UIKey	= UI_KEY_READTV;
				break;

			case HIU_CMD_TIME:

				//year
				//Month
				if (CmdString[2]>12)
					CmdString[2]=12;
				//day				
				if (CmdString[3]>31)
					CmdString[3]=31;
				//hour
				if (CmdString[4]>23)
					CmdString[4]=23;
				//minute
				if (CmdString[5]>59)
					CmdString[5]=59;
				//second
				if (CmdString[6]>59)
					CmdString[6]=59;
			
			
				KeyTime.year  = (u8)(CmdString[1] & 0xFF);
				KeyTime.month = (u8)(CmdString[2] & 0xFF);
				KeyTime.day   = (u8)(CmdString[3] & 0xFF);
				KeyTime.hour  = (u8)(CmdString[4] & 0xFF);
				KeyTime.min   = (u8)(CmdString[5] & 0xFF);
				KeyTime.sec   = (u8)(CmdString[6] & 0xFF);
			
#if 0	
				DEBUG_HIU("Date:20%02d/%02d/%02d/%02d/%02d/%02d\r\n", KeyTime.year
													   , KeyTime.month
													   , KeyTime.day
													   , KeyTime.hour
													   , KeyTime.min
													   , KeyTime.sec  );
#endif
				UIKey	= UI_KEY_TIME;
				

				break;

			case HIU_CMD_READTIME:
				
				UIKey	= UI_KEY_READTIME;
				break;

			case HIU_CMD_ALARMPD:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_ALARMPD_OFF: 	UISubKey	= UI_SUBKEY_ALARMPD_OFF;		break;
					case HIU_SUBCMD_ALARMPD_10S: 	UISubKey	= UI_SUBKEY_ALARMPD_10S;		break;
					case HIU_SUBCMD_ALARMPD_20S: 	UISubKey	= UI_SUBKEY_ALARMPD_20S;		break;
					case HIU_SUBCMD_ALARMPD_30S: 	UISubKey	= UI_SUBKEY_ALARMPD_30S;		break;
						
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_ALARMPD;

				break;

			case HIU_CMD_READALARMPD:
				
				UIKey	= UI_KEY_READALARMPD;
				break;

			case HIU_CMD_LANG:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_LANG_ENG: 	UISubKey	= UI_SUBKEY_LANG_ENG;		break;
					case HIU_SUBCMD_LANG_TCHI: 	UISubKey	= UI_SUBKEY_LANG_TCHI;		break;
					case HIU_SUBCMD_LANG_SCHI: 	UISubKey	= UI_SUBKEY_LANG_SCHI;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_ALARMPD;

				break;

			case HIU_CMD_READLANG:
				
				UIKey	= UI_KEY_READLANG;
				break;

			//case HIU_CMD_CHAN:
			//case HIU_CMD_READCHAN:

			case HIU_CMD_CARDINFO:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_CARD_TOTAL: 	UISubKey	= UI_SUBKEY_CARD_TOTAL;	break;
					case HIU_SUBCMD_CARD_USAGE: 	UISubKey	= UI_SUBKEY_CARD_USAGE;	break;
					case HIU_SUBCMD_CARD_REMAIN: 	UISubKey	= UI_SUBKEY_CARD_REMAIN; break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_CARDINFO;

				break;

			default:
				
				DEBUG_HIU("System Command Error, 0x%02X \r\n",*CmdString);

		}

	}
	else if((*CmdString & 0x30)==0x10)
	{
		switch(*CmdString)
		{

			case HIU_CMD_VPLAY:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_VPLAY_STOP: 	UISubKey	= UI_SUBKEY_VPLAY_STOP;		break;
					case HIU_SUBCMD_VPLAY_PLAY: 	UISubKey	= UI_SUBKEY_VPLAY_PLAY;		break;
					case HIU_SUBCMD_VPLAY_PAUSE: 	UISubKey	= UI_SUBKEY_VPLAY_PAUSE;	break;
					case HIU_SUBCMD_VPLAY_16XREW: 	UISubKey	= UI_SUBKEY_VPLAY_16XREW;	break;
					case HIU_SUBCMD_VPLAY_8XREW: 	UISubKey	= UI_SUBKEY_VPLAY_8XREW;	break;
					case HIU_SUBCMD_VPLAY_4XREW: 	UISubKey	= UI_SUBKEY_VPLAY_4XREW;	break;
					case HIU_SUBCMD_VPLAY_2XREW: 	UISubKey	= UI_SUBKEY_VPLAY_2XREW;	break;
					case HIU_SUBCMD_VPLAY_1XREW: 	UISubKey	= UI_SUBKEY_VPLAY_1XREW;	break;
					case HIU_SUBCMD_VPLAY_1XFF: 	UISubKey	= UI_SUBKEY_VPLAY_1XFF;		break;
					case HIU_SUBCMD_VPLAY_2XFF: 	UISubKey	= UI_SUBKEY_VPLAY_2XFF;		break;
					case HIU_SUBCMD_VPLAY_4XFF: 	UISubKey	= UI_SUBKEY_VPLAY_4XFF;		break;
					case HIU_SUBCMD_VPLAY_8XFF: 	UISubKey	= UI_SUBKEY_VPLAY_8XFF;		break;
					case HIU_SUBCMD_VPLAY_16XFF: 	UISubKey	= UI_SUBKEY_VPLAY_16XFF;	break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_VPLAY;

				break;

			case HIU_CMD_FILE:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_FILE_HEAD: 	UISubKey	= UI_SUBKEY_FILE_HEAD;	break;
					case HIU_SUBCMD_FILE_TAIL: 	UISubKey	= UI_SUBKEY_FILE_TAIL;	break;
					case HIU_SUBCMD_FILE_NEXT: 	UISubKey	= UI_SUBKEY_FILE_NEXT;	break;
					case HIU_SUBCMD_FILE_PRE: 	UISubKey	= UI_SUBKEY_FILE_PRE; 	break;
					case HIU_SUBCMD_FILE_NANE: 	UISubKey	= UI_SUBKEY_FILE_NANE; 	break;
					case HIU_SUBCMD_FILE_DATE: 	UISubKey	= UI_SUBKEY_FILE_DATE;	break;
					//case HIU_SUBCMD_FILE_SIZE: 	UISubKey	= UI_SUBKEY_FILE_SIZE;	break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_FILE;

				break;

			case HIU_CMD_DIR:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					//case HIU_SUBCMD_DIR_HEAD: 	UISubKey	= UI_SUBKEY_DIR_HEAD;	break;
					//case HIU_SUBCMD_DIR_TAIL: 		UISubKey	= UI_SUBKEY_DIR_TAIL;	break;
					//case HIU_SUBCMD_DIR_NEXT: 	UISubKey	= UI_SUBKEY_DIR_NEXT;	break;
					//case HIU_SUBCMD_DIR_PRE: 		UISubKey	= UI_SUBKEY_DIR_PRE;	break;
					case HIU_SUBCMD_DIR_NAME: 	UISubKey	= UI_SUBKEY_DIR_NAME;	break;
					case HIU_SUBCMD_DIR_DATE: 	UISubKey	= UI_SUBKEY_DIR_DATE;	break;
					case HIU_SUBCMD_DIR_FILENO: 	UISubKey	= UI_SUBKEY_DIR_FILENO;	break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_DIR;

				break;

			case HIU_CMD_DEL:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_DEL_FILE: 	UISubKey	= UI_SUBKEY_DEL_FILE;	break;
					case HIU_SUBCMD_DEL_DIR: 	UISubKey	= UI_SUBKEY_DEL_DIR;	break;
					case HIU_SUBCMD_DEL_ALL: 	UISubKey	= UI_SUBKEY_DEL_ALL;	break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_DEL;

				break;

			case HIU_CMD_FORMAT:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_FORMAT_SD: 	UISubKey	= UI_SUBKEY_FORMAT_SD;		break;
					//case HIU_SUBCMD_FORMAT_NAND: 	UISubKey	= UI_SUBKEY_FORMAT_NAND;		break;
					//case HIU_SUBCMD_FORMAT_UPDATE: 	UISubKey	= UI_SUBKEY_FORMAT_UPDATE;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_FORMAT;

				break;

			case HIU_CMD_AVOL:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					//case HIU_SUBCMD_AVOL_MIN: 	UISubKey	= UI_SUBKEY_AVOL_MIN;		break;
					//case HIU_SUBCMD_AVOL_MAX: 	UISubKey	= UI_SUBKEY_AVOL_MAX;		break;
					case HIU_SUBCMD_AVOL_INC: 	UISubKey	= UI_SUBKEY_AVOL_INC;			break;
					case HIU_SUBCMD_AVOL_DEC: 	UISubKey	= UI_SUBKEY_AVOL_DEC;			break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_AVOL;

				break;

			case HIU_CMD_READAVOL:
				
				UIKey	= UI_KEY_READAVOL;
				break;


	
			default:
			
				DEBUG_HIU("Playback Command Error, 0x%02X \r\n",*CmdString);
	
		}
	
	}
	else if((*CmdString & 0x30)==0x20)
	{
		switch(*CmdString)
		{

			case HIU_CMD_VQUAL:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case 0: 	UISubKey	= UI_SUBKEY_VQUAL_HIGH;		break;
					case 1: 	UISubKey	= UI_SUBKEY_VQUAL_MIDDLE;	break;
					case 2: 	UISubKey	= UI_SUBKEY_VQUAL_LOW;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_VQUAL;

				break;

			case HIU_CMD_READVQUAL:
				
				UIKey	= UI_KEY_READVQUAL;
				break;

			case HIU_CMD_FRAMERATE:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_FRAMERATE_30: 	UISubKey	= UI_SUBKEY_FRAMERATE_30;		break;
					case HIU_SUBCMD_FRAMERATE_15: 	UISubKey	= UI_SUBKEY_FRAMERATE_15;		break;
					case HIU_SUBCMD_FRAMERATE_05: 	UISubKey	= UI_SUBKEY_FRAMERATE_05;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_FRAMERATE;

				break;

			case HIU_CMD_READFRAMERATE:
				
				UIKey	= UI_KEY_READFRAMERATE;
				break;

			case HIU_CMD_STIME1:

				//year
				//Month
				if (CmdString[2]>12)
					CmdString[2]=12;
				//day				
				if (CmdString[3]>31)
					CmdString[3]=31;
				//hour
				if (CmdString[4]>23)
					CmdString[4]=23;
				//minute
				if (CmdString[5]>59)
					CmdString[5]=59;
				//second
				if (CmdString[6]>59)
					CmdString[6]=59;
			
			
				KeyTime.year  = (u8)(CmdString[1] & 0xFF);
				KeyTime.month = (u8)(CmdString[2] & 0xFF);
				KeyTime.day   = (u8)(CmdString[3] & 0xFF);
				KeyTime.hour  = (u8)(CmdString[4] & 0xFF);
				KeyTime.min   = (u8)(CmdString[5] & 0xFF);
				KeyTime.sec   = (u8)(CmdString[6] & 0xFF);
			
#if 0	
				DEBUG_HIU("Date:20%02d/%02d/%02d/%02d/%02d/%02d\r\n", KeyTime.year
													   , KeyTime.month
													   , KeyTime.day
													   , KeyTime.hour
													   , KeyTime.min
													   , KeyTime.sec  );
#endif
				UIKey	= UI_KEY_STIME1;
				

				break;

			case HIU_CMD_READSTIME1:
				
				UIKey	= UI_KEY_READSTIME1;
				break;

			case HIU_CMD_STIME2:

				//year
				//Month
				if (CmdString[2]>12)
					CmdString[2]=12;
				//day				
				if (CmdString[3]>31)
					CmdString[3]=31;
				//hour
				if (CmdString[4]>23)
					CmdString[4]=23;
				//minute
				if (CmdString[5]>59)
					CmdString[5]=59;
				//second
				if (CmdString[6]>59)
					CmdString[6]=59;
			
			
				KeyTime.year  = (u8)(CmdString[1] & 0xFF);
				KeyTime.month = (u8)(CmdString[2] & 0xFF);
				KeyTime.day   = (u8)(CmdString[3] & 0xFF);
				KeyTime.hour  = (u8)(CmdString[4] & 0xFF);
				KeyTime.min   = (u8)(CmdString[5] & 0xFF);
				KeyTime.sec   = (u8)(CmdString[6] & 0xFF);
			
#if 0	
				DEBUG_HIU("Date:20%02d/%02d/%02d/%02d/%02d/%02d\r\n", KeyTime.year
													   , KeyTime.month
													   , KeyTime.day
													   , KeyTime.hour
													   , KeyTime.min
													   , KeyTime.sec  );
#endif
				UIKey	= UI_KEY_STIME2;
				

				break;

			case HIU_CMD_READSTIME2:
				
				UIKey	= UI_KEY_READSTIME2;
				break;

			case HIU_CMD_OVERWRITE:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_OVERWRITE_OFF: 	UISubKey	= UI_SUBKEY_OVERWRITE_OFF;		break;
					case HIU_SUBCMD_OVERWRITE_ON: 	UISubKey	= UI_SUBKEY_OVERWRITE_ON;	break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_OVERWRITE;

				break;

			case HIU_CMD_READOVERWRITE:
				
				UIKey	= UI_KEY_READOVERWRITE;
				break;

			case HIU_CMD_SECTION:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_SECTION_15M: 	UISubKey	= UI_SUBKEY_SECTION_15M;		break;
					case HIU_SUBCMD_SECTION_30M: 	UISubKey	= UI_SUBKEY_SECTION_30M;	break;
					case HIU_SUBCMD_SECTION_60M: 	UISubKey	= UI_SUBKEY_SECTION_60M;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_SECTION;

				break;

			case HIU_CMD_READSECTION:
				
				UIKey	= UI_KEY_READSECTION;
				break;

			case HIU_CMD_MDSEN:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_MDSEN_HIGH: 	UISubKey	= UI_SUBKEY_MDSEN_HIGH;		break;
					case HIU_SUBCMD_MDSEN_MIDDLE: 	UISubKey	= UI_SUBKEY_MDSEN_MIDDLE;	break;
					case HIU_SUBCMD_MDSEN_LOW: 		UISubKey	= UI_SUBKEY_MDSEN_LOW;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_MDSEN;

				break;

			case HIU_CMD_READMDSEN:
				
				UIKey	= UI_KEY_READMDSEN;
				break;

			case HIU_CMD_MDSPEED:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_MDSPEED_HIGH: 	UISubKey	= UI_SUBKEY_MDSPEED_HIGH;		break;
					case HIU_SUBCMD_MDSPEED_MIDDLE: UISubKey	= UI_SUBKEY_MDSPEED_MIDDLE;	break;
					case HIU_SUBCMD_MDSPEED_LOW: 	UISubKey	= UI_SUBKEY_MDSPEED_LOW;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_MDSPEED;

				break;

			case HIU_CMD_READMDSPEED:
				
				UIKey	= UI_KEY_READMDSPEED;
				break;

			case HIU_CMD_MDNOISE:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_MDNOISE_HIGH: 	UISubKey	= UI_SUBKEY_MDNOISE_HIGH;		break;
					case HIU_SUBCMD_MDNOISE_MIDDLE: UISubKey	= UI_SUBKEY_MDNOISE_MIDDLE;	break;
					case HIU_SUBCMD_MDNOISE_LOW: 	UISubKey	= UI_SUBKEY_MDNOISE_LOW;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_MDNOISE;

				break;

			case HIU_CMD_READMDNOISE:
				
				UIKey	= UI_KEY_READMDNOISE;
				break;

			case HIU_CMD_MDMASKXY:
				
				UISubKey = CmdString[1];
				UISubKey1 = CmdString[2];
				
				UIKey	= UI_KEY_READMDMASKXY;
				break;

			case HIU_CMD_READMDMASKXY:
				
				UISubKey = CmdString[1];
				UISubKey1 = CmdString[2];
				UISubKey2 = CmdString[3];
				
				UIKey	= UI_KEY_MDMASKXY;
				break;

			case HIU_CMD_RECMODE:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_RECMODE_MANU: 		UISubKey	= UI_SUBKEY_RECMODE_MANU;		break;
					case HIU_SUBCMD_RECMODE_SCHE: 		UISubKey	= UI_SUBKEY_RECMODE_SCHE;	break;
					case HIU_SUBCMD_RECMODE_MOTI: 		UISubKey	= UI_SUBKEY_RECMODE_MOTI;		break;
					case HIU_SUBCMD_RECMODE_ALAR: 		UISubKey	= UI_SUBKEY_RECMODE_ALAR;	break;
					case HIU_SUBCMD_RECMODE_MOTI_ALAR: 	UISubKey	= UI_SUBKEY_RECMODE_MOTI_ALAR;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_RECMODE;

				break;

			case HIU_CMD_READRECMODE:
				
				UIKey	= UI_KEY_READRECMODE;
				break;

			case HIU_CMD_VREC:

				UISubKey = UI_KEY_READY;
				switch(CmdString[1])
				{
					case HIU_SUBCMD_VREC_START: 	UISubKey	= UI_SUBKEY_VREC_START;		break;
					case HIU_SUBCMD_VREC_STOP: 	UISubKey	= UI_SUBKEY_VREC_STOP;		break;
					default:
						DEBUG_HIU("0x%02X sub-Command Error, 0x%02X \r\n",CmdString[0],CmdString[1]);
				}
				
				if (UISubKey != UI_KEY_READY)
					UIKey	= UI_KEY_VREC;

				break;

			case HIU_CMD_READVREC:
				
				UIKey	= UI_KEY_READVREC;
				break;

			case HIU_CMD_MDMASKSAVE:
				
				UIKey	= UI_KEY_MDMASKSAVE;
				break;

			case HIU_CMD_READMASKNO:
				
				UIKey	= UI_KEY_READMASKNO;
				break;

	
			default:
			
				DEBUG_HIU("record Command Error, 0x%02X \r\n",*CmdString);
	
		}
	
	}
}

void hiuCmdTask(void *pdata)
{
//INT32U  uiHiuCmdBufSize;
OS_FLAGS HIUstatus;
INT8U	err;
int i;
	Hiu_open();

	while(1)
	{

		// wait for interrupt from UART
		HIUstatus = OSFlagPend(gHIUFlagGrp, FLAHIU_CMD_READY|FLAHIU_DATA_READY,
								 OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

		if (HIUstatus | FLAHIU_CMD_READY)
		{
			if(UIKey == UI_KEY_READY)
			{
			  for(i=0; i<MAX_HIU_CMDBUF_SIZE; i++)
				HIUCMDBUF[i] = HIUCommand[i];	
			  Hiu_CmdParse(HIUCMDBUF);
			}  

		}
//        uiHiuCmdBufSize = HIU_CMDBUF_SIZE;
//        marsHiuGetString(HIUCommand, &uiHiuCmdBufSize);
//		DEBUG_UART("\r\n");
	
//        if(UIKey == UI_KEY_READY)
//            HiuCmdParse(HIUCommand);
        OSTimeDly(1);
	}
}

#if 0
/*

Routine Description:

	Setting Hiu in WRITE CMD mode

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 hiuWriteCMDMode()
{
	u8 clk_div =4;

	HiuCtrl0 =	HIU_OUTPUT_EN|
				HIU_FIFO_SEND|
				HIU_WR_RISE|
				HIU_ENDIAN_BIG|
				HIU_IF_I80|
				HIU_SGL_RS|
				HIU_DT_8b|
				HIU_LATCH_SYSCLK;
				//HIU_LATCH_WR;
				

	HiuCtrl1 =	HIU_P_SIZE_256 |
				HIU_DMAMODE_DMA |
				(clk_div<<16) |
				((HIU_CMD_SIZE-1) << HIU_CMD_SIZE_SHIFT)&HIU_CMD_SIZE_MASK;

	return 1;
}

s32 hiuRecvData()
{
	u8 clk_div =4;
	
	HiuCtrl0 =	HIU_OUTPUT_EN|
				HIU_FIFO_RECV|
				HIU_WR_RISE|
				HIU_ENDIAN_BIG|
				HIU_IF_I80|
				HIU_SGL_RS|
				HIU_DT_8b|
				HIU_LATCH_SYSCLK;

	HiuCtrl1 =	HIU_P_SIZE_256 |//HIU_P_SIZE_4K |
				HIU_DMAMODE_DMA |
				(clk_div<<16) |
				((HIU_CMD_SIZE-1) << HIU_CMD_SIZE_SHIFT)&HIU_CMD_SIZE_MASK;
	
}

/*

Routine Description:

	Trigger DMA

Arguments:

	None.

Return Value:

	None.

*/
void HiuDMA_Ena()
{	
	HiuCtrl0 |=	HIU_DMA_EN;
}

/* SW 0702 S */
void HiuDMA_DIS()
{
	HiuCtrl0 &=	~(HIU_DMA_EN);
}
/* SW 0702 E */

void HiuInt_Ena()
{
	HiuIntEna = 0x00000000; // 0 : interrupt enable , 1 : interrupt disable
}

/* SW 1113 S */
void HiuReflashStatus(void)
{
	u32 data;

	data = HiuCmd;
	
	if (sysIsCardStatus_OK == 1)
		hiuOpStatus |= 0x10;
	else if (sysIsCardStatus_OK == 0)
		hiuOpStatus &= ~(0x10);

	HiuCmd = ((hiuOpStatus << 16) | (data &= 0xffff));
}
/* SW 1113 E */


void HiuCmdComp(u32 data)
{
	if(UIKey != UI_KEY_READY)
	{
		HIUCC = 1;
		return;
	}

	hiuReset();
	hiuWriteCMDMode();

	HiuCmd = ((hiuOpStatus << 16) | data);
	HIUCC = 0;
}


void HiuSetDataSize(u32 DataSize)
{
	HiuCmd = (DataSize | 0xFF000000);
}

void HiuSetDMAAdr(u32 address)
{
	HiuDramStart = address;
}

void HiuSendData(u8 * buf , s32 size)
{
	u8 err ;
	u8 * buf_ptr;
	u8 * temp_ptr;
	u32 j;
	u32 i,iterator;
	u32 ErrCode;

	Gpio0Dir = 0xF3FFFFFF;
	gpioSetLevel(0, 26, 1);
	gpioSetLevel(0, 27, 1);

	buf_ptr = buf;

	if(size % 256 == 0)
		iterator = size>> 8;
	else
		iterator = (size>> 8) + 1;

	hiuDataMode= 1;
	hiuReset();
	hiuWriteCMDMode();
	HiuCmd = (0xF0000000 | size);
	OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);

	hiuReset();

	i = 0;
	while(i < iterator)
	{
		hiuReset();

		hiuWriteCMDMode();

		HiuSetDMAAdr((u32)buf_ptr);

		gpioSetLevel(0, 26, 0);
		HiuDMA_Ena();

		OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);
		gpioSetLevel(0, 26, 1);

		gpioSetLevel(0, 27, 0);

		gpioSetLevel(0, 27, 1);

		if(DataError == 0)
		{
			buf_ptr += 256;
			i++;
		}
		else
		{
			DEBUG_HIU("Data Error ==> Resend\n");
		}
		OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);
	}
	hiuDataMode= 0;
	HiuCmd  = 0x5678;
}

/* SW 0622 S */
void HiuRecvData(u8 * buf , s32 size)
{
	u8 err;
	u32 i,length = 0;
	u8 type = 0,xpos = 0, ypos = 0;
	u8 dstbuf[256]={0};
	
	hiuReset();

	hiuRecvData();

	HiuSetDMAAdr((u32)buf);

	HiuSetDataSize(size);

	/* SW 1019 S */
	HiuCmd = ((hiuOpStatus << 16) | 0x4321);
	/* SW 1019 E */

	OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);

	hiuLogFileParse(buf, dstbuf, &xpos, &ypos, &type ,&length);

	if (type == 1)
	{
		for (i=0 ; i<length ; i++)
			hiuLogFile[hiuLogPosition + i] = dstbuf[i];
		hiuLogPosition += length;
	}
	else if (type ==2)
    {
	}
	else if (type == 3)
	{
		//OSD_String1(OSD_SizeX , dstbuf , 8 , 8 , xpos , ypos , 0);
	}
	else if (type == 4)
	{
		for (i=0 ; i<length ; i++)
			hiuTimeFile[i] = dstbuf[i];
		hiuTimeParse(length);
	}
}

void HiuRecvDataMode(u8 * buf , s32 size)
{
	u8 err;

	hiuReset();

	hiuRecvData();

	HiuSetDMAAdr((u32)buf);

	HiuSetDataSize(size);

	HiuCmd = 0x87654321;

	OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);
}

void HiuSendDataMode(u8 * buf)
{
	u8 err;

	hiuReset();

	hiuWriteCMDMode();

	HiuSetDMAAdr((u32)buf);

	HiuDMA_Ena();

	OSSemPend(hiuSemEvt, OS_IPC_WAIT_FOREVER, &err);

	HiuDMA_DIS();
}

/*

Routine Description:

	Initialize the host interface unit.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 hiuInit(void)
{
	u32 i;
	
	hiuSemEvt = OSSemCreate(0);

	for (i = 0; i < 256; i++)
		HIU_DMA_DRAM[255 - i] = i;

	HiuCmdComp(0x5678);
	
	return 1;	
}

/* SW 0730 S */
void hiuLogFileParse(u8 * srcbuf, u8 * dstbuf, u8* x, u8* y, u8* Type, u32* Length)
{
    u16 i;
    //u8 logFileLength;

    *x = srcbuf[0];

    *y = srcbuf[1];

    *Type = srcbuf[2];

    *Length = srcbuf[3];

    for (i=0 ; i<*Length ; i++)
        dstbuf[i] = srcbuf[i+4];
}
/* SW 0730 E */

/* SW 1003 S */
void hiuTimeParse(u32 length)
{
	HIU_TIME_CFG timecfg;
	u8* curtimeptr = hiuTimeFile,*curdigtimeptr;
	u8 curdigtime[30], i=0;
	
#if (HIU_CURRENT_TIME_FORMAT == 0x00)

	curdigtimeptr = curdigtime;
	/* Set real value from ASCII value */
	while (i < length)
	{
		if ((*curtimeptr != 0x2F) && (*curtimeptr != 0x3A))
		{
			*curdigtimeptr = (*curtimeptr - 0x30);
			curdigtimeptr++;
		}
		i++;
		curtimeptr++;
	}

	/* Set value into time Config structure */
	timecfg.CurYear = ((curdigtime[0]*1000) + (curdigtime[1]*100) + (curdigtime[2]*10) + (curdigtime[3]));
	timecfg.CurMonth = ((curdigtime[4]*10) + (curdigtime[5]));
	timecfg.CurDate = ((curdigtime[6]*10) + (curdigtime[7]));
	timecfg.CurHour = ((curdigtime[8]*10) + (curdigtime[9]));
	timecfg.CurMin =  ((curdigtime[10]*10) + (curdigtime[11]));
	timecfg.CurSec =  ((curdigtime[12]*10) + (curdigtime[13]));

#elif (HIU_CURRENT_TIME_FORMAT == 0x01)

	timecfg.CurYear = (curtimeptr[0] | (curtimeptr[1] << 8));
	timecfg.CurMonth = curtimeptr[2];
	timecfg.CurDate = curtimeptr[3];
	timecfg.CurHour = curtimeptr[4];
	timecfg.CurMin = curtimeptr[5];
	timecfg.CurSec = curtimeptr[6]; 	

#endif
	DateFileFormat = (((timecfg.CurYear - 1980) << 9) | ((u16)timecfg.CurMonth << 5) | (timecfg.CurDate));
	TimeFileFormat = (((u16)timecfg.CurHour << 11) | ((u16)timecfg.CurMin << 5) | (timecfg.CurSec)); 
}
/* SW 1003 E */

/*

Routine Description:

	The test routine of host interface.

Arguments:

	None.

Return Value:

	None.

*/
u8 hiuWriteData(void)
{
	u8 err, DataReadLength;
	u32 i,bufCount = 0,divideCount = 0,divideRemainder = 0;
	/* Set 1M buffer */
	u8* codeAddr = (u8*)((DRAM_MEMORY_END + 4) - 0x00100000);
	u8* bufptr;
	u8 buf[256];
	FS_FILE* pFile;
	s32 codeSize,readSize,sizeCount = 0;
	u32 checkSum = 0;

	/* Receive File type */
	HiuRecvDataMode(HIU_DMA_DRAM,256);

	if ((HIU_DMA_DRAM[0] & 0x07) == 0x01)
	{
		if((pFile = dcfOpen("\\EBOOT.bin","r")) == NULL)
		{
			/* Open File Failed */
			HiuCmd = 0x5a5a5a22;
			return 0;
		}
	}
	else if ((HIU_DMA_DRAM[0] & 0x07) == 0x02)
	{
		if((pFile = dcfOpen("\\NK.bin","r")) == NULL)
		{
			/* Open File Failed */
			HiuCmd = 0x5a5a5a22;
			return 0;
		}
	}
	else if ((HIU_DMA_DRAM[0] & 0x07) == 0x04)
	{
		if((pFile = dcfOpen("\\STEPLDR.nb0","r")) == NULL)
		{
			/* Open File Failed */
			HiuCmd = 0x5a5a5a22;
			return 0;
		}
	}
	else if ((HIU_DMA_DRAM[0] & 0x07) == 0x08)
	{
		if((pFile = dcfOpen("\\pa9001.bin","r")) == NULL)
		{
			/* Open File Failed */
			HiuCmd = 0x5a5a5a22;
			return 0;
		}
	}

	codeSize = pFile->size;

	if (codeSize > 0x00100000)
	{
		divideCount = (codeSize / 0x00100000);
		divideRemainder = (codeSize % 0x00100000);
		dcfRead(pFile, codeAddr, 0x00100000, &readSize);
	}
	else
		dcfRead(pFile, codeAddr,  codeSize, &readSize);
	
	/* Send Code Size to Host */
	hiuDataMode = 1;
	HIU_DMA_DRAM[0] = ((u8 *)(&codeSize))[0];
	HIU_DMA_DRAM[1] = ((u8 *)(&codeSize))[1];
	HIU_DMA_DRAM[2] = ((u8 *)(&codeSize))[2];
	HIU_DMA_DRAM[3] = ((u8 *)(&codeSize))[3];
	HiuSendDataMode(HIU_DMA_DRAM);
	HiuCmd = 0x5a5a5a11;
	while(hiuDataMode == 1);

	bufptr = codeAddr;
	
	while(codeSize > 0)
	{
		hiuDataMode = 1;

		for (i=0 ; i<256 ; i++)
		{
			if (i<=hiuDataReadLength)
			{
				checkSum += bufptr[i];
				HIU_DMA_DRAM[i] = bufptr[i];
			}
			else
				HIU_DMA_DRAM[i] = 0;
		} 		
		/* Compute CheckSum */
		HiuSendDataMode(HIU_DMA_DRAM);

		DataReadLength = hiuDataReadLength;

		HiuCmd = ((0x5a << 24) | checkSum);

		while(hiuDataMode == 1);

		/* Check if Data is correct */
		if (hiuDataCorrect_flag == 1)
		{
			sizeCount += DataReadLength + 1;
			codeSize -= (DataReadLength + 1);
			if ((sizeCount % 0x100000) == 0)
			{
				if ((sizeCount / 0x100000) < divideCount)
					dcfRead(pFile, codeAddr, 0x100000, &readSize);
				else
					dcfRead(pFile, codeAddr, divideRemainder, &readSize);
				bufptr = codeAddr;
			}
			else
			{
				bufptr = bufptr + DataReadLength + 1;;
			}
			checkSum = 0;
		}
		else if (hiuDataCorrect_flag == 0)
		{
			checkSum = 0;
		}

	}

	dcfClose(pFile);

	return 1;
}
#endif

#else
#include "general.h"
#include "board.h"
#include "hiu.h"
#include "hiureg.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
 
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
	
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
 
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */	
	
void hiuCmdInit(void)
{
}

/*

Routine Description:

	Initialize the host interface unit.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 hiuInit(void)
{
	return 1;	
}

/*

Routine Description:

	The IRQ handler of host interface unit.

Arguments:

	None.

Return Value:

	None.

*/
void hiuIntHandler(void)
{

}

/*

Routine Description:

	The test routine of host interface.

Arguments:

	None.

Return Value:

	None.

*/
void hiuTest(void)
{

}

#endif	
