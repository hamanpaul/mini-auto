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

#if(HW_BOARD_OPTION == MR6730_AFN)
    #include "..\..\ui\inc\Key.h"
    #include "..\..\ui\inc\MainFlow.h"		
	
	
	#include"ir_ppm.h"
		
		
		//Key KEYIR_MSG;
		Key KEYIR_MSG = KEY_None;	
		
	
#if (!EXT_IR_CODE)		
	/*
	//
	//	define IR code mappings depends on every project
	//
	
	//ex:
	const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
	{
		//by the sequence of enum
		//
		//	IrKeyId_Up=IrKeyId_Begin,
		//	IrKeyId_Down,
		//	IrKeyId_Left,
		//	IrKeyId_Right,
		//	IrKeyId_Ok,
		//	IrKeyId_Menu,
		//	IrKeyId_Del,
		//	IrKeyId_Esc,
		//	IrKeyId_Play,
		//	IrKeyId_Stop,
		//	IrKeyId_Pwr,
	
		//IRKC_TYPE_A
		{
		0X0D,
		0X15,
		0X10,
		0X12,
		0X11,
		0X02,
		0x0A,
		0X06,
		0X04,
		0x08,
		0X00,
		},
		//IRKC_TYPE_B
		{
		0X19,
		0X1C,
		0X0C,
		0X5E,
		0X18,
		0X47,
		0x09,
		0X43,
		0X44,
		0x07,
		0X45,
		},
	
	};
	*/
	
	
		
	
	
	
#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN)
		//IR customer code 0x807F
		const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
		{
			//IRKC_TYPE_A
			{
				0X0C,
				0X0B,
				0X09,
				0X08,
				0X0A,
				0X10,
				0X03,
				0X0D,
				0X01,
				0X12,		
			},
		
		};
		
#elif(DERIVATIVE_MODEL==MODEL_TYPE_SWG)
		//IR customer code 0x00FF
		const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
		{
			//IRKC_TYPE_A
			{			
				0X40,
				0X19,
				0X07,
				0X09,
				0X15,
				0X0C,
				0X47,
				0X5E,
				0x08,
				0X5A,
				0X45,					
			},
		
		};
	
	
		
		
#elif(DERIVATIVE_MODEL==MODEL_TYPE_YD)
		//IR customer code 0x807F
		const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
		{
			//IRKC_TYPE_A
			{
				0X0C,
				0X0B,
				0X09,
				0X08,
				0X0A,
				0X10,
				0X03,
				0X0D,
				0X01,
				0X12,					
			},
		#if(IR_CODE_TBL_SEL==1)	
		//IR customer code 0x866B
			//IRKC_TYPE_B
			{
				0X1B,//IrKeyId_Up
				0X1A,//IrKeyId_Down
				0X04,//IrKeyId_Left
				0X06,//IrKeyId_Right
				0X18,//IrKeyId_Ok
				0X19,//IrKeyId_Menu
				0X00,//IrKeyId_Del		(X)
				0X07,//IrKeyId_Esc
				0X05,//IrKeyId_Play
				0X00,//IrKeyId_Stop	(X)
			},
		#endif
		};
	
		
#elif(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)	
		//IR customer code 0x00FF
	
		
		const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
		{
			//IRKC_TYPE_A
			{
			0X0D,
			0X15,
			0X10,
			0X12,
			0X11,
			0X02,
			0x0A,
			0X06,
			0X04,
			0x08,
			0X00,
			},
			//IRKC_TYPE_B
			{
			
			//	IrKeyId_Up=IrKeyId_Begin,
			//	IrKeyId_Down,
			//	IrKeyId_Left,
			//	IrKeyId_Right,
			//	IrKeyId_Ok,
			//	IrKeyId_Menu,
			//	IrKeyId_Del,
			//	IrKeyId_Esc,
			//	IrKeyId_Play,
			//	IrKeyId_Stop,
			//	IrKeyId_Pwr,
			0X19,
			0X1C,
			0X0C,
			0X5E,
			0X18,
			0X47,
			0x09,
			0X43,
			0X44,
			0x07,
			0X45,
			},
		
		};
		
	
		
		
#elif (DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)	
		//IR customer code 0x807F
		const u8 IrKeyCode_Types[IRKC_TYPE_NUM][IrKeyId_Num]=
		{
			//IRKC_TYPE_A
			{
				0X0C,
				0X0B,
				0X09,
				0X08,
				0X0A,
				0X10,
				0X03,
				0X0D,
				0X01,
				0X12,		
			},
		
		};
		
#else
#error "IrKeyCode_Types[][] not defined!"
#endif 
	
	




	
#else	
//load IR_CODE from RES	



#include "..\..\ui\inc\ResID.h"




tIrCodeDat gIrCodeDat;




#endif	//#if (!EXT_IR_CODE)		
	
	
	
	
	
	
		
	u8* gIrKeyCodeTbl=0;
	

	
	void IrKeyCodeTbl_Init(u8 TypeSel)
	{
	
		cassert(IRKC_TYPE_END > 0);
		cassert(IRKC_TYPE_A < IRKC_TYPE_END);
	#if (!EXT_IR_CODE)	
	//#if(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)
	#if defined(IRKC_TYPE_B)
		cassert(IRKC_TYPE_B < IRKC_TYPE_END);
	#endif
		switch(TypeSel)
		{
	//#if(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)	
	#if( (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)\
		|| ((DERIVATIVE_MODEL==MODEL_TYPE_YD)&&(IR_CODE_TBL_SEL==1))\
		)
		case IRKC_TYPE_B:
			{
				gIrKeyCodeTbl=(u8*)&IrKeyCode_Types[IRKC_TYPE_B];
			}
			break;
	#endif	
		default:
			{
				gIrKeyCodeTbl=(u8*)&IrKeyCode_Types[IRKC_TYPE_A];
			}
			break;		
		}//switch
	
	
	//#if(DERIVATIVE_MODEL==MODEL_TYPE_PUSH)		
	#if( (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)||((DERIVATIVE_MODEL==MODEL_TYPE_YD)&&(IR_CODE_TBL_SEL==1))	)
		DEBUG_UI("\nIR KeyCode Table=%08X,(%08X,%08X)\n",(u32)gIrKeyCodeTbl,(u32)&IrKeyCode_Types[IRKC_TYPE_A],(u32)&IrKeyCode_Types[IRKC_TYPE_B]); 
	#else
		DEBUG_UI("\nIR KeyCode Table=%08X,(%08X,)\n",(u32)gIrKeyCodeTbl,(u32)&IrKeyCode_Types[IRKC_TYPE_A]);	
	#endif





	
	#else


		//if(IrCodeData.CustomCode)
		{//load IR_CODE from RES
		
			gIrKeyCodeTbl=(u8*)&IrCodeData.KeyCode[0];
		}


	#endif	//#if (!EXT_IR_CODE)	
	}
	
	
	
	
	u8 IrKeyCodeTbl_CodeMaptoId(u8 KeyCode) //internal used,don't export it
	{
		u8 ii=0;
	
		cassert(IrKeyId_Num > 0);
		
		////DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId(%d)?...",KeyCode);
	
		if(gIrKeyCodeTbl)
		{	
			for(ii=IrKeyId_Begin;ii<IrKeyId_Num;ii++)
			{
				if(gIrKeyCodeTbl[ii]==KeyCode)
				{
					break;
				}
			}
		}
		else
		{
			ii=IrKeyId_Num;//overrange means error
		}
		////DEBUG_GPIO("ID=%d\n",ii);
		return ii;//It didn't found the mapping Id if ii>=IrKeyId_Num
	}	











	

void IrKey_To_KeyEvent(u8 recKey)
{
//recKey:received IR key code	

	//cassert(IrKeyId_Num > 0);	

	//DEBUG_UI("received IR key:%02X\n",recKey);	
	
	switch(IrKeyCodeTbl_CodeMaptoId(recKey))
	{//map IrKeyId to UI key event

	
	#if (DERIVATIVE_MODEL==MODEL_TYPE_AFN)
	//AFN		

		////standard 9 keys for DEMO
		case IrKeyId_Play:	
			KEYIR_MSG = KEY_IRPLAY;
			DEBUG_GPIO("IR key UI_KEY_PLAY\n");
			break;
			
		case IrKeyId_Del:	
			KEYIR_MSG = KEY_IRDEL;
			DEBUG_GPIO("IR key KEY_DEL\n");
			break;
		
		case IrKeyId_Right: 
			KEYIR_MSG = KEY_IRRIGHT;
			DEBUG_GPIO("IR key KEY_RIGHT\n");
			break;
		
		case IrKeyId_Left:	
			KEYIR_MSG = KEY_IRLEFT;
			DEBUG_GPIO("IR key KEY_LEFT\n");
			break;
		
		case IrKeyId_Ok:		
			KEYIR_MSG = KEY_IROK;
			DEBUG_GPIO("IR key KEY_OK\n");
			break;
		
		case IrKeyId_Down:	
			KEYIR_MSG = KEY_IRDOWN;
			DEBUG_GPIO("IR key KEY_DOWN\n");
			break;
		
		case IrKeyId_Up:		
			KEYIR_MSG = KEY_IRUP;
			DEBUG_GPIO("IR key KEY_UP\n");
			break;
		
		case IrKeyId_Esc:		
			KEYIR_MSG = KEY_IRESC;
			DEBUG_GPIO("IR key KEY_ESC\n");
			break;
		
		case IrKeyId_Menu:	
			KEYIR_MSG = KEY_IRMENU;
			DEBUG_GPIO("IR key KEY_MENU\n");
			break;
		
		case IrKeyId_Stop:	
			KEYIR_MSG = KEY_IRSTOP;
			DEBUG_GPIO("IR key KEY_STOP\n");
			break;
			
		default:
			{
				//if(res>=IrKeyId_Num)
				//{
				//mapping error
				DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId() error\n");
				//}
			}
			break;										
	

	
	#elif (DERIVATIVE_MODEL==MODEL_TYPE_SWG)
	
		case IrKeyId_Pwr:	
			KEYIR_MSG = KEY_IRPWR;
			DEBUG_GPIO("IR key KEY_PWR\n");
			break;

		////standard 9 keys for DEMO
		case IrKeyId_Play:	
			KEYIR_MSG = KEY_IRPLAY;
			DEBUG_GPIO("IR key UI_KEY_PLAY\n");
			break;
			
		case IrKeyId_Del:	
			KEYIR_MSG = KEY_IRDEL;
			DEBUG_GPIO("IR key KEY_DEL\n");
			break;
		
		case IrKeyId_Right: 
			KEYIR_MSG = KEY_IRRIGHT;
			DEBUG_GPIO("IR key KEY_RIGHT\n");
			break;
		
		case IrKeyId_Left:	
			KEYIR_MSG = KEY_IRLEFT;
			DEBUG_GPIO("IR key KEY_LEFT\n");
			break;
		
		case IrKeyId_Ok:		
			KEYIR_MSG = KEY_IROK;
			DEBUG_GPIO("IR key KEY_OK\n");
			break;
		
		case IrKeyId_Down:	
			KEYIR_MSG = KEY_IRDOWN;
			DEBUG_GPIO("IR key KEY_DOWN\n");
			break;
		
		case IrKeyId_Up:		
			KEYIR_MSG = KEY_IRUP;
			DEBUG_GPIO("IR key KEY_UP\n");
			break;
		
		case IrKeyId_Esc:		
			KEYIR_MSG = KEY_IRESC;
			DEBUG_GPIO("IR key KEY_ESC\n");
			break;
		
		case IrKeyId_Menu:	
			KEYIR_MSG = KEY_IRMENU;
			DEBUG_GPIO("IR key KEY_MENU\n");
			break;
		
		case IrKeyId_Stop:	
			KEYIR_MSG = KEY_IRSTOP;
			DEBUG_GPIO("IR key KEY_STOP\n");
			break;
			
		default:
			{
				//if(res>=IrKeyId_Num)
				//{
				//mapping error
				DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId() error\n");
				//}
			}
			break;										

	
	#elif (DERIVATIVE_MODEL==MODEL_TYPE_YD)
	
	
		////standard 9 keys for DEMO
		case IrKeyId_Play:	
			KEYIR_MSG = KEY_IRPLAY;
			DEBUG_GPIO("IR key UI_KEY_PLAY\n");
			break;
			
		case IrKeyId_Del:	
			KEYIR_MSG = KEY_IRDEL;
			DEBUG_GPIO("IR key KEY_DEL\n");
			break;
		
		case IrKeyId_Right: 
			KEYIR_MSG = KEY_IRRIGHT;
			DEBUG_GPIO("IR key KEY_RIGHT\n");
			break;
		
		case IrKeyId_Left:	
			KEYIR_MSG = KEY_IRLEFT;
			DEBUG_GPIO("IR key KEY_LEFT\n");
			break;
		
		case IrKeyId_Ok:		
			KEYIR_MSG = KEY_IROK;
			DEBUG_GPIO("IR key KEY_OK\n");
			break;
		
		case IrKeyId_Down:	
			KEYIR_MSG = KEY_IRDOWN;
			DEBUG_GPIO("IR key KEY_DOWN\n");
			break;
		
		case IrKeyId_Up:		
			KEYIR_MSG = KEY_IRUP;
			DEBUG_GPIO("IR key KEY_UP\n");
			break;
		
		case IrKeyId_Esc:		
			KEYIR_MSG = KEY_IRESC;
			DEBUG_GPIO("IR key KEY_ESC\n");
			break;
		
		case IrKeyId_Menu:	
			KEYIR_MSG = KEY_IRMENU;
			DEBUG_GPIO("IR key KEY_MENU\n");
			break;
		
		case IrKeyId_Stop:	
			KEYIR_MSG = KEY_IRSTOP;
			DEBUG_GPIO("IR key KEY_STOP\n");
			break;
			
		default:
			{
				//if(res>=IrKeyId_Num)
				//{
				//mapping error
				DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId() error\n");
				//}
			}
			break;										

			
	#elif (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)
	
	
		case IrKeyId_Pwr:	
			KEYIR_MSG = KEY_IRPWR;
			DEBUG_GPIO("IR key KEY_PWR\n");
			break;
		

		////standard 9 keys for DEMO
		case IrKeyId_Play:	
			KEYIR_MSG = KEY_IRPLAY;
			DEBUG_GPIO("IR key UI_KEY_PLAY\n");
			break;
			
		case IrKeyId_Del:	
			KEYIR_MSG = KEY_IRDEL;
			DEBUG_GPIO("IR key KEY_DEL\n");
			break;
		
		case IrKeyId_Right: 
			KEYIR_MSG = KEY_IRRIGHT;
			DEBUG_GPIO("IR key KEY_RIGHT\n");
			break;
		
		case IrKeyId_Left:	
			KEYIR_MSG = KEY_IRLEFT;
			DEBUG_GPIO("IR key KEY_LEFT\n");
			break;
		
		case IrKeyId_Ok:		
			KEYIR_MSG = KEY_IROK;
			DEBUG_GPIO("IR key KEY_OK\n");
			break;
		
		case IrKeyId_Down:	
			KEYIR_MSG = KEY_IRDOWN;
			DEBUG_GPIO("IR key KEY_DOWN\n");
			break;
		
		case IrKeyId_Up:		
			KEYIR_MSG = KEY_IRUP;
			DEBUG_GPIO("IR key KEY_UP\n");
			break;
		
		case IrKeyId_Esc:		
			KEYIR_MSG = KEY_IRESC;
			DEBUG_GPIO("IR key KEY_ESC\n");
			break;
		
		case IrKeyId_Menu:	
			KEYIR_MSG = KEY_IRMENU;
			DEBUG_GPIO("IR key KEY_MENU\n");
			break;
		
		case IrKeyId_Stop:	
			KEYIR_MSG = KEY_IRSTOP;
			DEBUG_GPIO("IR key KEY_STOP\n");
			break;
			
		default:
			{
				//if(res>=IrKeyId_Num)
				//{
				//mapping error
				DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId() error\n");
				//}
			}
			break;										

	#elif(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
	

		////standard 9 keys for DEMO
		case IrKeyId_Play:	
			KEYIR_MSG = KEY_IRPLAY;
			DEBUG_GPIO("IR key UI_KEY_PLAY\n");
			break;
			
		case IrKeyId_Del:	
			KEYIR_MSG = KEY_IRDEL;
			DEBUG_GPIO("IR key KEY_DEL\n");
			break;
		
		case IrKeyId_Right: 
			KEYIR_MSG = KEY_IRRIGHT;
			DEBUG_GPIO("IR key KEY_RIGHT\n");
			break;
		
		case IrKeyId_Left:	
			KEYIR_MSG = KEY_IRLEFT;
			DEBUG_GPIO("IR key KEY_LEFT\n");
			break;
		
		case IrKeyId_Ok:		
			KEYIR_MSG = KEY_IROK;
			DEBUG_GPIO("IR key KEY_OK\n");
			break;
		
		case IrKeyId_Down:	
			KEYIR_MSG = KEY_IRDOWN;
			DEBUG_GPIO("IR key KEY_DOWN\n");
			break;
		
		case IrKeyId_Up:		
			KEYIR_MSG = KEY_IRUP;
			DEBUG_GPIO("IR key KEY_UP\n");
			break;
		
		case IrKeyId_Esc:		
			KEYIR_MSG = KEY_IRESC;
			DEBUG_GPIO("IR key KEY_ESC\n");
			break;
		
		case IrKeyId_Menu:	
			KEYIR_MSG = KEY_IRMENU;
			DEBUG_GPIO("IR key KEY_MENU\n");
			break;
		
		case IrKeyId_Stop:	
			KEYIR_MSG = KEY_IRSTOP;
			DEBUG_GPIO("IR key KEY_STOP\n");
			break;
			
		default:
			{
				//if(res>=IrKeyId_Num)
				//{
				//mapping error
				DEBUG_GPIO("IrKeyCodeTbl_CodeMaptoId() error\n");
				//}
			}
			break;										

	#else
	#error "IRKEYCODE_XXX to UI_KEY_XXX mapping not defined!"
	//...
	#endif 			
	}	


	/*
	if( KEYIR_MSG != KEY_None ){
		OSSemPost(uiSemEvt);
	}
	*/

}
#endif//#if(HW_BOARD_OPTION == MR6730_AFN)




	
	
	
	



#if ( (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
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
    u32 ret = 0;

#if IS_HECHI_DOORPHONE
    if (intStat & 0x00000001) {
        extern void IRCmd_feedTransferCmd(u8);

        IRCmd_feedTransferCmd(IR_DATA&0x000000FF);
        OSSemPost(uiSemEvt);
    }
    return;
#endif

    if(intStat & 0x00000001)
    {
        recKey = (IR_DATA&0x000000FF);
 	    DEBUG_GPIO("IRIntHandler key %x\n", recKey);

		#if(HW_BOARD_OPTION == MR6730_AFN)

				
			IrKey_To_KeyEvent(recKey);

			return;
				
					
					
        #elif(HW_BOARD_OPTION == MR8120_RX_DEMO_BOARD)
        //#elif((HW_BOARD_OPTION == MR8120_RX_DEMO_BOARD) || (HW_BOARD_OPTION == MR8200_RX_COMMAX))
            switch (recKey)
            {
                case 0x9c:
                    key = UI_KEY_UP;
 	                DEBUG_GPIO("IR key UI_KEY_UP\n");
                    break;

                case 0xc0:
                    key = UI_KEY_DOWN;
 	                DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                    break;

                case 0x92:
                    key = UI_KEY_RIGHT;
 	                DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                    break;

                case 0x9a:
                    key = UI_KEY_LEFT;
 	                DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                    break;

                case 0x93:
                    key = UI_KEY_ENTER;
 	                DEBUG_GPIO("IR key UI_KEY_ENTER\n");	//menu
                    break;

                case 0xc4:
                    key = UI_KEY_RF_CH;
 	                DEBUG_GPIO("IR key UI_KEY_RF_CH\n");	// IR CH
                    break;

                case 0x9d:
                    key = UI_KEY_PLAY;
 	                DEBUG_GPIO("IR key UI_KEY_PLAY\n");		// AV IN
                    break;

                case 0xc7:
                    key = UI_KEY_STOP;
 	                DEBUG_GPIO("IR key UI_KEY_STOP\n");		//AV out
                    break;

                case 0x8f:
                    key = UI_KEY_MENU;
 	                DEBUG_GPIO("IR key UI_KEY_MENU\n");		// mute
                    break;

                case 0xc3:
                    key = UI_KEY_MODE;
 	                DEBUG_GPIO("IR key UI_KEY_MODE\n");		//WIDE
                    break;

                case 0xc6:
                    key = UI_KEY_RF_QUAD;
 	                DEBUG_GPIO("IR key UI_KEY_RF_QUAD\n");	//TEMP
                    break;

                case 0x85:
                    key = UI_KEY_RF_PAIR;
 	                DEBUG_GPIO("IR key UI_KEY_RF_PAIR\n");	//COL
                    break;

                case 0xc2:
                    key = UI_KEY_RF_PAIR1;
 	                DEBUG_GPIO("IR key UI_KEY_RF_PAIR1\n");	//TINT
                    break;

                case 0xc1:
                    key = UI_KEY_VGA;
 	                DEBUG_GPIO("IR key UI_KEY_VGA\n");		//BRI
                    break;

                case 0xc5:
                    key = UI_KEY_HD;
 	                DEBUG_GPIO("IR key UI_KEY_HD\n");		//CONT
                    break;

				case 0x87:
                    key = UI_KEY_TVOUT_DET;
 	                DEBUG_GPIO("IR key UI_KEY_HD\n");		//power
                    break;
           }


        #elif ((HW_BOARD_OPTION == MR8200_RX_COMMAX) || (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX))//0x8080
            switch (recKey)
            {
                case 0x0e:
                    key = UI_KEY_UP;
 	                DEBUG_GPIO("IR key UI_KEY_UP\n");
                    break;

                case 0x0b:
                    key = UI_KEY_DOWN;
 	                DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                    break;

                case 0x1f:
                    key = UI_KEY_RIGHT;
 	                DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                    break;

                case 0x17:
                    key = UI_KEY_LEFT;
 	                DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                    break;

                case 0x07:
                    key = UI_KEY_ENTER;
 	                DEBUG_GPIO("IR key UI_KEY_ENTER\n");
                    break;

                case 0x0f:
                    key = UI_KEY_DELETE;
 	                DEBUG_GPIO("IR key UI_KEY_ALARM\n");
                    break;

                case 0x4c:  // play/stop
                    ret = uiCheckPlayback();
                    if (ret == 0)
                    {
                        key = UI_KEY_PLAY;
                        DEBUG_GPIO("IR key UI_KEY_PLAY\n");
                    }
                    else
                    {
                        key = UI_KEY_STOP;
                        DEBUG_GPIO("IR key UI_KEY_STOP\n");
                    }
 	                DEBUG_GPIO("IR key UI_KEY_PLAY or UI_KEY_STOP\n");
                    break;

                case 0x4e:  // playback mode
                    key = UI_KEY_PLAY_BACK;
 	                DEBUG_GPIO("IR key UI_KEY_PLAY\n");
                    break;

                case 0x1a:
                    key = UI_KEY_MENU;
 	                DEBUG_GPIO("IR key UI_KEY_MENU\n");
                    break;

                case 0x41:
                    key = UI_KEY_MODE;
 	                DEBUG_GPIO("IR key UI_KEY_MODE\n");
                    break;

                case 0x03:
                    key = UI_KEY_ONE;
 	                DEBUG_GPIO("IR key UI_KEY_ONE\n");
                    break;

                case 0x13:
                    key = UI_KEY_TWO;
 	                DEBUG_GPIO("IR key UI_KEY_TWO\n");
                    break;

                case 0x1B:
                    key = UI_KEY_THREE;
 	                DEBUG_GPIO("IR key UI_KEY_THREE\n");
                    break;

                case 0x04:
                    key = UI_KEY_FOUR;
 	                DEBUG_GPIO("IR key UI_KEY_FOUR\n");
                    break;

                case 0x14:
                    key = UI_KEY_FIVE;
 	                DEBUG_GPIO("IR key UI_KEY_FIVE\n");
                    break;

                case 0x1C:
                    key = UI_KEY_SIX;
 	                DEBUG_GPIO("IR key UI_KEY_SIX\n");
                    break;

                case 0x05:
                    key = UI_KEY_SEVEN;
 	                DEBUG_GPIO("IR key UI_KEY_SEVEN\n");
                    break;

                case 0x15:
                    key = UI_KEY_EIGHT;
 	                DEBUG_GPIO("IR key UI_KEY_EIGHT\n");
                    break;

                case 0x1D:
                    key = UI_KEY_NINE;
 	                DEBUG_GPIO("IR key UI_KEY_NINE\n");
                    break;

                case 0x1E:
                    key = UI_KEY_ZERO;
 	                DEBUG_GPIO("IR key UI_KEY_ZERO\n");
                    break;
            #if (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX)
				case 0x42:  // TV/Panel
                    key = UI_KEY_TVOUT_DET;
 	                DEBUG_GPIO("IR key UI_KEY_TVOUT_DET\n");		// 4
                    break;
            #endif

                case 0x10:  // enter PTZ mode
                    //key = UI_KEY_TVOUT_DET;
 	                DEBUG_GPIO("Enter PTZ mode\n");		    // 5
                    break;

                case 0x0d:  // record
                    key = UI_KEY_REC;
 	                DEBUG_GPIO("IR key UI_KEY_REC\n");		// 17
                    break;

                case 0x47:  // record stop
                    key = UI_KEY_STOP;
 	                DEBUG_GPIO("IR key UI_KEY_STOP\n");		// 15
                    break;

                case 0x4d:  /*vol +*/
                    key = UI_KEY_IR_VOL_U;
 	                DEBUG_GPIO("IR key UI_KEY_VOL_U\n");
                    break;

                case 0x09:  /*vol -*/
                    key = UI_KEY_IR_VOL_D;
 	                DEBUG_GPIO("IR key UI_KEY_VOL_D\n");
                    break;

                case 0x06:  /*RF Channel change*/
                    key = UI_KEY_IR_MODE;
 	                DEBUG_GPIO("IR key UI_KEY_RF_CH\n");
                    break;

                case 0x4b:  /*FF*/
                    key = UI_KEY_IR_FF;
 	                DEBUG_GPIO("IR key UI_KEY_IR_FF\n");
                    break;

                case 0x18:  /*FB*/
                    key = UI_KEY_IR_FB;
 	                DEBUG_GPIO("IR key UI_KEY_IR_FB\n");
                    break;
           }


        #elif((HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)||\
            (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)||\
              (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)||(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011)) //0X00FF
            switch (recKey)
            {
                case 0x40:
                    key = UI_KEY_UP;
 	                DEBUG_GPIO("IR key UI_KEY_UP\n");
                    break;

                case 0x19:
                    key = UI_KEY_DOWN;
 	                DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                    break;

                case 0x09:
                    key = UI_KEY_RIGHT;
 	                DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                    break;

                case 0x07:
                    key = UI_KEY_LEFT;
 	                DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                    break;

                case 0x15:
                    key = UI_KEY_ENTER;
 	                DEBUG_GPIO("IR key UI_KEY_ENTER\n");	//OK
                    break;

                case 0x16:
                    key = UI_KEY_MENU;
 	                DEBUG_GPIO("IR key UI_KEY_MENU\n");		//menu
                    break;

                case 0x0D:
                    key = UI_KEY_REC;
 	                DEBUG_GPIO("IR key UI_KEY_REC\n");		//REC
                    break;

                case 0x47:
                    key = UI_KEY_TVOUT_DET;
 	                DEBUG_GPIO("IR key UI_KEY_TVOUT_DET\n");//AV
                    break;

           }



        #elif ((HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_JIT) ||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||\
              (HW_BOARD_OPTION == MR8120_RX_JIT_LCD)||(HW_BOARD_OPTION == MR8120_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3)||\
              (HW_BOARD_OPTION  == MR8120_RX_JIT_M703SW4) || (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
           switch (recKey)
           {
                case 0x1a:
                    key = UI_KEY_UP;
 	                DEBUG_GPIO("IR key UI_KEY_UP\n");
                    break;

                case 0x15:
                    key = UI_KEY_DOWN;
 	                DEBUG_GPIO("IR key UI_KEY_DOWN\n");
                    break;

                case 0x0a:
                    key = UI_KEY_RIGHT;
 	                DEBUG_GPIO("IR key UI_KEY_RIGHT\n");
                    break;

                case 0x08:
                    key = UI_KEY_LEFT;
 	                DEBUG_GPIO("IR key UI_KEY_LEFT\n");
                    break;

                case 0x09:
                    key = UI_KEY_ENTER;
 	                DEBUG_GPIO("IR key UI_KEY_ENTER\n");	//menu
                    break;


                case 0x14:
                    key = UI_KEY_MENU;
 	                DEBUG_GPIO("IR key UI_KEY_MENU\n");		// mute
                    break;


                case 0x07:
                    key = UI_KEY_RF_QUAD;
 	                DEBUG_GPIO("IR key UI_KEY_RF_QUAD\n");	//TEMP
                    break;

				case 0x12:
                    key = UI_KEY_LCD_BL;
 	                DEBUG_GPIO("IR key UI_KEY_LCD_BL\n"); //LCD BL
                    break;

                case 0x0e:
                    key = UI_KEY_TVOUT_DET;
 	                DEBUG_GPIO("IR key UI_KEY_TVOUT_DET\n"); //TV out
                    break;

                case 0x18:
                    key = UI_KEY_CH1;
 	                DEBUG_GPIO("IR key UI_KEY_CH1\n");		//CH1
                    break;

                case 0x1b:
                    key = UI_KEY_CH2;
 	                DEBUG_GPIO("IR key UI_KEY_CH2\n");		//CH2
                    break;

                case 0x04:
                    key = UI_KEY_CH3;
 	                DEBUG_GPIO("IR key UI_KEY_CH3\n");		//CH3
                    break;
                case 0x05:
                    key = UI_KEY_CH4;
 	                DEBUG_GPIO("IR key UI_KEY_CH4\n");		//CH4
                    break;

                case 0x0c:
                    key = UI_KEY_REC;
 	                DEBUG_GPIO("IR key UI_KEY_REC\n");		//REC
                    break;

                case 0x16:
                    key = UI_KEY_DELETE;
 	                DEBUG_GPIO("IR key UI_KEY_DELETE\n");		//DEL
                    break;

           }
		#endif
    }
    else
	    DEBUG_GPIO("IRIntHandler Otherwise\n");

    if (key != 0xff)
    {
        if(UIKey == UI_KEY_READY || SpecialKey != UI_KEY_READY)
        {
            UIKey = key;
            DEBUG_GPIO("gpio post uiSemEvt key %d\n",UIKey);
            OSSemPost(uiSemEvt);
        }
        if (MsgKey == UI_KEY_WAIT_KEY)
        {
            MsgKey = key;
            OSMboxPost(message_MboxEvt, "gpio_key");
            DEBUG_GPIO("gpio post message_MboxEvt key %d\n",MsgKey);
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



#if(!( (HW_BOARD_OPTION == MR6730_AFN)&&(EXT_IR_CODE) ))

void hwIrInit(void)
{
    IRCtrEnable(TRUE);
    IRCtrReset(TRUE);
    IRCtrReset(FALSE);
    IRSetDiv(17);   // 216
    IRSetCustomCode(IR_CUSTOM_CODE_ID);
    IREnableInt(TRUE);
}

#else
//"EXT_IR_CODE"




//--------------------------
void hwIrInit_Ex(void* pParm)
{
//Parameters at u32 in size each

	IRCtrEnable(TRUE);
	IRCtrReset(TRUE);
	IRCtrReset(FALSE);
	if(pParm)
	{
		u32* pDat=(u32*)pParm;
		IRSetDiv(*pDat++);
		IRSetCustomCode(*pDat); 

	}
	else
	{

		IRSetDiv(17);	// 216
		IRSetCustomCode(IrCodeData.CustomCode);
	}	
	IREnableInt(TRUE);


}
//--------------------------
void hwIrInit(void)
{

	tIrCodeDat* pIrCodeDat=&gIrCodeDat;	
	ResID res=RES_DAT_IRCODE;
	u8 rtn=0;
	

	rtn=Res_load(res, pIrCodeDat);	
	if(rtn!=sizeof(tIrCodeDat))
	{
		DEBUG_GPIO("ERR:hwIrInit-1-,rtn=%d (%d)\n",rtn,sizeof(tIrCodeDat));
	}else
	{
		u8 idx=0;
		u8* pBuff=(u8*)pIrCodeDat;
		u32 CustomCode;
		
		DEBUG_GPIO("IrCodeData:\n");
		for(idx=0;idx<sizeof(tIrCodeDat);idx++){
			DEBUG_GPIO("[%d]:0x%02X\n",idx,pBuff[idx]);

			if(idx<2)
			{//CUSTOM CODE
				u8* pTmp=(u8*)&CustomCode;
				*(pTmp+idx)=*(pBuff+idx);//for little endian only
				
				if(idx==1)
				{					
					IrCodeData.CustomCode=CustomCode;
				}				
			}else
			//
			if(idx>=2)
			{//KEY CODE[]
				IrCodeData.KeyCode[(idx-2)]=*(pBuff+idx);
				//
				if(idx==(2+IrKeyId_Num-1))	break;
			}
		}
		

	#if 0//debug only
		DEBUG_GPIO("CustomCode=0x%04X\n",IrCodeData.CustomCode);
		for(idx=0;idx<IrKeyId_Num;idx++)
		{
			DEBUG_GPIO("IrKeyCode[%d]:0x%02X\n",idx,IrCodeData.KeyCode[idx]);
		}
	#endif
	}



	//--------------------------
	/*
	IRCtrEnable(TRUE);
	IRCtrReset(TRUE);
	IRCtrReset(FALSE);
	IRSetDiv(17);	// 216
	IRSetCustomCode(IrCodeData.CustomCode);
	IREnableInt(TRUE);
	*/
	hwIrInit_Ex(NULL);
	//--------------------------
}	
#endif


#endif  /*end of #if (CHIP_OPTION == CHIP_A1016A)*/

