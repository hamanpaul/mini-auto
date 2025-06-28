/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "board.h"
#include "osapi.h"
#include "task.h"
#include "ui.h"
#include "sysapi.h"
#include "uiapi.h"
#include "spiapi.h"
#include "asfapi.h"
#include "uimenu_project.h"
#include "uiKey.h"
#include "mpeg4api.h"
#include "i2capi.h"
#include "siuapi.h"
#include "timerapi.h"
#include "osd_draw_project.h"
#include "ui_project.h"
#include "dcfapi.h"
#include "rfiuapi.h"
#include "GlobalVariable.h"
#include "adcapi.h"
#if (defined(NEW_UI_ARCHITECTURE) || (HW_BOARD_OPTION == MR9670_WOAN))
    #include "MainFlow.h"
#endif
#include <mars_controller/mars_dma.h>

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#if ISFILE
#define TERMNULL        0
#define BACKSPACE       8
#define NEWLINE         10
#define ENTER           13
#endif

#define UI_P2P_RESTORT_WAIT_TIME    11
#define UI_KEY_INIT		0xfffffffe

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_STK uiTaskStack[UI_TASK_STACK_SIZE]; /* Stack of task uiTask() */
OS_STK uiSubTaskStack[UI_SUB_TASK_STACK_SIZE]; /* Stack of task uiLowTask() */
OS_EVENT* uiSemEvt;         /* semaphore to synchronize event processing */
OS_EVENT* message_MboxEvt;
OS_EVENT* uiOSDSemEvt;
OS_EVENT* uiAlarmSemEvt;    /* semaphore to synchronize event processing of alarm-out buzzer ctrl */
OS_FLAG_GRP  *gUiKeyFlagGrp;
OS_FLAG_GRP  *gUiStateFlagGrp;
OS_EVENT*   uiSetRecModeEvt;    /* set the rec mode to prevent from reserve rec clashed with manual rec */
UI_MENU_NODE uiMenuNode[UI_MENU_NODE_COUNT];
UI_MENU_NODE* uiCurrNodeListHead;
UI_MENU_NODE* uiCurrNode;
u32 UIKey = UI_KEY_INIT, SpecialKey = UI_KEY_READY, MsgKey = UI_KEY_READY;
u8 iconflag[UIACTIONNUM];
u8 start_iconflag[UIACTIONNUM];
u8 UI_CFG_RES[4] = {0};
u8 UI_TMP_RES[4] = {0};
u8 UI_P2P_RES[4] = {0};
u8 UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
u8 Start_UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
u8 UI_VGA_MaskArea[MAX_RFIU_UNIT][9];
u8 UI_HD_MaskArea[MAX_RFIU_UNIT][9];
#if ISFILE
s8  InitSettingFileName[32] = "\\Setup.ini";
#endif
u8 uiMenuEnable = 0x41;
u8 playbackflag = 2;
u8 switch_mode = 0;
u16 OSDDispWidth[]= {
    OSD_Width,
    TVOSD_SizeX,
};

u16 OSDDispHeight[]= {
    OSD_Height,
    TVOSD_SizeY,//TVOSD_SizeY,
};
u8  uiP2PMode = 0;  /*0: P2P not connect, 1: P2P connect*/
#if (UI_RX_PWRON_QUAD_ENA == 1)
u8  UISetRFMode = UI_MENU_RF_QUAD;
#else
u8  UISetRFMode = UI_MENU_RF_FULL;
#endif
u8  SD_detect_status=1;
u8  uiP2PRestoreCfgTime = 0;
u8  batteryflag = 0xff;
u8  UI_SDLastLevel;
u8  uiVersion[32]={0};
u8  uiVersionTime[9]={0};
u8  CurMotEnb[4], CurMotdayLev[4], CurMotnightLev[4];
u8  uiQuadDisplay;  /*1: quad mode, 0: single mode, 2: dual mode*/
u8 gChangResTimeout=0;
u32 prev_P2pVideoQuality=2;
u8  uiSetRfLightTimer[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY};
u8  uiSetRfLightDimmer[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY};
u8  uiSetRfLightDur[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY};
u8  uiSetRfLightState[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY};
u8  uiRetryResolition[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY,UI_SET_RF_BUSY,UI_SET_RF_BUSY,UI_SET_RF_BUSY};
u8  uiRetrySnapshot[MAX_RFIU_UNIT] = {UI_SET_RF_SNAP_INIT, UI_SET_RF_SNAP_INIT, UI_SET_RF_SNAP_INIT, UI_SET_RF_SNAP_INIT};
bool UIDEBUG_PRINT = FALSE;

#if(HW_BOARD_OPTION == MR8120_RX_HECHI)
u8 CAM_sleep = 2;
#endif

#if( (SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION  == DVP_RF_SELFTEST) )
u8  uiSetRfSensorLightState[MAX_RFIU_UNIT] = {UI_SET_RF_BUSY};
#endif

#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
u8 uiRFCHChg=0;
#endif
u8 testSensitive=0;
u8 uiEnPair2Preview=0;

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

#if(RFIU_SUPPORT)
extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];
#endif
extern OS_EVENT             *mpeg4ReadySemEvt;
extern u32                  guiIISPlayDMAId;
extern u32                  guiIISRecDMAId;
extern unsigned int         gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
extern unsigned int         gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];
extern OS_EVENT            *gRfiuSWReadySemEvt;
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
extern u8 uiIsVM9710;
#endif
#if (TUTK_SUPPORT)
extern int gOnlineNum;
extern int videoquality;

#endif

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */

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

void uiMenuBootSetInit(void)
{
    int i;

    /*avoid warning message*/
    if (i)
    {}

    #if ( (FLASH_OPTION  == FLASH_NO_DEVICE) )
        for (i = 0; i < UIACTIONNUM; i++)
            iconflag[i] = defaultvalue[i];

    #elif ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV)) //SPI flash
        Read_FB_Setting();
        Read_UI_Setting();
        smcReadAWBSetting();
    #else //NAND flash
        #if !IS_COMMAX_DOORPHONE
            sysSD_Disable();
            sysSPI_Enable();
        #endif

        Read_FB_Setting();
        spiReadRF();
        Read_UI_Setting();
        spiReadNet();
        spiReadVersion();
        
        #if !IS_COMMAX_DOORPHONE
            sysSPI_Disable();
            sysSD_Enable();
        #endif
    #endif
    uiMenuSetBootSetting();
}

void uiGetNodePhotoID(UI_NODE_DATA *node)
{
    u32 i,j;

    if(node->FileData == NULL)
        return;

#if ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    for (i = 0; i < node->FileNum; i++)
    {
        for (j = 0; j < UI_MULT_LANU_END; j++)
        {
            if (node->FileData[i].FileInfo[j].FileName == NULL)
            {
                continue;
            }
            if (node->FileData[i].FileInfo[j].bufIndex == 0)
            {
                node->FileData[i].FileInfo[j].bufIndex = spiGet_UI_FB_Index(node->FileData[i].FileInfo[j].FileName);
            }
        }
    }
#endif
}


/*

   Routine Description:

   Initialize menu tree of user interface.

   Arguments:

   None.

   Return Value:

   0 - Failure.
   1 - Success.

   */

void uiSDStateInit(void)
{
    u8 err;

    if((OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY | FLAGSYS_RDYSTAT_CARD_STATUS, OS_FLAG_WAIT_SET_ANY, &err)> 0)&& (err == OS_NO_ERR))
    {
        InitsysEvt();
        InitsysbackEvt();
        InitsysbackLowEvt();
        MemoryFullFlag = FALSE;
        dcfNewFile = 0;
        sysbackLowSetEvt(SYSBACKLOW_EVT_UI_KEY_SDCD,0,0,0,0);
        DEBUG_UI("Ui SDCD CARD_READY \n");
    }    
}


s32 uiMenuInit(void)
{
    u8 i;
    struct _UI_MENU_NODE* parent[12][UI_MENU_NODE_LIST_ITEM_MAX];
    s8 parentCurr, parentCurrIdx, parentNext, parentNextIdx;
    u32 nodeListIdx, nodeIdx, nodeHead;
    s8 depth, setidx;
    /* clear parent array */
    memset((void*)parent, 0, sizeof(parent));
    //parentCurr = 1;
    parentCurr = -1;
    parentCurrIdx = 0;
    //parentNext = 0;
    parentNext = 0;
    parentNextIdx = 0;

    /* build the tree */
    depth = -1;
    setidx = 0;
    nodeListIdx = 0;
    nodeIdx = 0;

    do
    {
        if (uiMenuNodeList[nodeListIdx].depth > depth)
        {
            //parentCurr = (parentCurr + 1) % 2;
            parentCurr++;
            parentCurrIdx = 0;
            //parentNext = (parentNext + 1) % 2;
            parentNext++;
            parentNextIdx = 0;
            depth = uiMenuNodeList[nodeListIdx].depth;
        }

        nodeHead = nodeIdx;
        for (i = 0; i < uiMenuNodeList[nodeListIdx].count; i++)
        {
            if (i == 0)
            {
                uiMenuNode[nodeIdx].left = &uiMenuNode[nodeIdx];
                uiMenuNode[nodeIdx].right = &uiMenuNode[nodeIdx];

                if (parent[parentCurr][parentCurrIdx])
                    parent[parentCurr][parentCurrIdx]->child = &uiMenuNode[nodeIdx];
            }
            else
            {
                uiMenuNode[nodeIdx].left = &uiMenuNode[nodeIdx-1];
                uiMenuNode[nodeIdx-1].right = &uiMenuNode[nodeIdx];
            }
            uiMenuNode[nodeIdx].parent = parent[parentCurr][parentCurrIdx];
            //uiMenuNode[nodeIdx].child = NULL;
            uiMenuNode[nodeIdx].item.index = i;
            uiMenuNode[nodeIdx].item.depth = uiMenuNodeList[nodeListIdx].depth;
            uiMenuNode[nodeIdx].item.name = uiMenuNodeListItem[nodeIdx].Node_Name;
            uiMenuNode[nodeIdx].item.NodeData = &uiMenuNodeListItem[nodeIdx];
            uiGetNodePhotoID(uiMenuNode[nodeIdx].item.NodeData);
            uiMenuNode[nodeIdx].item.leaf = uiMenuNodeList[nodeListIdx].leaf;
            uiMenuNode[nodeIdx].item.count=uiMenuNodeList[nodeListIdx].count;       //civic add

            if (uiMenuNodeList[nodeListIdx].leaf < 11)
            {
                uiMenuNode[nodeIdx].item.setidx = setidx++;

                parent[parentNext][parentNextIdx++] = &uiMenuNode[nodeIdx];
            }

            nodeIdx++;
        }

        if (uiMenuNodeList[nodeListIdx].count > 0)
        {
            uiMenuNode[nodeHead].left = &uiMenuNode[nodeIdx-1];
            uiMenuNode[nodeIdx-1].right = &uiMenuNode[nodeHead];
        }

      parentCurrIdx++;
      nodeListIdx++;
   } while (nodeListIdx < UI_MENU_NODE_LIST_COUNT);

#if 0  /*for debug*/
    for (i = 0; i < UI_MENU_NODE_COUNT; i++)
    {
        DEBUG_UI("\r\n***************************************************");
        DEBUG_UI("\r\n UI NODE %s", uiMenuNode[i].item.NodeData->Node_Name);
        DEBUG_UI("\r\n Left NODE %s", uiMenuNode[i].left->item.NodeData->Node_Name);
        DEBUG_UI("\r\n right NODE %s", uiMenuNode[i].right->item.NodeData->Node_Name);
        if (uiMenuNode[i].parent)
            DEBUG_UI("\r\n parent NODE %s", uiMenuNode[i].parent->item.NodeData->Node_Name);
        if (uiMenuNode[i].child)
            DEBUG_UI("\r\n child NODE %s", uiMenuNode[i].child->item.NodeData->Node_Name);
    }
#endif
   uiCurrNodeListHead = uiCurrNode = uiMenuNode;
   uiMenuEnable = 0;

   return 1;
}

void uiCheckTVInFormat(void)
{
    u8 status,cnt=0;

    /*avoid warning message*/
    if (status || cnt)
    {}

#if ( (Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT)
    if(sysVideoInSel == VIDEO_IN_TV)
    {
    #if(TV_DECODER == BIT1605) //use bit1605 tv decoder
        DEBUG_UI("check BIT1605 is locked to the video signal\n");
        i2cRead_BIT1605(0x7A,&status);  //check TVP5150 is locked to the video signal, if no video signal input, max check 100
        while(((status&0x04) != 0x04) && (cnt < 100))
        {
            cnt++;
            i2cRead_BIT1605(0x7A,&status);  //check Bitek1604 is locked to the video signal, if no video signal input, max check 100
        }
        if(cnt < 100)
        {
            DEBUG_UI("BIT1605 lock the video signal, get correctly TV in format\n");
            sysTVInFormatLocked = TRUE;
        }
        else
            DEBUG_UI("BIT1605 can't lock the video signal, use default format\n");
    #elif(TV_DECODER == TI5150) //use TI5150 tv decoder
        DEBUG_UI("check TVP5150 is locked to the video signal\n");
        i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);  //check TVP5150 is locked to the video signal, if no video signal input, max check 100
        while(((status&0x80) != 0x80) && (cnt < 100))
        {
            cnt++;
            i2cRead_TVP5150(0xc0,&status,TV_CHECK_FORMAT_RD_ADDR);
        }
        if((cnt < 100) &&(status != 0xff))
        {
            DEBUG_UI("TI5150 lock the video signal, get correctly TV in format\n");
            sysTVInFormatLocked = TRUE;
        }
        else
        {
            DEBUG_UI("TI5150 can't lock the video signal, use default TV in format-NTSC %d\n",FALSE);
            sysTVInFormatLocked = FALSE;
        }

		#if ((HW_BOARD_OPTION == MR6730_AFN) &&  MULTI_CHANNEL_SUPPORT ) 
		#if(MULTI_CH_DEGRADE_1CH)	
		//only one channel supportted
		#else
		{
			cnt=0;
			i2cRead_TVP5150(0x01,&status,I2C_TVP5150_RD_SLAV_ADDR_2);
			//DEBUG_I2C("status=0x%x \n\n",status);
			while(((status&0x80) != 0x00) && (cnt < 10))
			{
			cnt++;
			i2cRead_TVP5150(0x01,&status,I2C_TVP5150_RD_SLAV_ADDR_2);
			}
			if(cnt < 10)
			{
				DEBUG_UI("sysback TI5150_2 lock the video signal, get correctly TV in format\n");
				sysTVInFormatLocked1 = TRUE;
			}
			else
			{
				sysTVInFormatLocked1 = FALSE;
				DEBUG_UI("sysback TI5150_2 can't lock the video signal, use default TV in format-NTSC\n");   
			}

		}
		#endif 
		#endif 



		
	#elif(TV_DECODER == WT8861) //use TW9900 tv decoder
        i2cRead_WT8861(0x3A,&status,I2C_WT8861_RD_SLAV_ADDR);  //check WT8861 is locked to the video signal, if no video signal input, max check 100
	    while(((status&0x0F) != 0x0E) && (cnt < 10))
	    {
	        cnt++;
	        i2cRead_WT8861(0x3A,&status,I2C_WT8861_RD_SLAV_ADDR);
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
     #elif(TV_DECODER == TW9900) //use TW9900 tv decoder
        i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_ADDR);  //check TW9900 is locked to the video signal, if no video signal input, max check 100
        //DEBUG_I2C("status=%d \n\n",status);
        while(((status&0x80) != 0x00) && (cnt < 10))
        {
            cnt++;
            i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_ADDR);
        }
        if(cnt < 10)
        {
            DEBUG_UI("sysback TW9900_1 lock the video signal, get correctly TV in format\n");
            sysTVInFormatLocked = TRUE;
        }
        else
        {
            sysTVInFormatLocked = FALSE;
            DEBUG_UI("sysback TW9900_1 can't lock the video signal, use default TV in format-NTSC\n");
        }

		#if ((HW_BOARD_OPTION == MR6730_AFN) &&  MULTI_CHANNEL_SUPPORT ) 
		#if(MULTI_CH_DEGRADE_1CH)	
		//only one channel supportted
		#else
		{
			cnt=0;
			i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_2_ADDR);
			//DEBUG_I2C("status=0x%x \n\n",status);
			while(((status&0x80) != 0x00) && (cnt < 10))
			{
			cnt++;
			i2cRead_TW9900(0x01,&status,I2C_TW9900_RD_SLAV_2_ADDR);
			}
			if(cnt < 10)
			{
				//    DEBUG_UI("sysback TW9900_2 lock the video signal, get correctly TV in format\n");
				DEBUG_UI("sysback TW9900_2 lock the video signal, get correctly TV in format\n");
				sysTVInFormatLocked1 = TRUE;
			}
			else
			{
				sysTVInFormatLocked1 = FALSE;
				//	   DEBUG_UI("sysback TW9900_2 can't lock the video signal, use default TV in format-NTSC\n");
				DEBUG_UI("sysback TW9900_2 can't lock the video signal, use default TV in format-NTSC\n");   
  
			}

		}
		#endif 
		#endif 

    #endif


        if(sysVideoInSel == VIDEO_IN_TV)
        {
            sysTVinFormat = getTVinFormat();
            DEBUG_UI("-->sysTVinFormat=%d\n",sysTVinFormat);
        }
    }
#endif

}
void uiTask(void* pData)
{
    u8 err;

#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1, time2;
#endif

    uiWaitMainInitReady();
 #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    memset_hw_Word(uiMenuBuf3, 0x80800000, (RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2));
 #endif

    while (1)
    {
        UIKey = UI_KEY_READY;
        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_COMMAND_FINISH, OS_FLAG_SET, &err);

        #if Auto_Video_Test
            OSTimeDly(100);
            Video_Auto_Test(150, 150);
        #endif

        OSSemPend(uiSemEvt, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_UI("Error: uiSemEvt is %d.\n", err);
            continue ;
        }

        OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_COMMAND_FINISH, OS_FLAG_CLR, &err);
        //DEBUG_UI("uiTask OSSemPend key %d\r\n",UIKey);

        /*avoid to get wait key*/
        if (MsgKey != UI_KEY_READY )
        {

            if(SpecialKey != UI_KEY_READY)
            {
                UIKey = SpecialKey;
                SpecialKey = UI_KEY_READY;
                switch(UIKey)
                {
                    case UI_KEY_CAPTURE_VIDEO:
                    case UI_KEY_PWR_OFF:
                    case UI_KEY_TVOUT_DET:
                    case UI_KEY_USB_DET:
                    case UI_KEY_SDCD:
                        break;
                    default:
                        continue;
                }
            }
            MsgKey = UI_KEY_READY;
        }
        if(SpecialKey != UI_KEY_READY)
            SpecialKey = UI_KEY_READY;

#if(SHOW_UI_PROCESS_TIME == 1)
        time1=OSTimeGet();
        DEBUG_UI("UI Get Key Time =%d (x50ms)\n",time1);
#endif

    #if defined(NEW_UI_ARCHITECTURE)
        UI_step();
    #elif (HW_BOARD_OPTION==MR9670_WOAN)
        UI_step();
    #endif
        uiKeyParse(UIKey);

#if(SHOW_UI_PROCESS_TIME == 1)
        time2=OSTimeGet();
        DEBUG_UI("UI Time =%d (x50ms)\n",time2-time1);
#endif
    }
}

void uiSubTask(void* pData)
{
    u32 waitFlag=0;
    u32 setFlag=0;
    u8  err;
    u8  i;

    /*avoid warning message*/
    if (waitFlag || setFlag || err || i)
    {}

    #if(RFIU_SUPPORT == 1)

        #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
            for(i=0;i<4;i++)
            {
                setFlag|=(RFIU_TX_STA_LINK_BROKEN<<(i*8) ) ;
                setFlag|=(RFIU_TX_STA_LINK_OK<<(i*8));
                setFlag|=(RFIU_TX_STA_PAIR_OK<<(i*8));
            }
        #else
            for(i=0;i<4;i++)
            {
                setFlag|=(RFIU_RX_STA_LINK_BROKEN<<(i*8) ) ;
                setFlag|=(RFIU_RX_STA_LINK_OK<<(i*8));
                setFlag|=(RFIU_RX_STA_CHGRESO<<(i*8));
                #if ((SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) ||\
                    (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2)||\
                    (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)  )
                #else
                    setFlag|=(RFIU_RX_STA_PAIR_OK<<(i*8));
                    setFlag|=(RFIU_RX_STA_ERROR<<(i*8));
                    setFlag|=(RFIU_RX_STA_FRAMESYNC<<(i*8));
                #endif
            }
        #endif
    #endif


    while(1)
    {
        #if(RFIU_SUPPORT == 1)
        waitFlag = OSFlagPend(gRfiuStateFlagGrp, setFlag,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if(waitFlag > 0)
        {
            uiFlowRFStatus(waitFlag);
        }
        #else
            OSTimeDly(1000);
        #endif
    }
}




#if (HW_BOARD_OPTION==MR6730_AFN)	
/*UI Application extension task*/



OS_STK gTskUIAppTaskStack[UI_APP_TASK_STACK_SIZE]; /* Stack of task ui_App_Task() */



OS_FLAG_GRP  *gAppFlagGrp=0;

void ui_App_TaskInit(void)
{
	INT8U err;
#if (USE_UI_APP_TASK)
	gAppFlagGrp = OSFlagCreate(0x00000000, &err);
	if(gAppFlagGrp)
	{
	  	INT8U   Result;

        Result  = OSTaskCreate(UI_APP_TASK, UI_APP_TASK_PARAMETER, UI_APP_TASK_STACK, UI_APP_TASK_PRIORITY);
        if(Result != OS_NO_ERR)
        {
            DEBUG_UI("ui_App_TaskInit()-TaskCreate error!!!\n\r" );
        }
		
	}
	else
	{
		 DEBUG_UI("ui_App_TaskInit()-FlagCreate error!!!\n\r" );
	}
#endif //#if (USE_UI_APP_TASK)	
}//ui_App_TaskInit()




void ui_App_Task(void* pData)
{
    u32 waitFlag=0;
    u32 setFlag=0;
    u8  err;
    u8  i;

    /*avoid warning message*/
    if (waitFlag || setFlag || err || i)
    {}


DEBUG_UI("ui_App_Task() Begin\n\r" );

#if (USE_UI_APP_TASK)	
	DEBUG_UI("ui_App_Task() Running...\n\r" );

    while(1)
    {
    #if 1

	/////waitFlag = OSFlagPend(gAppFlagGrp, gFlagAppInt[i],OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);		
	//setFlag|=FLAG_APP_INT_0;//setFlag|=(0x01<<0);//bit0
	setFlag=(FLAG_APP_INT_7|FLAG_APP_INT_6|FLAG_APP_INT_5|FLAG_APP_INT_4|FLAG_APP_INT_3|FLAG_APP_INT_2|FLAG_APP_INT_1|FLAG_APP_INT_0);
	waitFlag = OSFlagPend(gAppFlagGrp, setFlag,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if(waitFlag > 0)
    {//event triggered
	    u8 ii=0;
	    u32 sftmsk=FLAG_APP_INT_0;
		#if 0
		//DEBUG_UI("ui_App_Task() setFlag:0x%08X, waitFlag:0x%08X\n\r", setFlag,waitFlag);
 		DEBUG_UI("ui_App_Task() gAppFlagGrp:0x%08X,setFlag:0x%08X, waitFlag:0x%08X\n\r",gAppFlagGrp->OSFlagFlags, setFlag,waitFlag);
		#endif
		for(ii=0;ii<FLAG_APP_INT_NUM;ii++)
		{//recursing
		switch(waitFlag&sftmsk)
		{
		case FLAG_APP_INT_0:
			{
			//DEBUG_UI("FLAG_APP_INT_0\n");
			if(gUiAppEvtOps.CB.EvtHndl_0) gUiAppEvtOps.CB.EvtHndl_0(NULL,0);
			}
		break;
		case FLAG_APP_INT_1:
			{
			//DEBUG_UI("FLAG_APP_INT_1\n");
			if(gUiAppEvtOps.CB.EvtHndl_1) gUiAppEvtOps.CB.EvtHndl_1(NULL,0);
			}
		break;
		case FLAG_APP_INT_2:
			{
			//DEBUG_UI("FLAG_APP_INT_2\n");
			if(gUiAppEvtOps.CB.EvtHndl_2) gUiAppEvtOps.CB.EvtHndl_2(NULL,0);
			}
		break;
		case FLAG_APP_INT_3:
			{
			//DEBUG_UI("FLAG_APP_INT_3\n");
			if(gUiAppEvtOps.CB.EvtHndl_3) gUiAppEvtOps.CB.EvtHndl_3(NULL,0);
			}
		break;		
		case FLAG_APP_INT_4:
			{
			//DEBUG_UI("FLAG_APP_INT_4\n");
			if(gUiAppEvtOps.CB.EvtHndl_4) gUiAppEvtOps.CB.EvtHndl_4(NULL,0);
			}
		break;
		case FLAG_APP_INT_5:
			{
			//DEBUG_UI("FLAG_APP_INT_5\n");
			if(gUiAppEvtOps.CB.EvtHndl_5) gUiAppEvtOps.CB.EvtHndl_5(NULL,0);
			}
		break;
		case FLAG_APP_INT_6:
			{
			//DEBUG_UI("FLAG_APP_INT_6\n");
			if(gUiAppEvtOps.CB.EvtHndl_6) gUiAppEvtOps.CB.EvtHndl_6(NULL,0);
			}
		break;
		case FLAG_APP_INT_7:
			{
			//DEBUG_UI("FLAG_APP_INT_7\n");
			if(gUiAppEvtOps.CB.EvtHndl_7) gUiAppEvtOps.CB.EvtHndl_7(NULL,0);
			}
		break;			
		}//switch()
		sftmsk<<=1;
    	}//for()
		
    }
    #else
        //OSTimeDly(1000);//1000ticks

		OSTimeDlyHMSM(0,0,0,999);//max 999ms //// 1sec=20ticks,1 tick 50ms
		//DEBUG_UI("ui_Ax_Task() running...\n\r" );
		{
			u32  unTimerCount = OSTimeGet();

			DEBUG_UI("APP_TEST tick:%d\r\n", unTimerCount);
        	}
		
    #endif
    }
#endif //#if (USE_UI_APP_TASK)


DEBUG_UI("ui_App_Task() End\n\r" );
}





#endif 




/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void uiEnterMenu(u8 index)
{
#if (UI_SUPPORT_TREE == 1)
    uiCurrNode = &uiMenuNode[index];
#endif
    return;
}

u8 Get_Node_Total_Index(void)
{
    u8 total_index=0,curr_index;
    UI_MENU_NODE* tempuiNode;

    tempuiNode=uiCurrNode;
    curr_index=uiCurrNode->item.index;

    while(tempuiNode->item.depth!=0)
    {
        tempuiNode=tempuiNode->parent;  // Go back to depth 0
    }

    if(tempuiNode->item.index==0)       // We are in 0,0 node
        return curr_index;

    while(tempuiNode->item.index!=0)
    {
        tempuiNode=tempuiNode->left;        //Go to left node
        tempuiNode=tempuiNode->child;
        ///if(tempuiNode->child->item.leaf!=0xFF) // Since the Voice didn't have the real icon on osd-lib
        total_index+=tempuiNode->item.count;
        tempuiNode=tempuiNode->parent;
    }

    total_index+=curr_index;

        return total_index;
}

void uiOutputRedirection(void)
{
#if(RFIU_SUPPORT == 1)
    if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR )
    {
        IDU_Init(0 , 1);

        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
        {
        #if RFRX_FULLSCR_HD_SINGLE
              iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH);
        #else
              iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
        #endif
        }
        else if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 240)
        {
              iduPlaybackMode(320,240,RF_RX_2DISP_WIDTH);
        }
        else
        {
           iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
        }

        if(sysTVOutOnFlag)
        {
	    #if(TV_DISP_BY_TV_INTR)
	        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
	        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
	    #endif
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
	}
    else if( sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA )
    {
        IDU_Init(0 , 1);
        if(sysTVOutOnFlag)
        {
            iduPlaybackMode(640,480,800);
	    #if(TV_DISP_BY_TV_INTR)
	        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
	        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
	    #endif
	    }
		else
	    {
	        iduPlaybackMode(800,480,800);
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}

    }
    else if(sysCameraMode==SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
        IDU_Init(0 , 1);
    #if UI_GRAPH_QVGA_ENABLE
            #if RFRX_HALF_MODE_SUPPORT
               if(rfiuRX_CamOnOff_Num <= 2)
               {
                  if(sysTVOutOnFlag)
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                  else
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
               }
               else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
    #else
          if( 
                (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) && 
                ( (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS) ) 
             )
          {
            #if RFRX_HALF_MODE_SUPPORT
               if(rfiuRX_CamOnOff_Num <= 2)
               {
                  if(sysTVOutOnFlag)
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                  else
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
               }
               else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
          }
          else
          {
            #if RFRX_HALF_MODE_SUPPORT
                if(rfiuRX_CamOnOff_Num <= 2)
                {
                   if(sysTVOutOnFlag)
                      iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
                   else
                      iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
                }
                else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
          }
   #endif
        if(sysTVOutOnFlag)
        {
	    #if(TV_DISP_BY_TV_INTR)
	        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
	        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control *
	    #endif
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
    }
	else
    {
	    MyHandler.MenuMode=VIDEO_MODE;
	    playbackflag = 0;
	    uiMenuEnable=0;
	    //uiMenuOSDReset();
	    uiMenuEnterPreview(0);
    }
#else
    MyHandler.MenuMode=VIDEO_MODE;
    playbackflag = 0;
    uiMenuEnable=0;
    //uiMenuOSDReset();
    uiMenuEnterPreview(0);
#endif
    uiOSDPreviewInit();
}
/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */




/*

   Routine Description:

   Initialize user interface.

   Arguments:

   None.

   Return Value:

   0 - Failure.
   1 - Success.

*/
s32 uiInit(void)
{
    u8 err;
    uiCheckTVInFormat();

    /* Initialize UI menu */
    uiMenuBootSetInit();
    #if (UI_SUPPORT_TREE == 1)
    uiMenuInit();
    #endif
    uiMenuOSDReset();

    uiMenuTreeInit();
    uiOsdIconInit();

    uiSemEvt = OSSemCreate(0);
    message_MboxEvt = OSMboxCreate(NULL);

    uiOSDSemEvt = OSSemCreate(1);
    uiAlarmSemEvt = OSSemCreate(1);
    gUiKeyFlagGrp = OSFlagCreate(0x00000000, &err);
    gUiStateFlagGrp = OSFlagCreate(0x00000000, &err);

    #if(UI_VERSION == UI_VERSION_THREE_TASK)
        MyHandler.MenuMode = VIDEO_MODE;
        OSTaskCreate(UIMOVIE_TASK, UIMOVIE_TASK_PARAMETER, UIMOVIE_TASK_STACK, UIMOVIE_TASK_PRIORITY);
    #else
        OSTaskCreate(UI_TASK, UI_TASK_PARAMETER, UI_TASK_STACK, UI_TASK_PRIORITY);
        OSTaskCreate(UI_SUB_TASK, UI_SUB_TASK_PARAMETER, UI_SUB_TASK_STACK, UI_SUB_TASK_PRIORITY);
    #endif
	
#if (HW_BOARD_OPTION==MR6730_AFN)	
	#if (USE_UI_APP_TASK)
		DEBUG_UI("ui_App_TaskInit()...\n\r" );
	#if 1
		UI_AppEventHandler_Init();
	#endif		
		ui_App_TaskInit();
		DEBUG_MAIN("mainTask::uiInit()::ui_App_TaskInit() finished\n");
	#endif //#if (USE_UI_APP_TASK)
#endif


    return 1;
}


void uiMenuEnterPreview(u8 mode)
{
#if(RFIU_SUPPORT == 0)
    //Preview Mode
    siuOpMode = SIUMODE_PREVIEW;

    sysPreviewInit(0);
    uiMenuEnable = mode;
#endif
}


s32 uiKeySnapshot(void)
{
    if (sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
        return 0;

    sysSetEvt(SYS_EVT_SNAPSHOT, 0);
    return 1;
}

s32 uiKeyVideoCapture(void)
{
    u8  err;
    u32 waitFlag;

    if (sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
        return 0;

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_REC|FLAGSYS_RDYSTAT_REC_START), OS_FLAG_WAIT_SET_ALL,&err);
    if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_REC, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
            sysSetEvt(SYS_EVT_VIDEOCAPTURE, 0);
        else
        {
            DEBUG_UI("Can't press capture key when capturing video.2\n");
            return 0;
        }
    }
    else
    {
        DEBUG_UI("Can't press capture key when capturing video return flag %d\n",waitFlag);
        return 0;
    }
    return 1;
}

/*

Routine Description:

    Set Read File Event to System Task

Arguments:

    None.

Return Value:

    0 - not rec.
    FLAGSYS_RDYSTAT_SET_REC - will start rec.
    FLAGSYS_RDYSTAT_REC_START - rec

*/
u32 uiCheckVideoRec(void)
{
    u8 err;

    return OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_REC|FLAGSYS_RDYSTAT_REC_START), OS_FLAG_WAIT_CLR_ANY, &err);
}

/*

Routine Description:

    Set Read File Event to System Task

Arguments:

    None.

Return Value:

    0 - not play.
    FLAGSYS_RDYSTAT_SET_PLAY - will start play.
    FLAGSYS_RDYSTAT_PLAY_START - play

*/
u32 uiCheckPlayback(void)
{
    u8 err;

    return OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START), OS_FLAG_WAIT_CLR_ANY, &err);
}

/*

Routine Description:

    Set Read File Event to System Task

Arguments:

    None.

Return Value:

    FALSE - Failure.

*/
BOOLEAN uiReadVideoFile(void)
{
    INT8U err;
    u32   waitFlag;

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START), OS_FLAG_WAIT_SET_ALL,&err);
    if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
        {
			sysSetEvt(SYS_EVT_ReadFile, playbackflag);
            DEBUG_UI("uiReadVideoFile\r\n");
            return TRUE;
        }
    }
    DEBUG_UI("Can not Read File %d\r\n",waitFlag);
    return FALSE;
}
/*

Routine Description:

    Set Read File Event to System Task Forward or Backward

Arguments:

    None.

Return Value:

    FALSE - Failure.

*/
BOOLEAN uiRead_Forward_VideoFile(u8 direction)
{
    INT8U err;
    u32   waitFlag;

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START), OS_FLAG_WAIT_SET_ALL,&err);
    if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
        {
            #if (HW_BOARD_OPTION == MR6730_AFN)     
            sysSetEvt(direction, 0);////playbackflag=0;//video mode 
			#else
			sysSetEvt(direction, playbackflag);
			#endif 
            DEBUG_UI("uiReadForward or Backward (%d) VideoFile\r\n",direction);			
			
            return TRUE;
        }
    }
    DEBUG_UI("Can not Read File %d\r\n",waitFlag);
    return FALSE;
}
/*

Routine Description:

    Set Playback Init Event to System Task

Arguments:

    None.

Return Value:

    FALSE - Failure.
    TRUE - Success.

*/
BOOLEAN uiPlaybackInit(s32 param, BOOLEAN wait)
{
    u8 err;

    if(wait)
    {
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        sysSetEvt(SYS_EVT_PLAYBACK_INIT, param);
    }
    else
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)> 0)
        {
            sysSetEvt(SYS_EVT_PLAYBACK_INIT, param);
            DEBUG_UI("Ui Playback Init Start\n");
        }
        else
        {
            DEBUG_UI("Ui Playback Init not Ready\n");
            return FALSE;
        }
    }
    return TRUE;
}

/*

Routine Description:

    Stop Playback

Arguments:

    ucSaveStatIdx - Save playback status into serial flash. 1 -> save, 0 -> not save.

Return Value:

    0 - Not need to stop.
    1 - Success.

*/
u8 uiPlaybackStop(u8 ucSaveStatIdx)
{
    u8 err;
    if (sysThumnailPtr->type == 0)
        return 0;
    if((OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_CLR_ANY, &err)> 0) && (err == OS_NO_ERR))
    {
        DEBUG_UI("uiPlaybackStop \r\n");
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_CLR_ANY, 200, &err);
        #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
        while (asfPlaybackVideoStop() == 0)
        #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)
        while (aviPlaybackVideoStop() == 0)
        #endif
        {
            OSTimeDly(4);
        }
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_SET_ANY, 200, &err);
        OSTimeDly(5); // Delay for protect system

        if (ucSaveStatIdx == GLB_ENA)
            Save_UI_Setting(); //Lsk 091204 : save volume setting

        return 1;
    }
    else
        DEBUG_UI("Not in playing \r\n");
    return 0;
}


#if Auto_Video_Test
void Video_Auto_Test(u32 FileNum, u32 FileTime)
{
    memset(&Video_Auto, 0, sizeof(Video_Auto));
    Video_Auto.VideoTest_FileNum = FileNum;
    Video_Auto.VideoTest_FileTime = FileTime;
    Video_Auto.VideoTest_Mode = 1;
}
#endif

void uiMenuFormatTime (u32 seconds, char *buffer)
{
    int  h, m, s, ms;

    /*aviod warning message*/
    if (ms){}
    h = seconds / 3600;
    m = (seconds - h*3600) / 60;
    s = seconds - h*3600 - m*60;

    sprintf (buffer, "%02d:%02d:%02d", h, m, s);
}

void uiDiskFreeforVideoClip(char * time_str)
{
    u32 remain_rec_sec;

    remain_rec_sec = asfRemainRecSecForDiskFreeSpace();
    uiMenuFormatTime(remain_rec_sec, time_str);
    DEBUG_UI("REC TIME: %s \n",time_str);
}

/*

Routine Description:

    Check SD Card status for usb. It includes checking SD init and checking formatting.

Arguments:

    None.

Return Value:

    0 - Not need to stop.
    1 - Success.

*/
s8 uiCheckSDCardStatForUsb(void)
{
    u8 err;
    u32 FlagResult1, FlagResult2;

    /* check SD card is under formatting or not */
    FlagResult2 = OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_WAIT_CLR_ANY, &err);
    if (FlagResult2 > 0)
    {   /* SD card is under formatting */

        /* check SD card is formatted complated or not */
        DEBUG_UI("UI: Waiting SD card Formatting Completed!\n");
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_FORMAT, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);

        return 1;
    }

    FlagResult1 = OSFlagAccept(gUiStateFlagGrp, (FLAGUI_SD_GOTO_FORMAT|FLAGUI_SD_INIT), OS_FLAG_WAIT_SET_ANY, &err);
    if(FlagResult1 & FLAGUI_SD_INIT)
    {   /* init Sd card */
        DEBUG_UI("UI: Waiting SD card Init!!\r\n");
        OSFlagPend(gUiStateFlagGrp, FLAGUI_SD_INIT, OS_FLAG_WAIT_CLR_ANY, OS_IPC_WAIT_FOREVER, &err);
        FlagResult2 = OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_WAIT_SET_ANY, &err);
        if (FlagResult2 > 0)
        {   /* enter go to format mode */
            /* Terminate waiting format command and enter usb */
            OSTimeDly(5);
            DEBUG_SYS("Terminate Format Command Waiting!!  1\r\n");
            OSMboxPost(speciall_MboxEvt, "FAILED");
        }

    }
    else if(FlagResult1 & FLAGUI_SD_GOTO_FORMAT)
    {   /* SD is under waiting command to go to format */

        /* Terminate waiting format command and enter usb */
        DEBUG_UI("Terminate Format Command Waiting!!  2\r\n");
        OSMboxPost(speciall_MboxEvt, "FAILED");
    }

    return 1;
}

void uiWaitAnyKey(void)
{
    u8 err;
    message_MboxEvt->OSEventPtr = (void *)0;
    MsgKey = UI_KEY_WAIT_KEY;
    OSMboxPend(message_MboxEvt, OS_IPC_WAIT_FOREVER, &err);
}

/*

Routine Description:

    Set Read File Event to System Task

Arguments:

    None.

Return Value:

    0 - UI busy, can not sent key.
    1 - sent key success.
    2 - UI Wait a message key, this key will not work

*/
u8 uiSentKeyToUi(u32 Key)
{
    if(UIKey == UI_KEY_READY || SpecialKey != UI_KEY_READY)
    {
        UIKey = Key;
        //if (Main_Init_Ready == 1)
        //    DEBUG_UI("post uiSemEvt key %d to UI\n",UIKey);
        OSSemPost(uiSemEvt);

        return 1;
    }
    return 0;

}


/*

Routine Description:

    Compare Time

Arguments:

    None.

Return Value:

    -1- time1 < time2.
    0 - time1 = time2.
    1 - time1 > time2.

*/
s8 uiCompareTwoTime(RTC_DATE_TIME* time1, RTC_DATE_TIME* time2)
{
    if(time1->year > time2->year)
        return 1;
    else if(time1->year < time2->year)
        return -1;

    if(time1->month > time2->month)
        return 1;
    else if(time1->month < time2->month)
        return -1;

    if(time1->day> time2->day)
        return 1;
    else if(time1->day < time2->day)
        return -1;

    if(time1->hour> time2->hour)
        return 1;
    else if(time1->hour < time2->hour)
        return -1;

    if(time1->min> time2->min)
        return 1;
    else if(time1->min < time2->min)
        return -1;

    if(time1->sec> time2->sec)
        return 1;
    else if(time1->sec < time2->sec)
        return -1;

    return 0;
}

#if ISFILE
/*

Routine Description:

    Get a line from the buffer

Arguments:

    pucAddr - address pointer of buffer position, and update the pointer to the next string
    bufferlen - the remain size in the buffer.
    LineLen - return the length in this line

Return Value:

    the address point of the current string

*/
u8* InitSet_GetLine(u8* pucAddr, u32 bufferlen, u32* LineLen)
{
u8* pucAddr1 = pucAddr;
u8 uchar;

    *LineLen = 0;

    while (*LineLen < bufferlen)
    {
        uchar = *pucAddr1;
        if( (uchar == ENTER) ||(uchar == NEWLINE))
        {
            *pucAddr1 = TERMNULL;
            uchar = *(pucAddr1+1);
            if( (uchar == ENTER) ||(uchar == NEWLINE))
            {
                *(pucAddr1+1) = TERMNULL;
                pucAddr1 += 2;
                (*LineLen)+=2;
            }
            else
            {
                pucAddr1+=1;
                (*LineLen)+=1;
            }

            //DEBUG_UI("IS Line: pucAddr1=0x%X, pucAddr=0x%X, nCodeLen=0x%X, LineLen=0x% \r\n", (u32)pucAddr1, (u32)pucAddr, bufferlen, *LineLen);
            return pucAddr1;    // OK

        }
        else
        {
            if ( (uchar>='a') && (uchar<='z') )
                (*pucAddr1) -= 0x20;

            (*LineLen)++;
            pucAddr1++;

        }

    }


    return NULL;    // Fail
}

/*

Routine Description:

    SPI update code routine.

Arguments:

    pucCodeAddr - address pointer of code position to update.
    unCodeSize - update code size.

Return Value:

    1 - success
    0 - failed

*/
s32 InitSetParse(u8* pucCodeAddr, u32 unCodeSize)
{
    u8 *pucAddr = pucCodeAddr, *pucAddr1;
    u32 nCodeLen = unCodeSize, LineLen;

    while(nCodeLen)
    {

        //DumpIS(pucAddr, nCodeLen);

        pucAddr1 = InitSet_GetLine(pucAddr, nCodeLen, &LineLen);

        //DEBUG_UI("IS: 1=0x%X, 0=0x%X, Len=0x%X, LLen=0x%X% \r\n", (u32)pucAddr1, (u32)pucAddr, nCodeLen, LineLen);

        nCodeLen -= LineLen;

        if (pucAddr1 == NULL)
        {
            DEBUG_UI("Error: Initial Setting Parse Error \n");
            return 0;
        }

        //DEBUG_UI("IS: %s \r\n", pucAddr);
        //remove the previous space/tab in this line
        while ( (*pucAddr==' ') || (*pucAddr==0x09))
            pucAddr++;

        switch(*pucAddr)
        {

            case '[':

                //DEBUG_UI("IS command line\r\n");
                uiSetFileDoCommand(pucAddr+1, &nCodeLen);
                //DEBUG_UI("IS cmd: Addr1=0x%X, Addr=0x%X, Len=0x%X\r\n", (u32)pucAddr1, (u32)pucAddr, nCodeLen);

                break;

            case ';':

                //DEBUG_UI("IS comment line\r\n");
                break;

            case ' ':

                //DEBUG_UI("IS Space line\r\n");
                break;

            case TERMNULL:
            case NEWLINE:
            case ENTER:

                //DEBUG_UI("IS ENTER line\r\n");
                break;

            default:
                DEBUG_UI("IS unknown 0x%X\r\n",(u8)*pucAddr);

        }

        pucAddr = pucAddr1;
    }

    Save_UI_Setting();

    return 1;

}


s32 InitSetProgram(u8 item)
{

    FS_FILE*    pFile;
    u32         codeSize;
    extern u8   ucNORInit;
    u8*         SettingAddr = (u8*)exifThumbnailBitstream;
    u8*         BackupAddr;
	u8	tmp;


    if(dcfStorageType != STORAGE_MEMORY_SD_MMC)
        return 0;

    if ((pFile = dcfOpen((signed char*)InitSettingFileName, "r")) == NULL)
    {
        DEBUG_UI("Error: dcf open %s error!\n", InitSettingFileName);
        return 0;
    }

    if(!dcfRead(pFile, SettingAddr, pFile->size, &codeSize))
    {
	   DEBUG_UI("Quit Initial seting!\n");
	   return 0;
    }
    DEBUG_UI("Trace: fileSize = 0x%08x, readSize = 0x%08x\n", pFile->size, codeSize);

    /* close file */
    dcfClose(pFile, &tmp);

    if (codeSize == 0)
    {
        DEBUG_UI("Initial Setting Err: setting Size is 0 Byte!\n");
        DEBUG_UI("Quit Initial seting!\n");
        return 0;
    }

    BackupAddr  = (u8*)(((u32)((SettingAddr + codeSize) + 3)) & ~3);
    memcpy(BackupAddr, SettingAddr, codeSize);

    if (InitSetParse(SettingAddr, codeSize)==0)
    {
        DEBUG_UI("Error: Initial Setting Parsing Error.\n");
        return 0;
    }

    return uiSetFileInitFinish();

}

#if 0
void DumpIS(char *pucbuf, u32 len)
{

    int val;
    u32 u32addr, u32size;
    int i;

    DEBUG_UI("Dump Adress 0x%x ,0xSize= %x\r\n", (u32)pucbuf, len);

    for( i=0 ; i<len ; i++)
    {
        if ((i%16)==0)
            DEBUG_UI("\r\n0x%08X : ",(u32)pucbuf);
        DEBUG_UI("%02X ", *pucbuf);
        pucbuf++;
    }

    DEBUG_UI("\r\n");

}

#endif

#endif

#if ERASE_SPI
/*

Routine Description:

    Stop Playback

Arguments:

    None.

Return Value:

    0 - Not need to stop.
    1 - Success.

*/
s8  uiEraseSpiWholeChip(void)
{
    u32 j;

    DEBUG_UI("\n\n\n**********Start to erase flash!\n");

#if ( (Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)|| MULTI_CHANNEL_SUPPORT )
    if(sysVideoInSel == VIDEO_IN_TV)
    {
       sysTVinFormat=getTVinFormat();
       DEBUG_UI("-->sysTVinFormat=%d\n",sysTVinFormat);
    }
#endif

    uiEnterTVMode();

    iduTVOSDDisplay(2, 0, 0, TVOSD_SizeX, TVOSD_SizeY);
    uiClearOSDBuf(2);
    uiOsdEnable(2);

    uiOSDASCIIStringByColor("Erasing Serial Flash...", 70 , 132, OSD_Blk2 , 0xC7, 0x41);

    if (spiIdentification() ==0)
    {
        uiMenuOSDFrame(TVOSD_SizeX , 240*OSD_STRING_W , 16, 50, 132, OSD_Blk2 , 0);     /* Clear OSD */
        uiOSDASCIIStringByColor("Serial Flash ID Error.", 40 , 132, OSD_Blk2 , 0xC2, 0x41);
        goto PROCESS_FAIL;
    }
    else
    {
        if (spiChipErase() == 0)
        {
            uiMenuOSDFrame(TVOSD_SizeX , 240*OSD_STRING_W , 16, 50, 132, OSD_Blk2 , 0);     /* Clear OSD */
            uiOSDASCIIStringByColor("Erasing Serial Flash Failed.", 40 , 132, OSD_Blk2 , 0xC2, 0x41);
            goto PROCESS_FAIL;
        }
        else
        {
            uiMenuOSDFrame(TVOSD_SizeX , 240*OSD_STRING_W , 16, 50, 132, OSD_Blk2 , 0);     /* Clear OSD */
            uiOSDASCIIStringByColor("Erasing Serial Flash Successfully.", 40 , 132, OSD_Blk2 , 0xC3, 0x41);
        }
    }

    for (j=0; j<0x1FFFFF; j++);     /* delay for keeping display osd message */

    uiMenuOSDFrame(TVOSD_SizeX , 240*OSD_STRING_W , 16, 50, 132, OSD_Blk2 , 0);

    DEBUG_UI("\n**********Erase flash procedure completed.\n\n\n");

    sysSPI_Disable();
    sysSD_Enable();

    return 1;

PROCESS_FAIL:


    for (j=0; j<0x1FFFFF; j++);     /* delay for keeping display osd message */
    uiMenuOSDFrame(TVOSD_SizeX , 240*OSD_STRING_W , 16, 50, 132, OSD_Blk2 , 0);
    DEBUG_UI("\n**********Erase flash procedure completed.\n\n\n");

    sysSPI_Disable();
    sysSD_Enable();

    return 0;

}
#endif

s32 uiCheckSDCD(u8 mode)       //from UI task
{
    u8 level,err;
    int status;


    //---------//
    DEBUG_UI("========== uiCheckSDCD ==========\n\r");
    dcfNewFile = 0;

    level= sysCheckSDCD();

    if(mode == 1)
        UI_SDLastLevel = level;
    else
        level = UI_SDLastLevel;

    if(level==SDC_CD_OFF)
    {//High
			#if (HW_BOARD_OPTION == MR6730_AFN)
				
			
			_APP_ENTER_CS_;
			sys_format = 0;//clear formatted flag
			_APP_EXIT_CS_;	

			
			#if (CIU_OSD_METHOD_2==0)
			#else
				#if 1	
				//
				//This special treatment is really wierd,but it works.
				//
				if(gPreviewInitDone)
				{
					if(gCOSD2bPosAutoChange)
					{
						if(COSD2b_X2==COSD2B_X2_ADJ)//if(COSD2b_Y1==COSD2B_Y1_ADJ)
						{
							UI_Adjust_COSD2bPos(0);
							//OSTimeDlyHMSM(0,0,1,0);
							DEBUG_UI("==((@@))==COSD2b_Xx restored\n");
						}
					}
				}
				#endif
			#endif

				
				DEBUG_UI("SD Remove!!( uiCheckSDCD(%d):%d )\n\r",mode,sys_format);
				
			#else
				DEBUG_UI("SD Remove!!\n\r");
			#endif 
        if(mode) // mode:1 first run on power on
        {
            sysSDCD_OFF(1);
        }
        else
        {
    	    MemoryFullFlag = FALSE;
            status = sysSetEvt(SYS_EVT_SDCD_OFF, 0);
            if(status)
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
            goto CheckSdCdAbnormal;
        }
        return 0;
    }
    else
    {  //Low
        DEBUG_UI("SD Card In!!\n\r");
        if(mode)
        {
            status=sysSDCD_IN(1);
        }
        else
        {
            status = sysSetEvt(SYS_EVT_SDCD_IN, 0);
            if(status)
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
            else
                OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_SET, &err);
        }
	
	#if (HW_BOARD_OPTION == MR6730_AFN)
		if(status)
		{
			MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_SD_RDY);	
			DEBUG_UI("uiCheckSDCD -> SD Ready\n");	
		}
		else
		{	
			MACRO_UI_SET_SYSSTAT_BIT_CLR(UI_SET_SYSSTAT_BIT_SD_RDY);
			DEBUG_UI("uiCheckSDCD -> SD not Ready\n");	
		}

		#if (CIU_OSD_METHOD_2==0)
		#else
			#if 1	
				//
				//This special treatment is really wierd,but it works.
				//
				if(gPreviewInitDone)
				{
					if(gCOSD2bPosAutoChange)
					{
						if(COSD2b_X2!=COSD2B_X2_ADJ)//if(COSD2b_Y1!=COSD2B_Y1_ADJ)
						{
							UI_Adjust_COSD2bPos(1);
							//OSTimeDlyHMSM(0,0,2,0);
							DEBUG_UI("==((@@))==COSD2b_Xx Changed\n");
						}
					}
				}
			#endif
		#endif

	#endif
	
        return 1;
    }

CheckSdCdAbnormal:
	#if (HW_BOARD_OPTION == MR6730_AFN)
	DEBUG_UI("uiCheckSDCD(),CheckSdCdAbnormal\n");
	#endif	
    if(sysPlaybackVideoStart==1)
    {
        DEBUG_UI("Abnormal Operation in playback!\n\r");
        uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        osdDrawMessage(MSG_ABNORMAL_OPERATION, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        sysPlaybackVideoStop = 1;
        sysPlaybackVideoPause=0;
        sysPlaybackVideoStart=0;
        timerCountPause(1, 0);
        OSTimeDly(10); // Delay for protect system
        //sysSetEvt(SYS_EVT_PWROFF, 0);
        sysPowerOff(0);
    }
    if (sysCaptureVideoStart == 1)
    {
        DEBUG_UI("Abnormal Operation in capture!\n\r");
        uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        osdDrawMessage(MSG_ABNORMAL_OPERATION, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        sysCaptureVideoStop = 1;
        sysCaptureVideoStart = 0;
        OSTimeDly(10); // Delay for protect system
        //sysSetEvt(SYS_EVT_PWROFF, 0);
        sysPowerOff(0);
    }
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    	UI_gotoStandbyMode();
	#endif
	
	#if (HW_BOARD_OPTION == MR6730_AFN) 
		#if 1
		if(MACRO_UI_SET_SYSSTAT_BIT_CHK(UI_SET_SYSSTAT_BIT_SD_RDY))
		{
			MACRO_UI_SET_SYSSTAT_BIT_CLR(UI_SET_SYSSTAT_BIT_SD_RDY);			
		}			
		#endif
		//
		#if (USE_IDLE_MODE)
		if(setUI.PwrCtrl_Phase==0)
		{
		    DEBUG_UI("---->UI : not initial mode\n");
	        UI_gotoPreviewMode();
		}
		else
		{
			//disable TV encoder
			UI_TVE_EN(0);
		}
		#else
	    DEBUG_UI("---->UI : not initial mode\n");
        UI_gotoPreviewMode();	     
		#endif 
	#endif		

    return 0;
}



#if (RFIU_SUPPORT)
u8 uiRfSuspendDelDecTask(u8 SelChannel)
{
    INT8U err;
    
    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    if (SelChannel >= MAX_RFIU_UNIT)
    {
        //DEBUG_UI("uiRfSuspendDelDecTask Error Channel %d \r\n",SelChannel);
        OSSemPost(gRfiuSWReadySemEvt);
        return 0;
    }
    if (gRfiu_MpegDec_Sta[SelChannel] != RFI_MPEGDEC_TASK_RUNNING)
    {
        //DEBUG_UI("uiRfSuspendDelDecTask Channel %d not Running\r\n",SelChannel);
        gRfiu_MpegDec_Sta[SelChannel] = RFI_MPEGDEC_TASK_NONE;
        OSSemPost(gRfiuSWReadySemEvt);
        return 0;
    }

    //rfiu_StopRXMpegDec(SelChannel);
    gRfiuUnitCntl[SelChannel].RX_MpegDec_Stop=1;
    OSTimeDly(5);
    switch(SelChannel)
    {
         case 0:
            OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT0);
            OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT0);
            break;

         case 1:
            OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT1);
            OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT1);
            break;

         case 2:
            OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT2);
            OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT2);
            break;

         case 3:
            OSTaskSuspend(RFIU_DEC_TASK_PRIORITY_UNIT3);
            OSTaskDel(RFIU_DEC_TASK_PRIORITY_UNIT3);
            break;
    }
    DEBUG_UI("\n-->Delete RFIU_DEC_TASK_%d\n",SelChannel);
    gRfiu_MpegDec_Sta[SelChannel] = RFI_MPEGDEC_TASK_NONE;
    gRfiuUnitCntl[SelChannel].RX_MpegDec_Stop=0;
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}

u8 uiRfCreateDecTask(u8 SelChannel)
{
    INT8U err;
    
    OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
	if (gRfiu_MpegDec_Sta[SelChannel] == RFI_MPEGDEC_TASK_NONE)
	{
        switch(SelChannel)
        {
           case 0:
              OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_0, RFIU_DEC_TASK_STACK_UNIT0, RFIU_DEC_TASK_PRIORITY_UNIT0);
              break;

           case 1:
              OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_1, RFIU_DEC_TASK_STACK_UNIT1, RFIU_DEC_TASK_PRIORITY_UNIT1);
              break;

           case 2:
              OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_2, RFIU_DEC_TASK_STACK_UNIT2, RFIU_DEC_TASK_PRIORITY_UNIT2);
              break;

           case 3:
              OSTaskCreate(RFIU_RX_DEC_TASK_UNITX, RFIU_DEC_TASK_PARAMETER_UNIT_3, RFIU_DEC_TASK_STACK_UNIT3, RFIU_DEC_TASK_PRIORITY_UNIT3);
              break;
         }
         gRfiu_MpegDec_Sta[SelChannel] = RFI_MPEGDEC_TASK_RUNNING;
         DEBUG_UI("\n-->Create RFIU_DEC_TASK_%d\n",SelChannel);
    }
	else
    {
         DEBUG_UI("\n-->Create RFIU_DEC_TASK_%d Existed ###\n",SelChannel);
    }
    OSSemPost(gRfiuSWReadySemEvt);
    return 1;
}

void uiClearFfQuadBuf(u8 index)
{
    u32  *startAddr, Width, Hight, totalW;
    u32  i,j;
    u32  *tempAddr;

    #if(UI_GRAPH_QVGA_ENABLE==0)
        Width = RF_RX_2DISP_WIDTH*2;
        Hight = RF_RX_2DISP_HEIGHT;
        totalW = Width*2;
    #else
        Width = RF_RX_2DISP_WIDTH;
        Hight = RF_RX_2DISP_HEIGHT>>1;
        totalW = Width*4;
    #endif

    switch (index)
    {
        case 0:
            startAddr = (u32*)MainVideodisplaybuf[0];
            break;

        case 1:
            startAddr = (u32*)(MainVideodisplaybuf[0]+Width);
            break;

        case 2:
            startAddr = (u32*)(MainVideodisplaybuf[0]+Hight*totalW);
            break;

        case 3:
            startAddr = (u32*)(MainVideodisplaybuf[0]+Width + Hight*totalW);
            break;

        case 4:  /* Half Mode Left*/
            startAddr = (u32*)MainVideodisplaybuf[0];
            Hight += 240;
            break;

        case 5:  /* Half Mode Right*/
            startAddr = (u32*)(MainVideodisplaybuf[0]+Width);            
            Hight += 240;
            break;
        
    }

  
    Width = Width>>2;
    for (i = 0; i < Hight; i++)
    {
        tempAddr = startAddr;
        for (j = 0; j < Width; j++)
        {
            *tempAddr = 0x80800000;
            tempAddr++;
        }
        startAddr+=(totalW>>2);
    }
}

/*

Routine Description:

    check RF talk on/off

Arguments:

    None.

Return Value:

    0: talk off
    1: talk on

*/

u8 uiCheckRfTalkStatus(void)
{
    if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
       return 1;
    else if(rfiuAudioRetStatus==RF_AUDIO_RET_OFF)
       return 0;
    else
       return 2;
}



/*

Routine Description:

    when p2p disconnection, restore RX resolution

Arguments:

    None.

Return Value:

    None.

*/


void uiCheckP2PMode(void)
{

#if UI_GRAPH_QVGA_ENABLE
    if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) && (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) )
    {
        memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);

    #if RFRX_HALF_MODE_SUPPORT
        if(rfiuRX_CamOnOff_Num <= 2)
        {
            if(sysTVOutOnFlag)
                iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
            else
                iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
        }
        else
    #endif
            iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
      
    #if TV_DISP_BY_IDU
        if(sysTVOutOnFlag) 
        {
        #if(TV_DISP_BY_TV_INTR)
            tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
            tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control * 
        #endif
        }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
    #endif
        osdDrawQuadIcon();
    }
#else
    if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) && (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) )
    {
        memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);

        if( 
           (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) && 
           ( (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS) ) 
        )
        {
        #if RFRX_HALF_MODE_SUPPORT
            if(rfiuRX_CamOnOff_Num <= 2)
            {
                if(sysTVOutOnFlag)
                    iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                else
                    iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
            }
            else
        #endif
            iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
        }
        else
        {
        #if RFRX_HALF_MODE_SUPPORT   
            if(rfiuRX_CamOnOff_Num <= 2)
            {
                if(sysTVOutOnFlag)
                    iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
                else
                    iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
            }
            else
        #endif
                iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
        }

    #if TV_DISP_BY_IDU
        if(sysTVOutOnFlag) 
    	{
    	#if(TV_DISP_BY_TV_INTR)
    	    tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
    	#else
    	    tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control * 
    	#endif
    	}
        else
    	{
    	    IduIntCtrl |= IDU_FTCINT_ENA;
        }
    #endif
        osdDrawQuadIcon();
    }

#endif        
}




/*

Routine Description:

    Setting RF talk on/off

Arguments:

    None.

Return Value:

    0: talk off
    1: talk on

*/

u8 uiSetTalkOnOff(void)
{
    if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
    {
        rfiu_AudioRetONOFF_IIS(0);
        DEBUG_GPIO("Talkint Key Off!\n\r");
        return 0;
    }
    else if(rfiuAudioRetStatus==RF_AUDIO_RET_OFF)
    {
        rfiu_AudioRetONOFF_IIS(1);
        DEBUG_GPIO("Talkint Key On!\n\r");
        return 1;
    }

    return 2; //APP 使用中.
}

u8 uiSetRfResolutionRxToTx(s8 setting,u8 camera)
{
    int RfBusy = 1, cnt = 0, RecStatus;
    u8 uartCmd[20];

    //DEBUG_UI("cam %d uiSetRfResolutionRxToTx %d:%d,%d,%d,%d\r\n",camera,setting,Main_Init_Ready,gRfiu_Op_Sta[camera],gRfiuUnitCntl[camera].TX_PicWidth,gRfiuUnitCntl[camera].TX_PicHeight);

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] == RFIU_RX_STA_LINK_BROKEN)
        return UI_SET_RF_NO_NEED;

    if (uiP2PMode == 0)
    {
        UI_CFG_RES[camera] = setting;
        Save_UI_Setting();
    }
    //UI_P2P_RES[camera] = setting;
    
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
    (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif
    //DEBUG_UI("-->DEBUG1:%d\n",setting);
    switch (setting)
    {
        case UI_RESOLTUION_VGA:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 640) && (gRfiuUnitCntl[camera].TX_PicHeight == 480))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }            
            sprintf((char*)uartCmd,"RESO 640 480");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 640;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 480;
        #endif
            break;
        case UI_RESOLTUION_HD:
            if ((gRfiuUnitCntl[camera].TX_Status & RFIU_TX_STA__HD_SUPPORT) == 0)
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }            
            if((gRfiuUnitCntl[camera].TX_PicWidth == 1280) && (gRfiuUnitCntl[camera].TX_PicHeight == 720))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }            
            sprintf((char*)uartCmd,"RESO 1280 720");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 1280;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 720;
        #endif
            break;

        case UI_RESOLTUION_QVGA:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 320) && (gRfiuUnitCntl[camera].TX_PicHeight == 240))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }            
            sprintf((char*)uartCmd,"RESO 320 240");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 320;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 240;
        #endif
            break;

        case UI_RESOLUTION_D1_480V:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 704) && (gRfiuUnitCntl[camera].TX_PicHeight == 480))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }            
            sprintf((char*)uartCmd,"RESO 704 480");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 704;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 480;
        #endif
            break;

        case UI_RESOLUTION_352x240:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 352) && (gRfiuUnitCntl[camera].TX_PicHeight == 240))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }
            sprintf((char*)uartCmd,"RESO 352 240");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 352;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 240;
        #endif
            break;

        case UI_RESOLUTION_D1_576V:
            if((gRfiuUnitCntl[camera].TX_PicWidth == 704) && (gRfiuUnitCntl[camera].TX_PicHeight == 576))
            {
                uiRetryResolition[camera] = UI_SET_RF_OK;
                return UI_SET_RF_OK;
            }
            sprintf((char*)uartCmd,"RESO 704 576");
        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = 704;
            VideoClipParameter[camera + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 576;
        #endif
            break;
    }

    /*避免x2模式下切換解析度後無法再切回x2模式,先關閉x2再去切換解析度*/
    uiSetZoomMode(camera, 0, 0 ,0);
#if MULTI_CHANNEL_VIDEO_REC
    RecStatus = MultiChannelGetCaptureVideoStatus(camera+MULTI_CHANNEL_LOCAL_MAX);
    if (RecStatus != 0)
        uiCaptureVideoStopByChannel(camera);
#endif
    cnt=0;
    RfBusy=1;
    //DEBUG_UI("-->DEBUG2:%d\n",setting);
    while(RfBusy != 0)
    {
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfResolutionRxToTx Timeout:%d !!!\n",camera);
        #if MULTI_CHANNEL_VIDEO_REC
            if (RecStatus != 0)
                uiCaptureVideoByChannel(camera);
        #endif
            uiRetryResolition[camera] = UI_SET_RF_BUSY;
            return UI_SET_RF_BUSY;
        }
        cnt++;
        OSTimeDly(1);
    }
    gChangResTimeout=20;

#if MULTI_CHANNEL_VIDEO_REC
    if (RecStatus != 0)
        uiCaptureVideoByChannel(camera);
#endif
    uiRetryResolition[camera] = UI_SET_RF_OK;
    return UI_SET_RF_OK;
}

u8 uiSetRfBrightnessRxToTx(s8 brivalue,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    //DEBUG_UI("cam %d uiSetRfBrightnessRxToTx %d day level %d night level %d\r\n",camera, brivalue, dayLev, nightLev);
    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    if (gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness == brivalue)
        return 0;

    sprintf((char*)uartCmd,"BRIT %d", brivalue);
    CurMotEnb[camera] = brivalue;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfBrightnessRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness = brivalue;
    
    return 1;
}

u8 uiSetSensorChromeRxToTx(s8 level,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    //DEBUG_UI("cam %d uiSetRfBrightnessRxToTx %d day level %d night level %d\r\n",camera, brivalue, dayLev, nightLev);
    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    if (gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness == level)
        return 0;

    sprintf((char*)uartCmd,"SATU %d", level);
    CurMotEnb[camera] = level;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetSensorChromeRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    gRfiuUnitCntl[camera].RFpara.TX_SensorBrightness = level;
    
    return 1;
}



u8 uiSetRfMotionRxToTx(s8 Enable,u8 dayLev,u8 nightLev,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (testSensitive == 1)
    {
        printf("cam %d uiSetRfMotionRxToTx %d day level %d night level %d\r\n",camera, Enable, dayLev, nightLev);
        printf("cam %d uiSetRfMotionRxToTx %d day level %d night level %d\r\n",camera, gRfiuUnitCntl[camera].RFpara.MD_en,gRfiuUnitCntl[camera].RFpara.MD_Level_Day, gRfiuUnitCntl[camera].RFpara.MD_Level_Night);
    }

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    if ((gRfiuUnitCntl[camera].RFpara.MD_en == Enable) && (gRfiuUnitCntl[camera].RFpara.MD_Level_Day == dayLev)&&
        (gRfiuUnitCntl[camera].RFpara.MD_Level_Night == nightLev))
        return 0;

    sprintf((char*)uartCmd,"MDCFG %d %d %d", Enable, dayLev, nightLev);
    CurMotEnb[camera] = Enable;
    CurMotdayLev[camera] = dayLev;
    CurMotnightLev[camera] = nightLev;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfMotionRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    gRfiuUnitCntl[camera].RFpara.MD_en = Enable;
    gRfiuUnitCntl[camera].RFpara.MD_Level_Day = dayLev;
    gRfiuUnitCntl[camera].RFpara.MD_Level_Night = nightLev;
    
    return 1;
}

u8 uiSetRfTimeRxToTx(u8 camera)
{
    u8 uartCmd[32];
    int RfBusy = 1, cnt = 0;
    RTC_DATE_TIME   localTime;
    
    //DEBUG_UI("cam %d uiSetRfBrightnessRxToTx %d day level %d night level %d\r\n",camera, brivalue, dayLev, nightLev);
    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO)|| (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    RTC_Get_Time(&localTime);
    sprintf((char*)uartCmd,"TIME %d/%d/%d %d:%d:%d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfTimeRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    
    return 1;
}




u8 uiSetRfPIRRxToTx(s8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    //DEBUG_UI("cam %d uiSetRfPIRRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
    (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    if (gRfiuUnitCntl[camera].RFpara.PIR_en == Enable)
        return 1;

    sprintf((char*)uartCmd,"PIRCFG %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfPIRRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }

    gRfiuUnitCntl[camera].RFpara.PIR_en = Enable;
    return 1;
}

u8 uiSetRfUnlockRxToTx(s8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfUnlockRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    sprintf((char*)uartCmd,"SETGPO %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfUnlockRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return 1;
}

u8 uiSetRfLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfLightingRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    Enable &= 0x01;
    sprintf((char*)uartCmd,"SETPWM %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightingRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return 1;
}

u8 uiSetRfLightDurationRxToTx(u8 Val, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfLightDurationRxToTx %d \r\n",camera, Val);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    
    Val = ((Val << 1) | 0x10);
    sprintf((char*)uartCmd,"SETPWM %d", Val);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightDurationRxToTx Timeout !!!\r\n");
            uiSetRfLightDur[camera] = RfBusy;
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightDur[camera] = RfBusy;
    return 1;
}

u8 uiSetRfLightTimerRxToTx(u8 Hour1, u8 Min1, u8 Hour2, u8 Min2, u8 Week, u8 camera, u8 isSyn)
{
    DEBUG_UI("cam %d uiSetRfLightTimerRxToTx %02d %02d %02d %02d %d %d\r\n",camera, Hour1, Min1, Hour2, Min2, Week, isSyn);

    if (Main_Init_Ready == 0)
        return 0;
    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    uiSetRfLightTimer[camera] = rfiu_SetTXSchedule(camera, Hour1, Min1, Hour2, Min2, Week, isSyn);
    return 1;
}

u8 uiSetRfLightDimmerRxToTx(u8 Val, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfLightDimmerRxToTx %d \r\n",camera, Val);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    
    Val = ((Val << 5) | 0x80);
    sprintf((char*)uartCmd,"SETPWM %d", Val);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfLightDimmerRxToTx Timeout !!!\r\n");
            uiSetRfLightDimmer[camera] = RfBusy;
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightDimmer[camera] = RfBusy;
    return 1;
}

u8 uiSetRfManualLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfManualLightingRxToTx %d \r\n",camera, Enable);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    sprintf((char*)uartCmd,"SETGPO %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfManualLightingRxToTx Timeout !!!\r\n");
            uiSetRfLightState[camera] = RfBusy;
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfLightState[camera] = RfBusy;
    return 1;

}

u8 uiSetRfNumberRxToTx(u8 Number, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;
    DEBUG_UI("cam %d uiSetRfNumberRxToTx %d \r\n",camera, Number);

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    sprintf((char*)uartCmd,"SETGPO %d", Number);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfNumberRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return 1;

}

u8 uiSetRfFlickerRxToTx(u8 setting)
{
    u8 i;
    int RfBusy = 1, cnt = 0;
    u8 uartCmd[20];  

    if (Main_Init_Ready == 0)
        return 0;


    switch (setting)
    {
        case SENSOR_AE_FLICKER_60HZ:
            sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_60HZ);
            break;

        case SENSOR_AE_FLICKER_50HZ:
            sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_50HZ);
            break;
    }

    if (Main_Init_Ready == 1)
    {
        for (i = 0; i < MAX_RFIU_UNIT; i++)
        {
            cnt=0;
            //DEBUG_UI("========================> cam%d !!!\r\n",i);
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_OFF)
                continue;
#endif
            if (gRfiu_Op_Sta[i] != RFIU_RX_STA_LINK_OK)
                continue;
            
            if (gRfiuUnitCntl[i].RFpara.TxSensorAE_FlickSetting == setting)
                continue;
            while (RfBusy != 0)
            {
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiMenuSet_TVout_Format Timeout !!!\r\n");
                    break;
                }
                RfBusy = rfiu_RXCMD_Enc(uartCmd, i);
                cnt++;
                OSTimeDly(1);
            }
            gRfiuUnitCntl[i].RFpara.TxSensorAE_FlickSetting = setting;
            RfBusy = 1;
        }
    }

    return 1;
    
}

u8 uiSetTxVolumeRxToTx(s8 level,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif
    sprintf((char*)uartCmd,"VOLUM %d", level);
    CurMotEnb[camera] = level;

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetTxVolumeRxToTx Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return 1;
}

#if RX_SNAPSHOT_SUPPORT
void uiRxSnapshot_All(int dummy1,int dummy2,int dummy3,int dummy4)
{
    int i,CH;
    int isSingleView;
    int count;
    int SnapFail[MAX_RFIU_UNIT];
    u32 oldBRI_IN_SIZE;
    u32 oldBRI_STRIDE;
    unsigned int  cpu_sr = 0;


    DEBUG_UI("==uiRxSnapshot_All:%d==\n",rfiuRX_CamOnOff_Num);
    if( (rfiuRX_CamOnOff_Num==1) && (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) )
    {
        uiRxSnapshot_One(sysRFRxInMainCHsel,0,0,0);
        return;
    }
    //--------------------------//
#if 1
    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if(gRfiu_Op_Sta[sysRFRxInMainCHsel]==RFIU_RX_STA_LINK_OK)
           memcpy_hw( uiMenuBuf3,MainVideodisplaybuf[ (rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]-1) % DISPLAY_BUF_NUM],RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT*2 );
        else
           memset_hw_Word(uiMenuBuf3, 0x80800000, (RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2));       
    }
    else
        memset_hw_Word(uiMenuBuf3, 0x80800000, (RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2));       
#endif
    
    sysEnSnapshot=1;
    isSingleView=0;

#if 1  //Lucian: save Width,Height,stide of IDU,show snapshot's Hint frame.
    oldBRI_IN_SIZE=BRI_IN_SIZE;
    oldBRI_STRIDE=BRI_STRIDE;    
    if(sysEnMenu == 1)
    {
       //BRI_IN_SIZE =(RF_RX_2DISP_HEIGHT<<16) | RF_RX_2DISP_WIDTH;
       //BRI_STRIDE = RF_RX_2DISP_WIDTH*2;
    }
    else
    {
       OS_ENTER_CRITICAL();
       if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==1280) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight==720) )
           BRI_IN_SIZE =(360<<16) | 640;
       else
           BRI_IN_SIZE =(RF_RX_2DISP_HEIGHT<<16) | RF_RX_2DISP_WIDTH;
       BRI_STRIDE = RF_RX_2DISP_WIDTH*2;
       OS_EXIT_CRITICAL();
    }
#endif    

    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        for (i = 0; i< MAX_RFIU_UNIT; i++)
        {
            if(gRfiu_Op_Sta[i] == RFIU_RX_STA_LINK_BROKEN)
               SnapFail[i]=1;
            else
               SnapFail[i]=0;
        }
    
        OS_ENTER_CRITICAL();
        for (i = 0; i< MAX_RFIU_UNIT; i++)
        {
           if(i != sysRFRxInMainCHsel)
              gRfiu_Op_Sta[i] = RFIU_RX_STA_LINK_BROKEN;
        }
        sysCameraMode=SYS_CAMERA_MODE_RF_RX_QUADSCR;
        OS_EXIT_CRITICAL();
        
        isSingleView=1;
        OSTimeDly(2);

        count=0;
        for (i = 0; i< MAX_RFIU_UNIT; i++)
        {
            if(iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                while( (gRfiu_Op_Sta[i] == RFIU_RX_STA_LINK_BROKEN) && (SnapFail[i]==0) )
                {
                  OSTimeDly(20);
                  if(count > 5)
                  {
                      DEBUG_UI("\n===Cam-%d can't wake up!!===\n",i);
                      break;
                  }
                  count ++;
                  DEBUG_UI("-->%d\n",count);
                }
             }
        }
    }
    
    CH=sysRFRxInMainCHsel;
    for (i = 0; i< MAX_RFIU_UNIT; i++)
    {
        SnapFail[CH]=0;
        if (iconflag[UI_MENU_SETIDX_CH1_ON+CH] == UI_MENU_SETTING_CAMERA_ON)
        {
            DEBUG_UI("Time to Snapshot: CH-%d\r\n",CH);
            uiRetrySnapshot[CH] = UI_SET_RF_BUSY;
            if(UI_SET_RF_OK != uiSetTxSnapshotRxToTx(UI_MENU_VIDEO_SIZE_1280X720, CH))
            {
                SnapFail[CH]=1;
                DEBUG_UI("CH-%d Snap cmd fail!!\n",CH);
            }
        }
        CH= (CH+1) % MAX_RFIU_UNIT;
    }
    //-----//
    CH=sysRFRxInMainCHsel;
    count=0;
    for (i = 0; i< MAX_RFIU_UNIT; i++)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+CH] == UI_MENU_SETTING_CAMERA_ON)
        {
            if(isSingleView)
            {
               while( (uiRetrySnapshot[CH] == UI_SET_RF_BUSY) && (SnapFail[CH]==0) )
               {
                   OSTimeDly(20);
                   count ++;
                   if(count > 20)
                   {
                       DEBUG_UI("\n*********************************\n");
                       DEBUG_UI  ("******** SNAP Fail:%d !!*********\n",CH);
                       DEBUG_UI  ("*********************************\n");
                       break;
                   }
                   DEBUG_UI("==>%d\n",count);
               }

            }        
        }
        CH= (CH+1) % MAX_RFIU_UNIT;
    }
    
    if(isSingleView)
    {
       sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;
       uiSetRfDisplayMode(UI_MENU_RF_FULL);
       if(gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_BROKEN)
          OSTimeDly(20*2);
       //uiSetRfDisplayMode(UI_MENU_RF_FULL);
    }
    else
       OSTimeDly(20*1);
    if(gRfiu_Op_Sta[sysRFRxInMainCHsel] != RFIU_RX_STA_LINK_OK)
       memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
    OS_ENTER_CRITICAL();
    if((sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysEnMenu==0) )
    {
    #if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
        {
              BRI_IN_SIZE =(360<<16) | 640;
        }
        else
              BRI_IN_SIZE =(480<<16) | 704;
        BRI_STRIDE = oldBRI_STRIDE;
    #else
       BRI_IN_SIZE =oldBRI_IN_SIZE;
       BRI_STRIDE = oldBRI_STRIDE;
    #endif
    }
    else
    {
       BRI_IN_SIZE =oldBRI_IN_SIZE;
       BRI_STRIDE = oldBRI_STRIDE;
    }
    OS_EXIT_CRITICAL();
    sysEnSnapshot=0;
    
    return;
}

void uiRxSnapshot_One(int RFUnit,int dummy1,int dummy2,int dummy3)
{
    int CH;
    int isSingleView;
    int count;
    int SnapFail;
    u32 oldBRI_IN_SIZE;
    u32 oldBRI_STRIDE;
    unsigned int  cpu_sr = 0;

    
    //--------------------------//
    DEBUG_UI("==uiRxSnapshot %d==\n",RFUnit);
#if 1
    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if(gRfiu_Op_Sta[sysRFRxInMainCHsel]==RFIU_RX_STA_LINK_OK)
           memcpy_hw( uiMenuBuf3,MainVideodisplaybuf[ (rfiuVideoBufPlay_idx[sysRFRxInMainCHsel]-1) % DISPLAY_BUF_NUM],RF_RX_2DISP_WIDTH*RF_RX_2DISP_HEIGHT*2 );
        else
           memset_hw_Word(uiMenuBuf3, 0x80800000, (RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2));       
    
    }
    else
        memset_hw_Word(uiMenuBuf3, 0x80800000, (RF_RX_2DISP_WIDTH* RF_RX_2DISP_HEIGHT * 2));

#endif
    
    isSingleView=0;
    sysEnSnapshot=1;

#if 1
    oldBRI_IN_SIZE=BRI_IN_SIZE;
    oldBRI_STRIDE=BRI_STRIDE;    
    if(sysEnMenu == 1)
    {
       //BRI_IN_SIZE =(RF_RX_2DISP_HEIGHT<<16) | RF_RX_2DISP_WIDTH;
       //BRI_STRIDE = RF_RX_2DISP_WIDTH*2;
    }
    else
    {
       OS_ENTER_CRITICAL();
       if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==1280) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight==720) )
           BRI_IN_SIZE =(360<<16) | 640;
       else
           BRI_IN_SIZE =(RF_RX_2DISP_HEIGHT<<16) | RF_RX_2DISP_WIDTH;
       
       BRI_STRIDE = RF_RX_2DISP_WIDTH*2;
       OS_EXIT_CRITICAL();
    }
#else
    oldBRI_IN_SIZE=BRI_IN_SIZE;
    oldBRI_STRIDE=BRI_STRIDE;

    BRI_IN_SIZE =(RF_RX_2DISP_HEIGHT<<16) | RF_RX_2DISP_WIDTH;
    BRI_STRIDE = RF_RX_2DISP_WIDTH*2;    
#endif

    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        if(sysRFRxInMainCHsel != RFUnit)
        {
           if(gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_BROKEN)
              SnapFail=1;
           else
              SnapFail=0;
            
           OS_ENTER_CRITICAL();
           gRfiu_Op_Sta[RFUnit] = RFIU_RX_STA_LINK_BROKEN;
           sysCameraMode=SYS_CAMERA_MODE_RF_RX_QUADSCR;
           OS_EXIT_CRITICAL();
           
           OSTimeDly(2);
           isSingleView=1;

           count=0;
           while( (gRfiu_Op_Sta[RFUnit] == RFIU_RX_STA_LINK_BROKEN) && (iconflag[UI_MENU_SETIDX_CH1_ON+RFUnit] == UI_MENU_SETTING_CAMERA_ON) && (SnapFail==0) )
           {
              OSTimeDly(20);
              if(count > 5)
              {
                  DEBUG_UI("\n===Cam-%d can't wake up!!===\n",RFUnit);
                  break;
              }
              count ++;
           }
        }
    }

    CH=RFUnit;
    if (iconflag[UI_MENU_SETIDX_CH1_ON+CH] == UI_MENU_SETTING_CAMERA_ON)
    {
        DEBUG_UI("Time to Snapshot: CH-%d\r\n",CH);
        SnapFail=0;
        uiRetrySnapshot[CH] = UI_SET_RF_BUSY;
        if( UI_SET_RF_OK != uiSetTxSnapshotRxToTx(UI_MENU_VIDEO_SIZE_1280X720, CH))
        {
           SnapFail=1;
           DEBUG_UI("CH-%d Snap cmd fail!!\n",CH);
        }
        
        if(isSingleView)
        {
           count=0;
           while( (uiRetrySnapshot[CH] ==UI_SET_RF_BUSY) && (SnapFail==0) )
           {
               OSTimeDly(20);
               count ++;
               if(count >15)
               {
                   DEBUG_UI("\n*********************************\n");
                   DEBUG_UI  ("******** SNAP Fail:%d !!*********\n",CH);
                   DEBUG_UI  ("*********************************\n");               
                   break;
               }
           }

        }
    }

    if(isSingleView)
    {
       sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR; 
       uiSetRfDisplayMode(UI_MENU_RF_FULL);
       if(gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_BROKEN)
          OSTimeDly(20*2);
    }
    else
       OSTimeDly(20*1);
    if(gRfiu_Op_Sta[sysRFRxInMainCHsel] != RFIU_RX_STA_LINK_OK) 
       memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
    OS_ENTER_CRITICAL();
    if((sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) && (sysEnMenu==1) )
    {
    #if(SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM)
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
        {
              BRI_IN_SIZE =(360<<16) | 640;
        }
        else
              BRI_IN_SIZE =(480<<16) | 704;
        BRI_STRIDE = oldBRI_STRIDE;
    #else
       BRI_IN_SIZE =oldBRI_IN_SIZE;
       BRI_STRIDE = oldBRI_STRIDE;
    #endif
    }
    else
    {
       BRI_IN_SIZE =oldBRI_IN_SIZE;
       BRI_STRIDE = oldBRI_STRIDE;
    }
    OS_EXIT_CRITICAL();
    sysEnSnapshot=0;
}
#endif

u8 uiSetTxSnapshotRxToTx(s8 videoSize,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif

    sprintf((char*)uartCmd,"SNAP %d", videoSize);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetTxSnapshotRxToTx Timeout !!!\r\n");
            uiRetrySnapshot[camera] = UI_SET_RF_BUSY;
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetTxPhotoTimeRxToTx(u8 hour,u8 min,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif

    sprintf((char*)uartCmd,"PHOTOTIME %d %d", hour,min);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetTxPhotoTimeRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetTxPushAppMsgRxToTx(u8 msgidx,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif

    sprintf((char*)uartCmd,"PUSHAPPMSG %d",msgidx);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetTxPhotoTimeRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

#if( (SW_APPLICATION_OPTION  == MR8100_DUALMODE_VBM) )
u8 uiSetTxMusicRxToTx(s8 opvalue,u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
    {
        sprintf((char*)uartCmd,"MUSIC %d", GPIO_MUSIC_STOP);
        while (RfBusy != 0)
        {
            if (cnt >UI_SET_FR_CMD_RETRY)
            {
                DEBUG_UI("uiSetTxMusicRxToTx Timeout !!!\r\n");
                return UI_SET_RF_BUSY;
            }
            RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
            cnt++;
            OSTimeDly(1);
        }
        return UI_SET_RF_NO_NEED;
    }
    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif
    sprintf((char*)uartCmd,"MUSIC %d", opvalue);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetTxMusicRxToTx Timeout !!!\r\n");
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return UI_SET_RF_OK;
}

u8 uiSetRfManualSensorLightingRxToTx(u8 Enable, u8 camera)
{
    u8 uartCmd[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return UI_SET_RF_NO_NEED;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return UI_SET_RF_NO_NEED;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return UI_SET_RF_NO_NEED;
#endif
    DEBUG_UI("cam %d uiSetRfManualSensorLightingRxToTx %d \r\n",camera, Enable);

    sprintf((char*)uartCmd,"LIGHT %d", Enable);

    while (RfBusy != 0)
    {
        if (cnt > UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfManualSensorLightingRxToTx Timeout !!!\r\n");
            uiSetRfSensorLightState[camera] = UI_SET_RF_BUSY;
            return UI_SET_RF_BUSY;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    uiSetRfSensorLightState[camera] = UI_SET_RF_OK;
    return UI_SET_RF_OK;

}
#endif

u8 uiGetP2PStatueToRF(void)
{
#if NIC_SUPPORT
    return iconflag[UI_MENU_SETIDX_P2P_LEVEL];
#else
    return 2;
#endif
}

#if NIC_SUPPORT
void uiSetP2PStatueToRF(u8  CamId, u8 level)
{
    u8  i;
    s8  SetRes;
    u8  uartCmd[16];
    int RfBusy = 1, cnt = 0;

    DEBUG_UI("uiSetP2PStatueToRF cam %d level %d\r\n",CamId, level);
    if (Main_Init_Ready == 0)
        return;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
    (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+CamId] == UI_MENU_SETTING_CAMERA_OFF)
        return;
#endif

    switch (level)
    {
        case 0: /*P2P disconnect*/
            uiP2PRestoreCfgTime = UI_P2P_RESTORT_WAIT_TIME;
            //prev_P2pVideoQuality=2;

            break;

        /*App Set level 1~5*/
        case 1:
        case 2:
        case 3:
            rfiuRX_P2pVideoQuality = level;   
            uiP2PMode = level;
            rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P;
            sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, rfiuRX_P2pVideoQuality);
            prev_P2pVideoQuality=rfiuRX_P2pVideoQuality;
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, CamId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    return;
                }
                cnt++;
                OSTimeDly(1);
            }
            iconflag[UI_MENU_SETIDX_P2P_LEVEL] = level;
            Save_UI_Setting();
            if (level == 1)
            {
            #if (UI_GRAPH_QVGA_ENABLE == 1)
                SetRes = UI_RESOLTUION_VGA;
            #else
                SetRes = UI_RESOLTUION_HD;
            #endif
            }
            else if ((level == 2)||(level == 4)) // Modified by AHER 2014/05/30
                SetRes = UI_RESOLTUION_VGA;
            else
                SetRes = UI_RESOLTUION_QVGA;
            UI_P2P_RES[CamId] = SetRes;
            uiSetRfResolutionRxToTx(SetRes, CamId);
            uiP2PRestoreCfgTime = 0;
            break;

        case 4: //Level 4 與 Level 2 解析度一樣,但frame rate 15 fps.
            rfiuRX_P2pVideoQuality = 2;   
            uiP2PMode = 2;
            rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P;
            sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, level);
            prev_P2pVideoQuality=rfiuRX_P2pVideoQuality;
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, CamId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    return;
                }
                cnt++;
                OSTimeDly(1);
            }
            iconflag[UI_MENU_SETIDX_P2P_LEVEL] = level;
            Save_UI_Setting();
            if (level == 1)
            {
            #if (UI_GRAPH_QVGA_ENABLE == 1)
                SetRes = UI_RESOLTUION_VGA;
            #else
                SetRes = UI_RESOLTUION_HD;
            #endif
            }
            else if ((level == 2)||(level == 4)) // Modified by AHER 2014/05/30
                SetRes = UI_RESOLTUION_VGA;
            else
                SetRes = UI_RESOLTUION_QVGA;
            uiSetRfResolutionRxToTx(SetRes, CamId);
            uiP2PRestoreCfgTime = 0;
            break;
            
        case 5:

            break;
            
        case 6: /*App connect in normal mode*/
            uiP2PMode = level;
            rfiuRX_OpMode |= RFIU_RX_OPMODE_P2P;
            rfiuRX_P2pVideoQuality=prev_P2pVideoQuality;

         
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, CamId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    return;
                }
                cnt++;
                OSTimeDly(1);
            }
            uiP2PRestoreCfgTime = 0;
            break;

        case 7:
            uiP2PRestoreCfgTime = 0;
            break;

        default:
            DEBUG_UI("err level %d in uiSetP2PStatueToRF\r\n", level);
            break;

    }

}
#endif
/*Type 0: HD
       1: VGA*/
u8 uiSetRfMotionMaskArea(u8 camera, u8 Type)
{
    u8  i;
    u8  uartCmd[50], tmpCmd[5];
    int RfBusy = 1, cnt = 0;
    
    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[camera] != RFIU_RX_STA_LINK_OK)
        return 0;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX)||\
     (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+camera] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    if (Type == 0)  /*HD*/
    {
        sprintf((char*)uartCmd,"MASKAREA_HD");
        for ( i = 0; i < 9; i++)
        {
            sprintf((char*)tmpCmd," %d", UI_HD_MaskArea[camera][i]);
            strcat((char*)uartCmd, (const char *)tmpCmd);
        }        
    }
    else
    {
        sprintf((char*)uartCmd,"MASKAREA_VGA");
        for ( i = 0; i < 9; i++)
        {
            sprintf((char*)tmpCmd," %d", UI_VGA_MaskArea[camera][i]);
            strcat((char*)uartCmd, (const char *)tmpCmd);
        }        
    }
    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("uiSetRfMotionMaskArea Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(uartCmd, camera);
        cnt++;
        OSTimeDly(1);
    }
    return 1;
}

u8 uiSetRfDisplayMode(u8 setting)
{
    int count;
    u8  err, i;
  

    OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_UI("uiSetRfDisplayMode %d \r\n",setting);
    UISetRFMode = setting;
    switch (setting)
    {
        case UI_MENU_RF_QUAD:
            for ( i = 0; i < 4; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;
            rfiuRX_OpMode= rfiuRX_OpMode | RFIU_RX_OPMODE_QUAD;
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_QUADSCR;
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);

          #if UI_GRAPH_QVGA_ENABLE
            
            #if RFRX_HALF_MODE_SUPPORT
               if(rfiuRX_CamOnOff_Num <= 2)
               {
                  if(sysTVOutOnFlag)
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                  else
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
               }
               else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
            
          #else
            if(  
                  (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) && 
                  ( (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS) || (rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS) ) 
              )
            {
            #if RFRX_HALF_MODE_SUPPORT
               if(rfiuRX_CamOnOff_Num <= 2)
               {
                  if(sysTVOutOnFlag)
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT_TV,RF_RX_2DISP_WIDTH*2);
                  else
                     iduPlaybackMode(RF_RX_2DISP_WIDTH,(RF_RX_2DISP_HEIGHT/2)+RFRX_HALF_MODE_SHIFT,RF_RX_2DISP_WIDTH*2);
               }
               else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
            }
            else
            {
              #if RFRX_HALF_MODE_SUPPORT
                if(rfiuRX_CamOnOff_Num <= 2)
                {
                  if(sysTVOutOnFlag)
                    iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT_TV*2,RF_RX_2DISP_WIDTH*2);
                  else
                    iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
                }
                else
              #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
            }
          #endif
          
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif   

          #if (RFIU_RX_SHOW_ONLY && ( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) ) )
            for(i=0;i<MAX_RFIU_UNIT;i++)
            {
               #if USE_704x480_RESO
               iconflag[UI_MENU_SETIDX_CH1_RES+i]=UI_MENU_SETTING_RESOLUTION_352x240;
               #else
               iconflag[UI_MENU_SETIDX_CH1_RES+i]=UI_MENU_SETTING_RESOLUTION_VGA;
               #endif
            }
          #endif

            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;

            count=0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfSuspendDelDecTask(count);
                count ++;
            }
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.         
            count=0;
            while( (gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] != RFI_WRAPDEC_TASK_RUNNING) || (gRfiu_Op_Sta[sysRFRxInMainCHsel] != RFIU_RX_STA_LINK_OK))
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
                if(count >MAX_RFIU_UNIT)
                {
                    sysRFRxInMainCHsel=0x0ff;
                    break;
                }
            }
            count   = 0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfCreateDecTask(count);
                count ++;
            }

            
            break;

        case UI_MENU_RF_DUAL:
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_DUALSCR;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif   
            rfiuRX_OpMode= rfiuRX_OpMode & (~ (RFIU_RX_OPMODE_QUAD | RFIU_RX_OPMODE_P2P));
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
            SYS_ANA_TEST2 |= 0x08; //swith TV DAC to subTV controller.
            subTV_Preview(640,480);
            //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            count   = 0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfCreateDecTask(count);
                count ++;
            }
            break;

        case UI_MENU_RF_FULL:

        #if( (SW_APPLICATION_OPTION != MR8100_BABYMONITOR) && (SW_APPLICATION_OPTION != MR8100_DUALMODE_VBM) )//Lucian: To walk arround temp screen @Quan to single.
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;
        #endif
        
            //rfiuRX_OpMode= rfiuRX_OpMode & (~RFIU_RX_OPMODE_QUAD);
            //rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
      
            for ( i = 0; i < 4; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;
        #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
            if(guiIISPlayDMAId != 0xFF)
            {
               marsDMAClose(guiIISPlayDMAId);
               guiIISPlayDMAId = 0xFF;
            }
        #endif

        #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
            if(guiIISRecDMAId != 0xFF)
            {
               marsDMAClose(guiIISRecDMAId);
               guiIISRecDMAId = 0xFF;
            }
        #endif   

            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            count=0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfSuspendDelDecTask(count);
                count ++;
            }

			rfiuRX_OpMode= rfiuRX_OpMode & (~RFIU_RX_OPMODE_QUAD);
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
       

        #if (RFIU_RX_SHOW_ONLY && ( (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) ) )
            for(i=0;i<MAX_RFIU_UNIT;i++)
            {
            #if USE_704x480_RESO
               iconflag[UI_MENU_SETIDX_CH1_RES+i]=UI_MENU_SETTING_RESOLUTION_D1_480V;
            #else
               iconflag[UI_MENU_SETIDX_CH1_RES+i]=UI_MENU_SETTING_RESOLUTION_HD;
            #endif
            }
        #endif
        

            if(sysRFRxInMainCHsel==0xff)
            {
                sysRFRxInMainCHsel=0; 
                count=0;
                while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) )
                {                  
                    sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                    count ++;
                }
            }
            
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_FULLSCR;

			#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);
				uiRfCreateDecTask(sysRFRxInMainCHsel);
		    #else
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfCreateDecTask(sysRFRxInMainCHsel);			
		    #endif
        
            break;

        case UI_MENU_RF_ENTER_PLAYBACK:
        case UI_MENU_RF_ENTER_SETUP:
            count=0;
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif   
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            for ( i = 0; i < 4; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;
            rfiuRX_OpMode= rfiuRX_OpMode & (~RFIU_RX_OPMODE_QUAD);
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            
            while(count < MAX_RFIU_UNIT)
            {
                uiRfSuspendDelDecTask(count);
                count ++;
            }
            OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                #if ((HW_BOARD_OPTION != MR8200_RX_COMMAX) && (HW_BOARD_OPTION != MR8200_RX_COMMAX_BOX))
                    memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                #endif
            #endif
            break;

       case UI_MENU_RF_ENTER_MASKAREA:
            sysCameraMode = SYS_CAMERA_MODE_RF_RX_MASKAREA;
            for ( i = 0; i < 4; i++)
                uiRFStatue[i] = UI_RF_STATUS_OTHER;

            if(sysTVOutOnFlag)
            {
                iduPlaybackMode(640,480,800);
    	    }
    		else
    	    {
    	        iduPlaybackMode(800,480,800);
    		}

            rfiuRX_OpMode= rfiuRX_OpMode & (~RFIU_RX_OPMODE_QUAD);
            rfiu_SetRXOpMode_All(rfiuRX_OpMode,rfiuRX_P2pVideoQuality);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif   

            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;

            count=0;
            while(count < MAX_RFIU_UNIT)
            {
                uiRfSuspendDelDecTask(count);
                count ++;
            }
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
            //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

            uiRfCreateDecTask(sysRFRxInMainCHsel);  
            break;

        default:
            DEBUG_UI("Error setting in  %d uiSetRfDisplayMode\r\n",setting);
            break;
    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
    return 1;
}

#if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
u8 uiSetRfChangeChannel(u8 setting)
{
    int count;
    u8 err, bRet=0;
    int CheckWifi=0;
    int i;
    unsigned int  cpu_sr = 0;

    DEBUG_UI("uiSetRfChangeChannel %d\r\n",setting);

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
            if(guiIISPlayDMAId != 0xFF)
            {
               marsDMAClose(guiIISPlayDMAId);
               guiIISPlayDMAId = 0xFF;
            }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
            if(guiIISRecDMAId != 0xFF)
            {
               marsDMAClose(guiIISRecDMAId);
               guiIISRecDMAId = 0xFF;
            }
            #endif
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
              rfiu_AudioRetONOFF_IIS(0);
        #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) )
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            uiRFCHChg=1;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
            count=0;                        
            OS_ENTER_CRITICAL();
            do
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
            OS_EXIT_CRITICAL();
            OSTimeDly(2);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            uiRFStatue[sysRFRxInMainCHsel]=UI_RF_STATUS_LINK;
            //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth=704;  
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight=480;
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
            OSSemPost(gRfiuSWReadySemEvt);
            uiRFCHChg=0;
            uiRfCreateDecTask(sysRFRxInMainCHsel);
            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        #elif(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)  
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            uiRFCHChg=1;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
            count=0;                        
            OS_ENTER_CRITICAL();
            do
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
            OS_EXIT_CRITICAL();
            OSTimeDly(2);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

            //if(uiRFStatue[sysRFRxInMainCHsel] != UI_RF_STATUS_LINK)
                //OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(sysRFRxInMainCHsel*8), OS_FLAG_SET, &err);
                
            CheckWifi=0;

            for(i=0;i<MAX_RFIU_UNIT;i++)
            {
                if( ((rfiuRX_CamOnOff_Sta >> i) & 0x01) == 1)
                {
                   if(gRfiuUnitCntl[i].RFpara.WifiLinkOn == 1)
                      CheckWifi=1;
                }
            }

            if(CheckWifi)
               memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            uiRFStatue[sysRFRxInMainCHsel]=UI_RF_STATUS_LINK;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth=704;  
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight=480;
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
            OSSemPost(gRfiuSWReadySemEvt);
            uiRFCHChg=0;
            uiRfCreateDecTask(sysRFRxInMainCHsel);
            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        #else
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

            count=0;
            do
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
               //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
               uiRfCreateDecTask(sysRFRxInMainCHsel);
            }
            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        #endif
            break;

        case UI_MENU_CHANNEL_SUB:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif            
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
        #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
            count=0;            
            OS_ENTER_CRITICAL();
            do
            {
                if (sysRFRxInMainCHsel == 0)
                    sysRFRxInMainCHsel = MAX_RFIU_UNIT;

                sysRFRxInMainCHsel = (sysRFRxInMainCHsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
            OS_EXIT_CRITICAL();

            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            //OSTimeDly(2);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
            OSSemPost(gRfiuSWReadySemEvt);

            uiRfCreateDecTask(sysRFRxInMainCHsel);
            DEBUG_UI("RF Channel decrease: %d\r\n",sysRFRxInMainCHsel);
        #else
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            count=0;
            
            do
            {
                if (sysRFRxInMainCHsel == 0)
                    sysRFRxInMainCHsel = MAX_RFIU_UNIT;

                sysRFRxInMainCHsel = (sysRFRxInMainCHsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
            }
            DEBUG_UI("RF Channel decrease: %d\r\n",sysRFRxInMainCHsel);
        #endif
            break;

        case UI_MENU_CHANNEL_1:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                //sysRFRxInMainCHsel = 0;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            //else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif            
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                
                
              #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 0;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
              #else
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 0;
              
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 1 OK\r\n");
                }
          #endif      
            }
          
            break;

        case UI_MENU_CHANNEL_2:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                //sysRFRxInMainCHsel = 1;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            //else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                         
             #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)  )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 1;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
             #else   
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 1;
             
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 2 OK\r\n");
                }
             #endif
            }
            break;

        case UI_MENU_CHANNEL_3:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                //sysRFRxInMainCHsel = 2;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            //else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                            
             #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 2;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
             #else   
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 2;
             
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 3 OK\r\n");
                }
             #endif
            }
            break;

        case UI_MENU_CHANNEL_4:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                //sysRFRxInMainCHsel = 3;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            //else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                            
              #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

                sysRFRxInMainCHsel = 3;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
              #else    
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 3;
              
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 4 OK\r\n");
                }
              #endif
            }
            break;

        case UI_MENU_CHANNEL_SCAN:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
            if(guiIISPlayDMAId != 0xFF)
            {
               marsDMAClose(guiIISPlayDMAId);
               guiIISPlayDMAId = 0xFF;
            }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
            if(guiIISRecDMAId != 0xFF)
            {
               marsDMAClose(guiIISRecDMAId);
               guiIISRecDMAId = 0xFF;
            }
            #endif            
            
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
         
            if (gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
                return 1;
            }        
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;

            count=0;
            while((gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] != RFI_WRAPDEC_TASK_RUNNING) && (count<4))
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;

            }
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel %d OK\r\n",sysRFRxInMainCHsel);
                bRet=1;
            }
            break;

    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
    return bRet;
}
#else
u8 uiSetRfChangeChannel(u8 setting)
{
    int count;
    u8 err, bRet=0;
    unsigned int  cpu_sr = 0;

    DEBUG_UI("uiSetRfChangeChannel %d\r\n",setting);

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
            if(guiIISPlayDMAId != 0xFF)
            {
               marsDMAClose(guiIISPlayDMAId);
               guiIISPlayDMAId = 0xFF;
            }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
            if(guiIISRecDMAId != 0xFF)
            {
               marsDMAClose(guiIISRecDMAId);
               guiIISRecDMAId = 0xFF;
            }
            #endif
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
              rfiu_AudioRetONOFF_IIS(0);
        #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) ) 
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            uiRFCHChg=1;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
            count=0;                        
            OS_ENTER_CRITICAL();
            do
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
            OS_EXIT_CRITICAL();
            OSTimeDly(2);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_LINK_BROKEN<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            uiRFStatue[sysRFRxInMainCHsel]=UI_RF_STATUS_LINK;
            //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth=704;  
            gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight=480;
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
            OSSemPost(gRfiuSWReadySemEvt);
            uiRFCHChg=0;
            uiRfCreateDecTask(sysRFRxInMainCHsel);
            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        #else
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

            count=0;
            do
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
               //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
               uiRfCreateDecTask(sysRFRxInMainCHsel);
            }
            DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
        #endif
            break;

        case UI_MENU_CHANNEL_SUB:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
                if(guiIISPlayDMAId != 0xFF)
                {
                   marsDMAClose(guiIISPlayDMAId);
                   guiIISPlayDMAId = 0xFF;
                }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
                if(guiIISRecDMAId != 0xFF)
                {
                   marsDMAClose(guiIISRecDMAId);
                   guiIISRecDMAId = 0xFF;
                }
            #endif            
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
        #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);

            OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
            count=0;            
            OS_ENTER_CRITICAL();
            do
            {
                if (sysRFRxInMainCHsel == 0)
                    sysRFRxInMainCHsel = MAX_RFIU_UNIT;

                sysRFRxInMainCHsel = (sysRFRxInMainCHsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
            OS_EXIT_CRITICAL();

            OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            //OSTimeDly(2);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
            OSSemPost(gRfiuSWReadySemEvt);

            uiRfCreateDecTask(sysRFRxInMainCHsel);
            DEBUG_UI("RF Channel decrease: %d\r\n",sysRFRxInMainCHsel);
        #else
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
            uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            count=0;
            
            do
            {
                if (sysRFRxInMainCHsel == 0)
                    sysRFRxInMainCHsel = MAX_RFIU_UNIT;

                sysRFRxInMainCHsel = (sysRFRxInMainCHsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> sysRFRxInMainCHsel) & 0x01) == 0) && (count<=4) );

            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
            }
            DEBUG_UI("RF Channel decrease: %d\r\n",sysRFRxInMainCHsel);
        #endif
            break;

        case UI_MENU_CHANNEL_1:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 0;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif            
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                
                
              #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 0;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
              #else
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 0;
              
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 1 OK\r\n");
                }
          #endif      
            }
          
            break;

        case UI_MENU_CHANNEL_2:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 1;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                         
             #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 1;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
             #else   
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 1;
             
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 2 OK\r\n");
                }
             #endif
            }
            break;

        case UI_MENU_CHANNEL_3:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 2;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                            
             #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                
                sysRFRxInMainCHsel = 2;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
             #else   
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 2;
             
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 3 OK\r\n");
                }
             #endif
            }
            break;

        case UI_MENU_CHANNEL_4:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                sysRFRxInMainCHsel = 3;
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
                
            }
            else
            {
                OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
                #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
                isr_marsDMA_StopAuto(guiIISPlayDMAId);
                iisStopPlay();
                    if(guiIISPlayDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISPlayDMAId);
                       guiIISPlayDMAId = 0xFF;
                    }
                #endif

                #if RFIU_RX_AUDIO_RETURN
                isr_marsDMA_StopAuto(guiIISRecDMAId);
                iisStopRec();
                    if(guiIISRecDMAId != 0xFF)
                    {
                       marsDMAClose(guiIISRecDMAId);
                       guiIISRecDMAId = 0xFF;
                    }
                #endif
                rfiuRxMainVideoPlayStart=0;
                rfiuRxMainAudioPlayStart=0;
                if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
                   rfiu_AudioRetONOFF_IIS(0);
                            
              #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;

                sysRFRxInMainCHsel = 3;
                OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(sysRFRxInMainCHsel*8), OS_FLAG_CLR, &err);

                OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                OSTimeDly(2);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_Task_Stop=0;
                OSSemPost(gRfiuSWReadySemEvt);

                uiRfCreateDecTask(sysRFRxInMainCHsel);
              #else    
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
                memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
                uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
                gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
                sysRFRxInMainCHsel = 3;
              
                if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
                {
                    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                    uiRfCreateDecTask(sysRFRxInMainCHsel);
                    DEBUG_UI("RF Channel 4 OK\r\n");
                }
              #endif
            }
            break;

        case UI_MENU_CHANNEL_SCAN:
            if (UISetRFMode != UI_MENU_RF_FULL)
            {
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            }
            OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
            isr_marsDMA_StopAuto(guiIISPlayDMAId);
            iisStopPlay();
            if(guiIISPlayDMAId != 0xFF)
            {
               marsDMAClose(guiIISPlayDMAId);
               guiIISPlayDMAId = 0xFF;
            }
            #endif

            #if RFIU_RX_AUDIO_RETURN
            isr_marsDMA_StopAuto(guiIISRecDMAId);
            iisStopRec();
            if(guiIISRecDMAId != 0xFF)
            {
               marsDMAClose(guiIISRecDMAId);
               guiIISRecDMAId = 0xFF;
            }
            #endif            
            
            rfiuRxMainVideoPlayStart=0;
            rfiuRxMainAudioPlayStart=0;
            if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
               rfiu_AudioRetONOFF_IIS(0);
         
            if (gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
                return 1;
            }        
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=1;
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); //clear display buffer.
            uiRfSuspendDelDecTask(sysRFRxInMainCHsel);
            gRfiuUnitCntl[sysRFRxInMainCHsel].RX_MpegDec_Stop=0;

            count=0;
            while((gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] != RFI_WRAPDEC_TASK_RUNNING) && (count<4))
            {
                sysRFRxInMainCHsel = (sysRFRxInMainCHsel+1) % MAX_RFIU_UNIT;
                count ++;

            }
            if(gRfiu_WrapDec_Sta[sysRFRxInMainCHsel] == RFI_WRAPDEC_TASK_RUNNING)
            {
                //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.
                uiRfCreateDecTask(sysRFRxInMainCHsel);
                DEBUG_UI("RF Channel %d OK\r\n",sysRFRxInMainCHsel);
                bRet=1;
            }
            break;

    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
    return bRet;
}
#endif

u8 uiSetRfSwAudio_DualMode(u8 setting)
{

    if(sysCameraMode != SYS_CAMERA_MODE_RF_RX_DUALSCR)
        return 0;


    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
        case UI_MENU_CHANNEL_SUB:
           break;

        case UI_MENU_CHANNEL_1:
           if(sysRFRxInMainCHsel==0)
           {
              return 0;
           }
           break;

        case UI_MENU_CHANNEL_2:
           if(sysRFRxInMainCHsel==1)
           {
              return 0;
           }
           break;

    }

    rfiu_RfSwAudio_DualMode();

    DEBUG_UI("-->Audio channel switch in dual mode:%d\n",sysRFRxInMainCHsel);
    return 1;
}

u8 uiSetRfChangeAudio_QuadMode(u8 setting)
{
    int count;
    u8 err;
    u8 Chsel;

    if (UISetRFMode != UI_MENU_RF_QUAD)
    {
        return 0;
    }

    OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_UI("uiSetRfChangeAudio at Quad mode %d\r\n",setting);

    #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
    isr_marsDMA_StopAuto(guiIISPlayDMAId);
    iisStopPlay();
        if(guiIISPlayDMAId != 0xFF)
        {
           marsDMAClose(guiIISPlayDMAId);
           guiIISPlayDMAId = 0xFF;
        }
    #endif

    #if RFIU_RX_AUDIO_RETURN
    isr_marsDMA_StopAuto(guiIISRecDMAId);
    iisStopRec();
        if(guiIISRecDMAId != 0xFF)
        {
           marsDMAClose(guiIISRecDMAId);
           guiIISRecDMAId = 0xFF;
        }
    #endif   
    rfiuRxMainAudioPlayStart=0;
    rfiuRxMainVideoPlayStart=0;
    if(rfiuAudioRetStatus==RF_AUDIO_RET_RX_USE)
       rfiu_AudioRetONOFF_IIS(0);

    count=0;
    while(count < MAX_RFIU_UNIT)
    {
        uiRfSuspendDelDecTask(count);
        count ++;
    }
    //OSSemSet(mpeg4ReadySemEvt, 1, &err); //Lucian:重新設定 mpeg4 resource semephore.

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            count=0;
            Chsel=sysRFRxInMainCHsel;
            do
            {
                Chsel = (Chsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=4) );
            sysRFRxInMainCHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_SUB:
            count=0;
            Chsel=sysRFRxInMainCHsel;
            do
            {
                Chsel = (Chsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=4) );
            sysRFRxInMainCHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_1:
            sysRFRxInMainCHsel = 0;
            break;

        case UI_MENU_CHANNEL_2:
            sysRFRxInMainCHsel = 1;
            break;

        case UI_MENU_CHANNEL_3:
            sysRFRxInMainCHsel = 2;
            break;

        case UI_MENU_CHANNEL_4:
            sysRFRxInMainCHsel = 3;
            break;

        case UI_MENU_CHANNEL_SCAN:

            break;
    }

    count   = 0;
    while(count < MAX_RFIU_UNIT)
    {
        uiRfCreateDecTask(count);
        count ++;
    }
    DEBUG_UI("RF Channel increase: %d\r\n",sysRFRxInMainCHsel);
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);

    return 0;
}

u8 uiSetRfChgPTZ_CH(u8 setting)
{
    u8 Chsel;
    int count;
    //==================//

    switch (setting)
    {
        case UI_MENU_CHANNEL_ADD:
            count=0;
            Chsel=sysRF_PTZ_CHsel;
            do
            {
                Chsel = (Chsel+1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=4) );
            sysRF_PTZ_CHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_SUB:
            count=0;
            Chsel=sysRF_PTZ_CHsel;
            do
            {
                Chsel = (Chsel-1) % MAX_RFIU_UNIT;
                count ++;
            }while( ( ((rfiuRX_CamOnOff_Sta >> Chsel) & 0x01) == 0) && (count<=4) );
            sysRF_PTZ_CHsel=Chsel;
            break;

        case UI_MENU_CHANNEL_1:
            sysRF_PTZ_CHsel = 0;
            break;

        case UI_MENU_CHANNEL_2:
            sysRF_PTZ_CHsel = 1;
            break;

        case UI_MENU_CHANNEL_3:
            sysRF_PTZ_CHsel = 2;
            break;

        case UI_MENU_CHANNEL_4:
            sysRF_PTZ_CHsel = 3;
            break;
    }
    return 1;
}

u8  uiSynRFConfigInP2P(u8 camId)
{
    uiSetRfResolutionRxToTx(UI_P2P_RES[camId], camId);
    return 1;
}

void ui_CamSleep(u8 RFUnit)
{
   int RfBusy;
   int cnt;
   
   DEBUG_UI("-----Set Camera-%d Sleep command----\n",RFUnit); 
   if( rfiuRX_CamOnOff_Sta & (0x01<<RFUnit) )
   {
       cnt=0;
       RfBusy=1;
       while(RfBusy != 0)
       {
       //sprintf(cmd,"SLEEP");
          RfBusy=rfiu_RXCMD_Enc("SLEEP",RFUnit);
          if (cnt >4)
          {
              DEBUG_UI("rfiuCamSleepCmd Timeout:%d!!!\n",RFUnit);
              return ;
          }
          cnt++;
          OSTimeDly(1);
       }  
   }
 #if (HW_BOARD_OPTION == MR8120_RX_HECHI)
   CAM_sleep=1;
 #endif
}

#endif

u32 uiGetVolumeLevel(void)
{
    return iconflag[UI_MENU_SETIDX_VOLUME];
}


u32 uiGetMenuMode(void)
{
    return MyHandler.MenuMode;
}

bool uiCompareNowIsMatchThisMode(u8 isTheMode)
{
    return (MyHandler.MenuMode == isTheMode);
}

#if ( ( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) ) && (USE_704x480_RESO==0) )
u32 uiSetZoomMode(u8 Channel, u32 ZoomX, u32 ZoomY, u8 act)
{

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[Channel] != RFIU_RX_STA_LINK_OK)
        return 0;   

    if(act)
    {
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
        OSTimeDly(1);
        sysEnZoom=1;
    #if RFRX_FULLSCR_HD_SINGLE
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight >= 720)
           iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH*2);
        else
           iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
    #else
        iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
    #endif
        IduIntCtrl |= IDU_FTCINT_ENA;
        OSTimeDly(3);
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
    }
    else
    {
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
        OSTimeDly(1);
        sysEnZoom=0;
    #if RFRX_FULLSCR_HD_SINGLE
        if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight >= 720)
           iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH);
        else
           iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    #else
        iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    #endif
        IduIntCtrl |= IDU_FTCINT_ENA;  
        OSTimeDly(5);
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
    }

    return 0;
}

#else
u32 uiSetZoomMode(u8 Channel, u32 ZoomX, u32 ZoomY, u8 act)
{
#if RFIU_SUPPORT
    u8 setcmdstr[20];
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 0)
        return 0;

    if (gRfiu_Op_Sta[Channel] != RFIU_RX_STA_LINK_OK)
        return 0;   

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_ST_2) || (UI_VERSION == UI_VERSION_COMMAX) ||\
    (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (iconflag[UI_MENU_SETIDX_CH1_ON+Channel] == UI_MENU_SETTING_CAMERA_OFF)
        return 0;
#endif

    if (act == 1)
    {
        DEBUG_UI2("UI Set Zoom to size cam%d %d, %d !!!\r\n",Channel, ZoomX, ZoomY);
    }
    else
    {
        DEBUG_UI2("UI Set CAM%d Zoom Off !!!\r\n",Channel);
    }

    sprintf((char *)setcmdstr,"ZOOM %d %d %d",act, ZoomX, ZoomY);

    while (RfBusy != 0)
    {
        if (cnt >UI_SET_FR_CMD_RETRY)
        {
            DEBUG_UI("UI Set Zoom to RF Timeout !!!\r\n");
            return 0;
        }
        RfBusy = rfiu_RXCMD_Enc(setcmdstr, Channel);
        cnt++;
        OSTimeDly(1);
    }
    return RfBusy;
#else
    DEBUG_UI("UI Set Zoom not ready !!!\r\n");
    return 0;
#endif

}
#endif


#if NIC_SUPPORT
void uiSetP2PImageLevel(u8 CamId, u8 level)
{
#if (RFIU_SUPPORT)
    uiSetP2PStatueToRF(CamId, level);
#endif
#if((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (level == 0)
        osdDrawRemoteOn(UI_OSD_CLEAR);
    else
        osdDrawRemoteOn(UI_OSD_DRAW);
#endif
}


u8 uiSetP2PPassword(u8 *password)
{
    DEBUG_UI("uiSetP2PPassword %s\r\n",password);
    memcpy(UI_P2P_PSW, password, sizeof(UI_P2P_PSW));
    LoadP2PPassword(password);
    if (Main_Init_Ready == 1)
        Save_UI_Setting();
}

#endif

/* ADC level= 0-9 ;0: mute  9: max*/
void uiSetAudioByCH(u8 channel, u8 level)
{
    u32 volVal[10] = {31, 28, 25, 22, 19, 16, 12, 8, 4, 0};
    switch(channel)
    {
        case UI_MENU_CHANNEL_1:
            adcSetDAC_R_CH_Gain(volVal[level]);
            break;

        case UI_MENU_CHANNEL_2:
            adcSetDAC_L_CH_Gain(volVal[level]);
            break;

    }
}


/***********************************************
Routine Description:
	Readback UI setting from NAND/SPI.

Arguments:
	None.

Return Value:
	None
************************************************/
void Read_UI_Setting(void)
{
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

    if (ucNANDInit)
    {
        smcStart();
        ucNANDInit=0;
    }
    smcReadWriteUI(0,1);		//replace above marking code
#elif ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
    spiReadUI();

#endif
}

void Read_FB_Setting(void)
{
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

    if (ucNANDInit)
    {
        smcStart();
        ucNANDInit=0;
    }
    smc_temp();		//replace above marking code
#elif ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

    if (spiInitIdx == SPI_UNINITED)
    {
        if (spiStart()==0)
        {
            DEBUG_SPI("Error! Init SPI Flash Error!\n");
            return;
        }
    }
    spiReadFBSetting();
#endif
}

void Save_UI_Setting(void)
{
    u8 check_val = 0;
    u8 update = 0, err;
    //--------------------------//
    
   #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
      return;
   #endif
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    update = uiCompareSaveData();
    if (update == 1)
    {
        DEBUG_UI("\n=Save_UI_Setting=\n");
        check_val = uiGetSaveChecksum();
	    iconflag[UI_MENU_SETIDX_CHECK] = check_val;
        start_iconflag[UI_MENU_SETIDX_CHECK] = iconflag[UI_MENU_SETIDX_CHECK];

#if(FLASH_OPTION == FLASH_NO_DEVICE)

#elif ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

        if (gInsertNAND==1)
        {
            smcWriteBackUISetting();
        }
        else
        {
            if (ucNANDInit)
            {
                smcStart();
                ucNANDInit=0;
            }
            else
            {
                sysSD_Disable();
                sysNAND_Enable();
            }
            smcWriteBackUISetting();
            sysNAND_Disable();
            sysSD_Enable();
        }

#else
        if (spiInitIdx == SPI_UNINITED)
        {
            if (spiStart()==0)
            {
                DEBUG_SPI("Error! Init SPI Flash Error!\n");
                OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
                return;
            }
        }
        sysSD_Disable();
        sysSPI_Enable();
        spiWriteUI();
        #if UI_ICONFLAG_BACKUP
        spiWriteUIBackup();
        #endif
        sysSPI_Disable();
        sysSD_Enable();
#endif
    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_USE_FLASH, OS_FLAG_CLR, &err);
}


void uiReadRFIDFromFlash(u8 *FlashAddr)
{
    memcpy(&uiRFID,FlashAddr,sizeof(uiRFID));
    DEBUG_UI("RFID: 0x%x, 0x%x, 0x%x, 0x%x \n",uiRFID[0],uiRFID[1],uiRFID[2],uiRFID[3]);
    FlashAddr+= UI_SETUP_RFID_SIZE;
    
    memcpy(&uiRFCODE,FlashAddr,sizeof(uiRFCODE));
    FlashAddr+= UI_SETUP_RFCODE_SIZE;
    DEBUG_UI("RFCODE: 0x%x, 0x%x, 0x%x, 0x%x \n",uiRFCODE[0],uiRFCODE[1],uiRFCODE[2],uiRFCODE[3]);

#if (SW_APPLICATION_OPTION == MR8100_BABYMONITOR)
    memcpy(&uiRFSTAT,FlashAddr,sizeof(uiRFSTAT)); //Lucian1113
    FlashAddr+= UI_SETUP_RFSTAT_SIZE;
    DEBUG_UI("RFSTAT: 0x%x, 0x%x, 0x%x, 0x%x \n",uiRFSTAT[0],uiRFSTAT[1],uiRFSTAT[2],uiRFSTAT[3]);
#endif    
}


void uiReadNetworkIDFromFlash(u8 *FlashAddr)
{
    memcpy(&uiP2PID,FlashAddr,sizeof(uiP2PID));
    DEBUG_MAIN("TUTKID: %s \n",uiP2PID);

    memcpy(&uiMACAddr,FlashAddr+UI_SETUP_GUID_SIZE,sizeof(uiMACAddr));
    DEBUG_MAIN("MAC: %x-%x-%x-%x-%x-%x \n",uiMACAddr[0],uiMACAddr[1],uiMACAddr[2],uiMACAddr[3],uiMACAddr[4],uiMACAddr[5]);
}


void uiReadVersionFromFlash(u8 * FlashAddr)
{
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    u8 uiVersiontmp[32]={0};
    u8 level=0;
    
    memcpy(&uiVersiontmp , FlashAddr, sizeof(uiVersion));
    gpioGetLevel(GPIO_GROUP_MODE_SEL, GPIO_BIT_MODE_SEL, &level);
    if(level==1)
    {
        uiIsVM9710=1;
        sprintf((char*)uiVersion,"VM9710%s",uiVersiontmp);
    }
    else
    {
        uiIsVM9710=0;
        sprintf((char*)uiVersion,"VM9700%s",uiVersiontmp);
    }
#else
    memcpy(&uiVersion , FlashAddr, sizeof(uiVersion));
#endif
    memcpy(&uiVersionTime , FlashAddr+sizeof(uiVersion), sizeof(uiVersionTime));
    DEBUG_UI("*******Version: %s***%s***\n",uiVersion,uiVersionTime);
}

void uiWriteRFIDFromFlash(u8 *FlashAddr)
{
    memcpy((void *)(FlashAddr),  (void *)&uiRFID, sizeof(uiRFID));
    FlashAddr += UI_SETUP_RFID_SIZE;
    
    memcpy((void *)(FlashAddr ),  (void *)&uiRFCODE, sizeof(uiRFCODE));
    FlashAddr+= UI_SETUP_RFCODE_SIZE;

#if (SW_APPLICATION_OPTION == MR8100_BABYMONITOR)//Lucian1113
    memcpy((void *)(FlashAddr ),  (void *)&uiRFSTAT, sizeof(uiRFSTAT));
    FlashAddr+= UI_SETUP_RFSTAT_SIZE;   
#endif

}

void uiWriteNetworkIDFromFlash(u8 *FlashAddr)
{
    memcpy((void *)FlashAddr,  (void *)&uiP2PID, sizeof(uiP2PID));
    memcpy((void *)(FlashAddr+UI_SETUP_GUID_SIZE), (void *)&uiMACAddr,sizeof(uiMACAddr));
}

u8 uiTemperatureConversion(u8 degreeVal, u8 ConverseType)
{
    if (ConverseType == UI_DEGREE_TYPE_FAHRENHEIT) /* (*F) Converse to (*C) */
        return (u8)((((degreeVal - 32) * 5) / 9.0) + 0.5);
    else /* (*C) Converse to (*F) */
        return (u8)((((degreeVal * 9) / 5) + 32.0) + 0.5);
}




