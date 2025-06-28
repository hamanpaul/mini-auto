/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpio.c

Abstract:

   	The routines of general purpose I/O.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2007/10/09	David Hsieh	porting(based on the driver from Newken Technologies Co., Ltd.)

*/

#include "general.h"
#include "board.h"
#include "gpioreg.h"
#include "uiapi.h"

#if(HW_BOARD_OPTION == MR9100_AHDINREC_MUXCOM)
enum{
IR_KEY_REC = 0x1F
};
#endif

void IRCtrReset(BOOLEAN reset)
{
    if(reset == TRUE)
        IR_CTL |= GPIO_IR_RESET;
    else
        IR_CTL &= ~GPIO_IR_RESET;
}

void IRCtrEnable(BOOLEAN enable)
{
    if(enable == TRUE)
        IR_CTL |= GPIO_IR_ENA;
    else
        IR_CTL &= ~GPIO_IR_ENA;
}

void IRIntHandler(void)
{
    u32 intStat     = IR_INT_STATUS;
    u8  recKey;
    u8  key = 0xFF;

/* IR Ctrl 在實體運作(給客戶)的時候不要打印訊息, 會造成BoBo聲音, 盡量在Debug Mode下再印訊息 */

    if(intStat & 0x00000001)
    {
        recKey = (IR_DATA&0x000000FF);
 	    //DEBUG_GPIO("IRIntHandler key %x\n", recKey);
#if(HW_BOARD_OPTION == MR9100_AHDINREC_MUXCOM)
        switch(recKey)
        {
            case IR_KEY_REC:
                key = UI_KEY_IR_REC;
                SpecialKey = key;
                break;
        }
#elif (HW_BOARD_OPTION == MR9200_RX_JIT_M916HN4)
       switch (recKey)
       {
            case 0x1a:
                key = UI_KEY_UP;
	                //DEBUG_GPIO("IR key UI_KEY_UP\n");
                break;

            case 0x15:
                key = UI_KEY_DOWN;
	                //DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                break;

            case 0x0a:
                key = UI_KEY_RIGHT;
	                //DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                break;

            case 0x08:
                key = UI_KEY_LEFT;
	                //DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                break;

            case 0x09:
                key = UI_KEY_ENTER;
	                //DEBUG_GPIO("IR key UI_KEY_ENTER\n");	//menu
                break;


            case 0x14:
                key = UI_KEY_MENU;
	                //DEBUG_GPIO("IR key UI_KEY_MENU\n");		// mute
                break;


            case 0x07:
                key = UI_KEY_RF_QUAD;
	                //DEBUG_GPIO("IR key UI_KEY_RF_QUAD\n");	//TEMP
                break;

			case 0x12:
                key = UI_KEY_LCD_BL;
	                //DEBUG_GPIO("IR key UI_KEY_LCD_BL\n"); //LCD BL
                break;

            case 0x0e:
                key = UI_KEY_TVOUT_DET;
	                //DEBUG_GPIO("IR key UI_KEY_TVOUT_DET\n"); //TV out
                break;

            case 0x18:
                key = UI_KEY_CH1;
	                //DEBUG_GPIO("IR key UI_KEY_CH1\n");		//CH1
                break;

            case 0x1b:
                key = UI_KEY_CH2;
	                //DEBUG_GPIO("IR key UI_KEY_CH2\n");		//CH2
                break;

            case 0x04:
                key = UI_KEY_CH3;
	                //DEBUG_GPIO("IR key UI_KEY_CH3\n");		//CH3
                break;
            case 0x05:
                key = UI_KEY_CH4;
	                //DEBUG_GPIO("IR key UI_KEY_CH4\n");		//CH4
                break;

            case 0x0c:
                key = UI_KEY_REC;
	                //DEBUG_GPIO("IR key UI_KEY_REC\n");		//REC
                break;

            case 0x16:
                key = UI_KEY_DELETE;
	                //DEBUG_GPIO("IR key UI_KEY_DELETE\n");		//DEL
                break;

       }
#elif ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
        switch (recKey)
        {
            case 0x0b:
                key = UI_KEY_MENU;
                //DEBUG_GPIO("IR key UI_KEY_MENU\n");
                break;

            case 0x03:
                key = UI_KEY_UP;
                //DEBUG_GPIO("IR key UI_KEY_UP\n");
                break;

           case 0x09:
                key = UI_KEY_LEFT;
                //DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                break;

           case 0x05:
                key = UI_KEY_ENTER;
                //DEBUG_GPIO("IR key UI_KEY_ENTER\n");
                break;

           case 0x02:
                key = UI_KEY_RIGHT;
                //DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                break;

           case 0x06:
                key = UI_KEY_DOWN;
                //DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                break;

           case 0x08:
                key = UI_KEY_VOL_UP;
                //DEBUG_GPIO("IR key UI_KEY_VOL_UP\n");
                break;

           case 0x1c:
                key = UI_KEY_REC;
                //DEBUG_GPIO("IR key UI_KEY_REC\n");
                break;

           case 0x1d:
                key = UI_KEY_RF_QUAD;
                //DEBUG_GPIO("IR key UI_KEY_RF_QUAD\n");
                break;

           case 0x18:
                key = UI_KEY_VOL_DOWN;
                //DEBUG_GPIO("IR key UI_KEY_VOL_DOWN\n");
                break;

            case 0x19:
                key = UI_KEY_TALK;
                //DEBUG_GPIO("IR key UI_KEY_TALK\n");
                break;

            case 0x4:
                key = UI_KEY_PLAY;
                //DEBUG_GPIO("IR key UI_KEY_PLAY\n");
                break;
        }
#elif (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018)
       switch (recKey)
       {
            case 0x0D:
                key = UI_KEY_UP;
	                //DEBUG_GPIO("IR key UI_KEY_UP\n");
                break;

            case 0x15:
                key = UI_KEY_DOWN;
	                //DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                break;

            case 0x12:
                key = UI_KEY_RIGHT;
	                //DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                break;

            case 0x10:
                key = UI_KEY_LEFT;
	                //DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                break;

            case 0x11:
                key = UI_KEY_ENTER;
	                //DEBUG_GPIO("IR key UI_KEY_ENTER\n");	
                break;


            case 0x0A:
                key = UI_KEY_MENU;
	                //DEBUG_GPIO("IR key UI_KEY_MENU\n");		
                break;

            case 0x02:
                key = UI_KEY_RF_QUAD;
	                //DEBUG_GPIO("IR key UI_KEY_RF_QUAD\n");	
                break;

			case 0x16:
                key = UI_KEY_LCD_BL;
	                //DEBUG_GPIO("IR key UI_KEY_LCD_BL\n"); //LCD BL
                break;

            case 0x00:
                key = UI_KEY_CH1;
	                //DEBUG_GPIO("IR key UI_KEY_CH1\n");		//CH1
                break;

            case 0x01:
                key = UI_KEY_CH2;
	                //DEBUG_GPIO("IR key UI_KEY_CH2\n");		//CH2
                break;

            case 0x04:
                key = UI_KEY_CH3;
	                //DEBUG_GPIO("IR key UI_KEY_CH3\n");		//CH3
                break;
                
            case 0x05:
                key = UI_KEY_CH4;
	                //DEBUG_GPIO("IR key UI_KEY_CH4\n");		//CH4
                break;

            case 0x08:
                key = UI_KEY_REC;
	                //DEBUG_GPIO("IR key UI_KEY_REC\n");		//REC
                break;

            case 0x06:
                key = UI_KEY_ENTER_PRV;
	                //DEBUG_GPIO("IR key UI_KEY_HOME\n");
                break;

            case 0x09:
                key = UI_KEY_AREC;
	                //DEBUG_GPIO("IR key UI_KEY_MOTION\n");
                break;

            case 0x18:
                key = UI_KEY_LIGHT;
	                //DEBUG_GPIO("IR key UI_KEY_LIGHT\n");
                break;
  
            case 0x19:
                key = UI_KEY_ALARM;
	                //DEBUG_GPIO("IR key UI_KEY_ALARM\n");
                break;
  
            case 0x1A:
                key = UI_KEY_TALK;
	                //DEBUG_GPIO("IR key UI_KEY_TALK\n");
                break;
  
       }        
#endif
    }
/*    else
	    DEBUG_GPIO("IRIntHandler Otherwise\n");*/

    if (key != 0xff)
    {
        if(UIKey == UI_KEY_READY || SpecialKey != UI_KEY_READY)
        {
            UIKey = key;
            //DEBUG_GPIO("gpio post uiSemEvt key %d\n",UIKey);
            OSSemPost(uiSemEvt);
        }
        if (MsgKey == UI_KEY_WAIT_KEY)
        {
            MsgKey = key;
            OSMboxPost(message_MboxEvt, "gpio_key");
            //DEBUG_GPIO("gpio post message_MboxEvt key %d\n",MsgKey);
        }
    }
    IRCtrReset(TRUE);
    IRCtrReset(FALSE);
    IRCtrEnable(TRUE);
}

void IRSetCustomCode(u32 customCode)
{
    IR_CUSTOM_CODE &= ~0x0000FFFF;
    IR_CUSTOM_CODE |= (customCode&0x0000FFFF);
}

void IREnableInt(BOOLEAN enable)
{
    if(enable == TRUE)
        IR_INT_EN |= GPIO_IR_INT_ENA;
    else
        IR_INT_EN &= ~GPIO_IR_INT_ENA;
}

void IRSetDiv(u32 divisor)
{
    SYS_CLK4 &= ~0x000000FF;
    SYS_CLK4 |= (divisor&0x000000FF);
}

void IRGetRecCustomCode(u32* RecCustom)
{
    *RecCustom = (IR_RECE_CUSTOM_CODE&0x0000FFFF);
}

void hwIrInit(void)
{
    IRCtrEnable(TRUE);
    IRCtrReset(TRUE);
    IRCtrReset(FALSE);
    IRSetDiv(39);   
    IRSetCustomCode(IR_CUSTOM_CODE_ID);
    IREnableInt(TRUE);
}


